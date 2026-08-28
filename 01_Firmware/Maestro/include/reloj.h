// ===== include/reloj.h =====
#pragma once
#include <Arduino.h>

// ---------------------------------------------------------------------------
// SFTY-18 — Reloj de tiempo real (RTC interno del STM32)
//
// Usa el RTC que el propio microcontrolador lleva dentro, con el cristal Y2 de
// 32.768 kHz que ya viene en la tarjeta y una pila CR2032 en VBAT.
// No ocupa ningun pin: el I2C por hardware esta copado (PB6/PB7 los usa la LCD y
// PB10/PB11 el RS-485), asi que un modulo externo habria obligado a I2C por
// software. Ver 03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md seccion 4.
//
// Sirve para la operacion intermitente nocturna: sin saber la hora, el equipo no
// puede decidir cuando entrar en ese modo.
// ---------------------------------------------------------------------------

void reloj_setup();

// El RTC arranca sin fecha valida la primera vez (pila recien puesta o agotada).
// Mientras devuelva false, la funcion nocturna NO debe activarse: un reloj sin
// poner en hora encenderia el modo intermitente a deshora.
bool reloj_enHora();

// N-24 — ¿arranco el oscilador del cristal Y2? Distinto de reloj_enHora(), y la
// diferencia es la que el operario necesita ver:
//
//   enHora=false, hayCristal=true   -> el reloj cuenta, pero nadie lo ha puesto en
//                                      hora. Se arregla desde AJUSTAR HORA.
//   enHora=false, hayCristal=false  -> no hay con que contar. NO se arregla desde el
//                                      menu: es la pila, R5 o el propio cristal.
//
// Sin esta distincion, ajustar la hora parecia funcionar -la pantalla la mostraba- y
// al apagar y encender volvia a ceros, sin nada que explicara por que.
bool reloj_hayCristal();

// N-45 — CONSULTA DEL RELOJ: los bits crudos, sin interpretar.
//
// Hasta aqui, cuando el reloj no arrancaba la pantalla decia "Revisa Y2, pila y R5" y
// "Es Y2: toca hardware". Esos textos estaban ESCRITOS A MANO: el firmware no habia
// medido la pila -el STM32F103 no tiene canal de ADC para VBAT- ni el cristal. Eran
// conclusiones fijas presentadas como diagnostico, y mandaron a cambiar pila, R5 e Y2
// tres veces con el hardware sano.
//
// Esta estructura no concluye nada. Dice lo que el micro VE, y quien mira decide:
//
//   lseOn=1, lseRdy=0  -> el oscilador esta pedido y no arranca. Ahi si mira Y2, sus
//                         condensadores de carga y la soldadura.
//   lseOn=0            -> ni siquiera se esta pidiendo. Es firmware o dominio de
//                         respaldo bloqueado, NO es el cristal.
//   lseByp=1           -> espera reloj externo por OSC32_IN. Con un cristal normal
//                         nunca arrancara, y no es culpa del cristal.
//   rtcSel=0           -> el RTC no esta atado a ninguna fuente: no cuenta aunque el
//                         cristal oscile.
//   rtcEn=0            -> RTC deshabilitado. OJO: con esto en 0 no se leen sus
//                         registros de contador, y por eso cnt no se rellena.
//   cnt cambiando entre dos visitas -> el RTC CUENTA. Distingue "no cuenta" de
//                         "cuenta pero nadie lo ha puesto en hora".
struct RelojDiag {
  bool lseOn;
  bool lseRdy;
  bool lseByp;
  uint8_t rtcSel;    // 0=ninguna, 1=LSE, 2=LSI, 3=HSE/128
  bool rtcEn;
  bool cntLeido;     // false si rtcEn=0: no se toco el periferico
  uint32_t cnt;      // contador crudo del RTC, solo si cntLeido
  bool configurado;  // rtc.isConfigured()
  uint16_t anio;     // 0 si no se pudo leer
};

// Rellena la estructura leyendo RCC->BDCR y, solo si el RTC esta habilitado, su
// contador. No modifica NADA: se puede llamar desde cualquier pantalla sin efectos.
void reloj_diagnostico(RelojDiag* d);

// N-49 — El contador crudo del RTC: 32 bits de SEGUNDOS que mantiene la pila.
//
// Es la unica medida de tiempo de este equipo que sobrevive al corte Y es monotona.
// La hora de pared no sirve para fechar: el calendario esta anclado a enero (ver
// reloj_fijarEnero) y el dia vuelve de 31 a 1, asi que restar dias del mes no
// distingue "ayer" de "hace un mes y un dia". Este contador no vuelve en 136 anos.
//
// Devuelve 0 si el RTC no esta operativo, y ese cero es un valor con significado:
// respaldo_marcarSync() y respaldo_horasDesdeSync() lo tratan como "no hay reloj" y
// se abstienen, en vez de fechar contra un contador que nadie hace avanzar.
uint32_t reloj_contadorSegundos();

// N-25 — reintento en segundo plano del cristal. Se llama desde el loop(). Si el
// oscilador no arranco en el setup, lo vuelve a mirar cada 30 s y ADOPTA el reloj en
// cuanto despierte, sin reiniciar. Un cristal marginal o frio puede tardar mas de los
// 2 s del arranque, y condenarlo por eso seria confundir "lento" con "muerto".
// No hace nada si el RTC ya esta operativo.
void reloj_actualizar();

