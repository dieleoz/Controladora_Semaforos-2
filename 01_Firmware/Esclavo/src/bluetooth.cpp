// ===== src/bluetooth.cpp (ESCLAVO) =====
#include "bluetooth.h"
#include "pines.h"
#include "semaforo.h"
#include "protocolo.h"
#include "demanda.h"
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
//
// IDENTICO AL MAESTRO A PROPOSITO: las dos tarjetas se cablean igual.
static HardwareSerial SerialBT(PB7, PB6); // USART1 remapeado: PB7 RX, PB6 TX

static unsigned long tUltimaTelemetria = 0;
static char btBufIn[64];
static uint8_t btIdxIn = 0;

// SFTY-21 / N-83 — UNA EMERGENCIA PEDIDA POR BLUETOOTH VALE LO MISMO QUE UNA DEL MANDO.
//
// Mientras vale true, main.cpp no obedece las ordenes de luz del Maestro, igual que con
// mando_ambarLocal(). Sin este latch el ambar pedido desde la app duraba lo que tardaba
// en llegar el siguiente latido -unos 3 s-: el operario veia al equipo obedecer y
// volverse atras solo, sin que nadie se lo dijera. La orden es la misma y la razon para
// darla es la misma; que la haya dado un dedo en el gabinete o un dedo en el telefono no
// puede cambiar cuanto dura.
static bool ambarEmergencia = false;

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
  snprintf(payload, sizeof(payload), "$ALARM,NODE:ESCLAVO,EVENTO:%s,CAUSA:%s,ACCION:%s,HORA:%s",
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
  snprintf(payload, sizeof(payload), "$EVENT,NODE:ESCLAVO,ORIGEN:%s,DETALLE:%s,HORA:%s",
           origen, detalle, horaBuf);
  enviarTramaConCrc(payload);
}

