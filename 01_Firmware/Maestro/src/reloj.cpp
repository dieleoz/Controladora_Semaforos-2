// ===== src/reloj.cpp =====
#include "reloj.h"
#include <STM32RTC.h>
#include <stm32f1xx_hal.h>   // N-17: arranque acotado del cristal, ver abajo

// ---------------------------------------------------------------------------
// SFTY-18 — Reloj de tiempo real sobre el RTC interno del STM32.
//
// El RTC vive en el "dominio de respaldo" del microcontrolador: se alimenta de
// VBAT y sigue contando con la tarjeta apagada, siempre que la pila este puesta
// y R5 retirado (ver 03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md seccion 4).
// ---------------------------------------------------------------------------

static STM32RTC &rtc = STM32RTC::getInstance();

// Como sabemos si la hora es de fiar:
// al ajustarla escribimos tambien el ano ANIO_MARCA. Un RTC que nunca se puso en
// hora, o que perdio la pila, arranca con el ano en 0 o 1. Si al encender leemos
// un ano anterior a ANIO_MARCA, la hora NO es de fiar y hay que decirlo, no
// suponerlo: un reloj sin poner en hora es peor que no tener reloj.
static const uint8_t ANIO_MARCA = 26;  // 2026, ano de puesta en servicio

static bool horaValida = false;

// N-24 — SI EL CRISTAL NO ARRANCO, EL RTC NO EXISTE.
//
// Distinto de horaValida, y la diferencia importa: horaValida dice "no se cuanta hora
// es", esto dice "no tengo con que contarla". Un reloj sin poner en hora se arregla
// poniendolo; un RTC sin oscilador no se arregla desde el menu, y hay que decirlo.
//
// CONFIRMADO EN BANCO EL 01/08/2026: se ajustaba la hora, la pantalla la daba por
// buena, y al apagar y encender volvia a ceros. El motivo es que reloj_ajustar()
// escribia en el RTC y ponia horaValida = true SIN COMPROBAR que rtc.begin() se
// hubiera llegado a ejecutar. Con el cristal muerto -que es el caso de N-17- se
// escribe sobre un contador que nadie hace avanzar: la hora se ve en pantalla, no
// avanza, y desaparece en el siguiente arranque.
//
// Y no se quedaba en lo cosmetico. El Maestro habria EMPUJADO esa hora al Esclavo y
// autorizado el Modo Degradado sobre un reloj parado, que es justo la situacion que
// reloj_enHora() existe para impedir: las dos puntas calculan su fase por reloj sin
// hablarse, y un reloj que no avanza da verde cuando no toca.
static bool rtcOperativo = false;

// Franja nocturna. Todavia NO se usa para nada: la operacion intermitente por
// horario quedo aplazada a peticion del cliente (31/07/2026), porque el horario
// no es el mismo en todas las obras. Se deja el almacenamiento listo.
static uint8_t nocheInicio = 22;
static uint8_t nocheFin = 5;

