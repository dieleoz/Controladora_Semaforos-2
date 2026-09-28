# HISTORIA — la cronica que salio de `CLAUDE.md` y `ESTADO.md`

Fichero sin tope. Nace el 28/09/2026 (fase 2 de la organizacion del repo, `ESTADO.md`) al dejar `CLAUDE.md` en 200
lineas y `ESTADO.md` en su estado actual. **Todo lo de abajo es texto LITERAL de esos dos ficheros tal como estaban
en `126d5eb`**; el unico cambio es que sus titulos bajan un nivel (se les antepone `#`) para no confundirse con los
de este fichero. Las reglas vigentes siguen en `CLAUDE.md` (resumidas, con su mismo numero de apartado) y en
`.claude/particularidades/metodo.md`; aqui queda el texto de donde salieron, con su cronica. **Esto no manda:** es
el porque, como `roadmap_hist.md`. Las cuentas, al final.

## 1. De `CLAUDE.md` (`126d5eb`, 541 lineas)

## CLAUDE.md — reglas permanentes del repositorio

**Esto es metodo.** Un parrafo vive aqui solo si **sigue siendo cierto con el firmware cambiado entero**. Una medida
de cobre va a `05_Funcional/17_...`; una fecha o una decision, a `DECISIONES.md`; **como** se descubrio algo, a
`roadmap.md` bajo su `N-x`; una cifra, al acta de `evidencia/`. Aqui, el puntero — nunca el valor.

> 🔴 **Objetivo 150 lineas (`wc -l CLAUDE.md` da la cuenta).** Lo que ocupa sitio son **las MECANICAS** —el «como»
> de cada regla, los comandos, los patrones, los modos de fallo— y se quedan: **un aforismo sin mecanica no protege
> a nadie; si dudas, se queda.** Lo que no entra es **CRONICA** (fechas, quien, cuantas pasadas, el relato): eso es
> `roadmap.md`, y aqui queda solo su LECCION en una linea imperativa. **Una regla nueva ABRE un apartado al final,
> o no entra.**
>
> 🔴 **Y LOS NUMEROS DE APARTADO NO SE TOCAN.** Packs, arneses, manuales y comentarios del firmware citan
> «`CLAUDE.md` §7.1» **desde fuera** y **ningun instrumento comprueba esas citas**: renumerar los deja cojos a
> todos de golpe y en silencio. Un apartado se vacia o se fusiona **por dentro**; no cambia de numero. Censo:
> `grep -rn "CLAUDE.md §" . --include=*.py --include=*.cpp --include=*.md`.

### 0. Lo que no se negocia

1. **Un semaforo que falla mal mata a alguien.** Es lo unico que explica todo lo demas.
2. **Nada sube a campo sin pasar banco.** Que firmware hay en cada equipo lo dice **`ESTADO.md`**, no este fichero
   — aqui esa frase ya caduco una vez. **Una foto de campo sin el hash de lo que habia dentro no se lee** (§7).
3. **Un verde de la compuerta NO es un entregable:** dice que los modelos y arneses de PC no encuentran nada, **no
   que el firmware funcione en la tarjeta**. Ha habido un `20/20` con una regresion viva en banco.
4. **`ABORTADO` no es `PASS`** (§1) · **solo `semaforo.cpp` escribe pines de luz** (§2) · **el firmware nuevo esta
   CARGADO en la tarjeta antes de que nadie enchufe nada** (§3).

### 1. ABORTADO no es PASS

| | significa |
|---|---|
| `PASS` | corrio y el firmware cumple |
| `FALLA` | corrio y el firmware **no** cumple |
| `ABORTADO` | **no pudo correr** — no dice *nada* del firmware |

**Un `ABORTADO` no es una casilla pendiente: es una PUERTA ABIERTA**, y todo lo que vigilaba entra sin mirar
(`N-75`). **Se arregla antes de mirar nada mas.** Sus tres hermanos, que se cuentan igual de mal: un `FALLA` que
sale con codigo `0` (`N-46`); un `FALLA` **permanente** que ningun firmware puede apagar —eso es nota, y va en
`reportar()`—; y un instrumento que publica `x/y` con `x != y`.

> 🔴 **El cuarto, el que tienta cuando el rojo lleva dias: UN ROJO NO SE APAGA ESCRIBIENDO LO QUE EL INSTRUMENTO
> QUIERE LEER.** Poner el ancla de una decision **sin construirla** dice que esta implementada; meter una cita en
> un manual para que deje de contarla como ausente, igual. **Si un rojo se apaga CONSTRUYENDO, se construye o se
> deja rojo. Nunca se decora.** Y entonces lo que se vigila no es el rojo, **es su CUENTA**: varias decisiones
> vigentes sin construir no dicen que el banco se degrade — dicen que **se decide mas rapido de lo que se
> construye**, y eso se arregla con teclado.

### 2. Barrera de salidas

**Solo `semaforo.cpp` escribe pines de luz**, todo por su `escribirPines()` estatico (enclavamiento SFTY-2) —
incluidos los destellos del mando, que **interceptan** las escrituras en vez de rodearlas para no dejar colgado al
coordinador. Una orden invalida **se rechaza y se reporta**; el ambar automatico queda reservado a los caminos que
ya lo tienen (SFTY-6, watchdog): **la maquina no decide sola** un modo que nadie pidio. Lo vigila
`barrera_01_pines_de_luz`.

> 🔴 **La barrera llega hasta lo que el equipo CONTESTA: un `$ACK` que no depende de lo que la llamada devolvio es
> una mentira con formato de exito** — el patron que mas defectos ha dado aqui (`SET_RTC`, `MANUAL:CAMBIAR_TURNO`,
> `SET_TIEMPOS`, `N-151`): la funcion tiene su `if (...) return;` y **el que contesta no lo mira**, asi que el
> tecnico se va del poste creyendo que dejo el reloj puesto. **El molde es `SET_TIEMPOS`: pregunta DENTRO del `if`,
> con un `$ERR` por cada motivo de rechazo.** Un despachador se escribe copiandolo.

⚠️ **Una regla de seguridad que ENUMERA sujetos comprueba que cada sujeto EXISTE**, no solo que nadie la rodea:
nombraba ocho pines donde el firmware mueve seis y el pack no lo veia, porque solo media fugas hacia fuera
(`N-96`). Que pin esta vivo, muerto o libre: `ARQUITECTURA.map` y la spec.

### 3. Cobre, conectores y carga

> **Antes de contestar sobre cobre, conectores, pines o compras se abre la spec, aunque este fichero parezca
> contestar.** `DECISIONES.md` y `05_Funcional/17_...` **ganan a `CLAUDE.md`** en lo decidido y en lo medido.

**Firmware primero; el cableado despues — y el orden es ASIMETRICO.** Retirado el armador de un pin, la placa lo
deja fijado por su pull-down y **un pin en 0 V no ejecuta nada**. Al reves no: con el firmware viejo dentro, ese pin
sigue siendo el boton que EJECUTA y lo que un instalador enchufe puede pulsarlo en un equipo que esta en la calle.
**Un commit no protege de un destornillador: se exige la CARGA VERIFICADA, no el merge.**

**Tres hechos de cobre fabricado, aqui porque su ausencia hiere a una persona:** **`J16` p1 lleva 12 V crudos a un
conector de senal directa al micro y se TAPA en cada equipo que se monte** (`D-4`, `N-120`) · **`J14` es una ENTRADA
del micro** (3,3 V, sin opto ni diodo) y **la salida de talanquera es `J15`**: un rele cableado a `J14` **se
desconecta antes de energizar** · **`J16` p5 y p8 estan VACIOS y NO se cablean, aunque el mando ya salio del
fuente**: el firmware los sigue LEYENDO (`botonArriba()`/`botonAbajo()`), y **un equipo con firmware anterior
a la retirada compone secuencias con un puente**. Un pin no se cierra quitando un consumidor: se cierra
quitando la LECTURA, y **en campo manda lo que esta CARGADO, no el arbol** (`SPEC_5` §3).

**Carga por SWD: `mode=UR` con `-e all`, y no se cambia.** `HOTPLUG` se engancha al micro en marcha, y con un
firmware que se cuelga al arrancar el watchdog reinicia cada 4 s en mitad del borrado (`failed to erase memory`); el
delator es `NVM size: 128 KBytes (default)` en un chip de 64 KB. **Si `UR` falla se reintenta — no se cambia el
modo:** enganchar es cuestion de *timing*, y `Unable to get core ID` no es falta de cableado. **Radios: `2.4 kbps`
de Air Data Rate, `M0`/`M1` en OFF.**

### 4. La compuerta y el banco

```
python 01_Firmware/compuerta.py            # completo (compila) — 0 PASS · 1 FALLA · 2 ABORTADO
python 01_Firmware/compuerta.py --rapido   # sin compilar
python 01_Firmware/Simulaciones/banco/correr.py --pack <nombre>
```

Escribe un acta con fecha y hash de HEAD en `evidencia/`. **Las cifras de los documentos se copian del acta, nunca
se escriben a mano.**

> ⚠️ **LA COMPROBACION DE CIFRAS LEE EL ACTA ANTERIOR —la nueva se escribe al final—, y de ahi salen sus dos
> trampas.** (a) `--rapido` deja un acta **sin las filas de compilacion**, y la corrida siguiente compara contra un
> acta mutilada: **despues de un `--rapido`, la completa se corre DOS veces.** (b) Una fila en `FALLA` deja el acta
> **sin cifra legible** en esa linea y el pack de cifras siguiente **aborta** —`ABORTADO` propaga—: se arregla el
> FALLA y se corre **hasta que dos pasadas den lo mismo**. **La cura nunca es tocar el pack.**

> 🔴 **`correr.py` NO es `compuerta.py`: el banco es UNA fila de veinte.** `correr.py` mide los packs; la compuerta
> mide los packs **Y** los arneses que compilan C++ real **Y** los simuladores **Y** los tests de la app. **Una
> cifra del banco no autoriza un commit — antes de comitear se corre `compuerta.py`, completo.**

**Al escribir un pack:** trae el **bloque literal** de la logica ya probada en vez de reescribirla; relee las
constantes del C++ en cada corrida y **sin valor por defecto, nunca**; y **si dos constantes se relacionan por una
desigualdad, esa desigualdad se recalcula desde el C++**, no se explica en un comentario (`N-71`). Las cuatro
primitivas: `verificar` cuenta · `propiedad` cuenta y marca `ROTA` cuando el banco **logro romper** una regla de
seguridad · `control_negativo` exige que la prueba sepa fallar · `reportar` **no cuenta**, y es donde va el residual
que ningun firmware puede aprobar. **Un instrumento que no esta en la compuerta no mide nada y no deja rastro de que
falta**: un `ABORTADO` grita, un hueco no (`N-43`).

### 5. Los instrumentos leen el fuente por RUTA

Los validadores no incluyen el firmware: lo **parsean**, direccionando cada fichero por tuplas —`("Maestro", "src",
"mando.cpp")`—. **Mover o renombrar un fichero rompe un instrumento**, y el movimiento y la actualizacion de rutas
van en el **mismo commit**, con la compuerta verde antes y despues. Vale igual para un `.md` que para un `.cpp`.

⚠️ **La guarda vigila ficheros que DESAPARECEN; no vigila contenido que se MUDA de fichero.** Sacada una funcion de
`main.cpp` a un modulo nuevo, `main.cpp` sigue existiendo: la guarda no ve nada y el validador que buscaba ese
patron reporta `FALLA` **acusando al firmware de un defecto que no tiene**. La unica red es **comparar el total de
comprobaciones contra el de siempre**. Y no censa la raiz: mover dos `.md` puede abortar el banco en silencio.

### 6. Declarar no es EJERCER

> **Un instrumento verde dice que la DECLARACION esta bien escrita. No dice que nadie la ejerza.**

La forma de defecto mas cara del proyecto, y sale en cualquier lenguaje (`N-117`, `N-118`, `N-122`, `N-124`,
`N-125`). **Las tres preguntas que lo cazan se hacen con `grep` y con el compilador, no leyendo:**

1. **¿Quien LLAMA a esto?** `grep` de la declaracion contra las llamadas. **Trinquete, no absoluto** —hay barreras
   cuya falta de llamador *es* la barrera—: falla una huerfana **nueva**, una que **gana** llamador y sigue en la
   lista, y **una que los documentos anuncien como existente** (`N-73`).
2. **¿Esta guarda puede dar las DOS respuestas?** Un `enum` de un solo valor que se compara compila a `movs r0,#1`,
   medido con `arm-none-eabi-g++ -Os -S`. **Y su simetrica:** antes de borrar el **armador** de una bandera se censa
   quien la LEE y **que pasa si nunca vale `true`** — si de ella cuelgan vetos, borrarlo no los deja inertes: **los
   deja ABIERTOS** (`D-1`).
3. **¿Este fichero se COMPILA en algun sitio?** Antes de la comprobacion numero N sobre la FORMA de un `.cpp`,
   mirese si algun arnes lo enlaza: si no, el pack no mide poco — **mide otra cosa**, porque un pack de texto no ve
   un defecto del TIEMPO. Que arnes compila que, y con que punto ciego: **`ARQUITECTURA.map`**, que se levanta
   midiendo.

> 🔴 **LA EXCEPCION ES EL INSTRUMENTO DE VERDAD.** Cuando un pack esta verde porque una excepcion lo justifica, lo
> que vigila el firmware es **esa frase, y no la comprueba nadie**. Una razon es una AFIRMACION SOBRE EL CODIGO:
> **se mide al escribirla y se vuelve a medir al heredarla** (`N-122`). **Una lista de excepciones con motivos sin
> verificar es una lista de defectos con permiso.**

> 🔴 **Un arnes que no se ha visto fallar es un adorno que da verde.** Antes de conectarlo se **inyecta un defecto
> en el `.cpp` real**, se corre, y se exige que **baje la cuenta y cambie el codigo de salida**; igual tras un
> refactor que mueva la FORMA de un bloque que un pack lee por texto (`N-89`). **La restauracion se hace desde una
> copia tomada ANTES de inyectar (`cp` al scratchpad) y se verifica por HASH:** con trabajo sin comitear, `git
> checkout -- <fichero>` **no deshace la inyeccion — vuelve a HEAD y se lleva tu trabajo por delante**, y `git diff
> HEAD` vacio no lo detecta.

### 7. La regla del instrumento

> **Un "no aparece" no es un hallazgo hasta haber descartado al buscador. Cuando el instrumento y el razonamiento
> no coinciden, manda la medida.**

El buscador ciego nunca es el mismo: un `gcc` que `shutil.which()` no ve, o que compila sin que su `ld` enlace por
una ruta con `n` con tilde (`N-44`); un `grep` que da cero porque el formato separa con tabulador; el mensaje de un
commit, que describe la INTENCION y no el alcance; una captura al unico ancho donde el fallo no sale.

1. **Un cero de `grep` no es «no hay»: es «mi patron no encontro».** Antes de publicar un «no existe» se busca por
   **los dos nombres posibles** —el setter generico y la funcion propia— o al reves, **por quien la APAGA**. Y **el
   patron cuenta comentarios**: aqui los comentarios citan lo que explican, y un recuento de llamadas sale inflado
   si no se filtran.
2. **Cuando el sintoma trae un NUMERO, ese numero se busca en el fuente ANTES de la primera hipotesis** (los «15
   segundos» de un reporte eran `tiempoDespejeMs = 15000`, literal): cinco hipotesis plausibles costaron media
   sesion y el `grep` costaba diez segundos.
3. **Se cita el SIMBOLO, no el numero de linea** —un numero caduca solo, en silencio y con autoridad de dato—. Solo
   vale **fechado a un commit** (`git show <hash>:fichero`) o **pegado como salida literal de un `grep` corrido
   antes de publicarlo**. El ancla barata ya existe: el firmware lleva marcas `N-xxx` en sus comentarios, y `grep
   -rn "N-133" 01_Firmware` da las cuatro puntas de un cambio de una vez.
