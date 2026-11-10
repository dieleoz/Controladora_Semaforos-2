// ===== dos_puntas/orquestador_espera.inc =====
//
// BLOQUE RE - D-175 (SPEC_8 8.4): EL ROJO DE ESPERA EN LAS DOS PUNTAS. El Maestro: coordinador.cpp y semaforo.cpp
// REALES, el MENU por la orden "forzar_menu" (lo que hace menu.cpp). El Esclavo: main.cpp y semaforo.cpp REALES; su
// espera la decide el despachador con cada trama de gobierno. El bit 0x01 de GO_RED (GO_RED_ESPERA) sale de SPEC_2 y
// de la decision, no del fuente. Lo incluye orquestador.cpp antes de main().

// LA TERCERA EXCEPCION POR NOMBRE de la invariante de la pluma (las otras: barrera fuera y veto/camara mala). Pluma
// arriba en rojo porque la punta anota la espera (semaforo_rojoEspera() REAL) Y el arnes la reconstruye por su
// cuenta: el Maestro, con su modo en MENU; el Esclavo, con un modelo de lo ENTREGADO desde su ultima carga: espera
// sin ninguna trama de gobierno, o si la ultima fue un GO_RED con el bit. Lo que el modelo no ve (GO_AMBAR, tramas
// inyectadas, el Degradado) lo deja en su valor y el AND con el getter REAL no excusa nada de mas.
static int g_esperaCargaE = -1;
static bool g_esperaModeloE = true;
static void esperaAlEntregar(const unsigned char* trama) {   // solo tramas hacia el Esclavo
  if (g_esperaCargaE != ESCLAVO.cargas) { g_esperaCargaE = ESCLAVO.cargas; g_esperaModeloE = true; }
  const uint8_t c = trama[1];
  if (c == CMD_PING_V || c == CMD_GO_RED_V || c == CMD_GO_GREEN_V || c == CMD_GO_NOCHE_V)
    g_esperaModeloE = (c == CMD_GO_RED_V && (trama[2] & 0x01) != 0);
}
static bool excepcionEsperaOrq(int i) {
  if (i == 0) return MAESTRO.orden("rojo_espera") == 1 && MAESTRO.orden("modo_actual") == 0;   // 0 = MENU
  if (g_esperaCargaE != ESCLAVO.cargas) { g_esperaCargaE = ESCLAVO.cargas; g_esperaModeloE = true; }
  return g_esperaModeloE && ESCLAVO.orden("rojo_espera") == 1;
}

static bool ePluma(Punta& p) { return p.pin(MOTOR_TALANQUERA) == TALANQUERA_ABRIR; }
// Avanza hasta que `cond` se cumpla (ms gastados) o -1 al tope.
template <typename C> static long eHasta(unsigned long tope, C cond) {
  for (unsigned long h = 0; h <= tope; h += PASO_MS) { if (cond()) return (long)h; unTick(); }
  return -1;
}
// La pluma de `p` sigue arriba en CADA tick durante `ms`.
static bool eSigueArriba(Punta& p, unsigned long ms) {
  bool ok = ePluma(p);
  for (unsigned long h = 0; h < ms; h += PASO_MS) { unTick(); ok = ok && ePluma(p); }
  return ok;
}

