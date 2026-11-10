// ===== src/interruptores.cpp =====
// SPEC_8 6 y 9 (D-94..D-97, D-148..D-152, D-166, D-169). IDENTICO EN LAS DOS PUNTAS: ver interruptores.h.
#include "interruptores.h"
#include <stdio.h>
#include <string.h>
#include "respaldo.h"
#include "semaforo.h"
#include "botones.h"     // camara_estado(), la fuente del campo CAM:; camara_letra() para CONSULTA_VETO
#include "bluetooth.h"   // bluetooth_reportarEvento(), el aviso de arranque

// La respuesta se compone aqui y la manda el despachador de bluetooth.cpp. Cabe en 64:
// la mas larga es "$ERR,CMD:SET_BARRERA_SERVICIO:OFF,DESC:RESPALDO_INVALIDO" (56). Las de D-166/D-169 no
// la pasan: "$ERR,CMD:SET_CAMARA_MALA:2:BAJA,DESC:RESPALDO_INVALIDO" (54), "$ERR,CMD:SET_CAMARA_MALA:2:ARRIBA,
// DESC:NO_QUEDO_PUESTO" (54), "$ACK,CMD:CONSULTA_VETO,RESULT:C1O_D1O_M1A_M2A" (45). Un valor que no casa no se
// repite en el CMD (FORMATO_INVALIDO va sin el), asi que la cota no depende de lo que mande la app (N-154).
static char respuesta[64];

static const char* responder(const char* cab, const char* cmd, const char* valor,
                             const char* cola) {
  snprintf(respuesta, sizeof(respuesta), "%s,CMD:%s%s%s,%s", cab, cmd, valor ? ":" : "",
           valor ? valor : "", cola);
  return respuesta;
}

// Lo que cada orden pone: el veto de una camara o de las dos (D-166), la barrera, o un MALAx (D-169).
enum Sujeto { VETO_C, VETO_D, VETO_LAS_DOS, BARRERA, MALA1, MALA2 };

// "Ya en ese estado" y "quedo hecho" son la MISMA pregunta (D-149): el getter del respaldo
// y, para la barrera, la anotacion de escribirPines() -el pin-, nunca solo el registro: un
// registro recien escrito devuelve siempre lo escrito. "puesto" es el valor que NO es de fabrica.
static bool enEstado(Sujeto s, bool puesto) {
  switch (s) {
    case VETO_C:       return respaldo_vetoCamaraOff(0) == puesto;
    case VETO_D:       return respaldo_vetoCamaraOff(1) == puesto;
    case VETO_LAS_DOS: return respaldo_vetoCamaraOff(0) == puesto && respaldo_vetoCamaraOff(1) == puesto;
    case MALA1:        return respaldo_malaBaja(1) == puesto;
    case MALA2:        return respaldo_malaBaja(2) == puesto;
    default:           return respaldo_barreraFuera() == puesto && semaforo_plumaFuera() == puesto;
  }
}

static void guardar(Sujeto s, bool puesto) {
  if (s == VETO_C || s == VETO_LAS_DOS) respaldo_guardarVetoCamaraOff(0, puesto);
  if (s == VETO_D || s == VETO_LAS_DOS) respaldo_guardarVetoCamaraOff(1, puesto);
  if (s == MALA1 || s == MALA2) respaldo_guardarMalaBaja(s == MALA1 ? 1 : 2, puesto);
  if (s == BARRERA) respaldo_guardarBarreraFuera(puesto);
}

// D-166 (SPEC_8 9.5): sin PIN, solo mira. Por camara: 1 con veto / 0 inhibida y su letra; MALA1/MALA2, A o B.
// Los pares salen de literales, no de un %c: el juez de tramas de la app (tests/dom_simulador.js) exige que
// cada hueco de una respuesta sea texto de un literal de este C++.
static const char* parCamara(int i) {
  static const char* const PAR[2][4] = {{"0O", "0?", "0C", "0P"}, {"1O", "1?", "1C", "1P"}};
  const char l = camara_letra(i);
  return PAR[respaldo_vetoCamaraOff(i) ? 0 : 1][l == 'O' ? 0 : l == '?' ? 1 : l == 'C' ? 2 : 3];
}
static const char* consultaVeto() {
  char r[sizeof("RESULT:C1O_D1O_M1A_M2A")];
  snprintf(r, sizeof(r), "RESULT:C%s_D%s_M1%s_M2%s", parCamara(0), parCamara(1),
           respaldo_malaBaja(1) ? "B" : "A", respaldo_malaBaja(2) ? "B" : "A");
  return responder("$ACK", "CONSULTA_VETO", nullptr, r);
}

