// ===== Validacion_Automatico/dos_puntas/adaptador_maestro.cpp =====
//
// LA PUNTA MAESTRO, DENTRO DE SU PROPIA DLL.
//
// Compila coordinador.cpp + semaforo.cpp + modo_automatico.cpp + mando.cpp REALES -los
// mismos cuatro ficheros que van a la tarjeta y los mismos que ya ejerce
// arnes_automatico.cpp- y les pone alrededor lo justo para que enlacen: pantalla,
// botones, reloj y radio simulados.
//
// LOS BLOQUES DE STUB VIENEN LITERALES DE arnes_automatico.cpp (regla 3.bis: reescribir
// logica ya probada para renombrar llamadas es como se cuelan los errores en un cambio
// que no debe cambiar comportamiento). Lo unico que cambia respecto de aquel es la
// RADIO: alli el otro extremo era un Esclavo simulado escrito en este mismo fichero;
// aqui no hay Esclavo simulado en absoluto -las tramas salen a una cola que lee el
// orquestador y se las entrega al Esclavo REAL de la otra DLL-.
//
// QUE NO SE COMPILA DE ESTA PUNTA, Y POR QUE
// ------------------------------------------
// Maestro/src/main.cpp y Maestro/src/modo_degradado.cpp NO entran. main.cpp despacha a
// los ocho modos y arrastra la pantalla entera (u8g2) por menu.cpp; modo_degradado.cpp
// incluye lcd.h y menu.h y arrastra lo mismo. El bucle se conduce como ya lo conduce el
// arnes de una punta -semaforo_actualizar(), el loop del modo, mando_actualizar()-, que
// es el orden literal de main.cpp para el camino del ciclo automatico.
//
// La consecuencia se dice sin adornos: en este arnes el MODO DEGRADADO del Maestro no
// existe, asi que "las dos puntas en degradado a la vez con configuraciones distintas"
// no se puede montar por ese lado. Lo que si se monta -y es donde vive el peligro real-
// es el Esclavo en Degradado por reloj mientras el Maestro sigue ordenando por radio:
// dos autoridades sobre la misma luz, cada una con su configuracion.

#include "punta_api.h"

#include <stdio.h>
#include <string.h>

#include "Arduino.h"
#include "pines.h"
#include "botones.h"
#include "lcd.h"
#include "menu.h"

#include "coordinador.h"
#include "semaforo.h"
#include "modo_automatico.h"
#include "protocolo.h"
#include "reloj.h"
#include "respaldo.h"
#include "mando.h"
#include "modo_ambar.h"
#include "modo_degradado.h"

// ---------------------------------------------------------------------------
// EL RELOJ SIMULADO Y LOS PINES OBSERVADOS. Los declara extern Arduino.h; existen
// aqui porque esta DLL es quien los posee. La otra DLL tiene los suyos.
// ---------------------------------------------------------------------------
unsigned long arnes_millis_valor = 0;
int arnes_pines[64];
int arnes_entradas[64];
unsigned long arnes_escrituras = 0;
unsigned long arnes_toques[64];

// El contrato de bytes de la radio. Si RF_Packet dejara de medir 4, el orquestador
// estaria moviendo tramas truncadas entre las dos puntas y nadie lo notaria.
static_assert(sizeof(RF_Packet) == 4, "RF_Packet dejo de medir 4 bytes");

// ---------------------------------------------------------------------------
// BOTONES SIMULADOS. Cada bool se consume solo (como el real: leerlo lo gasta).
// [bloque literal de arnes_automatico.cpp]
// ---------------------------------------------------------------------------
static bool g_pulsarArriba = false, g_pulsarAbajo = false;
static bool g_pulsarAceptar = false, g_pulsarCancelar = false;

void botones_setup() {}
void botones_actualizar() {}
bool botonArriba()   { bool v = g_pulsarArriba;   g_pulsarArriba = false;   return v; }
bool botonAbajo()    { bool v = g_pulsarAbajo;    g_pulsarAbajo = false;    return v; }
bool botonAceptar()  { bool v = g_pulsarAceptar;  g_pulsarAceptar = false;  return v; }
bool botonCancelar() { bool v = g_pulsarCancelar; g_pulsarCancelar = false; return v; }

