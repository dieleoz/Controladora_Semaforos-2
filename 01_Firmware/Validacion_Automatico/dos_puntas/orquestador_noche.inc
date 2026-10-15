// ===== dos_puntas/orquestador_noche.inc =====
//
// BLOQUE N - N-179: EL MODO NOCHE (D-106..D-111, SPEC_0 5.10 (j)), sobre modo_noche.cpp,
// coordinador.cpp y semaforo.cpp del Maestro y el main.cpp/semaforo.cpp del Esclavo REALES.
// Lo incluye orquestador.cpp antes de main(); main() llama a bloqueNoche() tras el bloque H.
// Las ordenes de texto van a modoNoche_orden() por el adaptador (adaptador_maestro_noche.inc),
// sin el despachador: lo que se mide aqui es la noche, no bluetooth.cpp.
//
// El reloj: dias desde el 01/01/2000 = 7005, (7005 + 5) % 7 = 3 = jueves (mascara ---J---).

static const long N_DIA = 7005;
static const char* const N_PIN = "CMD:PIN:1234:";
static int N_MODO_AUTO = -1, N_MODO_MANUAL = -1, N_MODO_NOCHE = -1;
static unsigned long N_TR_FABRICA_MS = 0, N_REENVIO_MS = 0;

static long nHora(int h, int m, int s) { return N_DIA * 86400L + h * 3600L + m * 60L + s; }
static bool nBt(const std::string& o) { return MAESTRO.orden(("bt:" + o).c_str()) == 1; }
static bool nBtEs(const std::string& r) { return MAESTRO.orden(("bt_es:" + r).c_str()) == 1; }
static long nEv(const char* d) { return MAESTRO.orden((std::string("ev:NOCHE/") + d).c_str()); }
static long nModo() { return MAESTRO.orden("modo_actual"); }

static int nIndice(const std::string& ruta, const char* enumNombre, const char* item) {
  std::smatch m;
  const std::string txt = sinComentarios(leerFuente(ruta));
  if (!std::regex_search(txt, m, std::regex(std::string("enum\\s+") + enumNombre + R"(\s*\{([^}]*)\})")))
    abortar(std::string("no se encuentra enum ") + enumNombre + " en " + ruta);
  std::stringstream ss(m[1].str());
  std::string it;
  for (int i = 0; std::getline(ss, it, ','); i++) {
    std::smatch n;
    if (std::regex_search(it, n, std::regex(R"((\w+))")) && n[1].str() == item) return i;
  }
  abortar(std::string("no se encuentra ") + item + " en enum " + enumNombre);
  return -1;
}

// Corre hasta que cond() valga o se acabe el plazo; devuelve si valio.
template <class F> static bool nHasta(F cond, unsigned long maxMs) {
  for (unsigned long h = 0; h < maxMs; h += PASO_MS) { if (cond()) return true; unTick(); }
  return cond();
}

// AUTO en marcha, hora fiable a las hh:mm:ss del jueves y, si se pide, tr y una franja.
static void nEscenario(int h, int m, int s, int trSeg, const std::string& franja) {
  escenarioLimpio(0);
  MAESTRO.orden("hora_fiable", 1);
  MAESTRO.orden("noche_fecha", nHora(h, m, s));
  if (trSeg > 0 && !nBt(std::string(N_PIN) + "SET_NOCHE_TR:" + std::to_string(trSeg)))
    abortar("SET_NOCHE_TR no la contesto la noche");
  if (!franja.empty() && !nBt(std::string(N_PIN) + "SET_NOCHE:" + franja))
    abortar("SET_NOCHE no la contesto la noche");
}

// Hasta el destello, contando verdes; devuelve el instante del rojo en los dos (pines).
struct NVentana { unsigned long tRojo = 0, tDestello = 0, verdes = 0; };
static NVentana nHastaDestello(unsigned long maxMs) {
  NVentana v;
  for (unsigned long h = 0; h < maxMs; h += PASO_MS) {
    unTick();
    if (MAESTRO.estado() == S_AMBAR_INT_V) { v.tDestello = g_t; break; }
    if (MAESTRO.verde() || ESCLAVO.verde()) v.verdes++;
    const bool rojos = MAESTRO.rojo() && ESCLAVO.rojo() && !MAESTRO.ambar() && !ESCLAVO.ambar();
    if (!rojos) v.tRojo = 0;
    else if (v.tRojo == 0) v.tRojo = g_t;
  }
  return v;
}

