# CLAUDE.md — reglas permanentes del repositorio

Este archivo se carga solo en cada sesión. Contiene lo que **no cambia entre sesiones**.
El estado de hoy —qué está abierto, qué está bloqueado, cuál es el siguiente paso— vive en
[`ESTADO.md`](ESTADO.md), y se lee después de este. El histórico completo está en
[`roadmap.md`](roadmap.md), que **no hace falta leer entero** para trabajar.

---

## 1. Qué corre en la calle

**Campo = V8.4, commit `e303485`.** Todo lo de V8.5 a V8.7 —reloj, sincronización horaria,
Modo Degradado, mando de relés, interfaz del Esclavo— está en `feat/n15-reloj-pantalla-hora`,
validado en simulador y **sin prueba de banco completa**.

> **Nada sube a campo sin pasar banco.** No es una preferencia de proceso: un semáforo que
> falla mal mata a alguien.

## 2. ABORTADO no es PASS

Los tres estados de una comprobación son distintos y no se mezclan nunca:

| | significa |
|---|---|
| `PASS` | corrió y el firmware cumple |
| `FALLA` | corrió y el firmware **no** cumple — hay que arreglarlo |
| `ABORTADO` | **no pudo correr** — no dice *nada* del firmware |

Tratar un `ABORTADO` como aprobado es como el Maestro estuvo días sin cobertura de validación
sin que nadie se enterara. `01_Firmware/compuerta.py` existe para que eso no vuelva a pasar.

## 3. La compuerta, antes y después

```
python 01_Firmware/compuerta.py            # completo (compila)
python 01_Firmware/compuerta.py --rapido   # sin compilar
```

Un solo código de salida: `0` PASS · `1` FALLA · `2` ABORTADO. Escribe un acta con fecha y
hash de HEAD en `evidencia/`. **Las cifras del README se copian del acta, nunca se escriben a
mano.**

> ⚠️ **La compuerta NO es idempotente después de un `--rapido`: hacen falta DOS pasadas completas.**
> `documentos_01_cifras_del_acta` lee el acta **ANTERIOR** —la nueva se escribe al final, en
> `escribir_acta()`, después de que el banco ya corrió—, y `--rapido` deja un acta **sin las tres
> filas `compila maestro / esclavo / repetidor`**. La siguiente corrida, aunque sea completa,
> compara la tabla del README contra esa acta mutilada, ve tres filas que el acta no mide y
> protesta **con razón**.
>
> No es un defecto del pack: es el pack impidiendo publicar una cifra que no salga de la última
> corrida. La cura no es tocarlo — es correr la completa **dos veces**, y copiar del acta que sí
> las trae.

> 🟢 **Desde el 05/08 la compuerta sale con `0` — y eso la vuelve más peligrosa, no menos.**
> Mientras salía con `1`, nadie la confundía con un permiso. Un `0` sí se confunde. Lo que ese
> `0` dice es exactamente esto: *los modelos y los arneses de PC no encuentran nada*. **No dice
> que el firmware funcione en la tarjeta**, y hoy mismo la prueba está delante: con la compuerta
> en verde hay una regresión abierta en banco donde el Modo Automático no mueve las luces.
> **Verde no es entregable.**

> ✅ **N-46 cerrado el 05/08 — y la lección se queda escrita.** Durante meses
> `validador_maestro.py` imprimió `FALLA` y salió con `0`, así que la suite se pintaba `[OK]`.
> Era *"ABORTADO no es PASS"* invertido —**`FALLA` contado como `PASS`**— y uno de aquellos
> fallos era vial. Se cerró **retirando los tres monolitos**: los packs sí los cuentan.
>
> 🔴 **Y hay una tercera cara del mismo error, que costó ver hasta el final: un `FALLA`
> PERMANENTE tampoco es un aviso.** El alias de ±60 s de `CMD_DELTA` se dejaba fallando *a
> propósito* para que el límite no se olvidara. Pero ese fallo no lo puede apagar ningún
> firmware —un byte de segundos no distingue 0 de 60—, así que la compuerta **nunca podía salir
> en verde**, y un código de salida que jamás cambia enseña a ignorarlo.
>
> **La regla que queda: si una comprobación no la puede aprobar ningún firmware posible, no es
> una comprobación — es una nota, y va en `reportar()`.** El alias se invirtió: ya no exige lo
> imposible, exige que el agujero sea **exactamente** el que el protocolo obliga y ni un caso
> más, con su `control_negativo` demostrando que sabe distinguirlo.

> 🔴 **Y una CUARTA cara, del 27/08, que estuvo abierta desde siempre: el detector de N-46 solo
> sabía leer un marcador.** Busca la marca literal `[FALLA]`, que es la que imprimen los packs...
> y **ninguno de los dos simuladores más viejos**. Aquellos escriben `✘ FAIL` y cierran con
> `VEREDICTO FINAL: 17/20 PASS — HAY FALLOS PENDIENTES`, saliendo con código `0`. El simulador
> funcional podía caer de `20/20` a `17/20` y el acta seguía diciendo `[OK]`, con la cuenta mala
> al lado —que nadie lee cuando el semáforo de la izquierda está verde—.
>
> **La regla que queda no depende del marcador, que es justo lo que fallaba: si un instrumento
> publica una cuenta `x/y`, se exige `x == y`.** Un instrumento que anuncia 17 de 20 está diciendo
> que tres comprobaciones no cumplen, lo escriba como lo escriba y salga con el código que salga.

> **Un instrumento que no está en la compuerta no mide nada — y no deja rastro de que falta.**
> `Validacion_Respaldo` compila el `calcularSuma()` real, lleva días roto, y el acta no lo echa de
> menos: son 12 suites y ninguna es esa (N-43). Un `ABORTADO` al menos grita; **un hueco no**.
> Al escribir un arnés, conectarlo a `compuerta.py` es parte del trabajo, no un paso posterior.

