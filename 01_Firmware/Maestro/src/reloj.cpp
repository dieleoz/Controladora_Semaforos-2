// ===== src/reloj.cpp =====
#include "reloj.h"
#include <STM32RTC.h>
#include <stm32f1xx_hal.h>   // N-17: arranque acotado del cristal, ver abajo

// ---------------------------------------------------------------------------
// SFTY-18 — Reloj de tiempo real sobre el RTC interno del STM32.
//
// D-20: LA AUTORIDAD DE LA HORA ES EL ESP32 (DS3231).
// El STM32 no tiene cristal de 32.768 kHz garantizado (Y2 muerto, N-17).
// Por tanto, reloj_ajustar() siembra una base de software extrapolada con millis(),
// asegurando que reloj_enHora() sea true y la hora avance una vez sembrada desde el
// ESP32. Desde el 11/09 (N-162) la siembra YA NO escribe el RTC hardware: ver el porque
// al final de reloj_ajustarConAcuse().
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

// Franja nocturna. Todavia NO se usa para nada: la operacion intermitente por
// horario quedo aplazada a peticion del cliente (31/07/2026), porque el horario
// no es el mismo en todas las obras. Se deja el almacenamiento listo.
static uint8_t nocheInicio = 22;
static uint8_t nocheFin = 5;

// Espera acotada a que arranque el oscilador del cristal Y2.
static const uint32_t ESPERA_LSE_MS = 2000;

static bool arrancarCristal() {
  // El oscilador vive en el dominio de respaldo y hay que poder escribirlo.
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_RCC_BKP_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess();

  // Si ya estaba en marcha -arranque en caliente con la pila puesta- no se toca:
  // reiniciar el oscilador perderia la hora que la pila venia manteniendo.
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

  if (!arrancarCristal()) return;

  rtc.setClockSource(STM32RTC::LSE_CLOCK);  // cristal Y2 de 32.768 kHz
  rtc.begin(false, STM32RTC::HOUR_24);      // false = NO borrar la hora guardada
  rtcOperativo = true;                      // N-24: a partir de aqui el RTC cuenta

  horaValida = rtc.isConfigured() && (rtc.getYear() >= ANIO_MARCA) &&
               (rtc.getHours() != 0 || rtc.getMinutes() != 0 || rtc.getSeconds() != 0);
}

static const unsigned long REINTENTO_LSE_MS = 30000;
static uint32_t tUltimoReintento = 0;

