// ===== include/modo_hora.h =====
#pragma once
#include <Arduino.h>

// ---------------------------------------------------------------------------
// SFTY-18 — Pantalla de ajuste del reloj.
//
// Es la unica via para poner en hora el RTC, y por tanto el requisito previo de
// todo lo que dependa de la hora: la operacion intermitente nocturna (SFTY-20) y
// el Modo Degradado (SFTY-21).
//
// ATENCION: ESAS TRES LINEAS DESCRIBEN UN EQUIPO QUE YA NO EXISTE, y se anotan en vez
// de borrarse porque son la cabecera que se lee primero.
//
//   D-15   la hora de este cruce la lleva el reloj con pila del modulo de expansion, y
//          es el UNICO que acusa la orden de ponerla. Esta pantalla ya no es "la unica
//          via": no es via ninguna.
//   D-17.bis  y ademas es INALCANZABLE. Su unico armador vive en la lista del menu, y
//          esa lista se navega con dos funciones que hoy devuelven siempre falso desde
//          que sus pines pasaron a ser camaras.
//
// El codigo se conserva porque la decision dice que se conserva; lo que no puede
// quedarse es la frase de arriba haciendose pasar por vigente.
//
// NO arranca ciclos ni toca las luces: el coordinador mantiene el estado que
// tuviera (Rojo Fijo con enlace, Ambar sin el), igual que PRUEBA ALCANCE.
// ---------------------------------------------------------------------------

void modo_hora_setup();
void modo_hora_loop();
