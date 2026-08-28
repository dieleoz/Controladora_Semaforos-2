// ===== src/modo_automatico.cpp =====
#include "modo_automatico.h"
#include "botones.h"
#include "semaforo.h"
#include "coordinador.h"
#include "lcd.h"
#include "menu.h"
#include <string.h>

enum FaseAuto { CONFIG_ROJO, CONFIG_VERDE, CONFIG_ESTATICO, CORRIENDO };
static FaseAuto fase;
static int minRojo = 1, minVerde = 1, segEstatico = 15;
static unsigned long tEstadoDesde = 0;
static bool primeraVezCorriendo = true;

void modoAutomatico_setup() {
  fase = CONFIG_ROJO;
  minRojo = 1; minVerde = 1; segEstatico = 15;
  primeraVezCorriendo = true;
  lcd_dibujarConfigValor("Minutos ROJO", minRojo, "min");
}

void modoAutomatico_loop() {
  if (botonCancelar()) {
    modoActual_set(MENU);
    menu_setup();
    return;
  }

  switch (fase) {
    case CONFIG_ROJO: {
      bool r = false;
      if (botonArriba()) { minRojo++; if (minRojo > 99) minRojo = 99; r = true; }
      if (botonAbajo())  { minRojo--; if (minRojo < 0) minRojo = 0; r = true; }
      if (botonAceptar()) {
        fase = CONFIG_VERDE;
        lcd_dibujarConfigValor("Minutos VERDE", minVerde, "min");
        return;
      }
      if (r) lcd_dibujarConfigValor("Minutos ROJO", minRojo, "min");
      break;
    }

    case CONFIG_VERDE: {
      bool r = false;
      if (botonArriba()) { minVerde++; if (minVerde > 99) minVerde = 99; r = true; }
      if (botonAbajo())  { minVerde--; if (minVerde < 0) minVerde = 0; r = true; }
      if (botonAceptar()) {
        fase = CONFIG_ESTATICO;
        lcd_dibujarConfigValor("Tiem. Despeje All-Red", segEstatico, "seg");
        return;
      }
      if (r) lcd_dibujarConfigValor("Minutos VERDE", minVerde, "min");
      break;
    }

    case CONFIG_ESTATICO: {
      bool r = false;
      // OPT-6 / SFTY-4: Se aumenta el límite de 99 a 999 segundos (casi 16 min).
      // Ver MANUAL_USUARIO.md. Requerido para obras extensas de hasta 500 metros
      // a baja velocidad (10 km/h) operando bajo antenas de 6km.
      if (botonArriba()) { segEstatico += 5; if (segEstatico > 999) segEstatico = 999; r = true; }
      if (botonAbajo())  { segEstatico -= 5; if (segEstatico < 0) segEstatico = 0; r = true; }
      if (botonAceptar()) {
        coordinador_configurar((unsigned long)segEstatico * 1000UL,
                                (unsigned long)minRojo * 60000UL,
                                (unsigned long)minVerde * 60000UL);
        semaforo_forzarRojo(); // SFTY-1: arranque limpio en Rojo al iniciar la configuración
        tEstadoDesde = millis();
        primeraVezCorriendo = true;
        fase = CORRIENDO;
        lcd_dibujarAutomatico(coordinador_nombreEstadoMaster(), minRojo, minVerde);
        return;
      }
      if (r) lcd_dibujarConfigValor("Tiem. Despeje All-Red", segEstatico, "seg");
      break;
    }

    case CORRIENDO: {
      coordinador_actualizar();

      if (coordinador_listoParaContar()) {
        if (primeraVezCorriendo) {
          tEstadoDesde = millis();
          primeraVezCorriendo = false;
        }

        unsigned long duracion = (semaforo_estado() == S_ROJO)
                                    ? (unsigned long)minRojo * 60000UL
                                    : (unsigned long)minVerde * 60000UL;

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
        lcd_dibujarAutomatico(actual, minRojo, minVerde);
        estadoAnt = actual;
      }
      break;
    }
  }
}