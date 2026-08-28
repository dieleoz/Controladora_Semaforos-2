# 📱 MANUAL TÉCNICO Y ESPECIFICACIÓN INTEGRAL DE LA APP MÓVIL Y BLUETOOTH (V9.0)

**Sistema:** Controladora de Semáforos Móviles de 3 Estados (Maestro y Esclavo V9.0)  
**Módulo de Diagnóstico:** Módulo Bluetooth Serial **SPP (Clásico)** — **no BLE** (Estándar Probado en Proyecto Baliza)  
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

### ✅ Decisión de obra del 28/08: se sigue con el módulo SPP dedicado

**Se instala `HC-05` / `JDY-30`, no ESP32.** El motivo no es la preferencia: es que un módulo SPP
dedicado **deja esta decisión congelada intacta** —el puente nativo Android, el emparejado en
Ajustes y el flujo que el técnico ya domina siguen siendo los mismos— y su consumo cabe en la
alimentación que la tarjeta ya tiene.

El ESP32 queda documentado como **ALTERNATIVA, no como sustituto**, por si los `HC-05` no llegan a
tiempo, y con las condiciones separadas porque **no cuestan lo mismo**:

| Camino | Qué exige | Estado de este apartado 1 |
|---|---|---|
| `HC-05` / `JDY-30` (SPP) | Nada nuevo. Es lo que este manual ya especifica | **Intacto** |
| ESP32 **clásico** (`WROOM-32/-32D/-32E/-32U`) haciendo de puente SPP | Confirmar la referencia por serigrafía o `chip_id`, y **fuente propia de 12 V** | **Intacto**: sigue siendo Bluetooth Clásico SPP, que es lo que este apartado congela |
| ESP32 **por WiFi** (`S3`, `C3`, `S2`, o cualquiera renunciando al SPP) | Rehacer el transporte de la app entero | 🛑 **Exige REABRIR ESTE APARTADO 1 POR ESCRITO**, antes de comprar y antes de escribir una línea |

> **Esa última fila no es burocracia, y no se ablanda.** El valor de una decisión congelada está
> justamente en que cambiarla cueste un documento: la vez anterior se cambió de transporte sin
> escribirlo y el resultado fue una app que no conectaba con nada. Un ESP32 por WiFi no es «el mismo
> módulo con otra radio»: cambia el emparejado, el descubrimiento, la autonomía sin internet y el
> flujo del técnico. Si se toma ese camino, **se reabre aquí, con fecha y firma, antes**.

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

> 🔴 **Esa última fila es una advertencia, no un formalismo.** El `.kicad_pcb` de este proyecto está
> vacío: entre el esquemático y la tarjeta que hay sobre la mesa **no existe ningún artefacto que
> las ate**, y el ruteo pudo hacerse a mano. **Antes de enchufar el módulo, compruebe con el
> multímetro las cuatro vías** (continuidad de p2 a la pata 43 de `U1`, de p3 a la 42, y tensión en
> p6 y p7). Son cinco minutos y son los que separan una hipótesis muy buena de un hecho.

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

### 2.3 Diagrama de conexión — VIGENTE

