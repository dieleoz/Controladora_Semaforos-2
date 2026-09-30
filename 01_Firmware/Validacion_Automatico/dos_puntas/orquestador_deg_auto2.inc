// ===== Validacion_Automatico/dos_puntas/orquestador_deg_auto2.inc =====
//
// BLOQUE H, SEGUNDA PARTE (29/09): H10..H14. Incluido desde orquestador_deg_auto.inc, justo
// antes de bloqueH(), que llama a bloqueH2(). Usa sus primitivas -montarH, cortarH, pasoH,
// correrH, asevera_no_entran, transversalH-. Vive aparte para que aquel no pase de 500 lineas.

// Espera hasta msMax o hasta que cond() se cumpla sobre las puntas; devuelve si se cumplio.
static bool hastaH(ObsH& o, unsigned long msMax, bool (*cond)()) {
  for (unsigned long t = 0; t < msMax && !cond(); t += PASO_MS) pasoH(o);
  return cond();
}
static bool rendidoE() { return ESCLAVO.orden("da_rendido") == 1; }
static int g_hRech = 1;
static bool rechazoX() { return puntaH(g_hRech).orden("da_rechazos") > 0; }
static void correrMs(ObsH& o, unsigned long ms) { for (unsigned long t = 0; t < ms; t += PASO_MS) pasoH(o); }
static long inicioManual() { return (MAESTRO.orden("segundos_del_dia") + 719L) / 300L * 300L % 86400L; }

