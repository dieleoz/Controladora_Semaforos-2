// ===== src/bluetooth.cpp (MAESTRO) =====
#include "bluetooth.h"
#include "pines.h"
#include "coordinador.h"
#include "modo_automatico.h"
#include "semaforo.h"
#include "menu.h"
#include "reloj.h"
#include "identidad.h"
#include <string.h>
#include <stdio.h>

// USART1 REMAPEADO a PB7 (RX) / PB6 (TX). Antes iba en PA9/PA10, y se movio
// porque el modulo Bluetooth de campo se conecta al CONECTOR J17 -posiciones 2
// y 3-, que es enchufable. PA9/PA10 no salen a ninguna bornera de la tarjeta:
// para usarlos hay que soldar en las patas del MAX3485 U2 o del propio micro.
//
// Es el MISMO periferico, no un segundo puerto serie: el STM32F103 permite
// sacar USART1 por PB6/PB7 en vez de PA9/PA10, pero solo por un sitio a la vez.
// Por eso protocolo_setup() dejo de abrir AiBus, que declaraba el mismo USART1
// sobre PA10/PA9 a 115200 -ver el comentario de alli-.
//
// Estos dos pines los tenia lcd.cpp para PSB y RST del display; ninguno de los
// dos llevaba datos, asi que la pantalla sigue funcionando con SCLK/SID/CS.
static HardwareSerial SerialBT(PB7, PB6); // USART1 remapeado: PB7 RX, PB6 TX

static unsigned long tUltimaTelemetria = 0;
static char btBufIn[64];
static uint8_t btIdxIn = 0;

static uint8_t calcularChecksum(const char* str) {
  uint8_t crc = 0;
  while (*str && *str != '*') {
    crc ^= (uint8_t)(*str);
    str++;
  }
  return crc;
}

static void enviarTramaConCrc(const char* payload) {
  uint8_t crc = calcularChecksum(payload + 1); // Salta el '$' inicial
  char tramaCompleta[140];
  snprintf(tramaCompleta, sizeof(tramaCompleta), "%s*%02X\r\n", payload, crc);
  SerialBT.print(tramaCompleta);
}

void bluetooth_setup() {
  // EL CHIP ES U2, NO U3. Aqui ponia "U3" y estaba invertido; se deja escrito para que no
  // se vuelva a poner al reves. Trazado red por red sobre el esquematico bueno
  // (01_Firmware/Controladora_Semaforos/.../Controladora_Semaforos.kicad_sch):
  //
  //   U2 = MAX3485 del USART1: RO(1)->PA10, ~RE(2) y DE(3)->PA8, DI(4)->PA9. Par A/B por J10.
  //   U3 = MAX3485 del USART3, que es el de la radio LoRa: PB11, PB12, PB10. Par A/B por J12.
  //
  // Y FALTA EL MATIZ QUE IMPORTA: PA8 gobierna A LA VEZ ~RE (pin 2) y DE (pin 3) de U2.
  // Ponerlo HIGH apaga el RECEPTOR -que es lo que se busca, porque asi PA10 queda libre
  // para el modulo Bluetooth- pero deja el TRANSMISOR ENCENDIDO. Consecuencia: U2 vuelca
  // la telemetria por J10 de forma permanente y esa linea NO puede recibir nunca. Hoy es
  // inofensivo porque J10 esta vacio; deja de serlo el dia que alguien cuelgue algo de
  // J10, y ese dia hay que tocar este codigo, no el cableado.
  //
  // Es la leccion del repetidor del 31/07/2026, ya escrita en 01_Firmware/TROUBLESHOOTING.md
  // ("RS-485 half-duplex: el control DE/RE"): si un DE/RE se queda permanentemente en alto,
  // esa linea queda bloqueada en AMBOS sentidos.
  pinMode(RS485_IN_DE_RE, OUTPUT);
  digitalWrite(RS485_IN_DE_RE, HIGH); // Apaga el receptor RO de U2 y libera PA10 al modulo Bluetooth
  SerialBT.begin(9600);
}

void bluetooth_reportarAlarma(const char* evento, const char* causa, const char* accion) {
  char horaBuf[16];
  if (reloj_enHora()) {
    snprintf(horaBuf, sizeof(horaBuf), "%02u:%02u:%02u", reloj_hora(), reloj_minuto(), reloj_segundo());
  } else {
    strncpy(horaBuf, "--:--:--", sizeof(horaBuf));
  }

  char payload[100];
  snprintf(payload, sizeof(payload), "$ALARM,NODE:MAESTRO,EVENTO:%s,CAUSA:%s,ACCION:%s,HORA:%s",
           evento, causa, accion, horaBuf);
  enviarTramaConCrc(payload);
}

