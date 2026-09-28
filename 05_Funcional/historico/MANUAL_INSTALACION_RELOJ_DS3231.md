# ⏱️ MANUAL TÉCNICO DE INSTALACIÓN — RELOJ RTC DS3231 Y BATERÍA (ESTÁNDAR BALIZA)

**Sistema:** Controladora de Semáforos Móviles de 3 Estados (Maestro y Esclavo V9.0)  
**Módulo RTC:** Módulo de Alta Precisión DS3231 TCXO (Mismo estándar probado en Proyecto Baliza)  
**Dónde se monta:** ~~Tarjeta STM32 del semáforo, pines `PB0` (SDA) y `PB8` (SCL) por I²C software~~ ⛔ **ANULADO el 31/08 — y además era FALSO: esos dos pines NO están libres.** El `DS3231` cuelga hoy del **ESP32** (`GPIO21` SDA / `GPIO22` SCL, con pila propia). Ver el aviso de abajo y el nuevo apartado 7  
**Propósito:** Sincronización horaria de precisión absoluta (±2 ppm) para Modo Degradado y Caja Negra  
**Verificación Hardware:** Esquemáticos KiCad `Controladora_Semaforos.kicad_sch`, `pines.h` y `03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md`  
**Fecha de Emisión:** 26 de Agosto de 2026  
~~**Última revisión:** 31 de agosto de 2026 — **el cableado a `PB0`/`PB8` de los apartados 2, 4 y 6 está ANULADO y NO se ejecuta.**~~
**Última revisión: 11 de septiembre de 2026 (tarde)** — 🔴 **`D-26`: la hora la manda el ESP32 de cada
poste** (construida en `68dd2c5`, sin banco): **se pone en LOS DOS postes**, y con la radio caída se va
al poste 2 a ponérsela. ~~**`D-20`: la hora se pone en el POSTE 1 y nunca en el poste 2**~~ *(revisión del
07/09, caducada)* (`DECISIONES.md`). Ver el recuadro del apartado 6 y la fila del desfase.
*7 de septiembre de 2026 (mañana)* — el cableado a `PB0`/`PB8` de los apartados 2, 4, 5
y 6 **sigue ANULADO** *(motivo original: mandaba meter un bus I²C en dos pines que ya tienen dueño en
el firmware que corre hoy, y uno de ellos es la entrada que **pide verde** — `N-105` de
[`roadmap.md`](../roadmap.md))*, **y además los apartados 7, 8 y 9 estaban CADUCADOS: decían que no
existe firmware que lea el `DS3231`, y existe.** Ver la cabecera de estado de abajo. **Nada se
borra:** lo caducado queda tachado en su sitio con el motivo, porque una vía descartada que
desaparece en silencio se vuelve a proponer.

> 🔴 **SEGUNDA PASADA DEL 07/09 (tarde) — la primera dejó vivas TRES copias de la frase que derogó.**
> Al derogar el apartado 8 —*«no hay driver»*— quedaron sin tocar el recuadro de cabecera del
> **apartado 5**, el sub-punto de su **paso 2**, y la última fila de la tabla del **apartado 6**, las
> tres repitiendo *«no hay software que lo lea»* / *«no es diagnosticable»*. **La fila del apartado 6
> era la peor: le decía al técnico que un reloj congelado no se puede diagnosticar, cuando el
> firmware contesta `OSCILADOR_PARADO_CAMBIE_PILA` con nombre y apellidos.**
>
> **La lección, que vale para cualquier documento largo: derogar un apartado no deroga a quien lo
> CITA.** Al tachar algo hay que buscar quién lo referencia —aquí, `grep -n "apartado 8"`— o la
> frase falsa sobrevive en los punteros, y con una fecha fresca encima.
>
> **Además, en esta pasada:** el recuento de `LEER_RTC` pasa de **siete a NUEVE** (recontado en el
> fuente, y la app coincide), se añade la respuesta **`MOTIVO_NO_CONTEMPLADO`** que faltaba, se
> escribe en grande que **el `$ACK` llega con `NODE:PUENTE` y no de la punta**, y se reparan **dos
> citas rotas a `ESTADO.md`** —una por número caducado y otra a una fila que ya no existe—.

---

> # 🛑 CABECERA DE ESTADO (07/09/2026) — EL RELOJ YA SE LEE. ESTE MANUAL DECÍA QUE NO
>
> **Lo que este documento afirmaba con la palabra «MEDIDO» encima, y que hoy es falso:** que *«no
> hay driver de `DS3231` en ninguna punta»*, que *«el del ESP32 tampoco existe»* y que *«el único
> fuente del ESP32 en el repositorio es `01_Firmware/Repetidor/src/main.cpp`»* (apartados 8 y 9).
>
> **Aquel `grep` era correcto el 31/08 y caducó cuando alguien escribió el firmware.** Medido el
> 07/09, y se cita el símbolo con el `grep` que lo encuentra:
>
> ```
> $ ls 01_Firmware/ESP32_Expansion/src/
> despachador.cpp  enlace_stm32.cpp  main.cpp  puente.cpp
> reloj_ds3231.cpp  trama.cpp  transporte_app.cpp  vigilante.cpp
>
> $ grep -rn "#include <Wire" 01_Firmware --include=*.cpp
> 01_Firmware/ESP32_Expansion/src/reloj_ds3231.cpp:5:#include <Wire.h>
>
> $ grep -n "DS3231_SDA\|DS3231_SCL" 01_Firmware/ESP32_Expansion/include/contrato.h
> 189:#define DS3231_SDA            21
> 190:#define DS3231_SCL            22
> ```
>
> | | |
> |---|---|
> | ✅ **El driver EXISTE** | `01_Firmware/ESP32_Expansion/src/reloj_ds3231.cpp`, sobre `Wire`, en `GPIO21`/`GPIO22` — exactamente los pines que el apartado 7 daba como *«decidido, sin construir»* |
> | ✅ **Y el reloj de ese módulo es HOY EL ÚNICO del cruce** | **`D-15` de [`DECISIONES.md`](../DECISIONES.md)** (05/09): *«el reloj lo lleva el ESP32 de cada punta, y es el ÚNICO que contesta a `SET_RTC`»*. El STM32 **no tiene reloj** (`Y2` muerto, N-17) y su rama de `SET_RTC` se consume **callada a propósito**, para que no salgan dos acuses a una sola orden |
> | ✅ **Y se puede CONSULTAR sin cambiarlo** | **`D-17`**: `CMD:LEER_RTC`, con ~~siete~~ **NUEVE** finales distintos, uno por motivo — *recontado el 07/09 en `ESP32_Expansion/src/despachador.cpp`; los documentos venían copiándose «siete» unos a otros* |
> | ✅ **Y hubo módulo en el banco** | **`A-5` cerrada el 05/09**: cada ESP32 lleva su reloj con pila propia y `HORA:22:19:58` es real. ⚠️ Sigue **`SIN VERIFICAR`** la dirección `0x68` sobre el módulo |
>
> ## 🔴 POR QUÉ ESTO ERA PELIGROSO Y NO SÓLO VIEJO
>
> El apartado 8 le decía al técnico, en letra grande, que **un `DS3231` mudo es «lo ESPERADO» y «no
> una avería»**. Con el firmware de hoy dentro, un módulo mudo **sí es una avería** —o una pila, o
> una dirección I²C, o un hilo— y el manual le estaba enseñando a no mirarla. Es el error de N-73
> girado del revés: antes se documentaba una función que no existía; aquí se documentaba la
> **ausencia** de una función que sí existe.
>
> ## ✅ LO QUE NO CAMBIA, y es la mitad que sigue protegiendo
>
> **`PB0` y `PB8` del STM32 siguen sin ser pines libres, y ahí no se cablea nada.** El aviso de
> seguridad de abajo **vale entero y no está caducado**: `PB0` es `CAM_DEMANDA_PIN` (bornera `J14`,
> la entrada que pide verde) y `PB8` es `LED_TESTIGO`. Lo que caducó es *«nadie lee el módulo»*, no
> *«el módulo no va al STM32»*.

