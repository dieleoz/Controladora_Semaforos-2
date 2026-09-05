# ENCARGO — Sesión de banco · Controladora de Semáforos

**Redactado:** 31 de Agosto de 2026 · **Ejecutado:** 3 y 4 de Septiembre de 2026 · **Rama:** `main-nuevo`

---

## ✅ ESTE ENCARGO YA SE EJECUTÓ — 3 y 4 de septiembre de 2026

**Se ejecutó sobre el paquete V9.0, commit `617bd00`.** El informe es
[`evidencia/Informe_Pruebas_Banco_Semaforos_V9.0.pdf`](../evidencia/Informe_Pruebas_Banco_Semaforos_V9.0.pdf)
— preparado por Sebastián, IT Vial SAS. **Lo que sigue en este documento ya no es una tarea: es el
guion de lo que se hizo, con lo que quedó fuera marcado.**

**El resultado, y va delante de todo lo demás: la CABECERA del informe declara 24 completos, cuatro
bloqueados y uno abortado por seguridad — y su propia enumeración no da esa cuenta. Aquí se publica
la discrepancia, no un total.**

| | lo que declara la CABECERA del informe | los pasos que esa misma cabecera NOMBRA |
|---|---|---|
| **Completos** | **24 / 29** | `1-6, 8-9, 15-18, 20-24` → **17 identificadores** |
| 🔴 **Bloqueados** por el enlace Bluetooth | **4 / 29** | `25, 26, 27, 28` → **4** |
| 🛑 **Abortado por seguridad** | **1 / 29** | `29` (mando de relés) → **1** |
| | **29** | **22 — faltan siete: `7`, `10`, `11`, `12`, `13`, `14`, `19`** |

> 🛑 **LA CUENTA NO SALE, y este encargo no la arregla inventando otra.** Los siete pasos que la
> cabecera no mete en ningún cajón **sí están descritos en el cuerpo del informe**: `7` y `19` como
> *PARCIAL*, `10` como *no logrado*, `11` y `12-14` como *BLOQUEADO*. Repartidos como el cuerpo los
> describe, lo que cuadra a 29 es **19 / 9 / 1**, no 24 / 4 / 1.
>
> **No se publica aquí ninguna de las dos.** Cuál vale lo decide quien ejecutó la sesión, no este
> repositorio. Lo que sí se deja escrito es **dónde está la diferencia** —los pasos **10 a 14**, el
> módulo que no se anuncia y todo lo que cuelga de él—, porque **un paso que no está en ningún cajón
> es un hueco que no deja rastro de que falta**. Mismo criterio que
> [`12_Cobertura_de_Pruebas_y_Huecos.md`](12_Cobertura_de_Pruebas_y_Huecos.md), §«La cuenta, y por
> qué este documento no la copia».
>
> ⚠️ ~~**Y tres de esos 24 son PARCIALES: los pasos 7, 19 y 21 … se cuentan dentro de los 24 por su
> parte verificable.**~~ → **CADUCADO el 04/09: esa reconciliación no cuadra.** El **paso 21 ya
> estaba dentro** de `20-24`, así que sumarlo aparte como *parcial* no explica nada; y dejaba fuera
> los pasos **10 a 14**. **Lo que sí es cierto y no depende de la cuenta:** los pasos **7 y 19**
> quedaron parciales —cableado y medida verificados; la respuesta funcional del semáforo **no**,
> porque depende del Modo Automático y el modo no se llegó a seleccionar; el informe lo atribuye a
> que *«sólo se puede seleccionar desde la app»*, **eso no está medido y este documento lo deja
> abierto**, ver el apartado «Orden de la sesión»—, y el **21** cableó cámara sin demandas fantasma.

### Lo que queda pendiente, que son 5 pasos y 3 repeticiones

| qué | por qué quedó fuera | qué hace falta para hacerlo |
|---|---|---|
| **Pasos 25, 26, 27, 28** — los dos módulos en la lista de Bluetooth, operar desde la app, el reloj conserva la hora, comandos con PIN | el ESP32 **nunca se anunció de forma fiable** en el teléfono. El chip **no era la causa**: se confirmó `ESP32-WROOM-32` clásico, BR/EDR, el perfil que el SPP de la app necesita | 🟢 **ya está: eran DOS defectos en serie y los dos están arreglados** — **N-117**, el watchdog del ESP32 se comía su propio arranque; **N-122**, la app nunca llamaba a `connect()`. **Hay APK nueva.** Se reintentan tal cual están escritos |
| **Paso 29** — el mando de relés por puente | 🛑 **incidente**: al puentear `J16` p5/p8 contra masa el semáforo no cambió de comportamiento **y el STM32 del Maestro empezó a calentarse**. Se cortó la alimentación en el acto | 🔴 **NO se repite sobre esa tarjeta: N-116.** Hay un **corto medido entre `3,3 V` y `GND`**; la placa funciona unos 30 s y se calienta. **No se reenergiza** hasta inspección técnica con el equipo frío |
| **Repetir el paso 7** — que el ciclo mueva las luces | el equipo nunca llegó a operar: se queda esperando selección de modo | resuelto el Bluetooth. **Es N-42, y sigue abierta** — ver abajo |
| **Repetir los pasos 19 y 21** — la concesión de paso ante demanda de cámara | ídem: el cableado y la medida sí; la reacción del semáforo no | ídem |
| **Medir la fuente `12 V -> 5 V`** de la placa del módulo (paso 22) | en banco el módulo se alimentó **por USB**, no por su fuente real | una carga de 12 V, antes de instalar en campo |

> 🔴 **N-42 no se cerró, y tampoco se descartó: no se pudo ejercer.** El bloque 1 de este encargo
> —*«que el ciclo mueva las luces»*— **no llegó a ejecutarse**: el equipo se quedó esperando
> selección de modo y el modo no se seleccionó. **`ABORTADO` no es `PASS`:** N-42 sigue abierta y
> **sigue siendo lo primero de la próxima sesión**, exactamente igual que cuando se escribió este
> encargo.

### Lo que la sesión SÍ cerró, y no hay que volver a preguntar

- ✅ **La medida crítica de `J16` (paso 20) — la medida `M3` del manual 17.** Los cuatro pines dieron
  lo mismo: **~9,9 kΩ a masa**, ~11,3 kΩ a 3,3 V. `p10` y `p12` (cámara) a **0 V** en reposo; `p5` y
  `p8` (mando) a **0,6 V**. **El pull-down de 10 kΩ del netlist es real**, y el conector pide contacto
  contra los 3,3 V: **entrada activa en ALTO, en las cuatro.**
- ✅ **La cámara en `J16` se cableó y funciona** (paso 21): `p10` contra `p11`, normalmente abierto,
  **sin falsas demandas con el cable puesto ni sin él**. El bloqueo *«no se cablea cámara a `J16`
  hasta M3»* **queda levantado**.
- 🔴 **Y la misma medida condena el mando: hoy `A`/`B` NO SE PUEDEN PULSAR (N-118).** Con el pin en
  0,6 V la lectura cruda ya es `LOW` al arrancar, el antirrebote lo siembra como *flanco ya
  consumido* y **la transición nunca llega**. No es que el mando funcione mal: es que **no hay gesto
  que lo active**. El apartado 4 de este encargo está escrito sobre la premisa contraria y **queda
  tachado** — ver allí.
- ✅ **`J17` es lo que decía el manual 17** (pasos 3 y 5): continuidad a las patas 42 y 43 del `U1`,
  ni un pin por encima de 3,3 V, **12 V sólo en `J16`**. El esquemático KiCad es el que está viejo.
- ✅ **El ámbar por pérdida de radio funciona** (paso 8): tres cortes, **~20 s** hasta ámbar
  intermitente en las dos puntas, y vuelta sola a rojo fijo en ~3 s al reconectar.
