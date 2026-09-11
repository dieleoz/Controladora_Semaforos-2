# 🔌 MANUAL 13: ESPECIFICACIÓN DE HARDWARE, MODIFICACIÓN DE PCB Y COMPRAS PARA EXPANSIÓN I²C Y POTENCIA (V9.0)

**Sistema:** Controladora de Semáforos Móviles de 3 Estados (Maestro y Esclavo)  
**Propósito:** Guía de taller para modificación de placa madre KiCad, ensamble del bus I²C, compras locales y etapa de potencia  
**Estado:** 🛑 **DISEÑO DE HARDWARE / PRE-IMPLEMENTACIÓN (V9.0). NO ESTÁ EN EL FIRMWARE V8.9.**  
**Fecha de Emisión:** 27 de Agosto de 2026  
**Última Revisión:** 11 de Septiembre de 2026 (tarde) — 🟢 **`D-27`: `J14` queda LIBRE, sin cablear —el fin de carrera NO se instala en este despliegue—, lo que cierra el conflicto de `A-2` que la revisión de abajo dejaba abierto** (fe de erratas, §3, §4.1, §5 y §6, tachado con esta fecha); y **la pieza de la talanquera es un relé con bobina de 12 V DC y contacto NA**, no un módulo optoacoplado con `JD-VCC` (§3, línea `A4` del Manual 15).  
**Revisión anterior:** 11 de Septiembre de 2026 — 📷 **`D-25`: la talanquera va a `J15` por la BOBINA de un relé (p1 12 V, p2 drenador de `Q10`, que NO es masa) y su contacto a la entrada `OPEN` de la centralita**; se tacha la conexión directa del drenador a `OPEN` de §3, se corrige lo que §3 decía de «la pluma se queda abajo», y `J14` deja de llamarse «la entrada de la cámara»: **~~`A-2` la reservó al fin de carrera~~ y el firmware la sigue leyendo como cámara — ~~CONFLICTO ABIERTO~~** *(derogado por `D-27` en la revisión de arriba: `J14` libre y sin cablear, el fin de carrera no se instala)*. Las cámaras son **cuatro, dos por poste, en `J16` p10 y p12**.  
**Revisión anterior:** 28 de Agosto de 2026 (segunda revisión del día) — 🔴 **corrige un error de cableado que puede DESTRUIR EL MICROCONTROLADOR** (`J14`/`J15`, §3 y §5), da por **muerta la ruta C** del bus (el Bluetooth ocupó `PB6`/`PB7` hoy mismo), añade la **ruta D**, avisa de que el `DS3231` **no tiene driver** y actualiza la cifra de flash. Ver las fes de erratas antes de usar cualquier copia impresa anterior.  

---

> [!CAUTION]
> ### 🩹 FE DE ERRATAS DEL 28/08/2026 (2.ª) — 🔴 **RIESGO ELÉCTRICO: LA TALANQUERA NO VA EN `J14`**
>
> **Si usted ya cableó un módulo de relé de talanquera a `J14` siguiendo una copia anterior de este
> manual o del Manual 15: DESCONÉCTELO ANTES DE ENERGIZAR.**
>
> Este manual afirmaba en §5 —y el `15_Lista_de_Compras_Hardware.md` en su fila **A4**— que la
> talanquera se conecta a la bornera **`J14`**. **Es falso, y es peligroso en las dos direcciones:**
>
> ```text
>   J14  = ENTRADA   U1 pin 18 (PB0) <── R64 10 kOhm + C25 100 nF <── J14
>                    CAM_DEMANDA_PIN. Contacto seco de la camara. Activa en alto, 3,3 V
>
>   J15  = SALIDA    U1 pin 19 (PB2) ──> R70/R69 ──> opto U15 (TLP127) ──> MOSFET Q10 ──> J15
>                    MOTOR_TALANQUERA. Canal de potencia propio, conmuta a masa
> ```
>
> ✏️ **11/09 — «Contacto seco de la cámara» en `J14` ya no vale.** Las cámaras van a `J16` (`D-2`,
> `D-3`; con `D-25`, **dos por poste, p10 y p12**) ~~y **`A-2` reservó `J14` al fin de carrera de la
> pluma**. 🔴 **CONFLICTO ABIERTO, que este manual no resuelve:**~~ → 🟢 **11/09, `D-27`: `J14` queda
> LIBRE, sin cablear — el fin de carrera NO se instala en este despliegue.** El firmware **sigue
> leyendo `PB0` como `CAM_DEMANDA_PIN`** (`pinMode(CAM_DEMANDA_PIN, INPUT)` en las dos puntas; el
> Maestro lo lee por nivel en el Modo Inteligente y el Esclavo por flanco, y lo manda por radio como
> demanda), así que lo que se cierre en `J14` **se lee como un coche pidiendo paso: en `J14` no se
> conecta nada.** Vacío, `R64` lo deja en 0 V y no pide nada (`J14` medido en banco el 03-04/09,
> pasos 17-18). **Lo que NO cambia de esta fe de erratas: `J14` es ENTRADA y la talanquera va en
> `J15`.**
>
> * **Lo que rompe:** un relé alimentado a 12 V cableado a `J14` **mete 12 V directamente en una
>   entrada del STM32** que solo tiene un RC de 1 ms (`R64` + `C25`) delante. `PB0` es un pin de
>   3,3 V. No hay opto ni diodo de protección en ese camino: lo que hay detrás es el micro que
>   gobierna el semáforo.
> * **Y además no habría funcionado:** `J14` no tiene ningún transistor de salida. Aunque
>   sobreviviera, **la pluma no se movería**, y el instalador buscaría el fallo en el relé o en el
>   actuador, que estarían bien.
> * **Medido** (28/08) en `01_Firmware/Maestro/include/pines.h`: `#define CAM_DEMANDA_PIN PB0`
>   (línea 46, `-> R64 10K + C25 100nF -> bornera J14`) y `#define MOTOR_TALANQUERA PB2`
>   (línea 31, `-> opto U15 -> MOSFET Q10 -> bornera J15`). Ver también `roadmap.md` N-64.
> * **El documento se contradecía consigo mismo:** la §3 de este mismo manual siempre dijo `J15`,
>   bien y con el diagrama correcto. Lo falso eran §5 y el Manual 15. **Cuando dos secciones del
>   mismo papel discrepan sobre dónde va un cable de potencia, no se elige la más cercana: se para
>   y se mide.**
> * **Las citas erróneas no se borran: se corrigen a la vista, con esta nota delante.** Quien cableó
>   siguiendo la versión vieja tiene que poder enterarse de que lo hizo mal.

---

> [!CAUTION]
> ### 🛑 ESTE MANUAL DESCRIBE UNA ARQUITECTURA QUE SE RETIRÓ EL 31/08/2026. NO SE EJECUTA
>
> **El bus I²C sobre el STM32 no se va a montar.** El `DS3231` vive desde el 31/08 en el **módulo de
> expansión ESP32** —`GPIO21` (`SDA`) / `GPIO22` (`SCL`), con pila propia—, y el `PCF8574` **queda
> retirado**: no porque fuera malo, sino porque el problema que resolvía desapareció. Ver
> `12_Cobertura_de_Pruebas_y_Huecos.md` §6.
>
> **Lo que de este manual SIGUE VIGENTE y hay que leer:** §3 y la fe de erratas `J14`/`J15` —riesgo
> eléctrico real y medido—, §4.1 (el censo de pines, la única medida red por red del cobre), el
> aviso de la pila `CR2032` contra `LIR2032` del `ZS-042`, y §7.0 (comprobaciones con el pito antes
> de energizar). **Lo que NO se ejecuta:** §1 (desoldar `C25`), la elección de ruta de §4.2 y §6, y
> la parte de §5 que compra para ese bus.

> ### 🛑 Y EL AVISO AL COMPRADOR QUE HABÍA AQUÍ ERA FALSO — se tacha con su motivo, 05/09
>
> ~~**Nada de lo que se compra en la §5 tiene driver en el firmware de hoy. Ni el expansor, ni el reloj.**~~
>
> | Pieza | ~~¿Hay driver?~~ **MEDIDO el 05/09** | Qué pasa al enchufarla hoy |
> |---|:---:|---|
> | **Expansor `PCF8574`** | ❌ **No existe** — y **da igual: la propuesta está retirada** | No se compra |
> | **Reloj `DS3231` `ZS-042`** | ~~❌ **No existe**~~ → 🟢 **SÍ EXISTE** | ~~🔴 No dará la hora… no hay código que le hable~~ 🛑 **FALSO.** Enchufado **al ESP32** hay software que lo lee |
>
> ~~**Medido el 28/08**, `grep -rniE "DS3231|Wire\.|0x68"` sobre `01_Firmware/Maestro/{src,include}` y
> `01_Firmware/Esclavo/{src,include}`: **cero coincidencias de código**.~~
>
> 🔴 **POR QUÉ ESTE MANUAL DIJO LO CONTRARIO, y es la lección que se queda: el grep miraba DOS de las
> TRES carpetas.** El driver vive en una tercera, `01_Firmware/ESP32_Expansion/`. Un cero de un
> buscador que no mira donde está la cosa se lee como *«no hay»* — es `CLAUDE.md` §4, y aquí sostuvo
> **un aviso al comprador en negrita**. El párrafo incluso se defendía diciendo *«el buscador sabe
> encontrar»*, probándolo con `SerialBT`… **en esas mismas dos carpetas**. Un control que se ejecuta
> dentro del alcance equivocado confirma el alcance, no el hallazgo.
>
> ```
> RE-MEDIDO el 05/09:
> grep -rni "ds3231" 01_Firmware --include=*.cpp --include=*.h | wc -l   ->  57
> wc -l 01_Firmware/ESP32_Expansion/src/reloj_ds3231.cpp                 ->  336 lineas
> grep -n "Wire.begin" .../reloj_ds3231.cpp   ->  :119 Wire.begin(DS3231_SDA, DS3231_SCL)
> llamadores: reloj_setup() <- main.cpp:161 · reloj_revisar() <- main.cpp:173
>             reloj_leer()  <- despachador.cpp:96,:120 · puente.cpp:222
> y el propio grep del manual, tal cual, YA NO DA CERO: Maestro/src/bluetooth.cpp:717
> ```
>
> 🔴 ~~**LO QUE SÍ SIGUE SIENDO CIERTO, y es lo que el comprador necesita: NO HAY `DS3231` COMPRADO**
> (línea `A6`) … **N-145 no se puede dar por probada.**~~
>
> # 🛑 07/09 — ESO CADUCÓ EN UNA HORA, Y ES EL EJEMPLO EXACTO DE POR QUÉ UNA FECHA NO VALIDA NADA
>
> **El párrafo de arriba se escribió el 05/09 a las 13:11** (`eaf6760`). **A las 14:12 del mismo
> día** el responsable resolvió `A-5` y la fila `A6` de la lista de compras se corrigió (`08c9d36`):
>
> > *«cada `ESP32` tiene un reloj y pila, ya te lo indiqué»* — **son DOS `DS3231`, uno por poste, y
> > están PUESTOS.** Estaba escrito desde el 28/08 en la propia lista de compras.
>
> **Consecuencias, y las tres invierten lo que decía el párrafo tachado:**
>
> | decía | hoy (`DECISIONES.md` `A-5`, `D-9`, `D-15`) |
> |---|---|
> | *«no hay `DS3231` comprado»* | ✅ **hay dos, uno por `ESP32`, con pila propia. `A6` sale de la lista de pedido** |
> | *«`N-145` no se puede dar por probada»* | 🟢 **`N-145` queda CONFIRMADA EN COBRE:** el `HORA:22:19:58` de la cinta del banco **es real** |
> | *«enchufado al STM32 sigue sin pasar nada»* | ✅ **sigue cierto: no hay driver de `DS3231` en el STM32.** ⚠️ **Pero «el reloj NO va al STM32» hay que leerlo con `D-20` (07/09) delante — ver la nota** |

