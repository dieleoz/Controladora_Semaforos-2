# MANUAL DE HARDWARE Y PRUEBAS (Ecosistema Semáforos - V9.0)

Este documento contiene las instrucciones paso a paso para el personal funcional encargado de ensamblar, probar y actualizar el firmware de los semáforos en campo o laboratorio.

## 📌 HARDWARE COMPLETO Y HOMOLOGADO (V9.0)

| Elemento | Estado | Ubicación / Conexión |
|---|---|---|
| **Pila `CR2032` en `VBAT`** | ✅ **Instalada en AMBAS tarjetas** (Maestro y Esclavo) | Alimentación directa RTC (R5 retirado). Ver §5 |
| **Pantalla LCD ST7920** | 🛑 **SE RETIRA (28/08/2026)** | No se lee desde el suelo. Sus pines `PB6`/`PB7` (conector `J17`) pasan al módulo Bluetooth. Ver §3 y §8 |
| **Botonera de 4 pulsadores** | ✅ **Se queda en AMBOS** | Botones 1 a 4 (`PB9`, `PB13`, `PB14`, `PB15`) por el conector **`J16`**. ⚠️ Siguen navegando un menú **que ya nadie ve** — ver §3 |
| **Cámaras IA de demanda (1 y 3)** | ✅ **1 en Maestro + 1 en Esclavo** | Contacto seco `1A`/`1B` en `PB0`. Operativas. Ver §7 |
| **Cámaras IA de umbral (2 y 4)** | 🛑 **NO SE INSTALAN** | **No hay entrada donde conectarlas:** `PB8` alimenta el LED testigo `D5` (`R16` 1 kΩ), no es bornera. Harían falta un hilo y un comando de radio. Ver Manual 9 y N-64 |
| **Módulo Bluetooth Telemetría** | ✅ **En Maestro y Esclavo (Baliza)** — **es la interfaz del equipo** | Conector **`J17`** p2/p3 = `PB7`/`PB6` (`USART1` **remapeado**), p6 = 3,3 V, p7 = GND. ⛔ **`J16` NO es `J17`: lleva 12 V.** Ver §8 |
| **Mando de 4 Relés Anti-Colisión** | ✅ **Secuencias Seguras A·B·A...** | Cableado en paralelo con `PB9`..`PB15`. Ver §6 |

> 📱 **El Módulo Bluetooth en el Esclavo resuelve la operación desde el suelo:** Gracias al módulo Bluetooth de diagnóstico estándar Baliza instalado en el Esclavo (`USART1` por `J17`), el operario puede consultar el estado, ver alarmas y operar el Esclavo desde el celular sin necesidad de subir al poste a 5 metros de altura.

---

> ## 🔴 CAMBIO DEL 28/08/2026 — LA PANTALLA SE RETIRA Y EL BLUETOOTH OCUPA SUS PINES
>
> **Por qué:** el equipo va montado en alto. Una LCD dentro del gabinete, a 5 m, **no se lee desde
> el suelo**, así que la interfaz de operación pasa a ser **la app por Bluetooth**.
>
> **Los tres cambios de firmware que lo hacen posible, ya aplicados en las DOS puntas y verificados
> leyendo el fuente el 28/08:**
>
> | fichero | qué cambió | por qué |
> |---|---|---|
> | `lcd.cpp` | el constructor de U8g2 recibe **`U8X8_PIN_NONE`** en lugar de `LCD_RST`, y `lcd_setup()` **ya no hace `pinMode`/`digitalWrite` sobre `LCD_PSB`** | así se sueltan **`PB6` y `PB7`**. Ninguno de los dos llevaba datos: `PSB` era un nivel estático y `RST` un pulso de arranque |
> | `bluetooth.cpp` | **`SerialBT(PB7, PB6)`** | el módulo entra por `J17`, que es **enchufable**. `PA9`/`PA10` no salen a ninguna bornera |
> | `protocolo.cpp` | `protocolo_setup()` **ya no llama a `AiBus.begin(115200)`** ni toca `RS485_IN_DE_RE` | `AiBus` estaba declarado sobre **el mismo `USART1`** que `SerialBT`, a 115200 contra 9600. Funcionaba **por accidente de orden**: `bluetooth_setup()` corre después y ganaba, así que el puerto quedaba a 9600 y *«el puerto IA» jamás existió a 115200* |
>
> ⚠️ **`AiBus` y `protocolo_actualizarAI()` NO se han borrado.** Esa función **no la llama nadie** en
> ninguna de las dos puntas —lleva huérfana desde siempre—, y retirarla es un cambio con sentido
> propio: irá en su propio `N-x`, no colado dentro de éste.
>
> ### ⚠️ La pantalla se va; el menú NO se ha ido del binario
>
> **Medido el 28/08:** `lcd.cpp`, `menu.cpp` y `modo_hora.cpp` siguen compilándose, `lcd_setup()`
> sigue llamando a `u8g2.begin()`, y **los cuatro botones de `J16` siguen navegando el menú**. Lo
> único que falta es el display.
>
> **Consecuencia para quien monte o mantenga el equipo:** pulsar esos botones —o accionar el mando
> de relés, que va en paralelo con ellos— **mueve un menú a ciegas**, y con los pulsos suficientes se
> llega a `AJUSTAR HORA` y **se confirma una hora que el equipo dará por válida**. **No se accionan
> los botones del gabinete para «probar».**

---

