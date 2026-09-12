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
// botones, el RTC y la radio. Ninguno de esos cuatro puede encender un verde; los tres
// que si pueden -radio, Degradado, mando- se compilan de verdad.
//
// 11/09 (N-142, §3.16-A): Y EL BLUETOOTH DEJA DE ESTAR EN ESA LISTA. bluetooth.cpp del
// Esclavo se compila en esta DLL -por #include, ver el bloque de mas abajo- porque el
// ambar de emergencia de la app vive ahi, tiene DOS puertas y solo una avisaba al
// Maestro. Mientras el arnes lo doblaba con una transcripcion, ese defecto era invisible
// aqui. Sigue doblado en la variante -DARNES_RELOJ_REAL (arnes del Degradado), que mide
// la FASE y no el despachador del telefono.
//
// EL RTC ES EL UNICO MODELO ESCRITO A MANO QUE QUEDA AQUI, y se dice en voz alta:
// reloj.cpp incluye <STM32RTC.h> y no hay sustituto de esa libreria en el repositorio.
// Lo que se modela es un PERIFERICO -un contador de segundos y cuatro getters-, no una
// regla del firmware; el Modo Degradado, que es quien lo consume, se compila entero.
//
// 11/09 (D-21 (1)) - DOS VARIANTES DE ESTE MISMO FICHERO, Y NINGUNA COPIA. Con
// -DARNES_RELOJ_REAL -lo pone SOLO compilar_degradado.ps1- el modelo de abajo NO se compila
// y entra el reloj.cpp REAL del Esclavo, con el silicio sustituido en reloj_real/ (hay
// STM32RTC.h ahi desde hoy). Asi la caducidad de la siembra y la frontera de 25 s de
// reloj_radioManda() las ejecuta el bloque F del orquestador del Degradado, en vez de leerse
// por regex. SIN el define -compilar_dos_puntas.ps1- entra el modelo de mas abajo.
//
// 12/09 (N-162, roadmap 1.16(c)) - AQUEL PUNTO CIEGO ESTABA DECLARADO Y HOY SE CIERRA.
// Decia: "el bloque D de orquestador.cpp mide la reanudacion sobre un RTC que el firmware
// de hoy ya no escribe", y era cierto -medido: el Esclavo del arnes despertaba con
// gobierna=1 despues del microcorte-. El modelo guardaba la hora en el RTC al ponerla; el
// reloj.cpp real dejo de hacerlo el 11/09. Lo que lo arregla NO es dejar de reponer el
// dominio -eso mataria el escenario- sino PARTIR LA BANDERA: la hora sembrada vive en RAM
// y el corte se la lleva, el marcador del RTC hardware vive en la pila y ningun firmware
// lo escribe. Ver el bloque de las banderas, y D6b/D6c/D6d y D8/D9 del orquestador.

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
#ifdef ARNES_RELOJ_REAL
#include "rtc_periferico.h"  // D-21 (1): el HSI, la linea del ESP32 y la siembra en frontera
#endif

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

#ifdef ARNES_RELOJ_REAL
// ---------------------------------------------------------------------------
// BLUETOOTH SIMULADO (arnes del Degradado). bluetooth.cpp no se compila aqui -arrastra
// el puerto serie entero y lo que este arnes mide es la FASE, no el despachador del
// telefono-, pero bluetooth_ambarEmergencia() NO puede ser un false fijo: es una de las
// DOS guardas que vetan un CMD_GO_GREEN en el despachador real (N-83). El orquestador la
// mueve, de modo que la rama vetada se recorre.
// ---------------------------------------------------------------------------
static bool g_ambarEmergencia = false;
static char g_ultimaAlarmaEvento[48] = "";
static char g_ultimaAlarmaCausa[48] = "";
static int  g_alarmasEmitidas = 0;
// D-21 (1): la alarma de la hora caducada, por su causa; y el $EVENT del salto que pasa por
// rojo (D-26 (4)), por su detalle. Ver el mismo par en adaptador_maestro_deg.cpp.
static int  g_alarmasCaducada = 0;
static int  g_eventosSaltoRojo = 0;

