// ===== Validacion_Automatico/dos_puntas/esclavo/botones.h =====
//
// Sustituto de Esclavo/include/botones.h. MISMAS FIRMAS, para que main.cpp y mando.cpp
// compilen letra por letra. Las definiciones viven en adaptador_esclavo.cpp y no leen
// ningun pin: los pulsos los inyecta el orquestador simulando al operario y al rele del
// mando, que van EN PARALELO sobre el mismo contacto.
#pragma once

#include <Arduino.h>

void botones_setup();
void botones_actualizar();
bool botonArriba();
bool botonAbajo();
bool botonAceptar();
bool botonCancelar();
bool camara_leerPin(uint8_t pin);
