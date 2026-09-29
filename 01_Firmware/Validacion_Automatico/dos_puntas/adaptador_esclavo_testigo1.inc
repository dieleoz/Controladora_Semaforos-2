// Dobles de D-35 (testigo), incluidos desde adaptador_esclavo.cpp.

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
#ifndef ARNES_RELOJ_REAL
// D-35: con el reloj MODELADO no hay fecha del DS3231 -el modelo no la guarda-. Sin fecha el
// testigo no entra ni reanuda, que es lo que hace el firmware en ese caso. Con el reloj REAL
// (compilar_degradado.ps1) las dos funciones son las de reloj.cpp.
uint32_t reloj_segundosDesde2000() { return 0; }
bool reloj_guardarFechaEsp32(const char*) { return false; }
#endif