static void procesarComando(const char* cmd) {
  // SFTY - EL AMBAR DE EMERGENCIA NO PIDE PIN, Y ES DELIBERADO.
  //
  // N-83: LA RAZON DE ANTES ERA FALSA Y POR ESO SE REESCRIBE. Decia "el PIN guarda lo
  // que ABRE paso o mueve luces; no lo que las para", dando por hecho que esto para el
  // trafico. No lo para: deja el equipo en S_FALLO, que es ambar intermitente a 500 ms
  // con la talanquera ARRIBA (semaforo.cpp, decision del cliente y del PMT del
  // 27/08/2026). O sea que este camino SI abre paso, y la exencion quedaba justificada
  // por algo que no ocurre.
  //
  // La exencion sigue siendo correcta, pero por esto otro: un ambar intermitente NO LE
  // DA PRIORIDAD A NADIE. No concede el paso a un sentido contra el otro -que es lo que
  // el PIN existe para custodiar-, sino que pone a los dos a pasar con precaucion y
  // bajo su propia responsabilidad. Es la caida segura universal, la misma a la que el
  // equipo llega solo cuando pierde el enlace (SFTY-6) o cuando el watchdog lo
  // reinicia. Y una caida segura que exija recordar una clave delante de un accidente
  // no es una caida segura: quien esta viendo el incidente tiene que poder pedirla,
  // aunque no sea el tecnico que se sabe el PIN.
  //
  // Se acepta tambien la forma con PIN mas abajo: la app la envia asi y el manual
  // la documenta. Las dos entradas hacen lo mismo.
  if (strcmp(cmd, "CMD:AMBAR_EMERGENCIA") == 0) {
    semaforo_iniciarFallo();
    ambarEmergencia = true;
    enviarTramaConCrc("$ACK,CMD:AMBAR_EMERGENCIA,RESULT:OK");
    bluetooth_reportarEvento("APP_BLUETOOTH", "AMBAR_EMERGENCIA_SIN_PIN");
    return;
  }

  // N-83: EL NOMBRE VIEJO SE RECHAZA ENSENANDO EL BUENO, NO EN SILENCIO.
  //
  // "FORZAR_ROJO" prometia rojo y hacia ambar con la pluma arriba, que es casi lo
  // contrario. El comportamiento es el correcto y no se toca; lo que se corrige es que
  // el equipo declare otra cosa de la que hace.
  //
  // Dejarlo como alias mudo conservaria la mentira: quien lo mande seguira creyendo
  // que detuvo el cruce. Y dejarlo caer al $ERR,CMD:DESCONOCIDO del final tampoco
  // sirve: quien manda esto es alguien con una app o un manual anteriores al cambio, y
  // lo que necesita no es enterarse de que el comando no existe, sino de como se llama
  // ahora. Es el mismo patron con el que esta punta rechaza TEST_LEDS: se dice el
  // motivo.
  //
  // La forma con PIN se rechaza en la cadena de 'accion', mas abajo, para no repetir
  // aqui el literal del PIN.
  //
  // OJO AL MAESTRO: alli CMD:FORZAR_ROJO se queda, y alli SI hace rojo de verdad. Que
  // las dos puntas usen literales distintos es lo correcto, porque hacen cosas
  // distintas; llamarlas igual era el defecto.
  if (strcmp(cmd, "CMD:FORZAR_ROJO") == 0) {
    enviarTramaConCrc("$ERR,CMD:FORZAR_ROJO,DESC:RENOMBRADO_USE_AMBAR_EMERGENCIA");
    bluetooth_reportarEvento("APP_BLUETOOTH", "FORZAR_ROJO_RENOMBRADO");
    return;
  }

  // Validación estricta de PIN de 4 dígitos (1234)
  if (strncmp(cmd, "CMD:PIN:1234:", 13) != 0) {
    enviarTramaConCrc("$ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO");
    return;
  }

  const char* accion = cmd + 13;

  if (strcmp(accion, "AMBAR_EMERGENCIA") == 0) {
    semaforo_iniciarFallo();
    ambarEmergencia = true;
    enviarTramaConCrc("$ACK,CMD:AMBAR_EMERGENCIA,RESULT:OK");
    bluetooth_reportarEvento("APP_BLUETOOTH", "AMBAR_EMERGENCIA_LOCAL");
  } else if (strcmp(accion, "FORZAR_ROJO") == 0) {
    // N-83: la misma puerta que arriba, con el PIN puesto. Ver alli el porque de
    // rechazar en vez de aliasar. Una app vieja lo manda por las dos, y las dos tienen
    // que contestar lo mismo: sin esta rama la forma con PIN caeria al DESCONOCIDO
    // generico y la forma sin PIN daria un motivo util, que es la peor combinacion
    // -el mismo error contestado de dos maneras segun por donde entre-.
    enviarTramaConCrc("$ERR,CMD:FORZAR_ROJO,DESC:RENOMBRADO_USE_AMBAR_EMERGENCIA");
    bluetooth_reportarEvento("APP_BLUETOOTH", "FORZAR_ROJO_RENOMBRADO");
  } else if (strcmp(accion, "SOLICITAR_PASO") == 0) {
    // EL ESCLAVO PIDE; NO ORDENA. Ver OPTIMIZACIONES.md SFTY-27.
    //
    // El funcional del PMT se coloca en el extremo que haga falta y no tiene por que
    // saber cual de los dos postes es el Maestro. Esto lo resuelve sin darle mando a
    // esta punta: aqui no se enciende nada, se manda la MISMA demanda que manda la
    // camara, y el Maestro decide, aplica el todo-rojo y ordena.
    //
    // Con dos funcionales, uno en cada extremo, el Maestro serializa: ninguno concede
    // nada, los dos piden. Que pulsen a la vez tiene que ser aburrido.
    if (demanda_solicitar()) {
      enviarTramaConCrc("$ACK,CMD:SOLICITAR_PASO,RESULT:PEDIDO_AL_MAESTRO");
      bluetooth_reportarEvento("APP_BLUETOOTH", "SOLICITUD_PASO_ENVIADA");
    } else {
      // No se finge un envio que no ocurrio: si el operario no sabe que su pulsacion
      // cayo en la ventana de silencio, volvera a pulsar creyendo que no le hacen caso.
      enviarTramaConCrc("$ERR,CMD:SOLICITAR_PASO,DESC:REPITA_EN_UNOS_SEGUNDOS");
    }
  } else if (strcmp(accion, "TEST_LEDS") == 0) {
    // RECHAZADO A PROPOSITO, y no es una limitacion pendiente de quitar.
    //
    // semaforo_iniciarTestLeds() enciende 6 s de secuencia -rojo, ambar y VERDE- sin
    // mirar nada. Lanzado sobre un Esclavo en servicio, ese verde sale mientras el
    // Maestro esta dando paso al otro sentido: dos vehiculos entrando de frente al
    // tramo. Y da igual que el tecnico se haya conectado al Esclavo correcto: el
    // peligro no es equivocarse de poste, es que esta punta acepte mover luces.
    //
    // Vuelve cuando exista un estado FUERA DE SERVICIO que el propio equipo conozca,
    // no una promesa del manual.
    enviarTramaConCrc("$ERR,CMD:TEST_LEDS,DESC:NO_EN_SERVICIO_USE_EL_MAESTRO");
    bluetooth_reportarEvento("APP_BLUETOOTH", "TEST_LEDS_RECHAZADO");
  } else if (strncmp(accion, "SET_RTC:", 8) == 0) {
    // CMD:PIN:1234:SET_RTC:YYYY-MM-DD,HH:MM:SS (Inyeccion Courier RTC)
    //
    // NO SE CONTESTA OK SIN HABER MIRADO SI SE PUDO. reloj_ajustar() no devuelve
    // nada y rechaza EN SILENCIO en dos sitios -sin oscilador, reloj.cpp:173; con
    // cifras fuera de rango, reloj.cpp:175-, asi que el $ACK incondicional de antes
    // era una promesa que nadie habia comprobado. Con N-17 confirmado en hardware
    // -el cristal Y2 no oscila en las tarjetas actuales- ese OK falso es hoy la
    // respuesta HABITUAL, no un caso raro: el tecnico se va del poste convencido de
    // que dejo el reloj puesto, y esta punta sigue sin hora.
    //
    // AQUI NO HAY reloj_hayCristal(): esa funcion solo la declara el reloj.h del
    // Maestro. La pregunta equivalente en esta punta es reloj_contadorSegundos(),
    // que reserva el 0 para "no hay reloj" y devuelve 1 antes que un cero real
    // (reloj.cpp:131 y 143), de modo que su cero significa exactamente "el RTC no
    // esta operativo". Se pregunta ANTES para no escribir sobre un contador parado.
    //
    // Y no se propaga nada despues: el Esclavo no tiene coordinador.cpp, su hora
    // viaja por radio desde el Maestro. Aqui solo se responde la verdad.
    //
    // Las cifras se acotan tambien aqui, ademas de en reloj.cpp, porque si no un
    // ajuste descartado por rango sobre un reloj que YA estaba en hora dejaria
    // reloj_enHora() en true y volveria a contestar OK sin haber escrito nada. Se
    // reusa FORMATO_INVALIDO -una hora imposible es tan mal formato como una trama
    // que no casa- para no gastar flash en un literal nuevo. El dia se exige 1..31
    // porque el Courier manda siempre la fecha completa; el 0 de "no toques la
    // fecha" no llega nunca por esta puerta.
    int y, mo, d, h, mi, s;
    if (sscanf(accion + 8, "%d-%d-%d,%d:%d:%d", &y, &mo, &d, &h, &mi, &s) == 6 &&
        h >= 0 && h <= 23 && mi >= 0 && mi <= 59 && s >= 0 && s <= 59 &&
        d >= 1 && d <= 31) {
      if (reloj_contadorSegundos()) reloj_ajustar((uint8_t)h, (uint8_t)mi, (uint8_t)s, (uint8_t)d);
      // Con las cifras ya acotadas y el oscilador en marcha, reloj_ajustar() no
      // puede rechazar: reloj_enHora() distingue entonces el ajuste hecho del que
      // no se pudo intentar.
      if (reloj_enHora()) {
        enviarTramaConCrc("$ACK,CMD:SET_RTC,RESULT:OK");
        bluetooth_reportarEvento("APP_BLUETOOTH", "COURIER_RTC_INYECTADO");
      } else {
        enviarTramaConCrc("$ERR,CMD:SET_RTC,DESC:SIN_CRISTAL");
      }
    } else {
      enviarTramaConCrc("$ERR,CMD:SET_RTC,DESC:FORMATO_INVALIDO");
    }
  } else {
    enviarTramaConCrc("$ERR,CMD:DESCONOCIDO,DESC:COMANDO_NO_SOPORTADO_EN_ESCLAVO");
  }
}

