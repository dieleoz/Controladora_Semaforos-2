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
  segEstatico = 3;
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
    if (botonAbajo())  { segEstatico--; if (segEstatico < 0) segEstatico = 0; redibujar = true; }
    if (botonAceptar()) {
      coordinador_configurar((unsigned long)segEstatico * 1000UL, 0, 0);
      fase = CORRIENDO;
      lcd_dibujarManual(semaforo_nombreEstado());
      return;
    }
    if (redibujar) lcd_dibujarConfigValor("Tiempo Rojo Estatico", segEstatico, "seg");
    return;
  }

  static const char* estadoAnt = "";
  if (botonArriba() || botonAbajo()) {
    coordinador_pedirCambio();
  }
  coordinador_actualizar();

  const char* actual = coordinador_nombreEstadoMaster();
  if (strcmp(actual, estadoAnt) != 0) {
    lcd_dibujarManual(actual);
    estadoAnt = actual;
  }
}