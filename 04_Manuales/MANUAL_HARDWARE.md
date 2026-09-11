# ⚙️ Manual de Hardware y Topología Física (V7.6 Definitiva)

Este documento describe la arquitectura física de las placas impresas (PCBs), los componentes electrónicos, y la topología de red utilizada en el ecosistema de semáforos móviles.

> ## 🔴 REVISIÓN DEL 7 DE SEPTIEMBRE DE 2026 — léase antes que el cuerpo
>
> | | |
> |---|---|
> | 🔴 **`D-14` está DECIDIDA y NO TIENE CAMINO** | *«el controlador cierra un contacto y la cámara graba»*: **el firmware no mueve ninguna salida libre**. `J9`, `J11` y `J13` están fabricadas enteras y **declaradas y muertas** — cero `pinMode`, cero `digitalWrite`. §5.bis punto 5 |
> | 🔴 **`A·A·A` en `J16` p5 arranca el ciclo sin guarda** | Es la única de las tres secuencias del mando que **abre paso**, y la única sin validación. §3.2 |
> | ⛔ **6 de las 9 citas `fichero:linea` estaban caducadas**, y en los bloques de `grep` publicados fallaron **6 de 14 lineas** | Y una señalaba al **fichero equivocado**: la lectura de cámara se mudó a `camara_leerPin()` en `botones.cpp`. Se sustituyen por símbolos |
> | ⛔ **Un `grep` publicado como cero daba hits al re-correrlo** | El de `HC-05`/`JDY` iba **sin acotar** y muerde los binarios de `U8g2` en `.pio/`. **Un cero de `grep` sólo vale si se dice sobre qué se corrió.** §4 |
> | 🔴 **11/09 — `D-25`: CUATRO CÁMARAS, DOS POR POSTE** | Cámara 1 en `J16` p9/p10 (`CAM_C_PIN`), cámara 2 en `J16` p11/p12 (`CAM_D_PIN`); talanquera en `J15` (p1 12 V, p2 drenador de `Q10`, **no masa**) por relé a `OPEN` de la centralita. Deroga *«una por poste / p12 vacío»*. **Las dos cámaras hacen lo mismo, ninguna frena la pluma y la app no las distingue.** §3.2, §3.3, §3.bis |
> | 🟢 **11/09 (tarde) — `D-27`: `J14` LIBRE, sin cablear; cuatro cámaras compradas; relé de bobina 12 V** | El fin de carrera **no se instala en este despliegue**, y eso cierra el conflicto de `A-2` que este manual dejaba abierto. La talanquera va por un **relé de bobina de 12 V DC con contacto NA**, no por un módulo optoacoplado. §3.bis |
>
> **Manda [`DECISIONES.md`](../DECISIONES.md)**, y en cobre medido
> `05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md`. **Este manual nunca gana.**

**Revisión anterior:** 31 de agosto de 2026 — **el apartado 3 (cámaras) estaba MAL y se ha corregido.**
Llamaba a `PB8` *«Entrada Libre»* y mandaba enchufarle una cámara: `PB8` es el **`LED_TESTIGO`**, una
salida por `R16` 1 kΩ al LED `D5`. Y mandaba las cuatro cámaras contra `GND` cuando la entrada es
**activa en ALTO**. Es `N-105` de [`roadmap.md`](../roadmap.md). El texto viejo queda **tachado en su
sitio con el motivo, no borrado**.

---

## 1. Topología de Comunicaciones y Conexión de Radios RS485

Para superar los obstáculos del terreno (montañas, maquinaria, polvo), el sistema utiliza radios industriales **E90-DTU (RS485/232 $\leftrightarrow$ RF)**.

```text
+------------------------+        2 Hilos RS485       +------------------------+
| Tarjeta Semáforo STM32 |                            |     Radio E90-DTU      |
|  (MAX3485 Integrado)   |                            |   (Caja Metalica RF)   |
|                        |                            |                        |
|        Bornera A  ----|----------------------------|----> Pin 485_A         |
|        Bornera B  ----|----------------------------|----> Pin 485_B         |
|                        |                            |      Pin V+ (12-24V)   |
|        GND / V-   ----|----------------------------|----> Pin V-            |
+------------------------+                            +------------------------+
```

### 💡 Reglas Inquebrantables de Cableado:
1. **La PCB del Semáforo ya incluye el integrado MAX3485:** Convierte las señales TTL del STM32 (`PB10`, `PB11`, `PB12`) a bus diferencial RS485.
2. **Conexión de solo 2 hilos de datos hacia la radio E90-DTU:**
   - Bornera **`A`** de la Tarjeta Semáforo $\rightarrow$ Pin **`485_A`** de la radio E90-DTU.
   - Bornera **`B`** de la Tarjeta Semáforo $\rightarrow$ Pin **`485_B`** de la radio E90-DTU.
   - **`V+`** y **`V-`** para alimentación de 12V/24V de la radio.
3. **Autoconmutación TX/RX:** La radio E90-DTU realiza la conmutación de dirección RS485 automáticamente por hardware. NO se requiere cablear pines DE/RE hacia la radio.

---

## 2. Topologías de Red Soportadas (Transparente Automático)

El firmware V7.6 en las placas STM32 es 100% agnóstico a la topología de red instalada:

### Opción A: Modo Directo (2 Radios - Línea de Vista)
- **Maestro:** Conectado por bornera A/B a Radio 1 (Frecuencia 170.0 MHz).
- **Esclavo:** Conectado por bornera A/B a Radio 2 (Frecuencia 170.0 MHz).
- **Operación:** Transmisión directa transparente por aire. No requiere la tarjeta Repetidor ESP32.

### Opción B: Modo Repetidor (4 Radios - Esquinas Ciegas / Montaña)
- **Maestro:** Conectado a Radio 1 (Frecuencia 170.0 MHz).
- **Repetidor ESP32 (Físico Back-to-Back V7.6):**
  - Radio B1 en 170.0 MHz (Habla con el Maestro).
  - Radio B2 en 172.0 MHz (Habla con el Esclavo).
  - El ESP32 reenvía tramas con temporizador de ráfaga inter-byte de 15ms y 2ms de estabilización RS485 en `01_Firmware/Repetidor`.
- **Esclavo:** Conectado a Radio 4 en 172.0 MHz.

### Opción C: Modo Cable Físico Directo (Respaldo si el radio no va)
- **Maestro:** Bornera `RS485_OUT` (Pines A, B, GND).
- **Esclavo:** Bornera `RS485_OUT` (Pines A, B, GND).
- **Conexión:** Cable trenzado directo 3 hilos (A con A, B con B, GND con GND).
- **Operación:** Comunicación cableada transparente sin necesidad de radios.

---

## 3. Integración de las Cámaras IA de Demanda Vehicular

