// Dobles de D-35 (testigo), incluidos desde arnes_puente.cpp.
// D-35: reloj.cpp no se compila aqui; su reloj de arnes no guarda fecha, asi que no hay dia
// del DS3231 que guardar ni que contar.
bool reloj_guardarFechaEsp32(const char*) { return false; }
uint32_t reloj_segundosDesde2000() { return 0; }

#include "arnes_puente_deg_auto.inc"  // A-15: dobles del Degradado automatico, las dos puntas

// SPEC_8 6 (D-95, D-148): los dos interruptores, en ON y sin memoria. interruptores.cpp entra REAL
// en las dos puntas porque bluetooth.cpp lo llama; las ordenes las ejercen arnes_automatico y dos_puntas.
void respaldo_guardarVetoCamaraOff(bool) {}
bool respaldo_vetoCamaraOff() { return false; }
void respaldo_guardarBarreraFuera(bool) {}
bool respaldo_barreraFuera() { return false; }
bool respaldo_perdidoAlArrancar() { return false; }