static void bloqueNocheEntradaYSalidaPorHora() {
  nEscenario(21, 59, 30, 0, "");
  comprobar(nBt(std::string("CMD:CONSULTA_NOCHE")) &&
            nBtEs("$ACK,CMD:NOCHE,RESULT:OK,TR:" + std::to_string(N_TR_FABRICA_MS / 1000) +
                  ",F1:--------0000-0000-OFF,F2:--------0000-0000-OFF,F3:--------0000-0000-OFF,"
                  "F4:--------0000-0000-OFF"),
            "N1 (D-106, D-108): sin pagina, CONSULTA_NOCHE (sin PIN) dice tr de fabrica y cuatro franjas OFF");
  comprobar(nBt(std::string(N_PIN) + "SET_NOCHE:1,---J---,2200,2210,A") &&
            nBtEs("$ACK,CMD:SET_NOCHE,RESULT:OK") && nBt("CMD:CONSULTA_NOCHE") &&
            nBtEs("$ACK,CMD:NOCHE,RESULT:OK,TR:" + std::to_string(N_TR_FABRICA_MS / 1000) +
                  ",F1:---J----2200-2210-A,F2:--------0000-0000-OFF,F3:--------0000-0000-OFF,"
                  "F4:--------0000-0000-OFF"),
            "N1 (D-110): SET_NOCHE con PIN contesta OK y la consulta relee la franja guardada");
  avanzar(20000);
  comprobar(nModo() == N_MODO_AUTO, "N1 (D-111): a las 21:59:50 sigue en AUTO");
  const long entradas0 = nEv("ENTRADA");
  comprobar(nHasta([] { return nModo() == N_MODO_NOCHE; }, 15000) && nEv("ENTRADA") == entradas0 + 1,
            "N1 (D-111): en el minuto de inicio (flanco de las 22:00) entra en MODO_NOCHE y anota ENTRADA");
  comprobar(MAESTRO.orden("pedir_cambio") == 0,
            "N1 (D-111): durante el todo-rojo de entrada DAR PASO no hace nada (veto de verde)");
  const NVentana v = nHastaDestello(N_TR_FABRICA_MS + 60000);
  char msg[300];
  std::snprintf(msg, sizeof(msg), "N1 (D-106, D-111): destello %ld ms despues del rojo en los dos (tr de "
                "fabrica %lu ms, holgura %lu ms) y ningun verde en la entrada (%lu)",
                (long)(v.tDestello - v.tRojo), N_TR_FABRICA_MS, 2 * LATIDO_MS_V, v.verdes);
  comprobar(v.tDestello != 0 && v.tRojo != 0 && v.verdes == 0 && v.tDestello - v.tRojo >= N_TR_FABRICA_MS &&
            v.tDestello - v.tRojo <= N_TR_FABRICA_MS + 2 * LATIDO_MS_V, msg);
  comprobar(nHasta([] { return ESCLAVO.estado() == S_AMBAR_INT_V; }, 2000) &&
            MAESTRO.pin(MOTOR_TALANQUERA) == TALANQUERA_ABRIR && ESCLAVO.pin(MOTOR_TALANQUERA) == TALANQUERA_ABRIR,
            "N1 (D-107, SPEC_0 5.10 (a)): ambar intermitente en los dos (CMD_GO_NOCHE) y pluma ARRIBA en los dos");
  // Salida por hora: hasta las 22:10 sigue; en el flanco de fin, rojo en los dos y AUTO tras tr.
  comprobar(nHasta([] { return MAESTRO.estado() == S_ROJO_V; }, 8 * 60000UL) && nModo() == N_MODO_NOCHE &&
            nEv("SALIDA_HORA") == 1,
            "N5 (D-111): en el flanco de fin (22:10) el Maestro pasa del destello a ROJO directo; SALIDA_HORA");
  unsigned long verdes = 0, tRojo = 0, tAuto = 0;
  for (unsigned long h = 0; h < N_TR_FABRICA_MS + 60000 && tAuto == 0; h += PASO_MS) {
    unTick();
    if (MAESTRO.verde() || ESCLAVO.verde()) verdes++;
    if (tRojo == 0 && MAESTRO.rojo() && ESCLAVO.rojo() && !ESCLAVO.ambar()) tRojo = g_t;
    if (nModo() == N_MODO_AUTO) tAuto = g_t;
  }
  std::snprintf(msg, sizeof(msg), "N5 (D-106): vuelve a AUTO %ld ms despues del rojo en los dos (tr %lu ms) "
                "sin un solo verde en la salida (%lu)", (long)(tAuto - tRojo), N_TR_FABRICA_MS, verdes);
  comprobar(tAuto != 0 && tRojo != 0 && verdes == 0 && tAuto - tRojo >= N_TR_FABRICA_MS, msg);
  comprobar(nHasta([] { return MAESTRO.verde() || ESCLAVO.verde(); }, 120000),
            "N5: CONTROL - el AUTO de destino vuelve a dar verde (la salida no deja el cruce parado)");
}

