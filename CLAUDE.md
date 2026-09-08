# CLAUDE.md — reglas permanentes del repositorio

Esto es **metodo**. La prueba que decide si un parrafo vive aqui: **¿sigue siendo cierto si el firmware
cambia entero?** Una **medida** de cobre va a `05_Funcional/17_...`; una **fecha** o una decision, a
`DECISIONES.md`; **como se descubrio algo**, a `roadmap.md` bajo su `N-x`; una **cifra** de la
compuerta, al acta de `evidencia/`. Aqui queda el puntero, nunca el valor.

> 🔴 **Objetivo: 150 lineas — y HOY NO SE CUMPLE.** Se dice aqui para que nadie lo lea como cumplido:
> `wc -l CLAUDE.md` da la cuenta de verdad, y lo que sobra son **las MECANICAS** —el «como» de cada
> regla—, conservadas a proposito porque un aforismo sin mecanica no protege a nadie. **Lo que no puede
> volver a entrar es cronica.** Del 28/08 al 05/09 esto paso de 518 a 1.387 lineas con **869 anadidas
> contra 36 borradas**, ningun commit retiro nunca un apartado, y la numeracion `bis/ter/quater/...`
> era el acuse de recibo. **Una regla nueva entra ABRIENDO un apartado de abajo, o no entra**: si no
> encuentra donde vivir no es metodo — es una entrada de `roadmap.md`.

## 0. Lo que no se negocia

1. **Un semaforo que falla mal mata a alguien.** Es lo unico que explica todo lo demas.
2. **Nada sube a campo sin pasar banco.** En campo corre **V8.4, `e303485` (31/07)** y **nada ha tocado
   una tarjeta desde entonces**: lo posterior esta validado en simulador, no en cobre.
3. **Un verde de la compuerta NO es un entregable.** Dice que los modelos y arneses de PC no encuentran
   nada; **no dice que el firmware funcione en la tarjeta**. Hubo un `20/20` con una regresion viva en
   banco, y los cinco defectos que pararon el banco del 3-4/09 lo pasaron sin despeinarlo.
4. **`ABORTADO` no es `PASS`** (§1) · **solo `semaforo.cpp` escribe pines de luz** (§2) · **el firmware
   nuevo esta CARGADO en la tarjeta antes de que nadie enchufe nada** (§3).

## 1. ABORTADO no es PASS

| | significa |
|---|---|
| `PASS` | corrio y el firmware cumple |
| `FALLA` | corrio y el firmware **no** cumple — hay que arreglarlo |
| `ABORTADO` | **no pudo correr** — no dice *nada* del firmware |

**Un `ABORTADO` no es una casilla pendiente: es una PUERTA ABIERTA** — mientras un instrumento esta
abortado, todo lo que vigilaba entra sin mirar, y asi entraron cuatro defectos detras del rewrite de la
app (`N-75`). **Se arregla antes de mirar nada mas.** Sus tres hermanos, que se cuentan igual de mal:
un `FALLA` que sale con codigo `0` (`N-46`); un `FALLA` **permanente** que ningun firmware posible
puede apagar —eso no es comprobacion, es nota, y va en `reportar()`—; y un instrumento que publica
`x/y` con `x != y`, **lo escriba como lo escriba y salga como salga**.

> 🔴 **Y el cuarto, que es el que tienta cuando el rojo lleva dias: UN ROJO NO SE APAGA
> ESCRIBIENDO LO QUE EL INSTRUMENTO QUIERE LEER.** `decisiones_01_anclas` acusa a una decision
> vigente que no tiene ancla en el fuente; poner el ancla **sin construirla** dice que esta
> implementada, y es exactamente el defecto que ese pack vino a cazar, cometido por el pack. Igual
> con una cita metida en un manual solo para que deje de contarla como ausente.
>
> **La regla: si un rojo se apaga CONSTRUYENDO, se construye o se deja rojo. Nunca se decora.** Y
> lo que hay que vigilar entonces no es el rojo, **es su CUENTA**: unas cuantas decisiones vigentes
> sin construir a la vez no dicen que el banco se degrade — dicen que **se esta decidiendo mas
> rapido de lo que se construye**, y eso se arregla con teclado, no con el instrumento.

