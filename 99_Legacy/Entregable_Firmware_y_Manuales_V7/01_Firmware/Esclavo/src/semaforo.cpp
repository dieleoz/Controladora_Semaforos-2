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

// OPT-6 (Manual de Señalización de Colombia): Eliminación de la transición Europea (Rojo+Amarillo).
// Ver MANUAL_USUARIO.md - Sección 1 (Comportamiento Físico de las Luces).
// Se usa semaforo_forzarVerde() para un salto directo y seguro a luz Verde.

void semaforo_iniciarTransicionARojo() {
  estado = S_AMARILLO;
  tCambio = millis();
  aplicarSalidas(LOW, HIGH, LOW);
}

void semaforo_toggle() {
  if (estado == S_ROJO || estado == S_FALLO) {
    // OPT-6: Transición limpia sin ámbar intermedio. Ver OPTIMIZACIONES.md
    semaforo_forzarVerde(); // Directo a verde (Norma Colombia OPT-6)
  } else if (estado == S_VERDE) {
    semaforo_iniciarTransicionARojo();
  }
}

void semaforo_iniciarFallo() {
  estado = S_FALLO;
  tCambio = millis();
  aplicarSalidas(LOW, LOW, LOW); // Empieza apagado, luego parpadea en actualizar()
}

void semaforo_actualizar() {
  unsigned long ahora = millis();
  // OPT-6: Aumento del tiempo de Ámbar a 4000ms para prevenir frenado de camiones.
  // Ver MANUAL_USUARIO.md - Sección 1.
  if (estado == S_AMARILLO && (ahora - tCambio >= 4000)) { // 4s de Amarillo
    estado = S_ROJO;
    aplicarSalidas(HIGH, LOW, LOW);
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
  }
  return "";
}