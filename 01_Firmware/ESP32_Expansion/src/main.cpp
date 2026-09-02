// ===========================================================================
// ESP32 DE EXPANSION  —  puente Bluetooth <-> J17  +  reloj DS3231
// ===========================================================================
//
// LA FRASE QUE GOBIERNA TODO EL DISENO:
//
//   El ESP32 es la superficie de mando, y NO es parte del lazo de seguridad.
//
// Las dos mitades importan por separado, y confundirlas es como se cuelan las
// propuestas que hay que rechazar:
//
//   - ES LA SUPERFICIE DE MANDO porque con la pantalla, los cuatro pulsadores y el
//     mando de reles retirados, TODA la operacion pasa por la app y la app pasa por
//     aqui. Un ESP32 colgado deja el equipo seguro pero NO OPERABLE.
//   - NO ES PARTE DEL LAZO DE SEGURIDAD porque el ciclo, el enclavamiento SFTY-2, el
//     todo-rojo y la caida a ambar de SFTY-6 viven enteros en el STM32 y no leen ni un
//     byte de este modulo.
//
// De ahi la regla de rechazo: cualquier propuesta que haga que el semaforo dependa del
// ESP32 para seguir siendo SEGURO se rechaza, por comoda que sea.
//
// 🔴 Y LO QUE NADIE VIGILA, QUE HAY QUE SABER ANTES DE FIARSE DE ESTE FIRMWARE:
//
// SFTY-6 mira la RADIO LoRa, no J17 -coordinador.cpp:656 usa tUltimaRxEsclavo y
// Esclavo/main.cpp:555 usa tUltimoComando; ninguna lee un byte de SerialBT-. Un ESP32
// colgado NO dispara SFTY-6: no dispara NADA. El STM32 sigue ciclando tan tranquilo y
// el unico testigo es la app, a 5 s. Eso esta abierto como AB-1 y es del responsable:
// no se cierra desde aqui.
// ===========================================================================

#include <Arduino.h>
#include "contrato.h"
#include "vigilante.h"
#include "enlace_stm32.h"
#include "transporte_app.h"
#include "reloj_ds3231.h"
#include "puente.h"

void setup() {
  // EL CENSO ANTES QUE EL PERRO, Y ES LA MITAD QUE FALTABA.
  //
  // Un puente que revive en silencio esconde el fallo que hay que contar: desde la app
  // y desde el STM32, un modulo que se reinicia cada dos segundos y uno sano son
  // INDISTINGUIBLES si el reinicio es rapido. Esto lee del chip por que arrancamos y
  // suma uno a la cuenta que vive en memoria RTC.
  //
  // Va antes de armar porque mira al PASADO -por que estamos aqui- y el armado al
  // FUTURO. Separadas, el censo corre aunque el armado falle, que es justo el modulo
  // que mas falta hace contar. NO EMITE NADA: setup() no pone un byte en ningun cable.
  vigilante_censarArranque();

  // W-5: EL PERRO PRIMERO. Antes del SPP y antes de tocar el I2C.
  //
  // El orden no es estetico: si el DS3231 cuelga el bus en el arranque -un SDA en corto
  // basta-, tiene que haber ya quien reinicie. Un watchdog armado despues del I2C no
  // veria nunca ese caso, que es justo uno de los que justifica tenerlo. Es la misma
  // razon por la que el Maestro arma el suyo ANTES de rtc.begin() (main.cpp:53 y :57).
  vigilante_armar();

  // El puente no manda nada al abrir, en ninguno de los dos sentidos.
  enlace_setup();
  reloj_setup();
  transporte_abrir();
  puente_setup();
}

void loop() {
  // Una vuelta = las dos direcciones + el reloj.
  puente_bombear();
  reloj_revisar();

  // EL PARTE DE ARRANQUE SALE AQUI, NO EN setup(), Y LAS DOS RAZONES SON DISTINTAS.
  //
  // La primera es 6.4: setup() no puede emitir. Un saludo del puente es una orden que
  // nadie pidio entrando por el mismo camino que las que si se piden.
  // La segunda es que en setup() NO HABRIA NADIE ESCUCHANDO -transporte_escribir()
  // devuelve 0 sin telefono conectado-, asi que el parte se perderia justo la vez que
  // hace falta. Esta funcion espera al enlace y se rearma cuando cae.
  vigilante_declarar();

  // W-2: el reset, UNA VEZ POR VUELTA DEL BUCLE EXTERIOR y aqui abajo, despues de haber
  // atendido las dos direcciones.
  //
  // 🔴 W-3: NO va dentro de ninguno de los while interiores de puente.cpp, y esa es la
  // linea entre un watchdog y un adorno. El fallo del 31/07/2026 fue exactamente eso:
  // con ruido continuo el bucle interior nunca terminaba. Un reset ahi dentro
  // alimentaria al perro para siempre mientras el puente no progresa -un watchdog que
  // un flujo de basura mantiene contento no vigila nada-.
  vigilante_alimentar();
}