// Espera acotada a que arranque el oscilador del cristal Y2.
//
// N-17, CONFIRMADO EN BANCO EL 01/08/2026. rtc.begin() con LSE espera al oscilador
// SIN LIMITE. Si el cristal no arranca -y MAPEO_TARJETA_KICAD.md seccion 4 ya
// advertia que en microcontroladores clonados el condensador de carga viene mal
// calculado y no oscila-, el arranque se queda ahi para siempre.
//
// Lo que se vio en las dos tarjetas:
//
//   MAESTRO: la bienvenida ya estaba pintada, asi que quedaba EN BUCLE en ella. El
//            watchdog reiniciaba a los 4 s y volvia a colgarse. Sin luces, sin
//            responder a los botones.
//   ESCLAVO: lcd_setup() va DESPUES, asi que la pantalla no llegaba a arrancar
//            -en blanco-, pero semaforo_setup() si habia corrido y las luces si
//            funcionaban. Mismo cuelgue, sintoma distinto.
//
// El watchdog convirtio un cuelgue mudo en un reinicio visible, que era su papel,
// pero el equipo seguia sin arrancar. UN SEMAFORO NO PUEDE DEPENDER DE UN CRISTAL DE
// RELOJ PARA ENCENDER: si el reloj no esta, se arranca sin reloj y quien dependa de
// la hora se abstiene, que es justo lo que reloj_enHora() existe para decir.
//
// El limite es de 2 s, por debajo de los 4 s del watchdog ya armado: si el cristal
// no ha arrancado en ese tiempo, no va a arrancar.
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
      // N-25: SE DEJA ENCENDIDO INTENTANDOLO. Antes se apagaba, con el argumento de
      // que un oscilador que no arranca solo consume. Pero eso convertia "lento" en
      // "muerto" para siempre: un cristal marginal, o con frio, puede tardar mas de
      // estos 2 s, y apagarlo garantizaba que no arrancara nunca. Dejarlo activo
      // cuesta microamperios y permite que reloj_actualizar() lo adopte despues.
      return false;
    }
  }
  return true;
}

void reloj_setup() {
  horaValida = false;
  rtcOperativo = false;

  // Sin cristal no se llama a rtc.begin(): esa llamada es la que se colgaba. El
  // equipo arranca igual y sigue su vida sin reloj.
  if (!arrancarCristal()) return;

  rtc.setClockSource(STM32RTC::LSE_CLOCK);  // cristal Y2 de 32.768 kHz
  rtc.begin(false, STM32RTC::HOUR_24);      // false = NO borrar la hora guardada
  rtcOperativo = true;                      // N-24: a partir de aqui el RTC cuenta

  // Solo damos el reloj por bueno si ademas de estar configurado lleva nuestra
  // marca de ano. El flag de configuracion por si solo puede quedar puesto por
  // basura en el dominio de respaldo tras un arranque sucio.
  horaValida = rtc.isConfigured() && (rtc.getYear() >= ANIO_MARCA);
}

// N-25 — REINTENTO EN SEGUNDO PLANO DEL CRISTAL.
//
// El arranque no puede esperar al oscilador -esa es toda la leccion de N-17-, pero
// tampoco hay motivo para condenarlo tras 2 s. Un cristal marginal, o simplemente
// frio, puede tardar mas; con el arranque acotado Y el oscilador apagado al agotarse,
// "lento" quedaba indistinguible de "muerto" hasta el siguiente reinicio.
//
// Ahora el equipo arranca sin reloj -sigue sin depender de el para encender- y lo
// ADOPTA en cuanto el cristal despierte, sin reiniciar y sin que nadie intervenga. Si
// esta muerto de verdad, esto no hace nada: solo lee un flag cada 30 s.
//
// Se comprueba de tarde en tarde a proposito. Leer LSERDY es barato, pero esto corre
// en el bucle de un semaforo y no hay ninguna prisa: nadie nota 30 s de diferencia en
// adoptar un reloj, y sale gratis.
static const unsigned long REINTENTO_LSE_MS = 30000;
static uint32_t tUltimoReintento = 0;

void reloj_actualizar() {
  if (rtcOperativo) return;  // ya esta, nada que hacer

  const uint32_t ahora = HAL_GetTick();
  if (ahora - tUltimoReintento < REINTENTO_LSE_MS) return;
  tUltimoReintento = ahora;

  if (__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY) == RESET) return;  // sigue sin arrancar

  // Arranco tarde. Se completa la inicializacion que reloj_setup() no pudo hacer.
  // rtc.begin() ya no puede colgarse: el oscilador esta listo, que era la condicion
  // que faltaba.
  rtc.setClockSource(STM32RTC::LSE_CLOCK);
  rtc.begin(false, STM32RTC::HOUR_24);
  rtcOperativo = true;

  // La hora solo se da por buena si el dominio de respaldo la traia de antes. Si el
  // cristal acaba de arrancar por primera vez no habra nada, y la pantalla pasara de
  // "SIN CRISTAL" a "RELOJ SIN PONER EN HORA": ahora si se arregla desde el menu.
  horaValida = rtc.isConfigured() && (rtc.getYear() >= ANIO_MARCA);
}

