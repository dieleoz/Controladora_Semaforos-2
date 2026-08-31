# 📱 MANUAL TÉCNICO Y ESPECIFICACIÓN INTEGRAL DE LA APP MÓVIL Y BLUETOOTH (V9.0)

**Sistema:** Controladora de Semáforos Móviles de 3 Estados (Maestro y Esclavo V9.0)  
**Transporte de Diagnóstico:** Bluetooth Serial **SPP (Clásico)** — **no BLE** (Estándar Probado en Proyecto Baliza). **El transporte NO cambia.**  
**Módulo que lo lleva:** ~~`HC-05` / `JDY-30` dedicado~~ → **`ESP32` clásico haciendo de puente SPP** *(decisión de obra del 28/08, ver §1.9)*. 🛑 **Condicionado a `BLQ-1`: la referencia del ESP32 que llegó a obra sigue SIN LEER.**  
**Software Móvil:** App Android (.apk) con Frontend Reactivo Dark-Theme (Estándar IOT-VIAL)  
**Propósito:** Telemetría en tiempo real, caja negra de alarmas, test de banco, sincronización Courier RTC y control desde el suelo con PIN  
**Verificación Hardware:** Esquemáticos KiCad `Controladora_Semaforos.kicad_sch`, `pines.h` y `MAPEO_TARJETA_KICAD.md`  
**Fecha de Emisión:** 26 de Agosto de 2026  
**Última revisión:** 28 de Agosto de 2026 — **el apartado 2 se redibujó entero: el módulo entra por
el conector `J17` (`USART1` REMAPEADO a `PB6`/`PB7`), NO por `PA9`/`PA10`.** Además, en el mismo
apartado: corrección del transceptor `U3` → **`U2`**, y el `PA8` reclasificado como **residuo
pendiente de revisar** (su motivo original desapareció con el remapeo). En el apartado 1: criterio
de compra del módulo (ESP32 sí/no), consumo y puesta a punto con `AT+NAME`.

> ⚠️ **Sobre la revisión anterior de esta misma fecha, que se quedó a medias.** Corrigió `U3` → `U2`
> y anunció esa corrección aquí, **pero dejó intacto el pinout `PA9`/`PA10` del mismo apartado 2**,
> que ya estaba obsoleto. El resultado era peor que no haber tocado nada: el apartado quedó con la
> referencia del chip bien y el cableado mal, y el detalle corregido en la cabecera daba a entender
> que el apartado entero estaba revisado. **Esta cabecera enumera lo que se revisó; no garantiza
> nada que no nombre explícitamente.**

**Revisión del 31 de Agosto de 2026 — QUÉ SE REVISÓ, y nada más que esto:**

1. **§1.9 rehecha.** Publicaba la decisión **contraria** a la vigente —*«se instala `HC-05` / `JDY-30`,
   no ESP32»`*— cuando el 28/08 se decidió y se dejó escrito lo opuesto. **La fila de la tabla de
   caminos se ha DECIDIDO y anotado**, que era lo que faltaba.
2. **§2.3: el módulo del diagrama pasa de `HC-05`/`JDY-30` a `ESP32`.** El **pinout de `J17` p2/p3 NO
   se ha tocado**: era y sigue siendo correcto. Lo que estaba mal era el módulo dibujado.
3. **§2.3.bis nueva: la alimentación.** El ESP32 lleva **fuente propia desde 12 V** y no cuelga del
   3,3 V de `J17`.
4. **«Puesta a punto del módulo… el `AT+NAME`» marcada como NO aplicable a un ESP32**, con lo que la
   sustituye y lo que falta.

5. **§2.1: se corrige el «el `.kicad_pcb` está vacío»**, que era falso — el fichero pesa 2.158.421 B y
   trae 185 huellas y 1.447 pistas. **La advertencia de medir con multímetro se mantiene íntegra**:
   lo que se corrige es el motivo, no la precaución.

**Lo que NO se ha tocado en esta revisión, y sigue como estaba:** el **apartado 1 congelado** (el
cuadro de arquitectura, las tres razones y la tabla de prohibiciones), el pinout de `J17`, §2.2
(`J16` vs `J17`), **§2.5 (`PA8`/`J10`, residuo pendiente)**, y los apartados 3, 4 y 5.

> 🛑 **NADA DE ESTO HA PASADO PRUEBA DE BANCO, y este manual no es un permiso para instalar.** Hay
> **dos bloqueos vivos delante del montaje**: `BLQ-1` —la serigrafía del ESP32, sin leer— y la línea
> `A5` —la fuente propia, sin pedir—. Y **el firmware del puente SPP del ESP32 no existe todavía**
> (MEDIDO el 31/08). El cableado de §2.3 describe **a dónde irán los hilos**, no una instalación
> autorizada.

> 🔒 **EL APARTADO 1 NO SE HA DEROGADO, Y ESTA REVISIÓN NO LO DEROGA.** Sigue congelado: **Bluetooth
> Clásico SPP, no BLE, no Web Bluetooth**. Un ESP32 **clásico** haciendo de puente SPP **cabe dentro
> de esa decisión sin reabrirla** —es la vía que el propio manual dejaba abierta—, y es justamente por
> eso que se eligió esa fila y no otra.

---

## 1. Arquitectura de la App — DECISIÓN CONGELADA

> ### 🔒 **Bluetooth Clásico SPP. No BLE. No Web Bluetooth. Y no es negociable sin reabrir este apartado por escrito.**

Esto se congela porque ya se fue por el camino equivocado una vez y hay que dejar constancia de por
qué, o se repetirá.

### Lo que se eligió

```text
 ┌─────────────────────────────────────────────────────────────────────────────┐
 │                    ARQUITECTURA DE LA APP MÓVIL (V9.0)                      │
 ├─────────────────────────────────────────────────────────────────────────────┤
 │ • ENLACE:        Bluetooth CLASICO, perfil SPP.                             │
 │                  UUID 00001101-0000-1000-8000-00805F9B34FB                  │
 │ • MODULOS:       HC-05 / JDY-30.  NO HM-10, NO JDY-31, NO BLE.              │
 │ • EMPAREJADO:    Lo hace ANDROID en Ajustes, con PIN 0000 o 1234.           │
 │                  La app NO empareja: solo lista getBondedDevices().         │
 │ • IMPLEMENTACION: Puente NATIVO Android en el proyecto Capacitor            │
 │                  (BluetoothAdapter + createRfcommSocketToServiceRecord).    │
 │                  NO navigator.bluetooth. NO navigator.serial.               │
 │ • CONEXION:      UNA a la vez, explicita. SIN reconexion automatica.        │
 │ • INTERFAZ:      HTML5 + CSS + JS en WebView (Dark Theme IOT-VIAL).         │
 │ • ALMACENAMIENTO: LocalStorage / IndexedDB para logs y cruces.              │
 │ • AUTONOMIA:     100% offline. Opera en montana sin internet ni 4G.         │
 └─────────────────────────────────────────────────────────────────────────────┘