> ## ⚠️ ANTES DE PROBAR: reconfigurar las radios a `2.4 kbps`
>
> El cableado descrito aquí es correcto y no cambia. Lo que **sí** debe cambiar es la
> **velocidad aérea (Air Data Rate)** de las radios: de `0.3 kbps` a **`2.4 kbps`**, en todas.
> Ver `4_Manual_Configuracion_Radios.md`. Sin ese ajuste el enlace sigue cayendo al paso de ciclo
> y el modo repetidor no funciona.

---

## 1. CABLEADO DE RADIOS INDUSTRIALES RS485 (E90-DTU)

> [!IMPORTANT]
> **ACLARACIÓN CRÍTICA DE CABLEADO:**
> Las radios industriales **E90-DTU (caja metálica)** utilizan la interfaz estándar **RS485 diferencial (2 hilos)**. **NO BUSQUE PINES TTL (DI, RO, DE, RE) EN LA RADIO**. La tarjeta impresa del semáforo (STM32) **ya incluye el integrado transceptor MAX3485 soldado**, por lo que la conexión se realiza mediante **solo 2 cables de datos** a la bornera de la tarjeta.

### Diagrama de Conexión Físico:
```text
+------------------------------------+        2 Hilos RS485       +------------------------------------+
| Tarjeta Controladora Semáforo STM32|                            |     Radio Industrial E90-DTU       |
|    (Chip MAX3485 Integrado)        |                            |         (Caja Metálica RF)         |
|                                    |                            |                                    |
|   Bornera Verde A  ----------------|----------------------------|----> Pin 485_A                     |
|   Bornera Verde B  ----------------|----------------------------|----> Pin 485_B                     |
|                                    |                            |      Pin V+ (12V - 24V DC)         |
|   Bornera GND / V- ----------------|----------------------------|----> Pin V- (Masa común / Tierra)  |
+------------------------------------+                            +------------------------------------+
```

### Reglas de Cableado RS485 (Directo, NO Cruzado):
1. **Bornera `A` de la Tarjeta Semáforo** $\rightarrow$ se conecta al **`Pin 485_A`** de la radio E90-DTU.
2. **Bornera `B` de la Tarjeta Semáforo** $\rightarrow$ se conecta al **`Pin 485_B`** de la radio E90-DTU.
3. **No se cruzan los hilos A y B:** A diferencia del puerto RS232 o UART TTL (donde TX va con RX), el bus RS485 es paralelo y mide voltaje diferencial ($V_A - V_B$). Si invierte los cables (A con B), la radio no recibirá datos.
4. **Sin control de dirección DE/RE externo:** Las radios E90-DTU conmutan la dirección de transmisión y recepción automáticamente por hardware interno. No requiere cablear pines de control hacia la radio.

---

## 2. Topologías de Red Soportadas

> En ambas topologías, **todas** las radios deben quedar con el mismo Air Data Rate de **`2.4 kbps`**,
> potencia `30 dBm` y `FEC: Enable`. Si una sola queda distinta, no enlazará con las demás.

### Opción 1: Modo Directo (2 Radios - Sin Repetidor)
* **Semáforo Maestro:** Tarjeta STM32 conectada por bornera A/B a la Radio 1 — Canal `0` (`170.0 MHz`).
* **Semáforo Esclavo:** Tarjeta STM32 conectada por bornera A/B a la Radio 2 — Canal `0` (`170.0 MHz`).
* **Funcionamiento:** La señal viaja transparente por aire. No requiere la tarjeta Repetidor ESP32.

### Opción 2: Modo Repetidor (4 Radios - Esquinas Ciegas / Montaña)
* **Semáforo Maestro:** Conectado a la Radio 1 — Canal `0` (`170.0 MHz`).
* **Repetidor Central (ESP32):**
  - Radio B1 en Canal `0` (`170.0 MHz`) — habla con el Maestro.
  - Radio B2 en Canal `10` (`172.0 MHz`) — habla con el Esclavo.
  - La tarjeta ESP32 reenvía los datos de forma asíncrona, liberando el bus RS485 tras 5 ms de silencio.
* **Semáforo Esclavo:** Conectado a la Radio 4 — Canal `10` (`172.0 MHz`).
* **Nota:** en esta topología cada mensaje da **dos saltos de aire por sentido**. Es la razón por la que la velocidad aérea de `0.3 kbps` resultaba insuficiente y quedó derogada.

---

## 2.1 📡 Antenas — la pieza que más alcance gana o pierde

> **Prueba de campo del 31/07/2026:** con las antenas originales el alcance era de **1 cuadra,
> esquina y 2 cuadras más**. Para 1 W a 170 MHz eso es muy poco — en línea de vista deberían ser
> kilómetros. La causa casi segura: **antenas que no son de la banda**.

### El error más caro: comprar en el mercado equivocado

Las antenas genéricas vendidas como "antena LoRa" suelen ser de **433 o 915 MHz**. Puestas en un
radio de 170 MHz **rinden pésimo**: se pierden fácilmente 15–20 dB, que es justo la diferencia entre
tres cuadras y varios kilómetros.

**170 MHz cae en la banda VHF comercial (136–174 MHz)** — la misma de los radios de taxi, seguridad
y vigilancia. **Ahí es donde hay que comprar**, en distribuidores de radiocomunicación profesional,
no en tiendas de electrónica de hobby.

### La física manda el tamaño

A 170 MHz la longitud de onda es de **1,76 m**. No hay forma de hacer una antena eficiente y corta:

| Tipo | Ganancia | Longitud |
|---|---|---|
| Látigo ¼ de onda | ~0 dB | **~42 cm** |
| Fibra de vidrio 1 sección, ⅝ de onda | ~3 dB | **~1 m** |
| Colineal fibra de vidrio 2 secciones | 6–7 dB | **~3,2 m** |
| Yagi 5 elementos | 9,2 dB | boom 1,5–2 m |

