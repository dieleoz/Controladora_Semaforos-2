// ===== src/bluetooth.cpp (ESCLAVO) =====
#include "bluetooth.h"
#include "pines.h"
#include "semaforo.h"
#include "protocolo.h"
#include "demanda.h"
#include "reloj.h"
#include "identidad.h"
#include "botones.h"       // D-13: camara_estado(), la fuente del campo CAM:
#include "modo_degradado.h"  // N-106: la salida ordenada y sus dos finales
#include "version_fw.h"    // el sello del binario, inyectado por platformio.ini
#include "deg_auto.h"      // A-15: SET_DEG_AUTO / CONSULTA_DEG_AUTO
#include "respaldo.h"      // A-15: la opcion propia y el APTO oido del Maestro
#include "interruptores.h" // SPEC_8 6: SET_VETO_CAMARA y SET_BARRERA_SERVICIO (D-94)
#include "instalacion.h"   // N-187: SET_CAMARA, SET_PLUMA_RETARDO, CONSULTA_INSTALACION
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

// ---------------------------------------------------------------------------
// D-23 (13/09) - EL POSTE 2 DICE COMO VE EL SU PROPIO ENLACE, Y LO DICE SOLO.
//
// EL HUECO QUE TAPA, medido el 08/09 y escrito en A-14: el $STATUS de esta punta publica
// T:--, RF:-- y RTT:-- como LITERALES, y esos cuatro marcadores NO SE RETIRAN -el porque
// entero esta en el propio $STATUS, abajo-. O sea que el poste 2 nunca dice como ve EL
// el enlace. Y SI lo sabe: protocolo_bytesRecibidos(), protocolo_tramasValidas() y
// protocolo_tramasDescartadas() existen, pero hasta hoy solo salian DENTRO del $ALARM,
// o sea CUANDO EL ENLACE YA SE CAYO. El caso de uso de D-23 es el contrario -"se va a
// degradado cada nada cuando llueve", y el tecnico llega DESPUES-: lo que hace falta es
// el dato con el equipo sano, delante del poste y por Bluetooth directo a esta punta.
//
// POR QUE PERIODICO Y NO "AL CONECTARSE", que es lo que A-14 pedia: EL STM32 NO PUEDE
// SABER QUE HAY UN TELEFONO. El unico que ve al telefono es el ESP32, y traer ese aviso
// seria la TERCERA orden que el accesorio origina hacia el micro, que esp32_05_no_origina
// condiciona POR ESCRITO a una fila de DECISIONES.md. El responsable eligio la otra via
// el 13/09 (D-32 (2)): periodico a cadencia baja, que no toca ninguna barrera y no toca
// el ESP32. El coste esta aceptado y escrito: el tecnico lo ve al cabo de la cadencia,
// no al instante.
//
// Y TAMPOCO SE DEDUCE DEL TRAFICO DE J17, que es la tentacion barata: por J17 entra el
// latido del puente cada LATIDO_MS (AB-1), asi que el puerto habla SOLO. Una linea
// entrando no dice que haya nadie mirando.
//
// POR QUE 30 s, Y POR QUE EL NUMERO NO SALE DE LOS BYTES. El peor segundo de esta punta
// NO depende de la cadencia mientras sea >= 1 s -a cualquier cadencia cabe UN $EVENT de
// mas en el peor segundo, y es el mismo $EVENT-, asi que elegir por bytes seria elegir
// por el numero que no manda. Lo que si depende de la cadencia es el coste SOSTENIDO, y
// a 30 s es una fraccion de punto porcentual del canal. Las dos cifras no se copian
// aqui: las rehace esp32_07_presupuesto_bytes en cada corrida leyendo ESTA constante.
//
// LO QUE SI MANDA ES LA BITACORA DE LA APP, y por eso el numero no baja mas: cada $EVENT
// entra en state.events, que addEvent() de app.js RECORTA A 30 ENTRADAS (MEDIDO el
// 13/09). A 30 s la bitacora de la sesion sigue cubriendo un cuarto de hora; a 10 s se
// vaciaria en cinco minutos y este diagnostico se comeria justo los $ALARM que el tecnico
// vino a leer. Es N-73 por inundacion, el mismo motivo por el que el $EVENT de J17 dejo
// de salir por linea y paso a salir por umbral (AB-1).
//
// NO SE DERIVA DE SFTY6_SILENCIO_MS AUNQUE TIENTE, y el porque esta escrito abajo, en el
// $EVENT de la vuelta del enlace: ese umbral es el del watchdog de la RADIO, y traerlo a
// este fichero -el del puerto del telefono- es la precondicion de que alguien acabe
// alimentando uno con el otro. Son dos silencios y dos instrumentos.
//
// LIMITE DECLARADO: arranca en 0, asi que la primera sale a los DIAG_ENLACE_MS de
// encender y no en el arranque. Es lo correcto y no un efecto lateral: al arrancar el
// STM32 puede no haber nadie escuchando -el ESP32 y el telefono llegan despues- y una
// trama que nadie oye no es un instrumento.
// ---------------------------------------------------------------------------
static const unsigned long DIAG_ENLACE_MS = 30000UL;
static unsigned long tUltimoDiagEnlace = 0;

