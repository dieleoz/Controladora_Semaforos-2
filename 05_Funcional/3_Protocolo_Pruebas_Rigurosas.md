# 📋 PROTOCOLO DE AUDITORÍA FUNCIONAL Y ACTA DE CERTIFICACIÓN EN CAMPO

**Revisión:** 4 de septiembre de 2026 *(la anterior, 31 de agosto)* · **Rama:** `main-nuevo`
**Documento para remisión al Ingeniero Funcional / Auditor de Tránsito**
**Ubicación:** `05_Funcional/3_Protocolo_Pruebas_Rigurosas.md`
**Entorno Auditable:** Ecosistema Semafórico Móvil (Maestro STM32, Esclavo STM32)

> **Esta revisión sustituye a la V8.7 del 1 de Agosto.** La anterior seguía dando por navegable un
> menú que ya no se puede abrir y por legible una pantalla que ya no conduce sus pines. Cada una de
> sus pruebas se ha revisado una por una y marcada; **ninguna se ha borrado en silencio.**

---

## 🛑 04/09 — LO PRIMERO DE TODO: EL BANCO YA CORRIÓ, Y HAY UNA TARJETA EN ALERTA

**El 3 y el 4 de septiembre se ejecutó por primera vez la `Guia_Cableado_y_Pruebas_Banco.html`
completa —29 pasos— sobre tarjetas reales, con el paquete `617bd00`.** El informe devuelto es
`evidencia/Informe_Pruebas_Banco_Semaforos_V9.0.pdf`, y **manda sobre cualquier suposición de este
documento**. Tres cosas de las que trae hay que leerlas **antes** de planificar una sesión con este
protocolo:

| | Qué se midió | Qué significa para este protocolo |
|---|---|---|
| 🔴 **1** | **Durante el paso 29, el STM32 de la placa MAESTRO se sobrecalentó** al puentear `J16` p5/p8 contra masa. La prueba se abortó, la placa siguió funcionando pero caliente, y el informe **pide inspección técnica en frío antes de cualquier otra prueba eléctrica en `J16`** | **Esa tarjeta no se energiza para este protocolo hasta que alguien la inspeccione.** No es una recomendación de proceso: el informe declara *señal de alerta de un posible daño interno ya iniciado* |
| 🔴 **2** | **`J16` p5 y p8 —el mando `A` y `B`— midieron `0,6 V` en reposo**, con `9,92 kΩ` a masa. El pull-up interno no ganaba | ~~**§0.3 escribió dos ramas y salió la mala: el mando está inoperante de fábrica.**~~ 🔧 **Corregido el 04/09 (N-118): ese `0,6 V` lo producía EL FIRMWARE, no sólo la placa** — era el pull-up interno contra los 10 kΩ. **El firmware ya está arreglado en las dos puntas** (`INPUT` pelado, activo en ALTO) y **el gesto para pulsar el mando CAMBIA: p5 contra p4 y p8 contra p7, NO contra masa.** La §8 y la parte de la §9 que dependen del puente **siguen sin ejecutarse**, pero **por otro motivo**: no hay tarjeta sana con la que ejercerlo. Ver §0.3 |
| 🔴 **3** | **El módulo Bluetooth nunca se anunció en el teléfono**, con el firmware cargado sin errores y el hardware descartado como causa | Todo lo que entra por la app sigue sin poder probarse. **La vía de `J17` por USB-TTL sigue siendo la única**, y este protocolo ya está escrito sobre ella |

> ⚠️ **Y lo que el banco NO decidió, que es lo que más se va a malinterpretar: la regresión N-42.**
> El informe es explícito: *«esta sesión no la confirma ni la descarta»*. El equipo **nunca llegó a
> operar en Modo Automático** porque la única vía de selección de modo es la app, y la app no
> conectó. **Un «no se pudo probar» no es un «sigue rota» ni un «ya está»** — es un `ABORTADO`, y
> un `ABORTADO` no dice nada del firmware. **Se decide repitiendo el paso 7 de la Guía con enlace**,
> y hasta entonces la advertencia de la Sección 3 sigue en pie **exactamente igual que antes**.

---

## 🔴 LO PRIMERO: POR QUÉ NO PUEDE USARSE LA REVISIÓN ANTERIOR

Tres cosas cambiaron en el equipo y las tres invalidan procedimiento escrito:

| Qué cambió | Dónde está medido | Consecuencia para este documento |
|---|---|---|
| **`botonAceptar()` y `botonCancelar()` devuelven `false` siempre.** Los pulsadores 3 y 4 dejaron de ser pulsadores: sus pines (`PB14`, `PB15` = `J16` p10/p12) pasan a ser entradas de cámara | `Maestro/src/botones.cpp:305-306` · `Esclavo/src/botones.cpp:316-317` — **MEDIDO** | **No se puede aceptar ni cancelar nada.** Todo paso que diga *«pulse Botón 3»*, *«confirme»*, *«entre en CONFIGURACION»* o *«recorra el menú»* **no se puede ejecutar** |
| **La pantalla no se retira, pero deja de conducir sus pines.** `PB3`/`PB4`/`PB5` quedan en alta impedancia porque comparten el conector `J17` con el ESP32 | `Maestro/src/lcd.cpp:74-75` · `Esclavo/src/lcd.cpp:92-93` (los cuatro pines a `U8X8_PIN_NONE`); el porqué, en el comentario de `lcd.cpp:18-73` — **MEDIDO** | **No hay imagen.** El framebuffer se sigue componiendo y no se vuelca al cable. Todo paso que diga *«la pantalla muestra»* o *«anote lo que muestra»* **no se puede ejecutar** |
| **El mando de relés SE CONSERVA**, en los canales `A` (`PB9` = `J16` p5) y `B` (`PB13` = `J16` p8). Las tres secuencias siguen en el firmware | `Maestro/src/mando.cpp:202-235` · `Esclavo/src/mando.cpp:218-250` — **MEDIDO**. Ventanas: `VENTANA_TRIPLE_MS = 12000` (`mando.cpp:38` Maestro, `:42` Esclavo) y `VENTANA_CUADRUPLE_MS = 18000` (`:39` / `:43`) | ~~Las secuencias se pueden **ejercer**~~ ~~**— corregido el 04/09: NO se pueden.** Siguen en el firmware, pero los dos pines miden `0,6 V` en reposo, así que **no hay flanco que darles**: ni con el puente a mano ni con el receptor, que además **nunca se compró**.~~ 🔧 **Corregido otra vez el 04/09, después de N-118, y esta vez con el fuente delante:** el `0,6 V` era obra del `INPUT_PULLUP` que el firmware ponía, y **ese firmware ya no existe** — `botones.cpp` lee `BOTON1`/`BOTON2` en `INPUT` pelado y **activo en ALTO** en las dos puntas. **El flanco sí se puede dar; lo que cambia es cómo: p5 contra p4, p8 contra p7 (3,3 V del pin de al lado), NUNCA contra masa.** Lo que sigue faltando es **una tarjeta con la que ejercerlo** —la Maestro está fuera de servicio por N-116— y el **receptor, que nunca se compró**. Ver §0.3 |

> ⚠️ **Y una consecuencia que no es evidente y hay que decir en voz alta.** Con `botonAceptar()`
> siempre `false`, `menu_estaAbierto()` no puede ser cierto nunca, así que **el mando del Esclavo ya
> no está inhibido en ningún caso** (`Esclavo/src/botones.cpp:316-317`, **MEDIDO**). La regla
> *«con el menú abierto el mando se ignora»* no está rota: **se quedó sin sujeto**.

---

## 📌 CÓMO SE MARCÓ CADA PRUEBA — la parte que decide si este documento sirve

Una prueba que no se puede ejecutar y **sigue teniendo casilla de firma es peor que una que falta**:
la que falta no miente, y ésa sí. Por eso ninguna se ha borrado, y **las que no se pueden ejecutar
hoy no llevan casilla**.

| Marca | Qué significa | Lleva casilla |
|---|---|---|
| ♻️ **SE REESCRIBE** | La propiedad sigue viva y **sólo cambia por dónde entra el operario** — del menú al comando o al puente de `J16` | **Sí** |
| ⏸️ **SE APLAZA** | Necesita algo que **no existe todavía**. Va escrito qué falta | **No** |
| 🚫 **SE RETIRA** | Medía algo que **ya no existe** | **No** |
| ⛔ **NO EJECUTABLE EN ESTA PLACA** *(marca nueva, 04/09)* | La propiedad **sigue viva y la prueba sigue siendo buena**, pero el estado **medido** del hardware impide provocar la condición. Va con el número que lo demuestra al lado | **La tiene, y se tacha con el motivo** — nunca `CUMPLE`, nunca `NO CUMPLE` |

> 🆕 **Por qué hizo falta una cuarta marca el 04/09.** Las tres de arriba reparten por lo que dice
> **el firmware**: se reescribe, se aplaza o se retira. El banco del 3–4/09 trajo un cuarto motivo
> que ninguna cubría: **una prueba correcta, sobre un firmware correcto, que no se puede ejecutar
> porque el estado del hardware impide provocar la condición** ~~(`J16` p5/p8 a `0,6 V`)~~. Meterla
> en «se aplaza» sería mentir sobre qué falta —no falta comprar nada—; dejarla con casilla abierta
> invitaría a firmarla; y marcarla `NO CUMPLE` acusaría al firmware de algo que no ha llegado a
> hacer. **Los tres estados de `PASS` / `FALLA` / `ABORTADO` valen igual aquí, y el tercero
> necesitaba nombre.**
>
> > ## 🔧 04/09, MÁS TARDE (N-118) — **EL MOTIVO DE TODOS LOS ⛔ DE ESTE DOCUMENTO HA CAMBIADO. LÉASE UNA VEZ Y VALE PARA TODOS**
> >
> > A lo largo de la §8 y la §9 hay **una docena de pruebas marcadas ⛔ citando como motivo *«el
> > puente de `J16`, medido a `0,6 V`»*.** Ese motivo **está caducado y se tacha aquí en bloque**,
> > porque tacharlo doce veces por separado haría el documento ilegible.
> >
> > **Lo que se midió era cierto; lo que se dedujo, no.** Los `0,6 V` no eran una propiedad del
> > cobre: los producía **el `INPUT_PULLUP` que el firmware ponía** contra los 10 kΩ a masa de la
> > placa. **N-118 cambió las dos puntas a `INPUT` pelado y activo en ALTO**, así que el puente sí
> > puede dar un flanco — **dado contra los 3,3 V del pin de al lado (p5–p4, p8–p7), nunca contra
> > masa**. Ver §0.3, que trae el gesto entero.
> >
> > **Lo que NO cambia: esas pruebas siguen marcadas ⛔ y siguen sin casilla firmada.** El motivo
> > vigente, y el único que hay que escribir en el acta, es:
> >
> > > ⛔ **NO EJECUTABLE (04/09): el mando está arreglado en el fuente (N-118) y PENDIENTE DE
> > > EJERCER EN TARJETA. La tensión de `J16` p5/p8 con el puente a 3,3 V y este firmware cargado
> > > NO SE HA MEDIDO NUNCA —el paso 29 no llegó a ese dato— y no se puede medir sobre la Maestro
> > > mientras siga el corto de N-116.**
> >
> > **Por qué se corrige en vez de dejarlo:** un ⛔ que dice *«la placa lo impide»* manda a rediseñar
> > una placa que está bien y a no comprar un receptor que sí funcionaría. Un ⛔ que dice *«falta
> > tarjeta sana»* manda a arreglar la tarjeta, que es lo que hay que hacer.

**El reparto de las 82 pruebas numeradas de la revisión anterior:**

| | pruebas | dónde están |
|---|---|---|
| ♻️ **SE REESCRIBE** | **49** | §1 (3) · §2 (5) · §3 (5) · §4 (2) · §5 (6) · §7 (6) · §8 (5) · §9 (12) · §11 (2) · §12 (3) |
| ⏸️ **SE APLAZA** | **12** | §6 (6) · §9 (2) · §11 (1) · §12 (3) |
| 🚫 **SE RETIRA** | **21** | §1 (1) · §4 (2) · §5 (2) · §7 (5) · §8 (3) · §10 (5) · §11 (1) · §13 (2) |
| ➕ **NUEVAS** | **4** | 8.9 · 9.15 · 9.16 · 12.7 |
| | **86** | |

> **Sobre el número: son 82 identificadores, no 80.** El resumen de la revisión anterior sumaba `80`
> porque `6.0` y `6.0-bis` no llevaban línea `CUMPLE` aunque sí llevaban casillas de respuesta.
> Contados por identificador numerado son **82**. Se dice aquí porque un total que no cuadra con lo
> que hay debajo se arrastra de revisión en revisión.
>
> ⚠️ **Y ese `49` de «se reescribe» NO es el `49` de la auditoría del 28/08.** Aquélla contó
> **49 pruebas que dejan de ser ejecutables**; aquí las no ejecutables son **33** (12 aplazadas +
> 21 retiradas). La coincidencia del número es casual y no debe leerse como confirmación. La
> diferencia se explica sola: la decisión del **31/08 de conservar el mando en `A` y `B`** rescató
> la Sección 8 y casi toda la 9, que aquella auditoría daba por perdidas.

---

## 🚨 PASO 0 — OBLIGATORIO ANTES DE CUALQUIER PRUEBA

### 0.1 · El cableado y las medidas de la tarjeta van ANTES, y no se repiten aquí

**Todo lo que es conectar, medir con multímetro y cargar el firmware está en
[`Guia_Cableado_y_Pruebas_Banco.html`](Guia_Cableado_y_Pruebas_Banco.html)**, en formato
`HAZ / COMPRUEBA / TIENES QUE VER / ANOTA`, 29 pasos. **Se ejecuta entera antes que este documento y
no se duplica aquí**: dos versiones del mismo procedimiento son dos cosas que alguien tendría que
mantener sincronizadas, y el día que difieran nadie sabrá cuál manda.

De sus 29 pasos, éstos son requisito de este protocolo. **La tercera columna es nueva: trae lo que
la sesión del 3–4/09 midió de verdad, y con eso estos cuatro requisitos ya están cubiertos.**

| Paso de la Guía | Por qué es requisito aquí | 🔬 Resultado 3–4/09 |
|---|---|---|
| **2** — cargar el firmware nuevo en las dos tarjetas | Va **antes** de enchufar nada en `J16`. Con el firmware viejo dentro, `J16` p10 sigue siendo *Aceptar* | 🟢 **COMPLETO.** Las dos, por SWD con ST-LINK: *«escrito y verificado» al primer intento*, **sin puente `BOOT0`** |
| **3** y **4** — distinguir `J16` de `J17` y **tapar el pin de 12 V** | `J16` p1 lleva 12 V crudos. Sin tapar, no se toca `J16` | 🟢 **COMPLETO.** `J16` p1 mide 12 V, `J17` p1 no. El pin de 12 V se **retiró físicamente** del conector volante, y se confirmó tapado en las medidas del paso 21 |
| **20** — la medida de reposo de `J16` p5, p8, p10, p12 | **Decide si §8 y §9 de este protocolo se pueden ejecutar.** Ver §0.3 | 🔴 **MEDIDO, y decide que NO.** Ver la tabla de abajo |
| **24** — los cuatro hilos a `J17` | Es el único camino por el que hoy entra una orden | 🟢 **COMPLETO** con el montaje definitivo: mismo comportamiento que en mesa, **sin calentamiento, sin reinicios ni parpadeo anómalo** |

~~🔴 **Un desacuerdo entre este protocolo y la Guía que hay que resolver antes de la sesión, y no lo
resuelve el técnico.** El paso 20 de la Guía mide `J16` p5, p8, p10 y p12 y dice *«los cuatro
tienen que dar lo mismo»*.~~ *(Ya no hay desacuerdo: se midió.)*

> ## ✅ 04/09 — ESE DESACUERDO SE RESOLVIÓ MIDIENDO, Y EL PROTOCOLO TENÍA RAZÓN
>
> La Guía pedía que los cuatro pines dieran lo mismo; ~~este protocolo avisó de que **si los cuatro
> dieran lo mismo, una de las dos funciones estaría rota**, porque p10/p12 son cámaras (activas en
> ALTO, reposo a masa) y p5/p8 son el mando (`INPUT_PULLUP`, activo en BAJO —
> `Esclavo/src/botones.cpp:37`, `:160-161`).~~ **El paso 20 lo midió:**
>
> | `J16` | Función | R a masa | R a 3,3 V | **V en reposo, con energía** | Lectura |
> |---|---|---|---|---|---|
> | **p5** | mando `A` | `9,92 kΩ` | `11,28 kΩ` | **`0,6 V`** | 🔴 **BAJO permanente** *(con el firmware de entonces)* |
> | **p8** | mando `B` | `9,92 kΩ` | `11,28 kΩ` | **`0,6 V`** | 🔴 **BAJO permanente** *(ídem)* |
> | **p10** | cámara `C` | `9,93 kΩ` | `11,29 kΩ` | `0 V` | 🟢 correcto |
> | **p12** | cámara `D` | `9,94 kΩ` | `11,31 kΩ` | `0 V` | 🟢 correcto |
>
> **Las resistencias son casi idénticas y el resultado salió opuesto.** Para la cámara, esos ~10 kΩ a
> masa **son** la resistencia de reposo que hacía falta y que sólo decía el netlist: **la medida `M3`
> queda CERRADA**. ~~Para el mando, esos mismos 10 kΩ **ganan al pull-up interno**, el pin queda
> leído como accionado en reposo y **nunca hay flanco que detectar**.~~
>
> ~~🔴 **Consecuencia dura, y no es opinable: el mando de relés está inoperante de fábrica en esta
> placa.** No es que falte el receptor —que también falta—: es que **el puente a masa de §0.3 tampoco
> puede funcionar**, porque no hay transición que provocar.~~ Y cuando en el paso 29 se intentó de
> todas formas, la placa se calentó.
>
> > ### 🔧 04/09 (N-118) — LA CONCLUSIÓN DE ARRIBA ERA DE LA PLACA **Y DEL FIRMWARE**, Y EL FIRMWARE YA ESTÁ ARREGLADO
> >
> > **La diferencia entre las dos mitades de esa tabla no la ponía el cobre: la ponía el `pinMode()`.**
> > Las cuatro resistencias son iguales; lo que no era igual es cómo declaraba el firmware cada pin.
> > `BOTON1`/`BOTON2` iban en `INPUT_PULLUP`, y el pull-up interno (30–50 kΩ) contra esos 10 kΩ deja
> > el pin en **0,55–0,83 V** — el banco midió `0,6 V`, dentro de la horquilla. Las cámaras iban en
> > `INPUT` pelado y por eso daban `0 V`. **La Guía tenía razón: los cuatro pines son eléctricamente
> > idénticos y tienen que leerse igual.**
> >
> > **MEDIDO sobre el fuente el 04/09, en las DOS puntas** (`Maestro/src/botones.cpp`,
> > `Esclavo/src/botones.cpp`):
> >
> > ```text
> >   pinMode(BOTON1, INPUT);   pinMode(BOTON2, INPUT);      <- INPUT pelado, ya no INPUT_PULLUP
> >   bool lecturaCruda = (digitalRead(b.pin) == HIGH);      <- ACTIVO EN ALTO, ya no LOW
> >   const bool pulsado = (digitalRead(...) == HIGH);       <- la siembra del arranque, igual
> > ```
> >
> > 🔴 **Lo que hay que llevarse al banco, porque el gesto es el contrario del que decía este
> > documento:** un pulso `A` es **tocar `J16` p5 contra p4**, y un pulso `B` es **tocar p8 contra
> > p7** — los 3,3 V que el conector reparte en la posición de al lado de cada entrada. **Un cable a
> > masa no produce absolutamente nada.** Sólo hay **una** masa en todo `J16` (p2), así que un
> > contacto por botón contra masa nunca pudo ser el diseño.
> >
> > 🛑 **Y lo que sigue SIN medirse, que es lo que impide dar esto por cerrado:** **nadie ha medido
> > la tensión de p5/p8 con el puente a 3,3 V puesto y este firmware cargado.** El paso 29 nunca
> > llegó a tomar ese dato —se abortó por el sobrecalentamiento— y **no se puede tomar sobre la
> > Maestro mientras siga con el corto de N-116**. El firmware está **arreglado y razonado, no
> > verificado en tarjeta**.

