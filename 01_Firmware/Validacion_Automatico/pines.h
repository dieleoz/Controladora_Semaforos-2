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

// LOS CUATRO PINES DE J16, desde que botones.cpp REAL se compila aqui.
//
// Los numeros no son los del micro -aqui un pin es un indice de arnes_pines[]-, pero LA
// SEPARACION SI IMPORTA: J14 (CAM_DEMANDA_PIN) y las dos camaras de J16 tienen que ser
// pines DISTINTOS, porque toda la pregunta que este arnes contesta es si una deteccion en
// J16 llega al Modo Inteligente, que lee J14. Con un solo pin compartido la respuesta
// saldria que si por construccion, y eso seria una tapia (CLAUDE.md 8.sexies).
#define BOTON1           11
#define BOTON2           12
#define CAM_C_PIN        13
#define CAM_D_PIN        14

// SFTY-28. La pluma entra en el arnes por la misma puerta que las luces, asi que su
// nivel queda grabado en arnes_pines[] como cualquier lampara: el invariante de abajo
// la mide sobre lo que semaforo.cpp ESCRIBIO, no sobre lo que la logica pretendia.
#define MOTOR_TALANQUERA  10
#define TALANQUERA_ABRIR  HIGH
#define TALANQUERA_CERRAR LOW
