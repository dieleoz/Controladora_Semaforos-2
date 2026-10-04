// ===== include/testigo_flash.h =====
#pragma once
#include <Arduino.h>

// ---------------------------------------------------------------------------
// D-35 - EL REGISTRO DEL DEGRADADO CON TESTIGO, EN LA ULTIMA PAGINA DE LA FLASH.
//
// ESTE FICHERO Y SU .cpp DEBEN SER IDENTICOS EN MAESTRO Y ESCLAVO.
//
// Por que aqui y no en la pila (SPEC_2 7.bis): los diez registros de respaldo del F103
// estan repartidos en respaldo.cpp y el registro del testigo no cabe sin retirar otras
// garantias. La pagina de 1 KB en TESTIGO_FLASH_DIR queda fuera del alcance del
// enlazador por board_upload.maximum_size = 64512 en platformio.ini: si el codigo
// llegara hasta aqui, el enlace falla en vez de pisarla.
//
// QUE GUARDA. Solo los parametros del testigo aceptado. Si el testigo SIGUE PUESTO no lo
// dice esta pagina: lo dice el bit FLAG_TESTIGO de la pila (respaldo_testigoActivo()),
// que borran todas las salidas del Degradado sin tocar la flash. La flash se escribe UNA
// vez por testigo aceptado, con el poste en rojo, y nunca al salir.
//
// Los instantes van en SEGUNDOS DESDE LA EPOCA DEL DS3231 (SU ANIO 00) contados con la fecha del DS3231 que
// trae CMD:HORA_ESP32 (reloj_segundosDesde2000()), no con el contador del RTC del STM32.
//
// `-e all` del upload (CLAUDE.md 3) borra esta pagina al cargar firmware: tras una carga
// no hay testigo que reanudar, y la reanudacion lo trata como ausente.
// ---------------------------------------------------------------------------

static const uint32_t TESTIGO_FLASH_DIR = 0x0800FC00UL;   // ultima pagina de 1 KB del mapa de 64 KB

struct TestigoFlash {
  uint32_t marcaS;    // instante en que se acepto el ultimo testigo, s desde 2000
  uint32_t inicioS;   // instante de inicio, s desde 2000
  uint8_t verdeSeg;
  uint8_t despejeSeg;
  uint32_t salidaS;   // D-52: salida programada, s desde 2000; 0 = ninguna
};

// true si la pagina tiene un registro integro (firma, version y suma). Con false, *t no se
// toca: quien llama no puede quedarse con medio registro.
bool testigoFlash_leer(TestigoFlash* t);

// Borra la pagina, programa el registro y lo relee. Devuelve true solo si lo releido
// coincide con lo pedido. *usBorrado recibe lo que tardo el borrado, en microsegundos,
// medido con el contador de ciclos del nucleo (el SysTick no corre durante el borrado).
bool testigoFlash_escribir(const TestigoFlash* t, uint32_t* usBorrado);
