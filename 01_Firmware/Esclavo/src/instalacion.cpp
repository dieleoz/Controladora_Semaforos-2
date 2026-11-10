// ===== src/instalacion.cpp =====
// N-187 (SPEC_8 8, D-161..D-165). IDENTICO EN LAS DOS PUNTAS: ver instalacion.h.
#include "instalacion.h"
#include <stdio.h>
#include <string.h>
#include "instalacion_flash.h"
#include "semaforo.h"   // semaforo_plumaVetada(), semaforo_reaplicarSalidas()
#include "botones.h"    // camaras_sembrar(), camara_presenciaJ16(i)

// Formato: siete medias palabras desde INSTALACION_FLASH_DIR.
//   [0] firma  [1] version  [2] ncC (0|1)  [3] ncD (0|1)  [4] retardo en s
//   [5..6] suma de Horner de 32 bits, sin plegar, sobre [1..4] (molde testigo_flash.cpp)
// Una pagina borrada lee 0xFFFF en todo: no casa la firma y vale fabrica.
static const uint16_t FIRMA = 0x1A57;
static const uint16_t VERSION = 1;

struct Instalacion {
  bool nc[2];
  uint16_t rSeg;
};

static bool retardoValido(uint32_t seg) {
  return seg * 1000UL >= PLUMA_RETARDO_MIN_MS && seg * 1000UL <= PLUMA_RETARDO_MAX_MS;
}

static uint32_t suma(const uint16_t hw[INSTALACION_FLASH_HW]) {
  uint32_t s = 0x2B61U;
  for (uint8_t i = 1; i <= 4; i++) s = s * 31U + hw[i];
  return s;
}

// Lee y valida la pagina. Con cualquier fallo deja *p en FABRICA y devuelve false: no hay
// registro a medias (un campo bueno al lado de uno malo tampoco se cree).
static bool leer(Instalacion* p) {
  uint16_t hw[INSTALACION_FLASH_HW];
  for (uint8_t i = 0; i < INSTALACION_FLASH_HW; i++) hw[i] = instalacionFlash_leerHw(i);
  const uint32_t s = suma(hw);
  const bool ok = hw[0] == FIRMA && hw[1] == VERSION && hw[2] <= 1 && hw[3] <= 1 &&
                  retardoValido(hw[4]) && hw[5] == (uint16_t)(s & 0xFFFFU) &&
                  hw[6] == (uint16_t)(s >> 16);
  p->nc[0] = ok && hw[2] == 1;
  p->nc[1] = ok && hw[3] == 1;
  p->rSeg = ok ? hw[4] : (uint16_t)(PLUMA_RETARDO_BAJADA_MS / 1000UL);
  return ok;
}

static bool escribir(const Instalacion* p) {
  uint16_t hw[INSTALACION_FLASH_HW];
  hw[0] = FIRMA;
  hw[1] = VERSION;
  hw[2] = p->nc[0] ? 1 : 0;
  hw[3] = p->nc[1] ? 1 : 0;
  hw[4] = p->rSeg;
  const uint32_t s = suma(hw);
  hw[5] = (uint16_t)(s & 0xFFFFU);
  hw[6] = (uint16_t)(s >> 16);
  return instalacionFlash_escribir(hw);
}

unsigned long instalacion_retardoPlumaMs() {
  Instalacion p;
  leer(&p);
  return (unsigned long)p.rSeg * 1000UL;   // leer() ya lo dejo en MIN..MAX
}

bool instalacion_camaraNC(int i) {
  if (i < 0 || i > 1) return false;
  Instalacion p;
  leer(&p);
  return p.nc[i];
}

// La cota de la trama sale de los literales que entran (N-154). La mas larga es
// "$ACK,CMD:CONSULTA_INSTALACION,RESULT:C_NA_D_NA_R3,VE:C0D0" (57); la mas larga de las
// ordenes, "$ACK,CMD:SET_PLUMA_RETARDO:5,RESULT:SIN_CAMBIO,VE:C0D0" (54). El valor que
// se repite va acotado por el formato (4 caracteres en SET_CAMARA, 2 en SET_PLUMA_RETARDO).
// Si aun asi no cabe, se dice: nunca una trama truncada.
static char respuesta[64];