void bluetooth_setup() {}
void bluetooth_loop() {}
bool bluetooth_ambarEmergencia() { return g_ambarEmergencia; }
bool bluetooth_testLedsActivo() { return false; }
void bluetooth_reportarAlarma(const char* evento, const char* causa, const char* accion) {
  (void)accion;
  snprintf(g_ultimaAlarmaEvento, sizeof(g_ultimaAlarmaEvento), "%s", evento);
  snprintf(g_ultimaAlarmaCausa, sizeof(g_ultimaAlarmaCausa), "%s", causa);
  g_alarmasEmitidas++;
  if (!strcmp(evento, "HORA_ESP32") && !strcmp(causa, "CADUCADA")) g_alarmasCaducada++;
}
void bluetooth_reportarEvento(const char* origen, const char* detalle) {
  if (!strcmp(origen, "DEGRADADO") && !strcmp(detalle, "SALTO_DE_HORA_POR_ROJO")) {
    g_eventosSaltoRojo++;
  }
}

#else   // !ARNES_RELOJ_REAL
// ---------------------------------------------------------------------------
// 🔴 EL BLUETOOTH ES EL REAL (11/09, N-142 / §3.16-A). bluetooth.cpp del Esclavo SE
// COMPILA EN ESTA DLL, y el motivo es un defecto que este arnes no podia ver.
//
// EL DEFECTO: el ambar de emergencia de la app tiene DOS puertas en ese fichero -sin PIN
// contra 'cmd' y con PIN contra 'accion'- y solo la de CON PIN avisaba al Maestro
// (CMD_AMBAR_ESCLAVO). La app usa la de SIN PIN. Aqui no se veia porque el ambar de la
// app entraba por la orden "ambar_emergencia_app" de mas abajo, que era una
// TRANSCRIPCION de la puerta CON PIN escrita en este fichero: el arnes medía la copia
// buena de una puerta mala. Es CLAUDE.md §8 en su forma mas cara -el modelo replicando
// lo que el firmware no hace- y se corrige de la unica manera que no vuelve a pasar:
// compilando el despachador de verdad y metiendole la LINEA que manda el telefono.
//
// SE INCLUYE EL .cpp, como arnes_respaldo.cpp con respaldo.cpp: el fuente entra intacto,
// byte por byte, y lo que se sustituye es el SILICIO de alrededor -el puerto serie- mas
// los cuatro modulos que esta DLL no compila (protocolo, identidad, botones y el RTC).
//
// LO QUE ESTO ANADE AL ARNES, Y NO ES POCO: el $ACK que el telefono recibe pasa a ser
// observable. Hasta hoy ninguna prueba de este repositorio leia lo que el equipo CONTESTA
// mientras la luz se movia; se leia por texto en los packs. Ver las ordenes "bt:" y "ack:".
//
// LO QUE NO ENTRA: protocolo.cpp (CRC, rafaga y replay), identidad.cpp y botones.cpp. Los
// tres se doblan aqui abajo y ninguno decide una luz. Y el reloj sigue siendo el modelo de
// periferico de siempre; la rama CMD:HORA_ESP32 del despachador se puede ejercer, pero
// quien mide la hora es el arnes del Degradado, con reloj.cpp real.
// ---------------------------------------------------------------------------

// Los tres pines que bluetooth.cpp nombra y que el sustituto comun de pines.h no tiene.
// NUMEROS FUERA DE LOS QUE OBSERVA EL ORQUESTADOR (0..12): si RS485_IN_DE_RE cayera
// encima de una luz, el HIGH de bluetooth_setup() encenderia una lampara en el arnes y
// el fallo se leeria como un defecto del firmware.
#ifndef PB7
#define PB7 14
#endif
#ifndef PB6
#define PB6 15
#endif
#ifndef RS485_IN_DE_RE
#define RS485_IN_DE_RE 13
#endif

// EL CABLE DEL TELEFONO. Entrada: lo que el orquestador teclea en la app. Salida: la
// cinta de tramas que el equipo emite, que es lo que la app leeria.
//
// Se guarda LA ULTIMA linea de acuse ($ACK o $ERR) y se cuentan las de alarma. Guardar
// solo la ultima no es pereza: cada orden del arnes se despacha en UN tick y se pregunta
// en el mismo, igual que el telefono, que tampoco tiene historial.
static char g_btIn[512];
static int  g_btInCab = 0, g_btInCola = 0;
static char g_btUltimoAcuse[192] = "";
static char g_btLinea[192];
static int  g_btLineaIdx = 0;
static unsigned long g_btAlarmas = 0, g_btAcuses = 0, g_btEventos = 0;

