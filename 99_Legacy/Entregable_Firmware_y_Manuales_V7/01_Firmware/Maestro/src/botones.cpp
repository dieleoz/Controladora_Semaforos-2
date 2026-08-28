// ===== src/botones.cpp =====
#include "botones.h"
#include "pines.h"

struct Boton {
  int pin;
  bool estadoAnt = false;
  bool estadoEstable = false;
  unsigned long tUltimoCambio = 0;
  unsigned long tUltimoFlanco = 0;
};

static Boton b1, b2, b3, b4;
const unsigned long DEBOUNCE_MS = 30;
const unsigned long FLANCO_MS = 200;

static void actualizar(Boton &b) {
  bool lecturaCruda = (digitalRead(b.pin) == LOW);

  if (lecturaCruda != b.estadoAnt) {
    b.tUltimoCambio = millis();
    b.estadoAnt = lecturaCruda;
  }

  if (millis() - b.tUltimoCambio > DEBOUNCE_MS) {
    b.estadoEstable = lecturaCruda;
  }
}

static bool disparadoAnt[4] = {false, false, false, false};

static bool flancoBoton(Boton &b, int idx) {
  actualizar(b);
  bool disparo = false;
  if (b.estadoEstable && !disparadoAnt[idx] && (millis() - b.tUltimoFlanco > FLANCO_MS)) {
    disparo = true;
    b.tUltimoFlanco = millis();
  }
  disparadoAnt[idx] = b.estadoEstable;
  return disparo;
}

void botones_setup() {
  pinMode(BOTON1, INPUT_PULLUP);
  pinMode(BOTON2, INPUT_PULLUP);
  pinMode(BOTON3, INPUT_PULLUP);
  pinMode(BOTON4, INPUT_PULLUP);

  b1.pin = BOTON1;
  b2.pin = BOTON2;
  b3.pin = BOTON3;
  b4.pin = BOTON4;
}

bool botonArriba()  { return flancoBoton(b1, 0); }
bool botonAbajo()   { return flancoBoton(b2, 1); }
bool botonAceptar() { return flancoBoton(b3, 2); }
bool botonCancelar(){ return flancoBoton(b4, 3); }