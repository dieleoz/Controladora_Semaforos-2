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

## 6. Filas cerradas que salieron de `roadmap.md` (`925c71e`, cierre de sesion)

Las 25 filas tachadas o CONSTRUIDAS de `roadmap.md`, literales y en su orden; las de mas de 600 caracteres,
partidas en lineas de 118 (el texto no cambia). Lineas de origen en `925c71e`: 161, 162, 164, 169, 171, 172, 173, 187,
188, 189, 197, 200, 201, 209, 213, 216, 231, 233, 234, 242, 243, 372, 373, 402, 507.

- | ~~**1.50**~~ | 🟢 **CONSTRUIDA EL 16/09 (sin comitear aun).** — 🔴 **era: LA APP ESCRIBIA ORDENES A UN ENLACE MUERTO
  Y NO LO DECIA — SEIS `FORZAR_ROJO` SE PERDIERON ASI el 16/09.** Dos ventanas sin un solo `$STATUS`:
  **11:43:52→11:44:43 (56 s, 18 ordenes)** y **11:47:53→11:49:08 (75 s)**; **24 ordenes sin respuesta** en la sesion.
  La app **ya tiene el dato** —calcula «el ultimo `$STATUS` es de 37,9 s antes de la orden»— y aun asi solo ensena
  `SIN RESPUESTA`, que su propio encabezado define como *«no quiere decir que rechazara la orden»*. El tecnico apreto
  el rojo de emergencia seis veces creyendo que lo mandaba | §2 (el `$ACK` que no depende de la llamada, por el lado
  de la app) | `05_Funcional/App_Semaforo/app.js` y **sus cuatro copias** (`CLAUDE.md` §14) | la cinta del 16/09:
  11:44:12–11:44:25, seis `--> [ENVIADA] CMD:FORZAR_ROJO` seguidos y **ninguna** `<--` entre ellos ——— 🟢
  **ARREGLADO:** el aviso sale **en el momento**, con la medida (*«el equipo lleva N s sin hablar»*), y **NO se
  bloquea el envio** —el enlace puede volver en el byte siguiente y tragarse un `FORZAR_ROJO` es peor que mandarlo a
  ciegas—. El diario rotula esa orden `ESCRITA SIN ENLACE` y **el RESUMEN la cuenta**, que es lo que separa «el equipo
  no obedecio» de «el equipo no estaba». **Reusa `TIMEOUT_ENLACE_MS`**, el mismo borde que pinta «Sin enlace» en la
  cabecera: un umbral propio seria una segunda respuesta a la misma pregunta. **VISTO FALLAR en las dos direcciones**
  (`test_dom_execution.js`): con la guarda clavada en `true` cae la mitad (a) y con ella en `false` cae la (b) —310 →
  307 y codigo 1 las dos veces—, restaurado desde copia previa y **verificado por `sha256`**. Los 12 packs de la app:
  **234/234** |

- | ~~**1.51**~~ | 🟢 **CONSTRUIDA EL 16/09 en `a55054a`** — era: **EL EQUIPO NO DECIA QUE FIRMWARE LLEVABA, Y POR ESO
  NINGUNA FOTO DE CAMPO SE PUEDE LEER** (§0.2: *«una foto de campo sin el hash de lo que habia dentro no se lee»*). El
  `$STATUS` lleva `SERIE:` y **no** version; el parte del puente solo `PERRO:ARMADO,WDT_MS:2000`. **No cabe en el
  `$STATUS`** —`payload[155]` esta en el borde del cable (`bluetooth.cpp:1420-1426`)—: va en su propio `$ACK` a un
  `CMD:VERSION`, o pegado al parte de arranque | §0.2 · §7 | `Maestro/src/bluetooth.cpp`,
  `ESP32_Expansion/src/vigilante.cpp`, y la app para ensenarlo | `grep -rn "VERSION\|FW_VER\|GIT_HASH\|__DATE__"
  01_Firmware/Maestro/{src,include}` da **cero** ——— 🟢 **HECHO:** las tres puntas contestan `CMD:VERSION` con el
  commit corto y **`+SUCIO`** si el binario se hizo sobre trabajo sin comitear; sin sello contestan
  `$ERR,DESC:SIN_SELLAR` con `FW:--` **en vez de `OK`** (§2). El puente lo saca en su propia `$EVENT,EVT:VERSION` para
  no reclamar el comando y dejar mudo al equipo. Flash **+96 B** Maestro, **+116 B** Esclavo, **+60 B** ESP32; **RAM
  0**. Lo vigila `version_01_sello_de_firmware` (**42 comprobaciones**), visto fallar con dos defectos inyectados: un
  `$ACK` que no depende de lo que se pudo componer, y **un espacio antes de un `;` en `platformio.ini`** que recorta
  la orden de sellado y deja el binario sin sello con el unico sintoma de que el equipo dice que no lo sabe. La app
  pregunta desde el boton *Consultar version del firmware*. ⚠️ **Y una consecuencia que hay que saber: el sello mira
  `git status`, asi que la cifra de flash solo es comparable con el ARBOL LIMPIO** —editar un `.md` le anade 8 B al
  binario— |

| ~~**1.53**~~ | 🟢 **CONSTRUIDA EL 28/09** — era: **LA PRUEBA DE FOCOS DABA VERDE CONTRA VERDE EN SERVICIO** (cinta de Marco, Maestro `4D2007`; medido con `coordinador`+`semaforo`+`modo_automatico` reales: 2 s de verde del Maestro con `ESC:VERDE`) y al acabar dejaba la luz en rojo con `ESTADO:VERDE`. Ahora `TEST_LEDS` solo corre en MENU/HORA/ALCANCE con el rojo del Esclavo acusado, y devuelve la luz del estado; en servicio, `$ERR` con su motivo. Lo vigila `maestro_09`. Residual escrito en `SPEC_1` §10: el ambar de emergencia pulsado en el Esclavo durante el test |

- | ~~**1.22**~~ | 🟢 **CONSTRUIDA Y EN `main` EL 14/09.** El bucle hace ahora **la segunda visita al contador** en
  cada vuelta: si no cambia en la ventana, baja la bandera del reloj y el getter vuelve a devolver 0 —que es el *«no
  hay reloj»* del que YA cuelgan los dos centinelas de `respaldo.cpp`, o sea **cero mecanismos nuevos**—. Ventana
  **2.051 ms DERIVADA** del tick fisico de 1 Hz, con **dos** flancos porque la primera lectura tras arrancar el RTC
  puede venir rancia —un falso positivo aqui le tira la hora al equipo—, inflada por el peor caso del oscilador
  interno y con `static_assert`. 🔴 **Y DOS MODOS DE FALLO QUE LA PROPIA CURA CREABA, cerrados:** el reintento de
  `N-25` la deshacia cada 30 s —solo mira el bit de arranque, que en este cristal vale 1— y ahora hay un cerrojo que
  **solo quitan una persona o un reinicio, nunca la maquina**; y la hora adoptada del RTC se daba por fiable **para
  siempre** sobre la premisa que este cristal rompe. ✅ **Y el arnes PUEDE VERLO POR FIN:** el defecto vivia dentro del
  modelo del silicio —el contador se derivaba de `millis()`—, asi que no era un escenario que faltara sino **un estado
  inexpresable**. Con la perilla de congelacion, 53/53 → **59/59**; con el defecto inyectado, **56/59 y codigo de
  salida 1**, y ahi se mide la mentira: `respaldo_horasDesdeSync()` contesta **0 h**. Coste +184 B / +172 B. ⚠️ **Sin
  banco y sin tarjeta.** ⬇️ *lo que decia:* 🔴 **CADA CORTE DE LUZ REGALA EL PLAZO ENTERO DEL DEGRADADO — y la puerta
  que se abre no es la de 48 h, es la de 2 h.** ⬇️ **REDACTADA DE NUEVO EL 12/09 CON LA MEDIDA DELANTE: mi version
  anterior acusaba al defecto equivocado y tres de sus cuatro afirmaciones eran falsas. No se borran, se marcan
  refutadas** (`CLAUDE.md` §7.4) **— la causa que desaparece en silencio se vuelve a proponer, y la segunda vez nadie
  recuerda que se comprobo.** 🔴 **EL MECANISMO, reproducido en el fuente:** el cristal tiene **TRES** estados y el
  firmware solo distingue dos. `arrancarCristal()` pone `rtcOperativo = true` en cuanto `LSERDY` sube, con el
  comentario *«N-24: a partir de aqui el RTC cuenta»* — pero **`LSERDY` dice que el oscilador ARRANCO, no que `CNT`
  INCREMENTE**, y el tercer estado —`LSERDY` arriba con `CNT` quieto— **es el de la cinta del Sisga**. Con el,
  `respaldo_horasDesdeSync()` resta dos lecturas iguales y da **`0 h`**, o sea *«acabo de hablar con el otro poste»*,
  sobre un acuerdo que puede ser de hace meses. 🔴 **Y el centinela que deberia taparlo lo destapa: `return v == 0 ?
  1UL : v;`** convierte un `CNT` congelado en **exactamente 0** —el caso del `179DB0`— en un **`1` NO nulo**, que pasa
  los dos centinelas de `respaldo.cpp`. ⬇️ **LO QUE SE REFUTA de mi redaccion vieja, cada cosa con su medida:** ~~«NO
  VENCE NUNCA»~~ → **vence con el equipo encendido**: `msDesdeSyncEfectivo()` toma el MAYOR de RAM y pila (`return
  (msPila > ms) ? msPila : ms;` en las dos puntas) y `millis()` si corre. **Lo que no sobrevive es el RESET**: al
  arrancar no hay RAM y solo queda la pila · ~~«el Maestro tiene CERO instrumentos»~~ → **ONCE packs y DOS arneses**
  abren sus tres ficheros, contra nueve del Esclavo: **es la punta MEJOR instrumentada**, falso por el lado contrario
  al que temi · ~~la pista de «dos afirmaciones que no pueden ser las dos ciertas»~~ → **no hay contradiccion viva**:
  el parrafo de `millis()` que yo citaba esta DENTRO de la rama `return 0` y **describe codigo RETIRADO** (la
  extrapolacion que `D-20` puso y `N-160` quito). La premisa mala no esta ahi, esta en `reloj_setup()` · 🆕 **y lo que
  la fila NO decia y es lo peor: la puerta de entrada del Maestro exige `SYNC_FRESCA_MS` = 2 h, no 48**, y tras un
  reset lee `0 ms` por este mismo camino: **la barrera que se pierde es VEINTICUATRO VECES mas estrecha que la que yo
  miraba** | `N-160`, `N-162` `H8`, `D-20`, **`D-29`** · ~~§3.16-D~~ **(⚠️ y §3.16-D hay que corregirlo: dice que el
  `00:00:00` del `179DB0` «cae del lado seguro» y NO esta garantizado — lo tapa el `? 1UL :`)** | **el arreglo barato
  NO toca `respaldo.cpp`:** `{Maestro,Esclavo}/src/reloj.cpp` + su `.h` — muestrear `CNT` y bajar `rtcOperativo` si no
  cambia. ⚠️ **Las otras tres formas medidas SI chocan**: dar un segundo dato al respaldo cambia el mapa de registros
  del dominio de respaldo y rompe la identidad byte a byte entre puntas; y exigir RAM en la puerta **contradice `D-29`
  de frente** —`D-29` existe para que un corte no mate la reanudacion, y se apoya justo en la marca de la pila—, asi
  que **eso no es una orden, es una fila nueva del responsable** (`CLAUDE.md` §11.1) | 🔴 **NINGUN instrumento puede
  cazarlo hoy, y no es un olvido: es que el defecto vive DENTRO del modelo del silicio.** En
  `Validacion_Automatico/dos_puntas/reloj_real/stm32f1xx_hal.h`, `arnes_rtc_cnt()` **deriva `CNT` de `millis()`**, asi
  que el contador congelado no es un escenario que falte — **es un estado que no se puede expresar**. El arnes lleva
  su borde ESCRITO y bien (`CLAUDE.md` §7): declara «cristal vivo» y «Y2 muerto»… **y el defecto vive en el tercero**.
  La perilla de congelacion que hay que anadirle **es a la vez el control negativo** que §6 exige antes de conectarlo.
  Los modelos Python arrastran la misma premisa: `maestro_03_puerta_degradado` alimenta la resta con un contador
  derivado del calendario, que **por construccion siempre avanza** |

- | ~~**1.1**~~ | ~~🟠 **EN CONSTRUCCION (worktrees, 11/09): el ESP32 manda la hora** —siembra ESP32→STM32 desde el
  `DS3231` al arrancar, tras cada `SET_RTC` bueno y cada ~5 min; el `SET_RTC` del telefono deja de cruzar al STM32; en
  el Esclavo manda la radio; `esp32_05` se estrecha a una excepcion—~~ 🟢 **CONSTRUIDA Y EN `main` desde `68dd2c5`**
  (11/09 por la tarde, merge de `a0313fe`, revisado por el diff por el arquitecto: «fusionar con cambios», §3.16
  `N-162`). **Medido sobre `68dd2c5`, filtrando comentarios:** la cadencia es `#define SIEMBRA_INTERVALO_MS 300000UL`
  (`ESP32_Expansion/include/contrato.h`; ~~`INTERVALO_SYNC_MS`~~ deja de reusarse **a proposito**, y en
  `ESP32_Expansion/` solo sale en dos comentarios que lo explican); `siembra_revisar()` se llama en el `loop()` de
  `ESP32_Expansion/src/main.cpp` y `siembra_ahora()` dentro de la rama `RELOJ_OK` del `SET_RTC` en `despachador.cpp`;
  `grep SET_RTC` sobre `{Maestro,Esclavo}/src/bluetooth.cpp` → **0 lineas de codigo** (la rama la sustituye
  `CMD:HORA_ESP32:`); `grep "rtc\.set"` sobre los dos `reloj.cpp` → solo `setClockSource()` y el `setMonth(1)` de
  `reloj_fijarEnero()`, que desde `68dd2c5` sale si hay base sembrada (`if (tBaseMillis > 0) return;`). **Cierra en el
  fuente §3.1-9 y la congelacion de ~3 s del Maestro.** ⚠️ **Sin banco y sin tarjeta** (§5), y **lo que deja abierto
  bloquea CAMPO**: `H1` → fila **1.13**; los instrumentos → **1.14** | `D-20`, `D-26`, `N-162` | — | ~~`grep
  INTERVALO_SYNC_MS ESP32_Expansion/src` → **0**; `reloj_ajustarConAcuse()` sigue llamando a `rtc.setHours()` con
  `rtcOperativo` en `true`~~ *(las dos medidas eran de `b79d904`; sobre `68dd2c5` la siembra existe con otro nombre y
  `reloj_ajustarConAcuse()` ya no escribe el RTC)* |

- | ~~**1.2**~~ | ~~🔴 **`AMBAR_EMERGENCIA` sin PIN no avisa al Maestro**, y el pack que mire las DOS puertas~~ 🟢
  **CONSTRUIDA y en `main` el 12/09 (`913c29c`)** — ⚠️ **sin banco y sin tarjeta**. Las dos puertas avisan, y el
  `$ACK` dice si el aviso pudo oirse: `OK_SIN_RADIO` / `YA_EN_AMBAR_LATCH_PUESTO_SIN_RADIO` cuando esta punta ya
  declaro `FALLO_RF`, leyendo `enlaceCaidoAnunciado` —el mismo dato con el que ya se publica esa alarma, no un segundo
  reloj de silencio (`CLAUDE.md` §2)—. **Y los TRES instrumentos que lo dejaban pasar, arreglados**: `esclavo_07`
  deduplicaba por NOMBRE y las dos ramas se llaman igual; `esclavo_08` comparaba seis prefijos y ninguno era
  `protocolo_`; y el arnes de dos puntas ejercia una **TRANSCRIPCION** de la puerta CON PIN escrita en su propio
  adaptador —dos copias buenas de una puerta mala, `CLAUDE.md` §8—: se retira y se compilan el `bluetooth.cpp` REAL
  del Esclavo y el `modo_ambar.cpp` REAL del Maestro, el telefono teclea la linea **leida del C++** y el `$ACK` que
  recibe pasa a ser observable. Bloque **H** nuevo (H0..H4): **76/77 → 85/86**. ⚠️ **Lo que NO cierra, y lo publica H4
  como nota que no cuenta:** si muere solo el transmisor del Esclavo, el aviso no sale, esta punta **no puede
  saberlo** —solo oye silencios de lo que RECIBE— y el `$ACK` sale igual que con la radio sana. Cerrarlo pide un
  **acuse al aviso**: protocolo y LAS DOS puntas, como el `ACK_RED` de `G9` | `N-142`, §3.16-A | hecho: `913c29c` |
  ~~`protocolo_enviarPaquete(CMD_AMBAR_ESCLAVO)`: **una sola llamada** en `Esclavo/src`, dentro de la puerta CON PIN~~
  → **dos**, una en la rama comparada contra `cmd` (sin PIN) y otra en la de `accion` (con PIN), medidas sobre
  `913c29c` filtrando comentarios |

- | ~~**1.3**~~ | 🟢 **CONSTRUIDA Y EN `main` EL 13/09 (`06f126e`)** — ⚠️ **sin banco y sin tarjeta**. El mecanismo lo
  eligió el responsable (`D-32` (2)): **`$EVENT` periódico**, no aviso ESP32→STM32 —esa habría sido la **tercera orden
  que el accesorio origina hacia el micro**—. El poste 2 publica ya sus tres contadores de enlace cada **30 s**, y
  **la cadencia no la eligieron los bytes**: la eligió la bitácora de la app, que `addEvent()` recorta a **30
  entradas**, así que a 10 s este diagnóstico se comería los `$ALARM` que el técnico vino a leer. Caudal 51,8 % → 63,9
  %. `decisiones_01_anclas` deja de acusar a `D-23` **con código detrás, no con un ancla a mano**. 🔴 **Y de
  construirla salió lo que más vale: `P-2` comparaba contra un buffer que NO acumula ninguna ráfaga**
  —`BUF_SALIDA_APP` tiene un solo usuario y es un array de pila donde el puente compone UNA trama suya—. Reescrita
  hacia el borde real, y ahí apareció lo que nadie vigilaba: **el puente descarta la trama ENTERA pasados 159
  caracteres y el `$STATUS` más largo del Maestro ocupa 157. MARGEN 2**, y la trama no se trunca: **desaparece**. Todo
  derivado, ni un 157 ni un 159 escritos a mano. ⬇️ *lo que la fila decía antes:* ~~🔴 **`D-23` — la VISTA DE LA APP
  para el poste 2**~~ **`D-23` ESTA CONSTRUIDA CASI ENTERA, y esta fila lo daba por ausente.** Medido el 12/09 al
  decir el responsable *«creo que ya la hicimos»*: la app **ya se adapta al poste al que esta conectado** (`app.js`:
  `state.node === 'ESCLAVO' ? 'POSTE 2' : 'POSTE 1'`, con rotulos y mensajes propios), el Esclavo **ya publica
  `$EVENT` en DOCE situaciones** —ordenes de la app, hora del ESP32, salto del Degradado, rechazos— y su `$STATUS`
  trae `MODO`, `ESTADO`, `HORA`, `PLUMA` y `CAM`, que es el diagnostico de esa punta. 🔴 **LO QUE DE VERDAD FALTA SON
  DOS COSAS, y esto es lo unico que queda de la fila:** **(a)** *«emitido tambien AL CONECTAR»* — **ninguno de los
  doce lo dispara la apertura de la conexion**, asi que el tecnico que se conecta al poste 2 **no recibe nada hasta
  que pasa algo**; ~~**(b)** la **antiguedad de sincronizacion no viaja**: once campos en el `$STATUS` y ninguno la
  lleva.~~ ⬇️ 🛑 **(b) SE TACHA EL 12/09 POR LA NOCHE: NO ES LO QUE `D-23` PIDE, Y ADEMAS NO CABE. El error era MIO, en
  el encargo, no del roadmap** — lance un agente a construirla sin leer el **cuerpo de `A-14`**, que ya lo habia
  medido y decidido el 08/09 (`CLAUDE.md` §11.1: si el encargo contradice una fila, **eso no es una orden, es una
  pregunta**). Lo que dice `A-14`, reproducido hoy con su instrumento: **🛑 un campo nuevo en el `$STATUS` NO ENTRA,
  POR 3 BYTES** —el peor `$STATUS` del Maestro son **151 caracteres y `payload[155]` guarda 154**—, y `documentos_03`
  **obliga a que el campo este en las DOS puntas**, asi que manda el margen del Maestro y no el del Esclavo. Y **el
  dato tampoco es la antiguedad**: `A-14` escribe cual es el hueco —*«el poste 2 nunca dice como ve EL el enlace»*— y
  los tres getters ya existen y son publicos (`protocolo_bytesRecibidos/tramasValidas/tramasDescartadas`), saliendo
  hoy **solo dentro del `$ALARM`, o sea cuando el enlace ya se cayo**. La via decidida es **`$EVENT` nuevo**, no campo
  en el `$STATUS`. 🆕 **Y una que no tenia fila: CUATRO de esos once campos —`T`, `RF`, `RTT`, `BAT`— son `--` FIJOS**,
  siempre, en todas las tramas** *(medido: son **25** caracteres, no 24)*. ~~Es el sitio natural de la antiguedad de
  sincronizacion que no viaja (§3.5-4).~~ 🛑 **NO SE RETIRAN, y esto tambien se midio el 12/09 al intentarlo:**
  `state.countdown` y `state.battery` de la app **solo se escriben dentro de** `if (data.T !== undefined)` y `if
  (data.BAT !== undefined)`, y **nadie los limpia al desconectar ni al cambiar de poste**, asi que quitarlos dejaria
  pintadas la cuenta atras y la bateria **del poste 1 sobre la pantalla del poste 2**, como si fueran de ahora. Hoy el
  `T:--` los pone en `null` correctamente, y el propio C++ ya lo razona: *«SE MARCAN, NO SE RETIRAN»*. 🔴 **LO UNICO
  VIVO DE ESTA FILA ES (a), Y ESTA BLOQUEADO EN UNA DECISION, NO EN TECLADO.** El STM32 **no puede saber** que un
  telefono se conecto: el unico que lo sabe es el ESP32 (`spp.hasClient()`). Y un aviso nuevo ESP32→STM32 seria **la
  tercera linea originada hacia el micro**, que `esp32_05_no_origina` condiciona **por escrito** a *«una decision
  escrita en DECISIONES.md, no un comentario»* — porque es *«una orden que el accesorio manda por su cuenta a un micro
  que gobierna un cruce y que no valida quien habla»*. **Ensancharla desde un agente seria apagar la barrera con un
  comentario** (`CLAUDE.md` §1). Las dos vias medidas, y la eleccion es del responsable: **(i)** la tercera linea
  originada, con su fila y su gemela anti-suplantacion como `HORA_ESP32` — trabajo pequeno una vez escrita; **(ii)**
  que el STM32 no se entere de nada y el `$EVENT` salga **periodico a cadencia baja** — no toca el ESP32, no toca
  `esp32_05`, y el caudal lo aguanta (**51,8 %**, 497 B de 960 B/s), pero contradice el *«no gasta periodico»* con el
  que se eligio la via ⚠️ **12/09: se retira de aqui el nombre «las dos pantallas» (§6.9), que era el concepto VIEJO y
  de LCD** —aquel diseño leia `lcd.cpp` de las dos puntas—. **Las dos LCD no existen** (`D-17.bis` retiro el hardware,
  `D-30` saca el software), asi que llamar «pantalla» a esto invitaba a leerlo como trabajo de pantalla fisica. `D-23`
  es **de la app**, y su propia fila lo dice: *«lo que se ve cuando el telefono se conecta por Bluetooth DIRECTAMENTE
  a ese poste»* | `D-23`, `A-14` | `Esclavo/src/bluetooth.cpp`, `app.js` (sus cuatro copias), la APK |
  `decisiones_01_anclas` acusa `D-23`; el `$STATUS` del Esclavo no lleva ningun campo de sincronizacion |

