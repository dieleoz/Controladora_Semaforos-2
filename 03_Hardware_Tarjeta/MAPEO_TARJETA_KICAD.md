# MAPEO DE LA TARJETA — Controladora de Semáforos

Fuente real: `01_Firmware\Controladora_Semaforos\Controladora_Semaforos\Controladora_Semaforos.kicad_sch`
(649 KB, el bueno). **La ruta que este documento declaraba antes —`03_Hardware_Tarjeta\KiCad\...`—
no existe:** ese directorio solo contiene este `.md`. Quien "verificó contra el esquemático real" el
31/07 no pudo abrir el fichero que citaba.

**Última revisión:** 28 de agosto de 2026 — trazado red por red sobre el `.kicad_sch` bueno.

---

## 0. Qué nivel de prueba tiene cada cosa de este documento

> 🔴 **NADA de este documento está verificado con multímetro sobre el cobre.**

| nivel | qué significa |
|---|---|
| **MEDIDO EN EL ESQUEMÁTICO** | se trazó la red en el `.kicad_sch`: es lo que alguien **dibujó** |
| **VERIFICADO EN LA PLACA** | se midió con multímetro sobre la tarjeta física — **hoy no hay ni una sola fila así** |

La diferencia no es formal. El `.kicad_pcb` de este proyecto **está vacío**, así que entre el
esquemático y la tarjeta que hay encima de la mesa no existe ningún artefacto que las ate: el
ruteo real pudo hacerse a mano y **el esquemático dice lo que se dibujó, no lo que se fabricó**.

**Antes de soldar, cortar una pista o pinchar un hilo sobre cualquier dato de aquí, se comprueba
con continuidad.** Un mapeo leído de un dibujo es una hipótesis muy buena; sigue siendo una
hipótesis.

---

## 1. Microcontrolador

* **U1: `STM32F103C8Tx`** (ARM Cortex-M3), **soldado directamente a la placa** — no es un módulo
  BluePill. En algunas unidades puede venir el clon pin-compatible `CKS32F103C8T6`.
* Compilación: `board = genericSTM32F103C8` (sirve para original y clon).

> **Tip para clones CKS32F103:** si el ST-Link rechaza la primera carga, mantener pulsado el botón
> RESET al dar "Upload" y soltarlo cuando la consola muestre "Connecting...".

---

## 2. Bloques de la tarjeta

| Bloque | Componentes | Función |
|---|---|---|
| **Microcontrolador** | `U1` STM32F103C8Tx | Lógica de control |
| **Osciladores** | `Y1` **8 MHz** · `Y2` **32.768 kHz** | Reloj principal y **reloj de tiempo real** |
| **Aislamiento de salidas** | **10** × TLP127 (`U6`–**`U15`**) | Optoacopladores — **aislamiento galvánico** entre GPIO y potencia |
| **Salidas de potencia** | **10** × IRLZ44N (`Q1`–**`Q10`**) | MOSFET canal N — 3 grupos semafóricos + barrera |
| **Comunicación** | 2 × MAX3485 (`U2`, `U3`) | **Dos puertos RS-485** — ver §2.bis: `U2` es el del **USART1** y `U3` el de la **radio** |
| **Alimentación** | `U4` LM7805 → `U5` LM1117DT-3.3 | 12/24 V → 5 V (RS-485 y optos) → 3,3 V (STM32) |
| **Conectores** | `J1` alimentación · `J2` **SWD** · `J3`–`J9`,`J11`,`J13`,`J15` borneras de potencia · `J10`/`J12` RS-485 · `J14` cámara · `J16` botones · `J17` LCD | Ver §7 para el mapa pin a pin |
| **Interfaz** | `SW1` SW_Omron_B3FS | RESET / configuración |

**Red de tierra:** una sola (`GND`), sin masas separadas. Cualquier punto de GND es válido.

---

## 2.bis Los dos MAX3485 — CORREGIDO el 28/08, estaban INTERCAMBIADOS

