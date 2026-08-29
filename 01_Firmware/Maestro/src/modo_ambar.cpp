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
#include "lcd.h"
#include "menu.h"
#include "modos.h"
#include "botones.h"

// Motivo con el que arranca el ambar. Apunta a literales, nunca a memoria temporal.
static const char* ambarL1 = "Ambar intermitente";
static const char* ambarL2 = "";

void modo_ambar_fijarMotivo(const char* linea1, const char* linea2) {
  ambarL1 = linea1;
  ambarL2 = linea2;
}

void modo_ambar_setup() {
  // Ciclo detenido y orden de rojo al Esclavo mientras el radio aun sirva. Despues el
  // Maestro calla -main.cpp no llama al coordinador en este modo-, y el Esclavo pasa
  // a su propio ambar por orfandad (SFTY-6). Se usa el mecanismo que ya
  // existe y esta probado en campo en vez de inventar una orden nueva de "pon ambar".
  coordinador_forzarRojoTotal();
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