> ## 🛑 AVISO — `PB8` **NO** ES UNA «ENTRADA LIBRE». ES UNA **SALIDA A LED** (31/08/2026)
>
> Este apartado llamaba a `PB8` *«Entrada Libre»* y mandaba enchufarle ahí la Cámara 2 y la Cámara 4.
>
> **MEDIDO EN EL FUENTE** (`01_Firmware/Maestro/include/pines.h:63`, línea idéntica en el Esclavo):
>
> ```
> #define LED_TESTIGO        PB8  // -> R16 1K -> LED D5. NO es entrada de camara
> ```
>
> `PB8` sale por **`R16` de 1 kΩ al LED `D5`**. No es bornera, no es entrada optoacoplada y no hay
> dónde enchufar un contacto seco. El firmware lo deja **en alta impedancia a propósito** —símbolo
> `pinMode(LED_TESTIGO, INPUT)` en `Maestro/src/modo_inteligente.cpp`; ⛔ *(la cita `:50` estaba
> caducada; hoy `:137`)*—: con `INPUT_PULLUP` se le colarían ~40 µA y quedaría un testigo encendido
> a medias que nadie sabría explicar.
>
> **Y el `GND` de las cuatro líneas también estaba mal.** La entrada de cámara es **activa en
> ALTO** y la bornera saca el pin **junto a 3,3 V**: cableada contra masa, **la cámara no dispara
> nunca**. Es `N-105`; ver el detalle y las dos configuraciones de la salida de la cámara en
> [`MANUAL_USUARIO.md`](MANUAL_USUARIO.md) §6.3.
>
> ⛔ **Y la cita de esa afirmación —~~`modo_inteligente.cpp:25`, `:46`~~— no sólo estaba caducada:
> SEÑALABA AL FICHERO EQUIVOCADO (corregido el 07/09).** La lectura de toda cámara se centralizó en
> **`camara_leerPin()`, en `botones.cpp` de las dos puntas**, y el `pinMode` se fue con ella. Es la
> deriva que un número de línea no puede delatar —el fichero viejo **sigue existiendo**, así que la
> guarda de rutas no ve nada—:
>
> ```
> $ grep -rn "^bool camara_leerPin" 01_Firmware/Maestro/src/botones.cpp 01_Firmware/Esclavo/src/botones.cpp
> 01_Firmware/Maestro/src/botones.cpp:114:bool camara_leerPin(uint8_t pin) {
> 01_Firmware/Esclavo/src/botones.cpp:127:bool camara_leerPin(uint8_t pin) {
> ```
>
> Cuerpo: `if (digitalRead(pin) == HIGH) { delay(5); return (digitalRead(pin) == HIGH); }` —
> **activo en ALTO, con doble lectura antirrebote.** La conclusión no cambia; el sitio donde
> comprobarla, sí.
>
> Es la **tercera** vez que este proyecto publica un pin como libre sin cruzarlo contra `pines.h`
> (`N-59`, `N-67`, y ésta). **«Pin libre» no es una observación: es una medida contra `pines.h`.**

### 3.1 ~~El cableado que este apartado mandaba hacer~~ — ⛔ ANULADO, CONSERVADO COMO RASTRO

* ~~**En el Semáforo Maestro (Extremo 1):**~~
  * ~~**Cámara 1 (Demanda Sentido 1):** Contacto seco `1A`/`1B` conectado a **`PB0`** (Entrada Libre) y **`GND`**.~~
  * ~~**Cámara 2 (Umbral Tramo 1):** Contacto seco `1A`/`1B` conectado a **`PB8`** (Entrada Libre) y **`GND`**.~~
* ~~**En el Semáforo Esclavo (Extremo 2):**~~
  * ~~**Cámara 3 (Demanda Sentido 2):** Contacto seco `1A`/`1B` conectado a **`PB0`** (Entrada Libre) y **`GND`**.~~
  * ~~**Cámara 4 (Umbral Tramo 2):** Contacto seco `1A`/`1B` conectado a **`PB8`** (Entrada Libre) y **`GND`**.~~
* ~~**Independencia de Buses:** Los pulsos de detección entran por las entradas dedicadas `PB0` y `PB8` sin interferir con los botones del panel LCD (`PB9`, `PB13`, `PB14`, `PB15`)…~~
  ⛔ **Falso por partida doble:** `PB8` no es entrada, y `PB14`/`PB15` **dejan de ser botones** y pasan
  a ser justamente las entradas de cámara (`N-104`). Lo que **sí** sigue siendo cierto es que nada de
  esto toca el bus de sincronización `RS485_OUT`.

### 3.2 ✅ El reparto real de estos pines

> ⛔ **BARRIDO DE CITAS DEL 07/09: de las **9** citas `fichero:linea` de este manual, **6** estaban caducadas; y en los bloques de `grep` publicados, **6 de 14 lineas**.**
> Las movió `4b2841b`, que insertó las marcas `D-x` en los comentarios del firmware. **Se
> sustituyen por símbolos** — un símbolo sobrevive a que alguien inserte veinte líneas encima; un
> número de línea caduca solo, en silencio y con la autoridad de un dato.

| pin | qué es de verdad | nivel |
|---|---|---|
| **`PB0`** | **`CAM_DEMANDA_PIN`** — ~~la **única** entrada de cámara con firmware~~ 🛑 **es UNA de TRES**: también `PB14` y `PB15` se leen como cámara. Bornera **`J14`**, con `R64` 10 kΩ (*pull-**down***) + `C25` 100 nF = antirrebote por hardware de 1 ms. *(11/09, `D-27`: **`J14` queda LIBRE, sin cablear** — ni cámara ni fin de carrera; el firmware lo sigue leyendo como demanda, así que no se conecta nada)* | ✅ **MEDIDO 07/09** (símbolo `CAM_DEMANDA_PIN` en `pines.h`; se lee vía `camara_leerPin(CAM_DEMANDA_PIN)` en `modo_inteligente.cpp` y `digitalRead(CAM_DEMANDA_PIN)` en `Esclavo/src/main.cpp`). ⛔ *(de las tres citas anteriores sólo `main.cpp:350` seguía valiendo)* |
| **`PB8`** | **`LED_TESTIGO`** — salida por `R16` 1 kΩ al LED `D5`. **No es entrada de nada** | ✅ **MEDIDO 07/09** (`pines.h:63` — **re-corrida y correcta**) |
| **`PB9`** (`J16` p5) | **`BOTON1` = `MANDO_A`** del mando de relés. 🛑 **BORNE VACÍO — pero el CÓDIGO lo sigue leyendo** | ✅ **MEDIDO** (símbolos `BOTON1` en `pines.h`, `mando_registrarPulso(MANDO_A)` en `botones.cpp`) |
| **`PB13`** (`J16` p8) | **`BOTON2` = `MANDO_B`**. 🛑 **BORNE VACÍO — y su código es el único que arma `ambarLocal`** | ✅ **MEDIDO** (símbolos `BOTON2` en `pines.h`, `mando_registrarPulso(MANDO_B)` en `botones.cpp`, `ambarLocal = true` en `Esclavo/src/mando.cpp`) |

