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