// N-31 — Reinicia el DOMINIO DE RESPALDO entero y reintenta arrancar el oscilador.
//
// Ultima carta antes de dar por muerto el cristal: si un firmware anterior dejo el
// LSE mal configurado, esos registros sobreviven a los reinicios -viven de la pila- y
// ningun arranque normal los limpia.
//
// BORRA LA HORA Y TODO EL RESPALDO (ciclo acordado, marca de sincronizacion,
// indicador del Degradado). Por eso lo pide una persona desde el menu y no se hace
// solo. Devuelve true si tras el reinicio el oscilador arranca.
bool reloj_reiniciarDominioRespaldo();

uint8_t reloj_hora();      // 0..23
uint8_t reloj_minuto();    // 0..59
uint8_t reloj_segundo();   // 0..59

// Mantiene el calendario anclado a ENERO.
//
// El Esclavo queda fijado a enero en cada sincronizacion, pero el mes del Maestro
// avanzaria solo (ene -> feb -> ...). Y el RTC decide cuando pasa de 31 a 1 SEGUN LA
// LONGITUD DEL MES: en febrero el Maestro volcaria 28->1 mientras el Esclavo, en
// enero, sigue en 29. Los dos calendarios volverian a separarse hasta la siguiente
// sincronizacion, y un corte de energia en esa ventana reproduce la asimetria
// ambar-contra-verde que CMD_HORA_D vino a cerrar.
//
// Anclando las dos puntas a enero, ambas vuelcan en 31 y en el mismo instante. No
// importa que el mes sea falso: el equipo no muestra la fecha ni la interpreta, solo
// resta dias. Lo que importa es que resten IGUAL.
//
// Se llama periodicamente; es idempotente y no toca la hora.
void reloj_fijarEnero();

// Dia del mes, 1..31. Devuelve 0 si el reloj no esta en hora.
//
// Lo necesita el respaldo (N-20) para saber cuanto hace de la ultima
// sincronizacion a traves de un reinicio: con solo los segundos del dia no se
// distingue "hace una hora" de "hace veinticinco". No interesa la fecha en si -el
// equipo no la muestra ni la usa para nada mas-, solo poder restar dias.
uint8_t reloj_dia();

// Segundos transcurridos desde medianoche: 0..86399.
// Devuelve 0 si el reloj no esta en hora.
//
// OJO: esto NO es la base del modo autonomo al perder el radio (SFTY-19 / N-9).
// Aquel modo sincroniza de forma RELATIVA al ultimo mensaje del Maestro y le basta
// millis(); no usa la hora absoluta ni obliga a poner pila en el Esclavo. Anclar
// las dos unidades a su reloj de pared seria peor: dos relojes puestos en hora a
// mano difieren desde el primer dia, mientras que la sincronizacion relativa
// arranca en cero.
// Esta funcion existe para la operacion intermitente NOCTURNA (N-3), aplazada.
uint32_t reloj_segundosDelDia();

// Ajusta el reloj y lo marca como valido.
//
// `dia` (1..31) fija ademas el dia del mes. Con 0 -el valor por defecto- la fecha
// no se toca, que es lo que necesita la pantalla de ajuste: el operario teclea
// HH:MM y no tiene por que saber la fecha.
//
// Por radio SI viaja el dia (CMD_HORA_D), para que las dos puntas cuenten los dias
// con el MISMO numero. No interesa la fecha real: interesa que esten acopladas. Con
// calendarios independientes, un corte el dia del cambio de mes deja a UNA punta
// sin reanudar -en ambar- mientras la otra reanuda y da verde.
void reloj_ajustar(uint8_t hora, uint8_t minuto, uint8_t segundo = 0, uint8_t dia = 0);

// --- Franja nocturna configurable -----------------------------------------
//
// ATENCION: HOY SE GUARDA SOLO EN RAM. No sobrevive a un corte de energia.
//
// Tras un apagon la HORA si sobrevive gracias a la pila, pero la franja revierte
// EN SILENCIO a 22-05. Y como la hora es valida, reloj_enHora() devuelve true y
// nada impide que el modo nocturno arranque con el horario equivocado de esa obra:
// la guarda de hora fiable NO cubre este caso.
//
// Por eso, mientras no exista la persistencia real (BKP->DR1..DR10; la libreria
// STM32duino RTC 1.9.0 no expone esos registros), la regla es:
//   franja NO confirmada tras el arranque  =>  modo nocturno INHIBIDO.
//
// Este comentario describia antes una persistencia que el codigo nunca tuvo. En un
// proyecto cuya trazabilidad se levanta buscando en los fuentes, esa mentira es
// peor que la carencia. Ver pendiente N-15.
void reloj_ajustarFranjaNocturna(uint8_t horaInicio, uint8_t horaFin);
uint8_t reloj_inicioNoche();
uint8_t reloj_finNoche();

// True si la hora actual cae dentro de la franja nocturna.
// Devuelve SIEMPRE false si el reloj no esta en hora (ver reloj_enHora).
// Contempla franjas que cruzan la medianoche, p. ej. 22:00 -> 05:00.
bool reloj_esHorarioNocturno();

// Texto "HH:MM" para pantalla. Devuelve "--:--" si el reloj no esta en hora.
const char* reloj_textoHora();
