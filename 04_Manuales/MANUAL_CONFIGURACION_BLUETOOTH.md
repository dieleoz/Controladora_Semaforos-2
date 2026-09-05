# 📱 MANUAL TÉCNICO Y ESPECIFICACIÓN INTEGRAL DE LA APP MÓVIL Y BLUETOOTH (V9.0)

**Sistema:** Controladora de Semáforos Móviles de 3 Estados (Maestro y Esclavo V9.0)  
**Módulo de Diagnóstico:** Módulo Bluetooth Serial SPP / BLE (Estándar Probado en Proyecto Baliza)  
**Software Móvil:** App Android (.apk) con Frontend Reactivo Dark-Theme (Estándar IOT-VIAL)  
**Propósito:** Telemetría en tiempo real, caja negra de alarmas, test de banco, sincronización Courier RTC y control desde el suelo con PIN  
**Verificación Hardware:** Esquemáticos KiCad `Controladora_Semaforos.kicad_sch`, `pines.h` y `MAPEO_TARJETA_KICAD.md`  
**Fecha de Emisión:** 26 de Agosto de 2026  
**Última revisión:** 31 de agosto de 2026 — ver el aviso de cabecera

---

> # 🔴 REVISIÓN DEL 31/08/2026 — LEA ESTO ANTES QUE NADA
>
> Este manual llevaba sin tocarse desde el 26/08 y **describía dos cosas que ya no son ciertas**. Lo
> superado se tacha con su motivo; no se borra, porque una corrección silenciosa se vuelve a proponer
> al mes siguiente.
>
> ## 1. 🛑 `FORZAR_ROJO` YA NO EXISTE EN EL ESCLAVO — Y ERA EL BOTÓN DE PÁNICO
>
> **MEDIDO el 31/08 sobre `01_Firmware/Esclavo/src/bluetooth.cpp`:**
>
> ```
>   Esclavo/src/bluetooth.cpp:157   if (strcmp(cmd, "CMD:FORZAR_ROJO") == 0) {
>   Esclavo/src/bluetooth.cpp:158     enviarTramaConCrc("$ERR,CMD:FORZAR_ROJO,DESC:RENOMBRADO_USE_AMBAR_EMERGENCIA");
>   Esclavo/src/bluetooth.cpp:176   } else if (strcmp(accion, "FORZAR_ROJO") == 0) {     <- la forma CON PIN
>   Esclavo/src/bluetooth.cpp:182     enviarTramaConCrc("$ERR,CMD:FORZAR_ROJO,DESC:RENOMBRADO_USE_AMBAR_EMERGENCIA");
> ```
>
> **Las dos formas —con PIN y sin PIN— contestan `$ERR`. El Esclavo no hace nada.** Este manual lo
> daba por válido en `§4.4` y en la tabla de `§ Qué acepta cada punta`, así que un operario que
> siguiera estas páginas creería haber detenido el cruce **sin haber detenido nada**. Es lo más grave
> que había aquí escrito y por eso encabeza la revisión.
>
> **El comando vigente en el Esclavo es `CMD:AMBAR_EMERGENCIA`** (`bluetooth.cpp:130` sin PIN,
> `:171` con PIN). Y el renombrado no es cosmético: lo que esa punta hace es
> `semaforo_iniciarFallo()` —**ámbar intermitente**—, no rojo. El nombre viejo prometía rojo y hacía
> ámbar, que es casi lo contrario; se corrigió el nombre, no el comportamiento.
>
> **En el MAESTRO `CMD:FORZAR_ROJO` se queda y allí SÍ hace rojo de verdad** (`Maestro/src/bluetooth.cpp:145`).
> Que las dos puntas usen literales distintos es lo correcto, porque hacen cosas distintas.
>
> ## 2. El módulo Bluetooth es hoy un **ESP32 de expansión**, y ya no entra por `PA9`/`PA10`
>
> El `§2` de este manual sigue dibujando el módulo sobre `PA9`/`PA10`. **Ese cableado ha quedado
> obsoleto** (ver el aviso dentro de la propia `§2`), y además el módulo SPP discreto ha sido
> **sustituido por un ESP32 de expansión** que entra por `J17`.
>
> ## 3. Y lo que NO cambia: **nada de esto ha pasado banco**
>
> En campo corre la **V8.4**. Todo lo descrito aquí está validado en simulador y arneses de PC.
> **Un manual corregido no es un permiso de carga.**

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
 │ • MODULO:        ESP32-WROOM-32 clasico (BT v4.2 BR/EDR + BLE) por J17.     │
 │                  Sustituye al modulo SPP discreto. NO HM-10, NO JDY-31.    │
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
| Módulos **solo BLE** (`HM-10`, `JDY-31`) | Obligarían a rehacer el puente nativo y cambiar el flujo del técnico |

