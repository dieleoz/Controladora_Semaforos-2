// ===== 01_Firmware/ESP32_Expansion/src/transporte_app.cpp =====

#include "transporte_app.h"
#include "contrato.h"
#include "trama.h"
#include <BluetoothSerial.h>
#include <esp_spp_api.h>
#include <Preferences.h>
#include <string.h>

static BluetoothSerial spp;
static Preferences memoria;
static bool abierto = false;
static char rotulo[ROTULO_MAX];

// D-70 (SPEC_4 9.7): estado de congestion del SPP, observado desde la tarea de la pila
// Bluetooth. Con el telefono sin leer, BluetoothSerial::write() espera hasta 1000 ticks
// por trama a que haya sitio en su cola: dos tramas en una vuelta de loop() son el techo
// del perro (2000 ms). Lo que no se escribe por congestion se cuenta, no se espera.
static volatile bool congestionado = false;
static unsigned long descartadasCongestion = 0;

// Observador, como transporte_aprenderRotulo(): no toca tramas ni decide nada mas.
static void observadorSpp(esp_spp_cb_event_t evento, esp_spp_cb_param_t* param) {
  if (evento == ESP_SPP_CONG_EVT) {
    congestionado = param->cong.cong;
  } else if (evento == ESP_SPP_WRITE_EVT) {
    congestionado = param->write.cong;
  } else if (evento == ESP_SPP_CLOSE_EVT || evento == ESP_SPP_SRV_OPEN_EVT) {
    congestionado = false;   // sesion nueva: sin deuda de la anterior
  }
}

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
  spp.register_callback(observadorSpp);   // antes de begin(): D-70
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
  // D-70: con el SPP congestionado el telefono no lee; llamar a write() solo esperaria.
  // D-71: sin espera de ningun tipo aqui (ni delay, ni millis, ni alimentar al perro).
  if (congestionado) {
    descartadasCongestion++;
    return 0;
  }
  return spp.write((const uint8_t*)datos, n);
}

unsigned long transporte_descartadasPorCongestion() {
  return descartadasCongestion;
}

void transporte_aprenderRotulo(const char* trama) {
  // Solo del $STATUS: es la unica trama que lleva SERIE: y NODE: a la vez, y la emiten
  // las dos puntas cada segundo. No se toca la trama ni se decide nada sobre ella.
  if (trama == NULL || strncmp(trama, "$STATUS,", 8) != 0) return;

  char serie[16];
  char nodo[16];
  if (!trama_campo(trama, "SERIE:", serie, sizeof(serie))) return;
  if (!trama_campo(trama, "NODE:", nodo, sizeof(nodo))) return;

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