```

> 📌 **El cuadro de arriba se deja LITERAL, porque es el acta de la decisión congelada — pero una de
> sus once líneas ya no describe el montaje.** La línea `MODULOS: HC-05 / JDY-30` se sustituye por
> **`ESP32` clásico haciendo de puente SPP** (§1.9, decisión de obra del 28/08).
>
> **Las otras diez líneas no cambian, y ese es exactamente el argumento**: el enlace sigue siendo
> Bluetooth Clásico SPP con el mismo UUID, el emparejado lo sigue haciendo Android en Ajustes, el
> puente nativo `createRfcommSocketToServiceRecord` **no se toca**, y `NO BLE` sigue siendo `NO BLE`.
> **Cambia quién lleva la radio, no qué radio es.** Por eso este apartado **no se reabre** — y por eso
> mismo, si el módulo resultara ser BLE, **sí habría que reabrirlo** (`BLQ-1`, §1.9).

### Por qué SPP y no BLE — las tres razones, para que no se vuelva a preguntar

1. **`navigator.bluetooth` (Web Bluetooth) solo habla BLE.** No es que falle con un HC-05: **la API
   no existe para SPP**. La versión anterior de esta app usaba `navigator.bluetooth` y por eso *no
   abría el Bluetooth y no se conectaba a ningún dispositivo*. No era un error de programación: era
   la tecnología equivocada.
2. **La app probada en campo de esta casa es la de Baliza, y usa SPP.** `BluetoothAdapter`,
   `getBondedDevices()`, `createRfcommSocketToServiceRecord(00001101-…)`, con reintento por
   `createInsecureRfcommSocketToServiceRecord`. Está funcionando en la calle. **Se copia ese bloque,
   no se reinventa.**
3. **El técnico ya sabe usarlo.** Empareja en Ajustes de Android con `0000` o `1234` —el PIN del
   módulo, no el del semáforo— y la app le lista lo que ya está emparejado. Cambiar a BLE le cambia
   un flujo que domina, sin darle nada a cambio.

### Lo que queda PROHIBIDO, y por qué

| Prohibido | Motivo |
|---|---|
| `navigator.bluetooth` / Web Bluetooth | No puede ver un HC-05. Es la causa del fallo anterior |
| Módulos BLE (`HM-10`, `JDY-31`) | Obligarían a rehacer el puente nativo y cambiar el flujo del técnico |
| Reconexión automática | El operario camina al Km 24, el teléfono se reengancha solo al Km 12 que sigue en rango, y la pantalla muestra un sistema vivo **que está a 12 km**. No hay forma de que lo note |
| Dos conexiones simultáneas | Físicamente imposible —los postes de una pareja están a cientos de metros y el Bluetooth alcanza 10-15 m— y Baliza ya usa un único socket. **Una a la vez es una propiedad de seguridad, no una limitación** |

### 🔍 Qué módulo cumple esta decisión: la etiqueta comercial NO lo dice

> **El 28/08 llegaron a obra módulos `ESP32` en vez de los `HC-05` que pedía la lista de compras.**
> Eso obliga a escribir aquí algo que faltaba: **«ESP32» no es una respuesta a la pregunta «¿habla
> SPP?»**. Unos sí y otros no, y el rótulo del vendedor no permite distinguirlos.

| Familia | Radio que lleva de verdad | ¿Sirve para esta app? |
|---|---|---|
| `ESP32-WROOM-32` · `-32D` · `-32E` · `-32U` (ESP32 **clásico**) | Bluetooth **Clásico (BR/EDR)** + BLE | ✅ **Sí.** Tiene SPP: `createRfcommSocketToServiceRecord` conecta y **la app funciona tal cual**, sin tocar el puente nativo |
| `ESP32-S3` · `ESP32-C3` | **Solo BLE** | ❌ **No.** El socket RFCOMM no abre contra ellos. Es **exactamente el fallo del `navigator.bluetooth`** que congeló este apartado, pagado por segunda vez |
| `ESP32-S2` | **Sin radio Bluetooth.** Solo WiFi | ❌ **No.** No hay nada que emparejar en Ajustes de Android |

**La etiqueta del vendedor no es un instrumento.** Las tres familias se anuncian con la misma
cadena —*«WiFi + BT · SoC · ISM 2.4G · 802.11»*— y esa cadena dice que hay una radio de 2,4 GHz;
**no dice si hay Bluetooth Clásico**. Comprar leyendo el anuncio es elegir al azar entre las tres
filas de la tabla, y dos de las tres no conectan.

> Es la **regla del instrumento** del repositorio aplicada a una etiqueta: *un «no aparece» no es un
> hallazgo hasta haber descartado al buscador*. Aquí el buscador es el rótulo comercial, y **no sabe
> distinguir lo que se necesita**. Un módulo no queda descartado ni aprobado por lo que diga el
> listado del proveedor.

**Dónde sí está la respuesta. Las dos formas miran el chip, no el anuncio:**

1. **La serigrafía del blindaje metálico del módulo.** Sobre la lata va impresa la referencia real
   (`ESP32-WROOM-32E`, `ESP32-S3-WROOM-1`, …). Es la que manda.
2. **Preguntándoselo al propio chip**, con el módulo conectado por USB:
   ```text
   python "C:/.platformio/packages/tool-esptoolpy/esptool.py" --port COM# chip_id
   ```
   `esptool.py v4.11.0` **ya está instalado** en la máquina de trabajo (viene con PlatformIO): no hay
   que descargar nada. Responde con la familia detectada —`Chip is ESP32-D0WD-V3`, `Chip is
   ESP32-S3`, …— y esa línea es la que decide.

> ⏳ **PENDIENTE — no se da por sabido.** De los módulos que llegaron a obra **la referencia exacta
> está sin confirmar**. Hasta que alguien lea el blindaje o corra el `chip_id` de arriba y lo anote,
> **no se sabe si sirven**. No es un trámite: si son `S3` o `C3`, la app no conectará jamás y el
> síntoma será indistinguible del fallo que congeló este apartado —*«no abre el Bluetooth, no se
> conecta a ningún dispositivo»*—, con el tiempo de diagnóstico gastado otra vez en el sitio
> equivocado.

### ⚡ Si se va con un ESP32, el consumo no es un detalle de montaje

Un ESP32 con WiFi activo da **picos de ~500 mA**. La alimentación de la tarjeta es
`12 V → LM7805 → LM1117-3.3`, y ese camino **no los aguanta**:

* **El `LM7805` se quema.** A 500 mA disipa $(12-5)\times 0{,}5 = 3{,}5\ \text{W}$, que sin disipador
  no se evacúan.
* **Y antes de quemarse, se lleva por delante al micro.** Si el riel de 3,3 V se hunde, **se reinicia
  el STM32 que está gobernando el semáforo**. El síntoma en campo no se parece a un fallo de
  telemetría: se parece a un cruce que se reinicia solo, y ahí ya no se está diagnosticando un
  módulo de diagnóstico.

> 🔌 **Regla de alimentación: un ESP32 va con FUENTE PROPIA desde los 12 V, masa común con la
> tarjeta y alimentación NO compartida.** Un `HC-05` (~40 mA) sí puede colgarse del riel de la placa
> —por eso el diagrama del apartado 2 lo dibuja así—, y **ese diagrama no vale para un ESP32**.

### 1.9 ✅ La decisión VIGENTE: el ESP32 clásico SUSTITUYE al módulo SPP dedicado

> ### ~~✅ Decisión de obra del 28/08: se sigue con el módulo SPP dedicado~~
> ### ~~**Se instala `HC-05` / `JDY-30`, no ESP32.**~~
>
> ⛔ **ANULADO. Era la decisión de la 1.ª revisión del 28/08, y ese mismo día se decidió lo
> contrario.** Este manual se quedó publicando la versión vieja **once días**, mientras
> `15_Lista_de_Compras_Hardware.md` ya publicaba la nueva desde el commit `2e6baf4`.
>
> **Su motivo era bueno y se conserva escrito**, porque es la mitad del argumento que sigue en pie:
> un módulo SPP dedicado deja esta decisión congelada intacta y su consumo cabe en la alimentación
> que la tarjeta ya tiene. **Lo que lo tumbó no fue el argumento: fue el almacén.**
>
> **No se borra**, porque una vía descartada que desaparece en silencio se vuelve a proponer, y la
> segunda vez ya nadie recuerda por qué se descartó.

**Lo que manda hoy, y por qué este manual no era quien lo decidía:**

```
15_Lista_de_Compras_Hardware.md:159  "Decision de obra del 28/08 - VIGENTE: el ESP32 SUSTITUYE al modulo SPP"
15_Lista_de_Compras_Hardware.md:85   linea A1 ANULADA: los HC-05 "nunca llegaron, y ya no se piden"
ESTADO.md:80                         "sustituyendo al modulo SPP dedicado"
ESTADO.md:251                        A3: "ya no se compran HC-05 ni JDY-30: los sustituye el ESP32"
```

📖 **LEÍDO** en esos documentos, no medido: **los `HC-05` nunca llegaron a obra, ya no se piden, y el
28/08 llegaron módulos `ESP32` en su lugar.** No se está eligiendo entre dos módulos disponibles: hay
uno.

#### La fila decidida — y se decide aquí, por escrito, porque faltaba

La tabla de caminos del 28/08 se publicó **sin ninguna fila marcada como elegida**, que es como este
manual acabó contradiciendo a la lista de compras sin que nada chirriara. **Se decide ahora:**

| Camino | Qué exige | Estado de este apartado 1 | Decisión |
|---|---|---|---|
| ~~`HC-05` / `JDY-30` (SPP)~~ | ~~Nada nuevo. Es lo que este manual ya especifica~~ | ~~Intacto~~ | ⛔ **DESCARTADO 28/08.** No llegaron y ya no se piden (`15_…:85`) |
| **ESP32 clásico** (`WROOM-32`/`-32D`/`-32E`/`-32U`) haciendo de **puente SPP** | Confirmar la referencia por serigrafía o `chip_id`, y **fuente propia de 12 V** | **Intacto**: sigue siendo Bluetooth Clásico SPP, que es lo que este apartado congela | ✅ **ELEGIDO.** 🛑 **Condicionado a `BLQ-1`** |
| ESP32 **por WiFi / BLE** (`S3`, `C3`, `S2`, o cualquiera renunciando al SPP) | Rehacer el transporte de la app entero | 🛑 **Exige REABRIR ESTE APARTADO 1 POR ESCRITO**, antes de comprar y antes de escribir una línea | ⛔ **NO ELEGIDO.** Es la rama que hay que evitar, no un plan |

> **Por qué la fila 2 y no otra, que es lo único que autoriza a no reabrir el apartado 1.** Esa vía
> ya estaba abierta en la tabla original: **un ESP32 clásico haciendo de puente SPP deja el apartado 1
> INTACTO**. Si la serigrafía dice `ESP32-WROOM-32`/`-32D`/`-32E`/`-32U`, **hay Bluetooth Clásico
> BR/EDR, hay SPP, y `createRfcommSocketToServiceRecord` conecta sin tocar una línea de la app.**
> Cambia el módulo; **no cambia el transporte**, que es lo que este apartado congela.
>
> **La última fila no es burocracia, y no se ablanda.** El valor de una decisión congelada está
> justamente en que cambiarla cueste un documento: la vez anterior se cambió de transporte sin
> escribirlo y el resultado fue una app que no conectaba con nada. Un ESP32 por WiFi o BLE no es «el
> mismo módulo con otra radio»: cambia el emparejado, el descubrimiento, la autonomía sin internet y
> el flujo del técnico. Si se toma ese camino, **se reabre aquí, con fecha y firma, antes**.

#### 🛑 `BLQ-1` — BLOQUEO VIVO: nadie ha leído la serigrafía. **No lo resuelve este manual.**

> ## Hasta que alguien lea el blindaje del módulo, **NO SE SABE en cuál de las dos ramas estamos**, y las dos consecuencias son distintas.

📖 **LEÍDO** en `ESTADO.md:23` (`BLQ-1`, dueño: **Responsable**) y `15_Lista_de_Compras_Hardware.md:86`
(línea `A1′`, **🛑 BLOQUEADA**): de los módulos que llegaron a obra el 28/08, la **referencia exacta
está sin confirmar** y la **cantidad sin anotar**.

| si la serigrafía dice… | qué pasa con la app | qué pasa con este apartado 1 |
|---|---|---|
| `ESP32-WROOM-32` · `-32D` · `-32E` · `-32U` | ✅ **Nada. Conecta tal cual**, sin tocar el puente nativo | ✅ **Intacto.** No se reabre |
| `ESP32-S3` · `ESP32-C3` | 🔴 **El socket RFCOMM no abre NUNCA.** Hay que **rehacer el transporte de la app entero** | 🛑 **Reapertura OBLIGATORIA, por escrito** |
| `ESP32-S2` | 🔴 **No tiene radio Bluetooth.** No hay nada que emparejar en Ajustes de Android | 🛑 **Reapertura OBLIGATORIA, por escrito** |

**Lo que cuesta no saberlo, y es la parte que no se ve:** si son `S3` o `C3`, el síntoma en campo es
**indistinguible** del fallo que congeló este apartado —*«no abre el Bluetooth, no se conecta a ningún
dispositivo»*—, y el tiempo de diagnóstico se gasta otra vez en el sitio equivocado. **Es la regla del
instrumento aplicada a una etiqueta:** el rótulo del vendedor no distingue las tres familias, así que
comprar o instalar leyendo el anuncio es elegir al azar entre tres filas de las que **dos no
conectan**.

**Cómo se levanta el bloqueo — treinta segundos, y es una medida, no una consulta al proveedor:**
leer la **serigrafía del blindaje metálico**, o correr el `chip_id` de «Qué módulo cumple esta
decisión: la etiqueta comercial NO lo dice», más arriba en este mismo apartado. **Y anotarlo**, en
`ESTADO.md` y en la línea `A1′` del Manual 15. Mientras no esté anotado, **`BLQ-1` sigue vivo y este
manual sigue sin poder afirmar que el montaje funcione**.

> 🔴 **Y `BLQ-1` bloquea también la compra:** no se compra ni un ESP32 más hasta leer qué referencia
> llegó (`15_…:86`). Un lote entero de `S3` es un lote entero de módulos que no sirven.

### El flujo de campo, que es el de Baliza

```text
  1. Ajustes de Android > Bluetooth > emparejar > PIN 0000 o 1234
  2. Abrir la app > "Buscar" > lista los emparejados (nombre + MAC)
        SEM-7A3F-M   Maestro, Km 12
        SEM-7A3F-E   Esclavo,  Km 12
        SEM-C104-M   Maestro, Km 24
  3. Tocar uno > socket RFCOMM sobre SPP > leer/escribir lineas
