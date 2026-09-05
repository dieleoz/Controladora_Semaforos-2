// ===== 01_Firmware/ESP32_Expansion/src/puente.cpp =====

#include "puente.h"
#include "contrato.h"
#include "trama.h"
#include "enlace_stm32.h"
#include "transporte_app.h"
#include "despachador.h"
#include "reloj_ds3231.h"
#include <stdio.h>
#include <string.h>

static char deApp[BUF_ENTRADA_APP];
static size_t idxApp = 0;
static bool appDesbordada = false;

static char deSTM32[BUF_ENTRADA_STM32];
static size_t idxSTM32 = 0;
static bool stmDesbordada = false;

static unsigned long descartadasCrc = 0;
static unsigned long descartadasLargo = 0;
static unsigned long tramasAlaApp = 0;

void puente_setup() {
  idxApp = 0;
  idxSTM32 = 0;
  appDesbordada = false;
  stmDesbordada = false;

  // 6.4 - SILENCIO NO ES ORDEN: aqui NO se manda nada, ni a la app ni al STM32.
  //
  // Un enlace serie en reposo esta en alto y un pin flotante tambien puede leerse alto:
  // desde el STM32, "el puente no dice nada" y "el puente no esta" no son
  // distinguibles, y por eso el puente no puede tener un estado de arranque que
  // signifique algo. Un saludo en setup() seria una orden que nadie pidio entrando por
  // el mismo camino por el que entran las que si se piden.
}

void puente_emitirPropio(const char* payload) {
  char trama[BUF_SALIDA_APP];
  size_t n = trama_componer(trama, sizeof(trama), payload);
  if (n == 0) return;
  if (transporte_escribir(trama, n) == n) tramasAlaApp++;
}

