// ===== src/reloj.cpp (ESCLAVO) =====
#include "reloj.h"
#include "protocolo.h"       // D-26 (3): SFTY6_SILENCIO_MS, la definicion de "sin radio"
#include <STM32RTC.h>
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

static STM32RTC &rtc = STM32RTC::getInstance();

// Como sabemos si la hora es de fiar:
// al ajustarla escribimos tambien el ano ANIO_MARCA. Un RTC que nunca se puso en
// hora, o que perdio la pila, arranca con el ano en 0 o 1. Si al encender leemos
// un ano anterior a ANIO_MARCA, la hora NO es de fiar y hay que decirlo, no
// suponerlo: un reloj sin poner en hora es peor que no tener reloj.
static const uint8_t ANIO_MARCA = 26;  // 2026, ano de puesta en servicio

static bool horaValida = false;
static bool rtcOperativo = false;

// D-20: Base de tiempo de software extrapolada por millis()
static uint32_t tBaseMillis = 0;
static uint32_t segBaseDelDia = 0;
static uint8_t diaBase = 1;

// D-26 (3) - DE QUIEN ES LA HORA QUE HAY. Ver el porque en reloj.h, sobre
// reloj_radioManda(). Ordenadas de menos a mas autoridad, y el orden SE USA: la del ESP32
// entra si la que hay es de menos autoridad que la radio, o si la radio calla.
//
// La marcan SOLO los que ponen la hora -reloj_setup()/reloj_actualizar() cuando la da el
// RTC, reloj_ajustar() (la radio) y reloj_sembrarDesdeIso() (el ESP32)-, y SIEMPRE junto
// a horaValida: una fuente que dijera RADIO sobre una hora que no entro seria otra vez el
// retorno que no depende de la llamada (N-160).
enum FuenteHora : uint8_t { FH_NINGUNA = 0, FH_RTC_HW, FH_ESP32, FH_RADIO };
static FuenteHora fuenteHora = FH_NINGUNA;

// La ultima trama valida del Maestro, de cualquier comando. Ver reloj_notarRadio().
static bool radioOida = false;
static uint32_t tUltimaRadio = 0;

// Espera acotada a que arranque el oscilador del cristal Y2.
static const uint32_t ESPERA_LSE_MS = 2000;

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

void reloj_setup() {
  horaValida = false;
  rtcOperativo = false;
  tBaseMillis = 0;
  segBaseDelDia = 0;
  diaBase = 1;
  fuenteHora = FH_NINGUNA;

  if (!arrancarCristal()) return;

  rtc.setClockSource(STM32RTC::LSE_CLOCK);  // cristal Y2 de 32.768 kHz
  rtc.begin(false, STM32RTC::HOUR_24);      // false = NO borrar la hora guardada
  rtcOperativo = true;                      // N-24: a partir de aqui el RTC cuenta

  horaValida = rtc.isConfigured() && (rtc.getYear() >= ANIO_MARCA) &&
               (rtc.getHours() != 0 || rtc.getMinutes() != 0 || rtc.getSeconds() != 0);
  // D-26 (3): una hora que solo trae el RTC es la de MENOS autoridad. Puede ser plausible
  // y estar congelada (Y2 muerto): la del ESP32 la pisa aunque haya radio.
  if (horaValida) fuenteHora = FH_RTC_HW;
}

static const unsigned long REINTENTO_LSE_MS = 30000;
static uint32_t tUltimoReintento = 0;

