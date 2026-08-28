// ===== src/mando.cpp (ESCLAVO) =====
#include "mando.h"
#include "menu.h"
#include "modo_degradado.h"
#include "semaforo.h"

// ---------------------------------------------------------------------------
// LAS TRES SECUENCIAS DEL ESCLAVO
//
//   A . A . A       (<= 12 s)   OBEDECER AL MAESTRO   2 destellos rojos
//   B . B . B       (<= 12 s)   AMBAR intermitente    3 destellos rojos
//   A . B . A . B   (<= 18 s)   MODO DEGRADADO        4 destellos rojos
//
// Son las mismas teclas, las mismas ventanas y los mismos destellos que en el
// Maestro, y eso NO es pereza de portar: el operario acciona las dos puntas en la
// misma salida, con el mismo mando en la mano y sin ver ninguna pantalla. Dos
// gramaticas distintas en los dos gabinetes serian dos oportunidades de equivocarse
// de secuencia estando a 5 m de altura.
//
// Lo unico que cambia es la primera accion, y por una razon de fondo: el Maestro
// decide el ciclo, el Esclavo obedece. Donde el Maestro arranca su Modo Automatico,
// aqui se DEVUELVE EL MANDO al Maestro. La memotecnia se conserva: A es arriba ->
// sube al funcionamiento normal; B es abajo -> baja al minimo seguro; alternar ->
// modo especial.
//
// LAS VENTANAS SALEN DE LA MEDIDA DE CAMPO, NO DE UN GUSTO. Con ~2 s por pulsacion,
// tres pulsos son ~6 s y cuatro son ~8 s en el caso comodo; el doble si el operario
// duda o el rele tarda. 12 s y 18 s dejan ese margen sin llegar a ser tan largas como
// para que dos gestos separados se sumen en una secuencia que nadie hizo.
//
// ASIMETRIA DELIBERADA: lo seguro, facil; lo peligroso, dificil.
//
//   Ambar por accidente     -> el equipo va a seguro. Molesto, no peligroso
//   Degradado por accidente -> verde sin confirmar el otro lado
//
// Por eso el Degradado pide cuatro pulsos ALTERNADOS -A.B.A.B no se produce nunca
// navegando: se sube o se baja, no se zigzaguea- y ademas pasa por la validacion del
// firmware. Y si el operario se equivoca a mitad de secuencia, lo unico que ha
// ocurrido es que el cursor se movio entre ESTADO y MODO DEGRADADO.
// ---------------------------------------------------------------------------

static const unsigned long VENTANA_TRIPLE_MS = 12000;
static const unsigned long VENTANA_CUADRUPLE_MS = 18000;

// Destellos de confirmacion. Contables desde el suelo y SIEMPRE ROJOS: el rojo nunca
// significa "pase", asi que si el operario cuenta mal, el peor caso sigue siendo
// seguro. Destellar los tres colores se descarto porque un conductor lejano podria
// interpretar el verde.
static const uint8_t DESTELLOS_OBEDECER = 2;
static const uint8_t DESTELLOS_AMBAR = 3;
static const uint8_t DESTELLOS_DEGRADADO = 4;

// Rechazo: ambar rapido 2 s. Ni destellos rojos -que significan "hecho"- ni nada
// parecido al ambar de fallo.
static const unsigned long RECHAZO_AMBAR_MS = 2000;

enum AccionMando { ACC_NINGUNA, ACC_OBEDECER, ACC_AMBAR, ACC_DEGRADADO };

static const uint8_t MAX_PULSOS = 4;
static uint8_t secBoton[MAX_PULSOS];
static unsigned long secTiempo[MAX_PULSOS];
static uint8_t nSec = 0;

static AccionMando pendiente = ACC_NINGUNA;

// Ver el porque completo en mando.h: mientras vale true, este nodo no obedece las
// ordenes de luz del Maestro.
static bool ambarLocal = false;

static void limpiar() { nSec = 0; }

