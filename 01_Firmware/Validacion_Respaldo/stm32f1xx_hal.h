// ===== Validacion_Respaldo/stm32f1xx_hal.h =====
//
// SUSTITUTO MINIMO del HAL de ST para compilar respaldo.cpp EN EL PC.
//
// No emula el STM32: solo cubre exactamente los cuatro simbolos que respaldo.cpp
// usa del HAL, y sustituye los diez registros de respaldo del dominio VBAT
// (BKP->DR1..DR10) por un array normal de memoria.
//
// El sentido de esto es N-29: dejar de mantener un modelo Python de
// calcularSuma(). El modelo ya divergio dos veces en un solo dia -suma llana,
// pesos ponderados, hash de Horner- y cada divergencia produjo un veredicto
// falso: primero un PASS sobre un algoritmo que ya no existia, despues un FALLA
// sobre un algoritmo que si funciona. Un modelo que hay que reescribir cada vez
// que cambia el firmware no vigila el firmware: lo persigue.
//
// La alternativa es la que ya esta resuelta en Validacion_LCD: NO SE MODELA, SE
// COMPILA EL FUENTE REAL. calcularSuma() es una funcion pura -lee cinco enteros y
// devuelve otro- sin nada de hardware dentro, asi que el unico obstaculo para
// correrla en el PC es el include del HAL. Este fichero lo quita.
//
// Se llama igual que la cabecera de ST a proposito: respaldo.cpp incluye
// <stm32f1xx_hal.h> y aqui NO se toca esa linea. El -I de este directorio va
// primero y gana. respaldo.cpp se compila LETRA POR LETRA como va a la tarjeta.
#pragma once

#include <stdint.h>

// Los diez registros de respaldo. En el silicio son BKP->DR1..DR10, de 16 bits
// utiles dentro de palabras de 32. respaldo.cpp los recorre con
// &(&BKP->DR1)[n-1], asi que lo unico que hace falta es que DR1..DR10 sean
// uint32_t CONSECUTIVOS en memoria: un array lo garantiza igual que la struct.
typedef struct {
  volatile uint32_t DR1, DR2, DR3, DR4, DR5;
  volatile uint32_t DR6, DR7, DR8, DR9, DR10;
} BKP_Simulado;

extern BKP_Simulado arnes_bkp;

// respaldo.cpp escribe "BKP->DRn". Con esto, BKP es un puntero al array de arriba.
#define BKP (&arnes_bkp)

// La proteccion de escritura del dominio de respaldo no tiene equivalente en el
// PC. Se dejan como no-op: lo que se valida es la ARITMETICA de calcularSuma() y
// la logica de fechado, no el arbitrado del bus de alimentacion.
static inline void HAL_PWR_EnableBkUpAccess(void) {}
static inline void HAL_PWR_DisableBkUpAccess(void) {}

#define __HAL_RCC_PWR_CLK_ENABLE() ((void)0)
#define __HAL_RCC_BKP_CLK_ENABLE() ((void)0)
