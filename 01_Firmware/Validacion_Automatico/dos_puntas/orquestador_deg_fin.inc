// ===== Validacion_Automatico/dos_puntas/orquestador_deg_fin.inc =====
//
// BLOQUE J - EL ROJO TOTAL DEL DEGRADADO (D-51 corregida, SPEC_2 7.quater (b), (i)): pruebas ANTERIORES
// al codigo, en ROJO hasta construirlo. Los valores salen de la SPEC (luces, ms, modo), ninguno del fuente.
// Incluido desde orquestador_degradado.cpp tras el bloque H: usa unTick, prepararSincronizadas,
// cortarLasDos, ambarH, transversalH y comprobar. Vive aparte porque aquel pasa de 500 lineas.
//
// J1-J3 y J4-J6 (salida programada: firmas de SPEC_2 7.quater (g), por las ordenes da_salida_* de los
// adaptadores, activas con -DARNES_SALIDA_D52). Sin las firmas en los .h, J4-J6 es UN rojo que lo dice.

struct ObsJ {
  unsigned long ms = 0, verdeM = 0, verdeE = 0, ambarM = 0, rojoM = 0;
};

static ObsJ observarJ(unsigned long ms) {
  ObsJ o;
  for (unsigned long h = 0; h < ms; h += PASO_MS) {
    unTick();
    o.ms += PASO_MS;
    if (MAESTRO.verde()) o.verdeM += PASO_MS;
    if (ESCLAVO.verde()) o.verdeE += PASO_MS;
    if (ambarH(MAESTRO)) o.ambarM += PASO_MS;
    if (MAESTRO.rojo()) o.rojoM += PASO_MS;
  }
  return o;
}

static std::string cifrasJ(const ObsJ& o) {
  return "verde M " + std::to_string(o.verdeM) + " ms, verde E " + std::to_string(o.verdeE) + " ms, ambar M " +
         std::to_string(o.ambarM) + " ms, rojo M " + std::to_string(o.rojoM) + " de " + std::to_string(o.ms) + " ms";
}

// Las dos alternando en Degradado y el Maestro llevado a su verde (la orden se da con luz en verde).
static bool alternandoEnVerdeM() {
  prepararSincronizadas(15, 8, 0, 0);
  entrarEnDegradadoLasDos(0);
  ceroH();
  reiniciarObservacion();
  for (unsigned long h = 0; h < 600000UL && !MAESTRO.verde(); h += PASO_MS) unTick();
  return MAESTRO.verde();
}

// Corte de las dos con siembra a +1000 ms, dentro de la ventana de D-29 (como G0).
static void corteConSiembra() {
  cortarLasDos();
  g_esp32Ds3231 = true;
  g_esp32Vivo[0] = g_esp32Vivo[1] = true;
  g_proxSiembra[0] = g_proxSiembra[1] = g_t + 1000;
}

// RRT_ROJO_FIJO sale de la SPEC (SPEC_2 7.quater firmas), no del fuente: es el acuse que B8 y J1 exigen.
static long rrtRojoFijo() {
  return (long)leerNumero(RAIZ + "/../05_Funcional/SPEC_2_Dos_Puntas_y_Radio.md",
                          R"(RRT_ROJO_FIJO\s*=\s*(\d+))", "RRT_ROJO_FIJO de la SPEC");
}

#ifdef ARNES_SALIDA_D52
// J4-J6: SALIDA PROGRAMADA (D-52). Valores de MotivoSalida = orden del enum de la SPEC (g), no del .h.
static const long MDF_REPROGRAMADA_V = 1, MDF_SIN_RESPALDO_V = 2, MDF_CANCELADA_V = 3, MDF_EN_VERDE_V = 10,
                  MDF_NADA_V = 12;
static bool progOk(long m) { return m == 0 || m == MDF_SIN_RESPALDO_V; }   // PROGRAMADA o PROGRAMADA_SIN_RESPALDO