> ⚠️ **Desconfía de cualquier antena de 15–20 cm vendida como "170 MHz".** Son helicoidales cargadas
> y su rendimiento es una fracción del de un cuarto de onda real.

### Cómo repartirlas en este sistema

| Equipo | Antena recomendada | Por qué |
|---|---|---|
| **Maestro y Esclavo** (móviles) | Fibra de vidrio **1 sección ⅝ de onda, ~1 m** | Se transportan y reubican. Una de 3,2 m es inmanejable en una unidad móvil |
| **Repetidor** (fijo en poste) | **Yagi 5 elementos**, una apuntando a cada extremo | Está fijo: ahí cabe antena grande y es donde más rinde. La directividad además **reduce el acoplamiento entre las dos radios del poste** |

### 📻 Una sola antena cubre 170 y 172 MHz

**No hacen falta dos modelos distintos.** Las dos frecuencias caen dentro de la banda **VHF comercial
136–174 MHz**, y cualquier antena que cubra ese rango sirve para ambas.

> ⚠️ **Al comprar, verificar el rango que declara la antena.** Muchas antenas "VHF" vienen sintonizadas
> solo para **136–144 MHz** (banda de radioaficionado) y **no cubren 170**. Hay que exigir que el rango
> declarado incluya **148–174 MHz** o **136–174 MHz**.

### ⚠️ Los conectores no coinciden — pídelos con la antena

**Lado del radio — dato verificado en el datasheet oficial** (`E90-DTU(230SL37)_UserManual_EN_V1.5`,
tabla de interfaces, punto 8):

> *"Antenna interface — **SMA-K** interface, external thread, 10 mm, **50 Ω** characteristic impedance"*

`SMA-K` es **SMA hembra**: cuerpo con rosca exterior y contacto central hueco. Lo que se le enrosca
debe ser **SMA macho**.

**Lado de la antena:** las antenas profesionales de VHF traen **`UHF hembra (SO-239)`** —lo más
habitual en fibra de vidrio— o **`N hembra`**. **No conectan directo al radio.**

Cadena completa de conexión:

```
   Antena VHF omnidireccional
   └─ UHF hembra (SO-239)
            │
   Cable coaxial RG-8X
   └─ UHF macho (PL-259) en ambos extremos
            │
   Adaptador o pigtail   UHF hembra → SMA macho
            │
   Radio E90-DTU
   └─ SMA-K (hembra, rosca exterior, 50 Ω)
```

Pide siempre junto con las antenas:

- Adaptador o pigtail **UHF/N → SMA macho**
- Cable coaxial **RG-8X** *(pierde menos que el RG-58 en VHF)* **con los conectores ya instalados de
  fábrica** — un conector mal soldado en obra es una de las causas más frecuentes de pérdida
- Todo de **50 Ω**. El coaxial de 75 Ω de televisión **no sirve**

*Si pides las antenas sin esto, llegan y no se pueden instalar.*

> ⚠️ **El dato del conector está verificado sobre el datasheet de la variante `230SL37`.** Toda la
> familia E90-DTU SL usa la misma caja e interfaz, pero **antes de comprar conviene mirar el conector
> del radio físico** — son diez segundos y evita un pedido equivocado.

### Modelos disponibles en Colombia

El sistema usa **170 y 172 MHz**. La columna de la derecha indica si el modelo cubre **las dos**.

| Referencia | Tipo | Banda | Ganancia | ¿170 y 172? |
|---|---|---|---|---|
| `TX-AB-136-74-FG1` (TXPRO) | Fibra, 1 sección ⅝ | 136–174 MHz | ~3 dB | ✅ Sí |
| `TXAB-136-74-FG2` (TXPRO) | Colineal, 3,2 m | 136–174 MHz | 6,7 dB | ✅ Sí |
| `1490` (TRAM Browning) | Colineal fibra | 144–174 MHz | 6,7 dB | ✅ Sí |
| `HX6-160-70` (Hustler) | Fibra, climas extremos | 160–170 MHz | 6 dB | ❌ **NO — se queda en 170** |
| `MYA-1505K` (PCTEL) | **Yagi 5 elementos** | 150–174 MHz | 9,2 dB | ✅ Sí |
| `MYA-1503K` (PCTEL) | Yagi 3 elementos | 150–174 MHz | 7,1 dB | ✅ Sí |

> ⚠️ **La `HX6-160-70` queda descartada para este sistema.** Su rango termina justo en 170 MHz y
> **no llega a 172**. En el borde de banda el acople empeora, la potencia se refleja hacia el radio en
> lugar de radiarse y **el alcance en 172 sería aún peor que hoy**. Se deja en la tabla precisamente
> para que nadie la pida por descuido.

**Proveedores:** SYSCOM Colombia, Sumitec Comunicaciones, PCredcom.
**Pide por la banda `136-174 MHz`**, no por "antena LoRa".

### Lo que ninguna antena resuelve

En ciudad densa **ninguna antena atraviesa edificios**. Lo que salva las esquinas es:

1. **La altura.** Subir la antena 3 m suele rendir más que duplicar la potencia. Es la palanca
   principal a VHF.
2. **La ubicación del repetidor.** No va a mitad de camino: va donde **vea los dos extremos**,
   normalmente arriba del talud o del poste, no en la vía.

### 🔥 Y una advertencia que ya costó un radio

**Transmitir con la antena en mal estado daña el amplificador de potencia.** Con 1 W bastan unas
horas radiando contra un conector suelto, un coaxial roto o una antena de otra banda.

