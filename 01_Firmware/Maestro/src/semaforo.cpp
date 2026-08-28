// ===== src/semaforo.cpp =====
#include "semaforo.h"
#include "pines.h"

static EstadoSemaforo estado = S_ROJO;
static unsigned long tCambio = 0;

// --- SFTY-21: senal del mando de reles (V8.7) ------------------------------
// Ver la explicacion completa en semaforo.h. Aqui solo lo imprescindible: mientras
// senalActiva vale true, la logica normal sigue corriendo pero sus salidas se guardan
// en ultR/ultA/ultV en lugar de escribirse; los pines los lleva la senal. Al acabar
// se vuelca lo guardado y todo continua como si nada.
static bool senalActiva = false;
static bool ultR = false, ultA = false, ultV = false;

static uint8_t senalDestellos = 0;   // destellos rojos que faltan
static bool senalEsAmbar = false;
static bool senalEncendida = false;
static unsigned long tSenal = 0;     // instante del ultimo cambio de la senal
static unsigned long tSenalInicio = 0;
static unsigned long senalDuracion = 0;  // solo para el ambar rapido

// 400 ms encendido y 400 ms apagado. Contable a 5 m y de dia: por debajo de ~250 ms
// el ojo deja de separar destellos y el operario ya no puede contarlos, que es lo
// unico que se le pide. Cuatro destellos son 3,6 s, dentro de lo que el operario
// espera mirando hacia arriba.
static const unsigned long DESTELLO_ON_MS = 400;
static const unsigned long DESTELLO_OFF_MS = 400;

// Ambar de rechazo: 150 ms. El ambar de fallo va a 500 ms, asi que el ritmo por si
// solo distingue "rechazado" de "estado seguro".
static const unsigned long AMBAR_RAPIDO_PERIODO_MS = 150;

static void escribirPines(bool rojo, bool amarillo, bool verde) {
  digitalWrite(ROJO1, rojo);
  digitalWrite(ROJO2, rojo);
  digitalWrite(AMARILLO1, amarillo);
  digitalWrite(AMARILLO2, amarillo);
  digitalWrite(VERDE1, verde);
  digitalWrite(VERDE2, verde);

  // SFTY-28: LA PLUMA SIGUE AL VERDE, Y SALE POR LA MISMA PUERTA QUE LAS LUCES.
  //
  // Va DENTRO de escribirPines() a proposito, no en un modo ni en un despachador: es
  // la regla 6 extendida. Si un modo pudiera mover la barrera por su cuenta habria
  // una pluma arriba con la luz en rojo, y eso es PEOR que no tener barrera, porque
  // el conductor confia en ella. Aqui no puede contradecir a las luces: se escribe
  // con el mismo 'verde' YA enclavado que acaba de encender la lampara.
  //
  // Sube con verde. Rojo, ambar de transicion, todo-rojo de despeje y destellos del
  // mando la dejan ABAJO.
  //
  // Y SUBE TAMBIEN EN S_FALLO, que es una decision de operacion, no del firmware.
  // S_FALLO es el ambar intermitente de SFTY-6: el equipo se quedo sin enlace y ya no
  // puede garantizar quien tiene el paso. Ahi caben dos politicas y ninguna es
  // obviamente correcta: con la pluma ABAJO se cierra la via por completo -y un
  // corredor de obra sin salida es su propio peligro-; con la pluma ARRIBA se deja
  // pasar a los dos lados con precaucion, que es lo que el ambar intermitente
  // significa en la calle. El cliente y el PMT eligieron ARRIBA el 27/08/2026.
  //
  // Si algun dia se cambia, se cambia AQUI y en la tabla de SFTY-28, y el arnes del
  // automatico lo notara: su invariante conoce esta excepcion por nombre.
  //
  // Esto vale porque es un digitalWrite local, que no puede bloquearse. El dia que la
  // pluma cuelgue de un bus (I2C del PCF8574), NO puede vivir aqui: un bus colgado
  // dejaria las luces esperando. Iria detras, con timeout, y sin tocar esta funcion.
  digitalWrite(MOTOR_TALANQUERA,
               (verde || estado == S_FALLO) ? TALANQUERA_ABRIR : TALANQUERA_CERRAR);
}

static void aplicarSalidas(bool rojo, bool amarillo, bool verde) {
  // SFTY-2: Enclavamiento Lógico (Safety Case)
  // Como el hardware (PCB) ya está fabricado y no tiene relés de interbloqueo,
  // evitamos por software que Verde y Rojo se enciendan simultáneamente.

  if (rojo) {
    verde = false;
  } else if (verde) {
    rojo = false;
  }

  // Prevención de Verde y Rojo al mismo tiempo por fallas de arriba
  if (rojo && verde) {
    verde = false; // El Rojo siempre gana por seguridad.
  }

  // SFTY-21: lo que la logica quiere se guarda SIEMPRE, incluso con una senal en
  // curso. Asi al terminar la senal los pines se ponen al dia con la ultima decision
  // real y no con una foto vieja.
  ultR = rojo; ultA = amarillo; ultV = verde;

  // El enclavamiento de arriba se aplica ANTES de este punto a proposito: lo que se
  // guarda ya viene saneado, de modo que el volcado posterior no puede reintroducir
  // una combinacion prohibida.
  if (senalActiva) return;

  escribirPines(rojo, amarillo, verde);
}

