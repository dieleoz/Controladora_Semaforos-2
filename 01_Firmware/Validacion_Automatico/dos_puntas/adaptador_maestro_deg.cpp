// ===== Validacion_Automatico/dos_puntas/adaptador_maestro_deg.cpp =====
//
// LA PUNTA MAESTRO **CON SU MODO DEGRADADO REAL**, DENTRO DE SU PROPIA DLL.
//
// Es hermano de adaptador_maestro.cpp y NO lo sustituye: aquel monta el Modo
// Automatico -coordinador + modo_automatico + mando- y declara en su cabecera que
// "en este arnes el MODO DEGRADADO del Maestro no existe". Este monta lo contrario:
// el Degradado, que es el unico modo en el que CADA PUNTA ENCIENDE SU VERDE SIN
// PREGUNTARLE A NADIE.
//
// Compila REALES:
//
//   modo_degradado.cpp  la unica linea del firmware del Maestro que enciende un verde
//                       sin confirmacion del otro extremo, con su puerta de entrada,
//                       su todo-rojo y su limite duro de 48 h
//   modo_ambar.cpp      donde cae el Degradado al rendirse
//   coordinador.cpp     la sincronizacion horaria REAL (SFTY-23): hora, configuracion
//                       de ciclo y medida de desfase, con sus ACK y sus reintentos
//   semaforo.cpp        las luces y el enclavamiento SFTY-2
//   respaldo.cpp        el dominio de la pila con su calcularSuma() de Horner, del que
//                       depende que el modo pueda reanudar tras un corte (N-20)
//   modos.cpp           modoActual_get/set, el estado del sistema
//
// ===========================================================================
// LA EXCLUSION "modo_degradado.cpp ARRASTRA u8g2" ERA FALSA. MEDIDA:
// ===========================================================================
//
// adaptador_maestro.cpp la escribio de buena fe: "modo_degradado.cpp incluye lcd.h y
// menu.h y arrastra lo mismo [que main.cpp: la pantalla entera, u8g2]". Se comprobo en
// vez de creerla, que es lo que pide la regla del instrumento:
//
//   grep -n '#include' Maestro/include/*.h
//     -> NINGUNA de las 23 cabeceras del Maestro incluye <U8g2lib.h>.
//        lcd.h incluye <Arduino.h> y "reloj.h". menu.h, <Arduino.h>. Y nada mas.
//   grep -n '#include' Maestro/src/lcd.cpp
//     -> 4:#include <U8g2lib.h>
//
// U8g2 lo arrastra lcd.cpp, no lcd.h. Compilar modo_degradado.cpp NO obliga a compilar
// lcd.cpp: basta con DEFINIR las tres funciones de dibujo que llama. Este fichero usa
// por eso las cabeceras REALES lcd.h, menu.h y botones.h del Maestro -no sustitutos-,
// que es estrictamente mejor: una declaracion copiada puede divergir en silencio, y la
// real no puede.
//
// POR QUE NO SE REUTILIZA EL CAMINO DE Validacion_LCD, con el numero delante. Aquel
// compila 131 ficheros .c del nucleo de U8g2 para poder enlazar lcd.cpp y medir
// GEOMETRIA DE PANTALLA sobre un framebuffer. Aqui no se mide ni una letra: se mide
// que dos puntas no den verde a la vez. Traer los 131 ficheros solo para que exista un
// dibujo que nadie lee no acerca este arnes ni un milimetro a su propiedad, y el
// dibujo del Degradado ya lo mide Validacion_LCD sobre el lcd.cpp real. Cuesta 131
// unidades de compilacion; da cero comprobaciones nuevas.
//
// ===========================================================================
// QUE NO SE COMPILA, DICHO PARA QUE NADIE LO CUENTE COMO CUBIERTO
// ===========================================================================
//
//   - main.cpp. Su loop() se transcribe abajo LITERALMENTE en pasoPrincipal(), acotado
//     a los modos que esta DLL compila. No entra entero porque arrastra los ocho modos
//     y con ellos menu.cpp y u8g2.
//   - lcd.cpp y menu.cpp: la pantalla. Las llamadas se cuentan, no se dibujan.
//   - protocolo.cpp: CRC, rafaga y proteccion de replay. Aqui la radio son dos colas.
//   - reloj.cpp: incluye <STM32RTC.h>, que no tiene sustituto en el repositorio. Se
//     modela el PERIFERICO -un contador de segundos y sus getters-, nunca una regla.
//     El modelo es el MISMO BLOQUE LITERAL que ya usa adaptador_esclavo.cpp, para que
//     las dos puntas cuenten el tiempo con la misma aritmetica y una diferencia entre
//     ellas sea del firmware y no del arnes.
//   - mando.cpp, bluetooth.cpp, botones.cpp: no deciden la fase del Degradado.