// D-31 (12/09) - LA MEMORIA DEL ACUSE DEL AVISO DE AMBAR. TRES BANDERAS Y UN INSTANTE,
// porque son tres preguntas distintas y una variable que contesta a dos no puede
// contestar bien a ninguna (CLAUDE.md 8).
//
//   avisoAmbarConfirmado  el Poste 1 ACUSO el aviso vigente. Mientras valga true las
//                         pulsaciones siguientes NO esperan nada, y ese es el corazon de
//                         D-31 (1): el aviso manda al Maestro a MODO_AMBAR y alli tiene
//                         PROHIBIDO transmitir (SFTY-21), asi que un diseno que esperase
//                         acuse en cada pulsacion diria "nadie me oyo" CON LA RADIO SANA
//                         cada vez que alguien pulsa dos veces -que es lo normal en un
//                         poste- y ensenaria al tecnico a ignorar la unica senal que
//                         esto existe para darle.
//   avisoAmbarEsperando   hay un aviso en vuelo con el plazo corriendo. Es lo unico que
//                         bluetooth_loop() mira para cronometrar.
//   avisoAmbarReenvios    reenvios ya gastados, contra AVISO_AMBAR_REINTENTOS. Ese numero
//                         esta ELEGIDO, no derivado -corregido el 12/09-: ver el motivo
//                         en protocolo.h, donde vive la constante.
//   tAvisoAmbar           cuando salio el intento que se esta cronometrando.
//
// QUIEN BORRA LA MEMORIA, Y POR QUE NO PUEDE HABER UN SEGUNDO CAMINO: la rama de
// CANCELAR_AMBAR, en la MISMA sentencia que tira ambarEmergencia. Censado el 12/09 sobre
// este fichero: "ambarEmergencia = false" aparece en UN SOLO sitio, asi que no hay via
// por la que el ambar se retire dejando la memoria puesta. Si la hubiera, la memoria
// mentiria: el aviso siguiente no esperaria acuse cuando SI debe.
static bool avisoAmbarConfirmado = false;
static bool avisoAmbarEsperando = false;
static uint8_t avisoAmbarReenvios = 0;
static unsigned long tAvisoAmbar = 0;

// Se llama JUSTO DESPUES de cada protocolo_enviarPaquete(CMD_AMBAR_ESCLAVO) de las DOS
// puertas del ambar de emergencia, y va en las dos letra por letra como todo el bloque de
// esa rama.
//
// OJO AL INSTRUMENTO, Y SE ESCRIBE PORQUE ES UNA DEBILIDAD CONOCIDA: el comparador de
// esclavo_08 -el que caza que las dos puertas hagan lo mismo- censa las llamadas cuyo
// prefijo sea semaforo_, demanda_, reloj_, config_, coordinador_, degradado_ o
// protocolo_, y este nombre no cae en ninguno. O sea que NINGUN pack vigila que las dos
// puertas la lleven. Lo que lo sostiene es la regla que este fichero ya tiene escrita
// -las dos puertas llevan el mismo bloque, letra por letra- y el bloque H2 del arnes de
// las dos puntas, que ejerce la segunda pulsacion por la puerta que usa la app.
static void avisoAmbarSalio() {
  // Con el Poste 1 ya avisado no se arranca ningun plazo: es la pulsacion repetida, el
  // Maestro esta callado en su ambar y su silencio es lo esperado, no una averia.
  if (avisoAmbarConfirmado) return;
  avisoAmbarEsperando = true;
  avisoAmbarReenvios = 0;
  tAvisoAmbar = millis();
}

void bluetooth_avisoAmbarAcusado() {
  // Sin ambar vigente no se cree nada: ver el porque en bluetooth.h.
  if (!ambarEmergencia) return;
  avisoAmbarEsperando = false;
  avisoAmbarConfirmado = true;
}

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

#include "bluetooth_enlace.inc"     // $ALARM, $EVENT, silencios de J17, hora del ESP32 y reloj

#include "bluetooth_ordenes.inc"    // procesarComando(), el despachador

bool bluetooth_ambarEmergencia() {
  return ambarEmergencia;
}

