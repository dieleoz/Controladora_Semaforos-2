// ===== src/lcd.cpp =====
#include "lcd.h"
#include "pines.h"
#include <U8g2lib.h>
#include <stdio.h>
#include <string.h>

#include "coordinador.h"

#ifdef LCD_VALIDACION_NATIVA
// Compilacion para PC (carpeta Validacion_LCD): el arnes provee un U8G2 con
// framebuffer para poder volcar las pantallas y comprobar que todo cabe en los
// 128x64 px. El codigo de dibujo de abajo es EXACTAMENTE el mismo que va al
// firmware, de modo que la validacion no puede desviarse de lo que se compila.
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
static U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, LCD_SCLK, LCD_SID, LCD_CS, U8X8_PIN_NONE);
#endif

// Abrevia un contador para que no desborde la fila: 999 -> "999", 12345 -> "12k".
// Sin esto, un contador de 6 cifras hacia que la linea de diagnostico invadiera
// el "4=Menu" de la derecha (lo detecto el arnes de validacion).
static void abreviarCuenta(unsigned long n, char *destino, size_t tam) {
  if (n < 1000UL)            snprintf(destino, tam, "%lu", n);
  else if (n < 1000000UL)    snprintf(destino, tam, "%luk", n / 1000UL);
  else                       snprintf(destino, tam, "%luM", n / 1000000UL);
}

// Linea inferior comun a los modos de operacion: calidad de enlace + atajo de menu.
// Se alimenta de la telemetria del latido de 3 s; no consulta a la radio.
static void dibujarLineaEnlace() {
  char buf[24];
  int calidad = coordinador_calidadEnlace();
  u8g2.setFont(u8g2_font_5x7_tr);
  if (calidad < 0) {
    snprintf(buf, sizeof(buf), "RF:---");
  } else if (calidad == 0) {
    snprintf(buf, sizeof(buf), "RF:SIN ENLACE");
  } else {
    snprintf(buf, sizeof(buf), "RF:%d%% %lums", calidad, coordinador_tiempoRespuestaMs());
  }
  u8g2.drawStr(2, 63, buf);
  u8g2.drawStr(96, 63, "4=Menu");
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
  u8g2.drawStr(5, 36, "Controlador Pro");
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(30, 54, "IT Vial SAS");
  u8g2.sendBuffer();
}

// ---------------------------------------------------------------------------
// V8.7 (SFTY-21): el dibujo del menu NO cambia. Lo que cambia es que ahora hay dos
// menus que lo usan -el principal y el submenu CONFIGURACION-, cada uno con su
// propio array y su propia cuenta.
//
// La alternativa que se descarto era una sexta opcion en una lista plana, y no
// cabia: 24 + 5*9 = 69, fuera de los 64 px de alto. El peligro no era que la sexta
// no se dibujase -la salvaguarda de abajo lo impide- sino que el cursor SI podia
// navegar hasta ella, dejando al operario en una opcion invisible. Partir el menu
// en dos niveles devuelve el principal a 4 opciones, que es EXACTAMENTE el layout
// validado en campo y en el arnes, y deja el submenu en 3, que usa ese mismo layout.
// Ninguno de los dos se acerca al limite.
//
// El titulo es opcional para no tocar la firma que ya usan todas las llamadas
// existentes: sin titulo se pinta el de siempre.
// ---------------------------------------------------------------------------
void lcd_dibujarMenu(int cursor, const char* opciones[], int cantidad, const char* titulo) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14_tr);
  if (titulo == 0) {
    u8g2.drawStr(30, 12, "MODO SEMAFORO");
  } else {
    // Centrado sobre el ancho real del texto. La fuente 7x14 es de paso fijo, asi
    // que la cuenta es exacta y ningun titulo puede salirse por el lado derecho.
    int ancho = (int)strlen(titulo) * 7;
    int x = (128 - ancho) / 2;
    if (x < 0) x = 0;
    u8g2.drawStr(x, 12, titulo);
  }
  u8g2.drawHLine(0, 16, 128);

  // La pantalla mide 64 px de alto y solo quedan 47 px utiles bajo la linea (y=16).
  // El interlineado se ajusta a la cantidad de opciones:
  //
  //   hasta 4 opciones -> base 28, paso 11  ->  y = 28, 39, 50, 61
  //   5 opciones       -> base 24, paso  9  ->  y = 24, 33, 42, 51, 60
  //
  // El caso de 4 se deja EXACTAMENTE como estaba: es el layout validado en campo
  // y en el arnes (30/30). Solo se compacta cuando de verdad hace falta.
  const int base = (cantidad <= 4) ? 28 : 24;
  const int paso = (cantidad <= 4) ? 11 : 9;

  u8g2.setFont(u8g2_font_6x10_tr);
  for (int i = 0; i < cantidad; i++) {
    int y = base + i * paso;
    if (y > 63) break; // salvaguarda: nunca dibujar fuera de pantalla
    if (i == cursor) {
      u8g2.drawStr(2, y, ">");
    }
    u8g2.drawStr(12, y, opciones[i]);
  }
  u8g2.sendBuffer();
}

