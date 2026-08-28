// ===== src/lcd.cpp (ESCLAVO) =====
#include "lcd.h"
#include "pines.h"
#include <U8g2lib.h>
#include <stdio.h>

// ---------------------------------------------------------------------------
// N-16 — Portado de src/lcd.cpp del Maestro y RECORTADO a lo que el Esclavo
// necesita. No se copian sus pantallas de modo de operacion: el Esclavo no
// decide el ciclo, y una pantalla que ofrece decidirlo acaba con las dos puntas
// peleandose por quien manda.
//
// Se conservan del original tres cosas que costaron fallos de campo aprender:
//
//   1. El interlineado adaptativo del menu (aqui sobra con 2 opciones, pero la
//      salvaguarda de no dibujar bajo y=63 se queda).
//   2. abreviarCuenta(), porque un contador de 6 cifras invadia el "4=Menu".
//   3. La lectura de linea de SFTY-15 tal cual, que distingue "no llega nada" de
//      "llega basura" sin instrumentos.
// ---------------------------------------------------------------------------

#ifdef LCD_VALIDACION_NATIVA
// Compilacion para PC (arnes de 01_Firmware/Validacion_LCD): el arnes provee un
// U8G2 con framebuffer para volcar las pantallas y comprobar que todo cabe en
// 128x64 px. El codigo de dibujo de abajo es EXACTAMENTE el mismo que va al
// firmware, de modo que la validacion no puede desviarse de lo que se compila.
//
// El arnes de hoy solo enlaza las pantallas del Maestro; este gancho queda
// puesto para que anadir el Esclavo sea cuestion de una linea en compilar.ps1 y
// no de tocar este fichero.
extern U8G2 &u8g2;
#else
// LA PANTALLA SE QUEDA CON TRES HILOS: SCLK, SID y CS. El reset va con
// U8X8_PIN_NONE a proposito, no por descuido.
//
// PB6 y PB7 son el USART1 REMAPEADO, y el modulo Bluetooth de campo entra por
// ahi (conector J17, posiciones 3 y 2). Mientras lcd.cpp se quedara con esos dos
// pines no habia telemetria: dos perifericos no pueden gobernar el mismo pin.
//
// Se pueden soltar porque NINGUNO DE LOS DOS LLEVA DATOS del display:
//   - LCD_RST (PB7) solo pulsa el reset al arrancar. El ST7920 arranca sin el;
//     si se conserva la pantalla, su patilla RST se ata a 3,3 V en el cable.
//   - LCD_PSB (PB6) era un nivel estatico -ver lcd_setup()-.
// Los datos son LCD_SCLK, LCD_SID y LCD_CS, y esos no se tocan.
//
// IDENTICO AL MAESTRO A PROPOSITO: las dos puntas comparten el cableado de J17.
static U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, LCD_SCLK, LCD_SID, LCD_CS, U8X8_PIN_NONE);
#endif

// Abrevia un contador para que no desborde la fila: 999 -> "999", 12345 -> "12k".
// Sin esto, un contador de 6 cifras hace que la linea de diagnostico invada el
// "4=Menu" de la derecha (lo detecto el arnes de validacion en el Maestro).
static void abreviarCuenta(unsigned long n, char *destino, size_t tam) {
  if (n < 1000UL)            snprintf(destino, tam, "%lu", n);
  else if (n < 1000000UL)    snprintf(destino, tam, "%luk", n / 1000UL);
  else                       snprintf(destino, tam, "%luM", n / 1000000UL);
}

