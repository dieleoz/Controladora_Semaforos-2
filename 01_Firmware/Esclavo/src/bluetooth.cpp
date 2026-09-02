// ===== src/bluetooth.cpp (ESCLAVO) =====
#include "bluetooth.h"
#include "pines.h"
#include "semaforo.h"
#include "protocolo.h"
#include "demanda.h"
#include "reloj.h"
#include "identidad.h"
#include "mando.h"           // R-3: mando_ambarLocal(), para no prometer un ambar que no se quita
#include "modo_degradado.h"  // N-106: la salida ordenada y sus dos finales
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

// ---------------------------------------------------------------------------
// A3 - EL REGISTRO DE SILENCIO DEL PUERTO J17.
//
// POR QUE LO LLEVA ESTE MICRO. El ESP32 no puede reportar su propia muerte: el equipo
// que sobrevive al fallo es este, asi que el registro es suyo. Hoy, si J17 se calla, no
// lo apunta nadie, y cuando el tecnico llega lo que paso ya no esta.
//
// QUE MIDEN ESTOS TRES NUMEROS Y QUE NO. ESTA ES LA MITAD IMPORTANTE.
//
// MEDIDO sobre el puente, no supuesto:
//   ESP32_Expansion/src/enlace_stm32.cpp:27-33  "6.4 - SILENCIO NO ES ORDEN [...] aqui
//                                               NO se manda nada"
//   ESP32_Expansion/src/puente.cpp:29-35        lo mismo por el otro lado
//   enlace_escribirLinea() tiene UN solo llamador -puente.cpp:116-, y es el reenvio
//   VERBATIM de lo que mando la app.
// Y sobre la app: no hay un solo envio periodico al firmware; sus dos setInterval son
// el reloj de la cabecera y vigilarEnlace(), que solo ESCUCHA.
//
// O sea que por J17 entra EXACTAMENTE lo que un dedo pulsa en el telefono. De modo que
// esto cuenta SILENCIOS DEL PUERTO, no muertes del puente: mientras nadie use la app el
// puerto esta mudo con el puente perfectamente vivo, y desde aqui "el puente no dice
// nada" y "el puente no esta" NO SE DISTINGUEN -lo dice el propio puente en los dos
// comentarios de arriba-.
//
// Se nombra asi, y no "cortes del puente", porque un contador que prometiera eso seria
// el RF:98% otra vez: un numero con forma de medida. El dia que el ESP32 emita un latido
// propio -AB-1, ESP32_Expansion/src/main.cpp:22-28, abierto y del responsable- estos
// mismos tres numeros pasan a ser el registro de cortes de verdad SIN TOCAR UNA LINEA de
// aqui. Esa es la mitad que si se puede construir hoy.
//
// NO HAY UMBRAL, Y ES LA DECISION DE DISENO. Un silencio se define por sus DOS EXTREMOS
// -la linea que lo abre y la que lo cierra-, nunca por un limite. Un limite seria un
// numero que nadie ha decidido gobernando lo que el tecnico ve, y ademas la precondicion
// para que alguien acabe alimentando este silencio con el de la radio. SFTY6_SILENCIO_MS
// vigila la RADIO y no se nombra en este fichero: son dos silencios y dos instrumentos.
//
// Y ESTO CUENTA; NO ACTUA. De aqui no sale una sola escritura de luz. El ambar
// automatico sigue reservado a los caminos que ya lo tienen -SFTY-6 y el watchdog-: la
// maquina no decide sola operar de un modo que nadie pidio.
//
// SE MIDE POR LINEA COMPLETA, no por byte: el puente entrega lineas y el receptor de
// abajo solo actua sobre lineas. La PRIMERA linea tras el arranque cierra el silencio
// que empezo en el arranque, que es un silencio real y el unico capaz de contestar
// "llevo tres dias sin que nadie hable por aqui".
//
//
// EL ULTIMO SILENCIO NO SE GUARDA, Y NO ES UN OLVIDO. Se publica en el instante en que
// se cierra -que es el unico en que alguien puede estar mirando, ver abajo-, asi que
// entre llamada y llamada no lo lee nadie. MEDIDO: con la variable static puesta,
// arm-none-eabi-nm sobre el .elf NO la encuentra en .bss -el enlazador ya sabia que
// nadie la lee y la habia borrado-. Dejarla declarada seria anunciar un estado que no
// existe: quien lo leyera creeria que se puede consultar mas tarde, y no se puede.
// LIMITE DECLARADO: millis() da la vuelta a los 49,7 dias. La resta sin signo mide bien
// UNA vuelta; un silencio mas largo que eso queda aliasado. Se escribe en vez de
// esconderse.
// ---------------------------------------------------------------------------
static unsigned long tUltimaLineaJ17  = 0;  // millis() de la ultima linea completa
static unsigned long j17Silencios     = 0;  // cerrados: uno por linea. 0 = ninguna aun
static unsigned long j17SilencioMaxMs = 0;  // el mas largo desde el arranque

