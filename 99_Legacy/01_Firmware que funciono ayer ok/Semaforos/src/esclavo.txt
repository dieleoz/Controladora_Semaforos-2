#include <Arduino.h>
#include "pines.h"
#include "semaforo.h"
#include "protocolo.h"
#include <string.h>

static unsigned long tUltimoPing = 0;

void setup() {
  semaforo_setup();
  protocolo_setup();
  semaforo_forzarRojo(); // SFTY-1: estado seguro explícito al arrancar

  // SFTY-1: 5s fijos en Rojo antes de decidir si hay comunicación con el maestro.
  delay(5000);

  char linea[64];
  bool huboComunicacion = false;
  while (protocolo_hayLineaDisponible(linea, sizeof(linea))) {
    if (strcmp(linea, "PING") == 0) {
      protocolo_enviarLinea("PONG");
      huboComunicacion = true;
    }
  }

  if (huboComunicacion) {
    tUltimoPing = millis();
    semaforo_iniciarEspera(); // SFTY-1: hay enlace, Rojo sostenido a la espera de una orden real del maestro
  } else {
    tUltimoPing = millis() - 9001; // fuerza la detección de fallo en el primer loop()
    semaforo_iniciarFallo();       // sin comunicación tras el arranque: parpadeo Ámbar
  }
}

void loop() {
  semaforo_actualizar();

  char linea[64];

  if (protocolo_hayLineaDisponible(linea, sizeof(linea))) {
    if (strcmp(linea, "PING") == 0) {
      protocolo_enviarLinea("PONG");
      tUltimoPing = millis();
      if (semaforo_estado() == S_FALLO) {
        semaforo_iniciarEspera(); // SFTY-1: recupera enlace -> Rojo sostenido hasta recibir orden real
      }
    } else if (strcmp(linea, "GO_RED") == 0) {
      semaforo_forzarRojo(); // De Verde a Rojo: cambio directo, sin Ámbar
    } else if (strcmp(linea, "GO_GREEN") == 0) {
      semaforo_iniciarTransicionAVerde(); // De Rojo a Verde: Ámbar 2s primero
    }
  }

  // Fallback si no hay comunicación del maestro. SFTY-1: 9000ms (antes 5000ms)
  // para tolerar caídas breves del radio LoRa sin resetear la alternancia.
  if (millis() - tUltimoPing > 9000) {
    if (semaforo_estado() != S_FALLO) {
      semaforo_iniciarFallo();
    }
  }

  static bool ackRojoEnviado = false, ackVerdeEnviado = false;
  if (semaforo_estable() && semaforo_estado() == S_ROJO && !ackRojoEnviado) {
    protocolo_enviarLinea("ACK_RED");
    ackRojoEnviado = true; ackVerdeEnviado = false;
  }
  if (semaforo_estable() && semaforo_estado() == S_VERDE && !ackVerdeEnviado) {
    protocolo_enviarLinea("ACK_GREEN");
    ackVerdeEnviado = true; ackRojoEnviado = false;
  }
}