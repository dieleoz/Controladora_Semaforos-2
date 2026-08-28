// ===== include/config_ciclo.h (ESCLAVO) =====
#pragma once
#include <Arduino.h>

// ---------------------------------------------------------------------------
// SFTY-23 — Configuracion del ciclo recibida del Maestro.
//
// FASE 2 (03/08/2026): la implementacion vive en src/config_ciclo.cpp. Antes estaba
// dentro de src/main.cpp, que era el delator de que faltaba un modulo: estas cuatro
// funciones son API PUBLICA y estaban implementadas EN EL PUNTO DE ENTRADA, de modo
// que no habia forma de probarlas sueltas.
//
// POR QUE EL FLAG "RECIBIDO" IMPORTA TANTO COMO EL VALOR
// -----------------------------------------------------
// Un 0 puede significar "el Maestro dijo cero" o "nunca llego nada", y el Modo
// Degradado no puede confundirlos: entrar sin saber la duracion del ciclo es
// operar a ciegas con las dos puntas calculando cosas distintas.
//
// LOS VALORES SE USAN TAL CUAL LLEGAN. NO SE AJUSTAN AQUI
// ------------------------------------------------------
// SFTY-21 pide que en Degradado el todo-rojo vaya AMPLIADO -del orden del doble-
// para absorber la deriva entre relojes. Esa ampliacion es decision del MAESTRO y
// viaja ya aplicada en CMD_CONFIG_DESPEJE. Si el Esclavo la duplicara por su
// cuenta, las dos puntas calcularian ciclos de duracion distinta sobre la misma
// hora: los verdes se solaparian a los pocos minutos. Es exactamente el fallo que
// ciclo_degradado.h existe para impedir, y se colaria por la puerta de al lado.
// ---------------------------------------------------------------------------

uint8_t config_verdeSegundos();     // duracion de CADA verde, en segundos
uint8_t config_despejeSegundos();   // duracion de CADA todo-rojo, en segundos
bool    config_verdeRecibido();
bool    config_despejeRecibido();

// ---------------------------------------------------------------------------
// Recepcion de las dos tramas del par (CMD_CONFIG_VERDE / CMD_CONFIG_DESPEJE).
//
// config_rxDespeje() DEVUELVE si el par se cerro, y el llamante decide si acusa. La
// division es deliberada: el acuse se hace con programarRespuesta(), que es la
// maquinaria de cortesia de SFTY-17 y vive en main.cpp. Dejar esa llamada dentro de
// este modulo lo ataria al transporte, y lo que aqui se guarda no depende de como se
// conteste.
//
// El silencio NO es un olvido: cuando el par no se cierra se calla A PROPOSITO, porque
// es lo que provoca el reintento del Maestro.
// ---------------------------------------------------------------------------
void config_rxVerde(uint8_t segundos);
bool config_rxDespeje(uint8_t segundos);   // true = par cerrado, hay que acusarlo