### ✅ 31/08/2026 — el módulo está identificado, y la decisión de arriba se sostiene sin cambios

> **`BLQ-1` está CERRADO.** Si algún documento lo sigue dando por abierto, ese documento está
> caducado.

**MEDIDO:** el módulo del `J17` es un **`ESP32-WROOM-32` clásico** —`Xtensa LX6` de doble núcleo,
**`Bluetooth v4.2 BR/EDR + BLE`**—. `BR/EDR` es Bluetooth **clásico**, así que **hay perfil SPP** y
la app conecta **sin tocar una línea**: el puente nativo de Baliza vale tal cual.

**Y el firmware de ese ESP32 YA EXISTE y compila.** Vive en `01_Firmware/ESP32_Expansion/`
(`main.cpp`, `puente.cpp`, `despachador.cpp`, `enlace_stm32.cpp`, `transporte_app.cpp`,
`reloj_ds3231.cpp`, `trama.cpp`, `vigilante.cpp`) y la compuerta lo compila como una suite más —
**la cifra de flash se lee del acta de `evidencia/`, no de aquí**.

| | |
|---|---|
| **Qué es** | **Módulo de expansión, no un segundo controlador.** El STM32 sigue siendo el controlador del cruce |
| **Qué aporta** | El **Bluetooth** (sustituye al módulo SPP discreto) y un **reloj `DS3231`** con pila propia, en `GPIO21` (`SDA`) / `GPIO22` (`SCL`) — `ESP32_Expansion/include/contrato.h:142-143` |
| **Qué NO hace** | 🛑 **No manda sobre las luces.** Es un puente: traduce y reenvía. La barrera de salidas sigue viviendo en `semaforo.cpp` del STM32 |
| **Por dónde entra** | `J17`, sobre el `USART1` **remapeado** a `PB6`/`PB7` — ver `§2` |

> ⚠️ **Que el ESP32 traiga también BLE no reabre nada.** La decisión congelada es *usar SPP*, y este
> chip lo tiene. No es una excepción al párrafo de arriba: es la confirmación de que se puede
> cumplir.
| Reconexión automática | El operario camina al Km 24, el teléfono se reengancha solo al Km 12 que sigue en rango, y la pantalla muestra un sistema vivo **que está a 12 km**. No hay forma de que lo note |
| Dos conexiones simultáneas | Físicamente imposible —los postes de una pareja están a cientos de metros y el Bluetooth alcanza 10-15 m— y Baliza ya usa un único socket. **Una a la vez es una propiedad de seguridad, no una limitación** |

### El flujo de campo, que es el de Baliza

```text
  1. Ajustes de Android > Bluetooth > emparejar > PIN 0000 o 1234
  2. Abrir la app > "Buscar" > lista los emparejados (nombre + MAC)
        SEM-7A3F-M   Maestro, Km 12
        SEM-7A3F-E   Esclavo,  Km 12
        SEM-C104-M   Maestro, Km 24
  3. Tocar uno > socket RFCOMM sobre SPP > leer/escribir lineas
```

El nombre visible del módulo lo fija el firmware con `AT+NAME`, así que **la lista de Android ya dice
quién es cada equipo antes de conectar**. El técnico lee; no adivina.

### Lo que la app NO puede hacer, aunque el operario lo pida

