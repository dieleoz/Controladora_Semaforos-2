// ===== src/coordinador.cpp =====
#include "coordinador.h"
#include "protocolo.h"
#include "semaforo.h"
#include <string.h>

enum EstadoCoord {
  C_IDLE,
  C_INICIAL_ESPERA_ESTATICO,
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
// Ver MANUAL_USUARIO.md - Sección 1: Evita choque frontal en túneles largos.
static unsigned long tiempoDespejeMs = 15000; // SFTY-4 All-Red time (15s por defecto)

static bool handshakeOk = false;
static unsigned long tUltimoPing = 0;
static unsigned long tUltimoPongRecibido = 0; // SFTY-1: para detectar enlace activo en el arranque

static unsigned long tEsperandoAck = 0;
static unsigned long tRef = 0;
// SFTY-1/5: el esclavo tarda ~2000ms en pasar de Ámbar a Verde tras un GO_GREEN
// (semaforo_iniciarTransicionAVerde(), ver semaforo.cpp) y recién ahí envía el
// ACK_GREEN. Un timeout muy justo deja poco margen para la latencia real del
// radio (E90-DTU), pudiendo provocar falsos "fallo de comunicación" que
// resetean quienVerde y desincronizan la alternancia maestro/esclavo.
// Se deja en 9000ms (2000ms de transición + margen amplio de radio).
const unsigned long TIMEOUT_ACK_MS = 9000;

void coordinador_setup() {
  protocolo_setup();
  semaforo_setup(); // luego se fuerza rojo desde main.cpp al conectar
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
      semaforo_forzarRojo(); // De Verde a Rojo: cambio directo, sin Ámbar
      estadoC = C_MASTER_A_ROJO;
      break;

    case QV_ESCLAVO:
      protocolo_enviarLinea("GO_RED");
      tEsperandoAck = millis();
      estadoC = C_ESPERANDO_ACK_RED;
      break;
  }
}

