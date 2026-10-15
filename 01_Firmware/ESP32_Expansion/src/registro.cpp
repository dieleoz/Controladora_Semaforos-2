// ===== 01_Firmware/ESP32_Expansion/src/registro.cpp =====
//
// EL REGISTRO DEL PUENTE (SPEC_4 9.8). Que se guarda, donde y como se sirve; el porque
// esta en registro.h y en la SPEC. Aqui solo la mecanica y sus bordes.

#include "registro.h"
#include "contrato.h"
#include "trama.h"
#include "puente.h"
#include "transporte_app.h"
#include "reloj_ds3231.h"
#include <Arduino.h>
#include <esp_partition.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// D-72: el registro en flash. 4+4+6+1+1+110+2 = 128 B, todos los campos alineados.
struct Registro {
  uint32_t seq;                       // desde 1; 0xFFFFFFFF = hueco (flash borrada)
  uint32_t up_ms;
  uint8_t  fecha[6];                  // AA MM DD hh mm ss; todo 0 sin hora fiable
  uint8_t  clase;                     // 'E', 'S' o 'P'
  uint8_t  len;
  char     texto[REGISTRO_TEXTO_MAX];
  uint16_t crc;                       // CRC-16/CCITT-FALSE de los 126 bytes de delante
};
static_assert(sizeof(Registro) == REGISTRO_REG_B, "el registro de D-72 mide 128 B");

static const uint32_t SECTOR_B = 4096;
static const uint32_t POR_SECTOR = SECTOR_B / REGISTRO_REG_B;   // 32
static const uint32_t HUECO = 0xFFFFFFFFUL;

static const esp_partition_t* part = NULL;
static uint32_t ranuras = 0;          // sectores x 32 (224 x 32 = 7168 con huge_app.csv)
static uint32_t posEscr = 0;          // ranura donde va el siguiente registro
static uint32_t ult = 0;              // seq del ultimo escrito (0 = ninguno)
static uint32_t primero = 0;          // seq mas viejo que sigue en flash (0 = ninguno)
static unsigned long perdidas = 0;    // ANILLO: pisadas en RAM antes de llegar a flash
static unsigned long fallos = 0;      // FLASH: borrado o escritura que no devolvio ESP_OK
static unsigned long malos = 0;       // MALOS: seq valido y CRC malo al leer

static Registro anillo[REGISTRO_ANILLO];
static uint8_t cabeza = 0;            // el mas viejo pendiente
static uint8_t pendientes = 0;

static char modoVisto[24] = "";
static char estadoVisto[24] = "";

static uint16_t crc16(const uint8_t* d, size_t n) {
  uint16_t c = 0xFFFF;
  for (size_t i = 0; i < n; i++) {
    c ^= (uint16_t)d[i] << 8;
    for (int b = 0; b < 8; b++) {
      c = (c & 0x8000) ? (uint16_t)((c << 1) ^ 0x1021) : (uint16_t)(c << 1);
    }
  }
  return c;
}

static uint32_t seqEn(uint32_t ranura) {
  uint32_t s = HUECO;
  if (esp_partition_read(part, ranura * REGISTRO_REG_B, &s, sizeof(s)) != ESP_OK) return HUECO;
  return s;
}

static bool vale(uint32_t s) { return s != HUECO && s != 0; }

// D-72, arranque: el sector vivo es el de cabeza con seq mayor; dentro, el primer hueco
// es la posicion de escritura. PRIMERO es la cabeza valida menor. Sin ninguna: sector 0.
bool registro_setup() {
  part = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, NULL);
  if (part == NULL || part->size < 2 * SECTOR_B) { part = NULL; return false; }
  uint32_t sectores = part->size / SECTOR_B;
  ranuras = sectores * POR_SECTOR;
  uint32_t mayor = 0, vivo = 0, menor = 0;
  for (uint32_t s = 0; s < sectores; s++) {
    uint32_t q = seqEn(s * POR_SECTOR);
    if (!vale(q)) continue;
    if (q > mayor) { mayor = q; vivo = s; }
    if (menor == 0 || q < menor) menor = q;
  }
  posEscr = 0; ult = 0; primero = menor;
  if (mayor == 0) return true;
  ult = mayor;
  posEscr = ((vivo + 1) % sectores) * POR_SECTOR;   // sector lleno: estrena el siguiente
  for (uint32_t i = 1; i < POR_SECTOR; i++) {
    uint32_t q = seqEn(vivo * POR_SECTOR + i);
    if (!vale(q)) { posEscr = vivo * POR_SECTOR + i; break; }
    ult = q;
  }
  return true;
}