## 2. Barrera de salidas

**Solo `semaforo.cpp` escribe pines de luz.** Todo pasa por su `escribirPines()` estatico —donde vive el
enclavamiento SFTY-2—, incluidos los destellos del mando, que **interceptan** las escrituras en vez de
rodearlas para no dejar colgado al coordinador. Una orden invalida **se rechaza y se reporta**, y el
ambar automatico queda reservado a los caminos que ya lo tienen (SFTY-6, watchdog): **la maquina no
decide sola** operar de un modo que nadie pidio. Lo vigila `barrera_01_pines_de_luz`.

> 🔴 **La barrera llega hasta lo que el equipo CONTESTA: un `$ACK` que no depende de lo que la llamada
> devolvio es una mentira con formato de exito.** Es el patron que mas defectos ha dado aqui (`SET_RTC`,
> `MANUAL:CAMBIAR_TURNO`, `SET_TIEMPOS`, `N-151`): la funcion tiene su `if (...) return;` bien razonado
> y **el que contesta no lo mira**, asi que el tecnico se va del poste creyendo que dejo el reloj
> puesto. **El molde de como se hace bien es `SET_TIEMPOS`: pregunta DENTRO del `if` y tiene un `$ERR`
> por cada motivo de rechazo.** Un despachador se escribe copiandolo.

⚠️ **Una regla de seguridad que ENUMERA sujetos tiene que comprobar que cada sujeto existe**, no solo
que nadie la rodea: nombraba ocho pines donde el firmware mueve seis, y el pack no lo veia porque solo
media fugas hacia fuera (`N-96`). Que pin esta vivo, muerto o libre: `ARQUITECTURA.map` y la spec.

## 3. Cobre, conectores y carga

> **Antes de contestar sobre cobre, conectores, pines o compras se abre la spec, aunque este fichero
> parezca contestar.** `DECISIONES.md` y `05_Funcional/17_...` **ganan a `CLAUDE.md`** en lo decidido y
> en lo medido: aqui se recitaron tres medidas derogadas en una sola noche, y las tres tenian la fuente
> buena alli.

**Firmware primero; el cableado despues — y el orden es ASIMETRICO.** Retirado el armador de un pin, la
placa lo deja fijado por su pull-down y **un pin en 0 V no ejecuta nada**. Al reves no: con el firmware
viejo dentro ese pin sigue siendo el boton que EJECUTA, y lo que un instalador enchufe puede pulsarlo en
un equipo que esta en la calle. **No basta con «van en el mismo commit»: un commit no protege de un
destornillador.** Se exige la **carga verificada**, no el merge.

**Tres hechos de cobre fabricado se repiten aqui porque su ausencia hiere a una persona:** **`J16` p1
lleva 12 V crudos a un conector de senal directa al micro y se TAPA en cada equipo que se monte**
(`D-4`, `N-120`) · **`J14` es una ENTRADA del micro** (3,3 V, sin opto ni diodo) y **la salida de
talanquera es `J15`**: un rele cableado a `J14` **se desconecta antes de energizar** · **`J16` p5 y p8
estan VACIOS y el codigo del mando SIGUE leyendo sus flancos** (`A-2`, `D-1`), asi que lo que se cablee
ahi compone secuencias sin que nadie lo pida.

