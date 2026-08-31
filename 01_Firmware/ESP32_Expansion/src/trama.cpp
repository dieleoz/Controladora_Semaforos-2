// ===== 01_Firmware/ESP32_Expansion/src/trama.cpp =====

#include "trama.h"
#include "contrato.h"
#include <string.h>

// El censo del contrato. Se define aqui y no en un .h para que haya UNA sola copia:
// una tabla repetida en dos unidades de traduccion es la segunda copia de siempre.
const char* const PREFIJOS_STM32[PREFIJOS_STM32_N] = {
  "$STATUS", "$ACK", "$ERR", "$ALARM", "$EVENT"
};

uint8_t trama_checksum(const char* desde_tras_el_dolar) {
  uint8_t crc = 0;
  const char* p = desde_tras_el_dolar;
  while (*p && *p != '*') {
    crc ^= (uint8_t)(*p);
    p++;
  }
  return crc;
}

// Un digito hexadecimal a valor, o -1 si no lo es.
//
// Se escribe a mano en vez de usar sscanf("%2x") porque sscanf ACEPTA cosas que aqui
// no valen: espacios delante, un signo, y sobre todo un solo digito donde el contrato
// exige dos. Un validador mas permisivo que el contrato deja pasar justo lo que vino a
// parar, y encima en verde.
static int hex(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  return -1;
}

bool trama_valida(const char* linea) {
  if (linea == NULL || linea[0] != '$') return false;

  const char* ast = strchr(linea, '*');
  if (ast == NULL) return false;

  // Payload vacio: "$*XX" no es una trama, es un checksum sin nada debajo. Se rechaza
  // aqui porque mas abajo su XOR seria 0x00 y casaria con "*00" sin haber medido nada.
  if (ast == linea + 1) return false;

  // Exactamente dos digitos hexadecimales y nada mas. El "nada mas" importa: el
  // terminador ya se quito antes de llamar, asi que cualquier byte extra detras del
  // checksum es una trama pegada a otra o basura, y las dos se descartan igual.
  int alto = hex(ast[1]);
  int bajo = (alto < 0) ? -1 : hex(ast[2]);
  if (alto < 0 || bajo < 0 || ast[3] != '\0') return false;

  return trama_checksum(linea + 1) == (uint8_t)((alto << 4) | bajo);
}

size_t trama_componer(char* destino, size_t capacidad, const char* payload) {
  if (destino == NULL || capacidad == 0) return 0;
  uint8_t crc = trama_checksum(payload + 1);   // Salta el '$', igual que el STM32
  int n = snprintf(destino, capacidad, "%s*%02X\r\n", payload, crc);

  // snprintf trunca en silencio y devuelve lo que HABRIA escrito. Una trama truncada
  // sale bien formada hasta la mitad y con un checksum calculado sobre otra cosa: el
  // receptor la descartaria por CRC y nadie sabria por que. Aqui se prefiere no mandar
  // nada -y que el contador lo diga- antes que mandar algo que miente.
  if (n < 0 || (size_t)n >= capacidad) {
    destino[0] = '\0';
    return 0;
  }
  return (size_t)n;
}
