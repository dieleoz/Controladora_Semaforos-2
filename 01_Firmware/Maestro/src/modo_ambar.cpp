// ===== src/modo_ambar.cpp (MAESTRO) =====
//
// FASE 4 DEL PLAN DE ARQUITECTURA — MOVIMIENTO PURO, SIN CAMBIO DE COMPORTAMIENTO.
//
// Salido de modo_degradado.cpp. El porque esta en modo_ambar.h: MODO_AMBAR es un modo
// del sistema -la salida de emergencia de B.B.B- y DEG_AMBAR es un estado interno de
// la maquina del Degradado. Comparten la pantalla y las dos lineas de motivo, nada mas.
//
// QUE ESTUVIERA ENTERRADO IMPORTABA: es un CAMINO DE SEGURIDAD, y revisarlo obligaba a
// leer 612 lineas de logica de reloj de las que no depende en absoluto.

#include <Arduino.h>
#include "modo_ambar.h"
#include "semaforo.h"
#include "coordinador.h"
#include "protocolo.h"   // N-134: la orden de ambar al Esclavo
#include "lcd.h"
#include "menu.h"
#include "modos.h"
#include "botones.h"

// Motivo con el que arranca el ambar. Apunta a literales, nunca a memoria temporal.
static const char* ambarL1 = "Ambar intermitente";
static const char* ambarL2 = "";

// N-152: de quien es el ambar vigente. Arranca en false -un equipo recien encendido no
// esta en un ambar que pidiera nadie- y lo apaga cualquier motivo local. El porque de
// que viva pegado al motivo, en modo_ambar.h.
static bool origenEsclavo = false;

void modo_ambar_fijarMotivo(const char* linea1, const char* linea2) {
  ambarL1 = linea1;
  ambarL2 = linea2;
  // N-152: quien fija motivo esta pidiendo el ambar DESDE ESTE POSTE. Apagar aqui es lo
  // que hace que la distincion no dependa de acordarse en cada uno de los sitios que
  // entran al modo: es la misma puerta por la que ya pasaban todos.
  origenEsclavo = false;
}

void modo_ambar_fijarMotivoDelEsclavo() {
  // El motivo se escribe aqui y no en main.cpp para que las dos lineas y el origen sean
  // el mismo gesto: no se puede declarar uno y olvidar el otro.
  ambarL1 = "Ambar pedido desde";
  ambarL2 = "el Poste 2 (radio)";
  origenEsclavo = true;
}

bool modo_ambar_origenEsclavo() { return origenEsclavo; }

void modo_ambar_setup() {
  // Ciclo detenido y orden de rojo al Esclavo mientras el radio aun sirva. Despues el
  // Maestro calla -main.cpp no llama al coordinador en este modo-, y el Esclavo pasa
  // a su propio ambar por orfandad (SFTY-6). Se usa el mecanismo que ya
  // existe y esta probado en campo en vez de inventar una orden nueva de "pon ambar".
  coordinador_forzarRojoTotal();

  // N-134 (04/09): Y SE LE ORDENA EL AMBAR, en vez de dejar que lo deduzca.
  //
  // El rojo de arriba SE QUEDA y es lo primero a proposito: es el intermedio seguro.
  // Si la orden de ambar se perdiera, el Esclavo queda PARADO -no dando paso- hasta
  // que la orfandad lo saque, y parado es la direccion segura.
  //
  // Antes solo iba el rojo y el ambar del Esclavo llegaba 25 s despues, por orfandad
  // (SFTY-6). El estado final era el correcto, pero nadie lo habia ordenado: era el
  // Esclavo rindiendose. En banco se vio como "a veces los dos pasan a ambar, a veces
  // solo el maestro" -segun cuanto mirase uno- y el operario pulsaba tres veces en
  // catorce segundos porque no veia cambiar la otra punta.
  //
  // LA ORFANDAD SE QUEDA COMO RED, decidido por el responsable el 04/09: si la radio se
  // cae justo en este instante, el Esclavo sigue yendo a ambar a los 25 s por su cuenta.
  // Se gana el caso bueno sin perder el malo.
  //
  // No se espera ACK: esta punta se calla a continuacion -main.cpp no llama al
  // coordinador en este modo- y quedarse esperando una respuesta que nadie va a atender
  // seria bloquear el arranque del modo por una trama de cortesia.
  protocolo_enviarPaquete(CMD_GO_AMBAR);

  semaforo_iniciarFallo();
  lcd_dibujarDegradadoAmbar(ambarL1, ambarL2);
}

void modo_ambar_loop() {
  semaforo_actualizar();

  if (botonCancelar()) {
    modoActual_set(MENU);
    menu_setup();
  }
}