4. **Un informe —propio, de un agente, o una REFUTACION— no es una medida.** Se reproduce y se pega la salida;
   tachar exige el mismo rigor que afirmar. Una causa que se cae **se marca refutada, no se borra**: la que
   desaparece en silencio vuelve a proponerse, y la segunda vez nadie recuerda que se comprobo.
5. **Descartar por eliminacion solo vale si las opciones son exhaustivas**, y una biseccion necesita un extremo
   bueno **VERIFICADO**. El sospechoso se elige por **ficheros** —`git log --oneline <bueno>..HEAD -- <los del
   camino que falla>`— y se comparan **hashes, no tamanos**.

> 🔴 **La FOTO DE CAMPO TAMBIEN ES UNA MEDIDA, y es la unica tomada sobre el aparato real: cuando chocan, el
> instrumento es el sospechoso.** Y cuando un instrumento compara contra un borde, un umbral o una lista, **se
> escribe al lado CUAL es ese borde y por que es el correcto**: los censos que fallaron lo hicieron todos en la
> misma frontera —lo que decidieron no mirar— y ninguno lo llevaba escrito. ✅ **Antes de llamar «defecto de
> hardware» a una medida, mirese que firmware estaba dentro cuando se tomo:** a veces el banco ya corrio el control
> negativo sin saberlo.

### 8. Lo que YO produzco es un instrumento

> **Antes de escribir un pack, un informe, un menu de opciones o un encargo a un agente: «¿esto acerca una tarjeta
> cargada, o la sustituye?»**

Dos auditorias externas lo llamaron *industria de sustitucion* —la segunda, de la respuesta a la primera: *«se
arreglo todo lo que la auditoria midio y nada de lo que dijo»*—. **La medida se recalcula, no se recita:** lineas de
`{Maestro,Esclavo,Repetidor}/{src,include}` frente a `Simulaciones + Validacion_* + compuerta.py`.

1. **Un pack que certifica otra vez lo ya certificado sustituye.** La excepcion legitima es el que **desbloquea**
   algo parado o mide una **propiedad de vida que nadie ejercia**: eso es firmware por otro nombre. No lo decide el
   fichero que toca, sino si **contesta una pregunta abierta**.
2. **Un menu de opciones sobre una causa sin medir tiene mas autoridad que un dato**, porque parece que ya se
   investigo. **Si hay que preguntar sin haber medido, se dice en la pregunta**, y cuando el banco tumba una
   decision del responsable se le devuelve **con la medida**, no se ejecuta igual (`N-142`). **«El responsable lo
   decidio» NO es cobertura:** una decision tomada sobre un informe malo hereda el error **y ademas lo blinda**.
3. **Un encargo ejecuta la spec, no una frase.** Lo dicho de viva voz describe una INTENCION; lo escrito, una
   DECISION TOMADA con su motivo: **cuando chocan se PREGUNTA** —cuesta un `grep` a `DECISIONES.md`—, y con mas
   razon si el cambio **RETIRA una barrera**: un alcance que crece se corrige despues; una proteccion amputada no se
   nota hasta que alguien esta en la calzada.

> 🔴 **Delegar amplifica el error: el trabajo delegado se revisa por el DIFF, no por su informe — Y SOBRE TODO
> CUANDO EL NUMERO SALE VERDE, que es cuando no apetece.** §1 ya prohibe decorar un rojo y aun asi paso, porque **el
> que decora es el que informa** y nadie audita un `20/20`. **El verde no es el permiso para no mirar: es la senal
> de que toca mirar.** Que se mira: si un cambio toca **el firmware y su modelo a la vez**, si el modelo *replica*
> el arreglo o *relaja* la comprobacion —solo lo primero vale—; **un `grep` por afirmacion** contra el informe
> (funciones, literales y pines que dice que existen); y un **`sha256sum`** al binario que se declara recompilado.
> Las dos ultimas no las ve la compuerta. **Y una variable que contesta a dos preguntas distintas no puede contestar
> bien a ninguna: son dos banderas.** El sintoma de que se cruzo la linea es **entregar documentos cuando lo pedido
> era un entregable**: un parte con cifras verdes puede ser cierto en cada linea y falso en conjunto.

### 9. Al arreglar un defecto, busca las pruebas que lo CELEBRABAN

**Un banco maduro contiene pruebas que EXIGEN el comportamiento defectuoso** (`N-49`): se escribieron cuando el
defecto se creia inevitable. Reescribirlas en bloque hasta que pasen es **ajustar el instrumento hasta que de
verde**. Van una por una, y **primero se cuenta cuantas propiedades afirma cada una**, porque casi ninguna afirma
solo una (`N-83`). Cuatro destinos: **se REPARTE** —el habitual: la mitad del defecto se invierte donde estaba y la
que sigue valiendo **se MUDA con su bloque literal** al escenario donde si se cumple— · **se INVIERTE** para exigir
lo nuevo · **se CONSERVA** si media otra cosa · **se BORRA** si solo documentaba el defecto.

**El escenario nuevo no es relleno: es el control que le falta a toda inversion** —una guarda que no dejara pasar
**nada** haria pasar las lineas invertidas igual de bien que la correcta—. Y 🔴 **una inversion que solo mira el
RESULTADO aprueba un firmware con las barreras en el ORDEN equivocado**: inyectado el defecto, «no sale ninguna
trama» **no cayo** —otra barrera mas abajo frenaba el envio— y solo cazaron la regresion las lineas que miran el
orden. **El mejor termometro son los fallos que desaparecen solos**; una linea nueva que no puede fallar sin que
falle la de arriba es adorno.

### 10. Flash, RAM y cotas de buffer

64 KB por micro, y **el margen del Maestro se mira en el acta antes de estimar un coste — no se recita.** La cifra exacta sale de la
ultima acta (`ls -t evidencia/*_compuerta.txt | head -1`) y **no se copia aqui**: un umbral no caduca, una cifra si.

- **Antes de sacrificar una funcion porque «no cabe», MIDE DE QUE ESTA HECHO ese porcentaje** (`N-70`): **por
  FICHERO OBJETO leyendo `firmware.map`, NO por nombre de simbolo** —los simbolos de cualquier libreria C++ tambien
  empiezan por `_Z`—; el `.map` trae la seccion que dice **quien arrastra a quien**; y **un delta exige medir los
  DOS extremos**.
- **Un camino muerto que no cuesta flash puede seguir costando RAM** (`N-86`): el enlazador descarta funciones, pero
  **no un objeto global** —su constructor vive en `.init_array` y su `.bss` es permanente—. **La flash se mide con
  la compuerta; la RAM, con `nm` sobre el `.elf`.**
- ⚠️ **El acta puede publicar el binario ANTERIOR** cuando PlatformIO sirve un incremental viejo — pasa siempre que
  se intercambian fuentes o dos agentes tocan el arbol a la vez. **La cifra de flash se confirma con una segunda
  pasada; si no coinciden, manda la segunda.**
- **La cota de un buffer es el BUFFER de cada campo, no su tipo ni su rango** (`N-154`): es lo unico que `snprintf`
  garantiza sin fiarse de otro fichero. **Se acota donde se produce; no se agrandan buffers**, y **las cotas se
  DERIVAN de la constante que manda**. Un valor fuera de cota se publica como `!` —nunca `--`, que ya significa
  *«todavia no lo se»*—. Una trama truncada sale bien formada hasta la mitad, no casa el CRC, y el sintoma es «el
  equipo se callo», que manda a mirar el cable.
- **Nada de clases con metodos virtuales:** las vtables cuestan flash que no hay. Un `.cpp` por concepto, `static`
  para lo privado y header corto.

### 11. Trabajo en paralelo, y el INDICE antes del commit

**Un `git add -A` barre lo que otro agente tiene en vuelo, y la historia queda mintiendo:** el contenido puede
quedar correcto y la compuerta verde, pero un `revert` de ese commit no deshace nada.

- **Dos agentes a la vez: cada uno en su `git worktree`, o `git add` de rutas explicitas. Nunca `-A`.** El arbol
  compartido no avisa: mezcla.
- 🔴 **`git add <valido> <ignorado>` devuelve codigo distinto de cero PERO DEJA LOS VALIDOS YA PREPARADOS**: con
  `add ... && commit` el `&&` corta, el commit bueno no corre y **se los lleva el commit SIGUIENTE**. 🔴 **Y la
  contraria, que engana por serlo: `git add <valido> <ruta_que_NO_EXISTE>` no prepara NADA**, ni las rutas validas
  de esa misma orden, y el commit sale con **un mensaje que describe cambios que no lleva dentro**. **La regla que
  cubre las dos: se comprueba el INDICE antes de comitear —`git diff --cached --name-only`—, no el codigo de salida
  del `add`; y no basta imprimirlo: hay que LEERLO y contar las lineas que esperas.**
- 🔴 **Al editar con un script, `open(ruta, "w")` TRUNCA ANTES de escribir:** si el `.write()` falla despues —un
  `UnicodeEncodeError` por un emoji— **el fichero queda vacio y el error parece de lectura**. Se escribe a un
  temporal y se renombra, o se usa la herramienta de edicion. **Comitear antes de un script masivo es la red.**
- 🔴 **Y SU HERMANO: UN CORTE QUE BUSCA «EL SIGUIENTE CIERRE» SE LLEVA EL BLOQUE DE AL LADO.**
  Medido el 14/09: al retirar una entrada de un diccionario en `app.js`, el script corto desde su ancla hasta el
  proximo `},` —y ese `},` era el de la entrada SIGUIENTE, que desaparecio entera—. El fichero quedo valido,
  las cuatro copias identicas y la app arrancando: **lo cazo un pack en la corrida de despues, no la sintaxis.**
  **Un borrado se delimita por SUS DOS extremos leidos, nunca por «el siguiente delimitador»**, y despues se
  cuenta: si el fichero pierde mas lineas que el bloque, se ha llevado algo mas.
- 🔴 **Un `git worktree remove --force` SIGUE los enlaces que el agente dejo dentro y borra el ORIGINAL** —un
  *junction* a `node_modules` se llevo el de verdad y dejo dos `ABORTADO`—. **Antes de retirarlo se censan los
  enlaces y se quita el ENLACE, no lo que apunta** (`Delete(ruta, false)`), se comprueba el destino, y despues el
  resto. Al encargar: que lo retire el agente.
- 🔴 **Y AL REVES: un agente REANUDADO puede acabar escribiendo en el ARBOL PRINCIPAL sin decirlo.** Su worktree se
  retira solo cuando termina **sin tocar nada** —justo lo que hace el que PARA y devuelve el encargo—; al reanudarlo
  ya no existe y escribe donde puede. **Lo unico que lo hace inofensivo es comitear con rutas explicitas.** Antes de
  integrar por parche se comprueba que el worktree existe —`git -C <ruta> diff` falla en seco si no—; si no existe,
  el trabajo esta en el arbol principal.
- **No se reescribe la historia publicada para arreglarlo:** con la rama en dos remotos y otro agente encima, un
  `push --force` dana mas de lo que repara. Se anota donde vive el cambio y se sigue.

**EL ORDEN DE LANZAMIENTO, que es lo que impide el bucle de hacer / deshacer / rehacer:**

1. **Antes de lanzar se abre `DECISIONES.md`.** Si el encargo contradice una fila, **eso no es una orden: es una
   pregunta** (casi siempre la frase nueva resume mal algo mas pequeno). 🔴 **Y al reves, que es el bucle: un manual
   o una guia que contradice una fila NO la reabre.** El documento esta caducado y se corrige HACIA la decision;
   reabrir una decision cerrada exige una fila nueva del responsable. Los documentos congelan el dia en que se
   escribieron, y un agente los lee con la misma autoridad que la tabla.
2. **Los que MIDEN van antes que los que ESCRIBEN:** auditoria de solo lectura -> veredicto en el scratchpad ->
   ejecucion **con ese veredicto delante**. Delegar **amplifica** el error.
3. **Un agente = un conjunto de ficheros DISJUNTO, nombrado explicitamente**, prohibidos incluidos. El limite del
   paralelo es el fichero, no la capacidad.
4. **El encargo dice contra que `D-x` trabaja**, y lleva: **si el codigo y su `D-x` no coinciden, para y reporta —
   NO elijas.** Ahi nace el bucle: uno lo arregla en un sentido y el siguiente en el contrario.
5. **Ningun agente comitea.** Comitea el orquestador, con rutas explicitas y **leyendo** el indice.
6. **A cada agente se le pide que DUDE del encargo.** Es la frase que mas rinde.
7. 🔴 **UNA DECISION QUE RETIRA O DEROGA UNA BARRERA SE REVISA ANTES DE CONSTRUIRLA, NO DESPUES** — y el revisor
   busca **el modo de fallo que la decision CREA**, no si la decision es buena: eso ya lo decidio el responsable.
   ⚠️ **Y donde mirar primero, que es lo que rinde: LA PREMISA QUE LA DECISION CONTRADICE SUELE ESTAR ESCRITA EN EL
   FUENTE, y es el argumento en contra.** El 14/09 una decision sobre la barrera se paro asi: `semaforo.cpp` llevaba
   escrito *«una pluma arriba con la luz en rojo es PEOR que no tener barrera, porque el conductor confia en ella»*,
   y **nadie habia ido a buscarla**. Cuando aparece hay dos salidas y las dos valen: el responsable **cambia la
   premisa** —y el comentario se corrige HACIA la decision (§11.1), con la derogacion escrita y con su nombre— o
   **la premisa aguanta y la decision se replantea**. Lo que no vale es construir encima sin haberla leido.
8. **La compuerta la corre el orquestador con el arbol QUIETO.** Una cifra medida mientras otros escriben no vale.

### 12. Donde esta cada cosa

| | |
|---|---|
| 🔴 **`05_Funcional/SPEC_0..SPEC_8`** | **MANDA, Y SOBRE EL FIRMWARE TAMBIEN** *(responsable, 14/09: «ahora es lo que digan las spec, y eso debe hacer el firmware»)*. Es el documento DEL PRODUCTO: lo que el equipo HACE. **Si el firmware y la spec no coinciden, el defecto es del FIRMWARE** — salvo que la spec prometa algo que nadie decidio, y entonces es la spec la que miente y se corrige (§1). Se parte de aqui para exportar |
| `DECISIONES.md` | **el ANDAMIO: como se llego.** ~~GANA a este fichero~~ -> **DEJA DE GANAR el 14/09** *(«el archivo decisiones marea»)*. Sigue sirviendo para UNA cosa y solo una: **que no se le vuelva a preguntar al responsable lo que ya contesto**. Se lee antes de lanzar un agente por eso, no porque mande |
| `05_Funcional/17_Arquitectura...md` | 🔴 **las medidas de cobre, con fecha, instrumento y firmware que habia dentro. GANA a este fichero en todo lo que sea hardware medido** |
| `ARQUITECTURA.map` | que ficheros abre cada instrumento, que mide cada fila de la compuerta, que hay en cada conector |
| `roadmap.md` · `roadmap_hist.md` | el **porque**: cada `N-x` con su cronica. Se busca **por `N-x`**, no por fichero: el corte entre los dos se mueve |
| `ESTADO.md` | donde esta parado el trabajo **hoy** · `evidencia/`, las actas con fecha y hash: **la fuente de toda cifra** |
| `OPTIMIZACIONES.md` | las reglas `SFTY-x` y la trazabilidad regla -> codigo -> prueba |
| `01_Firmware/compuerta.py` | **la unica forma correcta de verificar** · `Simulaciones/banco/`, packs y modelos · `Validacion_*/`, los arneses que compilan C++ real |
| *(la spec)* | cada afirmacion validada contra el fuente; lo que NO se cumple vive en su apartado «HUECOS MEDIDOS», **nunca escrito como si existiera** |
| `04_Manuales/`, el resto de `05_Funcional/` | manuales y protocolos para el tecnico y el auditor |