#include "punta_api.h"

#include <stdio.h>
#include <string.h>

#include "Arduino.h"
#include "pines.h"

// CABECERAS REALES DEL MAESTRO, sin sustituto. Ver la medida de arriba.
#include "botones.h"
#include "lcd.h"
#include "menu.h"
#include "modos.h"

#include "coordinador.h"
#include "semaforo.h"
#include "protocolo.h"
#include "reloj.h"
#include "respaldo.h"
#include "modo_ambar.h"
#include "modo_degradado.h"
#include "stm32f1xx_hal.h"   // para volcar el dominio de respaldo real

// ---------------------------------------------------------------------------
// EL RELOJ SIMULADO Y LOS PINES OBSERVADOS. Esta DLL tiene los SUYOS.
// ---------------------------------------------------------------------------
unsigned long arnes_millis_valor = 0;
int arnes_pines[64];
int arnes_entradas[64];
unsigned long arnes_escrituras = 0;
unsigned long arnes_toques[64];

// Los diez registros del dominio VBAT que respaldo.cpp recorre de verdad.
BKP_Simulado arnes_bkp;

static_assert(sizeof(RF_Packet) == 4, "RF_Packet dejo de medir 4 bytes");

// ---------------------------------------------------------------------------
// BOTONES SIMULADOS. Bloque literal de adaptador_maestro.cpp: leerlos los gasta.
// botonCancelar() lo consultan modo_degradado_loop() y modo_ambar_loop(), asi que la
// salida por boton se puede ejercer de verdad.
// ---------------------------------------------------------------------------
static bool g_pulsarArriba = false, g_pulsarAbajo = false;
static bool g_pulsarAceptar = false, g_pulsarCancelar = false;

void botones_setup() {}
void botones_actualizar() {}
bool botonArriba()   { bool v = g_pulsarArriba;   g_pulsarArriba = false;   return v; }
bool botonAbajo()    { bool v = g_pulsarAbajo;    g_pulsarAbajo = false;    return v; }
bool botonCancelar() { bool v = g_pulsarCancelar; g_pulsarCancelar = false; return v; }
bool botonAceptar()  { bool v = g_pulsarAceptar;  g_pulsarAceptar = false;  return v; }

// N-73: la Caja Negra. El stub no puede limitarse a callar. [literal de adaptador_maestro.cpp]
static char g_ultimaAlarmaEvento[48] = "";
static int  g_alarmasEmitidas = 0;
void bluetooth_reportarAlarma(const char* evento, const char* causa, const char* accion) {
  (void)causa; (void)accion;
  snprintf(g_ultimaAlarmaEvento, sizeof(g_ultimaAlarmaEvento), "%s", evento);
  g_alarmasEmitidas++;
}
void bluetooth_reportarEvento(const char*, const char*) {}

// ---------------------------------------------------------------------------
// PANTALLA SIMULADA. SOLO las funciones que los .cpp compilados llaman de verdad.
//
// Se declaran contra la lcd.h REAL, asi que si alguien cambiara la firma de
// lcd_dibujarDegradado() esto dejaria de compilar en vez de divergir en silencio. Un
// sustituto de cabecera no habria dado ese aviso.
// ---------------------------------------------------------------------------
static unsigned long g_lcdRedibujos = 0;
static unsigned long g_lcdRechazos = 0;
static unsigned long g_lcdAmbar = 0;
static char g_ultimaLinea1[32] = "";

void lcd_dibujarDegradado(const char* fase, const char* detalle, unsigned long restanteSeg,
                          unsigned long minutosDesdeSync, bool syncVencida, const char* aviso) {
  (void)fase; (void)detalle; (void)restanteSeg; (void)minutosDesdeSync;
  (void)syncVencida; (void)aviso;
  g_lcdRedibujos++;
}
void lcd_dibujarDegradadoRechazo(const char* linea1, const char* linea2) {
  (void)linea2;
  snprintf(g_ultimaLinea1, sizeof(g_ultimaLinea1), "%s", linea1 ? linea1 : "");
  g_lcdRechazos++;
}
void lcd_dibujarDegradadoAmbar(const char* linea1, const char* linea2) {
  (void)linea2;
  snprintf(g_ultimaLinea1, sizeof(g_ultimaLinea1), "%s", linea1 ? linea1 : "");
  g_lcdAmbar++;
}
void menu_setup() {}