> 🔴 **Este documento tuvo los dos MAX3485 al revés desde el 31/07/2026 hasta el 28/08/2026.**
> Decía *"MAX3485 U2 (Radio LoRa)"* y *"MAX3485 U3 en Hi-Z"* para el Bluetooth. **Es exactamente
> al contrario.** Se deja escrito el error, no se borra: una corrección que desaparece en silencio
> se vuelve a proponer, y la segunda vez ya nadie recuerda que se comprobó.
>
> Quien haya cableado radio o Bluetooth siguiendo este documento entre esas dos fechas **conectó
> al conector equivocado**, y la placa no se queja: los dos transceptores son la misma pieza con
> la misma terminación. El síntoma es silencio en el enlace, no humo.

MEDIDO EN EL ESQUEMÁTICO, red por red:

| | `U2` | `U3` |
|---|---|---|
| **Puerto** | **`USART1` — Bluetooth / telemetría** | **`USART3` — radio LoRa** |
| pin 1 `RO` (RX del micro) | `PA10` (`U1` pin 31) | `PB11` (`U1` pin 22) |
| pin 2 `~RE` + pin 3 `DE` | `PA8` (`U1` pin 29) — **una sola línea a los dos** | `PB12` (`U1` pin 25) — **una sola línea a los dos** |
| pin 4 `DI` (TX del micro) | `PA9` (`U1` pin 30) | `PB10` (`U1` pin 21) |
| **Par A/B (pines 6 y 7)** | **`J10`** | **`J12`** |
| Terminación 120 Ω | `R6` | `R10` |
| Polarización *fail-safe* 4,7 kΩ | `R7` (a 3,3 V) · `R8` (a GND) | `R9` (a 3,3 V) · `R11` (a GND) |

> ⚠️ **La radio sale por `J12`.** `J10` es el del `USART1` y **hoy está vacío**.
>
> El error era fácil de cometer y difícil de ver: los dos bloques son idénticos y están uno encima
> del otro en la hoja (`U2` a `y=40,64`, `J10` a `y=36,83`; `U3` a `y=88,90`, `J12` a `y=85,09`).
> Leer el dibujo "de arriba abajo" y suponer que el primero es la radio es la trampa. **La única
> forma de acertar es seguir la red hasta el pin del `U1`.**

**Consecuencia para el firmware:** `PA8` y `PB12` son líneas de **dirección del bus** (`DE`/`~RE`
unidos), no GPIO libres. Con `PA8` en HIGH el `U2` transmite; en LOW escucha. Eso es lo que
significaba *"`PA8` en HIGH"* en §6 — pero atribuido al transceptor equivocado.

---

## 3. Resistencias con función crítica

Identificadas en el esquemático. **Conviene conocerlas antes de modificar la placa.**

| Ref | Valor | Función |
|---|---|---|
| **`R5`** | **0 Ω** | 🔋 **Puente `VBAT` ↔ `3,3 V`.** Ver §4 — hay que retirarlo para instalar la pila del RTC |
| `R6` | 120 Ω | **Terminación RS-485 del `U2`** — el bus del `USART1`, el que sale por `J10` |
| `R7`, `R8` | 4,7 kΩ | **Polarización *fail-safe* del bus del `U2`** (`J10`). Mantienen la línea en estado definido cuando nadie transmite |
| **`R10`** | **120 Ω** | **Terminación RS-485 del `U3`** — el bus de la **radio**, el que sale por **`J12`** |
| **`R9`, `R11`** | **4,7 kΩ** | **Polarización *fail-safe* del bus de la radio** (`J12`) |
| `R1`, `R2`, `R3` | 10 kΩ | Pull-ups / divisores generales |
| `R4` | 1 MΩ | — |
| `R16` | 1 kΩ | Limitadora del LED testigo `D5` (`PB8`) |
| `R19`–`R20`, `R21`–`R22`, … | — | Pares de entrada y de puerta de cada cadena opto→MOSFET. Ver §8 |

