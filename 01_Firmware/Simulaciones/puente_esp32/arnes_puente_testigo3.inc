// Dobles de D-35 (testigo), incluidos desde arnes_puente.cpp.
// D-35: modo_degradado.cpp real pide el permiso de testigo de la pila y la pagina de flash.
// Dobles minimos, sin memoria: este simulador no ejerce el testigo y nada los escribe.
void respaldo_guardarTestigo() {}
bool respaldo_testigoActivo() { return false; }
#include "testigo_flash.h"
bool testigoFlash_leer(TestigoFlash*) { return false; }
bool testigoFlash_escribir(const TestigoFlash*, uint32_t* us) { if (us) *us = 0; return false; }

// A-15 (1a78873, 917ef1e): respaldo.cpp y reloj.cpp del Esclavo no se compilan aqui. La
// rendicion no se guarda (sin memoria, como el testigo) y el reloj de arnes no tiene fecha
// que anclar: fecharDesdeEsp32 devuelve false, como reloj_guardarFechaEsp32 de testigo2.
void respaldo_guardarRendido(bool) {}
void respaldo_guardarRojoSinHora() {}   // D-47: sin memoria, como la rendicion
bool reloj_fecharDesdeEsp32(const char*) { return false; }