> ### 🛑 «SE CONSERVA» DECÍA DOS COSAS A LA VEZ, Y SÓLO UNA ES CIERTA (corregido el 07/09)
>
> **Estas dos filas ponían *«se conserva»* a secas** —redacción del 31/08, cuando la decisión era
> conservar el mando entero—. **Se leía como que hay pulsadores montados en `J16` p5 y p8. No los
> hay.**
>
> **`D-1` de [`DECISIONES.md`](../DECISIONES.md)**, confirmado por el responsable el 05/09 —*«ya no
> tenemos mandos de A y B, sólo la app»*— dice **las dos mitades a la vez**, y hay que sostener las
> dos:
>
> | | |
> |---|---|
> | 🛑 **El HARDWARE se fue** | Nunca se compró receptor y ya no se va a comprar. **`J16` p5 y p8 están VACÍOS**, y ningún documento manda conectar nada ahí |
> | ✅ **El CÓDIGO se queda, y NO se toca** | `mando_ambarLocal()` tiene **cinco llamadas vivas** —tres vetos en `Esclavo/src/main.cpp` y dos decisiones de `CANCELAR_AMBAR` en `Esclavo/src/bluetooth.cpp`— y su veto es **SFTY-21**. Retirar el armador deja esos `if` **siempre verdaderos**: el veto **no queda inerte, queda ABIERTO**. Y trece packs caerían en **`ABORTADO`, no en rojo** |
>
> 🔴 **Consecuencia que no caduca: LIBRE DE COBRE NO ES LIBRE DE FIRMWARE.** `botones_actualizar()`
> lee esos dos pines en cada vuelta y alimenta el reconocedor de secuencias. **Lo que alguien cierre
> ahí compone órdenes del mando** —`A·A·A`, `B·B·B`, `A·B·A·B`— sin que nadie lo pida. Con el
> pulsador retirado la bandera simplemente **no se arma nunca**, que es lo correcto.
>
> ⚠️ **Y `N-118` está REFUTADO:** los `0,6 V` que se citaron como *«defecto de placa»* en esos dos
> pines **no lo eran**. En el binario que había en la tarjeta aquel día `BOTON1/2` iban en
> `INPUT_PULLUP` y los pines de cámara en `INPUT` pelado: **mismo cobre, distinto `pinMode`, distinta
> tensión** (9,92–9,94 kΩ en los cuatro). **No se cite como avería.** Hoy no queda **ningún**
> `INPUT_PULLUP` vivo en el firmware: **re-medido el 07/09, los `20` hits del `grep` sobre `src/` e
> `include/` de las dos puntas son comentarios, y `grep -rn "pinMode(.*INPUT_PULLUP"` da `0`.**
> *(La distinción importa: contar `INPUT_PULLUP` cuenta comentarios —en este repositorio los
> comentarios citan lo que explican—, así que el cero hay que medirlo con el patrón del `pinMode`,
> no con el del nombre.)*
| **`PB14`** (`J16` **p10**) | **`CAM_C_PIN` — LA CÁMARA, ~~una por poste~~** → **11/09, `D-25`: la CÁMARA 1 de cada poste**, contra los 3,3 V de p9. `INPUT` pelado, **activo en ALTO** | ✅ **MEDIDO** (`#define CAM_C_PIN`, `pinMode(CAM_C_PIN, INPUT)`) |
| **`PB15`** (`J16` **p12**) | **`CAM_D_PIN`** — pin de cámara, ~~hoy **vacío**~~ → **11/09, `D-25`: la CÁMARA 2 de cada poste**, contra los 3,3 V de p11. `INPUT` pelado, **activo en ALTO**. **Hace LO MISMO que `CAM_C_PIN`**: `botones.cpp` recorre `CAM_J16[2] = {CAM_C_PIN, CAM_D_PIN}` en el mismo bucle | ✅ **MEDIDO** (`#define CAM_D_PIN`, `pinMode(CAM_D_PIN, INPUT)`, `CAM_J16`) |

> ⛔ **LO QUE ESTAS DOS FILAS DECÍAN HASTA EL 05/09, y por qué se tacha en vez de corregirse en
> silencio:** *«~~Hoy `botonAceptar()`~~ · ~~Hoy `botonCancelar()`~~. Destino decidido de cámara tras la
> Fase 3»*, **con un ✅ MEDIDO al lado**. Era **falso desde el 31/08**: el destino ya llegó. Una
> afirmación falsa con la palabra *«MEDIDO»* encima es **peor que sin ella** —quien la lee deja de
> ir a la fuente—, y por eso queda escrito que aquí hubo una.
>
> **El estado de hoy, con el `grep` que lo encuentra —re-corrido el 07/09:**
>
> ```
> $ grep -n "define CAM_._PIN" 01_Firmware/Maestro/include/pines.h
> 165:#define CAM_C_PIN   PB14  // J16 p10 - camara de contacto seco (era BOTON3, "Aceptar")
> 166:#define CAM_D_PIN   PB15  // J16 p12 - camara de contacto seco (era BOTON4, "Cancelar")
> $ grep -n "^bool botonAceptar\|^bool botonCancelar" 01_Firmware/Maestro/src/botones.cpp
> 672:bool botonAceptar() { return false; }
> 673:bool botonCancelar(){ return false; }
> ```
>
> ⛔ **Re-corrido el 07/09: los CUATRO números se habían movido** (`148`/`149`→`165`/`166`,
> `659`/`660`→`672`/`673`). **La salida del `grep` es idéntica palabra por palabra; sólo cambió su
> posición** — que es exactamente por qué la posición no es la cita.
>
> Las dos líneas son idénticas en el Esclavo. `botonAceptar()`/`botonCancelar()` **siguen declaradas
> a propósito** —devolviendo `false` el compilador conserva cada punto de uso y `git grep` sigue
> listando de una vez todo lo que la pantalla movía—, pero **ya no leen ningún pin**.
>
> 🔴 **Y EL AVISO QUE NO SE PUEDE PERDER, porque libre de cobre NO es libre de firmware:**
> `botonArriba()` y `botonAbajo()` **SIGUEN VIVOS** —`consumir(0)` / `consumir(1)`, con llamadores en
> `menu.cpp`, `modo_hora.cpp` (Maestro) y `menu.cpp` (Esclavo)— y leen **`BOTON1`/`BOTON2`, que son
> los pines del mando (`PB9` / `PB13`, `J16` p5 y p8)**. El **hardware** del mando se retiró (`D-1`);
> **su código se queda**. Lo que alguien cierre sobre `J16` p5 o p8 **SIGUE ENTRANDO al firmware**.

