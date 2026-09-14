// ===== src/semaforo.cpp =====
#include "semaforo.h"
#include "pines.h"
// D-33: la pluma pregunta a las camaras antes de bajar. El porque, en escribirPines().
#include "botones.h"

static EstadoSemaforo estado = S_ROJO;
static unsigned long tCambio = 0;

// --- SFTY-21: senal del mando de reles -------------------------------------
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

// --- N-82: test de lamparas ------------------------------------------------
// La bandera vive AQUI ARRIBA, y no junto a semaforo_iniciarTestLeds() donde estaba,
// porque escribirPines() tiene que poder consultarla: la talanquera cuelga del mismo
// 'verde' que enciende la lampara y hay que saber si ese verde es un paso concedido
// o una lampara que se esta ensenando.
static bool testLedsActivo = false;
static unsigned long tInicioTest = 0;

// Cada lampara se ensena 2 s: es lo que tarda un tecnico en confirmarla mirando hacia
// arriba, y el total -tres fases, 6 s- es lo que aguanta sin bajar la vista. El
// numero se escribe una vez y las tres fases se cuentan sobre el, para que no puedan
// desincronizarse entre ellas.
static const unsigned long TEST_FASE_MS = 2000;

// --- N-153: LO QUE LA PLUMA ESTA HACIENDO, PARA PODER PUBLICARLO -----------
//
// La talanquera es el unico elemento del equipo que SE MUEVE, y el conductor le hace
// mas caso que a la lampara. Hasta hoy no salia del micro: el $STATUS no la llevaba y
// la app solo la nombraba dentro de los textos que explican botones, nunca como
// estado, asi que el operario no podia saber si la barrera estaba arriba o abajo.
//
// GUARDA LO QUE SE ESCRIBIO EN EL PIN. No es una segunda cuenta de la condicion: se
// asigna dentro de la propia orden de escribirPines(), que es el unico sitio que
// decide (SFTY-28).
static bool plumaAbierta = false;

// --- D-33: LA PLUMA BAJA UNOS SEGUNDOS DESPUES DEL ROJO --------------------
//
// TRES SEGUNDOS, ELEGIDOS POR EL RESPONSABLE EL 14/09/2026. No es un numero derivado y
// no se defiende como tal. El razonamiento completo -el dato que tuvo delante, los dos
// peligros uno a cada lado, y por que el static_assert es necesario pero NO suficiente-
// esta en el mismo bloque del semaforo.cpp del Maestro, y no se resume aqui para que no
// puedan divergir.
//
// ESTA PUNTA NO TIENE limites_ciclo.h, Y SE DICE EN VEZ DE INVENTARSE UNA CONSTANTE. El
// suelo vial del despeje lo FIJA y lo HACE CUMPLIR el Maestro -DESPEJE_SEG_MIN, la
// guarda de SET_TIEMPOS-; el Esclavo aplica el despeje que le mandan por radio y no
// tiene voto. Se declara aqui el suelo HEREDADO para poder escribir la misma
// desigualdad, y quien impide que las dos copias se separen NO es la disciplina: es
// barrera_03_talanquera, que en cada corrida lee DESPEJE_SEG_MIN del limites_ciclo.h
// del Maestro y exige que sea este mismo numero (N-71).
static const unsigned long DESPEJE_MIN_HEREDADO_SEG = 10UL;
static const unsigned long PLUMA_RETARDO_BAJADA_MS = 3000UL;

// N-71: LA DESIGUALDAD NO SE QUEDA EN EL COMENTARIO DE ARRIBA. Si alguien sube el
// retardo o baja el suelo vial del despeje, esto no compila.
static_assert(PLUMA_RETARDO_BAJADA_MS * 2UL <= DESPEJE_MIN_HEREDADO_SEG * 1000UL,
              "D-33: el retardo de bajada de la pluma se come mas de la MITAD del "
              "todo-rojo mas corto que el ciclo permite (DESPEJE_SEG_MIN). Con eso la "
              "pluma puede seguir arriba cuando la otra punta abre su verde, que es "
              "justo el accidente que el veto viene a evitar");

// EL CIERRE PEDIDO Y TODAVIA NO EJECUTADO. Una sola bandera, y contesta a UNA pregunta:
// "la luz ya no pide la pluma arriba, pero el pin sigue en ABRIR". Es lo que permite que
// semaforo_actualizar() vuelva a pasar por la puerta sin recalcular la condicion de
// SFTY-28 por segunda vez -dos formulas que alguien tendria que mantener iguales es lo
// que este fichero lleva evitando desde N-153-.
static bool plumaCierrePendiente = false;
static unsigned long tPlumaCierrePedido = 0;

