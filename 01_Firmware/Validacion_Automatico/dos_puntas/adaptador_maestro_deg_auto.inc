// ===== Validacion_Automatico/dos_puntas/adaptador_maestro_deg_auto.inc =====
//
// Incluido desde adaptador_maestro_deg.cpp, detras de sus cabeceras. Vive aparte porque aquel
// pasa de 500 lineas y no puede crecer. Lleva dos cosas:
//
//   1. pasoPrincipal(), la transcripcion de Maestro/src/main.cpp::loop(). Se mudo aqui el
//      29/09 (A-15) al ganar lo que a0d605b anadio a loop() -degAuto_loop() y el lector de
//      PRESENTE en Degradado- y los dos modos que el Degradado automatico necesita ver
//      -AUTOMATICO, donde la opcion se vuelve APTO, y MANUAL, donde deja de serlo-.
//   2. Lo que el BLOQUE H del orquestador (orquestador_deg_auto.inc) pide a esta punta: la
//      opcion y el APTO, los $EVENT/$ALARM de A-15 contados por su detalle, y el adelanto de
//      fecha de la SPEC_2 7.ter (h)6 ("vencimiento con el reloj adelantado a 31 dias").
//
// LA OPCION SE PONE CON respaldo_guardarDegAuto(), NO CON SET_DEG_AUTO: bluetooth.cpp no se
// compila en este arnes. El $ACK diferido de SET_DEG_AUTO no lo ejerce el bloque H.
#include "deg_auto.h"
#include "modo_automatico.h"
#include "modo_manual.h"

// ---------------------------------------------------------------------------
// LO QUE EL FIRMWARE PUBLICA DE A-15, CONTADO POR SU DETALLE. Lo llaman los dobles de
// bluetooth_reportarAlarma()/bluetooth_reportarEvento() del adaptador.
// ---------------------------------------------------------------------------
static long g_daEntradas = 0, g_daRechazos = 0, g_daEnlace = 0, g_daOtroPoste = 0;
static long g_daLimite31 = 0, g_daRenovar = 0, g_daRojoSinHora = 0;   // D-38: $ALARM ROJO_SIN_HORA
static long g_daInicioS = -1;   // el inicio que publico AUTO_ENTRADA_INICIO_HH:MM:SS
static char g_daUltRechazo[64] = "";   // la causa del ultimo AUTO_NO_, literal
static char g_daUltAccion[32] = "";    // la ACCION del ultimo AUTO_NO_, literal (la app la traduce)
static bool g_daFlashFalla = false;    // la pagina del testigo no se deja escribir (testigo1.inc)

static void arnesDegAuto_notar(const char* origen, const char* detalle) {
  if (strcmp(origen, "DEGRADADO") != 0 || detalle == nullptr) return;
  if (!strncmp(detalle, "AUTO_ENTRADA_INICIO_", 20)) {
    g_daEntradas++;
    int h = 0, m = 0, x = 0;
    if (sscanf(detalle + 20, "%d:%d:%d", &h, &m, &x) == 3) g_daInicioS = h * 3600L + m * 60L + x;
  }
  if (!strncmp(detalle, "AUTO_NO_", 8)) {
    g_daRechazos++;
    snprintf(g_daUltRechazo, sizeof(g_daUltRechazo), "%s", detalle);
  }
  if (!strcmp(detalle, "ENLACE_DISPONIBLE")) g_daEnlace++;
  if (!strcmp(detalle, "OTRO_EN_DEGRADADO")) g_daOtroPoste++;
  if (!strcmp(detalle, "LIMITE_31D")) g_daLimite31++;
  if (!strcmp(detalle, "RENOVAR_TESTIGO")) g_daRenovar++;
  if (!strcmp(detalle, "ROJO_SIN_HORA")) g_daRojoSinHora++;
}

// ---------------------------------------------------------------------------
// EL ADELANTO DE FECHA. La siembra del arnes lleva siempre "2026-01-DD" (arnes_iso): con eso
// no hay forma de llegar a 31 dias. g_daDiasExtra suma dias a la FECHA de toda siembra que
// pase por ramaHoraEsp32(), dejando la hora del dia intacta -la fase del Degradado no se
// mueve-. Con 0, devuelve la misma cadena: las filas de los bloques A..G no cambian.
// ---------------------------------------------------------------------------
static long g_daDiasExtra = 0;

