// =============================================================================
// registro.h - EL REGISTRO DEL PUENTE (SPEC_4 9.8; D-72, D-73, D-74)
// =============================================================================
//
// Lo que pasa sin telefono delante no lo guardaba nadie: el parte de arranque solo
// sale al conectar y la primera aparicion de un $ALARM se pierde. Este modulo guarda
// lo que SUBE del equipo en la particion `spiffs` (896 KB, sin uso hasta hoy), en
// crudo y sin sistema de ficheros, y lo sirve a la app cuando ella lo pide.
//
// Fuera del camino de las luces: no escribe nada hacia el STM32 y no cambia ni un
// byte de lo que cruza. registro_anotar() solo copia a RAM; la flash la toca
// registro_volcar(), un registro por vuelta de loop() como mucho.
// =============================================================================
#ifndef REGISTRO_H
#define REGISTRO_H

#include <stdint.h>
#include <stdbool.h>

// D-72: busca la particion y localiza la cabeza de escritura (224 lecturas de 4 B).
// No emite nada. false = sin particion: no se anota y la consulta contesta SIN_FLASH.
bool registro_setup();

// Copia a RAM un registro de clase 'E', 'S' o 'P'. texto: payload sin '$' (se corta
// en '*' o en el nulo, y a REGISTRO_TEXTO_MAX con '~'). Nunca bloquea.
void registro_anotar(char clase, const char* texto);

// D-73: el filtro de lo que sube del equipo. trama: la linea VALIDA ya sellada, con su
// '$' y su '*XX'. E = todo lo que no es $STATUS salvo REGISTRO_EXCLUIR; S = el $STATUS
// cuyo MODO o ESTADO cambia.
void registro_verSubida(const char* trama);

// Lleva a flash como mucho UN registro del anillo (y borra antes su sector si lo estrena).
void registro_volcar();

// D-74: contesta a la linea CMD:LOG,DESDE:<N> por puente_emitirPropio().
void registro_atender(const char* linea);

uint32_t registro_ultimo();
uint32_t registro_primero();
unsigned long registro_perdidasAnillo();
unsigned long registro_fallosFlash();
unsigned long registro_malos();

#endif // REGISTRO_H
