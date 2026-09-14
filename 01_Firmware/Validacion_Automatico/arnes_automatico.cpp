// ===== 01_Firmware/Validacion_Automatico/arnes_automatico.cpp =====
//
// EL ARNES QUE FALTABA: coordinador.cpp + semaforo.cpp + modo_automatico.cpp
// REALES, compilados y ejecutados en el PC.
//
// POR QUE ESTE ARNES Y NO OTRO.
//
// La regresion del Modo Automatico -no arranca el ciclo- paso con TODAS las
// comprobaciones en verde porque NINGUN instrumento ejercia el ciclo automatico
// sobre el C++ real. validador_maestro.py y simulador_sistema_v7_6.py son Python
// escrito a mano que REIMPLEMENTA lo que hace coordinador.cpp: su PASS prueba el
// modelo, no el codigo. Si coordinador.cpp se rompe y el modelo no, el banco sigue
// diciendo PASS mientras el semaforo real se queda sin arrancar.
//
// El molde es Validacion_Ciclo: incluir el .h/.cpp REAL del firmware y barrer sobre
// EL, sin espejo. Aqui es un nivel mas arriba -no una funcion pura como
// ciclo_degradado_fase(), sino tres modulos con estado y con E/S simulada- y por eso
// hace falta un driver mas grande: un Esclavo simulado que contesta al protocolo, un
// reloj simulado que el arnes mueve, y botones simulados que pulsa el arnes. Lo que
// NO se simula es lo que se esta midiendo: coordinador.cpp, semaforo.cpp y
// modo_automatico.cpp se compilan tal cual van a la tarjeta.
//
// QUE NO CUBRE. Este arnes compila solo el lado MAESTRO. No hay Esclavo real aqui
// -su firmware no se compila-, asi que "verde simultaneo en las dos puntas" sigue
// sin poder medirse en este camino (esa propiedad es la de Validacion_Ciclo, sobre
// ciclo_degradado.h, que es pura y corre en las dos puntas). Lo que este arnes SI
// puede medir, y hasta hoy nadie media sobre el C++ real, es que el propio Maestro
// LLEGA a dar verde, que lo suelta solo transcurrido el tiempo configurado, que
// nunca salta el amarillo de aviso, y que ante una respuesta ausente o incorrecta
// del otro lado cae al estado seguro en vez de quedarse esperando o de aceptar
// cualquier cosa como buena.
//
// A-12 (05/09): SE SUMAN modo_inteligente.cpp Y demanda.cpp REALES (Bloque E). El
// Modo Inteligente no leia ni uno de los tiempos que configura el operario y su
// Regla 1 podia cortar un verde a los 15 s. La propiedad que hace seguro el arreglo
// es de COMPORTAMIENTO -"con las camaras muertas se comporta EXACTAMENTE como el
// Automatico"- y no se puede leer en el fuente: hay que correr LOS DOS modos con la
// MISMA configuracion y comparar las dos duraciones con la misma regla. Eso es lo que
// hace el Bloque E, y por eso vive aqui y no en un pack.
//
// LO QUE EL BLOQUE E NO CUBRE, escrito para que no se lea como permiso: la camara es
// un bool que mueve el arnes. El antirrebote de 1 ms de camara_leerPin() vive en
// botones.cpp, que aqui NO se compila, y el cableado de J16 es cobre -M3-. Lo que se
// mide es que hace una deteccion con el ciclo en marcha, no como se detecta.
//
// D-30 (14/09): AQUI VIVIA EL BLOQUE D -EL MANDO DE RELES (SFTY-21)- Y SE FUE ENTERO.
//
// Compilaba mando.cpp REAL y pulsaba las tres secuencias (A.A.A, B.B.B, A.B.A.B) para
// ejercer senalActiva, el static de semaforo.cpp que congelaba escribirPines() mientras
// duraban los destellos. Retiradas las botoneras, mando.cpp no existe y la interceptacion
// salio entera de semaforo.cpp: no queda sujeto que medir.
//
// LO QUE SE REPARTIO EN VEZ DE BORRARSE (CLAUDE.md §9). El fuzz del Bloque D no solo
// media el mando: era el barrido mas largo del arnes, y dentro llevaba cuatro
// comprobaciones cuyo sujeto es D-33/N-153 -la pluma- y no el mando. Esas cuatro se
// MUDARON con su bloque literal al resumen de invariantes del final de main(), que es
// donde viven sus hermanas y donde siguen midiendo sobre TODO el barrido. Lo que se
// perdio con el fuzz esta escrito alli mismo, sin disimular.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cinttypes>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <regex>

#include "Arduino.h"
#include "pines.h"
#include "botones.h"
#include "lcd.h"
#include "menu.h"

#include "coordinador.h"
#include "semaforo.h"
#include "modo_automatico.h"
#include "protocolo.h"
#include "reloj.h"
#include "respaldo.h"
// D-30 (14/09): aqui se incluian mando.h, modo_ambar.h y modo_degradado.h. La primera
// ya no existe; las otras dos solo hacian falta por los stubs que enlazaban mando.cpp.
#include "modo_inteligente.h" // A-12: real, y su .cpp SI se compila (Bloque E)
#include "demanda.h"          // A-12: real, y su .cpp tambien -- entra en el OR

// ---------------------------------------------------------------------------
// EL RELOJ SIMULADO Y LOS PINES OBSERVADOS. Los declara extern Arduino.h; existen
// aqui porque el arnes es quien los mueve.
// ---------------------------------------------------------------------------
unsigned long arnes_millis_valor = 0;
int arnes_pines[64];
unsigned long arnes_escrituras = 0;
// Cuanto tiempo de delay() pidio el firmware. No mueve el reloj (ver Arduino.h);
// se acumula para poder DECIR cuanto no se esta modelando, en vez de callarlo.
unsigned long arnes_delays = 0;

// ---------------------------------------------------------------------------
// LECTURA DE CONSTANTES DEL C++ REAL. Mismo contrato que
// Simulaciones/banco/fuente.py::constante(): sin el patron no hay numero, y sin
// numero el arnes ABORTA -nunca cae a un valor escrito a mano que "casualmente
// coincide" con lo que el firmware tenia la ultima vez que alguien miro-.
// ---------------------------------------------------------------------------

static std::string dirDeEsteArchivo() {
  std::string f = __FILE__;
  size_t p = f.find_last_of("/\\");
  return (p == std::string::npos) ? std::string(".") : f.substr(0, p);
}

// Maestro/src, calculada a partir de donde vive ESTE fichero y no del directorio
// de trabajo: el arnes puede invocarse desde cualquier sitio (compilar.ps1, la
// compuerta, o a mano) y la ruta tiene que seguir siendo la misma.
static const std::string MAESTRO_SRC = dirDeEsteArchivo() + "/../Maestro/src/";
// N-137 (04/09): los seis limites del ciclo se mudaron de modo_automatico.cpp a
// include/limites_ciclo.h -vivian `static` y por eso otros modos no los veian, lo que
// produjo tres agujeros el mismo dia-. Este arnes ABORTO en la corrida siguiente, que
// es §5 funcionando: lee el fuente POR RUTA y la ruta cambio. Se resuelve por nombre
// de fichero, no anadiendo un segundo directorio a cada llamada.
static const std::string MAESTRO_INC = dirDeEsteArchivo() + "/../Maestro/include/";
static std::string rutaDe(const std::string& archivo) {
  return (archivo.size() > 2 && archivo.substr(archivo.size() - 2) == ".h")
           ? MAESTRO_INC + archivo : MAESTRO_SRC + archivo;
}

static void abortar(const std::string& motivo) {
  std::fprintf(stdout, "\n[ABORTADO] %s\n", motivo.c_str());
  std::fprintf(stdout,
      "Sin esa constante el arnes mediria otra cosa que el firmware, y seguiria\n"
      "dando un veredicto aunque ya no describa el C++ real. Regla del banco:\n"
      "sin valor por defecto, nunca.\n");
  std::exit(2);
}

static std::string leerArchivoFuente(const std::string& nombre) {
  std::string ruta = rutaDe(nombre);   // N-137: .h en include/, .cpp en src/
  std::ifstream f(ruta.c_str());
  if (!f) abortar("no se pudo abrir el fuente real " + ruta);
  std::ostringstream ss;
  ss << f.rdbuf();
  return ss.str();
}

// Todas las apariciones del patron (con exactamente un grupo de captura numerico).
// ABORTA si no hay ninguna.
static std::vector<long> leerConstantesTodas(const std::string& archivo,
                                              const std::string& patron,
                                              const std::string& que) {
  std::string txt = leerArchivoFuente(archivo);
  std::regex re(patron);
  std::vector<long> valores;
  for (std::sregex_iterator it(txt.begin(), txt.end(), re), fin; it != fin; ++it) {
    valores.push_back(std::strtol((*it)[1].str().c_str(), nullptr, 10));
  }
  if (valores.empty()) {
    abortar("no se pudo leer del C++ real la constante de " + que +
            " (patron no encontrado en " + archivo + ")");
  }
  return valores;
}

// La primera aparicion, para las constantes que solo se escriben una vez.
static long leerConstante(const std::string& archivo, const std::string& patron,
                           const std::string& que) {
  return leerConstantesTodas(archivo, patron, que)[0];
}

// ---------------------------------------------------------------------------
// EL CONTADOR DE COMPROBACIONES. Mismo patron que Validacion_Ciclo/arnes_ciclo.cpp.
// ---------------------------------------------------------------------------
static int total = 0, fallos = 0;

static void comprobar(bool ok, const char* que) {
  total++;
  if (ok) {
    std::printf("   [OK]    %s\n", que);
  } else {
    fallos++;
    std::printf("   [FALLA] %s\n", que);
  }
}

// ---------------------------------------------------------------------------
// SFTY-2, VIGILADO DE BALDE. Con digitalWrite() de Arduino.h grabando en
// arnes_pines[], cada barrido de este arnes puede comprobar -sobre lo que
// escribirPines() REALMENTE escribio, no sobre lo que la logica dijo que queria-
// que Rojo y Verde nunca coincidieron encendidos en la misma cara. Es la barrera de
// salidas (regla 6 de CLAUDE.md) medida, no supuesta.
// ---------------------------------------------------------------------------
static long violacionesEnclavamiento = 0;

// SFTY-28. La pluma sale por la misma puerta que las luces, asi que se vigila igual y
// en el mismo tick: CON LA PLUMA ARRIBA TIENE QUE HABER VERDE ENCENDIDO -o el equipo
// tiene que estar en S_FALLO-. Al reves no se exige -verde con la pluma abajo es
// degradado, feo pero no peligroso-; lo que mata es lo contrario: una barrera
// levantada invitando a pasar con la luz en rojo, porque el conductor le hace mas
// caso a la barrera que a la lampara.
//
// LA EXCEPCION DE S_FALLO ESTA AQUI POR NOMBRE, Y ES DELIBERADO. S_FALLO es el ambar
// intermitente de SFTY-6: sin enlace, el equipo ya no sabe quien tiene el paso, y la
// politica elegida por el cliente el 27/08 es dejar pasar con precaucion en vez de
// cerrar la via. Escribirla como excepcion explicita -y no relajando el invariante a
// "solo cuando hay rojo"- hace que el dia que alguien abra la pluma en CUALQUIER otro
// estado, esto siga cazandolo.
static long violacionesTalanquera = 0;

// ---------------------------------------------------------------------------
// D-33 (14/09/2026) - EL REPARTO DE LA INVARIANTE DE ARRIBA (CLAUDE.md 9).
//
// LA LINEA DE ARRIBA AFIRMABA TRES COSAS, Y SOLO UNA HA CAMBIADO. Se cuentan antes de
// tocarla porque casi ninguna invariante afirma una sola (N-83):
//
//   1. "la pluma arriba SIEMPRE tiene una razon nombrada, nunca sube sola"
//      -> SE CONSERVA ENTERA. Lo que crece es la lista de razones, de dos a cuatro.
//   2. "esa razon es el verde encendido"
//      -> SE REPARTE. Sigue siendo cierta fuera de la ventana de bajada -y ahi se
//         mide igual que siempre-; dentro, las razones son el retardo de D-33 y el
//         veto de la camara, y cada una se mide con su propia cota.
//   3. "la unica excepcion nombrada es S_FALLO"
//      -> SE CONSERVA LITERAL, con su nombre y su motivo.
//
// LAS DOS RAZONES NUEVAS NO SE CONCEDEN POR NOMBRE: SE MIDEN. Una excepcion escrita y
// no comprobada es una lista de defectos con permiso (CLAUDE.md 6):
//
//   EL RETARDO se CRONOMETRA contra PLUMA_RETARDO_BAJADA_MS leido del C++ real. No se
//   pregunta al firmware si "esta en el retardo" -eso seria creerle-: se mide cuanto
//   lleva la pluma arriba con la luz ya en rojo y se compara con su propia constante.
//
//   EL VETO se cruza con la OTRA funcion real: si semaforo_plumaVetada() dice que si,
//   camara_presenciaJ16() tiene que decir que si TAMBIEN. Un veto que se quedara
//   pegado -true sin nadie debajo- dejaria la pluma arriba para siempre y pasaria
//   cualquier comprobacion que se limitara a aceptar la excusa.
static long vetoSinPresencia = 0;
static unsigned long g_plumaSinVerdeDesde = 0;   // 0 = no hay ventana abierta
static unsigned long g_peorVentanaPlumaMs = 0;     // la mas larga, sea cual sea la razon
static unsigned long g_peorVentanaSinVetoMs = 0;  // la mas larga con el veto SUELTO
static unsigned long g_ventanasPluma = 0;
static unsigned long g_retardoPlumaMs = 0;       // leido del C++ al principio de main()

// EL MARGEN DEL RETARDO, RE-DERIVADO CON EL MANDO FUERA (D-30, 14/09).
//
// EL BORDE CAMBIO DE DUENO, Y POR ESO SE VUELVE A DERIVAR EN VEZ DE HEREDARSE
// (CLAUDE.md §7 y §14: una cifra que nadie recalcula envejece). Antes el margen era
// DESTELLO_ON_MS y el motivo era el mando: con una senal en curso las escrituras estaban
// INTERCEPTADAS -aplicarSalidas() guardaba sin escribir- y la unica puerta que volvia a
// pasar por escribirPines() era actualizarSenal(), cada DESTELLO_ON_MS. Esa interceptacion
// ya no existe, asi que ese borde no solo esta caducado: ha perdido el sujeto que lo
// justificaba, y copiarlo tal cual seria dejar una tolerancia sin motivo.
//
// EL BORDE DE HOY, Y POR QUE ES EL CORRECTO. El retardo se suelta en la siguiente
// entrada por escribirPines(), y hoy hay UNA sola y ocurre en cada vuelta:
// semaforo_actualizar() termina con "if (plumaCierrePendiente) aplicarSalidas(ultR, ultA,
// ultV)". O sea que el firmware no anade retraso ninguno -mira otra vez en la vuelta
// siguiente-, y lo unico que puede hacer que la ventana OBSERVADA pase del retardo es la
// granularidad con que ESTE ARNES muestrea: entre dos llamadas a vigilarEnclavamiento()
// pasa un paso de bombeo. El margen es, por tanto, UNA PROPIEDAD DEL ARNES y no del
// firmware, y por eso NO se lee del C++: leerlo de alli fingiria que el firmware la
// gobierna.
//
// Su valor es el mayor paso de bombeo con el que puede estar abierta una ventana SIN
// VETO. Los bloques que mueven la pluma sin camara son A-C (paso 200 ms) y G (paso
// PASO_G = 100 ms); el Bloque F corre a 60 s por vuelta, pero sus ventanas las sostiene
// el veto de una camara y por eso no entran en esta cota -son las que mide
// g_peorVentanaPlumaMs, que no tiene tope-. Medido tras el cambio: la peor ventana SIN
// VETO de todo el barrido se imprime en el RESUMEN del final, y si algun dia rebasa este
// margen la linea FALLA en vez de ensancharse.
static const unsigned long MARGEN_MUESTREO_MS = 200;
static unsigned long g_margenSenalMs = MARGEN_MUESTREO_MS;

