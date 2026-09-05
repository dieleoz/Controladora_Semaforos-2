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
// N-52: SE SUMA mando.cpp REAL (Bloque D). senalActiva -el static de semaforo.cpp
// que congela escribirPines() mientras dura una senal del mando- SOLO lo pone
// mando.cpp; hasta ahora ese fichero no se compilaba aqui y esa rama de
// aplicarSalidas() jamas se ejercia sobre el binario real. El Bloque D pulsa las
// tres secuencias (A.A.A, B.B.B, A.B.A.B) tal como llegan del rele -en paralelo
// con Boton1/Boton2, ver botones.cpp- y mide sobre los PINES, no sobre
// senalActiva ni sobre ninguna variable interna: exactamente el mismo criterio
// que los bloques A-C.

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
#include "mando.h"           // N-52: real, solo necesita Arduino.h
#include "modo_ambar.h"      // N-52: real, idem -- modo_ambar.cpp NO se compila
#include "modo_degradado.h"  // N-52: real, idem -- modo_degradado.cpp NO se compila
#include "modo_inteligente.h" // A-12: real, y su .cpp SI se compila (Bloque E)
#include "demanda.h"          // A-12: real, y su .cpp tambien -- entra en el OR

// ---------------------------------------------------------------------------
// EL RELOJ SIMULADO Y LOS PINES OBSERVADOS. Los declara extern Arduino.h; existen
// aqui porque el arnes es quien los mueve.
// ---------------------------------------------------------------------------
unsigned long arnes_millis_valor = 0;
int arnes_pines[64];
unsigned long arnes_escrituras = 0;

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
    violacionesTalanquera++;
  }
  if (semaforo_plumaArriba() != (arnes_pines[MOTOR_TALANQUERA] == TALANQUERA_ABRIR)) {
    discrepanciasPluma++;
  }
}

// ---------------------------------------------------------------------------
// N-52 — VIGILANTE DE LA SENAL. Es la comprobacion que decide la hipotesis del
// encargo: mide, en CADA tick de CUALQUIER bloque (se engancha en
// bombearGenerico igual que vigilarEnclavamiento), cuanto lleva encendida
// semaforo_senalEnCurso() sin que nadie la baje. No sabe nada de mando.cpp ni de
// que secuencia la disparo -solo mira el reloj y ese booleano publico-, asi que
// puede cazar un camino de "senalActiva pegada" que nadie escribio a proposito
// en los bloques de abajo. El fuzz del Bloque D8 es quien mas la ejercita.
//
// PRESUPUESTO_SENAL_MS se fija en main(), leido del C++ real (ver mas abajo):
// nunca un numero escrito a mano.
// ---------------------------------------------------------------------------
static bool g_senalEnCursoAnt = false;
static unsigned long g_tInicioSenalObservado = 0;
static long g_peorDuracionSenalMs = -1;
static bool g_senalExcedioPresupuesto = false;
static unsigned long PRESUPUESTO_SENAL_MS = 0;

static void vigilarSenal() {
  bool enCurso = semaforo_senalEnCurso();
  if (enCurso && !g_senalEnCursoAnt) {
    g_tInicioSenalObservado = arnes_millis_valor;
  }
  if (enCurso) {
    unsigned long transcurrido = arnes_millis_valor - g_tInicioSenalObservado;
    if (transcurrido > PRESUPUESTO_SENAL_MS) g_senalExcedioPresupuesto = true;
  } else if (g_senalEnCursoAnt) {
    long duracion = (long)(arnes_millis_valor - g_tInicioSenalObservado);
    if (duracion > g_peorDuracionSenalMs) g_peorDuracionSenalMs = duracion;
  }
  g_senalEnCursoAnt = enCurso;
}

// Compara los PINES (lo que semaforo.cpp REALMENTE escribio) contra lo que
// semaforo_estado() dice que deberia haber, para las tres luces estables. En
// S_FALLO no hay una combinacion fija -parpadea-, asi que ese caso no se
// contradice aqui: lo mide el Bloque A2 con su propio reloj.
static bool pinesCoincidenConEstado() {
  EstadoSemaforo e = semaforo_estado();
  if (e == S_FALLO) return true;
  bool rojoEsperado = (e == S_ROJO);
  bool amarilloEsperado = (e == S_AMARILLO);
  bool verdeEsperado = (e == S_VERDE);
  return arnes_pines[ROJO1] == (rojoEsperado ? HIGH : LOW) &&
         arnes_pines[ROJO2] == (rojoEsperado ? HIGH : LOW) &&
         arnes_pines[AMARILLO1] == (amarilloEsperado ? HIGH : LOW) &&
         arnes_pines[AMARILLO2] == (amarilloEsperado ? HIGH : LOW) &&
         arnes_pines[VERDE1] == (verdeEsperado ? HIGH : LOW) &&
         arnes_pines[VERDE2] == (verdeEsperado ? HIGH : LOW);
}

// ---------------------------------------------------------------------------
// BOTONES SIMULADOS. Cada bool se consume solo (como el real: leerlo lo gasta).
// ---------------------------------------------------------------------------
static bool g_pulsarArriba = false, g_pulsarAbajo = false;
static bool g_pulsarAceptar = false, g_pulsarCancelar = false;

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
void bluetooth_reportarEvento(const char*, const char*) {}

void botones_setup() {}
void botones_actualizar() {}
bool botonArriba()   { bool v = g_pulsarArriba;   g_pulsarArriba = false;   return v; }
bool botonAbajo()    { bool v = g_pulsarAbajo;    g_pulsarAbajo = false;    return v; }
bool botonAceptar()  { bool v = g_pulsarAceptar;  g_pulsarAceptar = false;  return v; }
bool botonCancelar() { bool v = g_pulsarCancelar; g_pulsarCancelar = false; return v; }

