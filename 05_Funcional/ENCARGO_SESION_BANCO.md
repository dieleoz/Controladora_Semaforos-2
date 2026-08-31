# ENCARGO — Sesión de banco · Controladora de Semáforos

**Revisión:** 31 de Agosto de 2026 · **Rama:** `main-nuevo`

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

---

## 📄 Los dos documentos que se llevan, y qué hace cada uno

Este encargo **no repite** ni el cableado ni las pruebas. Dice el **orden** y **de qué depende cada
cosa**. Lo que se ejecuta está en otros dos ficheros, y hay que llevar los dos:

| Documento | Qué es | Cómo se usa |
|---|---|---|
| [`Guia_Cableado_y_Pruebas_Banco.html`](Guia_Cableado_y_Pruebas_Banco.html) | **23 pasos** en formato `HAZ / COMPRUEBA / TIENES QUE VER / ANOTA`. Todo lo que es conectar, medir con multímetro y cargar firmware | Se abre en el navegador o se **imprime y se rellena a bolígrafo**. Guarda lo escrito en el propio navegador |
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
| **El mando de relés SE CONSERVA**, en `A` = `PB9` = `J16` p5 y `B` = `PB13` = `J16` p8 | `Maestro/src/mando.cpp:202-235` · `Esclavo/src/mando.cpp:218-250` | `A·A·A`, `B·B·B` y `A·B·A·B` siguen en el firmware. **Pero el receptor de radio nunca se compró**: hoy sólo se pueden inyectar los pulsos con un cable |
| **El umbral de silencio son 25 s, no 12** | `SFTY6_SILENCIO_MS = 25000UL` — `Maestro/include/protocolo.h:149` y `Esclavo/include/protocolo.h:149` | Al cortar la radio se cronometran **~25 s**. Si sale alrededor de 12, **el firmware cargado no es el nuevo** |
| **`VENTANA_TRIPLE_MS = 12000` sigue siendo 12 s, y es correcto** | `Maestro/src/mando.cpp:38` · `Esclavo/src/mando.cpp:42` | Es la ventana para encadenar `A·A·A` o `B·B·B`. **No son los 25 s de arriba: son cosas distintas y no se confunden** |
| **El ESP32 de expansión tiene firmware nuevo**, va en `J17`, con Bluetooth SPP y reloj `DS3231` | `01_Firmware/ESP32_Expansion/` | **La placa que lo lleva NO EXISTE**, ni su fuente está pedida. Ver el bloque de compras |

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

| Bloque | Qué | Dónde está | Por qué en ese sitio |
|---|---|---|---|
| **0** | Módulo, carga del firmware, `J16`/`J17`, tapar los 12 V | **Guía, pasos 1 a 5** | La carga va **antes** de enchufar nada en `J16`. Con el firmware viejo dentro, `J16` p10 sigue siendo *Aceptar* en un equipo que está en la calle |
| **1** | 🔴 **Que el ciclo arranque y mueva las luces — N-42** | **Guía, pasos 6 a 8** + **Protocolo §2 y §3** | **Es la regresión abierta y es lo primero.** Mientras siga, nada de lo demás se puede dar por bueno |
| **2** | 🔴 **La medida de `J16` p5, p8, p10 y p12** | **Guía, paso 14** | **Decide si los bloques 4 y 5 existen.** Va pronto a propósito: si sale mal, hay media sesión que replantear y conviene saberlo el primer día, no el último |
| **3** | Reloj, pila y veredicto del cristal `Y2` | **Protocolo §7** | Requisito del Modo Degradado: sin reloj en hora y sincronizado, el modo no entra. **Y va después del bloque 1** porque una carga nueva borra el binario que se estuviera bisecando |
| **4** | Mando de relés, por puente | **Protocolo §8** | Depende del bloque 2 |
| **5** | 🔴 **Modo Degradado, incluido el defecto N-106** | **Protocolo §9** | Depende de los bloques 3 y 4. Es lo más largo y lo más delicado |
| **6** | Talanquera y cámaras de demanda | **Guía, pasos 9 a 15** + **Protocolo §11** | Independiente. Si un bloque anterior se atasca, éste se puede adelantar |
| **7** | Telemetría, órdenes y **los rechazos** | **Protocolo §12** | Se puede hacer en paralelo con cualquier otro, con el terminal abierto |

---

## 1 · El bloque que manda: que el ciclo mueva las luces (N-42)

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
| **Adaptador USB-TTL a 9600** directo a `J17` — su RX a p3, su TX a p2, masa a p7 o p9 | ✅ **Sí** |
| ESP32 + Bluetooth SPP + app del móvil | ❌ **No.** La placa no existe |

