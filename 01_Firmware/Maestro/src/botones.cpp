// ===== src/botones.cpp =====
#include "botones.h"
#include "pines.h"
#include "mando.h"
#include "demanda.h"

struct Boton {
  int pin;
  bool estadoAnt = false;
  bool estadoEstable = false;
  unsigned long tUltimoCambio = 0;
  unsigned long tUltimoFlanco = 0;
};

// Solo A y B. C y D dejaron de ser botones el 31/08: son las camaras de J16, y su
// estado vive mas abajo, en camAnt[], porque no se leen igual ni significan lo mismo.
static Boton b1, b2;
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

static bool disparadoAnt[2] = {false, false};

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
static bool flanco[2] = {false, false};


// ---------------------------------------------------------------------------
// LAS CAMARAS DE J16 - N-97, 31/08/2026.
//
// DESDE AQUI HASTA EL FINAL DEL BLOQUE EL CODIGO ES IDENTICO EN LAS DOS PUNTAS, y hay un
// pack que lo exige (camara_02_j16). Es el motivo de que esto viva en botones.cpp y no en
// un modulo aparte: J16 TIENE UN SOLO DUENO. Repartir el conector entre dos ficheros es
// como PB14 acaba con dos pinMode() de modos distintos y gana el ultimo que corra, sin
// que nadie se entere.
//
// QUE CIERRA ESTO DE N-97. La camara de demanda no era la misma entrada en las dos
// puntas:
//
//   Maestro   pinMode DENTRO de modoInteligente_setup(): fuera de ese modo el pin no
//             estaba ni configurado
//   Esclavo   pinMode en setup(), incondicional
//
// Ahora las dos configuran sus camaras en el arranque y sin condiciones. LA UNICA
// ASIMETRIA QUE QUEDA ESTA RAZONADA: el Maestro lee PB0 POR NIVEL y el Esclavo POR
// FLANCO. No es descuido, es SFTY-27 -el Esclavo PIDE y el Maestro DECIDE-. Una peticion
// es un suceso y viaja bien como flanco; pero la regla del Maestro es "cede el paso si el
// otro pide Y NO hay cola local", y "hay cola AHORA" es un nivel. Un flanco no puede
// contestar a eso.
//
// Las camaras C y D no heredan esa asimetria: las dos puntas las leen IGUAL, por flanco,
// y entran por demanda_solicitar(), que es la unica puerta que ya existe en las dos y el
// unico sitio donde la diferencia entre pedir y decidir esta escrita.

// La lectura de una entrada de camara. ACTIVO EN ALTO -el porque, en pines.h-.
//
// El cuerpo viene LITERAL del leerPinCamara() que estaba en modo_inteligente.cpp: no se
// reescribe logica ya probada solo para renombrarla. Los 5 ms de antirrebote por software
// se mantienen, y en J16 pesan mas que en J14: PB0 lleva el RC de R64+C25 que filtra 1 ms
// por hardware, y PB14/PB15 no llevan mas que el 10K de R67/R68. Aqui el unico antirrebote
// es este.
bool camara_leerPin(uint8_t pin) {
  if (digitalRead(pin) == HIGH) {
    delay(5);
    return (digitalRead(pin) == HIGH);
  }
  return false;
}

// Las dos camaras de J16 y su nivel estable anterior, para tomar la demanda por FLANCO.
// La tabla es una sola y la comparten la siembra del arranque y el lector del loop: dos
// listas de pines separadas son como una acaba con un pin que la otra no tiene.
static const uint8_t CAM_J16[2] = {CAM_C_PIN, CAM_D_PIN};
static bool camAnt[2] = {false, false};

// N-26 APLICADO A LAS CAMARAS. Un contacto YA CERRADO al encender no es una deteccion:
// es un estado, exactamente igual que un boton ya pulsado. Sin sembrar camAnt[], la
// primera vuelta del loop encontraria nivel alto con anterior en false -que es la
// definicion de un flanco- y pediria paso sin que hubiera pasado ningun coche. Un rele de
// camara en reposo cerrado, un contacto trabado o el ruido en el cable hasta la bornera
// dejan el pin alto al encender sin que haya nada delante, y el firmware no los distingue.
//
// Se siembra el nivel REAL: si el contacto ya venia cerrado, no vuelve a pedir hasta que
// se ABRA y se cierre otra vez. Al encender no sabemos cuanto lleva asi, solo que nadie
// acaba de detectar nada.
static void camaras_sembrar() {
  for (int i = 0; i < 2; i++) {
    camAnt[i] = camara_leerPin(CAM_J16[i]);
  }
}

