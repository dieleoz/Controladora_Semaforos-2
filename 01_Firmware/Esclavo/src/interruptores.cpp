// ===== src/interruptores.cpp =====
// SPEC_8 6 (D-94..D-97, D-148..D-152). IDENTICO EN LAS DOS PUNTAS: ver interruptores.h.
#include "interruptores.h"
#include <stdio.h>
#include <string.h>
#include "respaldo.h"
#include "semaforo.h"
#include "botones.h"     // camara_estado(), la fuente del campo CAM:
#include "bluetooth.h"   // bluetooth_reportarEvento(), el aviso de arranque

// La respuesta se compone aqui y la manda el despachador de bluetooth.cpp. Cabe en 64:
// la mas larga es "$ERR,CMD:SET_BARRERA_SERVICIO:OFF,DESC:RESPALDO_INVALIDO" (56).
static char respuesta[64];

static const char* responder(const char* cab, const char* cmd, const char* valor,
                             const char* cola) {
  snprintf(respuesta, sizeof(respuesta), "%s,CMD:%s%s%s,%s", cab, cmd, valor ? ":" : "",
           valor ? valor : "", cola);
  return respuesta;
}

// "Ya en ese estado" y "quedo hecho" son la MISMA pregunta (D-149): el getter del respaldo
// y, para la barrera, la anotacion de escribirPines() -el pin-, nunca solo el registro: un
// registro recien escrito devuelve siempre lo escrito.
static bool enEstado(bool esVeto, bool puesto) {
  if (esVeto) return respaldo_vetoCamaraOff() == puesto;
  return respaldo_barreraFuera() == puesto && semaforo_plumaFuera() == puesto;
}

const char* interruptores_orden(const char* accion) {
  const bool esVeto = strncmp(accion, "SET_VETO_CAMARA:", 16) == 0;
  if (!esVeto && strncmp(accion, "SET_BARRERA_SERVICIO:", 21) != 0) return nullptr;
  const char* cmd = esVeto ? "SET_VETO_CAMARA" : "SET_BARRERA_SERVICIO";
  const char* valor = accion + (esVeto ? 16 : 21);
  const bool on = strcmp(valor, "ON") == 0;
  if (!on && strcmp(valor, "OFF") != 0) return responder("$ERR", cmd, nullptr, "DESC:FORMATO_INVALIDO");
  const bool puesto = !on;   // OFF es el interruptor PUESTO; ON, lo de fabrica
  if (enEstado(esVeto, puesto)) return responder("$ACK", cmd, valor, "RESULT:SIN_CAMBIO");
  // D-148: un interruptor que no va a sobrevivir al corte no se pone. El ON si: es el lado seguro.
  if (puesto && respaldo_perdidoAlArrancar()) {
    return responder("$ERR", cmd, valor, "DESC:RESPALDO_INVALIDO");
  }
  if (esVeto) {
    respaldo_guardarVetoCamaraOff(puesto);
  } else {
    respaldo_guardarBarreraFuera(puesto);
  }
  // D-149: la orden vuelve a pasar por la puerta. Sin esto, en S_ROJO estable el pin no se
  // moveria hasta el siguiente cambio de luz.
  semaforo_reaplicarSalidas();
  if (!enEstado(esVeto, puesto)) return responder("$ERR", cmd, valor, "DESC:NO_QUEDO_PUESTO");
  return responder("$ACK", cmd, valor, "RESULT:OK");
}

bool interruptores_camNoVeto() {
  if (!respaldo_vetoCamaraOff()) return false;
  const char* e = camara_estado();
  return strcmp(e, "OK") == 0 || strcmp(e, "?") == 0;
}

void interruptores_avisoArranque() {
  const bool veto = respaldo_vetoCamaraOff(), fuera = respaldo_barreraFuera();
  if (!veto && !fuera) return;
  bluetooth_reportarEvento("INTERRUPTOR", veto && fuera ? "ARRANQUE_CAM_NOVETO_Y_BARRERA_FUERA"
                                          : veto        ? "ARRANQUE_CAM_NOVETO"
                                                        : "ARRANQUE_BARRERA_FUERA");
}
