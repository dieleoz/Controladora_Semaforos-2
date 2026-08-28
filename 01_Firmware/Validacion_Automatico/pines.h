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
