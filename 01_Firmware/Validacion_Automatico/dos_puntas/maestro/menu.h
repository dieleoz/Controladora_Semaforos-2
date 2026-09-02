// ===== Validacion_Automatico/menu.h =====
// Sustituto de menu.h para compilar modo_automatico.cpp Y mando.cpp en el PC.
//
// YA NO COPIA EL ENUM, Y ESO ES UNA MEJORA, NO UN DESCUIDO.
//
// Hasta el 28/08 este sustituto redeclaraba ModoSistema entero "con los mismos
// nombres y el mismo orden que include/menu.h real", con una nota admitiendo que
// divergir seria un "casi igual" capaz de mentir en silencio. Esa copia ya no hace
// falta: el enum salio de la cabecera de la pantalla a modos.h, que NO arrastra ni
// STM32duino ni U8g2 -no incluye nada-, asi que aqui se toma el REAL. Un sustituto
// que no puede desincronizarse es mejor que uno vigilado a mano.
//
// Lo unico que sigue siendo sustituto es menu_setup(): modo_automatico.cpp lo llama
// al volver al menu y el arnes lo implementa como no-op, porque aqui no hay pantalla.
#pragma once

#include "modos.h"   // el de Maestro\include: -I lo resuelve, no hay copia local

void menu_setup();