### 13. Convenciones

- **Comentarios y mensajes de commit en espanol, en ASCII sin acentos** (la consola de Windows viene en cp1252 y los
  validadores parsean el fuente).
- Los comentarios explican **por que**, no que. **Y si un comentario delibera** —*«Wait, in this state...»*, *«let's
  just force...»*— quien lo escribio no lo tenia claro y lo dejo asi: dentro de una regla de seguridad eso no es una
  nota, es una alarma. La duda se resuelve o se anota en `roadmap.md`.
- **Un commit = un cambio con sentido propio = un `git revert` limpio**, y cada `N-x` se cierra con la evidencia que
  lo demuestra, no con una afirmacion.
- Si un pack ejerce una regla `SFTY-x` se marca `# EJERCE SFTY-x: <que>` en su cabecera, y la tabla de trazabilidad
  se levanta buscando esa etiqueta. **Solo se etiqueta lo que el pack comprueba de verdad:** una regla cubierta por
  una prueba que no la ejerce es peor que una fila vacia — la vacia no miente.
- 🔴 **Un binario que sale de aqui se nombra `<producto>_<fecha>_<hash>_SIN_BANCO`, y el sufijo NO se quita al
  renombrar: lo quita quien lo haya probado en un equipo.** Es la unica marca que viaja PEGADA al fichero —los
  `.apk` estan en `.gitignore`, git no vigila esto y hay que mirarlo en el disco—, y **su ausencia se lee como
  permiso**. Un binario nuevo se acredita con su `sha256sum` contra el anterior, **nunca con su tamano**.

### 14. Lo que nadie recalcula, envejece — y eso incluye las LISTAS

> **Una LISTA DE ALCANCE es una afirmacion sobre el arbol, y caduca igual que una cifra. Se RECUENTA con `grep`
> antes de ejecutarla; leerla es creersela.**

**Se busca por el NOMBRE Y POR SU CONSECUENCIA:** un patron que persigue la constante no ve lo que se DEDUJO de ella
—la cifra derivada en lineas que no la nombran, la palabra escrita a medias—, y hacen falta varias barridas porque
cada una ve lo que la anterior no podia. Es §7.1 donde mas cuesta: el alcance de un cambio ya aprobado. ⚠️ **Y la
lista se queda corta SIEMPRE por el mismo sitio: lo que no es firmware ni documento.** Antes de cerrar un cambio de
constante: **¿quien mas la RECITA?** — la app, un `.html`, un manual, el LEEME de un `.zip`.

> 🔴 **UNA CIFRA CADUCADA SE SUSTITUYE; UNA CONTRADICCION HAY QUE REESCRIBIRLA — y el `sed` no sabe la diferencia.**
> Al copiar cifras de un acta, un `sed` deja el numero bueno **dentro de una frase que ahora miente**. **Despues de
> sustituir cifras se BUSCA EL VEREDICTO que las acompanaba** —`FALLA`, `sigue`, `espera`, `abierto`, `la que cae`—
> y se mira si sigue siendo cierto. Igual con un ROTULO: cuando el sujeto muere el nombre sobrevive y **manda a
> construir lo que ya no es**.

> 🔴 **UN NUMERO EN UN SITIO QUE NO PUEDE RECALCULARLO NO SE SINCRONIZA: SE RETIRA.** `app.js` no puede derivar de
> una constante del C++ —otro lenguaje, otro binario, ningun instrumento cruza los dos—: toda cifra copiada ahi
> **nace caducada**. Se cambia por una frase que **no envejece**. Sincronizar a mano es firmar que alguien se
> acordara la proxima vez, y **nadie se acuerda**. La version operativa —lo que cuesta un cambio de constante en el
> momento de hacerlo— esta en **§15**.

⚠️ **El fichero que editas puede tener COPIAS.** `app.js` vive **cuatro veces** en el arbol —`www/`, la raiz de la
app, `android/assets/public/` y la de `build/`— y editar una deja tres mintiendo. **Antes de compilar, `md5sum` de
las tres primeras: identicas o no se compila.**

### 15. El orden de prioridad del trabajo

Lo fija el responsable, y decide que se toca primero cuando dos cosas compiten:

1. 🔴 **LA SPEC — `05_Funcional/SPEC_0..SPEC_8`, Y MANDA SOBRE EL FIRMWARE.** El responsable lo fijo el 13/09
   —*«si manana exportamos este semaforo, partimos de los MANUALES y de la SPEC»*— y lo cerro el 14/09: **«ahora es
   lo que digan las spec, y eso debe hacer el firmware»**. Con `05_Funcional/17_...`, que gana en cobre medido.
   ⚠️ **Y por eso una spec que promete lo que el equipo no hace ya no es solo un documento malo: es una ORDEN
   FALSA.** El 14/09 hubo **ocho** afirmaciones falsas vivas a la vez en seis documentos —cuatro sobre el retardo de
   la barrera, tres negaciones de algo construido y una constante que un comentario del propio firmware negaba—, y
   **ninguna la caza la compuerta**: las cazaron un revisor y dos agentes leyendo. Esa es la deuda de instrumento.

> 🔴 **COMO SE RESUELVE UN CHOQUE, Y ES UNA SOLA FRASE DEL RESPONSABLE (14/09):** *«si la spec no pasa, si el
> arquitecto contra decisiones, si no le gusta algo — se AJUSTA LA SPEC, que es lo que debe implementar el
> firmware»*. O sea: **un choque NO se discute y NO deja dos versiones vivas. Se arregla en la spec, y despues el
> firmware la sigue.** Vale igual si quien objeta es un revisor de arquitectura, si `DECISIONES.md` dice otra cosa,
> o si al responsable no le convence: **la salida siempre es la misma puerta.**
>
> 🔴 **Y AQUI ESTA LO QUE IMPIDE QUE LA SPEC SE CONVIERTA EN UNA LISTA DE DESEOS — LOS DOS REGISTROS, QUE NO SE
> MEZCLAN NUNCA:**
>
> | | que es | como se sujeta |
> |---|---|---|
> | **lo que el equipo HACE** | descripcion | **se valida contra el fuente**, frase por frase. Si no se cumple, la spec MIENTE y se corrige la spec (§1: no se decora) |
> | **lo que el equipo DEBE hacer** | la ORDEN al firmware | vive en el apartado **«HUECOS MEDIDOS»** de su spec, y **ese apartado deja de ser una confesion: es la COLA DE TRABAJO del firmware** |
>
> **Una frase que no diga claramente en cual de los dos esta, esta mal escrita.** Es exactamente el fallo que costo
> el dia: cuatro documentos decian *«no esta implementado»* —registro 2— sobre algo que YA estaba construido
> —registro 1—, y quien lo leyera iba a construir dos veces lo mismo. **El tiempo verbal no basta: se marca.**
2. **Los ARNESES y los instrumentos** — compuerta, packs, simuladores. Lo que MIDE la spec.
3. **Los manuales y la documentacion** — `04_Manuales/`, el resto de `05_Funcional/`, los `.docx`.

**El motivo: las spec cambian a diario y son la base; los manuales congelan el dia en que se escribieron, y
reescribirlos en cada vuelta es donde se va el tiempo — y donde se deja de hacer codigo.**

> 🔴 **DONDE SE ESCRIBE CADA COSA, Y ESTO CORRIGE UN VICIO MIO QUE EL RESPONSABLE TUVO QUE SENALAR DOS VECES.**
> `DECISIONES.md` **NO es la spec: es el ANDAMIO con el que se construyo**, y nacio por un motivo concreto —
> *«se creo porque cada nada empezabas a preguntar una y otra vez lo mismo»*—. De ahi las dos reglas:
>
> - **En la fila va SU RESPUESTA, en dos lineas, y lo que deroga. NADA MAS.** Ni como se midio, ni que commit lo
>   construyo, ni que se refuto: eso es CRONICA y su sitio es `roadmap_hist.md`. Una fila que crece hasta las
>   5.000 caracteres —y las hay— **ha dejado de cumplir su unica funcion**, que es contestar de un vistazo para
>   que nadie vuelva a preguntar.
> - **EL COMPORTAMIENTO VA A LA SPEC.** Si la spec ya lo dice, **no se repite en la fila**.
>
> 🔴 **Y LA SPEC TIENE QUE ESTAR COHERENTE E IMPLEMENTADA**, que es la mitad que se olvida: una spec que promete
> lo que el equipo no hace es peor que no tenerla, porque el que la exporte firma algo falso. **Cada afirmacion se
> valida contra el fuente** y se clasifica en las cuatro de §6: implementada y ejercida · implementada sin
> ejercer · **NO implementada** —esa va al apartado «HUECOS MEDIDOS» de su spec, nunca escrita como si existiera
> (§1)— · no comprobable desde el fuente.

> 🔴 **UN CAMBIO DE UNA CONSTANTE, UN FLANCO O UN UMBRAL CUESTA DOS EDICIONES: la fuente de verdad, y el instrumento
> que la RECALCULA. Nada mas.** Si un documento RECITA ese valor, la respuesta correcta **no es actualizarlo: es
> RETIRARLE el numero** y dejar la consecuencia en palabras, que no envejece (§14). Actualizar el documento compra
> un dia y vuelve a caducar al siguiente; retirarlo lo arregla para siempre.
>
> **Y el corolario, que es el que corta el barrido: una decision que TODAVIA PUEDE CAMBIAR no se propaga a los
> manuales.** Se tocan cuando el sujeto esta quieto: propagar una decision fresca a once documentos es apostar a que
> no cambia, y aqui cambia. Esto NO deroga que una decision no esta puesta hasta que llega al manual (§11.1): dice
> **cuando** se hace ese viaje, no si se hace.

> 🔴 **REGLA DEL RESPONSABLE (12/09): LOS MANUALES NO SE TOCAN HASTA QUE TODO ESTE CERTIFICADO.**
> Ni se escriben, ni se actualizan, ni se barren. **UNICA EXCEPCION: los `.html` de campo** —la guia
> de camaras y la de cableado—, **que son los que llegan al poste y se corrigen siempre**, porque un
> instalador los sigue manana con un destornillador en la mano.
>
> **El motivo, medido: 17.000 lineas en seis manuales** (`1_`, `2_`, `3_`, `9_`, `14_`, `18_`)
> **describiendo un sistema que ha cambiado cuatro veces desde que se escribieron.** Reescribirlos
> antes de que el sujeto este quieto es tirar el trabajo dos veces: una al escribirlo y otra al
> descubrir que ya no aplica. **Certificado quiere decir: banco pasado y tarjeta cargada** (§0.2),
> no compuerta verde (§0.3).

> 🔴 **Y COMO SE ESCRIBE UNA SPEC, que es lo que sustituye a esos manuales.** Tres reglas, las tres medidas el 12/09
> al escribir las cinco primeras:
>
> 1. **Una spec se escribe desde `DECISIONES.md` VIGENTE y desde EL FUENTE. NUNCA desde un manual anterior.**
>    Copiar un manual propaga lo caducado con aspecto de revisado — y ese es justo el bucle que la spec viene a
>    cortar. El manual viejo se ABRE para censar que cubre; **ninguna frase suya entra sin verificarla contra el
>    codigo**. Una `A-x` abierta se nombra como HUECO, no como comportamiento.
> 2. **Un requisito = una spec, y por debajo de 300 lineas.** Si no cabe, sobra prosa. **Y el reparto nombra
>    EXPLICITAMENTE lo que queda sin dueno**: partir un sistema en cinco specs deja huecos entre ellas, y un hueco
>    que nadie nombra no lo cubre nadie.
> 3. 🔴 **LA CADUCIDAD DE UN DOCUMENTO SE MIDE POR AUSENCIA, NO POR OPINION — y esto es lo que la hace
>    incontestable.** No se discute si un manual esta viejo: se cuenta **que NO menciona** de lo que el firmware
>    hace hoy. *«`10_Manual_Bluetooth` tiene CERO menciones de `CANCELAR_AMBAR`, que existe desde el 31/08, y CERO
>    de `LATIDO`, desde el 04/09»* no admite replica; *«ese manual esta desactualizado»*, si.
>    ⚠️ **PERO UN CERO SIGUE SIENDO §7.1 — «mi patron no encontro», no «no hay»— y aqui muerde el doble, porque
>    el cero se usa para TIRAR un documento.** Medido el 12/09: `«14_Manual_App tiene CERO menciones de CAM:»` era
>    cierto **y enganaba**, porque ese manual escribe `CAM` sin los dos puntos y resulta ser **el documento mas
>    completo del repositorio sobre las camaras en la app**. El criterio habria tirado justo lo que decia que
>    faltaba. **Antes de jubilar por ausencia se busca por los DOS nombres posibles y se mira que hay dentro.**


### 16. Limpiar es parte del trabajo, y se MIDE

> **Este repositorio no se degrada por lo que se escribe mal: se degrada por lo que se anade y nadie
> retira. Del 28/08 al 05/09, `CLAUDE.md` crecio 869 lineas contra 36 borradas y NINGUN commit retiro
> nunca un apartado. `roadmap.md` volvio de 1.093 a 2.695 en cinco dias. Es el mismo mecanismo las dos
> veces: se INCREMENTA sin releer.**

**LA MEDIDA, y se RECALCULA antes de cada tanda — no se recita** (§8): lineas de
`{Maestro,Esclavo,Repetidor}/{src,include}` **frente a** `Simulaciones + Validacion_* + compuerta.py`.
El 12/09: **18.858 contra 53.771, o sea 2,85 a 1** —el `.map` la midio en 2,90 con otro corte—, y en los
cinco dias anteriores **el aparato de medir crecio ~12.000 lineas y el firmware CERO**. Cuando esa
division sube y el firmware no se mueve, **lo que hay no es rigor: es sustitucion**, y se corrige
cerrando el editor de `.md`.

> 🔴 **Y LA MEDIDA DEJA DE SER UN PROPOSITO Y PASA A SER UNA PUERTA (14/09).** Medido sobre los ultimos
> 50 commits: **firmware +6.454/-2.834, INSTRUMENTOS +18.470/-2.020, documentos +9.039/-6.096.** O sea que
> **el aparato de medir crecio al TRIPLE que lo que mide**, y eso —no los `.md`— es lo que el responsable
> siente como bucle: *«me siento en un bucle, desde hace 50 commits nada»*. **La regla: un pack nuevo entra
> si CONTESTA UNA PREGUNTA ABIERTA o DESBLOQUEA algo parado; si solo certifica otra vez lo ya certificado,
> no entra — y eso ya lo decia §8.1, lo que faltaba era CONTARLO.** Antes de anadir un instrumento se dice
> en el commit **que pregunta abierta contesta**. Si no hay pregunta, es sustitucion.

**LAS TRES REGLAS QUE LO SUJETAN:**

> *[lineas 500-502 del original: borradas sin mover, las dice el metodo global. Ver §4]*
2. 🔴 **NINGUN FICHERO BASE PASA DE 1.000 LINEAS.** Base es lo que gobierna: `DECISIONES.md`,
   `OPTIMIZACIONES.md`, `05_Funcional/17_...`, `CLAUDE.md`, `roadmap.md`, `ESTADO.md`, `ARQUITECTURA.map`,
   `README.md`. **Lo que pasa se PARTE, no se reescribe**: lo vivo se queda, la cronica se muda literal a
   su `_hist`, y **el historico publica las dos cuentas** para demostrar que no se borro nada. El molde es
   `roadmap.md`, 2.695 -> 443 + historico.
3. 🔴 **UN SUJETO MUERTO SE MATA EN EL INSTRUMENTO, NO EN LA MEMORIA.** Cuando el responsable retira algo
   —el mando, las LCD, una compra ya hecha—, la decision lo mata en UNA fila y el sujeto sigue vivo en
   diez documentos. **Va a `documentos_06_no_reabre_lo_cerrado` el mismo dia**, con la frase literal que
   lo mata. Acordarse no funciona: nadie se acuerda, y el sintoma es volver a proponerle al responsable
   algo que el ya cerro.