> ✅ **Los DOS buses traen terminación y polarización.** Antes esta sección solo listaba `R6`/`R7`/`R8`
> y daba a entender que había un único juego: son dos completos y simétricos. No hay que añadir
> resistencias externas al bus en los nodos que usen esta placa.

---

## 4. 🔋 Instalación de la pila del RTC (modo intermitente nocturno)

El diseño **ya tiene todo previsto**: el cristal `Y2` de 32.768 kHz está en el esquemático y los
pines `PC14`/`PC15` del microcontrolador están cableados a él. **Solo falta la pila** — el diseño no
incluye portapilas.

### Por qué hay que quitar R5 primero

`R5` es un puente de 0 Ω que une `VBAT` con `3,3 V`. Es el patrón habitual cuando no se piensa
instalar pila: mantiene alimentado el dominio del RTC desde la fuente principal, y se deja como
resistencia **precisamente para poder retirarlo** si luego se añade la batería.

> ⚠️ **Si se conecta la pila sin quitar R5**, la CR2032 queda en paralelo con los 3,3 V y la fuente
> le inyecta corriente. Una pila **no recargable** en esa situación se calienta, se hincha y puede
> reventar.

### Procedimiento

**1. Verificar** — con la tarjeta **sin alimentación**, multímetro en continuidad:
   - Una punta en el **pin 1 de `U1`** (VBAT), otra en **3,3 V**
   - **Pita** → confirmado, R5 los une
   - **No pita** → R5 va a otro sitio; **no continuar** hasta aclararlo

**2. Desoldar `R5`.**

**3. Conectar el portapilas:**

```
    ┌──────────────┐
    │   CR2032     │   ← NO recargable
    │     3 V      │
    └──┬────────┬──┘
       │(+)     │(−)
       ▼        ▼
   pad de R5   GND
   lado VBAT
```

   - **Cable rojo (+)** → al **pad de `R5` que va a VBAT**. Es lo más cómodo: ese pad ya está
     conectado al pin 1, no hay que soldar sobre las patas del microcontrolador.
   - **Cable negro (−)** → cualquier punto de GND **verificado con multímetro**.

**4. Sin componentes adicionales.** Ni resistencias ni diodo: el STM32 conmuta solo a la pila
   cuando falta la energía principal y **no la realimenta** cuando vuelve.

### Puntos de GND cómodos para soldar

| Punto | Nota |
|---|---|
| **Aleta metálica del `U4`** (LM7805, TO-220) | Unida al pin central = GND. La superficie más grande |
| **Pin central del `U4`** | Pin 2 = GND |
| **Polo negativo de `J1`** | Entrada de alimentación |
| **Lado negativo de un electrolítico** | Marcado con franja |
| **Pin `GND` de las borneras RS-485** | Ya rotulado |

> ⚠️ **Verificar siempre con continuidad contra el negativo de la alimentación antes de soldar.**
> Soldar el negativo de la pila a los 3,3 V la pone en cortocircuito con la fuente.

### Tipo de pila — no confundir

| Montaje | Pila | Por qué |
|---|---|---|
| **VBAT de esta tarjeta** | **CR2032** (no recargable) | El STM32 **no carga** la pila |
| Módulo DS3231 externo | **LIR2032** (recargable) | El módulo **sí** trae circuito de carga |

**Poner la equivocada en cualquiera de los dos casos es peligroso.**

### Si el RTC no arranca

Algunos microcontroladores clonados traen mal calculados los condensadores de carga del cristal
($C_1/C_2 = 20\text{ pF}$) y el oscilador de 32.768 kHz no oscila.