> 🔵 **07/09 (`D-20`) — MATIZ QUE CAMBIA EL SENTIDO DE LA ÚLTIMA FILA.** *«El reloj no va al STM32»*
> se escribió para decir dos cosas a la vez, y sólo una sigue siendo cierta:
>
> | | |
> |---|---|
> | *«el `DS3231` no se cuelga de pines del STM32»* | ✅ **cierto, y no cambia.** No hay driver en esa punta, y este manual no monta ese bus |
> | ~~*«el STM32 no lleva hora, nunca»*~~ | 🔴 **derogado por `D-20`:** el STM32 **recibe la hora de SU PROPIO ESP32 y la retransmite por radio a la otra punta**. Es el **cartero** de la cadena `ESP32-M -> STM32-M -> radio -> STM32-E -> ESP32-E`, porque **los dos ESP32 no se hablan entre sí** |
>
> ⚠️ **SIN CONSTRUIR.** Lo que `D-20` decide es la **autoridad** de la hora, no la implementación:
> falta el mando ESP32 → STM32 que la siembre (`enlace_stm32.cpp` **no menciona `RTC` ni `hora`**,
> comprobado el 07/09). **Para las compras de este manual no cambia nada: siguen siendo DOS
> `DS3231`, uno por poste (`A-5`), y ninguno cuelga del STM32.**
>
> 🔴 **Lo único que de verdad sigue abierto: la dirección I²C `0x68` está `SIN VERIFICAR` sobre el
> módulo real** —es la del datasheet, y lo declara el propio fuente en
> `ESP32_Expansion/include/contrato.h`—.
>
> * Este documento es una **guía de taller para compras, preparación de hardware y modificaciones en
>   la PCB**, y **su bus I²C sobre el STM32 NO SE MONTA** (ver el recuadro de arriba).
> * ~~**No prometa hora al cliente por haber comprado el reloj.** El `DS3231` es una compra de
>   *preparación de hardware*; la funcionalidad llega con el firmware V9.0, no con el módulo.~~
>   ⛔ **CADUCADO EL 07/09: el firmware existe y está llamado** —`ESP32_Expansion/src/reloj_ds3231.cpp`,
>   con `reloj_setup()`, `reloj_revisar()`, `reloj_leer()` y `reloj_ajustar()` **todos con
>   llamador**—, y la hora **ya salió en una cinta de banco**. **Lo que sí hay que decirle al cliente
>   es lo contrario de lo que decía esta línea: la hora del cruce la lleva el `DS3231` del `ESP32`, y
>   el STM32 no tiene ninguno.**

---

> [!CAUTION]
> ### 🩹 FE DE ERRATAS DEL 28/08/2026 (1.ª) — LA FILA DE `PB8` DE LA SECCIÓN 4 ERA FALSA
>
> Este manual se emitió el **27/08/2026** afirmando que `PB8` ofrecía un **«pad libre / vía SCL»**.
> **No lo ofrece.** Trazado red por red sobre el esquemático bueno:
>
> ```text
>   U1 pin 45 (PB8) ──► R16 (1 kΩ) ──► [A] LED D5 [K] ──► GND
> ```
>
> Es el **LED testigo de la placa**, no un pad libre.
>
> * El dato **ya estaba medido ese mismo día** en `03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md`
>   (`N-63`) y en `01_Firmware/Maestro/include/pines.h` (`#define LED_TESTIGO PB8`). Este manual
>   salió sin recogerlo, y la causa es la que se explica en §4: **apuntaba al plano equivocado**.
> * **Por qué no es una errata de redacción.** Quien siga la tabla original suelda `SCL` encima de
>   un LED con 1 kΩ a masa. Esa es una carga permanente que **se come el nivel alto del bus**: el
>   síntoma sería un I²C que no comunica jamás con toda la soldadura correcta, y el instalador
>   buscaría el fallo en la placa hija, que estaría bien.
> * **La fila no se borra: se corrige a la vista** (§4, con su fecha). Una corrección que
>   desaparece en silencio la vuelve a escribir el siguiente que pase por aquí.
> * Y con esa fila cayó también su premisa: **`PB0`/`PB8` no eran «los únicos pines libres»**. Ver
>   el censo de §4.1 y las rutas de §4.2 — donde, además, la 2.ª revisión del 28/08 cerró la ruta C.

> [!WARNING]
> ### 📐 QUÉ ESTÁ MEDIDO Y QUÉ NO — LÉASE ANTES DE COMPRAR O DE CALENTAR EL CAUTÍN
>
> | | |
> |:---|:---|
> | ✅ **MEDIDO SOBRE EL ESQUEMÁTICO Y EL TRAZADO** (28/08/2026) | Las redes de §4.1 y §4.2, salidas de recorrer `Controladora_Semaforos.kicad_sch` y `.kicad_pcb` red por red y pad por pad, no de leer la leyenda del plano. |
> | ✅ **MEDIDO SOBRE EL FIRMWARE** (28/08/2026) | Que `PB6`/`PB7` están **tomados por el Bluetooth** (`SerialBT(PB7, PB6)` en los dos `bluetooth.cpp`), y que `PB0`→`J14` es entrada y `PB2`→`J15` es salida (`pines.h`). Son medidas sobre el código, no sobre el cobre. ~~y que **no hay driver de `DS3231`** (§0)~~ 🔴 **TACHADO EL 07/09: ESO ES FALSO, Y LO DICE EL PROPIO §0 DE ESTE MANUAL DESDE EL 05/09.** El driver existe —`ESP32_Expansion/src/reloj_ds3231.cpp`— y tiene llamadores. **La medida del 28/08 era cierta el 28/08 y miraba sólo dos de las tres carpetas de firmware**; esta fila la heredó sin volver a correrla, que es exactamente lo que §0 explica que no se debe hacer. |
> | ⛔ **NO VERIFICADO EN LA PLACA** | **Nada de este manual se ha comprobado con multímetro sobre el cobre.** Un esquemático dice lo que se dibujó; una placa dice lo que se fabricó y lo que alguien reparó después. Antes de soldar, la §7 manda comprobar con el pito lo que aquí sale como «medido». **Tampoco está verificado en banco el enlace Bluetooth sobre `J17` p2/p3**: la compuerta pasó, y la compuerta no toca la tarjeta. |
> | ❓ **ABIERTO — pero ya no bloquea la elección de ruta** | El **papel real de `PB6`**. El firmware lo llama `LCD_PSB`; la etiqueta de red del esquemático lo llama `RS(A0)`. **Los dos nombres no pueden ser ciertos a la vez.** Antes esto decidía si existía la ruta C; **hoy la ruta C está muerta por otro motivo (§4.2)**, así que la pregunta ya no gobierna el bus — pero **ha cambiado de dueño**: ahora `PB6` lleva el `TX` del Bluetooth, y si resultara ser de verdad el `RS/A0` del display, es la **pantalla** la que queda en riesgo. Se resuelve igual: siguiendo el hilo hasta la pata rotulada del módulo, y encendiendo el equipo a ver si dibuja. |

---

## 1. ⚠️ MODIFICACIÓN OBLIGATORIA EN LA PLACA MADRE: DESOLDAR `C25` (100 nF)

> [!NOTE]
> Esta sección **solo aplica si se elige la ruta A** de §4.2 (bus sobre `PB0`). **Las rutas B y D
> —las dos que siguen vivas— no tocan `PB0`, y por tanto NO necesitan desoldar `C25`.** Como la
> ruta A no se ejecuta, en la práctica **esta sección entera no aplica hoy**; se conserva porque el
> razonamiento está medido y es la razón por la que `PB0` es la peor opción.
>
> Y hay un motivo más, que no es de I²C y pesa más: **`C25` es el antirrebote de la entrada de la
> cámara de demanda** (`J14`). Desoldarlo no solo «libera un pin»: deja la entrada sin filtro.
> *(11/09: en `J14` ya no va cámara —`J16`, `D-25`— y con `D-27` queda **libre, sin cablear**; el
> filtro sigue siendo el de esa entrada, que el firmware sigue leyendo.)*

En el diseño original de KiCad, el pin `PB0` (Pin 18 de `U1`) fue previsto como entrada analógica/optoacoplada, por lo que incluye un **condensador de filtro `C25` (100 nF) conectado a masa**.