// ---------------------------------------------------------------------------
// SENTIDO APP -> STM32.  VERBATIM.
//
// El unico camino por el que se escribe en Serial2, y cada byte que sale de aqui vino
// de transporte_leer(). No hay literales: enlace_escribirLinea() recibe el buffer.
//
// AQUI NO SE VALIDA NINGUN CHECKSUM, Y NO ES UN OLVIDO. Es la correccion de un
// dato falso, y se deja escrita porque la version anterior de este fichero SI validaba
// y habria descartado el 100% de los comandos reales.
//
// 18_Especificacion_Firmware_ESP32.md 3.4 afirma -con la palabra MEDIDO encima- que
// "el *XX que la app anade, nmea_parser.js:20-21, formatearComando()" llega al strcmp,
// y de ahi ordena que el puente valide '$', '*' y el XOR-8 y descarte lo que no case.
// Los dos extremos son falsos. MEDIDO sobre la app, 31/08:
//
//   05_Funcional/App_Semaforo/app.js:199-207   el emisor VIVO es
//       enviarComandoFirmware(), y compone:  CMD:PIN:1234:SET_MODO:AUTO\r\n
//       Sin '$' delante y SIN '*XX' detras.
//
//   formatearComando() NO EXISTE. Se llama formatearTrama(), vive en js/nmea_parser.js,
//       y ese modulo esta cargado por index.html:589 y NO TIENE UN SOLO LLAMADOR en la
//       app -solo en tests-. Su generarComando() produce "$CMD:...*XX", que con el '$'
//       delante no casa con ningun strcmp del despachador.
//
// Si este puente exigiera '$' y checksum, descartaria TODOS los comandos que la app
// manda de verdad y contestaria un $ERR propio a cada uno. Es la prueba muerta al
// reves: un instrumento que no aprueba nada valido.
//
// Lo que SI se hace, porque la app tampoco lo hace y el STM32 no puede: medir la
// longitud contra los 63 utiles antes de reenviar. Lo demas viaja tal cual.
//
// Y NO SE ANADE CHECKSUM AL SALIR. El STM32 compara la linea ENTERA con strcmp: un
// "*4F" pegado detras haria que no casara ningun comando, y todos caerian en
// $ERR,CMD:DESCONOCIDO.
// ---------------------------------------------------------------------------
static void desdeLaApp() {
  int vueltas = 0;

  // W-4: tope de iteraciones, no solo condicion de disponibilidad. Es la mitad de la
  // defensa que W-3 monta por el otro lado: con un flujo continuo de basura, un bucle
  // "mientras haya bytes" no termina nunca, y ahi dentro no se alimenta al perro.
  while (transporte_disponible() > 0 && vueltas < PUENTE_MAX_ITER) {
    vueltas++;
    int c = transporte_leer();
    if (c < 0) break;

    if (c == '\r' || c == '\n') {
      // E-3: una linea vacia no es nada. El segundo byte de un "\r\n" cae aqui con el
      // indice a cero y no cuenta como comando entregado.
      if (idxApp == 0) {
        appDesbordada = false;
        continue;
      }
      deApp[idxApp] = '\0';
      size_t largoUtil = idxApp;
      idxApp = 0;

      if (appDesbordada) {
        // E-2 POR NUESTRO LADO, Y ES LA UNICA COMPROBACION QUE ESTE SENTIDO HACE.
        //
        // El STM32 truncaria y compararia la linea recortada como si estuviera entera:
        // una orden a medias que casa por accidente con otra mas corta. Aqui se
        // descarta y se DICE, con un $ERR marcado como del puente para que el tecnico
        // no diagnostique el poste equivocado.
        descartadasLargo++;
        puente_emitirPropio("$ERR,NODE:PUENTE,CMD:DESCONOCIDO,DESC:LINEA_DEMASIADO_LARGA");
        appDesbordada = false;
        continue;
      }

      // ---------------------------------------------------------------------
      // LA UNICA LINEA QUE NO CRUZA EL CABLE, Y LA REGLA NO ES UNA LISTA DE COMANDOS
      //
      // Todo lo demas sigue VERBATIM: los mismos bytes que mando la app, sin quitar
      // nada y sin anadir nada. SET_RTC incluido -se atiende aqui Y sigue viaje-,
      // porque es una orden que el STM32 tambien conoce.
      //
      // Lo que este `if` se queda es lo que va dirigido al PUENTE Y A NADIE MAS. El
      // criterio esta en despachador.h con su medida: reenviar CMD:LEER_RTC hace que
      // las dos puntas contesten $ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO -medido sobre
      // su bluetooth.cpp compilado-, o sea un rechazo ROJO en la app acusando al
      // operario de una clave que no tecleo, cada vez que pregunta la hora. Es el mismo
      // defecto por el que "$LATIDO" tiene rama muda en el Maestro.
      //
      // NO ES "el puente conociendo el protocolo del STM32": es el puente conociendo EL
      // SUYO. El protocolo del equipo puede crecer todo lo que quiera sin tocar este
      // fichero; lo que no puede es que una orden este a la vez en las dos listas, y eso
      // lo recalcula un pack leyendo los dos despachadores del STM32 en cada corrida.
      //
      // Y `propagada` nace en false por lo mismo que la barrera del reloj nace abajo: si
      // no se escribio en el cable, no se propago. La rama que atiende esta linea no lo
      // mira -no hay cable que cruzar-, pero un true de cortesia aqui seria un dato
      // falso esperando a que alguien lo lea.
      bool propagada = false;
      if (!despachador_esParaElPuente(deApp)) {
        propagada = (enlace_escribirLinea(deApp, largoUtil) > 0);
      }

      // El reloj del puente se atiende DESPUES de haber reenviado -cuando se reenvia-,
      // porque `propagada` es parte de la respuesta y no se puede saber antes.
      despachador_observar(deApp, propagada);
      continue;
    }

    if (idxApp < (size_t)TRAMA_MAX_UTIL) {
      deApp[idxApp++] = (char)c;
    } else {
      // Se MARCA en vez de recortar callando. Que el STM32 lo haga en silencio es
      // precisamente el defecto que este puente esta obligado a no repetir.
      appDesbordada = true;
    }
  }
}

