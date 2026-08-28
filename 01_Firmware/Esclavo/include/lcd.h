// ===== include/lcd.h (ESCLAVO) =====
#pragma once
#include <Arduino.h>

// ---------------------------------------------------------------------------
// N-16 — Interfaz del Esclavo.
//
// Hasta el 01/08/2026 este fichero era una COPIA LITERAL del Maestro y declaraba
// pantallas de MANUAL, AUTOMATICO e INTELIGENTE que ningun .cpp de este proyecto
// implementaba. Se reescribe entero. Una cabecera que promete funciones
// inexistentes es peor que no tener cabecera: quien la lee da por hecho que el
// equipo sabe hacer cosas que no sabe, y el compilador no la desmiente hasta el
// enlazado, que es tarde.
//
// El Esclavo tiene exactamente tres pantallas porque OBEDECE, NO MANDA: mirar su
// estado, entrar y salir del Modo Degradado, y el menu que lleva a las dos. No
// hay pantalla de ajuste de hora a proposito (SFTY-23): la hora llega por radio
// desde el Maestro, y ponerla a mano aqui reintroduce el desfase de hasta 59 s
// que ese mecanismo elimino.
//
// LIMITE DE 128x64 px, QUE NO ES UN CONSEJO
// -----------------------------------------
// U8g2 RECORTA EN SILENCIO lo que cae fuera: un texto mal colocado no da error,
// simplemente no aparece. En el Maestro, una quinta opcion de menu con
// interlineado de 11 px caia en y=72 -invisible- y el cursor SI podia navegar
// hasta ella. Se resolvio con interlineado adaptativo. Aqui el menu tiene 2
// opciones y ese caso no puede darse, pero la salvaguarda de no dibujar por
// debajo de y=63 se conserva igual: la proxima opcion que alguien anada no
// tiene por que volver a descubrir el fallo.
// ---------------------------------------------------------------------------

void lcd_setup();

void lcd_dibujarBienvenida();

// Menu de navegacion. "pie" es una linea inferior opcional (NULL para omitirla)
// donde se avisa de que el Modo Degradado esta gobernando la luz: el tecnico que
// sube al gabinete debe verlo sin tener que entrar a ninguna pantalla.
void lcd_dibujarMenu(int cursor, const char* opciones[], int cantidad, const char* pie);

// Pantalla ESTADO.
//
// La hora va en HH:MM:SS y los segundos NO son decorativos: dos relojes a 40 s de
// distancia muestran el mismo "14:32", asi que una pantalla en HH:MM no comprueba
// nada. Esta es la verificacion de respaldo cuando el radio ya murio y CMD_DELTA
// no esta disponible.
//
// syncVencida: la ultima sincronizacion supero el limite duro de 48 h. Se pasa
// como bandera aparte porque a partir de ahi el numero de milisegundos deja de
// ser fiable (millis() da la vuelta a los 49,7 dias) y se muestra ">48h".
void lcd_dibujarEstado(bool enHora, uint8_t hora, uint8_t minuto, uint8_t segundo,
                       bool huboSync, unsigned long msDesdeSync, bool syncVencida,
                       unsigned long bytes, unsigned long validas,
                       const char* nombreLuz);

// Pantalla MODO DEGRADADO (SFTY-21).
//
// estadoTxt: en que punto esta el modo (inactivo, entrando, activo, rendido...).
// detalleTxt: segunda linea libre; el modulo la compone con la fase y la cuenta
//   atras cuando esta activo, y con el resumen de condiciones cuando no lo esta,
//   para que el operario sepa si va a poder entrar ANTES de pulsar.
// avisoLimite: se acerca el limite duro de 48 h. Se recuadra la linea del
//   contador, que es la unica forma de que se note desde lejos y de madrugada.
void lcd_dibujarDegradado(const char* estadoTxt, const char* detalleTxt,
                          bool huboSync, unsigned long msDesdeSync, bool syncVencida,
                          bool avisoLimite, const char* pie);

// Entrada RECHAZADA al Modo Degradado. Pantalla propia y no una linea mas: el
// rechazo tiene que ser inequivoco. Si se mostrara como un aviso discreto, el
// operario podria marcharse creyendo que el modo quedo activo, que es
// exactamente el escenario contra el que existen las condiciones de entrada.
void lcd_dibujarRechazoDegradado(const char* motivo);
