// ===== include/menu.h =====
#pragma once
#include <Arduino.h>

// El enum ModoSistema y sus dos accesores viven en modos.h.
//
// menu_setup() ES LA PUERTA DEL TODO-ROJO DE LAS DOS PUNTAS: llama a coordinador_forzarMenu(),
// y la alcanzan los modos al salir, main.cpp y la app con SET_MODO:MENU. D-44: menu_loop() y
// la navegacion salieron con los botones; en MENU el bucle principal no hace nada mas.
void menu_setup();