---

## 🛑 AVISO DE SEGURIDAD — LÉASE ANTES DE COGER EL CAUTÍN (31/08/2026)

> ## ⛔ NO SE CABLEA NADA A `PB0` NI A `PB8`. NO ES UN MONTAJE DISPONIBLE: ES UN ERROR DE ESTE MANUAL.

Este documento —la copia de `04_Manuales/`— seguía **intacto y sin un solo aviso** mandando cablear
un bus I²C contra dos pines ocupados. **Es la SEGUNDA COPIA del mismo defecto:** la copia de
`05_Funcional/11_Manual_Instalacion_RTC_DS3231_Bateria.md` se corrigió la mañana del 31/08 (commit
`e1d3720`) y **ésta se quedó como estaba**. Se arregló el fichero que alguien nombró, no la propiedad.

**MEDIDO EN EL FUENTE** (`grep` sobre `pines.h`, línea idéntica en las dos puntas):

```
01_Firmware/Maestro/include/pines.h:46   #define CAM_DEMANDA_PIN    PB0  // -> R64 10K + C25 100nF -> bornera J14
01_Firmware/Esclavo/include/pines.h:46   #define CAM_DEMANDA_PIN    PB0  // (linea identica)
01_Firmware/Maestro/include/pines.h:63   #define LED_TESTIGO        PB8  // -> R16 1K -> LED D5. NO es entrada de camara
01_Firmware/Esclavo/include/pines.h:63   #define LED_TESTIGO        PB8  // (linea identica)
```

| pin | lo que dice este manual | lo que hay de verdad | nivel |
|---|---|---|---|
| `PB0` | «pin libre, `SDA` del I²C software» | **Entrada de la cámara de demanda**, bornera `J14`, con `R64` 10 kΩ de *pull-down* y `C25` de 100 nF de antirrebote | ✅ **MEDIDO EN EL FUENTE** (`pines.h:43-46`) |
| `PB8` | «pin libre, `SCL` del I²C software» | **`LED_TESTIGO`**: sale por `R16` de 1 kΩ al LED `D5`. No es bornera ni entrada optoacoplada | ✅ **MEDIDO EN EL FUENTE** (`pines.h:50-63`) |

**Qué le pasa al técnico que siga los apartados 2 y 4 tal como estaban:**

* En `PB0`, el `C25` de **100 nF** cuelga de la línea de datos: son **250 veces** el límite de carga
  capacitiva del I²C, así que **el bus no puede conmutar**. Y mientras tanto ese hilo entra en la
  **entrada de demanda de la cámara, que es la que solicita verde** en un equipo que gobierna un cruce.
  El fallo no sería «el reloj no anda»: sería **un accesorio metido en una entrada viva**.
* En `PB8`, `R16` de 1 kΩ **fija el nivel de la línea de reloj** contra el LED `D5`: un `SCL` que no
  puede subir. Y el firmware deja ese pin en alta impedancia **a propósito**
  (`01_Firmware/Maestro/src/modo_inteligente.cpp:50`).

> 🔴 **Es la TERCERA aparición del mismo error en este proyecto, y las tres tienen la misma forma:
> publicar un pin como libre sin cruzarlo contra `pines.h`.**
>
> * **`N-67`** — la entrada de cámara de `PB0` estaba **leída al revés**: `INPUT_PULLUP` contra el
>   *pull-down* de 10 kΩ de la placa deja el pin en 0,66 V, que el micro lee `LOW`. El firmware habría
>   visto **demanda permanente desde el arranque, sin ninguna cámara conectada**.
> * **`N-59`** — `PB8` se declaraba, se ponía en `INPUT_PULLUP` y **no se leía nunca**, mientras
>   **cuatro manuales** afirmaban que contaba entradas y salidas del tramo.
>
> **La regla que queda escrita: «pin libre» no es una observación, es una medida contra `pines.h`.**
> Un manual que llama libre a un pin ocupado no se equivoca en una palabra: **manda un cautín a una
> entrada viva.**

---

## 1. ¿Por qué se instala el Módulo Externo DS3231?

En las placas con microcontroladores STM32/CKS32 clonados, los cristales internos $Y_2$ (32.768 kHz) presentan problemas de oscilación o derivas térmicas de hasta ~17 segundos cada 48 horas.

