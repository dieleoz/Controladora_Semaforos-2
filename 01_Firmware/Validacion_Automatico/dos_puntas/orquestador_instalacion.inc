// ===== dos_puntas/orquestador_instalacion.inc =====
//
// BLOQUE I - SPEC_8 8 (N-187, D-161..D-165) EN LAS DOS PUNTAS, CON CORTES DE LUZ.
// El Esclavo REAL: instalacion.cpp, botones.cpp, semaforo.cpp y bluetooth.cpp; la orden entra
// por el puerto del telefono. El Maestro: instalacion.cpp y semaforo.cpp REALES, la orden por
// instalacion_orden() (su bluetooth.cpp no se compila aqui) y sin camaras (las ejerce el
// bloque K de arnes_automatico.cpp). La pagina de flash es el modelo de
// ../instalacion_modelo.inc dentro de cada DLL: la recarga de la DLL la BORRA (= `-e all`),
// y un corte de luz la vuelca antes y la repone despues (la flash sobrevive, la RAM no).
// Las tramas y los bordes (3..5 s, fabrica NA/NA/3) salen de SPEC_8 8 y de la hoja de banco
// del 10/11 (pruebas 26..33), no del fuente. Lo incluye orquestador.cpp antes de main().

static bool g_vetoIEsperado = false;   // el bloque I pone presencia en una camara del Esclavo
// D-33 por NOMBRE: la pluma del Esclavo arriba en rojo porque SU camara la veta, solo dentro
// del bloque I y con semaforo_plumaVetada() REAL diciendolo. Fuera del bloque no hay camaras.
static bool excepcionVetoOrq(int i) {
  return i == 1 && g_vetoIEsperado && ESCLAVO.orden("pluma_vetada") == 1;
}

static const std::string I_PIN = "CMD:PIN:1234:";
static long g_iCmdDemanda = -1, g_iDemandas = 0, g_iPinC = -1, g_iPinD = -1;

// Un tick, contando los CMD_DEMANDA que el Esclavo emitio EN ESE tick (salen al aire con
// tEntrega = instante de emision + latencia).
static void iTick() {
  unTick();
  for (const EnVuelo& e : g_aire)
    if (e.destino == 0 && e.trama[1] == g_iCmdDemanda && e.tEntrega == g_t - PASO_MS + g_latenciaMs) g_iDemandas++;
}
static void iAvanzar(unsigned long ms) { for (unsigned long h = 0; h < ms; h += PASO_MS) iTick(); }

// Una linea del telefono al Esclavo: un acuse NUEVO, y es exactamente `espera` (el '*' es el
// separador del CRC: nada detras).
static bool iBt(const std::string& linea, const std::string& espera) {
  const long a0 = ESCLAVO.orden("acuses");
  ESCLAVO.orden(("bt:" + linea).c_str());
  iTick();
  return ESCLAVO.orden("acuses") == a0 + 1 && ESCLAVO.orden(("ack:" + espera + "*").c_str()) == 1;
}
static bool iMaestro(const std::string& accion, const std::string& espera) {
  MAESTRO.orden(("inst_orden:" + accion).c_str());
  return MAESTRO.orden(("inst_igual:" + espera).c_str()) == 1;
}
static std::string iConsulta(const char* c, const char* d, int r, int vc, int vd) {
  return std::string("$ACK,CMD:CONSULTA_INSTALACION,RESULT:C_") + c + "_D_" + d + "_R" +
         std::to_string(r) + ",VE:C" + std::to_string(vc) + "D" + std::to_string(vd);
}
static bool iPluma(Punta& p) { return p.pin(MOTOR_TALANQUERA) == TALANQUERA_ABRIR; }

