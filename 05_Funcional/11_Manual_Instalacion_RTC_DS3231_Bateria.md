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
 │    DIAGNOSTICO POR LA APP  ──► $EVENT,ORIGEN:RELOJ  (pestana "Eventos")      │
 │      La pantalla "CONSULTA RELOJ" YA NO SE PUEDE ABRIR.  Ver apartado 4.     │
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

## 4. Verificación de Funcionamiento — **HOY SE LEE POR LA APP, NO POR LA PANTALLA**

> # 🛑 `CONSULTA RELOJ` YA NO SE PUEDE ABRIR. No lo intente.
>
> La pantalla **se sigue dibujando**, pero **no se puede llegar a ella**: `CONSULTA RELOJ` vive
> dentro de `CONFIGURACION`, y para entrar hacen falta **dos pulsaciones de *Aceptar*** —una para
> bajar de nivel y otra para entrar en la opción (`Maestro/src/menu.cpp:111`, `:129`)—. Los dos
> pulsadores que lo hacían **ya no existen**:
>
> ```
>   Maestro/src/botones.cpp:280-281   bool botonAceptar()  { return false; }
>                                     bool botonCancelar(){ return false; }
> ```
>
> `J16` p10 y p12 pasaron a ser entradas de cámara el 31/08. **La puerta está tapiada por los dos
> lados**, y por Bluetooth no existe ningún comando que abra esa pantalla.

### 4.1 ✅ Cómo se leen HOY esos mismos bits: por Bluetooth

**Los mismos seis bits que pintaba la pantalla salen ahora en una trama de evento.** No hay que
pedirla con un comando nuevo: **el equipo la manda sola, justo detrás del rechazo**, que es cuando
hay alguien mirando.

1. Conéctese al equipo con la app y **mande la hora**: `CMD:PIN:1234:SET_RTC:…`
2. Si el equipo la rechaza, contesta **primero** el error y **detrás** los bits:

```text
   $ERR,CMD:SET_RTC,DESC:SIN_CRISTAL_VEA_CONSULTA_RELOJ
   $EVENT,NODE:MAESTRO,ORIGEN:RELOJ,DETALLE:ON:1 RDY:0 BYP:0 SEL:1 EN:1 CNT:0,HORA:--:--:--
```

3. **Léalo en la pestaña `Eventos`** de la app. Es la pestaña 2 y la ven los dos roles.

*(Medido: `Maestro/src/bluetooth.cpp:305-333` compone el detalle y `:542` / `:577` lo emiten, detrás
de `SIN_CRISTAL_VEA_CONSULTA_RELOJ` y de `SIGUE_PARADO_VEA_CONSULTA_RELOJ`. El mismo camino existe
en el Esclavo.)*

> ⚠️ **`CNT:--` no es `CNT:0`, y la diferencia es el diagnóstico.** `--` significa *no se pudo leer
> el contador* —el periférico no tiene reloj y leerlo sería un fallo de bus—; `0` significa *se leyó
> y vale cero*. Un cero en lugar de los guiones haría indistinguibles dos averías que mandan a
> sitios opuestos.
>
> 💡 **Para saber si el contador AVANZA hacen falta dos lecturas.** Repita el mismo `SET_RTC` al cabo
> de unos segundos y compare el `CNT`: si cambia, el RTC cuenta. No cuesta nada — en esta rama el
> comando se rechaza **antes** de escribir.

### 4.2 ~~Verificación con la pantalla LCD~~ ⛔ NO EJECUTABLE

*Se conserva porque describe qué significa cada diagnóstico, y ese significado sigue valiendo — es
lo que hoy dicen los bits `ON` / `RDY` / `BYP` de la trama de arriba.*

~~En el menú del semáforo, ingresar a **`CONFIGURACION` ➔ `CONSULTA RELOJ`**~~ (`lcd.cpp:488`):

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

> ✅ **RESUELTO EL 01/09 — el hueco que este apartado dejaba abierto ya tiene instrumento.**
>
> La versión anterior decía que *«este procedimiento funciona tal cual mientras la LCD siga
> montada»*, y avisaba de que el día que se retirara la pantalla el apartado se quedaría sin
> instrumento. **Ese día llegó antes de lo previsto y por otro camino:** la pantalla sigue montada y
> el firmware la sigue dibujando (`Maestro/src/main.cpp:46`, `lcd.cpp:483-500`), pero **`CONSULTA
> RELOJ` dejó de ser alcanzable** cuando los pulsadores 3 y 4 pasaron a ser cámaras.
>
> El instrumento nuevo es el `$EVENT,ORIGEN:RELOJ` del apartado 4.1. **No se decidió una pantalla
> nueva: se mandaron los mismos bits por el canal que ya tenía interfaz construida.**

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
| La LCD y `CONSULTA RELOJ` siguen **dibujándose** en el firmware de hoy | ✅ **MEDIDO EN EL FUENTE** (`main.cpp:46`, `lcd.cpp:483`) |
| …pero **`CONSULTA RELOJ` ya no es alcanzable**: necesita dos `botonAceptar()`, que devuelve `false` | ✅ **MEDIDO el 02/09** (`menu.cpp:111`, `:129`; `botones.cpp:280-281`) |
| Los seis bits salen hoy por `$EVENT,ORIGEN:RELOJ`, detrás de los dos `$ERR` que nombran esa pantalla | ✅ **MEDIDO el 02/09** (`Maestro/src/bluetooth.cpp:305-333`, emitido en `:542` y `:577`) |
| El `DS3231` va al ESP32 por `GPIO21`/`GPIO22` | 📖 **LEÍDO** en los documentos de decisión (doc 17 §1.3, `ESTADO.md:80`, `:124`). **Sin construir y sin hardware que medir** |
| Que retirar `R5` y montar la `CR2032` funcione en la tarjeta que usted tiene delante | 🔴 **NO VERIFICADO en esa tarjeta.** El procedimiento de los apartados 2-4 es el mismo desde el 26/08 y **la única medida de banco que existe es la del 01/08 sobre UNA tarjeta** |

> **Nada de esto ha pasado prueba de banco completa**, y este documento **no autoriza a instalar
> nada**. La única forma correcta de verificar el firmware es `01_Firmware/compuerta.py`, y un verde
> suyo **tampoco es un permiso**: dice que los modelos y los arneses de PC no encuentran nada, no que
> el firmware funcione en la tarjeta (`CLAUDE.md` §3).

---
*Manual técnico oficial de instalación de pila RTC V9.0. El «Plan B» del `DS3231` sobre `PB0`/`PB8` está ANULADO — apartado 5.*