- ✅ **La talanquera** (pasos 15-16) y **la cámara de `J14`** (pasos 17-18) responden como se esperaba.
- 🔴 **NUEVO — N-120, y no lo pedía este encargo:** la placa protege sus **9 salidas** con 220 Ω en
  serie + opto `TLP127`, y **no protege ninguna de sus 5 entradas de campo**. Por eso **tapar `J16`
  p1 deja de ser una precaución de banco y pasa a ser obligatorio en cada equipo.** La decisión de
  diseño que abre —2K2 en serie por entrada— es del responsable: manual 17 §3.6.

---

**El commit exacto no se escribe a mano aquí.** Un hash copiado en un documento envejece en horas y
manda a la gente a un árbol que no es el que tiene delante. Se saca en el momento de preparar la
carga:

```bash
git rev-parse --short HEAD
git log -1 --format='%H  %ci  %s'
```

y el estado verificado de la suite —cifras, flash, comprobaciones— está en el acta más reciente de
[`evidencia/`](../evidencia/), con su fecha y su hash dentro. **Las cifras se leen del acta; no se
copian aquí.**

---

## ⚠️ Léase esto primero: qué es y qué NO es este encargo

**Esto NO es una entrega.** No instale nada de aquí en un cruce abierto al tráfico. Lo que corre en
campo es la **V8.4**, y sigue corriendo hasta que alguien diga lo contrario con esta sesión hecha y
revisada.

Todo lo que hay en este paquete está validado **sólo en simulador y en arneses de PC**. La suite del
repositorio sale en verde, y precisamente por eso hay que decir qué significa ese verde:

> Significa que **los modelos y los arneses de PC no encuentran nada**.
> **No significa que el firmware funcione sobre la tarjeta.**

Y la prueba está delante: **con esa suite en verde hay una regresión abierta (N-42) en la que el
Modo Automático no mueve las luces sobre hardware real.** Ningún simulador la ve. Por eso existe
esta sesión.

**El producto de esta sesión son MEDIDAS, no un visto bueno.** Si algo no se puede medir, se anota
como *no medido* — **nunca como correcto**. Un hueco declarado vale; un hueco callado, no.

> ✅ **Y esto se cumplió, que es lo mejor que trae el informe del 3-4/09.** Los cinco pasos que no se
> pudieron hacer están escritos como *bloqueado* y *abortado*, con su motivo, y no como *correcto*.
> El informe dice con todas las letras que **el sistema no puede considerarse listo para campo**, y
> deja **la causa del sobrecalentamiento abierta** en vez de nombrar un culpable cómodo.
>
> 🔴 **Todo lo de arriba sigue vigente palabra por palabra: la sesión NO fue un visto bueno.** Y no
> hace falta cerrar la cuenta de arriba para decirlo — ~~«con 24 de 29 pasos hechos»~~ **es
> justamente el número que este encargo se niega a publicar**: con o sin él, **la vía principal de
> operación del equipo no se verificó ni una vez**, y **N-42 sigue sin confirmar ni descartar**.
> Nada de esto sube a un cruce.

---

## 📄 Los dos documentos que se llevan, y qué hace cada uno

Este encargo **no repite** ni el cableado ni las pruebas. Dice el **orden** y **de qué depende cada
cosa**. Lo que se ejecuta está en otros dos ficheros, y hay que llevar los dos:

| Documento | Qué es | Cómo se usa |
|---|---|---|
| [`Guia_Cableado_y_Pruebas_Banco.html`](Guia_Cableado_y_Pruebas_Banco.html) | **29 pasos** en formato `HAZ / COMPRUEBA / TIENES QUE VER / ANOTA`. Todo lo que es conectar, medir con multímetro y cargar firmware | Se abre en el navegador o se **imprime y se rellena a bolígrafo**. Guarda lo escrito en el propio navegador |
| [`3_Protocolo_Pruebas_Rigurosas.md`](3_Protocolo_Pruebas_Rigurosas.md) | **52 pruebas ejecutables** de comportamiento, y **34 marcadas como no ejecutables hoy** con el motivo escrito. Es el acta que se firma | Se ejecuta **después** de la Guía, y sus secciones se enganchan a los tramos de la Guía según la tabla de abajo |

> 🔴 **Las pruebas del Protocolo que hoy no se pueden ejecutar NO tienen casilla de firma.** Es
> deliberado: una prueba que no se puede ejecutar y sigue siendo una casilla firmable **es peor que
> una que falta** — la que falta no miente. Si alguien encuentra una casilla sin poder ejecutarla,
> eso es un defecto del documento y hay que reportarlo.

---

## 🔴 Lo que cambió desde el encargo anterior (05/08), y por qué aquél ya no sirve

El encargo del 5 de agosto citaba **otra rama y otro `HEAD`**, y sus apartados 3, 5 y 6 se ejecutaban
**enteros por la pantalla LCD y la botonera**: `CONFIGURACION → AJUSTAR HORA`, `CONSULTA RELOJ`,
`MODO DEGRADADO → Botón 3`. **Nada de eso se puede hacer hoy.**

| Qué cambió | Medido en | Qué significa para la sesión |
|---|---|---|
| **`botonAceptar()` y `botonCancelar()` devuelven `false` siempre.** Los pulsadores 3 y 4 dejaron de existir: sus pines pasan a ser entradas de cámara | ~~`Maestro/src/botones.cpp:280-281` · `Esclavo/src/botones.cpp:294-295`~~ → **`Maestro/src/botones.cpp:305-306` · `Esclavo/src/botones.cpp:316-317`** *(re-medido el 05/09: `grep -n "^bool botonAceptar"`; las dos citas viejas señalaban a comentarios, no al código)* | **No hay forma de aceptar ni cancelar nada, ni de abrir un menú.** Toda la operación pasa por comandos |
| **La pantalla no se retira, pero deja de conducir sus pines.** `PB3`/`PB4`/`PB5` quedan en alta impedancia porque comparten `J17` con el ESP32 | `Maestro/src/lcd.cpp:74-75` · `Esclavo/src/lcd.cpp:92-93` | **No hay imagen.** Nada de lo que se leía en pantalla se puede leer. **No es una avería: no lleve una pantalla de repuesto** |
| **El mando de relés SE CONSERVA**, en `A` = `PB9` = `J16` p5 y `B` = `PB13` = `J16` p8 | `Maestro/src/mando.cpp:202-235` · `Esclavo/src/mando.cpp:218-250` | `A·A·A`, `B·B·B` y `A·B·A·B` siguen en el firmware. **Pero el receptor de radio nunca se compró**: ~~hoy sólo se pueden inyectar los pulsos con un cable~~ → 🔴 **MEDIDO EL 3/09: con `617bd00` no se podían inyectar de ninguna forma.** El pin está en 0,6 V en reposo y aquel `botones.cpp` lo leía activo en BAJO: nunca había flanco (**N-118**). 🟢 **Corregido en `346ea5f`** —`INPUT` pelado y `== HIGH`, las dos puntas—, y el pulso se inyecta ahora **contra los 3,3 V del pin contiguo** (`p5`–`p4`, `p8`–`p7`). 🔴 **Sin ejercer en tarjeta: el mando existe en el código y todavía no se ha visto en la mano** |
| **El umbral de silencio son 25 s, no 12** | `SFTY6_SILENCIO_MS = 25000UL` — `Maestro/include/protocolo.h:149` y `Esclavo/include/protocolo.h:149` | Al cortar la radio se cronometran **~25 s**. Si sale alrededor de 12, **el firmware cargado no es el nuevo**. ✅ **Medido: ~20 s en tres cortes** — es el firmware nuevo, y la diferencia va en la dirección segura, pero **no está explicada**: se vuelve a cronometrar con reloj |
| **`VENTANA_TRIPLE_MS = 12000` sigue siendo 12 s, y es correcto** | `Maestro/src/mando.cpp:38` · `Esclavo/src/mando.cpp:42` | Es la ventana para encadenar `A·A·A` o `B·B·B`. **No son los 25 s de arriba: son cosas distintas y no se confunden** |
| **El ESP32 de expansión tiene firmware nuevo**, va en `J17`, con Bluetooth SPP y reloj `DS3231` | `01_Firmware/ESP32_Expansion/` | ~~**La placa que lo lleva NO EXISTE**, ni su fuente está pedida~~ → ✅ **construida y montada el 4/09** (paso 23 y 24), con su fuente. ~~y su `DS3231`~~ 🛑 **«y su `DS3231`» ES FALSO — 05/09: el módulo NO ESTÁ COMPRADO** (`A6`). **Módulo ESP32 confirmado `ESP32-WROOM-32` clásico** — BR/EDR, el perfil que el SPP necesita. 🔴 **Lo que faltó fue el enlace**, no el hardware |