// Y ESTA CONTESTA A OTRA, POR ESO SON DOS Y NO UNA (CLAUDE.md 8): "el pin sigue en ABRIR
// PORQUE UNA CAMARA VE ALGO DEBAJO". La lee botones.cpp para contar los vetos que de
// verdad actuan; plumaCierrePendiente tambien es cierta durante el retardo, donde no hay
// veto ninguno, y una sola bandera para las dos preguntas no contestaria bien a ninguna.
static bool plumaVetada = false;

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
  // la regla 6 extendida. Si un modo pudiera mover la barrera por su cuenta, la barrera
  // y la luz podrian decir cosas distintas sin que nadie lo hubiera decidido. Aqui no
  // puede: se escribe con el mismo 'verde' YA enclavado que acaba de encender la lampara.
  //
  // ~~"una pluma arriba con la luz en rojo es PEOR que no tener barrera, porque el
  // conductor confia en ella"~~ -> DEROGADO POR EL RESPONSABLE EL 14/09/2026, dentro de
  // D-33, y se deja tachado en vez de borrado porque esa frase era el sosten del
  // argumento que estuvo a punto de parar este cambio (CLAUDE.md 7.4). Sus palabras:
  // "el veto es solo para la barrera con el problema... esas barreras son casi de
  // adorno, EL QUE MANDA ES EL SEMAFORO Y SU ESTADO". O sea: LA BARRERA NO ES PARTE DEL
  // ENCLAVAMIENTO. Quien reparte el paso es la luz; la pluma protege a quien esta
  // DEBAJO DE ELLA y a nadie mas, y por eso el veto es LOCAL a este poste, no para el
  // ciclo, no viaja en el ACK_RED y no retrasa el verde de la otra punta.
  //
  // LO QUE ESA DEROGACION CUESTA, ESCRITO EN VEZ DE DISIMULADO: habra ratos de luz roja
  // con la pluma arriba mientras la otra punta tiene verde. Es un estado DISENADO, no
  // una averia -por eso PLUMA: se publica desde N-153-, y el operario tiene que poder
  // distinguirlo: lo dice el $EVENT de vigilante_tick() cuando el veto se sostiene.
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
  //
  // N-82: Y NO SIGUE AL VERDE DE UN TEST DE LAMPARAS. El test enciende el verde para
  // que se vea la lampara, no para dar paso; con la condicion anterior una prueba de
  // taller abria la barrera 2 s en un cruce en servicio. La distincion va DENTRO de
  // esta condicion, en la unica funcion que escribe el pin, y no en un segundo
  // digitalWrite dentro del bloque del test: la regla 6 dice que todo sale por esta
  // puerta, y una barrera con dos puertas no es una barrera.
  //
  // Verde encendido con la pluma abajo es la direccion segura y esta admitida: la
  // barrera puede ser MAS restrictiva que la lampara -el arnes del automatico solo
  // exige lo contrario, que no haya pluma arriba sin verde-. Al reves seria una
  // invitacion a entrar que nadie autorizo.
  //
  // Y EL S_FALLO DE ARRIBA NO CONTRADICE AL REPOSO DE pines.h, que es lo que confunde
  // al leerlo (anotado el 04/09/2026, con la politica reconfirmada por el responsable).
  // S_FALLO es un fallo CONOCIDO con el firmware VIVO y decidiendo, y decide abrir. Un
  // equipo SIN ENERGIA no ejecuta esta linea: el pin cae a LOW, el MOSFET no conduce y
  // la pluma BAJA -SFTY-28-. Equipo vivo que sabe que fallo, abre; equipo muerto,
  // cierra. Las dos son ciertas y hay que leerlas juntas.
  //
  // N-153: Y LA MISMA ORDEN DEJA ANOTADO LO QUE ACABA DE MANDAR, para que el $STATUS
  // pueda publicarlo. La asignacion va DENTRO del parentesis de la condicion, y no en
  // una linea de al lado, porque una segunda escritura de esta formula seria una
  // SEGUNDA COPIA: el dia que la condicion cambie -y va a cambiar, D-13 trae el veto de
  // la pluma- la copia se queda vieja sin que nada falle, que es lo que este
  // repositorio lleva pagando. Aqui el pin y la bandera salen del mismo parentesis y no
  // pueden discrepar.
  //
  // =========================================================================
  // D-33 (14/09/2026) - LA CAMARA VETA LA BAJADA, Y LA BAJADA LLEVA RETARDO
  // =========================================================================
  //
  // DEROGA SFTY-28 EN SU "NUNCA AL REVES": la pluma sigue a la luz SALVO QUE HAYA
  // PRESENCIA. Las palabras del responsable: "es su sensor de presencia; si no baja por
  // la camara da igual, es justo ese el punto de la funcion de la camara: que la barrera
  // no se lleve una moto o un carro. Por eso incluso baja segundos despues de que el
  // semaforo cambie a rojo".
  //
  // LO QUE NO CAMBIA, Y ES LA MITAD QUE IMPORTA:
  //
  //   SUBIR SIGUE SIENDO INSTANTANEO Y SIGUE COLGANDO SOLO DE LA LUZ. Ni el retardo ni
  //   el veto pueden ABRIR la barrera: las dos condiciones nuevas solo saben RETENERLA
  //   ARRIBA cuando ya lo estaba. Una camara no puede levantar una pluma, igual que no
  //   puede encender un verde: eso sigue siendo la barrera de salidas.
  //
  //   SIGUE SALIENDO POR ESTA PUERTA Y SOBRE EL 'verde' YA ENCLAVADO. No hay un segundo
  //   digitalWrite en ningun sitio, ni un modo que mueva la pluma por su cuenta.
  //
  // EL ORDEN DE LAS TRES RAZONES NO ES ESTETICO (CLAUDE.md 9: una inversion que solo
  // mira el RESULTADO aprueba un firmware con las barreras en el ORDEN equivocado):
  //
  //   1. LA LUZ. Si la luz pide la pluma arriba, arriba, y se olvidan las dos banderas.
  //      Un veto que sobreviviera al verde siguiente seria un veto pegado.
  //   2. EL RETARDO. Mientras corre, la pluma se queda arriba PASE LO QUE PASE: el que
  //      entro con el verde sigue dentro y no hace falta que ninguna camara lo vea. Va
  //      ANTES del veto a proposito, porque es el unico tramo que NO depende de que el
  //      aparato de fuera funcione.
  //   3. EL VETO. Cumplido el retardo, la pluma solo baja si NINGUNA camara ve nada.
  //
  // VETA CUALQUIERA DE LAS DOS, NO HACE FALTA CONSENSO. camara_presenciaJ16() ya
  // contesta con un OR sobre las dos entradas de J16, y es la UNICA definicion de "hay
  // alguien" que tiene el firmware. El consenso -exigir que las dos vean- seria la
  // eleccion peligrosa: con una camara muerta desde la instalacion -que el vigilante NO
  // detecta (D-25)- el AND no se cumpliria jamas y el veto no existiria nunca, en
  // silencio. Con el OR, una camara muerta solo hace que vete la otra.
  //
  // LO QUE ESTO CUESTA, ESCRITO EN VEZ DE DISIMULADO: una camara que ve presencia para
  // siempre deja la pluma ARRIBA para siempre. NO SE LE PONE TOPE QUE LA BAJE -A-1.bis:
  // un tope que baja igual devuelve el peligro que el veto evita; tope es ALARMA, no
  // accion-. Quien avisa es el vigilante de botones.cpp: CAM_PEGADA a los 20 min si el
  // contacto se queda cerrado, y el $EVENT del contador en cuanto el veto actua.
  const bool luzPideArriba = (verde && !testLedsActivo) || estado == S_FALLO;
  bool plumaArriba;
  if (luzPideArriba) {
    plumaCierrePendiente = false;
    plumaVetada = false;
    plumaArriba = true;
  } else if (!plumaAbierta) {
    // Ya estaba abajo: no hay bajada que retrasar ni que vetar. Sin esta rama, una
    // camara pegada impediria que la pluma BAJARA una vez y despues impediria que
    // volviera a estar abajo, que no es lo mismo y es absurdo.
    plumaCierrePendiente = false;
    plumaVetada = false;
    plumaArriba = false;
  } else {
    if (!plumaCierrePendiente) {
      plumaCierrePendiente = true;
      tPlumaCierrePedido = millis();
    }
    const bool enRetardo = (millis() - tPlumaCierrePedido) < PLUMA_RETARDO_BAJADA_MS;
    plumaVetada = !enRetardo && camara_presenciaJ16();
    plumaArriba = enRetardo || plumaVetada;
    if (!plumaArriba) plumaCierrePendiente = false;
  }
  digitalWrite(MOTOR_TALANQUERA,
               (plumaAbierta = plumaArriba) ? TALANQUERA_ABRIR : TALANQUERA_CERRAR);
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
  // La bandera dice lo que dice el pin, tambien aqui: este digitalWrite no pasa por
  // escribirPines(), asi que es el unico sitio donde hay que repetirlo.
  plumaAbierta = false;
  // D-33: y con ella las dos banderas del cierre. Un arranque no hereda un retardo a
  // medias ni un veto de antes del reinicio: la pluma ya esta abajo, que es el unico
  // estado desde el que las dos sobran.
  plumaCierrePendiente = false;
  plumaVetada = false;

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

