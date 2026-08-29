// ===== src/modo_alcance.cpp =====
#include "modo_alcance.h"
#include "botones.h"
#include "coordinador.h"
#include "lcd.h"
#include "menu.h"
#include "modos.h"
#include "protocolo.h"

// Ultimos valores pintados, para no repetir transferencias a la pantalla.
static int  ultCalidad = -2;      // -2 = nada pintado todavia
static unsigned long ultRtt = 0;
static int  ultFallos = -1;
static unsigned long ultValidas = 0;
static unsigned long tUltimoRefresco = 0;

// Redibuja SOLO si cambio algo. La ST7920 va por SPI de 3 hilos por software y
// volcar el buffer de 1 KB bloquea el bucle unas decenas de ms; hacerlo cada
// 500 ms repetiria 6 veces la misma imagen, porque el dato de fondo solo se
// renueva con cada latido (3 s). Ademas, mientras se pinta no se atiende la radio.
static void refrescarSiCambio(bool forzar) {
  int calidad = coordinador_calidadEnlace();
  unsigned long rtt = coordinador_tiempoRespuestaMs();
  int fallos = coordinador_latidosSinRespuesta();
  unsigned long validas = protocolo_tramasValidas();

  // El contador de bytes se mueve constantemente; no se usa como disparador para
  // no provocar un redibujado por cada byte. Se pinta el valor del momento.
  bool cambio = forzar || calidad != ultCalidad || rtt != ultRtt ||
                fallos != ultFallos || validas != ultValidas;
  if (!cambio) return;

  ultCalidad = calidad; ultRtt = rtt; ultFallos = fallos; ultValidas = validas;
  tUltimoRefresco = millis();
  lcd_dibujarAlcance(calidad, rtt, fallos, protocolo_bytesRecibidos(), validas);
}

void modoAlcance_setup() {
  // Mismo estado seguro que el Menu Principal: Rojo Fijo en ambos extremos con
  // enlace, Amarillo Intermitente si se pierde. No arranca ningun ciclo.
  coordinador_forzarMenu();
  // SFTY-15: los contadores arrancan de cero al entrar, para que lo que se vea en
  // pantalla corresponda a ESTA medicion y no al acumulado desde el encendido.
  protocolo_reiniciarContadores();
  ultCalidad = -2; ultRtt = 0; ultFallos = -1; ultValidas = 0;
  tUltimoRefresco = 0;
  refrescarSiCambio(true);
}

void modoAlcance_loop() {
  if (botonCancelar()) {
    modoActual_set(MENU);
    menu_setup();
    return;
  }

  // Mantiene vivo el latido de 3 s, que es de donde sale la telemetria.
  coordinador_actualizar();

  // Redibujado por cambio de dato. Como salvaguarda, un refresco de cortesia
  // cada 5 s por si el enlace quedara completamente estatico: asegura que el
  // contador de bytes de la linea inferior se vea moverse aunque no cambie nada mas.
  refrescarSiCambio(millis() - tUltimoRefresco >= 5000);
}