*(Los `fichero:linea` de arriba son **MEDIDOS**. Los de `bluetooth.cpp` se citan en el Protocolo por
**literal y sin número de línea** a propósito: esos dos ficheros los está tocando otro trabajo en el
mismo árbol, y una línea citada que se mueve manda al lector a un sitio que no dice lo que promete.)*

> 🔴 **Y esa nota se cumplió CONTRA ESTA MISMA TABLA — 05/09.** Se re-midieron sus seis citas una
> por una. **Cinco seguían exactas** (`protocolo.h:149` las dos puntas · `mando.cpp:38`/`:42` ·
> `mando.cpp:202-235`/`:218-250` · `lcd.cpp:74-75`/`:92-93`); **la de `botones.cpp` había derivado
> 25 líneas en el Maestro y 22 en el Esclavo**, y va corregida arriba.
>
> **La lección, que es la de `CLAUDE.md` §4 y vale más que el arreglo: la palabra «MEDIDOS» de
> esta nota no medía nada.** Estaba escrita **debajo** de las citas, se heredó de una edición
> anterior y **nadie la volvió a ejercer** cuando el fuente creció. Una etiqueta de calidad que no
> se recalcula es una afirmación sobre el código sin comprobar — exactamente la lista de
> excepciones con motivos sin verificar de §3.bis, aplicada a una tabla de un encargo.
>
> **Al llevar esta tabla al banco, se re-mide. Un `fichero:linea` caduca solo.**

---

## 🧰 Qué llevar

Todo lo de la tabla del apartado **00 de la Guía**, y dos cosas que la Guía no pide porque no las
necesita:

- **Dos cables sueltos con punta fina** para hacer de mando sobre `J16` p5 y p8 (ver el Bloque 4).
- **Los dos adaptadores USB-TTL de 3,3 V**, uno por punta, para poder mirar el Maestro y el Esclavo
  **a la vez**. Con uno solo no se puede medir el desfase de relojes (prueba 7.9) ni ver las dos
  respuestas de un mismo gesto.

---

## 📋 Orden de la sesión, y el orden importa

| Bloque | Qué | Dónde está | Cómo salió el 3-4/09 |
|---|---|---|---|
| **0** | Módulo, carga del firmware, `J16`/`J17`, tapar los 12 V | **Guía, pasos 1 a 5** | ✅ **HECHO.** Las dos tarjetas cargaron por SWD **al primer intento y sin `BOOT0`**. `p1` tapado retirando el pin del conector volante |
| **1** | 🔴 **Que el ciclo arranque y mueva las luces — N-42** | **Guía, pasos 6 a 8** + **Protocolo §2 y §3** | 🔴 **NO SE PUDO EJERCER.** El equipo arranca y enlaza —rojo fijo con radio, ámbar sin ella— pero **se queda esperando selección de modo**, y el modo no se seleccionó. **N-42 ni confirmada ni descartada.** Sigue siendo lo primero |
| **2** | 🔴 **La medida de `J16` p5, p8, p10 y p12** | **Guía, paso 20** | ✅ **HECHA, y decidió los dos bloques como estaba previsto.** Cámara ✅ · **mando `A`/`B` ❌ (N-118)**. Ver la cabecera |
| **3** | Reloj, pila y veredicto del cristal `Y2` | **Protocolo §7** | 🔴 **BLOQUEADO** (paso 27): la única vía de consultar o poner la hora es `SET_RTC` **por Bluetooth**, y el enlace no subió |
| **4** | Mando de relés, por puente | **Protocolo §8** | 🛑 **ABORTADO POR SEGURIDAD** (paso 29). El puente no produjo ningún cambio **y el STM32 del Maestro se calentó**. Ver N-116 |
| **5** | 🔴 **Modo Degradado, incluido el defecto N-106** | **Protocolo §9** | ⛔ **NO EJECUTADO:** depende de los bloques 3 y 4, y los dos cayeron |
| **6** | Talanquera y cámaras de demanda | **Guía, pasos 15 a 21** + **Protocolo §11** | 🟡 **A MEDIAS, y fue lo mejor de la sesión.** Talanquera ✅ (sube en ámbar, baja al volver la radio) · `J14` ✅ · `J16` cableada ✅ · **la reacción del semáforo, no**: mismo bloqueo del bloque 1 |
| **7** | Telemetría, órdenes y **los rechazos** | **Protocolo §12** | ⛔ **NO EJECUTADO:** se hace por la app, y la app no conectó |

> **Lo que este orden acertó, y conviene no perderlo para la próxima:** el bloque 2 se puso pronto
> *«porque si sale mal hay media sesión que replantear»*. **Salió mal**, y se supo el primer día —
> con el resultado delante, el paso 21 se cableó igual y el paso 29 se abordó sabiendo qué esperar.
>
> **Lo que este orden NO previó:** que el bloque 1 no fallara, sino que **no se pudiera ni intentar**.
> Cuatro bloques cayeron detrás del enlace Bluetooth, **no detrás de un defecto**.
>
> 🔴 **Y aquí hay una pregunta abierta que este documento no puede cerrar, y no se tapa.** El informe
> describe el bloqueo como *«el modo sólo se puede seleccionar desde la app»*. **Pero el apartado 1
> de este mismo encargo dice cómo arrancar el ciclo por el terminal USB-TTL** —`CMD:PIN:1234:SET_TIEMPOS:3,3,15` *(era `1,1,15`; el mínimo vial subió a 3 min, N-137)*
> y `CMD:PIN:1234:SET_MODO:AUTO`, contra `J17` a 9600—, y el ESP32 no es más que un puente hacia ese
> mismo puerto. **Las dos cosas no pueden ser ciertas a la vez.** O el USB-TTL no se llegó a usar
> para operar, o esos comandos no hacen hoy lo que este encargo dice. **No está medido cuál de las
> dos**, y decide si N-42 se puede ejercer **sin esperar al Bluetooth**. Se comprueba en diez
> minutos con un adaptador y la tarjeta sana, y va **antes** de montar otra sesión entera.

---

## 1 · El bloque que manda: que el ciclo mueva las luces (N-42)

> 🔴 **NO SE PUDO EJERCER el 3-4/09, y por eso este apartado sigue entero y sigue siendo el primero.**
> El equipo arranca, las dos puntas se comunican y el ámbar por pérdida de radio funciona — pero
> **nunca apareció una luz verde**, porque el equipo se queda esperando selección de modo. **Eso no
> confirma N-42 ni la descarta**: es un `ABORTADO`, y un `ABORTADO` no dice nada del firmware.
>
> ⚠️ **Antes de montar otra sesión, resuelva la pregunta que abre el apartado «Orden de la sesión»:**
> si los dos comandos de aquí abajo arrancan el ciclo **por el USB-TTL**, N-42 se puede ejercer sin
> esperar al Bluetooth, y este bloque deja de depender de cuatro cosas que fallaron.

### El síntoma

Al cargar en las dos tarjetas: el enlace de radio va bien, pero **el ciclo del Modo Automático no
mueve las luces**.

### Cómo se arranca el ciclo hoy

Ya no hay botón. Por el terminal a 9600, en el **Maestro**:

```text
CMD:PIN:1234:SET_TIEMPOS:3,3,15      -> verde 3 min, rojo 3 min, despeje 15 s
CMD:PIN:1234:SET_MODO:AUTO
```

