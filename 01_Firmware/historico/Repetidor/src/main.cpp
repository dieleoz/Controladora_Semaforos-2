// ===========================================================================
// PUENTE REPETIDOR ESP32  —  V8.4
// ===========================================================================
// Enlaza dos radios E90-DTU back-to-back dentro de la topologia de 4 radios.
//
// SFTY-16 — POR QUE VALIDA EN VEZ DE REENVIAR A CIEGAS
// -----------------------------------------------------------------------
// La version anterior era un passthrough tonto: activaba la transmision hacia
// la radio opuesta en cuanto aparecia CUALQUIER byte, y la cerraba tras 5 ms de
// silencio. Con eso, si el par RS485 de entrada queda flotando (falta de
// resistencias de polarizacion, transceptor sin alimentar o cable partido), el
// receptor lee ruido continuo: el silencio de 5 ms NUNCA llega, la linea de
// transmision queda permanentemente activa y la radio de salida se queda
// RADIANDO BASURA AL AIRE sin parar, bloqueando el canal por completo.
//
// Sintoma observado en campo el 31/07/2026: LED TX de B2 encendido fijo en vez
// de destellar cada 3 s, y enlace del repetidor inservible.
//
// Ahora el puente conoce el formato (4 bytes con CRC-8 Maxim 0x31) y SOLO
// retransmite tramas completas y validas. El ruido de linea se descarta aqui
// dentro y no llega al aire. Ademas, la transmision se activa unicamente cuando
// hay algo real que enviar, no ante el primer byte que aparezca.
//
// El protocolo es propio, asi que perder la transparencia no cuesta nada.
//
// Compilar con -D PUENTE_TRANSPARENTE para volver al passthrough anterior.
// ===========================================================================

#include <Arduino.h>
#include "pines_repetidor.h"

HardwareSerial RadioA(1); // Radio B1 (Lado Maestro)
HardwareSerial RadioC(2); // Radio B2 (Lado Esclavo)

static const uint8_t TAM_TRAMA = 4;   // msgID, command, param, crc

// Mismo polinomio que usan Maestro y Esclavo (SFTY-3).
static uint8_t crc8Maxim(const uint8_t *datos, uint8_t largo) {
  uint8_t crc = 0x00;
  for (uint8_t i = 0; i < largo; i++) {
    crc ^= datos[i];
    for (uint8_t bit = 0; bit < 8; bit++) {
      crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x31) : (uint8_t)(crc << 1);
    }
  }
  return crc;
}

// Estado de recepcion de un sentido del puente.
struct Sentido {
  HardwareSerial *entrada;
  HardwareSerial *salida;
  uint8_t pinDE;            // control DE/RE del transceptor de salida
  uint8_t buf[TAM_TRAMA];
  uint8_t idx;
  unsigned long bytesLeidos;
  unsigned long tramasValidas;
  unsigned long tramasDescartadas;
};

static Sentido haciaEsclavo;   // A (B1, lado Maestro)  ->  C (B2, lado Esclavo)
static Sentido haciaMaestro;   // C (B2, lado Esclavo)  ->  A (B1, lado Maestro)

static void iniciarSentido(Sentido &s, HardwareSerial *ent, HardwareSerial *sal, uint8_t pinDE) {
  s.entrada = ent;
  s.salida = sal;
  s.pinDE = pinDE;
  s.idx = 0;
  s.bytesLeidos = 0;
  s.tramasValidas = 0;
  s.tramasDescartadas = 0;
}

// Emite una trama ya validada, abriendo y cerrando el bus solo para ella.
static void emitirTrama(Sentido &s) {
  digitalWrite(s.pinDE, HIGH);
  delayMicroseconds(50);          // asentamiento del MAX3485 antes de los datos
  s.salida->write(s.buf, TAM_TRAMA);
  s.salida->flush();              // espera a que salga el ultimo bit de parada
  delayMicroseconds(100);
  digitalWrite(s.pinDE, LOW);     // devuelve la linea a recepcion
}

static void procesarSentido(Sentido &s) {
  while (s.entrada->available() > 0) {
    s.buf[s.idx++] = (uint8_t)s.entrada->read();
    s.bytesLeidos++;

    if (s.idx < TAM_TRAMA) continue;

    if (crc8Maxim(s.buf, 3) != s.buf[3]) {
      // Ventana deslizante: desplaza un byte y reintenta el enganche. Es lo que
      // permite recuperar la trama aunque llegue precedida de ruido.
      s.tramasDescartadas++;
      memmove(s.buf, s.buf + 1, TAM_TRAMA - 1);
      s.idx = TAM_TRAMA - 1;
      continue;
    }

    s.tramasValidas++;
    emitirTrama(s);
    s.idx = 0;
  }
}

#ifdef PUENTE_TRANSPARENTE
// Passthrough anterior, sin validar. Se conserva solo para poder comparar.
static unsigned long tUltimoA = 0, tUltimoC = 0;
static bool txA_activa = false, txC_activa = false;
#endif

