// ===== src/botones.cpp (ESCLAVO) =====
#include "botones.h"
#include "pines.h"
#include "demanda.h"
#include "semaforo.h"
#include "bluetooth.h"
#include "instalacion.h"   // N-187: el contacto NA/NC de cada camara

// ---------------------------------------------------------------------------
// N-16: portado del Maestro. D-44: de J16 este fichero ya solo lee las dos camaras
// (PB14, PB15); p5 y p8 se declaran INPUT y nadie los lee (ver pines.h).
// ---------------------------------------------------------------------------

// D-44: J16 p5 y p8 (A y B) YA NO SE LEEN. Salieron la struct Boton, su antirrebote,
// los flancos, consumir() y la siembra de N-26 de los botones: el mando y el menu que
// los consumian ya no existen. Los dos pines solo se declaran INPUT en botones_setup().


// ---------------------------------------------------------------------------
// LAS CAMARAS DE J16 - N-97, 31/08/2026.
//
// DESDE AQUI HASTA EL FINAL DEL BLOQUE EL CODIGO ES IDENTICO EN LAS DOS PUNTAS, y hay un
// pack que lo exige (camara_02_j16). Es el motivo de que esto viva en botones.cpp y no en
// un modulo aparte: J16 TIENE UN SOLO DUENO. Repartir el conector entre dos ficheros es
// como PB14 acaba con dos pinMode() de modos distintos y gana el ultimo que corra, sin
// que nadie se entere. El vigilante vive en botones_vigilante.inc y es esta misma unidad:
// este fichero lo incluye en su sitio, y ningun pin se toca alli.
//
// QUE CIERRA ESTO DE N-97. La camara de demanda no era la misma entrada en las dos
// puntas:
//
//   Maestro   pinMode DENTRO de modoInteligente_setup(): fuera de ese modo el pin no
//             estaba ni configurado
//   Esclavo   pinMode en setup(), incondicional
//
// Ahora las dos configuran sus camaras en el arranque y sin condiciones. LA UNICA
// ASIMETRIA QUE QUEDA ESTA RAZONADA: el Maestro lee PB0 POR NIVEL y el Esclavo POR
// FLANCO. No es descuido, es SFTY-27 -el Esclavo PIDE y el Maestro DECIDE-. Una peticion
// es un suceso y viaja bien como flanco; pero la regla del Maestro es "cede el paso si el
// otro pide Y NO hay cola local", y "hay cola AHORA" es un nivel. Un flanco no puede
// contestar a eso.
//
// Las camaras C y D no heredan esa asimetria: las dos puntas las leen IGUAL, por flanco,
// y entran por demanda_solicitar(), que es la unica puerta que ya existe en las dos y el
// unico sitio donde la diferencia entre pedir y decidir esta escrita.

// La lectura de una entrada de camara. El NIVEL ELECTRICO es activo en alto -el porque, en
// pines.h-; con nc (N-187, D-162, SPEC_8 8.3) la presencia es el pin BAJO.
//
// El cuerpo viene del leerPinCamara() que estaba en modo_inteligente.cpp: no se reescribe logica
// ya probada solo para renombrarla, y N-187 solo le anadio el nc. Los 5 ms de antirrebote por
// software confirman el nivel ACTIVO, sea cual sea, y en J16 pesan mas que en J14: PB0 lleva el
// RC de R64+C25 que filtra 1 ms por hardware, y PB14/PB15 no llevan mas que el 10K de R67/R68.
// Aqui el unico antirrebote es este. CAM_DEMANDA_PIN se lee con nc = false, como siempre.
bool camara_leerPin(uint8_t pin, bool nc) {
  if ((digitalRead(pin) == HIGH) != nc) {
    delay(5);
    return (digitalRead(pin) == HIGH) != nc;
  }
  return false;
}

