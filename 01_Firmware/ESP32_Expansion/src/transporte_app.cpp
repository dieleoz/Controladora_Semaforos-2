// ===== 01_Firmware/ESP32_Expansion/src/transporte_app.cpp =====

#include "transporte_app.h"
#include "contrato.h"
#include <BluetoothSerial.h>
#include <Preferences.h>
#include <string.h>

static BluetoothSerial spp;
static Preferences memoria;
static bool abierto = false;
static char rotulo[ROTULO_MAX];

// Lo aprendido en ESTA arrancada, para no reescribir la flash en cada $STATUS: el
// equipo emite uno por segundo, y una escritura por segundo se come la NVS en semanas.
static char aprendido[ROTULO_MAX];

static void cargarRotulo() {
  // El rotulo del arranque sale de lo que se aprendio en arrancadas anteriores. Si no
  // hay nada guardado -modulo virgen, o STM32 que nunca hablo- se usa el provisional,
  // Y SE VE: aparece asi en la lista de emparejados de Android en vez de disfrazarse de
  // un equipo cualquiera. Un rotulo inventado es peor que uno que admite que no sabe.
  memoria.begin("puente", true);          // solo lectura
  String guardado = memoria.getString("rotulo", "");
  memoria.end();

  if (guardado.length() > 0 && guardado.length() < ROTULO_MAX) {
    strncpy(rotulo, guardado.c_str(), ROTULO_MAX - 1);
  } else {
    strncpy(rotulo, ROTULO_PROVISIONAL, ROTULO_MAX - 1);
  }
  rotulo[ROTULO_MAX - 1] = '\0';
  aprendido[0] = '\0';
}

bool transporte_abrir() {
  cargarRotulo();
  abierto = spp.begin(rotulo);
  return abierto;
}

bool transporte_conectado() {
  return abierto && spp.hasClient();
}

int transporte_disponible() {
  return abierto ? spp.available() : 0;
}

int transporte_leer() {
  return abierto ? spp.read() : -1;
}

size_t transporte_escribir(const char* datos, size_t n) {
  if (!abierto || datos == NULL || n == 0) return 0;
  // Sin telefono conectado no se escribe: BluetoothSerial acepta la llamada y la tira,
  // y contarla como entregada haria que el contador de descartes -que es lo unico que
  // se puede mirar desde fuera- mintiera justo cuando hace falta.
  if (!spp.hasClient()) return 0;
  return spp.write((const uint8_t*)datos, n);
}

// Copia el valor de un campo "CLAVE:" de una trama NMEA-like hasta la coma o el '*'.
// Devuelve false si el campo no esta o no cabe: sin valor por defecto, porque un
// rotulo a medias es un rotulo equivocado y este lo va a leer un tecnico para decidir
// a que poste se conecta.
static bool campo(const char* trama, const char* clave, char* dst, size_t cap) {
  const char* p = strstr(trama, clave);
  if (p == NULL) return false;
  p += strlen(clave);
  size_t i = 0;
  while (p[i] && p[i] != ',' && p[i] != '*' && i < cap - 1) {
    dst[i] = p[i];
    i++;
  }
  if (i == 0) return false;
  dst[i] = '\0';
  return true;
}

void transporte_aprenderRotulo(const char* trama) {
  // Solo del $STATUS: es la unica trama que lleva SERIE: y NODE: a la vez, y la emiten
  // las dos puntas cada segundo. No se toca la trama ni se decide nada sobre ella.
  if (trama == NULL || strncmp(trama, "$STATUS,", 8) != 0) return;

  char serie[16];
  char nodo[16];
  if (!campo(trama, "SERIE:", serie, sizeof(serie))) return;
  if (!campo(trama, "NODE:", nodo, sizeof(nodo))) return;

  // La letra final la decide el propio equipo, no una opcion de compilacion: el mismo
  // binario sirve a las dos puntas. Un firmware distinto por punta seria una segunda
  // copia que alguien tendria que sincronizar, y el dia que se cruzaran los dos postes
  // se llamarian igual.
  char letra;
  if (strcmp(nodo, "MAESTRO") == 0)      letra = 'M';
  else if (strcmp(nodo, "ESCLAVO") == 0) letra = 'E';
  else return;   // Un NODE: que no se reconoce no se rotula a medias

  char candidato[ROTULO_MAX];
  snprintf(candidato, sizeof(candidato), "%s%s-%c", ROTULO_PREFIJO, serie, letra);

  if (strcmp(candidato, aprendido) == 0) return;   // ya visto en esta arrancada
  strncpy(aprendido, candidato, ROTULO_MAX - 1);
  aprendido[ROTULO_MAX - 1] = '\0';

  if (strcmp(candidato, rotulo) == 0) return;      // ya es el que estamos usando

  // Se guarda para la SIGUIENTE arrancada. No se re-rotula en caliente: cambiar el
  // nombre SPP obliga a cerrar y reabrir el perfil, o sea a tirar la sesion del
  // operario que en ese momento puede estar dando una orden al cruce.
  memoria.begin("puente", false);
  memoria.putString("rotulo", candidato);
  memoria.end();
}

const char* transporte_rotulo() {
  return rotulo;
}
