// ===== src/testigo_flash.cpp =====
#include "testigo_flash.h"
#include <stm32f1xx_hal.h>

// Formato: once medias palabras desde TESTIGO_FLASH_DIR.
//   [0] firma  [1] version  [2..3] marcaS  [4..5] inicioS  [6] verde|despeje<<8  [7..8] salidaS (D-52)
//   [9..10] suma de Horner de 32 bits, sin plegar, sobre [1..8] (el porque del no plegar
//          es el de respaldo.cpp, N-51)
// Una pagina borrada lee 0xFFFF en todo: no casa la firma y no es registro.
static const uint16_t FIRMA   = 0x7E57;
static const uint16_t VERSION = 2;   // D-52: una pagina de la version 1 se lee como ausente
static const uint8_t  N_HW    = 11;

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
  hw[6] = (uint16_t)(t->verdeSeg | ((uint16_t)t->despejeSeg << 8));
  hw[7] = (uint16_t)(t->salidaS >> 16);
  hw[8] = (uint16_t)(t->salidaS & 0xFFFFU);
  uint32_t s = 0x1F35U;
  for (uint8_t i = 1; i <= 8; i++) s = s * 31U + hw[i];
  hw[9] = (uint16_t)(s & 0xFFFFU);
  hw[10] = (uint16_t)(s >> 16);
}

bool testigoFlash_leer(TestigoFlash* t) {
  if (leerHw(0) != FIRMA || leerHw(1) != VERSION) return false;
  uint32_t s = 0x1F35U;
  for (uint8_t i = 1; i <= 8; i++) s = s * 31U + leerHw(i);
  if (leerHw(9) != (uint16_t)(s & 0xFFFFU) || leerHw(10) != (uint16_t)(s >> 16)) return false;
  const uint16_t vd = leerHw(6);
  // Un verde o un despeje a cero no es un ciclo: es ausencia de el (criterio de
  // respaldo_guardarCiclo()).
  if ((vd & 0xFF) == 0 || (vd >> 8) == 0) return false;
  t->marcaS = ((uint32_t)leerHw(2) << 16) | leerHw(3);
  t->inicioS = ((uint32_t)leerHw(4) << 16) | leerHw(5);
  t->verdeSeg = (uint8_t)(vd & 0xFF);
  t->despejeSeg = (uint8_t)(vd >> 8);
  t->salidaS = ((uint32_t)leerHw(7) << 16) | leerHw(8);
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
