// ===== src/modo_manual.cpp =====
#include "modo_manual.h"
#include "botones.h"
#include "semaforo.h"
#include "coordinador.h"
#include "lcd.h"
#include "menu.h"
#include <string.h>

enum FaseManual { CONFIG_ESTATICO, CORRIENDO };
static FaseManual fase;
static int segEstatico = 3;

void modoManual_setup() {
  fase = CONFIG_ESTATICO;
  segEstatico = 5;
  lcd_dibujarConfigValor("Tiempo Rojo Estatico", segEstatico, "seg");
}

void modoManual_loop() {
  if (botonCancelar()) {
    modoActual_set(MENU);
    menu_setup();
    return;
  }

  if (fase == CONFIG_ESTATICO) {
    bool redibujar = false;
    if (botonArriba()) { segEstatico++; if (segEstatico > 99) segEstatico = 99; redibujar = true; }
    if (botonAbajo())  { segEstatico--; if (segEstatico < 5) segEstatico = 5; redibujar = true; } // FIX H-2: Piso mínimo 5s despeje
    if (botonAceptar()) {
      coordinador_configurar((unsigned long)segEstatico * 1000UL, 0, 0);
      coordinador_iniciarModo();
      fase = CORRIENDO;
      lcd_dibujarManual(semaforo_nombreEstado());
      return;
    }
    if (redibujar) lcd_dibujarConfigValor("Tiempo Rojo Estatico", segEstatico, "seg");
    return;
  }

  static const char* estadoAnt = "";
  // Botón 1 (Arriba) o Botón 2 (Abajo): Conmutar carril respetando tiempo de despeje All-Red
  if (botonArriba() || botonAbajo()) {
    coordinador_pedirCambio();
  }
  // SFTY-12: Botón 3 (OK/Aceptar) en Modo Manual fuerza ROJO FIJO CONTINUO en Maestro y Esclavo de forma INDEFINIDA
  if (botonAceptar()) {
    coordinador_forzarRojoTotal();
  }
  coordinador_actualizar();

  const char* actual = coordinador_nombreEstadoMaster();
  if (strcmp(actual, estadoAnt) != 0) {
    lcd_dibujarManual(actual);
    estadoAnt = actual;
  }
}