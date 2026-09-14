// ===== include/menu.h =====
#pragma once
#include <Arduino.h>

// El enum ModoSistema y sus dos accesores ya NO viven aqui: estan en modos.h, porque
// el modo del equipo lo consultan y lo escriben tres ficheros que no dibujan nada
// -main.cpp, mando.cpp y bluetooth.cpp-. Quien necesite el modo incluye "modos.h".
//
// D-32 (1), 13/09/2026: SALE EL LCD, Y ESTE FICHERO SE QUEDA. Aqui ponia "esta
// cabecera declara solo la pantalla", y eso dejo de ser cierto hoy: lo unico que
// quedaba de pantalla se ha ido y lo que queda GOBIERNA.
//
// menu_setup() ES LA PUERTA DEL TODO-ROJO DE LAS DOS PUNTAS. Llama a
// coordinador_forzarMenu(), que fuerza Rojo Fijo en Maestro y Esclavo, y tiene DOCE
// llamadores: los siete modos cuando se sale de ellos, main.cpp en el arranque y en su
// cambio de modo, y -la que importa- el despachador de Bluetooth, que la alcanza HOY
// desde la app con SET_MODO:MENU. Borrar este fichero con la pantalla habria borrado
// ese todo-rojo sin que ningun instrumento lo dijera.
//
// menu_loop() conserva la NAVEGACION -el cursor y el switch que arma cada modo- y ha
// perdido el dibujo. Que hoy no pueda alcanzarse (botonAceptar() es `return false;`
// desde el 31/08) esta medido y escrito en menu.cpp, junto al motivo de no retirarla
// en este commit.
void menu_setup();
void menu_loop();
