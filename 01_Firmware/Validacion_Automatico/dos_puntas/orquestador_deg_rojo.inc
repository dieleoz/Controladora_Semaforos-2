// ===== Validacion_Automatico/dos_puntas/orquestador_deg_rojo.inc =====
//
// BLOQUE S - D-173 (responsable 10/11): "ROJO TOTAL es rojo total". FORZAR_ROJO desde MODO_AUTOMATICO y
// MODO_INTELIGENTE deja el rojo FIJO en los dos postes hasta otra orden, igual que desde el ambar (D-170): el
// modo pasa a MODO_MANUAL. Escritas ANTES del codigo y vistas en rojo: con D-170 el ciclo seguia al vencer la fase.
// Incluido desde orquestador_deg_fin.inc; usa prepararSincronizadas, unTick, avanzar, observarJ y comprobar.
//
// LO QUE SE EXIGE SALE DE LA SPEC, NO DEL FUENTE: rojo en los dos, MODO_MANUAL (SPEC_4, FORZAR_ROJO) y el amarillo
// de cierre de la via que estaba en verde (D-45, SPEC_1: 3 s, nunca verde->rojo directo). Del fuente solo la
// posicion en el enum, SFTY6_SILENCIO_MS y los tiempos de ciclo CONFIGURADOS, que fijan cuanto se mira.
//
// EL BORDE DEL AMARILLO, escrito al lado (CLAUDE.md 7): 3000 ms de D-45, mas un tick y 3000/40 por el reloj de la
// punta (HSI_PPM_PEOR = 25000 ppm, el mismo criterio que RA_MS en orquestador_degradado.cpp).
//
// EL CONTROL ES SET_MODO:MANUAL, el camino que ya existia (bluetooth_acciones.inc: modoActual_set(MANUAL) y el
// todo-rojo): mide lo mismo y pasa con el firmware de antes y de despues. Si FORZAR_ROJO difiere de el, es defecto.

static const unsigned long AMARILLO_S_MS = 3000UL;   // D-45, SPEC_1: el amarillo de cierre
static const unsigned long TOL_AMARILLO_S_MS = PASO_MS + AMARILLO_S_MS / 40UL;
static const unsigned long CIERRE_S_MS = 6000UL;     // amarillo + un viaje + margen: despues, rojo en los dos

struct ObsT {
  bool habiaVerde = false;
  unsigned long ambarV = 0;         // ms de amarillo SOLO en la via que estaba en verde
  unsigned long directo = 0;        // veces verde -> rojo sin amarillo en esa via
  unsigned long simultaneo = 0;     // ms con verde en las dos
  unsigned long noRojoM = 0, noRojoE = 0;   // ms sin rojo pasado el cierre
  unsigned long ms = 0;
  long modo = -1;
};

// quien: 0 = la via en verde es la del Maestro, 1 = la del Esclavo. porManual: el control (SET_MODO:MANUAL).
static ObsT rojoTotalT(long origen, int quien, bool porManual, long manualV, unsigned long obsMs) {
  prepararSincronizadas(15, 8, 0, 0);
  MAESTRO.orden("set_modo", origen);
  Punta& p = (quien == 0) ? MAESTRO : ESCLAVO;
  avanzar(CIERRE_S_MS);   // la entrada al modo cierra lo que hubiera en verde (iniciarModo): se mira el verde DEL CICLO
  for (unsigned long h = 0; h < 3600000UL && !(p.verde() && !p.ambar()); h += PASO_MS) unTick();
  avanzar(2000);   // un verde estable, pasado el rojo+amarillo de apertura (D-53)
  ObsT o;
  o.habiaVerde = p.verde() && !p.ambar();
  if (porManual) MAESTRO.orden("set_modo", manualV);
  else MAESTRO.orden("forzar_rojo_app");   // la rama de bluetooth.cpp: modo_degradado_forzarRojo()
  bool antVerde = p.verde(), cerrado = false;
  for (unsigned long t = 0; t < obsMs; t += PASO_MS) {
    unTick();
    o.ms += PASO_MS;
    const bool v = p.verde(), a = p.ambar(), r = p.rojo();
    if (MAESTRO.verde() && ESCLAVO.verde()) o.simultaneo += PASO_MS;
    if (!cerrado) {
      if (a && !v && !r) o.ambarV += PASO_MS;
      if (antVerde && r) o.directo++;
      if (r && !a && !v) cerrado = true;
    }
    if (t >= CIERRE_S_MS) {
      if (!MAESTRO.rojo()) o.noRojoM += PASO_MS;
      if (!ESCLAVO.rojo()) o.noRojoE += PASO_MS;
    }
    antVerde = v;
  }
  o.modo = MAESTRO.orden("modo_actual");
  return o;
}

