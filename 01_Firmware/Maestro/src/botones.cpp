// ===== src/botones.cpp =====
#include "botones.h"
#include "pines.h"
#include "mando.h"

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

// Flanco de bajada ya filtrado. Sirve igual para el boton fisico y para el rele del
// mando, que esta cableado EN PARALELO con el: electricamente son el mismo contacto.
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
  // CONFIRMADO EN BANCO EL 01/08/2026, en cuanto N-17 dejo arrancar la tarjeta: el
  // Maestro aparecia solo en la pantalla de configuracion del Modo Manual sin que
  // nadie tocara nada. Es un ACEPTAR fantasma en la primera vuelta del loop.
  //
  // El motivo era que este setup declaraba los pines pero NUNCA LOS LEIA. Todo el
  // estado arrancaba en false -"ningun boton pulsado"- aunque el pin estuviera en LOW,
  // asi que la primera llamada a flancoBoton() encontraba estadoEstable=true con
  // disparadoAnt=false y eso, por definicion, es un flanco. El guarda de FLANCO_MS
  // tampoco frenaba nada: tUltimoFlanco valia 0 y para cuando corre el loop ya han
  // pasado de sobra 200 ms.
  //
  // Y con tUltimoCambio tambien en 0, "millis() - 0 > DEBOUNCE_MS" era cierto desde el
  // primer instante: el antirrebote no filtraba la lectura inicial. Dos agujeros
  // encadenados que solo se abren en el arranque, que es donde menos se miran.
  //
  // POR QUE IMPORTA MAS DE LO QUE PARECE: el Boton 3 EJECUTA. Subir o bajar con el 1 y
  // el 2 no rompe nada, pero un 3 fantasma arranca un modo que nadie pidio, y en un
  // semaforo eso es una maniobra. Ademas los pulsadores del gabinete van EN PARALELO
  // con el mando de reles: un rele en reposo cerrado, un pulsador trabado o el ruido
  // en los 5 m de cable hasta el gabinete dejan el pin en LOW al encender sin que haya
  // ningun dedo de por medio, y el firmware no puede distinguirlos.
  //
  // Se siembra el estado REAL de cada pin, y sobre todo se siembra disparadoAnt con el
  // mismo valor: un boton que ya venia pulsado queda marcado como "ya disparado", de
  // modo que no genera flanco hasta que se SUELTE y se vuelva a pulsar. Es la lectura
  // correcta: al encender no sabemos cuanto lleva asi, solo que nadie lo acaba de
  // pulsar. Si esta trabado de verdad, el equipo arranca en el menu y ese boton no
  // responde -que se diagnostica en diez segundos- en vez de ejecutar por su cuenta.
  //
  // El pull-up interno son unos 40 kOhm y el cable hasta la botonera es largo; se le da
  // un respiro para que la linea suba antes de creerse la lectura. Con el watchdog en
  // 4 s, 2 ms aqui no comprometen nada.
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

  // SFTY-21: el mando ve el pulso AQUI, antes de que ningun modo pueda consumirlo, y
  // sin consumirlo el mismo. Que la secuencia se reconozca no puede depender de en
  // que pantalla este el equipo; para eso existe el mando.
  //
  // Solo se le pasan A (Boton 1) y B (Boton 2). El Boton 3 EJECUTA y el 4 sale: si
  // formaran parte de alguna secuencia, repetirlos a ciegas podria arrancar un modo
  // que nadie pidio. Ver mando.cpp.
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