// Las dos camaras de J16 y su nivel estable anterior, para tomar la demanda por FLANCO.
// La tabla es una sola y la comparten la siembra del arranque y el lector del loop: dos
// listas de pines separadas son como una acaba con un pin que la otra no tiene.
static const uint8_t CAM_J16[2] = {CAM_C_PIN, CAM_D_PIN};
static bool camAnt[2] = {false, false};

// ---------------------------------------------------------------------------------
// EL VIGILANTE - D-13 FASE 1 (A-6), 05/09/2026. IDENTICO EN LAS DOS PUNTAS.
//
// Por que existe, y por que va ANTES que cualquier veto, esta escrito en botones.h.
// Aqui van los dos numeros y de que estan hechos.
//
// -------------------------------------------------------------------------------
// CAM_PEGADA_MS - CUANTO PUEDE DURAR UNA PRESENCIA LEGITIMA BAJO LA PLUMA
// -------------------------------------------------------------------------------
// La camara vigila EL BARRIDO DE LA PLUMA, no la zona de espera (D-13): un vehiculo
// que espera correctamente para ANTES y no entra en la region. Lo unico que la ocupa
// es alguien CRUZANDO, y solo se cruza con la pluma arriba, o sea durante un verde y
// su despeje -y, desde D-96, en cualquier luz con la barrera fuera de servicio
// (FLAG_BARRERA_FUERA, SPEC_8 6.3): ahi la pluma no baja nunca y quien manda es la luz-.
//
// El techo de eso son los MAXIMOS del ciclo, que viven en limites_ciclo.h:
//
//     VERDE_MIN_MAX (15 min) + DESPEJE_SEG_MAX (90 s) = 16,5 min
//
// Por encima de ese techo ninguna cola de vehiculos explica un contacto que lleva
// cerrado SIN ABRIRSE NI UNA VEZ. Se toman 20 min, el primer redondo por encima, y el
// margen no es decorativo: la pluma tambien se queda arriba en S_FALLO -ambar
// intermitente-, donde el paso es libre y la region puede ir muy cargada.
//
// LO QUE ESTA ALARMA NO SABE DISTINGUIR, ESCRITO EN VEZ DE DISIMULADO: un rele trabado
// y un vehiculo parado veinte minutos debajo de la pluma dan EL MISMO NIVEL, y este
// firmware no tiene con que separarlos. Las dos cosas necesitan que alguien vaya a
// mirar, asi que avisar es correcto en los dos casos; lo que NO seria correcto es que
// la trama dijera "averia". Por eso la CAUSA dice CONTACTO_FIJO y no AVERIA.
//
// Y LA DESIGUALDAD NO SE QUEDA EN ESTE COMENTARIO. Es N-71 letra por letra -un techo
// que vivia en prosa dejo dos reintentos inejecutables durante meses-: el pack
// camara_03_vigilante recalcula en cada corrida, leyendo los tres numeros del C++, que
//     CAM_PEGADA_MS > (VERDE_MIN_MAX * 60 + DESPEJE_SEG_MAX) * 1000
static const unsigned long CAM_PEGADA_MS = 1200000UL;

