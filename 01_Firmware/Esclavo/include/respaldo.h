// ===== include/respaldo.h =====
#pragma once
#include <Arduino.h>

// ---------------------------------------------------------------------------
// N-20 — Memoria que sobrevive al corte de energia.
//
// ESTE FICHERO Y SU .cpp DEBEN SER IDENTICOS EN MAESTRO Y ESCLAVO.
//
// Usa los registros de respaldo del STM32 (BKP->DR1..DR10), alimentados por LA
// MISMA pila CR2032 que ya mantiene el RTC. No hace falta EEPROM, ni escribir en
// flash, ni gastar ciclos de borrado: son diez palabras de 16 bits que siguen ahi
// mientras la pila aguante.
//
// PARA QUE, Y POR QUE NO ES UN LUJO
// ---------------------------------
// Sin esto, los indicadores de sincronizacion y la configuracion del ciclo viven
// en RAM. Un microcorte reinicia una unidad, que arranca sin ellos y cae a AMBAR,
// mientras la otra sigue dando verde por reloj:
//
//     una punta AMBAR   -> el conductor negocia el paso
//     otra punta VERDE  -> el conductor pasa confiado
//
// Ese cruce es el riesgo residual n.2 de SFTY-21, el que se documento como "sin
// solucion tecnica sin radio". Si la unidad recuerda que estaba en Degradado y con
// que ciclo, REANUDA EN FASE en lugar de crear la asimetria.
//
// Y conviene decirlo al reves para que no suene a comodidad: REANUDAR ES MAS
// SEGURO QUE CAER A AMBAR, porque caer a ambar es justo lo que crea el escenario
// peligroso. La pila no esta aqui para no reconfigurar: esta para no desincronizar.
//
// LO QUE ESTO NO ES
// -----------------
// No es autorizacion por adelantado. La unidad solo reanuda un modo que UNA PERSONA
// autorizo antes, y solo si la autorizacion sigue vigente. Guardar "si algun dia
// pierdes el radio, entra" seria entrada automatica con pasos extra, y eso sigue
// descartado: nadie confirmaria que la otra punta sigue viva.
// ---------------------------------------------------------------------------

void respaldo_setup();

// True si el contenido es nuestro y esta integro. Si devuelve false hay que tratar
// TODO lo demas como ausente: pila agotada, equipo nuevo o dominio de respaldo
// corrupto por un arranque sucio.
bool respaldo_valido();

// --- Configuracion del ciclo degradado -------------------------------------
void respaldo_guardarCiclo(uint8_t verdeSeg, uint8_t despejeSeg);
uint8_t respaldo_verdeSeg();
uint8_t respaldo_despejeSeg();

// --- N-133: los tiempos del ciclo AUTOMATICO, que no se guardaban en ningun sitio ---
//
// OJO A LA UNIDAD, porque las dos parejas de arriba y esta NO son lo mismo:
// respaldo_verdeSeg()/despejeSeg() son del MODO DEGRADADO y van en SEGUNDOS; estos son
// del ciclo AUTOMATICO y el rojo y el verde van en MINUTOS.
//
// Sobreviven al corte porque los BKP viven en el dominio de VBAT. Sin pila, o con la
// pila agotada, respaldo_setup() encuentra el contenido invalido y borra: entonces
// respaldo_tiemposCiclo() devuelve false y el equipo arranca con sus minimos, que es la
// direccion segura. Un cruce lento molesta; uno rapido mata.
void respaldo_guardarTiemposCiclo(uint8_t rojoMin, uint8_t verdeMin, uint8_t despejeSeg);
bool respaldo_tiemposCiclo(uint8_t* rojoMin, uint8_t* verdeMin, uint8_t* despejeSeg);
bool respaldo_hayCiclo();

// --- Marca de la ultima sincronizacion horaria -----------------------------
//
// Se guarda el CONTADOR DEL RTC (reloj_contadorSegundos()): 32 bits de segundos que
// mantiene la pila. No es un contador de milisegundos del programa -ese se pierde al
// reiniciar, que es justo el caso que esto cubre- ni una fecha de calendario.
//
// N-49: antes se guardaba dia-del-mes + segundo-del-dia, y esa pareja NO PUEDE fechar.
// Con el calendario anclado a enero el dia va 1..31 y vuelve a 1, asi que la vuelta
// daba una resta negativa -CADUCADA en una punta y no en la otra- y ademas "hoy" y
// "hace 31 dias" producian los mismos numeros. El contador del RTC es monotono y no
// vuelve en 136 anos: los dos agujeros se cierran con una resta.
//
// Estas dos funciones son PURAS a proposito -reciben el contador, no lo leen- para
// que Validacion_Respaldo pueda ejercerlas compiladas en el PC. Si leyeran el RTC por
// dentro, la unica forma de probarlas seria con la tarjeta delante.
//
// El cero significa "no hay reloj": no se marca y no se fecha.
void respaldo_marcarSync(uint32_t segundosRtc);
bool respaldo_haySync();

// Horas transcurridas desde la ultima sincronizacion, o RESPALDO_SYNC_CADUCADA si
// no se puede calcular sin ambiguedad.
//
// REGLA: ante la duda, CADUCADA. Es lo que separa "no se cuanto ha pasado" de
// "ha pasado poco", y confundirlos autorizaria el Modo Degradado sobre una
// sincronizacion de antiguedad desconocida. Quedan dos casos: sin reloj (cero), y
// contador que RETROCEDE -alguien puso la hora, o se reinicio el dominio de respaldo-.
static const uint32_t RESPALDO_SYNC_CADUCADA = 0xFFFFFFFFUL;
uint32_t respaldo_horasDesdeSync(uint32_t segundosRtcAhora);

// --- Modo Degradado activo -------------------------------------------------
// Permite REANUDAR tras un corte en vez de caer a ambar y desincronizarse de la
// otra punta. Ver la nota de cabecera.
void respaldo_guardarDegradado(bool activo);
bool respaldo_degradadoActivo();

// Borra todo. Deja el respaldo como en un equipo nuevo.
void respaldo_borrar();