El módulo **DS3231 (mismo equipo de Baliza)** resuelve esto definitivamente:
1. **Compensación Térmica Activa (TCXO):** Deriva **menos de 0.5 segundos cada 48 horas** en cualquier clima (-40°C a +85°C).
2. **Cero Fallos de Arranque:** Se comunica por bus digital I²C en menos de 2 ms al energizar.
3. **Independencia Eléctrica:** Mantiene la fecha y hora exacta funcionando durante más de 8 años con su batería de respaldo, incluso si el semáforo está totalmente apagado en bodega.

```
┌────────────────────────────────────────────────────────────────────────┐
│         GABINETE DEL SEMAFORO  --  DIAGRAMA ANULADO, NO EJECUTAR       │
│                                                                        │
│   ┌────────────────────────┐              ┌────────────────────────┐   │
│   │ TARJETA STM32 SEMAFORO │◄─X 4 Hilos X─│ MODULO RTC DS3231      │   │
│   │ (Pines PB0 / PB8)      │  (VCC/GND/   │ (Con Bateria LIR2032)  │   │
│   │  ^^^^^^^^^^^^^^^^      │   SDA/SCL)   │ (Estandar Baliza)      │   │
│   │  OCUPADOS: camara y    │              │                        │   │
│   │  LED testigo D5        │              │  --> hoy va al ESP32   │   │
│   └────────────────────────┘              └────────────────────────┘   │
│                                                                        │
│   El montaje vigente esta en el apartado 7 de este manual.             │
└────────────────────────────────────────────────────────────────────────┘
```

---

## 2. ⛔ ANULADO — ~~Diagrama de Cableado Pin a Pin en la Placa STM32~~

> ## 🛑 ESTE APARTADO NO DESCRIBE UN MONTAJE DISPONIBLE. DESCRIBE UN CABLEADO QUE NO SE HACE.
>
> **Dos motivos, y el segundo es el grave:**
>
> 1. **La decisión cambió:** el `DS3231` ya no va en el STM32. **Cuelga del ESP32** — apartado 7.
> 2. **Y aunque no hubiera cambiado, este cableado era ERRÓNEO desde el principio:** `PB0` y `PB8`
>    **no están libres**. Lo están en este manual, no en la placa. Ver el aviso de cabecera.
>
> **No se borra**, porque hubo una versión de este manual —**todas, hasta el 31/08**— que lo mandaba
> ejecutar, y hay que poder reconocerlo si alguien llega con una tarjeta ya cableada así.

~~Dado que los puertos I²C por hardware nativo de la placa están ocupados (`PB6`/`PB7` para la pantalla LCD y `PB10`/`PB11` para el bus RS-485), el módulo DS3231 se conecta mediante **I²C por Software** a los **pines libres `PB0` y `PB8`**:~~

```text
   ##############  DIAGRAMA ANULADO -- NO EJECUTAR ESTE CABLEADO  ##############

       MODULO RTC DS3231 (DE BALIZA)                 TARJETA SEMAFORO STM32
  ┌────────────────────────────────────┐         ┌───────────────────────────────┐
  │  [ VCC ]  (Alimentacion 3.3V) ─────┼─────────┼──► Pin 3.3V (o 5V)            │
  │  [ GND ]  (Tierra / Masa)    ──────┼─────────┼──► Pin GND (Tierra comun)     │
  │  [ SDA ]  (Datos I2C)        ──X───┼─X───────┼─X► Pin PB0   <-- OCUPADO:     │
  │                                    │         │      CAM_DEMANDA_PIN (J14),   │
  │                                    │         │      la entrada que PIDE VERDE│
  │  [ SCL ]  (Reloj I2C)        ──X───┼─X───────┼─X► Pin PB8   <-- OCUPADO:     │
  │                                    │         │      LED_TESTIGO (R16 -> D5)  │
  │  [ SQW ]  (No conectar)      ──────┼── (NC)  │                               │
  │  [ 32K ]  (No conectar)      ──────┼── (NC)  │                               │
  └────────────────────────────────────┘         └───────────────────────────────┘

   ##############  ANULADO EL 28/08 (decision) Y CORREGIDO EL 31/08 (error)  #####
```

### ~~Tabla de Conexión Física~~ — ⛔ ANULADA

| Pin Módulo DS3231 | Color Recomendado | ~~Pin en Tarjeta STM32~~ | Estado |
|---|---|---|---|
| **`VCC`** | 🔴 Rojo | ~~`3.3V` (o `5V`)~~ | ⛔ no se cablea al STM32 |
| **`GND`** | ⚫ Negro | ~~`GND`~~ | ⛔ no se cablea al STM32 |
| **`SDA`** | 🟢 Verde / Azul | ~~**`PB0`**~~ | ⛔ **`PB0` = entrada de cámara de demanda** |
| **`SCL`** | 🟡 Amarillo / Blanco | ~~**`PB8`**~~ | ⛔ **`PB8` = `LED_TESTIGO`, salida a LED `D5`** |

**Por qué el error no se veía:** la propuesta salió de `N-37`, cuando el cristal `Y2` se declaró
muerto y esos dos pines **aún no tenían función** — el manual tenía razón *en su momento*. **`V9.0` se
los dio a las cámaras**, y la frase se quedó escrita describiendo una placa que ya no existe. Es
exactamente lo que advierte `CLAUDE.md` §3.bis: los comentarios y los manuales **no fallan cuando
alguien cambia un número**, se quedan **con la autoridad de una cuenta hecha**.

---

## 3. ⚠️ REGLA CRÍTICA SOBRE LA PILA / BATERÍA (Evitar Daños)

Existen dos tipos de pilas de botón y no deben confundirse:

```text
  ┌─────────────────────────────────────┬─────────────────────────────────────┐
  │ TIPO 1: PILA RECARGABLE (LIR2032)   │ TIPO 2: PILA NO RECARGABLE (CR2032) │
  │ Voltaje: 3.6V - 4.2V                │ Voltaje: 3.0V                       │
  │ ✅ Apta para módulo DS3231 estándar  │ ⚠️ Requiere anular diodo de carga   │
  └─────────────────────────────────────┴─────────────────────────────────────┘
```

