// ===== src/lcd.cpp =====
#include "lcd.h"
#include "pines.h"
#include <U8g2lib.h>
#include <stdio.h>

static U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, LCD_SCLK, LCD_SID, LCD_CS, LCD_RST);

void lcd_setup() {
  pinMode(LCD_PSB, OUTPUT);
  digitalWrite(LCD_PSB, LOW);
  u8g2.begin();
}

void lcd_dibujarBienvenida() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(5, 20, "Semaforo");
  u8g2.drawStr(5, 36, "Inteligente V1.0");
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(30, 54, "IT Vial SAS");
  u8g2.sendBuffer();
}

void lcd_dibujarMenu(int cursor, const char* opciones[], int cantidad) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14_tr);
  u8g2.drawStr(30, 12, "MODO SEMAFORO");
  u8g2.drawHLine(0, 16, 128);

  for (int i = 0; i < cantidad; i++) {
    int y = 30 + i * 14;
    if (i == cursor) {
      u8g2.drawStr(2, y, ">");
    }
    u8g2.drawStr(14, y, opciones[i]);
  }
  u8g2.sendBuffer();
}

void lcd_dibujarManual(const char* nombreEstado) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14_tr);
  u8g2.drawStr(10, 14, "MODO: MANUAL");
  u8g2.drawHLine(0, 18, 128);
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.drawStr(10, 42, nombreEstado);
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(4, 60, "1/2=Cambiar 3=Rojo 4=Menu");
  u8g2.sendBuffer();
}

void lcd_dibujarNoDisponible() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14_tr);
  u8g2.drawStr(10, 30, "No disponible");
  u8g2.drawStr(10, 46, "aun (proximam.)");
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(4, 60, "4=Menu");
  u8g2.sendBuffer();
}

void lcd_dibujarConfigMinutos(const char* etiqueta, int valor) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14_tr);
  u8g2.drawStr(5, 14, "CONFIG. AUTOMATICO");
  u8g2.drawHLine(0, 18, 128);

  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(10, 32, etiqueta);

  char buf[16];
  snprintf(buf, sizeof(buf), "%02d min", valor);
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(30, 52, buf);

  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(4, 62, "1/2=+/- 3=OK 4=Menu");
  u8g2.sendBuffer();
}

void lcd_dibujarAutomatico(const char* nombreEstado, int minRojo, int minVerde) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14_tr);
  u8g2.drawStr(4, 14, "MODO: AUTOMATICO");
  u8g2.drawHLine(0, 18, 128);

  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.drawStr(10, 40, nombreEstado);

  char buf[24];
  snprintf(buf, sizeof(buf), "R:%02dm V:%02dm", minRojo, minVerde);
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(10, 52, buf);
  u8g2.drawStr(4, 62, "4=Menu");

  u8g2.sendBuffer();
}

void lcd_dibujarTextoRecibido(const char* texto) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14_tr);
  u8g2.drawStr(0, 14, "Recibido RS485:");
  u8g2.drawHLine(0, 18, 128);
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.drawStr(0, 40, texto);
  u8g2.sendBuffer();
}

void lcd_dibujarConfigValor(const char* etiqueta, int valor, const char* unidad) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14_tr);
  u8g2.drawStr(5, 14, "CONFIGURACION");
  u8g2.drawHLine(0, 18, 128);

  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(10, 32, etiqueta);

  char buf[16];
  snprintf(buf, sizeof(buf), "%02d %s", valor, unidad);
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(30, 52, buf);

  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(4, 62, "1/2=+/- 3=OK 4=Menu");
  u8g2.sendBuffer();
}