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
//   - ~~el RTC es un modelo de PERIFERICO en las dos puntas~~ -> DESDE EL 11/09 (D-21 (1))
//     reloj.cpp ENTRA REAL en las dos puntas: lo que se sustituye es el silicio (STM32RTC.h
//     y el HAL del LSE y del contador, en reloj_real/, con el mismo fichero para las dos).
//     Con eso la hora caduca de verdad, y por eso cada punta tiene ahora SU ESP32: la orden
//     "siembra_esp32" transcribe la rama CMD:HORA_ESP32 de bluetooth.cpp, que no se compila.
//     EN LOS BLOQUES B..E EL ESP32 ES UN ECO: siembra cada SIEMBRA_INTERVALO_MS la MISMA hora
//     que la punta ya tiene, en su frontera de segundo. Esos bloques miden la GEOMETRIA del
//     ciclo -el barrido, el salto de D-26 (4)- y no la deriva; sin siembras, la caducidad de
//     D-21 (1) los mandaria a ambar a los cinco minutos, y con un DS3231 "de verdad" el
//     residuo sub-segundo que miden C2..C4 dejaria de ser el de la radio. El DS3231 con su
//     propia hora y el HSI derivando son del BLOQUE F, que es el que los mide.
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

// La POSICION de un nombre dentro de un enum del C++ real, contando identificadores sin
// comentarios -el mismo metodo con que main() lee MODO_DEGRADADO-. Escribir el numero aqui
// seria un modelo a mano de la superficie del firmware. Solo sabe enums sin '=': si alguno
// lleva valor explicito, ABORTA en vez de contar mal.
static long posicionEnEnum(const std::string& ruta, const std::string& nombreEnum,
                           const std::string& ident) {
  std::string txt = leerFuente(ruta);
  std::smatch m;
  if (!std::regex_search(txt, m, std::regex("enum\\s+" + nombreEnum + "\\s*(?::\\s*\\w+\\s*)?\\{([^}]*)\\}"))) {
    abortar("no se pudo leer el enum " + nombreEnum + " de " + ruta);
  }
  std::string cuerpo = std::regex_replace(m[1].str(), std::regex(R"(//[^\n]*)"), "");
  if (cuerpo.find('=') != std::string::npos) {
    abortar("el enum " + nombreEnum + " de " + ruta + " lleva valores explicitos: contar su "
            "posicion daria otro numero");
  }
  long idx = 0;
  std::regex id(R"([A-Za-z_][A-Za-z0-9_]*)");
  for (std::sregex_iterator it(cuerpo.begin(), cuerpo.end(), id), fin; it != fin; ++it, ++idx) {
    if (it->str() == ident) return idx;
  }
  abortar(ident + " no aparece en el enum " + nombreEnum + " de " + ruta);
  return -1;
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
// El instante del banco en que el Esclavo recibio su ultima trama: su main.cpp REAL llama a
// reloj_notarRadio() en ESE tick, y de ahi cuenta reloj_radioManda() los 25 s (bloque F5).
static unsigned long g_tUltimaEntregaEsclavo = 0;

// ---------------------------------------------------------------------------
// EL ESP32 DE CADA POSTE (11/09, D-21 (1)). Cada SIEMBRA_INTERVALO_MS -leido del contrato.h
// del ESP32, el otro binario- le manda a SU punta la linea CMD:HORA_ESP32. Dos modos:
//
//   ECO      el DS3231 dice la hora que la punta ya tiene: la mantiene fresca sin moverla.
//            Bloques B..E. Ver la cabecera.
//   DS3231   la hora del reloj con pila del poste: la del banco desde un origen comun, mas
//            lo que ese DS3231 difiera del otro. Bloque F.
//
// Con g_esp32Vivo[p] a false el cable J17 de ese poste esta mudo: no llega nada.
// ---------------------------------------------------------------------------
static bool g_esp32Vivo[2] = {false, false};
static bool g_esp32Ds3231 = false;
static unsigned long g_proxSiembra[2] = {0, 0};
static unsigned long g_cadenciaSiembraMs = 0;   // se lee del contrato.h del ESP32
static long g_ds3231OffsetS[2] = {0, 0};
static unsigned long g_tRefDs = 0;              // el origen comun de los dos DS3231
static long g_segRefDs = 0;
static long g_diaRefDs = 15;
static long g_siembrasSembradas[2] = {0, 0};
static long g_siembrasIgnoradas[2] = {0, 0};
static unsigned long g_tUltimaSiembraBuena[2] = {0, 0};

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
// La hora del DS3231 de un poste en el instante del banco, empaquetada como la pide la orden
// "siembra_esp32": dia * 86400 + segundos del dia. Segundos ENTEROS, truncados: es lo que el
// ESP32 compone -el segundo en curso se pierde, el RESIDUO_SIEMBRA_S de esp32_13-.
static long horaDs3231(int poste) {
  long s = g_segRefDs + (long)((g_t - g_tRefDs) / 1000UL) + g_ds3231OffsetS[poste];
  long dia = g_diaRefDs;
  while (s < 0) { s += 86400L; dia -= 1; }
  while (s >= 86400L) { s -= 86400L; dia += 1; }
  while (dia < 1) dia += 31;
  while (dia > 31) dia -= 31;
  return dia * 86400L + s;
}

static void siembraDelEsp32(int poste) {
  Punta& p = (poste == 0) ? MAESTRO : ESCLAVO;
  const long r = g_esp32Ds3231 ? p.orden("siembra_esp32", horaDs3231(poste))
                               : p.orden("siembra_esp32_eco");
  if (r == 1) { g_siembrasSembradas[poste]++; g_tUltimaSiembraBuena[poste] = g_t; }
  if (r == 2) g_siembrasIgnoradas[poste]++;
}

static void unTick() {
  for (size_t i = 0; i < g_aire.size();) {
    if (g_aire[i].tEntrega <= g_t) {
      Punta& d = (g_aire[i].destino == 0) ? MAESTRO : ESCLAVO;
      d.rx(g_aire[i].trama);
      g_tramasEntregadas++;
      if (g_aire[i].destino == 1) g_tUltimaEntregaEsclavo = g_t;
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

  // El ESP32 de cada poste, DESPUES de la vuelta: la linea queda atendida con el millis() de
  // este instante y la luz la decide la vuelta siguiente, como en la tarjeta.
  for (int poste = 0; poste < 2; poste++) {
    if (g_esp32Vivo[poste] && g_cadenciaSiembraMs > 0 && g_t >= g_proxSiembra[poste]) {
      siembraDelEsp32(poste);
      g_proxSiembra[poste] += g_cadenciaSiembraMs;
    }
  }

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

// Avanza hasta que se cumpla una condicion SOBRE LOS PINES o lo que la punta contesta, o se
// rinde. Devuelve si llego: un escenario que no llega no mide nada, y se dice.
static bool esperarCond(bool (*cond)(), unsigned long maxMs) {
  unsigned long hecho = 0;
  while (!cond() && hecho < maxMs) { unTick(); hecho += PASO_MS; }
  return cond();
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
  // Los ESP32 arrancan callados: cada escenario los enciende cuando le toca (D-21 (1)).
  g_esp32Vivo[0] = g_esp32Vivo[1] = false;
  g_esp32Ds3231 = false;
  g_ds3231OffsetS[0] = g_ds3231OffsetS[1] = 0;
  g_siembrasSembradas[0] = g_siembrasSembradas[1] = 0;
  g_siembrasIgnoradas[0] = g_siembrasIgnoradas[1] = 0;
  MAESTRO.descargar(); MAESTRO.cargar();
  ESCLAVO.descargar(); ESCLAVO.cargar();
  MAESTRO.arrancar();
  ESCLAVO.arrancar();
  g_t += DELAY_ARRANQUE_MS;

  // UNA VUELTA ANTES DE LA PRIMERA ORDEN (11/09, D-21 (1)). Recien cargada, cada DLL tiene
  // millis() = DELAY_ARRANQUE_MS, no el reloj del banco: el primer tick lo pone. Hasta hoy
  // la hora se sembraba ANTES de ese tick, con millis() = 2000, y en el tick siguiente
  // saltaba hacia delante todo lo que el banco llevara corrido; el modelo del RTC no lo
  // notaba porque el salto ocurria antes de sincronizar y la radio lo copiaba al Esclavo.
  // Con el reloj.cpp real esa base nace con la edad del banco entero y CADUCA al instante:
  // la puerta del Maestro la rechazaba y el barrido del bloque C medio un cruce con UNA
  // sola punta en Degradado. Medido, no supuesto: 0 de los puntos del barrido con verde en
  // las dos. El arnes estaba sembrando con un reloj que la tarjeta no tiene.
  unTick();
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

  // D-21 (1): los dos ESP32 empiezan a sembrar, en ECO (ver la cabecera). NO ANTES: una
  // siembra del Maestro con la radio viva propaga la hora al Esclavo (D-26 (2)) y cambiaria
  // el residuo sub-segundo que miden C2..C4. El del Esclavo, detras de los 25 s de silencio
  // que siguen al corte de entrarEnDegradadoLasDos(): con la radio mandando la ignoraria.
  // Las dos primeras caen muy por dentro de HORA_CADUCA_MS desde la ultima hora que cada
  // punta recibio; despues, una cada SIEMBRA_INTERVALO_MS.
  g_esp32Ds3231 = false;
  g_esp32Vivo[0] = g_esp32Vivo[1] = true;
  g_proxSiembra[0] = g_t + 5000UL;
  g_proxSiembra[1] = g_t + 40000UL;
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
// BLOQUE F - LOS DOS DS3231 CON SU PROPIA HORA (D-21 (1)).
//
// El origen comun es la hora que el Maestro tiene AHORA, en su ultima frontera de segundo, en
// tiempo del banco: desde ahi, cada DS3231 cuenta el banco -mas su diferencia con el otro,
// g_ds3231OffsetS-. Se llama con el HSI del Maestro todavia sin deriva (su millis() es el del
// banco) y justo tras prepararSincronizadas(), con las dos puntas en la misma hora.
//
// Las ordenes se atienden con el millis() de la ULTIMA vuelta, que es g_t - PASO_MS: unTick()
// avanza g_t al terminar. Por eso la frontera se situa desde ahi.
static void activarDs3231(unsigned long primeraM, unsigned long primeraE) {
  const long fase = MAESTRO.orden("fase_subsegundo");
  if (fase < 0) abortar("activarDs3231: el Maestro no tiene hora de la que partir");
  g_tRefDs = (g_t - PASO_MS) - (unsigned long)fase;
  g_segRefDs = MAESTRO.orden("segundos_del_dia");
  g_diaRefDs = 15;
  g_esp32Ds3231 = true;
  g_esp32Vivo[0] = g_esp32Vivo[1] = true;
  g_proxSiembra[0] = g_t + primeraM;
  g_proxSiembra[1] = g_t + primeraE;
}

// La diferencia, en segundos y por el camino corto del dia, entre la hora de una punta y la
// del DS3231 de su poste. Positiva: la punta va adelantada.
static long desvioContraDs3231(Punta& p, int poste) {
  long d = p.orden("segundos_del_dia") - (horaDs3231(poste) % 86400L);
  while (d > 43200L) d -= 86400L;
  while (d < -43200L) d += 86400L;
  return d;
}

// Un borde de D-26 (4) sobre UNA punta, posicionado POR LOS PINES -el orquestador no calcula
// fases-. Hacia delante, en el despeje que PRECEDE a su verde (20 s dentro del despeje que
// sigue al verde de la otra); hacia atras, en el despeje que SIGUE a su verde (5 s dentro).
// Desde ahi un salto de J segundos cae DENTRO de su propio verde: si la regla lo deja pasar
// directo, la punta enciende -verde, o el ambar con que el Esclavo abre el suyo- en los
// 3 s siguientes; si lo manda a rojo, no, y el firmware lo dice con su $EVENT.
//
// LA FASE SUB-SEGUNDO SE CONTROLA, Y SE ESCRIBE POR QUE: saltoDeHora() mide el salto contra
// lo que corrio millis() desde la vuelta anterior, en segundos enteros. Si la frontera de
// segundo de la punta cae entre esa vuelta y la siguiente, el salto medido sale UNO MAS que
// el aplicado. Con la fase a 900 ms o menos, la vuelta siguiente -50 ms despues- no cruza la
// frontera y el salto medido es EXACTAMENTE J: el borde se mide donde esta, no un segundo al
// lado. El error del truncado lo cubre el -1 de SALTO_SIN_ROJO_MAX_S, y ese es otro borde.
struct Borde {
  bool listo = false;
  long eventos = 0;        // $EVENT SALTO_DE_HORA_POR_ROJO emitidos por el salto
  bool enciende = false;   // la punta encendio (verde o ambar de transicion) tras el salto
  unsigned long simultaneos = 0;
};

static Borde probarBorde(bool esMaestro, long J, unsigned long esperaMaxMs) {
  prepararSincronizadas(15, 8, 0, 0);
  entrarEnDegradadoLasDos(0);
  Borde b;
  bool ok;
  if (esMaestro) {
    if (J > 0) {
      ok = esperarCond([]() { return ESCLAVO.verde(); }, esperaMaxMs) &&
           esperarCond([]() { return !ESCLAVO.verde(); }, esperaMaxMs);
      if (ok) avanzar(20000);
    } else {
      ok = esperarCond([]() { return MAESTRO.verde(); }, esperaMaxMs) &&
           esperarCond([]() { return !MAESTRO.verde(); }, esperaMaxMs);
      if (ok) avanzar(5000);
    }
  } else {
    if (J > 0) {
      ok = esperarCond([]() { return MAESTRO.verde(); }, esperaMaxMs) &&
           esperarCond([]() { return !MAESTRO.verde(); }, esperaMaxMs);
      if (ok) avanzar(20000);
    } else {
      ok = esperarCond([]() { return ESCLAVO.verde(); }, esperaMaxMs) &&
           esperarCond([]() { return !ESCLAVO.verde(); }, esperaMaxMs);
      if (ok) avanzar(5000);
    }
  }
  Punta& p = esMaestro ? MAESTRO : ESCLAVO;
  for (int i = 0; ok && i < 40 && p.orden("fase_subsegundo") > 900; i++) unTick();
  if (!ok || p.orden("fase_subsegundo") > 900) return b;
  b.listo = true;
  const long ev0 = p.orden("eventos_salto_rojo");
  if (p.orden("desviar_rtc", J) != 1) { b.listo = false; return b; }
  for (unsigned long t = 0; t < 3000; t += PASO_MS) {
    unTick();
    if (p.verde() || (!esMaestro && p.ambar())) b.enciende = true;
    if (MAESTRO.verde() && ESCLAVO.verde()) b.simultaneos++;
  }
  b.eventos = p.orden("eventos_salto_rojo") - ev0;
  return b;
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

  // D-21 (1): la cadencia del ESP32 se lee de SU contrato -otro binario-, y el silencio que
  // define "sin radio" del protocolo.h del Esclavo. Sin valor por defecto.
  g_cadenciaSiembraMs = leerNumero(RAIZ + "/ESP32_Expansion/include/contrato.h",
                                   R"(#define\s+SIEMBRA_INTERVALO_MS\s+(\d+)UL)",
                                   "SIEMBRA_INTERVALO_MS del ESP32");
  const unsigned long SFTY6_SILENCIO_MS_E =
      leerNumero(RAIZ + "/Esclavo/include/protocolo.h",
                 R"(#define\s+SFTY6_SILENCIO_MS\s+(\d+)UL)", "SFTY6_SILENCIO_MS del Esclavo");
  const long MDG_FALTA_HORA_V =
      posicionEnEnum(RAIZ + "/Maestro/include/modo_degradado.h", "MotivoDegradado", "MDG_FALTA_HORA");
  const long MDG_OK_V =
      posicionEnEnum(RAIZ + "/Maestro/include/modo_degradado.h", "MotivoDegradado", "MDG_OK");
  const long DEG_ACEPTADO_V =
      posicionEnEnum(RAIZ + "/Esclavo/include/modo_degradado.h", "RechazoDegradado", "DEG_ACEPTADO");
  const long DEG_RECHAZO_SIN_HORA_V =
      posicionEnEnum(RAIZ + "/Esclavo/include/modo_degradado.h", "RechazoDegradado",
                     "DEG_RECHAZO_SIN_HORA");
  const long DEG_RENDIDO_V =
      posicionEnEnum(RAIZ + "/Esclavo/include/modo_degradado.h", "EstadoDegradado", "DEG_RENDIDO");
  const long S_FALLO_V =
      posicionEnEnum(RAIZ + "/Maestro/include/semaforo.h", "EstadoSemaforo", "S_FALLO");
  const long S_FALLO_E =
      posicionEnEnum(RAIZ + "/Esclavo/include/semaforo.h", "EstadoSemaforo", "S_FALLO");
  const long S_AMARILLO_E =
      posicionEnEnum(RAIZ + "/Esclavo/include/semaforo.h", "EstadoSemaforo", "S_AMARILLO");
  (void)MDG_OK_V;

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
  std::printf("\n--- BLOQUE E: D-26 (4), EL SALTO DE HORA PASA POR ROJO -----------\n");
  //
  // Desde D-26 cada punta se re-siembra del DS3231 de su ESP32 cada ~5 min tambien en
  // Degradado, y la fase sale de la hora: una siembra MUEVE la fase de golpe. Aqui se salta
  // la hora de UNA punta con las dos ciclando y se mira lo que escribieron los pines.
  //
  // EL ESCENARIO ES EL PELIGROSO A PROPOSITO: se espera a que la OTRA punta este en su
  // verde y se salta un ciclo de verde + despeje, que deja a la que salta en la MISMA
  // posicion de SU verde. Sin la regla, esa punta enciende verde con la otra en verde. El
  // orquestador no calcula ninguna fase: el instante lo da el pin de la otra punta, y el
  // tamano del salto sale de las dos constantes releidas del C++.
  //
  // LA VENTANA es el despeje menos dos segundos: cubre el resto del verde de la otra punta
  // -que empezo hace un instante por su ambar- y no llega al final del todo-rojo de la
  // regla. Mas alla, las dos puntas quedan desfasadas un ciclo de verde + despeje, que es
  // mas de lo que el cruce aguanta (bloque C): ese solape posterior es del salto, no de la
  // regla, y por eso no se observa.
  //
  // Y SU CONTROL, que es lo que impide la tapia: un salto PEQUENO -2 s, muy por debajo del
  // margen y del orden de lo que deriva el HSI en una cadencia- NO manda a rojo. Una regla
  // que mandara a rojo cualquier salto pasaria las dos lineas grandes igual de bien y
  // pararia el cruce en cada siembra.
  {
    const long SALTO_GRANDE = (long)(DEG_VERDE_SEG + DEG_DESPEJE_SEG);
    const unsigned long VENTANA_MS = (DEG_DESPEJE_SEG - 2UL) * 1000UL;
    const unsigned long ESPERA_MAX_MS = 3UL * CICLO_S * 1000UL;

    // Espera hasta que se cumpla la condicion sobre los pines, o se rinde. Devuelve si
    // llego: un escenario que no llega no mide nada y se dice.
    auto esperarPines = [&](bool (*cond)(), unsigned long maxMs) {
      unsigned long hecho = 0;
      while (!cond() && hecho < maxMs) { unTick(); hecho += PASO_MS; }
      return cond();
    };

    // --- E0: main.cpp REAL le cuenta a reloj.cpp cada trama de radio (D-26 (3)) -------
    //
    // 11/09 - SE MUDA DE PREGUNTA, NO SE RELAJA (CLAUDE.md 9). Contaba las llamadas a
    // reloj_notarRadio() en el doble del adaptador; con el reloj.cpp REAL no hay doble que
    // cuente, y se pregunta a lo que esas llamadas ALIMENTAN: reloj_radioManda() real, que
    // solo dice que si con la hora de radio puesta Y una trama oida en SFTY6_SILENCIO_MS. Es
    // mas exigente que la cuenta: una llamada que existiera y no marcara el instante daria
    // "N tramas" y aqui da 0.
    prepararSincronizadas(15, 8, 0, 0);
    const long radioManda = ESCLAVO.orden("radio_manda");
    comprobar(radioManda == 1,
              "E0 (D-26 (3)): mientras la radio del arnes estaba viva, el reloj.cpp REAL del "
              "Esclavo dice que la hora la MANDA LA RADIO (reloj_radioManda() = " +
              std::to_string(radioManda) + "): el main.cpp real le aviso de las tramas. Sin "
              "eso, 'sin radio' seria cierto siempre y la hora del ESP32 pisaria la del "
              "Maestro con la radio sana");

    // --- E1: el MAESTRO salta hacia su verde con el Esclavo en verde --------------------
    entrarEnDegradadoLasDos(0);
    const bool e1Listo = esperarPines(
        []() { return ESCLAVO.verde() && !MAESTRO.verde(); }, ESPERA_MAX_MS);
    unsigned long e1VerdeM = 0, e1Simul = 0, e1VerdeE = 0;
    if (e1Listo) {
      MAESTRO.orden("desviar_rtc", SALTO_GRANDE);
      for (unsigned long t = 0; t < VENTANA_MS; t += PASO_MS) {
        unTick();
        if (MAESTRO.verde()) e1VerdeM++;
        if (ESCLAVO.verde()) e1VerdeE++;
        if (MAESTRO.verde() && ESCLAVO.verde()) e1Simul++;
      }
    }
    comprobar(e1Listo && e1VerdeE > 0,
              "E1.0 (el escenario es el peligroso): el Esclavo estaba en verde cuando el "
              "Maestro salto " + std::to_string(SALTO_GRANDE) + " s, y siguio en verde " +
              std::to_string(e1VerdeE * PASO_MS) + " ms de la ventana");
    comprobar(e1Listo && e1VerdeM == 0 && e1Simul == 0,
              "E1 (D-26 (4)): el Maestro salto " + std::to_string(SALTO_GRANDE) + " s -de la "
              "fase del verde del Esclavo a la de su propio verde- y NO encendio verde en los " +
              std::to_string(VENTANA_MS / 1000) + " s siguientes (" +
              std::to_string(e1VerdeM) + " instantes en verde, " + std::to_string(e1Simul) +
              " con las dos en verde): el salto paso por rojo en vez de dar el verde en la "
              "misma vuelta");

    // --- E2: y un salto PEQUENO no lo manda a rojo (el control de E1) -------------------
    prepararSincronizadas(15, 8, 0, 0);
    entrarEnDegradadoLasDos(0);
    const bool e2Listo = esperarPines(
        []() { return MAESTRO.verde() && !ESCLAVO.verde(); }, ESPERA_MAX_MS);
    unsigned long e2VerdeM = 0, e2Ticks = 0;
    if (e2Listo) {
      avanzar(3000);                        // que no este en el borde de su verde
      MAESTRO.orden("desviar_rtc", 2);
      for (unsigned long t = 0; t < 5000; t += PASO_MS) {
        unTick();
        e2Ticks++;
        if (MAESTRO.verde()) e2VerdeM++;
      }
    }
    comprobar(e2Listo && e2VerdeM == e2Ticks,
              "E2 (control de E1): con el Maestro en SU verde, un salto de 2 s -una siembra "
              "normal- lo deja en verde los 5 s siguientes (" + std::to_string(e2VerdeM) +
              " de " + std::to_string(e2Ticks) + " instantes): la regla no manda a rojo "
              "cualquier salto, solo el que pasa del margen");

    // --- E3: el ESCLAVO salta hacia su verde con el Maestro en verde -------------------
    prepararSincronizadas(15, 8, 0, 0);
    entrarEnDegradadoLasDos(0);
    const bool e3Listo = esperarPines(
        []() { return MAESTRO.verde() && !ESCLAVO.verde(); }, ESPERA_MAX_MS);
    unsigned long e3VerdeE = 0, e3AmbarE = 0, e3Simul = 0, e3VerdeM = 0;
    if (e3Listo) {
      ESCLAVO.orden("desviar_rtc", SALTO_GRANDE);
      for (unsigned long t = 0; t < VENTANA_MS; t += PASO_MS) {
        unTick();
        if (ESCLAVO.verde()) e3VerdeE++;
        if (ESCLAVO.ambar()) e3AmbarE++;
        if (MAESTRO.verde()) e3VerdeM++;
        if (MAESTRO.verde() && ESCLAVO.verde()) e3Simul++;
      }
    }
    comprobar(e3Listo && e3VerdeM > 0,
              "E3.0 (el escenario es el peligroso): el Maestro estaba en verde cuando el "
              "Esclavo salto, y siguio en verde " + std::to_string(e3VerdeM * PASO_MS) +
              " ms de la ventana");
    comprobar(e3Listo && e3VerdeE == 0 && e3AmbarE == 0 && e3Simul == 0,
              "E3 (D-26 (4)): el Esclavo salto " + std::to_string(SALTO_GRANDE) + " s hacia "
              "su verde con el Maestro en verde y NO encendio ni el ambar de su transicion "
              "ni el verde en los " + std::to_string(VENTANA_MS / 1000) + " s siguientes (" +
              std::to_string(e3AmbarE) + " ambar, " + std::to_string(e3VerdeE) + " verde, " +
              std::to_string(e3Simul) + " simultaneos)");

    // --- E4: y en el Esclavo un salto pequeno tampoco lo manda a rojo ------------------
    prepararSincronizadas(15, 8, 0, 0);
    entrarEnDegradadoLasDos(0);
    const bool e4Listo = esperarPines(
        []() { return ESCLAVO.verde() && !MAESTRO.verde(); }, ESPERA_MAX_MS);
    unsigned long e4VerdeE = 0, e4Ticks = 0;
    if (e4Listo) {
      avanzar(3000);
      ESCLAVO.orden("desviar_rtc", 2);
      for (unsigned long t = 0; t < 5000; t += PASO_MS) {
        unTick();
        e4Ticks++;
        if (ESCLAVO.verde()) e4VerdeE++;
      }
    }
    comprobar(e4Listo && e4VerdeE == e4Ticks,
              "E4 (control de E3): con el Esclavo en SU verde, un salto de 2 s lo deja en "
              "verde los 5 s siguientes (" + std::to_string(e4VerdeE) + " de " +
              std::to_string(e4Ticks) + " instantes)");
  }

  // =========================================================================
  std::printf("\n--- BLOQUE F: D-21 (1), UNA HORA QUE CADUCA ES UNA HORA QUE MIENTE ---\n");
  //
  // H1 del veredicto del 11/09: con el J17 de una punta mudo, su hora corre sobre el HSI
  // -hasta 90 s por hora- mientras la otra se siembra de su DS3231. En Degradado eso es
  // verde-verde en cada ciclo. D-21 (1) lo contesta con ambar en la punta que la tiene, y la
  // caducidad de reloj.cpp -HORA_CADUCA_MS- es lo que la hace medible. Aqui corre el
  // reloj.cpp REAL de las dos puntas, con el HSI de la punta afectada en el extremo rapido de
  // su ficha (HSI_PPM_PEOR, releido) y cada DS3231 con su hora.
  //
  // LO QUE NO SE MIDE AQUI: el ambar en la OTRA punta. D-21 lo dice: en Degradado no hay
  // radio y cada punta decide por su cuenta; la otra sigue ciclando, y eso se COMPRUEBA como
  // lo que es -la asimetria aceptada (Riesgo 2)-, no como un fallo.
  {
    const long CADUCA_M = MAESTRO.orden("hora_caduca_ms");
    const long CADUCA_E = ESCLAVO.orden("hora_caduca_ms");
    const long PPM = (long)leerNumero(RAIZ + "/Maestro/include/reloj.h",
                                      R"(HSI_PPM_PEOR\s*=\s*(\d+)UL)", "HSI_PPM_PEOR");
    // La caducidad vista desde el BANCO con el HSI rapido: millis() cuenta (1 + ppm) veces
    // lo que pasa de verdad, asi que la hora caduca ANTES en tiempo del banco.
    // Una por punta: cada una se compara con SU constante compilada. Con una sola, un plazo
    // distinto en el Maestro hacia caer la comprobacion del Esclavo -medido al inyectarlo- y
    // la linea acusaba a la punta equivocada.
    const unsigned long CADUCA_BANCO_MS =
        (unsigned long)((long long)CADUCA_M * 1000000LL / (1000000LL + PPM));
    const unsigned long CADUCA_BANCO_E =
        (unsigned long)((long long)CADUCA_E * 1000000LL / (1000000LL + PPM));
    std::printf("   HORA_CADUCA_MS compilada: %ld ms (Maestro) / %ld ms (Esclavo); HSI %ld ppm;\n"
                "   cadencia del ESP32 %lu ms; con el HSI rapido caduca a los %lu ms del banco\n",
                CADUCA_M, CADUCA_E, PPM, g_cadenciaSiembraMs, CADUCA_BANCO_MS);

    // --- F0: el instrumento. La caducidad COMPILADA, ejercida en su frontera ---------
    // Las dos puntas sembradas en la MISMA vuelta, sin radio -para que el Esclavo acepte la
    // de su ESP32- y sin mas siembras: en la vuelta en que la base cumple HORA_CADUCA_MS
    // todavia es fiable (el ">" de reloj_horaFiable()), y en la siguiente ya no. Y la hora
    // SIGUE puesta: la guarda que habia -reloj_enHora()- no la veia caducar.
    {
      prepararSincronizadas(15, 8, 0, 0);
      g_esp32Vivo[0] = g_esp32Vivo[1] = false;
      g_enlace = false;
      g_aire.clear();
      avanzar(SFTY6_SILENCIO_MS_E + 1000UL);
      activarDs3231(0, 0);
      g_esp32Vivo[0] = g_esp32Vivo[1] = false;   // solo el origen: nada de calendario
      const long rM = MAESTRO.orden("siembra_esp32", horaDs3231(0));
      const long rE = ESCLAVO.orden("siembra_esp32", horaDs3231(1));
      const unsigned long tSiembra = g_t - PASO_MS;   // el millis() con que se atendieron
      while ((g_t - PASO_MS) - tSiembra < (unsigned long)CADUCA_M) unTick();
      const bool enFronteraExacta = ((g_t - PASO_MS) - tSiembra) == (unsigned long)CADUCA_M;
      const long fM0 = MAESTRO.orden("hora_fiable"), fE0 = ESCLAVO.orden("hora_fiable");
      unTick();
      const long fM1 = MAESTRO.orden("hora_fiable"), fE1 = ESCLAVO.orden("hora_fiable");
      const long hM = MAESTRO.orden("reloj_en_hora"), hE = ESCLAVO.orden("reloj_en_hora");
      comprobar(CADUCA_M == CADUCA_E && CADUCA_M > (long)g_cadenciaSiembraMs,
                "F0.0: las dos puntas compilan la MISMA caducidad (" + std::to_string(CADUCA_M) +
                " ms) y es mayor que la cadencia del ESP32 (" + std::to_string(g_cadenciaSiembraMs) +
                " ms). La desigualdad contra el aguante la recalcula reloj_04");
      comprobar(rM == 1 && rE == 1 && enFronteraExacta && fM0 == 1 && fE0 == 1 && fM1 == 0 &&
                    fE1 == 0 && hM == 1 && hE == 1,
                "F0: el reloj.cpp REAL de las dos puntas, sembrado una vez (" + std::to_string(rM) +
                "/" + std::to_string(rE) + "), es FIABLE con la base de EXACTAMENTE " +
                std::to_string(CADUCA_M) + " ms (" + std::to_string(fM0) + "/" +
                std::to_string(fE0) + ") y deja de serlo 50 ms despues (" + std::to_string(fM1) +
                "/" + std::to_string(fE1) + "), con la hora TODAVIA PUESTA (reloj_enHora() " +
                std::to_string(hM) + "/" + std::to_string(hE) + "): la guarda de antes no la veia");
    }

    // --- F1: H1 ENTERO. J17 del Maestro mudo dos horas con la radio viva, y cae la radio --
    {
      prepararSincronizadas(15, 8, 0, 0);
      activarDs3231(1000, 7000);
      MAESTRO.orden("hsi_ppm", PPM);
      avanzar(10UL * 60UL * 1000UL);
      const bool controlVivo = MAESTRO.orden("hora_fiable") == 1 &&
                               ESCLAVO.orden("radio_manda") == 1 &&
                               g_siembrasSembradas[0] >= 2 && g_siembrasIgnoradas[1] >= 1;
      comprobar(controlVivo,
                "F1.0 (control del escenario): con los dos J17 vivos y la radio viva, el Maestro "
                "se siembra de su ESP32 (" + std::to_string(g_siembrasSembradas[0]) +
                " siembras) y su hora es fiable, y el Esclavo IGNORA la de su ESP32 (" +
                std::to_string(g_siembrasIgnoradas[1]) + " veces) porque manda la radio");

      g_esp32Vivo[0] = false;                         // el J17 del Maestro se calla
      avanzar(2UL * 3600UL * 1000UL);                 // dos horas, con la radio viva
      const long desvioM = desvioContraDs3231(MAESTRO, 0);
      const long enHoraM = MAESTRO.orden("reloj_en_hora");
      const long fiableM = MAESTRO.orden("hora_fiable");
      long dME = MAESTRO.orden("segundos_del_dia") - ESCLAVO.orden("segundos_del_dia");
      if (dME > 43200L) dME -= 86400L;
      if (dME < -43200L) dME += 86400L;
      nota("F1: tras 2 h con el J17 del Maestro mudo, su hora va " + std::to_string(desvioM) +
           " s por delante de su DS3231 (HSI a +" + std::to_string(PPM) + " ppm), y la del "
           "Esclavo la sigue por radio a " + std::to_string(dME) + " s: estan en fase, como "
           "dice el veredicto.");

      comprobar(enHoraM == 1 && fiableM == 0 && desvioM > DESFASE_QUE_AGUANTA,
                "F1.1 (la premisa de H1): el Maestro SIGUE EN HORA para reloj_enHora() (" +
                std::to_string(enHoraM) + ") con su hora " + std::to_string(desvioM) +
                " s por delante de su DS3231 -mas que los " + std::to_string(DESFASE_QUE_AGUANTA) +
                " s que el cruce aguanta, bloque C- y reloj_horaFiable() la da por caducada (" +
                std::to_string(fiableM) + ")");

      g_enlace = false;                               // cae la radio
      g_aire.clear();
      const long motivo = MAESTRO.orden("deg_evaluar");
      comprobar(motivo == MDG_FALTA_HORA_V,
                "F1.2: con la radio caida, la puerta del Maestro RECHAZA el Degradado por la hora "
                "(motivo " + std::to_string(motivo) + " = MDG_FALTA_HORA, leido del enum) en vez "
                "de aceptarlo y dar verdes con una hora " + std::to_string(desvioM) +
                " s adelantada");

      MAESTRO.orden("set_modo", MODO_DEGRADADO_V);   // el operario lo intenta igual
      const bool esclavoFresco = esperarCond(
          []() {
            return ESCLAVO.orden("radio_manda") == 0 && ESCLAVO.orden("hora_fiable") == 1 &&
                   g_siembrasSembradas[1] > 0;
          },
          g_cadenciaSiembraMs + SFTY6_SILENCIO_MS_E + 10000UL);
      long dMEtras = MAESTRO.orden("segundos_del_dia") - ESCLAVO.orden("segundos_del_dia");
      if (dMEtras > 43200L) dMEtras -= 86400L;
      if (dMEtras < -43200L) dMEtras += 86400L;
      const long rE = ESCLAVO.orden("degradado_entrar");
      comprobar(esclavoFresco && rE == DEG_ACEPTADO_V &&
                    (dMEtras > DESFASE_QUE_AGUANTA || -dMEtras > DESFASE_QUE_AGUANTA),
                "F1.3 (el escenario ES el peligroso): sin radio, el Esclavo toma la hora de SU "
                "DS3231 (D-26 (3)) y entra en Degradado (" + std::to_string(rE) + "); su hora y "
                "la del Maestro quedan a " + std::to_string(dMEtras) + " s, mas que los " +
                std::to_string(DESFASE_QUE_AGUANTA) + " que aguanta el cruce: si el Maestro "
                "ciclara por reloj, habria verde-verde");

      reiniciarObservacion();
      avanzar(20UL * 60UL * 1000UL);
      const long estadoM = MAESTRO.estado();
      comprobar(g_verdeSimultaneo == 0 && g_pegados == 0 && g_ticksVerdeMaestro == 0 &&
                    g_ticksVerdeEsclavo > 0,
                "F1.4 (H1): en 20 min con la radio caida, el Maestro NO encendio verde ni una "
                "vez (" + std::to_string(g_ticksVerdeMaestro) + ") y el Esclavo ciclo por su "
                "reloj (" + std::to_string(g_ticksVerdeEsclavo) + " instantes en verde): " +
                std::to_string(g_verdeSimultaneo) + " instantes con las dos en verde");
      comprobar(estadoM == S_FALLO_V,
                "F1.5: y el Maestro acaba en AMBAR INTERMITENTE (estado " +
                std::to_string(estadoM) + " = S_FALLO): rechazado el Degradado, cae al ambar de "
                "la perdida de enlace de semaforo.cpp. Es la punta con la hora que miente, en "
                "ambar, que es lo que pide D-21");
    }

    // --- F2: el J17 del Maestro muere con las DOS ya en Degradado -----------------------
    {
      prepararSincronizadas(15, 8, 0, 0);
      activarDs3231(1000, 40000);
      MAESTRO.orden("hsi_ppm", PPM);
      entrarEnDegradadoLasDos(0);
      reiniciarObservacion();
      avanzar(5UL * 60UL * 1000UL);
      comprobar(g_ticksVerdeMaestro > 0 && g_ticksVerdeEsclavo > 0 && g_verdeSimultaneo == 0,
                "F2.0 (control): con los dos J17 vivos, cada punta sembrada de su DS3231 y el HSI "
                "del Maestro en su extremo rapido, las dos ciclan (" +
                std::to_string(g_ticksVerdeMaestro) + "/" + std::to_string(g_ticksVerdeEsclavo) +
                " instantes en verde) sin tocarse: la siembra cada cadencia absorbe la deriva");

      g_esp32Vivo[0] = false;
      const unsigned long tUltima = g_tUltimaSiembraBuena[0];
      const long alarmas0 = MAESTRO.orden("alarmas_caducada");
      reiniciarObservacion();
      unsigned long tAmbar = 0, tUltVerde = 0, verdeETrasAmbar = 0;
      for (unsigned long t = 0; t < 60UL * 60UL * 1000UL; t += PASO_MS) {
        unTick();
        if (MAESTRO.verde()) tUltVerde = g_t - PASO_MS;
        if (tAmbar == 0 && MAESTRO.estado() == S_FALLO_V) tAmbar = g_t - PASO_MS;
        if (tAmbar != 0 && ESCLAVO.verde()) verdeETrasAmbar++;
      }
      const long alarmas = MAESTRO.orden("alarmas_caducada") - alarmas0;
      const unsigned long aAmbar = tAmbar ? tAmbar - tUltima : 0;
      comprobar(g_verdeSimultaneo == 0 && g_pegados == 0,
                "F2.1 (H1 dentro del modo): en 60 min con el J17 del Maestro mudo y su HSI a +" +
                std::to_string(PPM) + " ppm -a esa deriva el cruce se habria roto a los ~" +
                std::to_string((long)(DESFASE_QUE_AGUANTA * 1000000L / PPM / 60L)) + " min- NO "
                "hubo ni un instante con las dos en verde (" + std::to_string(g_verdeSimultaneo) +
                ") ni un verde pegado al otro");
      comprobar(tAmbar != 0 && MAESTRO.estado() == S_FALLO_V && alarmas == 1 &&
                    aAmbar >= CADUCA_BANCO_MS && aAmbar <= (unsigned long)CADUCA_M + 2000UL + 2UL * PASO_MS &&
                    tUltVerde <= tUltima + (unsigned long)CADUCA_M,
                "F2.2: el Maestro paso a AMBAR a los " + std::to_string(aAmbar) + " ms de su "
                "ultima siembra buena -la caducidad compilada, vista desde el banco con su HSI "
                "rapido, mas los 2 s de rojo de irAAmbar()-, no volvio a dar verde despues y lo "
                "PUBLICO: " + std::to_string(alarmas) + " $ALARM HORA_ESP32,CADUCADA");
      comprobar(verdeETrasAmbar > 0,
                "F2.3 (la asimetria de D-21, medida y no escondida): con el Maestro en ambar, el "
                "Esclavo SIGUE en Degradado dando verdes por su reloj (" +
                std::to_string(verdeETrasAmbar) + " instantes): en Degradado no hay radio con que "
                "decirselo");

      // NO SE REANUDA SOLO (D-21; correccion del orquestador del 11/09): vuelve el J17, la
      // hora vuelve a ser fiable y el Maestro SIGUE en ambar. Se comprueba lo que el codigo
      // hace hoy, no se cambia.
      g_esp32Vivo[0] = true;
      g_proxSiembra[0] = g_t + 1000UL;
      reiniciarObservacion();
      avanzar(10UL * 60UL * 1000UL);
      comprobar(MAESTRO.orden("hora_fiable") == 1 && g_ticksVerdeMaestro == 0 &&
                    MAESTRO.estado() == S_FALLO_V &&
                    MAESTRO.orden("alarmas_caducada") - alarmas0 == 1,
                "F2.4 (NO se reanuda solo): vuelto el J17, la hora del Maestro vuelve a ser "
                "fiable y en 10 min NO enciende verde (" + std::to_string(g_ticksVerdeMaestro) +
                "): sigue en ambar hasta que una persona saque el Degradado (D-21: el equipo no "
                "decide solo si sale del modo ni si vuelve a el)");
    }

    // --- F3: el simetrico, en el ESCLAVO -------------------------------------------------
    {
      prepararSincronizadas(15, 8, 0, 0);
      activarDs3231(1000, 40000);
      ESCLAVO.orden("hsi_ppm", PPM);
      entrarEnDegradadoLasDos(0);
      reiniciarObservacion();
      avanzar(5UL * 60UL * 1000UL);
      comprobar(g_ticksVerdeMaestro > 0 && g_ticksVerdeEsclavo > 0 && g_verdeSimultaneo == 0,
                "F3.0 (control): con el HSI del Esclavo en su extremo rapido y su J17 vivo, las "
                "dos ciclan sin tocarse (" + std::to_string(g_ticksVerdeMaestro) + "/" +
                std::to_string(g_ticksVerdeEsclavo) + ")");

      g_esp32Vivo[1] = false;
      const unsigned long tUltima = g_tUltimaSiembraBuena[1];
      const long alarmas0 = ESCLAVO.orden("alarmas_caducada");
      reiniciarObservacion();
      unsigned long tAmbar = 0, tUltVerde = 0, verdeMTrasAmbar = 0;
      for (unsigned long t = 0; t < 60UL * 60UL * 1000UL; t += PASO_MS) {
        unTick();
        if (ESCLAVO.verde() || ESCLAVO.estado() == S_AMARILLO_E) {   // o el ambar que abre su verde
          tUltVerde = g_t - PASO_MS;
        }
        if (tAmbar == 0 && ESCLAVO.estado() == S_FALLO_E) tAmbar = g_t - PASO_MS;
        if (tAmbar != 0 && MAESTRO.verde()) verdeMTrasAmbar++;
      }
      const long alarmas = ESCLAVO.orden("alarmas_caducada") - alarmas0;
      const unsigned long aAmbar = tAmbar ? tAmbar - tUltima : 0;
      const unsigned long DESPEJE_MS = DEG_DESPEJE_SEG * 1000UL;
      comprobar(g_verdeSimultaneo == 0 && g_pegados == 0,
                "F3.1 (el simetrico de H1): en 60 min con el J17 del ESCLAVO mudo y su HSI a +" +
                std::to_string(PPM) + " ppm, ni un instante con las dos en verde (" +
                std::to_string(g_verdeSimultaneo) + ")");
      comprobar(tAmbar != 0 && ESCLAVO.estado() == S_FALLO_E &&
                    ESCLAVO.orden("degradado_estado") == DEG_RENDIDO_V && alarmas == 1 &&
                    aAmbar >= CADUCA_BANCO_E &&
                    aAmbar <= (unsigned long)CADUCA_E + DESPEJE_MS + 2UL * PASO_MS &&
                    tUltVerde <= tUltima + (unsigned long)CADUCA_E,
                "F3.2: el Esclavo se RINDIO -todo-rojo el despeje entero y despues ambar, "
                "DEG_RENDIDO- a los " + std::to_string(aAmbar) + " ms de su ultima siembra buena, "
                "sin volver a encender, y lo PUBLICO: " + std::to_string(alarmas) +
                " $ALARM HORA_ESP32,CADUCADA");
      comprobar(verdeMTrasAmbar > 0,
                "F3.3 (la asimetria de D-21): con el Esclavo en ambar, el Maestro sigue en "
                "Degradado dando verdes (" + std::to_string(verdeMTrasAmbar) + " instantes)");

      const long rechazo = ESCLAVO.orden("degradado_comprobar");
      g_esp32Vivo[1] = true;
      g_proxSiembra[1] = g_t + 1000UL;
      reiniciarObservacion();
      avanzar(10UL * 60UL * 1000UL);
      const long tras = ESCLAVO.orden("degradado_comprobar");
      comprobar(rechazo == DEG_RECHAZO_SIN_HORA_V && tras == DEG_ACEPTADO_V &&
                    ESCLAVO.orden("degradado_estado") == DEG_RENDIDO_V &&
                    g_ticksVerdeEsclavo == 0,
                "F3.4: con la hora caducada la puerta del Esclavo rechaza entrar (" +
                std::to_string(rechazo) + " = DEG_RECHAZO_SIN_HORA) y, vuelto el J17, la "
                "aceptaria (" + std::to_string(tras) + ") PERO NO ENTRA SOLA: sigue en "
                "DEG_RENDIDO y en 10 min no enciende (" + std::to_string(g_ticksVerdeEsclavo) + ")");
    }

    // --- F4: LOS BORDES DE D-26 (4), en las dos puntas y en los dos sentidos -------------
    // Hasta hoy el bloque E saltaba el ciclo entero -muy lejos del umbral- y 2 s -muy cerca
    // de cero-. El umbral es SALTO_SIN_ROJO_MAX_S = despeje - 1: el borde es EXACTAMENTE ahi.
    {
      const unsigned long ESPERA = 3UL * CICLO_S * 1000UL;
      const long D = (long)DEG_DESPEJE_SEG;
      struct Caso { bool maestro; long J; bool rojo; const char* quien; };
      const Caso casos[] = {
        { true,  D,       true,  "Maestro, +despeje" },
        { true,  D - 1,   false, "Maestro, +(despeje-1)" },
        { true,  -D,      true,  "Maestro, -despeje (hacia atras)" },
        { true,  -(D - 1), false, "Maestro, -(despeje-1) (hacia atras)" },
        { false, D,       true,  "Esclavo, +despeje" },
        { false, D - 1,   false, "Esclavo, +(despeje-1)" },
        { false, -D,      true,  "Esclavo, -despeje (hacia atras)" },
        { false, -(D - 1), false, "Esclavo, -(despeje-1) (hacia atras)" },
      };
      for (const Caso& c : casos) {
        const Borde b = probarBorde(c.maestro, c.J, ESPERA);
        const bool ok = b.listo && b.simultaneos == 0 &&
                        (c.rojo ? (b.eventos == 1 && !b.enciende)
                                : (b.eventos == 0 && b.enciende));
        comprobar(ok,
                  std::string("F4 (D-26 (4), borde): ") + c.quien + " = " + std::to_string(c.J) +
                  " s, aplicado DENTRO de su propio verde -> " +
                  (c.rojo ? "PASA POR ROJO" : "DIRECTO") + ": " + std::to_string(b.eventos) +
                  " $EVENT SALTO_DE_HORA_POR_ROJO, " + (b.enciende ? "enciende" : "no enciende") +
                  " en 3 s, " + std::to_string(b.simultaneos) + " instantes con las dos en verde" +
                  (b.listo ? "" : " [EL ESCENARIO NO SE PUDO MONTAR]"));
      }
    }

    // --- F5: reloj_radioManda() EJECUTADA: la frontera de 25 s y sus dos vecinos --------
    // H5 del veredicto: "leido, no ejecutado". Aqui corre la del reloj.cpp real.
    {
      prepararSincronizadas(15, 8, 0, 0);
      activarDs3231(0, 0);
      g_esp32Vivo[0] = g_esp32Vivo[1] = false;      // las siembras las pone este bloque
      avanzar(1000);
      const long conRadio = ESCLAVO.orden("siembra_esp32", horaDs3231(1));
      g_enlace = false;
      g_aire.clear();
      const unsigned long tU = g_tUltimaEntregaEsclavo;
      while ((g_t - PASO_MS) < tU + SFTY6_SILENCIO_MS_E) unTick();
      const bool exacta = (g_t - PASO_MS) == tU + SFTY6_SILENCIO_MS_E;
      const long enBorde = ESCLAVO.orden("siembra_esp32", horaDs3231(1));
      unTick();
      const long pasado = ESCLAVO.orden("siembra_esp32", horaDs3231(1));
      comprobar(conRadio == 2 && exacta && enBorde == 2 && pasado == 1,
                "F5.1 (D-26 (3), la frontera): con la hora de radio puesta, la siembra del ESP32 "
                "se IGNORA con la radio viva (" + std::to_string(conRadio) + ") y con el silencio "
                "en EXACTAMENTE SFTY6_SILENCIO_MS = " + std::to_string(SFTY6_SILENCIO_MS_E) +
                " ms (" + std::to_string(enBorde) + "), y ENTRA 50 ms despues (" +
                std::to_string(pasado) + "). 2 = ignorada, 1 = sembrada");

      arrancarLasDos();                               // sin sincronizar nada por radio
      avanzar(5000);                                  // la radio viva: latidos del Maestro
      const long sinHoraDeRadio = ESCLAVO.orden("siembra_esp32", horaDs3231(1));
      const long otraSinRadioHora = ESCLAVO.orden("siembra_esp32", horaDs3231(1));
      MAESTRO.orden("ajustar_reloj", 15L * 1000000L + 8L * 10000L);
      MAESTRO.orden("sincronizar_hora");
      avanzar(15000);
      const long trasRadio = ESCLAVO.orden("siembra_esp32", horaDs3231(1));
      g_enlace = false;
      g_aire.clear();
      avanzar(SFTY6_SILENCIO_MS_E + 1000UL);
      const long radioCaida = ESCLAVO.orden("siembra_esp32", horaDs3231(1));
      g_enlace = true;
      MAESTRO.orden("sincronizar_hora");
      avanzar(15000);
      const long radioVuelta = ESCLAVO.orden("siembra_esp32", horaDs3231(1));
      comprobar(sinHoraDeRadio == 1 && otraSinRadioHora == 1 && trasRadio == 2 &&
                    radioCaida == 1 && radioVuelta == 2,
                "F5.2 (arranque sin radio y radio intermitente): recien arrancado, con la radio "
                "latiendo pero sin hora del Maestro, la del ESP32 ENTRA (" +
                std::to_string(sinHoraDeRadio) + ", " + std::to_string(otraSinRadioHora) +
                "); en cuanto el Maestro le pone la suya, se IGNORA (" +
                std::to_string(trasRadio) + "); cae la radio y ENTRA (" +
                std::to_string(radioCaida) + "); vuelve y la hora del Maestro la pisa otra vez (" +
                std::to_string(radioVuelta) + ")");
    }
  }

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
