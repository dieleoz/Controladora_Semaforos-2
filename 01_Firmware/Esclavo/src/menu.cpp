// ===== src/menu.cpp (ESCLAVO) =====
#include "menu.h"
#include "botones.h"
#include "lcd.h"
#include "modo_degradado.h"
#include "protocolo.h"
#include "reloj.h"
#include "semaforo.h"
#include <stdio.h>

// ---------------------------------------------------------------------------
// N-16 — Navegacion de la pantalla del Esclavo.
//
// Este fichero SOLO mira y pulsa: no decide luces. Lo unico que puede cambiar el
// comportamiento del equipo desde aqui es entrar o salir del Modo Degradado, y
// eso lo hace llamando a modo_degradado.cpp, que es quien lleva las condiciones y
// las transiciones. El ciclo por reloj sigue corriendo aunque el operario navegue
// a otra pantalla o se vaya del menu: apagar el modo por dejar de mirarlo seria
// convertir una pantalla de consulta en un interruptor.
// ---------------------------------------------------------------------------

// El orden NO es casual: ESTADO va primero porque el cursor arranca en la primera
// opcion y solo lee. Los pulsadores estan en paralelo con el mando de reles que
// se acciona desde el piso sin ver la pantalla (SFTY-21): si la primera opcion
// fuera MODO DEGRADADO, un pulso suelto de ACEPTAR llevaria directo a la pantalla
// que puede cambiar la operacion. Asi lo peor que hace un pulso perdido es
// mostrar un diagnostico.
static const char* OPCIONES[2] = {"ESTADO", "MODO DEGRADADO"};
static const int N_OPCIONES = 2;

enum Pantalla {
  P_MENU,
  P_ESTADO,
  P_DEGRADADO,
  P_CONFIRMAR,   // el modo no se activa con una sola pulsacion
  P_RECHAZO
};

static Pantalla pantalla = P_MENU;
static int cursor = 0;
static RechazoDegradado ultimoRechazo = DEG_ACEPTADO;
static unsigned long tRechazo = 0;
static unsigned long tRepintado = 0;

// Las pantallas con reloj se repintan una vez por segundo. Ni mas -el volcado del
// framebuffer a la ST7920 por SPI de software cuesta decenas de milisegundos y
// esa CPU hace falta para atender la radio-, ni menos: los segundos son el dato
// que hace util esta pantalla y un reloj que avanza a saltos no se puede leer
// contra otro.
static const unsigned long REFRESCO_MS = 1000;

// Cuanto se sostiene el cartel de rechazo. Suficiente para leerlo sin que el
// equipo se quede con una pantalla que ya no informa de nada.
static const unsigned long RECHAZO_MS = 6000;

// SFTY-21: regreso automatico al listado por inactividad.
//
// No es una comodidad de interfaz, es lo que sostiene la inhibicion del mando (ver
// menu.h). Mientras haya una pantalla abierta por debajo del listado, las secuencias
// del mando NO se reconocen; si un tecnico baja del gabinete dejando MODO DEGRADADO
// en pantalla, el mando quedaria mudo y desde el suelo no habria forma de saberlo.
//
// Noventa segundos: de sobra para leer la pantalla de estado con calma -sus datos
// cambian solos cada segundo, no hay que pulsar para verlos avanzar- y lo bastante
// corto para que el mando vuelva a estar armado antes de que nadie llegue al piso.
// El regreso NO toca la operacion: solo cambia lo que se dibuja.
static const unsigned long INACTIVIDAD_MS = 90000;
static unsigned long tUltimaPulsacion = 0;

// ---------------------------------------------------------------------------

static const char* pieMenu() {
  // Que el Degradado esta gobernando la luz debe verse SIN entrar a ninguna
  // pantalla. Es el dato que cambia el significado de todo lo demas: un tecnico
  // que no sepa que el cruce va por reloj puede creer que el radio funciona.
  return degradado_gobiernaLuz() ? "MODO DEGRADADO ACTIVO" : NULL;
}

static void pintarMenu() {
  lcd_dibujarMenu(cursor, OPCIONES, N_OPCIONES, pieMenu());
}

static void pintarEstado() {
  lcd_dibujarEstado(reloj_enHora(), reloj_hora(), reloj_minuto(), reloj_segundo(),
                    degradado_huboSync(), degradado_msDesdeSync(),
                    degradado_syncVencida(),
                    protocolo_bytesRecibidos(), protocolo_tramasValidas(),
                    semaforo_nombreEstado());
}

static void pintarDegradado() {
  char detalle[26];
  const char* estadoTxt = degradado_textoEstado();

  if (degradado_gobiernaLuz()) {
    // Con el modo en marcha, lo que importa es la fase y cuanto le queda: es lo
    // que el operario compara contra el otro semaforo para dar por buena la
    // verificacion visual que el procedimiento exige.
    snprintf(detalle, sizeof(detalle), "%s %lus", degradado_textoFase(),
             (unsigned long)degradado_segundosParaCambio());
  } else {
    RechazoDegradado r = degradado_comprobar();
    if (r == DEG_ACEPTADO) {
      snprintf(detalle, sizeof(detalle), "Pulse 3 para entrar");
    } else {
      // El motivo va en su propia linea y la de arriba dice que es un impedimento.
      // Partirlo asi es lo que permite que quepan los 18 caracteres del motivo mas
      // largo sin recortarlo, y un motivo recortado no sirve para arreglar nada.
      estadoTxt = (degradado_estado() == DEG_RENDIDO) ? "RENDIDO 48h. FALTA:"
                                                      : "NO SE PUEDE. FALTA:";
      snprintf(detalle, sizeof(detalle), "%s", degradado_textoRechazo(r));
    }
  }

  lcd_dibujarDegradado(estadoTxt, detalle,
                       degradado_huboSync(), degradado_msDesdeSync(),
                       degradado_syncVencida(), degradado_avisoLimite(),
                       degradado_gobiernaLuz() ? "3=Salir   4=Menu"
                                               : "3=Entrar  4=Menu");
}

