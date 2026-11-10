  // ===========================================================================
  // BLOQUE K - PARAMETROS DE INSTALACION (N-187, SPEC_8 8, D-161..D-165, P1..P4 del 09/11)
  // ===========================================================================
  // Incluido dentro de main() de arnes_automatico.cpp. Ejerce instalacion.cpp, botones.cpp y
  // semaforo.cpp REALES del Maestro; la pagina es el doble de instalacion_modelo.inc. Las
  // tramas y los bordes (3..5 s, fabrica NA/NA/3) salen de la SPEC y de P3, no del fuente.
  std::printf("\n-- Bloque K: parametros de instalacion de N-187 --\n");
  {
    const unsigned long PASO_K = 100UL, TOL = 3UL * PASO_K;
    const unsigned long RET_FAB = g_retardoPlumaMs;   // fabrica, releida de instalacion.h
    auto correrK = [&](unsigned long ms) {
      for (unsigned long h = 0; h < ms; h += PASO_K) {
        arnes_millis_valor += PASO_K;
        botones_actualizar();
        semaforo_actualizar();
        vigilarEnclavamiento();
      }
    };
    auto pinArriba = [&]() { return arnes_pines[MOTOR_TALANQUERA] == TALANQUERA_ABRIR; };
    auto pinesCam = [&](bool c, bool d) {
      cerrarContacto(CAM_C_PIN, c);
      cerrarContacto(CAM_D_PIN, d);
    };
    auto orden = [&](const char* a) -> std::string {
      const char* r = instalacion_orden(a);
      return r ? r : "(nullptr)";
    };
    // Del VERDE al ROJO y cuanto tarda en bajar la pluma desde que se apaga el amarillo.
    // -1 = no bajo dentro del tope; -2 = ni siquiera subio con el verde.
    auto msBajada = [&](unsigned long tope) -> long {
      semaforo_forzarVerde();
      semaforo_actualizar();
      correrK(RA_MS + 2000UL);
      if (!pinArriba()) return -2;
      semaforo_forzarRojo();
      semaforo_actualizar();
      while (semaforo_estado() == S_AMARILLO) correrK(PASO_K);
      for (unsigned long h = PASO_K; h <= tope; h += PASO_K) {
        correrK(PASO_K);
        if (!pinArriba()) return (long)h;
      }
      return -1;
    };
    auto cerca = [&](long medido, unsigned long esperado) {
      return medido >= (long)(esperado - TOL) && medido <= (long)(esperado + TOL);
    };
    auto consulta = [&]() { return orden("CONSULTA_INSTALACION"); };
    instBorrar();
    pinesCam(false, false);
    arnes_millis_valor += 60000UL;
    botones_setup();

    // -- K0: fabrica = el equipo de antes de N-187 ---------------------------------------
    const std::string k0 = consulta();
    const long b0 = msBajada(RET_FAB * 3UL);
    pinesCam(true, false);
    correrK(PASO_K);
    const bool veNA = camara_presenciaJ16(0) && !camara_presenciaJ16(1);
    pinesCam(false, false);
    correrK(4000UL);
    char k0t[300];
    std::snprintf(k0t, sizeof(k0t),
        "K0: pagina borrada = fabrica: %s, la pluma baja a los %ld ms del rojo (fabrica %lu) y "
        "con NA el pin ALTO es presencia (C alto, D bajo: C=1 D=0 -> %d)", k0.c_str(), b0, RET_FAB, veNA);
    comprobar(k0 == "$ACK,CMD:CONSULTA_INSTALACION,RESULT:C_NA_D_NA_R3,VE:C0D0" && RET_FAB == 3000UL &&
              cerca(b0, RET_FAB) && veNA, k0t);

    // -- K1: formato, SIN_CAMBIO y FUERA_DE_RANGO (molde SET_TIEMPOS) --------------------
    const int esc1 = g_instEscrituras;
    const bool k1 = instalacion_orden("SET_MODO:AUTO") == nullptr &&
        orden("SET_CAMARA:X:NC") == "$ERR,CMD:SET_CAMARA,DESC:FORMATO_INVALIDO" &&
        orden("SET_CAMARA:C:NO") == "$ERR,CMD:SET_CAMARA,DESC:FORMATO_INVALIDO" &&
        orden("SET_CAMARA:C") == "$ERR,CMD:SET_CAMARA,DESC:FORMATO_INVALIDO" &&
        orden("SET_CAMARA:C,NC") == "$ERR,CMD:SET_CAMARA,DESC:FORMATO_INVALIDO" &&
        orden("SET_PLUMA_RETARDO:") == "$ERR,CMD:SET_PLUMA_RETARDO,DESC:FORMATO_INVALIDO" &&
        orden("SET_PLUMA_RETARDO:4s") == "$ERR,CMD:SET_PLUMA_RETARDO,DESC:FORMATO_INVALIDO" &&
        orden("SET_PLUMA_RETARDO:2") == "$ERR,CMD:SET_PLUMA_RETARDO:2,DESC:FUERA_DE_RANGO" &&
        orden("SET_PLUMA_RETARDO:6") == "$ERR,CMD:SET_PLUMA_RETARDO:6,DESC:FUERA_DE_RANGO" &&
        orden("SET_PLUMA_RETARDO:0") == "$ERR,CMD:SET_PLUMA_RETARDO:0,DESC:FUERA_DE_RANGO" &&
        orden("SET_CAMARA:D:NA") == "$ACK,CMD:SET_CAMARA:D:NA,RESULT:SIN_CAMBIO,VE:C0D0" &&
        orden("SET_PLUMA_RETARDO:3") == "$ACK,CMD:SET_PLUMA_RETARDO:3,RESULT:SIN_CAMBIO,VE:C0D0" &&
        g_instEscrituras == esc1 && consulta() == k0;
    comprobar(k1, "K1: otra orden no es de este modulo; texto que no casa es FORMATO_INVALIDO; 2, 6 y 0 "
                  "s son FUERA_DE_RANGO (P3: 3..5); lo ya vigente es SIN_CAMBIO con VE; nada se escribe");

    // -- K2: NC invierte la presencia, por INDICE, con datos asimetricos -----------------
    pinesCam(true, false);   // C alto, D bajo
    correrK(PASO_K);
    const std::string k2 = orden("SET_CAMARA:C:NC");
    const bool k2libre = !camara_presenciaJ16(0) && !camara_presenciaJ16(1);
    pinesCam(false, false);   // C abre: con NC eso es presencia; D sigue NA y bajo
    correrK(PASO_K);
    const std::string k2c = consulta();
    char k2t[360];
    std::snprintf(k2t, sizeof(k2t),
        "K2: SET_CAMARA:C:NC con C alto y D bajo contesta OK con VE:C0D0 (un indice invertido "
        "daria C1D1); al bajar el pin C la presencia es C y no D. Contesto %s / %s", k2.c_str(), k2c.c_str());
    comprobar(k2 == "$ACK,CMD:SET_CAMARA:C:NC,RESULT:OK,VE:C0D0" && k2libre &&
              k2c == "$ACK,CMD:CONSULTA_INSTALACION,RESULT:C_NC_D_NA_R3,VE:C1D0", k2t);

    // -- K3: PLUMA_VETADA. Con C NC abierta (presencia) la pluma se retiene en rojo y la
    //    orden no suelta el veto; el pin no se mueve y la pagina no cambia -----------------
    const long b3 = msBajada(RET_FAB * 3UL);
    const bool vetada3 = semaforo_plumaVetada() && pinArriba();
    const int esc3 = g_instEscrituras;
    const std::string k3 = orden("SET_CAMARA:C:NA");
    char k3t[300];
    std::snprintf(k3t, sizeof(k3t),
        "K3: con la pluma vetada por C (NC, abierta) SET_CAMARA contesta PLUMA_VETADA, no escribe y "
        "la pluma sigue arriba (SPEC_8 8.5). Bajada %ld (-1 = retenida); contesto %s", b3, k3.c_str());
    comprobar(b3 == -1 && vetada3 && k3 == "$ERR,CMD:SET_CAMARA:C:NA,DESC:PLUMA_VETADA" &&
              g_instEscrituras == esc3 && pinArriba() && semaforo_plumaVetada(), k3t);
    pinesCam(true, false);   // C cierra: con NC, libre; la pluma baja
    correrK(RET_FAB * 2UL);

    // -- K4: la re-siembra borra el flanco vigente (C2): sin ella VE saldria C1 ----------
    orden("SET_CAMARA:C:NA");
    pinesCam(false, false);
    correrK(PASO_K);
    pinesCam(true, false);
    correrK(PASO_K);   // flanco real en C (NA)
    pinesCam(false, false);
    correrK(PASO_K);
    const bool vigente = camara_presenciaJ16(0);   // flanco dentro de la ventana de demanda
    pinesCam(true, false);   // C cerrada otra vez, SIN pasar por botones_actualizar()
    const std::string k4 = orden("SET_CAMARA:C:NC");
    char k4t[300];
    std::snprintf(k4t, sizeof(k4t),
        "K4: tras un flanco real en C (vigente=%d), SET_CAMARA:C:NC con C cerrada contesta VE:C0 "
        "de inmediato: la re-siembra reinicia el flanco vigente. Contesto %s", vigente, k4.c_str());
    comprobar(vigente && k4 == "$ACK,CMD:SET_CAMARA:C:NC,RESULT:OK,VE:C0D0", k4t);
    orden("SET_CAMARA:C:NA");
    pinesCam(false, false);
    correrK(4000UL);

    // -- K5: el retardo 5, 4 y 3 se respeta en el pin; 2 y 6 no se aceptan ---------------
    g_retardoPlumaMs = 5000UL;   // la invariante del arnes sigue al valor PEDIDO, no al getter
    const std::string k5a = orden("SET_PLUMA_RETARDO:5");
    const long b5 = msBajada(20000UL);
    g_retardoPlumaMs = 4000UL;
    const std::string k5b = orden("SET_PLUMA_RETARDO:4");
    const long b4 = msBajada(20000UL);
    const std::string k5r = orden("SET_PLUMA_RETARDO:6");
    const long b4b = msBajada(20000UL);
    g_retardoPlumaMs = RET_FAB;
    const std::string k5c = orden("SET_PLUMA_RETARDO:3");
    const long b3b = msBajada(20000UL);
    char k5t[420];
    std::snprintf(k5t, sizeof(k5t),
        "K5: SET_PLUMA_RETARDO 5/4/3 contestan OK y la pluma baja a %ld/%ld/%ld ms del rojo "
        "(+-%lu); el 6 rechazado deja 4 (%ld ms). Contestaron %s | %s | %s | %s",
        b5, b4, b3b, TOL, b4b, k5a.c_str(), k5b.c_str(), k5r.c_str(), k5c.c_str());
    comprobar(k5a == "$ACK,CMD:SET_PLUMA_RETARDO:5,RESULT:OK,VE:C0D0" && cerca(b5, 5000UL) &&
              k5b == "$ACK,CMD:SET_PLUMA_RETARDO:4,RESULT:OK,VE:C0D0" && cerca(b4, 4000UL) &&
              k5r == "$ERR,CMD:SET_PLUMA_RETARDO:6,DESC:FUERA_DE_RANGO" && cerca(b4b, 4000UL) &&
              k5c == "$ACK,CMD:SET_PLUMA_RETARDO:3,RESULT:OK,VE:C0D0" && cerca(b3b, 3000UL), k5t);

    // -- K6: pagina corrupta = FABRICA entera (NA, NA, 3 s) ------------------------------
    instComponer(1, 0, 4, true);
    const bool k6pos = consulta() == "$ACK,CMD:CONSULTA_INSTALACION,RESULT:C_NC_D_NA_R4,VE:C0D0";
    const std::string fab = "$ACK,CMD:CONSULTA_INSTALACION,RESULT:C_NA_D_NA_R3,VE:C0D0";
    instComponer(1, 0, 4, false);   // suma mala
    bool k6 = consulta() == fab;
    const uint16_t malos[] = {0, 2, 6, 0xFFFFU};
    for (uint16_t r : malos) {
      instComponer(1, 1, r, true);   // suma BUENA con el retardo fuera de rango
      k6 = k6 && consulta() == fab && instalacion_retardoPlumaMs() == 3000UL && !instalacion_camaraNC(0);
    }
    instComponer(2, 0, 4, true);   // un contacto que no es 0 ni 1
    k6 = k6 && consulta() == fab;
    instComponer(0, 0, 0, true);   // y en el pin: R=0 con suma valida baja a los 3 s, no en el acto
    const long b6 = msBajada(20000UL);
    char k6t[300];
    std::snprintf(k6t, sizeof(k6t),
        "K6: una pagina valida con R4 se lee R4 (%d, control del formato); suma mala, R en "
        "0/2/6/0xFFFF con suma buena o contacto 2 leen FABRICA; con R=0 la pluma baja a %ld ms", k6pos, b6);
    comprobar(k6pos && k6 && cerca(b6, 3000UL), k6t);

    // -- K7: NO_QUEDO_PUESTO: la escritura no queda y el OK no se da ---------------------
    instBorrar();
    g_instFalla = true;
    const std::string k7 = orden("SET_PLUMA_RETARDO:4");
    const std::string k7c = orden("SET_CAMARA:D:NC");
    g_instFalla = false;
    comprobar(k7 == "$ERR,CMD:SET_PLUMA_RETARDO:4,DESC:NO_QUEDO_PUESTO" &&
              k7c == "$ERR,CMD:SET_CAMARA:D:NC,DESC:NO_QUEDO_PUESTO" && consulta() == fab &&
              instalacion_retardoPlumaMs() == 3000UL,
              "K7: si lo releido de la flash no es lo pedido, $ERR NO_QUEDO_PUESTO y el equipo sigue "
              "en fabrica (molde SET_TIEMPOS: el OK sale de la relectura, CLAUDE.md 2)");

    // -- K9: sin cambio de contacto NO se re-siembra: SET_PLUMA_RETARDO conserva el flanco
    //    vigente (y con el la pegada anunciada y el aviso del veto); K4: el que cambia, si ---
    pinesCam(false, false);
    correrK(PASO_K);
    pinesCam(true, false);
    correrK(PASO_K);   // flanco real en C (NA)
    pinesCam(false, false);   // abierta: la presencia es SOLO el flanco vigente (camHuboFlanco)
    correrK(PASO_K);
    const std::string k9a = orden("SET_PLUMA_RETARDO:4");
    const std::string k9b = orden("SET_PLUMA_RETARDO:3");
    char k9t[300];
    std::snprintf(k9t, sizeof(k9t),
        "K9: tras un flanco real en C ya abierta, SET_PLUMA_RETARDO:4 y :3 contestan VE:C1D0 "
        "(sin re-siembra: el contacto no cambio). Contestaron %s | %s", k9a.c_str(), k9b.c_str());
    comprobar(k9a == "$ACK,CMD:SET_PLUMA_RETARDO:4,RESULT:OK,VE:C1D0" &&
              k9b == "$ACK,CMD:SET_PLUMA_RETARDO:3,RESULT:OK,VE:C1D0" && camara_presenciaJ16(0), k9t);
    correrK(4000UL);

    // -- K8: lo medido en el Maestro es el MISMO fuente que lleva el Esclavo --------------
    bool k8 = true;
    for (const char* f : {"src/instalacion.cpp", "include/instalacion.h", "src/instalacion_flash.cpp",
                          "include/instalacion_flash.h"}) {
      std::ifstream fm(MAESTRO_SRC + "../" + f, std::ios::binary), fe(MAESTRO_SRC + "../../Esclavo/" + f,
                                                                       std::ios::binary);
      std::stringstream sm, se;
      sm << fm.rdbuf();
      se << fe.rdbuf();
      k8 = k8 && fm && fe && !sm.str().empty() && sm.str() == se.str();
    }
    comprobar(k8, "K8: instalacion.cpp/.h e instalacion_flash.cpp/.h son IDENTICOS en las dos puntas "
                  "(SPEC_8 8.6): lo medido aqui en el Maestro vale para el Esclavo");
    instBorrar();
    pinesCam(false, false);
    arnes_millis_valor += 60000UL;
    botones_setup();
    correrK(RET_FAB * 3UL);
  }
