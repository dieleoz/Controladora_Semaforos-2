// ===== include/ciclo_degradado.h =====
#pragma once
#include <Arduino.h>

// ---------------------------------------------------------------------------
// SFTY-21 — Calculo de la fase del Modo Degradado.
//
// ESTE FICHERO DEBE SER IDENTICO EN MAESTRO Y ESCLAVO.
//
// En Modo Degradado no hay radio: cada unidad decide su luz por su cuenta. Para
// que vayan en fase, las dos tienen que calcular EXACTAMENTE lo mismo a partir de
// lo unico que comparten, que es la hora. Por eso el calculo vive aqui, en una
// funcion compartida, y no reescrito en cada proyecto: dos implementaciones que
// "hacen lo mismo" es como se acaba con verde en las dos puntas.
//
// El ciclo se ancla a la HORA DE PARED, no a un contador propio:
//
//     posicion = segundos_del_dia  mod  duracion_del_ciclo
//
// Anclarlo a un contador local haria que dos equipos encendidos con un minuto de
// diferencia arrancaran el ciclo desfasados un minuto entero.
// ---------------------------------------------------------------------------

// D-53 (05/10): OCHO FASES. El rojo+amarillo va DESPUES del despeje y antes de cada verde,
// y se ANADE: ciclo = 2 x (ROJO_AMARILLO_SEG + verde + AMARILLO_SEG + despeje); la posicion
// 0 es el R+A del Maestro. En el R+A y en el verde la punta llama a forzarVerde() y el
// semaforo pone el R+A y el verde (SPEC_2 8 (e.ter)). Firmware mixto entre postes: desfase.
#include "protocolo.h"   // AMARILLO_SEG, ROJO_AMARILLO_SEG

enum FaseDegradado {
  FD_ROJO_AMARILLO_MAESTRO,  // D-53: Maestro rojo+amarillo, Esclavo rojo
  FD_VERDE_MAESTRO,     // Maestro verde, Esclavo rojo
  FD_AMARILLO_MAESTRO,  // D-45: Maestro amarillo de cierre, Esclavo rojo
  FD_DESPEJE_A,         // todo-rojo tras el verde del Maestro
  FD_ROJO_AMARILLO_ESCLAVO,  // D-53: Esclavo rojo+amarillo, Maestro rojo
  FD_VERDE_ESCLAVO,     // Esclavo verde, Maestro rojo
  FD_AMARILLO_ESCLAVO,  // D-45: Esclavo amarillo de cierre, Maestro rojo
  FD_DESPEJE_B          // todo-rojo tras el verde del Esclavo
};

static const uint32_t SEGUNDOS_DEL_DIA = 86400UL;

// La fase de una POSICION del ciclo, sin guardas. pos < 2 x (r + verde + amarillo + despeje).
inline FaseDegradado ciclo_degradado_faseCruda(uint32_t pos, uint16_t verdeSeg,
                                               uint16_t despejeSeg) {
  const uint32_t r = ROJO_AMARILLO_SEG, v = verdeSeg, a = AMARILLO_SEG, d = despejeSeg;
  if (pos < r) return FD_ROJO_AMARILLO_MAESTRO;
  if (pos < r + v) return FD_VERDE_MAESTRO;
  if (pos < r + v + a) return FD_AMARILLO_MAESTRO;
  if (pos < r + v + a + d) return FD_DESPEJE_A;
  if (pos < 2UL * r + v + a + d) return FD_ROJO_AMARILLO_ESCLAVO;
  if (pos < 2UL * (r + v) + a + d) return FD_VERDE_ESCLAVO;
  if (pos < 2UL * (r + v + a) + d) return FD_AMARILLO_ESCLAVO;
  return FD_DESPEJE_B;
}

