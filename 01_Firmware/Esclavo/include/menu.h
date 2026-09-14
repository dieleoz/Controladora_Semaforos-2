// ===== include/menu.h (ESCLAVO) =====
#pragma once
#include <Arduino.h>

// ---------------------------------------------------------------------------
// D-32 (1), 13/09/2026 — SALE EL LCD DEL FIRMWARE.
//
// D-17.bis (28/08) habia retirado la pantalla DEL EQUIPO y conservado el codigo a
// proposito; D-30 propuso llevarse ademas el mando, y D-32 (1) recorta esa decision a
// una sola mitad: sale el LCD y el mando A/B/C/D SE QUEDA. Asi que de la interfaz del
// Esclavo ya no queda nada: ni pantalla fisica, ni framebuffer, ni navegacion.
//
// LO QUE ESTA CABECERA DECLARABA Y YA NO EXISTE: un menu de dos entradas -ESTADO y
// MODO DEGRADADO- con su cursor, su confirmacion de entrada al Degradado y su regreso
// automatico al listado. El censo de por que se puede retirar sin dejar ningun camino
// de gobierno sin puerta esta en menu.cpp, symbol a symbol.
//
// LO QUE SIGUE SIENDO CIERTO Y NO SE TOCA: EL ESCLAVO NO OFRECE MODOS DE OPERACION, Y
// NO ES UN OLVIDO. Quien decide el ciclo es el Maestro. Dar a las dos puntas la
// capacidad de decidir a la vez deja el cruce con dos cerebros discutiendo por radio
// quien tiene razon. Entrar y salir del Modo Degradado en esta punta se pide hoy por
// la app (D-18) o por las secuencias del mando de reles, y las dos vias pasan por
// modo_degradado.cpp, que es quien lleva las condiciones y las transiciones.
// ---------------------------------------------------------------------------

// Las dos se conservan porque main.cpp las llama -menu_setup() al terminar la ventana
// de bienvenida, menu_loop() en cada vuelta- y porque son el hueco donde volveria a
// montarse una interfaz. Hoy no hacen nada; el motivo esta escrito en menu.cpp.
void menu_setup();
void menu_loop();

// ---------------------------------------------------------------------------
// SFTY-21 — LA UNICA DE LAS TRES QUE GOBIERNA ALGO. Lo consulta mando.cpp, en
// secuenciasInhibidas(), para decidir si reconoce o no las secuencias del mando de
// reles: con true el mando queda inhibido, con false queda armado.
//
// DEVUELVE SIEMPRE false DESDE D-32 (1), Y ESO NO ABRE NINGUN VETO. Medido antes de
// tocarlo, que es lo que CLAUDE.md 6.2 exige de un armador que se retira:
//
//   El armador era "hay una pantalla abierta por debajo del listado inicial". Para
//   bajar del listado hacia falta botonAceptar(), que es `return false;` en
//   botones.cpp desde el 31/08 (D-2: sus pines son camaras). O sea que la bandera ya
//   valia false en todas las vueltas del bucle desde entonces, y botones.cpp lo dice
//   por escrito: "con ACEPTAR mudo, la pantalla del Esclavo no puede bajar del
//   listado, asi que menu_estaAbierto() es siempre falso". El comportamiento del
//   equipo no cambia hoy; lo que cambia es que la propiedad pasa de ser de ALCANCE a
//   estar escrita en el fuente.
//
//   Y ES LA RESPUESTA CORRECTA, no un residuo. El veto protegia de un caso concreto:
//   que una rafaga de pulsos a ciegas cayera sobre un cursor capaz de CONFIRMAR algo
//   mientras hay una persona delante del gabinete. Sin pantalla ese caso no existe, e
//   inhibir el mando de todas formas seria dar por presente a un operario que no esta
//   -y en esta punta el mando es la unica via de entrar o salir del Degradado sin la
//   app-.
//
// LO QUE ESTO NO ARREGLA, y se dice para que nadie lo lea como cerrado: J16 p5/p8
// siguen VACIOS Y PELADOS y el mando sigue armado sobre ellos, asi que un puente ahi
// compone secuencias sin que nadie lo pida. Es lo mismo que antes de este commit
// -D-32 lo deja escrito- y no lo cierra el firmware: lo cierra la instruccion de no
// cablearlos.
// ---------------------------------------------------------------------------
bool menu_estaAbierto();
