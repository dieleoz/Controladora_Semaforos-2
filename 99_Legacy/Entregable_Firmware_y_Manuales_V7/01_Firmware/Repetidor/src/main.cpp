#include <Arduino.h>
#include "pines_repetidor.h"

HardwareSerial RadioA(1);
HardwareSerial RadioC(2);

void setup() {
 Serial.begin(115200);
 pinMode(M1_DE_RE, OUTPUT);
 pinMode(M2_DE_RE, OUTPUT);
 digitalWrite(M1_DE_RE, LOW);
 digitalWrite(M2_DE_RE, LOW);
 RadioA.begin(9600, SERIAL_8N1, M1_RX, M1_TX);
 RadioC.begin(9600, SERIAL_8N1, M2_RX, M2_TX);
}

void loop() {
 if (RadioA.available()) {
 digitalWrite(M2_DE_RE, HIGH);
 delayMicroseconds(50);
 while (RadioA.available()) {
 RadioC.write(RadioA.read());
 }
 RadioC.flush();
 delayMicroseconds(50);
 digitalWrite(M2_DE_RE, LOW);
 }
 if (RadioC.available()) {
 digitalWrite(M1_DE_RE, HIGH);
 delayMicroseconds(50);
 while (RadioC.available()) {
 RadioA.write(RadioC.read());
 }
 RadioA.flush();
 delayMicroseconds(50);
 digitalWrite(M1_DE_RE, LOW);
 }
}
