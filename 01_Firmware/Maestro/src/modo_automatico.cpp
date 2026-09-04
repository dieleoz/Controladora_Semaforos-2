// ===== src/modo_automatico.cpp =====
#include "modo_automatico.h"
#include "botones.h"
#include "semaforo.h"
#include "coordinador.h"
#include "lcd.h"
#include "menu.h"
#include "modos.h"
#include <string.h>

enum FaseAuto { CONFIG_ROJO, CONFIG_VERDE, CONFIG_ESTATICO, CORRIENDO };
static FaseAuto fase;
static const uint8_t VERDE_MIN_MIN = 3,  VERDE_MIN_MAX = 15;
static const uint8_t ROJO_MIN_MIN  = 3,  ROJO_MIN_MAX  = 15;
static const uint8_t DESPEJE_SEG_MIN = 10, DESPEJE_SEG_MAX = 90;

// LOS VALORES DE ARRANQUE SALEN DE LOS LIMITES, NO SE ESCRIBEN A MANO (04/09).
//
// Aqui ponia "1, 1, 15" y eso convertia la guarda de 3 minutos en media guarda: solo
// la cruzaba SET_TIEMPOS. Un equipo que arranca y al que nadie le manda tiempos corria
// con UN MINUTO por sentido -justo lo que el responsable prohibio hoy- y ningun
// instrumento lo veia, porque el numero prohibido no estaba en la guarda: estaba en el
// inicializador, tres lineas mas arriba, con pinta de detalle.
//
// Es §3.bis: un minimo declarado en un sitio y repetido a mano en otro es una segunda
// copia que alguien tiene que sincronizar, y el dia que difieran gana la que NO tiene
// el comentario de seguridad encima.
static int minRojo = ROJO_MIN_MIN, minVerde = VERDE_MIN_MIN,
           segEstatico = DESPEJE_SEG_MIN;
static unsigned long tEstadoDesde = 0;
static bool primeraVezCorriendo = true;

// SFTY-21: peticion de arranque sin asistente, desde el mando de reles (A.A.A).
static bool arranqueDirecto = false;

void modoAutomatico_pedirArranqueDirecto() { arranqueDirecto = true; }

// --- N-69: limites DUROS de los tiempos del ciclo --------------------------------
//
// No son preferencias de interfaz: son la ultima linea. La app valida por comodidad,
// pero puede cambiarse, quedarse vieja o no ser la app -cualquiera puede mandar una
// trama por el puerto serie-. Quien decide si un valor entra es este fichero.
//
// El DESPEJE es el unico de los tres que es seguridad vial: es el tiempo que garantiza
// que el tramo quedo vacio antes de dar verde al otro lado. Su minimo no sale de un
// numero redondo: 10 s es lo que tarda en despejarse el tramo mas corto que esta casa
// ha montado, y por debajo de eso el margen desaparece.
// EL MINIMO SUBE DE 1 A 3 MINUTOS EL 04/09, Y ES UNA DECISION VIAL DEL RESPONSABLE.
//
// EL PORQUE, con sus palabras: tres minutos es la MINIMA DISTANCIA DE SEGURIDAD. En un
// paso alternado de un solo carril, un camion pesado tarda entre 5 y 8 s solo en
// reaccionar y arrancar; con un verde de 60 s pasan tres o cuatro vehiculos antes de
// cortar a ambar. Lo que se produce no es una cola: es un conductor convencido de que el
// semaforo esta averiado, adelantando en rojo contra el sentido que acaba de recibir
// verde. El limite de 1 minuto era un valor de MESA DE PRUEBAS que se quedo abierto para
// la operacion en via.
//
// VA AQUI Y NO EN LA APP, y esa es toda la diferencia. La app puede impedir que un dedo
// teclee 1, pero no es la unica que habla por J17: cualquier otra cosa en ese cable -o
// una APK vieja, que hoy mismo se demostro que sobreviven en los telefonos- puede
// mandar SET_TIEMPOS con un minuto. Una guarda que solo vive en la interfaz es de
// cortesia. Esta rechaza con $ERR,CMD:SET_TIEMPOS,DESC:RANGO y no la puede saltar nadie.
//
// COSTE DECLARADO: ya no se puede probar en mesa con ciclos de 1 minuto. Se acepto a
// sabiendas: un banco cae del lado de esperar tres minutos, no del lado de dejar el
// limite de laboratorio suelto en una carretera.

