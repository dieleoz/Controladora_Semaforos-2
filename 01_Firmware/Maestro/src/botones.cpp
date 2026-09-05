// ===== src/botones.cpp =====
#include "botones.h"
#include "pines.h"
#include "mando.h"
#include "demanda.h"
#include "semaforo.h"
#include "bluetooth.h"

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
  // N-118: ACTIVO EN ALTO, y lo decide EL COBRE, no una preferencia.
  //
  // Aqui ponia `== LOW` con los pines en INPUT_PULLUP, y eso llevaba mal desde el primer
  // dia: R65/R66 son 10K A MASA sobre /Boton1 y /Boton2 -medido en el .kicad_pcb y
  // confirmado en banco el 03/09 con 9,92 kOhm-, y J16 reparte 3,3 V en la posicion de
  // al lado de cada boton (p4, p7, p9, p11) con UNA SOLA masa en todo el conector (p2).
  // Un contacto por boton contra masa necesitaria una masa por boton. Solo hay una.
  //
  // Con INPUT_PULLUP, el pull-up interno (30-50 kOhm) contra ese 10K deja el pin en
  // 0,55-0,83 V, por debajo del VIL de 0,99 V: el micro lo lee LOW SIEMPRE. El banco
  // midio 0,6 V, dentro de la horquilla. O sea que los dos botones estaban clavados en
  // "pulsado" y NUNCA producian un flanco: el mando A/B no se podia pulsar.
  //
  // NO ES UNA REGRESION NUESTRA, y conviene saberlo: el repositorio del que salio este
  // firmware -2semaforos_3estados- trae este mismo `== LOW` y el MISMO .kicad_pcb byte a
  // byte (md5 088667eac75207e8dcfa0ce5b93adce6). La contradiccion es original. Su huella
  // esta medida: el ACEPTAR fantasma al arrancar de N-26 (banco del 01/08) es justo lo
  // que hacen unos pines clavados en bajo.
  bool lecturaCruda = (digitalRead(b.pin) == HIGH);

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

// ---------------------------------------------------------------------------------
// EL VIGILANTE - D-13 FASE 1 (A-6), 05/09/2026. IDENTICO EN LAS DOS PUNTAS.
//
// Por que existe, y por que va ANTES que cualquier veto, esta escrito en botones.h.
// Aqui van los dos numeros y de que estan hechos.
//
// -------------------------------------------------------------------------------
// CAM_PEGADA_MS - CUANTO PUEDE DURAR UNA PRESENCIA LEGITIMA BAJO LA PLUMA
// -------------------------------------------------------------------------------
// La camara vigila EL BARRIDO DE LA PLUMA, no la zona de espera (D-13): un vehiculo
// que espera correctamente para ANTES y no entra en la region. Lo unico que la ocupa
// es alguien CRUZANDO, y solo se cruza con la pluma arriba, o sea durante un verde y
// su despeje.
//
// El techo de eso son los MAXIMOS del ciclo, que viven en limites_ciclo.h:
//
//     VERDE_MIN_MAX (15 min) + DESPEJE_SEG_MAX (90 s) = 16,5 min
//
// Por encima de ese techo ninguna cola de vehiculos explica un contacto que lleva
// cerrado SIN ABRIRSE NI UNA VEZ. Se toman 20 min, el primer redondo por encima, y el
// margen no es decorativo: la pluma tambien se queda arriba en S_FALLO -ambar
// intermitente-, donde el paso es libre y la region puede ir muy cargada.
//
// LO QUE ESTA ALARMA NO SABE DISTINGUIR, ESCRITO EN VEZ DE DISIMULADO: un rele trabado
// y un vehiculo parado veinte minutos debajo de la pluma dan EL MISMO NIVEL, y este
// firmware no tiene con que separarlos. Las dos cosas necesitan que alguien vaya a
// mirar, asi que avisar es correcto en los dos casos; lo que NO seria correcto es que
// la trama dijera "averia". Por eso la CAUSA dice CONTACTO_FIJO y no AVERIA.
//
// Y LA DESIGUALDAD NO SE QUEDA EN ESTE COMENTARIO. Es N-71 letra por letra -un techo
// que vivia en prosa dejo dos reintentos inejecutables durante meses-: el pack
// camara_03_vigilante recalcula en cada corrida, leyendo los tres numeros del C++, que
//     CAM_PEGADA_MS > (VERDE_MIN_MAX * 60 + DESPEJE_SEG_MAX) * 1000
static const unsigned long CAM_PEGADA_MS = 1200000UL;