- | ~~🆕 **1.17**~~ | 🟢 **CONSTRUIDA y en `main` el 12/09 (`9192063`)** — ⚠️ **sin banco y sin tarjeta**. `dos_puntas`
  **106/106 -> 110/110**: `H4` deja de ser nota y pasa a comprobacion, y entran `H5` (control con el transmisor sano),
  `H2` (segunda pulsacion: «ya avisado» y NINGUNA alarma) y `H6` (que el cancelar borre la memoria). 🔴 **El reintento
  esta DERIVADO y se prueba por los dos lados: con `2` NO COMPILA y con `0` tampoco.** Y un peligro que el diseño no
  nombraba, hallado al construirlo: **sin parar el plazo al cancelar, el reintento habria reenviado el armado DESPUES
  de la cancelacion**, devolviendo al Maestro a ambar. **DECIDIDO Y ESPECIFICADO ENTERO el 12/09 (`D-31`), y el COMO
  esta cerrado — no se vuelve a preguntar.** El primer diseño se descarto porque **construia algo PEOR que el
  defecto**, y las tres correcciones salieron de preguntas del responsable: **(1)** el acuse se espera **solo del
  PRIMER aviso** —lo demas diria «nadie me oyo» con la radio sana cada vez que alguien pulsa dos veces, y asi
  `SFTY-21` no se toca—; **(2)** el **cancelar entra en el mismo lote**, porque es lo que borra esa memoria y porque
  su ventana es peor —al cancelar el Maestro esta callado y la red de agotar reintentos no aplica—; **(3)** espera
  **`TIMEOUT_ACK_MS` (3,5 s)** y el ambar **no espera** —`$ACK` inmediato, desmentido posterior, patron de `N-130`—; a
  8 km **la distancia sube la PERDIDA, no la latencia**, asi que la cura es repetir: **UN reintento, derivado de lo
  que quede del presupuesto** (`costura_09` lo recalcula; hoy van 20,8 s de un techo de 25); **(4)** la alarma de la
  app **no puede decir «el otro no se entero» sino «NO HE PODIDO CONFIRMARLO»**, y entra `app.js` en sus cuatro copias
  mas la APK. **El detalle entero, en `D-31`.** ⚠️ **Y donde muerde es al reves de como suena, medido al decidirlo:**
  la rama principal del ambar de emergencia es `if (!degradado_gobiernaLuz())`, o sea que **actua en OPERACION
  NORMAL** obedeciendo al Maestro; solo cuando el Degradado gobierna la luz el ambar **se encola**. **El hueco esta en
  el caso COMUN, no en el raro.** *(De paso: los modos son del MAESTRO —ocho—; **el Esclavo NO TIENE modos**, su
  `modos.h` no existe.)* ✅ **Y una mitigacion que anade el responsable y NO sustituye al acuse:** la tarea de
  mantenimiento —*«el operador va al semaforo y se conecta y sabra esa alarma»*—; pero esa alarma la ve **quien se
  conecte a ESE poste**, y entre que el transmisor se rompe y alguien va, **el otro poste sigue dando verde**. El
  acuse cierra la ventana; la visita solo la descubre despues. 🟠 **EL AVISO DEL AMBAR DEL ESCLAVO NO SE ACUSA, y por
  eso hay una averia que nadie puede ver.** Medido el 12/09 en el bloque **H4** del arnes de dos puntas, que lo
  publica como **nota que no cuenta**: si muere **solo el transmisor** del Esclavo, `CMD_AMBAR_ESCLAVO` no sale, **esa
  punta no puede saberlo** —su unico dato de radio es el silencio de lo que RECIBE, y el Maestro le sigue hablando— y
  el `$ACK` al telefono sale **identico al de la radio sana**. El tecnico se va del poste creyendo que el Poste 1 se
  entero, que es justo lo que el `SIN_RADIO` de 1.2 vino a evitar en el caso que SI se puede detectar. ⚠️ **Cerrarlo
  CAMBIA EL CONTRATO DE LA RADIO** (como 1.8): pide un acuse al aviso en **las dos puntas**, del mismo tipo que el
  `ACK_RED` sin identificador de `G9` · 🆕 **(b)** y el arnes **no ejerce la mitad de VUELTA de `N-152`**
  —`CMD_CANCELA_AMBAR_ESCLAVO`, que en `main.cpp` lleva el cruce a `MODO_MANUAL`—: `modo_manual.cpp` no se compila en
  esa DLL, y transcribir su destino seria medir un doble otra vez. Hoy lo mide `costura_14` **por texto** | `N-142`,
  `N-152`, §3.16-A | (a) `{Maestro,Esclavo}/include/protocolo.h`, los dos `bluetooth.cpp`, `coordinador.cpp` · (b)
  `Validacion_Automatico/dos_puntas/adaptador_maestro.cpp` | la nota `[NOTA] H4` del arnes y la cabecera del
  `adaptador_maestro.cpp`, las dos escritas al medirlo |

- | ~~🆕 **1.18**~~ | 🟢 **CONSTRUIDA y en `main` el 12/09 (`6c25bda`)** — ⚠️ **sin banco y sin tarjeta**. `D-28` (2):
  **el plazo de caducidad de la hora pasa a cubrir DOS siembras perdidas.** Deja de derivarse del relevo y se deriva
  de `3 x cadencia` inflado por el HSI (369 s) -> deriva concedida 10 s -> `HORA_CADUCA_MS` **400 s** (hoy 280). **Lo
  que cuesta, medido y aceptado por el responsable:** el desfase relativo del cruce sube de 16 a 22 s contra un
  aguante de **29**, o sea que el margen para lo que difieran los dos `DS3231` **baja de 13 s a 7 s**. Sigue positivo,
  y el techo lo recalcula `reloj_04` en cada corrida. ⚠️ **El `static_assert` de minimalidad CAMBIA DE SUJETO** —el
  menor que cubre el caso peor, que ya no es el relevo—; el suelo del relevo se conserva porque vigila otro termino |
  `D-28`, `D-21` (1), `D-26` (2) | los dos `reloj.h`, `reloj_04`, `esp32_13` | la cuenta rehecha con `_aguante()` de
  `esp32_13` y `_relativa_s()` de `reloj_04`: 271000 -> 7 s -> 280000 -> margen 13 · 369000 -> 10 s -> 400000 ->
  margen 7 |

- | ~~🆕 **1.19**~~ | 🟢 **CONSTRUIDA y en `main` el 12/09 (`42fead0`)** — ⚠️ **sin banco y sin tarjeta**. `D-29`:
  **N-20 MURIO EN EL ESCLAVO Y SE RECONSTRUYE.** Desde `N-162` (11/09) la siembra no escribe el RTC hardware, asi que
  `reloj_setup()` deja `horaValida` en `false` **tras cada corte**, la primera puerta de
  `degradado_reanudarTrasCorte()` cierra, **y en ese mismo arranque se borra el indicador de la pila**: la hora del
  ESP32 llega en el `loop()` un segundo despues y ya no hay nada que reanudar. 🔴 **NADIE LO DECIDIO: es un efecto
  colateral de `D-20`/`D-26`**, y el comentario del firmware que lo achaca a «sin cristal» se queda corto —pasa
  **tambien con el cristal vivo**—. Se difiere el borrado hasta despues de la primera siembra del arranque. **Lo que
  NO se toca:** la segunda puerta (el limite duro de 48 h) y la activacion MANUAL de `SFTY-21`. **Cierra de paso las
  DOS FLOTAS:** hoy una tarjeta cuyo RTC escribio un firmware anterior al 11/09 **si** reanuda y una recien grabada
  no, con el mismo binario y sin que el `$STATUS` lo distinga. ⚠️ **El Maestro tiene el mismo patron y CERO
  instrumentos**: el bloque D solo corta al Esclavo | `D-29`, `N-20`, `N-162` `H8` | los dos `modo_degradado.cpp` + el
  arnes de dos puntas | el bloque D del arnes lo reproduce (`D6b`/`D6c`/`D6d`), y su control `D8` demuestra que la
  reanudacion **sigue viva**: con el marcador del RTC puesto, el mismo escenario SI reanuda. 🔴 **Y TRES COSAS MEDIDAS
  AL CONSTRUIRLA, que corrigen lo que esta fila y `D-29` prometian de mas:** **(a)** `D-29` **NO alcanza a una tarjeta
  con el cristal `Y2` muerto** —ahi cierra la SEGUNDA puerta (`reloj_contadorSegundos()` devuelve 0 a proposito,
  `N-160`, y la marca sale `CADUCADA`) y el diferimiento ni se activa—, asi que **estrecha pero no cierra la
  divergencia de las dos flotas**; corregido tambien en `D-29`. **(b)** La ventana de 360 s abria una carrera que en
  `setup()` era imposible: el ambar puesto con el mando podia quedar pisado por la reanudacion. El responsable decidio
  el 12/09 **consultar `mando_ambarLocal()` en el camino diferido**; con el mando desmontado (`D-1`) la bandera no se
  arma nunca, y la guarda existe porque `J16` p5/p8 siguen **vacios y pelados**. ⚠️ ~~**`D-1` YA decidio que el codigo
  del mando SE QUEDA, con su medida —cinco llamadas vivas, el veto es SFTY-21, y quitar el armador deja los `if`
  ABIERTOS, no inertes—: no se vuelve a abrir esa pregunta.**~~ 🔴 **DEROGADO EL MISMO 12/09, unas horas despues, por
  `D-30` (`70c4c53`): el responsable revirtio la segunda mitad de `D-1` y el mando SALE del firmware.** *(No se borra
  la frase: la reabrio quien podia —una fila nueva del responsable, que es exactamente lo que `CLAUDE.md` §11.1
  exige—, y su MEDIDA sigue siendo cierta y es ahora el ALCANCE del trabajo, no una objecion: las llamadas vivas de
  `mando_ambarLocal()` son **seis** desde que esta fila 1.19 anadio la de `D-29`, y el veto sigue siendo `SFTY-21`.)*
  **(c)** 🔴 **La mitad MAESTRO se construye pero NO la ejerce ningun instrumento**: `compilar_dos_puntas.ps1` no
  compila su `modo_degradado.cpp` ni su `main.cpp`, y el arnes del Degradado usa un adaptador que **transcribe a
  mano** su `loop()`, asi que no lleva la llamada nueva. Entra sin banco que la mire |

- | ~~**1.30**~~ | 🟢 **CONSTRUIDA Y EN `main` EL 14/09: el mando esta FUERA, entero.** **-1.050 lineas netas** de
  firmware, -333 de modelos, -586 de packs. **Maestro 63,6% → 62,9% (-424 B); Esclavo 55,7% → 54,9% (-548 B).** Con el
  sale **el camino de interceptacion de las luces**, que era la condicion del responsable —o entero o vigilado, no a
  medias—, y **lo que queda vigilando ese sitio es MAS fuerte**: la lista blanca de quien puede escribir una lampara
  baja de **cinco funciones a UNA**, asi que un camino nuevo a los pines es ahora un ROJO en vez de una excepcion
  pre-aprobada por su nombre. 🔴 **Y el censo de esta fila estaba CORTO: se dijeron cuatro guardas colgando de la
  bandera del ambar local y son SEIS** —faltaban las dos de `CANCELAR_AMBAR`—. Ninguna abre paso: una colapsa a una
  condicion identica y otra retira una rama **inalcanzable desde que la bandera se quedo sin armador**, lo que de paso
  retira un literal del protocolo. ⚠️ **Lo que se fue sin sustituto, dicho sin suavizar:** el vigilante de senal de
  `N-52` —su sujeto entero era la bandera que sale— y el fuzz que agitaba modos por el mando. ⚠️ **Sin banco y sin
  tarjeta.** ⬇️ *lo que decia:* 🟡 **SACAR DEL FIRMWARE LAS BOTONERAS A/B/C/D — el responsable lo reafirma el 14/09:
  «ahora es por app»— Y ES MAS PEQUENO DE LO QUE ESTA FILA DECIA.** 🔴 **Las dos frases que lo frenaban se midieron el
  14/09 y son FALSAS:** ~~«es el unico escritor de la bandera de senal, asi que se lleva por delante parte de la
  barrera de salidas»~~ → la bandera la arman **dos funciones de `semaforo.cpp`** y sus **unicos** llamadores son
  `mando.cpp:154` y `:166`; sin ellos la bandera se queda en falso y la guarda `if (senalActiva) return;` **deja de
  disparar**, o sea **camino normal**: no abre ningun veto, deja codigo muerto. Y ~~«borrar `menu.cpp` borra el
  todo-rojo de las dos puntas»~~ → `coordinador_forzarMenu()` tiene **TRES** llamadores —`menu.cpp:76`,
  `modo_alcance.cpp:38`, `modo_hora.cpp:102`— **y los dos ultimos se alcanzan desde la app**. ⚠️ **Lo que SI queda, y
  es lo unico real:** el camino de interceptacion se queda **sin nadie que lo ejerza** (`CLAUDE.md` §6). Sale entero
  con el mando, o se queda vigilado. ⬇️ *lo que decia:* 🔴 **`D-30` — SACAR DEL FIRMWARE EL LCD Y EL MANDO A/B/C/D, y
  es MUCHO mas grande de lo que parecia.** Decidido el 12/09: el hardware de los dos se retiro (`D-17.bis` 28/08,
  `D-1` 05/09) y el software se quedo; el responsable lo revierte —*«arrastrar esas funcionalidades hoy pesa»*—. 🔴
  **MEDIDO AL IR A LANZARLO, y por eso no se pudo hacer en paralelo con `D-31`: `lcd_`/`menu_` se llaman desde CATORCE
  ficheros** —los diez modos del Maestro, su `bluetooth.cpp`, `botones.cpp`, `main.cpp` y `modos.cpp`, mas
  `botones.cpp`, `main.cpp` y `mando.cpp` del Esclavo—. No es «borrar tres `.cpp`»: es tocar casi todo `Maestro/src`.
  **Los cuatro obstaculos que `D-1` dejo medidos son el ALCANCE, no una objecion:** las **seis** llamadas vivas de
  `mando_ambarLocal()` —cinco de `D-1` mas la de `D-29`—, cada una resuelta explicitamente porque su veto es `SFTY-21`
  y quitar el armador **deja los `if` ABIERTOS**; los **trece packs** que leen constantes de `mando.cpp` **en el
  import** y caerian en **`ABORTADO`, no en rojo**; y el **arnes de pantalla**, que se retira con ellos y **baja la
  cuenta de la compuerta a proposito** —hay que fijar la linea de base nueva antes, o la red de «el total no puede
  bajar» deja de valer—. **El ahorro de flash se mide al hacerlo** (`CLAUDE.md` §10: los DOS extremos, por fichero
  objeto sobre el `.map`), no se estima aqui | `D-30`, `D-1`, `D-17.bis` | 14 ficheros de `{Maestro,Esclavo}/src`,
  `Validacion_LCD/`, 13 packs | `grep -rlE "lcd_[a-z]\|menu_[a-z]"` sobre los dos `src/`, 12/09 |

- | ~~🆕 **1.32**~~ | 🟢 **HECHO el 12/09 (`36b8b70`)** — y de paso salieron **dos textos caducados mas** que esta fila
  no nombraba: un mensaje **PASS** de una comprobacion CONTADA que decia *«puente: NINGUNO, nadie lo vigila»*, y la
  cabecera del censo, que afirmaba que `tUltimaLineaJ17` *«solo puede contar el silencio DESPUES de que se acabe»*
  —falso desde `D-26`, que lo lee DURANTE—. El plazo **no se teclea**: se DERIVA del `reloj.h` de las dos puntas y
  **ABORTA** si no puede. ~~**EL `reportar()` DEL ESCENARIO F5 DEL PUENTE YA NO ES DEL TODO CIERTO.**~~ Dice *«no hay
  umbral, ni alarma, ni comprobacion periodica»* sobre la muerte del ESP32, y **`D-26` lo cambio**:
  `horaEsp32Vigilar()` corre incondicionalmente desde el bucle y a los **6 min** (`HORA_ESP32_ESPERA_MAX_MS` = 3 ×
  cadencia) publica `$ALARM …CAUSA:J17_MUDO`. **A los seis minutos el equipo SI dice que el puente esta mudo.** El
  agente que arreglo el centinela lo midio y **NO lo metio en el censo a proposito** —es un vigilante de la HORA a
  escala de minutos, no del puerto a escala de segundos, y contarlo habria construido la acusacion falsa que venia a
  quitar—, pero dejo dicho que el texto de politica hay que corregirlo | `D-26`, F5 |
  `Simulaciones/simulador_puente_esp32.py` | medido el 12/09 al arreglar el centinela |

- | ~~🆕 **1.33**~~ | 🟢 **HECHO el 12/09 (`36b8b70`), y VISTO FALLAR**: inyectado un `strcmp(cmd,"CMD:FALSO")` en una
  funcion que caia dentro del exceso, el lector VIEJO lo contaba —censo 17— y el nuevo no —16—. 🆕 **Y habia un TERCER
  lector mal acotado que nadie habia nombrado**: `_pred` de `despachador_esParaElPuente()`, 251 caracteres contra 198
  reales. **Acertaba hoy por como esta escrito el C++, no por como esta escrito el lector**, y su fallo habria sido
  PARCIAL —perder constantes sin vaciar la lista—, asi que la guarda `if not reclamadas` no lo habria visto. Arreglado
  con el mismo helper. ~~`Contrato._cuerpo_despachador()` ACOTA MAL~~: usa `find("void bluetooth_loop")` como
  terminador de `procesarComando()` y se lleva **551 caracteres de mas en el Maestro** y **187 en el Esclavo**. **Hoy
  no cambia ningun resultado** —los cinco patrones que alimenta se comprobaron uno a uno—, pero alimenta el censo de
  comandos (16 Maestro / 8 Esclavo) y media docena de escenarios: el dia que alguien mueva una funcion, miente. Se
  deja fuera del arreglo del centinela a proposito (§8.3: un alcance que crece se corrige despues) | F5, `N-145` |
  `Simulaciones/simulador_puente_esp32.py` | medido el 12/09; los otros tres lectores de `bluetooth_loop()` del repo
  (`enlace_01`, `enlace_02`, `reloj_03`) **si acotan bien** |

- | ~~**1.38**~~ | 🟢 **CONSTRUIDA Y EN `main`: la construyeron `599d1ce` y `d32feb7` (13/09) y esta fila se quedo
  vieja.** Recontado hoy sobre el fuente: `degradado_avisoLimite()` y `degradado_syncVencida()` **si tienen llamador**
  -`Esclavo/src/bluetooth.cpp:1260-1261`- y `modo_degradado_avisoLimite()` tambien
  -`Maestro/src/bluetooth.cpp:978-979`-, con `AVISO_LIMITE_MS` = 44 h, o sea las ultimas 4 h. ⚠️ **Sin banco y sin
  tarjeta.** ⬇️ *lo que decia:* 🔴 **EL AVISO PREVIO AL LIMITE DE 48 h SE QUEDO SIN SALIDA — consecuencia directa de
  retirar el LCD (`17d3a1f`), y hay que arreglarlo.** `degradado_avisoLimite()` y `degradado_syncVencida()` del
  Esclavo **se quedaron sin llamador**, y `AVISO_LIMITE_MS` del Maestro **no se usa en ninguna linea**: su lector era
  `menu_loop()`. **Hoy el cruce se va a ambar al vencer las 48 h SIN HABER AVISADO ANTES**, y la antiguedad de
  sincronizacion **no sale por ninguna trama**. ⬇️ **Y es la misma familia que otras dos capacidades que el LCD se
  llevo:** los **contadores de enlace del Maestro** (`SFTY-15`) y la **antiguedad de sync del Degradado del Esclavo**.
  Las tres se siguen calculando —**ninguna abre un veto**— pero **dejan de ser observables**. 🟢 **El responsable ya
  contesto el 13/09: *«todo es por la app»*, o sea que las tres se publican por `bluetooth.cpp`.** ⚠️ **Campo nuevo en
  el `$STATUS` NO CABE** —el peor del Maestro son **151 de 154**, tres bytes, y `documentos_03` obliga a las dos
  puntas—: **la via es extender el `$EVENT` periodico** que `D-32` (2) construyo | `D-32` (1) y (2), `SFTY-15` |
  `{Maestro,Esclavo}/src/bluetooth.cpp`, y `app.js` en sus cuatro copias para pintarlo |
  `costura_10_funciones_muertas` ya lo declara como excepcion y **nombra el sustituto**: no hay que descubrirlo, hay
  que construirlo |

- | ~~**1.41**~~ | 🟢 **CONSTRUIDA Y EN `main` EL 14/09** — 🔴 **y al cerrarla aparecio un hueco del instrumento que hay
  que nombrar: `A-1.bis` seguia escrita como ABIERTA en `DECISIONES.md` mientras el firmware ya la implementaba.**
  `decisiones_01_anclas` no lo vio **porque vigila las `D-x` vigentes sin ancla, no las `A-x` que una `D-x` dice
  cerrar**: la fila de `D-33` decia *«Cierra `A-1.bis`»* y nadie comprueba que la otra fila se tache. Es el bucle de
  `CLAUDE.md` §11.1 con la tabla contra si misma — el siguiente que la lea vuelve a preguntar algo cerrado. Cerrada a
  mano hoy; **el vigilante que lo cazaria solo NO existe** (`363e375` firmware, `de8939f` instrumentos, `96a30e5`
  spec) — ⚠️ **sin banco y sin tarjeta**. Compuerta **18 PASS · 1 FALLA · 0 ABORTADO** en dos pasadas iguales, banco
  **1411/1412**; Maestro **64.2 %**, Esclavo **56.4 %**. **Lo que costó de verdad no fue el veto: fueron los TRES
  instrumentos que medían la FORMA de una condición que `D-33` movió de sitio** —`maestro_09` abortó, `barrera_03`
  acusó al firmware de perder el `S_FALLO`, y `camara_03` exigía por escrito lo que ahora sería el defecto—, y **dos
  copias a mano del límite vial que el Esclavo llevaba sin vigilante** (`N-133`: `DESPEJE_{MIN,MAX}_HEREDADO_SEG`, que
  se habrían quedado viejas COMPILANDO). Bloque **G** nuevo en `Validacion_Automatico`, **99 → 115**, con las tres
  inyecciones hechas sobre el `.cpp` real (115 → 110, 113 y 113) y restauración verificada por hash. `N-154` cerrado
  en el campo de segundos del aviso: fuera de cota va `"!"`, no un número recortado. ⬇️ *la crónica de la decisión,
  que es lo que hay que conservar:* 🟢 **`A-1.bis` CERRADA — `D-33` (14/09): LA CAMARA VETA LA BAJADA DE LA PLUMA**, y
  la pluma baja **3 s despues del rojo**. Llevaba abierta desde el 05/09. El motivo que le faltaba, del responsable:
  *«es su SENSOR DE PRESENCIA; el punto de la camara es que la barrera no se lleve una moto o un carro»*. **Veta
  cualquiera de las dos** —es un sensor, no un voto— y **el fallo va SIEMPRE a barrera ARRIBA**: ante error o falsa
  alarma no baja, y la app pide **ajuste de camara**. ⬇️ 🔴 **Y PARO LA OBRA UN REVISOR, ANTES DE CONSTRUIRLA, con algo
  que nadie habia visto:** **el coordinador NO consulta la pluma en ningun estado** —`grep` de `semaforo_plumaArriba`
  sobre `coordinador.cpp`: **cero**—, asi que el Maestro daria `GO_GREEN` con su propia pluma vetada arriba, y **el
  `ACK_RED` del Esclavo describe la LAMPARA, no la pluma**. Resultado: **un carril de un solo sentido con las DOS
  bocas abiertas**. 🟢 **Lo disolvio el responsable cambiando la premisa** —*«esas barreras son casi de adorno; el que
  manda es el semaforo y su estado»*—: el veto es **LOCAL**, no para el ciclo, y la otra punta abre con normalidad. ⚠️
  **Eso DEROGA una frase escrita en `semaforo.cpp`** —*«una pluma arriba con la luz en rojo es PEOR que no tener
  barrera, porque el conductor confia en ella»*—, que era **el sosten del argumento del revisor**: se corrige HACIA la
  decision, con su nombre | **`D-33`**, deroga `SFTY-28` en su «nunca al reves» · cierra `A-1.bis` y `SPEC_1` §12.1 ·
  desbloquea la fase 2 de `D-13` | `{Maestro,Esclavo}/src/{semaforo,botones}.cpp`, `arnes_automatico.cpp`, los packs
  de pluma y camara, `SPEC_1` y `SPEC_5` | 🆕 **Capacidad nueva que entra con la misma decision y NO se construye en
  este lote: el operador podra RETIRAR LA BARRERA DE LA LOGICA desde la app en modo administrador, poste a poste.** 🟢
  **YA TIENE FILA: el 14/09 el responsable la definio como DOS interruptores -fila 1.44- y su publicacion como
  decision aparte -fila 2.13-.** Lo que esta fila pedia medir sigue sin medirse y viaja con ellas |