// ---------------------------------------------------------------------------
// D-32 (1), 13/09 - EL AVISO DEL LIMITE DE 48 h VUELVE A TENER SUPERFICIE, Y ES EL
// AGUJERO MAS GRAVE QUE DEJO LA RETIRADA DEL LCD.
//
// QUE SE ROMPIO, medido y no supuesto. degradado_avisoLimite() y degradado_syncVencida()
// se quedaron SIN NINGUN LLAMADOR en el firmware de esta punta: su unico lector era el
// menu_loop() que salio con la pantalla, y costura_10_funciones_muertas las lleva anotadas
// desde ese commit con el motivo escrito -"NO es una huerfana comoda, es una capacidad
// perdida"-. Ninguna de las dos es una guarda, asi que perder el lector no abrio ningun
// veto: lo que decide sigue decidiendo dentro de modo_degradado.cpp. LO QUE SE PERDIO ES
// EL AVISO.
//
// Y EL AVISO NO ES COSMETICA: sin el, el cruce se va a ambar sin que nadie lo haya visto
// venir. La caida la ejecuta degradado_actualizar() con `if (syncVencidaLatch && ...)
// iniciarSalida(true)`, y ese camino NO emite $EVENT ni $ALARM: la rendicion por las 48 h
// es HOY completamente muda. El equipo se degrada solo y el tecnico se entera al llegar.
// El apartado de modo_degradado.cpp que fija el plazo lo dice con todas las letras: "el
// estado seguro no puede depender de que alguien se acuerde, pero avisar con margen evita
// que el cruce se degrade sin que hubiera falta".
//
// EL PLAZO NO SE INVENTA: SE CONSERVA EL QUE YA HABIA. Esta funcion no compara contra
// ningun numero propio -no hay una sola constante de tiempo aqui-: pregunta a
// degradado_avisoLimite(), que es quien compara contra AVISO_SIN_SYNC_MS dentro de
// modo_degradado.cpp. Son 40 h de las 48, o sea que avisa con las ULTIMAS 8 h de margen,
// "un turno completo" por el motivo que alli esta escrito. Traer el numero a este fichero
// seria el gemelo en otro sitio que este repositorio ya sabe como acaba (CLAUDE.md 14):
// el dia que alguien mueva el plazo, el aviso dejaria de significar lo que dice.
// ⚠️ EL MAESTRO NO PUEDE HACER ESTO HOY y por eso esta mitad va sola: alli el plazo
// -AVISO_LIMITE_MS, 44 h de 48, las ultimas 4- vivia DENTRO de la funcion de dibujo del
// LCD y modo_degradado.h no publica ni el aviso ni la antiguedad. Falta un getter, y
// anadirlo no es de este fichero.
//
// POR QUE POR FLANCO Y NO PERIODICO A SECAS, que era la forma barata. El $EVENT periodico
// de D-23 que vive mas abajo lleva escrito su propio techo: cada $EVENT entra en
// state.events, que addEvent() de app.js RECORTA A 30 ENTRADAS, y un segundo periodico a
// la misma cadencia PARTE POR DOS la ventana de bitacora que el tecnico tiene para
// encontrar el $ALARM -de un cuarto de hora a siete minutos-. Es N-73 por inundacion, el
// mismo motivo por el que el $EVENT de J17 paso de salir por linea a salir por umbral
// (AB-1). Asi que en reposo esta funcion NO PONE UN SOLO BYTE EN EL CABLE:
//
//   - por FLANCO del trio (sabe fechar / aviso / vencido) el aviso llega EN EL INSTANTE
//     en que se arma, que ademas es mejor que esperar a la cadencia;
//   - y mientras el aviso SIGUE armado se repite a DIAG_ENLACE_MS, porque el tecnico que
//     llega DESPUES del flanco -que es el caso de uso entero de D-23- no vio aquel.
//
// O sea: coste cero cuando no hay nada que decir, y la segunda trama solo en las horas en
// que de verdad hace falta. El peor segundo del presupuesto SI la cuenta igual, entera y
// sin prorratear, porque un presupuesto que descuente un emisor que puede disparar mide
// otro equipo; lo recalcula esp32_07_presupuesto_bytes leyendo ESTA cadencia.
//
// LA CADENCIA ES LA MISMA CONSTANTE, NO UNA SEGUNDA: DIAG_ENLACE_MS. No es pereza -es lo
// contrario de un numero nuevo que nadie recalcularia-, y ademas los dos periodicos caen
// en la MISMA vuelta del bucle por construccion, que es lo que hace que el peor segundo
// se pueda calcular en vez de depender de la suerte.
//
// EL ORIGEN ES DEGRADADO Y NO UNO NUEVO: es el MISMO sujeto que el $EVENT de
// SALTO_DE_HORA_POR_ROJO que ya emite modo_degradado.cpp, y la app los distingue por el
// DETALLE, igual que hace con los dos ENLACE_RF.
//
// LOS MARCADORES SIGNIFICAN LO DE SIEMPRE (CLAUDE.md 10, y la tabla de SPEC 4 §5):
// "--" es "todavia no lo se" y NUNCA un cero de relleno. degradado_msDesdeSync() devuelve
// 0 cuando no hubo sync jamas, y publicar ese 0 diria "sincronizado hace un momento",
// que es exactamente lo contrario de la verdad; por eso se pregunta antes a
// degradado_huboSync(). Aqui no hay "!": el valor no tiene cota contra la que salirse
// -ver el buffer-, y una guarda que ningun dato puede disparar no es una comprobacion,
// es adorno (CLAUDE.md 6.2).
//
// LA COTA DE syncTxt[] ES POR TIPO, Y SE ESCRIBE POR QUE NO PUEDE SER OTRA: la antiguedad
// es un CONTADOR LIBRE -sigue creciendo despues del limite; el latch no la para-, asi que
// no hay constante que la acote, igual que los tres contadores de SFTY-15 de aqui al lado.
// El tope de %lu son las diez cifras de un unsigned long: 10 + la "h" = 11 caracteres +
// NUL = 12. Que el valor REAL no pueda pasar de 1.193 h -2^32 ms partido por 3.600.000-
// no se usa para estrechar el buffer: eso seria acotar por el rango, que es justo lo que
// N-154 corrigio.
//
// LA COTA DE det[], por BUFFER y no por rango, con el separador en ESPACIOS y no en comas
// por el mismo motivo que el $EVENT de D-23 -_camposNmea() de la app parte por ',' y una
// coma dentro del DETALLE convierte lo que va detras en campos sueltos que el pintor de
// $EVENT no mira-:
//
//   parte fija   21   "SYNC:" + " AVISO:" + " VENCIDA:"
//   SYNC         11   syncTxt[12]
//   AVISO         2   literal "SI"/"NO"
//   VENCIDA       2   literal "SI"/"NO"
//
//   21 + 15 = 36 caracteres + NUL = 37 B, que es lo que guarda det[37]. Que el $EVENT
//   ENTERO quepa luego en su payload no se afirma aqui: lo recalcula
//   esp32_07_presupuesto_bytes leyendo este snprintf y el del emisor.
// ---------------------------------------------------------------------------
static unsigned long tUltimoDiagDegradado = 0;
static int8_t diagDegradadoAnt = -1;   // -1: aun no se ha publicado ninguno

