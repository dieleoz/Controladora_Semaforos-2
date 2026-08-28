// ===== include/identidad.h =====
#pragma once
#include <Arduino.h>

// ESTE FICHERO Y SU .cpp DEBEN SER IDENTICOS EN MAESTRO Y ESCLAVO.
//
// LA IDENTIDAD DEL EQUIPO, Y POR QUE NO LA ESCRIBE NADIE
// ------------------------------------------------------
// Cada equipo necesita un nombre estable: para emparejarse con su pareja, para que la
// caja negra diga de quien es cada evento, y para que el tecnico sepa a que poste esta
// conectado antes de mandarle nada.
//
// SE DERIVA DEL UID DE 96 BITS QUE EL STM32 TRAE GRABADO DE FABRICA (UID_BASE,
// 0x1FFFF7E8 en el F103, definido por el CMSIS). Esa fuente gana a todas las
// alternativas que se consideraron:
//
//   - No hace falta base de datos de fabrica ni numerar equipos a mano.
//   - No se puede falsificar ni reescribir: no es un ajuste, es quien es.
//   - No gasta un registro de respaldo ni un ciclo de borrado de flash.
//   - No necesita reloj.
//
// SE DESCARTO ACUÑARLA CON FECHA Y HORA AL ARRANCAR, que era la idea natural. En un
// equipo recien salido de taller reloj_enHora() es FALSO hasta que alguien pone la
// hora, asi que todas las unidades nacerian con el mismo sello. Y un
// "SEM-20260826-193412" no le dice nada al tecnico parado en el Km 12.
//
// POR QUE TIENE QUE SER IDENTICO EN LAS DOS PUNTAS
// -------------------------------------------------
// El emparejamiento compara codigos. Si el Maestro derivara su serie de una forma y el
// Esclavo comprobara con otra, no casarian NUNCA y las dos puntas se quedarian en
// ambar sin que nadie entendiera por que. costura_01_contratos lo vigila byte a byte.

// El codigo de 24 bits que identifica a este equipo. Estable entre arranques: sale del
// silicio, no de memoria escribible.
//
// NUNCA devuelve 0x000000: ese valor esta reservado para "Esclavo sin matricular", y
// un equipo que se identificase asi romperia esa distincion.
uint32_t identidad_serie();   // 24 bits utiles

// La misma serie en 6 caracteres hexadecimales mas el terminador: "7A3F2C".
// `dst` debe tener sitio para 7 bytes. Es lo que va en la telemetria, en la pantalla y
// en el AT+NAME del modulo Bluetooth, para que la lista de emparejados de Android ya
// diga que equipo es cada uno ANTES de conectar.
void identidad_texto(char* dst);