static void btSalidaCaracter(char c) {
  if (c == '\r' || c == '\n') {
    if (g_btLineaIdx > 0) {
      g_btLinea[g_btLineaIdx] = '\0';
      if (!strncmp(g_btLinea, "$ACK", 4) || !strncmp(g_btLinea, "$ERR", 4)) {
        snprintf(g_btUltimoAcuse, sizeof(g_btUltimoAcuse), "%s", g_btLinea);
        g_btAcuses++;
      } else if (!strncmp(g_btLinea, "$ALARM", 6)) {
        g_btAlarmas++;
      } else if (!strncmp(g_btLinea, "$EVENT", 6)) {
        g_btEventos++;
      }
      g_btLineaIdx = 0;
    }
    return;
  }
  if (g_btLineaIdx < (int)sizeof(g_btLinea) - 1) g_btLinea[g_btLineaIdx++] = c;
}

class HardwareSerial {
 public:
  HardwareSerial(int, int) {}
  void begin(unsigned long) {}
  int available() { return (g_btInCab != g_btInCola) ? 1 : 0; }
  int read() {
    if (g_btInCab == g_btInCola) return -1;
    char c = g_btIn[g_btInCab];
    g_btInCab = (g_btInCab + 1) % (int)sizeof(g_btIn);
    return (int)(unsigned char)c;
  }
  void print(const char* s) { while (*s) btSalidaCaracter(*s++); }
};

// Una linea del telefono, entregada como la entrega el puente: con su salto de linea al
// final. La despacha bluetooth_loop() REAL en el siguiente tick de esta punta.
static bool btTeclear(const char* linea) {
  const char* p = linea;
  for (;; p++) {
    int sig = (g_btInCola + 1) % (int)sizeof(g_btIn);
    if (sig == g_btInCab) return false;   // cola llena: no se finge que entro
    g_btIn[g_btInCola] = *p ? *p : '\n';
    g_btInCola = sig;
    if (!*p) return true;
  }
}

// --- Los cuatro modulos que esta DLL no compila y bluetooth.cpp si llama -----
//
// protocolo.cpp: sus tres contadores de SFTY-15. tramasValidas() NO es un cero fijo, y
// eso es lo unico que importa aqui: bluetooth_loop() lo usa para saber que la radio
// VOLVIO -y con el baja la bandera de enlace caido que decide el $ACK del ambar-. Se
// cuenta lo que la radio de este arnes entrega de verdad.
static unsigned long g_rxValidas = 0;
unsigned long protocolo_bytesRecibidos()    { return g_rxValidas * 4UL; }
unsigned long protocolo_tramasValidas()     { return g_rxValidas; }
unsigned long protocolo_tramasDescartadas() { return 0; }

// identidad.cpp: el serie que viaja en el $STATUS. Un literal, porque ninguna decision
// de luz cuelga de el.
void identidad_texto(char* dst) { snprintf(dst, 7, "%s", "ARNES1"); }

// botones.cpp: el campo CAM: del $STATUS. La vigilancia de camaras la miden camara_01 y
// camara_03; aqui solo hace falta que la trama se componga.
const char* camara_estado() { return "OK"; }

// reloj.cpp: las dos funciones de D-26 que el despachador consulta y que el modelo de
// RTC de mas abajo no tiene. La FUENTE de la hora no la ejerce este arnes -la ejerce el
// del Degradado, con reloj.cpp real-, asi que aqui se contesta lo unico que no miente:
// que la radio no manda la hora mientras nadie la haya sembrado por radio.
bool reloj_radioManda() { return false; }

