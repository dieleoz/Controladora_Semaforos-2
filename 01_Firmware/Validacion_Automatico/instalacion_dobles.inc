// N-187: dobles de la pagina de instalacion para los arneses que enlazan instalacion.cpp
// REAL (semaforo.cpp lee de el el retardo de la pluma). La pagina no existe en el PC:
// borrada = fabrica (NA, NA, 3 s), y no se escribe.
uint16_t instalacionFlash_leerHw(uint8_t) { return 0xFFFFU; }
bool instalacionFlash_escribir(const uint16_t*) { return false; }
void camaras_sembrar() {}