void bluetooth_reportarEvento(const char* origen, const char* detalle) {
  char horaBuf[16];
  if (reloj_enHora()) {
    snprintf(horaBuf, sizeof(horaBuf), "%02u:%02u:%02u", reloj_hora(), reloj_minuto(), reloj_segundo());
  } else {
    strncpy(horaBuf, "--:--:--", sizeof(horaBuf));
  }

  char payload[100];
  snprintf(payload, sizeof(payload), "$EVENT,NODE:MAESTRO,ORIGEN:%s,DETALLE:%s,HORA:%s",
           origen, detalle, horaBuf);
  enviarTramaConCrc(payload);
}

static void procesarComando(const char* cmd) {
  // SFTY - EL ROJO DE EMERGENCIA NO PIDE PIN, Y ES DELIBERADO.
  //
  // mando.cpp ya lo dejo escrito para el mando de reles: "lo seguro, facil; lo
  // peligroso, dificil". Detener el trafico es la accion SEGURA -el equipo cae a
  // todo-rojo-, asi que ponerle una clave delante solo retrasa a quien esta viendo
  // el incidente. El PIN guarda lo que ABRE paso o mueve luces; no lo que las para.
  //
  // Se acepta tambien la forma con PIN mas abajo: la app la envia asi y el manual
  // la documenta. Las dos entradas hacen lo mismo.
  if (strcmp(cmd, "CMD:FORZAR_ROJO") == 0) {
    coordinador_forzarRojoTotal();
    enviarTramaConCrc("$ACK,CMD:FORZAR_ROJO,RESULT:OK");
    bluetooth_reportarEvento("APP_BLUETOOTH", "FORZAR_ROJO_SIN_PIN");
    return;
  }

  // Validación estricta de PIN de 4 dígitos (1234)
  if (strncmp(cmd, "CMD:PIN:1234:", 13) != 0) {
    enviarTramaConCrc("$ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO");
    return;
  }

  const char* accion = cmd + 13;

  if (strcmp(accion, "SET_MODO:AUTO") == 0) {
    modoActual_set(MODO_AUTOMATICO);
    coordinador_iniciarModo();
    enviarTramaConCrc("$ACK,CMD:SET_MODO:AUTO,RESULT:OK");
    bluetooth_reportarEvento("APP_BLUETOOTH", "SET_MODO_AUTO");
  } else if (strcmp(accion, "SET_MODO:MANUAL") == 0) {
    modoActual_set(MODO_MANUAL);
    coordinador_iniciarModo();
    enviarTramaConCrc("$ACK,CMD:SET_MODO:MANUAL,RESULT:OK");
    bluetooth_reportarEvento("APP_BLUETOOTH", "SET_MODO_MANUAL");
  } else if (strcmp(accion, "SET_MODO:AMBAR") == 0) {
    modoActual_set(MODO_AMBAR);
    enviarTramaConCrc("$ACK,CMD:SET_MODO:AMBAR,RESULT:OK");
    bluetooth_reportarEvento("APP_BLUETOOTH", "SET_MODO_AMBAR");
  } else if (strcmp(accion, "FORZAR_ROJO") == 0) {
    coordinador_forzarRojoTotal();
    enviarTramaConCrc("$ACK,CMD:FORZAR_ROJO,RESULT:OK");
    bluetooth_reportarEvento("APP_BLUETOOTH", "FORZAR_ROJO_TOTAL");
  } else if (strcmp(accion, "MANUAL:CAMBIAR_TURNO") == 0) {
    coordinador_pedirCambio();
    enviarTramaConCrc("$ACK,CMD:CAMBIAR_TURNO,RESULT:OK");
    bluetooth_reportarEvento("APP_BLUETOOTH", "CAMBIAR_TURNO_MANUAL");
  } else if (strcmp(accion, "TEST_LEDS") == 0) {
    semaforo_iniciarTestLeds();
    enviarTramaConCrc("$ACK,CMD:TEST_LEDS,RESULT:STARTING_6S");
    bluetooth_reportarEvento("APP_BLUETOOTH", "TEST_LEDS_INICIADO");
  } else if (strncmp(accion, "SET_TIEMPOS:", 12) == 0) {
    // N-69: los tiempos del ciclo, desde el celular en vez de subiendo a la pantalla.
    //
    // Los limites NO se comprueban aqui: los tiene modoAutomatico_fijarTiempos(), que
    // es quien conoce el ciclo. Este sitio solo traduce texto a numeros. Repetir los
    // rangos en los dos lados seria una segunda copia que alguien tendria que
    // sincronizar -y el dia que difieran, la app dejaria pasar lo que el ciclo rechaza-.
    int v = 0, r = 0, d = 0;
    if (sscanf(accion + 12, "%d,%d,%d", &v, &r, &d) != 3) {
      enviarTramaConCrc("$ERR,CMD:SET_TIEMPOS,DESC:FORMATO_INVALIDO");
    } else if (modoAutomatico_enMarcha()) {
      // Con el ciclo corriendo no se tocan: bajar un tiempo a mitad de fase acortaria
      // la fase EN CURSO, y una de esas fases es el todo-rojo de despeje.
      enviarTramaConCrc("$ERR,CMD:SET_TIEMPOS,DESC:EN_MARCHA_PARE_EL_MODO");
    } else if (!modoAutomatico_fijarTiempos((uint8_t)v, (uint8_t)r, (uint8_t)d)) {
      enviarTramaConCrc("$ERR,CMD:SET_TIEMPOS,DESC:RANGO");
    } else {
      enviarTramaConCrc("$ACK,CMD:SET_TIEMPOS,RESULT:OK");
      bluetooth_reportarEvento("APP_BLUETOOTH", "TIEMPOS_CAMBIADOS");
    }
  } else if (strncmp(accion, "SET_RTC:", 8) == 0) {
    // CMD:PIN:1234:SET_RTC:YYYY-MM-DD,HH:MM:SS
    int y, mo, d, h, mi, s;
    if (sscanf(accion + 8, "%d-%d-%d,%d:%d:%d", &y, &mo, &d, &h, &mi, &s) == 6) {
      reloj_ajustar((uint8_t)h, (uint8_t)mi, (uint8_t)s, (uint8_t)d);
      coordinador_sincronizarHora();
      enviarTramaConCrc("$ACK,CMD:SET_RTC,RESULT:OK");
      bluetooth_reportarEvento("APP_BLUETOOTH", "RTC_AJUSTADO_Y_SYNC");
    } else {
      enviarTramaConCrc("$ERR,CMD:SET_RTC,DESC:FORMATO_INVALIDO");
    }
  } else {
    enviarTramaConCrc("$ERR,CMD:DESCONOCIDO,DESC:COMANDO_NO_SOPORTADO");
  }
}

