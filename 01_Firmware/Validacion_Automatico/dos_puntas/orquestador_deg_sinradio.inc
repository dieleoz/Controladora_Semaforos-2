// ===== Validacion_Automatico/dos_puntas/orquestador_deg_sinradio.inc =====
//
// BLOQUE U - D-174 (responsable 10/11): "ROJO TOTAL sin radio, como antes". FORZAR_ROJO desde AUTOMATICO o
// INTELIGENTE pasa a MANUAL (D-173) SOLO con radio; sin ella, rojo en ESTE poste y el modo NO cambia, para que la
// puerta del Degradado automatico (deg_auto.cpp puertaAbierta(): AUTO, INTELIGENTE, NOCHE) siga abierta y los dos
// entren coordinados. Sin esto es el riesgo (f).1 de SPEC_2 7.ter, el de H13: el Esclavo entra solo y da verde por
// reloj contra el ambar del Maestro. Escrita ANTES del codigo y vista en rojo (con D-173 el Maestro iba a MANUAL).
// Incluido desde orquestador_deg_fin.inc; usa montarH, cortarH, correrMs, cuando, transversalH (bloque H).
//
// LO QUE SE EXIGE SALE DE LA SPEC: el acuse RRT_SIN_RADIO (firmas de SPEC_2 7.quater), el modo que no cambia
// (SPEC_4, FORZAR_ROJO), y de 7.ter (h) las dos entradas y ni verde contra verde ni verde contra ambar. El control
// con radio es el bloque S (orquestador_deg_rojo.inc), que sigue exigiendo MANUAL.
//
// LOS BORDES, escritos al lado (CLAUDE.md 7): la orden sale a los 100 s del corte, como H13 (pasados los
// SFTY6_SILENCIO_MS: el Maestro ya no tiene comunicacion); la ventana es la de H13, 1800 s desde el corte.

static long rrtSinRadio() {
  return (long)leerNumero(RAIZ + "/../05_Funcional/SPEC_2_Dos_Puntas_y_Radio.md",
                          R"(RRT_SIN_RADIO\s*=\s*(\d+))", "RRT_SIN_RADIO de la SPEC");
}

static void bloqueU() {
  std::printf("\n--- BLOQUE U: D-174 (ROJO TOTAL sin radio desde Automatico: el modo no cambia) -------\n");
  const unsigned long VENTANA = 1800000UL, A_LOS = 100000UL;   // los de H13
  CfgH c;
  ObsH o;
  bool ap = false;
  const std::string ctl = montarH(c, ap);
  cortarH(c, o);
  correrMs(o, A_LOS);
  const long acuse = MAESTRO.orden("forzar_rojo_app");
  long modoTras = -1;
  bool rojoTras = false;
  unsigned long rojoMs = 0;   // cuanto dura el rojo de este poste antes de volver a su ambar (dato, no se asevera)
  for (unsigned long t = 0; t < 60000UL; t += PASO_MS) {
    pasoH(o);
    if (t + PASO_MS == 2000UL) {
      modoTras = MAESTRO.orden("modo_actual");
      rojoTras = MAESTRO.rojo() && !MAESTRO.verde();
    }
    if (rojoMs == 0 && !MAESTRO.rojo()) rojoMs = t + PASO_MS;
  }
  correrMs(o, VENTANA - A_LOS - 60000UL);
  comprobar(ap && MAESTRO.orden("da_otro_apto") == 1 && ESCLAVO.orden("da_otro_apto") == 1,
            "U1.0 (control): antes del corte cada poste oyo el APTO del otro -" + ctl);
  comprobar(acuse == rrtSinRadio() && modoTras == MODO_AUTOMATICO_V && rojoTras,
            "U1.1 (D-174): FORZAR_ROJO desde AUTOMATICO a los 100 s del corte: acuse " + std::to_string(acuse) +
                " (RRT_SIN_RADIO " + std::to_string(rrtSinRadio()) + "), modo a los 2 s " + std::to_string(modoTras) +
                " (AUTOMATICO " + std::to_string(MODO_AUTOMATICO_V) + ", MANUAL " + std::to_string(MODO_MANUAL_V) +
                "), luz del Maestro en rojo " + (rojoTras ? "si" : "NO") + "; el rojo dura " +
                (rojoMs ? std::to_string(rojoMs) + " ms y vuelve su ambar" : std::string("los 60 s mirados")));
  const bool entran = o.tEnt[0] && o.tEnt[1];
  comprobar(entran && o.tVerde[0] && o.tVerde[1] && o.tEnt[1] < o.tVerde[0] && o.tEnt[0] < o.tVerde[1],
            "U1.2 (D-174, SPEC_2 7.ter (h)2): entran las dos en Degradado, cada una antes del primer verde de la "
            "otra (Maestro " + cuando(o, 0) + ", Esclavo " + cuando(o, 1) + ")");
  bool ok = false;
  const std::string t = transversalH(ok);
  comprobar(ok, "U1.3 (D-174, riesgo (f).1): en cada tick ni verde contra verde ni verde contra ambar (H13, con "
                "MANUAL, da verde del Esclavo contra el ambar del Maestro): " + t);
}
