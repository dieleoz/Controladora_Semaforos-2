// ===== Validacion_Automatico/dos_puntas/orquestador_degradado.cpp =====
//
// LAS DOS PUNTAS EN MODO DEGRADADO A LA VEZ, CADA UNA CON SU RELOJ, Y LA PREGUNTA
// QUE NINGUN INSTRUMENTO HABIA HECHO SOBRE EL C++ REAL:
//
//     ?CUANTOS SEGUNDOS DE DESFASE ENTRE LOS DOS RELOJES AGUANTA EL CRUCE ANTES DE
//     QUE LOS DOS VERDES SE TOQUEN, Y CUANTO PUEDE DERIVAR EL EQUIPO DE VERDAD?
//
// El Modo Degradado es el modo que se usa cuando la radio MUERE. Ahi el verde de cada
// punta sale de SU PROPIO RELOJ y no hay nadie que coordine: es el unico modo del
// equipo en el que un choque frontal depende de una desigualdad numerica y no de un
// enclavamiento. Hasta hoy esa desigualdad -despeje ampliado contra deriva acumulada-
// la recalculaba UNICAMENTE costura_02_fase_ciclo.py, o sea un modelo de Python
// escrito a mano; y ademas la recalculaba en UNA SOLA DIRECCION (ver el hallazgo del
// bloque C).
//
// Este arnes es hermano de orquestador.cpp y NO lo sustituye. Aquel monta el Maestro
// en Modo Automatico gobernando por radio y mide 42 comprobaciones que este no repite.
// Aquel declara ademas, en la cabecera de adaptador_maestro.cpp, el hueco que este
// viene a tapar: "en este arnes el MODO DEGRADADO del Maestro no existe".
//
// ===========================================================================
// LAS DOS RAZONES POR LAS QUE EL HUECO SEGUIA ABIERTO, Y COMO SE CIERRAN
// ===========================================================================
//
// (1) "modo_degradado.cpp del Maestro arrastra lcd.h/menu.h y por tanto u8g2".
//     MEDIDO Y FALSO. Ninguna de las 23 cabeceras del Maestro incluye <U8g2lib.h>;
//     lo arrastra lcd.cpp, que nadie obliga a compilar. La medida completa y por que
//     no se reutiliza el camino de Validacion_LCD -131 ficheros .c de U8g2 para medir
//     geometria de pantalla, que aqui no se mide- estan en la cabecera de
//     adaptador_maestro_deg.cpp. Este arnes usa las cabeceras REALES lcd.h, menu.h y
//     botones.h del Maestro, sin sustituto: una declaracion copiada puede divergir en
//     silencio, la real no.
//
// (2) "las dos puntas comparten arnes_millis, la deriva no es representable".
//     Cierto, y es la mitad que importa. Aqui se rompe por dos sitios:
//
//       a) CADA PUNTA TIENE SU PROPIO TICK. unTick() llama a MAESTRO.tick(tM) y a
//          ESCLAVO.tick(tE) con numeros distintos si hace falta. arnes_millis_valor
//          es una variable POR DLL -ese es el mecanismo entero del arnes-, asi que
//          nunca fue una variable compartida: lo compartido era que el orquestador
//          les pasaba el mismo numero.
//
//       b) EL DESFASE SE INYECTA EN EL RTC, NO EN millis(). Un salto de millis() hacia
//          atras haria que TODAS las restas sin signo del firmware -(ahora - tEstado)-
//          dieran ~4.290 millones y cada temporizador venceria de golpe: el arnes
//          estaria midiendo su propia averia. La deriva entre dos cristales de
//          32.768 kHz es una discrepancia de HORA DE PARED, y ahi es donde se mete.
//          millis() sigue siendo monotono en las dos puntas.
//
//          Y SE INYECTA CONSERVANDO LA FASE SUB-SEGUNDO. Las dos vias obvias
//          -reloj_ajustar() y la escritura del contador en el dominio de respaldo-
//          reanclan el RTC al millis() del momento, o sea que MUEVEN el instante en
//          que el RTC cambia de segundo. Con eso el arnes fabricaba su propio residuo
//          sub-segundo encima del residuo real de la sincronizacion, y como es JUSTO
//          ese residuo el que decide si un desfase de exactamente 30 s solapa, el
//          umbral publicado se movia con el instante en que uno decidiera inyectar.
//          Se vio porque el barrido daba solapes de 950 ms en la frontera: demasiado
//          redondo para un tiempo de aire de 50 ms. Hoy la inyeccion suma segundos a
//          la base de la hora de pared sin tocar el ancla.
//
//     POR QUE UN ESCALON Y NO UNA RAMPA. La fase del Degradado sale de
//     ciclo_degradado_fase(segundosDelDia, ...), que NO tiene memoria: depende solo de
//     la diferencia INSTANTANEA entre los dos relojes. Una rampa de 48 h atraviesa
//     todos los valores intermedios, y el barrido los recorre todos uno por uno, asi
//     que el conjunto de solapes es el mismo. Simular 48 h a 50 ms de paso serian
//     3,4 millones de vueltas por punta y por desfase; el escalon da el mismo numero.
//
// ===========================================================================
// QUE SE MIDE, Y SOBRE QUE
// ===========================================================================
//
// Sobre lo que semaforo.cpp ESCRIBIO EN LOS PINES, nunca sobre su logica ni sobre
// semaforo_estado(). El arnes ORQUESTA Y OBSERVA: no calcula en ningun sitio que fase
// tocaria ni cuando deberia haber verde. Si lo hiciera seria la segunda copia del
// firmware escrita a mano que este repositorio persigue, y el numero que publica -el
// desfase critico- saldria de esa copia y no del codigo.
//
// LO QUE ESTE ARNES NO CUBRE, dicho para que nadie lo cuente como cubierto:
//   - protocolo.cpp no se compila: CRC, rafaga y proteccion de replay van por otro
//     lado (costura_01 y el arnes del puente).
//   - la LCD no se dibuja: solo se cuentan las llamadas. La geometria la mide
//     Validacion_LCD sobre el lcd.cpp real.
//   - el RTC es un modelo de PERIFERICO en las dos puntas, con la MISMA aritmetica
//     literal, para que una diferencia entre ellas sea del firmware y no del arnes.
//   - EL MICROCORTE NO SE EJERCE AQUI. La reanudacion tras corte (N-20) la mide el
//     bloque D del orquestador.cpp hermano. Repetirla aqui exigiria reanclar el RTC
//     de la DLL recien cargada antes de su setup(), y un reanclado mal hecho falsea
//     justo el reloj que este arnes viene a medir.
//   - la deriva se inyecta como discrepancia de reloj de pared. NO se modela una
//     deriva de la BASE DE TIEMPO DE PROGRAMA (millis), que afectaria a los 30 s de
//     todo-rojo de entrada; es de segundo orden frente a la del ciclo, que sale del
//     RTC, pero no esta medido aqui.
//   - y nada de esto sustituye la prueba de banco.

#include <windows.h>

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <regex>

