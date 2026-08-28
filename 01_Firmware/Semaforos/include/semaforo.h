// ===== include/semaforo.h =====
#pragma once
#include <Arduino.h>

enum EstadoSemaforo { S_ROJO, S_VERDE, S_AMARILLO, S_FALLO };

void semaforo_setup();
void semaforo_apagarTodo();
void semaforo_forzarRojo();
void semaforo_forzarVerde();
void semaforo_iniciarTransicionAVerde();
void semaforo_iniciarTransicionARojo();
void semaforo_toggle();
void semaforo_iniciarFallo();
void semaforo_actualizar();
bool semaforo_estable();

EstadoSemaforo semaforo_estado();
const char* semaforo_nombreEstado();