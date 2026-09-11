// ===== Validacion_Automatico/dos_puntas/reloj_real/STM32RTC.h =====
//
// SUSTITUTO MINIMO DE LA LIBRERIA STM32RTC PARA COMPILAR EL reloj.cpp REAL EN EL PC.
//
// Cubre EXACTAMENTE lo que reloj.cpp de las dos puntas llama -censado con grep el 11/09:
// getInstance, setClockSource, begin, isConfigured, getYear/Month/Day/Hours/Minutes/Seconds
// y setMonth- y nada mas: una llamada nueva en reloj.cpp rompe la compilacion aqui, que es
// el aviso que se quiere, en vez de caer en un metodo generico que devuelva cero.
//
// Es el CALENDARIO del RTC de hardware, y desde N-162 (11/09) NADIE LO ESCRIBE: la siembra
// ya no toca el RTC. Por eso arranca SIN CONFIGURAR -como una tarjeta a la que ningun
// firmware viejo le puso la hora- y reloj_setup() real no encuentra hora en el: la hora la
// trae la siembra. Un orquestador que quiera modelar un RTC con hora vieja la escribe por
// punta_dominio_escribir(), igual que la pila la mantendria a traves de un corte.
#pragma once

#include <stdint.h>

extern unsigned long arnes_millis_valor;

// Una copia por DLL, en rtc_periferico.cpp.
extern bool arnes_rtc_configurado;
extern uint32_t arnes_rtc_cal_base;        // segundos del dia cuando millis() == ancla
extern unsigned long arnes_rtc_cal_ancla;
extern uint8_t arnes_rtc_dia;
extern uint8_t arnes_rtc_mes;
extern uint8_t arnes_rtc_anio;

class STM32RTC {
 public:
  enum Source_Clock { LSI_CLOCK, LSE_CLOCK, HSE_CLOCK };
  enum Hour_Format { HOUR_12, HOUR_24 };

  static STM32RTC& getInstance() {
    static STM32RTC unico;
    return unico;
  }

  void setClockSource(Source_Clock) {}
  void begin(bool resetTime = false, Hour_Format = HOUR_24) {
    if (resetTime) arnes_rtc_configurado = false;
  }
  bool isConfigured() { return arnes_rtc_configurado; }

  uint8_t getYear() { return arnes_rtc_anio; }
  uint8_t getMonth() { return arnes_rtc_mes; }
  uint8_t getDay() { return arnes_rtc_dia; }
  uint8_t getHours() { return (uint8_t)(seg() / 3600UL); }
  uint8_t getMinutes() { return (uint8_t)((seg() % 3600UL) / 60UL); }
  uint8_t getSeconds() { return (uint8_t)(seg() % 60UL); }
  void setMonth(uint8_t m) { arnes_rtc_mes = m; }

 private:
  STM32RTC() {}
  static uint32_t seg() {
    return (arnes_rtc_cal_base +
            (uint32_t)((arnes_millis_valor - arnes_rtc_cal_ancla) / 1000UL)) % 86400UL;
  }
};
