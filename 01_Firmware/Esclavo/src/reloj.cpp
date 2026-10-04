// ===== src/reloj.cpp (ESCLAVO) =====
#include "reloj.h"
#include "protocolo.h"       // D-26 (3): SFTY6_SILENCIO_MS, la definicion de "sin radio"
#include <stm32f1xx_hal.h>   // N-17: arranque acotado del cristal, ver abajo

// ---------------------------------------------------------------------------
// SFTY-18 / SFTY-23 — Reloj de tiempo real sobre el RTC interno del STM32.
//
// D-20: LA AUTORIDAD DE LA HORA ES EL ESP32 (DS3231).
// El STM32 no tiene cristal de 32.768 kHz garantizado (Y2 muerto, N-17).
// Por tanto, reloj_ajustar() siembra una base de software extrapolada con millis(),
// asegurando que reloj_enHora() sea true y la hora avance una vez sembrada desde la
// radio o el ESP32. Desde el 11/09 (N-162) la siembra YA NO escribe el RTC hardware: ver
// el porque al final de reloj_ajustarConAcuse().
// ---------------------------------------------------------------------------

// N-172: SIN LIBRERIA DEL RTC, gemelo del Maestro (el porque, alli): la libreria reescribia
// DR6/DR7 de la pila y plegaba el contador al dia. El RTC es SOLO el contador.
static const uint32_t ESPERA_RTC_MS = 5;   // RTOFF y RSF tardan 1-3 ciclos de 32 kHz

static bool horaValida = false;
static bool rtcOperativo = false;

// D-20: Base de tiempo de software extrapolada por millis()
static uint32_t tBaseMillis = 0;
static uint32_t segBaseDelDia = 0;
static uint8_t diaBase = 1;
// D-35: dias desde la epoca del DS3231 (su anio 00) del instante base, sacados de la FECHA que trae
// CMD:HORA_ESP32 (el DS3231). 0 = esta base no tiene fecha. Gemela de la del Maestro.
static uint16_t diaAbsBase = 0;

