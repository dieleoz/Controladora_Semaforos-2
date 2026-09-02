// ===== Validacion_Automatico/dos_puntas/comun/pines.h =====
//
// Sustituto de pines.h para compilar semaforo.cpp -y el bucle del Esclavo- en el PC.
//
// ES UNO SOLO PARA LAS DOS PUNTAS, y no por pereza: los dos pines.h reales asignan los
// MISMOS puertos a las mismas luces, y esa igualdad es un requisito, no una casualidad.
// El orquestador la RECOMPRUEBA leyendo los dos ficheros reales antes de medir nada, y
// aborta si divergen; asi este fichero no puede esconder una divergencia de mapeo.
//
// N-96: SEIS PINES DE LUZ, NO OCHO. ROJO_PEATON, VERDE_PEATON y BUZZER estan
// declarados en los pines.h reales y MUERTOS en las dos puntas -sin pinMode, sin
// digitalWrite-. Se declaran aqui con numero propio EXACTAMENTE para poder demostrarlo:
// el orquestador exige que sigan sin tocarse. Una regla de seguridad que enumera
// sujetos tiene que comprobar que cada sujeto existe.
#pragma once

#define ROJO1             0
#define AMARILLO1         1
#define VERDE1            2
#define ROJO2             3
#define AMARILLO2         4
#define VERDE2            5

// Los tres muertos de N-96. Aqui para poder vigilarlos, no para usarlos.
#define ROJO_PEATON       6
#define VERDE_PEATON      7
#define BUZZER            11

#define CAM_DEMANDA_PIN   8
#define LED_TESTIGO       9

// SFTY-28. La pluma entra por la misma puerta que las luces y queda grabada en
// arnes_pines[] como cualquier lampara.
#define MOTOR_TALANQUERA  10
#define TALANQUERA_ABRIR  HIGH
#define TALANQUERA_CERRAR LOW

// El Esclavo declara ademas el control DE/~RE del MAX3485. No lo toca nada de lo que
// aqui se compila -protocolo.cpp no entra-, pero el numero tiene que existir.
#define LORA_DE_RE        12
