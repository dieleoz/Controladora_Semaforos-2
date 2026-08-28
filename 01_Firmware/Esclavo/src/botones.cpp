// ===== src/botones.cpp (ESCLAVO) =====
#include "botones.h"
#include "pines.h"
#include "mando.h"

// ---------------------------------------------------------------------------
// N-16 — Portado SIN CAMBIOS de src/botones.cpp del Maestro.
//
// Se copia en vez de "mejorarse" porque los pulsadores del gabinete estan en
// paralelo con el mando de reles que el operario acciona desde el piso, a 5 m,
// sin ver la pantalla (SFTY-21). Ese mando entrega un pulso por flanco de ~2 s y
// no repite. Si cada punta filtrara los rebotes con criterios distintos, la
// misma orden se leeria distinto en cada gabinete y el operario no tendria forma
// de saber cual de los dos le hizo caso.
//
// Entradas en INPUT_PULLUP y pulsador contra masa: pulsado = LOW. Cableado
// identico al del Maestro (PB9, PB13, PB14, PB15; ver pines.h).
// ---------------------------------------------------------------------------

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

// Solo el FLANCO de pulsacion cuenta. Mantener pulsado no repite, y es
// deliberado: con el mando de reles la pulsacion sostenida no existe -da un solo
// pulso-, asi que una repeticion por mantener seria una funcion que el operario
// del piso nunca podria usar y que ademas dispararia sola en la botonera fisica.
//
// Sirve igual para el pulsador del gabinete y para el rele del mando, que esta
// cableado EN PARALELO con el: electricamente son el mismo contacto y el firmware
// no puede distinguir un dedo de un rele.
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

// Flanco detectado en ESTA iteracion, pendiente de que alguien lo consuma.
static bool flanco[4] = {false, false, false, false};

void botones_setup() {
  pinMode(BOTON1, INPUT_PULLUP);
  pinMode(BOTON2, INPUT_PULLUP);
  pinMode(BOTON3, INPUT_PULLUP);
  pinMode(BOTON4, INPUT_PULLUP);

  b1.pin = BOTON1;
  b2.pin = BOTON2;
  b3.pin = BOTON3;
  b4.pin = BOTON4;

  // N-26 — UN BOTON YA PULSADO AL ENCENDER NO ES UNA PULSACION, ES UN ESTADO.
  //
  // Se corrige IGUAL que en el Maestro, y por la misma razon por la que este archivo se
  // porto sin cambios: los pulsadores de los dos gabinetes van en paralelo con el mando
  // de reles, y si cada punta interpretara el arranque de forma distinta la misma orden
  // se leeria distinto en cada una.
  //
  // El fallo se vio en el MAESTRO -aparecia solo en la pantalla de configuracion del
  // Modo Manual, un ACEPTAR que nadie dio-, pero el codigo era identico aqui, asi que
  // el defecto tambien lo era. Este setup declaraba los pines sin LEERLOS: todo el
  // estado arrancaba en false aunque el pin estuviera en LOW, y la primera llamada a
  // flancoBoton() veia estadoEstable=true con disparadoAnt=false, que es la definicion
  // de un flanco. Con tUltimoCambio y tUltimoFlanco en 0, ni el antirrebote ni el
  // guarda de FLANCO_MS filtraban esa primera lectura.
  //
  // AQUI PESA MAS QUE EN EL MAESTRO. El Esclavo no tiene a nadie mirando su pantalla:
  // el operario esta abajo, junto al otro gabinete. Una maniobra que arranque sola en
  // esta punta no la ve nadie, y lo que se nota es el cruce descuadrado.
  //
  // Se siembra el estado REAL de cada pin, y disparadoAnt con el mismo valor: un boton
  // que ya venia pulsado queda como "ya disparado" y no genera flanco hasta que se
  // SUELTE y se vuelva a pulsar. Al encender no sabemos cuanto lleva asi, solo que
  // nadie lo acaba de pulsar. Si esta trabado, esta punta arranca normal y ese boton no
  // responde, en vez de ejecutar por su cuenta.
  //
  // Los 2 ms son para que el pull-up interno -unos 40 kOhm- levante la linea antes de
  // creerse la lectura. Con el watchdog en 4 s no comprometen nada.
  delay(2);

  Boton *todos[4] = {&b1, &b2, &b3, &b4};
  for (int i = 0; i < 4; i++) {
    const bool pulsado = (digitalRead(todos[i]->pin) == LOW);
    todos[i]->estadoAnt = pulsado;
    todos[i]->estadoEstable = pulsado;
    todos[i]->tUltimoCambio = millis();  // el antirrebote arranca contando desde AHORA
    disparadoAnt[i] = pulsado;           // pulsado al arrancar = flanco ya consumido
  }
}

void botones_actualizar() {
  flanco[0] = flancoBoton(b1, 0);
  flanco[1] = flancoBoton(b2, 1);
  flanco[2] = flancoBoton(b3, 2);
  flanco[3] = flancoBoton(b4, 3);

  // SFTY-21: el mando ve el pulso AQUI, antes de que ninguna pantalla pueda
  // consumirlo, y sin consumirlo el mismo. Que la secuencia se reconozca no puede
  // depender de en que pantalla este el equipo; para eso existe el mando.
  //
  // Solo se le pasan A (Boton 1) y B (Boton 2). El Boton 3 EJECUTA -entra al Modo
  // Degradado desde la pantalla de confirmacion- y el 4 sale: si formaran parte de
  // alguna secuencia, repetirlos a ciegas podria activar un modo que nadie pidio.
  // Ver mando.cpp.
  if (flanco[0]) mando_registrarPulso(MANDO_A);
  if (flanco[1]) mando_registrarPulso(MANDO_B);
}

static bool consumir(int idx) {
  bool v = flanco[idx];
  flanco[idx] = false;
  return v;
}

bool botonArriba()  { return consumir(0); }
bool botonAbajo()   { return consumir(1); }
bool botonAceptar() { return consumir(2); }
bool botonCancelar(){ return consumir(3); }