static uint16_t diasDesde2000(int anio, int mes, int dia) {
  if (anio < 2000 || anio > 2150 || mes < 1 || mes > 12 || dia < 1 || dia > 31) return 0;
  static const uint16_t ACUM[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
  uint32_t d = 365UL * (uint32_t)(anio - 2000) + (uint32_t)((anio - 2000 + 3) / 4);
  d += ACUM[mes - 1] + (uint32_t)(dia - 1);
  if (mes > 2 && (anio % 4) == 0) d += 1;   // 2000..2150: la regla de los siglos solo muerde en 2100
  if (anio > 2100 || (anio == 2100 && mes > 2)) d -= 1;
  return (uint16_t)(d + 1);                  // +1: el 0 queda como "sin fecha"
}

// Segundos desde la epoca del DS3231 (su anio 00) con la fecha del DS3231, o 0 si esta base no la tiene. Una
// sola lectura de millis() para dia y segundo. Declarada en modo_degradado.h.
uint32_t reloj_segundosDesde2000() {
  if (!horaValida || tBaseMillis == 0 || diaAbsBase == 0) return 0;
  const uint32_t t = segBaseDelDia + (millis() - tBaseMillis) / 1000UL;
  return (uint32_t)(diaAbsBase - 1U) * 86400UL + t;
}

// D-21 (1): la base pasada de HORA_CADUCA_MS, con cerrojo. Gemela de la del Maestro.
static bool siembraCaducada = false;

// D-26 (3) - DE QUIEN ES LA HORA QUE HAY. Ver el porque en reloj.h, sobre
// reloj_radioManda(). Ordenadas de menos a mas autoridad, y el orden SE USA: la del ESP32
// entra si la que hay es de menos autoridad que la radio, o si la radio calla.
//
// La marcan SOLO los que ponen la hora -reloj_ajustar() (la radio) y reloj_sembrarDesdeIso()
// (el ESP32); el RTC ya no da hora (N-172)-, y SIEMPRE junto a horaValida: una fuente que
// dijera RADIO sobre una hora que no entro seria otra vez el retorno que no depende de la
// llamada (N-160).
enum FuenteHora : uint8_t { FH_NINGUNA = 0, FH_ESP32, FH_RADIO };
static FuenteHora fuenteHora = FH_NINGUNA;

// La ultima trama valida del Maestro, de cualquier comando. Ver reloj_notarRadio().
static bool radioOida = false;
static uint32_t tUltimaRadio = 0;

// Espera acotada a que arranque el oscilador del cristal Y2.
static const uint32_t ESPERA_LSE_MS = 2000;

// ---------------------------------------------------------------------------
// 1.22 - EL CRISTAL QUE ARRANCA Y NO CUENTA. Gemelo del Maestro letra por letra salvo la
// fuente de hora, que en esta punta existe y cae con la hora. El porque entero y de que
// sale el plazo estan en reloj.h, sobre CNT_TICK_MS y CNT_VENTANA_MS.
//
// EL CERROJO. Una vez MEDIDO que el contador no avanza, este cristal no se vuelve a
// adoptar: el reintento de N-25 solo mira LSERDY, que en este cristal vale 1 -es justo el
// estado que engana-, asi que sin cerrojo volveria a subir rtcOperativo a los 30 s y
// desharia la cura en silencio. Aqui solo lo quita reloj_setup(): esta punta no tiene
// reloj_reiniciarDominioRespaldo().
static bool cristalCongelado = false;
static uint32_t cntMuestra = 0;
static uint32_t tCntMuestra = 0;
// 1.49: los flancos vistos desde que se adopto el cristal, saturados en 2. Es lo que separa
// VIGILANDO de CUENTA en reloj_estadoCristal(); ver reloj.h.
static uint8_t flancosVistos = 0;

// CNT en crudo, con la doble lectura de CNTH que exige el silicio -la pareja CNTH/CNTL
// puede cruzar un flanco entre las dos mitades-. NO aplica el disfraz "v == 0 -> 1" de
// reloj_contadorSegundos(): ese es de aquella funcion y aqui ESTORBA, porque un contador
// congelado EN CERO es exactamente lo que hay que poder ver.
static uint32_t leerCnt() {
  uint16_t alta = (uint16_t)RTC->CNTH;
  const uint16_t baja = (uint16_t)RTC->CNTL;
  if ((uint16_t)RTC->CNTH != alta) alta = (uint16_t)RTC->CNTH;
  return ((uint32_t)alta << 16) | baja;
}

// Se ancla en los DOS sitios que suben rtcOperativo, y no en la declaracion de las
// estaticas: con cntMuestra en 0 de fabrica, un contador congelado justo en 0 se declararia
// parado sin haber medido ninguna ventana -acertando por casualidad, que no es medir-.
static void anclarVigilancia() {
  cntMuestra = leerCnt();
  tCntMuestra = HAL_GetTick();
  flancosVistos = 0;   // 1.49: el veredicto empieza de cero con cada adopcion
}

// LA SEGUNDA VISITA, hecha por el firmware y no por un tecnico. La llama reloj_actualizar()
// en cada vuelta del bucle.
static void vigilarCristal() {
  if (!rtcOperativo || cristalCongelado) return;

  const uint32_t ahora = HAL_GetTick();
  const uint32_t cnt = leerCnt();

  if (cnt != cntMuestra) {   // conto: se reancla y no hay nada mas que mirar
    cntMuestra = cnt;
    tCntMuestra = ahora;
    if (flancosVistos < 2) flancosVistos++;   // 1.49: el segundo es el que da CUENTA
    return;
  }
  if ((uint32_t)(ahora - tCntMuestra) < CNT_VENTANA_MS) return;  // la ventana sigue abierta

  cristalCongelado = true;
  rtcOperativo = false;   // reloj_contadorSegundos() vuelve a devolver 0: ESA ES LA CURA

  // N-172: la hora no cae con el. Solo la da una siembra (radio o ESP32), que no sale de
  // este contador y la cubre el plazo de D-21 (1).
}

static bool arrancarCristal() {
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_RCC_BKP_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess();

  if (__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY) != RESET) return true;

  __HAL_RCC_LSE_CONFIG(RCC_LSE_ON);

  const uint32_t t0 = HAL_GetTick();
  while (__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY) == RESET) {
    if (HAL_GetTick() - t0 > ESPERA_LSE_MS) {
      return false;
    }
  }
  return true;
}

static void esperarRtc(uint32_t bit) {
  const uint32_t t0 = HAL_GetTick();
  while ((RTC->CRL & bit) == 0 && HAL_GetTick() - t0 <= ESPERA_RTC_MS) {}
}

