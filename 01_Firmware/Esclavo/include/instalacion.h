#pragma once
#include <Arduino.h>

// ---------------------------------------------------------------------------
// N-187 (SPEC_8 8, D-161..D-165) - PARAMETROS DE INSTALACION DE CAMARAS Y BARRERA.
//
// ESTE FICHERO Y SU .cpp SON IDENTICOS EN MAESTRO Y ESCLAVO. Las ordenes son LOCALES al
// poste conectado y no viajan por radio (como SPEC_8 6.1): cada punta atiende la suya.
//
// Dos parametros, y nada mas (P2 y P4: J15 activa = subir, de nivel, y la pluma ARRIBA en
// ambar intermitente / S_FALLO son FIJOS):
//   - el contacto de cada camara de J16, NA o NC (fabrica NA): lo aplica camara_leerPin()
//     con su parametro de sentido, llamado por indice desde camaras_sembrar() y
//     camaras_actualizar() (botones.cpp); CAM_DEMANDA_PIN (J14) sigue activo en alto.
//   - el retardo de bajada de la pluma tras el ROJO, 3..5 s enteros (P1, P3), fabrica 3 s.
//
// SIN COPIA EN RAM (molde D-149): los getters releen la pagina en CADA llamada. Pagina
// borrada, firma, version o suma que no casan, o un campo fuera de rango: FABRICA entera
// (NA, NA, 3 s), que es el equipo anterior a N-187.
// ---------------------------------------------------------------------------

// D-33 / D-163 / P3: el retardo de bajada tras el rojo. UNA sola copia de cada numero: la
// leen el getter, la orden y los static_assert (aqui y en semaforo.cpp, cota contra el
// despeje minimo). PLUMA_RETARDO_BAJADA_MS es el valor de FABRICA y conserva su nombre:
// los arneses lo releen por ese nombre (D-33, elegido por el responsable el 14/09).
static const unsigned long PLUMA_RETARDO_BAJADA_MS = 3000UL;
static const unsigned long PLUMA_RETARDO_MIN_MS = 3000UL;
static const unsigned long PLUMA_RETARDO_MAX_MS = 5000UL;
static_assert(PLUMA_RETARDO_MIN_MS <= PLUMA_RETARDO_BAJADA_MS &&
              PLUMA_RETARDO_BAJADA_MS <= PLUMA_RETARDO_MAX_MS &&
              PLUMA_RETARDO_MIN_MS % 1000UL == 0 && PLUMA_RETARDO_MAX_MS % 1000UL == 0,
              "N-187: el retardo de fabrica tiene que estar dentro del rango de la orden");

// El retardo vigente en ms, releido de la flash. Siempre en PLUMA_RETARDO_MIN_MS..MAX_MS.
unsigned long instalacion_retardoPlumaMs();

// true si la camara i (0 = C, J16 p10; 1 = D, J16 p12) esta declarada NC. Otro i, o la
// pagina ilegible: false (NA).
bool instalacion_camaraNC(int i);

// SET_CAMARA:<C|D>:<NA|NC>, SET_PLUMA_RETARDO:<s> o CONSULTA_INSTALACION, ya sin el
// "CMD:PIN:...:". Devuelve la trama que se contesta, o nullptr si no es de este modulo.
// Molde SET_TIEMPOS (CLAUDE.md 2): RESULT:OK solo sobre lo RELEIDO de la flash despues de
// escribir, re-sembrar las camaras y reentrar por semaforo_reaplicarSalidas(). Cada $ACK
// lleva VE:C<0|1>D<0|1>, la presencia logica de cada camara en ese momento.
const char* instalacion_orden(const char* accion);