```text
               DISEÑO ORIGINAL KICAD                           MODIFICACIÓN PARA BUS I²C
          ┌─────────────────────────────┐                  ┌─────────────────────────────┐
          │  Pin 18 (PB0)               │                  │  Pin 18 (PB0)               │
          │      │                      │                  │      │                      │
          │     ─┴─  C25 (100 nF)       │  ───► DESOLDAR   │      ├──► Línea SDA (I²C)   │
          │     ─┬─  (Mata el I²C)      │       C25        │     [R] Pull-up 4.7 kΩ      │
          │      │                      │                  │      │                      │
          │     GND                     │                  │    +3.3V (de U5)            │
          └─────────────────────────────┘                  └─────────────────────────────┘
```

> [!WARNING]
> ### 🔴 POR QUÉ ES OBLIGATORIO RETIRAR `C25`
> Con una resistencia de pull-up de $4.7\text{ k}\Omega$ y `C25` ($100\text{ nF}$), el tiempo de subida de la señal es:
> $$\tau = R \times C = 4.7\text{ k}\Omega \times 100\text{ nF} = 470\ \mu\text{s}$$
> La norma I²C exige flancos de subida menores a **$1\ \mu\text{s}$**. Con `C25` puesto, la señal digital queda totalmente aplanada y el bus I²C **no puede comunicar jamás**.
> **Acción:** Desoldar o levantar una pata del condensador `C25` antes de cablear el bus.

---

## 2. ⚡ NATURALEZA DEL BUS I²C: **BIT-BANG, Y YA NO ES UNA ELECCIÓN**

**Al 28/08 el bus I²C de este proyecto tiene que ser bit-bang, vaya por donde vaya.** No porque sea
una propiedad del proyecto, sino porque **el único par de pines con I²C por hardware (`PB6`/`PB7`)
se lo llevó el Bluetooth hoy mismo** (§4.2). Todas las rutas que quedan vivas —B y D— son pines sin
función alternativa de I²C:

* **`PB0` (Pin 18):** No posee función alternativa de I²C por hardware (es `ADC12_IN8` / `TIM3_CH3`). ~~En V8.0–V8.9 está asignado a `CAM_DEMANDA_PIN`~~ *(11/09, medido: en el `V8.4` de campo, `e303485`, **`PB0` no tiene uso** —`git grep CAM_DEMANDA_PIN e303485 -- 01_Firmware/Maestro 01_Firmware/Esclavo` → 0—; es `CAM_DEMANDA_PIN` en el V9 de hoy, ~~y `A-2` lo reservó al fin de carrera: **conflicto abierto**, ver la fe de erratas~~ → **y con `D-27` (11/09) `J14` queda LIBRE, sin cablear: el fin de carrera no se instala en este despliegue**, ver la fe de erratas)*, y **su bornera `J14` es una ENTRADA** (ver la fe de erratas de la cabecera).
* **`PB8` (Pin 45):** Para ser `I2C1_SCL` requeriría un remapeo completo que movería `SDA` a `PB9`, pero `PB9` está físicamente ocupado por `BOTON1` en la placa. Además es el LED testigo (§4.1).
* **`PA11`/`PA12`, `PB3`/`PB4`:** GPIO corrientes. Bit-bang.

> [!CAUTION]
> ### 🔴 ESTA SECCIÓN SE HA EQUIVOCADO DOS VECES, EN DIRECCIONES OPUESTAS — SE DEJAN LAS DOS
> * **27/08:** saltaba de *«en `PB0`/`PB8` el I²C es bit-bang»* a *«el bus **es** bit-bang»*. Ese
>   salto solo valía si `PB0`/`PB8` eran los únicos sitios posibles, y **no lo eran** (§4.1).
> * **28/08 (mañana):** se corrigió a *«el bit-bang es el precio de una ruta; hay otra —la C— en la
>   que el bus sería I²C1 por hardware»*. **Eso fue cierto durante unas horas.** Esa misma tarde el
>   Bluetooth ocupó `PB6`/`PB7` y la ruta C dejó de existir.
>
> **La conclusión del 27/08 vuelve a ser la buena, pero por un motivo distinto del que se dio
> entonces**, y eso importa: si mañana el Bluetooth se muda o se retira, `PB6`/`PB7` vuelven a estar
> disponibles y la ruta C **resucita**. Lo que la mata es un pin ocupado, no una imposibilidad.

* **Regla Anti-Bloqueo (vale para todas las rutas):** Todas las lecturas y escrituras I²C deben contar con **timeouts no bloqueantes** (< 2 ms) para garantizar que la ausencia o desconexión del módulo no cuelgue el bucle principal ni dispare el perro guardián `IWDG`. Un bus colgado dentro del bucle de luces es peor que no tener reloj.
* **Y la talanquera no se manda desde el bus.** Es una salida vial: va **detrás** de la barrera de `semaforo.cpp`, nunca dentro de `escribirPines()`, que hoy son seis `digitalWrite` que no pueden bloquearse. Una escritura I²C sí puede.

### 2.1 Presupuesto de Flash — la cifra de este manual estaba vencida

| | dice el manual del 27/08 | dijo la 1.ª revisión del 28/08 | ~~**manda el acta** `evidencia/2026-08-28_compuerta.txt`~~ |
|:---|:---:|:---:|:---:|
| Flash libre en el Maestro | ~4.728 B | 9.452 B (85,6 %) | ~~**9.276 B libres** — `85.8% (used 56260 bytes from 65536 bytes)`~~ |

> 🔴 **07/09 — ESA CIFRA CADUCÓ, Y AQUÍ NO SE ESCRIBE OTRA.** El acta del 28/08 sigue existiendo y
> su número era correcto **para aquel binario**; el Maestro ha crecido desde entonces. **La cifra
> vigente se lee del acta más reciente de `evidencia/`** —`ls -t evidencia/*_compuerta.txt | head -1`—,
> que la escribe `compuerta.py` con su fecha y el hash de `HEAD` encima. ⚠️ **Y se confirma con una
> segunda pasada:** el acta puede publicar el binario **anterior** cuando PlatformIO sirve un
> incremental viejo, y eso pasa siempre que dos agentes tocan el árbol a la vez (`CLAUDE.md` §10).
> **Lo único que este apartado necesita saber sigue siendo cierto: el bit-bang cabe** —presupuesto
> < 800 B de código propio— **y el bus no se monta**, así que no se va a gastar.

**Las cifras se copian del acta, nunca se escriben a mano.** Los 9.452 B eran una cifra correcta de
un binario anterior; el Maestro ha crecido desde entonces (entre otras cosas, con el Bluetooth de
`PB6`/`PB7`). Al escribir aquí un número de flash se pega la línea del acta, con su fecha y su hash
de `HEAD` (`614065d`), para que se vea contra qué binario se midió.

Los 4.728 B del 27/08 eran ciertos antes de `N-70`, que quitó **5.160 B de I²C y SPI por hardware
enlazados en un equipo sin un solo bus I²C**: `U8x8lib.cpp` referencia `TwoWire::setClock()` y el
enlazador arrastraba `Wire` entero. Se resolvió declarando `U8X8_NO_HW_I2C` en `platformio.ini`.

> [!IMPORTANT]
> ### La ruta ya no se elige por flash — y ahora tampoco se puede
> La 1.ª revisión del 28/08 comparaba aquí el coste de `Wire` (~1.080 B más su HAL) contra el del
> bit-bang (< 800 B de código propio) para decidir entre la ruta C y las demás. **Esa comparación
> ya no tiene objeto: la ruta C está muerta y no queda ninguna ruta por hardware** (§2, §4.2).
>
> * **Lo único que se va a gastar es el bit-bang:** presupuesto < 800 B de código propio, cero
>   librería nueva. ~~Con **9.276 B libres** cabe con holgura.~~ **Los bytes libres se leen del acta
>   más reciente de `evidencia/`, no de aquí** (07/09) — y **hoy no se gasta nada: el bus no se
>   monta.**
> * **Y hay un aviso que sigue vivo:** si algún día se vuelve a `Wire`, se re-enlaza justo lo que
>   `N-70` quitó. La bandera `U8X8_NO_HW_I2C` de `platformio.ini` **no se retira sin volver a medir**.

---

## 3. 🚦 ETAPA DE POTENCIA PARA TALANQUERA: EL MOSFET `Q10` YA EXISTE EN `J15`

No es necesario fabricar un circuito de potencia externo con relés para la talanquera. **La tarjeta madre KiCad ya incluye un MOSFET de potencia dedicado**:

```text
 ┌──────────────────────────────────────────────────────────────────────────────────────────────────┐
 │                               ETAPA DE POTENCIA NATIVA DE LA PLACA                               │
 ├───────────────────┬──────────────────────────────────────────────────────────────────────────────┤
 │ Componente KiCad  │ Transistor MOSFET `Q10` (`IRLZ44N`, Canal N, 55V / 47A)                      │
 │ Bornera de Salida │ Bornera `J15` (Salida de Potencia 12V / GND Conmutado)                       │
 │ Control Lógico    │ Conmuta directamente a masa (`GND`) cargas de hasta 10A sin calentamiento    │
 └───────────────────┴──────────────────────────────────────────────────────────────────────────────┘
```

```text
       TARJETA MADRE KICAD (BORNERA J15)                     CENTRALITA DE LA TALANQUERA
  ┌────────────────────────────────────────┐               ┌───────────────────────────────┐
  │ Borne J15 (+) ──► +12V Batería         │               │                               │
  │ Borne J15 (−) ──► Drenador MOSFET Q10  ├───────────────┼─► Entrada OPEN (Activa en LOW)│
  │                   (Conmuta a Masa)     │               │   o Bobina de Relé Auxiliar   │
  └────────────────────────────────────────┘               └───────────────────────────────┘
```

