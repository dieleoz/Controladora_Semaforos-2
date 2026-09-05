// ===== src/modo_manual.cpp =====
#include "modo_manual.h"
#include "botones.h"
#include "semaforo.h"
#include "coordinador.h"
#include "lcd.h"
#include "menu.h"
#include "modos.h"
#include <string.h>

// N-141 (04/09/2026): MODO MANUAL TENIA LA MISMA TRAMPA QUE N-42, Y SEGUIA ABIERTA.
//
// Aqui habia `enum FaseManual { CONFIG_ESTATICO, CORRIENDO };` con un asistente de una
// sola pregunta -"Tiempo Rojo Estatico"- cuya UNICA salida era `if (botonAceptar())`. Y
// botonAceptar() devuelve `false` SIEMPRE desde deeeab4 (31/08), cuando BOTON3 y BOTON4
// pasaron a ser entradas de camara.
//
// O sea que el modo entraba en CONFIG_ESTATICO y no salia nunca. Reportado en banco el
// 04/09: "en dar paso, Maestro queda en rojo y esclavo queda en ambar titilando; eso no
// esta bien". El operario pulsaba DAR PASO y el equipo contestaba
// $ERR,CMD:CAMBIAR_TURNO,DESC:EN_TRANSICION_REINTENTE, porque el coordinador nunca
// llegaba a C_IDLE.
//
// EL CAMBIO DE SENTIDO NO HABIA QUE CONSTRUIRLO: ya existe entero en
// coordinador_pedirCambio() -verde a rojo directo, todo-rojo de despeje, y los 4 s de
// ambar SOLO al pasar de rojo a verde, que es justo lo que pidio el responsable-. Lo
// unico que faltaba era poder LLEGAR a el.
//
// SE RETIRA EL ENUM ENTERO, no se deja con un valor. Es §3.septies literal: en el
// Automatico, dejar `enum FaseAuto { CORRIENDO }` convirtio enMarcha() en una constante
// -el compilador la plegaba a `movs r0,#1`- y rompio SET_TIEMPOS durante horas. Censado
// antes de borrar: modo_manual.h no expone ningun getter, asi que de esta fase no cuelga
// ninguna bandera fuera de este fichero.
//
// -------------------------------------------------------------------------------------
// Y AQUI VIVIAN LA QUINTA, SEXTA Y SEPTIMA COPIA DEL PISO DE DESPEJE
// -------------------------------------------------------------------------------------
//
// `static int segEstatico = 3;` -TRES segundos-, `segEstatico = 5;` en el setup, y un
// `if (segEstatico < 5) segEstatico = 5;` con el rotulo "Piso minimo 5s despeje" encima.
// El minimo vial son DIEZ (DESPEJE_SEG_MIN, limites_ciclo.h): es el tiempo que garantiza
// que el tramo quedo VACIO antes de dar verde al otro lado.
//
// N-137 centralizo los limites ese mismo dia y NO VIO ESTE FICHERO, porque su codigo
// estaba muerto y no habia sintoma que buscar. 🔴 Un arreglo ingenuo que se limitara a
// quitar la fase HABRIA ACTIVADO un despeje de 5 s en un cruce en servicio: el defecto
// de interfaz y el defecto vial estaban en la misma linea.
//
// Se van con la fase. Este modo YA NO CONFIGURA TIEMPOS: conserva el despeje que dejo
// el Automatico -que si pasa por limites_ciclo.h- o los 15 s por defecto del propio
// coordinador. Los dos estan por encima del minimo. Manual es para dar paso a mano, no
// para reconfigurar el cruce; para eso esta SET_TIEMPOS, que tiene la guarda.

void modoManual_setup() {
  // Arranca corriendo. Una sola puerta, como el Automatico desde N-42: el coordinador
  // empieza por todo-rojo y su despeje, y cuando llega a C_IDLE el paso ya se puede dar.
  coordinador_iniciarModo();
  lcd_dibujarManual(semaforo_nombreEstado());
}

void modoManual_loop() {
  if (botonCancelar()) {
    modoActual_set(MENU);
    menu_setup();
    return;
  }

  // EL MANDO A/B NO DA PASO, Y ES UNA DECISION PENDIENTE, NO UN OLVIDO.
  //
  // Aqui habia `if (botonArriba() || botonAbajo()) coordinador_pedirCambio();`, muerto
  // desde el 31/08 por estar detras de la fase inalcanzable. Al retirar la fase reviviria
  // SOLO, y con el mando de reles conectado eso significa que UN PULSO SUELTO de A o de B
  // cambia el sentido del trafico.
  //
  // No se reactiva sin que el responsable lo decida: dar paso ABRE paso, y anadir una via
  // nueva de abrirlo -que ademas es un mando a distancia- no es una consecuencia
  // colateral que deba colarse dentro del arreglo de otra cosa. Hoy el paso se da desde
  // la app, que ademas pregunta si el tramo esta despejado.
  //
  // Las secuencias A.A.A y B.B.B del mando siguen funcionando: mando.cpp lee los mismos
  // flancos y este bloque no las consumia.

  // SFTY-12 tambien se queda fuera por lo mismo: colgaba de botonAceptar(), que no puede
  // ser cierto. El rojo fijo se pide hoy con FORZAR_ROJO desde la app -sin PIN, porque
  // parar es la direccion segura- y esa via SI esta ejercida.

  coordinador_actualizar();

  static const char* estadoAnt = "";
  const char* actual = coordinador_nombreEstadoMaster();
  if (strcmp(actual, estadoAnt) != 0) {
    lcd_dibujarManual(actual);
    estadoAnt = actual;
  }
}
