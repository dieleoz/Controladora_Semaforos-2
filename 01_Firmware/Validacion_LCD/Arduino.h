// Sustituto minimo de Arduino.h para compilar el codigo de pantalla en el PC.
// Solo cubre lo que usa lcd.cpp; no pretende emular Arduino.
#pragma once

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define OUTPUT 1
#define INPUT 0
#define LOW 0
#define HIGH 1

inline void pinMode(int, int) {}
inline void digitalWrite(int, int) {}

// El arnes del Maestro solo DIBUJA: no hay nada que medir en el tiempo y un
// millis() constante le basta.
//
// El arnes del Esclavo si necesita reloj, y no por gusto: enlaza menu.cpp, que
// mide con millis() el regreso automatico al listado a los 90 s -lo que sostiene
// la inhibicion del mando de reles-, y modo_degradado.cpp, que mide con el el
// limite duro de 48 h. Con el tiempo congelado ninguna de las dos cosas se podria
// comprobar jamas, y son justo las que no se pueden probar a mano en un gabinete.
//
// Se separa con un define en vez de cambiarlo para todos para que el arnes del
// Maestro compile EXACTAMENTE el mismo codigo que hasta hoy: el preprocesador
// toma la misma rama de siempre y sus 83 comprobaciones no pueden verse afectadas.
#ifdef ARNES_MILLIS_CONTROLADO
extern unsigned long arnes_millis_valor;   // lo mueve el arnes, paso a paso
inline unsigned long millis() { return arnes_millis_valor; }
#else
inline unsigned long millis() { return 0; }
#endif
