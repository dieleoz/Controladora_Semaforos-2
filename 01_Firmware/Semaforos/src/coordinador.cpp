// ===== src/coordinador.cpp =====
#include "coordinador.h"
#include "protocolo.h"
#include "semaforo.h"
#include <string.h>

enum EstadoCoord {
  C_IDLE,
  C_INICIAL_ESPERA_ESTATICO,
  C_INICIAL_MASTER_A_VERDE,
  C_MASTER_A_ROJO,
  C_ESPERA_ESTATICO_TRAS_MASTER,
  C_ESPERANDO_ACK_GREEN,
  C_ESPERANDO_ACK_RED,
  C_ESPERA_ESTATICO_TRAS_ESCLAVO,
  C_MASTER_A_VERDE,
  C_FALLO
};

enum QuienVerde { QV_NINGUNO, QV_MASTER, QV_ESCLAVO };

static EstadoCoord estadoC = C_IDLE;
static QuienVerde quienVerde = QV_NINGUNO;

// SFTY-4 (Safety Case): Tiempo de Despeje / All-Red
static unsigned long tiempoDespejeMs = 15000;

static bool handshakeOk = false;
static unsigned long tUltimoPing = 0;

static unsigned long tEsperandoAck = 0;
static unsigned long tRef = 0;
static uint8_t retryCount = 0;
const unsigned long TIMEOUT_ACK_MS = 1500; // SFTY-6: 1.5s timeout per retry

void coordinador_setup() {
  protocolo_setup();
  semaforo_setup(); 
  estadoC = C_IDLE;
  quienVerde = QV_NINGUNO;
  handshakeOk = false;
}

void coordinador_reiniciarConexion() {
  handshakeOk = false;
  estadoC = C_IDLE;
  quienVerde = QV_NINGUNO;
  tUltimoPing = 0;
}

bool coordinador_intentarHandshake() {
  coordinador_actualizar();
  return handshakeOk;
}

void coordinador_configurar(unsigned long tiempoDespeje, unsigned long, unsigned long) {
  tiempoDespejeMs = tiempoDespeje;
}

void coordinador_pedirCambio() {
  if (estadoC != C_IDLE) return;

  switch (quienVerde) {
    case QV_NINGUNO:
      tRef = millis();
      estadoC = C_INICIAL_ESPERA_ESTATICO;
      break;

    case QV_MASTER:
      semaforo_iniciarTransicionARojo();
      estadoC = C_MASTER_A_ROJO;
      break;

    case QV_ESCLAVO:
      protocolo_enviarPaquete(CMD_GO_RED);
      tEsperandoAck = millis();
      retryCount = 0;
      estadoC = C_ESPERANDO_ACK_RED;
      break;
  }
}

void coordinador_actualizar() {
  semaforo_actualizar();

  RF_Packet pkt;
  bool llego = protocolo_hayPaqueteDisponible(&pkt);

  if (llego) {
    if (pkt.command == 0x04) { // PING (CMD_PING)
      protocolo_enviarPaquete(0x05); // PONG (CMD_PONG)
    } else if (pkt.command == 0x05) { // PONG
      handshakeOk = true;
      tUltimoPing = millis();
    }
  }

  // Heartbeat del Master
  if (millis() - tUltimoPing > 3000) {
    protocolo_enviarPaquete(0x04); // PING
    tUltimoPing = millis();
    static unsigned long tFalloCom = 0;
    if (!handshakeOk) {
      if (tFalloCom == 0) tFalloCom = millis();
      else if (millis() - tFalloCom > 9000) { // Tras 9 segundos sin heartbeat
        estadoC = C_FALLO;
      }
    } else {
      handshakeOk = false; // Requiere un PONG para confirmarse
      tFalloCom = 0;
      if (estadoC == C_FALLO) {
        estadoC = C_IDLE;
        quienVerde = QV_NINGUNO;
        semaforo_forzarRojo(); 
      }
    }
  }

  switch (estadoC) {

    case C_IDLE:
      break;

    case C_INICIAL_ESPERA_ESTATICO:
      if (millis() - tRef >= tiempoDespejeMs) {
        semaforo_forzarVerde(); 
        estadoC = C_INICIAL_MASTER_A_VERDE;
      }
      break;

    case C_INICIAL_MASTER_A_VERDE:
      if (semaforo_estable() && semaforo_estado() == S_VERDE) {
        quienVerde = QV_MASTER;
        estadoC = C_IDLE;
      }
      break;

    case C_MASTER_A_ROJO:
      if (semaforo_estable() && semaforo_estado() == S_ROJO) {
        tRef = millis();
        estadoC = C_ESPERA_ESTATICO_TRAS_MASTER;
      }
      break;

    case C_ESPERA_ESTATICO_TRAS_MASTER:
      if (millis() - tRef >= tiempoDespejeMs) {
        protocolo_enviarPaquete(CMD_GO_GREEN);
        tEsperandoAck = millis();
        retryCount = 0;
        estadoC = C_ESPERANDO_ACK_GREEN;
      }
      break;

    case C_ESPERANDO_ACK_GREEN:
      if (llego && pkt.command == CMD_ACK_GREEN) {
        quienVerde = QV_ESCLAVO;
        estadoC = C_IDLE;
      } else if (millis() - tEsperandoAck > TIMEOUT_ACK_MS) {
        retryCount++;
        if (retryCount >= 3) {
            estadoC = C_FALLO; // SFTY-6: Fallo tras 3 reintentos
        } else {
            protocolo_enviarPaquete(CMD_GO_GREEN);
            tEsperandoAck = millis();
        }
      }
      break;

    case C_ESPERANDO_ACK_RED:
      if (llego && pkt.command == CMD_ACK_RED) {
        tRef = millis();
        estadoC = C_ESPERA_ESTATICO_TRAS_ESCLAVO;
      } else if (millis() - tEsperandoAck > TIMEOUT_ACK_MS) {
        retryCount++;
        if (retryCount >= 3) {
            estadoC = C_FALLO; // SFTY-6: Fallo tras 3 reintentos
        } else {
            protocolo_enviarPaquete(CMD_GO_RED);
            tEsperandoAck = millis();
        }
      }
      break;

    case C_ESPERA_ESTATICO_TRAS_ESCLAVO:
      if (millis() - tRef >= tiempoDespejeMs) {
        semaforo_forzarVerde(); 
        estadoC = C_MASTER_A_VERDE;
      }
      break;

    case C_MASTER_A_VERDE:
      if (semaforo_estable() && semaforo_estado() == S_VERDE) {
        quienVerde = QV_MASTER;
        estadoC = C_IDLE;
      }
      break;

    case C_FALLO:
      if (semaforo_estado() != S_FALLO) {
        semaforo_iniciarFallo();
      }
      break;
  }
}

void coordinador_actualizar_background() {
  coordinador_actualizar();
}

bool coordinador_listoParaContar() {
  return estadoC == C_IDLE;
}

bool coordinador_comunicacionPerdida() {
  return estadoC == C_FALLO;
}

const char* coordinador_nombreEstadoMaster() {
  return semaforo_nombreEstado();
}