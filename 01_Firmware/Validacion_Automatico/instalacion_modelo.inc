// N-187 (SPEC_8 8.6): la pagina de instalacion doblada con memoria. instalacion.cpp entra REAL
// (formato, suma, getters y ordenes); solo el acceso crudo a la flash es doble, porque la
// pagina de 0x0800F400 no existe en el PC. Incluido en arnes_automatico.cpp a nivel de fichero.
#include "instalacion.h"
#include "instalacion_flash.h"

// Guardado en XOR 0xFFFF para que el cero inicial sea una pagina BORRADA (0xFFFF), como tras
// `-e all`. g_instFalla: la escritura no queda (la pagina queda borrada), para NO_QUEDO_PUESTO.
static uint16_t g_instPag[INSTALACION_FLASH_HW];
static bool g_instFalla = false;
static int g_instEscrituras = 0;
// Corte de luz A MITAD de la escritura (dos_puntas, bloque I): borrada la pagina, solo se
// programan las g_instCorteTras primeras medias palabras (-1 = sin corte). Lo que haga el
// firmware despues no cuenta: el orquestador recarga la DLL en ese mismo tick.
static int g_instCorteTras = -1;

uint16_t instalacionFlash_leerHw(uint8_t i) {
  return i < INSTALACION_FLASH_HW ? (uint16_t)(g_instPag[i] ^ 0xFFFFU) : (uint16_t)0xFFFFU;
}
bool instalacionFlash_escribir(const uint16_t hw[INSTALACION_FLASH_HW]) {
  g_instEscrituras++;
  for (uint8_t i = 0; i < INSTALACION_FLASH_HW; i++) {
    const bool programada = !g_instFalla && (g_instCorteTras < 0 || i < g_instCorteTras);
    g_instPag[i] = programada ? (uint16_t)(hw[i] ^ 0xFFFFU) : (uint16_t)0;
  }
  for (uint8_t i = 0; i < INSTALACION_FLASH_HW; i++) {
    if (instalacionFlash_leerHw(i) != hw[i]) return false;
  }
  return true;
}

// Una pagina compuesta por el ARNES con el formato de SPEC_8 8.6 (firma, version, ncC, ncD,
// retardo, suma de 32 bits). La firma, la version y la semilla de la suma son las del fuente:
// K5 lo comprueba antes de usarla (una pagina valida con R4 tiene que leerse R4); si el
// formato cambia, esa comprobacion cae en vez de aprobar paginas que el firmware no reconoce.
static void instComponer(uint16_t ncC, uint16_t ncD, uint16_t rSeg, bool sumaBuena) {
  uint16_t hw[INSTALACION_FLASH_HW] = {0x1A57, 1, ncC, ncD, rSeg, 0, 0};
  uint32_t s = 0x2B61U;
  for (int i = 1; i <= 4; i++) s = s * 31U + hw[i];
  if (!sumaBuena) s ^= 1U;
  hw[5] = (uint16_t)(s & 0xFFFFU);
  hw[6] = (uint16_t)(s >> 16);
  for (uint8_t i = 0; i < INSTALACION_FLASH_HW; i++) g_instPag[i] = (uint16_t)(hw[i] ^ 0xFFFFU);
}
static void instBorrar() {
  for (uint8_t i = 0; i < INSTALACION_FLASH_HW; i++) g_instPag[i] = 0;
}
