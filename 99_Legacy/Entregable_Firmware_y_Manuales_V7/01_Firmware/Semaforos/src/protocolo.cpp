// ===== src/protocolo.cpp =====
#include "protocolo.h"
#include "pines.h"
#include <string.h>

static HardwareSerial Bus(RS485_OUT_RX, RS485_OUT_TX);
static HardwareSerial AiBus(RS485_IN_RX, RS485_IN_TX); // SFTY-5: Segundo bus UART para IA

static char bufIn[64];
static uint8_t idxIn = 0;
static uint16_t msgIdCounter = 0;
static uint16_t ultimoIdRecibido = 0;

static int ai_autosEsperando = 0;
static unsigned long ai_ultimoMensaje = 0;
static char ai_bufIn[32];
static uint8_t ai_idxIn = 0;

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
  digitalWrite(LORA_DE_RE, LOW); // Bus Master-Slave escuchando
  
  pinMode(RS485_IN_DE_RE, OUTPUT);
  digitalWrite(RS485_IN_DE_RE, LOW); // SFTY-5: Habilitar recepción en MAX485 del puerto IA

  Bus.begin(9600);
  AiBus.begin(115200); // YOLO Edge script uses 115200
}

void protocolo_actualizarAI() {
  while (AiBus.available() > 0) {
    char c = AiBus.read();
    if (c == '\n' || c == '\r') {
      if (ai_idxIn > 0) {
        ai_bufIn[ai_idxIn] = '\0';
        ai_idxIn = 0;
        
        // Formato esperado: "AI_CARS:5"
        if (strncmp(ai_bufIn, "AI_CARS:", 8) == 0) {
           ai_autosEsperando = atoi(&ai_bufIn[8]);
           ai_ultimoMensaje = millis();
        }
      }
    } else if (ai_idxIn < sizeof(ai_bufIn) - 1) {
      ai_bufIn[ai_idxIn++] = c;
    }
  }
}

int protocolo_obtenerAutosEsperandoAI() {
  return ai_autosEsperando;
}

unsigned long protocolo_obtenerUltimoTiempoAI() {
  return ai_ultimoMensaje;
}

// SFTY-3: Función de cálculo de Checksum binario (XOR)
static uint8_t calcularCRC_Bin(const uint8_t* data, size_t len) {
  uint8_t crc = 0;
  for (size_t i = 0; i < len; i++) {
    crc ^= data[i];
  }
  return crc;
}

void protocolo_enviarPaquete(uint8_t cmd, uint8_t param) {
  msgIdCounter++;
  if (msgIdCounter > 255) msgIdCounter = 1;
  
  RF_Packet pkt;
  pkt.msgID = (uint8_t)msgIdCounter;
  pkt.command = cmd;
  pkt.param = param;
  pkt.crc = calcularCRC_Bin((const uint8_t*)&pkt, 3);
  
  digitalWrite(LORA_DE_RE, HIGH);
  delay(2); // Dar tiempo al MAX485 y módulo LoRa para entrar en modo TX
  Bus.write((const uint8_t*)&pkt, sizeof(RF_Packet));
  Bus.flush();
  digitalWrite(LORA_DE_RE, LOW);
}

static uint8_t binBuf[sizeof(RF_Packet)];
static uint8_t binIdx = 0;
static unsigned long lastByteTime = 0;

bool protocolo_hayPaqueteDisponible(RF_Packet* destino) {
  while (Bus.available() > 0) {
    // SFTY-3: Time-based sync. If more than 50ms passed since last byte, reset buffer
    if (millis() - lastByteTime > 50) {
      binIdx = 0; 
    }
    binBuf[binIdx++] = Bus.read();
    lastByteTime = millis();
    
    if (binIdx >= sizeof(RF_Packet)) {
      binIdx = 0; // Ready for next packet
      
      RF_Packet* pkt = (RF_Packet*)binBuf;
      uint8_t crcCalc = calcularCRC_Bin(binBuf, 3);
      
      if (crcCalc != pkt->crc) {
          continue; // Bad CRC, wait for next time-sync or packet
      }
      
      if (pkt->msgID == ultimoIdRecibido) {
          return false; // Replay attack protection
      }
      ultimoIdRecibido = pkt->msgID;
      
      *destino = *pkt;
      return true;
    }
  }
  return false;
}