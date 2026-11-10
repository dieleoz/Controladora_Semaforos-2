// ===== dos_puntas/adaptador_esclavo_interruptores.inc =====
// SPEC_8 6 (D-148..D-152) en el Esclavo REAL: interruptores.cpp, semaforo.cpp, respaldo.cpp,
// main.cpp y -sin ARNES_RELOJ_REAL- bluetooth.cpp, compilados en esta DLL. Lo incluye
// adaptador_esclavo.cpp en sus dos variantes; las preguntas entran por punta_mando().
#include "interruptores.h"

// botones.cpp no se compila aqui: camara_estado() es un literal que el orquestador elige, para
// ejercer las dos lecturas de CAM:NOVETO (D-151), '?' incluido -el arnes del automatico no
// puede volver a '?' tras sus bloques F y G-.
static const char* g_camEstadoArnes = "OK";
const char* camara_estado() { return g_camEstadoArnes; }

#ifndef ARNES_RELOJ_REAL
// La ULTIMA linea de $EVENT de ORIGEN:INTERRUPTOR y de $STATUS que el equipo emitio por el
// puerto del telefono. Solo las del interruptor: el arranque emite otros $EVENT detras.
static char g_btUltimoEvento[192] = "", g_btUltimoStatus[192] = "";
static void arnesInterruptores_linea(const char* l, unsigned long* eventos) {
  if (!strncmp(l, "$EVENT", 6)) {
    (*eventos)++;
    if (strstr(l, "ORIGEN:INTERRUPTOR")) snprintf(g_btUltimoEvento, sizeof(g_btUltimoEvento), "%s", l);
  } else if (!strncmp(l, "$STATUS", 7)) {
    snprintf(g_btUltimoStatus, sizeof(g_btUltimoStatus), "%s", l);
  }
}
#endif

static long arnesInterruptores_mando(const char* que, long arg) {
  if (!strcmp(que, "cam_estado")) {
    static const char* const E[] = {"?", "OK", "PEGADA", "CIEGA"};
    g_camEstadoArnes = E[arg & 3];
    return 1;
  }
  if (!strcmp(que, "pluma_fuera"))        return semaforo_plumaFuera() ? 1 : 0;
  if (!strcmp(que, "rojo_espera"))        return semaforo_rojoEspera() ? 1 : 0;   // D-175, el getter REAL
  if (!strcmp(que, "respaldo_perdido"))   return respaldo_perdidoAlArrancar() ? 1 : 0;
#ifndef ARNES_RELOJ_REAL
  if (!strncmp(que, "evento:", 7))        return strstr(g_btUltimoEvento, que + 7) ? 1 : 0;
  if (!strncmp(que, "status:", 7))        return strstr(g_btUltimoStatus, que + 7) ? 1 : 0;
#endif
  return PUNTA_DESCONOCIDO;
}