// ---------------------------------------------------------------------------
// N-145 - EL SELLO DE HORA. AQUI EL PUENTE DEJA DE SER VERBATIM, Y SE ESCRIBE POR QUE.
//
// EL DEFECTO, MEDIDO. El campo HORA: de $STATUS lo rellena el STM32 -Maestro/src/
// bluetooth.cpp:843-848 y Esclavo/src/bluetooth.cpp-, que es el micro que NO TIENE
// RELOJ: su cristal Y2 esta CONFIRMADO MUERTO en hardware (N-17). El unico DS3231 del
// equipo cuelga de ESTE modulo. En la cinta de tramas del 04/09 (21:10-21:18) las
// tramas salen TODAS asi:
//
//   $STATUS,NODE:MAESTRO,SERIE:179DB0,...,BAT:--,HORA:--:--:--*30
//
// El equipo tiene la hora en la mano y publica un hueco porque quien compone la trama
// no es quien tiene el reloj.
//
// 🔴 POR QUE ESTO ROMPE B-5 -"entra entera y sale entera"- Y CUANTO EXACTAMENTE.
//
// B-5 sigue en pie para todo lo demas: no se parte, no se une, no se filtra, no se
// reordena y NO SE AÑADE NI SE QUITA UN BYTE. Lo unico que cambia es que un HUECO que
// el STM32 declaro puede salir relleno. Las tres cotas del cambio, para que nadie
// tenga que deducirlas:
//
//   1. SOLO SE TOCA EL HUECO. La busqueda es el literal "HORA:--:--:--". Si el STM32
//      puso una hora -el dia que tenga cristal-, esta funcion no encuentra nada y no
//      hace nada. El puente NO ARBITRA entre dos relojes: rellena el que falta y calla
//      cuando ya hay uno. Asi el arreglo se apaga solo cuando deje de hacer falta, en
//      vez de convertirse en una segunda fuente de verdad peleando con la primera.
//   2. LA LONGITUD NO CAMBIA. "--:--:--" son 8 caracteres y "%02d:%02d:%02d" son 8.
//      El "*XX" tambien mide lo mismo antes y despues. O sea que `largo` sigue siendo
//      valido, el terminador se repone donde estaba y el presupuesto de bytes de
//      esp32_07 no se mueve ni un byte. No es una casualidad afortunada: es la razon
//      por la que se sella EN SITIO en vez de recomponer la trama.
//   3. NO SE INVENTA NUNCA. Si el DS3231 no contesta, si el oscilador se paro, si el
//      registro esta en modo 12 h, si una escritura quedo a medias o si los registros
//      no componen una fecha, reloj_leer() devuelve false y EL HUECO SE QUEDA COMO
//      ESTA. Sale "--:--:--", que es lo que hay que decir.
//
// LO QUE CUESTA N-144 SI ESTO SE ESCRIBE MAL, y por eso el punto 3 no es prolijidad:
// aquel dia el equipo se declaro EN HORA con el reloj parado en ceros y publico
// HORA:00:00:00. No es medianoche: es un contador que no avanza con la bandera de
// "tengo hora" puesta, y de esa bandera cuelga la autorizacion del Modo Degradado. Un
// cero que parece una hora es peor que un hueco, porque el hueco no engana a nadie.
// Aqui el equivalente seria rellenar con lo que el chip devuelva sin preguntar a la
// barrera: un DS3231 sin pila entrega una fecha PERFECTAMENTE FORMADA y falsa.
//
// EL CHECKSUM SE RECALCULA, Y NO ES OPCIONAL. La app SI valida el XOR-8 en la bajada
// -parseNmeaTelemetry() -> juzgarTrama() -> NMEAParser.validarTrama()-, asi que una
// trama con la hora cambiada y el checksum viejo no se pintaria: se descartaria en
// silencio y el tablero se quedaria congelado. El sintoma seria "el puente se comio la
// telemetria", que manda a diagnosticar el cable.
//
// 🛑 EL RESIDUAL QUE ESTO DEJA, ESCRITO EN VEZ DE DISIMULADO: desde el $STATUS solo, la
// app NO PUEDE SABER cual de los dos relojes sello la hora. Hoy siempre es este, porque
// el otro no existe, pero la trama no lo dice. No se le ha añadido marca -ni un campo
// nuevo a $STATUS ni un campo al parte de arranque- porque las dos cuestan contrato y
// buffer en sitios que hoy no fallan, y porque la cura de verdad es la otra mitad de
// N-145: que el STM32 deje de publicar un campo de un reloj que no tiene. Mientras esa
// mitad no entre, quien lea la hora en la app esta leyendo el DS3231 del puente y no
// tiene como saberlo desde ahi. Se escribe aqui y va al informe: una causa declarada
// vale mas que un mecanismo inventado.
// ---------------------------------------------------------------------------
#define HUECO_HORA "HORA:--:--:--"