static void bloqueNocheSinAckRojo() {
  nEscenario(21, 59, 50, 20, "1,---J---,2200,2230,A");
  g_enlaceHaciaEsclavo = g_enlaceHaciaMaestro = false;   // ni GO_RED ni ACK_RED
  comprobar(nHasta([] { return nModo() == N_MODO_NOCHE; }, 15000), "N2: entra en MODO_NOCHE sin radio");
  unsigned long verdes = 0;
  for (unsigned long h = 0; h < 60000; h += PASO_MS) { unTick(); if (MAESTRO.verde() || ESCLAVO.verde()) verdes++; }
  comprobar(MAESTRO.estado() != S_AMBAR_INT_V && verdes == 0,
            "N2 (D-111): sin ACK_RED no hay destello en 60 s con tr = 20 s (manda SFTY-6) y ningun verde");
  g_enlaceHaciaEsclavo = g_enlaceHaciaMaestro = true;
  const NVentana v = nHastaDestello(120000);
  comprobar(v.tDestello != 0 && v.verdes == 0 && nModo() == N_MODO_NOCHE,
            "N2 (D-111): vuelta la radio, la recuperacion del coordinador deja ROJO (no el despeje a verde) y "
            "entra al destello sin un solo verde");
}

static void bloqueNocheRojoIntYReenvio() {
  nEscenario(21, 59, 50, 10, "1,---J---,2200,2300,R");
  comprobar(nHasta([] { return ESCLAVO.estado() == S_ROJO_INT_V; }, 90000) && MAESTRO.estado() == S_AMBAR_INT_V,
            "N3 (D-107): color R -> Poste 1 ambar intermitente y Poste 2 ROJO intermitente");
  bool rojoOn = false, rojoOff = false, ambarEsc = false;
  for (unsigned long h = 0; h < 3000; h += PASO_MS) {
    unTick();
    (ESCLAVO.pin(ROJO1) == HIGH ? rojoOn : rojoOff) = true;
    ambarEsc = ambarEsc || ESCLAVO.ambar();
  }
  comprobar(rojoOn && rojoOff && !ambarEsc, "N3 (D-110): el rojo del Poste 2 PARPADEA y su ambar no se enciende");
  const long alarmas0 = MAESTRO.orden("alarmas");
  g_enlaceHaciaEsclavo = false;
  avanzar(SFTY6_SILENCIO_MS_V + 5000);
  comprobar(ESCLAVO.estado() == S_FALLO_V && MAESTRO.estado() == S_AMBAR_INT_V && nModo() == N_MODO_NOCHE &&
            MAESTRO.orden("alarmas") == alarmas0 + 1,
            "N4 (D-107): sin radio el Esclavo cae a S_FALLO (ambar) y el Maestro sigue en la noche con UNA alarma");
  g_enlaceHaciaEsclavo = true;
  comprobar(nHasta([] { return ESCLAVO.estado() == S_ROJO_INT_V; }, N_REENVIO_MS + 2 * LATIDO_MS_V),
            "N4 (D-107): vuelta la radio, el reenvio de CMD_GO_NOCHE devuelve el rojo intermitente");
}