static void diagDegradadoPublicar(unsigned long ahora) {
  const bool sabeFechar = degradado_huboSync();
  const bool aviso      = degradado_avisoLimite();
  const bool vencida    = degradado_syncVencida();

  // El trio en un solo entero para poder comparar flancos sin tres banderas estaticas.
  // No contesta a dos preguntas distintas: contesta a UNA -"que se publico la ultima
  // vez"- y por eso puede ser una sola variable.
  const int8_t ahoraEstado = (int8_t)((sabeFechar ? 4 : 0) | (aviso ? 2 : 0) | (vencida ? 1 : 0));

  // LA PRIMERA VUELTA SOLO ANOTA, Y ES EL MISMO LIMITE DECLARADO QUE LLEVA ESCRITO EL
  // PERIODICO DE D-23: al arrancar el STM32 puede no haber nadie escuchando -el ESP32 y
  // el telefono llegan despues- y una trama que nadie oye no es un instrumento. No se
  // pierde nada: si el aviso ya viene armado de un arranque anterior, la repeticion de
  // abajo lo publica a los DIAG_ENLACE_MS, que es cuando ya puede oirse.
  if (diagDegradadoAnt < 0) {
    diagDegradadoAnt = ahoraEstado;
    return;
  }

  const bool cambio = (ahoraEstado != diagDegradadoAnt);
  const bool tocaRepetir = aviso && (ahora - tUltimoDiagDegradado >= DIAG_ENLACE_MS);
  if (!cambio && !tocaRepetir) return;

  diagDegradadoAnt = ahoraEstado;
  tUltimoDiagDegradado = ahora;

  char syncTxt[12];
  if (!sabeFechar) {
    strncpy(syncTxt, "--", sizeof(syncTxt));
  } else {
    snprintf(syncTxt, sizeof(syncTxt), "%luh", degradado_msDesdeSync() / 3600000UL);
  }

  char det[37];
  snprintf(det, sizeof(det), "SYNC:%s AVISO:%s VENCIDA:%s",
           syncTxt, aviso ? "SI" : "NO", vencida ? "SI" : "NO");
  bluetooth_reportarEvento("DEGRADADO", det);
}

#include "bluetooth_bucle.inc"     // bluetooth_loop(): puerto, telemetria y diagnosticos
