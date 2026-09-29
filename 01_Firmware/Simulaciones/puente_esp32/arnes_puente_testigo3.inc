// Dobles de D-35 (testigo), incluidos desde arnes_puente.cpp.
// D-35: modo_degradado.cpp real pide el permiso de testigo de la pila y la pagina de flash.
// Dobles minimos, sin memoria: este simulador no ejerce el testigo y nada los escribe.
void respaldo_guardarTestigo() {}
bool respaldo_testigoActivo() { return false; }
#include "testigo_flash.h"
bool testigoFlash_leer(TestigoFlash*) { return false; }
bool testigoFlash_escribir(const TestigoFlash*, uint32_t* us) { if (us) *us = 0; return false; }