// ---------------------------------------------------------------------------
// EL RTC SIMULADO. BLOQUE LITERAL de adaptador_esclavo.cpp, con el mismo contrato:
// contador MONOTONO que sobrevive al corte, y la hora de pared anclada a millis().
//
// Que sea literalmente el mismo modelo en las dos puntas es la condicion para que el
// desfase que este arnes inyecta sea del ESCENARIO y no del arnes: si cada punta
// contara los segundos con una aritmetica distinta, el solape medido podria salir de
// la diferencia entre los dos modelos y no del firmware.
// ---------------------------------------------------------------------------
static bool     g_rtcEnHora = false;
static uint32_t g_rtcBaseSegundos = 0;      // valor del contador cuando millis()==g_rtcAncla
static unsigned long g_rtcAncla = 0;
static uint32_t g_rtcSegundosDelDiaBase = 0;
static uint8_t  g_rtcDia = 0;

static uint32_t rtcTranscurrido() {
  return (uint32_t)((arnes_millis_valor - g_rtcAncla) / 1000UL);
}

void reloj_setup() {}
void reloj_actualizar() {}
bool reloj_enHora() { return g_rtcEnHora; }
bool reloj_hayCristal() { return true; }

uint32_t reloj_contadorSegundos() {
  if (!g_rtcEnHora) return 0;   // el cero significa "no hay reloj", como en el real
  return g_rtcBaseSegundos + rtcTranscurrido();
}

uint32_t reloj_segundosDelDia() {
  if (!g_rtcEnHora) return 0;
  return (g_rtcSegundosDelDiaBase + rtcTranscurrido()) % 86400UL;
}
uint8_t reloj_hora()   { return (uint8_t)(reloj_segundosDelDia() / 3600UL); }
uint8_t reloj_minuto() { return (uint8_t)((reloj_segundosDelDia() / 60UL) % 60UL); }
uint8_t reloj_segundo(){ return (uint8_t)(reloj_segundosDelDia() % 60UL); }
uint8_t reloj_dia()    { return g_rtcEnHora ? g_rtcDia : 0; }

void reloj_ajustar(uint8_t hora, uint8_t minuto, uint8_t segundo, uint8_t dia) {
  if (hora > 23 || minuto > 59 || segundo > 59 || dia > 31) return;
  uint32_t contador = reloj_contadorSegundos();
  g_rtcAncla = arnes_millis_valor;
  g_rtcBaseSegundos = contador ? contador : 1;
  g_rtcSegundosDelDiaBase = (uint32_t)hora * 3600UL + (uint32_t)minuto * 60UL + segundo;
  if (dia >= 1) g_rtcDia = dia;
  g_rtcEnHora = true;
}

// coordinador.cpp lo llama cada 10 min para que las dos puntas vuelquen el dia a la
// vez. Aqui el calendario es un solo byte de dia, asi que no hay mes que fijar: se
// cuenta la llamada para que el arnes pueda exigir que ocurra.
static unsigned long g_fijarEnero = 0;
void reloj_fijarEnero() { g_fijarEnero++; }

// ---------------------------------------------------------------------------
// LA RADIO: DOS COLAS. [bloque literal de adaptador_maestro.cpp]
// ---------------------------------------------------------------------------
#define COLA_MAX 32
struct Cola {
  RF_Packet dato[COLA_MAX];
  int cabeza = 0, cola = 0;
  bool meter(const RF_Packet& p) {
    int sig = (cola + 1) % COLA_MAX;
    if (sig == cabeza) return false;   // llena: se pierde, como en el aire
    dato[cola] = p; cola = sig; return true;
  }
  bool sacar(RF_Packet* p) {
    if (cabeza == cola) return false;
    *p = dato[cabeza]; cabeza = (cabeza + 1) % COLA_MAX; return true;
  }
};
static Cola g_tx, g_rx;
static unsigned long g_tramasEmitidas = 0;

