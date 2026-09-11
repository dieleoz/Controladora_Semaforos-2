// ===== 01_Firmware/ESP32_Expansion/include/despachador.h =====
//
// LAS RAMAS QUE EL PUENTE ATIENDE POR SI MISMO: SU PROPIO RELOJ.
//
// ~~POR QUE ESTO NO CONTRADICE "EL PUENTE NO ORIGINA": esta rama pone en hora el DS3231
// y DESPUES la misma linea, sin tocar, sigue su camino hacia el STM32~~ - CADUCADO EL
// 11/09 (D-20/A-15). Se deja tachado y no se borra: una razon que se cae se marca.
//
// DESDE D-20 EL SET_RTC DEL TELEFONO SE QUEDA AQUI Y NO CRUZA. Lo que llega al STM32 es
// la hora RELEIDA del DS3231, en la linea CMD:HORA_ESP32 que compone siembra.cpp -y solo
// si el DS3231 la acepto-. Eso SI es originar hacia el STM32, y es la excepcion que
// esp32_05 tiene escrita y medida: la hora sale de reloj_leer() y de nada mas.
//
// Lo que esta rama emite por si misma sigue yendo SOLO hacia la app y MARCADO (B-4):
// NODE:PUENTE. Un $ERR del puente que pareciera del STM32 manda a diagnosticar el poste
// equivocado.
//
// 🔴 EL PIN NO SE TOCA... Y DESDE D-20 TAMPOCO LO MIRA NADIE EN SET_RTC. La rama busca
// "SET_RTC:" con strstr -en cualquier posicion de la linea-, porque el puente no conoce
// el PIN: esp32_09 exige que no aparezca en ningun fuente de aqui, y el motivo sigue en
// pie -una segunda copia del contrato de autenticacion que alguien tendria que
// sincronizar-. Antes lo comprobaba la guarda de PIN del STM32 cuando la linea le
// llegaba; ahora no le llega. Consecuencia: la hora del controlador se puede poner sin
// PIN. No se esconde, y el responsable lo ACEPTO el 11/09 (D-26 (1)): esp32_13 lo deja
// como nota que cita esa fila.

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
// en silencio. Eso no cambia. ~~SET_RTC lo demuestra: se atiende AQUI y ademas SIGUE
// VIAJE~~ - desde D-20 (11/09) SET_RTC es del puente y de nadie mas, como LEER_RTC: el
// criterio de abajo se le aplica igual, y esp32_12 exige que el STM32 no le tenga rama.
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
// SE PREGUNTA CON UN PREDICADO APARTE, porque el reenvio ocurre ANTES: puente.cpp tiene
// que saber si escribe en el cable antes de haber llamado a nadie. El predicado no
// emite, no toca el reloj y no cuenta nada.
//
// DESDE D-20 (11/09) RECLAMA TRES COSAS, y cada una tiene su rama en
// despachador_atender() con el MISMO criterio:
//   CMD:LEER_RTC      entera (strcmp): la consulta, que no lleva PIN.
//   "SET_RTC:" dentro la puesta en hora: el PIN va delante y el puente no lo conoce.
//   "HORA_ESP32" dentro la linea que solo origina el puente (siembra.cpp): del telefono
//                     se DESCARTA, o cualquiera con Bluetooth dictaria la hora sin PIN.
// ---------------------------------------------------------------------------------
bool despachador_esParaElPuente(const char* linea);

// Atiende una linea que despachador_esParaElPuente() reclamo -y SOLO esas: puente.cpp
// no la llama para lo que cruza-. Contesta siempre, y siempre hacia la app.
//
// ~~`propagada` dice si esa misma linea llego entera al STM32~~ - retirado el 11/09 con
// el parametro: ninguna linea reclamada cruza, asi que "esa misma linea" ya no viaja.
// Lo que el operario necesita distinguir -"la hora entro y va camino del equipo" frente
// a "entro y el equipo no se entero"- lo decide ahora siembra_ahora() DENTRO de la rama
// de SET_RTC, despues de poner el DS3231.
void despachador_atender(const char* linea);

#endif // DESPACHADOR_H