> 🛑 **AQUÍ PONÍA ~~`SET_TIEMPOS:1,1,15`~~ Y EL EQUIPO DE HOY LO RECHAZA — corregido el 05/09.**
> El mínimo vial subió de 1 a **3 minutos** por decisión del responsable (*«tres minutos es la
> mínima distancia de seguridad»*), y las seis constantes se mudaron de fichero con **N-137**:
> `Maestro/include/limites_ciclo.h:54-56` — `VERDE_MIN_MIN = 3`, `ROJO_MIN_MIN = 3`,
> `DESPEJE_SEG_MIN = 10`. Un `1,1,15` contesta **`$ERR,CMD:SET_TIEMPOS,DESC:RANGO`**
> (`modo_automatico.cpp:129`, traducido en `bluetooth.cpp:673`) **y el ciclo no arranca**.
>
> **Es el PRIMER comando del bloque que manda**, así que quien bajara al banco con la versión
> anterior se encontraba un `$ERR` de entrada — y el motivo, `RANGO`, no dice *«el documento está
> viejo»*. Se conserva tachado porque es la línea que se llevó al banco del 3-4/09.
>
> **Y la Guía de banco ya lo listaba en la columna contraria**, sin que nadie cruzara los dos
> ficheros: `Guia_Cableado_y_Pruebas_Banco.html:1793` → *«`DEBE RECHAZARSE` ·
> `CMD:PIN:1234:SET_TIEMPOS:1,1,15` · el limite VIEJO»*.

**Ese `3,3,15` es verde en MINUTOS, rojo en MINUTOS y despeje en SEGUNDOS**, en ese orden
(`modoAutomatico_fijarTiempos(verdeMin, rojoMin, despejeSeg)`,
~~`Maestro/src/modo_automatico.cpp:38`~~ → **`:128`** — re-medido el 05/09).

⚠️ **Y el coste va escrito porque cambia la agenda de la sesión:** un ciclo de 3+3 min con 15 s de
despeje son **6,5 minutos**. Ya **no se puede probar en mesa con ciclos de un minuto**; para ver
conmutar las luces hay que esperar, no basta con mirar un momento. El propio fuente lo declara:
*«COSTE DECLARADO Y ACEPTADO A SABIENDAS… un banco cae del lado de esperar tres minutos, no del
lado de dejar el limite de laboratorio suelto en una carretera»* (`limites_ciclo.h:44-46`).

Los tiempos **no se pueden cambiar con el ciclo en marcha**: contesta
`EN_MARCHA_PARE_EL_MODO`, y ese rechazo es correcto.

### La primera carga es el ancla, y su único trabajo es FUNCIONAR

**Si el ancla no arranca el ciclo, pare y replantee antes de gastar la sesión.** Una búsqueda cuyo
extremo *«bueno»* no es bueno **no acota nada — sólo consume cargas**, y cada carga es un SWD con
sus reintentos.

**Y compare por hash, nunca por tamaño.** Dos binarios del mismo tamaño pueden ser distintos, y dos
de nombres distintos pueden ser el mismo fichero. Un `md5` antes de cada carga ahorra cargas enteras.

### Qué anotar en cada carga

- `md5` del binario cargado.
- ¿Arranca el ciclo? ¿Las luces conmutan o se quedan fijas?
- Si se quedan fijas: **¿en qué color?** y ¿el equipo sigue contestando por el terminal?
- Lo que dice `$STATUS`: los campos `MODO:` y `ESTADO:`.

> ⚠️ **Un sospechoso se elige por ficheros, no por el mensaje del commit.** Si hay que buscar el
> cambio culpable, se acota con `git log` **sobre los ficheros del camino que falla**
> (`coordinador.cpp`, `modo_automatico.cpp`, `semaforo.cpp`), no leyendo resúmenes de commit: los
> resúmenes describen la intención, no el alcance.

---

## 2 · Carga por SWD — cómo entrar cuando el micro no se deja

**`mode=UR` con `-e all`. No se cambia el modo.**

```text
--connect port=swd mode=UR -e all --write firmware.bin -rst
```

`HOTPLUG` se engancha al micro **en marcha**: con un firmware que se cuelga al arrancar, el watchdog
reinicia cada 4 s en mitad del borrado y aparece `failed to erase memory`. **El delator de haberlo
hecho mal es ver `NVM size: 128 KBytes (default)` en un chip de 64 KB.**

**Si falla con `Unable to get core ID`: REINTENTE. No cambie el modo.** Enganchar es cuestión de
milisegundos y puede fallar dos o tres veces seguidas. **Eso no es falta de cableado.**

### Si tras varios reintentos sigue sin entrar

Método fiable, sin depender del tiempo de reacción de la mano:

1. Mueva el puente `BOOT0` de `0` a `1` (a 3,3 V).
2. Pulse `RESET` una vez.
3. Lance la carga. El micro arranca en su bootloader de fábrica e **ignora por completo el firmware
   colgado en Flash**, así que entra a la primera.
4. Devuelva `BOOT0` a `0` y pulse `RESET`.

> El método de *«mantener pulsado el reset y soltarlo un segundo después»* funciona a veces y falla
> muchas: el instante útil dura milisegundos. **Si falla dos veces, use `BOOT0`: es determinista.**

---

## 3 · Cómo se opera el equipo ahora: por el puerto serie de `J17`

**Es la única vía de operación que queda**, y hoy sólo hay una forma de usarla:

| vía | disponible hoy |
|---|---|
| **Adaptador USB-TTL a 9600** directo a `J17` — su RX a p3, su TX a p2, masa a p7 o p9 | ✅ **Sí.** Y el paso 5 lo confirmó en cobre: p3 -> pata 42, p2 -> pata 43 |
| ESP32 + Bluetooth SPP + app del móvil | ~~❌ **No.** La placa no existe~~ → 🟡 **la placa YA EXISTE** ~~(paso 22: fuente conmutada 12 V -> 5 V, `DS3231` por `GPIO21`/`GPIO22`, salida a `J17`)~~ 🛑 **corregido el 05/09: el «paso 22» es un PLANO, no un acta** —se titula *«la placa del módulo: cómo tiene que ser»*— **y el `DS3231` no está comprado.** Lo que sí es resultado: **el montaje definitivo funcionó igual que el de mesa (paso 24)** y la **masa común medida en `0 V` (paso 23)**. **Lo que no subió fue el enlace** — N-117 y N-122, arreglados después de la sesión y **sin banco todavía** |

**Anote siempre por cuál lo hizo.** Una orden que llega por USB-TTL demuestra que el firmware del
STM32 la entiende; **no demuestra nada del ESP32, ni del Bluetooth, ni de la app.**

**El censo completo de órdenes está en el §0.2 del Protocolo**, sacado del fuente y no de memoria.
No se copia aquí para no crear dos listas que alguien tendría que sincronizar.

> ⚠️ **Y una nota de lectura que evita reportar averías que no lo son.** En el `$STATUS`, **`BAT:12.6`
> en las dos puntas, y `RF:98%`, `RTT:85ms` y `MODO:SUBORDINADO` en el Esclavo, son literales fijos
> del código, no medidas.** No se usan para decidir nada. Consecuencia concreta: **desde el `$STATUS`
> del Esclavo no se puede saber si está en Modo Degradado.**

---

## 4 · ~~El mando sin receptor: cómo se ejerce, y cuándo NO se ejerce~~ → 🛑 **NO SE EJERCE. La rama de abajo se cumplió**

