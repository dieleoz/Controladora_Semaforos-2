// ===== src/main.cpp (MAESTRO) =====
#include <Arduino.h>
#include "pines.h"
#include "botones.h"
#include "coordinador.h"
#include "lcd.h"
#include "menu.h"
#include "modo_manual.h"
#include "modo_automatico.h"
#include "modo_demanda.h"
#include "semaforo.h"

static ModoSistema modoAnterior;

void setup() {
  botones_setup();
  coordinador_setup(); // incluye semaforo_setup() y protocolo_setup()
  lcd_setup();

  lcd_dibujarBienvenida();
  delay(2000);

  // SFTY-1: 5s fijos en Rojo al arrancar, dando tiempo a enlazar con el esclavo.
  semaforo_forzarRojo();
  unsigned long tArranque = millis();
  while (millis() - tArranque < 5000) {
    coordinador_actualizar(); // envía PING / procesa PONG en segundo plano
    delay(10);
  }

  // Tras los 5s: si no hubo respuesta del esclavo, arrancamos en fallo (parpadeo Ámbar).
  // Si sí la hay, pasamos a Rojo sostenido: enlazado, esperando que se elija
  // una configuración (Manual / Automático / Por demanda).
  if (!coordinador_comunicacionActiva()) {
    coordinador_forzarFallo();
  } else {
    semaforo_iniciarEspera();
  }

  // No bloqueamos el acceso al menú aunque no haya comunicación.
  modoActual_set(MENU);
  menu_setup();
  modoAnterior = MENU;
}

void loop() {
  // coordinador_actualizar() ya maneja todo el ciclo de vida de la comunicación:
  // sigue enviando PING, detecta la pérdida (parpadeo Ámbar vía semaforo_iniciarFallo())
  // y detecta la recuperación (semaforo_forzarRojo()) sin intervención externa.
  // OJO: NO llamar aquí a coordinador_reiniciarConexion() en cada tick de fallo:
  // resetea el estado a C_IDLE antes de que el coordinador pueda procesar la
  // reconexión real, y el semáforo se queda parpadeando para siempre aunque
  // la comunicación vuelva. No bloqueamos la UI: el usuario puede seguir
  // navegando el menú aunque no haya comunicación.
  coordinador_actualizar();

  // ---- SISTEMA ----
  ModoSistema modo = modoActual_get();

  if (modo != modoAnterior) {
    switch (modo) {
      case MODO_MANUAL:      modoManual_setup();      break;
      case MODO_AUTOMATICO:  modoAutomatico_setup();  break;
      case MODO_DEMANDA:     modoDemanda_setup();     break;
      case MENU:             menu_setup();            break;
    }
    modoAnterior = modo;
  }

  switch (modo) {
    case MENU:            menu_loop();            break;
    case MODO_MANUAL:     modoManual_loop();      break;
    case MODO_AUTOMATICO: modoAutomatico_loop();  break;
    case MODO_DEMANDA:    modoDemanda_loop();     break;
  }
}