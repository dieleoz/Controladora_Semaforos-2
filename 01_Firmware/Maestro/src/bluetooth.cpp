// ===== src/bluetooth.cpp (MAESTRO) =====
#include "bluetooth.h"
#include "pines.h"
#include "coordinador.h"
#include "demanda.h"
#include "modo_automatico.h"
#include "modo_degradado.h"
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

// --- Dos envoltorios que devuelven lo que la funcion de abajo no sabe decir -------
//
// Las dos que envuelven son `void` Y ABANDONAN EN SILENCIO si no se cumple su
// precondicion. Desde el despachador eso es invisible: se llama, no pasa nada, y el
// telefono recibe RESULT:OK igual. Aqui se pregunta la MISMA condicion que mira la
// guarda -no una parecida- y se devuelve un bool, que es lo unico que permite mandar
// $ERR en vez de mentir.

// coordinador_pedirCambio() empieza con "if (estadoC != C_IDLE) return;", y
// coordinador_listoParaContar() es literalmente "return estadoC == C_IDLE": no es una
// aproximacion a la guarda, es la guarda leida por la unica puerta publica que hay.
static bool pedirCambioVerificado() {
  if (!coordinador_listoParaContar()) return false;
  coordinador_pedirCambio();
  return true;
}

// reloj_ajustar() empieza con "if (!rtcOperativo) return;" -y reloj_hayCristal()
// devuelve ese mismo rtcOperativo-, y ademas descarta la llamada ENTERA si algun campo
// se sale de rango. Lo primero lo pregunta el llamante, porque tiene motivo propio; lo
// segundo no se puede preguntar antes sin copiar aqui los rangos de reloj.cpp, asi que
// se MIDE despues: se relee la hora que quedo puesta.
//
// El desempate importa cuando el reloj YA estaba en hora: ahi reloj_enHora() sigue
// diciendo true aunque el ajuste se haya descartado entero, y sin releer se contestaria
// OK sobre una hora que no es la que mandaron.
static bool ajustarRelojVerificado(uint8_t h, uint8_t mi, uint8_t s, uint8_t d) {
  reloj_ajustar(h, mi, s, d);
  // Los segundos no se comparan: entre escribir y releer el RTC puede haber avanzado
  // uno. Si el minuto avanza justo en ese hueco esto contesta que no pudo, con la hora
  // bien puesta -es el lado seguro del error, y el operario repite-.
  return reloj_enHora() && reloj_hora() == h && reloj_minuto() == mi;
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
  //
  // OTRAS DOS ORDENES ENTRAN SIN PIN, POR EL MISMO CRITERIO DE ARRIBA: ni MENU ni
  // ALCANCE abren paso -el primero deja el equipo en la pantalla, sin ciclo; el segundo
  // en rojo fijo-, y el PIN guarda lo que ABRE, no lo que para. Se aceptan tambien con
  // PIN, igual que FORZAR_ROJO: la app antepone la clave a todo lo que no este en su
  // lista SIN_PIN, y una orden que solo se aceptara sin PIN seria inalcanzable desde el
  // celular.
  //
  // Lo que se mueve es DONDE EMPIEZA LA ACCION, no la cadena de comparaciones: las dos
  // formas caen en el mismo strcmp de mas abajo. Una segunda cadena para las ordenes sin
  // PIN serian dos contratos que alguien tendria que sincronizar, y el dia que uno se
  // quede atras el comando funciona por una puerta y contesta DESCONOCIDO por la otra.
  const char* accion;
  if (strncmp(cmd, "CMD:PIN:1234:", 13) == 0) {
    accion = cmd + 13;
  } else if (strncmp(cmd, "CMD:", 4) == 0 &&
             (strcmp(cmd + 4, "SET_MODO:MENU") == 0 ||
              strcmp(cmd + 4, "SET_MODO:ALCANCE") == 0)) {
    accion = cmd + 4;
  } else {
    enviarTramaConCrc("$ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO");
    return;
  }

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
  } else if (strcmp(accion, "SET_MODO:MENU") == 0) {
    // EN DEGRADADO NO SE SALTA AL MENU. Se pide la salida, que es el todo-rojo de 30 s
    // de ROJO_TRANSICION_MS. Sin el, esta unidad cae a rojo mientras la otra sigue
    // dando verde por reloj, y el escenario peligroso de ese modo es exactamente que
    // UNA SOLA PUNTA lo abandone. Es la MISMA puerta que el boton 4 del gabinete.
    if (modoActual_get() == MODO_DEGRADADO) {
      // El bool distingue haber arrancado la salida de encontrarla ya en marcha, y en
      // ninguno de los dos casos se contesta OK: el menu tarda todavia el todo-rojo
      // entero en llegar, y decir OK seria dar por hecho un cambio que no ha ocurrido.
      if (modo_degradado_pedirSalida()) {
        enviarTramaConCrc("$ACK,CMD:SET_MODO:MENU,RESULT:SALIENDO_TODO_ROJO");
        bluetooth_reportarEvento("APP_BLUETOOTH", "SALIDA_DEGRADADO_PEDIDA");
      } else {
        enviarTramaConCrc("$ERR,CMD:SET_MODO:MENU,DESC:YA_VUELVE_AL_MENU");
      }
    } else {
      modoActual_set(MENU);
      menu_setup();
      enviarTramaConCrc("$ACK,CMD:SET_MODO:MENU,RESULT:OK");
      bluetooth_reportarEvento("APP_BLUETOOTH", "SET_MODO_MENU");
    }
  } else if (strcmp(accion, "SET_MODO:ALCANCE") == 0) {
    // Del Degradado se sale por su puerta y no por aqui. El rojo fijo del alcance es
    // seguro por si mismo, pero llegar a el saltandose el todo-rojo deja a la otra
    // punta dando verde sin que nadie haya verificado las dos.
    if (modoActual_get() == MODO_DEGRADADO) {
      enviarTramaConCrc("$ERR,CMD:SET_MODO:ALCANCE,DESC:EN_MARCHA_PARE_EL_MODO");
    } else {
      modoActual_set(MODO_ALCANCE);
      enviarTramaConCrc("$ACK,CMD:SET_MODO:ALCANCE,RESULT:OK");
      bluetooth_reportarEvento("APP_BLUETOOTH", "SET_MODO_ALCANCE");
    }
  } else if (strcmp(accion, "SET_MODO:INTELIGENTE") == 0) {
    // PIDE PIN porque arranca un ciclo que DA VERDES, y por eso mismo no se entra desde
    // el Degradado: la salida de aquel obliga a un todo-rojo de 30 s en las dos puntas,
    // y ponerse a ciclar sin cumplirlo deja a la otra unidad en su ciclo por reloj.
    if (modoActual_get() == MODO_DEGRADADO) {
      enviarTramaConCrc("$ERR,CMD:SET_MODO:INTELIGENTE,DESC:EN_MARCHA_PARE_EL_MODO");
    } else {
      modoActual_set(MODO_INTELIGENTE);
      enviarTramaConCrc("$ACK,CMD:SET_MODO:INTELIGENTE,RESULT:OK");
      bluetooth_reportarEvento("APP_BLUETOOTH", "SET_MODO_INTELIGENTE");
    }
  } else if (strcmp(accion, "SET_MODO:DEGRADADO") == 0) {
    // LA PUERTA ES LA MISMA QUE LA DE LA PANTALLA Y LA DEL MANDO. Tres vias de entrada
    // con tres criterios serian una sola puerta: la mas floja de las tres. Y esta es la
    // unica del firmware que enciende un verde sin confirmacion del otro extremo.
    const MotivoDegradado m = modo_degradado_evaluarEntrada();
    if (m != MDG_OK) {
      // El motivo se compone con los MISMOS dos textos que ensena el gabinete. Una
      // tabla propia para el celular seria una tercera que alguien tendria que
      // sincronizar, y el dia que difieran el tecnico de arriba y el de abajo leerian
      // causas distintas del mismo rechazo.
      char p[80];
      snprintf(p, sizeof(p), "$ERR,CMD:SET_MODO:DEGRADADO,DESC:%s %s",
               modo_degradado_motivoL1(m), modo_degradado_motivoL2(m));
      enviarTramaConCrc(p);
    } else {
      modoActual_set(MODO_DEGRADADO);
      enviarTramaConCrc("$ACK,CMD:SET_MODO:DEGRADADO,RESULT:OK");
      bluetooth_reportarEvento("APP_BLUETOOTH", "SET_MODO_DEGRADADO");
    }
  } else if (strcmp(accion, "FORZAR_ROJO") == 0) {
    coordinador_forzarRojoTotal();
    enviarTramaConCrc("$ACK,CMD:FORZAR_ROJO,RESULT:OK");
    bluetooth_reportarEvento("APP_BLUETOOTH", "FORZAR_ROJO_TOTAL");
  } else if (strcmp(accion, "MANUAL:CAMBIAR_TURNO") == 0) {
    // Antes se mandaba OK pasara lo que pasara. coordinador_pedirCambio() abandona en
    // silencio si el coordinador no esta en reposo -esta a mitad de una transicion, que
    // incluye un todo-rojo de despeje en curso-, asi que el operario recibia la
    // confirmacion, no cambiaba nada, y volvia a pulsar creyendo que no le hacian caso.
    //
    // No se fuerza el cambio: el rechazo es correcto -partir un despeje por la mitad es
    // justo lo que no se puede hacer-. Lo que faltaba era DECIRLO.
    if (pedirCambioVerificado()) {
      enviarTramaConCrc("$ACK,CMD:CAMBIAR_TURNO,RESULT:OK");
      bluetooth_reportarEvento("APP_BLUETOOTH", "CAMBIAR_TURNO_MANUAL");
    } else {
      enviarTramaConCrc("$ERR,CMD:CAMBIAR_TURNO,DESC:EN_TRANSICION_REINTENTE");
    }
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
    //
    // ESTE COMANDO CONTESTABA OK SIN HABER PUESTO NADA, y con N-17 confirmado en
    // hardware -el cristal Y2 que no oscila- ese era el caso NORMAL, no el raro: sin
    // oscilador reloj_ajustar() se abstiene a proposito -escribir dejaria una hora en
    // un contador que nadie hace avanzar-, y aqui se mandaba la confirmacion igual. El
    // tecnico se iba del poste creyendo que dejo el reloj puesto.
    //
    // Los tres finales son distintos y el operario necesita los tres distintos: no hay
    // con que contar el tiempo; la hora no entro; la hora entro y va camino del Esclavo.
    int y, mo, d, h, mi, s;
    if (sscanf(accion + 8, "%d-%d-%d,%d:%d:%d", &y, &mo, &d, &h, &mi, &s) != 6) {
      enviarTramaConCrc("$ERR,CMD:SET_RTC,DESC:FORMATO_INVALIDO");
    } else if (!reloj_hayCristal()) {
      // No se nombra ninguna pieza: N-45 quito "Es Y2: toca hardware" de la pantalla
      // por senalar un componente sin haberlo medido, y con Y2 nuevo seguia diciendo lo
      // mismo. Lo que el micro VE lo cuenta CONSULTA RELOJ.
      enviarTramaConCrc("$ERR,CMD:SET_RTC,DESC:SIN_CRISTAL_VEA_CONSULTA_RELOJ");
    } else if (!ajustarRelojVerificado((uint8_t)h, (uint8_t)mi, (uint8_t)s, (uint8_t)d)) {
      // El ajuste se descarto ENTERO por un campo fuera de rango. Los rangos siguen
      // viviendo en reloj.cpp, que es quien conoce el calendario; aqui solo se relee lo
      // que quedo puesto y se reusa el motivo que ya existe para una trama que no sirve.
      enviarTramaConCrc("$ERR,CMD:SET_RTC,DESC:FORMATO_INVALIDO");
    } else if (!coordinador_sincronizarHora()) {
      // HOY ESTE CAMINO NO PUEDE OCURRIR: sincronizarHora() solo se niega si
      // !reloj_enHora(), que la linea de arriba acaba de comprobar. Se deja porque su
      // bool existe y no se tira, y porque es lo unico que se enterara el dia que a esa
      // funcion le anadan otra precondicion -la hora quedaria puesta aqui y el Esclavo
      // no la recibiria, que es medio arreglo y se lee como entero-.
      enviarTramaConCrc("$ACK,CMD:SET_RTC,RESULT:HORA_PUESTA_SIN_PROPAGAR");
    } else {
      enviarTramaConCrc("$ACK,CMD:SET_RTC,RESULT:OK");
      bluetooth_reportarEvento("APP_BLUETOOTH", "RTC_AJUSTADO_Y_SYNC");
    }
  } else if (strcmp(accion, "REINICIAR_RELOJ") == 0) {
    // N-31. PIDE PIN porque BORRA LA HORA Y TODO EL RESPALDO -ciclo acordado, marca de
    // sincronizacion e indicador del Degradado-, o sea la autorizacion de la que cuelga
    // el unico modo que da verde sin el otro extremo.
    //
    // El bool no se ignora: dice si el oscilador arranco tras el reinicio. Contestar OK
    // en los dos casos mandaria al tecnico a poner la hora en un equipo que no puede
    // contarla. Y no se nombra ninguna pieza -N-45 quito "Es Y2: toca hardware" de la
    // pantalla por afirmar sin haber medido-: lo que sigue lo dice CONSULTA RELOJ.
    if (reloj_reiniciarDominioRespaldo()) {
      enviarTramaConCrc("$ACK,CMD:REINICIAR_RELOJ,RESULT:CRISTAL_OK_PONGA_LA_HORA");
      bluetooth_reportarEvento("APP_BLUETOOTH", "RELOJ_REINICIADO");
    } else {
      enviarTramaConCrc("$ERR,CMD:REINICIAR_RELOJ,DESC:SIGUE_PARADO_VEA_CONSULTA_RELOJ");
    }
  } else if (strcmp(accion, "DEMANDA") == 0) {
    // NO ES SOLICITAR_PASO, y la diferencia no es de nombre: alli el Esclavo PIDE a
    // este equipo (SFTY-27), y aqui el que decide ya es este. La peticion entra por la
    // misma puerta que la camara y la gobierna modo_inteligente.cpp.
    if (modoActual_get() != MODO_INTELIGENTE) {
      // Fuera del Modo Inteligente nadie lee la demanda: registrarla y contestar OK
      // seria apuntar una peticion que ningun ciclo va a mirar.
      enviarTramaConCrc("$ERR,CMD:DEMANDA,DESC:SOLO_EN_MODO_INTELIGENTE");
    } else if (demanda_solicitar()) {
      enviarTramaConCrc("$ACK,CMD:DEMANDA,RESULT:REGISTRADA");
      bluetooth_reportarEvento("APP_BLUETOOTH", "DEMANDA_LOCAL");
    } else {
      // No se finge una peticion que se descarto: si el operario no sabe que su
      // pulsacion cayo en la ventana de silencio, volvera a pulsar creyendo que no le
      // hacen caso.
      enviarTramaConCrc("$ERR,CMD:DEMANDA,DESC:REPITA_EN_UNOS_SEGUNDOS");
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