// N-31 — REINICIO DEL DOMINIO DE RESPALDO. Ultima carta antes del soldador.
//
// El dominio de respaldo CONSERVA su configuracion entre arranques: vive de VBAT y no
// se entera de los reinicios del micro. Si un firmware anterior dejo el LSE a medio
// configurar -y en esta tarjeta hubo varios flasheos con el RTC inicializandose a
// medias-, poner LSE_ON sobre unos registros ya sucios puede no arrancar NUNCA por
// muy sano que este el cristal. Nada de lo que haga reloj_setup() lo limpia, porque
// justamente esta pensado para NO tocar lo que la pila mantiene.
//
// BACKUPRESET deja esos registros como de fabrica y permite intentarlo desde cero.
//
// NO SE HACE AUTOMATICAMENTE AL ARRANCAR, y es deliberado: borra la hora y los
// registros de respaldo -el ciclo acordado, la marca de sincronizacion, el indicador
// del Degradado-. Un equipo que se limpiara la memoria solo cada vez que el cristal
// tarda en despertar perderia la reanudacion tras un corte, que es justo lo que N-20
// existe para dar. Lo pide una persona desde el menu, sabiendo lo que cuesta.
//
// Devuelve true si tras el reinicio el oscilador SI arranca. False significa que el
// estado sucio no era la causa: es el cristal, y toca hardware.
bool reloj_reiniciarDominioRespaldo() {
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_RCC_BKP_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess();

  __HAL_RCC_BACKUPRESET_FORCE();
  __HAL_RCC_BACKUPRESET_RELEASE();

  // Todo lo que se creia saber del reloj se acaba de borrar.
  horaValida = false;
  rtcOperativo = false;

  if (!arrancarCristal()) return false;   // sigue sin arrancar: es Y2

  rtc.setClockSource(STM32RTC::LSE_CLOCK);
  rtc.begin(false, STM32RTC::HOUR_24);
  rtcOperativo = true;
  // horaValida sigue en false a proposito: el dominio se acaba de borrar, asi que no
  // hay hora guardada que rescatar. Hay que volver a ponerla.
  return true;
}

bool reloj_enHora() { return horaValida; }

// N-144: la contrapartida que faltaba. Ver reloj.h para el porque.
//
// No toca el RTC: no hay nada que borrar alli -si el ajuste no quedo es porque el
// contador no avanza-, y escribirle mas seria insistir sobre lo mismo. Lo que se
// corrige es la CREENCIA del firmware, que es lo que estaba mal.
void reloj_invalidarHora() { horaValida = false; }

uint8_t reloj_hora() { return horaValida ? rtc.getHours() : 0; }
uint8_t reloj_minuto() { return horaValida ? rtc.getMinutes() : 0; }
uint8_t reloj_segundo() { return horaValida ? rtc.getSeconds() : 0; }
uint8_t reloj_dia() { return horaValida ? rtc.getDay() : 0; }

void reloj_fijarEnero() {
  // Sin hora fiable no hay calendario que anclar, y escribir el mes marcaria el RTC
  // como tocado sin que nadie lo haya puesto en hora.
  if (!horaValida) return;
  if (rtc.getMonth() != 1) rtc.setMonth(1);
}

uint32_t reloj_segundosDelDia() {
  if (!horaValida) return 0;
  return (uint32_t)rtc.getHours() * 3600UL + (uint32_t)rtc.getMinutes() * 60UL +
         (uint32_t)rtc.getSeconds();
}

bool reloj_hayCristal() { return rtcOperativo; }

