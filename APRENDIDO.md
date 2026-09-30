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

## L-05 — Por encima de 500 lineas, el neto es <= 0 en cada commit; el corte previo no da margen (28/09)
- **Paso:** para meter el sello en `app.js` (6324) hice dos cortes previos (`a36b35f`, `2584bde`) porque el hook
  pedia «Primero el corte, en su commit»; siguio rechazado contra el HEAD nuevo. Paso sacando lineas en el mismo
  commit (`e15388d`). Con `generar_entrega_v9_0.py` si sirvio: el corte lo dejo bajo 500.
- **Leccion:** el hook compara contra HEAD; el mensaje prometia un camino que no existe.
- **Que hacer en el siguiente:** todo encargo sobre un fichero grande lleva escrito «neto <= 0 en este commit».
- **Destino:** SUBE a metodo — atendido: mensaje nuevo en metodo `4e8e898` («No crece: saca en este commit lo que
  anades»).

## L-06 — La APK se prueba en un telefono por adb y CDP, y el telefono es de alguien (28/09)
- **Paso:** la revision en el telefono encontro lo que 337 pruebas jsdom no: la cinta anotaba ENVIADA sin enlace
  (contra SPEC_4 §4), atras cerraba la app, el Maestro enterrado entre MAC anonimas. `compilar_apk.bat` no corre con
  espacio en la ruta: se llama al wrapper de Gradle con `java -classpath gradle-wrapper.jar`.
- **Leccion:** una prueba de app que no toca el aparato mide la app que imaginamos. Y el telefono es personal: un
  toque por coordenadas cayo en ROJO TOTAL (sin enlace) y unas teclas en WhatsApp.
- **Que hacer en el siguiente:** pulsar por CDP (id del boton), comprobar la actividad en primer plano antes de cada
  tecla, pedir el telefono libre y no elegir nunca destino en la hoja de compartir.
- **Destino:** SE QUEDA (la receta de APK es de este proyecto).

## L-07 — El paquete del funcional no lo arma `generar_entrega_v9_0.py` (28/09)
- **Paso:** el script saca el paquete de revision (manuales `.md`/`.docx`, acta, 275 ficheros); la receta del
  funcional (`.claude/particularidades/entregar.md` §1) es otra y se armo a mano con `git archive` de HEAD.
- **Leccion:** dos recetas y un solo script: el que empaqueta para el funcional tiene que ser un instrumento.
- **Que hacer en el siguiente:** un modo `--funcional` del empaquetador, sin que ningun fichero de >500 crezca.
- **Destino:** SE QUEDA.

## L-08 — Un binario sin pruebas lleva defectos que las pruebas de despues cazan; el nombre tiene que decirlo (30/09)
- **Paso:** por decision del responsable salio `a0d605b` (Degradado automatico) sin pruebas ni compuerta, `SIN_BANCO`.
  El runbook `N-167` cazo despues 5 defectos en ese codigo: 180 s de verde contra ambar con corte de un sentido,
  reentrada tras rendicion y tras salida manual, `$ALARM` fuera de cota con checksum bueno y un aviso que no saltaba.
- **Leccion:** `SIN_BANCO` avisa de que falta la tarjeta, no de que falten SPEC y pruebas; quien lo recibe no distingue.
- **Que hacer en el siguiente:** un paquete sin pruebas propias lo dice en el nombre y en el mensaje al funcional, y se
  retira en cuanto sale el validado.
- **Destino:** SE QUEDA.

## L-09 — El rojo heredado se mide antes de tocar (30/09)
- **Paso:** tras `a0d605b` el banco tenia 7 packs en rojo; seis venian de ese commit y se mezclaron con el rojo del
  cambio siguiente (`93bce43` reapunto cinco a la forma nueva; `917ef1e` cerro `esp32_07`, defecto real).
- **Leccion:** sin el total de partida no se sabe que rompio cada cambio.
- **Que hacer en el siguiente:** compuerta antes de la primera linea y su acta como base del encargo.
- **Destino:** SE QUEDA.

## L-10 — Un permiso bloqueado a un agente no se sortea desde el orquestador (30/09)
- **Paso:** a un agente se le bloqueo retirar la salvaguarda de 31 dias y editar dos packs; no se reintento por otra via
  y se devolvio al responsable (la retirada la confirmo el; el control negativo de `app_01`/`app_08` sigue pendiente).
- **Leccion:** que un permiso impida retirar una barrera es informacion: se reporta y decide el responsable.
- **Que hacer en el siguiente:** todo encargo dice "si el permiso lo impide, para y reporta".
- **Destino:** SUBE a metodo (regla de encargos a subagentes). Aprobado 30/09: el responsable delega en el
  arquitecto del orquestador, que la propuso.

## L-11 — Lo que enlaza un arnes se actualiza en el mismo commit (30/09)
- **Paso:** el simulador del puente aborto dos veces: faltaban dobles de funciones nuevas (`49580cf`) y el aviso de
  paleteros bloqueaba AUTOMATICO al conectar porque preguntaba sin saber la opcion (`7173d3b`).
- **Leccion:** una funcion nueva del firmware o una guarda nueva de la app rompe el arnes que las ejerce, y la compuerta
  solo dice ABORTADO.
- **Que hacer en el siguiente:** grep de los dobles y del arnes afectado en el mismo commit que la funcion o la guarda.
- **Destino:** SE QUEDA.