> 🔴 **DESFASADO — N-37 SE CERRO EN BANCO EL 01/08/2026, un dia despues de revisar este documento
> (cabecera: 31 de julio).** El cristal `Y2` **esta muerto**, y no por descarte perezoso: `VBAT` a 3 V
> con la tarjeta apagada, el reintento de `N-25` y `REINICIAR RELOJ` devolviendo `SIGUE PARADO`.
> Ver `roadmap.md` N-37. **La salida es el `DS3231`, y necesita `PB0`/`PB8`.**
>
> ⚠️ Se conserva el texto de abajo porque su advertencia sobre medir antes de cambiar piezas sigue
> siendo valida para cualquier otra unidad. Lo que ya no vale es su conclusion.
>
> ⚠️ **Antes de tocar una sola pieza, léase `CONSULTA RELOJ`.** Que el cristal sea
> el culpable **no estaba medido cuando se escribio esto**. Se dio por culpable una vez sin medirlo y se cambiaron
> componentes sanos. La lectura se obtiene en `CONFIGURACION` → `AJUSTAR HORA`: si la hora queda
> puesta, el reloj arranca y **ninguna de estas dos opciones aplica**. Solo `Pedido, no oscila`
> señala al cristal.

Con esa lectura delante, y solo entonces:

* **Opción A (reparar PCB):** reemplazar $C_1$ y $C_2$ (SMD acoplados a $Y_2$) por capacitores de
  **6 pF a 10 pF (C0G/NP0)**, para bajar la impedancia de carga. **Hipótesis razonable, sin
  verificar en esta tarjeta:** es una petición a plantear al funcional, no una reparación decidida.
* **Opción B (módulo externo DS3231):** si el micro no responde, conectar módulo `DS3231`:

```
   DS3231  VCC ──► 3,3 V        SDA ──► PB0   ┐ I²C por software:
           GND ──► GND          SCL ──► PB8   ┘ el I²C por hardware está ocupado
```

> 🟠 **Corregido el 28/08: aquí decía que `PB0` y `PB8` eran "los únicos pines libres". Es falso
> por partida doble.** Ni son los únicos —`PA11`, `PA12`, `PA15` y `PC13` no tienen **ni una**
> conexión dibujada, ver §8.3—, ni están libres: `PB0` es la cámara de demanda (`J14`) y `PB8` el
> LED testigo `D5`. Y `PB6`/`PB7`, que se daban por ocupados por la LCD, **no son líneas de datos**
> — ver §6.bis, que es la vía más barata para un I²C por hardware.

---

## 5. Idea de diseño: el repetidor sobre esta misma placa

La tarjeta tiene **dos MAX3485** (`U2`, `U3`), es decir **dos puertos RS-485 independientes**. Eso
permitiría construir el **puente repetidor con esta misma placa**, escribiendo el firmware de puente
para STM32 en lugar de usar un ESP32 aparte.

**Ventajas:**
- Una sola referencia de placa para todo el sistema — mismo repuesto para semáforo y repetidor
- Terminación y polarización **ya diseñadas en los dos buses** (`R6`/`R7`/`R8` en `J10`, `R10`/`R9`/`R11` en `J12`)
- Los dos puertos ya salen a bornera: `J10` (`USART1`) y `J12` (radio). **`J10` está hoy vacío**, así
  que el segundo puerto del repetidor no necesita ni un hilo nuevo dentro de la placa
- Se elimina el ESP32, su alimentación y su cableado independiente
- El watchdog IWDG quedaría activo también en el repetidor *(el ESP32 hoy no lo tiene)*

La lógica del puente ya está escrita y validada en `01_Firmware/Repetidor`; portarla al STM32 es
cambiar el acceso al puerto serie. Pendiente de decisión.

---

## 6. Verificación cruzada PCB ↔ firmware