// N-73: la Caja Negra. El stub no puede limitarse a callar: si solo devolviera vacio,
// el arnes enlazaria y nadie sabria si la alarma se emite o no.
static char g_ultimaAlarmaEvento[48] = "";
static int  g_alarmasEmitidas = 0;
void bluetooth_reportarAlarma(const char* evento, const char* causa, const char* accion) {
  (void)causa; (void)accion;
  snprintf(g_ultimaAlarmaEvento, sizeof(g_ultimaAlarmaEvento), "%s", evento);
  g_alarmasEmitidas++;
}
void bluetooth_reportarEvento(const char*, const char*) {}

// ---------------------------------------------------------------------------
// PANTALLA SIMULADA. Solo cuenta llamadas: este arnes mide el CICLO, no el dibujo.
// ---------------------------------------------------------------------------
static unsigned long g_lcdRedibujos = 0;
void lcd_dibujarAutomatico(const char*, int, int) { g_lcdRedibujos++; }
void lcd_dibujarConfigValor(const char*, int, const char*) { g_lcdRedibujos++; }
void menu_setup() {}

// modoActual_get()/set() DE VERDAD: mando.cpp los necesita para decidir si "ya
// estabamos aqui". Arranca en MENU, igual que el enum real.
static ModoSistema g_modoActual = MENU;
ModoSistema modoActual_get() { return g_modoActual; }
void modoActual_set(ModoSistema m) { g_modoActual = m; }

// MODO_AMBAR / MODO_DEGRADADO del Maestro: sus .cpp no se compilan (ver cabecera).
// Se stubean SOLO las funciones que mando.cpp llama de verdad.
static MotivoDegradado g_entradaDegradado = MDG_OK;
static unsigned long g_modoAmbarSetups = 0;
static unsigned long g_modoDegradadoSetups = 0;
void modo_ambar_setup() { g_modoAmbarSetups++; }
void modo_ambar_fijarMotivo(const char*, const char*) {}
MotivoDegradado modo_degradado_evaluarEntrada() { return g_entradaDegradado; }
void modo_degradado_setup() { g_modoDegradadoSetups++; }

// ---------------------------------------------------------------------------
// RELOJ SIMULADO. Con reloj_enHora() en false, atenderSincronizacion() del
// coordinador no encola nada, asi que estas funciones no afectan al ciclo: existen
// porque coordinador.cpp las referencia y el enlazador las exige.
// ---------------------------------------------------------------------------
static bool g_enHora = false;
bool reloj_enHora() { return g_enHora; }
uint8_t reloj_hora() { return 0; }
uint8_t reloj_minuto() { return 0; }
uint8_t reloj_segundo() { return (uint8_t)((arnes_millis_valor / 1000UL) % 60UL); }
uint8_t reloj_dia() { return 1; }
uint32_t reloj_segundosDelDia() { return (uint32_t)(arnes_millis_valor / 1000UL); }
uint32_t reloj_contadorSegundos() { return (uint32_t)(arnes_millis_valor / 1000UL); }
void reloj_fijarEnero() {}
void respaldo_marcarSync(uint32_t) {}

// ---------------------------------------------------------------------------
// LA RADIO: DOS COLAS Y NADA MAS.
//
// Aqui NO hay Esclavo simulado. protocolo_enviarPaquete() deja la trama en la cola de
// salida y el orquestador la recoge; protocolo_hayPaqueteDisponible() saca de la cola
// de entrada lo que el orquestador metio, que viene del Esclavo REAL.
//
// LO QUE ESTO NO EJERCE, ESCRITO PARA QUE NADIE LO CUENTE COMO CUBIERTO:
// protocolo.cpp no se compila, asi que el CRC, la rafaga de RF_BURST_COPIES copias y la
// proteccion de replay no se ejercen en este camino. Los cubre costura_01 y el arnes
// del puente. Aqui se mide QUIEN ENCIENDE UN VERDE, no como viaja el byte.
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
// UN TICK DE main.cpp, EN EL ORDEN REAL.
// [bloque literal de arnes_automatico.cpp::pasoPrincipal()]
//
// mando.cpp se alimenta desde botones_actualizar() (ANTES de despachar al modo) y se
// resuelve desde mando_actualizar() (AL FINAL de loop()). El rele del mando esta
// cableado EN PARALELO con Boton1/Boton2, asi que un pulso real dispara los dos
// caminos a la vez.
// ---------------------------------------------------------------------------
static bool g_pendA = false, g_pendB = false;

static void pasoPrincipal() {
  if (g_pendA) { mando_registrarPulso(MANDO_A); g_pulsarArriba = true; }
  if (g_pendB) { mando_registrarPulso(MANDO_B); g_pulsarAbajo = true; }
  g_pendA = g_pendB = false;

  semaforo_actualizar();
  if (modoActual_get() == MODO_AUTOMATICO) {
    modoAutomatico_loop();
  }
  mando_actualizar();
}