> 🛑 **ESTE APARTADO ESTÁ TACHADO EN SU PREMISA, y no se borra: es el que predijo lo que pasó.** Lo
> que sigue da por hecho que el mando se pulsa **contra masa**, porque el `botones.cpp` de `617bd00`
> lo leía en `INPUT_PULLUP` y activo en BAJO. **El cobre dice lo contrario (N-118): los cuatro pines
> llevan pull-down de 10 kΩ y 3,3 V en la posición de al lado — el gesto es contra los 3,3 V, activo
> en ALTO.** Aquel fuente estaba invertido en `A` y `B`, y con el pin ya en 0,6 V en reposo **no
> había transición que detectar: el mando no se pudo pulsar.**
>
> 🟢 **ESTADO DEL FIRMWARE HOY (04/09), separado del defecto de arriba:** `346ea5f` deja `MANDO_A` y
> `MANDO_B` en **`INPUT` pelado y `== HIGH` en las dos puntas** (`Maestro/src/botones.cpp:40`,
> `:160-161`; `Esclavo/src/botones.cpp:54`, `:178-179`). 🔴 **PENDIENTE de ejercer en tarjeta:**
> nadie ha visto todavía a este equipo obedecer un `A·A·A`. **El fuente ya no es el bloqueante; la
> carga verificada sí.**
>
> ~~**Y el propio apartado tenía escrita esa rama, dos párrafos más abajo:** *«Si dan ~10 kΩ contra
> masa → el pull-up interno no puede ganarles, el pin queda permanentemente en BAJO y el mando está
> inoperante de fábrica. En ese caso el Protocolo §8 y §9 no se ejecutan: se anotan los números del
> paso 20 y eso es el hallazgo.»* **Dieron 9,92 kΩ. Es exactamente esa rama, y eso ES el hallazgo.**~~
>
> 🔧 **TACHADO EL MISMO 04/09 POR `N-118`, y se conserva porque es la frase que resucita el gesto
> malo.** La rama estaba escrita **suponiendo un firmware con pull-up interno**, y **ése era el
> defecto, no el criterio**. Con `346ea5f` los cuatro pines de `J16` se leen igual —`INPUT` pelado,
> **activo en ALTO**—, y esos 10 kΩ pasan a ser **la resistencia de reposo que hace falta**: es
> exactamente lo que ya ocurría en las cámaras `C` y `D`, medidas a `0 V`. **El mando NO está
> inoperante de fábrica.** Lo que sigue impidiendo ejecutar §8 y §9 es **otra cosa y hay que decirla
> por su nombre: no hay tarjeta sana** (N-116).
>
> 🛑 **Lo que NO se hizo bien, y hay que decirlo porque estaba escrito aquí:** el paso 29 se ejecutó
> **igualmente**, con el resultado del paso 20 ya delante. El puente no produjo ningún cambio —lo
> previsto— **y el STM32 del Maestro se calentó**. Hoy esa placa tiene un corto medido entre 3,3 V y
> GND (**N-116**). La instrucción de no ejecutar §8 y §9 con ese resultado **no era prudencia
> genérica: era ésta.**
>
> ✅ **Para la próxima sesión: la primera mitad de esa condición ya se cumple** —`botones.cpp` lee
> `INPUT` pelado y `HIGH` en las dos puntas desde `346ea5f`—, así que **el puente sí se repite, pero
> con DOS cambios que no son opcionales:**
> 1. **El gesto es contra los 3,3 V del pin contiguo: `p5`–`p4` para `A`, `p8`–`p7` para `B`.**
>    ~~Contra masa~~ es el gesto del paso 29, el que dejó al Maestro caliente.
> 2. 🛑 **No sobre la tarjeta Maestro** (N-116, corto medido entre 3,3 V y GND).

**El receptor de radio del mando nunca se compró** —y desde el 04/09 **ya se pide, con salida `NO`**
(Manual 15, línea `A9`). ~~Pero los dos canales que quedan son pines de entrada de la propia
tarjeta, leídos en `INPUT_PULLUP` y **activos en BAJO**:~~ → **medido activo en ALTO; el firmware
era el que estaba al revés (N-118), y se corrigió en `346ea5f`.**

```text
MANDO A  =  BOTON1  =  PB9   =  J16 p5      <- MEDIDO: 9,92 kOhm a masa, 0,6 V en reposo
MANDO B  =  BOTON2  =  PB13  =  J16 p8      <- MEDIDO: 9,92 kOhm a masa, 0,6 V en reposo
masa                          =  J16 p2     <- UNA sola en todo el conector: NUNCA es el gesto
3,3 V                         =  J16 p4 (para A) y p7 (para B)   <- el gesto va CONTRA esto

   El 0,6 V de arriba se midio con 617bd00 dentro, y lo ponia el INPUT_PULLUP de aquel
   firmware contra esos 10 kOhm.  Con 346ea5f (INPUT pelado) el reposo es 0 V, como en
   las camaras C y D.  La resistencia siempre estuvo bien; la polaridad, no.
```

~~**Un pulso `A` es tocar un instante `J16` p5 contra masa con un cable suelto.**~~ → 🛑 **No. Eso es
lo que se hizo en el paso 29 y es lo que acabó con el Maestro caliente.** ~~Cuando el firmware se
corrija~~ → **el firmware ya está corregido (`346ea5f`): el pulso `A` es tocar un instante `p5`
contra `p4` (3,3 V), y el `B`, `p8` contra `p7`** — y aun así **no sobre la Maestro** hasta que
N-116 esté diagnosticado. Es el mismo recurso que la Guía usa en su paso 19 para hacer de cámara con un
pulsador, **que sí funcionó** (paso 21), porque la cámara ya lee en la polaridad correcta.

> ## 👁️ NO HACE FALTA NINGÚN INSTRUMENTO PARA SABER SI EL MANDO OYÓ: EL EQUIPO LO CONFIRMA CON SUS PROPIAS LUCES
>
> **MEDIDO en `01_Firmware/Maestro/src/mando.cpp:45-47`** (`DESTELLOS_AUTOMATICO = 2`,
> `DESTELLOS_AMBAR = 3`, `DESTELLOS_DEGRADADO = 4`):
>
> ```text
>   A . A . A       (<= 12 s)   ->  2 destellos ROJOS   Automatico
>   B . B . B       (<= 12 s)   ->  3 destellos ROJOS   Ambar intermitente
>   A . B . A . B   (<= 18 s)   ->  4 destellos ROJOS   Modo Degradado
>   rechazado                   ->  ambar rapido de 2 s
> ```
>
> **Se ve DESDE EL SUELO: sin app, sin cable y sin segunda tarjeta.** Está diseñado así a propósito
> —quien acciona el mando está a 5 m y sin pantalla—, y los destellos son **siempre rojos** porque
> el rojo nunca significa *pase*: si el operario cuenta mal, el peor caso sigue siendo seguro. **Un
> rechazo no habla el mismo idioma que un éxito:** es un ámbar rápido de 2 s, no destellos.
>
> **Anote los destellos CONTADOS, no «funcionó».** Es el dato de vuelta de esta prueba.
>
> ⚠️ **Y la trampa que hay que conocer antes de dar el primer pulso: pruebe DESDE OTRO MODO.** Si el
> equipo **ya está** en el modo que la secuencia pide, `MODO:` no cambia — `mando.cpp` entra por la
> rama `if (modoActual_get() == MODO_AUTOMATICO) modoAutomatico_setup();` y **el terminal no
> distingue nada**. Un `A·A·A` sobre un equipo que ya está en Automático puede haber funcionado
> perfectamente y no mover ni un campo del `$STATUS`. **Los destellos, en cambio, se ven siempre.**
>
> 🔵 **Por eso el USB-TTL baja de rango PARA ESTA PRUEBA.** Sigue siendo el recurso legítimo para
> operar el equipo cuando la app no conecta —que es lo que pasó en la sesión 1— y para todo lo
> demás de este encargo; pero **la respuesta sobre si el mando oyó son los destellos**, no la
> trama. Mirar sólo el terminal es cómo un mando sano se apunta como sordo.

