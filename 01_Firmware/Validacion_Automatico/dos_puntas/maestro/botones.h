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
// D-33 (14/09/2026): semaforo.cpp REAL llama a esta antes de dejar bajar la pluma, asi
// que la firma tiene que estar o no compila. NO es un detalle de arnes: es la prueba de
// que el veto no se puede quitar en silencio -quitarlo rompe el ENLACE, no se queda
// callado-. La definicion esta en el adaptador y devuelve false: en este banco no hay
// camaras cableadas, o sea que la pluma sigue a la luz con su retardo y nada mas.
bool camara_presenciaJ16();