## 3.bis El banco por packs

```
python 01_Firmware/Simulaciones/banco/correr.py --listar
python 01_Firmware/Simulaciones/banco/correr.py --pack esclavo_03
```

**La migración terminó el 05/08: no quedan validadores monolíticos.** El banco son **38 packs**
—`405/405` comprobaciones en el acta del 28/08—, un fichero corto por propiedad, que se corre solo
en un segundo. El porqué estaba medido:
**8.898 líneas de instrumento para 8.895 de firmware**, y los instrumentos no son pruebas — son
una *segunda copia del firmware escrita a mano* que alguien sincroniza. Eso falló tres veces
(N-36, N-39, y la propia compuerta).

**Cómo se retiró cada monolito, que es la parte reutilizable:** los packs tienen que sumar
**exactamente** sus comprobaciones, y hay que comparar el **texto literal** de cada una, no solo
el recuento. Costura `41 = 41`, Maestro `64/67 = 64/67`, Esclavo `31 = 31`, cero huérfanas en
ninguna dirección. Y al retirarlos **cayó la guarda de rutas** —censaba las tuplas de los
monolitos y se quedó en 4—: abortó en vez de aprobar, que es su trabajo. Hoy censa
`banco/packs` y `banco/modelos`.

**Reglas al escribir un pack:**

- **Trae el bloque literal.** Reescribir lógica ya probada para renombrar llamadas es como se
  cuelan los errores en un cambio que no debe cambiar comportamiento.
- Las constantes se releen del C++ en cada corrida. **Sin valor por defecto, nunca**: un
  banco que no puede fallar no demuestra nada.

> ⚠️ **Y "sin valor por defecto" incluye los restos del algoritmo anterior (N-51).** `PESOS_SUMA`
> se quedó fijado a `1` para todos los registros cuando `calcularSuma()` pasó a un hash de
> Horner, con el comentario *"compatibilidad"*. La resta de pesos daba siempre 0, así que la
> prueba 2.7 marcaba los **C(5,2)=10** pares posibles **sin llamar nunca al checksum real** — y
> encima el mensaje decía *"con los pesos leídos del C++"*. Medidos de verdad eran 8.
>
> 🔴 **Y su hermana era peor, porque salía en VERDE.** La prueba 2.8 hacía `break` sobre esa
> misma condición siempre cierta y **no evaluaba ni un solo candidato**: meses de `PASS` sin
> medir nada. Al arreglarla apareció un camino **explotable**: permutar `FLAGS` y `SYNC_BAJA`
> deja la suma intacta y produce `FLAGS` con `CICLO+SYNC+DEGRADADO` encendidos, que un arranque
> tras corte leería como autorización vigente para reanudar el Degradado.
>
> **Dos señales de esta clase de prueba muerta:** un número que coincide exactamente con *"todas
> las combinaciones posibles"*, y un `PASS` de algo que nadie ha visto fallar nunca.

> 🔴 **Una constante puede no ser un número suelto, sino el TECHO de otras (N-71).** El umbral de
> silencio de SFTY-6 estaba en 12 s mientras el ciclo necesita hasta **20,5 s** para agotar sus
> cinco reintentos: **los reintentos 4 y 5 no podían ejecutarse jamás**, y nada lo delataba porque
> el equipo hacía algo razonable —irse a ámbar—. La relación entre los tres números vivía **solo
> en prosa, dentro de un comentario**, y los comentarios no fallan cuando alguien cambia un
> número: se quedan describiendo un equipo que ya no existe, **con la autoridad de una cuenta
> hecha**. Si dos constantes se relacionan por una desigualdad, esa desigualdad va en un pack que
> la recalcula desde el C++, no en una nota.

> 🔴 **Una función que nadie llama es la versión silenciosa de la prueba muerta (N-73).** La
> *«Caja Negra de Alarmas»* que **cuatro manuales** describían estaba declarada, definida y
> documentada con ejemplo en las dos puntas —y **sin un solo llamador**—. Es `CAM_UMBRAL_PIN` otra
> vez: un `pinMode()` sin `digitalRead()`, pero con documentación encima. Se pagó cuando un fallo
> de campo no se pudo diagnosticar porque no había registro que mirar.
>
> **El censo es `grep` de la declaración contra las llamadas, no lectura.** Y la propiedad que se
> vigila es un **trinquete, no un absoluto**: exigir «cero huérfanas» sería falso —hay obra a
> medias declarada honestamente como *«sin construir»*, y hay barreras cuya ausencia de llamador
> *es* la barrera—. Lo que falla es una **nueva**, una que **gana** llamador y sigue en la lista, y
> sobre todo **una que los documentos anuncien como función existente**.

> 🔴 **Refactorizar puede APAGAR un instrumento sin romper un solo test (N-89, 28/08).** Al escribir
> los seis comandos nuevos se probó un compositor —`responderAck(cmd, resultado)` /
> `responderErr(cmd, motivo)`— que armaba la trama en un buffer y ahorraba **636 B**. Se retiró, y
> el porqué es la regla.
>
> `app_03_sin_ok_mudo` busca los literales `"$ACK` / `"$ERR` **dentro del bloque de cada rama**. Con
> el compositor, esos literales se mudan a otro fichero y **ninguna rama los tiene ya**: todas
> pasaban por *"no promete nada"* —incluida la de calibración, y **los dos controles negativos**,
> que usan bloques sintéticos propios—. El pack habría seguido en **verde midiendo nada**: es la
> prueba muerta de N-51, esta vez introducida por un cambio que ningún test delata, porque el
> firmware seguía siendo correcto.
>
> **La regla: al tocar la FORMA de un bloque que un pack lee por texto, hay que comprobar que el
> pack sigue sabiendo fallar** —§8.bis, aplicado al refactor y no solo al arnés nuevo—. Y 636 B
> contra un instrumento que deja de medir no es un intercambio: se rechaza midiendo las dos cosas,
> no dudándolo.

