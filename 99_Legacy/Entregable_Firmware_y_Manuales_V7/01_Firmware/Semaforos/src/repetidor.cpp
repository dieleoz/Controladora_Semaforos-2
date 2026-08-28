#include <Arduino.h>
#include "pines_repetidor.h"
// === REPETIDOR TRANSPARENTE BINARIO (FASE 3) ===
// Este código ha sido adaptado para NO USAR Strings ni buscar saltos de línea (\n).
// Lee los bytes crudos y los reenvía tal cual para no romper el protocolo binario con CRC.

HardwareSerial RadioA(1);
HardwareSerial RadioC(2);

unsigned long ultimoLatido = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("=== Iniciando Repetidor B (Modo Binario Transparente) ===");

  pinMode(M1_DE_RE, OUTPUT);
  pinMode(M2_DE_RE, OUTPUT);
  digitalWrite(M1_DE_RE, LOW); // RX
  digitalWrite(M2_DE_RE, LOW); // RX
  Serial.println("DE/RE de ambos modulos en LOW (recepcion).");

  RadioA.begin(9600, SERIAL_8N1, M1_RX, M1_TX);
  Serial.println("UART1 (Radio A) iniciado en RX=16 TX=17.");

  RadioC.begin(9600, SERIAL_8N1, M2_RX, M2_TX);
  Serial.println("UART2 (Radio C) iniciado en RX=32 TX=33.");

  Serial.println("=== Repetidor B listo. Esperando tramas binarias... ===");
  ultimoLatido = millis();
}

void loop() {
  // Latido de diagnóstico
  if (millis() - ultimoLatido > 3000) {
    ultimoLatido = millis();
  }

  // ----- Reenvío de Lado A -> Lado C -----
  if (RadioA.available()) {
    // Esperamos 5ms para asegurar que el paquete binario (4 bytes) llegue completo al buffer
    delay(5); 
    
    digitalWrite(M2_DE_RE, HIGH); // Habilitar TX en Radio C
    delayMicroseconds(50);
    
    Serial.print("[A->C] Reenviando paquete binario: ");
    while (RadioA.available()) {
      uint8_t byteRecibido = RadioA.read();
      RadioC.write(byteRecibido);
      
      // Imprimir para debug en el PC
      Serial.print("0x");
      if (byteRecibido < 0x10) Serial.print("0");
      Serial.print(byteRecibido, HEX);
      Serial.print(" ");
    }
    Serial.println();
    
    RadioC.flush(); // Esperar a que todos los bytes se transmitan
    delayMicroseconds(50);
    digitalWrite(M2_DE_RE, LOW); // Volver a RX
  }

  // ----- Reenvío de Lado C -> Lado A -----
  if (RadioC.available()) {
    // Esperamos 5ms para asegurar que el paquete binario (4 bytes) llegue completo
    delay(5); 
    
    digitalWrite(M1_DE_RE, HIGH); // Habilitar TX en Radio A
    delayMicroseconds(50);
    
    Serial.print("[C->A] Reenviando paquete binario: ");
    while (RadioC.available()) {
      uint8_t byteRecibido = RadioC.read();
      RadioA.write(byteRecibido);
      
      // Imprimir para debug en el PC
      Serial.print("0x");
      if (byteRecibido < 0x10) Serial.print("0");
      Serial.print(byteRecibido, HEX);
      Serial.print(" ");
    }
    Serial.println();
    
    RadioA.flush(); // Esperar a que todos los bytes se transmitan
    delayMicroseconds(50);
    digitalWrite(M1_DE_RE, LOW); // Volver a RX
  }
}