static void bloqueNocheSalidasPorApp() {
  nEscenario(21, 59, 50, 10, "1,---J---,2200,2300,A");
  nHastaDestello(90000);
  comprobar(nBt(std::string(N_PIN) + "SET_NOCHE:1,LMXJVSD,2200,2300,R") &&
            nBtEs("$ERR,CMD:SET_NOCHE,DESC:EN_NOCHE_SALGA_PRIMERO"),
            "N6 (D-110): la franja en curso no se cambia: EN_NOCHE_SALGA_PRIMERO");
  comprobar(!nBt("CMD:SET_MODO:AUTO"), "N6: SET_MODO:AUTO sin PIN no la contesta la noche (PIN_INVALIDO)");
  comprobar(nBt(std::string(N_PIN) + "SET_MODO:AUTO") &&
            nBtEs("$ACK,CMD:SET_MODO:AUTO,RESULT:SALIENDO_TODO_ROJO") && MAESTRO.estado() == S_ROJO_V,
            "N6 (D-111): SET_MODO:AUTO con PIN -> SALIENDO_TODO_ROJO y rojo directo en el Maestro");
  comprobar(nBt(std::string(N_PIN) + "SET_MODO:MANUAL") && nBtEs("$ERR,CMD:SET_MODO:MANUAL,DESC:SALIDA_EN_CURSO"),
            "N6 (D-111): una segunda salida mientras corre el todo-rojo: SALIDA_EN_CURSO");
  unsigned long verdes = 0;
  const bool llega = nHasta([&verdes] {
    if (MAESTRO.verde() || ESCLAVO.verde()) verdes++;
    return nModo() == N_MODO_AUTO; }, 40000);
  comprobar(llega && verdes == 0 && nEv("SALIDA_APP") == 1,
            "N6 (D-111): tras tr llega a AUTO sin verde en la salida y anota SALIDA_APP");

  nEscenario(21, 59, 50, 10, "1,---J---,2200,2300,A");
  nHastaDestello(90000);
  comprobar(!nBt("CMD:FORZAR_ROJO") && nModo() == N_MODO_MANUAL && MAESTRO.estado() == S_ROJO_V &&
            nHasta([] { return ESCLAVO.estado() == S_ROJO_V; }, 1500) && nEv("SALIDA_APP") == 1,
            "N7 (D-111): FORZAR_ROJO sin PIN en la noche: rojo inmediato en los dos y fuera de la noche (MANUAL)");
}

static void bloqueNocheSaltadaYFlanco() {
  nEscenario(21, 59, 50, 0, "1,---J---,2200,2300,A");
  MAESTRO.orden("fijar_modo", N_MODO_MANUAL);
  avanzar(20000);
  comprobar(nModo() == N_MODO_MANUAL && nEv("FRANJA_SALTADA") == 1 && nEv("ENTRADA") == 0,
            "N8 (D-111): con una persona en MANUAL la franja se salta y se anota FRANJA_SALTADA");
  nEscenario(21, 59, 50, 0, "1,----V--,2200,2300,A");
  avanzar(20000);
  comprobar(nModo() == N_MODO_AUTO && nEv("FRANJA_SALTADA") == 0,
            "N8 (D-108): una franja de viernes no entra un jueves");
  nEscenario(22, 0, 5, 0, "1,---J---,2200,2300,A");
  avanzar(20000);
  comprobar(nModo() == N_MODO_AUTO,
            "N8 (D-111): el primer minuto que se ve no es flanco: con la hora puesta a las 22:00:05 no entra");
  nEscenario(21, 59, 50, 0, "1,---J---,2200,2300,A");
  MAESTRO.orden("hora_fiable", 0);
  avanzar(20000);
  comprobar(nModo() == N_MODO_AUTO, "N8 (SFTY-18): sin hora fiable no entra");
}

