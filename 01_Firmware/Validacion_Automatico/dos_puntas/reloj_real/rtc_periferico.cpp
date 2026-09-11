// ===== Validacion_Automatico/dos_puntas/reloj_real/rtc_periferico.cpp =====
//
// El periferico del RTC y el HSI de UNA punta. Se compila en las DOS DLL del arnes del
// Degradado -una copia de las estaticas en cada una, que es el mecanismo del arnes-. Ver
// rtc_periferico.h, stm32f1xx_hal.h y STM32RTC.h de este mismo directorio.

#include "rtc_periferico.h"

#include <stdio.h>

#include "Arduino.h"
#include "reloj.h"           // el de la punta que se compila: -I de su include REAL
#include "STM32RTC.h"
#include "stm32f1xx_hal.h"

// --- El silicio ---------------------------------------------------------------------
bool arnes_lse_listo = true;             // ver el borde en stm32f1xx_hal.h
uint32_t arnes_rtc_cnt_base = 1;
unsigned long arnes_rtc_cnt_ancla = 0;
ArnesRccRegs arnes_rcc;
ArnesRtcRegs arnes_rtc_regs;

bool arnes_rtc_configurado = false;      // ver STM32RTC.h: desde N-162 nadie lo escribe
uint32_t arnes_rtc_cal_base = 0;
unsigned long arnes_rtc_cal_ancla = 0;
uint8_t arnes_rtc_dia = 1;
uint8_t arnes_rtc_mes = 1;
uint8_t arnes_rtc_anio = 0;

// --- El HSI -----------------------------------------------------------------------------
static long g_ppm = 0;
static bool g_derivando = false;
static unsigned long g_anclaBanco = 0;
static unsigned long g_anclaLocal = 0;
static unsigned long g_ultimoBanco = 0;

unsigned long arnes_reloj_local(unsigned long banco) {
  g_ultimoBanco = banco;
  if (!g_derivando) return banco;
  const long long transcurrido = (long long)(unsigned long)(banco - g_anclaBanco);
  return g_anclaLocal +
         (unsigned long)(transcurrido * (1000000LL + (long long)g_ppm) / 1000000LL);
}

void arnes_hsi_ppm(long ppm) {
  g_anclaBanco = g_ultimoBanco;
  g_anclaLocal = arnes_millis_valor;
  g_ppm = ppm;
  g_derivando = true;
}

// --- La linea del ESP32 -----------------------------------------------------------------
void arnes_iso(char* buf, size_t n, uint8_t dia, long segDelDia) {
  long s = segDelDia % 86400L;
  if (s < 0) s += 86400L;
  snprintf(buf, n, "2026-01-%02u,%02ld:%02ld:%02ld", (unsigned)dia, s / 3600L, (s / 60L) % 60L,
           s % 60L);
}

// La ultima frontera de segundo de la hora de esta punta, PROBANDO reloj_segundosDelDia()
// real hacia atras milisegundo a milisegundo. Devuelve el millis() de esa frontera.
static unsigned long ultimaFrontera(unsigned long ahora, uint32_t s0) {
  unsigned long frontera = ahora;
  for (unsigned long k = 1; k <= 1000UL; k++) {
    arnes_millis_valor = ahora - k;
    if (reloj_segundosDelDia() != s0) {
      frontera = ahora - k + 1UL;
      break;
    }
  }
  arnes_millis_valor = ahora;
  return frontera;
}

long arnes_fase_subsegundo() {
  if (!reloj_enHora()) return -1;
  const unsigned long ahora = arnes_millis_valor;
  return (long)(ahora - ultimaFrontera(ahora, reloj_segundosDelDia()));
}

int arnes_sembrar_en_frontera(long deltaS, int (*sembrar)(const char* iso)) {
  if (!reloj_enHora()) return -1;
  const unsigned long ahora = arnes_millis_valor;
  const uint32_t s0 = reloj_segundosDelDia();
  long s = (long)s0 + deltaS;
  int dia = (int)reloj_dia();
  while (s < 0) { s += 86400L; dia -= 1; }
  while (s >= 86400L) { s -= 86400L; dia += 1; }
  while (dia < 1) dia += 31;
  while (dia > 31) dia -= 31;

  char iso[24];
  arnes_iso(iso, sizeof(iso), (uint8_t)dia, s);

  arnes_millis_valor = ultimaFrontera(ahora, s0);
  const int r = sembrar(iso);
  arnes_millis_valor = ahora;
  return r;
}

// --- El dominio de respaldo del RTC -------------------------------------------------------
long arnes_dominio_leer_rtc(int indice) {
  switch (indice) {
    case 10: return (long)arnes_rtc_cnt();
    case 11: return arnes_rtc_configurado ? 1 : 0;
    case 12: return (long)((arnes_rtc_cal_base +
                            (uint32_t)((arnes_millis_valor - arnes_rtc_cal_ancla) / 1000UL)) %
                           86400UL);
    case 13: return (long)arnes_rtc_dia;
    default: return 0;
  }
}

void arnes_dominio_escribir_rtc(int indice, long valor) {
  switch (indice) {
    case 10: arnes_rtc_cnt_base = (uint32_t)valor; arnes_rtc_cnt_ancla = arnes_millis_valor; break;
    case 11:
      arnes_rtc_configurado = (valor != 0);
      if (arnes_rtc_configurado) arnes_rtc_anio = 26;   // ANIO_MARCA de reloj.cpp
      break;
    case 12: arnes_rtc_cal_base = (uint32_t)valor; arnes_rtc_cal_ancla = arnes_millis_valor; break;
    case 13: arnes_rtc_dia = (uint8_t)valor; break;
    default: break;
  }
}