void registro_anotar(char clase, const char* texto) {
  if (part == NULL || texto == NULL) return;
  if (pendientes == REGISTRO_ANILLO) {             // anillo lleno: se pisa el mas viejo
    cabeza = (uint8_t)((cabeza + 1) % REGISTRO_ANILLO);
    pendientes--;
    perdidas++;
  }
  Registro* r = &anillo[(cabeza + pendientes) % REGISTRO_ANILLO];
  pendientes++;
  memset(r, 0, sizeof(*r));
  r->up_ms = millis();
  FechaHora fh;
  if (reloj_leer(&fh)) {
    r->fecha[0] = (uint8_t)(fh.anio % 100); r->fecha[1] = (uint8_t)fh.mes; r->fecha[2] = (uint8_t)fh.dia;
    r->fecha[3] = (uint8_t)fh.hora; r->fecha[4] = (uint8_t)fh.minuto; r->fecha[5] = (uint8_t)fh.segundo;
  }
  r->clase = (uint8_t)clase;
  size_t n = 0;
  while (texto[n] && texto[n] != '*' && n < REGISTRO_TEXTO_MAX + 1) n++;
  if (n > REGISTRO_TEXTO_MAX) {
    memcpy(r->texto, texto, REGISTRO_TEXTO_MAX);
    r->texto[REGISTRO_TEXTO_MAX - 1] = '~';
    n = REGISTRO_TEXTO_MAX;
  } else {
    memcpy(r->texto, texto, n);
  }
  r->len = (uint8_t)n;
}

// D-73: con o sin telefono, tras trama_valida() y sellarHoraSiFaltaba(). Nada de bajada.
void registro_verSubida(const char* trama) {
  if (trama == NULL || trama[0] != '$') return;
  if (strncmp(trama, "$STATUS,", 8) != 0) {
    if (strstr(trama, REGISTRO_EXCLUIR) == NULL) registro_anotar('E', trama + 1);
    return;
  }
  char modo[24] = "", estado[24] = "";
  trama_campo(trama, "MODO:", modo, sizeof(modo));
  trama_campo(trama, "ESTADO:", estado, sizeof(estado));
  if (strcmp(modo, modoVisto) == 0 && strcmp(estado, estadoVisto) == 0) return;
  strcpy(modoVisto, modo);
  strcpy(estadoVisto, estado);
  registro_anotar('S', trama + 1);
}

// D-72: un registro por vuelta como mucho; si estrena sector, antes lo borra (pisa los
// 32 mas viejos). Fallo: se cuenta y se tira, sin reintento. Sin delay() ni esperas.
// D-100: seq y ranura se consumen SOLO tras ESP_OK; un borrado fallido deja posEscr en
// la cabeza del sector, y el siguiente registro lo vuelve a borrar antes de escribir.
void registro_volcar() {
  if (part == NULL || pendientes == 0) return;
  Registro* r = &anillo[cabeza];
  cabeza = (uint8_t)((cabeza + 1) % REGISTRO_ANILLO);
  pendientes--;
  uint32_t ranura = posEscr;
  r->seq = ult + 1;
  r->crc = crc16((const uint8_t*)r, REGISTRO_REG_B - 2);
  if (ranura % POR_SECTOR == 0) {
    bool teniaDatos = vale(seqEn(ranura));
    if (esp_partition_erase_range(part, (ranura / POR_SECTOR) * SECTOR_B, SECTOR_B) != ESP_OK) {
      fallos++;
      return;
    }
    if (teniaDatos) {
      uint32_t sig = seqEn((ranura + POR_SECTOR) % ranuras);
      primero = vale(sig) ? sig : r->seq;
    }
  }
  if (esp_partition_write(part, ranura * REGISTRO_REG_B, r, REGISTRO_REG_B) != ESP_OK) {
    fallos++;
    return;
  }
  ult = r->seq;
  posEscr = (ranura + 1) % ranuras;
  if (primero == 0) primero = r->seq;
}

