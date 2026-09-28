Orquestador: f34fac3 · Metodo: ad6bd71
# CLAUDE.md — reglas permanentes del repositorio

Aqui vive solo lo que sigue siendo cierto con el firmware cambiado entero, en linea imperativa y con su mecanica. Cobre
medido, a `05_Funcional/17_...`; decisiones, a `DECISIONES.md`; el porque de un `N-x`, a `roadmap.md`; cifras, al acta
de `evidencia/`; cronica, a `HISTORIA.md`. Lo comun a todos los proyectos es el metodo global de Diego y no se repite.
La mecanica larga esta en `.claude/particularidades/metodo.md`, con el mismo numero de apartado. **Los numeros de
apartado NO se tocan:** packs, firmware y manuales citan «`CLAUDE.md` §7.1» y nada comprueba esas citas. Un apartado se
vacia por dentro; una regla nueva abre uno al final. Censo: `grep -rn "CLAUDE.md §" . --include=*.{py,cpp,md}`.

## 0. Lo que no se negocia

1. **Un semaforo que falla mal mata a alguien.** Es lo que explica todo lo demas.
2. **Nada sube a campo sin pasar banco.** Que firmware hay en cada equipo lo dice `ESTADO.md`, no este fichero. Una foto
   de campo sin el hash de lo que habia dentro no se lee (§7).
3. **Un verde de la compuerta no es un entregable:** los arneses de PC no encuentran nada; la tarjeta no esta probada.
4. `ABORTADO` no es `PASS` (§1) · solo `semaforo.cpp` escribe pines de luz (§2) · el firmware nuevo esta CARGADO en la
   tarjeta antes de que nadie enchufe nada (§3).

## 1. ABORTADO no es PASS

`PASS`: corrio y cumple · `FALLA`: corrio y no cumple · `ABORTADO`: no pudo correr; no dice nada del firmware.

- Un `ABORTADO` es una puerta abierta: se arregla antes de mirar nada mas (`N-75`). Cuentan igual de mal un `FALLA`
  con codigo `0` (`N-46`), un `FALLA` que ningun firmware apaga (va en `reportar()`) y un `x/y` con `x != y`.
- **Un rojo no se apaga escribiendo lo que el instrumento quiere leer** (un ancla sin construir, una cita metida en un
  manual): se construye o se deja rojo. Muchas decisiones sin construir: se decide mas rapido de lo que se construye.

## 2. Barrera de salidas

- **Solo `semaforo.cpp` escribe pines de luz**, todo por su `escribirPines()` estatico (SFTY-2); los destellos del mando
  interceptan las escrituras en vez de rodearlas. Lo vigila `barrera_01_pines_de_luz`.
- Una orden invalida se rechaza y se reporta. El ambar automatico queda para los caminos que ya lo tienen (SFTY-6,
  watchdog): la maquina no decide sola un modo que nadie pidio.
- **Un `$ACK` depende de lo que la llamada devolvio**, o es una mentira con formato de exito (`SET_RTC`,
  `MANUAL:CAMBIAR_TURNO`, `SET_TIEMPOS`, `N-151`). El molde es `SET_TIEMPOS`: pregunta DENTRO del `if`, con un `$ERR`
  por cada motivo de rechazo. Un despachador nuevo se escribe copiandolo.
- Una regla de seguridad que enumera sujetos comprueba que cada uno EXISTE (`N-96`); pines: `ARQUITECTURA.map` y spec.

## 3. Cobre, conectores y carga

- Antes de contestar sobre cobre, conectores, pines o compras se abre la spec: `SPEC_*` y `05_Funcional/17_...` ganan.
- **Firmware primero, cableado despues.** Retirado el armador de un pin, su pull-down lo deja en 0 V y no ejecuta nada;
  con el firmware viejo dentro sigue siendo un boton que EJECUTA. Se exige la CARGA VERIFICADA, no el merge.
- **`J16` p1 lleva 12 V crudos** a un conector de senal directa al micro: se TAPA en cada equipo (`D-4`, `N-120`).
- **`J14` es una ENTRADA del micro** (3,3 V, sin opto ni diodo); la salida de talanquera es `J15`. Un rele cableado a
  `J14` se desconecta antes de energizar.
