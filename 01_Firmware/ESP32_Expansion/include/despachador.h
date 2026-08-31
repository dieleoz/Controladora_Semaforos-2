// ===== 01_Firmware/ESP32_Expansion/include/despachador.h =====
//
// LA UNICA RAMA QUE EL PUENTE ATIENDE POR SI MISMO: SU PROPIO RELOJ.
//
// POR QUE ESTO NO CONTRADICE "EL PUENTE NO ORIGINA".
//
// B-1 dice que cada byte escrito HACIA EL STM32 procede del buffer de entrada, y eso
// se sigue cumpliendo al pie de la letra: esta rama pone en hora el DS3231 del ESP32 y
// DESPUES la misma linea, sin tocar, sigue su camino hacia el STM32, que atendera su
// propio SET_RTC y contestara por su cuenta.
//
// Lo que esta rama origina va SOLO hacia la app y va MARCADO (B-4): NODE:PUENTE. Un
// $ERR del puente que pareciera del STM32 manda a diagnosticar el poste equivocado.
//
// 🔴 Y EL PIN NO SE TOCA. La rama busca "SET_RTC:" con strstr -en cualquier posicion de
// la linea-, no "CMD:PIN:1234:SET_RTC:". El puente TRANSPORTA el PIN: no lo mejora, no
// lo sustituye, no lo almacena y no lo compara. Un puente que validara el PIN seria una
// segunda copia del contrato que alguien tendria que sincronizar, y el dia que
// difirieran un comando funcionaria por una puerta y seria rechazado por la otra.
// Hay un pack que exige que la cadena "1234" no aparezca en ningun fuente de aqui.

#ifndef DESPACHADOR_H
#define DESPACHADOR_H

#include <Arduino.h>

// Atiende lo que le toque de una linea que ya se valido y que YA VA a reenviarse.
//
// `propagada` dice si esa misma linea llego entera al STM32: es lo unico que distingue
// "la hora entro aqui y va camino del equipo" de "la hora entro aqui y el equipo no se
// entero", y el operario necesita los dos distintos.
//
// No devuelve nada porque NO decide sobre el reenvio: una rama que pudiera vetar el
// paso de una trama seria el puente conociendo comandos, y ese es justo el diseno que
// obligaria a recompilarlo cada vez que el protocolo crece.
void despachador_observar(const char* linea, bool propagada);

#endif // DESPACHADOR_H
