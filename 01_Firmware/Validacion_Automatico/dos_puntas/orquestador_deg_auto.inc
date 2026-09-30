// ===== Validacion_Automatico/dos_puntas/orquestador_deg_auto.inc =====
//
// BLOQUE H - EL DEGRADADO AUTOMATICO (A-15, SPEC_2 7.ter (h)), SOBRE EL C++ REAL DE LAS DOS
// PUNTAS: Maestro/src/deg_auto.cpp y Esclavo/src/deg_auto.cpp de a0d605b, con coordinador.cpp,
// modo_automatico.cpp, modo_manual.cpp, modo_degradado.cpp y el main.cpp del Esclavo reales.
//
// Incluido desde orquestador_degradado.cpp, justo antes de main(): usa sus primitivas -unTick,
// prepararSincronizadas, el canal g_aire, comprobar()-. Vive aparte porque aquel pasa de 500
// lineas y no puede crecer.
//
// LA ASERCION TRANSVERSAL es vigilarH(), que vigilar() llama en CADA tick del banco (tambien los
// del montaje): ni verde contra verde, ni verde contra ambar. Cada escenario pone a cero sus
// contadores al empezar y los asevera al acabar.
//
// LOS BORDES SALEN DE LA SPEC, NO DEL FUENTE (CLAUDE.md 15): 420 s de rojo minimo y 300 s de
// espera se leen de la tabla de SPEC_2 7.ter. Releerlos del .h haria que un defecto en la
// constante -la inyeccion de 420 a 60- moviera tambien el borde y el arnes siguiera verde. Lo
// que se relee del fuente se COMPARA contra la spec (H0).
//
// LO QUE NO CUBRE, dicho para que nadie lo cuente como cubierto:
//   - SET_DEG_AUTO y su $ACK diferido: bluetooth.cpp no se compila aqui; la opcion se pone con
//     respaldo_guardarDegAuto() (orden "da_opcion" de los adaptadores).
//   - el reinicio del Esclavo YA DENTRO del Degradado (la pagina del testigo es un doble en RAM
//     que el FreeLibrary borra) y el reinicio del Maestro.
//   - la entrada a las 23:55 y el control negativo "APTO del Esclavo forzado a 1" de (h).
//   - y nada de esto sustituye el banco de SPEC_2 7.ter (h), con dos tarjetas.

static unsigned long g_hVV = 0, g_hVA = 0, g_hPrimera = 0;
static std::string g_hQue;

// El ambar por PIN o por ESTADO del semaforo: el intermitente apaga su pin la mitad del tiempo, y
// esa mitad tambien es "el otro poste esta en ambar" para quien viene con verde (arquitecto, E).
static bool ambarH(Punta& p) { return p.ambar() || p.orden("da_en_fallo") == 1; }

static void vigilarH(bool vM, bool vE) {
  const bool vv = vM && vE;
  const bool va = (vM && ambarH(ESCLAVO)) || (vE && ambarH(MAESTRO));
  if (vv) g_hVV++;
  if (va) g_hVA++;
  if ((vv || va) && g_hPrimera == 0) {
    g_hPrimera = g_t;
    g_hQue = vv ? "verde contra verde" : (vM ? "verde del Maestro contra ambar del Esclavo"
                                             : "verde del Esclavo contra ambar del Maestro");
  }
}

static void ceroH() {
  g_hVV = g_hVA = g_hPrimera = 0;
  g_hQue.clear();
}

// El veredicto transversal de un escenario, en una linea.
static std::string transversalH(bool& ok) {
  ok = (g_hVV == 0 && g_hVA == 0 && g_pegados == 0);
  std::string s = "verde/verde " + std::to_string(g_hVV * PASO_MS) + " ms, verde/ambar " +
                  std::to_string(g_hVA * PASO_MS) + " ms, verde tras verde sin todo-rojo " +
                  std::to_string(g_pegados);
  if (g_hPrimera) s += " (primera: " + g_hQue + " a los " + std::to_string(g_hPrimera) + " ms del banco)";
  return s;
}

