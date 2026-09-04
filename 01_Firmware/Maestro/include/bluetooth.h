// ===== include/bluetooth.h =====
#pragma once
#include <Arduino.h>

/**
 * @brief Inicializa el puerto serie del modulo de radio corta (USART1 REMAPEADO:
 *        PB6 TX, PB7 RX) a 9600 bps. Sale por el conector J17, posiciones 3 y 2.
 */
void bluetooth_setup();

/**
 * @brief Procesa la recepción no bloqueante de comandos y emite telemetría periódica cada 2000ms.
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
 * @param origen Componente que origina el evento (ej. "CAMARA_1", "OPERARIO").
 * @param detalle Descripción de la acción (ej. "DEMANDA_VERDE_S1").
 *
 * N-114 - ORIGEN:RELOJ es la CONSULTA RELOJ de N-45 mudada a este canal. Sale detras de
 * los dos $ERR que nombran esa pantalla (SIN_CRISTAL_VEA_CONSULTA_RELOJ y
 * SIGUE_PARADO_VEA_CONSULTA_RELOJ) porque la pantalla ya no se puede abrir: MODO_HORA
 * solo se arma desde menu.cpp y hacen falta DOS botonAceptar(), que devuelve false
 * desde que PB14/PB15 son camaras. El DETALLE lleva los seis bits con los que razona la
 * cabecera de reloj.h -ON, RDY y BYP del LSE, RTCSEL, RTCEN y el contador crudo- y NO
 * lleva comas: el parser de la app corta por ',' y perderia todo lo que fuera detras.
 * Lo vigila el pack reloj_01_consulta_por_bluetooth.
 */
void bluetooth_reportarEvento(const char* origen, const char* detalle);

/**
 * @brief Indica si el modo de test de lámparas (6s) está en ejecución.
 */
bool bluetooth_testLedsActivo();
