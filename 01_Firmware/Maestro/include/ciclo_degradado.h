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

enum FaseDegradado {
  FD_VERDE_MAESTRO,   // Maestro verde, Esclavo rojo
  FD_DESPEJE_A,       // todo-rojo tras el verde del Maestro
  FD_VERDE_ESCLAVO,   // Esclavo verde, Maestro rojo
  FD_DESPEJE_B        // todo-rojo tras el verde del Esclavo
};

static const uint32_t SEGUNDOS_DEL_DIA = 86400UL;

// Devuelve la fase que corresponde al instante dado.
//
// verdeSeg   duracion de CADA verde
// despejeSeg duracion de CADA todo-rojo (en Degradado va AMPLIADO: es el margen
//            que absorbe la deriva entre los dos relojes)
//
// EL SALTO DE MEDIANOCHE
// ----------------------
// A las 00:00:00 los segundos del dia vuelven a 0. Si la duracion del ciclo no
// divide exactamente a 86400 -y casi nunca lo hara-, la posicion salta:
//
//     23:59:5x  ->  posicion 137 de un ciclo de 150
//     00:00:00  ->  posicion 0
//
// Las dos unidades saltan igual y a la vez, asi que NO se desincronizan. El
// problema es otro y es peor: ese salto puede caer en mitad de un verde y
// SALTARSE EL DESPEJE, dando verde a la otra punta sin todo-rojo de por medio.
//
// Se resuelve forzando todo-rojo alrededor de la medianoche: el ultimo tramo del
// dia y el primero del siguiente son siempre despeje. Cuesta un ciclo al dia y
// garantiza que la frontera se cruce siempre con las dos puntas en rojo.
inline FaseDegradado ciclo_degradado_fase(uint32_t segDia, uint16_t verdeSeg,
                                          uint16_t despejeSeg) {
  // Configuracion imposible: sin verde no hay ciclo que calcular. Todo-rojo es la
  // respuesta segura, no un caso que "no deberia pasar".
  if (verdeSeg == 0 || despejeSeg == 0) return FD_DESPEJE_A;

  const uint32_t ciclo = 2UL * ((uint32_t)verdeSeg + (uint32_t)despejeSeg);

  // Guarda de medianoche, en los dos sentidos de la frontera.
  if (segDia < despejeSeg) return FD_DESPEJE_B;
  if (SEGUNDOS_DEL_DIA - segDia <= despejeSeg) return FD_DESPEJE_B;

  const uint32_t pos = segDia % ciclo;

  if (pos < (uint32_t)verdeSeg) return FD_VERDE_MAESTRO;
  if (pos < (uint32_t)verdeSeg + despejeSeg) return FD_DESPEJE_A;
  if (pos < 2UL * verdeSeg + despejeSeg) return FD_VERDE_ESCLAVO;
  return FD_DESPEJE_B;
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
  const uint32_t tope = 2UL * ((uint32_t)verdeSeg + despejeSeg) + despejeSeg + 2UL;
  while (t < tope) {
    t++;
    uint32_t s = (segDia + t) % SEGUNDOS_DEL_DIA;
    if (ciclo_degradado_fase(s, verdeSeg, despejeSeg) != actual) return t;
  }
  return 0;
}
