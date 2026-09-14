# CLAUDE.md — reglas permanentes del repositorio

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

## 0. Lo que no se negocia

1. **Un semaforo que falla mal mata a alguien.** Es lo unico que explica todo lo demas.
2. **Nada sube a campo sin pasar banco.** Que firmware hay en cada equipo lo dice **`ESTADO.md`**, no este fichero
   — aqui esa frase ya caduco una vez. **Una foto de campo sin el hash de lo que habia dentro no se lee** (§7).
3. **Un verde de la compuerta NO es un entregable:** dice que los modelos y arneses de PC no encuentran nada, **no
   que el firmware funcione en la tarjeta**. Ha habido un `20/20` con una regresion viva en banco.
4. **`ABORTADO` no es `PASS`** (§1) · **solo `semaforo.cpp` escribe pines de luz** (§2) · **el firmware nuevo esta
   CARGADO en la tarjeta antes de que nadie enchufe nada** (§3).

## 1. ABORTADO no es PASS

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

## 2. Barrera de salidas

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

## 3. Cobre, conectores y carga

> **Antes de contestar sobre cobre, conectores, pines o compras se abre la spec, aunque este fichero parezca
> contestar.** `DECISIONES.md` y `05_Funcional/17_...` **ganan a `CLAUDE.md`** en lo decidido y en lo medido.

**Firmware primero; el cableado despues — y el orden es ASIMETRICO.** Retirado el armador de un pin, la placa lo
deja fijado por su pull-down y **un pin en 0 V no ejecuta nada**. Al reves no: con el firmware viejo dentro, ese pin
sigue siendo el boton que EJECUTA y lo que un instalador enchufe puede pulsarlo en un equipo que esta en la calle.
**Un commit no protege de un destornillador: se exige la CARGA VERIFICADA, no el merge.**

**Tres hechos de cobre fabricado, aqui porque su ausencia hiere a una persona:** **`J16` p1 lleva 12 V crudos a un
conector de senal directa al micro y se TAPA en cada equipo que se monte** (`D-4`, `N-120`) · **`J14` es una ENTRADA
del micro** (3,3 V, sin opto ni diodo) y **la salida de talanquera es `J15`**: un rele cableado a `J14` **se
desconecta antes de energizar** · **`J16` p5 y p8 estan VACIOS y el firmware SIGUE leyendo sus flancos POR DOS
CAMINOS, no uno** —medido el 12/09: el mismo `flanco[]` de `botones.cpp` alimenta `mando_registrarPulso()`
**y** `botonArriba()`/`botonAbajo()`, con llamadores vivos en `menu.cpp` y `modo_hora.cpp` de las dos puntas—,
asi que **quitar el mando NO cierra ese pin**: hay que censar los dos
(`A-2`, `D-1`): lo que se cablee ahi compone secuencias sin que nadie lo pida.

**Carga por SWD: `mode=UR` con `-e all`, y no se cambia.** `HOTPLUG` se engancha al micro en marcha, y con un
firmware que se cuelga al arrancar el watchdog reinicia cada 4 s en mitad del borrado (`failed to erase memory`); el
delator es `NVM size: 128 KBytes (default)` en un chip de 64 KB. **Si `UR` falla se reintenta — no se cambia el
modo:** enganchar es cuestion de *timing*, y `Unable to get core ID` no es falta de cableado. **Radios: `2.4 kbps`
de Air Data Rate, `M0`/`M1` en OFF.**

## 4. La compuerta y el banco

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

## 5. Los instrumentos leen el fuente por RUTA

Los validadores no incluyen el firmware: lo **parsean**, direccionando cada fichero por tuplas —`("Maestro", "src",
"mando.cpp")`—. **Mover o renombrar un fichero rompe un instrumento**, y el movimiento y la actualizacion de rutas
van en el **mismo commit**, con la compuerta verde antes y despues. Vale igual para un `.md` que para un `.cpp`.

⚠️ **La guarda vigila ficheros que DESAPARECEN; no vigila contenido que se MUDA de fichero.** Sacada una funcion de
`main.cpp` a un modulo nuevo, `main.cpp` sigue existiendo: la guarda no ve nada y el validador que buscaba ese
patron reporta `FALLA` **acusando al firmware de un defecto que no tiene**. La unica red es **comparar el total de
comprobaciones contra el de siempre**. Y no censa la raiz: mover dos `.md` puede abortar el banco en silencio.

## 6. Declarar no es EJERCER

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

## 7. La regla del instrumento

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

## 8. Lo que YO produzco es un instrumento

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

## 9. Al arreglar un defecto, busca las pruebas que lo CELEBRABAN

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

## 10. Flash, RAM y cotas de buffer

64 KB por micro, y **el Maestro esta por encima del 85 %: ya no queda margen comodo.** La cifra exacta sale de la
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

## 11. Trabajo en paralelo, y el INDICE antes del commit

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

## 12. Donde esta cada cosa

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

## 13. Convenciones

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

## 14. Lo que nadie recalcula, envejece — y eso incluye las LISTAS

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

## 15. El orden de prioridad del trabajo

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


## 16. Limpiar es parte del trabajo, y se MIDE

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

1. 🔴 **NADA ENTRA SIN QUE ALGO SALGA.** Un pack, un apartado o un documento nuevo **nombra en su commit
   que retira**. Si no retira nada, el commit lo dice y explica por que — «lo escribi y no borre nada» es
   una decision, no un descuido, y tiene que verse.
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