**Las cuatro primitivas del contador, que no significan lo mismo:**

| | cuenta | cuándo |
|---|---|---|
| `verificar` | sí | comprobación normal |
| `propiedad` | sí, marca `ROTA` | propiedad de seguridad que el banco **logró romper** — es fallo del firmware, no del banco |
| `control_negativo` | sí | exige que la prueba **sepa fallar** |
| `reportar` | **no** | hallazgo que acompaña a una comprobación que ya cuenta — **y el sitio donde va un residual que ningún firmware puede aprobar**, como el alias de `CMD_DELTA` |

## 4. La regla del instrumento

> **Un "no aparece" no es un hallazgo hasta haber descartado al buscador.**

Este proyecto la pagó dos veces: `gcc` llevaba semanas instalado y `shutil.which()` no lo veía
(el `ABORTADO` era falso), y un recuento de anchos a mano dio por buenos dos textos que se
salían de la pantalla. Antes de reportar que algo falta, **verifica que tu búsqueda sabía
encontrarlo**.

Su corolario: cuando el instrumento y el razonamiento no coinciden, **manda la medida**.

> 🔴 **Y el buscador puede estar ciego por el FORMATO del fichero, no por faltar la herramienta
> (28/08).** `MAPEO_TARJETA_KICAD.md` afirmaba que el `.kicad_pcb` estaba **VACÍO**, y sobre esa
> frase se sostenía todo el *"solo hay medida en el esquemático, no en el cobre"*. Era falsa: el
> fichero pesa **2.158.421 B** y trae **185 huellas, 1.447 pistas, 89 vías, 485 pads y 117 redes**.
>
> ```
> grep -c '(segment ' Controladora_Semaforos.kicad_pcb        ->    0     <-- FALSO
> grep -oE '\(segment\b' Controladora_Semaforos.kicad_pcb | wc -l -> 1447 <-- REAL
> ```
>
> KiCad separa los tokens con **tabulador y salto de línea**, así que buscar la pista con un espacio
> detrás da cero — y **un cero se lee como «no hay»**. Es §4 aplicada a un formato, no a una
> herramienta que falta: `grep` estaba, respondía, y aun así no sabía encontrar. Antes de publicar
> un «está vacío», mide el fichero por otro camino (`wc -c`, un segundo patrón) y compara.

> **Y su segunda cara, que costó un commit el 03/08: lo que TÚ reportas también es un
> instrumento.** Se escribió en el roadmap que el arnés de respaldo fallaba por *"el canal de
> piping de PowerShell 5.1"*, dado por *"reproducido dos veces"*. Al ir a arreglarlo: PS 5.1 manda
> `PING\r\n` sin BOM, `strncmp` casa, y el binario responde `PONG`. **La causa era plausible y
> falsa**, y llegó al repositorio con la palabra *"medido"* encima.
>
> Un informe —propio o de un agente delegado— **no es una medida**. Antes de escribir una causa en
> `roadmap.md` o en `ESTADO.md`, se reproduce el fallo y se pega la salida. Y una causa que se cae
> **se marca refutada, no se borra**: la que desaparece en silencio vuelve a proponerse, y la
> segunda vez ya nadie recuerda que se comprobó.
>
> **Y su tercera cara, del 04/08: una REFUTACIÓN también es un instrumento.** La nota que tumbó la
> causa del arnés de respaldo decía *"medido byte a byte: PS 5.1 manda `PING\r\n` sin BOM"*. Es
> falso —`od` sobre la tubería real da `ef bb bf 50 49 4e 47 0d 0a`— y la causa original era buena.
> Tachar algo exige el mismo rigor que afirmarlo; si no, la corrección se convierte en el error
> siguiente y encima llega blindada con la palabra *"medido"*.
>
> **Corolario práctico: descartar por eliminación solo vale si las opciones son exhaustivas.** El
> árbol de N-43 tenía tres ramas, dos tachadas, y se anunció la tercera como *"lo que queda"* —un
> hallazgo grave de firmware—. Al correr el comando disparó **una cuarta que nadie había listado**,
> y no había ningún defecto de firmware. Eliminar entre opciones incompletas es adivinar con tabla.

> **Un instrumento que existe no es un instrumento que mide (N-44).** `gcc` estaba instalado,
> respondía `--version` y compilaba a `.o`; su `ld` no enlazaba nada porque el toolchain vivía bajo
> una ruta con `ñ`. Los dos arneses que compilan C++ real cayeron a `ABORTADO` de un día para otro
> **con el mismo compilador registrado en el acta**. Por eso `compuerta.py` no pregunta *"¿hay
> gcc?"*: le **exige enlazar** un `main()` vacío antes de fiarse. Es `PASS` contra `ABORTADO`
> aplicado al propio compilador — y vale para cualquier herramienta que el banco dé por sentada.
>
> Su otra mitad: **un instrumento no puede depender del entorno de quien lo llama.** `compilar.ps1`
> moría en `Get-FileHash` porque el `PSModulePath` que hereda la sesión mezcla los módulos de
> PowerShell 7 con los de la extensión del IDE y el autocargado de PS 5.1 se queda sin encontrar
> `Microsoft.PowerShell.Utility`. Desde fuera parecía el arnés roto. Si una comprobación puede
> escribirse sin depender de módulos, se escribe así.

## 3.ter Un pack nuevo no es un parche: es la pasada que faltaba

> **Si los defectos aparecen porque alguien pregunta, no hay metodo — hay suerte.**

