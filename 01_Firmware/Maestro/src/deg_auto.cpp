// ===== src/deg_auto.cpp =====
// A-15 (SPEC_2 7.ter): el Degradado automatico en la punta Maestro. Ver deg_auto.h.
#include "deg_auto.h"
#include "bluetooth.h"
#include "coordinador.h"
#include "modo_degradado.h"
#include "modos.h"
#include "reloj.h"
#include "respaldo.h"
#include <stdio.h>

// La entrada: la primera marca de DEG_AUTO_MARCA_S que deja DEG_AUTO_ROJO_MIN_S de rojo o mas.
static uint32_t inicioAuto(uint32_t ahora) {
  return ((ahora + DEG_AUTO_ROJO_MIN_S + DEG_AUTO_MARCA_S - 1UL) / DEG_AUTO_MARCA_S *
          DEG_AUTO_MARCA_S) % 86400UL;
}

// Lo que la puerta automatica pide ademas del testigo: la opcion, ninguna rendicion en la pila
// sin intercambio sano despues (arquitecto, 29/09), un modo que cicla solo -en MENU, MANUAL,
// AMBAR, ALCANCE u HORA hay una persona- y la sync del PAR fresca.
static bool salioSinIntercambio = false;   // salio del Degradado y no ha oido un PONG
static bool puertaAbierta() {
  const ModoSistema m = modoActual_get();
  return respaldo_degAuto() && !respaldo_rendido() && !salioSinIntercambio &&
         (m == MODO_AUTOMATICO || m == MODO_INTELIGENTE) && modo_degradado_syncFresca();
}

// (a) APTO del Maestro = "mi puerta automatica me aceptaria AHORA" (responsable, 29/09, H4):
// puertaAbierta() y la MISMA comprobacion que hace modo_degradado_entrarTestigo() -hora fiable
// con fecha-, con el inicio que usaria entrar().
bool degAuto_aptoPropio() {
  if (!puertaAbierta()) return false;
  const uint32_t ahora = reloj_segundosDelDia();
  return modo_degradado_evaluarEntradaTestigo(ahora, inicioAuto(ahora), DEG_AUTO_DESPEJE_S) == MDT_OK;
}

// --- El $ACK diferido de SET_DEG_AUTO (molde REINICIAR_RELOJ) ---------------
// Solo vale un ECO que conteste a una trama emitida DESPUES de la orden: el PONG que llega
// antes refleja el APTO viejo.
static bool ordenPendiente = false;
static bool ordenValor = false;
static bool enviadoTrasOrden = false;
static bool aptoEnviado = false;
static unsigned long tOrden = 0;
static DegAutoVeredicto veredicto = DAV_NINGUNO;

void degAuto_enviar(uint8_t cmd, uint8_t param) {
  if (cmd == CMD_PING || cmd == CMD_GO_GREEN || cmd == CMD_GO_RED) {
    aptoEnviado = degAuto_aptoPropio();
    if (ordenPendiente) enviadoTrasOrden = true;
    if (aptoEnviado) param |= DEG_AUTO_APTO;
    if (respaldo_otroApto()) param |= DEG_AUTO_ECO;
  }
  protocolo_enviarPaquete(cmd, param);
}

DegAutoOrden degAuto_orden(bool activar) {
  if (modoActual_get() == MODO_DEGRADADO) return DAO_EN_DEGRADADO;
  // Sin enlace el Esclavo no se entera y creeria apta a esta punta (SPEC_2 7.ter (a)).
  if (coordinador_msDesdeRespuesta() > SFTY6_SILENCIO_MS) return DAO_SIN_ENLACE;
  respaldo_guardarDegAuto(activar);
  ordenPendiente = true;
  ordenValor = activar;
  enviadoTrasOrden = false;
  tOrden = millis();
  veredicto = DAV_NINGUNO;
  return DAO_ACEPTADA;
}

DegAutoVeredicto degAuto_veredicto() {
  const DegAutoVeredicto v = veredicto;
  veredicto = DAV_NINGUNO;
  return v;
}

// --- (c) PRESENTE: la radio volvio, sin salir del modo ------------------------
static bool presenteOido = false;
static unsigned long tPresente = 0;

static unsigned long tAvisoEnlace = 0;

static void oirPresente() {
  // Una vez por recuperacion: se rearma tras 3 x PRESENTE_S sin oirla. En Degradado, ademas,
  // se repite cada ENLACE_AVISO_REPETIR_MS mientras se siga oyendo (campo 30/09: el funcional
  // que no estaba conectado en ese instante no se enteraba). No sale del modo.
  const bool nueva = !presenteOido || millis() - tPresente > 3UL * PRESENTE_S * 1000UL;
  presenteOido = true;
  tPresente = millis();
  const bool enDeg = (modoActual_get() == MODO_DEGRADADO);
  if (!nueva && !(enDeg && millis() - tAvisoEnlace >= ENLACE_AVISO_REPETIR_MS)) return;
  tAvisoEnlace = millis();
  if (enDeg) {
    bluetooth_reportarEvento("DEGRADADO", "ENLACE_DISPONIBLE");
  } else {
    bluetooth_reportarAlarma("DEGRADADO", "OTRO_EN_DEGRADADO", "REVISE_OTRO");
  }
}

