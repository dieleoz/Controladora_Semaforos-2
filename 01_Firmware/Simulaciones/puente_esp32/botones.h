// ===== Validacion_Automatico/botones.h =====
// Sustituto de botones.h para compilar modo_automatico.cpp en el PC.
//
// MISMAS FIRMAS que include/botones.h, para que modo_automatico.cpp compile letra
// por letra. Las definiciones (en arnes_automatico.cpp) no leen ningun pin: las
// mueve el arnes con arnes_pulsar_*(), simulando al operario.
#pragma once

void botones_setup();
void botones_actualizar();

// D-13 (05/09): bluetooth.cpp REAL llama a esta para rellenar el campo CAM: del
// $STATUS, asi que la firma tiene que estar o no compila. La definicion esta en
// arnes_puente.cpp y devuelve el estado de ARRANQUE del vigilante, no un OK.
const char* camara_estado();
// D-33 (14/09/2026): semaforo.cpp REAL llama a esta antes de dejar bajar la pluma, asi
// que la firma tiene que estar o no compila. La definicion esta en arnes_puente.cpp y
// devuelve false: aqui no hay camaras cableadas.
bool camara_presenciaJ16(int cam = -1);   // N-187: firma de include/botones.h
void camaras_sembrar();                   // N-187: la llama instalacion.cpp
bool camara_mira(int i);              // D-166/D-169: firma de include/botones.h
bool camara_malaRetienePluma();
char camara_letra(int i);