// Simula al operario pulsando ACEPTAR una vez y deja que el modo lo procese. Se usa
// para atravesar el asistente (CONFIG_ROJO -> CONFIG_VERDE -> CONFIG_ESTATICO ->
// CORRIENDO) con los valores por defecto, exactamente como haria alguien que solo
// confirma tres veces sin tocar nada.
static void pulsarAceptar() {
  g_pulsarAceptar = true;
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
// A-12: LA CAMARA SIMULADA. Es lo unico del Bloque E que el arnes mueve.
//
// camara_leerPin() real vive en botones.cpp -antirrebote de 1 ms sobre el pin- y
// ese fichero no se compila aqui: lo que este arnes mide no es el antirrebote sino
// QUE PUEDE HACER una deteccion con el ciclo en marcha. La respuesta que se exige
// es asimetrica y por eso hay que ejercerla: puede ALARGAR una fase y no puede
// ACORTARLA. Un unico bool, movido por el escenario, es exactamente la superficie
// que el modo ve.
// ---------------------------------------------------------------------------
static bool g_camaraLocal = false;

bool camara_leerPin(uint8_t pin) {
  (void)pin;
  return g_camaraLocal;
}

// ---------------------------------------------------------------------------
// N-52: modoActual_get()/set() PASAN A SER DE VERDAD (antes set() era un no-op y
// get() ni se declaraba). mando.cpp los necesita: secuenciasInhibidas() lee el
// modo para bloquear las secuencias en MENU/MODO_HORA, y ejecutar() lee y escribe
// el modo para decidir si "ya estabamos aqui" o si hay que cambiar. Sin guardar
// el valor de verdad, ejecutar() siempre habria credo estar en MENU (el 0 del
// enum) y jamas habria tomado la rama "modoActual_set(...)": el arnes mediria un
// mando que nunca cambia de modo.
//
// Arranca en MENU, igual que el enum real: es el valor con el que main.cpp llega
// al primer loop() antes de que nadie pulse nada.
// ---------------------------------------------------------------------------
static ModoSistema g_modoActual = MENU;
ModoSistema modoActual_get() { return g_modoActual; }
void modoActual_set(ModoSistema m) { g_modoActual = m; }

// ---------------------------------------------------------------------------
// MODO_AMBAR / MODO_DEGRADADO: FUERA DE ALCANCE DE ESTE ARNES, igual que
// reloj_*/respaldo_* de mas abajo -sus .cpp no se compilan aqui-. Se stubean SOLO
// las funciones que mando.cpp llama de verdad (ver mando.cpp: ejecutar() y
// mando_registrarPulso()); modo_ambar_loop(), modo_degradado_loop() y el resto de
// la API de esos modos no hacen falta porque nadie de lo que aqui se compila los
// llama.
//
// modo_degradado_evaluarEntrada() es LA PUERTA que decide si A.B.A.B se acepta o
// se rechaza (ver mando.cpp linea ~213). El arnes controla su respuesta con
// g_entradaDegradado para poder ejercer LAS DOS ramas -aceptado y rechazado- sin
// depender de reloj_enHora() ni de ninguna otra condicion que este arnes no mueve.
// ---------------------------------------------------------------------------
static MotivoDegradado g_entradaDegradado = MDG_OK;
static unsigned long g_modoAmbarSetups = 0;
static unsigned long g_modoDegradadoSetups = 0;

void modo_ambar_setup() { g_modoAmbarSetups++; }
void modo_ambar_fijarMotivo(const char*, const char*) {}

MotivoDegradado modo_degradado_evaluarEntrada() { return g_entradaDegradado; }
void modo_degradado_setup() { g_modoDegradadoSetups++; }

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
    vigilarSenal();
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
  mando_setup();                   // N-52: limpia secBoton/pendiente del mando
  g_modoActual = MODO_AUTOMATICO;  // lo que main.cpp ya habria fijado al entrar
  // N-52: limpia flancos de boton que hubieran quedado sin consumir -un pulso de
  // A/B del Bloque D que CORRIENDO no llego a leer, por ejemplo-. botones_setup()
  // real arranca igual de limpio; que este arnes simule los cuatro botones con
  // bools sueltos no debe dejar arrastre de un bloque de prueba al siguiente.
  g_pulsarArriba = g_pulsarAbajo = g_pulsarAceptar = g_pulsarCancelar = false;
  modoAutomatico_setup();          // fase = CONFIG_ROJO
  pulsarAceptar();                 // -> CONFIG_VERDE
  pulsarAceptar();                 // -> CONFIG_ESTATICO
  pulsarAceptar();                 // -> CORRIENDO: coordinador_iniciarModo()
}

// ---------------------------------------------------------------------------
// N-52 — UN TICK DE main.cpp, EN EL ORDEN REAL.
//
// Los Bloques A-C avanzan el ciclo llamando solo a modoAutomatico_loop(): basta
// para medir el ciclo en soledad. mando.cpp no vive ahi: se alimenta desde
// botones_actualizar() (ANTES de despachar al modo, src/main.cpp linea 137) y se
// resuelve desde mando_actualizar() (AL FINAL de loop(), linea 221). Sin las dos
// llamadas en el sitio que les corresponde, una senal jamas podria empezar ni
// terminar en este arnes -no seria un defecto del firmware, seria un arnes que
// no recorre el camino que se le pidio medir-.
//
// pulsarA/pulsarB simulan el flanco de botones_actualizar(): el rele del mando
// esta cableado EN PARALELO con Boton1/Boton2 (mismo contacto, ver
// src/botones.cpp linea 33-34), asi que un pulso real dispara los dos caminos a
// la vez -mando_registrarPulso(), que ve todos los modos, y el flag de boton que
// cada modo consulta por su cuenta-. Pasar solo el primero seria medir un mando
// que no existe: el fisico jamas pulsa "solo para el mando".
//
// Los modos que no se compilan aqui (MENU, MODO_MANUAL, MODO_AMBAR,
// MODO_DEGRADADO...) no tienen loop() que llamar: igual que main.cpp
// despacharia al loop que corresponda y este arnes no lo tiene, este paso
// sencillamente no llama a ninguno cuando el modo cambia. semaforo_actualizar()
// y mando_actualizar() SIGUEN corriendo pase lo que pase, que es de lo unico que
// depende que una senal en curso se resuelva sola.
// ---------------------------------------------------------------------------
static void pasoPrincipal(bool pulsarA = false, bool pulsarB = false) {
  if (pulsarA) mando_registrarPulso(MANDO_A);
  if (pulsarB) mando_registrarPulso(MANDO_B);
  if (pulsarA) g_pulsarArriba = true;
  if (pulsarB) g_pulsarAbajo = true;

  semaforo_actualizar();
  if (modoActual_get() == MODO_AUTOMATICO) {
    modoAutomatico_loop();
  }
  mando_actualizar();
}

