// ===== dos_puntas/orquestador_deg_espera.inc =====
//
// D-175 (SPEC_8 8.4): la excepcion por NOMBRE del rojo de espera en la invariante de la pluma de este arnes. El
// Maestro: anota la espera (getter REAL) Y su modo es MENU. El Esclavo: anota la espera Y no le ha llegado NINGUNA
// trama desde su ultima carga (mas estricto que "ninguna de gobierno": una de servicio quita la excusa y lo que
// quede arriba cuenta). La espera del Esclavo la borra su Degradado (main.cpp): desde ahi corren R y el veto. Lo
// que no ve: una trama entregada en el mismo tick de la carga, antes de la primera llamada.
// Lo incluye orquestador_degradado.cpp antes de vigilar().
static unsigned long g_entregasEsclavo = 0;   // la suma unTick() al entregar al Esclavo
static int g_esperaCargaD = -1;
static unsigned long g_esperaEntregasD = 0;
static bool excepcionEsperaDeg(int i) {
  if (i == 0) return MAESTRO.orden("rojo_espera") == 1 && MAESTRO.orden("modo_actual") == 0;   // 0 = MENU
  if (g_esperaCargaD != ESCLAVO.cargas) { g_esperaCargaD = ESCLAVO.cargas; g_esperaEntregasD = g_entregasEsclavo; }
  return g_entregasEsclavo == g_esperaEntregasD && ESCLAVO.orden("rojo_espera") == 1;
}
