// ===== src/modod_demanda.cpp =====
#include "modo_demanda.h"
#include "botones.h"
#include "semaforo.h"
#include "coordinador.h"
#include "lcd.h"
#include "menu.h"
#include <string.h>

// Modo Por Demanda: EXACTAMENTE la misma lógica de alternancia maestro/esclavo
// que Modo Automático (coordinador_pedirCambio(), despeje All-Red, espera de
// ACK, etc.) -- lo único que cambia es que aquí no hay pantalla de
// configuración: los tiempos son fijos.
static const unsigned long DEMANDA_DESPEJE_MS = 15000UL; // 15s tiempo de seguridad (All-Red)
static const unsigned long DEMANDA_ROJO_MS    = 90000UL; // 90s (1 min 30s) en Rojo
static const unsigned long DEMANDA_VERDE_MS   = 60000UL; // 60s (1 min) en Verde
// El Ámbar de 2s entre Rojo y Verde ya lo maneja semaforo_iniciarTransicionAVerde()
// (el mismo mecanismo que usan Automático y Manual) -- no es un parámetro aquí.

static unsigned long tEstadoDesde = 0;
static bool primeraVezCorriendo = true;

void modoDemanda_setup() {
  coordinador_configurar(DEMANDA_DESPEJE_MS, DEMANDA_ROJO_MS, DEMANDA_VERDE_MS);
  semaforo_forzarRojo(); // SFTY-1: arranque limpio en Rojo, igual que Automático/Manual
  tEstadoDesde = millis();
  primeraVezCorriendo = true;
  lcd_dibujarDemanda(coordinador_nombreEstadoMaster());
}

void modoDemanda_loop() {
  if (botonCancelar()) {
    modoActual_set(MENU);
    menu_setup();
    return;
  }

  coordinador_actualizar();

  if (coordinador_listoParaContar()) {
    if (primeraVezCorriendo) {
      tEstadoDesde = millis();
      primeraVezCorriendo = false;
    }

    unsigned long duracion = (semaforo_estado() == S_ROJO) ? DEMANDA_ROJO_MS : DEMANDA_VERDE_MS;

    if (millis() - tEstadoDesde >= duracion) {
      coordinador_pedirCambio();
      tEstadoDesde = millis();
    }
  } else {
    primeraVezCorriendo = true;
  }

  static const char* estadoAnt = "";
  const char* actual = coordinador_nombreEstadoMaster();
  if (strcmp(actual, estadoAnt) != 0) {
    lcd_dibujarDemanda(actual);
    estadoAnt = actual;
  }
}
