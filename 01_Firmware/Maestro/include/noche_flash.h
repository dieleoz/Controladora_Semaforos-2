#pragma once
#include <Arduino.h>

// ---------------------------------------------------------------------------
// D-108 (SPEC_0 5.10 (c)) - LAS FRANJAS DEL MODO NOCHE, EN UNA PAGINA DE FLASH PROPIA.
//
// Por que aqui y no en la pila: BKP->DR1..DR10 estan todos ocupados (respaldo.cpp) y una
// franja son ~40 bits. Molde testigo_flash: firma, version, suma de Horner sin plegar y
// relectura. La pagina es la ANTERIOR a la del testigo; board_upload.maximum_size = 63488
// en platformio.ini deja las dos fuera del alcance del enlazador: si el codigo llegara hasta
// aqui, el enlace falla en vez de pisarla. Se escribe solo al cambiar una franja o el tr.
// `-e all` del upload (CLAUDE.md 3) la borra al cargar: tras una carga no hay franjas.
//
// Solo el Maestro la tiene: el Esclavo no guarda nada de la noche (D-107).
// ---------------------------------------------------------------------------

static const uint32_t NOCHE_FLASH_DIR = 0x0800F800UL;   // penultima pagina de 1 KB del mapa de 64 KB
static const uint8_t NOCHE_FRANJAS = 4;

struct FranjaNoche {
  uint8_t dias;      // bit0 = lunes .. bit6 = domingo (LMXJVSD); 0 = apagada
  uint16_t inicio;   // minuto del dia, 0..1439
  uint16_t fin;      // minuto del dia; fin <= inicio cruza la medianoche; inicio == fin = apagada
  bool rojo;         // true: Poste 2 en rojo intermitente; false: ambar en los dos (por defecto)
};

struct NocheConfig {
  uint16_t trSeg;    // D-109: el todo-rojo de entrada y salida, DESPEJE_SEG_MIN..NOCHE_TR_MAX_S
  FranjaNoche f[NOCHE_FRANJAS];
};

// true si la pagina tiene un registro integro (firma, version, suma y rangos). Con false, *c
// no se toca.
bool nocheFlash_leer(NocheConfig* c);

// Borra la pagina, programa el registro y lo relee. true solo si lo releido es lo pedido.
bool nocheFlash_escribir(const NocheConfig* c);
