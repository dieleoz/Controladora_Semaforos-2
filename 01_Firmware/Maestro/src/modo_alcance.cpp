// ===== src/modo_alcance.cpp =====
//
// D-17.bis: ESTE MODO DIBUJA EN LA PANTALLA QUE SE RETIRA DEL EQUIPO. La decision
// retira la INTERFAZ y conserva el codigo a proposito, asi que el fichero se queda
// donde esta; lo que hay que saber al leerlo es que su unica salida visible era el
// gabinete, de modo que hoy entrar aqui deja el cruce parado en su estado seguro y no
// ensena nada a nadie. Quien replantee la interfaz decide si esta telemetria se publica
// por el aire o si la orden que trae hasta aqui se retira; no se decide desde el fuente.
#include "modo_alcance.h"
#include "botones.h"
#include "coordinador.h"
#include "menu.h"
#include "modos.h"
#include "protocolo.h"

// D-32 (1), 13/09: SE RETIRA refrescarSiCambio() ENTERA, y con ella los cinco
// static que guardaban lo ultimo pintado. Era el unico consumidor de la telemetria
// en este fichero: leia calidad, RTT, latidos perdidos y los dos contadores de
// SFTY-15 SOLO para decidir si repetir el volcado a la ST7920.
//
// LO QUE EL MODO SIGUE HACIENDO, que es lo que lo mantiene vivo con la app:
//   1. coordinador_forzarMenu()        - el mismo estado seguro que el menu.
//   2. coordinador_actualizar()        - mantiene el latido de 3 s, que es de donde
//      sale la telemetria que publica bluetooth.cpp.
// D-63 (N-178 M1): YA NO pone a cero los contadores de SFTY-15. Su lector era el LCD
// (D-44); la app mide por ventanas de 30 s (D-23), y el cero del 06/10 se leyo como un
// reinicio que no hubo. Desde aqui un contador que baja es un arranque (REIN, D-60).
// O sea: la prueba de alcance se sigue haciendo; lo que cambia es que se lee por la
// app y no en el gabinete.
//
// protocolo_bytesRecibidos() y protocolo_tramasValidas() los publica hoy el $EVENT
// ENLACE_RF de bluetooth.cpp en las dos puntas (D-23).

void modoAlcance_setup() {
  // Mismo estado seguro que el Menu Principal: Rojo Fijo en ambos extremos con
  // enlace, Amarillo Intermitente si se pierde. No arranca ningun ciclo.
  coordinador_forzarMenu();
}

void modoAlcance_loop() {
  // Mantiene vivo el latido de 3 s, que es de donde sale la telemetria.
  coordinador_actualizar();
}
