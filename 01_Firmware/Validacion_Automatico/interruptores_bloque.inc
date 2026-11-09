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

    // -- I7: D-151, CAM:NOVETO solo con la camara sana; datos ASIMETRICOS ---------------
    rojoEstable();
    orden("SET_VETO_CAMARA:OFF");
    // El '?' (ninguna camara ha dado flanco) no se alcanza aqui: los bloques F y G ya dieron
    // flancos y botones_setup() no los olvida. Lo ejerce el arnes de dos puntas (Esclavo).
    deteccionI(CAM_C_PIN);
    deteccionI(CAM_D_PIN);
    const bool ok7 = interruptores_camNoVeto() && std::strcmp(camara_estado(), "OK") == 0;
    auto pegar = [&](int p) {
      cerrarContacto(p, true);
      for (unsigned long h = 0; h <= PEGADA_I + 60000UL; h += 60000UL) {
        arnes_millis_valor += 60000UL;
        botones_actualizar();
      }
    };
    pegar(CAM_C_PIN);   // C en PEGADA y D en OK
    const bool cPeg = !interruptores_camNoVeto() && std::strcmp(camara_estado(), "PEGADA") == 0;
    rojoEstable();
    deteccionI(CAM_C_PIN);
    deteccionI(CAM_D_PIN);
    pegar(CAM_D_PIN);   // y al reves: D en PEGADA y C en OK
    const bool dPeg = !interruptores_camNoVeto() && std::strcmp(camara_estado(), "PEGADA") == 0;
    rojoEstable();
    deteccionI(CAM_C_PIN);
    orden("SET_VETO_CAMARA:ON");
    const bool conVeto = !interruptores_camNoVeto();
    char i7t[300];
    std::snprintf(i7t, sizeof(i7t),
        "I7: con el veto fuera CAM dice NOVETO si camara_estado() es OK; con C PEGADA y D OK, y "
        "con D PEGADA y C OK, dice PEGADA; con el veto puesto, nunca NOVETO (D-151). "
        "Medido OK/C/D/veto: %d%d%d%d", ok7, cPeg, dPeg, conVeto);
    comprobar(ok7 && cPeg && dPeg && conVeto && g_dr4 == 0, i7t);

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
  }
