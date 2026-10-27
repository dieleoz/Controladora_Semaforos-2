// E0 de arnes_automatico.cpp, incluido dentro de main() (el arnes no crece: tope de 500 lineas).
    // -- E0: EL RANGO DE SET_TIEMPOS (D-131, SPEC_4 3.1) ------------------------------
    // Verde y rojo 3..15 min, despeje 10..90 s, juzgados con el numero que llego: 259 y
    // -253 eran 3 tras el (uint8_t) y entraban con OK. Los bordes matan el mutante que
    // borra las guardas de fijarTiempos(); el int() mata el que vuelve a truncar.
    {
      const int V[][3] = {{259, 3, 10}, {-253, 3, 10}, {3, 259, 10}, {3, 3, 266},
                          {2, 3, 10}, {16, 3, 10}, {3, 2, 10}, {3, 16, 10},
                          {3, 3, 9}, {3, 3, 91}};
      bool niUno = true;
      for (const auto& t : V) niUno = niUno && !configurarTiempos(t[0], t[1], t[2]);
      comprobar(niUno, "E0: SET_TIEMPOS rechaza 259, -253, 266 y los bordes 2/16 min y "
                       "9/91 s (D-131): el numero que llego, sin truncar");
      comprobar(configurarTiempos(3, 15, 90) && configurarTiempos(15, 3, 10),
                "E0: SET_TIEMPOS acepta los bordes 3 y 15 min, 10 y 90 s (D-131)");
    }