### 0.2 · Por dónde se manda una orden hoy

**El puerto serie de `J17` es la única vía de operación que queda.** Hay dos formas de usarlo, y
**para este protocolo son equivalentes salvo donde se diga**:

| vía | disponible hoy | cómo |
|---|---|---|
| **Adaptador USB-TTL a 9600** directo a `J17` (su RX a p3, su TX a p2, masa a p7 o p9) | ✅ **Sí** | Guía, paso 6 y paso 28 |
| **ESP32 + Bluetooth SPP + app del móvil** | ❌ **No** — pero ~~la placa del módulo no existe (§0.4)~~ **ese motivo caducó el 04/09: la placa EXISTE y está armada.** Lo que no ocurre es que el módulo **se anuncie en el teléfono** (Guía, pasos 10 y 25) | Guía, apartado 07 |

**Anote siempre por cuál de las dos lo hizo.** Una orden que llega por USB-TTL demuestra que el
firmware del STM32 la entiende; **no demuestra nada del ESP32, ni del enlace Bluetooth, ni de la app.**

**Las órdenes que el firmware acepta hoy están censadas del fuente** (`Maestro/src/bluetooth.cpp` y
`Esclavo/src/bluetooth.cpp`, **MEDIDO el 31/08**). Se citan por **literal**, no por número de línea:
esos dos ficheros los está tocando otro trabajo en el mismo árbol y una línea citada que se mueve
manda al lector a un sitio que no dice lo que promete.

**MAESTRO — sin PIN** (dos excepciones deliberadas de la puerta de autenticación):

```text
CMD:FORZAR_ROJO
CMD:SET_MODO:MENU
CMD:SET_MODO:ALCANCE
```

**MAESTRO — con PIN**, forma `CMD:PIN:1234:<acción>`:

```text
SET_MODO:AUTO           SET_MODO:MANUAL         SET_MODO:AMBAR
SET_MODO:MENU           SET_MODO:ALCANCE        SET_MODO:INTELIGENTE
SET_MODO:DEGRADADO      FORZAR_ROJO             MANUAL:CAMBIAR_TURNO
TEST_LEDS               REINICIAR_RELOJ         DEMANDA
SET_TIEMPOS:<verde_min>,<rojo_min>,<despeje_seg>
SET_RTC:AAAA-MM-DD,HH:MM:SS
```

**ESCLAVO — sin PIN:** `CMD:AMBAR_EMERGENCIA` · `CMD:FORZAR_ROJO` *(se rechaza a propósito)*
**ESCLAVO — con PIN:** `AMBAR_EMERGENCIA` · `FORZAR_ROJO` · `SOLICITAR_PASO` · `TEST_LEDS`
*(se rechaza a propósito)* · `SET_RTC:AAAA-MM-DD,HH:MM:SS`

> 🔴 **El Esclavo NO tiene ni un solo `SET_MODO:*`.** No hay comando que lo meta en un modo ni que
> lo saque. Es un hecho medido y tiene consecuencias en toda la Sección 9. Ver la prueba **9.15**.

**Lo que el equipo emite sin que nadie se lo pida** (los dos, ~~cada 1000 ms~~ **cada 2000 ms** el
`$STATUS` — **bajado el 04/09 por decisión del responsable, en las dos puntas**; si cronometra la
cadencia, el valor que tiene que dar es **2 s, no 1 s**):

```text
$STATUS,NODE:MAESTRO,SERIE:..,MODO:..,ESTADO:..,T:..,RF:..,RTT:..,BAT:--,HORA:HH:MM:SS,ESC:..*CRC
$STATUS,NODE:ESCLAVO,SERIE:..,MODO:SUBORDINADO,ESTADO:..,T:--,RF:--,RTT:--,BAT:--,HORA:..*CRC
$ALARM,NODE:..,EVENTO:..,CAUSA:..,ACCION:..,HORA:..*CRC
$EVENT,NODE:..,ORIGEN:..,DETALLE:..,HORA:..*CRC
```

> 🛑 **LAS DOS TRAMAS DE ARRIBA ESTABAN MAL EN CINCO CAMPOS — corregidas el 05/09 contra el
> `snprintf` real** (`Maestro/src/bluetooth.cpp:970`, `Esclavo/src/bluetooth.cpp:832`). El aviso que
> las acompañaba era correcto **en 2026-08**; N-108 retiró esos literales el 04/09 y el formato
> cambió debajo.
>
> ⚠️ **Campos MARCADOS con `--` porque el equipo no tiene con qué medirlos. Un `--` ES la respuesta
> correcta: no es una avería y no es un hueco.**
> - ~~**`BAT:12.6`** en las dos puntas.~~ → **`BAT:--` en las dos.** No hay **un solo**
>   `analogRead()` en las cuatro carpetas de firmware — **MEDIDO, `grep` sin una sola llamada** (las
>   dos coincidencias son comentarios). Sin divisor ni entrada analógica no hay batería que leer.
> - ~~**`RF:98%` y `RTT:85ms` en el Esclavo.**~~ → **`RF:--` y `RTT:--` en el Esclavo.** En el
>   Maestro **sí** son medidos, y publican `--` mientras no haya muestras.
> - ~~**`T:`** es `(millis()/1000)%60`, **no** el tiempo que le queda a la fase.~~ → 🔴 **AL REVÉS
>   DESDE EL 04/09: en el MAESTRO `T:` SÍ son los segundos que le quedan a la fase**
>   (`modoAutomatico_segundosRestantesFase()`), y `--` cuando ese plazo no existe. **En el ESCLAVO es
>   `--` siempre**: esa punta es subordinada y no conoce el plazo.
> - 🆕 **`ESC:` va SÓLO en la trama del Maestro y es el ÚLTIMO campo** (N-149), con valores
>   `ROJO`/`VERDE`/`AMBAR`/`?`. **Que falte en el `$STATUS` del Esclavo no es un fallo.**
> - **`MODO:SUBORDINADO` en el Esclavo es fijo**, así que **desde el `$STATUS` del Esclavo no se
>   puede ver si está en Modo Degradado.** Sólo lo insinúa `ESTADO:`. *(Esto no ha cambiado.)*
>
> `HORA:` cae a `--:--:--` cuando el reloj no está en hora. **Eso es un dato, no un fallo.**

### 0.3 · Cómo se ejercen las secuencias del mando SIN el receptor

El receptor de radio del mando **nunca se compró** (Guía, paso 29). Pero los dos canales que quedan
son pines de entrada de la propia tarjeta, y ~~el firmware los lee en `INPUT_PULLUP` **activo en
BAJO**~~ 🔧 **desde el 04/09 (N-118) el firmware los lee en `INPUT` pelado y ACTIVO EN ALTO**, igual
que las cámaras `C` y `D` (`Maestro/src/botones.cpp`, `Esclavo/src/botones.cpp`, mismo cuerpo —
**MEDIDO sobre el fuente el 04/09**):

```text
MANDO A  =  BOTON1  =  PB9   =  J16 p5     <- se cierra contra p4  (3,3 V)
MANDO B  =  BOTON2  =  PB13  =  J16 p8     <- se cierra contra p7  (3,3 V)

3,3 V                         =  J16 p4, p7, p9, p11   (uno al lado de cada entrada)
masa                          =  J16 p2                (UNA sola en todo el conector)
```

> 🛑 ~~**Un pulso `A` es tocar un instante `J16` p5 contra masa con un cable suelto.** Un pulso `B`,
> lo mismo en p8.~~ **CADUCADO EL 04/09 POR N-118, y es el error que más caro sale de este
> documento: con el firmware vigente un cable a masa NO PRODUCE ABSOLUTAMENTE NADA.** Quien vaya a
> banco con esta instrucción va a diagnosticar *«mando sordo»* sobre un firmware sano y va a perder
> la sesión.

**Un pulso `A` es tocar un instante `J16` p5 contra `J16` p4.** Un pulso `B`, tocar p8 contra p7.
**El contacto se cierra contra los 3,3 V del pin de al lado, nunca contra masa** — sólo hay una masa
en todo el conector, así que un contacto por botón contra masa nunca pudo ser el diseño. Es el mismo
recurso, y ahora la misma polaridad, que la Guía usa en su paso 19 para hacer de cámara con un
pulsador.

> ## 👁️ CÓMO SE SABE QUE EL EQUIPO OYÓ EL PULSO: LO CONFIRMA CON SUS PROPIAS LUCES
>
> **No hace falta ningún instrumento extra.** **MEDIDO en `01_Firmware/Maestro/src/mando.cpp:45-47`**
> (`DESTELLOS_AUTOMATICO = 2`, `DESTELLOS_AMBAR = 3`, `DESTELLOS_DEGRADADO = 4`):
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
> el rojo nunca significa *pase*: si el operario cuenta mal, el peor caso sigue siendo seguro. Y un
> rechazo **no habla el mismo idioma que un éxito**: ámbar rápido de 2 s, nunca destellos.
>
> 🛑 **LA TRAMPA DE ESTA PRUEBA, Y HAY QUE LEERLA ANTES DEL PRIMER PULSO: pruebe DESDE OTRO MODO.**
> Si el equipo **ya está** en el modo que la secuencia pide, `MODO:` **no cambia** —`mando.cpp` entra
> por la rama `if (modoActual_get() == MODO_AUTOMATICO) modoAutomatico_setup();`— y **la prueba no
> distingue nada**: un `A·A·A` que ha funcionado perfectamente sobre un equipo ya en Automático no
> mueve ni un campo del `$STATUS`. **O se prueba desde otro modo, o se cuentan los destellos — que
> se ven siempre.**
>
> 🔵 **Y por eso el USB-TTL baja de rango PARA EL MANDO.** Sigue siendo un recurso legítimo —es la
> vía de operación cuando la app no conecta, §0.2, que es justo lo que pasó en la sesión 1— pero
> **no es la forma de verificar el mando**: la respuesta son los destellos. Anotar «no cambió el
> `MODO:`» es cómo un mando sano se apunta como sordo.

> 🔴 **Requisitos, y no son opcionales:**
> 1. **El paso 4 de la Guía hecho**: el pin de 12 V de `J16` tapado. En el cobre, esos 12 V corren
>    a **1,36 mm** del más cercano de estos pines — no a los milímetros que se ven entre los pines
>    del conector.
> 2. **El firmware de N-118 CARGADO Y VERIFICADO en la tarjeta**, no mergeado. Con el firmware
>    anterior dentro, este puente no hace nada: los dos pines van en `INPUT_PULLUP` y ya se leen
>    accionados. **Anote el `md5` del binario cargado antes de tocar `J16`.**
> 3. ~~**El paso 20 de la Guía hecho, y con su resultado delante.** Si p5 y p8 dan **~10 kΩ contra
>    masa**, el pull-up interno no puede ganarles: el pin queda permanentemente en BAJO y el mando
>    está **inoperante de fábrica**.~~ ~~En ese caso **§8 y §9 no se ejecutan**~~ · ~~4. Si p5 y p8
>    dan **circuito abierto contra masa y ~3,3 V en reposo**, el puente funciona.~~ 🔧 **Los dos
>    ramales caducaron el 04/09.** Estaban escritos suponiendo un firmware con pull-up interno, y
>    **ése era el defecto, no el criterio**. El paso 20 ya midió lo que había que medir —los cuatro
>    pines llevan ~10 kΩ a masa y **eso es correcto para los cuatro**, porque los cuatro son activos
>    en ALTO—: **la medida `M3` quedó cerrada y no hay bifurcación que resolver**.
> 4. **La medida que SÍ falta, y que nadie ha tomado:** la tensión de p5/p8 **con el puente a 3,3 V
>    puesto y el firmware de N-118 cargado**. El paso 29 no llegó a ese dato y **no se puede tomar
>    sobre la Maestro mientras siga con el corto de N-116**. Hasta entonces, el mando está
>    **arreglado en el fuente y pendiente de ejercer en tarjeta** — que no es lo mismo que
>    verificado.
>
> ⚠️ **Lo que este recurso NO demuestra, y va escrito al lado:** demuestra que **el firmware
> reconoce las secuencias**. No demuestra que los pulsos lleguen **desde el piso por radio**, que es
> la condición real de uso — eso lleva el rebote del contacto del relé y sus ~2 s por pulsación, y
> **sigue sin receptor con el que probarlo**. Ver la prueba **8.9**.

> ## ⛔ 04/09 — LA BIFURCACIÓN DE ARRIBA YA ESTÁ RESUELTA, ~~Y SALIÓ EL CASO 2~~ **Y NINGUNO DE LOS DOS ERA EL CASO**
>
> **Este apartado escribió el 31/08 dos caminos y una regla para elegir. El banco del 3–4/09 midió,
> y salió `9,92 kΩ` contra masa y `0,6 V` en reposo** (paso 20 de la Guía, tabla completa en §0.1).
>
> ~~**Por tanto, y siguiendo la regla que este documento se dio a sí mismo:**~~
>
> - ~~**La §8 entera NO se ejecuta.** Sus cinco pruebas con casilla dependen de que un pulso `A` o `B`
>   produzca una transición, y **no hay transición que producir**.~~
> - ~~**De la §9, no se ejecuta nada que dependa del puente**: `9.5`, `9.6` en la punta del Esclavo,
>   `9.9`, `9.10`, `9.11` y `9.16`.~~
> - ~~**El hallazgo es ése**, y ya está anotado con sus números.~~
>
> > ### 🔧 CORREGIDO EL MISMO 04/09, DESPUÉS DE N-118 — la conclusión era del firmware, no del cobre
> >
> > Los dos caminos que este apartado escribió el 31/08 daban por bueno un `INPUT_PULLUP` que era
> > **el defecto**. Medido el fuente: `BOTON1`/`BOTON2` ya van en **`INPUT` pelado y activo en
> > ALTO** en las dos puntas, así que **sí hay transición que producir** — cerrando p5 contra p4 y
> > p8 contra p7.
> >
> > **Qué cambia y qué NO cambia en esta ronda:**
> >
> > | | |
> > |---|---|
> > | **Cambia** | el **gesto** (contra 3,3 V, no contra masa) y el **motivo** por el que §8 y la parte de §9 que dependen del puente siguen sin firmarse |
> > | **NO cambia** | **§8 y esas pruebas de §9 siguen SIN EJECUTAR.** No porque el firmware no pueda leer un flanco —eso está corregido— sino porque **no hay tarjeta sana con la que ejercerlo**: la Maestro está fuera de servicio por N-116 y el gesto nuevo **no se ha probado nunca sobre cobre** |
> >
> > **Se marcan «no ejecutable» con este motivo, no con el del `0,6 V`.** Un motivo caducado manda a
> > rediseñar una placa que no lo necesita, igual que uno manda a comprar lo que ya está comprado.
>
> - La entrada al Degradado del **Maestro** por comando sigue en pie (`9.6`, rama *«por comando»*),
>   y **la del Esclavo sigue sin vía practicable hoy** — ver `9.15`.
>
> ### 🛑 Y una prohibición que no estaba y ahora sí, porque se cobró una placa
>
> **En el paso 29 se intentó el puente de todas formas y el STM32 del MAESTRO se sobrecalentó.** No
> se observó ningún cambio de comportamiento en el semáforo —coherente con un pin ya en BAJO—, el
> calor se sintió **sobre el chip, no en el conector**, y se confirmó que el cable sólo tocó p5, p8 y
> masa. El informe **no afirma una causa**: pide inspección en frío antes de cualquier prueba
> eléctrica adicional en `J16`.
>
> > **No se vuelve a puentear `J16` p5/p8 contra masa — nunca, ni tras la inspección.** Desde N-118
> > ese gesto **no mide nada** (los pines son activos en ALTO) y es el que precedió al
> > calentamiento: no hay ninguna razón para repetirlo. **El gesto vigente es contra los 3,3 V del
> > pin de al lado** (p5–p4, p8–p7).
> >
> > 🛑 **Y eso NO es permiso para ejercerlo en esta placa.** La tarjeta Maestro no se energiza hasta
> > la inspección en frío (N-116). Cuando haya tarjeta sana y alguien lo intente, **se hace midiendo
> > la corriente**, no observando las luces: lo que falló aquí no se ve, se toca.

### 0.4 · Lo que NO existe, y por tanto no se prueba

> ⚠️ **Tabla corregida el 04/09.** Tres de sus filas decían *«no existe»* y **ya existen**: se
> construyeron y se montaron en el banco del 3–4/09. La consecuencia sigue siendo la misma —§12
> aplazada— pero **el motivo es otro**, y un motivo caducado manda a comprar lo que ya está comprado.

| Falta | Consecuencia |
|---|---|
| ~~**La placa del módulo ESP32** — no está diseñada, ni fabricada, ni medida~~ · 🆕 **EXISTE y está armada**: fuente conmutada, salida a `J17` ~~(paso 22): … `DS3231` por I²C en `GPIO21`/`GPIO22`~~ 🛑 **corregido el 05/09 — ver §12.6 de este mismo documento, que ya lo había tachado: el «paso 22» es el PLANO (*«cómo tiene que ser»*), no un acta, y el `DS3231` NO ESTÁ COMPRADO.** **Lo que falta es que el módulo se anuncie en el teléfono** (pasos 10 y 25) | §12 sigue aplazada, **por otra razón**: hay hardware, no hay enlace. El chip está descartado como causa —es un `ESP32-WROOM-32` clásico, con el `BR/EDR` que el `SPP` de la app necesita— y el firmware cargó sin errores. **La causa está abierta** |
| ~~**La fuente del ESP32** (DC-DC 12 V→5 V, ≥1 A) — no está pedida ni elegida la referencia~~ · 🆕 **está montada, y SIN MEDIR CON CARGA REAL** | El banco entero se alimentó **por USB**. **Medir esa fuente con 12 V de verdad es un pendiente propio**, independiente del Bluetooth, y va antes de campo |
| ~~**El reloj `DS3231`** — va sobre esa placa, que no existe~~ · 🆕 **montado y cableado** | La §12.6 (Courier RTC) **sigue aplazada**, y ahora se sabe por qué exactamente: **la única vía para leer o poner esa hora es `SET_RTC` desde la app**. Sin enlace, ese reloj no es verificable desde fuera por ninguna vía (paso 27) |
| **El receptor de radio del mando** — nunca se compró | Ver §0.3. ~~⚠️ **Y hoy es peor que una compra pendiente:** los pines del mando miden `0,6 V` en reposo, así que **el receptor tampoco funcionaría** si llegara mañana~~ 🔧 **Caducado el 04/09 (N-118): ese `0,6 V` lo ponía el `INPUT_PULLUP` del firmware, y ya no está.** Con `INPUT` pelado y activo en ALTO, un receptor de contacto seco **sí produciría flanco** cerrando contra los 3,3 V del pin vecino. Vuelve a ser lo que era: **una compra pendiente**, con su decisión de polaridad de salida (NO/NC) y la de N-19 —mismo código o códigos distintos— **antes** de pedirlo |
| **El repetidor y sus dos radios adicionales** | La §6 entera queda aplazada. Además, **la topología vigente es de enlace directo, 2 radios, sin repetidor** |
| **Las dos cámaras de demanda** — pendientes de confirmar si hay una en almacén | La §11.3 queda aplazada |