```text
 ┌─────────────────────────────────────────────────────────────────────────────┐
 │      CONEXION DE TELEMETRIA BLUETOOTH  --  CONECTOR J17  (VIGENTE 28/08)    │
 ├─────────────────────────────────────────────────────────────────────────────┤
 │                                                                             │
 │   MODULO BLUETOOTH (HC-05 / JDY-30)        TARJETA -- CONECTOR J17          │
 │   ┌────────────────────────────┐        ┌──────────────────────────────┐    │
 │   │  [ TXD ]  transmite        ├────────┤► J17 p2 = PB7  (U1 pin 43)   │    │
 │   │                            │        │           USART1 RX del STM32│    │
 │   │  [ RXD ]  recibe           ├────────┤► J17 p3 = PB6  (U1 pin 42)   │    │
 │   │                            │        │           USART1 TX del STM32│    │
 │   │  [ VCC ]  3,3 V            ├────────┤► J17 p6 = 3,3 V   (o p8)     │    │
 │   │  [ GND ]  masa             ├────────┤► J17 p7 = GND     (o p9)     │    │
 │   └────────────────────────────┘        └──────────────────────────────┘    │
 │                                                                             │
 │   LAS LINEAS DE DATOS VAN CRUZADAS:                                         │
 │      TXD del modulo  ->  RX del micro (p2).                                 │
 │      RXD del modulo  ->  TX del micro (p3).                                 │
 │   Conectarlas en paralelo -TXD contra TX- no rompe nada, pero no llega      │
 │   ni una trama, y el sintoma es identico al de un modulo muerto.            │
 └─────────────────────────────────────────────────────────────────────────────┘
```

| hilo del módulo | va a | pin del `U1` | qué es |
|---|---|---|---|
| **`TXD`** | **`J17` p2** | `PB7` (p43) | `USART1` **RX** del STM32 — el micro **escucha** aquí |
| **`RXD`** | **`J17` p3** | `PB6` (p42) | `USART1` **TX** del STM32 — el micro **habla** aquí |
| `VCC` | `J17` p6 (o p8) | — | 3,3 V |
| `GND` | `J17` p7 (o p9) | — | masa común |

**Por qué estos dos pines y no otros:** son los que dejó libres la **pantalla LCD, que se retira**
(ver `1_Manual_Usuario.md` y `OPTIMIZACIONES.md`, ambos del 28/08). El equipo va montado en alto y
la pantalla no se lee desde el suelo, así que **la app pasa a ser la única interfaz de operación**.
En `J17`, `PB7` era el `RST` del display y `PB6` su nivel estático de modo serie: **ninguno de los
dos transportaba datos**, y por eso el puerto serie sale gratis sin tocar los tres hilos que sí
dibujan (`PB3`, `PB4`, `PB5`).

> ⚠️ **PENDIENTE DE CONFIRMAR — la tensión de `VCC` del módulo que se instale.** `J17` entrega
> **3,3 V, y solo 3,3 V**: en esta tarjeta **los 5 V son un raíl interno que no sale a ningún
> conector de señal**. Muchas placas de evaluación de `HC-05` (tipo `ZS-040`) llevan su propio
> regulador y piden **3,6–6 V** en el pin rotulado `VCC`, aunque casi todas exponen además una
> entrada de 3,3 V directa al chip. **Antes de alimentar: lea la serigrafía del módulo concreto que
> tenga en la mano.** Si esa placa exige 5 V en `VCC` y no expone entrada de 3,3 V, **no hay 5 V en
> `J17`** y hay que resolverlo por escrito antes de cablear — no improvisando un hilo desde el
> regulador, que es soldadura en placa. Esto **no está resuelto en este manual**.

> ⚡ **Alimentar desde el riel de la tarjeta vale para un `HC-05`/`JDY-30` (~40 mA) y NO vale para un
> ESP32** (picos de ~500 mA). Ver el apartado 1, «Si se va con un ESP32, el consumo no es un detalle
> de montaje». **El diagrama de arriba no vale para un ESP32.**

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

### 📋 Qué acepta cada punta, y no es lo mismo

| Comando | Maestro | Esclavo | Por qué |
|---|---|---|---|
| `SET_MODO:AUTO` · `MANUAL` · `AMBAR` | ✅ con PIN | ❌ | El Maestro es el único que arbitra el ciclo |
| `MANUAL:CAMBIAR_TURNO` | ✅ con PIN | ❌ | idem |
| **`SOLICITAR_PASO`** | — | ✅ **con PIN** | **Pide**, no ordena: manda `CMD_DEMANDA` por radio y decide el Maestro |
| `FORZAR_ROJO` | ✅ **sin PIN** | ✅ **sin PIN** | Dirección segura, desde cualquier extremo |
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
