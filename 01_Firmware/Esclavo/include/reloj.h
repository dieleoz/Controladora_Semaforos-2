// ===== include/reloj.h (ESCLAVO) =====
#pragma once
#include <Arduino.h>

// ---------------------------------------------------------------------------
// SFTY-18 / SFTY-23 — Reloj de tiempo real (RTC interno del STM32)
//
// Portado del Maestro. Usa el RTC que el propio microcontrolador lleva dentro,
// con el cristal Y2 de 32.768 kHz que ya viene en la tarjeta y una pila CR2032
// en VBAT. No ocupa ningun pin, cosa que aqui importa tanto o mas que en el
// Maestro: el I2C por hardware esta copado y un modulo externo habria obligado a
// I2C por software. Ver 03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md seccion 4.
//
// POR QUE EL ESCLAVO NECESITA RELOJ (SFTY-23):
// no es para decidir nada por su cuenta hoy, sino para poder ser PUESTO EN HORA
// desde el Maestro por radio y para poder MEDIR su propio desfase. Ajustar a mano
// las dos puntas dejaba hasta 59 s de diferencia el primer dia, casi cuatro veces
// el todo-rojo, y dos pantallas en HH:MM no pueden detectarlo. El dia que exista
// el Modo Degradado, esa base de tiempo comun es lo que permitira que las dos
// puntas sigan en fase sin radio.
//
// DIFERENCIAS RESPECTO A LA VERSION DEL MAESTRO:
//
//  1. NO se porta la franja nocturna (reloj_ajustarFranjaNocturna,
//     reloj_inicioNoche, reloj_finNoche, reloj_esHorarioNocturno).
//     La operacion intermitente por horario es una decision de CICLO, y el ciclo
//     lo decide el Maestro: el Esclavo solo obedece ordenes de luz. Duplicar aqui
//     una franja que ademas solo vive en RAM crearia una segunda fuente de verdad
//     que tras un apagon revertiria en silencio a 22-05 y podria discrepar de la
//     del Maestro. Dos puntas decidiendo por separado cuando es de noche es
//     exactamente el fallo que este proyecto no se puede permitir. Si algun dia el
//     Esclavo tiene que entrar en intermitente nocturno, lo hara porque el Maestro
//     se lo ordena, no porque mire su propio calendario.
//
//  2. NO se porta reloj_textoHora(): esa funcion existe para pintar "HH:MM" en la
//     LCD, y el Esclavo no tiene pantalla ni menu.
//
// Se conserva reloj_segundosDelDia() porque el Modo Degradado necesitara una base
// absoluta de tiempo para computar la fase del ciclo sin radio.
// ---------------------------------------------------------------------------

void reloj_setup();

// El RTC arranca sin fecha valida la primera vez (pila recien puesta o agotada).
// Mientras devuelva false, la hora de este nodo NO es de fiar y no debe usarse
// para nada: un reloj sin poner en hora es peor que no tener reloj. En SFTY-23
// esto es lo que obliga a contestar DELTA_FUERA_DE_RANGO en vez de un numero
// inventado que el operario leeria como un desfase real.
bool reloj_enHora();

// N-49 — El contador crudo del RTC: 32 bits de SEGUNDOS que mantiene la pila.
//
// Es la unica medida de tiempo de esta punta que sobrevive al corte Y es monotona.
// La hora de pared no sirve para fechar: el calendario esta anclado a enero y el dia
// vuelve de 31 a 1. Devuelve 0 si el RTC no esta operativo, y ese cero significa
// "no hay reloj": respaldo_marcarSync() y respaldo_horasDesdeSync() se abstienen.
//
// Tiene que ser IDENTICA a la del Maestro. Las dos puntas fechan la misma
// sincronizacion, y que cada una lo hiciera a su manera es de donde salio N-49.
uint32_t reloj_contadorSegundos();

// N-25 — reintento en segundo plano del cristal. Se llama desde el loop(). Adopta el
// reloj si el oscilador despierta despues del arranque, sin reiniciar.
void reloj_actualizar();

uint8_t reloj_hora();      // 0..23
uint8_t reloj_minuto();    // 0..59
uint8_t reloj_segundo();   // 0..59

// Dia del mes, 1..31. Devuelve 0 si el reloj no esta en hora.
//
// Lo necesita el respaldo (N-20) para saber cuanto hace de la ultima
// sincronizacion a traves de un reinicio: con solo los segundos del dia no se
// distingue "hace una hora" de "hace veinticinco". No interesa la fecha en si,
// solo poder restar dias.
uint8_t reloj_dia();

// Segundos transcurridos desde medianoche: 0..86399.
// Devuelve 0 si el reloj no esta en hora.
//
// OJO: esto NO es la base del modo autonomo al perder el radio (SFTY-19 / N-9).
// Aquel modo sincroniza de forma RELATIVA al ultimo mensaje del Maestro y le basta
// millis(). Esta funcion queda para el Modo Degradado (aplazado), que si necesita
// tiempo absoluto comun a las dos puntas.
uint32_t reloj_segundosDelDia();

// Ajusta el reloj y lo marca como valido.
// En el Esclavo la llama UNICAMENTE el manejador de CMD_HORA_S de main.cpp, y solo
// con las CUATRO cifras completas: aqui no hay teclado con el que un operario pueda
// ponerlo en hora, la unica via es la radio.
//
// `dia` (1..31) fija ademas el dia del mes. Con 0 -el valor por defecto- la fecha no
// se toca; se mantiene ese caso para que la firma sea la misma que en el Maestro,
// donde la pantalla de ajuste teclea solo HH:MM. Un dia fuera de 0..31 descarta la
// llamada entera, igual que una hora imposible: por radio la trama pudo llegar
// corrupta y colar por el CRC, y media hora escrita es peor que ninguna.
//
// POR QUE VIAJA EL DIA (CMD_HORA_D): para que las dos puntas cuenten los dias con el
// MISMO numero. No interesa la fecha real, interesa que esten ACOPLADAS. Hasta ahora
// cada unidad sembraba su propio dia 1 la primera vez que se ponia en hora, y los
// calendarios quedaban desacoplados para siempre. Consecuencia medida por el
// validador de costura: un corte de energia el dia en que el calendario de UNA punta
// pasa de 31 a 1 hace que respaldo_horasDesdeSync() declare CADUCADA solo en esa
// punta; esa no reanuda el Modo Degradado y se queda en AMBAR, mientras la otra
// reanuda en fase y sigue dando VERDE cada ciclo. Ambar contra verde es el peor
// resultado posible. Con los calendarios acoplados las dos fallan a la vez:
// simetrico y seguro.
void reloj_ajustar(uint8_t hora, uint8_t minuto, uint8_t segundo = 0, uint8_t dia = 0);
