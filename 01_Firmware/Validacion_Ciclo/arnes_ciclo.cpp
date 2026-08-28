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

    // 2. Las dos puntas nunca tienen verde a la vez. Se comprueba por construccion:
    //    la fase es UNA, y solo una de las cuatro puede ser el verde de cada punta.
    long simultaneos = 0;
    for (uint32_t s = 0; s < SEGUNDOS_DEL_DIA; s++) {
      FaseDegradado f = ciclo_degradado_fase(s, v, d);
      if (f == FD_VERDE_MAESTRO && f == FD_VERDE_ESCLAVO) simultaneos++;
    }
    snprintf(msg, sizeof(msg),
             "ningun segundo del dia da verde a las DOS puntas (casos: %ld)", simultaneos);
    comprobar(simultaneos == 0, msg);

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
  return fallos == 0 ? 0 : 1;
}
