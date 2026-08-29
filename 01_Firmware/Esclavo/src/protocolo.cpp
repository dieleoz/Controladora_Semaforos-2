// ===== src/protocolo.cpp =====
#include "protocolo.h"
#include "pines.h"
#include <string.h>

static HardwareSerial Bus(RS485_OUT_RX, RS485_OUT_TX);

static char bufIn[64];
static uint8_t idxIn = 0;
static uint8_t msgIdCounter = 0;
static uint8_t ultimoIdRecibido = 0;

void protocolo_setup() {
  pinMode(LORA_DE_RE, OUTPUT);
  digitalWrite(LORA_DE_RE, LOW); // Bus Master-Slave escuchando
  
  // AQUI NO SE ABRE NINGUN SEGUNDO PUERTO SERIE, Y NO ES UN OLVIDO.
  //
  // Hasta N-76 se abria aqui "AiBus", un HardwareSerial declarado sobre
  // (RS485_IN_RX, RS485_IN_TX) = (PA10, PA9), que es EL MISMO USART1 que usa
  // SerialBT en bluetooth.cpp. Eran dos objetos peleandose un unico periferico,
  // y ademas a dos velocidades: 115200 aqui y 9600 alli. Funcionaba por
  // accidente de orden -bluetooth_setup() corre despues (main.cpp) y ganaba-,
  // de modo que el puerto quedaba a 9600 y el "puerto IA" jamas existio a
  // 115200.
  //
  // LA LECCION SIGUE VIGENTE AUNQUE EL CODIGO YA NO ESTE: dos objetos sobre el
  // mismo periferico a velocidades distintas no dan error, dan el ultimo que
  // arranco. Antes de declarar un HardwareSerial hay que mirar QUE USART tocan
  // sus pines, no si los pines estan libres en el conector. Hoy SerialBT vive
  // en PB6/PB7 (USART1 remapeado, conector J17), y abrir aqui ese mismo USART1
  // con el mapeo viejo seria repetir la pelea con otro disfraz.
  //
  // N-86 retiro el objeto entero, no solo su apertura, y esa distincion es la
  // parte medida: el enlazador SI descartaba las tres funciones AI -nadie las
  // llamaba-, pero NO el objeto, porque su constructor cuelga de .init_array y
  // corre en cada arranque. Eran 280 B de .bss permanentes por punta para un
  // puerto que no existia. Un camino muerto que no cuesta flash puede seguir
  // costando RAM.
  //
  // Y la etiqueta "SFTY-5" que llevaba aquella declaracion NO era SFTY-5: esa
  // regla es la transicion de luz legal (OPTIMIZACIONES.md), no un puerto
  // serie. Queda anotado para no arrastrar la etiqueta equivocada.
  //
  // PA8 (RS485_IN_DE_RE) no se toca aqui: bluetooth_setup() lo pone en HIGH y
  // corre despues, que es exactamente lo que pasaba antes. El comportamiento
  // del pin no cambia.

  Bus.begin(9600);
}

// SFTY-7: Polinomio CRC-8 Maxim/Dallas (0x31) bit a bit
static uint8_t calcularCRC_Bin(const uint8_t* data, size_t len) {
  uint8_t crc = 0x00;
  for (size_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (uint8_t bit = 0; bit < 8; bit++) {
      if (crc & 0x80) {
        crc = (crc << 1) ^ 0x31;
      } else {
        crc <<= 1;
      }
    }
  }
  return crc;
}