### ¿Cómo proceder según la pila que tenga en taller?

* **Opción A (Recomendada - Con Batería Recargable LIR2032):**
  * Colocar la batería **LIR2032 (3.6V)** en el portapilas del módulo DS3231.
  * El circuito de carga integrado del módulo mantendrá la batería cargada automáticamente mientras el semáforo esté encendido.
* **Opción B (Si solo tiene Pila Común CR2032 no recargable):**
  * Los módulos comerciales DS3231 (ZS-042) traen un diodo y una resistencia de 200 $\Omega$ que intentan cargar la pila.
  * **Acción obligatoria:** Desoldar o levantar con el cautín el diodo SMD marcado como `D1` o la resistencia `R1` del módulo DS3231. Con esto, el módulo funcionará como lector pasivo de la pila CR2032 durante 8 años sin peligro de sobrecargarla.

---

## 4. Guía de Instalación y Soldadura en Taller (Paso a Paso)

> 🛑 **El paso 2 de esta lista está ANULADO.** Los pasos 1, 3 y 4 siguen siendo válidos, pero **contra
> el ESP32** (apartado 7), no contra la tarjeta del semáforo.

1. **Fijación Mecánica:** ✅ **sigue válido**
   * Montar el módulo DS3231 en un separador plástico o cinta doble faz espumada dentro del gabinete
     del semáforo, **junto al ESP32** que lo va a leer.
2. ~~**Soldadura / Conexión de Cables:**~~ ⛔ **ANULADO**
   * ~~Conectar los 4 cables flexibles (24 AWG) desde el módulo DS3231 hacia la bornera o pines de cabecera de la placa (`3.3V`, `GND`, `PB0`, `PB8`).~~
   * 🛑 **NO se conecta ningún hilo del `DS3231` a la tarjeta STM32.** `PB0` es la entrada de la cámara
     de demanda y `PB8` es el `LED_TESTIGO` (aviso de cabecera, **MEDIDO**). El destino vigente es el
     **ESP32**: `GPIO21` (SDA) y `GPIO22` (SCL) — apartado 7.
3. **Verificación con Multímetro (Antes de Energizar):** ✅ **sigue válido** (referida a la masa del módulo)
   * Medir continuidad: `GND` del módulo debe tener continuidad ($0.0\ \Omega$) con el `GND` general de la placa.
   * Confirmar que no haya cortocircuito entre `3.3V` y `GND`.
4. **Verificación con Tarjeta Apagada:**
   * Con el semáforo desconectado de la fuente de 12V/24V, medir con el multímetro entre el pin positivo (+) de la pila del DS3231 y `GND`:
   * **Lectura esperada:** Entre **`3.0 V` y `3.6 V`**.

---

## 5. ⛔ ANULADO — ~~Protocolo de Validación en Banco de Pruebas (3 Minutos)~~

> ~~🛑 **Este protocolo NO SE PUEDE APROBAR HOY, y no porque el módulo esté mal: porque no hay software
> que lo lea.** Ver el apartado 8. Un técnico que lo ejecutara concluiría que el módulo está
> defectuoso, devolvería una unidad buena y gastaría la sesión buscando una avería que no existe.~~
>
> # 🛑 SEGUNDA CORRECCIÓN DEL 07/09 — ESTE RECUADRO SE QUEDÓ DICIENDO LO QUE SE ACABABA DE DEROGAR
>
> **La pasada de la mañana del 07/09 derogó el apartado 8 —*«sí hay driver»*— y este recuadro, que
> **apunta al apartado 8**, siguió diciendo *«no hay software que lo lea»*. Es la misma frase falsa,
> dos pantallas más arriba, y con la palabra «HOY» encima para que pareciera fresca.**
>
> **Lo que sigue siendo cierto y por qué el protocolo SIGUE ANULADO — pero por OTRO motivo:** el
> cableado que este protocolo valida es el de `PB0`/`PB8`, y **ése no se hace**. El módulo cuelga del
> **ESP32** (apartado 7). Un técnico que ejecute los pasos 1 a 3 tal como están **mide un montaje que
> nadie va a construir**.
>
> ✅ **Lo que se hace en su lugar, y sí es ejecutable hoy:** conectarse por Bluetooth y mandar
> **`CMD:LEER_RTC`**. Contesta el **puente**, con **nueve** finales distintos y una acción por cada
> uno — tabla completa en el **apartado 8**. Eso es la validación de banco del reloj.

~~Una vez cableado el módulo DS3231 en la placa:~~

1. ~~**Encendido:** Energizar la tarjeta STM32 ➔ El LED de encendido del módulo DS3231 debe iluminar fijo.~~
   ⚠️ El LED **sí** encenderá: sólo indica que el módulo tiene tensión, **no** que alguien lo esté leyendo.
