// ===== include/modo_ambar.h (MAESTRO) =====
#pragma once
#include <Arduino.h>

// ---------------------------------------------------------------------------
// AMBAR INTERMITENTE PEDIDO A PROPOSITO
//
// FASE 4 (03/08/2026): sale de modo_degradado.cpp, donde vivia por historia.
//
// EL MOTIVO POR EL QUE SE SEPARA NO ES EL TAMANO, SON DOS COSAS DISTINTAS.
//
//   MODO_AMBAR  es un MODO DEL SISTEMA. Se entra con B.B.B desde el mando y es la
//               salida de emergencia: funciona desde cualquier modo en marcha y SIN
//               CONDICIONES, porque una salida de emergencia con requisitos no es una
//               salida de emergencia.
//
//   DEG_AMBAR   es un ESTADO INTERNO de la maquina del Degradado, al que ese modo cae
//               solo cuando se declara insostenible -limite de 48 h agotado o reloj
//               perdido-.
//
// Comparten la pantalla y las dos lineas de motivo, nada mas. Que el destino de la
// salida de emergencia viviera dentro del fichero del Modo Degradado obligaba a leer
// 612 lineas de logica de reloj para revisar un camino de seguridad que no depende de
// ningun reloj.
//
// LO QUE EL COMENTARIO ANTERIOR DABA POR SUPUESTO Y NO ERA CIERTO: decia que separarlo
// "habria duplicado las mismas cuatro lineas de apagar radio y parpadear". No se
// duplica nada — modo_ambar_fijarMotivo() ya existia como funcion publica, asi que el
// Degradado fija el motivo llamandola en vez de escribir dos variables a pelo. El
// estado queda DISJUNTO, que es la condicion para que un corte sea real y no cosmetico.
// ---------------------------------------------------------------------------

void modo_ambar_setup();
void modo_ambar_loop();

// Motivo con el que arrancar el ambar, para que la pantalla no quede muda (el defecto
// que senala SFTY-22). Se fija ANTES de entrar al modo.
//
// Apunta a LITERALES, nunca a memoria temporal: el puntero se guarda tal cual y se
// pinta mas tarde, asi que un buffer de pila dejaria la pantalla leyendo basura.
void modo_ambar_fijarMotivo(const char* linea1, const char* linea2);