**Carga por SWD: `mode=UR` con `-e all`, y no se cambia.** `HOTPLUG` se engancha al micro en marcha, y
con un firmware que se cuelga al arrancar el watchdog reinicia cada 4 s en mitad del borrado
(`failed to erase memory`); el delator es `NVM size: 128 KBytes (default)` en un chip de 64 KB. **Si
`UR` falla se reintenta — no se cambia el modo:** enganchar es cuestion de *timing*, y `Unable to get
core ID` no es falta de cableado. **Radios: `2.4 kbps` de Air Data Rate, `M0`/`M1` en OFF.**

## 4. La compuerta y el banco

```
python 01_Firmware/compuerta.py            # completo (compila) — 0 PASS · 1 FALLA · 2 ABORTADO
python 01_Firmware/compuerta.py --rapido   # sin compilar
python 01_Firmware/Simulaciones/banco/correr.py --pack <nombre>
```

Escribe un acta con fecha y hash de HEAD en `evidencia/`. **Las cifras de los documentos se copian del
acta, nunca se escriben a mano.**

> ⚠️ **NO es idempotente despues de un `--rapido`: hacen falta DOS pasadas completas.** La comprobacion
> de cifras lee el acta **ANTERIOR** —la nueva se escribe al final— y `--rapido` deja un acta sin las
> filas de compilacion, asi que la corrida siguiente compara contra un acta mutilada y protesta con
> razon. La cura no es tocar el pack: correr la completa **dos veces**.

> 🔴 **`correr.py` NO es `compuerta.py`: el banco es UNA fila de veinte.** `correr.py` mide los packs; la
> compuerta mide los packs **Y** los arneses que compilan C++ real **Y** los simuladores **Y** los tests
> de la app: cifras y codigos distintos. **Una cifra del banco no autoriza un commit — antes de comitear
> se corre `compuerta.py`, completo.**

**Al escribir un pack:** trae el **bloque literal** de la logica ya probada en vez de reescribirla; relee
las constantes del C++ en cada corrida y **sin valor por defecto, nunca**; y **si dos constantes se
relacionan por una desigualdad, esa desigualdad se recalcula desde el C++**, no se explica en un
comentario (`N-71`). Las cuatro primitivas: `verificar` cuenta · `propiedad` cuenta y marca `ROTA`
cuando el banco **logro romper** una regla de seguridad · `control_negativo` exige que la prueba sepa
fallar · `reportar` **no cuenta**, y es donde va el residual que ningun firmware puede aprobar. **Un
instrumento que no esta en la compuerta no mide nada y no deja rastro de que falta**: un `ABORTADO`
grita, un hueco no (`N-43`).

## 5. Los instrumentos leen el fuente por RUTA

Los validadores no incluyen el firmware: lo **parsean**, direccionando cada fichero por tuplas
—`("Maestro", "src", "mando.cpp")`—. **Mover o renombrar un fichero rompe un instrumento**, y el
movimiento y la actualizacion de rutas van en el **mismo commit**, con la compuerta verde antes y
despues. Vale igual para un `.md` que para un `.cpp`.

⚠️ **La guarda de rutas vigila ficheros que DESAPARECEN; no vigila contenido que se MUDA de fichero.**
Si sacas una funcion de `main.cpp` a un modulo nuevo, `main.cpp` sigue existiendo: la guarda no ve nada
y el validador que buscaba ese patron reporta `FALLA` **acusando al firmware de un defecto que no
tiene**. La unica red para esa deriva es **comparar el total de comprobaciones contra el de siempre**. Y
no censa documentos de la raiz: mover dos `.md` puede abortar el banco entero en silencio.

## 6. Declarar no es EJERCER

> **Un instrumento verde dice que la DECLARACION esta bien escrita. No dice que nadie la ejerza.**

Es la forma de defecto mas cara del proyecto y sale en cualquier lenguaje: los cinco que pararon el banco
del 3-4/09 vivian los cinco ahi (`N-117`, `N-118`, `N-122`, `N-124`, `N-125`). **Las tres preguntas que
lo cazan se hacen con `grep` y con el compilador, no leyendo:**