// Devuelve la fase que corresponde al instante dado.
//
// verdeSeg   duracion de CADA verde
// despejeSeg duracion de CADA todo-rojo (en Degradado va AMPLIADO: es el margen
//            que absorbe la deriva entre los dos relojes)
//
// EL SALTO DE MEDIANOCHE
// ----------------------
// A las 00:00:00 los segundos del dia vuelven a 0. Si la duracion del ciclo no divide
// exactamente a 86400 -y casi nunca lo hara-, la posicion salta. Las dos unidades saltan
// igual y a la vez, asi que NO se desincronizan; el problema es que ese salto puede caer en
// mitad de un verde y SALTARSE EL DESPEJE. Por eso el ultimo tramo del dia y el primero del
// siguiente son siempre despeje: cuesta un ciclo al dia.
//
// D-45: Y UN VERDE QUE ESA GUARDA CORTA PASA ANTES POR SU AMARILLO. En los AMARILLO_SEG
// previos al tramo final, la punta que estaba en verde al empezar esa ventana esta en su
// amarillo; un verde que empezaria dentro de la ventana no se enciende. Y un amarillo solo
// se da detras de un verde encendido: el que caeria tras el tramo inicial sin verde delante
// sale como despeje. "Venia de verde" se CALCULA sobre la hora de pared, no se recuerda.
inline FaseDegradado ciclo_degradado_fase(uint32_t segDia, uint16_t verdeSeg,
                                          uint16_t despejeSeg) {
  // Configuracion imposible: sin verde no hay ciclo que calcular. Todo-rojo es la
  // respuesta segura, no un caso que "no deberia pasar".
  if (verdeSeg == 0 || despejeSeg == 0) return FD_DESPEJE_A;

  const uint32_t a = AMARILLO_SEG, r = ROJO_AMARILLO_SEG;
  const uint32_t ciclo = 2UL * (r + (uint32_t)verdeSeg + a + (uint32_t)despejeSeg);
  const uint32_t finDia = SEGUNDOS_DEL_DIA - despejeSeg;   // empieza el tramo final

  // Guarda de medianoche, en los dos sentidos de la frontera.
  if (segDia < despejeSeg) return FD_DESPEJE_B;
  if (segDia >= finDia) return FD_DESPEJE_B;

  const uint32_t pos = segDia % ciclo;
  const FaseDegradado f = ciclo_degradado_faseCruda(pos, verdeSeg, despejeSeg);

  // D-53 (C2): a la salida del tramo inicial el SEMAFORO pone r s de R+A antes del verde. Si
  // al verde le quedan <= r s, la luz seria R+A -> rojo: ese R+A sale como despeje, y su
  // amarillo tambien (no tiene verde delante). SPEC_2 8 (e.ter).
  if (f == FD_ROJO_AMARILLO_MAESTRO || f == FD_VERDE_MAESTRO ||
      f == FD_ROJO_AMARILLO_ESCLAVO || f == FD_VERDE_ESCLAVO) {
    const bool deM = (f == FD_ROJO_AMARILLO_MAESTRO || f == FD_VERDE_MAESTRO);
    const uint32_t finVerde = deM ? r + verdeSeg : 2UL * (r + verdeSeg) + a + despejeSeg;
    if (segDia + (finVerde - pos) <= (uint32_t)despejeSeg + r) return FD_DESPEJE_B;
  }

  // Un amarillo cuyo verde no llego a encenderse tras el tramo inicial no tiene verde delante.
  if (f == FD_AMARILLO_MAESTRO || f == FD_AMARILLO_ESCLAVO) {
    const uint32_t inicio = (f == FD_AMARILLO_MAESTRO) ? r + verdeSeg
                                                       : 2UL * (r + verdeSeg) + a + despejeSeg;
    if (segDia - (pos - inicio) <= (uint32_t)despejeSeg + r) {
      return (f == FD_AMARILLO_MAESTRO) ? FD_DESPEJE_A : FD_DESPEJE_B;
    }
  }

  // D-53: un rojo+amarillo cuyo verde empezaria dentro de la ventana final (y no se
  // encenderia) sale como despeje: el R+A anuncia un verde, y solo uno que va a abrir.
  if (f == FD_ROJO_AMARILLO_MAESTRO || f == FD_ROJO_AMARILLO_ESCLAVO) {
    const uint32_t finRA = (f == FD_ROJO_AMARILLO_MAESTRO) ? r
                                                           : 2UL * r + verdeSeg + a + despejeSeg;
    if (segDia + (finRA - pos) + a >= finDia) {
      return (f == FD_ROJO_AMARILLO_MAESTRO) ? FD_DESPEJE_B : FD_DESPEJE_A;
    }
  }

  // La ventana de AMARILLO_SEG antes del tramo final.
  if (segDia + a >= finDia) {
    const FaseDegradado f0 = ciclo_degradado_faseCruda((finDia - a) % ciclo, verdeSeg,
                                                       despejeSeg);
    // D-53: un verde que EMPIEZA justo en la ventana no se encendio: sin amarillo de cierre.
    const bool f0Encendido =
        (f0 == ciclo_degradado_faseCruda((finDia - a - 1) % ciclo, verdeSeg, despejeSeg));
    if (f0Encendido && f0 == FD_VERDE_MAESTRO) return FD_AMARILLO_MAESTRO;
    if (f0Encendido && f0 == FD_VERDE_ESCLAVO) return FD_AMARILLO_ESCLAVO;
    if (f == FD_VERDE_MAESTRO) return FD_DESPEJE_B;
    if (f == FD_VERDE_ESCLAVO) return FD_DESPEJE_A;
    if (f != f0) return (f == FD_AMARILLO_MAESTRO) ? FD_DESPEJE_A : FD_DESPEJE_B;
  }
  return f;
}

// Segundos que faltan para el siguiente cambio de fase. Sirve para mostrar la
// cuenta atras en pantalla sin recalcular el ciclo en cada modulo.
inline uint32_t ciclo_degradado_restante(uint32_t segDia, uint16_t verdeSeg,
                                         uint16_t despejeSeg) {
  if (verdeSeg == 0 || despejeSeg == 0) return 0;
  const FaseDegradado actual = ciclo_degradado_fase(segDia, verdeSeg, despejeSeg);
  uint32_t t = 0;
  // Busqueda hacia delante, acotada a un ciclo completo mas la guarda. Es lineal
  // pero se ejecuta una vez por segundo como mucho, y evita replicar aqui la
  // logica de fronteras -incluida la de medianoche-, que es justo donde estaria
  // el error si se calculara "a mano" por segunda vez.
  const uint32_t tope =
      2UL * (ROJO_AMARILLO_SEG + (uint32_t)verdeSeg + AMARILLO_SEG + despejeSeg) + despejeSeg +
      AMARILLO_SEG + ROJO_AMARILLO_SEG + 2UL;
  while (t < tope) {
    t++;
    uint32_t s = (segDia + t) % SEGUNDOS_DEL_DIA;
    if (ciclo_degradado_fase(s, verdeSeg, despejeSeg) != actual) return t;
  }
  return 0;
}