static void bloqueNocheErrores() {
  nEscenario(12, 0, 0, 0, "");
  struct { const char* orden; const char* resp; } c[] = {
    { "SET_NOCHE:1,LMXJVSD,2200,0500,AX", "$ERR,CMD:SET_NOCHE,DESC:FORMATO_INVALIDO" },
    { "SET_NOCHE:1,LMXJVSD,2200,0500", "$ERR,CMD:SET_NOCHE,DESC:FORMATO_INVALIDO" },
    { "SET_NOCHE:1,MLXJVSD,2200,0500,A", "$ERR,CMD:SET_NOCHE,DESC:FORMATO_INVALIDO" },
    { "SET_NOCHE:5,LMXJVSD,2200,0500,A", "$ERR,CMD:SET_NOCHE,DESC:RANGO" },
    { "SET_NOCHE:1,LMXJVSD,2460,0500,A", "$ERR,CMD:SET_NOCHE,DESC:RANGO" },
    { "SET_NOCHE:0,OFF", "$ERR,CMD:SET_NOCHE,DESC:RANGO" },
    { "SET_NOCHE:2,LMXJV--,2330,0400,R", "$ACK,CMD:SET_NOCHE,RESULT:OK" },
    { "SET_NOCHE_TR:9", "$ERR,CMD:SET_NOCHE_TR,DESC:RANGO" },
    { "SET_NOCHE_TR:901", "$ERR,CMD:SET_NOCHE_TR,DESC:RANGO" },
    { "SET_NOCHE_TR:30x", "$ERR,CMD:SET_NOCHE_TR,DESC:FORMATO_INVALIDO" },
    { "SET_NOCHE_TR:10", "$ACK,CMD:SET_NOCHE_TR,RESULT:OK" },
    { "SET_NOCHE_TR:900", "$ACK,CMD:SET_NOCHE_TR,RESULT:OK" },
  };
  for (auto& k : c)
    comprobar(nBt(std::string(N_PIN) + k.orden) && nBtEs(k.resp),
              std::string("N9 (D-110): ") + k.orden + " -> " + k.resp);
  comprobar(!nBt("CMD:SET_NOCHE_TR:60"), "N9: SET_NOCHE_TR sin PIN sigue al despachador (PIN_INVALIDO)");
  MAESTRO.orden("noche_flash_falla", 1);
  comprobar(nBt(std::string(N_PIN) + "SET_NOCHE_TR:60") && nBtEs("$ERR,CMD:SET_NOCHE_TR,DESC:FLASH_NO_RELEE") &&
            nBt(std::string(N_PIN) + "SET_NOCHE:2,OFF") && nBtEs("$ERR,CMD:SET_NOCHE,DESC:FLASH_NO_RELEE"),
            "N9 (D-110): si la flash no relee lo pedido, FLASH_NO_RELEE y no OK");
  MAESTRO.orden("noche_flash_falla", 0);
  MAESTRO.orden("noche_cargar");
  comprobar(nBt("CMD:CONSULTA_NOCHE") &&
            nBtEs("$ACK,CMD:NOCHE,RESULT:OK,TR:900,F1:--------0000-0000-OFF,F2:LMXJV---2330-0400-R,"
                  "F3:--------0000-0000-OFF,F4:--------0000-0000-OFF"),
            "N9 (D-108): releida la pagina queda lo ultimo aceptado (tr 900, F2), no lo rechazado");
}

static void bloqueNocheSinHora() {
  nEscenario(21, 59, 50, 10, "1,---J---,2200,2205,A");
  nHastaDestello(90000);
  const long a0 = MAESTRO.orden("alarmas");
  MAESTRO.orden("hora_fiable", 0);
  avanzar(1000);
  const long a1 = MAESTRO.orden("alarmas");
  avanzar(61000);
  comprobar(a1 == a0 + 1 && MAESTRO.orden("alarmas") == a0 + 2 && MAESTRO.orden("alarma_es:NOCHE") == 1,
            "N10 (D-111): sin hora fiable dentro, $ALARM NOCHE al perderla y otra al minuto");
  avanzar(5 * 60000UL);
  comprobar(nModo() == N_MODO_NOCHE && MAESTRO.estado() == S_AMBAR_INT_V,
            "N10 (D-111): pasado el fin (22:05) sin hora fiable se QUEDA en el destello: se sale por la app");
  // N11 (SPEC_0 5.10 (f), D-115): vuelta la hora fuera de la franja, sale por el todo-rojo; no espera 24 h.
  MAESTRO.orden("hora_fiable", 1);
  comprobar(nHasta([] { return nModo() == N_MODO_AUTO; }, 120000) && nEv("SALIDA_HORA") == 1,
            "N11 (D-115): la hora vuelve FIABLE pasado el fin (22:05): SALIDA_HORA y AUTO tras tr");
}