| Función | Pin STM32 (`pines.h`) | Componente |
|---|---|---|
| Luces semáforo 1 (R/A/V) | `PA0` · `PA1` · `PA2` | MOSFET `Q1`–`Q3` |
| Luces semáforo 2 (R/A/V) | `PA3` · `PA4` · `PA5` | MOSFET `Q4`–`Q6` |
| Peatonal (R/V) | `PA6` · `PA7` | MOSFET `Q7`–`Q8` |
| RS-485 radio (`USART3`) | `PB10` TX · `PB11` RX · `PB12` DE/~RE | **MAX3485 `U3`** → bornera **`J12`** · term. `R10` |
| Telemetría Bluetooth (`USART1`) | `PA9` TX · `PA10` RX · `PA8` DE/~RE | **MAX3485 `U2`** → bornera **`J10`** *(hoy vacía)* · term. `R6` |
| LCD ST7920 (**3 hilos de datos + 2 estáticos**) | datos: `PB3` `PB4` `PB5` — **estáticos: `PB6` `PB7`** | Conector **`J17`** — ver §6.bis y §7 |
| Botones 1–4 y Mando RF | `PB9` · `PB13` · `PB14` · `PB15` | Conector **`J16`** (pines 5, 8, 10, 12) — ver §7 |
| **Cristal RTC** | `PC14` · `PC15` | **`Y2` 32.768 kHz** |
| **Pila RTC** | `VBAT` (pin 1) | **vía `R5` (desoldada) — ver §4** |
| **Cámara de demanda** | `PB0` | ✅ **Resuelto.** La línea trae `R64` 10K + `C25` 100nF → bornera `J14`: antirrebote de ~1 ms que la placa ya da. Entrada **activa en alto** |
| **LED testigo** | `PB8` | ✅ **No es entrada de cámara.** Medido el 27/08 sobre el esquemático bueno: `PB8` → `R16` 1K → LED `D5`. Llevaba `pinMode()` y ni un `digitalRead()` (N-63) |
| **Barrera / talanquera** | `PB2` | `PB2` → opto `U15` → MOSFET `Q10` → bornera `J15`. **No es `Q9`**: `Q9` es el canal peatonal (`S7`→`U14`→`Q9`→`J11`) |
| 🟠 **Cámara de presencia (SFTY-29)** | `PA11` *(propuesto)* | **Sin cablear.** Es uno de los cuatro pines libres (`PA11`, `PA12`, `PA15`, `PC13`). Necesita un hilo y una bornera. Especificado, **sin construir** |

> **El conflicto `PB0`/`PB8` con el `DS3231` (N-37) quedó sin objeto:** el reloj es el **RTC interno del STM32** con el cristal `Y2` y pila en `VBAT` (SFTY-18), así que no hay módulo externo que reclame esos pines. La fila se retira porque **una fila que dice «sin resolver» sobre algo ya resuelto manda a alguien a arreglar lo que no está roto.**

---

## 6.bis 🟢 De los cinco pines de la LCD, DOS NO SON LÍNEAS DE DATOS

Hallazgo del 28/08. Este documento —y todos los que copiaban su tabla— listaba los cinco pines de la
pantalla en bloque, como si los cinco fueran señal. **No lo son**, y la diferencia vale un puerto
serie o un bus I²C **por hardware**, gratis.

| pin | red en `J17` | qué hace | ¿es dato? |
|---|---|---|---|
| `PB3` | `SCL` (p4) | reloj serie del ST7920 | ✅ **sí** — conmuta en cada bit |
| `PB4` | `CS` (p1) | *chip select* | ✅ **sí** |
| `PB5` | `SI` (p5) | dato serie | ✅ **sí** |
| **`PB6`** | **`RS(A0)`** (p3) | **`LCD_PSB`: selecciona el modo SERIE del ST7920** | ❌ **no** — nivel **estático** |
| **`PB7`** | **`RST`** (p2) | **reset del display** | ❌ **no** — pulso al arrancar y ya |

**La medida sobre el firmware**, que es lo que decide: en `lcd.cpp` (Maestro y Esclavo) `LCD_PSB`
aparece **exactamente dos veces**, seguidas, y nunca más:

```
pinMode(LCD_PSB, OUTPUT);
digitalWrite(LCD_PSB, LOW);   // ST7920 en modo serie
```

Un `pinMode()` + un `digitalWrite()` **una sola vez en toda la vida del equipo** no es una línea de
comunicación: es un puente que alguien decidió hacer con software. `LCD_RST` va a `u8g2` como pin de
reset, así que se mueve al arrancar y luego se queda quieto.

