// ===== Validacion_Automatico/dos_puntas/adaptador_esclavo.cpp =====
//
// LA PUNTA ESCLAVO, DENTRO DE SU PROPIA DLL.
//
// Compila SIETE ficheros REALES del Esclavo, y el que importa mas es el que ningun
// arnes habia compilado nunca: src/main.cpp.
//
//   semaforo.cpp        las luces y el enclavamiento SFTY-2 de esta punta
//   main.cpp            EL DESPACHADOR DE RADIO. Aqui vive lo que decide si esta
//                       punta obedece un CMD_GO_GREEN, y las dos guardas -mando
//                       local y ambar de Bluetooth- que lo pueden vetar
//   modo_degradado.cpp  la OTRA autoridad que puede encender un verde aqui, sin que
//                       nadie lo ordene por radio
//   config_ciclo.cpp    el par verde+despeje que llega del Maestro
//   mando.cpp           el mando de reles: senalActiva, el ambar local
//   demanda.cpp         la puerta unica de la demanda vehicular
//   respaldo.cpp        el dominio de respaldo REAL, con su calcularSuma() de Horner
//
// POR QUE main.cpp Y NO UN DRIVER ESCRITO AQUI. Porque el despachador ES lo que se
// esta midiendo. Un arnes que reimplementara "si llega CMD_GO_GREEN, llama a
// semaforo_iniciarTransicionAVerde()" seria la segunda copia del firmware escrita a
// mano que este repositorio persigue: mediria el modelo, no el codigo. Y las dos
// guardas de N-83 -las que revocan el ambar de la app- viven EXACTAMENTE ahi.
//
// LO QUE SE SUSTITUYE, Y ES TODO LO QUE NO DECIDE UNA LUZ: pantalla (lcd, menu),
// botones, Bluetooth, el RTC y la radio. Ninguno de esos cinco puede encender un
// verde; los tres que si pueden -radio, Degradado, mando- se compilan de verdad.
//
// EL RTC ES EL UNICO MODELO ESCRITO A MANO QUE QUEDA AQUI, y se dice en voz alta:
// reloj.cpp incluye <STM32RTC.h> y no hay sustituto de esa libreria en el repositorio.
// Lo que se modela es un PERIFERICO -un contador de segundos y cuatro getters-, no una
// regla del firmware; el Modo Degradado, que es quien lo consume, se compila entero.

#include "punta_api.h"

#include <stdio.h>
#include <string.h>

#include "Arduino.h"
#include "pines.h"
#include "botones.h"
#include "lcd.h"
#include "menu.h"
#include "IWatchdog.h"

#include "semaforo.h"
#include "protocolo.h"
#include "reloj.h"
#include "respaldo.h"
#include "mando.h"
#include "config_ciclo.h"
#include "modo_degradado.h"
#include "bluetooth.h"
#include "demanda.h"
#include "stm32f1xx_hal.h"   // para volcar el dominio de respaldo real

// setup() y loop() son de main.cpp, que se compila en esta misma DLL.
void setup();
void loop();

// ---------------------------------------------------------------------------
// EL RELOJ SIMULADO Y LOS PINES OBSERVADOS. Esta DLL tiene los SUYOS: el Maestro
// escribe en otro array de 64 enteros, en otro modulo. Ese es el mecanismo entero.
// ---------------------------------------------------------------------------
unsigned long arnes_millis_valor = 0;
int arnes_pines[64];
int arnes_entradas[64];
unsigned long arnes_escrituras = 0;
unsigned long arnes_toques[64];

// Los diez registros del dominio VBAT que respaldo.cpp recorre de verdad.
BKP_Simulado arnes_bkp;

IWatchdogClase IWatchdog;

static_assert(sizeof(RF_Packet) == 4, "RF_Packet dejo de medir 4 bytes");

// ---------------------------------------------------------------------------
// BOTONES SIMULADOS. Mismo contrato que en el arnes de una punta: leerlos los gasta.
// El rele del mando esta cableado EN PARALELO con Boton1/Boton2, asi que un pulso
// real dispara los dos caminos -mando_registrarPulso() y el flag del boton-.
// ---------------------------------------------------------------------------
static bool g_pulsarArriba = false, g_pulsarAbajo = false;
static bool g_pulsarAceptar = false, g_pulsarCancelar = false;
static bool g_pendA = false, g_pendB = false;

