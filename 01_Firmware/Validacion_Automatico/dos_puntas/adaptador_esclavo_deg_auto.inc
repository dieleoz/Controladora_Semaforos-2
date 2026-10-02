// ===== Validacion_Automatico/dos_puntas/adaptador_esclavo_deg_auto.inc =====
//
// Incluido desde adaptador_esclavo.cpp SOLO con -DARNES_RELOJ_REAL (compilar_degradado.ps1).
// Vive aparte porque aquel pasa de 500 lineas y no puede crecer. Lleva el bluetooth simulado
// de esa variante -mudado aqui literal el 29/09- y lo que el BLOQUE H del orquestador
// (orquestador_deg_auto.inc) pide a esta punta para el Degradado automatico (A-15).
#include "deg_auto.h"

// ---------------------------------------------------------------------------
// LO QUE EL FIRMWARE PUBLICA DE A-15, CONTADO POR SU DETALLE. Mismo contrato que en
// adaptador_maestro_deg_auto.inc.
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
// BLUETOOTH SIMULADO (arnes del Degradado). bluetooth.cpp no se compila aqui -arrastra
// el puerto serie entero y lo que este arnes mide es la FASE, no el despachador del
// telefono-, pero bluetooth_ambarEmergencia() NO puede ser un false fijo: es una de las
// DOS guardas que vetan un CMD_GO_GREEN en el despachador real (N-83). El orquestador la
// mueve, de modo que la rama vetada se recorre.
// ---------------------------------------------------------------------------
static bool g_ambarEmergencia = false;
static char g_ultimaAlarmaEvento[48] = "";
static char g_ultimaAlarmaCausa[48] = "";
static int  g_alarmasEmitidas = 0;
// D-21 (1): la alarma de la hora caducada, por su causa; y el $EVENT del salto que pasa por
// rojo (D-26 (4)), por su detalle. Ver el mismo par en adaptador_maestro_deg.cpp.
static int  g_alarmasCaducada = 0;
static int  g_eventosSaltoRojo = 0;

void bluetooth_setup() {}
void bluetooth_loop() {}
bool bluetooth_ambarEmergencia() { return g_ambarEmergencia; }
bool bluetooth_testLedsActivo() { return false; }
void bluetooth_reportarAlarma(const char* evento, const char* causa, const char* accion) {
  if (accion && (!strncmp(causa, "AUTO_NO_", 8) || !strcmp(causa, "CADUCADA")))
    snprintf(g_daUltAccion, sizeof(g_daUltAccion), "%s", accion);
  snprintf(g_ultimaAlarmaEvento, sizeof(g_ultimaAlarmaEvento), "%s", evento);
  snprintf(g_ultimaAlarmaCausa, sizeof(g_ultimaAlarmaCausa), "%s", causa);
  g_alarmasEmitidas++;
  arnesDegAuto_notar(evento, causa);   // A-15: bloque H
  if (!strcmp(evento, "HORA_ESP32") && !strcmp(causa, "CADUCADA")) g_alarmasCaducada++;
}
void bluetooth_reportarEvento(const char* origen, const char* detalle) {
  arnesDegAuto_notar(origen, detalle);   // A-15: bloque H
  if (!strcmp(origen, "DEGRADADO") && !strcmp(detalle, "SALTO_DE_HORA_POR_ROJO")) {
    g_eventosSaltoRojo++;
  }
}
// D-31: main.cpp del Esclavo la llama al recibir CMD_ACK_AVISO_AMBAR, y en ESTA variante
// bluetooth.cpp no se compila. Vacia a proposito: lo que la funcion hace -recordar que el
// Poste 1 acuso- vive dentro del despachador real, y este arnes mide la FASE, no el
// acuse. Quien lo ejerce es el arnes de las dos puntas, que si compila bluetooth.cpp.
// Un stub que contase llamadas aqui seria un contador que nadie lee (N-73 al reves).
void bluetooth_avisoAmbarAcusado() {}

