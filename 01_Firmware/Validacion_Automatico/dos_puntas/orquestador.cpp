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
// D-31: el plazo del acuse del aviso de ambar y sus reintentos, leidos de protocolo.h. El
// bloque H4 tiene que dejar correr el intento Y sus reintentos antes de exigir la alarma:
// con un presupuesto escrito a mano, el dia que el numero cambie el escenario preguntaria
// antes de tiempo y llamaria defecto a un firmware que estaba esperando.
static unsigned long AVISO_AMBAR_TIMEOUT_MS_V;
static unsigned long AVISO_AMBAR_REINTENTOS_V;
static unsigned long DESPEJE_POR_DEFECTO_S;
static unsigned long AMBAR_ESCLAVO_MS_V;
// N-162 (bloque G): los tres plazos que deciden cuanto dura una punta en verde frente a
// otra abierta. Releidos, no copiados: el bloque los usa para explicar la cifra que mide.
static unsigned long LATIDO_MS_V;            // Maestro: cadencia del latido (PING o GO_RED)
static unsigned long RETARDO_RESPUESTA_MS_V; // Esclavo: cortesia antes de contestar (SFTY-17)
static unsigned long MAX_VERDE_BACKSTOP_MS_V;// Esclavo: verde maximo, vigilante del final del bucle
static unsigned long VERDE_MIN_MIN_V, ROJO_MIN_MIN_V;   // limites_ciclo.h: el ciclo del bloque G
// N-162 (bloque D): modo_degradado.cpp del Esclavo, el limite duro sin sync, EN HORAS.
static unsigned long LIMITE_SIN_SYNC_H_V;
// D-29 (bloque D): la ventana en la que el permiso de la pila se conserva esperando la
// primera siembra del ESP32. Se DERIVA de reloj.h -cadencia x multiplicador-, que es de
// donde la deriva el firmware, y no se escribe aqui ningun numero.
static unsigned long VENTANA_REANUDACION_MS_V;
// D-29 (bloque D): el prefijo de la linea que el ESP32 pone por J17, leido del
// despachador REAL del Esclavo. Un literal escrito aqui mediria el comando de ayer el dia
// que alguien lo renombre, que es lo que ya paso con FORZAR_ROJO (N-83).
static std::string PREFIJO_HORA_ESP32;

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
// N-142 / §3.16-A (11/09): LA LINEA QUE TECLEA EL TELEFONO, LEIDA DEL C++.
//
// El bloque H pide el ambar de emergencia por la puerta SIN PIN del Esclavo -la que usa
// la app-, y para eso hace falta el literal del comando. NO SE ESCRIBE AQUI: se deduce
// del despachador igual que lo deducen esclavo_07 y esclavo_08 -la rama comparada contra
// 'cmd' cuyo bloque llama a semaforo_iniciarFallo()-, porque un literal escrito en el
// arnes seguiria midiendo el comando de ayer el dia que se renombre. Es lo que paso con
// FORZAR_ROJO (N-83), y el arnes habria seguido en verde tecleando un comando muerto.
//
// Los comentarios se quitan ANTES de buscar: este fichero cita sus propios comandos
// dentro de los comentarios -por eso los packs leen el fuente sin ellos-, y un lector que
// los cuente encuentra la rama equivocada.
static std::string sinComentarios(const std::string& src) {
  std::string fuera;
  fuera.reserve(src.size());
  for (size_t i = 0; i < src.size();) {
    if (src[i] == '/' && i + 1 < src.size() && src[i + 1] == '/') {
      while (i < src.size() && src[i] != '\n') i++;
    } else if (src[i] == '/' && i + 1 < src.size() && src[i + 1] == '*') {
      i += 2;
      while (i + 1 < src.size() && !(src[i] == '*' && src[i + 1] == '/')) i++;
      i = (i + 2 < src.size()) ? i + 2 : src.size();
    } else {
      fuera += src[i++];
    }
  }
  return fuera;
}

// El bloque de la rama que empieza en 'desde', hasta la siguiente comparacion o hasta su
// return. Mismo corte que el lector de los packs, y por el mismo motivo: sin el, el
// filtro de PIN que vive entre dos ramas cae dentro de la de arriba.
static std::string bloqueDeRama(const std::string& src, size_t desde) {
  // Desde DESPUES de la propia comparacion: buscar "strcmp" desde 'desde' se encuentra a
  // si misma y el bloque sale vacio -medido: el lector no hallaba ninguna rama-.
  const size_t sig = src.find("strcmp", desde + 6);
  size_t fin = (sig == std::string::npos) ? src.size() : sig;
  const size_t ret = src.find("return;", desde + 6);
  if (ret != std::string::npos && ret < fin) fin = ret;
  return src.substr(desde, fin - desde);
}

// Devuelve el literal de la puerta SIN PIN del ambar de emergencia (la comparada contra
// 'cmd') y, en 'results', los RESULT que esa rama puede contestar. Cadena vacia si no la
// encuentra: quien llama decide si eso es un ABORTADO o el caso malo de un control.
static std::string literalPuertaAmbar(const std::string& fuente,
                                      std::vector<std::string>* results) {
  const std::string src = sinComentarios(fuente);
  std::regex re(R"(strcmp\s*\(\s*cmd\s*,\s*\"([^\"]+)\"\s*\))");
  for (std::sregex_iterator it(src.begin(), src.end(), re), fin; it != fin; ++it) {
    const size_t desde = (size_t)it->position(0);
    const std::string bloque = bloqueDeRama(src, desde);
    if (bloque.find("semaforo_iniciarFallo") == std::string::npos) continue;
    if (results) {
      results->clear();
      std::regex reRes(R"(\"\$ACK,CMD:[A-Z0-9_:]+,RESULT:([A-Z0-9_]+)\")");
      for (std::sregex_iterator r(bloque.begin(), bloque.end(), reRes), rf; r != rf; ++r) {
        const std::string v = (*r)[1].str();
        bool ya = false;
        for (const std::string& x : *results) if (x == v) ya = true;
        if (!ya) results->push_back(v);
      }
    }
    return (*it)[1].str();
  }
  return std::string();
}

static std::string LINEA_AMBAR_APP;                  // "CMD:AMBAR_EMERGENCIA" hoy
static std::vector<std::string> RESULTS_AMBAR_APP;   // lo que esa rama puede contestar

// ---------------------------------------------------------------------------
// D-31 (12/09): LOS TRES CAMPOS DE LA ALARMA DEL AVISO, LEIDOS DEL C++.
//
// Ninguno se escribe aqui, por lo mismo que la linea del telefono: un literal escrito en
// el arnes sigue midiendo el mensaje de ayer el dia que se renombre, y el bloque seguiria
// en verde exigiendo una alarma que ya no existe.
//
// EL ANCLA ES AVISO_AMBAR_REINTENTOS, y se elige asi porque es lo unico que identifica a
// ESTA alarma y no a las otras cuatro que este fichero emite: la del aviso es la que vive
// detras del contador de reintentos del aviso. Anclarla al nombre del evento seria
// escribir el literal por la puerta de atras. Si el ancla desaparece, esto devuelve vacio
// y quien llama lo trata como ABORTADO -aprobar sin haber leido es fabricar un PASS-.
struct AlarmaAviso {
  std::string evento, causa, accion;
  bool valida() const { return !evento.empty() && !causa.empty() && !accion.empty(); }
};

static AlarmaAviso alarmaDelAviso(const std::string& fuente) {
  AlarmaAviso a;
  const std::string src = sinComentarios(fuente);
  const size_t ancla = src.find("AVISO_AMBAR_REINTENTOS");
  if (ancla == std::string::npos) return a;
  const std::string cola = src.substr(ancla);
  std::regex re(R"(bluetooth_reportarAlarma\s*\(\s*\"([A-Z0-9_]+)\"\s*,\s*\"([A-Z0-9_]+)\"\s*,\s*\"([A-Z0-9_]+)\"\s*\))");
  std::smatch m;
  if (!std::regex_search(cola, m, re)) return a;
  a.evento = m[1].str();
  a.causa = m[2].str();
  a.accion = m[3].str();
  return a;
}

static AlarmaAviso ALARMA_AVISO;

// D-31 (2): el prefijo con el que la app manda las ordenes que piden PIN. Se LEE del
// despachador -el strncmp() contra 'cmd'- porque el dia que el PIN cambie, un prefijo
// escrito aqui haria que el bloque H6 tecleara una linea que el equipo rechaza y el
// escenario aprobaria por no haber cancelado nada.
static std::string prefijoPinApp(const std::string& fuente) {
  const std::string src = sinComentarios(fuente);
  std::regex re(R"(strncmp\s*\(\s*cmd\s*,\s*\"(CMD:PIN:[^\"]*)\"\s*,)");
  std::smatch m;
  if (!std::regex_search(src, m, re)) return std::string();
  return m[1].str();
}

static std::string PREFIJO_PIN_APP;

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

// N-162 (bloque F): PERDIDA SELECTIVA, POR COMANDO Y POR ORDINAL. Cortar la direccion
// entera "el tiempo justo" dependeria de adivinar en que milisegundo sale cada acuse; aqui
// se tiran EXACTAMENTE las tramas que el escenario nombra -los primeros N ACK_GREEN del
// Esclavo, o los GO_GREEN del Maestro con ordinal en [desde, hasta]- y nada mas, y cada
// una se cuenta: el bloque exige despues que la perdida HAYA OCURRIDO tal como se pidio.
// Los codigos se releen de protocolo.h (ver main), no se escriben aqui.
static uint8_t CMD_GO_GREEN_V = 0, CMD_ACK_GREEN_V = 0;
static long g_ackVerdeAPerder = 0;
static unsigned long g_ackVerdePerdidos = 0;
static unsigned long g_goVerdeEmitidos = 0;       // ordinal del ultimo GO_GREEN puesto en el aire
static unsigned long g_goVerdePerderDesde = 0;    // 0 = no se tira ninguno
static unsigned long g_goVerdePerderHasta = 0;
static unsigned long g_goVerdePerdidos = 0;
// GO_GREEN que llegaron al Esclavo, por el estado de SU luz en el instante de entrega.
// Es el control de que el escenario ejercio la rama repetida: sin GO_GREEN entregados en
// AMARILLO o en VERDE, "no reinicia el ambar" pasaria igual con una rama que no existe.
static unsigned long g_goVerdeEntregadoEn[4] = { 0, 0, 0, 0 };

// N-162 (bloque G): el GO_RED es la UNICA orden que saca al Esclavo de un verde mientras le
// siga llegando trafico del Maestro -un PING le refresca la orfandad-. El bloque G necesita
// saber cuantos salieron, cuantos llegaron y cuantos se tiraron, y poder tirar los N
// siguientes a proposito. Codigo releido de protocolo.h (ver main).
static uint8_t CMD_GO_RED_V = 0;
static long g_goRojoAPerder = 0;           // se tiran los N siguientes GO_RED del Maestro
static unsigned long g_goRojoPerdidos = 0; // ...tirados por esa perdida selectiva
static unsigned long g_goRojoCortados = 0; // ...perdidos por tener cortada la direccion
static unsigned long g_goRojoEntregados = 0;

// N-163 (bloque G11): EL SILENCIO QUE CADA PUNTA MIDE DE VERDAD.
//
// Hace falta para DERIVAR cuantos ambares tocan en un barrido de microcortes en vez de
// escribirlos a mano: una cifra escrita a mano aqui aprobaria el firmware que la produjo.
// Y son DOS instantes distintos, que es justo lo que G3 mide:
//   - el Esclavo cuenta desde la ultima trama de GOBIERNO que RECIBIO (tUltimoComando de
//     Esclavo/src/main.cpp: PING, GO_RED o GO_GREEN; las de servicio no lo refrescan),
//   - el Maestro, desde la ultima trama que le LLEGO, sea cual sea (tUltimaRxEsclavo de
//     coordinador.cpp se refresca con CUALQUIER paquete).
// g_entregasEsclavoNoGobierno cuenta lo que este modelo NO sabria fechar: si un escenario
// mete trafico de servicio, la cuenta deja de ser exacta y la linea que la use lo dice.
static uint8_t CMD_PING_V = 0;
static unsigned long g_tGobiernoEsclavo = 0;
static unsigned long g_tRxMaestro = 0;
static unsigned long g_entregasEsclavoNoGobierno = 0;

// N-163: CUANDO EMITIO EL MAESTRO POR ULTIMA VEZ, LLEGARA O NO. Se fecha en la EMISION y
// no en la entrega a proposito: lo que decide si el otro poste se queda huerfano es cada
// cuanto habla esta punta, y eso no depende de que el aire este cortado. Sin esta marca,
// G11 no puede distinguir "el firmware habla igual que antes" de "el firmware habla menos
// y por eso el ambar de enfrente llega antes": la cuenta derivada de ambares sigue al
// canal, asi que sube con la medida y las dos casan igual. Medido: con la version de
// N-163 que salia a C_ESPERANDO_ACK_RED -que SUPRIME el latido (SFTY-13) y reintenta al
// ritmo del timeout de acuse en vez del latido- el hueco crecia lo que va de una cadencia
// a la otra, y el Esclavo entraba en ambar el DOBLE de veces en el mismo barrido de
// cortes. Esta es la linea que lo caza; la de arriba, sola, no puede.
static unsigned long g_tEmisionMaestro = 0;

// D-34 (bloques G13 y G14): el PONG, su param y la reanudacion que dispara. Codigos
// releidos de las dos protocolo.h (ver main). Se tiran los PONG que el Esclavo EMITE con
// g_t en [desde, hasta) -una ventana de tiempo y no un recuento, porque lo que G14 barre es
// la EDAD del ultimo PONG oido al acabar el despeje-. Una REANUDACION es el tick en que al
// Maestro le llega un PONG con PONG_VERDE_SOLTADO y el Maestro pone un GO_RED en el aire en
// ESE MISMO tick: es la firma de la rama de coordinador.cpp, que emite sin esperar a nada.
// NO ES UNICA, Y SE MIDIO (control negativo con la rama anulada): con el Maestro en C_FALLO
// la llegada de CUALQUIER trama le devuelve la comunicacion y SFTY-9 emite su GO_RED en ese
// mismo tick, asi que el contador sube tambien ahi. Por eso G13 solo lo exige en las celdas
// donde el silencio del Maestro NO paso de SFTY6_SILENCIO_MS -sin C_FALLO no hay SFTY-9-.
static uint8_t CMD_PONG_V = 0, PONG_VERDE_SOLTADO_V = 0;
static unsigned long g_pongPerderDesdeT = 0, g_pongPerderHastaT = 0;   // hasta == 0: ninguno
static unsigned long g_pongPerdidos = 0;
static unsigned long g_pongAvisosEntregados = 0;
static unsigned long g_reanudaciones = 0;

// N-162 (bloque G, G9): un ACK_RED RETENIDO en el aire y soltado cuando el escenario diga.
// Es la unica forma en que un acuse viejo puede enganar al Maestro -llegar despues de que
// el Esclavo haya vuelto a verde-, y el arnes no la produce sola: el canal es FIFO y de
// latencia fija.
static uint8_t CMD_ACK_RED_V = 0;
static long g_ackRojoARetener = 0;
static bool g_hayAckRojoRetenido = false;
static unsigned char g_ackRojoRetenido[4];
static unsigned long g_tAckRojoRetenido = 0;

// Los valores del enum EstadoSemaforo, releidos de semaforo.h (ver main). -1 = sin leer.
static int S_VERDE_V = -1, S_AMARILLO_V = -1;
static int S_ROJO_V = -1, S_FALLO_V = -1;

// N-162 (bloque G): cuantas veces la EXCEPCION de A9 -"la otra punta esta en S_FALLO"-
// perdono un instante en los bloques A a F, y su racha mas larga. La excepcion es el
// instrumento de verdad (CLAUDE.md 6): lo que dice es "en S_FALLO ya no hay quien
// gobierne", y eso es falso mientras la OTRA punta tenga un verde fijo encendido.
static unsigned long g_a9Perdonados = 0;
static unsigned long g_a9RachaMax = 0, g_a9Racha = 0;

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

// ---------------------------------------------------------------------------
// D-33 (14/09/2026) - EL REPARTO DE LA INVARIANTE DE LA PLUMA (CLAUDE.md 9).
//
// La linea de arriba afirmaba TRES cosas a la vez, y solo UNA ha cambiado:
//
//   1. "la pluma arriba SIEMPRE tiene una razon nombrada"   -> SE CONSERVA ENTERA.
//   2. "esa razon es el verde encendido"                    -> SE REPARTE: sigue siendo
//      cierta fuera de la bajada, y dentro la razon es el retardo de D-33.
//   3. "la unica excepcion nombrada es S_FALLO"             -> SE CONSERVA LITERAL.
//
// LA EXCEPCION NUEVA NO ES UN PERMISO: ES UNA COTA QUE SE MIDE. Se cronometra cuanto
// dura de verdad cada ventana de "pluma arriba sin verde" y se exige que no pase del
// PLUMA_RETARDO_BAJADA_MS leido del C++ real. Sin esta medida, la excepcion aprobaria
// igual de bien un firmware que dejara la pluma arriba diez minutos -que es justo el
// modo de fallo que D-33 crea-, y seria el "verde porque nadie mira" de CLAUDE.md 6.
//
// AQUI NO HAY CAMARAS (los adaptadores contestan false a camara_presenciaJ16), asi que
// el veto no puede actuar y la unica razon posible es el retardo. Quien ejerce el veto
// con camaras de verdad es el arnes de una punta, que compila botones.cpp REAL.
// El reloj comun del banco. Se declara AQUI ARRIBA -y no donde estaba, junto al bucle-
// porque el observador de la pluma necesita fechar sus ventanas y corre antes.
static unsigned long g_t = 0;
static unsigned long g_plumaSinVerdeDesde[2] = {0, 0};   // 0 = ninguna ventana abierta
static unsigned long g_peorVentanaPlumaMs[2] = {0, 0};
static unsigned long g_retardoPlumaMs = 0;               // leido del C++ en main()
static unsigned long g_verdeSinRojoEnfrente = 0;

// N-162 - LO QUE EL MAESTRO PUBLICA DEL ESCLAVO (campo ESC: del $STATUS). La cinta del
// Sisga (10/09, 12:20:54-12:21:10) lo pillo diciendo VERDE durante todo el despeje y
// durante el ambar del propio Maestro. No mueve una luz: miente en la pantalla que el
// operario mira para decidir.
//   g_escVerdeConMaestroAbierto: ESC dice VERDE y el Maestro esta en AMBAR o VERDE. Eso
//     no es cierto NUNCA -la barrera no los deja coincidir-, asi que no lleva tolerancia.
//   g_escVerdeRojoLargo: ESC dice VERDE y el Esclavo REAL lleva mas de ESC_TOLERANCIA_MS
//     seguidos en rojo y sin verde. EL BORDE ES 1 s Y POR ESTO: entre que el Esclavo pasa
//     a rojo y su ACK_RED llega al Maestro va un viaje de radio (g_latenciaMs = 50 ms en
//     este arnes) mas un tick; ahi el Maestro todavia no puede saberlo y decir VERDE es
//     prudente, no falso. 1 s son 20 veces ese viaje; el despeje que tapaba son 15 s.
static const unsigned long ESC_TOLERANCIA_MS = 1000;
static unsigned long g_escVerdeTicks = 0;
static unsigned long g_escVerdeConMaestroAbierto = 0;
static unsigned long g_escVerdeRojoLargo = 0;
static unsigned long g_escVerdeRojoDesde = 0;
static bool g_escVerdeRojoEnCurso = false;

// N-162 (bloque F): el Esclavo pasando de VERDE a AMARILLO. La Resolucion da verde->rojo
// DIRECTO, y el unico camino que escribe S_AMARILLO en el Esclavo es
// semaforo_iniciarTransicionAVerde(); de verde solo se puede llegar ahi por una orden de
// verde repetida que reinicie la transicion. Se cuenta en TODO el barrido, no solo en F.
static unsigned long g_escVerdeAAmbar = 0;
static int g_estadoEscAnt = -1;

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
  // N-162 (bloque G): y los instantes que esa excepcion PERDONO. No cambia A9: la mide.
  if ((vM && !vE && ESCLAVO.estado() == S_FALLO) || (vE && !vM && MAESTRO.estado() == S_FALLO)) {
    g_a9Perdonados++;
    if (++g_a9Racha > g_a9RachaMax) g_a9RachaMax = g_a9Racha;
  } else {
    g_a9Racha = 0;
  }

  // N-162: lo que se PUBLICA del Esclavo contra lo que el Esclavo TIENE encendido.
  const bool escVerde = MAESTRO.orden("esc_publica_verde") == 1;
  if (escVerde) g_escVerdeTicks++;
  if (escVerde && (vM || MAESTRO.ambar())) g_escVerdeConMaestroAbierto++;
  if (escVerde && !vE && ESCLAVO.rojo()) {
    if (!g_escVerdeRojoEnCurso) { g_escVerdeRojoEnCurso = true; g_escVerdeRojoDesde = t; }
    if (t - g_escVerdeRojoDesde > ESC_TOLERANCIA_MS) g_escVerdeRojoLargo++;
  } else {
    g_escVerdeRojoEnCurso = false;
  }

  // N-162: verde -> ambar en el Esclavo.
  const int eAhora = ESCLAVO.estado();
  if (g_estadoEscAnt == S_VERDE_V && eAhora == S_AMARILLO_V) g_escVerdeAAmbar++;
  g_estadoEscAnt = eAhora;

  // SFTY-2 dentro de cada punta, sobre lo que se escribio en el pin.
  Punta* dos[2] = { &MAESTRO, &ESCLAVO };
  for (int i = 0; i < 2; i++) {
    Punta* p = dos[i];
    if ((p->pin(ROJO1) == HIGH && p->pin(VERDE1) == HIGH) ||
        (p->pin(ROJO2) == HIGH && p->pin(VERDE2) == HIGH)) {
      g_enclavamientoRoto++;
    }
    // SFTY-28 con la derogacion parcial de D-33: pluma arriba sin verde. La excepcion de
    // S_FALLO va por nombre, y la del retardo va CRONOMETRADA -no por nombre-: se cuenta
    // violacion solo cuando la ventana ya paso del plazo que el C++ declara.
    if (p->pin(MOTOR_TALANQUERA) == TALANQUERA_ABRIR &&
        p->pin(VERDE1) != HIGH && p->pin(VERDE2) != HIGH &&
        p->estado() != S_FALLO) {
      if (g_plumaSinVerdeDesde[i] == 0) g_plumaSinVerdeDesde[i] = g_t + 1;
      const unsigned long dur = g_t - (g_plumaSinVerdeDesde[i] - 1);
      if (dur > g_peorVentanaPlumaMs[i]) g_peorVentanaPlumaMs[i] = dur;
      if (dur > g_retardoPlumaMs) {
        g_talanqueraSinVerde++;
      }
    } else {
      g_plumaSinVerdeDesde[i] = 0;
    }
  }
}

// ---------------------------------------------------------------------------
// EL BUCLE. Un tick = el MISMO millis() en las dos puntas.
// ---------------------------------------------------------------------------
static const unsigned long PASO_MS = 50;