static bool sellarHoraSiFaltaba(char* linea) {
  // TODO LO QUE PUEDE FALLAR SE MIRA ANTES DE ESCRIBIR UN SOLO BYTE.
  //
  // El orden es la mitad del arreglo: si el checksum se buscara despues de haber
  // pisado la hora, un fallo aqui dejaria la trama con la hora nueva y el checksum
  // viejo -bien formada, con el sello puesto, y descartada por la app sin que nada lo
  // dijera-. Es la escritura a medias de R-5 aplicada a una trama en vez de a un bus.
  char* hueco = strstr(linea, HUECO_HORA);
  if (hueco == NULL) return false;

  // trama_valida() ya exigio que exista, y aun asi se comprueba: esta funcion MUTA la
  // trama, y una precondicion que se da por buena es como se cuelan los defectos en el
  // camino que "no puede ocurrir".
  char* ast = strchr(linea, '*');
  if (ast == NULL) return false;

  // LA BARRERA DECIDE, Y SE PREGUNTA POR reloj_leer(), NO POR EL CHIP.
  //
  // reloj_leer() lleva reloj_enHora() delante y no tiene variante "damela igual": si la
  // hora no es fiable, aqui no llega. Es el unico camino por el que la hora del DS3231
  // sale hacia la app, y por eso pasa por la misma puerta que el $ACK de SET_RTC.
  FechaHora fh;
  if (!reloj_leer(&fh)) return false;

  // Exactamente ocho, o no se toca nada. reloj_leer() ya valido los rangos por barrido
  // -0..23, 0..59, 0..59-, asi que %02d no puede desbordar; el guardia esta para que el
  // dia que alguien afloje aquella validacion esto no escriba 9 bytes en un hueco de 8.
  char hhmmss[9];
  int n = snprintf(hhmmss, sizeof(hhmmss), "%02d:%02d:%02d", fh.hora, fh.minuto, fh.segundo);
  if (n != 8) return false;

  // Se sella UN hueco: el de la trama, que es uno. Se copian 8 bytes sin nulo detras
  // -strcpy pondria el '\0' encima del caracter siguiente y partiria la trama-.
  // El desplazamiento sale del propio literal -largo total menos los 8 del hueco-, no
  // de un 5 tecleado: el dia que la clave deje de llamarse "HORA:" esto sigue apuntando
  // donde debe en vez de escribir cinco caracteres mas alla de donde creia.
  memcpy(hueco + (sizeof(HUECO_HORA) - 1) - 8, hhmmss, 8);

  // Y AHORA el checksum, sobre la trama ya sellada. trama_checksum() para en el '*',
  // asi que lee exactamente el mismo tramo que el STM32 conto al emitirla.
  //
  // Se llama HEXA y no HEX porque `HEX` YA ES UNA MACRO del framework -Print.h:30, "#define
  // HEX 16"-, asi que la version con el nombre corto ni siquiera compila. Lo caza el
  // compilador, no un instrumento, y se anota para que nadie lo "arregle" de vuelta.
  static const char HEXA[] = "0123456789ABCDEF";
  uint8_t crc = trama_checksum(linea + 1);
  ast[1] = HEXA[(crc >> 4) & 0x0F];
  ast[2] = HEXA[crc & 0x0F];
  return true;
}

