// ===== 01_Firmware/ESP32_Expansion/include/siembra.h =====
//
// D-20 / D-26 (11/09): EL ESP32 LE DA LA HORA A SU STM32.
//
// La unica linea de ORDEN que este modulo origina hacia el micro que gobierna el cruce:
//
//     CMD:HORA_ESP32:YYYY-MM-DD,HH:MM:SS
//
// Por que eso no rompe "el puente no origina" -esp32_05-, con la excepcion medida alli:
// la hora sale SOLO de reloj_leer(), que es la barrera del DS3231 con su releida, y de
// nada mas. Ni un byte viene del telefono: el SET_RTC del telefono ya no cruza (lo
// atiende despachador.cpp), y cualquier linea del telefono que contenga HORA_ESP32 se
// DESCARTA alli mismo, porque sin eso cualquiera con Bluetooth dictaria la hora del
// cruce escribiendo esta linea a mano.
//
// CUANDO SALE -las tres, y ninguna mas-:
//   1. al arrancar, en cuanto el DS3231 da hora fiable, con los dos reintentos de
//      contrato.h (el STM32 abre J17 mas tarde que nosotros);
//   2. justo despues de un SET_RTC del telefono que termino en RELOJ_OK con releida OK;
//   3. cada SIEMBRA_INTERVALO_MS (D-26 (2): ~2 min; ~~~5 min~~; ~~una hora, A-15~~). El
//      numero no se recita aqui: de esa constante cuelga el plazo de caducidad de D-21 (1),
//      y el porque de bajarla esta con ella en contrato.h.
//
// LO QUE NO SABE, Y NO PUEDE: si el STM32 la acepto. `true` significa que la linea se
// puso ENTERA en el cable, no que alguien la leyera. Esperar un acuse obligaria a parar
// el bombeo, y un puente que espera deja de pasar telemetria.

#ifndef SIEMBRA_H
#define SIEMBRA_H

#include <Arduino.h>

// Una vuelta: decide si toca sembrar (arranque, reintento o cadencia) y, si toca, siembra.
// Va en loop(), NUNCA en setup(): en setup() no se pone un byte en ningun cable (6.4).
void siembra_revisar();

// Siembra YA. Devuelve true solo si la linea salio entera por J17; false si el DS3231 no
// dio hora por su barrera (y entonces no sale nada) o si el enlace no la acepto.
bool siembra_ahora();

#endif // SIEMBRA_H