- | ~~**1.43**~~ | 🟢 **CONSTRUIDA Y EN `main` EL 14/09.** La app ya no deja el aviso en la bitacora: hay un cartel que
  **vive FUERA de la bitacora y FUERA de las pestanas** —un test le mete **32 lineas** de otro asunto y comprueba que
  sigue—, dice lo unico accionable —*mire debajo del brazo; si no hay nada, revise el apunte de la camara*— y **nunca
  dice «averiada»**, con censo y control negativo sobre los literales del modulo. 🔴 **Y aparecio un TERCER formato que
  el encargo no traia:** `VETO_SOSTENIDO_S:!`, la cota de `N-154`. Sin admitirlo, **la retencion MAS LARGA seria justo
  la que cae al camino crudo**. ✅ Y una decision tomada del fuente: el flanco de cada veto **no abre cartel** —*un
  cartel que salta con cada vehiculo se aprende a ignorar, y entonces el sostenido tampoco se lee*—. Suites: jsdom
  281→302, unitarias 52→63 y 69→75, funcional 65→70. ⬇️ *lo que decia:* 🟡 **QUE LA APP DIGA «REVISE EL AJUSTE DE LA
  CAMARA», que hoy no lo dice.** El equipo YA publica el aviso cuando una camara deja la barrera retenida mas alla del
  todo-rojo mas largo, y lo repite mientras dure; **la app lo deja caer en la bitacora general como una linea mas** y
  no dice lo unico accionable. 🔴 **Y el aviso se PIERDE de la pantalla:** la bitacora guarda **30** entradas y en el
  poste 2 se ven **12**, asi que un aviso que llegue mientras el tecnico mira otra cosa desaparece. Decidido el 14/09
  que **basta UNA vez** —no hay umbral que construir, solo la traduccion— | `D-33` · `SPEC_4` §7 hueco 9 |
  `05_Funcional/App_Semaforo/app.js` **y sus cuatro copias** (`CLAUDE.md` §14) | `grep "CAMARA_PLUMA\|VETO_SOSTENIDO"`
  sobre `app.js` -> **cero**; `app.js:584` `if (state.events.length > 30) state.events.pop()` |

- | ~~**2.1**~~ | ~~🔴 **CONFLICTO ABIERTO `J14`/`PB0` — `A-2` contra el codigo, y es de SEGURIDAD.**~~ 🟢 **CERRADO por
  `D-27` (11/09): `J14` LIBRE, sin cablear — el fin de carrera no se instala en este despliegue.** Lo medido sigue
  valiendo y es el motivo de no conectar nada: el firmware lee ese pin como **camara de demanda** (`CAM_DEMANDA_PIN`)
  en las dos puntas —en el **Esclavo**, `loop()` de `main.cpp` llama a `demanda_solicitar()` en cada flanco de subida
  → `CMD_DEMANDA` por radio **en cualquier modo**; en el **Maestro**, `modoInteligente_loop()` lo lee por NIVEL como
  presencia—. ~~Un fin de carrera cableado ahi hoy **mete demandas falsas con cada movimiento de la pluma**. **No se
  resuelve aqui, y hasta que se resuelva no se cablea nada en `J14`**~~ Vacio, `R64` lo deja en 0 V (`J14` medido en
  banco el 03-04/09, pasos 17-18). ⚠️ **Si algun dia se quiere el fin de carrera, vuelve a ser esta decision, con la
  lectura de `PB0` delante** | `D-27` · ~~`A-2` · §2~~ |

- | ~~🆕 **2.1.ter**~~ | ~~🔴 **`05_Funcional/Camaras_Sisga_4x.html` contradice `D-27` en el objetivo** —paso 07, el
  recuadro «Y estos pasos cambian» y la nota de fuentes: *«Objetivo: no filtrar (vehiculo y persona)»*—. **La guia
  esta caducada en ese punto** (regla del recuadro 🔒 de arriba); el 11/09 solo se toco su `J14`, por encargo. Hay que
  corregirla hacia `D-27` antes de volver a mandarla al instalador~~ 🟢 **HECHO el 11/09** *(sin comitear al escribir
  esto)*: **la guia se corrigio HACIA `D-27`** —objetivo **solo Vehiculo** (☑ Vehiculo · ☐ Humano si la casilla
  existe; si no existe, se anota, se avisa y se sube el minimo del `Size Filter`)— en el paso 07, en «Y estos pasos
  cambian», en el recuadro de quien ya configuro el 10/09 y en la nota de fuentes, con una linea nueva en la fe de
  erratas. Por consecuencia, los pasos 09 y 11 prueban con **un vehiculo** (una persona ya no deberia disparar) y «la
  cuenta» es de vehiculos. Zona, umbral y sensibilidad siguen en los de `D-13` (barrido de la pluma, minimo, alta). Lo
  caducado, tachado con «D-27, 11/09». ⚠️ **Medido al corregirla: la guia del 10/09 (`86683e8`) ya decia ☑ Vehiculo ·
  ☐ Persona** —en eso estaba bien; fue la corregida del 11/09 la que lo invirtio—, asi que quien configuro el 10/09
  **no toca el objetivo**, pero **si** el umbral (`1 s`) y la sensibilidad (`50`), que el recuadro no le pedia
  revisar: se anadieron. Cierra solo el choque con `D-27`; **volver a mandarla al instalador sigue siendo decision del
  responsable** | `D-27` · la guia |

- | ~~**2.2**~~ | ✅ **CERRADA el 14/09 por el responsable (`A-16`): `D-22` va la ULTIMA, va SOLA y no se carga sin una
  tarjeta delante.** 🔴 **Y el argumento que la tenia trabada era FALSO, medido hoy:** *«en Degradado no se siembra
  nada»* dejo de ser cierto —`Esclavo/include/reloj.h`, `D-21` (1): *«esta punta tiene DOS sembradores; con radio la
  siembra es la del Maestro, **sin radio al reves**»* (`D-26` (3))—, y la cadencia son **2 min**, no una hora. Con eso
  **`Y1` ya no decide los 29 s del cruce**. ⚠️ **Lo que SIGUE en pie y no lo arregla la siembra: el contador de 48
  h**, que cuenta desde la ultima sincronizacion **por radio** y se mide con el reloj de programa —`SPEC_7` §5.1—. Lo
  aceptado a cambio esta escrito alli. ⬇️ *lo que decia esta fila, conservado porque es el sujeto de una excepcion de
  `documentos_06_no_reabre_lo_cerrado`:* 🔴 **`A-16` — el sitio de `D-22` en la cola.** Este fichero la da a la vez
  como «opcional y la ultima» y como «va sola y va PRIMERO» (§3.4.quater), y su fila dice que `Y1` «pasa a decidir los
  29 s». ⚠️ **Y el argumento MEDIDO que la decide, conservado aqui literal:** en Degradado **no se siembra nada** —no
  hay radio, que es por lo que se entro, ni telefono—, asi que las dos puntas corren libres hasta 48 h sobre el HSI y
  la deriva va de **~29 min a ~1,2 h**: ***«se siembre cada hora o cada mes, eso no cambia»***. Con `Y2` muerto, `Y1`
  **no es una mejora: es lo unico que hace seguro el Degradado**, y su modo de fallo —si no oscila, la tarjeta queda a
  oscuras y sin reiniciarse— sigue siendo el peor del proyecto: por eso lo que se pide no es construirlo ya, es su
  ORDEN. **No se decide aqui.** *(La frase entrecomillada es literal de §3.11 y se queda AQUI, viva, a proposito: es
  el sujeto de la segunda excepcion de `documentos_06_no_reabre_lo_cerrado` —la que no acusa «cada hora» cuando le
  sigue «o cada»—, y `roadmap_hist.md` esta en su `EXCLUIDOS_RAIZ`. Su comentario todavia la cita como «roadmap.md
  §3.10»: hay que re-apuntarlo a esta fila.)* | `A-16` |

- | ~~🆕 **2.9**~~ | ~~🟠 **`G3` del arnes de dos puntas: la punta en verde tarda en soltar frente a un `S_FALLO`**~~ 🟢
  **DECIDIDO Y CONSTRUIDO el 12/09 (`N-163`, `b24578c`) — la compuerta baja de DOS rojos a UNO.** El responsable puso
  la condicion, y paso a ser el criterio de aceptacion: *«lo que no puede ser es que por microcortes por mala senal de
  radio… cada nada el esclavo se pasa a ambar»*. **El umbral de 25 s NO se toca** —lo demuestra un `static_assert`:
  `LATIDO_MS + 5·TIMEOUT_ACK_MS + TIMEOUT_ACK_MS <= SFTY6_SILENCIO_MS`, 20500 ≤ 21500, o sea que soltar el verde antes
  **no recorta el presupuesto de reintentos de `N-71`**—; lo que cambia es **cuando** se suelta, a `SFTY6_SILENCIO_MS
  − TIMEOUT_ACK_MS`. **Criterio medido caso por caso sobre 32 cortes** de 3 a 24,95 s en las dos direcciones: ambares
  del Esclavo **8 → 8**, del Maestro **10 → 10**. Ventana **250 ms → 0**; `dos_puntas` **106/106**. 🔴 **Y la
  implementacion OBVIA rompia el criterio:** salir por `C_ESPERANDO_ACK_RED` **suprime el latido** (`SFTY-13`) y el
  Esclavo se va a ambar **16 veces en vez de 8** — descartada, y guardada como inyeccion. ⚠️ **Tres correcciones de
  premisa:** el desfase **no es un viaje de radio** —200 de los 250 ms son la cortesia de `SFTY-17`—; hay **TRES**
  puertas al verde propio, no una, mas una cuarta en el Degradado que **no se toca a proposito** (`SFTY-21`); y soltar
  el verde sin mas dejaria el cruce en todo-rojo **hasta 15 min** con el ciclo maximo, de ahi la reanudacion. **Flash
  +244 B: 89,8 %, quedan 6.656.** ⚠️ **Sin banco y sin tarjeta** | `SFTY-6`, `N-163` |

- | ~~🆕 **2.10**~~ | 🟢 **CERRADA ENTERA EL 12/09. La cadencia esta CONSTRUIDA (`dca17cd`) y lo que quedaba del plazo
  lo decidio el responsable ese dia (`D-28`).** Tres cosas que esta fila NO sabia y se apuntan aqui porque son la
  leccion, no la cronica: **(1) su derivacion del plazo era una TAUTOLOGIA** —el tiempo en que el HSI acumula la
  deriva de UNA cadencia *es* la cadencia; los 20 s de holgura que veia a 300 s salian del redondeo `ceil(7,5)->8`—,
  asi que **a 120 s habria dado plazo = cadencia, margen CERO**: cambiar solo el numero, que es lo que esta fila
  pedia, habria metido el defecto. El plazo pasa a derivarse del **relevo**. **(2) Su relevo de «265 s» estaba mal:
  son 271 s** —aplica la inflacion del HSI al caso (a) y se le olvida en el (b)—; no cambia el veredicto. **(3) `D-28`
  lo lleva mas alla:** el responsable eligio que el plazo cubra **DOS** siembras perdidas y no una, o sea `3C` inflado
  = 369 s -> plazo **400 s**, y el margen de los dos `DS3231` **baja de 13 a 7 s** sobre un aguante de 29 (medido con
  `_aguante()` de `esp32_13` y `_relativa_s()` de `reloj_04`, no a mano). **EN CONSTRUCCION el 12/09**, fila **1.18**.
  ~~Lo que la motivo, medido~~ *(lo de abajo se conserva porque es como se hallo; sus cifras son de la cadencia
  vieja)* — medido al construirla el 11/09 por la noche. El plazo de caducidad sale de la cadencia: `HORA_CADUCA_MS` =
  deriva de UNA cadencia al HSI peor = **320 s** frente a una cadencia de 300 s. Consecuencias: **(a)** en Degradado,
  **una sola siembra perdida o tardia** (20 s de holgura) manda la punta a ambar, y **no vuelve sola**; tolerar una
  siembra perdida con 5 min no cabe (32 s de separacion contra 29); **(b)** el relevo de `D-26` (3): al callarse la
  radio, la hora del Esclavo puede tener hasta 300 + 25 + 300 = 625 s antes de la primera siembra de su ESP32 — **~52
  % de las caidas de radio** (fases al azar: P(a+b > 295 s), a y b uniformes en [0, 300]; rehecho a mano) el Esclavo
  **no puede entrar en Degradado** durante esos minutos o se rinde si ya estaba. No da verde-verde; da un cruce en
  ambar. **Propuesta con la medida:** bajar la cadencia a **~2 min** (120 s): plazo ~280 s, cabe una siembra perdida
  (246 s), el relevo cabe (2x120+25 = 265 s), y el margen de los dos `DS3231` **sube** de 11 a 13 s. Coste: la hora
  por `J17` y por radio 2,5 veces mas a menudo. **Cambia el numero de una decision del responsable: es SUYA** | `D-26`
  (2), `D-21` (1) | `ESP32_Expansion/include/contrato.h` (`SIEMBRA_INTERVALO_MS`), `{Maestro,Esclavo}/include/reloj.h`
  (el plazo tendria que derivarse tambien del relevo: `2C + SFTY6`), `reloj_04`, `esp32_13` | `HORA_CADUCA_MS` 320000
  contra `SIEMBRA_INTERVALO_MS` 300000UL; bloque F del Degradado a dos puntas |

| ~~**T-4**~~ | ~~🟠 **Confirmar `N-117` sobre el modulo** con el monitor serie: el arranque del ESP32 cronometrado de verdad, `reset -> primer byte`~~ — 🟢 **el SINTOMA se cerro en banco el 04/09** (`roadmap_hist.md` `N-126`: el modulo se anuncia estable, `SEM-179DB0-M`). La causa ya no se puede discriminar con el arreglo dentro | §6.4 · 🔴 **lo que queda en esta superficie es OTRO sintoma**: los reinicios del ESP32 del Sisga (§3.16), que no son el perro de `N-117` |

| ~~**T-5**~~ | ~~🟠 **`0x68` del `DS3231` sobre el modulo real.** El reloj esta cerrado en cobre (`HORA:22:19:58` en la cinta del 05/09), pero la direccion I2C sigue `SIN VERIFICAR`~~ — 🟢 **verificada el 10/09 en el modulo del Maestro `179DB0`**: en la cinta del Sisga el puente, que habla con `DS3231_DIR 0x68`, contesta los `SET_RTC` con la hora releida y `LEER_RTC` la da avanzando (12:17:31 → 12:18:52) | falta el modulo del **Esclavo**; y el comentario de `contrato.h` sigue diciendo `SIN VERIFICAR` (firmware: no se toca desde aqui) |

- | ~~**D-c**~~ | ~~🔴 **Las dos pantallas** (04/09)~~ 🟢 **NO NECESITA FILA: el sujeto ya no existe.** 🔴 **Las dos LCD
  NO EXISTEN** —el responsable, 12/09: *«las dos LCD hoy no existen»*; `D-17.bis` retiro el hardware el 28/08 y `D-30`
  saca el software—, y aquel diseño era **de pantalla fisica**: leia `lcd.cpp` de las dos puntas. **Lo que sobrevive
  de la idea no es la pantalla: es que el diagnostico de CADA punta llegue al tecnico**, y hoy eso es la app — la
  mitad Esclavo es **`D-23`**, decidida el 07/09 con la via `$EVENT`, y la mitad Maestro es la fila **1.26**. Las dos
  tienen fila viva, asi que no hace falta un `D-x` nuevo. ~~Es **la unica decision suya sin una sola linea de codigo
  detras**~~ *(11/09: no es la unica — `D-14`, `D-22` y `D-23` tampoco tienen codigo)* | §6.9 en el historico |

| ~~**N-117**~~ | el perro del ESP32 se comia su propio arranque | ~~**arreglado en el arbol el 04/09; la causa NO esta confirmada sobre el modulo.**~~ 🟢 **el SINTOMA se cerro en banco el 04/09** (`roadmap_hist.md` `N-126`: *«CERRADO con evidencia en hardware»*, anuncio estable como `SEM-179DB0-M`). La causa ya no se puede discriminar con el arreglo dentro, y `ESP32_ARRANQUE_MEDIDO = 0` sigue en `contrato.h` (el responsable midio 2–3 s *energizado → primer dato en la app*, que no es la ventana del perro). **Los reinicios del Sisga son otro sintoma** (§3.16). §6.4 |

## 7. `ESTADO.md` tal como estaba en `925c71e` (109 lineas)

Literal; sus titulos bajan un nivel (`#` delante), como en el apartado 2, y las lineas anchas se parten.

## ESTADO — dónde está parado el trabajo HOY (28/09/2026)

