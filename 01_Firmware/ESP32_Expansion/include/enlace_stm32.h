// ===== 01_Firmware/ESP32_Expansion/include/enlace_stm32.h =====
//
// LA UNICA PUERTA HACIA EL STM32. Nadie mas toca Serial2.
//
// Es la misma forma que la barrera de salidas del STM32 -solo semaforo.cpp escribe
// pines de luz-, y por la misma razon: una regla que hay que respetar en N sitios se
// rompe en el sitio N+1. Con una sola puerta, "el puente no origina" se puede
// COMPROBAR leyendo un fichero corto en vez de confiando en la disciplina.
//
// 🔴 B-1: CADA BYTE QUE SALE POR AQUI PROCEDE DEL BUFFER DE ENTRADA DE LA APP.
//
// Por eso enlace_escribirLinea() recibe un puntero y una longitud, y no hay ninguna
// version que reciba un literal. Un literal de comando en este fichero seria el puente
// mandando ordenes por su cuenta a un equipo que gobierna un cruce; que no se pueda
// escribir sin cambiar la firma es el punto.
//
// El pack esp32_05_no_origina lo vigila leyendo este fichero por texto.

#ifndef ENLACE_STM32_H
#define ENLACE_STM32_H

#include <Arduino.h>

// Abre UART2 con el baudio Y EL FORMATO explicitos.
//
// El 8N1 se escribe aunque sea el valor por defecto: en el STM32 esa eleccion es
// implicita -SerialBT.begin(9600) con un solo argumento- y no se puede leer de ningun
// sitio. Este literal es lo unico escrito que ata las dos puntas.
void enlace_setup();

int enlace_disponible();
int enlace_leer();

// Escribe una linea ENTERA hacia el STM32 en UNA sola llamada, anadiendo el
// terminador que el bucle receptor de la otra punta exige.
//
// B-5: una trama entra entera y sale entera. Partirla en dos escrituras no es un
// detalle de eficiencia: el receptor delimita por '\r' o '\n' (E-1), asi que una trama
// partida por el puente se entrega como DOS lineas y las dos son basura -y la primera
// puede casar por accidente con un comando mas corto-.
//
// E-1: el terminador lo pone esta funcion porque SIN EL el despachador del STM32 no
// dispara NUNCA: la linea se queda en btBufIn, muda, y el siguiente comando se pega
// detras. Lo que NO se hace es anadir terminadores a un byte suelto que llego sin el:
// eso seria compensar una propiedad del STM32 que el puente no debe compensar (6.4).
//
// Devuelve los bytes puestos en el cable, o 0 si no cabia o el enlace no esta abierto.
size_t enlace_escribirLinea(const char* datos, size_t n);

// P-3: ni un byte se descarta en silencio. Lo que se tira se cuenta y se puede leer.
unsigned long enlace_bytesEscritos();
unsigned long enlace_lineasEscritas();
unsigned long enlace_lineasRechazadas();

#endif // ENLACE_STM32_H