> 🔴 **`PB9`/`PB13` no son un destino posible para una cámara, ni siquiera «provisionalmente».** Son
> los dos canales del mando: tres pulsos dentro de la ventana de **12 s** (símbolo
> `VENTANA_TRIPLE_MS` en `mando.cpp`; ⛔ *cita `:38` caducada*) componen una secuencia —`A·A·A` =
> Modo Automático, `B·B·B` = Ámbar y armado de `ambarLocal`—, así que **el tráfico cambiaría el modo
> del semáforo solo**. Detalle completo en `MANUAL_USUARIO.md` §6.
>
> 🔴 **Y lo que se midió el 07/09 y agrava esto: de las tres secuencias, `A·A·A` es la ÚNICA que
> ABRE PASO y la ÚNICA SIN GUARDA.** `case ACC_AUTOMATICO:` llama a
> `modoAutomatico_pedirArranqueDirecto()` y arranca el ciclo **sin comprobar nada**; `A·B·A·B` sí
> tiene su `if (modo_degradado_evaluarEntrada() == MDG_OK)` delante, y `B·B·B` va a un estado
> seguro. **Un hilo suelto en `J16` p5 no molesta: programa un verde.** Desarrollo en
> [`MANUAL_MANDO_4_RELES.md`](MANUAL_MANDO_4_RELES.md) §8.

### 3.3 ✅ `M3` CERRADA EL 03/09 — las cámaras de `J16` **se cablean**

> ⛔ **ESTE APARTADO DECÍA «no se cablea todavía» Y ESO CADUCÓ.** Se tacha con su motivo, no se
> borra. **`M3` se cerró el 03/09 con medidas en cobre** (paso 20 de la Guía de banco, multímetro y
> conector vacío), y la fuente que manda en esto es
> [`05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md`](../05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md)
> sección **M3**, no este manual. Los dos bloqueos de abajo ya no existen:
>
> 1. ~~**Falta la medida M3** — la polaridad está en contradicción entre el netlist y `botones.cpp`,
>    que lee `== LOW` con `INPUT_PULLUP`.~~ ✅ **RESUELTA, y en los DOS lados a la vez.** En cobre:
>    el pull-**down** de **10 kΩ** (`R65`–`R68` con su 100 nF) **es real y está en las cuatro
>    posiciones**; `p10` y `p12` dan **0 V en reposo**, y el paso 21 cableó `p10` en normalmente
>    abierto **sin demandas fantasma**. Y en el fuente **ya no queda ningún `INPUT_PULLUP`**: los
>    botones pasaron a `INPUT` pelado leído **`== HIGH`**, que es lo que el cobre pedía.
>    **Activa en ALTO, los cuatro pines y sin excepción.**
>
>    ```
>    $ grep -n "INPUT_PULLUP" 01_Firmware/Maestro/src/botones.cpp
>    26:  // Aqui ponia `== LOW` con los pines en INPUT_PULLUP, y eso llevaba mal desde el primer
>    32:  // Con INPUT_PULLUP, el pull-up interno (30-50 kOhm) contra ese 10K deja el pin en
>    518:  // que se leen igual. Aqui ponia INPUT_PULLUP "y ese camino no se toca": el camino sigue
>    527:  // LAS ENTRADAS DE CAMARA: INPUT PELADO, NUNCA INPUT_PULLUP. El reposo lo fija el
>    ```
>
>    — **cuatro comentarios que cuentan la historia, y ni un solo `pinMode(..., INPUT_PULLUP)` vivo.**
>    ⚠️ **Y esa última frase no se deduce de este `grep`, que cuenta comentarios: se mide aparte, y
>    se re-midió el 07/09** —
>    `grep -rn "pinMode(.*INPUT_PULLUP" Maestro/src Maestro/include Esclavo/src Esclavo/include | wc -l`
>    → **`0`**. *(Y ⛔ dos de las cuatro líneas de arriba se habían movido: `511`→`518`, `520`→`527`.)*
>
> 2. ~~**Orden asimétrico:** `PB14` es `botonAceptar()`, el que EJECUTA, leído activo en BAJO.~~
>    ✅ **SIN OBJETO: `botonAceptar()` ya no lee ningún pin** —es `return false;`— y `PB14` es
>    `CAM_C_PIN`. La regla de `CLAUDE.md` §9.bis (**firmware primero; el cableado después**) **sigue
>    valiendo tal cual** para cualquier equipo al que aún no se le haya cargado el firmware nuevo:
>    lo que la levanta no es el commit, es **la carga verificada en la tarjeta**.
>    🔴 **11/09, `D-25`: y con la cámara 2 en `p12` vale igual para `PB15`** — en un binario
>    anterior a `deeeab4` (el V8.4 `e303485` de campo incluido) `PB14`/`PB15` son
>    `botonAceptar()`/`botonCancelar()` (`git show e303485:01_Firmware/Maestro/src/botones.cpp`):
>    lo que se cablee en `p10` **o** en `p12` puede actuar como un botón.

**Lo que SÍ sigue vigente antes de cablear:**

1. 🔴 **`J16` p1 lleva 12 V crudos** —sin opto, sin limitadora, sin clamp— y **se tapa físicamente
   antes de cablear**. La separación **real sobre cobre** contra la red de 12 V, **MEDIDA** sobre el
   `.kicad_pcb` y publicada en `03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md` —tabla *«separacion
   minima real»*, que se encuentra con `grep -n "separacion minima real"`—, no es el paso del
   conector:

   | red de 12 V contra | separación mínima real |
   |---|---|
   | `/Boton1` (p5) | 1,405 mm |
   | `/Boton2` (p8) | 1,408 mm |
   | `/Boton3` (**p10**) | **4,269 mm** |
   | `/Boton4` (**p12**) | **1,359 mm** ← el peor |

   👉 ~~**Si una de las dos cámaras es más crítica, va en `p10`**, no en `p12`.~~ → 🔴 **11/09,
   `D-25`: van las DOS, cámara 1 en `p10` y cámara 2 en `p12`, y son iguales —ninguna es «más
   crítica»—. O sea que `p12`, el peor borne del conector, SIEMPRE lleva cable: `p1` tapado antes
   del primer hilo (`D-4`) y trabajo limpio en `p12`.**
   👉 **Y la app no las distingue:** pinta `CAM: OK` —*«las dos ven y ninguna está pegada»*— con la
   primera detección de **cualquiera** (`camara_estado()` salta la que nunca dio flanco), así que
   **cada cámara se comprueba en su borne con el multímetro**: 0 V en reposo, 3,3 V con algo en la
   zona. Una cámara 2 muerta desde la instalación **no la anuncia el equipo**.
2. 🔴 **`J16` p5 y p8 —`MANDO_A`/`MANDO_B`— NO son pines libres, aunque su hardware ya no esté.**
   El **mando de relés se retiró físicamente** (`D-1`), pero **su código se queda**: `botonArriba()`
   y `botonAbajo()` siguen vivos y leen `BOTON1`/`BOTON2`, que **son esos dos pines**. **Libre de
   cobre no es libre de firmware:** lo que alguien cierre en `p5` o `p8` sigue entrando. No se
   cablea cámara ahí —ver el aviso de §3.2—.