void lcd_dibujarAjusteHora(uint8_t hora, uint8_t minuto, uint8_t digito, bool enHora,
                           bool hayCristal) {
  char buf[8];

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14_tr);
  u8g2.drawStr(14, 12, "AJUSTAR HORA");
  u8g2.drawHLine(0, 16, 128);

  // "HH:MM" con la fuente de ancho fijo 7x14, centrado: 5 caracteres = 35 px.
  // Al ser de paso fijo, la posicion de cada digito es calculable y el subrayado
  // cae siempre donde debe.
  const int X0 = 46;      // x del primer digito
  const int ANCHO = 7;    // paso de la fuente
  const int BASE = 40;    // linea base del texto

  snprintf(buf, sizeof(buf), "%02u:%02u", (unsigned)hora, (unsigned)minuto);
  u8g2.drawStr(X0, BASE, buf);

  // Subrayado bajo el digito activo. Los indices 0,1,3,4 de la cadena son los
  // digitos; el 2 es el ':' y se salta.
  const int posCadena[4] = {0, 1, 3, 4};
  int x = X0 + posCadena[digito & 3] * ANCHO;
  u8g2.drawHLine(x, BASE + 3, ANCHO - 1);

  u8g2.setFont(u8g2_font_5x7_tr);
  // N-24: se distingue "no puesto" de "no hay con que contar". Sin esto, ajustar la
  // hora parecia funcionar y al apagar y encender volvia a ceros, sin nada en
  // pantalla que explicara por que. Uno se arregla desde aqui; el otro NO, y mandar
  // al operario a teclear la hora una y otra vez contra un RTC parado es peor que no
  // decirle nada. Con la 5x7 caben 25 caracteres (25*5=125 + margen 2 = 127).
  if (!hayCristal) {
    u8g2.drawStr(2, 52, "SIN CRISTAL: Y2, PILA, R5");
  } else if (!enHora) {
    // Aviso deliberadamente visible: mientras el reloj no este puesto, ninguna
    // funcion que dependa de la hora debe activarse (ver SFTY-18).
    u8g2.drawStr(2, 52, "RELOJ SIN PONER EN HORA");
  }
  u8g2.drawStr(2, 62, "1=+ 2=- 3=sig 4=salir");
  u8g2.sendBuffer();
}

void lcd_dibujarManual(const char* nombreEstado) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14_tr);
  u8g2.drawStr(10, 14, "MODO: MANUAL");
  u8g2.drawHLine(0, 18, 128);
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.drawStr(10, 40, nombreEstado);
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawStr(2, 52, "1/2=Cambiar 3=Rojo");
  dibujarLineaEnlace();
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
  dibujarLineaEnlace();

  u8g2.sendBuffer();
}

