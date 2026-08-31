// ===== 01_Firmware/ESP32_Expansion/include/vigilante.h =====
//
// EL WATCHDOG. VA PRIMERO, Y NO ES UN ORDEN ARBITRARIO.
//
// Las otras dos funciones -el reloj y el puente- cuelgan de que este modulo siga vivo,
// y hoy nada garantiza que lo este:
//
//   MEDIDO  Maestro/src/main.cpp:53    IWatchdog.begin(4000000)  -> 4 s, con reload en :130
//   MEDIDO  Esclavo/src/main.cpp:238   IWatchdog.begin(4000000)  -> 4 s, con reload en :318
//   MEDIDO  Repetidor/src/  grep -rniE "watchdog|esp_task_wdt|WDT"  ->  CERO
//
// Los dos STM32 tienen perro. El ESP32 de este proyecto no tenia ninguno, y hay
// precedente escrito de uno de este mismo proyecto clavado tumbando el enlace
// (01_Firmware/TROUBLESHOOTING.md:48 y :55, 31/07/2026).
//
// Y LO QUE ESTO NO CUBRE, ESCRITO AL LADO PORQUE ES LA MITAD DEL PROBLEMA:
// un watchdog rescata al ESP32 COLGADO. No hace nada por uno MUERTO, desalimentado o
// desenchufado. Con la pantalla, los pulsadores y el mando retirados, eso deja el
// equipo seguro pero SIN NINGUNA SUPERFICIE DE MANDO. Sigue abierto como AB-2 y tiene
// dueno: no se cierra desde el firmware.

#ifndef VIGILANTE_H
#define VIGILANTE_H

#include <Arduino.h>

// W-5: se arma en setup(), ANTES de abrir el transporte de la app y ANTES de tocar el
// I2C. Si el DS3231 cuelga el bus en el arranque -un SDA en corto basta- tiene que
// haber ya quien reinicie; un watchdog que se armara despues del I2C no veria nunca
// ese caso, que es justo uno de los que justifica tenerlo.
void vigilante_armar();

// W-2: UNA VEZ POR VUELTA DEL BUCLE EXTERIOR, despues de haber atendido las dos
// direcciones.
//
// 🔴 W-3: NO se llama desde dentro del while interior de ningun sentido. Esa es la
// forma exacta del fallo del 31/07: con ruido continuo el bucle interior nunca termina,
// y un reset ahi dentro alimentaria al perro para siempre mientras el puente no
// progresa. Un watchdog que un flujo de basura mantiene contento no vigila nada.
void vigilante_alimentar();

// Para el diagnostico: si el registro fallo, el resto del firmware corre creyendose
// vigilado y no lo esta. Un esp_task_wdt_init() sin su reset en el sitio correcto es
// CAM_UMBRAL_PIN con otro nombre -un pinMode() sin digitalRead(), con documentacion
// encima-, y esta funcion existe para que ese caso se pueda ver en vez de suponerse.
bool vigilante_armado();

#endif // VIGILANTE_H