> ⛔ **11/09 — DE ESTE DIAGRAMA, `D-25` DEJA SÓLO LA RAMA DEL RELÉ. La otra —el drenador de `Q10`
> directo a la entrada `OPEN` de la centralita— ~~«Entrada OPEN (Activa en LOW)»~~ NO es la
> conexión decidida.** Lo decidido por el responsable (`DECISIONES.md` `D-25`) es:
>
> ```text
>   J15 p1 (12 V) ─────────────┐
>                              [ BOBINA del relé, 12 V DC ]
>   J15 p2 (drenador Q10) ─────┘        (D30 de la placa hace de rueda libre)
>
>   CONTACTO del relé (NO + COM) ─────────> entrada OPEN de la centralita (y su comun)
> ```
>
> 🔴 **`J15` p2 NO ES MASA**, aunque el diagrama de arriba lo rotule «(−)» y «Conmuta a Masa»: con
> `Q10` conduciendo baja a masa; **con `Q10` abierto está a ~12 V** (a través de la bobina). No se
> usa como retorno de nada. Y el *«hasta 10A sin calentamiento»* del primer cuadro **no tiene medida
> detrás** —y la rueda libre `D30` es un `1N4148` (bloque `E2` del Manual 15)—: con `D-25` la carga
> de `J15` es **la bobina de un relé**, no un motor.
>
> ⚠️ **Qué hace la centralita cuando el relé suelta `OPEN` —si la pluma baja sola, con qué retardo,
> y qué hace sin energía— lo decide LA CENTRALITA, no esta tarjeta: `SIN VERIFICAR`.** Se comprueba
> con la barrera real al montar.
>
> 🛒 **11/09, `D-27` punto 4 — QUÉ RELÉ, porque la lista de compras se ajusta a este cableado** (el de
> la guía del Sisga, `05_Funcional/Camaras_Sisga_4x.html`, apartado *«Talanquera en J15»*):
>
> | | |
> |---|---|
> | **Pieza** | **un relé con BOBINA de 12 V DC**, uno por poste, con **un contacto normalmente abierto** (NA + común) |
> | **Bobina** | entre `J15` **p1** (12 V) y **p2** (drenador de `Q10`) — dos hilos, sin masa ni 3,3 V: `J15` no los tiene |
> | **Contacto** | NA → entrada **`OPEN`** de la centralita; común → **su común**, como diga el manual de esa centralita |
> | **Rueda libre** | **ya está en la placa**: `D30`, un `1N4148` de 200 mA entre `/12V` y p2. No se pone otro diodo. ⚠️ **La corriente de la bobina tiene que quedar por debajo de esos 200 mA**; qué bobina se elige y cuánto consume: `SIN VERIFICAR` — **la referencia la decide el responsable** |
> | **Lo que NO se compra** | ~~un **módulo de 1 relé optoacoplado con jumper `JD-VCC`**~~ — pide alimentación de lógica y masa propias, que `J15` no da; y el porqué del `JD-VCC` era el del expansor `PCF8574`, retirado |
>
> **El régimen del contacto frente a la entrada `OPEN`** depende de la centralita, que no está
> especificada en ningún documento de este proyecto: `SIN VERIFICAR`.

* **Ventaja:** ~~Cero componentes extra.~~ *(11/09: con `D-25` hace falta **un relé por poste** —
  línea `A4` del Manual 15—; `D-27`: **de bobina de 12 V DC, contacto NA**, tabla de arriba.)* La salida de potencia para la talanquera ~~o electroimán de retención~~ se toma directamente de la bornera **`J15`** de la placa madre.

> [!CAUTION]
> ### 🔴 `J15` SÍ — `J14` NO. NO SON INTERCAMBIABLES NI POR ERROR
> | | `J14` | `J15` |
> |---|---|---|
> | Sentido | **ENTRADA** | **SALIDA** |
> | Red del esquemático | `Puerta` | `Motor` |
> | Pin del STM32 | `PB0` (`CAM_DEMANDA_PIN`) | `PB2` (`MOTOR_TALANQUERA`) |
> | Qué hay en medio | `R64` 10 kΩ + `C25` 100 nF (antirrebote de 1 ms) | opto `U15` (TLP127) + MOSFET `Q10` (`IRLZ44N`) |
> | Qué se le conecta | ~~El **contacto seco de la cámara** de demanda, a 3,3 V~~ → *(11/09)* ~~**reservada al fin de carrera** (`A-2`); 🔴 **conflicto abierto**~~ → 🟢 **NADA: LIBRE, sin cablear (`D-27`, 11/09)** — el fin de carrera no se instala en este despliegue, y el firmware la lee como cámara (`CAM_DEMANDA_PIN`), así que lo que se cierre ahí pide paso. Las cámaras van a `J16` p10/p12 (`D-25`) | ~~La **centralita de la talanquera** o~~ la **bobina del relé**, a 12 V — *(11/09, `D-25`: bobina entre p1 y p2; el contacto del relé a `OPEN` de la centralita)* |
> | Si se equivoca | 🔴 **12 V en un pin de 3,3 V del micro. Se destruye el STM32** | La cámara no se lee |
>
> **Este manual y el `15_Lista_de_Compras_Hardware.md` decían `J14` en la §5 y en la fila A4.
> Corregido el 28/08 (2.ª revisión).** La confusión tiene una raíz que conviene conocer para no
> repetirla: la red de `J14` se llama **`Puerta`** en el esquemático, y «puerta» suena a talanquera.
> **No lo es: es ~~la entrada por donde la cámara pide paso~~ una entrada** *(11/09: ~~reservada al fin
> de carrera por `A-2`, y leída por el firmware como cámara — conflicto abierto~~ → leída por el
> firmware como cámara, y **libre, sin cablear** por `D-27`: el fin de carrera no se instala en este
> despliegue)*. El canal de la pluma se llama `Motor`.
>
> El nivel de reposo de `PB2` es `LOW` (`TALANQUERA_CERRAR`, SFTY-28): con el MOSFET sin conducir,
> ~~el motor queda sin energía y **la pluma se queda abajo**. Ese es el fallo seguro, y por eso la
> especificación de compra exige actuador **con retorno por muelle o gravedad** — eso el software
> no lo puede garantizar.~~
>
> ✏️ **11/09 — lo tachado ya no se sostiene, por dos motivos medidos:**
>
> 1. **Con `D-25` en `J15` no hay motor: hay la bobina de un relé que cierra `OPEN` en la centralita.**
>    Con el MOSFET sin conducir el relé **suelta `OPEN`**; que la pluma baje entonces lo decide la
>    centralita (`SIN VERIFICAR`, arriba). Y la «especificación de compra con muelle o gravedad» **no
>    está en `15_Lista_de_Compras_Hardware.md`** —se buscó—: **sin fuente**.
> 2. **`PB2` no está en `LOW` sólo en reposo.** `escribirPines()` (`semaforo.cpp`, las dos puntas)
>    lo sube con `(verde && !testLedsActivo) || estado == S_FALLO`: **con verde Y con el ámbar
>    intermitente** —Modo Ámbar, radio perdida, y un poste recién encendido que todavía no enlazó
>    con el otro—. **Ninguna cámara entra en esa condición**: la pluma baja aunque haya un coche
>    debajo; el veto es `A-1.bis`, **sin construir**. `LOW` es lo que queda **sin energía** o con
>    cualquier otro estado.

---

## 4. 🎯 MAPEO FÍSICO PIN A PIN EN LA TARJETA KICAD

> [!CAUTION]
> ### 🗺️ EL PLANO QUE CITABA ESTE MANUAL NO ES EL BUENO — Y ESA ES LA CAUSA DE LA ERRATA
> La versión del 27/08 daba como fuente `03_Hardware_Tarjeta/KiCad/Controladora_Semaforos.kicad_sch`.
> **Ese fichero ya no existe:** era el plano **incompleto** (451 KB, sin LCD, sin botones, sin el
> canal del motor) y lo borró el commit `8668498` — *«PB8 no es una camara, es un LED - y se borra
> el plano incompleto»*— **el mismo día en que se emitió este manual**.
>
> **El plano bueno, y el único que se debe citar, es:**
> ```
> 01_Firmware/Controladora_Semaforos/Controladora_Semaforos/Controladora_Semaforos.kicad_sch   (649 KB)
> 01_Firmware/Controladora_Semaforos/Controladora_Semaforos/Controladora_Semaforos.kicad_pcb   (2,1 MB)
> ```
> Es la regla del instrumento en su forma más cara: **la medida no estaba mal tomada, estaba tomada
> sobre el objeto equivocado**. Todo lo que sigue está medido sobre estos dos ficheros.

### 4.1 Censo de pines: qué está ocupado y qué está realmente libre

Medido recorriendo las redes del esquemático y contando **pistas y vías** de cada pad en el trazado.
No es una lectura de la leyenda del plano:

```text
 ┌──────────────┬──────────────────────────┬──────────────────────────────────────────────────────┐
 │ Pin STM32 U1 │ Red del esquemático      │ A dónde va, medido                                   │
 ├──────────────┼──────────────────────────┼──────────────────────────────────────────────────────┤
 │ Pin 18  PB0  │ "Puerta"                 │ R64 10 kΩ + C25 100 nF -> bornera J14. ENTRADA con   │
 │              │                          │ antirrebote de ~1 ms. Es CAM_DEMANDA_PIN             │
 │ Pin 45  PB8  │ "PB8"                    │ R16 1 kΩ -> LED D5 (ánodo a R16, cátodo a GND).      │
 │              │                          │ 🔴 LED TESTIGO. NO ES UN PAD LIBRE                   │
 │ Pin 39  PB3  │ "SCL"  (clock del LCD)   │ J17 pin 4  -- dato de la pantalla. Libre SI se va   │
 │              │                          │ SIN PANTALLA -> ruta D                               │
 │ Pin 40  PB4  │ "CS"                     │ J17 pin 1  -- dato de la pantalla. Libre SI se va   │
 │              │                          │ SIN PANTALLA -> ruta D                               │
 │ Pin 41  PB5  │ "SI"                     │ J17 pin 5  -- dato de la pantalla. Libre SI se va   │
 │              │                          │ SIN PANTALLA -> ruta D (sobra: el bus usa dos)      │
 │ Pin 42  PB6  │ "RS(A0)"                 │ J17 pin 3  -- 🔴 TOMADO 28/08: USART1 TX del modulo  │
 │              │                          │ Bluetooth SPP. SI transporta datos, y a 9600 bps    │
 │ Pin 43  PB7  │ "RST"                    │ J17 pin 2  -- 🔴 TOMADO 28/08: USART1 RX del modulo  │
 │              │                          │ Bluetooth SPP. SI transporta datos, y a 9600 bps    │
 │ Pin 34  PA13 │ (sin etiqueta)           │ J2 pin 3   -- SWDIO, programación                    │
 │ Pin 37  PA14 │ (sin etiqueta)           │ J2 pin 2   -- SWCLK, programación                    │
 ├──────────────┼──────────────────────────┼──────────────────────────────────────────────────────┤
 │ Pin 32  PA11 │ sin red                  │ 🟢 SIN UNA SOLA CONEXIÓN, y CERO pistas/vías         │
 │ Pin 33  PA12 │ sin red                  │ 🟢 SIN UNA SOLA CONEXIÓN, y CERO pistas/vías         │
 │ Pin 38  PA15 │ sin red                  │ 🟢 SIN UNA SOLA CONEXIÓN, y CERO pistas/vías         │
 │ Pin  2  PC13 │ sin red                  │ 🟡 libre, pero NO SIRVE PARA UN BUS (ver abajo)      │
 └──────────────┴──────────────────────────┴──────────────────────────────────────────────────────┘
```