static const char* arnesDegAuto_fecha(const char* iso) {
  static char buf[24];
  int a = 0, m = 0, d = 0;
  if (g_daDiasExtra == 0 || sscanf(iso, "%d-%d-%d", &a, &m, &d) != 3 || strlen(iso) < 11) return iso;
  static const int DIAS_MES[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  for (long k = 0; k < g_daDiasExtra; k++) {
    if (++d > DIAS_MES[m - 1] + ((m == 2 && a % 4 == 0) ? 1 : 0)) { d = 1; if (++m > 12) { m = 1; a++; } }
  }
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d%s", a, m, d, iso + 10);
  return buf;
}

// ---------------------------------------------------------------------------
// UN TICK DE main.cpp, EN EL ORDEN REAL Y CON SUS REGLAS DE FONDO.
//
// Transcripcion de Maestro/src/main.cpp::loop() acotada a los modos que esta DLL compila
// (AUTOMATICO, MANUAL, DEGRADADO, AMBAR). Las lineas que NO son despacho y si son
// comportamiento se conservan tal cual, porque deciden luz:
//
//   1. semaforo_actualizar() SIEMPRE, en todos los modos. El comentario de main.cpp
//      explica que sin esto el cabezal se quedaba a oscuras.
//   2. El coordinador de fondo queda FUERA en AUTOMATICO (lo llama modoAutomatico_loop()),
//      DEGRADADO y AMBAR: en esos dos ultimos el Maestro CALLA en la radio a proposito. En
//      AMBAR oye (N-152) y en DEGRADADO solo atiende PRESENTE (A-15).
//   3. Al SALIR del Degradado por cualquier via se borra el indicador de la pila (N-20).
//   4. degAuto_loop() antes de leer 'modo', como la reanudacion (A-15).
//
// NO SE TRANSCRIBE el consumo de CMD_AMBAR_ESCLAVO (N-142) ni menu_loop(): ninguna fila
// del bloque H pide ambar desde el Esclavo ni pasa por el menu. Si main.cpp mueve algo de
// lo transcrito, esto se queda viejo.
// ---------------------------------------------------------------------------
static ModoSistema modoAnterior = MENU;
static bool esperaReanudacion = false;   // D-40, como main.cpp
static void entrarAmbarDeArranque() { modo_ambar_fijarMotivoDeArranque(); modoActual_set(MODO_AMBAR); }

static void pasoPrincipal() {
  reloj_actualizar();
  botones_actualizar();
  semaforo_actualizar();

  // D-29 / 1.49(b): la reanudacion que main.cpp vuelve a preguntar en cada vuelta.
  if (modo_degradado_reanudarTrasCorte()) {
    modoActual_set(MODO_DEGRADADO);
  }
  if (esperaReanudacion && (modoActual_get() != MENU || !respaldo_degradadoActivo())) {   // D-40
    esperaReanudacion = false;
    if (modoActual_get() == MENU) entrarAmbarDeArranque();
  }
  degAuto_loop();   // A-15: cuenta, entrada automatica y PRESENTE en Degradado

  ModoSistema modo = modoActual_get();
  if (modo != MODO_AUTOMATICO && modo != MODO_DEGRADADO && modo != MODO_AMBAR) {
    coordinador_actualizar_background();
  } else if (modo == MODO_AMBAR) {
    coordinador_escucharEnAmbar();
  } else if (modo == MODO_DEGRADADO) {
    degAuto_escucharEnDegradado();   // A-15: solo PRESENTE; lo demas se descarta
  }

  if (modo != modoAnterior) {
    if (modoAnterior == MODO_DEGRADADO) {
      respaldo_guardarDegradado(false);
    }
    switch (modo) {
      case MODO_MANUAL:      modoManual_setup();      break;
      case MODO_AUTOMATICO:  modoAutomatico_setup();  break;
      case MODO_DEGRADADO:   modo_degradado_setup();  break;
      case MODO_AMBAR:       modo_ambar_setup();      break;
      case MENU:             menu_setup();            break;
      default: break;
    }
    modoAnterior = modo;
  }

  switch (modo) {
    case MODO_MANUAL:     modoManual_loop();      break;
    case MODO_AUTOMATICO: modoAutomatico_loop();  break;
    case MODO_DEGRADADO:  modo_degradado_loop();  break;
    case MODO_AMBAR:      modo_ambar_loop();      break;
    default: break;   // MENU: menu_loop() es pantalla y no se compila
  }
}

// ---------------------------------------------------------------------------
// LAS ORDENES DEL BLOQUE H. PUNTA_DESCONOCIDO si no es suya: punta_mando() sigue buscando.
// ---------------------------------------------------------------------------
static long arnesDegAuto_mando(const char* que, long arg) {
  if (!strcmp(que, "da_opcion"))      { respaldo_guardarDegAuto(arg != 0); return 1; }
  if (!strcmp(que, "da_opcion_leer")) return respaldo_degAuto() ? 1 : 0;
  if (!strcmp(que, "da_apto"))        return degAuto_aptoPropio() ? 1 : 0;
  if (!strcmp(que, "da_otro_apto"))   return respaldo_otroApto() ? 1 : 0;
  if (!strcmp(que, "da_dias_extra"))  { g_daDiasExtra = arg; return 1; }
  if (!strcmp(que, "da_entradas"))    return g_daEntradas;
  if (!strcmp(que, "da_rechazos"))    return g_daRechazos;
  if (!strcmp(que, "da_inicio_s"))    return g_daInicioS;
  if (!strncmp(que, "da_rechazo_es:", 14)) return strstr(g_daUltRechazo, que + 14) ? 1 : 0;
  if (!strcmp(que, "da_enlace"))      return g_daEnlace;
  if (!strcmp(que, "da_otro_poste"))  return g_daOtroPoste;
  if (!strcmp(que, "da_limite31"))    return g_daLimite31;
  if (!strcmp(que, "da_renovar"))     return g_daRenovar;
  if (!strcmp(que, "da_rojo_sin_hora")) return g_daRojoSinHora;
  if (!strcmp(que, "da_en_degradado")) return modoActual_get() == MODO_DEGRADADO ? 1 : 0;
  if (!strcmp(que, "da_ms_desde_respuesta")) return (long)coordinador_msDesdeRespuesta();
  // Bloque H, 29/09: lo mismo que pide a la otra punta (ver adaptador_esclavo_deg_auto.inc).
  if (!strcmp(que, "da_rendido_pila")) return respaldo_rendido() ? 1 : 0;
  if (!strcmp(que, "da_en_fallo"))    return semaforo_estado() == S_FALLO ? 1 : 0;
  if (!strcmp(que, "da_flash_falla")) { g_daFlashFalla = (arg != 0); return 1; }
  if (!strcmp(que, "da_testigo"))
    return modo_degradado_entrarTestigo(reloj_segundosDelDia(), (uint32_t)arg, DEG_AUTO_DESPEJE_S) == MDT_OK;
  if (!strncmp(que, "da_accion_es:", 13)) return strcmp(g_daUltAccion, que + 13) == 0 ? 1 : 0;
  return PUNTA_DESCONOCIDO;
}