void protocolo_setup() {}
void protocolo_resetReplayProtection() {}

void protocolo_enviarPaquete(uint8_t cmd, uint8_t param) {
  RF_Packet p; p.msgID = 0; p.command = cmd; p.param = param; p.crc = 0;
  g_tramasEmitidas++;
  g_tx.meter(p);
}

bool protocolo_hayPaqueteDisponible(RF_Packet* destino) {
  return g_rx.sacar(destino);
}

// ---------------------------------------------------------------------------
// UN TICK DE main.cpp, EN EL ORDEN REAL Y CON SUS DOS REGLAS DE FONDO.
//
// Transcripcion literal de Maestro/src/main.cpp::loop() acotada a los modos que esta
// DLL compila. Las dos lineas que NO son despacho y si son comportamiento se
// conservan tal cual, porque las dos deciden luz:
//
//   1. semaforo_actualizar() SIEMPRE, en todos los modos. El comentario de main.cpp
//      explica que sin esto el cabezal se quedaba a oscuras.
//   2. El coordinador queda FUERA en MODO_DEGRADADO y MODO_AMBAR: en esos dos modos el
//      Maestro CALLA en la radio a proposito. Quitarlo aqui convertiria el Degradado en
//      "modo normal con otra pantalla" y el arnes no mediria nada de lo que viene a
//      medir.
//   3. Al SALIR del Degradado por cualquier via se borra el indicador de la pila (N-20).
//      Ese punto es el unico por el que pasan todos los caminos de salida.
// ---------------------------------------------------------------------------
static ModoSistema modoAnterior = MENU;

static void pasoPrincipal() {
  botones_actualizar();
  semaforo_actualizar();

  ModoSistema modo = modoActual_get();
  if (modo != MODO_AUTOMATICO && modo != MODO_DEGRADADO && modo != MODO_AMBAR) {
    coordinador_actualizar_background();
  }

  if (modo != modoAnterior) {
    if (modoAnterior == MODO_DEGRADADO) {
      respaldo_guardarDegradado(false);
    }
    switch (modo) {
      case MODO_DEGRADADO:   modo_degradado_setup();  break;
      case MODO_AMBAR:       modo_ambar_setup();      break;
      case MENU:             menu_setup();            break;
      default: break;
    }
    modoAnterior = modo;
  }

  switch (modo) {
    case MODO_DEGRADADO:  modo_degradado_loop();  break;
    case MODO_AMBAR:      modo_ambar_loop();      break;
    default: break;   // MENU: menu_loop() es pantalla y no se compila
  }
}

