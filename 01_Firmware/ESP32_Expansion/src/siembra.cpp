// ===== 01_Firmware/ESP32_Expansion/src/siembra.cpp =====
//
// D-20 / D-26 (11/09): LA HORA DEL DS3231 HACIA EL STM32.
//
// POR QUE VIVE EN SU PROPIO FICHERO Y NO EN puente.cpp NI EN enlace_stm32.cpp.
//
// Por la misma razon que el latido vive en vigilante.cpp: esos dos son EL CAMINO DE
// DATOS y esp32_07 les exige que no tengan reloj (P-1/P-4). Esto necesita millis() para
// la cadencia de D-26, y meter un reloj alli abriria justo lo que esa regla cierra.
//
// 🔴 ESTA ES UNA DE LAS DOS LINEAS QUE EL ESP32 ORIGINA HACIA EL MICRO QUE GOBIERNA EL
// CRUCE -la otra es el latido, N-127-, y la barrera esp32_05 la tiene escrita como su
// UNICA excepcion de orden. Lo que la hace defendible, y lo que el pack mide:
//
//   - el contenido sale SOLO de reloj_leer(): la barrera del DS3231, que no tiene
//     variante "damela igual" y relee el chip en cada llamada. Un DS3231 sin pila da una
//     fecha PERFECTAMENTE FORMADA y falsa; aqui no llega, porque reloj_leer() dice que no
//     y no sale nada -ni un hueco, ni un cero, ni la ultima que se vio-;
//   - ni un byte viene del telefono: despachador.cpp se queda el SET_RTC y descarta
//     cualquier linea del telefono que contenga HORA_ESP32;
//   - el formato es UN literal, entero y con nombre, para que el pack lo lea sin tener
//     que reconstruirlo pegando trozos (la misma decision que FORMATO_PARTE).

#include "siembra.h"
#include "contrato.h"
#include "reloj_ds3231.h"
#include "enlace_stm32.h"
#include <stdio.h>

// El formato del contrato con el STM32, tal cual viaja. enlace_escribirLinea() le pone
// el "\r\n", asi que aqui no va. Los rangos de reloj_rangoValido() -anio 2000..2099 y el
// resto de dos cifras- hacen que salga SIEMPRE con la misma longitud: 34 caracteres, por
// debajo de los 63 utiles del STM32. esp32_13 recalcula esa cota desde este literal.
static const char FORMATO_HORA_ESP32[] = "CMD:HORA_ESP32:%04d-%02d-%02d,%02d:%02d:%02d";

// El calendario de la siembra. No sobrevive al reset, y es a proposito: tras un reset
// hay que volver a sembrar como en un arranque, que es lo que pasa con todo esto a cero.
static bool anclada = false;          // ya salio la primera desde que hay hora fiable
static unsigned long tAncla = 0;      // cuando salio esa primera
static unsigned long tUltima = 0;     // cuando salio la ultima, sea por la causa que sea
static uint8_t reintentosHechos = 0;

static const unsigned long REINTENTOS_MS[] = { SIEMBRA_REINTENTO_1_MS, SIEMBRA_REINTENTO_2_MS };
static const uint8_t REINTENTOS_N = (uint8_t)(sizeof(REINTENTOS_MS) / sizeof(REINTENTOS_MS[0]));

bool siembra_ahora() {
  // EL CALENDARIO SE APUNTA ANTES DE LEER, Y NO ES UN DESCUIDO. Si la lectura falla
  // -bus caido entre reloj_enHora() y reloj_leer()-, apuntarlo despues dejaria a
  // siembra_revisar() reintentando en cada vuelta del bucle. Lo que se pierde es una
  // siembra, que vuelve a salir en la cadencia siguiente o con el proximo SET_RTC.
  const unsigned long ahora = millis();
  if (!anclada) {
    anclada = true;
    tAncla = ahora;
  }
  tUltima = ahora;

  // LA BARRERA DECIDE. Si el DS3231 no tiene hora fiable, no sale nada: un STM32 sin
  // siembra se queda con la que tenga -y la declara vieja por su lado- mientras que un
  // STM32 sembrado con una hora falsa se autorizaria el Degradado sobre ella (N-144).
  FechaHora leida;
  if (!reloj_leer(&leida)) return false;

  // Solo campos de `leida`: ni un literal de hora, ni un byte que no haya pasado por
  // reloj_leer(). snprintf trunca en silencio y devuelve lo que HABRIA escrito, asi que
  // una linea que no cupiera no sale -una orden recortada que casa con otra mas corta es
  // el accidente de E-2-.
  char linea[TRAMA_MAX_UTIL + 1];
  int n = snprintf(linea, sizeof(linea), FORMATO_HORA_ESP32,
                   leida.anio, leida.mes, leida.dia,
                   leida.hora, leida.minuto, leida.segundo);
  if (n <= 0 || (size_t)n >= sizeof(linea)) return false;

  // enlace_escribirLinea() devuelve los bytes puestos, o 0 si no cabia o fallo la
  // escritura. `true` es "salio entera", no "el STM32 la acepto": ver siembra.h.
  return enlace_escribirLinea(linea, (size_t)n) > 0;
}

void siembra_revisar() {
  // Sin hora fiable no hay nada que sembrar. Se pregunta a la barrera y no a una bandera
  // propia: la hora puede dejar de ser fiable en marcha -R-4, la pila- y volver con un
  // SET_RTC, y el unico que lo sabe es reloj_ds3231.cpp.
  if (!reloj_enHora()) return;

  const unsigned long ahora = millis();

  // (1) LA PRIMERA desde que hay hora fiable. El resultado no se mira aqui porque no hay
  // a quien contestarle: esta siembra no la pidio nadie. Lo que no salio se cuenta en
  // enlace_lineasRechazadas() (P-3), y los reintentos de abajo son la respuesta.
  if (!anclada) {
    (void)siembra_ahora();
    return;
  }

  // (1.bis) LOS REINTENTOS DE ARRANQUE, contados desde la primera. El porque de cada
  // numero esta en contrato.h.
  if (reintentosHechos < REINTENTOS_N && ahora - tAncla >= REINTENTOS_MS[reintentosHechos]) {
    reintentosHechos++;
    (void)siembra_ahora();
    return;
  }

  // (3) D-26 (2): CADA SIEMBRA_INTERVALO_MS (~5 min; ~~cada hora, A-15~~) desde la ultima
  // que salio, la haya disparado un SET_RTC o este mismo calendario. La resta sin signo
  // aguanta la vuelta de millis() a los 49,7 dias.
  if (ahora - tUltima >= SIEMBRA_INTERVALO_MS) {
    (void)siembra_ahora();
  }
}