El 26/08 se cerraron ocho defectos de V9.0 y **los ocho salieron de que el responsable preguntara**:
camaras en pines de botones, PIN sin validar, radio sin direccionar, el Esclavo moviendo luces, la app
incapaz de hablar SPP... Ninguno de una revision sistematica. Su critica fue exacta: *"planteo uno de
los problemas, no todos, y no haces mas que dar un fix puntual a cada caso"*.

La pasada sistematica —censar **toda** la superficie de entrada del firmware y cruzarla con los packs
y con todos los documentos— encontro sola, en una tarde, dos defectos que nadie habia preguntado:

- **`CAM_UMBRAL_PIN` (`PB8`) tenia `pinMode()` y ni un `digitalRead()`** mientras cuatro documentos
  describian su funcion. Un `pinMode()` sin lectura es la version silenciosa de la prueba muerta.
- **El enclavamiento SFTY-2 no era el mismo en las dos puntas.** El Esclavo llevaba un
  `amarillo = false` de mas, con deliberacion de un modelo en ingles debajo razonando sobre un estado
  (`S_ROJO_AMARILLO`) **que no existe**. No cambiaba el comportamiento de hoy: era codigo muerto
  dentro de una regla de seguridad, esperando a que alguien anadiera una transicion rojo+ambar.

**La herramienta es el censo, no la lectura.** `grep` de `digitalRead`, de `pinMode`, de las llamadas
de cada despachador, y `diff` entre las dos puntas. Y su producto no es un arreglo: es un pack que
impide que vuelva.

> **Corolario sobre lo que se escribe en el fuente:** si un comentario delibera —*"Wait, in
> S_ROJO_AMARILLO state..."*, *"let's just force rojo LOW"*— es que quien lo escribio no lo tenia
> claro **y lo dejo asi**. En una regla de seguridad eso no es una nota: es una alarma. Los
> comentarios de este repositorio explican **por que**, en espanol y en ASCII; una duda sin resolver
> se resuelve o se anota en `roadmap.md`, no se deja flotando dentro de `aplicarSalidas()`.

> 🔴 **Y el censo tiene una segunda dirección: retirar código NO es neutro cuando otros dependen de
> que una bandera pueda ser CIERTA.** `mando_ambarLocal()` —`Esclavo/src/mando.cpp:103`, un getter
> de una línea— tiene **tres consumidores** en `Esclavo/src/main.cpp` (`:406`, `:416`, `:540`), y
> los tres la usan para **vetar**: `if (!mando_ambarLocal() && !bluetooth_ambarEmergencia())`. Es
> la desobediencia deliberada que documenta `mando.h`: mientras un operario pidió ámbar local, una
> orden de radio **no** saca a esa punta del ámbar.
>
> Al retirar el mando, esa bandera **no se arma nunca**, los tres `if` se vuelven siempre
> verdaderos y **el veto desaparece**. No queda inerte: queda abierto, y el código que lo abre es
> el que se borró en otro fichero. Antes de borrar el **armador** de una bandera, se censa quién la
> lee y **qué pasa si nunca vale `true`** — que casi nunca es «nada».

---

## 3.quater Un ABORTADO es una puerta abierta, no una casilla pendiente

> **Mientras un instrumento esta abortado, todo lo que vigilaba entra sin mirar.**

`ABORTADO no es PASS` ya estaba escrito, pero se leia como *"esa comprobacion no cuenta"*. La otra
mitad la cobro N-75: el rewrite de la app entro con **dos** instrumentos en `ABORTADO` —el banco
entero, porque un pack buscaba la rama de `$STATUS` en `app.js` y el parser se habia mudado a
`js/`; y el arnes de DOM, que reventaba con un `TypeError` sobre una pestana que ya no existia—.
Eran justo **los dos unicos que ejercen la app**. Detras entraron cuatro defectos: una app que
dejo de oir al equipo y pintaba un estado inventado, una barrera de PIN que la propia app abria,
un parser de un protocolo que ninguna punta habla, y comandos del firmware sin interfaz.

**Un `ABORTADO` no se apunta para luego: se arregla antes de mirar nada mas.** Y su corolario para
los informes: un parte de trabajo con seis artefactos que existen y 29 tests que pasan puede ser
**cierto en cada linea y falso en conjunto**. El dato que lo habria dicho en diez segundos era el
que no estaba: la salida de `compuerta.py`.

---

## 3.quinquies La interfaz tambien tiene pruebas muertas, y ademas mienten a alguien de pie

> **Un panel de demo que escribe en los MISMOS widgets que el dato real es la version de interfaz
> de la prueba que no mide nada.**

La app de campo traia un *"SIMULADOR DE PRUEBAS - DEMO EN VIVO"* en la pantalla principal: ocho
botones que pintaban fases, bateria baja y radio caida sobre los mismos semaforos y el mismo
contador que la telemetria, avisando con un *toast* que se va solo. Y traia su gemelo, peor:
`runLocalTicker()` animaba un ciclo completo **sin que nadie lo pulsara**, en cuanto no habia
equipo conectado.

**Lo que sustituye a un dato que no se tiene no es una simulacion: es decirlo.** Sin enlace la
pantalla se congela y lo declara. Un tablero quieto que admite que no sabe es honesto; uno que
anima un cruce que no existe le miente a quien decide sobre el trafico mirandolo.

---

## 4.ter Una captura a un solo ancho no es una prueba de interfaz

> **El sintoma y la causa no viven en el mismo sitio. Se mide; no se mira donde duele.**

Reportado desde el telefono: *"no veo el boton de la derecha, y DAR PASO y ROJO TOTAL salen a la
mitad"*. Medido con el navegador a cuatro anchos:

| 412 px | 390 px | 360 px | 320 px |
|---|---|---|---|
| **0 px** | 11 px | **41 px** | **81 px** |

