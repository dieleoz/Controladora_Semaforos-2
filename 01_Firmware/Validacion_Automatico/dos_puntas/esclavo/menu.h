// ===== Validacion_Automatico/dos_puntas/esclavo/menu.h =====
//
// Sustituto de Esclavo/include/menu.h. Tres funciones, las que de verdad se llaman
// desde main.cpp (menu_setup, menu_loop) y desde mando.cpp (menu_estaAbierto).
//
// menu_estaAbierto() NO se cablea a false y punto: es la puerta que INHIBE las
// secuencias del mando de reles (SFTY-21), y un sustituto que siempre dijera "cerrado"
// dejaria esa rama sin ejercer. El orquestador la mueve por la API, de modo que las dos
// ramas -mando vivo y mando inhibido- se pueden recorrer.
#pragma once

#include <Arduino.h>

void menu_setup();
void menu_loop();
bool menu_estaAbierto();