2. ~~**Puesta en Hora:** Desde la pantalla LCD, ingresar a `CONFIGURACION > AJUSTAR HORA`…~~
   ⛔ **Doblemente anulado, y el segundo motivo es nuevo:**
   - **`AJUSTAR HORA` pone en hora el RTC INTERNO del STM32** (cristal `Y2`, `PC14`/`PC15`), **no el
     `DS3231`**. ~~No hay driver que escriba en el módulo — apartado 8.~~ ⛔ **CADUCADO EL 07/09 (era
     el mismo error dos veces): SÍ hay driver — `reloj_ajustar()` en
     `ESP32_Expansion/src/reloj_ds3231.cpp`, llamado desde `despachador.cpp`.** Lo que sigue siendo
     cierto es que **`AJUSTAR HORA` no lo toca**: escribe en el `Y2` del STM32, que es otro reloj y
     está muerto (N-17).
   - 🛑 **Y desde el 31/08 esa pantalla NO SE PUEDE ABRIR.** Está dentro de `CONFIGURACION` y llegar
     ahí necesita **dos pulsaciones de *Aceptar***; `botonAceptar()` devuelve `false` siempre desde
     que `J16` p10 y p12 son entradas de cámara. **Lo mismo vale para `CONSULTA RELOJ`.** ✅ **Se cita
     el símbolo, no el número de línea** — *y la prueba de por qué está aquí mismo: decía
     `botones.cpp:280-281`, la pasada de la MAÑANA del 07/09 lo «arregló» a `:659-660`, y **esa tarde
     ya era `:672`** porque un commit intermedio (`4b2841b`) insertó comentarios encima. **Renumerar
     es la cura equivocada; se publica el `grep` SIN `-n`**:*

     ```
     $ grep -h "^bool botonAceptar" 01_Firmware/Maestro/src/botones.cpp 01_Firmware/Esclavo/src/botones.cpp
     bool botonAceptar() { return false; }
     bool botonAceptar() { return false; }
     ```

     🛑 **Y desde el 05/09 hay un motivo más, que ya no es de botones: la pantalla NO SE MONTA**
     (`D-17.bis`). No es que el menú sea inalcanzable — es que no hay cristal donde mirarlo.

   **La hora se pone hoy desde la app:** `CMD:PIN:1234:SET_RTC:YYYY-MM-DD,HH:MM:SS`, y **hay que leer
   la respuesta**. ⛔ ~~tiene cinco ramas~~ **CORREGIDO EL 07/09: son OCHO, y las contesta el PUENTE
   con `NODE:PUENTE`, no el STM32** (`D-15`).

   > 🔴 **Y ESTO ES LO QUE MÁS DAÑO HACE SI SE LEE MAL, así que va aquí y no en una nota:**
   > **el `$ACK` NO llega con `NODE:MAESTRO` ni `NODE:ESCLAVO`. Llega con `NODE:PUENTE`.** Las ramas
   > de `SET_RTC` de las dos puntas del STM32 **consumen la orden en silencio, a propósito**, para
   > que no salgan dos acuses opuestos a una sola orden.
   >
   > **Un técnico que espere el acuse de la punta creerá que el comando se perdió, repetirá el
   > `SET_RTC` tres veces, y acabará cambiando la pila y el módulo — que están sanos.** *«El STM32 no
   > contesta»* **no es un síntoma: es el diseño.** Este proyecto ya pagó una vez el error contrario
   > —un `$ACK` que decía que sí y no ponía la hora—; éste es su reverso.
   >
   > | dos respuestas de éxito, y **no significan lo mismo** | |
   > |---|---|
   > | `$ACK,NODE:PUENTE,CMD:SET_RTC,RESULT:OK` | entró **y** ~~**va camino de la otra punta**~~ *(11/09, `D-26`: la línea `CMD:HORA_ESP32` con esa hora salió entera hacia **la controladora de ESTE poste**; en el poste 1, ésa la pasa al poste 2 por radio)* |
   > | `$ACK,NODE:PUENTE,CMD:SET_RTC,RESULT:HORA_PUESTA_SIN_PROPAGAR` | **entró aquí y NO viajó** ~~**.** El otro poste sigue con su hora vieja: hay que ir a ponérsela~~ *(11/09: «viajar» es a la controladora de este mismo armario por `J17`, no al otro poste. El reloj está bien; falla el enlace de dentro. Si se repite, alarma `HORA_ESP32`)* |
   >
   > **Las seis negativas y qué hacer con cada una: apartado 8 de este manual.**

   ⚠️ ~~Ver `05_Funcional/11_Manual_Instalacion_RTC_DS3231_Bateria.md` §4.~~ **Ese puntero manda a la
   tabla del STM32, que es la que acaba de caducar.** El censo vigente de las respuestas del puente
   está en **`04_Manuales/MANUAL_CONFIGURACION_BLUETOOTH.md` §4.4** y, en versión de diagnóstico, en
   el **apartado 8** de este manual.
3. ~~**Prueba de Corte de Energía:** … la hora debe marcar exactamente `18:02:00`.~~
   ⛔ Esa prueba mide la **pila `CR2032` del `VBAT` del STM32**, que es **otra pila y otro reloj**. El
   procedimiento correcto para ésa está en
   [`05_Funcional/11_Manual_Instalacion_RTC_DS3231_Bateria.md`](../05_Funcional/11_Manual_Instalacion_RTC_DS3231_Bateria.md),
   apartados 2 a 4 (retirar `R5` antes de colocar la pila — **peligro de que reviente si no se hace**).

---

## 6. Diagnóstico de Fallas (Troubleshooting)

| Síntoma | Causa Probable | Solución |
|---|---|---|
| ~~**La pantalla muestra `CONSULTA RELOJ: I2C No Responde`**~~ | ~~Cables SDA o SCL invertidos o sueltos.~~ | ⛔ **FILA ANULADA.** ~~Verificar que `SDA` esté en `PB0` y `SCL` esté en `PB8`.~~ **No se verifica nada en `PB0`/`PB8`: no va nada ahí.** `CONSULTA RELOJ` informa del **RTC interno del STM32** (cristal `Y2`), que **no tiene ninguna relación con el `DS3231`** |
| **El módulo `DS3231` no responde / parece muerto** | ⛔ ~~Es lo esperado hoy: no hay driver~~ → **CADUCADO EL 07/09: SÍ hay driver** (`ESP32_Expansion/src/reloj_ds3231.cpp`), así que **es una avería de verdad** | **Pregúntele al puente, no al LED:** `CMD:LEER_RTC` contesta el motivo concreto —bus mudo, pila, hora nunca puesta, escritura a medias—. Tabla completa en el apartado 8 |
| **El reloj pierde la hora cada vez que se apaga el semáforo** | Pila agotada o mal colocada en el módulo. | Medir la pila con multímetro (>3.0V) y verificar que el polo positivo (+) quede hacia arriba. ✅ **sigue válido** |
| **La hora no avanza (se queda congelada)** | ⛔ ~~Módulo DS3231 dañado o sin cristal activo~~ → **el oscilador está parado, y casi siempre es la PILA** | ⛔ ~~**Sólo aplica cuando exista firmware que lo lea.** Hoy la hora del módulo no se lee desde ningún sitio, así que este síntoma **no es diagnosticable**~~ 🔴 **CADUCADO EL 07/09 — ver abajo.** ✅ **Hoy SÍ es diagnosticable y el equipo lo dice con nombre propio:** `CMD:LEER_RTC` → `$ERR,…,DESC:OSCILADOR_PARADO_CAMBIE_PILA`. **Primero la lectura, después la pieza** |
| **La app enseña un desfase grande entre los dos postes** | Son **dos relojes independientes** —uno por ESP32— y **hoy nadie los sincroniza solo** (`D-17`) | `CMD:LEER_RTC` en los dos postes. 🔴 ~~si uno se ha ido, `SET_RTC` **en ése**~~ ~~⛔ **DEROGADO EL 07/09 POR `D-20`: la hora se corrige SIEMPRE en el poste 1 (Maestro), NUNCA en el poste 2.** Poner la hora en el Esclavo **no es sincronizar: es crear una segunda fuente**, que es justo lo que `D-20` prohíbe.~~ 🔵 **11/09 (`D-26`): si uno se ha ido, `SET_RTC` en ése — también en el poste 2.** La radio no escribe el `DS3231` del poste 2, y ése desfase es el salto que dará su hora al perder la radio. ⚠️ **Un desfase no es una avería del módulo**: es lo que `D-17` existe para poder ver |
| **El puente contesta `MOTIVO_NO_CONTEMPLADO`** | Es la **rama de cierre** del firmware: un estado que no estaba previsto | 🛑 **NO se cambia la pila ni el módulo.** Se anota la trama literal y se reporta: es firmware, no cobre |

