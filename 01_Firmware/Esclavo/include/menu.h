// ===== include/menu.h (ESCLAVO) =====
#pragma once
#include <Arduino.h>

// ---------------------------------------------------------------------------
// N-16 — Menu del Esclavo.
//
// Esta cabecera era una copia huerfana del Maestro: declaraba MODO_MANUAL,
// MODO_AUTOMATICO y MODO_INTELIGENTE en un firmware que no tiene ninguno de los
// tres, y ningun .cpp la implementaba. Se reescribe entera.
//
// EL ESCLAVO NO OFRECE MODOS DE OPERACION, Y NO ES UN OLVIDO
// ---------------------------------------------------------
// Quien decide el ciclo es el Maestro. Poner aqui MANUAL o AUTOMATICO seria dar a
// las dos puntas la capacidad de decidir a la vez, y el dia que un tecnico
// arranque un modo en cada gabinete el cruce queda con dos cerebros discutiendo
// por radio quien tiene razon. El menu tiene exactamente dos entradas:
//
//   ESTADO          - solo lee: hora propia con SEGUNDOS, antiguedad de la ultima
//                     sincronizacion y contadores de linea (SFTY-15).
//   MODO DEGRADADO  - entrar y salir (SFTY-21).
//
// Tampoco hay ajuste de hora: llega por radio desde el Maestro (SFTY-23), y
// teclearla aqui reintroduce el desfase de hasta 59 s que ese mecanismo elimino.
//
// La interfaz NO detiene el ciclo. En el Maestro, entrar al menu fuerza rojo fijo
// en las dos puntas porque el menu es donde se decide; aqui no se decide nada, y
// dejar el cruce parado porque alguien esta mirando una pantalla de diagnostico
// seria cambiar la operacion por consultarla. El Esclavo sigue obedeciendo al
// Maestro mientras el tecnico navega.
// ---------------------------------------------------------------------------

void menu_setup();
void menu_loop();

// ---------------------------------------------------------------------------
// SFTY-21 — Lo consulta mando.cpp para inhibir las secuencias del mando de reles.
//
// "MENU ABIERTO" NO PUEDE SIGNIFICAR AQUI LO MISMO QUE EN EL MAESTRO
// ------------------------------------------------------------------
// En el Maestro el menu es un MODO del que se sale para volver a operar, asi que
// "en el menu" es un estado excepcional. En el Esclavo la pantalla no se cierra
// nunca: menu_loop() corre en cada vuelta y siempre hay algo dibujado. Tomar al pie
// de la letra "con la pantalla encendida no hay mando" dejaria el mando muerto
// SIEMPRE, que es justo lo contrario de lo que N-19 viene a resolver.
//
// Lo que se traslada es el RIESGO, no la letra: que una rafaga de pulsos a ciegas
// caiga sobre un cursor que puede CONFIRMAR algo. En el Esclavo eso solo ocurre por
// debajo del listado inicial -en MODO DEGRADADO y en su pantalla de confirmacion,
// donde ACEPTAR activa el modo-, y ademas estar ahi significa que hay una persona
// delante del gabinete navegando: sus pulsaciones son para la pantalla, no para el
// mando. En el listado inicial, A y B solo mueven el cursor entre dos opciones y
// ningun pulso puede confirmar nada, asi que ese es el estado de reposo en el que el
// mando DEBE funcionar.
//
// Va acompanado del regreso automatico al listado por inactividad (ver menu.cpp).
// Sin el, una pantalla dejada abierta al bajar del gabinete dejaria el mando mudo de
// forma indefinida, y desde el suelo no habria como notarlo: a diferencia del
// Maestro, aqui el menu NO detiene el ciclo, asi que "las luces ciclan luego el menu
// esta cerrado" no vale como indicio.
bool menu_estaAbierto();