// N-172: gemela letra por letra de la del Maestro (el porque, alli).
static bool configurarRtc() {
  HAL_PWR_EnableBkUpAccess();
  const uint32_t fuente = __HAL_RCC_GET_RTC_SOURCE();
  if (fuente != 0 && fuente != RCC_RTCCLKSOURCE_LSE) return false;
  if (fuente == 0) __HAL_RCC_RTC_CONFIG(RCC_RTCCLKSOURCE_LSE);
  __HAL_RCC_RTC_ENABLE();
  RTC->CRL &= ~RTC_CRL_RSF;   // tras un reinicio, CNT no se lee hasta resincronizar
  esperarRtc(RTC_CRL_RSF);
  if (fuente == 0) {
    esperarRtc(RTC_CRL_RTOFF);
    RTC->CRL |= RTC_CRL_CNF;
    RTC->PRLH = 0;
    RTC->PRLL = 0x7FFF;       // 32768 Hz / (0x7FFF + 1) = 1 Hz
    RTC->CRL &= ~RTC_CRL_CNF;
    esperarRtc(RTC_CRL_RTOFF);
  }
  return true;
}

void reloj_setup() {
  horaValida = false;
  rtcOperativo = false;
  tBaseMillis = 0;
  segBaseDelDia = 0;
  diaBase = 1;
  diaAbsBase = 0;   // D-35: sin base no hay dia
  fuenteHora = FH_NINGUNA;
  siembraCaducada = false;
  cristalCongelado = false;   // 1.22: el arranque del periferico es lo que quita el cerrojo

  if (!arrancarCristal() || !configurarRtc()) return;
  rtcOperativo = true;                      // N-24: a partir de aqui el RTC cuenta
  anclarVigilancia();                       // 1.22: y a partir de aqui se vigila que CUENTE
  // N-172: la hora NO sale del RTC. Llega con la radio o con el ESP32 (D-20, D-26 (3)).
}

static const unsigned long REINTENTO_LSE_MS = 30000;
static uint32_t tUltimoReintento = 0;

void reloj_actualizar() {
  // D-21 (1): el cerrojo de la caducidad, en CADA vuelta. Ver la gemela del Maestro.
  (void)reloj_horaFiable();

  // 1.22 - LA SEGUNDA VISITA AL CONTADOR, Y VA ANTES DE LA SALIDA TEMPRANA: el caso que
  // hay que vigilar es justo rtcOperativo == true, que es por donde esta funcion se va.
  vigilarCristal();

  if (rtcOperativo) return;

  // 1.22 - Y UN CRISTAL YA MEDIDO COMO PARADO NO SE VUELVE A ADOPTAR. El reintento de
  // abajo solo mira LSERDY, y en este cristal LSERDY vale 1: sin este cerrojo readoptaria
  // el cristal cada 30 s y desharia la cura sin decir nada. Ver vigilarCristal().
  if (cristalCongelado) return;

  const uint32_t ahora = HAL_GetTick();
  if (ahora - tUltimoReintento < REINTENTO_LSE_MS) return;
  tUltimoReintento = ahora;

  if (__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY) == RESET) return;

  // N-160/N-162/N-172: se adopta el CONTADOR y nada mas, gemelo del Maestro. reloj_dia() se
  // queda sin lector en esta punta -lo apunta costura_10-.
  if (!configurarRtc()) return;
  rtcOperativo = true;
  anclarVigilancia();   // 1.22: se adopta el cristal Y se empieza a medir si CUENTA
}

bool reloj_enHora() { return horaValida; }

// 1.49 - VER reloj.h. Gemela de la del Maestro: se deduce de rtcOperativo -lo baja el cerrojo
// de 1.22 o un arranque fallido- y de los flancos que cuenta vigilarCristal().
// 1.49b3: el cerrojo PRIMERO, gemela de la del Maestro.
EstadoCristal reloj_estadoCristal() {
  if (cristalCongelado) return RELOJ_CRISTAL_CONGELADO;
  if (!rtcOperativo) return RELOJ_CRISTAL_SIN_CRISTAL;
  return flancosVistos >= 2 ? RELOJ_CRISTAL_CUENTA : RELOJ_CRISTAL_VIGILANDO;
}

