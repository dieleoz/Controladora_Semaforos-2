#include "modo_noche.h"
#include "noche_flash.h"
#include "coordinador.h"
#include "modos.h"
#include "reloj.h"
#include "modo_degradado.h"   // reloj_segundosDesde2000(): la fecha, para el dia de la semana
#include "bluetooth.h"
#include "limites_ciclo.h"
#include <stdio.h>
#include <string.h>

// D-109: el rango de tr. El suelo es el del despeje del ciclo (lo que tarda el tramo en
// vaciarse); el techo, la fase mas larga del equipo.
static_assert(NOCHE_TR_MAX_S == (uint16_t)VERDE_MIN_MAX * 60U, "D-109: el techo de tr es VERDE_MIN_MAX");
static_assert(NOCHE_TR_FABRICA_S >= DESPEJE_SEG_MIN && NOCHE_TR_FABRICA_S <= NOCHE_TR_MAX_S,
              "D-106: el tr de fabrica fuera de su propio rango");

enum FaseNoche { FN_ENTRANDO, FN_DESTELLO, FN_SALIENDO };

static NocheConfig cfg;
static FaseNoche fase = FN_ENTRANDO;
static ModoSistema origen = MODO_AUTOMATICO;    // el modo del que vino: destino de la salida por hora
static ModoSistema destino = MODO_AUTOMATICO;
static int8_t activa = -1;                      // la franja en curso; -1 = ninguna
static bool rojoConsta = false;                 // el rojo consta en los dos desde tRojo
static unsigned long tRojo = 0;
static uint16_t trEnCurso = NOCHE_TR_FABRICA_S;  // el tr se fija al empezar cada todo-rojo
static int32_t ultimoMinuto = -1;               // el ultimo minuto (desde 2000) ya mirado
static int32_t mDesde = 0, mHasta = 0;          // D-115: la franja activa en minutos desde 2000, [desde, hasta)
static bool horaPerdida = false;
static unsigned long tAvisoHora = 0;

static bool apagada(const FranjaNoche& f) { return f.dias == 0 || f.inicio == f.fin; }

void modoNoche_cargar() {
  if (nocheFlash_leer(&cfg) && cfg.trSeg >= DESPEJE_SEG_MIN && cfg.trSeg <= NOCHE_TR_MAX_S) return;
  memset(&cfg, 0, sizeof(cfg));   // sin registro (o tras una carga, que la borra): sin franjas
  cfg.trSeg = NOCHE_TR_FABRICA_S;
}

// El todo-rojo de entrada y de salida: la puerta de SET_MODO:MANUAL, y tr desde que consta.
static void empezarTodoRojo(FaseNoche f) {
  fase = f;
  rojoConsta = false;
  trEnCurso = cfg.trSeg;
  coordinador_vetarVerde(true);
  coordinador_forzarRojoTotal();   // amarillo si habia verde (D-45), GO_RED, rojo del Esclavo sin constar
}

static void iniciarSalida(ModoSistema d, const char* motivo) {
  destino = d;
  bluetooth_reportarEvento("NOCHE", motivo);   // al EMPEZAR la salida: tr segundos antes del destino
  empezarTodoRojo(FN_SALIENDO);
}

// D-111: el flanco. Solo con hora fiable Y fecha (SFTY-18: sin hora no entra), y solo al
// CAMBIAR de minuto: el primer minuto que se ve no es flanco (un corte no reanuda).
// D-115: se sale en cualquier minuto fiable FUERA de [inicio, fin) de la franja en curso, contado
// desde 2000; no en el minuto exacto de fin, que un salto de reloj o la hora perdida se saltan.
void modoNoche_vigilar() {
  const uint32_t s = reloj_segundosDesde2000();
  if (!reloj_horaFiable() || s == 0) return;
  const int32_t m = (int32_t)(s / 60UL);
  if (m == ultimoMinuto) return;
  const bool primero = (ultimoMinuto < 0);
  ultimoMinuto = m;
  if (primero) return;
  const uint16_t minDia = (uint16_t)(m % 1440);
  const uint8_t dia = (uint8_t)(((m / 1440) + 5) % 7);   // 0 = lunes: el dia 0 de la cuenta (2000) fue sabado
  const ModoSistema modo = modoActual_get();
  if (modo == MODO_NOCHE && activa >= 0 && fase != FN_SALIENDO && (m < mDesde || m >= mHasta))
    iniciarSalida(origen, "SALIDA_HORA");   // sin return: una franja que empieza ahora se anota abajo
  for (int8_t i = 0; i < (int8_t)NOCHE_FRANJAS; i++) {
    const FranjaNoche& f = cfg.f[i];
    if (apagada(f) || minDia != f.inicio || ((f.dias >> dia) & 1U) == 0) continue;
    if (modo == MODO_AUTOMATICO || modo == MODO_INTELIGENTE) {
      origen = modo;
      activa = i;
      mDesde = m;
      mHasta = m + (int32_t)((f.fin + 1440U - f.inicio) % 1440U);   // apagada() excluye inicio == fin
      modoActual_set(MODO_NOCHE);   // main.cpp llama a modoNoche_setup() en el flanco de modo
      return;
    }
    bluetooth_reportarEvento("NOCHE", "FRANJA_SALTADA");   // 5.10 (e): tambien en noche (contigua)
  }
}

