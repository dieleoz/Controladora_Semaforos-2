// ===== 01_Firmware/ESP32_Expansion/include/puente.h =====
//
// EL BOMBEO. Dos sentidos, una vuelta, y el reset del watchdog fuera de los dos.
//
// LO QUE ESTE MODULO NO HACE, Y CADA UNO TIENE UN PACK DETRAS:
//
//   - NO ORIGINA (B-1). Todo lo que sale hacia el STM32 procede del buffer de entrada
//     de la app. No hay un solo literal de comando en el camino de escritura.
//   - NO PARTE NI UNE TRAMAS (B-5). Una entra entera y sale entera.
//   - Y DESDE N-145 TIENE UNA EXCEPCION, LA UNICA, ESCRITA AQUI PARA QUE NADIE LEA LA
//     LINEA DE ARRIBA COMO UN ABSOLUTO QUE YA NO ES: en el sentido equipo -> app, un
//     "HORA:--:--:--" puede salir relleno con la hora del DS3231 de este modulo. Ni un
//     byte mas ni uno menos -"--:--:--" y "HH:MM:SS" miden ocho-, solo sobre el HUECO
//     -si el STM32 puso una hora, no se toca-, y solo si la barrera del reloj la da por
//     fiable -si no, el hueco se queda-. El porque, con la medida del 04/09 y las tres
//     cotas del cambio, esta entero sobre sellarHoraSiFaltaba() en puente.cpp.
//   - NO FILTRA POR PREFIJO. Son cinco -$STATUS, $ACK, $ERR, $ALARM y $EVENT- y un
//     puente que filtrara por cuatro se comeria la bitacora entera. Se valida el
//     FORMATO, no el contenido: asi el protocolo puede crecer sin recompilar el puente.
//   - NO REINTENTA (B-2). Reintentar MANUAL:CAMBIAR_TURNO es pedir dos cambios de turno.
//   - NO ANADE TRAFICO PERIODICO (P-1) NI AGRUPA TELEMETRIA (P-4). La app declara el
//     enlace perdido a los 5 s sin trama: un puente que juntara dos $STATUS para
//     ahorrar aire haria que la app diera por caido a un equipo sano.
//   - NO TIENE MODO POR DEFECTO y no manda nada en setup(). Silencio no es orden.

#ifndef PUENTE_H
#define PUENTE_H

#include <Arduino.h>

void puente_setup();

// UNA vuelta del bucle exterior: atiende los dos sentidos y vuelve.
//
// Los dos bucles interiores llevan tope de iteraciones (W-4) para que esta funcion
// SIEMPRE devuelva. Es la mitad de la defensa del watchdog: si el bucle interior
// pudiera no terminar -con ruido continuo, como el 31/07-, alimentar al perro desde
// dentro lo mantendria contento mientras el puente no progresa.
void puente_bombear();

// Emite una trama PROPIA del puente hacia la app. Nunca hacia el STM32.
//
// B-3: el puente no compone $ACK ni $STATUS en nombre del equipo. Lo que sale de aqui
// habla de si mismo y va marcado con NODE:PUENTE, porque un $ERR del puente que
// pareciera del STM32 manda a diagnosticar el poste equivocado.
void puente_emitirPropio(const char* payload);

// P-3: lo que se descarta se cuenta y se puede leer. Un byte tirado en silencio se lee
// como que nunca existio, y eso es E-2 otra vez -el truncado mudo del STM32-.
unsigned long puente_descartadasPorCrc();
unsigned long puente_descartadasPorLargo();
unsigned long puente_tramasAlaApp();

#endif // PUENTE_H