void coordinador_actualizar() {
  semaforo_actualizar();

  char linea[64];
  bool llego = protocolo_hayLineaDisponible(linea, sizeof(linea));

  if (llego) {
    if (strcmp(linea, "PING") == 0) {
      protocolo_enviarLinea("PONG");
    } else if (strcmp(linea, "PONG") == 0) {
      handshakeOk = true;
      tUltimoPing = millis();
      tUltimoPongRecibido = millis();
    }
  }

  // Heartbeat del Master
  if (millis() - tUltimoPing > 2000) {
    protocolo_enviarLinea("PING");
    tUltimoPing = millis();
    // Si pasa mucho tiempo sin handshakeOk, consideramos pérdida de comunicación.
    // SFTY-1: 9000ms (en vez de 5000ms) para tolerar caídas breves del radio
    // LoRa (E90-DTU) sin resetear la alternancia maestro/esclavo por gusto.
    static unsigned long tFalloCom = 0;
    if (!handshakeOk) {
      if (tFalloCom == 0) tFalloCom = millis();
      else if (millis() - tFalloCom > 9000) {
        estadoC = C_FALLO;
      }
    } else {
      handshakeOk = false; // Requiere un PONG para confirmarse
      tFalloCom = 0;
      if (estadoC == C_FALLO) {
        estadoC = C_IDLE;
        quienVerde = QV_NINGUNO;
        semaforo_forzarRojo(); // Recupera estado seguro (Rojo) en lugar de parpadeo fallo
      }
    }
  }

  switch (estadoC) {

    case C_IDLE:
      break;

    case C_INICIAL_ESPERA_ESTATICO:
      // SFTY-1: arranque limpio -- el ESCLAVO entra en función desde el
      // primer cambio, usando el MISMO mecanismo (GO_GREEN + espera de
      // ACK_GREEN) que cualquier otro turno. Ya no hay una "vuelta de
      // calentamiento" donde el maestro se pone verde solo, sin avisarle
      // nada al esclavo.
      if (millis() - tRef >= tiempoDespejeMs) {
        protocolo_enviarLinea("GO_GREEN");
        tEsperandoAck = millis();
        estadoC = C_ESPERANDO_ACK_GREEN;
      }
      break;

    case C_MASTER_A_ROJO:
      if (semaforo_estable() && semaforo_estado() == S_ROJO) {
        tRef = millis();
        estadoC = C_ESPERA_ESTATICO_TRAS_MASTER;
      }
      break;

    case C_ESPERA_ESTATICO_TRAS_MASTER:
      // SFTY-4: En este estado AMBOS semáforos están en Rojo (All-Red).
      // Se espera `tiempoDespejeMs` antes de enviar la orden `GO_GREEN` al esclavo.
      // Si la distancia es 500m, este valor puede ser de casi 3 minutos.
      if (millis() - tRef >= tiempoDespejeMs) {
        protocolo_enviarLinea("GO_GREEN");
        tEsperandoAck = millis();
        estadoC = C_ESPERANDO_ACK_GREEN;
      }
      break;

    case C_ESPERANDO_ACK_GREEN:
      if (llego && strcmp(linea, "ACK_GREEN") == 0) {
        quienVerde = QV_ESCLAVO;
        estadoC = C_IDLE;
      } else if (millis() - tEsperandoAck > TIMEOUT_ACK_MS) {
        estadoC = C_FALLO;
      }
      break;

    case C_ESPERANDO_ACK_RED:
      if (llego && strcmp(linea, "ACK_RED") == 0) {
        tRef = millis();
        estadoC = C_ESPERA_ESTATICO_TRAS_ESCLAVO;
      } else if (millis() - tEsperandoAck > TIMEOUT_ACK_MS) {
        estadoC = C_FALLO;
      }
      break;

    case C_ESPERA_ESTATICO_TRAS_ESCLAVO:
      // SFTY-4: El Esclavo ya está en rojo. El Maestro espera `tiempoDespejeMs` (All-Red).
      // Tras el tiempo de espera seguro, el Maestro se pone Verde.
      if (millis() - tRef >= tiempoDespejeMs) {
        semaforo_iniciarTransicionAVerde(); // De Rojo a Verde: Ámbar 2s primero
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
  // Solo se encarga de mantener viva la conexión o detectar fallos si no estamos
  // activamente en el modo automático controlando el ciclo.
  coordinador_actualizar();
}

bool coordinador_listoParaContar() {
  return estadoC == C_IDLE;
}

bool coordinador_comunicacionPerdida() {
  return estadoC == C_FALLO;
}

// SFTY-1: usado solo en el arranque para decidir, tras los 5s en Rojo fijo,
// si ya hay enlace con el esclavo (PONG reciente) o si se debe arrancar en fallo.
bool coordinador_comunicacionActiva() {
  return tUltimoPongRecibido != 0 && (millis() - tUltimoPongRecibido) < 5000;
}

// SFTY-1: fuerza el estado de fallo del coordinador (parpadeo Ámbar) manteniendo
// consistente la máquina de estados, para que la reconexión posterior (PONG real)
// dispare correctamente la recuperación normal a Rojo.
void coordinador_forzarFallo() {
  estadoC = C_FALLO;
  if (semaforo_estado() != S_FALLO) {
    semaforo_iniciarFallo();
  }
}

const char* coordinador_nombreEstadoMaster() {
  return semaforo_nombreEstado();
}

// SFTY-1: al salir de cualquier modo activo (Manual/Automático/Demanda) de
// vuelta al menú, ambos semáforos deben quedar en Rojo -- sin importar en qué
// color haya quedado cada uno -- hasta que se elija una nueva configuración.
// Se llama una sola vez, al entrar al menú (ver menu_setup()).
void coordinador_volverAmbosARojo() {
  protocolo_enviarLinea("GO_RED"); // fuerza al esclavo a Rojo directo (idempotente si ya está en Rojo)
  semaforo_forzarRojo();           // fuerza al maestro a Rojo directo
  estadoC = C_IDLE;
  quienVerde = QV_NINGUNO;
}