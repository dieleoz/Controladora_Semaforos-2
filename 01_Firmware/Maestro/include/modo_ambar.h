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

// ---------------------------------------------------------------------------
// N-152: DE QUIEN ES ESTE AMBAR. Y VIVE PEGADO AL MOTIVO PORQUE ES EL MOTIVO.
//
// El aviso de cancelacion del Esclavo (CMD_CANCELA_AMBAR_ESCLAVO) saca al Maestro del
// ambar, y eso solo puede valer para el ambar QUE PIDIO EL ESCLAVO. A MODO_AMBAR se
// llega tambien por B.B.B, por SET_MODO:AMBAR y desde el Degradado, y esos los pidio
// una persona que puede estar en la calzada del Poste 1: sacarla de ahi porque el otro
// extremo lo pida seria peor que el defecto que se arregla.
//
// EL MAESTRO NO PODIA DISTINGUIRLOS. Los cuatro caminos hacian el mismo
// modoActual_set(MODO_AMBAR) y nada quedaba escrito sobre quien lo pidio, asi que esta
// distincion es parte del arreglo y no un adorno.
//
// POR QUE NO ES UNA BANDERA APARTE: la pregunta "quien pidio este ambar" ya se
// contestaba una vez, en las dos lineas de motivo que se pintan en la pantalla. Dos
// variables para la misma pregunta son dos que alguien tiene que sincronizar, y este
// repositorio ya pago eso (N-36, N-39). Asi que fijar el motivo ES declarar el origen:
//
//   modo_ambar_fijarMotivo(...)          -> lo pidio ALGUIEN DE ESTE POSTE
//   modo_ambar_fijarMotivoDelEsclavo()   -> lo pidio el Poste 2 por radio
//
// CONSECUENCIA QUE HAY QUE CONOCER, y es la direccion segura: si un ambar del Esclavo
// esta vigente y alguien de este poste pide ambar -B.B.B, o la app-, ese
// fijarMotivo() apaga el origen remoto y el aviso del Esclavo deja de poder sacar al
// cruce. Gana quien esta aqui de pie.
//
// EFECTO LATERAL QUE VA EN LA DIRECCION BUENA: hasta hoy el ambar de N-142 no fijaba
// motivo ninguno, asi que la pantalla del gabinete seguia mostrando el ANTERIOR -tras
// un B.B.B, "Ambar pedido desde el mando (B.B.B)" para un ambar que pidio el Poste 2-.
// Un motivo heredado es una fuente de pantalla que miente (SFTY-22).
void modo_ambar_fijarMotivoDelEsclavo();

// true mientras el ambar vigente sea el que pidio el Esclavo. Lo consulta main.cpp
// antes de atender la cancelacion, y NO se consulta fuera de MODO_AMBAR: su valor solo
// tiene sentido acompanado del modo.
bool modo_ambar_origenEsclavo();