- **`J16` p5 y p8 quedan vacios y no se cablean:** el firmware los sigue LEYENDO (`botonArriba()`/`botonAbajo()`) y uno
  anterior a la retirada del mando compone secuencias con un puente. En campo manda lo CARGADO (`SPEC_5` §3).
- **Carga por SWD: `mode=UR` con `-e all`, y no se cambia.** `HOTPLUG` engancha en marcha y, con un firmware que se
  cuelga al arrancar, el watchdog reinicia cada 4 s en mitad del borrado (`failed to erase memory`; delator: `NVM size:
  128 KBytes (default)` en 64 KB). Si `UR` falla se reintenta. Radios: `2.4 kbps` de Air Data Rate, `M0`/`M1` en OFF.

## 4. La compuerta y el banco

```
python 01_Firmware/compuerta.py            # completo (compila) — 0 PASS · 1 FALLA · 2 ABORTADO
python 01_Firmware/compuerta.py --rapido   # sin compilar
python 01_Firmware/Simulaciones/banco/correr.py --pack <nombre>
```

Escribe un acta con fecha y hash de HEAD en `evidencia/`; las cifras de los documentos se copian del acta. `correr.py`
es UNA fila de veinte: la compuerta mide ademas los arneses que compilan C++ real, los simuladores y la app.

**Cadencia: que se corre segun lo que se toca.**
- App (boton, texto): solo los tests de la app.
- Firmware que no toca luces ni coordinacion: `compuerta.py --rapido`.
- Firmware que toca luces, coordinacion o seguridad: la compuerta completa, una vez.
- Candidata a paquete: la completa una vez; otra solo si la primera movio cifras del acta o compilo tras cambiar
  fuentes (manda la segunda). QA una sola vez. Un fallo intermitente es defecto del instrumento, no motivo de pasada.
- **Simulador CONGELADO y con trinquete:** ningun pack ni arnes nuevo, y la razon instrumentos/producto no sube. Un
  hallazgo de campo se comprueba con tramas o con la tarjeta. Un pack sale a historico solo con un acta de campo
  `evidencia/<fecha>_campo_<tema>.txt` (tramas o medida, y hash cargado) que el responsable declara.

La comprobacion de cifras lee el acta ANTERIOR: tras un `--rapido`, la siguiente completa se corre dos veces; y un
`FALLA` deja sin cifra el acta y aborta el pack de cifras siguiente: se arregla y se corre hasta que dos pasadas den lo
mismo. La cura nunca es tocar el pack. Como se escribe una guarda: `metodo.md` §4.

## 5. Los instrumentos leen el fuente por RUTA

Los validadores parsean por tuplas (`("Maestro", "src", "mando.cpp")`): mover un `.cpp` o `.md` rompe un instrumento.
Movimiento y rutas en el MISMO commit, compuerta verde antes y despues; lo mudado de fichero no lo ve la guarda: se
compara el total de comprobaciones contra el de siempre.

## 6. Declarar no es EJERCER

Un instrumento verde dice que la DECLARACION esta bien escrita, no que nadie la ejerza. Con `grep` y compilador:

1. **¿Quien LLAMA a esto?** Trinquete: falla una huerfana nueva, una que gana llamador y sigue en la lista, y una que
   los documentos anuncian como existente (`N-73`).
2. **¿Esta guarda puede dar las DOS respuestas?** Y antes de borrar el ARMADOR de una bandera se censa quien la LEE:
   si de ella cuelgan vetos, quedan ABIERTOS (`D-1`).
3. **¿Este fichero se COMPILA en algun sitio?** Si ningun arnes lo enlaza, un pack de texto no ve un defecto del TIEMPO.

Una excepcion que deja un pack en verde se mide al escribirla y al heredarla (`N-122`). Un arnes que no se ha visto
fallar es un adorno: se le inyecta un defecto en el `.cpp` real, restaurando por HASH (`metodo.md` §6).

## 7. La regla del instrumento

Un «no aparece» no es hallazgo sin descartar al buscador (`N-44`). Instrumento contra razonamiento: manda la medida.

