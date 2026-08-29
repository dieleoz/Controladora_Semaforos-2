// ===== include/modos.h =====
//
// EL MODO DEL EQUIPO NO ES UN ASUNTO DE LA PANTALLA.
//
// Este enum y sus dos accesores vivian dentro de menu.h/menu.cpp, y eso hacia que el
// fichero de la pantalla fuese tambien el dueno de la maquina de estados del sistema:
// main.cpp despacha sobre el, mando.cpp decide con el si inhibe las secuencias del
// suelo y bluetooth.cpp lo escribe desde los SET_MODO. Ninguno de esos tres dibuja
// nada. Mientras el modo colgase de menu.h, retirar o rehacer la interfaz arrastraba
// consigo la maquina de estados, que es exactamente la clase de acoplamiento que
// convierte un cambio de presentacion en un cambio de comportamiento.
//
// Aqui no hay logica: solo el conjunto de estados y la pareja de accesores. La
// implementacion esta en src/modos.cpp, tambien fuera de la pantalla.
#pragma once

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

ModoSistema modoActual_get();
void modoActual_set(ModoSistema m);