void botones_setup() {}
void botones_actualizar() {
  if (g_pendA) { mando_registrarPulso(MANDO_A); g_pulsarArriba = true; }
  if (g_pendB) { mando_registrarPulso(MANDO_B); g_pulsarAbajo = true; }
  g_pendA = g_pendB = false;
}
bool botonArriba()   { bool v = g_pulsarArriba;   g_pulsarArriba = false;   return v; }
bool botonAbajo()    { bool v = g_pulsarAbajo;    g_pulsarAbajo = false;    return v; }
bool botonAceptar()  { bool v = g_pulsarAceptar;  g_pulsarAceptar = false;  return v; }
bool botonCancelar() { bool v = g_pulsarCancelar; g_pulsarCancelar = false; return v; }
bool camara_leerPin(uint8_t pin) { return digitalRead(pin) == HIGH; }

// ---------------------------------------------------------------------------
// PANTALLA SIMULADA. Solo cuenta llamadas.
//
// menu_estaAbierto() NO esta cableado a false: es la puerta que INHIBE las secuencias
// del mando (SFTY-21), y dejarla siempre cerrada seria no ejercer nunca esa rama.
// ---------------------------------------------------------------------------
static unsigned long g_lcdRedibujos = 0;
static bool g_menuAbierto = false;
void lcd_setup() {}
void lcd_dibujarBienvenida() { g_lcdRedibujos++; }
void menu_setup() {}
void menu_loop() { g_lcdRedibujos++; }
bool menu_estaAbierto() { return g_menuAbierto; }

// ---------------------------------------------------------------------------
// BLUETOOTH SIMULADO. bluetooth.cpp no se compila -y el 31/08 esta ademas en manos de
// otro agente-, pero bluetooth_ambarEmergencia() NO puede ser un false fijo: es una de
// las DOS guardas que vetan un CMD_GO_GREEN en el despachador real (N-83). El
// orquestador la mueve, de modo que la rama vetada se recorre.
// ---------------------------------------------------------------------------
static bool g_ambarEmergencia = false;
static char g_ultimaAlarmaEvento[48] = "";
static char g_ultimaAlarmaCausa[48] = "";
static int  g_alarmasEmitidas = 0;

void bluetooth_setup() {}
void bluetooth_loop() {}
bool bluetooth_ambarEmergencia() { return g_ambarEmergencia; }
bool bluetooth_testLedsActivo() { return false; }
void bluetooth_reportarAlarma(const char* evento, const char* causa, const char* accion) {
  (void)accion;
  snprintf(g_ultimaAlarmaEvento, sizeof(g_ultimaAlarmaEvento), "%s", evento);
  snprintf(g_ultimaAlarmaCausa, sizeof(g_ultimaAlarmaCausa), "%s", causa);
  g_alarmasEmitidas++;
}
void bluetooth_reportarEvento(const char*, const char*) {}

// ---------------------------------------------------------------------------
// EL RTC SIMULADO. Ver la cabecera: es un modelo de PERIFERICO, no de firmware.
//
// El contador de segundos es MONOTONO y sobrevive al corte -lo mantiene la pila-, que
// es la propiedad de la que cuelga todo el fechado de N-49. Avanza con el reloj
// simulado del arnes: reloj_contadorSegundos() = base + millis()/1000.
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
  // El contador crudo NO se reinicia al poner en hora: en el silicio es el mismo
  // registro que sigue corriendo. Solo se ancla la hora de pared.
  uint32_t contador = reloj_contadorSegundos();
  g_rtcAncla = arnes_millis_valor;
  g_rtcBaseSegundos = contador ? contador : 1;
  g_rtcSegundosDelDiaBase = (uint32_t)hora * 3600UL + (uint32_t)minuto * 60UL + segundo;
  if (dia >= 1) g_rtcDia = dia;
  g_rtcEnHora = true;
}