**Antes de energizar un radio nuevo, revisa su coaxial y su conector.** Si lo montas sobre el cable
que causó la avería anterior, quemas también el radio de reemplazo.

---

## 3. Interfaz Local — 🔴 RETIRADA EL 28/08. QUEDA LA BOTONERA, SIN PANTALLA

**La LCD ST7920 (128×64) ya no se instala.** La configuración de tiempos, los modos y la puesta en
hora se hacen **desde la app por Bluetooth** (§8). Lo que sigue se conserva porque **el hardware de
la botonera sigue en la tarjeta y el menú sigue en el firmware**, y quien monte el equipo tiene que
saberlo.

~~La pantalla LCD (128x64) permite configurar los tiempos de la vía mediante 4 pulsadores físicos:~~

* ~~**Botón 1 (Arriba / +):** Sube el valor en la pantalla o cambia la luz en Modo Manual.~~
* ~~**Botón 2 (Abajo / -):** Baja el valor o cambia la luz en Modo Manual.~~
* ~~**Botón 3 (Aceptar / OK):** Confirma la selección.~~
* ~~**Botón 4 (Cancelar / Menú):** Sube un nivel de menú.~~

> ## ⚠️ LOS BOTONES SIGUEN CONECTADOS Y EL MENÚ SIGUE VIVO — A CIEGAS
>
> **Esto no es una nota histórica: es el estado de hoy.** El firmware **no ha perdido el menú**, solo
> ha perdido dónde dibujarlo. Los cuatro pulsadores de `J16` siguen entrando por `PB9`, `PB13`,
> `PB14` y `PB15`, y siguen navegando exactamente el mismo menú de dos niveles de antes.
>
> **Lo que eso permite, sin que nadie lo vea:** llegar a `CONFIGURACION → AJUSTAR HORA` y confirmar
> **una hora cualquiera**, que el equipo dará por buena. Desde ahí, el Modo Degradado entraría sobre
> una hora inventada con las dos puntas desfasadas — que es exactamente el escenario que el diseño
> del reloj existe para evitar.
>
> **Instrucción de montaje:** los botones se dejan cableados (los usa el mando de relés), pero
> **nadie los pulsa para «ver qué pasa»**. Si el gabinete queda accesible, considérese poner el
> cartel correspondiente.

### Pines de la botonera *(sin cambios: siguen ahí)*

| Botón | Pin del STM32 |
|---|---|
| 1 — Arriba / `A` | `PB9` |
| 2 — Abajo / `B` | `PB13` |
| 3 — Aceptar / `C` | `PB14` |
| 4 — Menú / `D` | `PB15` |

**Ambas tarjetas usan los mismos pines.** El mando de 4 relés se cablea **en paralelo con estos cuatro
pulsadores** — no hay entradas dedicadas para él (ver §6).

### Las dos unidades tienen interfaz, pero menús distintos — 📕 HISTÓRICO (hasta el 28/08)

> **Esta tabla describe el equipo CON pantalla.** Se conserva porque el menú sigue en el binario y
> porque su última fila —*por qué el Esclavo no ajusta hora ni modos*— **sigue siendo la regla
> vigente**, ahora aplicada a la app en vez de a la pantalla.

| | Maestro | Esclavo |
|---|---|---|
| LCD y botonera | ~~✅~~ 🛑 sin LCD desde el 28/08; botonera sí | ~~✅~~ 🛑 igual |
| Menú | 2 niveles: `MANUAL` / `AUTOMATICO` / `INTELIGENTE` / `CONFIGURACION`, y dentro `PRUEBA ALCANCE` / `AJUSTAR HORA` / `MODO DEGRADADO` | 2 opciones: `ESTADO` / `MODO DEGRADADO` |
| Ajuste de hora | ✅ Única vía para poner el reloj | ❌ **A propósito** — la hora llega por radio |
| Modos de operación | ✅ | ❌ **A propósito** — quien manda el ciclo es el Maestro |

> **El Esclavo no ofrece ajuste de hora ni modos de operación, y no es un olvido.** Ajustar la hora en
> las dos puntas a mano deja hasta 59 s de desfase que dos pantallas no pueden detectar; y ofrecer los
> modos en el Esclavo invita a dejar las dos unidades peleando por el ciclo.

### Verificación del Modo Legal (Norma Colombia Res. 2024 - V8.0)
1. Ingrese a **Modo Manual**, **Automático** o **Inteligente**.
2. Al ordenar el paso a Verde:
   - Focos físicos: El estado pasa de **Rojo** $\rightarrow$ **Amarillo Fijo (4.0s)** $\rightarrow$ **Verde** (en Maestro y Esclavo).
3. Al ordenar el paso a Rojo:
   - Focos físicos: El estado pasa de **Verde** $\rightarrow$ **Rojo** DIRECTO (0s de Amarillo).

---

## 4. INSTRUCCIONES PARA ACTUALIZAR EL FIRMWARE (VSCode + PlatformIO)

Siga estos pasos al pie de la letra para compilar y flashear el firmware en las tarjetas.

**Paso 1: Abrir la carpeta específica en VSCode**
1. Abra **VSCode** y haga clic en el ícono de **PlatformIO** en la barra lateral.
2. Haga clic en **"Open Project"**.
3. **CRÍTICO:** Navegue y seleccione ÚNICAMENTE la subcarpeta específica correspondiente:
   - `01_Firmware/Maestro` si va a flashear la tarjeta Maestro STM32.
   - `01_Firmware/Esclavo` si va a flashear la tarjeta Esclavo STM32.
   - `01_Firmware/Repetidor` si va a flashear la tarjeta Repetidor ESP32.

