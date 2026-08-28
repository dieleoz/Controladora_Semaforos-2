// ===== include/modo_alcance.h =====
#pragma once
#include <Arduino.h>

// Modo PRUEBA DE ALCANCE (V8.1).
//
// Herramienta de campo: muestra en pantalla la calidad del enlace de radio y el
// tiempo de respuesta, derivados del latido de 3 s que el Maestro ya emite.
// Permite al operario mover el equipo y determinar hasta donde hay cobertura
// real, en lugar de estimarlo a ojo.
//
// Seguridad: mantiene el mismo estado que el Menu Principal (Rojo Fijo en ambos
// semaforos con enlace; Amarillo Intermitente si se pierde). No inicia ciclos.

void modoAlcance_setup();
void modoAlcance_loop();