static void terminarSenal() {
  senalActiva = false;
  senalDestellos = 0;
  senalEsAmbar = false;
  // Los pines se ponen al dia con lo ultimo que pidio la logica normal mientras la
  // senal ocupaba la salida.
  escribirPines(ultR, ultA, ultV);
}

static void actualizarSenal() {
  unsigned long ahora = millis();

  if (senalEsAmbar) {
    if (ahora - tSenal >= AMBAR_RAPIDO_PERIODO_MS) {
      tSenal = ahora;
      senalEncendida = !senalEncendida;
      escribirPines(false, senalEncendida, false);
    }
    if (ahora - tSenalInicio >= senalDuracion) {
      terminarSenal();
    }
    return;
  }

  if (senalEncendida) {
    if (ahora - tSenal >= DESTELLO_ON_MS) {
      senalEncendida = false;
      escribirPines(false, false, false);
      tSenal = ahora;
      if (senalDestellos > 0) senalDestellos--;
      if (senalDestellos == 0) terminarSenal();
    }
  } else {
    if (ahora - tSenal >= DESTELLO_OFF_MS) {
      senalEncendida = true;
      escribirPines(true, false, false);   // ROJO: nunca verde para confirmar
      tSenal = ahora;
    }
  }
}

void semaforo_destellosRojos(uint8_t n) {
  if (n == 0) return;
  senalActiva = true;
  senalEsAmbar = false;
  senalDestellos = n;
  senalEncendida = false;
  tSenal = millis();
  tSenalInicio = tSenal;
  escribirPines(false, false, false);  // hueco inicial: hace visible el 1er destello
}

void semaforo_ambarRapido(unsigned long ms) {
  senalActiva = true;
  senalEsAmbar = true;
  senalDestellos = 0;
  senalEncendida = true;
  tSenal = millis();
  tSenalInicio = tSenal;
  senalDuracion = ms;
  escribirPines(false, true, false);
}

bool semaforo_senalEnCurso() { return senalActiva; }

void semaforo_setup() {
  pinMode(ROJO1, OUTPUT);
  pinMode(AMARILLO1, OUTPUT);
  pinMode(VERDE1, OUTPUT);
  pinMode(ROJO2, OUTPUT);
  pinMode(AMARILLO2, OUTPUT);
  pinMode(VERDE2, OUTPUT);

  // SFTY-28: la pluma se declara y se CIERRA antes que nada. Un arranque que la
  // dejara en el estado en que quedo el pin es una via abierta sin regulacion
  // durante los dos segundos de bienvenida.
  pinMode(MOTOR_TALANQUERA, OUTPUT);
  digitalWrite(MOTOR_TALANQUERA, TALANQUERA_CERRAR);

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

void semaforo_iniciarTransicionAVerde() {
  estado = S_AMARILLO;
  tCambio = millis();
  aplicarSalidas(LOW, HIGH, LOW);
}

void semaforo_toggle() {
  if (estado == S_ROJO || estado == S_FALLO) {
    semaforo_iniciarTransicionAVerde();
  } else if (estado == S_VERDE) {
    semaforo_forzarRojo(); // Directo a rojo
  }
}

void semaforo_iniciarFallo() {
  estado = S_FALLO;
  tCambio = millis();
  aplicarSalidas(LOW, LOW, LOW); // Empieza apagado, luego parpadea en actualizar()
}

static bool testLedsActivo = false;
static unsigned long tInicioTest = 0;

void semaforo_iniciarTestLeds() {
  testLedsActivo = true;
  tInicioTest = millis();
}

bool semaforo_testLedsEnCurso() {
  return testLedsActivo;
}

void semaforo_actualizar() {
  unsigned long ahora = millis();

  // Test de lámparas en taller (6 segundos: 2s Rojo -> 2s Amarillo -> 2s Verde)
  if (testLedsActivo) {
    unsigned long elapsed = ahora - tInicioTest;
    if (elapsed < 2000) {
      escribirPines(true, false, false);
    } else if (elapsed < 4000) {
      escribirPines(false, true, false);
    } else if (elapsed < 6000) {
      escribirPines(false, false, true);
    } else {
      testLedsActivo = false;
      aplicarSalidas(true, false, false);
    }
    return;
  }

  // SFTY-21: la senal se atiende ANTES y NO se sale de la funcion. La logica de
  // abajo tiene que seguir corriendo aunque la senal ocupe las luces; si se
  // devolviera aqui, una transicion a verde pedida justo antes se quedaria congelada
  // y el coordinador esperaria indefinidamente un estado que nadie va a alcanzar.
  if (senalActiva) actualizarSenal();
  // Transición Rojo -> Amarillo -> Verde
  if (estado == S_AMARILLO && (ahora - tCambio >= 4000)) { // 4s de Amarillo
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
  }
  return "";
}