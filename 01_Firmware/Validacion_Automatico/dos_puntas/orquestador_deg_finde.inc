// ===== Validacion_Automatico/dos_puntas/orquestador_deg_finde.inc =====
//
// BLOQUE L - EL FIN DE SEMANA EN DEGRADADO (responsable, 09/11; El Sisga: el par cae a ambar viernes o sabado).
// El firmware de campo (7ff7d12) no tiene testigo y su Degradado cae a las 48 h sin sync. El de hoy dice que con
// testigo no hay tope (modo_degradado.cpp de las dos puntas, H9, D-35; SPEC_2 7.ter (d)). Lo ejecutan aqui las dos
// puntas reales con la radio muerta y el DS3231 de cada poste sembrando, a PASO_MS y sin atajo de reloj:
//   L1  testigo, viernes 18:00 a lunes 18:00 (72 h): ni ambar, ni verde contra verde, las dos alternando.
//   L2  sin testigo (solo por la orden del arnes: en campo la unica entrada es el testigo, D-46): ambar a las 48 h.
//   L3  testigo a los 28 dias (fecha adelantada, como H9): solo el aviso RENOVAR_TESTIGO, una vez al dia; luz igual.
// Incluido desde orquestador_deg_fin.inc tras el bloque K; usa montarK/testigoK/iniK de orquestador_deg_verde.inc.

static const uint8_t L_VIERNES = 16;   // la siembra del arnes lleva 2026-01-DD: el 16/01/2026 fue viernes
static const unsigned long L_HORA_MS = 3600000UL;

struct ObsL {
  unsigned long fallo[2] = {0, 0}, fuera[2] = {0, 0}, verde[2] = {0, 0};
  unsigned long sinVerde[2] = {0, 0}, maxSinVerde[2] = {0, 0}, tFallo[2] = {0, 0};
};

static void pasoL(ObsL& o, unsigned long t0) {
  unTick();
  for (int i = 0; i < 2; i++) {
    Punta& p = puntaH(i);
    if (p.orden("da_en_fallo") == 1) { if (!o.fallo[i]) o.tFallo[i] = g_t - t0; o.fallo[i]++; }
    if (p.orden("da_en_degradado") != 1) o.fuera[i]++;
    if (p.verde()) { o.verde[i]++; o.sinVerde[i] = 0; continue; }
    o.sinVerde[i] += PASO_MS;
    if (o.sinVerde[i] > o.maxSinVerde[i]) o.maxSinVerde[i] = o.sinVerde[i];
  }
}

static std::string cifrasL(const ObsL& o) {
  std::string s;
  for (int i = 0; i < 2; i++)
    s += std::string(i ? "; E " : "M ") + "ambar " + std::to_string(o.fallo[i] * PASO_MS) + " ms, fuera " +
         std::to_string(o.fuera[i] * PASO_MS) + " ms, verde " + std::to_string(o.verde[i] * PASO_MS / 1000UL) +
         " s, mayor hueco sin verde " + std::to_string(o.maxSinVerde[i] / 1000UL) + " s";
  return s;
}

// Las dos sincronizadas el viernes a las 18:00, DS3231 sembrando con esa fecha y la radio ya muerta (montarK).
static void montarL() {
  prepararSincronizadas(L_VIERNES, 18, 0, 0);
  activarDs3231(1000, 7000);
  g_diaRefDs = L_VIERNES;   // activarDs3231 fija el 15
  avanzar(5000);
  g_enlace = false;
  g_aire.clear();
  avanzar(60000);
}

