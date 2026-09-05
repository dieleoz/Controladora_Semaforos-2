// ===== src/modo_automatico.cpp =====
#include "modo_automatico.h"
#include "limites_ciclo.h"   // N-137: los limites, en un solo sitio
#include "modos.h"      // N-135: enMarcha() pregunta por el MODO, no por una fase
#include "respaldo.h"   // N-133: los tiempos del ciclo sobreviven al corte
#include "botones.h"
#include "semaforo.h"
#include "coordinador.h"
#include "lcd.h"
#include "menu.h"
#include "modos.h"
#include <string.h>

// N-135 (04/09, HORAS DESPUES DE N-42): AQUI HABIA UN ENUM DE UN SOLO VALOR, Y ESO NO
// ES UNA MAQUINA DE ESTADOS: ES UNA CONSTANTE DISFRAZADA.
//
// Al retirar las tres fases del asistente quedo `enum FaseAuto { CORRIENDO };` con su
// `static FaseAuto fase;`, y el comentario que habia aqui decia que el enum sobrevivia
// porque enMarcha() "se lee mejor preguntando por la fase". Se leia mejor y YA NO
// PREGUNTABA NADA. Medido con el compilador del proyecto, no razonado:
//
//     bool modoAutomatico_enMarcha() { return modoActual_get() == MODO_AUTOMATICO; }
//     arm-none-eabi-g++ -Os -S -mcpu=cortex-m3 -mthumb  ->   movs r0, #1
//                                                            bx   lr
//
// Con un solo enumerador la comparacion es cierta SIEMPRE, y desde antes de que corra
// modoAutomatico_setup(): en todos los modos, menu incluido. De enMarcha() cuelgan las
// dos guardas de SET_TIEMPOS, asi que el equipo contestaba
// $ERR,CMD:SET_TIEMPOS,DESC:EN_MARCHA_PARE_EL_MODO A TODO Y PARA SIEMPRE. Y como
// modoAutomatico_fijarTiempos() es el UNICO llamador de respaldo_guardarTiemposCiclo(),
// N-133 se quedo con camino de LECTURA y sin camino de ESCRITURA: los tiempos no se
// podian guardar nunca. Un arreglo cerro la puerta del otro y ningun instrumento lo vio.
//
// LA PREGUNTA CORRECTA ES OTRA, y es la que siempre quiso ser: el ciclo esta en marcha
// si el equipo ESTA EN ESTE MODO. Eso no puede degenerar -MODO_AUTOMATICO es uno de
// nueve- y ademas dice la verdad: desde N-42, entrar al modo ES ponerse a correr.
//
// Un estado que no puede tener dos valores no es un estado. Se retiran los dos.
// N-137: las seis constantes se mudan a include/limites_ciclo.h. Vivian `static` aqui,
// o sea invisibles para los demas modos, y eso produjo TRES agujeros distintos el
// mismo dia -el ultimo, un Modo Inteligente configurando 2 minutos-. El porque
// completo, y el de los 3 minutos, estan en la cabecera de ese fichero.

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
// N-143: lo que falta de la fase LARGA, en segundos, o SIN_CUENTA_ATRAS. Lo lee
// bluetooth.cpp para el campo T: del $STATUS cuando el coordinador no tiene cuenta
// propia. Vive aqui porque aqui esta el plazo, y en ningun otro sitio.
static int restanteFaseSeg = SIN_CUENTA_ATRAS;
static bool primeraVezCorriendo = true;

// A-12 (05/09): el respaldo se lee UNA VEZ, la primera que alguien pregunta por los
// tiempos, no solo al entrar a este modo. El porque, entero, sobre
// modoAutomatico_tiemposCiclo().
static bool respaldoLeido = false;

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

bool modoAutomatico_enMarcha() { return modoActual_get() == MODO_AUTOMATICO; }

// LA FRASE QUE SOSTIENE EL "--" DEL $STATUS VIVIA EN OTRO FICHERO Y ERA FALSA (05/09).
//
// bluetooth.cpp escribe, sobre la llamada a esta funcion: "el modo solo contesta si es
// el suyo: fuera del Automatico devuelve SIN_CUENTA_ATRAS". No lo hacia.
// restanteFaseSeg es un static y SALIR del modo no lo toca: se quedaba con el ultimo
// valor que tuvo, asi que en Inteligente o en Manual la app publicaba una cuenta atras
// CONGELADA de una fase que ya no existe. Y un numero quieto es peor que un "--",
// porque parece que sigue contando -lo dice el propio comentario de abajo-.
//
// Se contesta por el MODO, no por el residuo. Es §2.ter: una frase escrita al lado de
// un comportamiento, que nadie comprueba porque las frases no se compilan. Sale a la
// luz con A-12 porque el Inteligente pasa a ser un modo con tiempos configurados de
// verdad, y su cuenta atras -que no la tiene- se confundiria con la del Automatico.
int modoAutomatico_segundosRestantesFase() {
  if (!modoAutomatico_enMarcha()) return SIN_CUENTA_ATRAS;
  return restanteFaseSeg;
}

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
  respaldoLeido = true;
  uint8_t r = 0, v = 0, d = 0;
  if (!respaldo_tiemposCiclo(&r, &v, &d)) return;
  if (r < ROJO_MIN_MIN  || r > ROJO_MIN_MAX)  return;
  if (v < VERDE_MIN_MIN || v > VERDE_MIN_MAX) return;
  if (d < DESPEJE_SEG_MIN || d > DESPEJE_SEG_MAX) return;
  minRojo = r; minVerde = v; segEstatico = d;
}