void semaforo_iniciarTestLeds() {
  // SIN GUARDA, Y ES DELIBERADO. La tentacion era rechazar aqui el test cuando una
  // senal del mando ocupa las luces. Seria un rechazo MUDO: esta funcion no devuelve
  // nada y el $ACK de bluetooth.cpp se manda igual, asi que el tecnico se iria del
  // poste con una confirmacion de algo que no ocurrio. La espera se resuelve en
  // semaforo_actualizar(), donde no hay que prometer nada.
  testLedsActivo = true;
  tInicioTest = millis();
}

bool semaforo_testLedsEnCurso() {
  return testLedsActivo;
}

// N-153: lo ULTIMO que se le mando al pin de la pluma. Lo publica el campo PLUMA: del
// $STATUS; ver el porque de que sea una bandera y no un recalculo sobre plumaAbierta.
bool semaforo_plumaArriba() {
  return plumaAbierta;
}

// D-33: true mientras la pluma esta arriba PORQUE UNA CAMARA VE ALGO. Lo lee el
// contador de botones.cpp -que es quien tiene el bluetooth- para saber cuando el veto
// ACTUA. Ver el porque de que sea una bandera distinta de plumaAbierta en su
// declaracion: son dos preguntas.
bool semaforo_plumaVetada() {
  return plumaVetada;
}

