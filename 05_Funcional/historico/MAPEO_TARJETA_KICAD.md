# MAPEO DE LA TARJETA — Controladora de Semáforos

Fuentes reales, las dos en `01_Firmware\Controladora_Semaforos\Controladora_Semaforos\`:

| fichero | tamaño | qué contiene |
|---|---|---|
| `Controladora_Semaforos.kicad_sch` | 649 KB | el esquemático bueno |
| `Controladora_Semaforos.kicad_pcb` | **2.158.421 B** | **el ruteo: 185 huellas, 1.447 pistas, 89 vías, 485 pads, 117 redes** |

**La ruta que este documento declaraba antes —`03_Hardware_Tarjeta\KiCad\...`— no existe:** ese
directorio solo contiene este `.md`. Quien "verificó contra el esquemático real" el 31/07 no pudo
abrir el fichero que citaba.

**Última revisión:** 28 de agosto de 2026 — trazado red por red sobre el `.kicad_sch` **y sobre el
`.kicad_pcb`**, que hasta hoy este documento daba por vacío (ver §0).

---

## 0. Qué nivel de prueba tiene cada cosa de este documento

> ✅ **YA NO ES CIERTO — corregido el 05/09: `J16` SÍ está medido con multímetro (M3, 03/09).**
> Lo que cambia hoy es que ya no hay solo dos niveles, sino **tres**, y el intermedio existía desde
> el principio sin que nadie lo mirara.

### 🔴 Lo que este documento afirmó hasta el 28/08, y era falso

> *"El `.kicad_pcb` de este proyecto **está vacío**, así que entre el esquemático y la tarjeta que
> hay encima de la mesa no existe ningún artefacto que las ate."*

**No está vacío. Está enteramente ruteado.** El plano bueno pesa **2.158.421 bytes** y contiene
**185 huellas, 1.447 segmentos de pista, 89 vías, 485 pads y 117 redes**.

Los `.kicad_pcb` vacíos existen, pero **no son este**: son los de
`99_Legacy\Controladora_Semaforos-backups\`, cinco ficheros de **78 bytes** —una cabecera y un
paréntesis de cierre, sin una sola pista—. **Se midió sobre una copia incompleta y la conclusión se
escribió apuntando al plano bueno.** Es N-64 repitiéndose.

Las cinco copias grandes del repositorio (`01_Firmware` y cuatro entregables de `99_Legacy`) son
**el mismo fichero**: `md5 = 088667eac75207e8dcfa0ce5b93adce6`. No hay ambigüedad sobre cuál es.

**Y la trampa que probablemente lo causó, porque hay que dejarla escrita:** el fichero separa los
tokens con **tabulador y salto de línea**, no con espacio. Buscar la pista con un espacio detrás
**da cero**, y un cero se lee como "vacío":

```
grep -c '(segment ' Controladora_Semaforos.kicad_pcb    ->    0     <-- FALSO
grep -oE '\(segment\b' Controladora_Semaforos.kicad_pcb | wc -l -> 1447   <-- REAL
```

Un `0` de un buscador que no sabía buscar. **Un "no aparece" no es un hallazgo hasta haber
descartado al buscador**, y aquí el buscador era el culpable entero.

### Los tres niveles

| nivel | qué significa | qué NO significa |
|---|---|---|
| 📐 **ESQUEMÁTICO** | se trazó la red en el `.kicad_sch`: es lo que alguien **dibujó** | que se rutease así |
| 🟦 **PCB** | se trazó sobre el cobre del `.kicad_pcb`: es **lo que se mandó fabricar** | que la placa de la mesa sea así |
| 🔬 **MULTÍMETRO** | se midió con puntas sobre la tarjeta física — **hoy no hay ni una sola fila así** | — |

**El nivel que se gana es real, y su límite también.** El PCB ata el esquemático al cobre: ya no
hace falta suponer que el ruteo respetó la netlist, porque las 1.447 pistas dicen por dónde va cada
señal, y además dan lo que el esquemático **no puede dar** —distancias en milímetros, capas,
holguras—. Pero **el PCB dice lo que se mandó fabricar, no lo que salió de fábrica**: una tarjeta
puede llegar con un componente no montado, y puede haberse retocado a mano después —un puente, una
pista cortada, una resistencia quitada— sin que nada de eso vuelva al fichero.

**Antes de soldar, cortar una pista o pinchar un hilo sobre cualquier dato de aquí, se comprueba
con continuidad.** Un mapeo leído de un dibujo es una hipótesis muy buena; leído del PCB es mejor;
**sigue siendo una hipótesis.**

### Estado de verificación de lo que este documento afirma

> 🔴 ~~**La tercera columna está vacía en todo lo que este documento afirma sobre la tarjeta.**~~ →
> ✅ **CADUCADO EL 03/09: `M3` puso las puntas sobre `J16`** (Guía de banco, pasos 20 y 21). Se marca y
> no se borra. Lo que **sigue** sin medir es todo lo demás: `J17`, `J2`, `J10`/`J12` y las diez cadenas.

| afirmación | 📐 ESQ | 🟦 PCB | 🔬 MULT | dónde |
|---|:---:|:---:|:---:|---|
| `U2` = `USART1`/Bluetooth (`J10`) · `U3` = radio (`J12`) | ✅ | ✅ **trazado hasta el pin del `U1`** | ⬜ | §2.bis |
| Terminación y polarización dobles (`R6`/`R7`/`R8`, `R10`/`R9`/`R11`) | ✅ | ✅ **los dos juegos, en `J10` y `J12`** | ⬜ | §3 |
| `R5` = 0 Ω puentea `VBAT` ↔ 3,3 V | ✅ | ✅ *(valor `0` en el PCB)* | ⬜ | §3, §4 |
| Cadena `GPIO → R → opto → MOSFET → bornera` | ✅ | ⬜ | ⬜ | §8.2 |
| Los 5 V no salen a ningún conector | ✅ | ✅ *(15 pads en la red, ninguno de conector)* | ⬜ | §8.1 |
| **Los dos MAX3485 se alimentan de 3,3 V, no de 5 V** | ✅ | ✅ **corregido** | ⬜ | §2, §7.bis.4 |
| `PA11`/`PA12`/`PA15`/`PC13` sin conexión | ✅ | ✅ *(`unconnected-` en `U1` p32/33/38/2)* | ⬜ | §8.3 |
| **`J16` p5/p8/p10/p12 → `U1` p46/p26/p27/p28, sin nada en serie** | ✅ | ✅ **trazado pista a pista** | 🟡 **p10 en banco** *(paso 21: cerrado contra p11 movió `CAM_C_PIN`)* | **§7.bis** |
| **`R65`–`R68` van a GND (pull-DOWN, activo en ALTO)** | ✅ | ✅ **resuelto** | ✅ **M3, 03/09 — las CUATRO posiciones, `0 V` en reposo** | **§7.bis** |
| **`J16` p4/p7/p9/p11 y `J14` p2 son el mismo nudo de 3,3 V** | ✅ | ✅ **resuelto** | ⬜ | **§7.bis** |
| **Separación real 12 V ↔ señal en `J16`** | ~10 mm *(estimado)* | ✅ **1,359 mm medidos** | ⬜ | **§7.bis** |
| `J17` p1–p5 → `U1` p40/p43/p42/p39/p41 | ✅ | ✅ | ⬜ | §7 |
| `J17`/`J16`: símbolo de 13/12 posiciones, footprint de 16 | ✅ | ✅ **confirmado** | ⬜ | §7 |
| **Nombre real del pin 3 de `J17` (`RS` contra `PSB`)** | ❌ **en disputa** | ❌ **el PCB tampoco lo cierra** | ⬜ | §6.bis |
| El cristal `Y2` está muerto en la unidad de banco | — | — | ✅ *(N-37, en banco)* | §4 |

> ⚠️ **Una fila con 🟦 y sin 🔬 no es una fila cerrada.** Sube de "alguien lo dibujó" a "así se pidió
> a fábrica". La placa de la mesa sigue sin haber sido medida ni una vez.

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
| **Alimentación** | `U4` LM7805 → `U5` LM1117DT-3.3 | 12/24 V → **5 V (solo los diez optos)** → **3,3 V (STM32 *y los dos MAX3485*)** — corregido, ver abajo |
| **Conectores** | `J1` alimentación · `J2` **SWD** · `J3`–`J9`,`J11`,`J13`,`J15` borneras de potencia · `J10`/`J12` RS-485 · `J14` cámara · `J16` botones · **`J17` LCD + módulo Bluetooth/ESP32** | Ver §7 para el mapa pin a pin |
| **Interfaz** | `SW1` SW_Omron_B3FS | RESET / configuración |

**Red de tierra:** una sola (`GND`), sin masas separadas. Cualquier punto de GND es válido.

> 🟠 **Corregido el 28/08 al trazar el PCB: los RS-485 NO van a 5 V, van a 3,3 V.** Esta tabla decía
> *"5 V (RS-485 y optos)"*. Medido sobre el cobre, `U2` p8 y `U3` p8 —el `VCC` de los dos
> transceptores— están en el nudo de **3,3 V**, el mismo del STM32 (§7.bis.4). La red de **5 V toca
> exactamente 15 pads y ninguno es de un MAX3485**: el pin 6 de los diez optos, `U4` p3 (`VO`),
> `U5` p3 (`VI`), `C13`/`C14` y `R17`. Coincide con lo que §8.1 ya decía; lo que estaba mal era esta
> fila.
>
> **No es un detalle de nomenclatura:** el `MAX3485` es precisamente la variante de 3,3 V del
> `MAX485`, así que el diseño es coherente — pero quien leyera esta tabla para alimentar un
> transceptor de repuesto **le habría metido 5 V**.

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

MEDIDO EN EL ESQUEMÁTICO, red por red — y 🟦 **confirmado pista a pista sobre el `.kicad_pcb`**: las
doce redes de la tabla salen igual siguiendo el cobre hasta la pata del `U1`, incluidos los dos pares
`~RE`+`DE` compartiendo una sola red (`/PA8` y `/PB12`).

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
| ✅ **`R65`–`R68`** | **10 kΩ** | **Pull-DOWN de las cuatro posiciones de `J16` (p5/p8/p10/p12) — un extremo en la señal, el otro en `GND`.** 🟦 Cobre y 🔬 **multímetro (M3, 03/09)**. **Entrada activa en ALTO, y desde `346ea5f` el firmware TAMBIÉN lee activa en ALTO: la contradicción está CERRADA — ver §7.bis.3** |
| **`R64`** | **10 kΩ** | Pull-DOWN de la cámara de demanda (`J14` p1, `PB0`). Mismo patrón: a `GND`, con `C25` |
| `C26`–`C29` | 100 nF | Antirrebote de los botones, **en paralelo a `GND`** — no en serie con la señal |
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
> — ver §6.bis.
>
> 🔴 **Pero esa puerta ya se cerró: no propongas `PB6`/`PB7` para un `DS3231`.** Era la vía más
> barata de la tarjeta para un I²C por hardware, y **N-76 la gastó en el puerto serie del ESP32**
> (`J17` p3/p2). Quien necesite hoy un bus tiene que disputárselo al Bluetooth o irse a los cuatro
> pines sueltos de §8.3, **que no salen a ningún conector** y obligan a soldar en la pata del `U1`.

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

> ⚠️ **Cuidado desde N-76: esta idea y el módulo Bluetooth se disputan el mismo `USART1`.** El
> puerto de `U2`/`J10` **es** el `USART1` por su salida original (`PA9`/`PA10`), y el `USART1` solo
> puede salir por un sitio a la vez. Desde N-76 sale **remapeado por `PB6`/`PB7`** hacia el ESP32 de
> `J17`. Un repetidor sobre esta placa tendría que **renunciar al Bluetooth de telemetría** o
> repartirse el puerto por turnos. **No son dos recursos independientes**, aunque `J10` y `J17` sean
> dos conectores distintos.

---

## 6. Verificación cruzada PCB ↔ firmware

| Función | Pin STM32 (`pines.h`) | Componente |
|---|---|---|
| Luces semáforo 1 (R/A/V) | `PA0` · `PA1` · `PA2` | MOSFET `Q1`–`Q3` |
| Luces semáforo 2 (R/A/V) | `PA3` · `PA4` · `PA5` | MOSFET `Q4`–`Q6` |
| Peatonal (R/V) | `PA6` · `PA7` | MOSFET `Q7`–`Q8` |
| RS-485 radio (`USART3`) | `PB10` TX · `PB11` RX · `PB12` DE/~RE | **MAX3485 `U3`** → bornera **`J12`** · term. `R10` |
| 🟠 **`USART1` por su salida ORIGINAL — hoy SIN USAR** | `PA9` TX · `PA10` RX · `PA8` DE/~RE | **MAX3485 `U2`** → bornera **`J10`** *(vacía)* · term. `R6`. **Desde N-76 el `USART1` sale remapeado por `PB6`/`PB7`, no por aquí** — y solo puede salir por un sitio a la vez, así que `U2` y `J10` quedan libres |
| LCD ST7920 (**3 hilos de datos, y solo tres desde N-76**) | `PB3` `PB4` `PB5` | Conector **`J17`** p4, p1, p5 — ver §6.bis y §7 |
| 🟢 **Módulo Bluetooth / ESP32 (`USART1` REMAPEADO)** | **`PB6` TX · `PB7` RX** | Conector **`J17`** p3 y p2 — **antes eran `PSB`/`RST` de la pantalla**, ver §6.bis y §7 |
| **Mando `A`/`B`** *(`BOTON1`/`BOTON2`)* | `PB9` · `PB13` | **`J16`** p5 · p8. ✅ **`INPUT` pelado, activo en ALTO** (`346ea5f`, N-118) — casa con el pull-DOWN `R65`/`R66`. 🔴 **El mando FÍSICO se retiró** (`DECISIONES.md` `D-1`, 05/09); su **código se queda**. Qué se cablea aquí es **`A-2`, abierto** — y el firmware **sigue leyendo estos pines** |
| 🆕 **Cámaras `C` y `D`** *(`CAM_C_PIN`/`CAM_D_PIN`, ex-`BOTON3`/`BOTON4`)* | `PB14` · `PB15` | **`J16`** p10 · p12. `INPUT` pelado, **activo en ALTO** — **las cuatro posiciones de `J16` son la misma polaridad**, sin excepción (§7.bis.3). **Cableadas y verificadas en banco el 03/09** (paso 21) |
| **Cristal RTC** | `PC14` · `PC15` | **`Y2` 32.768 kHz** |
| **Pila RTC** | `VBAT` (pin 1) | **vía `R5` (desoldada) — ver §4** |
| **Cámara de demanda** | `PB0` | ✅ **Resuelto.** La línea trae `R64` 10K + `C25` 100nF → bornera `J14`: antirrebote de ~1 ms que la placa ya da. Entrada **activa en alto** |
| **LED testigo** | `PB8` | ✅ **No es entrada de cámara.** Medido el 27/08 sobre el esquemático bueno: `PB8` → `R16` 1K → LED `D5`. Llevaba `pinMode()` y ni un `digitalRead()` (N-63) |
| **Barrera / talanquera** | `PB2` | `PB2` → opto `U15` → MOSFET `Q10` → bornera `J15`. **No es `Q9`**: `Q9` es el canal peatonal (`S7`→`U14`→`Q9`→`J11`) |
| 🟠 **Cámara de presencia (SFTY-29)** | `PA11` *(propuesto)* | **Sin cablear.** Es uno de los cuatro pines libres (`PA11`, `PA12`, `PA15`, `PC13`). Necesita un hilo y una bornera. Especificado, **sin construir** |

> **El conflicto `PB0`/`PB8` con el `DS3231` (N-37) quedó sin objeto:** el reloj es el **RTC interno del STM32** con el cristal `Y2` y pila en `VBAT` (SFTY-18), así que no hay módulo externo que reclame esos pines. La fila se retira porque **una fila que dice «sin resolver» sobre algo ya resuelto manda a alguien a arreglar lo que no está roto.**

---

## 6.bis 🟢 De los cinco pines de la LCD, DOS NO SON LÍNEAS DE DATOS — y ya los usa el ESP32

Hallazgo del 28/08. Este documento —y todos los que copiaban su tabla— listaba los cinco pines de la
pantalla en bloque, como si los cinco fueran señal. **No lo son**, y la diferencia valía un puerto
serie o un bus I²C **por hardware**, gratis.

> 🟢 **Ya no es una propuesta: N-76 lo hizo el mismo día.** `PB6`/`PB7` son hoy el `USART1`
> remapeado por donde entra el módulo Bluetooth/ESP32, y la pantalla funciona con tres hilos. En
> `lcd.cpp` (las dos puntas) el constructor recibe `U8X8_PIN_NONE` en vez de `LCD_RST` y
> `lcd_setup()` ya no toca `LCD_PSB`. Lo de abajo se conserva porque **es el razonamiento que
> permitió el cambio**, y porque el puente físico de `J17` sigue siendo el paso pendiente en placa.

| pin | red en `J17` | qué hacía para la LCD | ¿era dato? | **hoy** |
|---|---|---|---|---|
| `PB3` | `SCL` (p4) | reloj serie del ST7920 | ✅ **sí** — conmuta en cada bit | LCD |
| `PB4` | `CS` (p1) | *chip select* | ✅ **sí** | LCD |
| `PB5` | `SI` (p5) | dato serie | ✅ **sí** | LCD |
| **`PB6`** | **`RS(A0)`** (p3) | **`LCD_PSB`: selecciona el modo SERIE del ST7920** | ❌ **no** — nivel **estático** | 🟢 **`USART1` TX → ESP32** |
| **`PB7`** | **`RST`** (p2) | **reset del display** | ❌ **no** — pulso al arrancar y ya | 🟢 **`USART1` RX → ESP32** |

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
>
> 🟦 **El PCB confirma que el puente es cómodo:** `J17` reparte `GND` en **p7 y p9** y 3,3 V en
> **p6 y p8**, y las cuatro posiciones son del mismo nudo que el resto de la placa (§7.bis.4). El
> puente se hace **dentro del conector**, sin tocar cobre.

Era la vía más barata que tenía esta tarjeta para un periférico I²C —un `DS3231`, un sensor— o para
un segundo puerto serie, y **no competía con nada**: no toca los tres hilos de datos, no toca `J2`
(SWD), no toca ninguno de los dos RS-485. **N-76 gastó esa vía en el puerto serie del ESP32**, así
que quien venga ahora a pedir un I²C por hardware ya no la tiene libre: tendría que disputársela al
Bluetooth o irse a los pines sueltos de §8.3, que exigen soldar en la pata del `U1`.

> ⚠️ **PENDIENTE DE CONFIRMAR EN LA PLACA — no lo des por resuelto.** Hay una discrepancia de nombres
> que este documento **no puede cerrar leyendo dibujos**, y **el `.kicad_pcb` tampoco la cierra**: el
> cobre confirma que `J17` p3 va a `U1` p42 y a ningún sitio más, pero el nombre discutido es el de
> **la pata del módulo del display**, que está al otro lado del conector y fuera de la tarjeta.
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

Nivel **📐 ESQUEMÁTICO**, salvo lo marcado con 🟦, que está además trazado sobre el cobre del
`.kicad_pcb` (§0, §7.bis). **Ninguna de estas filas se ha comprobado con multímetro.**

### `J17` — LCD **y entrada del módulo Bluetooth / ESP32**

> 🟢 **BUSCA AQUÍ SI BUSCAS "DÓNDE VA EL ESP32": es `J17`, posiciones 3 y 2.** Este documento llamó a
> `J17` *"el conector de la LCD"* a secas hasta el 28/08, y desde N-76 **eso ya solo es la mitad**:
> el módulo Bluetooth/ESP32 de telemetría entra por dos de sus pines. No se borra que fue de la
> pantalla —es exactamente así como se explica **por qué** esos pines están ahí y por qué se
> llaman `RST` y `RS(A0)` en el cobre—.

> ⚠️ **El símbolo y el footprint no coinciden, y en el cobre manda el footprint.** El símbolo es
> `Conn_01x13_Pin` (13 posiciones) pero el footprint es `Molex_KK-254_AE-6410-16A_1x16`: **en la placa
> hay 16 posiciones y en el esquema solo 13**. 🟦 **Confirmado en el PCB:** hay 16 pads. Al contar
> pines sobre la tarjeta física hay que contar **desde el pin 1**, no desde el borde del conector, o
> todo el mapa se desplaza.

| pin | red | `U1` | destino | **hoy lo usa** |
|---|---|---|---|---|
| 1 | `CS` | p40 | `PB4` — dato | LCD |
| **2** | `RST` | **p43** | **`PB7`** | 🟢 **ESP32 — `RX` del micro** (recibe el `TXD` del módulo) |
| **3** | `RS(A0)` | **p42** | **`PB6`** | 🟢 **ESP32 — `TX` del micro** (va al `RXD` del módulo) |
| 4 | `SCL` | p39 | `PB3` — dato | LCD |
| 5 | `SI` | p41 | `PB5` — dato | LCD |
| 6 | `3,3 V` | — | alimentación | ambos |
| 7 | `GND` | — | — | ambos |
| 8 | `3,3 V` | — | alimentación | ambos |
| 9 | `GND` | — | — | ambos |
| **10–13** | **SIN RED** | — | **no van a ningún sitio.** Dibujados y sin conectar | — |
| 14–16 | — | — | pads en el cobre, **no en el esquema** | — |

🟦 **Los cinco destinos de señal están confirmados sobre el cobre**, y cada red contiene
**exactamente dos pads** —el del conector y el del micro—: `CS`→`U1` p40, `RST`→p43, `RS(A0)`→p42,
`SCL`→p39, `SI`→p41. **Nada en serie en ninguna de las cinco.**

> ⚠️ **`J16` y `J17` son el mismo modelo de conector y están uno al lado del otro.** Los dos son
> `Molex KK-254` de 16 posiciones. **`J16` trae 12 V en su pin 1; `J17` no trae 12 V en ningún
> pin.** Antes de enchufar el módulo Bluetooth, **medir el pin 1 contra `GND`: si hay 12 V, ése es
> `J16` y no es el suyo.**
>
> 🟠 Y hay un dato del PCB que conviene tener delante al hacerlo: por el hueco entre `J17` p7
> (`GND`) y p8 (`3,3 V`) **pasa una pista de 12 V** de 0,4 mm, con **0,200 mm de holgura a cada
> lado**. Es ruteo legal y no toca nada, pero significa que **hay cobre a 12 V por debajo de `J17`**:
> al pinchar una punta o un hilo por esa zona no se está tan lejos de la potencia como sugiere el
> hecho de que `J17` no reparta 12 V.

### `J16` — **mando `A`/`B` (p5/p8) y CÁMARAS (p10/p12)** · ~~botones (y **futuras** cámaras)~~

Mismo desajuste: símbolo `Conn_01x12_Pin`, footprint `1x16`. **Confirmado en el PCB:** hay 16 pads,
los 4 últimos sin red ninguna.

| pin | red | destino |
|---|---|---|
| 1 | `12V` | — |
| 2 | `GND` | — |
| **3** | **SIN RED** | — |
| 4 | `3,3 V` | — |
| 5 | `Boton1` *(nombre de la red)* | `PB9` = **`BOTON1` / mando `A`** — `U1` p46 |
| **6** | **SIN RED** | — |
| 7 | `3,3 V` | — |
| 8 | `Boton2` *(nombre de la red)* | `PB13` = **`BOTON2` / mando `B`** — `U1` p26 |
| 9 | `3,3 V` | — |
| 10 | `Boton3` *(nombre de la red)* | `PB14` = 🆕 **`CAM_C_PIN` — LA CÁMARA**, ya no un botón — `U1` p27 |
| 11 | `3,3 V` | — |
| 12 | `Boton4` *(nombre de la red)* | `PB15` = 🆕 **`CAM_D_PIN` — cámara, posición HOY VACÍA** — `U1` p28 |
| **13–16** | **sin red** | pads que existen en el cobre y **no** en el esquema |

> 🔴 **`Boton3` y `Boton4` son NOMBRES DE RED DEL KiCad, no funciones vivas.** Desde `deeeab4` esos dos pines son `CAM_C_PIN`/`CAM_D_PIN`, y `botonAceptar()`/`botonCancelar()` no tienen sujeto: `$ grep -n "bool botonAceptar" 01_Firmware/Maestro/src/botones.cpp` → `539:bool botonAceptar() { return false; }`. **El nombre del cobre no se cambia** —el `.kicad_pcb` dice `/Boton3`— pero **quien cablee tiene que leer la tercera columna, no la segunda.** Cada posición lleva su antirrebote en placa (`R65`–`R68` + `C26`–`C29`).
> **`J16` es el único conector de señal que trae 12 V** (pin 1), y **su p1 se tapa en cada equipo que se monte** — `DECISIONES.md` `D-4`, N-120. No es cautela de banco.

### `J2` — SWD

`PinHeader_1x04_P2.54mm_Vertical`.

| pin | red |
|---|---|
| 1 | `GND` |
| 2 | `PA14` (`U1` p37) — `SWCLK` |
| 3 | `PA13` (`U1` p34) — `SWDIO` |
| 4 | `3,3 V` |

🟦 **Confirmado sobre el cobre:** `J2` p2 → `U1` p37 y `J2` p3 → `U1` p34, dos pads por red y nada
en serie; p1 y p4 caen en los nudos generales de `GND` y 3,3 V.

> 🔴 **`J2` es la ÚNICA vía de carga de firmware de esta tarjeta.** No hay USB, no hay puerto de
> *bootloader* serie cableado. **No se reutiliza para nada**, por muy tentadores que sean sus cuatro
> pines: sin `J2` la placa se queda sin forma de recibir una actualización, y recuperarla exige
> soldar sobre las patas del `U1`.

---

## 7.bis 🟦 `J16` TRAZADO SOBRE EL COBRE — ~~es donde **van a ir** las cámaras~~ → **YA ESTÁN AHÍ** (banco 03/09, paso 21)

Todo lo de esta sección es nivel **PCB**: sale de seguir las 1.447 pistas del `.kicad_pcb`, no de
leer el esquemático. **Ninguna fila está medida con multímetro.**

**Cómo se trazó, que es la parte reutilizable:** no se leyó la netlist para decidir —eso solo diría
lo que KiCad *cree*—. Se construyó el grafo geométrico **pad ↔ segmento ↔ vía**, uniendo extremos que
coinciden en la misma capa y cerrando capas por las vías, y se sacaron las componentes conexas. La
netlist se usó solo para **contrastar**: el instrumento se da por bueno porque **ninguna de las
componentes de cobre halladas contiene dos redes distintas** —serían decenas si el transformado de
coordenadas estuviera mal— y porque **459 de los 485 pads tocan cobre trazado**.

### 1. Cada posición de `J16` llega a su pin del micro, y no hay nada en serie

| `J16` | red del PCB | llega a | GPIO — **qué es HOY** | pistas | vías | **todo el cobre de esa red** |
|---|---|---|---|---|---|---|
| **p5** | `/Boton1` | **`U1` p46** ✅ | `PB9` — **mando `A`** | 14 | 3 | `J16`.5 · `U1`.46 · `R65`.2 · `C26`.1 |
| **p8** | `/Boton2` | **`U1` p26** ✅ | `PB13` — **mando `B`** | 20 | 1 | `J16`.8 · `U1`.26 · `R66`.2 · `C27`.1 |
| **p10** | `/Boton3` | **`U1` p27** ✅ | `PB14` — 🆕 **`CAM_C_PIN`, la cámara** | 26 | 2 | `J16`.10 · `U1`.27 · `R67`.2 · `C28`.1 |
| **p12** | `/Boton4` | **`U1` p28** ✅ | `PB15` — 🆕 **`CAM_D_PIN`, vacío** | 31 | 3 | `J16`.12 · `U1`.28 · `R68`.2 · `C29`.1 |

> ✅ **"No hay nada en medio" se confirma, y conviene decir en qué sentido.** En cada red hay
> exactamente cuatro pads. Dos son los extremos (`J16` y `U1`); los otros dos son `R65` y `C26`, y
> **los dos cuelgan de lado, no en serie**: su otro extremo está en `GND` (`R65`.1 = `GND`,
> `C26`.2 = `GND`). Es un antirrebote en paralelo. **Entre la bornera y la pata del micro no hay
> resistencia en serie, ni divisor, ni diodo, ni opto**: la señal del conector llega al GPIO por
> cobre continuo. Cualquier tensión que entre por `J16` p5 **llega entera a `PB9`**.

### 2. La distancia real entre los 12 V y las señales

El esquemático solo permitía estimar "~10 mm por el paso del conector". El PCB tiene las
coordenadas, y **la estimación era correcta para los pads y engañosa para el cobre**.

**Paso del conector: 2,540 mm exactos.** Pad de `J16`: 1,74 × 2,19 mm.

| desde `J16` p1 (12 V) | centro a centro | borde de pad a borde de pad |
|---|---|---|
| p2 `GND` | 2,540 mm | **0,800 mm** |
| p4 `3,3 V` | 7,620 mm | 5,880 mm |
| **p5 `Boton1`** | **10,160 mm** | **8,420 mm** |
| p8 `Boton2` | 17,780 mm | 16,040 mm |
| p10 `Boton3` | 22,860 mm | 21,120 mm |
| p12 `Boton4` | 27,940 mm | 26,200 mm |

> 🟠 **Pero la distancia entre pads NO es la separación entre los 12 V y la señal.** Los 12 V son
> una **red**, no un pad: salen de `J16` p1 y recorren la placa hasta las diez borneras de potencia
> con pistas de hasta **1,0 mm** de ancho. Medida de cobre a cobre —pads, pistas y vías, respetando
> capas—, la red de 12 V se acerca a las de botón **mucho más que los 10 mm del conector**:

| red de 12 V contra | separación mínima real | dónde |
|---|---|---|
| `/Boton1` | **1,405 mm** | pista `B.Cu` 1,0 mm ↔ pad `J16`.5 |
| `/Boton2` | **1,408 mm** | pista `B.Cu` 1,0 mm ↔ pad `J16`.8 |
| `/Boton3` | 4,269 mm | pista `B.Cu` 1,0 mm ↔ pad `J16`.10 |
| **`/Boton4`** | **1,359 mm** ← el peor | vía ↔ pista `F.Cu` |

**Lo que hay que llevarse para las cámaras:** el margen real contra los 12 V no es de 10 mm sino de
**1,36 mm**, y eso es cobre de diseño, sin contar la tolerancia de fábrica ni la suciedad ni la
humedad de un armario en la calle. Un error de una posición al enchufar `J16` **mete 12 V en un pin
de 3,3 V** sin ninguna protección en medio (§7.bis.1: no hay nada en serie).

> 📐 Como referencia, la separación mínima de la red de 12 V a **cualquier** otra red de la placa es
> de **0,099 mm** (pad `R28`.2 contra una pista de `GND`), y **ninguna pareja de pistas** de redes
> distintas baja de 0,200 mm en toda la tarjeta.

### 3. ✅ `R65`–`R68` van a **GND**: son pull-DOWN — ~~y el firmware dice lo contrario~~ **CERRADO (M3 / N-118)**

**Ésta era la contradicción de polaridad. El PCB la cerró en la dirección del esquemático, el multímetro la confirmó (`M3`, 03/09) y `346ea5f` alineó el firmware. Hoy NO hay contradicción.**

| | `R65` | `R66` | `R67` | `R68` |
|---|---|---|---|---|
| valor (en el PCB) | 10 kΩ | 10 kΩ | 10 kΩ | 10 kΩ |
| **pin 1** | **`GND`** | **`GND`** | **`GND`** | **`GND`** |
| pin 2 | `/Boton1` | `/Boton2` | `/Boton3` | `/Boton4` |

**No hay ni una resistencia de `J16` a 3,3 V.** Las cuatro van a masa, y sus condensadores
(`C26`–`C29`, 100 nF) también. La netlist que se trazó es la del esquemático: **pull-DOWN, entrada
activa en ALTO**.

**Y el reparto de pines del conector lo confirma por su cuenta:** cada pin de botón viene precedido
de un pin de 3,3 V —p4/p5, p7/p8, p9/p10, p11/p12—, que es el patrón de un pulsador que **cierra
3,3 V contra la señal**. Hay un solo pin de `GND` en todo `J16` (p2), no uno por botón.

> 🔴 ~~**El firmware hace lo contrario.**~~ → ✅ **YA NO. Se tacha y no se borra.** Hasta `346ea5f`
> (`fix(N-118): el mando A/B pasa a activo en ALTO - lo decide el cobre`) este documento tenía razón
> y el firmware estaba mal. **Hoy las dos puntas leen lo que el cobre pide**, y se cita el símbolo,
> no la línea:
>
> ```
> $ grep -rn "INPUT_PULLUP" 01_Firmware/Maestro/src/botones.cpp 01_Firmware/Esclavo/src/botones.cpp
> 01_Firmware/Maestro/src/botones.cpp:26:  // Aqui ponia `== LOW` con los pines en INPUT_PULLUP, y eso llevaba mal desde el primer
> 01_Firmware/Esclavo/src/botones.cpp:42:  // Aqui ponia `== LOW` con los pines en INPUT_PULLUP, y eso llevaba mal desde el primer
> ```
>
> **Las dos únicas apariciones que quedan son COMENTARIOS que cuentan el defecto ya arreglado.** Lo
> que se ejecuta hoy, medido el 05/09 por símbolo:
>
> ```
> $ grep -n "pinMode(BOTON1\|pinMode(CAM_C_PIN\|lecturaCruda" 01_Firmware/Maestro/src/botones.cpp
> 42:  bool lecturaCruda = (digitalRead(b.pin) == HIGH);
> 394:  pinMode(BOTON1, INPUT);
> 411:  pinMode(CAM_C_PIN, INPUT);
> ```
>
> `INPUT` **pelado** y **activo en ALTO**, en las cuatro posiciones y sin excepción — el reposo lo fija
> el pull-down de la placa, que es para lo que está. **Lo mismo en el Esclavo** (`:412` / `:428`, lectura
> en `:56`).
>
> ### ✅ Y la cuenta de este documento se comprobó con puntas — acertó, y de una forma que enseña
>
> La predicción que estaba escrita aquí era: *el pull-up interno (30–50 kΩ) contra los 10 kΩ a masa deja
> el pin a* **0,83 / 0,66 / 0,55 V**, *por debajo del* `V_IL` *de 0,3·VDD ≈ 0,99 V, **así que en reposo el
> pin lee LOW y el firmware viejo lo cree "pulsado", los cuatro, permanentemente**.*
>
> **Medido en el paso 20 de la Guía de banco, 03/09, con `617bd00` dentro de la tarjeta:**
>
> | | `R` a masa medida | tensión en reposo | `pinMode` de ese binario |
> |---|---|---|---|
> | `MANDO_A` (p5) · `MANDO_B` (p8) | **9,92–9,94 kΩ** | **0,6 V** | `INPUT_PULLUP` |
> | `CAM_C` (p10) · `CAM_D` (p12) | **9,92–9,94 kΩ** | **0 V** | `INPUT` pelado |
>
> **Mismo cobre, distinto `pinMode`, distinta tensión** — las dos ramas del experimento en la misma
> tabla. Los `0,6 V` no eran un defecto de placa: eran **exactamente la cuenta de arriba**, y su ausencia
> en p10/p12 es el control negativo que nadie pidió. `R65`–`R68` **están montadas de verdad** en la
> tarjeta física, que era la primera de las dos cosas que el `.kicad_pcb` no podía decir.
>
> **Y la segunda se cerró en el paso 21:** `p10` cableado contra `p11` (3,3 V) en normalmente abierto
> movió `CAM_C_PIN` **sin una sola demanda fantasma**. El gesto que pide este conector es **cerrar
> contra los 3,3 V**, no contra masa — coherente con que haya **un solo pin de `GND` en todo `J16`**
> (p2, arriba): un contacto por posición contra masa necesitaría una masa por posición, y no la hay.
>
> ⚠️ **Lo que esto NO cierra:** que un pin en `INPUT` pelado y **sin nada enchufado** queda a merced del
> pull-down y de nada más. Eso está bien —`0 V` medidos— pero significa que **la protección contra el
> ruido es esa resistencia y solo esa**: entre la bornera y la pata del micro no hay nada en serie
> (§7.bis.1), ni clamp, ni opto.
>
> Nótese que `botones.cpp` lleva un tratamiento explícito (N-26) para *"un botón ya pulsado al
> encender"*. **Con el firmware viejo ese caso saltaba siempre**; con el de hoy es lo que debe ser, una
> siembra de flanco al arrancar.

### 4. ✅ `J16` p4/p7/p9/p11 y `J14` p2 son el mismo nudo de 3,3 V

**Sí, y no por igualdad de nombre de red: por cobre continuo.** Los cinco pads caen en la misma
componente conexa, junto con:

```
J16 p4, p7, p9, p11        J14 p2        J17 p6, p8        J2 p4
U1 p9, p24, p36, p48 (VDD)               U5 p2 (salida del LM1117)      R5 p1 (-> VBAT)
U2 p8, U3 p8 (VCC de los DOS MAX3485)    R1.1  R7.2  R9.2  R18.2
C1 C2 C3 C4 C10 C11 C15 (desacoplo)
```

98 pistas y 1 vía. **Un solo nudo de 3,3 V en toda la tarjeta**, alimentado por `U5` y repartido a
los cuatro conectores de señal. No hay raíles de 3,3 V separados ni ferrita en medio: lo que cargue
`J16` lo notan `J14`, `J17`, `J2`, **los dos transceptores RS-485** y el propio micro.

> 🔴 **Que los MAX3485 cuelguen de este mismo nudo es lo que sube el listón.** No es un raíl
> auxiliar de sensores: es el que alimenta al STM32 **y a las dos radios**. Un cortocircuito o un
> consumo excesivo en un pin de 3,3 V de `J16` —una cámara mal conectada, por ejemplo— **no deja
> a oscuras solo a la cámara: puede tumbar el micro y los dos enlaces a la vez.**

> **Consecuencia para las cámaras:** los cuatro pines de 3,3 V de `J16` **no son cuatro fuentes**,
> son cuatro tomas del mismo nudo, y ese nudo alimenta también al STM32. El `LM1117DT-3.3` tiene
> margen de sobra, pero **cuánta corriente puede sacarse por `J16` sin hundir el micro es una
> pregunta de banco**, no de este fichero: depende del disipado real de `U5` en el armario.

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
| **28/08 → 28/08** | 🔴 *"el `.kicad_pcb` **está vacío**"*, y de ahí *"no existe ningún artefacto que ate el esquemático a la tarjeta"* | **falso: 2.158.421 B, 185 huellas, 1.447 pistas, 89 vías, 485 pads.** Los vacíos (78 B) son los de `99_Legacy\...-backups\` | **§0** |
| **28/08 → 28/08** | `J17` = *"el conector de la LCD"*, a secas | **también es por donde entra el ESP32** desde N-76: p2 = `PB7` (RX), p3 = `PB6` (TX) | §2, §6, §6.bis, §7 |
| **28/08 → 28/08** | separación 12 V ↔ señal en `J16`: *"~10 mm por el paso del conector"* (estimado) | **cierto entre pads (10,160 mm), engañoso en cobre: la red de 12 V se acerca a 1,359 mm** | §7.bis.2 |
| **28/08 → 28/08** | §3 no listaba `R64`–`R68` | son los pull-**DOWN** de los botones y de la cámara, **a `GND`** | §3, §7.bis.3 |
| **28/08 → 28/08** | 🟠 *"5 V (RS-485 y optos)"* | **los MAX3485 van a 3,3 V**: `U2` p8 y `U3` p8 están en el nudo del STM32. Los 5 V solo tocan los diez optos, `U4`, `U5`, `C13`/`C14` y `R17` | §2, §7.bis.4 |
| 28/08 → **05/09** | 🔴 *"el firmware lee los botones con `INPUT_PULLUP` + activo en BAJO — **contradicción abierta**"* | **ERA CIERTO Y YA NO LO ES.** `346ea5f` (N-118) puso las dos puntas en `INPUT` pelado + `== HIGH`. Las dos únicas apariciones de `INPUT_PULLUP` que quedan en `botones.cpp` son **comentarios que cuentan el defecto arreglado** | §3, §6, §7.bis.3 |
| 28/08 → **05/09** | 🔴 *"la columna 🔬 está vacía entera: nadie ha puesto una punta en esta tarjeta"* | **`M3` la puso el 03/09** (pasos 20 y 21 de la Guía de banco): `R65`–`R68` medidas en **9,92–9,94 kΩ**, y el paso 21 movió `CAM_C_PIN` cerrando p10 contra p11. **`J16` está a nivel 🔬**; el resto de la tarjeta no | §0 |
| 28/08 → **05/09** | 🔴 `J16` p10/p12 llamados ***"`Boton3`" y "`Boton4`"*** en §6, §7 y §7.bis, y §7.bis titulada *"donde **van a ir** las cámaras"*, **en futuro** | **`deeeab4` los renombró a `CAM_C_PIN`/`CAM_D_PIN`** y `botonAceptar()`/`botonCancelar()` son `return false;`. **Ya no es futuro: `M3` cerró el 03/09 y `p10` se cableó y verificó en banco el mismo día.** `/Boton3` y `/Boton4` **siguen siendo los nombres de red del cobre** y por eso no se tocan — pero **quien cablea lee la función, no la red** | §6, §7, §7.bis.1 |
| 28/08 → **05/09** | *"`J16` — **botones** y Mando RF"*, sin decir qué es cada posición | **p5/p8 = mando `A`/`B`** (hardware **retirado** el 05/09, `DECISIONES.md` `D-1`; **el código se queda** y **sigue leyendo esos pines**) · **p10 = la cámara** · **p12 = cámara, vacío**. Qué se cablea en p5/p8 es **`A-2`, abierto** | §6, §7 |

**Cómo se midió, que es la parte reutilizable:** no se leyó el dibujo — se **trazó la conectividad**.
Se parsea el `.kicad_sch`, se sitúan los pines de cada símbolo aplicando su rotación y espejo, se unen
por los segmentos de `wire`, se cierran las etiquetas del mismo nombre y se sacan las componentes
conexas. **Leer el esquemático "de arriba abajo" es justo lo que produjo el error de los MAX3485:**
los dos bloques son idénticos y están uno sobre otro, así que la posición en la hoja no dice nada. La
única forma de acertar es seguir la red hasta el pin del `U1`.

**Y lo mismo sobre el `.kicad_pcb`, con dos trampas propias que costaron las dos conclusiones:**

1. **El buscador.** `grep -c '(segment '` da **0** porque el fichero separa tokens con tabulador y
   salto de línea. Hay que usar `grep -oE '\(segment\b'`, que da **1447**. Un cero de un buscador
   roto se lee como "vacío", y así nació *"el PCB está vacío"*.
2. **El ángulo de los pads.** KiCad guarda el ángulo de cada pad **ya en absoluto**, con la rotación
   del footprint dentro. Sumarle otra vez la del footprint lo gira 90° de más. Con el ángulo mal,
   este trazado *"encontró"* un **cortocircuito de 25 µm entre una pista de 12 V y el pin de `GND`
   de `J17`**. No existe: con el ángulo correcto la holgura es de **+0,200 mm**, y ninguna pareja de
   pistas de la placa baja de 0,200 mm. **Se comprobó antes de escribirlo, y por eso no está
   escrito como hallazgo.**

   *Cómo se zanjó, sin suponer:* se censó el fichero entero —**481 de 485 pads tienen
   `pad_rot == fp_rot`**, y las 4 excepciones son las de `SW1`, cuyo pad de librería lleva 180°
   propios—. Un footprint a 90° con pads a 90° y otro idéntico a −90° con pads a 270° solo es
   posible si el ángulo guardado es absoluto.

**Validación del trazador, que es lo que permite fiarse de §7.bis:** el grafo se construye por
geometría (pad ↔ segmento ↔ vía) y **ninguna componente conexa contiene dos redes distintas** — si
el transformado de coordenadas estuviera mal, aparecerían decenas.

**Lo que sigue abierto, y no se cierra desde un fichero:**

1. **El nombre real del pin 3 de `J17`** (`RS(A0)` en el esquema contra `PSB` en el firmware) —
   §6.bis. **El PCB no lo cierra**: el nombre en disputa es el de la pata del display, al otro lado
   del conector.
2. ~~🔴 **La polaridad de los botones** — §7.bis.3.~~ → ✅ **CERRADA EL 03/09 (`M3` / N-118).** El
   multímetro dio la razón al PCB —pull-DOWN real de 10 kΩ en las cuatro posiciones— y `346ea5f`
   alineó el firmware: **`INPUT` pelado, activo en ALTO, las cuatro**. Se tacha y no se borra.
3. **Todo lo demás de este documento**, en el sentido de §0. Buena parte subió el 28/08 de 📐 a 🟦
   —**lo que se mandó fabricar**— y el 03/09 **`J16` subió a 🔬**. **El resto de la columna 🔬 sigue
   vacío**: `J17`, `J2`, `J10`/`J12`, las diez cadenas de potencia y el nudo de 3,3 V **no los ha
   tocado una punta**. Quien tenga la tarjeta delante y un multímetro puede convertir más filas, y
   conviene que lo anote **fila a fila en la tabla de §0**.
4. 🔴 **NUEVO, y no se cierra desde un fichero: `J8` p2 (`VERDE2`) flota en reposo, y sólo ése.** El
   LED `D21` de ese canal tiene el **cátodo sin conectar** —red `unconnected-(D21-K-Pad1)`, cero
   pistas y cero hilos, mientras su gemelo `D23` los tiene en los dos extremos—. Los otros **nueve**
   drenadores suben a ~12 V por su pull-up de 1 kΩ + LED; **éste no**, y además ese canal **no tiene
   testigo luminoso**. Queda `SIN VERIFICAR` si es defecto de diseño o decisión. Es `A-10` de
   `DECISIONES.md`, y **es comprobación de banco**, no de firmware.
