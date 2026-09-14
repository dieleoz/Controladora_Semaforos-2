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
//   2. protocolo_reiniciarContadores() - SFTY-15 arranca de cero, para que la
//      medida sea la de ESTA prueba y no el acumulado desde el encendido.
//   3. coordinador_actualizar()        - mantiene el latido de 3 s, que es de donde
//      sale la telemetria que publica bluetooth.cpp.
// O sea: la prueba de alcance se sigue haciendo; lo que cambia es que se lee por la
// app y no en el gabinete.
//
// EFECTO LATERAL MEDIDO Y NO DISIMULADO: protocolo_bytesRecibidos() y
// protocolo_tramasValidas() se quedan SIN NINGUN LECTOR en el Maestro -aqui estaba el
// unico-. En el Esclavo si los publica su $ALARM/$EVENT; en esta punta nadie. Queda
// anotado en costura_10 y reportado: el sustituto seria publicarlos en el $STATUS.

void modoAlcance_setup() {
  // Mismo estado seguro que el Menu Principal: Rojo Fijo en ambos extremos con
  // enlace, Amarillo Intermitente si se pierde. No arranca ningun ciclo.
  coordinador_forzarMenu();
  // SFTY-15: los contadores arrancan de cero al entrar, para que lo que se vea en
  // pantalla corresponda a ESTA medicion y no al acumulado desde el encendido.
  protocolo_reiniciarContadores();
}

void modoAlcance_loop() {
  if (botonCancelar()) {
    modoActual_set(MENU);
    menu_setup();
    return;
  }

  // Mantiene vivo el latido de 3 s, que es de donde sale la telemetria.
  coordinador_actualizar();
}