struct ObsH {
  unsigned long tCorte = 0;
  unsigned long tReinicio = 0;       // H5: el banco del ENCENDIDO del Esclavo (su millis() = 0)
  unsigned long tEnt[2] = {0, 0};     // 0 = no entro
  unsigned long tVerde[2] = {0, 0};   // el primer verde tras entrar
  unsigned long noRojo[2] = {0, 0};   // ticks sin todo-rojo entre entrar y ese verde
  unsigned long verdes[2] = {0, 0};   // ticks con verde tras entrar
  long sEnt[2] = {-1, -1}, sVerde[2] = {-1, -1};   // segundo del dia de cada punta en esos dos
};

static Punta& puntaH(int i) { return i == 0 ? MAESTRO : ESCLAVO; }

static void pasoH(ObsH& o) {
  unTick();
  for (int i = 0; i < 2; i++) {
    Punta& p = puntaH(i);
    if (!o.tEnt[i] && p.orden("da_en_degradado")) { o.tEnt[i] = g_t; o.sEnt[i] = p.orden("segundos_del_dia"); }
    if (!o.tEnt[i]) continue;
    if (p.verde()) o.verdes[i]++;
    if (o.tVerde[i]) continue;
    if (p.verde()) { o.tVerde[i] = g_t; o.sVerde[i] = p.orden("segundos_del_dia"); }
    else if (!p.rojo()) o.noRojo[i]++;
  }
}

static int MODO_AUTOMATICO_V = -1, MODO_MANUAL_V = -1;
static unsigned long H_ROJO_MIN_MS = 0, H_ESPERA_MS = 0;
static int H_CMD_PRESENTE = -1;

struct CfgH {
  bool opM = true, opE = true;
  bool cortaME = true, cortaEM = true;
  int modoM = -1;                 // -1 = AUTOMATICO
  long reinicioEsclavoMs = -1;    // tras el corte; -1 = sin reinicio
  bool esp32E = true;             // false: el J17 del Esclavo mudo desde el montaje (H14c)
};

// Monta: sync real por radio, opcion en cada poste, Maestro en AUTOMATICO -y en MANUAL si se
// pide- con la radio viva el tiempo de oirse el APTO. Devuelve la linea de control.
static std::string montarH(const CfgH& c, bool& apOk) {
  ceroH();
  g_cortaME = g_cortaEM = false;
  // 08:05:00, NO 08:00:00, Y ES EL BORDE DE H1.2. La luz sale de segDia % 420 (ciclo_degradado.h)
  // y el inicio es multiplo de 300: segun donde caiga la entrada, la fase se come el rojo minimo
  // y un 420 -> 60 en DEG_AUTO_ROJO_MIN_S NO se ve. MEDIDO: montando a las 08:00 el primer verde
  // del Maestro salia a los 542,8 s de su entrada con 420 y con 60. A las 08:05 la entrada cae a
  // las 08:12:57: con 60 el inicio es 08:15:00 y el verde, a los ~243 s; con 420, a los ~663 s.
  prepararSincronizadas(15, 8, 5, 0);
  g_esp32Vivo[1] = c.esp32E;
  MAESTRO.orden("da_opcion", c.opM ? 1 : 0);
  ESCLAVO.orden("da_opcion", c.opE ? 1 : 0);
  MAESTRO.orden("set_modo", MODO_AUTOMATICO_V);
  avanzar(60000);
  if (c.modoM >= 0 && c.modoM != MODO_AUTOMATICO_V) {
    MAESTRO.orden("set_modo", c.modoM);
    avanzar(30000);
  }
  const long aM = MAESTRO.orden("da_apto"), aE = ESCLAVO.orden("da_apto");
  const long oM = MAESTRO.orden("da_otro_apto"), oE = ESCLAVO.orden("da_otro_apto");
  apOk = (oM == aE && oE == aM);
  return "APTO propio M " + std::to_string(aM) + " / E " + std::to_string(aE) +
         ", ultimo APTO oido del otro M " + std::to_string(oM) + " / E " + std::to_string(oE);
}