// 🔴 D-29 (12/09) - Y ESTA DEJA DE SER UN "return false" FIJO, PORQUE DE ELLA CUELGA EL
// BLOQUE D ENTERO.
//
// D-29 difiere el borrado del permiso de la pila para que la reanudacion pueda decidirse
// con LA HORA QUE EL ESP32 TRAE DESPUES DEL ARRANQUE. Con esta funcion contestando
// siempre que no, esa hora no podia llegar nunca por el camino real y el bloque D no
// tendria forma de ejercer lo construido: mediria el diferimiento y no su desenlace.
//
// LO QUE MODELA, Y LO QUE NO. Modela el PERIFERICO -la hora entra y la base de software
// queda sembrada-, delegando en reloj_ajustar(), que es el mismo sembrador del modelo que
// usa la radio, con su misma regla de rango. Y NO TOCA g_rtcHwEnHora, exactamente como
// reloj_ajustarConAcuse() dejo de tocar el RTC hardware en N-162: si lo tocara, el modelo
// estaria devolviendo la capacidad que el firmware perdio y el bloque D volveria a medir
// una tarjeta que no existe.
//
// EL BORDE QUE ESTE MODELO NO EJERCE, ESCRITO AL LADO (CLAUDE.md 7): el RECHAZO POR
// FORMATO. reloj.cpp real exige el patron exacto de 19 caracteres (isoBienFormado) y aqui
// solo se pide que sscanf saque los seis campos y que esten en rango. Esa frontera la
// ejerce la otra variante de este mismo fichero -reloj.cpp REAL, bloque F del arnes del
// Degradado-, que es donde vive; duplicar aqui el patron seria una segunda copia del
// firmware escrita a mano. Lo que el bloque D necesita de aqui es que una siembra BIEN
// FORMADA entre, y que una malformada no invente una hora.
bool reloj_sembrarDesdeIso(const char* str) {
  if (str == nullptr) return false;
  int anio = 0, mes = 0, dia = 0, h = 0, m = 0, s = 0;
  if (sscanf(str, "%d-%d-%d,%d:%d:%d", &anio, &mes, &dia, &h, &m, &s) != 6) return false;
  if (h < 0 || h > 23 || m < 0 || m > 59 || s < 0 || s > 59 || dia < 1 || dia > 31) return false;
  reloj_ajustar((uint8_t)h, (uint8_t)m, (uint8_t)s, (uint8_t)dia);
  return true;
}

// EL FUENTE REAL, SIN TOCAR.
#include "../../Esclavo/src/bluetooth.cpp"   // NOLINT: deliberado, ver la cabecera
#endif  // ARNES_RELOJ_REAL

#ifdef ARNES_RELOJ_REAL
// ---------------------------------------------------------------------------
// EL RELOJ ES EL REAL (compilar_degradado.ps1). La rama CMD:HORA_ESP32 de bluetooth.cpp,
// TRANSCRITA en lo que decide -bluetooth.cpp no se compila aqui-: si la radio manda, se
// ignora; si no, se siembra. La pregunta a la radio se hace en el instante del banco, y la
// siembra, si hay que conservar la fase, en la frontera de segundo: por eso van separadas.
// Si la rama cambia, esto se queda viejo: lo compara reloj_04.
// ---------------------------------------------------------------------------
static int sembrarDirecto(const char* iso) { return reloj_sembrarDesdeIso(iso) ? 1 : 0; }

static int ramaHoraEsp32(const char* iso) {
  if (reloj_radioManda()) return 2;          // HE_IGNORADA: manda la radio
  return sembrarDirecto(iso);                // 1 sembrada, 0 rechazada
}
#else
// ---------------------------------------------------------------------------
// EL RTC SIMULADO. Ver la cabecera: es un modelo de PERIFERICO, no de firmware.
//
// El contador de segundos es MONOTONO y sobrevive al corte -lo mantiene la pila-, que
// es la propiedad de la que cuelga todo el fechado de N-49. Avanza con el reloj
// simulado del arnes: reloj_contadorSegundos() = base + millis()/1000.
//
// 🔴 12/09 (N-162, roadmap 1.16(c)) - AQUI HABIA UNA SOLA BANDERA CONTESTANDO A DOS
// PREGUNTAS DISTINTAS, Y POR ESO EL BLOQUE D MEDIA OTRA COSA (CLAUDE.md §8).
//
// g_rtcEnHora valia a la vez por "hay hora sembrada" -horaValida de reloj.cpp, que vive
// en RAM y un corte se la lleva- y por "el RTC hardware esta configurado y con ano >=
// ANIO_MARCA" -que vive en el dominio de la CR2032 y es LO UNICO que reloj_setup() mira
// al arrancar-. Como el indice 11 del dominio guardaba y reponia esa unica bandera, el
// Esclavo del arnes DESPERTABA EN HORA despues de un microcorte y reanudaba el Modo
// Degradado. MEDIDO hoy sobre el fuente real, eso ya no puede pasar:
//
//   grep -n "rtc\.set" Esclavo/src/reloj.cpp  ->  solo rtc.setClockSource()
//   Esclavo/src/reloj.cpp:283  "N-162 (11/09) - LA SIEMBRA YA NO ESCRIBE EL RTC HARDWARE"
//   Esclavo/src/reloj.cpp:186  "sin cristal el Degradado NO SE REANUDA tras un corte"
//
// o sea que reloj_ajustarConAcuse() no escribe ni la hora ni el ano, rtc.getYear() no
// llega nunca a ANIO_MARCA, y reloj_setup() deja horaValida en false despues de CADA
// corte. La misma medida esta en reloj_real/rtc_periferico.cpp -"arnes_rtc_configurado =
// false; desde N-162 nadie lo escribe"-: aquella variante ya era fiel y esta no.
//
// Son dos banderas, y desde hoy lo son:
//   g_horaSembrada  RAM. La pone reloj_ajustar() (la radio) y se la lleva el corte.
//   g_rtcHwEnHora   dominio de la pila, indice 11. NINGUNA linea del firmware la pone;
//                   solo puede ponerla el orquestador, y eso modela un equipo cuyo RTC
//                   escribio un firmware ANTERIOR al 11/09. Es el control del bloque D.
static bool     g_horaSembrada = false;
static bool     g_rtcHwEnHora = false;
static uint32_t g_rtcBaseSegundos = 0;      // valor del contador cuando millis()==g_rtcAncla
static unsigned long g_rtcAncla = 0;
static uint32_t g_rtcSegundosDelDiaBase = 0;
static uint8_t  g_rtcDia = 0;

