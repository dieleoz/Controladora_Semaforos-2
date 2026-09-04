// ===== Validacion_Automatico/dos_puntas/orquestador.cpp =====
//
// EL INSTRUMENTO QUE FALTABA: LAS DOS PUNTAS, EL C++ REAL DE LAS DOS, EJECUTANDOSE A
// LA VEZ, Y UN OBSERVADOR QUE MIRA LOS DOCE PINES EN EL MISMO INSTANTE.
//
// La auditoria del 31/08 lo dejo por escrito y se verifico: NINGUN instrumento ejecuta
// el C++ real de las dos puntas a la vez y comprueba que nunca dan verde las dos. Lo
// que cerraba ese lazo era una copia del firmware escrita a mano en Python -la prueba
// 5 de simulador_sistema_v7_6-, que es justo lo que el apartado 8 de CLAUDE.md avisa
// que NO prueba el codigo. Y de los otros dos candidatos:
//
//   barrera_02_dos_puntas   compara el enclavamiento como TEXTO. Buen proxy. Dos
//                           ficheros identicos pueden estar identicamente mal, y
//                           ademas la barrera no es lo unico que decide un verde:
//                           esta quien la llama.
//   Validacion_Automatico   compila C++ real... SOLO DEL MAESTRO. Su Esclavo es un
//                           switch de veinte lineas en el propio arnes.
//
// Verde contra verde en un cierre de carril es un choque frontal. Es la propiedad de
// seguridad mas cara del equipo y era la peor cubierta.
//
// ===========================================================================
// EL PROBLEMA TECNICO: LOS SIMBOLOS CHOCAN. COMO SE RESOLVIO Y QUE SE DESCARTO
// ===========================================================================
//
// Maestro y Esclavo definen LOS MISMOS NOMBRES con implementaciones distintas
// -semaforo_setup(), semaforo_estado(), protocolo_enviarPaquete(), setup(), loop()...-.
// Enlazarlos en un solo binario es imposible. Validacion_LCD ya se topo con esto y lo
// resolvio construyendo DOS programas, ejecutandolos uno detras de otro y SUMANDO sus
// resultados. Eso vale para geometria de pantalla y NO vale aqui: dos ejecuciones
// separadas no pueden comprobar "nunca las dos en verde EN EL MISMO INSTANTE", porque
// no hay un instante comun donde mirar.
//
// SE ELIGIO: UNA DLL POR PUNTA, LAS DOS CARGADAS EN EL MISMO PROCESO.
//
// Cada punta se compila con g++ -shared en su propio modulo. El orquestador las abre
// con LoadLibrary y resuelve la API con GetProcAddress, asi que su propia tabla de
// simbolos no contiene NI UN nombre del firmware: puede haber dos semaforo_estado()
// vivos a la vez sin que el enlazador tenga nada que decidir. Un tick del arnes pone
// el MISMO millis() en las dos y llama a las dos; entre tick y tick se leen los doce
// pines. Ese es el instante comun.
//
// Lo que cuesta: dos ficheros mas en build/, y una tabla de punteros a funcion.
// Lo que da, y es lo que decidio la eleccion frente a las otras tres:
//
//   1. UN SOLO PROCESO. No hay protocolo entre procesos que inventar, ni ordenacion de
//      mensajes de la que fiarse, ni dos relojes que sincronizar. El instante comun no
//      se negocia: es una variable del bucle.
//   2. EL MICROCORTE SALE GRATIS Y SALE EXACTO. FreeLibrary + LoadLibrary vuelve a
//      mapear la DLL con su .data reinicializada y su .bss a cero. TODAS las estaticas
//      del firmware de esa punta -las de semaforo.cpp, las del despachador, las del
//      Modo Degradado, incluidas las que nadie recuerda- vuelven a su valor de
//      arranque, y la OTRA punta no se entera. Eso es un arranque en frio de verdad.
//      La alternativa habitual -escribir un reset() a mano- es una lista mantenida por
//      una persona, y una variable olvidada convertiria el escenario en un fraude
//      silencioso. Aqui la garantia la da el cargador del sistema.
//      (El arnes exige ademas que ese reinicio HAYA ocurrido: ver el bloque E.)
//
// SE DESCARTARON, y por que:
//
//   (a) DOS PROCESOS QUE SE HABLAN (tuberia o socket). Habria hecho falta serializar el
//       estado de cada punta en cada tick y un protocolo propio para el reloj comun. El
//       instante de observacion pasaria a depender del orden en que llegan los mensajes
//       -exactamente la clase de cosa que "funciona por accidente" (N-89)- y una
//       carrera dentro del arnes se leeria como un hallazgo de firmware. Ademas, un
//       microcorte seria matar y relanzar un proceso: correcto, pero mas lento y con la
//       misma necesidad de repescar el dominio de respaldo.
//
//   (b) PREFIJAR SIMBOLOS AL COMPILAR (-Dsemaforo_setup=esclavo_semaforo_setup...).
//       Exige una LISTA de macros escrita a mano, una por funcion publica. Una funcion
//       nueva en el firmware no aparece en la lista, no se renombra, y el choque
//       reaparece -o peor: enlaza contra la punta equivocada y el arnes mide el Maestro
//       creyendo medir el Esclavo-. Es un modelo mantenido a mano de la superficie del
//       firmware: la misma forma del defecto que este banco lleva tres anos pagando.
//
//   (b') Su variante mecanica, objcopy --prefix-symbols sobre un enlace parcial, no
//       necesita lista... pero renombra TAMBIEN los simbolos no definidos, asi que
//       memcpy y snprintf pasan a ser esclavo_memcpy y esclavo_snprintf y no enlaza
//       nada. Desenredarlo pide una segunda pasada de --redefine-sym dependiente de la
//       libc del dia. Se descarta por fragil, no por imposible.
//
//   (c) ESPACIOS DE NOMBRES, incluyendo el .cpp dentro de un namespace. Cambia el orden
//       y el alcance de los #include del firmware, de modo que lo que se compila deja
//       de ser letra por letra lo que va a la tarjeta -que es el unico motivo por el que
//       este arnes vale mas que un modelo-. Y basta un extern "C" o un #pragma once
//       resuelto antes para que se caiga de formas dificiles de leer.
//
// ===========================================================================
// QUE SE MIDE, Y SOBRE QUE
// ===========================================================================
//
// La propiedad de vida: NUNCA VERDE1 NI VERDE2 ENCENDIDO EN LAS DOS PUNTAS A LA VEZ,
// medido sobre lo que semaforo.cpp ESCRIBIO EN LOS PINES -la barrera de salidas dice
// que solo el escribe luz, y todo pasa por su escribirPines()-, no sobre su logica ni
// sobre semaforo_estado().
//
// N-96: escribirPines() mueve SEIS pines, no ocho. ROJO_PEATON, VERDE_PEATON y el
// BUZZER estan declarados y muertos. Este arnes no da por hecho ocho: cuenta las
// escrituras pin a pin y exige que esos tres sigan a cero Y que los seis vivos no lo
// esten.
//
// LO QUE ESTE ARNES NO CUBRE, dicho para que nadie lo cuente como cubierto:
//   - protocolo.cpp no se compila: CRC, rafaga y proteccion de replay van por otro lado.
//   - del Maestro no entran main.cpp ni modo_degradado.cpp (ver adaptador_maestro.cpp).
//   - la LCD, el Bluetooth y el RTC son sustitutos.
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
// que la DLL resuelve. De pines.h solo necesita los NUMEROS, y HIGH/LOW se definen
// aqui porque comun/pines.h los usa para los niveles de la pluma.
#define HIGH 1
#define LOW 0
#include "comun/pines.h"
#include "punta_api.h"

