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
//   - ~~reloj.cpp: incluye <STM32RTC.h>, que no tiene sustituto en el repositorio. Se
//     modela el PERIFERICO~~ -> DESDE EL 11/09 (D-21 (1)) reloj.cpp ENTRA REAL, el de las
//     dos puntas. Lo que se sustituye es el SILICIO: STM32RTC.h y el HAL del LSE y del
//     contador, en reloj_real/, con el borde escrito alli (Y2 arrancando). Sin eso la
//     caducidad de la siembra y la frontera de 25 s de reloj_radioManda() no las ejecutaba
//     nadie: el doble que habia aqui no las tenia.
//   - mando.cpp, bluetooth.cpp, botones.cpp: no deciden la fase del Degradado. La rama
//     CMD:HORA_ESP32 de bluetooth.cpp -la que siembra- se transcribe en la orden
//     "siembra_esp32", literal, porque bluetooth.cpp arrastra el puerto serie entero.

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
#include "rtc_periferico.h"  // D-21 (1): el HSI, la linea del ESP32 y la siembra en frontera

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
// D-21 (1): la alarma de la hora caducada se cuenta POR SU CAUSA, que es lo que el tecnico
// lee. Y el $EVENT del salto de hora que pasa por rojo (D-26 (4)), por su detalle: es la
// palabra del firmware sobre lo que acaba de hacer, y el bloque F la contrasta con los pines.
static int  g_alarmasCaducada = 0;
static int  g_eventosSaltoRojo = 0;
void bluetooth_reportarAlarma(const char* evento, const char* causa, const char* accion) {
  (void)accion;
  snprintf(g_ultimaAlarmaEvento, sizeof(g_ultimaAlarmaEvento), "%s", evento);
  g_alarmasEmitidas++;
  if (!strcmp(evento, "HORA_ESP32") && !strcmp(causa, "CADUCADA")) g_alarmasCaducada++;
}
void bluetooth_reportarEvento(const char* origen, const char* detalle) {
  if (!strcmp(origen, "DEGRADADO") && !strcmp(detalle, "SALTO_DE_HORA_POR_ROJO")) {
    g_eventosSaltoRojo++;
  }
}

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
// EL RELOJ ES EL REAL (11/09, D-21 (1)). Aqui habia un modelo del RTC -contador,
// getters y un reloj_ajustar() escrito a mano- que se retira ENTERO: reloj.cpp se compila
// en esta DLL y lo que se sustituye es solo el silicio (reloj_real/). Con el modelo, la
// hora del Maestro no caducaba nunca y la regla nueva no la ejecutaba nadie.
//
// LA RAMA CMD:HORA_ESP32 DE bluetooth.cpp, TRANSCRITA. bluetooth.cpp no se compila aqui
// -arrastra el puerto serie y el despachador entero- y la rama que siembra es de cuatro
// lineas: se copia LITERAL en lo que decide -sembrar, y SOLO SI ENTRO propagar al Esclavo-,
// sin el diario. Si la rama cambia, esto se queda viejo: lo compara reloj_04.
// ---------------------------------------------------------------------------
static int ramaHoraEsp32(const char* iso) {
  if (reloj_sembrarDesdeIso(iso)) {
    coordinador_sincronizarHora();
    return 1;
  }
  return 0;
}

// El salto de hora que el bloque E inyecta: una siembra ACEPTADA, sin la propagacion. Es
// la herramienta con que el orquestador mueve la hora de esta punta, no un camino del
// equipo, y por eso va directa al sembrador.
static int sembrarDirecto(const char* iso) { return reloj_sembrarDesdeIso(iso) ? 1 : 0; }

