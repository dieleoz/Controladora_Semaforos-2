// ===== Simulaciones/puente_esp32/Arduino.h =====
//
// Sustituto minimo de Arduino.h para compilar el bluetooth.cpp REAL de las dos
// puntas en el PC. Mismo patron que Validacion_LCD, Validacion_Respaldo y
// Validacion_Automatico: no emula la placa, solo cubre lo que estos fuentes usan.
//
// LA PIEZA QUE ESTE ARNES ANADE A LAS OTRAS TRES: HardwareSerial.
//
// bluetooth.cpp declara `static HardwareSerial SerialBT(PB7, PB6)` y todo el contrato
// del ESP32 pasa por ahi: available()/read() son el cable que viene del puente, y
// print() es el cable que va hacia el. Aqui ese objeto es una PAREJA DE COLAS DE
// BYTES que el arnes llena y vacia. No se emula el UART -ni baudios, ni bits de
// parada, ni FIFO del silicio-: se emula lo unico que el fuente puede observar, que
// son bytes que entran de uno en uno y bytes que salen.
//
// POR QUE ESO IMPORTA MAS DE LO QUE PARECE. El bucle receptor de bluetooth_loop()
// hace `while (SerialBT.available() > 0)`, asi que lo que decide si una trama se
// parte, se concatena o se trunca es EXACTAMENTE cuantos bytes hay en esa cola cuando
// se entra al bucle. Un arnes que entregara la linea entera de golpe no podria
// ejercer F1 -media trama sin terminador- ni F2 -dos escrituras-, que son justo los
// escenarios que el puente tiene que sobrevivir.
#pragma once

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <deque>

#define OUTPUT 1
#define INPUT 0
#define INPUT_PULLUP 2
#define LOW 0
#define HIGH 1

inline void pinMode(int, int) {}

// digitalWrite() GRABA, igual que en Validacion_Automatico y por el mismo motivo:
// el EFECTO de un comando de la app se mide sobre lo que semaforo.cpp escribio en
// los pines, no sobre la respuesta que el despachador imprimio. Un $ACK es una
// promesa; el pin es el hecho.
extern int arnes_pines[64];
extern unsigned long arnes_escrituras;
inline void digitalWrite(int pin, int valor) {
  if (pin >= 0 && pin < 64) arnes_pines[pin] = valor;
  arnes_escrituras++;
}

// La demanda por camara entra por aqui. El arnes la mueve; por defecto no hay coche.
extern int arnes_entradas[64];
inline int digitalRead(int pin) {
  return (pin >= 0 && pin < 64) ? arnes_entradas[pin] : HIGH;
}

extern unsigned long arnes_millis_valor;   // lo mueve el arnes, nunca solo
inline unsigned long millis() { return arnes_millis_valor; }

// No se define delay(): si algun fuente empezara a usarlo, mejor un error de
// enlazado que un arnes que se duerme de verdad.

// ---------------------------------------------------------------------------------
// HardwareSerial — las dos colas de bytes que SON el enlace con el ESP32.
// ---------------------------------------------------------------------------------
class HardwareSerial;
extern HardwareSerial* arnes_puertos[8];
extern int arnes_n_puertos;

class HardwareSerial {
 public:
  // SE REGISTRA SOLO AL CONSTRUIRSE, y esa es la unica forma de alcanzarlo.
  //
  // SerialBT es `static` dentro de bluetooth.cpp: desde el arnes NO hay manera de
  // nombrarlo, y hacerlo publico seria tocar el firmware para poder medirlo -que es
  // exactamente lo que este repositorio no hace-. El objeto se apunta a si mismo en
  // una lista global, y el arnes lo busca POR SUS PINES. Si manana alguien mueve
  // SerialBT a otro par, el arnes no lo encuentra y ABORTA, en vez de medir otro
  // puerto sin enterarse (N-86: dos objetos sobre el mismo periferico no dan error,
  // dan el ultimo que arranco).
  HardwareSerial(int rx, int tx) : pinRx(rx), pinTx(tx), baudios(0) {
    if (arnes_n_puertos < 8) arnes_puertos[arnes_n_puertos++] = this;
  }

  void begin(unsigned long b) { baudios = b; }

  int available() { return (int)entrada.size(); }

  int read() {
    if (entrada.empty()) return -1;
    char c = entrada.front();
    entrada.pop_front();
    return (int)(unsigned char)c;
  }

  void print(const char* s) { salida += s; }

  // --- puertas del arnes, no del firmware ---
  void arnes_meter(const char* datos, size_t n) {
    for (size_t i = 0; i < n; i++) entrada.push_back(datos[i]);
  }
  std::string arnes_sacar() { std::string s = salida; salida.clear(); return s; }
  int arnes_pinRx() const { return pinRx; }
  int arnes_pinTx() const { return pinTx; }
  unsigned long arnes_baudios() const { return baudios; }

 private:
  std::deque<char> entrada;
  std::string salida;
  int pinRx, pinTx;
  unsigned long baudios;
};