// D-21 (1) - VER reloj.h. Gemela letra por letra de la del Maestro: la siembra que la
// renueva es cualquiera que haya pasado por reloj_ajustarConAcuse() -la radio o el ESP32-.
bool reloj_horaFiable() {
  if (!horaValida) return false;
  if (tBaseMillis == 0) return true;
  if ((uint32_t)(millis() - tBaseMillis) > HORA_CADUCA_MS) siembraCaducada = true;
  return !siembraCaducada;
}

// N-49 — el contador crudo del RTC, en segundos.
uint32_t reloj_contadorSegundos() {
  if (rtcOperativo) {
    uint16_t alta = (uint16_t)RTC->CNTH;
    const uint16_t baja = (uint16_t)RTC->CNTL;
    if ((uint16_t)RTC->CNTH != alta) alta = (uint16_t)RTC->CNTH;
    const uint32_t v = ((uint32_t)alta << 16) | baja;
    return v == 0 ? 1UL : v;
  }

  // N-160 - SIN CRISTAL SE DEVUELVE 0, Y ESO ES A PROPOSITO. NO SE EXTRAPOLA AQUI.
  //
  // Este cero es el "no hay reloj" del que cuelgan los DOS centinelas de respaldo.cpp:
  //   respaldo_marcarSync():      "if (segundosRtc == 0) return;"
  //   respaldo_horasDesdeSync():  "if (segundosRtcAhora == 0) return RESPALDO_SYNC_CADUCADA;"
  //
  // D-20 le habia puesto aqui una extrapolacion con millis(), y eso APAGABA a los dos:
  // con horaValida en true y sin cristal -que es el caso NORMAL que D-20 crea- el cero
  // no salia nunca. Y el valor no sirve como contador monotono por dos motivos
  // independientes: millis() vuelve a cero tras un corte de energia, y tBaseMillis se
  // reasigna en CADA siembra, o sea que el contador BAJA cada vez que llega la hora del
  // Maestro por CMD_HORA_S -aqui eso pasa cada vez que el Maestro reenvia-.
  // respaldo_horasDesdeSync() esta escrita sobre la premisa contraria -"una resta de dos
  // contadores monotonos... el contador no vuelve"-, asi que la resta pasaba a mentir:
  // marca guardada en 31, seis meses de corte, arranque nuevo, siembra a los 40 s de
  // uptime -> contador 41 -> (41-31)/3600 = 0 horas, o sea "sincronizado hace un rato"
  // sobre un acuerdo de hace medio ano. De esa cuenta cuelga el limite duro de 48 h del
  // Modo Degradado, que es el modo que da VERDES sin confirmar la otra punta.
  //
  // CONSECUENCIA QUE SE ASUME, escrita para que nadie la descubra por sorpresa: sin
  // cristal el Degradado NO SE REANUDA tras un corte, porque la marca sale CADUCADA.
  // Ya estaba asi con Y2 muerto; lo que esto impide es que se autorice sobre una marca
  // que no significa nada. Se prefiere la puerta CERRADA a un verde mal fechado.
  //
  // LO QUE SI SIGUE EXTRAPOLANDO CON millis() ES D-20 Y SE QUEDA COMO ESTA:
  // reloj_segundosDelDia() y reloj_hora()/minuto()/segundo()/dia() tienen su propia
  // cuenta y NO pasan por aqui. Este contador es solo el que fecha el respaldo.
  return 0;
}

uint32_t reloj_segundosDelDia() {
  if (!horaValida) return 0;
  // D-20: si hay base de software sembrada (tBaseMillis > 0), manda la extrapolacion
  // por millis() pues el cristal Y2 de 32 kHz no esta garantizado (N-17) y puede no oscilar.
  if (tBaseMillis > 0) {
    const uint32_t deltaS = (millis() - tBaseMillis) / 1000UL;
    return (segBaseDelDia + deltaS) % 86400UL;
  }
  return 0;
}

uint8_t reloj_hora() {
  if (!horaValida) return 0;
  if (tBaseMillis > 0) return (uint8_t)(reloj_segundosDelDia() / 3600UL);
  return 0;
}

uint8_t reloj_minuto() {
  if (!horaValida) return 0;
  if (tBaseMillis > 0) return (uint8_t)((reloj_segundosDelDia() % 3600UL) / 60UL);
  return 0;
}

uint8_t reloj_segundo() {
  if (!horaValida) return 0;
  if (tBaseMillis > 0) return (uint8_t)(reloj_segundosDelDia() % 60UL);
  return 0;
}