// -------------------------------------------------------------------------------
// CAM_CIEGA_MS - CUANTO PASO ABIERTO SIN UN SOLO FLANCO ES DEMASIADO
// -------------------------------------------------------------------------------
// ESTE NUMERO NO SALE DE NINGUNA CONSTANTE DEL FIRMWARE, Y SE DICE ASI EN VEZ DE
// VESTIRLO DE CUENTA. Cuanto tarda en pasar el siguiente vehiculo es una propiedad de
// LA CARRETERA, no del equipo; fabricarle una derivacion seria el "~1 s del rele" de
// A-7 otra vez: un numero nuestro que despues nos citamos como si fuera del fabricante.
//
// Lo que si es del equipo son las dos cosas que lo acotan, y las dos las comprueba el
// pack:
//
//   1. NO CUENTA TIEMPO DE RELOJ: cuenta tiempo CON LA PLUMA ARRIBA. Con el cruce
//      parado -menu, rojo total, esta punta sin turno- nadie puede cruzar el barrido, y
//      una camara callada esta diciendo la verdad. Ese es el "con el ciclo corriendo"
//      de D-13, medido sobre lo unico que de verdad abre el paso.
//   2. TIENE QUE SER MAYOR QUE CAM_PEGADA_MS. Una camara pegada tampoco da flancos, o
//      sea que su cronometro de silencio corre igual; si este numero fuera el menor, un
//      contacto trabado se anunciaria como CIEGA -el diagnostico CONTRARIO- y el
//      tecnico saldria a buscar un cable cortado teniendo un rele cerrado.
//
// Se toman 6 h de paso abierto. Con el ciclo minimo de D-5 -3 min por sentido- cada
// poste tiene el paso abierto aproximadamente la mitad del tiempo, asi que son del
// orden de 12 h de reloj: NO PUEDE dispararse dentro de una sola noche sin trafico, que
// es el unico silencio largo que es legitimo. Y llega mas de diez veces antes que la
// unica referencia que habia -"lleva 8 dias con presencia", que es como el responsable
// lo descubrio a ojo-.
//
// SE PODRA BAJAR CUANDO HAYA DATOS, y los datos los da esta misma fase 1: sus eventos
// son los que diran cuanto silencio hay de verdad en este cruce. Hoy no hay ninguno.
static const unsigned long CAM_CIEGA_MS = 21600000UL;

// EL ORDEN DE LOS VALORES ES LA GRAVEDAD, y por eso el campo CAM: se resuelve con un
// simple mayor-que en vez de con una cadena de ifs que alguien tendria que mantener
// ordenada. "?" pesa mas que "OK" a proposito; el porque, en botones.h.
enum EstadoCamara { CAM_OK, CAM_DESCONOCIDA, CAM_CIEGA, CAM_PEGADA };

static EstadoCamara camEstado[2] = {CAM_DESCONOCIDA, CAM_DESCONOCIDA};
static unsigned long camAltoDesde[2] = {0, 0};
static unsigned long camSinFlancoMs[2] = {0, 0};
static unsigned long camUltimoFlanco[2] = {0, 0};
static bool camHuboFlanco[2] = {false, false};
static unsigned long camTickAnt = 0;
static bool camPlumaAnt = false;
static uint16_t camVetos = 0;

// Los dos nombres que salen al aire, indexados por la MISMA i que CAM_J16[]. Se
// escriben una vez en vez de repetirse dentro de cada snprintf, por el mismo motivo por
// el que CAM_J16[] es una sola tabla: dos listas paralelas acaban desalineadas y la
// alarma sale culpando a la camara de al lado.
static const char* const CAM_NOMBRE[2] = {"CAM_C", "CAM_D"};

const char* camara_estado() {
  EstadoCamara peor = camEstado[0] > camEstado[1] ? camEstado[0] : camEstado[1];
  switch (peor) {
    case CAM_PEGADA:      return "PEGADA";
    case CAM_CIEGA:       return "CIEGA";
    case CAM_DESCONOCIDA: return "?";
    default:              return "OK";
  }
}

uint16_t camara_vetosPluma() {
  return camVetos;
}

