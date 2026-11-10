// ===== Validacion_Automatico/espera_bloque.inc =====
//
// D-175 (SPEC_8 8.4): EL ROJO DE ESPERA DEL MAESTRO, sobre coordinador.cpp y semaforo.cpp REALES.
// arnes_automatico.cpp lo incluye DOS veces: la primera, antes de vigilarEnclavamiento(), deja la
// excepcion por NOMBRE de la invariante de la pluma; la segunda, dentro de main(), el bloque M.
#ifndef ESPERA_BLOQUE_EXCEPCION
#define ESPERA_BLOQUE_EXCEPCION

// La tercera excepcion por NOMBRE de SFTY-28 (las otras: barrera fuera, D-96; camara mala, D-169). Pluma arriba en
// rojo porque el firmware anota la espera (semaforo_rojoEspera() REAL) Y el arnes ve el modo MENU por su cuenta
// (g_modoActual, que solo escriben el arnes y modoActual_set()). La espera fuera de MENU es una violacion propia:
// se cuenta en CADA tick, porque vigilarEnclavamiento() llama a esta funcion la primera de su condicion.
static long g_esperaFueraDeMenu = 0;
static bool excepcionEspera() {
  const bool espera = semaforo_rojoEspera();
  if (espera && modoActual_get() != MENU) g_esperaFueraDeMenu++;
  return espera && modoActual_get() == MENU;
}

#else
  // ===========================================================================
  // BLOQUE M - D-175: PLUMA ARRIBA EN EL ROJO DE ESPERA (SPEC_8 8.4), MAESTRO EN MENU
  // ===========================================================================
  // El MENU entra por coordinador_forzarMenu() REAL (lo que hace menu_setup() en la tarjeta) y corre por
  // coordinador_actualizar_background(), que es lo que main.cpp llama en MENU. El Esclavo simulado acusa el rojo:
  // con enlace, MENU es rojo fijo (TEST 5 de coordinador.cpp), no el ambar de sin enlace.
  std::printf("\n-- Bloque M: rojo de espera del Maestro (D-175) --\n");
  {
    const unsigned long PASO_M = 100UL, R = g_retardoPlumaMs;
    const ModoSistema modoAntes = g_modoActual;
    auto menuTick = []() { coordinador_actualizar_background(); };
    auto arriba = []() { return arnes_pines[MOTOR_TALANQUERA] == TALANQUERA_ABRIR; };
    // ms con la pluma arriba desde ahora, en MENU (-1: sigue arriba al tope).
    auto msArriba = [&](unsigned long tope) -> long {
      for (unsigned long h = 0; h <= tope; h += PASO_M) {
        if (!arriba()) return (long)h;
        arnes_millis_valor += PASO_M;
        menuTick();
        vigilarEnclavamiento();
      }
      return -1;
    };

    // -- M1 (R): MENU con la pluma ARRIBA (viene del verde): a 2R del rojo sigue arriba, con la espera anotada --
    arrancarAutomaticoPorDefecto();
    const long hastaVerde = bombear(PASO_M, 600000UL, []() { return semaforo_estado() == S_VERDE; });
    const bool arribaEnVerde = hastaVerde >= 0 && arriba();
    g_modoActual = MENU;
    coordinador_forzarMenu();
    const long hastaRojo = bombearGenerico(PASO_M, 10000UL, menuTick, []() { return semaforo_estado() == S_ROJO; });
    const long m1 = msArriba(2UL * R);
    char t1[400];
    std::snprintf(t1, sizeof(t1),
        "M1 (D-175, SPEC_8 8.4): coordinador_forzarMenu() REAL con la pluma arriba (verde %d): tras el amarillo "
        "(%ld ms) queda en S_ROJO con la pluma ARRIBA a 2R = %lu ms (-1 = sigue arriba: %ld) y "
        "semaforo_rojoEspera() = %d. Hoy bajaba a los R", arribaEnVerde, hastaRojo, 2UL * R, m1,
        semaforo_rojoEspera());
    comprobar(arribaEnVerde && hastaRojo >= 0 && m1 == -1 && semaforo_estado() == S_ROJO && semaforo_rojoEspera(),
              t1);

    // -- M2 (R): ROJO TOTAL desde MENU es rojo de accidente (D-173): borra la espera y baja tras R --
    coordinador_forzarRojoTotal();
    const bool sinEspera = !semaforo_rojoEspera();
    const long m2 = msArriba(3UL * R);
    char t2[300];
    std::snprintf(t2, sizeof(t2),
        "M2 (D-175): ROJO TOTAL desde MENU (el modo sigue MENU) borra la espera (%d) y la pluma baja a los R = %lu ms "
        "del rojo (medido %ld; -1 = no bajo en 3R)", sinEspera, R, m2);
    comprobar(sinEspera && m2 >= (long)(R - PASO_M) && m2 <= (long)(R + 2UL * PASO_M), t2);

    // -- M3 (C): MENU con la pluma ya ABAJO: no sube (la espera solo RETIENE) --
    coordinador_forzarMenu();
    bool nuncaSube = !arriba();
    for (unsigned long h = 0; h < 2UL * R; h += PASO_M) {
      arnes_millis_valor += PASO_M;
      menuTick();
      vigilarEnclavamiento();
      nuncaSube = nuncaSube && !arriba();
    }
    comprobar(nuncaSube && semaforo_rojoEspera() && semaforo_estado() == S_ROJO,
              "M3 (D-175): MENU con la pluma ya abajo: la espera queda anotada y la pluma NO sube en 2R "
              "(D-33: ni el retardo ni la espera abren la barrera)");

    // -- M4 (control): la violacion propia, vista fallar. Con la espera puesta y el modo fuera de MENU, la
    // excepcion no vale y el contador sube; se restaura para no ensuciar el resumen.
    const long antesM4 = g_esperaFueraDeMenu;
    g_modoActual = MODO_AUTOMATICO;
    const bool m4 = !excepcionEspera() && g_esperaFueraDeMenu == antesM4 + 1;
    g_modoActual = MENU;
    g_esperaFueraDeMenu = antesM4;
    comprobar(m4, "M4 (control): con semaforo_rojoEspera() true y el modo fuera de MENU, excepcionEspera() no "
                  "excusa la pluma y cuenta una violacion de la espera");

    coordinador_forzarRojoTotal();   // se sale sin espera y con la pluma abajo tras R
    msArriba(3UL * R);
    g_modoActual = modoAntes;
    comprobar(g_esperaFueraDeMenu == 0,
              "M5 (D-175): en NINGUN tick del barrido semaforo_rojoEspera() fue true fuera de MENU (la espera del "
              "Maestro la pone solo coordinador_forzarMenu() y la borran ROJO TOTAL y la entrada a un modo)");
  }
#endif