### 0.5 · Comprobaciones que sí se hacen antes de empezar

- [ ] **0.5.1** Las **dos** radios a `2.4 kbps` de Air Data Rate, mismo canal. Ver `4_Manual_Configuracion_Radios.md`.
- [ ] **0.5.2** Firmware cargado en las dos tarjetas por SWD, **`mode=UR` con `-e all`**. Ver Guía, paso 2.
  - **Si sale `Unable to get core ID`, se reintenta. NO se cambia el modo.** Enganchar es cuestión de
    milisegundos y puede fallar dos o tres veces seguidas; eso no es falta de cableado. Si tras varios
    intentos no entra, el camino determinista es el puente de `BOOT0` que describe la Guía.
  - **El delator de haberlo hecho mal es `NVM size: 128 KBytes (default)`** en un chip de 64 KB.
  - 🟢 **MEDIDO el 3–4/09:** las dos tarjetas cargaron desde VSCode + PlatformIO por SWD con ST-LINK,
    *«escrito y verificado» al primer intento* y **sin necesidad del puente `BOOT0`**. Es la primera
    vez que este paso deja de ser una previsión.
- [ ] **0.5.3** **Las dos tarjetas con la MISMA versión.** Se compara por **`md5`**, nunca por tamaño:
  dos binarios del mismo tamaño pueden ser distintos, y dos de nombre distinto pueden ser el mismo.
  - `md5` Maestro: ________________________  `md5` Esclavo: ________________________
- [ ] **0.5.4** Adaptador USB-TTL conectado a `J17` de cada punta, terminal a 9600, y **`$STATUS`
  llegando ~~cada segundo~~ **cada 2 s** en las dos** *(cadencia bajada a 2000 ms el 04/09)*. Si no llega, se para aquí y se sigue el árbol del apartado 09
  de la Guía. Sin `$STATUS` no se puede operar nada.

---

## 🆔 IDENTIFICACIÓN DEL EQUIPO BAJO PRUEBA

```text
Fecha y hora de inicio: _________________________________________________
Firmware MAESTRO — md5: ______________________  Fecha binario: __________
Firmware ESCLAVO — md5: ______________________  Fecha binario: __________
   -> Son la MISMA version en ambas puntas?   [ ] SI   [ ] NO   <- si es NO, DETENER
Nº de serie Maestro: ______________  Nº de serie Esclavo: ______________
Air Data Rate verificado: __________ kbps      Canal: __________
Via de mando usada:  [ ] USB-TTL a 9600   [ ] ESP32 + app
Puente de mando en J16:  [ ] SI, p5-p4 y p8-p7 (3,3 V del pin contiguo)   [ ] NO
   ~~[ ] SI, tras el paso 20   [ ] paso 20 lo desaconseja~~   <- CADUCADO (N-118, 04/09):
   el paso 20 ya no decide nada, y el puente NUNCA va contra masa. Un cable a p2 no
   produce absolutamente nada con este firmware. Confirmacion = destellos ROJOS, no el terminal.
   Firmware de N-118 cargado y verificado antes de tocar J16?  [ ] SI   [ ] NO  <- si es NO, DETENER
Auditor responsable: ____________________________________________________
```

> ⚠️ **Si las dos tarjetas no llevan la misma versión, deténgase aquí.** El cálculo de la fase del
> Modo Degradado tiene que dar **exactamente el mismo resultado** en las dos puntas. Dos versiones
> distintas pueden calcular fases distintas **sin ningún aviso en ningún sitio** — y ahora, sin
> pantalla, menos todavía.

---

## ⚠️ QUÉ SIGNIFICA Y QUÉ NO SIGNIFICA ESTA RONDA

**Nada de lo que se prueba aquí ha pasado banco, y nada de este documento autoriza a subir nada a
un cruce abierto al tráfico.** En campo sigue la **V8.4**.

La suite de verificación del repositorio sale en verde. Lo que ese verde dice es exactamente esto:
**los modelos y los arneses de PC no encuentran nada. No dice que el firmware funcione sobre la
tarjeta.** Las cifras de esa suite —comprobaciones, porcentajes de flash— **no se copian aquí**
porque se mueven cada hora: viven en las actas de `evidencia/`, con su fecha y el hash de `HEAD`.

> ✅ **Y desde el 04/09 esa frase ya no hay que creerla: está medida.** El paquete `617bd00` bajó al
> banco con su acta entera en verde, y **la sesión se paró tres veces** — por un chip que se
> calienta, un módulo que no se anuncia y una resistencia de 10 kΩ en el cobre. **Ninguna de las tres
> cosas es una propiedad del código fuente**, así que ninguna podía salir en un acta, por buena que
> fuera. *Verde no es entregable* dejó de ser una advertencia y pasó a ser el parte de una sesión.

> 🔴 **Y hay una regresión abierta que es anterior a toda esta arquitectura: N-42, el Modo
> Automático no mueve las luces en banco.** Es lo primero que hay que reproducir. Mientras siga
> abierta, **ningún resultado de las Secciones 3 a 9 se puede dar por bueno**, porque todas
> descansan sobre un ciclo que funcione.
>
> ⚠️ **04/09 — el banco NO la cerró en ninguna de las dos direcciones, y esto se lee entero o se
> lee mal.** El informe: *«esta sesión no la confirma ni la descarta»*. El equipo se quedó en el
> estado de espera de selección de modo —rojo fijo con radio, ámbar intermitente sin ella— porque
> **la única vía para elegir Modo Automático es la app**, y la app no conectó. Los pasos 7, 19 y 21
> de la Guía quedaron a medias por ese mismo motivo, no por el ciclo.
>
> **Lo que NO se puede escribir en un informe a partir de esto:** ni *«N-42 sigue rota»* —nadie la
> vio fallar— ni *«N-42 ya está»* —nadie la vio funcionar—. Es un `ABORTADO`, y un `ABORTADO` **no
> dice nada del firmware**. **Se decide repitiendo el paso 7 con enlace**, y hasta entonces la
> advertencia de arriba vale exactamente igual que el 31/08.

## ℹ️ COMPORTAMIENTOS ESPERADOS — NO son fallas

| Observación | Explicación |
|---|---|
| Al encender, **~2 s con todas las luces apagadas** | Arranque. Tras ese lapso el Maestro fija Rojo |
| El Amarillo previo al Verde dura **4,0 s**; de Verde a Rojo el paso es **directo, 0 s de aviso** | Res. 2024 |
| Al perder la radio, el equipo va a **ámbar intermitente y NO entra solo en Modo Degradado** | Es la regla central de SFTY-21. La entrada es 100 % manual |
| El equipo tarda **25 s**, no 12, en irse a ámbar por silencio | `SFTY6_SILENCIO_MS = 25000UL` — `Maestro/include/protocolo.h:149` y `Esclavo/include/protocolo.h:149`, **MEDIDO**. El techo de 12 s estaba **por debajo** del peor caso de reintentos (20,5 s): los reintentos 4 y 5 no llegaban a ejecutarse |
| 🆕 **Al recuperar el enlace vuelve solo a rojo fijo en ~3 s** | Medido en banco el 3–4/09, en los tres cortes del paso 8. No hay que reiniciar nada |
| 🆕 **La talanquera de `J15` SUBE en ámbar intermitente y baja al volver a rojo** | Medido en banco: `J15` es una **salida directa de motor** (`MOT+`/`MOT−`), no una entrada de disparo. En rojo `0 V` y abajo; en ámbar `12 V` y sube. **No es una avería: es lo que hace el equipo cuando pierde el enlace** |
| 🆕 **El zumbador de `J13` no suena** | Medido en el paso 6. Esperado |
| Las secuencias del mando siguen aceptándose dentro de **12 s** (`A·A·A`, `B·B·B`) y **18 s** (`A·B·A·B`) | `VENTANA_TRIPLE_MS = 12000` y `VENTANA_CUADRUPLE_MS = 18000`. **⚠️ Estos 12 s NO son los 25 s de arriba: son cosas distintas y no se confunden** |
| El `$STATUS` del Esclavo dice siempre `MODO:SUBORDINADO` | Literal fijo. **MEDIDO** |
| `HORA:` en `--:--:--` | El reloj no está en hora. Es un dato |
| Varios comentarios del fuente siguen diciendo *«12 s de silencio»* | **Están caducados** (`Esclavo/src/mando.cpp:113`, `Esclavo/src/main.cpp:396`). El umbral real es 25 s. Anotado, no es fallo de comportamiento |

> ## ⏱️ 04/09 — UN NÚMERO QUE NO CUADRA, Y SE DEJA ESCRITO SIN EXPLICARLO
>
> El firmware declara `SFTY6_SILENCIO_MS = 25000UL`. **El banco cronometró ~20 s**, en los tres
> cortes del paso 8, y el informe lo despacha como *«ligeramente más rápido, diferencia menor sin
> impacto»*.
>
> **Puede que lo sea y puede que no, y este documento no lo decide.** Cinco segundos sobre
> veinticinco son un **20 %**, y ese umbral es justo el que N-71 tuvo que subir de 12 s a 25 s porque
> **estaba por debajo del peor caso de reintentos, 20,5 s**. Un ámbar que llega a los 20 s vuelve a
> caer peligrosamente cerca de ese suelo. Puede ser el cronómetro, puede ser desde cuándo se cuenta
> —el último paquete recibido no es el instante en que se corta la antena—, o puede ser otra cosa.
>
> **Cómo se trata en esta ronda, y es lo único que se pide:** las pruebas **1.3, 2.1, 2.2, 2.3, 9.1
> y 12.3 siguen diciendo 25 s** porque es lo que el firmware declara, **y todas piden segundos
> medidos**. Se anota el número que salga, sin ajustarlo a la expectativa. **Si vuelven a salir ~20 s
> con el cronómetro en la mano, eso es un hallazgo y no una tolerancia** — y se resuelve mirando de
> dónde arranca la cuenta en `main.cpp`, no discutiéndolo en el poste.

---

## 📑 SECCIÓN 1 — ESTADO DE REPOSO E INDEPENDENCIA DE RADIO (SFTY-12)

**1.1 En reposo, Rojo Fijo con enlace activo** — ♻️ **SE REESCRIBE** *(antes: «Rojo Fijo en Menú»)*
- *Qué cambia:* el «Menú» ya no es una pantalla que se abre: es el **estado en el que el equipo
  arranca y en el que se queda** mientras nadie mande un modo. No hace falta llegar a él.
- *Acción:* encender Maestro y Esclavo con la radio enlazada. No mandar ninguna orden.
- *Esperado:* tras los ~2 s de arranque, **ambos en 🔴 ROJO FIJO continuo**, y el `$STATUS` del
  Maestro con `MODO:MENU`.
- `MODO:` leído en el Maestro: ______________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**1.2 Navegación LCD fluida sin radio** — 🚫 **SE RETIRA**
- *Por qué:* medía que la pantalla ST7920 navegara sin trabarse. **Hoy no hay imagen**: los cuatro
  pines del transporte están en `U8X8_PIN_NONE` (`Maestro/src/lcd.cpp:74-75`, **MEDIDO**).
- **No se firma.**

**1.3 Sin comunicación, el equipo se va a ámbar intermitente** — ♻️ **SE REESCRIBE**
- *Qué cambia:* la cifra —**25 s, no 12**— y que el equipo ya no está *«en el Menú»* sino en reposo.
- *Acción:* con las dos puntas en reposo y enlazadas, apagar la radio del Esclavo. Cronometrar.
- *Esperado:* a los **25 s**, **ambas puntas en 🟡 ÁMBAR INTERMITENTE (~1 Hz)**. **Las dos parpadean**,
  ninguna se queda en Rojo ni apagada.
- Segundos medidos: ________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > *El mismo corte, cronometrado tres veces y con el reenganche, está en el **paso 8 de la Guía**.
  > Si ya lo hizo allí, traiga aquí los números y no lo repita.*

**1.4 Arranque de modo sin ciclos fantasma** — ♻️ **SE REESCRIBE** *(antes: «presionar Botón 3»)*
- *Acción:* mandar `CMD:PIN:1234:SET_TIEMPOS:3,3,15` *(era `1,1,15`; el mínimo vial subió a 3 min con N-137 y `1,1,15` hoy da `DESC:RANGO`)* y después `CMD:PIN:1234:SET_MODO:AUTO`.
- *Esperado:* la primera contesta `$ACK,CMD:SET_TIEMPOS,RESULT:OK`; la segunda,
  `$ACK,CMD:SET_MODO:AUTO,RESULT:OK`, y el equipo entra **directo al Despeje Todo-Rojo**, sin
  parpadeos ni saltos de estado extraños.
- Respuesta literal recibida: ______________________________________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

---

## 📑 SECCIÓN 2 — PÉRDIDA DE COMUNICACIÓN Y SELF-HEALING (SFTY-6 / SFTY-9)

> Las cinco se reescriben por lo mismo: **el modo se arranca con `CMD:PIN:1234:SET_MODO:AUTO`**, no
> con un botón. Lo que se observa son las luces, igual que antes.

**2.1 Apagado del Esclavo** — ♻️ **SE REESCRIBE**
- *Acción:* con el sistema en Modo Automático, apagar la radio o la batería del Esclavo. Cronometrar.
- *Esperado:* a los **25 s** el Maestro pasa a **🟡 ÁMBAR INTERMITENTE**.
- Segundos medidos: ________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**2.2 Apagado del Maestro** — ♻️ **SE REESCRIBE**
- *Acción:* con el sistema corriendo, apagar la radio o la batería del Maestro. Cronometrar.
- *Esperado:* a los **25 s** el Esclavo pasa a **🟡 ÁMBAR INTERMITENTE**.
- Segundos medidos: ________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**2.3 Esclavo en Verde cuando cae el enlace (seguridad crítica)** — ♻️ **SE REESCRIBE**
- *Acción:* con el ciclo corriendo, esperar a que el **Esclavo esté en Verde**. Entonces desconectar
  la antena del Maestro.
- *Esperado:* el Esclavo **no puede quedarse en Verde indefinidamente**. Pasa a 🔴 Rojo o a
  🟡 Ámbar intermitente en un máximo de **25 s** (SFTY-6).
- Estado final del Esclavo: ____________  Segundos: ________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**2.4 Self-Healing sin reinicio manual** — ♻️ **SE REESCRIBE**
- *Acción:* con ambos en ámbar por falta de señal, reconectar la radio o la antena. **No tocar la
  alimentación de las tarjetas ni mandar ninguna orden.**
- *Esperado:* reconectan solas.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**2.5 Despeje Todo-Rojo tras la reconexión** — ♻️ **SE REESCRIBE**
- *Acción:* observar las luces inmediatamente después de la reconexión automática.
- *Esperado:* primero **🔴 ROJO FIJO en ambos** para vaciar la vía, y sólo después se abre un carril.
- Segundos de todo-rojo medidos: ________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

---

## 📑 SECCIÓN 3 — MODO AUTOMÁTICO

> **Configuración de la prueba:** ~~`CMD:PIN:1234:SET_TIEMPOS:1,1,15` — **verde 1 minuto, rojo
> 1 minuto, despeje 15 segundos**~~ 🛑 **CADUCADO (N-137, 04/09): el mínimo vial subió a TRES
> minutos y `1,1,15` contesta hoy `$ERR,CMD:SET_TIEMPOS,DESC:RANGO`.** La configuración vigente es
> **`CMD:PIN:1234:SET_TIEMPOS:3,3,15`** — verde 3 min, rojo 3 min, despeje 15 s. Ese es el orden y
> esas son las unidades: los dos primeros en **minutos** y el tercero en **segundos**
> (`modoAutomatico_fijarTiempos(verdeMin, rojoMin, despejeSeg)`,
> ~~`Maestro/src/modo_automatico.cpp:38`~~ → **`:128`**; los límites, en
> **`Maestro/include/limites_ciclo.h:54-56`** — verde y rojo **3-15 min**, despeje **10-90 s**.
> Re-medido el 05/09: las seis constantes **se mudaron de fichero** con N-137, así que la ruta vieja
> ya no las contiene).
>
> ⚠️ **Y el coste va escrito, porque cambia la agenda de la sesión:** ya **NO** se puede probar en
> mesa con ciclos de un minuto. Un ciclo completo son **~6,5 min**, así que **el «Verde 60 s» de
> 3.2 y 3.3 pasa a 180 s, y los «5 ciclos ≈ 12 min» de 3.5 pasan a ≈ 33 min.** Lo dice el propio
> fuente: *«COSTE DECLARADO Y ACEPTADO A SABIENDAS… un banco cae del lado de esperar tres minutos,
> no del lado de dejar el limite de laboratorio suelto en una carretera»* (`limites_ciclo.h:44-46`).
>
> **Los tiempos no se pueden cambiar con el ciclo en marcha**: contesta
> `$ERR,CMD:SET_TIEMPOS,DESC:EN_MARCHA_PARE_EL_MODO`, y ese rechazo es correcto — bajar un tiempo a
> mitad de fase acortaría la fase en curso, y una de esas fases es el todo-rojo de despeje.
>
> 🔴 **Aquí es donde vive N-42.** Si el ciclo no mueve las luces, **se para y se reporta**: es la
> regresión abierta y es lo primero que esta sesión tiene que reproducir.

**3.1 Secuencia lumínica normativa (Res. 2024)** — ♻️ **SE REESCRIBE**
- *Esperado:* Rojo → **Amarillo Fijo 4,0 s** → Verde. Y de Verde a Rojo, **directo, sin amarillo**.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**3.2 Arranque y Turno 1 (Verde Maestro)** — ♻️ **SE REESCRIBE**
- *Esperado:* Todo-Rojo 15 s → Maestro 4 s Amarillo → Maestro Verde 60 s, con Esclavo en Rojo.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**3.3 Turno 2 (Verde Esclavo) — punto crítico histórico** — ♻️ **SE REESCRIBE**
- *Acción:* observar con atención el momento exacto en que el **Esclavo pasa a Amarillo y luego a Verde**.
- *Esperado:* Maestro a Rojo → Todo-Rojo 15 s → Esclavo 4 s Amarillo → Esclavo Verde 60 s. **Sin
  fallo de comunicación y sin que el ciclo se reinicie.**
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > *Si esta prueba falla, verifique primero el Air Data Rate (0.5.1) antes de reportar.*

**3.4 Retorno al Maestro sin falso fallo** — ♻️ **SE REESCRIBE**
- *Esperado:* terminado el verde del Esclavo y su despeje, el Maestro toma el Verde limpiamente.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**3.5 Estabilidad en ciclos sucesivos** — ♻️ **SE REESCRIBE**
- *Acción:* dejar correr **al menos 5 ciclos completos** (≈ 12 min) sin intervenir.
- Ciclos completados sin fallo: ________ de 5
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

---

## 📑 SECCIÓN 4 — MODO INTELIGENTE (demanda por cámara)

> 🔴 **Lo que esta sección ya no puede medir, y hay que decirlo antes de las pruebas.** El *«bus de
> IA»* que recibía las tramas `AI_CARS:X` **se retiró** (N-86): era un `HardwareSerial` declarado
> sobre un puerto que nunca existió, sin un solo llamador, y costaba 280 B de RAM por punta. **No
> hay hoy ningún camino por el que inyectar un conteo de vehículos**, ni ninguna salida por la que
> leerlo. Lo que queda del Modo Inteligente es la **demanda**: un contacto seco que pide paso.

