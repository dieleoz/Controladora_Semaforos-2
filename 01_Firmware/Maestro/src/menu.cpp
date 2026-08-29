// ===== src/menu.cpp =====
#include "menu.h"
#include "modos.h"
#include "reloj.h"   // N-31
#include "botones.h"
#include "lcd.h"
#include "semaforo.h"
#include "pines.h"
#include "coordinador.h"

static int cursorMenu = 0;

// ---------------------------------------------------------------------------
// V8.7 (SFTY-21): el menu se parte en DOS NIVELES.
//
// Historia, porque explica la decision. En V8.1 se anadio PRUEBA ALCANCE (4a opcion)
// y en V8.6 AJUSTAR HORA (5a), que ya obligo a comprimir el interlineado de 11 a 9 px.
// Al llegar MODO DEGRADADO, la sexta linea caia en 24 + 5*9 = 69: fuera de una
// pantalla de 64 px. Y el fallo no habria sido que no se dibujara -la salvaguarda de
// lcd_dibujarMenu() lo impide- sino que el CURSOR SI PODIA LLEGAR HASTA ELLA, dejando
// al operario navegando a ciegas sobre una opcion invisible. Justo encima de la
// opcion que arranca el modo que da verde por reloj.
//
// La causa no era el numero de opciones, era mezclar dos cosas distintas en una lista
// plana:
//
//   MODOS DE OPERACION   MANUAL / AUTOMATICO / INTELIGENTE   se eligen a diario
//   HERRAMIENTAS         PRUEBA ALCANCE / AJUSTAR HORA /     se tocan rara vez, y
//   Y CASOS ESPECIALES   MODO DEGRADADO                      requieren criterio
//
// Separandolas, el menu principal vuelve a 4 opciones -EXACTAMENTE el layout validado
// en campo y en el arnes, base 28 y paso 11- y el submenu se queda en 3, que usa ese
// mismo layout. Ninguno de los dos se acerca al limite, y una septima opcion futura
// tampoco obligaria a comprimir nada.
//
// Y hay un beneficio de seguridad que no es accesorio: con el mando de reles operando
// A CIEGAS desde el suelo, una rafaga accidental de pulsos ya no puede alcanzar
// AJUSTAR HORA ni MODO DEGRADADO, porque estan un nivel por debajo y para bajar hace
// falta el Boton 3, que es el unico que las secuencias del mando tienen prohibido
// (ver mando.cpp). Refuerza POR ESTRUCTURA el requisito de ignorar secuencias con el
// menu abierto, en vez de dejarlo todo colgando de una sola comprobacion.
// ---------------------------------------------------------------------------
enum NivelMenu { NIVEL_RAIZ, NIVEL_CONFIG };
static NivelMenu nivel = NIVEL_RAIZ;

static const char* opcionesRaiz[4] = {"MANUAL", "AUTOMATICO", "INTELIGENTE",
                                      "CONFIGURACION"};
static const int OPCIONES_RAIZ = 4;

// N-31: la cuarta, REINICIAR RELOJ. Cabe sin comprimir nada -el layout de 4 usa el
// interlineado de 11 px, que es el validado en campo- y solo la busca quien ya sabe
// que el reloj no arranca.
static const char* opcionesConfig[4] = {"PRUEBA ALCANCE", "AJUSTAR HORA",
                                        "MODO DEGRADADO", "REINICIAR RELOJ"};
static const int OPCIONES_CONFIG = 4;

// N-31: cuanto se sostiene el resultado del reinicio antes de repintar el menu. El
// operario esta a 5 m del gabinete y este mensaje decide si sigue con software o coge
// el soldador; no puede parpadear y desaparecer. 0 = no hay resultado en pantalla.
static unsigned long tResultadoReinicio = 0;
static const unsigned long RESULTADO_REINICIO_MS = 6000;

static int cantidadOpciones() {
  return (nivel == NIVEL_RAIZ) ? OPCIONES_RAIZ : OPCIONES_CONFIG;
}