// -------------------------------------------------------------------------------
// CAM_CIEGA_MS - CUANTO PASO ABIERTO SIN UN SOLO FLANCO ES DEMASIADO
// -------------------------------------------------------------------------------
// ESTE NUMERO NO SALE DE NINGUNA CONSTANTE DEL FIRMWARE, Y SE DICE ASI EN VEZ DE
// VESTIRLO DE CUENTA. Cuanto tarda en pasar el siguiente vehiculo es una propiedad de
// LA CARRETERA, no del equipo; fabricarle una derivacion seria el "~1 s del rele" de
// A-7 otra vez: un numero nuestro que despues nos citamos como si fuera del fabricante.
//
// Lo que si es del equipo son las dos cosas que lo acotan, y las dos las comprueba el
// pack:
//
//   1. NO CUENTA TIEMPO DE RELOJ: cuenta tiempo CON LA PLUMA ARRIBA. Con el cruce
//      parado -menu, rojo total, esta punta sin turno- nadie puede cruzar el barrido, y
//      una camara callada esta diciendo la verdad. Ese es el "con el ciclo corriendo"
//      de D-13, medido sobre lo unico que de verdad abre el paso.
//   2. TIENE QUE SER MAYOR QUE CAM_PEGADA_MS. Una camara pegada tampoco da flancos, o
//      sea que su cronometro de silencio corre igual; si este numero fuera el menor, un
//      contacto trabado se anunciaria como CIEGA -el diagnostico CONTRARIO- y el
//      tecnico saldria a buscar un cable cortado teniendo un rele cerrado.
//
// SE TOMAN 24 h DE PASO ABIERTO (08/09). Antes eran 6 h, y la razon del cambio es un
// FALSO POSITIVO, no una holgura: con el ciclo minimo de D-5 -3 min por sentido- cada
// poste tiene el paso abierto aproximadamente la mitad del tiempo, asi que 6 h de paso
// abierto son del orden de 12 h de RELOJ. Eso aguanta una noche sin trafico -el unico
// silencio largo que es legitimo- pero NO aguanta un fin de semana con la obra activa y
// la carretera vacia, que en una via secundaria es un sabado normal. Una alarma que
// suena sola ensena a ignorarla, y entonces no protege de nada.
//
// 24 h de paso abierto son del orden de 2 DIAS de reloj: sobreviven ese fin de semana.
//
// POR QUE NO MAS, que es la mitad que decide: la unica referencia real que existe es
// "lleva 8 dias con presencia", que es como el responsable lo descubrio A OJO. Con 4
// dias de paso abierto -unos 8 dias de reloj- la alarma llegaria EXACTAMENTE cuando ya
// se habia visto sin ella, o sea que dejaria de servir. Con 24 h llega unas cuatro
// veces antes que el ojo, que es lo que la hace util.
//
// LO QUE ESTE NUMERO NO ES: no es el plazo de la desincronizacion del reloj, que se
// mide en MESES (D-21). Son dos cosas distintas y se anota aqui porque se confundieron
// al decidirlo: cuanto tarda en pasar un vehiculo es propiedad de la carretera; cuanto
// tarda una hora en volverse mentirosa es propiedad del oscilador.
//
// SE PODRA BAJAR CUANDO HAYA DATOS, y los datos los da esta misma fase 1: sus eventos
// son los que diran cuanto silencio hay de verdad en este cruce. Hoy no hay ninguno.
// D-24: CAM_CIEGA salta a las 24 h de paso abierto sin flanco.
static const unsigned long CAM_CIEGA_MS = 86400000UL;

// -------------------------------------------------------------------------------
// VETO_SOSTENIDO_MS - CUANDO UNA PLUMA RETENIDA DEJA DE SER SEGURA PARA EL TRAMO
// -------------------------------------------------------------------------------
// D-33: para el que esta DEBAJO, la barrera arriba siempre es lo seguro. Para el TRAMO
// no: en cuanto vence el todo-rojo, la otra punta puede abrir su verde, y un carril
// unico con la luz de un lado en verde y la barrera del otro levantada tiene las dos
// bocas abiertas. Ese es el instante en que hay que avisar, y por eso el plazo sale del
// DESPEJE y no del contacto.
//
// SE TOMA EL MAXIMO DEL RANGO CONFIGURABLE, y el borde va escrito: el despeje en curso
// vive en coordinador.cpp y aqui no se ve, asi que se compara contra el techo. Un veto
// mas largo que DESPEJE_SEG_MAX ha vencido al despeje en CUALQUIER configuracion; al
// reves no, y por eso no se toma el minimo: avisaria de un despeje que aun no ha vencido.
// ESTA PUNTA NO TIENE limites_ciclo.h: el techo del despeje lo fija el Maestro y aqui
// se declara HEREDADO, igual que DESPEJE_MIN_HEREDADO_SEG en semaforo.cpp. Quien impide
// que las dos copias se separen es camara_03_vigilante, que lee DESPEJE_SEG_MAX del
// limites_ciclo.h del Maestro y exige que sea este mismo numero (N-71).
static const unsigned long DESPEJE_MAX_HEREDADO_SEG = 90UL;
static const unsigned long VETO_SOSTENIDO_MS = DESPEJE_MAX_HEREDADO_SEG * 1000UL;