// El rele de la camara mantiene el contacto cerrado ~1 s por deteccion: leer el nivel
// repetiria la peticion en cada vuelta del loop durante todo ese segundo. Lo que pide
// paso es el flanco. La ventana de silencio de demanda_solicitar() -3 s, mas larga que el
// pulso del rele- es la que impide que una misma deteccion se cuente dos veces, y es la
// misma para la camara y para el boton de la app: dos origenes con temporizador propio no
// sabrian nada el uno del otro.
//
// NADA DE AQUI ESCRIBE UN PIN DE LUZ, y no es un detalle de estilo: solo semaforo.cpp los
// toca. Una camara PIDE; no ordena.
static void camaras_actualizar() {
  for (int i = 0; i < 2; i++) {
    const bool ahora = camara_leerPin(CAM_J16[i]);
    if (ahora && !camAnt[i]) {
      demanda_solicitar();
    }
    camAnt[i] = ahora;
  }
}

void botones_setup() {
  // A y B siguen igual: INPUT_PULLUP y pulsador contra masa, activo en BAJO. Son los
  // dos que alimentan las secuencias del mando de reles, y ese camino no se toca.
  pinMode(BOTON1, INPUT_PULLUP);
  pinMode(BOTON2, INPUT_PULLUP);

  b1.pin = BOTON1;
  b2.pin = BOTON2;

  // LAS ENTRADAS DE CAMARA: INPUT PELADO, NUNCA INPUT_PULLUP. El reposo lo fija el
  // pull-down de 10K que la placa ya trae -R64 en PB0; R67 y R68 en PB14 y PB15-, y el
  // contacto seco cierra contra los 3,3 V del propio conector. La cuenta que lo demuestra
  // esta en pines.h: con el pull-up interno el pin se queda en 0,66 V, que es LOW, o sea
  // demanda permanente sin camara y ninguna al cerrarla.
  //
  // SE DECLARAN AQUI, EN EL ARRANQUE, Y NO DENTRO DE UN MODO: eso era N-97. Un pin de
  // entrada declarado por el modo que lo usa solo existe mientras ese modo esta puesto,
  // mientras la otra punta lo declaraba siempre. En el Esclavo PB0 se declara en el
  // setup() de main.cpp, que es su camino de arranque; el resto es igual en las dos.
  pinMode(CAM_DEMANDA_PIN, INPUT);
  pinMode(CAM_C_PIN, INPUT);
  pinMode(CAM_D_PIN, INPUT);

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
  // POR QUE SIGUE IMPORTANDO CON SOLO DOS BOTONES: el 3 -el que EJECUTABA- ya no existe,
  // pero A y B alimentan las secuencias del mando de reles, y un flanco fantasma en el
  // arranque puede ser el primer pulso de una secuencia que nadie dio. Ademas los
  // pulsadores del gabinete van EN PARALELO con el mando de reles: un rele en reposo
  // cerrado, un pulsador trabado o el ruido
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

  Boton *todos[2] = {&b1, &b2};
  for (int i = 0; i < 2; i++) {
    const bool pulsado = (digitalRead(todos[i]->pin) == LOW);
    todos[i]->estadoAnt = pulsado;
    todos[i]->estadoEstable = pulsado;
    todos[i]->tUltimoCambio = millis();  // el antirrebote arranca contando desde AHORA
    disparadoAnt[i] = pulsado;           // pulsado al arrancar = flanco ya consumido
  }

  // Y lo mismo para las camaras, por la misma razon y en el mismo sitio.
  camaras_sembrar();
}

