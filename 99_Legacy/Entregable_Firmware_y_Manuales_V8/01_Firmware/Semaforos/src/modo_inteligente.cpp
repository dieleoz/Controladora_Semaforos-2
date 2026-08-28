// ===== src/modo_inteligente.cpp =====
#include "modo_inteligente.h"
#include "botones.h"
#include "semaforo.h"
#include "coordinador.h"
#include "lcd.h"
#include "menu.h"
#include "protocolo.h"
#include <string.h>

enum FaseInt { CONFIG_MAX_VERDE, INT_ESTATICO, INT_CORRIENDO };
static FaseInt faseI;
static int maxVerde = 2, segEstatico = 15;
static unsigned long tEstadoDesde = 0;
static bool primeraVezCorriendo = true;

void modoInteligente_setup() {
  faseI = CONFIG_MAX_VERDE;
  maxVerde = 2; segEstatico = 15;
  primeraVezCorriendo = true;
  lcd_dibujarConfigValor("Max. VERDE (AI)", maxVerde, "min");
}

void modoInteligente_loop() {
  if (botonCancelar()) {
    modoActual_set(MENU);
    menu_setup();
    return;
  }

  switch (faseI) {
    case CONFIG_MAX_VERDE: {
      bool r = false;
      if (botonArriba()) { maxVerde++; if (maxVerde > 99) maxVerde = 99; r = true; }
      if (botonAbajo())  { maxVerde--; if (maxVerde < 1) maxVerde = 1; r = true; }
      if (botonAceptar()) {
        faseI = INT_ESTATICO;
        lcd_dibujarConfigValor("Tiem. Despeje All-Red", segEstatico, "seg");
        return;
      }
      if (r) lcd_dibujarConfigValor("Max. VERDE (AI)", maxVerde, "min");
      break;
    }

    case INT_ESTATICO: {
      bool r = false;
      if (botonArriba()) { segEstatico += 5; if (segEstatico > 999) segEstatico = 999; r = true; }
      if (botonAbajo())  { segEstatico -= 5; if (segEstatico < 0) segEstatico = 0; r = true; }
      if (botonAceptar()) {
        coordinador_configurar((unsigned long)segEstatico * 1000UL,
                                (unsigned long)maxVerde * 60000UL, // Tiempo base si la IA falla
                                (unsigned long)maxVerde * 60000UL);
        tEstadoDesde = millis();
        primeraVezCorriendo = true;
        faseI = INT_CORRIENDO;
        lcd_dibujarAutomatico(coordinador_nombreEstadoMaster(), maxVerde, maxVerde);
        return;
      }
      if (r) lcd_dibujarConfigValor("Tiem. Despeje All-Red", segEstatico, "seg");
      break;
    }

    case INT_CORRIENDO: {
      coordinador_actualizar();
      protocolo_actualizarAI(); // Leer datos desde Raspberry Pi (YOLO)

      if (coordinador_listoParaContar()) {
        if (primeraVezCorriendo) {
          tEstadoDesde = millis();
          primeraVezCorriendo = false;
        }

        unsigned long duracionMaxima = (unsigned long)maxVerde * 60000UL;
        unsigned long tiempoActual = millis() - tEstadoDesde;
        
        bool forzarCambio = false;

        // ==========================================
        // LÓGICA DE INTELIGENCIA ARTIFICIAL Y FALLBACK
        // ==========================================
        unsigned long tUltimoMsjAI = protocolo_obtenerUltimoTiempoAI();
        int autosEsperando = protocolo_obtenerAutosEsperandoAI();

        // FALLBACK: Si no hay datos de la IA en más de 60 segundos (Fallo de cámara o USB)
        if (millis() - tUltimoMsjAI > 60000UL) {
            // Se comporta como MODO_AUTOMATICO normal (estático)
            if (tiempoActual >= duracionMaxima) {
                forzarCambio = true;
            }
        } else {
            // MODO INTELIGENTE ACTIVO
            // Si el semáforo está en ROJO, evalúa si debe pedir el paso.
            // Si está en VERDE, evalúa si debe cortar el verde temprano.
            
            // Regla 1: Tiempo mínimo de verde/rojo para no marear a los conductores (ej: 15 seg min)
            if (tiempoActual > 15000UL) {
                if (semaforo_estado() == S_ROJO && autosEsperando > 0) {
                    // Si estoy en rojo y tengo autos, reduzco la tolerancia a la mitad
                    if (tiempoActual >= (duracionMaxima / 2)) {
                        forzarCambio = true;
                    }
                } else if (semaforo_estado() == S_VERDE && autosEsperando == 0) {
                    // Si estoy en verde, pero ya no me quedan autos, cedo el paso rápido
                    if (tiempoActual >= 20000UL) {
                         forzarCambio = true;
                    }
                }
            }
            
            // Regla 2: Límite máximo (nunca exceder el Max Verde aunque no haya autos esperando del otro lado)
            if (tiempoActual >= duracionMaxima) {
                forzarCambio = true;
            }
        }

        if (forzarCambio) {
          coordinador_pedirCambio();
          tEstadoDesde = millis();
        }
      } else {
        primeraVezCorriendo = true;
      }

      static const char* estadoAnt = "";
      const char* actual = coordinador_nombreEstadoMaster();
      if (strcmp(actual, estadoAnt) != 0) {
        lcd_dibujarAutomatico(actual, maxVerde, maxVerde);
        estadoAnt = actual;
      }
      break;
    }
  }
}