// SFTY-21 / N-83 — UNA EMERGENCIA PEDIDA POR BLUETOOTH VALE LO MISMO QUE UNA DEL MANDO.
//
// Mientras vale true, main.cpp no obedece las ordenes de luz del Maestro, igual que con
// mando_ambarLocal(). Sin este latch el ambar pedido desde la app duraba lo que tardaba
// en llegar el siguiente latido -unos 3 s-: el operario veia al equipo obedecer y
// volverse atras solo, sin que nadie se lo dijera. La orden es la misma y la razon para
// darla es la misma; que la haya dado un dedo en el gabinete o un dedo en el telefono no
// puede cambiar cuanto dura.
//
// N-106 (31/08) - AQUI VIVIA LA REVOCACION AUTOMATICA, Y SE RETIRA. Estaba en
// bluetooth_loop() y decia: "si el latch esta puesto y la luz ya no esta en S_FALLO,
// tirar el latch". Su razonamiento era correcto MIENTRAS el ambar fuera inmediato, y
// dejo de serlo el dia que este comando pasa a salir del Modo Degradado por el todo-rojo
// de despedida: durante esos 10 a 90 s la luz esta en ROJO, o sea fuera de S_FALLO, asi
// que el latch moria milisegundos despues de armarse y con el se apagaban los tres vetos
// de main.cpp (:406, :416, :540). El $ACK ya se habia enviado.
//
// No se parchea con una excepcion: se retira, y en su lugar queda EXACTAMENTE lo que
// tiene el mando -un sostenedor en el bucle y una revocacion explicita-. El motivo por el
// que existia esta escrito en bluetooth.h y era honesto: "no hay comando de Bluetooth que
// lo revoque, asi que se cae solo". Desde R-3 SI lo hay (CMD:PIN:1234:CANCELAR_AMBAR), y
// una revocacion que ocurre porque una persona la pide es lo contrario de una que ocurre
// porque la maquina deshizo sola una proteccion que puso alguien.
static bool ambarEmergencia = false;