```

El nombre visible del módulo se fija con `AT+NAME`, así que **la lista de Android ya dice quién es
cada equipo antes de conectar**. El técnico lee; no adivina.

### 🔧 Puesta a punto del módulo ANTES de instalarlo — el `AT+NAME` no es opcional

> ## ⚠️ ESTE PROCEDIMIENTO ES DE `HC-05`, Y CON UN ESP32 **NO SE APLICA**. La NECESIDAD sí sigue en pie.
>
> **Lo que NO cambia:** dos módulos sin matricular en el mismo corredor salen en la lista de Android
> como dos filas iguales, y el técnico **no sabe a qué poste se ha conectado**. Eso es igual de grave
> con un ESP32 que con un `HC-05`, y sigue siendo obligatorio.
>
> **Lo que SÍ cambia:** un ESP32 **no tiene modo AT ni `AT+NAME`** — el nombre lo fija **su propio
> firmware**, en la llamada que abre el `SerialBT` del lado ESP32 (`SerialBT.begin("SEM-7A3F-M")` en
> el API clásico de Arduino-ESP32). No hay terminal a 38400 bps que abrir: **hay un firmware que
> escribir y grabar**.
>
> 🔴 **Y ese firmware NO EXISTE. MEDIDO el 31/08:** el único fuente de ESP32 del repositorio es
> `01_Firmware/Repetidor/src/main.cpp` (un solo fichero, 8.348 B), **que es el del repetidor de radio,
> no un puente Bluetooth**. Mientras no exista, **no hay nada que rotular y no hay módulo que
> instalar**: no es un paso que se salta, es un paso que todavía no tiene con qué hacerse.
>
> 🟡 **PENDIENTE, con dueño: escribir el firmware del puente SPP del ESP32, y en él la matrícula.**
> Va **detrás del watchdog** del ESP32 —hoy no tiene ninguno, y hay precedente escrito de uno clavado
> tumbando el enlace el 31/07— según el orden de fases de `ESTADO.md:107`. **Lo decide y lo ordena el
> responsable**, no este manual.
>
> **Lo que sigue se conserva íntegro** porque describe el `HC-05` que puede haber montado ya en algún
> equipo, y porque **el `9600 8N1` del modo datos lo impone el firmware del STM32 y vale para los dos
> módulos**.

> **Ese «la lista ya dice quién es cada equipo» no ocurre solo: hay que provocarlo en el banco, con
> el módulo en la mano y antes de que suba al poste.**

Un módulo recién sacado de la bolsa se anuncia con su nombre de fábrica. Dos módulos de fábrica en
el mismo cruce salen en la lista de Android como **dos `HC-05` idénticos**, distinguibles solo por
la MAC — que nadie lleva apuntada subido a una escalera. El técnico ve dos filas iguales, elige una,
y **no sabe a qué poste se ha conectado**. Ese es justo el error que la matrícula `SEM-…-M` /
`SEM-…-E` existe para impedir.

**Procedimiento, uno por módulo, rotulando el módulo por fuera con el mismo nombre:**

```text
  1. Modo AT: alimentar el modulo con el boton/pin KEY en alto (HC-05 arranca en AT)
  2. Terminal serie a 38400 bps 8N1, final de linea CR+LF
  3. AT              -> debe responder  OK
     AT+NAME=SEM-7A3F-M     (Maestro del cruce)      -> OK
     AT+NAME=SEM-7A3F-E     (Esclavo del mismo cruce) -> OK
     AT+UART=9600,0,0       (deja el modo datos a 9600 8N1)
  4. Quitar el modo AT, reiniciar, y comprobar que Android lo lista con el nombre nuevo
```

> ⚠️ **Las dos velocidades son distintas y confundirlas parece un módulo muerto:**
>
> | | velocidad |
> |---|---|
> | **Modo AT** (configuración, antes de instalar) | **38400 bps** |
> | **Modo datos** (operación, ya en el poste) | **9600 bps** — que es la de `USART1` en el firmware y la de todas las tramas del apartado 4 |
>
> Si el módulo se deja a otra velocidad en modo datos, la telemetría llega como basura y el síntoma
> es idéntico al de un cable cruzado. **El `9600` no se elige: lo impone el firmware.**

**La matrícula sigue el patrón del apartado anterior:** `SEM-<SERIE>-M` y `SEM-<SERIE>-E`, donde
`<SERIE>` son los 24 bits en hexadecimal que el propio equipo emite en `$STATUS` (§ 4.2). Así el
nombre que se lee en la lista de Android y el que viaja en la trama **son el mismo dato**, y un
módulo trasplantado a otra placa se delata al primer `$STATUS`.

### Lo que la app NO puede hacer, aunque el operario lo pida

**Al Esclavo no se le manda nada que abra paso.** Puede leer telemetría, ajustar el reloj, forzar
rojo —que es la dirección segura— y **solicitar paso**, que viaja por radio al Maestro como una
demanda: el Maestro decide, aplica el todo-rojo y ordena. El detalle está en `OPTIMIZACIONES.md`
§ SFTY-27.

> **Consecuencia para el operario, que es lo que importa:** da igual en qué extremo esté. Los dos
> postes se operan igual y en la pantalla **no aparecen las palabras «maestro» ni «esclavo»** —
> aparece el sentido de la vía.

---

## 2. Diagrama de Conexión Físico y Desacoplo de Hardware (KiCad)

> ## 🔴 28/08/2026 — ESTE APARTADO SE REDIBUJÓ ENTERO. EL CABLEADO CAMBIÓ.
>
> **El módulo Bluetooth entra por el conector `J17`, en `USART1` REMAPEADO a `PB7`/`PB6`.**
>
> Hasta esta revisión, este manual mandaba cablear a **`PA9`/`PA10`**. Ese pinout **ya no es el
> vigente** y **el firmware que corre hoy no escucha por ahí**. Como este es el **único documento
> de la entrega con dibujo de conexión del módulo**, quien siguiera el dibujo anterior cableaba un
> equipo que no responde, sin nada que le contradijera.
>
> **La corrección no se borra el rastro:** si usted ya cableó a `PA9`/`PA10`, no se ha equivocado
> siguiendo el manual — seguía la versión anterior. Vea **«Si ya se cableó a `PA9`/`PA10`»** al
> final de este apartado antes de tocar nada.

### 2.1 De dónde sale este pinout — y con qué nivel de prueba

Las dos mitades de este apartado **no tienen la misma solidez**, y se separan a propósito:

| dato | nivel de prueba |
|---|---|
| El firmware abre `USART1` sobre `PB7` (RX) y `PB6` (TX) a 9600 bps | ✅ **MEDIDO EN EL FUENTE.** `01_Firmware/Maestro/src/bluetooth.cpp` y `01_Firmware/Esclavo/src/bluetooth.cpp`, línea idéntica en las dos puntas: `static HardwareSerial SerialBT(PB7, PB6);` |
| `J17` p2 = `PB7` (`U1` p43) · `J17` p3 = `PB6` (`U1` p42) · p6/p8 = 3,3 V · p7/p9 = `GND` | 📐 **MEDIDO EN EL ESQUEMÁTICO.** Trazado sobre `Controladora_Semaforos.kicad_sch` (§7 de `03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md`) |
| Que en la tarjeta física esas vías vayan a donde dice el esquemático | 🔴 **NO VERIFICADO EN LA PLACA.** Nadie lo ha medido con multímetro |

> 🔴 **Esa última fila es una advertencia, no un formalismo. Antes de enchufar el módulo, compruebe
> con el multímetro las vías que vaya a usar** (continuidad de p2 a la pata 43 de `U1`, de p3 a la 42,
> y masa en p7). Son cinco minutos y son los que separan una hipótesis muy buena de un hecho.
>
> ⚠️ **CORRECCIÓN DEL 31/08 al motivo que esta advertencia daba.** Esta nota decía que *«el
> `.kicad_pcb` de este proyecto está vacío»* y que *«no existe ningún artefacto que ate el esquemático
> con la tarjeta»*. **Las dos frases son falsas, y se corrigen midiendo:**
>
> ```
>   wc -c  Controladora_Semaforos.kicad_pcb           ->  2.158.421 B
>   grep -oE '\(footprint\b'  ... | wc -l             ->      185 huellas
>   grep -oE '\(segment\b'    ... | wc -l             ->    1.447 pistas
>   grep -oE '\(via\b'        ... | wc -l             ->       89 vias
>   grep -c  'J17'            ...                     ->  J17 esta en el cobre, con sus pads
> ```
>
> **Por qué se creyó vacío:** KiCad separa los tokens con **tabulador y salto de línea**, así que un
> `grep -c '(segment '` —con espacio detrás— devuelve **0**, y un cero se lee como *«no hay»*. Es la
> **regla del instrumento** aplicada a un formato: `grep` estaba, respondía, y aun así no sabía
> encontrar (`CLAUDE.md` §4).
>
> **Lo que NO cambia con la corrección, y es lo importante:** sigue sin haber **una sola medida con
> punta sobre la tarjeta física**. Que el `.kicad_pcb` esté ruteado sube el nivel de prueba de
> *«hipótesis»* a *«hay un diseño de cobre que lo dice»* — **no lo sube a hecho**. La comprobación con
> multímetro se mantiene íntegra.

### 2.2 ⚠️ ANTES DE ENCHUFAR NADA: `J16` y `J17` SON EL MISMO CONECTOR A LA VISTA

> ## 🔴 `J16` LLEVA 12 V EN SU POSICIÓN 1. ENCHUFAR EL MÓDULO EN `J16` LO QUEMA.

`J16` (botones) y `J17` (donde va el módulo) **comparten el mismo footprint Molex 1x16 en el
cobre**. No se distinguen por la forma, ni por el tamaño, ni por el número de vías: **a la vista son
idénticos.** Y no fallan igual: `J17` reparte 3,3 V y masa, mientras que **`J16` es el único
conector de señal de toda la tarjeta que trae 12 V**, y lo trae en la posición 1 — justo donde cae
el `VCC` si alguien cuenta desde el borde equivocado.

**Cómo se distinguen, y es una medida, no un vistazo:**

```text
  1. Multimetro en tension continua, negra a masa del chasis.
  2. Punta roja en la POSICION 1 del conector, con la tarjeta alimentada.

       ~12 V  -> es J16.  ES LA BOTONERA. NO ENCHUFE AQUI EL MODULO.
       ~3,3 V o 0 V -> es J17.  Confirmelo con el paso 3.

  3. Confirmacion de J17: debe haber 3,3 V en las posiciones 6 y 8,
     y masa en las 7 y 9.  J17 NO TIENE 12 V EN NINGUNA POSICION.