// ---------------------------------------------------------------------------
// Las secuencias NO se reconocen con el menu abierto.
//
// El riesgo que esto cubre es el mismo que en el Maestro: que una rafaga de pulsos a
// ciegas caiga sobre un cursor capaz de CONFIRMAR algo. En el Esclavo ese cursor solo
// existe por debajo del listado inicial -en la pantalla del Modo Degradado y en su
// confirmacion, donde ACEPTAR activa el modo-, y ademas estar ahi significa que hay
// una persona delante del gabinete: sus pulsaciones son para la pantalla que esta
// mirando, no para el mando.
//
// Lo que NO se puede copiar del Maestro es el criterio literal. Alli el menu es un
// modo del que se sale; aqui la pantalla no se cierra nunca, y tomar "hay algo
// dibujado" como "el menu esta abierto" dejaria el mando muerto SIEMPRE. La
// definicion vive en menu.cpp, junto al regreso automatico al listado por
// inactividad que impide que una pantalla olvidada abierta inhiba el mando de forma
// indefinida.
//
// Que esto deje tambien fuera al B.B.B no contradice "el ambar funciona desde
// cualquier estado": significa que hay alguien delante del gabinete, con la pantalla
// en la mano y el boton 4 para volver al listado en un pulso.
// ---------------------------------------------------------------------------
static bool secuenciasInhibidas() {
  return menu_estaAbierto();
}

void mando_setup() {
  limpiar();
  pendiente = ACC_NINGUNA;
  ambarLocal = false;
}

bool mando_ambarLocal() { return ambarLocal; }

// Ejecuta la accion ya confirmada. Se llama DESPUES de los destellos, nunca antes:
// primero se confirma, luego se actua.
static void ejecutar(AccionMando a) {
  switch (a) {
    case ACC_OBEDECER:
      // "A ver si volvio el radio". No necesita proteccion porque el propio sistema se
      // corrige: si el enlace sigue muerto, este nodo vuelve solo a ambar a los 12 s
      // de silencio (la caida de siempre en main.cpp). El peor caso de intentarlo es
      // volver al ambar, que es justo donde se estaba. Y el resultado se ve desde el
      // suelo sin pantalla: luces ciclando, el radio volvio; luces en ambar, sigue
      // muerto.
      ambarLocal = false;
      if (degradado_gobiernaLuz()) {
        // Salida ORDENADA, por el todo-rojo de despedida que lleva modo_degradado.cpp.
        // Devolver el mando desde un verde por reloj directamente a lo que el Maestro
        // ordene seria encadenar dos autoridades sin cerrar el paso en medio.
        degradado_salir();
      } else {
        // Rojo mientras se espera la primera orden. Es el unico estado en el que se
        // puede quedar un semaforo que ya no decide nada por su cuenta.
        semaforo_forzarRojo();
      }
      break;

    case ACC_AMBAR:
      // Sin condiciones y desde cualquier estado. Es la regla que impide que nadie
      // quede atrapado con un semaforo en estado raro a 5 m de altura.
      ambarLocal = true;
      if (degradado_gobiernaLuz()) {
        // Tambien por el todo-rojo: se sale del Degradado como se sale siempre. Saltar
        // de un verde por reloj a ambar intermitente le daria a quien ya venia lanzado
        // una senal que invita a negociar el paso mientras aun cree tener prioridad.
        // El ambar lo enciende despues el sostenedor de mando_actualizar().
        degradado_salir();
      } else {
        semaforo_iniciarFallo();
      }
      break;

    case ACC_DEGRADADO:
      // El todo-rojo de entrada y la revalidacion de condiciones los hace
      // degradado_entrar(): una sola puerta, un solo criterio.
      ambarLocal = false;
      degradado_entrar();
      break;

    default:
      break;
  }
}

// Arranca la confirmacion y deja la accion pendiente de que termine.
static void confirmarYActuar(AccionMando a, uint8_t destellos) {
  // ROJO ANTES DE NADA. Las tres acciones detienen aqui la luz, antes de que la nueva
  // empiece: ninguna transicion del mando puede saltar a verde desde lo que hubiera
  // antes. En el Esclavo el rojo es solo el de esta punta -no hay coordinador que
  // pare las dos-, y no es una carencia: cada punta se autoriza por separado, que es
  // precisamente lo que hace verificable el procedimiento.
  semaforo_forzarRojo();
  semaforo_destellosRojos(destellos);
  pendiente = a;
  limpiar();
}

static void rechazar() {
  // AMBAR RAPIDO 2 s = RECHAZADO. No se toca el ciclo en curso: el equipo sigue
  // haciendo lo que hacia, porque el operario pidio un cambio que NO se le ha
  // concedido y dejarlo en un estado distinto del que tenia seria concederle otro
  // distinto del que pidio. La senal solo ocupa las luces esos 2 s; al terminar, las
  // salidas se ponen al dia con lo que la logica haya decidido mientras tanto (ver
  // semaforo.cpp).
  semaforo_ambarRapido(RECHAZO_AMBAR_MS);
  limpiar();
}

