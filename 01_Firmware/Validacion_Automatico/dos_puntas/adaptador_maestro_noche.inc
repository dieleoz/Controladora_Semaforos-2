// ===== dos_puntas/adaptador_maestro_noche.inc =====
//
// N-179 (D-106..D-111, SPEC_0 5.10): lo que el Maestro de este arnes necesita para correr
// modo_noche.cpp REAL. Lo incluye adaptador_maestro.cpp en el sitio del stub de
// bluetooth_reportarEvento(), que aqui pasa a ANOTAR (el bloque N cuenta los $EVENT NOCHE).
//
//   - Un reloj con FECHA y "hora fiable" que pone el orquestador: modo_noche.cpp decide por el
//     minuto y el dia de la semana de reloj_segundosDesde2000().
//   - Un DOBLE de noche_flash.cpp: la pagina de 1 KB del F103 no existe en el PC. Guarda en RAM
//     y una orden la hace fallar (FLASH_NO_RELEE). Un microcorte (DLL recargada) la pierde,
//     como la pierde `-e all` al cargar.
//   - Las ordenes de texto, entregadas a modoNoche_orden() como lo hace bluetooth.cpp.
//
// QUE NO SE CUENTE COMO CUBIERTO: noche_flash.cpp y el despachador (bluetooth.cpp) no entran
// en esta DLL; los compila pio y el despachador lo enlaza el arnes del puente.
#include "modo_noche.h"
#include "noche_flash.h"

static char g_ev[64][48];
static int g_nEv = 0;
void bluetooth_reportarEvento(const char* origen, const char* detalle) {
  if (g_nEv < 64) snprintf(g_ev[g_nEv++], sizeof(g_ev[0]), "%s/%s", origen, detalle);
}
static long contarEv(const char* que) {
  long n = 0;
  for (int i = 0; i < g_nEv; i++) n += (strcmp(g_ev[i], que) == 0) ? 1 : 0;
  return n;
}

static bool g_horaFiable = false;
static long g_s2000 = 0;
static unsigned long g_s2000Ms = 0;
bool reloj_horaFiable() { return g_horaFiable; }
uint32_t reloj_segundosDesde2000() {
  return g_s2000 == 0 ? 0 : (uint32_t)(g_s2000 + (long)((arnes_millis_valor - g_s2000Ms) / 1000UL));
}

static NocheConfig g_pagina;
static bool g_hayPagina = false, g_flashFalla = false;
bool nocheFlash_leer(NocheConfig* c) {
  if (!g_hayPagina) return false;
  *c = g_pagina;
  return true;
}
bool nocheFlash_escribir(const NocheConfig* c) {
  if (g_flashFalla) return false;
  g_pagina = *c;
  g_hayPagina = true;
  return true;
}

static char g_btUltima[160] = "";
static void btCapturar(const char* p) { snprintf(g_btUltima, sizeof(g_btUltima), "%s", p); }

static long mandoNoche(const char* que, long arg) {
  if (!strncmp(que, "bt:", 3)) {   // 1 = la contesto la noche; 0 = sigue el despachador
    g_btUltima[0] = '\0';
    const bool contestada = modoNoche_orden(que + 3, btCapturar);
    // FORZAR_ROJO sigue en el despachador real: modo_degradado_forzarRojo() fuera del
    // Degradado es coordinador_forzarRojoTotal() (bluetooth.cpp); aqui se hace esa llamada.
    if (!contestada && strstr(que + 3, "FORZAR_ROJO")) coordinador_forzarRojoTotal();
    return contestada ? 1 : 0;
  }
  if (!strncmp(que, "bt_es:", 6)) {   // si no casa, se imprime lo contestado: el FALLA se lee solo
    if (strcmp(g_btUltima, que + 6) == 0) return 1;
    printf("           contesto: %s\n", g_btUltima);
    return 0;
  }
  if (!strncmp(que, "ev:", 3)) return contarEv(que + 3);
  if (!strncmp(que, "alarma_es:", 10)) return strcmp(g_ultimaAlarmaEvento, que + 10) == 0 ? 1 : 0;
  if (!strcmp(que, "noche_fecha")) { g_s2000 = arg; g_s2000Ms = arnes_millis_valor; return 1; }
  if (!strcmp(que, "hora_fiable")) { g_horaFiable = (arg != 0); return 1; }
  if (!strcmp(que, "noche_flash_falla")) { g_flashFalla = (arg != 0); return 1; }
  if (!strcmp(que, "noche_cargar")) { modoNoche_cargar(); return 1; }
  if (!strcmp(que, "fijar_modo")) { modoActual_set((ModoSistema)arg); return 1; }
  return PUNTA_DESCONOCIDO;
}
