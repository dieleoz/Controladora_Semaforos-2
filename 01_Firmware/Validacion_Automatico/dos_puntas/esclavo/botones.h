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
// D-33 (14/09/2026): semaforo.cpp REAL llama a esta antes de dejar bajar la pluma, asi
// que la firma tiene que estar o no compila. NO es un detalle de arnes: es la prueba de
// que el veto no se puede quitar en silencio -quitarlo rompe el ENLACE, no se queda
// callado-. La definicion esta en el adaptador y devuelve false: en este banco no hay
// camaras cableadas, o sea que la pluma sigue a la luz con su retardo y nada mas.
bool camara_presenciaJ16();