// ---------------------------------------------------------------------------
// LA RADIO: DOS COLAS. Identica a la del Maestro; el canal lo lleva el orquestador.
// protocolo.cpp no se compila, asi que CRC, rafaga y proteccion de replay no se
// ejercen por este camino. Se mide QUIEN ENCIENDE UN VERDE.
// ---------------------------------------------------------------------------
#define COLA_MAX 32
struct Cola {
  RF_Packet dato[COLA_MAX];
  int cabeza = 0, cola = 0;
  bool meter(const RF_Packet& p) {
    int sig = (cola + 1) % COLA_MAX;
    if (sig == cabeza) return false;
    dato[cola] = p; cola = sig; return true;
  }
  bool sacar(RF_Packet* p) {
    if (cabeza == cola) return false;
    *p = dato[cabeza]; cabeza = (cabeza + 1) % COLA_MAX; return true;
  }
};
static Cola g_tx, g_rx;
static unsigned long g_tramasEmitidas = 0;
static unsigned long g_replayReseteos = 0;

void protocolo_setup() {}
void protocolo_resetReplayProtection() { g_replayReseteos++; }

void protocolo_enviarPaquete(uint8_t cmd, uint8_t param) {
  RF_Packet p; p.msgID = 0; p.command = cmd; p.param = param; p.crc = 0;
  g_tramasEmitidas++;
  g_tx.meter(p);
}

bool protocolo_hayPaqueteDisponible(RF_Packet* destino) {
  return g_rx.sacar(destino);
}

// ---------------------------------------------------------------------------
// LA API QUE VE EL ORQUESTADOR
// ---------------------------------------------------------------------------
extern "C" {

PUNTA_API const char* punta_nombre(void) { return "ESCLAVO"; }

// EL setup() REAL DEL FIRMWARE. No una version recortada: el mismo que corre en la
// tarjeta, con su orden -luces primero, pantalla, watchdog, RTC, respaldo, mando,
// bluetooth y degradado_reanudarTrasCorte() al final-. Esa ultima llamada es la que
// decide si tras un corte esta punta REANUDA el Modo Degradado o cae a ambar, y es
// justo lo que hay que ejercer en el escenario de microcorte.
PUNTA_API void punta_arrancar(void) {
  for (int i = 0; i < 64; i++) { arnes_pines[i] = LOW; arnes_entradas[i] = LOW; }
  arnes_escrituras = 0;
  for (int i = 0; i < 64; i++) arnes_toques[i] = 0;
  setup();
}

PUNTA_API void punta_tick(unsigned long ms) {
  arnes_millis_valor = ms;
  loop();
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
  if (!strcmp(que, "degradado_gobierna"))  return degradado_gobiernaLuz() ? 1 : 0;
  if (!strcmp(que, "degradado_estado"))    return (long)degradado_estado();
  if (!strcmp(que, "degradado_fase"))      return (long)degradado_fase();
  if (!strcmp(que, "degradado_comprobar")) return (long)degradado_comprobar();
  if (!strcmp(que, "degradado_entrar"))    return (long)degradado_entrar();
  if (!strcmp(que, "degradado_salir"))     { degradado_salir(); return 1; }
  if (!strcmp(que, "degradado_hubo_sync")) return degradado_huboSync() ? 1 : 0;
  if (!strcmp(que, "config_verde"))        return (long)config_verdeSegundos();
  if (!strcmp(que, "config_despeje"))      return (long)config_despejeSegundos();
  if (!strcmp(que, "toques"))
    return (arg >= 0 && arg < 64) ? (long)arnes_toques[arg] : -1;
  if (!strcmp(que, "senal_en_curso"))      return semaforo_senalEnCurso() ? 1 : 0;
  if (!strcmp(que, "ambar_local"))         return mando_ambarLocal() ? 1 : 0;
  if (!strcmp(que, "tramas_emitidas"))     return (long)g_tramasEmitidas;
  if (!strcmp(que, "alarmas"))             return (long)g_alarmasEmitidas;
  if (!strcmp(que, "recargas_watchdog"))   return (long)IWatchdog.recargas;
  if (!strcmp(que, "replay_reseteos"))     return (long)g_replayReseteos;
  if (!strcmp(que, "ambar_bluetooth"))     { g_ambarEmergencia = (arg != 0); return 1; }
  if (!strcmp(que, "menu_abierto"))        { g_menuAbierto = (arg != 0); return 1; }
  if (!strcmp(que, "respaldo_valido"))     return respaldo_valido() ? 1 : 0;
  if (!strcmp(que, "respaldo_degradado"))  return respaldo_degradadoActivo() ? 1 : 0;
  return PUNTA_DESCONOCIDO;
}

// --- El dominio de respaldo: lo que la pila mantiene a traves de un corte -------
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