// N-153. LO QUE EL EQUIPO PUBLICA DE LA PLUMA TIENE QUE SER LO QUE HAY EN EL PIN.
//
// Desde N-153 el $STATUS lleva un campo PLUMA que sale de semaforo_plumaArriba(), y la
// app dibuja la barrera con el. Un getter que se desincronice del pin no rompe ninguna
// luz -el cruce sigue funcionando igual- y por eso ningun pack de texto puede verlo:
// barrera_03 comprueba la FORMA de la orden, no su resultado. Lo unico que puede medir
// esto es un arnes que compile semaforo.cpp de verdad, escriba el pin y pregunte al
// getter en el MISMO instante, que es lo que se hace aqui en cada tick.
//
// Y lo que se compara no es la formula: es el pin. Recalcular la condicion aqui seria
// una tercera copia de SFTY-28 -y las copias es justo lo que este campo evita-.
static long discrepanciasPluma = 0;

static void vigilarEnclavamiento() {
  if ((arnes_pines[ROJO1] == HIGH && arnes_pines[VERDE1] == HIGH) ||
      (arnes_pines[ROJO2] == HIGH && arnes_pines[VERDE2] == HIGH)) {
    violacionesEnclavamiento++;
  }
  if (arnes_pines[MOTOR_TALANQUERA] == TALANQUERA_ABRIR &&
      arnes_pines[VERDE1] != HIGH && arnes_pines[VERDE2] != HIGH &&
      semaforo_estado() != S_FALLO) {
    // D-33: la ventana se abre en el primer tick sin luz y se cierra sola. Se fecha con
    // el reloj del arnes, no con una bandera del firmware.
    if (g_plumaSinVerdeDesde == 0) {
      g_plumaSinVerdeDesde = arnes_millis_valor + 1;
      g_ventanasPluma++;
    }
    const unsigned long dur = arnes_millis_valor - (g_plumaSinVerdeDesde - 1);
    if (dur > g_peorVentanaPlumaMs) g_peorVentanaPlumaMs = dur;
    const bool dentroDelRetardo = (dur <= g_retardoPlumaMs + g_margenSenalMs);
    const bool vetoVivo = semaforo_plumaVetada() && camara_presenciaJ16();
    // DOS MEDIDAS, NO UNA. La ventana TOTAL puede durar horas y estar bien -es una camara
    // vetando-; la que tiene cota es la que corre SIN veto, porque ahi lo unico que puede
    // estar reteniendo la pluma es el retardo. Meterlas en el mismo numero haria que el
    // maximo lo fijara siempre el veto y la cota del retardo dejaria de medirse.
    if (!vetoVivo && dur > g_peorVentanaSinVetoMs) g_peorVentanaSinVetoMs = dur;
    if (!dentroDelRetardo && !vetoVivo) {
      violacionesTalanquera++;
    }
  } else {
    g_plumaSinVerdeDesde = 0;
  }
  // D-33: y el veto no puede estar puesto sin nadie debajo, en NINGUN instante. Es el
  // unico modo de fallo del veto que deja la barrera arriba para siempre sin que ninguna
  // luz cambie, o sea el unico que ninguna otra comprobacion de aqui podria ver.
  if (semaforo_plumaVetada() && !camara_presenciaJ16()) {
    vetoSinPresencia++;
  }
  if (semaforo_plumaArriba() != (arnes_pines[MOTOR_TALANQUERA] == TALANQUERA_ABRIR)) {
    discrepanciasPluma++;
  }
}

// ---------------------------------------------------------------------------
// D-30 (14/09): AQUI VIVIA EL VIGILANTE DE LA SENAL DE N-52, Y SALE ENTERO.
//
// Media, en cada tick de cualquier bloque, cuanto llevaba encendida
// semaforo_senalEnCurso() sin que nadie la bajara, contra un presupuesto leido del C++.
// Su sujeto era ESA funcion y nada mas: salio de semaforo.cpp con la interceptacion de
// SFTY-21, no queda ninguna otra bandera con esa forma -una que el firmware encienda y
// tenga que apagar sola- y por tanto no hay nada a lo que reapuntarlo. Se retira con sus
// globales (g_senalEnCursoAnt, g_peorDuracionSenalMs, g_senalExcedioPresupuesto,
// PRESUPUESTO_SENAL_MS), con la lectura de su presupuesto y con lo que lo imprimia.
// ---------------------------------------------------------------------------

// D-30 (14/09): AQUI VIVIA pinesCoincidenConEstado(), Y SE QUEDO SIN LLAMADOR.
//
// Comparaba los PINES que semaforo.cpp escribio contra lo que semaforo_estado() dice
// que deberia haber. Sus UNICOS llamadores eran los escenarios D1-D6, que la usaban
// como "requisito b": al terminar una senal del mando, los pines tenian que volver a
// coincidir con la logica en vez de quedarse congelados en el patron de destellos. Sin
// interceptacion no hay nada que pueda descongelar mal, y fuera del Bloque D nadie la
// llamaba nunca. Se retira en vez de dejarla huerfana (CLAUDE.md §6.1: una huerfana
// NUEVA es senal de defecto, y ademas el compilador la delata con -Wunused-function).

// ---------------------------------------------------------------------------
// LOS BOTONES YA NO SE SIMULAN: botones.cpp REAL SE COMPILA AQUI (D-13, 05/09).
//
// Aqui vivian seis definiciones -botones_setup(), botones_actualizar() y los cuatro
// botonX()- sobre cuatro bools, mas un camara_leerPin() que devolvia otro bool. Con eso,
// botones.cpp no se compilaba en NINGUN arnes del proyecto y su vigilante de camaras
// -675 lineas de pack mirandolo- no se habia ejecutado nunca.
//
// SE RETIRARON Y NO SE PIERDE COBERTURA, y esto esta censado antes de tocarlo, no
// despues: de los seis, los unicos que algun fuente compilado aqui llama son
// botonCancelar() -modo_automatico.cpp y modo_inteligente.cpp- y nada mas.
// botonArriba(), botonAbajo() y botonAceptar() tienen CERO llamadores en este binario
// desde que el asistente de tres pantallas del Automatico se retiro (N-42), y los bools
// que los movian no se ponian a true en ningun escenario salvo Arriba/Abajo, que nadie
// leia. El botonCancelar() real devuelve false SIEMPRE -sus pines son camaras desde el
// 31/08-, que es exactamente lo que devolvia el bool, que nunca se armaba.
//
// Lo que se GANA es lo que no se podia medir: el flanco de J16 lo produce ahora
// camaras_actualizar() de verdad, sobre un pin de verdad, con su antirrebote de verdad.
// ---------------------------------------------------------------------------

// N-73: la Caja Negra. El stub no puede limitarse a callar: si solo devolviera vacio,
// el arnes enlazaria y nadie sabria si la alarma se emite o no -que es exactamente el
// defecto que N-73 arreglo, una funcion que existe y no se llama-. Aqui se GUARDA lo
// ultimo reportado, y mas abajo se exige que al caer a ambar por reintentos agotados
// haya salido una alarma con su causa.
char g_ultimaAlarmaEvento[48] = "";
char g_ultimaAlarmaCausa[48]  = "";
int  g_alarmasEmitidas = 0;
void bluetooth_reportarAlarma(const char* evento, const char* causa, const char* accion) {
  (void)accion;
  std::snprintf(g_ultimaAlarmaEvento, sizeof(g_ultimaAlarmaEvento), "%s", evento);
  std::snprintf(g_ultimaAlarmaCausa,  sizeof(g_ultimaAlarmaCausa),  "%s", causa);
  g_alarmasEmitidas++;
}
// El $EVENT del vigilante tambien se guarda: los "CAMARA_RECUPERADA" y los
// "VETO_HABRIA_ACTUADO_N:" son la otra mitad de lo que la fase 1 produce, y un escenario
// que solo mirase las alarmas no podria distinguir "no alarmo" de "alarmo y se recupero".
char g_ultimoEventoTipo[48] = "";
char g_ultimoEventoDetalle[48] = "";
int  g_eventosEmitidos = 0;
void bluetooth_reportarEvento(const char* tipo, const char* detalle) {
  std::snprintf(g_ultimoEventoTipo, sizeof(g_ultimoEventoTipo), "%s", tipo);
  std::snprintf(g_ultimoEventoDetalle, sizeof(g_ultimoEventoDetalle), "%s", detalle);
  g_eventosEmitidos++;
}

// Lo que hacia pulsarAceptar(): tres confirmaciones para atravesar el asistente del
// Automatico. El asistente se retiro con N-42 -modoAutomatico_setup() deja el modo en
// marcha directamente- y botonAceptar() devuelve false desde el 31/08, asi que lo unico
// que quedaba de aquello era llamar al loop. Se conserva el nombre para no reescribir
// once llamadas por un cambio que no cambia comportamiento.
static void pulsarAceptar() {
  modoAutomatico_loop();
}

// ---------------------------------------------------------------------------
// PANTALLA SIMULADA. Solo registra la ultima llamada: este arnes mide el CICLO, no
// el dibujo -eso ya lo cubre Validacion_LCD sobre el lcd.cpp real-.
// ---------------------------------------------------------------------------
static unsigned long g_lcdRedibujos = 0;

void lcd_dibujarAutomatico(const char* nombreEstado, int, int) {
  g_lcdRedibujos++;
  (void)nombreEstado;
}
void lcd_dibujarConfigValor(const char*, int, const char*) { g_lcdRedibujos++; }
void lcd_dibujarInteligente(const char*, int, bool) { g_lcdRedibujos++; }

void menu_setup() {}

// ---------------------------------------------------------------------------
// LA CAMARA YA NO SE SIMULA: SE CIERRA EL CONTACTO EN EL PIN.
//
// Aqui habia un `static bool g_camaraLocal` y un camara_leerPin() propio que lo
// devolvia. Con botones.cpp real compilado, camara_leerPin() es LA DE VERDAD -con su
// digitalRead(), su delay(5) y su segunda lectura-, asi que el escenario deja de mover
// un bool y pasa a mover EL PIN. La diferencia no es cosmetica: J14 y J16 son pines
// distintos, y con un solo bool detras de todos ellos la pregunta "una deteccion en J16
// llega al Modo Inteligente?" salia que si por construccion.
// ---------------------------------------------------------------------------

// Cierra o abre el contacto seco de una entrada de camara. No pasa por digitalWrite()
// a proposito: eso contaria como una escritura de salida del firmware y ensuciaria
// arnes_escrituras, que es lo que mide la barrera de pines de luz.
static void cerrarContacto(int pin, bool cerrado) {
  arnes_pines[pin] = cerrado ? HIGH : LOW;
}

// El nombre viejo, conservado para los escenarios del Bloque E que ya existian: mueve la
// camara de J14 (PB0), que es la que modo_inteligente.cpp lee POR NIVEL.
static void camaraJ14(bool hayCoche) { cerrarContacto(CAM_DEMANDA_PIN, hayCoche); }

// ---------------------------------------------------------------------------
// modoActual_get()/set() DE VERDAD, y siguen haciendo falta con el mando fuera: los
// llaman coordinador.cpp (tres sitios) y modo_automatico.cpp / modo_inteligente.cpp,
// que SI se compilan aqui -modoAutomatico_enMarcha() es literalmente una lectura de
// este valor-. Medido con grep antes de tocarlos, no supuesto.
//
// Arranca en MENU, igual que el enum real: es el valor con el que main.cpp llega
// al primer loop() antes de que nadie pulse nada.
// ---------------------------------------------------------------------------
static ModoSistema g_modoActual = MENU;
ModoSistema modoActual_get() { return g_modoActual; }
void modoActual_set(ModoSistema m) { g_modoActual = m; }

// ---------------------------------------------------------------------------
// D-30 (14/09): AQUI VIVIAN LOS STUBS DE MODO_AMBAR Y MODO_DEGRADADO, Y SALEN LOS CINCO.
//
// modo_ambar_setup(), modo_ambar_fijarMotivo(), modo_degradado_evaluarEntrada() y
// modo_degradado_setup() existian por una sola razon, escrita en el comentario que
// habia aqui: "se stubean SOLO las funciones que mando.cpp llama de verdad". Retirado
// mando.cpp, se comprobo con grep cual de los .cpp que este arnes SI compila
// -coordinador, botones, semaforo, modo_automatico, modo_inteligente, demanda- las
// llama: ninguno, y las unicas apariciones que quedan son comentarios. El enlazador es
// el segundo instrumento: si alguna hiciera falta, no enlaza.
//
// Con ellas sale g_entradaDegradado, la perilla con la que el arnes elegia el veredicto
// de la puerta del Degradado para ejercer las dos ramas de A.B.A.B.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// RELOJ SIMULADO. SFTY-23 (sincronizacion horaria) esta fuera del alcance de este
// arnes -es una segunda maquina de estados independiente del ciclo, con arnes
// propio pendiente-. Con reloj_enHora() en false, atenderSincronizacion() no
// encola nada (pendHora/pendConfig/pendDelta solo se activan por
// coordinador_reiniciarConexion() o por la recuperacion SFTY-9, que este arnes no
// ejercita), asi que las funciones de reloj de aqui abajo nunca se llegan a usar
// para nada que afecte al ciclo: existen solo porque coordinador.cpp las referencia
// y el enlazador las exige.
// ---------------------------------------------------------------------------
bool reloj_enHora() { return false; }
uint8_t reloj_hora() { return 0; }
uint8_t reloj_minuto() { return 0; }
uint8_t reloj_segundo() { return 0; }
uint8_t reloj_dia() { return 0; }
uint32_t reloj_segundosDelDia() { return 0; }
uint32_t reloj_contadorSegundos() { return 0; }   // N-49: sin reloj en este arnes
void reloj_fijarEnero() {}

void respaldo_marcarSync(uint32_t) {}   // N-49: ahora recibe el contador del RTC

// N-133/N-135: EL RESPALDO DE LOS TIEMPOS DEL CICLO, DOBLADO CON MEMORIA DE VERDAD.
//
// No es un stub vacio a proposito. Un doble que devolviera siempre "no hay nada
// guardado" dejaria el camino de recuperacion de modo_automatico.cpp sin ejercer, y
// este arnes existe para ejecutar ese .cpp, no para enlazarlo. Con memoria, guardar y
// recuperar se recorren de verdad.
//
// No replica el checksum ni la FIRMA -eso lo mide maestro_02_respaldo sobre el
// respaldo.cpp real-: aqui solo importa que lo que se guardo es lo que vuelve.
static uint8_t _bkRojo = 0, _bkVerde = 0, _bkDespeje = 0;

void respaldo_guardarTiemposCiclo(uint8_t rojoMin, uint8_t verdeMin, uint8_t despejeSeg) {
  // Misma negativa que el real: un cero no es configuracion, es ausencia de ella.
  if (rojoMin == 0 || verdeMin == 0 || despejeSeg == 0) return;
  _bkRojo = rojoMin; _bkVerde = verdeMin; _bkDespeje = despejeSeg;
}

bool respaldo_tiemposCiclo(uint8_t* rojoMin, uint8_t* verdeMin, uint8_t* despejeSeg) {
  if (_bkRojo == 0 || _bkVerde == 0 || _bkDespeje == 0) return false;
  *rojoMin = _bkRojo; *verdeMin = _bkVerde; *despejeSeg = _bkDespeje;
  return true;
}