static void unTick() {
  bool pongAvisoEsteTick = false;   // D-34
  // 1. Lo que ya vencio en el aire se entrega ANTES de que las puntas corran.
  for (size_t i = 0; i < g_aire.size();) {
    if (g_aire[i].tEntrega <= g_t) {
      Punta& d = (g_aire[i].destino == 0) ? MAESTRO : ESCLAVO;
      if (g_aire[i].destino == 0 && g_aire[i].trama[1] == CMD_PONG_V &&
          g_aire[i].trama[2] == PONG_VERDE_SOLTADO_V) {
        pongAvisoEsteTick = true;
        g_pongAvisosEntregados++;
      }
      // N-162: con que luz encuentra al Esclavo cada GO_GREEN que le llega.
      if (g_aire[i].destino == 1 && g_aire[i].trama[1] == CMD_GO_GREEN_V) {
        const int e = ESCLAVO.estado();
        if (e >= 0 && e < 4) g_goVerdeEntregadoEn[e]++;
      }
      if (g_aire[i].destino == 1 && g_aire[i].trama[1] == CMD_GO_RED_V) g_goRojoEntregados++;
      // N-163: el reloj de silencio de cada punta, fechado sobre la ENTREGA.
      if (g_aire[i].destino == 1) {
        const uint8_t c163 = g_aire[i].trama[1];
        // D-34 (CLAUDE.md 9): el GO_GREEN que encuentra la luz en AMARILLO o en VERDE es una
        // repeticion y ya NO refresca tUltimoComando. Se mira la luz ANTES de entregarla
        // -d.rx() va debajo-, que es lo que ve la guarda de main.cpp. Sin esto el modelo
        // fecharia el silencio del Esclavo con cada repeticion y derivaria menos ambares.
        const bool goVerdeRepetido =
            (c163 == CMD_GO_GREEN_V &&
             (ESCLAVO.estado() == S_AMARILLO_V || ESCLAVO.estado() == S_VERDE_V));
        if (c163 == CMD_PING_V || c163 == CMD_GO_RED_V ||
            (c163 == CMD_GO_GREEN_V && !goVerdeRepetido))
          g_tGobiernoEsclavo = g_t;
        else if (!goVerdeRepetido)
          g_entregasEsclavoNoGobierno++;
      } else {
        g_tRxMaestro = g_t;
      }
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
    g_tEmisionMaestro = g_t;   // N-163: emitida. Que llegue o no es cosa del aire.
    // D-34: se cuenta en la EMISION, antes de cualquier perdida: la reanudacion es lo que
    // el Maestro decide, no lo que el aire deja pasar.
    if (pongAvisoEsteTick && b[1] == CMD_GO_RED_V) { g_reanudaciones++; pongAvisoEsteTick = false; }
    // N-162: se tiran los GO_GREEN cuyo ordinal cae en [desde, hasta].
    if (b[1] == CMD_GO_GREEN_V) {
      g_goVerdeEmitidos++;
      if (g_goVerdePerderDesde != 0 && g_goVerdeEmitidos >= g_goVerdePerderDesde &&
          g_goVerdeEmitidos <= g_goVerdePerderHasta) {
        g_goVerdePerdidos++;
        g_tramasPerdidas++;
        continue;
      }
    }
    // N-162 (bloque G): se tiran los g_goRojoAPerder GO_RED siguientes.
    if (b[1] == CMD_GO_RED_V && g_goRojoAPerder > 0) {
      g_goRojoAPerder--;
      g_goRojoPerdidos++;
      g_tramasPerdidas++;
      continue;
    }
    if (g_enlaceHaciaEsclavo) {
      EnVuelo e; memcpy(e.trama, b, 4); e.tEntrega = g_t + g_latenciaMs; e.destino = 1;
      g_aire.push_back(e);
    } else {
      if (b[1] == CMD_GO_RED_V) g_goRojoCortados++;
      g_tramasPerdidas++;
    }
  }
  while (ESCLAVO.tx(b)) {
    // D-34 (G14): los PONG emitidos dentro de la ventana de perdida.
    if (b[1] == CMD_PONG_V && g_pongPerderHastaT != 0 && g_t >= g_pongPerderDesdeT &&
        g_t < g_pongPerderHastaT) {
      g_pongPerdidos++;
      g_tramasPerdidas++;
      continue;
    }
    // N-162: se tiran los primeros g_ackVerdeAPerder ACK_GREEN del Esclavo.
    if (b[1] == CMD_ACK_GREEN_V && g_ackVerdeAPerder > 0) {
      g_ackVerdeAPerder--;
      g_ackVerdePerdidos++;
      g_tramasPerdidas++;
      continue;
    }
    // N-162 (bloque G, G9): se retiene el siguiente ACK_RED hasta soltarAckRojo().
    if (b[1] == CMD_ACK_RED_V && g_ackRojoARetener > 0) {
      g_ackRojoARetener--;
      memcpy(g_ackRojoRetenido, b, 4);
      g_hayAckRojoRetenido = true;
      g_tAckRojoRetenido = g_t;
      continue;
    }
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

// N-162 (bloque G, G9): el ACK_RED retenido sale ahora, con un viaje de radio normal.
static void soltarAckRojo() {
  if (!g_hayAckRojoRetenido) return;
  EnVuelo e;
  memcpy(e.trama, g_ackRojoRetenido, 4);
  e.tEntrega = g_t + g_latenciaMs;
  e.destino = 0;
  g_aire.push_back(e);
  g_hayAckRojoRetenido = false;
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
// N-162 (12/09): rtcHwEscrito entrega el dominio con el marcador del RTC HARDWARE puesto
// (indice 11), y solo el bloque D lo usa. No es una puerta de atras al firmware: es el
// SILICIO de un equipo cuyo RTC quedo escrito por un firmware ANTERIOR al 11/09, que es la
// unica forma en que ese marcador puede estar puesto hoy. Sin el, la linea que dice "tras
// un corte ya no reanuda" la aprobaria igual de bien un escenario que no sabe reanudar
// nada (CLAUDE.md §9: el control que le falta a toda inversion). -1 = no se toca.
static void microcorte(Punta& p, int rtcHwEscrito = -1) {
  long dominio[PUNTA_DOMINIO_PALABRAS];
  for (int i = 0; i < PUNTA_DOMINIO_PALABRAS; i++) dominio[i] = p.domLeer(i);
  if (rtcHwEscrito >= 0) dominio[11] = rtcHwEscrito;
  p.descargar();
  p.cargar();
  for (int i = 0; i < PUNTA_DOMINIO_PALABRAS; i++) p.domEscribir(i, dominio[i]);
  p.arrancar();
}

// Deja las dos puntas recien arrancadas y el canal limpio. No es cosmetica: sin esto,
// una trama en vuelo de un escenario llegaria al siguiente y el arnes estaria midiendo
// una averia que el mismo fabrico.
//
// N-162 (bloque G): exigirTiempos. MEDIDO: fijar_tiempos(1, 1, 15) -lo que piden los bloques
// A a F- lo RECHAZA modoAutomatico_fijarTiempos() desde N-137 (verde y rojo minimos de 3
// min), y aqui se ignoraba el valor devuelto: esos bloques corren con 3 min / 3 min / 10 s,
// no con lo que dicen sus textos. El bloque G pide que un rechazo ABORTE. Los bloques
// anteriores no se tocan en este cambio: su arreglo mueve todos sus tiempos y va aparte.
static void escenarioLimpio(long tiemposMaestro, bool exigirTiempos = false) {
  g_aire.clear();
  g_enlaceHaciaEsclavo = g_enlaceHaciaMaestro = true;
  // N-162: ningun escenario hereda la perdida selectiva del anterior.
  g_ackVerdeAPerder = 0;
  g_goVerdeEmitidos = 0;
  g_goVerdePerderDesde = g_goVerdePerderHasta = 0;
  g_goRojoAPerder = 0;
  g_ackRojoARetener = 0;
  g_hayAckRojoRetenido = false;
  g_pongPerderDesdeT = g_pongPerderHastaT = 0;   // D-34
  MAESTRO.descargar(); MAESTRO.cargar(); MAESTRO.arrancar();
  ESCLAVO.descargar(); ESCLAVO.cargar(); ESCLAVO.arrancar();
  if (tiemposMaestro > 0 && MAESTRO.orden("fijar_tiempos", tiemposMaestro) != 1 && exigirTiempos)
    abortar("el Maestro RECHAZO fijar_tiempos(" + std::to_string(tiemposMaestro) + "): el "
            "escenario correria con otros tiempos que los que dice medir");
  MAESTRO.orden("arrancar_automatico");
  // N-163: las dos puntas acaban de arrancar, asi que su reloj de silencio empieza AQUI.
  // Heredar el del escenario anterior daria un silencio ya vencido en el primer tick.
  g_tGobiernoEsclavo = g_tRxMaestro = g_tEmisionMaestro = g_t;
  g_entregasEsclavoNoGobierno = 0;
  avanzar(500);
}

// ---------------------------------------------------------------------------
// D-29 — EL RELOJ DEL BANCO VUELVE A CERO, Y HAY QUE DECIR POR QUE Y QUE CUESTA.
//
// El arnes tiene UN reloj absoluto (g_t) que nunca vuelve, y punta_tick() se lo impone a
// las dos puntas. Un microcorte recarga la DLL -las estaticas del firmware vuelven a su
// valor de arranque- pero NO devuelve ese reloj a cero, asi que setup() corre con
// millis()==0 (la global de la DLL recien mapeada) y la primera vuelta del bucle salta de
// golpe a g_t. Con la ventana de D-29 medida sobre millis() -tiempo desde el arranque,
// que es lo que es en la tarjeta, donde el ESP32 y el STM32 encienden a la vez- ese salto
// cerraria la ventana en la primera vuelta y el bloque no podria ejercer nada.
//
// Se pone g_t a cero al PRINCIPIO del escenario, justo despues de escenarioLimpio(), que
// es el unico instante en que las DOS puntas acaban de recargarse y ninguna arrastra una
// marca de tiempo futura. Mover g_t con una punta viva le haria correr el reloj hacia
// atras y sus restas sin signo darian plazos enormes: eso si seria fabricar una averia.
//
// LO QUE ESTO NO MODELA, ESCRITO AL LADO (CLAUDE.md 7): el corte llega unos segundos
// DESPUES del cero, asi que la ventana que el escenario deja al equipo es la del
// firmware MENOS lo que el escenario gasto antes de cortar. Es un recorte, no un regalo:
// el equipo del banco tiene MENOS margen que el de la calzada, nunca mas.
static void relojDelBancoACero() {
  g_t = 0;
  // La racha de "el Esclavo publica verde y tiene rojo" se cuenta con restas de g_t: si
  // quedara abierta al saltar el reloj, la resta daria un plazo enorme y contaria un
  // largo que no ocurrio.
  g_escVerdeRojoEnCurso = false;
}

// D-29: LA HORA DEL ESP32, POR EL CABLE DE VERDAD. Se teclea en el puerto del telefono la
// MISMA linea que el puente pone por J17 -prefijo leido del despachador real- y la
// despacha el bluetooth.cpp REAL en el siguiente tick de esta punta.
static void sembrarEsclavoDesdeEsp32(int dia, int h, int m, int s) {
  char linea[64];
  std::snprintf(linea, sizeof(linea), "%s2026-01-%02d,%02d:%02d:%02d",
                PREFIJO_HORA_ESP32.c_str(), dia, h, m, s);
  ESCLAVO.orden(("bt:" + std::string(linea)).c_str());
  unTick();   // el despachador real atiende la linea en este tick
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
// N-162 (bloque G): LA VENTANA "UNA PUNTA EN VERDE Y LA OTRA SIN ROJO", S_FALLO INCLUIDO.
//
// Es la condicion de A9 SIN su excepcion. A9 perdona S_FALLO porque "ahi ya no hay quien
// gobierne"; pero S_FALLO es ambar intermitente CON LA PLUMA ARRIBA (SFTY-28, semaforo.cpp)
// y, mientras la otra punta tenga un verde fijo, SI hay quien gobierna: esta dando paso al
// mismo carril que esta punta acaba de abrir. El detector va aislado, como el de E4, para
// que su control negativo lo ejerza con valores sinteticos.
// ---------------------------------------------------------------------------
static bool hayVerdeFrenteASinRojo(bool verdeA, bool rojoB) { return verdeA && !rojoB; }

static const char* nombreLuz(int e) {
  if (e == S_ROJO_V) return "ROJO";
  if (e == S_VERDE_V) return "VERDE";
  if (e == S_AMARILLO_V) return "AMBAR";
  if (e == S_FALLO_V) return "S_FALLO";
  return "?";
}

// Lo que se ve en un instante, en una linea: luz de cada punta y si el coordinador del
// Maestro esta en C_FALLO (comunicacion_perdida, la funcion REAL de coordinador.cpp).
static std::string fotoG() {
  std::string s = "M=";
  s += nombreLuz(MAESTRO.estado());
  if (MAESTRO.orden("comunicacion_perdida") == 1) s += "(C_FALLO)";
  s += " E=";
  s += nombreLuz(ESCLAVO.estado());
  return s;
}

struct VentanaG {
  unsigned long instantes = 0;     // instantes dentro de la ventana
  unsigned long rachas = 0;        // cuantas veces se abrio
  unsigned long maxMs = 0;         // la racha mas larga, en ms (instantes x PASO_MS)
  unsigned long simultaneo = 0;    // de ellos, con verde en LAS DOS
  unsigned long frenteAFallo = 0;  // de ellos, con la punta de enfrente en S_FALLO
  bool enCurso = false;
  unsigned long largo = 0;
  std::string empezo, empezoMax, acaboMax;
};

struct CorridaG {
  VentanaG v;
  std::vector<std::string> traza;
  std::string ultimaFoto;
  unsigned long tCorte = 0;
  bool maestroFallo = false;       // el coordinador llego a C_FALLO en algun instante
  int esclavoAlFallo = -1;         // luz del Esclavo en el primer instante con C_FALLO
  unsigned long tFallo = 0;
  // N-162 (bloque G, el control de toda inversion -CLAUDE.md 9-): un Maestro que no se
  // abriera NUNCA pasaria la ventana igual de bien que el correcto. Se anota cuando se abre
  // -ambar o verde de la transicion; S_FALLO no cuenta- y el ultimo instante en que el
  // Esclavo estuvo en verde antes de eso: la resta es el todo-rojo que hubo de verdad.
  long tAperturaM = -1;
  long tUltVerdeE = -1;
  long tSueltaE = -1;              // primer instante con el Esclavo fuera de verde
  // G8: cuantas veces ENTRA el Maestro en S_FALLO. Una oscilacion rojo <-> ambar -la pluma
  // del poste subiendo y bajando- se ve aqui y no en la ventana: en rojo no hay ventana.
  unsigned long entradasFalloM = 0;
  bool mEnFallo = false;
  // N-163 (G11): y las del ESCLAVO, que es la punta cuyo ambar decide el criterio del
  // responsable -"ni uno mas en microcortes que se recuperan"-. La del Maestro ya estaba;
  // esta no, y sin ella el criterio no se podia medir.
  unsigned long entradasFalloE = 0;
  bool eEnFallo = false;
};

// El todo-rojo medido: del primer instante sin verde del Esclavo al primero con el Maestro
// abriendo. -1 si en la corrida no hubo las dos cosas.
static long todoRojoMsG(const CorridaG& c) {
  if (c.tAperturaM < 0 || c.tUltVerdeE < 0) return -1;
  return c.tAperturaM - c.tUltVerdeE - (long)PASO_MS;
}

// LO QUE SE COMPARA CON EL BORDE ES EL ACUMULADO, NO LA RACHA MAS LARGA. Medido con un
// defecto inyectado: un Maestro que conmuta S_FALLO <-> ROJO en cada instante deja la ventana
// abierta la mitad del tiempo con rachas de UN instante, y un borde sobre la racha lo daba
// por bueno. Un corte es un suceso: todo lo que abra en su escenario cuenta junto.
static unsigned long acumuladoMsG(const VentanaG& v) { return v.instantes * PASO_MS; }

static void cerrarRachaG(VentanaG& v, const std::string& porque) {
  const unsigned long dur = v.largo * PASO_MS;
  if (dur > v.maxMs) { v.maxMs = dur; v.empezoMax = v.empezo; v.acaboMax = porque; }
  v.enCurso = false;
  v.largo = 0;
}

// Un tick del arnes y, detras, la observacion de la ventana y de la historia de luces.
static void pasoG(CorridaG& c) {
  const unsigned long gr0 = g_goRojoEntregados;
  const unsigned long t = g_t;
  unTick();
  const bool goRojo = (g_goRojoEntregados != gr0);
  VentanaG& v = c.v;
  const bool vM = MAESTRO.verde(), vE = ESCLAVO.verde();
  const bool dentro = hayVerdeFrenteASinRojo(vM, ESCLAVO.rojo()) ||
                      hayVerdeFrenteASinRojo(vE, MAESTRO.rojo());
  const std::string foto = fotoG();
  if (dentro) {
    v.instantes++;
    if (vM && vE) v.simultaneo++;
    if ((vM && ESCLAVO.estado() == S_FALLO_V) || (vE && MAESTRO.estado() == S_FALLO_V))
      v.frenteAFallo++;
    if (!v.enCurso) { v.enCurso = true; v.largo = 0; v.rachas++; v.empezo = foto; }
    v.largo++;
  } else if (v.enCurso) {
    cerrarRachaG(v, foto + (goRojo ? " [GO_RED entregado al Esclavo en ese instante]" : ""));
  }
  if (!c.maestroFallo && MAESTRO.orden("comunicacion_perdida") == 1) {
    c.maestroFallo = true;
    c.esclavoAlFallo = ESCLAVO.estado();
    c.tFallo = t;
  }
  if (c.tAperturaM < 0) {
    if (vE) c.tUltVerdeE = (long)t;
    if (vM || MAESTRO.estado() == S_AMARILLO_V) c.tAperturaM = (long)t;
  }
  if (c.tSueltaE < 0 && !vE) c.tSueltaE = (long)t;
  const bool mFallo = (MAESTRO.estado() == S_FALLO_V);
  if (mFallo && !c.mEnFallo) c.entradasFalloM++;
  c.mEnFallo = mFallo;
  const bool eFallo = (ESCLAVO.estado() == S_FALLO_V);
  if (eFallo && !c.eEnFallo) c.entradasFalloE++;
  c.eEnFallo = eFallo;
  if (foto != c.ultimaFoto) {
    char pre[32];
    std::snprintf(pre, sizeof(pre), "t%+7ld ms  ", (long)t - (long)c.tCorte);
    c.traza.push_back(std::string(pre) + foto + (goRojo ? "   <- GO_RED entregado" : ""));
    c.ultimaFoto = foto;
  }
}

static void correrG(CorridaG& c, unsigned long ms) {
  for (unsigned long h = 0; h < ms; h += PASO_MS) pasoG(c);
}

static void finG(CorridaG& c) {
  if (c.v.enCurso) cerrarRachaG(c.v, "SEGUIA ABIERTA al acabar el escenario: " + fotoG());
}

// N-142: el telefono del Poste 2 pide el ambar de emergencia POR LA PUERTA SIN PIN, que
// es la que manda la app. La linea sale del C++ (ver literalPuertaAmbar); la despacha el
// bluetooth.cpp REAL del Esclavo en su siguiente tick.
static long tecleaAmbarApp() {
  return ESCLAVO.orden(("bt:" + LINEA_AMBAR_APP).c_str());
}

// Cual de los RESULT de esa rama trae el ultimo acuse que el equipo le mando al telefono.
// Se pregunta por CADA literal leido del C++ -ninguno escrito aqui-, de modo que el
// escenario puede comparar dos respuestas sin saber como se llaman.
static std::string resultDelUltimoAcuse() {
  std::string hallado;
  for (const std::string& r : RESULTS_AMBAR_APP) {
    if (ESCLAVO.orden(("ack:RESULT:" + r).c_str()) == 1) {
      // El mas largo gana: "OK" es prefijo de "OK_SIN_RADIO" y strstr casaria los dos.
      if (r.size() > hallado.size()) hallado = r;
    }
  }
  return hallado;
}

static bool alcanzarVerdeG(Punta& p, unsigned long presupuesto) {
  for (unsigned long g = 0; g < presupuesto; g += PASO_MS) {
    unTick();
    if (p.verde()) return true;
  }
  return false;
}

// Hasta el tick en que el Maestro pone un GO_GREEN en el aire (queda en vuelo: llega).
static bool alcanzarGoVerdeG(unsigned long presupuesto) {
  const unsigned long n0 = g_goVerdeEmitidos;
  for (unsigned long g = 0; g < presupuesto; g += PASO_MS) {
    unTick();
    if (g_goVerdeEmitidos != n0) return true;
  }
  return false;
}

static void imprimirTrazaG(const char* titulo, const CorridaG& c) {
  std::printf("      %s\n", titulo);
  const size_t n = c.traza.size();
  const size_t MAXL = 16;
  for (size_t i = 0; i < n && i < MAXL; i++) std::printf("        %s\n", c.traza[i].c_str());
  if (n > MAXL) std::printf("        ... (%lu cambios mas)\n", (unsigned long)(n - MAXL));
  std::printf("        ventana: %lu ms acumulados (%lu instantes) en %lu racha(s), la mas larga "
              "%lu ms (%lu instantes con verde en las dos, %lu frente a S_FALLO)\n",
              acumuladoMsG(c.v), c.v.instantes, c.v.rachas, c.v.maxMs, c.v.simultaneo,
              c.v.frenteAFallo);
  if (c.v.maxMs > 0) {
    std::printf("        la mas larga empezo en [%s] y la cerro [%s]\n",
                c.v.empezoMax.c_str(), c.v.acaboMax.c_str());
  }
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
  // N-137 (04/09): los seis limites del ciclo se mudaron a include/limites_ciclo.h.
  // Vivian `static` dentro del .cpp -invisibles para los demas modos- y eso produjo
  // tres agujeros el mismo dia. Este arnes ABORTO al cambiarlo, que es §5 funcionando.
  const std::string LIMITES = RAIZ + "/Maestro/include/limites_ciclo.h";

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

    // N-162 (bloque F): el filtro del canal reconoce GO_GREEN y ACK_GREEN por su codigo.
    // El GO_GREEN lo emite el Maestro y el ACK_GREEN el Esclavo, y cada uno lo escribe con
    // SU protocolo.h: si los dos ficheros discreparan, el filtro tiraria una trama que no
    // es la que el escenario nombra, y el bloque mediria otra perdida sin enterarse.
    const std::string PROTO_E = RAIZ + "/Esclavo/include/protocolo.h";
    CMD_GO_GREEN_V  = hex(PROTO_M, "CMD_GO_GREEN");
    CMD_ACK_GREEN_V = hex(PROTO_E, "CMD_ACK_GREEN");
    if (hex(PROTO_E, "CMD_GO_GREEN") != CMD_GO_GREEN_V ||
        hex(PROTO_M, "CMD_ACK_GREEN") != CMD_ACK_GREEN_V) {
      abortar("CMD_GO_GREEN o CMD_ACK_GREEN DIFIEREN entre Maestro/include/protocolo.h y "
              "Esclavo/include/protocolo.h: el bloque F no sabria que trama esta tirando");
    }
    if (CMD_GO_GREEN_V == CMD_ACK_GREEN_V) abortar("CMD_GO_GREEN y CMD_ACK_GREEN comparten codigo");

    // N-162 (bloque G): el GO_RED lo emite el Maestro y lo obedece el Esclavo; mismo trato.
    CMD_GO_RED_V = hex(PROTO_M, "CMD_GO_RED");
    if (hex(PROTO_E, "CMD_GO_RED") != CMD_GO_RED_V)
      abortar("CMD_GO_RED DIFIERE entre las dos protocolo.h: el bloque G no sabria que trama cuenta");
    if (CMD_GO_RED_V == CMD_GO_GREEN_V || CMD_GO_RED_V == CMD_ACK_GREEN_V)
      abortar("CMD_GO_RED comparte codigo con GO_GREEN o ACK_GREEN");
    // N-163 (G11): el PING, que es la otra trama que refresca tUltimoComando en el Esclavo.
    // Sin leerlo, el reloj de silencio del Esclavo contaria un PING como servicio y G11
    // derivaria mas ambares de los que tocan.
    CMD_PING_V = hex(PROTO_M, "CMD_PING");
    if (hex(PROTO_E, "CMD_PING") != CMD_PING_V)
      abortar("CMD_PING DIFIERE entre las dos protocolo.h: G11 no sabria fechar el silencio");
    if (CMD_PING_V == CMD_GO_RED_V || CMD_PING_V == CMD_GO_GREEN_V)
      abortar("CMD_PING comparte codigo con una orden de luz");
    // Y el ACK_RED, que el bloque G retiene en el aire a proposito (G9).
    CMD_ACK_RED_V = hex(PROTO_E, "CMD_ACK_RED");
    if (hex(PROTO_M, "CMD_ACK_RED") != CMD_ACK_RED_V)
      abortar("CMD_ACK_RED DIFIERE entre las dos protocolo.h: el bloque G no sabria que acuse retiene");
    if (CMD_ACK_RED_V == CMD_GO_RED_V || CMD_ACK_RED_V == CMD_ACK_GREEN_V ||
        CMD_ACK_RED_V == CMD_GO_GREEN_V)
      abortar("CMD_ACK_RED comparte codigo con otra orden de luz");
    // D-34: el PONG y el valor de su param que dice "verde soltado". Los emite el Esclavo
    // y los lee el Maestro, cada uno con SU protocolo.h.
    CMD_PONG_V = hex(PROTO_E, "CMD_PONG");
    PONG_VERDE_SOLTADO_V = hex(PROTO_E, "PONG_VERDE_SOLTADO");
    if (hex(PROTO_M, "CMD_PONG") != CMD_PONG_V ||
        hex(PROTO_M, "PONG_VERDE_SOLTADO") != PONG_VERDE_SOLTADO_V)
      abortar("CMD_PONG o PONG_VERDE_SOLTADO DIFIEREN entre las dos protocolo.h: G13 no sabria "
              "que aviso cuenta");
    if (PONG_VERDE_SOLTADO_V == 0)
      abortar("PONG_VERDE_SOLTADO vale 0, que es el param del PONG de siempre: el aviso no "
              "se distinguiria de un PONG normal");
  }

  // N-162 (bloque F): los valores del enum EstadoSemaforo, releidos de las DOS cabeceras.
  // punta_estado() devuelve ese enum como int, y el orquestador no incluye cabeceras del
  // firmware; un numero escrito aqui a mano dejaria de significar AMARILLO el dia que
  // alguien reordene el enum, y el bloque contaria otra luz sin fallar.
  {
    auto indiceEnum = [&](const std::string& ruta, const char* nombre) -> int {
      std::string txt = leerFuente(ruta);
      std::smatch m;
      if (!std::regex_search(txt, m, std::regex(R"(enum\s+EstadoSemaforo\s*\{([^}]*)\})")))
        abortar("no se encuentra enum EstadoSemaforo en " + ruta);
      std::string cuerpo = m[1].str();
      std::stringstream ss(cuerpo);
      std::string item;
      int i = 0;
      while (std::getline(ss, item, ',')) {
        if (item.find('=') != std::string::npos)
          abortar("enum EstadoSemaforo lleva valores explicitos en " + ruta +
                  ": este lector cuenta posiciones y ya no sabria el valor");
        std::smatch n;
        if (std::regex_search(item, n, std::regex(R"((\w+))")) && n[1].str() == nombre) return i;
        i++;
      }
      abortar(std::string("no se encuentra ") + nombre + " en enum EstadoSemaforo de " + ruta);
      return -1;
    };
    const std::string SEM_M = RAIZ + "/Maestro/include/semaforo.h";
    const std::string SEM_E = RAIZ + "/Esclavo/include/semaforo.h";
    S_VERDE_V    = indiceEnum(SEM_E, "S_VERDE");
    S_AMARILLO_V = indiceEnum(SEM_E, "S_AMARILLO");
    if (indiceEnum(SEM_M, "S_VERDE") != S_VERDE_V || indiceEnum(SEM_M, "S_AMARILLO") != S_AMARILLO_V)
      abortar("enum EstadoSemaforo DIFIERE entre las dos puntas");
    // N-162 (bloque G): los otros dos valores, con el mismo cruce entre puntas.
    S_ROJO_V  = indiceEnum(SEM_E, "S_ROJO");
    S_FALLO_V = indiceEnum(SEM_E, "S_FALLO");
    if (indiceEnum(SEM_M, "S_ROJO") != S_ROJO_V || indiceEnum(SEM_M, "S_FALLO") != S_FALLO_V)
      abortar("enum EstadoSemaforo DIFIERE entre las dos puntas (S_ROJO o S_FALLO)");
    // vigilar() -A9 y SFTY-28- lleva "const int S_FALLO = 3" escrito a mano desde antes
    // del bloque G. No se toca aqui, pero si el enum se reordena ese 3 dejaria de ser
    // S_FALLO sin fallar nada: se exige que siga siendolo.
    if (S_FALLO_V != 3)
      abortar("S_FALLO ya no vale 3 en enum EstadoSemaforo y vigilar() lo lleva escrito a mano");
  }
  SFTY6_SILENCIO_MS_V  = leerNumero(PROTO_M, R"(#define\s+SFTY6_SILENCIO_MS\s+(\d+)UL)", "SFTY6_SILENCIO_MS");
  TIMEOUT_ACK_MS_V     = leerNumero(COORD, R"(TIMEOUT_ACK_MS\s*=\s*(\d+))", "TIMEOUT_ACK_MS");
  CICLO_MAX_REINTENTOS_V = leerNumero(COORD, R"(CICLO_MAX_REINTENTOS\s*=\s*(\d+))", "CICLO_MAX_REINTENTOS");
  // N-131 (04/09): el despeje de arranque ya no es un literal en el inicializador.
  // Sale de DESPEJE_SEG_MIN, la misma constante que la guarda de SET_TIEMPOS, para que
  // no puedan divergir. Este arnes ABORTO al cambiarlo -leia el patron viejo- y eso es
  // lo correcto: §5, mover contenido rompe al que lee por patron, y un ABORTADO avisa
  // mientras que un numero supuesto no.
  DESPEJE_POR_DEFECTO_S  = leerNumero(LIMITES, R"(DESPEJE_SEG_MIN\s*=\s*(\d+))", "despeje por defecto");

  // D-33: el retardo de bajada de la pluma, leido de LAS DOS puntas. Se comparan aqui
  // porque escribirPines() tiene que ser identico en las dos (barrera_03) y este numero
  // es parte de esa orden: si se separan, una punta bajaria la barrera antes que la otra
  // con el mismo rojo. SIN VALOR POR DEFECTO: si el patron desaparece, aborta.
  {
    const unsigned long rM = leerNumero(RAIZ + "/Maestro/src/semaforo.cpp",
        R"(PLUMA_RETARDO_BAJADA_MS\s*=\s*(\d+)UL)", "retardo de bajada de la pluma (Maestro)");
    const unsigned long rE = leerNumero(RAIZ + "/Esclavo/src/semaforo.cpp",
        R"(PLUMA_RETARDO_BAJADA_MS\s*=\s*(\d+)UL)", "retardo de bajada de la pluma (Esclavo)");
    if (rM != rE) {
      abortar("D-33: el retardo de bajada de la pluma NO es el mismo en las dos puntas");
    }
    g_retardoPlumaMs = rM;
  }
  // N-162 (bloque F): el ambar de transicion rojo->verde del ESCLAVO. Es un literal dentro
  // de la condicion de semaforo_actualizar(), sin nombre; se lee de esa misma condicion,
  // que es la que decide cuando la luz pasa a verde.
  AMBAR_ESCLAVO_MS_V = leerNumero(RAIZ + "/Esclavo/src/semaforo.cpp",
      R"(estado\s*==\s*S_AMARILLO\s*&&\s*\(ahora\s*-\s*tCambio\s*>=\s*(\d+)\))",
      "el ambar de transicion del Esclavo");

  LATIDO_MS_V = leerNumero(COORD, R"(const\s+unsigned\s+long\s+LATIDO_MS\s*=\s*(\d+))", "LATIDO_MS");
  RETARDO_RESPUESTA_MS_V = leerNumero(RAIZ + "/Esclavo/src/main.cpp",
      R"(RETARDO_RESPUESTA_MS\s*=\s*(\d+))", "RETARDO_RESPUESTA_MS del Esclavo");
  MAX_VERDE_BACKSTOP_MS_V = leerNumero(RAIZ + "/Esclavo/src/main.cpp",
      R"(MAX_VERDE_BACKSTOP_MS\s*=\s*(\d+))", "MAX_VERDE_BACKSTOP_MS del Esclavo");
  VERDE_MIN_MIN_V = leerNumero(LIMITES, R"(VERDE_MIN_MIN\s*=\s*(\d+))", "VERDE_MIN_MIN");
  ROJO_MIN_MIN_V  = leerNumero(LIMITES, R"(ROJO_MIN_MIN\s*=\s*(\d+))", "ROJO_MIN_MIN");

  // N-162 (bloque D): el limite duro sin sincronizacion, la SEGUNDA puerta de
  // degradado_reanudarTrasCorte(). Se relee del C++ y en HORAS derivadas de la misma
  // expresion que escribe el firmware -"48UL * 3600UL * 1000UL"-, no de un 48 escrito
  // aqui: si manana son 24, el bloque compara contra 24 sin que nadie se acuerde.
  LIMITE_SIN_SYNC_H_V = leerNumero(RAIZ + "/Esclavo/src/modo_degradado.cpp",
      R"(LIMITE_SIN_SYNC_MS\s*=\s*(\d+)UL\s*\*\s*3600UL\s*\*\s*1000UL)",
      "LIMITE_SIN_SYNC_MS del Modo Degradado del Esclavo");

  // D-29 (bloque D): LA VENTANA DEL DIFERIMIENTO, DERIVADA COMO LA DERIVA EL FIRMWARE.
  //
  // modo_degradado.cpp no escribe un plazo propio: dice
  // "VENTANA_REANUDACION_MS = HORA_ESP32_ESPERA_MAX_MS", y ese simbolo vale
  // 3 x HORA_ESP32_CADENCIA_MS en reloj.h. Se leen las TRES cosas: que la ventana siga
  // colgando de ese simbolo -si alguien la cambia por un numero suelto esto ABORTA, que es
  // lo correcto (§5)- y los dos factores de los que sale el valor.
  {
    const std::string DEG_E = RAIZ + "/Esclavo/src/modo_degradado.cpp";
    const std::string RELOJ_E = RAIZ + "/Esclavo/include/reloj.h";
    if (!std::regex_search(leerFuente(DEG_E),
            std::regex(R"(VENTANA_REANUDACION_MS\s*=\s*HORA_ESP32_ESPERA_MAX_MS)"))) {
      abortar("la ventana de D-29 de Esclavo/src/modo_degradado.cpp ya no se deriva de "
              "HORA_ESP32_ESPERA_MAX_MS. Este bloque estaria midiendo contra un plazo que "
              "el firmware ya no usa");
    }
    const unsigned long cadencia = leerNumero(RELOJ_E,
        R"(HORA_ESP32_CADENCIA_MS\s*=\s*(\d+)UL)", "HORA_ESP32_CADENCIA_MS del Esclavo");
    const unsigned long veces = leerNumero(RELOJ_E,
        R"(HORA_ESP32_ESPERA_MAX_MS\s*=\s*(\d+)UL\s*\*\s*HORA_ESP32_CADENCIA_MS)",
        "el multiplicador de HORA_ESP32_ESPERA_MAX_MS del Esclavo");
    VENTANA_REANUDACION_MS_V = cadencia * veces;
    if (VENTANA_REANUDACION_MS_V == 0) abortar("la ventana de D-29 salio en cero");
  }

  // D-29 (bloque D): EL PREFIJO DE LA LINEA DEL ESP32, LEIDO DEL DESPACHADOR REAL.
  //
  // Se localiza por donde SIEMBRA -"reloj_sembrarDesdeIso(cmd + N)"-, se toma esa N y se
  // busca el strncmp que compara EXACTAMENTE esos N caracteres. Asi el arnes no lleva
  // escrito el nombre del comando ni su longitud: si se renombra, esto sigue tecleando la
  // linea buena; si el despachador deja de sembrar por ahi, ABORTA en vez de medir otra
  // cosa (§5 y §7 sobre lo que un literal copiado mide al dia siguiente).
  {
    const std::string bt = leerFuente(RAIZ + "/Esclavo/src/bluetooth.cpp");
    std::smatch m;
    if (!std::regex_search(bt, m, std::regex(R"(reloj_sembrarDesdeIso\(cmd\s*\+\s*(\d+)\))"))) {
      abortar("no se encuentra en Esclavo/src/bluetooth.cpp la siembra "
              "reloj_sembrarDesdeIso(cmd + N): el bloque D no sabria por que linea entra "
              "la hora del ESP32 y teclearia un comando inventado");
    }
    const std::string n = m[1].str();
    std::smatch mp;
    if (!std::regex_search(bt, mp,
            std::regex(R"RX(strncmp\(cmd,\s*"([^"]+)",\s*)RX" + n + R"RX(\)\s*==\s*0)RX"))) {
      abortar("no se encuentra el strncmp de " + n + " caracteres que guarda la rama de "
              "la hora del ESP32 en Esclavo/src/bluetooth.cpp");
    }
    PREFIJO_HORA_ESP32 = mp[1].str();
    if (PREFIJO_HORA_ESP32.size() != (size_t)std::atoi(n.c_str())) {
      abortar("el literal '" + PREFIJO_HORA_ESP32 + "' de la rama de la hora del ESP32 no "
              "mide los " + n + " caracteres que el despachador compara: el arnes no puede "
              "componer la linea sin adivinar");
    }
  }

  // N-142 (bloque H): la linea del telefono y los acuses que esa rama puede contestar,
  // leidos del despachador real. Sin ellos el bloque H tecleeria un comando inventado y
  // el Esclavo lo rechazaria con un $ERR generico: el escenario pasaria por "el Maestro
  // no se entera" cuando lo que no entero fue el arnes.
  LINEA_AMBAR_APP = literalPuertaAmbar(
      leerFuente(RAIZ + "/Esclavo/src/bluetooth.cpp"), &RESULTS_AMBAR_APP);
  if (LINEA_AMBAR_APP.empty() || RESULTS_AMBAR_APP.empty()) {
    abortar("no se pudo leer del C++ la puerta SIN PIN del ambar de emergencia del "
            "Esclavo -la rama comparada contra 'cmd' que llama a semaforo_iniciarFallo()- "
            "o sus RESULT. El bloque H teclearia un comando que no existe");
  }

  // D-31: los tres campos de la alarma del aviso, del mismo fuente y por el mismo motivo.
  ALARMA_AVISO = alarmaDelAviso(leerFuente(RAIZ + "/Esclavo/src/bluetooth.cpp"));
  if (!ALARMA_AVISO.valida()) {
    abortar("no se pudo leer del C++ la alarma del aviso de ambar (D-31): se busca la "
            "bluetooth_reportarAlarma() que sigue a AVISO_AMBAR_REINTENTOS en "
            "Esclavo/src/bluetooth.cpp. Sin ella H4 exigiria una alarma escrita a mano, "
            "que es una prueba que aprueba el firmware de ayer");
  }
  PREFIJO_PIN_APP = prefijoPinApp(leerFuente(RAIZ + "/Esclavo/src/bluetooth.cpp"));
  if (PREFIJO_PIN_APP.empty()) {
    abortar("no se pudo leer del C++ el prefijo de PIN del despachador del Esclavo "
            "(strncmp contra 'cmd' que empieza por CMD:PIN:). El bloque H6 teclearia una "
            "cancelacion que el equipo rechaza y aprobaria sin haber cancelado nada");
  }
  AVISO_AMBAR_TIMEOUT_MS_V = leerNumero(PROTO_M,
      R"(#define\s+AVISO_AMBAR_TIMEOUT_MS\s+(\d+)UL)", "AVISO_AMBAR_TIMEOUT_MS");
  AVISO_AMBAR_REINTENTOS_V = leerNumero(PROTO_M,
      R"(#define\s+AVISO_AMBAR_REINTENTOS\s+(\d+))", "AVISO_AMBAR_REINTENTOS");

  std::printf("\n Constantes releidas del C++ real: silencio SFTY-6 = %lu ms,\n",
              SFTY6_SILENCIO_MS_V);
  std::printf(" timeout de ACK = %lu ms x %lu reintentos, despeje por defecto = %lu s.\n",
              TIMEOUT_ACK_MS_V, CICLO_MAX_REINTENTOS_V, DESPEJE_POR_DEFECTO_S);
  std::printf(" ambar de transicion del Esclavo = %lu ms.\n", AMBAR_ESCLAVO_MS_V);

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
  // N-162. A10 es el control positivo de A11 y A12: sin instantes con ESC:VERDE las dos
  // de abajo pasarian igual con un campo que dijera ROJO siempre.
  comprobar(g_escVerdeTicks > 0,
            "A10 (N-162): el Maestro SI llego a publicar ESC:VERDE (" +
            std::to_string(g_escVerdeTicks) + " instantes): sin esto A11 y A12 no "
            "medirian nada");
  comprobar(g_escVerdeConMaestroAbierto == 0,
            "A11 (N-162, cinta del Sisga 12:21:08): el Maestro NUNCA publico ESC:VERDE "
            "estando el mismo en ambar o en verde. Instantes: " +
            std::to_string(g_escVerdeConMaestroAbierto));
  comprobar(g_escVerdeRojoLargo == 0,
            "A12 (N-162, cinta del Sisga 12:20:54): ESC:VERDE no siguio publicandose mas "
            "de 1 s con el Esclavo REAL en rojo -el despeje entero lo tapaba-. "
            "Instantes fuera de tolerancia: " + std::to_string(g_escVerdeRojoLargo));

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
  //
  // 🔴 12/09 (N-162, roadmap 1.16(c)) - QUE CAMBIO AQUI Y POR QUE. La parte del microcorte
  // media la reanudacion contra un modelo de RTC que guardaba la hora al ponerla, y el
  // firmware dejo de escribir ese RTC el 11/09: el escenario reanudaba y la tarjeta ya no
  // puede. Ninguna linea se reescribio en bloque hasta que pasara (CLAUDE.md §9): D1 a D4
  // y D7 se conservan tal cual porque no dependen del corte, D5 se reparte -se queda con
  // la precondicion y suelta la frase que afirmaba que la reanudacion se estaba
  // ejerciendo-, D6 se conserva porque su propiedad de seguridad sigue valiendo, y lo que
  // se anade son el desenlace (D6b), el ORDEN de las dos puertas (D6c), lo que la pila
  // pierde (D6d) y el escenario de control que las tres necesitan (D8/D9).
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
    relojDelBancoACero();   // D-29: el porque, y lo que recorta, en esa funcion
    unsigned long sim0 = g_verdeSimultaneo;
    sincronizarEsclavo(10, 8, 0, 0);
    configurarEsclavo(20, 10);
    g_enlaceHaciaEsclavo = false;
    g_enlaceHaciaMaestro = false;
    ESCLAVO.orden("degradado_entrar");
    avanzar(30000);
    bool gobernabaAntes = ESCLAVO.orden("degradado_gobierna") == 1;
    const bool respaldoAntes = ESCLAVO.orden("respaldo_degradado") == 1;
    const long cntAntes = ESCLAVO.domLeer(10);

    microcorte(ESCLAVO);

    // SE LEE ANTES DE AVANZAR NI UN TICK. degradado_reanudarTrasCorte() ya corrio dentro
    // de setup() y no se vuelve a llamar nunca -"Solo la llama setup(), y una sola vez",
    // modo_degradado.cpp-, asi que lo que decidio esta puesto en esta primera vuelta.
    const bool gobiernaTrasCorte = ESCLAVO.orden("degradado_gobierna") == 1;
    const bool huboSyncTrasCorte = ESCLAVO.orden("degradado_hubo_sync") == 1;
    const bool enHoraTrasCorte   = ESCLAVO.orden("reloj_en_hora") == 1;
    const bool rtcHwTrasCorte    = ESCLAVO.orden("rtc_hw_en_hora") == 1;
    const long horasSyncTrasCorte = ESCLAVO.orden("respaldo_horas_sync");
    const long cntTrasCorte       = ESCLAVO.domLeer(10);
    const bool respaldoTrasCorte  = ESCLAVO.orden("respaldo_degradado") == 1;

    const unsigned long vE0 = g_ticksVerdeEsclavo;
    avanzar(120000);
    const unsigned long verdesTrasCorte = g_ticksVerdeEsclavo - vE0;
    comprobar(gobernabaAntes && respaldoAntes,
              "D5 (control): el Esclavo estaba gobernando por reloj cuando se le corto la "
              "energia Y la pila lo tenia anotado, que son las dos precondiciones de "
              "degradado_reanudarTrasCorte(). Sin ellas nada de lo que sigue mide nada");
    comprobar(g_verdeSimultaneo == sim0,
              "D6: tras el microcorte, con el Esclavo decidiendo por su cuenta si reanuda "
              "el Degradado y el Maestro sin enterarse, NUNCA coincidio un verde. QUE "
              "decidio lo dice D6b: hasta el 12/09 esta linea llevaba escrito 'reanudando "
              "-o no-', y ese '-o no-' era el hueco por el que el bloque se quedo midiendo "
              "un mecanismo que el firmware ya no tiene");

    // 🔴 N-162, roadmap 1.16(c) - LO QUE D5/D6 NO MEDIAN, Y POR QUE HACIA FALTA.
    //
    // Hasta el 12/09 este escenario reanudaba: el modelo de RTC del adaptador guardaba la
    // hora al ponerla y el dominio se la devolvia al arrancar. El firmware perdio esa
    // capacidad el 11/09 -reloj_ajustarConAcuse() ya no llama a rtc.setHours/Minutes/
    // Seconds, o sea que rtc.getYear() no llega nunca a ANIO_MARCA y reloj_setup() deja
    // horaValida en false-, asi que el arnes estaba ejerciendo un camino que la tarjeta ya
    // no tiene. Un instrumento que mide contra un mecanismo muerto no mide poco: mide otra
    // cosa, y da verde. D6 sobrevive porque la propiedad de seguridad que vigila sigue
    // valiendo; lo que se anade es el DESENLACE, que D6 tapaba con su "-o no-".
    comprobar(!gobiernaTrasCorte && !huboSyncTrasCorte && verdesTrasCorte == 0,
              "D6b: tras el corte el Esclavo NO reanuda el Degradado -no gobierna la luz, "
              "degradado_huboSync() sigue en false y en los 2 min siguientes dio " +
              std::to_string(verdesTrasCorte) + " verdes por reloj-, porque la hora vivia "
              "en RAM y el corte se la llevo. 🔴 12/09, D-29: ESTA LINEA SOBREVIVE Y CAMBIA "
              "DE PRECONDICION. Mide la ventana en la que el ESP32 TODAVIA NO HA HABLADO, "
              "que con el diferimiento es justo donde hay que mirar: el permiso de la pila "
              "sigue puesto (D6d) y aun asi no se enciende nada. Un diferimiento que "
              "reanudara sin hora seria la entrada automatica que SFTY-21 prohibe");

    // 🔴 Y EL ORDEN, no solo el desenlace (CLAUDE.md §9). degradado_reanudarTrasCorte()
    // tiene DOS puertas en serie -"reloj_enHora() && respaldo_hayCiclo()" primero, y las
    // horas desde la ultima sincronizacion despues-. Una inversion que solo mirase D6b la
    // aprobaria igual con las dos cerradas, y no distinguiria "no reanuda porque no sabe
    // que hora es" de "no reanuda porque el contador de la pila se perdio", que son
    // averias distintas y una de ellas seria del arnes. Aqui se exige que la SEGUNDA este
    // ABIERTA: el contador sobrevivio, avanzo, y la marca de sync es fresca.
    comprobar(!enHoraTrasCorte && !rtcHwTrasCorte &&
                  cntTrasCorte >= cntAntes && horasSyncTrasCorte >= 0 &&
                  (unsigned long)horasSyncTrasCorte < LIMITE_SIN_SYNC_H_V,
              "D6c (el orden): la puerta que cerro fue la PRIMERA. El contador de la pila "
              "sobrevivio y avanzo (" + std::to_string(cntAntes) + " -> " +
              std::to_string(cntTrasCorte) + " s) y la marca de sync sale de hace " +
              std::to_string(horasSyncTrasCorte) + " h contra un limite de " +
              std::to_string(LIMITE_SIN_SYNC_H_V) + " h, o sea que la segunda puerta "
              "estaba ABIERTA; lo que falta es la hora, y el RTC hardware no la trae "
              "porque desde N-162 ninguna linea del firmware lo escribe");

    // 🔴 D-29 (12/09) — LA LINEA INVERTIDA. Hasta hoy exigia !respaldoTrasCorte: el
    // indicador se borraba en ese mismo arranque y esa era "la perdida definitiva", con la
    // hora del ESP32 llegando despues y sin nada que reanudar. Es exactamente el defecto
    // que D-29 manda reconstruir, asi que la linea se INVIERTE -no se borra: la propiedad
    // que vigila -que pasa con el permiso de la pila en el arranque- sigue siendo la
    // pregunta buena, solo que la respuesta correcta es la contraria.
    comprobar(respaldoTrasCorte,
              "D6d (invertida por D-29): el indicador de la pila NO se borra en ese "
              "arranque. Se conserva mientras la primera siembra del ESP32 pueda llegar, "
              "porque es la unica forma de que la reanudacion se decida con la hora que "
              "esa siembra trae. Y se conserva SIN reanudar nada todavia (D6b): lo que se "
              "difiere es el borrado, no la puerta");

    // 🔴 D-29 — Y EL BORDE DE ESE DIFERIMIENTO, MEDIDO (CLAUDE.md §7: cuando un
    // instrumento compara contra un borde, se escribe CUAL es y por que es el correcto).
    //
    // CUAL: VENTANA_REANUDACION_MS de modo_degradado.cpp, que no es un plazo propio sino
    // HORA_ESP32_ESPERA_MAX_MS -tres cadencias de siembra, releidas de reloj.h aqui
    // arriba-. POR QUE ESE: es el mismo instante en el que bluetooth.cpp da por muda la
    // siembra del ESP32 y publica su $ALARM, o sea que el permiso dura exactamente lo que
    // el propio firmware considera que la siembra puede tardar, ni una vuelta mas.
    //
    // Sin esta linea el diferimiento seria indefinido, y un permiso que no caduca acaba
    // siendo la entrada automatica que el comentario del Maestro lleva avisando desde
    // N-20: "el dia que el operario ponga el reloj en hora por otro motivo".
    const unsigned long vE1 = g_ticksVerdeEsclavo;
    avanzar(VENTANA_REANUDACION_MS_V);
    const bool respaldoTrasVentana = ESCLAVO.orden("respaldo_degradado") == 1;
    // Y una siembra que llega TARDE ya no resucita nada: la ventana no es una pausa, es
    // un plazo. Sin esta segunda mitad la linea solo diria "el bit se apago".
    sembrarEsclavoDesdeEsp32(10, 8, 45, 0);
    avanzar(120000);
    const bool gobiernaTrasVentana = ESCLAVO.orden("degradado_gobierna") == 1;
    const bool enHoraTrasVentana = ESCLAVO.orden("reloj_en_hora") == 1;
    const unsigned long verdesTrasVentana = g_ticksVerdeEsclavo - vE1;
    comprobar(!respaldoTrasVentana && enHoraTrasVentana && !gobiernaTrasVentana &&
                  verdesTrasVentana == 0,
              "D6e (el borde de D-29): pasados los " +
              std::to_string(VENTANA_REANUDACION_MS_V / 1000UL) + " s de la ventana sin "
              "una sola siembra, el permiso de la pila se TIRA, y una siembra posterior "
              "-que si pone el equipo en hora- ya no reanuda nada ni enciende un verde. El "
              "diferimiento caduca solo; no hace falta que nadie se acuerde");
  }

  {
    // La guarda de N-83 del despachador real: con el ambar de Bluetooth pedido, un
    // CMD_GO_GREEN del Maestro NO enciende verde en el Esclavo. Es una de las dos
    // unicas ramas que pueden vetar una orden de verde, y vive en src/main.cpp.
    //
    // 11/09 (N-142): el cerrojo ya no se mueve a mano -aquella orden era un doble del
    // firmware- sino TECLEANDO la linea de la app en el bluetooth.cpp REAL. Y el aviso al
    // Maestro se pierde a proposito, en el tick en que sale: con el aviso, el Maestro se
    // va a MODO_AMBAR y deja de mandar GO_GREEN, o sea que la guarda que esta linea viene
    // a medir no se recorreria ni una vez. Lo que el aviso SI cambia se mide en el bloque H.
    escenarioLimpio(tiempos(1, 1, 15));
    unsigned long verdeE0 = g_ticksVerdeEsclavo;
    g_enlaceHaciaMaestro = false;
    bool d7 = tecleaAmbarApp() == 1;
    unTick();                       // el despachador real atiende la linea en este tick
    g_enlaceHaciaMaestro = true;
    d7 = d7 && ESCLAVO.orden("ambar_latch") == 1 && ESCLAVO.estado() == S_FALLO_V;
    avanzar(300000);
    comprobar(d7 && g_ticksVerdeEsclavo == verdeE0,
              "D7: con el ambar de emergencia pedido por la app -la linea " +
              LINEA_AMBAR_APP + " tecleada en el bluetooth.cpp REAL-, el despachador del "
              "Esclavo no encendio verde ni una sola vez en 5 minutos de ordenes del "
              "Maestro (la guarda de N-83, ejercida sobre el .cpp y no sobre su texto)");
  }

  {
    // 🔴 D-29 (12/09) — LO QUE SE CONSTRUYE, EJERCIDO DE PUNTA A PUNTA.
    //
    // Mismo escenario que D5/D6b: el Esclavo se corta en Degradado y despierta sin hora.
    // Lo unico que se anade es lo que pasa DE VERDAD en la tarjeta unos segundos despues:
    // el ESP32 pone su linea por J17. Se teclea la MISMA linea, con el prefijo leido del
    // despachador, y la atiende el bluetooth.cpp REAL de esta DLL.
    //
    // 🔴 Y LO QUE ESTE ESCENARIO MIDE NO ES EL DESENLACE, ES EL ORDEN (CLAUDE.md §9). Una
    // inversion que solo mirase "acaba reanudando" no distinguiria "reanuda PORQUE la
    // siembra llego a tiempo" de "reanuda porque alguien dejo la puerta abierta": las dos
    // acaban en verde por reloj. Por eso se lee el estado JUSTO ANTES de la siembra -60 s
    // de arranque, que es SIEMBRA_REINTENTO_2_MS del calendario del ESP32, o sea el rato
    // que de verdad puede pasar- y JUSTO DESPUES, en el mismo tick.
    escenarioLimpio(tiempos(1, 1, 15));
    relojDelBancoACero();
    {
      unsigned long simD = g_verdeSimultaneo;
      sincronizarEsclavo(10, 8, 0, 0);
      configurarEsclavo(20, 10);
      g_enlaceHaciaEsclavo = false;
      g_enlaceHaciaMaestro = false;
      ESCLAVO.orden("degradado_entrar");
      avanzar(30000);
      const bool gobernabaAntes = ESCLAVO.orden("degradado_gobierna") == 1;

      microcorte(ESCLAVO);

      const bool respaldoTrasCorte = ESCLAVO.orden("respaldo_degradado") == 1;
      const bool gobiernaTrasCorte = ESCLAVO.orden("degradado_gobierna") == 1;

      const unsigned long vAntes = g_ticksVerdeEsclavo;
      avanzar(60000);
      const bool enHoraAntes = ESCLAVO.orden("reloj_en_hora") == 1;
      const bool gobiernaAntes = ESCLAVO.orden("degradado_gobierna") == 1;
      const unsigned long verdesAntes = g_ticksVerdeEsclavo - vAntes;
      const long horasSyncAntes = ESCLAVO.orden("respaldo_horas_sync");
      const int luzAntes = ESCLAVO.estado();

      sembrarEsclavoDesdeEsp32(10, 8, 31, 0);   // la linea del ESP32, en un solo tick

      const bool enHoraDespues = ESCLAVO.orden("reloj_en_hora") == 1;
      const bool rtcHwDespues = ESCLAVO.orden("rtc_hw_en_hora") == 1;
      const bool gobiernaDespues = ESCLAVO.orden("degradado_gobierna") == 1;
      const bool huboSyncDespues = ESCLAVO.orden("degradado_hubo_sync") == 1;
      const bool respaldoDespues = ESCLAVO.orden("respaldo_degradado") == 1;
      const int luzDespues = ESCLAVO.estado();

      const unsigned long vE0 = g_ticksVerdeEsclavo;
      avanzar(120000);
      const unsigned long verdesDespues = g_ticksVerdeEsclavo - vE0;

      comprobar(gobernabaAntes && respaldoTrasCorte && !gobiernaTrasCorte,
                "D10 (control): el Esclavo gobernaba por reloj al cortarse la luz, "
                "despierta con el permiso de la pila TODAVIA PUESTO y sin haber reanudado "
                "nada. Son las tres precondiciones de D-29; sin ellas lo que sigue no mide "
                "el diferimiento, mide otra cosa");

      comprobar(!enHoraAntes && !gobiernaAntes && verdesAntes == 0 && luzAntes != S_VERDE_V,
                "D11 (el orden, primera mitad): durante los 60 s que el ESP32 tarda en "
                "poder hablar -SIEMBRA_REINTENTO_2_MS de su calendario- el equipo sigue "
                "SIN hora, sin gobernar y sin un solo verde por reloj. La puerta no estaba "
                "abierta esperando: estaba cerrada por la hora, que es la primera");

      comprobar(enHoraDespues && !rtcHwDespues && gobiernaDespues && huboSyncDespues &&
                    respaldoDespues && luzDespues == S_ROJO_V,
                "D12 (el orden, segunda mitad): EN EL TICK de la siembra -y no antes- el "
                "equipo entra en hora y el Degradado vuelve a gobernar, y lo hace por el "
                "TODO-ROJO de degradado_entrar() (la luz pasa a ROJO, no a verde). El "
                "marcador del RTC hardware sigue en false: la hora la trajo el ESP32, que "
                "es lo que D-29 construye, y no un RTC que ningun firmware escribe");

      comprobar(verdesDespues > 0,
                "D13: y a partir de ahi vuelve a dar verde por su reloj (" +
                std::to_string(verdesDespues) + " instantes en los 2 min siguientes), con "
                "una marca de sync de hace " + std::to_string(horasSyncAntes) +
                " h contra un limite de " + std::to_string(LIMITE_SIN_SYNC_H_V) + " h. Sin "
                "este conteo, D12 lo pasaria igual un equipo que entra en Degradado y se "
                "queda en todo-rojo para siempre");

      comprobar(g_verdeSimultaneo == simD,
                "D13b: y en toda esa reanudacion tardia -el Esclavo volviendo a gobernar "
                "por reloj minuto y medio despues del corte, con el Maestro sin enterarse- "
                "NUNCA coincidio un verde en las dos puntas");
    }
  }

  {
    // 🔴 D-29 — LA SEGUNDA PUERTA NO SE TOCA, Y ESTA ES LA LINEA QUE LO EXIGE.
    //
    // El limite duro de 48 h es lo que impide reanudar sobre una marca que ya no significa
    // nada (N-160 / D-20), y D-29 dice explicitamente que no se toca. Un diferimiento
    // escrito de la forma facil -conservar el permiso siempre que falte la hora- la
    // abriria: bastaria que la siembra llegase para que un equipo con la sincronizacion
    // caducada volviera a dar verdes por reloj.
    //
    // Se monta el MISMO escenario cambiando UNA cosa, y por el mismo camino que la pila
    // usa de verdad: el contador del RTC -el registro que la CR2032 mantiene y del que
    // sale la antiguedad de la marca- salta hacia delante el limite duro MAS una hora. No
    // se toca ni el firmware ni el indicador; lo que envejece es el dato, como envejece en
    // la calle. El salto y el corte van SIN un solo tick por medio a proposito: con uno,
    // degradado_actualizar() se rendiria antes y el escenario ya no mediria el arranque.
    escenarioLimpio(tiempos(1, 1, 15));
    relojDelBancoACero();
    {
      unsigned long simE = g_verdeSimultaneo;
      sincronizarEsclavo(10, 8, 0, 0);
      configurarEsclavo(20, 10);
      g_enlaceHaciaEsclavo = false;
      g_enlaceHaciaMaestro = false;
      ESCLAVO.orden("degradado_entrar");
      avanzar(30000);
      const bool gobernabaAntes = ESCLAVO.orden("degradado_gobierna") == 1;

      const long cnt = ESCLAVO.domLeer(10);
      ESCLAVO.domEscribir(10, cnt + (long)((LIMITE_SIN_SYNC_H_V + 1UL) * 3600UL));
      const long horasAntes = ESCLAVO.orden("respaldo_horas_sync");

      microcorte(ESCLAVO);

      const bool respaldoTrasCorte = ESCLAVO.orden("respaldo_degradado") == 1;
      const long horasTrasCorte = ESCLAVO.orden("respaldo_horas_sync");

      sembrarEsclavoDesdeEsp32(10, 9, 0, 0);
      const bool enHora = ESCLAVO.orden("reloj_en_hora") == 1;

      const unsigned long vE0 = g_ticksVerdeEsclavo;
      avanzar(120000);
      const unsigned long verdes = g_ticksVerdeEsclavo - vE0;
      const bool gobierna = ESCLAVO.orden("degradado_gobierna") == 1;

      comprobar(gobernabaAntes && horasAntes > (long)LIMITE_SIN_SYNC_H_V &&
                    horasTrasCorte > (long)LIMITE_SIN_SYNC_H_V,
                "D14 (control): el escenario esta montado -el Esclavo gobernaba por reloj y "
                "su marca de sync sale de hace " + std::to_string(horasTrasCorte) +
                " h contra un limite de " + std::to_string(LIMITE_SIN_SYNC_H_V) + " h-, o "
                "sea que lo que cierra aqui es la SEGUNDA puerta y no la primera");

      comprobar(!respaldoTrasCorte && enHora && !gobierna && verdes == 0,
                "D15: con la sincronizacion pasada del limite duro, el permiso de la pila "
                "se tira EN EL ARRANQUE -no se difiere: ninguna siembra arregla una marca "
                "de hace mas de 48 h- y la hora del ESP32, que si entra, no reanuda nada ni "
                "enciende un verde. El diferimiento de D-29 no roza la segunda puerta");
    }
  }

  {
    // 🔴 D-30 / D-29 — EL AMBAR DE LA APP, DENTRO DE LA VENTANA DEL DIFERIMIENTO.
    //
    // ESTE ESCENARIO MEDIA EL AMBAR DEL MANDO, Y SE REAPUNTA EN VEZ DE BORRARSE
    // (CLAUDE.md §9). Su sujeto era mando_ambarLocal(), armado con B.B.B desde el
    // gabinete; el mando salio del producto el 14/09 (D-30) y con el la guarda que D-29
    // habia puesto dentro de degradado_reanudarTrasCorte() para esa bandera concreta.
    //
    // LO QUE SE MIDE SIGUE EXISTIENDO, PERO LO CIERRA OTRA PUERTA, Y ESA ES LA MITAD QUE
    // HAY QUE ESCRIBIR. El veto que sobrevive es el LATCH DE LA APP
    // -bluetooth_ambarEmergencia()-, y la reanudacion diferida no lo esquiva porque no
    // tiene camino propio: degradado_reanudarTrasCorte() entra por degradado_entrar(),
    // que revalida con degradado_comprobar(), y ahi vive
    // "if (bluetooth_ambarEmergencia()) return DEG_RECHAZO_AMBAR_VIGENTE". Al rechazar,
    // reanudarTrasCorte() hace respaldo_guardarDegradado(false): el permiso se tira
    // igual que antes. O sea que la propiedad es la misma -con un ambar de emergencia
    // puesto, un microcorte no devuelve el poste al Degradado por su cuenta- y lo que
    // cambia es QUIEN la sostiene: ya no una guarda especifica del camino diferido,
    // sino la puerta unica del modo.
    //
    // POR QUE NO ES ADORNO, Y AHORA MENOS QUE ANTES. El ambar del mando habia que
    // justificarlo con el cobre -J16 p5/p8 pelados-, porque con las botoneras
    // desmontadas ninguna persona podia armarlo. El de la app lo arma un tecnico con el
    // telefono todos los dias, y se pide AQUI POR EL CAMINO REAL: tecleaAmbarApp() manda
    // la MISMA LINEA que manda el telefono -leida del C++, no escrita aqui- y la despacha
    // el bluetooth.cpp REAL de esta DLL.
    //
    // Y EL CONTROL VA PEGADO, no en otro escenario: se corre DOS VECES con la MISMA
    // temporizacion y la unica diferencia es la linea del telefono. Sin el, la linea de
    // abajo la pasaria igual de bien un escenario que se quedo sin siembra o sin permiso.
    struct SalidaAmbar { bool ambar; bool permiso; bool permisoFinal; bool gobierna; unsigned long verdes; };
    auto correrConAmbarApp = [&](bool pedirAmbar) -> SalidaAmbar {
      escenarioLimpio(tiempos(1, 1, 15));
      relojDelBancoACero();
      sincronizarEsclavo(10, 8, 0, 0);
      configurarEsclavo(20, 10);
      g_enlaceHaciaEsclavo = false;
      g_enlaceHaciaMaestro = false;
      ESCLAVO.orden("degradado_entrar");
      avanzar(30000);
      microcorte(ESCLAVO);
      avanzar(20000);   // dentro de la ventana, y ya con el ambar de orfandad puesto
      if (pedirAmbar) {
        // La puerta SIN PIN, que es la que usa la app (N-142). La despacha el
        // bluetooth.cpp REAL en su siguiente tick.
        tecleaAmbarApp();
        unTick();
      }
      avanzar(20000);
      SalidaAmbar r;
      r.ambar = ESCLAVO.orden("ambar_latch") == 1;
      // ANTES de la siembra: aqui la decision TODAVIA NO SE HA TOMADO. Se lee para poder
      // decirlo en el mensaje, no para exigir nada (ver el borde, abajo).
      r.permiso = ESCLAVO.orden("respaldo_degradado") == 1;
      sembrarEsclavoDesdeEsp32(10, 8, 40, 0);
      const unsigned long v0 = g_ticksVerdeEsclavo;
      avanzar(120000);
      r.verdes = g_ticksVerdeEsclavo - v0;
      r.gobierna = ESCLAVO.orden("degradado_gobierna") == 1;
      // DESPUES de la siembra: ESTE es el instante en que se decide, y por eso es el que
      // se exige. Ver el borde escrito arriba del comprobar().
      r.permisoFinal = ESCLAVO.orden("respaldo_degradado") == 1;
      return r;
    };
    const SalidaAmbar conAmbar = correrConAmbarApp(true);
    const SalidaAmbar sinAmbar = correrConAmbarApp(false);

    comprobar(conAmbar.ambar && !sinAmbar.ambar,
              "D16 (control): la linea del telefono armo de verdad el latch del ambar de "
              "emergencia -el bluetooth.cpp REAL, por la puerta sin PIN que usa la app-, y "
              "el escenario gemelo sin teclearla no lo armo. Sin esta linea las dos de "
              "abajo mediran lo mismo por casualidad");

    // 🔴 EL BORDE, Y POR QUE SE MIDE DESPUES DE LA SIEMBRA Y NO ANTES (CLAUDE.md §7).
    //
    // La version del mando exigia el permiso ANTES de la siembra, y podia: la guarda de
    // D-29 miraba mando_ambarLocal() en la PRIMERA linea del camino diferido y tiraba el
    // permiso en cuanto la bandera aparecia, sin esperar a tener hora. Retirada esa
    // guarda, lo que cierra es la puerta unica, y la puerta unica NO SE CONSULTA HASTA
    // QUE HAY HORA: mientras falta, degradado_reanudarTrasCorte() se va por la rama del
    // diferimiento -"return false sin borrar"- y el permiso sigue puesto a proposito,
    // porque la decision aun no se ha tomado. Medido: antes de la siembra permiso=1 CON
    // ambar y CON el gemelo sin ambar -o sea que ahi no discrimina nada-; despues de la
    // siembra da 0 con ambar y 1 sin el.
    //
    // O SEA QUE LO QUE SE MOVIO ES EL INSTANTE, NO LA PROPIEDAD, y exigirla en el
    // instante viejo seria acusar al firmware de un defecto que no tiene. El instante
    // correcto es aquel en que la decision existe: cuando la hora ya llego. Que la
    // propiedad sigue siendo una propiedad -y no algo que se cumple solo- lo demuestra
    // D18, que es el mismo escenario sin la linea del telefono y acaba con permiso=1.
    comprobar(!conAmbar.permisoFinal && !conAmbar.gobierna && conAmbar.verdes == 0,
              "D17: con el ambar de emergencia de la app puesto DENTRO de la ventana, la "
              "reanudacion diferida NO ocurre: una vez llega la hora del ESP32 -que es "
              "cuando la decision se toma- el permiso de la pila se tira, y ni se gobierna "
              "la luz ni se enciende un verde. Quien lo cierra es la PUERTA UNICA "
              "-degradado_reanudarTrasCorte() entra por degradado_entrar(), que revalida "
              "con degradado_comprobar() y ahi vive el rechazo por ambar vigente-, no una "
              "guarda propia del camino diferido: desde D-30 esa guarda ya no existe");

    comprobar(sinAmbar.permisoFinal && sinAmbar.gobierna && sinAmbar.verdes > 0,
              "D18 (el control negativo de D17): el MISMO escenario con la MISMA "
              "temporizacion y sin la linea del telefono SI reanuda (" +
              std::to_string(sinAmbar.verdes) + " verdes por reloj). O sea que lo que "
              "paro la reanudacion fue el latch del ambar y no el escenario");
  }

  {
    // 🔴 EL CONTROL DE LA INVERSION, CON SUJETO NUEVO DESDE EL 12/09 (CLAUDE.md §9).
    //
    // Hasta hoy este escenario era el control de D6b: demostraba que la reanudacion de
    // N-20 seguia VIVA y que lo unico que la apagaba era que nadie escribe ya el RTC
    // hardware. Con D-29 construida las DOS ramas acaban reanudando, asi que como control
    // de "reanuda o no" ya no distingue nada y habria que borrarlo o darle otro sujeto.
    //
    // SE LE DA OTRO SUJETO, Y NO ES RELLENO. Aqui NO SE ENTREGA NINGUNA SIEMBRA, y aun asi
    // el equipo reanuda EN setup(). Eso separa las dos causas que D10-D13 podrian estar
    // confundiendo: alli la reanudacion la trae la siembra dentro de la ventana; aqui la
    // trae el RTC hardware sin ventana ninguna. Si el diferimiento hubiera roto el camino
    // que ya funcionaba -por ejemplo dejando la decision pendiente para siempre en vez de
    // tomarla cuando se puede-, esta linea caeria y las de alla no. Es ademas la mitad de
    // la divergencia de flotas que D-29 dice cerrar: la tarjeta anterior al 11/09 sigue
    // haciendo lo de siempre.
    //
    // El dominio de la pila llega con el marcador del RTC hardware PUESTO. No es una
    // puerta de atras al firmware: es el silicio de un equipo cuyo RTC escribio un
    // firmware ANTERIOR al 11/09 -y con la CR2032 dentro, ese marcador sobrevive incluso a
    // una recarga por SWD-.
    escenarioLimpio(tiempos(1, 1, 15));
    unsigned long sim0 = g_verdeSimultaneo;
    sincronizarEsclavo(10, 8, 0, 0);
    configurarEsclavo(20, 10);
    g_enlaceHaciaEsclavo = false;
    g_enlaceHaciaMaestro = false;
    ESCLAVO.orden("degradado_entrar");
    avanzar(30000);
    const bool gobernabaAntes = ESCLAVO.orden("degradado_gobierna") == 1;

    microcorte(ESCLAVO, 1);   // el unico cambio: el RTC hardware venia escrito

    const bool enHora   = ESCLAVO.orden("reloj_en_hora") == 1;
    const bool gobierna = ESCLAVO.orden("degradado_gobierna") == 1;
    const bool huboSync = ESCLAVO.orden("degradado_hubo_sync") == 1;
    const bool respaldoSigue = ESCLAVO.orden("respaldo_degradado") == 1;
    const unsigned long vE0 = g_ticksVerdeEsclavo;
    avanzar(120000);
    const unsigned long verdesTrasCorte = g_ticksVerdeEsclavo - vE0;

    comprobar(gobernabaAntes && enHora && gobierna && huboSync && respaldoSigue &&
                  verdesTrasCorte > 0,
              "D8 (el control de D10-D13 desde el 12/09): con el MISMO escenario, el RTC "
              "hardware escrito -un equipo anterior al 11/09- y SIN ENTREGAR NI UNA "
              "SIEMBRA, el Esclavo reanuda el Degradado ya en setup() y vuelve a encender "
              "verde por su reloj (" + std::to_string(verdesTrasCorte) + " instantes). "
              "Separa las dos causas: alla reanuda porque la siembra llego dentro de la "
              "ventana, aqui porque la hora estaba desde el arranque. Y demuestra que el "
              "diferimiento de D-29 no rompio el camino que ya funcionaba -la mitad de la "
              "divergencia de flotas que D-29 cierra-");
    comprobar(g_verdeSimultaneo == sim0,
              "D9: y en ese caso -el peor de los dos, porque aqui el Esclavo SI vuelve a dar "
              "verde por su reloj con el Maestro sin enterarse- tampoco coincidio un verde "
              "en las dos puntas. Sin el conteo de D8 esta linea seria adorno: una ventana "
              "sin un solo verde del Esclavo la pasaria igual");
  }

  // =========================================================================
  std::printf("\n--- BLOQUE F: un ACK_GREEN perdido y el GO_GREEN que se repite -----\n");
  // N-162. El banco del 04/09 -"queda maestro en rojo y ... esclavo [en] rojo y ambar",
  // "el cruce esta cambiando de fase, repita"- y el Sisga el 10/09. El mecanismo esta en el
  // fuente: el Maestro repite GO_GREEN cada TIMEOUT_ACK_MS mientras no le llegue el
  // ACK_GREEN, y si cada repeticion REINICIA la transicion del Esclavo, perder acuses basta
  // para que su ambar no termine mientras duren los reintentos -o para que un verde vuelva a
  // ambar-. Aqui los acuses se pierden A PROPOSITO y se exige que la luz no lo note.
  //
  // EL BORDE DE (a), escrito al lado (CLAUDE.md §7): el ambar dura lo que dice la condicion
  // de semaforo_actualizar() -AMBAR_ESCLAVO_MS_V, releido- contado desde el tick en que la
  // luz ENTRO en ambar, con UN tick del arnes (PASO_MS) de tolerancia por arriba, que es la
  // granularidad con la que se miran los pines y nada mas. NI MAS CORTO -la Resolucion pide
  // ese aviso entero, y un "arreglo" que saltara a verde con la repeticion lo recortaria-
  // NI MAS LARGO, que es el defecto.
  {
    const unsigned long AMBAR = AMBAR_ESCLAVO_MS_V;
    const unsigned long TOUT  = TIMEOUT_ACK_MS_V;
    const unsigned long NMAX  = CICLO_MAX_REINTENTOS_V;

    // F-a: se pierden los NMAX-1 primeros ACK_GREEN. Es la MAYOR perdida que no agota los
    // reintentos del Maestro: emite NMAX ordenes como mucho, y con la rama vieja cada una
    // produce un solo acuse, asi que el ultimo pasa. Es el caso del banco: reintentos
    // cayendo uno tras otro dentro del ambar.
    const long ACK_A = (long)NMAX - 1;
    // F-b: para que un GO_GREEN repetido encuentre al Esclavo YA EN VERDE con cualquier
    // firmware -tambien con el viejo, que es el control-, entre dos GO_GREEN ENTREGADOS
    // tiene que pasar mas que el ambar. El Maestro los separa TOUT, asi que se tiran los
    // n = AMBAR / TOUT reintentos siguientes al primero (ordinales 2..n+1). Y se pierden los
    // DOS acuses que el Esclavo emite antes -el de la orden y el de "ya estoy en verde"-,
    // porque cualquiera de los dos cerraria la espera del Maestro antes de tiempo.
    const unsigned long N_GO_B = AMBAR / TOUT;
    const long ACK_B = 2;
    if (ACK_A < 1 || N_GO_B + 2 > NMAX) {
      abortar("con estas constantes (ambar " + std::to_string(AMBAR) + " ms, timeout " +
              std::to_string(TOUT) + " ms, " + std::to_string(NMAX) + " reintentos) el "
              "bloque F no se puede montar sin que el Maestro agote sus reintentos: hay que "
              "rehacer el escenario, no darlo por bueno");
    }

    struct Fase {
      bool pilladoAmbar = false, pilladoVerde = false;
      unsigned long tAmbar = 0, tVerde = 0;
      unsigned long ackPerdidos = 0, goPerdidos = 0;
      unsigned long entregadoEn[4] = { 0, 0, 0, 0 };
      unsigned long verdeAAmbar = 0, simultaneo = 0, sinRojo = 0;
      bool maestroAcuso = false, maestroFallo = false;
    };

    auto correrFase = [&](long ackAPerder, unsigned long goDesde, unsigned long goHasta) -> Fase {
      Fase f;
      escenarioLimpio(tiempos(1, 1, 15));
      g_ackVerdeAPerder = ackAPerder;
      g_goVerdePerderDesde = goDesde;
      g_goVerdePerderHasta = goHasta;
      const unsigned long ack0 = g_ackVerdePerdidos, go0 = g_goVerdePerdidos;
      unsigned long ent0[4];
      for (int i = 0; i < 4; i++) ent0[i] = g_goVerdeEntregadoEn[i];
      const unsigned long va0 = g_escVerdeAAmbar, sim0 = g_verdeSimultaneo;
      const unsigned long sr0 = g_verdeSinRojoEnfrente;

      // 1. Hasta que el Esclavo ENTRA en ambar: el primer GO_GREEN de su fase.
      for (unsigned long gastado = 0; gastado < 400000; gastado += PASO_MS) {
        const unsigned long t = g_t;
        unTick();
        if (ESCLAVO.estado() == S_AMARILLO_V) { f.pilladoAmbar = true; f.tAmbar = t; break; }
      }
      // 2. Hasta VERDE en los pines. El presupuesto cubre el peor caso del DEFECTO -cada
      //    reintento reinicia el ambar- para poder decir cuanto se alarga, no solo que falla.
      if (f.pilladoAmbar) {
        const unsigned long presupuesto = AMBAR + TOUT * (NMAX + 1) + 15000;
        for (unsigned long gastado = 0; gastado < presupuesto; gastado += PASO_MS) {
          const unsigned long t = g_t;
          unTick();
          if (ESCLAVO.verde()) { f.pilladoVerde = true; f.tVerde = t; break; }
        }
      }
      // 3. Y hasta que el Maestro deja de esperar -le llega un acuse o agota reintentos-,
      //    mas un segundo para que lo que quede en el aire aterrice. Es en esta ventana
      //    donde un GO_GREEN repetido encuentra al Esclavo ya en verde.
      for (unsigned long gastado = 0; gastado < TOUT * (NMAX + 1); gastado += PASO_MS) {
        unTick();
        if (MAESTRO.orden("listo_para_contar") == 1 ||
            MAESTRO.orden("comunicacion_perdida") == 1) break;
      }
      avanzar(1000);
      // "Acuso" = el coordinador salio de la espera por un ACK_GREEN: esta en reposo y
      // publica al Esclavo en verde. Salir por reintentos agotados es C_FALLO, no esto.
      f.maestroAcuso = MAESTRO.orden("listo_para_contar") == 1 &&
                       MAESTRO.orden("esc_publica_verde") == 1;
      f.maestroFallo = MAESTRO.orden("comunicacion_perdida") == 1;

      f.ackPerdidos = g_ackVerdePerdidos - ack0;
      f.goPerdidos  = g_goVerdePerdidos - go0;
      for (int i = 0; i < 4; i++) f.entregadoEn[i] = g_goVerdeEntregadoEn[i] - ent0[i];
      f.verdeAAmbar = g_escVerdeAAmbar - va0;
      f.simultaneo  = g_verdeSimultaneo - sim0;
      f.sinRojo     = g_verdeSinRojoEnfrente - sr0;
      g_ackVerdeAPerder = 0;
      g_goVerdePerderDesde = g_goVerdePerderHasta = 0;
      return f;
    };

    const Fase fa = correrFase(ACK_A, 0, 0);
    const Fase fb = correrFase(ACK_B, N_GO_B ? 2 : 0, N_GO_B ? 1 + N_GO_B : 0);

    const unsigned long dAmbarA = fa.pilladoVerde ? fa.tVerde - fa.tAmbar : 0;
    const unsigned long repetidosA = fa.entregadoEn[S_AMARILLO_V] + fa.entregadoEn[S_VERDE_V];

    comprobar(fa.pilladoAmbar && fa.ackPerdidos == (unsigned long)ACK_A && repetidosA >= 1 &&
              !fa.maestroFallo,
              "F1 (control de F-a): la perdida OCURRIO como se pidio -" +
              std::to_string(fa.ackPerdidos) + " de " + std::to_string(ACK_A) + " ACK_GREEN "
              "tirados- y el Maestro repitio la orden: " + std::to_string(repetidosA) +
              " GO_GREEN llegaron con la transicion ya empezada (" +
              std::to_string(fa.entregadoEn[S_AMARILLO_V]) + " en ambar, " +
              std::to_string(fa.entregadoEn[S_VERDE_V]) + " en verde), sin agotar reintentos");

    comprobar(fa.pilladoVerde && dAmbarA >= AMBAR && dAmbarA <= AMBAR + PASO_MS,
              "F2 (a): con esos acuses perdidos, el ambar del Esclavo antes de VERDE midio " +
              std::to_string(dAmbarA) + " ms; se exige [" + std::to_string(AMBAR) +
              ", " + std::to_string(AMBAR + PASO_MS) + "] (ambar releido + un tick). Si cada "
              "GO_GREEN reiniciara el ambar, seria del orden de " + std::to_string(ACK_A) +
              " x timeout + ambar = " + std::to_string((unsigned long)ACK_A * TOUT + AMBAR) +
              " ms, con el Maestro en rojo 'en transicion' todo ese tiempo" +
              (fa.pilladoVerde ? "" : " -NO LLEGO A VERDE en el presupuesto-"));

    comprobar(fb.pilladoVerde && fb.ackPerdidos == (unsigned long)ACK_B &&
              fb.goPerdidos == N_GO_B && fb.entregadoEn[S_VERDE_V] >= 1,
              "F3 (control de F-b): se tiraron " + std::to_string(fb.ackPerdidos) + " de " +
              std::to_string(ACK_B) + " ACK_GREEN y " + std::to_string(fb.goPerdidos) + " de " +
              std::to_string(N_GO_B) + " GO_GREEN repetidos, y " +
              std::to_string(fb.entregadoEn[S_VERDE_V]) + " GO_GREEN llegaron con el Esclavo "
              "YA EN VERDE: el caso del reintento tardio se ejercio de verdad");

    comprobar(g_escVerdeAAmbar == 0,
              "F4 (b): en TODO el barrido (bloques A a F) el Esclavo nunca paso de VERDE a "
              "AMARILLO -la Resolucion da verde->rojo directo-. En F-a y F-b llegaron " +
              std::to_string(fa.entregadoEn[S_VERDE_V] + fb.entregadoEn[S_VERDE_V]) +
              " GO_GREEN con el Esclavo en verde. Transiciones verde->ambar: " +
              std::to_string(g_escVerdeAAmbar) + " (en F: " +
              std::to_string(fa.verdeAAmbar + fb.verdeAAmbar) + ")");

    comprobar(fa.maestroAcuso && fb.maestroAcuso && !fa.maestroFallo && !fb.maestroFallo,
              "F5: el GO_GREEN repetido SE RE-ACUSA: en F-a y en F-b el Maestro salio de la "
              "espera por un ACK_GREEN (reposo y ESC:VERDE), no por reintentos agotados. Una "
              "orden repetida que no reiniciara nada pero CALLARA dejaria al Maestro "
              "reintentando hasta C_FALLO");

    comprobar(fa.simultaneo == 0 && fb.simultaneo == 0 && fa.sinRojo == 0 && fb.sinRojo == 0,
              "F6 (c): durante F-a y F-b nunca hubo verde en las dos puntas, y con el Esclavo "
              "en verde el Maestro tuvo SIEMPRE sus dos rojos encendidos");
  }

  // =========================================================================
  std::printf("\n--- BLOQUE G: una punta en VERDE y la otra ABIERTA o sin rojo --------\n");
  // N-162, lo que el bloque F dejo a la vista: con acuses perdidos el Esclavo ya esta en
  // VERDE mientras el Maestro espera, y A9 excluye S_FALLO por nombre. Aqui se MIDE la
  // ventana -instantes con una punta en verde y la otra sin sus dos rojos, S_FALLO incluido-,
  // cuanto dura y QUE la cierra, con la radio cortada en una direccion, en las dos, o
  // perdiendo tramas concretas.
  //
  // EL BORDE, escrito al lado (CLAUDE.md 7): UN VIAJE DE RADIO (g_latenciaMs) MAS UN TICK DE
  // OBSERVACION (PASO_MS). Es lo minimo que puede durar la ventana cuando una punta decide
  // abrir -o cerrar- y la otra solo puede enterarse por radio: la orden sale en el mismo
  // instante, cruza una vez y se ejecuta en el tick en que llega. Todo lo que pase de ahi
  // es una punta ESPERANDO algo que no necesita esperar -el siguiente latido, un umbral
  // igual al de la otra punta, un acuse que no pide-, y eso lo puede cerrar un firmware:
  // por eso por encima del borde es FALLA y no reportar(). Ni una tolerancia mas: la
  // excepcion de A9 ya es una tolerancia infinita, y es la que este bloque viene a medir.
  //
  // QUE CADA FALLA SE PUEDE APAGAR, Y QUE CADA OK SE PUEDE ENCENDER (11/09, sobre una COPIA
  // de coordinador.cpp fuera del arbol; el firmware no se toco): con tres parches de prueba
  // -GO_RED al entrar en C_FALLO, verde propio soltado antes que la orfandad de enfrente, y
  // verde propio solo tras ACK_RED- el arnes da todo OK, y cada parche apaga SOLO su fila.
  // Con defectos inyectados caen G2 (a, b) -sin GO_RED en C_FALLO y "reintentos agotados =
  // enlace perdido"- y G4 -Maestro en rojo que se abre 3 s antes que la orfandad-.
  //
  // 11/09, EL ARREGLO YA EN EL FIRMWARE (coordinador.cpp, rojoEsclavoConfirmado): GO_RED al
  // entrar en C_FALLO, y ningun verde propio sin el ACK_RED de su GO_RED -autorrecuperacion,
  // iniciarModo y DAR PASO tras un ROJO TOTAL-. Contra el coordinador.cpp de 648b62f
  // este mismo bloque da G1, G2-d, G5, G6, G7, G8 y G10 en FALLA (180 s con verde en las
  // dos en G2-d, G5, G6-b y G7; 431 s de verde frente al ambar de emergencia en G8-a).
  // G3 sigue en FALLA a proposito: cerrarlo toca SFTY-6 y es del responsable.
  const unsigned long BORDE_MS = g_latenciaMs + PASO_MS;
  const unsigned long a9PerdonadosAF = g_a9Perdonados, a9RachaAF = g_a9RachaMax;
  {
    const unsigned long TOUT = TIMEOUT_ACK_MS_V;
    const unsigned long NMAX = CICLO_MAX_REINTENTOS_V;
    const unsigned long SIL  = SFTY6_SILENCIO_MS_V;
    // EL CICLO DEL BLOQUE: los minimos de limites_ciclo.h, releidos, y con el rechazo del
    // firmware convertido en ABORTADO (ver escenarioLimpio). Con los minimos cada ventana
    // que dure "lo que dure el verde" sale con su valor MAS CORTO posible: con otro ciclo
    // solo puede crecer, hasta VERDE_MIN_MAX.
    const long TIEMPOS_G = tiempos((int)VERDE_MIN_MIN_V, (int)ROJO_MIN_MIN_V,
                                   (int)DESPEJE_POR_DEFECTO_S);
    const unsigned long DESPEJE_MS = DESPEJE_POR_DEFECTO_S * 1000UL;
    const unsigned long VERDE_MS = VERDE_MIN_MIN_V * 60000UL;
    const unsigned long ROJO_MS = ROJO_MIN_MIN_V * 60000UL;
    const unsigned long ALCANCE = 2 * (DESPEJE_MS + AMBAR_ESCLAVO_MS_V) + VERDE_MS + ROJO_MS + 60000;
    const unsigned long POST = SIL + TOUT * (NMAX + 2) + 30000;   // el mismo de C6/C7
    // Lo que tarda en cerrarse una ventana que dura "el verde del Maestro": su despeje, su
    // ambar, su verde entero y un minuto para ver quien la cierra.
    const unsigned long TRAS_VERDE = DESPEJE_MS + AMBAR_ESCLAVO_MS_V + VERDE_MS + 60000;

    std::printf("   Ciclo del bloque: verde %lu min, rojo %lu min, despeje %lu s (minimos de "
                "limites_ciclo.h, aceptados por el firmware).\n",
                VERDE_MIN_MIN_V, ROJO_MIN_MIN_V, DESPEJE_POR_DEFECTO_S);
    std::printf("   Borde: %lu ms (un viaje de radio del arnes, %lu ms, + un tick, %lu ms).\n",
                BORDE_MS, g_latenciaMs, PASO_MS);
    std::printf("   Plazos releidos: latido %lu ms, cortesia del Esclavo %lu ms, silencio SFTY-6 "
                "%lu ms, verde maximo del Esclavo %lu ms.\n",
                LATIDO_MS_V, RETARDO_RESPUESTA_MS_V, SIL, MAX_VERDE_BACKSTOP_MS_V);

    comprobar(hayVerdeFrenteASinRojo(true, false) && !hayVerdeFrenteASinRojo(true, true) &&
              !hayVerdeFrenteASinRojo(false, false) && !hayVerdeFrenteASinRojo(false, true),
              "G0 (control negativo): el detector de la ventana SI dispara con verde frente a "
              "una punta sin rojo y NO con verde frente a rojo ni sin verde");

    // ---- G1: Esclavo -> Maestro muerto con el ESCLAVO en verde ------------------------
    // El corte se barre en pasos de 250 ms a lo largo de un latido entero: la ventana
    // depende de en que punto del latido cae el silencio, y un solo instante mediria la
    // fase que tocara, no la peor.
    unsigned long g1Max = 0, g1Min = (unsigned long)-1, g1PeorOff = 0, g1Fases = 0;
    bool g1Ejercido = true;
    CorridaG g1Peor;
    for (unsigned long off = 0; off <= LATIDO_MS_V; off += 250) {
      CorridaG c;
      g1Fases++;
      escenarioLimpio(TIEMPOS_G, true);
      bool ok = alcanzarVerdeG(ESCLAVO, ALCANCE);
      avanzar(off);
      ok = ok && ESCLAVO.verde() && MAESTRO.rojo();
      g_enlaceHaciaMaestro = false;
      c.tCorte = g_t;
      correrG(c, POST);
      finG(c);
      if (!(ok && c.maestroFallo && c.esclavoAlFallo == S_VERDE_V && !ESCLAVO.verde()))
        g1Ejercido = false;
      const unsigned long acu = acumuladoMsG(c.v);
      if (acu < g1Min) g1Min = acu;
      if (acu >= g1Max) { g1Max = acu; g1PeorOff = off; g1Peor = c; }
    }
    imprimirTrazaG(("G1, peor fase (corte " + std::to_string(g1PeorOff) + " ms despues de "
                    "encenderse el verde del Esclavo; en las " + std::to_string(g1Fases) +
                    " fases la ventana midio entre " + std::to_string(g1Min) + " y " +
                    std::to_string(g1Max) + " ms):").c_str(), g1Peor);
    comprobar(g1Ejercido,
              "G1 (control): en las " + std::to_string(g1Fases) + " fases, el "
              "corte Esclavo->Maestro cayo con el Esclavo en VERDE y el Maestro en rojo, el "
              "Maestro llego a C_FALLO con el Esclavo TODAVIA en verde, y el Esclavo acabo "
              "fuera de verde");
    comprobar(g1Max <= BORDE_MS,
              "G1: Esclavo->Maestro muerto con el Esclavo en verde. El Maestro cae a S_FALLO "
              "(ambar intermitente, pluma ARRIBA) por silencio y el Esclavo sigue en verde hasta "
              "que le llega un GO_RED: ventana " + std::to_string(g1Max) + " ms en la peor fase (" +
              std::to_string(g1PeorOff) + " ms); borde " + std::to_string(BORDE_MS) + " ms" +
              (g1Max <= BORDE_MS ? std::string(": el GO_RED sale AL ENTRAR en C_FALLO")
                                 : std::string(". Por encima del borde el GO_RED espera al LATIDO (") +
                                   std::to_string(LATIDO_MS_V) + " ms) y los PING que siguen "
                                   "llegando le refrescan la orfandad"));

    // ---- G2: el Maestro esperando su ACK_GREEN ----------------------------------------
    //   a) Esclavo->Maestro muerto desde el GO_GREEN, para siempre.
    //   b) solo se pierden los ACK_GREEN; el resto del enlace vive.
    //   d) corte TOTAL desde el GO_GREEN hasta el instante siguiente a C_FALLO, y vuelve.
    //      Es la averia de lluvia: un desvanecimiento de ~18 s que se lleva los acuses y,
    //      en su ultimo instante, el GO_RED con el que el Maestro se autorrecupera.
    CorridaG g2a, g2b, g2d;
    bool g2Ejercido = true;
    unsigned long g2dGoRojoCortados = 0;
    {
      escenarioLimpio(TIEMPOS_G, true);
      g2Ejercido = alcanzarGoVerdeG(ALCANCE) && g2Ejercido;
      g_enlaceHaciaMaestro = false;
      g2a.tCorte = g_t;
      correrG(g2a, TOUT * (NMAX + 1) + POST);
      finG(g2a);
    }
    {
      escenarioLimpio(TIEMPOS_G, true);
      g2Ejercido = alcanzarGoVerdeG(ALCANCE) && g2Ejercido;
      g_ackVerdeAPerder = 1000000;   // todos: el escenario acaba antes del siguiente verde
      g2b.tCorte = g_t;
      correrG(g2b, TOUT * (NMAX + 1) + POST);
      finG(g2b);
      g_ackVerdeAPerder = 0;
    }
    {
      escenarioLimpio(TIEMPOS_G, true);
      g2Ejercido = alcanzarGoVerdeG(ALCANCE) && g2Ejercido;
      g_enlaceHaciaMaestro = g_enlaceHaciaEsclavo = false;
      g2d.tCorte = g_t;
      for (unsigned long g = 0; g < TOUT * (NMAX + 2) && !g2d.maestroFallo; g += PASO_MS) pasoG(g2d);
      const unsigned long cortados0 = g_goRojoCortados;
      pasoG(g2d);   // el instante siguiente a C_FALLO: la autorrecuperacion emite su GO_RED
      g2dGoRojoCortados = g_goRojoCortados - cortados0;
      g_enlaceHaciaMaestro = g_enlaceHaciaEsclavo = true;
      correrG(g2d, TRAS_VERDE);
      finG(g2d);
    }
    imprimirTrazaG("G2-a (Esclavo->Maestro muerto desde el GO_GREEN):", g2a);
    imprimirTrazaG("G2-b (solo se pierden los ACK_GREEN):", g2b);
    imprimirTrazaG(("G2-d (corte total desde el GO_GREEN hasta el instante siguiente a "
                    "C_FALLO; " + std::to_string(g2dGoRojoCortados) + " GO_RED perdidos en ese "
                    "instante):").c_str(), g2d);
    comprobar(g2Ejercido && g2a.maestroFallo && g2b.maestroFallo && g2d.maestroFallo &&
              g2a.esclavoAlFallo == S_VERDE_V && g2b.esclavoAlFallo == S_VERDE_V &&
              g2d.esclavoAlFallo == S_VERDE_V && g2dGoRojoCortados >= 1,
              "G2 (control): en a, b y d el Maestro agoto reintentos esperando el ACK_GREEN con "
              "el Esclavo YA EN VERDE (la idempotencia de hoy), y en d el corte se llevo " +
              std::to_string(g2dGoRojoCortados) + " GO_RED en el instante de la autorrecuperacion");
    comprobar(acumuladoMsG(g2a.v) <= BORDE_MS && acumuladoMsG(g2b.v) <= BORDE_MS,
              "G2 (a, b): con la otra direccion viva el Maestro NO se abre: al agotar reintentos "
              "se AUTORRECUPERA en el instante siguiente -silencio aun por debajo de " +
              std::to_string(SIL) + " ms- con rojo y un GO_RED que llega. Ventana: " +
              std::to_string(acumuladoMsG(g2a.v)) + " ms (a), " +
              std::to_string(acumuladoMsG(g2b.v)) + " ms (b); "
              "borde " + std::to_string(BORDE_MS) + " ms");
    comprobar(acumuladoMsG(g2d.v) <= BORDE_MS && g2d.v.simultaneo == 0,
              "G2-d: perdido el GO_RED de la autorrecuperacion (SFTY-9) y vuelto el enlace, el "
              "Maestro NO se abre sin el ACK_RED de su GO_RED: ventana " +
              std::to_string(acumuladoMsG(g2d.v)) + " ms, " +
              std::to_string(g2d.v.simultaneo * PASO_MS) + " ms con VERDE EN LAS DOS; borde " +
              std::to_string(BORDE_MS) + " ms" +
              (acumuladoMsG(g2d.v) <= BORDE_MS ? std::string("")
               : std::string(". Se abrio a los ") + std::to_string(DESPEJE_MS) + " ms de despeje "
                 "contando con un GO_RED que no llego: los PING le refrescan la orfandad al "
                 "Esclavo, que sigue en verde"));
    // El control de la inversion (CLAUDE.md 9): un Maestro que no se abriera NUNCA -atascado
    // en rojo con la radio viva- pasaria la linea de arriba igual de bien.
    comprobar(g2d.tAperturaM >= 0 && todoRojoMsG(g2d) >= (long)DESPEJE_MS,
              "G2-d (control de la inversion): el Maestro SI se abre despues -no se queda en rojo "
              "con la radio viva- y con el todo-rojo entero: " + std::to_string(todoRojoMsG(g2d)) +
              " ms entre el ultimo verde del Esclavo y el ambar del Maestro; despeje " +
              std::to_string(DESPEJE_MS) + " ms (-1 = no se abrio en " +
              std::to_string(TRAS_VERDE) + " ms)");

    // ---- G3: Maestro -> Esclavo muerto con el MAESTRO en verde (la simetrica) ----------
    unsigned long g3Max = 0, g3Min = (unsigned long)-1, g3PeorOff = 0, g3Fases = 0;
    bool g3Ejercido = true;
    CorridaG g3Peor;
    for (unsigned long off = 0; off <= LATIDO_MS_V; off += 250) {
      CorridaG c;
      g3Fases++;
      escenarioLimpio(TIEMPOS_G, true);
      bool ok = alcanzarVerdeG(MAESTRO, ALCANCE);
      avanzar(off);
      ok = ok && MAESTRO.verde() && ESCLAVO.rojo();
      g_enlaceHaciaEsclavo = false;
      c.tCorte = g_t;
      correrG(c, POST);
      finG(c);
      if (!(ok && c.maestroFallo && ESCLAVO.estado() == S_FALLO_V && !MAESTRO.verde()))
        g3Ejercido = false;
      const unsigned long acu = acumuladoMsG(c.v);
      if (acu < g3Min) g3Min = acu;
      if (acu >= g3Max) { g3Max = acu; g3PeorOff = off; g3Peor = c; }
    }
    imprimirTrazaG(("G3, peor fase (corte " + std::to_string(g3PeorOff) + " ms despues de "
                    "encenderse el verde del Maestro; en las " + std::to_string(g3Fases) +
                    " fases la ventana midio entre " + std::to_string(g3Min) + " y " +
                    std::to_string(g3Max) + " ms):").c_str(), g3Peor);
    comprobar(g3Ejercido,
              "G3 (control): en todas las fases el corte Maestro->Esclavo cayo con el Maestro en "
              "VERDE y el Esclavo en rojo, el Esclavo acabo en S_FALLO por orfandad y el Maestro "
              "en C_FALLO sin verde");
    comprobar(g3Max <= BORDE_MS,
              "G3: Maestro->Esclavo muerto con el Maestro en verde. El Esclavo cae a S_FALLO "
              "(pluma ARRIBA) por orfandad ANTES de que el Maestro apague su verde por silencio: "
              "los dos umbrales son el mismo SFTY6_SILENCIO_MS y el Maestro cuenta desde el PONG, "
              "que sale " + std::to_string(RETARDO_RESPUESTA_MS_V) + " ms (cortesia) + un viaje "
              "despues del PING que cuenta el Esclavo. Ventana " +
              std::to_string(g3Max) + " ms en la peor fase (" + std::to_string(g3PeorOff) +
              " ms); borde " +
              std::to_string(BORDE_MS) + " ms");

    // ---- G4: los controles, el mismo detector donde el orden SI es el bueno ------------
    //   a) Maestro->Esclavo muerto con el ESCLAVO en verde: su orfandad lo saca a S_FALLO
    //      con el Maestro en rojo.
    //   b) corte total con el ESCLAVO en verde: el Esclavo sale de verde antes de que el
    //      Maestro se abra, por la misma cortesia que en G3 juega al reves.
    CorridaG g4a, g4b;
    bool g4Ejercido = true;
    for (int k = 0; k < 2; k++) {
      CorridaG& c = (k == 0) ? g4a : g4b;
      escenarioLimpio(TIEMPOS_G, true);
      bool ok = alcanzarVerdeG(ESCLAVO, ALCANCE);
      avanzar(1000);
      ok = ok && ESCLAVO.verde();
      g_enlaceHaciaEsclavo = false;
      if (k == 1) g_enlaceHaciaMaestro = false;
      c.tCorte = g_t;
      correrG(c, POST);
      finG(c);
      if (!(ok && c.maestroFallo && ESCLAVO.estado() == S_FALLO_V)) g4Ejercido = false;
    }
    imprimirTrazaG("G4-a (Maestro->Esclavo muerto con el Esclavo en verde):", g4a);
    imprimirTrazaG("G4-b (corte total con el Esclavo en verde):", g4b);
    comprobar(g4Ejercido && acumuladoMsG(g4a.v) <= BORDE_MS && acumuladoMsG(g4b.v) <= BORDE_MS,
              "G4 (control positivo): donde el orden de salida es el bueno el MISMO detector da " +
              std::to_string(acumuladoMsG(g4a.v)) + " ms (a) y " +
              std::to_string(acumuladoMsG(g4b.v)) + " ms (b) "
              "sobre el C++ real: la ventana no es un artefacto del arnes que salga siempre");

    // ---- G5: cambio de modo con el Esclavo en verde y el GO_RED de arranque perdido -----
    // coordinador_iniciarModo() tiene la misma forma que la autorrecuperacion: UN GO_RED,
    // despeje y verde propio, sin ACK_RED. Se entra en Automatico con el Esclavo en verde -lo
    // que hace un operario que cambia de modo desde la app- y se pierde esa unica trama.
    CorridaG g5;
    bool g5Ejercido;
    {
      escenarioLimpio(TIEMPOS_G, true);
      bool ok = alcanzarVerdeG(ESCLAVO, ALCANCE);
      avanzar(3000);
      ok = ok && ESCLAVO.verde();
      const unsigned long perd0 = g_goRojoPerdidos, entr0 = g_goRojoEntregados;
      g_goRojoAPerder = 1;
      MAESTRO.orden("arrancar_automatico");
      g5.tCorte = g_t;
      correrG(g5, TRAS_VERDE);
      finG(g5);
      g5Ejercido = ok && g_goRojoPerdidos - perd0 == 1 && g_goRojoAPerder == 0;
      std::printf("      (G5: GO_RED tirados %lu, GO_RED entregados despues %lu)\n",
                  g_goRojoPerdidos - perd0, g_goRojoEntregados - entr0);
    }
    imprimirTrazaG("G5 (entrada en Automatico con el Esclavo en verde, su GO_RED perdido):", g5);
    comprobar(g5Ejercido,
              "G5 (control): se entro en Automatico con el Esclavo en VERDE y se perdio "
              "EXACTAMENTE el GO_RED de coordinador_iniciarModo()");
    comprobar(acumuladoMsG(g5.v) <= BORDE_MS && g5.v.simultaneo == 0,
              "G5: perdido el GO_RED de coordinador_iniciarModo(), el Maestro NO se abre sin su "
              "ACK_RED: ventana " + std::to_string(acumuladoMsG(g5.v)) + " ms, " +
              std::to_string(g5.v.simultaneo * PASO_MS) + " ms con VERDE EN LAS DOS; borde " +
              std::to_string(BORDE_MS) + " ms" +
              (acumuladoMsG(g5.v) <= BORDE_MS ? std::string("")
               : std::string(". Se abrio a los ") + std::to_string(DESPEJE_MS) +
                 " ms contando con una trama que no llego"));
    comprobar(g5.tAperturaM >= 0 && todoRojoMsG(g5) >= (long)DESPEJE_MS,
              "G5 (control de la inversion): el Maestro SI se abre despues y con el todo-rojo "
              "entero: " + std::to_string(todoRojoMsG(g5)) + " ms entre el ultimo verde del "
              "Esclavo y el ambar del Maestro; despeje " + std::to_string(DESPEJE_MS) +
              " ms (-1 = no se abrio en " + std::to_string(TRAS_VERDE) + " ms)");

    // ---- G6: ROJO TOTAL con el Esclavo en verde y su GO_RED perdido --------------------
    // coordinador_forzarRojoTotal() -el rojo de emergencia de la app y del mando, y la
    // entrada en Manual- manda UN GO_RED y deja la maquina en reposo con nadie en verde. Si
    // esa trama se pierde, (a) el rojo no llega al otro lado mientras los PING le mantengan
    // vivo el verde, y (b) la siguiente peticion de cambio abre el verde propio contando con
    // un despeje "ya pagado" que no vacio nada (el case QV_NINGUNO de pedirCambio, N-147).
    //
    // EL BORDE DE (a), escrito al lado (CLAUDE.md 7): una trama perdida solo se descubre
    // porque no vuelve su acuse, y en reposo el unico reintento que tiene el Maestro es el
    // LATIDO. Lo mas tarde que sale es un latido entero despues del anterior -mas un tick,
    // por el '>' estricto del firmware-; luego cruza un viaje y se ejecuta en el tick en que
    // llega. Se barre la fase del latido como en G1: un solo instante mediria la que tocara.
    const unsigned long BORDE_REINTENTO_MS = LATIDO_MS_V + PASO_MS + BORDE_MS;
    unsigned long g6SueltaMax = 0, g6Fases = 0, g6PeorOff = 0;
    bool g6SueltaEjercido = true;
    for (unsigned long off = 0; off <= LATIDO_MS_V; off += 250) {
      CorridaG c;
      g6Fases++;
      escenarioLimpio(TIEMPOS_G, true);
      bool ok = alcanzarVerdeG(ESCLAVO, ALCANCE);
      avanzar(off);
      ok = ok && ESCLAVO.verde();
      const unsigned long perd0 = g_goRojoPerdidos;
      g_goRojoAPerder = 1;
      MAESTRO.orden("forzar_rojo_total");
      c.tCorte = g_t;
      correrG(c, LATIDO_MS_V + 2000);
      if (!(ok && g_goRojoPerdidos - perd0 == 1)) g6SueltaEjercido = false;
      // -1 = no salio de verde en toda la corrida: se cuenta como la corrida entera.
      const unsigned long suelta = (c.tSueltaE < 0) ? (LATIDO_MS_V + 2000)
                                   : (unsigned long)(c.tSueltaE - (long)c.tCorte);
      if (suelta >= g6SueltaMax) { g6SueltaMax = suelta; g6PeorOff = off; }
    }
    CorridaG g6;
    bool g6Ejercido;
    {
      escenarioLimpio(TIEMPOS_G, true);
      bool ok = alcanzarVerdeG(ESCLAVO, ALCANCE);
      avanzar(3000);
      ok = ok && ESCLAVO.verde();
      const unsigned long perd0 = g_goRojoPerdidos;
      g_goRojoAPerder = 1;
      MAESTRO.orden("forzar_rojo_total");
      g6.tCorte = g_t;
      correrG(g6, ROJO_MS + TRAS_VERDE);
      finG(g6);
      g6Ejercido = ok && g_goRojoPerdidos - perd0 == 1;
    }
    imprimirTrazaG("G6 (ROJO TOTAL con el Esclavo en verde, su GO_RED perdido; el Automatico "
                   "sigue y pide el cambio al vencer su rojo):", g6);
    comprobar(g6SueltaEjercido && g6Ejercido,
              "G6 (control): en las " + std::to_string(g6Fases + 1) + " corridas se dio el ROJO "
              "TOTAL con el Esclavo en VERDE y se perdio EXACTAMENTE su GO_RED");
    comprobar(g6SueltaMax <= BORDE_REINTENTO_MS,
              "G6-a: el ROJO TOTAL llega al otro lado aunque se pierda su trama: el Esclavo sale "
              "de verde en " + std::to_string(g6SueltaMax) + " ms en la peor fase (" +
              std::to_string(g6PeorOff) + " ms); borde " + std::to_string(BORDE_REINTENTO_MS) +
              " ms (un latido + un tick + un viaje + un tick)");
    comprobar(acumuladoMsG(g6.v) <= BORDE_MS && g6.v.simultaneo == 0 &&
              g6.tAperturaM >= 0 && todoRojoMsG(g6) >= (long)DESPEJE_MS,
              "G6-b: la peticion de cambio que sigue al ROJO TOTAL abre el verde propio con el "
              "Esclavo en rojo y el todo-rojo entero: ventana " +
              std::to_string(acumuladoMsG(g6.v)) + " ms, " +
              std::to_string(g6.v.simultaneo * PASO_MS) + " ms con VERDE EN LAS DOS (borde " +
              std::to_string(BORDE_MS) + " ms); todo-rojo " + std::to_string(todoRojoMsG(g6)) +
              " ms, despeje " + std::to_string(DESPEJE_MS) + " ms (-1 = no se abrio)");

    // ---- G7: DAR PASO con el despeje "ya pagado" y el rojo del otro lado sin constar ----
    // El case QV_NINGUNO de pedirCambio() abre en el acto si el cruce lleva en rojo mas que
    // el despeje (N-147: el despeje cumplido no se vuelve a cobrar). Aqui el ROJO TOTAL se da
    // con la direccion Maestro->Esclavo cortada un despeje y dos segundos -se lleva su GO_RED
    // y lo que el Maestro mande detras-, vuelve, y el operario pulsa DAR PASO. Lo que se
    // exige: que el verde propio espere al ACK_RED y cuente el despeje ENTERO desde el,
    // porque no se sabe desde cuando esta en rojo el otro lado.
    CorridaG g7;
    bool g7Ejercido;
    long g7Acepto = -1;
    {
      escenarioLimpio(TIEMPOS_G, true);
      bool ok = alcanzarVerdeG(ESCLAVO, ALCANCE);
      avanzar(3000);
      ok = ok && ESCLAVO.verde();
      const unsigned long cort0 = g_goRojoCortados;
      MAESTRO.orden("forzar_rojo_total");
      g_enlaceHaciaEsclavo = false;
      g7.tCorte = g_t;
      correrG(g7, DESPEJE_MS + 2000);
      g_enlaceHaciaEsclavo = true;
      ok = ok && ESCLAVO.verde() && !g7.maestroFallo && g_goRojoCortados - cort0 >= 1;
      g7Acepto = MAESTRO.orden("pedir_cambio");
      correrG(g7, TRAS_VERDE);
      finG(g7);
      g7Ejercido = ok && g7Acepto == 1;
    }
    imprimirTrazaG("G7 (ROJO TOTAL con Maestro->Esclavo cortado un despeje + 2 s, y DAR PASO al "
                   "volver):", g7);
    comprobar(g7Ejercido,
              "G7 (control): al pulsar DAR PASO el Esclavo seguia en VERDE sin haber recibido el "
              "ROJO TOTAL, el Maestro no estaba en C_FALLO y la orden se ACEPTO (" +
              std::to_string(g7Acepto) + ")");
    comprobar(acumuladoMsG(g7.v) <= BORDE_MS && g7.v.simultaneo == 0 &&
              g7.tAperturaM >= 0 && todoRojoMsG(g7) >= (long)DESPEJE_MS,
              "G7: DAR PASO sin el rojo del otro lado confirmado espera su ACK_RED y cobra el "
              "despeje entero: ventana " + std::to_string(acumuladoMsG(g7.v)) + " ms, " +
              std::to_string(g7.v.simultaneo * PASO_MS) + " ms con VERDE EN LAS DOS (borde " +
              std::to_string(BORDE_MS) + " ms); todo-rojo " + std::to_string(todoRojoMsG(g7)) +
              " ms, despeje " + std::to_string(DESPEJE_MS) + " ms (-1 = no se abrio)");

    // ---- G8: el Esclavo en su AMBAR DE EMERGENCIA, que VETA el GO_RED sin acusarlo -----
    // Esa punta, con el cerrojo de la app puesto, ni obedece ni acusa un GO_RED -N-83/D-8,
    // Esclavo/src/main.cpp- pero SIGUE CONTESTANDO PONG a un PING. Si el Maestro esperase su
    // ACK_RED volviendo a ver comunicacion por los PONG, la autorrecuperacion se rearmaria
    // sin fin: rojo 17,5 s, C_FALLO, rojo otra vez... con la pluma subiendo y bajando. Aqui
    // se monta con el aviso CMD_AMBAR_ESCLAVO PERDIDO (N-142 lo manda una vez y sin
    // reintento; con el aviso, main.cpp lleva el cruce a MODO_AMBAR y esto no pasa), y en
    // las dos fases: (a) con el Esclavo en verde y (b) con el Maestro en verde.
    //
    // EL BORDE DE LA OSCILACION, escrito al lado: UNA entrada en S_FALLO en toda la corrida.
    // Es lo que el propio Esclavo declara que tiene que pasar -"cae a C_FALLO ... y el cruce
    // entero termina en ambar, que es lo que el operario pidio"- y es lo unico que no deja
    // una pluma subiendo y bajando delante de quien pidio el ambar.
    const unsigned long G8_MS = 12UL * 60000UL;
    CorridaG g8[2];
    long g8Alarmas[2] = {0, 0};
    bool g8Ejercido = true;
    for (int k = 0; k < 2; k++) {
      CorridaG& c = g8[k];
      escenarioLimpio(TIEMPOS_G, true);
      bool ok = alcanzarVerdeG(k == 0 ? ESCLAVO : MAESTRO, ALCANCE);
      avanzar(3000);
      ok = ok && (k == 0 ? ESCLAVO.verde() : MAESTRO.verde());
      const long alarmas0 = MAESTRO.orden("alarmas");
      g_enlaceHaciaMaestro = false;          // se lleva el aviso de este instante, y solo el
      // 11/09 (N-142): la linea de la app en el despachador REAL, no la transcripcion que
      // este arnes tenia. El aviso sale DENTRO del pasoG() de abajo y es el que se pierde.
      ok = ok && tecleaAmbarApp() == 1;
      c.tCorte = g_t;
      pasoG(c);
      ok = ok && ESCLAVO.orden("ambar_latch") == 1;
      g_enlaceHaciaMaestro = true;
      correrG(c, G8_MS);
      finG(c);
      ok = ok && ESCLAVO.estado() == S_FALLO_V;
      g8Alarmas[k] = MAESTRO.orden("alarmas") - alarmas0;
      if (!ok) g8Ejercido = false;
    }
    imprimirTrazaG("G8-a (ambar de emergencia del Esclavo, con el Esclavo en verde y su aviso "
                   "perdido):", g8[0]);
    imprimirTrazaG("G8-b (ambar de emergencia del Esclavo, con el MAESTRO en verde y el aviso "
                   "perdido):", g8[1]);
    comprobar(g8Ejercido,
              "G8 (control): en a y b el ambar de emergencia del Esclavo se puso sobre la fase "
              "pedida, su aviso CMD_AMBAR_ESCLAVO se perdio y el Esclavo seguia en S_FALLO al "
              "acabar");
    comprobar(g8[0].entradasFalloM <= 1 && g8[1].entradasFalloM <= 1 &&
              g8[0].mEnFallo && g8[1].mEnFallo,
              "G8: con el Esclavo vetando el GO_RED el Maestro NO oscila: entra en S_FALLO " +
              std::to_string(g8[0].entradasFalloM) + " vez/veces (a) y " +
              std::to_string(g8[1].entradasFalloM) + " (b) en " + std::to_string(G8_MS / 60000) +
              " min y se queda ahi (borde: 1); alarmas " + std::to_string(g8Alarmas[0]) + " (a) y " +
              std::to_string(g8Alarmas[1]) + " (b)");
    comprobar(acumuladoMsG(g8[0].v) <= BORDE_MS,
              "G8-a: con el Esclavo en su ambar de emergencia el Maestro NUNCA se abre frente a "
              "el: ventana " + std::to_string(acumuladoMsG(g8[0].v)) + " ms; borde " +
              std::to_string(BORDE_MS) + " ms");
    // (b) empieza con el Maestro YA en verde: lo que dure hasta el final de su fase es el
    // residual de N-142 con el aviso perdido -"hasta 3 minutos"- y ningun cambio de ESTE
    // fichero lo acorta, porque el Maestro no tiene como saber que el otro lado cambio.
    // Por eso va a reportar() y no cuenta; lo que SI cuenta es que no se abra otra vez.
    comprobar(g8[1].v.rachas <= 1,
              "G8-b: pasado el verde que ya tenia, el Maestro no se vuelve a abrir frente al "
              "ambar de emergencia: " + std::to_string(g8[1].v.rachas) + " racha(s) de ventana "
              "(borde 1, la del verde que ya estaba encendido)");
    std::printf("   [NOTA]  G8-b: el verde del Maestro que YA estaba encendido siguio %lu ms frente "
                "al ambar de emergencia del Esclavo, con el aviso CMD_AMBAR_ESCLAVO perdido. Es el "
                "residual de N-142 (aviso sin reintento): no lo cierra coordinador.cpp. No cuenta.\n",
                acumuladoMsG(g8[1].v));

    // G8-c: el rojo del otro lado CONSTABA -el Maestro estaba en verde, o sea con el Esclavo
    // acusado en rojo- y el Esclavo se fue a su ambar de emergencia POR SU CUENTA, con el
    // aviso perdido. Detras, alguien da ROJO TOTAL y, pasado el despeje, DAR PASO. Un rojo
    // acusado ANTES de la orden de rojo no dice nada del de ahora: si el Maestro contara con
    // el, abriria frente al ambar. Aqui el control es que no se abra nunca -el otro lado no
    // va a acusar- y que termine en S_FALLO como el, no parado en rojo.
    CorridaG g8c;
    bool g8cEjercido;
    long g8cAcepto = -1;
    {
      escenarioLimpio(TIEMPOS_G, true);
      bool ok = alcanzarVerdeG(MAESTRO, ALCANCE);
      avanzar(3000);
      g_enlaceHaciaMaestro = false;
      ok = ok && MAESTRO.verde() && tecleaAmbarApp() == 1;
      unTick();                               // el despachador real atiende la linea aqui
      ok = ok && ESCLAVO.orden("ambar_latch") == 1;
      g_enlaceHaciaMaestro = true;
      avanzar(1000);
      MAESTRO.orden("forzar_rojo_total");
      g8c.tCorte = g_t;
      correrG(g8c, DESPEJE_MS + 2000);
      g8cAcepto = MAESTRO.orden("pedir_cambio");
      correrG(g8c, POST);
      finG(g8c);
      g8cEjercido = ok && ESCLAVO.estado() == S_FALLO_V;
    }
    imprimirTrazaG("G8-c (el rojo constaba; el Esclavo se va a su ambar de emergencia con el aviso "
                   "perdido; ROJO TOTAL y DAR PASO pasado el despeje):", g8c);
    comprobar(g8cEjercido,
              "G8-c (control): el Maestro estaba en verde -rojo del Esclavo acusado-, el Esclavo "
              "se fue a su ambar de emergencia con el aviso perdido y seguia en el al acabar; "
              "DAR PASO devolvio " + std::to_string(g8cAcepto));
    comprobar(acumuladoMsG(g8c.v) <= BORDE_MS && g8c.tAperturaM < 0 && g8c.mEnFallo,
              "G8-c: un rojo acusado ANTES del ROJO TOTAL no abre el DAR PASO de despues: el "
              "Maestro no se abre frente al ambar de emergencia (ventana " +
              std::to_string(acumuladoMsG(g8c.v)) + " ms, borde " + std::to_string(BORDE_MS) +
              " ms; apertura " + (g8c.tAperturaM < 0 ? std::string("ninguna")
                                                     : std::to_string(g8c.tAperturaM - (long)g8c.tCorte) + " ms") +
              ") y termina en S_FALLO como el otro lado");

    // ---- G9: un ACK_RED VIEJO (revision del arquitecto, punto 4) -----------------------
    // El protocolo no dice en el ACK_RED a que GO_RED contesta, asi que el Maestro solo puede
    // exigir que llegue DESPUES de su orden. Engana si el acuse es de ANTES de que el Esclavo
    // volviera a verde. Se monta el peor caso: se retiene un ACK_RED en el aire, el ciclo
    // sigue hasta que el Esclavo esta otra vez en verde, se entra en Automatico perdiendo el
    // GO_RED de entrada, y se suelta el acuse viejo en ese instante.
    //
    // reportar(): no cuenta, y es deliberado. Ningun firmware DEL MAESTRO puede distinguir
    // ese acuse del bueno; cerrarlo pide un identificador en el ACK_RED, o sea protocolo y
    // las DOS puntas. Lo que se publica es CUANTO tiene que retener la radio una trama para
    // que pase: el Maestro no emite GO_GREEN hasta despeje + ambar + verde + despeje despues
    // del ultimo acuse de rojo.
    //
    // Y SUS INSTANTES SE SACAN DEL RESUMEN FINAL, que cuenta verdes simultaneos de todo el
    // barrido: dejarlos dentro lo convertiria en un FALLA permanente que ningun firmware del
    // Maestro puede apagar (CLAUDE.md 1). Se imprimen aparte, con su numero.
    CorridaG g9;
    bool g9Ejercido = false;
    unsigned long g9Edad = 0;
    const unsigned long simAntesG9 = g_verdeSimultaneo;
    const unsigned long primerAntesG9 = g_primerSimultaneoMs;
    {
      escenarioLimpio(TIEMPOS_G, true);
      bool ok = alcanzarVerdeG(ESCLAVO, ALCANCE);
      g_ackRojoARetener = 1;
      bool rojo = false;
      for (unsigned long g = 0; g < ALCANCE && !rojo; g += PASO_MS) { unTick(); rojo = !ESCLAVO.verde(); }
      ok = ok && rojo && alcanzarVerdeG(ESCLAVO, ALCANCE) && g_hayAckRojoRetenido;
      avanzar(3000);
      ok = ok && ESCLAVO.verde();
      g9Edad = g_t - g_tAckRojoRetenido;
      g_goRojoAPerder = 1;
      MAESTRO.orden("arrancar_automatico");
      soltarAckRojo();
      g9.tCorte = g_t;
      correrG(g9, TRAS_VERDE);
      finG(g9);
      g9Ejercido = ok && g_goRojoAPerder == 0;
    }
    const unsigned long simG9 = g_verdeSimultaneo - simAntesG9;
    g_verdeSimultaneo = simAntesG9;
    g_primerSimultaneoMs = primerAntesG9;
    std::printf("      (G9: %lu instantes con verde en las dos, FUERA del RESUMEN final)\n", simG9);
    imprimirTrazaG("G9 (ACK_RED retenido desde ANTES del ultimo verde del Esclavo, soltado tras "
                   "un GO_RED de entrada perdido):", g9);
    std::printf("   [NOTA]  G9%s: un ACK_RED retenido %lu ms en el aire -desde antes del ultimo "
                "GO_GREEN- y soltado tras un GO_RED perdido: ventana %lu ms, %lu ms con VERDE EN "
                "LAS DOS. El Maestro no puede distinguirlo sin un identificador en el acuse "
                "(protocolo, dos puntas). Hace falta que la radio RETENGA una trama al menos "
                "despeje + ambar + verde + despeje (%lu ms con este ciclo) y la suelte en ese "
                "instante. No cuenta.\n",
                g9Ejercido ? "" : " (NO EJERCIDO: el escenario no llego a montarse)",
                g9Edad, acumuladoMsG(g9.v), g9.v.simultaneo * PASO_MS,
                2 * DESPEJE_MS + AMBAR_ESCLAVO_MS_V + VERDE_MS);

    // ---- G10: el Esclavo en Degradado, en verde por reloj, y el Maestro entra en Automatico
    // (revision del arquitecto, punto 5). El GO_RED de entrada lo saca del Degradado
    // -degradado_salir(), todo-rojo- y lo acusa; el Maestro cuenta SU despeje desde ese
    // ACK_RED. El Esclavo se configura con un despeje MAS LARGO que el del Maestro para ver
    // cual de los dos manda. Lo que cuenta es que haya al menos el del Maestro -es lo que
    // este fichero garantiza-; si deberia mandar el mayor de los dos es decision vial, y
    // va a reportar().
    CorridaG g10;
    bool g10Ejercido = false;
    long g10DespE = -1;
    {
      escenarioLimpio(TIEMPOS_G, true);
      sincronizarEsclavo(10, 8, 0, 0);
      configurarEsclavo(20, (uint8_t)(3 * DESPEJE_POR_DEFECTO_S));
      g10DespE = ESCLAVO.orden("config_despeje");
      g_enlaceHaciaEsclavo = g_enlaceHaciaMaestro = false;
      bool ok = ESCLAVO.orden("degradado_entrar") == 0;
      ok = ok && alcanzarVerdeG(ESCLAVO, ALCANCE) && ESCLAVO.orden("degradado_gobierna") == 1;
      avanzar(1000);
      ok = ok && ESCLAVO.verde();
      g_enlaceHaciaEsclavo = g_enlaceHaciaMaestro = true;
      MAESTRO.orden("arrancar_automatico");
      g10.tCorte = g_t;
      correrG(g10, TRAS_VERDE);
      finG(g10);
      g10Ejercido = ok && ESCLAVO.orden("degradado_gobierna") == 0;
    }
    imprimirTrazaG("G10 (Esclavo en Degradado y en verde por reloj; el Maestro entra en "
                   "Automatico):", g10);
    comprobar(g10Ejercido && g10DespE == (long)(3 * DESPEJE_POR_DEFECTO_S),
              "G10 (control): el Esclavo daba verde POR RELOJ en su Degradado, con un despeje de " +
              std::to_string(g10DespE) + " s configurado, y el GO_RED de entrada del Maestro lo "
              "saco del modo");
    comprobar(acumuladoMsG(g10.v) <= BORDE_MS && g10.tAperturaM >= 0 &&
              todoRojoMsG(g10) >= (long)DESPEJE_MS,
              "G10: al sacar al Esclavo de su Degradado el Maestro espera su ACK_RED y deja al "
              "menos SU despeje: ventana " + std::to_string(acumuladoMsG(g10.v)) + " ms; todo-rojo " +
              std::to_string(todoRojoMsG(g10)) + " ms, despeje del Maestro " +
              std::to_string(DESPEJE_MS) + " ms (-1 = no se abrio)");
    std::printf("   [NOTA]  G10: manda el despeje del MAESTRO: todo-rojo %ld ms contra %ld ms del "
                "despeje configurado en el Degradado del Esclavo. Si debe mandar el mayor de los "
                "dos es decision vial. No cuenta.\n",
                todoRojoMsG(g10), g10DespE * 1000L);

    // ---- G11: MICROCORTES QUE SE RECUPERAN. El COSTE del margen de G3 -----------------
    //
    // Este bloque NO mide la ventana de G3: mide lo que cerrarla podria costar. Soltar el
    // verde propio un margen ANTES del silencio solo vale si no compra el defecto
    // contrario -"se va a degradado cada nada cuando llueve", reporte de campo del 27/08-,
    // y esa propiedad no la medio nunca nadie: entradasFalloM existia desde G8 y del ESCLAVO no
    // habia cuenta ninguna, que es justo la punta cuyo ambar decide.
    //
    // EL BORDE, ESCRITO AL LADO Y DERIVADO, NO ELEGIDO (CLAUDE.md 7): por cada corte se
    // deriva si TOCABA ambar comparando el silencio que cada punta midio de verdad
    // -g_tGobiernoEsclavo para el Esclavo, g_tRxMaestro para el Maestro, que son los dos
    // instantes DISTINTOS que G3 mide- contra SFTY6_SILENCIO_MS, que es el unico umbral de
    // los dos. La cuenta esperada sale del canal; escribirla a mano aqui seria aprobar el
    // firmware con el numero que ese mismo firmware produjo.
    //
    // POR QUE LOS CORTES SON ESTOS: se barre por debajo y por encima de donde el margen
    // muerde. 18500 ms es el ultimo corte que NO puede dejar huerfano a nadie
    // -SFTY6_SILENCIO_MS menos el latido menos un TIMEOUT_ACK_MS: peor fase del latido mas
    // el viaje-, 21500 ms es el instante EXACTO en que el margen suelta el verde, y
    // 24950 ms es el ultimo milisegundo util del umbral. Si el margen estuviera mal
    // derivado, asomaria entre 18500 y 24950.
    //
    // Y EN LAS DOS DIRECCIONES: corte TOTAL (la lluvia) y corte solo Maestro->Esclavo (la
    // averia de G3). Cortar una sola direccion no es lo mismo: el Maestro sigue emitiendo.
    {
      const unsigned long CORTES_G11[] = {
        3000, 8000, 15000, SIL - LATIDO_MS_V - TOUT, SIL - TOUT, SIL - TOUT + 1000,
        SIL - 2000, SIL - PASO_MS
      };
      const unsigned nCortes = (unsigned)(sizeof(CORTES_G11) / sizeof(CORTES_G11[0]));
      // LA RECUPERACION NO ES UN PLAZO FIJO: SE ESPERA A QUE EL MAESTRO VUELVA A DAR
      // VERDE. Lo aprendio el instrumento (CLAUDE.md 7): con un plazo fijo -despeje +
      // ambar + dos reintentos + holgura- el segundo corte de los pares mas largos caia
      // con el cruce todavia rehaciendose, o sea que el barrido NO estaba midiendo lo que
      // dice medir -cortes CON EL MAESTRO EN VERDE-. El tope es el peor camino completo:
      // el silencio entero, el presupuesto de reintentos, el despeje y el ambar.
      const unsigned long RECUP_MS = SIL + TOUT * (NMAX + 2) + DESPEJE_MS +
                                     AMBAR_ESCLAVO_MS_V + 30000;
      // Dos cortes por escenario: 2 x (corte + recuperacion) cabe dentro del verde minimo
      // del Maestro (VERDE_MIN_MIN), asi que el barrido ejerce lo que dice ejercer -cortes
      // CON EL MAESTRO EN VERDE- y no se le cuela un relevo de ciclo en medio.
      const int REPS_G11 = 2;
      unsigned long g11Cortes = 0, g11EspE = 0, g11EspM = 0, g11AmbE = 0, g11AmbM = 0;
      unsigned long g11MaxSilE = 0, g11MaxSilM = 0, g11VueltasAVerde = 0;
      unsigned long g11MaxHueco = 0;
      unsigned long g11ConMaestroVerde = 0;
      bool g11Ejercido = true;
      CorridaG g11Peor;
      unsigned long g11PeorD = 0;
      for (int dir = 0; dir < 2; dir++) {
        for (unsigned n = 0; n < nCortes; n++) {
          const unsigned long D = CORTES_G11[n];
          CorridaG c;
          escenarioLimpio(TIEMPOS_G, true);
          if (!alcanzarVerdeG(MAESTRO, ALCANCE)) g11Ejercido = false;
          c.tCorte = g_t;
          for (int k = 0; k < REPS_G11; k++) {
            g11Cortes++;
            if (MAESTRO.verde()) g11ConMaestroVerde++;
            bool orfE = false, orfM = false;
            g_enlaceHaciaEsclavo = false;
            if (dir == 0) g_enlaceHaciaMaestro = false;
            for (unsigned long h = 0; h < D + RECUP_MS; h += PASO_MS) {
              if (h >= D) g_enlaceHaciaEsclavo = g_enlaceHaciaMaestro = true;
              pasoG(c);
              const unsigned long silE = g_t - g_tGobiernoEsclavo;
              const unsigned long silM = g_t - g_tRxMaestro;
              if (silE > SIL) orfE = true;
              if (silM > SIL) orfM = true;
              if (silE > g11MaxSilE) g11MaxSilE = silE;
              if (silM > g11MaxSilM) g11MaxSilM = silM;
              const unsigned long hueco = g_t - g_tEmisionMaestro;
              if (hueco > g11MaxHueco) g11MaxHueco = hueco;
              // Se corta la recuperacion cuando el cruce ha vuelto a su verde Y EL
              // ENLACE ESTA VERIFICABLEMENTE VIVO -las dos puntas oyendose por debajo de
              // un latido mas un timeout-. Las dos mitades hacen falta, y la segunda la
              // enseno el instrumento (CLAUDE.md 7): con solo "el Maestro esta en verde",
              // los cortes que NO llegan a apagar el verde salian con una recuperacion de
              // CERO ticks -el corte siguiente empezaba en el mismo instante en que
              // acababa el anterior-, asi que los silencios se encadenaban y el barrido
              // media cortes de 45 s donde decia medir dos de 22,5 s.
              if (h >= D && MAESTRO.verde() &&
                  g_t - g_tGobiernoEsclavo <= LATIDO_MS_V + TOUT &&
                  g_t - g_tRxMaestro <= LATIDO_MS_V + TOUT) {
                g11VueltasAVerde++;
                break;
              }
            }
            if (orfE) g11EspE++;
            if (orfM) g11EspM++;
          }
          finG(c);
          g11AmbE += c.entradasFalloE;
          g11AmbM += c.entradasFalloM;
          if (D >= g11PeorD) { g11PeorD = D; g11Peor = c; }
        }
      }
      imprimirTrazaG(("G11, el corte mas largo (" + std::to_string(g11PeorD) + " ms, "
                      "repetido " + std::to_string(REPS_G11) + " veces sobre el mismo "
                      "escenario):").c_str(), g11Peor);
      std::printf("      G11: %lu cortes de %lu a %lu ms, en las dos direcciones. Silencio "
                  "maximo medido: Esclavo %lu ms, Maestro %lu ms (umbral %lu ms). Entradas en "
                  "ambar: Esclavo %lu (derivadas %lu), Maestro %lu (derivadas %lu).\n",
                  g11Cortes, CORTES_G11[0], CORTES_G11[nCortes - 1], g11MaxSilE, g11MaxSilM,
                  SIL, g11AmbE, g11EspE, g11AmbM, g11EspM);
      // DE QUE DEPENDE LA DERIVACION, Y POR ESO SE MIDE AQUI: el reloj de silencio del
      // Esclavo se modela contando SOLO PING, GO_RED y GO_GREEN, que son los tres sitios
      // donde main.cpp REFRESCA tUltimoComando. El dia que aparezca un cuarto, este modelo
      // se quedaria fechando el silencio de otro firmware y la cuenta derivada de abajo
      // aprobaria un ambar de mas sin decir nada. Se recuenta sobre el fuente sin
      // comentarios: este repositorio cita sus propios simbolos en los comentarios.
      //
      // SE ESPERAN CUATRO, NO TRES, Y ESO LO CORRIGIO EL INSTRUMENTO (CLAUDE.md 7): el
      // cuarto es la DECLARACION -"static unsigned long tUltimoComando = millis()"-, que
      // casa el mismo patron y no es un refresco. Se cuenta el total en vez de filtrarla
      // porque std::regex no tiene lookbehind y un patron mas fino se rompe con el primer
      // salto de linea que alguien meta; lo que importa es que la cuenta NO SE MUEVA.
      unsigned long refrescosTUC = 0;
      {
        const std::string mainE =
            sinComentarios(leerFuente(RAIZ + "/Esclavo/src/main.cpp"));
        const std::regex reTUC(R"(tUltimoComando\s*=\s*millis\(\))");
        for (std::sregex_iterator it(mainE.begin(), mainE.end(), reTUC), fin; it != fin; ++it)
          refrescosTUC++;
      }
      comprobar(g11Ejercido && g11ConMaestroVerde == g11Cortes && refrescosTUC == 4 &&
                g11MaxSilE > SIL - TOUT && g11MaxSilE < SIL + LATIDO_MS_V + TOUT,
                "G11 (control): los " + std::to_string(g11Cortes) + " cortes cayeron TODOS con "
                "el Maestro en VERDE, el silencio del Esclavo llego a " +
                std::to_string(g11MaxSilE) + " ms -pasado el punto donde el margen muerde (" +
                std::to_string(SIL - TOUT) + " ms) y sin desbordar la fase del latido- y "
                "main.cpp del Esclavo sigue refrescando tUltimoComando en " +
                std::to_string(refrescosTUC) + " sitios (su declaracion + PING, GO_RED y "
                "el GO_GREEN que arranca la transicion -D-34: la repeticion ya no-), que es lo "
                "unico sobre lo que este modelo fecha el silencio. Trafico de servicio "
                "entregado y correctamente NO contado: " +
                std::to_string(g_entregasEsclavoNoGobierno) + " tramas");
      // 🔴 LA LINEA QUE DECIDE SI EL CAMBIO ENTRA (criterio del responsable, 12/09): ni un
      // ambar de mas en cortes que se recuperan. Se compara contra lo DERIVADO del canal,
      // no contra un numero escrito: si el firmware se rindiera antes de tiempo -por el
      // margen o por cualquier otra cosa- la cuenta medida subiria y la derivada no.
      comprobar(g11AmbE == g11EspE && g11AmbM == g11EspM,
                "G11: en " + std::to_string(g11Cortes) + " microcortes que se RECUPERAN, cada "
                "punta entra en ambar EXACTAMENTE las veces que su propio silencio paso de " +
                std::to_string(SIL) + " ms: Esclavo " + std::to_string(g11AmbE) + " contra " +
                std::to_string(g11EspE) + " derivadas, Maestro " + std::to_string(g11AmbM) +
                " contra " + std::to_string(g11EspM) + ". Soltar el verde antes que el silencio "
                "no adelanta el ambar de nadie");
      // 🔴 Y EL CONTROL QUE LE FALTA A LA LINEA DE ARRIBA, porque la cuenta derivada SIGUE
      // AL CANAL: si el firmware hablara menos, subirian a la vez la medida y la derivada
      // y las dos casarian igual de bien. Lo que no puede cambiar es CADA CUANTO habla
      // esta punta con el aire cortado, porque de eso -y solo de eso- depende cuando se
      // entera el otro poste al volver el enlace.
      //
      // EL BORDE, ESCRITO AL LADO Y DERIVADO (CLAUDE.md 7): LATIDO_MS, mas un tick de
      // observacion. Es la cadencia con la que el Maestro habla en un modo activo -PING, o
      // GO_RED cuando el rojo no consta- y el primer sumando del presupuesto de radio de
      // N-71. Cualquier camino nuevo que deje a esta punta callada mas de un latido
      // ADELANTA el ambar del otro poste, aunque el umbral de 25 s no se haya tocado.
      comprobar(g11MaxHueco <= LATIDO_MS_V + PASO_MS,
                "G11 (el control de la cuenta de arriba): con el aire cortado el Maestro no se "
                "calla mas de un latido. Hueco maximo entre emisiones suyas: " +
                std::to_string(g11MaxHueco) + " ms; borde " +
                std::to_string(LATIDO_MS_V + PASO_MS) + " ms (LATIDO_MS + un tick). Un camino "
                "que la callara mas -una espera de acuse, que suprime el latido por SFTY-13 y "
                "reintenta cada " + std::to_string(TOUT) + " ms- adelantaria el ambar del otro "
                "poste sin tocar el umbral, y la cuenta derivada de arriba subiria con el");
      // El control de la inversion (CLAUDE.md 9): un Maestro que se quedara en rojo para
      // siempre despues del primer corte pasaria la linea de arriba igual de bien.
      comprobar(g11VueltasAVerde == g11Cortes,
                "G11 (control de la inversion): tras CADA UNO de los " +
                std::to_string(g11Cortes) + " cortes el Maestro VUELVE a dar verde (" +
                std::to_string(g11VueltasAVerde) + " vueltas), asi que la cuenta de ambares de "
                "arriba no sale de un cruce que se quedo parado en rojo -que la pasaria igual "
                "de bien-");
    }

    // ---- G12: GO_GREEN ENTREGADO, SU ACUSE PERDIDO, Y LA BAJADA MUERE DESPUES (1.39, E2b) --
    //
    // La medida del 15/09 ("bloque H" en la copia del scratchpad; aqui G12 porque el bloque H
    // de este arnes ya es el del ambar del Poste 2 y sus H1..H6 estan citados fuera). La
    // subida Esclavo->Maestro muere 'pre' ms ANTES de que el Maestro emita el GO_GREEN
    // (0 = en el instante de emitirlo), y la bajada vive hasta que el Esclavo ha recibido
    // k repeticiones de ese GO_GREEN -o hasta que el Maestro cae a S_FALLO, lo que llegue
    // antes-. Cada punta cuenta su silencio desde un instante DISTINTO: el Maestro desde su
    // ultima recepcion -anterior al corte de la subida-, el Esclavo desde su ultima orden
    // -la ultima repeticion entregada-. Si el reloj del Esclavo se refresca despues que el
    // del Maestro, el Maestro cae a ambar con la pluma arriba y el Esclavo sigue en verde
    // hasta su propia orfandad. Ningun escenario de G1..G11 corta las dos direcciones en
    // ese orden: G2-a deja viva la bajada, G2-d la devuelve, G4-b corta las dos a la vez.
    //
    // POR QUE ESTE BARRIDO (CLAUDE.md 7), derivado del C++:
    //   pre en multiplos del LATIDO_MS: el Maestro fecha su silencio sobre el PONG de cada
    //     latido, asi que dentro de un latido la fase cambia poco y entre latidos cambia el
    //     instante del ultimo PONG oido. Hasta 6 latidos, y se ABORTA si el ultimo corte mas
    //     un latido llega al silencio SFTY-6: ahi el Maestro ya estaria en C_FALLO antes de
    //     poder emitir, y la fila no mediria lo que dice.
    //   k de 0 a CICLO_MAX_REINTENTOS - 1: las repeticiones que el Maestro puede llegar a
    //     entregar antes de agotar reintentos. k = CICLO_MAX_REINTENTOS no hace falta como
    //     columna: la rama "la bajada vive hasta el S_FALLO del Maestro" esta en todas las
    //     celdas y la ejercen las de pre alto.
    // EL BORDE DE G12a ES CERO, NO BORDE_MS, y por eso se escribe: con las DOS direcciones
    // muertas no hay viaje de radio que esperar -cada punta decide sobre su propio reloj-,
    // asi que nada obliga a que el ambar de una se solape con el verde de la otra. Las celdas
    // que salen con 50 ms en HEAD (la bajada cortada en el instante del S_FALLO, con el
    // GO_RED ya en el aire) tambien cuentan: la trama salio porque la radio aun vivia, y el
    // Maestro encendio el ambar sin esperar a que llegara.
    //
    // LO QUE SE CAMBIO DEL BLOQUE LITERAL DEL SCRATCHPAD, y por que: (1) la ventana se
    // observa desde el corte de la subida, no desde el GO_GREEN, para que un firmware que
    // abra la ventana ANTES de emitir -o que no emita- no quede sin mirar; (2) se retiro la
    // reconstruccion por texto de la traza (umbral de 4000 ms escrito a mano), que en las
    // celdas sin GO_GREEN daba 25000 ms de "cruce" con la ventana real a cero: se cuenta
    // con VentanaG, el mismo detector de G1..G11; (3) el horizonte de 120 s sale del C++.
    {
      unsigned long dtGo = 0;
      {
        escenarioLimpio(TIEMPOS_G, true);
        const unsigned long t0 = g_t;
        if (!alcanzarGoVerdeG(ALCANCE)) abortar("G12: con la radio sana no hubo GO_GREEN en el alcance");
        dtGo = g_t - t0;   // tick siguiente al de la emision
      }
      const unsigned N_PRES = 5;
      const unsigned long PRES[N_PRES] = { 0, LATIDO_MS_V, 2 * LATIDO_MS_V, 4 * LATIDO_MS_V,
                                           6 * LATIDO_MS_V };
      if (PRES[N_PRES - 1] + LATIDO_MS_V >= SIL)
        abortar("G12: el corte mas temprano (" + std::to_string(PRES[N_PRES - 1]) + " ms) mas un "
                "latido llega al silencio SFTY-6 (" + std::to_string(SIL) + " ms): el Maestro ya "
                "estaria en C_FALLO antes de emitir el GO_GREEN");
      if (PRES[N_PRES - 1] + PASO_MS >= dtGo)
        abortar("G12: el corte mas temprano caeria antes del primer tick del escenario");
      if (NMAX == 0) abortar("G12: CICLO_MAX_REINTENTOS leido en cero");
      // Horizonte tras el GO_GREEN: el mismo que G2-a. Cubre el peor cierre posible -el
      // Maestro cae a S_FALLO como tarde un silencio despues, y el Esclavo huerfano otro
      // silencio despues de su ultima orden-; se aborta si no lo cubre.
      const unsigned long HORIZONTE = TOUT * (NMAX + 1) + POST;
      if (HORIZONTE <= 2 * SIL + LATIDO_MS_V + BORDE_MS)
        abortar("G12: el horizonte no cubre dos silencios SFTY-6 mas un latido");

      unsigned long celdas = 0, frenteMaxMs = 0, frentePeorPre = 0, frentePeorK = 0;
      unsigned long celdasConCruce = 0, simultaneoMs = 0, emitidas = 0, porK = 0, porFallo = 0;
      unsigned long frenteInstantes = 0;
      bool precondicion = true, filaCeroEjerce = true;
      std::string malas;
      CorridaG peor;
      bool hayPeor = false;
      const unsigned long a9Antes = g_a9Perdonados;
      std::printf("   G12: GO_GREEN emitido a %lu ms del arranque del escenario con la radio sana.\n",
                  dtGo);
      std::printf("   G12  pre_ms  k  GO_GREEN  bajada_cortada_por  frente_a_S_FALLO_ms  ventana_ms\n");
      for (unsigned ip = 0; ip < N_PRES; ip++) {
        const unsigned long pre = PRES[ip];
        for (unsigned long k = 0; k < NMAX; k++) {
          celdas++;
          escenarioLimpio(TIEMPOS_G, true);
          const unsigned long t0 = g_t;
          const unsigned long n0 = g_goVerdeEmitidos;
          unsigned long entregados0 = 0;
          for (int i = 0; i < 4; i++) entregados0 += g_goVerdeEntregadoEn[i];
          CorridaG c;
          bool cortadoSub = false;
          while (g_goVerdeEmitidos == n0 && g_t - t0 < ALCANCE) {
            if (!cortadoSub && pre > 0 && g_t - t0 + pre + PASO_MS >= dtGo) {
              g_enlaceHaciaMaestro = false; cortadoSub = true; c.tCorte = g_t;
            }
            if (cortadoSub) pasoG(c); else unTick();
          }
          const bool emitido = (g_goVerdeEmitidos != n0);
          if (!cortadoSub && emitido) { cortadoSub = true; c.tCorte = g_t; }   // pre == 0
          g_enlaceHaciaMaestro = false;   // el ACK_GREEN no vuelve
          // la bajada vive hasta k repeticiones mas entregadas, o hasta el S_FALLO del Maestro
          bool bajadaCortada = false, cortePorK = false, eVerde = false;
          unsigned long tEmK = 0;
          for (unsigned long g = 0; g < HORIZONTE; g += PASO_MS) {
            if (!bajadaCortada) {
              if (g_goVerdeEmitidos >= n0 + 1 + k) {
                if (tEmK == 0) tEmK = g_t;
                if (g_t >= tEmK + 2 * PASO_MS) {
                  g_enlaceHaciaEsclavo = false; bajadaCortada = true; cortePorK = true;
                }
              }
              if (MAESTRO.estado() == S_FALLO_V && !bajadaCortada) {
                g_enlaceHaciaEsclavo = false; bajadaCortada = true;
              }
            }
            pasoG(c);
            if (ESCLAVO.verde()) eVerde = true;
          }
          finG(c);
          unsigned long entregados = 0;
          for (int i = 0; i < 4; i++) entregados += g_goVerdeEntregadoEn[i];
          entregados -= entregados0;

          const unsigned long frenteMs = c.v.frenteAFallo * PASO_MS;
          frenteInstantes += c.v.frenteAFallo;
          if (frenteMs > 0) celdasConCruce++;
          if (!hayPeor || frenteMs > frenteMaxMs) {
            frenteMaxMs = frenteMs; frentePeorPre = pre; frentePeorK = k; peor = c; hayPeor = true;
          }
          simultaneoMs += c.v.simultaneo * PASO_MS;
          if (emitido) emitidas++;
          if (cortePorK) porK++; else if (bajadaCortada) porFallo++;
          const bool okCelda = cortadoSub && bajadaCortada && (!emitido || entregados >= 1);
          if (!okCelda) {
            precondicion = false;
            malas += " (" + std::to_string(pre) + "," + std::to_string(k) + ")";
          }
          if (pre == 0 && !(emitido && entregados >= 1 && eVerde)) filaCeroEjerce = false;
          std::printf("   G12  %6lu  %lu  %-8s  %-18s  %19lu  %10lu\n", pre, k,
                      emitido ? "si" : "NO", cortePorK ? "k repeticiones"
                                              : (bajadaCortada ? "S_FALLO Maestro" : "NO CORTADA"),
                      frenteMs, acumuladoMsG(c.v));
        }
      }
      imprimirTrazaG(("G12, peor celda (pre " + std::to_string(frentePeorPre) + " ms, k " +
                      std::to_string(frentePeorK) + "):").c_str(), peor);
      comprobar(frenteMaxMs == 0,
                "G12a: GO_GREEN entregado, acuse perdido y radio muerta en las dos direcciones "
                "(subida cortada 0 a " + std::to_string(PRES[N_PRES - 1]) + " ms antes del "
                "GO_GREEN, bajada viva durante 0 a " + std::to_string(NMAX - 1) + " repeticiones): "
                "en las " + std::to_string(celdas) + " celdas ninguna punta da verde frente a la "
                "otra en S_FALLO (ambar, pluma arriba) -el caso medido: Maestro en S_FALLO y "
                "Esclavo en verde-. Medido: " +
                std::to_string(celdasConCruce) + " celdas con cruce, el peor " +
                std::to_string(frenteMaxMs) + " ms (pre " + std::to_string(frentePeorPre) +
                " ms, k " + std::to_string(frentePeorK) + "); borde 0 ms (las dos direcciones "
                "muertas: no hay viaje de radio que esperar)");
      comprobar(simultaneoMs == 0,
                "G12b (control): en esas " + std::to_string(celdas) + " celdas nunca verde en las "
                "dos puntas a la vez: " + std::to_string(simultaneoMs) + " ms");
      comprobar(precondicion && filaCeroEjerce,
                "G12c (control negativo): en las " + std::to_string(celdas) + " celdas la subida "
                "se corto y la bajada se corto (" + std::to_string(porK) + " tras k repeticiones, " +
                std::to_string(porFallo) + " al caer el Maestro a S_FALLO), y todo GO_GREEN emitido "
                "llego al Esclavo (" + std::to_string(emitidas) + " celdas con GO_GREEN); en la "
                "fila pre = 0 -radio sana hasta el instante de emitir- el GO_GREEN salio, llego y "
                "el Esclavo SI se puso en verde: un arnes que no cortara nada o un escenario sin "
                "verde no aprueba G12a" +
                (malas.empty() ? std::string("") : std::string(". Celdas sin precondicion:") + malas));
      std::printf("   [NOTA]  G12: la excepcion de A9 (S_FALLO por nombre) perdono %lu instantes en "
                  "este barrido; el detector de G conto %lu frente a S_FALLO. A9 se comprueba "
                  "tras el bloque A y no ve estos instantes. No cuenta.\n",
                  g_a9Perdonados - a9Antes, frenteInstantes);
    }

    // ---- G13: D-34 (i). EL MICROCORTE QUE VUELVE CON EL MARGEN DEL ESCLAVO YA PASADO ------
    //
    // El modo de fallo que D-34 CREA (CLAUDE.md 11.7): el Esclavo suelta su verde
    // AVISO_AMBAR_TIMEOUT_MS antes de su silencio. Un corte total que vuelve en ese hueco ya
    // no cuesta un ambar -no llego al silencio- pero SI un rojo, un despeje y un verde del
    // Maestro de mas: el Esclavo lo dice en el PONG y el Maestro reanuda como N-163. Se mide
    // que ese coste este acotado, que no traiga ambar y que la reanudacion sea UNA.
    //
    // POR QUE ESTE BARRIDO (CLAUDE.md 7): el silencio con que el Esclavo recibe la primera
    // orden tras la vuelta esta cuantizado por el latido -siempre un multiplo del periodo del
    // PING-, asi que barrer solo el instante de la vuelta repite los mismos valores. Lo que
    // lo mueve de verdad es el VIAJE de radio tras la vuelta, y por eso se barre la latencia
    // del aire desde el instante de la vuelta: hasta 1500 ms, porque por encima el viaje de
    // ida y vuelta de un GO_RED con su ACK_RED (2 x latencia + RETARDO_RESPUESTA_MS) ya no
    // cabe en TIMEOUT_ACK_MS y el escenario mediria reintentos, no esto (se aborta).
    // LA CLASIFICACION SALE DEL CANAL, NO DEL FIRMWARE: una celda esta "en el hueco" si el
    // silencio del Esclavo paso de SFTY6_SILENCIO_MS - AVISO_AMBAR_TIMEOUT_MS en algun tick
    // y ninguna de las dos puntas paso de SFTY6_SILENCIO_MS (el mismo modelo que G11).
    // EL BORDE del todo-rojo: despeje + LATIDO_MS + 2 x TIMEOUT_ACK_MS, contado desde la
    // vuelta del enlace hasta que el Maestro abre (ambar de su transicion o verde): un latido
    // para que salga el PING que se lleva el aviso, un acuse para el PONG y otro para el
    // GO_RED con su ACK_RED, y el despeje desde ese acuse (N-162).
    {
      const unsigned long LAT_BASE = g_latenciaMs;
      const unsigned long MARGEN = AVISO_AMBAR_TIMEOUT_MS_V;
      const unsigned long LATS_VUELTA[] = { LAT_BASE, 450, 1000, 1500 };
      const unsigned nLats = (unsigned)(sizeof(LATS_VUELTA) / sizeof(LATS_VUELTA[0]));
      const unsigned long OBJ[] = { SIL - MARGEN - LATIDO_MS_V, SIL - MARGEN,
                                    SIL - LATIDO_MS_V / 2, SIL - PASO_MS };
      const unsigned nObj = (unsigned)(sizeof(OBJ) / sizeof(OBJ[0]));
      if (MARGEN == 0 || MARGEN >= SIL) abortar("G13: AVISO_AMBAR_TIMEOUT_MS leido fuera de (0, SFTY6)");
      for (unsigned i = 0; i < nLats; i++)
        if (2 * LATS_VUELTA[i] + RETARDO_RESPUESTA_MS_V + 2 * PASO_MS >= TOUT)
          abortar("G13: con latencia " + std::to_string(LATS_VUELTA[i]) + " ms un GO_RED y su "
                  "ACK_RED no caben en TIMEOUT_ACK_MS: la celda mediria reintentos");
      const unsigned long BORDE_TR = DESPEJE_MS + LATIDO_MS_V + 2 * TOUT;
      const unsigned long HOR13 = SIL + LATIDO_MS_V + TOUT * (NMAX + 2) + DESPEJE_MS +
                                  AMBAR_ESCLAVO_MS_V + 30000;
      unsigned long celdas = 0, enHueco = 0, sinMargen = 0, ambE = 0, espE = 0, ambM = 0, espM = 0;
      unsigned long peorTR = 0, reanudHueco = 0, malasHueco = 0, espureas = 0, simult = 0;
      bool ejercido = true;
      std::string malas;
      std::printf("   G13  obj_ms  lat_vuelta  silE_entrega  hueco  reanud  ambarE/M  todo_rojo_vuelta_ms\n");
      for (unsigned il = 0; il < nLats; il++) {
        for (unsigned io = 0; io < nObj; io++) {
          celdas++;
          g_latenciaMs = LAT_BASE;
          escenarioLimpio(TIEMPOS_G, true);
          if (!alcanzarVerdeG(ESCLAVO, ALCANCE)) ejercido = false;
          avanzar(LATIDO_MS_V + TOUT);   // que el ACK_GREEN haya llegado: QV_ESCLAVO en C_IDLE
          if (!ESCLAVO.verde() || MAESTRO.verde()) ejercido = false;
          CorridaG c;
          c.tCorte = g_t;
          const unsigned long r0 = g_reanudaciones;
          g_enlaceHaciaEsclavo = g_enlaceHaciaMaestro = false;
          long tVuelta = -1, tAbre = -1;
          unsigned long silEntrega = 0;
          bool entregado = false, pasoMargen = false, orfE = false, orfM = false, solto = false;
          for (unsigned long h = 0; h < HOR13; h += PASO_MS) {
            if (tVuelta < 0 && g_t - g_tGobiernoEsclavo >= OBJ[io]) {
              g_latenciaMs = LATS_VUELTA[il];
              g_enlaceHaciaEsclavo = g_enlaceHaciaMaestro = true;
              tVuelta = (long)g_t;
            }
            const unsigned long tg0 = g_tGobiernoEsclavo;
            const unsigned long t = g_t;
            pasoG(c);
            // El silencio que el firmware evaluo en ESTE tick: si en el hubo entrega, la
            // entrega va antes y lo pone a cero -main.cpp refresca antes de mirar-.
            if (!entregado && g_tGobiernoEsclavo == tg0 && t - tg0 > SIL - MARGEN) pasoMargen = true;
            if (!entregado && !ESCLAVO.verde() && ESCLAVO.estado() != S_FALLO_V) solto = true;
            if (tVuelta >= 0 && !entregado && g_tGobiernoEsclavo != tg0) {
              entregado = true;
              silEntrega = g_tGobiernoEsclavo - tg0;
            }
            if (t - g_tGobiernoEsclavo > SIL) orfE = true;
            if (t - g_tRxMaestro > SIL) orfM = true;
            if (tVuelta >= 0 && tAbre < 0 &&
                (MAESTRO.verde() || MAESTRO.estado() == S_AMARILLO_V))
              tAbre = (long)t;
            if (tAbre >= 0 && MAESTRO.verde()) break;
          }
          finG(c);
          g_latenciaMs = LAT_BASE;
          const unsigned long reanud = g_reanudaciones - r0;
          const bool hueco = pasoMargen && !orfE && !orfM;
          if (orfE) espE++;
          if (orfM) espM++;
          ambE += c.entradasFalloE;
          ambM += c.entradasFalloM;
          simult += c.v.simultaneo;
          const long tr = (tAbre >= 0 && tVuelta >= 0) ? tAbre - tVuelta : -1;
          if (hueco) {
            enHueco++;
            reanudHueco += reanud;
            if (tr > (long)peorTR) peorTR = (unsigned long)tr;
            const bool ok = (reanud == 1 && c.entradasFalloE == 0 && c.entradasFalloM == 0 &&
                             tr >= 0 && (unsigned long)tr <= BORDE_TR && solto &&
                             acumuladoMsG(c.v) <= g_latenciaMs + PASO_MS);
            if (!ok) {
              malasHueco++;
              malas += " (obj " + std::to_string(OBJ[io]) + ", lat " +
                       std::to_string(LATS_VUELTA[il]) + ")";
            }
          } else if (!pasoMargen) {
            sinMargen++;
            if (reanud != 0 || solto) espureas++;
          }
          std::printf("   G13  %6lu  %10lu  %12lu  %-5s  %6lu  %4lu/%-4lu  %19ld\n", OBJ[io],
                      LATS_VUELTA[il], silEntrega, hueco ? "si" : (pasoMargen ? "orf" : "no"),
                      reanud, c.entradasFalloE, c.entradasFalloM, tr);
        }
      }
      comprobar(ejercido && enHueco > 0 && malasHueco == 0,
                "G13a (D-34, i): en las " + std::to_string(enHueco) + " celdas cuyo corte total "
                "vuelve con el silencio del Esclavo pasado su margen (" +
                std::to_string(SIL - MARGEN) + " ms) y sin llegar al de nadie (" +
                std::to_string(SIL) + " ms), el Esclavo solto su verde, NO hubo ambar en "
                "ninguna punta, el Maestro reanudo UNA vez por corte (" +
                std::to_string(reanudHueco) + " reanudaciones) y abrio como tarde " +
                std::to_string(peorTR) + " ms despues de la vuelta; borde " +
                std::to_string(BORDE_TR) + " ms (despeje + LATIDO_MS + 2 x TIMEOUT_ACK_MS). "
                "Coste: un rojo, un despeje y un verde de mas, no un ambar" +
                (malas.empty() ? std::string("") : std::string(". Celdas malas:") + malas));
      comprobar(ambE == espE && ambM == espM && simult == 0,
                "G13b: en las " + std::to_string(celdas) + " celdas cada punta entra en ambar "
                "EXACTAMENTE las veces que su silencio paso de " + std::to_string(SIL) +
                " ms: Esclavo " + std::to_string(ambE) + " contra " + std::to_string(espE) +
                " derivadas, Maestro " + std::to_string(ambM) + " contra " +
                std::to_string(espM) + "; verde en las dos a la vez: " +
                std::to_string(simult * PASO_MS) + " ms");
      comprobar(sinMargen > 0 && espureas == 0,
                "G13c (control negativo): en las " + std::to_string(sinMargen) + " celdas que "
                "vuelven ANTES del margen el Esclavo NO solto su verde y el Maestro NO reanudo "
                "(" + std::to_string(espureas) + " celdas con suelta o reanudacion): un firmware "
                "que soltara o reanudara siempre no aprueba G13a");
    }

    // ---- G14: D-34 (ii) y (iii). EL PRECIO DE EXIGIR UN PONG RECIENTE ANTES DEL GO_GREEN ---
    //
    // (ii) con el enlace sano el GO_GREEN sale donde salia: tRef + despeje, a un tick -tRef
    // es el primer instante del rojo del Maestro, que se lee de sus pines-; y con PONG
    // perdidos se retrasa como mucho LATIDO_MS + un tick + un viaje de vuelta (RETARDO del
    // Esclavo + ida y vuelta), sin ambar. (iii) se barre la EDAD del ultimo PONG oido al
    // acabar el despeje tirando los PONG de una ventana que acaba justo ahi, con dos
    // latencias (la del arnes y 450 ms), y se exige que al emitir esa edad no pase de
    // LATIDO_MS. POR QUE ESTAS VENTANAS (CLAUDE.md 7): de cero a cuatro latidos, que llevan
    // la edad de "sano" a bastante mas que un latido sin acercarse al silencio SFTY-6
    // (se aborta si la ventana mas un latido llegara a el).
    {
      const unsigned long LAT_BASE = g_latenciaMs;
      const unsigned long LATS14[] = { LAT_BASE, 450 };
      const unsigned long VENT[] = { 0, LATIDO_MS_V / 2, LATIDO_MS_V, 2 * LATIDO_MS_V,
                                     3 * LATIDO_MS_V, 4 * LATIDO_MS_V };
      const unsigned nV = (unsigned)(sizeof(VENT) / sizeof(VENT[0]));
      if (VENT[nV - 1] + 2 * LATIDO_MS_V >= SIL) abortar("G14: la ventana mas larga llega al silencio SFTY-6");
      unsigned long celdas = 0, emitidas = 0, conEspera = 0, esperaCero = 0, edadMax = 0;
      unsigned long retrasoMax = 0, ambares = 0, ventanaMs = 0, perdidosTot = 0;
      long d0Max = -1, d0Min = 1000000;
      bool ejercido = true, retrasoOk = true;
      std::printf("   G14  lat  ventana_ms  pong_perdidos  edad_fin_despeje  edad_al_emitir  retraso_ms\n");
      for (unsigned il = 0; il < 2; il++) {
        const unsigned long viaje = RETARDO_RESPUESTA_MS_V + 2 * LATS14[il] + PASO_MS;
        const unsigned long BORDE_RET = LATIDO_MS_V + PASO_MS + viaje;
        for (unsigned iv = 0; iv < nV; iv++) {
          celdas++;
          g_latenciaMs = LATS14[il];
          escenarioLimpio(TIEMPOS_G, true);
          CorridaG c;
          if (!alcanzarVerdeG(MAESTRO, ALCANCE)) { ejercido = false; continue; }
          long tRojoM = -1;
          for (unsigned long g = 0; g < VERDE_MS + 60000; g += PASO_MS) {
            const unsigned long t = g_t;
            pasoG(c);
            if (!MAESTRO.verde()) { tRojoM = (long)t; break; }
          }
          if (tRojoM < 0 || !MAESTRO.rojo()) { ejercido = false; continue; }
          const unsigned long tFin = (unsigned long)tRojoM + DESPEJE_MS;
          g_pongPerderDesdeT = tFin - VENT[iv];
          g_pongPerderHastaT = (VENT[iv] == 0) ? 0 : tFin;
          const unsigned long p0 = g_pongPerdidos;
          const unsigned long n0 = g_goVerdeEmitidos;
          long edadFin = -1, tEmit = -1, edadEmit = -1;
          for (unsigned long g = 0; g < DESPEJE_MS + SIL; g += PASO_MS) {
            const unsigned long t = g_t;
            pasoG(c);
            if (edadFin < 0 && t >= tFin) edadFin = (long)(t - g_tRxMaestro);
            if (g_goVerdeEmitidos != n0) { tEmit = (long)t; edadEmit = (long)(t - g_tRxMaestro); break; }
          }
          // y un rato despues, para ver que no aparece ningun ambar por la espera
          correrG(c, AMBAR_ESCLAVO_MS_V + 2 * TOUT);
          finG(c);
          g_pongPerderDesdeT = g_pongPerderHastaT = 0;
          g_latenciaMs = LAT_BASE;
          const unsigned long perdidos = g_pongPerdidos - p0;
          ambares += c.entradasFalloE + c.entradasFalloM;
          ventanaMs += acumuladoMsG(c.v);
          long retraso = -1;
          if (tEmit >= 0) {
            emitidas++;
            retraso = tEmit - (long)tFin;
            if (VENT[iv] == 0) {
              if (retraso > d0Max) d0Max = retraso;
              if (retraso < d0Min) d0Min = retraso;
            }
            if ((unsigned long)edadEmit > edadMax) edadMax = (unsigned long)edadEmit;
            if (retraso > (long)retrasoMax) retrasoMax = (unsigned long)retraso;
            if (retraso < 0 || (unsigned long)retraso > BORDE_RET) retrasoOk = false;
            if (edadFin > (long)LATIDO_MS_V) {
              conEspera++;
              if (retraso <= (long)PASO_MS) esperaCero++;
            }
          } else {
            ejercido = false;
          }
          perdidosTot += perdidos;
          std::printf("   G14  %3lu  %10lu  %13lu  %16ld  %14ld  %10ld\n", LATS14[il], VENT[iv],
                      perdidos, edadFin, edadEmit, retraso);
        }
      }
      g_latenciaMs = LAT_BASE;
      comprobar(ejercido && d0Min >= 0 && d0Max <= (long)PASO_MS && retrasoOk && ambares == 0,
                "G14a (D-34, ii): el GO_GREEN sale con el enlace sano a " + std::to_string(d0Min) +
                ".." + std::to_string(d0Max) + " ms del fin del despeje (medido desde el primer "
                "rojo del Maestro en sus pines; borde un tick, " + std::to_string(PASO_MS) +
                " ms) y, con PONG perdidos, como tarde " + std::to_string(retrasoMax) +
                " ms (borde LATIDO_MS + tick + viaje de vuelta); en las " +
                std::to_string(celdas) + " celdas, " + std::to_string(ambares) +
                " entradas en ambar y " + std::to_string(ventanaMs) + " ms de ventana");
      comprobar(emitidas == celdas && edadMax <= LATIDO_MS_V,
                "G14b (D-34, iii): en las " + std::to_string(emitidas) + " emisiones barridas "
                "(dos latencias, ventanas de 0 a " + std::to_string(VENT[nV - 1]) + " ms sin "
                "PONG) la edad del ultimo PONG oido al emitir el GO_GREEN fue como mucho " +
                std::to_string(edadMax) + " ms; borde LATIDO_MS, " + std::to_string(LATIDO_MS_V) +
                " ms");
      // Una ventana de medio latido puede no contener ninguna emision de PONG: por eso no se
      // exige perdida en cada celda, sino que el barrido entero haya tirado alguno y que
      // alguna celda acabe el despeje con el PONG mas viejo que un latido.
      comprobar(perdidosTot > 0 && conEspera > 0 && esperaCero == 0,
                "G14c (control negativo): en " + std::to_string(conEspera) + " celdas el despeje "
                "acabo con el ultimo PONG MAS VIEJO que un latido y en TODAS el GO_GREEN espero "
                "(" + std::to_string(esperaCero) + " salieron en el tick del despeje): un "
                "firmware que no exigiera el PONG aprobaria G14a sin haber esperado nada");
    }
  }
  // =========================================================================
  std::printf("\n--- BLOQUE H: el ambar del Poste 2 AVISA al Poste 1 (N-142, §3.16-A) --\n");
  // EL DEFECTO QUE ESTE BLOQUE VIENE A VIGILAR, medido en el fuente el 11/09: el ambar de
  // emergencia del Esclavo tiene DOS puertas en bluetooth.cpp -sin PIN contra 'cmd' y con
  // PIN contra 'accion'- y protocolo_enviarPaquete(CMD_AMBAR_ESCLAVO) tenia UN SOLO
  // llamador, en la de CON PIN. La app manda por la de SIN PIN (app.js, lista SIN_PIN), o
  // sea que el aviso de N-142 NO LO HABIA DISPARADO NUNCA UN TELEFONO: el tecnico del
  // Poste 2 pedia ambar, esa punta se iba a S_FALLO -intermitente con la pluma ARRIBA- y
  // el Maestro seguia su ciclo dando VERDE en el Poste 1 hacia el mismo carril, hasta que
  // agotara reintentos en el siguiente cambio. Es el candidato mas firme del DAR PASO del
  // Sisga (roadmap §3.16).
  //
  // POR QUE NO LO VEIA ESTE ARNES: el ambar de la app entraba por una orden del adaptador
  // que era una TRANSCRIPCION de la puerta CON PIN. Se median dos copias buenas de una
  // puerta mala. Ahora se teclea la linea de la app -leida del C++- en el bluetooth.cpp
  // REAL, que se compila en la DLL del Esclavo.
  //
  // EL BORDE, escrito al lado (CLAUDE.md §7), y sale del CAMINO, no de un gusto: la linea
  // se despacha en un tick y el aviso sale al aire en ese mismo instante; cruza UN VIAJE
  // de radio; el coordinador del Maestro lo lee en el tick en que llega -dentro de
  // modoAutomatico_loop()-; main.cpp lo consume en la vuelta SIGUIENTE, y el cambio de
  // modo corre su setup() una vuelta despues, porque main.cpp compara contra una COPIA
  // del modo leida al principio de la vuelta -eso esta medido y escrito en el propio
  // main.cpp, y el adaptador lo transcribe sin corregirlo-. Mas el tick de observacion:
  // un viaje + cuatro ticks. Todo lo que pase de ahi es el Maestro esperando algo que no
  // necesita esperar, y eso lo cierra un firmware.
  {
    const long TIEMPOS_H = tiempos((int)VERDE_MIN_MIN_V, (int)ROJO_MIN_MIN_V,
                                   (int)DESPEJE_POR_DEFECTO_S);
    const unsigned long DESPEJE_MS_H = DESPEJE_POR_DEFECTO_S * 1000UL;
    const unsigned long VERDE_MS_H = VERDE_MIN_MIN_V * 60000UL;
    const unsigned long ROJO_MS_H = ROJO_MIN_MIN_V * 60000UL;
    const unsigned long ALCANCE_H =
        2 * (DESPEJE_MS_H + AMBAR_ESCLAVO_MS_V) + VERDE_MS_H + ROJO_MS_H + 60000;
    const unsigned long BORDE_AVISO_MS = g_latenciaMs + 4 * PASO_MS;
    // Lo que se deja correr despues de pedir el ambar: el verde entero que el Maestro
    // tenia encendido mas un minuto. Si no se parara, la ventana seria ese verde entero
    // -"hasta 3 minutos", el residual de N-142 que el bloque G8-b publica sin cerrar-.
    const unsigned long TRAS_AMBAR_H = VERDE_MS_H + 60000;

    std::printf("   La app pide el ambar con la linea %s, leida del despachador real.\n",
                LINEA_AMBAR_APP.c_str());
    std::printf("   Acuses que esa rama puede contestar (leidos del C++): ");
    for (size_t i = 0; i < RESULTS_AMBAR_APP.size(); i++)
      std::printf("%s%s", RESULTS_AMBAR_APP[i].c_str(),
                  i + 1 < RESULTS_AMBAR_APP.size() ? ", " : "\n");
    std::printf("   Borde: %lu ms (un viaje de radio, %lu ms, + cuatro ticks de %lu ms).\n",
                BORDE_AVISO_MS, g_latenciaMs, PASO_MS);

    // ---- H0 (control negativo): el lector de la puerta sabe fallar --------------------
    // Si literalPuertaAmbar() devolviera cualquier rama, el bloque teclearia otra cosa y
    // "el Maestro no se entera" mediria al arnes. Se ejerce sobre un despachador
    // sintetico: una rama que enciende ambar y otra que no, y la de un comentario.
    {
      const std::string FALSO =
          "// if (strcmp(cmd, \"CMD:COMENTADO\") == 0) { semaforo_iniciarFallo(); }\n"
          "if (strcmp(cmd, \"CMD:OTRA\") == 0) { semaforo_forzarRojo(); return; }\n"
          "if (strcmp(cmd, \"CMD:LA_BUENA\") == 0) { semaforo_iniciarFallo();\n"
          "  enviarTramaConCrc(\"$ACK,CMD:LA_BUENA,RESULT:OK\");\n"
          "  enviarTramaConCrc(\"$ACK,CMD:LA_BUENA,RESULT:YA\"); return; }\n";
      std::vector<std::string> res;
      const std::string hallada = literalPuertaAmbar(FALSO, &res);
      std::vector<std::string> nada;
      const std::string vacia = literalPuertaAmbar(
          "if (strcmp(cmd, \"CMD:OTRA\") == 0) { semaforo_forzarRojo(); }", &nada);
      comprobar(hallada == "CMD:LA_BUENA" && res.size() == 2 && res[0] == "OK" &&
                res[1] == "YA" && vacia.empty(),
                "H0 (control negativo): el lector de la puerta del ambar encuentra la rama "
                "que enciende ambar y NO la del comentario ni la que solo fuerza rojo, y "
                "devuelve vacio -no una rama cualquiera- cuando no hay ninguna");
    }

    // ---- H1: con radio, el aviso llega y el cruce se para -----------------------------
    CorridaG h1;
    bool h1Ejercido;
    long h1Entradas = -1, h1Origen = -1;
    std::string h1Result;
    {
      escenarioLimpio(TIEMPOS_H, true);
      bool ok = alcanzarVerdeG(MAESTRO, ALCANCE_H);
      avanzar(3000);
      ok = ok && MAESTRO.verde() && ESCLAVO.rojo() &&
           MAESTRO.orden("entradas_ambar") == 0;
      ok = ok && tecleaAmbarApp() == 1;
      h1.tCorte = g_t;
      pasoG(h1);                      // el despachador real atiende la linea en este tick
      ok = ok && ESCLAVO.orden("ambar_latch") == 1 && ESCLAVO.estado() == S_FALLO_V;
      h1Result = resultDelUltimoAcuse();
      correrG(h1, TRAS_AMBAR_H);
      finG(h1);
      h1Entradas = MAESTRO.orden("entradas_ambar");
      h1Origen = MAESTRO.orden("ambar_es_del_esclavo");
      h1Ejercido = ok;
    }
    imprimirTrazaG("H1 (el Poste 2 pide ambar desde la app con el Maestro en VERDE):", h1);
    comprobar(h1Ejercido,
              "H1 (control): el Maestro estaba en VERDE con el Esclavo en rojo, se tecleo " +
              LINEA_AMBAR_APP + " en el despachador real del Esclavo y esa punta quedo en "
              "S_FALLO con el cerrojo puesto. El acuse al telefono fue RESULT:" +
              (h1Result.empty() ? std::string("(ninguno)") : h1Result));
    comprobar(acumuladoMsG(h1.v) <= BORDE_AVISO_MS && h1.v.simultaneo == 0,
              "H1: el Maestro DEJA DE DAR VERDE frente al ambar de emergencia del otro "
              "poste: ventana " + std::to_string(acumuladoMsG(h1.v)) + " ms (borde " +
              std::to_string(BORDE_AVISO_MS) + " ms), " +
              std::to_string(h1.v.simultaneo * PASO_MS) + " ms con verde en las dos. Sin "
              "el aviso esa ventana es el verde entero que tuviera encendido -hasta " +
              std::to_string(VERDE_MS_H / 1000) + " s con este ciclo-");
    comprobar(h1Entradas == 1 && h1Origen == 1,
              "H1 (la otra mitad): el Maestro entro en MODO_AMBAR " +
              std::to_string(h1Entradas) + " vez -no ciclo, no oscilo- y lo apunto como "
              "AMBAR DEL ESCLAVO (origen=" + std::to_string(h1Origen) + "), que es lo que "
              "decide si una cancelacion del Poste 2 puede sacarlo (D-8)");

    // ---- H2: segunda pulsacion CON radio, con la luz ya en ambar ----------------------
    // Es el control de H3: la MISMA fila de la tabla del firmware -el equipo ya estaba en
    // ambar- pero con la radio viva. Sin ella, H3 podria salir distinto por estar en otra
    // fila y no por la radio, que es justo lo que se quiere aislar.
    std::string h2Result;
    long h2Entradas = -1, h2AlarmasAntes = -1, h2AlarmasDespues = -1;
    bool h2Ejercido;
    {
      const bool yaEnAmbar = (ESCLAVO.estado() == S_FALLO_V);
      h2AlarmasAntes = ESCLAVO.orden("alarmas");
      bool ok = yaEnAmbar && tecleaAmbarApp() == 1;
      avanzar(3 * PASO_MS);
      h2Result = resultDelUltimoAcuse();
      // D-31: se deja correr el plazo ENTERO del acuse mas sus reintentos -leidos del C++-
      // antes de mirar las alarmas. Preguntar antes seria aprobar por no haber esperado.
      avanzar(AVISO_AMBAR_TIMEOUT_MS_V * (AVISO_AMBAR_REINTENTOS_V + 1) + 5000);
      h2AlarmasDespues = ESCLAVO.orden("alarmas");
      h2Entradas = MAESTRO.orden("entradas_ambar");
      h2Ejercido = ok && ESCLAVO.estado() == S_FALLO_V && !h2Result.empty();
    }
    comprobar(h2Ejercido && h2Result != h1Result && h2Entradas == 1,
              "H2: la SEGUNDA pulsacion, con el equipo ya en ambar y la radio viva, se "
              "contesta distinto de la primera (RESULT:" + h2Result + " contra RESULT:" +
              h1Result + ") y NO vuelve a entrar en MODO_AMBAR (" +
              std::to_string(h2Entradas) + " entrada): re-armar manda un todo-rojo y el "
              "operario que pulsa dos veces no sabria cual de las dos movio la luz");
    // D-31 (1): LA LINEA QUE IMPIDE QUE ESTE ARREGLO SE VUELVA UNA FALSA ALARMA.
    //
    // Es la mitad que costo descartar el primer diseno -"que el Poste 1 acuse siempre"-.
    // Aqui el Maestro YA esta en MODO_AMBAR, o sea CALLADO por SFTY-21, asi que un
    // firmware que esperase acuse en cada pulsacion no recibiria ninguno y gritaria con
    // la radio SANA. Y eso no es un aviso de mas: es lo que ensena al tecnico a ignorar
    // la unica senal que D-31 existe para darle. Se exige las DOS cosas juntas -que el
    // acuse diga "ya avisado" y que NO salga alarma-, porque cada una sin la otra se
    // puede cumplir por el motivo equivocado.
    comprobar(h2AlarmasDespues == h2AlarmasAntes &&
                  ESCLAVO.orden(("ack:RESULT:" + h2Result).c_str()) == 1 &&
                  h2Result != h1Result,
              "H2 (D-31): la segunda pulsacion contesta que el Poste 1 YA ESTABA AVISADO "
              "(RESULT:" + h2Result + ") y NO espera nada: pasado el plazo entero del "
              "acuse (" + std::to_string(AVISO_AMBAR_TIMEOUT_MS_V) + " ms x " +
              std::to_string(AVISO_AMBAR_REINTENTOS_V + 1) + " intentos) las alarmas del "
              "telefono siguen en " + std::to_string(h2AlarmasDespues) + ", las mismas de "
              "antes. Con el Maestro ya en MODO_AMBAR esta callado por SFTY-21: esperar "
              "acuse aqui daria 'nadie me oyo' CON LA RADIO SANA cada vez que alguien "
              "pulsa dos veces");

    // ---- H3: la radio CAIDA, y lo que el equipo le dice al telefono -------------------
    // El corte es TOTAL y dura mas que el silencio de SFTY-6, que es la unica averia de
    // radio que esta punta PUEDE conocer: la declara ella misma con su $ALARM FALLO_RF.
    // Con el corte, el Esclavo ya esta en ambar por orfandad, asi que la fila de la tabla
    // es la de H2 -"ya estaba en ambar"- y la unica diferencia entre las dos es la radio.
    std::string h3Result;
    long h3Entradas = -1, h3Alarmas = 0;
    bool h3Ejercido;
    CorridaG h3;
    {
      escenarioLimpio(TIEMPOS_H, true);
      bool ok = alcanzarVerdeG(MAESTRO, ALCANCE_H);
      avanzar(3000);
      ok = ok && MAESTRO.verde();
      g_enlaceHaciaEsclavo = g_enlaceHaciaMaestro = false;
      h3.tCorte = g_t;
      correrG(h3, SFTY6_SILENCIO_MS_V + 5000);
      h3Alarmas = ESCLAVO.orden("alarmas");
      ok = ok && ESCLAVO.estado() == S_FALLO_V && h3Alarmas >= 1;
      ok = ok && tecleaAmbarApp() == 1;
      correrG(h3, 3 * PASO_MS);
      h3Result = resultDelUltimoAcuse();
      ok = ok && ESCLAVO.orden("ambar_latch") == 1;
      correrG(h3, TRAS_AMBAR_H);
      finG(h3);
      h3Entradas = MAESTRO.orden("entradas_ambar");
      h3Ejercido = ok && !h3Result.empty();
    }
    imprimirTrazaG("H3 (radio caida del todo; el Poste 2 pide ambar con la radio muerta):", h3);
    comprobar(h3Ejercido,
              "H3 (control): con la radio cortada mas de " +
              std::to_string(SFTY6_SILENCIO_MS_V) + " ms el Esclavo ya estaba en ambar por "
              "orfandad y habia emitido " + std::to_string(h3Alarmas) + " alarma(s) por el "
              "cable del telefono; la orden de ambar se atendio igual y el cerrojo quedo "
              "puesto. El ambar de quien esta en la calzada NO depende de la radio");
    comprobar(h3Result != h2Result,
              "H3: y el $ACK LO DICE. Con la radio caida el equipo contesta RESULT:" +
              h3Result + " donde con la radio viva y la MISMA luz contesta RESULT:" +
              h2Result + ": el tecnico se entera de que el otro poste no se ha enterado. Un "
              "acuse igual en los dos casos seria una mentira con formato de exito "
              "(CLAUDE.md §2), y aqui la mentira manda a alguien a casa creyendo que el "
              "cruce entero esta en ambar");
    comprobar(h3Entradas == 0,
              "H3 (la medida que explica al Sisga): con la radio muerta el Maestro NO entra "
              "en MODO_AMBAR (" + std::to_string(h3Entradas) + " entradas) porque el aviso "
              "no llega. Lo que lo protege entonces es lo de siempre -agotar reintentos y "
              "caer a C_FALLO-, no el aviso; por eso el $ACK de arriba tiene que decirlo");

    // ---- H4: la averia FEA -solo muere el transmisor del Esclavo- ---------------------
    //
    // 🔴 D-31 (12/09): ESTE BLOQUE ERA UNA [NOTA] QUE NO CONTABA, Y HOY CUENTA.
    //
    // Lo era con razon: ningun firmware de esta punta podia aprobarlo solo. El Maestro le
    // sigue hablando, asi que el Esclavo NO tiene forma de saber que lo que EL emite no
    // sale -su unico dato de radio es el silencio de lo que RECIBE-, y el $ACK que el
    // telefono recibia era identico al de la radio sana. Con el acuse construido esa
    // pregunta ya tiene quien la conteste, asi que la nota se convierte en comprobacion.
    //
    // LO QUE SE EXIGE, Y ES LO UNICO QUE CIERRA LA VENTANA: que el equipo lo DIGA. No que
    // el Poste 1 se entere -no puede: el transmisor esta muerto-, ni que el $ACK cambie
    // -sale antes de saberlo, y eso es D-31 (3): el ambar no espera-. Lo que se exige es
    // que, pasado el plazo y sus reintentos, salga un $ALARM que diga que NO SE PUDO
    // CONFIRMAR. La trama se lee entera y no se cuenta: un contador aprobaria igual con
    // la alarma de radio de siempre, que en este escenario no se emite.
    CorridaG h4;
    bool h4Ejercido;
    long h4Entradas = -1, h4AlarmasAntes = -1, h4AlarmasDespues = -1;
    long h4DiceEvento = 0, h4DiceCausa = 0, h4DiceAccion = 0;
    std::string h4Result;
    {
      escenarioLimpio(TIEMPOS_H, true);
      bool ok = alcanzarVerdeG(MAESTRO, ALCANCE_H);
      avanzar(3000);
      ok = ok && MAESTRO.verde() && ESCLAVO.rojo();
      g_enlaceHaciaMaestro = false;          // solo esta direccion, y no vuelve
      h4.tCorte = g_t;
      h4AlarmasAntes = ESCLAVO.orden("alarmas");
      ok = ok && tecleaAmbarApp() == 1;
      pasoG(h4);
      ok = ok && ESCLAVO.orden("ambar_latch") == 1 && ESCLAVO.estado() == S_FALLO_V;
      h4Result = resultDelUltimoAcuse();
      correrG(h4, TRAS_AMBAR_H);
      finG(h4);
      h4AlarmasDespues = ESCLAVO.orden("alarmas");
      h4DiceEvento = ESCLAVO.orden(("alarma:EVENTO:" + ALARMA_AVISO.evento).c_str());
      h4DiceCausa  = ESCLAVO.orden(("alarma:CAUSA:" + ALARMA_AVISO.causa).c_str());
      h4DiceAccion = ESCLAVO.orden(("alarma:ACCION:" + ALARMA_AVISO.accion).c_str());
      h4Entradas = MAESTRO.orden("entradas_ambar");
      h4Ejercido = ok;
    }
    imprimirTrazaG("H4 (solo muere el transmisor del Esclavo: pide ambar y el aviso no sale):",
                   h4);
    comprobar(h4Ejercido,
              "H4 (control): con la direccion Esclavo->Maestro muerta y el Maestro todavia "
              "hablando, el ambar de emergencia se pone IGUAL en el Poste 2 y el cerrojo "
              "queda puesto: lo que protege a quien esta en esa calzada no cuelga de la "
              "radio. El $ACK inmediato fue RESULT:" + h4Result + ", y sale antes de saber "
              "nada del otro poste a proposito (D-31 (3): el ambar no espera)");
    comprobar(h4Ejercido && h4Entradas == 0 &&
                  h4AlarmasDespues > h4AlarmasAntes &&
                  h4DiceEvento == 1 && h4DiceCausa == 1 && h4DiceAccion == 1,
              "H4 (D-31, LA QUE CIERRA LA VENTANA): el Maestro NO se entero (" +
              std::to_string(h4Entradas) + " entradas en MODO_AMBAR) y el equipo LO DICE: "
              "tras el plazo del acuse y sus " +
              std::to_string(AVISO_AMBAR_REINTENTOS_V) + " reintentos emitio $ALARM EVENTO:" +
              ALARMA_AVISO.evento + " CAUSA:" + ALARMA_AVISO.causa + " ACCION:" +
              ALARMA_AVISO.accion + " (" + std::to_string(h4AlarmasAntes) + " -> " +
              std::to_string(h4AlarmasDespues) + " alarmas). Dice que NO PUDO CONFIRMARLO, "
              "no que el otro poste no se entero: con reintentos de por medio una trama "
              "perdida se ve igual que un transmisor roto. La ventana medida -verde del "
              "Maestro frente al ambar del Esclavo- fue de " +
              std::to_string(acumuladoMsG(h4.v)) + " ms, " +
              std::to_string(h4.v.simultaneo * PASO_MS) + " ms con verde en las dos");

    // ---- H5: EL CONTROL DE H4 -el MISMO escenario con el transmisor SANO- -------------
    //
    // Sin esta corrida, H4 aprobaria igual de bien a un firmware que gritase SIEMPRE: una
    // alarma que no sabe callarse no distingue nada, y es exactamente la forma de defecto
    // que D-31 (1) descarto en el primer diseno. Aqui no se corta ninguna direccion; todo
    // lo demas -el ciclo, la fase, la linea que se teclea, el tiempo que se deja correr-
    // es letra por letra lo de H4.
    CorridaG h5;
    bool h5Ejercido;
    long h5Entradas = -1, h5AlarmasAntes = -1, h5AlarmasDespues = -1;
    std::string h5Result, h5Segundo;
    {
      escenarioLimpio(TIEMPOS_H, true);
      bool ok = alcanzarVerdeG(MAESTRO, ALCANCE_H);
      avanzar(3000);
      ok = ok && MAESTRO.verde() && ESCLAVO.rojo();
      h5.tCorte = g_t;
      h5AlarmasAntes = ESCLAVO.orden("alarmas");
      ok = ok && tecleaAmbarApp() == 1;
      pasoG(h5);
      ok = ok && ESCLAVO.orden("ambar_latch") == 1 && ESCLAVO.estado() == S_FALLO_V;
      h5Result = resultDelUltimoAcuse();
      correrG(h5, TRAS_AMBAR_H);
      finG(h5);
      h5AlarmasDespues = ESCLAVO.orden("alarmas");
      h5Entradas = MAESTRO.orden("entradas_ambar");
      // La prueba positiva de que el acuse LLEGO y quedo recordado: se vuelve a pulsar y
      // el equipo tiene que contestar lo de "ya avisado", que solo puede decir si hubo
      // trama de vuelta. Mirar solo la ausencia de alarma dejaria pasar un firmware que
      // no cronometra nada.
      ok = ok && tecleaAmbarApp() == 1;
      avanzar(3 * PASO_MS);
      h5Segundo = resultDelUltimoAcuse();
      h5Ejercido = ok;
    }
    comprobar(h5Ejercido && h5Entradas == 1 && h5AlarmasDespues == h5AlarmasAntes &&
                  h5Segundo != h5Result && !h5Segundo.empty(),
              "H5 (control de H4): el MISMO escenario con el transmisor SANO. El Maestro se "
              "entero (" + std::to_string(h5Entradas) + " entrada en MODO_AMBAR), NO salio "
              "ninguna alarma (" + std::to_string(h5AlarmasDespues) + ", las mismas que "
              "antes) y la segunda pulsacion contesta RESULT:" + h5Segundo + " en vez de "
              "RESULT:" + h5Result + ", que es la memoria del acuse: solo puede decirla si "
              "hubo trama de vuelta. Sin esta corrida, H4 la pasaria un firmware que "
              "gritara siempre");

    // ---- H6: D-31 (2) - EL CANCELAR BORRA LA MEMORIA, Y SI NO, LA MEMORIA MIENTE ------
    //
    // Es la mitad de D-31 que ningun otro bloque toca, y sin ella el arreglo se degrada
    // SOLO CON EL TIEMPO: armado y cancelado son la misma maquina, y partirlos deja un
    // estado que miente. Con la memoria sin borrar, el ambar SIGUIENTE de este poste
    // contestaria "el Poste 1 ya lo sabe" apoyandose en un acuse de un ambar que ya no
    // existe, y NO ESPERARIA confirmacion: volveria la ceguera de H4 sin que nada la
    // delate, y encima solo a partir de la segunda vez -que es como se cuelan los
    // defectos que nadie reproduce-.
    //
    // LA SECUENCIA, y cada paso hace falta: se arma con la radio SANA (el acuse llega y
    // la memoria queda puesta) -> se CANCELA -> se mata el transmisor -> se vuelve a
    // armar. Lo que se exige es que ESE SEGUNDO ARMADO VUELVA A ESPERAR y acabe en
    // alarma. Si el paso del cancelar no borrara nada, no habria alarma y esta linea cae.
    long h6AlarmasAntes = -1, h6AlarmasDespues = -1, h6DiceCausa = 0;
    std::string h6Primero, h6TrasCancelar, h6Segundo;
    bool h6Ejercido;
    {
      escenarioLimpio(TIEMPOS_H, true);
      bool ok = alcanzarVerdeG(MAESTRO, ALCANCE_H);
      avanzar(3000);
      ok = ok && MAESTRO.verde();
      // 1. Armado con la radio sana: el acuse llega y la memoria se pone.
      ok = ok && tecleaAmbarApp() == 1;
      avanzar(3 * PASO_MS);
      h6Primero = resultDelUltimoAcuse();
      avanzar(AVISO_AMBAR_TIMEOUT_MS_V * (AVISO_AMBAR_REINTENTOS_V + 1) + 5000);
      // 2. Cancelacion desde el telefono, con PIN. El prefijo del PIN se LEE del
      //    despachador; el nombre de la accion se escribe, igual que hace costura_14 y
      //    por el mismo motivo: es un contrato con el exterior -lo teclea la app- y
      //    deducirlo seria adivinar.
      ok = ok && ESCLAVO.orden(("bt:" + PREFIJO_PIN_APP + "CANCELAR_AMBAR").c_str()) == 1;
      avanzar(3 * PASO_MS);
      h6TrasCancelar = ESCLAVO.orden("ack:CMD:CANCELAR_AMBAR") == 1 ? "contestado" : "";
      ok = ok && ESCLAVO.orden("ambar_latch") == 0 && !h6TrasCancelar.empty();
      avanzar(5000);
      // 3. Muere el transmisor y se vuelve a pedir ambar.
      g_enlaceHaciaMaestro = false;
      h6AlarmasAntes = ESCLAVO.orden("alarmas");
      ok = ok && tecleaAmbarApp() == 1;
      avanzar(3 * PASO_MS);
      h6Segundo = resultDelUltimoAcuse();
      avanzar(AVISO_AMBAR_TIMEOUT_MS_V * (AVISO_AMBAR_REINTENTOS_V + 1) + 5000);
      h6AlarmasDespues = ESCLAVO.orden("alarmas");
      h6DiceCausa = ESCLAVO.orden(("alarma:CAUSA:" + ALARMA_AVISO.causa).c_str());
      h6Ejercido = ok;
    }
    // El literal contra el que se compara NO se escribe: es h5Segundo, o sea el que el
    // propio equipo contesta cuando la memoria SI esta puesta (H5). Asi la linea sigue
    // midiendo lo mismo el dia que ese RESULT se renombre.
    comprobar(h6Ejercido && h6AlarmasDespues > h6AlarmasAntes && h6DiceCausa == 1 &&
                  !h6Segundo.empty() && h6Segundo != h5Segundo,
              "H6 (D-31 (2)): tras CANCELAR, el ambar siguiente VUELVE A ESPERAR el acuse. "
              "Se armo con la radio sana (RESULT:" + h6Primero + "), se cancelo, se mato el "
              "transmisor y se volvio a armar: el equipo contesta RESULT:" + h6Segundo +
              " y NO el de 'ya avisado' (RESULT:" + h5Segundo + ", que es el que da con la "
              "memoria puesta), y acaba emitiendo $ALARM CAUSA:" + ALARMA_AVISO.causa +
              " (" + std::to_string(h6AlarmasAntes) + " -> " +
              std::to_string(h6AlarmasDespues) + " alarmas). Sin el borrado de la memoria "
              "en el cancelar, este segundo ambar diria 'el Poste 1 ya lo sabe' sobre un "
              "acuse de un ambar que ya no existe y no avisaria de nada");
  }

  // reportar(): no cuenta. Es lo que la excepcion de A9 dejo pasar en los bloques A a F, que
  // no se diseniaron para esto. MEDIDO el 11/09 con una copia instrumentada: la racha larga
  // es del bloque D -D5/D6-, el Esclavo dando verde POR RELOJ en su Modo Degradado con el
  // Maestro en S_FALLO. El Degradado del Maestro no se compila en este arnes (ver
  // adaptador_maestro.cpp), asi que esa racha no dice que pasa en campo: dice que A9 la
  // perdona sin mirarla. Si orquestador_degradado -que si compila los dos Degradados- mide
  // un verde frente a S_FALLO, NO se comprobo al escribir esto.
  std::printf("   [NOTA]  la excepcion de A9 (S_FALLO por nombre) perdono %lu instantes en los "
              "bloques A a F, racha mas larga %lu ms (bloque D: verde por reloj del Degradado "
              "del Esclavo frente al S_FALLO de un Maestro sin Degradado compilado). No cuenta.\n",
              a9PerdonadosAF, a9RachaAF * PASO_MS);

  // =========================================================================
  std::printf("\n==============================================================\n");
  comprobar(g_verdeSimultaneo == 0,
            "RESUMEN: en los " + std::to_string(g_instantes) + " instantes observados "
            "de TODO el barrido -bloques A a G- no hubo NI UNO con verde encendido en "
            "las dos puntas. Es la propiedad que motivo este arnes, medida sobre el C++ "
            "real de las dos y sobre lo que escribio en los pines");
  comprobar(g_enclavamientoRoto == 0,
            "RESUMEN (SFTY-2): en ninguno de esos instantes coincidieron rojo y verde "
            "en la misma cara de ninguna de las dos puntas");
  {
    char msg[420];
    std::snprintf(msg, sizeof(msg),
        "RESUMEN (SFTY-28 con la derogacion parcial de D-33): en ninguno de esos "
        "instantes hubo pluma arriba sin verde encendido fuera de S_FALLO MAS ALLA del "
        "retardo de bajada. La ventana mas larga medida fue de %lu ms en el Maestro y "
        "%lu ms en el Esclavo, contra los %lu ms que declara el C++ real: la excepcion "
        "de D-33 esta ACOTADA, no concedida",
        g_peorVentanaPlumaMs[0], g_peorVentanaPlumaMs[1], g_retardoPlumaMs);
    comprobar(g_talanqueraSinVerde == 0, msg);
  }
  {
    // EL CONTROL QUE LE FALTA A TODA INVERSION (CLAUDE.md 9). La linea de arriba la
    // pasaria igual de bien un firmware que NUNCA levantara la pluma: sin esto, el
    // reparto habria cambiado una comprobacion por una tapia. Se exige que la ventana
    // haya EXISTIDO -o sea, que la pluma llegara a estar arriba con la luz ya en rojo-,
    // que es el comportamiento nuevo que D-33 manda construir.
    char msg[360];
    std::snprintf(msg, sizeof(msg),
        "CONTROL de D-33: la ventana de 'pluma arriba con la luz ya en rojo' EXISTIO de "
        "verdad en las dos puntas (%lu ms y %lu ms). Si alguna fuera cero, la pluma "
        "seguiria bajando en el mismo instante del rojo y el retardo seria un adorno",
        g_peorVentanaPlumaMs[0], g_peorVentanaPlumaMs[1]);
    comprobar(g_peorVentanaPlumaMs[0] > 0 && g_peorVentanaPlumaMs[1] > 0, msg);
  }

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
