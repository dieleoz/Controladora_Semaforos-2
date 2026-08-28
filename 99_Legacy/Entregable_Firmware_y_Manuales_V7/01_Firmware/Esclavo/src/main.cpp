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
  // SFTY-1: Watchdog CKS32 reload
  IWDG->KR = 0xAAAA;
  semaforo_actualizar();

  RF_Packet pkt;
  static unsigned long tUltimoComando = millis();

  if (protocolo_hayPaqueteDisponible(&pkt)) {
    tUltimoComando = millis();
    
    if (pkt.command == 0x04) { // PING
      protocolo_enviarPaquete(0x05); // Responder PONG (Heartbeat SFTY-8)
    } else if (pkt.command == CMD_GO_RED) {
      semaforo_iniciarTransicionARojo();
      protocolo_enviarPaquete(CMD_ACK_RED); // ACK inmediato para mitigar retardo RF
    } else if (pkt.command == CMD_GO_GREEN) {
      semaforo_forzarVerde();
      protocolo_enviarPaquete(CMD_ACK_GREEN);
    }
    
    // Si recuperamos conexion tras un fallo
    if (semaforo_estado() == S_FALLO) {
      semaforo_forzarRojo();
    }
  }

  // Fallback unificado si no hay comunicación del maestro en 3s (3000ms)
  if (millis() - tUltimoComando > 3000) {
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

