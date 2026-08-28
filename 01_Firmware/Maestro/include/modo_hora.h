// ===== include/modo_hora.h =====
#pragma once
#include <Arduino.h>

// ---------------------------------------------------------------------------
// SFTY-18 — Pantalla de ajuste del reloj.
//
// Es la unica via para poner en hora el RTC, y por tanto el requisito previo de
// todo lo que dependa de la hora: la operacion intermitente nocturna (SFTY-20) y
// el Modo Degradado (SFTY-21).
//
// NO arranca ciclos ni toca las luces: el coordinador mantiene el estado que
// tuviera (Rojo Fijo con enlace, Ambar sin el), igual que PRUEBA ALCANCE.
// ---------------------------------------------------------------------------

void modo_hora_setup();
void modo_hora_loop();