uint8_t reloj_dia() {
  if (!horaValida) return 0;
  if (tBaseMillis > 0) {
    const uint32_t deltaDias = (segBaseDelDia + ((millis() - tBaseMillis) / 1000UL)) / 86400UL;
    uint32_t d = (uint32_t)diaBase + deltaDias;
    while (d > 31) d -= 31;
    return (uint8_t)(d == 0 ? 1 : d);
  }
  return 1;
}

// D-20 / N-160: aqui vive la regla de rango, y en ningun otro sitio. Ver reloj.h.
bool reloj_ajustarConAcuse(int hora, int minuto, int segundo, int dia) {
  // Los limites se miran sobre el int, ANTES de castear: un 256 casteado a uint8_t
  // entra como 0 y pasaria por medianoche. Y el negativo hay que mirarlo porque
  // sscanf("%d") acepta "-5" sin protestar.
  if (hora < 0 || hora > 23) return false;
  if (minuto < 0 || minuto > 59) return false;
  if (segundo < 0 || segundo > 59) return false;
  if (dia < 0 || dia > 31) return false;

  // D-35: una hora sin fecha (la radio) no pierde el dia del DS3231: se conserva el dia en
  // curso y, si el ajuste cruza la medianoche, se toma el dia mas cercano.
  if (diaAbsBase != 0 && tBaseMillis != 0) {
    const uint32_t t = segBaseDelDia + (millis() - tBaseMillis) / 1000UL;
    uint32_t d = (uint32_t)diaAbsBase + t / 86400UL;
    const int32_t antes = (int32_t)(t % 86400UL);
    const int32_t nuevo = hora * 3600L + minuto * 60L + segundo;
    if (nuevo - antes < -43200L) d += 1;
    else if (nuevo - antes > 43200L && d > 1) d -= 1;
    diaAbsBase = (uint16_t)d;
  }

  // D-20: Siembra de la base de software (independiente de si Y2 oscila)
  segBaseDelDia = (uint32_t)hora * 3600UL + (uint32_t)minuto * 60UL + (uint32_t)segundo;
  tBaseMillis = millis();
  if (dia >= 1) {
    diaBase = (uint8_t)dia;
  } else if (diaBase < 1 || diaBase > 31) {
    diaBase = 1;
  }
  horaValida = true;
  siembraCaducada = false;   // D-21 (1): una siembra buena es lo unico que la rejuvenece

  // N-162 (11/09) - LA SIEMBRA YA NO ESCRIBE EL RTC HARDWARE. Aqui habia un bloque
  // "if (rtcOperativo) { rtc.setHours(); rtc.setMinutes(); rtc.setSeconds(); ... }", y
  // se quita por tres motivos medidos, identicos en las dos puntas:
  //
  //   1. BLOQUEABA ~3 s POR SIEMBRA con el RTC parado. Cada rtc.setX() acaba en
  //      HAL_RTC_SetTime() -> RTC_WriteTimeCounter(), que espera RTOFF en
  //      RTC_EnterInitMode() y RTC_ExitInitMode() con RTC_TIMEOUT_VALUE = 1000 ms
  //      (stm32f1xx_hal_rtc.c/.h). Sin reloj en el RTC, RTOFF no vuelve nunca: 1 s en la
  //      salida de setHours y 1 s en la entrada de setMinutes y de setSeconds. La cinta
  //      del Sisga (179DB0) lo tiene: el $EVENT de la siembra sale +3 s despues de cada
  //      SET_RTC, las cuatro veces. Con el perro en 4 s, dos siembras en la misma vuelta
  //      de loop() reinician el cruce, y cada una congela coordinador y radio 3 s.
  //   2. NO TENIA LECTOR DE HORA: desde c51cc85 los getters miran primero la base de
  //      software, y D-20 dice que al STM32 no se le pregunta la hora.
  //   3. ROMPIA EL CONTADOR DEL RESPALDO: rtc.setX() reescribe CNT con los segundos del
  //      dia, y reloj_contadorSegundos() lo lee como contador MONOTONO para las 48 h del
  //      Degradado (N-49). Una reescritura hacia atras la caza respaldo_horasDesdeSync()
  //      -"ahora < guardado" -> CADUCADA, puerta cerrada-, pero una que deja el contador
  //      por debajo de lo que habria contado y POR ENCIMA de la marca no la caza nadie, y
  //      rejuvenece la marca. Con una siembra cada ~5 min (D-26) seria cada 5 min.
  //
  // LO QUE SE PIERDE, dicho para que no se descubra en campo: el RTC hardware ya no
  // guarda la hora del ESP32 a traves de un corte. La trae el ESP32 al arrancar (A-15).
  //
  // N-160: se llego al final, o sea que la hora quedo puesta de verdad. Este true es
  // lo unico que autoriza a contestar $ACK; cualquier salida de arriba dice false.
  return true;
}