// ---------------------------------------------------------------------------
// LA API QUE VE EL ORQUESTADOR
// ---------------------------------------------------------------------------
extern "C" {

PUNTA_API const char* punta_nombre(void) { return "MAESTRO"; }

// EL setup() REAL DE main.cpp, acotado a lo que esta DLL compila y EN SU ORDEN, que es
// de fondo y no estetico:
//
//   respaldo_setup() DESPUES del reloj -mismo dominio de pila-, y
//   modo_degradado_reanudarTrasCorte() ANTES de modo_degradado_publicarConfig(),
//   porque publicar guarda el ciclo en la pila y despues respaldo_hayCiclo() seria
//   cierto SIEMPRE: esa condicion dejaria de comprobar nada.
//
// El delay(2000) de N-22 se conserva: aqui no duerme, ADELANTA el reloj simulado, que
// es lo que ese delay significa para todo lo que venga despues.
PUNTA_API void punta_arrancar(void) {
  for (int i = 0; i < 64; i++) { arnes_pines[i] = LOW; arnes_entradas[i] = LOW; }
  arnes_escrituras = 0;
  for (int i = 0; i < 64; i++) arnes_toques[i] = 0;

  botones_setup();
  coordinador_setup();
  delay(2000);
  reloj_setup();
  respaldo_setup();

  const bool reanudarDegradado = modo_degradado_reanudarTrasCorte();
  modo_degradado_publicarConfig();

  if (reanudarDegradado) {
    modoActual_set(MODO_DEGRADADO);
    modo_degradado_setup();
    modoAnterior = MODO_DEGRADADO;
  } else {
    modoActual_set(MENU);
    modoAnterior = MENU;
  }
}

PUNTA_API void punta_tick(unsigned long ms) {
  arnes_millis_valor = ms;
  pasoPrincipal();
}

PUNTA_API int punta_pin(int pin) {
  return (pin >= 0 && pin < 64) ? arnes_pines[pin] : -1;
}
PUNTA_API int punta_estado(void) { return (int)semaforo_estado(); }
PUNTA_API unsigned long punta_escrituras(void) { return arnes_escrituras; }

PUNTA_API int punta_tx(unsigned char* trama4) {
  RF_Packet p;
  if (!g_tx.sacar(&p)) return 0;
  memcpy(trama4, &p, sizeof(RF_Packet));
  return 1;
}

PUNTA_API void punta_rx(const unsigned char* trama4) {
  RF_Packet p;
  memcpy(&p, trama4, sizeof(RF_Packet));
  g_rx.meter(p);
}

PUNTA_API void punta_entrada(int pin, int nivel) {
  if (pin >= 0 && pin < 64) arnes_entradas[pin] = nivel;
}

PUNTA_API void punta_pulsar(int boton) {
  switch (boton) {
    case 1: g_pulsarArriba = true; break;
    case 2: g_pulsarAbajo = true; break;
    case 3: g_pulsarAceptar = true; break;
    case 4: g_pulsarCancelar = true; break;
    default: break;
  }
}

PUNTA_API long punta_mando(const char* que, long arg) {
  // --- El reloj de pared de esta punta -------------------------------------
  // arg empaquetado como d*1000000 + h*10000 + m*100 + s. Entra por reloj_ajustar(),
  // que es la MISMA puerta que usa la pantalla AJUSTAR HORA del equipo real.
  if (!strcmp(que, "ajustar_reloj")) {
    uint8_t d = (uint8_t)((arg / 1000000L) % 100);
    uint8_t h = (uint8_t)((arg / 10000L) % 100);
    uint8_t m = (uint8_t)((arg / 100L) % 100);
    uint8_t s = (uint8_t)(arg % 100L);
    reloj_ajustar(h, m, s, d);
    return reloj_enHora() ? 1 : 0;
  }
  // LA DERIVA DEL CRISTAL, Y POR QUE NO SE HACE CON reloj_ajustar() NI CON EL DOMINIO.
  //
  // Mueve la hora de pared de ESTA punta arg segundos, CONSERVANDO LA FASE SUB-SEGUNDO
  // -no se toca g_rtcAncla-. Es lo que hace un cristal que corre mas rapido: la hora se
  // separa, el instante en que cambia el segundo no salta.
  //
  // Las otras dos vias reanclaban: reloj_ajustar() y la escritura del indice 10 del
  // dominio ponen g_rtcAncla = millis(), con lo que la frontera de segundo del RTC
  // saltaba al instante de la inyeccion. El arnes acababa midiendo un residuo
  // sub-segundo FABRICADO POR EL, encima del residuo real de la sincronizacion, y el
  // umbral que publica se movia con el instante en que uno decidia inyectar. Se vio
  // porque el barrido daba un solape de 950 ms justo en la frontera: demasiado
  // redondo para un tiempo de aire de 50 ms.
  if (!strcmp(que, "desviar_rtc")) {
    long s = (long)g_rtcSegundosDelDiaBase + arg;
    while (s < 0) s += 86400L;
    g_rtcSegundosDelDiaBase = (uint32_t)(s % 86400L);
    g_rtcBaseSegundos = (uint32_t)((long)g_rtcBaseSegundos + arg);
    return 1;
  }
  if (!strcmp(que, "reloj_en_hora"))      return reloj_enHora() ? 1 : 0;
  if (!strcmp(que, "segundos_del_dia"))   return (long)reloj_segundosDelDia();
  if (!strcmp(que, "fijar_enero"))        return (long)g_fijarEnero;

  // --- SFTY-23: el intercambio horario REAL, encolado por el coordinador ----
  if (!strcmp(que, "sincronizar_hora"))   return coordinador_sincronizarHora() ? 1 : 0;
  if (!strcmp(que, "medir_desfase"))      return coordinador_medirDesfase() ? 1 : 0;
  if (!strcmp(que, "publicar_config"))    { modo_degradado_publicarConfig(); return 1; }
  if (!strcmp(que, "config_confirmada"))  return coordinador_configConfirmada() ? 1 : 0;
  if (!strcmp(que, "desfase_valido"))     return coordinador_desfaseValido() ? 1 : 0;
  if (!strcmp(que, "desfase"))            return (long)coordinador_desfaseEsclavo();
  if (!strcmp(que, "ms_desde_sync"))      return (long)coordinador_msDesdeUltimaSync();
  if (!strcmp(que, "listo_para_contar"))  return coordinador_listoParaContar() ? 1 : 0;
  if (!strcmp(que, "comunicacion_perdida")) return coordinador_comunicacionPerdida() ? 1 : 0;
  if (!strcmp(que, "forzar_rojo_total"))  { coordinador_forzarRojoTotal(); return 1; }

  // --- La puerta y el modo -------------------------------------------------
  // "deg_evaluar" devuelve el MotivoDegradado real: 0 = MDG_OK. No se toca nada.
  if (!strcmp(que, "deg_evaluar"))        return (long)modo_degradado_evaluarEntrada();
  // Se cambia de modo por la MISMA variable que escribe la pantalla y el mando. El
  // modo_degradado_setup() lo dispara pasoPrincipal() en la siguiente vuelta, por el
  // camino de main.cpp, con su borrado de indicador incluido. No hay puerta trasera.
  if (!strcmp(que, "set_modo"))           { modoActual_set((ModoSistema)arg); return 1; }
  if (!strcmp(que, "modo_actual"))        return (long)modoActual_get();
  if (!strcmp(que, "deg_pedir_salida"))   return modo_degradado_pedirSalida() ? 1 : 0;

  // --- Lo que sobrevive al corte -------------------------------------------
  if (!strcmp(que, "respaldo_valido"))    return respaldo_valido() ? 1 : 0;
  if (!strcmp(que, "respaldo_degradado")) return respaldo_degradadoActivo() ? 1 : 0;
  if (!strcmp(que, "respaldo_hay_ciclo")) return respaldo_hayCiclo() ? 1 : 0;
  if (!strcmp(que, "respaldo_verde"))     return (long)respaldo_verdeSeg();
  if (!strcmp(que, "respaldo_despeje"))   return (long)respaldo_despejeSeg();
  if (!strcmp(que, "respaldo_horas_sync"))
    return (long)respaldo_horasDesdeSync(reloj_contadorSegundos());

  // --- Observacion ---------------------------------------------------------
  if (!strcmp(que, "toques"))
    return (arg >= 0 && arg < 64) ? (long)arnes_toques[arg] : -1;
  if (!strcmp(que, "senal_en_curso"))     return semaforo_senalEnCurso() ? 1 : 0;
  if (!strcmp(que, "tramas_emitidas"))    return (long)g_tramasEmitidas;
  if (!strcmp(que, "alarmas"))            return (long)g_alarmasEmitidas;
  if (!strcmp(que, "redibujos"))          return (long)g_lcdRedibujos;
  if (!strcmp(que, "lcd_rechazos"))       return (long)g_lcdRechazos;
  if (!strcmp(que, "lcd_ambar"))          return (long)g_lcdAmbar;
  return PUNTA_DESCONOCIDO;
}

// --- El dominio de respaldo: lo que la pila mantiene a traves de un corte -------
// [bloque literal de adaptador_esclavo.cpp: las dos puntas tienen el mismo dominio y
//  el mismo respaldo.cpp de Horner detras]
PUNTA_API long punta_dominio_leer(int indice) {
  volatile uint32_t* dr = &arnes_bkp.DR1;
  if (indice >= 0 && indice < 10) return (long)dr[indice];
  switch (indice) {
    case 10: return (long)reloj_contadorSegundos();
    case 11: return g_rtcEnHora ? 1 : 0;
    case 12: return (long)reloj_segundosDelDia();
    case 13: return (long)g_rtcDia;
    default: return 0;
  }
}

PUNTA_API void punta_dominio_escribir(int indice, long valor) {
  volatile uint32_t* dr = &arnes_bkp.DR1;
  if (indice >= 0 && indice < 10) { dr[indice] = (uint32_t)valor; return; }
  switch (indice) {
    case 10: g_rtcBaseSegundos = (uint32_t)valor; g_rtcAncla = arnes_millis_valor; break;
    case 11: g_rtcEnHora = (valor != 0); break;
    case 12: g_rtcSegundosDelDiaBase = (uint32_t)valor; break;
    case 13: g_rtcDia = (uint8_t)valor; break;
    default: break;
  }
}

}  // extern "C"