Las capturas del `evidencia/` estaban limpias porque se hicieron a **412 px, el unico de los cuatro
donde el fallo no aparece**. Un `.png` de una interfaz demuestra que a ESE ancho se veia bien, y
nada mas.

Y la causa estaba tres bloques mas arriba que el sintoma: **un hijo flex no baja de su ancho de
contenido salvo que se le diga** (`min-width: auto` por defecto), asi que el boton *"Dispositivo"*
de la cabecera ensanchaba el documento entero y cortaba todo lo que quedaba a su derecha. Quien
mira donde duele arregla la botonera y no cambia nada.

**Corolario de paleta, del mismo dia:** *"el tema oscuro contrasta bien"* es una opinion; el
contraste WCAG sale del propio CSS y es una cuenta. Al hacerla apareció que el **rojo** —el color
que dice *ESPERA*— estaba en 4,9:1 y el texto atenuado en 4,0:1, por debajo de AA. Se corrigen
midiendo, no eligiendo, y con un arnes que recalcula los ratios del CSS en cada corrida. **Lo que
esa cuenta NO cubre y va escrito al lado: a pleno sol el reflejo sube el nivel de negro y comprime
los ratios, y a un tema oscuro le comprime mas que a uno claro.** Contra el sol la intervencion
demostrada es un modo dia, no un color mas brillante.

---

## 4.bis Un sospechoso se elige por ficheros, no por el mensaje del commit## 4.bis Un sospechoso se elige por ficheros, no por el mensaje del commit

> **Antes de nombrar un culpable, cruza el commit contra los ficheros que toca.**

El triage de la regresión del Modo Automático nombró *"sospechoso principal"* a `N-23`
—*"cambió quién y cuándo mueve el coordinador"*— leyendo su **mensaje**. Por ficheros, `N-23`
toca `lcd.h`, `lcd.cpp` y `modo_hora.cpp`: cambió quién mueve el coordinador **desde la pantalla
AJUSTAR HORA**, no desde el ciclo. Ninguno de los 8 candidatos tocaba `coordinador.cpp`,
`modo_automatico.cpp`, `semaforo.cpp` ni `mando.cpp`. Es la regla del instrumento aplicada al
triage: **el buscador era el resumen del commit**, y los resúmenes describen la intención, no el
alcance.

```
git log --oneline <bueno>..HEAD -- <los ficheros del camino que falla>
```

**Una bisección necesita un extremo bueno VERIFICADO.** Aquella ventana empezaba hora y media
después del último firmware sano: los 8 candidatos eran posteriores al fallo, así que los 8
habrían fallado. Una bisección cuyo extremo *"bueno"* no es bueno **no acota nada — solo consume
cargas**, y cada carga es un SWD en `mode=UR` con sus reintentos. La primera carga de banco
siempre es el ancla, y su trabajo es **funcionar**; si falla, se para y se replantea la ventana
antes de gastar la sesión.

**Y compara hashes, no tamaños.** Dos binarios del mismo tamaño pueden ser el mismo fichero
—`2779d9b` y `831c4f0` lo son: el módulo que añade el segundo no se conecta hasta más tarde y el
enlazador lo descarta— o no serlo —`8a45ae7` y `f37581f` pesan igual y difieren—. **El tamaño no
decide en ninguna de las dos direcciones.** Un `md5sum` antes de mandar a alguien al banco ahorra
cargas enteras.

## 5. Los instrumentos leen el fuente por ruta

Los validadores no incluyen el firmware: lo **parsean**, y direccionan cada archivo por tuplas
—`("Maestro", "src", "mando.cpp")`—. Consecuencia dura:

> **Mover o renombrar un `.cpp` rompe un instrumento.** El movimiento y la actualización de
> rutas van en el **mismo commit**, con la compuerta verde antes y después.

`compuerta.py` lleva una *guarda de rutas* que censa las que los validadores declaran y aborta
si alguna no existe. No la desactives: es la red de la migración a `lib/Common`.

> ⚠️ **Pero la guarda de rutas NO lo cubre todo, y saber dónde acaba importa.**
>
> Vigila **ficheros que desaparecen**. **No** vigila **contenido que se muda de fichero.** Si
> sacas una función de `main.cpp` a un módulo nuevo, `main.cpp` sigue existiendo: la guarda no ve
> nada, y el validador que buscaba ese patrón **deja de encontrarlo y reporta `FALLA`, acusando
> al firmware de un defecto que no tiene**.
>
> Pasó en la Fase 2, con `cfgVerdeSeg = pkt.param;`. Lo cazó la **comparación de totales**
> —`36/41` contra los `37/41` de siempre—, que es la única red para esta clase de deriva. Es la
> segunda vez que esa comparación salva la migración.

## 6. Barrera de salidas

**Solo `semaforo.cpp` escribe pines de luz.** Todo pasa por su `escribirPines()` estático, incluidos los
destellos del mando —que *interceptan* las escrituras en vez de rodearlas, para no dejar
colgado al coordinador esperando un `S_VERDE` que no llegaría—.

> ⚠️ **Y la regla dice OCHO pines donde el firmware mueve SEIS (N-96, 31/08).** `escribirPines()`
> escribe `ROJO1/2`, `AMARILLO1/2` y `VERDE1/2` — seis—. **`ROJO_PEATON` (`PA6`), `VERDE_PEATON`
> (`PA7`) y el `BUZZER` (`PB1`) están declarados en `pines.h` y MUERTOS en las dos puntas**: sin
> `pinMode`, sin `digitalRead`, sin `digitalWrite`. La regla era **vacuamente cierta** para dos de
> sus ocho sujetos, y la palabra *«custodia»* sugería lo contrario de lo que pasa.
>
> Lo peor no es el hardware muerto —`OPTIMIZACIONES.md:1427` ya lo decía bien—: es que
> **`barrera_01_pines_de_luz` no podía detectarlo**. Solo mide *fugas hacia fuera*, acepta
> `len(luces) >= 6` sobre una lista que devuelve 8, y su `control_negativo` nunca ejerció un pin
> peatonal. **Una regla de seguridad que enumera sujetos tiene que comprobar que cada sujeto
> existe**, no solo que nadie la rodea.

