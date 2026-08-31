# ⏱️ MANUAL TÉCNICO DE INSTALACIÓN — PILA DEL RELOJ RTC Y PLAN DE CONTINGENCIA DS3231 (V9.0)

**Sistema:** Controladora de Semáforos Móviles de 3 Estados (Maestro y Esclavo V9.0)  
**Hardware Principal:** RTC Interno STM32 (`PC14`/`PC15` cristal `Y2` de 32.768 kHz) + Pila CR2032 en `VBAT`  
**Plan de Contingencia:** ~~Módulo Externo DS3231 en pines libres `PB0` (SDA) y `PB8` (SCL) por I²C Software~~ ⛔ **ANULADO el 28/08 — y además era FALSO: esos dos pines NO están libres.** El `DS3231` cuelga hoy del **ESP32** (`GPIO21` SDA / `GPIO22` SCL). Ver el aviso de abajo y el apartado 5  
**Propósito:** Sincronización horaria ininterrumpida para Modo Degradado, horario nocturno y Caja Negra  
**Verificación Hardware:** Esquemáticos KiCad `Controladora_Semaforos.kicad_sch`, `pines.h` y `MAPEO_TARJETA_KICAD.md`  
**Fecha de Emisión:** 26 de Agosto de 2026  
**Última revisión:** 31 de Agosto de 2026 — **el apartado 5 (Plan B DS3231) está ANULADO y no se cablea.**
Motivo: mandaba conectar un bus I²C a `PB0` y `PB8` llamándolos *«los dos únicos pines libres de la
placa»*, y **ninguno de los dos lo está**. Corregido también el renglón «Plan de Contingencia» de esta
cabecera y la columna de acción del apartado 4. **Lo demás de este manual —`R5`, la pila `CR2032` en
`VBAT` y la comprobación por pantalla— no se ha tocado y sigue vigente.**

---

## 🛑 AVISO DE SEGURIDAD — LÉASE ANTES DE TOCAR NADA (31/08/2026)

> ## ⛔ EL CABLEADO DEL APARTADO 5 (DS3231 a `PB0`/`PB8`) **NO SE EJECUTA**. NO ES UN «PLAN B» DISPONIBLE: ES UN ERROR DE ESTE MANUAL.

Este documento estaba **intacto desde el commit raíz y sin un solo aviso**, mandando cablear un bus
I²C contra dos pines que **ya tienen dueño en el firmware que corre hoy**.

**MEDIDO EN EL FUENTE** (`grep` sobre `pines.h`, idéntico en las dos puntas):

```
01_Firmware/Maestro/include/pines.h:46   #define CAM_DEMANDA_PIN    PB0  // -> R64 10K + C25 100nF -> bornera J14
01_Firmware/Esclavo/include/pines.h:46   #define CAM_DEMANDA_PIN    PB0  // (linea identica)
01_Firmware/Maestro/include/pines.h:63   #define LED_TESTIGO        PB8  // -> R16 1K -> LED D5. NO es entrada de camara
01_Firmware/Esclavo/include/pines.h:63   #define LED_TESTIGO        PB8  // (linea identica)
```

| pin | lo que dice este manual | lo que hay de verdad | nivel de prueba |
|---|---|---|---|
| `PB0` | «pin libre, `SDA` del I²C software» | **Entrada de la cámara de demanda**, bornera `J14`, con `R64` 10 kΩ de *pull-down* y `C25` de 100 nF de antirrebote | ✅ **MEDIDO EN EL FUENTE** (`pines.h:43-46`) |
| `PB8` | «pin libre, `SCL` del I²C software» | **`LED_TESTIGO`**: sale por `R16` de 1 kΩ al LED `D5` | ✅ **MEDIDO EN EL FUENTE** (`pines.h:50-63`) |

**Qué pasa si alguien sigue el diagrama del apartado 5:**

