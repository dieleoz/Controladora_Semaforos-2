// ===== include/mando.h =====
#pragma once
#include <Arduino.h>

// ---------------------------------------------------------------------------
// SFTY-21 — MANDO DE 4 RELES (V8.7)
//
// El operario maneja el equipo desde el suelo con un mando de 4 reles cableados EN
// PARALELO con los botones fisicos (PB9, PB13, PB14, PB15; no hay entradas dedicadas).
// La pantalla esta a 5 m, dentro del gabinete: NO LA VE. Un menu es inservible a
// ciegas -no se sabe donde esta el cursor ni si la pulsacion entro-, asi que se
// reconocen SECUENCIAS y se contesta con las luces.
//
// RESTRICCIONES MEDIDAS EN CAMPO (01/08/2026), no negociables:
//
//   - El rele da UN PULSO POR FLANCO. Sostener el boton 10 s da un solo pulso:
//     LA PULSACION LARGA NO EXISTE, y cualquier diseno que la use es papel mojado.
//   - Cada pulsacion tarda ~2 s en conmutar. Una ventana de 3 s es inviable; hacen
//     falta 12-18 s para 3-4 pulsos.
//   - No hay repeticion automatica: cada pulso exige una pulsacion.
//
// SOLO SE USAN A (Boton 1) Y B (Boton 2). NUNCA C NI D.
//
// El riesgo grave no es el falso positivo: es que la pulsacion llegue cuando el
// sistema esta en un sitio distinto del que el operario cree, y a ciegas eso siempre
// es posible.
//
//    Equipo dejado en el MENU y llega C.C.C desde el piso:
//      1er C -> SELECCIONA lo que tenga el cursor -> arranca un modo que nadie pidio
//      2o  C -> en Modo Manual, C es ROJO FIJO INDEFINIDO
//    Mismo caso con A.A.A: el cursor sube tres veces. No ocurre NADA.
//
// C EJECUTA; A y B solo mueven. A ciegas se usan unicamente los botones cuya
// repeticion accidental es inofensiva.
// ---------------------------------------------------------------------------

enum PulsoMando { MANDO_A = 0, MANDO_B = 1 };

void mando_setup();

// La llama botones.cpp por cada flanco de A o B, antes de que ningun modo consuma el
// boton. No consume nada: solo observa.
void mando_registrarPulso(uint8_t boton);

// Maquina de la confirmacion y de la accion diferida. Va al FINAL del loop principal.
void mando_actualizar();
