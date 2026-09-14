// ===== Validacion_Automatico/dos_puntas/esclavo/menu.h =====
//
// Sustituto de Esclavo/include/menu.h. DOS funciones, las que de verdad se llaman
// desde main.cpp: menu_setup() y menu_loop().
//
// D-30 (14/09): aqui se declaraba tambien menu_estaAbierto(), la puerta que INHIBIA
// las secuencias del mando de reles. Su unico lector era secuenciasInhibidas() de
// Esclavo/src/mando.cpp; retirado el mando no la llama nadie, asi que sale de la
// cabecera sustituta igual que ha salido de la real.
#pragma once

#include <Arduino.h>

void menu_setup();
void menu_loop();