**Paso 2: Conectar y Flashear**
1. Conecte el STM32 (vía ST-Link) o la ESP32 (vía USB) a la computadora.
2. Haga clic en el ícono de **visto bueno (✓ Build)** en la barra azul inferior. Esperar mensaje `[SUCCESS]`.
3. Haga clic en el ícono de la **flecha a la derecha (→ Upload)** para grabar la placa.
4. *(Tip para chips CKS32):* Si el ST-Link falla al conectar, mantenga presionado el botón RESET físico de la tarjeta BluePill al presionar "Upload" y suéltelo justo cuando aparezca "Connecting..." en la consola.

> ⚠️ **Las dos tarjetas deben llevar firmware de la MISMA versión.** El cálculo de la fase del Modo
> Degradado vive en un fichero que es idéntico en los dos proyectos, y las dos puntas tienen que
> calcular exactamente lo mismo a partir de la hora. **Flashear versiones distintas en cada punta
> puede romper la fase sin ningún aviso en pantalla** — cada unidad creería estar en lo correcto.
> Anote la versión y el commit cargados en cada tarjeta.

---

## 5. 🔋 La pila del RTC (`CR2032` en `VBAT`)

Desde el 01/08/2026 **ambas tarjetas llevan pila**. Es lo que hace que la hora sobreviva a un corte de
energía; sin ella habría que reponerla a mano tras cada apagón, y el Modo Degradado quedaría
inutilizable en la práctica.

### Qué se hizo en la tarjeta

| Elemento | Detalle |
|---|---|
| Cristal | **`Y2` de 32.768 kHz**, ya venía en el diseño, cableado a `PC14`/`PC15` |
| Pila | **`CR2032` NO recargable**, 3 V, soldada al pad de `R5` del lado `VBAT` y a GND |
| **`R5` retirado** | Era un puente de 0 Ω que unía `VBAT` con los 3,3 V |
| Pines ocupados | **Ninguno.** El RTC es interno del STM32 |

> ## ⚠️ POR QUÉ HAY QUE RETIRAR `R5` ANTES DE PONER LA PILA
>
> `R5` une `VBAT` con la fuente de 3,3 V. **Si se conecta la pila sin quitarlo, la `CR2032` queda en
> paralelo con los 3,3 V y la fuente le inyecta corriente.** Una pila **no recargable** en esa
> situación **se calienta, se hincha y puede reventar**.
>
> Verificación previa, con la tarjeta **sin alimentación** y el multímetro en continuidad: una punta
> en el pin 1 de `U1` (`VBAT`) y otra en 3,3 V. **Si pita**, `R5` los une y hay que desoldarlo. **Si
> no pita**, `R5` va a otro sitio: **no continúe** hasta aclararlo.

> ⚠️ **No confundir el tipo de pila.** En `VBAT` de esta tarjeta va **`CR2032` (no recargable)**,
> porque **el STM32 no carga la pila**. La `LIR2032` recargable solo corresponde a los módulos
> externos `DS3231`, que sí traen circuito de carga. **Poner la equivocada en cualquiera de los dos
> casos es peligroso.**

### Detalle completo de la instalación

El procedimiento paso a paso —puntos de GND cómodos para soldar, verificación con multímetro y la
alternativa por módulo `DS3231` si el cristal no arranca— está en
`03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md §4`. **Este manual no lo duplica a propósito:** dos copias
de un procedimiento de soldadura acaban divergiendo.

> 🛑 **PERO NO PIDA UN `DS3231` CREYENDO QUE FUNCIONARÁ AL ENCHUFARLO.** Medido sobre el firmware el
> 28/08: **no hay driver de `DS3231` en ninguna de las dos puntas.** El reloj que usa el equipo es
> el **RTC interno del STM32** con el cristal `Y2` y la pila en `VBAT` (SFTY-18). Un módulo `DS3231`
> montado hoy sería **una pieza de hardware sin una sola línea de software que la lea**.
>
> La detección automática de `DS3231` en el arranque está **diseñada y no construida** — vive en
> `OPTIMIZACIONES.md` como **SFTY-26**, marcada `DISEÑO, NO IMPLEMENTADO`. **Esa alternativa es un
> plan, no un repuesto.**

### Vida útil — no es mantenimiento periódico

Con ~1,4 µA de consumo por `VBAT`, la autonomía teórica supera los **15 años**. El límite real es la
**caducidad de la pila (~10 años)**, no su descarga. No hay que programar sustituciones.

### 🧪 Dos pruebas de banco que faltan — y por qué importan

> **Ninguna de las dos se ha hecho todavía.** Hasta que se hagan, el reloj está construido pero no
> verificado, y todo lo que depende de él va sobre un supuesto.

| Prueba | Qué se comprueba | Por qué importa |
|---|---|---|
| **Contraste contra hora patrón y corte de energía** (N-15) | Que el RTC marca la hora correcta y **la conserva** al desconectar la alimentación | Si la pila no está bien soldada, el equipo pierde la hora en cada apagón y **nadie se entera hasta que el Degradado se rechaza en obra** |
| **Arranque con el cristal `Y2` desconectado o fallido** (N-17) | Que el equipo **bootea igual** y declara la hora como no fiable | Algunos microcontroladores clonados traen mal los condensadores de carga y **el oscilador de 32.768 kHz no arranca**. La rutina de reloj ya se movió detrás del watchdog para que un bloqueo sea un reinicio visible y no un cuelgue mudo con las luces apagadas — pero eso **hay que comprobarlo con la tarjeta en la mano** |

