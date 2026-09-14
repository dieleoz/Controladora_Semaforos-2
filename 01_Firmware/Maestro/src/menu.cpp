// ===== src/menu.cpp =====
#include "menu.h"
#include "modos.h"
#include "reloj.h"   // N-31
#include "botones.h"
#include "semaforo.h"
#include "pines.h"
#include "coordinador.h"

static int cursorMenu = 0;

// D-17.bis: LA PANTALLA Y ESTE MENU SE RETIRAN DEL EQUIPO -no del codigo, y el matiz
// ES la decision-. El fichero sigue compilando y su arnes sigue midiendo un framebuffer
// en el PC, que no necesita la pantalla fisica; lo que muere es la INTERFAZ. O sea que
// todo lo que se lee debajo describe una navegacion que ya no puede recorrer nadie: se
// conserva porque de esta lista cuelga el unico armador de algun modo, y retirarla los
// dejaria sin puerta sin que ningun instrumento lo dijera.
// ---------------------------------------------------------------------------
// V8.7 (SFTY-21): el menu se parte en DOS NIVELES.
//
// Historia, porque explica la decision. En V8.1 se anadio PRUEBA ALCANCE (4a opcion)
// y en V8.6 AJUSTAR HORA (5a), que ya obligo a comprimir el interlineado de 11 a 9 px.
// Al llegar MODO DEGRADADO, la sexta linea caia en 24 + 5*9 = 69: fuera de una
// pantalla de 64 px. Y el fallo no habria sido que no se dibujara -la salvaguarda de
// lcd_dibujarMenu() lo impide- sino que el CURSOR SI PODIA LLEGAR HASTA ELLA, dejando
// al operario navegando a ciegas sobre una opcion invisible. Justo encima de la
// opcion que arranca el modo que da verde por reloj.
//
// La causa no era el numero de opciones, era mezclar dos cosas distintas en una lista
// plana:
//
//   MODOS DE OPERACION   MANUAL / AUTOMATICO / INTELIGENTE   se eligen a diario
//   HERRAMIENTAS         PRUEBA ALCANCE / AJUSTAR HORA /     se tocan rara vez, y
//   Y CASOS ESPECIALES   MODO DEGRADADO                      requieren criterio
//
// Separandolas, el menu principal vuelve a 4 opciones -EXACTAMENTE el layout validado
// en campo y en el arnes, base 28 y paso 11- y el submenu se queda en 3, que usa ese
// mismo layout. Ninguno de los dos se acerca al limite, y una septima opcion futura
// tampoco obligaria a comprimir nada.
//
// Y hay un beneficio de seguridad que no es accesorio: con el mando de reles operando
// A CIEGAS desde el suelo, una rafaga accidental de pulsos ya no puede alcanzar
// AJUSTAR HORA ni MODO DEGRADADO, porque estan un nivel por debajo y para bajar hace
// falta el Boton 3, que es el unico que las secuencias del mando tienen prohibido
// (ver mando.cpp). Refuerza POR ESTRUCTURA el requisito de ignorar secuencias con el
// menu abierto, en vez de dejarlo todo colgando de una sola comprobacion.
// ---------------------------------------------------------------------------
enum NivelMenu { NIVEL_RAIZ, NIVEL_CONFIG };
static NivelMenu nivel = NIVEL_RAIZ;

// D-32 (1), 13/09: los ROTULOS de las opciones se van con la pantalla; los INDICES
// se quedan, porque son los que el switch de menu_loop() traduce a modos. El orden es
// el de siempre: 0 MANUAL, 1 AUTOMATICO, 2 INTELIGENTE, 3 CONFIGURACION.
static const int OPCIONES_RAIZ = 4;

// Submenu CONFIGURACION: 0 PRUEBA ALCANCE, 1 AJUSTAR HORA, 2 MODO DEGRADADO,
// 3 REINICIAR RELOJ (N-31).
static const int OPCIONES_CONFIG = 4;

static int cantidadOpciones() {
  return (nivel == NIVEL_RAIZ) ? OPCIONES_RAIZ : OPCIONES_CONFIG;
}