bool modoAutomatico_enMarcha() { return fase == CORRIENDO; }

bool modoAutomatico_fijarTiempos(uint8_t verdeMin, uint8_t rojoMin, uint8_t despejeSeg) {
  if (verdeMin  < VERDE_MIN_MIN   || verdeMin  > VERDE_MIN_MAX)   return false;
  if (rojoMin   < ROJO_MIN_MIN    || rojoMin   > ROJO_MIN_MAX)    return false;
  if (despejeSeg < DESPEJE_SEG_MIN || despejeSeg > DESPEJE_SEG_MAX) return false;

  // Y no se tocan los tiempos con el ciclo en marcha. La duracion se recalcula en cada
  // vuelta a partir de estas variables, asi que bajarlas a mitad de fase acortaria la
  // fase EN CURSO -incluido un todo-rojo ya empezado-. Los tiempos se cambian con el
  // modo parado, y entran enteros en el siguiente arranque.
  if (modoAutomatico_enMarcha()) return false;

  minVerde = verdeMin;
  minRojo = rojoMin;
  segEstatico = despejeSeg;
  return true;
}

void modoAutomatico_setup() {
  // Arranque desde el suelo: no se puede rellenar el asistente sin ver la pantalla,
  // asi que se salta y se corre con los ultimos valores. Se hace aqui, en el unico
  // punto de entrada del modo, para que el camino sea el mismo se llegue desde el
  // menu o desde el mando; duplicar el arranque en dos sitios es como se acaba con dos
  // configuraciones distintas segun por donde se entrase.
  if (arranqueDirecto) {
    arranqueDirecto = false;
    coordinador_configurar((unsigned long)segEstatico * 1000UL,
                           (unsigned long)minRojo * 60000UL,
                           (unsigned long)minVerde * 60000UL);
    coordinador_iniciarModo();   // empieza SIEMPRE por todo-rojo y su despeje
    tEstadoDesde = millis();
    primeraVezCorriendo = true;
    fase = CORRIENDO;
    lcd_dibujarAutomatico(coordinador_nombreEstadoMaster(), minRojo, minVerde);
    return;
  }

  fase = CONFIG_ROJO;
  // Mismo motivo que el inicializador: reentrar en el modo no puede devolver el
  // equipo por debajo del minimo vial. SET_MODO:AUTO llama aqui, asi que con el "1"
  // escrito a mano unos tiempos aceptados con $ACK se perdian al arrancar el modo.
  minRojo = ROJO_MIN_MIN; minVerde = VERDE_MIN_MIN; segEstatico = DESPEJE_SEG_MIN;
  primeraVezCorriendo = true;
  lcd_dibujarConfigValor("Minutos ROJO", minRojo, "min");
}