static void cortarH(const CfgH& c, ObsH& o) {
  // Simetrico: se apaga el enlace entero y los sentidos quedan libres, para que devolver la
  // radio (H8) sea una sola perilla. De un solo sentido: el enlace sigue y se corta ese.
  const bool simetrico = c.cortaME && c.cortaEM;
  g_enlace = !simetrico;
  g_cortaME = simetrico ? false : c.cortaME;
  g_cortaEM = simetrico ? false : c.cortaEM;
  g_aire.clear();
  reiniciarObservacion();
  o.tCorte = g_t;
}

// Un corte de energia del Esclavo: la RAM se va (FreeLibrary), la pila se queda. Su ESP32 le
// devuelve la hora a los 5 s con la hora del Maestro -el mismo origen, residuo < 1 s-.
static void reiniciarEsclavoH(ObsH& o) {
  long dom[PUNTA_DOMINIO_PALABRAS];
  for (int i = 0; i < PUNTA_DOMINIO_PALABRAS; i++) dom[i] = ESCLAVO.domLeer(i);
  ESCLAVO.descargar();
  ESCLAVO.cargar();
  for (int i = 0; i < PUNTA_DOMINIO_PALABRAS; i++) ESCLAVO.domEscribir(i, dom[i]);
  ESCLAVO.arrancar();
  // su millis() sigue desde el que dejo setup() (el delay(2000) de N-22), no desde el del banco
  o.tReinicio = g_t - (unsigned long)ESCLAVO.orden("da_arranque_en", (long)g_t);
  for (unsigned long t = 0; t < 5000; t += PASO_MS) pasoH(o);
  ESCLAVO.orden("siembra_esp32", 15L * 86400L + MAESTRO.orden("segundos_del_dia"));
}

// Corre hasta msMax tras el corte, o hasta 60 s despues de que las dos hayan dado su verde.
static ObsH correrH(const CfgH& c, unsigned long msMax, std::string& control, bool& apOk) {
  ObsH o;
  control = montarH(c, apOk);
  cortarH(c, o);
  bool reiniciado = false;
  unsigned long fin = 0;
  while (g_t - o.tCorte < msMax && (fin == 0 || g_t < fin)) {
    if (c.reinicioEsclavoMs >= 0 && !reiniciado && g_t - o.tCorte >= (unsigned long)c.reinicioEsclavoMs) {
      reiniciado = true;
      reiniciarEsclavoH(o);
    }
    pasoH(o);
    if (fin == 0 && o.tVerde[0] && o.tVerde[1]) fin = g_t + 60000;
  }
  return o;
}

static std::string seg(unsigned long ms) {
  return std::to_string(ms / 1000) + "," + std::to_string(ms % 1000 / 100) + " s";
}

static std::string hms(long s) {
  if (s < 0) return "--";
  char b[16];
  std::snprintf(b, sizeof(b), "%02ld:%02ld:%02ld", s / 3600, s / 60 % 60, s % 60);
  return b;
}

static std::string cuando(const ObsH& o, int i) {
  return o.tEnt[i] ? "a los " + seg(o.tEnt[i] - o.tCorte) + " del corte" : std::string("NUNCA");
}