// N-49 — el contador crudo del RTC, en segundos. Ver la nota de reloj.h.
uint32_t reloj_contadorSegundos() {
  if (!rtcOperativo) return 0;

  // Dos registros de 16 bits. Se relee CNTH si cambio entre las dos lecturas: sin
  // eso, un acarreo justo en medio da un valor adelantado en 65.536 segundos, y esa
  // cifra alimenta el limite de 48 h del Modo Degradado.
  uint16_t alta = (uint16_t)RTC->CNTH;
  const uint16_t baja = (uint16_t)RTC->CNTL;
  if ((uint16_t)RTC->CNTH != alta) alta = (uint16_t)RTC->CNTH;

  const uint32_t v = ((uint32_t)alta << 16) | baja;

  // El cero esta reservado para "no hay reloj". Un contador legitimamente en cero
  // -el primer segundo tras un reinicio del dominio de respaldo- se declara 1: un
  // segundo de error no significa nada frente a un limite de 48 h, y a cambio el
  // valor centinela no se confunde nunca con una medida real.
  return v == 0 ? 1UL : v;
}

// N-45 — la consulta. Solo lee; no configura, no arranca y no borra nada.
void reloj_diagnostico(RelojDiag* d) {
  if (d == nullptr) return;

  // Leer BDCR necesita acceso al dominio de respaldo, igual que arrancarCristal().
  // Habilitar estos relojes es idempotente y no altera la configuracion del RTC.
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_RCC_BKP_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess();

  const uint32_t bdcr = RCC->BDCR;
  d->lseOn  = (bdcr & RCC_BDCR_LSEON) != 0;
  d->lseRdy = (bdcr & RCC_BDCR_LSERDY) != 0;
  d->lseByp = (bdcr & RCC_BDCR_LSEBYP) != 0;
  d->rtcSel = (uint8_t)((bdcr & RCC_BDCR_RTCSEL) >> RCC_BDCR_RTCSEL_Pos);
  d->rtcEn  = (bdcr & RCC_BDCR_RTCEN) != 0;

  // CON RTCEN=0 NO SE TOCA EL PERIFERICO. Leer los registros de un periferico sin
  // reloj es un fallo de bus, y esto corre en un semaforo: una pantalla de consulta
  // que tumba el equipo es peor que no tener pantalla de consulta.
  d->cntLeido = false;
  d->cnt = 0;
  if (d->rtcEn) {
    // El contador del F1 son dos registros de 16 bits. Se relee CNTH si CNTL dio la
    // vuelta entre las dos lecturas; sin eso, un acarreo justo en medio da un valor
    // adelantado en 65.536 segundos y parece que el reloj salta.
    uint16_t alta = (uint16_t)RTC->CNTH;
    const uint16_t baja = (uint16_t)RTC->CNTL;
    if ((uint16_t)RTC->CNTH != alta) alta = (uint16_t)RTC->CNTH;
    d->cnt = ((uint32_t)alta << 16) | baja;
    d->cntLeido = true;
  }

  // isConfigured() y getYear() pasan por la libreria, y esa solo es fiable si se
  // llego a llamar a rtc.begin(). Sin eso devolverian lo que hubiera en memoria.
  d->configurado = rtcOperativo ? rtc.isConfigured() : false;
  d->anio = rtcOperativo ? (uint16_t)rtc.getYear() : 0;
}