1. **¿Quien LLAMA a esto?** `grep` de la declaracion contra las llamadas. **Trinquete, no absoluto**
   —hay barreras cuya falta de llamador *es* la barrera—: falla una huerfana **nueva**, una que **gana**
   llamador y sigue en la lista, y **una que los documentos anuncien como existente** (`N-73`).
2. **¿Esta guarda puede dar las DOS respuestas?** Un `enum` de un solo valor que se compara compila a
   `movs r0,#1`, medido con `arm-none-eabi-g++ -Os -S`. **Y su simetrica:** antes de borrar el
   **armador** de una bandera se censa quien la LEE y **que pasa si nunca vale `true`** — si de ella
   cuelgan vetos, borrarlo no los deja inertes: **los deja ABIERTOS** (`D-1`).
3. **¿Este fichero se COMPILA en algun sitio?** Antes de escribir la comprobacion numero N sobre la
   FORMA de un `.cpp`, mirese si algun arnes lo enlaza: si no, el pack no mide poco — **mide otra cosa**,
   porque un pack de texto no ve un defecto del TIEMPO. Que arnes compila que y con que punto ciego:
   **`ARQUITECTURA.map`**, que se levanta midiendo (una cuenta a mano aqui caduca en dias).

> 🔴 **LA EXCEPCION ES EL INSTRUMENTO DE VERDAD.** Cuando un pack esta verde porque una excepcion lo
> justifica, lo que vigila el firmware es **esa frase, y no la comprueba nadie**. Una razon es una
> AFIRMACION SOBRE EL CODIGO: **se mide al escribirla y se vuelve a medir al heredarla** (`N-122`).
> **Una lista de excepciones con motivos sin verificar es una lista de defectos con permiso.**

> 🔴 **Un arnes que no se ha visto fallar es un adorno que da verde.** Antes de conectarlo se **inyecta
> un defecto en el `.cpp` real**, se corre, y se exige que **baje la cuenta y cambie el codigo de
> salida**; igual tras un refactor que mueva la FORMA de un bloque que un pack lee por texto (`N-89`).
> **La restauracion se hace desde una copia tomada ANTES de inyectar (`cp` al scratchpad) y se verifica
> por HASH:** con trabajo sin comitear, `git checkout -- <fichero>` **no deshace la inyeccion — vuelve a
> HEAD y se lleva tu trabajo por delante**, y `git diff HEAD` vacio no lo detecta: nunca lo estara.

## 7. La regla del instrumento

> **Un "no aparece" no es un hallazgo hasta haber descartado al buscador. Cuando el instrumento y el
> razonamiento no coinciden, manda la medida.**

El buscador ciego nunca fue el mismo: un `gcc` que `shutil.which()` no veia, y ese mismo `gcc`
compilando sin que su `ld` enlazara por una ruta con `n` con tilde (`N-44`); un `grep` de KiCad que dio
cero sobre 1.447 pistas porque el formato separa con tabulador; el mensaje de un commit, que describe la
INTENCION y no el alcance; una captura al unico ancho donde el fallo no salia.

1. **Un cero de `grep` no es «no hay»: es «mi patron no encontro».** Antes de publicar un «no existe»
   sobre una capacidad se busca por **sus dos nombres posibles** —el setter generico y la funcion
   propia— o al reves, **por quien la APAGA**. Y **el patron cuenta comentarios**: aqui los comentarios
   citan lo que explican, asi que un recuento de llamadas sale inflado si no se filtran.
2. **Cuando el sintoma trae un NUMERO, ese numero se busca en el fuente ANTES de la primera hipotesis.**
   Los «15 segundos» de un reporte eran `tiempoDespejeMs = 15000`, literal: cinco hipotesis plausibles y
   falsas costaron media sesion y el `grep` costaba diez segundos.