static void bloqueL() {
  std::printf("\n--- BLOQUE L: EL FIN DE SEMANA EN DEGRADADO (72 h sin radio) -------------\n");
  const long V = 180;   // el verde de la app vieja (SPEC_2 7.bis)
  // EL BORDE del hueco: dos ciclos enteros, 2 * 2*(v+40) s (msDosCiclos: R+A, amarillo y despeje). La guarda de
  // medianoche cuesta un ciclo al dia (ciclo_degradado.h); una punta que pasa DOS sin verde dejo de alternar.
  const unsigned long HUECO_MAX_MS = 4UL * ((unsigned long)V + 40UL) * 1000UL;

  // L1: testigo, 72 h.
  {
    montarL();
    long rM = -1, rE = -1;
    testigoK(V, iniK(), rM, rE);
    const long dia0 = horaDs3231(0) / 86400L;
    reiniciarObservacion();
    ceroH();
    ObsL o;
    const unsigned long t0 = g_t;
    for (unsigned long h = 0; h < 72UL * L_HORA_MS; h += PASO_MS) pasoL(o, t0);
    bool okT = false;
    const std::string tr = transversalH(okT);
    const long dia1 = horaDs3231(0) / 86400L;
    const long lM = MAESTRO.orden("alarmas_limite_48h"), lE = ESCLAVO.orden("alarmas_limite_48h");
    const long vM = MAESTRO.orden("da_sync_vencida"), vE = ESCLAVO.orden("da_sync_vencida");
    comprobar(rM == 0 && rE == 0 && dia1 - dia0 == 3 && o.fallo[0] == 0 && o.fallo[1] == 0 && o.fuera[0] == 0 &&
                  o.fuera[1] == 0 && o.maxSinVerde[0] < HUECO_MAX_MS && o.maxSinVerde[1] < HUECO_MAX_MS &&
                  lM == 0 && lE == 0 && vM == 0 && vE == 0 && okT && g_verdeSimultaneo == 0,
              "L1 (H9, D-35): testigo de " + std::to_string(V) + " s (acuse M " + std::to_string(rM) + " / E " +
                  std::to_string(rE) + ") del dia " + std::to_string(dia0) + " al " + std::to_string(dia1) +
                  " (viernes 18:00 a lunes 18:00, 72 h sin radio): " + cifrasL(o) + " (borde " +
                  std::to_string(HUECO_MAX_MS / 1000UL) + " s); LIMITE_48H M " + std::to_string(lM) + " / E " +
                  std::to_string(lE) + ", vencida publicada M " + std::to_string(vM) + " / E " + std::to_string(vE) +
                  "; " + tr);
  }

  // L2: SIN testigo. La entrada es la orden del arnes (set_modo / degradado_entrar): en campo ya no existe (D-46);
  // es el Degradado del firmware de El Sisga. Cae a ambar en las DOS a las 48 h de la sync, no antes.
  {
    const unsigned long LIM = leerNumero(RAIZ + "/Maestro/src/modo_degradado.cpp",
                                         R"(LIMITE_DURO_MS\s*=\s*(\d+))", "LIMITE_DURO_MS del Maestro (L2)");
    prepararSincronizadas(L_VIERNES, 18, 0, 0);
    activarDs3231(1000, 7000);
    g_diaRefDs = L_VIERNES;
    entrarEnDegradadoLasDos(0);
    const unsigned long sync0 = (unsigned long)MAESTRO.orden("ms_desde_sync");
    reiniciarObservacion();
    ceroH();
    ObsL o;
    const unsigned long t0 = g_t;
    for (unsigned long h = 0; h < LIM + L_HORA_MS && !(o.fallo[0] && o.fallo[1]); h += PASO_MS) pasoL(o, t0);
    bool okT = false;
    const std::string tr = transversalH(okT);
    const unsigned long fM = sync0 + o.tFallo[0], fE = sync0 + o.tFallo[1];
    const long lM = MAESTRO.orden("alarmas_limite_48h"), lE = ESCLAVO.orden("alarmas_limite_48h");
    comprobar(o.fallo[0] && o.fallo[1] && fM >= LIM && fM < LIM + 60000UL && fE >= LIM - L_HORA_MS &&
                  fE < LIM + 300000UL && lM >= 1 && lE >= 1 && g_verdeSimultaneo == 0,
              "L2 (sin testigo, firmware de El Sisga): ambar en el Maestro a " + std::to_string(fM) +
                  " ms de la sync y en el Esclavo a " + std::to_string(fE) + " ms (limite " + std::to_string(LIM) +
                  "; borde: M dentro del minuto, E dentro de 5 min, su despeje de salida); LIMITE_48H M " +
                  std::to_string(lM) + " / E " + std::to_string(lE) + "; " + tr);
  }

  // L3: testigo a los 27, 28 y 29 dias (la fecha de las siembras adelantada, la hora del dia intacta, como H9).
  {
    montarL();
    long rM = -1, rE = -1;
    testigoK(V, iniK(), rM, rE);
    avanzar(600000UL);
    long renov[3][2];
    ObsL o;
    reiniciarObservacion();
    ceroH();
    const unsigned long t0 = g_t;
    for (int k = 0; k < 3; k++) {
      MAESTRO.orden("da_dias_extra", 27 + k);
      ESCLAVO.orden("da_dias_extra", 27 + k);
      for (unsigned long h = 0; h < 900000UL; h += PASO_MS) pasoL(o, t0);
      renov[k][0] = MAESTRO.orden("da_renovar");
      renov[k][1] = ESCLAVO.orden("da_renovar");
    }
    MAESTRO.orden("da_dias_extra", 0);
    ESCLAVO.orden("da_dias_extra", 0);
    bool okT = false;
    const std::string tr = transversalH(okT);
    comprobar(rM == 0 && rE == 0 && renov[0][0] == 0 && renov[0][1] == 0 && renov[1][0] == 1 && renov[1][1] == 1 &&
                  renov[2][0] == 2 && renov[2][1] == 2 && o.fallo[0] == 0 && o.fallo[1] == 0 && o.fuera[0] == 0 &&
                  o.fuera[1] == 0 && o.verde[0] > 0 && o.verde[1] > 0 && okT,
              "L3 (H9): RENOVAR_TESTIGO a los 27 / 28 / 29 dias: M " + std::to_string(renov[0][0]) + "/" +
                  std::to_string(renov[1][0]) + "/" + std::to_string(renov[2][0]) + ", E " +
                  std::to_string(renov[0][1]) + "/" + std::to_string(renov[1][1]) + "/" +
                  std::to_string(renov[2][1]) + " (0/1/2: el dia 28 y uno al dia) y la luz sigue: " + cifrasL(o) +
                  "; " + tr);
  }
}