static void bloqueEspera() {
  std::printf("\n-- Bloque RE: el rojo de espera (D-175) en las dos puntas --\n");
  const unsigned long R = g_retardoPlumaMs, LAT = g_latenciaMs;
  char t[480];

  // -- RE1 (R, prueba 6): el Esclavo arranca sin oir NINGUNA orden (el Maestro calla, como en su ambar de arranque,
  // D-40): a R + 1 s sigue en S_ROJO con la pluma ARRIBA. Hoy bajaba a los R.
  escenarioLimpio(0);
  g_enlaceHaciaEsclavo = false;
  microcorte(ESCLAVO);
  const bool e1Rojo = ESCLAVO.estado() == S_ROJO_V;
  const bool e1 = eSigueArriba(ESCLAVO, R + 1000UL) && ESCLAVO.estado() == S_ROJO_V;
  std::snprintf(t, sizeof(t), "RE1 (D-175, prueba 6): Esclavo recien arrancado sin ninguna trama del Maestro: en "
                "S_ROJO (%d) con la pluma ARRIBA en cada tick durante R + 1 s = %lu ms (%d) y la espera anotada (%ld)",
                e1Rojo, R + 1000UL, e1, ESCLAVO.orden("rojo_espera"));
  comprobar(e1Rojo && e1 && ESCLAVO.orden("rojo_espera") == 1, t);
  // ... y la primera orden de ciclo (el PING del Automatico) la suelta: baja tras R.
  g_enlaceHaciaEsclavo = true;
  const long e1b = eHasta(LATIDO_MS_V + R + 1000UL, []() { return !ePluma(ESCLAVO); });
  std::snprintf(t, sizeof(t), "RE1b (D-175): vuelto el enlace, el primer PING del Maestro borra la espera y la pluma "
                "del Esclavo baja en <= LATIDO + R + 1 s = %lu ms (medido %ld; -1 = no bajo)",
                LATIDO_MS_V + R + 1000UL, e1b);
  comprobar(e1b >= (long)R && ESCLAVO.orden("rojo_espera") == 0, t);

  // -- RE2 (R): Maestro en MENU con las dos plumas ARRIBA (vienen del ambar de SFTY-6, enlace caido): con el enlace
  // de vuelta y la orden de MENU, las dos quedan en S_ROJO y a 2R siguen arriba. Hoy bajaban a los R.
  escenarioLimpio(0);
  g_enlaceHaciaEsclavo = g_enlaceHaciaMaestro = false;
  avanzar(35000UL);
  const bool e2Fallo = MAESTRO.estado() == S_FALLO_V && ESCLAVO.estado() == S_FALLO_V;
  g_enlaceHaciaEsclavo = g_enlaceHaciaMaestro = true;
  MAESTRO.orden("forzar_menu");
  const long e2r = eHasta(10000UL, []() { return MAESTRO.estado() == S_ROJO_V && ESCLAVO.estado() == S_ROJO_V; });
  bool e2 = true;
  for (unsigned long h = 0; h < 2UL * R; h += PASO_MS) { unTick(); e2 = e2 && ePluma(MAESTRO) && ePluma(ESCLAVO); }
  std::snprintf(t, sizeof(t), "RE2 (D-175): las dos en ambar (%d), MENU con enlace: las dos en S_ROJO (%ld ms) y las "
                "dos plumas ARRIBA en cada tick durante 2R = %lu ms (%d); espera M %ld, E %ld",
                e2Fallo, e2r, 2UL * R, e2, MAESTRO.orden("rojo_espera"), ESCLAVO.orden("rojo_espera"));
  comprobar(e2Fallo && e2r >= 0 && e2 && MAESTRO.estado() == S_ROJO_V && ESCLAVO.estado() == S_ROJO_V &&
            MAESTRO.orden("rojo_espera") == 1 && ESCLAVO.orden("rojo_espera") == 1, t);

  // -- RE4 (R): ROJO TOTAL desde MENU es rojo de accidente (D-173): las dos bajan tras R (el Esclavo, + un viaje).
  MAESTRO.orden("forzar_rojo_total");
  const unsigned long t0 = g_t;
  long bajaM = -1, bajaE = -1;
  for (unsigned long h = 0; h <= 3UL * R && (bajaM < 0 || bajaE < 0); h += PASO_MS) {
    if (bajaM < 0 && !ePluma(MAESTRO)) bajaM = (long)(g_t - t0);
    if (bajaE < 0 && !ePluma(ESCLAVO)) bajaE = (long)(g_t - t0);
    unTick();
  }
  std::snprintf(t, sizeof(t), "RE4 (D-175, D-173): ROJO TOTAL desde MENU: el Maestro baja a los R = %lu ms (%ld) y el "
                "Esclavo a los R + viaje (%ld; tope R + %lu)", R, bajaM, bajaE, LAT + 2UL * PASO_MS);
  comprobar(bajaM >= (long)(R - PASO_MS) && bajaM <= (long)(R + 2UL * PASO_MS) && bajaE >= (long)(R - PASO_MS) &&
            bajaE <= (long)(R + LAT + 2UL * PASO_MS), t);

  // -- RE6 (C): MENU con las dos plumas ya ABAJO: ninguna sube (la espera solo RETIENE, D-33).
  MAESTRO.orden("forzar_menu");
  bool e6 = !ePluma(MAESTRO) && !ePluma(ESCLAVO);
  for (unsigned long h = 0; h < 2UL * R; h += PASO_MS) { unTick(); e6 = e6 && !ePluma(MAESTRO) && !ePluma(ESCLAVO); }
  comprobar(e6 && MAESTRO.orden("rojo_espera") == 1 && ESCLAVO.orden("rojo_espera") == 1,
            "RE6 (D-175): MENU con las dos plumas abajo: las dos anotan la espera y ninguna pluma sube en 2R");

  // -- RE3 (C): de MENU al Automatico (sin coordinador_setup(), como main.cpp): las dos plumas estan abajo antes del
  // primer verde del otro poste. Se sale de un MENU con las dos ARRIBA (E2 otra vez, abreviado).
  escenarioLimpio(0);
  g_enlaceHaciaEsclavo = g_enlaceHaciaMaestro = false;
  avanzar(35000UL);
  g_enlaceHaciaEsclavo = g_enlaceHaciaMaestro = true;
  MAESTRO.orden("forzar_menu");
  avanzar(2UL * R + 5000UL);
  const bool e3Arriba = ePluma(MAESTRO) && ePluma(ESCLAVO);
  MAESTRO.orden("menu_a_automatico");
  int e3Malos = 0, e3Verdes = 0;
  for (unsigned long h = 0; h < 300000UL && e3Verdes < 2; h += PASO_MS) {
    const bool vM = MAESTRO.verde(), vE = ESCLAVO.verde();
    unTick();
    if (!vM && MAESTRO.verde()) { e3Verdes++; if (ePluma(ESCLAVO)) e3Malos++; }
    if (!vE && ESCLAVO.verde()) { e3Verdes++; if (ePluma(MAESTRO)) e3Malos++; }
  }
  std::snprintf(t, sizeof(t), "RE3 (D-175): de MENU con las dos arriba (%d) al Automatico: en los 2 primeros verdes "
                "(%d vistos) la pluma del OTRO poste ya estaba abajo (%d con ella arriba)", e3Arriba, e3Verdes,
                e3Malos);
  comprobar(e3Arriba && e3Verdes == 2 && e3Malos == 0, t);

  // -- RE5 (C): Maestro VIEJO (GO_RED sin el bit) que pasa a MENU con el Esclavo en verde: el Esclavo cierra y su
  // pluma baja a los R del rojo, como antes de D-175. Ninguna mezcla deja la pluma arriba en un rojo de ciclo.
  escenarioLimpio(0);
  const long e5v = eHasta(300000UL, []() { return ESCLAVO.verde(); });
  MAESTRO.orden("go_red_sin_bit", 1);
  MAESTRO.orden("forzar_menu");
  const long e5r = eHasta(10000UL, []() { return ESCLAVO.estado() == S_ROJO_V; });
  const long e5 = eHasta(3UL * R, []() { return !ePluma(ESCLAVO); });
  MAESTRO.orden("go_red_sin_bit", 0);
  std::snprintf(t, sizeof(t), "RE5 (D-175): GO_RED sin el bit en MENU (Maestro viejo) con el Esclavo en verde (%ld): "
                "rojo (%ld) y la pluma baja a los R = %lu ms (%ld); espera del Esclavo %ld", e5v, e5r, R, e5,
                ESCLAVO.orden("rojo_espera"));
  comprobar(e5v >= 0 && e5r >= 0 && e5 >= (long)(R - PASO_MS) && e5 <= (long)(R + 2UL * PASO_MS) &&
            ESCLAVO.orden("rojo_espera") == 0, t);

  // -- RE7 (C, F2): el Esclavo se reinicia durante el verde del Maestro: arranca en espera con la pluma arriba y la
  // suelta con el primer PING; baja antes de LATIDO + R + viaje + paso, sin perdidas.
  escenarioLimpio(0);
  const long e7v = eHasta(300000UL, []() { return MAESTRO.verde(); });
  microcorte(ESCLAVO);
  const long e7 = eHasta(LATIDO_MS_V + R + 5000UL, []() { return !ePluma(ESCLAVO); });
  const unsigned long cota7 = LATIDO_MS_V + R + LAT + 2UL * PASO_MS;
  std::snprintf(t, sizeof(t), "RE7 (D-175, F2): Esclavo reiniciado con el Maestro en verde (%ld): la pluma del "
                "Esclavo baja a los %ld ms del arranque (cota LATIDO + R + viaje + paso = %lu)", e7v, e7, cota7);
  comprobar(e7v >= 0 && e7 >= 0 && e7 <= (long)cota7 && MAESTRO.verde(), t);

  escenarioLimpio(0);   // nadie hereda la espera ni el enlace de este bloque
}