// A-12 (05/09): LA VENTANA A LOS TIEMPOS CONFIGURADOS, PORQUE EL MODO INTELIGENTE NO
// LOS PODIA VER Y SE FABRICABA LOS SUYOS.
//
// minVerde/minRojo/segEstatico vivian `static` aqui -invisibles para el resto del
// firmware- y modo_inteligente.cpp arrancaba el coordinador con VERDE_MIN_MIN, o sea
// 3 minutos SIEMPRE. Un operario que configura 6 porque ese tramo es largo veia el
// cruce correr a 3 en cuanto entraba en Inteligente, y ninguna guarda lo delataba
// porque ese modo no pasa por SET_TIEMPOS.
//
// SE ABRE UNA VENTANA, NO SE COPIAN LOS NUMEROS. Pasarle los valores al otro fichero
// para que los guarde por su cuenta seria la sexta copia de N-137, y el dia que
// difieran gana la que no lleva encima el comentario de seguridad.
//
// Y SE RECUPERA EL RESPALDO ANTES DE CONTESTAR, que es la mitad que faltaba: hasta hoy
// recuperarTiemposGuardados() solo corria dentro de modoAutomatico_setup(). Un equipo
// que vuelve de un corte y entra DIRECTO a Inteligente -SET_MODO:INTELIGENTE desde la
// app, o el menu- habria leido los minimos de fabrica teniendo los tiempos buenos
// guardados en los BKP a dos centimetros. Se lee una vez y no en cada vuelta: los BKP
// son memoria de respaldo, no un sitio donde repicar.
//
// SIN COMPROBAR LOS PUNTEROS A PROPOSITO: una guarda que ningun llamador puede hacer
// falsa no es una guarda, es la constante disfrazada de §3.septies. Los tres punteros
// son obligatorios y asi lo dice la cabecera.
void modoAutomatico_tiemposCiclo(uint8_t* verdeMin, uint8_t* rojoMin,
                                 uint8_t* despejeSeg) {
  if (!respaldoLeido) recuperarTiemposGuardados();
  *verdeMin   = (uint8_t)minVerde;
  *rojoMin    = (uint8_t)minRojo;
  *despejeSeg = (uint8_t)segEstatico;
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
  lcd_dibujarAutomatico(coordinador_nombreEstadoMaster(), minRojo, minVerde);
}

void modoAutomatico_loop() {
  if (botonCancelar()) {
    modoActual_set(MENU);
    menu_setup();
    return;
  }

  // N-135: LA LLAMADA AL COORDINADOR ES INCONDICIONAL, Y ESO NO ES ESTILO.
  // main.cpp EXCLUYE a este modo del refresco de fondo, asi que si esta llamada
  // quedara dentro de una rama que pudiera no alcanzarse, el Maestro se quedaria
  // mudo en la radio -que es exactamente N-42-. Aqui vivia dentro de un `switch`
  // de un solo caso; al retirarlo quedo un bloque suelto que segun se mire parece
  // condicional. Se deja plana: lo que no tiene rama no se puede dejar sin visitar.
  coordinador_actualizar();

  if (coordinador_listoParaContar()) {
    if (primeraVezCorriendo) {
      tEstadoDesde = millis();
      primeraVezCorriendo = false;
    }

    unsigned long duracion = (semaforo_estado() == S_ROJO)
                                ? (unsigned long)minRojo * 60000UL
                                : (unsigned long)minVerde * 60000UL;

    // N-143 (04/09): SE PUBLICA LO QUE FALTA. Es la fase que el operario MIRA.
    //
    // El responsable, dos veces: "debe ser un contador decreciente, de saber cuanto
    // tiempo falta para el cambio; hoy es un contador creciente que no aporta".
    //
    // N-139 arreglo el campo T: para que fueran los segundos que faltan, pero la cuenta
    // la hace el coordinador y el coordinador NO SABE cuanto dura esta fase:
    // coordinador_configurar() recibe los dos tiempos de ciclo SIN NOMBRE y los tira
    // -mirese su firma-. Solo conoce el despeje. Asi que la unica fase con cuenta atras
    // era el todo-rojo de 10-90 s, y la LARGA -3 a 15 min, o sea casi todo el tiempo que
    // el cruce esta funcionando- seguia diciendo "--". Justo la que se mira.
    //
    // POR QUE SE PUBLICA DESDE AQUI Y NO SE RECONSTRUYE EN EL COORDINADOR: alli seria
    // una segunda copia del ciclo escrita a mano -R-9- y ademas MENTIRIA en dos de los
    // tres modos que lo usan: en Manual la fase acaba cuando alguien pulsa, y en
    // Inteligente el tiempo configurado es un MAXIMO, no una duracion. Quien sabe cuanto
    // falta es quien pone el plazo, y es este fichero.
    //
    // PISO, no redondeo, igual que la del coordinador: vale N al entrar y 0 durante el
    // ultimo segundo. El operario nunca ve un numero MAYOR que lo que de verdad queda.
    const unsigned long va = millis() - tEstadoDesde;
    restanteFaseSeg = (va >= duracion) ? 0 : (int)((duracion - va) / 1000UL);

    if (millis() - tEstadoDesde >= duracion) {
      coordinador_pedirCambio();
      tEstadoDesde = millis();
    }
  } else {
    primeraVezCorriendo = true;
    // Fuera de C_IDLE manda el coordinador -hay un despeje o una transicion en curso- y
    // el que tiene la cuenta buena es el. Se marca para no publicar una cifra vieja: un
    // numero congelado es peor que un "--", porque parece que sigue contando.
    restanteFaseSeg = SIN_CUENTA_ATRAS;
  }

  static const char* estadoAnt = "";
  const char* actual = coordinador_nombreEstadoMaster();
  if (strcmp(actual, estadoAnt) != 0) {
    lcd_dibujarAutomatico(actual, minRojo, minVerde);
    estadoAnt = actual;
  }
}