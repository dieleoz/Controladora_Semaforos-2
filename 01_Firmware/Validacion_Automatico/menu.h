// ===== Validacion_Automatico/menu.h =====
// Sustituto de menu.h para compilar modo_automatico.cpp Y mando.cpp en el PC.
//
// EXTENDIDO PARA N-52: hasta ahora este sustituto solo daba MENU, porque
// modo_automatico.cpp no necesitaba mas. mando.cpp si necesita el enum COMPLETO
// -secuenciasInhibidas() compara contra MODO_HORA, ejecutar() contra
// MODO_AUTOMATICO, MODO_AMBAR y MODO_DEGRADADO- y necesita modoActual_get(), que
// antes ni se declaraba aqui.
//
// MISMOS NOMBRES Y MISMO ORDEN que include/menu.h real. El orden no lo compara
// nadie por entero en este arnes, pero divergir de el es exactamente el tipo de
// "casi igual" que un sustituto no puede permitirse: si algun dia alguien
// compara valores crudos, un enum reordenado mentiria en silencio.
#pragma once

enum ModoSistema {
  MENU,
  MODO_MANUAL,
  MODO_AUTOMATICO,
  MODO_INTELIGENTE,
  MODO_ALCANCE,
  MODO_HORA,
  MODO_DEGRADADO,
  MODO_AMBAR
};

void menu_setup();
ModoSistema modoActual_get();
void modoActual_set(ModoSistema m);
