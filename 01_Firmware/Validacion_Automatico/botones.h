// ===== Validacion_Automatico/botones.h =====
//
// ESTE FICHERO YA NO ES UN SUSTITUTO: REENVIA AL HEADER REAL DEL MAESTRO.
//
// Hasta el 05/09 declaraba a mano las firmas de botones.h "para que modo_automatico.cpp
// compile letra por letra", y las definiciones las ponia arnes_automatico.cpp con bools
// sueltos. O sea que botones.cpp NO SE COMPILABA EN NINGUN SITIO: ni aqui, ni en ningun
// otro arnes. Todo lo que ese fichero hace -el vigilante de camaras entero, la lectura de
// J16, la siembra de N-26- estaba medido UNICAMENTE por packs que leen texto, y un pack
// de texto no puede ver a las 6 h una alarma que no debia salir.
//
// Se reenvia en vez de copiarse por la misma razon que el resto del arnes usa los .h
// reales: una copia de firmas escrita a mano es una segunda declaracion que alguien tiene
// que mantener igual, y el dia que diverja el arnes compilaria contra un botones.h que ya
// no es el del firmware. La ruta es explicita -no "botones.h"- porque -I de este
// directorio va primero y un include simple se incluiria a si mismo.
#pragma once

#include "../Maestro/include/botones.h"