// Las filas de un escenario en el que las dos TIENEN que entrar.
static void asevera_entran(const char* id, const char* titulo, const ObsH& o, const std::string& control,
                           bool apOk) {
  const std::string pre = std::string(id);
  comprobar(apOk && MAESTRO.orden("da_otro_apto") == 1 && ESCLAVO.orden("da_otro_apto") == 1,
            pre + ".0 (control): antes del corte cada poste oyo el APTO del otro -" + control);
  const bool entran = o.tEnt[0] && o.tEnt[1];
  comprobar(entran && MAESTRO.orden("da_entradas") == 1 && ESCLAVO.orden("da_entradas") == 1,
            pre + ".1 " + titulo + ": entran las dos solas (Maestro " + cuando(o, 0) + ", Esclavo " +
                cuando(o, 1) + "; AUTO_ENTRADA M " +
                std::to_string(MAESTRO.orden("da_entradas")) + " / E " +
                std::to_string(ESCLAVO.orden("da_entradas")) + ", rechazos M " +
                std::to_string(MAESTRO.orden("da_rechazos")) + " / E " +
                std::to_string(ESCLAVO.orden("da_rechazos")) + ")");
  // El motivo de un rechazo de la puerta del testigo, por su nombre del enum (lo que publica).
  static const char* MOTIVOS[] = {"FALTA_HORA", "SIN_HORA", "AHORA_DESFASADO", "INICIO_VENCIDO",
                                  "DESPEJE_RANGO", "AMBAR_VIGENTE", "EN_VERDE", "NO_GUARDADO"};
  for (int i = 0; i < 2; i++) {
    if (puntaH(i).orden("da_rechazos") == 0) continue;
    std::string m = "?";
    for (const char* x : MOTIVOS)
      if (puntaH(i).orden((std::string("da_rechazo_es:") + x).c_str())) m = x;
    nota(pre + ": el " + (i ? "Esclavo" : "Maestro") + " publico AUTO_RECHAZADA_..." + m);
  }
  for (int i = 0; i < 2; i++)
    nota(pre + ": " + (i ? "Esclavo" : "Maestro") + " entra a " + hms(o.sEnt[i]) + ", publica inicio " +
         hms(puntaH(i).orden("da_inicio_s")) + ", primer verde a " + hms(o.sVerde[i]));
  const long rM = (o.tEnt[0] && o.tVerde[0]) ? (long)(o.tVerde[0] - o.tEnt[0]) : -1;
  const long rE = (o.tEnt[1] && o.tVerde[1]) ? (long)(o.tVerde[1] - o.tEnt[1]) : -1;
  comprobar(rM >= (long)H_ROJO_MIN_MS && rE >= (long)H_ROJO_MIN_MS,
            pre + ".2 " + titulo + ": el primer verde de cada una llega tras >= " + seg(H_ROJO_MIN_MS) +
                " de rojo desde su entrada (SPEC_2 7.ter (b)): Maestro " +
                (rM < 0 ? std::string("sin verde") : seg((unsigned long)rM)) + ", Esclavo " +
                (rE < 0 ? std::string("sin verde") : seg((unsigned long)rE)) + "; ms sin todo-rojo en ese tramo M " +
                std::to_string(o.noRojo[0] * PASO_MS) + " / E " + std::to_string(o.noRojo[1] * PASO_MS));
  comprobar(entran && o.tVerde[0] && o.tVerde[1] && o.tEnt[1] < o.tVerde[0] && o.tEnt[0] < o.tVerde[1],
            pre + ".3 " + titulo + ": cada una esta en Degradado antes del primer verde de la otra "
                "(SPEC_2 7.ter (h)2)");
  bool ok = false;
  const std::string t = transversalH(ok);
  comprobar(ok, pre + ".4 " + titulo + ": en cada tick, ni verde contra verde ni verde contra ambar: " + t);
}

static void asevera_no_entran(const char* id, const char* titulo, const ObsH& o, const std::string& control) {
  bool ok = false;
  const std::string t = transversalH(ok);
  comprobar(!o.tEnt[0] && !o.tEnt[1] && MAESTRO.orden("da_entradas") == 0 &&
                ESCLAVO.orden("da_entradas") == 0,
            std::string(id) + " " + titulo + ": no entra ninguna en " + seg(g_t - o.tCorte) +
                " sin radio (Maestro " + (o.tEnt[0] ? "ENTRO" : "fuera") + ", Esclavo " +
                (o.tEnt[1] ? "ENTRO" : "fuera") + "; " + control + ")");
  comprobar(ok, std::string(id) + "t " + titulo + ": en cada tick, ni verde contra verde ni verde contra ambar: " + t);
}