```

> ⚠️ **Y el conector se orienta midiendo, no contando huecos.** El símbolo del esquemático de `J17`
> tiene **13 posiciones** y su footprint en la placa tiene **16** (el de `J16`: 12 en el símbolo, 16
> en el cobre). **Sobran vías físicas que no están en el esquema**, así que contar desde el borde
> del conector desplaza el mapa entero. Se cuenta **desde la posición 1 marcada**, y se confirma la
> orientación **leyendo 3,3 V en la 6 y masa en la 7** antes de insertar el módulo.

### 2.3 Diagrama de conexión — VIGENTE (ESP32 clásico, 31/08)

> ⚠️ **Lo que cambió el 31/08 en este apartado es EL MÓDULO, no el pinout.** `J17` p2 = `PB7` y
> `J17` p3 = `PB6` **siguen siendo correctos y no se han tocado**: son los que midió N-76 sobre el
> esquemático y el fuente. Lo que estaba mal era que el dibujo mandaba enchufar un `HC-05`, y **ese
> módulo ya no es el que se instala** (§1.9).
>
> 🛑 **Y sigue condicionado a `BLQ-1`:** hasta leer la serigrafía del blindaje no se sabe si el ESP32
> que hay en obra habla SPP. **Este diagrama no autoriza a instalar nada.**

```text
 ┌─────────────────────────────────────────────────────────────────────────────┐
 │   CONEXION DE TELEMETRIA BLUETOOTH  --  CONECTOR J17   (VIGENTE 31/08)      │
 ├─────────────────────────────────────────────────────────────────────────────┤
 │                                                                             │
 │   ESP32 CLASICO (WROOM-32/-32D/-32E/-32U)   TARJETA -- CONECTOR J17         │
 │   ┌────────────────────────────┐        ┌──────────────────────────────┐    │
 │   │  GPIO17 (TX2)  transmite   ├────────┤► J17 p2 = PB7  (U1 pin 43)   │    │
 │   │                            │        │           USART1 RX del STM32│    │
 │   │  GPIO16 (RX2)  recibe      ├────────┤► J17 p3 = PB6  (U1 pin 42)   │    │
 │   │                            │        │           USART1 TX del STM32│    │
 │   │  GND                       ├────────┤► J17 p7 = GND     (o p9)     │    │
 │   └────────────────────────────┘        └──────────────────────────────┘    │
 │        ▲                                                                    │
 │        │  ALIMENTACION: NO SALE DE J17.  Fuente propia desde 12 V.          │
 │        │  J17 p6/p8 (3,3 V) SE DEJAN SIN CONECTAR.  Ver 2.3.bis.            │
 │                                                                             │
 │   SON TRES HILOS, NO CUATRO:  RX, TX y MASA COMUN.                          │
 │                                                                             │
 │   LAS LINEAS DE DATOS VAN CRUZADAS:                                         │
 │      TX del ESP32 (GPIO17)  ->  RX del micro (p2).                          │
 │      RX del ESP32 (GPIO16)  <-  TX del micro (p3).                          │
 │   Conectarlas en paralelo -TX contra TX- no rompe nada, pero no llega       │
 │   ni una trama, y el sintoma es identico al de un modulo muerto.            │
 │                                                                             │
 │   LA MASA COMUN ES OBLIGATORIA: con dos fuentes distintas y sin masa        │
 │   comun, las dos puntas no comparten referencia y el serie no cuadra.       │
 └─────────────────────────────────────────────────────────────────────────────┘
```

| hilo del ESP32 | dirección | va a | pin del `U1` | qué es |
|---|---|---|---|---|
| **`GPIO17`** (`TX2`) | ---> | **`J17` p2** | `PB7` (p43) | `USART1` **RX** del STM32 — el micro **escucha** aquí |
| **`GPIO16`** (`RX2`) | <--- | **`J17` p3** | `PB6` (p42) | `USART1` **TX** del STM32 — el micro **habla** aquí |
| `GND` | --- | `J17` p7 (o p9) | — | **masa común, obligatoria** |
| *(alimentación)* | — | **NO va a `J17`** | — | 🔴 **Fuente propia desde 12 V.** `J17` p6/p8 **se dejan libres** — ver 2.3.bis |

**`9600 8N1`.** Es la del `USART1` del firmware y la de todas las tramas del apartado 4: **no se
elige, la impone el firmware.**

📖 **Nivel de prueba de la columna del ESP32:** **LEÍDO** en
`05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md` §1.4 y `ESTADO.md:80` — es la asignación
decidida, **sin firmware que la ejerza todavía** y sin nadie que la haya cableado. `GPIO16`/`GPIO17`
son la `UART2` por defecto del ESP32 clásico; **si el firmware del puente acaba usando otros pines, lo
que manda es el firmware y esta tabla se corrige entonces.** La columna del STM32 sí está **MEDIDA EN
EL FUENTE** (§2.1).

> ⚠️ **Duda abierta que este manual NO cierra y que no es de aquí:** la etiqueta de red del **pin 3**
> de `J17` está en disputa —el esquemático la llama `RS(A0)` y el firmware `LCD_PSB`—, y los dos
> nombres no pueden ser ciertos a la vez (doc 17 §1.4, `MAPEO_TARJETA_KICAD.md` §6.bis). **No cambia
> nada del cableado de arriba**, pero se cierra siguiendo el hilo del pin 3 hasta la pata rotulada, no
> leyendo más código. **Dueño: responsable.**

#### 2.3.bis 🔌 La alimentación del ESP32 — **no cuelga de `J17`, y esto no es un detalle de montaje**

> ## 🔴 EL ESP32 LLEVA FUENTE PROPIA DESDE LOS 12 V. NO SE ALIMENTA DEL 3,3 V DE `J17` p6/p8.

**El motivo no es la corriente: es de quién es esa corriente.** Ese riel de 3,3 V es **el mismo que
alimenta al STM32 que gobierna el semáforo**, y un accesorio de diagnóstico **no puede tumbar al que
manda**.

Un ESP32 con radio activa da **picos de ~500 mA**, y el camino `12 V → LM7805 → LM1117-3.3` de la
tarjeta no los da:

* **El `LM7805` se quema:** a 500 mA disipa $(12-5)\times 0{,}5 = 3{,}5\ \text{W}$, que sin disipador
  no se evacúan.
* **Y antes de quemarse se lleva por delante al micro:** si el riel de 3,3 V se hunde, **se reinicia
  el STM32 que está gobernando el cruce**. El síntoma en campo **no se parece a un fallo de
  telemetría**: se parece a un cruce que se reinicia solo, y ahí ya no se está diagnosticando un
  módulo de diagnóstico.

| | `HC-05` / `JDY-30` (~40 mA) | **ESP32 (~500 mA de pico)** |
|---|---|---|
| ¿puede colgarse de `J17` p6/p8? | ✅ sí — por eso el diagrama viejo lo dibujaba así | 🔴 **NO. Nunca.** |
| alimentación | del riel de la tarjeta | **fuente propia DC-DC 12 V → 5 V, 1 A** |
| masa | la del conector | **masa común con la tarjeta, obligatoria** |

> 🛑 **PENDIENTE Y BLOQUEANTE DE MONTAJE: esa fuente NO SE HA PEDIDO.** Es la línea **`A5`** de
> `05_Funcional/15_Lista_de_Compras_Hardware.md:90`, marcada **🔴 NO cubierta** — *«sin ella el `ESP32`
> reinicia el STM32 del semáforo»*. **Sin `A5` no se monta el ESP32**, ni «provisionalmente desde el
> 3,3 V para probar»: esa prueba provisional se hace sobre el equipo que controla el tráfico.
> **Dueño: responsable** (`ESTADO.md:251`, línea `A3`).

#### ~~Diagrama anterior (`HC-05` / `JDY-30`)~~ — conservado, NO es el montaje vigente

**No se borra**, porque las revisiones anteriores de este manual lo mandaban y **puede haber equipos
montados así**. Un `HC-05` cableado a `J17` con este dibujo **funcionaría eléctricamente** —el pinout
era y es correcto—; lo que ya no es cierto es que sea el módulo que se instala, porque **esos módulos
nunca llegaron y ya no se piden** (`15_…:85`).

```text
   ##########  MONTAJE ANTERIOR -- YA NO ES EL MODULO QUE SE INSTALA  ##########

   MODULO BLUETOOTH (HC-05 / JDY-30)        TARJETA -- CONECTOR J17
   ┌────────────────────────────┐        ┌──────────────────────────────┐
   │  [ TXD ]  transmite        ├────────┤► J17 p2 = PB7  (U1 pin 43)   │
   │  [ RXD ]  recibe           ├────────┤► J17 p3 = PB6  (U1 pin 42)   │
   │  [ VCC ]  3,3 V            ├────────┤► J17 p6 = 3,3 V   (o p8)     │
   │  [ GND ]  masa             ├────────┤► J17 p7 = GND     (o p9)     │
   └────────────────────────────┘        └──────────────────────────────┘

   El pinout de J17 NO cambia. Lo que cambia es el modulo, y con el la
   alimentacion: el ESP32 NO usa p6/p8.  (Ver 2.3.bis.)
