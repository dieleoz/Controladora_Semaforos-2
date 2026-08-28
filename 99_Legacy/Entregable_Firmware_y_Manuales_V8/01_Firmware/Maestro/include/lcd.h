// ===== include/lcd.h =====
#pragma once
#include <Arduino.h>

void lcd_setup();

void lcd_dibujarBienvenida();
void lcd_dibujarMenu(int cursor, const char* opciones[], int cantidad);
void lcd_dibujarManual(const char* nombreEstado);
void lcd_dibujarNoDisponible();
void lcd_dibujarConfigMinutos(const char* etiqueta, int valor);
void lcd_dibujarAutomatico(const char* nombreEstado, int minRojo, int minVerde);
void lcd_dibujarTextoRecibido(const char* texto);
void lcd_dibujarConfigValor(const char* etiqueta, int valor, const char* unidad);