  // ===========================================================================
  // BLOQUE I - LOS DOS INTERRUPTORES DE ADMINISTRADOR (SPEC_8 6: D-94..D-97, D-148..D-151)
  // ===========================================================================
  // Incluido dentro de main() de arnes_automatico.cpp (el arnes no crece: tope de 500).
  // Ejerce interruptores.cpp y semaforo.cpp REALES del Maestro; el DR4 es el doble de
  // interruptores_modelo.inc. Las tramas esperadas salen de SPEC_8 6.1, no del fuente.
  std::printf("\n-- Bloque I: los interruptores de SPEC_8 6 --\n");
  {
    const unsigned long PASO_I = 100UL;
    const unsigned long RET = g_retardoPlumaMs, TOL = 3UL * PASO_I;
    const unsigned long PEGADA_I = (unsigned long)leerConstante("botones.cpp",
        R"(CAM_PEGADA_MS\s*=\s*(\d+)UL)", "el plazo de CAM_PEGADA (D-151)");
    auto correrI = [&](unsigned long ms) {
      for (unsigned long h = 0; h < ms; h += PASO_I) {
        arnes_millis_valor += PASO_I;
        botones_actualizar();   // el orden de main.cpp, como correrG()
        semaforo_actualizar();
        vigilarEnclavamiento();
      }
    };
    auto pinArriba = [&]() { return arnes_pines[MOTOR_TALANQUERA] == TALANQUERA_ABRIR; };
    auto msHastaQueBajeI = [&](unsigned long tope) -> long {
      for (unsigned long h = PASO_I; h <= tope; h += PASO_I) {
        correrI(PASO_I);
        if (!pinArriba()) return (long)h;
      }
      return -1;
    };
    auto orden = [&](const char* a) -> std::string {
      const char* r = interruptores_orden(a);
      const std::string s = r ? r : "(nullptr)";
      if (s == "$ACK,CMD:SET_BARRERA_SERVICIO:OFF,RESULT:OK") g_fueraPorOrden = true;
      if (s == "$ACK,CMD:SET_BARRERA_SERVICIO:ON,RESULT:OK") g_fueraPorOrden = false;
      return s;
    };
    auto rojoEstable = [&]() {
      cerrarContacto(CAM_C_PIN, false);
      cerrarContacto(CAM_D_PIN, false);
      camaraJ14(false);
      arnes_millis_valor += 60000UL;
      botones_setup();
      semaforo_forzarRojo();
      semaforo_actualizar();
      while (semaforo_estado() == S_AMARILLO) correrI(PASO_I);
      correrI(RET * 3UL);
    };
    auto deteccionI = [&](int p) {
      cerrarContacto(p, true);
      arnes_millis_valor += 500UL;
      botones_actualizar();
      cerrarContacto(p, false);
      arnes_millis_valor += 500UL;
      botones_actualizar();
    };
    rojoEstable();
    comprobar(semaforo_estado() == S_ROJO && !pinArriba() && g_dr4 == 0,
              "I0: punto de partida, S_ROJO estable, pluma abajo y los dos interruptores en ON");

    // -- I1: lo que no es de este modulo, el formato y SIN_CAMBIO (D-94, D-65) ----------
    comprobar(interruptores_orden("SET_MODO:AUTO") == nullptr &&
              orden("SET_BARRERA_SERVICIO:XX") == "$ERR,CMD:SET_BARRERA_SERVICIO,DESC:FORMATO_INVALIDO" &&
              orden("SET_VETO_CAMARA:") == "$ERR,CMD:SET_VETO_CAMARA,DESC:FORMATO_INVALIDO" &&
              orden("SET_BARRERA_SERVICIO:ON") == "$ACK,CMD:SET_BARRERA_SERVICIO:ON,RESULT:SIN_CAMBIO" &&
              orden("SET_VETO_CAMARA:ON") == "$ACK,CMD:SET_VETO_CAMARA:ON,RESULT:SIN_CAMBIO" &&
              g_dr4 == 0 && !pinArriba(),
              "I1: otra orden no es de este modulo; otro texto es FORMATO_INVALIDO; ON ya en ON "
              "es SIN_CAMBIO; nada se escribe (D-94, molde SET_TIEMPOS)");

    // -- I2: D-148, el respaldo perdido en este arranque -------------------------------
    g_perdidoAlArrancar = true;
    const std::string i2b = orden("SET_BARRERA_SERVICIO:OFF");
    const std::string i2v = orden("SET_VETO_CAMARA:OFF");
    g_perdidoAlArrancar = false;
    comprobar(i2b == "$ERR,CMD:SET_BARRERA_SERVICIO:OFF,DESC:RESPALDO_INVALIDO" &&
              i2v == "$ERR,CMD:SET_VETO_CAMARA:OFF,DESC:RESPALDO_INVALIDO" &&
              g_dr4 == 0 && !pinArriba(),
              "I2: con el respaldo perdido al arrancar el OFF de las dos ordenes sale con "
              "RESPALDO_INVALIDO y no toca ni DR4 ni el pin (D-148)");

    // -- I3: D-149 + D-96, el OFF en S_ROJO ESTABLE mueve el pin en la MISMA orden -----
    const std::string i3 = orden("SET_BARRERA_SERVICIO:OFF");
    char i3t[300];
    std::snprintf(i3t, sizeof(i3t),
        "I3: SET_BARRERA_SERVICIO:OFF en S_ROJO estable contesta OK y el pin queda en ABRIR "
        "DENTRO de la orden, con el rojo encendido y semaforo_plumaFuera() cierta (D-96 ARRIBA, "
        "D-149 reentrada, D-150). Contesto: %s", i3.c_str());
    comprobar(i3 == "$ACK,CMD:SET_BARRERA_SERVICIO:OFF,RESULT:OK" && pinArriba() &&
              semaforo_plumaFuera() && arnes_pines[ROJO1] == HIGH && (g_dr4 & FLAG_BARRERA_FUERA), i3t);
    const long antesI4 = violacionesTalanquera;
    correrI(RET * 20UL);
    comprobar(pinArriba() && semaforo_plumaFuera() && violacionesTalanquera == antesI4 &&
              orden("SET_BARRERA_SERVICIO:OFF") == "$ACK,CMD:SET_BARRERA_SERVICIO:OFF,RESULT:SIN_CAMBIO",
              "I4: la pluma SIGUE ARRIBA en rojo sin tope de tiempo, la invariante la excusa por el "
              "NOMBRE FLAG_BARRERA_FUERA, y repetir el OFF es SIN_CAMBIO (D-96, D-65)");

    // -- I5: el ON en rojo: OK, y la pluma baja a PLUMA_RETARDO_BAJADA_MS, no al instante
    const std::string i5 = orden("SET_BARRERA_SERVICIO:ON");
    const bool arribaTrasOn = pinArriba(), fueraTrasOn = semaforo_plumaFuera();
    const long baja5 = msHastaQueBajeI(RET * 3UL);
    char i5t[300];
    std::snprintf(i5t, sizeof(i5t),
        "I5: SET_BARRERA_SERVICIO:ON en rojo contesta OK, quita la anotacion FUERA en la misma "
        "orden y la pluma baja a los %lu ms del retardo (+-%lu), no en el acto (D-149). "
        "Contesto: %s; bajo a %ld ms", RET, TOL, i5.c_str(), baja5);
    comprobar(i5 == "$ACK,CMD:SET_BARRERA_SERVICIO:ON,RESULT:OK" && arribaTrasOn && !fueraTrasOn &&
              baja5 >= (long)(RET - TOL) && baja5 <= (long)(RET + TOL) && g_dr4 == 0, i5t);

    // -- I6: D-96, camara fuera del veto: con presencia debajo la pluma BAJA igual -----
    auto verdeYRojoConC = [&](bool presencia) -> long {
      cerrarContacto(CAM_C_PIN, presencia);
      correrI(PASO_I * 5UL);
      semaforo_forzarVerde();
      semaforo_actualizar();
      correrI(RA_MS + 2000UL);
      const bool subio = pinArriba();
      semaforo_forzarRojo();
      semaforo_actualizar();
      while (semaforo_estado() == S_AMARILLO) correrI(PASO_I);
      const long b = msHastaQueBajeI(RET * 4UL);
      return subio ? b : -2;
    };
    const std::string i6 = orden("SET_VETO_CAMARA:OFF");
    const long baja6 = verdeYRojoConC(true);
    const bool vetada6 = semaforo_plumaVetada();
    cerrarContacto(CAM_C_PIN, false);
    rojoEstable();
    const std::string i6on = orden("SET_VETO_CAMARA:ON");
    const long baja6on = verdeYRojoConC(true);   // control: con el veto, NO baja
    cerrarContacto(CAM_C_PIN, false);
    const long baja6fin = msHastaQueBajeI(120000UL);   // suelta la presencia, baja
    char i6t[360];
    std::snprintf(i6t, sizeof(i6t),
        "I6: con SET_VETO_CAMARA:OFF (%s) y la camara C viendo presencia, la pluma baja a los "
        "%ld ms del rojo (retardo %lu) sin veto; con el veto devuelto (%s) la MISMA presencia "
        "la retiene (%ld = no bajo). D-96", i6.c_str(), baja6, RET, i6on.c_str(), baja6on);
    comprobar(i6 == "$ACK,CMD:SET_VETO_CAMARA:OFF,RESULT:OK" && !vetada6 &&
              baja6 >= (long)(RET - TOL) && baja6 <= (long)(RET + TOL) &&
              i6on == "$ACK,CMD:SET_VETO_CAMARA:ON,RESULT:OK" && baja6on == -1 && baja6fin >= 0, i6t);

    // -- I7: D-151 y D-166 (Q6), CAM:NOVETO solo con las camaras NO inhibidas sanas; ASIMETRICOS -----
    // CLAUDE.md 9, contadas sus cuatro propiedades: (1) veto fuera y sanas -> NOVETO, SE CONSERVA (con las dos
    // fuera ya no queda ninguna que juzgar: '?'); (2) y (3) "PEGADA con el veto fuera de las dos" SE INVIERTEN:
    // la averia de una inhibida no tapa el NOVETO; "la averia gana" SE REPARTE a la no inhibida (C fuera, D
    // PEGADA -> PEGADA); (4) con el veto puesto nunca NOVETO, SE CONSERVA.
    rojoEstable();
    orden("SET_VETO_CAMARA:OFF");
    // El '?' por falta de flanco no se alcanza aqui: los bloques F y G ya dieron flancos y botones_setup() no los
    // olvida. Lo ejerce el arnes de dos puntas (Esclavo). Aqui el '?' es el de "ninguna sin inhibir" (D-166).
    deteccionI(CAM_C_PIN);
    deteccionI(CAM_D_PIN);
    const bool ok7 = interruptores_camNoVeto() && std::strcmp(camara_estado(), "?") == 0;
    auto pegar = [&](int p) {
      cerrarContacto(p, true);
      for (unsigned long h = 0; h <= PEGADA_I + 60000UL; h += 60000UL) {
        arnes_millis_valor += 60000UL;
        botones_actualizar();
      }
    };
    pegar(CAM_C_PIN);   // las dos fuera y C en PEGADA: sigue NOVETO
    const bool dosPeg = interruptores_camNoVeto();
    rojoEstable();
    orden("SET_VETO_CAMARA:ON");
    orden("SET_VETO_CAMARA:C:OFF");
    deteccionI(CAM_C_PIN);
    deteccionI(CAM_D_PIN);
    pegar(CAM_C_PIN);   // C fuera y en PEGADA, D en OK: NOVETO, y la peor de las no inhibidas es OK
    const bool cPeg = interruptores_camNoVeto() && std::strcmp(camara_estado(), "OK") == 0;
    rojoEstable();
    deteccionI(CAM_C_PIN);
    deteccionI(CAM_D_PIN);
    pegar(CAM_D_PIN);   // y al reves: C fuera y en OK, D en PEGADA: PEGADA
    const bool dPeg = !interruptores_camNoVeto() && std::strcmp(camara_estado(), "PEGADA") == 0;
    rojoEstable();
    deteccionI(CAM_C_PIN);
    orden("SET_VETO_CAMARA:C:ON");
    const bool conVeto = !interruptores_camNoVeto();
    char i7t[340];
    std::snprintf(i7t, sizeof(i7t),
        "I7: con las dos fuera CAM dice NOVETO ('?') aunque C este PEGADA; con C fuera y PEGADA y D OK, NOVETO; "
        "con C fuera y D PEGADA, PEGADA; con el veto puesto, nunca NOVETO (D-151, D-166). "
        "Medido dos/dosPEG/C/D/veto: %d%d%d%d%d", ok7, dosPeg, cPeg, dPeg, conVeto);
    comprobar(ok7 && dosPeg && cPeg && dPeg && conVeto && g_dr4 == 0, i7t);

    // -- I8: CONTROLES NEGATIVOS de la excepcion por NOMBRE (SPEC_8 6.3, CLAUDE.md 9) ---
    g_aperturaEnPausa = true;
    auto controlNegativo = [&](bool forzarSinOrden) -> bool {
      const long antesV = violacionesTalanquera;
      const unsigned long gV = g_plumaSinVerdeDesde, gP = g_peorVentanaPlumaMs;
      const unsigned long gS = g_peorVentanaSinVetoMs, gN = g_ventanasPluma;
      bool anotacionSigueAlPin = true;
      if (forzarSinOrden) {
        g_dr4 = (uint16_t)(g_dr4 | FLAG_BARRERA_FUERA);   // la bandera, sin la orden
        anotacionSigueAlPin = !semaforo_plumaFuera() && !pinArriba();   // D-150
        semaforo_forzarRojo();   // cualquier repintado de la luz la obedece
      } else {
        orden("SET_BARRERA_SERVICIO:OFF");
        g_dr4 = (uint16_t)(g_dr4 & ~FLAG_BARRERA_FUERA);   // quitada, con la pluma arriba
      }
      const bool arriba = pinArriba();
      correrI(RET + g_margenSenalMs + 5UL * PASO_I);
      const bool caza = violacionesTalanquera > antesV;
      g_dr4 = (uint16_t)(g_dr4 | FLAG_BARRERA_FUERA);
      g_fueraPorOrden = true;
      orden("SET_BARRERA_SERVICIO:ON");
      correrI(RET * 3UL);
      violacionesTalanquera = antesV;
      g_plumaSinVerdeDesde = gV; g_peorVentanaPlumaMs = gP;
      g_peorVentanaSinVetoMs = gS; g_ventanasPluma = gN;
      return anotacionSigueAlPin && arriba && caza && !pinArriba() && g_dr4 == 0;
    };
    rojoEstable();
    const bool cn1 = controlNegativo(true);
    const bool cn2 = controlNegativo(false);
    g_aperturaEnPausa = false;
    comprobar(cn1 && cn2,
              "I8: control negativo de la excepcion: FLAG_BARRERA_FUERA forzada SIN la orden "
              "(la anotacion no la sigue hasta que el pin se mueve, D-150) y quitada con la pluma "
              "arriba en rojo dan violacionesTalanquera > 0; la excepcion no se ensancha a 'con rojo'");

    // ===== D-166 y D-169 (SPEC_8 9): las tramas y bits salen de la SPEC, no del fuente =====
    // -- I9: las ordenes nuevas, molde SET_TIEMPOS, y los bits de SPEC_8 9.4 --------------------------------
    rojoEstable();
    deteccionI(CAM_C_PIN);
    deteccionI(CAM_D_PIN);
    const bool i9a = orden("CONSULTA_VETO") == "$ACK,CMD:CONSULTA_VETO,RESULT:C1O_D1O_M1A_M2A" &&
        orden("SET_VETO_CAMARA:E:OFF") == "$ERR,CMD:SET_VETO_CAMARA,DESC:FORMATO_INVALIDO" &&
        orden("SET_CAMARA_MALA:3:BAJA") == "$ERR,CMD:SET_CAMARA_MALA,DESC:FORMATO_INVALIDO" &&
        orden("SET_CAMARA_MALA:1:XX") == "$ERR,CMD:SET_CAMARA_MALA,DESC:FORMATO_INVALIDO" &&
        orden("SET_CAMARA_MALA:2:ARRIBA") == "$ACK,CMD:SET_CAMARA_MALA:2:ARRIBA,RESULT:SIN_CAMBIO" && g_dr4 == 0;
    g_perdidoAlArrancar = true;
    const bool i9b = orden("SET_CAMARA_MALA:1:BAJA") == "$ERR,CMD:SET_CAMARA_MALA:1:BAJA,DESC:RESPALDO_INVALIDO" &&
        orden("SET_VETO_CAMARA:D:OFF") == "$ERR,CMD:SET_VETO_CAMARA:D:OFF,DESC:RESPALDO_INVALIDO" &&
        orden("SET_CAMARA_MALA:1:ARRIBA") == "$ACK,CMD:SET_CAMARA_MALA:1:ARRIBA,RESULT:SIN_CAMBIO" && g_dr4 == 0;
    g_perdidoAlArrancar = false;
    const bool i9c = orden("SET_VETO_CAMARA:D:OFF") == "$ACK,CMD:SET_VETO_CAMARA:D:OFF,RESULT:OK" && g_dr4 == 0x0800 &&
        orden("SET_CAMARA_MALA:1:BAJA") == "$ACK,CMD:SET_CAMARA_MALA:1:BAJA,RESULT:OK" && g_dr4 == 0x1800 &&
        orden("SET_CAMARA_MALA:2:BAJA") == "$ACK,CMD:SET_CAMARA_MALA:2:BAJA,RESULT:OK" && g_dr4 == 0x3800 &&
        orden("CONSULTA_VETO") == "$ACK,CMD:CONSULTA_VETO,RESULT:C1O_D0O_M1B_M2B" &&
        orden("SET_VETO_CAMARA:OFF") == "$ACK,CMD:SET_VETO_CAMARA:OFF,RESULT:OK" && g_dr4 == 0x3A00 &&
        orden("SET_VETO_CAMARA:ON") == "$ACK,CMD:SET_VETO_CAMARA:ON,RESULT:OK" && g_dr4 == 0x3000 &&
        orden("SET_CAMARA_MALA:1:ARRIBA") == "$ACK,CMD:SET_CAMARA_MALA:1:ARRIBA,RESULT:OK" &&
        orden("SET_CAMARA_MALA:2:ARRIBA") == "$ACK,CMD:SET_CAMARA_MALA:2:ARRIBA,RESULT:OK" && g_dr4 == 0;
    comprobar(i9a && i9b && i9c,
              "I9 (D-166, D-169): CONSULTA_VETO de fabrica C1O_D1O_M1A_M2A; FORMATO_INVALIDO sin valor; ARRIBA "
              "ya puesto es SIN_CAMBIO; con el respaldo perdido BAJA y D:OFF son RESPALDO_INVALIDO; D en 0x0800, "
              "MALA1 en 0x1000, MALA2 en 0x2000, la orden sin camara pone y quita las dos (0x0200|0x0800)");

    // -- I10: el veto POR CAMARA, datos ASIMETRICOS: C fuera y D ve, y al reves (D-166) --------------------
    auto verdeYRojoCon = [&](int pin) -> long {
      cerrarContacto(pin, true);
      correrI(PASO_I * 5UL);
      semaforo_forzarVerde();
      semaforo_actualizar();
      correrI(RA_MS + 2000UL);
      const bool subio = pinArriba();
      semaforo_forzarRojo();
      semaforo_actualizar();
      while (semaforo_estado() == S_AMARILLO) correrI(PASO_I);
      const long b = msHastaQueBajeI(RET * 4UL);
      cerrarContacto(pin, false);
      msHastaQueBajeI(120000UL);
      return subio ? b : -2;
    };
    auto cerca = [&](long b) { return b >= (long)(RET - TOL) && b <= (long)(RET + TOL); };
    rojoEstable();
    orden("SET_VETO_CAMARA:C:OFF");
    const long cFueraVeC = verdeYRojoCon(CAM_C_PIN), cFueraVeD = verdeYRojoCon(CAM_D_PIN);
    orden("SET_VETO_CAMARA:C:ON");
    orden("SET_VETO_CAMARA:D:OFF");
    const long dFueraVeD = verdeYRojoCon(CAM_D_PIN), dFueraVeC = verdeYRojoCon(CAM_C_PIN);
    orden("SET_VETO_CAMARA:D:ON");
    char i10t[300];
    std::snprintf(i10t, sizeof(i10t),
        "I10 (D-166): con C fuera, C viendo baja a %ld ms (retardo %lu) y D viendo la retiene (%ld = no bajo); "
        "con D fuera, D viendo baja a %ld ms y C viendo la retiene (%ld)", cFueraVeC, RET, cFueraVeD,
        dFueraVeD, dFueraVeC);
    comprobar(cerca(cFueraVeC) && cFueraVeD == -1 && cerca(dFueraVeD) && dFueraVeC == -1 && g_dr4 == 0, i10t);

    // -- I11: la demanda POR CAMARA, por flanco (demanda_solicitar(), D-166) ----------------------------------
    auto pide = [&](int pin) {
      arnes_millis_valor += 10000UL;   // fuera de la ventana de la demanda anterior
      deteccionI(pin);
      return demanda_hayLocal();
    };
    orden("SET_VETO_CAMARA:C:OFF");
    const bool cNo = !pide(CAM_C_PIN), dSi = pide(CAM_D_PIN);
    orden("SET_VETO_CAMARA:C:ON");
    orden("SET_VETO_CAMARA:D:OFF");
    const bool dNo = !pide(CAM_D_PIN), cSi = pide(CAM_C_PIN);
    orden("SET_VETO_CAMARA:D:ON");
    comprobar(cNo && dSi && dNo && cSi && pide(CAM_C_PIN) && pide(CAM_D_PIN) && g_dr4 == 0,
              "I11 (D-166): con C fuera, un flanco de C no pide demanda y uno de D si; con D fuera, al reves; "
              "con las dos dentro, las dos piden");

    // -- I12: camara MALA (D-169): MALA1/MALA2 de fabrica ARRIBA, BAJA, y la ACCION del $ALARM ---------------
    rojoEstable();
    deteccionI(CAM_C_PIN);
    deteccionI(CAM_D_PIN);
    semaforo_forzarVerde();
    semaforo_actualizar();
    correrI(RA_MS + 2000UL);
    pegar(CAM_C_PIN);   // C trabada 20 min en verde: PEGADA, con D mirando -> rige MALA1, fabrica ARRIBA
    const std::string acc1 = g_ultimaAlarmaAccion;
    auto aRojo = [&]() {
      semaforo_forzarRojo();
      semaforo_actualizar();
      while (semaforo_estado() == S_AMARILLO) correrI(PASO_I);
      correrI(RET * 4UL);
    };
    aRojo();
    const bool m1Arriba = pinArriba() && semaforo_plumaRetenidaMala() && !semaforo_plumaVetada();
    const std::string m1b = orden("SET_CAMARA_MALA:1:BAJA");   // la reentrada la baja en la orden
    const bool m1Baja = !pinArriba();
    semaforo_forzarVerde();
    semaforo_actualizar();
    correrI(RA_MS + 2000UL);
    cerrarContacto(CAM_D_PIN, true);   // la buena ve: retiene ELLA, como veto
    aRojo();
    const bool dRetiene = pinArriba() && semaforo_plumaVetada() && !semaforo_plumaRetenidaMala();
    cerrarContacto(CAM_D_PIN, false);
    const long dSuelta = msHastaQueBajeI(120000UL);
    orden("SET_VETO_CAMARA:D:OFF");   // C mala y D fuera: no queda ninguna -> MALA2, fabrica ARRIBA
    semaforo_forzarVerde();
    semaforo_actualizar();
    correrI(RA_MS + 2000UL);
    aRojo();
    const bool m2Arriba = pinArriba() && semaforo_plumaRetenidaMala();
    const std::string m2b = orden("SET_CAMARA_MALA:2:BAJA");
    const bool m2Baja = !pinArriba();
    orden("SET_VETO_CAMARA:OFF");   // las dos fuera: D-96, baja sin leer MALA2
    orden("SET_CAMARA_MALA:2:ARRIBA");
    semaforo_forzarVerde();
    semaforo_actualizar();
    correrI(RA_MS + 2000UL);
    aRojo();
    const bool dosFueraBaja = !pinArriba() && !semaforo_plumaRetenidaMala();
    orden("SET_VETO_CAMARA:ON");
    rojoEstable();   // re-siembra: C deja de estar PEGADA
    deteccionI(CAM_C_PIN);
    deteccionI(CAM_D_PIN);
    pegar(CAM_C_PIN);   // MALA1 = BAJA: la camara se ignora
    const std::string acc2 = g_ultimaAlarmaAccion;
    rojoEstable();
    orden("SET_VETO_CAMARA:C:OFF");
    deteccionI(CAM_C_PIN);
    pegar(CAM_C_PIN);   // inhibida: ya estaba ignorada
    const std::string acc3 = g_ultimaAlarmaAccion;
    rojoEstable();
    orden("SET_VETO_CAMARA:C:ON");
    orden("SET_CAMARA_MALA:1:ARRIBA");
    char i12t[460];
    std::snprintf(i12t, sizeof(i12t),
        "I12 (D-169): C PEGADA y D mirando, MALA1 de fabrica: ACCION %s y la pluma ARRIBA en rojo sin veto (%d); "
        "MALA1:BAJA (%s) la baja en la orden (%d); D viendo la retiene como veto (%d) y suelta en %ld ms; C mala y D "
        "fuera, MALA2 de fabrica ARRIBA (%d), MALA2:BAJA la baja (%d); las dos fuera baja (%d); ACCION con BAJA %s, "
        "inhibida %s", acc1.c_str(), m1Arriba, m1b.c_str(), m1Baja, dRetiene, dSuelta, m2Arriba, m2Baja,
        dosFueraBaja, acc2.c_str(), acc3.c_str());
    comprobar(acc1 == "PLUMA_ARRIBA" && m1Arriba && m1b == "$ACK,CMD:SET_CAMARA_MALA:1:BAJA,RESULT:OK" && m1Baja &&
              dRetiene && dSuelta >= 0 && m2Arriba && m2b == "$ACK,CMD:SET_CAMARA_MALA:2:BAJA,RESULT:OK" &&
              m2Baja && dosFueraBaja && acc2 == "IGNORA_CAMARA" && acc3 == "NINGUNA" && g_dr4 == 0, i12t);

    // -- I13: la demanda por NIVEL del Modo Inteligente (demandaLocalS1), inhibida y mala (D-166, D-169, Q7) ---
    {
      const long FACTOR13 = leerConstante("modo_inteligente.cpp", R"(TECHO_POR_SUELO\s*=\s*(\d+))",
                                          "el factor del techo del Modo Inteligente (I13)");
      const unsigned long V13 = 6UL * 60000UL, TECHO13 = V13 * (unsigned long)FACTOR13, P13 = 500UL;
      const unsigned long TOL13 = 3UL * P13;
      static int pinPulso = -1;   // la camara que pulsa como una cola de vehiculos (Bloque F)
      auto paso13 = [](){
        static int fase = 0;
        fase = (fase + 1) % 3;
        if (pinPulso >= 0) arnes_pines[pinPulso] = (fase == 0) ? LOW : HIGH;
        botones_actualizar();
        modoInteligente_loop();
      };
      auto verde13 = [&](int pin) {
        pinPulso = pin;
        bombearGenerico(P13, TECHO13 * 3UL, paso13, [](){ return semaforo_estado() != S_VERDE; });
        const long v = medirFase(paso13, S_VERDE, P13, TECHO13 * 3UL);
        pinPulso = -1;
        if (pin >= 0) cerrarContacto(pin, false);   // la cola se va: el pin no se queda en alto
        return v;
      };
      auto es = [&](long v, unsigned long ms) {
        return v >= 0 && (unsigned long)v + TOL13 >= ms && (unsigned long)v <= ms + TOL13;
      };
      const bool cfg = configurarTiempos(6, 5, 10);
      arrancarInteligente();
      orden("SET_VETO_CAMARA:C:OFF");
      const long cFuera = verde13(CAM_C_PIN), dDentro = verde13(CAM_D_PIN);
      orden("SET_VETO_CAMARA:C:ON");
      orden("SET_CAMARA_MALA:1:BAJA");   // sin pluma retenida en el ciclo: aqui se mide la demanda
      cerrarContacto(CAM_C_PIN, true);   // C trabada hasta que el vigilante la declare PEGADA
      bombearGenerico(P13, PEGADA_I + 120000UL, [](){ botones_actualizar(); modoInteligente_loop(); },
                      [](){ return camara_letra(0) == 'P'; });
      const long cMala = verde13(-1), dConMala = verde13(CAM_D_PIN);
      const bool sigue = g_modoActual == MODO_INTELIGENTE;
      cerrarContacto(CAM_C_PIN, false);
      cerrarContacto(CAM_D_PIN, false);
      orden("SET_CAMARA_MALA:1:ARRIBA");
      char i13t[420];
      std::snprintf(i13t, sizeof(i13t),
          "I13 (D-166, D-169, Q7): en Inteligente con C fuera y pulsando el verde dura %ld ms (suelo %lu) y con D "
          "pulsando %ld (techo %lu); con C PEGADA el verde dura %ld (suelo) y con D pulsando %ld (techo); el modo "
          "sigue en INTELIGENTE (%d)", cFuera, V13, dDentro, TECHO13, cMala, dConMala, sigue);
      comprobar(cfg && es(cFuera, V13) && es(dDentro, TECHO13) && es(cMala, V13) && es(dConMala, TECHO13) &&
                sigue && g_dr4 == 0, i13t);
    }
    rojoEstable();
  }