> # 🛑 ANTES DE TOCAR NADA DE ESTA TABLA: A QUIÉN SE LE PREGUNTA POR EL RELOJ (`D-15`)
>
> **Se le pregunta al PUENTE, por Bluetooth, con `CMD:LEER_RTC`. No al STM32.**
>
> El Maestro y el Esclavo **no tienen reloj** —el cristal `Y2` está muerto (N-17)— y sus ramas de
> `SET_RTC` ~~**están calladas a propósito**~~ *(11/09, `D-26`: **ya no existen** — la orden se queda en
> el puente y no llega al STM32)* para que no salgan dos acuses opuestos a una sola orden.
> **Que el STM32 no conteste no es un síntoma: es el diseño.**
>
> 🔴 **Este es el error que este manual existe para impedir, y este proyecto ya lo pagó:** el técnico
> manda `SET_RTC`, no ve respuesta de la punta, concluye *«el reloj está muerto»* y **cambia pila y
> cristal que están sanos**. La respuesta estaba llegando todo el rato, con `NODE:PUENTE` delante.
>
> ✅ **Y la herramienta buena es `LEER_RTC`, no `SET_RTC`** (`D-17`): **lee sin cambiar**. Hasta que
> existió, la única forma de mirar el reloj era mandarlo — y con eso se perdía justo el dato que se
> buscaba. **Primero la lectura, después la pieza.**
>
> ⚠️ **Y hay DOS relojes por cruce, uno por poste.** Un poste en hora no dice nada del otro: la app
> compara los dos y enseña el desfase, porque **los dos ESP32 no se hablan entre sí**.

> # 🔴 `D-26` (11/09) — LA HORA LA MANDA EL ESP32 DE CADA POSTE · ~~`D-20` (07/09) — LA HORA SE PONE EN EL POSTE 1. EN EL POSTE 2 NUNCA~~
>
> Filas **`D-20`** y **`D-26`** de [`DECISIONES.md`](../DECISIONES.md). **La autoridad de la hora es el
> ESP32** ~~**, siempre y para todo, y es UNA SOLA:** la app se la da al **ESP32 Maestro**, ése al
> **ESP32 Esclavo**, y el STM32 de cada punta la recibe **de su propio ESP32**. **El Maestro manda la
> hora y el Esclavo hace caso siempre.**~~ **de cada poste**: cada uno le pasa la de su `DS3231` a su
> controladora. **Con radio, el poste 2 hace caso a la del Maestro; SIN radio (25 s), a la de su
> propio `DS3231`.** Construida en `68dd2c5`, **sin banco**.
>
> ~~🔴 **Consecuencia para el técnico, y es la que cambia lo que usted hace con el multímetro en la
> mano: un `SET_RTC` dirigido al poste 2 SE RECHAZA. No es una sincronización, es una segunda
> fuente.** Si el poste 2 va desfasado, **la hora se corrige en el poste 1** y viaja
> `ESP32-M -> STM32-M -> radio -> STM32-E -> ESP32-E`~~ 🔴 **Consecuencia para el técnico: al poste 2
> SÍ se le pone la hora**, en la puesta en marcha y **cuando se cae la radio** (`D-26` (5)). Con radio
> la hora viaja `ESP32-M -> STM32-M -> radio -> STM32-E` — los STM32 quedan de **carteros** de la hora,
> no de dueños—; **la radio no escribe el `DS3231` del poste 2**. Qué hacer ante cada alarma:
> `05_Funcional/14_Manual_App_Movil_IOT_VIAL.md` §5.3.bis.
>
> ~~⚠️ **`D-20` está DECIDIDA Y SIN CONSTRUIR (07/09).**~~ Medido *(el 07/09, crónica)*: el despachador del puente es el mismo
> firmware en los dos postes y **no nombra al Esclavo ni una vez** *(y con `D-26` no hace falta)*:
>
> ```
> $ cd 01_Firmware/ESP32_Expansion
> $ grep -c "ESCLAVO\|Esclavo\|esclavo" src/despachador.cpp
> 0
> ```
>
> Así que **hoy sigue atendiendo el `SET_RTC` venga por donde venga** *(11/09: y es lo correcto con
> `D-26`)*. ~~Mientras eso siga así, **el poste 2 se pone en hora a mano y visitándolo**, porque no hay
> otra vía — y eso es un estado temporal declarado, no la arquitectura.~~ **El poste 2 se pone en hora
> visitándolo**, y es el procedimiento, no un estado temporal.
>
> ✅ **Y la regla: EL POSTE 2 SE PONE EN HORA EN LA PUESTA EN MARCHA** ~~**, NO DURANTE LA AVERÍA.**
> Con `D-20` construida, el único camino a su reloj pasa por la radio — y la radio se cae justo cuando
> hace falta el Degradado, que es el modo que exige hora.~~ **Y CON LA RADIO CAÍDA, ALLÍ MISMO**
> (`D-26`). **Su `DS3231` tiene pila y conserva la hora que ya tenía.** *Perder la radio no es perder
> la hora.* Por eso se pone también **antes**: en la puesta en marcha, al cambiar la pila del módulo
> ~~la `CR2032`~~, y después de cualquier `OSCILADOR_PARADO_CAMBIE_PILA` en ese poste.