void lcd_dibujarInteligente(const char* nombreEstado, int autosEsperando, bool iaActiva) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14_tr);
  u8g2.drawStr(4, 14, "MODO: INTELIGENTE AI");
  u8g2.drawHLine(0, 18, 128);

  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.drawStr(10, 38, nombreEstado);

  char buf[28];
  if (iaActiva) {
    snprintf(buf, sizeof(buf), "IA: OK (Autos: %d)", autosEsperando);
  } else {
    snprintf(buf, sizeof(buf), "IA: Standby (Fallback)");
  }
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(6, 52, buf);
  dibujarLineaEnlace();

  u8g2.sendBuffer();
}

// Pantalla de PRUEBA DE ALCANCE (V8.1).
// Pensada para que el operario camine con el equipo y vea degradarse el enlace
// hasta el punto en que se pierde, en lugar de estimarlo a ojo.
void lcd_dibujarAlcance(int calidadPct, unsigned long rttMs, int latidosPerdidos,
                        unsigned long bytes, unsigned long validas) {
  char buf[28];
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_7x14_tr);
  u8g2.drawStr(8, 12, "PRUEBA ALCANCE");
  u8g2.drawHLine(0, 15, 128);

  if (calidadPct < 0) {
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(14, 32, "Midiendo enlace...");
    u8g2.drawStr(20, 44, "espere 3 seg");
  } else if (calidadPct == 0) {
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.drawStr(16, 32, "SIN ENLACE");
    u8g2.setFont(u8g2_font_6x10_tr);
    snprintf(buf, sizeof(buf), "Fallos seguidos: %d", latidosPerdidos);
    u8g2.drawStr(4, 46, buf);
  } else {
    // Porcentaje grande a la izquierda, tiempo de respuesta a la derecha
    u8g2.setFont(u8g2_font_ncenB12_tr);
    snprintf(buf, sizeof(buf), "%d%%", calidadPct);
    u8g2.drawStr(4, 30, buf);

    u8g2.setFont(u8g2_font_6x10_tr);
    snprintf(buf, sizeof(buf), "%lums", rttMs);
    u8g2.drawStr(72, 30, buf);

    // Barra de 10 segmentos proporcional a la calidad
    int segmentos = (calidadPct + 9) / 10; // 1..10
    for (int i = 0; i < 10; i++) {
      if (i < segmentos) {
        u8g2.drawBox(4 + i * 12, 34, 10, 7);
      } else {
        u8g2.drawFrame(4 + i * 12, 34, 10, 7);
      }
    }

    u8g2.setFont(u8g2_font_6x10_tr);
    snprintf(buf, sizeof(buf), "Fallos: %d", latidosPerdidos);
    u8g2.drawStr(4, 52, buf);
  }

  // SFTY-15: diagnostico de linea. Lo que se lee aqui separa tres fallos que
  // antes se veian todos como "no hay comunicacion":
  //   RX 0b            -> no llega nada    (cobertura, canal, antena)
  //   RX 4512b 0tr     -> llega basura     (cableado, linea flotando, radio atascada)
  //   RX 36b 9tr       -> enlace correcto
  u8g2.setFont(u8g2_font_5x7_tr);
  {
    char nb[8], nv[8];
    abreviarCuenta(bytes, nb, sizeof(nb));
    abreviarCuenta(validas, nv, sizeof(nv));
    // El texto se mantiene por debajo de x=90 para no invadir el "4=Menu" de la
    // derecha, incluso con contadores de 6 cifras. El arnes lo comprueba.
    if (bytes == 0) {
      u8g2.drawStr(2, 62, "RX 0 - nada llega");
    } else if (validas == 0) {
      snprintf(buf, sizeof(buf), "RX %s - BASURA", nb);
      u8g2.drawStr(2, 62, buf);
    } else {
      snprintf(buf, sizeof(buf), "RX %s  %s tr", nb, nv);   // tr = tramas validas
      u8g2.drawStr(2, 62, buf);
    }
  }
  u8g2.drawStr(96, 62, "4=Menu");
  u8g2.sendBuffer();
}

