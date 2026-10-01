// ===== include/deg_auto.h =====
#pragma once
#include <Arduino.h>
#include "protocolo.h"

// ---------------------------------------------------------------------------
// A-15 (SPEC_2 7.ter) - EL DEGRADADO AUTOMATICO, PUNTA MAESTRO.
//
// Modulo pareado con Esclavo/{src,include}/deg_auto.*: misma idea, distinta punta. Con la
// opcion apagada en cualquiera de los dos postes el equipo hace exactamente lo de antes.
// Con la opcion puesta en los dos, tras DEG_AUTO_ESPERA_MS sin respuesta del Esclavo entra
// solo por la puerta del testigo (modo_degradado_entrarTestigo, D-35), sin telefono.
// La luz no la toca este modulo: la decide modo_degradado.cpp como con el testigo manual.
// ---------------------------------------------------------------------------

static const unsigned long DEG_AUTO_ESPERA_MS = 300000UL;  // 5 min sin respuesta (A-15)
static const uint32_t DEG_AUTO_MARCA_S = 300;               // inicio: multiplo de 5 min
static const uint32_t DEG_AUTO_ROJO_MIN_S = 420;            // rojo fijo minimo antes de inicio
static const int DEG_AUTO_DESPEJE_S = 30;                   // el suelo del testigo
static const uint32_t PRESENTE_S = 10;                      // cadencia de CMD_PRESENTE
static const unsigned long ENLACE_AVISO_REPETIR_MS = 60000UL; // ENLACE_DISPONIBLE mientras se oiga
static const unsigned long DEG_AUTO_ACUSE_MS = 10000UL;     // tres latidos y margen

// Lo que devuelve SET_DEG_AUTO en el instante de la orden (molde SET_TIEMPOS).
enum DegAutoOrden { DAO_ACEPTADA, DAO_SIN_ENLACE, DAO_EN_DEGRADADO };
// El $ACK diferido, cuando llega el ECO del otro poste o vence el plazo. Se consume una vez.
enum DegAutoVeredicto { DAV_NINGUNO, DAV_ON_EFECTIVO, DAV_ON_FALTA, DAV_OFF, DAV_SIN_ACUSE };

// Cuenta, entrada, emision de PRESENTE en Degradado y plazo del $ACK diferido. main.cpp.
void degAuto_loop();
// Lector de la radio con el Maestro en Degradado: consume todo y solo atiende PRESENTE.
void degAuto_escucharEnDegradado();
// El envio de coordinador.cpp: anade APTO|ECO al param de PING/GO_GREEN/GO_RED.
void degAuto_enviar(uint8_t cmd, uint8_t param = 0);
// Copia el APTO del PONG y cierra el $ACK diferido. true si consumio un PRESENTE.
bool degAuto_alRecibir(const RF_Packet* pkt);
DegAutoOrden degAuto_orden(bool activar);
DegAutoVeredicto degAuto_veredicto();
// El APTO de esta punta ahora mismo (CONSULTA_DEG_AUTO).
bool degAuto_aptoPropio();