**4.1 Interfaz dedicada (`IA: OK (Autos: X)`)** — 🚫 **SE RETIRA**
- *Por qué:* medía dos cosas y **las dos perdieron su sitio**. El rótulo se dibujaba en la pantalla
  (`lcd_dibujarInteligente()`, `Maestro/src/lcd.cpp:256-269`, que hoy compone un framebuffer que no
  se vuelca), y el contador de autos venía del bus retirado en N-86.
- *La mitad que sobrevive* —que el equipo declare `MODO:INTELIGENTE`— se comprueba en el `$STATUS`,
  y eso ya lo cubre la prueba **12.2**. No se duplica aquí.
- **No se firma.**

**4.2 Cede de paso por demanda** — ♻️ **SE REESCRIBE** *(antes: «inyectar la trama `AI_CARS:X`»)*
- *Acción:* con `CMD:PIN:1234:SET_MODO:INTELIGENTE` puesto, provocar una demanda por **una** de estas
  dos vías, y anotar cuál:
  - cerrando el contacto seco en `J14` (cámara real, o un pulsador suelto — **Guía, pasos 18 y 19**);
  - mandando `CMD:PIN:1234:DEMANDA` al Maestro.
- *Esperado:* `$ACK,CMD:DEMANDA,RESULT:REGISTRADA`, y el cruce **respeta el todo-rojo de despeje**
  antes de conceder el verde a ese sentido. Cronometrar desde el cierre del contacto hasta el verde.
- *Y el rechazo, que es igual de importante:* fuera del Modo Inteligente, `CMD:PIN:1234:DEMANDA`
  tiene que contestar `$ERR,CMD:DEMANDA,DESC:SOLO_EN_MODO_INTELIGENTE`. **Provóquelo a propósito.**
- Vía usada: ____________  Segundos hasta el verde: ________
- Respuesta literal fuera del modo: ______________________________________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**4.3 Fallback por pérdida de cámara (60 s sin datos → `IA: Standby`)** — 🚫 **SE RETIRA**
- *Por qué:* medía qué pasaba al dejar de llegar datos por un bus de IA que ya no existe (N-86), y
  lo comprobaba en un rótulo de pantalla que ya no se dibuja. **No queda ni la entrada ni la salida.**
- **No se firma.**
- > *Hueco declarado:* lo que **sí** convendría poder comprobar —qué hace el Modo Inteligente cuando
  > la cámara de `J14` se queda muda o se queda pegada— **no tiene hoy prueba en este documento**.
  > Escribirla exige antes decidir qué debe hacer el equipo en ese caso, y eso es decisión del
  > responsable, no del auditor.

**4.4 Ciclo completo sin caída** — ♻️ **SE REESCRIBE**
- *Acción:* dejar correr un ciclo completo en Modo Inteligente, observando el paso del Esclavo a Verde.
- *Esperado:* igual que 3.3 — **sin caída de comunicación**.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

---

## 📑 SECCIÓN 5 — MODO MANUAL Y MEDIDA DE ENLACE

> ## 🔵 05/09 — EL MODO MANUAL CAMBIÓ DE COMPORTAMIENTO (`N-147`). LEA ESTO ANTES DE 5.1
>
> **Lo que pasaba, reportado en el banco del 04/09:** *«el botón dar paso maestro queda en rojo,
> pasan 15 seg y … pasa a ámbar intermitente»*. El Modo Manual **entraba por la puerta del Modo
> Automático**, que deja un verde ya programado: **DAR PASO se rechazaba durante 15 s** —el tiempo de
> despeje— **y al vencer el plazo el cruce cambiaba solo**, sin que nadie pulsara.
>
> **Y un tercer defecto que nadie había reportado:** **cada pulsación reiniciaba el plazo**, así que
> pulsando cada 10 s **no se veía el verde nunca** y cada pulsación contestaba `OK`.
>
> **Lo que hace ahora:** al entrar en Manual el equipo **se pone en todo-rojo y se queda quieto, sin
> plazo ninguno**. La primera pulsación de DAR PASO **se acepta**.
>
> 🛑 **`SFTY-4` NO SE HA DEBILITADO, y la prueba 5.2 es la que lo comprueba:** un cambio **de verde a
> verde** sigue pasando por **su todo-rojo y su despeje completos**. Lo único que se dejó de cobrar es
> un despeje **ya cumplido**.
>
> 🔴 **NADA DE ESTO SE HA EJERCIDO EN TARJETA.** Las pruebas 5.0, 5.1 y 5.2 son justo las que lo
> deciden.

**5.0 🆕 Al entrar en Manual, el equipo NO programa nada** — *(prueba nueva, `N-147`)*
- *Acción:* desde cualquier modo, mandar `CMD:PIN:1234:SET_MODO:MANUAL` y **NO tocar nada durante 60
  segundos**, con el cronómetro en la mano y mirando las dos puntas.
- *Esperado:* el cruce va a **todo-rojo** y **se queda ahí**. 🛑 **Si a los ~15 s se pone en ámbar y
  luego da un verde que nadie pidió, el defecto NO está arreglado.**
- Tiempo observado hasta cualquier movimiento espontáneo: ________ s *(lo correcto es: ninguno)*
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**5.0-bis 🆕 Pulsar rápido no esconde el verde** — *(prueba nueva, el tercer defecto de `N-147`)*
- *Acción:* en Manual y con el cruce en rojo, mandar `MANUAL:CAMBIAR_TURNO` **tres veces seguidas,
  separadas ~5 segundos**, sin esperar a que nada acabe.
- *Esperado:* **el verde llega.** 🛑 **Si cada orden contesta `OK` y el cruce nunca abre, es el
  defecto de `tRef` y NO está arreglado** — obedecer y no avanzar es la peor forma de fallar, porque
  no deja rastro de avería.
- ¿Llegó el verde? `[ ] SÍ  [ ] NO` — ¿en cuántas órdenes? ________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**5.1 Primer cambio de carril** — ♻️ **SE REESCRIBE** *(antes: «presionar Botón 1»)*
- *Acción:* con `CMD:PIN:1234:SET_MODO:MANUAL` puesto, mandar `CMD:PIN:1234:MANUAL:CAMBIAR_TURNO`.
- *Esperado:* `$ACK,CMD:CAMBIAR_TURNO,RESULT:OK` y cambio de vía respetando el todo-rojo (mínimo 5 s)
  y los 4 s de Amarillo.
  > ⚠️ **MATIZ DEL 05/09 (`N-147`), y hay que anotarlo o esta prueba se lee como un fallo:** si el
  > cruce **ya lleva un rato en todo-rojo** —que es lo normal recién entrado en Manual— **el despeje
  > ya está cumplido y NO se vuelve a cobrar**: el verde puede entrar **enseguida**, tras sus 4 s de
  > Amarillo. **Eso es lo correcto, no un salto de barrera.** El despeje que sí tiene que verse
  > entero es el de **verde a verde**, y lo mide la prueba 5.2.
- ¿Cuánto tardó en abrir desde la orden? ________ s
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**5.2 Cambios sucesivos** — ♻️ **SE REESCRIBE**
- *Acción:* repetir `CMD:PIN:1234:MANUAL:CAMBIAR_TURNO` una **segunda, tercera y cuarta vez**,
  dejando completar cada cambio.
- *Esperado:* la comunicación se mantiene estable en **todos** los cambios.
- *Y el rechazo honesto:* si se manda **en mitad de una transición**, el equipo tiene que contestar
  `$ERR,CMD:CAMBIAR_TURNO,DESC:EN_TRANSICION_REINTENTE` — **no `$ACK`**. **Provóquelo a propósito**
  mandando la orden justo durante el amarillo.
- Nº de cambios consecutivos sin fallo: ________
- Respuesta literal en transición: ______________________________________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**5.2-bis Medida de calidad de enlace** — ♻️ **SE REESCRIBE** *(antes: pantalla `PRUEBA ALCANCE`)*
- *Acción:* mandar `CMD:SET_MODO:ALCANCE` (**sin PIN**, es una de las dos excepciones). Esperar unos
  15 s a que se estabilice y leer `RF:` y `RTT:` **del `$STATUS` del MAESTRO**.
- *Esperado:* `$ACK,CMD:SET_MODO:ALCANCE,RESULT:OK`. Se espera **100 % de calidad** y **menos de
  1000 ms** de respuesta. Si la calidad no llega al 100 % o la respuesta pasa de 3000 ms, avise antes
  de continuar: indicaría que alguna radio no quedó bien configurada.
- *Y el rechazo:* con un modo en marcha, la orden tiene que contestar
  `$ERR,CMD:SET_MODO:ALCANCE,DESC:EN_MARCHA_PARE_EL_MODO`. **Provóquelo.**

```text
Enlace DIRECTO (2 radios):   RF ______ %     RTT ______ ms
```

- > 🔴 **Se lee el `$STATUS` del MAESTRO y sólo el del Maestro.** En el Esclavo, `RF:98%` y
  > `RTT:85ms` son **literales fijos** y no miden nada (§0.2).
- Resultado: `[ ] MEDIDO  [ ] NO SE PUDO MEDIR` — Observación: ________________________________

**5.3 Rojo total inmediato** — ♻️ **SE REESCRIBE** *(antes: «Botón 3»)*
- *Acción:* mandar `CMD:FORZAR_ROJO` (**sin PIN**, a propósito: quien ve el incidente tiene que
  poder pararlo aunque no se sepa el PIN) en **cualquier estado en que estén las luces**.
- *Esperado:* `$ACK,CMD:FORZAR_ROJO,RESULT:OK` y ambos pasan **de inmediato a 🔴 ROJO FIJO**,
  manteniéndose así de forma indefinida. No deben conmutar solos pasados unos segundos.
- Tiempo observado en Rojo sin conmutar: ________ *(esperar al menos 60 s)*
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**5.4 Reanudación desde el Rojo total** — ♻️ **SE REESCRIBE**
- *Acción:* estando en Rojo por 5.3, mandar `CMD:PIN:1234:MANUAL:CAMBIAR_TURNO`.
- *Esperado:* reanuda abriendo el carril correspondiente, respetando el despeje.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**5.5 Vuelta al reposo** — ♻️ **SE REESCRIBE** *(antes: «Botón 4 — Retorno a Menú»)*
- *Acción:* mandar `CMD:SET_MODO:MENU` (**sin PIN**).
- *Esperado:* `$ACK,CMD:SET_MODO:MENU,RESULT:OK` y ambos semáforos quedan en **Rojo Fijo** con el
  enlace activo. **Entrar en reposo no arranca ningún ciclo.**
- *Y las otras dos respuestas, que también hay que ver:*
  - mandado **estando ya en reposo** → `$ERR,CMD:SET_MODO:MENU,DESC:YA_VUELVE_AL_MENU`;
  - mandado **desde Modo Degradado** → `$ACK,CMD:SET_MODO:MENU,RESULT:SALIENDO_TODO_ROJO`, y el
    equipo **no salta al reposo de golpe: pide la salida por el todo-rojo**. Es lo correcto.
- Respuestas literales obtenidas: ______________________________________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**5.6 Menú de dos niveles y layout de la pantalla** — 🚫 **SE RETIRA**
- *Por qué:* medía que las opciones de un menú **cupieran dibujadas** en los 128×64 de la ST7920,
  porque la librería recorta en silencio y el cursor sí llega a una opción invisible. Hoy no se
  dibuja nada en ningún cristal: los pines están en alta impedancia y **no hay menú que abrir**,
  porque no hay `Aceptar`.
- **No se firma.**

**5.7 Diagnóstico de línea RS-485 en pantalla (`RX <bytes> <n> tr`)** — 🚫 **SE RETIRA**
- *Por qué:* la fila inferior donde se leía ya no se dibuja.
- **No se firma.**
- > 🔴 **Hueco declarado, y es una pérdida real de capacidad de diagnóstico.** Ese renglón
  > distinguía tres cosas que desde fuera se parecen: *no llega nada* · *llega y es basura* ·
  > *llega bien*. La segunda apunta a cableado, línea suelta o radio emitiendo ruido, y evitaba
  > desplazamientos al poste con instrumentos. **Los contadores siguen existiendo en el firmware;
  > lo que no existe es por dónde leerlos.** Recuperarlo es **trabajo de firmware** —añadirlos al
  > `$STATUS`—, no una compra. Queda anotado, no cerrado.

---

## 📑 SECCIÓN 6 — REPETIDOR ESP32 (topología de 4 radios)

> ⏸️ **LA SECCIÓN ENTERA SE APLAZA — sus 6 pruebas (6.0, 6.0-bis, 6.1, 6.2, 6.3, 6.4).**
>
> **Qué falta, en concreto:**
> - **Dos radios más** (`B1` y `B2`) con sus antenas y coaxiales.
> - **La placa del repetidor** con su transceptor `MAX3485` y su ESP32, alimentada.
> - Que el repetidor esté **flasheado** con su firmware.
>
> **Y una razón que no es de material:** la configuración vigente del sistema es de **2 radios en
> enlace directo, sin repetidor**. Mientras eso no cambie, esta sección certifica una topología que
> no se está desplegando. **Si el funcional necesita certificarla, hay que decidirlo antes de
> comprar las radios, no después.**
>
> **Ninguna de las seis lleva casilla. No se firman.**
>
> *(El contenido de la revisión anterior —el diagnóstico de cadena por LEDs contando destellos cada
> 3 s, el árbol A/B/C/D/E y el firmware `repetidor_diag`— sigue siendo válido y no se ha borrado del
> histórico: está en `roadmap.md`. Se recupera tal cual el día que haya cuatro radios.)*

---

## 📑 SECCIÓN 7 — RELOJ Y SINCRONIZACIÓN POR RADIO (SFTY-18 / SFTY-23)

> **Requisito de la Sección 9.** Sin reloj en hora y sincronizado, el Modo Degradado **no entra**,
> así está construido.
>
> **Lo que cambia en bloque:** la hora ya no se teclea dígito a dígito en una pantalla. Entra en
> **un solo comando**, `CMD:PIN:1234:SET_RTC:AAAA-MM-DD,HH:MM:SS`, y **el equipo contesta con tres
> respuestas distintas que no significan lo mismo**. Ésa es la propiedad central de la sección.

**7.1 El reloj arranca declarándose NO fiable, y `SET_RTC` no miente** — ♻️ **SE REESCRIBE**
- *Acción:* con una tarjeta **nunca puesta en hora** (o con la pila retirada), encender y leer
  `HORA:` en su `$STATUS`. Después mandar `CMD:PIN:1234:SET_RTC:2026-08-31,10:00:00`.
- *Esperado:* `HORA:` en `--:--:--` — **no una hora cualquiera como si fuera buena**. Y la orden
  contesta **una** de estas tres, que hay que copiar literal:

| respuesta | qué significa |
|---|---|
| `$ACK,CMD:SET_RTC,RESULT:OK` | la hora entró **y** va camino del Esclavo |
| `$ACK,CMD:SET_RTC,RESULT:HORA_PUESTA_SIN_PROPAGAR` | la hora entró en esta punta, **pero no se propagó** |
| `$ERR,CMD:SET_RTC,DESC:SIN_CRISTAL_VEA_CONSULTA_RELOJ` | **no hay con qué contar el tiempo.** La hora **no** quedó puesta |
| `$ERR,CMD:SET_RTC,DESC:FORMATO_INVALIDO` | la orden no se entendió. El reloj **no se tocó** |

- Respuesta literal: ______________________________________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > 🔴 **Un `$ACK` aquí sobre una tarjeta que luego pierde la hora es un hallazgo importante**, no
  > una molestia: es el equipo diciendo que sí y no haciéndolo, y el técnico se va del poste creyendo
  > que dejó el reloj puesto. **Se anota literalmente lo que contestó.** *(Guía, paso 27.)*

**7.2 Edición dígito a dígito** — 🚫 **SE RETIRA**
- *Por qué:* medía el subrayado del dígito activo y el recorrido con Botones 1/2/3 en una pantalla
  de edición que ya no se puede abrir ni ver. La hora entra hoy en un solo comando.
- **No se firma.**

**7.3 Salir con Botón 4 no altera la hora** — 🚫 **SE RETIRA**
- *Por qué:* medía que la pantalla de edición trabajara sobre una copia y sólo escribiera al
  confirmar, para que **entrar por error no rompiera la hora**. Sin pantalla de edición no hay
  copia, no hay confirmación y no hay error de entrada posible.
- *Lo que hereda esa preocupación* —que una orden mal formada **no toque** el reloj— se comprueba en
  **7.1**, con `FORMATO_INVALIDO`. No se duplica.
- **No se firma.**

**7.4 La pantalla NO arranca ciclos** — 🚫 **SE RETIRA**
- *Por qué:* no hay pantalla en la que permanecer. La propiedad equivalente —que volver al reposo no
  arranque nada— se comprueba en **5.5**.
- **No se firma.**

**7.5 Contraste contra hora patrón** — ♻️ **SE REESCRIBE** *(antes: leído en pantalla)*
- *Acción:* poner el Maestro en hora contra un celular con hora de red. Dejarlo **al menos 2 h**
  encendido y volver a comparar, leyendo `HORA:` del `$STATUS`.
- *Esperado:* la diferencia es de **pocos segundos**. Un cristal sin calibrar deriva ~30–50 ppm, es
  decir **menos de 1 s en 2 h**.
- Hora patrón: ________  `HORA:` del equipo: ________  Diferencia: ________ s
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**7.6 La hora sobrevive al corte de energía (valida la pila)** — ♻️ **SE REESCRIBE**
- *Acción:* con el equipo en hora, **desconectar la alimentación 10 minutos**. Volver a energizar y
  leer `HORA:`.
- *Esperado:* al arrancar, el equipo **conserva la hora**; no vuelve a `--:--:--`.
- Maestro — `HORA:` al volver: ________  Diferencia contra patrón: ________ s — `[ ] CUMPLE [ ] NO CUMPLE`
- Esclavo — `HORA:` al volver: ________  Diferencia: ________ s — `[ ] CUMPLE [ ] NO CUMPLE`
- > **Si falla, la pila no está haciendo su trabajo.** Revise que `R5` esté retirado y el positivo
  > soldado al pad de `VBAT` (`2_Manual_Hardware_y_Pruebas.md §5`).
  > **Sólo importa cuál de las dos falla.** Si la que pierde la hora es la del Esclavo, no hay que
  > comprar nada: la coge del Maestro por radio. *(Guía, paso 27.)*

**7.7 Veredicto del cristal `Y2`** — ♻️ **SE REESCRIBE** *(antes: la pantalla `CONSULTA RELOJ`)*
- *Qué cambia:* la pantalla de diagnóstico con sus cuatro líneas y su punto parpadeante **ya no se
  puede leer**. El veredicto se pide ahora con un comando, y **el comando distingue los dos casos
  por sí solo**.
- *Acción:* mandar `CMD:PIN:1234:REINICIAR_RELOJ`.
- *Esperado:* **una** de estas dos, y cualquiera de las dos **es** el veredicto:

| respuesta | qué significa |
|---|---|
| `$ACK,CMD:REINICIAR_RELOJ,RESULT:CRISTAL_OK_PONGA_LA_HORA` | **el oscilador arranca. El cristal NO era el problema.** No cambie ningún componente |
| `$ERR,CMD:REINICIAR_RELOJ,DESC:SIGUE_PARADO_VEA_CONSULTA_RELOJ` | **se pidió el oscilador y no arranca.** Aquí sí apunta al `Y2` y su entorno |

- *Y la segunda mitad de la prueba, que es de seguridad:* con la tarjeta **desenergizada**,
  desconectar el cristal `Y2` y energizar. **El equipo tiene que bootear con normalidad y encender
  las luces**, declarando la hora no fiable. **No debe quedarse colgado y a oscuras.**