> ⛔ **Aquí había un cuarto punto que decía *«~~Ningún firmware lee `PB14`/`PB15` como cámara
> todavía: siguen en `pinMode(..., INPUT_PULLUP)` como botones~~»*.** **Falso desde el 31/08 en las
> dos mitades de la frase:** hay firmware que los lee como cámara, y el `pinMode` es `INPUT` pelado.
> Se tacha con su motivo porque era el argumento en que se apoyaba el *«no se cablea todavía»* de la
> cabecera de este mismo apartado.

Detalle de parametrización de la cámara —incluidas las **dos configuraciones `NO`/`NC`** de la salida
de relé y cuál elegir según **M3**— en
[`05_Funcional/9_Manual_Parametrizacion_Camara_IA.md`](../05_Funcional/9_Manual_Parametrizacion_Camara_IA.md).

> **Nada de este apartado ha pasado prueba de banco y no autoriza a instalar ni a cablear nada.**

---

## 3.bis 🔴 `J14` es una ENTRADA. La salida de la talanquera es `J15`. No se confundan

> **Este manual no mandaba cablear nada mal aquí — el aviso va porque el error es fácil, ya se ha
> escrito antes en este proyecto, y se ejecuta con un destornillador.**

| bornera | qué es | pin | qué hay detrás |
|---|---|---|---|
| **`J14`** | 🔴 **ENTRADA del micro** | `PB0` (`CAM_DEMANDA_PIN`) | **`R64` 10 kΩ a masa + `C25` 100 nF. Y NADA MÁS: sin opto, sin diodo, sin limitadora.** El borne saca el pin **junto a 3,3 V** |
| **`J15`** | ✅ **SALIDA de potencia** | `PB2` (`MOTOR_TALANQUERA`) | opto `TLP127` (`U15`) → MOSFET `IRLZ44N` (`Q10`) → bornera, con su diodo de rueda libre |

🔴 **La trampa está en el esquemático: `J14` viene rotulada «Puerta».** Quien busque dónde enchufar
la pluma y lea ese rótulo cablea un **relé de 12 V contra una entrada de 3,3 V sin protección
ninguna**, y se lleva el micro por delante. **La pluma va a `J15`.**

✅ **MEDIDO en el fuente el 07/09** — se citan los símbolos:

```
$ grep -n "MOTOR_TALANQUERA\|CAM_DEMANDA_PIN" 01_Firmware/Maestro/include/pines.h
31:#define MOTOR_TALANQUERA   PB2  // -> opto U15 -> MOSFET Q10 -> bornera J15
46:#define CAM_DEMANDA_PIN    PB0  // -> R64 10K + C25 100nF -> bornera J14 (antirrebote 1 ms)
```

La talanquera se mueve **sólo desde `escribirPines()` de `semaforo.cpp`** —la barrera de salidas— y
arranca **cerrada** en `semaforo_setup()`: `LOW` = MOSFET sin conducir = pluma abajo. Es el fallo
seguro de SFTY-28.

🔴 **11/09 — CÓMO SE CABLEA Y CUÁNDO SUBE, que es lo que el instalador tiene que llevarse (`D-25`):**

- **Cableado decidido:** `J15` **p1 = 12 V**, **p2 = drenador de `Q10`** → a la **bobina de un relé**;
  el contacto del relé va a la entrada **`OPEN`** de la centralita de la barrera. ⚠️ **p2 NO es
  masa**: con `Q10` abierto está a ~12 V.
- **Cuándo sube, medido en `escribirPines()`:** `(verde && !testLedsActivo) || estado == S_FALLO`.
  **Con verde, y TAMBIÉN con el ámbar intermitente** (`S_FALLO`): Modo Ámbar, radio perdida y **un
  poste recién encendido que aún no enlaza con el otro**. No sólo con verde.
- **Ninguna cámara la frena.** `escribirPines()` no lee ninguna cámara: **la pluma baja al acabar el
  verde aunque haya un coche debajo**. El veto es `A-1.bis`, **sin construir**.
- Qué hace la centralita cuando el relé suelta `OPEN` (si baja sola, con qué retardo) **no lo
  controla este equipo y no está medido**: se comprueba con la barrera real delante.

🔴 **Y `J14` (`PB0`): ~~`A-2` (05/09) lo reserva al fin de carrera de la pluma, pero~~ el firmware lo
sigue leyendo como `CAM_DEMANDA_PIN`** —el Maestro por nivel en Inteligente, el Esclavo por flanco,
mandando `CMD_DEMANDA`—: ~~un fin de carrera ahí daría **demandas falsas**. **CONFLICTO ABIERTO**; no
lo resuelve este manual, y hasta que se resuelva no se cablea nada en `J14` sin preguntarlo.~~ →
🟢 **11/09, `D-27`: `J14` queda LIBRE, sin cablear — el fin de carrera NO se instala en este
despliegue.** Lo que se cablee ahí pediría paso como una cámara, así que **en `J14` no se conecta
nada**; vacío, `R64` lo deja en 0 V y no pide nada (`J14` medido en banco el 03-04/09, pasos 17-18).

🛒 **Y el relé de la talanquera** (`D-27` punto 4, como la guía del Sisga): **bobina de 12 V DC**
entre `J15` p1 y p2 y **contacto NA** a `OPEN` y su común en la centralita; **sin módulo
optoacoplado ni `JD-VCC`** —`J15` sólo da esos dos hilos—. La rueda libre ya está en la placa
(`D30`, `1N4148` de 200 mA): la bobina tiene que quedar por debajo; la referencia la decide el
responsable (línea `A4` de `05_Funcional/15_Lista_de_Compras_Hardware.md`).

⚠️ **Y `J15` no está a 0 V en reposo, está a ~12 V** por el pull-up de 1 kΩ del cobre — ver §5.bis
punto 4. En banco dio *«0 V en rojo, 12 V en ámbar»*, que es exactamente eso.

---

## 4. Módulo de expansión (Bluetooth + reloj) para Telemetría y Diagnóstico Móvil

