// ===== src/menu.cpp =====
#include "menu.h"
#include "botones.h"
#include "lcd.h"
#include "semaforo.h"
#include "pines.h"

static ModoSistema modoActual = MENU;
static int cursorMenu = 0;
static const char* opcionesMenu[3] = {"MANUAL", "AUTOMATICO", "INTELIGENTE"};

static unsigned long tBlink = 0;
static bool amarilloOn = false;
const unsigned long INTERVALO_BLINK = 500;

void menu_setup() {
  modoActual = MENU;
  cursorMenu = 0;
  semaforo_apagarTodo();
  tBlink = millis();
  amarilloOn = false;
  lcd_dibujarMenu(cursorMenu, opcionesMenu, 3);
}

ModoSistema modoActual_get() { return modoActual; }
void modoActual_set(ModoSistema m) { modoActual = m; }

static void actualizarBlink() {
  if (millis() - tBlink >= INTERVALO_BLINK) {
    tBlink = millis();
    amarilloOn = !amarilloOn;
    digitalWrite(AMARILLO1, amarilloOn);
    digitalWrite(AMARILLO2, amarilloOn);
  }
}

void menu_loop() {
  actualizarBlink();

  bool redibujar = false;

  if (botonArriba()) {
    cursorMenu = (cursorMenu + 2) % 3;
    redibujar = true;
  }
  if (botonAbajo()) {
    cursorMenu = (cursorMenu + 1) % 3;
    redibujar = true;
  }
  if (botonAceptar()) {
    if (cursorMenu == 0) {
      modoActual = MODO_MANUAL;
    } else if (cursorMenu == 1) {
      modoActual = MODO_AUTOMATICO;
    } else {
      modoActual = MODO_INTELIGENTE;
    }
    return;
  }
  botonCancelar();

  if (redibujar) {
    lcd_dibujarMenu(cursorMenu, opcionesMenu, 3);
  }
}