// Comandos del protocolo que el orquestador inyecta como si fueran del Maestro para
// montar los escenarios de configuracion y de hora. Los VALORES SE RELEEN de
// protocolo.h en tiempo de ejecucion (ver leerDefine): aqui no hay ni un numero de
// comando escrito a mano.
static uint8_t CMD_HORA_D_V, CMD_HORA_H_V, CMD_HORA_M_V, CMD_HORA_S_V;
static uint8_t CMD_CONFIG_VERDE_V, CMD_CONFIG_DESPEJE_V;
static unsigned long SFTY6_SILENCIO_MS_V;
static unsigned long TIMEOUT_ACK_MS_V;
static unsigned long CICLO_MAX_REINTENTOS_V;
static unsigned long DESPEJE_POR_DEFECTO_S;

// ---------------------------------------------------------------------------
// EL CONTADOR. Mismo patron que arnes_automatico.cpp y arnes_ciclo.cpp.
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

static void abortar(const std::string& motivo) {
  std::fprintf(stdout, "\n[ABORTADO] %s\n", motivo.c_str());
  std::fprintf(stdout,
      "Un ABORTADO no dice NADA del firmware, y menos que un PASS. No se apunta\n"
      "para luego: mientras este arnes no corra, el verde simultaneo no lo vigila\n"
      "nadie sobre el codigo real.\n");
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

// Las constantes se releen del C++ en cada corrida. SIN VALOR POR DEFECTO, NUNCA: un
// banco que no puede fallar no demuestra nada, y una constante escrita a mano que
// "casualmente coincide" es como se cuelan las pruebas muertas.
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
// UNA PUNTA: SU DLL, SU API Y SU DOMINIO DE RESPALDO.
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
                    ". Las dos puntas tienen que cumplir el MISMO contrato: una API "
                    "que falta se descubriria en mitad de un escenario");
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

  // ORDENES CON NOMBRE. Una clave que la punta no conoce ABORTA: devolver 0 en
  // silencio convertiria una errata del arnes en un PASS.
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
  long toques(int p) { return orden("toques", p); }
};

static Punta MAESTRO, ESCLAVO;

// ---------------------------------------------------------------------------
// EL CANAL DE RADIO. Ninguna punta habla con la otra: las dos hablan con esto.
//
// Es lo que permite cortar el enlace en un instante elegido, y cortarlo EN UNA SOLA
// DIRECCION -que es la averia fea y la que ningun modelo estaba ejerciendo: el Maestro
// oye al Esclavo pero el Esclavo no oye al Maestro-.
// ---------------------------------------------------------------------------
struct EnVuelo {
  unsigned char trama[4];
  unsigned long tEntrega;
  int destino;   // 0 = Maestro, 1 = Esclavo
};

static std::vector<EnVuelo> g_aire;
static bool g_enlaceHaciaEsclavo = true;
static bool g_enlaceHaciaMaestro = true;
static unsigned long g_latenciaMs = 50;
static unsigned long g_tramasEntregadas = 0;
static unsigned long g_tramasPerdidas = 0;

