// ===== src/deg_auto.cpp =====
// A-15 (SPEC_2 7.ter): el Degradado automatico en la punta Esclavo. Ver deg_auto.h.
#include "deg_auto.h"
#include "bluetooth.h"
#include "modo_degradado.h"
#include "reloj.h"
#include "respaldo.h"
#include <stdio.h>

// (a) APTO del Esclavo: opcion propia, hora fiable y sin ambar de emergencia. No exige sync
// propia: la del par la trae el APTO del Maestro.
bool degAuto_aptoPropio() {
  return respaldo_degAuto() && reloj_horaFiable() && !bluetooth_ambarEmergencia();
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
    bluetooth_reportarAlarma("DEGRADADO", "OTRO_POSTE_EN_DEGRADADO", "REVISE_EL_OTRO_POSTE");
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
    if (pkt->command != CMD_GO_RED) tCuenta = millis();
    respaldo_guardarOtroApto((pkt->param & DEG_AUTO_APTO) != 0);
    const bool eco = (pkt->param & DEG_AUTO_ECO) != 0;
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
    case DEG_RECHAZO_T_SIN_HORA:        return "DEG_RECHAZO_T_SIN_HORA";
    case DEG_RECHAZO_T_AHORA_DESFASADO: return "DEG_RECHAZO_T_AHORA_DESFASADO";
    case DEG_RECHAZO_T_INICIO_VENCIDO:  return "DEG_RECHAZO_T_INICIO_VENCIDO";
    case DEG_RECHAZO_T_DESPEJE_RANGO:   return "DEG_RECHAZO_T_DESPEJE_RANGO";
    case DEG_RECHAZO_T_AMBAR_VIGENTE:   return "DEG_RECHAZO_T_AMBAR_VIGENTE";
    case DEG_RECHAZO_T_EN_VERDE:        return "DEG_RECHAZO_T_EN_VERDE";
    case DEG_RECHAZO_T_NO_GUARDADO:     return "DEG_RECHAZO_T_NO_GUARDADO";
    default:                            return "DEG_T_ACEPTADO";
  }
}

static void entrar() {
  const uint32_t ahora = reloj_segundosDelDia();
  // La primera marca de DEG_AUTO_MARCA_S que deja DEG_AUTO_ROJO_MIN_S de rojo o mas.
  const uint32_t inicio = ((ahora + DEG_AUTO_ROJO_MIN_S + DEG_AUTO_MARCA_S - 1UL) /
                           DEG_AUTO_MARCA_S * DEG_AUTO_MARCA_S) % 86400UL;
  const RechazoTestigo r = degradado_entrarTestigo(ahora, inicio, DEG_AUTO_DESPEJE_S);
  if (r == DEG_T_ACEPTADO) {
    char det[sizeof("AUTO_ENTRADA_INICIO_HH:MM:SS")];
    snprintf(det, sizeof(det), "AUTO_ENTRADA_INICIO_%02lu:%02lu:%02lu",
             (unsigned long)(inicio / 3600UL), (unsigned long)(inicio / 60UL % 60UL),
             (unsigned long)(inicio % 60UL));
    bluetooth_reportarEvento("DEGRADADO", det);
  } else {
    char causa[sizeof("AUTO_RECHAZADA_DEG_RECHAZO_T_AHORA_DESFASADO")];
    snprintf(causa, sizeof(causa), "AUTO_RECHAZADA_%s", nombreRechazo(r));
    bluetooth_reportarAlarma("DEGRADADO", causa, "SIGUE_EN_AMBAR");
  }
}

static uint32_t segPresente = 0xFFFFFFFFUL;

void degAuto_loop() {
  if (ordenPendiente && millis() - tOrden >= DEG_AUTO_ACUSE_MS) {
    ordenPendiente = false;
    veredicto = DAV_SIN_ACUSE;   // el cambio queda y se sigue publicando
  }

  if (degradado_gobiernaLuz()) {
    // Esclavo en s % 10 == 5, Maestro en s % 10 == 0: medio duplex sin choque.
    const uint32_t s = reloj_segundosDelDia();
    if (reloj_enHora() && s % PRESENTE_S == 5 && s != segPresente) {
      segPresente = s;
      protocolo_enviarPaquete(CMD_PRESENTE);
    }
    return;
  }

  // Desde el ultimo PING o GO_GREEN; sin ninguno, desde el arranque. No depende de S_FALLO:
  // con la subida muerta esta punta pasa la cuenta en rojo y cuenta igual.
  if (millis() - tCuenta < DEG_AUTO_ESPERA_MS) {
    intentoHecho = false;
    return;
  }
  // Opcion EFECTIVA -la propia Y el ultimo APTO oido del Maestro- y APTO propio ahora. En
  // DEG_RENDIDO no se vuelve a entrar: ese ambar es el final del Degradado.
  if (intentoHecho || degradado_estado() != DEG_INACTIVO || !respaldo_otroApto() ||
      !degAuto_aptoPropio()) return;
  intentoHecho = true;
  entrar();
}