> ## 🛑 QUÉ SE ENCHUFA HOY EN `J17`: UN **ESP32**, NO UN MÓDULO BLUETOOTH SUELTO (07/09/2026)
>
> **Los pines y el conector de abajo son correctos y no cambian.** Lo que cambia es **qué hay al
> otro lado del cable**, y con ello dos cosas que se compran:
>
> | | |
> |---|---|
> | 🛑 **Ya no se compran `HC-05` ni `JDY-30`** | El módulo SPP discreto está **sustituido por un `ESP32-WROOM-32` clásico** (`BT v4.2 BR/EDR + BLE`, o sea **sí hay perfil SPP**). Su firmware existe y compila: `01_Firmware/ESP32_Expansion/` |
> | ✅ **Y ese ESP32 trae el RELOJ del cruce** | Un **`DS3231` con pila propia** por I²C en `GPIO21` (`SDA`) / `GPIO22` (`SCL`). **`D-9`/`D-15`:** el STM32 **no tiene reloj propio** (`Y2` muerto, N-17) y el del ESP32 es **el único que contesta a `SET_RTC`**. 🔴 **Y desde `D-20` (07/09), el del ESP32 MAESTRO es el único que lo contesta EN TODO EL CRUCE** — ver la nota |
> | ⚠️ **El ESP32 NO se alimenta del 3,3 V de `J17`** | Ese riel alimenta al STM32 que gobierna el cruce; **el accesorio no puede tumbar al que manda**. Fuente propia desde los 12 V |
> | 🛑 **No manda sobre las luces** | Es un puente: traduce y reenvía. La barrera de salidas sigue en `semaforo.cpp` del STM32 |
>
> > # 🔴 07/09 — `D-20`: HAY UNA SOLA FUENTE DE HORA EN EL CRUCE, Y ES EL ESP32 MAESTRO
> >
> > `DECISIONES.md` `D-20` (decidida por el responsable): **la app le da la hora al ESP32 Maestro; ése
> > al ESP32 Esclavo; y el STM32 de cada punta la recibe de SU PROPIO ESP32.** El Maestro manda y el
> > Esclavo hace caso siempre — **una sola fuente, así que no hay desfase inicial que acotar.**
> >
> > 🔴 **Consecuencia dura para quien monta y para quien opera: la app NO pone la hora en el poste 2.
> > Nunca.** Un `SET_RTC` dirigido al Esclavo **se rechaza**: no es una sincronización, **es una
> > segunda fuente**. *(Consultar sí: `CMD:LEER_RTC` (`D-17`) se sigue mandando **a los dos postes**
> > — leer no escribe nada.)*
> >
> > **Los dos ESP32 no se hablan entre sí.** El único enlace entre postes es la **radio entre los
> > STM32**, así que la hora viaja `ESP32-M -> STM32-M -> radio -> STM32-E -> ESP32-E`: **los STM32
> > son CARTEROS de la hora, no dueños.** Por eso *«el STM32 no tiene reloj»* hay que leerlo como
> > **no tiene reloj PROPIO** — sí lleva la hora, sólo que no es suya.
> >
> > ⚠️ **SIN CONSTRUIR.** `D-20` decide la **autoridad**, no la implementación. Falta el mando
> > ESP32 → STM32 que siembre la hora (`ESP32_Expansion/src/enlace_stm32.cpp` **no menciona `RTC` ni
> > `hora` ni una vez**, comprobado el 07/09), y falta el rechazo en el poste 2: el puente es **el
> > mismo firmware en los dos postes** y su despachador **no sabe en cuál está** —
> > `grep -c "ESCLAVO\|Esclavo\|esclavo" 01_Firmware/ESP32_Expansion/src/despachador.cpp` → `0`.
> >
> > 🔴 **Y lo que NO cambia, para que nadie retire una compra: siguen haciendo falta DOS `DS3231`,
> > uno por poste** (`A-5`). El del Esclavo es el que **conserva la hora con su pila cuando se cae la
> > radio** — y de ahí la regla de campo: **el poste 2 se pone en hora en la PUESTA EN MARCHA, no
> > durante la avería**, porque la radio se cae justo cuando hace falta el Modo Degradado, que es el
> > modo que exige hora. *Perder la radio no es perder la hora.*
> >
> > 🔴 **Ni se retira la `CR2032` del STM32.** Sigue siendo **obligatoria**, y ya no por la hora: es
> > la que alimenta el **dominio de respaldo** (`BKP->DR1..DR10`, símbolo `respaldo.h`), donde viven
> > la marca de sincronización y el indicador del Degradado — **el cómputo de las 48 h**. Y no se
> > escriba *«el STM32 no tiene ni pila ni cristal»*: **es falso**. `VBAT` midió **3 V con la tarjeta
> > apagada** (N-37, **en al menos una** tarjeta; la otra `SIN VERIFICAR`) y `Y1` de 8 MHz **está en
> > la placa**. **Lo muerto es `Y2`, y sólo `Y2`** — el cristal de 32.768 kHz del RTC.
> > *(Lo que **no** está verificado, y no se repita como medido: que el firmware arranque con el
> > **HSI** en vez de con `Y1`. Eso **no aparece en el fuente de este proyecto** —`grep` de `HSI`
> > sobre `Maestro/src`, `Maestro/include` y `platformio.ini` no devuelve nada, 07/09—; si es cierto
> > viene del *variant* del core STM32duino. **`SIN VERIFICAR`.**)*
>
> ✅ **MEDIDO el 07/09:** `DS3231_SDA 21` / `DS3231_SCL 22` en `ESP32_Expansion/include/contrato.h`,
> con driver en `src/reloj_ds3231.cpp` (`#include <Wire.h>`). Y en el firmware del STM32 **no queda
> ni una configuración de módulo Bluetooth clásico**:
>
> ```
> $ grep -rni "HC-05\|HC05\|JDY" 01_Firmware/Maestro/src 01_Firmware/Maestro/include \
>                                01_Firmware/Esclavo/src 01_Firmware/Esclavo/include | wc -l
> 0
> ```
>
> ⚠️ **El `grep` va ACOTADO a `src/` e `include/` a propósito, y eso es parte de la medida.** Sin
> acotar —como estaba publicado— **da hits**: los directorios `.pio/` traen la librería `U8g2`, y
> la cadena `JDY` aparece dentro de sus **datos binarios de fuentes**. Un cero de `grep` sólo vale
> si se dice sobre qué se corrió.
>
> ⚠️ **Ojo con el PIN, que son DOS y no el mismo:** el `0000`/`1234` del **emparejado de Android** es
> el del módulo; el `1234` de `CMD:PIN:1234:` es el del **semáforo**. Detalle completo en
> [`MANUAL_CONFIGURACION_BLUETOOTH.md`](MANUAL_CONFIGURACION_BLUETOOTH.md) y en
> [`MANUAL_INSTALACION_RELOJ_DS3231.md`](MANUAL_INSTALACION_RELOJ_DS3231.md) §7.

