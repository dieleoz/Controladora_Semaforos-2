#include <Arduino.h>
#include "pines.h"
#include "semaforo.h"
#include "protocolo.h"
#include <IWatchdog.h>

void setup() {
  semaforo_setup();
  protocolo_setup();
  semaforo_forzarRojo();
  // IWatchdog.begin(2000000); // 2 segundos
}

void loop() {
  // IWatchdog.reload();
  semaforo_actualizar();

  RF_Packet pkt;
  static unsigned long tUltimoComando = millis();

  if (protocolo_hayPaqueteDisponible(&pkt)) {
    tUltimoComando = millis();
    
    // El PING ahora es implícito o con CMD_GO_GREEN/RED
    if (pkt.command == CMD_GO_RED) {
      semaforo_iniciarTransicionARojo();
    } else if (pkt.command == CMD_GO_GREEN) {
      semaforo_forzarVerde();
    }
    
    // Si recuperamos conexion tras un fallo
    if (semaforo_estado() == S_FALLO) {
      semaforo_forzarRojo();
    }
  }

  // Fallback si no hay comunicación del maestro en 5s
  if (millis() - tUltimoComando > 5000) {
    if (semaforo_estado() != S_FALLO) {
      semaforo_iniciarFallo();
    }
  }

  static bool ackRojoEnviado = false, ackVerdeEnviado = false;
  if (semaforo_estable() && semaforo_estado() == S_ROJO && !ackRojoEnviado) {
    protocolo_enviarPaquete(CMD_ACK_RED);
    ackRojoEnviado = true; ackVerdeEnviado = false;
  }
  if (semaforo_estable() && semaforo_estado() == S_VERDE && !ackVerdeEnviado) {
    protocolo_enviarPaquete(CMD_ACK_GREEN);
    ackVerdeEnviado = true; ackRojoEnviado = false;
  }
}
