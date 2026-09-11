// ===== Validacion_Automatico/dos_puntas/reloj_real/rtc_periferico.h =====
//
// LO QUE LOS DOS ADAPTADORES DEL DEGRADADO NECESITAN PARA MOVER EL reloj.cpp REAL: el HSI de
// cada punta, la linea del ESP32 y la siembra que conserva la fase. UN SOLO FICHERO para las
// dos puntas -compilado una vez en cada DLL-, para que las dos cuenten y siembren con la
// MISMA aritmetica y una diferencia entre ellas sea del firmware y no del arnes.
//
// Nada de esto decide una luz ni una hora: la hora la acepta o la rechaza reloj.cpp real, y
// la luz la escribe semaforo.cpp real.
#pragma once

#include <stddef.h>
#include <stdint.h>

// --- EL HSI DE ESTA PUNTA -----------------------------------------------------------
//
// El orquestador pasa a punta_tick() el reloj del BANCO. Cada punta lo convierte en SU
// millis(): identico mientras nadie fije una deriva, y (1 + ppm/1e6) veces el del banco
// desde que se fija. Es la deriva que D-26 le atribuye al HSI -la del oscilador del micro, no
// la del reloj de pared-, y por eso afecta a TODO lo que cuenta millis() en esa punta: la hora
// extrapolada y tambien sus temporizadores. Se reancla en el instante de fijarla, asi que
// millis() nunca retrocede.
unsigned long arnes_reloj_local(unsigned long banco);
void arnes_hsi_ppm(long ppm);

// --- LA LINEA DEL ESP32 ---------------------------------------------------------------
// La forma exacta de FORMATO_HORA_ESP32 (ESP32_Expansion/src/siembra.cpp):
// "YYYY-MM-DD,HH:MM:SS". Anio y mes los descarta el STM32; van fijos.
void arnes_iso(char* buf, size_t n, uint8_t dia, long segDelDia);

// Milisegundos desde la ultima frontera de segundo de reloj_segundosDelDia() de esta punta,
// medidos PROBANDO la funcion real, sin mirar sus estaticas. -1 sin hora.
long arnes_fase_subsegundo();

// LA SIEMBRA QUE CONSERVA LA FASE. Compone la hora ACTUAL de esta punta mas deltaS, y se la
// entrega a 'sembrar' con millis() puesto en la ULTIMA FRONTERA de segundo -despues lo
// restaura-. Asi el instante en que la hora cambia de segundo no se mueve: el arnes no
// fabrica un residuo sub-segundo propio encima del real (el hallazgo de los 950 ms que
// cuenta la cabecera de orquestador_degradado.cpp). Devuelve lo que devolvio 'sembrar', o
// -1 si esta punta no tenia hora que desplazar.
int arnes_sembrar_en_frontera(long deltaS, int (*sembrar)(const char* iso));

// --- EL DOMINIO DE RESPALDO DEL RTC (indices 10..13 de punta_api.h) -------------------
long arnes_dominio_leer_rtc(int indice);
void arnes_dominio_escribir_rtc(int indice, long valor);