static uint32_t rtcTranscurrido() {
  return (uint32_t)((arnes_millis_valor - g_rtcAncla) / 1000UL);
}

// N-162: el reloj_setup() real hace EXACTAMENTE esto y nada mas que importe aqui:
//   horaValida = rtc.isConfigured() && rtc.getYear() >= ANIO_MARCA && (h|m|s) != 0
// o sea que la hora que hay al arrancar es la del RTC HARDWARE, la que la pila mantuvo,
// y ninguna otra. La sembrada por radio esta en RAM y ya no existe. Sin esta linea el
// modelo despertaba en hora porque el indice 11 le devolvia su unica bandera.
void reloj_setup() { g_horaSembrada = g_rtcHwEnHora; }
void reloj_actualizar() {}
bool reloj_enHora() { return g_horaSembrada; }
// D-21 (1): modo_degradado.cpp REAL pregunta si la hora puede decidir una luz. En ESTA
// variante no hay base de tiempo que caduque y se contesta lo mismo que reloj_enHora(): la
// caducidad la ejerce la variante con el reloj.cpp real (bloque F del arnes del Degradado).
bool reloj_horaFiable() { return g_horaSembrada; }
// D-26 (3): main.cpp REAL la llama con cada trama de radio para que reloj.cpp sepa si la
// radio del Maestro llega. Aqui reloj.cpp no se compila y nadie pregunta por la fuente de
// la hora -eso lo decide la rama CMD:HORA_ESP32 de bluetooth.cpp, que tampoco se compila
// aqui-, asi que se cuenta la llamada y nada mas: lo que este arnes mide es la luz.
static unsigned long g_radioNotada = 0;
void reloj_notarRadio() { g_radioNotada++; }

// 🔴 EL BORDE QUE ESTE MODELO ELIGE, ESCRITO AL LADO PORQUE ES UNA DECISION (CLAUDE.md §7):
// AQUI EL CRISTAL CUENTA. El reloj_contadorSegundos() real devuelve 0 solo cuando NO hay
// cristal (rtcOperativo == false); con cristal devuelve CNT, que la pila mantiene y que
// sigue contando a traves del corte, y NO depende de que la hora este puesta. Colgarlo de
// la bandera de la hora -como estaba- juntaba las dos puertas de
// degradado_reanudarTrasCorte() en una sola y hacia imposible saber CUAL cerro:
//
//   sigueVigente = reloj_enHora() && respaldo_hayCiclo();          <- primera
//   horas = respaldo_horasDesdeSync(reloj_contadorSegundos());     <- segunda
//
// Con el contador a 0, la segunda tambien cierra y una inversion que solo mire el
// desenlace aprueba las barreras en cualquier orden (CLAUDE.md §9). Se modela el caso
// FAVORABLE al firmware -cristal vivo, marca fresca, la segunda puerta ABIERTA- para que
// si aun asi no reanuda, la que cerro sea la primera y se pueda medir cual es.
uint32_t reloj_contadorSegundos() {
  const uint32_t v = g_rtcBaseSegundos + rtcTranscurrido();
  return v == 0 ? 1UL : v;   // el mismo suelo que el real: 0 significa "no hay reloj"
}

