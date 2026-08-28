// ===== include/menu.h =====
#pragma once
#include <Arduino.h>

enum ModoSistema {
  MENU,
  MODO_MANUAL,
  MODO_AUTOMATICO,
  MODO_INTELIGENTE,
  MODO_ALCANCE,
  MODO_HORA,       // SFTY-18: ajuste del reloj. No arranca ciclos.
  MODO_DEGRADADO,  // SFTY-21: operacion por reloj, sin radio. Activacion MANUAL.

  // SFTY-21: ambar intermitente pedido a proposito, no por fallo.
  // Es el destino de la secuencia B.B.B del mando de reles y el estado al que cae
  // solo el Modo Degradado al agotarse el limite de 48 h. No aparece en el menu: no
  // es un modo que se "elija" desde la pantalla, es una salida de emergencia.
  MODO_AMBAR
};

void menu_setup();
void menu_loop();

ModoSistema modoActual_get();
void modoActual_set(ModoSistema m);
