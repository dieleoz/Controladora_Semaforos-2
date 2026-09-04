// ===== src/modo_automatico.cpp =====
#include "modo_automatico.h"
#include "respaldo.h"   // N-133: los tiempos del ciclo sobreviven al corte
#include "botones.h"
#include "semaforo.h"
#include "coordinador.h"
#include "lcd.h"
#include "menu.h"
#include "modos.h"
#include <string.h>

// N-42 (04/09): SE FUERON LAS TRES FASES DE CONFIGURACION. Queda una sola, y por eso
// el enum sobrevive: modoAutomatico_enMarcha() -de la que cuelga la guarda de
// SET_TIEMPOS- se lee mejor preguntando por la fase que por una bandera suelta.
enum FaseAuto { CORRIENDO };
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
// arranqueDirecto DESAPARECE como concepto: ahora TODAS las entradas al modo son
// directas, asi que no queda de que escapar. La funcion publica se conserva vacia
// porque su llamador vive en mando.cpp -la secuencia A.A.A del mando de reles- y
// borrarla convertiria este arreglo en un cambio de dos ficheros por comodidad.
// El compilador conserva el punto de uso y un grep lo sigue encontrando.

void modoAutomatico_pedirArranqueDirecto() { /* N-42: ya no hace falta, ver arriba */ }

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

// N-133: recupera los tiempos guardados, si los hay Y si siguen siendo legales.
//
// EL RANGO SE COMPRUEBA AQUI, no en respaldo.cpp, y no es reparto arbitrario: los
// limites viales viven en este fichero con el comentario que explica por que son 3
// minutos. Copiarlos al respaldo seria una segunda copia sin ese porque encima.
//
// Y SE COMPRUEBA AUNQUE EL CHECKSUM APRUEBE, que es el caso que de verdad importa: un
// equipo que se actualiza puede traer guardado un ciclo de 1 minuto, perfectamente
// integro, escrito cuando 1 era legal. Un dato integro no es un dato valido. Si algo
// no cuadra se descarta el conjunto -no se corrige el campo malo y se quedan los
// otros dos-: medio ciclo del respaldo y medio de los minimos es un ciclo que nadie
// configuro nunca.
static void recuperarTiemposGuardados() {
  uint8_t r = 0, v = 0, d = 0;
  if (!respaldo_tiemposCiclo(&r, &v, &d)) return;
  if (r < ROJO_MIN_MIN  || r > ROJO_MIN_MAX)  return;
  if (v < VERDE_MIN_MIN || v > VERDE_MIN_MAX) return;
  if (d < DESPEJE_SEG_MIN || d > DESPEJE_SEG_MAX) return;
  minRojo = r; minVerde = v; segEstatico = d;
}

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

  // N-133: Y SE GUARDAN, que es lo que faltaba. Hasta el 04/09 vivian solo en RAM: un
  // corte de luz -o entrar al modo- devolvia el cruce a los minimos SIN AVISAR, despues
  // de que el equipo hubiera contestado $ACK,CMD:SET_TIEMPOS,RESULT:OK. El tecnico se
  // iba del poste creyendo que dejo el ciclo puesto.
  //
  // Se escribe AQUI y no en cada vuelta del ciclo: los BKP son memoria de respaldo, no
  // un sitio donde repicar cada segundo. Solo cuando alguien los cambia de verdad.
  respaldo_guardarTiemposCiclo(rojoMin, verdeMin, despejeSeg);
  return true;
}

void modoAutomatico_setup() {
  // Arranque desde el suelo: no se puede rellenar el asistente sin ver la pantalla,
  // asi que se salta y se corre con los ultimos valores. Se hace aqui, en el unico
  // punto de entrada del modo, para que el camino sea el mismo se llegue desde el
  // menu o desde el mando; duplicar el arranque en dos sitios es como se acaba con dos
  // configuraciones distintas segun por donde se entrase.
  // N-42 (04/09): SE RETIRA EL ASISTENTE. UNA SOLA PUERTA, Y EL MODO ARRANCA CORRIENDO.
  //
  // EL DEFECTO QUE ESTO CIERRA, medido y confirmado en banco el 04/09: aqui se entraba
  // en fase CONFIG_ROJO -un cuestionario de tres preguntas de la epoca de la pantalla
  // LCD- cuya UNICA salida era botonAceptar(). Esa funcion devuelve `false` SIEMPRE
  // desde el commit deeeab4 del 31/08, cuando BOTON3 y BOTON4 pasaron a ser entradas de
  // camara. El equipo entraba en el cuestionario y no salia nunca.
  //
  // Y no era solo que las luces no se movieran. coordinador_actualizar() vivia DENTRO
  // del `case CORRIENDO`, y main.cpp EXCLUYE a MODO_AUTOMATICO del respaldo de fondo
  // -con el comentario "ya se llama en modo_automatico.cpp", cierto sobre el papel y
  // falso en ejecucion-. Asi que en Automatico el Maestro se quedaba MUDO en la radio:
  // ni un PING. El Esclavo, sin oir nada durante SFTY6_SILENCIO_MS, se iba a ambar por
  // orfandad haciendo lo correcto, y desde fuera parecia un fallo de comunicaciones.
  // El Maestro estaba VIVO pero no HABLANDO, que no es lo mismo.
  //
  // POR QUE SE RETIRA EN VEZ DE PARCHEARSE: el asistente no esta roto, esta HUERFANO.
  // No tiene pantalla donde mostrarse -retirada el 28/08- ni boton donde aceptarse, y
  // no los va a volver a tener. Dejarlo con un atajo por al lado seria codigo muerto
  // dentro del camino que decide las luces, y ademas DOS puertas de entrada al mismo
  // modo: exactamente como se acaba con dos configuraciones distintas segun por donde
  // se entrase. Por eso desaparece tambien arranqueDirecto: ya no hay de que escapar.
  //
  // LOS TIEMPOS YA NO SE PISAN (N-133). Aqui se reescribian a los minimos en cada
  // entrada, asi que unos tiempos aceptados con $ACK por SET_TIEMPOS se perdian en
  // cuanto alguien mandaba SET_MODO:AUTO. Se respetan los que haya; de donde salen al
  // arrancar el equipo lo decide el inicializador, y sobrevivir a un corte es cosa del
  // respaldo.
  // N-133: lo primero, antes de configurar nada. Si el equipo venia de un corte, los
  // tiempos estan en el respaldo y no en RAM.
  recuperarTiemposGuardados();

  coordinador_configurar((unsigned long)segEstatico * 1000UL,
                         (unsigned long)minRojo * 60000UL,
                         (unsigned long)minVerde * 60000UL);
  coordinador_iniciarModo();   // empieza SIEMPRE por todo-rojo y su despeje
  tEstadoDesde = millis();
  primeraVezCorriendo = true;
  fase = CORRIENDO;
  lcd_dibujarAutomatico(coordinador_nombreEstadoMaster(), minRojo, minVerde);
}

void modoAutomatico_loop() {
  if (botonCancelar()) {
    modoActual_set(MENU);
    menu_setup();
    return;
  }

  switch (fase) {
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