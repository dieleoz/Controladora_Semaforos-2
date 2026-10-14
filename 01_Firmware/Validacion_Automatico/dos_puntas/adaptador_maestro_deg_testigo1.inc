// Dobles de D-35 (testigo), incluidos desde adaptador_maestro_deg.cpp.

// ---------------------------------------------------------------------------
// D-35 - DOBLE MINIMO DE LA PAGINA DEL TESTIGO. Lo escriben los bloques H, J y K (D-59, abajo); el
// testigo en las tarjetas se valida en banco (decision del responsable del 29/09). Existe tambien
// para que modo_degradado.cpp enlace. testigo_flash.cpp no se compila aqui porque lee y
// escribe la direccion absoluta 0x0800FC00 con el HAL de flash, que en el PC no existen; su
// doble es una pagina en RAM que arranca vacia.
#include "testigo_flash.h"
static bool g_testigoFlashLleno = false;
static TestigoFlash g_testigoFlash;
bool testigoFlash_leer(TestigoFlash* t) {
  if (!g_testigoFlashLleno) return false;
  *t = g_testigoFlash;
  return true;
}
bool testigoFlash_escribir(const TestigoFlash* t, uint32_t* usBorrado) {
  if (g_daFlashFalla) return false;   // bloque H: el borrado que falla (NO_GUARDADO)
  g_testigoFlash = *t;
  g_testigoFlashLleno = true;
  if (usBorrado) *usBorrado = 0;
  return true;
}

// D-59 - BLOQUE K del orquestador del Degradado (orquestador_deg_verde.inc): el testigo con el VERDE elegido y la
// pagina que SOBREVIVE al corte. La pagina real esta en flash: un corte no la borra. El FreeLibrary si borra este
// doble, asi que el orquestador la lee campo a campo antes de descargar la DLL y la repone despues (dt_flash /
// dt_flash_pon:i), igual que hace con la pila. dt_testigo devuelve 0 OK, 1 RENOVADO, 2 VERDE_RANGO, 10+motivo el resto.
static long g_dtVerde = 180;
static long arnesTestigo_mando(const char* que, long arg) {
  TestigoFlash& f = g_testigoFlash;
  if (!strcmp(que, "dt_verde")) { g_dtVerde = arg; return 1; }
  if (!strcmp(que, "dt_testigo")) {
    const long m = (long)modo_degradado_entrarTestigo(reloj_segundosDelDia(), (uint32_t)arg, (int)g_dtVerde, DEG_AUTO_DESPEJE_S);
    return m == MDT_OK ? 0 : m == MDT_RENOVADO ? 1 : m == MDT_VERDE_RANGO ? 2 : 10 + m;
  }
  if (!strcmp(que, "dt_flash")) {
    const long v[6] = { g_testigoFlashLleno ? 1L : 0L, (long)f.marcaS, (long)f.inicioS, (long)f.verdeSeg,
                        (long)f.despejeSeg, (long)f.salidaS };
    return (arg >= 0 && arg < 6) ? v[arg] : PUNTA_DESCONOCIDO;
  }
  if (!strncmp(que, "dt_flash_pon:", 13)) {
    switch (que[13] - '0') {
      case 0: g_testigoFlashLleno = arg != 0; return 1;
      case 1: f.marcaS = (uint32_t)arg; return 1;
      case 2: f.inicioS = (uint32_t)arg; return 1;
      case 3: f.verdeSeg = (uint16_t)arg; return 1;
      case 4: f.despejeSeg = (uint8_t)arg; return 1;
      case 5: f.salidaS = (uint32_t)arg; return 1;
      default: return PUNTA_DESCONOCIDO;
    }
  }
  return PUNTA_DESCONOCIDO;
}
