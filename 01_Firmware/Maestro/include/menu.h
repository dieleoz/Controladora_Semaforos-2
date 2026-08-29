// ===== include/menu.h =====
#pragma once
#include <Arduino.h>

// El enum ModoSistema y sus dos accesores ya NO viven aqui: estan en modos.h, porque
// el modo del equipo lo consultan y lo escriben tres ficheros que no dibujan nada
// -main.cpp, mando.cpp y bluetooth.cpp-. Esta cabecera declara solo la pantalla.
// Quien necesite el modo incluye "modos.h"; quien necesite las dos cosas, las dos.

void menu_setup();
void menu_loop();
