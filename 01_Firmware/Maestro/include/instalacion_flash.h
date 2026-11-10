#pragma once
#include <Arduino.h>

// ---------------------------------------------------------------------------
// N-187 (SPEC_8 8.6, D-161..D-165) - LA PAGINA DE LOS PARAMETROS DE INSTALACION.
//
// ESTE FICHERO Y SU .cpp SON IDENTICOS EN MAESTRO Y ESCLAVO.
//
// Solo el acceso CRUDO a la pagina, molde testigo_flash: leer una media palabra y
// borrar + programar + releer. El formato (firma, version, ncC, ncD, retardo, suma) y lo
// que vale cada media palabra viven en instalacion.cpp, que es lo que el arnes de PC
// compila real; esto es lo unico que se dobla alli, porque la pagina no existe en el PC.
//
// La pagina es la tercera empezando por el final (0x0800F400): board_upload.maximum_size =
// 62464 en platformio.ini deja las TRES ultimas fuera del alcance del enlazador.
//
// `-e all` del upload (CLAUDE.md 3) la borra al cargar firmware: TRAS CADA CARGA EL EQUIPO
// VUELVE A FABRICA (NA, NA, 3 s). Un equipo con una camara NC queda declarado NA y ve
// presencia permanente (pluma arriba en rojo) hasta que se reenvia SET_CAMARA.
// ---------------------------------------------------------------------------

static const uint32_t INSTALACION_FLASH_DIR = 0x0800F400UL;   // antepenultima pagina de 1 KB
static const uint8_t INSTALACION_FLASH_HW = 7;               // medias palabras del registro

// La media palabra i de la pagina (0xFFFF fuera de rango, como una pagina borrada).
uint16_t instalacionFlash_leerHw(uint8_t i);

// Borra la pagina, programa las INSTALACION_FLASH_HW medias palabras y las relee. true solo
// si lo releido coincide con lo pedido. Con false la pagina puede quedar borrada (= fabrica).
bool instalacionFlash_escribir(const uint16_t hw[INSTALACION_FLASH_HW]);