### Lo que eso abre

`PB6`/`PB7` **no son dos GPIO cualquiera**: en el STM32F103 son a la vez

- **`USART1` remapeado** (`PB6` = TX, `PB7` = RX), y
- **`I2C1` por hardware** (`PB6` = SCL, `PB7` = SDA).

> 🟢 **Se liberan SIN SOLDAR EN LA PLACA y SIN QUITAR LA PANTALLA:** basta puentear, **en el propio
> conector `J17`**, `PSB` a GND y `RST` a 3,3 V. La pantalla sigue en modo serie y sin reset activo,
> exactamente igual que hoy, pero los dos pines del micro quedan sueltos.

Es la vía más barata que tiene esta tarjeta para un periférico I²C —un `DS3231`, un sensor— o para un
segundo puerto serie, y **no compite con nada**: no toca los tres hilos de datos, no toca `J2` (SWD),
no toca ninguno de los dos RS-485.

> ⚠️ **PENDIENTE DE CONFIRMAR EN LA PLACA — no lo des por resuelto.** Hay una discrepancia de nombres
> que este documento **no puede cerrar leyendo dibujos**:
>
> - La **etiqueta de red del esquemático** para `PB6` es **`RS(A0)`**.
> - El **firmware** lo llama **`LCD_PSB`** y lo usa como `PSB` (nivel estático, modo serie).
>
> **`RS` y `PSB` son patas distintas del ST7920 y los dos nombres no pueden ser ciertos a la vez.**
> Si la pata real fuera `RS`, dejarla fija a LOW cambiaría el significado de lo que se manda, y el
> puente propuesto arriba sería incorrecto. Lo más probable es que la etiqueta sea un resto de
> plantilla de otro display —el resto de nombres de `J17` (`SCL`, `SI`, `CS`) son de un módulo
> genérico, no de un ST7920—, y a favor de esa lectura está que **la pantalla funciona hoy** con el
> firmware tratándolo como `PSB`. Pero *"funciona"* no distingue entre las dos hipótesis con
> seguridad suficiente para cortar o puentear nada.
>
> **Se cierra siguiendo el hilo del pin 3 de `J17` hasta la pata del módulo del display y leyendo su
> serigrafía.** Es una comprobación de cinco minutos con la tarjeta delante. Hasta que alguien la
> haga y lo anote aquí, **esto es una hipótesis, no un permiso para soldar.**

---

## 7. Mapa pin a pin de los conectores de señal

Todo MEDIDO EN EL ESQUEMÁTICO (§0). Ninguna de estas filas se ha comprobado con multímetro.

### `J17` — conector de la LCD

> ⚠️ **El símbolo y el footprint no coinciden, y en el cobre manda el footprint.** El símbolo es
> `Conn_01x13_Pin` (13 posiciones) pero el footprint es `Molex_KK-254_AE-6410-16A_1x16`: **en la placa
> hay 16 posiciones y en el esquema solo 13**. Al contar pines sobre la tarjeta física hay que
> contar **desde el pin 1**, no desde el borde del conector, o todo el mapa se desplaza.

| pin | red | destino |
|---|---|---|
| 1 | `CS` | `PB4` (`U1` p40) — dato |
| 2 | `RST` | `PB7` (`U1` p43) — **estático**, ver §6.bis |
| 3 | `RS(A0)` | `PB6` (`U1` p42) — **estático**, y **nombre en disputa**, ver §6.bis |
| 4 | `SCL` | `PB3` (`U1` p39) — dato |
| 5 | `SI` | `PB5` (`U1` p41) — dato |
| 6 | `3,3 V` | alimentación |
| 7 | `GND` | — |
| 8 | `3,3 V` | alimentación |
| 9 | `GND` | — |
| **10–13** | **SIN RED** | **no van a ningún sitio.** Dibujados y sin conectar |
| 14–16 | — | existen en el cobre, **no en el esquema** |

### `J16` — botones