// ---------------------------------------------------------------------------
// MODO DEGRADADO (V8.7, SFTY-21)
//
// Es la unica pantalla del equipo que acompana a un VERDE dado sin confirmacion del
// otro extremo, asi que tiene que decir en todo momento las cuatro cosas que le
// permiten a un tecnico decidir si sigue confiando en el:
//
//   1. En que fase esta        -> que luz toca ahora
//   2. Cuanto le queda         -> si el ciclo corre o se ha quedado congelado
//   3. Cuanto lleva sin sincronizar -> de cuanta deriva estamos hablando
//   4. Si el limite de 48 h esta cerca
//
// El punto 3 es el que de verdad importa: sin radio, las horas sin sincronizar son
// la unica medida de cuanto se pueden haber separado los dos relojes.
// ---------------------------------------------------------------------------
void lcd_dibujarDegradado(const char* fase, const char* detalle,
                          unsigned long restanteSeg, unsigned long minutosSinSync,
                          bool syncVencida, const char* aviso) {
  char buf[24];

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14_tr);
  u8g2.drawStr(4, 12, "MODO DEGRADADO");
  u8g2.drawHLine(0, 15, 128);

  // La fase, en grande: es lo que se compara de un vistazo con la luz de la calle.
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.drawStr(4, 32, fase);

  // Cuenta atras al siguiente cambio, a la derecha. Sale de ciclo_degradado_restante(),
  // no de un contador propio: un segundero paralelo acabaria discrepando del calculo
  // que de verdad manda sobre las luces.
  u8g2.setFont(u8g2_font_6x10_tr);
  snprintf(buf, sizeof(buf), "%lus", restanteSeg);
  u8g2.drawStr(88, 32, buf);

  u8g2.drawStr(4, 44, detalle);

  u8g2.setFont(u8g2_font_5x7_tr);
  if (syncVencida) {
    snprintf(buf, sizeof(buf), "Sin sync: >48h");
  } else {
    snprintf(buf, sizeof(buf), "Sin sync: %luh%02lum",
             minutosSinSync / 60UL, minutosSinSync % 60UL);
  }
  u8g2.drawStr(2, 54, buf);

  // El aviso ocupa el hueco de la izquierda de la fila inferior; el "4=Menu" de la
  // derecha se respeta siempre, porque salir tiene que poder hacerse aunque la
  // pantalla este dando una alarma.
  if (aviso != 0) u8g2.drawStr(2, 63, aviso);
  u8g2.drawStr(96, 63, "4=Menu");
  u8g2.sendBuffer();
}