* En `PB0`, el `C25` de **100 nF** cuelga de la línea de datos. Son **250 veces** el límite de carga
  capacitiva del I²C (`roadmap.md:1899`): el bus **no puede conmutar**. Y mientras tanto ese hilo
  entra a la **entrada de demanda de la cámara**, que es la que pide paso.
* En `PB8`, `R16` de 1 kΩ **fija el nivel de la línea de reloj** contra el LED (`roadmap.md:1931`):
  un `SCL` que no puede subir.
* El resultado no es «un reloj que no funciona»: es **un hilo de un accesorio metido en la entrada
  que solicita verde**, en un equipo que gobierna un cruce.

> 🔴 **Este proyecto ya pagó DOS VECES por publicar un pin como libre sin cruzarlo con `pines.h`, y
> esta es la tercera aparición del mismo error.**
>
> * **N-67** (`roadmap.md:1745`): la entrada de cámara de `PB0` estaba **leída al revés** —`INPUT_PULLUP`
>   contra el *pull-down* de 10 kΩ de la placa deja el pin en 0,66 V, que el micro lee `LOW`—. El
>   firmware habría visto **demanda permanente desde el arranque, sin ninguna cámara conectada**. Salió
>   de ir a escribir *«la cámara se conecta aquí»* y no saber qué había en el otro borne de `J14`.
> * **N-59** (`roadmap.md:2194`): `CAM_UMBRAL_PIN` (`PB8`) se declaraba, se ponía en `INPUT_PULLUP` y
>   **no se leía nunca**, mientras **cuatro documentos** afirmaban que las cámaras 2 y 4 contaban
>   entradas y salidas del tramo. Cuatro manuales describiendo una función que no existía.
>
> **La regla que queda escrita: «pin libre» no es una observación, es una medida contra `pines.h`.**
> Un manual que llama libre a un pin ocupado no se equivoca en una palabra: manda un destornillador
> a una entrada viva.

---

## 1. Arquitectura del Reloj en la Tarjeta Madre STM32

El diseño de la tarjeta controladora **ya incluye el cristal `Y2` de 32.768 kHz** ruteado a los pines `PC14` y `PC15` del microcontrolador STM32F103C8T6.

Para mantener la hora y fecha exactas durante cortes de energía o traslados en bodega, el microcontrolador conmuta automáticamente al dominio de batería **`VBAT` (Pin 1)**.

```text
 ┌─────────────────────────────────────────────────────────────────────────────┐
 │                 ARQUITECTURA DEL RELOJ EN LA TARJETA MADRE                  │
 ├─────────────────────────────────────────────────────────────────────────────┤
 │                                                                             │
 │    CRISTAL Y2 (32.768 kHz) ──► Pines PC14 / PC15 del STM32 (Oscilador LSE)  │
 │                                                                             │
 │    PORTAPILAS CR2032 (3V)  ──► Pad VBAT (Pin 1 de U1) tras desoldar R5      │
 │                                                                             │
 │    DIAGNÓSTICO EN PANTALLA ──► Menú "CONSULTA RELOJ" (Oscilando OK / En Hora)│
 │                                                                             │
 └─────────────────────────────────────────────────────────────────────────────┘
```

---

## 2. ⚠️ ADVERTENCIA CRÍTICA: Desoldar Obligatoriamente `R5` antes de Colocar la Pila

En la PCB de fábrica, `R5` es una resistencia puente de **0 Ω** que une el pin `VBAT` con la línea principal de **3.3V**.

> ### 🛑 PELIGRO DE SOBRECALENTAMIENTO Y EXPLOSIÓN:
> Si se conecta una pila **CR2032 (no recargable)** sin retirar previamente la resistencia `R5`, la fuente de 3.3V inyectará corriente continua a la pila cuando el semáforo esté encendido.
> La pila se calentará, se hinchará y **puede reventar dentro del gabinete**.

