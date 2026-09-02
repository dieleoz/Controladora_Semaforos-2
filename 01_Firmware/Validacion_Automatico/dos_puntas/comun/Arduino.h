// ===== Validacion_Automatico/dos_puntas/comun/Arduino.h =====
//
// Sustituto minimo de Arduino.h para compilar el firmware de UNA punta dentro de una
// DLL. Es hermano del de Validacion_Automatico/Arduino.h y trae el mismo contrato -el
// reloj lo mueve el arnes, digitalWrite() GRABA en vez de tirar el dato-, con dos
// diferencias que el arnes de dos puntas necesita y aquel no:
//
//   - digitalRead(). El bucle real del Esclavo lee CAM_DEMANDA_PIN en cada vuelta
//     (main.cpp, N-67: activo en ALTO). Sin esta funcion no enlaza; devolviendo un
//     valor fijo escrito aqui, el arnes no podria ejercer nunca la demanda por camara.
//     Se lee del mismo array que se escribe, y el orquestador lo mueve por la API.
//
//   - delay(). setup() del Esclavo tiene un delay(2000) deliberado (N-22). Aqui NO se
//     duerme: se AVANZA el reloj simulado, que es lo que ese delay significa para todo
//     lo que venga despues. Dejarlo como no-op haria que el arranque de esta punta
//     ocurriese en un instante que el equipo real nunca ve.
//
// CADA DLL TIENE SU PROPIA COPIA DE ESTAS VARIABLES, y ese es el mecanismo entero del
// arnes: el enlazador de Windows resuelve arnes_pines[] dentro de cada modulo, asi que
// el Maestro y el Esclavo escriben en dos arrays distintos aunque el codigo fuente que
// los escribe sea el mismo fichero compilado dos veces. Comparar "verde en las dos a
// la vez" es entonces leer dos arrays en el mismo tick.
#pragma once

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define OUTPUT 1
#define INPUT 0
#define INPUT_PULLUP 2
#define LOW 0
#define HIGH 1

inline void pinMode(int, int) {}

extern int arnes_pines[64];
extern unsigned long arnes_escrituras;

// N-96 SE MIDE CON ESTE CONTADOR, NO CON EL NIVEL DEL PIN.
//
// ROJO_PEATON, VERDE_PEATON y el BUZZER estan declarados en pines.h y muertos en las
// dos puntas. Comprobarlo mirando el NIVEL no valdria: arnes_pines[] arranca en LOW y
// un digitalWrite(pin, LOW) lo dejaria igual, asi que un pin escrito con cero seria
// indistinguible de un pin que nadie toca. Lo que hay que contar son las ESCRITURAS.
//
// Y sirve en las dos direcciones: tambien exige que los SEIS pines vivos SI se
// escriban. Una regla de seguridad que enumera sujetos tiene que comprobar que cada
// sujeto existe, no solo que nadie la rodea.
extern unsigned long arnes_toques[64];

inline void digitalWrite(int pin, int valor) {
  if (pin >= 0 && pin < 64) { arnes_pines[pin] = valor; arnes_toques[pin]++; }
  arnes_escrituras++;
}

// Las entradas viven en su propio array: si compartieran el de salidas, un
// digitalWrite() a un pin de entrada -que el firmware no hace, pero que un defecto
// podria introducir- se leeria a si mismo y la lectura confirmaria la escritura.
extern int arnes_entradas[64];
inline int digitalRead(int pin) {
  return (pin >= 0 && pin < 64) ? arnes_entradas[pin] : LOW;
}

extern unsigned long arnes_millis_valor;   // lo mueve el orquestador, nunca solo
inline unsigned long millis() { return arnes_millis_valor; }

// Ver la cabecera: adelantar el reloj es lo que un delay() hace de verdad. Que este
// arnes no bloquee no lo exime de contar el tiempo.
inline void delay(unsigned long ms) { arnes_millis_valor += ms; }
