// ===== include/menu.h =====
#pragma once
#include <Arduino.h>

enum ModoSistema { MENU, MODO_MANUAL, MODO_AUTOMATICO, MODO_DEMANDA };

void menu_setup();
void menu_loop();

ModoSistema modoActual_get();
void modoActual_set(ModoSistema m);