// ---------------------------------------------------------------------------
// EL ESCLAVO SIMULADO. Esto es lo unico de este arnes que "actua": contesta al
// protocolo como lo haria (o no) el otro extremo, con la latencia y el
// comportamiento que cada comprobacion elige.
// ---------------------------------------------------------------------------
enum ModoEsclavo {
  ESC_CORRECTO,    // contesta lo que el protocolo pide, con latencia de radio
  ESC_MUDO,        // no contesta NADA -ni al latido-: orfandad de verdad
  ESC_TRAMA_MALA,  // contesta, pero con el comando que NO corresponde
};
static ModoEsclavo g_modoEsclavo = ESC_CORRECTO;
static unsigned long g_latenciaEsclavoMs = 50;

struct PaqueteEntrante { bool hay; RF_Packet pkt; unsigned long tEntrega; };
static PaqueteEntrante g_entrante = { false, {0, 0, 0, 0}, 0 };

// El instante de la ULTIMA entrega, tal como lo veria coordinador.cpp al fijar
// tUltimaRxEsclavo. No hay getter de esa variable interna -es estatica del
// modulo-, asi que el arnes lleva su propia copia del mismo evento: el arnes es
// quien decide cuando "llega" un paquete, de modo que esta marca de tiempo es
// exacta y no una aproximacion.
static unsigned long g_ultimaEntregaMs = 0;

void protocolo_setup() {}
void protocolo_resetReplayProtection() {}

void protocolo_enviarPaquete(uint8_t cmd, uint8_t param) {
  (void)param;
  if (g_modoEsclavo == ESC_MUDO) return;  // orfandad real: nadie contesta nada

  RF_Packet resp = { 0, 0, 0, 0 };
  bool contesta = true;
  switch (cmd) {
    case CMD_GO_GREEN:
      resp.command = (g_modoEsclavo == ESC_TRAMA_MALA) ? CMD_ACK_RED : CMD_ACK_GREEN;
      break;
    case CMD_GO_RED:
      resp.command = CMD_ACK_RED;
      break;
    case CMD_PING:
      resp.command = CMD_PONG;
      break;
    default:
      contesta = false;  // hora, delta, config: fuera del alcance de este arnes
  }
  if (!contesta) return;

  g_entrante.hay = true;
  g_entrante.pkt = resp;
  g_entrante.tEntrega = arnes_millis_valor + g_latenciaEsclavoMs;
}

// A-12: LA DEMANDA DEL OTRO LADO, QUE NO ES UNA RESPUESTA SINO UNA TRAMA ESPONTANEA.
//
// El Esclavo simulado de arriba solo REACCIONA a lo que el Maestro le manda. CMD_DEMANDA
// no es eso: la levanta la camara del sentido 2 cuando le da la gana, y viaja sola. Se
// encola aparte para no pisar una respuesta en vuelo -si compartiera el hueco, encolar
// una demanda mientras el coordinador espera un ACK_GREEN borraria ese ACK y el arnes
// mediria una orfandad que el firmware no tiene-.
static bool g_demandaRemotaEncolada = false;

static void encolarDemandaRemota() { g_demandaRemotaEncolada = true; }

bool protocolo_hayPaqueteDisponible(RF_Packet* destino) {
  if (g_demandaRemotaEncolada && !g_entrante.hay) {
    g_demandaRemotaEncolada = false;
    RF_Packet d = { 0, 0, 0, 0 };
    d.command = CMD_DEMANDA;
    *destino = d;
    g_ultimaEntregaMs = arnes_millis_valor;
    return true;
  }
  if (!g_entrante.hay) return false;
  if (arnes_millis_valor < g_entrante.tEntrega) return false;
  *destino = g_entrante.pkt;
  g_entrante.hay = false;
  g_ultimaEntregaMs = arnes_millis_valor;
  return true;
}

// ---------------------------------------------------------------------------
// BOMBEO DEL RELOJ SIMULADO. Cada paso llama a la funcion que se esta midiendo,
// vigila el enclavamiento sobre lo que de verdad se escribio en los pines, y
// comprueba la condicion ANTES de avanzar el reloj -asi el instante en el que la
// condicion se cumple queda en arnes_millis_valor, listo para medir con precision
// de 'pasoMs'-.
// ---------------------------------------------------------------------------
template <typename Paso, typename Cond>
static long bombearGenerico(unsigned long pasoMs, unsigned long presupuestoMs,
                             Paso paso, Cond condicion) {
  unsigned long gastado = 0;
  for (;;) {
    paso();
    vigilarEnclavamiento();
    if (condicion()) return (long)gastado;
    if (gastado >= presupuestoMs) return -1;
    arnes_millis_valor += pasoMs;
    gastado += pasoMs;
  }
}

template <typename Cond>
static long bombear(unsigned long pasoMs, unsigned long presupuestoMs, Cond condicion) {
  return bombearGenerico(pasoMs, presupuestoMs, []() { modoAutomatico_loop(); }, condicion);
}

// Arranca un modo Automatico limpio: fase CONFIG_ROJO -> CONFIG_VERDE ->
// CONFIG_ESTATICO -> CORRIENDO, aceptando los valores por defecto (1 min rojo,
// 1 min verde, el despeje por defecto que se relee mas abajo). Es exactamente lo
// que hace un operario que confirma tres veces sin tocar Arriba/Abajo, y es la
// puerta de entrada real al ciclo: si esto no llegase a CORRIENDO, ninguna
// comprobacion posterior significaria nada.
static void arrancarAutomaticoPorDefecto() {
  coordinador_setup();
  g_modoActual = MODO_AUTOMATICO;  // lo que main.cpp ya habria fijado al entrar
  // Limpia flancos de boton que hubieran quedado sin consumir. Desde el 05/09 esto
  // lo hace botones_setup() REAL, que es ademas lo que hace el equipo al arrancar: y
  // de paso siembra camAnt[] y camAltoDesde[] leyendo los pines (N-26 aplicado a las
  // camaras), que es la unica forma de que un bloque no arrastre al siguiente una
  // camara que quedo con el contacto cerrado.
  botones_setup();
  modoAutomatico_setup();          // fase = CONFIG_ROJO
  pulsarAceptar();                 // -> CONFIG_VERDE
  pulsarAceptar();                 // -> CONFIG_ESTATICO
  pulsarAceptar();                 // -> CORRIENDO: coordinador_iniciarModo()
}

// ---------------------------------------------------------------------------
// D-30 (14/09): AQUI VIVIA LA CADENA DE BOMBEO DEL BLOQUE D, Y SE VA ENTERA.
//
// Eran tres piezas encadenadas y con un unico consumidor entre las tres:
//
//   pasoPrincipal()    la vuelta COMPLETA de main.cpp -botones_actualizar(),
//                      semaforo_actualizar(), el loop del modo y, hasta hoy,
//                      mando_actualizar()-. Hacia falta porque una senal del mando solo
//                      podia empezar y terminar si se llamaba a las dos funciones del
//                      mando en el sitio exacto que les toca en la vuelta real.
//   avanzar()          corria N ms de reloj a base de pasoPrincipal().
//   bombearPrincipal() como bombear(), pero con pasoPrincipal() como paso.
//
// Las tres las usaba SOLO el Bloque D. Los bloques que quedan avanzan con bombear() (el
// lazo del modo), bombearInteligente(), o con sus propios lazos -correrConPluma() en el
// F y correrG() en el G-, que llaman a botones_actualizar() por su cuenta y en el orden
// de main.cpp, con su porque escrito alli. Dejarlas seria dejar tres huerfanas nuevas.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// A-12 — EL BOMBEO DEL MODO INTELIGENTE, Y POR QUE ES OTRO.
//
// bombear() llama a modoAutomatico_loop() a pelo. Para el Bloque E hace falta el
// otro loop, y ademas hay que poder correr LOS DOS con la MISMA configuracion y el
// MISMO paso para que la comparacion de duraciones signifique algo: la propiedad
// que sostiene todo el modo es "con las camaras muertas se comporta EXACTAMENTE
// como el Automatico", y eso solo se puede afirmar midiendo las dos cosas con la
// misma regla.
// ---------------------------------------------------------------------------
template <typename Cond>
static long bombearInteligente(unsigned long pasoMs, unsigned long presupuestoMs,
                               Cond condicion) {
  return bombearGenerico(pasoMs, presupuestoMs, [](){ modoInteligente_loop(); },
                         condicion);
}

// Deja el equipo con unos tiempos de ciclo CONFIGURADOS -los que mandaria el
// operario por SET_TIEMPOS- y entra en el modo que se pida, desde cero.
//
// El orden importa: modoAutomatico_fijarTiempos() se niega mientras
// modoAutomatico_enMarcha(), asi que la configuracion se hace con el equipo FUERA
// del Automatico, igual que en el equipo real -es la misma guarda que rechaza con
// $ERR,CMD:SET_TIEMPOS,DESC:EN_MARCHA_PARE_EL_MODO-.
static bool configurarTiempos(int verdeMin, int rojoMin, int despejeSeg) {
  g_modoActual = MENU;
  return modoAutomatico_fijarTiempos((uint8_t)verdeMin, (uint8_t)rojoMin,
                                     (uint8_t)despejeSeg);
}

static void arrancarInteligente() {
  coordinador_setup();
  camaraJ14(false);
  cerrarContacto(CAM_C_PIN, false);
  cerrarContacto(CAM_D_PIN, false);
  g_demandaRemotaEncolada = false;
  coordinador_limpiarDemandaRemota();
  botones_setup();
  g_modoActual = MODO_INTELIGENTE;   // lo que main.cpp habria fijado al entrar
  modoInteligente_setup();
}

static void arrancarAutomatico() {
  coordinador_setup();
  botones_setup();
  g_modoActual = MODO_AUTOMATICO;
  modoAutomatico_setup();
}

// Cuanto dura la fase de VERDE del Maestro, medida sobre lo que semaforo.cpp
// escribio: desde que el coordinador queda listo para contar con la luz en verde
// hasta que deja de estar en verde. Es la unica cifra que el operario ve en la
// calle, y la misma para los dos modos.
//
// Devuelve -1 si no se llego a verde o si el verde no termino dentro del
// presupuesto: un -1 se distingue de un numero, que es justo lo que hace falta para
// que un modo que se queda pegado no se confunda con uno que dura mucho.
template <typename Paso>
static long medirFase(Paso paso, EstadoSemaforo color, unsigned long pasoMs,
                      unsigned long presupuestoMs) {
  if (bombearGenerico(pasoMs, presupuestoMs, paso,
        [color](){ return semaforo_estado() == color && coordinador_listoParaContar(); }) < 0) {
    return -1;
  }
  // EL FINAL DE LA FASE ES EL INSTANTE EN QUE EL MODO PIDE EL CAMBIO, y ese instante
  // se lee en el coordinador: pedirCambio() lo saca de C_IDLE. Medir "hasta que la luz
  // cambie" solo valdria para el verde -del rojo se sale por un despeje que dura otra
  // cosa- y entonces las dos fases no serian comparables entre si ni entre modos.
  return bombearGenerico(pasoMs, presupuestoMs, paso,
        [](){ return !coordinador_listoParaContar(); });
}