static const char* obtenerNombreModo(ModoSistema m) {
  switch (m) {
    case MENU: return "MENU";
    case MODO_MANUAL: return "MANUAL";
    case MODO_AUTOMATICO: return "AUTO";
    case MODO_INTELIGENTE: return "INTELIGENTE";
    case MODO_ALCANCE: return "ALCANCE";
    case MODO_HORA: return "HORA";
    case MODO_DEGRADADO: return "DEGRADADO";
    case MODO_AMBAR: return "AMBAR";
    default: return "DESCONOCIDO";
  }
}

bool bluetooth_testLedsActivo() {
  return semaforo_testLedsEnCurso();
}

void bluetooth_loop() {
  const unsigned long ahora = millis();

  // 1. Recepción de Comandos desde la App Móvil
  while (SerialBT.available() > 0) {
    char c = (char)SerialBT.read();
    if (c == '\n' || c == '\r') {
      if (btIdxIn > 0) {
        btBufIn[btIdxIn] = '\0';
        procesarComando(btBufIn);
        btIdxIn = 0;
      }
    } else if (btIdxIn < sizeof(btBufIn) - 1) {
      btBufIn[btIdxIn++] = c;
    }
  }

  // 2. Emisión periódica de Telemetría cada 1000ms ($STATUS,...)
  if (ahora - tUltimaTelemetria >= 1000) {
    tUltimaTelemetria = ahora;

    const char* modoStr = obtenerNombreModo(modoActual_get());
    const char* estadoStr = coordinador_nombreEstadoMaster();
    int rfCalidad = coordinador_calidadEnlace();
    if (rfCalidad < 0) rfCalidad = 0;
    unsigned long rtt = coordinador_tiempoRespuestaMs();

    char horaBuf[16];
    if (reloj_enHora()) {
      snprintf(horaBuf, sizeof(horaBuf), "%02u:%02u:%02u", reloj_hora(), reloj_minuto(), reloj_segundo());
    } else {
      strncpy(horaBuf, "--:--:--", sizeof(horaBuf));
    }

    // Cuenta de segundos transcurridos en fase actual (T:)
    unsigned long tFaseSeg = (ahora / 1000UL) % 60UL;

    char serieTxt[7];
    identidad_texto(serieTxt);

    char payload[128];
    snprintf(payload, sizeof(payload),
             "$STATUS,NODE:MAESTRO,SERIE:%s,MODO:%s,ESTADO:%s,T:%lu,RF:%d%%,RTT:%lums,BAT:12.6,HORA:%s",
             serieTxt, modoStr, estadoStr, tFaseSeg, rfCalidad, rtt, horaBuf);

    enviarTramaConCrc(payload);
  }
}
