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

// D-13 (05/09): bluetooth.cpp REAL llama a esta para rellenar el campo CAM: del
// $STATUS, asi que la firma tiene que estar o no compila. La definicion esta en
// arnes_puente.cpp y devuelve el estado de ARRANQUE del vigilante, no un OK.
const char* camara_estado();