> 🔴 **Por qué la fila *«la hora no avanza»* estaba caducada, y va escrito para que no vuelva.** La
> pasada de la mañana del 07/09 derogó el apartado 8 —*«no hay driver»*— **y esta fila, que decía lo
> mismo con otras palabras, sobrevivió dos filas más abajo de la que sí se corrigió.** Decía que un
> reloj congelado *«no es diagnosticable»*, cuando el firmware contesta
> `OSCILADOR_PARADO_CAMBIE_PILA` con nombre y apellidos. **Enseñaba a no mirar una avería que el
> equipo ya sabe nombrar** — el mismo defecto que la cabecera de este manual denuncia, en la página
> de al lado.

---

## 7. ✅ Dónde vive HOY el reloj externo: colgado del ESP32, fuera de la tarjeta

> ⛔ **AQUÍ DECÍA: *«~~📖 LEÍDO en los documentos de decisión, NO medido en hardware — no hay hardware
> que medir todavía: el módulo es la línea de compra `A6` y no se ha comprado~~»*. CADUCADO, y se
> tacha con su motivo (07/09):**
>
> * **El montaje ya está MEDIDO en el fuente, no leído en una decisión:** `DS3231_SDA 21` /
>   `DS3231_SCL 22` en `ESP32_Expansion/include/contrato.h`, con driver en `reloj_ds3231.cpp`.
> * **Y hubo módulo en el banco.** `A-5` de [`DECISIONES.md`](../DECISIONES.md), cerrada el 05/09:
>   *«sí — cada ESP32 lleva su reloj con pila propia, y así estaba escrito desde el 28/08 en la
>   lista de compras»*. ⚠️ Lo que sigue **`SIN VERIFICAR`** es la dirección `0x68` sobre el módulo.

| dato | fuente |
|---|---|
| El `DS3231` cuelga del **ESP32** por I²C: **`GPIO21` = SDA, `GPIO22` = SCL**, con **pila propia**. El módulo `ZS-042` trae sus *pull-ups* | ✅ **MEDIDO EN EL FUENTE (07/09):** `DS3231_SDA`/`DS3231_SCL` en `ESP32_Expansion/include/contrato.h`. *(Decidido en `05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md` §1.3)* |
| «El ESP32 es un módulo de expansión y **no manda sobre las luces**» | ⛔ ~~`ESTADO.md:80`~~ **el número caducó (07/09): `ESTADO.md` se reescribió entero y hoy son 303 líneas.** Se cita por la frase: `grep -n "no manda sobre las luces" ESTADO.md` |
| ~~La fila `PIN-0` —*«`PB0`/`PB8` van a bus I²C»*— está **⛔ ANULADA**~~ | 🔴 **PUNTERO ROTO, corregido el 07/09: esa fila YA NO EXISTE en `ESTADO.md`** —`grep -n "PIN-0" ESTADO.md` → **cero líneas**—, así que la cita apuntaba a un sitio y al vacío a la vez. **La fuente vigente de que el I²C no vive en el STM32 es `D-15` de [`DECISIONES.md`](../DECISIONES.md)**, que gana a cualquier documento en lo decidido, y el aviso `PB0`/`PB8` de la cabecera de este manual, que está medido contra `pines.h` |
| **Los módulos ESTÁN COMPRADOS: son DOS, uno por poste, y con pila propia** | **`A6` de `05_Funcional/15_Lista_de_Compras_Hardware.md`** (cubierta el 05/09) y **`A-5` de `DECISIONES.md`** (cerrada). ⚠️ Lo que sigue **`SIN VERIFICAR`** es la dirección `0x68` sobre el módulo real |

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
 │   manda.                                                                    │
 └─────────────────────────────────────────────────────────────────────────────┘