- Respuesta literal: ______________________________________
- Con `Y2` desconectado, ¿arrancan las luces? ____________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > **No sustituya ningún componente en esta sesión.** El objetivo es traer la lectura. Ya se dio por
  > culpable a este cristal una vez sin haber medido nada, y se cambiaron piezas sanas.
  > **Hay que hacerlo en las DOS tarjetas: una está diagnosticada, la otra no.**
- > ⚠️ El texto `VEA CONSULTA RELOJ` que aparece en la respuesta **remite a una pantalla que hoy no
  > se puede ver**. No es un fallo del equipo: es un mensaje caducado. Anótelo y siga.

**7.8 Poner en hora SINCRONIZA en el mismo gesto** — ♻️ **SE REESCRIBE**
- *Acción:* con **enlace de radio activo**, poner la hora en el **Maestro** con `SET_RTC`. Leer
  enseguida `HORA:` del `$STATUS` **del Esclavo**, por su propio USB-TTL. **Nadie toca el Esclavo.**
- *Esperado:* el Maestro contesta `RESULT:OK` (no `HORA_PUESTA_SIN_PROPAGAR`) y el Esclavo muestra
  **la misma hora, con segundos**.
- `HORA:` Maestro: ______:______:______   `HORA:` Esclavo: ______:______:______
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**7.9 Desfase entre las dos puntas — es un NÚMERO** — ♻️ **SE REESCRIBE**
- *Qué cambia, y a mejor:* antes se leía la medida interna del Maestro, que **transporta sólo el
  segundo** y por eso confundía un desfase de 45 s con uno de −15 s. Ahora se comparan directamente
  los dos `HORA:HH:MM:SS`, que **no tienen ese alias**.
- *Acción:* con los dos terminales abiertos a la vez, capturar el `$STATUS` de cada punta y comparar
  los campos `HORA:` del mismo segundo de pared.

```text
HORA: Maestro ______:______:______     HORA: Esclavo ______:______:______
Desfase calculado: ________ s
```

- *Esperado:* desfase **dentro de ±3 s** tras una sincronización reciente.
- Resultado: `[ ] MEDIDO  [ ] NO SE PUDO MEDIR` — Observación: ________________________________
- > *Por qué se anota y no se mira:* dos relojes separados 40 s muestran el mismo `14:32`. Cuarenta
  > segundos es **más que el todo-rojo entero**. Los segundos no son un adorno.
- > *Sesgo conocido, no lo persiga:* las dos capturas no son simultáneas al milisegundo. Frente a un
  > todo-rojo de 30 s, unas décimas son irrelevantes.

**7.10 Resincronización periódica cada hora** — 🚫 **SE RETIRA**
- *Por qué:* medía que la **marca de última sincronización** se actualizara sola, y esa marca se leía
  en pantalla. **Hoy no hay ningún campo de telemetría que la exponga.**
- *Y por qué no se sustituye por «comprobar que los relojes siguen cuadrados»:* en una hora la deriva
  de dos cristales es de **~0,36 s**, muy por debajo de la resolución de `HORA:`. Los dos relojes se
  verían cuadrados **hayan resincronizado o no**. Sería una prueba que **no puede fallar**, y una
  prueba que no puede fallar no mide nada — sólo enseña a firmar.
- **No se firma.**
- > 🔴 **Hueco declarado.** Exponer la antigüedad de la última sincronización en el `$STATUS` es
  > trabajo de firmware, y hasta que exista **la frescura de la sincronización no es verificable
  > desde fuera del equipo**. Importa porque es justo la garantía en la que se apoya el Modo
  > Degradado (ver 9.4).

**7.11 El Esclavo NO ofrece ajuste de hora** — 🚫 **SE RETIRA**
- *Por qué, y no es sólo la pantalla:* la prueba recorría el menú del Esclavo para confirmar que no
  existía `AJUSTAR HORA`, apoyada en la regla *«la hora se cuadra una sola vez en el Maestro y viaja
  por radio»*. **El firmware de hoy ya no cumple esa regla:** el Esclavo **acepta**
  `CMD:PIN:1234:SET_RTC:` y contesta `$ACK,CMD:SET_RTC,RESULT:OK` (**MEDIDO**, `Esclavo/src/bluetooth.cpp`).
- **No se firma como estaba.** Lo que hoy hay que comprobar del `SET_RTC` del Esclavo —que rechace
  con `SIN_CRISTAL` cuando no hay reloj y con `FORMATO_INVALIDO` cuando la orden está mal— se hace
  en **7.6** y **12.7**.
- > 🔴 **Esto es una decisión pendiente, no un simple ajuste de documento.** Poner la hora del
  > Esclavo a mano **reintroduce exactamente el desfase que la sincronización por radio elimina**, y
  > el Modo Degradado descansa en que las dos puntas cuenten igual. Que el Esclavo acepte `SET_RTC`
  > es lo que hace posible el *Courier RTC* (12.6) cuando no hay radio — **es una capacidad
  > deliberada, no un descuido** —, pero la regla escrita en los manuales dice lo contrario. **Hay
  > que decidir cuál de las dos manda, y es decisión del responsable.**

---

## 📑 SECCIÓN 8 — MANDO DE 4 RELÉS Y SECUENCIAS (SFTY-21)

> ## ⛔ 04/09 — ESTA SECCIÓN SIGUE SIN EJECUTARSE, PERO **EL MOTIVO HA CAMBIADO EL MISMO DÍA**
>
> ~~El banco del 3–4/09 midió el paso 20: **`J16` p5 y p8 dan `9,92 kΩ` contra masa y `0,6 V` en
> reposo**. Es exactamente la rama que el párrafo de abajo describe. **El mando está inoperante de
> fábrica: el pin ya se lee como accionado y no hay flanco que provocar**, ni con el puente ni con el
> receptor que falta comprar.~~
>
> 🔧 **CORREGIDO POR N-118, con el fuente delante: ese `0,6 V` lo producía el `INPUT_PULLUP` DEL
> FIRMWARE contra los 10 kΩ del cobre, y ese firmware ya no existe.** `BOTON1` y `BOTON2` van en
> **`INPUT` pelado y activo en ALTO** en las dos puntas, igual que las cámaras. **El mando no está
> sordo: el gesto para pulsarlo es otro** — p5 contra **p4** y p8 contra **p7** (3,3 V del pin de al
> lado), **nunca contra masa**. Lea §0.3, que trae el gesto entero.
>
> **Sus cinco pruebas con casilla —8.2, 8.3, 8.4, 8.5 y 8.7— siguen sin firmarse en esta ronda, y
> por un motivo distinto:** **no hay tarjeta sana con la que ejercerlas.** La Maestro está fuera de
> servicio por el corto de N-116, y **la tensión de p5/p8 con el puente a 3,3 V y este firmware
> cargado NO SE HA MEDIDO NUNCA** — el paso 29 no llegó a ese dato. No se marcan `NO CUMPLE`, que
> acusaría al firmware de algo que no ha podido hacer: se marcan **«no ejecutable»** con ese motivo.
> *(`ABORTADO` no es `NO CUMPLE`, igual que no es `CUMPLE`.)*
>
> 🛑 **Y no se reintenta el puente en la placa dañada.** En el paso 29 se intentó y **el STM32 del
> Maestro se sobrecalentó**; esa placa necesita inspección técnica en frío antes de cualquier prueba
> eléctrica en `J16`. Ver §0.3.
>
> **Lo que queda de la §8 no es una casilla: es la primera sesión de banco que ejerza el mando con
> el firmware de N-118 cargado.** ~~Se investiga el origen de esa red de ~10 kΩ y se decide si hay
> ajuste de diseño **antes** de comprar el receptor.~~ *(Caducado: la red de 10 kΩ es correcta y está
> cerrada como `M3`. Lo que queda antes de comprar el receptor es la polaridad de su salida —NO/NC—
> y la decisión de N-19.)*

> **Lea §0.3 antes de esta sección.** ~~El paso 20 de la Guía tenía que decir si `J16` p5 y p8 llevan
> unos 10 kΩ a masa. **Lo dice: `9,92 kΩ`.** **El mando está inoperante y esta sección no se
> ejecuta**: se anotan los números del paso 20 y se para. Ése es el hallazgo, y ya está anotado.~~
> *(Caducado el 04/09: esos 10 kΩ son la resistencia de reposo correcta para una entrada activa en
> ALTO, que es como el firmware las lee desde N-118.)*
>
> **Sólo aplica al MAESTRO** salvo donde se diga. El Esclavo tiene sus propias secuencias en el
> firmware, y en él se ejercen en la Sección 9.
>
> ⚠️ **Con el puente ~~a masa~~ a 3,3 V, un pulso es instantáneo. El relé real tarda ~2 s.** Deje al
> menos un segundo entre pulsos: si los da demasiado juntos puede quedar por debajo del filtro de
> rebote.
>
> 🛑 **Y las dos reglas de §0.3 que deciden si esta sección mide algo:**
> 1. **El gesto es `p5` contra `p4` y `p8` contra `p7`** —los 3,3 V del pin contiguo—, **jamás
>    contra masa.** Hay **una sola masa** en todo `J16` (`p2`), y un cable a ella no produce
>    absolutamente nada con el firmware de N-118.
> 2. **Se prueba DESDE OTRO MODO, y lo que se anota son los DESTELLOS.** Si el equipo ya está en el
>    modo que la secuencia pide, `MODO:` no cambia y la prueba no distingue nada. **2 destellos
>    rojos = Automático · 3 = Ámbar · 4 = Degradado · ámbar rápido de 2 s = rechazado.**

**8.1 El mando navega el menú igual que la botonera** — 🚫 **SE RETIRA**
- *Por qué:* no hay menú que navegar ni cursor que mover, y `C` y `D` no existen como canales.
- **No se firma.**

**8.2 `A · A · A` → AUTOMÁTICO, con 2 destellos rojos** — ♻️ **SE REESCRIBE** *(entrada por puente)*
- *Acción:* con el equipo **fuera de cualquier modo**, dar tres pulsos `A` (~~`J16` p5 a masa~~
  **`J16` p5 contra p4 — los 3,3 V del pin de al lado, N-118**) en **menos de 12 s**.
- *Esperado:* **2 destellos ROJOS contables**, y luego el equipo intenta Modo Automático.
- Destellos contados: ________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**8.3 `B · B · B` → ÁMBAR, con 3 destellos rojos** — ♻️ **SE REESCRIBE** *(entrada por puente)*
- *Acción:* tres pulsos `B` (~~`J16` p8 a masa~~ **`J16` p8 contra p7 — los 3,3 V del pin de al
  lado, N-118**) en **menos de 12 s**.
- *Esperado:* **3 destellos ROJOS** y el equipo pasa a 🟡 Ámbar intermitente.
- Destellos contados: ________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**8.4 `B · B · B` funciona DESDE CUALQUIER ESTADO** 🛟 — ♻️ **SE REESCRIBE**
- *Acción:* repetir 8.3 desde **cada uno** de estos estados: Modo Manual, Modo Automático a mitad de
  verde, Modo Inteligente, y Modo Degradado activo.
- *Esperado:* en **todos** los casos va a ámbar, **sin condiciones ni rechazos**.
- Estados probados con éxito: ______ de 4
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > *Por qué se prueba desde todos:* es la regla que **impide que nadie quede atrapado** con un
  > semáforo en un estado raro a 5 m de altura. Si falla desde algún estado, es un hallazgo de
  > seguridad. **Es la salida de emergencia: si pide algo para funcionar, no es una salida de
  > emergencia.**

**8.5 Los destellos son ROJOS, nunca verdes** 🔴 — ♻️ **SE REESCRIBE**
- *Acción:* observar las confirmaciones **desde lejos**, como lo haría un conductor que se acerca.
- *Esperado:* **sólo destella el ROJO.** En ningún momento se enciende el verde ni los tres colores.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > *Por qué importa:* el operario cuenta destellos **sin ver ninguna pantalla** — y ahora eso ya no
  > es una hipótesis de uso, es la única forma que hay. Si cuenta mal, el peor caso debe seguir
  > siendo seguro, y **el rojo nunca significa «pase»**.

**8.6 Con el menú abierto, las secuencias se ignoran** — 🚫 **SE RETIRA**
- *Por qué:* **la condición no se puede provocar.** `menu_estaAbierto()` no puede ser cierto, porque
  se abre con `botonAceptar()` y ése devuelve `false` siempre (`Esclavo/src/botones.cpp:316-317`,
  **MEDIDO**). La función `secuenciasInhibidas()` sigue en el fuente de las dos puntas y **se ha
  quedado sin sujeto**.
- **No se firma.**
- > 🔴 **Se retira la prueba, no la preocupación.** El riesgo que cubría —una ráfaga accidental de
  > pulsos confirmando una hora inventada— **desapareció con la pantalla de ajuste**, así que la
  > retirada es honesta. Lo que queda anotado para desarrollo es distinto: **hay una barrera de
  > seguridad en el firmware que hoy no puede activarse nunca.** Un código muerto dentro de una regla
  > de seguridad no es neutro, y merece decisión: se conecta a otra condición, o se retira a
  > sabiendas.

**8.7 Ráfagas accidentales no dejan el equipo en estado peligroso** — ♻️ **SE REESCRIBE**
- *Acción:* dar pulsos `A` y `B` de forma desordenada durante un minuto (simulando un pulso espurio
  o un operario confundido).
- *Esperado:* el equipo termina en **Automático, en Ámbar, o donde estaba** — nunca en un estado
  peligroso, y **nunca en Degradado si los requisitos no se cumplen**.
- Estado final: ______________________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**8.8 `C` y `D` NO forman secuencias** — 🚫 **SE RETIRA**
- *Por qué:* **ya no hay canales `C` ni `D`.** Sus pines (`J16` p10 y p12, `PB14`/`PB15`) pasaron a
  ser entradas de cámara. No hay nada que pulsar y por tanto nada que no deba dispararse.
- **No se firma.**

**8.9 Que los pulsos lleguen desde el piso, por radio** ➕ **NUEVA** — ⏸️ **SE APLAZA**
- *Qué mediría:* que el mando funcione **en su condición real de uso**: alguien de pie en el suelo,
  a distancia, sin ver el equipo. Es lo que el puente de §0.3 **no** demuestra.
- **Falta: el receptor de radio del mando. Nunca se compró.** ~~La tarjeta tiene las entradas
  preparadas; falta el aparato.~~
- ~~🔴 **04/09 — y esa última frase era falsa, lo demostró el paso 20: las entradas NO están
  preparadas.** `J16` p5 y p8 tienen ~10 kΩ a masa y se quedan en `0,6 V`, o sea BAJO permanente.
  **Un receptor comprado mañana se enchufaría a un pin que ya lee «accionado» y no produciría ningún
  flanco.** Comprarlo antes de resolver esa red de resistencias es gastar dinero en un aparato que
  no puede funcionar.~~
- 🔧 **04/09, más tarde el mismo día (N-118) — y la corrección de arriba también era falsa a
  medias.** Ese `0,6 V` no salía del cobre solo: lo producía **el `INPUT_PULLUP` del firmware**
  contra esos 10 kΩ. Con `BOTON1`/`BOTON2` en **`INPUT` pelado y activo en ALTO**, esos 10 kΩ pasan
  a ser exactamente la resistencia de reposo que hace falta —lo mismo que ya ocurría en las cámaras
  `C` y `D`, medidas a `0 V`—, y **un receptor de contacto seco cerrando contra los 3,3 V del pin
  vecino sí produciría flanco**. **Las entradas SÍ están preparadas; lo que falta es el aparato.**
  Lo que hay que decidir antes de comprarlo es la **polaridad de su salida (NO/NC)** y la decisión
  de N-19 —mismo código o códigos distintos en las dos puntas—, no un rediseño de la placa.
  *(Se conservan las dos frases tachadas a propósito: es la misma medida leída tres veces, y la
  tercera es la que tenía el fuente delante.)*
- **No se firma.** *(Guía, paso 29: si no lo tiene, se marca «No se pudo» con ese motivo — **nunca**
  como correcto.)*
- > ⚠️ **Y hay una decisión de seguridad abierta antes de comprarlo (N-19):** si se quieren mandos en
  > las dos puntas, ¿llevan **el mismo código o códigos distintos**? Un solo código metería las dos
  > torres en Degradado a la vez desde el piso, **saltándose la verificación de cada punta**;
  > códigos distintos obligan a comprobar torre por torre. **No está decidido. Que el funcional
  > opine antes de comprar nada.**

---

## 📑 SECCIÓN 9 — MODO DEGRADADO (SFTY-21)

> ## ⚠️ ANTES DE EMPEZAR ESTA SECCIÓN
>
> **1. Lea completo el `8_Procedimiento_Modo_Degradado.md`**, y en particular los riesgos residuales
> de su Sección 6.
>
> **2. Estas pruebas se hacen con el tramo CERRADO al tráfico.** Varias provocan deliberadamente el
> escenario de verde en una punta y ámbar en la otra.
>
> **3. La Sección 7 debe estar CUMPLE** — sin reloj en hora y sincronizado, el modo no entra.
>
> **4. La entrada en el ESCLAVO sólo es posible por el puente de `J16` (§0.3).** No hay comando, no
> hay pantalla y no hay receptor. Si el paso 20 de la Guía desaconseja el puente, **de 9.6 en
> adelante no se ejecuta nada** y se anota el motivo.
>
> > ⛔ **04/09 — ~~el paso 20 ya se midió y lo desaconseja: `0,6 V` en reposo, `9,92 kΩ` a masa.~~**
> > 🔧 **Motivo corregido el mismo día por N-118:** ese `0,6 V` lo ponía el `INPUT_PULLUP` del
> > firmware, que ya no está; el puente **sí funcionaría**, dado **contra los 3,3 V del pin de al
> > lado** (p5–p4, p8–p7) y **no contra masa** — ver §0.3. **Lo que impide ejecutar sigue siendo
> > cierto y es otro: no hay tarjeta sana.** La Maestro está fuera de servicio por N-116 y el gesto
> > nuevo nunca se ha ejercido sobre cobre.
> > Por tanto, **de 9.6 en adelante no se ejecuta nada que necesite el puente** — es decir `9.5`,
> > la punta del Esclavo de `9.6`, `9.9`, `9.10`, `9.11` y `9.16`. Y **no se reintenta**: el puente
> > se cobró el sobrecalentamiento del Maestro en el paso 29 (§0.3).
> >
> > **Lo que SÍ queda ejecutable de esta sección, y conviene no perderlo:** `9.1` —que el equipo **no
> > entre solo** en Degradado, que es *la prueba más importante de la sección*—, los tres rechazos
> > `9.2`, `9.3` y `9.4`, la entrada del **Maestro** por comando en `9.6`, `9.14` y `9.15`. Todo eso
> > entra por `J17` y no toca `J16`.
> >
> > **Y hay que decirlo entero: sin la punta del Esclavo, `9.7` y `9.8` tampoco se pueden montar**,
> > porque las dos parten de *«con las dos puntas en Degradado»*. `9.7` —la verificación visual de
> > dos ciclos completos, la que mira que no haya verde simultáneo— **es la comprobación por la que
> > existe la sección**. Queda **no ejecutable**, no aprobada.
>
> Material: forma de **cortar el radio** y **dos observadores**, uno en cada punta, comunicados.

**9.1 Sin radio, el comportamiento por defecto SIGUE siendo el ámbar** ✅ — ♻️ **SE REESCRIBE**
- *Acción:* con el sistema en Modo Automático, desconectar la antena del Maestro. Esperar
  **5 minutos sin tocar nada**.