```

> ⚠️ **Si usted ya tiene un `HC-05` montado en `J17` y funcionando, no lo desconecte por este
> manual.** Está cableado al pinout correcto y el firmware del STM32 le habla igual. Lo que este
> apartado dice es **qué se instala de aquí en adelante**, no que lo montado esté mal. **Que un cruce
> se quede sin consola por seguir un cambio de documento es peor que la inconsistencia.**

**Por qué estos dos pines y no otros:** son los que dejó libres la **pantalla LCD, que se retira**
(ver `1_Manual_Usuario.md` y `OPTIMIZACIONES.md`, ambos del 28/08). El equipo va montado en alto y
la pantalla no se lee desde el suelo, así que **la app pasa a ser la única interfaz de operación**.
En `J17`, `PB7` era el `RST` del display y `PB6` su nivel estático de modo serie: **ninguno de los
dos transportaba datos**, y por eso el puerto serie sale gratis sin tocar los tres hilos que sí
dibujan (`PB3`, `PB4`, `PB5`).

> ✅ **RESUELTO PARA EL CAMINO VIGENTE, y solo para ese: con el ESP32 esta pregunta desaparece,
> porque el módulo NO se alimenta de `J17`** (2.3.bis, fuente propia de 12 V). Lo de abajo **sigue
> valiendo íntegro para un `HC-05` ya montado o para quien vuelva a esa vía**, y por eso no se borra.

> ⚠️ **PENDIENTE DE CONFIRMAR *(solo en el camino `HC-05`)* — la tensión de `VCC` del módulo que se instale.** `J17` entrega
> **3,3 V, y solo 3,3 V**: en esta tarjeta **los 5 V son un raíl interno que no sale a ningún
> conector de señal**. Muchas placas de evaluación de `HC-05` (tipo `ZS-040`) llevan su propio
> regulador y piden **3,6–6 V** en el pin rotulado `VCC`, aunque casi todas exponen además una
> entrada de 3,3 V directa al chip. **Antes de alimentar: lea la serigrafía del módulo concreto que
> tenga en la mano.** Si esa placa exige 5 V en `VCC` y no expone entrada de 3,3 V, **no hay 5 V en
> `J17`** y hay que resolverlo por escrito antes de cablear — no improvisando un hilo desde el
> regulador, que es soldadura en placa. Esto **no está resuelto en este manual**.

> ⚡ **Alimentar desde el riel de la tarjeta vale para un `HC-05`/`JDY-30` (~40 mA) y NO vale para un
> ESP32** (picos de ~500 mA). Ver 2.3.bis y el apartado 1, «Si se va con un ESP32, el consumo no es un
> detalle de montaje». **El diagrama histórico de `HC-05` no vale para un ESP32.**

### 2.4 `PA9`/`PA10`: alternativa histórica, NO el montaje vigente

**No se borra, porque hubo una versión de este manual que la mandaba cablear** y puede haber equipos
ya montados así.

`USART1` es **un solo periférico** y el STM32F103 lo puede sacar por `PA9`/`PA10` **o** por
`PB6`/`PB7`, **nunca por los dos a la vez**. Eléctricamente `PA9`/`PA10` sigue siendo válido, pero:

| | `J17` — `PB6`/`PB7` **(vigente)** | `PA9`/`PA10` (histórico) |
|---|---|---|
| ¿sale a una bornera? | ✅ **Sí, `J17`, enchufable** | ❌ **No sale a ninguna bornera de la tarjeta** |
| cómo se conecta | se enchufa | **soldando** en las patas del `MAX3485 U2` o del propio micro |
| ¿lo escucha el firmware de hoy? | ✅ sí | ❌ **no** — `SerialBT` no está ahí |

**Un montaje en `PA9`/`PA10` exigiría cambiar el firmware de las dos puntas**, no solo el cableado.
Mientras el firmware declare `SerialBT(PB7, PB6)`, `PA9`/`PA10` es un camino muerto.

#### Si ya se cableó a `PA9`/`PA10`

1. **No es un fallo del montaje**: era lo que decía la versión anterior de este manual, hasta el
   28/08.
2. **Hay que recablear a `J17`** siguiendo §2.3. El módulo, el emparejado y el `AT+NAME` del
   apartado 1 **se conservan tal cual**: solo cambian los cuatro hilos.
3. Si el módulo se soldó a las patas de `U2` o del micro, **retire esos hilos** antes de enchufar en
   `J17`: dejarlos puestos cuelga dos cosas del mismo pin del transceptor.
4. **Síntoma que confirma que el equipo estaba en el cableado viejo:** la app empareja con el módulo
   por Bluetooth sin problema —el emparejado es cosa del módulo, no del semáforo— pero **no llega
   ni una trama `$STATUS`** y ningún comando surte efecto. El módulo está vivo; simplemente no está
   hablando con el micro.

### 2.5 ⚙️ El `PA8` y el transceptor `MAX3485` (`U2`) — RESIDUO PENDIENTE DE REVISAR

> ## ⚠️ ESTE APARTADO YA NO DESCRIBE UN DISEÑO VIGENTE. DESCRIBE UN RESTO.
>
> Todo lo que sigue existía para **liberar `PA10` al `TXD` del Bluetooth**. **Ese motivo desapareció
> el 28/08**, cuando el módulo se movió a `J17` (`PB6`/`PB7`): **el Bluetooth ya no usa `PA10`, así
> que no hay ningún conflicto que desacoplar.**
>
> **Pero el firmware no ha cambiado.** `bluetooth_setup()` sigue clavando `PA8` en `HIGH` en las dos
> puntas, con un comentario que sigue justificándolo por un `PA10` que ya nadie disputa.
>
> **Se deja escrito aquí, y no se borra, porque el efecto eléctrico sigue ocurriendo en la placa.**
> Lo que cambia es su clasificación: **era el precio aceptado de una decisión de diseño; hoy es un
> residuo sin beneficio.**

> 🔴 **Corrección de referencia: el transceptor que se pone en Hi-Z es `U2`, NO `U3`.** Varios
> documentos —y los comentarios de `pines.h` y `bluetooth.cpp`— lo tienen invertido. Medido sobre
> `Controladora_Semaforos.kicad_sch`, que es la fuente:
>
> | | `RO` | `DE`/`~RE` | `DI` | par `A`/`B` sale por | qué es |
> |---|---|---|---|---|---|
> | **`U2`** | **`PA10`** | **`PA8`** | **`PA9`** | **`J10`** | **el que comparte pines con el Bluetooth** |
> | `U3` | `PB11` | `PB12` | `PB10` | `J12` | el de la **radio** — no tiene nada que ver con `USART1` |
>
> Los dos son `MAX3485`, y ese es justo el motivo de que se confundan: **se distinguen por la red a
> la que van, no por el encapsulado**. Quien vaya a medir con punta a la placa buscando `U3` estará
> pinchando el transceptor de la radio.

#### Qué hace hoy el firmware, y qué justificación le queda

**MEDIDO EN EL FUENTE (28/08):** `bluetooth_setup()`, en las dos puntas, sigue ejecutando

```cpp
pinMode(RS485_IN_DE_RE, OUTPUT);        // RS485_IN_DE_RE = PA8
digitalWrite(RS485_IN_DE_RE, HIGH);     // el comentario dice: "libera PA10 al modulo Bluetooth"
```

**Ese comentario ya no describe la realidad.** `SerialBT` vive en `PB7`/`PB6`; **`PA10` no lo
disputa nadie**, así que liberarlo no beneficia a nada.

`PA8` no llega a un pin: **llega a dos**. En la PCB va a la vez a `~RE` (pin 2) y a `DE` (pin 3) de
`U2`, unidos en la misma red. Un solo nivel manda sobre las dos mitades del transceptor, y **no las
apaga: las intercambia**.

| `PA8` | receptor (`RO` → `PA10`) | transmisor (`DI` ← `PA9`, salida a `J10`) |
|:---:|---|---|
| `LOW` | escuchando | apagado |
| **`HIGH`** *(lo que hace hoy el firmware)* | Hi-Z — `PA10` libre, **que ya no le hace falta a nadie** | **ENCENDIDO de forma permanente** |

**Consecuencia real, y ahora sin contrapartida:** con `PA8` en alto, el transmisor de `U2` está
**permanentemente tomado sobre el par `A`/`B` de `J10`**, y ese conector **nunca puede recibir
nada**. El puerto RS-485 de `J10` no está inactivo: está tomado y sordo.

> **Antes esto era un precio pagado a cambio de algo** —liberar `PA10` para el Bluetooth—. **Hoy es
> solo el precio.** El beneficio se fue con el remapeo a `J17` y el coste se quedó.

> **Es la misma lección que `01_Firmware/TROUBLESHOOTING.md` dejó escrita con el repetidor el
> 31/07/2026:** *«si un DE/RE se queda permanentemente en alto, esa línea queda bloqueada en ambos
> sentidos»*. Allí fue un fallo; aquí **dejó de ser una decisión y volvió a ser lo que era allí**.

> ⚠️ **Y un dato que hay que mirar antes de tocar nada, MEDIDO EN EL FUENTE, no en la placa:** tras
> el remapeo **ya nadie configura `PA9`**. `protocolo_setup()` retiró la apertura de `AiBus` —el
> único código que declaraba `PA9`/`PA10`— y `USART1` se fue a `PB6`/`PB7`. Es decir: el `DI` de
> `U2` **no está gobernado por ningún periférico**, mientras su transmisor sigue habilitado por
> `PA8`. **Qué nivel presenta entonces `J10` no se ha medido en la placa y este manual no lo
> afirma.** Es parte de lo que hay que revisar.

#### Qué NO se ha hecho, y es deliberado

> 🛑 **Retirar el `digitalWrite(PA8, HIGH)` es un cambio de firmware que AÚN NO SE HA HECHO.**

No se ha tocado, y no se toca desde este manual, por tres razones:

1. **Es un cambio con sentido propio** y va en su propio `N-x`, con su pack y su compuerta — no
   colado dentro del cambio de cableado.
2. **Toca las dos puntas** (`Maestro` y `Esclavo`) y afecta a una línea RS-485, no solo a un
   comentario.
3. **No se ha medido en la placa** qué hace hoy `J10` con `PA9` sin gobernar. Cambiar el nivel de un
   `DE`/`~RE` sin saber el estado de partida es exactamente lo que este repositorio no hace.

**Estado que queda anotado, para que nadie lo lea como diseño:**

| | |
|---|---|
| **Qué es** | Un **residuo**: código que sobrevivió al motivo que lo justificaba |
| **Efecto hoy** | `J10` con el transmisor de `U2` tomado y sin poder recibir. **Inofensivo mientras `J10` esté vacío** |
| **Cuándo deja de ser inofensivo** | El día que alguien cuelgue **cualquier cosa** de `J10`. Ese día se toca **este código**, no el cableado |
| **Acción pendiente** | Revisar si `PA8` debe pasar a `LOW` o dejar de tocarse. **Cambio de firmware aparte, sin fecha** |
| **Nivel de prueba** | Firmware: **MEDIDO EN EL FUENTE**. Comportamiento de `J10`: **NO VERIFICADO EN LA PLACA** |

---

## 3. Especificación de Pantallas y Módulos de la App

La App cuenta con **5 pantallas funcionales**, diseñadas con alto contraste para visibilidad bajo sol directo en carretera:

```text
 ┌─────────────────────────────────────────────────────────────────────────────┐
 │ 🚦 STATUS BAR: [● Conectado (👑 MAESTRO P1)] · [RSSI: -62 dBm] · [18:25:00] │
 ├─────────────────────────────────────────────────────────────────────────────┤
 │   [ 🗺️ ESTADO ]   [ 🎮 CONTROL ]   [ 🔔 EVENTOS ]   [ 🔬 TEST ]   [ ⚙️ RTC ] │
 └─────────────────────────────────────────────────────────────────────────────┘
