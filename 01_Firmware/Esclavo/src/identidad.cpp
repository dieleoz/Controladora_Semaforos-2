// ===== src/identidad.cpp =====
#include "identidad.h"

// ESTE FICHERO Y SU .h DEBEN SER IDENTICOS EN MAESTRO Y ESCLAVO.

// El UID de 96 bits vive en tres palabras consecutivas de solo lectura. UID_BASE lo
// define el CMSIS del F103 (0x1FFFF7E8); no se escribe la direccion a mano aqui para
// que un cambio de familia de micro no deje esto leyendo memoria que no es.
#ifndef UID_BASE
#error "UID_BASE no definido: sin UID de silicio no hay identidad que derivar"
#endif

// POR QUE UN MEZCLADO DE HORNER Y NO UN XOR DE LAS TRES PALABRAS.
//
// El UID del F1 no es aleatorio: la palabra 0 lleva las coordenadas X/Y del chip en la
// oblea, y las otras dos el lote y el numero de oblea. Dos chips vecinos de la misma
// oblea difieren SOLO en los bits bajos de la primera palabra, y dos del mismo lote
// comparten casi todo. Un XOR plano de las tres palabras cancela justo lo que tienen
// en comun y concentra las colisiones entre equipos que, encima, es probable que se
// compren juntos y acaben en la misma carretera.
//
// El multiplicador impar arrastra cada bit hacia los altos en cada vuelta, asi que una
// diferencia de un solo bit en la entrada cambia medio codigo de salida. Es el mismo
// criterio que ya usa calcularSuma() en respaldo.cpp.
static const uint32_t MEZCLA = 2654435761UL;   // 2^32 / razon aurea, impar

uint32_t identidad_serie() {
  const volatile uint32_t* uid = (const volatile uint32_t*)UID_BASE;

  uint32_t h = 0;
  for (uint8_t i = 0; i < 3; i++) {
    h = (h + uid[i]) * MEZCLA;
    // EL XORSHIFT NO ES ADORNO, y lo midio el pack identidad_01.
    //
    // La multiplicacion solo propaga acarreo HACIA ARRIBA: un cambio en el bit 0 de la
    // entrada mueve toda la palabra, pero un cambio en el bit 31 mueve solo el bit 31.
    // Promediado sobre las 32 posiciones, un bit de entrada movia 5,8 de los 24 bits de
    // salida cuando el ideal son 12: el mezclado copiaba la mitad alta en vez de
    // repartirla, y las series se habrian agrupado por lote de fabricacion.
    //
    // Plegar los altos sobre los bajos lo arregla. Medido: 5,78 -> 11,84 de 12.
    h ^= h >> 15;
  }

  // POR QUE 24 BITS Y NO 16, que fue la primera version.
  //
  // Con 16 bits el pack identidad_01 midio 88 colisiones entre 4.096 chips vecinos de
  // una misma oblea. Lo revelador es que la cota del cumpleanos para 4.096 codigos en
  // 65.536 huecos son 128: el mezclado iba MEJOR que el azar, asi que lo estrecho era
  // el ANCHO, no la funcion. Ninguna funcion puede batir esa cota, y ajustar la prueba
  // hasta que pasara habria sido tapar el defecto en vez de arreglarlo.
  //
  //     16 bits,   256 equipos -> 0,5    colisiones esperadas
  //     24 bits,   256 equipos -> 0,002
  //     24 bits, 4.096 equipos -> 0,5
  //
  // 24 bits cuesta un byte mas en la trama de radio -despreciable frente a ciclos de
  // 15 s- y deja el codigo en 6 hexadecimales, que un tecnico sigue pudiendo comparar
  // de un vistazo entre dos pantallas.
  //
  // Se pliegan los 32 bits a 24 con XOR del byte alto sobre los bajos, en vez de
  // truncar: truncar tiraria 8 bits de entropia ya repartida.
  uint32_t serie = (h ^ (h >> 24)) & 0xFFFFFFUL;

  // 0x000000 esta reservado para "Esclavo sin matricular". Un equipo que se presentase
  // con ese codigo se leeria como sin adoptar, que es justo lo contrario de tener
  // identidad. Se desplaza al 0x000001 y se acepta el sesgo: es un caso entre 16,7 M.
  if (serie == 0x000000UL) {
    serie = 0x000001UL;
  }
  return serie;
}

void identidad_texto(char* dst) {
  // OJO CON EL NOMBRE: Arduino define HEX como la constante 16 para Serial.print(x,
  // HEX), asi que una tabla llamada HEX se convierte en 16[...] y no compila. Costo
  // tres errores identicos de "invalid types int[int]" hasta ver de donde salian.
  static const char HEXDIG[] = "0123456789ABCDEF";
  const uint32_t s = identidad_serie();
  for (int8_t i = 0; i < 6; i++) {
    dst[i] = HEXDIG[(s >> (20 - 4 * i)) & 0xF];
  }
  dst[6] = '\0';
}
