// ===== include/protocolo.h =====
#pragma once
#include <Arduino.h>

// RF Binary Commands
#define CMD_GO_GREEN   0x01
#define CMD_GO_RED     0x02
#define CMD_ACK_GREEN  0x03
#define CMD_ACK_RED    0x04

#pragma pack(push, 1)
struct RF_Packet {
    uint8_t msgID;
    uint8_t command;
    uint8_t param;
    uint8_t crc;
};
#pragma pack(pop)

void protocolo_setup();
void protocolo_enviarPaquete(uint8_t cmd, uint8_t param = 0);
bool protocolo_hayPaqueteDisponible(RF_Packet* destino);

// Funciones de IA (Mantiene strings porque va por cable serie directo a la RPi)
void protocolo_actualizarAI();
int protocolo_obtenerAutosEsperandoAI();
unsigned long protocolo_obtenerUltimoTiempoAI();