```text
       ESTADO DE FÁBRICA (SIN PILA)                ESTADO MODIFICADO (CON PILA CR2032)
   ┌───────────────────────────────────┐       ┌───────────────────────────────────┐
   │                                   │       │                                   │
   │   3.3V ───[ R5: 0 Ω ]───► VBAT   │       │   3.3V ───[ X R5 RETIRADA X ]     │
   │                           (Pin 1) │       │                                   │
   │                                   │       │   (+) Pila CR2032 ──► Pad VBAT    │
   │                                   │       │   (-) Pila CR2032 ──► GND         │
   └───────────────────────────────────┘       └───────────────────────────────────┘
```

---

## 3. Procedimiento de Instalación Paso a Paso

### Paso 1: Comprobación con Multímetro
1. Con la tarjeta **totalmente desenergizada**, poner el multímetro en modo continuidad (pito).
2. Colocar una punta en el **Pin 1 de U1 (VBAT)** y la otra en el punto de **3.3V**.
3. Debe pitar continuo (confirmando que `R5` está presente).

### Paso 2: Retiro de `R5` (Desoldadura)
1. Con cautín a 350°C y malla desoldadora o pinzas finas, **desoldar y retirar la resistencia SMD `R5`**.
2. Volver a medir continuidad entre Pin 1 (`VBAT`) y 3.3V: **NO debe pitar**.

### Paso 3: Soldadura del Portapilas CR2032
1. **Cable Positivo (Rojo `+`):** Soldar al pad de `R5` que conecta directamente con `VBAT` (Pin 1).
2. **Cable Negativo (Negro `-`):** Soldar a cualquier punto de masa `GND` confiable:
   * Aleta metálica del regulador `U4` (LM7805).
   * Pin central `GND` del regulador `U4`.
   * Pin `GND` de las borneras RS-485.

### Paso 4: Inserción de la Batería
* Insertar una pila botón **CR2032 de 3.0V (no recargable)** de marca reconocida (Panasonic, Maxell, Sony).

---

## 4. Verificación de Funcionamiento con la Pantalla LCD

En el menú del semáforo, ingresar a **`CONFIGURACION` ➔ `CONSULTA RELOJ`** (`lcd.cpp:421`):

```text
 ┌──────────────────────────────────────┐
 │ CONSULTA RELOJ                       │
 │                                      │
 │ Estado:  Oscilando OK                │
 │ Registro: 18:25:00                   │
 │ Contador: En marcha (+1s)            │
 └──────────────────────────────────────┘
```

### Tabla de Diagnósticos Oficiales del Firmware:

| Mensaje en Pantalla LCD | Causa Técnica | Acción Requerida |
|---|---|---|
| **`Oscilando OK / En hora`** | El oscilador `Y2` y la pila operan perfectamente. | Ninguna. Sistema listo para operación. |
| **`Pedido, no oscila`** | Los condensadores de carga $C_1/C_2$ del cristal no resuenan. | Reemplazar $C_1/C_2$ por 6–10 pF C0G/NP0. ~~o aplicar Plan B (DS3231)~~ ⛔ **el «Plan B» de este manual está ANULADO: no se cablea nada a `PB0`/`PB8`.** Ver apartado 5 |
| **`Parado / Sin bateria`** | Pila agotada o $R_5$ no retirado correctamente. | Medir voltaje en Pin 1 (`VBAT` debe ser > 2.8V). |

> ⚠️ **Este apartado 4 se comprueba desde la pantalla LCD, y la pantalla está en la lista de lo que se
> retira.** La arquitectura de obra del 28/08 retira la LCD, los cuatro pulsadores y el mando de relés
> (`ESTADO.md:80`, `05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md` §1.3). **MEDIDO hoy en
> el fuente: la pantalla sigue en el firmware** —`01_Firmware/Maestro/src/main.cpp:46` llama a
> `lcd_setup()`, y `CONSULTA RELOJ` vive en `01_Firmware/Maestro/src/lcd.cpp:428-443`—, así que **este
> procedimiento funciona tal cual mientras la LCD siga montada**.
>
> 🟡 **PENDIENTE, con dueño: el día que se retire la pantalla, este apartado se queda sin instrumento**
> y hay que decidir por dónde se lee el diagnóstico del reloj (la consola por celular es la candidata
> obvia, pero **hoy no muestra estos tres mensajes** y este manual no lo afirma). **Lo decide el
> responsable**, no este documento.

