// ===== Validacion_Automatico/dos_puntas/orquestador_deg_verde.inc =====
//
// BLOQUE K - D-59 (responsable, 13/10; SPEC_2 7.bis): el verde del Degradado CON TESTIGO lo elige el operario en
// minutos enteros 3..15 y viaja en SEGUNDOS, multiplo de 60 entre 180 y 900; fuera, rechazo con su motivo. 180 (la
// app vieja) sigue valiendo. La renovacion exige el mismo verde y despeje; el verde va a la flash del testigo y es el
// que se usa al reanudar. Los valores salen de la decision y de la SPEC, ninguno del fuente. El verde se mide en los
// PINES de las dos puntas: la duracion de cada verde completo, y que nunca haya paso abierto en las dos a la vez.
// Incluido desde orquestador_deg_fin.inc; usa unTick, prepararSincronizadas, activarDs3231, puntaH, ceroH,
// transversalH, reiniciarObservacion y comprobar.

struct ObsK {
  unsigned long racha[2] = {0, 0}, minV[2] = {~0UL, ~0UL}, maxV[2] = {0, 0}, n[2] = {0, 0};
  bool limpio[2] = {false, false};   // el verde que ya corria al empezar no es un verde completo
};

static void pasoK(ObsK& o) {
  unTick();
  for (int i = 0; i < 2; i++) {
    if (puntaH(i).verde()) { o.racha[i] += PASO_MS; continue; }
    if (o.racha[i] && o.limpio[i]) {
      o.n[i]++;
      if (o.racha[i] < o.minV[i]) o.minV[i] = o.racha[i];
      if (o.racha[i] > o.maxV[i]) o.maxV[i] = o.racha[i];
    }
    o.racha[i] = 0;
    o.limpio[i] = true;
  }
}

static ObsK correrK(unsigned long ms) {
  ObsK o;
  reiniciarObservacion();
  ceroH();
  for (unsigned long h = 0; h < ms; h += PASO_MS) pasoK(o);
  return o;
}

// Dos verdes completos o mas en esa punta, todos de v s (un segundo de tolerancia: el tick y la frontera de segundo).
static bool verdeDe(const ObsK& o, int i, long v) {
  const long ms = v * 1000L;
  return o.n[i] >= 2 && labs((long)o.minV[i] - ms) <= 1000L && labs((long)o.maxV[i] - ms) <= 1000L;
}

static std::string cifrasK(const ObsK& o) {
  std::string s;
  for (int i = 0; i < 2; i++)
    s += std::string(i ? "; E " : "M ") + std::to_string(o.n[i]) + " verdes de " +
         std::to_string(o.n[i] ? o.minV[i] : 0) + ".." + std::to_string(o.maxV[i]) + " ms";
  return s + "; paso abierto en las dos " + std::to_string(g_verdeSimultaneo * PASO_MS) + " ms";
}

// Lo que dura ver dos verdes enteros de cada punta desde un inicio a 120 s. BORDE: tres ciclos (cada uno 2*(v+35) s
// con R+A, amarillo y despeje 30), porque la entrada espera a dejar atras el verde en curso: con dos, el de 900 s
// dejaba al Esclavo un solo verde (medido el 14/10).
static unsigned long msDosCiclos(long v) { return (120UL + 6UL * ((unsigned long)v + 40UL) + 120UL) * 1000UL; }

// Las dos sincronizadas por radio, con su DS3231 sembrando y la radio YA muerta: el caso para el que existe el testigo.
static void montarK() {
  prepararSincronizadas(15, 8, 0, 0);
  activarDs3231(1000, 7000);
  avanzar(5000);
  g_enlace = false;
  g_aire.clear();
  avanzar(60000);
}