// Antiguedad de la ultima sincronizacion, en texto corto y acotado a 6
// caracteres ("59s", "45m", "12h30m", ">48h").
//
// El caso "vencida" NO se calcula, se recibe como bandera. millis() da la vuelta
// a los 49,7 dias y una resta sin signo volveria a dar un numero pequeno: la
// pantalla diria "12m" tras dos meses sin sincronizar. Ese numero seria mentira
// justo en el escenario mas peligroso, asi que el latch lo decide quien lleva la
// cuenta y aqui solo se pinta.
static void textoAntiguedad(bool hubo, unsigned long ms, bool vencida,
                            char *destino, size_t tam) {
  if (!hubo)    { snprintf(destino, tam, "NUNCA"); return; }
  if (vencida)  { snprintf(destino, tam, ">48h");  return; }
  unsigned long s = ms / 1000UL;
  if (s < 60UL)        snprintf(destino, tam, "%lus", s);
  else if (s < 3600UL) snprintf(destino, tam, "%lum", s / 60UL);
  else                 snprintf(destino, tam, "%luh%02lum", s / 3600UL, (s % 3600UL) / 60UL);
}

// Linea inferior de diagnostico de enlace (SFTY-15). El Esclavo no tiene
// telemetria de calidad como el Maestro -no es el que interroga-, asi que lo
// unico honesto que puede mostrar son sus contadores de linea en bruto:
//   RX 0             -> no llega nada    (cobertura, canal, antena)
//   RX 4k - BASURA   -> llega ruido      (cableado, linea flotando, radio atascada)
//   RX 36  9tr       -> enlace correcto
static void dibujarLineaEnlace(unsigned long bytes, unsigned long validas) {
  char buf[24], nb[8], nv[8];
  u8g2.setFont(u8g2_font_5x7_tr);
  abreviarCuenta(bytes, nb, sizeof(nb));
  abreviarCuenta(validas, nv, sizeof(nv));
  if (bytes == 0) {
    snprintf(buf, sizeof(buf), "RX 0 - nada llega");
  } else if (validas == 0) {
    snprintf(buf, sizeof(buf), "RX %s - BASURA", nb);
  } else {
    snprintf(buf, sizeof(buf), "RX %s  %s tr", nb, nv);   // tr = tramas validas
  }
  // Se mantiene por debajo de x=90 para no invadir el "4=Menu" de la derecha,
  // incluso con contadores de seis cifras: por eso van abreviados.
  u8g2.drawStr(2, 62, buf);
  u8g2.drawStr(96, 62, "4=Menu");
}

void lcd_setup() {
  // AQUI YA NO SE TOCA LCD_PSB (PB6), y el pin queda libre para el USART1.
  //
  // Lo que hacia era un unico digitalWrite(LOW) en el arranque y nunca mas: un
  // NIVEL ESTATICO que elige el modo serie del ST7920, no una linea de datos.
  // Un GPIO entero -y ademas uno de los dos del USART1 remapeado- gastado en
  // sujetar un pin a masa. Eso lo hace igual de bien un hilo: en el cable de la
  // pantalla, PSB va a GND (J17 tiene masa en las posiciones 7 y 9).
  //
  // OJO SI SE VUELVE A MONTAR LA PANTALLA (N-22): hay modulos que traen PSB
  // estrapado a VCC de fabrica. Ahi el puente a masa serian dos salidas en
  // corto, asi que se comprueba con el multimetro ANTES de puentear.
  u8g2.begin();
}

void lcd_dibujarBienvenida() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(5, 20, "Semaforo");
  u8g2.drawStr(5, 38, "ESCLAVO");
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(30, 56, "IT Vial SAS");
  u8g2.sendBuffer();
}

void lcd_dibujarMenu(int cursor, const char* opciones[], int cantidad, const char* pie) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(20, 12, "MENU ESCLAVO");
  u8g2.drawHLine(0, 16, 128);

  // Mismo interlineado adaptativo que el Maestro. Con 2 opciones cae en y=28 y
  // y=39, muy dentro de pantalla; se deja la regla completa para que quien anada
  // una tercera o una cuarta no tenga que volver a descubrir por que existia.
  const int base = (cantidad <= 4) ? 28 : 24;
  const int paso = (cantidad <= 4) ? 11 : 9;

  u8g2.setFont(u8g2_font_6x10_tr);
  for (int i = 0; i < cantidad; i++) {
    int y = base + i * paso;
    if (y > 63) break;   // salvaguarda: nunca dibujar fuera de pantalla
    if (i == cursor) u8g2.drawStr(2, y, ">");
    u8g2.drawStr(12, y, opciones[i]);
  }

  if (pie != NULL) {
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(2, 62, pie);
  }
  u8g2.sendBuffer();
}