const char* interruptores_orden(const char* accion) {
  if (strcmp(accion, "CONSULTA_VETO") == 0) return consultaVeto();
  const bool esVeto = strncmp(accion, "SET_VETO_CAMARA:", 16) == 0;
  const bool esMala = strncmp(accion, "SET_CAMARA_MALA:", 16) == 0;
  if (!esVeto && !esMala && strncmp(accion, "SET_BARRERA_SERVICIO:", 21) != 0) return nullptr;
  const char* cmd = esVeto ? "SET_VETO_CAMARA" : esMala ? "SET_CAMARA_MALA" : "SET_BARRERA_SERVICIO";
  const char* valor = accion + (esVeto || esMala ? 16 : 21);
  // El sujeto y lo que se pide. D-166: "C:" o "D:" delante elige camara; sin el, las dos (la orden de D-94).
  // D-169: "1:" o "2:" y BAJA (lo puesto) o ARRIBA (fabrica). Otro texto, FORMATO_INVALIDO sin valor.
  Sujeto s = esVeto ? VETO_LAS_DOS : BARRERA;
  const char* v = valor;
  if (esVeto && (v[0] == 'C' || v[0] == 'D') && v[1] == ':') {
    s = v[0] == 'C' ? VETO_C : VETO_D;
    v += 2;
  } else if (esMala) {
    if ((v[0] != '1' && v[0] != '2') || v[1] != ':') return responder("$ERR", cmd, nullptr, "DESC:FORMATO_INVALIDO");
    s = v[0] == '1' ? MALA1 : MALA2;
    v += 2;
  }
  const char* const fabrica = esMala ? "ARRIBA" : "ON";
  const bool vaFabrica = strcmp(v, fabrica) == 0;
  if (!vaFabrica && strcmp(v, esMala ? "BAJA" : "OFF") != 0) {
    return responder("$ERR", cmd, nullptr, "DESC:FORMATO_INVALIDO");
  }
  const bool puesto = !vaFabrica;   // OFF o BAJA es lo PUESTO; ON o ARRIBA, lo de fabrica
  if (enEstado(s, puesto)) return responder("$ACK", cmd, valor, "RESULT:SIN_CAMBIO");
  // D-148: lo que no va a sobrevivir al corte no se pone. Lo de fabrica si: es el lado seguro.
  if (puesto && respaldo_perdidoAlArrancar()) {
    return responder("$ERR", cmd, valor, "DESC:RESPALDO_INVALIDO");
  }
  guardar(s, puesto);
  // D-149: la orden vuelve a pasar por la puerta. Sin esto, en S_ROJO estable el pin no se
  // moveria hasta el siguiente cambio de luz.
  semaforo_reaplicarSalidas();
  if (!enEstado(s, puesto)) return responder("$ERR", cmd, valor, "DESC:NO_QUEDO_PUESTO");
  return responder("$ACK", cmd, valor, "RESULT:OK");
}

// D-151 y D-166 (Q6): NOVETO si ALGUNA camara esta inhibida y la peor de las demas -camara_estado(), que ya
// salta las inhibidas- es OK o '?'. Con las dos inhibidas camara_estado() dice '?': NOVETO.
bool interruptores_camNoVeto() {
  if (!respaldo_vetoCamaraOff(0) && !respaldo_vetoCamaraOff(1)) return false;
  const char* e = camara_estado();
  return strcmp(e, "OK") == 0 || strcmp(e, "?") == 0;
}

// D-152 y D-166: un aviso por arranque; todos llevan CAM_NOVETO, que es lo que busca la app.
void interruptores_avisoArranque() {
  const bool c = respaldo_vetoCamaraOff(0), d = respaldo_vetoCamaraOff(1), fuera = respaldo_barreraFuera();
  if (!c && !d && !fuera) return;
  const char* cam = c && d ? "ARRANQUE_CAM_NOVETO" : c ? "ARRANQUE_CAM_NOVETO_C" : d ? "ARRANQUE_CAM_NOVETO_D" : "";
  char detalle[sizeof("ARRANQUE_CAM_NOVETO_C_Y_BARRERA_FUERA")];
  if (!c && !d) {
    snprintf(detalle, sizeof(detalle), "ARRANQUE_BARRERA_FUERA");
  } else {
    snprintf(detalle, sizeof(detalle), "%s%s", cam, fuera ? "_Y_BARRERA_FUERA" : "");
  }
  bluetooth_reportarEvento("INTERRUPTOR", detalle);
}