// El orquestador NO incluye ninguna cabecera del firmware: si lo hiciera volveria a
// tener nombres de las dos puntas en su propia tabla de simbolos, que es el problema
// que la DLL resuelve.
#define HIGH 1
#define LOW 0
#include "comun/pines.h"
#include "punta_api.h"

// ---------------------------------------------------------------------------
// EL CONTADOR. [bloque literal de orquestador.cpp]
// ---------------------------------------------------------------------------
static int total = 0, fallos = 0;

static void comprobar(bool ok, const std::string& que) {
  total++;
  if (ok) {
    std::printf("   [OK]    %s\n", que.c_str());
  } else {
    fallos++;
    std::printf("   [FALLA] %s\n", que.c_str());
  }
}

// Lo que NO cuenta como comprobacion: un dato que acompana a una comprobacion que ya
// cuenta. Es la primitiva reportar() del banco, con el mismo contrato.
static void nota(const std::string& que) {
  std::printf("   [NOTA]  %s\n", que.c_str());
}

static void abortar(const std::string& motivo) {
  std::fprintf(stdout, "\n[ABORTADO] %s\n", motivo.c_str());
  std::fprintf(stdout,
      "Un ABORTADO no dice NADA del firmware, y menos que un PASS. Mientras este arnes\n"
      "no corra, el margen del Degradado contra la deriva entre relojes no lo mide\n"
      "nadie sobre el codigo real: solo un modelo de Python escrito a mano.\n");
  std::exit(2);
}

static std::string dirDeEsteArchivo() {
  std::string f = __FILE__;
  size_t p = f.find_last_of("/\\");
  return (p == std::string::npos) ? std::string(".") : f.substr(0, p);
}

static const std::string AQUI = dirDeEsteArchivo();
static const std::string RAIZ = AQUI + "/../..";        // 01_Firmware

static std::string leerFuente(const std::string& ruta) {
  std::ifstream f(ruta.c_str());
  if (!f) abortar("no se pudo abrir el fuente real " + ruta);
  std::ostringstream ss;
  ss << f.rdbuf();
  return ss.str();
}

// Las constantes se releen del C++ en cada corrida. SIN VALOR POR DEFECTO, NUNCA.
static unsigned long leerNumero(const std::string& ruta, const std::string& patron,
                                const std::string& que) {
  std::string txt = leerFuente(ruta);
  std::smatch m;
  std::regex re(patron);
  if (!std::regex_search(txt, m, re)) {
    abortar("no se pudo leer del C++ real la constante de " + que +
            " (patron no encontrado en " + ruta + ")");
  }
  return std::strtoul(m[1].str().c_str(), nullptr, 10);
}

// ---------------------------------------------------------------------------
// UNA PUNTA: SU DLL Y SU API. [bloque literal de orquestador.cpp]
// ---------------------------------------------------------------------------
typedef const char* (*FnNombre)(void);
typedef void (*FnVoid)(void);
typedef void (*FnTick)(unsigned long);
typedef int  (*FnPin)(int);
typedef int  (*FnEstado)(void);
typedef unsigned long (*FnEscrituras)(void);
typedef int  (*FnTx)(unsigned char*);
typedef void (*FnRx)(const unsigned char*);
typedef void (*FnEntrada)(int, int);
typedef void (*FnPulsar)(int);
typedef long (*FnMando)(const char*, long);
typedef long (*FnDomLeer)(int);
typedef void (*FnDomEscribir)(int, long);

struct Punta {
  std::string ruta;
  std::string etiquetaEsperada;
  HMODULE h = nullptr;

  FnNombre      nombre = nullptr;
  FnVoid        arrancar = nullptr;
  FnTick        tick = nullptr;
  FnPin         pin = nullptr;
  FnEstado      estado = nullptr;
  FnEscrituras  escrituras = nullptr;
  FnTx          tx = nullptr;
  FnRx          rx = nullptr;
  FnEntrada     entrada = nullptr;
  FnPulsar      pulsar = nullptr;
  FnMando       mando = nullptr;
  FnDomLeer     domLeer = nullptr;
  FnDomEscribir domEscribir = nullptr;

  template <typename T>
  T resolver(const char* simbolo) {
    FARPROC p = GetProcAddress(h, simbolo);
    if (!p) abortar("la DLL " + ruta + " no exporta " + simbolo +
                    ". Las dos puntas tienen que cumplir el MISMO contrato");
    return reinterpret_cast<T>(reinterpret_cast<void*>(p));
  }

  void cargar() {
    h = LoadLibraryA(ruta.c_str());
    if (!h) abortar("no se pudo cargar " + ruta + " (LoadLibrary devolvio NULL, error " +
                    std::to_string((unsigned long)GetLastError()) + ")");
    nombre      = resolver<FnNombre>("punta_nombre");
    arrancar    = resolver<FnVoid>("punta_arrancar");
    tick        = resolver<FnTick>("punta_tick");
    pin         = resolver<FnPin>("punta_pin");
    estado      = resolver<FnEstado>("punta_estado");
    escrituras  = resolver<FnEscrituras>("punta_escrituras");
    tx          = resolver<FnTx>("punta_tx");
    rx          = resolver<FnRx>("punta_rx");
    entrada     = resolver<FnEntrada>("punta_entrada");
    pulsar      = resolver<FnPulsar>("punta_pulsar");
    mando       = resolver<FnMando>("punta_mando");
    domLeer     = resolver<FnDomLeer>("punta_dominio_leer");
    domEscribir = resolver<FnDomEscribir>("punta_dominio_escribir");

    if (etiquetaEsperada != nombre()) {
      abortar("la DLL " + ruta + " dice llamarse '" + nombre() + "' y se esperaba '" +
              etiquetaEsperada + "'. Cargar dos veces la misma punta daria un arnes "
              "midiendo una punta contra si misma, y no lo notaria nadie");
    }
  }

  void descargar() {
    if (h) FreeLibrary(h);
    h = nullptr;
  }

  long orden(const char* que, long arg = 0) {
    long r = mando(que, arg);
    if (r == PUNTA_DESCONOCIDO) {
      abortar(std::string("la punta ") + etiquetaEsperada + " no conoce la orden '" +
              que + "'. Una consulta que no existe no puede devolver un valor util");
    }
    return r;
  }

  bool verde()  { return pin(VERDE1) == HIGH || pin(VERDE2) == HIGH; }
  bool rojo()   { return pin(ROJO1) == HIGH && pin(ROJO2) == HIGH; }
  bool ambar()  { return pin(AMARILLO1) == HIGH || pin(AMARILLO2) == HIGH; }
};

static Punta MAESTRO, ESCLAVO;

// ---------------------------------------------------------------------------
// EL CANAL DE RADIO. En este arnes se usa para montar el escenario -la
// sincronizacion horaria REAL de SFTY-23- y despues SE CORTA: el Modo Degradado se
// define por no tener radio, y con el enlace vivo no se estaria midiendo el Degradado.
// ---------------------------------------------------------------------------
struct EnVuelo {
  unsigned char trama[4];
  unsigned long tEntrega;
  int destino;   // 0 = Maestro, 1 = Esclavo
};