void reloj_actualizar() {
  if (rtcOperativo) return;

  const uint32_t ahora = HAL_GetTick();
  if (ahora - tUltimoReintento < REINTENTO_LSE_MS) return;
  tUltimoReintento = ahora;

  if (__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY) == RESET) return;

  // N-160 - SI YA HABIA HORA SEMBRADA, SE LA PASAMOS AL CRISTAL QUE ACABA DE ARRANCAR.
  //
  // Identico a la otra punta y por el mismo motivo, que es la unica forma de que las
  // dos cuenten igual (N-49). Sin esto la hora SALTA en silencio: se arranca sin
  // cristal, llega la siembra y se queda SOLO en software, y treinta segundos despues
  // este reintento adopta el LSE; a partir de ahi los getters leen de un RTC QUE NUNCA
  // SE SEMBRO con horaValida todavia en true.
  //
  // Aqui no hay radio que empujar -esta punta no origina-, pero de reloj_segundosDelDia()
  // sale la FASE del Degradado, que es el modo que da verde sin el otro extremo: dos
  // puntas con la hora saltando por separado es el ambar-contra-verde que CMD_HORA_D
  // vino a cerrar.
  //
  // SE LEE ANTES DE MOVER LA BANDERA. N-162 (11/09): desde c51cc85 los getters miran
  // PRIMERO la base de software (tBaseMillis > 0) y solo sin ella el RTC, asi que la hora
  // que se lee ya no salta al adoptar el cristal.
  //
  // ~~Copiarla al RTC sigue haciendo falta: es lo que hace que reloj_contadorSegundos() -el
  // que fecha el respaldo- cuente desde una hora escrita y no desde la que el RTC traiga~~
  // -> REFUTADO Y RETIRADO el 11/09 (D-26), con la medida que lo decide:
  //
  //   1. NADIE LEE ESA HORA ESCRITA. Con base de software los getters no tocan el RTC
  //      (tBaseMillis > 0 manda), y el unico que lo lee es reloj_contadorSegundos(), que es
  //      CNT en crudo. respaldo.cpp lo usa SOLO para RESTAR dos lecturas
  //      (respaldo_horasDesdeSync: "ahora - guardado"), asi que el valor desde el que
  //      empieza a contar no interviene: interviene que cuente.
  //   2. Y ESCRIBIRLO PODIA MENTIR. rtc.setHours/Minutes/Seconds reescriben CNT con los
  //      segundos DEL DIA: una marca de sync guardada en la pila en un arranque anterior
  //      queda comparada contra un contador reescrito a mano, y si cae por debajo de lo
  //      que habria contado y por encima de la marca, la REJUVENECE -el mismo motivo 3
  //      por el que se retiro la escritura de la siembra, abajo-.
  //   3. Y BLOQUEABA: con un cristal que da LSERDY y no cuenta -el de la cinta del Sisga-
  //      cada setX() espera RTOFF hasta 1 s (RTC_TIMEOUT_VALUE), ~3 s con el perro en 4.
  //
  // Lo que queda es adoptar el cristal y nada mas: la hora sigue siendo la sembrada, y el
  // contador cuenta desde lo que el RTC tuviera. reloj_dia() se queda sin lector en esta
  // punta por esto -lo apunta costura_10 con este motivo-.
  const bool teniaBase = horaValida;

  rtc.setClockSource(STM32RTC::LSE_CLOCK);
  rtc.begin(false, STM32RTC::HOUR_24);
  rtcOperativo = true;

  if (teniaBase) return;  // la hora es la MISMA de antes: no salta y no se escribe

  if (rtc.isConfigured() && (rtc.getYear() >= ANIO_MARCA) &&
      (rtc.getHours() != 0 || rtc.getMinutes() != 0 || rtc.getSeconds() != 0)) {
    horaValida = true;
    fuenteHora = FH_RTC_HW;
  }
}

bool reloj_enHora() { return horaValida; }

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
  if (rtcOperativo) {
    return (uint32_t)rtc.getHours() * 3600UL + (uint32_t)rtc.getMinutes() * 60UL +
           (uint32_t)rtc.getSeconds();
  }
  return 0;
}

uint8_t reloj_hora() {
  if (!horaValida) return 0;
  if (tBaseMillis > 0) return (uint8_t)(reloj_segundosDelDia() / 3600UL);
  if (rtcOperativo) return rtc.getHours();
  return 0;
}

uint8_t reloj_minuto() {
  if (!horaValida) return 0;
  if (tBaseMillis > 0) return (uint8_t)((reloj_segundosDelDia() % 3600UL) / 60UL);
  if (rtcOperativo) return rtc.getMinutes();
  return 0;
}

uint8_t reloj_segundo() {
  if (!horaValida) return 0;
  if (tBaseMillis > 0) return (uint8_t)(reloj_segundosDelDia() % 60UL);
  if (rtcOperativo) return rtc.getSeconds();
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
  if (rtcOperativo) return rtc.getDay();
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

  // D-20: Siembra de la base de software (independiente de si Y2 oscila)
  segBaseDelDia = (uint32_t)hora * 3600UL + (uint32_t)minuto * 60UL + (uint32_t)segundo;
  tBaseMillis = millis();
  if (dia >= 1) {
    diaBase = (uint8_t)dia;
  } else if (diaBase < 1 || diaBase > 31) {
    diaBase = 1;
  }
  horaValida = true;

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
  // -el calendario del STM32 es enero fijo por construccion-.
  //
  // D-26 (3): si entro, la hora pasa a ser DEL ESP32. Se guarda el veredicto en una
  // variable y se devuelve ESA, para que el retorno siga siendo el de la llamada
  // (reloj_02 lo exige) y la fuente solo cambie cuando la hora cambio.
  const bool puesta = reloj_ajustarConAcuse(h, m, s, dia);
  if (puesta) fuenteHora = FH_ESP32;
  return puesta;
}