**Al Esclavo no se le manda nada que abra paso.** Puede leer telemetría, ajustar el reloj, pedir
~~forzar rojo~~ **`AMBAR_EMERGENCIA`** —que es la dirección segura— y **solicitar paso**, que viaja
por radio al Maestro como una demanda: el Maestro decide, aplica el todo-rojo y ordena. Esa asimetría
—**el Esclavo pide y el Maestro decide**— vive escrita en la puerta única por la que entra una
demanda: `Maestro/include/demanda.h` y `Esclavo/include/demanda.h`.

> 🛑 **Corregido el 31/08:** decía *«forzar rojo»* y ese literal **ya no funciona en el Esclavo**.
> Ver el aviso de cabecera y `§4.4`. Lo que esa punta hace es **ámbar intermitente**, no rojo.

> 🛑 **Corregido el 01/09 — el puntero mandaba a la regla equivocada.** Decía ~~*«El detalle está en
> `OPTIMIZACIONES.md` § SFTY-27»*~~. **MEDIDO:** el apartado que lleva ese número se titula
> *«SFTY-27 — Matrícula de pareja: quién obedece a quién»* y está marcado **DISEÑO, NO
> IMPLEMENTADO** — no es la asimetría de la demanda. El número `SFTY-27` designa hoy dos reglas
> distintas y **renumerar es decisión del responsable**, así que aquí se hace lo único que no depende
> de esa decisión: **se apunta a donde la regla está escrita de verdad**, que es el fuente.

> **Consecuencia para el operario, que es lo que importa:** da igual en qué extremo esté. Los dos
> postes se operan igual y en la pantalla **no aparecen las palabras «maestro» ni «esclavo»** —
> aparece el sentido de la vía.

---

## 2. Diagrama de Conexión Físico y Desacoplo de Hardware (KiCad)

> # 🔴 31/08 — ESTE APARTADO DESCRIBE EL MONTAJE ANTERIOR. EL VIGENTE ES `J17` / `PB6`-`PB7`
>
> **MEDIDO** sobre el fuente: `static HardwareSerial SerialBT(PB7, PB6);` — **idéntico en las dos
> puntas**. Es el `USART1` **remapeado**, no un segundo puerto serie, y el STM32F103 solo lo saca
> **por un sitio a la vez**.
>
> | `J17` | pin del STM32 | al módulo |
> |---|---|---|
> | **2** | **`PB7`** — `USART1_RX` | `TXD` |
> | **3** | **`PB6`** — `USART1_TX` | `RXD` |
> | **6** | 3,3 V | `VCC` |
> | **7** | GND | `GND` |
>
> **`PA9`/`PA10` sigue siendo válido eléctricamente, pero NO SALE A NINGUNA BORNERA:** habría que
> soldar en las patas del micro o del `MAX3485 U2`. Queda como alternativa de laboratorio.
>
> ⛔ **Y `J16` NO es `J17`.** ~~`J16` es la botonera~~ → **`J16` fue la botonera; hoy es el
> conector de las CÁMARAS** (`p10` = `CAM_C_PIN`, `p12` = `CAM_D_PIN`, `D-2`/`D-3`), con `p5` y
> `p8` **libres de cobre pero todavía leídos por el firmware**. **Su posición 1 lleva 12 V
> crudos** —taparla es obligatorio en cada equipo (`D-4`, N-120)—; el módulo
> es de 3,3 V. Confundirlos lo quema sin aviso previo. El procedimiento para distinguirlos **con
> multímetro** —y el aviso de contar los pines desde el pin 1, porque símbolo y footprint no tienen
> el mismo número de posiciones— está en `05_Funcional/2_Manual_Hardware_y_Pruebas.md §8`. **Este
> manual no lo duplica a propósito.**
>
> Lo que sigue **no se borra**: era correcto para el montaje anterior y el desacoplo de `PA8` que
> describe **el firmware lo sigue haciendo**, aunque ya no sea necesario para el Bluetooth.

~~En la tarjeta controladora del semáforo, el módulo Bluetooth se conecta al puerto **`USART1` (`PA9` TX y `PA10` RX)**:~~

