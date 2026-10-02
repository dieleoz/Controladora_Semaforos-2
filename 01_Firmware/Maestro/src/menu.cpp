// ===== src/menu.cpp =====
#include "menu.h"
#include "modos.h"
#include "coordinador.h"

// D-44: DE ESTE FICHERO SOLO QUEDA menu_setup(), LA PUERTA DEL TODO-ROJO DE LAS DOS PUNTAS.
// La navegacion (cursor, niveles y el switch que armaba cada modo) salio con los botones:
// cada modo se pide hoy por la app con su SET_MODO. menu_setup() se queda con su nombre
// porque la llaman los modos al salir, main.cpp y el despachador (SET_MODO:MENU), y porque
// la espera de la reanudacion (D-29) y el ambar de arranque (D-40) miran el modo MENU.
void menu_setup() {
  modoActual_set(MENU);
  coordinador_forzarMenu(); // Fuerza Rojo Fijo en Maestro y Esclavo
}