3. **Se cita el SIMBOLO, no el numero de linea** —un numero caduca solo, en silencio y con autoridad de
   dato, y renumerar a mano es la cura equivocada—. Solo vale **fechado a un commit**
   (`git show <hash>:fichero`) o **pegado como salida literal de un `grep` corrido antes de publicarlo**.
   El ancla barata ya existe: el firmware lleva las marcas `N-xxx` en sus comentarios, y
   `grep -rn "N-133" 01_Firmware` da las cuatro puntas de un cambio de una vez.
4. **Un informe —propio, de un agente, o una REFUTACION— no es una medida.** Se reproduce y se pega la
   salida; tachar exige el mismo rigor que afirmar. Una causa que se cae **se marca refutada, no se
   borra**: la que desaparece en silencio vuelve a proponerse, y la segunda vez nadie recuerda que se
   comprobo.
5. **Descartar por eliminacion solo vale si las opciones son exhaustivas**, y una biseccion necesita un
   extremo bueno **VERIFICADO**. El sospechoso se elige por **ficheros** —`git log --oneline
   <bueno>..HEAD -- <los del camino que falla>`— y se comparan **hashes, no tamanos**: dos binarios del
   mismo peso pueden ser el mismo fichero o no serlo.

> 🔴 **La FOTO DE CAMPO TAMBIEN ES UNA MEDIDA, y es la unica tomada sobre el aparato real: cuando chocan,
> el instrumento es el sospechoso.** Y cuando un instrumento compara contra un borde, un umbral o una
> lista, **se escribe al lado CUAL es ese borde y por que es el correcto**: los censos que fallaron lo
> hicieron todos en la misma frontera —lo que decidieron no mirar— y ninguno lo llevaba escrito. ✅ **A
> veces el banco ya corrio el control negativo sin saberlo: antes de llamar «defecto de hardware» a una
> medida, mirese que firmware estaba dentro cuando se tomo.**

## 8. Lo que YO produzco es un instrumento

> **Antes de escribir un pack, un informe, un menu de opciones o un encargo a un agente, la pregunta no
> es «¿esta bien hecho?» sino «¿esto acerca una tarjeta cargada, o la sustituye?»**

Dos auditorias externas lo llamaron *industria de sustitucion*, y la segunda lo dijo de la respuesta a la
primera: *«se arreglo todo lo que la auditoria midio y nada de lo que dijo»*. **La medida se recalcula,
no se recita:** lineas de `{Maestro,Esclavo,Repetidor}/{src,include}` frente a `Simulaciones +
Validacion_* + compuerta.py`.

1. **Un pack que certifica otra vez lo ya certificado sustituye.** La excepcion legitima es el que
   **desbloquea** algo parado o mide una **propiedad de vida que nadie ejercia**: eso es firmware por
   otro nombre. No lo decide el fichero que toca, sino si **contesta una pregunta abierta**.
2. **Un menu de opciones sobre una causa sin medir tiene mas autoridad que un dato**, porque parece que
   ya se investigo. **Si hay que preguntar sin haber medido, se dice en la pregunta**, y cuando el banco
   tumba una decision del responsable se le devuelve **con la medida**, no se ejecuta igual (`N-142`).
   **«El responsable lo decidio» NO es cobertura:** una decision tomada sobre un informe malo hereda el
   error **y ademas lo blinda**, porque desde ahi ya nadie mira la causa.
3. **Un encargo que ejecuta una frase en vez de la spec.** Lo dicho de viva voz describe una INTENCION;
   lo escrito describe una DECISION TOMADA, con su motivo: **cuando chocan se PREGUNTA** —cuesta un
   `grep` a `DECISIONES.md`—, y con mas razon si el cambio **RETIRA una barrera**: un alcance que crece
   se corrige despues; una proteccion amputada no se nota hasta que alguien esta en la calzada.