void reloj_ajustar(uint8_t hora, uint8_t minuto, uint8_t segundo, uint8_t dia) {
  // N-24: SIN OSCILADOR NO SE ACEPTA EL AJUSTE. Escribir aqui dejaria una hora en un
  // contador que nadie hace avanzar: se veria en pantalla, no avanzaria, y se
  // perderia en el siguiente arranque -que es exactamente el "vuelve a ceros" que se
  // vio en banco-. Peor aun, horaValida quedaria en true y sobre esa mentira el
  // Maestro empujaria la hora al Esclavo y autorizaria el Modo Degradado.
  //
  // Se rechaza en silencio y horaValida SIGUE en false, que es lo que hace que la
  // pantalla y el resto del firmware se abstengan. Quien quiera distinguir "no
  // puesto" de "sin cristal" tiene reloj_hayCristal().
  if (!rtcOperativo) return;

  if (hora > 23 || minuto > 59 || segundo > 59) return;  // no aceptamos basura

  // El dia se valida con el mismo criterio que el resto: fuera de rango se descarta
  // la llamada ENTERA, no solo el campo malo. Una trama corrompida que trajera un
  // dia imposible traera probablemente tambien una hora imposible; aplicar la mitad
  // de un ajuste dudoso es peor que no aplicarlo.
  // El 0 no es basura: es "no me han dicho el dia, no toques la fecha".
  if (dia > 31) return;

  rtc.setHours(hora);
  rtc.setMinutes(minuto);
  rtc.setSeconds(segundo);

  // La marca de ano es lo que hara que el proximo arranque acepte esta hora.
  if (rtc.getYear() < ANIO_MARCA) rtc.setYear(ANIO_MARCA);

  // LA FECHA SE SIEMBRA AQUI SI NO ES VALIDA, aunque el operario nunca la teclee.
  //
  // La pantalla solo pide HH:MM: pedir tambien la fecha serian dos digitos mas a
  // ciegas con cuatro botones, para un dato que no se muestra en ningun sitio. Por
  // eso `dia` vale 0 cuando el ajuste viene del teclado, y esta siembra sigue siendo
  // el unico sitio donde ese camino consigue una fecha con la que poder restar dias.
  //
  // Pero el respaldo (N-20) necesita el DIA DEL MES para saber cuanto hace de la
  // ultima sincronizacion a traves de un reinicio: con solo los segundos del dia no
  // se distingue "hace tres horas" de "hace veintisiete". Y si la fecha quedara sin
  // inicializar, reloj_dia() podria devolver un valor que el respaldo rechaza, con
  // lo que la antiguedad saldria siempre CADUCADA y el Modo Degradado no podria
  // reanudar NUNCA tras un corte.
  //
  // No importa QUE dia sea, solo poder restar dias: se fija uno conocido y el RTC
  // avanza a partir de ahi por su cuenta. Es deliberado que el equipo no sepa la
  // fecha real: no la necesita para nada y no vamos a pedirsela al operario.
  if (rtc.getDay() < 1 || rtc.getDay() > 31) {
    rtc.setDay(1);
    rtc.setMonth(1);
  }

  // Y AQUI SE ACOPLA EL CALENDARIO CON LA OTRA PUNTA.
  //
  // Con dia 1..31 manda el numero que llega de fuera, por encima de lo sembrado
  // arriba: lo que importa no es QUE dia sea, sino que las dos unidades cuenten los
  // dias con el MISMO numero. Sembrando cada una el suyo, los calendarios quedaban
  // desacoplados para siempre y respaldo_horasDesdeSync() podia declarar CADUCADA en
  // una sola punta -la que cruzaba el fin de mes-, que se quedaba en ambar mientras
  // la otra reanudaba y daba verde. Acoplados, o fallan las dos o no falla ninguna.
  //
  // Se fuerza enero junto con el dia porque el propio RTC decide cuando pasa de 31 a
  // 1 segun la longitud del mes: dos puntas en meses distintos volverian a romper la
  // cuenta aunque el dia se les imponga. Enero tiene 31 dias, asi que cualquier valor
  // del rango es legal y el salto de mes cae en el mismo sitio en las dos.
  if (dia >= 1) {
    rtc.setDay(dia);
    rtc.setMonth(1);
  }

  horaValida = true;
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
  uint8_t h = rtc.getHours();
  if (nocheInicio == nocheFin) return false;
  if (nocheInicio < nocheFin) return (h >= nocheInicio && h < nocheFin);
  return (h >= nocheInicio || h < nocheFin);  // franja que cruza medianoche
}

const char *reloj_textoHora() {
  static char buf[6];
  if (!horaValida) return "--:--";
  snprintf(buf, sizeof(buf), "%02u:%02u", (unsigned)rtc.getHours(),
           (unsigned)rtc.getMinutes());
  return buf;
}
