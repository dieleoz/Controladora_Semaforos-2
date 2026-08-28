// ===== src/reloj.cpp (ESCLAVO) =====
#include "reloj.h"
#include <STM32RTC.h>
#include <stm32f1xx_hal.h>   // N-17: arranque acotado del cristal, ver abajo

// ---------------------------------------------------------------------------
// SFTY-18 / SFTY-23 — Reloj de tiempo real sobre el RTC interno del STM32.
//
// El RTC vive en el "dominio de respaldo" del microcontrolador: se alimenta de
// VBAT y sigue contando con la tarjeta apagada, siempre que la pila este puesta
// y R5 retirado (ver 03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md seccion 4).
//
// Que sobreviva al apagado es justo lo que hace util la sincronizacion por radio:
// si la hora se perdiera en cada corte de energia, habria que repetir el proceso
// cada manana y nadie lo haria.
//
// Portado del Maestro SIN la franja nocturna ni el formateo para LCD; el motivo
// esta razonado en reloj.h.
// ---------------------------------------------------------------------------

static STM32RTC &rtc = STM32RTC::getInstance();

// Como sabemos si la hora es de fiar:
// al ajustarla escribimos tambien el ano ANIO_MARCA. Un RTC que nunca se puso en
// hora, o que perdio la pila, arranca con el ano en 0 o 1. Si al encender leemos
// un ano anterior a ANIO_MARCA, la hora NO es de fiar y hay que decirlo, no
// suponerlo: un reloj sin poner en hora es peor que no tener reloj.
//
// DEBE valer lo mismo que en el Maestro. Si una punta usara otra marca, un mismo
// RTC seria "valido" para una e "invalido" para la otra.
static const uint8_t ANIO_MARCA = 26;  // 2026, ano de puesta en servicio

static bool horaValida = false;

// N-24 — si el cristal no arranco, el RTC no cuenta. Mismo razonamiento que en el
// Maestro y misma consecuencia: escribir la hora sobre un contador parado la deja
// visible pero sin avanzar, y horaValida en true seria una mentira sobre la que el
// Modo Degradado se autorizaria. Aqui llega por RADIO, asi que el Esclavo aceptaria
// sin rechistar una hora que no puede mantener.
static bool rtcOperativo = false;

// Espera acotada a que arranque el oscilador del cristal Y2.
//
// N-17, CONFIRMADO EN BANCO EL 01/08/2026. rtc.begin() con LSE espera al oscilador
// SIN LIMITE. Si el cristal no arranca -y MAPEO_TARJETA_KICAD.md seccion 4 ya
// advertia que en microcontroladores clonados el condensador de carga viene mal
// calculado y no oscila-, el arranque se queda ahi para siempre.
//
// En ESTA tarjeta el sintoma era la PANTALLA EN BLANCO con las luces funcionando:
// semaforo_setup() ya habia corrido, pero lcd_setup() viene despues de aqui y no
// llegaba a ejecutarse nunca. En el Maestro, con la pantalla arrancada antes, el
// mismo cuelgue se veia como un bucle en la pantalla de bienvenida. Mismo fallo,
// dos sintomas, y ninguno de los dos parecia un problema de reloj.
//
// UN SEMAFORO NO PUEDE DEPENDER DE UN CRISTAL DE RELOJ PARA ENCENDER: si el reloj no
// esta, se arranca sin reloj y quien dependa de la hora se abstiene, que es justo lo
// que reloj_enHora() existe para decir. Aqui pesa aun mas que en el Maestro: sin
// pantalla, un Esclavo colgado no tiene forma de contarle a nadie lo que le pasa.
//
// El limite es de 2 s, por debajo de los 4 s del watchdog ya armado.
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
      // N-25: se deja ENCENDIDO intentandolo. Apagarlo convertia "lento" en "muerto"
      // para siempre; dejarlo cuesta microamperios y permite adoptarlo despues.
      return false;
    }
  }
  return true;
}

void reloj_setup() {
  horaValida = false;
  rtcOperativo = false;

  // Sin cristal no se llama a rtc.begin(): esa llamada es la que se colgaba.
  if (!arrancarCristal()) return;

  rtc.setClockSource(STM32RTC::LSE_CLOCK);  // cristal Y2 de 32.768 kHz
  rtc.begin(false, STM32RTC::HOUR_24);      // false = NO borrar la hora guardada
  rtcOperativo = true;                      // N-24: a partir de aqui el RTC cuenta

  // Solo damos el reloj por bueno si ademas de estar configurado lleva nuestra
  // marca de ano. El flag de configuracion por si solo puede quedar puesto por
  // basura en el dominio de respaldo tras un arranque sucio.
  horaValida = rtc.isConfigured() && (rtc.getYear() >= ANIO_MARCA);
}

// N-25 — reintento en segundo plano del cristal. Identico al del Maestro y por el
// mismo motivo: el arranque no puede esperar al oscilador, pero tampoco condenarlo
// tras 2 s. Aqui importa mas todavia, porque esta punta no tiene a nadie mirando su
// pantalla: si el cristal despierta tarde, nadie estaria delante para reiniciarla.
static const unsigned long REINTENTO_LSE_MS = 30000;
static uint32_t tUltimoReintento = 0;

void reloj_actualizar() {
  if (rtcOperativo) return;

  const uint32_t ahora = HAL_GetTick();
  if (ahora - tUltimoReintento < REINTENTO_LSE_MS) return;
  tUltimoReintento = ahora;

  if (__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY) == RESET) return;

  rtc.setClockSource(STM32RTC::LSE_CLOCK);
  rtc.begin(false, STM32RTC::HOUR_24);
  rtcOperativo = true;
  horaValida = rtc.isConfigured() && (rtc.getYear() >= ANIO_MARCA);
}