#include "botones_vigilante.inc"   // el vigilante y el contador de vetos, identico en las dos puntas

// N-26 APLICADO A LAS CAMARAS. Un contacto YA CERRADO al encender no es una deteccion:
// es un estado, exactamente igual que un boton ya pulsado. Sin sembrar camAnt[], la
// primera vuelta del loop encontraria nivel alto con anterior en false -que es la
// definicion de un flanco- y pediria paso sin que hubiera pasado ningun coche. Un rele de
// camara en reposo cerrado, un contacto trabado o el ruido en el cable hasta la bornera
// dejan el pin alto al encender sin que haya nada delante, y el firmware no los distingue.
//
// Se siembra el nivel REAL: si el contacto ya venia cerrado, no vuelve a pedir hasta que
// se ABRA y se cierre otra vez. Al encender no sabemos cuanto lleva asi, solo que nadie
// acaba de detectar nada. N-187/D-162: tambien tras cambiar el contacto (instalacion.cpp), y
// por eso reinicia TODO el estado por camara: sin ello un flanco vigente daria VE:1 fantasma.
void camaras_sembrar() {
  const unsigned long ahora = millis();
  camTickAnt = ahora;
  // N-26 tambien aqui: un veto ya activo al encender no es un veto NUEVO. Hoy no puede
  // estarlo -semaforo_setup() deja la pluma abajo y las dos banderas limpias-, y se
  // siembra igual para que la siembra no dependa de que eso siga siendo cierto.
  camVetoAnt = semaforo_plumaVetada();
  camVetoDesde = ahora;
  camVetoAvisos = 0;
  for (int i = 0; i < 2; i++) {
    camAnt[i] = camara_leerPin(CAM_J16[i], instalacion_camaraNC(i));
    // Y EL CRONOMETRO DE PEGADA SE SIEMBRA IGUAL, por el mismo motivo una capa mas
    // abajo: un contacto ya cerrado no va a dar el flanco que lo pondria en marcha, asi
    // que sin esta linea el rele trabado desde antes del encendido -el caso que mas
    // tarda en descubrirse solo- seria justo el unico que no se detecta nunca.
    camAltoDesde[i] = ahora;
    camHuboFlanco[i] = false;
    camUltimoFlanco[i] = 0;
    camSinFlancoMs[i] = 0;
    camEstado[i] = CAM_DESCONOCIDA;
  }
}

// El rele de la camara mantiene el contacto cerrado ~1 s por deteccion: leer el nivel
// repetiria la peticion en cada vuelta del loop durante todo ese segundo. Lo que pide
// paso es el flanco. La ventana de silencio de demanda_solicitar() -3 s, mas larga que el
// pulso del rele- es la que impide que una misma deteccion se cuente dos veces, y es la
// misma para la camara y para el boton de la app: dos origenes con temporizador propio no
// sabrian nada el uno del otro.
//
// NADA DE AQUI ESCRIBE UN PIN DE LUZ, y no es un detalle de estilo: solo semaforo.cpp los
// toca. Una camara PIDE; no ordena. El vigilante que cuelga de estas mismas lecturas
// tampoco: cuenta y avisa. El contacto NA/NC de cada una entra por indice (D-162).
static void camaras_actualizar() {
  const unsigned long ahora = millis();
  vigilante_tick(ahora);

  for (int i = 0; i < 2; i++) {
    const bool nivel = camara_leerPin(CAM_J16[i], instalacion_camaraNC(i));
    if (nivel && !camAnt[i]) {
      demanda_solicitar();
      vigilante_flanco(i, ahora);
    }
    vigilante_nivel(i, nivel, ahora);
    camAnt[i] = nivel;
  }
}

