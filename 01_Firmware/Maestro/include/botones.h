// ===== include/botones.h =====
#pragma once
#include <Arduino.h>

void botones_setup();

// ---------------------------------------------------------------------------
// SFTY-21 (V8.7): la deteccion de flancos se hace UNA VEZ POR ITERACION, aqui, y no
// dentro de cada botonX() como antes.
//
// El motivo es el mando de reles. Sus secuencias (A.A.A, B.B.B, A.B.A.B) tienen que
// verse SIEMPRE, con independencia de que la pantalla en la que este el equipo lea o
// no ese boton. Con la deteccion metida dentro de botonArriba(), un modo que no
// consultase el Boton 1 hacia que sus pulsaciones no existieran para nadie: el
// operario habria pulsado tres veces desde el suelo y el equipo no habria contado
// ninguna.
//
// Efecto secundario bueno: antes, si un modo no consultaba un boton durante un rato,
// la pulsacion quedaba latente y se disparaba en cuanto alguien preguntaba, aunque
// hubiera ocurrido mucho antes. Ahora el flanco vive solo la iteracion en la que
// ocurre.
//
// DEBE llamarse al principio del loop principal, antes de atender ningun modo.
void botones_actualizar();

// Estas siguen consumiendo el flanco, igual que antes: leerlo lo gasta.
bool botonArriba();
bool botonAbajo();
bool botonAceptar();
bool botonCancelar();