> ## ▶️ PUNTO DE CONTINUACION — 28/09/2026, Y ES POR AQUI POR DONDE SE RETOMA
>
> 🚦 **LLEGARON LAS TRAMAS DEL FUNCIONAL (Marco, Maestro `4D2007`, firmware `1889631` por declaracion:
> el puente contesto `FW:--`).** Lo medido en la cinta y lo hecho:
>
> - 🔴 **La prueba de focos daba VERDE CONTRA VERDE en servicio** (medido con el C++ real: 2 s con el
>   Esclavo en verde). **Ahora se rechaza en servicio** (`EN_SERVICIO_PASE_A_MENU`), solo corre en
>   MENU/HORA/ALCANCE **con el rojo del Esclavo acusado** (`ESPERANDO_ROJO_DEL_ESCLAVO`) y al acabar
>   devuelve la luz del estado. Decision del responsable: *«es lo esperado, asi no le guste al funcional»*.
> - La app ya no rotula `FALLO COM` el ambar PEDIDO, y la Prueba de Alcance pinta el enlace donde se pulsa.
> - **Camaras: `CAM:?` toda la sesion** — ninguna cerro su contacto; el cableado que paso Marco casa con
>   `SPEC_5` §3. Sin medir si es la camara o el equipo: lo separa el puente p10-p9 del `.html` nuevo.
> - **Hora, degradado:** sin evidencia en la cinta. Van en `05_Funcional/Pruebas_Funcional_2026-09-28.html`,
>   que el funcional rellena y devuelve en PDF.
>
> 🧭 **ORGANIZAR EL REPO POR FASES (acordado con el responsable y el arquitecto de plataforma, 28/09).
> Es ordenar, no romper: cada fase con su commit y la compuerta en verde.**
>
> | fase | que | estado |
> |---|---|---|
| 0 | simulador congelado (ningun pack nuevo); QA solo sobre la candidata; paquetes FUERA del repo, en `D:\@Proyect\Entregas_Semaforos\` (lo viejo en `RETIRADOS\`) | ✅ paquetes movidos · la cadencia entra en `CLAUDE.md` §4 con la fase 2 |
| 1 | inventario de instrumentos por requisito de SPEC, y borrador de `particularidades` de las skills | ✅ hecho: 1,76 a 1 medido; opcion B elegida |
| 2 | `CLAUDE.md` a 200 lineas sin renumerar; `ESTADO.md` sin cronica (a `HISTORIA.md`, citado en `ARQUITECTURA.map`) | ✅ hecha: `CLAUDE.md` 200, `ESTADO.md` 109, cronica en `HISTORIA.md` |
| 3 | plugin `orquestador@diego` en el proyecto; skills `entregar`/`verificar` a `.claude/particularidades/` | 🔄 plugin instalado (`6651752`); faltan pre-commit con trinquete y mudar las skills · el `/plugin install` lo hace el responsable |
| 4 | poda de instrumentos segun el inventario; lo archivado a historico y citado en `ARQUITECTURA.map` | 🔄 opcion B en una rama, midiendo mutantes; despues, goteo con acta de campo |
> | 5 | lo duplicado Maestro/Esclavo a `lib/` | **despues** de que la candidata pase banco |
>
> 📦 **Enviado: `Paquete_Banco_2026-09-28_1e56d83_SIN_BANCO.zip`** (sha256 `5edc8055`) con la APK
> `IOT_VIAL_Semaforos_2026-09-28_1e56d83_SIN_BANCO.apk` (`a249e42a`) y el `.html` de pruebas.
>
> **Lo siguiente: el PDF de Marco con las cintas de LAS DOS tarjetas.** Pedir la version antes de leer nada.

> **Este fichero es el estado VIVO:** lo abierto, lo que bloquea y lo que falta medir. La cronica de los puntos de
> continuacion anteriores (16/09, 14/09, 12/09 y la V9.0) esta literal en [`HISTORIA.md`](HISTORIA.md); el porque de
> cada `N-x`, en [`roadmap.md`](roadmap.md). La spec manda (`CLAUDE.md` §15). El HEAD se mide: `git rev-parse --short
  HEAD`.

### Que firmware hay en cada equipo (`CLAUDE.md` §0.2)

| equipo | firmware | como se sabe |
|---|---|---|
| instalacion certificada | V8.4, `e303485` (31/07) | la ultima que paso banco |
| Maestro `SERIE:179DB0` (El Sisga) | V9 `SIN_BANCO`: cargado `7ff7d12` (paquete del 08/09), probado despues `b354fe9` (10/09) | cinta y diario del Maestro en `evidencia/` |
| Esclavo del Sisga | sin medir | su cinta sigue sin traerse |
| Maestro `4D2007` (funcional, Marco) | `1889631` **por declaracion**: el puente contesto `FW:--` | cinta del 28/09 |
| paquete enviado el 28/09 | `1e56d83` `SIN_BANCO` | nadie ha informado aun de que este cargado |

- El `Y2` de 32,768 kHz del Maestro `179DB0` NO oscila (`ON:1 RDY:0 BYP:0 SEL:0 EN:0 CNT:--`): es soldadura (`C-6`).
  Un fallo de hora en ESA tarjeta es esto.
- Los «17 segundos del Esclavo» no existen en el firmware (`grep 17000` = cero): son 21,5 s para soltar el verde
  (`SFTY6_SILENCIO_MS - AVISO_AMBAR_TIMEOUT_MS`) y 25 s para el ambar. Pendiente de decision del responsable.

### Dependencias de esta maquina que no estan en el repositorio

- **`D:\toolchain\mingw64`**: el `gcc` de host fuera de la ruta con `ñ` (alli su `ld` no abre `crt2.o`, `N-44`). Si
  desaparece, los arneses que compilan C++ real caen a `ABORTADO` a la vez. Un `ABORTADO` se lee siempre.
- **No hay `java` en el `PATH`.** `gradlew` necesita `JAVA_HOME` apuntando a un JDK de `D:\@Proyect\Baliza\7 sw apk\`;
  `sdk.dir = C:/android-sdk` (lo dice `android/local.properties`).
- **Los paquetes salen a `D:\@Proyect\Entregas_Semaforos\`**, fuera del repo; los viejos, en su `RETIRADOS\`.

### ABIERTO, por orden de lo que duele

Los tres primeros no los cierra nadie escribiendo codigo.

1. **`BAT:--`**: falta un divisor de tension y una entrada analogica (`grep -rn analogRead` da cero; `N-108`).
2. **`J16` p1 lleva 12 V crudos:** taparlo es obligatorio en cada equipo que se monte (`N-120`).
3. **Matriculacion por ID de Bluetooth:** `RF_Packet` son 4 bytes sin campo de direccion. Decision de protocolo del
   responsable, aplazada a despues del banco.
4. `buildCommand()` sigue siendo copia a mano de `generarComando()`.
5. Retirar `parseStatus()` de verdad exige tocar `simulador_app_bluetooth.py` y `documentos_03`.
6. `FW-N53`: decidir si se redefinen los gestos (hoy Auto `A·A·A`, Ambar `B·B·B`). Es decision de spec.

### BLOQUEANTES

| # | Que esta bloqueado | Que lo desbloquea | De quien |
|---|---|---|---|
| BLQ-3 | La Maestro de la sesion 1 del banco (`N-116`) se calienta y muere a los ~30 s; causa que sostiene el cobre: latch-up | Medir el consumo del riel de 3,3 V en frio con fuente limitada. No reenergizar «a ver si pasa» | Responsable |
| BLQ-6 | Luces, Esclavo, `N-151` y `N-152` no han visto cobre; lo posterior a `7ff7d12` no ha tocado una tarjeta con cinta | Una carga y una pasada de ambar, rojo total y `DAR PASO` | Banco |
| BLQ-5 | Ninguna de las 5 entradas de campo esta protegida (`N-120`) | Revision de diseno (2K2 en serie). Mientras, tapar `J16` p1 | Responsable |
| BLQ-4 | Reinicios del ESP32 del Sisga (`OTRO_PERRO`, `SUBIDA_DE_TENSION`) | USB-TTL en `TX0` a 115200, osciloscopio en 3V3 y `EN`, fuente de 5 V buena | Tecnico |
| BLQ-2 | El cristal `Y2` no oscila en la tarjeta medida | Diagnosticar el `Y2` de la segunda tarjeta | Responsable |

### VERIFICACION EN ESCRITORIO — lo que dice la ultima acta

Cifras **copiadas del acta
[`evidencia/2026-09-28_compuerta.txt`](evidencia/2026-09-28_compuerta.txt)**, no escritas a mano —
lo comprueban `documentos_01`, `documentos_04` y `documentos_05` en cada corrida.

| | |
|---|---|
| Flash | Maestro **64.7 %** (**42380** de 65536 B → **23.156 B libres**) · Esclavo **55.4 %** (36336 B) · Repetidor **20.6 %** · ESP32 **35.7 %** |
| Banco por packs | 🔴 **1318/1319 comprobaciones** en **70 packs** — 69 PASS, **1 FALLA**. El rojo es CORRECTO: `decisiones_01_anclas` acusa a **`D-22`**, la única decisión vigente sin construir, y **necesita una tarjeta delante** (`CLAUDE.md` §1: no se decora) |
| Arneses que compilan C++ real | ~~287/287 pantalla~~ *(retirado con el LCD, `D-32` (1))* · **75/75** automático · 22/22 ciclo · **122/122 dos puntas** · **71/71 Degradado a dos puntas** |
| Puente ESP32 | **101/101** |
| App | **310/310** jsdom · **70/70** funcional · **63/63** unitarios · **75/75** TDD |

> 🔴 **Qué HEAD y con qué árbol se midió lo dice el acta en su cabecera, y no se copia aquí**: aquí
> ponía `f27f1a0` cuando el acta citada decía otro. Si dice `CON CAMBIOS SIN COMMITEAR`, sus cifras
> **no corresponden exactamente** a ningún commit, y para que sean reproducibles hay que volver a
> correr la compuerta con el árbol limpio.

### Donde esta cada artefacto

- App: [`05_Funcional/App_Semaforo/`](05_Funcional/App_Semaforo/). APK y paquetes: fuera del repo (arriba).
- Guia de cableado y formulario de vuelta: `05_Funcional/Guia_Cableado_y_Pruebas_Banco.html`, devuelta en PDF.
- Pruebas del funcional del 28/09: `05_Funcional/Pruebas_Funcional_2026-09-28.html`, devuelta en PDF.
- Esquematico KiCad: [`01_Firmware/Controladora_Semaforos/`](01_Firmware/Controladora_Semaforos/).
- Informe de banco 3-4/09: `evidencia/Informe_Pruebas_Banco_Semaforos_V9.0.pdf` (24 de 29 pasos, sobre `617bd00`).

## 8. Lo que salio de `README.md` (`925c71e`, lineas 13-17 y 30-39)

Literal; la linea ancha, partida.

> ▶️ **16/09/2026 — SE ESPERAN LAS TRAMAS DEL FUNCIONAL.** Se le mando
> `Paquete_Banco_2026-09-16_1889631_SIN_BANCO.zip`. **Por donde se retoma, y que hay que
> preguntarle a esas tramas ANTES de leerlas, esta en el punto de continuacion de
> [`ESTADO.md`](ESTADO.md)** — empieza pidiendo la version del firmware, que desde hoy el
> equipo sabe contestar.

| | firmware | instrumento | ratio |
|---|---|---|---|
| 28/08 | 8.895 | 8.898 | 1,00 : 1 |
| 02/09 | **14.976** | **34.532** | **2,31 : 1** |
| 05-06/09 | — | — | **2,74 : 1** *(acumulado)* |

> **La última medida que hay es el `2,74 : 1` del 05-06/09** (`roadmap_hist.md`, sesión del
> arquitecto —el roadmap se partió el 07/09 y esa sesión está en el histórico—). Las dos primeras filas son las únicas
> con sus cifras absolutas escritas; el
> `2,74` se publicó como ratio y su recuento no quedó anotado, así que **no se le inventa aquí
> un par de números para rellenar la fila**.

## 9. `ARQUITECTURA.map` tal como estaba en `925c71e` (1.243 lineas)

Literal; sus titulos bajan dos niveles y las 7 lineas anchas se parten. El mapa vigente es `ARQUITECTURA.map`.

### ARQUITECTURA.map - DONDE VIVE CADA COSA Y QUIEN DEPENDE DE QUIEN

    **El 12/09/2026 se le saco §7 entero** -el hardware: conectores, pines y cadenas de
    potencia- a [`ARQUITECTURA_conectores.map`](ARQUITECTURA_conectores.map), y **la cronica
    de §1.2, §1.3 y §9** a [`ARQUITECTURA_hist.map`](ARQUITECTURA_hist.map). Los dos son
    INTEGROS Y LITERALES -nada resumido, nada reescrito- y el segundo publica al final la
    cuenta contra las 1.517 lineas de partida. **Los numeros de apartado NO se tocaron**
    (CLAUDE.md §5): cada uno que se mudo se quedo con su cabecera y una frase que dice el
    veredicto de hoy y donde vive el desarrollo.

    ⚠️ **TOPE: 1.000 LINEAS. HOY NO SE CUMPLE, Y SE DICE AQUI EN VEZ DE DISIMULARLO** -
    `wc -l ARQUITECTURA.map` da la cuenta de verdad-. Sacar §7 y esa cronica no bastaba para
    bajar del tope: lo que sigue pesando son **las dos tablas de instrumentos que el encargo
    del 12/09 pidio conservar intactas** -§3.3 y §3.7, lo que se consulta a diario- mas el
    resto de §2 y §3, que es mecanica de HOY -que pack abre que fichero, que rompe moverlo-,
    no cronica. **Perder una fila de instrumento cuesta mas que pasarse del tope.** La cuenta
    completa y por que no se recorto mas, en `ARQUITECTURA_hist.map`, ultima seccion.


    ESTE FICHERO ES UNA TABLA, NO UNA CRONICA. Cada fila es una afirmacion sobre el
    arbol y CADUCA SOLA (CLAUDE.md 14): se RE-MIDE, no se lee. Como se re-mide va
    escrito al lado de cada seccion, en `[MEDIDO ... - <comando>]`. Donde el comando
    no cabe, esta en 3.7.

    El PORQUE de cada hallazgo -como se descubrio, que se refuto, que paso tal dia-
    NO vive aqui: vive en `roadmap.md` bajo su `N-x`. Aqui queda el puntero.

    Ultima re-medida  : 12/09/2026, sobre `7c85543` (rama main-nuevo, arbol quieto)
    Acta vigente      : `evidencia/2026-09-12_compuerta.txt` (HEAD `7ef5340`)
                        19 PASS | 1 FALLA | 0 ABORTADO - 82 packs - 1388/1396
    El unico rojo     : `decisiones_01_anclas` 57/65 (fila 13). ES EL ROJO ESPERADO
                        de CLAUDE.md 1: D-14, D-22, D-23, D-25, D-27 y D-30 estan
                        VIGENTES sin ancla en el fuente, y D-30/D-31 sin nombrar en
                        ningun manual. No se apaga escribiendo el ancla -eso seria
                        decorarlo-: se apaga construyendo. Su cuenta es la medida de
                        cuanto se esta decidiendo por encima de lo que se construye.

    🔴 **NO ES UN "20/20", Y CUANDO LO SEA TAMPOCO SERA UN ENTREGABLE.** Un verde
    dice que los modelos y arneses de PC no encuentran nada; NO dice que el firmware
    funcione en la tarjeta. Los cinco defectos que pararon el banco del 3-4/09 pasaron
    esas veinte filas sin despeinarlas.

    ⚠️ **ESTE FICHERO LO PARSEA UN PACK.** `documentos_06_no_reabre_lo_cerrado` lo censa
    junto a los `.md` de la raiz (ver 0): una frase derogada por `D-20/25/26/27` que se
    deje aqui SIN `~~tachar~~` **hace caer un pack**. Se tacha con `~~`, no con otra marca.

#### 0. Como se lee, y que NO es este fichero

| etiqueta | significa |
|---|---|
| `[MEDIDO dd/mm]` | se corrio el comando ese dia y esto es lo que devolvio |
| `[ESCRITO]` | lo afirma un documento, un comentario o un acta. No se re-comprobo |
| `[SIN VERIFICAR]` | nadie lo ha comprobado nunca |

**ESTE MAPA NO ES UN INSTRUMENTO.** No mide nada y nada falla por lo que aqui ponga.
`[MEDIDO]` aqui significa *se midio sobre un FICHERO*: el `.cpp`, el `.h`, el
`.kicad_pcb`. Un fichero dice lo que alguien dibujo o escribio; una placa dice lo que se
fabrico. Lo que MIDE es `01_Firmware/compuerta.py`.

**PERO UN PACK SI LO PARSEA** `[MEDIDO 12/09]`:

```
$ grep -rn "ARQUITECTURA" --include=*.py .        (sin node_modules ni .claude/worktrees)
  banco/packs/documentos_06_no_reabre_lo_cerrado.py   LEE este fichero
  banco/packs/esp32_05_no_origina.py                  prosa de un comentario
  05_Funcional/generar_graficas_arquitectura.py       texto de una grafica
  generar_entrega_v9_0.py                             la constante es 17_Arquitectura...md
```

`documentos_06_no_reabre_lo_cerrado` censa **todo `*.md` de la raiz mas `ARQUITECTURA.map`**
-`n.endswith(".md") or n == "ARQUITECTURA.map"`- buscando frases derogadas por
`D-20/25/26/27` SIN TACHAR. Una frase vieja dejada aqui sin `~~tachar~~` **hace caer un
pack**, y la marca tiene que ser exactamente `~~...~~`. La entrada de `flash_01_lastre` NO
es una lectura de este fichero: parsea `firmware.map`, el del enlazador -misma extension,
ninguna relacion.

**QUIEN LO NOMBRA** `[MEDIDO 12/09 - grep -rn "ARQUITECTURA.map" --include=*.md .]` - se da
el comando, no el numero de linea: `INDICE_CRUZADO.md` 11 · `roadmap_hist.md` 6 ·
`roadmap.md` 4 · `CLAUDE.md` 3 · `README.md` 2 · uno cada uno en `DECISIONES.md`,
`02_LCD/MANUAL_PANTALLA_LCD.md`, `05_Funcional/README.md` y `99_Legacy/LEEME.md`.
⚠️ `OPTIMIZACIONES.md` ya NO lo nombra, y la pasada del 07/09 decia que si.

> **AVISO PARA QUIEN MANTENGA `INDICE_CRUZADO.md`:** cita este fichero **por numero de
> linea**. Toda reescritura las invalida y no se pueden arreglar desde aqui. Es CLAUDE.md
> 7.3 cobrandose una cita numerica: se cita el SIMBOLO o el titulo de seccion, nunca la linea.

**SU HERMANO ES `INDICE_CRUZADO.md`, y contestan preguntas distintas:**

| | contesta |
|---|---|
| `ARQUITECTURA.map` | **COMO** esta hecho el equipo y **QUE ROMPO** si muevo un fichero |
| `INDICE_CRUZADO.md` | **DONDE** vive cada hecho y **QUIEN SE QUEDA COLGANDO** si se reescribe un documento |

**AQUI NO SE COPIA NINGUNA CIFRA DEL ACTA** salvo la cabecera de arriba, fechada. El
recuento de packs, las comprobaciones y los porcentajes de flash se mueven cada hora y hay
packs que los vigilan; una copia a mano seria una segunda version que sincronizar.

---

#### 1. EL ARBOL REAL

##### 1.1 Primer nivel `[MEDIDO 12/09 - ls -la de la raiz]`

```
Controladora_Semaforos 2/
|
+-- 01_Firmware/            firmware de los 4 roles + TODOS los instrumentos
+-- 02_LCD/                 2 ficheros. Manual de la pantalla -LEGACY, ver D-17.bis-
+-- 03_Hardware_Tarjeta/    MAPEO_TARJETA_KICAD.md + indice_netlist.py
+-- 04_Manuales/            manuales de terceros (camara, radios) y los 2 propios
|                           MANUAL_USUARIO.md / MANUAL_HARDWARE.md - mudados aqui el 05/09
+-- 05_Funcional/           los 19 documentos numerados (.md + .docx), App_Semaforo/ y dos guias
|                           HTML: Guia_Cableado_y_Pruebas_Banco.html y Camaras_Sisga_4x.html
+-- 05_Imagenes/            3 .jpg de la app
+-- 99_Legacy/              zips de entregas viejas y firmware retirado. NO se toca
+-- evidencia/              actas de la compuerta + fotos de banco (WhatsApp *.jpeg)
+-- scratch/                6 .py sueltos de exploracion. EN .gitignore
|
+-- ARQUITECTURA.map        este fichero
+-- CLAUDE.md               reglas permanentes del repositorio
+-- DECISIONES.md           tabla D-x / A-x. GANA en toda decision vigente
+-- ESTADO.md               donde esta parado el trabajo HOY
+-- HISTORIA.md             cronica literal que salio de CLAUDE.md y ESTADO.md el 28/09. Sin tope; no manda
+-- roadmap.md              el porque, con los N-x debajo
+-- README.md               portada. Sus cifras se copian del acta
+-- CERTIFICACION_SW.md     lo que se puede afirmar del software
+-- OPTIMIZACIONES.md       reglas SFTY-x y trazabilidad regla -> codigo -> prueba
+-- INDICE_CRUZADO.md       donde vive cada hecho; huecos abiertos
+-- LEEME_PRIMERO.md        portada del paquete que se entrega
+-- ORDEN_EJECUCION.md      2 KB, del 28/07. Sin tocar desde entonces
+-- generar_entrega_v9_0.py el empaquetador del .zip
+-- *.apk (3) y *.zip (1)   artefactos de entrega. EN .gitignore
+-- v8_definitiva_changes.patch   480 KB del 31/07. EN .gitignore
```

##### 1.2 Ruido: existe y se ignora - NO se inventaria `[MEDIDO 12/09 - .gitignore + test -e]`

**MOVIDO integro a [`ARQUITECTURA_hist.map`](ARQUITECTURA_hist.map) §1.2 el 12/09/2026.**
Ahi sigue el censo completo -`scratch/`, `node_modules/`, los `.pio/`, `build/`,
`__pycache__/` y los binarios en `.gitignore`- y el aviso de que el arbol esta en vuelo
mientras otros agentes escriben.

##### 1.3 `01_Firmware/` - segundo nivel `[MEDIDO 12/09]`

**MOVIDO integro a [`ARQUITECTURA_hist.map`](ARQUITECTURA_hist.map) §1.3 el 12/09/2026.**
El arbol completo de `01_Firmware/` sigue alli. **La trampa que no se puede perder, aqui en
vivo:** `01_Firmware/Controladora_Semaforos/` esta ANIDADO DOS VECES -`ls` sobre el primero
devuelve otro directorio del mismo nombre, no los ficheros de KiCad-. Ruta buena, entera:
`01_Firmware/Controladora_Semaforos/Controladora_Semaforos/Controladora_Semaforos.kicad_pcb`

---

#### 2. EL FIRMWARE

##### 2.1 Los cuatro roles

`[MEDIDO 12/09 - ls <rol>/src/*.cpp | wc -l ; cat <rol>/{src/*.cpp,include/*.h} | wc -l]`
Flash: de la ultima acta (`ls -t evidencia/*_compuerta.txt | head -1`), NUNCA a mano.

| rol | `.cpp` | lineas `.cpp`+`.h` | flash (acta 12/09, HEAD `7ef5340`) |
|---|---|---|---|
| `Maestro` | 21 | 10.630 | 🔴 **89,9 %** - 58.888 de 65.536 B. **Solo 6.648 B libres** |
| `Esclavo` | 14 | 7.940 | **70,7 %** - 46.356 de 65.536 B |
| `Repetidor` | 1 (`main.cpp`) | 214 | 20,6 % - 270.497 de 1.310.720 B |
| `ESP32_Expansion` | 9 | 2.947 | 35,7 % - 1.123.521 de 3.145.728 B |
| **total firmware** | **45** | **21.731** | |

> 🔴 **EL PRESUPUESTO DE FLASH DEL MAESTRO ES EL NUMERO QUE DECIDE, y se estrecha en cada
> pasada:** 87,6 % el 07/09 -> 89,4 % -> **89,9 % el 12/09**. A este nivel una funcion nueva
> de tamano medio NO entra sin medir antes de que esta hecho el porcentaje (CLAUDE.md 10):
> por FICHERO OBJETO leyendo `firmware.map`, no por nombre de simbolo.

> ⚠️ **El acta puede publicar el binario ANTERIOR** si se escribe justo despues de tocar
> codigo (PlatformIO sirve un incremental viejo). La cifra de flash se confirma con una
> segunda pasada; si no coinciden, manda la segunda.

##### 2.2 Maestro - un `.cpp` por concepto `[MEDIDO 12/09 - grep -c '' Maestro/src/*.cpp]`

| fichero | lineas | para que sirve |
|---|---|---|
| `main.cpp` | 339 | `setup()`/`loop()`, watchdog, despacho por modo |
| `semaforo.cpp` | 380 | **LA BARRERA DE SALIDAS.** Unico que escribe pines de luz |
| `coordinador.cpp` | 1.555 | protocolo de cambio de fase con la otra punta, por radio |
| `bluetooth.cpp` | 1.240 | despachador de la app + emision de `$STATUS`/`$ALARM`/`$EVENT` |
| `botones.cpp` | 705 | J16 entero: mando A/B, camaras C/D, **el vigilante de camaras** |
| `modo_automatico.cpp` | 326 | ciclo por tiempos configurados |
| `modo_inteligente.cpp` | 311 | automatico + camaras: PIDE y SOSTIENE fase |
| `modo_degradado.cpp` | 808 | ciclo por reloj, sin radio. UNICO modo sin coordinador |
| `modo_manual.cpp` | 119 | `DAR PASO` disparado por orden |
| `modo_ambar.cpp` | 89 | ambar intermitente pedido a proposito |
| `modo_alcance.cpp` | 71 | prueba de alcance de radio |
| `modo_hora.cpp` | 274 | ajuste del reloj desde el menu |
| `modos.cpp` | 13 | `modoActual_get/set`. El estado del sistema, fuera de la pantalla |
| `menu.cpp` | 163 | navegacion de la LCD |
| `lcd.cpp` | 599 | dibujo. **Los 4 pines en `U8X8_PIN_NONE`** |
| `mando.cpp` | 257 | reconocedor de secuencias A/B del mando de reles |
| `demanda.cpp` | 39 | cola de peticion de paso |
| `reloj.cpp` | 439 | hora, franjas |
| `respaldo.cpp` | 316 | persistencia en flash. **Byte-identico al del Esclavo** |
| `protocolo.cpp` | 169 | CRC y trama de radio. **Byte-identico al del Esclavo** |
| `identidad.cpp` | 84 | numero de serie desde el UID. **Byte-identico al del Esclavo** |

**Ficheros byte-identicos entre puntas** `[MEDIDO 12/09]`. **Los hashes NO se copian
aqui**: este fichero no puede recalcularlos y naceran caducados (CLAUDE.md 14). Se da el
comando -`md5sum Maestro/<f> Esclavo/<f>`- y la lista de los SIETE que hoy coinciden:

```
src/respaldo.cpp   src/protocolo.cpp   src/identidad.cpp
include/respaldo.h include/protocolo.h include/identidad.h  include/ciclo_degradado.h
```

`pines.h` **NO** es byte-identico, pero **el bloque de `#define` SI**:
`diff <(grep '^#define' M/pines.h) <(grep '^#define' E/pines.h)` sale **vacio**
`[MEDIDO 12/09]`. Lo que diverge es comentario.

##### 2.3 Esclavo - lo que NO tiene

`[MEDIDO 07/09 - ls de Esclavo/src frente a Maestro/src]`

**No existen en el Esclavo:** `modo_automatico.cpp`, `modo_inteligente.cpp`,
`modo_manual.cpp`, `modo_ambar.cpp`, `modo_alcance.cpp`, `modo_hora.cpp`,
`modos.cpp`, `coordinador.cpp`.

**Solo existen en el Esclavo:** `config_ciclo.cpp` (160 lineas) - el par de tiempos
que llega por radio.

El Esclavo **no tiene `enum ModoSistema`**: su unico modo propio es el Degradado, y
se entra por `degradado_entrar()`, no por `modoActual_set()`.

> 🔴 **UN CONCEPTO SE LLAMA DISTINTO EN CADA PUNTA, y ese es el error que este
> repositorio ya publico con la palabra "medido" encima.**
> `grep -c 'modoActual_set(MODO_DEGRADADO)' Esclavo/src/*.cpp` da **0** y es
> cierto; y el Esclavo entra al Degradado igual. Se busca por los DOS nombres.

##### 2.4 La barrera de salidas - SFTY-2 y SFTY-28

`[MEDIDO 07/09 - grep de digitalWrite sobre nombres de luz en Maestro/src y Esclavo/src]`

```
$ grep -rn 'digitalWrite' Maestro/src Esclavo/src | grep -E 'ROJO|AMARILLO|VERDE|MOTOR_TALANQUERA'
  Maestro/src/semaforo.cpp  6 escrituras de luz + 2 de talanquera
  Esclavo/src/semaforo.cpp  6 escrituras de luz + 2 de talanquera
  Maestro/src/main.cpp:30   UN COMENTARIO, no codigo
```

**Cero escrituras fuera de `semaforo.cpp` en las dos puntas.** Las seis viven dentro
de `escribirPines(bool rojo, bool amarillo, bool verde)`, que es `static`.

```
                       modo_automatico / modo_manual / modo_degradado
                       modo_ambar / modo_inteligente / coordinador
                                        |
                                        v
                              semaforo_actualizar()
                                        |
                       +----------------+----------------+
                       |                                 |
                 escribirPines()                  destellos del mando
                 (unico, static)          <---- INTERCEPTAN, no rodean
                       |
        +--------+-----+-----+--------+          + MOTOR_TALANQUERA (PB2)
        v        v           v        v            dentro de escribirPines()
      ROJO1  AMARILLO1    VERDE1   ROJO2 ...        reposo = CERRAR (LOW)
```

> ⚠️ **La regla enumera OCHO pines y el firmware mueve SEIS.** `ROJO_PEATON` (PA6),
> `VERDE_PEATON` (PA7) y `BUZZER` (PB1) estan declarados en `pines.h` y **muertos en
> las dos puntas**: sin `pinMode`, sin `digitalRead`, sin `digitalWrite`. La regla es
> **vacuamente cierta** para tres de sus ocho sujetos (N-96).

##### 2.5 Los ocho modos del Maestro y quien abre cada puerta

`[MEDIDO 07/09 - grep -rn 'modoActual_set(' Maestro/src]`

| modo | quien lo ARMA | ¿alcanzable hoy? |
|---|---|---|
| `MENU` | `bluetooth.cpp` (`SET_MODO:MENU`), `main.cpp`, y la salida de los 7 modos | SI, por app |
| `MODO_MANUAL` | `bluetooth.cpp`, `main.cpp`, `menu.cpp` | SI, por app |
| `MODO_AUTOMATICO` | `bluetooth.cpp`, `mando.cpp`, `menu.cpp` | SI, por app |
| `MODO_INTELIGENTE` | `bluetooth.cpp`, `menu.cpp` | SI, por app |
| `MODO_ALCANCE` | `bluetooth.cpp`, `menu.cpp` | SI por app; **sin salida visible: su unica salida era `lcd_dibujarAlcance()`** |
| `MODO_HORA` | **SOLO `menu.cpp`** | 🔴 **NO.** Ver abajo |
| `MODO_DEGRADADO` | `bluetooth.cpp`, `main.cpp`, `mando.cpp`, `menu.cpp` | SI, por app |
| `MODO_AMBAR` | `bluetooth.cpp`, `main.cpp`, `mando.cpp` | SI, por app |

> 🔴 **`MODO_HORA` NO TIENE PUERTA PRACTICABLE, y el motivo es de dos saltos.**
> Su unico armador es `menu.cpp` `case 1: modoActual_set(MODO_HORA)`, y para llegar
> ahi hacen falta **dos `botonAceptar()`** -uno para bajar a `NIVEL_CONFIG` y otro
> para entrar-. Medido `[07/09]`:
> ```
> $ grep -n '^bool boton' Maestro/src/botones.cpp
> 617:bool botonArriba()  { return consumir(0); }     <- VIVO
> 618:bool botonAbajo()   { return consumir(1); }     <- VIVO
> 659:bool botonAceptar() { return false; }           <- CONSTANTE
> 660:bool botonCancelar(){ return false; }           <- CONSTANTE
> ```
> En el Esclavo lo mismo (`:603 :604 :645 :646`). **Se retiraron DOS de los CUATRO
> botones, no los cuatro**: `botonArriba()`/`botonAbajo()` siguen leyendo `BOTON1`/
> `BOTON2`, que son **los pines del mando, J16 p5 y p8**. Cualquier cosa que se
> cablee ahi mueve el cursor del menu ademas de entrar por el reconocedor de
> secuencias. Que no se pueda SELECCIONAR no es que no pase nada (A-2).

##### 2.6 El vigilante de camaras - J16, en las DOS puntas

`[MEDIDO 07/09 - grep de simbolos sobre botones.cpp de las dos puntas]`

| simbolo | Maestro | Esclavo | que hace |
|---|---|---|---|
| `camara_leerPin()` | SI | SI | lector, codigo identico |
| `CAM_PEGADA_MS` | `1200000UL` | `1200000UL` | 20 min de contacto fijo -> `CAM_PEGADA` |
| `CAM_CIEGA_MS` | ~~`21600000UL`~~ **`86400000UL`** | ~~`21600000UL`~~ **`86400000UL`** | ~~6 h~~ **24 h** de paso abierto sin un flanco -> `CAM_CIEGA` `[RE-MEDIDO 11/09: grep -n "CAM_CIEGA_MS =" */src/botones.cpp; subio el 08/09, D-24]` |
| `camaras_sembrar()` | SI | SI | N-26: un contacto ya cerrado al arrancar es estado, no deteccion |
| `camaras_actualizar()` | SI | SI | una lectura por vuelta; no relee |
| `camara_estado()` | SI | SI | el PEOR de las dos camaras -> campo `CAM:` del `$STATUS` |
| `camara_vetosPluma()` | SI | SI | **contador**, no actuador. Ver abajo |
| `camara_presenciaJ16()` | **SI** | **NO** | SOSTENER la fase por NIVEL |

**PEDIR y SOSTENER no son lo mismo:**

```
  flanco en CAM_C/CAM_D --> camaras_actualizar() --> demanda_solicitar()   [PEDIR]
                                                     ventana de 3 s, no se prolonga

  nivel de CAM_C/CAM_D  --> camara_presenciaJ16() --> tercer termino del OR de
                                                     modo_inteligente.cpp     [SOSTENER]