void botones_setup() {
  // D-44: J16 p5 y p8 NO SE LEEN. Se declaran INPUT de forma explicita para no depender
  // del estado de reset del core, que no esta medido; el reposo lo fija el pull-down de
  // 10K de la placa (R65/R66). Nadie llama a digitalRead() sobre ellos.
  pinMode(J16_P5_SIN_USO, INPUT);
  pinMode(J16_P8_SIN_USO, INPUT);

  // LAS ENTRADAS DE CAMARA: INPUT PELADO, NUNCA INPUT_PULLUP. El reposo lo fija el
  // pull-down de 10K que la placa ya trae -R64 en PB0; R67 y R68 en PB14 y PB15-, y el
  // contacto seco cierra contra los 3,3 V del propio conector. La cuenta que lo demuestra
  // esta en pines.h: con el pull-up interno el pin se queda en 0,66 V, que es LOW, o sea
  // demanda permanente sin camara y ninguna al cerrarla.
  //
  // SE DECLARAN AQUI, EN EL ARRANQUE, Y NO DENTRO DE UN MODO: eso era N-97. Un pin de
  // entrada declarado por el modo que lo usa solo existe mientras ese modo esta puesto,
  // mientras la otra punta lo declaraba siempre. En el Esclavo PB0 se declara en el
  // setup() de main.cpp, que es su camino de arranque; el resto es igual en las dos.
  pinMode(CAM_C_PIN, INPUT);
  pinMode(CAM_D_PIN, INPUT);

  // Las entradas se asientan antes de sembrar el nivel de las camaras (N-26). D-44:
  // aqui se sembraban ademas los dos botones; la espera se conserva tal cual.
  delay(2);

  camaras_sembrar();
}

void botones_actualizar() {
  // D-44: de J16 solo se leen las camaras; p5 y p8 ya no tienen lector.
  camaras_actualizar();
}

// D-44: aqui vivian botonArriba()/botonAbajo() y botonAceptar()/botonCancelar(),
// sin llamador desde que salieron el menu, el Modo Hora y las salidas por boton de
// cada modo. Los sustitutos son las ordenes de la app (D-16).

// LA PRESENCIA SOSTENIDA EN J16. El porque de que esto exista -y de que desde D-33 exista
// en las DOS puntas- esta en botones.h.
//
// NO ES UN LECTOR DE PINES: es camara_presencia() -la definicion unica que este fichero
// tiene de "hay coche"- preguntada por las dos camaras. Se escribio primero como
// `camAnt[0] || camAnt[1]`, o sea nivel puro, y EL ARNES LO TUMBO: con el contacto
// pulsando como pulsa el rele de la AcuSense -cerrado ~1 s por coche y abierto entre
// coche y coche- el nivel se cae en los huecos, el Modo Inteligente muestrea uno de esos
// huecos y la fase termina en el suelo igual que sin camara. El verde medido era 363000 ms
// contra los 720000 del techo. El termino que faltaba es el flanco vigente, y ya existia.
//
// NO RELEE LOS PINES a proposito: camaras_actualizar() ya los leyo en esta misma vuelta y
// dejo el resultado en camAnt[]. Releerlos daria un SEGUNDO valor de "hay coche" en la
// misma iteracion -quien decide el semaforo mirando un instante distinto del que mira el
// vigilante que juzga esa camara- y ademas arrastraria dos delay(5) mas por vuelta.
bool camara_presenciaJ16(int cam) {
  const unsigned long ahora = millis();
  for (int i = 0; i < 2; i++) {
    if ((cam < 0 || cam == i) && camara_presencia(i, ahora)) {
      return true;
    }
  }
  return false;
}
