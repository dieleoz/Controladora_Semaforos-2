// ===== src/protocolo.cpp =====
#include "protocolo.h"
#include "pines.h"
#include <string.h>

static HardwareSerial Bus(RS485_OUT_RX, RS485_OUT_TX);
static char bufIn[64];
static uint8_t idxIn = 0;
static uint16_t msgIdCounter = 0;
static uint16_t ultimoIdRecibido = 0;

// SFTY-3: Función de cálculo de Checksum (XOR)
static uint8_t calcularCRC(const char* str) {
  uint8_t crc = 0;
  while (*str) {
    crc ^= *str++;
  }
  return crc;
}

void protocolo_setup() {
  pinMode(LORA_DE_RE, OUTPUT);
  digitalWrite(LORA_DE_RE, LOW);
  Bus.begin(9600);
}

// SFTY-3: Enviar con formato "TEXTO,ID,CRC"
void protocolo_enviarLinea(const char* linea) {
  msgIdCounter++;
  
  char payload[64];
  snprintf(payload, sizeof(payload), "%s,%u", linea, msgIdCounter);
  
  uint8_t crc = calcularCRC(payload);
  
  char paquete[80];
  snprintf(paquete, sizeof(paquete), "%s,%02X\n", payload, crc);
  
  digitalWrite(LORA_DE_RE, HIGH);
  delay(2); // Dar tiempo al MAX485 y módulo LoRa para entrar en modo TX
  Bus.print(paquete);
  Bus.flush();
  digitalWrite(LORA_DE_RE, LOW);
}

bool protocolo_hayLineaDisponible(char* destino, size_t maxLen) {
  while (Bus.available() > 0) {
    char c = Bus.read();
    if (c == '\n' || c == '\r') {
      if (idxIn > 0) {
        bufIn[idxIn] = '\0';
        idxIn = 0;
        
        // SFTY-3: Validar formato "TEXTO,ID,CRC"
        char* lastComma = strrchr(bufIn, ',');
        if (!lastComma) continue; // Descartar si no hay CRC
        
        *lastComma = '\0'; // Cortamos el string para dejar "TEXTO,ID"
        const char* crcStr = lastComma + 1;
        uint8_t crcRecibido = (uint8_t)strtol(crcStr, NULL, 16);
        uint8_t crcCalculado = calcularCRC(bufIn);
        
        if (crcRecibido != crcCalculado) {
            continue; // CRC Inválido (Ruido de radio)
        }
        
        char* secondComma = strrchr(bufIn, ',');
        if (!secondComma) continue; // Descartar si no hay ID
        
        *secondComma = '\0'; // Cortamos para dejar solo "TEXTO"
        uint16_t idRecibido = (uint16_t)atoi(secondComma + 1);
        
        if (idRecibido == ultimoIdRecibido) {
            continue; // Replay Attack (Mismo paquete rebotado por el repetidor)
        }
        ultimoIdRecibido = idRecibido;
        
        strncpy(destino, bufIn, maxLen - 1);
        destino[maxLen - 1] = '\0';
        return true;
      }
    } else if (idxIn < sizeof(bufIn) - 1) {
      bufIn[idxIn++] = c;
    }
  }
  return false;
}