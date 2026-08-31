// ===== 01_Firmware/ESP32_Expansion/include/transporte_app.h =====
//
// LA MITAD BLUETOOTH. Interfaz estrecha: abrir / disponible / leer / escribir.
//
// BLQ-1 SE CERRO EL 31/08 Y EL MODULO ES UN ESP32-WROOM-32 CLASICO.
//
// La ficha del modulo comprado trae tres confirmaciones independientes de que NO es un
// S3 ni un C3: CPU Xtensa LX6 dual-core -el S3 es LX7, el C3 y el C6 son RISC-V-, y
// Bluetooth v4.2 "BR/EDR and BLE". BR/EDR es Bluetooth Clasico, que es exactamente el
// perfil que abre el createRfcommSocketToServiceRecord de la app.
//
// Consecuencia: hay SPP, la app conecta sin tocar una linea, y el apartado 1 del
// Manual 10 -congelado por escrito, "No BLE. No Web Bluetooth"- queda INTACTO.
//
// POR QUE LA INTERFAZ ESTRECHA SE QUEDA AUNQUE YA NO HAYA BLOQUEO.
//
// Nacio como frontera para que la rama BLE, si tocaba, no obligara a reescribir el
// resto. Se conserva por una razon distinta y mejor: es lo que permite ejercer el
// puente SIN RADIO. Detras de estas cinco funciones se puede poner un doble en el PC y
// probar el bombeo, el validador y los topes de linea; con BluetoothSerial cableado
// dentro del bucle, la unica forma de probar el puente seria con un telefono en la mano.
//
// Y no es teorico: los cuatro arneses que compilan C++ real de este repositorio existen
// justo porque los modelos en Python no prueban el codigo. Esta frontera es lo que hace
// que el del ESP32 pueda existir algun dia.

#ifndef TRANSPORTE_APP_H
#define TRANSPORTE_APP_H

#include <Arduino.h>

// Abre el perfil SPP con el rotulo aprendido -o el provisional-. Devuelve false si el
// perfil no arranco: ese caso no se disimula, porque un puente sin radio es un equipo
// sin superficie de mando y el operario tiene derecho a saberlo por el LED, no por
// deduccion.
bool transporte_abrir();

// true mientras haya un telefono conectado. NO es lo mismo que "el perfil esta
// abierto": un SPP levantado sin nadie al otro lado esta sano y mudo.
bool transporte_conectado();

int transporte_disponible();
int transporte_leer();

// Devuelve los bytes entregados. Menos de n significa buffer lleno, y el que llama
// tiene que contarlo (P-3): lo que se descarta callando se lee como que nunca existio.
size_t transporte_escribir(const char* datos, size_t n);

// APRENDE EL ROTULO DE UNA TRAMA QUE YA VA DE CAMINO, SIN TOCARLA.
//
// La serie sale del silicio del STM32; el ESP32 no puede saberla al arrancar. Se lee
// de los campos NODE: y SERIE: del $STATUS que el puente ya esta retransmitiendo y se
// guarda para el ARRANQUE SIGUIENTE -renombrar en caliente obligaria a cerrar el
// perfil, o sea a tirar la sesion del operario que esta conectado ahora mismo-.
//
// Esta funcion NO altera la trama, NO decide si se retransmite y NO puede hacer que
// una trama valida se descarte. Es un observador.
void transporte_aprenderRotulo(const char* trama);

const char* transporte_rotulo();

#endif // TRANSPORTE_APP_H