bool bluetooth_testLedsActivo() {
  return semaforo_testLedsEnCurso();
}

bool bluetooth_ambarEmergencia() {
  return ambarEmergencia;
}

void bluetooth_loop() {
  const unsigned long ahora = millis();

  // LA SALIDA DE ESTE AMBAR ES LOCAL, Y ES LA MISMA QUE LA DEL MANDO.
  //
  // Un latch persistente sin salida no es una mejora: es un nodo sordo al Maestro
  // hasta el siguiente corte de corriente, y eso seria peor que el defecto que esto
  // arregla. La salida existe y es la de siempre: el A.A.A del mando, que llama a
  // semaforo_forzarRojo() y saca la luz de S_FALLO. Aqui no hace falta que mando.cpp
  // avise de nada -este latch no es suyo-: basta con mirar el resultado. El ambar de
  // emergencia solo tiene sentido MIENTRAS la luz sigue en el ambar que lo motivo.
  //
  // Y la radio no puede colarse por esta puerta, que es lo que la hace segura: con el
  // latch puesto main.cpp no obedece ninguna orden de luz, asi que el Maestro no puede
  // sacar al nodo de S_FALLO para que el latch se caiga solo. Solo lo saca de ahi
  // alguien que este delante del gabinete.
  //
  // Va aqui y no dentro del getter para que la revocacion ocurra UNA vez por vuelta y
  // ANTES de que se lea la radio: main.cpp llama a bluetooth_loop() antes del bloque de
  // paquetes, asi que no queda ni una vuelta con un latch caduco decidiendo.
  if (ambarEmergencia && semaforo_estado() != S_FALLO) ambarEmergencia = false;

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

    const char* estadoStr = semaforo_nombreEstado();

    char horaBuf[16];
    if (reloj_enHora()) {
      snprintf(horaBuf, sizeof(horaBuf), "%02u:%02u:%02u", reloj_hora(), reloj_minuto(), reloj_segundo());
    } else {
      strncpy(horaBuf, "--:--:--", sizeof(horaBuf));
    }

    unsigned long tFaseSeg = (ahora / 1000UL) % 60UL;

    char serieTxt[7];
    identidad_texto(serieTxt);

    char payload[128];
    snprintf(payload, sizeof(payload),
             "$STATUS,NODE:ESCLAVO,SERIE:%s,MODO:SUBORDINADO,ESTADO:%s,T:%lu,RF:98%%,RTT:85ms,BAT:12.6,HORA:%s",
             serieTxt, estadoStr, tFaseSeg, horaBuf);

    enviarTramaConCrc(payload);
  }
}