Una orden inválida **se rechaza y se reporta**. El ámbar automático queda reservado a los
caminos que ya lo tienen (SFTY-6, watchdog): **la máquina no decide sola** operar de un modo
que nadie pidió.

> 🔴 **Y la barrera se extiende a lo que el equipo CONTESTA: un `$ACK` que no depende de lo que la
> llamada devolvió es una mentira con formato de éxito (28/08).** La rama `SET_RTC` del despachador
> de Bluetooth llamaba a `reloj_ajustar()` y a `coordinador_sincronizarHora()` y mandaba
> `"$ACK,CMD:SET_RTC,RESULT:OK"` **sin mirar ninguna de las dos**. Las dos negativas son correctas
> y están razonadas donde deben —`if (!rtcOperativo) return;`, `if (!reloj_enHora()) return false;`—;
> lo que estaba mal era el que contestaba. Con `Y2` **confirmado muerto en hardware** (N-17), en la
> tarjeta real ese comando decía que sí y no ponía la hora, y el técnico se iba del poste creyendo
> que dejó el reloj puesto.
>
> **Eran tres ramas, no una** —no hay con qué contar el tiempo · la hora no entró · la hora entró y
> va camino del Esclavo—, y **las dos de más las encontró un pack** (`app_03_sin_ok_mudo`), no una
> revisión humana: el mismo defecto estaba en el `SET_RTC` del Esclavo y en
> `MANUAL:CAMBIAR_TURNO`, que llama a `coordinador_pedirCambio()` con su `if (estadoC != C_IDLE)
> return;` delante.
>
> **El molde de cómo se hace bien vive en el mismo fichero: `SET_TIEMPOS`** — pregunta dentro del
> `if` y tiene un `$ERR` por cada motivo de rechazo. Un despachador se escribe copiándolo, y un
> pack que sabe acusar tiene que saber también reconocerlo, o sus acusaciones no valen nada.

## 7. Presupuesto de flash

64 KB por micro; el Maestro va por el **88,3 %** —`57880` de `65536` B, o sea **7.656 B libres**—
y el Esclavo por el **64,4 %** (acta del 28/08). **Ya no queda margen cómodo**: a este nivel una
función nueva de tamaño medio no entra sin haber medido antes de qué está hecho el porcentaje.
Antes de proponer estructura:

> **Y antes de sacrificar una función porque «no cabe», MIDE DE QUÉ ESTÁ HECHO ese porcentaje.**
> Durante meses la conversación fue *«qué quitamos para que quepa lo siguiente»* sin que nadie
> hubiera mirado nunca. Al medirlo (N-70): **el firmware propio era el 30 % del binario**, y había
> **5.160 B de I2C enlazados en un equipo sin un solo bus I2C** —`U8x8lib.cpp` referencia
> `TwoWire::setClock()` y el enlazador arrastra `Wire` entero—. Dos banderas de compilación,
> cero cambios de código, y los bytes libres pasaron de 4.292 a 9.452.
>
> **Cómo se mide, que es la parte reutilizable:**
> - **Por FICHERO OBJETO, leyendo `firmware.map`. NO por nombre de símbolo.** Los símbolos de C++
>   de cualquier librería también empiezan por `_Z` —`TwoWire::setClock` es `_ZN7TwoWire8setClockEm`—,
>   así que clasificar por el nombre mete librería ajena dentro de *«lo nuestro»*. El primer censo
>   de N-70 se equivocó exactamente así.
> - **La causa no se deduce: `firmware.map` tiene una sección que dice quién arrastra a quién.**
> - **Un delta exige medir los DOS extremos.** El primer número publicado de N-70 —*«−5.492 B»*—
>   salió de restar contra un `.elf` viejo que había en disco. Era la cifra correcta de ninguna
>   pareja de binarios.
>
> El toolchain vive en `C:\.platformio` (fuera de la ruta con `ñ`, N-44):
> `arm-none-eabi-nm --size-sort -S -td firmware.elf`.

> 🔴 **Un camino muerto que no cuesta flash puede seguir costando RAM (N-86, 28/08).** `AiBus` —un
> `HardwareSerial` declarado sobre `(PA10, PA9)` para un «puerto IA» que nunca existió— no tenía un
> solo llamador, y `--gc-sections` **ya descartaba sus tres funciones**: retirarlas ahorró **16 B**
> de flash. Un censo que mirara solo el mapa habría dicho *«esto no vale la pena»* y lo habría
> dejado.
>
> Pero **el objeto no se podía descartar**: tiene constructor, y su llamada vive en `.init_array`,
> así que corre en cada arranque y su `.bss` es permanente. Eran **280 B de RAM por punta**, el
> **5,2 %** de la RAM viva del equipo — por un puerto que no se abre.
>
> **La flash se mide con la compuerta; la RAM, con `nm` sobre el `.elf`.** Un censo de código muerto
> que solo mire flash **no ve los objetos globales**, que son justo los que el enlazador no puede
> tocar. Se retira el objeto entero, no solo su apertura.
>
> *(De paso, el dato duro que salió de ahí: `SerialBT` vive hoy en **`PB6`/`PB7`, USART1 remapeado,
> conector `J17`** —N-76—, no en `PA9`/`PA10`. Dos objetos sobre el mismo periférico a velocidades
> distintas no dan error: dan el último que arrancó.)*


- **Nada de clases con métodos virtuales.** Las vtables cuestan flash que no hay.
- La separación se hace con lo que ya se usa: un `.cpp` por concepto, `static` para lo privado,
  header corto.