> 🔴 **Delegar amplifica el error, y por eso el trabajo delegado se revisa por el DIFF, no por su
> informe.** Si un cambio toca **el firmware y su modelo a la vez**, mirese si el modelo *replica* el
> arreglo o si *relaja* la comprobacion: solo lo primero vale, y una cifra verde despues de tocar las dos
> puntas no demuestra nada por si sola. **Una variable que contesta a dos preguntas distintas no puede
> contestar bien a ninguna: son dos banderas.** Y el sintoma de que se cruzo la linea es **entregar
> documentos cuando lo pedido era un entregable**: un parte con cifras verdes puede ser cierto en cada
> linea y falso en conjunto.

> 🔴 **Y EL DIFF SE MIRA SOBRE TODO CUANDO EL NUMERO SALE VERDE, que es cuando no apetece.** §1 ya
> prohibe decorar un rojo y aun asi paso, porque **el que decora es el que informa**: quien apago la
> casilla escribe luego el parte que dice que la construyo, y nadie audita un `20/20`. **El verde no
> es el permiso para no mirar: es la senal de que toca mirar.** Lo que cazo la ultima tanda fue un
> `grep` por afirmacion —siete no sobrevivieron: una funcion que no existia, unos literales
> inventados, unos pines que eran otra cosa— y **un `sha256sum` a un binario que se declaraba
> recompilado y era el anterior renombrado**. Ninguna de las dos cosas la ve la compuerta.

## 9. Al arreglar un defecto, busca las pruebas que lo CELEBRABAN

**Un banco maduro contiene pruebas que EXIGEN el comportamiento defectuoso** (`N-49`): se escribieron
cuando el defecto se creia inevitable. Reescribirlas en bloque hasta que pasen es **ajustar el
instrumento hasta que de verde**. Van una por una, y **primero se cuenta cuantas propiedades afirma cada
una**, porque casi ninguna afirma solo una (`N-83`). Cuatro destinos: **se REPARTE** —el habitual: la
mitad del defecto se invierte donde estaba y la que sigue valiendo **se MUDA con su bloque literal** al
escenario donde si se cumple— · **se INVIERTE** para exigir lo nuevo · **se CONSERVA** si media otra
cosa · **se BORRA** si solo documentaba el defecto.

**El escenario nuevo no es relleno: es el control que le falta a toda inversion** —una guarda que no
dejara pasar **nada** haria pasar las lineas invertidas igual de bien que la correcta—. Y 🔴 **una
inversion que solo mira el RESULTADO aprueba un firmware con las barreras en el ORDEN equivocado**:
medido inyectando el defecto, la linea «no sale ninguna trama» **no cayo** porque otra barrera mas abajo
frenaba el envio, y lo unico que cazo la regresion fueron las lineas que miran el orden. **El mejor
termometro del arreglo son los fallos que desaparecen solos**; y una linea nueva que no puede fallar sin
que falle la de arriba no es una comprobacion: es adorno.

## 10. Flash, RAM y cotas de buffer

64 KB por micro, y **el Maestro esta por encima del 85 %: ya no queda margen comodo.** La cifra exacta
sale de la ultima acta (`ls -t evidencia/*_compuerta.txt | head -1`) y **no se copia aqui**: un umbral no
caduca, una cifra si — y la que habia invitaba a meter algo que ya no entraba.

- **Antes de sacrificar una funcion porque «no cabe», MIDE DE QUE ESTA HECHO ese porcentaje** (`N-70`):
  **por FICHERO OBJETO leyendo `firmware.map`, NO por nombre de simbolo** —los simbolos de cualquier
  libreria C++ tambien empiezan por `_Z`—; el `.map` trae la seccion que dice **quien arrastra a quien**;
  y **un delta exige medir los DOS extremos**.
- **Un camino muerto que no cuesta flash puede seguir costando RAM** (`N-86`): el enlazador descarta
  funciones, pero **no un objeto global** —su constructor vive en `.init_array` y su `.bss` es
  permanente—. **La flash se mide con la compuerta; la RAM, con `nm` sobre el `.elf`.**