// El informe periodico va SIEMPRE, tambien en produccion.
//
// Antes solo existia en el entorno `repetidor_diag`, y en campo eso costo horas:
// se cargaba `pio run -t upload`, que sube el entorno POR DEFECTO (produccion,
// mudo), la consola no mostraba mas que el arranque de la ROM y se concluia que
// "no hay datos de flujo". Un informe cada 2 s por USB no le cuesta nada a un
// ESP32 de 240 MHz y elimina esa trampa por completo.
static unsigned long tUltimoInforme = 0;

void setup() {
  Serial.begin(115200);
  pinMode(M1_DE_RE, OUTPUT);
  pinMode(M2_DE_RE, OUTPUT);
  digitalWrite(M1_DE_RE, LOW);  // ambas lineas en recepcion al arrancar
  digitalWrite(M2_DE_RE, LOW);

  RadioA.begin(9600, SERIAL_8N1, M1_RX, M1_TX);
  RadioC.begin(9600, SERIAL_8N1, M2_RX, M2_TX);

  iniciarSentido(haciaEsclavo, &RadioA, &RadioC, M2_DE_RE);
  iniciarSentido(haciaMaestro, &RadioC, &RadioA, M1_DE_RE);

  // Encabezado siempre visible: si esto NO aparece tras el arranque de la ROM,
  // el firmware no esta corriendo. Es la primera comprobacion en campo.
  Serial.println();
  Serial.println(F("=================================================="));
  Serial.println(F(" REPETIDOR ESP32  V8.4  -  puente con validacion CRC"));
  Serial.println(F("=================================================="));
  Serial.println(F("Informe cada 2 s. Con el Maestro encendido debe verse trafico en A:"));
  Serial.println(F("un latido cada 3 s = 12 bytes (rafaga de 3 copias) y 3 validas."));
  Serial.println(F("Si 'bytes' sube de miles y 'validas' se queda en 0, esa linea mete"));
  Serial.println(F("ruido: revisar cableado y resistencias de polarizacion."));
  Serial.println();
}

void loop() {
#ifdef PUENTE_TRANSPARENTE
  unsigned long ahora = millis();
  if (RadioA.available()) {
    if (!txC_activa) { digitalWrite(M2_DE_RE, HIGH); txC_activa = true; }
    while (RadioA.available()) RadioC.write(RadioA.read());
    tUltimoA = ahora;
  }
  if (txC_activa && (ahora - tUltimoA >= 5)) {
    RadioC.flush(); digitalWrite(M2_DE_RE, LOW); txC_activa = false;
  }
  if (RadioC.available()) {
    if (!txA_activa) { digitalWrite(M1_DE_RE, HIGH); txA_activa = true; }
    while (RadioC.available()) RadioA.write(RadioC.read());
    tUltimoC = ahora;
  }
  if (txA_activa && (ahora - tUltimoC >= 5)) {
    RadioA.flush(); digitalWrite(M1_DE_RE, LOW); txA_activa = false;
  }
#else
  procesarSentido(haciaEsclavo);
  procesarSentido(haciaMaestro);
#endif

  unsigned long t = millis();
  if (t - tUltimoInforme >= 2000) {
    tUltimoInforme = t;
    Serial.printf("[%6lus]  A<-Maestro: %6lu b / %4lu val / %5lu desc"
                  "   |   C<-Esclavo: %6lu b / %4lu val / %5lu desc\n",
                  t / 1000,
                  haciaEsclavo.bytesLeidos, haciaEsclavo.tramasValidas, haciaEsclavo.tramasDescartadas,
                  haciaMaestro.bytesLeidos, haciaMaestro.tramasValidas, haciaMaestro.tramasDescartadas);

    // Interpretacion automatica: dice en texto donde esta el corte.
    if (haciaEsclavo.tramasValidas > 0 && haciaMaestro.bytesLeidos == 0) {
      Serial.println(F("   >> El MAESTRO llega bien al puente, pero el ESCLAVO NO RESPONDE."));
      Serial.println(F("      El corte esta en el salto B2 -> Esclavo. Revisar que la radio"));
      Serial.println(F("      del Esclavo este en CANAL 10 (172 MHz), igual que B2."));
    }

#ifdef MODO_DIAGNOSTICO
    if (haciaEsclavo.bytesLeidos == 0 && haciaMaestro.bytesLeidos == 0) {
      Serial.println(F("   >> NO LLEGA NADA por ningun lado. Revisar canal y velocidad aerea"));
      Serial.println(F("      de las radios, alimentacion y cableado A/B."));
    } else if (haciaEsclavo.bytesLeidos > 500 && haciaEsclavo.tramasValidas == 0) {
      Serial.println(F("   >> RUIDO CONTINUO desde B1 (lado Maestro): entran muchos bytes y"));
      Serial.println(F("      ninguna trama valida. Linea RS485 flotando o mal cableada."));
      Serial.println(F("      El puente lo esta DESCARTANDO: ya no llega al aire."));
    } else if (haciaMaestro.bytesLeidos > 500 && haciaMaestro.tramasValidas == 0) {
      Serial.println(F("   >> RUIDO CONTINUO desde B2 (lado Esclavo). Mismo diagnostico."));
    }
#endif
  }
}