**De donde se sigue que la premisa de este manual era falsa:** `PB0` y `PB8` **no** eran «los únicos
pines libres». Hay cuatro pads que el trazado deja sin una sola pista, y `PB8` —que este manual
vendía como libre— es justo el que **no** lo está.

> [!WARNING]
> **`PC13` está libre y aun así no se usa.** El manual del fabricante le da ~3 mA de capacidad y una
> velocidad de conmutación baja: sirve para un LED, no para una línea de bus con pull-up. Se deja en
> la tabla **precisamente para que nadie lo proponga otra vez** creyendo que se pasó por alto.
>
> **`PA11` y `PA12` son `USB_DM` / `USB_DP` en el `STM32F103`.** En este diseño **no hay USB**, y
> están medidos con cero pistas y cero vías, así que en el trazado son pads sueltos. Aun así, la §7
> manda comprobarlos con el pito antes de soldar: lo medido es el fichero de diseño, no el cobre que
> salió de fábrica.

### 4.2 Las cuatro rutas para el bus, y cuál manda

| | Ruta | Qué hay que modificar | Qué cuesta | Tipo de bus | Veredicto |
|:---:|---|---|---|:---:|---|
| **A** | `PB0` (SDA) + `PB8` (SCL) *— la del manual original* | **Desoldar `C25`** · **Retirar `R16` o `D5`** | La cámara de demanda **y** el LED testigo | Bit-bang | 🔴 **La peor.** Dos desoldaduras sobre una placa acabada, y sacrifica dos funciones que sí existen |
| **B** | `PA11` (SDA) + `PA12` (SCL) | **Nada que desoldar.** Dos hilos a dos pads sin pista | **Nada** | Bit-bang | 🟢 **MANDA ESTA.** Es la única que no le quita nada a nadie |
| **C** | ~~`PB6` (`I2C1_SCL`) + `PB7` (`I2C1_SDA`)~~ | — | — | ~~Por hardware~~ | ⚫ **MUERTA desde el 28/08.** Los pines se los llevó el Bluetooth. Ver abajo |
| **D** | `PB4` (SDA) + `PB3` (SCL), por `J17` | **Nada.** Cuatro hilos al conector `J17`: p1, p4, p8, p9 | **La pantalla entera** | Bit-bang | 🟡 **La más barata en taller, y la única con cero soldadura — pero solo en un equipo que se monte SIN LCD** |

> **Cómo se lee esta tabla:** las cuatro rutas son bit-bang (§2), así que **la elección no es
> técnica, es de qué función se sacrifica.** Ordenadas por lo que cuestan: **B** no cuesta nada,
> **D** cuesta la pantalla, **A** cuesta la cámara y el testigo, **C** ya no se puede pagar.

#### 🔴 Ruta C: **CERRADA el 28/08/2026.** El Bluetooth se llevó `PB6` y `PB7`

> [!CAUTION]
> ### ⚫ NO ES «BLOQUEADA POR COMPROBACIONES PENDIENTES». ES **MUERTA**. NO SE PUENTEE `J17`
>
> La versión anterior de esta sección decía que la ruta C estaba *bloqueada por tres comprobaciones
> pendientes* y que *«hoy `USART1` vive sin remapear en `PA9`/`PA10`, así que el remapeo está
> libre»*. **Desde el 28/08 es exactamente al revés, y lo pendiente ya no lo es: se decidió.**
>
> **Medido en el firmware (28/08):**
> ```text
>   01_Firmware/Maestro/src/bluetooth.cpp:25   static HardwareSerial SerialBT(PB7, PB6);
>   01_Firmware/Esclavo/src/bluetooth.cpp:26   static HardwareSerial SerialBT(PB7, PB6);
>                                                     USART1 REMAPEADO: PB7 = RX, PB6 = TX, 9600 bps
> ```
> * El **remapeo de `USART1` está USADO**, no libre. La tercera comprobación de la lista anterior
>   —*«`PB6`/`PB7` no pueden ser `USART1` e `I²C1` a la vez»*— **ya tiene respuesta: son `USART1`.**
> * El **módulo Bluetooth SPP ya está cableado a `J17` p2/p3** en las dos placas. Esas dos vías del
>   conector, que la ruta C mandaba puentear a `GND` y a `3,3 V`, **ahora llevan la telemetría**.
>   Puentearlas hoy es **cortocircuitar la salida `TX` del micro contra masa y la del módulo
>   Bluetooth contra `3,3 V`**, y quedarse sin consola de servicio.
> * Las otras dos comprobaciones —el papel real de `PB6` y el strap de `PSB` en el dorso del
>   módulo— **ya no deciden nada sobre el bus**, porque no hay ruta C que decidir. Siguen vivas,
>   pero han cambiado de expediente: ahora son riesgos del **enlace Bluetooth**, no del I²C (ver el
>   bloque «qué está medido» de la cabecera y §7.0).
>
> **Por qué la premisa de la ruta C se cayó sola:** la ruta C existía porque `PB6`/`PB7` *no
> transportaban datos del display* —`PB6` era un nivel estático y `PB7` el reset—. Eso era cierto y
> sigue siéndolo; **por eso mismo se los llevó el Bluetooth.** `lcd.cpp` ya construye U8g2 solo con
> `PB3`/`PB4`/`PB5` y `U8X8_PIN_NONE` de reset, y su línea 59 dice literalmente *«AQUI YA NO SE TOCA
> LCD_PSB (PB6), y el pin queda libre para el USART1»*. **La ruta C y el Bluetooth querían los
> mismos dos pines por la misma razón, y llegó antes el Bluetooth.**
>
> **Si algún día el Bluetooth se muda o se retira, esta ruta resucita** — con sus tres
> comprobaciones intactas. Por eso la sección se cierra en vez de borrarse.

#### 🟡 Ruta D: sin pantalla, el bus sale por `J17` sin tocar el soldador

**Esta ruta no estaba en el manual, y en un montaje sin LCD es la más barata de las cuatro.**

Si un equipo se monta **sin pantalla** —la LCD es interfaz de servicio, no de operación vial: el
ciclo, el Modo Degradado y la telemetría no dependen de ella—, se liberan de golpe `PB3`, `PB4` y
`PB5`, **y los tres ya salen al conector `J17`**. No hay que soldar en la placa, ni retirar
componentes, ni buscar pads sueltos: se conecta la placa hija al propio conector de la pantalla.

```text
   RUTA D -- TODO SALE DE J17, CON LA PANTALLA DESCONECTADA

     J17 pin 1  (PB4, red "CS")   ──►  SDA
     J17 pin 4  (PB3, red "SCL")  ──►  SCL
     J17 pin 8  (+3,3 V)          ──►  VCC de la placa hija
     J17 pin 9  (GND)             ──►  GND de la placa hija

     J17 pin 5  (PB5, red "SI")   ──►  sin usar: el bus solo necesita dos lineas
     J17 pin 2  (PB7) y pin 3 (PB6) ──► 🔴 NO TOCAR: son el Bluetooth (ruta C muerta)
```

* **Los pull-up de 4,7 kΩ siguen haciendo falta**, igual que en las demás rutas: `J17` da la
  alimentación, no el bus.
* **`PB3` y `PB4` son `JTDO` y `NJTRST` en el `STM32F103`**, y por defecto los toma el JTAG. Aquí no
  es un problema pendiente: **el firmware de hoy ya los usa como GPIO corrientes** para hablarle al
  ST7920, así que la liberación del JTAG ya está hecha y la programación por `SWD` (`PA13`/`PA14`)
  no se toca. *(Medido en el fuente; **no** verificado con el analizador sobre la placa.)*
* **Lo que cuesta, dicho sin adornos:** se pierde el menú, la pantalla `AJUSTAR HORA` y todo el
  diagnóstico local. Quien vaya a la calle solo tendrá el Bluetooth y los LED.

> [!IMPORTANT]
> ### 🎯 CUÁL MANDA — ⚠️ **y hoy la pregunta ya no se hace: el bus NO SE MONTA** (recuadro del principio)
> **Manda la ruta B (`PA11`/`PA12`).** ~~En el equipo de hoy —que lleva pantalla en las dos puntas—~~
> 🔴 **CADUCADO EL 07/09 (`D-17.bis`, 05/09): LA PANTALLA Y EL MENÚ SE RETIRAN DEL EQUIPO.** *(Del
> equipo, no del código: `lcd.cpp` compila y `Validacion_LCD` sigue dando `271/271` sobre un
> framebuffer en el PC.)* **Esa decisión, que este apartado dejaba abierta al «responsable de obra»,
> YA ESTÁ TOMADA** — así que, si el bus llegara a montarse, **la condición de la ruta D se cumple**.
> Se deja escrito y no se reordena la tabla porque **la elección de ruta no se ejecuta**: el bus I²C
> sobre el STM32 está retirado desde el 31/08.
>
> ~~**La ruta D manda —y con ventaja— solo si el montaje se decide explícitamente SIN LCD.** Entonces
> es cero soldadura, cero pads que buscar y cero componentes que retirar. **Esa decisión es del
> responsable de obra y se escribe antes de comprar**, no se descubre en el poste: no se desmonta
> una pantalla que ya está puesta para ahorrarse dos hilos.~~ → **la decisión está escrita:
> `DECISIONES.md` `D-17.bis`.**
>
> **La ruta A no se ejecuta.** **La ruta C no se puede ejecutar.**