// Entrada rechazada. Dice CUAL condicion falta, no un "no se puede" generico: el
// tecnico esta a 5 m de altura con el equipo abierto y necesita saber si tiene que
// poner el reloj, esperar a que sincronice o revisar el radio.
void lcd_dibujarSyncHora(bool esperando, bool ok, bool sinReloj) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14_tr);
  u8g2.drawStr(4, 12, "AJUSTAR HORA");
  u8g2.drawHLine(0, 15, 128);

  // N-38: LOS TITULOS VAN EN 7x14B, NO EN ncenB10, Y ESO NO ES ESTETICA.
  //
  // El arnes lo midio el 01/08/2026 con getStrWidth() y desmintio el recuento a mano
  // que traia esta funcion: "SINCRONIZADA" terminaba en x=129 y "SOLO MAESTRO" en
  // x=131. U8g2 recorta en silencio lo que cae fuera de los 128, asi que el texto
  // simplemente aparecia mordido sin que nada diera error.
  //
  // El error de razonamiento esta abajo, y conviene dejarlo escrito: se contaron
  // caracteres por 6 px, que es correcto para la 6x10 -de paso fijo- y NO DICE NADA
  // de la ncenB10, que es PROPORCIONAL. Las nueve lineas de 6x10 estaban todas bien;
  // fallaban justo los titulos, que eran los unicos que no se podian contar.
  //
  // 7x14B es de paso fijo: 12 caracteres por 7 px son 84, con sitio de sobra. Vuelve
  // a hacer el ancho calculable, que es lo que permitia contar en primer lugar.
  //
  // ANCHOS COMPROBADOS A MANO, no por el arnes: la maquina de hoy no tiene gcc de
  // host y compilar.ps1 no pudo correr. Con la 6x10 caben 21 caracteres (21*6=126 mas
  // el margen de 2 = 128 justo), asi que ninguna linea de abajo pasa de 20. El primer
  // borrador llevaba "El esclavo no contesto", 22 caracteres, y se salia de la
  // pantalla; es exactamente el tipo de fallo que el arnes detecta y que aqui hubo
  // que cazar contando. Cuando se pueda correr el arnes, esta pantalla es candidata a
  // entrar en el.
  if (sinReloj) {
    // N-30: no se llego a enviar NADA, asi que no se puede culpar al Esclavo.
    // coordinador_sincronizarHora() se niega si el reloj propio no esta en hora, y
    // con el cristal parado ese es el caso. Decir "Esclavo no responde" mandaria al
    // tecnico a revisar la otra punta, la radio y las antenas por un fallo que esta
    // en la tarjeta que tiene delante.
    u8g2.setFont(u8g2_font_7x14B_tr);
    u8g2.drawStr(4, 32, "SIN RELOJ");
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(2, 46, "No se envio nada");      // 16
    // N-45: aqui ponia "Revisa Y2, pila y R5". El firmware NUNCA midio la pila -el
    // F103 no tiene canal de ADC para VBAT- ni el cristal: era una conclusion fija
    // disfrazada de diagnostico, y mando a cambiar los tres con el hardware sano.
    u8g2.drawStr(2, 58, "Mira CONSULTA RELOJ");   // 19
  } else if (esperando) {
    u8g2.setFont(u8g2_font_7x14B_tr);
    u8g2.drawStr(4, 34, "ENVIANDO...");
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(2, 50, "Esperando al esclavo");   // 20
  } else if (ok) {
    u8g2.setFont(u8g2_font_7x14B_tr);
    u8g2.drawStr(4, 34, "SINCRONIZADA");
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(2, 50, "El esclavo la aplico");   // 20
  } else {
    // No se dice "error" a secas: se dice QUE quedo hecho y QUE no. El reloj del
    // Maestro SI quedo puesto, y ocultarlo llevaria a repetir el ajuste una y otra
    // vez creyendo que no se guarda nada. Lo que falta es la otra punta.
    u8g2.setFont(u8g2_font_7x14B_tr);
    u8g2.drawStr(4, 32, "SOLO MAESTRO");
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(2, 46, "Esclavo no responde");    // 19
    u8g2.drawStr(2, 58, "No habra Degradado");     // 18
  }
  u8g2.sendBuffer();
}