// ---------------------------------------------------------------------------
// SENTIDO STM32 -> APP.  AQUI SI SE VALIDA EL CHECKSUM.
//
// Y es asimetrico a proposito, porque las dos direcciones no son iguales:
//
//   app -> STM32   la app NO pone checksum (MEDIDO en app.js:199-207). No hay nada que
//                  validar, y exigirlo descartaria el 100% del trafico real.
//   STM32 -> app   el equipo SI lo pone -enviarTramaConCrc(), Maestro:43-48- y desde el
//                  31/08 la app TAMBIEN lo comprueba: parseNmeaTelemetry() empieza por
//                  juzgarTrama(), que llama a NMEAParser.validarTrama() y vuelve sin
//                  pintar si no casa. Aqui decia lo contrario, MEDIDO, y era cierto
//                  cuando se escribio.
//
// Consecuencia medida el 01/09, corregida: en la BAJADA hay DOS validadores del mismo
// XOR-8 -este y el de la app- y juzgan IGUAL en los siete casos frontera. Este bucle no
// estrena la comprobacion: la duplica. Una trama con CRC malo es ruido de cable, y el
// ruido no sube al telefono.
//
// LO QUE SIGUE SIN CUBRIR, y es la mitad que importa: EN LA SUBIDA NO HAY CHECKSUM EN
// NINGUN SITIO. Ninguna de las dos puntas llama a calcularChecksum() en RECEPCION -esta
// definida y solo se usa al emitir-. Un bit cambiado DENTRO del parametro de SET_TIEMPOS
// o SET_RTC sigue casando con el strncmp del prefijo, y el equipo obedece con los
// valores mutilados.
//
// Y ojo al efecto de esta asimetria en el diagnostico: aqui la trama mala se descarta EN
// SILENCIO, asi que con puente la averia se lee como un HUECO -"no llegaba nada"-,
// mientras que sin puente -la topologia de campo de hoy- la app la caza y la NOMBRA:
// CHECKSUM. Dos diagnosticos distintos de la misma averia segun haya puente o no.
//
// Lo que NO se hace es mirar el prefijo: se valida FORMATO, no contenido.
// ---------------------------------------------------------------------------
static void desdeElEquipo() {
  int vueltas = 0;

  while (enlace_disponible() > 0 && vueltas < PUENTE_MAX_ITER) {
    vueltas++;
    int c = enlace_leer();
    if (c < 0) break;

    if (c == '\r' || c == '\n') {
      if (idxSTM32 == 0) {
        stmDesbordada = false;
        continue;
      }
      deSTM32[idxSTM32] = '\0';
      size_t largo = idxSTM32;
      idxSTM32 = 0;

      if (stmDesbordada) {
        stmDesbordada = false;
        descartadasLargo++;
        continue;
      }

      if (!trama_valida(deSTM32)) {
        descartadasCrc++;
        continue;
      }

      // NO HAY FILTRO POR PREFIJO, Y ES DELIBERADO. Son cinco -$STATUS, $ACK, $ERR,
      // $ALARM y $EVENT-, el Maestro emite $EVENT desde catorce ramas y la app lo
      // consume como bitacora del equipo: un puente que filtrara por una lista de
      // cuatro se comeria el registro entero, y esa es la perdida silenciosa que costo
      // N-73 -un registro que cuatro documentos describen y que nadie puede mirar
      // cuando hay que diagnosticar un fallo de campo-.
      //
      // Se retransmite toda trama bien formada, se conozca o no. Asi el protocolo crece
      // sin recompilar el puente.

      // El rotulo se aprende de paso. Observa; no altera la trama ni decide si sube.
      transporte_aprenderRotulo(deSTM32);

      // N-145: EL UNICO SITIO DONDE ESTE PUENTE MODIFICA UNA TRAMA DEL EQUIPO.
      //
      // Va DESPUES de trama_valida() a proposito: sellar primero obligaria a recalcular
      // el checksum de una trama que podria ser ruido de cable, y entonces el puente
      // estaria FABRICANDO tramas validas a partir de basura -exactamente lo que el
      // sentido de vuelta existe para no hacer-. Primero se comprueba que la trama es
      // del equipo; solo entonces se le rellena el hueco que el equipo declaro.
      //
      // Y va ANTES de componer `salida` porque sella EN SITIO y sin cambiar la
      // longitud: `largo` sigue valiendo lo mismo despues.
      sellarHoraSiFaltaba(deSTM32);

      // B-5: entra entera y sale entera -salvo el hueco de HORA, que es la unica
      // excepcion y esta acotada y razonada sobre sellarHoraSiFaltaba()-. Se reponen los
      // DOS bytes del terminador que el STM32 puso -S-1: siempre "\r\n"- para que el
      // parser de la app reciba lo mismo que salio del equipo, byte a byte.
      char salida[BUF_ENTRADA_STM32 + 2];
      memcpy(salida, deSTM32, largo);
      salida[largo] = '\r';
      salida[largo + 1] = '\n';
      if (transporte_escribir(salida, largo + 2) == largo + 2) tramasAlaApp++;
      continue;
    }

    if (idxSTM32 < (size_t)(BUF_ENTRADA_STM32 - 1)) {
      deSTM32[idxSTM32++] = (char)c;
    } else {
      stmDesbordada = true;
    }
  }
}

void puente_bombear() {
  // Las DOS direcciones en cada vuelta. El reset del watchdog lo da el bucle exterior
  // -main.cpp- despues de que esta funcion vuelva, que es lo unico que mide progreso.
  desdeLaApp();
  desdeElEquipo();
}

unsigned long puente_descartadasPorCrc()   { return descartadasCrc; }
unsigned long puente_descartadasPorLargo() { return descartadasLargo; }
unsigned long puente_tramasAlaApp()        { return tramasAlaApp; }
