// ===== src/menu.cpp (ESCLAVO) =====
#include "menu.h"

// ---------------------------------------------------------------------------
// D-32 (1), 13/09/2026 — SALE EL LCD.
//
// D-30 (14/09): Y AQUI DECIA "DE ESTE FICHERO SOBREVIVE UNA SOLA COSA: menu_estaAbierto()".
// Tampoco sobrevivio. Era la puerta que inhibia las secuencias del mando y su unico
// lector era mando.cpp; retirado el mando se fue con el. De este fichero quedan hoy
// menu_setup() y menu_loop(), las dos VACIAS, que main.cpp sigue llamando.
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
// LO QUE FUE DE menu_estaAbierto()
// ---------------------------------------------------------------------------
// D-32 la dejo devolviendo `false` fijo, con el censo que CLAUDE.md 6.2 exige: su
// unico lector era secuenciasInhibidas() de mando.cpp, y con la bandera siempre falsa
// el mando quedaba SIEMPRE armado -que no era un veto abierto por aquel commit, sino
// el estado de hecho desde el 31/08, cuando botonAceptar() paso a ser `return false`-.
//
// D-30 (14/09) cierra eso del todo: retirado el mando, la funcion se quedo sin lector
// y se retiro. Ya no hay ningun reconocedor de secuencias al que inhibir ni armar.
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

// D-30 (14/09): AQUI ESTABA menu_estaAbierto(), Y SE VA CON EL MANDO.
//
// Era la puerta que INHIBIA las secuencias del mando con una pantalla abierta por
// debajo del listado, y su UNICO lector en todo el firmware era secuenciasInhibidas()
// de mando.cpp. Retirado el mando no la lee nadie: se quedaba huerfana, y una funcion
// que no llama nadie no es una barrera, es un adorno que aparenta una (CLAUDE.md 6.1).
//
// No abre nada al irse, y esto se censo antes de tocarla: devolvia `false` fijo desde
// que quedo sin armador, asi que el mando ya estaba SIEMPRE armado y lo que se retira
// es un veto que no vetaba. Hoy ni siquiera hay mando al que inhibir.
