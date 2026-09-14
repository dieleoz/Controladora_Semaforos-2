// ===== src/menu.cpp (ESCLAVO) =====
#include "menu.h"

// ---------------------------------------------------------------------------
// D-32 (1), 13/09/2026 — SALE EL LCD. DE ESTE FICHERO SOBREVIVE UNA SOLA COSA, Y NO
// ES UNA PANTALLA: menu_estaAbierto(), que es la puerta que INHIBE las secuencias del
// mando de reles (SFTY-21) y que lee Esclavo/src/mando.cpp.
//
// LO QUE HABIA AQUI Y POR QUE SE VA ENTERO. Este fichero era la navegacion de la
// pantalla del Esclavo: cinco pantallas (P_MENU, P_ESTADO, P_DEGRADADO, P_CONFIRMAR y
// P_RECHAZO), su cursor, el repintado de una vez por segundo, el regreso al listado
// por inactividad a los 90 s y el cartel de rechazo de 6 s. Todo eso dibujaba, y
// dibujar es lo que D-32 (1) retira.
//
// LO QUE NO SE PIERDE, MEDIDO Y NO SUPUESTO -esto es lo unico que importa aqui, porque
// desde este fichero se podia ENTRAR Y SALIR del Modo Degradado, que es el unico modo
// que da verde sin confirmar la otra punta-:
//
//   Las dos llamadas que cambiaban la operacion eran degradado_entrar() en P_CONFIRMAR
//   y degradado_salir() en P_DEGRADADO. LAS DOS ESTABAN YA MUERTAS ANTES DE ESTE
//   COMMIT, y no por descuido sino por D-2: para llegar a cualquiera de esas dos
//   pantallas hace falta botonAceptar(), y botonAceptar() es `return false;` en
//   botones.cpp desde el 31/08, cuando J16 p10 y p12 pasaron a ser camaras. El cursor
//   no podia bajar del listado inicial. O sea que aqui no se retira ningun camino
//   vivo: se retira codigo al que no se llegaba.
//
//   Y NO SE QUEDAN SIN PUERTA, que es la comprobacion que de verdad hay que hacer:
//     - degradado_entrar() la llaman ademas bluetooth.cpp (la orden por app de D-18) y
//       mando.cpp (la secuencia A.B.A.B).
//     - degradado_salir() la llaman ademas bluetooth.cpp y mando.cpp (A.A.A y B.B.B),
//       y main.cpp por sus propios caminos.
//   Censado symbol a symbol antes de borrar nada.
//
// menu_estaAbierto() Y LA REGLA DE LA BANDERA QUE SOLO SABE DAR UNA RESPUESTA
// ---------------------------------------------------------------------------
// CLAUDE.md 6.2 obliga a censar, antes de retirar el armador de una bandera, quien la
// LEE y que pasa si nunca vale true. Hecho:
//
//   LECTOR: uno solo, secuenciasInhibidas() en Esclavo/src/mando.cpp, que devuelve
//   exactamente menu_estaAbierto(). Con true, las secuencias del mando NO se
//   reconocen; con false, el mando esta armado.
//
//   QUE PASA SI NUNCA VALE true: el mando queda SIEMPRE armado. Eso NO es un veto que
//   se abre en este commit, y esta medido: el armador era `pantalla != P_MENU`, y
//   `pantalla` solo salia de P_MENU dentro de `if (aceptar)`, con aceptar =
//   botonAceptar() = false desde el 31/08. La bandera ya era falsa en todas las
//   vueltas del bucle desde entonces, y botones.cpp lo dice por escrito al lado de esa
//   definicion: "con ACEPTAR mudo, la pantalla del Esclavo no puede bajar del listado,
//   asi que menu_estaAbierto() es siempre falso". Lo que cambia hoy es que deja de ser
//   una propiedad de alcance y pasa a estar escrita en el fuente.
//
//   Y ADEMAS ES LA RESPUESTA CORRECTA, no un resto: el veto existia para el caso "hay
//   una persona delante del gabinete mirando una pantalla que puede CONFIRMAR algo".
//   Sin pantalla ese estado no existe nunca, asi que inhibir el mando seria inventarse
//   un operario que no esta. La contrapartida se dice entera: J16 p5/p8 siguen VACIOS
//   Y PELADOS, el mando sigue armado sobre ellos y un puente ahi sigue componiendo
//   secuencias. Eso es exactamente lo de ayer -D-32 lo deja escrito-, no una novedad
//   de este commit, y no lo arregla el firmware sino la instruccion de no cablearlos.
//
// CONSECUENCIA QUE HAY QUE SABER, y va aqui porque es donde se causo: botonArriba() y
// botonAbajo() se quedan SIN NINGUN CONSUMIDOR en esta punta -su unico llamador eran
// las lineas 183-184 de este fichero-. Los pines se siguen leyendo igual: quien los
// lee es botones_actualizar(), una sola vez por vuelta, y desde ahi alimenta al mando
// con mando_registrarPulso() ANTES de que nadie consuma el flanco. O sea que no se
// pierde ninguna lectura de J16 p5/p8; lo que baja de dos a uno es el numero de
// CONSUMIDORES de esa lectura. Queda anotado en costura_10 y reportado.
// ---------------------------------------------------------------------------

void menu_setup() {
  // No queda estado que reiniciar. Se conserva porque main.cpp la llama al terminar la
  // ventana de bienvenida y porque es donde volveria a montarse una interfaz si algun
  // dia la hay; retirarla obligaria a tocar main.cpp para dejarla huerfana igual.
}

void menu_loop() {
  // Vacia a proposito. Ver la cabecera: aqui vivia la navegacion de cinco pantallas.
}

// SFTY-21. El razonamiento completo, con la medida, esta en la cabecera de este
// fichero y en menu.h. No se cablea a false "y punto": se cablea a false porque el
// estado que la ponia a true -una pantalla abierta por debajo del listado- ya no puede
// existir, y porque desde el 31/08 tampoco podia.
bool menu_estaAbierto() {
  return false;
}
