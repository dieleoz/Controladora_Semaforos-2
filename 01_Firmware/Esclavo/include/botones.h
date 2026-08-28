// ===== include/botones.h (ESCLAVO) =====
#pragma once
#include <Arduino.h>

// ---------------------------------------------------------------------------
// N-16 — Botonera del Esclavo.
//
// Esta cabecera existia desde hace meses copiada del Maestro y SIN ningun .cpp
// que la implementara: prometia cuatro botones en un firmware que no leia
// ninguno. Desde el 01/08/2026 la implementa src/botones.cpp, portado tal cual
// del Maestro, y deja de mentir.
//
// El antirrebote y la ventana de flanco son IGUALES a los del Maestro a
// proposito. Los cuatro pulsadores estan en paralelo con el mando de reles del
// operario (ver SFTY-21), que entrega pulsos de ~2 s sin repeticion: dos puntas
// con criterios distintos de que es "una pulsacion" harian que la misma orden se
// leyera distinto en cada gabinete.
// ---------------------------------------------------------------------------

void botones_setup();

// ---------------------------------------------------------------------------
// SFTY-21 — La deteccion de flancos se hace UNA VEZ POR ITERACION, aqui, y no
// dentro de cada botonX() como hacia la version portada del Maestro en N-16.
//
// El motivo es el mando de reles, y es el mismo que obligo a cambiarlo en el
// Maestro: sus secuencias (A.A.A, B.B.B, A.B.A.B) tienen que verse SIEMPRE, sin
// que importe que la pantalla en la que este el equipo lea o no ese boton. Con la
// deteccion metida dentro de botonArriba(), una pantalla que no consultase el
// Boton 1 hacia que esas pulsaciones NO EXISTIERAN PARA NADIE: el operario habria
// pulsado tres veces desde el suelo y el equipo no habria contado ninguna.
//
// En el Esclavo el agujero era todavia mas ancho que en el Maestro. Durante el
// segundo y medio de bienvenida menu_loop() ni siquiera corre, asi que sin esta
// llamada nadie leeria los pulsadores en todo ese rato.
//
// Efecto secundario bueno: antes, si una pantalla dejaba de consultar un boton
// durante un rato, la pulsacion quedaba latente y se disparaba en cuanto alguien
// preguntaba, aunque hubiera ocurrido mucho antes. Ahora el flanco vive solo la
// iteracion en la que ocurre.
//
// DEBE llamarse al principio del loop principal, antes de atender nada mas.
void botones_actualizar();

// Estas siguen CONSUMIENDO el flanco: leerlo lo gasta.
bool botonArriba();
bool botonAbajo();
bool botonAceptar();
bool botonCancelar();