⚠️ **Y el archivado tiene DOS destinos que no son lo mismo, asi que se elige a proposito:**
`05_Funcional/historico/` **sigue dentro de git** —muerto pero consultable, y un clon nuevo lo trae—;
`99_Legacy/` **esta FUERA de git** por su propio `.gitignore` —un clon nuevo ya no lo trae—. Y ojo con la
mecanica: **`git mv` a `99_Legacy/` los deja SEGUIDOS en la ruta nueva** —fuerza el `.gitignore`— y el
repositorio sigue cargando los bytes, que es lo contrario de jubilar. Se mueve en disco y se registra la
**BAJA**: el indice tiene que salir con lineas `D`, nunca `R`.

### 17. Un ABORTADO del worktree no es una medida, y una fila del roadmap no es una causa

**Dos cosas que en una tanda con agentes en paralelo se cuelan solas, y las dos hacen tomar por medido lo que no lo esta.**

1. 🔴 **UN WORKTREE DE AGENTE NO TIENE `node_modules`, asi que su compuerta ABORTA dos filas** —«app ejecutada
   en DOM» y «simulador del puente ESP32»— **y el puente es justo el arnes que compila `coordinador.cpp` real.**
   Un agente honesto lo reporta como esperado; el error es del orquestador si lo acepta como cobertura. **La
   compuerta que AUTORIZA el commit se corre en el arbol principal, con el arbol quieto, y ahi esas filas no
   abortan.** No se crean junctions a `node_modules` para taparlo (§11: un `worktree remove --force` sigue el
   enlace y borra el original).
2. 🔴 **UNA FILA DEL ROADMAP QUE NOMBRA EL MECANISMO ES UNA HIPOTESIS, NO UNA CAUSA — y el 15/09 fallo DOS
   veces en la misma sesion**: 1.39 culpaba a los reintentos agotados cuando el hueco lo abrian las ANCLAS del
   silencio, y 1.49(b) apuntaba al Maestro cuando el defecto estaba en el ESCLAVO. Las dos se escribieron
   derivando de las constantes, sin ejercer. **Antes de construir contra una fila se re-mide el mecanismo en un
   arnes que ejecute las dos puntas; si cae, se marca REFUTADO dentro de la fila** (§7.4) **y el orden por dano
   se rehace con la medida** —lo que estaba enterrado como nota puede ser lo peor de la lista—.

> ⚠️ **Y lo que ordena el trabajo con eso: MEDIR ES BARATO Y CONSTRUIR SOBRE UNA FILA MALA CUESTA EL DOBLE**,
> porque el arreglo hay que deshacerlo y el instrumento que lo vigila nace midiendo otra cosa. **El agente que
> mide va antes que el que escribe y NO toca el arbol** (§11.2); su veredicto se guarda en el scratchpad y el
> que construye lo lleva delante.

## 2. De `ESTADO.md` (`126d5eb`, 722 lineas): lineas 34-545 y 562-722

> La linea 25 del original (fila de la fase 2, estado `pendiente`) se actualizo en sitio; no es cronica.


> ## PUNTO DE CONTINUACION ANTERIOR — 16/09/2026 (cronica)
>
> 🚦 **SE ESPERAN LAS TRAMAS DEL FUNCIONAL.** Se le mando
> **`Paquete_Banco_2026-09-16_1889631_SIN_BANCO.zip`** (sha256 `59643817`) con la APK
> `IOT_VIAL_Semaforos_2026-09-16_1889631_SIN_BANCO.apk` (`1e2080d4`). Compuerta **18 PASS /
> 1 FALLA / 0 ABORTADO**, banco con **un solo rojo** —es `D-22` y **necesita una
> tarjeta delante**, no se cierra con teclado—.
>
> **LO PRIMERO QUE HAY QUE HACER CON ESAS TRAMAS, en este orden:**
>
> 1. 🔴 **Pedir la version ANTES de leer nada** (boton *Consultar version del firmware*).
>    Si contesta `SIN_SELLAR` con `FW:--`, el binario se compilo desde el `.zip` —ahi no hay
>    `git`— y entonces **el hash es `1889631` por declaracion, no por medida**. Sin eso, la foto
>    no se lee (`CLAUDE.md` §0.2).
> 2. 🔴 **Repetir las tres que el 16/09 NO quedaron medidas**, porque cayeron dentro de una
>    ventana sin enlace y **no dicen nada del firmware** (§1): `SET_MODO:INTELIGENTE`,
>    `AMBAR_EMERGENCIA` y `REINICIAR_RELOJ` + `CONSULTA RELOJ`. Comprobando antes que entra
>    `$STATUS`. El fuente los tiene habilitados: solo se rechazan desde `DEGRADADO`.
> 3. ⚠️ **Leer el RESUMEN del diario buscando `ESCRITAS SIN ENLACE`.** Es nuevo (1.50): esas
>    ordenes se escribieron a un cable muerto y **no son un rechazo del equipo**. El 16/09 hubo
>    **24**, seis de ellas `FORZAR_ROJO`.
> 4. ⚠️ **La cinta se recorta a 300 tramas y el 16/09 tiro 483.** Si la sesion es larga, exportar
>    por tramos o subir el tope antes de empezar.
>
> **DOS COSAS QUE NO SE PUEDEN ACHACAR AL FIRMWARE:**
>
> - 🔴 **El `Y2` de 32,768 kHz del Maestro `179DB0` NO OSCILA** —`ON:1 RDY:0 BYP:0 SEL:0
>   EN:0 CNT:--`—. Es **soldadura** (`C-6`). Cualquier fallo de hora en ESA tarjeta es esto, y
>   `1.49b3` funciono: el equipo lo rotula bien en vez de culpar a la radio.
> - ⚠️ **Los «17 segundos del Esclavo» no existen en el firmware** (`grep 17000` = cero). Son
>   **21,5 s** para soltar el verde (`SFTY6_SILENCIO_MS - AVISO_AMBAR_TIMEOUT_MS`) y **25 s** para
>   el ambar. Quien cronometre vera esos. Pendiente de decision del responsable.
>
> **Y LA REGLA DE TRABAJO MIENTRAS HAYA PRUEBAS FUNCIONALES EN MARCHA:** firmware, app,
> compilar, verificar, empaquetar. El roadmap se cierra al final y en dos lineas; **ningun `.md`
> nuevo**, y lo que haya que pedirle al funcional va en el `.html` que le da un PDF.

> ## ▶️ PUNTO DE CONTINUACIÓN — 14/09/2026
>
> 🔴 **LO QUE CAMBIO HOY Y MANDA SOBRE TODO LO DEMAS: LA SPEC PASA A SER LA AUTORIDAD.**
> Palabras del responsable: *«el archivo decisiones marea, ahora es lo que digan las spec y eso debe
> hacer el firmware»* y *«si la spec no pasa, si el arquitecto vs decisiones, si no le gusta algo, pues
> debemos ajustar la spec»*. **Si el firmware y la spec no coinciden, el defecto es del firmware.**
> `DECISIONES.md` baja a andamio. Y la spec pasa a tener **dos registros que no se mezclan** —lo que el
> equipo HACE, que se valida contra el fuente, y lo que DEBE hacer, que vive en «HUECOS MEDIDOS» y **es la
> cola de trabajo del firmware**— (`CLAUDE.md` §12 y §15).
>
> 🔴 **Y LA MEDIDA DEL BUCLE, porque el responsable pidio entenderlo y el numero NO era el que creiamos.**
> Sobre los ultimos 50 commits: **firmware +6.454/-2.834 · INSTRUMENTOS +18.470/-2.020 · documentos
> +9.039/-6.096**. No son los `.md`: **el aparato de medir crece al TRIPLE que lo que mide.** `CLAUDE.md`
> §16 convierte esa medida en **puerta**: un instrumento entra si **contesta una pregunta abierta**, y el
> commit dice cual.
>
> ▶️ **POR DONDE SE SIGUE, y son cinco piezas de FIRMWARE, no de documentos** (`roadmap.md` §0, la tabla
> del orden): **(1)** ✅ los flancos de `J16` p5/p8, hecho hoy · **(2)** **retirar el mando entero**, ~880
> lineas FUERA, 16 packs y 6 arneses — **es la recomendada: la unica que borra codigo y devuelve sitio** ·
> **(3)** el cristal que arranca y no cuenta · **(4)** la ventana de ambar contra verde · **(5)** que la
> app traduzca el aviso de barrera retenida.
>
> ⚠️ **Y una deuda abierta que NO se ha tocado a proposito: el arquitecto funcional dijo NO OK a las nueve
> spec, con 12 puntos.** Los doce son de documento —**cero defectos de firmware**— y tres son afirmaciones
> falsas vivas. Estan en `roadmap.md` 1.47. No se tocaron porque el responsable pidio parar los `.md`.
>
> 🔴 **LO PRIMERO, y no cambia por nada de lo de abajo: NADA DE ESTO HA VISTO UNA TARJETA.**
> Lo que decide si funciona es la sesión de banco (`roadmap.md` §0, grupo 3) y **la cinta del
> Esclavo del Sisga, que sigue sin traerse**. Los binarios que salgan de aquí llevan `SIN_BANCO`
> pegado al nombre, y **el sufijo lo quita quien los pruebe en un equipo**, no quien los compila.
>
> **LO QUE ENTRÓ HOY: `D-33` — LA CÁMARA VETA LA BAJADA DE LA PLUMA** (`363e375` firmware,
> `de8939f` instrumentos, `96a30e5` spec). La pluma **sube por la luz y por nada más**; **baja
> 3 s después del rojo**; y pasado el retardo **cualquiera de las dos cámaras del poste retiene
> la bajada** mientras vea algo. El veto es **local**: el otro poste abre su verde igual. Ante
> error, falsa alarma o contacto pegado **la barrera NO baja y se avisa**, diciendo cuántos
> segundos lleva retenida — nunca «cámara averiada», que este micro no puede saber.
>
> **Y lo que costó de verdad no fue el veto.** Fueron **tres instrumentos que medían la FORMA de
> una condición que `D-33` movió de sitio** —uno abortó, otro acusó al firmware de haber perdido
> el `S_FALLO`, y el tercero exigía por escrito justo lo que ahora sería el defecto— y **dos
> copias a mano del límite vial que el Esclavo llevaba sin vigilante** (`N-133`), que se habrían
> quedado viejas **compilando y sin decir nada**. Todo eso está repartido y medido, y las tres
> inyecciones se hicieron sobre el `.cpp` real con restauración verificada por hash.
>
> **Compuerta: 18 PASS · 1 FALLA · 0 ABORTADO**, dos pasadas iguales con el árbol quieto. Banco
> **1469/1470** en 80 packs. **El único rojo es correcto y no se decora** (`CLAUDE.md` §1): es
> `decisiones_01_anclas` acusando a **`D-22`**, la única decisión vigente sin construir, y **no
> se cierra con teclado: necesita una tarjeta delante**. Las que este rojo contaba antes ya
> salieron: `D-23` y `D-33` están construidas, `D-14` no se instala y `D-30` quedó recortada.
>
> 🚚 **15/09 — ENVIADO AL FUNCIONAL: `ca2de3d`.** Es el primer paquete que sale con la ventana ambar-contra-verde
> cerrada en las dos puntas (`D-34`) y con los tres defectos de `1.49` arreglados: la camara pidiendo paso ya no
> sostiene el silencio del Maestro, el cristal parado ya no deja reanudar el Degradado y `REINICIAR_RELOJ` no dice
> OK sin comprobarlo. Compuerta **18 PASS / 1 FALLA (`D-22`) / 0 ABORTADO**.
> **Se retiran sin mandarse `226ae26` y `622a20b`** (`roadmap` E5). **Nada de esto ha visto una tarjeta todavia.**
>
> **LO QUE ESPERA AL RESPONSABLE, hoy:**
>
> - ~~**El umbral de «demasiadas falsas alarmas»**~~ → **YA NO ESPERA: decidido el 14/09, basta UNA**
>   (`SPEC_8` §3), y la app lo traduce desde `f57a401`. *(Se le volvió a preguntar el 15/09 por
>   leer esta línea: por eso se tacha.)*
> - **El modo administrador que saque una barrera de la lógica** desde la app, poste a poste
>   (decidido el 14/09, sin construir; **FUERA del primer estable**, `roadmap` §0 tabla E). Antes hay que **medir** si persiste tras un corte y qué
>   pasa si se retira con la pluma abajo — probablemente sea fila propia (`roadmap` 1.41).
> - **La sesión de banco y la cinta del Sisga**, que es lo único que no destraba nadie escribiendo.
>
> ⚠️ **LO QUE SIGUE ABIERTO Y PUEDE HERIR**, sin adornos: ~~la ventana que no cierra nadie (`roadmap` **1.39**)~~ → cerrada en el fuente
> por `D-34` el 15/09, sin banco, y **cuatro de los seis modos de fallo que el veto crea** (`roadmap`
> **1.42**) — entre ellos que el firmware **no distingue una presencia sostenida real de una
> cámara mal apuntada**, y que **los últimos metros de la bajada no los ve nadie**: no hay fin de
> carrera, y **está sin preguntar al fabricante** si la centralita trae fotocélula o borde sensible.

