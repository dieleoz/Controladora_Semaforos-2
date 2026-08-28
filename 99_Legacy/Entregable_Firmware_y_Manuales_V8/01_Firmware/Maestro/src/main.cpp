// ===== src/main.cpp (MAESTRO) =====
#include <Arduino.h>
#include "pines.h"
#include "botones.h"
#include "coordinador.h"
#include "lcd.h"
#include "menu.h"
#include "modo_manual.h"
#include "modo_automatico.h"
#include "modo_inteligente.h"
#include "semaforo.h"
#include <IWatchdog.h> // SFTY-1: Watchdog Timer

enum EstadoGlobal { HANDSHAKE, SISTEMA };
static EstadoGlobal estadoGlobal = HANDSHAKE;

static unsigned long tBlink = 0;
static bool amarilloOn = false;

static ModoSistema modoAnterior;

static void iniciarParpadeoFallo() {
  digitalWrite(ROJO1, LOW); digitalWrite(ROJO2, LOW);
  digitalWrite(VERDE1, LOW); digitalWrite(VERDE2, LOW);
  tBlink = millis();
  amarilloOn = false;
}

void setup() {
  botones_setup();
  coordinador_setup();
  lcd_setup();

  // Inicializar UI
  lcd_dibujarBienvenida();
  delay(2000);

  // SFTY-1: Iniciar Watchdog Timer a 2 segundos (despues del delay inicial para evitar loop de reinicio)
  // IWatchdog.begin(2000000);

  // Eliminamos el bloqueo inicial por Handshake.
  // Arrancamos directamente en el menú.
  estadoGlobal = SISTEMA;
  modoActual_set(MENU);
  menu_setup();
  modoAnterior = MENU;
}

void loop() {
  // SFTY-1: Alimentar al perro guardián. Si el loop se traba por > 2s, la placa se reinicia.
  // IWatchdog.reload();

  if (estadoGlobal == SISTEMA && coordinador_comunicacionPerdida()) {
    // Si se pierde la comunicación, el coordinador forza el estado seguro
    // No bloqueamos la UI. El usuario puede seguir navegando el menú.
    coordinador_reiniciarConexion(); // Intenta reconectar en segundo plano
  }

  // Siempre actualizamos el estado del coordinador para mantener vivos los PINGs/PONGs
  // y las secuencias de semáforo (si no estamos en manual).
  // Nota: En modo_automatico.cpp ya se llama a coordinador_actualizar(), 
  // pero para que el handshake en background funcione, lo llamamos aquí si no estamos en auto.
  if (modoActual_get() != MODO_AUTOMATICO) {
     coordinador_actualizar_background(); 
  }

  ModoSistema modo = modoActual_get();

  if (modo != modoAnterior) {
    switch (modo) {
      case MODO_MANUAL:      modoManual_setup();      break;
      case MODO_AUTOMATICO:  modoAutomatico_setup();  break;
      case MODO_INTELIGENTE: modoInteligente_setup(); break;
      case MENU:             menu_setup();            break;
    }
    modoAnterior = modo;
  }

  switch (modo) {
    case MENU:            menu_loop();            break;
    case MODO_MANUAL:     modoManual_loop();      break;
    case MODO_AUTOMATICO: modoAutomatico_loop();  break;
    case MODO_INTELIGENTE:modoInteligente_loop(); break;
  }
}