// El testigo con verde v en las dos (el operario lleva la misma orden), inicio a 120 s. Devuelve los dos codigos.
static void testigoK(long v, long ini, long& rM, long& rE) {
  MAESTRO.orden("dt_verde", v);
  ESCLAVO.orden("dt_verde", v);
  rM = MAESTRO.orden("dt_testigo", ini);
  rE = ESCLAVO.orden("dt_testigo", ini);
}
static long iniK() { return (MAESTRO.orden("segundos_del_dia") + 120L) % 86400L; }

// Las dos en rojo sin amarillo a la vez (el despeje): la flash solo se escribe en rojo (SPEC_2 7.bis).
static bool rojoLasDos() {
  return MAESTRO.rojo() && !MAESTRO.ambar() && ESCLAVO.rojo() && !ESCLAVO.ambar();
}

// El corte de las dos, como cortarLasDos() + corteConSiembra(), y ADEMAS la pagina del testigo: la flash no se
// borra con un corte, el doble de la DLL si (adaptador_*_testigo1.inc), y se repone campo a campo antes de arrancar.
static void cortarK() {
  const long H = horaDs3231(0);
  long domM[PUNTA_DOMINIO_PALABRAS], domE[PUNTA_DOMINIO_PALABRAS], flM[6], flE[6];
  for (int i = 0; i < PUNTA_DOMINIO_PALABRAS; i++) { domM[i] = MAESTRO.domLeer(i); domE[i] = ESCLAVO.domLeer(i); }
  for (int i = 0; i < 6; i++) { flM[i] = MAESTRO.orden("dt_flash", i); flE[i] = ESCLAVO.orden("dt_flash", i); }
  MAESTRO.descargar(); MAESTRO.cargar();
  ESCLAVO.descargar(); ESCLAVO.cargar();
  for (int i = 0; i < PUNTA_DOMINIO_PALABRAS; i++) { MAESTRO.domEscribir(i, domM[i]); ESCLAVO.domEscribir(i, domE[i]); }
  for (int i = 5; i >= 0; i--) {
    const std::string k = "dt_flash_pon:" + std::to_string(i);
    MAESTRO.orden(k.c_str(), flM[i]);
    ESCLAVO.orden(k.c_str(), flE[i]);
  }
  g_aire.clear();
  g_enlace = false;
  MAESTRO.arrancar();
  ESCLAVO.arrancar();
  g_t = DELAY_ARRANQUE_MS;
  g_plumaSinVerdeDesde[0] = g_plumaSinVerdeDesde[1] = 0;
  g_tRefDs = g_t;
  g_segRefDs = (H % 86400L) + 10;
  g_diaRefDs = H / 86400L;
  g_esp32Ds3231 = true;
  g_esp32Vivo[0] = g_esp32Vivo[1] = true;
  g_proxSiembra[0] = g_proxSiembra[1] = g_t + 1000;
}