void modoAutomatico_loop() {
  if (botonCancelar()) {
    modoActual_set(MENU);
    menu_setup();
    return;
  }

  switch (fase) {
    case CONFIG_ROJO: {
      bool r = false;
      // LOS TOPES SALEN DE LOS MISMOS LIMITES QUE LA GUARDA DE SET_TIEMPOS (04/09).
      // Ponia 99 arriba y 1 abajo: por la pantalla se podia dejar el cruce en un
      // minuto -prohibido desde hoy- y en 99, que el propio SET_TIEMPOS rechaza. Dos
      // caminos hacia el mismo ciclo con dos reglas distintas es como se cuela un
      // valor vial por la puerta que nadie mira.
      if (botonArriba()) { minRojo++; if (minRojo > ROJO_MIN_MAX) minRojo = ROJO_MIN_MAX; r = true; }
      if (botonAbajo())  { minRojo--; if (minRojo < ROJO_MIN_MIN) minRojo = ROJO_MIN_MIN; r = true; }
      if (botonAceptar()) {
        fase = CONFIG_VERDE;
        lcd_dibujarConfigValor("Minutos VERDE", minVerde, "min");
        return;
      }
      if (r) lcd_dibujarConfigValor("Minutos ROJO", minRojo, "min");
      break;
    }

    case CONFIG_VERDE: {
      bool r = false;
      if (botonArriba()) { minVerde++; if (minVerde > VERDE_MIN_MAX) minVerde = VERDE_MIN_MAX; r = true; }
      if (botonAbajo())  { minVerde--; if (minVerde < VERDE_MIN_MIN) minVerde = VERDE_MIN_MIN; r = true; }
      if (botonAceptar()) {
        fase = CONFIG_ESTATICO;
        lcd_dibujarConfigValor("Tiem. Despeje All-Red", segEstatico, "seg");
        return;
      }
      if (r) lcd_dibujarConfigValor("Minutos VERDE", minVerde, "min");
      break;
    }

    case CONFIG_ESTATICO: {
      bool r = false;
      // OPT-6 / SFTY-4: Se aumenta el límite de 99 a 999 segundos (casi 16 min).
      // Ver MANUAL_USUARIO.md. Requerido para obras extensas de hasta 500 metros
      // a baja velocidad (10 km/h) operando bajo antenas de 6km.
      // El piso era 5 s y el minimo vial son 10: el despeje es el UNICO de los tres
      // que garantiza que el tramo quedo vacio antes de dar verde al otro lado, y
      // por la pantalla se podia dejar en la mitad. El techo de 999 ademas no cabe
      // en el uint8_t con que viaja por radio.
      if (botonArriba()) { segEstatico += 5; if (segEstatico > DESPEJE_SEG_MAX) segEstatico = DESPEJE_SEG_MAX; r = true; }
      if (botonAbajo())  { segEstatico -= 5; if (segEstatico < DESPEJE_SEG_MIN) segEstatico = DESPEJE_SEG_MIN; r = true; }
      if (botonAceptar()) {
        coordinador_configurar((unsigned long)segEstatico * 1000UL,
                                (unsigned long)minRojo * 60000UL,
                                (unsigned long)minVerde * 60000UL);
        coordinador_iniciarModo();
        tEstadoDesde = millis();
        primeraVezCorriendo = true;
        fase = CORRIENDO;
        lcd_dibujarAutomatico(coordinador_nombreEstadoMaster(), minRojo, minVerde);
        return;
      }
      if (r) lcd_dibujarConfigValor("Tiem. Despeje All-Red", segEstatico, "seg");
      break;
    }

    case CORRIENDO: {
      coordinador_actualizar();

      if (coordinador_listoParaContar()) {
        if (primeraVezCorriendo) {
          tEstadoDesde = millis();
          primeraVezCorriendo = false;
        }

        unsigned long duracion = (semaforo_estado() == S_ROJO)
                                    ? (unsigned long)minRojo * 60000UL
                                    : (unsigned long)minVerde * 60000UL;

        if (millis() - tEstadoDesde >= duracion) {
          coordinador_pedirCambio();
          tEstadoDesde = millis();
        }
      } else {
        primeraVezCorriendo = true;
      }

      static const char* estadoAnt = "";
      const char* actual = coordinador_nombreEstadoMaster();
      if (strcmp(actual, estadoAnt) != 0) {
        lcd_dibujarAutomatico(actual, minRojo, minVerde);
        estadoAnt = actual;
      }
      break;
    }
  }
}