- *Esperado:* a los **25 s** ambas puntas van a 🟡 Ámbar intermitente **y se quedan ahí**. **El
  equipo NO entra solo en Modo Degradado, ni a los 5 minutos ni nunca.**
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > *Es la prueba más importante de la sección.* Si el equipo entrara solo, estaría dando verde **sin
  > poder saber si la otra punta sigue viva** — podría estar apagada, colgada o movida a otra obra.
  > **Si esta prueba no cumple: RECHAZAR.**

**9.2 Rechazo con el reloj sin poner en hora** — ♻️ **SE REESCRIBE**
- *Acción:* con el Maestro **sin hora fiable**, mandar `CMD:PIN:1234:SET_MODO:DEGRADADO`.
- *Esperado:* **lo rechaza** con `$ERR,CMD:SET_MODO:DEGRADADO,DESC:<motivo>`, y **el motivo es el
  dato**: se copia literal, sin resumir.
- Respuesta literal: ______________________________________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**9.3 Rechazo sin sincronización previa por radio** — ♻️ **SE REESCRIBE**
- *Acción:* con la hora puesta pero **sin que haya habido nunca sincronización** con la otra punta,
  mandar `CMD:PIN:1234:SET_MODO:DEGRADADO`.
- *Esperado:* **rechazado**, con su motivo en el `$ERR`.
- Respuesta literal: ______________________________________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**9.4 Rechazo con sincronización caducada (más de 2 h)** — ♻️ **SE REESCRIBE**
- *Acción:* sincronizar, dejar el radio caído **más de 2 horas**, y mandar la orden.
- *Esperado:* **rechazado**, con su motivo.
- Horas transcurridas: ________  Respuesta literal: ______________________________________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > *Por qué no basta con medir el desfase:* la medida interna transporta **sólo el segundo**, así que
  > un desfase real de 45 s **se lee como −15 s**. Lo que cierra ese agujero es **la frescura**: tras
  > una sincronización de hace una hora la deriva es de ~0,36 s y la medida no puede estar
  > equivocada. **El desfase es una comprobación de cordura; la garantía es la sincronización
  > reciente.**

**9.5 Rechazo desde el piso: ámbar rápido en vez de destellos** — ♻️ **SE REESCRIBE** *(por puente)*
— ⛔ **NO EJECUTABLE (04/09): necesita el puente de `J16`** ~~, medido a `0,6 V`~~ **— y no hay
tarjeta sana con la que darlo (N-116). El puente en sí SÍ funciona con el firmware de N-118: p5
contra p4, p8 contra p7, nunca contra masa. Ver §0.3 y el bloque de marcas**
- *Acción:* con alguno de los requisitos sin cumplir, dar `A · B · A · B` en menos de 18 s.
- *Esperado:* **NO hay 4 destellos rojos.** Aparece un **ámbar rápido** que significa *rechazado*, y
  el equipo **no entra**.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > *Por qué se distingue así:* los destellos rojos significan **«hecho»**. Un rechazo no puede
  > confirmarse con el mismo lenguaje que un éxito, o el operario se va creyendo que quedó activo.

**9.6 Entrada correcta en las DOS puntas** 👁️ — ♻️ **SE REESCRIBE**
— ⛔ **PARCIAL (04/09): la rama del MAESTRO por comando sigue viva; la del ESCLAVO no, porque su
única vía es el puente de `J16`** ~~y está medido a `0,6 V`~~ **y no hay tarjeta sana con la que
darlo (N-116) — el puente en sí funciona desde N-118**
- *Acción:* cumplidos los requisitos y cortado el radio, entrar al Degradado en cada punta:
  - **MAESTRO:** `CMD:PIN:1234:SET_MODO:DEGRADADO` → `$ACK,...,RESULT:OK`, **o** `A·B·A·B` por puente.
  - **ESCLAVO:** **sólo `A·B·A·B` por el puente de `J16` p5/p8** — ~~contra masa~~ **cerrando p5
    contra p4 y p8 contra p7 (los 3,3 V del pin de al lado), N-118**. No hay otra vía.
- *Esperado:* las dos puntas entran, y la del mando confirma con **4 destellos ROJOS**.

```text
MAESTRO: [ ] por comando   [ ] con A.B.A.B por puente  -> destellos contados: ____
ESCLAVO: [ ] con A.B.A.B por puente   [ ] NO se pudo entrar, motivo: ______________
```

- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > 🔴 **La doble confirmación desapareció, y no por decisión de diseño.** Antes entrar exigía dos
  > pulsaciones (`Botón 3` → `CONFIRMAR ENTRADA?` → `Botón 3`) contra una sola para salir: **lo
  > peligroso difícil, lo seguro fácil**. Esa asimetría vivía en la pantalla. Hoy el Maestro entra
  > con **un solo comando** y el Esclavo con **una sola secuencia**. **No es un fallo de esta
  > sesión, es una decisión pendiente**: si la protección de entrada debe reconstruirse por comando,
  > lo decide el responsable. Anótelo si le parece relevante para su dictamen.

**9.7 VERIFICACIÓN VISUAL DEL CICLO — al menos 2 ciclos completos** 👁️👁️ — ♻️ **SE REESCRIBE**
— ⛔ **NO EJECUTABLE (04/09): exige las DOS puntas en Degradado, y al Esclavo no hay cómo meterlo.
Es la comprobación por la que existe esta sección, y hoy no se puede montar — se deja escrito así,
no se sustituye por nada**
- *Acción:* con las dos puntas en Degradado, **observar dos ciclos completos (4 minutos)** con un
  observador en cada punta.
- *Marcar cada punto con lo observado en las LUCES:*

```text
[ ] Maestro VERDE  <->  Esclavo ROJO           (verificado a las ______)
[ ] Todo-rojo de ~30 s con AMBAS en rojo       (medido: ______ s)
[ ] Esclavo VERDE  <->  Maestro ROJO           (verificado a las ______)
[ ] Todo-rojo de ~30 s con AMBAS en rojo       (medido: ______ s)
[ ] EN NINGUN MOMENTO hubo verde en las dos puntas a la vez
```

- Duración medida del ciclo completo: ________ s *(esperado ~120 s)*
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > **Por qué las luces y no la telemetría.** Cada unidad informa de la fase que *ella* calcula. Si
  > las dos calculan mal —relojes desfasados, versiones distintas, una unidad reiniciada— **las dos
  > dirán que todo va bien** mientras las luces cuentan otra historia. Y ahora hay una razón más
  > dura todavía: **el `$STATUS` del Esclavo dice `MODO:SUBORDINADO` pase lo que pase** (§0.2), así
  > que desde el teléfono **no se puede ni saber que está en Degradado**. **Las luces son la
  > evidencia.**
- > **Verde simultáneo en las dos puntas es la única forma en que este equipo puede matar a alguien.**
  > Si lo observa, saque las dos puntas con `B·B·B` de inmediato y **RECHACE**.

**9.8 Corte de energía en UNA punta — riesgo residual nº 2** ⚠️ — ♻️ **SE REESCRIBE**
— ⛔ **NO EJECUTABLE (04/09): parte de *«con las dos puntas en Degradado»*, y al Esclavo no hay cómo
meterlo. El riesgo residual sigue documentado y sigue **sin reproducirse en hardware**
- *Acción:* con las dos puntas en Degradado, **cortar y restituir la alimentación de una sola unidad**.
- *Esperado hoy:* la unidad reiniciada **arranca en reposo, sin enlace, y cae a ÁMBAR**, mientras
  **la otra sigue dando verde por reloj**.
- Lo observado: ______________________________________
- Resultado: `[ ] SE REPRODUJO  [ ] NO SE REPRODUJO` — *(escenario documentado, no puntuado)*
- > **Es el riesgo residual nº 2, conocido y aceptado.** No es un defecto nuevo: el estado del modo
  > vive en RAM (pendiente **N-20**, módulo escrito y sin conectar). Un lado en ámbar contra un lado
  > en verde es **exactamente el escenario que este modo quiere evitar**.
  > **La mitigación es procedimental, no técnica:** verificación visual de ambas puntas, también al
  > salir. Esta prueba existe para que el funcional **lo vea con sus ojos antes de firmar**.
- > 🔴 **Y con la arquitectura de hoy es peor que antes, por una razón concreta:** si la unidad que
  > se reinicia es el **Esclavo**, ya no queda ninguna forma de devolverlo al Degradado salvo volver
  > al puente de `J16`. Antes bastaba con subir al gabinete y usar su pantalla.

**9.9 Salida asimétrica provocada** ⚠️ — ♻️ **SE REESCRIBE**
— ⛔ **NO EJECUTABLE (04/09): `A·A·A` por puente de `J16`**
- *Acción:* con las dos puntas en Degradado, sacar del modo **una sola** unidad con `A·A·A` por puente.
- *Esperado:* se reproduce el mismo escenario que 9.8 — una punta en ámbar, la otra en verde.
- Resultado: `[ ] SE REPRODUJO  [ ] NO SE REPRODUJO` — *(escenario documentado, no puntuado)*
- > *Para qué sirve provocarlo:* demuestra por qué **la verificación visual de ambas puntas es
  > obligatoria también AL SALIR**, no sólo al entrar.

**9.10 Salida a Automático desde el piso** — ♻️ **SE REESCRIBE**
— ⛔ **NO EJECUTABLE (04/09): `A·A·A` por puente de `J16`**
- *Acción:* con el radio **restablecido**, dar `A·A·A` por puente en **ambas** unidades.
- *Esperado:* 2 destellos rojos y, a los ~15 s, **las luces vuelven a ciclar**.
- > *Ojo a la asimetría, que es de diseño y no un fallo:* en el **Maestro**, `A·A·A` significa
  > *«ponte en Automático»*; en el **Esclavo** significa *«vuelve a obedecer al Maestro»*
  > (`Esclavo/src/mando.cpp:109-127`, **MEDIDO**). No son la misma orden, y por eso hay que darla en
  > las dos.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**9.11 Salida a Automático con el radio TODAVÍA muerto** — ♻️ **SE REESCRIBE**
— ⛔ **NO EJECUTABLE (04/09): `A·A·A` por puente de `J16`**
- *Acción:* con el radio aún desconectado, dar `A·A·A` en ambas unidades.
- *Esperado:* 2 destellos, intenta Automático, **y a los 25 s cae solo a 🟡 Ámbar** en ambas.
- Segundos hasta el ámbar: ________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > *Por qué esta prueba tranquiliza:* **el peor caso de intentar Automático es volver al ámbar**,
  > que es justo donde se quería estar.

**9.12 Límite duro de 48 h** ⏱️ — ⏸️ **SE APLAZA**
- *Qué mediría:* que a las **44 h** apareciera el aviso `LIMITE 48h` y que a las **48 h** el modo
  cayera solo a ámbar intermitente, sin que nadie intervenga.
- **Falta:** el aviso de las 44 h **se mostraba en pantalla** y hoy no hay ningún campo de telemetría
  que lo exponga; y la caída de las 48 h **no se cronometra en una sesión de banco**.
- **No se firma.** Queda como **prueba larga pendiente**, y su mitad observable —la caída a ámbar—
  se puede medir en una prueba de duración dedicada, no aquí.
- > **Por qué el límite existe, con el número concreto:** el colchón que impide el verde simultáneo
  > es el todo-rojo de 30 s, y se lo come la deriva de dos cristales sin calibrar a la intemperie
  > (**±30 a 50 ppm**, ~8,6 s de separación por día). El margen teórico son ~3,5 días; las 48 h dejan
  > **factor de seguridad 2**. **Y es un tope, no un aviso:** *el estado seguro no puede depender de
  > que alguien se acuerde.* Si hace falta más autonomía, la respuesta no es alargar el plazo —una
  > semana obligaría a un todo-rojo de ~90 s— sino **ir a arreglar el radio**.

**9.13 Salir y volver a entrar NO reinicia la cuenta de 48 h** — ⏸️ **SE APLAZA**
- *Qué mediría:* que la cuenta siga donde iba, y que **sólo una sincronización nueva por radio** la
  reinicie.
- **Falta:** no hay hoy **por dónde leer la cuenta**. Es el mismo hueco de 7.10. Sin exponerla en el
  `$STATUS`, la prueba no se puede observar en menos de 48 h.
- **No se firma.**

**9.14 Comportamiento en el cambio de medianoche** — ♻️ **SE REESCRIBE**
- *Acción:* dejar el Modo Degradado corriendo **a través de las 00:00**, con observadores en ambas
  puntas.
- *Esperado:* al cruzar la medianoche **ambas puntas quedan en todo-rojo**. **No debe saltarse el
  despeje ni darse verde sin todo-rojo previo.**
- Lo observado entre 23:58 y 00:02: ______________________________________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > *Por qué se prueba específicamente:* a las 00:00:00 el contador del día vuelve a cero, y si la
  > duración del ciclo no divide exactamente al día —casi nunca lo hace— **la posición del ciclo
  > salta**. Las dos unidades saltan igual y a la vez, así que no se desincronizan; el problema es
  > otro y es peor: **ese salto podría caer en mitad de un verde y saltarse el despeje**.

**9.15 El Esclavo no acepta ninguna orden de modo** ➕ **NUEVA** — ♻️ *(ejecutable hoy)*
- *Por qué existe:* documenta con una medida lo que hoy es el hueco más grande del sistema. **Se
  ejecuta y se firma**, porque el resultado tiene que quedar en el acta.
- *Acción:* mandar al **Esclavo** `CMD:PIN:1234:SET_MODO:DEGRADADO` y después
  `CMD:PIN:1234:SET_MODO:AUTO`.
- *Esperado:* las dos contestan `$ERR,CMD:DESCONOCIDO,DESC:COMANDO_NO_SOPORTADO_EN_ESCLAVO`.
- Respuestas literales: ______________________________________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > 🔴 **Lo que este `$ERR` significa, dicho entero.** El Esclavo **no tiene ni un solo `SET_MODO:*`**
  > (**MEDIDO**, `Esclavo/src/bluetooth.cpp`). Sumado a que no hay pantalla y a que
  > `botonAceptar()` es `false`, hoy **las únicas vías que entran o sacan al Esclavo del Modo
  > Degradado son el mando —sin receptor comprado— y la vuelta automática de la radio.** La vía de
  > la pantalla (`Esclavo/src/menu.cpp:215`) **existe en el código y está muerta**, porque el
  > `aceptar` que la dispara no puede ser cierto nunca.
  >
  > **En claro: si el Esclavo entra en Degradado y no hay puente ni receptor, la única forma de
  > sacarlo es que vuelva la radio.** Es un dato para el dictamen, no una opinión.

**9.16 El ámbar de emergencia de la app en un Esclavo en Degradado (N-106)** ➕ **NUEVA** — ~~♻️ *(ejecutable hoy, por puente)*~~
— ⛔ **NO EJECUTABLE (04/09).** Su paso 1 es *«poner el Esclavo en Degradado con `A·B·A·B` por
puente»*, ~~y el puente está medido a `0,6 V`~~ **y no hay tarjeta sana con la que darlo (N-116);
el puente en sí funciona desde N-118, cerrando contra los 3,3 V del pin de al lado**. **El defecto
sigue abierto y sigue sin ejercerse nunca** — ni en banco ni en arnés. *La prueba se conserva entera: es correcta, y el día que haya
vía para meter al Esclavo en Degradado se ejecuta tal cual está escrita.*
- *Por qué existe:* es un **defecto abierto y conocido**, y esta sesión tiene que **ejercerlo**, no
  ignorarlo. Está razonado por lectura del fuente y **nadie lo ha ejecutado nunca** — ni en banco ni
  en arnés. Esta prueba es la primera vez que se mide.
- *Acción:*
  1. Poner el **Esclavo** en Modo Degradado con `A·B·A·B` por puente (requiere 9.6).
  2. Confirmar en las **luces** que el Degradado gobierna: el Esclavo cicla verde/rojo por reloj.
  3. Mandar al Esclavo `CMD:AMBAR_EMERGENCIA` (sin PIN).
  4. **Observar las luces durante 3 minutos completos, sin tocar nada.**
- *Anotar los tres datos, y los tres importan:*

```text
Respuesta literal del Esclavo: ______________________________________
La luz paso a AMBAR INTERMITENTE?         [ ] SI   [ ] NO
Y a los 3 minutos, seguia en AMBAR?       [ ] SI   [ ] NO, volvio a: ____________
```

- *Cómo se lee el resultado:*

| lo que ocurre | qué significa |
|---|---|
| Contesta `$ACK,...,RESULT:OK` y **la luz vuelve a ciclar** por reloj | 🔴 **El defecto está vivo.** El equipo dijo que sí y no lo hizo: el ámbar pedido desde el teléfono se cae solo justo en el modo donde más falta hace |
| Contesta `$ACK` y **se queda en ámbar** los 3 minutos | El arreglo está dentro y funciona. Anótelo con el `md5` del binario |
| Contesta `$ERR` con un motivo | El equipo **declara que no puede**, que es honesto. Copie el motivo literal |

- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > **Estado al redactar este documento (31/08, por lectura del árbol de trabajo):** el defecto
  > **sigue abierto**. `Esclavo/src/bluetooth.cpp` no nombra el Degradado en ninguna línea y ni
  > siquiera incluye su cabecera, mientras `Esclavo/src/mando.cpp:129-142` —la vía del `B·B·B`— sí
  > comprueba `degradado_gobiernaLuz()` y llama a `degradado_salir()`. **Las dos vías de ámbar de
  > emergencia no hacen lo mismo, y el firmware declara por escrito que sí.**
  >
  > **Se escribe la prueba, no el resultado.** El arreglo está en curso y puede estar dentro del
  > binario que se cargue; por eso la tabla de arriba admite las tres salidas. **Lo que no vale es
  > firmar esta casilla sin haber mirado las luces tres minutos**: el modo que las gobierna repinta
  > la luz en cada vuelta, así que un ámbar que aparece **no demuestra** que el ámbar se quede.

---

## 📑 SECCIÓN 10 — INTERFAZ PROPIA DEL ESCLAVO

> 🚫 **LA SECCIÓN ENTERA SE RETIRA — sus 5 pruebas (10.1 a 10.5).**
>
> Certificaba la pantalla y el menú propios del Esclavo: sus dos opciones, la pantalla `ESTADO`, el
> aviso `MODO DEGRADADO ACTIVO` en el pie y la doble confirmación de entrada. **Los cinco puntos
> viven en una pantalla que no conduce sus pines y en un menú que no se puede abrir.**
>
> **Ninguna lleva casilla. No se firman.**
>
> 🔴 **Dos huecos declarados que salen de aquí y no se cierran retirando la sección:**
>
> 1. **Ya no hay forma de saber, desde fuera, si el Esclavo está en Modo Degradado.** El aviso del
>    pie del menú lo decía sin entrar a ninguna pantalla, y era el dato que **cambia el significado
>    de todo lo demás**: un técnico que revise sin saber que el cruce va por reloj **puede creer que
>    el radio funciona** y dar por buena una avería que sigue ahí. El `$STATUS` del Esclavo no lo
>    sustituye: `MODO:SUBORDINADO` es un literal fijo (§0.2). **Exponer el modo real del Esclavo en
>    su `$STATUS` es trabajo de firmware, y hoy falta.**
> 2. **La entrada al Degradado del Esclavo perdió su doble confirmación** al perder la pantalla. Ver
>    la nota de 9.6.

---

## 📑 SECCIÓN 11 — CÁMARAS DE DEMANDA

