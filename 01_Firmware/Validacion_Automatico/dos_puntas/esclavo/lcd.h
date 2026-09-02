// ===== Validacion_Automatico/dos_puntas/esclavo/lcd.h =====
//
// Sustituto de Esclavo/include/lcd.h. De toda la pantalla, lo unico que llaman los
// fuentes que este arnes compila son las dos funciones del arranque: setup() dibuja la
// bienvenida y nada mas. El resto de la API vive en menu.cpp, que aqui NO se compila.
//
// Un sustituto que ofreciera las seis funciones invitaria a que este arnes creciera
// hacia emular la LCD; el dibujo ya tiene su arnes propio sobre el lcd.cpp real
// (Validacion_LCD). Aqui lo que se mide es quien enciende un verde.
#pragma once

#include <Arduino.h>

void lcd_setup();
void lcd_dibujarBienvenida();