static void hastaDia(Punta& p, long dia, unsigned long maxMs) {
  for (unsigned long h = 0; h < maxMs && p.orden("segundos_del_dia") < dia; h += PASO_MS) unTick();
}
static void esperarRojo(Punta& p) {
  for (unsigned long h = 0; h < 600000UL && !p.rojo(); h += PASO_MS) unTick();
}
// La flash solo se escribe con el poste en rojo (SPEC (c)): se espera su rojo y se programa.
static long progEnRojo(Punta& p, long dia) {
  esperarRojo(p);
  return p.orden("da_salida_programar", dia);
}
// Las dos alternando y con la MISMA salida (HH:MM:SS) puesta en cada una, cada cual con su reloj.
static bool montarSalida(long delta, long adelanteS, long& salida) {
  prepararSincronizadas(15, 8, 0, 0);
  entrarEnDegradadoLasDos(delta);
  salida = MAESTRO.orden("segundos_del_dia") + adelanteS;
  const long m = progEnRojo(MAESTRO, salida);
  return progOk(m) && progOk(progEnRojo(ESCLAVO, salida));
}

static void bloqueJ456(unsigned long despejeS) {
  // J4: delta 0, +6, -6 s: de salida-60 s a salida+despeje+60 s ni verde/verde ni verde/ambar; al final, las dos en ambar.
  for (long delta : {0L, 6L, -6L}) {
    long s = 0;
    const bool prog = montarSalida(delta, 1500, s);
    hastaDia(MAESTRO, s - 60, 1500000UL);
    ceroH();
    reiniciarObservacion();
    avanzar((120UL + despejeS) * 1000UL + 90000UL);   // la ventana de SPEC y 90 s para el ambar por silencio
    bool okT = false;
    const std::string tr = transversalH(okT);
    comprobar(prog && okT && MAESTRO.orden("modo_actual") == MENU_V && ESCLAVO.orden("da_en_degradado") == 0 &&
                  ambarH(MAESTRO) && ambarH(ESCLAVO),
              "J4 (D-52, delta " + std::to_string(delta) + " s): las dos salen a su hora sin verde contra verde ni "
              "contra ambar y acaban en ambar (programadas " + (prog ? "si" : "NO") + "; " + tr + ")");
  }
  // J5: corte DESPUES de salida: no reanuda (la salida iniciada ya bajo el permiso).
  {
    long s = 0;
    const bool prog = montarSalida(0, 1500, s);
    hastaDia(MAESTRO, s + 10, 1800000UL);
    corteConSiembra();
    const ObsJ o = observarJ(600000UL);
    comprobar(prog && o.verdeM == 0 && o.verdeE == 0 && MAESTRO.orden("modo_actual") != MODO_DEGRADADO_V &&
                  MAESTRO.orden("respaldo_degradado") == 0,
              "J5 (D-52, corte DESPUES de salida): ninguna reanuda (" + cifrasJ(o) + ", permiso " +
              std::to_string(MAESTRO.orden("respaldo_degradado")) + ")");
  }
  // J5b: corte ANTES de salida, Degradado SIN registro de testigo (SPEC (d)): reanuda y la salida se pierde.
  // El corte ANTES con registro de testigo (reanuda y sale a la hora) NO se puede ejercer aqui: la pagina de flash
  // del testigo es un doble en RAM que el FreeLibrary borra (adaptador_*_testigo1.inc). Va al banco de dos tarjetas.
  {
    long s = 0;
    const bool prog = montarSalida(0, 1500, s);
    corteConSiembra();
    const ObsJ o = observarJ(900000UL);
    comprobar(prog && o.verdeM > 0 && o.verdeE > 0 && MAESTRO.orden("da_salida_s") == 0 &&
                  MAESTRO.orden("modo_actual") == MODO_DEGRADADO_V,
              "J5b (D-52 (d)): corte antes de salida SIN registro de testigo: reanudan y alternan, y la salida se "
              "pierde (salidaS " + std::to_string(MAESTRO.orden("da_salida_s")) + "; " + cifrasJ(o) + ")");
  }
  // J6a: reprogramar sale a la hora nueva; y tras salir se vuelve a entrar con el testigo y se programa otra.
  {
    long s = 0;
    const bool prog = montarSalida(0, 1500, s);
    const long m2 = progEnRojo(MAESTRO, s + 600);
    const bool repro = m2 == MDF_REPROGRAMADA_V && MAESTRO.orden("da_salida_s") == s + 600;
    hastaDia(MAESTRO, s + 30, 1800000UL);
    const bool sigueEnS = MAESTRO.orden("modo_actual") == MODO_DEGRADADO_V;
    hastaDia(MAESTRO, s + 620, 1800000UL);
    const bool salioEnS2 = MAESTRO.orden("modo_actual") != MODO_DEGRADADO_V;
    comprobar(prog && repro && sigueEnS && salioEnS2,
              "J6 (D-52): reprogramar -> REPROGRAMADA (" + std::to_string(m2) + "); no sale a la hora vieja y SI a la nueva");
    avanzar(120000UL);
    const long ini = MAESTRO.orden("segundos_del_dia") + 60;
    const bool entro = MAESTRO.orden("da_testigo", ini) == 1 && MAESTRO.orden("da_salida_s") == 0;
    const long m3 = progEnRojo(MAESTRO, ini + 600);
    comprobar(entro && progOk(m3),
              "J6 (D-52): tras salir se vuelve a entrar con el testigo (" + std::string(entro ? "si" : "NO") +
              ") y se puede programar otra salida (motivo " + std::to_string(m3) + ")");
  }
  // J6b: cancelar en rojo no sale a la hora vieja.
  {
    long s = 0;
    const bool prog = montarSalida(0, 1500, s);
    esperarRojo(MAESTRO);
    const long c = MAESTRO.rojo() ? MAESTRO.orden("da_salida_cancelar") : -1;
    hastaDia(MAESTRO, s + 30, 1800000UL);
    comprobar(prog && c == MDF_CANCELADA_V && MAESTRO.orden("da_salida_s") == 0 &&
                  MAESTRO.orden("modo_actual") == MODO_DEGRADADO_V,
              "J6 (D-52): cancelar en rojo -> CANCELADA (" + std::to_string(c) + ") y a la hora vieja NO sale");
  }
  // J6c: cancelar sin nada, y programar con la luz en verde.
  {
    prepararSincronizadas(15, 8, 0, 0);
    entrarEnDegradadoLasDos(0);
    esperarRojo(MAESTRO);
    const long nada = MAESTRO.orden("da_salida_cancelar");
    for (unsigned long h = 0; h < 600000UL && !MAESTRO.verde(); h += PASO_MS) unTick();
    const long sd = MAESTRO.orden("segundos_del_dia") + 900;
    const long enVerde = MAESTRO.verde() ? MAESTRO.orden("da_salida_programar", sd) : -1;
    comprobar(nada == MDF_NADA_V && enVerde == MDF_EN_VERDE_V && MAESTRO.orden("da_salida_s") == 0,
              "J6 (D-52): cancelar sin nada -> NADA_QUE_CANCELAR (" + std::to_string(nada) + "); programar con la luz "
              "en verde -> EN_VERDE (" + std::to_string(enVerde) + ") y no queda programada");
  }
}
#else
static void bloqueJ456(unsigned long) {
  comprobar(false, "J4-J6 (D-52): SIN CONSTRUIR: modo_degradado_programarSalida() no esta en modo_degradado.h de las "
                   "dos puntas (compilar_degradado.ps1 no activa -DARNES_SALIDA_D52). Rojo hasta que exista");
}
#endif