// ---------------------------------------------------------------------------
// LA API QUE VE EL ORQUESTADOR
// ---------------------------------------------------------------------------
extern "C" {

PUNTA_API const char* punta_nombre(void) { return "MAESTRO"; }

// Arranque. El Maestro no tiene un setup() aislable -el suyo arrastra la pantalla-,
// asi que arranca por el mismo camino que ya usa el arnes de una punta:
// semaforo_setup() deja el cruce en rojo, y el asistente del modo Automatico se
// atraviesa confirmando tres veces, que es exactamente lo que hace un operario.
PUNTA_API void punta_arrancar(void) {
  for (int i = 0; i < 64; i++) { arnes_pines[i] = LOW; arnes_entradas[i] = LOW; }
  arnes_escrituras = 0;
  for (int i = 0; i < 64; i++) arnes_toques[i] = 0;
  semaforo_setup();
  protocolo_setup();
  semaforo_forzarRojo();
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
    case 1: g_pendA = true; break;
    case 2: g_pendB = true; break;
    case 3: g_pulsarAceptar = true; break;
    case 4: g_pulsarCancelar = true; break;
    default: break;
  }
}

PUNTA_API long punta_mando(const char* que, long arg) {
  // Arranca un modo Automatico limpio: CONFIG_ROJO -> CONFIG_VERDE -> CONFIG_ESTATICO
  // -> CORRIENDO, aceptando lo configurado. Es la puerta de entrada real al ciclo.
  if (!strcmp(que, "arrancar_automatico")) {
    coordinador_setup();
    mando_setup();
    g_modoActual = MODO_AUTOMATICO;
    g_pulsarArriba = g_pulsarAbajo = g_pulsarAceptar = g_pulsarCancelar = false;
    modoAutomatico_setup();
    for (int i = 0; i < 3; i++) { g_pulsarAceptar = true; modoAutomatico_loop(); }
    return modoAutomatico_enMarcha() ? 1 : 0;
  }
  // verde/rojo en minutos y despeje en segundos, empaquetados: v*10000 + r*100 + d.
  // Los rechaza el FIRMWARE si estan fuera de rango, que es donde tiene que decidirse.
  if (!strcmp(que, "fijar_tiempos")) {
    uint8_t v = (uint8_t)((arg / 10000) % 100);
    uint8_t r = (uint8_t)((arg / 100) % 100);
    uint8_t d = (uint8_t)(arg % 100);
    return modoAutomatico_fijarTiempos(v, r, d) ? 1 : 0;
  }
  if (!strcmp(que, "en_marcha"))          return modoAutomatico_enMarcha() ? 1 : 0;
  if (!strcmp(que, "listo_para_contar"))  return coordinador_listoParaContar() ? 1 : 0;
  if (!strcmp(que, "comunicacion_perdida")) return coordinador_comunicacionPerdida() ? 1 : 0;
  if (!strcmp(que, "toques"))
    return (arg >= 0 && arg < 64) ? (long)arnes_toques[arg] : -1;
  if (!strcmp(que, "senal_en_curso"))     return semaforo_senalEnCurso() ? 1 : 0;
  if (!strcmp(que, "tramas_emitidas"))    return (long)g_tramasEmitidas;
  if (!strcmp(que, "alarmas"))            return (long)g_alarmasEmitidas;
  if (!strcmp(que, "redibujos"))          return (long)g_lcdRedibujos;
  if (!strcmp(que, "reloj_en_hora"))      { g_enHora = (arg != 0); return 1; }
  if (!strcmp(que, "forzar_rojo_total"))  { coordinador_forzarRojoTotal(); return 1; }
  if (!strcmp(que, "modo_actual"))        return (long)modoActual_get();
  return PUNTA_DESCONOCIDO;
}

// El dominio de respaldo del Maestro NO se modela en este arnes: su respaldo.cpp no se
// compila aqui -solo se stubea respaldo_marcarSync(), lo unico que coordinador.cpp
// llama-. Se exporta igual para que las dos DLL cumplan el MISMO contrato: una API que
// falta se descubre con un GetProcAddress nulo en mitad de un escenario, y eso seria un
// ABORTADO tardio. Devolver siempre 0 es honesto: aqui no sobrevive nada porque aqui no
// se guarda nada.
PUNTA_API long punta_dominio_leer(int) { return 0; }
PUNTA_API void punta_dominio_escribir(int, long) {}

}  // extern "C"