void modoNoche_setup() {
  empezarTodoRojo(FN_ENTRANDO);
  bluetooth_reportarEvento("NOCHE", "ENTRADA");
}

void modoNoche_loop() {
  // El coordinador lo mueve main.cpp (coordinador_actualizar_background()).
  if (fase != FN_DESTELLO) {
    const bool consta = coordinador_rojoConstaEnLosDos();
    if (consta && !rojoConsta) tRojo = millis();
    rojoConsta = consta;
    if (consta && millis() - tRojo >= (unsigned long)trEnCurso * 1000UL) {
      if (fase == FN_SALIENDO) {
        modoActual_set(destino);   // main.cpp: modoNoche_alSalir() y el setup() del destino
      } else if (activa >= 0 && coordinador_entrarNoche(cfg.f[activa].rojo)) {
        fase = FN_DESTELLO;
      }
    }
  }
  // D-111: sin hora fiable DENTRO se queda (la luz no depende del reloj) y avisa cada minuto.
  if (reloj_horaFiable()) {
    horaPerdida = false;
  } else if (!horaPerdida || millis() - tAvisoHora >= NOCHE_AVISO_HORA_MS) {
    horaPerdida = true;
    tAvisoHora = millis();
    bluetooth_reportarAlarma("NOCHE", "HORA_NO_FIABLE", "SALGA_POR_APP");   // 13: el $ALARM cabe (esp32_07)
  }
}

void modoNoche_alSalir() {
  coordinador_vetarVerde(false);
  activa = -1;
  fase = FN_ENTRANDO;
  horaPerdida = false;
}

// --- Las ordenes (D-110), molde SET_TIEMPOS -------------------------------------------
static void responderConsulta(void (*tx)(const char*)) {
  char p[156];
  int n = snprintf(p, sizeof(p), "$ACK,CMD:NOCHE,RESULT:OK,TR:%u", (unsigned)cfg.trSeg);
  for (uint8_t i = 0; i < NOCHE_FRANJAS && n > 0 && n < (int)sizeof(p); i++) {
    const FranjaNoche& f = cfg.f[i];
    char d[8];
    for (uint8_t b = 0; b < 7; b++) d[b] = ((f.dias >> b) & 1U) ? "LMXJVSD"[b] : '-';
    d[7] = '\0';
    n += snprintf(p + n, sizeof(p) - n, ",F%u:%s-%02u%02u-%02u%02u-%s", (unsigned)(i + 1), d,
                  f.inicio / 60U, f.inicio % 60U, f.fin / 60U, f.fin % 60U,
                  apagada(f) ? "OFF" : (f.rojo ? "R" : "A"));
  }
  tx(p);
}

static bool horaHHMM(const char* t, uint16_t* minutos) {
  const int h = (t[0] - '0') * 10 + (t[1] - '0'), mi = (t[2] - '0') * 10 + (t[3] - '0');
  if (h > 23 || mi > 59) return false;
  *minutos = (uint16_t)(h * 60 + mi);
  return true;
}

// 0 = OK; si no, el DESC del $ERR. La flash se escribe solo si todo lo demas paso.
static const char* fijarFranja(const char* a) {
  int n = 0;
  char dias[8] = "", ini[5] = "", fin[5] = "", txt[4] = "", col = 0, sobra = 0;
  FranjaNoche f = { 0, 0, 0, false };
  if (sscanf(a, "%d,%3[A-Z]%c", &n, txt, &sobra) == 2 && strcmp(txt, "OFF") == 0) {
    // SET_NOCHE:n,OFF: la franja queda a cero
  } else if (sscanf(a, "%d,%7[LMXJVSD-],%4[0-9],%4[0-9],%c%c", &n, dias, ini, fin, &col, &sobra) == 5 &&
             strlen(dias) == 7 && strlen(ini) == 4 && strlen(fin) == 4 && (col == 'A' || col == 'R')) {
    for (uint8_t b = 0; b < 7; b++) {
      if (dias[b] == "LMXJVSD"[b]) f.dias |= (uint8_t)(1U << b);
      else if (dias[b] != '-') return "FORMATO_INVALIDO";
    }
    if (!horaHHMM(ini, &f.inicio) || !horaHHMM(fin, &f.fin)) return "RANGO";
    f.rojo = (col == 'R');
  } else {
    return "FORMATO_INVALIDO";
  }
  if (n < 1 || n > (int)NOCHE_FRANJAS) return "RANGO";
  if (modoActual_get() == MODO_NOCHE && activa == n - 1) return "EN_NOCHE_SALGA_PRIMERO";
  NocheConfig nueva = cfg;
  nueva.f[n - 1] = f;
  if (!nocheFlash_escribir(&nueva)) return "FLASH_NO_RELEE";
  cfg = nueva;
  return 0;
}