> ⚠️ **Y no se cambia nada antes de leer `CONSULTA RELOJ` (N-37, abierto).** Existe una hipótesis
> razonable —que los condensadores de carga de $Y_2$ estén mal calculados, y que sustituirlos por
> **6 a 10 pF (C0G/NP0)** arregle la oscilación—, pero **hoy no está medida**. Este proyecto ya
> pagó una vez por saltarse este paso: una pantalla acusaba a *"Y2, pila y R5"* sin haber medido
> ninguno de los tres, y mandó a cambiar componentes sanos.
>
> **El orden es: primero la lectura, después la pieza.** ~~`CONFIGURACION` → `AJUSTAR HORA` →
> confirmar. Si la hora queda puesta y no aparece pantalla de error, **el cristal no era el
> problema y no se toca nada**. Si aparece `CONSULTA RELOJ`, solo la línea `Pedido, no oscila`
> señala al cristal; `LSE no se pide`, `Oscila; RTC no atado` y `Oscila y atado a LSE` dicen que
> no era.~~ Con esa lectura en la mano, la sustitución de condensadores pasa a ser una petición
> concreta al funcional, no una conjetura.
>
> 🔴 **28/08 — ESTE PROCEDIMIENTO SE HA QUEDADO SIN INSTRUMENTO, Y HAY QUE DECIRLO.** La pantalla
> `CONSULTA RELOJ` y sus cuatro líneas de diagnóstico **eran de la LCD**, y la LCD se retira. **Esas
> cuatro líneas no viajan en la trama `$STATUS`.**
>
> Lo que **sí** se puede hacer hoy desde la app: poner la hora con
> `CMD:PIN:1234:SET_RTC:YYYY-MM-DD,HH:MM:SS` y leer el campo `HORA:` de `$STATUS`. **Si la hora
> queda puesta y sigue avanzando, el cristal oscila**; si `HORA:` devuelve `--:--:--`, el reloj se
> declara no fiable. Eso distingue *«funciona»* de *«no funciona»*, pero **NO distingue cuál de las
> cuatro causas es** — que era exactamente el punto de aquella pantalla.
>
> **Consecuencia honesta: mientras ese diagnóstico no esté en la app, N-37 no se puede cerrar por
> lectura.** No se sustituyen condensadores por conjetura; se anota que falta el instrumento.

> **Un semáforo no puede depender de un cristal de reloj para encender.** Ésa es la razón de la
> segunda prueba, y es la más fácil de olvidar porque en una tarjeta sana nunca se nota.

---

## 6. 🎛️ El mando de 4 relés

El operario acciona el equipo **desde el piso**, con un mando de 4 relés (`A`, `B`, `C`, `D`) cableados
**en paralelo con los cuatro pulsadores físicos** (`PB9`, `PB13`, `PB14`, `PB15`). **No hay entradas
dedicadas**: para el firmware, un relé y un botón son indistinguibles.

### Características medidas en campo (01/08/2026) — condicionan el diseño

| Medida | Valor | Consecuencia |
|---|---|---|
| Tipo de señal | **Pulso por flanco**, no se sostiene | **La pulsación larga NO existe.** Sostener el botón 10 s da un solo pulso |
| Retardo por pulsación | **~2 s** | Una ventana de 3 s es inviable; hacen falta 12–18 s para 3–4 pulsos |
| Repetición automática | **No la hay** | Cada pulso exige una pulsación |

Si va a reemplazar o comprar mando, **verifique estas tres características antes de pedirlo**: todo el
diseño de las secuencias y de la pantalla `AJUSTAR HORA` (edición dígito a dígito) descansa sobre
ellas. Un mando que se comporte distinto obliga a rehacer ambas.

> ## 🪜 EL ESCLAVO NO TIENE RECEPTOR DE MANDO (pendiente N-19)
>
> **Hoy solo el Maestro puede operarse desde el piso.** La tarjeta del Esclavo **ya trae las cuatro
> entradas** (`PB9`, `PB13`, `PB14`, `PB15`) y su firmware las atiende: **falta únicamente comprar e
> instalar el receptor**.
>
> **No hay atajo por software.** El Maestro no puede ordenárselo por radio, porque *el radio muerto es
> justamente la razón de entrar al Modo Degradado*.
>
> **Consecuencia operativa, sin adornos:** el procedimiento del Modo Degradado exige activarlo en las
> dos puntas, y ambas pantallas están a **5 m dentro del gabinete**. Mientras el receptor no esté
> instalado, **activar el Degradado en el Esclavo obliga a subir físicamente al gabinete**, con
> escalera o canasta, en las condiciones que haya. **Mientras tanto el sistema funciona** — es una
> limitación de operación, no una avería.

> ## ⚠️ AL COMPRAR EL RECEPTOR: EXIJA CÓDIGO INDEPENDIENTE DEL MANDO DEL MAESTRO
>
> Si ambos receptores responden al mismo mando —y las dos puntas suelen estar a **menos de una
> cuadra**— una sola secuencia metería **las dos unidades en Modo Degradado a la vez**.
>
> Eso se salta la **verificación por separado de cada punta**, que es exactamente lo que justifica que
> este modo se considere seguro. Un mando compartido convierte un procedimiento verificado en una
> pulsación a ciegas.
>
> Pídalo con **código o dirección distinta**, y **compruébelo en banco** antes de instalarlo:
> accione el mando del Maestro y confirme que el Esclavo **no** reacciona.

### Verificación del cableado del mando