---

## 5. ⛔ ANULADO — «Plan B» del DS3231 sobre `PB0`/`PB8` en el STM32

> ## 🛑 ESTE APARTADO NO DESCRIBE UN MONTAJE DISPONIBLE. DESCRIBE UN CABLEADO QUE NO SE HACE.
>
> **Dos motivos, y el segundo es el grave:**
>
> 1. **La decisión cambió:** el `DS3231` ya no va en el STM32. Cuelga del **ESP32** (§5.2).
> 2. **Y aunque no hubiera cambiado, el cableado de abajo era ERRÓNEO desde el principio:** `PB0` y
>    `PB8` **no están libres** —lo están en este manual, no en la placa—. Ver el aviso de cabecera.
>
> **No se borra**, porque hubo una versión de este manual —**todas, hasta el 31/08**— que lo mandaba
> ejecutar, y porque una vía descartada que desaparece en silencio se vuelve a proponer.

### 5.1 ~~El montaje que este manual mandaba hacer~~ — CONSERVADO SOLO COMO RASTRO

~~Si la tarjeta posee un microcontrolador clonado cuyo oscilador interno `Y2` no lograse oscilar
(`Pedido, no oscila`), se conecta un módulo **DS3231 TCXO externo** a los **dos únicos pines libres de
la placa (`PB0` y `PB8`)**:~~

```text
   ##############  DIAGRAMA ANULADO -- NO EJECUTAR ESTE CABLEADO  ##############

       MODULO RTC DS3231 (EXTERNO)                   TARJETA SEMAFORO STM32
   ┌────────────────────────────────────┐         ┌───────────────────────────────┐
   │  [ VCC ]  (Alimentacion 3.3V) ─────┼─────────┼──► Pin 3.3V                   │
   │  [ GND ]  (Tierra / Masa)    ──────┼─────────┼──► Pin GND (Tierra comun)     │
   │  [ SDA ]  (Datos I2C)        ──X───┼─X───────┼─X► Pin PB0   <-- OCUPADO:     │
   │                                    │         │      CAM_DEMANDA_PIN (J14)    │
   │  [ SCL ]  (Reloj I2C)        ──X───┼─X───────┼─X► Pin PB8   <-- OCUPADO:     │
   │                                    │         │      LED_TESTIGO (R16 -> D5)  │
   └────────────────────────────────────┘         └───────────────────────────────┘

   ##############  ANULADO EL 28/08 (decision) Y CORREGIDO EL 31/08 (error)  #####
```

**Por qué el error no se veía:** el manual llamaba `PB0`/`PB8` *«los dos únicos pines libres»* y
**tenía razón en su momento** — la propuesta salió de `N-37` (`roadmap.md:4559`), cuando el cristal `Y2`
se declaró muerto y esos dos pines aún no tenían función. **`V9.0` se los dio a las cámaras**, y la
frase se quedó escrita describiendo una placa que ya no existía. Es exactamente lo que
`CLAUDE.md` §3.bis advierte de los comentarios: no fallan cuando alguien cambia un número, **se quedan
con la autoridad de una cuenta hecha**. El choque quedó anotado como `N-57` (`roadmap.md:2417`):
*«`PB0`/`PB8` están asignados dos veces, y el reloj llegó primero»*.

### 5.2 ✅ Dónde vive HOY el reloj externo: colgado del ESP32, fuera de la tarjeta

📖 **LEÍDO en los documentos de decisión, no medido en hardware** (no hay hardware que medir todavía):

