// ===== 01_Firmware/ESP32_Expansion/include/trama.h =====
//
// FORMATO DE TRAMA. Se valida el FORMATO, nunca los comandos.
//
// 🔴 ESTO SE USA EN UN SOLO SENTIDO: STM32 -> APP. Y LA RAZON ES UNA MEDIDA.
//
// El censo de quien pone y quien comprueba el checksum, hecho sobre los tres fuentes
// el 31/08, da esto:
//
//   quien           pone *XX          comprueba el de entrada
//   ------------    --------------    -----------------------------------------------
//   la app          NO (app.js:199)   NO (app.js:734 hace split('*')[0] y lo tira)
//   el STM32        SI (Maestro:46)   NO (procesarComando() arranca con strcmp)
//
// Las dos casillas de la derecha estan vacias: HOY NO HAY UN SOLO CHECKSUM VERIFICADO
// EN TODA LA CADENA. Este fichero es el primero que verifica uno, y solo puede hacerlo
// en el sentido de vuelta, porque es el unico donde alguien lo pone.
//
// EN EL SENTIDO DE IDA NO SE VALIDA NI SE ANADE. El comando real que la app manda es
// "CMD:PIN:1234:SET_MODO:AUTO\r\n": sin '$' y sin '*XX'. Exigirle checksum descartaria
// el 100% del trafico legitimo, y ANADIRSELO al salir haria que el strcmp del STM32
// -que compara la linea entera- no casara con ningun comando.
//
// (18_Especificacion_Firmware_ESP32.md 3.4 ordena lo contrario y cita un
// formatearComando() que no existe. Necesita correccion; no se toca desde aqui.)
//
// EL COROLARIO QUE HAY QUE ESCRIBIR PARA QUE NADIE LO "MEJORE": validar el CRC aqui NO
// convierte el enlace en autenticado. El STM32 sigue aceptando cualquier linea que le
// llegue por PB7; quien pinche un hilo en J17 p2 manda comandos sin CRC y el equipo
// obedece. El ESP32 es una PUERTA, no una cerradura.

#ifndef TRAMA_H
#define TRAMA_H

#include <Arduino.h>

// XOR-8 sobre el payload SALTANDO el '$' inicial y PARANDO en el '*'.
//
// Los dos detalles son del fuente del STM32, no de una convencion general: la llamada
// es calcularChecksum(payload + 1) -de ahi el salto- y el bucle es
// while (*str && *str != '*') -de ahi la parada-. Copiar "un XOR de la cadena" sin esos
// dos matices produce un checksum que nunca casa y un puente que descarta todo.
uint8_t trama_checksum(const char* desde_tras_el_dolar);

// true si la linea tiene la forma $<payload>*<XX> y el XOR-8 casa.
//
// NO mira que comando es, ni si el comando existe, ni si lleva PIN. Formato, no
// contenido: un puente que conociera la lista de comandos habria que recompilarlo cada
// vez que el protocolo crece.
bool trama_valida(const char* linea);

// Compone una trama PROPIA DEL PUENTE con su checksum, en el buffer dado.
//
// B-4: todo lo que sale de aqui lleva NODE:PUENTE, y eso no es cosmetica. Un $ERR del
// puente que pareciera del STM32 manda a diagnosticar el poste equivocado: el tecnico
// buscaria en el firmware del semaforo un rechazo que lo genero el accesorio.
//
// Y estas tramas van SIEMPRE hacia la app, NUNCA hacia el STM32 (B-3): el puente no
// compone $ACK ni $STATUS en nombre del equipo.
size_t trama_componer(char* destino, size_t capacidad, const char* payload);

#endif // TRAMA_H