**Anote siempre por cuál lo hizo.** Una orden que llega por USB-TTL demuestra que el firmware del
STM32 la entiende; **no demuestra nada del ESP32, ni del Bluetooth, ni de la app.**

**El censo completo de órdenes está en el §0.2 del Protocolo**, sacado del fuente y no de memoria.
No se copia aquí para no crear dos listas que alguien tendría que sincronizar.

> ⚠️ **Y una nota de lectura que evita reportar averías que no lo son.** En el `$STATUS`, **`BAT:12.6`
> en las dos puntas, y `RF:98%`, `RTT:85ms` y `MODO:SUBORDINADO` en el Esclavo, son literales fijos
> del código, no medidas.** No se usan para decidir nada. Consecuencia concreta: **desde el `$STATUS`
> del Esclavo no se puede saber si está en Modo Degradado.**

---

## 4 · El mando sin receptor: cómo se ejerce, y cuándo NO se ejerce

**El receptor de radio del mando nunca se compró.** Pero los dos canales que quedan son pines de
entrada de la propia tarjeta, leídos en `INPUT_PULLUP` y **activos en BAJO**:

```text
MANDO A  =  BOTON1  =  PB9   =  J16 p5
MANDO B  =  BOTON2  =  PB13  =  J16 p8
masa                          =  J16 p2
```

**Un pulso `A` es tocar un instante `J16` p5 contra masa con un cable suelto.** Es el mismo recurso
que la Guía usa en su paso 13 para hacer de cámara con un pulsador.

> 🔴 **Antes de tocar `J16`, dos requisitos que no son opcionales:**
> 1. **Paso 4 de la Guía hecho: el pin de 12 V tapado.** En el cobre esos 12 V corren a **1,36 mm**
>    del más cercano de estos pines — no a los milímetros que se ven entre los pines del conector.
> 2. **Paso 14 de la Guía hecho, con su resultado delante:**
>    - Si p5 y p8 dan **circuito abierto contra masa y ~3,3 V en reposo** → el puente funciona, y los
>      bloques 4 y 5 se ejecutan.
>    - Si dan **~10 kΩ contra masa** → el pull-up interno no puede ganarles, el pin queda
>      permanentemente en BAJO y **el mando está inoperante de fábrica**. En ese caso **el Protocolo
>      §8 y §9 no se ejecutan**: se anotan los números del paso 14 y **eso es el hallazgo**.
>
> ⚠️ **Lo que el puente NO demuestra, y va escrito al lado:** demuestra que **el firmware reconoce
> las secuencias**. No demuestra que los pulsos lleguen **desde el piso, por radio**, que es la
> condición real de uso — con su rebote de contacto y sus ~2 s por pulsación. Eso sigue sin
> receptor con el que probarlo, y queda aplazado (prueba 8.9 del Protocolo).

> ⚠️ **Y un detalle de numeración que conviene saber antes de buscarlo en el poste:** la Guía se
> refiere a esta medida unas veces como *«la medida del paso 14»* y otras como *«del paso 15»*. **La
> medida es el PASO 14** —*«Antes de tocar las cámaras de `J16`: la medida que lo decide»*—; el paso
> 15 es el que **cablea** la cámara, y sólo si el 14 salió bien. Cuando este encargo dice «paso 14»
> se refiere a la medida.

> 🔴 **Un desacuerdo entre documentos que hay que resolver antes de la sesión, y no lo resuelve el
> técnico.** El paso 14 de la Guía dice *«los cuatro tienen que dar lo mismo»*. Eso era cierto
> cuando p5, p8, p10 y p12 eran los cuatro pulsadores. **Desde el 31/08 no lo es:** p10 y p12 son
> cámaras y las quieren en reposo a masa (activas en ALTO), mientras p5 y p8 son el mando, activo
> en BAJO. **Si los cuatro dieran lo mismo, una de las dos funciones estaría rota.** Se anota como
> hallazgo del paso 14 y se decide con el dato delante, no en el poste.

---

## 5 · El Modo Degradado, y el hueco que hay que mirar de frente

### La regla que se confirma antes que ninguna otra

**El equipo NUNCA entra solo al Modo Degradado.** Al perder la radio, el comportamiento correcto es
caer a **ámbar intermitente** en las dos puntas. Un ámbar le dice al conductor *«nadie controla este
cruce»*; un verde por reloj sin confirmar la otra punta le da confianza falsa. La entrada es **100 %
manual**.

> **Primera prueba, y es de seguridad (Protocolo 9.1):** corte la radio y compruebe que **ambas
> puntas caen a ámbar intermitente** y que **ninguna entra sola en Degradado**. Si alguna entra
> sola, **pare la sesión y repórtelo de inmediato.**

### 🔴 El Esclavo hoy sólo tiene una vía de entrada y de salida, y es el puente

