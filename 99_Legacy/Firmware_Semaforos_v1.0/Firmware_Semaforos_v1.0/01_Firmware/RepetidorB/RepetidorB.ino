#include "pines.h"

HardwareSerial RadioA(1);
HardwareSerial RadioC(2);

String bufA = "";
String bufC = "";
unsigned long ultimoLatido = 0;

// OPT-5: Prevención de RAM Overflow y Deadlock
const size_t MAX_BUFFER = 64;
const unsigned long TIMEOUT_RX = 500;
unsigned long tUltimoByteA = 0;
unsigned long tUltimoByteC = 0;

void enviarLinea(HardwareSerial &puerto, int pinDeRe, const String &linea) {
  digitalWrite(pinDeRe, HIGH);
  delayMicroseconds(50);
  puerto.print(linea);
  puerto.print('\n');
  puerto.flush();
  delayMicroseconds(50);
  digitalWrite(pinDeRe, LOW);
}

void setup() {
  Serial.begin(BAUD_DEBUG);
  delay(1000);
  Serial.println("=== Iniciando Repetidor B (modo combinado) ===");

  pinMode(M1_DE_RE, OUTPUT);
  pinMode(M2_DE_RE, OUTPUT);
  digitalWrite(M1_DE_RE, LOW);
  digitalWrite(M2_DE_RE, LOW);
  Serial.println("DE/RE de ambos modulos en LOW (recepcion).");

  RadioA.begin(BAUD_RADIO, SERIAL_8N1, M1_RX, M1_TX);
  Serial.println("UART1 (modulo #1) iniciado en pines RX=16 TX=17.");

  RadioC.begin(BAUD_RADIO, SERIAL_8N1, M2_RX, M2_TX);
  Serial.println("UART2 (modulo #2) iniciado en pines RX=32 TX=33.");

  Serial.println("=== Repetidor B listo. Esperando datos... ===");
  ultimoLatido = millis();
}

void loop() {
  // Latido cada 3 segundos, para confirmar que el ESP32 sigue vivo y el loop corre
  if (millis() - ultimoLatido > 3000) {
    Serial.println("[latido] ESP32 activo, esperando trafico...");
    ultimoLatido = millis();
  }

  // ----- Lado A (modulo #1) -----
  if (bufA.length() > 0 && (millis() - tUltimoByteA > TIMEOUT_RX)) {
    Serial.println("[A] Timeout alcanzado. Limpiando buffer...");
    bufA = "";
  }

  while (RadioA.available()) {
    int raw = RadioA.read();
    char c = (char)raw;
    tUltimoByteA = millis();

    Serial.print("  [byte crudo A] 0x");
    if (raw < 0x10) Serial.print("0");
    Serial.println(raw, HEX);

    if (c == '\n') {
      Serial.print("A->C (linea completa): [");
      Serial.print(bufA);
      Serial.println("]");
      enviarLinea(RadioC, M2_DE_RE, bufA);
      bufA = "";
    } else if (c != '\r') {
      bufA += c;
      if (bufA.length() >= MAX_BUFFER) {
        Serial.println("[A] Buffer Overflow detectado por ruido. Limpiando...");
        bufA = "";
      }
    }
  }

  // ----- Lado C (modulo #2) -----
  if (bufC.length() > 0 && (millis() - tUltimoByteC > TIMEOUT_RX)) {
    Serial.println("[C] Timeout alcanzado. Limpiando buffer...");
    bufC = "";
  }

  while (RadioC.available()) {
    int raw = RadioC.read();
    char c = (char)raw;
    tUltimoByteC = millis();

    Serial.print("  [byte crudo C] 0x");
    if (raw < 0x10) Serial.print("0");
    Serial.println(raw, HEX);

    if (c == '\n') {
      Serial.print("C->A (linea completa): [");
      Serial.print(bufC);
      Serial.println("]");
      enviarLinea(RadioA, M1_DE_RE, bufC);
      bufC = "";
    } else if (c != '\r') {
      bufC += c;
      if (bufC.length() >= MAX_BUFFER) {
        Serial.println("[C] Buffer Overflow detectado por ruido. Limpiando...");
        bufC = "";
      }
    }
  }
}