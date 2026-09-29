// Dobles de D-35 (testigo), incluidos desde adaptador_maestro_deg.cpp.

// ---------------------------------------------------------------------------
// D-35 - DOBLE MINIMO DE LA PAGINA DEL TESTIGO. Este arnes no ejerce el Degradado con
// testigo (se valida en banco, decision del responsable del 29/09): solo tiene que existir
// para que modo_degradado.cpp enlace. testigo_flash.cpp no se compila aqui porque lee y
// escribe la direccion absoluta 0x0800FC00 con el HAL de flash, que en el PC no existen; su
// doble es una pagina en RAM que arranca vacia, y ninguna prueba de este arnes la escribe.
#include "testigo_flash.h"
static bool g_testigoFlashLleno = false;
static TestigoFlash g_testigoFlash;
bool testigoFlash_leer(TestigoFlash* t) {
  if (!g_testigoFlashLleno) return false;
  *t = g_testigoFlash;
  return true;
}
bool testigoFlash_escribir(const TestigoFlash* t, uint32_t* usBorrado) {
  g_testigoFlash = *t;
  g_testigoFlashLleno = true;
  if (usBorrado) *usBorrado = 0;
  return true;
}