static std::string cifrasT(const ObsT& o, long manualV) {
  return "amarillo " + std::to_string(o.ambarV) + " ms (D-45 " + std::to_string(AMARILLO_S_MS) + " +-" +
         std::to_string(TOL_AMARILLO_S_MS) + "), verde->rojo directo " + std::to_string(o.directo) +
         ", verde en las dos " + std::to_string(o.simultaneo) + " ms, sin rojo M " + std::to_string(o.noRojoM) +
         " E " + std::to_string(o.noRojoE) + " ms de " + std::to_string(o.ms) + " ms; modo " + std::to_string(o.modo) +
         " (MANUAL " + std::to_string(manualV) + ")" + (o.habiaVerde ? "" : "; EL VERDE NO SE MONTO");
}

static bool cumpleT(const ObsT& o, long manualV) {
  const long dif = (long)o.ambarV - (long)AMARILLO_S_MS;
  return o.habiaVerde && o.directo == 0 && (dif < 0 ? -dif : dif) <= (long)TOL_AMARILLO_S_MS &&
         o.simultaneo == 0 && o.noRojoM == 0 && o.noRojoE == 0 && o.modo == manualV;
}

static void bloqueT() {
  std::printf("\n--- BLOQUE S: D-173 (ROJO TOTAL desde Automatico e Inteligente: rojo fijo y Manual) -\n");
  const std::string MODOS = RAIZ + "/Maestro/include/modos.h";
  const long MANUAL_V = posicionEnEnum(MODOS, "ModoSistema", "MODO_MANUAL");
  const long AUTO_V = posicionEnEnum(MODOS, "ModoSistema", "MODO_AUTOMATICO");
  const long INTEL_V = posicionEnEnum(MODOS, "ModoSistema", "MODO_INTELIGENTE");
  const unsigned long SILENCIO_MS = leerNumero(RAIZ + "/Maestro/include/protocolo.h",
                                               R"(#define\s+SFTY6_SILENCIO_MS\s+(\d+)UL)", "SFTY6_SILENCIO_MS");
  prepararSincronizadas(15, 8, 0, 0);
  const long tc = MAESTRO.orden("tiempos_ciclo");
  const unsigned long faseMs = (unsigned long)((tc / 10000) % 100 + (tc / 100) % 100) * 60000UL;   // verde + rojo
  unsigned long obsMs = 3UL * faseMs;
  if (obsMs < 4UL * SILENCIO_MS) obsMs = 4UL * SILENCIO_MS;
  obsMs += CIERRE_S_MS;
  const std::string mira = std::to_string(obsMs) + " ms mirados = 3 x (verde + rojo configurados, " +
                           std::to_string(faseMs) + " ms) o 4 x SFTY6_SILENCIO_MS, el mayor";
  const char* nombreVia[2] = { "Maestro", "Esclavo" };
  // T0: el control. SET_MODO:MANUAL desde Automatico con verde: amarillo de 3 s y rojo en los dos.
  for (int quien = 0; quien < 2; quien++) {
    const ObsT o = rojoTotalT(AUTO_V, quien, true, MANUAL_V, obsMs);
    comprobar(cumpleT(o, MANUAL_V), std::string("T0 (control, D-173): SET_MODO:MANUAL desde AUTOMATICO con verde en ") +
                                        nombreVia[quien] + ": " + cifrasT(o, MANUAL_V) + "; " + mira);
  }
  // T1-T4: FORZAR_ROJO desde Automatico e Inteligente, con el verde en cada via.
  const long origenes[2] = { AUTO_V, INTEL_V };
  const char* nombreOrigen[2] = { "AUTOMATICO", "INTELIGENTE" };
  int n = 1;
  for (int k = 0; k < 2; k++) {
    for (int quien = 0; quien < 2; quien++, n++) {
      const ObsT o = rojoTotalT(origenes[k], quien, false, MANUAL_V, obsMs);
      comprobar(cumpleT(o, MANUAL_V), "S" + std::to_string(n) + " (D-173): FORZAR_ROJO desde " + nombreOrigen[k] +
                                          " con verde en " + nombreVia[quien] + ": amarillo de cierre, rojo "
                                          "fijo en los "
                                          "DOS y modo Manual (" + cifrasT(o, MANUAL_V) + "; " + mira + ")");
      if (k == 0 && quien == 0) {   // T1b: de ahi se sale con otra orden; SET_MODO:AUTO vuelve a dar paso
        MAESTRO.orden("set_modo", AUTO_V);
        const ObsJ j = observarJ(1800000UL);
        comprobar((j.verdeM > 0 || j.verdeE > 0) && MAESTRO.orden("modo_actual") == AUTO_V,
                  "T1b (D-173): tras el rojo total, SET_MODO:AUTO vuelve a dar paso (" + cifrasJ(j) + ")");
      }
    }
  }
}