static std::vector<EnVuelo> g_aire;
static bool g_enlace = true;
static unsigned long g_latenciaMs = 50;
static unsigned long g_tramasEntregadas = 0;

// ---------------------------------------------------------------------------
// LOS DOS RELOJES. g_t es el tiempo del BANCO; cada punta recibe el suyo.
//
// g_desfaseRtcS es la deriva acumulada del RTC del Esclavo respecto del Maestro. No
// se aplica aqui: se aplica reanclando su RTC (ver aplicarDesfaseRtc), que es donde
// vive de verdad.
// ---------------------------------------------------------------------------
static unsigned long g_t = 2000;          // arranca por encima del delay(2000) de N-22
static const unsigned long PASO_MS = 50;

// ---------------------------------------------------------------------------
// EL OBSERVADOR. Corre DESPUES de que las dos puntas hayan ejecutado su instante.
// ---------------------------------------------------------------------------
static unsigned long g_instantes = 0;
static unsigned long g_verdeSimultaneo = 0;
static unsigned long g_ticksVerdeMaestro = 0;
static unsigned long g_ticksVerdeEsclavo = 0;
static unsigned long g_enclavamientoRoto = 0;
static unsigned long g_talanqueraSinVerde = 0;

// Verde de una punta seguido del verde de la otra SIN un solo instante de todo-rojo en
// medio. Es igual de mortal que el solape y solo se ve mirando la transicion, no el
// instante: lo aprendio costura_02 y aqui se mide sobre los pines reales.
static unsigned long g_pegados = 0;
static bool g_antVerdeM = false, g_antVerdeE = false;
static bool g_huboRojoTotalDesdeVerde = true;

// La racha MAS LARGA de instantes seguidos con las dos en verde. Es lo que distingue
// un solape estructural -de segundos- de uno de milisegundos, y a diferencia del total
// NO depende de cuantos ciclos quepan en la ventana de observacion.
static unsigned long g_solapeRacha = 0;
static unsigned long g_solapeMax = 0;

// El detector, aislado para que el control negativo pueda ejercerlo con valores
// sinteticos. Un detector que solo se prueba cuando nada falla es un adorno.
static bool hayVerdeSimultaneo(bool verdeA, bool verdeB) { return verdeA && verdeB; }

static void reiniciarObservacion() {
  g_instantes = 0;
  g_verdeSimultaneo = 0;
  g_ticksVerdeMaestro = 0;
  g_ticksVerdeEsclavo = 0;
  g_pegados = 0;
  g_antVerdeM = g_antVerdeE = false;
  g_huboRojoTotalDesdeVerde = true;
  g_solapeRacha = 0;
  g_solapeMax = 0;
}

static void vigilar() {
  g_instantes++;
  const bool vM = MAESTRO.verde();
  const bool vE = ESCLAVO.verde();
  if (vM) g_ticksVerdeMaestro++;
  if (vE) g_ticksVerdeEsclavo++;

  if (hayVerdeSimultaneo(vM, vE)) {
    g_verdeSimultaneo++;
    g_solapeRacha++;
    if (g_solapeRacha > g_solapeMax) g_solapeMax = g_solapeRacha;
  } else {
    g_solapeRacha = 0;
  }

  // Transicion verde -> verde sin todo-rojo de por medio, en cualquiera de los dos
  // sentidos. Se exige haber visto un instante con las DOS puntas sin verde entre un
  // verde y el siguiente de la otra.
  if (!vM && !vE) g_huboRojoTotalDesdeVerde = true;
  if (vM && g_antVerdeE && !g_huboRojoTotalDesdeVerde) g_pegados++;
  if (vE && g_antVerdeM && !g_huboRojoTotalDesdeVerde) g_pegados++;
  if (vM || vE) g_huboRojoTotalDesdeVerde = false;
  g_antVerdeM = vM;
  g_antVerdeE = vE;

  const int S_FALLO = 3;
  Punta* dos[2] = { &MAESTRO, &ESCLAVO };
  for (int i = 0; i < 2; i++) {
    Punta* p = dos[i];
    if ((p->pin(ROJO1) == HIGH && p->pin(VERDE1) == HIGH) ||
        (p->pin(ROJO2) == HIGH && p->pin(VERDE2) == HIGH)) {
      g_enclavamientoRoto++;
    }
    if (p->pin(MOTOR_TALANQUERA) == TALANQUERA_ABRIR &&
        p->pin(VERDE1) != HIGH && p->pin(VERDE2) != HIGH &&
        p->estado() != S_FALLO) {
      g_talanqueraSinVerde++;
    }
  }
}

// ---------------------------------------------------------------------------
// EL BUCLE. Un tick = un instante del banco, CON EL RELOJ DE CADA PUNTA.
// ---------------------------------------------------------------------------
static void unTick() {
  for (size_t i = 0; i < g_aire.size();) {
    if (g_aire[i].tEntrega <= g_t) {
      Punta& d = (g_aire[i].destino == 0) ? MAESTRO : ESCLAVO;
      d.rx(g_aire[i].trama);
      g_tramasEntregadas++;
      g_aire.erase(g_aire.begin() + i);
    } else {
      i++;
    }
  }

  // Las dos puntas ejecutan el mismo instante del banco. millis() es MONOTONO en las
  // dos: lo que difiere entre ellas es la hora de pared de su RTC, que es donde vive
  // la deriva entre cristales.
  MAESTRO.tick(g_t);
  ESCLAVO.tick(g_t);

  unsigned char b[4];
  while (MAESTRO.tx(b)) {
    if (g_enlace) {
      EnVuelo e; memcpy(e.trama, b, 4); e.tEntrega = g_t + g_latenciaMs; e.destino = 1;
      g_aire.push_back(e);
    }
  }
  while (ESCLAVO.tx(b)) {
    if (g_enlace) {
      EnVuelo e; memcpy(e.trama, b, 4); e.tEntrega = g_t + g_latenciaMs; e.destino = 0;
      g_aire.push_back(e);
    }
  }

  vigilar();
  g_t += PASO_MS;
}

static void avanzar(unsigned long ms) {
  unsigned long hecho = 0;
  while (hecho < ms) { unTick(); hecho += PASO_MS; }
}

// ---------------------------------------------------------------------------
// LA INYECCION DE LA DERIVA.
//
// El desfase se define como "el Esclavo va D segundos por delante del Maestro", y se
// aplica MOVIENDO EL RELOJ DEL MAESTRO -D. Solo la DIFERENCIA entre los dos relojes
// entra en ciclo_degradado_fase(), asi que las dos formas son la misma; se elige esta
// porque la orden "desviar_rtc" conserva la fase sub-segundo del RTC y las vias del
// Esclavo no (ver el comentario de esa orden en adaptador_maestro_deg.cpp).
//
// Que conserve la fase sub-segundo NO es un detalle: el residuo sub-segundo de la
// sincronizacion es JUSTO lo que decide si un desfase de exactamente 30 s solapa o no,
// y un arnes que fabricara el suyo propio estaria publicando su umbral, no el del
// equipo.
// ---------------------------------------------------------------------------
static void aplicarDesfaseEsclavoAdelantado(long segundos) {
  if (segundos == 0) return;
  MAESTRO.orden("desviar_rtc", -segundos);
}

