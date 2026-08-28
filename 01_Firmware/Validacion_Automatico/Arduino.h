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

extern unsigned long arnes_millis_valor;   // lo mueve el arnes, nunca solo
inline unsigned long millis() { return arnes_millis_valor; }

// coordinador.cpp y semaforo.cpp no llaman a delay(); modo_automatico.cpp tampoco.
// No se define: si algun dia alguno empezara a usarlo, mejor un error de enlazado
// que un arnes que se queda dormido de verdad esperando un delay() de minutos.