```

`camara_presenciaJ16()` **solo existe en el Maestro** y no es una asimetria a
corregir: el Esclavo no tiene `modo_inteligente.cpp`, no elige tiempos, los recibe
por radio. No hay fase que sostener.

> 🟢 **EL VETO DE PLUMA NO ESTA CONSTRUIDO: SOLO SE CUENTA.** `camara_vetosPluma()`
> se incrementa cuando la pluma ACABA de bajar habiendo presencia, y emite
> `$EVENT CAMARA_PLUMA / VETO_HABRIA_ACTUADO_N:<n>`. **No veta, no baja la pluma, no
> toca una luz.** El comentario del propio fuente lo dice: vetar exigiria entrar en
> `escribirPines()`, y eso es SFTY-28 y necesita derogacion escrita (A-1.bis). El
> campo `ACCION:NINGUNA` del `$ALARM` no es relleno: es no mentir en el acuse.

> ⚠️ **UN PIN QUE NUNCA DIO UN FLANCO NO SE VIGILA.** ~~Hay **una camara por poste**,
> asi que uno de los dos pines de J16 esta VACIO en todo equipo montado.~~ Sin esa
> guarda, ~~a las 6 h~~ cumplido `CAM_CIEGA_MS` emitia `CAM_CIEGA` de una camara que no existe -y `CIEGA` pesa
> mas que `OK`, o sea que tapaba en el campo `CAM:` a la camara que si esta.
> **Lo que cuesta:** una camara muerta DESDE EL DIA DE LA INSTALACION no se distingue
> de una bornera vacia. Lo compensa el instalador provocando la primera deteccion
> (Manual 9), no el firmware.
>
> 🔴 **11/09, `D-25`: LA EXCEPCION PIERDE SU MOTIVO.** Ya no hay pin vacio: son **dos
> camaras por poste**, `p10` y `p12` en las dos tarjetas. El codigo no ha cambiado
> (`vigilante_tick()` salta el pin con `!camHuboFlanco[i]` y `camara_estado()` lo salta
> al publicar `CAM:`, medido en `b79d904` = `a6980e4`), asi que **una segunda camara
> muerta desde la instalacion NO la avisa nadie** y **la app pinta `CAM: OK — las dos
> ven` con la primera deteccion de CUALQUIERA**. Pendiente de rehacer EN FIRMWARE
> (`CLAUDE.md` §6: una excepcion es una afirmacion sobre el codigo); hasta entonces lo
> compensa el multimetro en el borne de cada camara (paso 11 de
> `05_Funcional/Camaras_Sisga_4x.html`). Y tras cada reinicio la vigilancia de silencio
> queda desarmada hasta la primera deteccion, como dice su propio comentario.

##### 2.7 El despachador de Bluetooth - HAY TRES, no dos

`[MEDIDO 07/09 - grep de strcmp(accion,...) / strncmp(accion,...:) / strcmp(cmd,"CMD:...")]`

| aparato | comandos con PIN | prefijos | sin PIN |
|---|---|---|---|
| **Maestro** `bluetooth.cpp` | `DEMANDA` `FORZAR_ROJO` `MANUAL:CAMBIAR_TURNO` `REINICIAR_RELOJ` `TEST_LEDS` `SET_MODO:{ALCANCE,AMBAR,AUTO,DEGRADADO,INTELIGENTE,MANUAL,MENU}` | ~~`SET_RTC:`~~ `SET_TIEMPOS:` | `CMD:FORZAR_ROJO` · `CMD:HORA_ESP32:` *(D-26)* |
| **Esclavo** `bluetooth.cpp` | `AMBAR_EMERGENCIA` `CANCELAR_AMBAR` `FORZAR_ROJO` `SOLICITAR_PASO` `TEST_LEDS` `SET_MODO:DEGRADADO` | ~~`SET_RTC:`~~ - | `CMD:AMBAR_EMERGENCIA` `CMD:FORZAR_ROJO` · `CMD:HORA_ESP32:` *(D-26)* |
| **ESP32** `despachador.cpp` | `CMD:LEER_RTC` *(strcmp)* | `SET_RTC:` *(strstr, dentro de la linea: el puente no conoce el PIN)* · `HORA_ESP32` *(strstr: del telefono se DESCARTA)* | - |

> 🔵 **`D-26`: LA FILA DE `SET_RTC` SE MUDO DE DESPACHADOR** `[MEDIDO 11/09 - grep de
> `strncmp(cmd,"CMD:` / `strncmp(accion,"...:` en los dos `bluetooth.cpp`]`. Ninguna punta tiene ya
> rama `SET_RTC:`: lo atiende SOLO el puente y no cruza `J17`. Las dos tienen `CMD:HORA_ESP32:`
> **antes** de la guarda de PIN. Una linea del telefono con `HORA_ESP32` **la tira el puente**
> (`suplantaLaSiembra()`); sin eso cualquiera con Bluetooth dictaria la hora sin PIN. Lo miden
> `esp32_12` y `esp32_13`. El porque: `roadmap.md`, `D-26`.

> 🔴 **EL TERCER DESPACHADOR ES FACIL DE OLVIDAR.** `app_01_comandos` lo lee desde
> el 05/09 (A-9). Antes, que `SET_RTC` saliera en verde era **casualidad**: el
> Maestro conservaba una rama muda que el censo encontraba. `LEER_RTC` ya no tiene
> casualidad: **solo el puente lo puede contestar** -las dos puntas devuelven
> `$ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO`-.

**La trama `$STATUS`, medida sobre el `snprintf` de cada punta** `[MEDIDO 07/09]`:

```
Maestro  char payload[155];
  $STATUS,NODE:MAESTRO,SERIE:%s,MODO:%s,ESTADO:%s,T:%s,RF:%s,RTT:%s,BAT:--,
          HORA:%s,ESC:%s,PLUMA:%s,CAM:%s

Esclavo  char payload[155];
  $STATUS,NODE:ESCLAVO,SERIE:%s,MODO:%s,ESTADO:%s,T:--,RF:--,RTT:--,BAT:--,
          HORA:%s,PLUMA:%s,CAM:%s
```

`ESC:` solo lo emite el Maestro: el Esclavo no tiene a quien preguntar por el otro
poste. `155` **es el techo, no holgura**: `tramaCompleta[160]` admite payload + `*XX`
+ CR + LF, y `esp32_07_presupuesto_bytes` exige `bufTrama >= bufStatus + 5`, que con
155 queda en `160 >= 160`, **sin holgura que gastar**.

##### 2.8 El Degradado del Esclavo - la puerta y sus seis rechazos

`[MEDIDO 07/09 - grep -rn degradado_entrar Esclavo/]`

```
  Esclavo/src/modo_degradado.cpp:244   RechazoDegradado degradado_entrar()   <- definicion
  Esclavo/src/bluetooth.cpp:725        SET_MODO:DEGRADADO                    <- D-18, por APP
  Esclavo/src/mando.cpp:148            secuencia A.B.A.B del mando           <- llave retirada (D-1)
  Esclavo/src/menu.cpp:227             desde el menu                         <- botonAceptar()=false
```

Devuelve un `RechazoDegradado`, **no un bool**, y son SEIS motivos con un `$ERR`
cada uno: `SIN_HORA` `SIN_CONFIG` `CICLO_NULO` `SIN_SYNC` `SYNC_VENCIDA`
`AMBAR_VIGENTE`. El texto lo pone `degradado_textoRechazo()`, un solo sitio.

> **El Degradado es el UNICO modo que da verde sin confirmar la otra punta.** El
> choque no lo impide un enclavamiento: lo impide una **desigualdad numerica**, y el
> margen medido es **1,44** -aguanta 29 s de desfase; el equipo acumula 20,2 s en
> 48 h-, no el **2** que dos comentarios afirmaban.

##### 2.9 SFTY-21 - el veto que se borra si se borra su armador

`[MEDIDO 07/09 - grep -rn mando_ambarLocal Esclavo/]`

```
  Esclavo/src/mando.cpp:103        bool mando_ambarLocal() { return ambarLocal; }   <- ARMADOR
  Esclavo/src/main.cpp:453,:476,:617   if (!mando_ambarLocal() && !bluetooth_ambarEmergencia())
  Esclavo/src/bluetooth.cpp:620,:631   deciden CANCELAR_AMBAR
