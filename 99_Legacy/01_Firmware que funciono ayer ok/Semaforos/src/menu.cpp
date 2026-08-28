// ===== src/menu.cpp =====
#include "menu.h"
#include "botones.h"
#include "lcd.h"
#include "semaforo.h"
#include "coordinador.h"
#include "pines.h"

static ModoSistema modoActual = MENU;
static int cursorMenu = 0;
static const char* opcionesMenu[3] = {"MANUAL", "AUTOMATICO", "POR DEMANDA"};

void menu_setup() {
  modoActual = MENU;
  cursorMenu = 0;
  // SFTY-1: al entrar al menú (arranque, o al cancelar Manual/Automático/
  // Demanda) ambos semáforos deben quedar en Rojo, sin importar en qué color
  // haya quedado cada uno, hasta que se elija una nueva configuración.
  // Si no hay enlace, dejamos el parpadeo Ámbar que ya esté manejando el
  // coordinador (semaforo_iniciarFallo()) intacto.
  if (!coordinador_comunicacionPerdida()) {
    coordinador_volverAmbosARojo();
    semaforo_iniciarEspera();
  }
  lcd_dibujarMenu(cursorMenu, opcionesMenu, 3);
}

ModoSistema modoActual_get() { return modoActual; }
void modoActual_set(ModoSistema m) { modoActual = m; }

void menu_loop() {
  // SFTY-1: mientras estemos en MENU (sin configuración activa), el semáforo
  // debe reflejar Rojo sostenido si hay enlace. La pérdida de enlace ya la
  // detecta y aplica el coordinador (parpadeo Ámbar) en cada tick, incluso
  // estando en este modo, así que aquí solo nos aseguramos de no quedarnos
  // en otro estado una vez que el enlace se recupera.
  if (!coordinador_comunicacionPerdida() && semaforo_estado() != S_ESPERA) {
    semaforo_iniciarEspera();
  }

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
      modoActual = MODO_DEMANDA;
    }
    return;
  }
  botonCancelar();

  if (redibujar) {
    lcd_dibujarMenu(cursorMenu, opcionesMenu, 3);
  }
}