void reloj_actualizar() {
  if (rtcOperativo) return;  // ya esta, nada que hacer

  const uint32_t ahora = HAL_GetTick();
  if (ahora - tUltimoReintento < REINTENTO_LSE_MS) return;
  tUltimoReintento = ahora;

  if (__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY) == RESET) return;  // sigue sin arrancar

  // N-160 - SI YA HABIA HORA SEMBRADA, SE LA PASAMOS AL CRISTAL QUE ACABA DE ARRANCAR.
  //
  // Sin esto la hora SALTA en silencio y nadie se entera. El caso es real y lo abre
  // D-20: arranca sin cristal -rtcOperativo false-, llega la siembra -desde el 11/09 la
  // del ESP32 de este poste, rama CMD:HORA_ESP32 de bluetooth.cpp- y la
  // base se queda SOLO en software -desde el 11/09 la siembra no escribe el RTC nunca-,
  // y treinta segundos despues este reintento adopta el LSE. A partir de esa
  // linea reloj_hora/minuto/segundo/dia() y reloj_segundosDelDia() cambian de fuente
  // al RTC hardware, QUE NUNCA SE SEMBRO, mientras horaValida sigue en true.
  //
  // En esta punta eso no se queda en la pantalla: enviarHoraCompleta() empuja esa hora
  // al Esclavo por radio, y la fase del Degradado sale de reloj_segundosDelDia(). Es
  // N-24 del reves: no "hora escrita sobre un contador parado", sino "contador
  // arrancado bajo una hora que nunca se le escribio".
  //
  // SE LEE ANTES DE MOVER LA BANDERA. N-162 (11/09): desde c51cc85 los getters miran
  // PRIMERO la base de software (tBaseMillis > 0) y solo sin ella el RTC, asi que la hora
  // que se lee ya no salta al adoptar el cristal.
  //
  // ~~Copiarla al RTC sigue haciendo falta: es lo que hace que reloj_contadorSegundos() -el
  // que fecha el respaldo- cuente desde una hora escrita y no desde la que el RTC traiga~~
  // -> REFUTADO Y RETIRADO el 11/09 (D-26), identico en las dos puntas y con la misma
  // medida: (1) con base de software NADIE lee la hora del RTC -los getters miran
  // tBaseMillis, reloj_fijarEnero() tambien se aparta- y el unico que lo lee,
  // reloj_contadorSegundos(), es CNT en crudo que respaldo.cpp solo RESTA
  // (respaldo_horasDesdeSync: "ahora - guardado"), asi que el valor desde el que empieza no
  // interviene; (2) reescribir CNT con los segundos del dia puede REJUVENECER una marca de
  // sync de un arranque anterior -el motivo 3 de la siembra, abajo-; (3) con un cristal que
  // da LSERDY y no cuenta, cada setX() espera RTOFF hasta 1 s: ~3 s con el perro en 4 s.
  // Queda adoptar el cristal y nada mas.
  const bool teniaBase = horaValida;

  rtc.setClockSource(STM32RTC::LSE_CLOCK);
  rtc.begin(false, STM32RTC::HOUR_24);
  rtcOperativo = true;

  if (teniaBase) return;  // horaValida ya estaba en true y la hora es la MISMA: no salta

  // Sin base previa si vale adoptar lo que el RTC traiga: es un arranque en caliente
  // con la hora que sobrevivio en el dominio de respaldo.
  if (rtc.isConfigured() && (rtc.getYear() >= ANIO_MARCA) &&
      (rtc.getHours() != 0 || rtc.getMinutes() != 0 || rtc.getSeconds() != 0)) {
    horaValida = true;
  }
}

bool reloj_reiniciarDominioRespaldo() {
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_RCC_BKP_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess();

  __HAL_RCC_BACKUPRESET_FORCE();
  __HAL_RCC_BACKUPRESET_RELEASE();

  horaValida = false;
  rtcOperativo = false;
  tBaseMillis = 0;
  segBaseDelDia = 0;
  diaBase = 1;

  if (!arrancarCristal()) return false;

  rtc.setClockSource(STM32RTC::LSE_CLOCK);
  rtc.begin(false, STM32RTC::HOUR_24);
  rtcOperativo = true;
  return true;
}

bool reloj_enHora() { return horaValida; }

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

void reloj_fijarEnero() {
  if (!horaValida) return;
  // N-162 (11/09): CON BASE SEMBRADA NO SE TOCA EL RTC. Esto corre cada 10 min desde
  // coordinador.cpp, y rtc.getMonth() no es una lectura inocente en el F1: pasa por
  // HAL_RTC_GetTime(), que con el contador por encima de 24 h lo REESCRIBE -plegandolo al
  // dia- con la misma espera de RTOFF de 1000 ms que la siembra quito arriba. Y con base
  // sembrada el calendario del RTC no lo lee nadie: reloj_dia() cuenta desde diaBase, que
  // ya vuelve de 31 a 1 como enero. Solo queda util cuando la hora vino del propio RTC.
  if (tBaseMillis > 0) return;
  if (rtcOperativo && rtc.getMonth() != 1) rtc.setMonth(1);
}

bool reloj_hayCristal() { return rtcOperativo; }

// N-49 — el contador crudo del RTC, en segundos. Ver la nota de reloj.h.
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
  // reasigna en CADA siembra, o sea que el contador BAJA cada vez que llega la hora.
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