- Cada cambio estructural anota la cifra de flash antes y después. Si sube más de ~2 %, se
  revisa antes de seguir.

## 8. Los simuladores validan el modelo, no el código

`simulador_sistema_v7_6.py` y los packs del banco son **Python escrito a mano** que reimplementa
lo que hace el C++. Un `PASS` suyo no prueba el firmware; prueba el modelo.

**Los que sí compilan el C++ real son cuatro, y conviene saber qué cubre cada uno:**

| arnés | compila de verdad | punto ciego |
|---|---|---|
| `Validacion_LCD` | `lcd.cpp`, `menu.cpp`, `modo_degradado.cpp` | framebuffer en el PC, **no** la ST7920 |
| `Validacion_Ciclo` | `ciclo_degradado.h` | función pura: no hay maquina de estados |
| `Validacion_Respaldo` | `calcularSuma()`, Horner, `respaldo_horasDesdeSync()` | no ejerce el arranque |
| `Validacion_Automatico` | `coordinador.cpp` + `semaforo.cpp` + `modo_automatico.cpp` | **solo el Maestro**: "verde simultáneo en las dos puntas" no se mide ahí |

`Validacion_Automatico` existe por una razón concreta: la regresión del Modo Automático pasó con
la compuerta en verde y el arnés de pantalla en `241/241`, porque **nadie ejercía el ciclo**. Mide
SFTY-2 sobre lo que `semaforo.cpp` **escribió en los pines**, no sobre su lógica.

Nada de esto sustituye la prueba de banco.

## 8.bis Un arnés que no se ha visto fallar es un adorno que da verde

> **Antes de conectar un arnés a la compuerta, rómpele el firmware a propósito y compruébalo.**

No basta con que tenga controles negativos escritos dentro: eso es una etiqueta. Se inyecta un
defecto **en el `.cpp` real**, se corre, y se exige que **baje la cuenta y cambie el código de
salida**. `Validacion_Automatico` se conectó tras verlo caer a `25/26` con `VERDE1` forzado a HIGH
por debajo del enclavamiento de `aplicarSalidas()`. El firmware se restaura acto seguido y se
verifica con `git diff HEAD` **vacío** — no con la impresión de haberlo restaurado.

## 8.quater Al arreglar un defecto, busca las pruebas que lo celebraban

> **Un banco maduro contiene pruebas que EXIGEN el comportamiento defectuoso.** No por descuido:
> se escribieron cuando el defecto se creía inevitable, y documentan su coste con honestidad.

Pasó en N-49. La prueba 2.3 de `maestro_02_respaldo` afirmaba que una sincronización de hace **dos
horas** debía declararse `CADUCADA` al cruzar de mes, y lo llamaba *"el coste operativo del criterio
conservador… es la dirección segura"*. Era cierto **mientras el dato guardado no permitiera fechar**.
Arreglado el fechado, el firmware devuelve `2` —lo correcto— y esa prueba **falla**.

Cuando eso ocurra, la tentación es reescribirlas en bloque hasta que pasen. Eso es **ajustar el
instrumento hasta que dé verde**, y es lo que este repositorio castiga. Van una por una, y cada una
acaba en uno de tres sitios, anotado:

| | |
|---|---|
| **se borra** | solo existía para documentar el defecto |
| **se invierte** | pasa a exigir el comportamiento nuevo |
| **se conserva** | medía otra cosa y sigue valiendo |

**Y el mejor termómetro del arreglo son los fallos que desaparecen solos.** Si arreglas la causa
raíz de cuatro `FALLA` y tras actualizar el instrumento siguen ahí, el arreglo no está completo.

## 8.ter El trabajo delegado se revisa por el diff, no por su informe

Es la regla del instrumento aplicada a los agentes, y costó una regresión el 04/08. Un agente
entregó *"31/31 verificado"* sobre el `30/31` del Esclavo: había cambiado a la vez el firmware **y
las dos copias del modelo que debían vigilarlo**. Las tres copias decían lo mismo, así que el banco
no podía verlo, y el arreglo introducía un defecto nuevo —apagaba la bandera de la que cuelgan los
cuatro getters públicos—.

- Si un cambio toca **el firmware y su modelo a la vez**, mira si el modelo *replica* el arreglo o
  si *relaja* la comprobación. Solo lo primero es válido.
- Una cifra en verde después de tocar las dos puntas **no demuestra nada por sí sola**.
- Y su corolario: **una variable que contesta a dos preguntas distintas no puede contestar bien a
  ninguna.** `cfgVerdeRecibido` significaba *"la radio entregó el par"* y *"hay un VERDE sin
  emparejar"*; arreglar una rompía la otra. Eran dos banderas.

## 8.quinquies Dos agentes sobre el mismo árbol se pisan sin avisar

> **Un `git add -A` barre lo que el otro tiene en vuelo, y la historia queda mintiendo.**

Pasó el 27/08. El commit `ff6bd19 fix(N-71): el techo de silencio...` contiene **un solo fichero:
el acta**. Todo el firmware de N-71 —un umbral de seguridad, dos packs, cuatro instrumentos—
acabó dentro de `1bf9251`, cuyo mensaje habla de *«insercion, borrado de 17 y re-agregado de 4
cruces»*.

El contenido era correcto y la compuerta verde; **lo roto es la historia**, y eso se cobra tarde:
*«un commit = un cambio con sentido propio = un `git revert` limpio»*. Un `revert` de `ff6bd19`
no deshace nada.

- **Con dos agentes a la vez: o cada uno en su `git worktree`, o `git add` de rutas explícitas.
  Nunca `-A`.** El árbol compartido no avisa: simplemente mezcla.
- **Y no se reescribe la historia publicada para arreglarlo.** Si la rama está en dos remotos y
  el otro agente sigue encima, un `push --force` causa más daño del que repara: se anota dónde
  vive de verdad el cambio y se sigue.

