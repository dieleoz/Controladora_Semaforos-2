---
paths: ["**/*.c", "**/*.h", "**/*.java", "**/*.kt", "**/*.py"]
---

# Modularidad

Fuente y justificación: `informes/INFORME-MODULARIDAD.md` del plugin metodo.

- **Función: 60 líneas objetivo, 100 de techo** (BARR-C:2018 6.2.a; NASA Power of Ten, regla 4).
  **Fichero: 500 líneas**, decisión de Diego del 21-sep-2026 (los linters toleran 1000-2000).
  Las pruebas cuentan igual: un fichero de prueba por módulo.
- **Funcionalidad nueva, módulo nuevo**: `.c/.h` pareados en firmware; clase o subpaquete en la app;
  módulo propio en Python. La SPEC nombra el fichero y su responsabilidad antes de escribir código.
- **Capas**: calibración, protocolo, medida y persistencia no viven en la interfaz (Activity, pantalla
  STONE, CLI). La interfaz pinta y llama.
- **Fichero existente por encima del límite**: no se parte por adelantado. La próxima vez que haya que
  tocarlo, primero el corte en su commit propio con una prueba de salida idéntica, después el arreglo.
- **Excepción: firmware grabado en un equipo.** Partirlo cambia el `.hex` y obliga a repetir las
  puertas. Hasta el primer cambio funcional que obligue a regrabar, el arreglo mínimo va donde está;
  en ese cambio entra el corte, como commit previo y con md5 comparado.
- El arquitecto rechaza un cambio que haga crecer un fichero por encima del límite.