- [ ] Cada relé cierra contra **el mismo pin** que su pulsador correspondiente
- [ ] Con el equipo en el menú, accionar `A` mueve el cursor **una sola posición** por pulsación
- [ ] Accionar `C` desde el mando **selecciona**, igual que el Botón 3
- [ ] El mando **no** genera pulsos espurios al energizar el gabinete

> **El último punto no es paranoia.** Un pulso espurio al energizar, con el menú abierto, puede llevar
> el cursor a una opción que nadie pidió. El firmware ya lo acota —las secuencias se ignoran con el
> menú abierto— pero un mando que emite basura al arrancar es un mando defectuoso.

---

## 7. 📷 Conexión del Sistema de 4 Cámaras IA (Hikvision AcuSense)

Para detección vehicular por demanda en obra vial (analítica embebida sin computadores externos):

```text
       CÁMARAS HIKVISION ACUSENSE                     TARJETA CONTROLADORA STM32
  ┌─────────────────────────────────┐              ┌───────────────────────────────┐
  │ CÁMARA 1 (Demanda Sentido 1)    ┼─ Hilos 1A/1B ┼──► Bornera PB0 (Entrada Libre)│
  │ CAMARA 2 (Umbral) NO SE INSTALA ┼───── — ──────┼──► sin bornera: PB8 es LED   │ (En Maestro)
  ├─────────────────────────────────┤              ├───────────────────────────────┤
  │ CÁMARA 3 (Demanda Sentido 2)    ┼─ Hilos 1A/1B ┼──► Bornera PB0 (Entrada Libre)│
  │ CAMARA 4 (Umbral) NO SE INSTALA ┼───── — ──────┼──► sin bornera: PB8 es LED   │ (En Esclavo)
  └─────────────────────────────────┘              └───────────────────────────────┘
```

* **Salida de Alarma de la Cámara:** Contacto seco libre de potencial (`1A` y `1B` del conector de alarma de la cámara Hikvision, configurado en N/O a 1s).
* **Analítica Embebida:** La cámara ejecuta internamente su algoritmo AcuSense (Detección de Intrusión con filtro `☑ Solo Vehículo`), ignorando peatones, ramas y sombras.
* **Seguridad Vial Inquebrantable:** Cada transición de sentido impone automáticamente el tiempo de **Despeje Todo-Rojo (`cfgDespejeSeg`)** configurado en pantalla antes de habilitar el verde con demanda.

---

## 8. 📱 Conexión del Módulo Bluetooth — 🔴 REESCRITA EL 28/08: ENTRA POR `J17`

Desde el 28/08 esto **no es un accesorio de diagnóstico: es la interfaz del equipo.**

> # ⛔ PARE. `J16` LLEVA 12 V Y QUEMA EL MÓDULO
>
> **Léase esto ANTES de tocar un solo cable.**
>
> La tarjeta tiene dos conectores de señal que se parecen y están cerca:
>
> | | qué es | **posición 1** |
> |---|---|---|
> | **`J16`** | conector de la **BOTONERA** | 🔴 **`12 V`** |
> | **`J17`** | conector de la **PANTALLA** — **el suyo** | `CS` (señal, no alimentación) |
>
> **`J16` es el único conector de señal de esta tarjeta que trae 12 V.** El módulo Bluetooth es de
> **3,3 V**. Confundirlos **lo quema**, y no hay aviso previo: se enchufa, se energiza y se acabó.
>
> ### Cómo distinguirlos — se MIDE, no se mira
>
> Con el equipo energizado y el multímetro en tensión continua, **mida la posición 1 del conector
> contra GND**:
>
> - **≈ 12 V → es `J16`. NO es el suyo. Retire la punta y busque el otro conector.**
> - **No hay 12 V → puede ser `J17`.** Confirme entonces que las posiciones 6 y 8 dan **3,3 V**.
>
> ⚠️ **Y cuente los pines DESDE EL PIN 1, no desde el borde del conector.** El símbolo de `J17` en
> el esquema tiene **13 posiciones** y el footprint de la placa tiene **16**: si cuenta desde el
> borde, todo el mapa se desplaza tres posiciones. Lo mismo en `J16` (símbolo 12, footprint 16).

### El cableado

```text
       MÓDULO BLUETOOTH (BALIZA)                      TARJETA CONTROLADORA STM32
  ┌─────────────────────────────────┐              ┌───────────────────────────────┐
  │   [ VCC ] ──────────────────────┼──────────────┼──► J17 pos. 6   (3,3 V)       │
  │   [ GND ] ──────────────────────┼──────────────┼──► J17 pos. 7   (GND)         │
  │   [ TXD ] ──────────────────────┼──────────────┼──► J17 pos. 2 = PB7 (USART1 RX)│
  │   [ RXD ] ──────────────────────┼──────────────┼──► J17 pos. 3 = PB6 (USART1 TX)│
  └─────────────────────────────────┘              └───────────────────────────────┘
                                        (USART1 REMAPEADO — ver abajo)
```

| `J17` | red del esquema | pin del STM32 | al módulo |
|---|---|---|---|
| **2** | `RST` | **`PB7`** — `USART1_RX` | **`TXD`** |
| **3** | `RS(A0)` | **`PB6`** — `USART1_TX` | **`RXD`** |
| **6** | `3,3 V` | — | `VCC` |
| **7** | `GND` | — | `GND` |

* **Es el `USART1` REMAPEADO, no un segundo puerto serie.** El STM32F103 permite sacar el `USART1`
  por `PB6`/`PB7` en lugar de `PA9`/`PA10`, **pero solo por un sitio a la vez**. El firmware ya está
  ahí: `static HardwareSerial SerialBT(PB7, PB6);` en `bluetooth.cpp`, **idéntico en las dos puntas**.
