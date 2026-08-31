// ===== 01_Firmware/ESP32_Expansion/src/puente.cpp =====

#include "puente.h"
#include "contrato.h"
#include "trama.h"
#include "enlace_stm32.h"
#include "transporte_app.h"
#include "despachador.h"
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
// 🔴 AQUI NO SE VALIDA NINGUN CHECKSUM, Y NO ES UN OLVIDO. Es la correccion de un
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

      // VERBATIM. Los mismos bytes que mando la app, sin quitar nada y sin anadir nada.
      bool propagada = (enlace_escribirLinea(deApp, largoUtil) > 0);

      // El reloj del puente se atiende DESPUES de haber reenviado, y sin poder vetar el
      // reenvio: una rama que pudiera quedarse una trama seria el puente conociendo
      // comandos, que es el diseno que obliga a recompilarlo cada vez que el protocolo
      // crece.
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
// SENTIDO STM32 -> APP.  AQUI SI SE VALIDA EL CHECKSUM.
//
// Y es asimetrico a proposito, porque las dos direcciones no son iguales:
//
//   app -> STM32   la app NO pone checksum (MEDIDO en app.js:199-207). No hay nada que
//                  validar, y exigirlo descartaria el 100% del trafico real.
//   STM32 -> app   el equipo SI lo pone -enviarTramaConCrc(), Maestro:43-48- y ademas
//                  MEDIDO que la app NO lo comprueba: parseNmeaTelemetry() hace
//                  line.split('*')[0] (app.js:734) y lo tira sin mirarlo.
//
// 🔴 Consecuencia medida: hoy NO HAY UN SOLO CHECKSUM VERIFICADO EN TODA LA CADENA, en
// ninguna direccion. Este bucle es el primero que lo hace. Una trama con CRC malo es
// ruido de cable, y el ruido no sube al telefono.
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

      // 🔴 NO HAY FILTRO POR PREFIJO, Y ES DELIBERADO. Son cinco -$STATUS, $ACK, $ERR,
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

      // B-5: entra entera y sale entera. Se reponen los DOS bytes del terminador que el
      // STM32 puso -S-1: siempre "\r\n"- para que el parser de la app reciba lo mismo
      // que salio del equipo, byte a byte.
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