void botones_actualizar() {
  flanco[0] = flancoBoton(b1, 0);
  flanco[1] = flancoBoton(b2, 1);

  // SFTY-21: el mando ve el pulso AQUI, antes de que ningun modo pueda consumirlo, y
  // sin consumirlo el mismo. Que la secuencia se reconozca no puede depender de en
  // que pantalla este el equipo; para eso existe el mando.
  //
  // Solo hay A (Boton 1) y B (Boton 2), que son justo los dos que el mando necesita. El
  // 3 EJECUTABA y el 4 salia, y por eso nunca formaron parte de ninguna secuencia:
  // repetirlos a ciegas podria haber arrancado un modo que nadie pidio. Desde el 31/08
  // sus pines son camaras, asi que esa exclusion ya no hay que sostenerla. Ver mando.cpp.
  if (flanco[0]) mando_registrarPulso(MANDO_A);
  if (flanco[1]) mando_registrarPulso(MANDO_B);

  // Las camaras de J16 se leen en la MISMA vuelta y en el mismo sitio que la botonera,
  // porque comparten conector y porque asi hay un solo punto donde mirar cuando J16 se
  // comporte raro. Van despues del mando a proposito: si un pulso y una deteccion caen
  // en la misma iteracion, la secuencia del operario que esta subido al poste se
  // registra antes que la peticion de un coche.
  camaras_actualizar();
}

static bool consumir(int idx) {
  bool v = flanco[idx];
  flanco[idx] = false;
  return v;
}

bool botonArriba()  { return consumir(0); }
bool botonAbajo()   { return consumir(1); }

// SIN SUJETO: YA NO HAY PIN QUE PUEDA LEVANTAR ESTOS DOS FLANCOS (31/08/2026).
//
// J16 p10 y p12 son camaras, asi que ACEPTAR y CANCELAR no tienen ya pulsador ni rele
// detras. No se BORRAN, y la razon no es comodidad: tienen veintitantos llamadores
// repartidos por nueve ficheros -menu.cpp y los siete modos en el Maestro, el menu en el
// Esclavo-, y borrarlas convertiria una reasignacion de pines en una reescritura del
// control de flujo de cada modo, la salida del Degradado incluida. Ahi es exactamente
// donde se cuelan los errores en un cambio que no deberia cambiar comportamiento.
//
// Devolviendo false el compilador conserva cada punto de uso, y "git grep botonCancelar"
// sigue devolviendo EN UNA SOLA LISTA todo lo que la retirada de C y D se llevo por
// delante. Borrarlas dispersa esa lista en nueve diffs y la vuelve ilegible.
//
// LO QUE SE PIERDE, CENSADO, Y CON QUE SE SUSTITUYE -verificado el 31/08 llamador a
// llamador; si alguna vez falta un sustituto, esto NO se puede hacer-:
//
//   MAESTRO
//     entrar a un modo desde el panel   menu.cpp:111        SET_MODO:AUTO|MANUAL|AMBAR|
//                                                           MENU|ALCANCE|INTELIGENTE|
//                                                           DEGRADADO (bluetooth.cpp:177+)
//     salir de Alcance/Ambar/Auto/      modo_*.cpp          SET_MODO:MENU  (:191)
//     Manual/Inteligente/Hora
//     salir del Degradado               modo_degradado:465  SET_MODO:MENU (:196, sale por
//                                                           el todo-rojo) y el mando B.B.B
//     confirmar tiempos                 modo_auto/manual    SET_TIEMPOS    (:275)
//     dar paso en Manual                modo_manual.cpp:49  MANUAL:CAMBIAR_TURNO (:257)
//     confirmar la hora                 modo_hora.cpp:208   SET_RTC (:295) y
//                                                           REINICIAR_RELOJ (:330)
//
//   ESCLAVO  -- esta punta NO tiene SET_MODO por Bluetooth, asi que el sustituto no es la
//   app sino EL MANDO DE RELES, que sigue entero sobre A y B (PB9/PB13):
//     entrar al Degradado               menu.cpp:227        A.B.A.B -> ACC_DEGRADADO
//                                                           (mando.cpp:148)
//     salir del Degradado               menu.cpp:215        A.A.A -> ACC_OBEDECER (:121)
//                                                           B.B.B -> ACC_AMBAR    (:138)
//
// EFECTO LATERAL QUE VA EN LA DIRECCION BUENA: con ACEPTAR mudo, la pantalla del Esclavo
// no puede bajar del listado, asi que menu_estaAbierto() es siempre falso y el mando deja
// de poder quedarse inhibido por una pantalla que alguien olvido abierta (SFTY-21).
bool botonAceptar() { return false; }
bool botonCancelar(){ return false; }
