// ===== include/deg_auto.h =====
#pragma once
#include <Arduino.h>
#include "protocolo.h"

// ---------------------------------------------------------------------------
// A-15 (SPEC_2 7.ter) - EL DEGRADADO AUTOMATICO, PUNTA ESCLAVO.
//
// Modulo pareado con Maestro/{src,include}/deg_auto.*. Con la opcion apagada en cualquiera
// de los dos postes el equipo hace exactamente lo de antes. Con la opcion puesta en los dos,
// tras DEG_AUTO_ESPERA_MS sin CMD_PING ni CMD_GO_GREEN entra solo por la puerta del testigo
// (degradado_entrarTestigo, D-35), sin telefono. La luz la decide modo_degradado.cpp.
// ---------------------------------------------------------------------------

static const unsigned long DEG_AUTO_ESPERA_MS = 300000UL;  // 5 min sin gobierno (A-15)
static const uint32_t DEG_AUTO_MARCA_S = 300;               // inicio: multiplo de 5 min
static const uint32_t DEG_AUTO_ROJO_MIN_S = 420;            // rojo fijo minimo antes de inicio
static const int DEG_AUTO_DESPEJE_S = 30;                   // el suelo del testigo
static const uint32_t PRESENTE_S = 10;                      // cadencia de CMD_PRESENTE
static const unsigned long DEG_AUTO_ACUSE_MS = 10000UL;     // tres latidos y margen

enum DegAutoOrden { DAO_ACEPTADA, DAO_SIN_ENLACE, DAO_EN_DEGRADADO };
enum DegAutoVeredicto { DAV_NINGUNO, DAV_ON_EFECTIVO, DAV_ON_FALTA, DAV_OFF, DAV_SIN_ACUSE };

// Cuenta, entrada, emision de PRESENTE en Degradado y plazo del $ACK diferido. main.cpp.
void degAuto_loop();
// Los bits APTO|ECO del CMD_PONG; 0 para cualquier otro comando.
uint8_t degAuto_paramSaliente(uint8_t cmd);
// Se llama con cada trama ANTES de reloj_notarRadio(). true si consumio un PRESENTE: esa
// trama no llega al resto de main.cpp (ni hora, ni tUltimoComando, ni salida del Degradado).
bool degAuto_alRecibir(const RF_Packet* pkt);
DegAutoOrden degAuto_orden(bool activar);
DegAutoVeredicto degAuto_veredicto();
bool degAuto_aptoPropio();