Para soporte técnico, caja negra de alarmas y monitoreo desde el suelo sin subir al poste (estándar probado en proyecto Baliza):
* **Pines de Conexión:** Puerto **USART1 REMAPEADO** del STM32 — **`PB6` TX ➔ `RXD` BT, `PB7` RX ➔ `TXD` BT**, conector **`J17`**. ✅ **MEDIDO EN EL FUENTE el 07/09:** `grep -n "HardwareSerial SerialBT" 01_Firmware/Maestro/src/bluetooth.cpp` → **`30:static HardwareSerial SerialBT(PB7, PB6);`** (Esclavo, `29:`); el porqué del remapeo está en el comentario que empieza *«USART1 REMAPEADO a PB7 (RX) / PB6 (TX)»*, justo encima. ⛔ *(las citas `:28` y `:16-22` estaban caducadas.)*
  > ⛔ Este apartado publicó ~~«`PA9` TX ➔ `RXD` BT, `PA10` RX ➔ `TXD` BT»~~ hasta el 31/08/2026. Ese
  > era el sitio del puerto **antes de `N-76`**, y no es un detalle de redacción: manda soldar el módulo
  > Bluetooth a dos pines por los que hoy **no sale ni un byte**. El técnico no vería un error — vería
  > un módulo mudo, que es el fallo más caro de diagnosticar desde el suelo.
* **Desacoplo Eléctrico:** Pin `PA8` forzado en `HIGH` permanente para poner en alta impedancia ($\text{Hi-Z}$) el transceptor `MAX3485 U3`.
* **Alimentación:** `5V` (o `3.3V`) y `GND` de la tarjeta controladora.
* **Funcionalidad:** Envío continuo de telemetría (estado de luces, modo, cuenta regresiva `T:`, calidad de señal RF, % de paquetes y motivos de alarma ~~con hora exacta de RTC~~ **con la hora que rellena EL PUENTE ESP32, no el STM32**) hacia la App en celular con comandos protegidos por PIN (`1234`). Detalle completo en [`05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.md`](../05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.md).

> 🔵 **07/09 — POR QUÉ SE TACHA «hora exacta de RTC».** Se leía como *«la hora la pone el RTC del
> STM32»*, y **no es así desde `D-9`/`D-15`**: el STM32 emite el hueco `HORA:--:--:--` —que es el
> firmware **negándose a inventar**, no fallando— y **el puente ESP32 sella ese hueco con la hora de
> su `DS3231` y recalcula el checksum** (`N-145`, símbolo `puente.cpp`). O sea que el sello de tiempo
> de una alarma **es tan exacto como el `DS3231` del poste desde el que se lee**, y **no** hay ningún
> RTC del STM32 detrás.
>
> 🔴 **Y con `D-20` (07/09) eso adquiere una consecuencia de campo:** como la única fuente de hora es
> el **ESP32 Maestro**, dos alarmas leídas en postes distintos sólo llevan el mismo sello si **la
> siembra del Maestro llegó al poste 2**. Se comprueba con `CMD:LEER_RTC` (`D-17`) **en los dos
> postes** — leer no escribe. ⚠️ **La siembra está SIN CONSTRUIR** (ver §4, recuadro de `D-20`), así
> que **hoy los dos sellos pueden diferir y nada lo avisa.**

---

## 5. Placa Base (Mainboard STM32)

La tarjeta principal de control que gobierna las luces del semáforo.

### Componentes Clave:
- **Cerebro:** Microcontrolador **STM32F103C8T6** ("Blue Pill" / CKS32F103).
- **Pantalla y UI:** ⛔ ~~LCD ST7920 (128x64 píxeles) conectada por SPI de 3 hilos. 4 botones físicos de navegación.~~
  🛑 **NO SE MONTA NINGUNA DE LAS DOS COSAS (`D-17.bis`, 05/09).** Ver el aviso §5.bis de abajo.
- **Salidas de Potencia:** Transistores MOSFET N-Channel (**IRLZ44N**) a 12V/24V para lámparas LED (Rojo, Amarillo, Verde).
- **Aislamiento:** ⛔ ~~9 Entradas Optoacopladas para aislamiento eléctrico de botoneras y sensores.~~
  🛑 **Mal en las tres palabras: ni son 9, ni son entradas, ni «aíslan» lo que esa frase sugiere.**
  Lo medido: los optoacopladores **`TLP127` están en las DIEZ CADENAS DE SALIDA** (`Q1`–`Q10` con
  `U6`–`U15`), no en las entradas; las **entradas de campo son 5**. Ver §5.bis.
- **Transceptor Integrado:** Chip MAX485 (Half-Duplex) conectado a la bornera RS485 `A` / `B`.
- **Watchdog:** Inicialización IWDG por registros directos adaptada a CKS32F103 (refresco en `loop()`).

---

## 5.bis 🛑 Correcciones al apartado 5 (07/09/2026) — se tachan arriba y se explican aquí

### 1. La pantalla y los cuatro pulsadores **no se montan**

**`D-17.bis` de [`DECISIONES.md`](../DECISIONES.md)** (05/09, deroga `D-6`): *«LA PANTALLA LCD Y EL
MENÚ SE RETIRAN DEL EQUIPO. Todo se opera por la app.»*

**Y el matiz que hay que sostener, porque decir «no existe» sería otra frase falsa: se retira del
EQUIPO, no del código.** `lcd.cpp` y `menu.cpp` compilan, y el arnés `Validacion_LCD` sigue dando
`271/271` **sobre un framebuffer en el PC**. Lo que muere es la **interfaz**. Medido en el fuente el
07/09 — el objeto entrega **los cuatro pines en `U8X8_PIN_NONE`**, así que la librería no emite ni un
`pinMode` ni un `digitalWrite`:

```
$ grep -n "U8X8_PIN_NONE" 01_Firmware/Maestro/src/lcd.cpp
74:static U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, U8X8_PIN_NONE, U8X8_PIN_NONE,
75:                                        U8X8_PIN_NONE, U8X8_PIN_NONE);
```

👉 **Consecuencia de cobre que sí importa:** `PB3` (`LCD_SCLK`), `PB4` (`LCD_CS`) y `PB5` (`LCD_SID`)
quedan **en alta impedancia, con pista hasta `J17`** p4, p1 y p5. Son los únicos GPIO libres del
proyecto **con bornera ya cableada**.

**De los cuatro pulsadores:** `BOTON3`/`BOTON4` **ya no existen** —son `CAM_C_PIN`/`CAM_D_PIN`
(§3.2)— y `botonAceptar()`/`botonCancelar()` son `return false;` en las dos puntas. `BOTON1`/`BOTON2`
**siguen declarados y siguen leyéndose**, pero su hardware tampoco se monta (`D-1`).

### 2. Los optoacopladores están en las SALIDAS, y son DIEZ

| | lo que decía §5 | lo medido |
|---|---|---|
| **cuántos** | 9 | **10 cadenas** — `Q1`–`Q10` con `U6`–`U15` |
| **dónde** | «entradas … de botoneras y sensores» | **en las salidas de potencia**: `R` 220 Ω → opto `TLP127` → `R` 10 K + 220 Ω → MOSFET `IRLZ44N` → bornera, con diodo de rueda libre |
| **las entradas** | — | son **5**, y van **del borne directo al micro** |

