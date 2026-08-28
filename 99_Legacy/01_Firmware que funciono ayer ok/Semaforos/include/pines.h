// ===== include/pines.h =====
#pragma once

// --- Semáforo 1 (vehicular principal) ---
#define ROJO1       PA0   // S1 -> J3
#define AMARILLO1   PA1   // S2 -> J4
#define VERDE1      PA2   // S3 -> J5

// --- Semáforo 2 (vehicular secundario) ---
#define ROJO2       PA3   // S4 -> J6
#define AMARILLO2   PA4   // S5 -> J7
#define VERDE2      PA5   // S6 -> J8

// --- Semáforo peatonal ---
#define ROJO_PEATON   PA6 // S7 -> J11
#define VERDE_PEATON  PA7 // S8 -> J9

// --- Actuadores ---
#define LORA_DE_RE        PB12 // Control DE/~RE del MAX3485 (bus OUT) -> E90-DTU LoRa
                                // CORREGIDO: la PCB real tiene DE/~RE en PB12, no en PB0.
                                // PB0 está en el net "/Puerta", sin relación con el radio.
#define BUZZER             PB1  // J13 (19)
#define MOTOR_TALANQUERA   PB2  // J15 (20)

// --- LCD ST7920 (modo serial, 3 hilos + PSB + RST) ---
#define LCD_SCLK    PB3   // -> E del LCD  (clock serial)
#define LCD_CS      PB4   // -> RS del LCD (chip select)
#define LCD_SID     PB5   // -> RW del LCD (dato serial)
#define LCD_PSB     PB6   // -> PSB del LCD (fijo LOW)
#define LCD_RST     PB7   // -> RST del LCD (reset)

// --- Botones ---
#define BOTON1      PB9   // Arriba
#define BOTON2      PB13  // Abajo
#define BOTON3      PB14  // Aceptar
#define BOTON4      PB15  // Cancelar

// --- RS485 "IN" (USART1) ---
#define RS485_IN_RX     PA10
#define RS485_IN_TX     PA9
#define RS485_IN_DE_RE  PA8

// --- RS485 "OUT" (USART3) ---
#define RS485_OUT_RX    PB11
#define RS485_OUT_TX    PB10
// DE/RE is controlled by LORA_DE_RE (PB12) as defined above.