```text
 ┌─────────────────────────────────────────────────────────────────────────────┐
 │               CONEXIÓN DE TELEMETRÍA BLUETOOTH EN TARJETA MADRE             │
 ├─────────────────────────────────────────────────────────────────────────────┤
 │                                                                             │
 │   MÓDULO BLUETOOTH (HC-05 / JDY-30)           TARJETA CONTROLADORA STM32    │
 │   ┌─────────────────────────────┐          ┌────────────────────────────┐   │
 │   │  [ VCC ] (3.6V - 6.0V)      ├──────────┤► Pin 5V (o 3.3V)           │   │
 │   │  [ GND ] (Tierra)           ├──────────┤► Pin GND (Tierra común)    │   │
 │   │  [ TXD ] (Transmisión)      ├──────────┤► Pin PA10 (USART1 RX)      │   │
 │   │  [ RXD ] (Recepción)        ├──────────┤► Pin PA9  (USART1 TX)      │   │
 │   └─────────────────────────────┘          └────────────────────────────┘   │
 │                                                                             │
 └─────────────────────────────────────────────────────────────────────────────┘
```

### ⚙️ Desacoplo Eléctrico del Transceptor MAX3485 (~~U3~~ **`U2`**) en Hardware:

> 🔧 **Corrección de chip, 31/08:** este manual decía **`U3`** donde va **`U2`**. Trazado red por red
> sobre `Controladora_Semaforos.kicad_sch`: **`U2`** es el MAX3485 del **`USART1`** (`RO`→`PA10`,
> `~RE`/`DE`→`PA8`, `DI`→`PA9`, par A/B por `J10`); **`U3`** es el del **`USART3`**, el de la **radio
> LoRa** (`PB10`/`PB11`/`PB12`, par A/B por `J12`). Se corrige dejando constancia porque una
> corrección silenciosa se vuelve a proponer al mes siguiente.
>
> ⚠️ **Y el matiz que este manual no decía: `PA8` gobierna A LA VEZ `~RE` (pin 2) y `DE` (pin 3).**
> Ponerlo en `HIGH` apaga el receptor —que es lo que se busca— **pero deja el transmisor
> ENCENDIDO**: `U2` vuelca la telemetría por `J10` de forma permanente y esa línea **no puede recibir
> nunca**. Hoy es inofensivo porque `J10` está vacío; el día que alguien cuelgue algo de `J10` **hay
> que tocar el código, no el cableado** (`01_Firmware/TROUBLESHOOTING.md`, lección del repetidor del
> 31/07/2026).

Dado que ~~`U3`~~ **`U2`** (`MAX3485`) está físicamente conectado a `PA9`/`PA10`/`PA8` en la PCB:
* **Manejo en Firmware:** El pin `PA8` (`RS485_IN_DE_RE`) se configura en **`HIGH` permanente**:
  $$\text{pinMode}(\text{PA8}, \text{OUTPUT});\quad \text{digitalWrite}(\text{PA8}, \text{HIGH});$$
* **Efecto Físico:** Al poner $\text{DE}/\overline{\text{RE}} = 1$, la salida del receptor $\text{RO}$ de `U3` pasa a **Alta Impedancia ($\text{Hi-Z}$)**, liberando completamente el pin `PA10` y evitando cualquier conflicto de corriente con el `TXD` del módulo Bluetooth.

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

  > 🔴 **AVISO — tres de esos campos NO son medidas.** `RF:` y `RTT:` son **literales** en el Esclavo
  > (`Esclavo/src/bluetooth.cpp:328`: `RF:98%%,RTT:85ms`) y `BAT:12.6` es literal en **las dos
  > puntas**. El `RF:98%` del Esclavo se emite igual **con la antena desconectada**. No se usan para
  > juzgar el enlace ni la batería, y **no se apuntan en un acta como si fueran medidas**. Detalle en
  > `05_Funcional/2_Manual_Hardware_y_Pruebas.md §8`.

