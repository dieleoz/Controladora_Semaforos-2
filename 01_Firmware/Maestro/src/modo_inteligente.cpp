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
#include "limites_ciclo.h"   // N-137: el minimo vial, no un 2 escrito a mano

// N-135 OTRA VEZ, EN EL FICHERO DE AL LADO (05/09). Aqui habia:
//
//     enum FaseInt { INT_CORRIENDO };
//     static FaseInt faseI;      ... switch (faseI) { case INT_CORRIENDO: ... }
//
// Un enum de UN SOLO valor no es un estado: es una constante disfrazada, y el switch de
// abajo era un envoltorio que no podia elegir nada. Inerte hoy -no colgaba ninguna guarda
// de el, al contrario que en modo_automatico.cpp, donde costo SET_TIEMPOS entero- pero es
// la misma forma, y se retira antes de que alguien cuelgue una guarda.
//
// Y LO ENCONTRO UNA REVISION EXTERNA, NO EL PACK QUE EXISTE PARA ESTO: maestro_10 censa
// enums de un solo valor "que ademas se COMPARAN" y solo miraba `==`. Un `case` es una
// comparacion.
//
// 🔴 Y AQUI DECIA "el pack se afila en el mismo commit; si no, esto vuelve". NO SE AFILO,
// y la frase se quedo prometiendolo: es exactamente 2.ter -una frase que sostiene un verde
// y que no comprueba nadie-, escrita por quien hizo el cambio y en el mismo commit. La
// encontro una revision externa leyendo, no un test.
//
// Lo que paso, medido: se anadio `case` al detector y se INYECTO el defecto en este .cpp
// para comprobarlo. EL PACK SIGUIO DANDO 12/12. El censo aislado si lo encuentra -el enum
// se detecta y el `case` tambien-, asi que hay una tercera causa sin localizar. Un parche
// que no se ha visto fallar es un adorno que da verde (8.bis), asi que se revirtio.
//
// QUEDA ABIERTO: maestro_10 es el UNICO pack que censa esta forma, y hoy solo mira `==`.
// Un enum de un valor dentro de un switch volveria a entrar sin que nada lo delate.
// N-137 (04/09): AQUI PONIA `maxVerde = 2` MINUTOS, POR DEBAJO DEL MINIMO VIAL.
//
// Este modo configura el coordinador por su cuenta -no pasa por SET_TIEMPOS-, asi que
// la guarda de los 3 minutos no lo tocaba. Y era el modo que la guia de banco
// recomendaba como salida mientras el Automatico estuvo roto: el cruce habria corrido
// con verdes de 2 minutos justo donde el responsable dijo que el minimo son 3.
//
// Los limites salen ahora de limites_ciclo.h, que es el unico sitio donde viven.
static int maxVerde = VERDE_MIN_MIN, segEstatico = DESPEJE_SEG_MIN;
static unsigned long tEstadoDesde = 0;
static bool primeraVezCorriendo = true;

// N-97 (31/08): el lector antirrebotado de camara se mudo a botones.cpp, que es el dueno
// de J16 y ahora tambien de las dos camaras nuevas. No se copio: es LA MISMA funcion,
// camara_leerPin(), y la usan las dos puntas. Aqui se llama, no se redefine.

void modoInteligente_setup() {
  // N-97 (31/08): AQUI YA NO SE DECLARA LA CAMARA, Y ESE ERA EL DEFECTO.
  //
  // pinMode(CAM_DEMANDA_PIN, INPUT) vivia en esta funcion, o sea que el pin de la camara
  // solo estaba configurado mientras el equipo estuviera EN Modo Inteligente -mientras el
  // Esclavo lo declaraba en su setup(), siempre-. Un modo no es dueno de una entrada
  // fisica: la entrada existe desde que la tarjeta arranca, la mire quien la mire. Ahora
  // las dos puntas la declaran en el arranque; en esta, en botones_setup().
  //
  // La cuenta que fija la polaridad -activo en ALTO- sigue estando, entera, en pines.h.
  //
  // N-64: PB8 no es una entrada de camara, es el LED testigo D5 (R16 1K). Se deja en
  // alta impedancia: un pin flotante no puede encenderlo sea cual sea el sentido del
  // diodo, y con INPUT_PULLUP quedaria a medio encender por 40 uA de fuga.
  pinMode(LED_TESTIGO, INPUT);

  // Mismo motivo que el inicializador: reentrar en el modo no puede devolver el
  // cruce por debajo del minimo vial.
  maxVerde = VERDE_MIN_MIN;
  segEstatico = DESPEJE_SEG_MIN;
  
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

  {
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
        bool demandaLocalS1 = camara_leerPin(CAM_DEMANDA_PIN) || demanda_hayLocal();
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
      int presenciaActual = (camara_leerPin(CAM_DEMANDA_PIN) ? 1 : 0) + (coordinador_hayDemandaRemota() ? 1 : 0);

      if (strcmp(actual, estadoAnt) != 0 || presenciaActual != presenciaAnt) {
        lcd_dibujarInteligente(actual, presenciaActual, true);
        estadoAnt = actual;
        presenciaAnt = presenciaActual;
      }
  }
}