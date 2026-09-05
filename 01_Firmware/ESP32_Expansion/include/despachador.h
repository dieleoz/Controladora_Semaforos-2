// ===== 01_Firmware/ESP32_Expansion/include/despachador.h =====
//
// LAS RAMAS QUE EL PUENTE ATIENDE POR SI MISMO: SU PROPIO RELOJ.
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

// ---------------------------------------------------------------------------------
// LA LINEA QUE NO SIGUE VIAJE, Y POR QUE ESTE FICHERO DECIA ANTES QUE ESO NO EXISTIA
//
// Aqui ponia, sobre despachador_observar(): "No devuelve nada porque NO decide sobre el
// reenvio: una rama que pudiera vetar el paso de una trama seria el puente conociendo
// comandos, y ese es justo el diseno que obligaria a recompilarlo cada vez que el
// protocolo crece."
//
// LA MITAD QUE SIGUE SIENDO CIERTA: el puente no puede conocer el protocolo DEL STM32.
// Un filtro por lista de comandos del equipo obligaria a recompilar este modulo cada
// vez que aquel creciera, y el dia que alguien lo olvidara la funcion nueva se caeria
// en silencio. Eso no cambia, y SET_RTC lo demuestra: se atiende AQUI y ademas SIGUE
// VIAJE, porque es una orden que el STM32 tambien ha conocido siempre.
//
// LA MITAD QUE ERA FALSA, Y ESTA MEDIDA -no razonada- CON EL bluetooth.cpp REAL DE LAS
// DOS PUNTAS COMPILADO (Simulaciones/puente_esp32/build/arnes_*.exe, 05/09):
//
//   entrada: CMD:LEER_RTC
//   MAESTRO -> $ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO*5C
//   ESCLAVO -> $ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO*5C
//
//   entrada: CMD:PIN:1234:LEER_RTC
//   MAESTRO -> $ERR,CMD:DESCONOCIDO,DESC:COMANDO_NO_SOPORTADO*4B
//   ESCLAVO -> $ERR,CMD:DESCONOCIDO,DESC:COMANDO_NO_SOPORTADO_EN_ESCLAVO*01
//
// La app pinta esas dos en ROJO, como "Rechazo de Firmware" (app.js, rama '$ERR' de
// juzgarTrama -> addEvent('red', cabecera ...)). O sea que reenviar una consulta de
// reloj le acusa al operario de teclear mal una clave que no ha tecleado, CADA VEZ QUE
// PREGUNTA LA HORA. Es exactamente el defecto por el que el Maestro tiene su rama muda
// de "$LATIDO" -medido el 04/09- y el defecto que D-15 acaba de cerrar en SET_RTC.
//
// 🔴 Y EL CRITERIO NO ES UNA LISTA CON UN MOTIVO ESCRITO, QUE ES LO QUE COSTO N-122.
// Una linea se queda aqui SOLO SI NINGUNA PUNTA TIENE UN LITERAL PARA ELLA. Eso no es
// una frase: es una afirmacion sobre el codigo, y hay un pack que la recalcula leyendo
// los dos despachadores del STM32 en cada corrida. El dia que alguien le escriba a
// LEER_RTC una rama en Maestro/src/bluetooth.cpp, la compuerta se pone roja y esta
// decision se vuelve a tomar con el dato delante, en vez de envejecer en un comentario.
//
// SE PREGUNTA CON UN PREDICADO APARTE, no con el valor de retorno de observar(), porque
// el reenvio ocurre ANTES: puente.cpp tiene que saber si escribe en el cable antes de
// haber llamado a nadie. El predicado no emite, no toca el reloj y no cuenta nada.
// ---------------------------------------------------------------------------------
bool despachador_esParaElPuente(const char* linea);

// Atiende lo que le toque de una linea que ya se valido.
//
// `propagada` dice si esa misma linea llego entera al STM32: es lo unico que distingue
// "la hora entro aqui y va camino del equipo" de "la hora entro aqui y el equipo no se
// entero", y el operario necesita los dos distintos. Para las lineas que
// despachador_esParaElPuente() reclama NO significa nada -no hay cable que cruzar- y su
// rama no lo mira.
void despachador_observar(const char* linea, bool propagada);

#endif // DESPACHADOR_H
