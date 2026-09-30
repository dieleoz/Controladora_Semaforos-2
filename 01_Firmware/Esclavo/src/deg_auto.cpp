// ===== src/deg_auto.cpp =====
// A-15 (SPEC_2 7.ter): el Degradado automatico en la punta Esclavo. Ver deg_auto.h.
#include "deg_auto.h"
#include "bluetooth.h"
#include "modo_degradado.h"
#include "reloj.h"
#include "respaldo.h"
#include <stdio.h>

// La entrada: la primera marca de DEG_AUTO_MARCA_S que deja DEG_AUTO_ROJO_MIN_S de rojo o mas.
static uint32_t inicioAuto(uint32_t ahora) {
  return ((ahora + DEG_AUTO_ROJO_MIN_S + DEG_AUTO_MARCA_S - 1UL) / DEG_AUTO_MARCA_S *
          DEG_AUTO_MARCA_S) % 86400UL;
}

// Lo que la puerta automatica pide ademas del testigo: la opcion, ninguna rendicion en la
// pila sin intercambio sano despues (arquitecto, 29/09) y el modo quieto -en DEG_RENDIDO no se
// vuelve a entrar: ese ambar es el final del Degradado-.
static bool salioSinIntercambio = false;   // salio del Degradado y no ha oido PING ni GO_GREEN
static bool puertaAbierta() {
  return respaldo_degAuto() && !respaldo_rendido() && !salioSinIntercambio &&
         degradado_estado() == DEG_INACTIVO;
}

// (a) APTO del Esclavo = "mi puerta automatica me aceptaria AHORA" (responsable, 29/09, H4):
// puertaAbierta() y la MISMA comprobacion que hace degradado_entrarTestigo() -hora fiable con
// fecha, ambar de emergencia-, con el inicio que usaria entrar(). No exige sync propia: la
// del par la trae el APTO del Maestro.
bool degAuto_aptoPropio() {
  if (!puertaAbierta()) return false;
  const uint32_t ahora = reloj_segundosDelDia();
  return degradado_comprobarTestigo(ahora, inicioAuto(ahora), DEG_AUTO_DESPEJE_S) == DEG_T_ACEPTADO;
}

// --- El $ACK diferido de SET_DEG_AUTO ------------------------------------------
// Solo vale un ECO que llegue en una orden del Maestro posterior a un PONG ya emitido con
// el APTO nuevo: el PING anterior refleja el APTO viejo.
static bool ordenPendiente = false;
static bool ordenValor = false;
static bool enviadoTrasOrden = false;
static bool aptoEnviado = false;
static unsigned long tOrden = 0;
static DegAutoVeredicto veredicto = DAV_NINGUNO;

uint8_t degAuto_paramSaliente(uint8_t cmd) {
  if (cmd != CMD_PONG) return 0;
  aptoEnviado = degAuto_aptoPropio();
  if (ordenPendiente) enviadoTrasOrden = true;
  return (uint8_t)((aptoEnviado ? DEG_AUTO_APTO : 0) | (respaldo_otroApto() ? DEG_AUTO_ECO : 0));
}

// El enlace de esta punta: una orden de gobierno del Maestro en los ultimos
// SFTY6_SILENCIO_MS. La cuenta de (b), en cambio, solo la reinician PING y GO_GREEN.
static bool huboGobierno = false;
static unsigned long tGobierno = 0;
static unsigned long tCuenta = 0;   // 0 = desde el arranque
static bool intentoHecho = false;   // un intento por corte: lo rearma un PING o GO_GREEN
static bool enDegradado = false;    // la vuelta anterior gobernaba el Degradado (H11)