// CORTE DE LUZ en una punta: el dominio de respaldo y (si conserva) la pagina de flash
// sobreviven; la RAM no. Sin conservar, la pagina vuelve borrada: es la carga con `-e all`.
// envenenar: antes del arranque, J15 en SUBIR y solo semaforo_setup() REAL (instalacion_mandos.inc).
// Las entradas de camara del Esclavo quedan como estaban cableadas (nivelC, nivelD).
static long iCorte(Punta& p, bool conserva, int nivelC = 0, int nivelD = 0, bool envenenar = false) {
  long pag[7], dom[PUNTA_DOMINIO_PALABRAS];
  for (int i = 0; i < 7; i++) pag[i] = p.orden("inst_hw", i);
  for (int i = 0; i < PUNTA_DOMINIO_PALABRAS; i++) dom[i] = p.domLeer(i);
  p.descargar();
  p.cargar();
  for (int i = 0; i < PUNTA_DOMINIO_PALABRAS; i++) p.domEscribir(i, dom[i]);
  if (conserva) for (int i = 0; i < 7; i++) p.orden("inst_poner", ((long)i << 16) | pag[i]);
  if (&p == &ESCLAVO) {
    p.orden("entrada_al_arrancar", (g_iPinC << 1) | nivelC);
    p.orden("entrada_al_arrancar", (g_iPinD << 1) | nivelD);
  }
  const long env = envenenar ? p.orden("arranque_envenenado") : 0;
  p.arrancar();
  if (&p == &MAESTRO) MAESTRO.orden("arrancar_automatico");
  return env;
}

// Del siguiente VERDE de la punta a su ROJO, y cuantos ms tarda la pluma en bajar desde el
// primer tick en S_ROJO. -2: no hubo verde ni rojo en 10 min; -3: no estaba arriba al entrar
// en rojo; -1: no bajo dentro del tope.
static long iBajada(Punta& p, unsigned long tope = 20000) {
  unsigned long h = 0;
  while (!p.verde()) { iTick(); if ((h += PASO_MS) > 600000UL) return -2; }
  while (p.estado() != S_ROJO_V) { iTick(); if ((h += PASO_MS) > 600000UL) return -2; }
  if (!iPluma(p)) return -3;
  for (unsigned long m = 0; m <= tope; m += PASO_MS) {
    if (!iPluma(p)) return (long)m;
    iTick();
  }
  return -1;
}
static bool iCerca(long medido, unsigned long esperado) {   // un tick de margen por cada lado
  return medido >= (long)(esperado - PASO_MS) && medido <= (long)(esperado + 2 * PASO_MS);
}