> **Qué cambió:** no son cuatro cámaras ni hay pin de umbral. Son **dos cámaras de demanda, una por
> poste**. La única entrada de cámara que el firmware del Maestro lee hoy es `CAM_DEMANDA_PIN = PB0`,
> que sale por la bornera **`J14`** (`Maestro/include/pines.h:46`, **MEDIDO**).
>
> 🔴 **`PB8` NO es una entrada de cámara** y no lo ha sido nunca en el firmware. Alimenta un LED
> testigo de la placa. Hay documentación antigua que dice lo contrario (Guía, al final: el
> desplegable **«Consulta · qué va en cada bornera»**).
>
> ~~🔴 **Las cámaras de `J16` p10/p12 no se cablean en esta sesión** hasta que el paso 20 de la Guía
> mida la resistencia de reposo de esos pines.~~
>
> ✅ **04/09 — medido, y la condición se cumple: se pueden cablear.** `J16` p10 y p12 dan
> **`9,93 kΩ` y `9,94 kΩ` contra masa** y **`0 V` en reposo**: la resistencia real existe en el cobre
> y ya no depende de que el netlist tenga razón. Con eso se cableó la cámara del p10 a p11 (3,3 V) en
> normalmente-abierto, y **en reposo el equipo no pidió paso solo, ni con el cable puesto ni sin él**
> (Guía, paso 21).
>
> **La preocupación de la que salía esa prohibición no se borra, porque es la que se verificó:** sin
> resistencia real, un cable flojo no queda en reposo — **queda flotando, y el ruido del armario mete
> peticiones de paso que nadie ha hecho**. Aquí no flota. En cualquier otra placa, se vuelve a medir.
>
> ⚠️ **Lo que sigue sin verificarse es la otra mitad: que una demanda por `J16` conceda el paso.**
> El banco no llegó —hace falta el Modo Automático, y eso hace falta la app—. **Cableado sin falsa
> activación no es lo mismo que cámara que funciona.**

**11.1 Demanda en el Maestro (`J14`)** — ♻️ **SE REESCRIBE**
- *Acción:* provocar una detección con la cámara —o con un pulsador suelto entre los dos bornes de
  `J14`, que hace exactamente el mismo papel— estando ese sentido en Rojo.
- *Esperado:* el Maestro registra la demanda, **aplica el todo-rojo de despeje** y conmuta a
  🟢 Verde Maestro. **En reposo no pide paso solo.**
- *El montaje y la comprobación de que el contacto conmuta de verdad están en los **pasos 17, 18 y 19
  de la Guía**. No se repiten aquí: traiga de allí los números.*
- > 🔬 **Y de la sesión del 3–4/09 ya vienen dos de esos números, así que la mitad de esta prueba
  > está hecha:** el borne «3,3 V» de `J14` mide `3,3 V` y el borne «Puerta» `0 V`; **al puentearlos
  > el borne sube y al soltar vuelve** (pasos 17 y 18). **La conmutación está verificada.**
  > **Lo que quedó pendiente es justo lo que esta prueba mide de más: la reacción del semáforo**
  > (paso 19), porque el equipo nunca salió del estado de espera de selección de modo. **Se repite
  > entera cuando haya enlace.**
- Segundos hasta el verde: ________  En reposo, ¿pidió paso solo? ____________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**11.2 Demanda en el Esclavo, y el Maestro decide** — ♻️ **SE REESCRIBE**
- *Acción:* provocar la detección en el `J14` del **Esclavo**. Alternativa sin cámara: mandarle
  `CMD:PIN:1234:SOLICITAR_PASO`.
- *Esperado:* `$ACK,CMD:SOLICITAR_PASO,RESULT:PEDIDO_AL_MAESTRO`. El Esclavo **transmite la petición
  al Maestro y no enciende nada por su cuenta**; el Maestro aplica el todo-rojo y concede el verde.
- Vía usada: ____________  Segundos hasta el verde: ________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > **La asimetría es la regla, no un defecto: el Esclavo pide y el Maestro decide** (SFTY-27). Si el
  > Esclavo encendiera verde por su cuenta al detectar un vehículo, estaría abriendo un carril sin
  > que nadie compruebe el otro.

**11.3 Inmunidad a peatones, sombras y lluvia (filtro AcuSense)** — ⏸️ **SE APLAZA**
- *Qué mediría:* que el filtro *«sólo vehículo»* ignore a una persona que camina, salta o agita los
  brazos frente al lente.
- **Falta: la cámara real.** Es una propiedad **del procesador de la cámara**, no del semáforo: con
  un pulsador suelto no se puede ejercer, y sustituirla por el pulsador sería firmar una casilla que
  no midió nada.
- **No se firma.**

**11.4 Independencia entre las entradas de cámara y los pulsadores** — 🚫 **SE RETIRA**
- *Por qué:* medía que las activaciones en `PB0`/`PB8` no interfirieran con los pulsadores `PB9`,
  `PB13`, `PB14`, `PB15`. **Hoy no hay dos sujetos que separar:** `PB8` no es entrada de cámara,
  `PB14` y `PB15` **son las cámaras**, y `PB9`/`PB13` ya no son pulsadores de menú sino los canales
  `A` y `B` del mando.
- **No se firma.**
- > ~~🔴 **Lo que sí hay que medir en su lugar ya está escrito, y es el paso 20 de la Guía**: la
  > polaridad de reposo de `J16` p5, p8, p10 y p12, hoy en **contradicción medida** entre el netlist
  > (pull-**down** de 10 kΩ, activo en ALTO) y el fuente (`INPUT_PULLUP`, activo en BAJO). Cablear al
  > revés da **demanda permanente** o **demanda que nunca llega**: las dos son de calle.~~
- > 🔧 **04/09 — CADUCADO, y se tacha porque mandaba a repetir una medida que este mismo documento
  > ya publica 1.100 líneas más arriba.** El paso 20 **se ejecutó** en el banco del 3–4/09 y su tabla
  > completa está en **§0.1**: los cuatro pines llevan ~10 kΩ a masa (`9,92`–`9,94 kΩ`). **La
  > contradicción se resolvió a favor del netlist y ya no existe:** desde N-118 el fuente lee los
  > **cuatro** pines en `INPUT` pelado y **activo en ALTO**, así que netlist y firmware dicen lo
  > mismo. **No hay nada que volver a medir aquí.** Lo que queda por medir —y está anotado en §0.3—
  > es otra cosa: la tensión de p5/p8 **con el puente a 3,3 V puesto y este firmware cargado**, que
  > nadie ha tomado y no se puede tomar sobre la Maestro mientras siga el corto de N-116.
  > *(Un documento que se contradice consigo mismo manda a gastar una sesión de banco en repetir lo
  > hecho. Por eso esto se tacha en vez de dejarse «por si acaso».)*

---

## 📑 SECCIÓN 12 — TELEMETRÍA Y ÓRDENES POR EL PUERTO SERIE

**12.1 Emparejamiento y enlace Bluetooth** — ⏸️ **SE APLAZA** *(motivo corregido el 04/09)*
- *Qué mediría:* que el móvil empareje con el módulo a 10 m, en menos de 5 s.
- ~~**Falta: la placa del ESP32** (no diseñada, no fabricada, no medida), **su fuente** (DC-DC 12 V→5 V
  ≥1 A, sin pedir) y **el firmware del módulo**, que se entrega aparte.~~
- 🔴 **Lo que falta hoy es otra cosa, y es peor porque no se compra: el módulo NO SE ANUNCIA.** La
  placa existe y está armada, la fuente está montada, el firmware `ESP32_Expansion` **compiló y se
  cargó sin errores**, y el chip está descartado como causa —es un `ESP32-WROOM-32` clásico, con el
  `BR/EDR` que el `SPP` de la app necesita; un `S3`/`C3`/`S2` **jamás conectaría**—. Aun así, **el
  dispositivo no apareció de forma fiable en el teléfono** en toda la sesión (Guía, pasos 10 y 25).
  **La causa está abierta**: el informe apunta a mirar si el *advertising* `SPP` llega a arrancar
  (log por el monitor serie del USB), el estado de la antena, e interferencia.
- **No se firma.** *(El montaje, ya hecho, está en los pasos 22 a 25 de la Guía.)*
- > ⚠️ **Y hay un dato de proceso que salió de allí y evita una hora perdida:** el ESP32 empareja por
  > **«Just Works», sin PIN del sistema operativo** — confirmado en el firmware. **El `1234` de la
  > Guía es el PIN de COMANDO dentro de la app** (`CMD:PIN:1234:...`), no el de emparejamiento
  > Bluetooth. Son dos cosas distintas y se han confundido ya una vez.
- > ⚠️ **Y un requisito que va antes de comprar:** dos cruces vecinos ponen cuatro módulos al alcance
  > del mismo teléfono. **El nombre del módulo es lo único que impide mandar una orden de emergencia
  > al poste equivocado** (Guía, paso 25).

**12.2 Telemetría periódica `$STATUS`** — ♻️ **SE REESCRIBE**
- *Acción:* con el terminal a 9600 en cada punta, observar el flujo un minuto.
- *Esperado:* ~~**una trama por segundo**~~ **una trama cada 2 s** en cada punta, con su checksum, en
  el formato de §0.2 *(cadencia bajada a 2000 ms el 04/09, decisión del responsable — medido en
  `Maestro/src/bluetooth.cpp:851` y `Esclavo/src/bluetooth.cpp:768`)*.
  Comprobar que aparece `MODO:` y que **cambia** al mandar un `SET_MODO`.
- *Y lo que hay que confirmar que el equipo NO INVENTA* —copie los valores:

```text
BAT del Maestro tras 5 min: ______   BAT del Esclavo: ______   (los dos deben salir "--")
T / RF / RTT del Esclavo: ____ / ____ / ____              (los tres deben salir "--")
Campo ESC: del Maestro: ______                            (ROJO / VERDE / AMBAR / ?)
```

- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

> 🛑 **ESTA PRUEBA EXIGÍA EL DEFECTO, y por eso se invierte — 05/09.** Pedía *«los dos deben salir
> **`12.6` fijos**»* y *«deben salir **`98%` y `85ms` fijos**»*, que es **exactamente lo que N-108
> retiró el 04/09 por inventarse una medida**. Tal como estaba escrita, **un firmware arreglado
> salía `NO CUMPLE`**.
>
> **MEDIDO** —`Maestro/src/bluetooth.cpp:970` y `Esclavo/src/bluetooth.cpp:832`—: el `snprintf` de
> las dos puntas lleva **`BAT:--`** literal, y el del Esclavo además **`T:--,RF:--,RTT:--`**. La
> causa está medida y no es un hueco: **`grep -rn analogRead` sobre `Maestro/{src,include}` y
> `Esclavo/{src,include}` devuelve DOS coincidencias y las dos son COMENTARIOS** —cero llamadas—.
> Sin divisor de tensión ni entrada analógica **no hay batería que leer**.
>
> **Lo que se comprueba ahora es lo contrario de lo que se pedía: que el equipo se NIEGUE a publicar
> un número que no ha medido. Un `--` es un `CUMPLE`; un `12.6` sería el hallazgo.**
>
> **Y se conserva entera la frase que sigue valiendo**, que era la mitad buena de la prueba vieja:
> *un campo que siempre da el mismo número no es un dato, es un adorno con formato de medida.* Eso
> es justo lo que el `--` corrige — no se reporta como avería y tampoco se usa para decidir nada.
>
> 🆕 **`ESC:` sólo va en la trama del MAESTRO y va el último** (N-149). Que **falte** en el `$STATUS`
> del Esclavo **no es un fallo**.

**12.3 Caja Negra de Alarmas `$ALARM`** — ♻️ **SE REESCRIBE** *(la cifra estaba mal)*
- *Qué cambia:* la revisión anterior mandaba cortar la antena **12 s** y esperar una alarma cuyo
  propio texto de ejemplo decía `SILENCIO_25000ms`. **Con 12 s no salta nada**: el umbral son 25 s.
- *Acción:* desconectar la antena del Esclavo y **mantenerla desconectada más de 25 segundos**.
- *Esperado:* ambos caen a Ámbar Intermitente (SFTY-6) y llega una trama del tipo
  `$ALARM,NODE:...,EVENTO:FALLO_RF,CAUSA:SILENCIO_25000ms,ACCION:CAMBIO_A_AMBAR,HORA:...`
- Trama literal recibida: ______________________________________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**12.4 Modo Manual y Rojo Total desde el teléfono** — ♻️ **SE REESCRIBE** *(literales corregidos)*
- *Acción:* mandar `CMD:PIN:1234:SET_MODO:MANUAL` y después `CMD:FORZAR_ROJO` (**este segundo sin
  PIN**, que es como lo acepta el firmware).
- *Esperado:* conmuta a Modo Manual, y al recibir el forzado aplica **🔴 ROJO TOTAL INMEDIATO** en
  ambos extremos.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > **En el Esclavo, `CMD:FORZAR_ROJO` tiene que ser RECHAZADO** con
  > `$ERR,CMD:FORZAR_ROJO,DESC:RENOMBRADO_USE_AMBAR_EMERGENCIA`. Es deliberado: el nombre prometía
  > rojo y hacía ámbar con la pluma arriba, que es casi lo contrario. **Provóquelo y copie la
  > respuesta.**

**12.4-bis 🆕 El ámbar después de un ROJO TOTAL — el botón que se quedaba muerto** (`N-146`)
- *Por qué existe:* en la cinta del 04/09 hay **seis** órdenes de ámbar seguidas, las **seis
  contestadas `RESULT:OK`**, y el cruce **en rojo durante 47 tramas**. El equipo obedecía y no
  encendía nada. **Esta prueba es la única que lo caza, y el orden de los tres pasos ES la prueba.**
- *Acción:* **exactamente en este orden**, sobre el **Maestro**:
  1. `CMD:PIN:1234:SET_MODO:AMBAR` → el cruce se pone en **ámbar intermitente**.
  2. `CMD:FORZAR_ROJO` (sin PIN) → el cruce se pone en **ROJO TOTAL**. *(El modo sigue siendo ÁMBAR:
     `FORZAR_ROJO` cambia la LUZ, no el MODO. Eso es deliberado.)*
  3. `CMD:PIN:1234:SET_MODO:AMBAR` **otra vez**.
- *Esperado en el paso 3:* el cruce **vuelve a ámbar intermitente**, y la respuesta es
  **`$ACK,CMD:SET_MODO:AMBAR,RESULT:REARMADO`** — 🔵 **`REARMADO`, no `OK`. Las dos son un ÉXITO**;
  se dicen distinto porque son dos cosas distintas y el diario de órdenes tiene que poder
  separarlas.
- 🛑 **Si contesta `OK` y el cruce SIGUE EN ROJO, el defecto NO está arreglado**, y es grave: es una
  **salida de emergencia** que dice que sí y no hace nada.
- Respuesta literal del paso 3: ______________________________________
- ¿Volvió el ámbar? `[ ] SÍ  [ ] NO`
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**12.4-ter 🆕 El ámbar del Poste 2 llega al Poste 1** (`N-142`) — **necesita las DOS puntas vivas y
el enlace de radio en pie**
- *Por qué existe, y es lo más grave que se encontró:* si un operario ponía ámbar de emergencia desde
  la app **en el Esclavo**, el Maestro **no se enteraba** — y el Esclavo **seguía contestando al
  latido**, así que el enlace le parecía perfecto—. Con el Maestro en VERDE, **seguía dándolo hasta
  3 minutos** con el otro lado en ámbar: **los dos sentidos podían entrar al carril**.
- *Acción:* con el cruce **ciclando en Automático** y esperando a que **el MAESTRO esté en VERDE**,
  mandar al **Esclavo** `CMD:AMBAR_EMERGENCIA` (sin PIN).
- *Esperado:* el Esclavo pasa a ámbar intermitente **y el MAESTRO también, en segundos** — no en
  minutos y no esperando a que se agote nada. El Maestro **deja de ciclar**.
- 🛑 **Si el Maestro sigue en verde, PARE la prueba y anótelo: es la ventana de verde simultáneo.**
- Tiempo desde la orden hasta que el **Maestro** deja el verde: ________ s
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > ⚠️ **Y compruebe también lo que NO debe pasar, que es la otra mitad (`SFTY-21`):** mientras el
  > ámbar del Esclavo esté puesto, **el Maestro no puede quitárselo**. Si en algún momento el Esclavo
  > **se sale solo del ámbar** a los pocos segundos, eso es el veto caído y **es un `NO CUMPLE`**,
  > aunque el cruce parezca funcionar. Se sale con `CANCELAR_AMBAR` **con PIN, y desde el Esclavo**.

**12.4-quater 🆕 El campo del estado del Esclavo en la telemetría del Maestro** (`N-149`)
- *Acción:* conectado al **Maestro**, mirar una trama `$STATUS` y localizar el campo **`ESC:`** (va
  el último). Después **desconectar la antena del Esclavo** y esperar a que el enlace caiga.
- *Esperado:* con enlace, `ESC:` dice **`ROJO`**, **`VERDE`** o **`AMBAR`**; **sin enlace dice `?`**.
- 🔵 **El `?` significa «el enlace está caído y esta punta no sabe de qué color está la otra». NO
  significa «sin medida», y NO es un hueco: es la respuesta correcta.**
- ⚠️ **El ESCLAVO no emite este campo, y es a propósito** — no tiene de dónde sacarlo. **Que falte
  en la telemetría del Esclavo NO es un fallo.**
- Trama literal con enlace: ______________________________________
- Trama literal sin enlace: ______________________________________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**12.5 Selector de cruces en el corredor (multicruce con un teléfono)** — ⏸️ **SE APLAZA**
- **Falta:** dos módulos ESP32 montados con nombres distinguibles, la app, y dos cruces. Depende
  entera de 12.1.
- **No se firma.**

**12.6 Sincronización Puente Móvil (Courier RTC)** — ⏸️ **SE APLAZA**
- *Qué mediría:* capturar la hora en el Maestro, desplazarse hasta el Esclavo e inyectarla
  compensando el tiempo de viaje.
- ~~**Falta: el reloj `DS3231`**, que va montado **sobre la placa del ESP32** — la que no existe — con
  su propia pila.~~ ~~**04/09: el `DS3231` está montado y cableado** (I²C por `GPIO21`/`GPIO22`, paso
  22), con la masa común contra la STM32 medida en `0 V` (paso 23).~~
  > 🛑 **ESA FRASE ES FALSA Y SE TACHA — 05/09. EL `DS3231` NO ESTÁ COMPRADO NI CONECTADO, y la
  > confusión es exactamente la que `CLAUDE.md` §2.ter describe: se citó un PROTOCOLO como si fuera
  > un RESULTADO.** El «paso 22» de la Guía de banco se titula *«la placa del módulo: cómo tiene que
  > ser»* — **es un dibujo de cómo debe quedar cableado, no un acta de que se cableó**. Lo que sí es
  > un resultado es el paso 23: la **masa común** medida en `0 V`, y esa medida **no necesita
  > reloj**.
  >
  > **La Guía de banco de esta noche lo dice con todas las letras y hay que respetarlo al firmar:**
  > *«el `DS3231` no está comprado … márcalo "No se pudo probar" y escribe "no hay DS3231". NO lo
  > marques "NO CUMPLE"»*. **Un paso que no se pudo ejercer no es un suspenso — y tampoco un
  > aprobado.** Línea `A6` de la lista de compras.
  >
  > 🔵 **Y lo que sí cambió el 05/09 (`N-145`), que NO desbloquea esta prueba:** el firmware del
  > módulo **ya lee su `DS3231` y ya rellena el hueco de hora** de las tramas del equipo. **Sin
  > módulo en el bus, la hora seguirá saliendo `--:--:--`, y eso es el firmware negándose a inventar,
  > no fallando.** Quien vea el hueco sin `DS3231` delante **no puede concluir nada**.
- 🔴 **Lo que falta es la app, y no es un detalle de conveniencia:** al revisar el firmware del
  puente se confirmó que **la hora del `DS3231` sólo se lee y se ajusta con `SET_RTC` por Bluetooth.
  No existe otra vía para consultarla en banco** (paso 27, bloqueado). Mientras el módulo no se
  anuncie, **ese reloj no es verificable desde fuera por ningún camino** — ni con el USB-TTL de
  `J17`, que habla con la STM32, no con el ESP32.