// N-160: envoltorio. Existe para NO cambiar la firma que doblan los arneses (ver
// reloj.h) y para que los llamadores que no miran el retorno -la rama CMD_HORA_S de
// main.cpp- sigan compilando sin tocarlos. No repite la guarda: la unica copia de la
// regla de rango esta en reloj_ajustarConAcuse().
//
// D-26 (3): y es la que marca la hora como DE RADIO, dentro del `if` y no fuera: una
// trama que la regla de rango tiro no puede dejar la fuente diciendo RADIO sobre la hora
// de antes. Que su unico llamador sea la radio lo recalcula reloj_03.
void reloj_ajustar(uint8_t hora, uint8_t minuto, uint8_t segundo, uint8_t dia) {
  if (reloj_ajustarConAcuse((int)hora, (int)minuto, (int)segundo, (int)dia)) {
    fuenteHora = FH_RADIO;
  }
}

// D-26 (3) - VER reloj.h. Cualquier trama valida del Maestro, de cualquier comando: lo
// que se pregunta es si su radio LLEGA, no si gobierna.
void reloj_notarRadio() {
  radioOida = true;
  tUltimaRadio = millis();
}

// D-26 (3): la radio manda si la hora que hay es SUYA y su radio se oyo dentro del mismo
// silencio con el que main.cpp declara la orfandad (SFTY6_SILENCIO_MS, protocolo.h). Un
// solo `return` y sin escribir nada, a proposito: la rama de bluetooth.cpp la consulta
// ANTES del sembrador y reloj_02 solo se lo permite a una lectura pura.
//
// ">= FH_RADIO" y no "== FH_RADIO": hoy es lo mismo -no hay fuente por encima-, y el dia
// que la haya tiene que mandar tambien sobre el ESP32. Con la hora del RTC o de nadie,
// la del ESP32 entra aunque haya radio: todavia no hay hora del Maestro a la que hacer
// caso, y la radio la pisara en cuanto llegue (D-20: la radio sobrescribe siempre).
bool reloj_radioManda() {
  return fuenteHora >= FH_RADIO && radioOida &&
         (uint32_t)(millis() - tUltimaRadio) <= SFTY6_SILENCIO_MS;
}

// N-162 (11/09) - LA HORA SOLO SE LEE SI TIENE EXACTAMENTE LA FORMA QUE EL ESP32 COMPONE.
//
// "YYYY-MM-DD,HH:MM:SS": 19 caracteres, cifras donde van cifras, separadores donde van
// separadores y NADA detras. sscanf("%d") no lo garantiza: acepta signos, espacios y
// cifras de menos, y NO MIRA lo que sobra. Este STM32 no comprueba checksum de entrada,
// asi que una linea truncada y pegada a la siguiente -"...12:00:0" + "$LATIDO"- daba
// seis campos con los segundos mal, y una truncada a secas -"...12:00:0"- tambien. Con
// el patron las dos se rechazan, y quien llama lo escribe en el diario. Identico en las
// dos puntas: la linea la compone el mismo firmware de ESP32 en los dos postes.
static const char PATRON_ISO[] = "0000-00-00,00:00:00";

static bool isoBienFormado(const char* s) {
  for (uint8_t i = 0; i < sizeof(PATRON_ISO) - 1; i++) {
    const char p = PATRON_ISO[i];
    const char c = s[i];
    // El '\0' de una cadena corta no es cifra ni separador: sale aqui, sin leer detras.
    if (p == '0' ? (c < '0' || c > '9') : (c != p)) return false;
  }
  return s[sizeof(PATRON_ISO) - 1] == '\0';
}