// ---------------------------------------------------------------------------
// EL ESCENARIO. Las dos puntas en Degradado, POR SU PUERTA REAL.
//
// Nada de esto se simula: la hora, la configuracion del ciclo y la medida de desfase
// viajan por la radio del arnes entre el coordinador REAL del Maestro y el despachador
// REAL del Esclavo, con sus ACK y sus reintentos. La puerta del Maestro
// -modo_degradado_evaluarEntrada()- exige las tres, y si el arnes se saltara una, no
// dejaria entrar.
// ---------------------------------------------------------------------------
static int MODO_DEGRADADO_V = -1;   // se lee de modos.h, nunca se escribe a mano

// El delay(2000) de N-22 que gasta el setup() de las dos puntas. Se le suma al reloj
// del banco tras arrancar para que millis() no retroceda en la primera vuelta: un
// millis() que retrocede hace vencer TODOS los temporizadores del firmware de golpe.
static const unsigned long DELAY_ARRANQUE_MS = 2000;

static void arrancarLasDos() {
  g_aire.clear();
  g_enlace = true;
  MAESTRO.descargar(); MAESTRO.cargar();
  ESCLAVO.descargar(); ESCLAVO.cargar();
  MAESTRO.arrancar();
  ESCLAVO.arrancar();
  g_t += DELAY_ARRANQUE_MS;
}

struct Escenario {
  long motivoMaestro = -1;    // MotivoDegradado: 0 = MDG_OK
  long rechazoEsclavo = -1;   // RechazoDegradado: 0 = DEG_ACEPTADO
  long desfaseMedido = 0;
  bool configConfirmada = false;
  bool desfaseValido = false;
};

// Deja las dos puntas sincronizadas por radio y listas para entrar en Degradado.
// La hora se elige lejos de medianoche: la guarda de medianoche fuerza todo-rojo y
// mediria justo el tramo en el que ninguna punta puede dar verde.
static Escenario prepararSincronizadas(uint8_t dia, uint8_t hh, uint8_t mm, uint8_t ss) {
  arrancarLasDos();

  const long empaquetado = (long)dia * 1000000L + (long)hh * 10000L +
                           (long)mm * 100L + (long)ss;
  MAESTRO.orden("ajustar_reloj", empaquetado);

  // Los tres intercambios de SFTY-23, encolados por las MISMAS funciones publicas que
  // usan la pantalla AJUSTAR HORA y el setup() del equipo.
  MAESTRO.orden("sincronizar_hora");
  MAESTRO.orden("publicar_config");
  MAESTRO.orden("medir_desfase");

  // Tiempo de sobra para los tres intercambios con sus reintentos y el retardo de
  // cortesia del Esclavo (SFTY-17).
  avanzar(120000);

  Escenario e;
  e.motivoMaestro     = MAESTRO.orden("deg_evaluar");
  e.rechazoEsclavo    = ESCLAVO.orden("degradado_comprobar");
  e.desfaseMedido     = MAESTRO.orden("desfase");
  e.configConfirmada  = MAESTRO.orden("config_confirmada") != 0;
  e.desfaseValido     = MAESTRO.orden("desfase_valido") != 0;
  return e;
}

// Mete a las dos en Degradado, corta la radio y aplica la deriva. Devuelve cuando el
// escenario esta montado; la observacion la hace quien llama.
//
// EL ORDEN NO ES ARBITRARIO Y LO IMPUSO EL FIRMWARE, no una preferencia del arnes.
// Esclavo/src/main.cpp:383 saca al Esclavo del Degradado en cuanto le llega una trama
// de GOBIERNO -PING, GO_RED o GO_GREEN-, y con razon: "si vuelve el radio, el Maestro
// manda". Meter al Esclavo primero, con el Maestro todavia en el menu latiendo cada
// 3 s, lo expulsaba en el siguiente PING y el arnes media un cruce con UNA sola punta
// en Degradado. El orden real de la calle es el otro:
//
//   1. El Maestro entra en Degradado y CALLA (main.cpp lo deja fuera del coordinador).
//   2. Muere la radio.
//   3. El operario sube al otro poste y entra alli.
static void entrarEnDegradadoLasDos(long desfaseSegEsclavo) {
  MAESTRO.orden("set_modo", MODO_DEGRADADO_V);

  // Una vuelta para que pasoPrincipal() del Maestro dispare modo_degradado_setup() por
  // el camino de main.cpp -no se llama a mano: eso seria saltarse la puerta-. En esa
  // vuelta el Maestro manda su ultimo GO_RED (coordinador_forzarRojoTotal) y despues
  // enmudece.
  avanzar(2000);

  // Y AHORA se corta la radio. El Degradado se define por no tenerla; dejarla viva
  // seria medir "modo normal con otra pantalla".
  g_enlace = false;
  g_aire.clear();

  ESCLAVO.orden("degradado_entrar");

  // La deriva acumulada, de golpe. Ver la cabecera: la fase no tiene memoria.
  aplicarDesfaseEsclavoAdelantado(desfaseSegEsclavo);
}

struct Medida {
  unsigned long instantes = 0;
  unsigned long simultaneos = 0;
  unsigned long solapeMaxMs = 0;
  unsigned long verdeM = 0;
  unsigned long verdeE = 0;
  unsigned long pegados = 0;
};

// Un barrido completo para un desfase dado. msObservacion tiene que cubrir el
// todo-rojo de entrada MAS varios ciclos completos, o el "no hubo solape" seria el de
// un cruce que nunca llego a dar verde.
static Medida correrConDesfase(long desfaseSegEsclavo, unsigned long msObservacion,
                               uint8_t dia, uint8_t hh, uint8_t mm, uint8_t ss) {
  prepararSincronizadas(dia, hh, mm, ss);
  entrarEnDegradadoLasDos(desfaseSegEsclavo);
  reiniciarObservacion();
  avanzar(msObservacion);

  Medida m;
  m.instantes   = g_instantes;
  m.simultaneos = g_verdeSimultaneo;
  m.solapeMaxMs = g_solapeMax * PASO_MS;
  m.verdeM      = g_ticksVerdeMaestro;
  m.verdeE      = g_ticksVerdeEsclavo;
  m.pegados     = g_pegados;
  return m;
}