// Avanza 'ms' en pasos de 'pasoMs', vigilando enclavamiento y senal en cada uno.
static void avanzar(unsigned long ms, unsigned long pasoMs) {
  unsigned long hecho = 0;
  while (hecho < ms) {
    pasoPrincipal();
    vigilarEnclavamiento();
    vigilarSenal();
    arnes_millis_valor += pasoMs;
    hecho += pasoMs;
  }
}

// Un pulso del mando (A o B), tal como lo veria botones_actualizar() en el tick
// en que ocurre, seguido de 'gapMs' de espera -el tiempo real que tarda el rele
// en conmutar y el operario en pulsar otra vez (12-18 s de ventana, ver
// mando.cpp)-.
static void pulsarYAvanzar(PulsoMando p, unsigned long gapMs, unsigned long pasoMs) {
  pasoPrincipal(p == MANDO_A, p == MANDO_B);
  vigilarEnclavamiento();
  vigilarSenal();
  arnes_millis_valor += pasoMs;
  unsigned long hecho = pasoMs;
  while (hecho < gapMs) {
    pasoPrincipal();
    vigilarEnclavamiento();
    vigilarSenal();
    arnes_millis_valor += pasoMs;
    hecho += pasoMs;
  }
}

// Igual que bombear(), pero con pasoPrincipal() -semaforo_actualizar() +
// modoAutomatico_loop() (si toca) + mando_actualizar()- como paso. bombear() se
// queda para los Bloques A-C, que no tocan el mando: usarlo en el Bloque D
// dejaria a mando_actualizar() sin quien lo llame y ninguna senal terminaria
// nunca, por un motivo ajeno al firmware.
template <typename Cond>
static long bombearPrincipal(unsigned long pasoMs, unsigned long presupuestoMs, Cond condicion) {
  return bombearGenerico(pasoMs, presupuestoMs, [](){ pasoPrincipal(); }, condicion);
}

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
  mando_setup();
  g_camaraLocal = false;
  g_demandaRemotaEncolada = false;
  coordinador_limpiarDemandaRemota();
  g_pulsarArriba = g_pulsarAbajo = g_pulsarAceptar = g_pulsarCancelar = false;
  g_modoActual = MODO_INTELIGENTE;   // lo que main.cpp habria fijado al entrar
  modoInteligente_setup();
}

