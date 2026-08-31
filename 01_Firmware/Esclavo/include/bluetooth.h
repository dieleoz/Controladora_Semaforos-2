// ===== include/bluetooth.h =====
#pragma once
#include <Arduino.h>

/**
 * @brief Inicializa el puerto serie del modulo de radio corta (USART1 REMAPEADO:
 *        PB6 TX, PB7 RX) a 9600 bps. Sale por el conector J17, posiciones 3 y 2.
 */
void bluetooth_setup();

/**
 * @brief Procesa la recepción no bloqueante de comandos y emite telemetría periódica cada 1000ms.
 */
void bluetooth_loop();

/**
 * @brief Emite una trama inmediata de alarma de Caja Negra por Bluetooth ($ALARM,...).
 * @param evento Nombre del fallo (ej. "FALLO_RF"). N-73: el ejemplo decia
 *        "FALLO_RF_12S" y el numero quedo mintiendo al subir el umbral a 25 s
 *        (N-71). El umbral va en la CAUSA, no en el nombre del evento.
 * @param causa Causa raíz (ej. "TIMEOUT_LATIDO").
 * @param accion Medida de seguridad vial ejecutada (ej. "CAMBIO_A_AMBAR").
 */
void bluetooth_reportarAlarma(const char* evento, const char* causa, const char* accion);

/**
 * @brief Emite una trama de evento operativo ($EVENT,...).
 * @param origen Componente que origina el evento (ej. "CAMARA_3", "OPERARIO").
 * @param detalle Descripción de la acción (ej. "DEMANDA_VERDE_S2").
 */
void bluetooth_reportarEvento(const char* origen, const char* detalle);

/**
 * @brief Indica si el modo de test de lámparas (6s) está en ejecución.
 */
bool bluetooth_testLedsActivo();

// ---------------------------------------------------------------------------
// AMBAR DE EMERGENCIA PEDIDO POR BLUETOOTH (CMD:AMBAR_EMERGENCIA) — N-83.
//
// Mientras vale true, este nodo NO OBEDECE las ordenes de luz del Maestro: se queda
// en ambar intermitente con la talanquera arriba. Lo consulta main.cpp, en los MISMOS
// tres sitios que mando_ambarLocal(), y por el mismo motivo que explica mando.h.
//
// POR QUE TIENE QUE PESAR LO MISMO QUE EL DEL MANDO. Antes de N-83 no habia latch: la
// orden de la app dejaba el equipo en S_FALLO y el siguiente CMD_GO_RED del Maestro
// -que llega cada pocos segundos- lo devolvia a rojo. El ambar del mando era sagrado y
// el pedido desde el telefono se lo llevaba el siguiente latido. Para el operario eso
// se ve como un equipo que obedece y se vuelve atras solo, sin que nadie se lo diga; y
// la razon para pedir el ambar -alguien trabajando bajo la luz, un incidente en el
// tramo- no cambia segun por donde entro la orden.
//
// COMO SE SALE - N-106 (31/08). ESTE PARRAFO DECIA OTRA COSA Y SE REESCRIBE ENTERO.
//
// Decia que se salia "por A.A.A en el mando", que "no hay comando de Bluetooth que lo
// revoque" y que "el latch se cae solo en cuanto la luz deja de estar en S_FALLO". Lo
// primero y lo tercero eran la misma frase: la revocacion automatica de bluetooth_loop().
//
// Esa revocacion se RETIRA, porque dejo de ser correcta el dia que este comando pasa a
// salir del Modo Degradado por el todo-rojo de despedida: durante esos 10 a 90 s la luz
// esta en ROJO -fuera de S_FALLO-, asi que el latch moria antes de servir para nada y con
// el se apagaban los tres vetos de main.cpp. En su lugar queda lo mismo que tiene el
// mando: un SOSTENEDOR en bluetooth_loop() que re-arma el ambar en cuanto el Degradado
// suelta la luz.
//
// Y la salida ahora se pide, no se deduce: CMD:PIN:1234:CANCELAR_AMBAR (R-3). PIDE PIN
// porque quitar el ambar devuelve el cruce a dar verdes -abre paso-, mientras que pedirlo
// es la caida segura y no pide clave. Un latch sin salida seria un nodo sordo al Maestro
// hasta el proximo corte de corriente; una salida que ocurre sola es la maquina
// deshaciendo una proteccion que puso una persona. Esta es la tercera: la deshace otra
// persona, y queda escrita en la caja negra.
//
// El ambar del MANDO no lo quita este comando: los vetos de main.cpp miran las dos
// banderas, y CANCELAR_AMBAR lo dice en su RESULT cuando la otra sigue puesta.
// ---------------------------------------------------------------------------
bool bluetooth_ambarEmergencia();