// N-108 - "esta punta ya declaro que se quedo sin radio". Lo pone quien manda la alarma
// y lo baja el aviso de vuelta; no es un watchdog y no tiene reloj. Ver bluetooth_loop().
static bool enlaceCaidoAnunciado = false;

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
  // N-108: 160 y no 140. El $ALARM pasa a llevar el ultimo tramo del enlace y su peor
  // caso medido son 128 B de payload; con 140 aqui, el cierre del checksum entraba
  // justo y una CAUSA mas larga habria truncado la trama SIN AVISO -y una trama
  // truncada es una que el otro extremo descarta por checksum, o sea una alarma que
  // desaparece justo cuando hace falta-.
  char tramaCompleta[160];
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

  // N-108 - LA ALARMA LLEVA EL ULTIMO TRAMO, Y EN ESTA PUNTA ES OTRO. Y ESO ES EL DATO.
  //
  // El Maestro puede contar latidos porque el los MANDA: sabe cuantos salieron y cuantos
  // volvieron. Esta punta solo CONTESTA, asi que no tiene ni ventana de latidos ni ida y
  // vuelta que cronometrar. Lo que si tiene, y es real, son los tres contadores de
  // SFTY-15 (protocolo.cpp:101-103), que separan las tres averias que desde el suelo se
  // ven todas igual -"no hay comunicacion"-:
  //
  //   RX en 0                 no llega NADA: cobertura, canal o antena
  //   RX alto y OK en 0       llega BASURA: cableado, linea flotando, radio atascada
  //   los dos suben, RUIDO alto  enlace marginal por distancia o lluvia
  //
  // Ese reparto es exactamente "desde donde se cayo" visto desde este extremo, que es el
  // que se queda sin radio a 5 m de altura y el que hasta hoy no tenia un solo dato.
  //
  // NO SE PUBLICA UN RF% AQUI, Y NO ES POR PEREZA. El unico cociente que se podria formar
  // -OK / (OK + RUIDO)- no es una tasa de perdida, y lo dice el propio protocolo.cpp:120
  // en su nota: "un solo byte de ruido puede provocar varios descartes seguidos mientras
  // la ventana se desplaza. El contador mide RUIDO, no tramas perdidas una a una". Ademas
  // OK cuenta las 3 copias de cada rafaga (SFTY-11), asi que ni el numerador es de
  // mensajes. Un porcentaje sobre eso seria el RF:98% de antes con otra forma: peor, de
  // hecho, porque se moveria y por eso nadie sospecharia de el.
  char tramo[44];
  snprintf(tramo, sizeof(tramo), "RX:%lu,OK:%lu,RUIDO:%lu",
           protocolo_bytesRecibidos(), protocolo_tramasValidas(),
           protocolo_tramasDescartadas());

  // Se anota que la caida YA se anuncio, para que la vuelta tenga con que compararse.
  // No decide nada: la decision es de SFTY-6, en main.cpp; aqui solo se toma nota.
  enlaceCaidoAnunciado = true;

  char payload[144];
  snprintf(payload, sizeof(payload), "$ALARM,NODE:ESCLAVO,EVENTO:%s,CAUSA:%s,%s,ACCION:%s,HORA:%s",
           evento, causa, tramo, accion, horaBuf);
  enviarTramaConCrc(payload);
}

void bluetooth_reportarEvento(const char* origen, const char* detalle) {
  char horaBuf[16];
  if (reloj_enHora()) {
    snprintf(horaBuf, sizeof(horaBuf), "%02u:%02u:%02u", reloj_hora(), reloj_minuto(), reloj_segundo());
  } else {
    strncpy(horaBuf, "--:--:--", sizeof(horaBuf));
  }

  // N-114: 112 y no 100, IGUAL QUE EL MAESTRO Y A PROPOSITO. Alli el numero lo obliga
  // el DETALLE de ORIGEN:RELOJ -los bits del RTC, 44 caracteres en su peor caso por
  // tipo-, y esta punta todavia no lo emite porque su reloj.h no declara
  // reloj_diagnostico(). Se sube igual: el dia que lo declare, este emisor tiene que
  // ser el mismo bloque letra por letra, y dos buffers distintos en la misma funcion de
  // las dos puntas son la divergencia que luego nadie recuerda haber introducido.
  char payload[112];
  snprintf(payload, sizeof(payload), "$EVENT,NODE:ESCLAVO,ORIGEN:%s,DETALLE:%s,HORA:%s",
           origen, detalle, horaBuf);
  enviarTramaConCrc(payload);
}

