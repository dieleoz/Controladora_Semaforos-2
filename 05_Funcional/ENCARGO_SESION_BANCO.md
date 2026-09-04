# ENCARGO — Sesión de banco · Controladora de Semáforos

**Redactado:** 31 de Agosto de 2026 · **Ejecutado:** 3 y 4 de Septiembre de 2026 · **Rama:** `main-nuevo`

---

## ✅ ESTE ENCARGO YA SE EJECUTÓ — 3 y 4 de septiembre de 2026

**Se ejecutó sobre el paquete V9.0, commit `617bd00`.** El informe es
[`evidencia/Informe_Pruebas_Banco_Semaforos_V9.0.pdf`](../evidencia/Informe_Pruebas_Banco_Semaforos_V9.0.pdf)
— preparado por Sebastián, IT Vial SAS. **Lo que sigue en este documento ya no es una tarea: es el
guion de lo que se hizo, con lo que quedó fuera marcado.**

**El resultado, y va delante de todo lo demás: de los 29 pasos de la Guía se completaron 24, cuatro
quedaron bloqueados y uno se abortó por seguridad.**

| | cuántos | cuáles |
|---|---|---|
| **Completos** | **24 / 29** | pasos 1-6, 8-9, 15-18, 20-24 |
| 🔴 **Bloqueados** por el enlace Bluetooth | **4 / 29** | pasos **25, 26, 27 y 28** |
| 🛑 **Abortado por seguridad** | **1 / 29** | paso **29** (mando de relés) |

> ⚠️ **Y tres de esos 24 son PARCIALES, con su motivo escrito: los pasos 7, 19 y 21.** La parte de
> cableado y de medida se verificó; la respuesta funcional del semáforo **no**, porque depende del
> Modo Automático y el modo no se llegó a seleccionar —el informe lo atribuye a que *«sólo se puede
> seleccionar desde la app»*; **eso no está medido y este documento lo deja abierto**, ver el
> apartado «Orden de la sesión»—. Se cuentan dentro de los 24 por su parte verificable — **no por la
> que falta**.

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
> 🔴 **Todo lo de arriba sigue vigente palabra por palabra: la sesión NO fue un visto bueno.** Con
> 24 de 29 pasos hechos, **la vía principal de operación del equipo no se verificó ni una vez**, y
> **N-42 sigue sin confirmar ni descartar**. Nada de esto sube a un cruce.

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
| **`botonAceptar()` y `botonCancelar()` devuelven `false` siempre.** Los pulsadores 3 y 4 dejaron de existir: sus pines pasan a ser entradas de cámara | `Maestro/src/botones.cpp:280-281` · `Esclavo/src/botones.cpp:294-295` | **No hay forma de aceptar ni cancelar nada, ni de abrir un menú.** Toda la operación pasa por comandos |
| **La pantalla no se retira, pero deja de conducir sus pines.** `PB3`/`PB4`/`PB5` quedan en alta impedancia porque comparten `J17` con el ESP32 | `Maestro/src/lcd.cpp:74-75` · `Esclavo/src/lcd.cpp:92-93` | **No hay imagen.** Nada de lo que se leía en pantalla se puede leer. **No es una avería: no lleve una pantalla de repuesto** |
| **El mando de relés SE CONSERVA**, en `A` = `PB9` = `J16` p5 y `B` = `PB13` = `J16` p8 | `Maestro/src/mando.cpp:202-235` · `Esclavo/src/mando.cpp:218-250` | `A·A·A`, `B·B·B` y `A·B·A·B` siguen en el firmware. **Pero el receptor de radio nunca se compró**: ~~hoy sólo se pueden inyectar los pulsos con un cable~~ → 🔴 **MEDIDO EL 3/09: hoy no se pueden inyectar de ninguna forma.** El pin está en 0,6 V en reposo y `botones.cpp` lo lee activo en BAJO: nunca hay flanco (**N-118**). El mando existe en el código y **no existe en la mano** |
| **El umbral de silencio son 25 s, no 12** | `SFTY6_SILENCIO_MS = 25000UL` — `Maestro/include/protocolo.h:149` y `Esclavo/include/protocolo.h:149` | Al cortar la radio se cronometran **~25 s**. Si sale alrededor de 12, **el firmware cargado no es el nuevo**. ✅ **Medido: ~20 s en tres cortes** — es el firmware nuevo, y la diferencia va en la dirección segura, pero **no está explicada**: se vuelve a cronometrar con reloj |
| **`VENTANA_TRIPLE_MS = 12000` sigue siendo 12 s, y es correcto** | `Maestro/src/mando.cpp:38` · `Esclavo/src/mando.cpp:42` | Es la ventana para encadenar `A·A·A` o `B·B·B`. **No son los 25 s de arriba: son cosas distintas y no se confunden** |
| **El ESP32 de expansión tiene firmware nuevo**, va en `J17`, con Bluetooth SPP y reloj `DS3231` | `01_Firmware/ESP32_Expansion/` | ~~**La placa que lo lleva NO EXISTE**, ni su fuente está pedida~~ → ✅ **construida y montada el 4/09** (paso 22), con su fuente y su `DS3231`. **Módulo confirmado `ESP32-WROOM-32` clásico** — BR/EDR, el perfil que el SPP necesita. 🔴 **Lo que faltó fue el enlace**, no el hardware |