```

### Pantalla 1: ESTADO (Monitoreo en Tiempo Real)
* **Semáforos Duales con Glow Dinámico:** Representación gráfica en vivo de Maestro (Sentido 1) y Esclavo (Sentido 2).
* **Anillo de Cuenta Regresiva SVG:** Círculo animado en tiempo real con indicador gigante en monospace del tiempo restante de verde/rojo (`T:<segundos>`).
* **Métricas Clave:** Calidad de enlace de radio inter-semáforo (%), tiempo de respuesta RTT (ms) y nivel de batería (V).
* **Botón de Pánico Rojo Total:** Forzado inmediato de All-Red ante emergencias viales.

### Pantalla 2: CONTROL (Comandos Protegidos por PIN)
* **Selector de Modos:** Automático, Manual, Ámbar de Seguridad y Parada de Emergencia.
* **Control de Tráfico Manual:** Botón táctil para alternar el turno vehicular desde el suelo respetando el tiempo de despeje All-Red normativo.
* **Seguridad Obligatoria:** Requiere validación del PIN de 4 dígitos (`1234`) antes de enviar comandos que alteren las luces.

### Pantalla 3: EVENTOS (Caja Negra y Registro Histórico)
* **Feed de Alarmas:** Registro cronológico de caídas de radio, transiciones de modo y fallas con timestamp del reloj RTC.
* **Compartir por WhatsApp:** Genera automáticamente el mensaje formateado para interventoría con ubicación y nodo.
* **Exportar a CSV:** Descarga el archivo de auditoría para archivo formal.

### Pantalla 4: TEST Y DIAGNÓSTICO EN TALLER
* **Test de Lámparas de 6 Segundos:** Secuencia de prueba de banco (2s Rojo ➔ 2s Amarillo ➔ 2s Verde) ejecutada a través de `semaforo_iniciarTestLeds()` bajo la barrera de seguridad de `semaforo.cpp` con semáforo fuera de servicio o Todo-Rojo controlado.

### Pantalla 5: AJUSTES & ASISTENTE COURIER RTC
* **Sincronización Directa de Hora:** Ajuste del reloj RTC del nodo conectado con la hora del teléfono móvil.
* **Modo Asistente Courier RTC (Sincronización Puente sin Radio):**
  1. *Paso 1:* Capturar hora y ciclo en el Poste Maestro.
  2. *Paso 2:* Viajar hasta el Poste Esclavo (la App cronometra el tiempo de viaje).
  3. *Paso 3:* Inyectar en el Esclavo la hora compensada ($\Delta t < 0.1\text{ s}$), permitiendo sincronizar el Modo Degradado sin cables ni radio.

---

## 4. Estructura de Tramas y Protocolo Serie ASCII (Estilo NMEA con Checksum XOR)

Todas las tramas viajan a **9600 baudios (8N1)** y finalizan en `\r\n`.

### 4.1 Cálculo del Checksum NMEA (*XX)
El checksum se calcula aplicando la operación **XOR bit a bit** de todos los bytes contenidos entre el carácter `$` inicial y el asterisco `*` (ambos excluidos), formateado como dos caracteres hexadecimales en mayúsculas (`00` a `FF`).

### 4.2 Telemetría Periódica ($STATUS) — Emitida cada 1 segundo
$$\text{Formato: }\$STATUS,NODE:\langle N\rangle,SERIE:\langle S\rangle,MODO:\langle M\rangle,ESTADO:\langle E\rangle,T:\langle S\rangle,RF:\langle R\rangle\%,RTT:\langle T\rangle ms,BAT:\langle V\rangle,HORA:\langle H\rangle*\langle CRC\rangle\backslash r\backslash n$$

**Ejemplo Maestro en Modo Automático:**
```text
$STATUS,NODE:MAESTRO,SERIE:A3F19C,MODO:AUTO,ESTADO:V1_R2,T:24,RF:98%,RTT:82ms,BAT:12.6,HORA:18:25:00*42\r\n
```

> **`SERIE` (N-62).** El firmware lo emite desde `f7d613f` y este manual no lo documentaba:
> la especificación describía una trama que ya no salía del micro. Son **24 bits en
> hexadecimal** derivados del UID de silicio de 96 bits del STM32 (`identidad.cpp`), iguales
> en las dos puntas y **no reescribibles**: identifican la **placa**, no el cruce donde esté
> montada hoy. El pack `documentos_03_trama_status` compara este apartado contra el `.cpp`
> en cada corrida del banco, así que la próxima vez que la trama cambie y el manual no, la
> compuerta lo dirá.
>
> ⚠️ **El separador de clave/valor es el PRIMER `:` de cada campo.** `HORA` lleva dos más
> dentro del valor (`18:25:00`); un parser que parta por todos los `:` se queda con `18`. Le
> pasaba a la app hasta N-62.

### 4.3 Trama de Alarma Inmediata ($ALARM) — Emitida ante incidentes
```text
$ALARM,NODE:MAESTRO,EVENTO:FALLO_RF,CAUSA:SILENCIO_25000ms,ACCION:CAMBIO_A_AMBAR,HORA:17:54:58*3D\r\n
```

> **Los dos checksums de arriba estaban mal** (`*4F` y `*3B` frente a los `*42` y `*43`
> reales), y llevaban desde el 26/08 en el manual y en el `.docx`. Un ejemplo con checksum
> inválido en el mismo apartado que explica cómo calcularlo es una trampa para quien escriba
> un parser: descarta la trama buena y busca el fallo donde no está. Ahora los recalcula el
> pack `documentos_03_trama_status` sobre **todos** los ejemplos de este manual.

### 4.4 Comandos desde la App hacia la Controladora (Protegidos por PIN)
```text
 ┌─────────────────────────────────────────────────────────────────────────────┐
 │ COMANDOS DE CONTROL CON VALIDACIÓN DE PIN (USART1 a 9600 bps)               │
 ├─────────────────────────────────────────────────────────────────────────────┤
 │ • CMD:PIN:1234:SET_MODO:AUTO\r\n            ➔ Pone el cruce en Automático.  │
 │ • CMD:PIN:1234:SET_MODO:MANUAL\r\n          ➔ Pone el cruce en Modo Manual. │
 │ • CMD:PIN:1234:SET_MODO:AMBAR\r\n           ➔ Pone el cruce en Ámbar Seguro.│
 │ • CMD:PIN:1234:FORZAR_ROJO\r\n              ➔ ROJO TOTAL DE EMERGENCIA.     │
 │ • CMD:PIN:1234:MANUAL:CAMBIAR_TURNO\r\n     ➔ Concede turno opuesto.        │
 │ • CMD:PIN:1234:TEST_LEDS\r\n                ➔ Inicia test de lámparas de 6s.│
 │ • CMD:PIN:1234:SET_RTC:YYYY-MM-DD,HH:MM:SS\r\n ➔ Ajusta reloj RTC.          │
 │ • CMD:PIN:1234:SOLICITAR_PASO\r\n     ➜ Solo ESCLAVO: pide al Maestro. │
 │ • CMD:FORZAR_ROJO\r\n                    ➜ SIN PIN. Ver nota abajo.   │
 └─────────────────────────────────────────────────────────────────────────────┘
```

### 🛑 El ROJO DE EMERGENCIA no pide PIN, y es deliberado

`mando.cpp` lo dejó escrito hace meses para el mando de relés: **«asimetría deliberada: lo seguro,
fácil; lo peligroso, difícil»**. Detener el tráfico es la acción **segura** —el equipo cae a
todo-rojo—, así que ponerle una clave delante solo retrasa a quien está viendo el incidente.

Se aceptan **las dos formas**: `CMD:FORZAR_ROJO` y `CMD:PIN:1234:FORZAR_ROJO` hacen lo mismo. El PIN
sigue guardando lo que **abre** paso o mueve luces.

> ⚠️ **Este apartado habla del MAESTRO. En el Esclavo el comando cambió de nombre (N-83) y el
> razonamiento de arriba no le vale.**
>
> ~~«Detener el tráfico es la acción segura, así que no pide PIN»~~ — en el Esclavo eso **no
> describe lo que ocurre**: `semaforo_iniciarFallo()` deja el equipo en `S_FALLO`, que es **ámbar
> intermitente a 500 ms con la talanquera ARRIBA**, no rojo. Ese camino **sí abre paso**. La
> exención de PIN sigue siendo correcta por otro motivo, escrito en
> `Esclavo/src/bluetooth.cpp:110-129`: un ámbar intermitente **no le da prioridad a nadie** —pone a
> los dos sentidos a pasar con precaución—, y una caída segura que exija recordar una clave delante
> de un accidente no es una caída segura.
>
> El literal `FORZAR_ROJO` **sigue existiendo en el Esclavo, rechazándose y enseñando el nombre
> bueno** (`:157` y `:176`). No se convirtió en alias mudo a propósito: quien lo manda tiene una app
> o un manual anteriores al cambio, y lo que necesita no es enterarse de que el comando no existe,
> sino de cómo se llama ahora. **El nombre vigente es `AMBAR_EMERGENCIA`, y qué contesta en cada
> estado es §4.5.**

### 📋 Qué acepta cada punta, y no es lo mismo

| Comando | Maestro | Esclavo | Por qué |
|---|---|---|---|
| `SET_MODO:AUTO` · `MANUAL` · `AMBAR` | ✅ con PIN | ❌ | El Maestro es el único que arbitra el ciclo |
| `MANUAL:CAMBIAR_TURNO` | ✅ con PIN | ❌ | idem |
| **`SOLICITAR_PASO`** | — | ✅ **con PIN** | **Pide**, no ordena: manda `CMD_DEMANDA` por radio y decide el Maestro |
| ~~`FORZAR_ROJO`~~ ~~✅ sin PIN en las dos~~ | ✅ **sin PIN** | 🛑 **RECHAZADO con motivo** | 🔴 **Esta fila era falsa desde N-83.** En el Esclavo `FORZAR_ROJO` **ya no se atiende**: contesta `$ERR,CMD:FORZAR_ROJO,DESC:RENOMBRADO_USE_AMBAR_EMERGENCIA` (**MEDIDO POR LECTURA**, `Esclavo/src/bluetooth.cpp:157` sin PIN y `:176` con PIN). En el Maestro sí hace rojo de verdad (`Maestro/src/bluetooth.cpp:253`). Llamarlas igual era el defecto |
| **`AMBAR_EMERGENCIA`** | ❌ *(el Maestro usa `SET_MODO:AMBAR`, con PIN)* | ✅ **sin PIN y con PIN** | Ámbar intermitente con la pluma arriba. **No es «lo seguro» a secas: SÍ abre paso a los dos sentidos** — la exención de PIN se razona en `Esclavo/src/bluetooth.cpp:110-129`, no en «esto para el tráfico». **Su tabla de respuestas es §4.5** |
| `SET_RTC:…` | ✅ con PIN | ✅ con PIN | Ajusta el reloj, no las luces |
| `TEST_LEDS` | ✅ con PIN | 🛑 **RECHAZADO** | Ver abajo |

### 🛑 Por qué el Esclavo rechaza `TEST_LEDS`

El test enciende 6 s de secuencia —rojo, ámbar y **verde**— sin mirar el estado del ciclo. Lanzado
sobre un Esclavo en servicio, ese verde sale **mientras el Maestro está dando paso al otro sentido**:
dos vehículos entrando de frente al tramo.

Y lo que costó ver: **conectarse al Esclavo *correcto* era igual de peligroso.** El fallo no es
equivocarse de poste —eso lo arreglaría una matrícula—, es que esa punta acepte mover luces. Por eso
la guarda no vive en la app: una app se actualiza, se instala otra, se usa una vieja. Vive en el
firmware, y la vigila el pack `esclavo_06_no_abre_paso`.

El Esclavo contesta `$ERR,CMD:TEST_LEDS,DESC:NO_EN_SERVICIO_USE_EL_MAESTRO` — un motivo legible, no
un silencio que el técnico leería como equipo colgado. **El test de lámparas se hace desde el
Maestro.**

> **Volverá al Esclavo** cuando exista un estado `FUERA DE SERVICIO` que el propio equipo conozca, no
> una promesa del manual.

### ✋ `SOLICITAR_PASO`: el funcional trabaja desde cualquier extremo

El PMT coloca a un funcional en el extremo que haga falta, y ese funcional **no tiene por qué saber
cuál de los dos postes es el Maestro**:

```text
   ORDEN     "ponte en verde"    -> el Esclavo la ejecutaria          NO
   PETICION  "hay demanda aqui"  -> viaja al Maestro por radio,
                                    el Maestro decide, aplica el
                                    todo-rojo y ordena                SI
```

Es **la misma demanda que manda la cámara**: un botón del funcional y un coche detectado significan
lo mismo. Con dos funcionales, uno en cada extremo, el Maestro **serializa** — ninguno concede nada,
los dos piden. Que pulsen a la vez tiene que ser aburrido, y así lo es.

Si la petición cae en la ventana de silencio de 3 s, el equipo contesta
`$ERR,CMD:SOLICITAR_PASO,DESC:REPITA_EN_UNOS_SEGUNDOS`. **No se finge un envío que no ocurrió:** si el
operario no lo sabe, vuelve a pulsar creyendo que no le hacen caso.

---

### 4.5 🟠 `AMBAR_EMERGENCIA` en el Esclavo — la tabla de respuestas

> 🛑 **ESTO ES ESPECIFICACIÓN, NO ES LO QUE EL FIRMWARE HACE HOY.** Escrito el **31/08/2026**.
> Ninguna línea de esta sección ha pasado banco, y el firmware de hoy contesta
> `RESULT:OK` **en los cinco casos**. Mientras esta tabla no esté implementada y vista fallar, el
> `$ACK` que llega del Esclavo **no distingue nada** y no debe leerse como si distinguiera.
>
> **La tabla vive AQUÍ y en ningún otro sitio.** El Manual 8 (§5.bis) y el Manual 18 (§3.6) la
> **referencian**; copiarla crearía dos versiones que alguien tendría que sincronizar a mano, que es
> exactamente el defecto que este proyecto lleva un mes cerrando.

#### 4.5.0 La decisión que ya está tomada, y de quién es

**Decisión del responsable, 31/08/2026:**

1. `CMD:AMBAR_EMERGENCIA` en el Esclavo **sale del Modo Degradado de forma ORDENADA** —por el
   todo-rojo de despedida—, **igual que hace `B·B·B` desde el mando**, en vez de limitarse a
   `semaforo_iniciarFallo()`.
2. **La jerarquía: la app es la superficie de mando; el `B·B·B` del mando es la vía de último
   recurso** —cuando no hay teléfono, no hay cobertura o el ESP32 se colgó—.

**Lo que esa decisión NO dijo, y es lo que especifica esta sección: qué contesta el equipo en cada
caso.** Y hace falta decirlo porque el arreglo directo —copiar el molde de `mando.cpp:129-141` a las
dos puertas— **lo rechazó el banco, con razón**. El pack `app_03_sin_ok_mudo` dijo:

```text
FALLA Esclavo / CMD:AMBAR_EMERGENCIA: llama a degradado_salir() y TIRA lo que
      devuelve -o no comprueba su guarda- y aun asi manda $ACK. El tecnico recibe
      una confirmacion de algo que puede no haber ocurrido, se va del poste, y el
      equipo se queda como estaba sin que nada lo diga
