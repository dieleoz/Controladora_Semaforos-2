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

// D-45 (02/10): SEIS FASES. El amarillo de cierre va entre cada verde y su despeje, y se
// ANADE: el verde y el despeje no se tocan. Ciclo = 2 x (verde + AMARILLO_SEG + despeje).
// Cada punta enciende su amarillo en la fase suya; la otra sigue en rojo. Las dos puntas
// tienen que llevar esta cabecera A LA VEZ: con ciclos de duracion distinta sobre la misma
// hora se desfasan en cada vuelta (SPEC_2 8 (e.bis)).
#include "protocolo.h"   // AMARILLO_SEG

enum FaseDegradado {
  FD_VERDE_MAESTRO,     // Maestro verde, Esclavo rojo
  FD_AMARILLO_MAESTRO,  // D-45: Maestro amarillo de cierre, Esclavo rojo
  FD_DESPEJE_A,         // todo-rojo tras el verde del Maestro
  FD_VERDE_ESCLAVO,     // Esclavo verde, Maestro rojo
  FD_AMARILLO_ESCLAVO,  // D-45: Esclavo amarillo de cierre, Maestro rojo
  FD_DESPEJE_B          // todo-rojo tras el verde del Esclavo
};

static const uint32_t SEGUNDOS_DEL_DIA = 86400UL;

// La fase de una POSICION del ciclo, sin guardas. pos < 2 x (verde + amarillo + despeje).
inline FaseDegradado ciclo_degradado_faseCruda(uint32_t pos, uint16_t verdeSeg,
                                               uint16_t despejeSeg) {
  const uint32_t v = verdeSeg, a = AMARILLO_SEG, d = despejeSeg;
  if (pos < v) return FD_VERDE_MAESTRO;
  if (pos < v + a) return FD_AMARILLO_MAESTRO;
  if (pos < v + a + d) return FD_DESPEJE_A;
  if (pos < 2UL * v + a + d) return FD_VERDE_ESCLAVO;
  if (pos < 2UL * (v + a) + d) return FD_AMARILLO_ESCLAVO;
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

  const uint32_t a = AMARILLO_SEG;
  const uint32_t ciclo = 2UL * ((uint32_t)verdeSeg + a + (uint32_t)despejeSeg);
  const uint32_t finDia = SEGUNDOS_DEL_DIA - despejeSeg;   // empieza el tramo final

  // Guarda de medianoche, en los dos sentidos de la frontera.
  if (segDia < despejeSeg) return FD_DESPEJE_B;
  if (segDia >= finDia) return FD_DESPEJE_B;

  const uint32_t pos = segDia % ciclo;
  const FaseDegradado f = ciclo_degradado_faseCruda(pos, verdeSeg, despejeSeg);

  // Un amarillo cuyo verde acabo dentro del tramo inicial no tiene verde delante.
  if (f == FD_AMARILLO_MAESTRO || f == FD_AMARILLO_ESCLAVO) {
    const uint32_t inicio = (f == FD_AMARILLO_MAESTRO) ? verdeSeg
                                                       : 2UL * verdeSeg + a + despejeSeg;
    if (segDia - (pos - inicio) <= despejeSeg) {
      return (f == FD_AMARILLO_MAESTRO) ? FD_DESPEJE_A : FD_DESPEJE_B;
    }
  }

  // La ventana de AMARILLO_SEG antes del tramo final.
  if (segDia + a >= finDia) {
    const FaseDegradado f0 = ciclo_degradado_faseCruda((finDia - a) % ciclo, verdeSeg,
                                                       despejeSeg);
    if (f0 == FD_VERDE_MAESTRO) return FD_AMARILLO_MAESTRO;
    if (f0 == FD_VERDE_ESCLAVO) return FD_AMARILLO_ESCLAVO;
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
      2UL * ((uint32_t)verdeSeg + AMARILLO_SEG + despejeSeg) + despejeSeg + AMARILLO_SEG + 2UL;
  while (t < tope) {
    t++;
    uint32_t s = (segDia + t) % SEGUNDOS_DEL_DIA;
    if (ciclo_degradado_fase(s, verdeSeg, despejeSeg) != actual) return t;
  }
  return 0;
}