// coordinador.cpp llama a reloj_fijarEnero() -la real, ahora- cada 10 min. Con base de
// software sembrada no toca el RTC (N-162); el arnes ya no cuenta la llamada.

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
  // main.cpp la llama la primera de la vuelta, detras del perro. Con el modelo del RTC era
  // un cuerpo vacio y se omitia; con el reloj.cpp real es la que mantiene el cerrojo de la
  // caducidad de D-21 (1), asi que entra en su sitio.
  reloj_actualizar();
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
  arnes_millis_valor = arnes_reloj_local(ms);   // el HSI de esta punta: rtc_periferico.h
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
  // EL SALTO DE HORA, CONSERVANDO LA FASE SUB-SEGUNDO.
  //
  // Mueve la hora de pared de ESTA punta arg segundos sin mover el instante en que cambia
  // de segundo. Con el reloj.cpp real (11/09) eso es una SIEMBRA -la unica forma que tiene
  // el equipo de mover su hora- entregada en la ultima frontera de segundo
  // (arnes_sembrar_en_frontera, rtc_periferico.h). El motivo de conservar la fase no ha
  // cambiado: reanclar al millis() de la inyeccion fabricaba un residuo sub-segundo propio
  // del arnes -el solape de 950 ms en la frontera, demasiado redondo para un tiempo de aire
  // de 50 ms- y el umbral publicado se movia con el instante de inyectar.
  if (!strcmp(que, "desviar_rtc"))        return (long)arnes_sembrar_en_frontera(arg, sembrarDirecto);
  if (!strcmp(que, "reloj_en_hora"))      return reloj_enHora() ? 1 : 0;
  if (!strcmp(que, "segundos_del_dia"))   return (long)reloj_segundosDelDia();

  // --- D-21 (1): la hora que caduca, y el ESP32 que la siembra ------------------
  // La caducidad COMPILADA en esta DLL, no un numero copiado: el orquestador la pide aqui.
  if (!strcmp(que, "hora_caduca_ms"))     return (long)HORA_CADUCA_MS;
  if (!strcmp(que, "hora_fiable"))        return reloj_horaFiable() ? 1 : 0;
  if (!strcmp(que, "hsi_ppm"))            { arnes_hsi_ppm(arg); return 1; }
  if (!strcmp(que, "fase_subsegundo"))    return arnes_fase_subsegundo();
  // La linea del ESP32 con la hora de SU DS3231, empaquetada como dia*86400 + segundos del
  // dia. Entra por la rama transcrita, en el instante del banco en que llega.
  if (!strcmp(que, "siembra_esp32")) {
    char iso[24];
    arnes_iso(iso, sizeof(iso), (uint8_t)(arg / 86400L), arg % 86400L);
    return (long)ramaHoraEsp32(iso);
  }
  // Un ESP32 cuyo DS3231 dice la MISMA hora que ya tiene esta punta: la siembra que la
  // mantiene fresca sin moverla. Es la de los bloques B..E, que miden la geometria del
  // ciclo y no la deriva; ver la cabecera del bloque F del orquestador.
  if (!strcmp(que, "siembra_esp32_eco"))  return (long)arnes_sembrar_en_frontera(0, ramaHoraEsp32);
  if (!strcmp(que, "alarmas_caducada"))   return (long)g_alarmasCaducada;
  if (!strcmp(que, "eventos_salto_rojo")) return (long)g_eventosSaltoRojo;

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
// Los indices 10..13 son ahora el SILICIO del RTC (rtc_periferico.cpp): el contador y el
// calendario que la pila mantiene. Con el reloj.cpp real la hora sembrada NO vive ahi
// -desde N-162 la siembra no escribe el RTC- y por eso no sobrevive a un corte; este
// arnes no ejerce el corte (ver la cabecera del orquestador).
PUNTA_API long punta_dominio_leer(int indice) {
  volatile uint32_t* dr = &arnes_bkp.DR1;
  if (indice >= 0 && indice < 10) return (long)dr[indice];
  return arnes_dominio_leer_rtc(indice);
}

PUNTA_API void punta_dominio_escribir(int indice, long valor) {
  volatile uint32_t* dr = &arnes_bkp.DR1;
  if (indice >= 0 && indice < 10) { dr[indice] = (uint32_t)valor; return; }
  arnes_dominio_escribir_rtc(indice, valor);
}

}  // extern "C"