// ---------------------------------------------------------------------------
// LA SIEMBRA DE ESTA PUNTA, como la rama CMD:HORA_ESP32 de bluetooth.cpp la hace desde a0d605b:
// si reloj_sembrarDesdeIso() entra, reloj_guardarFechaEsp32() con la MISMA cadena (D-35). Sin
// la fecha, degradado_entrarTestigo() rechaza por DEG_RECHAZO_T_SIN_HORA y el Degradado
// automatico no entraria nunca en este arnes.
//
// g_daDiasExtra: el adelanto de fecha de SPEC_2 7.ter (h)6. La siembra del arnes lleva siempre
// "2026-01-DD" (arnes_iso); esto suma dias a la FECHA sin tocar la hora del dia. Con 0 devuelve
// la misma cadena.
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

// La rama con la radio mandando: bluetooth.cpp guarda SOLO la fecha (reloj_fecharDesdeEsp32).
// En eco (iso nulo) el ESP32 trae la misma hora y el dia del mes que la radio.
static int arnesDegAuto_ignorada(const char* iso) {
  char buf[24];
  if (iso == nullptr) { arnes_iso(buf, sizeof(buf), reloj_dia(), (long)reloj_segundosDelDia()); iso = buf; }
  reloj_fecharDesdeEsp32(arnesDegAuto_fecha(iso));
  return 2;
}

static int arnesDegAuto_sembrar(const char* iso) {
  iso = arnesDegAuto_fecha(iso);
  if (!reloj_sembrarDesdeIso(iso)) return 0;
  reloj_guardarFechaEsp32(iso);
  return 1;
}

// ---------------------------------------------------------------------------
// LAS ORDENES DEL BLOQUE H. PUNTA_DESCONOCIDO si no es suya: punta_mando() sigue buscando.
// ---------------------------------------------------------------------------
static long arnesDegAuto_mando(const char* que, long arg) {
  // EL millis() DE UN MICRO RECIEN ARRANCADO. Tras un FreeLibrary + LoadLibrary, punta_tick() le
  // pasaria el reloj del BANCO (horas) y "desde el arranque" -tCuenta = 0 de deg_auto.cpp- ya
  // habria vencido en la primera vuelta: el Esclavo entraba 5 s despues de reiniciar (MEDIDO,
  // H5). Se ancla el HSI de esta punta para que el banco 'arg' sea el millis() que dejo setup().
  if (!strcmp(que, "da_arranque_en")) {
    const unsigned long local = arnes_millis_valor;
    arnes_reloj_local((unsigned long)arg);
    arnes_millis_valor = local;
    arnes_hsi_ppm(0);
    return (long)local;
  }
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
  if (!strcmp(que, "da_en_degradado")) return degradado_gobiernaLuz() ? 1 : 0;
  // Bloque H, 29/09: rendicion y su pila, el ambar por ESTADO (el pin del intermitente se
  // apaga la mitad del tiempo), la entrada a mano por testigo y la ACCION literal.
  if (!strcmp(que, "da_rojo_fijo"))   return degradado_estado() == DEG_ROJO_SIN_HORA ? 1 : 0;   // D-38
  if (!strcmp(que, "da_rendido_pila")) return respaldo_rendido() ? 1 : 0;
  if (!strcmp(que, "da_en_fallo"))    return semaforo_estado() == S_FALLO ? 1 : 0;
  if (!strcmp(que, "da_s2000"))       return (long)reloj_segundosDesde2000();
  if (!strcmp(que, "da_flash_falla")) { g_daFlashFalla = (arg != 0); return 1; }
  if (!strcmp(que, "da_testigo"))
    return degradado_entrarTestigo(reloj_segundosDelDia(), (uint32_t)arg, DEG_AUTO_DESPEJE_S) == DEG_T_ACEPTADO;
  if (!strncmp(que, "da_accion_es:", 13)) return strcmp(g_daUltAccion, que + 13) == 0 ? 1 : 0;
  return PUNTA_DESCONOCIDO;
}