void semaforo_iniciarFallo() {
  estado = S_FALLO;
  tCambio = millis();
  aplicarSalidas(LOW, LOW, LOW); // Empieza apagado, luego parpadea en actualizar()
}

void semaforo_actualizar() {
  unsigned long ahora = millis();

  // Test de lámparas en taller (6 segundos: 2s Rojo -> 2s Amarillo -> 2s Verde)
  if (testLedsActivo) {
    // CON UNA SENAL DEL MANDO EN CURSO, EL TEST ESPERA: no se abandona ni corre por
    // debajo. Dos motivos, y ninguno es cortesia con el mando.
    //
    // No corre por debajo porque aplicarSalidas() con senalActiva guarda y NO escribe:
    // el test gastaria sus seis segundos sin encender una lampara y un tecnico leeria
    // eso como tres lamparas fundidas. Y porque el return de mas abajo dejaria
    // actualizarSenal() sin llamar: la senal no terminaria nunca, senalActiva se
    // quedaria en true y aplicarSalidas() no volveria a escribir un pin en toda la
    // vida del equipo.
    //
    // No se abandona porque el $ACK de TEST_LEDS ya salio: tirar la peticion en
    // silencio seria la misma mentira por otro camino. Re-armando el reloj, el test
    // empieza entero en cuanto la senal suelta las luces.
    if (senalActiva) {
      tInicioTest = ahora;
    } else {
      unsigned long elapsed = ahora - tInicioTest;
      if (elapsed < TEST_FASE_MS) {
        aplicarSalidas(true, false, false);
      } else if (elapsed < 2 * TEST_FASE_MS) {
        aplicarSalidas(false, true, false);
      } else if (elapsed < 3 * TEST_FASE_MS) {
        // El verde del test pasa por el enclavamiento como cualquier otro: si algun
        // dia SFTY-2 se lo niega, esta fase se queda sin encender y eso es la
        // respuesta correcta, no un estorbo que rodear. La pluma no lo sigue -ver
        // escribirPines()-, asi que el tecnico ve la lampara sin que se abra la via.
        aplicarSalidas(false, false, true);
      } else {
        testLedsActivo = false;
        aplicarSalidas(true, false, false);
      }
      return;
    }
  }

  // SFTY-21: la senal se atiende ANTES y NO se sale de la funcion. La logica de
  // abajo tiene que seguir corriendo aunque la senal ocupe las luces; si se
  // devolviera aqui, una transicion a verde pedida justo antes se quedaria congelada
  // y quien la espere -el Maestro por radio o el Modo Degradado- aguardaria
  // indefinidamente un estado que nadie va a alcanzar.
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

  // D-33: HAY QUE VOLVER A PASAR POR LA PUERTA, Y ESTE ES EL SITIO.
  //
  // Antes de D-33 la pluma solo dependia de los argumentos de escribirPines(), asi que
  // bastaba con que se la llamara en cada CAMBIO de luz. Ahora depende ademas del RELOJ
  // -el retardo- y de un aparato de fuera -la camara-, y ninguna de las dos cosas
  // provoca una llamada por si sola: en rojo estable, sin senal del mando y sin
  // S_FALLO, nadie vuelve a escribir un pin hasta la transicion siguiente. Sin esta
  // linea la pluma se quedaria arriba HASTA EL PROXIMO CAMBIO DE LUZ, que es minutos.
  //
  // NO ES UN SEGUNDO ESCRITOR: se vuelve a entrar por aplicarSalidas() con lo ultimo
  // que pidio la logica -ultR/ultA/ultV-, o sea que el enclavamiento SFTY-2 y la
  // intercepcion de SFTY-21 siguen delante. Y la condicion es la BANDERA que dejo
  // puesta escribirPines(), no una segunda copia de la formula de SFTY-28.
  if (plumaCierrePendiente) aplicarSalidas(ultR, ultA, ultV);
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