1. Un cero de `grep` es «mi patron no encontro»: se busca por los dos nombres, o por quien la APAGA, sin comentarios.
2. Si el sintoma trae un NUMERO, se busca en el fuente antes de la primera hipotesis.
3. Se cita el SIMBOLO, no la linea (salvo fechada a un commit o pegada de un `grep`): `grep -rn "N-133" 01_Firmware`.
4. Un informe o una refutacion no es una medida: se reproduce. Una causa que cae se marca REFUTADA, no se borra.
5. Eliminar solo vale con opciones exhaustivas, y biseccionar con un extremo bueno VERIFICADO. Sospechoso por ficheros
   (`git log --oneline <bueno>..HEAD -- <ficheros>`); se comparan hashes, no tamanos.

La foto de campo es la unica medida sobre el aparato real: si choca con un instrumento, se sospecha del instrumento.
Todo instrumento que compara contra un borde escribe al lado cual es y por que.

## 8. Lo que YO produzco es un instrumento

Antes de un pack, informe, menu o encargo: «¿esto acerca una tarjeta cargada, o la sustituye?». La medida se recalcula:
lineas de `{Maestro,Esclavo,Repetidor}/{src,include}` frente a `Simulaciones + Validacion_* + compuerta.py`.

1. Un pack que certifica otra vez lo certificado sustituye (y hoy el simulador esta congelado, §4).
2. Un menu de opciones sobre una causa sin medir blinda el error: se dice en la pregunta que no esta medido (`N-142`).
3. Un encargo ejecuta la spec, no una frase: si chocan se PREGUNTA, y mas si el cambio retira una barrera.

El trabajo delegado se revisa por el DIFF, y mas cuando sale verde: ¿el modelo replica el arreglo o relaja la
comprobacion?; un `grep` por afirmacion; `sha256sum` al binario. Dos preguntas en una variable: dos banderas.

## 9. Al arreglar un defecto, busca las pruebas que lo CELEBRABAN

Hay pruebas que EXIGEN el defecto (`N-49`). Van una por una, contando antes sus propiedades (`N-83`): se REPARTEN, se
INVIERTEN, se CONSERVAN o se BORRAN. Una inversion que solo mira el RESULTADO aprueba barreras en el orden equivocado.

## 10. Flash, RAM y cotas de buffer

64 KB por micro; el margen se lee en la ultima acta (`ls -t evidencia/*_compuerta.txt | head -1`), no se recita. Antes
de sacrificar una funcion se mide por FICHERO OBJETO en `firmware.map` (`N-70`); la RAM, con `nm` sobre el `.elf`
(`N-86`); la flash, con una segunda pasada. La cota de un buffer es el BUFFER de cada campo (`N-154`); fuera de cota se
publica `!`, nunca `--`. Sin metodos virtuales. Detalle: `metodo.md` §10.

## 11. Trabajo en paralelo, y el INDICE antes del commit

Nunca `git add -A`: rutas explicitas, o cada agente en su `git worktree`. Antes de comitear se LEE el indice (`git diff
--cached --name-only`) y se cuentan las lineas; el codigo de salida del `add` no vale. Trampas de `add`, scripts que
truncan, borrados por delimitador, `worktree remove` y agentes reanudados: `metodo.md` §11. Orden de lanzamiento:

1. Antes de lanzar se abre `DECISIONES.md`: si el encargo contradice una fila, es una pregunta; un manual no la reabre.
2. Los que MIDEN van antes que los que ESCRIBEN; el veredicto va al scratchpad y el que construye lo lleva delante.
3. Un agente = un conjunto de ficheros DISJUNTO, nombrado, con los prohibidos.
4. El encargo dice contra que `D-x` trabaja: si el codigo y la `D-x` no coinciden, para y reporta; no elige.
5. Ningun agente comitea: comitea el orquestador, con rutas explicitas.
6. A cada agente se le pide que DUDE del encargo.
7. Una decision que retira una barrera se revisa ANTES de construirla, buscando el modo de fallo que crea; la premisa
   que contradice suele estar en un comentario del fuente: se corrige hacia la decision o se replantea.
8. La compuerta la corre el orquestador con el arbol QUIETO.

## 12. Donde esta cada cosa