> ## 📜 PUNTO DE CONTINUACIÓN DEL 12/09/2026 — la crónica, conservada
>
> ⚠️ **LAS CIFRAS DE ESTE BLOQUE SON LAS DE ESE DÍA Y NO SE SINCRONIZAN** (`CLAUDE.md` §14): el
> *«Maestro al 89,9 %, quedan 6.648 B»* de abajo dejó de ser cierto el **13/09**, al retirarse el
> LCD, y *«las cinco decisiones sin construir»*, el **14/09**. Se conservan porque son el porqué
> de lo que vino después; **la cifra viva está en la tabla de más abajo, copiada del acta**.
>
> 🔴 **LO PRIMERO, y no cambia por nada de lo de abajo: NADA DE LO DE HOY HA VISTO UNA TARJETA.**
> Son once horas de PC. Lo que decide si esto funciona es la sesión de banco (`roadmap.md` §0,
> grupo 3) y **la cinta del Esclavo del Sisga, que sigue sin traerse**.
>
> **La compuerta pasa de DOS rojos a UNO**, y el que queda **no es un defecto de firmware**: es
> `decisiones_01_anclas` contando las cinco decisiones tomadas y sin construir (`D-14`, `D-22`,
> `D-23`, `D-25`, `D-27`). De ésas, `D-23` es teclado, `D-22` necesita una tarjeta delante, y
> `D-14`/`D-25`/`D-27` puede que **no necesiten ni una línea de código** y el instrumento les
> exige ancla igual — eso es un defecto del instrumento, y está en la lista.
>
> **Lo que entró hoy en firmware, todo `SIN_BANCO`:** `N-142` —el ámbar de emergencia sin PIN
> **avisa al Maestro**, y el `$ACK` dice si el aviso pudo oírse— · `D-28` —el plazo de la hora a
> **400 s**, y de paso se descubrió que su derivación era una **tautología** que habría dado
> margen cero— · `D-29` —**`N-20` resucitado**: un corte de luz ya no mata la reanudación del
> Degradado— · **`D-31` —el aviso de ámbar entre postes YA SE ACUSA** (`9192063`): el Esclavo espera
> acuse del **primer** aviso, con **un** reintento, y si no llega lo dice; `dos_puntas` pasa de
> **106/106 a 110/110** porque `H4` deja de ser nota y se convierte en comprobación— · `N-163` —**`G3`
> cerrado**: la ventana de verde-contra-ámbar, de 250 ms a **cero**,
> sin tocar el umbral de 25 s—. **Maestro al 89,9 %, quedan 6.648 B** *(acta de `13591c7`: `58888 B`
> de `65536`; no se transcribe más cifra que ésta, y ésta se copió del acta, no de memoria)*.
>
> **Y el repositorio:** de **1.081 a 549 ficheros versionados** —`99_Legacy/` sale de git con el
> firmware monolítico dentro—, `OPTIMIZACIONES.md` de **2.310 a 954 líneas** con las 29 reglas
> contadas antes y después, y **18 filas nuevas en §0** del roadmap: pendientes que sólo vivían
> en el porqué y que una mudanza al histórico habría enterrado.
>
> **LO QUE ESPERA AL RESPONSABLE — y el 12/09 por la tarde se quedó en NADA.** Las cuatro que había
> se cerraron con él delante, y se dice aquí **para que nadie las vuelva a preguntar**:
> ~~microSD~~ *(tienen SD y graban; lo que falta no es decidir, es **el paso que falta en la guía**,
> y eso es teclado — fila 1.29)* · ~~tres siembras perdidas~~ *(pregunta mal planteada por mí: la hora
> la mandan los ESP32, y lo que se decidía era sólo cuánto aguanta el STM32 con su reloj interno.
> **Se queda en una**, que es lo seguro)* · ~~`A-8`~~ *(**cerrada**: la guía ya dice 24×7 en la regla y
> en la salida, en tres sitios)* · ~~el acuse del aviso de ámbar~~ *(**`D-31`, decidida Y
> ESPECIFICADA entera** — el primer diseño se descartó porque construía algo **peor** que el defecto,
> y las tres correcciones salieron de sus preguntas: acuse sólo del primero, el cancelar en el mismo
> lote, y **un** reintento porque a 8 km lo que sube es la pérdida y el presupuesto de radio sólo deja
> ~4 s)*. **Lo que queda es todo teclado**, en `roadmap.md` §0 grupo (1). Lo único que NO destraba
> nadie escribiendo está en el grupo (3): la sesión de banco y **la cinta del Esclavo del Sisga**.
>
> 🔴 **CORRECCIÓN DE ESE «NADA», ESCRITA AL FINAL DEL 12/09: HAY UNA, Y ES `D-30`.** No es una
> pregunta de las que ya estaban contestadas —el responsable YA decidió que el LCD y el mando A/B/C/D
> salen del firmware, y eso no se reabre—: es que **al medir el alcance antes de construirlo apareció
> un precio que la decisión no podía conocer**, y quien lo paga tiene que verlo. Los tres hechos, medidos:
>
> - **`mando.cpp` es el único escritor de `senalActiva`**, así que retirarlo **se lleva por delante
>   parte de `semaforo.cpp`, que es `SFTY-2`** —la barrera de salidas—.
> - Después del cambio **ningún instrumento del repositorio puede cazar un defecto en la
>   interceptación de `escribirPines()` por señal**, mientras que el código interceptado **se queda**
>   como camino sin ejercicio. Es exactamente el patrón de `CLAUDE.md` §6.
> - **`menu_setup()` llama a `coordinador_forzarMenu()`**, el todo-rojo de las dos puntas, **alcanzable
>   HOY desde la app**: borrar `menu.cpp` borra ese todo-rojo si no se le da otra puerta.
>
> **La pregunta, y es la única:** ¿se ejecuta `D-30` entera sabiendo eso, o se saca **sólo el LCD** y se
> deja el mando —que es la mitad que de verdad está trenzada con la seguridad— para cuando haya un
> instrumento que vigile la interceptación? ⚠️ **Mientras no se conteste, `D-30` NO se construye**, y
> lo que sigue vivo en `main` es la fila **1.21**: `A.A.A` entra al modo Automático **sin guarda** en el
> Maestro, con `J16` p5 y p8 **vacíos y pelados**. La ventana entre hoy y el cierre de `D-30` es real y
> está contada ahí.
>
> 1. **En `main` (`68dd2c5`): `D-26`, la hora la manda el ESP32 de cada poste.** El ESP32 siembra a su STM32
>    desde el `DS3231` (al arrancar, tras cada `SET_RTC` bueno y cada ~~300 s~~ **120 s desde el 12/09**, `CMD:HORA_ESP32`); el `SET_RTC` del
>    teléfono lo atiende el puente y ya no cruza; la siembra ya no escribe el RTC del STM32; el Esclavo toma la hora
>    del Maestro con radio y la de su ESP32 sin radio 25 s; en Degradado un salto > 29 s pasa por rojo; y
>    `$ALARM …EVENTO:HORA_ESP32…` (`J17_MUDO`, `SIN_HORA_DEL_ESP32`, `RECHAZADA_FORMATO`). Revisado por el diff por el
>    arquitecto antes de fusionar (**`roadmap.md` §3.16, `N-162`, `H1`..`H9`**). ⚠️ **Sin banco y sin tarjeta.** La
>    foto `wip/d26-hora-esp32-foto-1315` quedó integrada en `a0313fe`: ya no se usa. Los dos rojos de la compuerta
>    siguen siendo reales —`decisiones_01_anclas` y ~~**G3**~~ *(**G3 cerrado el 12/09**, `N-163`: queda UN solo rojo)*—; las cifras, en la última acta.
> 2. ~~🟠 **EN CONSTRUCCIÓN (worktree, NO está en `main`)**~~ 🟢 **CONSTRUIDA y en `main` el 11/09 por la noche, sin banco: `D-21` pieza (1)** —ámbar intermitente en la punta cuya
>    hora caducó, dentro del Degradado—. Es la cura de **`H1`**: con el `J17` del Maestro mudo, al caer la radio el
>    Esclavo adopta su `DS3231` y el Maestro sigue con la hora derivada del HSI → **verde contra verde**. **Bloquea
>    campo.** ~~Se integra por el diff~~ Integrada por el diff (`roadmap.md` §0, fila 1.13); el plazo es ~~320 s~~ ~~280 s~~ **400 s desde el 12/09 por la tarde** (`D-28` (2), `6c25bda`) —lo bajó primero la cadencia de ~2 min, y de paso se descubrió que la derivación vieja era una **tautología**: se derivaba de la propia cadencia, así que a 120 s habría dado plazo = cadencia, **margen cero**. Pasó por derivarse del **relevo** (271 s) unas horas, y **el responsable decidió esa misma tarde que el plazo cubra DOS siembras perdidas**: hoy se deriva de `HORA_DOS_PERDIDAS_MS` = `3C + deriva` = **369 s**, cuantizado a segundos enteros de deriva → **400 s**, con **cuatro** `static_assert` encadenados que lo sujetan. ⚠️ **Los 7 s que el responsable aceptó NO son este margen**: son el margen entre los dos `DS3231`, otra cantidad — aquí el colchón sobre la segunda pérdida es de **31 s**— y la fuente de los dos números es `{Maestro,Esclavo}/include/reloj.h`, idéntico en las dos puntas, no este párrafo. Y la app lo
>    enseña (`HORA_ESP32,CAUSA:CADUCADA`). La punta en ámbar **no vuelve sola** (`D-21`).
> 3. **Espera al responsable:** ~~🆕 **la cadencia de la siembra, ~5 min → ~2 min**~~ *(decidida por el responsable
>    el 11/09 por la noche y **CONSTRUIDA el 12/09 en `dca17cd`**: `SIEMBRA_INTERVALO_MS` = 120000 y el plazo
>    rederivado. ~~**Lo que sí queda para el responsable son dos cosas nuevas que salieron al construirla:** «~2 min»
>    no está fijado al segundo en `DECISIONES.md`, y a esa cadencia **tolerar DOS siembras perdidas cabría** a cambio
>    de bajar el margen entre los dos `DS3231` de 13 s a 7 s~~ *(**las dos las cerró el responsable el 12/09 por la
>    tarde y son `D-28`**: la cadencia queda fijada en **120000 ms exactos** —deja de ser «~2 min», que era lo que
>    permitía que cada lector derivase su propia cifra—, y el plazo **sí cubre dos siembras perdidas**, con el margen
>    entre los dos `DS3231` bajado de 13 s a 7 s. Está construido en `6c25bda`)* ·
>    ~~**G3/SFTY-6** (la punta en verde tarda 250 ms en soltar frente a un `S_FALLO`)~~ *(**DECIDIDO y
>    CONSTRUIDO el 12/09**, `N-163`: la ventana pasa de 250 ms a CERO **sin tocar el umbral de 25 s** —lo
>    sujeta un `static_assert` que demuestra que no se recorta el presupuesto de reintentos—, y el criterio
>    del responsable se midió caso por caso sobre 32 cortes: los ámbares no subieron ni uno)*; la
>    referencia del **relé** de `J15`; la **alarma por discrepancia de los dos `DS3231`** —con el matiz de `H3`: el
>    umbral **no puede ser 11 s a secas**, porque el HSI de cada punta mete entre siembras ~~hasta 7,5 s~~ **hasta
>    3 s desde el 12/09** (la cifra la manda la cadencia: `roadmap.md` fila 2.8, y el umbral sube de 11 a 13 s)—;
>    **tachar «sin filtro de objetivo» en `D-13`** (lo derogó `D-27`); y **los `.zip` del 10/09** —cinco en la raíz, y
>    tres (`44967db`, `bd77271`, `fae4b3e`) llevan `Camaras_Sisga_4x.html` de ese día, con las afirmaciones de
>    seguridad retiradas—. Y el texto de los «MESES» de `D-21`/`D-23`, que la medida tumbó (fila 2.7). El resto,
>    `roadmap.md` §0, grupo (2).
> 4. **Siguiente trabajo, ya decidido:** ~~los instrumentos de `D-26` (fila 1.14: ningún arnés compila el reloj del
>    Esclavo)~~ *(hecho el 11/09 por la noche: el Degradado a dos puntas compila el `reloj.cpp` real, 53/53)*; ~~`AMBAR_EMERGENCIA` sin PIN que avise al Maestro (§3.16-A)~~ *(**hecho el 12/09 en `913c29c`**, con los tres instrumentos que lo dejaban pasar arreglados —el de dos puntas ejercía una **transcripción** de la puerta buena escrita en su propio adaptador—; `roadmap.md` §0 fila 1.2. **Sin banco y sin tarjeta**, y **no confirma la causa del DAR PASO del Sisga**: eso lo diría la cinta del Esclavo. Queda viva la vía de `H4`: si muere sólo el transmisor del Esclavo, el aviso no sale, esa punta no puede saberlo y el `$ACK` sale igual que con la radio sana)*; la **APK** recompilada desde `main` —y su
>    texto de `SET_RTC|OK`, que en `68dd2c5` dice *«no hay nada que los sincronice entre sí»*, caducado por `D-20`/`D-26`
>    ~~(al escribir esto hay cambios sin comitear en `app.js` que lo tocan: se integran por el diff)~~ *(integrados en `fd595ea`)*—;
>    portar a los modelos Python la autorrecuperación nueva; los **`.docx`** cuando se cierren las indefiniciones.
> 5. **Lo cerrado el 11/09 NO se reabre desde un documento** (`roadmap.md` §0, recuadro 🔒; `CLAUDE.md` §11.1).

> **Este fichero es el estado VIVO.** Lo que está abierto, lo que bloquea y lo que falta medir.
> El *porqué* completo de cada `N-x` vive en [`roadmap.md`](roadmap.md); las decisiones vigentes,
> en [`DECISIONES.md`](DECISIONES.md) — **si un párrafo de aquí contradice una fila de allí, gana
> la fila**. Lo de debajo del separador es histórico y se conserva tachado, no borrado.

**Rama de trabajo: ~~`main`~~ `main` EN GITHUB (`origin/main`), que en ESTE clon es la rama local `main-nuevo`** *(medido el 11/09: `git rev-parse --abbrev-ref main-nuevo@{upstream}` → `origin/main`)*. ⚠️ **La rama local `main` de este clon es OTRA y vieja** —sigue a `padre/main`—: un `git checkout main` aquí no lleva al trabajo vigente. *(El 11/09 se fusionaron, por avance rápido y tras validarlas por el
diff, `feat/d20-d23-construccion` y `fix/campo-sisga-rtc-darpaso`, que ya la contenía).* 🔴 **Aquí había un hash de HEAD escrito a mano y estaba
caducado: se ha retirado en vez de sustituirse por otro que caducaría igual.** El HEAD vigente se
mide, no se lee: `git rev-parse --short HEAD`.

**La instalación certificada es la V8.4 (`e303485`, 31/07).** 🔴 **Pero el 10/09 un Maestro
(`SERIE:179DB0`) corrió en El Sisga con firmware V9 `SIN_BANCO`**: cargado el paquete del 08/09
(`7ff7d12`), probado después el del 10/09 (`b354fe9`). La cinta y el diario del Maestro están en
`evidencia/`. Lo primero: 🔴 **no usar el Modo Degradado allí con `7ff7d12`** (el Maestro se declara
en hora con el reloj parado); **avisar al instalador** de que la guía de 4 cámaras que le llegó por
WhatsApp está RETIRADA —ninguna cámara protege la pluma— *(11/09: sus CONEXIONES —cuatro cámaras,
dos por poste, `J16` p9/p10 y p11/p12, talanquera en `J15`— las dejó el responsable como
definitivas en `D-25`; lo que se retira son sus afirmaciones de seguridad, que siguen siendo
falsas)*; **medir la alimentación del ESP32** (~~se reinició 5 veces en 97 s~~ — **al menos 3
reinicios** en 12:18–12:19: la cinta trae 5 partes `EVT:ARRANQUE`, pero el parte se emite una vez
por CONEXIÓN Bluetooth, así que un arranque puede anunciarse dos veces; 2 de los 5 son
`SUBIDA_DE_TENSION`, `roadmap.md` §3.16); y **traer la cinta del Esclavo**. ~~Lo que hay
que construir es que **el ESP32 mande la hora**~~ → 🟢 **construido y en `main` desde `68dd2c5`** (`D-20`/`A-15`, con las reglas de **`D-26`**: siembra ~~cada ~5 min~~ **cada ~2 min desde el 12/09 (`dca17cd`)**, no cada hora) — **sin banco, y con `H1` abierto, que bloquea campo**. Todo en `roadmap.md` §3.16 (`N-162`).
🎯 **11/09, `D-27` — CERRADO por el responsable y alineado en los documentos (sin los `.docx`):**
**las cuatro cámaras están compradas**; **`J14` queda libre y sin cablear** (el fin de carrera no se
instala — se cierra el conflicto con `A-2`); **la configuración de cada cámara es la del manual del
modelo** (`04_Manuales/MANUAL_CONFIGURACION_CAMARAS_IA.md` §4; tabla valor a valor en el Manual 9
§4 Paso 3). Lo que dejó a la vista —la zona, el filtro y el contador de la fase 1, y **la guía del
Sisga, que en su paso 07 sigue diciendo «no filtrar»**— está en `roadmap.md` §0, filas 2.1.bis y
2.1.ter. **`D-25`, `D-26` y `D-27` no se reabren desde un documento** (recuadro 🔒 de `roadmap.md` §0). La compuerta en
verde dice que los modelos y los arneses de PC no encuentran nada; **no dice que el firmware funcione
sobre la tarjeta**.

> 🔴 **Y esta rama no es un retoque: TODO EL RELOJ DE LAS DOS PUNTAS SE REESCRIBIÓ.** El diff contra
> el arranque de la rama son **cientos de líneas** en `Maestro/src/reloj.cpp` y
> `Esclavo/src/reloj.cpp` —los dos ficheros más movidos de la tanda—, y de ahí cuelga
> `reloj_enHora()`, que es **la autorización del Modo Degradado: el único modo que da verde SIN
> confirmar la otra punta.** Antes valía por el RTC sobre `Y2`; ahora vale por una base de software
> sembrada desde fuera. **Es exactamente la clase de cambio que no se juzga en el PC.**

> 🔴 **DEPENDENCIA DE ESTA MÁQUINA QUE NO ESTÁ EN EL REPOSITORIO: `D:\toolchain\mingw64`.**
> Es la copia del `gcc` de host **fuera de la ruta con `ñ`** de `Diego.Zuñiga`, cuyo `ld` no abre
> `crt2.o` aunque el fichero exista (N-44). **Si esa carpeta desaparece, los siete arneses que
> compilan C++ real caen a `ABORTADO` a la vez** — y la compuerta lo dice en una línea que nadie
> lee cuando el resumen de arriba parece normal. Ya pasó el 05/09: `13 PASS · 7 ABORTADO`, y no se
> enteró nadie.
>
> **Por eso un `ABORTADO` se lee SIEMPRE, y un número de `PASS` sin su total al lado no significa
> nada.** Esto vive aquí y no en `CLAUDE.md` porque **es estado de una máquina, no una regla**:
> quien clone el repositorio en otro equipo no tiene este problema, tiene el suyo.

> 🔴 **SEGUNDA DEPENDENCIA DE ESTA MÁQUINA, anotada el 07/09 porque compilar la APK obligó a
> encontrarla y no estaba escrita en ningún sitio: NO HAY `java` EN EL `PATH`.** `gradlew` no
> arranca sin un JDK, y el que hay vive **fuera del proyecto y con un nombre que nadie adivina**:
>
> ```
> JAVA_HOME = D:\@Proyect\Baliza\7 sw apk\java-21-openjdk-21.0.4.0.7-1.win.jdk.x86_64
> sdk.dir   = C:/android-sdk        (lo dice android/local.properties, y existe)
> ```
>
> **Es el mismo modo de fallo que `N-44` una capa arriba:** la herramienta existe, no está donde se
> la busca, y `command not found` se lee como *«no se puede compilar»* cuando lo que falta es la
> ruta. **Un `--version` que no responde no prueba que la herramienta no esté** (`CLAUDE.md` §7):
> aquí hicieron falta dos búsquedas en disco para dar con ella. Si `compilar_apk.bat` falla en otra
> máquina, esto es lo primero que hay que mirar.

---

### ✅ Lo que está CONFIRMADO EN COBRE

~~**La última cinta es del 05/09 a las 22:19, y cargó `42a52cd`.**~~ 🔴 **CADUCO desde el 10/09: la
última cinta es la del Sisga** —`evidencia/2026-09-10_Sisga_179DB0_cinta_tramas.txt` y
`…_diario_ordenes.txt`, **sólo del Maestro**, con `7ff7d12` dentro— y lo que ejerce está en
`roadmap.md` §3.16 y §5. **Esta tabla sigue siendo la de la cinta del 05/09** (`42a52cd`), que lo
demuestra, no lo afirma:

| | evidencia en la cinta |
|---|---|
| **N-42** el Modo Automático mueve las luces | cerrado en la sesión de banco del 04/09 por la noche, con el responsable delante: *«ahí cambia ese amarillo y este a verde. Ahora está funcionando»*. Arreglo en `ceb8cc5` |
| **N-146** el ámbar re-armado | `$ACK,CMD:SET_MODO:AMBAR,RESULT:REARMADO` a las 22:19:40, y el `ESTADO` pasa de `ROJO` a `FALLO COM` |
| **N-149** el Esclavo en la trama | `ESC:AMBAR` y `ESC:ROJO` viajando en todos los `$STATUS` |
| **N-145** la hora del DS3231 | `HORA:22:19:58` — el campo dejó de ser `--:--:--` |
| **N-142** el aviso del ámbar | el `ESC:` sigue al ámbar del Esclavo sin esperar los 25 s |

### 🔴 Lo que se ARREGLÓ DESPUÉS de esa cinta y NO ha pasado por una tarjeta

| | commit | lo que enseñó de paso |
|---|---|---|
| **N-151** `DAR PASO` en un modo sin coordinador trababa el cruce para siempre | `273b315` | el equipo decía que SÍ a una orden que no iba a ejecutar, y se quedaba PEOR que antes de pedirla. Lo vigila `maestro_12_dar_paso_sin_coordinador` |
| **N-152** `CANCELAR_AMBAR` no avisaba al Maestro | `d6ce67e` | **en `MODO_AMBAR` el Maestro estaba SORDO**: un comando copiado de N-142 habría entrado sin lector |
| **N-150** el ciclo no arrancaba tras aplicar tiempos | `414b962` | `ACK_TEXTO` no tenía `SET_TIEMPOS|OK`: el genérico lo pintaba **en verde** |
| **los parsers de la app** | `414b962` | eran **TRES**, y `parseError()` leía por posición con una prueba de un formato que ningún micro emite |
| **la línea falsa de `pines.h`** | — | decía que `BOTON1`/`BOTON2` son `INPUT_PULLUP` activos en BAJO. El fuente hace `INPUT` pelado y lee `== HIGH`. Es la cabecera que todo el mundo lee antes de cablear |

> **De ninguno hay una sola prueba en cobre. Que la compuerta esté en `20/20` no dice nada de
> eso.** ⚠️ *11/09: los tres primeros iban dentro de `7ff7d12`, el firmware del Maestro del Sisga
> (`git merge-base --is-ancestor`), y la cinta del 10/09 ejerce **del lado del Maestro y por
> telemetría** `N-150` (el ciclo arranca tras `SET_TIEMPOS:3,3,15`). Ni `N-151` ni `N-152` salen en
> esa cinta —no hay un `DAR PASO` fuera de coordinador ni un `CANCELAR_AMBAR` del Poste 2—, y nada
> de ella ve las luces ni el Esclavo: no se tacha nada hasta tener su cinta (`roadmap.md` §5).*

---

### 🟢 Lo que la rama `feat/d20-d23-construccion` SÍ construyó — y lo que NO

**Verificado por el DIFF, no por el parte que la acompañaba** (`CLAUDE.md` §8). El *porqué* de por
qué hubo que auditarla vive en [`roadmap.md`](roadmap.md) bajo **`N-160`**.

| decisión | veredicto medido |
|---|---|
| **`D-20`** — la autoridad de la hora es el ESP32 | 🟢 **CONSTRUIDA** (`9dd8bbf`). `reloj_sembrarDesdeIso()` existe en las **dos** puntas **con llamadores reales** —la rama `SET_RTC:` de `Maestro/src/bluetooth.cpp` y la de `Esclavo/src/bluetooth.cpp`— y `reloj.cpp` trae el **extrapolador por software** `segBaseDelDia + (millis() - tBaseMillis) / 1000`. **Es lo que permite que `reloj_enHora()` sea cierto sin `Y2`, y por tanto lo que desbloquea el Modo Degradado del poste 2, que estaba muerto.** La propagación al Esclavo la dispara `coordinador_sincronizarHora()`, llamada **dentro del `if`** del sembrador |
| **`D-21` pieza B** — ámbar si la hora deja de ser fiable | 🟡 **ESCRITA** (`7adee76`) en `Esclavo/src/modo_degradado.cpp`, 🔴 **pero INALCANZABLE**: el único `horaValida = false` del Esclavo vive dentro de `reloj_setup()`, así que dentro del Degradado la guarda no puede dispararse (`roadmap.md` §3.10.bis y §3.16-E). Falta la pieza A —el `OSF` hasta `reloj_enHora()`—. ⚠️ **La del Maestro —`irAAmbar("Reloj no fiable", "Degradado detenido")`— YA EXISTÍA antes de la rama**: el commit sólo le añadió un comentario, y el parte la contó como nueva. ⚠️ **Y las dos no son la misma línea:** el Maestro va **directo** al ámbar; el Esclavo llama a `iniciarSalida(true)` —rendición—, que fuerza **todo-rojo primero** y termina en `DEG_RENDIDO` con `semaforo_iniciarFallo()`. Las dos acaban en intermitente; sólo el Esclavo pasa por el despeje, y es lo correcto |
| **`D-14`** — la entrada de alarma de la cámara | 🛑 **CERO CÓDIGO.** Entró como **dos comentarios en `pines.h`**; **revertido en `def6374`**. ~~Sigue BLOQUEADA por una medida de multímetro~~ — 🟢 **esa medida NO hacía falta** (08/09, `roadmap.md` §3.11.bis: el Manual 9 ya tenía leído `1 input … max. 24VDC`). **Lo que la bloquea es una DECISIÓN del responsable: cuál de los tres últimos canales de potencia (`J9`/`J11`/`J13`) se gasta, para siempre** (§3.11.ter) |
| **`D-22`** — `Y1` como latido del micro | 🛑 **CERO CÓDIGO.** Entró como **tres comentarios en cada `main.cpp`**, y describía **el HSI de hoy como si fuera la decisión implementada**; **revertido en `903f483`**. Medido buscando por sus dos nombres (`Y1` y `HSE`/`SystemClock_Config`) sobre `{Maestro,Esclavo,Repetidor}/{src,include}`: **cero apariciones**. `Y1` **no se ha arrancado nunca** y el firmware sigue en el HSI. **Va la ÚLTIMA, va SOLA, y no se carga sin una tarjeta delante:** si `Y1` no oscila, el `_Error_Handler` del núcleo es `noreturn` + `while (1)` y **la tarjeta queda a oscuras, sin luces y sin reiniciarse** |
| **`D-23`** — pantalla propia del poste 2 | 🛑 **CERO CÓDIGO, y es una decisión de la APP.** Entró como **dos comentarios**; **revertido en `5d0a0b9`**. Medido sobre **toda** la rama: `app.js` e `index.html` **no aparecen en el diff**, y las **CUATRO** copias de `app.js` del árbol son **idénticas por hash** (`md5 b09dcc85…`) *(el parte decía tres)*. ~~🔴 **Y antes de construirla hay que ELEGIR LA VÍA, que no está elegida: `A-14` en [`DECISIONES.md`](DECISIONES.md)**~~ — 🟢 **CADUCO desde el 07/09: la vía está elegida** —`$EVENT` nuevo, emitido también al conectar (`A-14`, resuelta en índice y cuerpo)—. **Lo que falta es construirla**, y sigue en cero código (`decisiones_01_anclas` la acusa, medido el 11/09) |

> 🔴 **Tres de las cinco casillas se apagaron con COMENTARIOS.** El instrumento que las acusaba,
> `decisiones_01_anclas`, dejó de acusarlas sin que se construyera nada. **Los tres reverts son la
> corrección, y el rojo que vuelve es el correcto:** una decisión vigente sin ancla en el fuente
> **es** una decisión sin construir, y el pack está para decirlo (`CLAUDE.md` §1).
>
> ✅ **Lo que salió limpio, y se dice para no volver a mirarlo:** no se tocó **ni un pack**, ni
> `compuerta.py`, ni ningún `Validacion_*`. **El instrumento no se ajustó para que diera verde**, que
> era el riesgo mayor.

#### 🟢 `N-160` · CERRADO el 08/09 en las DOS puntas — y su residual, también

**Salió al auditar la tanda de arriba, y éste sí es firmware.** Así estaba en `reloj.cpp` de **las
dos puntas** cuando se midió, el 07/09 por la noche —**el bloque se fecha a propósito: hay un
arreglo EN VUELO y sin comitear mientras se escribe esto**, así que este código puede haber cambiado
ya; lo que no ha cambiado es que **el defecto no está cerrado**—:

```c
if (sscanf(str, "%d-%d-%d,%d:%d:%d", &anio, &mes, &dia, &h, &m, &s) == 6) {
  reloj_ajustar((uint8_t)h, (uint8_t)m, (uint8_t)s, (uint8_t)dia);   // void: rechaza EN SILENCIO
  return true;                                                        // no depende de nada
}
```

`reloj_ajustar()` descarta con `if (hora > 23 || minuto > 59 || segundo > 59) return;` **y el
retorno dice `true` igual**. Es el patrón de `CLAUDE.md` §2 —*un acuse que no depende de lo que la
llamada devolvió*— **una capa por debajo del `$ACK`**, que es donde no lo buscaba nadie.

- **Hay una barrera debajo, y por eso no es peor:** `coordinador_sincronizarHora()` se niega si
  `!reloj_enHora()`, así que el caso del reloj **nunca sembrado** está cubierto.
- 🔴 **Lo que NO cubre es la RE-SIEMBRA.** Con la hora ya puesta, un `SET_RTC` malformado se rechaza
  dentro, el retorno dice que sí, **la propagación pasa porque el reloj seguía en hora**, y el
  técnico se va del poste con el `$ACK` del puente **creyendo que dejó la hora nueva**. Lo que se
  propaga es **la hora VIEJA**.
- 🔴 **En el Esclavo el retorno se ignora del todo:** la llamada es `reloj_sembrarDesdeIso(accion + 8);`
  sin `if`.
- ⚠️ **Y el cast va ANTES de la validación:** `h = 256` se convierte en `(uint8_t)0` y **entra como
  medianoche**. Se valida el `int`, y luego se castea.

> 🟢 **Estado: CERRADO, y no con una afirmación.** `reloj_sembrarDesdeIso()` devuelve hoy
> `reloj_ajustarConAcuse(h, m, s, dia)` en **las dos puntas**, y los `int` llegan **sin castear**, que
> era la mitad silenciosa —`h = 256` ya no entra como medianoche—. Lo vigila
> `reloj_02_siembra_que_miente` con **2.401 casos de borde por punta** y **siete controles
> negativos**.
>
> 🟢 **Y el residual que dejó, cerrado el 08/09 en `6c90ff0`:** el Esclavo escribía en el Diario si
> la hora había entrado y **el Maestro no** —su `bluetooth_reportarEvento()` salía FUERA del `if`—.
> Era el mismo defecto una capa arriba, y en la punta que **propaga la hora**, o sea la que se
> consulta cuando las dos discrepan. Lo mide una comprobación nueva del pack, acreditada inyectando
> el defecto en el `.cpp` real (20/20 → 19/20, acusando sólo a la punta que lo tenía).
>
> 🔴 **Sin prueba en tarjeta**, como todo lo de esta rama.

---

### 🔴 ABIERTO — por orden de lo que duele

**Los tres primeros NO los cierra nadie escribiendo código.** Confundirlos con los que sí es como
se acumula un `20/20` que no acerca una tarjeta (`CLAUDE.md` §2.bis).

1. **`BAT:--`** — falta un **divisor de tensión** y una entrada analógica. La causa está MEDIDA:
   `grep -rn analogRead` sobre las cuatro carpetas da **cero**, y N-108 puso el `--` a propósito
   para que nadie leyera un 12,6 V que era un literal.
2. **`J16` p1 lleva 12 V crudos.** Taparlo es **obligatorio en cada equipo que se monte**
   (N-120), no una cautela de banco.
3. **Matriculación por ID de Bluetooth**, no por nombre y sin hacerlo a mano. Es una **decisión
   de protocolo del responsable** y cuesta bytes: `RF_Packet` son 4 bytes
   `{msgID, command, param, crc}` y **no tiene campo de dirección**; el CRC cubre 3. Meter
   direccionamiento cambia el contrato de la radio en las dos puntas. **Aplazado a después del
   banco por decisión del responsable.**
4. **`buildCommand()`** de esa misma suite sigue siendo copia a mano de `generarComando()`, que
   tiene cero llamadores **a propósito**. Unificarlo mueve la pregunta abierta del `*XX` que
   vigila `simulador_puente_esp32.py`.
5. **Retirar `parseStatus()` de verdad** exige tocar `simulador_app_bluetooth.py` y
   `documentos_03`. Hoy queda como **vista tipada** encima del único partidor, no como un segundo
   parseo — que era el defecto.
6. **`FW-N53`** — la inhibición de secuencias ya está en las dos puntas; falta decidir si se
   redefinen los gestos (hoy Auto es `A·A·A` y Ámbar `B·B·B`). Es **decisión de spec**: cambia el
   Manual 1, el Manual 3 y el adiestramiento del operario.

> 🔴 **`APP-APK` — recompilar la APK: SIGUE ABIERTO. El cierre del 07/09 era FALSO y se REFUTA
> aquí en vez de borrarse.** Decía: *«CERRADO el 07/09: APK recompilada con éxito mediante Capacitor
> 6 y Gradle (`assembleDebug`), generando la copia maestra en
> `05_Funcional/IOT_VIAL_Semaforos_v9.0.apk`»*.
>
> **Medido el 07/09 por la noche, con `ls` y con `sha256sum` —hashes, no tamaños (`CLAUDE.md` §7.5)—:
> ese fichero NO EXISTE en el disco.** En `05_Funcional/` sólo hay **cinco** APK, y las cinco llevan
> el sufijo `_SIN_BANCO`; la más nueva es la de `7586c46` (05/09). Lo que hubo fue **una copia
> renombrada de esa misma APK** —mismo `SHA-256`, `892a0e2a…`— **que al renombrarse perdió el
> `_SIN_BANCO`**, que es justo la etiqueta que impide que alguien la suba creyéndola validada. El
> parte publicaba su **tamaño**, y los dos pesaban lo mismo: por eso el tamaño no delataba nada.
> **Los `.apk` están en `.gitignore`, así que git no avisa de esto: hay que mirarlo en el disco.**
>
> 🟢 **CERRADO DE VERDAD el 07/09 a las 20:22, y la prueba es el HASH, no el parte.** Se corrió
> `npx cap sync android` y `gradlew clean assembleDebug` —`BUILD SUCCESSFUL in 42s`— y salió
> **`05_Funcional/IOT_VIAL_Semaforos_2026-09-07_ca86869_SIN_BANCO.apk`**, 3.988.161 B:
>
> | | `SHA-256` |
> |---|---|
> | **la nueva** (`ca86869`, 07/09) | `82f558884aeb2ae597acc918fb24ead5b3373e34748b10f998b5c0b264be2294` |
> | la del 05/09 (`7586c46`) | `892a0e2a37048a8f10ada77572b6c75305f9f3d3197e685429d81e2158527fa9` |
>
> **Distintos: es una compilación, no una copia.** Y lleva su `_SIN_BANCO`, que se queda hasta que
> alguien la instale en un teléfono con un equipo delante.
>
> ⚠️ **PERO ES LA MISMA APP, y se dice para que nadie lea el hash nuevo como funcionalidad nueva:**
> `git diff 7586c46..HEAD` sobre `www/`, `app.js`, `index.html`, `js/` y `style.css` sale **vacío**.
> Esta rama no tocó una línea de la app —`D-23` sigue sin construir—, así que lo único que cambia
> respecto a la del 05/09 son las marcas de tiempo del `.apk`. **Lo que se cierra es el proceso, no
> el producto.**

> ~~**`validateTiempos()` de los unitarios de la app sigue en 1..15 min**~~ — 🟢 **CERRADO de verdad
> el 07/09 en `31170e8`, y verificado por el DIFF:** `validateTiempos()` pasó de `v < 1`/`r < 1` a
> `v < 3`/`r < 3` (`VERDE_MIN_MIN = 3`, `limites_ciclo.h`), **y además movió los dos casos borde de
> `0` a `2` (verde) y a `1` (rojo)** — antes ninguno de sus casos tocaba el borde, así que era una
> copia vieja **que no podía fallar** con la palabra «probado» encima. **Es el único arreglo real de
> aquella tanda fuera del reloj.** *(La cifra de la suite se lee en la tabla del acta, más abajo; no
> se repite aquí a mano.)*

> ~~**`MANDO_A`/`MANDO_B` no responden — `0,6 V` en reposo (N-118), y van cableados**~~ —
> **REFUTADO el 05/09** (`d020f3c`), con la medida del propio banco: en `617bd00` —el binario que
> estaba en la tarjeta— `BOTON1/2` iban en `INPUT_PULLUP` y `CAM_C/D_PIN` en `INPUT` pelado; el
> paso 20 midió **9,92–9,94 kΩ en los cuatro pines** y **`0,6 V` sólo en los dos con pull-up**.
> Mismo cobre, distinto `pinMode`, distinta tensión. **El banco había corrido las dos ramas del
> experimento en la misma tabla y nadie lo leyó así.** Y además es moot: **ya no hay mando**
> (`D-1`). La tensión de `J16` p5/p8 queda como **prueba CANCELADA**, no como casilla pendiente:
> una casilla abierta invita a puentear `J16`, que es el gesto que precedió al calentamiento del
> paso 29.
>
> ~~**N-145 no se puede dar por probada: falta comprar el `DS3231`**~~ — **RESUELTO el 05/09 por
> el responsable: cada ESP32 lleva su reloj con pila propia** (`D-9`, `D-15`). ~~Lo que sigue sin
> verificar es la dirección `0x68` sobre el módulo.~~ *(11/09: verificada en el módulo del Maestro
> `179DB0` — en la cinta del Sisga el puente contesta `SET_RTC` con la hora releída del `DS3231` y
> `LEER_RTC` la da avanzando, 12:17:31 → 12:18:52; el del Esclavo sigue sin medir)*
>
> ~~**N-148 · la app no pide confirmación de vía al dar ámbar en Manual**~~ — **CERRADO EN SOFTWARE**: `SET_MODO:AMBAR` está en la tabla `VIA_MANIOBRA` de `app.js` y pasa por `confirmarVia()`, igual que `MANUAL:CAMBIAR_TURNO` y `SET_MODO:AUTO`. Y el texto **no dice «se pone en ámbar»** —eso ya lo dice el rótulo del botón—: dice lo que significa en la calzada, que los dos postes quedan en intermitente a la vez. 🔴 **Sin prueba en tarjeta.**
>
> ~~**N-106 · el ámbar de emergencia de la app no saca al Esclavo del Degradado, y aun así se
> contesta `$ACK`**~~ — **CERRADO EN SOFTWARE**: `Esclavo/src/bluetooth.cpp` llama hoy a
> `degradado_salir()` por `salidaDegradadoIniciada()`, que **pregunta la misma guarda que ella
> tiene** —no una parecida— y contesta distinto por rama: `SALIENDO_TODO_ROJO`,
> `SALIDA_YA_EN_CURSO` o `$ERR ... REPITA`. `app_03_sin_ok_mudo` da **18/18**. 🔴 **Sin prueba en
> tarjeta.** *(Este fichero lo publicó como abierto apoyándose en un `grep` de `degradado_salir` que
> ya no daba cero — §4: un cero de `grep` es «mi patrón no encontró», y aquí ni siquiera lo era.)*
>
> ~~**A-12 · el Modo Inteligente corta un verde a los 15 s**~~ — **arreglado**: el
> `tiempoActual >= 15000UL` ya no está en `modo_inteligente.cpp`, y `app_11_rangos_de_tiempos`
> volvió a verde. ✅ **07/09 noche: `DECISIONES.md` ya no lo lista como abierto con la compuerta en
> rojo** —se le retiró la cifra caducada y el choque «`D-5` contra el firmware» quedó marcado como
> RESUELTO, tachado y no borrado—. ⚠️ **Lo que SÍ sigue abierto es la condición de `D-19`**, que no
> la cierra un commit: la firma del funcional sobre el manual.

---

### 🛑 BLOQUEANTES

| # | Qué está bloqueado | Qué lo desbloquea | De quién es |
|---|---|---|---|
| 🛑 **BLQ-3** | **La tarjeta Maestro dañada** (**N-116**) *(11/09: es **la Maestro de la sesión 1 del banco**, que se descartó entera; la `SERIE:179DB0` es **otra placa**, reprogramada como Maestro el 04/09 —`roadmap_hist.md` N-126, anunciada `SEM-179DB0-M`— y es la que corrió V9 en el Sisga el 10/09. **Ya no bloquea ejercer firmware en cobre: bloquea recuperar esta placa**)*: se calienta y deja de funcionar a los ~30 s. **El firmware queda descartado por censo**, así que reflashear no lo arregla. **La causa que sostiene el cobre es latch-up**: los 5 pines de bornera van desnudos al die y `J16` p1 lleva 12 V crudos | **Medir el consumo del riel de 3,3 V en frío** con fuente limitada en corriente, antes de energizar. 🛑 **No reenergizar «a ver si pasa»** | **Responsable** |
| 🛑 **BLQ-6** | **Nada de lo arreglado después de la cinta del 05/09 ha pasado por una tarjeta** *(11/09: **en parte caduco** — el Maestro del Sisga llevaba `7ff7d12`, que los contiene, y su cinta ejerce del lado del Maestro `N-150`; **las luces, el Esclavo, `N-151` y `N-152` siguen sin ver cobre**, y lo posterior a `7ff7d12` —`c51cc85`, `141f191`, `63d6964`— no ha tocado ninguna tarjeta con cinta; `roadmap.md` §5)*: N-150, N-151 y N-152 tocan el camino del ámbar y del Modo Manual, o sea **lo que decide qué ve un conductor** | **Una carga y una pasada de los pasos de ámbar, rojo total y `DAR PASO`.** Nada lo sustituye | Banco |
| 🔴 **BLQ-5** | **Todas las tarjetas, no sólo la dañada** (**N-120**): la placa protege sus **9 salidas** con 220 Ω y optoacoplador, y **ninguna de sus 5 entradas de campo** | Revisión de diseño (**2K2 en serie**). **Mientras tanto: tapar el pin de 12 V de `J16` es obligatorio en cada equipo** | **Responsable** |
| 🔴 **BLQ-4** | **La única vía de operación del equipo**: ~~el ESP32 no se anuncia por Bluetooth de forma fiable (**N-117**). Arreglado en el árbol el 04/09, **causa no confirmada en el módulo**~~ — *(11/09: **el síntoma de N-117 se cerró en banco el 04/09** —`roadmap_hist.md` N-126: el módulo se anuncia estable como `SEM-179DB0-M`—; la causa ya no se puede discriminar con el arreglo dentro. **Lo que hay HOY es otro síntoma de la misma superficie:** el ESP32 del Sisga se reinició al menos 3 veces en 12:18–12:19, con `CAUSA:OTRO_PERRO` y `SUBIDA_DE_TENSION`, que **no son el perro de N-117** —ése se publicaría `PERRO_DE_TAREAS`, `nombreCausa()`—; `roadmap.md` §3.16)* | ~~**1º (30 s, gratis): buscar el equipo en la lista del teléfono.** **2º: monitor serie a 115200 sobre el CP2102, ANTES de reflashear**~~ *(11/09, para los reinicios del Sisga:)* **USB-TTL en `TX0` a 115200** —la ROM imprime `rst:0x..` en cada arranque—, **osciloscopio en 3V3 y `EN`**, y **una fuente de 5 V buena** (es la línea `A5` de la lista de compras, sin pedir) | Técnico |
| **BLQ-2** | 🟠 **El cristal `Y2`.** No oscila en la tarjeta medida (N-17, N-37, medida de banco del 01/08). La mitad de firmware **ya está hecha** (N-80): `SET_RTC` contesta con motivo en vez de mentir | **Diagnosticar el `Y2` de la SEGUNDA tarjeta** para decidir entre reparar el cristal o reloj de software | **Responsable** |
| ~~**BLQ-1**~~ | 🟢 **CERRADO el 31/08 — es un `ESP32-WROOM-32` clásico**, con `BR/EDR` y por tanto SPP | — | — |

---

### 📏 VERIFICACIÓN EN ESCRITORIO — lo que dice la última acta

🔴 **LAS CIFRAS DE ABAJO ESTÁN PENDIENTES DE ACTA NUEVA: se midieron ANTES de los tres reverts**
(`def6374`, `903f483`, `5d0a0b9`), que retiraron las anclas de `D-14`, `D-22` y `D-23`. **No se
actualizan a mano** —una cifra escrita a mano nace caducada—: **se copian del acta que salga de la
próxima corrida completa**, y hacen falta **dos pasadas** si antes hubo un `--rapido`
(`CLAUDE.md` §4).

🔴 **Aquí había además un resumen de compuerta escrito a mano que contradecía a la tabla de abajo
—una línea decía un total y la otra otro—. Retirado, no actualizado.** El resumen vigente sale de
`ls -t evidencia/*_compuerta.txt | head -1`, nunca de este fichero. **Lo que sí se puede decir sin
acta es el ESTADO: `decisiones_01_anclas` vuelve a acusar a `D-14`, `D-22` y `D-23` de no tener
ancla en el fuente, y esa acusación es CORRECTA — están decididas y sin construir.**


🔴 **Y el verde sigue sin ser un entregable — con el contraejemplo delante en vez de como
advertencia.** El banco del 3-4/09 encontró **tres defectos que ninguna línea de instrumento podía
ver**, porque ninguno es una propiedad del fuente. Lo que sí hay que apuntarles: **no fallaron en
nada de lo que sabían mirar**. La medida del desequilibrio —**2,31 a 1** el 02/09, **2,74 a 1**
acumulado el 05-06/09— está en el README y en `roadmap.md`.

⚠️ ~~19 PASS | 1 FALLA | 0 ABORTADO · 1180/1180~~ y ~~18 PASS | 1 FALLA | 1 ABORTADO · 995/1010 en
70 packs~~ — **cifras de corridas intermedias del 05/09**, con el árbol a medias mientras varios
agentes trabajaban a la vez. **Estas cifras se vuelven a copiar del acta en cada corrida; no se
escriben a mano** (N-93).

---

### 🧭 MAPA RÁPIDO DE ARTEFACTOS

| Componente / Documento | Ubicación | Nota |
|---|---|---|
| **App móvil de campo** | [`05_Funcional/App_Semaforo/`](05_Funcional/App_Semaforo/) | Frontend Web Bluetooth / WebView, selector de cruces y Courier RTC |
| **APK Android** | ~~la más nueva del disco es `05_Funcional/IOT_VIAL_Semaforos_2026-09-08_ded4416_SIN_BANCO.apk`~~ **la más nueva del disco es `05_Funcional/IOT_VIAL_Semaforos_2026-09-10_b354fe9_SIN_BANCO.apk`** (`SHA-256` `3bfd9e61…`, distinto de la del 08/09, `c7ec9e8e…`) *(medido el 11/09 con `ls` y `sha256sum`; los `.apk` están en `.gitignore`, así que git no vigila esto)* | 🔴 **NO está al día con `main`: hay que RECOMPILARLA.** `git diff b354fe9..HEAD` sobre `App_Semaforo/app.js` y `www/app.js` **no sale vacío** (`e91854c`, 11/09: se retiraron los textos de DAR PASO y el `state.hora` del `$ACK` del puente), así que esa APK lleva lo que se retiró. ~~🟢 **al día**~~ *(lo que sigue es de la del 08/09)*: lleva el aviso del Modo Inteligente a ciegas (`D-24`) y su contenido se verificó **entrada por entrada y por CRC** contra los 13 ficheros de `www/`, no por que el build saliera bien. ⚠️ **Las anteriores NO se borran pero están caducadas.** Y el sufijo `_SIN_BANCO` se queda hasta que alguien la instale con un equipo delante |
| **Paquete de REVISIÓN** *(no es entrega de versión)* | `Paquete_Revision_V9.0_2026-09-08_<hash>_SIN_BANCO.zip`, generado por `generar_entrega_v9_0.py` | 🟢 Se regenera de un commit concreto y **no se versiona**. Lleva fuente para PlatformIO, manuales, la guía de cableado, la APK, el acta y el **`LEEME_PRIMERO.htm`** (se abre con doble clic). 🛑 **NO es una entrega de versión: eso exige banco pasado.** Ver la skill `entregar` §1 |
| **Guía de cableado y banco (HTML)** | [`05_Funcional/Guia_Cableado_y_Pruebas_Banco.html`](05_Funcional/Guia_Cableado_y_Pruebas_Banco.html) | **El documento de conexiones que se entrega**, y el **formulario de vuelta**: se rellena y se devuelve en PDF |
| **Esquemático KiCad bueno** | [`01_Firmware/Controladora_Semaforos/`](01_Firmware/Controladora_Semaforos/) | 649 KB con LCD, botones y el canal del motor, y el `.kicad_pcb` de 2,1 MB. La copia incompleta de `03_Hardware_Tarjeta/KiCad/` **se borró el 27/08** |
| **Informe de banco 3-4/09** | `evidencia/Informe_Pruebas_Banco_Semaforos_V9.0.pdf` | 24 de 29 pasos, sobre `617bd00` |

#### Manuales que siguen describiendo el aparato anterior

| Manual | Qué dice de más | Qué es cierto hoy |
|---|---|---|
| **1 · Usuario** | cámaras en `PB0`/`PB8`, mando y pulsadores | ~~**2 cámaras en `J16` p10/p12**~~ **4 cámaras, dos por poste, en `J16` p10/p12** (`D-25`; las cuatro compradas, `D-27`); el mando **no se monta** (`D-1`) y la operación es por app (`D-16`) |
| **3 · Protocolo de pruebas** | ⚠️ la cuenta de «80 pruebas» es la del protocolo **anterior** a la reescritura | **MEDIDO el 01/09**: 75 identificadores únicos, 47 casillas `CUMPLE` y 22 «No se firma» — **47 + 22 = 69, no 75**. 🔴 **No se publica un recuento nuevo porque no sale limpio**: seis pruebas no caen en ningún grupo, y hasta saber por qué cualquier cifra sería inventada |
| **9 · Cámara IA** | contactos en `PB0` (Demanda) y `PB8` (Umbral) | **`PB8` ya no es destino de cámara**; el pinout se muda a `J16` p10/p12 · *11/09: alineado con `D-25` y `D-27` —cuatro cámaras, `J14` libre, valores del manual del modelo—; el `.docx` no* |
| **10 · Bluetooth** | puerto `USART1` en `PA9`/`PA10`, y un módulo SPP dedicado | **`USART1` remapeado a `PB6`/`PB7`, salida por `J17`** (N-76), y **lo sustituye el ESP32** |
| **11 · RTC** | `DS3231` en `PB0`/`PB8` del STM32 | **el `DS3231` vive en el ESP32** (`GPIO21`/`GPIO22`, pila propia) |
| **13 · Expansión I²C** | sacar bus de `PB0`/`PB8` | **su §4 queda sin sujeto**: el I²C ya no vive en el STM32 |

---
---

> ## 📚 DE AQUÍ ABAJO ES HISTÓRICO
>
> **El estado vivo está arriba.** Lo que sigue son las decisiones de la V9.0 con su fecha y su
> motivo: se conserva porque una fila tachada con su motivo no se vuelve a proponer y un hueco sí.
> **El porqué completo, al día, vive en [`roadmap.md`](roadmap.md)** — este fichero no es la
> bitácora.

### 📌 Las cuatro decisiones que reconfiguraron la V9.0

#### 1. ~~Sistema de 4 cámaras IA AcuSense~~ → ~~**2 cámaras de demanda**~~ → **4 cámaras, dos por poste** *(11/09: `D-25`; las cuatro compradas, `D-27`)*

* **Lo que sigue en pie:** descartar ordenadores externos. La analítica corre dentro del
  procesador AcuSense de las cámaras Hikvision, y el controlador consume **un contacto seco** por
  cámara: no hay red, ni imagen, ni vídeo (`D-12`). Todo cambio de sentido respeta el **Despeje
  Todo-Rojo** (`cfgDespejeSeg`).
* ⚠️ ~~Maestro: Cámara 1 (`PB0`) + Cámara 2 (Umbral, `PB8`); Esclavo: Cámaras 3 y 4~~ — **falso
  desde el 28/08**: ~~son **dos cámaras de demanda, una por poste**, en **`J16` p10 (`PB14`) y p12
  (`PB15`)**~~. ⚠️ **11/09: `D-25` lo cambia otra vez — CUATRO cámaras, DOS POR POSTE, p10 y p12 en
  cada uno** (ver `DECISIONES.md`). **`D-27` (11/09): las cuatro están compradas, y `J14` queda
  libre y sin cablear.**
* ✅ **La medida `M3` está cerrada desde el 03/09 y las cámaras se cablean** (`D-3`): pull-down
  real de 10 kΩ en las cuatro posiciones, `p10` y `p12` a **0 V** en reposo, entrada **activa en
  ALTO** — que es lo que el firmware ya hacía. ~~🔴 No se cablea todavía: polaridad en
  contradicción~~ — **caducado.**
* 🔴 **`J16` p1 lleva 12 V crudos.** Se tapa físicamente antes de cablear nada.
* ~~`p5`/`p8` se dejan vacíos «de colchón»~~ — **REFUTADO el 31/08**: son `MANDO_A` (`PB9`) y
  `MANDO_B` (`PB13`), el firmware **sigue leyéndolos**, y qué se pone ahí es la decisión abierta
  `A-2`. Cualquier cosa que se cierre en esos pines entra por el reconocedor de secuencias del
  mando.

#### 2. Telemetría Bluetooth (estándar Baliza)

* ⚠️ ~~el módulo Bluetooth en `USART1` (`PA9` TX, `PA10` RX)~~ — **dos cosas cambiaron y las dos
  están medidas:** (1) **N-76 remapeó `USART1` a `PB6` TX / `PB7` RX**, con salida por `J17`
  p3/p2; (2) el **módulo SPP dedicado se retira y lo sustituye el ESP32**.
* **Desacoplo hardware `U3`:** `PA8` (`RS485_IN_DE_RE`) en `HIGH` permanente, para poner en Hi-Z
  la salida `RO` y evitar choque con el `TXD` del módulo.
* **Caja Negra de alarmas:** `$ALARM,NODE:...,EVENTO:FALLO_RF_...*XX`. ⚠️ ~~SFTY-6 a los 12 s~~ →
  **son 25 s desde N-71**: el techo de 12 s estaba **por debajo** del peor caso de reintentos
  (20,5 s), así que los reintentos 4 y 5 no se ejecutaban nunca.

#### 3. N-53 — interferencia entre el mando y la pantalla

* **Causa raíz:** los relés remotos van en paralelo con los pulsadores frontales (`PB9` Botón 1 /
  `PB13` Botón 2). Al pulsar tres veces rápido en `AJUSTAR HORA`, el firmware interpretaba `A·A·A`
  o `B·B·B` y cancelaba la edición.
* **Lo que hay en el firmware hoy:** `secuenciasInhibidas()` en las dos puntas, y el Degradado
  exige cuatro pulsos alternados `A·B·A·B`. **Pero Automático sigue siendo `A·A·A` y Ámbar
  `B·B·B`.**
* **Lo que este apartado prometía y NO está:** la redefinición a `A·B·A`, `B·A·B`, `B·A·B·A` y
  `A·A·B·B`, escrita como *«Solución V9.0»* en pasado. **El Manual 3 sí decía la verdad** — el
  documento del auditor estaba bien y **el estado interno era el que mentía**, que es peor, porque
  es el que se usa para decidir qué falta hacer. Ver `FW-N53`.

#### 4. La arquitectura del 28/08 — el ESP32 es expansión, no controlador

**El documento completo es
[`05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md`](05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md).
Aquí no se copia: se enlaza.**

* **El STM32 sigue siendo el controlador.** Conserva las luces, la barrera (`J15`), el buzzer, la
  radio LoRa (`USART3`) **y las cámaras**.
* **El ESP32 es un módulo de expansión y no manda sobre las luces.** Se lleva el **reloj
  `DS3231`** y el **Bluetooth**, con **fuente propia desde 12 V**: el accesorio no puede tumbar al
  que manda.
* **Se retiran** la pantalla LCD (`D-17.bis`) y ~~los cuatro pulsadores y el mando de 4 relés~~ →
  el **hardware** del mando tampoco se monta (`D-1`, 05/09), pero **su código se queda**.

> 🔴 **Las dos consecuencias que fijan el orden, y no son opinión:**
> **(a)** ~~por Bluetooth sólo se alcanzan tres de los ocho modos, sin `SET_MODO:MENU`~~ →
> **REFUTADO el 31/08 por el propio firmware (N-100):** `SET_MODO:MENU` **existe**, entra **sin
> PIN**, y existen también `SET_MODO:DEGRADADO`, `SET_MODO:INTELIGENTE`, `SET_MODO:ALCANCE`,
> `REINICIAR_RELOJ` y `DEMANDA`. **La salida por app ya está construida** (`d34cfe2`, N-78).
> **(b)** Retirar el código del mando **no deja `if` inertes: borra un veto.** `mando_ambarLocal()`
> tiene **cinco llamadas vivas** y `ambarLocal = true` se arma en **un único sitio**, el
> `case ACC_AMBAR` al que sólo se llega por `B·B·B`. Sin ese armador la bandera no se arma jamás,
> los `if` se vuelven siempre-verdaderos y una orden de radio puede sacar al Esclavo de un ámbar
> que un operario dejó puesto a propósito: **SFTY-21 desapareciendo por sustracción.**

---

### 🟢 Las cinco pasadas que dejaron el banco como está

| | qué encontró | por qué sigue escrito |
|---|---|---|
| **N-46** (05/08) | los tres validadores monolíticos imprimían `FALLA` y salían con código `0` | se retiraron exigiendo que los packs sumaran **exactamente** sus comprobaciones y que el **texto** de cada una coincidiera: Costura `41 = 41`, Maestro `64/67`, Esclavo `31 = 31` |
| **N-62** (27/08) | tres packs `documentos_*` nuevos, **que nacieron con 10 fallos reales**: el README publicaba 32 rutas y 86,4 % de flash contra las 38 y el 92,8 % del acta que él mismo citaba | es la única forma de saber que un instrumento mide |
| **N-75** (28/08) | el rewrite de la app entró con **dos instrumentos en `ABORTADO`** —los únicos dos que ejercen la app— y detrás entraron cuatro defectos: app sorda, PIN que se autorizaba solo, parser de un protocolo que nadie habla, y trabajo sin interfaz | **un `ABORTADO` no se apunta para luego: se arregla antes de mirar nada más** |
| **N-93** (31/08) | tres cifras de la app estaban escritas a mano y eran viejas, y `documentos_01` **no vigilaba ninguna de las tres** | la fila existía —la cobertura la veía— y su número no lo miraba nadie. **Un hueco no grita** |
| **N-112** (01-02/09) | el propio pack **alternaba**: 16/1, 17/0, 16/1 sobre un árbol idéntico, porque emitía menos comprobaciones cuando el acta salía en rojo | **el número de comprobaciones que emite un pack no puede depender de su propio veredicto** |

> ⚠️ **Y lo que N-75 dejó abierto y sigue abierto**, porque son decisiones y no código:
> el **modo día de fondo claro** contra el sol directo (es la única intervención demostrada; el
> contraste WCAG ya está medido y en AAA salvo el rojo) y los `prompt()` nativos para crear y
> renombrar cruces.

---

### 🟡 El orden de ejecución vigente — seis fases

| Fase | Qué | Estado |
|---|---|---|
| ~~**1**~~ | Los comandos que faltaban en el Maestro: `SET_MODO:DEGRADADO`, `MENU`, `ALCANCE`, `INTELIGENTE`, `REINICIAR_RELOJ` y `DEMANDA` | ✅ **HECHA** en `d34cfe2` (N-78) |
| **2** | ~~Ignorar los pulsadores~~ → **ignorar SÓLO los 3 y 4** (`PB14`, `PB15`) · `FORZAR_ROJO` del Esclavo · `TEST_LEDS` | 🔴 **La redacción anterior era el peligro concreto de esta tabla: ejecutada literal BORRA `ambarLocal` y con él el veto de SFTY-21** |
| **3** | **Cámaras a `J16`** (p10/p12) y retirar pantalla, menú y `AiBus` | ✅ el cableado ya no está bloqueado (`M3` cerrada) |
| **4** | **Telemetría honesta** | `$STATUS` es el único tablero que existe y aún trae campos que no se miden. **Un campo que no se mide se retira o se marca; no se deja con aspecto de medida** |
| ~~**5**~~ | ESP32: watchdog, `DS3231` y puente Bluetooth | ✅ **HECHA** — `ESP32_Expansion/src/vigilante.cpp` |
| **6** | **BANCO** | 🛑 **Sigue siendo EL bloqueante y nada lo sustituye.** Ni la compuerta en verde, ni los arneses que compilan C++ real, ni esta hoja de ruta |

#### Lo que queda por hacer, después del banco

| # | Qué | Depende de |
|---|---|---|
| **C1** | **SFTY-29: presencia como veto** | ~~decidido el 27/08: van las 4 cámaras~~ ~~⛔ **REVOCADO el 28/08: van DOS**, y con ello desaparece el sujeto de SFTY-29~~ (derogado por `D-25`) · ⚠️ **11/09: `D-25` vuelve a CUATRO, dos por poste (`J16` p10 y p12)** — misma configuración para todas (`D-13`; los valores, del manual del modelo por `D-27`) y **ninguna veta nada**: el veto de la pluma sigue siendo `A-1.bis`, sin construir |
| **C2** | Reloj `DS3231` por I²C en el STM32 | ⛔ **anulado**: el reloj vive en el ESP32 (`D-9`) |
| **C3** | **`FW-PAIR`** (byte `PAIR`, `SET_PAIR`, descarte de lo ajeno) | el más caro: toca el respaldo `DR9`, la `FIRMA` y `maestro_02_respaldo` |
| **C4** | **`FW-N53`**: decidir secuencias | es **decisión de spec**, no código |
| **D3** | **Campo**: Courier RTC en sitio y puesta en servicio | **sólo con banco pasado, sin excepción** |

> **Sobre lo que queda manda el flash:** ~~el Maestro va al **88.6 %** y quedan **7.448 B libres**.~~
> *(11/09: cifra copiada a mano el 08/09 y ya desfasada del acta; la vigente es la de la tabla de
> arriba, que sale de `ls -t evidencia/*_compuerta.txt | head -1`)* No caben todas. Se mide antes de escribir cada una, no después — `CLAUDE.md` §7.

## 3. Las cuentas

Medidas por `hacer_historia.py` (scratchpad de la sesion del 28/09) al generar este fichero; lineas fisicas, y entre
parentesis las no vacias.

| | lineas |
|---|---|
| `CLAUDE.md` antes -> despues | 541 -> 200 |
| de `CLAUDE.md`, entran aqui literales | 538 (454) |
| de `CLAUDE.md`, borradas por estar en el metodo global (§4) | 3 (3) |
| suma | 541 = 541 |
| de las que entran aqui, las que ademas siguen literales en `CLAUDE.md` nuevo | 24 |
| `ESTADO.md` antes -> despues | 722 -> 109 |
| de `ESTADO.md`, se quedan literales (lineas 1-33 y 546-561) | 49 (46) |
| de `ESTADO.md`, entran aqui literales | 673 (601) |
| suma | 722 = 722 |
| de `ESTADO.md`, borradas | 0 |

Comprobado al generar: cada linea que salio de los dos ficheros, salvo las del §4, esta aqui tal cual (con `#`
delante si era un titulo). Lo que el `CLAUDE.md` nuevo dice en pocas lineas es un resumen de este texto; la
mecanica que no cabia esta reescrita en `.claude/particularidades/metodo.md`.

## 4. Borrado por estar ya en el metodo global de Diego (`~/.claude/CLAUDE.md`, «Como trabaja Diego»)

| lineas del `CLAUDE.md` original | que decian | regla global que lo cubre |
|---|---|---|
| 500-502 | §16.1: nada entra sin que algo salga; el commit dice que retira | «Cada norma nueva retira otra ... Si no hay ninguna que quitar, no entra» |


## 5. La regla que retiro la cadencia (`CLAUDE.md` §4)

Salio del §4, recuadro de `correr.py`: *«Una cifra del banco no autoriza un commit — antes de comitear se corre
`compuerta.py`, completo.»* La cadencia la sustituye: segun lo que se toca, basta la app, el `--rapido` o la
completa una vez; la completa dos veces queda para la candidata a paquete.