static const char* fijarTr(const char* a) {
  int s = 0;
  char sobra = 0;
  if (sscanf(a, "%d%c", &s, &sobra) != 1) return "FORMATO_INVALIDO";
  if (s < (int)DESPEJE_SEG_MIN || s > (int)NOCHE_TR_MAX_S) return "RANGO";
  NocheConfig nueva = cfg;
  nueva.trSeg = (uint16_t)s;
  if (!nocheFlash_escribir(&nueva)) return "FLASH_NO_RELEE";
  cfg = nueva;   // un todo-rojo ya empezado sigue con su trEnCurso
  return 0;
}

static void contestar(void (*tx)(const char*), const char* cmd, const char* err) {
  char p[80];
  if (err) snprintf(p, sizeof(p), "$ERR,CMD:%s,DESC:%s", cmd, err);
  else snprintf(p, sizeof(p), "$ACK,CMD:%s,RESULT:OK", cmd);
  tx(p);
  if (!err) bluetooth_reportarEvento("APP_BLUETOOTH", cmd);
}

// Dentro de la noche: las salidas por la app (D-111). A AUTO, INTELIGENTE o MANUAL, con PIN,
// por el todo-rojo; una segunda mientras tanto, SALIDA_EN_CURSO. MENU, ALCANCE y FORZAR_ROJO
// siguen al despachador (rojo e inmediato): aqui solo se anota el $EVENT de la salida.
// SET_MODO:DEG_T y SET_MODO:AMBAR no estan en 5.10 (f): el despachador los entraria con su
// propio rojo, sin tr; se rechazan sin tocar luces. (El Degradado automatico y el ambar del
// Esclavo no son ordenes de la app: siguen sacando de la noche con su $EVENT.)
static bool ordenEnNoche(const char* a, bool conPin, void (*tx)(const char*)) {
  if (conPin && (!strncmp(a, "SET_MODO:DEG_T:", 15) || !strcmp(a, "SET_MODO:AMBAR"))) {
    char p[80];
    snprintf(p, sizeof(p), "$ERR,CMD:%.14s,DESC:EN_NOCHE_SALGA_PRIMERO", a);
    tx(p);
    return true;
  }
  if (!strcmp(a, "SET_MODO:MENU") || !strcmp(a, "SET_MODO:ALCANCE")) {
    bluetooth_reportarEvento("NOCHE", "SALIDA_APP");
    return false;
  }
  if (!strcmp(a, "FORZAR_ROJO")) {   // D-111: rojo inmediato y fuera de la noche, a MANUAL
    bluetooth_reportarEvento("NOCHE", "SALIDA_APP");
    modoActual_set(MODO_MANUAL);
    return false;
  }
  ModoSistema d;
  if (!strcmp(a, "SET_MODO:AUTO")) d = MODO_AUTOMATICO;
  else if (!strcmp(a, "SET_MODO:INTELIGENTE")) d = MODO_INTELIGENTE;
  else if (!strcmp(a, "SET_MODO:MANUAL")) d = MODO_MANUAL;
  else return false;
  if (!conPin) return false;   // el despachador contesta PIN_INVALIDO
  char p[80];
  if (fase == FN_SALIENDO) {
    snprintf(p, sizeof(p), "$ERR,CMD:%s,DESC:SALIDA_EN_CURSO", a);
  } else {
    iniciarSalida(d, "SALIDA_APP");
    snprintf(p, sizeof(p), "$ACK,CMD:%s,RESULT:SALIENDO_TODO_ROJO", a);
  }
  tx(p);
  return true;
}

bool modoNoche_orden(const char* cmd, void (*tx)(const char*)) {
  const bool conPin = strncmp(cmd, "CMD:PIN:1234:", 13) == 0;
  if (!conPin && strncmp(cmd, "CMD:", 4) != 0) return false;
  const char* a = cmd + (conPin ? 13 : 4);
  if (!strcmp(a, "CONSULTA_NOCHE")) {
    responderConsulta(tx);
    return true;
  }
  if (conPin && !strncmp(a, "SET_NOCHE:", 10)) {
    contestar(tx, "SET_NOCHE", fijarFranja(a + 10));
    return true;
  }
  if (conPin && !strncmp(a, "SET_NOCHE_TR:", 13)) {
    contestar(tx, "SET_NOCHE_TR", fijarTr(a + 13));
    return true;
  }
  if (modoActual_get() == MODO_NOCHE) return ordenEnNoche(a, conPin, tx);
  return false;
}