void protocolo_enviarPaquete(uint8_t cmd, uint8_t param) {
  msgIdCounter++;
  if (msgIdCounter == 0) msgIdCounter = 1; // FIX H-4: Prevenir msgID=0 al desbordar uint8_t
  
  RF_Packet pkt;
  pkt.msgID = (uint8_t)msgIdCounter;
  pkt.command = cmd;
  pkt.param = param;
  pkt.crc = calcularCRC_Bin((const uint8_t*)&pkt, 3);
  
  digitalWrite(LORA_DE_RE, HIGH);
  delay(2); // Dar tiempo al MAX485 y módulo LoRa para entrar en modo TX
  // SFTY-11: Ráfaga (Burst) de RF_BURST_COPIES copias para resiliencia al ruido RF.
  // El coste de la ráfaga depende de la TASA AEREA, no del protocolo: a 2.4 kbps son
  // ~0.13s de aire, a 0.3 kbps eran ~2.2s y ahí saturaba el canal (fallo N-1 en campo).
  // Con la tasa corregida la redundancia vuelve a ser barata y se mantiene en 3.
  for (uint8_t r = 0; r < RF_BURST_COPIES; r++) {
    Bus.write((const uint8_t*)&pkt, sizeof(RF_Packet));
  }
  Bus.flush();
  delayMicroseconds(1200); // SFTY-8: Asegurar transmisión completa del bit de STOP del CRC
  digitalWrite(LORA_DE_RE, LOW);
}

static uint8_t binBuf[sizeof(RF_Packet)];
static uint8_t binIdx = 0;
static unsigned long lastByteTime = 0;

// SFTY-15: Contadores de diagnostico de linea. Permiten distinguir en pantalla
// tres fallos que de otro modo se ven todos igual ("no hay comunicacion"):
//   - 0 bytes                  -> no llega nada: cobertura, canal o antena
//   - muchos bytes, 0 validas  -> llega BASURA: cableado, linea flotando, radio atascada
//   - bytes y validas, calidad baja -> enlace marginal por distancia
static unsigned long cntBytes = 0;
static unsigned long cntValidas = 0;
static unsigned long cntDescartadas = 0;

bool protocolo_hayPaqueteDisponible(RF_Packet* destino) {
  while (Bus.available() > 0) {
    // SFTY-3: Time-based sync. If more than 50ms passed since last byte, reset buffer
    if (millis() - lastByteTime > 50) {
      binIdx = 0;
    }
    binBuf[binIdx++] = Bus.read();
    cntBytes++;
    lastByteTime = millis();

    if (binIdx >= sizeof(RF_Packet)) {
      RF_Packet* pkt = (RF_Packet*)binBuf;
      uint8_t crcCalc = calcularCRC_Bin(binBuf, 3);

      if (crcCalc != pkt->crc) {
        // SFTY-10: Ventana Deslizante (Sliding Window). Si el CRC falla, desplazamos 1 byte
        // para re-enganchar el siguiente paquete de la ráfaga de respaldo sin desalinear el búfer
        // NOTA: un solo byte de ruido puede provocar varios descartes seguidos mientras la
        // ventana se desplaza. El contador mide RUIDO, no tramas perdidas una a una.
        cntDescartadas++;
        memmove(binBuf, binBuf + 1, sizeof(RF_Packet) - 1);
        binIdx--;
        continue;
      }

      cntValidas++;
      binIdx = 0; // Paquete válido procesado, resetear índice para próximo paquete
      
      // SFTY-11: Filtrar copias redundantes de la ráfaga (mismo msgID). Con
      // RF_BURST_COPIES = 3 descarta las 2 copias de respaldo; ademas protege
      // ante ecos del bus o retransmisiones de la radio.
      if (pkt->msgID == ultimoIdRecibido) {
        return false; // Duplicado de ráfaga, descartar
      }
      ultimoIdRecibido = pkt->msgID;
      
      *destino = *pkt;
      return true;
    }
  }
  return false;
}

void protocolo_resetReplayProtection() {
  ultimoIdRecibido = 0;
}

// SFTY-15: Diagnostico de linea.
unsigned long protocolo_bytesRecibidos()    { return cntBytes; }
unsigned long protocolo_tramasValidas()     { return cntValidas; }
unsigned long protocolo_tramasDescartadas() { return cntDescartadas; }

void protocolo_reiniciarContadores() {
  cntBytes = 0;
  cntValidas = 0;
  cntDescartadas = 0;
}

