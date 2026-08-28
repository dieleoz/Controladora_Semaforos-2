// ===== src/mando.cpp =====
#include "mando.h"
#include "coordinador.h"
#include "lcd.h"
#include "menu.h"
#include "modo_automatico.h"
#include "modo_degradado.h"
#include "modo_ambar.h"
#include "semaforo.h"

// ---------------------------------------------------------------------------
// LAS TRES SECUENCIAS
//
//   A . A . A       (<= 12 s)   AUTOMATICO      2 destellos rojos
//   B . B . B       (<= 12 s)   AMBAR interm.   3 destellos rojos
//   A . B . A . B   (<= 18 s)   MODO DEGRADADO  4 destellos rojos
//
// Memotecnia: A es arriba -> SUBE al modo normal. B es abajo -> BAJA al minimo
// seguro. Alternar -> modo especial. Se aprende en un minuto, que es el requisito
// real para alguien que lo usa de madrugada y bajo lluvia.
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
// ocurrido es que el cursor se movio.
// ---------------------------------------------------------------------------

static const unsigned long VENTANA_TRIPLE_MS = 12000;
static const unsigned long VENTANA_CUADRUPLE_MS = 18000;

// Destellos de confirmacion. Contables desde el suelo y SIEMPRE ROJOS: el rojo nunca
// significa "pase", asi que si el operario cuenta mal, el peor caso sigue siendo
// seguro. Destellar los tres colores se descarto porque un conductor lejano podria
// interpretar el verde.
static const uint8_t DESTELLOS_AUTOMATICO = 2;
static const uint8_t DESTELLOS_AMBAR = 3;
static const uint8_t DESTELLOS_DEGRADADO = 4;

// Rechazo: ambar rapido 2 s. Ni destellos rojos -que significan "hecho"- ni nada
// parecido al ambar de fallo.
static const unsigned long RECHAZO_AMBAR_MS = 2000;

enum AccionMando { ACC_NINGUNA, ACC_AUTOMATICO, ACC_AMBAR, ACC_DEGRADADO };

static const uint8_t MAX_PULSOS = 4;
static uint8_t secBoton[MAX_PULSOS];
static unsigned long secTiempo[MAX_PULSOS];
static uint8_t nSec = 0;

static AccionMando pendiente = ACC_NINGUNA;

static void limpiar() { nSec = 0; }

// ---------------------------------------------------------------------------
// Las secuencias NO se reconocen con el menu abierto.
//
// Empezo siendo un afinamiento opcional -evitar que un tecnico que baja tres veces
// con B dispare el ambar, molesto pero inofensivo-. Al anadirse AJUSTAR HORA paso a
// ser REQUISITO, porque el riesgo dejo de ser inofensivo:
//
//    Rafaga accidental de pulsos con el menu abierto
//      -> el cursor llega a AJUSTAR HORA
//      -> unos pulsos mas CONFIRMAN una hora cualquiera
//      -> el reloj queda MARCADO COMO VALIDO con una hora inventada
//
// Eso es exactamente el veneno que SFTY-18 existe para evitar: no la falta de reloj,
// sino un reloj falso que se cree bueno. Y habilitaria el propio Modo Degradado sobre
// una hora inventada.
//
// MODO_HORA se inhibe por el mismo motivo y con mas razon: es la pantalla que escribe
// el RTC.
//
// Que esto deje tambien fuera al B.B.B no es una excepcion a "el ambar funciona desde
// cualquier estado", es que en estas dos pantallas EL EQUIPO YA ESTA EN ESTADO SEGURO:
// el menu mantiene Rojo Fijo con enlace y Ambar Intermitente sin el. No hay nada de lo
// que rescatar a nadie. Y desde el piso se distingue sin ver la pantalla: si las luces
// estan ciclando, el menu no esta abierto.
// ---------------------------------------------------------------------------
static bool secuenciasInhibidas() {
  ModoSistema m = modoActual_get();
  return (m == MENU || m == MODO_HORA);
}

void mando_setup() {
  limpiar();
  pendiente = ACC_NINGUNA;
}

// Ejecuta la accion ya confirmada. Se llama DESPUES de los destellos, nunca antes:
// primero se confirma, luego se actua.
static void ejecutar(AccionMando a) {
  switch (a) {
    case ACC_AUTOMATICO:
      // "A ver si volvio el radio". No necesita proteccion porque el propio sistema se
      // corrige: si el enlace sigue muerto, el Automatico se va solo a ambar
      // (SFTY-6). El peor caso de intentarlo es volver al ambar, que es justo donde se
      // estaba. Y el resultado se ve desde el suelo sin pantalla: luces ciclando, el
      // radio volvio; luces en ambar, sigue muerto.
      //
      // Arranque DIRECTO, sin el asistente de configuracion: desde el suelo no hay
      // pantalla que rellenar.
      modoAutomatico_pedirArranqueDirecto();
      if (modoActual_get() == MODO_AUTOMATICO) {
        modoAutomatico_setup();   // ya estabamos aqui: main no detectaria cambio
      } else {
        modoActual_set(MODO_AUTOMATICO);
      }
      break;

    case ACC_AMBAR:
      modo_ambar_fijarMotivo("Ambar pedido desde", "el mando (B.B.B)");
      if (modoActual_get() == MODO_AMBAR) {
        modo_ambar_setup();
      } else {
        modoActual_set(MODO_AMBAR);
      }
      break;

    case ACC_DEGRADADO:
      if (modoActual_get() == MODO_DEGRADADO) {
        modo_degradado_setup();
      } else {
        modoActual_set(MODO_DEGRADADO);
      }
      break;

    default:
      break;
  }
}

// Arranca la confirmacion y deja la accion pendiente de que termine.
static void confirmarYActuar(AccionMando a, uint8_t destellos) {
  // TODO-ROJO ANTES DE NADA. Las tres acciones detienen el ciclo aqui, en las dos
  // puntas, antes de que la nueva empiece: ninguna transicion del mando puede saltar
  // a verde desde lo que hubiera antes. Ademas deja la maquina del coordinador en un
  // estado definido (C_IDLE, sin nadie en verde) en vez de a mitad de un cambio.
  coordinador_forzarRojoTotal();
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
      // Aunque alguien acierte A.B.A.B por casualidad, el firmware no entra si la hora
      // no esta validada. El mando permite reactivar en campo sin gruas, pero NO
      // saltarse la puesta a punto. Se usa la misma funcion que la entrada por
      // pantalla: una sola puerta, un solo criterio.
      if (modo_degradado_evaluarEntrada() == MDG_OK) {
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
        confirmarYActuar(ACC_AUTOMATICO, DESTELLOS_AUTOMATICO);
        return;
      }
      if (secBoton[nSec - 3] == MANDO_B && secBoton[nSec - 2] == MANDO_B &&
          secBoton[nSec - 1] == MANDO_B) {
        // Sin condiciones y desde cualquier modo en marcha. Es la regla que impide que
        // nadie quede atrapado con un semaforo en estado raro a 5 m de altura.
        confirmarYActuar(ACC_AMBAR, DESTELLOS_AMBAR);
        return;
      }
    }
  }
}

void mando_actualizar() {
  if (pendiente == ACC_NINGUNA) return;
  // Se espera a que la confirmacion termine. Los destellos son la unica realimentacion
  // que tiene el operario; ejecutar a medias dejaria la cuenta incompleta y el
  // operario sin saber que se reconocio.
  if (semaforo_senalEnCurso()) return;

  AccionMando a = pendiente;
  pendiente = ACC_NINGUNA;
  ejecutar(a);
}
