// ===== Validacion_Respaldo/Arduino.h =====
// Sustituto minimo de Arduino.h para compilar respaldo.cpp en el PC.
// respaldo.h lo incluye solo por los tipos enteros; respaldo.cpp no llama a
// ninguna funcion de Arduino. Se deja tan corto como eso: un sustituto que
// ofreciera mas de lo que el fuente usa invitaria a que el arnes creciera hacia
// emular la tarjeta, y lo que hace falta es lo contrario.
#pragma once

#include <stdint.h>
#include <stddef.h>
