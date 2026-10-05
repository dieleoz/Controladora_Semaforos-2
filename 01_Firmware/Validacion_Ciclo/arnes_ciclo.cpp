// ===== 01_Firmware/Validacion_Ciclo/arnes_ciclo.cpp =====
//
// FASE 6 DEL PLAN — EL PRIMER TROZO DE FIRMWARE QUE SE MIDE EN VEZ DE DUPLICARSE.
//
// QUE CAMBIA RESPECTO AL BANCO EN PYTHON.
//
// Hasta hoy, el barrido de las 86.400 posiciones del dia corria contra un ESPEJO en
// Python de ciclo_degradado_fase(), reescrito a mano. Y como nadie puede garantizar
// que un espejo siga al original, el validador de costura llego a comprobar el espejo
// LINEA POR LINEA contra el C++ con expresiones regulares: una prueba para vigilar a
// la prueba. Eso es un sintoma, no una solucion.
//
// Aqui se incluye el ciclo_degradado.h REAL del firmware y se barre sobre EL. Si
// alguien cambia el calculo, esto mide el calculo nuevo automaticamente. No hay espejo
// que envejecer, y por tanto no hay N-36 posible en este camino.
//
// POR QUE ESTA FUNCION Y NO OTRA. Es la unica barrera contra el VERDE SIMULTANEO. En
// Modo Degradado no hay radio: cada unidad decide su luz por su cuenta, y lo unico que
// comparten es la hora. Si las dos se creyeran con derecho a verde en el mismo
// segundo, el cruce queda abierto por los dos lados. Ademas es pura -sin millis(), sin
// pines, sin radio-, que es la propiedad que la hace compilable en el PC.
//
// SE BARRE EL DIA ENTERO, NO UNA MUESTRA. Los fallos de aritmetica circular viven en
// los bordes -medianoche, el salto del modulo-, que es justo lo que un muestreo se
// salta. 86.400 iteraciones por configuracion cuestan milisegundos aqui.

#include <stdio.h>
#include <stdint.h>
#include <cstdlib>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

// El fichero REAL del firmware. No una copia.
#include "ciclo_degradado.h"

// ---------------------------------------------------------------------------
// LA CONFIGURACION "REAL DEL FIRMWARE" SE RELEE DE modo_degradado.cpp, NO SE
// ESCRIBE A MANO. Mismo patron que Validacion_Automatico/arnes_automatico.cpp:
// dirDeEsteArchivo() para no depender del directorio de trabajo, y ABORTAR sin
// valor por defecto si el patron no aparece (CLAUDE.md 4 y 5). Antes esta tabla
// llevaba `{ 30, 30, ... }` escrito a mano: cuando la constante subio a 180 el
// literal se habria quedado mintiendo en silencio si nadie tocaba este fichero.
// ---------------------------------------------------------------------------
static std::string dirDeEsteArchivo() {
  std::string f = __FILE__;
  size_t p = f.find_last_of("/\\");
  return (p == std::string::npos) ? std::string(".") : f.substr(0, p);
}
static const std::string MAESTRO_SRC = dirDeEsteArchivo() + "/../Maestro/src/";

static void abortar(const std::string& motivo) {
  std::fprintf(stdout, "\n[ABORTADO] %s\n", motivo.c_str());
  std::fprintf(stdout,
      "Sin esa constante el arnes mediria otra cosa que el firmware, y seguiria\n"
      "dando un veredicto aunque ya no describa el C++ real. Regla del banco:\n"
      "sin valor por defecto, nunca.\n");
  std::exit(2);
}

// La primera aparicion del patron en modo_degradado.cpp. ABORTA si no aparece.
static long leerConstante(const std::string& patron, const std::string& que) {
  std::string ruta = MAESTRO_SRC + "modo_degradado.cpp";
  std::ifstream f(ruta.c_str());
  if (!f) abortar("no se pudo abrir el fuente real " + ruta);
  std::ostringstream ss;
  ss << f.rdbuf();
  std::string txt = ss.str();
  std::regex re(patron);
  std::smatch m;
  if (!std::regex_search(txt, m, re)) {
    abortar("no se pudo leer del C++ real la constante de " + que +
            " (patron no encontrado en modo_degradado.cpp)");
  }
  return std::strtol(m[1].str().c_str(), nullptr, 10);
}