- ⚠️ **El acta puede publicar el binario ANTERIOR** cuando PlatformIO sirve un incremental viejo, y eso
  pasa siempre que se intercambian fuentes o dos agentes tocan el arbol a la vez. **La cifra de flash se
  confirma con una segunda pasada; si no coinciden, manda la segunda.**
- **La cota de un buffer es el BUFFER de cada campo, no su tipo ni su rango** (`N-154`): es lo unico que
  `snprintf` garantiza sin fiarse de otro fichero. **Se acota donde se produce; no se agrandan
  buffers**, y **las cotas se DERIVAN de la constante que manda**. Un valor fuera de cota se publica como
  `!` —nunca `--`, que ya significa *«todavia no lo se»*—. Una trama truncada sale bien formada hasta la
  mitad, no casa el CRC, y el sintoma es «el equipo se callo», que manda a mirar el cable.
- **Nada de clases con metodos virtuales:** las vtables cuestan flash que no hay. Se separa con un `.cpp`
  por concepto, `static` para lo privado y header corto.

## 11. Trabajo en paralelo, y el INDICE antes del commit

**Un `git add -A` barre lo que otro agente tiene en vuelo, y la historia queda mintiendo:** el contenido
puede quedar correcto y la compuerta verde, pero un `revert` de ese commit no deshace nada.

- **Con dos agentes a la vez: o cada uno en su `git worktree`, o `git add` de rutas explicitas. Nunca
  `-A`.** El arbol compartido no avisa: mezcla.
- 🔴 **Segunda via al mismo dano con un solo agente: `git add <valido> <ignorado>` devuelve codigo
  distinto de cero PERO DEJA LOS VALIDOS YA PREPARADOS.** Con `git add ... && git commit` el `&&` corta,
  el commit bueno no corre, y **se los lleva el commit SIGUIENTE**. **La regla: se comprueba el INDICE
  antes de comitear —`git diff --cached --name-only`—, no el codigo de salida del `add`.**
- 🔴 **Y una TERCERA, que es la contraria y por eso engana: `git add <valido> <ruta_que_NO_EXISTE>`
  no prepara NADA** —ni las rutas validas de esa misma orden—. Paso el 07/09 con un fichero que se
  acababa de mover: el commit salio con **un mensaje que describia cambios que no llevaba dentro**.
  **Imprimir el indice no basta: hay que LEERLO.** Cuenta las lineas que esperas.
- 🔴 **Al editar un fichero con un script, `open(ruta, "w")` TRUNCA ANTES de escribir.** Si el
  `.write()` falla despues —un `UnicodeEncodeError` por un emoji, por ejemplo— **el fichero queda
  vacio y el error parece de lectura**. Paso el 07/09 con `DECISIONES.md`, la tabla vinculante; se
  recupero **porque estaba comiteada**. Se escribe a un temporal y se renombra, o se usa la
  herramienta de edicion, que no tiene esta forma de fallo. **Comitear antes de un script masivo no
  es orden: es la red.**
- **No se reescribe la historia publicada para arreglarlo:** con la rama en dos remotos y otro agente
  encima, un `push --force` dana mas de lo que repara. Se anota donde vive el cambio y se sigue.

**EL ORDEN DE LANZAMIENTO, que es lo que impide el bucle de hacer / deshacer / rehacer:**

1. **Antes de lanzar se abre `DECISIONES.md`.** Si el encargo contradice una fila, **eso no es una
   orden: es una pregunta**. Casi siempre la frase nueva es un resumen impreciso de algo mas pequeno.
2. **Los que MIDEN van antes que los que ESCRIBEN.** Auditoria de solo lectura -> veredicto en el
   scratchpad -> ejecucion **con ese veredicto delante**. Un veredicto malo ejecutado a fondo es peor
   que uno discutido: delegar **amplifica** el error.