uint32_t reloj_segundosDelDia() {
  if (!g_horaSembrada) return 0;
  return (g_rtcSegundosDelDiaBase + rtcTranscurrido()) % 86400UL;
}
uint8_t reloj_hora()   { return (uint8_t)(reloj_segundosDelDia() / 3600UL); }
uint8_t reloj_minuto() { return (uint8_t)((reloj_segundosDelDia() / 60UL) % 60UL); }
uint8_t reloj_segundo(){ return (uint8_t)(reloj_segundosDelDia() % 60UL); }
uint8_t reloj_dia()    { return g_horaSembrada ? g_rtcDia : 0; }

void reloj_ajustar(uint8_t hora, uint8_t minuto, uint8_t segundo, uint8_t dia) {
  if (hora > 23 || minuto > 59 || segundo > 59 || dia > 31) return;
  // El contador crudo NO se reinicia al poner en hora: en el silicio es el mismo
  // registro que sigue corriendo. Solo se ancla la hora de pared.
  uint32_t contador = reloj_contadorSegundos();
  g_rtcAncla = arnes_millis_valor;
  g_rtcBaseSegundos = contador ? contador : 1;
  g_rtcSegundosDelDiaBase = (uint32_t)hora * 3600UL + (uint32_t)minuto * 60UL + segundo;
  if (dia >= 1) g_rtcDia = dia;
  // N-162: SIEMBRA LA BASE DE SOFTWARE Y NO TOCA g_rtcHwEnHora, y esa omision es el
  // modelo. reloj_ajustarConAcuse() perdio el bloque "if (rtcOperativo) { rtc.setHours();
  // ... }" el 11/09 por tres motivos medidos -bloqueaba 3 s por siembra, nadie leia esa
  // hora, y rejuvenecia el contador del respaldo-. Poner aqui la bandera del hardware
  // volveria a escribir el RTC que el firmware dejo de escribir.
  g_horaSembrada = true;
}
#endif  // ARNES_RELOJ_REAL

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
  const bool hay = g_rx.sacar(destino);
#ifndef ARNES_RELOJ_REAL
  // N-142: el contador de tramas validas de SFTY-15, que aqui vive en el doble de
  // protocolo.cpp. Lo lee bluetooth_loop() REAL para saber que la radio VOLVIO, y de esa
  // bandera cuelga el $ACK del ambar de emergencia: sin este ++, el equipo contestaria
  // "sin radio" para siempre despues de la primera caida.
  if (hay) g_rxValidas++;
#endif
  return hay;
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
#ifdef ARNES_RELOJ_REAL
  arnes_millis_valor = arnes_reloj_local(ms);   // el HSI de esta punta: rtc_periferico.h
#else
  arnes_millis_valor = ms;
#endif
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
#ifdef ARNES_RELOJ_REAL
  // Con el reloj.cpp real no hay doble que cuente las llamadas a reloj_notarRadio(): se le
  // pregunta a reloj_radioManda() REAL, que es lo que esas llamadas alimentan (bloque E0).
  if (!strcmp(que, "radio_manda"))         return reloj_radioManda() ? 1 : 0;
  // D-26 (4): el salto de hora de ESTA punta. Una siembra ACEPTADA entregada en la frontera
  // de segundo, con la MISMA funcion que el Maestro (rtc_periferico.h): las dos puntas saltan
  // con la misma aritmetica.
  if (!strcmp(que, "desviar_rtc"))         return (long)arnes_sembrar_en_frontera(arg, sembrarDirecto);
  if (!strcmp(que, "hora_caduca_ms"))      return (long)HORA_CADUCA_MS;
  if (!strcmp(que, "hora_fiable"))         return reloj_horaFiable() ? 1 : 0;
  if (!strcmp(que, "reloj_en_hora"))       return reloj_enHora() ? 1 : 0;
  if (!strcmp(que, "segundos_del_dia"))    return (long)reloj_segundosDelDia();
  if (!strcmp(que, "hsi_ppm"))             { arnes_hsi_ppm(arg); return 1; }
  if (!strcmp(que, "fase_subsegundo"))     return arnes_fase_subsegundo();
  if (!strcmp(que, "siembra_esp32")) {
    char iso[24];
    arnes_iso(iso, sizeof(iso), (uint8_t)(arg / 86400L), arg % 86400L);
    return (long)ramaHoraEsp32(iso);
  }
  // El eco: la guarda de la radio se pregunta AHORA, la siembra se entrega en la frontera.
  if (!strcmp(que, "siembra_esp32_eco")) {
    if (reloj_radioManda()) return 2;
    return (long)arnes_sembrar_en_frontera(0, sembrarDirecto);
  }
  if (!strcmp(que, "alarmas_caducada"))    return (long)g_alarmasCaducada;
  if (!strcmp(que, "eventos_salto_rojo"))  return (long)g_eventosSaltoRojo;
  // Con bluetooth.cpp fuera, las alarmas se cuentan en el doble de arriba, y el veto del
  // ambar de la app se mueve a mano: es la unica forma de recorrer la rama vetada cuando
  // el despachador que arma el cerrojo no se compila.
  if (!strcmp(que, "alarmas"))             return (long)g_alarmasEmitidas;
  if (!strcmp(que, "ambar_bluetooth"))     { g_ambarEmergencia = (arg != 0); return 1; }
