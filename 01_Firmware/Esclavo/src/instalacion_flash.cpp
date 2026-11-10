// ===== src/instalacion_flash.cpp =====
// N-187 (SPEC_8 8.6). IDENTICO EN LAS DOS PUNTAS: ver instalacion_flash.h.
#include "instalacion_flash.h"
#include <stm32f1xx_hal.h>

uint16_t instalacionFlash_leerHw(uint8_t i) {
  if (i >= INSTALACION_FLASH_HW) return 0xFFFFU;
  return *(volatile const uint16_t*)(INSTALACION_FLASH_DIR + 2UL * i);
}

bool instalacionFlash_escribir(const uint16_t hw[INSTALACION_FLASH_HW]) {
  HAL_FLASH_Unlock();
  FLASH_EraseInitTypeDef e;
  e.TypeErase = FLASH_TYPEERASE_PAGES;
  e.Banks = FLASH_BANK_1;
  e.PageAddress = INSTALACION_FLASH_DIR;
  e.NbPages = 1;
  uint32_t errPagina = 0;
  bool ok = HAL_FLASHEx_Erase(&e, &errPagina) == HAL_OK;
  for (uint8_t i = 0; ok && i < INSTALACION_FLASH_HW; i++) {
    ok = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, INSTALACION_FLASH_DIR + 2UL * i, hw[i]) == HAL_OK;
  }
  HAL_FLASH_Lock();
  if (!ok) return false;
  // El si se da sobre lo RELEIDO, no sobre lo que el HAL dijo (CLAUDE.md 2).
  for (uint8_t i = 0; i < INSTALACION_FLASH_HW; i++) {
    if (instalacionFlash_leerHw(i) != hw[i]) return false;
  }
  return true;
}