> 🔴 **Antes de tocar `J16`, dos requisitos que no son opcionales:**
> 1. **Paso 4 de la Guía hecho: el pin de 12 V tapado.** En el cobre esos 12 V corren a **1,36 mm**
>    del más cercano de estos pines — no a los milímetros que se ven entre los pines del conector.
>    ✅ **HECHO el 3/09**, retirando el pin del cuerpo del conector volante. 🔴 **Y desde N-120 esto
>    sube: no es una precaución de banco, es obligatorio en cada equipo** — ninguna de las 5 entradas
>    de campo de la placa lleva nada en serie, mientras las 9 salidas llevan 220 Ω y opto.
> 2. ~~**Paso 20 de la Guía hecho, con su resultado delante:**~~
>    - ~~Si p5 y p8 dan **circuito abierto contra masa y ~3,3 V en reposo** → el puente funciona, y los
>      bloques 4 y 5 se ejecutan.~~
>    - ~~**Si dan ~10 kΩ contra masa** → el pull-up interno no puede ganarles, el pin queda
>      permanentemente en BAJO y **el mando está inoperante de fábrica**. En ese caso **el Protocolo
>      §8 y §9 no se ejecutan**: se anotan los números del paso 20 y **eso es el hallazgo**.~~
>      ~~🛑 **ESTA. Dieron 9,92 kΩ y 0,6 V. El mando está inoperante de fábrica, y ése es el hallazgo.**~~
>
>    🔧 **LAS DOS RAMAS CADUCARON EL 04/09 (`N-118`), y se tachan sin borrarse porque son las que
>    mandan al técnico a puentear contra masa.** Estaban escritas suponiendo un firmware con
>    `INPUT_PULLUP`, y ése era el defecto. El paso 20 ya midió lo que había que medir —**los cuatro
>    pines llevan ~10 kΩ a masa, y eso es CORRECTO para los cuatro**, porque los cuatro son activos
>    en ALTO desde `346ea5f`—: **la medida `M3` quedó cerrada y no hay bifurcación que resolver.**
>    **El requisito que SÍ queda es otro:** el firmware de N-118 **cargado y verificado** en la
>    tarjeta —no mergeado—, con su `md5` anotado **antes** de tocar `J16`. Con el firmware anterior
>    dentro, el puente no hace nada.
>
> ⚠️ **Lo que el puente NO demuestra, y va escrito al lado:** demuestra que **el firmware reconoce
> las secuencias**. No demuestra que los pulsos lleguen **desde el piso, por radio**, que es la
> condición real de uso — con su rebote de contacto y sus ~2 s por pulsación. Eso sigue sin
> receptor con el que probarlo, y queda aplazado (prueba 8.9 del Protocolo).

> ⚠️ **Y no confunda los dos pasos de la Guía que hablan de `J16`, porque hacen cosas distintas:**
> **el PASO 20 es la MEDIDA** —*«La medida que decide lo de `J16`»*—, y es la que decide; **el PASO 21
> es el que CABLEA** la cámara, y sólo se hace si el 20 salió bien. Cuando este encargo dice «paso
> 20» se refiere a la medida.

> 🔴 ~~**Un desacuerdo entre documentos que hay que resolver antes de la sesión, y no lo resuelve el
> técnico.**~~ → ✅ **RESUELTO POR LA MEDIDA, y el pronóstico se cumplió al pie de la letra.** El paso
> 20 de la Guía dice *«los cuatro tienen que dar lo mismo»*. Eso era cierto cuando p5, p8, p10 y p12
> eran los cuatro pulsadores. **Desde el 31/08 no lo es:** p10 y p12 son cámaras y las quieren en
> reposo a masa (activas en ALTO), mientras p5 y p8 son el mando, activo en BAJO. **Si los cuatro
> dieran lo mismo, una de las dos funciones estaría rota.**
>
> **Los cuatro dieron lo mismo** —9,92 / 9,92 / 9,93 / 9,94 kΩ— **y la función rota era el mando.**
> La placa trata igual a los cuatro pines, así que el firmware no puede leer dos al revés de los
> otros dos: o los cuatro son activos en ALTO, o ninguno. La cámara ya estaba corregida desde N-67;
> 🟢 **el mando lo está desde `346ea5f` (04/09), y con eso los CUATRO leen ya como pide el cobre.**
> 🔴 **Ninguna de las dos correcciones del mando se ha ejercido sobre una tarjeta.**
>
> **Nota de método, porque este documento la pedía y se cobró sola:** la frase de la Guía no era un
> error de redacción a corregir antes de bajar al banco — era **la comprobación que hacía visible el
> defecto**. Cambiarla para *«que cuadrara con lo esperado»* habría tapado N-118.

---

## 5 · El Modo Degradado, y el hueco que hay que mirar de frente

> ⛔ **NO SE EJECUTÓ NADA DE ESTE APARTADO el 3-4/09.** Dependía de los bloques 3 y 4, y los dos
> cayeron: el reloj quedó bloqueado por el Bluetooth (paso 27) y el mando se abortó por seguridad
> (paso 29). **Se conserva entero y sigue vigente para la próxima sesión** — con un cambio de peso:
> con el mando `A`/`B` inoperante (N-118) y sin comando por app, la tabla de vías de entrada y salida
> del Esclavo de más abajo **queda con una sola fila viva: la vuelta de la radio**. Eso no es una
> corrección del apartado; **es su advertencia, cumplida.**
>
> 🟢 **Lo único de este bloque que sí se midió, y salió bien: la regla de seguridad.** Ver justo
> debajo.

### La regla que se confirma antes que ninguna otra

**El equipo NUNCA entra solo al Modo Degradado.** Al perder la radio, el comportamiento correcto es
caer a **ámbar intermitente** en las dos puntas. Un ámbar le dice al conductor *«nadie controla este
cruce»*; un verde por reloj sin confirmar la otra punta le da confianza falsa. La entrada es **100 %
manual**.

> **Primera prueba, y es de seguridad (Protocolo 9.1):** corte la radio y compruebe que **ambas
> puntas caen a ámbar intermitente** y que **ninguna entra sola en Degradado**. Si alguna entra
> sola, **pare la sesión y repórtelo de inmediato.**
>
> ✅ **HECHA el 3/09 en el paso 8 de la Guía, y PASÓ.** Tres cortes de comunicación; en los tres, las
> dos puntas a **ámbar intermitente en ~20 s**, parpadeando hasta restablecer el enlace, y vuelta
> sola a rojo fijo en ~3 s al reconectar. **Ninguna entró sola en Degradado.**
>
> ⚠️ **Con un número que no cuadra del todo y no se redondea: la Guía espera 23-28 s y salieron ~20.**
> `SFTY6_SILENCIO_MS` son 25 000 ms. Va en la dirección segura —cae antes, no después— y el informe
> lo llama diferencia menor. **No está explicado**, y un umbral de seguridad que se dispara 5 s antes
> de lo calculado es exactamente la clase de cifra que en este repositorio acaba siendo otra cosa.
> Se cronometra otra vez con reloj, no a ojo, y se anota.

### 🔴 El Esclavo hoy sólo tiene una vía de entrada y de salida, y es el puente

**MEDIDO:** el Esclavo **no acepta ni un solo comando `SET_MODO:*`** por el puerto serie. Sumado a
que no hay pantalla y a que `botonAceptar()` es `false`:

| vía de entrada / salida del Degradado en el ESCLAVO | estado hoy |
|---|---|
| El mando (`A·B·A·B` para entrar, `A·A·A` o `B·B·B` para salir) | ~~**Viva** — pero sólo con el puente de `J16`, porque no hay receptor~~ · ~~🔴 **MUERTA, medido el 3/09 (N-118).** Sin receptor **y** con el pin a 0,6 V, el puente tampoco sirve: no hay flanco que registrar~~ → 🟡 **VIVA POR PUENTE, SIN EJERCER.** El `0,6 V` lo ponía el `INPUT_PULLUP` del propio firmware; con `346ea5f` (`INPUT` pelado, activo en ALTO) **el puente sí produce flanco** — dado **`p5` contra `p4` y `p8` contra `p7`**, los 3,3 V del pin contiguo, **NUNCA contra masa**. Confirmación: **4 destellos rojos** al entrar, 2 o 3 al salir. 🔴 Lo que falta es **una tarjeta sana** (N-116) y el **receptor**, que nunca se compró |
| La vuelta automática de la radio (deja de gobernar cuando el Maestro habla) | **Viva** |
| La pantalla (`Esclavo/src/menu.cpp:215`) | **Existe en el código y está MUERTA**: el `aceptar` que la dispara no puede ser cierto nunca |
| Por comando / app | **No existe** |

