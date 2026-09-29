// Dobles de D-35 (testigo), incluidos desde arnes_puente.cpp.
// D-35: reloj.cpp no se compila aqui; su reloj de arnes no guarda fecha, asi que no hay dia
// del DS3231 que guardar ni que contar.
bool reloj_guardarFechaEsp32(const char*) { return false; }
uint32_t reloj_segundosDesde2000() { return 0; }

