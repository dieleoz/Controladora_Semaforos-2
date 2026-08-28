// ===== src/semaforo.cpp =====
#include "semaforo.h"
#include "pines.h"

static EstadoSemaforo estado = S_ROJO;
static unsigned long tCambio = 0;

static void aplicarSalidas(bool rojo, bool amarillo, bool verde) {
  // SFTY-2: Enclavamiento Lógico (Safety Case)
  // Como el hardware (PCB) ya está fabricado y no tiene relés de interbloqueo,
  // evitamos por software que Verde y Rojo se enciendan simultáneamente.
  
  if (rojo) {
    verde = false;
    amarillo = false; // Opcional: forzar amarillo apagado si rojo está encendido, a menos que sea transición Rojo-Amarillo.
    // Wait, in S_ROJO_AMARILLO state, the code explicitly passes (HIGH, HIGH, LOW).
    // So if rojo is HIGH, amarillo CAN be HIGH. But verde MUST be LOW.
  } else if (verde) {
    rojo = false;
    // If verde is HIGH, amarillo usually is LOW, but let's just force rojo LOW.
  }

  // Prevención de Verde y Rojo al mismo tiempo por fallas de arriba
  if (rojo && verde) {
    verde = false; // El Rojo siempre gana por seguridad.
  }

  digitalWrite(ROJO1, rojo);
  digitalWrite(ROJO2, rojo);
  digitalWrite(AMARILLO1, amarillo);
  digitalWrite(AMARILLO2, amarillo);
  digitalWrite(VERDE1, verde);
  digitalWrite(VERDE2, verde);
}

void semaforo_setup() {
  pinMode(ROJO1, OUTPUT);
  pinMode(AMARILLO1, OUTPUT);
  pinMode(VERDE1, OUTPUT);
  pinMode(ROJO2, OUTPUT);
  pinMode(AMARILLO2, OUTPUT);
  pinMode(VERDE2, OUTPUT);

  semaforo_apagarTodo();
}

void semaforo_apagarTodo() {
  estado = S_ROJO;
  aplicarSalidas(LOW, LOW, LOW);
}

void semaforo_forzarRojo() {
  estado = S_ROJO;
  aplicarSalidas(HIGH, LOW, LOW);
}

void semaforo_forzarVerde() {
  estado = S_VERDE;
  aplicarSalidas(LOW, LOW, HIGH);
}

// Rojo -> Ámbar (2s) -> Verde. De Verde a Rojo el cambio es DIRECTO
// (semaforo_forzarRojo()), sin fase intermedia de Ámbar.

void semaforo_iniciarTransicionAVerde() {
  estado = S_AMARILLO;
  tCambio = millis();
  aplicarSalidas(LOW, HIGH, LOW);
}

void semaforo_iniciarFallo() {
  // SFTY-1: Pérdida de comunicación EN OPERACIÓN -> parpadeo Ámbar inmediato
  // (el retardo de 5s en Rojo aplica solo al arranque, ver setup() en main.cpp/maestro.txt).
  estado = S_FALLO;
  tCambio = millis();
  aplicarSalidas(LOW, LOW, LOW); // Empieza apagado, luego parpadea en actualizar()
}

void semaforo_iniciarEspera() {
  // SFTY-1: Rojo sostenido = hay enlace, pero aún no se ha iniciado una
  // configuración (Manual / Automático / Por demanda). Se mantiene como estado
  // interno separado de S_ROJO para poder reportarlo distinto (LCD/telemetría),
  // aunque físicamente sea Rojo igual que semaforo_forzarRojo().
  estado = S_ESPERA;
  tCambio = millis();
  aplicarSalidas(HIGH, LOW, LOW);
}

void semaforo_actualizar() {
  unsigned long ahora = millis();
  if (estado == S_AMARILLO && (ahora - tCambio >= 2000)) { // 2s de Ámbar antes de Verde
    estado = S_VERDE;
    aplicarSalidas(LOW, LOW, HIGH);
  } else if (estado == S_FALLO) {
    if (ahora - tCambio >= 500) {
      tCambio = ahora;
      static bool ambarStatus = false;
      ambarStatus = !ambarStatus;
      aplicarSalidas(LOW, ambarStatus, LOW);
    }
  }
}

bool semaforo_estable() {
  return estado == S_ROJO || estado == S_VERDE || estado == S_FALLO;
}

EstadoSemaforo semaforo_estado() {
  return estado;
}

const char* semaforo_nombreEstado() {
  switch (estado) {
    case S_ROJO: return "ROJO";
    case S_VERDE: return "VERDE";
    case S_AMARILLO: return "AMARILLO";
    case S_FALLO: return "FALLO COM";
    case S_ESPERA: return "ESPERA";
  }
  return "";
}