static void bloqueK() {
  std::printf("\n--- BLOQUE K: D-59, EL VERDE DEL DEGRADADO CON TESTIGO (SPEC_2 7.bis) -\n");
  // K1: 180 (app vieja), 240, 360 y 900 s en las dos: cada verde dura lo pedido, la flash lo guarda, y ni un
  // instante con paso abierto en las dos.
  for (long v : {180L, 240L, 360L, 900L}) {
    montarK();
    long rM = -1, rE = -1;
    testigoK(v, iniK(), rM, rE);
    const ObsK o = correrK(msDosCiclos(v));
    bool okT = false;
    const std::string tr = transversalH(okT);
    const long fM = MAESTRO.orden("dt_flash", 3), fE = ESCLAVO.orden("dt_flash", 3);
    comprobar(rM == 0 && rE == 0 && fM == v && fE == v && verdeDe(o, 0, v) && verdeDe(o, 1, v) && okT &&
                  g_verdeSimultaneo == 0,
              "K1 (D-59): testigo con verde " + std::to_string(v) + " s en las dos (acuse M " + std::to_string(rM) +
                  " / E " + std::to_string(rE) + ", flash " + std::to_string(fM) + " / " + std::to_string(fE) +
                  "): " + cifrasK(o) + "; " + tr);
  }
  // K2: fuera de 180..900 o no multiplo de 60: rechazo con el motivo del verde (2) en las DOS, y ninguna entra.
  montarK();
  for (long v : {960L, 170L, 250L, 120L, 0L}) {
    long rM = -1, rE = -1;
    testigoK(v, iniK(), rM, rE);
    avanzar(2000);
    const long eM = MAESTRO.orden("da_en_degradado"), eE = ESCLAVO.orden("da_en_degradado");
    comprobar(rM == 2 && rE == 2 && eM == 0 && eE == 0,
              "K2 (D-59): verde " + std::to_string(v) + " s rechazado por VERDE fuera de rango en las dos (M " +
                  std::to_string(rM) + " / E " + std::to_string(rE) + "; 2 = VERDE_RANGO) y ninguna entra (" +
                  std::to_string(eM) + " / " + std::to_string(eE) + ")");
  }
  // K3: alternando con 360. El mismo verde renueva (1); otro verde NO renueva: es entrada nueva (0), vuelve a rojo
  // hasta su inicio, y con la orden en las dos alternan con el verde nuevo sin solaparse.
  {
    montarK();
    long rM = -1, rE = -1;
    testigoK(360, iniK(), rM, rE);
    avanzar(1200000UL);
    for (unsigned long h = 0; h < 1200000UL && !rojoLasDos(); h += PASO_MS) unTick();
    long nM = -1, nE = -1;
    testigoK(360, iniK(), nM, nE);
    for (unsigned long h = 0; h < 1200000UL && !rojoLasDos(); h += PASO_MS) unTick();
    long cM = -1, cE = -1;
    testigoK(240, iniK(), cM, cE);
    const ObsK o = correrK(msDosCiclos(240));
    bool okT = false;
    const std::string tr = transversalH(okT);
    const long fM = MAESTRO.orden("dt_flash", 3), fE = ESCLAVO.orden("dt_flash", 3);
    comprobar(rM == 0 && rE == 0 && nM == 1 && nE == 1,
              "K3a (D-59): con el MISMO verde (360) y despeje el testigo renueva en las dos (M " + std::to_string(nM) +
                  " / E " + std::to_string(nE) + "; 1 = RENOVADO)");
    comprobar(cM == 0 && cE == 0 && fM == 240 && fE == 240 && verdeDe(o, 0, 240) && verdeDe(o, 1, 240) && okT &&
                  g_verdeSimultaneo == 0,
              "K3b (D-59): con OTRO verde (240) no renueva: entrada nueva (M " + std::to_string(cM) + " / E " +
                  std::to_string(cE) + "; 0 = OK, no 1), flash " + std::to_string(fM) + " / " + std::to_string(fE) +
                  ", y alternan con 240: " + cifrasK(o) + "; " + tr);
  }
  // K4: alternando con 360, corte de las dos: reanudan con el verde de la FLASH (360), no con 180, sin solaparse.
  {
    montarK();
    long rM = -1, rE = -1;
    testigoK(360, iniK(), rM, rE);
    avanzar(1200000UL);
    cortarK();
    const ObsK o = correrK(msDosCiclos(360) + 300000UL);
    bool okT = false;
    const std::string tr = transversalH(okT);
    const long eM = MAESTRO.orden("da_en_degradado"), eE = ESCLAVO.orden("da_en_degradado");
    comprobar(rM == 0 && rE == 0 && eM == 1 && eE == 1 && verdeDe(o, 0, 360) && verdeDe(o, 1, 360) && okT &&
                  g_verdeSimultaneo == 0,
              "K4 (D-59): corte de las dos alternando con 360: reanudan (" + std::to_string(eM) + " / " +
                  std::to_string(eE) + ") con el verde guardado: " + cifrasK(o) + "; " + tr);
  }
}
