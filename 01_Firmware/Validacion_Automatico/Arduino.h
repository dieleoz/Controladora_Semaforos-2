// ===== Validacion_Automatico/Arduino.h =====
// Sustituto minimo de Arduino.h para compilar coordinador.cpp, semaforo.cpp y
// modo_automatico.cpp en el PC. Mismo patron que Validacion_LCD/Arduino.h y
// Validacion_Respaldo/Arduino.h: no emula la placa, solo cubre lo que estos tres
// fuentes usan.
//
// EL RELOJ ES SIEMPRE CONTROLADO POR EL ARNES, sin rama condicional. A diferencia
// de Validacion_LCD -que tiene un arnes que solo DIBUJA y otro que mide tiempo-,
// aqui el ciclo automatico ES tiempo: todo-rojo, amarillo de 4s, timeout de ACK a
// 3.5s y orfandad a 12s se miden avanzando millis() a voluntad. Un millis()
// congelado no podria ejercer ni una sola de esas reglas.
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

// digitalWrite() GRABA en vez de tirar el dato al vacio. No es un capricho: es lo
// que permite comprobar SFTY-2 (Rojo y Verde nunca a la vez) y el ritmo del
// parpadeo de FALLO sobre lo que semaforo.cpp REALMENTE escribe, en vez de fiarse
// de la logica que decide escribir. Un pin fuera de rango se ignora: este arnes no
// necesita mas de los seis de semaforo.cpp (ver pines.h de este directorio).
extern int arnes_pines[64];
extern unsigned long arnes_escrituras;
inline void digitalWrite(int pin, int valor) {
  if (pin >= 0 && pin < 64) arnes_pines[pin] = valor;
  arnes_escrituras++;
}

// digitalRead() LEE EL MISMO arnes_pines[] EN EL QUE ESCRIBE digitalWrite(). Desde que
// botones.cpp REAL se compila aqui (D-13 fase 1), las entradas de camara son pines de ese
// mismo array: el escenario cierra el contacto poniendo el pin a HIGH y camara_leerPin()
// -la funcion de verdad, con su antirrebote- lo lee. Un pin que el escenario no ha tocado
// vale 0, que es exactamente lo que da el pull-down de 10K de la placa en una bornera
// vacia; ese es el caso que hay que poder ejercer.
inline int digitalRead(int pin) {
  return (pin >= 0 && pin < 64) ? arnes_pines[pin] : LOW;
}

extern unsigned long arnes_millis_valor;   // lo mueve el arnes, nunca solo
inline unsigned long millis() { return arnes_millis_valor; }

// delay() NO MUEVE EL RELOJ, Y ESO ES UNA DECISION CON SU MOTIVO, NO UN OLVIDO.
//
// Antes no existia -"mejor un error de enlazado que un arnes dormido"- y esa razon sigue
// valiendo para un delay() de minutos. La necesita camara_leerPin() de botones.cpp, que
// hace delay(5) entre sus dos lecturas del pin: es el antirrebote por software, y sin el
// no hay forma de compilar el fichero real.
//
// SE DEJA COMO NO-OP QUE CUENTA, y no como un avance de arnes_millis_valor, porque este
// arnes MIDE DURACIONES DE FASE con el mismo reloj: hacer que el antirrebote de la camara
// empujase millis() convertiria "cuanto dura el verde" en una funcion de cuantas veces se
// leyo un pin, y la cifra medida dejaria de ser la del firmware. Lo que se pierde queda
// anotado: este arnes NO mide el coste en tiempo de loop de los delay() de camara_leerPin().
extern unsigned long arnes_delays;
inline void delay(unsigned long ms) { arnes_delays += ms; }