Fuente: `05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md`, que manda en cobre medido.
⚠️ **Ese documento se contradice a sí mismo en este punto** —una tabla suya sigue diciendo «Salidas
de campo: 9» mientras su propio texto lo tacha y lo corrige a diez—; **aquí se recoge la cifra
corregida y se deja anotada la discrepancia, sin elegir por él.**

### 3. 🔴 «El opto aísla galvánicamente» es MEDIO CIERTO — y de ahí sale una conclusión falsa

**Hay UNA sola red `GND` de 103 pads**, y en ella están el cátodo del LED del opto y la fuente del
MOSFET. **El opto separa el pin del micro del nodo de puerta; NO crea una masa separada.**

👉 **Cualquier cosa colgada de esas borneras COMPARTE LA MASA DEL CONTROLADOR.** Quien lea «aislado»
y conecte un equipo con su propia tierra puede meter una diferencia de potencial en la masa del
micro.

### 4. 🔴 El borne NO está a 0 V en reposo: está a ~12 V

**Nueve de los diez** drenadores llevan un **pull-up de 1 kΩ + LED al riel de 12 V** (`R23`, `R28`,
`R33`, `R38`, `R43`, `R48`, `R53`, `R58`, `R63`, `R73`) que está **en el cobre, no en el conector**:
**no se evita dejando un hilo sin poner.** Con el MOSFET abierto el borne sube a **~12 V**, con
~10 mA *calculados* —la cuenta `(12 − Vf)/1 kΩ`, no una sonda—.

**Encaja con lo medido en banco:** `J15` dio *«0 V en rojo, 12 V en ámbar»* — es exactamente este
circuito con la sonda entre p1 y p2.

> 🟡 **Y el décimo es la excepción, en un canal de luz vivo:** `D21`, el LED del canal `Q6` → `J8` →
> `VERDE2`, **tiene el cátodo sin conectar** (red `unconnected-(D21-K-Pad1)`, cero pistas en el
> `.kicad_pcb`). Ese canal **no tiene indicador luminoso** y **`J8` p2 flota en reposo** en vez de
> subir a ~12 V. Queda **`SIN VERIFICAR`** si es defecto o decisión (`A-10`): es comprobación de
> banco, no de firmware.

### 5. 🔴 Tres canales de potencia completos SIN una línea de firmware detrás

`J9` (`VERDE_PEATON`, `PA6`), `J11` (`ROJO_PEATON`, `PA7`) y `J13` (`BUZZER`, `PB1`) están
**fabricados enteros** —opto, MOSFET, diodo, bornera— y el firmware **no los toca**: medido el
07/09, cero `pinMode` y cero `digitalWrite` sobre los tres en las dos puntas.

```
$ grep -rn "PEATON\|BUZZER" 01_Firmware/Maestro/src 01_Firmware/Esclavo/src
01_Firmware/Maestro/src/main.cpp:35:  // ROJO_PEATON y VERDE_PEATON, que estaban sin custodia.
```

**Un solo hit, y es un comentario.** *(Re-corrido el 07/09: sigue dando exactamente esta línea —
una de las pocas citas de este manual que no caducó.)* No es una avería: es obra no hecha. Se anota
para que nadie prometa una cabeza peatonal o un zumbador como función del equipo.

> ## 🔴 Y DE AQUÍ CUELGA UNA DECISIÓN YA TOMADA QUE **NO TIENE CAMINO**: `D-14` (07/09)
>
> **`D-14` de [`DECISIONES.md`](../DECISIONES.md)** decide que la vía buena para grabar las
> incidencias es **la ENTRADA de alarma de la cámara**: *«el controlador cierra un contacto en ROJO
> → la cámara graba»*, verificado en el manual del fabricante y **sin la coletilla de modelos** que
> bloquea la otra vía. Es la única vía documentada de punta a punta.
>
> 🔴 **Lo que NO estaba escrito en ninguna parte, y es de esta página: ESE CONTACTO NO EXISTE EN EL
> FIRMWARE.** No es que falte configurar la cámara — **es que el controlador no tiene hoy ninguna
> salida con la que cerrarlo**:
>
> | salida | estado |
> |---|---|
> | Las **seis** de luz (`ROJO1/2`, `AMARILLO1/2`, `VERDE1/2`) | ✅ vivas, pero **son la luz**: no se pueden reutilizar |
> | `J15` (`MOTOR_TALANQUERA`, `PB2`) | ✅ viva, y **es la pluma** |
> | **`J9`** (`VERDE_PEATON`), **`J11`** (`ROJO_PEATON`), **`J13`** (`BUZZER`) | 🛑 **fabricadas enteras y DECLARADAS Y MUERTAS** — el `grep` de arriba: cero `pinMode`, cero `digitalWrite` |
>
> 👉 **O sea que `D-14` necesita gastar uno de esos tres canales muertos, y eso es firmware que hoy
> no está escrito.** El molde existe y está probado —es el mismo de `J15`, que **funcionó en banco
> el 04/09**— y el coste está medido en **16 B de flash por desensamblado**, pero **el trabajo no
> se ha hecho y nadie debe darlo por hecho al parametrizar la cámara.**
>
> ⚠️ **Y el aviso eléctrico que va pegado, porque quien cablee ahí se lo encuentra:** esos bornes
> **no están a 0 V en reposo, están a ~12 V** (punto 4 de este mismo apartado), y **comparten la
> masa del controlador** (punto 3). La entrada de alarma de la cámara espera un **contacto seco**;
> lo que estos bornes entregan es un **drenador con pull-up a 12 V**. **No se conectan directamente
> sin resolver eso**, y cómo resolverlo **no está decidido**: va como pregunta, no como
> instrucción.

---

## 6. Placa Repetidora (ESP32 V7.6)

Ubicada en la carpeta [`01_Firmware/Repetidor`](../01_Firmware/Repetidor).

### Componentes Clave:
- **Cerebro:** Microcontrolador **ESP32 Dev Module**.
- **Puertos Seriales:** `RadioA` (UART1: RX=16, TX=17, DE/RE=4) y `RadioC` (UART2: RX=32, TX=33, DE/RE=22).
- **Firmware V7.6:** Passthrough asíncrono con `delay(2)` y 15ms de ventana de ráfaga que previene la fragmentación de tramas binarias.

---

## 7. Guía de Pruebas y Carga de Firmware en PlatformIO

### 5.1 Carga del Firmware a los STM32 y ESP32:
En PlatformIO (VSCode), usar el botón **"Open Project"** y abrir de forma independiente la carpeta específica del nodo a programar:
1. `01_Firmware/Maestro` para la tarjeta Maestro STM32.
2. `01_Firmware/Esclavo` para la tarjeta Esclavo STM32.
3. `01_Firmware/Repetidor` para la tarjeta Repetidor ESP32.

> **Tip para Chips Clones CKS32F103:** Si el ST-Link rechaza la primera carga, mantener presionado el botón RESET de la tarjeta BluePill al hacer clic en "Upload" y soltarlo en el instante en que la consola indique "Connecting...".