// La alarma de una camara. Las dos que hay salen por esta puerta para que el peor caso
// de bytes sea UNO SOLO y se pueda medir: el $ALARM lleva ademas el tramo del enlace y
// la hora, y N-108 costo una trama truncada por no haber hecho antes esta cuenta. El
// pack la rehace leyendo estos literales y el snprintf de bluetooth.cpp.
//
// ACCION:NINGUNA NO ES RELLENO. Ese campo significa "medida de seguridad vial
// ejecutada", y la fase 1 no ejecuta ninguna: no veta, no baja la pluma, no toca una
// luz. Escribir ahi cualquier otra cosa seria el $ACK que no mira lo que devolvio la
// llamada (CLAUDE.md 6), trasladado a la caja negra.
static void camara_alarmar(int i, const char* evento, const char* motivo) {
  char causa[28];
  snprintf(causa, sizeof(causa), "%s_%s", CAM_NOMBRE[i], motivo);
  bluetooth_reportarAlarma(evento, causa, "NINGUNA");
}

// Una alarma que no se cierra nunca deja al operario sin saber si aquello se arreglo:
// las dos vuelven a OK por su prueba contraria, y las dos lo dicen.
static void camara_recuperada(int i) {
  char detalle[24];
  snprintf(detalle, sizeof(detalle), "%s_RECUPERADA", CAM_NOMBRE[i]);
  bluetooth_reportarEvento("CAMARA", detalle);
}

// UN FLANCO ES LA UNICA PRUEBA POSITIVA DE QUE LA CAMARA VE. Reinicia su cronometro de
// silencio y, si estaba anunciada CIEGA, la devuelve a OK con su evento.
//
// AQUI EL ESTADO NO PUEDE SER PEGADA, y por eso no se pregunta por el: un flanco es una
// subida, o sea que el contacto tuvo que ABRIRSE antes, y abrirse es justo lo que cierra
// PEGADA en vigilante_nivel(). Preguntarlo seria una guarda que no puede ser falsa
// (CLAUDE.md 3.septies).
static void vigilante_flanco(int i, unsigned long ahora) {
  camUltimoFlanco[i] = ahora;
  camHuboFlanco[i] = true;
  camAltoDesde[i] = ahora;
  camSinFlancoMs[i] = 0;
  if (camEstado[i] == CAM_CIEGA) {
    camara_recuperada(i);
  }
  camEstado[i] = CAM_OK;
}

// EL NIVEL SOSTENIDO. Es la unica de las dos alarmas que puede dispararse SIN que la
// camara haya dado nunca un flanco -un contacto que ya venia cerrado al encender-, y
// por eso su cronometro se siembra en camaras_sembrar() y no aqui: sin esa siembra, el
// rele trabado desde ANTES del arranque seria precisamente el unico que no se detecta.
// Es N-26 aplicado al vigilante.
static void vigilante_nivel(int i, bool alto, unsigned long ahora) {
  if (!alto) {
    if (camEstado[i] == CAM_PEGADA) {
      camEstado[i] = CAM_OK;
      camara_recuperada(i);
    }
    return;
  }
  if ((ahora - camAltoDesde[i]) >= CAM_PEGADA_MS && camEstado[i] != CAM_PEGADA) {
    camEstado[i] = CAM_PEGADA;
    camara_alarmar(i, "CAM_PEGADA", "CONTACTO_FIJO");
  }
}