```

**CINCO llamadas vivas, no tres.** Retirar el armador **no deja los `if` inertes: los
deja siempre ciertos, o sea BORRA el veto.** Con el mando desmontado (D-1) la bandera
simplemente no se arma nunca, que es lo correcto. Ademas el banco se caeria en
**ABORTADO y no en rojo**: son 13 packs, y los dos modelos leen constantes de
`mando.cpp` **en el import**.

##### 2.10 El ESP32 de expansion - accesorio, NO controlador

`[MEDIDO 07/09 - ls ESP32_Expansion/src + cabeceras de cada fichero]`

| fichero | lineas | funcion |
|---|---|---|
| `main.cpp` | ~~202~~ 223 | arranque; el orden es la especificacion. Desde `D-26` llama a `siembra_revisar()` en `loop()`, detras de `reloj_revisar()` |
| `vigilante.cpp` | 245 | **FUNCION 1 - watchdog.** `esp_task_wdt` + parte de arranque (`esp_reset_reason`, 11 causas) |
| `reloj_ds3231.cpp` | 336 | **FUNCION 2 - reloj** DS3231 por I2C (GPIO21/22, 0x68). Bit OSF |
| `puente.cpp` | ~~387~~ 398 | **FUNCION 3 - puente SPP.** No origina, no parte ni une tramas. Desde `D-26` lo que `despachador_esParaElPuente()` reclama NO cruza (antes `SET_RTC` cruzaba) |
| `enlace_stm32.cpp` | 92 | **UNICO fichero del proyecto que nombra `Serial2`** |
| `despachador.cpp` | ~~273~~ 355 | el TERCER despachador: `SET_RTC:` y `CMD:LEER_RTC`, y desde `D-26` el descarte de `HORA_ESP32` del telefono. En la rama `RELOJ_OK` de `SET_RTC` llama a `siembra_ahora()` |
| 🆕 `siembra.cpp` *(+ `include/siembra.h`, 39)* | 109 | **`D-20`/`D-26` (11/09, `68dd2c5`): LA HORA DEL `DS3231` HACIA EL STM32.** Compone `CMD:HORA_ESP32:%04d-%02d-%02d,%02d:%02d:%02d` (`FORMATO_HORA_ESP32`) **solo** con lo que devuelve `reloj_leer()`, y lo escribe por `enlace_escribirLinea()`. Sale al arrancar (con dos reintentos, `SIEMBRA_REINTENTO_1_MS`/`_2_MS`), tras cada `SET_RTC` bueno y cada `SIEMBRA_INTERVALO_MS` (~~300 s~~ **120 s desde el 12/09,
`dca17cd`, D-26 (2)** `[MEDIDO 12/09 - grep SIEMBRA_INTERVALO_MS ESP32_Expansion/include/contrato.h]`) -constantes en
  `contrato.h`-. Es la **segunda** linea que el ESP32 origina hacia el STM32 (la otra es el latido de
  `vigilante.cpp`): `esp32_05_no_origina` censa las escrituras y no admite una tercera. `true` = «salio entera por
  J17», no «el STM32 la acepto» |
| `transporte_app.cpp` | 119 | transporte hacia el telefono |
| `trama.cpp` | 70 | formato y CRC |

`[MEDIDO 11/09 sobre 68dd2c5 - wc -l ESP32_Expansion/src/*.cpp include/siembra.h]` para las
filas que cambiaron con `D-26`; el resto sigue siendo la medida del 07/09.

**Las tres barreras del puente, con un pack cada una:** NO ORIGINA (`esp32_05`) ·
NO PARTE NI UNE TRAMAS (`esp32_06`) · SILENCIO NO ES ORDEN (`esp32_08`). **Y NO
FILTRA POR COMANDO**: retransmite toda trama bien formada; valida FORMATO.

**El STM32 sigue siendo el controlador.** El ESP32 PIDE; el STM32 acepta o rechaza.
SFTY-6 mira la radio, no J17: un ESP32 colgado no puede mover una luz ni disparar un
ambar.

> 🔵 **`D-26`: EL ESP32 ES LA FUENTE DE LA HORA de su STM32** (`siembra.cpp`), y de la hora
> cuelgan la fase del Degradado y la franja nocturna. **Este lado:** solo sale la que da
> `reloj_leer()`, la barrera del `DS3231`. **El STM32:** en Degradado un salto mayor que el
> margen pasa por rojo; sin siembra buena en tres cadencias, `$ALARM ...EVENTO:HORA_ESP32...`.
> **Lo que NO hace:** declarar vieja la hora que tiene. Lo dice la cabecera de
> `ESP32_Expansion/src/main.cpp`; el porque, `roadmap.md` `N-162`/`D-21`.

##### 2.11 El enlace por radio

`[MEDIDO 07/09 - grep SFTY6_SILENCIO_MS en los dos protocolo.h]`

```
  Maestro/include/protocolo.h:149   #define SFTY6_SILENCIO_MS   25000UL
  Esclavo/include/protocolo.h:149   #define SFTY6_SILENCIO_MS   25000UL   (ficheros byte-identicos)
```

**En la rama son 25 s desde N-71.** En el poste siguen los 12 s: el equipo de calle
es la V8.4 (`e303485`) y esto no ha salido de la rama. El reporte de campo -"se va a
ambar a los 12 segundos, por nada"- **confirma N-71 por el otro lado**: el equipo se
rendia antes de terminar de intentarlo. La desigualdad la **recalcula**
`costura_09_presupuesto_radio` desde el C++.

Configuracion vigente: **2 radios en enlace directo, sin repetidor**, `2.4 kbps`,
`M0`/`M1` ambos en OFF.

---

#### 3. LOS INSTRUMENTOS Y SUS RUTAS

**EL TAMANO DEL APARATO DE MEDIR** -la medida de CLAUDE.md 8, que se recalcula, no se
recita-. `[MEDIDO 12/09]`, con la definicion literal de esa regla:

```
$ cat {Maestro,Esclavo,Repetidor}/{src/*.cpp,include/*.h} | wc -l          18.784
$ find Simulaciones Validacion_* -type f \( -name '*.py' -o -name '*.cpp'       -o -name '*.h' -o -name '*.ps1' \)
  -not -path '*/build*'       -not -path '*__pycache__*' | xargs cat | wc -l      53.520  (+ compuerta.py 994)

  firmware      18.784 lineas
  instrumento   54.514 lineas          ratio  2,90 : 1
```

🔴 **EL RATIO SUBE Y NADIE LO ESTABA MIRANDO:** 2,31 el 02/09 · 2,27 el 07/09 ·
**2,90 hoy**. En cinco dias el aparato de medir crecio ~12.000 lineas y el firmware ~0.
**Ninguna de esas 54.514 lineas ha tocado una tarjeta desde el 31/07.** Antes de escribir un pack nuevo la pregunta no
  es *"¿esta bien hecho?"*
sino *"¿esto acerca una tarjeta cargada, o la sustituye?"*.

> **ESTA ES LA SECCION MAS UTIL DE ESTE FICHERO.** Los validadores **no incluyen el
> firmware: lo PARSEAN**, y direccionan cada archivo por tuplas
> `("Maestro","src","mando.cpp")`. **Mover o renombrar un fichero rompe un
> instrumento**, y el movimiento y la actualizacion de rutas van en el MISMO commit.

##### 3.1 Como direccionan - `banco/fuente.py` es la unica puerta

`[MEDIDO 07/09 - lectura de Simulaciones/banco/fuente.py]`

| funcion | que hace | si falta el fichero |
|---|---|---|
| `ruta(*partes)` | resuelve dentro de `01_Firmware` | **`Abortado`** |
| `texto()` / `codigo()` | el fuente; `codigo()` quita comentarios | **`Abortado`** |
| `constante(partes, patron, ...)` | lee un numero del C++ | **`Abortado`. Sin valor por defecto, nunca** |
| `comando(partes, nombre)` | codigo de comando en hex | **`Abortado`** |
| `huella()` | SHA-256 del fichero **completo** | **`Abortado`** |
| `fuentes_de(punta, carpeta, ext)` | censa el DIRECTORIO, no una lista a mano | `Abortado` si falta el dir |
| `existe()` | pregunta sin morir (para la migracion a `lib/Common`) | devuelve `False` |
| `ruta_repo()` / `texto_repo()` | **desde la RAIZ del repositorio** - documentos y app | **`Abortado`** |
| `actas()` / `acta()` | las actas de `evidencia/`, ordenadas **por su NOMBRE** | `Abortado` si no hay |

##### 3.2 La guarda de rutas - que cubre y que NO

`[MEDIDO 12/09 - reimplementando los regex de compuerta.py sobre el arbol]`

```
RUTAS CENSADAS: 65      inexistentes: 0      (identico a lo que dice el acta del 12/09)
```

Censa dos formas, y **solo bajo cuatro roles**:

```
_RE_TRIPLE   ("Maestro"|"Esclavo"|"Repetidor"|"ESP32_Expansion", "src"|"include", "*.h|cpp|ini")
_RE_PAR      ("src"|"include", "*.h|cpp|ini")   -> se exige en Maestro Y Esclavo
```

| la guarda ... | |
|---|---|
| SI vigila | ficheros del firmware **que desaparecen**, bajo los 4 roles |
| **NO** vigila | **contenido que se muda de fichero** (`main.cpp` sigue existiendo) |
| **NO** vigila | **los DOCUMENTOS** que los packs abren con `ruta_repo()` |
| **NO** vigila | **los ficheros de la APP** (`05_Funcional/App_Semaforo/...`) |
| **NO** vigila | **los ARNESES** que los packs abren con `fw.texto("Validacion_LCD", ...)` |
| **NO** vigila | un quinto rol: un `.cpp` bajo un directorio nuevo **no lo puede nombrar ningun pack** |

> 🔴 **`[RE-MEDIDO 12/09]` - EL `PASS` DE LA GUARDA DEPENDE DEL ORDEN
> ALFABETICO DE LOS PACKS.** El censo acumula en un set GLOBAL y solo expande un
> par `("src","vigilante.cpp")` a las dos puntas **si el triple no esta ya dentro**.
> Corriendo el mismo censo con los ficheros en orden inverso:
> ```
> orden real (sorted)  -> 65 rutas, 0 inexistentes
> orden INVERSO        -> 70 rutas, 4 inexistentes:
>                         Esclavo/src/trama.cpp   Esclavo/src/vigilante.cpp
>                         Maestro/src/trama.cpp   Maestro/src/vigilante.cpp
> ```
> Hoy funciona porque `esp32_02` y `esp32_09` -que traen los triples de
> `ESP32_Expansion/src/vigilante.cpp` y `trama.cpp`- ordenan **antes** que
> `esp32_10`, que trae los pares. **Renombrar `esp32_02` o `esp32_09` a algo que
> ordene despues de `esp32_10` pone la guarda en ABORTADO** y con ella el sentido de
> las otras 19 filas. No es un defecto de hoy; es una dependencia sin escribir.

##### 3.3 Que ficheros direcciona cada pack

`[MEDIDO 12/09 - censo pack a pack con los regex de la guarda, mas rastreo real
instrumentando `fuente.ruta`/`fuente.ruta_repo` y corriendo los 82 packs]`

Clave: `M:` = Maestro · `E:` = Esclavo · `X:` = ESP32_Expansion · `M+E:` = tupla de
dos, se exige en las dos puntas · `via modelos` = no direcciona por su cuenta, las
rutas las pone `banco/modelos/`.

| pack | ficheros del firmware que direcciona | documentos / app | via |
|---|---|---|---|
| `app_01_comandos` | `X:despachador.cpp`, `M+E:bluetooth.cpp` | `app.js`, `index.html` | propia |
| `app_02_modos_simetricos` | `E:modo_degradado.h`, `M:modos.h`, `M+E:menu.h`, `M+E:bluetooth.cpp` | - | propia |
| `app_03_sin_ok_mudo` | `M+E:bluetooth.cpp` | - | propia |
| `app_04_valores_de_status` | `E:bluetooth.cpp`, `M+E:botones.cpp`, `M+E:semaforo.cpp` | `app.js` | propia |
| `app_05_sin_exito_mudo` | - | `app.js` | propia |
| `app_06_formato_de_hora` | `X:despachador.cpp`, `M+E:bluetooth.cpp` | `app.js`, `js/courier_rtc.js` | propia |
| `app_07_generadores_de_trama` | `E:bluetooth.cpp`, `M:bluetooth.cpp` | `index.html` + **censo de `App_Semaforo/`** | propia |
| `app_08_enrutado_por_punta` | `E:bluetooth.cpp`, `M:bluetooth.cpp` | `app.js` | propia |
| `app_09_registro_de_enlace` | - | `app.js`, `index.html`, `js/registro_enlace.js`, `style.css` | propia |
| `app_10_ack_con_varios_sies` | `X:despachador.cpp`, `E:bluetooth.cpp`, `M:bluetooth.cpp` | `app.js` | propia |
| `app_11_rangos_de_tiempos` | `M:coordinador.h`, `M:limites_ciclo.h`, `M:modo_automatico.cpp` | `app.js`, `js/config.js`, `index.html` | propia |
| `app_12_un_solo_parser` | `M+E:bluetooth.cpp` | `app.js`, `js/nmea_parser.js`, `tests/test_unitarios.js`, `test_unitarios_app.js` | propia |
| `barrera_01_pines_de_luz` | `M:main.cpp`, `M+E:pines.h` | - | propia |
| `barrera_02_dos_puntas` | `M+E:semaforo.cpp` | - | propia |
| `barrera_03_talanquera` | `M+E:pines.h`, `M+E:semaforo.cpp` | - | propia |
| `barrera_04_arnes_dos_puntas` | `M:protocolo.h`, `M:coordinador.cpp`, `M:identidad.cpp` | 🔴 **`Validacion_Automatico/dos_puntas` y `compilar_dos_puntas.ps1`** | propia |
| `camara_01_demanda` | `E:demanda.cpp`, `E:main.cpp`, `M+E:pines.h` | - | propia |
| `camara_02_j16` | `E:mando.cpp`, `M:bluetooth.cpp`, `M+E:botones.h`, `M+E:pines.h`, `M+E:botones.cpp`, `M+E:demanda.cpp`, `M+E:main.cpp` | - | propia |
| `camara_03_vigilante` | `M:limites_ciclo.h`, `M:modo_inteligente.cpp`, `M+E:botones.h`, `M+E:bluetooth.cpp`, `M+E:botones.cpp`, `M+E:demanda.cpp` | - | propia |
| `costura_01_contratos` | `M:ciclo_degradado.h`, `M+E:identidad.h`, `M+E:protocolo.h`, `M+E:respaldo.h`, `M+E:identidad.cpp`, `M+E:protocolo.cpp`, `M+E:respaldo.cpp` | - | propia |
| `costura_02_fase_ciclo` | (via modelos) | - | **modelos** |
| `costura_03_comandos` | `M:coordinador.cpp` | - | **modelos** |
| `costura_04_config` | (via modelos) | - | **modelos** |
| `costura_05_limite_48h` | (via modelos) | - | **modelos** |
| `costura_06_reanudacion` | `M+E:modo_degradado.cpp` 🆕 *(12/09: la fila decia solo «via modelos»; lo nombra por tupla)* | - | **modelos** |
| `costura_07_motivos_rechazo` | (via modelos) | - | **modelos** |
| `costura_08_silencio` | `M+E:protocolo.h` | - | propia |
| `costura_09_presupuesto_radio` | `M:protocolo.h`, `M:coordinador.cpp` | - | propia |
| `costura_10_funciones_muertas` | **censa `src/` e `include/` enteros** (`fuentes_de`) | - | propia |
| `costura_11_lcd_sin_bus` | `E:pines.h`, `E:lcd.cpp`, `M:pines.h`, `M:lcd.cpp` | - | propia |
| `costura_12_acuse_de_demanda` | `E:main.cpp`, `M:coordinador.cpp`, `M+E:protocolo.h` | - | propia |
| `costura_12_margen_deriva` | (via modelos) | 🔴 **`Validacion_Automatico/compilar_degradado.ps1` y `dos_puntas`** | **modelos** |
| `costura_13_ambar_ordenado` | `E:main.cpp` | - | propia |
| `costura_14_cancela_ambar` | `E:bluetooth.cpp`, `E:main.cpp`, `M:coordinador.cpp`, `M:main.cpp`, `M:modo_ambar.cpp` | - | propia |
| 🆕 `decisiones_01_anclas` *(⚠️ FALTABA EN ESTA TABLA hasta el 12/09; es el unico `FALLA` del acta)* | no nombra ninguno por tupla: **censa los CUATRO roles, `src` E `include` enteros** (`fuentes_de(punta, car, ".cpp")` + `".h"`), los 95 ficheros | `DECISIONES.md` (de ahi saca la tabla `D-x`; si falta, **ABORTA**) + **censa por directorio** `05_Funcional/*.md` y `04_Manuales/*.md` para exigir que cada `D-x` vigente se nombre en algun manual | propia |
| `documentos_01_cifras_del_acta` | - | `README.md`, `ESTADO.md`, **el acta ANTERIOR de `evidencia/`** | propia |
| `documentos_02_trazabilidad_sfty` | - | `OPTIMIZACIONES.md` | propia |
| `documentos_03_trama_status` | `M+E:bluetooth.cpp` | `05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.md`, `app.js`, `index.html` + **las 3 copias de la app** | propia |
| `documentos_04_cifras_sin_vigilante` | `M:limites_ciclo.h`, `M:mando.cpp`, `M+E:protocolo.h` | 🔴 **`04_Manuales/MANUAL_USUARIO.md`, `04_Manuales/MANUAL_HARDWARE.md`**, `CERTIFICACION_SW.md`, `OPTIMIZACIONES.md` | propia |
| `documentos_05_copias_coherentes` | - | `README.md`, `ESTADO.md`, `CERTIFICACION_SW.md` (`A.md`/`B.md` son el `control_negativo`, no rutas) | propia |
| 🆕 `documentos_06_no_reabre_lo_cerrado` | - | no direcciona firmware. **CENSA por directorio** -no lista a mano- `05_Funcional/*.md`+`*.html` (primer nivel), `04_Manuales/*.md` (primer nivel) y la raiz: todo `*.md` **y `ARQUITECTURA.map`** (ver 0). Excluye `DECISIONES.md`, `roadmap_hist.md`, `05_Funcional/historico/` y `99_Legacy/` a proposito: son la fuente que deroga o la cronica de lo derogado, no algo que pueda reabrirlo | propia |
| `enlace_01_transporte` | `E:bluetooth.h`, `E:pines.h`, `E:bluetooth.cpp`, `E:lcd.cpp`, `M:bluetooth.h`, `M:pines.h`, `M:bluetooth.cpp`, `M:lcd.cpp`, `M:main.cpp` | - | propia |
| `enlace_02_silencio_j17` | `M+E:bluetooth.cpp` | - | propia |
| `esclavo_01_latch_ambar` | (via modelos) | - | **modelos** |
| `esclavo_02_inhibicion_menu` | (via modelos) | - | **modelos** |
| `esclavo_03_par_config` | (via modelos) | - | **modelos** |
| `esclavo_04_desfase` | (via modelos) | - | **modelos** |
| `esclavo_05_hora_atomica` | (via modelos) | - | **modelos** |
| `esclavo_06_no_abre_paso` | `E:bluetooth.cpp`, `E:demanda.cpp`, `E:protocolo.cpp` | - | propia |
| `esclavo_07_ambar_emergencia` | `E:bluetooth.h`, `E:protocolo.h`, `E:bluetooth.cpp`, `E:main.cpp`, `M:bluetooth.cpp` · **ademas censa `Maestro/src` ENTERO** (`fw.fuentes_de("Maestro","src")`) para exigir que ALGUN fichero de esa carpeta lea el aviso de radio que emite el ambar de emergencia -no una lista a mano- | - | propia |
| `esclavo_08_ambar_en_degradado` | `E:bluetooth.cpp`, `E:main.cpp`, `E:mando.cpp`, `E:modo_degradado.cpp`, `M:coordinador.cpp` · **ademas censa `Esclavo/src` ENTERO** (`fw.fuentes_de("Esclavo","src")`) fichero a fichero, para que un `.cpp` nuevo que ponga o quite el ambar entre bajo vigilancia solo | - | propia |
| `esp32_01_watchdog_desigualdad` | `X:contrato.h`, `E:main.cpp`, `M:coordinador.cpp`, `M+E:protocolo.h` | `app.js` | propia |
| `esp32_02_watchdog_alimentado` | `X:contrato.h`, `X:main.cpp`, `X:puente.cpp`, `X:vigilante.cpp` | - | propia |
| `esp32_03_ack_que_mira` | `X:reloj_ds3231.h`, `X:despachador.cpp` | - | propia |
| `esp32_04_osf` | `X:contrato.h`, `X:main.cpp`, `X:reloj_ds3231.cpp` | - | propia |
| `esp32_05_no_origina` | `X:puente.cpp`, `X:contrato.h` *(11/09, `D-26`: `LATIDO_LINEA`)* y **censa `ESP32_Expansion/src/*.cpp` e `include/*.h` enteros** (`fuentes_de`): cada escritura hacia el STM32 del proyecto, con dos excepciones con nombre -el latido y `siembra.cpp`- | - | propia |
| `esp32_06_no_parte_tramas` | `X:puente.h`, `X:enlace_stm32.cpp`, `X:puente.cpp` | - | propia |
| `esp32_07_presupuesto_bytes` | `X:contrato.h`, `M:coordinador.h`, `M:limites_ciclo.h`, `M:protocolo.h`, `M:bluetooth.cpp`, `M:coordinador.cpp` | - | propia |
| `esp32_08_silencio_no_es_orden` | `X:enlace_stm32.cpp`, `X:main.cpp`, `X:puente.cpp` | - | propia |
| `esp32_09_contrato_de_bytes` | `X:contrato.h`, `X:enlace_stm32.cpp`, `X:trama.cpp`, `M+E:bluetooth.cpp` | - | propia |
| `esp32_10_parte_de_arranque` | `X:contrato.h`, `X:vigilante.h`, y **pares** `src/main.cpp`, `src/trama.cpp`, `src/vigilante.cpp` ⚠️ ver 3.2 | - | propia |
| `esp32_11_bien_formada_no_es_cierta` | `X:contrato.h`, `X:reloj_ds3231.h`, `X:reloj_ds3231.cpp` | - | propia |
| `esp32_12_consulta_de_reloj` | `X:despachador.h`, `X:reloj_ds3231.h`, `X:despachador.cpp`, `X:puente.cpp`, `X:siembra.cpp` *(11/09, `D-26`)*, `M+E:bluetooth.cpp` | - | propia |
| 🆕 `esp32_13_siembra_de_hora` | `X:contrato.h`, `X:siembra.cpp`, `X:despachador.cpp`, `X:main.cpp`, `M:modo_degradado.cpp`, `E:modo_degradado.cpp`, `M+E:reloj.h`, `M+E:reloj.cpp`, `M+E:bluetooth.cpp` · ⚠️ **abre tambien `{M,E}/src/main.cpp` y la guarda NO lo registra**: su propia `MAIN = ("ESP32_Expansion","src","main.cpp")` tapa el par que generan sus `fw.codigo(p,"src","main.cpp")` -el hueco de 3.2 mordiendo dentro de un solo pack- `[MEDIDO 12/09]` | - | propia |
| `flash_01_lastre` | **parsea `firmware.map`** del enlazador, no un fuente | - | propia |
| `identidad_01_serie` | `M:identidad.cpp` | - | propia |
| `maestro_01_mando` | `M:main.cpp`, `M:modo_automatico.cpp` | - | **modelos** |
| `maestro_02_respaldo` | `M:respaldo.cpp`, `M+E:respaldo.h` | - | **modelos** |
| `maestro_03_puerta_degradado` | `E:lcd.cpp`, `E:modo_degradado.cpp`, `M:coordinador.cpp`, `M:lcd.cpp`, `M:modo_degradado.cpp` | - | **modelos** |
| `maestro_04_sync_horaria` | (via modelos) | - | **modelos** |
| `maestro_05_ciclo_sin_radio` | `M:modo_degradado.cpp` | - | **modelos** |
| `maestro_06_fuentes_pantalla` | `M:lcd.cpp` | 🔴 **`Validacion_LCD/arnes_lcd.cpp`** | propia |
| `maestro_07_menu_opciones` | `M:menu.cpp` | 🔴 **`Validacion_LCD/arnes_lcd.cpp`** | propia |
| `maestro_08_set_tiempos` | `M:limites_ciclo.h`, `M:bluetooth.cpp`, `M:modo_automatico.cpp` | - | propia |
| `maestro_09_test_leds` | `M:semaforo.cpp` | - | propia |
| `maestro_10_coordinador_alcanzable` | `M:coordinador.h`, `M:protocolo.h`, `M:main.cpp` | - | propia |
| `maestro_11_manual_no_cicla` | `M:coordinador.cpp`, `M:modo_automatico.cpp`, `M:modo_manual.cpp` | - | propia |
| `maestro_12_dar_paso_sin_coordinador` | `M:bluetooth.cpp`, `M:main.cpp` | - | propia |
| `reloj_01_consulta_por_bluetooth` | `E:reloj.h`, `E:bluetooth.cpp`, `M:reloj.h`, `M:bluetooth.cpp` | - | propia |
| `reloj_02_siembra_que_miente` | `E:reloj.cpp`, `E:reloj.h`, `E:bluetooth.cpp`, `M:reloj.cpp`, `M:reloj.h`, `M:bluetooth.cpp`, `M:bluetooth.h` *(11/09: faltaba; `BLUETOOTH_H`, la excepcion que se comprueba declarada)* | - | propia |
| 🆕 `reloj_03_manda_la_radio` *(`D-26`)* | `E:bluetooth.cpp`, `E:reloj.cpp`, `E:main.cpp`, `M:bluetooth.cpp`,
  `M:reloj.cpp`, `X:siembra.cpp`, `X:contrato.h`, y **censa `{Maestro,Esclavo}/include/*.h` y `Esclavo/src/*.cpp`
  enteros** (`fuentes_de`) | 🔴 **`Simulaciones/puente_esp32/arnes_puente.cpp`** y
  **`Validacion_Automatico/dos_puntas/{orquestador_degradado,adaptador_esclavo}.cpp`**, que lee por texto: que la
  copia de `isoBienFormado()` del arnes del puente sea la del fuente, y que el bloque E del orquestador salte la hora
  de las dos puntas. ⚠️ El arnes del Degradado SI compila `Esclavo/src/reloj.cpp` (`reloj_real/`); el del puente y
  `dos_puntas` siguen con el muñon (4) | propia |
| 🆕 `reloj_04_hora_que_caduca` *(`D-21` (1))* | `M:reloj.h`, `E:reloj.h`, `M:reloj.cpp`, `E:reloj.cpp`,
  `M:modo_degradado.cpp`, `E:modo_degradado.cpp`, `X:contrato.h`, `E:protocolo.h` *(⚠️ solo el del Esclavo se registra
  suelto: `Maestro/include/protocolo.h` lo tapa la misma tupla, igual hueco que en `esp32_13`)*, `M+E:bluetooth.cpp`,
  y el `_aguante` que importa de `esp32_13` | 🔴
  **`Validacion_Automatico/dos_puntas/{orquestador_degradado,reloj_real}`**, que es quien EJECUTA el plazo: este pack
  **evalua las expresiones de `reloj.h` en 32 bits** (suelo, techo, y que el plazo tenga sujeto y llamador en las dos
  puntas) y comprueba que el arnes compila el reloj real. Las cifras vigentes salen del C++: `grep -n
  "HSI_PPM_PEOR\|HORA_RELEVO_MS\|HORA_DERIVA_S\|HORA_CADUCA_MS" Maestro/include/reloj.h` | propia |

🔴 **LA TABLA ESTABA COMPLETA MENOS UNA FILA, Y ERA LA DEL PACK QUE HOY ESTA EN ROJO.**
`[MEDIDO 12/09 - censo pack a pack + rastreo real instrumentando `fuente.ruta`]`: de los 82
packs, **81 tenian fila y `decisiones_01_anclas` no tenia ninguna** desde que existe, pese a
ser el que 3.7 usa como argumento y el unico `FALLA` del acta. Ya la tiene, arriba.
Corregida ademas la de `costura_06`, que decia solo *(via modelos)* y nombra por tupla
`M+E:modo_degradado.cpp`. **Las otras 80 cuadran con la medida.**

Dos filas parecen no cuadrar y no es un error de la tabla, es el hueco de dedup de 3.2
mordiendo dentro de UN SOLO pack: `esp32_13` abre `{M,E}/src/main.cpp` de verdad, pero su
propia `MAIN = ("ESP32_Expansion","src","main.cpp")` tapa el par que eso genera y **la
guarda no lo registra por ese pack**; a `reloj_04` le pasa igual con
`Maestro/include/protocolo.h`. En las dos filas va escrito.

> ⚠️ **`fuentes_de` NO SALE CON EL REGEX DE LA GUARDA, y por eso se cuenta mal.**
> `[MEDIDO 12/09 - grep -rn "fuentes_de(" banco/packs/*.py banco/modelos/*.py]`: **25 packs**
> censan un directorio entero, no los seis que esta tabla venia reflejando. La columna de
> ficheros solo lista los que el pack NOMBRA; para la vuelta completa -incluido lo que cada
> pack barre por directorio- esta **3.7**.

##### 3.4 Los tres MODELOS - 19 packs se apoyan en ellos

`[MEDIDO 12/09 - grep -l 'banco.modelos' packs/*.py -> 19 ficheros]`. Las tres tablas de
abajo se re-midieron pack a pack y **cuadran exactamente**; lo que habia caducado era la
CUENTA de quien depende de ellos.

| modelo | ficheros que abre |
|---|---|
| `banco/modelos/costura.py` | `M+E`: `ciclo_degradado.h`, `modo_degradado.h`, `protocolo.h`, `respaldo.h`, `config_ciclo.cpp`, `coordinador.cpp`, `main.cpp`, `modo_degradado.cpp`, `protocolo.cpp`, `reloj.cpp`, `respaldo.cpp`, `semaforo.cpp` |
| `banco/modelos/esclavo.py` | `M+E`: `ciclo_degradado.h`, `protocolo.h`, `config_ciclo.cpp`, `coordinador.cpp`, `main.cpp`, `mando.cpp`, `menu.cpp`, `modo_degradado.cpp`, `semaforo.cpp` |
| `banco/modelos/maestro.py` | `M+E`: `ciclo_degradado.h`, `protocolo.h`, `respaldo.h`, `botones.cpp`, `coordinador.cpp`, `main.cpp`, `mando.cpp`, `modo_degradado.cpp`, `respaldo.cpp`, `semaforo.cpp` |

Los ~~17~~ **19** que dependen de ellos `[MEDIDO 12/09]`: `costura_02` `costura_03`
`costura_04` `costura_05` `costura_06` `costura_07` `costura_12_margen_deriva`
`esclavo_01` `esclavo_02` `esclavo_03` `esclavo_04` `esclavo_05` `maestro_01`
`maestro_02` `maestro_03` `maestro_04` `maestro_05` 🆕 `esp32_13_siembra_de_hora`
🆕 `reloj_04_hora_que_caduca` *(los dos entraron con `D-26`/`D-21`(1) y esta cuenta no se
habia vuelto a hacer)*.

> **Tocar un modelo mueve 19 packs a la vez.** Y los modelos leen constantes **en el
> import**: un fichero que falta no da FALLA, da **ABORTADO en cascada**.

##### 3.5 Los ficheros mas leidos - donde duele mas mover algo

`[MEDIDO 12/09]`. **SE DAN LAS DOS CUENTAS PORQUE MIDEN COSAS DISTINTAS, y la pasada
anterior publicaba una sola sin decir cual** (CLAUDE.md 7: cuando un instrumento compara
contra un borde, se escribe al lado CUAL es):

- **NOMBRAN** = packs que citan el fichero por tupla. **Es el radio de destruccion: mover
  el fichero los deja en ABORTADO.**
- **ABREN** = packs que llegan a leerlo, incluidos los que barren su directorio con
  `fuentes_de`. Mover el fichero **NO** los aborta: dejan de verlo en silencio.

| fichero | NOMBRAN (mover = ABORTADO) | ABREN |
|---|---|---|
| `Maestro/src/bluetooth.cpp` | **24** | 39 |
| `Esclavo/src/bluetooth.cpp` | **24** | 36 |
| `ESP32_Expansion/include/contrato.h` | 11 | 11 |
| `Maestro/src/coordinador.cpp` | 10 | 28 |
| `Esclavo/src/main.cpp` | 10 | 24 |
| `Maestro/include/protocolo.h` | 9 | 21 |
| `Maestro/src/main.cpp` | 8 | 23 |
| `Esclavo/include/protocolo.h` | 7 | 20 |
| `Maestro/include/pines.h` · `Esclavo/include/pines.h` | 6 c/u | 21 · 18 |
| `ESP32_Expansion/src/despachador.cpp` | 6 | 12 |
| `Maestro/src/modo_degradado.cpp` | 5 | **27** |
| `Maestro/src/reloj.cpp` | 4 | 24 |

⚠️ **Las dos columnas se separan mucho y por eso la sola no valia:** `modo_degradado.cpp`
del Maestro es el 4.º fichero mas LEIDO del arbol y solo el 11.º mas NOMBRADO. La vuelta
completa, fichero a fichero, en **3.7**.

##### 3.6 Las rutas que NADIE vigila - el hueco abierto

`[MEDIDO 12/09 - rastreo real: instrumentando `fuente.ruta_repo` y corriendo los 82 packs]`

Estos ficheros los ABREN instrumentos con `ruta_repo()`/`texto_repo()` **-que ABORTA si
faltan-** y **la guarda de rutas no los censa**. Moverlos deja packs en ABORTADO sin que
ninguna fila avise. 🔴 **La lista del 07/09 se quedaba corta por CUATRO sitios, y todos
son de la app** (CLAUDE.md 14: la lista se queda corta siempre por lo que no es firmware
ni documento):

| ruta | quien la abre | efecto de moverla |
|---|---|---|
| `README.md` | `documentos_01`, `documentos_05` | 2 packs ABORTADO |
| `ESTADO.md` | `documentos_01`, `documentos_05` | 2 packs ABORTADO |
| `CERTIFICACION_SW.md` | `documentos_04`, `documentos_05` | 2 packs ABORTADO |
| `OPTIMIZACIONES.md` | `documentos_02`, `documentos_04` | 2 packs ABORTADO |
| `04_Manuales/MANUAL_USUARIO.md` | `documentos_04` | 1 pack ABORTADO |
| `04_Manuales/MANUAL_HARDWARE.md` | `documentos_04` | 1 pack ABORTADO |
| `05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.md` | `documentos_03` | 1 pack ABORTADO |
| `05_Funcional/App_Semaforo/{app.js, index.html, style.css}` | 11 packs `app_*` + 2 simuladores | media docena de packs ABORTADO |
| `05_Funcional/App_Semaforo/js/{config,courier_rtc,nmea_parser,registro_enlace}.js` | `app_06`, `app_09`, `app_11`, `app_12` | 4 packs ABORTADO |
| 🆕 `05_Funcional/App_Semaforo/js/{avisos_equipo,bluetooth_driver,depuracion,site_manager}.js` | `app_07`, `documentos_03` | 2 packs ABORTADO. **`app_07` NO lleva lista a mano: saca los `.js` de los `<script src=>` de `index.html` y abre cada uno.** Un `.js` nuevo referenciado ahi entra solo en este agujero |
| `05_Funcional/App_Semaforo/tests/test_unitarios.js` | `app_12` | 1 pack ABORTADO |
| 🆕 `05_Funcional/App_Semaforo/test_unitarios_app.js` | `app_12` (`SUITE_APP`, tupla literal) | 1 pack ABORTADO |
| 🆕 **las COPIAS** `App_Semaforo/www/*` y `android/app/src/main/assets/public/*` (11 ficheros cada una) | `documentos_03` | 1 pack ABORTADO. Las compara una a una por su ruta: mover o borrar una copia no da «copias distintas», da **ABORTADO** |
| `Validacion_LCD/arnes_lcd.cpp` | `maestro_06`, `maestro_07` | 2 packs ABORTADO |
| `Validacion_Automatico/compilar_dos_puntas.ps1` y `dos_puntas/` | `barrera_04` | 1 pack ABORTADO |
| `Validacion_Automatico/compilar_degradado.ps1` | `costura_12_margen_deriva` | 1 pack ABORTADO |
| `evidencia/*_compuerta.txt` | `fuente.actas()`, `documentos_01`, `documentos_04` | 2 packs ABORTADO |
| 🆕 `DECISIONES.md` | `decisiones_01_anclas` | 1 pack ABORTADO |

> ⚠️ **Y LO CONTRARIO, que enganaba al leer esta tabla:** `decisiones_01_anclas` y
> `documentos_06` tambien tocan **todos** los `.md` de `05_Funcional`, `04_Manuales` y la
> raiz, pero por `os.listdir` del directorio. **Mover uno de esos NO aborta nada: deja de
> mirarse, en silencio** -que es peor, porque un ABORTADO grita y un hueco no. La unica
> ruta de las suyas que aborta es `DECISIONES.md`, que si va nombrada.

> **Y un pack en ABORTADO tumba la fila entera `banco por packs`**: **82** packs
> `[MEDIDO 12/09]` sin medir por un fichero mudado. La guarda seguiria publicando
> *"65 rutas, todas existen"*.

##### 3.7 LA VUELTA: de cada fichero, quien lo abre y quien lo compila

`[MEDIDO 12/09 - censo `_RE_TRIPLE`/`_RE_PAR` pack a pack + RASTREO REAL: se instrumentan
`fuente.ruta`, `fuente.ruta_repo` y `fuente.fuentes_de` y se corren los 82 packs, anotando
cada apertura + lectura de los siete `compilar*.ps1` y de los `#include "*.cpp"` de los
adaptadores]`

**3.3 dice «este pack abre estos ficheros». Esta seccion contesta al reves.** No se copio
3.3 al derecho: se midio aparte, y por eso caza lo que 3.3 no veia.

**LAS TRES FORMAS DE DIRECCIONAR, y por que hay que buscar las tres:**

| forma | como se busca | ¿mover el fichero ABORTA? |
|---|---|---|
| **tupla** `("Maestro","src","mando.cpp")` o su par suelto | los regex de la guarda, **pack a pack, NO acumulados** | **SI** |
| **`fuentes_de(punta, carpeta[, ext])`** - censo de un DIRECTORIO | `grep -rn "fuentes_de(" banco/packs/*.py banco/modelos/*.py`, y leer si los argumentos son literales o una variable que itera `PUNTAS` | **NO** - deja de verlo en silencio |
| **el arnes que COMPILA el `.cpp` real** | los `compilar*.ps1` y `grep -rn '#include ".*\.cpp"'` en los adaptadores | **SI** - el `.ps1` hace `Write-Error` |

> 🔴 **ACUMULAR ENTRE PACKS ESCONDE FILAS, Y ES LO QUE HACIA LA PASADA ANTERIOR.** La
> guarda de `compuerta.py` acumula en un set global a proposito -su trabajo es «¿existe
> todo?»-, pero para ESTA tabla eso tapa las parejas de un pack detras de la tupla de
> OTRO. Se censa aislado. Y **`fuentes_de` no sale con el regex de la guarda**: son
> **25 packs**, no los seis que 3.3 venia reflejando.

**LAS DOS PREGUNTAS QUE ESTA TABLA CONTESTA DE UN VISTAZO:**

1. *¿si muevo este fichero, que rompo?* -> columna 2. Es una cuenta de **ABORTADO**, y un
   solo pack abortado tumba la fila `banco por packs` entera (3.6).
2. *¿esto lo COMPILA alguien, o solo se lee por texto?* -> columna 3, que es CLAUDE.md 6.3:
   **un pack de texto no ve un defecto del TIEMPO.**

Clave de la columna 3: `LCD` `CICLO` `RESP` `AUTO` = los cuatro `Validacion_*` · `2P` =
`dos_puntas` · `2P-DEG` = `dos_puntas` del Degradado · `PUENTE` = `Simulaciones/puente_esp32`
· `PlatformIO` = solo lo compila el cruzado de las filas 2-5 de la compuerta, **nunca se
ejecuta en el PC**. Prefijos: `M:` Maestro · `E:` Esclavo · `R:` Repetidor · `X:` ESP32.

| fichero | packs que lo NOMBRAN (mover = ABORTADO) | arnes que lo COMPILA y EJECUTA |
|---|---|---|
| `M:bluetooth.cpp` | **24** packs | PUENTE |
| `M:botones.cpp` | `app_04_valores_de_status`, `camara_02_j16`, `camara_03_vigilante` | AUTO |
| `M:coordinador.cpp` | **10** packs | 2P, 2P-DEG, AUTO, PUENTE |
| `M:demanda.cpp` | `camara_02_j16`, `camara_03_vigilante` | AUTO, PUENTE |
| `M:identidad.cpp` | `barrera_04_arnes_dos_puntas`, `costura_01_contratos`, `identidad_01_serie` | PUENTE |
| `M:lcd.cpp` | **4** packs | LCD |
| `M:main.cpp` | **8** packs | PlatformIO |
| `M:mando.cpp` | `documentos_04_cifras_sin_vigilante` | 2P, AUTO, PUENTE |
| `M:menu.cpp` | `maestro_07_menu_opciones` | LCD |
| `M:modo_alcance.cpp` | — *(solo censo de directorio: 20)* | PlatformIO |
| `M:modo_ambar.cpp` | `costura_14_cancela_ambar` | 2P, 2P-DEG |
| `M:modo_automatico.cpp` | **4** packs | 2P, AUTO, PUENTE |
| `M:modo_degradado.cpp` | **5** packs | 2P-DEG |
| `M:modo_hora.cpp` | — *(solo censo de directorio: 20)* | PlatformIO |
| `M:modo_inteligente.cpp` | `camara_03_vigilante` | AUTO |
| `M:modo_manual.cpp` | `maestro_11_manual_no_cicla` | PlatformIO |
| `M:modos.cpp` | — *(solo censo de directorio: 20)* | 2P-DEG, LCD, PUENTE |
| `M:protocolo.cpp` | `costura_01_contratos` | PlatformIO |
| `M:reloj.cpp` | **4** packs | 2P-DEG |
| `M:respaldo.cpp` | `costura_01_contratos`, `maestro_02_respaldo` | 2P-DEG, RESP |
| `M:semaforo.cpp` | **4** packs | 2P, 2P-DEG, AUTO, PUENTE |
| `M:bluetooth.h` | `enlace_01_transporte`, `reloj_02_siembra_que_miente` | PlatformIO *(via #include)* |
| `M:botones.h` | `camara_02_j16`, `camara_03_vigilante` | PlatformIO *(via #include)* |
| `M:ciclo_degradado.h` | `costura_01_contratos` | PlatformIO *(via #include)* |
| `M:coordinador.h` | `app_11_rangos_de_tiempos`, `esp32_07_presupuesto_bytes`, `maestro_10_coordinador_alcanzable` | PlatformIO *(via #include)* |
| `M:demanda.h` | — *(solo censo de directorio: 6)* | PlatformIO *(via #include)* |
| `M:identidad.h` | `costura_01_contratos` | PlatformIO *(via #include)* |
| `M:lcd.h` | — *(solo censo de directorio: 6)* | PlatformIO *(via #include)* |
| `M:limites_ciclo.h` | **5** packs | PlatformIO *(via #include)* |
| `M:mando.h` | — *(solo censo de directorio: 6)* | PlatformIO *(via #include)* |
| `M:menu.h` | `app_02_modos_simetricos` | PlatformIO *(via #include)* |
| `M:modo_alcance.h` | — *(solo censo de directorio: 6)* | PlatformIO *(via #include)* |
| `M:modo_ambar.h` | — *(solo censo de directorio: 6)* | PlatformIO *(via #include)* |
| `M:modo_automatico.h` | — *(solo censo de directorio: 6)* | PlatformIO *(via #include)* |
| `M:modo_degradado.h` | — *(solo censo de directorio: 6)* | PlatformIO *(via #include)* |
| `M:modo_hora.h` | — *(solo censo de directorio: 6)* | PlatformIO *(via #include)* |
| `M:modo_inteligente.h` | — *(solo censo de directorio: 6)* | PlatformIO *(via #include)* |
| `M:modo_manual.h` | — *(solo censo de directorio: 6)* | PlatformIO *(via #include)* |
| `M:modos.h` | `app_02_modos_simetricos` | PlatformIO *(via #include)* |
| `M:pines.h` | **6** packs | PlatformIO *(via #include)* |
| `M:pines_repetidor.h` | — *(solo censo de directorio: 6)* | 🔴 **NADIE lo compila** |
| `M:protocolo.h` | **9** packs | PlatformIO *(via #include)* |
| `M:reloj.h` | **4** packs | PlatformIO *(via #include)* |
| `M:respaldo.h` | `costura_01_contratos`, `maestro_02_respaldo` | PlatformIO *(via #include)* |
| `M:semaforo.h` | — *(solo censo de directorio: 6)* | PlatformIO *(via #include)* |
| `E:bluetooth.cpp` | **24** packs | 2P, PUENTE |
| `E:botones.cpp` | `app_04_valores_de_status`, `camara_02_j16`, `camara_03_vigilante` | PlatformIO |
| `E:config_ciclo.cpp` | — *(solo censo de directorio: 17)* | 2P, 2P-DEG |
| `E:demanda.cpp` | **4** packs | 2P, 2P-DEG, PUENTE |
| `E:identidad.cpp` | `costura_01_contratos` | PUENTE |
| `E:lcd.cpp` | `costura_11_lcd_sin_bus`, `enlace_01_transporte`, `maestro_03_puerta_degradado` | LCD |
| `E:main.cpp` | **10** packs | 2P, 2P-DEG |
| `E:mando.cpp` | `camara_02_j16`, `esclavo_08_ambar_en_degradado` | 2P, 2P-DEG |
| `E:menu.cpp` | — *(solo censo de directorio: 18)* | LCD |
| `E:modo_degradado.cpp` | **5** packs | 2P, 2P-DEG, LCD, PUENTE |
| `E:protocolo.cpp` | `costura_01_contratos`, `esclavo_06_no_abre_paso` | PlatformIO |
| `E:reloj.cpp` | **4** packs | 2P-DEG |
| `E:respaldo.cpp` | `costura_01_contratos` | 2P, 2P-DEG |
| `E:semaforo.cpp` | `app_04_valores_de_status`, `barrera_02_dos_puntas`, `barrera_03_talanquera` | 2P, 2P-DEG, PUENTE |
| `E:bluetooth.h` | `enlace_01_transporte`, `esclavo_07_ambar_emergencia` | PlatformIO *(via #include)* |
| `E:botones.h` | `camara_02_j16`, `camara_03_vigilante` | PlatformIO *(via #include)* |
| `E:ciclo_degradado.h` | — *(solo censo de directorio: 8)* | PlatformIO *(via #include)* |
| `E:config_ciclo.h` | — *(solo censo de directorio: 6)* | PlatformIO *(via #include)* |
| `E:demanda.h` | — *(solo censo de directorio: 6)* | PlatformIO *(via #include)* |
| `E:identidad.h` | `costura_01_contratos` | PlatformIO *(via #include)* |
| `E:lcd.h` | — *(solo censo de directorio: 6)* | PlatformIO *(via #include)* |
| `E:mando.h` | — *(solo censo de directorio: 6)* | PlatformIO *(via #include)* |
| `E:menu.h` | `app_02_modos_simetricos` | PlatformIO *(via #include)* |
| `E:modo_degradado.h` | `app_02_modos_simetricos` | PlatformIO *(via #include)* |
| `E:pines.h` | **6** packs | PlatformIO *(via #include)* |
| `E:pines_repetidor.h` | — *(solo censo de directorio: 6)* | 🔴 **NADIE lo compila** |
| `E:protocolo.h` | **7** packs | PlatformIO *(via #include)* |
| `E:reloj.h` | **4** packs | PlatformIO *(via #include)* |
| `E:respaldo.h` | `costura_01_contratos`, `maestro_02_respaldo` | PlatformIO *(via #include)* |
| `E:semaforo.h` | — *(solo censo de directorio: 6)* | PlatformIO *(via #include)* |
| `R:main.cpp` | — *(solo censo de directorio: 2)* | PlatformIO |
| `R:pines_repetidor.h` | — *(solo censo de directorio: 1)* | PlatformIO *(via #include)* |
| `X:despachador.cpp` | **6** packs | PlatformIO |
| `X:enlace_stm32.cpp` | `esp32_06_no_parte_tramas`, `esp32_08_silencio_no_es_orden`, `esp32_09_contrato_de_bytes` | PlatformIO |
| `X:main.cpp` | **4** packs | PlatformIO |
| `X:puente.cpp` | **5** packs | PlatformIO |
| `X:reloj_ds3231.cpp` | `esp32_04_osf`, `esp32_11_bien_formada_no_es_cierta` | PlatformIO |
| `X:siembra.cpp` | `esp32_12_consulta_de_reloj`, `esp32_13_siembra_de_hora`, `reloj_03_manda_la_radio` | PlatformIO |
| `X:trama.cpp` | `esp32_09_contrato_de_bytes` | PlatformIO |
| `X:transporte_app.cpp` | — *(solo censo de directorio: 6)* | PlatformIO |
| `X:vigilante.cpp` | `esp32_02_watchdog_alimentado` | PlatformIO |
| `X:contrato.h` | **11** packs | PlatformIO *(via #include)* |
| `X:despachador.h` | `esp32_12_consulta_de_reloj` | PlatformIO *(via #include)* |
| `X:enlace_stm32.h` | — *(solo censo de directorio: 5)* | PlatformIO *(via #include)* |
| `X:puente.h` | `esp32_06_no_parte_tramas` | PlatformIO *(via #include)* |
| `X:reloj_ds3231.h` | `esp32_03_ack_que_mira`, `esp32_11_bien_formada_no_es_cierta`, `esp32_12_consulta_de_reloj` | PlatformIO *(via #include)* |
| `X:siembra.h` | — *(solo censo de directorio: 5)* | PlatformIO *(via #include)* |
| `X:trama.h` | — *(solo censo de directorio: 5)* | PlatformIO *(via #include)* |
| `X:transporte_app.h` | — *(solo censo de directorio: 5)* | PlatformIO *(via #include)* |
| `X:vigilante.h` | `esp32_10_parte_de_arranque` | PlatformIO *(via #include)* |

**LO QUE ESTA TABLA DEJA VER, y no se veia antes:**

1. 🔴 **NINGUN `.cpp`/`.h` DE LOS 4 ROLES SE QUEDA SIN QUE ALGO LO ABRA — y eso no es una
   buena noticia, es CLAUDE.md 6.** Lo consigue `decisiones_01_anclas`, que censa los 95
   buscando anclas `D-x` **en los comentarios**. Un `grep` sobre un comentario no comprueba
   una linea de codigo. **La pregunta util no es «¿lo abre algo?» -siempre si-, es «¿lo abre
   algo que mire su CONTENIDO?»**, y eso son las columnas 2 y 3.
2. 🔴 **SIETE `.cpp` DE LAS DOS PUNTAS NO SE COMPILAN NI EJECUTAN EN EL PC EN NINGUN SITIO**
   `[MEDIDO 12/09]` — solo los cruza PlatformIO, que comprueba que compilan, no que hagan
   nada: **`M:main.cpp`** (el `loop()` y el watchdog) · **`M:protocolo.cpp` y
   `E:protocolo.cpp`** (el CRC y la trama de radio: **byte-identicos, o sea que el CRC no se
   ejecuta en PC en ninguna punta**) · **`E:botones.cpp`** · `M:modo_manual.cpp` ·
   `M:modo_alcance.cpp` · `M:modo_hora.cpp`.
   ⚠️ **`E:botones.cpp` es el que hay que mirar:** 4 de §2.6 ya documenta que el vigilante de
   camaras del ESCLAVO vive ahi, y §4 celebra que el del Maestro dejo de estar solo medido por
   texto cuando `Validacion_Automatico` empezo a compilar `M:botones.cpp`. **En el Esclavo ese
   arreglo no se hizo:** su vigilante sigue medido solo por packs que leen texto.
3. 🟡 **EL ESP32 ENTERO (9 `.cpp`) NO SE EJECUTA EN EL PC.** Su simulador (fila 20) lo modela
   en Python; su C++ existe y solo se compila. Sabido, y aqui contado.
4. 🔴 **`{Maestro,Esclavo}/include/pines_repetidor.h`: NADIE LOS COMPILA**
   `[MEDIDO 12/09 - grep -rn "pines_repetidor" Maestro Esclavo Repetidor]`. Ningun `.cpp` de
   su propio rol los `#include`, asi que **PlatformIO ni los parsea** y las filas 2-3 de la
   compuerta no los tocan. Solo `Repetidor/src/main.cpp` incluye su copia, y **las tres son
   byte-identicas**: no hay tres versiones divergiendo, hay dos copias muertas.
5. 🟡 **`R:main.cpp`** (todo el rol) SI lo compila PlatformIO (fila 4) y `flash_01_lastre` mira
   una propiedad real suya -que no enlace `Wire.h`/`SPI.h`-. **Lo que ningun instrumento
   ejerce es su LOGICA de repetidor:** la fila 7, `simulador_repetidor.py`, es un modelo
   Python que **no importa `fuente.py` ni nombra una sola ruta real**
   `[MEDIDO 12/09 - grep -n "Repetidor\|fw\." Simulaciones/simulador_repetidor.py]`. Coherente
   con que `Repetidor/` esta fuera de la config vigente (1.1, 2.11): el hueco es sabido.

> **LO QUE ESTA VUELTA NO MIDE, dicho para que nadie lo lea de mas.** Que un fichero este en
> la columna 2 dice que **algo se rompe si lo mueves**, no que su comportamiento este
> probado; que este en la 3 dice que **se compila y corre en el PC**, no que lo que corre
> cubra el fichero entero. «¿Se PROBO?» sigue contestandola 4, no esta tabla.

---

#### 4. LOS ARNESES QUE COMPILAN C++ REAL - Y SU PUNTO CIEGO

`[MEDIDO 12/09 - lectura de las listas `$fuentesMaestro`/`$fuentesEsclavo` de cada
`compilar*.ps1` + `grep -rn '#include ".*\.cpp"'` en los adaptadores]`

**SON SEIS, no cuatro.** Y hay un septimo que compila C++ real sin ser una fila
propia de la compuerta.

| arnes | guion | `.cpp` REALES que enlaza | PUNTO CIEGO - lo que su PASS **no** demuestra |
|---|---|---|---|
| **`Validacion_LCD`** | `compilar.ps1` | **M:** `lcd.cpp` `menu.cpp` `modos.cpp` · **E:** `lcd.cpp` `menu.cpp` `modo_degradado.cpp` | framebuffer en el PC, **no la ST7920**. Y la pantalla esta retirada del EQUIPO (D-17.bis) |
| **`Validacion_Ciclo`** | `compilar.ps1` | solo `arnes_ciclo.cpp`, que `#include "ciclo_degradado.h"` | funcion **pura**: no hay maquina de estados, no hay tiempo |
| **`Validacion_Respaldo`** | `compilar.ps1` | `arnes_respaldo.cpp` **`#include "respaldo.cpp"`** | **no ejerce el arranque**. Comprueba ademas que `respaldo.cpp/.h` son identicos entre puntas |
| **`Validacion_Automatico`** | `compilar.ps1` | **M:** `coordinador.cpp` `botones.cpp` `semaforo.cpp` `modo_automatico.cpp` `modo_inteligente.cpp` `demanda.cpp` `mando.cpp` | **solo el Maestro**. No ejerce el Degradado ni el microcorte |
| **`.../dos_puntas`** | `compilar_dos_puntas.ps1` | **M(4):** `coordinador` `semaforo` `modo_automatico` `mando` ·
  **E(7):** `semaforo` `main` `modo_degradado` `config_ciclo` `mando` `demanda` `respaldo` · **+2 POR `#include` DESDE
  LOS ADAPTADORES, no por el `.ps1`:** el `bluetooth.cpp` REAL del **Esclavo** y el `modo_ambar.cpp` REAL del
  **Maestro** *(`913c29c`, `N-142`)*. **13 `.cpp` reales** `[MEDIDO 12/09]` | **el microcorte y el respaldo del
  MAESTRO no entran** (su `M(4)` no trae `respaldo.cpp`); `Maestro/src/protocolo.cpp` tampoco. Su bloque D mide la
  reanudacion del Esclavo tras microcorte y su bloque H el cruce con la puerta SIN PIN; el porque de cada uno, en
  `roadmap.md` `N-142`/`N-162` |
| **`.../dos_puntas` Degradado** | `compilar_degradado.ps1` | **M(7):** `coordinador` `semaforo` `modo_degradado`
  `modo_ambar` `modos` `respaldo` `reloj` · **E(8):** los MISMOS SIETE del arnes hermano **MAS `reloj.cpp`** ⚠️ *(la
  fila anterior decia solo «el mismo `adaptador_esclavo.cpp`» y se leia como que aqui no entraba ningun `.cpp` del
  Esclavo: entran ocho)* · el silicio sustituido en `dos_puntas/reloj_real/` (`STM32RTC.h`, HAL,
  `rtc_periferico.cpp`). **15 `.cpp` reales** `[MEDIDO 12/09]` | 🔴 **NO ejerce el microcorte** -su propio comentario
  lo dice: *"la mide el bloque D del orquestador.cpp hermano"*-. ⚠️ **Y NO compila el `bluetooth.cpp` del Esclavo**:
  ese `#include` vive en la rama `!ARNES_RELOJ_REAL` del adaptador, y este guion es el unico que define
  `ARNES_RELOJ_REAL` `[MEDIDO 12/09 - lectura de los `#ifdef` de `adaptador_esclavo.cpp`]`. Aqui sigue doblado |
| `Simulaciones/puente_esp32` | `compilar.ps1` | **M(8):** `bluetooth` `semaforo` `coordinador` `modo_automatico` `mando` `modos` `demanda` `identidad` · **E(5):** `bluetooth` `semaforo` `demanda` `identidad` `modo_degradado` | **el ESP32 es modelo en Python**: el simulador NO carga su binario aunque su C++ exista |

**Por que `dos_puntas` necesita DOS DLL:** Maestro y Esclavo definen **los mismos
simbolos** y no enlazan juntos. Una DLL por punta en el mismo proceso, un tick pone
el mismo `millis()` en las dos, las llama, y **solo entonces** lee los doce pines.
Ese instante comun es lo que dos ejecutables separados no pueden tener - y es lo que
permite medir *"verde en las dos A LA VEZ"* sobre las puntas reales.

> ✅ **`Validacion_Automatico/botones.h` YA NO ES UN STUB** `[MEDIDO 07/09]`. Tiene
> 19 lineas y **reenvia al header real**: `#include "../Maestro/include/botones.h"`.
> Hasta el 05/09 declaraba las firmas a mano y **`botones.cpp` no se compilaba en
> ningun sitio del proyecto**: el vigilante de camaras entero estaba medido solo por
> packs que leen texto. **CLAUDE.md 8.septies sigue describiendo el mundo anterior.**

**Los sustitutos que quedan** (`-I` del directorio del arnes va PRIMERO):
`Arduino.h`, `pines.h`, `lcd.h`, `menu.h`, `bluetooth.h` en `Validacion_Automatico`;
`Arduino.h`, `pines.h` en `Validacion_LCD`; `Arduino.h` + `stm32f1xx_hal.h` en
`Validacion_Respaldo`; `Arduino.h` `pines.h` `botones.h` `lcd.h` `menu.h`
`uid_arnes.h` en `puente_esp32`. **Todo lo demas son los `.h` reales, a proposito.**

> **Un arnes que no se ha visto fallar es un adorno que da verde.** Antes de
> conectar uno se inyecta un defecto **en el `.cpp` real**, se corre, y se exige que
> **baje la cuenta y cambie el codigo de salida**. Y se restaura **desde una copia
> hecha antes de inyectar**, verificando por hash: con trabajo sin comitear,
> `git checkout --` no deshace la inyeccion, **vuelve a HEAD**.

---

#### 5. LAS VEINTE FILAS DE LA COMPUERTA

> 🔴 **`correr.py` NO ES `compuerta.py`. El banco por packs es UNA fila de veinte.**
> `correr.py` mide los packs; la compuerta mide los packs **Y** los arneses que compilan
> C++ real **Y** los simuladores **Y** los tests de la app. **`correr.py` es para iterar;
> `compuerta.py` es para autorizar**, y una cifra del banco no autoriza un commit.

`[MEDIDO 12/09 - main() de compuerta.py + evidencia/2026-09-12_compuerta.txt, HEAD 7ef5340]`.
**Una sola columna, y del acta**: la de «cifra del 07/09» se retira porque un historico de
cifras dentro de un indice es justo lo que caduca sin que nadie lo mire. El historico esta
en las actas de `evidencia/`, que es donde se puede recalcular.

| # | fila | que mide | cifra (acta 12/09) |
|---|---|---|---|
| | **-- Instrumentos --** | | |
| 1 | `guarda de rutas` | que cada fuente que los instrumentos dicen abrir **existe**. Suelo: `RUTAS_MINIMAS_ESPERADAS = 45` | 65 rutas, todas existen |
| | **-- Compilacion --** (se salta con `--rapido`) | | |
| 2 | `compila maestro` | PlatformIO sobre `Maestro/` | 🔴 **89,9 %** - 58.888/65.536 B. **6.648 B libres** |
| 3 | `compila esclavo` | PlatformIO sobre `Esclavo/` | **70,7 %** - 46.356/65.536 B |
| 4 | `compila repetidor` | PlatformIO sobre `Repetidor/` | 20,6 % - 270.497/1.310.720 B |
| 5 | `compila esp32` | PlatformIO sobre `ESP32_Expansion/` | 35,7 % - 1.123.521/3.145.728 B |
| | **-- Modelos de comportamiento --** (Python escrito a mano: prueban EL MODELO) | | |
| 6 | `simulador funcional` | `simulador_sistema_v7_6.py` | 9/9 |
| 7 | `simulador de repetidor` | `simulador_repetidor.py` — ⚠️ modelo puro: **no abre una sola ruta real** (3.7) | 10/10 |
| 8 | `simulador de app y bluetooth` | `simulador_app_bluetooth.py` - app vs firmware BT | 12/12 |
| 9 | `test funcional de la app` | `05_Funcional/App_Semaforo/test_funcional_app.py` | 65/65 |
| 10 | `test unitarios de la app` | `App_Semaforo/test_unitarios_app.js` (node) | 42/42 |
| 11 | `test unitarios TDD de la app` | `App_Semaforo/tests/test_unitarios.js` (node) | 69/69 |
| 12 | `app ejecutada en DOM` | `App_Semaforo/test_dom_execution.js` sobre jsdom | 268/268 |
| | **-- Validadores de firmware --** | | |
| 13 | `banco por packs` | `Simulaciones/banco/correr.py` - **82 packs** | 🔴 **FALLA - 1388/1396 - 81 PASS, 1 FALLA, 0 ABORTADO.** `decisiones_01_anclas` **57/65**. Rojo esperado (CLAUDE.md 1): ver cabecera |
| | **-- Firmware compilado y ejecutado en el PC --** (lo unico cuyo PASS habla del codigo) | | |
| 14 | `arnes de pantalla` | `Validacion_LCD` | 287/287 (M 145 + E 142) |
| 15 | `arnes del ciclo` | `Validacion_Ciclo` | 22/22 |
| 16 | `arnes del respaldo` | `Validacion_Respaldo` | identicos entre puntas + PING/PONG |
| 17 | `arnes del automatico` | `Validacion_Automatico` | 99/99 |
| 18 | `arnes de las dos puntas` | `Validacion_Automatico/dos_puntas` | ✅ **110/110.** ~~🔴 FALLA - 90/91, "G3 sigue en FALLA a proposito"~~ **CADUCADO: G3 se cerro y la fila esta en PASS desde `7ef5340`** `[MEDIDO 12/09 - acta]` |
| 19 | `arnes del Degradado a dos puntas` | `compilar_degradado.ps1` | 53/53 |
| 20 | `simulador del puente ESP32` | `simulador_puente_esp32.py` - **lazo entero app-ESP32-STM32** | 119/119 |

**Codigos de salida:** `0` PASS · `1` FALLA · `2` ABORTADO. Escribe el acta en `evidencia/`
con fecha y hash de HEAD. **Las cifras de los documentos se copian del acta, nunca a mano.**

> ⚠️ **NO ES IDEMPOTENTE DESPUES DE UN `--rapido`: hacen falta DOS pasadas completas.**
> `documentos_01_cifras_del_acta` lee el acta **ANTERIOR**, y `--rapido` deja un acta **sin
> las filas de `compila`**. La cura no es tocar el pack: correr la completa dos veces.

> 🔴 **UN `20/20` DICE "los modelos y arneses de PC no encuentran nada". NO DICE QUE EL
> FIRMWARE FUNCIONE EN LA TARJETA.** Los defectos que pararon el banco del 3-4/09 pasaron
> esas veinte comprobaciones sin despeinarlas. **Verde no es entregable.** Hoy son
> **19 de 20**, y el rojo que falta es el de la cabecera.

> 🔴 **N-44 SIGUE VIVO.** Los siete arneses que compilan C++ real caen a la vez si `ld` no
> abre la ruta con `ñ` de `Diego.Zuñiga`. La cura -copiar el toolchain a
> `D:\toolchain\mingw64`- **no esta en el repositorio: es una dependencia de la maquina**.
> Si desaparece, la proxima sesion mide siete comprobaciones menos y el acta lo dice en una
> linea que nadie lee.

##### 5.1 Instrumentos que EXISTEN y NO estan en la compuerta

`[MEDIDO 07/09 - grep -rl del nombre de cada fichero sobre .py/.js/.ps1]`

| fichero | lo nombra | por que no esta |
|---|---|---|
| `App_Semaforo/test_e2e_puppeteer.js` | **0 ficheros** | no afirma nada -captura pantallas-, pide `localhost:3000`, y **se traga toda excepcion saliendo con `0`** imprimiendo *"TODOS LOS CONTROLES FUERON PROBADOS CON EXITO"*. Eso es N-46 |
| `App_Semaforo/tests/test_e2e_visual.js` | (solo su directorio) | igual: capturador, no afirmador |
| `App_Semaforo/herramientas_medir_pulsables.js` | **0 ficheros** | herramienta de medida manual, no arnes |
| `App_Semaforo/herramientas_medir_consola.js` | 1 | idem |
| `App_Semaforo/herramientas_medir_desborde.js` | 3 | idem |

> **Un instrumento que no esta en la compuerta no mide nada - y no deja rastro de
> que falta.** Un ABORTADO al menos grita; un hueco no.

---

#### 6. LOS DEPOSITOS DE DOCUMENTOS - QUIEN GANA A QUIEN

```
                         EL FUENTE
                (lo que el firmware HACE de verdad)
                            |  gana a todo lo de abajo
                            v
   05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md   248 KB
                (HARDWARE MEDIDO: cobre, conectores, pines, compras)
                            |
                            v
                      DECISIONES.md                               43 KB
                (toda DECISION vigente: la tabla D-x y las abiertas A-x)
                            |
                            v
                      ESTADO.md   62 KB      <- el HOY: que esta abierto y bloqueado
                      roadmap.md 346 KB      <- el PORQUE, con los N-x debajo
                            |
                            v
        OPTIMIZACIONES.md 144 KB   reglas SFTY-x, trazabilidad regla->codigo->prueba
        INDICE_CRUZADO.md  70 KB   donde vive cada hecho; huecos
        README.md          58 KB   portada. SUS CIFRAS SE COPIAN DEL ACTA
        CERTIFICACION_SW.md 11 KB
        ARQUITECTURA.map  este fichero: un INDICE, no una fuente
```

| pregunta | quien manda |
|---|---|
| "¿que hace el firmware?" | **el fuente**. Todo lo demas es un resumen |
| "¿que hay en este pin / este conector / este componente?" | **`05_Funcional/17_...`**. Gana a `CLAUDE.md` y a este fichero |
| "¿esto esta decidido?" | **`DECISIONES.md`**. Una frase de viva voz no deroga una decision escrita |
| "¿donde esta parado el trabajo?" | **`ESTADO.md`** |
| "¿por que se hizo asi?" | **`roadmap.md`** (no hace falta leerlo entero) |
| "¿que regla SFTY cubre esto?" | **`OPTIMIZACIONES.md`** |
| "¿cuanto mide / cuantos packs hay?" | **el acta mas reciente de `evidencia/`**, nunca una copia a mano |

> 🔴 **`CLAUDE.md` TAMBIEN CADUCA, Y ES EL PEOR SITIO DONDE PUEDE PASAR.** Se carga
> en cada sesion, asi que una regla caducada ahi **se recita con autoridad y sin que
> nadie vaya a la fuente**. Ya paso tres veces en una noche -M3, la polaridad de
> `BOTON1/2`, y el NO/NC de la camara-, y las tres veces la fuente buena estaba en
> `05_Funcional/`. **Antes de contestar sobre cobre, conectores, pines o compras se
> abre la spec**, aunque `CLAUDE.md` parezca contestar.

**Documentos que un pack VIGILA** (y que por tanto no se pueden mover sin avisar):
`README.md`, `ESTADO.md`, `CERTIFICACION_SW.md`, `OPTIMIZACIONES.md`,
`04_Manuales/MANUAL_USUARIO.md`, `04_Manuales/MANUAL_HARDWARE.md`,
`05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.md`. Ver 3.6.

**`05_Funcional/` son 19 documentos numerados, cada uno con su `.md` y su `.docx`**
`[MEDIDO 07/09]`. El `.docx` **no se regenera solo**: el empaquetador solo mira que
exista.

**Y dos guias HTML de campo, que NO tienen `.docx`** `[11/09]`:

| guia | que es | estado |
|---|---|---|
| `05_Funcional/Guia_Cableado_y_Pruebas_Banco.html` | protocolo de la sesion de banco, se rellena y se devuelve en PDF | vigente |
| `05_Funcional/Camaras_Sisga_4x.html` | instalacion de las **4 camaras del Sisga (dos por poste, `D-25`)** y de la **talanquera en `J15`**; empieza por la fe de erratas de la version del 10/09 | ✏️ **version corregida del 11/09**. La del 10/09 sigue RETIRADA en `05_Funcional/historico/` (afirmaba que una camara protege la pluma y que la pluma solo sube con verde). ⚠️ **No esta en el empaquetador** (`generar_entrega_v9_0.py` la saco el 11/09 en `86b1b3e`): si tiene que viajar en el `.zip`, hay que volver a meterla alli |

> **Lo que la guia del Sisga le dice al instalador y que NO sale de ella, sino del
> firmware** `[MEDIDO 11/09 en b79d904; firmware identico en a6980e4]`: las dos camaras
> de un poste hacen LO MISMO (`CAM_J16[2]`, un solo bucle en `camaras_actualizar()`);
> ninguna frena la pluma (`escribirPines()`); la pluma sube tambien con el ambar
> intermitente (`S_FALLO`), incluido un poste recien encendido sin enlace; y la app pinta
> `CAM: OK — las dos ven` con la primera deteccion de CUALQUIERA (`camara_estado()` salta
> el pin sin flanco), por eso cada camara se comprueba con multimetro en su borne.

---

#### 7. EL HARDWARE

**MOVIDO integro a [`ARQUITECTURA_conectores.map`](ARQUITECTURA_conectores.map) el
12/09/2026** -para bajar este fichero del tope de 1.000 lineas-. Ahi siguen, con sus mismos
numeros de apartado (7.1..7.7): el fichero de KiCad no vacio, las diez cadenas de potencia,
`J14`/`J15`/`J16`/`J17` pad a pad, el censo de pines, los pines libres, lo que sigue abierto
y los siete avisos que no se pueden perder. **`05_Funcional/17_...` GANA a esa seccion en
todo lo medido con puntas** (§6 de este fichero, CLAUDE.md §3 y §12).

---

#### 8. LA CADENA DE LA APP

##### 8.1 El arbol `[MEDIDO 07/09]`

```
05_Funcional/App_Semaforo/
+-- index.html  app.js  style.css  sw.js  manifest.json   <-- LO QUE SE EDITA
+-- css/  js/
|     +-- js/bluetooth_driver.js  config.js  courier_rtc.js  depuracion.js
|            nmea_parser.js  registro_enlace.js  site_manager.js
+-- www/                                    <-- COPIA 2: de aqui construye Capacitor
|     +-- app.js index.html style.css sw.js manifest.json css/ js/
+-- android/app/src/main/assets/public/     <-- COPIA 3: lo que va en la APK
+-- android/app/build/...                   <-- ruido, ignorado
+-- capacitor.config.json                   webDir = "www"
+-- test_dom_execution.js        fila 12 de la compuerta (jsdom)
+-- test_unitarios_app.js        fila 10
+-- tests/test_unitarios.js      fila 11 (la "TDD")
+-- test_funcional_app.py        fila 9
+-- test_e2e_puppeteer.js, tests/test_e2e_visual.js   NO conectados - ver 5.1
+-- herramientas_medir_{consola,desborde,pulsables}.js   medida manual
+-- servidor_puente_simulador.py
+-- node_modules/  package.json  package-lock.json   ruido, ignorado
```

##### 8.2 Las TRES copias tienen que ser la misma

`[MEDIDO 07/09 - md5sum de raiz vs www vs android/.../public]`

```
app.js             b09dcc85  =  b09dcc85  =  b09dcc85
index.html         27f59a1a  =  27f59a1a  =  27f59a1a
style.css          ab208d35  =  ab208d35  =  ab208d35
sw.js              194be8ee  =  194be8ee  =  194be8ee
js/config.js       c88cab1c  =  c88cab1c  =  c88cab1c
js/nmea_parser.js  aded879f  =  aded879f  =  aded879f
```

**Las tres coinciden hoy.** Lo vigila `documentos_03_trama_status`, y **el fichero
concreto ya NO se escribe a mano: se CENSA el directorio** -una lista escrita a mano
solo vigila lo que alguien se acordo de anadir, y el fichero nuevo es justo el que
nadie recuerda-.

> **Por que existe:** el 27/08 `app.js` y `www/app.js` ya no eran el mismo fichero -mismo
> numero de lineas, contenido distinto-: **lo que se probaba en el navegador y lo que se
> instalaba en el telefono eran dos programas.**

##### 8.3 Como se compila la APK

```
  se edita  App_Semaforo/{index.html, app.js, js/*, style.css, sw.js}
     |
     |  npx cap copy / sync   (webDir = "www" en capacitor.config.json)
     v
  www/  ---->  android/app/src/main/assets/public/
     |
     |  Gradle (appId com.iotvial.semaforos, appName "IOT-VIAL Semaforos")
     v
  IOT_VIAL_Semaforos_<fecha>_<hash>_SIN_BANCO.apk   (en la raiz; ignorado por git)
```

**El sufijo `_SIN_BANCO` no es decorativo:** ningun artefacto de este repositorio ha
pasado una prueba de banco completa. En la raiz hay 3 APK y 1 `.zip` con ese sufijo.

##### 8.4 Los tres paneles de honestidad que la app NO tiene

**No hay panel de demo ni `runLocalTicker()`.** Un tablero que anima un cruce que no
existe le miente a quien decide sobre el trafico mirandolo. **Sin enlace la pantalla
se congela y lo declara.** Y donde un valor se sale de su cota se publica `!`, no el
numero ni `--`: `--` ya significa *"todavia no lo se"*.

---

#### 9. LO QUE ESTA PASADA NO MIDIO - y nadie debe dar por medido

**MOVIDO integro a [`ARQUITECTURA_hist.map`](ARQUITECTURA_hist.map) §9 el 12/09/2026.** Ahi
sigue la lista completa: el cobre real (gana `05_Funcional/17_...`), el comportamiento en
tarjeta (nunca en banco completo en esta rama), la asignacion opto sin correspondencia
comprobada, el `ABORTADO` en cascada del orden alfabetico sin correr de verdad, las cifras de
flash sin recompilar, los `.docx` sin comprobar contra su `.md`, y que "3.7 esta PROBADO" no
es lo mismo que "3.7 existe".

---

#### 10. RESUMEN OPERATIVO - "¿donde toco esto y que rompo?"

| voy a tocar... | mira antes |
|---|---|
| **un `.cpp` o `.h` del firmware** | **3.7**: quien lo abre, quien lo rompe si lo mueves y si alguien lo COMPILA. Si lo MUEVES, el commit lleva tambien las rutas |
| **`Maestro/src/bluetooth.cpp`** | **24 packs lo NOMBRAN y 39 lo abren.** Es el fichero mas caro del arbol |
| **`banco/modelos/*.py`** | mueves **19 packs a la vez**, y el fallo es ABORTADO en el import, no FALLA |
| **un documento de la raiz o de `04_Manuales`** | seccion 3.6: **la guarda de rutas NO te cubre** |
| **`05_Funcional/App_Semaforo/*`** | 12 packs `app_*`/`documentos_03` + 2 simuladores + 4 filas de la compuerta (3.6). Y **sincroniza las tres copias** |
| **anadir un `.js` a `index.html`** | `app_07` lo abrira solo, por el `<script src=>`, y **abortara si no existe** (3.6) |
| **la FORMA de un bloque que un pack lee por texto** | comprueba que el pack **sigue sabiendo fallar** (N-89) |
| **una constante** | ¿se relaciona con otra por una desigualdad? Esa desigualdad va en un pack que la **recalcula desde el C++**, no en un comentario |
| **retirar codigo** | ¿que bandera deja de armarse? ¿que guarda deja de poder ser falsa? (3.ter y 3.septies) |
| **algo del cobre, un conector o una compra** | **abre `05_Funcional/17_...` ANTES.** Gana a `CLAUDE.md` y a este fichero |
| **cualquier cosa, antes de comitear** | `python 01_Firmware/compuerta.py` **completo**. `correr.py` es para iterar, no para autorizar |

## 10. Sesion del 30/09-01/10/2026: banco de Marco, decisiones de campo y dos paquetes sin runbook

- **30/09, banco de `a505fa2`** (`evidencia/300920261130/`): 111 ordenes, ninguna fallida; el Degradado con testigo
  cuadro al segundo con `ciclo_degradado_fase()` (fase por hora del dia modulo 420 s). La prueba de 2 min sin
  operario dio el verde del Maestro contra el ambar del Esclavo: es la ventana aceptada, ratificada como `D-41`.
- **Decisiones del responsable con el funcional:** `D-39` (camaras vehiculo y persona; sensibilidad 50 %),
  `D-40` (arranque en ambar tras corte o watchdog; en Degradado reanuda por reloj), `D-41`, `D-42` (la espera de la
  pluma la da la alarma de 5 s de la camara; el retardo sigue en 3 s porque 6 s no compila sin subir
  `DESPEJE_SEG_MIN`).
- **Campo 30/09 12:29:** el Maestro en Degradado no reanudo tras un corte (`N-172`). El agente no encontro defecto
  propio; si uno latente: la libreria STM32duino RTC guarda su fecha en DR6/DR7 = `REG_SYNC_BAJA`/`REG_SUMA_BAJA`.
- **Paquete `6bd1e4f`** (30/09, sin runbook por decision del responsable): `D-40` solo en el Maestro (el agente
  rechazo el Esclavo con razon: reiniciado solo daria ambar contra el verde del Maestro) y `N-170`, aviso de corte
  (la trama `EVT:ARRANQUE` del puente se pintaba vacia).
- **01/10, banco de `6bd1e4f`** (`evidencia/011020260934/`): el testigo entra desde el ambar de arranque; fase al
  segundo; `ENLACE_DISPONIBLE` salio una sola vez y el funcional no se entero; ESP32 del Esclavo con ultimo arranque
  por watchdog (`OTRO_PERRO`). Para salir del Degradado basta el Poste 1 (el funcional fue a los dos).
- **Paquete `d3606be`** (01/10, sin runbook): `D-38` rojo fijo sin hora (`N-168`), `ENLACE_DISPONIBLE` cada 60 s,
  reposicion de DR6/DR7 en `respaldo_setup()`, carteles de radio y de rojo fijo en la app, sin linea vacia.
- **Lo que se concluyo mal y se corrigio en la sesion:** que la pluma «baja 3 s despues de la senal de la camara»
  (cuenta desde el rojo: `escribirPines()`); y la frase de la hoja del 30/09 «este paquete lo corrige» sobre
  `N-172`, que no lo corregia.

## 11. Sesion del 02/10/2026: decisiones pieza a pieza, amarillo de la norma, legacy fuera y NO APTO

- **Decisiones del responsable, explicadas una a una:** `D-45` (Rojo-Verde-Amarillo 3 s-Rojo, Manual de
  Senalizacion 4.4.3, tambien `FORZAR_ROJO`), `D-46` (salen `SOLICITAR_PASO` y `SET_MODO:DEGRADADO`; `J14`, SFTY-20 y
  peatonales se quedan porque las guias de campo los asignan), `D-47`, `D-48` (radio a 28 s), `D-49`; `N-175` modo
  nocturno se construira. `DECISIONES.md` paso a indice (el viejo, a `historico/`).
- **Construido:** I (`cda33df`) y G (`f9cad1f`). Compuerta convergio en 4 pasadas (18 PASS, 1 FALLA = `D-22`).
- **Arquitecto: NO APTO** (`3d16b39`): amarillo del Esclavo contra el ambar del Maestro (la inversion de `D-45` no
  llego al detector de ventanas G); `FORZAR_ROJO` en Degradado acusaba y el verde volvia; `D-47` sin prueba; la
  libreria RTC pisa DR6/DR7 en marcha. DeepSeek y Nemotron coincidieron en lo ultimo: causa probable de `N-172`.
- **Paquete a Marco sin la correccion:** `851805c` `SIN_BANCO`, con los defectos avisados.
- **Correccion interrumpida a peticion** en la rama `wip/correccion-no-apto`.
- **Lo que se concluyo mal:** que la pluma bajaba 3 s tras la senal de la camara (cuenta desde el rojo); que la
  copia de DR6/DR7 en el arranque cerraba `N-172` (la libreria escribe tambien en marcha).

## 12. Sesion del 04/10/2026: correccion del NO APTO, salida programada del Degradado y paquete e831249

- **Correccion del NO APTO** retomada de `wip/correccion-no-apto`: el parche de `app.js` y el informe con `D-50`/`D-51`
  se habian perdido; se rehicieron. Fusion a `main` en `f6ef03d`.
- **Arquitecto: APTO CON CONDICIONES.** `FORZAR_ROJO` en Degradado sin radio dejaba al Poste 1 en ambar contra el verde
  por reloj del Poste 2. El responsable decidio, por criterio y no por opcion: salida programada inversa a la entrada
  (`D-52`) y rojo total = rojo fijo en ese poste (`D-51` corregida).
- **Runbook en el dia:** SPEC -> arquitecto -> pruebas vistas en rojo -> firmware y app -> compuerta -> arquitecto
  y QA -> correcciones (B1, B2, precedencia) -> compuerta convergida en `7aeb327` -> paquete `e831249` SIN_BANCO.
- **Hook de proyecto** que recuerda al editar `DECISIONES.md` que es un indice (tercera recaida).
- **Lo que se concluyo mal:** que `D-51` (rojo total = salida del modo) cerraba el punto 2: sin radio el otro poste
  sigue alternando; mi pregunta lo daba por hecho y la decision heredo la premisa. Y J6a y el escenario P eran defectos
  de la prueba, no del firmware.
- **Tarde:** revisores de SPEC, firmware y app (lanzados tambien desde otras sesiones, que cambiaban la rama del arbol
  principal); sin defectos del firmware contra la SPEC; trece defectos de la app corregidos con prueba en rojo; el aviso
  `SYNC` entro en bucle y lo cerro Fable tras DeepSeek y Nemotron. Paquete final `22fd0d8` SIN_BANCO (app corregida,
  firmware de `e831249`); el de `e831249` a RETIRADOS.
- **Lo que se concluyo mal (tarde):** que la app podia deducir el testigo por las horas del `$EVENT`; que el Esclavo
  avisaba con alarma al rendirse (es muda); que un revisor sin informe habia perdido el trabajo (estaba en otra rama).