#else
  if (!strcmp(que, "radio_notada"))        return (long)g_radioNotada;
  // D-26 (4): el salto de hora de ESTA punta, para el bloque E del orquestador del
  // Degradado. BLOQUE LITERAL de la orden "desviar_rtc" de adaptador_maestro_deg.cpp -mueve
  // la hora de pared arg segundos SIN tocar el ancla, o sea sin fabricar un residuo
  // sub-segundo propio-: las dos puntas tienen que saltar con la misma aritmetica.
  if (!strcmp(que, "desviar_rtc")) {
    long s = (long)g_rtcSegundosDelDiaBase + arg;
    while (s < 0) s += 86400L;
    g_rtcSegundosDelDiaBase = (uint32_t)(s % 86400L);
    g_rtcBaseSegundos = (uint32_t)((long)g_rtcBaseSegundos + arg);
    return 1;
  }
  // --- N-142: EL TELEFONO, POR EL CABLE DE VERDAD -----------------------------
  //
  // "bt:<linea>" teclea una linea en el puerto del telefono y la despacha el
  // bluetooth.cpp REAL en el siguiente tick de esta punta. El orquestador NO escribe
  // aqui el literal del comando: lo LEE del C++ (ver leerLiteralAmbar() alli), porque un
  // literal escrito en el arnes seguiria midiendo el comando de ayer el dia que se
  // renombre -que es exactamente lo que paso con FORZAR_ROJO en N-83-.
  //
  // "ack:<subcadena>" contesta si el ULTIMO acuse que el equipo mando al telefono la
  // contiene. Es lo que la app leeria, no lo que el arnes supone: hasta hoy ninguna
  // prueba que moviera luces miraba lo que el equipo CONTESTA.
  if (!strncmp(que, "bt:", 3))             return btTeclear(que + 3) ? 1 : 0;
  if (!strncmp(que, "ack:", 4))            return strstr(g_btUltimoAcuse, que + 4) ? 1 : 0;
  if (!strcmp(que, "acuses"))              return (long)g_btAcuses;
  if (!strcmp(que, "eventos_bt"))          return (long)g_btEventos;
  // El cerrojo REAL de bluetooth.cpp, no una copia: es una de las dos guardas que vetan
  // un CMD_GO_GREEN en el despachador de main.cpp (N-83 / D-8).
  if (!strcmp(que, "ambar_latch"))         return bluetooth_ambarEmergencia() ? 1 : 0;
  if (!strcmp(que, "alarmas"))             return (long)g_btAlarmas;
  // N-162 (12/09): las dos banderas del reloj, por separado. El bloque D las necesita para
  // decir CUAL de las dos puertas de degradado_reanudarTrasCorte() cerro tras un corte, en
  // vez de mirar solo el desenlace. "rtc_hw_en_hora" es la de la pila, la que ningun
  // firmware escribe desde el 11/09; "reloj_en_hora" es la de RAM que el corte se lleva.
  if (!strcmp(que, "reloj_en_hora"))       return reloj_enHora() ? 1 : 0;
  if (!strcmp(que, "hora_fiable"))         return reloj_horaFiable() ? 1 : 0;
  if (!strcmp(que, "rtc_hw_en_hora"))      return g_rtcHwEnHora ? 1 : 0;