void lcd_dibujarEstado(bool enHora, uint8_t hora, uint8_t minuto, uint8_t segundo,
                       bool huboSync, unsigned long msDesdeSync, bool syncVencida,
                       unsigned long bytes, unsigned long validas,
                       const char* nombreLuz) {
  char buf[28], ant[10];

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(2, 12, "ESTADO");
  u8g2.drawHLine(0, 15, 128);

  // Reloj propio, con SEGUNDOS. Si no esta en hora se pintan guiones y no ceros:
  // un "00:00:00" parece una hora y alguien podria darla por buena.
  if (enHora) {
    snprintf(buf, sizeof(buf), "%02u:%02u:%02u",
             (unsigned)hora, (unsigned)minuto, (unsigned)segundo);
  } else {
    snprintf(buf, sizeof(buf), "--:--:--");
  }
  u8g2.drawStr(30, 30, buf);

  u8g2.setFont(u8g2_font_6x10_tr);
  textoAntiguedad(huboSync, msDesdeSync, syncVencida, ant, sizeof(ant));
  snprintf(buf, sizeof(buf), "Sync hace: %s", ant);
  u8g2.drawStr(2, 42, buf);

  snprintf(buf, sizeof(buf), "Luz: %s", nombreLuz);
  u8g2.drawStr(2, 52, buf);

  dibujarLineaEnlace(bytes, validas);
  u8g2.sendBuffer();
}

void lcd_dibujarDegradado(const char* estadoTxt, const char* detalleTxt,
                          bool huboSync, unsigned long msDesdeSync, bool syncVencida,
                          bool avisoLimite, const char* pie) {
  char buf[28], ant[10];

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(2, 12, "MODO DEGRADADO");
  u8g2.drawHLine(0, 15, 128);

  u8g2.setFont(u8g2_font_6x10_tr);
  if (estadoTxt  != NULL) u8g2.drawStr(2, 27, estadoTxt);
  if (detalleTxt != NULL) u8g2.drawStr(2, 38, detalleTxt);

  // Contador contra el limite duro de 48 h. Va SIEMPRE, activo el modo o no:
  // es el dato que decide si se puede entrar, y esconderlo hasta que el modo
  // arranca obligaria a intentarlo para averiguarlo.
  textoAntiguedad(huboSync, msDesdeSync, syncVencida, ant, sizeof(ant));
  snprintf(buf, sizeof(buf), "Sin sync: %s", ant);
  u8g2.drawStr(4, 49, buf);

  // El aviso de proximidad al limite se recuadra en lugar de escribirse: la
  // pantalla se lee de madrugada, con lluvia y a un metro. Una palabra mas en la
  // misma tipografia no se ve; un recuadro si.
  if (avisoLimite) u8g2.drawFrame(0, 40, 128, 12);

  if (pie != NULL) {
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(2, 62, pie);
  }
  u8g2.sendBuffer();
}

void lcd_dibujarRechazoDegradado(const char* motivo) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(2, 14, "RECHAZADO");
  u8g2.drawHLine(0, 18, 128);

  u8g2.setFont(u8g2_font_6x10_tr);
  if (motivo != NULL) u8g2.drawStr(2, 34, motivo);

  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawStr(2, 48, "El Degradado NO entro.");
  u8g2.drawStr(2, 58, "Todo sigue igual. 4=Menu");
  u8g2.sendBuffer();
}