> **En claro: si el Esclavo entra en Degradado y no hay puente ni receptor, la única forma de sacarlo
> es que vuelva la radio.** Es un dato para el dictamen, no una opinión, y por eso el Protocolo lo
> mide y lo hace firmar en su prueba **9.15**.
>
> 🔴 **Y desde el 3/09 el «si» sobra: las cuatro filas están en una sola.** El puente no funciona
> (N-118), el receptor no existe, la pantalla está muerta y no hay comando. **La única vía viva de
> sacar al Esclavo del Modo Degradado es que vuelva la radio** — la que no depende de que nadie la
> pida. Eso sube **F4** de *«mejora pendiente»* a la barrera que falta.

### 🔴 Y el defecto abierto que esta sesión tiene que EJERCER, no rodear (N-106)

**El ámbar de emergencia mandado desde la app no saca al Esclavo del Modo Degradado.** Está razonado
por lectura del fuente y **nadie lo ha ejecutado nunca** — ni en banco ni en arnés. Las dos vías de
ámbar de emergencia no hacen lo mismo, y el firmware declara por escrito que sí:

| vía | ¿sale del Degradado? |
|---|---|
| Mando, `B·B·B` (`Esclavo/src/mando.cpp:129-142`) | **Sí** — comprueba `degradado_gobiernaLuz()` y llama a `degradado_salir()` |
| Serie, `CMD:AMBAR_EMERGENCIA` | **No** — `Esclavo/src/bluetooth.cpp` no nombra el Degradado en ninguna línea, ni incluye su cabecera |

Y lo agravante: se contesta `$ACK,...,RESULT:OK` igualmente, mientras el sostenedor del Degradado
sigue repintando la luz en cada vuelta. **El ámbar pedido desde el teléfono puede caerse solo justo
en el modo donde más falta hace.**

> **La prueba está escrita: es la 9.16 del Protocolo, y admite las tres salidas posibles** —defecto
> vivo, arreglo dentro y funcionando, o rechazo honesto—, porque el arreglo está en curso y puede
> estar dentro del binario que se cargue. **Lo que no vale es firmar esa casilla sin mirar las luces
> tres minutos:** un ámbar que aparece no demuestra que el ámbar se quede.

**Estado al redactar este encargo (31/08, por lectura del árbol de trabajo): el defecto sigue
abierto.** Verifíquelo contra el `md5` del binario que se lleve, no contra esta frase.

### Lo que hay que cronometrar en el ciclo

Verde 30 s por sentido, todo-rojo 30 s entre medias, ciclo completo 120 s. El checklist está en la
prueba **9.7** del Protocolo, y se marca **mirando las luces, no la telemetría**: cada unidad informa
de la fase que *ella* calcula, y si las dos calculan mal, las dos dirán que todo va bien.

> **Verde simultáneo en las dos puntas es la única forma en que este equipo puede matar a alguien.**
> Si lo observa: saque las dos puntas con `B·B·B`, corte la alimentación y **RECHACE**.

---

## 6 · Qué devolver

> ✅ **Devuelto el 04/09:** `evidencia/Informe_Pruebas_Banco_Semaforos_V9.0.pdf`, 11 páginas, paso por
> paso, con los cuatro hallazgos consolidados y **la causa del sobrecalentamiento declarada como no
> confirmada**. Los puntos 1, 3, 4, 6 y 7 de la lista de abajo están cubiertos; **el 2 y el 5 no**:
> el Protocolo no se llegó a ejecutar —depende del Modo Automático y de la app— y el veredicto del
> reloj quedó bloqueado con el paso 27.
>
> 🟡 **Lo que faltó, y es el punto 3:** el informe no trae el `md5` de los binarios cargados. No hizo
> daño esta vez porque **no hubo bisección** —no se llegó a arrancar el ciclo—, pero era el dato que
> ataba «lo que se cargó» con «lo que se midió». **Va en la próxima.**

1. **La Guía rellena**, con los tres cuadros de `ANOTA` de los 29 pasos — **también los que salieron
   bien**. Una casilla en blanco no dice nada.
2. **El Protocolo rellenado y firmado**, con su acta. Las pruebas sin casilla **no se firman**: si
   ejecutó alguna de ellas por su cuenta, anótelo en el cuadro de texto libre, no invente casilla.
3. **N-42:** `md5` de cada binario cargado y qué hizo cada uno. **Si el ancla falló, dígalo — es un
   resultado, no un fracaso.**
4. **Los números del paso 20 de la Guía**, tal cual, aunque no se entiendan. Deciden dos bloques.
5. **El veredicto del reloj en las DOS tarjetas** (una está diagnosticada, la otra no): la respuesta
   literal de `CMD:PIN:1234:REINICIAR_RELOJ` en cada una.
6. **La respuesta literal de cada rechazo provocado** (prueba 12.7). Un `$ACK` donde debía haber un
   `$ERR` es el fallo entero, aunque el equipo se comporte bien después.
7. **Lo que no se pudo probar, y por qué.**

> **No sustituya ningún componente en esta sesión.** El objetivo es traer la lectura. Ya se dio por
> culpable a un cristal una vez apoyándose en una pantalla que acusaba **sin haber medido nada**, y
> se mandaron a cambiar componentes sanos. **La reparación se decide después, con el dato delante.**

---

## 🛒 Lo que hace falta COMPRAR o CONSTRUIR antes de que esta sesión se pueda ejecutar entera

Con lo que hay hoy se pueden ejecutar **52 de las 86 pruebas** del Protocolo. Las **34 restantes**
esperan a esto — y algunas esperan a una decisión, no a una compra.

> 🔴 **Y una advertencia sobre esa cifra, del 04/09: de las 52 no se ejecutó NINGUNA.** La sesión del
> 3-4/09 recorrió **la Guía**, no el Protocolo — que va después y depende del Modo Automático y de la
> app. **`52 ejecutables` sigue queriendo decir *«nada lo impide en principio»*, no *«52 hechas»*.**
> El acta del Protocolo sigue **sin firmar y sin empezar**.

### Hardware que hay que comprar

| # | Qué | Desbloquea | Ojo |
|---|---|---|---|
| **H1** | **Receptor de radio del mando de relés** (uno por punta si se quiere en las dos) | Prueba **8.9**, y convierte todo el bloque 4/5 de *«ejercido con un cable»* a *«ejercido como se usa»* | 🔴 **No se compra hasta decidir D1** — **y desde el 3/09, tampoco hasta arreglar N-118**: hoy el firmware no puede leer un pulso en `A`/`B` **venga de un cable o de un receptor**. Comprarlo ahora es comprar un mando que el equipo no oye |
| **H2** | **Fuente conmutada DC-DC 12 V → 5 V, ≥ 1 A**, con fusible y protección de polaridad inversa | Toda la §12 del Protocolo | **Conmutada, no lineal.** Un lineal desde 12 V disiparía más de 4 W en un armario cerrado y al sol, y cae de tensión justo cuando el módulo tira del pico — el síntoma parece un problema de programa. **La referencia concreta no está elegida** |
| **H3** | **Módulo de reloj `DS3231`** con su pila | Prueba **12.6** (Courier RTC) | ~~✅ **comprado y montado** sobre la placa C1 (paso 22), conexiones I2C correctas.~~ 🛑 **FALSO, y se tacha en vez de borrarse — 05/09. NO ESTÁ COMPRADO** (línea `A6` de la lista de compras), así que tampoco está montado ni tiene conexiones que juzgar. **El error es citar un PROTOCOLO como RESULTADO** (`CLAUDE.md` §2.ter): el «paso 22» de la Guía se titula *«la placa del módulo: **cómo tiene que ser**»* — es el plano de cómo debe quedar, no el acta de que quedó. 🔴 **Sigue sin verificarse que dé la hora**, y ahora por dos motivos y no uno: **no hay pieza**, y aunque la hubiera la única vía de consultarla es `SET_RTC` por Bluetooth (paso 27, bloqueado). **Sin el módulo, el hueco de hora sale como `--:--:--`: eso es el firmware callándose bien, no una avería.** Y sigue en pie el aviso **para cuando se compre**: si trae **`CR2032` en vez de `LIR2032`**, hay que desoldar el diodo o la resistencia de su circuito de carga — **la `CR2032` no es recargable y ese circuito la calienta** |
| **H4** | **Dos cámaras de demanda** *(confirmar antes si ya hay una en almacén)* | Prueba **11.3** | Contacto seco **normalmente abierto**. Se cablean a `J14`, que está probado — **no** a `J16` hasta que el paso 20 lo permita |
| **H5** | **Dos radios más, con antenas y coaxiales**, y la placa del repetidor con su `MAX3485` | La §6 entera del Protocolo (6 pruebas) | 🔴 **No se compra hasta decidir D2.** La topología vigente es de **2 radios en enlace directo, sin repetidor**: hoy esas 6 pruebas certifican algo que no se está desplegando |

