// ===== include/semaforo.h =====
#pragma once
#include <Arduino.h>

enum EstadoSemaforo { S_ROJO, S_VERDE, S_AMARILLO, S_FALLO };

void semaforo_setup();
void semaforo_apagarTodo();
void semaforo_forzarRojo();
void semaforo_forzarVerde();
void semaforo_iniciarTransicionAVerde();
void semaforo_toggle();
void semaforo_iniciarFallo();
void semaforo_actualizar();
bool semaforo_estable();

EstadoSemaforo semaforo_estado();
const char* semaforo_nombreEstado();

// D-30 (reafirmada 14/09): LA SENAL DE CONFIRMACION DEL MANDO SALIO DE AQUI.
//
// Lo que habia era SFTY-21: semaforo_destellosRojos(), semaforo_ambarRapido() y
// semaforo_senalEnCurso(), mas la interceptacion de escrituras a pines que las
// sostenia. Existian para contestarle con las luces a un operario que accionaba el
// equipo desde el suelo con un mando de reles, porque no veia la pantalla.
//
// Sus UNICOS llamadores vivian en mando.cpp. Retirado el mando -el hardware el
// 05/09, la lectura de flancos el 14/09-, la bandera no volvia a armarse nunca y la
// guarda de aplicarSalidas() no volvia a disparar: era un camino que ningun
// instrumento podia ejercer, y eso es lo que CLAUDE.md 6 prohibe dejar dentro.
// Sale entero, no a medias. El equipo escribe las luces por el camino normal.
// Test de lámparas de 6 segundos en taller (2s Rojo -> 2s Amarillo -> 2s Verde)
void semaforo_iniciarTestLeds();
bool semaforo_testLedsEnCurso();

// ---------------------------------------------------------------------------
// N-153 - EL ESTADO DE LA PLUMA, PUBLICADO.
//
// Devuelve lo que escribirPines() escribio la ultima vez en MOTOR_TALANQUERA: true si
// quedo ARRIBA. No recalcula la condicion de SFTY-28 -eso serian dos formulas que
// alguien tendria que mantener iguales-, devuelve la bandera que la propia orden dejo
// puesta.
//
// EXISTE PARA QUE LA APP PUEDA DIBUJARLA. Con D-13 va a haber ratos de LUZ ROJA CON LA
// PLUMA ARRIBA -presencia debajo, la barrera no baja-, y un operario que hoy vea eso
// lo lee como averia. Publicarlo no depende de las camaras y cierra el hueco antes.
// ---------------------------------------------------------------------------
bool semaforo_plumaArriba();

// ---------------------------------------------------------------------------
// D-33 (14/09/2026) - POR QUE LA PLUMA SIGUE ARRIBA.
//
// true SOLO cuando el pin esta en ABRIR porque una camara ve algo debajo. Durante los
// segundos de retardo que siguen al rojo devuelve FALSE aunque la pluma siga arriba: ahi
// no hay veto, hay cortesia con el que entro legalmente, y confundir las dos cosas
// inflaria el contador de vetos con bajadas que nadie impidio.
//
// EXISTE PARA QUE EL CONTADOR PUEDA CONTAR LO QUE PASA DE VERDAD. Hasta D-33,
// vigilante_tick() contaba mirando la BAJADA ya hecha ("el veto habria actuado"); con el
// veto construido esa bajada no ocurre, asi que ese contador se habria callado justo
// cuando empieza a haber algo que contar. Se cuenta el flanco de subida de esta bandera.
// ---------------------------------------------------------------------------
bool semaforo_plumaVetada();