// ===== src/reloj.cpp (ESCLAVO) =====
#include "reloj.h"
#include <STM32RTC.h>
#include <stm32f1xx_hal.h>   // N-17: arranque acotado del cristal, ver abajo

// ---------------------------------------------------------------------------
// SFTY-18 / SFTY-23 — Reloj de tiempo real sobre el RTC interno del STM32.
//
// D-20: LA AUTORIDAD DE LA HORA ES EL ESP32 (DS3231).
// El STM32 no tiene cristal de 32.768 kHz garantizado (Y2 muerto, N-17).
// Por tanto, reloj_ajustar() siembra tanto el RTC hardware (si está operativo)
// como una base de software extrapolada con millis(), asegurando que reloj_enHora()
// sea true y la hora avance fiablemente una vez sembrada desde la radio o ESP32.
// ---------------------------------------------------------------------------

static STM32RTC &rtc = STM32RTC::getInstance();

// Como sabemos si la hora es de fiar:
// al ajustarla escribimos tambien el ano ANIO_MARCA. Un RTC que nunca se puso en
// hora, o que perdio la pila, arranca con el ano en 0 o 1. Si al encender leemos
// un ano anterior a ANIO_MARCA, la hora NO es de fiar y hay que decirlo, no
// suponerlo: un reloj sin poner en hora es peor que no tener reloj.
static const uint8_t ANIO_MARCA = 26;  // 2026, ano de puesta en servicio

static bool horaValida = false;
static bool rtcOperativo = false;

// D-20: Base de tiempo de software extrapolada por millis()
static uint32_t tBaseMillis = 0;
static uint32_t segBaseDelDia = 0;
static uint8_t diaBase = 1;

// Espera acotada a que arranque el oscilador del cristal Y2.
static const uint32_t ESPERA_LSE_MS = 2000;

static bool arrancarCristal() {
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_RCC_BKP_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess();

  if (__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY) != RESET) return true;

  __HAL_RCC_LSE_CONFIG(RCC_LSE_ON);

  const uint32_t t0 = HAL_GetTick();
  while (__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY) == RESET) {
    if (HAL_GetTick() - t0 > ESPERA_LSE_MS) {
      return false;
    }
  }
  return true;
}

void reloj_setup() {
  horaValida = false;
  rtcOperativo = false;
  tBaseMillis = 0;
  segBaseDelDia = 0;
  diaBase = 1;

  if (!arrancarCristal()) return;

  rtc.setClockSource(STM32RTC::LSE_CLOCK);  // cristal Y2 de 32.768 kHz
  rtc.begin(false, STM32RTC::HOUR_24);      // false = NO borrar la hora guardada
  rtcOperativo = true;                      // N-24: a partir de aqui el RTC cuenta

  horaValida = rtc.isConfigured() && (rtc.getYear() >= ANIO_MARCA);
}

static const unsigned long REINTENTO_LSE_MS = 30000;
static uint32_t tUltimoReintento = 0;

void reloj_actualizar() {
  if (rtcOperativo) return;

  const uint32_t ahora = HAL_GetTick();
  if (ahora - tUltimoReintento < REINTENTO_LSE_MS) return;
  tUltimoReintento = ahora;

  if (__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY) == RESET) return;

  rtc.setClockSource(STM32RTC::LSE_CLOCK);
  rtc.begin(false, STM32RTC::HOUR_24);
  rtcOperativo = true;

  if (rtc.isConfigured() && (rtc.getYear() >= ANIO_MARCA)) {
    horaValida = true;
  }
}

bool reloj_enHora() { return horaValida; }

// N-49 — el contador crudo del RTC, en segundos.
uint32_t reloj_contadorSegundos() {
  if (rtcOperativo) {
    uint16_t alta = (uint16_t)RTC->CNTH;
    const uint16_t baja = (uint16_t)RTC->CNTL;
    if ((uint16_t)RTC->CNTH != alta) alta = (uint16_t)RTC->CNTH;
    const uint32_t v = ((uint32_t)alta << 16) | baja;
    return v == 0 ? 1UL : v;
  }
  // D-20: extrapolacion para contador continuo de segundos cuando rtcOperativo es false
  if (!horaValida) return 0;
  const uint32_t v = (uint32_t)((millis() - tBaseMillis) / 1000UL) + 1UL;
  return v == 0 ? 1UL : v;
}

uint32_t reloj_segundosDelDia() {
  if (!horaValida) return 0;
  if (rtcOperativo) {
    return (uint32_t)rtc.getHours() * 3600UL + (uint32_t)rtc.getMinutes() * 60UL +
           (uint32_t)rtc.getSeconds();
  }
  // D-20: extrapolacion por software usando millis()
  const uint32_t deltaS = (millis() - tBaseMillis) / 1000UL;
  return (segBaseDelDia + deltaS) % 86400UL;
}

uint8_t reloj_hora() {
  if (!horaValida) return 0;
  if (rtcOperativo) return rtc.getHours();
  return (uint8_t)(reloj_segundosDelDia() / 3600UL);
}

uint8_t reloj_minuto() {
  if (!horaValida) return 0;
  if (rtcOperativo) return rtc.getMinutes();
  return (uint8_t)((reloj_segundosDelDia() % 3600UL) / 60UL);
}

uint8_t reloj_segundo() {
  if (!horaValida) return 0;
  if (rtcOperativo) return rtc.getSeconds();
  return (uint8_t)(reloj_segundosDelDia() % 60UL);
}

uint8_t reloj_dia() {
  if (!horaValida) return 0;
  if (rtcOperativo) return rtc.getDay();
  const uint32_t deltaDias = (segBaseDelDia + ((millis() - tBaseMillis) / 1000UL)) / 86400UL;
  uint32_t d = (uint32_t)diaBase + deltaDias;
  while (d > 31) d -= 31;
  return (uint8_t)(d == 0 ? 1 : d);
}

void reloj_ajustar(uint8_t hora, uint8_t minuto, uint8_t segundo, uint8_t dia) {
  if (hora > 23 || minuto > 59 || segundo > 59 || dia > 31) return;

  // D-20: Siembra de la base de software (independiente de si Y2 oscila)
  segBaseDelDia = (uint32_t)hora * 3600UL + (uint32_t)minuto * 60UL + (uint32_t)segundo;
  tBaseMillis = millis();
  if (dia >= 1) {
    diaBase = dia;
  } else if (diaBase < 1 || diaBase > 31) {
    diaBase = 1;
  }
  horaValida = true;

  // Si el oscilador hardware esta operativo, tambien mantenemos sincronizado el RTC
  if (rtcOperativo) {
    rtc.setHours(hora);
    rtc.setMinutes(minuto);
    rtc.setSeconds(segundo);
    if (rtc.getYear() < ANIO_MARCA) rtc.setYear(ANIO_MARCA);

    if (dia >= 1) {
      rtc.setDay(dia);
      rtc.setMonth(1);
    } else if (rtc.getDay() < 1 || rtc.getDay() > 31) {
      rtc.setDay(1);
      rtc.setMonth(1);
    }
  }
}