// N-45 — la consulta. Solo lee; no configura, no arranca y no borra nada.
void reloj_diagnostico(RelojDiag* d) {
  if (d == nullptr) return;

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_RCC_BKP_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess();

  const uint32_t bdcr = RCC->BDCR;
  d->lseOn  = (bdcr & RCC_BDCR_LSEON) != 0;
  d->lseRdy = (bdcr & RCC_BDCR_LSERDY) != 0;
  d->lseByp = (bdcr & RCC_BDCR_LSEBYP) != 0;
  d->rtcSel = (uint8_t)((bdcr & RCC_BDCR_RTCSEL) >> RCC_BDCR_RTCSEL_Pos);
  d->rtcEn  = (bdcr & RCC_BDCR_RTCEN) != 0;

  d->cntLeido = false;
  d->cnt = 0;
  if (d->rtcEn) {
    uint16_t alta = (uint16_t)RTC->CNTH;
    const uint16_t baja = (uint16_t)RTC->CNTL;
    if ((uint16_t)RTC->CNTH != alta) alta = (uint16_t)RTC->CNTH;
    d->cnt = ((uint32_t)alta << 16) | baja;
    d->cntLeido = true;
  }

  d->configurado = rtcOperativo ? rtc.isConfigured() : false;
  d->anio = rtcOperativo ? (uint16_t)rtc.getYear() : 0;
}

// D-20 / N-160: aqui vive la regla de rango, y en ningun otro sitio. Ver reloj.h.
bool reloj_ajustarConAcuse(int hora, int minuto, int segundo, int dia) {
  // Los limites se miran sobre el int, ANTES de castear: un 256 casteado a uint8_t
  // entra como 0 y pasaria por medianoche. Y el negativo hay que mirarlo porque
  // sscanf("%d") acepta "-5" sin protestar.
  if (hora < 0 || hora > 23) return false;      // no aceptamos basura
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
// reloj.h) y para que los llamadores que no miran el retorno -la pantalla AJUSTAR HORA
// y la rama CMD_HORA_S del Esclavo- sigan compilando sin tocarlos. No repite la guarda:
// la unica copia de la regla de rango esta en reloj_ajustarConAcuse().
void reloj_ajustar(uint8_t hora, uint8_t minuto, uint8_t segundo, uint8_t dia) {
  (void)reloj_ajustarConAcuse((int)hora, (int)minuto, (int)segundo, (int)dia);
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
  // sobre un equipo QUE YA ESTABA EN HORA se acusaba como puesto: la barrera de abajo
  // -coordinador_sincronizarHora() se niega si !reloj_enHora()- no lo veia, porque el
  // reloj seguia en hora con la hora VIEJA. El tecnico se iba del poste con el $ACK.
  //
  // Y SE PASAN LOS int SIN CASTEAR, tambien a proposito: el cast a uint8_t iba ANTES de
  // la validacion y convertia un h=256 en un 0 que la guarda aceptaba como medianoche.
  //
  // anio y mes se parsean para consumir el formato ISO y se DESCARTAN aqui: por radio
  // solo viaja el dia del mes (CMD_HORA_D) y reloj_ajustar() no tiene donde ponerlos
  // -el calendario del STM32 es enero fijo por construccion, ver reloj_fijarEnero()-.
  return reloj_ajustarConAcuse(h, m, s, dia);
}

void reloj_ajustarFranjaNocturna(uint8_t horaInicio, uint8_t horaFin) {
  if (horaInicio > 23 || horaFin > 23) return;
  nocheInicio = horaInicio;
  nocheFin = horaFin;
}

uint8_t reloj_inicioNoche() { return nocheInicio; }
uint8_t reloj_finNoche() { return nocheFin; }

bool reloj_esHorarioNocturno() {
  if (!horaValida) return false;  // sin hora fiable, nunca
  uint8_t h = reloj_hora();
  if (nocheInicio == nocheFin) return false;
  if (nocheInicio < nocheFin) return (h >= nocheInicio && h < nocheFin);
  return (h >= nocheInicio || h < nocheFin);  // franja que cruza medianoche
}

const char *reloj_textoHora() {
  static char buf[6];
  if (!horaValida) return "--:--";
  snprintf(buf, sizeof(buf), "%02u:%02u", (unsigned)reloj_hora(),
           (unsigned)reloj_minuto());
  return buf;
}