* **Botón de Pánico:** ~~Forzado inmediato de All-Red ante emergencias viales.~~
  **31/08 — depende de la punta, y no es un detalle de interfaz:**
  * **Maestro** → `CMD:FORZAR_ROJO`, sin PIN. **Rojo total de verdad.**
  * **Esclavo** → `CMD:AMBAR_EMERGENCIA`, sin PIN. **Ámbar intermitente.** `FORZAR_ROJO` aquí
    contesta `$ERR` y **no detiene nada**.

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
$$\text{Formato: }\$STATUS,NODE:\langle N\rangle,MODO:\langle M\rangle,ESTADO:\langle E\rangle,T:\langle S\rangle,RF:\langle R\rangle\%,RTT:\langle T\rangle ms,BAT:\langle V\rangle,HORA:\langle H\rangle*\langle CRC\rangle\backslash r\backslash n$$

**Ejemplo Maestro en Modo Automático:**
```text
$STATUS,NODE:MAESTRO,MODO:AUTO,ESTADO:V1_R2,T:24,RF:98%,RTT:82ms,BAT:12.6,HORA:18:25:00*4F\r\n
```

### 4.3 Trama de Alarma Inmediata ($ALARM) — Emitida ante incidentes
```text
$ALARM,NODE:MAESTRO,EVENTO:FALLO_RF_12S,CAUSA:TIMEOUT_LATIDO,ACCION:CAMBIO_A_AMBAR,HORA:17:54:58*3B\r\n
```

### 4.4 Comandos desde la App hacia la Controladora (Protegidos por PIN)

> ⚠️ **La lista de abajo estaba INCOMPLETA y con un comando que el Esclavo rechaza.** Se conserva
> tachada y debajo va la censada el 31/08. **Ninguna de las dos es un permiso: nada ha pasado banco.**