static void bloqueInstalacion() {
  std::printf("\n-- Bloque I: parametros de instalacion de N-187 en las dos puntas, con cortes --\n");
  escenarioLimpio(0);   // las dos DLL recien cargadas: paginas BORRADAS
  g_iCmdDemanda = ESCLAVO.orden("cmd_demanda");
  g_iPinC = ESCLAVO.orden("cam_pin", 0);
  g_iPinD = ESCLAVO.orden("cam_pin", 1);
  const unsigned long FAB = g_retardoPlumaMs;   // fabrica, releida de instalacion.h
  const std::string FAB_E = iConsulta("NA", "NA", 3, 0, 0);
  avanzar(5000);

  // -- I0: fabrica en las dos (prueba 26); CONSULTA sin PIN y con PIN; SET sin PIN, no -------
  const bool i0 = FAB == 3000UL && iBt("CMD:CONSULTA_INSTALACION", FAB_E) &&
      iBt("CMD:PIN:1234:CONSULTA_INSTALACION", FAB_E) &&
      iBt("CMD:SET_CAMARA:C:NC", "$ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO") &&
      iBt("CMD:SET_PLUMA_RETARDO:5", "$ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO") &&
      ESCLAVO.orden("inst_escrituras") == 0 && ESCLAVO.orden("inst_nc", 0) == 0 &&
      ESCLAVO.orden("inst_retardo_ms") == 3000 && iMaestro("CONSULTA_INSTALACION", FAB_E);
  comprobar(i0, "I0 (SPEC_8 8, prueba 26): pagina borrada = fabrica en las dos puntas (" + FAB_E +
            "); el Esclavo contesta CONSULTA_INSTALACION sin PIN y con PIN, y SET_CAMARA / "
            "SET_PLUMA_RETARDO sin PIN son AUTH_FAILED sin escribir la pagina");

  // -- I1: C en NC y D en NA, datos ASIMETRICOS: la presencia y la demanda van por INDICE ------
  // Con los dos pines BAJOS, C NC ve presencia y D NA no: un indice cruzado daria C0D1.
  const bool i1a = iBt(I_PIN + "SET_CAMARA:C:NC", "$ACK,CMD:SET_CAMARA:C:NC,RESULT:OK,VE:C1D0");
  ESCLAVO.entrada(g_iPinC, HIGH);   // el puente de la prueba 28: con NC, nadie delante
  iAvanzar(4000);
  const bool i1b = iBt("CMD:CONSULTA_INSTALACION", iConsulta("NC", "NA", 3, 0, 0));
  const long d0 = g_iDemandas;
  ESCLAVO.entrada(g_iPinC, LOW);    // C abre: con NC es un coche (flanco LOGICO)
  iAvanzar(200);
  const bool i1c = ESCLAVO.orden("presencia", 0) == 1 && ESCLAVO.orden("presencia", 1) == 0 &&
                   g_iDemandas == d0 + 1;
  iAvanzar(4000);
  const long d1 = g_iDemandas;
  ESCLAVO.entrada(g_iPinC, HIGH);   // C cierra: flanco ELECTRICO de subida, no es un coche
  iAvanzar(4000);
  const bool i1d = g_iDemandas == d1 && ESCLAVO.orden("presencia", 0) == 0;
  ESCLAVO.entrada(g_iPinD, HIGH);   // D NA: el pin alto SI es un coche
  iAvanzar(200);
  const bool i1e = ESCLAVO.orden("presencia", 1) == 1 && ESCLAVO.orden("presencia", 0) == 0 &&
                   g_iDemandas == d1 + 1;
  ESCLAVO.entrada(g_iPinD, LOW);
  iAvanzar(4000);
  comprobar(i1a && i1b && i1c && i1d && i1e,
            "I1 (D-162, prueba 29): en el Esclavo REAL, SET_CAMARA:C:NC con C y D bajos contesta VE:C1D0; "
            "con C cerrada CONSULTA dice C_NC_D_NA_R3,VE:C0D0; C abriendo es presencia de C (no de D) y "
            "pide UNA demanda; C cerrando no pide; D (NA) subiendo es presencia de D y pide UNA");

  // -- I2: R fuera de rango, rechazado sin escribir (P3: 3..5 s) ------------------------------
  const long e2 = ESCLAVO.orden("inst_escrituras");
  const bool i2 = iBt(I_PIN + "SET_PLUMA_RETARDO:2", "$ERR,CMD:SET_PLUMA_RETARDO:2,DESC:FUERA_DE_RANGO") &&
      iBt(I_PIN + "SET_PLUMA_RETARDO:6", "$ERR,CMD:SET_PLUMA_RETARDO:6,DESC:FUERA_DE_RANGO") &&
      iBt(I_PIN + "SET_PLUMA_RETARDO:0", "$ERR,CMD:SET_PLUMA_RETARDO:0,DESC:FUERA_DE_RANGO") &&
      iBt(I_PIN + "SET_PLUMA_RETARDO:3", "$ACK,CMD:SET_PLUMA_RETARDO:3,RESULT:SIN_CAMBIO,VE:C0D0") &&
      ESCLAVO.orden("inst_escrituras") == e2 && ESCLAVO.orden("inst_retardo_ms") == 3000;
  comprobar(i2, "I2 (P3, prueba 27): en el Esclavo REAL SET_PLUMA_RETARDO 2, 6 y 0 son FUERA_DE_RANGO, "
                "3 es SIN_CAMBIO, y la pagina no se escribe (retardo sigue en 3000 ms)");

  // -- I3: R=5 en el pin del Esclavo; el Maestro sigue en 3 (prueba 31: no va por radio) ------
  g_retardoPlumaIMs[1] = 5000UL;   // la invariante sigue al valor PEDIDO, no al getter
  const bool i3a = iBt(I_PIN + "SET_PLUMA_RETARDO:5", "$ACK,CMD:SET_PLUMA_RETARDO:5,RESULT:OK,VE:C0D0");
  const long b5 = iBajada(ESCLAVO);
  const long bM = iBajada(MAESTRO);
  const bool i3m = iMaestro("CONSULTA_INSTALACION", FAB_E) && MAESTRO.orden("inst_escrituras") == 0;
  char t3[360];
  std::snprintf(t3, sizeof(t3), "I3 (P1, pruebas 27 y 31): SET_PLUMA_RETARDO:5 en el Esclavo contesta OK y su "
                "pluma baja a los %ld ms del rojo (5000, -%lu/+%lu), no a los 3000; el Maestro sigue en "
                "fabrica y la suya baja a los %ld ms (3000)", b5, PASO_MS, 2 * PASO_MS, bM);
  comprobar(i3a && iCerca(b5, 5000UL) && iCerca(bM, FAB) && i3m, t3);

  // -- I4: PLUMA_VETADA, y la presencia de C (NC, abierta) es la que veta (prueba 30) ---------
  for (unsigned long h = 0; !ESCLAVO.verde() && h < 600000UL; h += PASO_MS) iTick();
  ESCLAVO.entrada(g_iPinC, LOW);    // un coche bajo la pluma, visto por C en NC
  g_vetoIEsperado = true;
  for (unsigned long h = 0; ESCLAVO.estado() != S_ROJO_V && h < 600000UL; h += PASO_MS) iTick();
  iAvanzar(5000UL + 2000UL);
  const bool vetada = ESCLAVO.orden("pluma_vetada") == 1 && iPluma(ESCLAVO) && ESCLAVO.rojo();
  const long e4 = ESCLAVO.orden("inst_escrituras");
  const bool i4a = iBt(I_PIN + "SET_CAMARA:C:NA", "$ERR,CMD:SET_CAMARA:C:NA,DESC:PLUMA_VETADA") &&
                   ESCLAVO.orden("inst_escrituras") == e4 && ESCLAVO.orden("inst_nc", 0) == 1;
  iAvanzar(1000);
  const bool sigue = iPluma(ESCLAVO) && ESCLAVO.orden("pluma_vetada") == 1;
  ESCLAVO.entrada(g_iPinC, HIGH);   // el coche sale: C cierra
  unsigned long baja = 0;
  for (; iPluma(ESCLAVO) && baja < 2000UL; baja += PASO_MS) iTick();
  g_vetoIEsperado = false;
  char t4[360];
  std::snprintf(t4, sizeof(t4), "I4 (SPEC_8 8.5, prueba 30): con C en NC abierta en rojo la pluma del Esclavo "
                "queda retenida pasados los 5 s (%d); SET_CAMARA:C:NA contesta PLUMA_VETADA, no escribe, C sigue "
                "NC y la pluma sigue arriba (%d); al cerrar C baja en %lu ms", vetada, sigue, baja);
  comprobar(vetada && i4a && sigue && !iPluma(ESCLAVO) && baja <= 2 * PASO_MS, t4);
  iAvanzar(4000);

  // -- I5 (a): lo puesto sobrevive a un corte de luz, en las dos (prueba 28) -----------------
  const std::string NC5 = iConsulta("NC", "NA", 5, 0, 0);
  const bool antes5 = iBt("CMD:CONSULTA_INSTALACION", NC5) && iMaestro("SET_PLUMA_RETARDO:5",
      "$ACK,CMD:SET_PLUMA_RETARDO:5,RESULT:OK,VE:C0D0") && iMaestro("SET_CAMARA:C:NC",
      "$ACK,CMD:SET_CAMARA:C:NC,RESULT:OK,VE:C0D0");
  g_retardoPlumaIMs[0] = 5000UL;
  iCorte(ESCLAVO, true, HIGH, LOW);   // el puente 9-10 sigue puesto durante el corte
  iAvanzar(3000);
  const bool trasE = iBt("CMD:CONSULTA_INSTALACION", NC5) && ESCLAVO.orden("inst_retardo_ms") == 5000 &&
                     ESCLAVO.orden("presencia", 0) == 0;
  const long b5e = iBajada(ESCLAVO);
  iCorte(MAESTRO, true);
  const bool trasM = iMaestro("CONSULTA_INSTALACION", NC5) && MAESTRO.orden("inst_retardo_ms") == 5000;
  const long b5m = iBajada(MAESTRO);
  char t5[420];
  std::snprintf(t5, sizeof(t5), "I5 (prueba 28): C NC, D NA, R5 antes del corte (%d); tras recargar cada DLL con "
                "la pagina y el respaldo conservados, CONSULTA dice C_NC_D_NA_R5,VE:C0D0 en el Esclavo (%d, "
                "puente puesto) y en el Maestro (%d), y la pluma baja a los %ld / %ld ms del rojo (5000)",
                antes5, trasE, trasM, b5e, b5m);
  comprobar(antes5 && trasE && trasM && iCerca(b5e, 5000UL) && iCerca(b5m, 5000UL), t5);

  // -- I6 (b): al encender, J15 en BAJAR antes que ninguna luz (SFTY-28, SPEC_8 tabla 8.4) ---
  // Con C en NC y SIN puente (presencia desde el primer instante) la pluma tampoco sube: una
  // camara no levanta una pluma (D-33), ni pide una demanda que nadie hizo (N-26: se siembra).
  const long dI6 = g_iDemandas;
  const long envE = iCorte(ESCLAVO, true, LOW, LOW, true);
  const long envM = iCorte(MAESTRO, true, LOW, LOW, true);
  bool abajo = !iPluma(ESCLAVO) && !iPluma(MAESTRO) && ESCLAVO.rojo();
  for (unsigned long h = 0; h < 3000UL; h += PASO_MS) {
    iTick();
    abajo = abajo && (ESCLAVO.verde() || !iPluma(ESCLAVO));
  }
  const bool veC = ESCLAVO.orden("presencia", 0) == 1 && g_iDemandas == dI6;
  ESCLAVO.entrada(g_iPinC, HIGH);
  char t6[360];
  std::snprintf(t6, sizeof(t6), "I6 (SFTY-28, prueba 6): con J15 envenenado en SUBIR, semaforo_setup() REAL lo "
                "deja en BAJAR con las seis luces apagadas (Esclavo %ld, Maestro %ld; 3 = las dos cosas); tras "
                "el arranque la pluma sigue abajo en rojo aunque C (NC, sin puente) vea presencia, sin demanda (%d)",
                envE, envM, veC);
  comprobar(envE == 3 && envM == 3 && abajo && veC, t6);
  iAvanzar(4000);

  // -- I7 (c): corte A MITAD de la escritura = FABRICA entera, nunca un retardo < 3 s ----------
  // Primero en la pagina: las 12 combinaciones validas, cortadas tras k = 0..6 medias palabras
  // (borrada y a medio programar), leen fabrica; k = 7 (completa) lee lo compuesto (control).
  bool i7a = true;
  int i7n = 0;
  for (int c = 0; c < 2; c++) for (int d = 0; d < 2; d++) for (int r = 3; r <= 5; r++) {
    ESCLAVO.orden("inst_componer", c | (d << 1) | (r << 2));
    long hw[7];
    for (int i = 0; i < 7; i++) hw[i] = ESCLAVO.orden("inst_hw", i);
    for (int k = 0; k <= 7; k++) {
      for (int i = 0; i < 7; i++) ESCLAVO.orden("inst_poner", ((long)i << 16) | (i < k ? hw[i] : 0xFFFF));
      const long ms = ESCLAVO.orden("inst_retardo_ms");
      const bool nc0 = ESCLAVO.orden("inst_nc", 0) == 1, nc1 = ESCLAVO.orden("inst_nc", 1) == 1;
      const bool ok = k == 7 ? (ms == r * 1000L && nc0 == (c == 1) && nc1 == (d == 1))
                             : (ms == 3000 && !nc0 && !nc1);
      i7a = i7a && ok && ms >= 3000;
      i7n++;
    }
  }
  // Despues, de verdad: desde C NC R5, SET_PLUMA_RETARDO:4 con la luz cortada tras k medias
  // palabras, y arranque. El puente en C sigue: declarada NA otra vez, ve presencia (prueba 33).
  bool i7b = true, i7m = true;
  for (int k = 0; k <= 6; k++) {
    ESCLAVO.orden("inst_componer", 1 | (0 << 1) | (5 << 2));
    ESCLAVO.orden("inst_corte_tras", k);
    ESCLAVO.orden(("bt:" + I_PIN + "SET_PLUMA_RETARDO:4").c_str());
    iTick();
    iCorte(ESCLAVO, true, HIGH, LOW);
    i7b = i7b && iBt("CMD:CONSULTA_INSTALACION", iConsulta("NA", "NA", 3, 1, 0)) &&
          ESCLAVO.orden("inst_retardo_ms") == 3000;
    MAESTRO.orden("inst_componer", 1 | (0 << 1) | (5 << 2));
    MAESTRO.orden("inst_corte_tras", k);
    MAESTRO.orden("inst_orden:SET_CAMARA:D:NC");
    iCorte(MAESTRO, true);
    i7m = i7m && iMaestro("CONSULTA_INSTALACION", FAB_E) && MAESTRO.orden("inst_retardo_ms") == 3000;
  }
  g_retardoPlumaIMs[0] = g_retardoPlumaIMs[1] = 0;
  iCorte(ESCLAVO, true, LOW, LOW);   // sin puente: con NA nadie delante
  const long b7 = iBajada(ESCLAVO);
  char t7[420];
  std::snprintf(t7, sizeof(t7), "I7 (SPEC_8 8.6): %d paginas (12 combinaciones x k=0..7) cortadas tras k "
                "medias palabras leen fabrica, y con k=7 (completa) lo compuesto (%d); un SET_PLUMA_RETARDO:4 / "
                "SET_CAMARA:D:NC cortados tras k=0..6 medias palabras arrancan en fabrica en el Esclavo "
                "(C_NA_D_NA_R3,VE:C1D0 con el puente: "
                "%d) y en el Maestro (%d); la pluma baja a los %ld ms (3000, nunca menos)", i7n, i7a, i7b, i7m, b7);
  comprobar(i7a && i7b && i7m && iCerca(b7, FAB), t7);

  // -- I8 (d): cargar firmware con `-e all` borra la pagina = FABRICA (prueba 33) -------------
  ESCLAVO.orden("inst_componer", 1 | (0 << 1) | (4 << 2));
  MAESTRO.orden("inst_componer", 1 | (0 << 1) | (4 << 2));
  const bool antes8 = iBt("CMD:CONSULTA_INSTALACION", iConsulta("NC", "NA", 4, 1, 0)) &&
                      iMaestro("CONSULTA_INSTALACION", iConsulta("NC", "NA", 4, 0, 0));
  iCorte(ESCLAVO, false, HIGH, LOW);   // DLL recargada SIN la pagina; el puente sigue
  iCorte(MAESTRO, false);
  bool borrada = true;
  for (int i = 0; i < 7; i++)
    borrada = borrada && ESCLAVO.orden("inst_hw", i) == 0xFFFF && MAESTRO.orden("inst_hw", i) == 0xFFFF;
  const bool i8 = antes8 && borrada && iBt("CMD:CONSULTA_INSTALACION", iConsulta("NA", "NA", 3, 1, 0)) &&
                  iMaestro("CONSULTA_INSTALACION", FAB_E) && ESCLAVO.orden("inst_retardo_ms") == 3000;
  comprobar(i8, "I8 (prueba 33): C NC y R4 antes de cargar; tras la carga (pagina borrada) las dos puntas "
                "estan en fabrica, y el Esclavo con el puente de C ve presencia (C_NA_D_NA_R3,VE:C1D0): hay "
                "que volver a poner el contacto tras cada carga");

  // Deja el banco como lo encuentra el siguiente: paginas borradas, sin puentes, sin cotas.
  iCorte(ESCLAVO, false, LOW, LOW);
  iCorte(MAESTRO, false);
  avanzar(5000);
}
