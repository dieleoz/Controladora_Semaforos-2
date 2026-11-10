// Dobles de D-35 (testigo), incluidos desde arnes_puente.cpp.
// D-35: reloj.cpp no se compila aqui; su reloj de arnes no guarda fecha, asi que no hay dia
// del DS3231 que guardar ni que contar.
bool reloj_guardarFechaEsp32(const char*) { return false; }
uint32_t reloj_segundosDesde2000() { return 0; }

#include "arnes_puente_deg_auto.inc"  // A-15: dobles del Degradado automatico, las dos puntas

// SPEC_8 6 (D-94, D-95, D-148): los dos interruptores con memoria para ejercer interruptores.cpp
// REAL a traves del despachador bluetooth.cpp REAL de ambas puntas (Escenario P6).
static bool g_vetoCamOff[2] = {false, false}, g_malaBaja[3] = {false, false, false}, g_barreraFuera = false;
void respaldo_guardarVetoCamaraOff(int c, bool v) { g_vetoCamOff[c == 0 ? 0 : 1] = v; }   // D-166
bool respaldo_vetoCamaraOff(int c) { return g_vetoCamOff[c == 0 ? 0 : 1]; }
void respaldo_guardarMalaBaja(int n, bool b) { g_malaBaja[n == 1 ? 1 : 2] = b; }   // D-169
bool respaldo_malaBaja(int n) { return g_malaBaja[n == 1 ? 1 : 2]; }
void respaldo_guardarBarreraFuera(bool b) { g_barreraFuera = b; }
bool respaldo_barreraFuera() { return g_barreraFuera; }
bool respaldo_perdidoAlArrancar() { return false; }