```

Es la barrera de `CLAUDE.md` §6 literal, y es **el mismo defecto que N-80 cerró en `SET_RTC`**
apareciendo en la rama de al lado. Por eso la respuesta tiene que **distinguir los casos**, y cuántos
son y cómo se llaman es especificación.

#### 4.5.1 Lo MEDIDO POR LECTURA — para que nadie lo suponga

*(fichero y línea; nadie lo ha ejercido ni en banco ni en arnés)*

| qué | dónde | qué dice |
|---|---|---|
| Puerta **SIN PIN** | `Esclavo/src/bluetooth.cpp:130-136` | `semaforo_iniciarFallo(); ambarEmergencia = true;` → `RESULT:OK` |
| Puerta **CON PIN** | `Esclavo/src/bluetooth.cpp:171-176` | lo mismo, literalmente |
| El molde bueno | `Esclavo/src/mando.cpp:129-141` | `if (degradado_gobiernaLuz()) degradado_salir(); else semaforo_iniciarFallo();` |
| **La otra mitad del molde** | `Esclavo/src/mando.cpp:274` | el **sostenedor**: `if (ambarLocal && !semaforo_senalEnCurso() && !degradado_gobiernaLuz() && semaforo_estado() != S_FALLO) semaforo_iniciarFallo();` |
| `degradado_salir()` | `Esclavo/src/modo_degradado.cpp:246-252` | es **`void`**, y tiene guarda interna: desde `DEG_RENDIDO` solo baja el cartel; desde `DEG_INACTIVO` o `DEG_SALIENDO` **no hace nada** |
| Los cinco estados | `Esclavo/include/modo_degradado.h:30-36` | `DEG_INACTIVO` · `DEG_ENTRANDO` · `DEG_ACTIVO` · `DEG_SALIENDO` · `DEG_RENDIDO` |
| `degradado_gobiernaLuz()` | `Esclavo/src/modo_degradado.cpp:367-369` | `ENTRANDO \|\| ACTIVO \|\| SALIENDO` — **`RENDIDO` NO gobierna la luz** |
| Duración del todo-rojo de despedida | `Esclavo/src/modo_degradado.cpp:108-111` y `:49` | `max(cfgDespeje × 1000 ms, ROJO_MINIMO_MS = 4000 ms)` |
| Rango legal del despeje | `Maestro/src/modo_automatico.cpp:34` (`DESPEJE_SEG_MIN = 10`, `MAX = 90`), llega al Esclavo por radio (`config_ciclo.cpp:158`) | **el todo-rojo de despedida dura entre 10 y 90 segundos** |
| `semaforo_iniciarFallo()` | `Esclavo/src/semaforo.cpp:256-260` | **no tiene guarda y no puede fallar**: pone `S_FALLO` siempre |
| El Esclavo **no tiene** comando de Bluetooth para ENTRAR en Degradado | `grep -in "degradado" Esclavo/src/bluetooth.cpp` → **CERO** | la entrada siempre la pide alguien delante del poste (mando `A·B·A·B`, o `menu.cpp:211`) o el arranque tras corte |

#### 4.5.2 🔴 La tabla — qué contesta en cada estado

**`<E>` es `CMD:AMBAR_EMERGENCIA`** en las respuestas de abajo; se escribe entero en la trama. El
`*XX` se omite en esta tabla a propósito: se calcula como dice §4.1, sobre la trama final.

| # | estado al recibir | qué hace | qué contesta |
|---|---|---|---|
| **A** | **`DEG_INACTIVO`** y la luz **no** está en `S_FALLO` — manda el Maestro por radio, o el equipo está en rojo/verde | `semaforo_iniciarFallo()` + arma el latch. **No puede fallar** (`semaforo.cpp:256`) | `$ACK,CMD:AMBAR_EMERGENCIA,RESULT:OK` |
| **B** | **`DEG_INACTIVO`** y la luz **ya** está en `S_FALLO` — por SFTY-6 (25 s de silencio), por watchdog, por `B·B·B`, o por un `AMBAR_EMERGENCIA` anterior | reinicia el parpadeo y arma el latch. **El latch sí cambia algo**: convierte un ámbar que el siguiente `CMD_GO_RED` del Maestro se llevaría en un ámbar **vetado** (`main.cpp:406`, `:416`, `:540`) | `$ACK,CMD:AMBAR_EMERGENCIA,RESULT:YA_EN_AMBAR_LATCH_PUESTO` |
| **C** | **`DEG_ENTRANDO`** o **`DEG_ACTIVO`** — el Degradado gobierna la luz y `degradado_salir()` **sí** opera | arranca la **salida ordenada**: todo-rojo de despedida de **10 a 90 s**, y el ámbar llega **al final**. Arma el latch | `$ACK,CMD:AMBAR_EMERGENCIA,RESULT:SALIENDO_TODO_ROJO` |
| **D** | **`DEG_SALIENDO`** — ya hay una salida en curso, pedida por otro (el mando, el menú, el regreso del radio, o un `AMBAR_EMERGENCIA` anterior) | `degradado_salir()` es **NO-OPERATIVO** (`modo_degradado.cpp:250`). Lo único que ocurre es que se arma el latch | 🟠 **`R-2` — PENDIENTE DE DECISIÓN DEL RESPONSABLE.** Ver §4.5.5 |
| **E** | **`DEG_RENDIDO`** — 48 h sin sincronizar; `degradado_actualizar()` ya lo dejó en ámbar intermitente (`modo_degradado.cpp:352-353`) | la luz ya está donde se pidió. Se arma el latch y **se baja el cartel** del modo a `DEG_INACTIVO` | `$ACK,CMD:AMBAR_EMERGENCIA,RESULT:YA_EN_AMBAR_LATCH_PUESTO` |
| **F** | PIN mal formado, **solo por la puerta con PIN** | nada | `$ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO` *(ya existe, `bluetooth.cpp:164`)* |

**Filas que NO existen, y por qué se dice en vez de callarlo:**

- **«La orden no se pudo cumplir»** no tiene fila porque **no hay ningún estado en el que
  `semaforo_iniciarFallo()` o el armado del latch puedan fallar** — MEDIDO: la función no tiene
  guarda y el latch es una asignación. Lo único que puede quedar sin ocurrir **en el instante de la
  respuesta** es el ámbar de la fila **C**, que llega hasta 90 s después; de ahí que su `RESULT:` no
  sea `OK`.
- **Señal de confirmación del mando en curso** (`semaforo_senalEnCurso()`, los destellos de
  `B·B·B`/`A·A·A`) **no es una fila**: `aplicarSalidas()` guarda y no escribe mientras dura la señal
  (`semaforo.cpp`, cabecera del test de lámparas), así que el **estado** pasa a `S_FALLO`
  inmediatamente y los **pines** enseñan la señal un par de segundos más. La respuesta no cambia, y
  el firmware **no necesita distinguirlo**.
- **`TEST_LEDS` en curso** no es una fila en esta punta: el Esclavo **rechaza** `TEST_LEDS`
  (`bluetooth.cpp:202`), así que no hay camino para que esté activo.

#### 4.5.3 🔴 Lo que el firmware NO puede distinguir hoy — y por eso se dice, no se inventa

**`DEG_SALIENDO` es un solo estado con dos finales distintos, y desde fuera no se pueden separar.**

**MEDIDO POR LECTURA:** `rendicionEnCurso` es `static bool` en `Esclavo/src/modo_degradado.cpp:58`,
se escribe en `:147` y `:234`, se lee en `:346` — y **no tiene getter**: `modo_degradado.h` no lo
declara. `degradado_estado()` devuelve `DEG_SALIENDO` en los dos casos.

| final de la salida en curso | dónde | dónde acaba la luz |
|---|---|---|
| salida normal (`rendicion = false`) | `modo_degradado.cpp:356` | `DEG_INACTIVO` — **la luz se queda en ROJO** |
| rendición por las 48 h (`rendicion = true`) | `modo_degradado.cpp:346-353` | `DEG_RENDIDO` + `semaforo_iniciarFallo()` — **ámbar** |

Consecuencia para la fila **D**: **el equipo no sabe si el ámbar que se le está pidiendo va a
aparecer solo al final del todo-rojo o no.** Eso es exactamente lo que decide `R-2`, y por eso `R-2`
tiene dos partes: qué se contesta, y si hace falta un getter para poder contestarlo con verdad.

#### 4.5.4 🔴 Las DOS condiciones que hay que cumplir ANTES de poder contestar `SALIENDO_TODO_ROJO`

**Sin las dos, la fila C es un `$ACK` que miente — y sería `CLAUDE.md` §6 otra vez, esta vez
introducido por el propio arreglo.** Las dos son **MEDIDAS POR LECTURA** y **RAZONADAS, NO
EJERCIDAS**.

**Condición 1 — el molde del mando tiene DOS mitades, y solo se estaba copiando una.**

`mando.cpp:129-141` es la primera. La segunda es el **sostenedor** de `mando.cpp:274`, y su propio
comentario dice para qué está: *«la orden del operario tiene que sobrevivir a lo que pase después: al
todo-rojo de salida del Degradado —que termina en INACTIVO, no en ámbar—»*.

`bluetooth.cpp` **no tiene sostenedor**. Copiada solo la primera mitad, el todo-rojo de despedida
termina en `DEG_INACTIVO`, **nadie enciende el ámbar, y el equipo se queda en ROJO** — que es lo
contrario de lo que se pidió, con un `$ACK` ya enviado.

**Condición 2 — el latch se revoca a sí mismo DURANTE el todo-rojo, en la vuelta siguiente.**

`bluetooth.cpp:292`: `if (ambarEmergencia && semaforo_estado() != S_FALLO) ambarEmergencia = false;`
corre **una vez por vuelta de `bluetooth_loop()`**. Y `degradado_salir()` → `iniciarSalida()` →
`semaforo_forzarRojo()` (`modo_degradado.cpp:145`) deja el estado en `S_ROJO`. O sea: **el latch
muere milisegundos después de armarse**, antes de que el todo-rojo llegue ni a la mitad, y con él
desaparecen los tres vetos de `main.cpp` (`:406`, `:416`, `:540`).

> La revocación **no está mal escrita**: su razonamiento (`bluetooth.cpp:275-291`) es correcto
> *mientras el ámbar sea inmediato*. Lo que rompe es que ahora hay un tercer estado que no existía
> cuando se escribió: **«ámbar pedido y todavía en camino»**. Un latch que solo sabe distinguir
> *«estoy en ámbar»* de *«ya no»* no puede sostener una orden que tarda 90 s en cumplirse.

**Requisito de especificación, que es lo que aquí se fija:** el Esclavo **no contesta**
`RESULT:SALIENDO_TODO_ROJO` mientras no sea cierto que, al terminar el todo-rojo de despedida, **la
luz queda en ámbar y el latch sigue puesto**. Cómo se consigue es implementación —un sostenedor
propio, un tercer valor del latch, o cualquier otra— y no se decide aquí; **lo que no se admite es la
rama sin esa garantía**, porque entonces el `$ACK` promete un ámbar que nadie va a encender.

> **Y esto se cierra con un arnés que hay que VER FALLAR antes de tocar una línea de firmware**
> (`CLAUDE.md` §8.bis). El pack `esclavo_08_ambar_en_degradado` ya existe y **nace en rojo a
> propósito**; lo que mide es el **texto** del C++ —que la rama se entere del Degradado—, **no** la
> consecuencia dinámica. Las dos condiciones de arriba **ese pack no las ve**, y hacen falta dos
> comprobaciones más que sí.

#### 4.5.5 🟠 Las dos invariantes que la implementación tiene que cumplir

1. **Las DOS puertas —con PIN (`:171-176`) y sin PIN (`:130-136`)— contestan EXACTAMENTE lo mismo.**
   Un parche a una sola deja media puerta abierta contestando el `$ACK` viejo. Ya hay un pack que lo
   caza: `esclavo_07_ambar_emergencia` compara las dos ramas.
2. **El `$ACK`/`$ERR` de cada rama nombra el MISMO comando que la rama atiende** —`AMBAR_EMERGENCIA`,
   nunca `FORZAR_ROJO`—. Es la propiedad 2 de `esclavo_07_ambar_emergencia`, y es la general: impide
   que el defecto de N-83 vuelva con cualquier otro nombre.

#### 4.5.6 🟠 Las filas que decide el RESPONSABLE — no se eligen en silencio

**Están aquí porque lo que se elija LO VE UN CONDUCTOR**, o cambia lo que el operario cree que
consiguió. Ninguna está decidida.

| # | Qué hay que decidir | Opciones y su consecuencia | Dueño |
|---|---|---|---|
| **R-1** | **El ámbar de emergencia pedido desde la app tarda de 10 a 90 s en aparecer** cuando el Degradado gobierna la luz (fila **C**). Es el precio del todo-rojo de despedida, y sale de `cfgDespeje`, que el instalador configura | **(a) Se acepta tal cual** —es exactamente lo que ya cuesta `B·B·B`, y es la decisión del 31/08 leída al pie de la letra—. Un incidente en el tramo espera hasta minuto y medio a ver el ámbar. **(b) Se acota el todo-rojo de la vía de emergencia** a `ROJO_MINIMO_MS` (4 s) o a un número que se fije aquí. Consecuencia: el ámbar llega antes, pero el margen que garantiza que el tramo quedó vacío **se recorta justo en la transición menos vigilada** — y ese margen es el único de todo el modo. **(c) Ámbar inmediato sin todo-rojo** = revocar la decisión del 31/08: quien venía lanzado con verde por reloj se encuentra una señal que invita a negociar el paso creyendo que aún tiene prioridad | **Responsable** |
| **R-2** | **Qué contesta la fila D** (`DEG_SALIENDO`: ya hay una salida en curso, la pidió otro). `degradado_salir()` no hace nada; el latch sí se arma; y **el firmware no sabe si esa salida termina en ámbar o en rojo** (§4.5.3) | **(a) `$ACK,CMD:AMBAR_EMERGENCIA,RESULT:SALIDA_YA_EN_CURSO`** — informa sin prometer `OK`. El operario espera. Es honesto **solo si** se cumple la Condición 1 de §4.5.4, porque entonces el ámbar llega igual venga la salida de donde venga. **(b) `$ERR,CMD:AMBAR_EMERGENCIA,DESC:SALIDA_YA_EN_CURSO`** — rechaza. Consecuencia: el operario cree que **no le hicieron caso** y va a por el mando (`B·B·B`), que es la vía de último recurso — y **el latch sí se armó**, así que el `$ERR` sería falso en la única parte que sí ocurrió. **(c) Dar un getter a `rendicionEnCurso`** y contestar distinto según el final: `RESULT:SALIDA_YA_EN_CURSO` si acaba en ámbar, y `$ERR` con motivo si acaba en rojo y nadie va a encender el ámbar. Consecuencia: es la única que dice la verdad en los dos casos, y cuesta una función de una línea | **Responsable** *(la (c) además necesita firmware)* |
| **R-3** | **Si `AMBAR_EMERGENCIA` debe poder REVOCARSE desde la app.** Hoy no: `bluetooth.h` lo declara —*«no hay comando de Bluetooth que lo revoque, y es coherente con el del mando»*—, y la única salida es `A·A·A` en el mando. Con la app como superficie de mando (decisión del 31/08), **el técnico sin mando a mano no puede deshacer su propia orden** | **(a) Se queda como está** —*«una salida de emergencia que se pueda cancelar desde fuera no es una salida»*—. Consecuencia: hace falta subir al poste. **(b) Un comando de revocación con PIN.** Consecuencia: lo que hoy solo puede deshacer alguien delante del gabinete pasa a poder deshacerse por radio desde cualquier sitio, incluido mientras alguien trabaja bajo la luz | **Responsable** |
| **R-4** | **Qué pasa si se ENTRA en Degradado con el latch de Bluetooth puesto.** Es el agujero que el arreglo de C **no cierra por sí solo**; ver §4.5.7 | Ver §4.5.7 | **Responsable** |

#### 4.5.7 🔴 El segundo agujero: ENTRAR en Degradado con el ámbar de la app puesto

**Este caso queda vivo aunque se arregle todo lo anterior**, y hoy no lo cubre nadie.

**MEDIDO POR LECTURA:** `degradado_entrar()` (`Esclavo/src/modo_degradado.cpp:212-243`) llama a
`semaforo_forzarRojo()` en `:224` **sin guarda ninguna** — no pregunta por `bluetooth_ambarEmergencia()`
ni por `mando_ambarLocal()`. Y el sostenedor del Degradado escribe luz por otra puerta:
`aplicarLuz()` desde `degradado_actualizar()` (`main.cpp:363`), que **tampoco** está vetada por
`bluetooth_ambarEmergencia()`.

**RAZONADO, NO EJERCIDO** —y corrige por lo alto lo que se había escrito en `ESTADO.md` §N-106:

- No es que el ámbar *«aguante hasta el siguiente cambio de fase»*. El todo-rojo **de entrada** saca
  la luz de `S_FALLO` **en el instante de entrar**, así que `bluetooth.cpp:292` revoca el latch **en
  la vuelta siguiente**, no en el siguiente cambio de fase.
- A partir de ahí los tres vetos de `main.cpp` se apagan, y en la siguiente frontera de fase
  `aplicarLuz()` puede dar **verde por reloj** donde se había pedido ámbar de precaución.
- El `$ACK` ya se envió hace rato. **Nada se lo dice a nadie.**

**Quién puede entrar en Degradado, y por qué importa aquí:** el Esclavo **no tiene comando de
Bluetooth** para entrar (MEDIDO: cero coincidencias de `degradado` en su `bluetooth.cpp`). Las vías
son el mando `A·B·A·B` (`mando.cpp`, `ACC_DEGRADADO`), la pantalla (`menu.cpp:211` y `P_CONFIRMAR`) y
`degradado_reanudarTrasCorte()` al arrancar. **La reanudación tras corte no está afectada**: el latch
vive en RAM y arranca en `false`. Es decir que el caso real es siempre **alguien delante del poste
entrando en Degradado mientras un ámbar pedido por teléfono sigue vigente**.

> **La comparación que lo deja claro: el mando ya resolvió esto, y lo resolvió EXPLÍCITAMENTE.**
> `ejecutar(ACC_DEGRADADO)` pone `ambarLocal = false` antes de llamar a `degradado_entrar()` (`mando.cpp:144-148`). O sea
> que el mando **declara** que entrar en Degradado revoca su propio ámbar. El latch de Bluetooth **no
> lo revoca nadie: se cae solo**, que es la misma opción tomada **en silencio y por accidente** — la
> peor de las tres.

**`R-4` — las opciones, para el responsable:**

| opción | qué hace | consecuencia |
|---|---|---|
| **(a) El Degradado RECHAZA la entrada mientras haya ámbar de emergencia vigente** | un `RechazoDegradado` nuevo —en la forma que ya usa el enum, p. ej. `DEG_RECHAZO_AMBAR_VIGENTE`, texto `"AMBAR EMERGENCIA VIGENTE"`— que la pantalla enseña igual que los otros cinco | El operario del poste **no puede** entrar en Degradado hasta que alguien revoque el ámbar, y hoy eso solo se hace con `A·A·A` en el mando. Con la app como superficie de mando y `R-3` en la opción (a), **puede quedar bloqueado**: las dos decisiones están atadas |
| **(b) Entrar en Degradado REVOCA el latch explícitamente**, como hace el mando con `ambarLocal` | quien está delante del gabinete manda sobre quien está al teléfono | Es el comportamiento de hoy, pero **declarado**: se escribe un `$EVENT` que lo diga y deja rastro en la caja negra, en vez de que el latch se caiga solo sin que nadie se entere. **El ámbar que alguien pidió por precaución desaparece**, y el que lo pidió no está delante para verlo |
| **(c) El latch VETA la luz del Degradado**, igual que veta la del Maestro | `aplicarLuz()` y el todo-rojo de entrada pasan a mirar `bluetooth_ambarEmergencia()` | Es lo más coherente con lo que el latch promete —*«pesa lo mismo que el del mando»*—, pero deja el equipo **en Degradado nominal y en ámbar real**: dos autoridades sobre la misma luz, que es justo lo que SFTY-21 evita. Y la otra punta seguiría dando verde por reloj creyendo que ésta está en su fase |

> **Ninguna de las tres es gratis y la tercera parece la peor**, pero eso es una opinión técnica y la
> decisión no lo es: **lo que se elija lo ve un conductor**.

---

## 5. Operación en Corredor Vial (Un solo celular para múltiples cruces)

En un proyecto de carretera con múltiples cruces (ej. Km 12, Km 24, Km 38), el técnico opera toda la concesión con **un solo teléfono móvil**:

### 5.1 Identificación de Nodos y Roles
1. **👑 Nodo Maestro (Poste 1):** Controla el ciclo global, programas de verde/despeje y radio hacia el Esclavo.
2. **📡 Nodo Esclavo (Poste 2):** Opera subordinado al Maestro y reporta diagnóstico local de batería, focos y recepción RF.

### 5.2 Modo Courier RTC (Sincronización Puente sin Radio)
Cuando no hay cobertura de radio entre los dos postes:
1. El técnico abre la App junto al **Maestro (Poste 1)** y toca **`[ 📸 Capturar Maestro ]`** (memoriza hora y ciclo).
2. El técnico viaja en moto/vehículo hasta el **Esclavo (Poste 2)**; la App cronometra el viaje con su reloj interno de alta resolución.
3. Al conectarse al Esclavo, toca **`[ 🚀 Inyectar en Esclavo ]`**; la App inyecta la hora exacta:  
   $$\text{Hora Inyectada} = \text{Hora Capturada} + \text{Tiempo de Viaje}$$
   logrando un desfase $\Delta t < 0.1\text{ s}$ para el Modo Degradado sin cables ni escaleras.