static void pintarConfirmacion() {
  // Entrar exige DOS pulsaciones sobre esta pantalla. La asimetria es
  // deliberada: salir del Degradado lleva al equipo hacia el estado seguro y no
  // necesita proteccion, mientras que entrar habilita verdes sin confirmacion del
  // otro extremo. Lo peligroso se hace dificil; lo seguro, facil.
  lcd_dibujarDegradado("CONFIRMAR ENTRADA?", "Verifique el Maestro",
                       degradado_huboSync(), degradado_msDesdeSync(),
                       degradado_syncVencida(), degradado_avisoLimite(),
                       "3=SI entrar   4=NO");
}

static void repintar() {
  switch (pantalla) {
    case P_MENU:       pintarMenu();        break;
    case P_ESTADO:     pintarEstado();      break;
    case P_DEGRADADO:  pintarDegradado();   break;
    case P_CONFIRMAR:  pintarConfirmacion();break;
    case P_RECHAZO:    lcd_dibujarRechazoDegradado(degradado_textoRechazo(ultimoRechazo)); break;
  }
  tRepintado = millis();
}

static void irA(Pantalla p) {
  pantalla = p;
  repintar();
}

// ---------------------------------------------------------------------------

void menu_setup() {
  pantalla = P_MENU;
  cursor = 0;
  tUltimaPulsacion = millis();
  repintar();
}

// SFTY-21: ver el razonamiento completo en menu.h. El listado inicial es el estado
// de reposo -alli A y B solo mueven un cursor entre dos opciones-, asi que no cuenta
// como menu abierto; todo lo que hay por debajo, si.
bool menu_estaAbierto() {
  return pantalla != P_MENU;
}

void menu_loop() {
  // Los cuatro se leen SIEMPRE, en todas las pantallas, aunque alguna no use
  // alguno.
  //
  // Antes era OBLIGATORIO: el flanco se detectaba dentro de cada botonX() y una
  // pantalla que dejara de consultarlo congelaba su estado, de modo que al volver a
  // otra que si lo usaba se disparaba una pulsacion vieja que nadie hizo. Desde
  // SFTY-21 el flanco lo detecta botones_actualizar() al principio del loop y vive
  // una sola iteracion, asi que ese peligro ya no existe; se mantiene la lectura
  // completa porque tambien alimenta la deteccion de inactividad de aqui abajo.
  const bool arriba   = botonArriba();
  const bool abajo    = botonAbajo();
  const bool aceptar  = botonAceptar();
  const bool cancelar = botonCancelar();
  const bool hayPulsacion = arriba || abajo || aceptar || cancelar;

  // SFTY-21: la pantalla vuelve sola al listado si nadie toca nada. Es lo que
  // garantiza que el mando de reles no se quede inhibido para siempre por una
  // pantalla olvidada abierta (ver menu.h).
  if (hayPulsacion) {
    tUltimaPulsacion = millis();
  } else if (pantalla != P_MENU && (millis() - tUltimaPulsacion) >= INACTIVIDAD_MS) {
    cursor = 0;
    irA(P_MENU);
    return;
  }

  bool cambio = false;

  switch (pantalla) {
    case P_MENU:
      if (arriba) { cursor = (cursor + N_OPCIONES - 1) % N_OPCIONES; cambio = true; }
      if (abajo)  { cursor = (cursor + 1) % N_OPCIONES;              cambio = true; }
      if (aceptar) {
        irA(cursor == 0 ? P_ESTADO : P_DEGRADADO);
        return;
      }
      break;

    case P_ESTADO:
      if (cancelar) { irA(P_MENU); return; }
      break;

    case P_DEGRADADO:
      if (cancelar) { irA(P_MENU); return; }
      if (aceptar) {
        if (degradado_gobiernaLuz()) {
          // La salida es inmediata y sin confirmar. Retrasar el camino hacia el
          // estado seguro con una pregunta seria proteger al equipo del operario
          // en el sentido equivocado.
          degradado_salir();
          cambio = true;
        } else {
          irA(P_CONFIRMAR);
          return;
        }
      }
      break;

    case P_CONFIRMAR:
      if (cancelar) { irA(P_DEGRADADO); return; }
      if (aceptar) {
        ultimoRechazo = degradado_entrar();
        if (ultimoRechazo == DEG_ACEPTADO) {
          irA(P_DEGRADADO);
        } else {
          tRechazo = millis();
          irA(P_RECHAZO);
        }
        return;
      }
      break;

    case P_RECHAZO:
      // Cualquier pulsacion lo cierra, y ademas caduca solo: el cartel no puede
      // quedarse puesto tapando el estado real del equipo si nadie vuelve.
      if (hayPulsacion || (millis() - tRechazo) > RECHAZO_MS) { irA(P_DEGRADADO); return; }
      break;
  }

  // Repintado periodico solo donde hay datos que cambian solos. El menu y el
  // cartel de rechazo no se redibujan por tiempo: no tienen nada que actualizar y
  // cada volcado a la LCD son decenas de milisegundos robados al bucle.
  const bool vivo = (pantalla == P_ESTADO || pantalla == P_DEGRADADO || pantalla == P_CONFIRMAR);
  if (cambio || (vivo && (millis() - tRepintado) >= REFRESCO_MS)) {
    repintar();
  }
}
