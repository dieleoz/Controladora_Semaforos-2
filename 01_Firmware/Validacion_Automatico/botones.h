// ===== Validacion_Automatico/botones.h =====
// Sustituto de botones.h para compilar modo_automatico.cpp en el PC.
//
// MISMAS FIRMAS que include/botones.h, para que modo_automatico.cpp compile letra
// por letra. Las definiciones (en arnes_automatico.cpp) no leen ningun pin: las
// mueve el arnes con arnes_pulsar_*(), simulando al operario.
#pragma once

void botones_setup();
void botones_actualizar();
bool botonArriba();
bool botonAbajo();
bool botonAceptar();
bool botonCancelar();

// A-12 (05/09): camara_leerPin() se suma para poder compilar modo_inteligente.cpp.
//
// Es la MISMA firma de include/botones.h. La definicion vive en
// arnes_automatico.cpp y no lee ningun pin: la mueve el arnes, que es quien decide
// si "hay coche" en cada escenario. Lo que se mide no es el antirrebote -eso es
// cosa de botones.cpp y de su propio pack-, es que la camara pueda ALARGAR una fase
// y no ACORTARLA.
#include <stdint.h>

bool camara_leerPin(uint8_t pin);
