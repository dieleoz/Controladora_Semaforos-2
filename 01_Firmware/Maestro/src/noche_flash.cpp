#include "noche_flash.h"
#include <stm32f1xx_hal.h>

// D-108: formato, en medias palabras desde NOCHE_FLASH_DIR (molde testigo_flash.cpp):
//   [0] firma  [1] version  [2] tr
//   [3 + 3i] dias | rojo << 8   [4 + 3i] inicio   [5 + 3i] fin      (i = 0..3)
//   [15..16] suma de Horner de 32 bits, sin plegar, sobre [1..14]
// Una pagina borrada lee 0xFFFF en todo: no casa la firma y no es registro.
static const uint16_t FIRMA   = 0x4E0C;
static const uint16_t VERSION = 1;
static const uint8_t  N_HW    = 17;

static inline uint16_t leerHw(uint8_t i) {
  return *(volatile const uint16_t*)(NOCHE_FLASH_DIR + 2UL * i);
}

static uint32_t suma(const uint16_t* hw) {
  uint32_t s = 0x4E17U;
  for (uint8_t i = 1; i <= 14; i++) s = s * 31U + hw[i];
  return s;
}

static void componer(const NocheConfig* c, uint16_t hw[N_HW]) {
  hw[0] = FIRMA;
  hw[1] = VERSION;
  hw[2] = c->trSeg;
  for (uint8_t i = 0; i < NOCHE_FRANJAS; i++) {
    hw[3 + 3 * i] = (uint16_t)(c->f[i].dias & 0x7FU) | (c->f[i].rojo ? 0x100U : 0U);
    hw[4 + 3 * i] = c->f[i].inicio;
    hw[5 + 3 * i] = c->f[i].fin;
  }
  const uint32_t s = suma(hw);
  hw[15] = (uint16_t)(s & 0xFFFFU);
  hw[16] = (uint16_t)(s >> 16);
}

bool nocheFlash_leer(NocheConfig* c) {
  uint16_t hw[N_HW];
  for (uint8_t i = 0; i < N_HW; i++) hw[i] = leerHw(i);
  if (hw[0] != FIRMA || hw[1] != VERSION) return false;
  const uint32_t s = suma(hw);
  if (hw[15] != (uint16_t)(s & 0xFFFFU) || hw[16] != (uint16_t)(s >> 16)) return false;
  NocheConfig n;
  n.trSeg = hw[2];
  for (uint8_t i = 0; i < NOCHE_FRANJAS; i++) {
    const uint16_t a = hw[3 + 3 * i];
    n.f[i].dias = (uint8_t)(a & 0x7FU);
    n.f[i].rojo = (a & 0x100U) != 0;
    n.f[i].inicio = hw[4 + 3 * i];
    n.f[i].fin = hw[5 + 3 * i];
    // Un minuto fuera del dia no es registro: con la suma buena solo pasaria por un defecto
    // de quien lo escribio, y entrar en la noche con el no tiene arreglo en campo.
    if ((a & ~0x17FU) != 0 || n.f[i].inicio > 1439 || n.f[i].fin > 1439) return false;
  }
  *c = n;
  return true;
}

bool nocheFlash_escribir(const NocheConfig* c) {
  uint16_t hw[N_HW];
  componer(c, hw);
  HAL_FLASH_Unlock();
  FLASH_EraseInitTypeDef e;
  e.TypeErase = FLASH_TYPEERASE_PAGES;
  e.Banks = FLASH_BANK_1;
  e.PageAddress = NOCHE_FLASH_DIR;
  e.NbPages = 1;
  uint32_t errPagina = 0;
  bool ok = HAL_FLASHEx_Erase(&e, &errPagina) == HAL_OK;
  for (uint8_t i = 0; ok && i < N_HW; i++) {
    ok = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, NOCHE_FLASH_DIR + 2UL * i, hw[i]) == HAL_OK;
  }
  HAL_FLASH_Lock();
  if (!ok) return false;
  // El si se da sobre lo RELEIDO, no sobre lo que el HAL dijo (CLAUDE.md 2).
  for (uint8_t i = 0; i < N_HW; i++) {
    if (leerHw(i) != hw[i]) return false;
  }
  return true;
}