// N11 (SPEC_0 5.10 (e), (f), D-115): la salida es «hora fiable y minuto fuera de la franja», no el
// minuto exacto de fin; y una franja que empieza cuando otra acaba no se pierde muda.
static void bloqueNocheFueraDeFranja() {
  nEscenario(21, 59, 50, 10, "1,---J---,2200,2205,A");
  nHastaDestello(90000);
  avanzar(4 * 60000UL);                          // ~22:04:30
  MAESTRO.orden("noche_fecha", nHora(22, 6, 0));  // re-siembra: el reloj salta el minuto 22:05
  comprobar(nHasta([] { return nModo() == N_MODO_AUTO; }, 120000) && nEv("SALIDA_HORA") == 1,
            "N11 (D-115): el reloj salta de 22:04 a 22:06 sin pasar por el fin: SALIDA_HORA y AUTO tras tr");
  nEscenario(21, 59, 50, 10, "1,---J---,2200,2205,A");
  nHastaDestello(90000);
  avanzar(60000);
  MAESTRO.orden("noche_fecha", nHora(21, 30, 0));  // el reloj retrocede antes del inicio
  comprobar(nHasta([] { return nModo() == N_MODO_AUTO; }, 120000) && nEv("SALIDA_HORA") == 1,
            "N11 (D-115): el reloj retrocede antes del inicio (21:30): fuera de la franja, sale");
  nEscenario(21, 59, 50, 10, "1,---J---,2200,2205,A");
  if (!nBt(std::string(N_PIN) + "SET_NOCHE:2,---J---,2205,2210,A")) abortar("SET_NOCHE 2 no la contesto la noche");
  avanzar(8 * 60000UL);                          // ~22:07:50
  comprobar(nModo() == N_MODO_NOCHE || nEv("FRANJA_SALTADA") == 1,
            "N11 (5.10 (e)): F2 22:05-22:10 empieza cuando F1 acaba: sigue en noche o consta FRANJA_SALTADA");
}

static void bloqueNoche() {
  std::printf("\n--- BLOQUE N: el modo noche (N-179, D-106..D-111) ------------------\n");
  const std::string SEM_M = RAIZ + "/Maestro/include/semaforo.h";
  const std::string SEM_E = RAIZ + "/Esclavo/include/semaforo.h";
  S_AMBAR_INT_V = nIndice(SEM_E, "EstadoSemaforo", "S_AMBAR_INT");
  S_ROJO_INT_V = nIndice(SEM_E, "EstadoSemaforo", "S_ROJO_INT");
  if (nIndice(SEM_M, "EstadoSemaforo", "S_AMBAR_INT") != S_AMBAR_INT_V)
    abortar("S_AMBAR_INT DIFIERE entre las dos puntas");
  const std::string MODOS = RAIZ + "/Maestro/include/modos.h";
  N_MODO_AUTO = nIndice(MODOS, "ModoSistema", "MODO_AUTOMATICO");
  N_MODO_MANUAL = nIndice(MODOS, "ModoSistema", "MODO_MANUAL");
  N_MODO_NOCHE = nIndice(MODOS, "ModoSistema", "MODO_NOCHE");
  N_TR_FABRICA_MS = 1000UL * leerNumero(RAIZ + "/Maestro/include/modo_noche.h",
                                        R"(NOCHE_TR_FABRICA_S\s*=\s*(\d+))", "NOCHE_TR_FABRICA_S");
  N_REENVIO_MS = leerNumero(RAIZ + "/Maestro/include/protocolo.h", R"(NOCHE_REENVIO_MS\s+(\d+)UL)", "NOCHE_REENVIO_MS");
  bloqueNocheEntradaYSalidaPorHora();
  bloqueNocheSinAckRojo();
  bloqueNocheRojoIntYReenvio();
  bloqueNocheSalidasPorApp();
  bloqueNocheSaltadaYFlanco();
  bloqueNocheErrores();
  bloqueNocheSinHora();
  bloqueNocheFueraDeFranja();
}
