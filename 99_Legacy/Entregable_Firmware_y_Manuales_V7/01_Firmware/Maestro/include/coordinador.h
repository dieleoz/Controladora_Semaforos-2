// ===== include/coordinador.h =====
#pragma once
#include <Arduino.h>

void coordinador_setup();
bool coordinador_intentarHandshake();
void coordinador_configurar(unsigned long tiempoEstaticoMs, unsigned long minRojoMs, unsigned long minVerdeMs);
void coordinador_pedirCambio();
void coordinador_actualizar();
void coordinador_actualizar_background();
bool coordinador_listoParaContar();
bool coordinador_comunicacionPerdida();
void coordinador_reiniciarConexion();
const char* coordinador_nombreEstadoMaster();