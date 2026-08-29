// ===== src/modo_inteligente.cpp =====
#include "modo_inteligente.h"
#include "pines.h"
#include "botones.h"
#include "semaforo.h"
#include "coordinador.h"
#include "demanda.h"
#include "lcd.h"
#include "menu.h"
#include "modos.h"
#include "protocolo.h"
#include <string.h>

enum FaseInt { INT_CORRIENDO };
static FaseInt faseI;
static int maxVerde = 2, segEstatico = 15;
static unsigned long tEstadoDesde = 0;
static bool primeraVezCorriendo = true;

// Filtro antirrebote para lectura digital de cámaras AcuSense (PB0 / PB8)
static bool leerPinCamara(uint8_t pin) {
  // N-67: activo en ALTO -ver el porque en modoInteligente_setup()-. El antirrebote
  // de 5 ms se mantiene: el RC de la placa filtra 1 ms, y el rele de la camara puede
  // rebotar mas que eso.
  if (digitalRead(pin) == HIGH) {
    delay(5);
    return (digitalRead(pin) == HIGH);
  }
  return false;
}

void modoInteligente_setup() {
  // N-67: LA POLARIDAD LA MANDA LA PLACA, Y LA PLACA DICE ACTIVO EN ALTO.
  //
  // Medido sobre el esquematico bueno: PB0 lleva R64 de 10 kOhm A MASA -pull-DOWN- y
  // C25 de 100 nF tambien a masa; la bornera J14 saca ese pin JUNTO A 3,3 V. O sea que
  // el contacto seco de la camara va entre los dos bornes de J14 y CIERRA A 3,3 V.
  //
  // Con INPUT_PULLUP esto no podia funcionar de ninguna manera: el pull-up interno
  // (~40 kOhm) contra los 10 kOhm externos deja el pin en 3,3 x 10/50 = 0,66 V, que es
  // LOW. El firmware habria visto DEMANDA PERMANENTE desde el arranque, sin camara
  // conectada; y al cerrar el contacto el pin sube a 3,3 V y lo habria leido como
  // "no hay demanda". Invertido y siempre activo a la vez.
  //
  // Se deja en INPUT a secas -sin pull-up-: el reposo lo fija el 10k de la placa.
  pinMode(CAM_DEMANDA_PIN, INPUT);
  // N-64: PB8 no es una entrada de camara, es el LED testigo D5 (R16 1K). Se deja en
  // alta impedancia: un pin flotante no puede encenderlo sea cual sea el sentido del
  // diodo, y con INPUT_PULLUP quedaria a medio encender por 40 uA de fuga.
  pinMode(LED_TESTIGO, INPUT);

  faseI = INT_CORRIENDO;
  maxVerde = 2; // 2 min max verde
  segEstatico = 15; // 15 seg all-red
  
  coordinador_configurar((unsigned long)segEstatico * 1000UL,
                          (unsigned long)maxVerde * 60000UL,
                          (unsigned long)maxVerde * 60000UL);
  coordinador_iniciarModo();
                          
  tEstadoDesde = millis();
  primeraVezCorriendo = true;
  lcd_dibujarInteligente(coordinador_nombreEstadoMaster(), 0, true);
}

void modoInteligente_loop() {
  if (botonCancelar()) {
    modoActual_set(MENU);
    menu_setup();
    return;
  }

  switch (faseI) {
    case INT_CORRIENDO: {
      coordinador_actualizar();

      if (coordinador_listoParaContar()) {
        if (primeraVezCorriendo) {
          tEstadoDesde = millis();
          primeraVezCorriendo = false;
        }

        unsigned long duracionMaxima = (unsigned long)maxVerde * 60000UL;
        unsigned long tiempoActual = millis() - tEstadoDesde;
        
        bool forzarCambio = false;

        // ==============================================================
        // LÓGICA DE PRESENCIA VEHICULAR POR CÁMARAS ACUSENSE
        // ==============================================================
        // Cámara 1 (Maestro): PB0 (Demanda Sentido 1)
        // Cámara 3 (Esclavo): PB0 en Esclavo -> Transmite CMD_DEMANDA al Maestro (Sentido 2)
        // La demanda pedida a mano por Bluetooth entra POR AQUI, en el mismo OR que la
        // camara, y no por un camino propio hasta el coordinador: asi se le aplican los
        // dos limites que gobiernan a la camara -el minimo de 15 s de verde y el tope
        // de verde maximo-. Un atajo se los saltaria, y ademas partiria el turno sin
        // que el ciclo se enterase.
        bool demandaLocalS1 = leerPinCamara(CAM_DEMANDA_PIN) || demanda_hayLocal();
        bool demandaRemotaS2 = coordinador_hayDemandaRemota();

        // Regla 1: Mínimo 15 segundos de verde antes de permitir cualquier alternancia
        if (tiempoActual >= 15000UL) {
          if (semaforo_estado() == S_ROJO) {
            // Maestro está en ROJO (Sentido 2 en Verde).
            // Si hay vehículo esperando en Sentido 1 (Cámara 1 activa), solicita cambio de turno
            if (demandaLocalS1) {
              forzarCambio = true;
            }
          } else if (semaforo_estado() == S_VERDE) {
            // Maestro está en VERDE (Sentido 1).
            // Si hay vehículo esperando en Sentido 2 (Cámara 3 activa) y no hay cola local, cede el paso
            if (demandaRemotaS2 && !demandaLocalS1) {
              forzarCambio = true;
              coordinador_limpiarDemandaRemota();
            }
          }
        }

        // Regla 2: Límite máximo de verde (Max Verde = 2 min) para evitar monopolio de carril
        if (tiempoActual >= duracionMaxima) {
          forzarCambio = true;
          coordinador_limpiarDemandaRemota();
        }

        if (forzarCambio) {
          coordinador_pedirCambio();
          tEstadoDesde = millis();
        }
      } else {
        primeraVezCorriendo = true;
      }

      static const char* estadoAnt = "";
      static int presenciaAnt = -1;
      const char* actual = coordinador_nombreEstadoMaster();
      int presenciaActual = (leerPinCamara(CAM_DEMANDA_PIN) ? 1 : 0) + (coordinador_hayDemandaRemota() ? 1 : 0);

      if (strcmp(actual, estadoAnt) != 0 || presenciaActual != presenciaAnt) {
        lcd_dibujarInteligente(actual, presenciaActual, true);
        estadoAnt = actual;
        presenciaAnt = presenciaActual;
      }
      break;
    }
  }
}