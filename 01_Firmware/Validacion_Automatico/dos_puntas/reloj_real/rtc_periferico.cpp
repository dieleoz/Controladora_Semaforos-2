// ===== Validacion_Automatico/dos_puntas/reloj_real/rtc_periferico.cpp =====
//
// El periferico del RTC y el HSI de UNA punta. Se compila en las DOS DLL del arnes del
// Degradado -una copia de las estaticas en cada una, que es el mecanismo del arnes-. Ver
// rtc_periferico.h, stm32f1xx_hal.h y STM32RTC.h de este mismo directorio.

#include "rtc_periferico.h"

#include <stdio.h>

#include "Arduino.h"
#include "reloj.h"           // el de la punta que se compila: -I de su include REAL
#include "stm32f1xx_hal.h"

// --- El silicio ---------------------------------------------------------------------
bool arnes_lse_listo = true;             // ver el borde en stm32f1xx_hal.h
uint32_t arnes_rtc_cnt_base = 1;
unsigned long arnes_rtc_cnt_ancla = 0;
bool arnes_rtc_congelado = false;        // 1.22: el tercer estado, ver stm32f1xx_hal.h
uint32_t arnes_rtc_cnt_congelado = 0;
ArnesRccRegs arnes_rcc;
ArnesRtcRegs arnes_rtc_regs;

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

// --- 1.22: la perilla de congelacion. Ver rtc_periferico.h -------------------------------
void arnes_rtc_congelar(bool congelado) {
  if (congelado == arnes_rtc_congelado) return;
  if (congelado) {
    // Se queda en lo que vale AHORA. Congelarlo en otro numero seria un salto, y un salto
    // lo caza otra barrera ("ahora < guardado" de respaldo_horasDesdeSync): el escenario
    // mediria esa y no la parada.
    arnes_rtc_cnt_congelado = arnes_rtc_cnt();
  } else {
    // Y al soltarlo sigue desde donde se quedo, sin recuperar el tiempo perdido: eso es lo
    // que hace un cristal que vuelve a oscilar.
    arnes_rtc_cnt_base = arnes_rtc_cnt_congelado;
    arnes_rtc_cnt_ancla = arnes_millis_valor;
  }
  arnes_rtc_congelado = congelado;
}

bool arnes_rtc_esta_congelado() { return arnes_rtc_congelado; }

// --- El dominio de respaldo del RTC -------------------------------------------------------
// Solo el contador (indice 10). N-172: el calendario de la libreria vive en DR6/DR7 (STM32RTC.h)
// y no tiene registros propios; los indices 11..13 quedan sin uso.
long arnes_dominio_leer_rtc(int indice) {
  return indice == 10 ? (long)arnes_rtc_cnt() : 0;
}

void arnes_dominio_escribir_rtc(int indice, long valor) {
  if (indice != 10) return;
  arnes_rtc_cnt_base = (uint32_t)valor;
  arnes_rtc_cnt_ancla = arnes_millis_valor;
  // 1.22: la reposicion tras un microcorte tiene que llegar tambien al valor congelado,
  // o el corte DESCONGELARIA el cristal por la puerta de atras: la pila mantiene el
  // contador, no repara el oscilador.
  if (arnes_rtc_congelado) arnes_rtc_cnt_congelado = (uint32_t)valor;
}
