// ===== Validacion_Automatico/pines.h =====
// Sustituto de pines.h para compilar semaforo.cpp en el PC.
#pragma once

#define ROJO1             0
#define AMARILLO1         1
#define VERDE1            2
#define ROJO2             3
#define AMARILLO2         4
#define VERDE2            5
#define ROJO_PEATON       6
#define VERDE_PEATON      7
#define CAM_DEMANDA_PIN   8
#define LED_TESTIGO       9

// SFTY-28. La pluma entra en el arnes por la misma puerta que las luces, asi que su
// nivel queda grabado en arnes_pines[] como cualquier lampara: el invariante de abajo
// la mide sobre lo que semaforo.cpp ESCRIBIO, no sobre lo que la logica pretendia.
#define MOTOR_TALANQUERA  10
#define TALANQUERA_ABRIR  HIGH
#define TALANQUERA_CERRAR LOW

// --- Anadidos de este arnes sobre el sustituto heredado de Validacion_Automatico ---
//
// bluetooth.cpp REAL toca RS485_IN_DE_RE en bluetooth_setup(): pone PA8 en HIGH para
// apagar el receptor de U2. Aqui no hay MAX3485, pero el pin tiene que EXISTIR o el
// fuente no compila, y su escritura queda grabada en arnes_pines[] como cualquier
// otra: asi el arnes puede comprobar que el setup real lo dejo donde lo deja el
// firmware, en vez de dar por hecho que lo hace.
#define RS485_IN_DE_RE    11

// PB6 / PB7 son los del conector J17 por donde va el ESP32 (USART1 remapeado, N-76).
// bluetooth.cpp los usa como argumentos del constructor de HardwareSerial, asi que
// aqui tienen que ser numeros: el arnes elige su puerto por ESTOS valores, no por el
// nombre de la variable, que es static y no se puede alcanzar desde fuera.
#define PB7   12   // RX del micro <- TX del ESP32 (J17 p2)
#define PB6   13   // TX del micro -> RX del ESP32 (J17 p3)