### Lo que hay que construir

| # | Qué | Desbloquea | Estado |
|---|---|---|---|
| **C1** | **La placa del módulo ESP32** — con sus dos tiras de conector hembra, la fuente H2 y el reloj H3 encima, y el USB del módulo accesible | Pruebas **12.1**, **12.5**, **12.6**, y el uso de la app en toda la sesión | ~~🔴 **No está diseñada, ni fabricada, ni medida.**~~ → ✅ **CONSTRUIDA el 4/09**, y **cada mitad de esta casilla con su nivel al lado, porque antes iban mezcladas**: 🟢 **RESULTADOS de banco** — masa común medida en **0 V** (paso 23), montaje encendido **sin calentamiento ni reinicios** (paso 24), módulo confirmado de **30 pines**. 🔵 **PLANO, no resultado** — ~~`DS3231` por `GPIO21`/`GPIO22` (paso 22)~~ 🛑 **el «paso 22» se titula *«cómo tiene que ser»* y el `DS3231` NO ESTÁ COMPRADO (`A6`): el reloj NO está en esta placa.** La fuente conmutada 12 V -> 5 V **sí está montada**. 🟡 **Falta una cosa más: esa fuente no se midió con carga** — en banco se alimentó por USB. **Antes de campo, no antes de la próxima sesión** |
| **C2** | **Cargar el firmware del ESP32** en el módulo | Ídem | ~~El firmware **existe** y **no está cargado**~~ → ✅ **CARGADO, y compiló sin errores.** 🔴 **Y aun así el módulo no se anunció de forma fiable en el teléfono**: eran N-117 (el watchdog se comía el arranque) y N-122 (la app no llamaba a `connect()`), **los dos arreglados después de la sesión y ninguno verificado en banco**. La nota de abajo sigue valiendo: un módulo que aparece y no reenvía nada **no es una avería del módulo y no se cambia** |

### Firmware que falta, y no es una compra

| # | Qué | Desbloquea |
|---|---|---|
| **F1** | **Exponer en el `$STATUS` del Esclavo su modo real**, en vez del literal fijo `MODO:SUBORDINADO` | Hoy **no hay forma de saber desde fuera si el Esclavo está en Degradado** — el dato que cambia el significado de todo lo demás. Recupera lo que daba el aviso del pie de su menú (§10 retirada) |
| **F2** | **Añadir al `$STATUS` la antigüedad de la última sincronización** | Pruebas **7.10** y **9.13**, hoy imposibles de observar: en una hora la deriva es ~0,36 s, así que comprobarlo por las horas sería una prueba que **no puede fallar** |
| **F3** | **Añadir al `$STATUS` los contadores de línea RS-485** | Prueba **5.7**, retirada. Distinguía *no llega nada* de *llega y es basura* — la segunda apunta a cableado o radio con ruido, y evitaba desplazamientos al poste con instrumentos |
| **F4** | **Una vía de entrada y salida del Modo Degradado en el Esclavo por comando** | Prueba **9.15**. Hoy, sin puente ni receptor, **sólo la vuelta de la radio saca al Esclavo del modo** |
| **F5** | **El arreglo de N-106**, con su arnés visto fallar primero | Prueba **9.16**. El orden importa: el arnés tiene que **fallar sobre el firmware de hoy** antes de que nadie toque una línea, o el arreglo entra sin testigo |

### Decisiones del responsable — bloquean compras y no las desbloquea ninguna medida

| # | Qué hay que decidir | Bloquea |
|---|---|---|
| **D1** | **Si los mandos de las dos puntas llevan el mismo código o códigos distintos** (N-19). Un solo código metería las dos torres en Degradado a la vez desde el piso, **saltándose la verificación de cada punta**; códigos distintos obligan a comprobar torre por torre | **H1.** No se compra receptor sin esto |
| **D2** | **Si se certifica la topología con repetidor** o se cierra en el enlace directo de 2 radios | **H5** y la §6 entera |
| **D3** | **Qué debe hacer el ámbar de la app cuando el Esclavo está en Degradado**: ¿salir ordenado como el `B·B·B`, o quedarse en ámbar sin salir? | **F5.** Es lo que ve un conductor |
| **D4** | **Si el Esclavo debe seguir aceptando `SET_RTC`.** Hoy lo acepta y contesta `RESULT:OK`, mientras los manuales dicen que la hora se cuadra **sólo** en el Maestro y viaja por radio. Aceptarlo es lo que hace posible el Courier RTC sin radio; **las dos cosas no pueden ser ciertas a la vez** | La redacción de la §7 del Protocolo y del Manual de Usuario |
| **D5** | **Si la entrada al Modo Degradado debe recuperar su doble confirmación.** La tenía en la pantalla (`Botón 3` → `CONFIRMAR ENTRADA?` → `Botón 3`) contra una sola pulsación para salir: **lo peligroso difícil, lo seguro fácil.** Hoy el Maestro entra con **un solo comando** | La §9 del Protocolo |
| **D6** | **Si se redefinen las secuencias del mando** (N-53). Las reales son `A·A·A`, `B·B·B` y `A·B·A·B`; la redefinición a `A·B·A` / `B·A·B` / `B·A·B·A` **nunca se implementó**, aunque documentos internos la daban por hecha | Manual de Usuario, §8 del Protocolo y el adiestramiento del operario |
| **D7** 🆕 | **N-120 — si V2 le pone protección a las entradas de campo.** La placa protege sus **9 salidas** con 220 Ω + opto `TLP127` y **no protege ninguna de sus 5 entradas**: del borne directo a la pata del micro, en el mismo conector que reparte 12 V crudos. La propuesta es **2K2 en serie por entrada**: deja la inyección en **3,6 mA** (bajo los 5 mA del datasheet) y el nivel alto en **2,70 V** (sobre los **2,31 V** de `VIH`). **Con 4K7 ya no leería la cámara** — el margen es de un salto de valor | El rediseño de placa de V2. **No lo desbloquea ninguna medida**: es coste de rediseño contra riesgo. Cuenta y detalle en el manual **17 §3.6** |
| **D8** 🆕 | **N-116 — qué se hace con la tarjeta Maestro.** Tiene un corto **medido** entre 3,3 V y GND: arranca, aguanta ~30 s y se calienta. La causa está **abierta**; el firmware queda descartado por censo —ninguna de las 9 salidas toca un pin de `J16`— | La próxima sesión entera. **No se reenergiza** hasta inspección técnica en frío, y **hasta entonces sólo hay una tarjeta sana para probar** |

---

> **Nada de esta versión sube a un cruce abierto al tráfico hasta que esta sesión esté hecha y sus
> resultados revisados.** Y ni siquiera entonces lo autoriza este documento: lo autoriza el
> responsable, con el acta firmada delante.
>
> 🛑 **La sesión está hecha, y esta frase NO se levanta.** ~~24 de 29 pasos~~ → **la cuenta no se
> publica** (ver la cabecera de este documento): lo verificado son buenas noticias sobre el
> cableado, la radio, la talanquera y las cámaras — **y ninguna sobre lo que este equipo hace en un
> cruce**. El ciclo no se vio moverse ni una vez, el Modo Degradado no se tocó, el Protocolo no se
> empezó, hay una tarjeta con un corto y el único mando físico del equipo **resultó no ser pulsable
> con el firmware que se llevó al banco**. **Lo que corre en campo sigue siendo la V8.4.**
</content>