Lo que sigue es el mapa del conector `J17`, que vale para las rutas C (histórica) y D — `J17` trae
las dos alimentaciones a mano, medidas:

```text
                    CONECTOR J17 (13 vías) — MEDIDO EN EL ESQUEMÁTICO (28/08, 2.ª rev.)
 ┌──────┬──────────────┬───────────────────────────────────────────────────────────────────┐
 │ Vía  │ Red          │                                                                   │
 ├──────┼──────────────┼───────────────────────────────────────────────────────────────────┤
 │  1   │ CS   (PB4)   │  dato del display -- RUTA D: aqui va el SDA (sin pantalla)        │
 │  2   │ RST  (PB7)   │  🔴 BLUETOOTH: USART1 RX. NO PUENTEAR (la ruta C mandaba a 3,3 V) │
 │  3   │ RS(A0)(PB6)  │  🔴 BLUETOOTH: USART1 TX. NO PUENTEAR (la ruta C mandaba a GND)   │
 │  4   │ SCL  (PB3)   │  dato del display -- RUTA D: aqui va el SCL (sin pantalla)        │
 │  5   │ SI   (PB5)   │  dato del display -- RUTA D: queda libre y sin usar               │
 │  6   │ +3,3 V       │  ┐                                                                │
 │  7   │ GND          │  │ las dos alimentaciones estan DENTRO del mismo conector:        │
 │  8   │ +3,3 V       │  │ la placa hija se alimenta del conector, no del cobre           │
 │  9   │ GND          │  ┘ RUTA D: VCC de la via 8, GND de la via 9                       │
 │ 10-13│ sin conexion │                                                                   │
 └──────┴──────────────┴───────────────────────────────────────────────────────────────────┘
```

<details>
<summary><b>🗄️ Histórico — los tres bloqueos que tenía la ruta C antes de morir (27–28/08)</b></summary>

**Se conservan porque dos de ellos siguen vivos, con otro dueño, y porque si el Bluetooth se mudara
de `PB6`/`PB7` habría que volver a contestarlos.** Ninguno se hace leyendo código.

1. **El papel real de `PB6` no está cerrado.** El firmware lo llama `LCD_PSB`; la etiqueta de red
   del esquemático lo llama **`RS(A0)`**, y en ese mismo bloque el nombre `RS` ya está dado a
   `PB4`. **Los dos nombres no pueden ser ciertos a la vez.** ➜ **SIGUE ABIERTO, y ahora el que
   arriesga es el Bluetooth:** si `PB6` fuera de verdad el `RS/A0` del display, el `TX` del módulo
   estaría peleando con la línea de comando de la pantalla. Se comprueba encendiendo el equipo y
   mirando si la LCD dibuja (§7.0).
2. **El módulo del display puede traer `PSB` ya estrapado** (`roadmap.md` N-22). ➜ **SIGUE
   ABIERTO**, por el mismo motivo que el anterior.
3. **`PB6`/`PB7` no pueden ser `USART1` e `I²C1` a la vez.** El manual daba el remapeo por libre
   —*«hoy `USART1` vive sin remapear en `PA9`/`PA10`»*— y avisaba de que tomarlos para el I²C
   *«lo cierra para siempre»*. ➜ ⚫ **CONTESTADO EL 28/08, Y AL REVÉS DE COMO SE ESPERABA: el
   remapeo se lo quedó `USART1`.** El aviso era bueno; lo que falló es que se leyó como una
   hipótesis lejana —*«si el proyecto llegara a necesitar un módulo Bluetooth ahí»*— y ese «si
   llegara» ocurrió el mismo día. **El propio manual escribió que en ese caso el bus se mudaría a
   la ruta B. Es exactamente lo que manda hoy.**

</details>

### 4.3 Alimentación de la placa hija (igual en todas las rutas)

```text
 ┌───────────────────┬───────────────┬───────────────────────────┬──────────────────────────────────┐
 │ Señal de Poder    │ Pin STM32 U1  │ Componente KiCad          │ Punto Físico de Soldadura / Test │
 ├───────────────────┼───────────────┼───────────────────────────┼──────────────────────────────────┤
 │ 🔴 VCC (+3.3V)    │ Pines 9/24/48 │ Regulador U5 (LM1117-3.3) │ Pestaña TAB / Pin 2 de U5        │
 │ ⚫ GND (Masa)     │ Pines 8/23/47 │ Regulador U4 (LM7805)     │ Aleta metálica de U4 o Borne (-) │
 └───────────────────┴───────────────┴───────────────────────────┴──────────────────────────────────┘
```

* Si se elige la **ruta D**, `J17` ya trae `+3,3 V` (vías 6 y 8) y `GND` (vías 7 y 9): la placa hija
  puede alimentarse del propio conector y evitar por completo soldar en la placa madre.
  *(Esta línea decía «ruta C» hasta el 28/08. La ruta C ya no existe; la ventaja de alimentarse del
  conector pasó íntegra a la ruta D, que es la que hoy usa `J17`.)*

---

## 5. 🛒 LISTA DE COMPRAS Y ESPECIFICACIÓN DE COMPONENTES DE MOSTRADOR

Todos estos componentes se consiguen en cualquier mostrador de electrónica local.

> [!IMPORTANT]
> ### 📉 EL EXPANSOR `PCF8574` YA NO ES OBLIGATORIO — Y ESTE MANUAL SE LEÍA COMO SI LO FUERA
> `05_Funcional/15_Lista_de_Compras_Hardware.md` lo degradó a **«solo si además se quiere
> expandir»** al cruzar la placa hija con el cobre (`N-63`):
> * **Las talanqueras no lo necesitan.** Es una por poste, y la salida ya existe en la placa madre:
>   red **`Motor`** -> **`PB2`** -> opto `U15` -> MOSFET `Q10` -> bornera **`J15`** (§3 y `N-64`).
>   ~~Lo que falta es firmware, no un chip.~~ *(11/09: el firmware **ya la manda** —`escribirPines()`,
>   SFTY-28—; lo que falta es **el relé** de `D-25` entre `J15` p1 y p2, línea `A4` del Manual 15 — `D-27`:
>   **bobina de 12 V DC, contacto NA**, sin módulo optoacoplado ni `JD-VCC`; ver la tabla 🛒 de §3.)*
>   > 🔴 **ERRATA CORREGIDA EL 28/08 (2.ª rev.):** esta línea decía *«red `Puerta` -> bornera
>   > `J14`»*. **Las dos cosas eran falsas y peligrosas.** `Puerta`/`J14` es ~~la **ENTRADA** de la
>   > cámara de demanda~~ una **ENTRADA** (`PB0`) *(11/09: ~~reservada al fin de carrera por `A-2`;
>   > conflicto abierto con el firmware, que la lee como cámara~~ → **libre, sin cablear** por `D-27`:
>   > el fin de carrera no se instala en este despliegue, y el firmware la lee como cámara)*; cablear ahí un relé de 12 V destruye el micro y no mueve la pluma.
>   > Ver la fe de erratas de la cabecera y el cuadro comparativo de §3. La misma errata estaba en
>   > la fila **A4** de `15_Lista_de_Compras_Hardware.md`, y se ha corregido allí con su nota.
> * **Las cámaras tampoco.** ~~`PB0` ya se lee con antirrebote por hardware, por `J14`.~~ *(11/09:
>   las cámaras van a **`J16` p10 y p12, dos por poste** —`D-2`/`D-3`/`D-25`—, con el pull-down de
>   10 kΩ de la placa; ~~`J14` está reservada al fin de carrera por `A-2`~~ → `J14` queda **libre, sin
>   cablear** (`D-27`, 11/09).)*
> * ~~**El único que necesita bus es el reloj**, y solo si el cristal muerto es el del **Maestro**: si
>   es el del Esclavo, ese ya toma la hora por radio (SFTY-23) y no hay nada que comprar.~~
>
> 🛑 **07/09 — ESTA LÍNEA Y LA DE ABAJO ESTÁN DEROGADAS POR `D-9` y `D-15`, Y ERAN LAS QUE HACÍAN
> CREER QUE FALTABA COMPRAR UN RELOJ Y QUE BASTABA UNO:**
>
> * **El STM32 no tiene reloj en ninguna de las dos puntas** (`Y2` muerto, `N-17`) y **desde `D-15`
>   ni siquiera contesta a `SET_RTC`**. El camino que sincronizaba el reloj **del STM32** por radio
>   —el `SFTY-23` que citaba la línea tachada— **está derogado**: ~~esa punta **no tiene de dónde tomar
>   la hora si no es de su propio `DS3231`**~~.
>   > # 🔴 07/09 (`D-20`) — ESA ÚLTIMA FRASE ES FALSA DESDE HOY, Y ERA LA MÁS PELIGROSA DEL MANUAL
>   >
>   > **Declaraba IMPOSIBLE la propagación de la hora entre postes**, y sobre esa imposibilidad se
>   > sostenía todo el *«hay dos relojes por cruce y no se hablan»* que este documento repite.
>   > `DECISIONES.md` `D-20` **construye justo ese camino**:
>   >
>   > ```
>   > ESP32-M  ->  STM32-M  ->  radio  ->  STM32-E  ->  ESP32-E
>   > ```
>   >
>   > **La autoridad de la hora es el ESP32 Maestro, y sólo él. El Esclavo hace caso siempre.** Los
>   > STM32 son **carteros**: reciben la hora de su propio ESP32 y la pasan. **Sí hay de dónde tomar
>   > la hora sin tocar el `DS3231` local: del Maestro, por radio.**
>   >
>   > ⚠️ **PERO SIN CONSTRUIR** (`D-20` decide la autoridad, no la implementación): falta el mando
>   > ESP32 → STM32 que siembre la hora. El camino físico existe
>   > (`ESP32_Expansion/src/enlace_stm32.cpp`), **el mando no** — ese fichero no menciona `RTC` ni
>   > `hora` ni una vez (comprobado el 07/09).
>   >
>   > 🔴 **Y ESTO NO CAMBIA LA COMPRA: SIGUEN HACIENDO FALTA DOS `DS3231`, UNO POR POSTE (`A-5`).**
>   > El del Esclavo es el que **conserva la hora con su pila cuando se cae la radio** — que es
>   > exactamente por lo que perder la radio no es perder la hora. **`D-20` NO reduce la compra a
>   > uno.** Lo que obliga es a poner en hora el poste 2 **en la puesta en marcha, no durante la
>   > avería**: la radio se cae justo cuando hace falta el Modo Degradado, que es el modo que exige
>   > hora.
> * **Por tanto son DOS, uno por poste**, no uno; **`Y2` no decide cuántos**; y **ya están puestos**
>   (`A-5`, 05/09). **No hay reloj que comprar para este manual.**
> * **Y el bus tampoco se monta aquí:** el `DS3231` cuelga del `ESP32` (`GPIO21`/`GPIO22`), no del
>   STM32. Ver el recuadro del principio.
>
> ⚠️ ~~**Y el `DS3231` que se compre no dará la hora al enchufarlo: no hay driver** (§0). Se compra
> para tenerlo cuando llegue el firmware V9.0, no para arreglar el reloj esta semana.~~
> ⛔ **CADUCADO EL 07/09: el driver existe y está llamado.** Lo que sí puede pasar al enchufarlo, y
> es otra cosa, es que el módulo **no entregue hora válida** —pila equivocada, oscilador parado
> (`OSF`), modo 12 h—; en ese caso el firmware publica el hueco `--:--:--` **a propósito**, y el
> motivo se lee con **`CMD:LEER_RTC`** (`D-17`), que contesta `$ERR ... DESC:` por causa.

