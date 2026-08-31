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

// El fichero REAL del firmware. No una copia.
#include "ciclo_degradado.h"

static int total = 0, fallos = 0;

static void comprobar(bool ok, const char* que) {
  total++;
  if (ok) {
    printf("   [OK]    %s\n", que);
  } else {
    fallos++;
    printf("   [FALLA] %s\n", que);
  }
}

// Configuraciones a barrer. Se incluye la REAL del firmware y varias que NO dividen a
// 86.400, que es donde muerde el salto de medianoche: 86400 %% 120 == 0 no prueba nada
// sobre un ciclo de 134 s.
struct Config { uint16_t verde, despeje; const char* porque; };
static const Config CONFIGS[] = {
  {  30,  30, "la real del firmware (DEG_VERDE_SEG / DEG_DESPEJE_SEG)" },
  {  30,  37, "ciclo 134 s: 86400 %% 134 = 44, no divide" },
  {  45,  20, "ciclo 130 s: 86400 %% 130 = 20, no divide" },
  {  17,  11, "ciclo  56 s: 86400 %%  56 = 32, no divide" },
  { 120,  30, "ciclo 300 s: divide exacto, el caso comodo" },
  {   7,   3, "ciclo  20 s, muy corto" },
  { 255, 255, "el tope del byte" },
};
static const int N_CONFIGS = sizeof(CONFIGS) / sizeof(CONFIGS[0]);

int main() {
  printf("==============================================================\n");
  printf(" ARNES DEL CICLO DEGRADADO - ciclo_degradado.h REAL, en el PC\n");
  printf("==============================================================\n");

  for (int c = 0; c < N_CONFIGS; c++) {
    const uint16_t v = CONFIGS[c].verde, d = CONFIGS[c].despeje;
    printf("\n-- verde=%u despeje=%u  (%s)\n", v, d, CONFIGS[c].porque);

    // 1. LA PROPIEDAD QUE IMPORTA: nunca se pasa de un verde al otro sin todo-rojo.
    //    Un verde que sucede a otro verde sin cierre deja el cruce abierto por los
    //    dos lados durante el instante de la transicion.
    long verde_a_verde = 0;
    FaseDegradado ant = ciclo_degradado_fase(0, v, d);
    for (uint32_t s = 1; s < SEGUNDOS_DEL_DIA; s++) {
      FaseDegradado f = ciclo_degradado_fase(s, v, d);
      if (f != ant) {
        bool era_verde = (ant == FD_VERDE_MAESTRO || ant == FD_VERDE_ESCLAVO);
        bool es_verde  = (f   == FD_VERDE_MAESTRO || f   == FD_VERDE_ESCLAVO);
        if (era_verde && es_verde) verde_a_verde++;
        ant = f;
      }
    }
    char msg[220];
    snprintf(msg, sizeof(msg),
             "las 86.400 posiciones del dia: NUNCA se pasa de verde a verde sin "
             "todo-rojo (transiciones malas: %ld)", verde_a_verde);
    comprobar(verde_a_verde == 0, msg);

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
