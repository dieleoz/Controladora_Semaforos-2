// ===== dos_puntas/instalacion_mandos.inc =====
// N-187 (SPEC_8 8): la pagina de instalacion de ESTA punta, modelada con memoria
// (../instalacion_modelo.inc), y las preguntas del bloque I del orquestador. La incluyen
// adaptador_maestro.cpp y adaptador_esclavo_instalacion.inc. instalacion.cpp y semaforo.cpp
// son los REALES; lo unico doble es el acceso crudo a la flash, que en el PC no existe.
// La pagina vive en la DLL: un microcorte (FreeLibrary) la BORRA, como `-e all`. Para un
// corte de luz el orquestador la vuelca antes ("inst_hw") y la repone despues ("inst_poner").
#include "../instalacion_modelo.inc"

static char g_instResp[80] = "";

static long arnesInstalacion_mando(const char* que, long arg) {
  if (!strcmp(que, "inst_hw"))         return (long)instalacionFlash_leerHw((uint8_t)arg);
  if (!strcmp(que, "inst_poner")) {    // arg = (indice << 16) | media palabra, en crudo
    const long i = arg >> 16;
    if (i < 0 || i >= INSTALACION_FLASH_HW) return 0;
    g_instPag[i] = (uint16_t)((arg & 0xFFFF) ^ 0xFFFFU);
    return 1;
  }
  if (!strcmp(que, "inst_borrar"))     { instBorrar(); return 1; }
  // Una pagina VALIDA compuesta con el formato de SPEC_8 8.6: arg = ncC | ncD << 1 | rSeg << 2.
  if (!strcmp(que, "inst_componer"))   { instComponer(arg & 1, (arg >> 1) & 1, (uint16_t)(arg >> 2), true); return 1; }
  if (!strcmp(que, "inst_corte_tras")) { g_instCorteTras = (int)arg; return 1; }
  if (!strcmp(que, "inst_escrituras")) return (long)g_instEscrituras;
  if (!strcmp(que, "inst_retardo_ms")) return (long)instalacion_retardoPlumaMs();
  if (!strcmp(que, "inst_nc"))         return instalacion_camaraNC((int)arg) ? 1 : 0;
  if (!strcmp(que, "pluma_vetada"))    return semaforo_plumaVetada() ? 1 : 0;
  if (!strcmp(que, "pluma_mala"))      return semaforo_plumaRetenidaMala() ? 1 : 0;   // D-169
  // La orden LOCAL por instalacion_orden() REAL, sin bluetooth.cpp (el Maestro de este
  // arnes no lo compila). La respuesta se compara entera con "inst_igual:".
  if (!strncmp(que, "inst_orden:", 11)) {
    const char* r = instalacion_orden(que + 11);
    snprintf(g_instResp, sizeof(g_instResp), "%s", r ? r : "(nullptr)");
    return 1;
  }
  if (!strncmp(que, "inst_igual:", 11)) return strcmp(g_instResp, que + 11) == 0 ? 1 : 0;
  // SFTY-28/D-168 al encender: J15 envenenado en BAJAR (lo contrario de lo que debe dejar) y las luces
  // apagadas; se corre SOLO semaforo_setup() REAL, lo primero que toca pines en el arranque de las dos
  // puntas. bit0: J15 en ABRIR (nivel de reset), escrito y anotado arriba; bit1: ninguna luz encendida.
  // El setup() entero no sirve para esto: su forzarRojo() escribe J15 de nuevo y taparia
  // un semaforo_setup() que no lo cerrara.
  if (!strcmp(que, "arranque_envenenado")) {
    arnes_pines[MOTOR_TALANQUERA] = TALANQUERA_CERRAR;
    arnes_toques[MOTOR_TALANQUERA] = 0;
    semaforo_setup();
    const int luces[] = {ROJO1, AMARILLO1, VERDE1, ROJO2, AMARILLO2, VERDE2};
    bool apagadas = true;
    for (int p : luces) apagadas = apagadas && arnes_pines[p] == LOW;
    return (arnes_pines[MOTOR_TALANQUERA] == TALANQUERA_ABRIR && semaforo_plumaArriba() &&
            arnes_toques[MOTOR_TALANQUERA] > 0 ? 1 : 0) | (apagadas ? 2 : 0);
  }
  return PUNTA_DESCONOCIDO;
}