// Cierra el silencio de J17 que la linea recien recibida acaba de terminar, y lo publica.
//
// SE PUBLICA EN EL INSTANTE EN QUE SE CIERRA PORQUE ES EL UNICO EN QUE SE PUEDE. "Cuanto
// lleva mudo ahora mismo" no es observable desde la app por construccion: la unica forma
// de que alguien este mirando es que acabe de mandar una linea, y esa linea es justo la
// que termina el silencio. Guardar el dato para un comando que lo pida seria guardarlo
// para el unico momento en que ya vale cero.
//
// Por eso sale una linea de bitacora por cada silencio cerrado y no solo por los que
// baten el record: el que le interesa al tecnico que acaba de llegar es el SUYO -"este
// puerto llevaba catorce horas sin que nadie hablara"-, y ese no tiene por que ser el
// mayor. El coste esta medido y cabe: ver el presupuesto de bytes de J17.
//
// EN SEGUNDOS, no en ms. Un corte se cuenta en horas y el ms no aporta nada; en ms el
// peor caso del tipo son diez cifras y la trama se comia el margen del payload -que es
// como N-108 encontro un $ALARM truncandose por la HORA sin que nada lo dijera-.
static void j17RegistrarLinea(unsigned long ahora) {
  const unsigned long silencio = ahora - tUltimaLineaJ17;
  tUltimaLineaJ17 = ahora;
  if (silencio > j17SilencioMaxMs) j17SilencioMaxMs = silencio;
  j17Silencios++;

  char det[48];
  snprintf(det, sizeof(det), "MUDO:%lus,MAX:%lus,N:%lu",
           silencio / 1000UL, j17SilencioMaxMs / 1000UL, j17Silencios);
  bluetooth_reportarEvento("J17", det);
}