3. **Un agente = un conjunto de ficheros DISJUNTO, nombrado explicitamente**, prohibidos incluidos. El
   limite del paralelo es el fichero, no la capacidad.
4. **El encargo dice contra que `D-x` trabaja**, y lleva esta instruccion: **si el codigo y su `D-x` no
   coinciden, para y reporta — NO elijas.** Ahi es donde nace el bucle: uno lo arregla en un sentido y
   el siguiente en el contrario.
5. **Ningun agente comitea.** Comitea el orquestador, con rutas explicitas y **leyendo** el indice.
6. **A cada agente se le pide que DUDE del encargo.** Es la frase que mas rinde: en una sola sesion los
   agentes refutaron siete datos que iban dentro de sus propios briefs.
7. **La compuerta la corre el orquestador con el arbol QUIETO.** Una cifra medida mientras otros
   escriben no vale, aunque el numero salga bien formado.

## 12. Donde esta cada cosa

| | |
|---|---|
| `DECISIONES.md` | 🔴 **lo decidido: `D-x` vigentes y `A-x` abiertos. GANA a este fichero, y se lee ANTES de lanzar un agente o de ejecutar un cambio de alcance** |
| `05_Funcional/17_Arquitectura...md` | 🔴 **las medidas de cobre, con fecha, instrumento y firmware que habia dentro. GANA a este fichero en todo lo que sea hardware medido** |
| `ARQUITECTURA.map` | que ficheros abre cada instrumento, que mide cada fila de la compuerta, que hay en cada conector |
| `roadmap.md` · `roadmap_hist.md` | el **porque**: cada `N-x` con su cronica. Se busca **por `N-x`**, no por fichero: el corte entre los dos se mueve |
| `ESTADO.md` | donde esta parado el trabajo **hoy** · `evidencia/`, las actas con fecha y hash: **la fuente de toda cifra** |
| `OPTIMIZACIONES.md` | las reglas `SFTY-x` y la trazabilidad regla -> codigo -> prueba |
| `01_Firmware/compuerta.py` | **la unica forma correcta de verificar** · `Simulaciones/banco/`, packs y modelos · `Validacion_*/`, los arneses que compilan C++ real |
| `04_Manuales/`, `05_Funcional/` | manuales y protocolos para el tecnico y el auditor |

## 13. Convenciones

- **Comentarios y mensajes de commit en espanol, en ASCII sin acentos** (la consola de Windows viene en
  cp1252 y los validadores parsean el fuente).
- Los comentarios explican **por que**, no que. **Y si un comentario delibera** —*«Wait, in this
  state...»*, *«let's just force...»*— quien lo escribio no lo tenia claro y lo dejo asi: dentro de una
  regla de seguridad eso no es una nota, es una alarma. La duda se resuelve o se anota en `roadmap.md`.
- **Un commit = un cambio con sentido propio = un `git revert` limpio**, y cada `N-x` se cierra con la
  evidencia que lo demuestra, no con una afirmacion.
- Si un pack ejerce una regla `SFTY-x` se marca `# EJERCE SFTY-x: <que>` en su cabecera, y la tabla de
  trazabilidad se levanta buscando esa etiqueta. **Solo se etiqueta lo que el pack comprueba de verdad:**
  una regla cubierta por una prueba que no la ejerce es peor que una fila vacia — la vacia no miente.
- 🔴 **Un binario que sale de aqui se nombra `<producto>_<fecha>_<hash>_SIN_BANCO`, y el sufijo NO se
  quita al renombrar: lo quita quien lo haya probado en un equipo.** Es la unica marca que viaja
  PEGADA al fichero —los `.apk` estan en `.gitignore`, asi que git no vigila esto y hay que mirarlo en
  el disco—, y **su ausencia se lee como permiso**. Un binario nuevo se acredita con su `sha256sum`
  contra el anterior, **nunca con su tamano**: dos compilaciones de la misma app pesan igual.