- **No se firma.**
- > ⚠️ **Cuando exista, hay un aviso de montaje que no es opcional:** si el módulo del reloj trae una
  > **`CR2032` en vez de una `LIR2032`**, hay que desoldar el diodo o la resistencia de su circuito
  > de carga. **La `CR2032` no es recargable y ese circuito la calienta** (Guía, paso 22).

**12.7 El equipo dice que NO cuando no puede** ➕ **NUEVA** — ♻️ *(ejecutable hoy)*
- *Por qué existe:* **es la propiedad más importante de todo el protocolo y no tenía prueba propia.**
  Un equipo que contesta *«hecho»* sin hacerlo manda al técnico a casa creyendo que lo dejó
  arreglado. Se prueban los rechazos **a propósito**, uno por uno.
- *Acción:* mandar estas órdenes y **copiar la respuesta literal de cada una, sin resumirla**:

```text
En el MAESTRO:
   CMD:PIN:0000:SET_MODO:AUTO        -> tiene que dar AUTH_FAILED, y el modo NO puede cambiar
   CMD:PIN:1234:SET_MODO:ALCANCE     -> con un modo en marcha, EN_MARCHA_PARE_EL_MODO
   CMD:PIN:1234:SET_TIEMPOS:1,1,15   -> con el ciclo corriendo, EN_MARCHA_PARE_EL_MODO
   CMD:PIN:1234:SET_TIEMPOS:9,9,9    -> fuera de rango, RANGO
   CMD:PIN:1234:SET_RTC:31-08-2026   -> formato al reves, FORMATO_INVALIDO
   CMD:PIN:1234:DEMANDA              -> fuera del modo inteligente, SOLO_EN_MODO_INTELIGENTE
   CMD:PIN:1234:ESTO_NO_EXISTE       -> COMANDO_NO_SOPORTADO

En el ESCLAVO:
   CMD:PIN:1234:TEST_LEDS            -> NO_EN_SERVICIO_USE_EL_MAESTRO
   CMD:FORZAR_ROJO                   -> RENOMBRADO_USE_AMBAR_EMERGENCIA
```

- *Esperado:* **las nueve contestan `$ERR` con su motivo. Ninguna contesta `$ACK`.** Con el PIN mal,
  además, **el modo no cambia**: compruébelo **mirando las luces**, no sólo la respuesta.
- Con el PIN mal, ¿cambió el modo? ____________
- ¿Alguna contestó `$ACK`? ¿cuál? ______________________________________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > 🔴 **Un `$ACK` en cualquiera de las nueve es el fallo entero, aunque el equipo se comporte bien
  > después.** Y hay una que hay que mirar **con los ojos**, no en el terminal: con
  > `CMD:PIN:1234:TEST_LEDS` en el Maestro las lámparas se encienden pero **la talanquera NO se puede
  > mover**. Si la pluma sube durante esa prueba, anótelo — es importante *(Guía, paso 28)*.

---

## 📑 SECCIÓN 13 — BLINDAJE DEL MANDO ANTI-COLISIÓN (N-53)

> 🚫 **LA SECCIÓN ENTERA SE RETIRA — sus 2 pruebas.**

**13.1 Estrés con codillo en la pantalla `AJUSTAR HORA`** — 🚫 **SE RETIRA**
- *Por qué:* medía que 15 pulsaciones rápidas del Botón 1 subieran los minutos sin que el firmware
  las confundiera con la secuencia `A·A·A` del mando. **No hay pantalla de ajuste que abrir, ni
  Botón 3 con que llegar a ella.** La causa raíz que documentaba —los relés en paralelo con los
  pulsadores frontales— sigue siendo cierta, pero **ya no hay pulsador frontal con el que chocar**.
- **No se firma.**

**13.2 «Nuevas secuencias oficiales del mando»** — 🚫 **SE RETIRA**
- *Por qué, y esto es un error de la revisión anterior, no un cambio de arquitectura:* pedía probar
  `A·B·A` (Auto), `B·A·B` (Ámbar) y `B·A·B·A` (Manual). **Esas secuencias nunca se implementaron.**
  Las reales son, y siguen siendo, `A·A·A` (Automático), `B·B·B` (Ámbar) y `A·B·A·B` (Degradado)
  — `Maestro/src/mando.cpp:202-235`, **MEDIDO**.
- **No se firma.** Las tres secuencias que sí existen se certifican en la **Sección 8**.
- > 🔴 **La redefinición de secuencias sigue siendo una decisión de especificación abierta**, no
  > código pendiente: cambiarla toca el Manual de Usuario, este protocolo y el adiestramiento del
  > operario. **Es del responsable.** Se anota aquí para que no se pierda al retirar la prueba.

---

## 📊 RESUMEN DE RESULTADOS

> **Sólo se cuentan las pruebas que llevan casilla.** Las aplazadas y las retiradas **no suman ni al
> numerador ni al denominador**: contarlas como denominador convertiría un hueco en un suspenso, y
> contarlas como aprobadas sería exactamente lo que este documento existe para impedir.

> ## ⚠️ 04/09 — EL DENOMINADOR DE ABAJO ES EL DEL 31/08 Y HAY QUE RECORTARLO ANTES DE FIRMAR
>
> Las cifras del recuadro siguiente se escribieron cuando el estado de `J16` era una incógnita. **El
> banco del 3–4/09 la despejó y varias pruebas dejaron de ser ejecutables**, no por el firmware sino
> por la placa:
>
> - **La §8 entera —5 pruebas— no se ejecuta.** ~~`J16` p5/p8 en `0,6 V` (paso 20).~~ 🔧 **Motivo
>   corregido el 04/09 (N-118): el mando está arreglado en el fuente —`INPUT` pelado, activo en ALTO
>   en las dos puntas— y PENDIENTE DE EJERCER EN TARJETA.** El gesto es p5 contra p4 y p8 contra p7,
>   no contra masa (§0.3), y **nadie lo ha medido nunca con este firmware cargado**.
> - **De las 14 de la §9, seis dependen directamente del puente** —`9.5`, la punta del Esclavo de
>   `9.6`, `9.9`, `9.10`, `9.11` y `9.16`— **y otras dos, `9.7` y `9.8`, no se pueden montar sin
>   meter al Esclavo en Degradado**. `9.7` es la verificación visual de dos ciclos completos: **la
>   comprobación por la que existe la sección**.
> - **Y hay una condición previa que no es del papel: la tarjeta MAESTRO está pendiente de inspección
>   técnica** tras el sobrecalentamiento del paso 29. Sin esa inspección **no se energiza**, y sin
>   Maestro no se ejecuta nada.
>
> **Este documento NO publica un total nuevo, y es deliberado.** Escribirlo aquí sería inventar una
> cifra que nadie ha medido, sobre un alcance que todavía depende de ~~dos decisiones abiertas —qué
> pasa con la red de ~10 kΩ del mando, y qué dice la inspección de la placa—~~ **una decisión
> abierta: qué dice la inspección de la tarjeta Maestro.** *(La red de ~10 kΩ dejó de ser una
> decisión el 04/09: es correcta, es la resistencia de reposo de una entrada activa en ALTO, y el
> firmware ya la lee así — N-118.)* **El denominador lo
> recorta quien ejecute la sesión, tachando línea por línea lo no ejecutable con su motivo**, y el
> alcance certificado del acta se escribe con ese número, no con el `50` de abajo.
>
> > **Y la marca que se usa para tacharlas importa:** una prueba que no se pudo ejecutar se anota
> > **«no ejecutable» con el motivo**, nunca `NO CUMPLE`. `NO CUMPLE` acusa al firmware de fallar
> > algo que no ha llegado a hacer, y `CUMPLE` sería peor todavía. **Son tres estados distintos y no
> > se mezclan.**

```text
PRUEBAS EJECUTABLES EN ESTA RONDA
Seccion  1 — Reposo e independencia de radio ..........  ___ / 3
Seccion  2 — Perdida de comunicacion y Self-Healing ...  ___ / 5
Seccion  3 — Modo Automatico ..........................  ___ / 5
Seccion  4 — Modo Inteligente (demanda) ...............  ___ / 2
Seccion  5 — Modo Manual y medida de enlace ...........  ___ / 6
Seccion  7 — Reloj y sincronizacion ...................  ___ / 6
Seccion  8 — Mando de reles (por puente en J16) .......  --- / 5   <- NO EJECUTABLE (04/09)
                                                                     motivo VIGENTE: N-118 arreglo
                                                                     el firmware (INPUT pelado,
                                                                     activo en ALTO) y falta
                                                                     ejercerlo en tarjeta sana.
                                                                     El puente va p5-p4 y p8-p7,
                                                                     NO contra masa.  Ver 0.3
                                                                     (el motivo viejo "p5/p8 =
                                                                      0,6 V" esta CADUCADO)
Seccion  9 — Modo Degradado ...........................  ___ / 14   (12 + 9.15 + 9.16)
   de las 14, NO son ejecutables sin el puente de J16:  9.5 . 9.6(punta Esclavo) . 9.9 .
   9.10 . 9.11 . 9.16   -- y 9.7 y 9.8 no se pueden montar sin meter al Esclavo en
   Degradado (9.7 es la verificacion visual de dos ciclos, la clave de la seccion).
   Se marcan "no ejecutable" con su motivo, NUNCA "NO CUMPLE" ni "CUMPLE".
Seccion 11 — Camaras de demanda .......................  ___ / 2
Seccion 12 — Telemetria y ordenes .....................  ___ / 4    (3 + 12.7)
                                                         -----------------
                                 TOTAL DEL 31/08         ___ / 52

   De las cuales, ESCENARIOS documentados y NO puntuados:  9.8 y 9.9
   -> total puntuable del 31/08: ___ / 50

   ALCANCE REAL DE ESTA SESION  (se recorta aqui, no se hereda):
      Pruebas retiradas del alcance por no ejecutables ...... ______
      -> DENOMINADOR DE ESTA SESION ......................... ______
      Este numero se escribe el dia de la sesion, contando lo que de verdad
      se pudo ejecutar, y es el que va en el ACTA. El 50 de arriba es del 31/08
      y quedo caducado por la medida del paso 20.


NO EJECUTABLES EN ESTA RONDA  (no se firman, no se cuentan)
   APLAZADAS ... 13   Seccion 6 entera (6) . 8.9 . 9.12 . 9.13 . 11.3 . 12.1 . 12.5 . 12.6
                      (12 heredadas de la revision anterior + 8.9, que es nueva)
   RETIRADAS ... 21   1.2 . 4.1 . 4.3 . 5.6 . 5.7 . 7.2 . 7.3 . 7.4 . 7.10 . 7.11
                      8.1 . 8.6 . 8.8 . Seccion 10 entera (5) . 11.4 . 13.1 . 13.2


DATOS MEDIDOS  (no cuentan como CUMPLE/NO CUMPLE: son registro para el acta)

  md5 del binario cargado:  Maestro ______________  Esclavo ______________

  Enlace (5.2-bis), leido del $STATUS del MAESTRO:
     RF ______ %        RTT ______ ms

  Reloj (7.5 / 7.6 / 7.7):
     Deriva contra hora patron en 2 h ....... ______ s
     Diferencia tras corte de energia ....... ______ s (Maestro)  ______ s (Esclavo)
     Veredicto de REINICIAR_RELOJ ........... Maestro: ______________
                                              Esclavo: ______________

  Sincronizacion horaria (7.9):
     HORA: Maestro ______:______:______   HORA: Esclavo ______:______:______
     Desfase calculado ...................... ______ s   (tolerancia +-3 s)

  Modo Degradado (9.7):
     Duracion del ciclo completo ............ ______ s   (esperado ~120 s)
     Todo-rojo medido ....................... ______ s   (esperado ~30 s)

  Mando (§0.3):
     ~~J16 p5 contra masa ... ______   p8 contra masa ... ______~~   CADUCADO (N-118, 04/09):
     el gesto es contra los 3,3 V del pin de al lado, NO contra masa.

     LO QUE FALTA MEDIR, y nadie ha medido nunca -es EL dato de esta seccion-:
        V en p5 con el puente p5-p4 puesto y el firmware de N-118 cargado ... ______ V
        V en p8 con el puente p8-p7 puesto y el firmware de N-118 cargado ... ______ V
        md5 del binario cargado (tiene que ser el de N-118) ......... ________________
     -> El puente se pudo usar?  [ ] SI   [ ] NO, motivo: ______________
     -> NO se toma sobre la Maestro mientras siga el corto de N-116.

     MEDIDO YA el 3-4/09 sobre estas placas, para contrastar:
        p5  9,92 kOhm a masa | 11,28 kOhm a 3,3 V | 0,6 V en reposo  <- lo ponia el
        p8  9,92 kOhm a masa | 11,28 kOhm a 3,3 V | 0,6 V en reposo     INPUT_PULLUP viejo
        p10 9,93 kOhm a masa | 11,29 kOhm a 3,3 V | 0   V en reposo  -> correcto (camara)
        p12 9,94 kOhm a masa | 11,31 kOhm a 3,3 V | 0   V en reposo  -> correcto (camara)
     Con el firmware de N-118 los CUATRO deberian dar 0 V en reposo, porque los cuatro
     van en INPUT pelado. Si p5/p8 vuelven a dar 0,6 V, el binario cargado NO es el de
     N-118: se comprueba el md5 antes de reportar nada.
     Si las resistencias difieren de estas, NO son las mismas placas o algo cambio:
     eso es un hallazgo por si solo y se anota antes de seguir.
```

### Pruebas NO CUMPLE — detalle para el equipo de desarrollo

| Nº | Qué se observó | Segundo / momento exacto | Modo y vía usada |
|---|---|---|---|
|  |  |  |  |
|  |  |  |  |
|  |  |  |  |

### Lo que vio y nadie pensó en preguntarle

Un ruido, un olor, un parpadeo, una lámpara a media luz, una soldadura rehecha, un componente que no
coincide con el resto, un número que no cuadra. **Sin diagnosticar: sólo lo que vio.**

________________________________________________________________________________
________________________________________________________________________________

---

## ✍️ ACTA DE CERTIFICACIÓN FUNCIONAL

```text
Fecha de Auditoria: _____ / _____ / 2026       Hora inicio: ______  Hora fin: ______
Lugar / Tramo: __________________________________________________________________
md5 del firmware certificado: Maestro ______________  Esclavo ______________
Air Data Rate verificado: ________ kbps        Via de mando usada: ________________

ALCANCE CERTIFICADO:  las ______ pruebas puntuables EJECUTADAS en esta sesion, y NINGUNA
                      OTRA. El numero sale del recorte del resumen, no del 50 del 31/08.
                      Las no ejecutables van listadas con su motivo, no contadas.

DICTAMEN:
  [ ] APROBADO ..................... todas las pruebas del alcance en CUMPLE
  [ ] APROBADO CON OBSERVACIONES ... sin hallazgos de seguridad vial; detallar arriba
  [ ] RECHAZADO .................... uno o mas hallazgos de seguridad vial

DICTAMEN ESPECIFICO SOBRE EL MODO DEGRADADO (marcar solo si se ejecuto la Seccion 9):
  [ ] APTO para operacion en campo con el procedimiento de 8_Procedimiento_Modo_Degradado.md
  [ ] APTO CON RESTRICCION ... detallar: ______________________________________
  [ ] NO APTO ................ no debe operarse en via abierta al trafico
```

**Se deja constancia expresa de que NO forman parte del alcance de esta certificación** las ~~12~~
**13** pruebas aplazadas *(las 12 heredadas más `8.9`, que es nueva — el resumen ya decía 13 y esta
línea se había quedado con la cifra vieja)* y las 21 retiradas que se listan en el resumen, y en
particular:

- **el receptor de radio del mando de relés**, que nunca se compró: las secuencias se ejercerían con
  un puente ~~a masa~~ **a los 3,3 V del pin de al lado** en `J16`, lo que verificaría el firmware
  pero **no la condición real de uso** — y en esta ronda **ni siquiera eso se pudo hacer**, porque
  no hay tarjeta sana (N-116) y el gesto de N-118 **nunca se ha ejercido sobre cobre**;
- **el módulo ESP32, su placa, su fuente y el reloj `DS3231`**, que no existen: no se ha certificado
  ningún enlace Bluetooth, ninguna función de la app y ninguna sincronización por courier;
- **la topología con repetidor** y sus cuatro radios;
- **la persistencia del estado del Modo Degradado ante corte de energía** (N-20), reproducida y
  documentada en 9.8 como riesgo residual conocido;
- **la ausencia de doble confirmación** en la entrada al Modo Degradado, consecuencia de la retirada
  de la pantalla (9.6);
- **el hecho de que el Esclavo no acepte ninguna orden de modo** (9.15), con lo que su salida del
  Modo Degradado depende hoy del mando o de la vuelta de la radio.

**Y lo que el banco del 3–4/09 añade a esa constancia, que no estaba el 31/08:**

- **la §8 entera y siete pruebas de la §9**, no ejecutables ~~porque `J16` p5/p8 miden `0,6 V` en
  reposo: **el respaldo físico de mando está sordo en esta placa**~~ 🔧 **— motivo corregido el
  04/09 (N-118): el respaldo físico de mando estaba sordo POR EL FIRMWARE, y el firmware ya está
  arreglado en las dos puntas. Lo que falta es ejercerlo en una tarjeta sana, y no la hay** —, y con
  él la única vía que tiene hoy el Esclavo para entrar o salir del Modo Degradado sin radio;
- **la regresión N-42**, que **sigue sin determinar**: el banco no la confirmó ni la descartó porque
  el equipo nunca llegó a operar en Modo Automático. **No se certifica ningún ciclo vehicular**;
- **el verde simultáneo en las dos puntas**, que sigue **sin ejercerse sobre hardware** — sólo en
  arneses de PC;
- **el estado eléctrico de la tarjeta MAESTRO**, pendiente de inspección técnica tras el
  sobrecalentamiento del paso 29;
- **la fuente `12 V → 5 V` de la placa del módulo**, nunca medida con carga real: todo el banco se
  alimentó por USB;
- **el reloj `DS3231`**, montado pero **no verificable por ninguna vía** mientras no haya enlace
  Bluetooth, porque sólo se lee y se pone con `SET_RTC` desde la app.

```text
Ingeniero Funcional / Auditor de Transito
Nombre: _________________________________________________________________________
Cargo / Empresa: ________________________________________________________________
Matricula profesional: __________________________  Firma: _______________________


Ingeniero Responsable de Desarrollo
Nombre: _________________________________________________________________________
Matricula profesional: __________________________  Firma: _______________________
```

> **Nada de lo probado en esta ronda sube a un cruce abierto al tráfico.** En campo sigue la V8.4.
> Esta es una sesión de comprobación, y su producto son **medidas, no un visto bueno**. Si algo no se
> pudo medir, se anota como **no medido** — nunca como correcto. Un hueco declarado vale; un hueco
> callado, no.
>
> 🔬 **Y desde el 04/09 esto tiene un ejemplo, no una moraleja.** La sesión del 3 y 4 de septiembre
> devolvió 29 pasos con sus números, tres hallazgos abiertos y **una prueba abortada por seguridad**.
> Ninguno de los tres hallazgos se podía ver desde el repositorio, y el que más costó —el
> sobrecalentamiento del Maestro— **se encontró porque alguien tocó el chip**. Eso es lo que aporta
> una sesión de banco y lo que no aporta ningún acta verde: **la parte del equipo que no está
> escrita en ningún fichero.**