| Cant. | Componente | Encapsulado | ¿Obligatorio? | Función Técnica |
|:---:|---|---|:---:|---|
| ~~**1**~~ → **0** | **Módulo RTC DS3231 `ZS-042`** | **Módulo** de 4 pines rotulados `VCC GND SDA SCL`, con portapilas de moneda | ⛔ **NO SE COMPRA AQUÍ — 07/09.** ~~✅ Sí (solo si el cristal muerto es el del **Maestro**)~~ | Reloj de $\pm2\text{ ppm}$ para Modo Degradado. 🛑 **Son DOS, uno por `ESP32`, y YA ESTÁN PUESTOS** (`A-5` / `D-9` / `D-15`; línea `A6` de `15_Lista_de_Compras_Hardware.md`). ~~🔴 SIN DRIVER: no dará la hora al enchufarlo~~ ⛔ **falso desde el 05/09: el driver existe y está llamado (§0)** |
| ~~**1**~~ → **0** | **Pila de Botón `LIR2032`** *(o `CR2032`, ver aviso)* | Litio | ⛔ **con el anterior: no se compra aquí.** El aviso de recepción sigue valiendo y vive en `A6` de la lista 15 | Respaldo horario del `ZS-042` |
| ~~**2**~~ → **0** | **Resistencias de 4.7 kΩ** | 1/4 W | ⛔ **con el anterior.** El `ZS-042` trae sus propios pull-ups y **este bus no se monta** | ~~Pull-ups del Bus I²C (`SDA` y `SCL`). Hacen falta en **todas** las rutas de §4.2~~ |
| **1** | **Integrado `PCF8574P`** (o `PCF8574`) | **Circuito integrado DIP-16** *(no es un módulo)* | ⬜ Solo si además se quiere expandir E/S | Expansor I²C (8 E/S) - Dirección `0x20` |
| **1** | **Base / Zócalo DIP-16** | 16 pines | ⬜ Con el anterior | Protección térmica del chip al soldar |
| **2** | **Optoacopladores PC817** (o EL817) | DIP-4 | ⬜ Solo si se añaden entradas nuevas | Aislamiento galvánico para entradas de contacto seco |
| **2** | **Resistencias de 1 kΩ** | 1/4 W | ⬜ Con los anteriores | Limitación de corriente de optoacopladores |
| **3** | **Borneras de Tornillo Azules** | 2 pines (paso 5.08 mm) | ⬜ Con los anteriores | Conexión de cables |
| **1** | **Placa Perforada PCB** | 5×7 cm | ⬜ Solo si se monta placa hija | Base de ensamble |

> [!WARNING]
> ### 🧩 NO SON LA MISMA CLASE DE PIEZA — ES EL ERROR DE MOSTRADOR MÁS FÁCIL DE COMETER
> | | |
> |---|---|
> | **`PCF8574P`** | Un **integrado DIP-16**: una pastilla negra con 16 patas y **ninguna rotulación de `VCC`/`SDA`/`SCL`**. No trae portapilas, no trae placa. Si en el mostrador le entregan una placa pequeña con cuatro pines rotulados, **no es esto**. |
> | **`DS3231 ZS-042`** | El **módulo** con cuatro pines rotulados `VCC GND SDA SCL` y **portapilas de moneda**. Es lo que la mayoría de la gente tiene en mente al decir «el módulo I²C». |
>
> Pedir «el PCF8574 de cuatro pines con la pila» es pedir dos cosas distintas a la vez, y lo que
> vuelve en la bolsa es un `ZS-042`.

> ⚠️ **NO COMPRAR `PCF8574A`:** El modelo con sufijo "A" tiene dirección base `0x38`, lo que rompería la compatibilidad del firmware. Usar `PCF8574P` o `PCF8574` (dirección base `0x20`).

> ⚠️ **Pila del `ZS-042`:** el módulo trae un circuito de carga (`D1`/`R1`) pensado para una
> **`LIR2032` recargable**. Si se monta una **`CR2032` no recargable**, hay que **desoldar `D1` o
> `R1`** antes de energizar, o el módulo intentará cargar una pila que no admite carga.

---

## 6. 📐 ESQUEMA ELÉCTRICO DE LA PLACA HIJA DE ENTRADAS OPTOPACLADAS

```text
 ====================================================================================================
                                ESQUEMA ELÉCTRICO DE LA PLACA HIJA
 ====================================================================================================

  [1. ENTRADA CAMARA 1 (DEMANDA)]              [2. ENTRADA CAMARA 2 (ver aviso debajo)]
  Bornera J1 (Camara 1):                       Bornera J2 (Camara 2):
  Pin 1: +12V Entrada                          Pin 1: +12V Entrada
  Pin 2: Señal Relé Cámara                     Pin 2: Señal Relé Cámara
    │                                            │
    └──/\/\/\──►[1] Opto U1 [4]──► Pin 4 (P0)    └──/\/\/\──►[1] Opto U2 [4]──► Pin 5 (P1)
        1 kΩ    [2] (PC817) [3]──► GND Placa         1 kΩ    [2] (PC817) [3]──► GND Placa


  [3. BUS I2C HACIA STM32 KICAD]               [4. RELOJ DS3231 (ZS-042)]
  Conector 4 Pines:                            Conectar en paralelo con el Bus I2C:
  • VCC ──► +3.3V (U5 KiCad, o J17 v6/v8)      • Pin VCC ──► +3.3V
  • GND ──► Aleta GND (U4 KiCad, o J17 v7/v9)  • Pin GND ──► GND General
  • SDA ──► segun la RUTA elegida (ver 4.2)    • Pin SDA ──► al SDA del bus
  • SCL ──► segun la RUTA elegida (ver 4.2)    • Pin SCL ──► al SCL del bus
                                                 (ambas lineas con pull-up de 4.7 kOhm a 3.3 V)
 ====================================================================================================
```

> ### 🛑 05/09 — DOS CORRECCIONES AL DIAGRAMA DE ARRIBA, QUE ES DE UNA PLACA QUE NO SE HA FABRICADO
>
> 1. **~~«CÁMARA 2 (UMBRAL TRAMO)»~~ ⛔ NO EXISTE, y no es que falte: es que no hay tal función.**
>    La *«cámara de umbral»* venía de creer que `PB8` era una entrada; **`PB8` es el LED testigo `D5`
>    por `R16` de 1 kΩ** (N-64), y el símbolo `CAM_UMBRAL_PIN` **ya no existe en el fuente de ninguna
>    de las dos puntas**. **Las cámaras de este equipo son todas de DEMANDA: piden paso, no miden
>    ocupación de tramo.** Si una copia impresa manda cablear una cámara de umbral, **esa copia está
>    caducada**.
> 2. **Las borneras de cámara NO se llaman `J1` y `J2`.** En la tarjeta real la entrada de cámara es
>    ~~**`J14`** (`PB0`) y, desde el 31/08,~~ **`J16` p10 y p12** (`PB14`/`PB15`) *(11/09, `D-25`:
>    **las dos en cada poste**, p10 contra p9 y p12 contra p11; ~~`J14` quedó reservada al fin de
>    carrera por `A-2`, aunque el firmware la sigue leyendo como cámara — conflicto abierto~~ →
>    `J14` queda **libre, sin cablear** por `D-27` (11/09), y el firmware la sigue leyendo como cámara)*. `J1`/`J2` son
>    rótulos de este esquema hipotético y **no corresponden a nada en el cobre** — cablear guiándose
>    por ellos es el mismo error que mandó la talanquera a `J14`.
>
> 🔵 **Y el contrato, que aquí ahorra trabajo:** de cada cámara el sistema consume **un contacto seco
> y nada más** — sin red, sin imagen, sin analítica en el controlador (**D-12**). O sea que **esta
> placa hija de entradas optoacopladas no tiene nada que ver con las cámaras que hoy se instalan**:
> las ~~dos~~ que se cablean *(11/09: **cuatro, dos por poste** — `D-25`)* van **directas a `J16`**, que ya tiene su pull-down de 10 kΩ medido en cobre
> (`M3` cerrada, 03/09). **No hace falta `PCF8574` ni placa hija para leer cámaras.** El detalle, en
> **`9_Manual_Parametrizacion_Camara_IA.md`**.

**Dónde aterrizan `SDA` y `SCL` según la ruta de §4.2 — este manual ya no lo da por decidido:**