// ---------------------------------------------------------------------------
int main() {
  std::printf("==============================================================\n");
  std::printf(" LAS DOS PUNTAS EN MODO DEGRADADO - el C++ REAL de las dos,\n");
  std::printf(" cada una con su reloj, y el barrido del desfase\n");
  std::printf("==============================================================\n");

  // --- Constantes releidas del C++ real. SIN VALOR POR DEFECTO, NUNCA ------
  const std::string M_DEG = RAIZ + "/Maestro/src/modo_degradado.cpp";
  const std::string E_DEG = RAIZ + "/Esclavo/src/modo_degradado.cpp";
  const std::string M_SEM = RAIZ + "/Maestro/src/semaforo.cpp";
  const std::string E_SEM = RAIZ + "/Esclavo/src/semaforo.cpp";
  const std::string MODOS = RAIZ + "/Maestro/include/modos.h";

  const unsigned long DEG_VERDE_SEG =
      leerNumero(M_DEG, R"(DEG_VERDE_SEG\s*=\s*(\d+))", "DEG_VERDE_SEG");
  const unsigned long DEG_DESPEJE_SEG =
      leerNumero(M_DEG, R"(DEG_DESPEJE_SEG\s*=\s*(\d+))", "DEG_DESPEJE_SEG");
  const unsigned long LIMITE_DURO_MS =
      leerNumero(M_DEG, R"(LIMITE_DURO_MS\s*=\s*(\d+))", "LIMITE_DURO_MS del Maestro");
  const unsigned long TOLERANCIA_DESFASE_S =
      leerNumero(M_DEG, R"(TOLERANCIA_DESFASE_S\s*=\s*(\d+))", "TOLERANCIA_DESFASE_S");
  // El del Esclavo esta escrito como producto: 48UL * 3600UL * 1000UL.
  const unsigned long LIMITE_SIN_SYNC_H_E =
      leerNumero(E_DEG, R"(LIMITE_SIN_SYNC_MS\s*=\s*(\d+)UL\s*\*\s*3600UL\s*\*\s*1000UL)",
                 "LIMITE_SIN_SYNC_MS del Esclavo");
  const unsigned long M_AMARILLO_MS =
      leerNumero(M_SEM, R"(estado\s*==\s*S_AMARILLO\s*&&\s*\(ahora\s*-\s*tCambio\s*>=\s*(\d+)\))",
                 "amarillo fijo del Maestro");
  const unsigned long E_AMARILLO_MS =
      leerNumero(E_SEM, R"(estado\s*==\s*S_AMARILLO\s*&&\s*\(ahora\s*-\s*tCambio\s*>=\s*(\d+)\))",
                 "amarillo fijo del Esclavo");

  // LA DERIVA POR DIA NO ES UNA CONSTANTE DEL FIRMWARE: VIVE EN UN COMENTARIO.
  // Se lee IGUAL del comentario, y con patron estricto, por dos motivos. Uno, para no
  // escribir a mano un numero que sostiene una desigualdad de seguridad. Dos, para que
  // el dia que alguien toque esa frase el arnes ABORTE en vez de seguir comparando
  // contra una cifra que el fuente ya no dice. Ver el hallazgo del bloque D.
  const unsigned long DERIVA_ENTERO =
      leerNumero(E_DEG, R"(derivan hasta ~(\d+),\d+ s/dia)", "deriva diaria (entero)");
  const unsigned long DERIVA_DECIMA =
      leerNumero(E_DEG, R"(derivan hasta ~\d+,(\d+) s/dia)", "deriva diaria (decima)");
  const double DERIVA_S_POR_DIA = (double)DERIVA_ENTERO + (double)DERIVA_DECIMA / 10.0;

  // El valor de MODO_DEGRADADO se lee del ENUM, contando su posicion. Escribir un 6
  // aqui seria un modelo a mano de la superficie del firmware: basta que alguien meta
  // un modo antes para que el arnes ponga al equipo en otro modo y siga en verde.
  {
    std::string txt = leerFuente(MODOS);
    std::smatch m;
    if (!std::regex_search(txt, m, std::regex(R"(enum\s+ModoSistema\s*\{([^}]*)\})"))) {
      abortar("no se pudo leer el enum ModoSistema de " + MODOS);
    }
    std::string cuerpo = m[1].str();
    // Fuera comentarios de linea, que llevan nombres de modo dentro.
    cuerpo = std::regex_replace(cuerpo, std::regex(R"(//[^\n]*)"), "");
    int idx = 0;
    std::regex ident(R"([A-Za-z_][A-Za-z0-9_]*)");
    for (std::sregex_iterator it(cuerpo.begin(), cuerpo.end(), ident), fin; it != fin; ++it, ++idx) {
      if (it->str() == "MODO_DEGRADADO") { MODO_DEGRADADO_V = idx; break; }
    }
    if (MODO_DEGRADADO_V < 0) abortar("MODO_DEGRADADO no aparece en el enum ModoSistema");
  }

  const unsigned long CICLO_S = 2UL * (DEG_VERDE_SEG + DEG_DESPEJE_SEG);

  std::printf("\n Constantes releidas del C++ real:\n");
  std::printf("   ciclo degradado: verde %lu s, despeje %lu s -> ciclo %lu s\n",
              DEG_VERDE_SEG, DEG_DESPEJE_SEG, CICLO_S);
  std::printf("   limite duro sin sync: %lu h (Maestro) / %lu h (Esclavo)\n",
              LIMITE_DURO_MS / 3600000UL, LIMITE_SIN_SYNC_H_E);
  std::printf("   tolerancia de desfase en la puerta: +-%lu s\n", TOLERANCIA_DESFASE_S);
  std::printf("   amarillo fijo: %lu ms (Maestro) / %lu ms (Esclavo)\n",
              M_AMARILLO_MS, E_AMARILLO_MS);
  std::printf("   deriva declarada: %.1f s/dia (leida del COMENTARIO, no de codigo)\n",
              DERIVA_S_POR_DIA);
  std::printf("   MODO_DEGRADADO = %d (posicion leida del enum, no escrita a mano)\n",
              MODO_DEGRADADO_V);

  // --- Guarda de mapeo de pines: comun/pines.h es UNO para las dos puntas ---
  {
    std::string pm = leerFuente(RAIZ + "/Maestro/include/pines.h");
    std::string pe = leerFuente(RAIZ + "/Esclavo/include/pines.h");
    const char* luces[] = { "ROJO1", "AMARILLO1", "VERDE1", "ROJO2", "AMARILLO2", "VERDE2" };
    for (const char* l : luces) {
      std::regex re(std::string("#define\\s+") + l + "\\s+(\\w+)");
      std::smatch a, b;
      if (!std::regex_search(pm, a, re) || !std::regex_search(pe, b, re)) {
        abortar(std::string("no se encuentra ") + l + " en algun pines.h real");
      }
      if (a[1].str() != b[1].str()) {
        abortar(std::string("el pin de ") + l + " DIFIERE entre puntas (" + a[1].str() +
                " vs " + b[1].str() + "). El sustituto comun lo estaria escondiendo");
      }
    }
  }

  MAESTRO.ruta = AQUI + "/build_deg/punta_maestro_deg.dll";
  MAESTRO.etiquetaEsperada = "MAESTRO";
  ESCLAVO.ruta = AQUI + "/build_deg/punta_esclavo_deg.dll";
  ESCLAVO.etiquetaEsperada = "ESCLAVO";
  MAESTRO.cargar();
  ESCLAVO.cargar();

  // =========================================================================
  std::printf("\n--- BLOQUE A: que el arnes SEA lo que dice ser -------------------\n");
  // Va PRIMERO. Todo lo que viene detras solo significa algo si son dos modulos, si el
  // detector sabe ver una violacion y si las dos puntas llegan de verdad a Degradado.

  comprobar(std::string(MAESTRO.nombre()) == "MAESTRO" &&
            std::string(ESCLAVO.nombre()) == "ESCLAVO" && MAESTRO.h != ESCLAVO.h,
            "A1: hay DOS modulos distintos cargados y cada uno se identifica como su "
            "punta (cargar dos veces el mismo daria un arnes midiendo una punta contra "
            "si misma)");

  comprobar(hayVerdeSimultaneo(true, true) &&
            !hayVerdeSimultaneo(true, false) &&
            !hayVerdeSimultaneo(false, true) &&
            !hayVerdeSimultaneo(false, false),
            "A2 (control negativo del detector): hayVerdeSimultaneo() dice SI ante dos "
            "verdes y NO ante los otros tres casos. Sin esto, los ceros de todo el "
            "barrido podrian ser los de un detector que no sabe encender");

  // La puerta del Maestro NIEGA antes de sincronizar. Es el control negativo de la
  // puerta: si aceptara siempre, el "entro en Degradado" de los bloques siguientes no
  // demostraria que el firmware comprueba nada.
  {
    arrancarLasDos();
    avanzar(3000);
    const long motivoSinNada = MAESTRO.orden("deg_evaluar");
    const long rechazoSinNada = ESCLAVO.orden("degradado_comprobar");
    comprobar(motivoSinNada != 0 && rechazoSinNada != 0,
              "A3 (control negativo de las dos puertas): recien arrancadas y sin "
              "sincronizar, el Maestro RECHAZA la entrada al Degradado (motivo " +
              std::to_string(motivoSinNada) + ", 0 seria aceptar) y el Esclavo tambien "
              "(rechazo " + std::to_string(rechazoSinNada) + ")");
  }

  // =========================================================================
  std::printf("\n--- BLOQUE B: las dos puntas en Degradado, con la MISMA hora -----\n");
  // El control POSITIVO, y es el que impide que todo lo demas sea una tapia: si
  // ninguna de las dos llegara a dar verde, "nunca hay verde simultaneo" seria cierto
  // y no mediria nada. Es la prueba muerta de N-51 aplicada a este arnes.

  const unsigned long OBSERVACION_MS = 20UL * 60UL * 1000UL;   // 20 min: >= 9 ciclos
  {
    Escenario e = prepararSincronizadas(15, 8, 0, 0);
    comprobar(e.configConfirmada && e.desfaseValido,
              "B1: por la radio del arnes, el coordinador REAL del Maestro y el "
              "despachador REAL del Esclavo completaron los tres intercambios de "
              "SFTY-23: el Esclavo acuso la configuracion del ciclo y devolvio una "
              "medida de desfase valida (" + std::to_string(e.desfaseMedido) + " s)");

    comprobar(e.motivoMaestro == 0 && e.rechazoEsclavo == 0,
              "B2 (control positivo de las dos puertas): con hora, configuracion "
              "acusada y desfase en tolerancia, LAS DOS puntas declaran que pueden "
              "entrar en Modo Degradado (MDG_OK y DEG_ACEPTADO)");

    entrarEnDegradadoLasDos(0);
    reiniciarObservacion();
    avanzar(OBSERVACION_MS);

    comprobar(g_ticksVerdeMaestro > 0 && g_ticksVerdeEsclavo > 0,
              "B3 (CONTROL POSITIVO, el que impide la tapia): en " +
              std::to_string(OBSERVACION_MS / 1000) + " s de Degradado con la radio "
              "CORTADA, el Maestro encendio verde en " +
              std::to_string(g_ticksVerdeMaestro) + " instantes y el Esclavo en " +
              std::to_string(g_ticksVerdeEsclavo) + ". Los dos ciclan por su propio "
              "reloj: sin esto, un cero de solapes no diria nada");

    comprobar(g_verdeSimultaneo == 0,
              "B4: con los dos relojes iguales, en los " + std::to_string(g_instantes) +
              " instantes observados NO hubo ni uno con verde en las dos puntas");

    comprobar(g_pegados == 0,
              "B5: y nunca se paso del verde de una punta al de la otra sin al menos un "
              "instante de todo-rojo entre medias");

    comprobar(g_enclavamientoRoto == 0 && g_talanqueraSinVerde == 0,
              "B6: en esos mismos instantes, ni rojo+verde a la vez en una misma cara "
              "(SFTY-2) ni pluma arriba sin verde fuera de S_FALLO (SFTY-28)");

    // N-96: los tres pines declarados y muertos. Se cuentan ESCRITURAS, no niveles: un
    // digitalWrite(pin, LOW) dejaria el nivel igual que un pin que nadie toca.
    const long tM_rojoPeaton = MAESTRO.orden("toques", ROJO_PEATON);
    const long tM_verdePeaton = MAESTRO.orden("toques", VERDE_PEATON);
    const long tM_buzzer = MAESTRO.orden("toques", BUZZER);
    const long tE_rojoPeaton = ESCLAVO.orden("toques", ROJO_PEATON);
    const long tE_verdePeaton = ESCLAVO.orden("toques", VERDE_PEATON);
    const long tE_buzzer = ESCLAVO.orden("toques", BUZZER);
    comprobar(tM_rojoPeaton == 0 && tM_verdePeaton == 0 && tM_buzzer == 0 &&
              tE_rojoPeaton == 0 && tE_verdePeaton == 0 && tE_buzzer == 0 &&
              MAESTRO.orden("toques", VERDE1) > 0 && MAESTRO.orden("toques", ROJO1) > 0 &&
              ESCLAVO.orden("toques", VERDE1) > 0 && ESCLAVO.orden("toques", ROJO1) > 0,
              "B7 (N-96): en Degradado, escribirPines() movio los SEIS pines vivos en "
              "las dos puntas y NO toco ni una vez ROJO_PEATON, VERDE_PEATON ni el "
              "BUZZER. Se cuentan escrituras, no niveles");
  }

  // =========================================================================
  std::printf("\n--- BLOQUE C: EL BARRIDO DEL DESFASE -----------------------------\n");
  // El numero que motiva este arnes. Se barre el desfase del RTC del Esclavo respecto
  // del Maestro en los DOS SENTIDOS, porque cual de las dos puntas adelanta es un
  // accidente del cristal y no una eleccion del diseno.
  //
  // La observacion de cada punto tiene que cubrir el todo-rojo de entrada MAS al menos
  // dos ciclos completos: un solape que solo ocurre en la segunda vuelta no puede
  // quedar fuera de la ventana.
  const unsigned long OBS_BARRIDO_MS = (DEG_DESPEJE_SEG + 3UL * CICLO_S + 30UL) * 1000UL;
  const long TOPE_BARRIDO = (long)CICLO_S;   // un ciclo entero: mas alla se repite

  long primerSolapePositivo = -1;   // Esclavo ADELANTADO
  long primerSolapeNegativo = -1;   // Esclavo ATRASADO
  unsigned long solapeMsPositivo = 0, solapeMsNegativo = 0;
  long verdesVistos = 0;

  std::printf("   (barriendo de -%ld a +%ld s, %lu s de observacion por punto,\n",
              TOPE_BARRIDO, TOPE_BARRIDO, OBS_BARRIDO_MS / 1000);
  std::printf("    muestreando cada %lu ms)\n", PASO_MS);

  // Se sigue barriendo TRES puntos mas alla del primer solape en cada sentido, y se
  // anota CUANTO dura. La duracion es el dato que distingue un solape estructural -de
  // segundos, el que el diseno teme- de uno de milisegundos, que sale de un retardo
  // del firmware y no de la geometria del ciclo. Sin ella, "rompe a los 30 s" y
  // "rompe a los 35 s" se leerian igual.
  for (long d = 1; d <= TOPE_BARRIDO; d++) {
    Medida m = correrConDesfase(d, OBS_BARRIDO_MS, 15, 8, 0, 0);
    if (m.verdeM > 0 && m.verdeE > 0) verdesVistos++;
    if (m.simultaneos > 0) {
      if (primerSolapePositivo < 0) {
        primerSolapePositivo = d;
        solapeMsPositivo = m.solapeMaxMs;
      }
      std::printf("    +%3ld s -> solape mas largo: %5lu ms\n", d, m.solapeMaxMs);
    }
    if (primerSolapePositivo >= 0 && d >= primerSolapePositivo + 3) break;
  }
  for (long d = 1; d <= TOPE_BARRIDO; d++) {
    Medida m = correrConDesfase(-d, OBS_BARRIDO_MS, 15, 8, 0, 0);
    if (m.verdeM > 0 && m.verdeE > 0) verdesVistos++;
    if (m.simultaneos > 0) {
      if (primerSolapeNegativo < 0) {
        primerSolapeNegativo = d;
        solapeMsNegativo = m.solapeMaxMs;
      }
      std::printf("    -%3ld s -> solape mas largo: %5lu ms\n", d, m.solapeMaxMs);
    }
    if (primerSolapeNegativo >= 0 && d >= primerSolapeNegativo + 3) break;
  }

  comprobar(verdesVistos > 0,
            "C0 (control positivo del barrido): en " + std::to_string(verdesVistos) +
            " de los puntos barridos las DOS puntas llegaron a encender verde. Un "
            "barrido en el que nadie da verde no puede encontrar un solape");

  comprobar(primerSolapePositivo > 0 && primerSolapeNegativo > 0,
            "C1: el barrido ENCUENTRA el desfase que rompe el cruce en los dos "
            "sentidos -adelantado a los " + std::to_string(primerSolapePositivo) +
            " s, atrasado a los " + std::to_string(primerSolapeNegativo) + " s-. Un "
            "barrido que no encontrara nunca el fallo no estaria midiendo un margen: "
            "estaria midiendo una tapia");

  const long MARGEN_MEDIDO = (primerSolapePositivo < primerSolapeNegativo)
                             ? primerSolapePositivo : primerSolapeNegativo;
  const long DESFASE_QUE_AGUANTA = MARGEN_MEDIDO - 1;

  // EL NUMERO. Lo que el cruce aguanta es el PEOR de los dos sentidos, menos uno: el
  // ultimo desfase que todavia NO rompio.
  std::printf("\n   >>> DESFASE QUE AGUANTA: %ld s (el ultimo que NO rompe).\n",
              DESFASE_QUE_AGUANTA);
  std::printf("       Rompe a los %ld s con el Esclavo adelantado (solape de %lu ms) y a\n",
              primerSolapePositivo, solapeMsPositivo);
  std::printf("       los %ld s con el Esclavo atrasado (solape de %lu ms).\n",
              primerSolapeNegativo, solapeMsNegativo);

  // --- Por que los dos sentidos NO son simetricos, y de donde sale el medio segundo -
  //
  // Las dos primeras roturas duran MENOS DE UN SEGUNDO, y sus dos duraciones SUMAN un
  // segundo. No es casualidad: hay un unico residuo sub-segundo entre los dos relojes
  // -la hora viaja por radio en SEGUNDOS ENTEROS (CMD_HORA_S lleva reloj_segundo()) y
  // el Esclavo la aplica al recibirla-, y ese residuo empuja una frontera hacia dentro
  // exactamente lo que retira de la otra.
  //
  // Quitado el residuo, las dos fronteras ESTRUCTURALES son:
  //     Esclavo atrasado   -> el despeje ampliado
  //     Esclavo adelantado -> el despeje MAS el amarillo del Esclavo, que se come el
  //                           principio de su verde y protege solo en ese sentido
  const long SUMA_RESIDUOS = (long)(solapeMsPositivo + solapeMsNegativo);
  comprobar(SUMA_RESIDUOS >= 1000 - 2 * (long)PASO_MS &&
            SUMA_RESIDUOS <= 1000 + 2 * (long)PASO_MS,
            "C2: los solapes de las dos primeras roturas duran " +
            std::to_string(solapeMsPositivo) + " ms y " + std::to_string(solapeMsNegativo) +
            " ms, y SUMAN " + std::to_string(SUMA_RESIDUOS) + " ms: un unico segundo. Es "
            "el residuo sub-segundo de la sincronizacion -la hora viaja en segundos "
            "enteros y el Esclavo la aplica al recibirla-, y demuestra que las dos "
            "fronteras medidas son la MISMA geometria vista desde los dos lados, no dos "
            "accidentes");

  comprobar(primerSolapeNegativo == (long)DEG_DESPEJE_SEG,
            "C3: con el Esclavo ATRASADO -el sentido malo- el cruce rompe EXACTAMENTE en "
            "el despeje ampliado (" + std::to_string(DEG_DESPEJE_SEG) + " s). El colchon "
            "que el diseno declara es real y no hay ni un segundo de mas: esta linea ata "
            "la constante del ciclo con lo que el firmware hace en los pines, y se mueve "
            "con ella si alguien la toca");

  comprobar(primerSolapePositivo - primerSolapeNegativo ==
                (long)(E_AMARILLO_MS / 1000UL) + 1,
            "C4: con el Esclavo ADELANTADO aguanta " +
            std::to_string(primerSolapePositivo - primerSolapeNegativo) + " s mas, que "
            "son los " + std::to_string(E_AMARILLO_MS / 1000UL) + " s de amarillo con "
            "que esa punta empieza su verde mas el segundo del residuo. EL MARGEN REAL "
            "ES EL DEL SENTIDO MALO: cual de los dos cristales adelanta no lo elige "
            "nadie");

  nota("C4.bis: costura_02_fase_ciclo.py barre el desfase en UN SOLO SENTIDO y publica "
       "los " + std::to_string(primerSolapePositivo) + " s del sentido bueno como 'el "
       "margen real contra la deriva entre relojes... el colchon que justifica el limite "
       "de 48 h'. Medido aqui sobre el C++ de las dos puntas, el margen es " +
       std::to_string(DESFASE_QUE_AGUANTA) + " s: " +
       std::to_string(primerSolapePositivo - 1 - DESFASE_QUE_AGUANTA) + " s menos.");

  // =========================================================================
  std::printf("\n--- BLOQUE D: EL MARGEN, contra lo que el equipo puede derivar ---\n");
  //
  // La desigualdad completa, con sus dos sumandos:
  //
  //   deriva posible = (lo que la puerta admite en el instante de entrar)
  //                  + (lo que los dos cristales se separan durante el limite duro)
  //
  // El primer sumando NO es cero y no puede serlo: la puerta acepta hasta
  // TOLERANCIA_DESFASE_S de desfase medido, asi que el modo puede arrancar ya con ese
  // error encima. El segundo es la deriva declarada por el limite duro completo.
  const double HORAS_LIMITE = (double)(LIMITE_DURO_MS / 3600000UL);
  const double DERIVA_LIMITE_S = DERIVA_S_POR_DIA * HORAS_LIMITE / 24.0;
  const double DERIVA_POSIBLE_S = DERIVA_LIMITE_S + (double)TOLERANCIA_DESFASE_S;
  const double MARGEN_S = (double)DESFASE_QUE_AGUANTA - DERIVA_POSIBLE_S;

  std::printf("   deriva de los cristales en %.0f h : %.1f s\n", HORAS_LIMITE, DERIVA_LIMITE_S);
  std::printf("   error admitido por la puerta      : %lu s\n", TOLERANCIA_DESFASE_S);
  std::printf("   DERIVA POSIBLE TOTAL              : %.1f s\n", DERIVA_POSIBLE_S);
  std::printf("   DESFASE QUE AGUANTA (medido)      : %ld s\n", DESFASE_QUE_AGUANTA);
  std::printf("   MARGEN                            : %.1f s  (factor %.2f)\n",
              MARGEN_S, (double)DESFASE_QUE_AGUANTA / DERIVA_POSIBLE_S);

  comprobar(MARGEN_S > 0.0,
            "D1: el desfase que el cruce aguanta (" + std::to_string(DESFASE_QUE_AGUANTA) +
            " s, MEDIDO sobre el C++ real de las dos puntas) es MAYOR que todo lo que "
            "el equipo puede acumular dentro de su limite duro: " +
            std::to_string((int)(DERIVA_LIMITE_S + 0.5)) + " s de deriva entre "
            "cristales mas los " + std::to_string(TOLERANCIA_DESFASE_S) + " s que la "
            "propia puerta admite al entrar");

  // El limite duro es el que hace verdadera esa desigualdad. Se comprueba que las dos
  // puntas lo tengan IGUAL: si una se rindiera mas tarde que la otra, la que sigue
  // ciclando lo haria contra una punta ya en ambar y despues sola.
  comprobar(LIMITE_DURO_MS / 3600000UL == LIMITE_SIN_SYNC_H_E,
            "D2: las dos puntas se rinden al MISMO plazo (" +
            std::to_string(LIMITE_DURO_MS / 3600000UL) + " h). Dos plazos distintos "
            "dejarian a una ciclando por reloj contra otra ya en ambar");

  // Y el plazo tiene que caber en el margen: es la desigualdad de N-71 -una constante
  // que es el TECHO de otra-, aqui recalculada desde el C++ en vez de vivir en prosa.
  const double HORAS_QUE_CABEN = (double)(DESFASE_QUE_AGUANTA - (long)TOLERANCIA_DESFASE_S) *
                                 24.0 / DERIVA_S_POR_DIA;
  std::printf("   con ese margen, el limite duro podria llegar a %.1f h (hoy son %.0f)\n",
              HORAS_QUE_CABEN, HORAS_LIMITE);
  comprobar(HORAS_QUE_CABEN > HORAS_LIMITE,
            "D3 (la desigualdad de N-71, recalculada y no escrita en prosa): el limite "
            "duro de " + std::to_string((int)HORAS_LIMITE) + " h cabe dentro del "
            "margen medido, que aguantaria hasta " +
            std::to_string((int)HORAS_QUE_CABEN) + " h. Si alguien subiera el limite "
            "por encima de esa cifra sin tocar el despeje, esta linea FALLA");

  nota("D4: la deriva de " + std::to_string(DERIVA_S_POR_DIA).substr(0, 4) + " s/dia con "
       "la que se hace toda esta cuenta NO es una constante del firmware: vive en un "
       "COMENTARIO de Maestro/src/modo_degradado.cpp y de Esclavo/src/modo_degradado.cpp. "
       "Este arnes la lee de ahi con patron estricto -aborta si cambia la frase-, pero "
       "un comentario no falla cuando alguien cambia un cristal: se queda describiendo "
       "un equipo que ya no existe, con la autoridad de una cuenta hecha (N-71).");

  nota("D5: la puerta admite +-" + std::to_string(TOLERANCIA_DESFASE_S) + " s de desfase "
       "MEDIDO, y CMD_DELTA lleva solo el segundo (0..59), asi que todo multiplo de 60 s "
       "se lee como cero. Un desfase de 60 s -por encima del margen medido- pasaria la "
       "puerta. Lo que lo impide no es la tolerancia sino la FRESCURA exigida a la sync "
       "(2 h, en las que la deriva es de decimas). Esta anotado aqui porque la cuenta de "
       "arriba se apoya en esa frescura y no en el numero.");

  // =========================================================================
  std::printf("\n==============================================================\n");
  std::printf(" RESULTADO: %d/%d comprobaciones OK\n", total - fallos, total);
  std::printf("==============================================================\n");
  std::printf(" EL NUMERO: el cruce aguanta %ld s de desfase entre relojes.\n",
              DESFASE_QUE_AGUANTA);
  std::printf(" El equipo puede acumular %.1f s dentro de su limite de %.0f h.\n",
              DERIVA_POSIBLE_S, HORAS_LIMITE);
  std::printf(" MARGEN: %.1f s (factor %.2f).\n", MARGEN_S,
              (double)DESFASE_QUE_AGUANTA / DERIVA_POSIBLE_S);
  std::printf(" %lu tramas entregadas antes de cortar la radio.\n", g_tramasEntregadas);
  std::printf(" Medido sobre el C++ REAL de las DOS puntas -modo_degradado.cpp de las\n");
  std::printf(" dos incluido- ejecutandose en el mismo proceso, cada una con su reloj,\n");
  std::printf(" y observado sobre lo que semaforo.cpp escribio en los pines.\n");

  MAESTRO.descargar();
  ESCLAVO.descargar();
  return fallos == 0 ? 0 : 1;
}