// Lee el registro de seq dado: las ranuras van seguidas, una por seq, hasta ult.
static bool leer(uint32_t seq, Registro* r) {
  uint32_t atras = ult - seq;
  uint32_t ranura = (posEscr + ranuras - 1 - (atras % ranuras)) % ranuras;
  if (esp_partition_read(part, ranura * REGISTRO_REG_B, r, REGISTRO_REG_B) != ESP_OK) return false;
  return r->seq == seq && r->len <= REGISTRO_TEXTO_MAX &&
         r->crc == crc16((const uint8_t*)r, REGISTRO_REG_B - 2);
}

static void emitirRegistro(const Registro* r) {
  char d[REGISTRO_TEXTO_MAX + 1];
  for (uint8_t i = 0; i < r->len; i++) {           // , -> ;  y  : -> =  (SPEC_4 9.8 punto 7)
    char c = r->texto[i];
    d[i] = (c == ',') ? ';' : (c == ':') ? '=' : c;
  }
  d[r->len] = '\0';
  char fecha[11] = "-", hora[9] = "-";
  if (r->fecha[1] != 0) {
    snprintf(fecha, sizeof(fecha), "20%02u-%02u-%02u",
             (unsigned)r->fecha[0], (unsigned)r->fecha[1], (unsigned)r->fecha[2]);
    snprintf(hora, sizeof(hora), "%02u:%02u:%02u",
             (unsigned)r->fecha[3], (unsigned)r->fecha[4], (unsigned)r->fecha[5]);
  }
  char p[REGISTRO_TEXTO_MAX + 96];
  snprintf(p, sizeof(p), "$EVENT,NODE:PUENTE,EVT:LOG,"
           "N:%lu,UP:%lu,FECHA:%s,HORA:%s,C:%c,D:%s",
           (unsigned long)r->seq, (unsigned long)(r->up_ms / 1000UL), fecha, hora, (char)r->clase, d);
  puente_emitirPropio(p);
}

// D-74: CMD:LOG,DESDE:<N>, sin PIN. Cada $ERR en su if; el $ACK sale DESPUES de leer el
// lote, asi que MALOS ya cuenta lo que este lote encontro roto. Nada se anota ni cruza.
void registro_atender(const char* linea) {
  const char* p = strrchr(linea, ':');               // tras "DESDE:"; el prefijo ya lo miro el despachador
  p = (p != NULL) ? p + 1 : linea;
  size_t k = 0;
  while (p[k] >= '0' && p[k] <= '9') k++;
  if (k == 0 || k > 10 || p[k] != '\0') {
    puente_emitirPropio("$ERR,NODE:PUENTE,CMD:LOG,DESC:FORMATO_INVALIDO");
    return;
  }
  if (part == NULL) {
    puente_emitirPropio("$ERR,NODE:PUENTE,CMD:LOG,DESC:SIN_FLASH");
    return;
  }
  unsigned long long n = strtoull(p, NULL, 10);
  uint32_t a = (n == 0) ? 1 : (n > 0xFFFFFFFEULL ? 0xFFFFFFFEUL : (uint32_t)n);
  if (a < primero) a = primero;
  uint32_t b = 0;
  if (ult != 0 && a <= ult) b = (ult - a >= REGISTRO_LOTE_MAX - 1) ? a + REGISTRO_LOTE_MAX - 1 : ult;
  Registro r;
  for (uint32_t s = a; b != 0 && s <= b; s++) {
    if (!leer(s, &r)) malos++;
  }
  char ack[200];
  snprintf(ack, sizeof(ack), "$ACK,NODE:PUENTE,CMD:LOG,RESULT:OK,"
           "DESDE:%lu,HASTA:%lu,ULT:%lu,PRIMERO:%lu,"
           "CONG:%lu,CRC:%lu,LARGO:%lu,ANILLO:%lu,FLASH:%lu,MALOS:%lu",
           (unsigned long)a, (unsigned long)b, (unsigned long)ult, (unsigned long)primero,
           transporte_descartadasPorCongestion(), puente_descartadasPorCrc(), puente_descartadasPorLargo(),
           perdidas, fallos, malos);
  puente_emitirPropio(ack);
  for (uint32_t s = a; b != 0 && s <= b; s++) {
    if (leer(s, &r)) emitirRegistro(&r);
  }
}

uint32_t registro_ultimo()              { return ult; }
uint32_t registro_primero()             { return primero; }
unsigned long registro_perdidasAnillo() { return perdidas; }
unsigned long registro_fallosFlash()    { return fallos; }
unsigned long registro_malos()          { return malos; }