| Ruta | SDA | SCL | Requisito previo |
|:---:|---|---|---|
| **A** | Pin 18 (`PB0`) | Pin 45 (`PB8`) | 🔴 `C25` desoldado **y** `R16`/`D5` retirados. **No se ejecuta** |
| **B** | Pin 32 (`PA11`) | Pin 33 (`PA12`) | 🟢 Ninguno. Pads sin pista. **← MANDA ESTA** |
| ~~**C**~~ | ~~Pin 43 (`PB7`)~~ | ~~Pin 42 (`PB6`)~~ | ⚫ **MUERTA.** Los dos pines los usa el Bluetooth desde el 28/08 |
| **D** | Pin 40 (`PB4`), `J17` p1 | Pin 39 (`PB3`), `J17` p4 | 🟡 Solo en un equipo montado **sin LCD**. `VCC` de `J17` p8, `GND` de p9 |

> [!CAUTION]
> ### 🛑 DOS AVISOS SOBRE COPIAS IMPRESAS ANTERIORES
> * **Copia del 27/08:** el diagrama de arriba decía `PB0`/`PB8` **como si fuera la única opción**,
>   y en el caso de `PB8` mandaba soldar sobre el LED testigo.
> * **Copia del 28/08 (1.ª revisión):** daba la **ruta C** como *«la mejor»* y mandaba **puentear
>   `J17` vía 3 a `GND` y vía 2 a `3,3 V`**. 🔴 **Hoy eso pone en corto el `TX` del micro y el del
>   módulo Bluetooth.** Si alguien tiene esa copia, esa copia manda romper el enlace de telemetría.
>
> Se deja constancia en vez de reescribirlo en silencio.

---

## 7. 🔍 CONTROL DE CALIDAD ANTES DE ENERGIZAR

### 7.0 🔴 PRIMERO: confirmar sobre el cobre lo que este manual solo tiene medido en los ficheros

> **Un esquemático dice lo que se dibujó; una placa dice lo que se fabricó — y lo que alguien
> reparó después.** Estas cuatro comprobaciones se hacen **con la tarjeta desconectada y el pito
> del multímetro**, antes de decidir la ruta y antes de calentar el cautín.

* [ ] **`PB8` es el LED testigo, no un pad libre.** Medir continuidad entre el pin 45 de `U1` y
      `R16`, y entre `R16` y el LED `D5`. **Si hay continuidad, la ruta A exige desoldar `R16` o
      `D5`.** *(Si no la hubiera, es un hallazgo: la placa no coincide con su plano, y se para.)*
* [ ] **`PA11` y `PA12` están de verdad sueltos.** Medir de cada pad a `GND`, a `+3,3 V` y entre
      ambos: **no debe haber continuidad con nada**. Es lo que habilita la ruta B, y es lo único de
      este manual que la ruta B necesita.
* [ ] 🔴 **`J14` NO lleva la talanquera — antes de energizar nada.** Si hay un módulo de relé
      cableado a `J14`, **retirarlo**. La pluma va en `J15`. Comprobar con el pito: de `J15` debe
      haber camino al drenador de `Q10`; de `J14` lo hay a `R64`/`C25` y al pin 18 de `U1`. **Es la
      única comprobación de esta lista que puede salvar el microcontrolador.** *(11/09, `D-27`: y
      **en `J14` no va nada** —ni relé ni fin de carrera—: queda libre, sin cablear.)*
* [ ] **El papel de `PB6` — ahora es una comprobación del BLUETOOTH, no del bus.** Continuidad desde
      `J17` vía 3 hasta la pata **rotulada** del display, y leer qué dice esa rotulación: `PSB` o
      `RS`/`A0`. **Si dice `RS` o `A0`, quien está en riesgo es la pantalla**, porque esa vía lleva
      hoy el `TX` del módulo Bluetooth. **Ya no se puentea nada en ningún caso.**
* [ ] **El dorso del módulo del display.** Buscar un strap o puente de fábrica que fije `PSB` a
      `VCC` (`roadmap.md` N-22). **Si lo hay, hay dos salidas peleando en `J17` vía 3** — el strap
      del módulo contra el `TX` del Bluetooth — y hay que resolverlo antes de dar por bueno el
      enlace. *(Antes este punto decía «no se puentea `J17` vía 3 a `GND`». Ese puente pertenecía a
      la ruta C, que ya no existe.)*
* [ ] **Con la ruta D:** confirmar que la pantalla está **desconectada** antes de meter el bus por
      `J17` p1/p4. Con la LCD puesta, sus líneas de dato y el I²C compartirían pines.

### 7.1 Después de montar, y según la ruta elegida

* [ ] **Ruta A — retiro de `C25` verificado:** Medir con multímetro en capacitancia o resistencia entre el pad de `PB0` y `GND` para confirmar que `C25` fue retirado.
* [ ] **Ruta A — retiro de `R16`/`D5` verificado:** Confirmar que el pin 45 de `U1` ya **no** tiene camino a `GND` por el LED. Sin esto, el bus no alcanzará el nivel alto.
* [ ] **Nivel alto real del bus (todas las rutas):** con los pull-up de 4,7 kΩ puestos y el bus en reposo, medir `SDA` y `SCL` contra `GND`. **Deben estar a $3,3\text{ V} \pm 0,1$.** Cualquier lectura por debajo de ~2,5 V indica una carga colgando de la línea — que es exactamente el fallo que provocaba la fila errónea de `PB8`.
* [ ] **Continuidad de Tierras** *(solo si se monta el `PCF8574`)*: Verificar con el pito del multímetro que Pines 1, 2, 3 y 8 del `PCF8574` estén unidos a `GND` (fijando dirección `0x20`).
* [ ] **Aislamiento en VCC** *(solo si se monta el `PCF8574`)*: Medir resistencia entre Pin 16 (`VCC`) y Pin 8 (`GND`). No debe haber continuidad (< 10 kΩ indica cortocircuito).
* [ ] **Voltaje de Alimentación:** Conectar solo la tarjeta madre y verificar que al Pin `VCC` de la placa hija le lleguen exactamente $3.30\text{V} \pm 0.1\text{V}$.
* [ ] **La pantalla sigue pintando.** Recomendable en todas las rutas (y **sin objeto en la D**, que
      va sin pantalla): encender y comprobar que el LCD dibuja como antes. **Ahora este paso vigila
      el Bluetooth**: si `PB6` resultara ser el `RS/A0` del display, es aquí donde se ve.
* [ ] **El enlace Bluetooth responde.** Emparejar desde el celular y comprobar que llega telemetría.
      **No verificado en banco todavía**: el módulo se cableó a `J17` p2/p3 el 28/08 y la compuerta
      pasó, pero la compuerta no toca la tarjeta.
* [ ] 🔴 ~~**El reloj NO va a dar la hora, y eso no es un fallo del montaje.** Si se montó el
      `DS3231`, no se busque avería cuando la hora no aparezca: **no hay driver** (§0).~~
      🛑 **ANULADO EL 07/09 — ESTA CASILLA ENSEÑABA A NO MIRAR UNA AVERÍA.** El driver **existe y
      está llamado** desde el 31/08 (§0), y desde `D-9`/`D-15` **ese `DS3231` es el ÚNICO reloj del
      cruce**. Si la hora no aparece, **eso SÍ es un hallazgo y se diagnostica**:
      manda **`CMD:PIN:1234:` no — `CMD:LEER_RTC` va SIN PIN** (`D-17`), y el puente contesta
      `$ACK,NODE:PUENTE,...,FECHA:,HORA:` o un `$ERR ... DESC:` **por causa**
      (`OSCILADOR_PARADO_CAMBIE_PILA`, `NUNCA_SE_PUSO_PONGA_LA_HORA`, `SIN_RELOJ_NO_RESPONDE`, …).
      *(Este manual ya no compra ese reloj —ver §5—; la casilla se conserva corregida porque quien
      tenga una copia impresa anterior tiene que enterarse de que decía lo contrario.)*

---

*Manual de taller y especificación de hardware V9.0.*

*Revisado el 28/08/2026 (2.ª revisión):*
1. 🔴 *corregida la bornera de la talanquera en §5 —decía `J14`, es **`J15`**—, con fe de erratas en
   cabecera y cuadro comparativo en §3;*
2. *cerrada la **ruta C**: `PB6`/`PB7` los ocupó el Bluetooth ese mismo día, con el histórico de sus
   tres bloqueos conservado;*
3. *añadida la **ruta D** (`PB3`/`PB4` por `J17`, solo sin LCD) y declarado que **manda la ruta B**;*
4. *avisado, bien visible, de que el **`DS3231` no tiene driver**;*
5. *actualizada la cifra de flash contra el acta: **9.276 B libres, 85,8 %**.*

*Revisado el 28/08/2026 (1.ª revisión): corregida la fila de `PB8` de §4 (errata del 27/08), añadido
el censo de pines libres y las rutas del bus.*

> **SIGUE SIENDO PRE-IMPLEMENTACIÓN.** ~~Ni el `PCF8574` ni el `DS3231` tienen driver en el
> firmware.~~ 🔧 **CORREGIDO EL 07/09: el `PCF8574` no tiene driver Y NO SE COMPRA** (propuesta
> retirada, ver el recuadro del principio); **el `DS3231` SÍ lo tiene** —`ESP32_Expansion/src/reloj_ds3231.cpp`,
> con llamadores— **y ya está montado, uno por poste** (`A-5`, `D-9`, `D-15`). **De este manual no se
> compra ninguno de los dos.**
> Lo del 28/08 está **MEDIDO SOBRE EL FUENTE Y SOBRE LOS FICHEROS DE DISEÑO**; **nada de este manual
> está VERIFICADO SOBRE LA PLACA con multímetro.** Las dos cosas no son la misma, y la §7 existe
> para convertir la primera en la segunda.
>
> 🔴 **Y la lección que este manual se ha cobrado dos veces, escrita para que no haya una tercera:**
> el recuadro §0 corrigió el 05/09 lo de *«el `DS3231` no tiene driver»*… **y la frase siguió viva en
> otros CUATRO sitios de este mismo fichero** —la tabla «qué está medido», la §5, la fila de compra y
> la casilla de §7.1— hasta el 07/09. **Una corrección que no se propaga a las copias del propio
> documento no es una corrección: es una nota que la copia caducada tapa.**
