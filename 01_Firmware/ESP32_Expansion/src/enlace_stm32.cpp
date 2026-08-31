// ===== 01_Firmware/ESP32_Expansion/src/enlace_stm32.cpp =====
//
// 🔴 ESTE ES EL UNICO FICHERO DEL PROYECTO QUE NOMBRA Serial2.
//
// Es la misma disciplina que la barrera de salidas del STM32, y se puede COMPROBAR:
// el pack esp32_05_no_origina censa el directorio src/ entero y exige que ningun otro
// fichero mencione el puerto. Una barrera que hay que respetar en N sitios se rompe en
// el sitio N+1; con una sola puerta, "el puente no origina" se lee en 60 lineas.

#include "enlace_stm32.h"
#include "contrato.h"

static HardwareSerial haciaSTM32(ENLACE_UART);
static bool abierto = false;

static unsigned long bytesEscritos = 0;
static unsigned long lineasEscritas = 0;
static unsigned long lineasRechazadas = 0;

void enlace_setup() {
  // El 8N1 va EXPLICITO. En el STM32 esa eleccion es un valor por defecto del framework
  // -SerialBT.begin(9600) con un solo argumento- que no se puede leer de ningun sitio;
  // este literal es lo unico escrito que ata las dos puntas.
  haciaSTM32.begin(ENLACE_BAUDIO, ENLACE_FORMATO, ENLACE_PIN_RX, ENLACE_PIN_TX);
  abierto = true;

  // 6.4 - SILENCIO NO ES ORDEN, y su otra mitad: aqui NO se manda nada.
  //
  // Un enlace serie en reposo esta en alto, y un pin flotante tambien puede leerse
  // alto: desde el STM32, "el puente no dice nada" y "el puente no esta" no son
  // distinguibles. Por eso el puente no tiene modo por defecto que aplicar al arrancar
  // ni saluda: un saludo del puente seria una orden que nadie pidio, entrando por el
  // mismo camino por el que entran las que si se piden.
}

int enlace_disponible() {
  return abierto ? haciaSTM32.available() : 0;
}

int enlace_leer() {
  return abierto ? haciaSTM32.read() : -1;
}

size_t enlace_escribirLinea(const char* datos, size_t n) {
  if (!abierto || datos == NULL || n == 0) {
    lineasRechazadas++;
    return 0;
  }

  // E-2 SE COMPRUEBA AQUI PORQUE ALLI NO SE COMPRUEBA.
  //
  // El bucle receptor del STM32 -btBufIn[64] con la guarda btIdxIn < sizeof-1- no
  // aborta la linea larga ni protesta: deja de guardar caracteres y entrega al
  // despachador una linea TRUNCADA que se compara con strcmp como si estuviera entera.
  // Una orden recortada que casa por accidente con otra mas corta es un accidente, asi
  // que lo que no cabe no sale de aqui, y se cuenta.
  if (n > (size_t)TRAMA_MAX_UTIL) {
    lineasRechazadas++;
    return 0;
  }

  // B-5: UNA trama, UNA escritura. El receptor delimita por '\r'/'\n', asi que partir
  // una linea en dos write() la entrega como dos lineas y las dos son basura. Se arma
  // entera en pila y se vuelca de una vez.
  //
  // E-1: el terminador lo pone esta funcion. Sin el, el despachador del STM32 no
  // dispara NUNCA: la linea se queda muda en btBufIn y el siguiente comando se le pega
  // detras. Lo que no se hace es anadirselo a un byte suelto que llego sin terminador
  // -eso seria compensar una propiedad del STM32 que el puente no debe compensar-.
  char linea[TRAMA_MAX_UTIL + 3];
  memcpy(linea, datos, n);
  linea[n] = '\r';
  linea[n + 1] = '\n';

  size_t puestos = haciaSTM32.write((const uint8_t*)linea, n + 2);
  if (puestos != n + 2) {
    // P-3: ni un byte se descarta en silencio. Una escritura corta deja al STM32 con
    // media linea en el buffer que el siguiente terminador convertira en un comando
    // desconocido; el contador es lo unico que permite verlo desde fuera.
    lineasRechazadas++;
    bytesEscritos += puestos;
    return 0;
  }

  bytesEscritos += puestos;
  lineasEscritas++;
  return puestos;
}

unsigned long enlace_bytesEscritos()    { return bytesEscritos; }
unsigned long enlace_lineasEscritas()   { return lineasEscritas; }
unsigned long enlace_lineasRechazadas() { return lineasRechazadas; }