// H10..H14: rendicion y reinicio, salida a mano, el literal de NO_GUARDADO, MANUAL durante el
// corte y el APTO en 0 por cada causa. H10 y H11 se vieron en ROJO antes de su arreglo.
static void bloqueH2(unsigned long VENTANA, unsigned long SIN_ENTRADA) {
  std::string ctl;
  bool ap = false;
  // H10 (arquitecto 2): el Esclavo se rinde -su ESP32 calla y la hora caduca- y se reinicia con la
  // radio muerta. La rendicion queda en la pila y no reentra solo sin un intercambio sano.
  {
    CfgH c;
    ObsH o = correrH(c, VENTANA, ctl, ap);
    g_esp32Vivo[1] = false;
    const bool rindio = hastaH(o, 1800000UL, rendidoE);
    g_esp32Vivo[1] = true;
    reiniciarEsclavoH(o);
    const long pila = ESCLAVO.orden("da_rendido_pila");
    correrMs(o, H_ESPERA_MS + 600000UL);
    comprobar(rindio && pila == 1 && !ESCLAVO.orden("da_en_degradado") && ESCLAVO.orden("da_entradas") == 0,
              "H10 rendicion + reinicio del Esclavo sin radio: no reentra solo (se rindio " +
                  std::string(rindio ? "si" : "NO") + ", rendicion en la pila tras reiniciar " + std::to_string(pila) +
                  ", AUTO_ENTRADA tras reiniciar " + std::to_string(ESCLAVO.orden("da_entradas")) +
                  ", en Degradado " + std::to_string(ESCLAVO.orden("da_en_degradado")) + ")");
  }
  // H11 (arquitecto 3): testigo MANUAL en los dos con la radio muerta y el operario saca un poste
  // a mano; sin intercambio sano no reentra solo. H11b: el Maestro, despues puesto en AUTOMATICO.
  for (int quien = 0; quien < 2; quien++) {
    CfgH c;
    ObsH o;
    ctl = montarH(c, ap);
    cortarH(c, o);
    correrMs(o, 60000);
    const long ini = inicioManual();
    const long tm = MAESTRO.orden("da_testigo", ini), te = ESCLAVO.orden("da_testigo", ini);
    correrMs(o, 60000);
    if (quien == 1) ESCLAVO.orden("degradado_salir");
    else MAESTRO.orden("deg_pedir_salida");
    correrMs(o, 90000);
    if (quien == 0) MAESTRO.orden("set_modo", MODO_AUTOMATICO_V);
    correrMs(o, H_ESPERA_MS + 300000UL);
    Punta& p = puntaH(quien);
    comprobar(tm == 1 && te == 1 && !p.orden("da_en_degradado") && p.orden("da_entradas") == 0,
              std::string(quien ? "H11" : "H11b") + " testigo a mano y salida a mano del " +
                  (quien ? "Esclavo" : "Maestro (luego AUTOMATICO)") + " sin radio: no reentra solo (testigo M " +
                  std::to_string(tm) + " / E " + std::to_string(te) + ", AUTO_ENTRADA " +
                  std::to_string(p.orden("da_entradas")) + ", en Degradado " +
                  std::to_string(p.orden("da_en_degradado")) + ")");
  }
  // H12 (literal): la flash del testigo de una punta falla. La ACCION de su $ALARM dice lo que la
  // luz hace en los 10 s siguientes: SIGUE_AMBAR con el ambar, QUEDA_ROJO con el rojo.
  for (int i = 0; i < 2; i++) {
    CfgH c;
    ObsH o;
    ctl = montarH(c, ap);
    Punta& p = puntaH(i);
    p.orden("da_flash_falla", 1);
    cortarH(c, o);
    g_hRech = i;
    const bool rech = hastaH(o, SIN_ENTRADA, rechazoX);
    unsigned long msRojo = 0, msAmbar = 0;
    for (unsigned long t = 0; t < 10000; t += PASO_MS) {
      pasoH(o);
      if (p.rojo()) msRojo += PASO_MS;
      if (ambarH(p)) msAmbar += PASO_MS;
    }
    const long lr = p.orden("da_accion_es:QUEDA_ROJO"), la = p.orden("da_accion_es:SIGUE_AMBAR");
    comprobar(rech && p.orden("da_rechazo_es:AUTO_NO_GUARDADO") && ((lr && msRojo == 10000) || (la && msAmbar == 10000)),
              std::string("H12") + (i ? "" : "b") + " NO_GUARDADO en el " + (i ? "Esclavo" : "Maestro") +
                  ": la ACCION dice lo que hace la luz (QUEDA_ROJO " + std::to_string(lr) + ", SIGUE_AMBAR " +
                  std::to_string(la) + "; en 10 s: rojo " + std::to_string(msRojo) + " ms, ambar " +
                  std::to_string(msAmbar) + " ms)");
  }
  // H13 (riesgo residual (f).1, NO falla el arnes): el Maestro pasa a MANUAL durante el corte. Se
  // registra cuanto verde da contra el ambar o el Degradado del otro; la app avisa de paleteros.
  {
    CfgH c;
    ObsH o;
    ctl = montarH(c, ap);
    cortarH(c, o);
    correrMs(o, 100000);
    MAESTRO.orden("set_modo", MODO_MANUAL_V);
    correrMs(o, VENTANA - 100000UL);
    bool ok = false;
    nota("H13 (medida, riesgo residual (f).1) Maestro a MANUAL a los 100 s del corte: Esclavo " +
         cuando(o, 1) + ", Maestro " + cuando(o, 0) + "; " + transversalH(ok));
  }
  // H14: el APTO en 0 por cada causa, de una en una, oido antes del corte: no entra ninguna.
  for (int k = 0; k < 3; k++) {
    CfgH c;
    ObsH o;
    c.esp32E = (k != 2);
    ctl = montarH(c, ap);
    if (k == 0) {   // reloj no fiable: el ESP32 del Maestro calla hasta que caduca su hora
      g_esp32Vivo[0] = false;
      avanzar((unsigned long)MAESTRO.orden("hora_caduca_ms") + 60000UL);
    } else if (k == 1) {   // ambar de emergencia puesto en el Esclavo
      ESCLAVO.orden("ambar_bluetooth", 1);
      avanzar(30000);
    }   // k == 2: el J17 del Esclavo mudo desde el montaje -sin fecha no hay testigo- (el H4 de a0d605b)
    const std::string ap2 = "APTO M " + std::to_string(MAESTRO.orden("da_apto")) + " / E " +
                            std::to_string(ESCLAVO.orden("da_apto"));
    cortarH(c, o);
    // Y el J17 mudo vuelve tras el corte: ese APTO pasa a 1 sin que el otro lo oiga (ECO en 0).
    if (k != 1) g_esp32Vivo[k == 0 ? 0 : 1] = true;
    correrMs(o, SIN_ENTRADA);
    static const char* QUE[3] = {"H14a reloj del Maestro no fiable", "H14b ambar de emergencia en el Esclavo",
                                 "H14c el Esclavo sin fecha (su J17 mudo)"};
    asevera_no_entran(QUE[k], "APTO en 0 antes del corte", o, ap2);
  }
}

