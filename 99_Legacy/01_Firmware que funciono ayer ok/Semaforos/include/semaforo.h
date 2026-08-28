// ===== include/semaforo.h =====
#pragma once
#include <Arduino.h>

enum EstadoSemaforo { S_ROJO, S_VERDE, S_AMARILLO, S_FALLO, S_ESPERA };

void semaforo_setup();
void semaforo_apagarTodo();
void semaforo_forzarRojo();
void semaforo_forzarVerde();
void semaforo_iniciarTransicionAVerde(); // Ámbar 2s antes de Verde (Rojo->Ámbar->Verde)
void semaforo_iniciarFallo();
void semaforo_iniciarEspera(); // SFTY-1: Rojo sostenido (enlazado, sin configuración activa)
void semaforo_actualizar();
bool semaforo_estable();

EstadoSemaforo semaforo_estado();
const char* semaforo_nombreEstado();