## 9. Carga por SWD

**`mode=UR` con `-e all`, y no se cambia.** `HOTPLUG` se engancha al micro en marcha: con un
firmware que se cuelga al arrancar, el watchdog reinicia cada 4 s en mitad del borrado
(`failed to erase memory`). El delator es `NVM size: 128 KBytes (default)` en un chip de 64 KB.

> Si `UR` falla, **reintenta — no cambies el modo.** Enganchar es cuestión de *timing* y puede
> fallar varias veces con `Unable to get core ID`. Eso no es falta de cableado.

## 9.bis Firmware primero; el cableado después. Un commit no protege de un destornillador

> **Cuando un cambio reparte un conector entre firmware y cobre, el orden no es «el mismo commit»:
> es ASIMÉTRICO, y solo uno de los dos sentidos es seguro.**

`J16` es el conector de los cuatro botones, y ahí es donde van a ir las cámaras. Los dos pines que
importan están medidos: `BOTON3 = PB14` es **`botonAceptar()`, el que EJECUTA**, y `BOTON4 = PB15`
es `botonCancelar()` (`Maestro/include/pines.h:94-95`, `Maestro/src/botones.cpp:131-132`).

- **Firmware primero es seguro.** Retirado `botones_setup()`, los pines dejan de estar en
  `INPUT_PULLUP` y quedan fijados a **0 V** por los `R65`–`R68` de **10 kΩ a masa** que la placa ya
  trae. Un pin en 0 V no ejecuta nada.
- **Cableado primero NO lo es.** Con el firmware viejo todavía dentro, `PB14` sigue siendo
  `botonAceptar()` leído **activo en BAJO**: cualquier cosa que un instalador enchufe en `J16` p10
  puede pulsar *Aceptar* en un equipo que está en la calle.

**Por eso la regla no es «van en el mismo commit» —un commit no protege de un destornillador—: es
que el firmware nuevo tiene que ESTAR CARGADO EN LA TARJETA antes de que nadie enchufe nada.** Se
exige la carga verificada, no el merge. Y mientras la polaridad de esos cuatro pines siga en
contradicción entre el netlist y el fuente (medida `M3` de `05_Funcional/17_...`), **no se cablea
cámara a `J16`** ni con el orden correcto.

> ✅ **Matiz del 31/08, y baja el coste de M3.** La contradicción es entre el netlist y
> **`botones.cpp`** (`INPUT_PULLUP` + `== LOW`). El camino de **cámara** ya lee al revés
> —`pinMode(INPUT)` pelado y **activo en ALTO**, `modo_inteligente.cpp:46` y `:25`, desde N-67—, así
> que **en cuanto `PB14`/`PB15` se lean como cámara el firmware ya coincide con el netlist**. Y la
> salida de la AcuSense es configurable (NO/NC), así que se elige qué estado significa demanda sin
> tocar placa ni firmware.
>
> **Lo que M3 sigue decidiendo, y es su tercer resultado posible:** con `INPUT` pelado el pin
> necesita **resistencia real a masa en la placa** o queda flotando y el ruido dispara demandas
> fantasma. `PB0` la tiene declarada (`pines.h:43-46`, `R64` 10K + `C25` 100nF); de `PB14`/`PB15`
> **sólo lo dice el netlist y nadie lo ha medido en cobre**. M3 pasa de bloqueante a
> **confirmación que parametriza la cámara** — pero se hace **antes** de cablear, no después.

## 10. Radios

**`2.4 kbps` de Air Data Rate, `M0`/`M1` ambos en OFF** durante la operación. Configuración
vigente: 2 radios en enlace directo, **sin repetidor**.

---

## Dónde está cada cosa

| | |
|---|---|
| `01_Firmware/Maestro`, `Esclavo`, `Repetidor` | firmware |
| `01_Firmware/compuerta.py` | **la única forma correcta de verificar** |
| `01_Firmware/Simulaciones/` | simuladores, y `banco/` con los **38 packs** |
| `01_Firmware/Validacion_LCD/` | arnés de pantalla (compila el `lcd.cpp` real) |
| `01_Firmware/Validacion_Ciclo/` · `_Respaldo/` · `_Automatico/` | los otros tres que compilan C++ real — ver §8 |
| `OPTIMIZACIONES.md` | las reglas `SFTY-x` y la **trazabilidad regla → código → prueba** |
| `05_Funcional/` | manuales y protocolo de pruebas para el auditor |
| `evidencia/` | actas de la compuerta, con fecha y hash |
| `ESTADO.md` | dónde está parado el trabajo **hoy** |
| `roadmap.md` | **estado del proyecto** desde el 31/08: qué hay, qué está decidido, qué está abierto y **el orden de arranque** — con los `N-x` debajo como el *porqué*. Ya no es una bitácora; lo anterior a esa fecha vive en el `git log` y en el remoto `padre` |

## Convenciones

- **Comentarios y mensajes de commit en español, en ASCII sin acentos** (la consola de Windows
  viene en cp1252 y los validadores parsean el fuente).
- Los comentarios explican **por qué**, no qué. El código ya dice qué hace.
- Un commit = un cambio con sentido propio = un `git revert` limpio.
- Cada `N-x` del roadmap se cierra con la evidencia que lo demuestra, no con una afirmación.
- Si un pack ejerce una regla `SFTY-x`, se marca con `# EJERCE SFTY-x: <qué>` en su cabecera. La
  tabla de trazabilidad de `OPTIMIZACIONES.md` se levanta **buscando** esa etiqueta. **Solo se
  etiqueta lo que el pack comprueba de verdad**: una regla que aparece cubierta por una prueba que
  no la ejerce es peor que una fila vacía, porque la vacía no miente.