// EL RELOJ DEL VIGILANTE, y el unico sitio del fichero donde se mira la pluma.
//
// EL SILENCIO SE ACUMULA SOLO CON LA PLUMA ARRIBA, y ese es el "con el ciclo corriendo"
// de D-13: con la pluma abajo nadie puede cruzar el barrido y una camara callada no
// esta fallando. Se lee semaforo_plumaArriba(), que devuelve LO QUE escribirPines() dejo
// puesto en el pin y no una segunda formula de SFTY-28 que alguien tendria que mantener
// igual a la primera.
static void vigilante_tick(unsigned long ahora) {
  const bool arriba = semaforo_plumaArriba();
  const unsigned long dt = ahora - camTickAnt;
  camTickAnt = ahora;

  if (arriba) {
    for (int i = 0; i < 2; i++) {
      camSinFlancoMs[i] += dt;
      if (camSinFlancoMs[i] >= CAM_CIEGA_MS
          && camEstado[i] != CAM_CIEGA && camEstado[i] != CAM_PEGADA) {
        camEstado[i] = CAM_CIEGA;
        camara_alarmar(i, "CAM_CIEGA", "SIN_FLANCO");
      }
    }
  }

  // EL CONTADOR QUE JUSTIFICA LA FASE 2, Y NADA MAS QUE ESO.
  //
  // Se mira en el instante en que la pluma ACABA DE BAJAR, que es el mismo en el que la
  // fase 2 la habria dejado arriba. Esto OBSERVA una transicion ya hecha: vetarla
  // exigiria entrar en escribirPines(), y eso es SFTY-28 y necesita derogacion escrita
  // (A-1.bis). Por eso el contador se construye ANTES que el veto y no despues: es el
  // que dice si el veto merece la pena.
  //
  // "Hay presencia" se contesta con el nivel de la vuelta anterior o con un flanco
  // todavia vigente, y "vigente" NO se inventa aqui: es el mismo plazo con el que este
  // firmware ya decide que una demanda sigue en pie -demanda_ventanaMs()-.
  //
  // camHuboFlanco[] existe por N-26: sin el, con millis() casi en cero contra un
  // camUltimoFlanco[] tambien en cero la resta cae DENTRO de la ventana, y la primera
  // bajada de pluma tras el arranque contaria un veto que nadie provoco.
  if (camPlumaAnt && !arriba) {
    bool presencia = false;
    for (int i = 0; i < 2; i++) {
      if (camAnt[i]
          || (camHuboFlanco[i] && (ahora - camUltimoFlanco[i]) <= demanda_ventanaMs())) {
        presencia = true;
      }
    }
    if (presencia) {
      if (camVetos < 65535) {
        camVetos++;
      }
      char detalle[28];
      snprintf(detalle, sizeof(detalle), "VETO_HABRIA_ACTUADO_N:%u", camara_vetosPluma());
      bluetooth_reportarEvento("CAMARA_PLUMA", detalle);
    }
  }
  camPlumaAnt = arriba;
}

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
  const unsigned long ahora = millis();
  camTickAnt = ahora;
  camPlumaAnt = semaforo_plumaArriba();
  for (int i = 0; i < 2; i++) {
    camAnt[i] = camara_leerPin(CAM_J16[i]);
    // Y EL CRONOMETRO DE PEGADA SE SIEMBRA IGUAL, por el mismo motivo una capa mas
    // abajo: un contacto ya cerrado no va a dar el flanco que lo pondria en marcha, asi
    // que sin esta linea el rele trabado desde antes del encendido -el caso que mas
    // tarda en descubrirse solo- seria justo el unico que no se detecta nunca.
    camAltoDesde[i] = ahora;
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
// toca. Una camara PIDE; no ordena. El vigilante que cuelga de estas mismas lecturas
// tampoco: cuenta y avisa.
static void camaras_actualizar() {
  const unsigned long ahora = millis();
  vigilante_tick(ahora);

  for (int i = 0; i < 2; i++) {
    const bool nivel = camara_leerPin(CAM_J16[i]);
    if (nivel && !camAnt[i]) {
      demanda_solicitar();
      vigilante_flanco(i, ahora);
    }
    vigilante_nivel(i, nivel, ahora);
    camAnt[i] = nivel;
  }
}

void botones_setup() {
  // N-118: A y B pasan a INPUT PELADO, igual que las camaras C y D. Los CUATRO pines de
  // J16 son electricamente identicos -10K a masa y 3,3 V en la posicion de al lado-, asi
  // que se leen igual. Aqui ponia INPUT_PULLUP "y ese camino no se toca": el camino sigue
  // siendo el mismo -las secuencias del mando-, lo que cambia es que ahora se puede
  // recorrer. El porque completo, en actualizar().
  pinMode(BOTON1, INPUT);
  pinMode(BOTON2, INPUT);

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
    // N-118: la siembra lee con la MISMA polaridad que actualizar(). Si aqui quedara un
    // `== LOW` con el resto en ALTO, un boton suelto se sembraria como "pulsado" y el
    // primer flanco de verdad se perderia: la guarda de N-26 se comeria la pulsacion
    // buena en vez de la fantasma. Las dos lecturas se cambian juntas o ninguna.
    const bool pulsado = (digitalRead(todos[i]->pin) == HIGH);
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