bool reloj_sembrarDesdeIso(const char* str) {
  if (str == nullptr || !isoBienFormado(str)) return false;
  int anio = 0, mes = 0, dia = 0, h = 0, m = 0, s = 0;
  if (sscanf(str, "%d-%d-%d,%d:%d:%d", &anio, &mes, &dia, &h, &m, &s) != 6) return false;

  // D-20 / N-160: el retorno es EL DE LA LLAMADA, no un true fijo. Antes se devolvia
  // true siempre y reloj_ajustar() rechazaba en silencio, asi que un SET_RTC malformado
  // sobre un equipo QUE YA ESTABA EN HORA se acusaba como puesto: el reloj seguia en
  // hora con la hora VIEJA y nada lo delataba. El llamador -desde el 11/09 la rama
  // CMD:HORA_ESP32 de bluetooth.cpp- escribe en el diario una linea distinta segun lo
  // que esto devuelva, y eso solo vale si esto dice la verdad.
  //
  // Y SE PASAN LOS int SIN CASTEAR, tambien a proposito: el cast a uint8_t iba ANTES de
  // la validacion y convertia un h=256 en un 0 que la guarda aceptaba como medianoche.
  //
  // anio y mes se parsean para consumir el formato ISO y se DESCARTAN aqui: por radio
  // solo viaja el dia del mes (CMD_HORA_D) y reloj_ajustar() no tiene donde ponerlos
  // -el STM32 no lleva calendario (N-172)-.
  //
  // D-26 (3): si entro, la hora pasa a ser DEL ESP32. Se guarda el veredicto en una
  // variable y se devuelve ESA, para que el retorno siga siendo el de la llamada
  // (reloj_02 lo exige) y la fuente solo cambie cuando la hora cambio.
  const bool puesta = reloj_ajustarConAcuse(h, m, s, dia);
  if (puesta) fuenteHora = FH_ESP32;
  return puesta;
}

// D-35: guarda el dia absoluto que trae la siembra del ESP32, para la edad del testigo (aviso de 28 dias).
// La llama la rama CMD:HORA_ESP32 de bluetooth.cpp SOLO si reloj_sembrarDesdeIso() devolvio
// true, con la misma cadena: la base recien sembrada es la de esa hora. Declarada en
// modo_degradado.h (reloj.h del Maestro no puede crecer).
bool reloj_guardarFechaEsp32(const char* str) {
  if (str == nullptr || !isoBienFormado(str) || tBaseMillis == 0) return false;
  int anio = 0, mes = 0, dia = 0;
  if (sscanf(str, "%d-%d-%d", &anio, &mes, &dia) != 3) return false;
  const uint16_t d = diasDesde2000(anio, mes, dia);
  if (d == 0) return false;
  diaAbsBase = d;
  return true;
}

// A-15 (29/09, H4) - LA FECHA DEL ESP32 CUANDO MANDA LA RADIO: solo el dia, no la hora (D-26 (3)
// intacta). Por radio viaja el dia del MES (CMD_HORA_D); la fecha que exige la puerta del
// testigo solo la trae el ESP32, y con la radio mandando desde el arranque el Esclavo no la
// guardaba nunca: su puerta rechazaba SIN_HORA. El dia se ancla a la base de radio por el
// camino corto; si el DS3231 discrepa de esa base en mas de FECHA_TOLERANCIA_S, no se toca.
static const int32_t FECHA_TOLERANCIA_S = 3600;
bool reloj_fecharDesdeEsp32(const char* str) {
  if (str == nullptr || !isoBienFormado(str) || !horaValida || tBaseMillis == 0) return false;
  int anio = 0, mes = 0, dia = 0, h = 0, m = 0, s = 0;
  if (sscanf(str, "%d-%d-%d,%d:%d:%d", &anio, &mes, &dia, &h, &m, &s) != 6) return false;
  const uint16_t d = diasDesde2000(anio, mes, dia);
  if (d == 0 || h > 23 || m > 59 || s > 59) return false;
  const int64_t esp = (int64_t)(d - 1U) * 86400 + h * 3600L + m * 60L + s;
  const int64_t t = (int64_t)segBaseDelDia + (millis() - tBaseMillis) / 1000UL;
  const int64_t k = (esp - t + 43200) / 86400;   // el dia de la base por el camino corto
  const int64_t resto = esp - (k * 86400 + t);
  if (k < 0 || k > 65534 || resto > FECHA_TOLERANCIA_S || resto < -FECHA_TOLERANCIA_S) return false;
  diaAbsBase = (uint16_t)(k + 1);
  return true;
}