DegAutoOrden degAuto_orden(bool activar) {
  if (degradado_estado() != DEG_INACTIVO) return DAO_EN_DEGRADADO;
  // Sin enlace el Maestro no se entera y creeria apta a esta punta (SPEC_2 7.ter (a)).
  if (!huboGobierno || millis() - tGobierno > SFTY6_SILENCIO_MS) return DAO_SIN_ENLACE;
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

// --- (c) PRESENTE ----------------------------------------------------------------
static bool presenteOido = false;
static unsigned long tPresente = 0;

static void oirPresente() {
  // Una vez por recuperacion: se rearma tras 3 x PRESENTE_S sin oirla.
  const bool nueva = !presenteOido || millis() - tPresente > 3UL * PRESENTE_S * 1000UL;
  presenteOido = true;
  tPresente = millis();
  if (!nueva) return;
  if (degradado_gobiernaLuz()) {
    bluetooth_reportarEvento("DEGRADADO", "ENLACE_DISPONIBLE");
  } else {
    bluetooth_reportarAlarma("DEGRADADO", "OTRO_EN_DEGRADADO", "REVISE_OTRO");
  }
}

bool degAuto_alRecibir(const RF_Packet* pkt) {
  if (pkt->command == CMD_PRESENTE) {
    oirPresente();
    return true;
  }
  if (pkt->command == CMD_PING || pkt->command == CMD_GO_GREEN || pkt->command == CMD_GO_RED) {
    huboGobierno = true;
    tGobierno = millis();
    // CMD_GO_RED no cuenta: el Maestro en C_FALLO solo emite eso, y con la subida E->M muerta
    // esta punta empezaria a contar 300 s tarde (SPEC_2 7.ter (b)).
    if (pkt->command != CMD_GO_RED) {
      tCuenta = millis();
      respaldo_guardarRendido(false);   // el intercambio sano que levanta la rendicion
      salioSinIntercambio = false;      // y la salida a mano (H11)
    }
    respaldo_guardarOtroApto((pkt->param & DEG_AUTO_APTO) != 0);
    const bool eco = (pkt->param & DEG_AUTO_ECO) != 0;
    respaldo_guardarAptoDado(eco);   // el Maestro me oyo APTO (o no): lo que el cree de mi
    if (ordenPendiente && enviadoTrasOrden && eco == aptoEnviado) {
      ordenPendiente = false;
      veredicto = !ordenValor ? DAV_OFF : (respaldo_otroApto() ? DAV_ON_EFECTIVO : DAV_ON_FALTA);
    }
  }
  return false;
}

// --- (b) La entrada ----------------------------------------------------------------
static const char* nombreRechazo(RechazoTestigo r) {
  switch (r) {
    case DEG_RECHAZO_T_SIN_HORA:        return "HORA";
    case DEG_RECHAZO_T_AHORA_DESFASADO: return "DESFASE";
    case DEG_RECHAZO_T_INICIO_VENCIDO:  return "INICIO";
    case DEG_RECHAZO_T_DESPEJE_RANGO:   return "DESPEJE";
    case DEG_RECHAZO_T_AMBAR_VIGENTE:   return "AMBAR";
    case DEG_RECHAZO_T_EN_VERDE:        return "EN_VERDE";
    case DEG_RECHAZO_T_NO_GUARDADO:     return "GUARDADO";
    default:                            return "OK";
  }
}

static void entrar() {
  const uint32_t ahora = reloj_segundosDelDia();
  const uint32_t inicio = inicioAuto(ahora);
  const RechazoTestigo r = degradado_entrarTestigo(ahora, inicio, DEG_AUTO_DESPEJE_S);
  if (r == DEG_T_ACEPTADO) {
    char det[sizeof("AUTO_ENTRADA_INICIO_HH:MM:SS")];
    snprintf(det, sizeof(det), "AUTO_ENTRADA_INICIO_%02lu:%02lu:%02lu",
             (unsigned long)(inicio / 3600UL), (unsigned long)(inicio / 60UL % 60UL),
             (unsigned long)(inicio % 60UL));
    bluetooth_reportarEvento("DEGRADADO", det);
  } else {
    char causa[sizeof("AUTO_NO_GUARDADO")];   // la CAUSA del $ALARM cabe en 19 (esp32_07)
    snprintf(causa, sizeof(causa), "AUTO_NO_%s", nombreRechazo(r));
    // SIGUE_AMBAR tambien con NO_GUARDADO: el rojo que fuerza la puerta dura una vuelta, y la
    // orfandad de main.cpp lo devuelve a S_FALLO (medido 29/09, H12 del arnes del Degradado).
    bluetooth_reportarAlarma("DEGRADADO", causa, "SIGUE_AMBAR");
  }
}

static uint32_t segPresente = 0xFFFFFFFFUL;

void degAuto_loop() {
  if (ordenPendiente && millis() - tOrden >= DEG_AUTO_ACUSE_MS) {
    ordenPendiente = false;
    veredicto = DAV_SIN_ACUSE;   // el cambio queda y se sigue publicando
  }

  if (degradado_gobiernaLuz()) {
    enDegradado = true;
    // Esclavo en s % 10 == 5, Maestro en s % 10 == 0: medio duplex sin choque.
    const uint32_t s = reloj_segundosDelDia();
    if (reloj_enHora() && s % PRESENTE_S == 5 && s != segPresente) {
      segPresente = s;
      protocolo_enviarPaquete(CMD_PRESENTE);
    }
    return;
  }

  // Salir del Degradado -a mano, por la radio o rindiendose- cierra la puerta hasta el siguiente
  // PING o GO_GREEN (arquitecto, 29/09, H11). La cuenta sola no la reabre.
  if (enDegradado) {
    enDegradado = false;
    salioSinIntercambio = true;
  }
  // Desde el ultimo PING o GO_GREEN; sin ninguno, desde el arranque. No depende de S_FALLO:
  // con la subida muerta esta punta pasa la cuenta en rojo y cuenta igual.
  if (millis() - tCuenta < DEG_AUTO_ESPERA_MS) {
    intentoHecho = false;
    return;
  }
  // Opcion EFECTIVA -la propia Y el ultimo APTO oido del Maestro-, el ECO del Maestro en 1 -me
  // oyo APTO; si no, el no entra y esta daria verde sola- y la puerta abierta. La
  // comprobacion del testigo la hace entrar(): si rechaza, lo publica una vez.
  if (intentoHecho || !respaldo_otroApto() || !respaldo_aptoDado() || !puertaAbierta()) return;
  intentoHecho = true;
  entrar();
}
