// ===== Validacion_Automatico/dos_puntas/reloj_real/stm32f1xx_hal.h =====
//
// EL HAL QUE HACE FALTA PARA COMPILAR EL reloj.cpp REAL DE LAS DOS PUNTAS EN EL PC.
//
// POR QUE EXISTE (11/09, D-21 (1)). Hasta hoy ningun arnes enlazaba reloj.cpp: incluye
// <STM32RTC.h> y <stm32f1xx_hal.h>, y los adaptadores de dos_puntas lo SUSTITUIAN por un
// modelo escrito a mano. Con eso reloj_radioManda() -la frontera de 25 s de D-26 (3)- y la
// caducidad de la siembra de D-21 (1) no los ejecutaba nadie: se leian por regex. Este
// directorio va DELANTE de comun/ en compilar_degradado.ps1, y SOLO alli: los demas arneses
// siguen con sus sustitutos y no ven nada de esto.
//
// NO ES UNA COPIA DEL HAL DE comun/: lo INCLUYE -los diez registros de respaldo que
// respaldo.cpp recorre siguen siendo esos- y le anade lo que reloj.cpp toca y respaldo.cpp
// no: el flag del LSE, HAL_GetTick(), RCC->BDCR y el contador RTC->CNTH/CNTL.
//
// SE MODELA EL PERIFERICO, NUNCA UNA REGLA. Todo lo que hay aqui es el silicio: si el
// oscilador esta listo, cuanto vale el contador, que bits tiene BDCR. Lo que el firmware
// decide con eso lo decide reloj.cpp, compilado letra por letra.
//
// EL BORDE, ESCRITO (CLAUDE.md 7): el cristal Y2 se modela ARRANCANDO (LSERDY a 1). Es lo
// que ya suponian los modelos de los adaptadores -el contador del RTC avanzaba- y es lo que
// permite que respaldo.cpp feche. Y2 MUERTO (N-17, medido en una tarjeta) NO se ejerce en
// este arnes: si alguien pone arnes_lse_listo a false, cada lectura del flag gasta 1 ms del
// reloj simulado para que la espera acotada de arrancarCristal() termine como en la
// tarjeta, pero ningun bloque lo usa hoy.
#pragma once

#include "../comun/stm32f1xx_hal.h"

#include <stdint.h>

extern unsigned long arnes_millis_valor;

// --- El estado del periferico. Una copia por DLL, definida en rtc_periferico.cpp ------
extern bool arnes_lse_listo;
extern uint32_t arnes_rtc_cnt_base;        // lo que valia CNT cuando millis() == ancla
extern unsigned long arnes_rtc_cnt_ancla;

inline uint32_t arnes_rtc_cnt() {
  return arnes_rtc_cnt_base + (uint32_t)((arnes_millis_valor - arnes_rtc_cnt_ancla) / 1000UL);
}

#ifndef RESET
#define RESET 0
#endif
#ifndef SET
#define SET 1
#endif

#define RCC_FLAG_LSERDY 0x41U
#define RCC_LSE_ON      1U

inline int arnes_rcc_flag(uint32_t flag) {
  if (flag != RCC_FLAG_LSERDY) return RESET;
  if (arnes_lse_listo) return SET;
  arnes_millis_valor += 1;   // ver la cabecera: la espera acotada tiene que poder acabar
  return RESET;
}

#define __HAL_RCC_GET_FLAG(f)          (arnes_rcc_flag(f))
#define __HAL_RCC_LSE_CONFIG(x)        ((void)(x))
#define __HAL_RCC_BACKUPRESET_FORCE()  ((void)0)
#define __HAL_RCC_BACKUPRESET_RELEASE() ((void)0)

inline uint32_t HAL_GetTick(void) { return (uint32_t)arnes_millis_valor; }

// --- RCC->BDCR: solo lo lee reloj_diagnostico() del Maestro. Bits de la ficha (RM0008) --
#define RCC_BDCR_LSEON      (1UL << 0)
#define RCC_BDCR_LSERDY     (1UL << 1)
#define RCC_BDCR_LSEBYP     (1UL << 2)
#define RCC_BDCR_RTCSEL_Pos 8U
#define RCC_BDCR_RTCSEL     (3UL << RCC_BDCR_RTCSEL_Pos)
#define RCC_BDCR_RTCEN      (1UL << 15)

struct ArnesRcc {
  uint32_t leerBdcr() const {
    return arnes_lse_listo
               ? (RCC_BDCR_LSEON | RCC_BDCR_LSERDY | (1UL << RCC_BDCR_RTCSEL_Pos) | RCC_BDCR_RTCEN)
               : RCC_BDCR_LSEON;
  }
};
struct ArnesRegBdcr {
  const ArnesRcc* rcc;
  operator uint32_t() const { return rcc->leerBdcr(); }
};
struct ArnesRccRegs {
  ArnesRcc r;
  ArnesRegBdcr BDCR{&r};
};
extern ArnesRccRegs arnes_rcc;
#define RCC (&arnes_rcc)

// --- RTC->CNTH / CNTL: el contador de segundos, leido en dos mitades como en el silicio --
struct ArnesRegCnt {
  bool alta;
  operator uint32_t() const {
    const uint32_t v = arnes_rtc_cnt();
    return alta ? (v >> 16) : (v & 0xFFFFUL);
  }
};
struct ArnesRtcRegs {
  ArnesRegCnt CNTH{true};
  ArnesRegCnt CNTL{false};
};
extern ArnesRtcRegs arnes_rtc_regs;
#define RTC (&arnes_rtc_regs)