**MEDIDO:** el Esclavo **no acepta ni un solo comando `SET_MODO:*`** por el puerto serie. Sumado a
que no hay pantalla y a que `botonAceptar()` es `false`:

| vía de entrada / salida del Degradado en el ESCLAVO | estado hoy |
|---|---|
| El mando (`A·B·A·B` para entrar, `A·A·A` o `B·B·B` para salir) | **Viva** — pero sólo con el puente de `J16`, porque no hay receptor |
| La vuelta automática de la radio (deja de gobernar cuando el Maestro habla) | **Viva** |
| La pantalla (`Esclavo/src/menu.cpp:215`) | **Existe en el código y está MUERTA**: el `aceptar` que la dispara no puede ser cierto nunca |
| Por comando / app | **No existe** |

> **En claro: si el Esclavo entra en Degradado y no hay puente ni receptor, la única forma de sacarlo
> es que vuelva la radio.** Es un dato para el dictamen, no una opinión, y por eso el Protocolo lo
> mide y lo hace firmar en su prueba **9.15**.

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

1. **La Guía rellena**, con los tres cuadros de `ANOTA` de los 23 pasos — **también los que salieron
   bien**. Una casilla en blanco no dice nada.
2. **El Protocolo rellenado y firmado**, con su acta. Las pruebas sin casilla **no se firman**: si
   ejecutó alguna de ellas por su cuenta, anótelo en el cuadro de texto libre, no invente casilla.
3. **N-42:** `md5` de cada binario cargado y qué hizo cada uno. **Si el ancla falló, dígalo — es un
   resultado, no un fracaso.**
4. **Los números del paso 14 de la Guía**, tal cual, aunque no se entiendan. Deciden dos bloques.
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

### Hardware que hay que comprar

| # | Qué | Desbloquea | Ojo |
|---|---|---|---|
| **H1** | **Receptor de radio del mando de relés** (uno por punta si se quiere en las dos) | Prueba **8.9**, y convierte todo el bloque 4/5 de *«ejercido con un cable»* a *«ejercido como se usa»* | 🔴 **No se compra hasta decidir D1.** Ver abajo |
| **H2** | **Fuente conmutada DC-DC 12 V → 5 V, ≥ 1 A**, con fusible y protección de polaridad inversa | Toda la §12 del Protocolo | **Conmutada, no lineal.** Un lineal desde 12 V disiparía más de 4 W en un armario cerrado y al sol, y cae de tensión justo cuando el módulo tira del pico — el síntoma parece un problema de programa. **La referencia concreta no está elegida** |
| **H3** | **Módulo de reloj `DS3231`** con su pila | Prueba **12.6** (Courier RTC) | Si trae **`CR2032` en vez de `LIR2032`**, hay que desoldar el diodo o la resistencia de su circuito de carga: **la `CR2032` no es recargable y ese circuito la calienta** |
| **H4** | **Dos cámaras de demanda** *(confirmar antes si ya hay una en almacén)* | Prueba **11.3** | Contacto seco **normalmente abierto**. Se cablean a `J14`, que está probado — **no** a `J16` hasta que el paso 14 lo permita |
| **H5** | **Dos radios más, con antenas y coaxiales**, y la placa del repetidor con su `MAX3485` | La §6 entera del Protocolo (6 pruebas) | 🔴 **No se compra hasta decidir D2.** La topología vigente es de **2 radios en enlace directo, sin repetidor**: hoy esas 6 pruebas certifican algo que no se está desplegando |

### Lo que hay que construir

| # | Qué | Desbloquea | Estado |
|---|---|---|---|
| **C1** | **La placa del módulo ESP32** — con sus dos tiras de conector hembra, la fuente H2 y el reloj H3 encima, y el USB del módulo accesible | Pruebas **12.1**, **12.5**, **12.6**, y el uso de la app en toda la sesión | 🔴 **No está diseñada, ni fabricada, ni medida.** El apartado 06 de la Guía dice **cómo tiene que ser**, no cómo es. **No se manda a fabricar hasta contar los pines y medir el ancho del módulo — paso 1 de la Guía**: vienen de 30 y de 38 pines y de anchos distintos |
| **C2** | **Cargar el firmware del ESP32** en el módulo | Ídem | El firmware **existe** (`01_Firmware/ESP32_Expansion/`) y **no está cargado**. Un módulo sin él aparece en el teléfono, deja conectar y **no reenvía nada**: eso es lo esperado, **no se reporta como avería y no se cambia el módulo** |

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

---

> **Nada de esta versión sube a un cruce abierto al tráfico hasta que esta sesión esté hecha y sus
> resultados revisados.** Y ni siquiera entonces lo autoriza este documento: lo autoriza el
> responsable, con el acta firmada delante.
</content>