int main() {
  std::printf("==============================================================\n");
  std::printf(" ARNES DEL CICLO AUTOMATICO - coordinador.cpp + semaforo.cpp +\n");
  std::printf(" modo_automatico.cpp + modo_inteligente.cpp + botones.cpp REALES,\n");
  std::printf(" compilados y ejecutados en el PC (Bloque E / A-12: el Modo\n");
  std::printf(" Inteligente contra el Automatico; Bloque F/G: camaras y pluma)\n");
  std::printf("==============================================================\n");

  // -------------------------------------------------------------------------
  // Constantes releidas del C++ real. Ni una se escribe a mano: si el patron no
  // aparece, el arnes ABORTA antes de comprobar nada (ver leerConstante()).
  // -------------------------------------------------------------------------
  // D-33: ESTA VA LA PRIMERA, y no por orden alfabetico. vigilarEnclavamiento() la usa
  // para decidir si una pluma arriba sin verde esta justificada; si se leyera despues de
  // arrancar los bloques, los primeros ticks compararian contra un cero y acusarian al
  // firmware de un defecto que no tiene.
  g_retardoPlumaMs = (unsigned long)leerConstante("semaforo.cpp",
      R"(PLUMA_RETARDO_BAJADA_MS\s*=\s*(\d+)UL)",
      "el retardo con el que la pluma baja DESPUES del rojo (D-33)");

  // D-30 (14/09): AQUI SE LEIA DESTELLO_ON_MS DEL C++ PARA EL MARGEN DE LA PLUMA, y ya
  // no se lee: esa constante salio de semaforo.cpp con el mando. El margen de hoy es una
  // propiedad de ESTE arnes -su paso de muestreo- y no del firmware; vive en
  // MARGEN_MUESTREO_MS, con el porque escrito al lado de su declaracion.

  long AMBAR_MS = leerConstante("semaforo.cpp",
      R"(S_AMARILLO\s*&&\s*\(ahora\s*-\s*tCambio\s*>=\s*(\d+)\))",
      "la duracion del amarillo fijo Rojo->Verde (SFTY-5)");

  long FALLO_PERIODO_MS = leerConstante("semaforo.cpp",
      R"(S_FALLO\)\s*\{\s*if\s*\(ahora\s*-\s*tCambio\s*>=\s*(\d+)\))",
      "el periodo del ambar intermitente de fallo");

  // N-69: el umbral se mudo al contrato compartido (protocolo.h) para que las dos
  // puntas no puedan divergir. Este arnes ABORTO el dia del cambio en vez de seguir
  // midiendo el numero viejo, que es lo que se le pide.
  long ORFANDAD_MS = leerConstante("../include/protocolo.h",
      R"(#define\s+SFTY6_SILENCIO_MS\s+(\d+)UL)",
      "el timeout de orfandad (SFTY-6)");

  long TIMEOUT_ACK_MS = leerConstante("coordinador.cpp",
      R"(TIMEOUT_ACK_MS\s*=\s*(\d+))",
      "el timeout de reintento de ACK (SFTY-7)");

  std::vector<long> reintentos = leerConstantesTodas("coordinador.cpp",
      R"(CICLO_MAX_REINTENTOS\s*=\s*(\d+))",
      "el numero de reintentos del ciclo antes de C_FALLO");

  // LOS VALORES POR DEFECTO YA NO SON LITERALES, Y ESO ES EL ARREGLO (N-131, 04/09).
  //
  // Aqui se leian tres numeros escritos a mano en el inicializador:
  //     static int minRojo = 1, minVerde = 1, segEstatico = 15;
  // El 04/09 se descubrio que ese "1" convertia la guarda de 3 minutos en media
  // guarda -solo la cruzaba SET_TIEMPOS- y los defectos pasaron a salir de las MISMAS
  // constantes que el limite. Este arnes ABORTO en la primera corrida siguiente, que
  // es exactamente su trabajo: leia por patron literal y el patron dejo de existir
  // (CLAUDE.md §5). Un ABORTADO grita; lo que no se puede permitir es que siguiera
  // dando veredicto sobre un firmware que ya no describe.
  //
  // Ahora se leen las constantes por nombre. Sin valor por defecto, igual que antes:
  // si alguien las renombra, esto vuelve a ABORTAR en vez de medir otra cosa.
  long MIN_ROJO_DEFECTO = leerConstante("limites_ciclo.h",
      R"(ROJO_MIN_MIN\s*=\s*(\d+))",
      "el minimo de rojo, que es tambien el valor de arranque del asistente");
  long MIN_VERDE_DEFECTO = leerConstante("limites_ciclo.h",
      R"(VERDE_MIN_MIN\s*=\s*(\d+))",
      "el minimo de verde, que es tambien el valor de arranque del asistente");
  long SEG_ESTATICO_DEFECTO = leerConstante("limites_ciclo.h",
      R"(DESPEJE_SEG_MIN\s*=\s*(\d+))",
      "el despeje All-Red minimo, que es tambien el de arranque");
  (void)MIN_ROJO_DEFECTO;

  std::printf("\nConstantes releidas del C++ real (Maestro/src), no escritas a mano:\n");
  std::printf("   amarillo fijo (SFTY-5) ......... %ld ms\n", AMBAR_MS);
  std::printf("   periodo del ambar de fallo ..... %ld ms\n", FALLO_PERIODO_MS);
  std::printf("   orfandad (SFTY-6) .............. %ld ms\n", ORFANDAD_MS);
  std::printf("   timeout de ACK (SFTY-7) ........ %ld ms\n", TIMEOUT_ACK_MS);
  std::printf("   despeje por defecto del asistente %ld s\n", SEG_ESTATICO_DEFECTO);
  std::printf("   minutos de verde por defecto .... %ld min\n", MIN_VERDE_DEFECTO);

  // N-71: ESTA COMPROBACION SE INVIRTIO, Y CONVIENE SABER POR QUE.
  //
  // Exigia encontrar DOS literales iguales -"retryCount >= 5" en la rama verde y en la
  // roja- porque el numero estaba escrito dos veces y podian divergir. Al darle nombre
  // (CICLO_MAX_REINTENTOS) esa duplicidad desaparece, y con ella el defecto que la
  // comprobacion vigilaba: ya no se puede cambiar una rama y olvidar la otra.
  //
  // Pero eliminarla sin mas dejaria un hueco: una sola declaracion no sirve de nada si
  // alguna rama sigue con su literal. Asi que ahora se exige lo contrario y mas fuerte:
  // UNA sola declaracion, y las DOS ramas usandola por nombre.
  int usosPorNombre = 0;
  {
    const std::string src = leerArchivoFuente("coordinador.cpp");
    std::size_t pos = 0;
    while ((pos = src.find("retryCount >= CICLO_MAX_REINTENTOS", pos)) != std::string::npos) {
      usosPorNombre++;
      pos++;
    }
  }
  comprobar(reintentos.size() == 1 && usosPorNombre == 2,
            "el numero de reintentos se declara UNA vez y las DOS ramas (verde y roja) "
            "lo usan por nombre: ya no hay dos literales que puedan divergir");
  long MAX_REINTENTOS = reintentos[0];

  unsigned long SEG_ESTATICO_MS = (unsigned long)SEG_ESTATICO_DEFECTO * 1000UL;
  // 60000 = ms por minuto. Es aritmetica de unidades, identica en el firmware y
  // aqui por definicion -no es una politica que el firmware pueda decidir cambiar
  // sin que deje de ser "minutos"-, asi que no se relee por regex como las demas.
  unsigned long MIN_VERDE_MS = (unsigned long)MIN_VERDE_DEFECTO * 60000UL;

  // D-30 (14/09): AQUI SE LEIAN LAS DIEZ CONSTANTES DE LA SENAL DEL MANDO.
  //
  // DESTELLO_ON_MS / DESTELLO_OFF_MS / AMBAR_RAPIDO_PERIODO_MS de semaforo.cpp y
  // DESTELLOS_AUTOMATICO / _AMBAR / _DEGRADADO, VENTANA_TRIPLE_MS, VENTANA_CUADRUPLE_MS
  // y RECHAZO_AMBAR_MS de mando.cpp, mas el PRESUPUESTO_SENAL_MS que se derivaba de
  // ellas. Ninguna existe ya en el firmware: las tres primeras salieron de semaforo.cpp
  // con la interceptacion y el resto con el fichero entero. Se retiran en vez de
  // quedarse con un valor por defecto, que es lo que leerConstante() prohibe.

  // ===========================================================================
  // BLOQUE A: semaforo.cpp EN AISLAMIENTO — SFTY-5, medido al milisegundo
  // ===========================================================================
  std::printf("\n-- Bloque A: semaforo.cpp en aislamiento (SFTY-5) --\n");
  {
    semaforo_setup();
    arnes_millis_valor = 1000000UL;   // arranca lejos de 0 a proposito
    unsigned long t0 = arnes_millis_valor;

    semaforo_iniciarTransicionAVerde();
    vigilarEnclavamiento();
    comprobar(semaforo_estado() == S_AMARILLO,
              "Rojo->Verde pasa SIEMPRE por AMARILLO, nunca salta a VERDE directo");

    arnes_millis_valor = t0 + (unsigned long)AMBAR_MS - 1UL;
    semaforo_actualizar();
    vigilarEnclavamiento();
    comprobar(semaforo_estado() == S_AMARILLO,
              "un ms antes del limite leido del C++ real, el amarillo AUN no cedio el paso");

    arnes_millis_valor = t0 + (unsigned long)AMBAR_MS;
    semaforo_actualizar();
    vigilarEnclavamiento();
    comprobar(semaforo_estado() == S_VERDE,
              "en el limite EXACTO leido del C++ real, el amarillo cede el paso al verde "
              "(fila 9 del README medida sobre el binario, no sobre la tabla)");

    semaforo_forzarRojo();
    vigilarEnclavamiento();
    comprobar(semaforo_estado() == S_ROJO,
              "Verde->Rojo es DIRECTO: una sola llamada basta, sin amarillo intermedio "
              "(fila 8 del README: 0s)");
  }

  std::printf("\n-- Bloque A2: parpadeo de FALLO (ambar intermitente) --\n");
  {
    semaforo_setup();
    arnes_millis_valor = 2000000UL;
    semaforo_iniciarFallo();
    unsigned long t0 = arnes_millis_valor;
    vigilarEnclavamiento();
    comprobar(arnes_pines[AMARILLO1] == LOW,
              "al entrar en FALLO el ambar arranca APAGADO -si arrancara encendido, el "
              "primer destello no se veria como tal-");

    arnes_millis_valor = t0 + (unsigned long)FALLO_PERIODO_MS;
    semaforo_actualizar();
    vigilarEnclavamiento();
    comprobar(arnes_pines[AMARILLO1] == HIGH,
              "en el primer limite del periodo leido del C++, el ambar de FALLO se enciende");

    arnes_millis_valor = t0 + 2UL * (unsigned long)FALLO_PERIODO_MS;
    semaforo_actualizar();
    vigilarEnclavamiento();
    comprobar(arnes_pines[AMARILLO1] == LOW,
              "en el segundo limite vuelve a apagarse: PARPADEA, no queda ambar fijo");

    comprobar(semaforo_estado() == S_FALLO,
              "durante todo el parpadeo el estado logico se mantiene en S_FALLO");

    // SFTY-28, la politica del cliente EJERCIDA y no solo exceptuada. El invariante
    // global de la pluma se calla en S_FALLO; si nadie comprobara aqui que ademas
    // esta ARRIBA, borrar la excepcion del firmware dejaria la barrera cerrando la
    // via en cada caida de enlace y el arnes seguiria en verde.
    comprobar(arnes_pines[MOTOR_TALANQUERA] == TALANQUERA_ABRIR,
              "con el ambar intermitente de SFTY-6 la talanquera queda ARRIBA: sin "
              "enlace se deja pasar con precaucion, que es la politica elegida el "
              "27/08 -y la contraria, cerrar la via, seria un corredor sin salida-");
  }

  // ===========================================================================
  // BLOQUE B: EL CICLO COMPLETO — coordinador + semaforo + modo_automatico REALES
  // ===========================================================================
  std::printf("\n-- Bloque B: el ciclo COMPLETO del Modo Automatico, sobre el C++ real --\n");
  {
    arnes_millis_valor = 5000000UL;
    g_modoEsclavo = ESC_CORRECTO;
    g_latenciaEsclavoMs = 50;

    arrancarAutomaticoPorDefecto();
    vigilarEnclavamiento();
    comprobar(semaforo_estado() == S_ROJO,
              "al arrancar el Automatico, el Maestro sale por ROJO -no por lo ultimo "
              "que hubiera antes-");

    // ESTA es la comprobacion que la regresion se salto por completo: hasta hoy
    // NINGUN instrumento verificaba, sobre el C++ real, que el Modo Automatico
    // llega a dar VERDE. simulador_sistema_v7_6.py y validador_maestro.py median
    // un modelo en Python que podia seguir "funcionando" aunque coordinador.cpp
    // real se hubiera roto.
    long msHastaVerde = bombear(200, SEG_ESTATICO_MS + (unsigned long)AMBAR_MS + 5000UL,
        [](){ return semaforo_estado() == S_VERDE; });
    comprobar(msHastaVerde >= 0,
              "EL MODO AUTOMATICO ARRANCA EL CICLO: el Maestro llega a VERDE tras el "
              "todo-rojo inicial y el amarillo, dentro del presupuesto de tiempo");

    comprobar(coordinador_listoParaContar(),
              "tras llegar a VERDE, el coordinador queda listo para contar la duracion "
              "(C_IDLE): modo_automatico.cpp puede empezar a medir el minuto de verde");

    // El propio modo_automatico.cpp es quien cuenta minVerde y pide el cambio: se
    // avanza esa duracion y se comprueba que el Maestro suelta el verde SOLO -si el
    // contador no arrancara (la forma concreta en que la regresion se manifiesta en
    // campo), esto se quedaria en VERDE para siempre y el bombeo agotaria el
    // presupuesto-.
    long msHastaRojo = bombear(200, MIN_VERDE_MS + 2000UL,
        [](){ return semaforo_estado() != S_VERDE; });
    comprobar(msHastaRojo >= 0,
              "agotado el minuto de VERDE configurado, el Maestro suelta el verde solo, "
              "sin intervencion externa -el sintoma de campo era exactamente que esto NO "
              "pasaba-");
    comprobar(semaforo_estado() == S_ROJO,
              "Verde->Rojo del Maestro es DIRECTO (0s): en el mismo tick cae a ROJO, "
              "nunca pasa por AMARILLO (fila 8 del README, SFTY-5)");

    // Y el turno pasa al Esclavo: el Maestro vuelve a quedar listo para contar,
    // con su propia luz en ROJO -senal de que concedio el verde al otro lado tras
    // un intercambio GO_GREEN/ACK_GREEN real, no supuesto-.
    long msHastaListo = bombear(200, SEG_ESTATICO_MS + (unsigned long)TIMEOUT_ACK_MS + 3000UL,
        [](){ return coordinador_listoParaContar(); });
    comprobar(msHastaListo >= 0,
              "tras el todo-rojo y el intercambio GO_GREEN/ACK_GREEN con el Esclavo "
              "simulado, el coordinador vuelve a quedar listo: le paso el turno al otro "
              "lado de verdad, no solo en apariencia");
    comprobar(semaforo_estado() == S_ROJO,
              "mientras el turno es del Esclavo, el Maestro permanece en ROJO FIJO");
  }

  // ===========================================================================
  // BLOQUE C: ORFANDAD EN PLENO CICLO — SFTY-6, filas 1/3/4/5 del README
  // ===========================================================================
  std::printf("\n-- Bloque C: orfandad en pleno ciclo (SFTY-6) --\n");
  {
    arnes_millis_valor += 10000000UL;
    g_modoEsclavo = ESC_CORRECTO;
    g_latenciaEsclavoMs = 50;

    arrancarAutomaticoPorDefecto();
    bombear(200, SEG_ESTATICO_MS + (unsigned long)AMBAR_MS + 5000UL,
        [](){ return coordinador_listoParaContar(); });
    comprobar(coordinador_listoParaContar() && !coordinador_comunicacionPerdida(),
              "con el Esclavo simulado respondiendo, el ciclo llega a C_IDLE sin caer "
              "en fallo");

    // Deja pasar un latido completo (3 s) ANTES de cortar el enlace, para que
    // g_ultimaEntregaMs quede fijado por un intercambio real y no por lo que
    // quedara de la ultima ACK del arranque -asi el instante de referencia de esta
    // comprobacion es el MISMO que coordinador.cpp usa para tUltimaRxEsclavo-.
    bombear(200, 4000, [](){ return false; });

    g_modoEsclavo = ESC_MUDO;   // el Esclavo "se apaga": ni luz ni latido
    unsigned long t0 = g_ultimaEntregaMs;

    long r = bombearGenerico(50, (unsigned long)ORFANDAD_MS + 5000UL,
        [](){ modoAutomatico_loop(); },
        [](){ return coordinador_comunicacionPerdida(); });
    comprobar(r >= 0,
              "sin respuesta del Esclavo -ni siquiera al latido-, el Maestro SI "
              "detecta la orfandad y cae a C_FALLO");

    if (r >= 0) {
      long limite = (long)t0 + (long)ORFANDAD_MS;
      long observado = (long)arnes_millis_valor;
      long diff = observado - limite;
      if (diff < 0) diff = -diff;
      // La tolerancia es el paso de bombeo (50ms), no un margen de gracia del
      // firmware: el propio codigo cae a C_FALLO en el mismo tick en que
      // millis()-tUltimaRxEsclavo supera los 12000 leidos arriba.
      char msg[200];
      std::snprintf(msg, sizeof(msg),
          "la orfandad se detecta A LOS %ld ms leidos del C++ real, ni antes ni "
          "sensiblemente despues (SFTY-6; desviacion medida: %ld ms)",
          ORFANDAD_MS, diff);
      comprobar(diff <= 50, msg);
    }

    comprobar(semaforo_estado() == S_FALLO,
              "detectada la orfandad, el Maestro DEJA de dar verde o rojo fijo y pasa "
              "a AMBAR INTERMITENTE: el estado seguro, no un semaforo apagado ni "
              "congelado en la luz que tuviera");
  }

  // ===========================================================================
  // CONTROL NEGATIVO 1: el Esclavo contesta con el comando que NO toca
  // ===========================================================================
  std::printf("\n-- Control negativo 1: ACK_RED en respuesta a un GO_GREEN --\n");
  {
    arnes_millis_valor += 10000000UL;
    g_modoEsclavo = ESC_TRAMA_MALA;
    g_latenciaEsclavoMs = 50;

    arrancarAutomaticoPorDefecto();
    bombear(200, SEG_ESTATICO_MS + (unsigned long)AMBAR_MS + 5000UL,
        [](){ return semaforo_estado() == S_VERDE; });
    bombear(200, MIN_VERDE_MS + 2000UL,
        [](){ return semaforo_estado() != S_VERDE; });

    // Aqui es donde el Maestro SI necesita al Esclavo, y el Esclavo simulado
    // contesta con el comando equivocado a proposito. El presupuesto tiene que
    // cubrir DOS esperas seguidas, no solo la de los reintentos: primero el
    // todo-rojo (SEG_ESTATICO_MS) antes de que se mande el primer GO_GREEN, y
    // solo despues arrancan los reintentos de ACK.
    long presupuesto = (long)SEG_ESTATICO_MS +
        (unsigned long)TIMEOUT_ACK_MS * (unsigned long)(MAX_REINTENTOS + 2) + 5000UL;
    bool aceptoLaTramaMala = false;
    long gastado = 0;
    while (gastado <= presupuesto) {
      modoAutomatico_loop();
      vigilarEnclavamiento();
      if (coordinador_listoParaContar()) { aceptoLaTramaMala = true; break; }
      if (coordinador_comunicacionPerdida()) break;
      arnes_millis_valor += 200;
      gastado += 200;
    }

    comprobar(!aceptoLaTramaMala,
              "CONTROL NEGATIVO: una respuesta con el comando EQUIVOCADO (ACK_RED a "
              "un GO_GREEN) NUNCA se acepta como si fuera el ACK_GREEN esperado -si "
              "el coordinador aceptara cualquier trama entrante, esta linea fallaria-");
    // N-73: y al caer, la Caja Negra tiene que haber dejado rastro. Antes de hoy no
    // dejaba ninguno: el tecnico veia una luz ambar y ni fecha ni causa. Se exige la
    // CAUSA concreta -distinguir "se agotaron los reintentos" de "silencio total" es
    // la diferencia entre un enlace que se degrada y uno que se corta-.
    comprobar(g_alarmasEmitidas > 0 &&
              std::strcmp(g_ultimaAlarmaEvento, "FALLO_RF") == 0 &&
              std::strcmp(g_ultimaAlarmaCausa, "REINTENTOS_AGOTADOS") == 0,
              "al caer a ambar por reintentos agotados, la Caja Negra emitio "
              "FALLO_RF/REINTENTOS_AGOTADOS -sin esto el tecnico ve la luz y no sabe "
              "si el enlace se degrado o se corto (N-73)-");

    comprobar(coordinador_comunicacionPerdida(),
              "CONTROL NEGATIVO: agotados los reintentos sin la respuesta correcta, "
              "el Maestro cae a C_FALLO (estado seguro) en vez de quedarse esperando "
              "para siempre");
  }

  // ===========================================================================
  // CONTROL NEGATIVO 2: todo-rojo configurado a 0 (imposible por la UI, no por el
  // propio coordinador -el piso de 5s de FIX H-2 vive en modo_automatico.cpp-)
  // ===========================================================================
  std::printf("\n-- Control negativo 2: despeje All-Red = 0 pedido DIRECTO a la API --\n");
  {
    arnes_millis_valor += 10000000UL;
    g_modoEsclavo = ESC_CORRECTO;
    g_latenciaEsclavoMs = 50;

    coordinador_setup();
    coordinador_configurar(0, 60000, 60000);
    coordinador_iniciarModo();
    vigilarEnclavamiento();
    comprobar(semaforo_estado() == S_ROJO,
              "con despeje=0 el arranque sigue empezando en ROJO, no salta directo a "
              "verde por tener el todo-rojo a cero");

    // N-162 (bloque G del arnes de las dos puntas): ESTA LINEA AFIRMABA DOS COSAS y una era
    // el defecto. Pedia el amarillo a <= 5 ms de coordinador_iniciarModo(), o sea el verde
    // propio arrancando SIN esperar el ACK_RED de su GO_RED; con esa trama perdida y el
    // Esclavo en verde eran 180 s de verde en las dos. Se REPARTE (CLAUDE.md 9): lo que
    // sigue valiendo -no hay atajo que se salte el amarillo con el despeje a cero- se queda
    // entero; el plazo se cuenta desde el ACK_RED, que el Esclavo simulado entrega a
    // g_latenciaEsclavoMs, y se exige tambien que NO llegue antes.
    long msHastaAmarillo = bombearGenerico(1, 200,
        [](){ coordinador_actualizar(); },
        [](){ return semaforo_estado() == S_AMARILLO; });
    const std::string queAmarillo =
        "CONTROL NEGATIVO: con despeje=0 el todo-rojo se salta casi al instante DESDE EL "
        "ACK_RED (" + std::to_string(msHastaAmarillo) + " ms, acuse a " +
        std::to_string(g_latenciaEsclavoMs) + " ms) y no antes, pero "
        "coordinador_actualizar() SIGUE llamando a semaforo_iniciarTransicionAVerde(): no "
        "hay un atajo que se salte el aviso de amarillo por tener el despeje a cero";
    comprobar(msHastaAmarillo >= (long)g_latenciaEsclavoMs &&
              msHastaAmarillo <= (long)g_latenciaEsclavoMs + 5,
              queAmarillo.c_str());

    long msHastaVerde = bombearGenerico(1, (unsigned long)AMBAR_MS + 500UL,
        [](){ coordinador_actualizar(); },
        [](){ return semaforo_estado() == S_VERDE; });
    comprobar(msHastaVerde >= (long)AMBAR_MS - 2 && msHastaVerde <= (long)AMBAR_MS + 2,
              "CONTROL NEGATIVO: aun con la configuracion imposible, el amarillo "
              "dura EXACTAMENTE lo que el C++ real dice que dura -SFTY-5 no depende "
              "de que la UI haya validado el despeje-");
  }

  // ===========================================================================
  // D-30 (14/09): AQUI ESTABA EL BLOQUE D - EL MANDO DE RELES (SFTY-21)
  // ===========================================================================
  //
  // Nueve escenarios (D1-D8 mas el fuzz D9) que pulsaban A.A.A, B.B.B y A.B.A.B sobre
  // mando.cpp REAL y median los destellos sobre los PINES. Se retiran porque su sujeto
  // se retiro: no hay mando.cpp, no hay semaforo_destellosRojos() ni semaforo_ambarRapido()
  // ni semaforo_senalEnCurso(), y la interceptacion de escrituras que hacia falta medir
  // salio entera de aplicarSalidas(). No es una prueba que se acalla: es una prueba sin
  // sujeto (CLAUDE.md §9, "se BORRA si solo documentaba el defecto" -- aqui, si solo
  // documentaba un mecanismo que ya no existe).
  //
  // Las CUATRO comprobaciones del D9 que NO median el mando se mudaron con su bloque
  // literal al resumen de invariantes del final de main(). Ver alli, que ademas lleva
  // escrito lo que se perdio por el camino.

  // ===========================================================================
  // BLOQUE E: EL MODO INTELIGENTE — modo_inteligente.cpp REAL, contra el Automatico
  // ===========================================================================
  //
  // A-12 (05/09). Este modo se fijaba los tiempos por su cuenta -VERDE_MIN_MIN en el
  // arranque, y nadie mas los escribia nunca- y su Regla 1 podia cortar un verde a los
  // 15 SEGUNDOS. Un operario que configuraba 6 minutos veia el cruce correr a 3, y la
  // app no tenia la culpa: mandaba bien el dato.
  //
  // LO QUE SE MIDE AQUI, Y POR QUE NINGUN PACK PODIA HACERLO: la propiedad que hace
  // seguro este modo es de COMPORTAMIENTO -"con las camaras muertas hace exactamente lo
  // que el Automatico"- y solo se puede afirmar corriendo los dos modos con la misma
  // configuracion y comparando las dos duraciones. Leer el fuente diria que las lineas
  // estan; no diria que las dos fases duran lo mismo.
  //
  // 🔴 LO QUE ESTE BLOQUE NO EJERCE, ESCRITO PARA QUE NO SE LEA COMO APROBADO: la
  // recuperacion PEREZOSA del respaldo. modoAutomatico_tiemposCiclo() llama a
  // recuperarTiemposGuardados() la PRIMERA vez que alguien pregunta, para el equipo que
  // vuelve de un corte y entra directo a Inteligente sin pasar por el Automatico. En
  // este proceso los Bloques A-D ya han corrido modoAutomatico_setup() decenas de veces,
  // asi que esa primera vez ya paso y aqui no se puede volver a provocar. Lo que SI se
  // ejerce es recuperarTiemposGuardados() en si -por la puerta del setup()-; lo que no,
  // es que el getter la dispare. Queda como residual de A-12.
  std::printf("\n-- Bloque E: el Modo Inteligente sobre el C++ real (A-12) --\n");
  {
    // Las dos cifras que gobiernan el modo se releen del C++. Ni una escrita a mano: si
    // el patron desaparece, esto ABORTA en vez de medir contra un numero de ayer.
    long VERDE_MAX = leerConstante("limites_ciclo.h",
        R"(VERDE_MIN_MAX\s*=\s*(\d+))",
        "el maximo de verde del rango vial, que es donde satura el techo");
    long FACTOR_TECHO = leerConstante("modo_inteligente.cpp",
        R"(TECHO_POR_SUELO\s*=\s*(\d+))",
        "el factor del que se deriva el techo a partir del suelo configurado");

    const unsigned long PASO = 500UL;
    const unsigned long TOL  = 3UL * PASO;   // dos bordes de muestreo mas holgura

    // -- E1/E2: el suelo es el CONFIGURADO, y coincide con el Automatico -----------
    //
    // 6 minutos de verde y 5 de rojo: numeros que NO son ninguno de los limites del
    // rango, para que un firmware que se cayera a los minimos -el defecto de A-12- de
    // una cifra distinta y no una que se pueda confundir con la buena.
    const int V_CFG = 6, R_CFG = 5, D_CFG = 10;
    const unsigned long V_CFG_MS = (unsigned long)V_CFG * 60000UL;
    const unsigned long R_CFG_MS = (unsigned long)R_CFG * 60000UL;

    arnes_millis_valor += 10000000UL;
    g_modoEsclavo = ESC_CORRECTO;
    g_latenciaEsclavoMs = 50;

    comprobar(configurarTiempos(V_CFG, R_CFG, D_CFG),
              "E1: SET_TIEMPOS acepta 6 min de verde, 5 de rojo y 10 s de despeje con el "
              "equipo parado -es la misma guarda del firmware, no una puerta del arnes-");

    arrancarInteligente();
    long verdeInt = medirFase([](){ modoInteligente_loop(); }, S_VERDE, PASO,
                              V_CFG_MS * 4UL);
    char m1[220];
    std::snprintf(m1, sizeof(m1),
        "E1: EL MODO INTELIGENTE CORRE CON EL VERDE CONFIGURADO: %ld ms medidos contra "
        "los %lu ms de los 6 minutos que mando el operario. Con el defecto de A-12 aqui "
        "salian %lu ms -VERDE_MIN_MIN- y la app no tenia la culpa",
        verdeInt, V_CFG_MS, (unsigned long)MIN_VERDE_DEFECTO * 60000UL);
    comprobar(verdeInt >= 0 &&
              (unsigned long)verdeInt >= V_CFG_MS - TOL &&
              (unsigned long)verdeInt <= V_CFG_MS + TOL, m1);

    long rojoInt = medirFase([](){ modoInteligente_loop(); }, S_ROJO, PASO,
                             R_CFG_MS * 4UL);
    char m2[200];
    std::snprintf(m2, sizeof(m2),
        "E1b: y el ROJO tambien sale del configurado, no del mismo numero que el verde: "
        "%ld ms contra los %lu ms de los 5 minutos. Antes las dos fases se configuraban "
        "con la MISMA variable (maxVerde) en las tres posiciones", rojoInt, R_CFG_MS);
    comprobar(rojoInt >= 0 &&
              (unsigned long)rojoInt >= R_CFG_MS - TOL &&
              (unsigned long)rojoInt <= R_CFG_MS + TOL, m2);

    // EL CONTROL POSITIVO (§8.sexies). Sin este caso lo de arriba mide una tapia: que
    // una fase dure 6 minutos no dice que el modo degrade bien, dice que el numero
    // llego. Lo que hay que exigir es que el MISMO ciclo, con la MISMA configuracion y
    // la misma regla de medida, de lo mismo en los dos modos.
    arrancarAutomatico();
    long verdeAuto = medirFase([](){ modoAutomatico_loop(); }, S_VERDE, PASO,
                               V_CFG_MS * 4UL);
    long rojoAuto = medirFase([](){ modoAutomatico_loop(); }, S_ROJO, PASO,
                              R_CFG_MS * 4UL);
    char m3[260];
    std::snprintf(m3, sizeof(m3),
        "E2 (CONTROL POSITIVO): CON LAS CAMARAS MUDAS EL INTELIGENTE ES EL AUTOMATICO. "
        "Verde %ld vs %ld ms, rojo %ld vs %ld ms, medidos con la misma regla sobre los "
        "dos .cpp reales. Esta es la propiedad que hace seguro el modo: si la camara "
        "nunca dice 'hay coches', no se alarga nada y se degrada a lo conocido",
        verdeInt, verdeAuto, rojoInt, rojoAuto);
    comprobar(verdeAuto >= 0 && rojoAuto >= 0 &&
              labs(verdeInt - verdeAuto) <= (long)TOL &&
              labs(rojoInt - rojoAuto) <= (long)TOL, m3);

    // -- E3: con trafico propio y NADIE enfrente, alarga hasta el techo ------------
    const unsigned long TECHO_MS = V_CFG_MS * (unsigned long)FACTOR_TECHO;
    arrancarInteligente();
    camaraJ14(true);                   // coches en mi sentido, todo el rato
    long verdeLargo = medirFase([](){ modoInteligente_loop(); }, S_VERDE, PASO,
                                TECHO_MS * 3UL);
    char m4[260];
    std::snprintf(m4, sizeof(m4),
        "E3: con la camara local viendo trafico y NADIE pidiendo paso enfrente, el verde "
        "se ALARGA hasta el techo: %ld ms contra los %lu del suelo y los %lu del techo "
        "(el doble). Esto es lo unico que aportan las camaras, y solo cuando no molesta "
        "a nadie", verdeLargo, V_CFG_MS, TECHO_MS);
    comprobar(verdeLargo >= 0 &&
              (unsigned long)verdeLargo >= TECHO_MS - TOL &&
              (unsigned long)verdeLargo <= TECHO_MS + TOL, m4);

    char m5[240];
    std::snprintf(m5, sizeof(m5),
        "E3b (§3.septies): SUELO Y TECHO SON DOS NUMEROS DISTINTOS -%lu y %lu ms-, asi "
        "que la guarda del techo puede dar las dos respuestas. El 'arreglo de una linea' "
        "-subir el piso a 3 min y dejar maxVerde en 3- los habria igualado y habria "
        "dejado las camaras INERTES en el unico modo que las usa", V_CFG_MS, TECHO_MS);
    comprobar(verdeLargo >= 0 && (unsigned long)verdeLargo > V_CFG_MS + TOL, m5);

    // -- E4: si el OTRO lado pide paso, se cambia en el suelo aunque yo tenga cola ---
    arrancarInteligente();
    camaraJ14(true);
    encolarDemandaRemota();
    long verdeCedido = medirFase([](){ modoInteligente_loop(); }, S_VERDE, PASO,
                                 TECHO_MS * 3UL);
    char m6[240];
    std::snprintf(m6, sizeof(m6),
        "E4: con trafico propio PERO con el otro lado pidiendo paso, el verde termina en "
        "el suelo y no en el techo: %ld ms contra %lu. Alargar cuando hay alguien "
        "esperando enfrente seria monopolizar el carril, y eso no es lo que se decidio",
        verdeCedido, V_CFG_MS);
    comprobar(verdeCedido >= 0 &&
              (unsigned long)verdeCedido >= V_CFG_MS - TOL &&
              (unsigned long)verdeCedido <= V_CFG_MS + TOL, m6);

    // -- E5: NINGUNA camara puede ACORTAR por debajo del suelo ---------------------
    //
    // Las dos gritando desde el primer instante del verde. Antes, esto lo cortaba a los
    // 15 s -`tiempoActual >= 15000UL`-, medio minuto por debajo de los 3 minutos que
    // fijo el responsable el 04/09 (D-5): por debajo del minimo el conductor se convence
    // de que el semaforo esta averiado y adelanta en rojo.
    char m7[260];
    std::snprintf(m7, sizeof(m7),
        "E5 (LA ASIMETRIA QUE PROTEGE): con la camara local y la demanda remota activas "
        "desde el primer instante, el verde NO baja del suelo: %ld ms, y el suelo son "
        "%lu. Una camara puede ALARGAR una fase; ACORTARLA por debajo del minimo vial no "
        "lo puede hacer nadie", verdeCedido, V_CFG_MS);
    comprobar(verdeCedido >= 0 && (unsigned long)verdeCedido >= V_CFG_MS - TOL, m7);

    // -- E6: el techo SATURA al maximo del rango vial ------------------------------
    //
    // 10 minutos configurados: el doble son 20 y el rango llega a 15. Si el techo se
    // saliera, el cruce correria un plazo que la propia guarda de SET_TIEMPOS
    // rechazaria si alguien intentara configurarlo a mano.
    const int V_SAT = 10;
    const unsigned long V_SAT_MS = (unsigned long)V_SAT * 60000UL;
    const unsigned long TECHO_SAT_MS = (unsigned long)VERDE_MAX * 60000UL;
    comprobar((unsigned long)V_SAT * (unsigned long)FACTOR_TECHO > (unsigned long)VERDE_MAX,
              "E6a: con 10 minutos configurados el doble (20) SE SALE del maximo del "
              "rango (15), o sea que este escenario ejerce la saturacion de verdad y no "
              "un caso donde el recorte no haria falta");
    comprobar(configurarTiempos(V_SAT, R_CFG, D_CFG),
              "E6b: SET_TIEMPOS acepta los 10 minutos de verde");
    arrancarInteligente();
    camaraJ14(true);
    long verdeSat = medirFase([](){ modoInteligente_loop(); }, S_VERDE, PASO,
                              V_SAT_MS * (unsigned long)FACTOR_TECHO * 2UL);
    char m8[260];
    std::snprintf(m8, sizeof(m8),
        "E6: EL TECHO SE SATURA AL MAXIMO DEL RANGO: con 10 min configurados y trafico "
        "propio el verde dura %ld ms -los %lu del maximo vial-, no los %lu del doble sin "
        "recortar", verdeSat, TECHO_SAT_MS, V_SAT_MS * (unsigned long)FACTOR_TECHO);
    comprobar(verdeSat >= 0 &&
              (unsigned long)verdeSat >= TECHO_SAT_MS - TOL &&
              (unsigned long)verdeSat <= TECHO_SAT_MS + TOL, m8);

    // -- E7: con la configuracion de fabrica, el suelo son los MINIMOS y no cero -----
    //
    // Un suelo de cero dejaria el cruce alternando sin plazo, y ese camino no se deja
    // abierto aunque hoy no lo recorra nadie.
    //
    // NO SE PUEDE VACIAR EL RESPALDO DESDE AQUI, y se dice en vez de disimularse: el
    // respaldo_guardarTiemposCiclo() real -y su sustituto- ignoran los ceros a
    // proposito, porque un cero no es configuracion sino ausencia de ella. Lo que se
    // ejerce es lo que un equipo de fabrica tiene: los minimos.
    comprobar(configurarTiempos((int)MIN_VERDE_DEFECTO, (int)MIN_ROJO_DEFECTO,
                                (int)SEG_ESTATICO_DEFECTO),
              "E7a: se vuelve a los minimos de fabrica, que es con lo que arranca un "
              "equipo al que nadie ha mandado tiempos");
    arrancarInteligente();
    camaraJ14(true);
    const unsigned long V_MIN_MS = (unsigned long)MIN_VERDE_DEFECTO * 60000UL;
    long verdeMin = medirFase([](){ modoInteligente_loop(); }, S_VERDE, PASO,
                              V_MIN_MS * (unsigned long)FACTOR_TECHO * 3UL);
    char m9[260];
    std::snprintf(m9, sizeof(m9),
        "E7: con la configuracion de fabrica el suelo son los %lu ms del minimo vial "
        "-nunca cero- y el techo su doble: el verde con trafico propio dura %ld ms. Es la "
        "misma pareja de numeros, derivada, no dos constantes que alguien sincroniza",
        V_MIN_MS, verdeMin);
    comprobar(verdeMin >= 0 &&
              (unsigned long)verdeMin >= V_MIN_MS * (unsigned long)FACTOR_TECHO - TOL &&
              (unsigned long)verdeMin <= V_MIN_MS * (unsigned long)FACTOR_TECHO + TOL, m9);

    camaraJ14(false);
    g_demandaRemotaEncolada = false;
  }

  // ===========================================================================
  // BLOQUE F - EL VIGILANTE DE CAMARAS DE J16, EJECUTADO (D-13 fase 1, 05/09)
  // ===========================================================================
  //
  // POR QUE ESTE BLOQUE EXISTE, Y ES LA PARTE QUE HAY QUE LEER:
  //
  // camara_03_vigilante son 675 lineas de Python que miran botones.cpp -2,8 lineas de
  // pack por cada linea de C++ vigilada- y esta en VERDE. No vio ninguno de los dos
  // defectos que este bloque caza, y no por estar mal escrito: mide la FORMA -que el
  // enum tenga cuatro valores, que el getter este en el snprintf, que un umbral sea mayor
  // que el otro-, y los dos defectos son de COMPORTAMIENTO EN EL TIEMPO. El propio pack
  // lo dice en su cabecera: "NO EJERCE EL TIEMPO... eso solo lo demuestra una tarjeta -o
  // un arnes que compile este .cpp-". Este es ese arnes.
  //
  // Y ANTES DE HOY botones.cpp NO SE COMPILABA EN NINGUN SITIO. Validacion_Automatico lo
  // sustituia por once lineas de stub, asi que el vigilante entero -las dos alarmas, la
  // siembra, el contador de vetos- no se habia ejecutado nunca fuera de una tarjeta.
  std::printf("\n-- Bloque F: el vigilante de camaras de J16, ejecutado (D-13) --\n");
  {
    // Los tres plazos se releen del C++ REAL. Si el patron desaparece esto ABORTA en vez
    // de medir contra un numero de ayer (CLAUDE.md 3.bis: sin valor por defecto, nunca).
    long CIEGA_MS = leerConstante("botones.cpp",
        R"(CAM_CIEGA_MS\s*=\s*(\d+)UL)",
        "el plazo de silencio con la pluma arriba que declara CIEGA a una camara");
    long PEGADA_MS = leerConstante("botones.cpp",
        R"(CAM_PEGADA_MS\s*=\s*(\d+)UL)",
        "el plazo de contacto fijo que declara PEGADA a una camara");
    long VENTANA_MS = leerConstante("demanda.cpp",
        R"(SILENCIO_MS\s*=\s*(\d+))",
        "la ventana de silencio entre demandas, que es la vigencia de una deteccion");

    comprobar(CIEGA_MS > PEGADA_MS,
              "F0: CAM_CIEGA_MS > CAM_PEGADA_MS, leidos los dos del C++. Si fuera al reves "
              "un contacto trabado se anunciaria como CIEGA -el diagnostico CONTRARIO- y "
              "el tecnico saldria a buscar un cable cortado teniendo un rele cerrado");

    // Deja el equipo con J16 como diga el escenario y el vigilante recien sembrado. NO se
    // toca el reloj hacia atras: se avanza, que es lo unico que hace un equipo.
    auto reiniciarVigilante = [&](bool camC, bool camD) {
      cerrarContacto(CAM_C_PIN, camC);
      cerrarContacto(CAM_D_PIN, camD);
      camaraJ14(false);
      arnes_millis_valor += 60000UL;
      botones_setup();          // siembra camAnt[] y camAltoDesde[] leyendo los pines
      g_alarmasEmitidas = 0;
      g_eventosEmitidos = 0;
      g_ultimaAlarmaEvento[0] = 0;
      g_ultimaAlarmaCausa[0] = 0;
    };

    // ABRE EL PASO DE VERDAD. El silencio solo corre con la pluma arriba, y "arriba" sale
    // de semaforo_plumaArriba(), que devuelve LO QUE escribirPines() dejo en el pin. Se
    // levanta llamando al semaforo real: fabricar aqui la condicion seria una segunda
    // copia de SFTY-28, que es justo lo que el firmware evita.
    auto abrirPaso = [&]() {
      semaforo_forzarVerde();
      semaforo_actualizar();
    };

    // Corre 'ms' de reloj con la pluma como este, en pasos de 'pasoMs', llamando a
    // botones_actualizar() REAL en cada uno -que es donde vive el vigilante-.
    auto correrConPluma = [&](unsigned long ms, unsigned long pasoMs) {
      unsigned long hecho = 0;
      while (hecho < ms) {
        arnes_millis_valor += pasoMs;
        semaforo_actualizar();
        botones_actualizar();
        hecho += pasoMs;
      }
    };

    // Una deteccion completa: el contacto cierra y vuelve a abrir, que es lo que hace el
    // rele de la AcuSense. El flanco lo toma camaras_actualizar() de verdad.
    auto deteccion = [&](int pin) {
      cerrarContacto(pin, true);
      arnes_millis_valor += 500UL;
      botones_actualizar();
      cerrarContacto(pin, false);
      arnes_millis_valor += 500UL;
      botones_actualizar();
    };

    const unsigned long PASO_F = 60000UL;   // 1 min de reloj por vuelta

    // -- F1: EL PIN VACIO NO ALARMA. Es el defecto 2, y el control que no existia -----
    //
    // HAY UNA CAMARA POR POSTE (D-13), asi que uno de los dos pines de J16 esta vacio en
    // TODOS los equipos que se monten. Antes de hoy ese pin acumulaba silencio como si
    // fuera una camara y, cumplido el plazo, emitia $ALARM CAM_CIEGA de algo que no
    // existe: el manual mandaba al tecnico a mirar una bornera vacia.
    reiniciarVigilante(false, false);
    abrirPaso();
    comprobar(semaforo_plumaArriba(),
              "F1a: la pluma esta ARRIBA -medido con semaforo_plumaArriba() sobre el pin "
              "que escribio semaforo.cpp-, o sea que el cronometro de silencio del "
              "vigilante SI esta corriendo. Sin esta linea, F1 seria una tapia");
    correrConPluma((unsigned long)CIEGA_MS * 2UL, PASO_F);
    char f1[320];
    std::snprintf(f1, sizeof(f1),
        "F1: CON LAS DOS BORNERAS DE J16 VACIAS Y %lu ms de paso abierto -el DOBLE del "
        "plazo de %ld ms-, el vigilante NO emite ni una alarma (emitidas: %d). Un pin sin "
        "camara no ha dado nunca un flanco, y lo que nunca ha visto no puede quedarse "
        "ciego. Antes de este arreglo aqui salia CAM_CIEGA de una camara inexistente",
        (unsigned long)CIEGA_MS * 2UL, CIEGA_MS, g_alarmasEmitidas);
    comprobar(g_alarmasEmitidas == 0, f1);

    char f1b[300];
    std::snprintf(f1b, sizeof(f1b),
        "F1b: y el campo CAM: de ese equipo dice '%s'. Con las dos vacias -o antes de la "
        "primera deteccion- lo unico cierto es el '?', y eso es lo que publica: no se "
        "inventa un OK de una camara que no ha demostrado nada", camara_estado());
    comprobar(std::strcmp(camara_estado(), "?") == 0, f1b);

    // -- F2: LA CAMARA CONECTADA SI LLEGA A CIEGA. El control positivo de 8.sexies ----
    //
    // Sin este caso, F1 estaria midiendo una tapia: un vigilante que no alarmase NUNCA
    // pasaria F1 igual de bien que el correcto. Lo que se exige aqui es que la misma
    // pieza que se callo con el pin vacio SI hable cuando la camara existe y calla.
    reiniciarVigilante(false, false);
    deteccion(CAM_C_PIN);                  // el gesto del instalador (Manual 9)
    comprobar(g_alarmasEmitidas == 0 && std::strcmp(camara_estado(), "OK") == 0,
              "F2a: tras la PRIMERA deteccion -la que el instalador provoca delante de la "
              "camara- el campo CAM: pasa de '?' a 'OK' y no hay alarma. Ese flanco es lo "
              "que ARMA la vigilancia: el vigilante empieza a vigilar cuando el instalador "
              "ha demostrado que la camara ve, y ni un segundo antes");
    abrirPaso();
    correrConPluma((unsigned long)CIEGA_MS + PASO_F * 2UL, PASO_F);
    char f2[340];
    std::snprintf(f2, sizeof(f2),
        "F2 (CONTROL POSITIVO): la camara que SI esta conectada y despues calla %ld ms de "
        "paso abierto acaba anunciada: %d alarma(s), la ultima '%s' con causa '%s', y el "
        "campo CAM: dice '%s'. Sin esta comprobacion, F1 aprobaria un vigilante que no "
        "alarma nunca (CLAUDE.md 8.sexies)",
        CIEGA_MS, g_alarmasEmitidas, g_ultimaAlarmaEvento, g_ultimaAlarmaCausa,
        camara_estado());
    comprobar(g_alarmasEmitidas == 1 &&
              std::strcmp(g_ultimaAlarmaEvento, "CAM_CIEGA") == 0 &&
              std::strcmp(g_ultimaAlarmaCausa, "CAM_C_SIN_FLANCO") == 0 &&
              std::strcmp(camara_estado(), "CIEGA") == 0, f2);

    // -- F3: EL PIN VACIO NO EMPEORA EL CAM: DE LA CAMARA BUENA ----------------------
    //
    // Es la tercera cara del defecto 2: CIEGA pesa mas que el '?' y el '?' mas que OK, asi
    // que un pin vacio que se declarase ciego TAPABA para siempre el estado de la camara
    // que si esta -y con una camara por poste, eso es todos los equipos-.
    deteccion(CAM_C_PIN);                  // pasa un coche: la camara demuestra que ve
    char f3[300];
    std::snprintf(f3, sizeof(f3),
        "F3: con CAM_C recuperada por un flanco y CAM_D VACIA todo el rato, el campo CAM: "
        "dice '%s' y no queda tapado por el pin vacio. La alarma se cierra por su prueba "
        "contraria, y el vecino mudo no opina", camara_estado());
    comprobar(std::strcmp(camara_estado(), "OK") == 0, f3);

    // Y el pin vacio TAMPOCO puede tapar una alarma de verdad: el NIVEL si se juzga
    // siempre, aunque no haya habido flanco nunca. Un contacto trabado desde antes del
    // arranque -el caso que mas tarda en descubrirse solo- sigue saliendo.
    reiniciarVigilante(true, false);       // CAM_C ya cerrada al encender, CAM_D vacia
    abrirPaso();
    correrConPluma((unsigned long)PEGADA_MS + PASO_F * 2UL, PASO_F);
    char f3b[340];
    std::snprintf(f3b, sizeof(f3b),
        "F3b: un contacto CERRADO DESDE ANTES DEL ARRANQUE -que no da flancos, igual que "
        "un pin vacio- sigue anunciandose: %d alarma(s), '%s' causa '%s', CAM: dice '%s'. "
        "Esta es la linea que separa las dos: el SILENCIO no se juzga sin flanco, el NIVEL "
        "si. Si el arreglo de F1 hubiera callado tambien esto, habria cambiado un falso "
        "positivo por un falso negativo",
        g_alarmasEmitidas, g_ultimaAlarmaEvento, g_ultimaAlarmaCausa, camara_estado());
    comprobar(g_alarmasEmitidas >= 1 &&
              std::strcmp(g_ultimaAlarmaEvento, "CAM_PEGADA") == 0 &&
              std::strcmp(camara_estado(), "PEGADA") == 0, f3b);

    // -- F4: UNA DETECCION EN J16 MUEVE EL MODO INTELIGENTE --------------------------
    //
    // LA FRASE DEL ENCARGO ERA FALSA, Y SE DEJA ESCRITA: "el vigilante mira J16 y el modo
    // lee J14, o sea que el modo no recibe demanda de la camara". El modo SI la recibia:
    // camaras_actualizar() llama a demanda_solicitar() en cada flanco de J16 y eso sale
    // por demanda_hayLocal(), que es un termino del mismo OR; y main.cpp llama a
    // botones_actualizar() en todas las vueltas y en todos los modos.
    //
    // LO QUE SI ESTABA ROTO, Y SOLO SE VE CORRIENDOLO: ese camino sirve para PEDIR PASO,
    // no para SOSTENER una fase. La ventana de demanda_hayLocal() dura VENTANA_MS y NO se
    // prolonga con cada deteccion -la peticion que cae dentro se descarta como peticion
    // nueva-, asi que entre dos peticiones aceptadas hay siempre un hueco en el que "hay
    // cola" contesta que no. modo_inteligente.cpp muestrea eso en cada vuelta: basta caer
    // en un hueco para terminar la fase en el suelo. Por eso se anadio el termino de
    // NIVEL, camara_presenciaJ16(). Los escenarios de abajo son la medida.
    {
      long FACTOR = leerConstante("modo_inteligente.cpp",
          R"(TECHO_POR_SUELO\s*=\s*(\d+))",
          "el factor del que se deriva el techo del Modo Inteligente");
      const int V_F = 6, R_F = 5, D_F = 10;
      const unsigned long V_F_MS = (unsigned long)V_F * 60000UL;
      const unsigned long TECHO_F_MS = V_F_MS * (unsigned long)FACTOR;
      const unsigned long PASO_I = 500UL;
      const unsigned long TOL_F = 3UL * PASO_I;

      comprobar((unsigned long)VENTANA_MS < V_F_MS,
                "F4a: la ventana de demanda_hayLocal() leida del C++ es MUCHO mas corta "
                "que la fase que se va a medir, o sea que este escenario ejerce de verdad "
                "el hueco entre peticiones y no un caso donde no cabria");

      // F4c - CONTROL NEGATIVO DEL ESCENARIO: sin ninguna camara, el verde dura el suelo.
      // Sin esto, "el verde llego al techo" no diria nada: podria llegar solo.
      camaraJ14(false);
      cerrarContacto(CAM_C_PIN, false);
      cerrarContacto(CAM_D_PIN, false);
      comprobar(configurarTiempos(V_F, R_F, D_F),
                "F4b: SET_TIEMPOS acepta los tiempos del escenario de J16");
      arrancarInteligente();
      long verdeSinCamara = medirFase([](){ botones_actualizar(); modoInteligente_loop(); },
                                      S_VERDE, PASO_I, TECHO_F_MS * 3UL);
      char f4c[300];
      std::snprintf(f4c, sizeof(f4c),
          "F4c (CONTROL): con las TRES borneras de camara vacias el verde dura %ld ms, que "
          "es el suelo configurado (%lu). Sin este caso, ver el techo en F4 no probaria "
          "que lo movio la camara", verdeSinCamara, V_F_MS);
      comprobar(verdeSinCamara >= 0 &&
                (unsigned long)verdeSinCamara >= V_F_MS - TOL_F &&
                (unsigned long)verdeSinCamara <= V_F_MS + TOL_F, f4c);

      // F4 - LA MEDIDA. Trafico continuo VISTO POR LA CAMARA DE J16 y nadie enfrente.
      // El contacto se cierra y se abre como lo hace el rele de la AcuSense con una cola
      // de vehiculos: detecciones seguidas, ninguna sostenida. J14 sigue VACIA.
      camaraJ14(false);
      cerrarContacto(CAM_D_PIN, false);
      arrancarInteligente();
      long verdeJ16 = medirFase(
          [](){
            // El rele de la camara: cerrado dos vueltas, abierto una. Es el pulso de una
            // cola de vehiculos, no un contacto trabado -eso es CAM_PEGADA y lo mide F3b-.
            static int fase = 0;
            fase = (fase + 1) % 3;
            arnes_pines[CAM_C_PIN] = (fase == 0) ? LOW : HIGH;
            botones_actualizar();
            modoInteligente_loop();
          },
          S_VERDE, PASO_I, TECHO_F_MS * 3UL);
      char f4[360];
      std::snprintf(f4, sizeof(f4),
          "F4: UNA DETECCION EN J16 MUEVE EL MODO INTELIGENTE. Con la camara SOLO en J16 "
          "-J14 vacia- y nadie pidiendo paso enfrente, el verde se alarga hasta %ld ms: el "
          "techo son %lu y el suelo %lu. Con el camino de flanco a secas se quedaba en el "
          "suelo, porque la ventana de %ld ms deja huecos entre peticiones aceptadas",
          verdeJ16, TECHO_F_MS, V_F_MS, VENTANA_MS);
      comprobar(verdeJ16 >= 0 &&
                (unsigned long)verdeJ16 >= TECHO_F_MS - TOL_F &&
                (unsigned long)verdeJ16 <= TECHO_F_MS + TOL_F, f4);

      // Y LO QUE PASA SI LAS DOS DAN SENAL A LA VEZ, que es lo que el encargo pedia medir:
      // NADA DISTINTO. Es un OR, no una suma: el techo lo cierra igual.
      camaraJ14(true);
      arrancarInteligente();
      long verdeAmbas = medirFase(
          [](){
            static int fase2 = 0;
            fase2 = (fase2 + 1) % 3;
            arnes_pines[CAM_C_PIN] = (fase2 == 0) ? LOW : HIGH;
            botones_actualizar();
            modoInteligente_loop();
          },
          S_VERDE, PASO_I, TECHO_F_MS * 3UL);
      char f4d[340];
      std::snprintf(f4d, sizeof(f4d),
          "F4d: CON J14 Y J16 DANDO SENAL A LA VEZ el verde dura %ld ms - EXACTAMENTE lo "
          "mismo que con J16 sola (%ld ms). Las presencias no se suman ni se cuentan: es "
          "un OR, y el techo de la fase lo cierra igual. Dos camaras no pueden monopolizar "
          "el carril mas que una", verdeAmbas, verdeJ16);
      comprobar(verdeAmbas >= 0 && labs(verdeAmbas - verdeJ16) <= (long)TOL_F, f4d);

      camaraJ14(false);
      cerrarContacto(CAM_C_PIN, false);
      cerrarContacto(CAM_D_PIN, false);
      g_demandaRemotaEncolada = false;
    }
  }


  // ===========================================================================
  // BLOQUE G - EL VETO DE LA PLUMA Y SU RETARDO (D-33, 14/09), EJECUTADOS
  // ===========================================================================
  //
  // POR QUE ESTE BLOQUE EXISTE Y NO BASTA CON HABER REPARTIDO LA INVARIANTE.
  //
  // El reparto de vigilarEnclavamiento() dice "la pluma no se queda arriba MAS DE LA
  // CUENTA". Eso lo pasaria igual de bien un firmware que no hubiera construido nada:
  // una pluma que sigue bajando en el mismo instante del rojo cumple la cota trivialmente
  // (CLAUDE.md 9: el escenario nuevo no es relleno, es el control que le falta a toda
  // inversion). Aqui se exige lo CONTRARIO -que el comportamiento nuevo OCURRA- y se
  // miden las dos mitades por separado, porque son dos mecanismos distintos:
  //
  //   EL RETARDO no depende de ninguna camara. Tiene que cumplirse con las borneras
  //   vacias, que es como esta la mayoria de los equipos hoy.
  //   EL VETO depende de la camara, y su direccion de fallo esta DECIDIDA por el
  //   responsable el 14/09: ante error, falsa alarma o contacto pegado, LA BARRERA NO
  //   BAJA. No se acota para que acabe bajando; se AVISA. G4 es el control de que el
  //   fail-safe apunta a donde se dijo, y no al reves.
  std::printf("\n-- Bloque G: el veto de la pluma y su retardo (D-33) --\n");
  {
    const unsigned long PASO_G = 100UL;

    // Los dos plazos, releidos del C++ REAL en este mismo bloque. VENTANA_MS vive en el
    // scope del Bloque F y aqui se vuelve a leer en vez de sacarse fuera: son dos lecturas
    // del MISMO fichero y del MISMO patron, asi que no pueden divergir, y sacar la
    // variable a main() ataria los dos bloques por una variable compartida.
    long VENTANA_G_MS = leerConstante("demanda.cpp",
        R"(SILENCIO_MS\s*=\s*(\d+))",
        "la vigencia de una deteccion, que es lo que hace que el veto se suelte");
    long DESPEJE_MAX_G = leerConstante("limites_ciclo.h",
        R"(DESPEJE_SEG_MAX\s*=\s*(\d+))",
        "el techo del despeje configurable, contra el que se mide un veto sostenido");
    const unsigned long VETO_SOSTENIDO_G_MS = (unsigned long)DESPEJE_MAX_G * 1000UL;

    // Avanza el reloj llamando a los DOS lazos reales. botones_actualizar() tiene que ir
    // en cada vuelta o camAnt[] no se refresca y la presencia se congelaria en el valor
    // de la siembra, que es justo lo contrario de lo que se quiere medir.
    auto correrG = [&](unsigned long ms) {
      unsigned long hecho = 0;
      while (hecho < ms) {
        arnes_millis_valor += PASO_G;
        // EL ORDEN ES EL DE main.cpp, Y NO ES UN DETALLE: alli botones_actualizar() va en
        // la linea 145 y semaforo_actualizar() en la 168, o sea que escribirPines() lee
        // camAnt[] REFRESCADO EN ESTA MISMA VUELTA. Con el orden invertido, el veto
        // trabajaria sobre la lectura de la vuelta anterior y el arnes mediria un desfase
        // de un tick que el equipo real no tiene. Medido: invertido, este arnes acusaba al
        // firmware de un veto sin presencia que solo existia en el arnes.
        botones_actualizar();
        semaforo_actualizar();
        vigilarEnclavamiento();
        hecho += PASO_G;
      }
    };

    // Cuanto tarda la pluma en bajar desde AHORA, o -1 si no baja en 'tope' ms.
    auto msHastaQueBaje = [&](unsigned long tope) -> long {
      unsigned long hecho = 0;
      while (hecho < tope) {
        arnes_millis_valor += PASO_G;
        botones_actualizar();     // el orden de main.cpp; ver correrG()
        semaforo_actualizar();
        vigilarEnclavamiento();
        hecho += PASO_G;
        if (!semaforo_plumaArriba()) return (long)hecho;
      }
      return -1;
    };

    // Deja el equipo con la pluma ARRIBA por un verde de verdad y las camaras como diga
    // el escenario. El verde se pide al semaforo real: fabricar aqui la condicion seria
    // una segunda copia de SFTY-28.
    auto plumaArribaCon = [&](bool camC, bool camD) {
      camaraJ14(false);
      cerrarContacto(CAM_C_PIN, camC);
      cerrarContacto(CAM_D_PIN, camD);
      arnes_millis_valor += 60000UL;
      botones_setup();
      semaforo_forzarVerde();
      semaforo_actualizar();
      correrG(2000UL);
      g_eventosEmitidos = 0;
      g_ultimoEventoDetalle[0] = 0;
      g_alarmasEmitidas = 0;
    };

    const unsigned long RETARDO = g_retardoPlumaMs;
    const unsigned long TOL_G = 3UL * PASO_G;

    comprobar(RETARDO > 0,
              "G0: el retardo de bajada de la pluma se leyo del C++ real y no es cero. "
              "Con un cero, todo lo de abajo mediria el firmware de antes de D-33 y "
              "pasaria sin enterarse");

    // -- G1: SIN NADIE DEBAJO, LA PLUMA BAJA - PERO NO EN EL INSTANTE DEL ROJO -------
    //
    // Las dos mitades de la misma linea: que baje -si no, el equipo se queda sin barrera-
    // y que NO baje antes de tiempo -si no, el retardo no existe-. Medir solo una de las
    // dos deja pasar el firmware contrario.
    plumaArribaCon(false, false);
    comprobar(semaforo_plumaArriba(),
              "G1a: con un verde real la pluma esta ARRIBA -medido sobre el pin que "
              "escribio semaforo.cpp-, o sea que el escenario parte de donde dice");
    semaforo_forzarRojo();
    semaforo_actualizar();
    vigilarEnclavamiento();
    char g1b[320];
    std::snprintf(g1b, sizeof(g1b),
        "G1b: en el INSTANTE del rojo la pluma sigue ARRIBA (pin=%d, ABRIR=%d). Antes de "
        "D-33 bajaba aqui mismo, con el que entro legalmente todavia bajo el barrido",
        arnes_pines[MOTOR_TALANQUERA], TALANQUERA_ABRIR);
    comprobar(semaforo_plumaArriba(), g1b);
    long tBajada = msHastaQueBaje(RETARDO * 4UL);
    char g1c[380];
    std::snprintf(g1c, sizeof(g1c),
        "G1c: y baja sola %ld ms despues del rojo, contra los %lu ms que declara el C++ "
        "real (tolerancia %lu ms por el paso del arnes). Sin camaras no hay veto: lo "
        "unico que la retiene es el retardo, y se suelta solo",
        tBajada, RETARDO, TOL_G);
    comprobar(tBajada > 0 && (unsigned long)tBajada >= RETARDO &&
              (unsigned long)tBajada <= RETARDO + TOL_G, g1c);
    comprobar(g_eventosEmitidos == 0,
              "G1d: y NO se conto ningun veto. Un contador que se disparase con el simple "
              "retardo diria que las camaras estan parando bajadas que nadie paro, y ese "
              "numero es el que decide si el veto merece la pena");

    // -- G2: CON PRESENCIA, LA PLUMA NO BAJA, Y SE DICE ------------------------------
    plumaArribaCon(true, false);
    semaforo_forzarRojo();
    semaforo_actualizar();
    long noBaja = msHastaQueBaje(RETARDO * 6UL);
    char g2[380];
    std::snprintf(g2, sizeof(g2),
        "G2a: con CAM_C viendo presencia, la pluma NO baja en %lu ms -seis veces el "
        "retardo- (msHastaQueBaje=%ld, -1 = no bajo). Es D-33 letra por letra: la camara "
        "es el sensor de presencia y la barrera no se lleva lo que hay debajo",
        RETARDO * 6UL, noBaja);
    comprobar(noBaja == -1 && semaforo_plumaArriba(), g2);
    char g2b[360];
    std::snprintf(g2b, sizeof(g2b),
        "G2b: y el equipo LO DICE: %d evento(s), el ultimo '%s'. El contador dejo de "
        "decir HABRIA -esa transicion ya no ocurre- y cuenta el veto que ACTUO. Sin ese "
        "cambio se habria callado justo en el caso que vino a medir",
        g_eventosEmitidos, g_ultimoEventoDetalle);
    comprobar(g_eventosEmitidos >= 1 &&
              std::strncmp(g_ultimoEventoDetalle, "VETO_ACTUADO_N:", 15) == 0, g2b);
    const int eventosTrasVeto = g_eventosEmitidos;
    correrG(RETARDO * 4UL);
    char g2c[340];
    std::snprintf(g2c, sizeof(g2c),
        "G2c: y NO repite el conteo mientras el mismo veto sigue vivo (%d eventos antes, "
        "%d despues de otros %lu ms). Un veto que dura tres minutos es UN veto: contarlo "
        "por vuelta mediria la velocidad del bucle en vez de los coches",
        eventosTrasVeto, g_eventosEmitidos, RETARDO * 4UL);
    comprobar(g_eventosEmitidos == eventosTrasVeto, g2c);

    // -- G3: Y CUANDO EL DE DEBAJO SE VA, LA PLUMA BAJA ------------------------------
    //
    // El control que le falta a G2: un veto que no supiera SOLTARSE pasaria G2 igual de
    // bien que el correcto, y dejaria la barrera arriba para siempre en cada ciclo.
    cerrarContacto(CAM_C_PIN, false);
    long tSuelta = msHastaQueBaje((unsigned long)VENTANA_G_MS + RETARDO * 4UL);
    char g3[360];
    std::snprintf(g3, sizeof(g3),
        "G3: al abrirse el contacto, la pluma baja %ld ms despues -lo que tarda en caducar "
        "la vigencia del flanco, %ld ms-. El veto se SUELTA solo: sin esta linea, G2 "
        "aprobaria un veto pegado, que es una barrera que no vuelve a bajar nunca",
        tSuelta, VENTANA_G_MS);
    comprobar(tSuelta > 0, g3);

    // -- G4: CAMARA PEGADA - LA BARRERA NO BAJA, Y ESO ES LO CORRECTO ----------------
    //
    // EL SENTIDO DEL FALLO LO DECIDIO EL RESPONSABLE EL 14/09: "si la camara da error,
    // asumo que nunca baja la barrera por error o falsa alarma. Se informa a la app para
    // pedir ajuste de camara, y la barrera nunca baja". Una barrera arriba no aplasta a
    // nadie; el precio es que deja de proteger, y por eso tiene que VERSE. Este caso mide
    // las dos mitades: que NO baja, y que se dice.
    plumaArribaCon(true, false);
    semaforo_forzarRojo();
    semaforo_actualizar();
    long pegada = msHastaQueBaje(VETO_SOSTENIDO_G_MS * 2UL);
    char g4[400];
    std::snprintf(g4, sizeof(g4),
        "G4a: con el contacto PEGADO -cerrado para siempre- la pluma NO baja en %lu ms, y "
        "no se le pone tope que la baje (A-1.bis: un tope que baja igual devuelve el "
        "peligro que el veto evita; este firmware NO distingue un rele trabado de un "
        "vehiculo parado debajo). msHastaQueBaje=%ld, -1 = no bajo",
        VETO_SOSTENIDO_G_MS * 2UL, pegada);
    comprobar(pegada == -1, g4);
    char g4b[420];
    std::snprintf(g4b, sizeof(g4b),
        "G4b: y pasados %lu ms el equipo lo PUBLICA: ultimo evento '%s'. Ese plazo es el "
        "techo del despeje configurable, o sea el instante a partir del cual la otra punta "
        "YA pudo abrir su verde en CUALQUIER configuracion: es cuando la barrera dejo de "
        "hacer su trabajo y hay que verlo. El aviso dice cuantos segundos lleva retenida "
        "-lo unico medido-, nunca 'camara averiada'",
        VETO_SOSTENIDO_G_MS, g_ultimoEventoDetalle);
    comprobar(std::strncmp(g_ultimoEventoDetalle, "VETO_SOSTENIDO_S:", 17) == 0, g4b);

    // -- G5: CAMARA CIEGA - NO VETA, Y ESE ES EL LADO SEGURO PARA EL TRAMO -----------
    //
    // D-25 midio que el vigilante NO detecta una camara muerta desde la instalacion: sin
    // un solo flanco no se la juzga. Con el veto, esa camara tampoco veta NUNCA, asi que
    // la pluma baja como antes de D-33. Es el lado seguro para el tramo y el INSEGURO
    // para quien este debajo, y por eso se mide en vez de suponerse: es el unico caso en
    // que D-33 no protege a nadie, y quien monte el equipo tiene que saberlo.
    plumaArribaCon(false, false);
    semaforo_forzarRojo();
    semaforo_actualizar();
    long ciega = msHastaQueBaje(RETARDO * 4UL);
    char g5[380];
    std::snprintf(g5, sizeof(g5),
        "G5: con las dos borneras de J16 VACIAS -que es lo mismo que una camara muerta "
        "desde la instalacion: ni un flanco nunca- la pluma baja a los %ld ms, igual que "
        "antes de D-33. El veto no puede proteger a quien nadie ve, y eso va escrito en "
        "la spec en vez de disimulado", ciega);
    comprobar(ciega > 0, g5);

    // -- G6: VETA CUALQUIERA DE LAS DOS, SIN CONSENSO --------------------------------
    //
    // Decidido por el responsable el 14/09: "asumo que cualquiera, la camara ES ESE
    // SENSOR". Se mide con la SEGUNDA camara sola, porque el defecto que esto caza -un
    // veto escrito solo sobre CAM_J16[0]- pasaria todos los casos de arriba.
    plumaArribaCon(false, true);
    semaforo_forzarRojo();
    semaforo_actualizar();
    long soloD = msHastaQueBaje(RETARDO * 6UL);
    char g6[360];
    std::snprintf(g6, sizeof(g6),
        "G6: con CAM_D sola -CAM_C vacia- la pluma tampoco baja (msHastaQueBaje=%ld). "
        "Veta CUALQUIERA de las dos: el consenso seria la eleccion peligrosa, porque con "
        "una camara muerta el AND no se cumpliria jamas y el veto no existiria nunca",
        soloD);
    comprobar(soloD == -1, g6);

    // Se deja el equipo limpio: sin contactos cerrados y con la pluma abajo, para que el
    // resumen de invariantes de mas abajo no herede una ventana abierta de este bloque.
    cerrarContacto(CAM_C_PIN, false);
    cerrarContacto(CAM_D_PIN, false);
    correrG((unsigned long)VENTANA_G_MS + RETARDO * 4UL);
  }


  // ===========================================================================
  // LO QUE SE REPARTIO DEL BLOQUE D AL RETIRAR EL MANDO (D-30, 14/09, CLAUDE.md §9)
  // ===========================================================================
  //
  // Estas cuatro comprobaciones vivian DENTRO del fuzz del Bloque D9. Su sujeto nunca
  // fue el mando: son D-33 (la pluma y su retardo) y N-153 (el getter de PLUMA contra
  // el pin). Estaban alli porque el fuzz era el barrido mas largo del arnes, no porque
  // midieran el mando, asi que se MUDAN con su bloque literal al sitio donde viven sus
  // hermanas -el resumen de invariantes de abajo- en vez de irse con el bloque.
  //
  // SIGUEN MIDIENDO SOBRE TODO EL BARRIDO, y eso no es suerte: violacionesTalanquera y
  // discrepanciasPluma son contadores GLOBALES que vigilarEnclavamiento() alimenta en
  // cada tick de cada bloque desde el arranque de main(), y nadie los reinicia. Leerlos
  // aqui -despues del Bloque G- es leerlos sobre mas barrido del que veian antes.
  //
  // LO QUE SI SE PERDIO, ESCRITO SIN DISIMULAR: el fuzz metia 600 pulsos
  // pseudoaleatorios que hacian CAMBIAR DE MODO al equipo una y otra vez -A.A.A
  // relanzaba el Automatico, B.B.B se iba a MODO_AMBAR, A.B.A.B al Degradado-, y esa
  // agitacion de modos era la que mas escenarios distintos le daba a estos cuatro
  // contadores. Con el mando fuera este arnes no tiene ningun otro camino para
  // provocarla: los modos que quedan se entran desde la pantalla o por radio, y ninguno
  // de los dos se compila aqui. O sea que las cuatro propiedades SIGUEN VIGILADAS pero
  // sobre un barrido MENOS AGITADO que el de ayer. No se ha sustituido por un fuzz
  // inventado: eso seria escribir un instrumento nuevo para tapar el hueco en vez de
  // decir que existe.
  comprobar(violacionesTalanquera == 0,
          "en NINGUN instante del barrido la talanquera estuvo ARRIBA sin una razon "
          "nombrada (SFTY-28 con la derogacion parcial de D-33, medido sobre el pin que "
          "semaforo.cpp real escribio). Las razones son cuatro y NO se admiten por su "
          "nombre: el verde; el ambar intermitente de SFTY-6, por decision del cliente; "
          "el retardo de bajada de D-33, CRONOMETRADO contra su constante; y el veto de "
          "la camara, CRUZADO contra camara_presenciaJ16(). El rojo, el ambar de "
          "transicion y el todo-rojo la dejan abajo igual que antes, solo que unos "
          "segundos despues");
  {
  // Control negativo del vigilante de la pluma: se falsea el pin a mano y se exige
  // que el detector lo cace. Sin esto, el dia que MOTOR_TALANQUERA dejara de
  // escribirse -o el arnes dejara de conocer el pin- la comprobacion de arriba
  // seguiria en verde midiendo un pin que nadie toca.
  long antes = violacionesTalanquera;
  // N-153: y tambien el contador de PLUMA. Falsear el pin a mano dispara los DOS
  // vigilantes -el getter sigue diciendo lo que escribio el firmware, que es
  // justamente lo que el otro invariante mide-, y dejarlo contado convertiria este
  // control negativo en un fallo del vigilante de al lado. Medido: sin esta linea el
  // arnes cae a 72/73 acusando a un firmware sano.
  long antesPluma = discrepanciasPluma;
  int guardaP = arnes_pines[MOTOR_TALANQUERA];
  int guardaV1 = arnes_pines[VERDE1], guardaV2 = arnes_pines[VERDE2];
  // D-33: Y LA VENTANA SE FALSEA VIEJA, que es la mitad nueva de este control. Con el
  // reparto de la invariante, una pluma arriba sin verde RECIEN abierta esta justificada
  // por el retardo; si este control se limitara a falsear el pin, dejaria de disparar y
  // se habria convertido en un adorno que da verde el mismo dia del cambio. Se le
  // envejece la ventana a proposito para ejercer el camino que SI tiene que contar.
  unsigned long guardaVent = g_plumaSinVerdeDesde;
  unsigned long guardaPeor = g_peorVentanaPlumaMs;
  unsigned long guardaPeorSV = g_peorVentanaSinVetoMs;
  unsigned long guardaNvent = g_ventanasPluma;
  arnes_pines[MOTOR_TALANQUERA] = TALANQUERA_ABRIR;
  arnes_pines[VERDE1] = LOW; arnes_pines[VERDE2] = LOW;
  g_plumaSinVerdeDesde = arnes_millis_valor - (g_retardoPlumaMs * 2UL) + 1;
  vigilarEnclavamiento();
  bool detecta = (violacionesTalanquera == antes + 1);
  arnes_pines[MOTOR_TALANQUERA] = guardaP;
  arnes_pines[VERDE1] = guardaV1; arnes_pines[VERDE2] = guardaV2;
  violacionesTalanquera = antes;
  discrepanciasPluma = antesPluma;
  g_plumaSinVerdeDesde = guardaVent;
  g_peorVentanaPlumaMs = guardaPeor;
  g_peorVentanaSinVetoMs = guardaPeorSV;
  g_ventanasPluma = guardaNvent;
  comprobar(detecta,
            "control negativo: el vigilante de la pluma SI cuenta una violacion "
            "cuando la talanquera esta arriba con los dos verdes apagados Y la ventana "
            "de bajada ya paso del retardo de D-33 -o sea, sin ninguna de las razones "
            "que la invariante repartida admite-");
  }
  comprobar(discrepanciasPluma == 0,
          "en NINGUN instante del barrido semaforo_plumaArriba() dijo algo distinto "
          "de lo que habia en el pin (N-153: es el valor que viaja en PLUMA del "
          "$STATUS y con el que la app dibuja la barrera; un getter desincronizado "
          "no rompe ninguna luz y ningun pack de texto podria verlo)");
  {
  // Control negativo del vigilante de arriba, por lo mismo que el de la talanquera:
  // una comprobacion que nadie ha visto fallar es un adorno que da verde. Se falsea
  // el PIN -no el getter, que es codigo real- y se exige que la discrepancia salte.
  long antesD = discrepanciasPluma;
  long antesT = violacionesTalanquera;
  int guardaP = arnes_pines[MOTOR_TALANQUERA];
  // D-33: falsear el pin puede ABRIR una ventana de bajada que no existio. Se guarda y
  // se restaura igual que los contadores, por el mismo motivo que ya estaba escrito
  // abajo: dejarla contada convertiria este control en un fallo del de al lado.
  unsigned long guardaVent = g_plumaSinVerdeDesde;
  unsigned long guardaPeor = g_peorVentanaPlumaMs;
  unsigned long guardaPeorSV = g_peorVentanaSinVetoMs;
  unsigned long guardaNvent = g_ventanasPluma;
  arnes_pines[MOTOR_TALANQUERA] =
      semaforo_plumaArriba() ? TALANQUERA_CERRAR : TALANQUERA_ABRIR;
  vigilarEnclavamiento();
  bool cazado = (discrepanciasPluma == antesD + 1);
  arnes_pines[MOTOR_TALANQUERA] = guardaP;
  g_plumaSinVerdeDesde = guardaVent;
  g_peorVentanaPlumaMs = guardaPeor;
  g_peorVentanaSinVetoMs = guardaPeorSV;
  g_ventanasPluma = guardaNvent;
  // Los dos contadores se restauran: falsear el pin puede disparar tambien el
  // invariante de SFTY-28, y dejarlo contado convertiria este control en un fallo
  // del otro.
  discrepanciasPluma = antesD;
  violacionesTalanquera = antesT;
  comprobar(cazado,
            "control negativo: el vigilante de PLUMA SI cuenta una discrepancia "
            "cuando el pin dice lo contrario que el getter");
  }

  // ===========================================================================
  comprobar(violacionesEnclavamiento == 0,
            "en NINGUN instante de todo el barrido -los siete bloques que quedan, A a G- coincidieron "
            "ROJO y VERDE encendidos a la vez en la misma cara (SFTY-2, medido sobre "
            "lo que semaforo.cpp real escribio en los pines, no sobre la logica)");
  {
    char rp[460];
    std::snprintf(rp, sizeof(rp),
        "RESUMEN (SFTY-28 con la derogacion parcial de D-33): en ninguno de los instantes "
        "del barrido hubo pluma arriba sin verde y fuera de S_FALLO MAS ALLA de las dos "
        "razones nuevas. Se abrieron %lu ventanas de bajada; la mas larga duro %lu ms "
        "-esa la sostenia el veto de una camara- y la mas larga CON EL VETO SUELTO duro "
        "%lu ms, contra los %lu ms de retardo mas %lu ms de margen -el paso de muestreo "
        "de este arnes, que es lo unico que puede estirar la ventana desde que el mando "
        "salio: ver MARGEN_MUESTREO_MS-: la excepcion del retardo esta ACOTADA y MEDIDA, "
        "no concedida por su nombre",
        g_ventanasPluma, g_peorVentanaPlumaMs, g_peorVentanaSinVetoMs, g_retardoPlumaMs,
        g_margenSenalMs);
    comprobar(violacionesTalanquera == 0, rp);
  }
  comprobar(g_ventanasPluma > 0,
            "CONTROL de D-33: esas ventanas EXISTIERON. Si fueran cero, la pluma seguiria "
            "bajando en el mismo instante del rojo y el reparto de la invariante habria "
            "cambiado una comprobacion por una tapia (CLAUDE.md 9)");
  comprobar(vetoSinPresencia == 0,
            "RESUMEN (D-33): en NINGUN instante semaforo_plumaVetada() dijo que si con "
            "camara_presenciaJ16() diciendo que no. Un veto pegado dejaria la barrera "
            "arriba para siempre sin mover una sola luz, o sea sin que ninguna otra "
            "comprobacion de este arnes pudiera verlo");
  std::printf("\n==============================================================\n");
  std::printf(" RESULTADO: %d/%d comprobaciones OK\n", total - fallos, total);
  std::printf("==============================================================\n");
  std::printf(" Medido sobre coordinador.cpp + semaforo.cpp + modo_automatico.cpp +\n");
  std::printf(" botones.cpp (D-13) + modo_inteligente.cpp y demanda.cpp (A-12) REALES,\n");
  std::printf(" compilados para el PC. %lu redibujos de\n",
              g_lcdRedibujos);
  std::printf(" pantalla observados, %lu escrituras de pin observadas.\n",
              arnes_escrituras);
  std::printf(" Ningun paso de este arnes reimplementa el ciclo: lo que se mide es el\n");
  std::printf(" binario, no un modelo de el.\n");
  return fallos == 0 ? 0 : 1;
}
