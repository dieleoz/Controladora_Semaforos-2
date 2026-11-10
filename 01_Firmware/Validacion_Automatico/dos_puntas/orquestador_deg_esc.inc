// ===== Validacion_Automatico/dos_puntas/orquestador_deg_esc.inc =====
// BLOQUE S - D-171 (SPEC_4 6, responsable 10/11): "sin dato fresco del Poste 2, ESC:?". Lo que aqui se exige es que
// la GRACIA de SFTY-6 no fabrica ese dato: coordinador_forzarRojoTotal() (entrada en Manual) y
// coordinador_iniciarModo() (entrada en Automatico) renuevan el reloj de la respuesta SIN que haya llegado ninguna,
// y el Maestro publicaba ESC:ROJO hasta SFTY6_SILENCIO_MS con el Poste 2 en su ambar. Escritas ANTES del codigo y
// vistas en rojo. Del fuente solo SFTY6_SILENCIO_MS (el borde) y la posicion de los modos en el enum.
// Los controles (S3..S5) miden la otra cara: con radio sana, un ACK_RED basta para ESC:ROJO, y un ciclo de
// Automatico sano no publica ningun '?' despues de la entrada.

struct ObsS {
  unsigned long ms = 0, rojoConEAbierto = 0, sinInterrogacion = 0, interrogacion = 0, rojo = 0, verde = 0;
};

// Mira ESC: en cada tick. rojoConEAbierto: ESC:ROJO con el Esclavo SIN rojo en sus pines.
static ObsS observarS(unsigned long ms) {
  ObsS o;
  for (unsigned long h = 0; h < ms; h += PASO_MS) {
    unTick();
    o.ms += PASO_MS;
    const bool q = MAESTRO.orden("esc_desconocido") == 1, r = MAESTRO.orden("esc_es:ROJO") == 1;
    if (q) o.interrogacion += PASO_MS; else o.sinInterrogacion += PASO_MS;
    if (r) o.rojo += PASO_MS;
    if (MAESTRO.orden("esc_es:VERDE") == 1) o.verde += PASO_MS;
    if (r && !ESCLAVO.rojo()) o.rojoConEAbierto += PASO_MS;
  }
  return o;
}

static std::string cifrasS(const ObsS& o) {
  return std::to_string(o.ms) + " ms: ESC:? " + std::to_string(o.interrogacion) + ", ESC:ROJO " +
         std::to_string(o.rojo) + ", ESC:VERDE " + std::to_string(o.verde) + ", ESC:ROJO con el Esclavo sin rojo " +
         std::to_string(o.rojoConEAbierto);
}

static void bloqueS() {
  std::printf("\n--- BLOQUE S: D-171, la gracia de SFTY-6 no es un dato del Poste 2 -----------\n");
  const std::string MODOS = RAIZ + "/Maestro/include/modos.h";
  const long MANUAL_V = posicionEnEnum(MODOS, "ModoSistema", "MODO_MANUAL");
  const long AUTO_V = posicionEnEnum(MODOS, "ModoSistema", "MODO_AUTOMATICO");
  const unsigned long SILENCIO_MS = leerNumero(RAIZ + "/Maestro/include/protocolo.h",
                                               R"(#define\s+SFTY6_SILENCIO_MS\s+(\d+)UL)", "SFTY6_SILENCIO_MS");
  // S1/S2: MENU sano, la radio cae; pasado el silencio el Esclavo esta en su ambar de orfandad. El operario entra en
  // Manual (S1) o en Automatico (S2) sin radio: hasta el borde de SFTY-6 no hay ACK_RED, asi que ESC: es '?'.
  // Despues vuelve la radio: el ACK_RED de la orden en curso devuelve ESC:ROJO (S1b/S2b; '?' no se queda pegado).
  for (int i = 0; i < 2; i++) {
    const long modo = i == 0 ? MANUAL_V : AUTO_V;
    const std::string n = i == 0 ? "S1 (Manual, coordinador_forzarRojoTotal)"
                                 : "S2 (Automatico, coordinador_iniciarModo)";
    prepararSincronizadas(15, 8, 0, 0);
    MAESTRO.orden("forzar_menu");
    avanzar(10000);
    const long frescoAntes = MAESTRO.orden("esc_es:ROJO");
    g_enlace = false;
    avanzar(SILENCIO_MS + 10000UL);
    const bool eAmbar = !ESCLAVO.rojo() && ambarH(ESCLAVO);
    MAESTRO.orden("set_modo", modo);
    const ObsS o = observarS(SILENCIO_MS - 1000UL);
    comprobar(frescoAntes == 1 && eAmbar && o.rojo == 0 && o.sinInterrogacion == 0,
              n + " (D-171): sin radio y con el Esclavo en ambar (" + std::to_string(eAmbar) +
                  "), la gracia de SFTY-6 no publica ESC:ROJO; se exige ESC:? en todo el tramo (" + cifrasS(o) + ")");
    g_enlace = true;
    unsigned long hastaRojo = 0;
    while (MAESTRO.orden("esc_es:ROJO") != 1 && hastaRojo < 2UL * SILENCIO_MS) { unTick(); hastaRojo += PASO_MS; }
    comprobar(hastaRojo < 2UL * SILENCIO_MS && ESCLAVO.rojo(),
              n + "b (D-171): vuelta la radio, ESC:ROJO a los " + std::to_string(hastaRojo) +
                  " ms con el Esclavo en rojo (" + std::to_string(ESCLAVO.rojo()) + "); el '?' no se queda pegado");
  }
  // S3 (control): MENU sano con ACK_RED. Cuatro silencios con radio: ESC:ROJO siempre, ningun '?'.
  {
    prepararSincronizadas(15, 8, 0, 0);
    MAESTRO.orden("forzar_menu");
    avanzar(10000);
    const ObsS o = observarS(4UL * SILENCIO_MS);
    comprobar(o.interrogacion == 0 && o.rojo == o.ms && o.rojoConEAbierto == 0,
              "S3 (control D-171): MENU sano con ACK_RED no pasa a '?' (" + cifrasS(o) + ")");
  }
  // S4 (control): Manual con radio. El ACK_RED llega en un viaje: ESC:ROJO desde ahi y ningun '?' despues.
  {
    prepararSincronizadas(15, 8, 0, 0);
    MAESTRO.orden("forzar_menu");
    avanzar(10000);
    MAESTRO.orden("set_modo", MANUAL_V);
    avanzar(5000);
    const ObsS o = observarS(4UL * SILENCIO_MS);
    comprobar(o.interrogacion == 0 && o.rojo == o.ms && o.rojoConEAbierto == 0,
              "S4 (control D-171): Manual con radio, pasado el primer acuse, ESC:ROJO sin '?' (" + cifrasS(o) + ")");
  }
  // S5 (control): Automatico sano. Pasada la entrada, quince minutos de ciclo sin ningun '?', con VERDE y ROJO vistos.
  {
    prepararSincronizadas(15, 8, 0, 0);
    MAESTRO.orden("forzar_menu");
    avanzar(10000);
    MAESTRO.orden("set_modo", AUTO_V);
    avanzar(5000);
    const ObsS o = observarS(900000UL);
    // Sin exigir rojoConEAbierto: el ACK_GREEN acusa el verde ya encendido y llega un viaje despues (D-53).
    comprobar(o.interrogacion == 0 && o.verde > 0 && o.rojo > 0,
              "S5 (control D-171): Automatico con radio sana, ciclo sin ningun '?' tras la entrada (" +
                  cifrasS(o) + ")");
  }
}