static void repintar() {
  if (nivel == NIVEL_RAIZ) {
    lcd_dibujarMenu(cursorMenu, opcionesRaiz, OPCIONES_RAIZ);
  } else {
    lcd_dibujarMenu(cursorMenu, opcionesConfig, OPCIONES_CONFIG, "CONFIGURACION");
  }
}

void menu_setup() {
  modoActual_set(MENU);
  // Se vuelve SIEMPRE al nivel raiz. Al salir de un modo el equipo debe aparecer
  // donde el operario espera encontrarlo, y no en un submenu en el que quiza nunca
  // estuvo -por ejemplo tras una vuelta al menu provocada por el propio firmware-.
  nivel = NIVEL_RAIZ;
  cursorMenu = 0;
  coordinador_forzarMenu(); // Fuerza Rojo Fijo en Maestro y Esclavo
  repintar();
}

void menu_loop() {
  // N-31: mientras el resultado del reinicio esta en pantalla, se ignoran los botones
  // y no se repinta el menu encima. Sin esto el mensaje duraria una vuelta del bucle
  // -invisible- y el operario no sabria si el reinicio hizo algo.
  if (tResultadoReinicio != 0) {
    if (millis() - tResultadoReinicio < RESULTADO_REINICIO_MS) {
      // Se consumen los flancos para que un boton pulsado durante la espera no salte
      // a un modo en cuanto la pantalla se retire.
      botonArriba(); botonAbajo(); botonAceptar(); botonCancelar();
      return;
    }
    tResultadoReinicio = 0;
    repintar();
  }
  bool redibujar = false;
  const int n = cantidadOpciones();

  if (botonArriba()) {
    cursorMenu = (cursorMenu + n - 1) % n;
    redibujar = true;
  }
  if (botonAbajo()) {
    cursorMenu = (cursorMenu + 1) % n;
    redibujar = true;
  }
  if (botonAceptar()) {
    if (nivel == NIVEL_RAIZ) {
      switch (cursorMenu) {
        case 0: modoActual_set(MODO_MANUAL);      return;
        case 1: modoActual_set(MODO_AUTOMATICO);  return;
        case 2: modoActual_set(MODO_INTELIGENTE); return;
        default:
          // CONFIGURACION: baja de nivel. NO arranca ningun ciclo ni cambia el
          // estado de las luces; el equipo sigue en el mismo estado seguro que el
          // menu principal (Rojo Fijo con enlace, Ambar sin el).
          nivel = NIVEL_CONFIG;
          cursorMenu = 0;
          repintar();
          return;
      }
    } else {
      switch (cursorMenu) {
        case 0:  modoActual_set(MODO_ALCANCE);   break;
        case 1:  modoActual_set(MODO_HORA);      break;
        case 2:  modoActual_set(MODO_DEGRADADO); break;
        default: {
          // N-31: se ejecuta AQUI mismo, sin pantalla propia. Es una operacion de un
          // solo paso cuyo resultado se lee en AJUSTAR HORA: si el aviso pasa de
          // "SIN CRISTAL" a "RELOJ SIN PONER EN HORA", el estado sucio ERA la causa.
          const bool arranco = reloj_reiniciarDominioRespaldo();
          lcd_dibujarReinicioReloj(arranco);
          tResultadoReinicio = millis();
          return;
        }
      }
      return;
    }
  }

  // Boton 4 en el submenu: vuelve al menu principal, no sale a ningun modo. Es la
  // misma salida que el operario ya conoce del resto de pantallas, y deja el cursor
  // sobre CONFIGURACION para que se vea de donde acaba de venir.
  if (botonCancelar() && nivel == NIVEL_CONFIG) {
    nivel = NIVEL_RAIZ;
    cursorMenu = OPCIONES_RAIZ - 1;
    redibujar = true;
  }

  if (redibujar) {
    repintar();
  }
}