`05_Funcional/SPEC_0..SPEC_8` MANDA, tambien sobre el firmware (§15) · `05_Funcional/17_...`, cobre medido con fecha,
instrumento y firmware dentro · `DECISIONES.md`, el andamio: lo ya contestado · `ARQUITECTURA.map`, que abre cada
instrumento y que hay en cada conector · `roadmap.md`/`roadmap_hist.md`, el porque, por `N-x` · `ESTADO.md`, hoy ·
`evidencia/`, las actas · `HISTORIA.md`, la cronica · `OPTIMIZACIONES.md`, reglas `SFTY-x` y su trazabilidad ·
`01_Firmware/compuerta.py`, la unica forma de verificar · `04_Manuales/`, para el tecnico y el auditor.

## 13. Convenciones

- Comentarios y commits en espanol, ASCII sin acentos (consola cp1252; los validadores parsean el fuente). Un
  comentario que delibera dentro de una regla de seguridad es una alarma: se resuelve o va a `roadmap.md`.
- Un commit = un cambio con sentido propio = un `git revert` limpio; cada `N-x` se cierra con su evidencia.
- Un pack que ejerce una regla lleva `# EJERCE SFTY-x: <que>` en la cabecera, solo si la comprueba de verdad.
- Un binario se nombra `<producto>_<fecha>_<hash>_SIN_BANCO`; el sufijo lo quita solo quien lo probo en un equipo. Un
  binario nuevo se acredita con `sha256sum`, nunca con su tamano.
- **Los paquetes salen a `entregas/` (ignorada); los viejos, a `entregas/RETIRADOS/`. Nada fuera del repo.**

## 14. Lo que nadie recalcula, envejece — y eso incluye las LISTAS

Una lista de alcance se RECUENTA con `grep` antes de ejecutarla, por el nombre y por su consecuencia; se queda corta
por lo que no es firmware (app, `.html`, manual, LEEME de un `.zip`). Tras un `sed` de cifras se busca el VEREDICTO
que las acompanaba. Un numero donde no se puede recalcular (`app.js`) se retira. Copias de `app.js`: `metodo.md` §14.

## 15. El orden de prioridad del trabajo

1. **La SPEC (`SPEC_0..SPEC_8`) manda sobre el firmware**; en cobre medido, `17_...`. Un choque (revisor, decisiones
   o el responsable) no deja dos versiones vivas: se ajusta la spec y el firmware la sigue.
2. Los arneses e instrumentos, que miden la spec.
3. Los manuales y la documentacion.

Dos registros que no se mezclan: lo que el equipo HACE (validado contra el fuente; si no se cumple, se corrige la spec)
y lo que DEBE hacer («HUECOS MEDIDOS», la cola del firmware); una frase que no marca en cual esta, esta mal escrita. Una
fila de `DECISIONES.md` lleva su respuesta en dos lineas y lo que deroga. Un cambio de constante cuesta dos ediciones:
la fuente y el instrumento que la recalcula. Los manuales no se tocan hasta estar todo certificado (banco pasado y
tarjeta cargada), salvo los `.html` de campo. Como se escribe una spec: `metodo.md` §15.

## 16. Limpiar es parte del trabajo, y se MIDE

La medida de §8 se recalcula antes de cada tanda. Un fichero base (`DECISIONES.md`, `OPTIMIZACIONES.md`,
`05_Funcional/17_...`, `roadmap.md`, `ESTADO.md`, `ARQUITECTURA.map`, `README.md`) no pasa de 1.000 lineas: se PARTE y
la cronica va literal a su `_hist` con las dos cuentas. ~~Un sujeto muerto va a `documentos_06`~~: retirado el 28/09
con su pack; cubre ese caso leer `DECISIONES.md` antes de lanzar (§11.1). Archivar: `metodo.md` §16.
## 17. Un ABORTADO del worktree no es una medida, y una fila del roadmap no es una causa

1. Un worktree de agente no tiene `node_modules` y aborta «app ejecutada en DOM» y «simulador del puente ESP32». La
   compuerta que autoriza se corre en el arbol principal; no se crean junctions a `node_modules`.
2. Una fila del roadmap que nombra el mecanismo es una hipotesis: se re-mide en un arnes que ejecute las dos puntas
   antes de construir; si cae, se marca REFUTADO en la fila (§7.4) y se rehace el orden por dano.