```text
 ┌─────────────────────────────────────────────────────────────────────────────┐
 │ (SUPERADO 31/08 - lista de 9 formas, ver la censada mas abajo)              │
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

#### 📋 Censo del 31/08 — lo que el MAESTRO despacha de verdad

**MEDIDO** con `grep` sobre `01_Firmware/Maestro/src/bluetooth.cpp`, rama por rama. Son **15 acciones
distintas** y **17 formas de trama aceptadas**, porque dos de ellas admiten además una entrada sin
PIN (`:168-170`).

| Forma de trama | Línea | Qué hace / qué contesta |
|---|---|---|
| `CMD:FORZAR_ROJO` | `:145` | **Sin PIN.** Rojo total. `$ACK,CMD:FORZAR_ROJO,RESULT:OK` |
| `CMD:SET_MODO:MENU` | `:168` | **Sin PIN.** Alias de la forma con PIN — no mueve luces a verde |
| `CMD:SET_MODO:ALCANCE` | `:168` | **Sin PIN.** Ídem |
| `CMD:PIN:1234:SET_MODO:AUTO` | `:177` | Modo Automático |
| `CMD:PIN:1234:SET_MODO:MANUAL` | `:182` | Modo Manual |
| `CMD:PIN:1234:SET_MODO:AMBAR` | `:187` | Ámbar de seguridad |
| `CMD:PIN:1234:SET_MODO:MENU` | `:191` | **Vuelve al panel.** Sustituye al botón *Cancelar* retirado. Contesta `RESULT:SALIENDO_TODO_ROJO` o `$ERR,…,DESC:YA_VUELVE_AL_MENU` |
| `CMD:PIN:1234:SET_MODO:ALCANCE` | `:212` | Prueba de alcance. `$ERR,…,DESC:EN_MARCHA_PARE_EL_MODO` si hay un modo corriendo |
| `CMD:PIN:1234:SET_MODO:INTELIGENTE` | `:223` | Modo Inteligente (cámaras). Mismo rechazo |
| `CMD:PIN:1234:SET_MODO:DEGRADADO` | `:234` | Modo Degradado. **Contesta el motivo concreto del rechazo**, no un no seco |
| `CMD:PIN:1234:FORZAR_ROJO` | `:253` | Igual que la forma sin PIN |
| `CMD:PIN:1234:MANUAL:CAMBIAR_TURNO` | `:257` | `$ERR,…,DESC:EN_TRANSICION_REINTENTE` si el coordinador no está en reposo |
| `CMD:PIN:1234:TEST_LEDS` | `:271` | `$ACK,…,RESULT:STARTING_6S` |
| `CMD:PIN:1234:SET_TIEMPOS:v,r,d` | `:275` | **El molde de cómo se contesta bien:** un `$ERR` por cada motivo (`FORMATO_INVALIDO`, `EN_MARCHA_PARE_EL_MODO`, `RANGO`) |
| `CMD:PIN:1234:SET_RTC:YYYY-MM-DD,HH:MM:SS` | `:295` | **Cinco ramas**, ver abajo |
| `CMD:PIN:1234:REINICIAR_RELOJ` | `:330` | `CRISTAL_OK_PONGA_LA_HORA` o `$ERR,…,DESC:SIGUE_PARADO_VEA_CONSULTA_RELOJ` |
| `CMD:PIN:1234:DEMANDA` | `:345` | `REGISTRADA`, o `$ERR` `SOLO_EN_MODO_INTELIGENTE` / `REPITA_EN_UNOS_SEGUNDOS` |

Cualquier otra cosa cae en `$ERR,CMD:DESCONOCIDO,DESC:COMANDO_NO_SOPORTADO` (`:363`).

#### 🕐 `SET_RTC` tiene CINCO ramas, y ninguna miente

Un `$ACK` que no depende de lo que devolvió la llamada **es una mentira con formato de éxito**. Este
comando llegó a contestar `RESULT:OK` sin mirar nada; hoy contesta lo que pasó:

| Respuesta | Línea | Significa |
|---|---|---|
| `$ERR,CMD:SET_RTC,DESC:FORMATO_INVALIDO` | `:308`, `:318` | La trama no se pudo leer, o las cifras están fuera de rango |
| `$ERR,CMD:SET_RTC,DESC:SIN_CRISTAL_VEA_CONSULTA_RELOJ` | `:313` | **No hay con qué contar el tiempo.** El reloj no quedó puesto |
| `$ACK,CMD:SET_RTC,RESULT:HORA_PUESTA_SIN_PROPAGAR` | `:325` | La hora entró aquí, pero **no viajó al Esclavo** |
| `$ACK,CMD:SET_RTC,RESULT:OK` | `:327` | La hora entró y va camino del Esclavo |

> **Por qué esto importa en el poste:** con el cristal `Y2` confirmado muerto en hardware (N-17), la
> versión anterior decía que sí y **no ponía la hora** — y el técnico se iba del poste creyendo que
> lo había dejado puesto.

#### 🛑 Y lo que el ESCLAVO acepta, que NO es lo mismo

**MEDIDO** sobre `01_Firmware/Esclavo/src/bluetooth.cpp`:

| Forma de trama | Línea | Qué hace |
|---|---|---|
| `CMD:AMBAR_EMERGENCIA` | `:130` | **Sin PIN.** Ámbar intermitente + latch. `$ACK,…,RESULT:OK` |
| `CMD:PIN:1234:AMBAR_EMERGENCIA` | `:171` | Lo mismo, con PIN. Las dos entradas hacen lo mismo |
| `CMD:PIN:1234:SOLICITAR_PASO` | `:184` | **Pide**, no ordena. `PEDIDO_AL_MAESTRO` o `$ERR,…,DESC:REPITA_EN_UNOS_SEGUNDOS` |
| `CMD:PIN:1234:SET_RTC:…` | `:215` | `OK`, `$ERR,…,DESC:SIN_CRISTAL` o `$ERR,…,DESC:FORMATO_INVALIDO` |
| ~~`CMD:FORZAR_ROJO`~~ | `:157` | 🛑 **RECHAZADO** — `$ERR,CMD:FORZAR_ROJO,DESC:RENOMBRADO_USE_AMBAR_EMERGENCIA` |
| ~~`CMD:PIN:1234:FORZAR_ROJO`~~ | `:176` | 🛑 **RECHAZADO** — mismo `$ERR`. **Las dos formas.** |
| ~~`CMD:PIN:1234:TEST_LEDS`~~ | `:202` | 🛑 **RECHAZADO** — `$ERR,…,DESC:NO_EN_SERVICIO_USE_EL_MAESTRO` |

Cualquier otra cosa: `$ERR,CMD:DESCONOCIDO,DESC:COMANDO_NO_SOPORTADO_EN_ESCLAVO` (`:260`).

> **El rechazo de `FORZAR_ROJO` se hace ENSEÑANDO EL NOMBRE BUENO, no en silencio.** Quien manda ese
> literal es alguien con una app o un manual anteriores al cambio —este manual, hasta hoy—, y lo que
> necesita no es enterarse de que el comando no existe, sino **de cómo se llama ahora**.

### 🛑 El ROJO DE EMERGENCIA no pide PIN, y es deliberado

`mando.cpp` lo dejó escrito hace meses para el mando de relés: **«asimetría deliberada: lo seguro,
fácil; lo peligroso, difícil»**. Detener el tráfico es la acción **segura** —el equipo cae a
todo-rojo—, así que ponerle una clave delante solo retrasa a quien está viendo el incidente.

**En el Maestro** se aceptan **las dos formas**: `CMD:FORZAR_ROJO` y `CMD:PIN:1234:FORZAR_ROJO` hacen
lo mismo. El PIN sigue guardando lo que **abre** paso o mueve luces.

> 🛑 **EN EL ESCLAVO NO — y era el error de pánico de este manual.** Allí las dos formas contestan
> `$ERR,CMD:FORZAR_ROJO,DESC:RENOMBRADO_USE_AMBAR_EMERGENCIA` y **no pasa nada**. La asimetría
> deliberada sigue viva en esa punta, pero **con otro literal**: `CMD:AMBAR_EMERGENCIA`, que también
> se acepta **sin PIN**, por la misma razón —quien está viendo el incidente tiene que poder pedir la
> caída segura aunque no se sepa el PIN—.

### 📋 Qué acepta cada punta, y no es lo mismo — **corregida el 31/08**

| Comando | Maestro | Esclavo | Por qué |
|---|---|---|---|
| `SET_MODO:AUTO` · `MANUAL` · `AMBAR` | ✅ con PIN | ❌ | El Maestro es el único que arbitra el ciclo |
| `SET_MODO:MENU` · `ALCANCE` | ✅ con PIN **y sin PIN** | ❌ | No abren paso: vuelven al panel o arrancan una prueba |
| `SET_MODO:INTELIGENTE` · `DEGRADADO` | ✅ con PIN | ❌ | idem — y el Degradado además pasa su propia puerta |
| `MANUAL:CAMBIAR_TURNO` | ✅ con PIN | ❌ | idem |
| `SET_TIEMPOS:v,r,d` | ✅ con PIN | ❌ | La configuración del ciclo la arbitra el Maestro |
| **`SOLICITAR_PASO`** | — | ✅ **con PIN** | **Pide**, no ordena: manda `CMD_DEMANDA` por radio y decide el Maestro |
| **`DEMANDA`** | ✅ con PIN | — | Equivalente al `SOLICITAR_PASO` del otro extremo, sobre el Maestro |
| ~~`FORZAR_ROJO`~~ **en el Esclavo** | ✅ **sin PIN**, hace rojo | 🛑 **RECHAZADO** — `RENOMBRADO_USE_AMBAR_EMERGENCIA` | Prometía rojo y hacía ámbar. **Se corrigió el nombre, no el comportamiento** |
| **`AMBAR_EMERGENCIA`** | ❌ | ✅ **sin PIN** *(y con PIN)* | **Éste es el botón de pánico del Esclavo.** Dirección segura |
| `SET_RTC:…` | ✅ con PIN, **5 ramas** | ✅ con PIN, 3 ramas | Ajusta el reloj, no las luces |
| `REINICIAR_RELOJ` | ✅ con PIN | ❌ | Diagnóstico del cristal, solo donde se pone la hora |
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