#endif
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
  if (!strcmp(que, "recargas_watchdog"))   return (long)IWatchdog.recargas;
  if (!strcmp(que, "replay_reseteos"))     return (long)g_replayReseteos;
  // 🔴 11/09 - AQUI VIVIA "ambar_emergencia_app", Y SE RETIRA EN VEZ DE ARREGLARSE.
  //
  // Era una TRANSCRIPCION de la rama del ambar de emergencia -"bloque literal", decia su
  // comentario- copiada de la puerta CON PIN, la unica que avisaba al Maestro. La app usa
  // la de SIN PIN, que no avisaba: el arnes ejercia la copia buena de una puerta mala y
  // por eso el bloque G podia dar verde sin ver nada (N-142 a medias, §3.16-A). Una copia
  // a mano del firmware no se arregla copiandola mejor: se retira y se compila el
  // despachador de verdad. Lo que la sustituye es "bt:", que teclea la MISMA LINEA que
  // manda el telefono.
  if (!strcmp(que, "menu_abierto"))        { g_menuAbierto = (arg != 0); return 1; }
  if (!strcmp(que, "respaldo_valido"))     return respaldo_valido() ? 1 : 0;
  if (!strcmp(que, "respaldo_degradado"))  return respaldo_degradadoActivo() ? 1 : 0;
  // N-162 (12/09): la SEGUNDA puerta de degradado_reanudarTrasCorte(), preguntada con los
  // mismos dos argumentos con los que la pregunta el firmware -respaldo.cpp REAL y el
  // contador crudo del RTC-. El -1 es RESPALDO_SYNC_CADUCADA, que no es una hora grande
  // sino "no se cuanto ha pasado" (respaldo.h): se traduce aqui, donde la constante esta
  // incluida, para no escribir 0xFFFFFFFF en el orquestador.
  if (!strcmp(que, "respaldo_horas_sync")) {
    const uint32_t h = respaldo_horasDesdeSync(reloj_contadorSegundos());
    return (h == RESPALDO_SYNC_CADUCADA) ? -1L : (long)h;
  }
  return PUNTA_DESCONOCIDO;
}

// --- El dominio de respaldo: lo que la pila mantiene a traves de un corte -------
PUNTA_API long punta_dominio_leer(int indice) {
  volatile uint32_t* dr = &arnes_bkp.DR1;
  if (indice >= 0 && indice < 10) return (long)dr[indice];
#ifdef ARNES_RELOJ_REAL
  return arnes_dominio_leer_rtc(indice);   // el silicio del RTC: rtc_periferico.cpp
#else
  switch (indice) {
    case 10: return (long)reloj_contadorSegundos();
    // N-162: lo que la pila mantiene NO es "hay hora", es "el RTC hardware quedo escrito".
    // Desde el 11/09 ninguna linea del firmware lo escribe, asi que esto sale 0 siempre y
    // el equipo despierta sin hora, igual que la tarjeta. Ver el bloque de las banderas.
    case 11: return g_rtcHwEnHora ? 1 : 0;
    // Y por eso estos dos son el CALENDARIO DEL RTC, no la hora de pared sembrada: solo
    // valen algo cuando el indice 11 dice que aquel RTC se escribio alguna vez.
    case 12: return (long)(g_rtcSegundosDelDiaBase + rtcTranscurrido()) % 86400L;
    case 13: return (long)g_rtcDia;
    default: return 0;
  }
#endif
}

PUNTA_API void punta_dominio_escribir(int indice, long valor) {
  volatile uint32_t* dr = &arnes_bkp.DR1;
  if (indice >= 0 && indice < 10) { dr[indice] = (uint32_t)valor; return; }
#ifdef ARNES_RELOJ_REAL
  arnes_dominio_escribir_rtc(indice, valor);
#else
  switch (indice) {
    case 10: g_rtcBaseSegundos = (uint32_t)valor; g_rtcAncla = arnes_millis_valor; break;
    case 11: g_rtcHwEnHora = (valor != 0); break;
    case 12: g_rtcSegundosDelDiaBase = (uint32_t)valor; break;
    case 13: g_rtcDia = (uint8_t)valor; break;
    default: break;
  }
#endif
}

}  // extern "C"
