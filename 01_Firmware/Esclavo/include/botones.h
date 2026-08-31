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

// ACEPTAR y CANCELAR YA NO TIENEN SUJETO (31/08/2026). Sus pines -PB14 y PB15, J16 p10 y
// p12- son camaras desde esa fecha, asi que las dos devuelven siempre false. Se conservan
// declaradas a proposito, y el censo de lo que se pierde y de con que se sustituye cada
// caso esta en botones.cpp, junto a las definiciones. No se llaman en vano: mientras
// existan, "git grep botonCancelar" sigue listando de una sola vez todo lo que la
// retirada de los botones C y D dejo sin mando fisico.
bool botonAceptar();
bool botonCancelar();

// ---------------------------------------------------------------------------
// LAS CAMARAS DE J16 - N-97, 31/08/2026. ESTE BLOQUE ES IDENTICO EN LAS DOS PUNTAS.
//
// J16 p10 (PB14) y p12 (PB15) ya no son botones: son entradas de camara de contacto seco,
// INPUT pelado y ACTIVAS EN ALTO (ver pines.h). Viven en botones.cpp, y no en un modulo
// propio, porque J16 TIENE UN SOLO DUENO: el fichero que declara sus pines es el mismo que
// los lee en cada vuelta. Partir el conector entre dos modulos es como un pin acaba con
// dos pinMode() de modos distintos y gana el que corra el ultimo.
//
// Lo que hacen las dos camaras es PEDIR PASO, por la misma puerta que ya existia en las
// dos puntas -demanda_solicitar()-, que es donde esta escrita la diferencia entre pedir y
// decidir (SFTY-27). No encienden nada: solo semaforo.cpp escribe pines de luz.

// Lectura antirrebotada de una entrada de camara. Devuelve true con el contacto CERRADO.
//
// Es publica porque el Modo Inteligente del Maestro tambien lee asi la camara de PB0: una
// sola definicion de "que es una deteccion" para las tres entradas y para las dos puntas.
bool camara_leerPin(uint8_t pin);
