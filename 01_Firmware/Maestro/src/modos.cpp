// ===== src/modos.cpp =====
//
// El estado del sistema, fuera del fichero de la pantalla (ver modos.h).
//
// El arranque en MENU no es una eleccion de este fichero: es el mismo valor que tenia
// la variable cuando vivia en menu.cpp, y menu_setup() lo vuelve a fijar cada vez que
// el equipo regresa al menu.
#include "modos.h"

static ModoSistema modoActual = MENU;

ModoSistema modoActual_get() { return modoActual; }
void modoActual_set(ModoSistema m) { modoActual = m; }