#include "orquestador_deg_auto2.inc"   // H10..H14 (29/09): este fichero no pasa de 500 lineas

static void bloqueH() {
  std::printf("\n--- BLOQUE H: A-15, EL DEGRADADO AUTOMATICO (SPEC_2 7.ter) --------\n");
  const std::string SPEC2 = RAIZ + "/../05_Funcional/SPEC_2_Dos_Puntas_y_Radio.md";
  H_ROJO_MIN_MS = 1000UL * leerNumero(SPEC2, R"(\|\s*`DEG_AUTO_ROJO_MIN_S`\s*\|\s*(\d+))",
                                      "DEG_AUTO_ROJO_MIN_S de la SPEC");
  H_ESPERA_MS = leerNumero(SPEC2, R"(\|\s*`DEG_AUTO_ESPERA_MS`\s*\|\s*(\d+))", "DEG_AUTO_ESPERA_MS de la SPEC");
  H_CMD_PRESENTE = (int)std::strtoul(std::regex_replace(leerFuente(RAIZ + "/Maestro/include/protocolo.h"),
      std::regex(R"([\s\S]*#define\s+CMD_PRESENTE\s+(0x[0-9A-Fa-f]+)[\s\S]*)"), "$1").c_str(), nullptr, 16);
  const std::string MODOS = RAIZ + "/Maestro/include/modos.h";
  MODO_AUTOMATICO_V = (int)posicionEnEnum(MODOS, "ModoSistema", "MODO_AUTOMATICO");
  MODO_MANUAL_V = (int)posicionEnEnum(MODOS, "ModoSistema", "MODO_MANUAL");
  unsigned long fuente[2];
  const char* hdr[2] = {"/Maestro/include/deg_auto.h", "/Esclavo/include/deg_auto.h"};
  for (int i = 0; i < 2; i++)
    fuente[i] = leerNumero(RAIZ + hdr[i], R"(DEG_AUTO_ROJO_MIN_S\s*=\s*(\d+))", "DEG_AUTO_ROJO_MIN_S");
  if (H_CMD_PRESENTE <= 0 || H_CMD_PRESENTE > 255) abortar("no se pudo leer CMD_PRESENTE de protocolo.h");
  comprobar(fuente[0] * 1000UL == H_ROJO_MIN_MS && fuente[1] * 1000UL == H_ROJO_MIN_MS,
            "H0: DEG_AUTO_ROJO_MIN_S del fuente (Maestro " + std::to_string(fuente[0]) + ", Esclavo " +
                std::to_string(fuente[1]) + ") es el de la SPEC (" + seg(H_ROJO_MIN_MS) + ")");

  const unsigned long VENTANA = 1800000UL;               // corte + espera + rojo + dos verdes
  const unsigned long SIN_ENTRADA = H_ESPERA_MS + 900000UL;
  std::string ctl;
  bool ap = false;

  // H1 + H8 + H9: corte simetrico; luego vuelve la radio; luego la fecha salta 31 dias.
  {
    CfgH c;
    ObsH o = correrH(c, VENTANA, ctl, ap);
    asevera_entran("H1", "corte simetrico, opcion en los dos", o, ctl, ap);

    ceroH();
    reiniciarObservacion();
    g_enlace = true;
    const long enM0 = MAESTRO.orden("da_enlace"), enE0 = ESCLAVO.orden("da_enlace");
    for (unsigned long t = 0; t < 60000; t += PASO_MS) pasoH(o);
    const long enM = MAESTRO.orden("da_enlace") - enM0, enE = ESCLAVO.orden("da_enlace") - enE0;
    const bool siguen = MAESTRO.orden("da_en_degradado") && ESCLAVO.orden("da_en_degradado");
    bool ok = false;
    const std::string t = transversalH(ok);
    comprobar(enM == 1 && enE == 1 && siguen && ok,
              "H8 reconexion: vuelve la radio y cada poste publica ENLACE_DISPONIBLE una vez (M " +
                  std::to_string(enM) + ", E " + std::to_string(enE) + ") y los dos SIGUEN en Degradado (" +
                  (siguen ? "si" : "NO") + "); " + t);

    // H9 (responsable, 29/09, SPEC_2 7.ter (d)): el testigo NO vence. La fecha salta 31 dias en
    // los dos postes -la siembra de su ESP32 con la hora del dia intacta- y tienen que seguir
    // alternando. Vista en rojo sobre 38ad119 (LIMITE_31D M 1 / E 1).
    ceroH();
    reiniciarObservacion();
    MAESTRO.orden("da_dias_extra", 31);
    ESCLAVO.orden("da_dias_extra", 31);
    const long sM = MAESTRO.orden("siembra_esp32_eco"), sE = ESCLAVO.orden("siembra_esp32_eco");
    ObsH o9;
    o9.tEnt[0] = o9.tEnt[1] = g_t;
    for (unsigned long t9 = 0; t9 < 900000UL; t9 += PASO_MS) pasoH(o9);
    const long l31M = MAESTRO.orden("da_limite31"), l31E = ESCLAVO.orden("da_limite31");
    const bool sigueM = MAESTRO.orden("da_en_degradado") && o9.verdes[0] > 0;
    const bool sigueE = ESCLAVO.orden("da_en_degradado") && o9.verdes[1] > 0;
    comprobar(sM == 1 && sE == 1 && sigueM && sigueE && l31M == 0 && l31E == 0,
              "H9 el testigo NO vence a los 31 dias (SPEC_2 7.ter (d)): con la fecha adelantada 31 dias "
              "(siembra M " + std::to_string(sM) + " / E " + std::to_string(sE) + ") los dos siguen alternando "
              "15 min despues (verde M " + seg(o9.verdes[0] * PASO_MS) + " / E " + seg(o9.verdes[1] * PASO_MS) +
                  "; LIMITE_31D M " + std::to_string(l31M) + " / E " + std::to_string(l31E) + ")");
    const std::string t9s = transversalH(ok);
    comprobar(ok, "H9t tras el salto de 31 dias, en cada tick ni verde contra verde ni verde contra ambar: " + t9s);
  }

  // H2: la opcion en un solo poste. Ninguno entra: con la opcion EFECTIVA uno solo daria verde
  // por reloj contra el ambar del otro (SPEC_2 7.ter (a)).
  {
    CfgH c;
    c.opE = false;
    ObsH o = correrH(c, SIN_ENTRADA, ctl, ap);
    asevera_no_entran("H2a", "opcion solo en el Maestro", o, ctl);
    c.opE = true;
    c.opM = false;
    o = correrH(c, SIN_ENTRADA, ctl, ap);
    asevera_no_entran("H2b", "opcion solo en el Esclavo", o, ctl);
  }

  // H3/H4: el corte de un solo sentido (una antena o un TX muertos). SPEC_2 7.ter (h)2.
  // MEDIDO 29/09 sobre a0d605b: H4 FALLABA. Con M->E viva la radio "mandaba" y el ESP32 del Esclavo
  // se ignoraba: sin fecha, su puerta rechazaba SIN_HORA con su APTO en 1 y el Maestro daba 180 s
  // de verde contra su ambar. Arreglo: la radio no manda sobre una base sin fecha (reloj.cpp) y el
  // APTO es la puerta misma (deg_auto.cpp de las dos puntas).
  {
    CfgH c;
    c.cortaEM = false;
    ObsH o = correrH(c, VENTANA, ctl, ap);
    asevera_entran("H3", "corte solo M->E", o, ctl, ap);
    c.cortaEM = true;
    c.cortaME = false;
    o = correrH(c, VENTANA, ctl, ap);
    asevera_entran("H4", "corte solo E->M", o, ctl, ap);
  }

  // H5: el Esclavo se reinicia a mitad de su cuenta: vuelve a contar desde el arranque (+300 s,
  // SPEC_2 7.ter (b)(2)) y aun asi entra antes del primer verde del Maestro.
  {
    CfgH c;
    c.reinicioEsclavoMs = 150000;
    ObsH o = correrH(c, VENTANA, ctl, ap);
    asevera_entran("H5", "reinicio del Esclavo a los 150 s del corte", o, ctl, ap);
    comprobar(o.tReinicio && o.tEnt[1] > o.tReinicio && o.tEnt[1] - o.tReinicio >= H_ESPERA_MS,
              "H5.5 tras reiniciar, el Esclavo vuelve a contar desde el arranque (SPEC_2 7.ter (b)(2)): entra a los " +
                  (o.tEnt[1] > o.tReinicio ? seg(o.tEnt[1] - o.tReinicio) : std::string("--")) +
                  " del encendido (espera " + seg(H_ESPERA_MS) + ")");
  }

  // H6: el Maestro en MANUAL cuando empieza el corte. Su APTO es 0 (hay una persona) y el Esclavo
  // lo oye antes del corte: no entra ninguno.
  {
    CfgH c;
    c.modoM = MODO_MANUAL_V;
    ObsH o = correrH(c, SIN_ENTRADA, ctl, ap);
    asevera_no_entran("H6", "Maestro en MANUAL al empezar el corte", o, ctl);
  }

  // H7: CMD_PRESENTE oido por un poste NO degradado: $ALARM OTRO_POSTE_EN_DEGRADADO, y nada mas
  // -no es respuesta, no reinicia la cuenta, no mete en el modo- (SPEC_2 7.ter (c)).
  {
    CfgH c;
    ObsH o;
    ctl = montarH(c, ap);
    cortarH(c, o);
    for (unsigned long t = 0; t < 30000; t += PASO_MS) pasoH(o);
    const long opM0 = MAESTRO.orden("da_otro_poste"), opE0 = ESCLAVO.orden("da_otro_poste");
    for (int d = 0; d < 2; d++) {
      EnVuelo e;
      e.trama[0] = 0; e.trama[1] = (unsigned char)H_CMD_PRESENTE; e.trama[2] = 0; e.trama[3] = 0;
      e.tEntrega = g_t;
      e.destino = d;
      g_aire.push_back(e);
    }
    for (unsigned long t = 0; t < 2000; t += PASO_MS) pasoH(o);
    const long msResp = MAESTRO.orden("da_ms_desde_respuesta");
    const long opM = MAESTRO.orden("da_otro_poste") - opM0, opE = ESCLAVO.orden("da_otro_poste") - opE0;
    const bool fuera = !MAESTRO.orden("da_en_degradado") && !ESCLAVO.orden("da_en_degradado");
    comprobar(opM == 1 && opE == 1 && fuera && msResp >= 30000,
              "H7 PRESENTE oido por un poste no degradado: $ALARM OTRO_POSTE_EN_DEGRADADO en cada uno (M " +
                  std::to_string(opM) + ", E " + std::to_string(opE) + "), ninguno entra por ella (" +
                  (fuera ? "fuera" : "ENTRO") + ") y el Maestro no la toma por respuesta (" +
                  std::to_string(msResp) + " ms desde la ultima)");
    while (g_t - o.tCorte < VENTANA && !(o.tVerde[0] && o.tVerde[1])) pasoH(o);
    comprobar(o.tEnt[0] && o.tEnt[1] && o.tEnt[1] - o.tCorte < H_ESPERA_MS + 60000UL,
              "H7b la PRESENTE no reinicio la cuenta del Esclavo: entra a los " +
                  (o.tEnt[1] ? seg(o.tEnt[1] - o.tCorte) : std::string("NUNCA")) + " del corte (espera " +
                  seg(H_ESPERA_MS) + ")");
    bool ok = false;
    const std::string t = transversalH(ok);
    comprobar(ok, "H7t en cada tick, ni verde contra verde ni verde contra ambar: " + t);
  }

  bloqueH2(VENTANA, SIN_ENTRADA);
  g_enlace = true;
  g_cortaME = g_cortaEM = false;
}