*(Los `fichero:linea` de arriba son **MEDIDOS**. Los de `bluetooth.cpp` se citan en el Protocolo por
**literal y sin número de línea** a propósito: esos dos ficheros los está tocando otro trabajo en el
mismo árbol, y una línea citada que se mueve manda al lector a un sitio que no dice lo que promete.)*

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
> de este mismo encargo dice cómo arrancar el ciclo por el terminal USB-TTL** —`CMD:PIN:1234:SET_TIEMPOS:1,1,15`
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
CMD:PIN:1234:SET_TIEMPOS:1,1,15      -> verde 1 min, rojo 1 min, despeje 15 s
CMD:PIN:1234:SET_MODO:AUTO
```

**Ese `1,1,15` es verde en MINUTOS, rojo en MINUTOS y despeje en SEGUNDOS**, en ese orden
(`modoAutomatico_fijarTiempos(verdeMin, rojoMin, despejeSeg)`, `Maestro/src/modo_automatico.cpp:38`
— MEDIDO). Los tiempos **no se pueden cambiar con el ciclo en marcha**: contesta
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
| ESP32 + Bluetooth SPP + app del móvil | ~~❌ **No.** La placa no existe~~ → 🟡 **la placa YA EXISTE** (paso 22: fuente conmutada 12 V -> 5 V, `DS3231` por `GPIO21`/`GPIO22`, salida a `J17`), y el montaje definitivo funcionó igual que el de mesa (paso 24). **Lo que no subió fue el enlace** — N-117 y N-122, arreglados después de la sesión y **sin banco todavía** |

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
> que sigue da por hecho que el mando se pulsa **contra masa**, porque `botones.cpp` lo lee en
> `INPUT_PULLUP` y activo en BAJO. **El cobre dice lo contrario (N-118): los cuatro pines llevan
> pull-down de 10 kΩ y 3,3 V en la posición de al lado — el gesto es contra los 3,3 V, activo en
> ALTO.** El fuente está invertido en `A` y `B`, y con el pin ya en 0,6 V en reposo **no hay
> transición que detectar: el mando no se puede pulsar.**
>
> **Y el propio apartado tenía escrita esa rama, dos párrafos más abajo:** *«Si dan ~10 kΩ contra
> masa → el pull-up interno no puede ganarles, el pin queda permanentemente en BAJO y el mando está
> inoperante de fábrica. En ese caso el Protocolo §8 y §9 no se ejecutan: se anotan los números del
> paso 20 y eso es el hallazgo.»* **Dieron 9,92 kΩ. Es exactamente esa rama, y eso ES el hallazgo.**
>
> 🛑 **Lo que NO se hizo bien, y hay que decirlo porque estaba escrito aquí:** el paso 29 se ejecutó
> **igualmente**, con el resultado del paso 20 ya delante. El puente no produjo ningún cambio —lo
> previsto— **y el STM32 del Maestro se calentó**. Hoy esa placa tiene un corto medido entre 3,3 V y
> GND (**N-116**). La instrucción de no ejecutar §8 y §9 con ese resultado **no era prudencia
> genérica: era ésta.**
>
> **Para la próxima sesión: no se repite el puente hasta que `botones.cpp` lea `INPUT` pelado y
> `HIGH` en las dos puntas, y no se repite sobre la tarjeta Maestro.**

**El receptor de radio del mando nunca se compró.** ~~Pero los dos canales que quedan son pines de
entrada de la propia tarjeta, leídos en `INPUT_PULLUP` y **activos en BAJO**:~~ → **medido activo en
ALTO; el firmware es el que está al revés (N-118).**

```text
MANDO A  =  BOTON1  =  PB9   =  J16 p5      <- MEDIDO: 9,92 kOhm a masa, 0,6 V en reposo
MANDO B  =  BOTON2  =  PB13  =  J16 p8      <- MEDIDO: 9,92 kOhm a masa, 0,6 V en reposo
masa                          =  J16 p2
3,3 V                         =  J16 p4 (para A) y p7 (para B)   <- el gesto va CONTRA esto
```

~~**Un pulso `A` es tocar un instante `J16` p5 contra masa con un cable suelto.**~~ → 🛑 **No. Eso es
lo que se hizo en el paso 29 y es lo que acabó con el Maestro caliente.** Cuando el firmware se
corrija, el pulso será tocar `p5` contra **`p4` (3,3 V)** — y aun así **no antes** de que N-116 esté
diagnosticado. Es el mismo recurso que la Guía usa en su paso 19 para hacer de cámara con un
pulsador, **que sí funcionó** (paso 21), porque la cámara ya lee en la polaridad correcta.

> 🔴 **Antes de tocar `J16`, dos requisitos que no son opcionales:**
> 1. **Paso 4 de la Guía hecho: el pin de 12 V tapado.** En el cobre esos 12 V corren a **1,36 mm**
>    del más cercano de estos pines — no a los milímetros que se ven entre los pines del conector.
>    ✅ **HECHO el 3/09**, retirando el pin del cuerpo del conector volante. 🔴 **Y desde N-120 esto
>    sube: no es una precaución de banco, es obligatorio en cada equipo** — ninguna de las 5 entradas
>    de campo de la placa lleva nada en serie, mientras las 9 salidas llevan 220 Ω y opto.
> 2. **Paso 20 de la Guía hecho, con su resultado delante:**
>    - ~~Si p5 y p8 dan **circuito abierto contra masa y ~3,3 V en reposo** → el puente funciona, y los
>      bloques 4 y 5 se ejecutan.~~ → **no fue esta rama.**
>    - **Si dan ~10 kΩ contra masa** → el pull-up interno no puede ganarles, el pin queda
>      permanentemente en BAJO y **el mando está inoperante de fábrica**. En ese caso **el Protocolo
>      §8 y §9 no se ejecutan**: se anotan los números del paso 20 y **eso es el hallazgo**.
>      🛑 **ESTA. Dieron 9,92 kΩ y 0,6 V. El mando está inoperante de fábrica, y ése es el hallazgo.**
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
> **Los cuatro dieron lo mismo** —9,92 / 9,92 / 9,93 / 9,94 kΩ— **y la función rota es el mando.** La
> placa trata igual a los cuatro pines, así que el firmware no puede leer dos al revés de los otros
> dos: o los cuatro son activos en ALTO, o ninguno. Y la cámara ya está corregida desde N-67.
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
| El mando (`A·B·A·B` para entrar, `A·A·A` o `B·B·B` para salir) | ~~**Viva** — pero sólo con el puente de `J16`, porque no hay receptor~~ → 🔴 **MUERTA, medido el 3/09 (N-118).** Sin receptor **y** con el pin a 0,6 V, el puente tampoco sirve: no hay flanco que registrar |
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
| **H3** | **Módulo de reloj `DS3231`** con su pila | Prueba **12.6** (Courier RTC) | ✅ **comprado y montado** sobre la placa C1 (paso 22), conexiones I2C correctas. 🔴 **Sin verificar que dé la hora**: la única vía de consultarlo es `SET_RTC` por Bluetooth (paso 27, bloqueado). Y sigue en pie el aviso: si trae **`CR2032` en vez de `LIR2032`**, hay que desoldar el diodo o la resistencia de su circuito de carga — **la `CR2032` no es recargable y ese circuito la calienta** |
| **H4** | **Dos cámaras de demanda** *(confirmar antes si ya hay una en almacén)* | Prueba **11.3** | Contacto seco **normalmente abierto**. Se cablean a `J14`, que está probado — **no** a `J16` hasta que el paso 20 lo permita |
| **H5** | **Dos radios más, con antenas y coaxiales**, y la placa del repetidor con su `MAX3485` | La §6 entera del Protocolo (6 pruebas) | 🔴 **No se compra hasta decidir D2.** La topología vigente es de **2 radios en enlace directo, sin repetidor**: hoy esas 6 pruebas certifican algo que no se está desplegando |

### Lo que hay que construir

| # | Qué | Desbloquea | Estado |
|---|---|---|---|
| **C1** | **La placa del módulo ESP32** — con sus dos tiras de conector hembra, la fuente H2 y el reloj H3 encima, y el USB del módulo accesible | Pruebas **12.1**, **12.5**, **12.6**, y el uso de la app en toda la sesión | ~~🔴 **No está diseñada, ni fabricada, ni medida.**~~ → ✅ **CONSTRUIDA el 4/09** (paso 22): fuente conmutada 12 V -> 5 V, `DS3231` por `GPIO21`/`GPIO22`, salida a `J17`; masa común medida en **0 V** (paso 23) y montaje encendido **sin calentamiento ni reinicios** (paso 24). Módulo confirmado de **30 pines**. 🟡 **Falta una cosa: la fuente 12 V -> 5 V no se midió con carga** — en banco se alimentó por USB. **Antes de campo, no antes de la próxima sesión** |
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
> 🛑 **La sesión está hecha, y esta frase NO se levanta.** 24 de 29 pasos son buenas noticias sobre
> el cableado, la radio, la talanquera y las cámaras — **y ninguna sobre lo que este equipo hace en
> un cruce**. El ciclo no se vio moverse ni una vez, el Modo Degradado no se tocó, el Protocolo no se
> empezó, hay una tarjeta con un corto y el único mando físico del equipo resultó no ser pulsable.
> **Lo que corre en campo sigue siendo la V8.4.**
</content>