void menu_setup() {
  modoActual_set(MENU);
  // Se vuelve SIEMPRE al nivel raiz. Al salir de un modo el equipo debe aparecer
  // donde el operario espera encontrarlo, y no en un submenu en el que quiza nunca
  // estuvo -por ejemplo tras una vuelta al menu provocada por el propio firmware-.
  nivel = NIVEL_RAIZ;
  cursorMenu = 0;
  // ESTA LINEA ES EL MOTIVO DE QUE menu.cpp SIGA EXISTIENDO DESPUES DE D-32 (1).
  // coordinador_forzarMenu() es el todo-rojo de LAS DOS PUNTAS, y menu_setup() tiene
  // doce llamadores -once modos y el despachador de Bluetooth-, asi que es la puerta
  // por la que la app pide hoy ese todo-rojo con SET_MODO:MENU. Retirar el fichero con
  // la pantalla habria borrado ese camino sin que ningun instrumento lo dijera.
  coordinador_forzarMenu(); // Fuerza Rojo Fijo en Maestro y Esclavo
}

// D-32 (1), 13/09: LA NAVEGACION SE QUEDA Y NO ES UN DESCUIDO.
//
// Lo que se ha retirado de aqui es el dibujo; lo que mueve el cursor y lo que ARMA
// cada modo sigue en pie, y esa es la diferencia entre retirar una pantalla y
// reescribir el control de flujo del equipo. Medido antes de tocar nada:
//
//   - botonAceptar() y botonCancelar() devuelven false SIEMPRE desde el 31/08 (sus
//     pines son camaras, D-2). O sea que este switch NO PUEDE alcanzarse hoy: el
//     cursor se mueve con A y B y no hay forma de confirmar nada. Quitar la
//     navegacion no cerraria ningun camino vivo, y dejarla no abre ninguno.
//   - Dejarla conserva a botonArriba()/botonAbajo() un llamador en esta punta, que es
//     lo que sostiene los DOS caminos de lectura de J16 p5/p8 que D-32 describe.
//   - Y conserva el unico armador de MODO_HORA. Retirarlo obligaria a tocar modos.h y
//     a retirar modo_hora.cpp entero, que es OTRO cambio y no "retirar el lcd".
//
// Lo que ha desaparecido con la pantalla es el sostenimiento de 6 s del resultado de
// REINICIAR RELOJ (N-31): existia para que un mensaje no parpadeara y se perdiera, y
// sin mensaje no sostiene nada. La operacion en si -reloj_reiniciarDominioRespaldo()-
// sigue donde estaba, mas abajo, y ademas la app la pide por REINICIAR_RELOJ.
void menu_loop() {
  bool redibujar = false;
  const int n = cantidadOpciones();

  if (botonArriba()) {
    cursorMenu = (cursorMenu + n - 1) % n;
    redibujar = true;
  }
  if (botonAbajo()) {
    cursorMenu = (cursorMenu + 1) % n;
    redibujar = true;
  }
  if (botonAceptar()) {
    if (nivel == NIVEL_RAIZ) {
      switch (cursorMenu) {
        case 0: modoActual_set(MODO_MANUAL);      return;
        case 1: modoActual_set(MODO_AUTOMATICO);  return;
        case 2: modoActual_set(MODO_INTELIGENTE); return;
        default:
          // CONFIGURACION: baja de nivel. NO arranca ningun ciclo ni cambia el
          // estado de las luces; el equipo sigue en el mismo estado seguro que el
          // menu principal (Rojo Fijo con enlace, Ambar sin el).
          nivel = NIVEL_CONFIG;
          cursorMenu = 0;
          return;
      }
    } else {
      switch (cursorMenu) {
        case 0:  modoActual_set(MODO_ALCANCE);   break;
        case 1:  modoActual_set(MODO_HORA);      break;
        case 2:  modoActual_set(MODO_DEGRADADO); break;
        default: {
          // N-31: se ejecuta AQUI mismo. D-32 (1): su resultado ya no se pinta, asi
          // que se descarta explicitamente -no se deja un valor sin mirar-. Quien
          // necesite el desenlace lo pide por la app: bluetooth.cpp atiende
          // REINICIAR_RELOJ y contesta con el resultado de esta misma llamada.
          (void)reloj_reiniciarDominioRespaldo();
          return;
        }
      }
      return;
    }
  }

  // Boton 4 en el submenu: vuelve al menu principal, no sale a ningun modo. Es la
  // misma salida que el operario ya conoce del resto de pantallas, y deja el cursor
  // sobre CONFIGURACION para que se vea de donde acaba de venir.
  if (botonCancelar() && nivel == NIVEL_CONFIG) {
    nivel = NIVEL_RAIZ;
    cursorMenu = OPCIONES_RAIZ - 1;
    redibujar = true;
  }

  // D-32 (1): `redibujar` era la orden de volcar el framebuffer. Se conserva la
  // variable porque las dos ramas que la ponen son las que mueven el cursor, y
  // borrarla convertiria este bucle en dos `if` sueltos sin nada que los una.
  (void)redibujar;
}