Mismo desajuste: símbolo `Conn_01x12_Pin`, footprint `1x16`.

| pin | red | destino |
|---|---|---|
| 1 | `12V` | — |
| 2 | `GND` | — |
| **3** | **SIN RED** | — |
| 4 | `3,3 V` | — |
| 5 | `Boton1` | `PB9` (`U1` p46) |
| **6** | **SIN RED** | — |
| 7 | `3,3 V` | — |
| 8 | `Boton2` | `PB13` (`U1` p26) |
| 9 | `3,3 V` | — |
| 10 | `Boton3` | `PB14` (`U1` p27) |
| 11 | `3,3 V` | — |
| 12 | `Boton4` | `PB15` (`U1` p28) |

Cada botón lleva su antirrebote en placa (`R65`–`R68` + `C26`–`C29`). **`J16` es el único conector de
señal que trae 12 V** (pin 1) además de 3,3 V.

### `J2` — SWD

`PinHeader_1x04_P2.54mm_Vertical`.

| pin | red |
|---|---|
| 1 | `GND` |
| 2 | `PA14` (`U1` p37) — `SWCLK` |
| 3 | `PA13` (`U1` p34) — `SWDIO` |
| 4 | `3,3 V` |

> 🔴 **`J2` es la ÚNICA vía de carga de firmware de esta tarjeta.** No hay USB, no hay puerto de
> *bootloader* serie cableado. **No se reutiliza para nada**, por muy tentadores que sean sus cuatro
> pines: sin `J2` la placa se queda sin forma de recibir una actualización, y recuperarla exige
> soldar sobre las patas del `U1`.

---

## 8. Lo que NO hay: dónde se acaban las señales

Tres huecos que este documento no decía y que cambian cualquier plan de ampliación.

### 8.1 Los 5 V no salen a ningún conector

MEDIDO: la red `5V` toca el pin 6 de los diez optos (`U6`–`U15`), el pin 3 (`VO`) del `U4` LM7805, el
pin 3 (`VI`) del `U5` LM1117, los condensadores `C13`/`C14` y la limitadora `R17` de un LED. **Ni un
solo pin de conector.**

> Si algo externo necesita 5 V **hay que pinchar en la pata del `U4`**. No existe la alternativa
> cómoda. Los conectores de señal reparten 3,3 V (`J17`, `J16`, `J2`, `J14`) y `J16` reparte además
> 12 V; los 5 V son un raíl **interno**.

### 8.2 Las borneras de potencia no llevan señal del micro

`J3`–`J9`, `J11`, `J13`, `J15`: **pin 1 = 12 V, pin 2 = drenador del MOSFET.** Eso es todo.

Entre el GPIO y la bornera hay un **opto `TLP127`**, es decir **aislamiento galvánico**. Cadena
completa, medida sobre el canal del `PA0`:

```
PA0 (U1 p10, red "S1") -> R19 -> opto U6 -> R21/R22 -> gate Q1 -> drenador Q1 -> J3 p2
                                  |                                              J3 p1 = 12 V
                                  (barrera galvanica)
```

> ⚠️ **Quitar el MOSFET NO hace aparecer el GPIO en la bornera.** Es el error natural —"desueldo el
> `Q1` y me llevo la señal"— y aquí no funciona: al otro lado del opto no hay ninguna conexión
> eléctrica con el micro. Reutilizar un canal de potencia como entrada o como línea de datos exige
> **puentear por encima del opto**, que es otra intervención, mucho más invasiva, y que **destruye el
> aislamiento** que protege al STM32 de la línea de 12 V.

### 8.3 Pines del micro sin ninguna conexión dibujada

MEDIDO, uno por uno: **no tienen ni un hilo, ni una etiqueta, ni un `no_connect`.** Están sueltos.

| pin `U1` | GPIO | nota |
|---|---|---|
| 32 | `PA11` | libre — es el propuesto para la cámara de presencia (SFTY-29), **sin cablear** |
| 33 | `PA12` | libre |
| 38 | `PA15` | libre |
| 2 | **`PC13`** | libre, **pero NO sirve para un bus** |