// --- El envoltorio que devuelve lo que degradado_salir() no sabe decir ------------
//
// degradado_salir() es `void` Y ABANDONA EN SILENCIO: desde DEG_INACTIVO y desde
// DEG_SALIENDO no hace nada, y desde DEG_RENDIDO solo baja el cartel. Llamarla suelta y
// contestar $ACK detras es el OK mudo que este repositorio persigue -es literalmente lo
// que el banco rechazo del primer intento de N-106-.
//
// Aqui se pregunta la MISMA guarda que ella tiene, no una parecida: modo_degradado.cpp
// arranca la salida en iniciarSalida(false) solo si el estado es DEG_ENTRANDO o
// DEG_ACTIVO. Es el mismo patron que pedirCambioVerificado() del Maestro.
static bool salidaDegradadoIniciada() {
  const EstadoDegradado e = degradado_estado();
  if (e != DEG_ENTRANDO && e != DEG_ACTIVO) return false;
  degradado_salir();
  return true;
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
  //
  // N-106 (31/08) - Y NO SE SALTA EL MODO DEGRADADO. La tabla completa es el Manual 10
  // S4.5.2; aqui va el porque, que es vial:
  //
  // Antes esto era semaforo_iniciarFallo() a secas. Con el Degradado gobernando la luz
  // eso saltaba de un VERDE POR RELOJ directo a ambar intermitente, y su propio mando.cpp
  // ya tenia escrito lo que eso significa: "le daria a quien ya venia lanzado una senal
  // que invita a negociar el paso mientras aun cree tener prioridad". Por eso el Degradado
  // entra y sale SIEMPRE por todo-rojo, y por eso el B.B.B del mando sale por ahi.
  //
  // R-1 (31/08): se acepta que el ambar tarde de 10 a 90 s -el todo-rojo de despedida sale
  // de cfgDespeje-, porque ese margen es lo unico que protege a quien ya venia lanzado. Es
  // exactamente lo que ya cuesta el B.B.B.
  //
  // LAS DOS PUERTAS -esta sin PIN y la de 'accion' con PIN- LLEVAN EL MISMO BLOQUE, letra
  // por letra. Un parche a una sola deja media puerta abierta contestando el $ACK viejo;
  // lo vigilan esclavo_07 y esclavo_08.
  if (strcmp(cmd, "CMD:AMBAR_EMERGENCIA") == 0) {
    if (!degradado_gobiernaLuz()) {
      // Filas A y B: nadie mas gobierna la luz, el ambar se enciende ya. No hay $ERR
      // posible en este camino y no se inventa uno: semaforo_iniciarFallo() no tiene
      // guarda y no puede fallar, y armar el latch es una asignacion.
      const bool yaEnAmbar = (semaforo_estado() == S_FALLO);
      semaforo_iniciarFallo();
      ambarEmergencia = true;
      if (yaEnAmbar) {
        // Fila B. Lo que esta orden cambia NO es la luz -ya estaba en ambar por SFTY-6,
        // por el watchdog o por un B.B.B-: es el latch, que convierte un ambar que el
        // siguiente CMD_GO_RED se llevaria en uno vetado. Contestar OK ocultaria que lo
        // unico nuevo es la proteccion.
        enviarTramaConCrc("$ACK,CMD:AMBAR_EMERGENCIA,RESULT:YA_EN_AMBAR_LATCH_PUESTO");
      } else {
        enviarTramaConCrc("$ACK,CMD:AMBAR_EMERGENCIA,RESULT:OK");
      }
      bluetooth_reportarEvento("APP_BLUETOOTH", "AMBAR_EMERGENCIA_SIN_PIN");
    } else if (salidaDegradadoIniciada()) {
      // Fila C. El RESULT no es OK a proposito: el ambar no esta puesto todavia y decir
      // OK seria dar por hecho un cambio de luz que tarda hasta 90 s. Lo que garantiza
      // que llegue es el sostenedor de bluetooth_loop() -la segunda mitad del molde-.
      ambarEmergencia = true;
      enviarTramaConCrc("$ACK,CMD:AMBAR_EMERGENCIA,RESULT:SALIENDO_TODO_ROJO");
      bluetooth_reportarEvento("APP_BLUETOOTH", "AMBAR_EMERGENCIA_SIN_PIN");
    } else if (degradado_rendicionEnCurso()) {
      // Fila D con la salida en curso terminando en AMBAR: es la rendicion por el limite
      // de 48 h, que acaba en DEG_RENDIDO encendiendo el ambar por su cuenta. La orden no
      // arranca nada -degradado_salir() ya no opera- pero el latch SI cambia algo: sin el,
      // ese ambar duraria hasta la siguiente orden del Maestro. R-2.
      ambarEmergencia = true;
      enviarTramaConCrc("$ACK,CMD:AMBAR_EMERGENCIA,RESULT:SALIDA_YA_EN_CURSO");
      bluetooth_reportarEvento("APP_BLUETOOTH", "AMBAR_EMERGENCIA_SIN_PIN");
    } else {
      // Fila D con la salida en curso terminando en ROJO -la pidio otro: el A.A.A del
      // mando, la pantalla o el regreso del radio-. NO se arma el latch y NO se contesta
      // OK, y las dos cosas van juntas: armarlo seria quedarse con el final de una salida
      // que mando otro, y contestar OK seria prometer un ambar que nadie va a encender.
      //
      // La espera no cuesta seguridad: mientras dura esa salida el equipo esta en TODO
      // ROJO, que es mas seguro que el ambar que se pide. Al terminar, el mismo comando
      // cae en la fila A y enciende el ambar de inmediato; por eso el motivo dice REPITA.
      enviarTramaConCrc("$ERR,CMD:AMBAR_EMERGENCIA,DESC:SALIDA_A_ROJO_EN_CURSO_REPITA");
      bluetooth_reportarEvento("APP_BLUETOOTH", "AMBAR_EMERGENCIA_RECHAZADO");
    }
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

  // LA MISMA PUERTA QUE ARRIBA, CON EL PIN PUESTO, Y CON EL MISMO BLOQUE LETRA POR LETRA.
  // El porque de cada fila esta escrito una sola vez, en la puerta sin PIN; repetirlo aqui
  // serian dos explicaciones que alguien tendria que sincronizar. Lo que NO se puede
  // repartir es el codigo: los packs leen el bloque de CADA rama, y una respuesta que
  // viviera en una funcion comun dejaria a los dos instrumentos midiendo un bloque vacio
  // -es N-89, el refactor que apaga el instrumento sin romper un solo test-.
  if (strcmp(accion, "AMBAR_EMERGENCIA") == 0) {
    if (!degradado_gobiernaLuz()) {
      const bool yaEnAmbar = (semaforo_estado() == S_FALLO);
      semaforo_iniciarFallo();
      ambarEmergencia = true;
      if (yaEnAmbar) {
        enviarTramaConCrc("$ACK,CMD:AMBAR_EMERGENCIA,RESULT:YA_EN_AMBAR_LATCH_PUESTO");
      } else {
        enviarTramaConCrc("$ACK,CMD:AMBAR_EMERGENCIA,RESULT:OK");
      }
      bluetooth_reportarEvento("APP_BLUETOOTH", "AMBAR_EMERGENCIA_LOCAL");
    } else if (salidaDegradadoIniciada()) {
      ambarEmergencia = true;
      enviarTramaConCrc("$ACK,CMD:AMBAR_EMERGENCIA,RESULT:SALIENDO_TODO_ROJO");
      bluetooth_reportarEvento("APP_BLUETOOTH", "AMBAR_EMERGENCIA_LOCAL");
    } else if (degradado_rendicionEnCurso()) {
      ambarEmergencia = true;
      enviarTramaConCrc("$ACK,CMD:AMBAR_EMERGENCIA,RESULT:SALIDA_YA_EN_CURSO");
      bluetooth_reportarEvento("APP_BLUETOOTH", "AMBAR_EMERGENCIA_LOCAL");
    } else {
      enviarTramaConCrc("$ERR,CMD:AMBAR_EMERGENCIA,DESC:SALIDA_A_ROJO_EN_CURSO_REPITA");
      bluetooth_reportarEvento("APP_BLUETOOTH", "AMBAR_EMERGENCIA_RECHAZADO");
    }
  } else if (strcmp(accion, "CANCELAR_AMBAR") == 0) {
    // R-3 (31/08) - EL AMBAR DE EMERGENCIA SE REVOCA DESDE LA APP, CON PIN.
    //
    // Hasta hoy la unica salida era subir al gabinete y hacer A.A.A en el mando, y ademas
    // el latch se caia solo -ver la cabecera de ambarEmergencia-. Con la app como
    // superficie de mando, el tecnico que pidio el ambar por telefono no podia deshacer su
    // propia orden sin escalera.
    //
    // PIDE PIN, y aqui la asimetria de mando.cpp se lee al derecho: pedir el ambar es la
    // accion SEGURA y no pide clave; QUITARLO devuelve el cruce a dar verdes, o sea que
    // ABRE PASO, y eso es justo lo que el PIN existe para custodiar.
    //
    // Y no se toca la luz: retirar el latch no ordena nada, solo levanta el veto. Quien
    // vuelve a mover la luz es el Maestro con su siguiente orden. Por eso el RESULT dice
    // RETIRADO y no OK -un OK se leeria como "el ambar ya no esta", y sigue estando hasta
    // que llegue esa orden-.
    if (!ambarEmergencia) {
      // No se finge una revocacion que no ocurrio: si el operario no sabe que no habia
      // nada que quitar, se ira creyendo que desactivo algo que sigue puesto por otra via.
      enviarTramaConCrc("$ERR,CMD:CANCELAR_AMBAR,DESC:NO_HAY_AMBAR_VIGENTE");
    } else {
      ambarEmergencia = false;
      if (mando_ambarLocal()) {
        // El otro latch, el del mando, NO lo puede quitar este comando: los tres vetos de
        // main.cpp son "!mando_ambarLocal() && !bluetooth_ambarEmergencia()", asi que con
        // el del gabinete puesto la luz sigue vetada. Contestar OK a secas mandaria al
        // tecnico a esperar un cambio que no va a llegar hasta que alguien haga A.A.A.
        enviarTramaConCrc("$ACK,CMD:CANCELAR_AMBAR,RESULT:RETIRADO_QUEDA_MANDO");
      } else {
        enviarTramaConCrc("$ACK,CMD:CANCELAR_AMBAR,RESULT:RETIRADO");
      }
      bluetooth_reportarEvento("APP_BLUETOOTH", "AMBAR_EMERGENCIA_REVOCADO");
    }
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
    //
    // EL %c DE MAS NO SE LEE NUNCA: ESTA PARA QUE sscanf DELATE LO QUE SOBRA.
    //
    // sscanf cuenta CAMPOS CONVERTIDOS y no exige que la cadena se acabe. Con dos ordenes
    // en el mismo buffer -"...,18:25:00CMD:AMBAR_EMERGENCIA", que es lo que queda cuando una
    // trama entra a medias y la siguiente se le concatena- esto convertia sus 6 campos,
    // ponia la hora y contestaba "$ACK,CMD:SET_RTC,RESULT:OK", tirando sin aviso la orden
    // que venia detras -que en esta punta es justo la de emergencia-.
    //
    // Con el %c detras la cuenta separa los dos casos: una cadena que se acaba convierte 6
    // -el %c se queda sin nada que leer- y una con basura pegada convierte 7. Se exige el 6
    // EXACTO: ante dos ordenes pegadas la respuesta segura es no ejecutar ninguna y
    // decirlo, no adivinar cual era.
    int y, mo, d, h, mi, s;
    char sobra = 0;
    if (sscanf(accion + 8, "%d-%d-%d,%d:%d:%d%c", &y, &mo, &d, &h, &mi, &s, &sobra) == 6 &&
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

  // SOSTENEDOR DEL AMBAR PEDIDO POR LA APP - LA SEGUNDA MITAD DEL MOLDE (N-106).
  //
  // Es mando.cpp:274 con la otra bandera, y existe por el mismo motivo que alli: la orden
  // de quien la dio tiene que sobrevivir a lo que pase despues. En concreto al todo-rojo
  // de salida del Modo Degradado, que TERMINA EN INACTIVO, NO EN AMBAR: copiada solo la
  // primera mitad del molde -la del despachador-, el todo-rojo de despedida acababa y
  // nadie encendia nada, con lo que el equipo se quedaba en ROJO despues de haber
  // contestado SALIENDO_TODO_ROJO. Eso es lo contrario de lo que se pidio, con el $ACK ya
  // enviado.
  //
  // Se RE-ARMA en vez de encenderse una sola vez porque un ambar que se apaga solo no es
  // un estado seguro, es un parpadeo.
  //
  // Las tres guardas, y ninguna sobra:
  //   !semaforo_senalEnCurso()    mientras hay destellos de confirmacion las luces son de
  //                              la senal; pisarla dejaria al operario sin la cuenta.
  //   !degradado_gobiernaLuz()    durante el todo-rojo de despedida la luz es del modo. Es
  //                              tambien lo que impide que esto pise al Degradado.
  //   estado() != S_FALLO         si la rendicion por 48 h ya encendio el ambar, no se
  //                              reinicia el parpadeo por gusto.
  //
  // Y AQUI YA NO HAY REVOCACION AUTOMATICA. La habia -"si la luz salio de S_FALLO, tira el
  // latch"- y se retira con su porque escrito en la cabecera de ambarEmergencia: durante
  // el todo-rojo de despedida la luz esta en ROJO, asi que mataba el latch milisegundos
  // despues de armarse. Desde R-3 el latch se quita pidiendolo: CANCELAR_AMBAR.
  if (ambarEmergencia && !semaforo_senalEnCurso() && !degradado_gobiernaLuz() &&
      semaforo_estado() != S_FALLO) {
    semaforo_iniciarFallo();
  }

  // 1. Recepción de Comandos desde la App Móvil
  while (SerialBT.available() > 0) {
    char c = (char)SerialBT.read();
    if (c == '\n' || c == '\r') {
      if (btIdxIn > 0) {
        btBufIn[btIdxIn] = '\0';
        procesarComando(btBufIn);
        btIdxIn = 0;
        // A3: DESPUES de despachar, no antes. La linea que llega puede ser el ambar de
        // emergencia, y anteponerle una trama de bitacora le mete el tiempo de cable de
        // esa trama por delante. El registro es diagnostico; la orden, no.
        //
        // Y se anota la linea RECIBIDA, se haya reconocido o no: quien mide el silencio
        // del puerto es el puerto. Una linea que el despachador rechaza por desconocida
        // sigue siendo prueba de que el puente esta vivo y hablando.
        j17RegistrarLinea(ahora);
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

    // N-108 - RF, RTT Y BAT DEJAN DE INVENTARSE. Los tres eran LITERALES: "RF:98%",
    // "RTT:85ms" y "BAT:12.6" salian iguales del equipo estuviera como estuviera, asi que
    // el tablero ensenaba un enlace perfecto y una bateria sana mientras el poste se
    // quedaba sin radio. Un dato fijo con aspecto de medida es peor que un hueco: el
    // hueco hace preguntar, y el numero bueno hace descartar la causa sin mirarla.
    //
    // SE MARCAN, NO SE RETIRAN, y el motivo es que el campo si tiene que existir: el dia
    // que se midan, la app y el Manual 10 no cambian de contrato. Retirarlos ademas
    // dejaria las dos puntas emitiendo tramas distintas, que es lo que
    // documentos_03_trama_status impide con razon -la app es una sola-.
    //
    // POR QUE NINGUNO SE PUEDE MEDIR HOY EN ESTA PUNTA, medido y no supuesto:
    //   RTT   esta punta nunca ORIGINA una peticion. Recibe CMD_PING y contesta
    //         CMD_PONG tras el retardo de cortesia de SFTY-17. No hay ida y vuelta que
    //         cronometrar: lo unico que podria medir es su propio retardo.
    //   RF    no hay ventana de latidos aqui -los latidos los manda el Maestro-, y los
    //         contadores de SFTY-15 no dan una tasa: ver el porque entero en
    //         bluetooth_reportarAlarma(), donde SI se publican, en crudo y sin cociente.
    //   BAT   no hay un solo analogRead() en Esclavo/src ni en Esclavo/include -ni en las
    //         dos carpetas equivalentes del Maestro-. MEDIDO: grep sin coincidencias.
    //         Sin divisor y sin entrada analogica no hay bateria que leer.
    //
    // Y por decision del responsable (31/08) NO se anade RSSI: no se mide potencia. Lo
    // que hay se llega a latidos, y el estado del enlace se indica visualmente -abajo, en
    // el $EVENT, y en el $ALARM de la caida-.
    char payload[128];
    snprintf(payload, sizeof(payload),
             "$STATUS,NODE:ESCLAVO,SERIE:%s,MODO:SUBORDINADO,ESTADO:%s,T:%lu,RF:--,RTT:--,BAT:--,HORA:%s",
             serieTxt, estadoStr, tFaseSeg, horaBuf);

    enviarTramaConCrc(payload);

    // N-108 - EL $EVENT QUE FALTABA ES EL DE LA VUELTA DEL ENLACE, NO EL DE LA CAIDA.
    //
    // La caida YA tiene dueno y ya deja rastro: SFTY-6 la decide en main.cpp y manda el
    // $ALARM, que desde hoy lleva ademas los tres contadores de linea. Lo que no anunciaba
    // nadie era el REGRESO, y por eso en la caja negra una radio que va y viene con la
    // lluvia -el reporte del 27/08- se leia como una sola caida en vez de como doce. Sin
    // la vuelta no se puede contar el numero de cortes, que es el dato que separa "se
    // mojo el conector" de "esta al limite de alcance".
    //
    // AQUI NO HAY RELOJ NI UMBRAL, Y ES DELIBERADO. La primera version media el silencio
    // con SFTY6_SILENCIO_MS y el simulador del puente la tumbo con razon: ese umbral es el
    // del watchdog de la RADIO, y traerlo a este fichero -el del puerto del telefono- es
    // la precondicion de que alguien acabe alimentando uno con el otro. El dia que eso
    // pase, un ESP32 colgado mandaria el cruce a ambar y, peor, un telefono conectado
    // SALVARIA al cruce de una caida de radio real. Son dos silencios y dos instrumentos.
    //
    // Asi que este bloque no decide nada: OBSERVA. La caida la declara quien manda la
    // alarma; aqui solo se anota que ocurrio y se avisa cuando vuelven a entrar tramas.
    // Vale con "cualquier alarma" porque en esta punta hay UNA sola llamada a
    // bluetooth_reportarAlarma() -main.cpp:577, MEDIDO- y es justo la del fallo de RF; si
    // algun dia hay otra, el peor caso es un aviso de vuelta de mas, que se ve.
    {
      static unsigned long validasAnt = 0;
      const unsigned long validas = protocolo_tramasValidas();
      if (enlaceCaidoAnunciado && validas != validasAnt) {
        enlaceCaidoAnunciado = false;
        char det[40];
        snprintf(det, sizeof(det), "RECUPERADO_OK:%lu_RUIDO:%lu",
                 validas, protocolo_tramasDescartadas());
        bluetooth_reportarEvento("ENLACE_RF", det);
      }
      validasAnt = validas;
    }
  }
}