static int total = 0, fallos = 0;

// D-53 (05/10): ROJO+AMARILLO de RA_S segundos tras el despeje y antes de CADA verde: OCHO
// fases, con la posicion 0 = R+A del Poste 1 (SPEC_2 8 (e.ter)). El valor sale de la
// decision. Las fases nuevas NO se nombran: un firmware sin ellas no compilaria este arnes
// (ABORTADO en vez de FALLA). Se reconocen por EXCLUSION: lo que no es ninguna de las seis
// fases de D-45 es un R+A, y se exige que tenga la forma de uno.
static const uint32_t RA_S = 2;
static bool esConocida(FaseDegradado f) {
  return f == FD_VERDE_MAESTRO || f == FD_AMARILLO_MAESTRO || f == FD_DESPEJE_A ||
         f == FD_VERDE_ESCLAVO || f == FD_AMARILLO_ESCLAVO || f == FD_DESPEJE_B;
}

static void comprobar(bool ok, const char* que) {
  total++;
  if (ok) {
    printf("   [OK]    %s\n", que);
  } else {
    fallos++;
    printf("   [FALLA] %s\n", que);
  }
}

// Configuraciones a barrer. La REAL del firmware se antepone en main(), releida de
// modo_degradado.cpp; estas son ademas, sinteticas, para forzar ciclos que NO dividen a
// 86.400, que es donde muerde el salto de medianoche: 86400 %% 120 == 0 no prueba nada
// sobre un ciclo de 134 s.
struct Config { uint16_t verde, despeje; const char* porque; };
static const Config CONFIGS_EXTRA[] = {
  {  30,  37, "ciclo 134 s: 86400 %% 134 = 44, no divide" },
  {  45,  20, "ciclo 130 s: 86400 %% 130 = 20, no divide" },
  {  17,  11, "ciclo  56 s: 86400 %%  56 = 32, no divide" },
  { 120,  30, "ciclo 300 s: divide exacto, el caso comodo" },
  {   7,   3, "ciclo  20 s, muy corto" },
  { 255, 255, "el tope del byte" },
  { 180, 180, "C2: al salir del tramo inicial al verde le quedan RA_S s" },
  { 180, 181, "C2: al salir del tramo inicial al verde le queda 1 s" },
};
static const int N_CONFIGS_EXTRA = sizeof(CONFIGS_EXTRA) / sizeof(CONFIGS_EXTRA[0]);

