// ===== include/menu.h (ESCLAVO) =====
#pragma once
#include <Arduino.h>

// ---------------------------------------------------------------------------
// D-32 (1), 13/09/2026 — SALE EL LCD DEL FIRMWARE.
//
// D-17.bis (28/08) habia retirado la pantalla DEL EQUIPO y conservado el codigo a
// proposito; D-30 propuso llevarse ademas el mando, y D-32 (1) recorta esa decision a
// una sola mitad: sale el LCD y el mando A/B/C/D SE QUEDA. Asi que de la interfaz del
// Esclavo ya no queda nada: ni pantalla fisica, ni framebuffer, ni navegacion.
//
// LO QUE ESTA CABECERA DECLARABA Y YA NO EXISTE: un menu de dos entradas -ESTADO y
// MODO DEGRADADO- con su cursor, su confirmacion de entrada al Degradado y su regreso
// automatico al listado. El censo de por que se puede retirar sin dejar ningun camino
// de gobierno sin puerta esta en menu.cpp, symbol a symbol.
//
// LO QUE SIGUE SIENDO CIERTO Y NO SE TOCA: EL ESCLAVO NO OFRECE MODOS DE OPERACION, Y
// NO ES UN OLVIDO. Quien decide el ciclo es el Maestro. Dar a las dos puntas la
// capacidad de decidir a la vez deja el cruce con dos cerebros discutiendo por radio
// quien tiene razon. Entrar y salir del Modo Degradado en esta punta se pide hoy por
// la app (D-18) o por las secuencias del mando de reles, y las dos vias pasan por
// modo_degradado.cpp, que es quien lleva las condiciones y las transiciones.
// ---------------------------------------------------------------------------

// Las dos se conservan porque main.cpp las llama -menu_setup() al terminar la ventana
// de bienvenida, menu_loop() en cada vuelta- y porque son el hueco donde volveria a
// montarse una interfaz. Hoy no hacen nada; el motivo esta escrito en menu.cpp.
void menu_setup();
void menu_loop();

// ---------------------------------------------------------------------------
// D-30 (14/09): AQUI SE DECLARABA menu_estaAbierto(), Y SE FUE CON EL MANDO.
//
// Era la tercera funcion de este header y la unica que gobernaba algo: la puerta que
// inhibia las secuencias del mando de reles cuando habia una pantalla abierta por
// debajo del listado. Su UNICO lector era secuenciasInhibidas() de mando.cpp.
//
// Retirado el mando (D-30) no la llama nadie. Se retira en vez de dejarla huerfana
// porque una funcion sin llamador no es una barrera: es un adorno con forma de
// barrera, y los documentos acaban anunciandola como existente (CLAUDE.md 6.1).
// Devolvia `false` fijo desde D-32, asi que al irse no se abre ningun veto: lo que
// desaparece es un veto que ya no vetaba, sobre un mando que ya no existe.
// ---------------------------------------------------------------------------