// Descarta los pulsos demasiado viejos para formar parte de ninguna secuencia. Sin
// esto, dos gestos separados por minutos podrian sumarse en una secuencia que nadie
// hizo.
static void purgarViejos(unsigned long ahora) {
  uint8_t primero = 0;
  while (primero < nSec && (ahora - secTiempo[primero]) > VENTANA_CUADRUPLE_MS) primero++;
  if (primero == 0) return;
  for (uint8_t i = primero; i < nSec; i++) {
    secBoton[i - primero] = secBoton[i];
    secTiempo[i - primero] = secTiempo[i];
  }
  nSec = (uint8_t)(nSec - primero);
}

void mando_registrarPulso(uint8_t boton) {
  if (secuenciasInhibidas()) { limpiar(); return; }

  // Con una confirmacion en curso se ignoran los pulsos nuevos. El operario esta
  // contando destellos, no pulsando; y si pulsa de nervios, no debe encadenarse una
  // segunda accion sobre la primera.
  if (semaforo_senalEnCurso() || pendiente != ACC_NINGUNA) return;

  unsigned long ahora = millis();
  purgarViejos(ahora);

  if (nSec >= MAX_PULSOS) {
    for (uint8_t i = 1; i < MAX_PULSOS; i++) {
      secBoton[i - 1] = secBoton[i];
      secTiempo[i - 1] = secTiempo[i];
    }
    nSec = MAX_PULSOS - 1;
  }
  secBoton[nSec] = boton;
  secTiempo[nSec] = ahora;
  nSec++;

  // A.B.A.B primero. No hay ambiguedad con las otras dos: sus tres ultimos pulsos son
  // B.A.B, que no es ni A.A.A ni B.B.B.
  if (nSec >= 4) {
    if (secBoton[nSec - 4] == MANDO_A && secBoton[nSec - 3] == MANDO_B &&
        secBoton[nSec - 2] == MANDO_A && secBoton[nSec - 1] == MANDO_B &&
        (ahora - secTiempo[nSec - 4]) <= VENTANA_CUADRUPLE_MS) {

      // LA RED DE SEGURIDAD REAL NO ES LA SECUENCIA, ES ESTA COMPROBACION.
      // Aunque alguien acierte A.B.A.B por casualidad, el firmware no entra si falta
      // la hora, la configuracion del ciclo o una sincronizacion vigente. El mando
      // permite activar en campo sin gruas, pero NO saltarse las condiciones de
      // entrada. Se usa la misma funcion que la entrada por pantalla: una sola
      // puerta, un solo criterio.
      if (degradado_comprobar() == DEG_ACEPTADO) {
        confirmarYActuar(ACC_DEGRADADO, DESTELLOS_DEGRADADO);
      } else {
        rechazar();
      }
      return;
    }
  }

  if (nSec >= 3) {
    unsigned long tramo = ahora - secTiempo[nSec - 3];
    if (tramo <= VENTANA_TRIPLE_MS) {
      if (secBoton[nSec - 3] == MANDO_A && secBoton[nSec - 2] == MANDO_A &&
          secBoton[nSec - 1] == MANDO_A) {
        confirmarYActuar(ACC_OBEDECER, DESTELLOS_OBEDECER);
        return;
      }
      if (secBoton[nSec - 3] == MANDO_B && secBoton[nSec - 2] == MANDO_B &&
          secBoton[nSec - 1] == MANDO_B) {
        confirmarYActuar(ACC_AMBAR, DESTELLOS_AMBAR);
        return;
      }
    }
  }
}

void mando_actualizar() {
  // Se espera a que la confirmacion termine. Los destellos son la unica realimentacion
  // que tiene el operario; ejecutar a medias dejaria la cuenta incompleta y el
  // operario sin saber que se reconocio.
  if (pendiente != ACC_NINGUNA && !semaforo_senalEnCurso()) {
    AccionMando a = pendiente;
    pendiente = ACC_NINGUNA;
    ejecutar(a);
  }

  // Sostenedor del ambar pedido con B.B.B.
  //
  // Existe porque la orden del operario tiene que sobrevivir a lo que pase despues:
  // al todo-rojo de salida del Degradado -que termina en INACTIVO, no en ambar- y a
  // cualquier otra cosa que mueva la luz. Se re-arma en vez de encenderse una sola
  // vez porque un ambar que se apaga solo no es un estado seguro, es un parpadeo.
  //
  // No pelea con la senal de confirmacion: mientras haya destellos en curso, las
  // luces son de la senal.
  if (ambarLocal && !semaforo_senalEnCurso() && !degradado_gobiernaLuz() &&
      semaforo_estado() != S_FALLO) {
    semaforo_iniciarFallo();
  }
}
