#pragma once
#include <Arduino.h>

// ---------------------------------------------------------------------------
// D-106..D-111 (SPEC_0 5.10) - EL MODO NOCHE DEL MAESTRO.
//
// En los dias y franjas programados las dos puntas destellan: ambar en los dos o, elegido,
// Poste 2 en rojo intermitente. El Maestro guarda las franjas (noche_flash) y decide con SU
// hora; el Esclavo obedece CMD_GO_NOCHE (D-107).
//
// ENTRADA (D-111): por el FLANCO del minuto de inicio, con hora fiable y desde AUTO o
// INTELIGENTE (otro modo: FRANJA_SALTADA; un corte no reanuda). Por la puerta de
// SET_MODO:MANUAL -coordinador_forzarRojoTotal()-, el ACK_RED del Esclavo y tr segundos de
// rojo en los dos; despues coordinador_entrarNoche(). Sin ACK_RED no entra: manda SFTY-6.
// SALIDA (D-106, D-111): por el flanco de fin o desde la app, SIEMPRE por el todo-rojo (rojo
// en los dos, ACK_RED, tr) y despues el destino; a MENU, ALCANCE y FORZAR_ROJO, rojo e
// inmediata (esos destinos no dan verde). Durante toda la noche el coordinador tiene el veto
// de verde puesto (coordinador_vetarVerde).
// ---------------------------------------------------------------------------

static const uint16_t NOCHE_TR_FABRICA_S = 180;   // D-106: 3 min de fabrica, no un valor quemado
static const uint16_t NOCHE_TR_MAX_S = 900;       // D-109: VERDE_MIN_MAX en segundos (static_assert en el .cpp)
static const unsigned long NOCHE_AVISO_HORA_MS = 60000UL;   // = AVISO_ROJO_SIN_HORA_MS (D-67)

void modoNoche_cargar();     // setup(): lee las franjas de la flash (o las de fabrica)
void modoNoche_vigilar();    // cada vuelta, en cualquier modo: los flancos de inicio y fin
void modoNoche_setup();      // main.cpp, al entrar en MODO_NOCHE
void modoNoche_loop();
void modoNoche_alSalir();    // main.cpp, al dejar MODO_NOCHE por cualquier camino

// Las ordenes de noche (SET_NOCHE, SET_NOCHE_TR, CONSULTA_NOCHE) y, dentro de la noche, las
// salidas por la app. true si la orden quedo contestada aqui; false si sigue el despachador.
bool modoNoche_orden(const char* cmd, void (*responder)(const char*));