// N-45 — CONSULTA DEL RELOJ.
//
// ANCHOS: fuente 6x10 de paso fijo, 21 caracteres justos (21*6 = 126 + 2 de margen).
// Ninguna linea de aqui pasa de 20. La ultima cae en y=60, dentro de los 63.
//
// NO SE ACUSA A NINGUN COMPONENTE, y es el motivo de existir de esta pantalla. La
// anterior decia "Revisa Y2, pila y R5" sin haber medido ni la pila ni el cristal, y
// mando a cambiar los tres con el hardware sano. La linea de abajo describe LOS BITS
// -"pedido, no oscila"-, no dictamina la pieza culpable.
void lcd_dibujarDiagnosticoReloj(const RelojDiag& d, bool latido) {
  char buf[24];

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14_tr);
  u8g2.drawStr(4, 12, "CONSULTA RELOJ");
  // El latido va a la derecha del titulo: "CONSULTA RELOJ" ocupa 4+14*7 = 102 px, y
  // el punto cae en 118, dentro de los 128. Si este punto parpadea y CNT no cambia,
  // el RTC esta parado de verdad; si no parpadea, lo parado es el firmware.
  if (latido) u8g2.drawDisc(118, 8, 3);
  u8g2.drawHLine(0, 15, 128);

  u8g2.setFont(u8g2_font_6x10_tr);

  snprintf(buf, sizeof(buf), "LSE ON:%u RDY:%u BYP:%u",
           (unsigned)d.lseOn, (unsigned)d.lseRdy, (unsigned)d.lseByp);
  u8g2.drawStr(2, 27, buf);                                       // 20

  snprintf(buf, sizeof(buf), "RTCSEL:%u EN:%u CFG:%u",
           (unsigned)d.rtcSel, (unsigned)d.rtcEn, (unsigned)d.configurado);
  u8g2.drawStr(2, 38, buf);                                       // 19

  if (d.cntLeido) {
    snprintf(buf, sizeof(buf), "CNT:%lu A:%u", (unsigned long)d.cnt,
             (unsigned)d.anio);
  } else {
    // Sin RTCEN no se leyo el periferico a proposito, y se dice: un "0" ahi seria
    // indistinguible de un contador parado en cero, que es un diagnostico distinto.
    snprintf(buf, sizeof(buf), "CNT:sin RTCEN A:%u", (unsigned)d.anio);
  }
  u8g2.drawStr(2, 49, buf);                                       // <= 19

  const char* pista;
  if (d.lseByp)        pista = "LSE en BYPASS ext.";   // 18
  else if (!d.lseOn)   pista = "LSE no se pide";       // 14
  else if (!d.lseRdy)  pista = "Pedido, no oscila";    // 17
  else if (d.rtcSel != 1) pista = "Oscila; RTC no atado";  // 20
  else                 pista = "Oscila y atado a LSE"; // 20
  u8g2.drawStr(2, 60, pista);

  u8g2.sendBuffer();
}

void lcd_dibujarReinicioReloj(bool arranco) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14_tr);
  u8g2.drawStr(4, 12, "REINICIAR RELOJ");
  u8g2.drawHLine(0, 15, 128);
  u8g2.setFont(u8g2_font_7x14B_tr);
  if (arranco) {
    u8g2.drawStr(4, 34, "CRISTAL OK");
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(2, 50, "Pon la hora de nuevo");     // 20
  } else {
    // El estado sucio NO era la causa. Lo que sigue lo dice la CONSULTA RELOJ, no
    // esta pantalla: N-45 quito de aqui "Es Y2: toca hardware" porque nombraba un
    // componente concreto sin haberlo medido, y con Y2 nuevo seguia diciendo lo
    // mismo. Descartar el estado no identifica la pieza.
    u8g2.drawStr(4, 32, "SIGUE PARADO");
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(2, 46, "No era el estado");       // 16
    u8g2.drawStr(2, 58, "Mira CONSULTA RELOJ");    // 19
  }
  u8g2.sendBuffer();
}

void lcd_dibujarDegradadoRechazo(const char* linea1, const char* linea2) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14_tr);
  u8g2.drawStr(4, 12, "DEGRADADO");
  u8g2.drawHLine(0, 15, 128);

  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.drawStr(4, 32, "RECHAZADO");

  u8g2.setFont(u8g2_font_6x10_tr);
  if (linea1 != 0) u8g2.drawStr(2, 46, linea1);
  if (linea2 != 0) u8g2.drawStr(2, 58, linea2);
  u8g2.sendBuffer();
}

// Caida a ambar intermitente: por el limite duro de 48 h o por peticion desde el
// mando (B.B.B). Se dice el motivo porque un ambar mudo obliga a subir a preguntarle
// al equipo que le pasa, que es justo lo que SFTY-22 senala como defecto.
void lcd_dibujarDegradadoAmbar(const char* linea1, const char* linea2) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14_tr);
  u8g2.drawStr(4, 12, "AMBAR INTERM.");
  u8g2.drawHLine(0, 15, 128);

  u8g2.setFont(u8g2_font_6x10_tr);
  if (linea1 != 0) u8g2.drawStr(2, 32, linea1);
  if (linea2 != 0) u8g2.drawStr(2, 44, linea2);

  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawStr(96, 63, "4=Menu");
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