* **Por qué estos pines quedaron libres:** eran `LCD_PSB` (`PB6`) y `LCD_RST` (`PB7`) de la pantalla,
  y **ninguno de los dos llevaba datos** — uno era un nivel estático y el otro un pulso de arranque.
  Los tres hilos de datos del ST7920 (`SCLK`, `SID`, `CS`) no se tocan.
* **Baudrate:** 9600 bps, 8-N-1.

> ### `PA9`/`PA10`: válido eléctricamente, **NO es el montaje vigente**
>
> El puerto sigue existiendo en esos pines, pero **no salen a ninguna bornera de la tarjeta**. Para
> usarlos hay que **soldar** en las patas del MAX3485 `U2` o del propio micro. **Queda como
> alternativa de laboratorio, no como el cableado de campo.** Las ediciones anteriores de este
> manual daban `PA9` TX / `PA10` RX: era correcto para el montaje anterior y **ha quedado obsoleto,
> no se ha borrado.**

### 🔧 Corrección de chip: `U2` y `U3` estaban INVERTIDOS

> **Este manual decía `MAX3485 U3` donde va `U2`.** Se corrige dejando constancia, porque una
> corrección silenciosa se vuelve a proponer al mes siguiente.

Trazado red por red sobre el esquemático (`Controladora_Semaforos.kicad_sch`):

| chip | qué puerto | pines del micro | par A/B por |
|---|---|---|---|
| **`U2`** | MAX3485 del **`USART1`** | `RO`(1)→`PA10` · `~RE`(2) y `DE`(3)→`PA8` · `DI`(4)→`PA9` | **`J10`** |
| **`U3`** | MAX3485 del **`USART3`** — el de la **radio LoRa** | `PB11` · `PB12` · `PB10` | **`J12`** |

* **Desacoplo de Hardware:** el pin `PA8` (`RS485_IN_DE_RE`) se fija en `HIGH` permanente en el
  firmware. Eso apaga el receptor `RO` del **`U2`** (no del `U3`) y deja libre `PA10`.

> ⚠️ **El matiz que este manual no decía y sí importa: `PA8` gobierna A LA VEZ `~RE` (pin 2) y `DE`
> (pin 3) de `U2`.** Ponerlo en `HIGH` apaga el **receptor** —que es lo que se busca— pero **deja el
> transmisor ENCENDIDO**. Consecuencia: `U2` vuelca la telemetría por **`J10`** de forma permanente,
> y **esa línea no puede recibir nunca**.
>
> **Hoy es inofensivo porque `J10` está vacío.** Deja de serlo el día que alguien cuelgue algo de
> `J10` — y ese día **hay que tocar el código, no el cableado**. Es la lección del repetidor del
> 31/07/2026, ya escrita en `01_Firmware/TROUBLESHOOTING.md`: **un `DE`/`RE` clavado en alto bloquea
> la línea en AMBOS sentidos.**
>
> Nótese además que este desacoplo **ya no es estrictamente necesario para el Bluetooth**, puesto que
> el módulo se ha mudado a `PB6`/`PB7`. **El firmware lo sigue haciendo**, y se documenta tal como
> está en lugar de describir lo que sería razonable.

### Telemetría, caja negra y seguridad

* **Telemetría:** emisión cada 1 s de
  `$STATUS,NODE:...,SERIE:...,MODO:...,ESTADO:...,T:...,RF:...%,RTT:...ms,BAT:...,HORA:...*XX\r\n`.
* **Caja Negra:** registro instantáneo de caídas de radio con hora del RTC
  (`$ALARM,NODE:...,EVENTO:FALLO_RF,CAUSA:SILENCIO_25000ms,ACCION:CAMBIO_A_AMBAR,HORA:...*XX\r\n`).
  **Censado el 28/08: tiene llamadores reales** — `coordinador.cpp` líneas 683 y 775 en el Maestro,
  `main.cpp` línea 560 en el Esclavo. *(No siempre fue así: la función estuvo documentada en cuatro
  manuales y sin un solo llamador — N-73.)*
* **Seguridad:** los comandos de control exigen PIN de 4 dígitos (`CMD:PIN:1234:...`), **salvo
  `CMD:FORZAR_ROJO`, que se acepta SIN PIN a propósito**: llevar el cruce a rojo total es la
  dirección segura y no debe depender de recordar un código.

> 🔴 **AVISO SOBRE TRES CAMPOS DE `$STATUS` QUE NO SON MEDIDAS.** Medido sobre `bluetooth.cpp` el
> 28/08:
>
> | campo | Maestro | Esclavo |
> |---|---|---|
> | `RF:` | ✅ real (SFTY-14) | 🔴 **literal `98%`** en el `snprintf` (línea 215) |
> | `RTT:` | ✅ real | 🔴 **literal `85ms`** |
> | `BAT:` | 🔴 **literal `12.6`** | 🔴 **literal `12.6`** |
> | `T:` | ⚠️ `(millis()/1000) % 60` — contador libre 0–59, **no** la cuenta regresiva de la fase | ⚠️ igual |
>
> **Un tablero que rellena con constantes el dato que no tiene miente a quien decide mirándolo.**
> El `RF:98%` del Esclavo se emite igual con la antena desconectada. **No use esos campos para
> juzgar el enlace del Esclavo ni la batería de ninguna de las dos puntas**, y no los apunte en un
> acta como si fueran medidas.