bool reloj_enHora() { return horaValida; }

// N-49 — el contador crudo del RTC, en segundos. Copia literal del Maestro: las dos
// puntas tienen que fechar la sincronizacion con la MISMA aritmetica, y toda la
// grieta que N-49 cierra venia justo de que cada una medía a su manera.
uint32_t reloj_contadorSegundos() {
  if (!rtcOperativo) return 0;

  // Dos registros de 16 bits. Se relee CNTH si cambio entre las dos lecturas: sin
  // eso, un acarreo justo en medio da un valor adelantado en 65.536 segundos, y esa
  // cifra alimenta el limite de 48 h del Modo Degradado.
  uint16_t alta = (uint16_t)RTC->CNTH;
  const uint16_t baja = (uint16_t)RTC->CNTL;
  if ((uint16_t)RTC->CNTH != alta) alta = (uint16_t)RTC->CNTH;

  const uint32_t v = ((uint32_t)alta << 16) | baja;

  // El cero esta reservado para "no hay reloj". Ver la nota del Maestro.
  return v == 0 ? 1UL : v;
}

uint8_t reloj_hora() { return horaValida ? rtc.getHours() : 0; }
uint8_t reloj_minuto() { return horaValida ? rtc.getMinutes() : 0; }
uint8_t reloj_segundo() { return horaValida ? rtc.getSeconds() : 0; }
uint8_t reloj_dia() { return horaValida ? rtc.getDay() : 0; }

uint32_t reloj_segundosDelDia() {
  if (!horaValida) return 0;
  return (uint32_t)rtc.getHours() * 3600UL + (uint32_t)rtc.getMinutes() * 60UL +
         (uint32_t)rtc.getSeconds();
}

void reloj_ajustar(uint8_t hora, uint8_t minuto, uint8_t segundo, uint8_t dia) {
  // No aceptamos basura. En el Maestro el filtro era casi teorico porque la hora
  // venia de un menu que ya acota cada digito; aqui viene de la RADIO, donde una
  // trama corrupta que pase el CRC por casualidad si puede traer 0xFF. Es la
  // ultima barrera antes de escribir en el RTC.
  //
  // El dia entra en la misma guarda desde que viaja por radio (CMD_HORA_D): 0 es el
  // caso legitimo "no toques la fecha", 1..31 es un dia, y cualquier otra cosa es
  // basura. Se descarta la llamada COMPLETA y no solo el dia, porque un valor
  // imposible delata una trama corrupta y con una trama corrupta en el envio no hay
  // motivo para fiarse de las otras tres.
  // N-24: sin oscilador no se acepta el ajuste, ni siquiera viniendo del Maestro.
  // Aceptarlo dejaria a esta punta anunciando una hora que no avanza, y el cruce se
  // sostiene en que las DOS cuentan igual. Rechazar aqui hace que no se envie el
  // ACK_HORA, con lo que el Maestro se entera de que esta punta no tiene reloj en vez
  // de darla por sincronizada.
  if (!rtcOperativo) return;

  if (hora > 23 || minuto > 59 || segundo > 59 || dia > 31) return;
  rtc.setHours(hora);
  rtc.setMinutes(minuto);
  rtc.setSeconds(segundo);
  // La marca de ano es lo que hara que el proximo arranque acepte esta hora.
  if (rtc.getYear() < ANIO_MARCA) rtc.setYear(ANIO_MARCA);

  // LA FECHA.
  //
  // El respaldo (N-20) necesita el DIA DEL MES para saber cuanto hace de la ultima
  // sincronizacion a traves de un reinicio: con solo los segundos del dia no se
  // distingue "hace tres horas" de "hace veintisiete". Si la fecha quedara sin
  // inicializar, reloj_dia() podria devolver un valor que el respaldo rechaza, con
  // lo que la antiguedad saldria siempre CADUCADA y el Modo Degradado no podria
  // reanudar NUNCA tras un corte.
  //
  // AHORA EL DIA VIENE DEL MAESTRO. Antes cada punta sembraba su propio dia 1 la
  // primera vez que se ponia en hora: dos calendarios que arrancaban en dias
  // distintos y no se volvian a encontrar jamas. Y como la antiguedad del respaldo
  // se calcula restando dias, un corte de energia en la vuelta de 31 a 1 del
  // calendario de UNA punta la dejaba en AMBAR sin poder reanudar mientras la otra
  // reanudaba y daba VERDE. Con el mismo numero de dia en las dos, o reanudan las
  // dos o falla ninguna; el modo de fallo pasa a ser simetrico, que es lo unico que
  // este cruce puede permitirse.
  //
  // Sigue sin importar QUE dia sea -nadie lo muestra ni lo interpreta como fecha-,
  // importa que las dos puntas cuenten con el MISMO numero y que el RTC avance a
  // partir de ahi.
  if (dia >= 1) {
    rtc.setDay(dia);
    // El mes tambien tiene que ser valido o el RTC podria rechazar dias altos; no se
    // transporta por radio porque las cuentas del respaldo son de dias, nunca de
    // meses. Se fija uno cualquiera, igual en las dos puntas.
    rtc.setMonth(1);
  } else if (rtc.getDay() < 1 || rtc.getDay() > 31) {
    // Camino de reserva: dia == 0 significa "no toques la fecha", pero si lo que hay
    // en el RTC no es un dia valido hay que sembrar algo o el respaldo no podria
    // restar dias nunca. Este caso ya no lo alcanza la radio -main.cpp exige el dia
    // para aplicar el envio-, y queda por si el RTC arranca sucio.
    rtc.setDay(1);
    rtc.setMonth(1);
  }
  horaValida = true;
}