```

**Lo que NO cambia con la mudanza** — es una propiedad del módulo, no de dónde se enchufe:

* **La regla de la pila del apartado 3 sigue vigente entera.** `LIR2032` recargable, o `CR2032` con el
  diodo/resistencia de carga del módulo desoldado.
* ⚠️ **Ojo con confundir las dos pilas de este proyecto:** la de este manual es la **del módulo
  `DS3231`** y puede tener que ser recargable. La otra es la **`CR2032` del `VBAT` del STM32**, que
  **nunca es recargable** y exige retirar `R5` antes (Manual 11, apartado 2). Son pilas distintas en
  equipos distintos.

---

## 8. ~~🛑 Un `DS3231` conectado hoy se queda MUDO, y eso es lo ESPERADO — no una avería~~ 🔴 **APARTADO DEROGADO EL 07/09: YA HAY QUIEN LO LEA**

> # 🔴 ESTE APARTADO ENSEÑABA A NO MIRAR UNA AVERÍA. Se tacha con su motivo, no se borra
>
> **Decía, en letra grande, que un `DS3231` mudo era «lo ESPERADO» y «no una avería».** Con el
> firmware de hoy dentro, **un `DS3231` mudo SÍ es una avería** — pila, dirección I²C, hilo o
> módulo—, y este apartado le estaba diciendo al técnico que se fuera del poste sin mirarla.
>
> **El `grep` que lo sostenía era correcto el 31/08 y caducó cuando alguien escribió el firmware.**
> Re-medido el 07/09, con la salida literal:
>
> ```
> $ grep -rn "#include <Wire" 01_Firmware --include=*.cpp
> 01_Firmware/ESP32_Expansion/src/reloj_ds3231.cpp:5:#include <Wire.h>
>
> $ grep -n "DS3231_SDA\|DS3231_SCL" 01_Firmware/ESP32_Expansion/include/contrato.h
> 189:#define DS3231_SDA            21
> 190:#define DS3231_SCL            22
> ```
>
> **Sigue siendo cierta la mitad del STM32** —`grep -rni ds3231` sobre `Maestro/src` y `Esclavo/src`
> no encuentra driver, y no debe encontrarlo: el reloj **no vive ahí** (`D-15`)—. Lo falso era la
> otra mitad: *«el del ESP32 tampoco existe»*.

### ✅ Lo que hay que hacer HOY si el módulo no contesta

**El firmware del puente no se calla: dice el motivo.** `CMD:LEER_RTC` consulta el reloj **sin
cambiarlo** (`D-17`) y contesta uno de estos, cada uno con su acción distinta. Léase la respuesta;
no se diagnostique por el LED del módulo, que sólo indica que hay tensión.

| respuesta del puente | qué significa | qué se hace |
|---|---|---|
| `$ACK,NODE:PUENTE,CMD:LEER_RTC,RESULT:OK,…` | el reloj contesta y la hora es fiable | nada |
| `$ERR,…,DESC:SIN_RELOJ_NO_RESPONDE` | **el bus I²C está mudo** | ahí sí: hilos `SDA`/`SCL`, masa común, alimentación, módulo |
| `$ERR,…,DESC:OSCILADOR_PARADO_CAMBIE_PILA` | el módulo está, la pila no | apartado 3 de este manual |
| `$ERR,…,DESC:NUNCA_SE_PUSO_PONGA_LA_HORA` | módulo sano, nadie le puso la hora | `CMD:PIN:1234:SET_RTC:…` |
| `$ERR,…,DESC:ESCRITURA_A_MEDIAS_REPITA_SET_RTC` | una puesta en hora se cortó | repetir `SET_RTC` |
| `$ERR,…,DESC:MODO_12H_PONGA_LA_HORA` · `REGISTROS_INCOHERENTES` · `BARRERA_INCOHERENTE` | registros del módulo en un estado que el puente no acepta | volver a poner la hora |
| 🆕 `$ERR,…,DESC:MOTIVO_NO_CONTEMPLADO` | **rama de cierre: el puente prefiere admitir que no sabe antes que inventarse un `OK`** | 🛑 **NO cambie pila ni módulo.** Anote la trama literal y repórtela: es firmware, no cobre |

> 🛑 **Añadida el 07/09, y no es una fila de relleno.** Faltaba **la única respuesta que un técnico
> no puede interpretar solo**, y su ausencia empuja justo al gesto que este manual existe para
> impedir: cambiar componentes sanos ante una respuesta que no se entiende. **Son NUEVE finales**
> —un `$ACK,RESULT:OK` y ocho `$ERR`—, contados en `ESP32_Expansion/src/despachador.cpp` el 07/09.
> Los documentos venían diciendo *«siete»*; **el firmware y la app coinciden en nueve** (la app
> traduce las nueve, `MOTIVO_NO_CONTEMPLADO` incluida), **así que el número equivocado era el del
> papel.**

> ⚠️ **Y el contrario, que también hay que saber:** *«el `DS3231` no se lee»* **no se diagnostica
> desde el STM32**. El Maestro y el Esclavo **no tienen reloj** y su rama de `SET_RTC` está
> **callada a propósito** (`D-15`), así que no contestar **no es un síntoma**: es el diseño. Quien
> contesta es el puente.

> 🟡 **Lo que este apartado NO puede decir, y por eso no lo dice:** nada de esto se ha ejercido
> contra un `DS3231` de este proyecto con un multímetro delante. La dirección `0x68` sobre el
> módulo sigue **`SIN VERIFICAR`** (`A-5`). Lo medido es **que el software existe y qué contesta**,
> no que el módulo responda en la tarjeta.

---

## 9. 🛑 Nivel de prueba de este manual — no es un permiso para instalar

| lo que este manual afirma | nivel |
|---|---|
| `PB0` = cámara de demanda · `PB8` = `LED_TESTIGO` | ✅ **MEDIDO EN EL FUENTE** (símbolos `CAM_DEMANDA_PIN` y `LED_TESTIGO` en `pines.h`, las dos puntas) |
| ~~No hay driver de `DS3231` ni I²C en ninguna punta, ni en el ESP32~~ → **no lo hay en el STM32, y SÍ lo hay en el ESP32** | ⛔ **LA FILA ANTERIOR FUE FALSA CON UN «MEDIDO» AL LADO** desde que se escribió `ESP32_Expansion`. ✅ Lo medido hoy (07/09): `reloj_ds3231.cpp` con `#include <Wire.h>`, y `DS3231_SDA`/`DS3231_SCL` en `contrato.h` |
| La regla de la pila `LIR2032` / `CR2032` del módulo | 📖 **LEÍDO** de la hoja del módulo comercial `ZS-042`. **No verificado sobre una unidad de este proyecto** |
| ~~El `DS3231` va al ESP32 por `GPIO21`/`GPIO22`: LEÍDO, sin construir, sin comprar~~ → **construido** | ✅ **MEDIDO EN EL FUENTE** (`DS3231_SDA 21` / `DS3231_SCL 22`, `contrato.h`). Y hubo módulo en banco: `A-5` cerrada el 05/09 |
| ~~Que el reloj externo llegue alguna vez a poner en hora el semáforo: NO EXISTE~~ | ⛔ **CADUCADA.** Hoy el reloj del ESP32 es **el único del cruce** y el único que contesta a `SET_RTC` (**`D-15`**), y se puede consultar con `CMD:LEER_RTC` (**`D-17`**) |
| Que un `DS3231` mudo sea normal *(lo que decía el apartado 8)* | ⛔ **DEROGADO EL 07/09.** Hoy es un síntoma, y el puente dice de qué. Ver el apartado 8 |
| Que el módulo conteste en la tarjeta, en la dirección `0x68` | 🟡 **SIN VERIFICAR** (`A-5`). Medido está el software, no el cobre |

> **Nada de esto ha pasado prueba de banco completa**, y este documento **no autoriza a instalar
> nada**. La única forma correcta de verificar el firmware es `01_Firmware/compuerta.py`, y un verde
> suyo **tampoco es un permiso**: dice que los modelos y los arneses de PC no encuentran nada, no que
> el firmware funcione en la tarjeta (`CLAUDE.md` §3).

---
*Manual técnico de instalación del módulo RTC DS3231 para semáforos móviles V9.0. El cableado a `PB0`/`PB8` del STM32 está ANULADO — apartados 2, 4, 5 y 6.*