// ---------------------------------------------------------------------------
// EL OBSERVADOR. Corre DESPUES de que las dos puntas hayan ejecutado el mismo
// instante, que es lo que da sentido a la palabra "a la vez".
// ---------------------------------------------------------------------------
static unsigned long g_instantes = 0;
static unsigned long g_verdeSimultaneo = 0;
static unsigned long g_primerSimultaneoMs = 0;
static unsigned long g_ticksVerdeMaestro = 0;
static unsigned long g_ticksVerdeEsclavo = 0;
static unsigned long g_enclavamientoRoto = 0;   // rojo y verde a la vez EN LA MISMA punta
static unsigned long g_talanqueraSinVerde = 0;
static unsigned long g_verdeSinRojoEnfrente = 0;

// El detector, aislado en una funcion para que el control negativo del bloque E pueda
// ejercerlo con valores sinteticos. Un detector que solo se prueba a si mismo cuando
// nada falla es un adorno.
static bool hayVerdeSimultaneo(bool verdeA, bool verdeB) { return verdeA && verdeB; }

static void vigilar(unsigned long t) {
  g_instantes++;
  bool vM = MAESTRO.verde();
  bool vE = ESCLAVO.verde();
  if (vM) g_ticksVerdeMaestro++;
  if (vE) g_ticksVerdeEsclavo++;

  if (hayVerdeSimultaneo(vM, vE)) {
    if (g_verdeSimultaneo == 0) g_primerSimultaneoMs = t;
    g_verdeSimultaneo++;
  }

  // Con una punta en verde, la otra tiene que tener ROJO ENCENDIDO -no basta con "no
  // verde": ambar o apagado tambien serian una via sin ROJO frente a un verde-. Se
  // excluye el ambar intermitente de SFTY-6 (S_FALLO), que es el estado seguro
  // declarado: ahi ya no hay quien gobierne y las dos vias pasan con precaucion.
  const int S_FALLO = 3;
  if (vM && !vE && ESCLAVO.estado() != S_FALLO && !ESCLAVO.rojo()) g_verdeSinRojoEnfrente++;
  if (vE && !vM && MAESTRO.estado() != S_FALLO && !MAESTRO.rojo()) g_verdeSinRojoEnfrente++;

  // SFTY-2 dentro de cada punta, sobre lo que se escribio en el pin.
  Punta* dos[2] = { &MAESTRO, &ESCLAVO };
  for (int i = 0; i < 2; i++) {
    Punta* p = dos[i];
    if ((p->pin(ROJO1) == HIGH && p->pin(VERDE1) == HIGH) ||
        (p->pin(ROJO2) == HIGH && p->pin(VERDE2) == HIGH)) {
      g_enclavamientoRoto++;
    }
    // SFTY-28: pluma arriba sin verde. La excepcion de S_FALLO va por nombre.
    if (p->pin(MOTOR_TALANQUERA) == TALANQUERA_ABRIR &&
        p->pin(VERDE1) != HIGH && p->pin(VERDE2) != HIGH &&
        p->estado() != S_FALLO) {
      g_talanqueraSinVerde++;
    }
  }
}

// ---------------------------------------------------------------------------
// EL BUCLE. Un tick = el MISMO millis() en las dos puntas.
// ---------------------------------------------------------------------------
static unsigned long g_t = 0;
static const unsigned long PASO_MS = 50;

static void unTick() {
  // 1. Lo que ya vencio en el aire se entrega ANTES de que las puntas corran.
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

  // 2. Las dos puntas ejecutan EL MISMO INSTANTE.
  MAESTRO.tick(g_t);
  ESCLAVO.tick(g_t);

  // 3. Se recoge lo que cada una quiso emitir.
  unsigned char b[4];
  while (MAESTRO.tx(b)) {
    if (g_enlaceHaciaEsclavo) {
      EnVuelo e; memcpy(e.trama, b, 4); e.tEntrega = g_t + g_latenciaMs; e.destino = 1;
      g_aire.push_back(e);
    } else {
      g_tramasPerdidas++;
    }
  }
  while (ESCLAVO.tx(b)) {
    if (g_enlaceHaciaMaestro) {
      EnVuelo e; memcpy(e.trama, b, 4); e.tEntrega = g_t + g_latenciaMs; e.destino = 0;
      g_aire.push_back(e);
    } else {
      g_tramasPerdidas++;
    }
  }

  // 4. Y SOLO ENTONCES se observa: los doce pines del mismo instante.
  vigilar(g_t);
  g_t += PASO_MS;
}

static void avanzar(unsigned long ms) {
  unsigned long hecho = 0;
  while (hecho < ms) { unTick(); hecho += PASO_MS; }
}

// Inyecta una trama en una punta como si viniera de la otra. Se usa para el trafico de
// SERVICIO que el Maestro de este arnes no emite -hora y configuracion del ciclo salen
// de pantallas del Maestro que aqui no se compilan-. La trama la procesa el
// DESPACHADOR REAL del Esclavo: lo que se inyecta son bytes, no comportamiento.
static void inyectar(Punta& destino, uint8_t cmd, uint8_t param) {
  unsigned char t[4] = { 0, cmd, param, 0 };
  destino.rx(t);
}

// ---------------------------------------------------------------------------
// EL MICROCORTE. Un corte de energia en UNA punta, con la otra corriendo.
//
// FreeLibrary + LoadLibrary devuelve la DLL con .data reinicializada y .bss a cero:
// todas las estaticas del firmware de esa punta vuelven al arranque. Lo que NO se
// pierde es el dominio de respaldo -pila CR2032-, asi que se vuelca antes y se repone
// despues. Si esto se olvidara, el Modo Degradado no podria reanudar nunca y el arnes
// estaria midiendo un equipo que no existe.
// ---------------------------------------------------------------------------
static void microcorte(Punta& p) {
  long dominio[PUNTA_DOMINIO_PALABRAS];
  for (int i = 0; i < PUNTA_DOMINIO_PALABRAS; i++) dominio[i] = p.domLeer(i);
  p.descargar();
  p.cargar();
  for (int i = 0; i < PUNTA_DOMINIO_PALABRAS; i++) p.domEscribir(i, dominio[i]);
  p.arrancar();
}

