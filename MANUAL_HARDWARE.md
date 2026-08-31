# ⚙️ Manual de Hardware y Topología Física (V7.6 Definitiva)

Este documento describe la arquitectura física de las placas impresas (PCBs), los componentes electrónicos, y la topología de red utilizada en el ecosistema de semáforos móviles.

**Última revisión:** 31 de agosto de 2026 — **el apartado 3 (cámaras) estaba MAL y se ha corregido.**
Llamaba a `PB8` *«Entrada Libre»* y mandaba enchufarle una cámara: `PB8` es el **`LED_TESTIGO`**, una
salida por `R16` 1 kΩ al LED `D5`. Y mandaba las cuatro cámaras contra `GND` cuando la entrada es
**activa en ALTO**. Es `N-105` de [`roadmap.md`](roadmap.md). El texto viejo queda **tachado en su
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
> dónde enchufar un contacto seco. El firmware lo deja **en alta impedancia a propósito**
> (`01_Firmware/Maestro/src/modo_inteligente.cpp:50`): con `INPUT_PULLUP` se le colarían ~40 µA y
> quedaría un testigo encendido a medias que nadie sabría explicar.
>
> **Y el `GND` de las cuatro líneas también estaba mal.** La entrada de cámara es
> **activa en ALTO** (`modo_inteligente.cpp:25`, `:46`) y la bornera saca el pin **junto a 3,3 V**:
> cableada contra masa, **la cámara no dispara nunca**. Es `N-105`; ver el detalle y las dos
> configuraciones de la salida de la cámara en [`MANUAL_USUARIO.md`](MANUAL_USUARIO.md) §6.3.
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

| pin | qué es de verdad | nivel |
|---|---|---|
| **`PB0`** | **`CAM_DEMANDA_PIN`** — la **única** entrada de cámara con firmware que la lee hoy. Bornera **`J14`**, con `R64` 10 kΩ (*pull-**down***) + `C25` 100 nF = antirrebote por hardware de 1 ms | ✅ **MEDIDO** (`pines.h:43-46`; se lee en `modo_inteligente.cpp:98`, `:136` y `Esclavo/src/main.cpp:350`) |
| **`PB8`** | **`LED_TESTIGO`** — salida por `R16` 1 kΩ al LED `D5`. **No es entrada de nada** | ✅ **MEDIDO** (`pines.h:63`) |
| **`PB9`** (`J16` p5) | **`BOTON1` = `MANDO_A`** del mando de relés. **Se conserva** (`N-104`) | ✅ **MEDIDO** (`pines.h:92`, `botones.cpp:119`) |
| **`PB13`** (`J16` p8) | **`BOTON2` = `MANDO_B`**. **Se conserva** — es el único canal que arma `ambarLocal` | ✅ **MEDIDO** (`pines.h:93`, `botones.cpp:120`, `Esclavo/src/mando.cpp:129-132`) |
| **`PB14`** (`J16` **p10**) | Hoy `botonAceptar()`. **Destino decidido de cámara** tras la Fase 3 | ✅ **MEDIDO** hoy (`pines.h:94`, `botones.cpp:131`) · 📖 **LEÍDO** el destino (`ESTADO.md:83`) |
| **`PB15`** (`J16` **p12**) | Hoy `botonCancelar()`. **Destino decidido de cámara** tras la Fase 3 | ✅ **MEDIDO** hoy (`pines.h:95`, `botones.cpp:132`) · 📖 **LEÍDO** el destino (`ESTADO.md:83`) |

> 🔴 **`PB9`/`PB13` no son un destino posible para una cámara, ni siquiera «provisionalmente».** Son
> los dos canales del mando: tres pulsos dentro de la ventana de **12 s** (`mando.cpp:38`) componen
> una secuencia —`A·A·A` = Modo Automático, `B·B·B` = Ámbar y armado de `ambarLocal`—, así que **el
> tráfico cambiaría el modo del semáforo solo**. Detalle completo en `MANUAL_USUARIO.md` §6.

### 3.3 🛑 Bloqueos vigentes antes de cablear cámara a `J16`

**No se cablea todavía.** Las cuatro condiciones están enumeradas en `MANUAL_USUARIO.md` §6.5; en
resumen y con su medida:

1. 🔴 **Falta la medida M3** — la polaridad de los pines de botón está en **contradicción medida**:
   el `.kicad_pcb` traza `R65`–`R68` de 10 kΩ **a `GND`** (pull-**down**, activo en ALTO) y
   `botones.cpp:19` lee `== LOW` con `INPUT_PULLUP`. Las dos no pueden ser ciertas.
   `03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md` §3 · `05_Funcional/17_…` sección **M3**.
   **`PB0` tiene su resistencia a masa MEDIDA; de `PB14`/`PB15` sólo lo dice el netlist** — y con
   `pinMode(INPUT)` pelado, un pin sin resistencia real **flota** y el ruido dispara demandas fantasma.
2. 🔴 **Orden asimétrico** (`CLAUDE.md` §9.bis): `PB14` es **`botonAceptar()`, el que EJECUTA**, leído
   hoy activo en BAJO. **El firmware nuevo tiene que estar CARGADO EN LA TARJETA antes de que nadie
   enchufe nada en `J16`.** Un commit no protege de un destornillador.
3. 🔴 **`J16` p1 lleva 12 V crudos** —sin opto, sin limitadora, sin clamp— y **se tapa físicamente
   antes de cablear**. La separación **real sobre cobre** contra la red de 12 V, **MEDIDA** en
   `03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md:576-588`, no es el paso del conector:

   | red de 12 V contra | separación mínima real |
   |---|---|
   | `/Boton1` (p5) | 1,405 mm |
   | `/Boton2` (p8) | 1,408 mm |
   | `/Boton3` (**p10**) | **4,269 mm** |
   | `/Boton4` (**p12**) | **1,359 mm** ← el peor |

   👉 **Si una de las dos cámaras es más crítica, va en `p10`**, no en `p12`.
4. 🔴 **Ningún firmware lee `PB14`/`PB15` como cámara todavía**: siguen en `pinMode(…, INPUT_PULLUP)`
   como botones (`botones.cpp:52-53`).

Detalle de parametrización de la cámara —incluidas las **dos configuraciones `NO`/`NC`** de la salida
de relé y cuál elegir según **M3**— en
[`05_Funcional/9_Manual_Parametrizacion_Camara_IA.md`](05_Funcional/9_Manual_Parametrizacion_Camara_IA.md).

> **Nada de este apartado ha pasado prueba de banco y no autoriza a instalar ni a cablear nada.**

---

## 4. Módulo Bluetooth para Telemetría y Diagnóstico Móvil

Para soporte técnico, caja negra de alarmas y monitoreo desde el suelo sin subir al poste (estándar probado en proyecto Baliza):
* **Pines de Conexión:** Puerto USART1 del STM32 (`PA9` TX ➔ `RXD` BT, `PA10` RX ➔ `TXD` BT).
* **Desacoplo Eléctrico:** Pin `PA8` forzado en `HIGH` permanente para poner en alta impedancia ($\text{Hi-Z}$) el transceptor `MAX3485 U3`.
* **Alimentación:** `5V` (o `3.3V`) y `GND` de la tarjeta controladora.
* **Funcionalidad:** Envío continuo de telemetría (estado de luces, modo, cuenta regresiva `T:`, calidad de señal RF, % de paquetes y motivos de alarma con hora exacta de RTC) hacia la App en celular con comandos protegidos por PIN (`1234`). Detalle completo en [`05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.md`](05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.md).

---

## 5. Placa Base (Mainboard STM32)

La tarjeta principal de control que gobierna las luces del semáforo.

### Componentes Clave:
- **Cerebro:** Microcontrolador **STM32F103C8T6** ("Blue Pill" / CKS32F103).
- **Pantalla y UI:** LCD ST7920 (128x64 píxeles) conectada por SPI de 3 hilos. 4 botones físicos de navegación.
- **Salidas de Potencia:** Transistores MOSFET N-Channel (**IRLZ44N**) a 12V/24V para lámparas LED (Rojo, Amarillo, Verde).
- **Protección Galvánica:** 9 Entradas Optoacopladas para aislamiento eléctrico de botoneras y sensores.
- **Transceptor Integrado:** Chip MAX485 (Half-Duplex) conectado a la bornera RS485 `A` / `B`.
- **Watchdog:** Inicialización IWDG por registros directos adaptada a CKS32F103 (refresco en `loop()`).

---

## 6. Placa Repetidora (ESP32 V7.6)

Ubicada en la carpeta [`01_Firmware/Repetidor`](file:///d:/@Proyect/Controladora_Semaforos/01_Firmware/Repetidor).

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
