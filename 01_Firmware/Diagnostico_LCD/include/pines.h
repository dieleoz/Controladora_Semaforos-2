// ===== Diagnostico_LCD/include/pines.h =====
//
// COPIA DELIBERADA Y RECORTADA de 01_Firmware/Esclavo/include/pines.h.
//
// Este proyecto existe para quitar variables de en medio, asi que no incluye la
// cabecera del Esclavo ni depende de ella: si alguien tocara aquella mientras se
// diagnostica, el diagnostico cambiaria bajo los pies del operario sin que nadie
// lo notara. Aqui los numeros estan ESCRITOS, a la vista, para poder contrastarlos
// con la tarjeta y un multimetro.
//
// Los cinco pines de la LCD son los mismos en las DOS puntas (Maestro y Esclavo);
// eso ya esta comprobado byte a byte y NO es la causa de N-22. Se repiten aqui solo
// para que este proyecto se pueda leer entero sin abrir otro.
#pragma once

// --- LCD ST7920 (modo serie: 3 hilos + PSB + RST) --------------------------
#define LCD_SCLK    PB3   // -> E   del LCD (reloj serie)
#define LCD_CS      PB4   // -> RS  del LCD (chip select; en serie es CS, activo ALTO)
#define LCD_SID     PB5   // -> RW  del LCD (dato serie)
#define LCD_PSB     PB6   // -> PSB del LCD (LOW = serie, HIGH = paralelo)
#define LCD_RST     PB7   // -> RST del LCD (reset, activo BAJO)

// --- Testigo de vida -------------------------------------------------------
// Se usa la salida del ROJO del semaforo 1. No es un LED de la placa: es la salida
// a rele/optoacoplador que ya esta cableada y que el operario puede ver o medir.
// Si no hay nada conectado a J3, mida PA0 con el multimetro en continuidad de
// tension: debe alternar entre 0 V y 3,3 V.
#define TESTIGO     PA0   // ROJO1 -> J3