// Deja las dos puntas recien arrancadas y el canal limpio. No es cosmetica: sin esto,
// una trama en vuelo de un escenario llegaria al siguiente y el arnes estaria midiendo
// una averia que el mismo fabrico.
static void escenarioLimpio(long tiemposMaestro) {
  g_aire.clear();
  g_enlaceHaciaEsclavo = g_enlaceHaciaMaestro = true;
  MAESTRO.descargar(); MAESTRO.cargar(); MAESTRO.arrancar();
  ESCLAVO.descargar(); ESCLAVO.cargar(); ESCLAVO.arrancar();
  if (tiemposMaestro > 0) MAESTRO.orden("fijar_tiempos", tiemposMaestro);
  MAESTRO.orden("arrancar_automatico");
  avanzar(500);
}

// Empaqueta verde(min), rojo(min), despeje(s) como espera punta_mando("fijar_tiempos").
static long tiempos(int verdeMin, int rojoMin, int despejeSeg) {
  return verdeMin * 10000L + rojoMin * 100L + despejeSeg;
}

// Pone al Esclavo en hora POR EL CAMINO REAL: cuatro tramas por radio que procesa su
// despachador. No se le escribe el RTC por la puerta de atras.
static void sincronizarEsclavo(uint8_t dia, uint8_t h, uint8_t m, uint8_t s) {
  inyectar(ESCLAVO, CMD_HORA_D_V, dia);
  inyectar(ESCLAVO, CMD_HORA_H_V, h);
  inyectar(ESCLAVO, CMD_HORA_M_V, m);
  inyectar(ESCLAVO, CMD_HORA_S_V, s);
  avanzar(400);
}

static void configurarEsclavo(uint8_t verdeSeg, uint8_t despejeSeg) {
  inyectar(ESCLAVO, CMD_CONFIG_VERDE_V, verdeSeg);
  inyectar(ESCLAVO, CMD_CONFIG_DESPEJE_V, despejeSeg);
  avanzar(400);
}