static void bloqueJ(unsigned long despejeS) {
  std::printf("\n--- BLOQUE J: D-51 CORREGIDA, EL ROJO TOTAL DEL DEGRADADO (SPEC_2 7.quater) -\n");
  const unsigned long OBS = 600000UL;   // SPEC (i) J1: 600 s

  // J1: FORZAR_ROJO en el verde del Maestro = rojo fijo en ESE poste; el Esclavo sigue alternando.
  {
    const bool habiaVerde = alternandoEnVerdeM();
    const long acuse = MAESTRO.orden("forzar_rojo_app");
    ObsJ o;
    for (unsigned long h = 0; h < OBS; h += PASO_MS) {   // amarillo de D-45 (3 s) fuera de la cuenta
      unTick();
      o.ms += PASO_MS;
      if (h > 3000UL + PASO_MS && MAESTRO.verde()) o.verdeM += PASO_MS;
      if (ESCLAVO.verde()) o.verdeE += PASO_MS;
    }
    bool okT = false;
    const std::string tr = transversalH(okT);
    const long modo = MAESTRO.orden("modo_actual");
    comprobar(habiaVerde && acuse == rrtRojoFijo() && o.verdeM == 0 && o.verdeE > 0 && MAESTRO.rojo() && modo == MODO_DEGRADADO_V && okT,
              "J1 (D-51): FORZAR_ROJO en el verde del Maestro: rojo fijo en ESE poste (" + cifrasJ(o) +
              "; modo " + std::to_string(modo) + ", MODO_DEGRADADO " + std::to_string(MODO_DEGRADADO_V) + "), el Esclavo sigue alternando; " + tr);
  }

  // J2: corte tras J1: arranca en rojo fijo y NO reanuda (FLAG_ROJO_SIN_HORA de D-47). Control: el Esclavo
  // reanuda y da verde (G0), o "el Maestro no da verde" valdria con un escenario muerto.
  {
    alternandoEnVerdeM();
    MAESTRO.orden("forzar_rojo_app");
    avanzar(60000);
    corteConSiembra();
    const ObsJ o = observarJ(900000UL);
    const long modo = MAESTRO.orden("modo_actual");
    comprobar(o.verdeE > 0, "J2.0 (control): tras el corte el Esclavo reanuda y da verde (" + cifrasJ(o) + ")");
    comprobar(o.verdeM == 0 && o.ambarM == 0 && MAESTRO.rojo() && modo == MODO_DEGRADADO_V &&
                  MAESTRO.orden("respaldo_degradado") == 0,
              "J2 (D-51, D-47): corte tras el rojo total: el Maestro arranca en rojo fijo, no reanuda y no cae a "
              "ambar (" + cifrasJ(o) + "; modo " + std::to_string(modo) + ", permiso de la pila " +
              std::to_string(MAESTRO.orden("respaldo_degradado")) + ")");
  }

  // J3: la ventana de D-29. Maestro en MENU con permiso, esperando la siembra: FORZAR_ROJO revoca el permiso y
  // entra en ROJO FIJO (ni verde ni ambar) en los 1800 s de SPEC (i). Control sin la orden: reanuda (G0).
  for (int conOrden = 1; conOrden >= 0; conOrden--) {
    prepararSincronizadas(15, 8, 0, 0);
    activarDs3231(1000, 7000);
    entrarEnDegradadoLasDos(0);
    avanzar(300000UL);
    corteConSiembra();
    const bool ventana = MAESTRO.orden("modo_actual") == MENU_V && MAESTRO.orden("respaldo_degradado") == 1;
    if (conOrden) MAESTRO.orden("forzar_rojo_app");
    const ObsJ o = observarJ(1800000UL);
    const long modo = MAESTRO.orden("modo_actual"), pila = MAESTRO.orden("respaldo_degradado");
    if (conOrden) {
      comprobar(ventana && o.verdeM == 0 && o.ambarM == 0 && o.ms - o.rojoM <= 1000UL &&
                    modo == MODO_DEGRADADO_V && pila == 0,
                "J3 (D-51, D-29): Maestro en MENU con permiso + FORZAR_ROJO + siembra: la LUZ del Maestro es rojo "
                "fijo sostenido (" + cifrasJ(o) + "; tolerancia 1000 ms sin rojo: la vuelta de arranque), modo " +
                std::to_string(modo) + " (MODO_DEGRADADO " + std::to_string(MODO_DEGRADADO_V) + ") y permiso " + std::to_string(pila) +
                (ventana ? "" : "; LA VENTANA DE D-29 NO SE MONTO"));
    } else {
      comprobar(ventana && o.verdeM > 0 && modo == MODO_DEGRADADO_V && pila == 1,
                "J3.0 (control, G0): sin la orden la siembra REANUDA el Degradado y el Maestro da verde (" +
                cifrasJ(o) + "; modo " + std::to_string(modo) + ", permiso " + std::to_string(pila) + ")");
    }
  }
  bloqueJ456(despejeS);
}
