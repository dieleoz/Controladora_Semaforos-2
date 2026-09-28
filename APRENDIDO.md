# APRENDIDO — lecciones de este proyecto para el siguiente

Formato de `pluginMetodo/docs/TRASPASO.md`. Una entrada por leccion; lo que `SUBE` se propone a Diego y no se aplica
sin su visto bueno.

## L-01 — Simular no sustituye a medir: el bucle se corta congelando el simulador y exigiendo acta de campo (28/09)
- **Paso:** la prueba de focos daba verde contra verde en servicio y ningun pack lo vio; lo encontraron las tramas de
  Marco (arreglo `1e56d83`, roadmap 1.53). El aparato de medir llego a crecer mas que el firmware sin acercar una
  tarjeta cargada.
- **Leccion:** cuando el instrumento crece y el producto no, se para de anadir instrumento: el simulador se congela y
  solo se archiva o se cambia con una medida sobre el equipo real.
- **Que hacer en el siguiente:** fijar desde el principio que un pack sale a historico solo con un acta
  `evidencia/<fecha>_campo_<tema>.txt` (tramas o medida, y hash cargado), como `CLAUDE.md` §4.
- **Destino:** SE QUEDA.

## L-02 — La razon instrumentos/producto se hace cumplir en el pre-commit, no en un texto (28/09)
- **Paso:** la regla «el aparato de medir no crece mas que lo que mide» vivio en `CLAUDE.md` §16 y no paro nada;
  medida en el pre-commit (`.githooks/trinquete.sh`, `RAZON_TECHO=1.665` en `.topes`) para un commit en seco.
- **Leccion:** un techo medido que solo baja, contado sobre el indice y sin `historico/`, es una puerta; la frase no.
- **Que hacer en el siguiente:** traer el trinquete con el esqueleto del proyecto y fijar su techo en la primera poda.
- **Destino:** SUBE a metodo (hook de `iniciar-proyecto`) — propuesto.

## L-03 — Que archivar se decide con un grafo de referencias reales, no leyendo (28/09)
- **Paso:** el grafo (`D:\@Proyect\Entregas_Semaforos\grafo\grafo.py`, `REPO.map`) separo lo que se LEE, se COMPILA o
  solo se CITA, y midio que el presupuesto de documentos del hook cuenta `historico/` (`REPO.map:11`): mover dentro de
  git no baja la cifra.
- **Leccion:** antes de archivar, un grafo por referencias reales (lecturas en codigo, compilacion, citas) y su borde
  escrito; lo que nadie lee ni cita se archiva, lo que se lee exige mover la ruta en el mismo commit.
- **Que hacer en el siguiente:** `grafo.py` como herramienta del metodo, y decidir si `docs_total()` excluye
  `historico/`.
- **Destino:** SUBE a metodo (skill `cerrar-sesion` o una nueva de archivado) — propuesto.

## L-04 — Al funcional se le manda el paquete sin manuales (28/09)
- **Paso:** los `.docx` viejos de `05_Funcional/` describian un sistema que ya no es; Diego los quito del paquete
  (`5cbbbe4`, `.claude/particularidades/entregar.md:22`).
- **Leccion:** en pruebas funcionales el paquete lleva fuente, APK y el `.html` de pruebas; un manual sin certificar
  confunde mas de lo que ayuda.
- **Que hacer en el siguiente:** el empaquetador no incluye manuales hasta que el equipo este certificado.
- **Destino:** SE QUEDA.