// ---------------------------------------------------------------------------
int main() {
  std::printf("==============================================================\n");
  std::printf(" ARNES DE LAS DOS PUNTAS - el C++ REAL de Maestro y Esclavo\n");
  std::printf(" ejecutandose A LA VEZ, observado sobre los pines\n");
  std::printf("==============================================================\n");

  // --- Constantes releidas del C++ real -------------------------------------
  const std::string PROTO_M = RAIZ + "/Maestro/include/protocolo.h";
  const std::string COORD   = RAIZ + "/Maestro/src/coordinador.cpp";
  const std::string AUTOM   = RAIZ + "/Maestro/src/modo_automatico.cpp";

  // Los codigos de comando van en hexadecimal en protocolo.h; leerNumero() lee decimal.
  {
    auto hex = [&](const std::string& ruta, const char* nombre) -> uint8_t {
      std::string txt = leerFuente(ruta);
      std::smatch m;
      std::regex re(std::string("#define\\s+") + nombre + "\\s+0x([0-9A-Fa-f]+)");
      if (!std::regex_search(txt, m, re)) abortar(std::string("falta ") + nombre);
      return (uint8_t)std::strtoul(m[1].str().c_str(), nullptr, 16);
    };
    CMD_HORA_D_V         = hex(PROTO_M, "CMD_HORA_D");
    CMD_HORA_H_V         = hex(PROTO_M, "CMD_HORA_H");
    CMD_HORA_M_V         = hex(PROTO_M, "CMD_HORA_M");
    CMD_HORA_S_V         = hex(PROTO_M, "CMD_HORA_S");
    CMD_CONFIG_VERDE_V   = hex(PROTO_M, "CMD_CONFIG_VERDE");
    CMD_CONFIG_DESPEJE_V = hex(PROTO_M, "CMD_CONFIG_DESPEJE");
  }
  SFTY6_SILENCIO_MS_V  = leerNumero(PROTO_M, R"(#define\s+SFTY6_SILENCIO_MS\s+(\d+)UL)", "SFTY6_SILENCIO_MS");
  TIMEOUT_ACK_MS_V     = leerNumero(COORD, R"(TIMEOUT_ACK_MS\s*=\s*(\d+))", "TIMEOUT_ACK_MS");
  CICLO_MAX_REINTENTOS_V = leerNumero(COORD, R"(CICLO_MAX_REINTENTOS\s*=\s*(\d+))", "CICLO_MAX_REINTENTOS");
  // N-131 (04/09): el despeje de arranque ya no es un literal en el inicializador.
  // Sale de DESPEJE_SEG_MIN, la misma constante que la guarda de SET_TIEMPOS, para que
  // no puedan divergir. Este arnes ABORTO al cambiarlo -leia el patron viejo- y eso es
  // lo correcto: §5, mover contenido rompe al que lee por patron, y un ABORTADO avisa
  // mientras que un numero supuesto no.
  DESPEJE_POR_DEFECTO_S  = leerNumero(AUTOM, R"(DESPEJE_SEG_MIN\s*=\s*(\d+))", "despeje por defecto");

  std::printf("\n Constantes releidas del C++ real: silencio SFTY-6 = %lu ms,\n",
              SFTY6_SILENCIO_MS_V);
  std::printf(" timeout de ACK = %lu ms x %lu reintentos, despeje por defecto = %lu s.\n",
              TIMEOUT_ACK_MS_V, CICLO_MAX_REINTENTOS_V, DESPEJE_POR_DEFECTO_S);

  // --- Guarda de mapeo: comun/pines.h es UNO para las dos puntas ------------
  // Este arnes usa un solo sustituto de pines.h. Vale porque los dos reales asignan
  // los mismos puertos a las mismas luces; si eso dejara de ser cierto, el sustituto
  // lo ESCONDERIA. Se comprueba aqui, antes de medir nada.
  {
    std::string pm = leerFuente(RAIZ + "/Maestro/include/pines.h");
    std::string pe = leerFuente(RAIZ + "/Esclavo/include/pines.h");
    const char* luces[] = { "ROJO1", "AMARILLO1", "VERDE1", "ROJO2", "AMARILLO2",
                            "VERDE2", "MOTOR_TALANQUERA" };
    for (const char* l : luces) {
      std::regex re(std::string("#define\\s+") + l + "\\s+(\\w+)");
      std::smatch a, b;
      if (!std::regex_search(pm, a, re) || !std::regex_search(pe, b, re)) {
        abortar(std::string("no se encuentra ") + l + " en algun pines.h real: el "
                "sustituto de este arnes no se puede dar por bueno");
      }
      if (a[1].str() != b[1].str()) {
        abortar(std::string("el pin de ") + l + " DIFIERE entre puntas (" +
                a[1].str() + " vs " + b[1].str() + "). El sustituto comun de este "
                "arnes lo estaria escondiendo");
      }
    }
  }

  // --- Carga de las dos puntas ---------------------------------------------
  MAESTRO.ruta = AQUI + "/build/punta_maestro.dll";
  MAESTRO.etiquetaEsperada = "MAESTRO";
  ESCLAVO.ruta = AQUI + "/build/punta_esclavo.dll";
  ESCLAVO.etiquetaEsperada = "ESCLAVO";
  MAESTRO.cargar();
  ESCLAVO.cargar();

  // =========================================================================
  std::printf("\n--- BLOQUE E: que el arnes SEA lo que dice ser -------------------\n");
  // Va PRIMERO a proposito. Todo lo que viene detras solo significa algo si las dos
  // DLL son de verdad dos, si el observador sabe ver una violacion y si el microcorte
  // reinicia de verdad. Un arnes que no se ha visto fallar es un adorno que da verde.

  comprobar(std::string(MAESTRO.nombre()) == "MAESTRO" &&
            std::string(ESCLAVO.nombre()) == "ESCLAVO",
            "E1: hay DOS modulos cargados y cada uno se identifica como su punta "
            "(cargar dos veces el mismo daria un arnes midiendo una punta contra si "
            "misma)");

  comprobar(MAESTRO.h != ESCLAVO.h,
            "E2: los dos HMODULE son distintos: el enlazador de Windows resolvio "
            "semaforo_estado() dentro de cada modulo, que es lo que permite tener los "
            "dos vivos a la vez");

  {
    // Los pines de una punta no son los de la otra. Si compartieran array, "verde
    // simultaneo" seria imposible de medir y el arnes daria verde para siempre.
    MAESTRO.arrancar(); ESCLAVO.arrancar();
    MAESTRO.orden("arrancar_automatico");
    unsigned long eM = MAESTRO.escrituras(), eE = ESCLAVO.escrituras();
    MAESTRO.tick(0);
    comprobar(MAESTRO.escrituras() >= eM && ESCLAVO.escrituras() == eE,
              "E3: un tick del Maestro no mueve ni un pin del Esclavo: cada punta "
              "escribe en SU propio arnes_pines[]");
  }

  comprobar(hayVerdeSimultaneo(true, true) &&
            !hayVerdeSimultaneo(true, false) &&
            !hayVerdeSimultaneo(false, true) &&
            !hayVerdeSimultaneo(false, false),
            "E4 (control negativo): el detector de verde simultaneo SI dispara con las "
            "dos en verde y NO dispara con ninguna de las otras tres combinaciones");

  {
    // El microcorte tiene que reiniciar de verdad. Se lleva al Esclavo a un estado
    // distinto del de arranque -unas cuantas escrituras de pin- y se exige que tras el
    // corte el contador vuelva a empezar. Si FreeLibrary no descargara el modulo, el
    // escenario de reinicio asimetrico seria un fraude silencioso.
    escenarioLimpio(0);
    avanzar(20000);
    unsigned long antes = ESCLAVO.escrituras();
    long recargasAntes = ESCLAVO.orden("recargas_watchdog");
    microcorte(ESCLAVO);
    comprobar(antes > 0 && ESCLAVO.escrituras() < antes && recargasAntes > 0 &&
              ESCLAVO.orden("recargas_watchdog") == 0,
              "E5: el microcorte REINICIA de verdad (las estaticas del Esclavo vuelven "
              "al arranque: " + std::to_string(antes) + " escrituras y " +
              std::to_string(recargasAntes) + " recargas de watchdog antes, y a cero "
              "despues). Es lo que hace honesto todo el bloque B");
  }

  {
    // Y el dominio de respaldo NO se pierde en ese corte: lo mantiene la pila.
    escenarioLimpio(0);
    sincronizarEsclavo(10, 12, 0, 0);
    long dominioAntes[PUNTA_DOMINIO_PALABRAS];
    for (int i = 0; i < PUNTA_DOMINIO_PALABRAS; i++) dominioAntes[i] = ESCLAVO.domLeer(i);
    bool habiaAlgo = false;
    for (int i = 0; i < 10; i++) if (dominioAntes[i] != 0) habiaAlgo = true;
    microcorte(ESCLAVO);
    bool iguales = true;
    for (int i = 0; i < 10; i++) if (ESCLAVO.domLeer(i) != dominioAntes[i]) iguales = false;
    comprobar(habiaAlgo && iguales,
              "E6: el dominio de respaldo (BKP->DR1..DR10, el que mantiene la CR2032) "
              "SOBREVIVE al microcorte. Sin esto el Modo Degradado no podria reanudar "
              "nunca y el bloque B mediria un equipo que no existe");
  }

  // =========================================================================
  std::printf("\n--- BLOQUE A: el lazo normal, las dos puntas de verdad -----------\n");
  // El control positivo de todo lo demas. Una guarda que no dejara pasar NADA haria
  // pasar las comprobaciones de verde simultaneo igual de bien que el firmware
  // correcto: sin ver primero que las dos puntas CICLAN, no se estaria midiendo
  // seguridad, se estaria midiendo una tapia.
  {
    escenarioLimpio(tiempos(1, 1, 15));
    unsigned long vM0 = g_ticksVerdeMaestro, vE0 = g_ticksVerdeEsclavo;
    unsigned long sim0 = g_verdeSimultaneo;
    avanzar(400000);   // unos dos ciclos completos de 1 min + 1 min + dos despejes

    comprobar(MAESTRO.orden("en_marcha") == 1,
              "A1: el Maestro llego a CORRIENDO con el asistente por defecto "
              "(si no arrancara, nada de lo que sigue significaria nada)");
    comprobar(g_ticksVerdeMaestro > vM0,
              "A2: el MAESTRO llego a dar verde: " +
              std::to_string(g_ticksVerdeMaestro - vM0) + " instantes con VERDE1/2 "
              "encendido sobre sus pines");
    comprobar(g_ticksVerdeEsclavo > vE0,
              "A3: el ESCLAVO llego a dar verde OBEDECIENDO AL DESPACHADOR REAL de "
              "src/main.cpp: " + std::to_string(g_ticksVerdeEsclavo - vE0) +
              " instantes. Es la punta que ningun arnes habia ejecutado nunca");
    comprobar(g_verdeSimultaneo == sim0,
              "A4: en los " + std::to_string(400000 / PASO_MS) + " instantes del ciclo "
              "normal NUNCA hubo verde en las dos puntas a la vez, medido sobre los "
              "doce pines del MISMO tick");
    comprobar(ESCLAVO.orden("tramas_emitidas") > 0 && g_tramasEntregadas > 0,
              "A5: las dos puntas se hablaron de verdad: " +
              std::to_string(g_tramasEntregadas) + " tramas cruzaron el canal");
  }

  {
    // N-96 medido, no supuesto: seis pines vivos y tres muertos, en LAS DOS puntas.
    long muertos = ESCLAVO.toques(ROJO_PEATON) + ESCLAVO.toques(VERDE_PEATON) +
                   ESCLAVO.toques(BUZZER) + MAESTRO.toques(ROJO_PEATON) +
                   MAESTRO.toques(VERDE_PEATON) + MAESTRO.toques(BUZZER);
    comprobar(muertos == 0,
              "A6 (N-96): ROJO_PEATON, VERDE_PEATON y el BUZZER no recibieron NI UNA "
              "escritura en ninguna de las dos puntas. La regla 6 enumera ocho pines y "
              "el firmware mueve seis: esto lo mide en vez de suponerlo");

    int vivos[6] = { ROJO1, AMARILLO1, VERDE1, ROJO2, AMARILLO2, VERDE2 };
    bool todosVivos = true;
    for (int p : vivos) {
      if (MAESTRO.toques(p) == 0 || ESCLAVO.toques(p) == 0) todosVivos = false;
    }
    comprobar(todosVivos,
              "A7 (N-96, la otra mitad): los SEIS pines de luz SI se escribieron en las "
              "dos puntas. Una regla de seguridad que enumera sujetos tiene que "
              "comprobar que cada sujeto existe, no solo que nadie la rodea");
  }

  comprobar(g_enclavamientoRoto == 0,
            "A8 (SFTY-2): en ningun instante coincidieron ROJO y VERDE encendidos en la "
            "misma cara, en ninguna de las dos puntas");
  comprobar(g_verdeSinRojoEnfrente == 0,
            "A9: con una punta en verde, la otra tuvo SIEMPRE los dos rojos encendidos "
            "-no basta con 'no verde': ambar o apagado frente a un verde es una via sin "
            "rojo-. La excepcion de S_FALLO va por nombre");

  // =========================================================================
  std::printf("\n--- BLOQUE B: una punta se reinicia y la otra no -----------------\n");
  // OPTIMIZACIONES.md:422 da esa salida asimetrica por riesgo residual aceptado: un
  // microcorte basta. Aqui se ejerce en los cuatro momentos que importan.
  {
    struct Caso { const char* nombre; bool cortarEsclavo; bool durantVerdeEsclavo; };
    Caso casos[4] = {
      { "B1: corte del ESCLAVO en mitad de SU verde",        true,  true  },
      { "B2: corte del ESCLAVO en mitad del verde del MAESTRO", true,  false },
      { "B3: corte del MAESTRO en mitad del verde del ESCLAVO", false, true  },
      { "B4: corte del MAESTRO en mitad de SU propio verde",  false, false },
    };

    for (const Caso& c : casos) {
      escenarioLimpio(tiempos(1, 1, 15));
      unsigned long sim0 = g_verdeSimultaneo;

      // Se avanza hasta pillar el verde que pide el caso, con presupuesto acotado.
      bool pillado = false;
      for (unsigned long gastado = 0; gastado < 400000; gastado += PASO_MS) {
        unTick();
        bool objetivo = c.durantVerdeEsclavo ? ESCLAVO.verde() : MAESTRO.verde();
        if (objetivo) { pillado = true; break; }
      }
      comprobar(pillado,
                std::string(c.nombre) + " - se alcanzo el verde sobre el que hay que "
                "cortar (sin el, este caso no mediria nada)");
      if (!pillado) continue;

      avanzar(3000);   // ya bien dentro del verde
      microcorte(c.cortarEsclavo ? ESCLAVO : MAESTRO);
      if (!c.cortarEsclavo) MAESTRO.orden("arrancar_automatico");  // el operario lo rearranca

      avanzar(120000);
      comprobar(g_verdeSimultaneo == sim0,
                std::string(c.nombre) + " - la punta reiniciada arranca en rojo y en "
                "los 2 min siguientes NUNCA coincidio un verde con el de la otra");
    }
  }

  // =========================================================================
  std::printf("\n--- BLOQUE C: la radio se cae en distintos momentos --------------\n");
  // El silencio se corta en cinco instantes repartidos por el ciclo, y ademas EN UNA
  // SOLA DIRECCION, que es la averia que ningun modelo estaba ejerciendo.
  {
    // Cinco instantes repartidos por el ciclo: dentro del primer todo-rojo, en el
    // primer verde, en el despeje siguiente, en el verde de la otra punta y ya en la
    // segunda vuelta. Ninguno coincide a proposito con el techo de silencio: una
    // coincidencia entre un instante de escenario y una constante del firmware es la
    // clase de acoplamiento que cambia de significado sin que nadie lo pida.
    const unsigned long momentos[5] = { 5000, 22000, 45000, 70000, 95000 };
    for (int i = 0; i < 5; i++) {
      escenarioLimpio(tiempos(1, 1, 15));
      unsigned long sim0 = g_verdeSimultaneo;
      avanzar(momentos[i]);
      g_enlaceHaciaEsclavo = false;
      g_enlaceHaciaMaestro = false;
      // Se deja correr mas del techo de silencio mas el presupuesto entero de
      // reintentos, leidos los dos del C++ real.
      avanzar(SFTY6_SILENCIO_MS_V + TIMEOUT_ACK_MS_V * (CICLO_MAX_REINTENTOS_V + 2) + 30000);

      bool ningunVerde = !MAESTRO.verde() && !ESCLAVO.verde();
      comprobar(g_verdeSimultaneo == sim0 && ningunVerde,
                "C" + std::to_string(i + 1) + ": enlace cortado en t=" +
                std::to_string(momentos[i]) + " ms. Ni verde simultaneo durante la "
                "caida, ni verde en ninguna punta pasado el techo de silencio (" +
                std::to_string(SFTY6_SILENCIO_MS_V) + " ms)");
    }

    // La direccion unica: el Maestro sigue oyendo al Esclavo, el Esclavo no oye al
    // Maestro. Es peor que el corte total porque cada punta ve una averia distinta.
    for (int dir = 0; dir < 2; dir++) {
      escenarioLimpio(tiempos(1, 1, 15));
      unsigned long sim0 = g_verdeSimultaneo;
      avanzar(30000);
      if (dir == 0) g_enlaceHaciaEsclavo = false; else g_enlaceHaciaMaestro = false;
      avanzar(SFTY6_SILENCIO_MS_V + TIMEOUT_ACK_MS_V * (CICLO_MAX_REINTENTOS_V + 2) + 30000);
      comprobar(g_verdeSimultaneo == sim0,
                std::string("C") + std::to_string(6 + dir) + ": enlace roto SOLO en la "
                "direccion " + (dir == 0 ? "Maestro -> Esclavo" : "Esclavo -> Maestro") +
                ". Cada punta ve una averia distinta y aun asi no coincide un verde");
      comprobar(!MAESTRO.verde() || !ESCLAVO.verde(),
                std::string("C") + std::to_string(6 + dir) + " (final): al terminar el "
                "escenario asimetrico, como mucho UNA de las dos puntas tiene verde");
    }
  }

  // =========================================================================
  std::printf("\n--- BLOQUE D: dos autoridades y configuraciones distintas --------\n");
  // El Esclavo en Modo Degradado decide su luz POR RELOJ, con la configuracion que el
  // Maestro le dejo. Si esa configuracion no es la que el Maestro esta usando, hay dos
  // ciclos de distinta duracion sobre el mismo cruce. Aqui se monta a proposito.
  {
    escenarioLimpio(tiempos(1, 1, 15));
    unsigned long sim0 = g_verdeSimultaneo;

    // Se le da al Esclavo hora y un ciclo DISTINTO del que corre el Maestro,
    // por el camino real: tramas que procesa su despachador.
    sincronizarEsclavo(10, 8, 0, 0);
    configurarEsclavo(20, 10);   // 20 s de verde y 10 s de despeje contra 60 s y 15 s

    comprobar(ESCLAVO.orden("config_verde") == 20 && ESCLAVO.orden("config_despeje") == 10,
              "D1: el Esclavo acepto por radio una configuracion de ciclo DISTINTA de la "
              "del Maestro (20 s / 10 s contra 60 s / 15 s). Sin esto el bloque no "
              "mediria configuraciones distintas, mediria dos veces la misma");

    long rechazo = ESCLAVO.orden("degradado_comprobar");
    comprobar(rechazo == 0,
              "D2: con hora y configuracion, el Esclavo declara que PUEDE entrar en "
              "Modo Degradado (degradado_comprobar = " + std::to_string(rechazo) + ", "
              "0 = aceptado). Es el control positivo del bloque");

    ESCLAVO.orden("degradado_entrar");
    avanzar(2000);
    comprobar(ESCLAVO.orden("degradado_gobierna") == 1,
              "D3: el Modo Degradado del Esclavo GOBIERNA la luz: hay dos autoridades "
              "vivas sobre el mismo cruce, el reloj de esta punta y el coordinador de "
              "la otra");

    // Con el enlace vivo, la primera trama de gobierno tiene que sacarlo por la via
    // ordenada. Lo que se mide es que en TODA la transicion no coincida un verde.
    avanzar(200000);
    comprobar(g_verdeSimultaneo == sim0,
              "D4: durante la entrada y la salida del Degradado con el Maestro ciclando "
              "por radio y una configuracion distinta, NUNCA coincidio un verde en las "
              "dos puntas");
  }

  {
    // El caso feo de N-20 combinado con el reinicio asimetrico: el Esclavo se corta
    // estando en Degradado, REANUDA por su cuenta al arrancar, y el Maestro ni se ha
    // enterado. Dos autoridades y una de ellas acaba de nacer.
    escenarioLimpio(tiempos(1, 1, 15));
    unsigned long sim0 = g_verdeSimultaneo;
    sincronizarEsclavo(10, 8, 0, 0);
    configurarEsclavo(20, 10);
    g_enlaceHaciaEsclavo = false;
    g_enlaceHaciaMaestro = false;
    ESCLAVO.orden("degradado_entrar");
    avanzar(30000);
    bool gobernabaAntes = ESCLAVO.orden("degradado_gobierna") == 1;
    microcorte(ESCLAVO);
    avanzar(120000);
    comprobar(gobernabaAntes,
              "D5: el Esclavo estaba gobernando por reloj cuando se le corto la energia "
              "(sin eso, la reanudacion no se estaria ejerciendo)");
    comprobar(g_verdeSimultaneo == sim0,
              "D6: tras el microcorte, con el Esclavo reanudando -o no- el Degradado por "
              "su cuenta y el Maestro sin enterarse, NUNCA coincidio un verde");
  }

  {
    // La guarda de N-83 del despachador real: con el ambar de Bluetooth pedido, un
    // CMD_GO_GREEN del Maestro NO enciende verde en el Esclavo. Es una de las dos
    // unicas ramas que pueden vetar una orden de verde, y vive en src/main.cpp.
    escenarioLimpio(tiempos(1, 1, 15));
    unsigned long verdeE0 = g_ticksVerdeEsclavo;
    ESCLAVO.orden("ambar_bluetooth", 1);
    avanzar(300000);
    comprobar(g_ticksVerdeEsclavo == verdeE0,
              "D7: con el ambar de emergencia pedido por la app, el despachador REAL del "
              "Esclavo no encendio verde ni una sola vez en 5 minutos de ordenes del "
              "Maestro (la guarda de N-83, ejercida sobre el .cpp y no sobre su texto)");
    ESCLAVO.orden("ambar_bluetooth", 0);
  }

  // =========================================================================
  std::printf("\n==============================================================\n");
  comprobar(g_verdeSimultaneo == 0,
            "RESUMEN: en los " + std::to_string(g_instantes) + " instantes observados "
            "de TODO el barrido -bloques A a D- no hubo NI UNO con verde encendido en "
            "las dos puntas. Es la propiedad que motivo este arnes, medida sobre el C++ "
            "real de las dos y sobre lo que escribio en los pines");
  comprobar(g_enclavamientoRoto == 0,
            "RESUMEN (SFTY-2): en ninguno de esos instantes coincidieron rojo y verde "
            "en la misma cara de ninguna de las dos puntas");
  comprobar(g_talanqueraSinVerde == 0,
            "RESUMEN (SFTY-28): en ninguno de esos instantes hubo pluma arriba sin verde "
            "encendido fuera de S_FALLO, en ninguna de las dos puntas");

  std::printf("\n==============================================================\n");
  std::printf(" RESULTADO: %d/%d comprobaciones OK\n", total - fallos, total);
  std::printf("==============================================================\n");
  std::printf(" %lu instantes observados. Verde del Maestro en %lu, del Esclavo en %lu,\n",
              g_instantes, g_ticksVerdeMaestro, g_ticksVerdeEsclavo);
  std::printf(" DE LOS DOS A LA VEZ EN %lu.\n", g_verdeSimultaneo);
  if (g_verdeSimultaneo) {
    std::printf(" Primer instante con verde simultaneo: t = %lu ms.\n", g_primerSimultaneoMs);
  }
  std::printf(" %lu tramas entregadas, %lu perdidas por enlace cortado.\n",
              g_tramasEntregadas, g_tramasPerdidas);
  std::printf(" Medido sobre el C++ REAL de las DOS puntas -cuatro ficheros del Maestro\n");
  std::printf(" y SIETE del Esclavo, src/main.cpp incluido- ejecutandose en el mismo\n");
  std::printf(" proceso y en el mismo instante. Ningun paso reimplementa el ciclo.\n");

  MAESTRO.descargar();
  ESCLAVO.descargar();
  return fallos == 0 ? 0 : 1;
}