| dato | fuente |
|---|---|
| El `DS3231` cuelga del **ESP32** por I²C: **`GPIO21` = SDA, `GPIO22` = SCL**, con **pila propia**. El módulo `ZS-042` trae sus *pull-ups* | `05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md` §1.3 (línea 89) |
| «El ESP32 es un módulo de expansión colgado de un puerto serie, y **no manda sobre las luces**» | `ESTADO.md:80` |
| La fila `PIN-0` —*«`PB0`/`PB8` van a bus I²C»*— está **⛔ ANULADA**: *«el I²C ya no vive en el STM32 […] `PB0` se queda como cámara de demanda»* | `ESTADO.md:124` |
| El módulo es la línea de compra **`A6`**, y **NO se compró** todavía | `05_Funcional/15_Lista_de_Compras_Hardware.md:91` |

```text
 ┌─────────────────────────────────────────────────────────────────────────────┐
 │        DONDE VA EL DS3231 HOY  --  FUERA DE LA TARJETA DEL SEMAFORO         │
 ├─────────────────────────────────────────────────────────────────────────────┤
 │                                                                             │
 │   STM32F103  (gobierna el semaforo)          ESP32  (accesorio, no manda)   │
 │   ┌──────────────────────────┐               ┌──────────────────────────┐   │
 │   │  8 luces, radio, camaras │               │  DS3231  <-- GPIO21 SDA  │   │
 │   │                          │   J17 serie   │          <-- GPIO22 SCL  │   │
 │   │  PB7 (RX) p2  <──────────┼───────────────┼── GPIO17 (TX2)           │   │
 │   │  PB6 (TX) p3  ──────────►┼───────────────┼─► GPIO16 (RX2)           │   │
 │   │  GND      p7/p9 ─────────┼───────────────┼── GND  (masa comun)      │   │
 │   └──────────────────────────┘               └──────────────────────────┘   │
 │        alimentado por la tarjeta               FUENTE PROPIA desde 12 V      │
 │                                                (linea A5 -- NO PEDIDA)      │
 │                                                                             │
 │   9600 8N1.  El ESP32 NO cuelga del 3,3 V de J17 p6/p8: ese riel alimenta   │
 │   al STM32 que gobierna el cruce, y el accesorio no puede tumbar al que     │
 │   manda.  (Manual 10 §1, doc 17 §1.5.)                                      │
 └─────────────────────────────────────────────────────────────────────────────┘
```

* **Tipo de batería en el módulo `DS3231`:** si el módulo incluye circuito de carga activo, usar
  **`LIR2032` (recargable)** o desoldar la resistencia de carga si se usa una `CR2032` estándar.
  *(Esto no cambia con la mudanza al ESP32: es una propiedad del módulo, no de dónde se enchufe.)*
* **Ojo con confundir las dos pilas de este manual:** la del apartado 3 es la **`CR2032` del `VBAT`
  del STM32**, y **nunca es recargable**. La del `DS3231` puede tener que serlo. Son pilas distintas
  en equipos distintos.

### 5.3 🛑 Un `DS3231` conectado hoy se queda MUDO, y eso es lo esperado — no una avería

> **MEDIDO EL 31/08, con `grep` sobre todo `01_Firmware/`:**
>
> ```
> grep -rni "ds3231" 01_Firmware --include=*.cpp --include=*.h   ->  0 coincidencias
> grep -rn  "Wire\.|#include <Wire" Maestro/src Esclavo/src      ->  0 coincidencias
> ```
>
> **No hay driver de `DS3231` en ninguna punta del STM32.** Y el del ESP32 **tampoco existe**: el
> único fuente del ESP32 en el repositorio es `01_Firmware/Repetidor/src/main.cpp` (un solo fichero,
> 8.348 B) y **no menciona `DS3231`, ni `Wire`, ni `GPIO21`/`GPIO22`**.

**Consecuencia para quien esté en el poste:** si alguien conecta un `DS3231` —donde sea— **hoy no
pasa nada**. No hay software que lo lea. **El síntoma esperado es el silencio**, y confundirlo con un
módulo defectuoso hace que se devuelva un módulo bueno y se gaste la sesión buscando un fallo de
hardware que no existe.

