// ===== src/testigo_flash.cpp =====
#include "testigo_flash.h"
#include <stm32f1xx_hal.h>

// Formato: doce medias palabras desde TESTIGO_FLASH_DIR.
//   [0] firma  [1] version  [2..3] marcaS  [4..5] inicioS  [6] verde  [7] despeje  [8..9] salidaS (D-52)
//   [10..11] suma de Horner de 32 bits, sin plegar, sobre [1..9] (el porque del no plegar
//          es el de respaldo.cpp, N-51)
// D-59: el verde va en SEGUNDOS (180..900) y en su propia media palabra: 900 no cabe en el
// byte de la version 2. En minutos cabria, pero ciclo, trama y el resto del registro van en
// segundos, y una unidad distinta solo aqui es una conversion que alguien olvida al leer.
// Una pagina borrada lee 0xFFFF en todo: no casa la firma y no es registro.
static const uint16_t FIRMA   = 0x7E57;
static const uint16_t VERSION = 3;   // D-59: una pagina de la version 1 o 2 se lee como ausente
static const uint8_t  N_HW    = 12;

static inline uint16_t leerHw(uint8_t i) {
  return *(volatile const uint16_t*)(TESTIGO_FLASH_DIR + 2UL * i);
}

static void componer(const TestigoFlash* t, uint16_t hw[N_HW]) {
  hw[0] = FIRMA;
  hw[1] = VERSION;
  hw[2] = (uint16_t)(t->marcaS >> 16);
  hw[3] = (uint16_t)(t->marcaS & 0xFFFFU);
  hw[4] = (uint16_t)(t->inicioS >> 16);
  hw[5] = (uint16_t)(t->inicioS & 0xFFFFU);
  hw[6] = t->verdeSeg;
  hw[7] = t->despejeSeg;
  hw[8] = (uint16_t)(t->salidaS >> 16);
  hw[9] = (uint16_t)(t->salidaS & 0xFFFFU);
  uint32_t s = 0x1F35U;
  for (uint8_t i = 1; i <= 9; i++) s = s * 31U + hw[i];
  hw[10] = (uint16_t)(s & 0xFFFFU);
  hw[11] = (uint16_t)(s >> 16);
}

bool testigoFlash_leer(TestigoFlash* t) {
  if (leerHw(0) != FIRMA || leerHw(1) != VERSION) return false;
  uint32_t s = 0x1F35U;
  for (uint8_t i = 1; i <= 9; i++) s = s * 31U + leerHw(i);
  if (leerHw(10) != (uint16_t)(s & 0xFFFFU) || leerHw(11) != (uint16_t)(s >> 16)) return false;
  const uint16_t v = leerHw(6), d = leerHw(7);
  // Un verde o un despeje a cero no es un ciclo: es ausencia de el (criterio de
  // respaldo_guardarCiclo()). Un despeje que no cabe en su byte tampoco es registro.
  if (v == 0 || d == 0 || d > 0xFFU) return false;
  t->marcaS = ((uint32_t)leerHw(2) << 16) | leerHw(3);
  t->inicioS = ((uint32_t)leerHw(4) << 16) | leerHw(5);
  t->verdeSeg = v;
  t->despejeSeg = (uint8_t)d;
  t->salidaS = ((uint32_t)leerHw(8) << 16) | leerHw(9);
  return true;
}

bool testigoFlash_escribir(const TestigoFlash* t, uint32_t* usBorrado) {
  uint16_t hw[N_HW];
  componer(t, hw);

  // Contador de ciclos del Cortex-M3. Cuenta tambien los ciclos en que el nucleo esta
  // parado esperando a la flash, que es justo lo que se quiere medir.
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  HAL_FLASH_Unlock();
  FLASH_EraseInitTypeDef e;
  e.TypeErase = FLASH_TYPEERASE_PAGES;
  e.Banks = FLASH_BANK_1;
  e.PageAddress = TESTIGO_FLASH_DIR;
  e.NbPages = 1;
  uint32_t errPagina = 0;
  const uint32_t c0 = DWT->CYCCNT;
  bool ok = HAL_FLASHEx_Erase(&e, &errPagina) == HAL_OK;
  const uint32_t ciclos = DWT->CYCCNT - c0;
  if (usBorrado) *usBorrado = ciclos / (SystemCoreClock / 1000000UL);

  for (uint8_t i = 0; ok && i < N_HW; i++) {
    ok = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, TESTIGO_FLASH_DIR + 2UL * i, hw[i]) == HAL_OK;
  }
  HAL_FLASH_Lock();
  if (!ok) return false;

  // El si se da sobre lo RELEIDO, no sobre lo que el HAL dijo (CLAUDE.md 2).
  for (uint8_t i = 0; i < N_HW; i++) {
    if (leerHw(i) != hw[i]) return false;
  }
  return true;
}