static const char* responder(const char* cab, const char* cmd, const char* valor,
                             const char* cola) {
  char ve[12] = "";
  if (cab[1] == 'A') {   // solo el $ACK lleva la presencia logica (SPEC_8 8.5, detector 1)
    snprintf(ve, sizeof(ve), ",VE:C%dD%d", camara_presenciaJ16(0) ? 1 : 0,
             camara_presenciaJ16(1) ? 1 : 0);
  }
  const int n = snprintf(respuesta, sizeof(respuesta), "%s,CMD:%s%s%s,%s%s", cab, cmd,
                         valor ? ":" : "", valor ? valor : "", cola, ve);
  if (n < 0 || n >= (int)sizeof(respuesta)) return "$ERR,CMD:INSTALACION,DESC:NO_CABE";
  return respuesta;
}

// Escribe, re-siembra si hace falta y reentra (SPEC_8 8.5/8.6). La re-siembra va solo si el
// contacto RELEIDO de la flash difiere del de antes, salga bien o no la escritura: una fallida
// puede dejar la pagina borrada (otro contacto), y sin sembrar se veria un flanco que no paso.
// Sin cambio de contacto no se siembra: SET_PLUMA_RETARDO no borra una CAM_PEGADA anunciada
// ni reinicia el aviso del veto. El OK sale de lo RELEIDO: lo compuesto dice siempre lo compuesto.
static const char* aplicar(const char* cmd, const char* valor, const Instalacion* pedido) {
  Instalacion antes, q;
  leer(&antes);
  escribir(pedido);
  const bool leido = leer(&q);
  if (q.nc[0] != antes.nc[0] || q.nc[1] != antes.nc[1]) camaras_sembrar();
  semaforo_reaplicarSalidas();
  if (!leido || q.nc[0] != pedido->nc[0] || q.nc[1] != pedido->nc[1] || q.rSeg != pedido->rSeg) {
    return responder("$ERR", cmd, valor, "DESC:NO_QUEDO_PUESTO");
  }
  return responder("$ACK", cmd, valor, "RESULT:OK");
}

static const char* ordenCamara(const char* v) {
  static const char* const CMD = "SET_CAMARA";
  const int i = v[0] == 'C' ? 0 : v[0] == 'D' ? 1 : -1;
  if (i < 0 || v[1] != ':' || (strcmp(v + 2, "NA") != 0 && strcmp(v + 2, "NC") != 0)) {
    return responder("$ERR", CMD, nullptr, "DESC:FORMATO_INVALIDO");
  }
  const bool nc = strcmp(v + 2, "NC") == 0;
  Instalacion p;
  leer(&p);
  if (p.nc[i] == nc) return responder("$ACK", CMD, v, "RESULT:SIN_CAMBIO");
  // El cambio no suelta un veto en curso (SPEC_8 8.5): primero SET_VETO_CAMARA:OFF.
  if (semaforo_plumaVetada()) return responder("$ERR", CMD, v, "DESC:PLUMA_VETADA");
  p.nc[i] = nc;
  return aplicar(CMD, v, &p);
}

static const char* ordenRetardo(const char* v) {
  static const char* const CMD = "SET_PLUMA_RETARDO";
  const size_t n = strlen(v);
  const bool digitos = n >= 1 && n <= 2 && v[0] >= '0' && v[0] <= '9' &&
                       (n == 1 || (v[1] >= '0' && v[1] <= '9'));
  if (!digitos) return responder("$ERR", CMD, nullptr, "DESC:FORMATO_INVALIDO");
  const uint16_t seg = (uint16_t)(n == 1 ? v[0] - '0' : (v[0] - '0') * 10 + (v[1] - '0'));
  if (!retardoValido(seg)) return responder("$ERR", CMD, v, "DESC:FUERA_DE_RANGO");
  Instalacion p;
  leer(&p);
  if (p.rSeg == seg) return responder("$ACK", CMD, v, "RESULT:SIN_CAMBIO");
  p.rSeg = seg;
  return aplicar(CMD, v, &p);
}

const char* instalacion_orden(const char* accion) {
  if (strncmp(accion, "SET_CAMARA:", 11) == 0) return ordenCamara(accion + 11);
  if (strncmp(accion, "SET_PLUMA_RETARDO:", 18) == 0) return ordenRetardo(accion + 18);
  if (strcmp(accion, "CONSULTA_INSTALACION") != 0) return nullptr;
  Instalacion p;
  leer(&p);
  char r[32];
  snprintf(r, sizeof(r), "RESULT:C_%s_D_%s_R%u", p.nc[0] ? "NC" : "NA", p.nc[1] ? "NC" : "NA",
           (unsigned)p.rSeg);
  return responder("$ACK", "CONSULTA_INSTALACION", nullptr, r);
}