static void arrancarAutomatico() {
  coordinador_setup();
  mando_setup();
  g_pulsarArriba = g_pulsarAbajo = g_pulsarAceptar = g_pulsarCancelar = false;
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
  std::printf(" modo_automatico.cpp + mando.cpp + modo_inteligente.cpp REALES,\n");
  std::printf(" compilados y ejecutados en el PC (Bloque D / N-52: el mando de\n");
  std::printf(" reles; Bloque E / A-12: el Modo Inteligente contra el Automatico)\n");
  std::printf("==============================================================\n");

  // -------------------------------------------------------------------------
  // Constantes releidas del C++ real. Ni una se escribe a mano: si el patron no
  // aparece, el arnes ABORTA antes de comprobar nada (ver leerConstante()).
  // -------------------------------------------------------------------------
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

  // -------------------------------------------------------------------------
  // N-52: constantes de la senal del mando, releidas de semaforo.cpp y de
  // mando.cpp -el mismo criterio de arriba, sin valor por defecto nunca-.
  // -------------------------------------------------------------------------
  long DESTELLO_ON_MS = leerConstante("semaforo.cpp",
      R"(DESTELLO_ON_MS\s*=\s*(\d+))",
      "la duracion ON de un destello del mando");
  long DESTELLO_OFF_MS = leerConstante("semaforo.cpp",
      R"(DESTELLO_OFF_MS\s*=\s*(\d+))",
      "la duracion OFF de un destello del mando");

  long DESTELLOS_AUTOMATICO = leerConstante("mando.cpp",
      R"(DESTELLOS_AUTOMATICO\s*=\s*(\d+))",
      "los destellos de confirmacion de A.A.A");
  long DESTELLOS_AMBAR = leerConstante("mando.cpp",
      R"(DESTELLOS_AMBAR\s*=\s*(\d+))",
      "los destellos de confirmacion de B.B.B");
  long DESTELLOS_DEGRADADO = leerConstante("mando.cpp",
      R"(DESTELLOS_DEGRADADO\s*=\s*(\d+))",
      "los destellos de confirmacion de A.B.A.B");
  long VENTANA_TRIPLE_MS = leerConstante("mando.cpp",
      R"(VENTANA_TRIPLE_MS\s*=\s*(\d+))",
      "la ventana de A.A.A / B.B.B");
  long VENTANA_CUADRUPLE_MS = leerConstante("mando.cpp",
      R"(VENTANA_CUADRUPLE_MS\s*=\s*(\d+))",
      "la ventana de A.B.A.B");
  long RECHAZO_AMBAR_MS = leerConstante("mando.cpp",
      R"(RECHAZO_AMBAR_MS\s*=\s*(\d+))",
      "la duracion del ambar de rechazo del mando");
  long AMBAR_RAPIDO_PERIODO_MS = leerConstante("semaforo.cpp",
      R"(AMBAR_RAPIDO_PERIODO_MS\s*=\s*(\d+))",
      "el periodo del ambar de rechazo del mando");

  std::printf("   destello ON/OFF (mando) ........ %ld/%ld ms\n", DESTELLO_ON_MS, DESTELLO_OFF_MS);
  std::printf("   destellos A.A.A / B.B.B / A.B.A.B  %ld / %ld / %ld\n",
              DESTELLOS_AUTOMATICO, DESTELLOS_AMBAR, DESTELLOS_DEGRADADO);
  std::printf("   ventana triple / cuadruple ..... %ld / %ld ms\n",
              VENTANA_TRIPLE_MS, VENTANA_CUADRUPLE_MS);
  std::printf("   ambar de rechazo ................ %ld ms\n", RECHAZO_AMBAR_MS);

  // Duracion TEORICA maxima de cualquier senal del mando: la mas larga de las
  // cuatro (los destellos de A.B.A.B, que son los que mas tardan) o el ambar de
  // rechazo, lo que sea mayor. PRESUPUESTO_SENAL_MS le suma un margen generoso
  // -dos pasos de bombeo mas 1000 ms- para que la granularidad del barrido nunca
  // dispare un falso positivo: el vigilante existe para cazar una senal
  // REALMENTE atascada, no un redondeo del propio arnes.
  long duracionDestellos = DESTELLOS_DEGRADADO * (DESTELLO_ON_MS + DESTELLO_OFF_MS);
  long duracionMaximaTeorica = (duracionDestellos > RECHAZO_AMBAR_MS)
                                    ? duracionDestellos : RECHAZO_AMBAR_MS;
  PRESUPUESTO_SENAL_MS = (unsigned long)duracionMaximaTeorica + 1000UL;
  std::printf("   presupuesto del vigilante de senal  %lu ms\n", PRESUPUESTO_SENAL_MS);

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

    long msHastaAmarillo = bombearGenerico(1, 200,
        [](){ coordinador_actualizar(); },
        [](){ return semaforo_estado() == S_AMARILLO; });
    comprobar(msHastaAmarillo >= 0 && msHastaAmarillo <= 5,
              "CONTROL NEGATIVO: con despeje=0 el todo-rojo se salta casi al "
              "instante, pero coordinador_actualizar() SIGUE llamando a "
              "semaforo_iniciarTransicionAVerde(): no hay un atajo que se salte el "
              "aviso de amarillo por tener el despeje a cero");

    long msHastaVerde = bombearGenerico(1, (unsigned long)AMBAR_MS + 500UL,
        [](){ coordinador_actualizar(); },
        [](){ return semaforo_estado() == S_VERDE; });
    comprobar(msHastaVerde >= (long)AMBAR_MS - 2 && msHastaVerde <= (long)AMBAR_MS + 2,
              "CONTROL NEGATIVO: aun con la configuracion imposible, el amarillo "
              "dura EXACTAMENTE lo que el C++ real dice que dura -SFTY-5 no depende "
              "de que la UI haya validado el despeje-");
  }

  // ===========================================================================
  // BLOQUE D: EL MANDO DE RELES (SFTY-21) — mando.cpp REAL, sobre los PINES
  // ===========================================================================
  //
  // N-52: hasta este bloque, senalActiva -el static de semaforo.cpp que congela
  // escribirPines() mientras dura una senal- nunca se habia puesto a true en este
  // arnes: mando.cpp no se compilaba, y la rama "if (senalActiva) return;" de
  // aplicarSalidas() jamas se ejercia sobre el binario real. Cada comprobacion de
  // aqui abajo mide PINES (arnes_pines[]) y el reloj (vigilarSenal(), que corre en
  // TODOS los bombeos desde el arranque de este main()), nunca senalActiva
  // directamente: el mismo criterio que los Bloques A-C.
  std::printf("\n-- Bloque D: el mando de reles (SFTY-21), sobre coordinador+semaforo+mando REALES --\n");

  long duracionAAA_MS = DESTELLOS_AUTOMATICO * (DESTELLO_ON_MS + DESTELLO_OFF_MS);
  long duracionBBB_MS = DESTELLOS_AMBAR * (DESTELLO_ON_MS + DESTELLO_OFF_MS);
  long duracionABAB_MS = DESTELLOS_DEGRADADO * (DESTELLO_ON_MS + DESTELLO_OFF_MS);
  const long TOLERANCIA_MS = 150;  // 3 pasos de bombeo (50ms): granularidad, no gracia

  // ---- D1: A.A.A DURANTE CORRIENDO, con el Maestro en VERDE al 3er pulso -----
  {
    arnes_millis_valor += 10000000UL;
    g_modoEsclavo = ESC_CORRECTO;
    g_latenciaEsclavoMs = 50;

    arrancarAutomaticoPorDefecto();
    bombear(200, SEG_ESTATICO_MS + (unsigned long)AMBAR_MS + 5000UL,
        [](){ return semaforo_estado() == S_VERDE; });
    comprobar(semaforo_estado() == S_VERDE && arnes_pines[VERDE1] == HIGH,
              "D1: arranca en VERDE de verdad -pin en HIGH- antes de meter el mando "
              "por medio");

    pulsarYAvanzar(MANDO_A, 2000, 100);
    pulsarYAvanzar(MANDO_A, 2000, 100);
    pulsarYAvanzar(MANDO_A, 0, 100);   // el 3er pulso confirma A.A.A
    comprobar(semaforo_senalEnCurso(),
              "D1: al 3er pulso de A.A.A la senal arranca DE INMEDIATO (SFTY-21)");
    comprobar(arnes_pines[VERDE1] == LOW && arnes_pines[VERDE2] == LOW,
              "D1: el hueco inicial de la senal apaga el VERDE que hubiera antes -la "
              "senal INTERCEPTA, no espera a que la logica suelte el verde por su "
              "cuenta-");

    bool verdeSeVioDuranteSenal = false;
    long msHastaFinSenal = bombearGenerico(50UL, PRESUPUESTO_SENAL_MS,
        [](){ pasoPrincipal(); },
        [&verdeSeVioDuranteSenal](){
          if (arnes_pines[VERDE1] == HIGH || arnes_pines[VERDE2] == HIGH) {
            verdeSeVioDuranteSenal = true;
          }
          return !semaforo_senalEnCurso();
        });
    comprobar(msHastaFinSenal >= 0,
              "D1: LA SENAL DE A.A.A SE RESUELVE SOLA -no queda pegada esperando nada- "
              "dentro del presupuesto leido del C++ real (LA HIPOTESIS PRINCIPAL DEL "
              "ENCARGO: senalActiva atascada en true)");
    comprobar(msHastaFinSenal >= duracionAAA_MS - TOLERANCIA_MS &&
              msHastaFinSenal <= duracionAAA_MS + TOLERANCIA_MS,
              "D1: y tarda EXACTAMENTE lo que dictan los destellos leidos del C++ "
              "real, ni una senal que termina antes de tiempo ni una que se demora");
    comprobar(!verdeSeVioDuranteSenal,
              "D1: en NINGUN tick de toda la senal el VERDE volvio a encenderse -la "
              "logica normal (que ya reinicio el ciclo por debajo) no puede mover los "
              "pines mientras la senal los ocupa (requisito a del encargo)");
    comprobar(pinesCoincidenConEstado(),
              "D1: al TERMINAR la senal, terminarSenal() volco ultR/ultA/ultV y los "
              "pines quedan AL DIA con la ultima decision de la logica -si esto "
              "fallara, las luces quedarian congeladas en el patron de destellos: "
              "el sintoma de banco exacto (requisito b del encargo)");

    // El A.A.A relanza el Automatico desde cero (arranque directo, SFTY-21): vuelve
    // a pasar por el todo-rojo y el amarillo, y llega a VERDE otra vez sin que nadie
    // intervenga. Si el reinicio hubiera dejado el coordinador colgado -la otra cara
    // del mismo sintoma de banco-, este bombeo agota el presupuesto.
    long msHastaVerdeDeNuevo = bombearPrincipal(200UL,
        SEG_ESTATICO_MS + (unsigned long)AMBAR_MS + 5000UL,
        [](){ return semaforo_estado() == S_VERDE; });
    comprobar(msHastaVerdeDeNuevo >= 0,
              "D1: tras la confirmacion, A.A.A relanzo el ciclo completo -todo-rojo, "
              "amarillo, VERDE- sin que nadie mas interviniera");
  }

  // ---- D2: B.B.B DURANTE CORRIENDO -> AMBAR INTERMITENTE (emergencia) -------
  {
    arnes_millis_valor += 10000000UL;
    g_modoEsclavo = ESC_CORRECTO;
    g_latenciaEsclavoMs = 50;

    arrancarAutomaticoPorDefecto();
    bombear(200, SEG_ESTATICO_MS + (unsigned long)AMBAR_MS + 5000UL,
        [](){ return semaforo_estado() == S_VERDE; });

    pulsarYAvanzar(MANDO_B, 2000, 100);
    pulsarYAvanzar(MANDO_B, 2000, 100);
    pulsarYAvanzar(MANDO_B, 0, 100);
    comprobar(semaforo_senalEnCurso(),
              "D2: al 3er pulso de B.B.B la senal arranca de inmediato -funciona "
              "DESDE CUALQUIER MODO EN MARCHA, sin condiciones: es la salida de "
              "emergencia-");

    long msHastaFinSenal = bombearPrincipal(50UL, PRESUPUESTO_SENAL_MS,
        [](){ return !semaforo_senalEnCurso(); });
    comprobar(msHastaFinSenal >= 0,
              "D2: LA SENAL DE B.B.B SE RESUELVE SOLA dentro del presupuesto leido "
              "del C++ real");
    comprobar(msHastaFinSenal >= duracionBBB_MS - TOLERANCIA_MS &&
              msHastaFinSenal <= duracionBBB_MS + TOLERANCIA_MS,
              "D2: y tarda EXACTAMENTE los 3 destellos leidos del C++ real");
    comprobar(pinesCoincidenConEstado(),
              "D2: al terminar, los pines vuelven a coincidir con lo que la logica "
              "decidio (requisito b)");
    comprobar(modoActual_get() == MODO_AMBAR,
              "D2: la confirmacion ejecuto la accion pendiente -el sistema paso a "
              "MODO_AMBAR, la salida de emergencia real de B.B.B-");
  }

  // ---- D3: A.B.A.B ACEPTADO -> MODO DEGRADADO --------------------------------
  {
    arnes_millis_valor += 10000000UL;
    g_modoEsclavo = ESC_CORRECTO;
    g_latenciaEsclavoMs = 50;
    g_entradaDegradado = MDG_OK;   // la puerta real (modo_degradado.cpp) esta fuera
                                    // de alcance de este arnes; el arnes decide su
                                    // veredicto para poder ejercer las DOS ramas

    arrancarAutomaticoPorDefecto();
    bombear(200, SEG_ESTATICO_MS + (unsigned long)AMBAR_MS + 5000UL,
        [](){ return semaforo_estado() == S_VERDE; });

    pulsarYAvanzar(MANDO_A, 2000, 100);
    pulsarYAvanzar(MANDO_B, 2000, 100);
    pulsarYAvanzar(MANDO_A, 2000, 100);
    pulsarYAvanzar(MANDO_B, 0, 100);   // el 4o pulso confirma A.B.A.B
    comprobar(semaforo_senalEnCurso(),
              "D3: al 4o pulso de A.B.A.B, con la puerta de SFTY-18 en OK, la senal "
              "de 4 destellos arranca");

    long msHastaFinSenal = bombearPrincipal(50UL, PRESUPUESTO_SENAL_MS,
        [](){ return !semaforo_senalEnCurso(); });
    comprobar(msHastaFinSenal >= 0,
              "D3: LA SENAL DE A.B.A.B SE RESUELVE SOLA dentro del presupuesto leido "
              "del C++ real");
    comprobar(msHastaFinSenal >= duracionABAB_MS - TOLERANCIA_MS &&
              msHastaFinSenal <= duracionABAB_MS + TOLERANCIA_MS,
              "D3: y tarda EXACTAMENTE los 4 destellos leidos del C++ real -la senal "
              "mas larga de las tres-");
    comprobar(pinesCoincidenConEstado(),
              "D3: al terminar, los pines coinciden con la logica (requisito b)");
    comprobar(modoActual_get() == MODO_DEGRADADO,
              "D3: la confirmacion ejecuto la accion pendiente -el sistema paso a "
              "MODO_DEGRADADO SOLO porque la puerta de modo_degradado_evaluarEntrada() "
              "dio MDG_OK-");
  }

  // ---- D4: A.B.A.B RECHAZADO -> AMBAR RAPIDO, NO Degradado -------------------
  {
    arnes_millis_valor += 10000000UL;
    g_modoEsclavo = ESC_CORRECTO;
    g_latenciaEsclavoMs = 50;
    g_entradaDegradado = MDG_FALTA_HORA;  // SFTY-18: sin hora valida, se rechaza

    arrancarAutomaticoPorDefecto();
    bombear(200, SEG_ESTATICO_MS + (unsigned long)AMBAR_MS + 5000UL,
        [](){ return semaforo_estado() == S_VERDE; });

    pulsarYAvanzar(MANDO_A, 2000, 100);
    pulsarYAvanzar(MANDO_B, 2000, 100);
    pulsarYAvanzar(MANDO_A, 2000, 100);
    pulsarYAvanzar(MANDO_B, 0, 100);
    comprobar(semaforo_senalEnCurso(),
              "D4: CONTROL NEGATIVO: A.B.A.B con la puerta en FALTA_HORA sigue "
              "generando una senal -pero de RECHAZO, no de confirmacion-");

    // El patron de rechazo es ambar parpadeando al periodo leido del C++ (150 ms
    // por defecto), distinto del rojo a 400/400 de una confirmacion: si el arnes
    // viera el patron de rechazo tomar el camino de A.B.A.B aceptado, esta
    // comprobacion lo delataria en vez de dar un falso OK por casualidad.
    unsigned long t0 = arnes_millis_valor;
    bool vioAmbarEncendido = false;
    bombearPrincipal(10UL, (unsigned long)AMBAR_RAPIDO_PERIODO_MS + 50UL,
        [&vioAmbarEncendido](){
          if (arnes_pines[AMARILLO1] == HIGH) vioAmbarEncendido = true;
          return vioAmbarEncendido;
        });
    (void)t0;
    comprobar(vioAmbarEncendido,
              "D4: CONTROL NEGATIVO: el rechazo SI enciende el ambar -no es un rojo "
              "de confirmacion disfrazado-");

    long msHastaFinSenal = bombearPrincipal(10UL, PRESUPUESTO_SENAL_MS,
        [](){ return !semaforo_senalEnCurso(); });
    comprobar(msHastaFinSenal >= 0,
              "D4: CONTROL NEGATIVO: LA SENAL DE RECHAZO TAMBIEN SE RESUELVE SOLA "
              "-un rechazo no es forma de dejar la maquina pegada-");
    comprobar(msHastaFinSenal >= RECHAZO_AMBAR_MS - TOLERANCIA_MS &&
              msHastaFinSenal <= RECHAZO_AMBAR_MS + TOLERANCIA_MS,
              "D4: y dura EXACTAMENTE los 2000 ms de rechazo leidos del C++ real, no "
              "los 4 destellos de una confirmacion");
    comprobar(pinesCoincidenConEstado(),
              "D4: al terminar el rechazo, los pines coinciden con la logica "
              "(requisito b)");
    comprobar(modoActual_get() == MODO_AUTOMATICO,
              "D4: CONTROL NEGATIVO: el rechazo NO cambia de modo -sigue en "
              "MODO_AUTOMATICO, el sistema sigue haciendo lo que hacia-");
  }

  // ---- D5: SECUENCIA A MEDIAS -> SE PURGA, NO ARRASTRA A LA SIGUIENTE --------
  {
    arnes_millis_valor += 10000000UL;
    g_modoEsclavo = ESC_CORRECTO;
    g_latenciaEsclavoMs = 50;

    arrancarAutomaticoPorDefecto();
    bombear(200, SEG_ESTATICO_MS + (unsigned long)AMBAR_MS + 5000UL,
        [](){ return semaforo_estado() == S_VERDE; });

    // Dos pulsos de A y se abandona: sin el 3ro, no hay secuencia.
    pulsarYAvanzar(MANDO_A, 2000, 100);
    pulsarYAvanzar(MANDO_A, 0, 100);
    comprobar(!semaforo_senalEnCurso(),
              "D5: dos pulsos de A, SIN el tercero, NO disparan ninguna senal");

    // Se deja pasar la ventana completa de purgado -la secuencia a medias queda
    // fuera del historial- y se intenta un A.A.A fresco: tiene que funcionar igual
    // que si los dos pulsos viejos no hubieran ocurrido nunca.
    avanzar((unsigned long)VENTANA_CUADRUPLE_MS + 500UL, 500UL);
    comprobar(!semaforo_senalEnCurso(),
              "D5: pasada la ventana de purgado, sigue sin haber ninguna senal en "
              "curso -el abandono no dejo nada pendiente-");

    pulsarYAvanzar(MANDO_A, 2000, 100);
    pulsarYAvanzar(MANDO_A, 2000, 100);
    pulsarYAvanzar(MANDO_A, 0, 100);
    comprobar(semaforo_senalEnCurso(),
              "D5: tras la purga, un A.A.A fresco SI dispara -los pulsos viejos no "
              "arrastraron ni bloquearon nada-");
    long msHastaFinSenal = bombearPrincipal(50UL, PRESUPUESTO_SENAL_MS,
        [](){ return !semaforo_senalEnCurso(); });
    comprobar(msHastaFinSenal >= 0,
              "D5: y esa senal fresca tambien se resuelve sola");
  }

  // ---- D6: A Y B EN EL MISMO TICK (mismo contacto que Boton1/Boton2) --------
  {
    arnes_millis_valor += 10000000UL;
    g_modoEsclavo = ESC_CORRECTO;
    g_latenciaEsclavoMs = 50;

    arrancarAutomaticoPorDefecto();
    bombear(200, SEG_ESTATICO_MS + (unsigned long)AMBAR_MS + 5000UL,
        [](){ return semaforo_estado() == S_VERDE; });

    pulsarYAvanzar(MANDO_A, 2000, 100);
    pulsarYAvanzar(MANDO_A, 2000, 100);

    // El 3er A completa A.A.A EN EL MISMO TICK en que, electricamente, tambien
    // podria llegar un B (los dos reles son flancos independientes, pero nada en
    // el firmware impide que botones_actualizar() los vea en la misma iteracion).
    // Es el escenario de "orden invalida / secuencia a medias" mas exigente: dos
    // acciones queriendo nacer en el mismo instante.
    pasoPrincipal(/*pulsarA=*/true, /*pulsarB=*/true);
    vigilarEnclavamiento();
    vigilarSenal();
    arnes_millis_valor += 100UL;

    comprobar(semaforo_senalEnCurso(),
              "D6: con A y B en el MISMO tick, la senal SI arranca -la registrada "
              "primero por botones_actualizar(), aqui A- y no se pierde ni se "
              "corrompe por la coincidencia");

    long msHastaFinSenal = bombearPrincipal(50UL, PRESUPUESTO_SENAL_MS,
        [](){ return !semaforo_senalEnCurso(); });
    comprobar(msHastaFinSenal >= 0,
              "D6: CONTROL NEGATIVO: la coincidencia de flancos NO deja la senal "
              "pegada -se resuelve dentro del presupuesto igual que cualquier otra-");
    comprobar(msHastaFinSenal >= duracionAAA_MS - TOLERANCIA_MS &&
              msHastaFinSenal <= duracionAAA_MS + TOLERANCIA_MS,
              "D6: y fue LA DE A.A.A la que se ejecuto -2 destellos, no 3- porque B "
              "llego con pendiente ya distinto de ACC_NINGUNA y mando_registrarPulso() "
              "lo descarta sin registrarlo");
    comprobar(pinesCoincidenConEstado(),
              "D6: al terminar, los pines coinciden con la logica (requisito b)");
  }

  // ---- D7: REINICIO DE LA SENAL A MEDIO DESTELLO (defensivo) -----------------
  //
  // mando.cpp SOLO llama a semaforo_destellosRojos()/semaforo_ambarRapido() con la
  // senal ya resuelta (mando_registrarPulso() se corta en seco si
  // semaforo_senalEnCurso() es true). Este caso no lo puede producir mando.cpp, asi
  // que se ejerce llamando a la API de semaforo.cpp DIRECTAMENTE -no hay guarda que
  // lo impida a nivel de modulo-, para comprobar que semaforo.cpp por si solo, sin
  // depender de que mando.cpp le haga de niñera, tampoco se queda pegado si alguien
  // lo reinicia a medio destello.
  {
    semaforo_setup();
    arnes_millis_valor += 10000000UL;

    semaforo_destellosRojos((uint8_t)DESTELLOS_DEGRADADO);
    // Deja pasar UN destello y medio -senal claramente A MEDIAS- antes de pedir
    // otra vez el arranque.
    long avance = (DESTELLO_ON_MS + DESTELLO_OFF_MS) + DESTELLO_ON_MS / 2;
    unsigned long hecho = 0;
    while ((long)hecho < avance) {
      semaforo_actualizar();
      vigilarEnclavamiento();
      vigilarSenal();
      arnes_millis_valor += 20UL;
      hecho += 20UL;
    }
    comprobar(semaforo_senalEnCurso(),
              "D7: a medio destello, la primera senal SIGUE en curso -confirma que "
              "el reinicio de abajo llega de verdad a mitad, no despues de terminar-");

    semaforo_destellosRojos((uint8_t)DESTELLOS_AUTOMATICO);   // reinicio a medias
    long msHastaFinSenal = 0;
    {
      unsigned long inicio = arnes_millis_valor;
      for (;;) {
        semaforo_actualizar();
        vigilarEnclavamiento();
        vigilarSenal();
        if (!semaforo_senalEnCurso()) { msHastaFinSenal = (long)(arnes_millis_valor - inicio); break; }
        if (arnes_millis_valor - inicio >= PRESUPUESTO_SENAL_MS) { msHastaFinSenal = -1; break; }
        arnes_millis_valor += 20UL;
      }
    }
    comprobar(msHastaFinSenal >= 0,
              "D7: CONTROL NEGATIVO: reiniciar la senal A MEDIO DESTELLO tampoco la "
              "deja pegada -semaforo.cpp resuelve la NUEVA senal por su cuenta, sin "
              "depender de que nadie la vigile desde fuera-");
  }

  // ---- D8: DOS GESTOS COMPLETOS CONSECUTIVOS, SIN RESPIRO --------------------
  {
    arnes_millis_valor += 10000000UL;
    g_modoEsclavo = ESC_CORRECTO;
    g_latenciaEsclavoMs = 50;

    arrancarAutomaticoPorDefecto();
    bombear(200, SEG_ESTATICO_MS + (unsigned long)AMBAR_MS + 5000UL,
        [](){ return semaforo_estado() == S_VERDE; });

    pulsarYAvanzar(MANDO_A, 2000, 100);
    pulsarYAvanzar(MANDO_A, 2000, 100);
    pulsarYAvanzar(MANDO_A, 0, 100);
    long msFinA = bombearPrincipal(50UL, PRESUPUESTO_SENAL_MS,
        [](){ return !semaforo_senalEnCurso(); });
    comprobar(msFinA >= 0, "D8: la primera senal (A.A.A) se resuelve sola");

    // El B.B.B llega EN CUANTO termina la anterior -sin margen-: si algo de la
    // primera senal quedara a medio limpiar, esta es la que lo delataria.
    pulsarYAvanzar(MANDO_B, 2000, 100);
    pulsarYAvanzar(MANDO_B, 2000, 100);
    pulsarYAvanzar(MANDO_B, 0, 100);
    comprobar(semaforo_senalEnCurso(),
              "D8: el B.B.B pegado a la cola del A.A.A SI dispara -nada de la "
              "primera senal bloqueo a la segunda-");
    long msFinB = bombearPrincipal(50UL, PRESUPUESTO_SENAL_MS,
        [](){ return !semaforo_senalEnCurso(); });
    comprobar(msFinB >= 0,
              "D8: y la segunda senal (B.B.B) tambien se resuelve sola -dos gestos "
              "seguidos, dos resoluciones limpias-");
    comprobar(modoActual_get() == MODO_AMBAR,
              "D8: la segunda accion (AMBAR) es la que quedo vigente, no un residuo "
              "de la primera (AUTOMATICO)");
  }

  // ---- D9: FUZZ — cientos de pulsos pseudoaleatorios, con el vigilante puesto ----
  //
  // Los D1-D8 prueban los caminos que se nos ocurrieron. Este no prueba UN camino:
  // recorre miles de combinaciones de A/B con huecos aleatorios -algunos formaran
  // A.A.A o B.B.B o A.B.A.B por casualidad, la mayoria no formaran nada- mientras
  // vigilarSenal() (enganchado en TODO bombeo desde el arranque del programa) vigila
  // que ninguna senal exceda el presupuesto. Semilla FIJA: la corrida es reproducible,
  // no un dado distinto cada vez que alguien ejecuta el arnes.
  {
    arnes_millis_valor += 10000000UL;
    g_modoEsclavo = ESC_CORRECTO;
    g_latenciaEsclavoMs = 50;
    g_entradaDegradado = MDG_OK;

    arrancarAutomaticoPorDefecto();
    bombear(200, SEG_ESTATICO_MS + (unsigned long)AMBAR_MS + 5000UL,
        [](){ return semaforo_estado() == S_VERDE; });

    std::srand(0xA070u);   // N-52, fecha del encargo: semilla fija y documentada
    const int PULSOS = 600;
    for (int i = 0; i < PULSOS; i++) {
      bool esA = (std::rand() % 2) == 0;
      unsigned long gap = 300UL + (unsigned long)(std::rand() % 3000);
      pulsarYAvanzar(esA ? MANDO_A : MANDO_B, gap, 50UL);
      // Si el pulso disparo una senal, se deja correr hasta que se resuelva -o
      // hasta que vigilarSenal() ya la haya marcado como excedida-. No hace falta
      // esperar mas que el propio presupuesto: si a esta altura sigue activa, ya
      // esta contado como fallo y seguir esperando no aporta nada.
      unsigned long esperado = 0;
      while (semaforo_senalEnCurso() && esperado < PRESUPUESTO_SENAL_MS) {
        pasoPrincipal();
        vigilarEnclavamiento();
        vigilarSenal();
        arnes_millis_valor += 50UL;
        esperado += 50UL;
      }
    }
    comprobar(violacionesTalanquera == 0,
            "en NINGUN instante del barrido la talanquera estuvo ARRIBA sin verde "
            "encendido y fuera de S_FALLO (SFTY-28, medido sobre el pin que "
            "semaforo.cpp real escribio: rojo, ambar de transicion, todo-rojo y "
            "destellos del mando la dejan abajo; el ambar intermitente de SFTY-6 la "
            "sube, por decision del cliente)");
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
    arnes_pines[MOTOR_TALANQUERA] = TALANQUERA_ABRIR;
    arnes_pines[VERDE1] = LOW; arnes_pines[VERDE2] = LOW;
    vigilarEnclavamiento();
    bool detecta = (violacionesTalanquera == antes + 1);
    arnes_pines[MOTOR_TALANQUERA] = guardaP;
    arnes_pines[VERDE1] = guardaV1; arnes_pines[VERDE2] = guardaV2;
    violacionesTalanquera = antes;
    discrepanciasPluma = antesPluma;
    comprobar(detecta,
              "control negativo: el vigilante de la pluma SI cuenta una violacion "
              "cuando la talanquera esta arriba con los dos verdes apagados");
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
    arnes_pines[MOTOR_TALANQUERA] =
        semaforo_plumaArriba() ? TALANQUERA_CERRAR : TALANQUERA_ABRIR;
    vigilarEnclavamiento();
    bool cazado = (discrepanciasPluma == antesD + 1);
    arnes_pines[MOTOR_TALANQUERA] = guardaP;
    // Los dos contadores se restauran: falsear el pin puede disparar tambien el
    // invariante de SFTY-28, y dejarlo contado convertiria este control en un fallo
    // del otro.
    discrepanciasPluma = antesD;
    violacionesTalanquera = antesT;
    comprobar(cazado,
              "control negativo: el vigilante de PLUMA SI cuenta una discrepancia "
              "cuando el pin dice lo contrario que el getter");
  }
  comprobar(!g_senalExcedioPresupuesto,
              "D9 (FUZZ): en 600 pulsos pseudoaleatorios de A/B -con huecos entre "
              "300 y 3300 ms, formando y rompiendo secuencias por casualidad- "
              "NINGUNA senal excedio el presupuesto leido del C++ real");
    char msgPeor[160];
    std::snprintf(msgPeor, sizeof(msgPeor),
        "D9 (FUZZ): la senal MAS LARGA observada en todo el fuzz duro %ld ms, "
        "contra un presupuesto de %lu ms",
        g_peorDuracionSenalMs, PRESUPUESTO_SENAL_MS);
    comprobar(g_peorDuracionSenalMs < 0 ||
                  (unsigned long)g_peorDuracionSenalMs <= PRESUPUESTO_SENAL_MS,
              msgPeor);
  }

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
    g_camaraLocal = true;              // coches en mi sentido, todo el rato
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
    g_camaraLocal = true;
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
    g_camaraLocal = true;
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
    g_camaraLocal = true;
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

    g_camaraLocal = false;
    g_demandaRemotaEncolada = false;
  }

  // ===========================================================================
  comprobar(violacionesEnclavamiento == 0,
            "en NINGUN instante de todo el barrido -los nueve bloques- coincidieron "
            "ROJO y VERDE encendidos a la vez en la misma cara (SFTY-2, medido sobre "
            "lo que semaforo.cpp real escribio en los pines, no sobre la logica)");
  comprobar(!g_senalExcedioPresupuesto,
            "RESUMEN DEL VIGILANTE DE SENAL: en NINGUN bloque -A a D9- "
            "semaforo_senalEnCurso() estuvo pegada en true mas alla del presupuesto "
            "leido del C++ real. Esta es la comprobacion que responde a la hipotesis "
            "del encargo (N-52)");

  std::printf("\n==============================================================\n");
  std::printf(" RESULTADO: %d/%d comprobaciones OK\n", total - fallos, total);
  std::printf("==============================================================\n");
  std::printf(" Medido sobre coordinador.cpp + semaforo.cpp + modo_automatico.cpp +\n");
  std::printf(" mando.cpp (N-52) + modo_inteligente.cpp y demanda.cpp (A-12) REALES,\n");
  std::printf(" compilados para el PC. %lu redibujos de\n",
              g_lcdRedibujos);
  std::printf(" pantalla observados, %lu escrituras de pin observadas. Peor duracion\n",
              arnes_escrituras);
  std::printf(" de senal observada en todo el barrido: %ld ms (presupuesto %lu ms).\n",
              g_peorDuracionSenalMs, PRESUPUESTO_SENAL_MS);
  std::printf(" Ningun paso de este arnes reimplementa el ciclo: lo que se mide es el\n");
  std::printf(" binario, no un modelo de el.\n");
  return fallos == 0 ? 0 : 1;
}