> ⚠️ **`PC13` no es un GPIO normal.** En el STM32F103 pertenece al dominio de respaldo: da **3 mA** y
> su velocidad de salida está limitada a **2 MHz**. Vale para un LED o un relé a través de un
> transistor; **no vale para `SCL`, ni para `TX`, ni para nada que conmute rápido**. Contarlo como
> "cuarto pin libre" sin esta nota es lo que lleva a diseñar sobre él y descubrirlo al soldar.
>
> Y los cuatro tienen el mismo problema práctico: **están sueltos en el micro, no en un conector.**
> Usarlos significa **soldar sobre la pata del `U1`** (paso 0,5 mm) y sacar un hilo. Por eso el
> `PB6`/`PB7` de §6.bis es mejor camino que cualquiera de estos cuatro: sale por un conector.

---

## 9. Registro de correcciones

**No se borra lo que estaba mal.** Una corrección que desaparece en silencio se vuelve a proponer, y
la segunda vez ya nadie recuerda que se comprobó.

| fecha | qué decía | qué mide el esquemático | dónde |
|---|---|---|---|
| 31/07 → **28/08** | *"MAX3485 `U2` (Radio LoRa)"* y *"`U3` en Hi-Z"* para el Bluetooth | **al revés**: `U2` = `USART1`/Bluetooth (`J10`), `U3` = radio (`J12`) | §2.bis, §6 |
| 31/07 → **28/08** | la radio sale por `J10` | la radio sale por **`J12`**; **`J10` está vacío** | §2.bis |
| 31/07 → **28/08** | terminación/polarización = `R6`/`R7`/`R8` (un solo juego) | **dos juegos**: `R6`/`R7`/`R8` (`J10`) y `R10`/`R9`/`R11` (`J12`) | §3 |
| 31/07 → **28/08** | *"`PB0` y `PB8` son los únicos pines libres"* | falso: `PA11`/`PA12`/`PA15`/`PC13` sueltos, y `PB0`/`PB8` **ocupados** | §4, §8.3 |
| 31/07 → **28/08** | los cinco pines de la LCD, en bloque | **tres son datos, dos son estáticos** (`PB6`, `PB7`) | §6.bis |
| 31/07 → **28/08** | *"9 × TLP127 (`U6`–`U14`)"* y *"9 × IRLZ44N (`Q1`–`Q9`)"* | **10 y 10**: `U6`–`U15`, `Q1`–`Q10` | §2 |
| 31/07 → **28/08** | fuente: `03_Hardware_Tarjeta\KiCad\...` | **esa ruta no existe**; el bueno está en `01_Firmware\Controladora_Semaforos\...` | cabecera |

**Cómo se midió, que es la parte reutilizable:** no se leyó el dibujo — se **trazó la conectividad**.
Se parsea el `.kicad_sch`, se sitúan los pines de cada símbolo aplicando su rotación y espejo, se unen
por los segmentos de `wire`, se cierran las etiquetas del mismo nombre y se sacan las componentes
conexas. **Leer el esquemático "de arriba abajo" es justo lo que produjo el error de los MAX3485:**
los dos bloques son idénticos y están uno sobre otro, así que la posición en la hoja no dice nada. La
única forma de acertar es seguir la red hasta el pin del `U1`.

**Lo que sigue abierto, y no se cierra desde un fichero:**

1. **El nombre real del pin 3 de `J17`** (`RS(A0)` en el esquema contra `PSB` en el firmware) — §6.bis.
2. **Todo lo demás de este documento**, en el sentido de §0: está medido sobre lo que alguien
   **dibujó**. El `.kicad_pcb` está vacío. La primera persona que tenga la tarjeta delante y un
   multímetro puede convertir filas de "MEDIDO EN EL ESQUEMÁTICO" en "VERIFICADO EN LA PLACA", y
   conviene que lo anote aquí fila a fila, porque **hoy no hay ni una sola**.