`05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md` §1.3 lo clasifica con las dos palabras
exactas: **«decidido, sin construir»**.

### 5.4 🟡 Lo que queda ABIERTO, y de quién es

**No lo resuelve este manual.** Se escribe aquí para que no se lea como cerrado:

| abierto | qué falta | dueño |
|---|---|---|
| **`BLQ-2` — el cristal `Y2` de la SEGUNDA tarjeta** | `N-17`/`N-37` midieron **una** tarjeta el 01/08 y el cristal **no oscila** (`roadmap.md:4559`, `4569`). **El otro sigue sin diagnosticar.** Ya **no decide la compra** —el `DS3231` va al ESP32 pase lo que pase—, pero **sí decide el firmware**: reparar el `Y2` (`C1`/`C2` a 6–10 pF C0G/NP0) o reloj de software disciplinado por el ESP32 | **Responsable** — `ESTADO.md:24`, `:261` (prueba de banco `B5`) |
| **Reloj de software colgado del accesorio** | Es la vía alternativa si el cristal no se repara, y **tiene un coste anotado**: cuelga el reloj del semáforo de un módulo accesorio, *«que es justo lo que la arquitectura del 28/08 separa»*, y **si el ESP32 no está, el reloj se va yendo sin que nadie lo vea** | **Responsable** — doc 17 §3.2 (líneas 516-525) |
| **Compra `A6` (`DS3231`) y `A5` (fuente propia del ESP32)** | `A6` **no se ha comprado**; **`A5` no se ha pedido y hace falta** — sin ella el ESP32 reinicia el STM32 que gobierna el semáforo | **Responsable** — `15_Lista_de_Compras_Hardware.md:90-91` |
| **Firmware del `DS3231` en el ESP32** | No existe. Y va **detrás del watchdog**: el ESP32 de este proyecto no tiene ninguno, con precedente escrito de uno clavado tumbando el enlace el 31/07 | **Responsable** — `ESTADO.md:107` (fase 5) |

---

## 6. 🛑 Nivel de prueba de este manual — no es un permiso para instalar

| lo que este manual afirma | nivel |
|---|---|
| `PB0` = cámara de demanda · `PB8` = `LED_TESTIGO` | ✅ **MEDIDO EN EL FUENTE** (`pines.h:46`, `:63`, las dos puntas) |
| No hay driver de `DS3231` ni I²C en ninguna punta, ni en el ESP32 | ✅ **MEDIDO** (`grep`, 31/08) |
| La LCD y `CONSULTA RELOJ` siguen en el firmware de hoy | ✅ **MEDIDO EN EL FUENTE** (`main.cpp:46`, `lcd.cpp:428`) |
| El `DS3231` va al ESP32 por `GPIO21`/`GPIO22` | 📖 **LEÍDO** en los documentos de decisión (doc 17 §1.3, `ESTADO.md:80`, `:124`). **Sin construir y sin hardware que medir** |
| Que retirar `R5` y montar la `CR2032` funcione en la tarjeta que usted tiene delante | 🔴 **NO VERIFICADO en esa tarjeta.** El procedimiento de los apartados 2-4 es el mismo desde el 26/08 y **la única medida de banco que existe es la del 01/08 sobre UNA tarjeta** |

> **Nada de esto ha pasado prueba de banco completa**, y este documento **no autoriza a instalar
> nada**. La única forma correcta de verificar el firmware es `01_Firmware/compuerta.py`, y un verde
> suyo **tampoco es un permiso**: dice que los modelos y los arneses de PC no encuentran nada, no que
> el firmware funcione en la tarjeta (`CLAUDE.md` §3).

---
*Manual técnico oficial de instalación de pila RTC V9.0. El «Plan B» del `DS3231` sobre `PB0`/`PB8` está ANULADO — apartado 5.*
