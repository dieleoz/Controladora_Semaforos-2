// ===== Simulaciones/puente_esp32/uid_arnes.h =====
//
// UID_BASE PARA COMPILAR identidad.cpp REAL EN EL PC.
//
// identidad.cpp lee el UID de 96 bits del silicio en tres palabras consecutivas, y
// se niega a compilar si UID_BASE no viene definido -"#error: sin UID de silicio no
// hay identidad que derivar"-. En la tarjeta lo define el CMSIS del F103; aqui no hay
// CMSIS.
//
// La salida barata habria sido sustituir identidad_texto() por un literal. No se hace
// porque entonces el SERIE: que viaja en cada $STATUS -y que el ESP32 tiene que
// transportar entero- saldria de este arnes y no del firmware, y el tamano de la
// trama es justo lo que §3.3 vigila. Se le da al fuente REAL un bloque de memoria
// legible con un UID de mentira, y el mezclado de Horner que deriva la serie es el de
// verdad.
//
// Se inyecta con -include y -DUID_BASE=arnes_uid, sin tocar identidad.cpp.
#pragma once

#include <stdint.h>

// Tres palabras, como el F103. Los valores imitan la forma del UID real -dos chips
// vecinos difieren solo en los bits bajos de la primera palabra- para que el hash
// trabaje sobre una entrada con la misma pinta que la de campo.
extern const uint32_t arnes_uid[3];