int main() {
  printf("==============================================================\n");
  printf(" ARNES DEL CICLO DEGRADADO - ciclo_degradado.h REAL, en el PC\n");
  printf("==============================================================\n");

  // La primera configuracion del barrido ES la real del firmware, releida de
  // modo_degradado.cpp en cada corrida (CLAUDE.md 4): ni un numero escrito a mano, para
  // que esta etiqueta no pueda mentir en silencio el dia que alguien mueva la constante.
  const uint16_t verdeReal = (uint16_t)leerConstante(
      R"(DEG_VERDE_SEG\s*=\s*(\d+))", "el verde del ciclo degradado (DEG_VERDE_SEG)");
  const uint16_t despejeReal = (uint16_t)leerConstante(
      R"(DEG_DESPEJE_SEG\s*=\s*(\d+))", "el despeje del ciclo degradado (DEG_DESPEJE_SEG)");
  char porqueReal[128];
  snprintf(porqueReal, sizeof(porqueReal),
           "la real del firmware, releida de modo_degradado.cpp "
           "(DEG_VERDE_SEG=%u / DEG_DESPEJE_SEG=%u)",
           verdeReal, despejeReal);

  std::vector<Config> configs;
  configs.push_back(Config{ verdeReal, despejeReal, porqueReal });
  for (int i = 0; i < N_CONFIGS_EXTRA; i++) configs.push_back(CONFIGS_EXTRA[i]);

  for (size_t c = 0; c < configs.size(); c++) {
    const uint16_t v = configs[c].verde, d = configs[c].despeje;
    printf("\n-- verde=%u despeje=%u  (%s)\n", v, d, configs[c].porque);

    // 1. LA PROPIEDAD QUE IMPORTA: nunca se pasa de un verde al otro sin todo-rojo.
    //    Un verde que sucede a otro verde sin cierre deja el cruce abierto por los
    //    dos lados durante el instante de la transicion.
    // D-45: EL ORDEN, no solo "nunca verde a verde". Todo verde acaba en el amarillo de SU
    // punta, que dura AMARILLO_SEG enteros y acaba en despeje; ningun amarillo sin su verde
    // delante. El viejo "de verde se pasa a despeje" es justo lo que esto hace fallar.
    long verde_a_verde = 0;
    uint32_t tAmarillo = 0;
    // D-53: la apertura. Todo verde viene de un R+A de RA_S s que viene de un despeje; un R+A
    // solo sale a SU verde; y el R+A de cada punta es siempre la MISMA fase, distinta de la
    // de la otra (si no, una punta no sabria si el R+A es suyo).
    long apertura_mala = 0, aperturas = 0;
    uint32_t tRA = 0, sRA = 0;
    int raDeM = -1, raDeE = -1;
    FaseDegradado ant = ciclo_degradado_fase(0, v, d);
    for (uint32_t s = 1; s < SEGUNDOS_DEL_DIA; s++) {
      FaseDegradado f = ciclo_degradado_fase(s, v, d);
      if (f != ant) {
        const bool aVerde = (f == FD_VERDE_MAESTRO || f == FD_VERDE_ESCLAVO);
        if (!esConocida(f)) {
          if (ant != FD_DESPEJE_A && ant != FD_DESPEJE_B) apertura_mala++;
          tRA = 0;
        }
        if (!esConocida(f)) sRA = s;
        // SPEC_2 8 (e.ter): a la salida de la guarda de medianoche la fase puede saltar a
        // mitad de un verde o de un R+A; ahi el R+A entero lo pone el SEMAFORO, no la fase.
        // Solo en s == despeje; en cualquier otro segundo se exige el R+A entero.
        const bool entradaDelDia = (s == (uint32_t)d) || (!esConocida(ant) && sRA == (uint32_t)d);
        if (aVerde && entradaDelDia) {
          aperturas++;
        } else if (aVerde) {
          int& suyo = (f == FD_VERDE_MAESTRO) ? raDeM : raDeE;
          if (esConocida(ant) || tRA != RA_S) apertura_mala++;
          else if (suyo < 0) suyo = (int)ant;
          else if (suyo != (int)ant) apertura_mala++;
          aperturas++;
        }
        if (!esConocida(ant) && !aVerde) apertura_mala++;   // un R+A sin su verde
        const bool deVerde = (ant == FD_VERDE_MAESTRO || ant == FD_VERDE_ESCLAVO);
        const bool aAmarillo = (f == FD_AMARILLO_MAESTRO || f == FD_AMARILLO_ESCLAVO);
        const bool deAmarillo = (ant == FD_AMARILLO_MAESTRO || ant == FD_AMARILLO_ESCLAVO);
        if (deVerde && f != (ant == FD_VERDE_MAESTRO ? FD_AMARILLO_MAESTRO : FD_AMARILLO_ESCLAVO))
          verde_a_verde++;
        if (aAmarillo && ant != (f == FD_AMARILLO_MAESTRO ? FD_VERDE_MAESTRO : FD_VERDE_ESCLAVO))
          verde_a_verde++;
        if (deAmarillo && ((f != FD_DESPEJE_A && f != FD_DESPEJE_B) || tAmarillo != AMARILLO_SEG))
          verde_a_verde++;
        tAmarillo = 0;
        ant = f;
      }
      if (f == FD_AMARILLO_MAESTRO || f == FD_AMARILLO_ESCLAVO) tAmarillo++;
      if (!esConocida(f)) tRA++;
    }
    char msg[400];
    snprintf(msg, sizeof(msg),
             "las 86.400 posiciones del dia en ORDEN: verde -> amarillo de su punta (%lu s) "
             "-> despeje, sin amarillo huerfano (transiciones malas: %ld)",
             (unsigned long)AMARILLO_SEG, verde_a_verde);
    comprobar(verde_a_verde == 0, msg);
    snprintf(msg, sizeof(msg),
             "D-53: en las 86.400 posiciones, los %ld verdes abren tras un ROJO+AMARILLO de %lu s "
             "que sigue a un despeje, cada punta con SU fase R+A, y ningun R+A sin su verde "
             "(aperturas malas: %ld)", aperturas, (unsigned long)RA_S, apertura_mala);
    comprobar(aperturas > 0 && apertura_mala == 0 && raDeM >= 0 && raDeE >= 0 && raDeM != raDeE,
              msg);

    // D-53 (C2, SPEC_2 8 (e.ter)): en R+A y verde la punta llama a forzarVerde() y el SEMAFORO
    // pone RA_S s de R+A antes del verde. Cada racha de una punta en {su R+A, su verde} dura
    // pues al menos RA_S + 1 s, INCLUIDA la salida del tramo inicial (s == despeje), que la
    // fila anterior exime: si no, la luz da R+A -> rojo, o un verde de milisegundos.
    {
      long rachas = 0, cortas = 0;
      // Los R+A sin nombre (ver esConocida): el de la posicion 0 y el de tras el despeje A.
      const FaseDegradado raP[2] = { ciclo_degradado_faseCruda(0, v, d),
          ciclo_degradado_faseCruda(RA_S + v + AMARILLO_SEG + d, v, d) };
      const FaseDegradado veP[2] = { FD_VERDE_MAESTRO, FD_VERDE_ESCLAVO };
      for (int p = 0; p < 2; p++) {
        uint32_t run = 0;
        for (uint32_t s = 0; s < SEGUNDOS_DEL_DIA; s++) {
          const FaseDegradado f = ciclo_degradado_fase(s, v, d);
          if (f == raP[p] || f == veP[p]) { run++; continue; }
          if (run > 0) { rachas++; if (run < RA_S + 1) cortas++; }
          run = 0;
        }
      }
      snprintf(msg, sizeof(msg),
               "D-53 C2: las %ld rachas R+A+verde de cada punta dan >= %lu s, todo R+A seguido de "
               ">= 1 s de verde, tambien a la salida del tramo inicial (cortas: %ld)",
               rachas, (unsigned long)(RA_S + 1), cortas);
      comprobar(rachas > 0 && cortas == 0, msg);
    }

    // D-53: LA FORMA DEL CICLO, sobre la fase CRUDA: 8 fases y posicion 0 = R+A del Poste 1.
    {
      const uint32_t ciclo = 2UL * ((uint32_t)v + RA_S + AMARILLO_SEG + (uint32_t)d);
      uint32_t nVM = 0, nVE = 0, nAM = 0, nAE = 0, nDA = 0, nDB = 0, nRA = 0;
      for (uint32_t p = 0; p < ciclo; p++) {
        const FaseDegradado f = ciclo_degradado_faseCruda(p, v, d);
        if (f == FD_VERDE_MAESTRO) nVM++; else if (f == FD_VERDE_ESCLAVO) nVE++;
        else if (f == FD_AMARILLO_MAESTRO) nAM++; else if (f == FD_AMARILLO_ESCLAVO) nAE++;
        else if (f == FD_DESPEJE_A) nDA++; else if (f == FD_DESPEJE_B) nDB++;
        else nRA++;
      }
      const bool pos0 = !esConocida(ciclo_degradado_faseCruda(0, v, d)) &&
                        ciclo_degradado_faseCruda(RA_S, v, d) == FD_VERDE_MAESTRO;
      snprintf(msg, sizeof(msg),
               "D-53: ciclo de %lu s = 2 x (verde + R+A + amarillo + despeje), posicion 0 = R+A "
               "del Poste 1 (%d); segundos por fase VM/VE %lu/%lu, AM/AE %lu/%lu, DA/DB %lu/%lu, "
               "R+A %lu", (unsigned long)ciclo, (int)pos0, (unsigned long)nVM, (unsigned long)nVE,
               (unsigned long)nAM, (unsigned long)nAE, (unsigned long)nDA, (unsigned long)nDB,
               (unsigned long)nRA);
      comprobar(pos0 && nVM == v && nVE == v && nAM == AMARILLO_SEG && nAE == AMARILLO_SEG &&
                nDA == d && nDB == d && nRA == 2UL * RA_S, msg);
    }

    // 3. La guarda de medianoche, en los dos sentidos. El dia no dura un numero
    //    entero de ciclos, asi que el ultimo ciclo antes de las 00:00 queda cortado:
    //    sin esta guarda, un verde podria empezar a las 23:59:5x y morir a medianoche
    //    dejando a la otra punta creyendo que todavia es su turno.
    bool borde_ok = true;
    for (uint32_t s = 0; s < (uint32_t)d && borde_ok; s++)
      if (ciclo_degradado_fase(s, v, d) != FD_DESPEJE_B) borde_ok = false;
    for (uint32_t s = SEGUNDOS_DEL_DIA - d; s < SEGUNDOS_DEL_DIA && borde_ok; s++)
      if (ciclo_degradado_fase(s, v, d) != FD_DESPEJE_B) borde_ok = false;
    comprobar(borde_ok,
              "la frontera de medianoche esta en todo-rojo por los DOS lados: ningun "
              "verde queda cortado por el cambio de dia");

    // 4. ciclo_degradado_restante() concuerda con la fase: el numero que se pinta en
    //    pantalla tiene que ser el que de verdad falta. Si mintiera, el operario veria
    //    una cuenta atras que no corresponde a lo que van a hacer las luces.
    long restante_malo = 0;
    for (uint32_t s = 0; s < SEGUNDOS_DEL_DIA; s += 7) {   // paso primo: no se alinea
      uint32_t r = ciclo_degradado_restante(s, v, d);
      if (r == 0) continue;
      FaseDegradado ahora = ciclo_degradado_fase(s, v, d);
      FaseDegradado antes = ciclo_degradado_fase((s + r - 1) % SEGUNDOS_DEL_DIA, v, d);
      FaseDegradado justo = ciclo_degradado_fase((s + r) % SEGUNDOS_DEL_DIA, v, d);
      if (antes != ahora || justo == ahora) restante_malo++;
    }
    snprintf(msg, sizeof(msg),
             "la cuenta atras cae EXACTAMENTE en el cambio de fase, ni antes ni "
             "despues (desajustes: %ld)", restante_malo);
    comprobar(restante_malo == 0, msg);
  }

  // CONTROL NEGATIVO. Una comprobacion que aprueba todo no comprueba nada: se exige
  // que el barrido SEPA detectar un ciclo roto. Con despeje = 0 la funcion devuelve
  // todo-rojo permanente -su respuesta segura- y por tanto NO puede haber verdes.
  printf("\n-- control negativo\n");
  bool hay_verde_con_despeje_cero = false;
  for (uint32_t s = 0; s < SEGUNDOS_DEL_DIA && !hay_verde_con_despeje_cero; s++) {
    FaseDegradado f = ciclo_degradado_fase(s, 30, 0);
    if (f == FD_VERDE_MAESTRO || f == FD_VERDE_ESCLAVO) hay_verde_con_despeje_cero = true;
  }
  comprobar(!hay_verde_con_despeje_cero,
            "con despeje=0 la funcion NO da un solo verde en todo el dia: una "
            "configuracion imposible cae al lado seguro, no al comodo");

  printf("\n==============================================================\n");
  printf(" RESULTADO: %d/%d comprobaciones OK\n", total - fallos, total);
  printf("==============================================================\n");
  printf(" Medido sobre el ciclo_degradado.h REAL del firmware, no sobre un\n");
  printf(" espejo en Python. Si alguien cambia el calculo, esto mide el nuevo.\n");
  // --- FUERA DEL BUCLE DE CONFIGURACIONES, A PROPOSITO ---
  // No depende de v ni de d: sus argumentos son (0,0) fijos. Dentro del bucle
  // contaba una vez por configuracion, o sea N veces la MISMA propiedad, que es
  // inflar la cuenta igual que hacia la tautologia que sustituye.
  {
    char msg[260];
  // 2. LA CONFIGURACION IMPOSIBLE DE VERDAD: verde=0 Y despeje=0.
  //
  // Aqui vivia una comprobacion que NO PODIA FALLAR y lo parecia:
  //
  //     if (f == FD_VERDE_MAESTRO && f == FD_VERDE_ESCLAVO) simultaneos++;
  //
  // `f` es UN valor del enum: no puede ser los dos, asi que `simultaneos` valia 0
  // pasara lo que pasara, mientras el mensaje afirmaba "ningun segundo del dia da
  // verde a las DOS puntas" — una propiedad de clase SFTY-2. Prueba muerta dentro
  // de una barrera de seguridad. Su propio comentario decia "se comprueba por
  // construccion", y eso es lo que la descalifica (CLAUDE.md §3).
  //
  // Que ese verde simultaneo no ocurre es cierto POR EL TIPO DE RETORNO. **El
  // riesgo real vive en otro sitio**: en como cada punta TRADUCE la fase a luz, en
  // el aplicarLuz() de su modo_degradado.cpp. Ahi si pueden divergir, y hace falta
  // un pack que compare las dos puntas: este arnes compila la funcion pura y no
  // puede verlo. Queda dicho para que la ausencia no se lea como cobertura.
  //
  // Lo que se pone en su lugar es el unico caso que no ejercia nadie. La guarda es
  // `if (verdeSeg == 0 || despejeSeg == 0)`, y de sus dos mitades:
  //   - `despeje == 0` SI es portante: sin ella, pos < v da VERDE_MAESTRO y
  //     pos < 2v da VERDE_ESCLAVO sin todo-rojo en medio. Ya la ejerce la config
  //     de despeje=0 de la prueba 4.
  //   - `verde == 0` con despeje valido NO es portante para el verde: el ciclo
  //     vale 2d, y la aritmetica no devuelve verde aunque se quite la guarda.
  //     Se comprobo inyectandolo: la cuenta no bajaba.
  // La combinacion que si importa es **las dos a cero**: `ciclo` vale 0 y
  // `segDia % ciclo` es una division por cero. Nadie la ejercia.
  //
  // ALCANCE, MEDIDO Y NO SUPUESTO. Al escribir esto se predijo que retirar la
  // guarda mataria el arnes por division por cero -o sea, ABORTADO y no FALLA-.
  // **Se inyecto y la prediccion era falsa**: la comprobacion cae limpia,
  //
  //     [FALLA] ... (verdes: 86400)
  //
  // asi que es un detector de FALLA, que es lo que se queria. Queda escrito el
  // error porque la prediccion llego a estar en este comentario: en este
  // repositorio lo que uno afirma tambien es un instrumento, y una suposicion con
  // aspecto de medida es justo lo que §4 castiga.
  long verdes_config_nula = 0;
  for (uint32_t s = 0; s < SEGUNDOS_DEL_DIA; s++) {
    FaseDegradado f = ciclo_degradado_fase(s, 0, 0);
    if (f == FD_VERDE_MAESTRO || f == FD_VERDE_ESCLAVO) verdes_config_nula++;
  }
  snprintf(msg, sizeof(msg),
           "verde=0 Y despeje=0 -la unica combinacion que hace ciclo=0- devuelve "
           "todo-rojo en los 86.400 segundos, sin dividir por cero (verdes: %ld)",
           verdes_config_nula);
  comprobar(verdes_config_nula == 0, msg);
  }

  return fallos == 0 ? 0 : 1;
}