bool degAuto_alRecibir(const RF_Packet* pkt) {
  // PRESENTE no es respuesta: no renueva tUltimaRespuestaEsclavo ni pone handshakeOk.
  if (pkt->command == CMD_PRESENTE) {
    oirPresente();
    return true;
  }
  if (pkt->command == CMD_PONG) {
    respaldo_guardarOtroApto((pkt->param & DEG_AUTO_APTO) != 0);
    respaldo_guardarRendido(false);   // el intercambio sano que levanta la rendicion
    salioSinIntercambio = false;      // y la salida a mano (H11)
    const bool eco = (pkt->param & DEG_AUTO_ECO) != 0;
    respaldo_guardarAptoDado(eco);   // el Esclavo me oyo APTO (o no): lo que el cree de mi
    if (ordenPendiente && enviadoTrasOrden && eco == aptoEnviado) {
      ordenPendiente = false;
      veredicto = !ordenValor ? DAV_OFF : (respaldo_otroApto() ? DAV_ON_EFECTIVO : DAV_ON_FALTA);
    }
  }
  return false;
}

void degAuto_escucharEnDegradado() {
  RF_Packet pkt;
  if (protocolo_hayPaqueteDisponible(&pkt) && pkt.command == CMD_PRESENTE) {
    degAuto_alRecibir(&pkt);
  }
}

// --- (b) La entrada ----------------------------------------------------------
static const char* nombreMotivo(MotivoTestigo m) {
  switch (m) {
    case MDT_FALTA_HORA:      return "HORA";
    case MDT_AHORA_DESFASADO: return "DESFASE";
    case MDT_DESPEJE_RANGO:   return "DESPEJE";
    case MDT_INICIO_VENCIDO:  return "INICIO";
    case MDT_AMBAR_VIGENTE:   return "AMBAR";
    case MDT_EN_VERDE:        return "EN_VERDE";
    case MDT_NO_GUARDADO:     return "GUARDADO";
    default:                  return "OK";
  }
}

static void entrar() {
  const uint32_t ahora = reloj_segundosDelDia();
  const uint32_t inicio = inicioAuto(ahora);
  const MotivoTestigo m = modo_degradado_entrarTestigo(ahora, inicio, DEG_AUTO_DESPEJE_S);
  if (m == MDT_OK) {
    char det[sizeof("AUTO_ENTRADA_INICIO_HH:MM:SS")];
    snprintf(det, sizeof(det), "AUTO_ENTRADA_INICIO_%02lu:%02lu:%02lu",
             (unsigned long)(inicio / 3600UL), (unsigned long)(inicio / 60UL % 60UL),
             (unsigned long)(inicio % 60UL));
    bluetooth_reportarEvento("DEGRADADO", det);
  } else {
    char causa[sizeof("AUTO_NO_GUARDADO")];   // la CAUSA del $ALARM cabe en 19 (esp32_07)
    snprintf(causa, sizeof(causa), "AUTO_NO_%s", nombreMotivo(m));
    // Sin flash, modo_degradado_entrarTestigo() ya forzo el rojo y dejo el MENU: rojo fijo.
    bluetooth_reportarAlarma("DEGRADADO", causa,
                             m == MDT_NO_GUARDADO ? "QUEDA_ROJO" : "SIGUE_AMBAR");
  }
}

static bool intentoHecho = false;   // un intento por corte: lo rearma una respuesta sana
static bool enDegradado = false;    // la vuelta anterior estaba en MODO_DEGRADADO (H11)
static uint32_t segPresente = 0xFFFFFFFFUL;

void degAuto_loop() {
  if (ordenPendiente && millis() - tOrden >= DEG_AUTO_ACUSE_MS) {
    ordenPendiente = false;
    veredicto = DAV_SIN_ACUSE;   // el cambio queda y se sigue publicando
  }

  if (modoActual_get() == MODO_DEGRADADO) {
    enDegradado = true;
    // Maestro en s % 10 == 0, Esclavo en s % 10 == 5: medio duplex sin choque.
    const uint32_t s = reloj_segundosDelDia();
    if (reloj_enHora() && s % PRESENTE_S == 0 && s != segPresente) {
      segPresente = s;
      protocolo_enviarPaquete(CMD_PRESENTE);
    }
    return;
  }

  // Salir del Degradado -a mano o tras rendirse- cierra la puerta hasta el siguiente PONG
  // (arquitecto, 29/09, H11). La cuenta sola no la reabre.
  if (enDegradado) {
    enDegradado = false;
    salioSinIntercambio = true;
  }
  // Desde la ultima respuesta del Esclavo; sin ninguna, desde el arranque. No se mira
  // C_FALLO: tambien llega por reintentos agotados con enlace (SPEC_2 3).
  const unsigned long ms = coordinador_msDesdeRespuesta();
  const unsigned long cuenta = (ms == 0xFFFFFFFFUL) ? millis() : ms;
  if (cuenta < DEG_AUTO_ESPERA_MS) {
    intentoHecho = false;
    return;
  }
  // Opcion EFECTIVA -la propia Y el ultimo APTO oido del otro-, el ECO del Esclavo en 1 -me
  // oyo APTO; si no, el no entra y esta daria verde sola- y la puerta abierta. La
  // comprobacion del testigo la hace entrar(): si rechaza, lo publica una vez.
  if (intentoHecho || !respaldo_otroApto() || !respaldo_aptoDado() || !puertaAbierta()) return;
  intentoHecho = true;
  entrar();
}
