# 📡 Guía Definitiva y Auditoría de Radios EBYTE E90-DTU

**Ubicación:** `04_Manuales/MANUAL_EXACTO_RADIOS_E90_DTU.md`
**Referencia exacta de hardware:** EBYTE serie **E90-DTU (RS485/232 $\leftrightarrow$ RF)**
**Herramienta de configuración:** `04_Manuales/RF_Setting4.6.exe`
**Revisado:** 1 de agosto de 2026
**Última revisión:** **5 de septiembre de 2026** — 🛑 **la §6 («Diagnóstico desde la pantalla»)
queda DEROGADA: no hay pantalla, y en el Maestro el dato tampoco sale por Bluetooth.** El resto
del documento —configuración, DIP `M0`/`M1`, topología, bandas— **sigue vigente sin cambios**.

---

> ## 🛑 AVISO DE ESTADO (05/09/2026) — LO QUE CAMBIA Y LO QUE NO
>
> | | |
> |---|---|
> | ✅ **§1 a §5, §7: VIGENTES** | La configuración de las radios no la toca esta revisión. `2.4 kbps`, canal `0`, `M0`/`M1` en `OFF`, 2 radios en enlace directo |
> | 🛑 **§6: DEROGADA** | Mandaba diagnosticar leyendo `PRUEBA ALCANCE` en la pantalla del Maestro. **La pantalla no se monta** (28/08, confirmado el 05/09), y ese modo **para el cruce en rojo fijo** a cambio de nada |
> | 📌 **Y deja un pendiente de FIRMWARE, no de documento** | Los contadores `RX:`/`OK:`/`RUIDO:` **sí salen del Esclavo** por Bluetooth y **no salen del Maestro**. Ver el desarrollo medido en §6 |

---

> ## ✅ CONFIGURACIÓN QUE OPERA HOY (01/08/2026)
>
> **2 radios en enlace directo, sin repetidor.** Confirmado por el funcional.
>
> | Parámetro | Valor vigente |
> |---|---|
> | Radios en servicio | **2** — una en el Maestro, una en el Esclavo |
> | Topología | **Enlace directo.** El repetidor ESP32 **no está en uso** |
> | Air Data Rate | **`2.4 kbps`** en ambas |
> | Canal | **`0`** (170,0 MHz) en ambas |
> | Potencia | `30 dBm` (1 W) |
> | FEC | `Enable` |
> | Modo de transmisión | `Transparent` |
> | **DIP `M0` / `M1` en operación** | **AMBOS en `OFF`** |
>
> El diagnóstico del 31/07 resultó correcto: **la radio B1 estaba averiada** y era la causa
> del fallo, no el firmware. Retirada del montaje, el sistema opera con dos radios.
>
> **En curso:** conseguir antenas VHF de la banda correcta para recuperar alcance.

---

> ## ⚠️ CAMBIO OBLIGATORIO — Air Data Rate `0.3 kbps` → `2.4 kbps`
>
> **Debe aplicarse a TODAS las radios del enlace antes de cualquier prueba de campo.** Hoy
> son **dos**; si algún día vuelve el repetidor, son **cuatro**. Todas las radios del enlace
> deben quedar con **el mismo** Air Data Rate o no se comunicarán entre sí.
>
> A `0.3 kbps` cada trama de 4 bytes ocupa ~0,75 s de aire por salto (FEC incluido). En modo
> repetidor son 2 saltos por sentido, luego el ciclo orden→confirmación (`GO_GREEN` →
> `ACK_GREEN`) rondaba los **3,0–3,2 s** y desbordaba el plazo de espera del Maestro. El
> reintento colisionaba con la confirmación entrante en el bus half-duplex y el sistema caía
> a `C_FALLO`.
>
> **Síntoma en campo:** caída de comunicación siempre en el mismo punto —cuando el Esclavo
> va a pasar a verde— en modo Automático, Inteligente y Manual; y **repetidor que no
> funciona nunca**.
>
> A `2.4 kbps` el mismo ciclo baja a ~0,35 s. Coste: ~6 dB de sensibilidad, despreciable con
> 1 W sobre una obra de 20–500 m frente a los 6 km de alcance especificado.

### 🔧 Constantes de firmware que acompañan a este ajuste — **valores vigentes**

> **Corrección de este manual (01/08/2026).** La versión anterior afirmaba que junto al
> cambio de tasa iban *"ráfaga reducida de 3× a 1× y `TIMEOUT_ACK_MS` elevado de 3500 a
> 8000 ms"*. **Eso ya no es cierto: ambos se revirtieron en V8.1** tras la validación de
> campo, y el manual se quedó describiendo un estado intermedio que ya no existe.

| Constante | Valor en firmware hoy | Dónde | Por qué ese valor |
|---|---|---|---|
| `RF_BURST_COPIES` | **3 copias** | `protocolo.h` (Maestro **y** Esclavo) | A 2,4 kbps cuestan ~0,13 s de aire, despreciable. Los equipos son **móviles** y la distancia de despliegue es desconocida: a mayor distancia sube la pérdida de tramas, y la redundancia es la palanca que paga |
| `TIMEOUT_ACK_MS` | **3.500 ms** | `coordinador.cpp` | Con 3,5 s caben **4 intentos** dentro de la ventana de seguridad de 12 s; con 8 s solo caben 2 |

> ⚠️ **`RF_BURST_COPIES` DEBE ser idéntico en Maestro y Esclavo.** Están definidos en dos
> archivos distintos que hay que mantener a mano en sincronía.
>
> A `0.3 kbps` esas mismas 3 copias costaban ~2,2 s y saturaban el canal half-duplex. **El
> problema nunca fue la redundancia en sí, sino su coste a una tasa aérea absurdamente
> baja.** Se dice así porque la conclusión contraria —"quitar la ráfaga"— fue la reacción
> inicial y era la equivocada.

---

## 🔍 1. Diagnóstico: por qué hubo tantas confusiones previas

Cuatro razones generaban confusión entre los manuales y el personal de campo:

1. **Confundir el módulo suelto (TTL) con la unidad DTU (caja industrial).**
   En tutoriales genéricos se habla de pines `DI`, `RO`, `DE`, `RE` de chips MAX485 o de
   módulos de radio sueltos. **La realidad del hardware:** el equipo usado es un
   **E90-DTU** en caja metálica industrial. Internamente **ya trae el chip MAX485 y el
   circuito de autoconmutación**, por lo que **solo expone la bornera verde `485_A`,
   `485_B`, `V+` y `V-`**.

2. **Inversión de cables RS485 (el falso mito de cruzar TX/RX).**
   El personal intentaba "cruzar" los cables pensando en RS232. **En RS485 NO SE CRUZAN.**
   El cableado es estrictamente paralelo: **`A` con `485_A`** y **`B` con `485_B`**.

3. **Los DIP switches `M0` y `M1`.** En los módulos sueltos hay que usar cables o jumpers;
   en la caja E90-DTU hay **DIP switches físicos en la parte inferior**. Sin ponerlos en la
   posición de configuración, `RF_Setting4.6.exe` no puede leer la radio.

   > 🚨 **Este punto contenía un dato FALSO hasta esta revisión.** Decía que la posición de
   > configuración era **`ON/ON`**. **No lo es** — es **`M0=ON`, `M1=OFF`** (ver §3). El
   > dato erróneo sobrevivía aquí aunque la tabla del §3 ya estaba corregida, y **un manual
   > que se contradice a sí mismo es peor que uno equivocado de forma consistente**: el
   > técnico elige la parte que lee primero.

4. **Creer que el firmware distingue la topología.** No la distingue. Ver §5.

---

## 🛠️ 2. Guía paso a paso: configurar el E90-DTU con `RF_Setting4.6.exe`

### PASO 1: poner la radio en modo configuración

1. Desconecte la alimentación de la radio E90-DTU.
2. En la parte inferior de la caja metálica, ubique los **DIP switches `M0` y `M1`**.
3. Mueva **`M0` a `ON` y deje `M1` en `OFF`**. *(Modo 1 / WOR — es también el modo de
   configuración por PC según el datasheet del fabricante.)*
4. Conecte la radio al PC con un convertidor **USB a RS485** (`A` con `485_A`, `B` con
   `485_B`) o un cable DB9 RS232.
5. Alimente la radio con 12 V DC (`V+` y `V-`).

### PASO 2: abrir `RF_Setting4.6.exe` y cargar parámetros

1. Ejecute `04_Manuales/RF_Setting4.6.exe`.
2. Seleccione el puerto **COM** del adaptador y BaudRate **`9600`**. Clic en **`Open Port`**.
3. Clic en **`Read Option`**. Los campos se rellenan con los datos de la radio.
4. Ajuste los parámetros:

| Parámetro | Valor | Nota |
|---|---|---|
| **Baud Rate (UART)** | `9600 bps`, `8N1` | |
| **Air Data Rate** | **`2.4 kbps`** | ⚠️ **Obligatorio.** Sustituye al antiguo `0.3 kbps`. Idéntico en todas las radios del enlace |
| **Transmit Power** | `30 dBm` (1 W) | Máxima potencia |
| **FEC** | `Enable` | |
| **Transmission Mode** | `Transparent` | Desactivar Fixed-Point |
| **Channel** | Ver §5 | **Hoy: canal `0` en las dos radios** |

5. Clic en **`Write Option`** para guardar en la memoria no volátil.
6. Cierre el programa y desconecte la radio del PC.

### PASO 3: poner la radio en modo operación normal

1. Desconecte la alimentación.
2. Mueva **`M0` y `M1` a `OFF`** (modo 0, transmisión normal).
3. Conecte `485_A` y `485_B` a las borneras `A` y `B` de la tarjeta del semáforo.
4. Alimente semáforo y radio. El LED `PWR` debe encender en rojo y los LEDs `TXD`/`RXD`
   destellarán al transmitir.

---

## 📌 3. Tabla de DIP switches en el E90-DTU

> ## 🚨 ERROR CORREGIDO — LEER ANTES DE TOCAR NADA
>
> Una versión anterior de este manual indicaba que el **modo de configuración** era
> `M0=ON, M1=ON`. **Era incorrecto.** El datasheet del fabricante indica `M0=ON, M1=OFF`.
>
> El 31/07/2026 esa tabla llevó a dejar **las cuatro radios en ON/ON**, un modo especial
> donde la transmisión puede quedar deshabilitada mientras la recepción sigue activa.
> Resultado: **radios que oían pero no contestaban**, y un día entero persiguiendo un fallo
> de comunicación inexistente.
>
> **Regla que no falla: en operación, `M0` y `M1` van SIEMPRE los dos en `OFF`.**

| Modo de trabajo | `M0` | `M1` | Función |
|---|---|---|---|
| **Modo 0: transmisión normal** | **OFF (0)** | **OFF (0)** | ✅ **SIEMPRE en operación.** Único modo válido con la radio conectada a la tarjeta |
| Modo 1: Wake-up (WOR) | ON (1) | OFF (0) | Preámbulo de activación. **También es el modo de configuración por PC** |
| Modo 2: Power Saving (WOR) | OFF (0) | ON (1) | Bajo consumo. ⚠️ La transmisión puede quedar deshabilitada |
| Modo 3: reposo | ON (1) | ON (1) | ⚠️ **NO usar en operación.** Modo especial, no transparente |

> ⚠️ **Si una radio oye pero no contesta, lo primero que hay que mirar son estos dos
> switches.** Es la causa más frecuente de un enlace que funciona en un solo sentido.
>
> Y hay un segundo sospechoso, con síntoma casi idéntico: **la propia radio averiada**. Fue
> lo que ocurrió con B1 el 31/07. Se distinguen porque los switches se ven, y una radio
> averiada solo se descarta **sustituyéndola**. Antes de rediseñar nada, cambie la radio.

---

## 🔄 4. Lo que el enlace transporta — y por qué el manual no lo limita

El firmware envía **paquetes de 4 bytes**: `msgID`, `command`, `param`, `crc` (CRC-8 Maxim,
polinomio `0x31`). La radio va en modo **transparente**: no interpreta nada, solo mueve
bytes. **Ningún ajuste de radio depende del comando que viaje dentro.**

### 4.1 Los comandos nuevos `0x07`–`0x0F` (SFTY-23) no exigen tocar las radios

SFTY-23 añadió la sincronización horaria y de configuración de ciclo por radio, ocupando el
rango de comandos libre a partir de `0x07`:

| Rango | Para qué |
|---|---|
| `0x01`–`0x06` | Ciclo semafórico: `GO_GREEN`, `GO_RED`, `ACK_GREEN`, `PING`, `PONG`, `ACK_RED` |
| `0x07`–`0x0A` | Hora (`H`, `M`, `S` y su confirmación). El Esclavo **aplica las tres juntas** al recibir la de segundos: nunca queda una hora a medias |
| `0x0B`–`0x0C` | Medición de desfase entre relojes y su respuesta |
| `0x0D`–`0x0F` | Configuración del ciclo degradado (verde, despeje) y su confirmación |

> ### ✅ Verificado: el puente ESP32 valida **formato y CRC, no comandos**
>
> Es la pregunta que motivó esta revisión, porque un puente que filtrara por comando habría
> **bloqueado en silencio** todas las tramas nuevas de SFTY-23, y el síntoma habría sido
> "el reloj no sincroniza" sin ninguna pista de por qué.
>
> **No ocurre.** `01_Firmware/Repetidor/src/main.cpp` acumula 4 bytes y compara
> `crc8Maxim(buf, 3)` contra `buf[3]`. **No mira `buf[1]`, que es el byte de comando.** Las
> tramas `0x07`–`0x0F` atraviesan el ESP32 sin ninguna modificación del puente.
>
> **Este manual no contiene —y no debe contener— ninguna lista de comandos permitidos.** Si
> alguna vez aparece una, será un error: la radio es transparente y el puente es agnóstico
> al comando por diseño.

### 4.2 Lo que sí importa a la radio

| Sí depende de la radio | No depende de la radio |
|---|---|
| Tasa aérea, canal, potencia, FEC | Qué comando viaja dentro de la trama |
| Tiempo de aire por trama → plazos de espera | El número de comandos definidos |
| Modo transparente vs. fixed-point | El significado del byte `param` |

> ℹ️ **La E90-DTU no entrega RSSI en modo transparente**, así que la pantalla no puede
> mostrar potencia en dBm. Activar la salida de RSSI añadiría un byte a cada paquete
> recibido y **rompería el parser de tramas de 4 bytes**. No se ha hecho, y no debe hacerse
> sin tocar antes el parser.

---

## 📻 5. Topología: enlace directo (hoy) y repetidor (no en uso)

> **El firmware de las STM32 es agnóstico a la topología.** No hay que recompilar ni
> reflashear al pasar de 2 a 4 radios ni al revés. **Lo único que cambia es el CANAL de las
> radios** (y, con repetidor, que haya un ESP32 flasheado en medio).

| Equipo | **Modo directo (2 radios) — VIGENTE** | Modo repetidor (4 radios) — no en uso |
|---|---|---|
| Radio del **Maestro** | Canal `0` — 170,0 MHz | Canal `0` — 170,0 MHz |
| Radio **B1** (entrada del repetidor) | — no se usa | Canal `0` — 170,0 MHz |
| Radio **B2** (salida del repetidor) | — no se usa | Canal `10` — 172,0 MHz |
| Radio del **Esclavo** | **Canal `0` — 170,0 MHz** | Canal `10` — 172,0 MHz |

**Por qué dos canales con repetidor:** el repetidor escucha en una frecuencia y retransmite
en otra. Si las cuatro radios estuvieran en el mismo canal, el repetidor se oiría a sí mismo.

> ### ⚠️ Al RETIRAR el repetidor (pasar de 4 radios a 2)
>
> **No basta con desconectar el ESP32.** Si se retira sin más, el Maestro queda en canal `0`
> y el Esclavo en canal `10`: **frecuencias distintas, no se comunican.** Ambos entran en
> Ámbar Intermitente y parece un fallo de firmware.
>
> - [ ] Reconfigurar la radio del **Esclavo** de canal `10` a canal `0`.
> - [ ] Verificar que ambas quedan en `2.4 kbps`, `30 dBm` y `FEC: Enable`.
> - [ ] Confirmar `M0` y `M1` **en OFF** en las dos.

**Al INSTALAR el repetidor (de 2 a 4):** Esclavo de canal `0` a `10`; B1 en `0` y B2 en
`10`; flashear el ESP32 con `01_Firmware/Repetidor` (ver `05_Funcional/5_Manual_Puente_ESP32.md`).

> ### 🧪 El camino con repetidor NO está validado sobre hardware (N-11)
>
> El sistema opera hoy con dos radios, así que **`SFTY-16` (puente que valida antes de
> retransmitir) y `SFTY-17` (retardo de cortesía del Esclavo, 200 ms) nunca se han
> ejercitado sobre hardware real** — se diseñaron precisamente para el camino de 4 radios
> que ahora no está en uso. Están verificados **en simulación**, que no es lo mismo.
>
> Al reintroducir el repetidor hay que **probarlos de nuevo, no darlos por buenos**.
>
> `SFTY-17` es inofensivo en enlace directo: el Maestro espera hasta 3.500 ms.
> El **Repetidor ESP32 sigue sin watchdog**, a diferencia de las dos STM32.

---

## 🧰 6. ~~Diagnóstico desde la pantalla, sin instrumentos~~ 🛑 **DEROGADO — NO HAY PANTALLA**

> # 🛑 NO SUBA AL POSTE A LEER ESTO. LA PANTALLA NO SE MONTA (05/09/2026)
>
> **Esta sección mandaba al técnico a leer contadores en `PRUEBA ALCANCE` del Maestro, y decía
> que *«la distinción vale un viaje»*. Hoy el viaje se hace y no se lee nada.**
>
> El módulo ST7920 **no se monta en ninguna punta** (decidido el 28/08, confirmado el 05/09), y
> el objeto U8g2 tiene **los cuatro pines en `U8X8_PIN_NONE`**: no queda ni una escritura de
> pin. El código sigue compilando —`D-6`, 271 comprobaciones— pero **no hay cristal donde
> mirar**.
>
> ## 🔴 Y en el MAESTRO no es sólo que no se vea: es que el dato NO SALE POR NINGÚN SITIO
>
> Es peor que una pantalla ausente, y por eso se escribe como **pendiente de firmware** en vez
> de taparse con prosa:
>
> 1. **Entrar a `PRUEBA ALCANCE` PARA EL CRUCE.** `modoAlcance_setup()` llama a
>    `coordinador_forzarMenu()`: **rojo fijo** en las dos puntas con enlace, ámbar intermitente
>    sin él. El cruce deja de ciclar mientras dure la prueba.
> 2. **Y su único consumidor es `lcd_dibujarAlcance()`**, que pinta sobre un framebuffer
>    invisible. El operario **para el cruce y no recibe nada**.
> 3. **El contador que esta tabla usa para el diagnóstico —el de basura— no lo lee NADIE en el
>    Maestro.** Es el que separa *«no llega nada»* de *«llega basura»*, o sea justo lo que
>    *«vale un viaje»*:
>    ```
>    $ grep -rn "protocolo_tramasDescartadas" Maestro/src/
>    Maestro/src/protocolo.cpp:162:unsigned long protocolo_tramasDescartadas() { return cntDescartadas; }
>    ```
>    **Una sola línea: la definición. Cero llamadores.** Se incrementa en cada trama descartada
>    y nadie lo lee jamás. El banco ya lo tiene anotado como huérfana **del Maestro**, con su
>    motivo escrito: *«sigue huérfana en el MAESTRO, donde nadie los publica todavía»*
>    (`01_Firmware/Simulaciones/banco/packs/costura_10_funciones_muertas.py`).
>
> ## ✅ EN EL ESCLAVO SÍ SALE — y esa asimetría es la prueba de que es un hueco, no un diseño
>
> Los **tres** contadores viajan por Bluetooth en el `$ALARM` del Esclavo:
>
> ```
> snprintf(tramo, sizeof(tramo), "RX:%lu,OK:%lu,RUIDO:%lu",
>          protocolo_bytesRecibidos(), protocolo_tramasValidas(),
>          protocolo_tramasDescartadas());          // Esclavo/src/bluetooth.cpp
> ```
>
> **La API es idéntica en las dos puntas** —`protocolo_bytesRecibidos`,
> `protocolo_tramasValidas` y `protocolo_tramasDescartadas` están declaradas igual en los dos
> `protocolo.h`—. **Misma capacidad, publicada en una punta y encerrada en la otra.**
>
> ## 📌 PENDIENTE DE FIRMWARE, escrito como tal
>
> **Publicar `RX:` / `OK:` / `RUIDO:` del Maestro por Bluetooth, como ya hace el Esclavo.**
> Mientras eso no exista, el diagnóstico de tres estados de SFTY-15 **sólo se puede hacer sobre
> el Esclavo**, desde la app. No hay forma de hacerlo sobre el Maestro, con pantalla o sin ella.
>
> ✅ **Lo que SÍ se puede leer hoy del Maestro, desde la app**, y no es lo mismo: `RF:` (calidad
> de enlace en %), `RTT:` (tiempo de respuesta) y `SINRESP:` (latidos sin respuesta), en el
> `$STATUS` periódico y en el `$ALARM`. **Sirven para medir alcance caminando; NO separan
> *«nada llega»* de *«llega basura»***, que es la distinción que valía el viaje.
>
> ⚠️ **Y una advertencia de método, que este manual ya se aplicó a sí mismo en §7:** *«la
> E90-DTU no entrega RSSI en modo transparente»*. Un porcentaje de calidad **no es potencia de
> señal**: sale de contar latidos contestados. Sigue siendo la mejor medida que hay, y sigue sin
> ser un RSSI.

> ### 📕 HISTÓRICO — el texto derogado, se conserva y no se borra
>
> Se conserva porque **la clasificación de tres estados sigue siendo correcta y sigue siendo la
> que hay que publicar**: lo que caducó es *dónde se lee*, no *qué significa*.

~~Antes de subir al poste con portátil y destornillador, la pantalla del equipo ya separa tres
fallos que antes se veían todos como "no hay comunicación" (SFTY-15). Se lee en la fila
inferior de `PRUEBA ALCANCE` en el Maestro y de `ESTADO` en el Esclavo:~~

| En pantalla | Qué significa | Dónde mirar |
|---|---|---|
| `RX 0 - nada llega` | No entra ni un byte | **Radio:** cobertura, canal distinto, antena, `M0`/`M1`, radio averiada |
| `RX 4k - BASURA` | Entran bytes pero ninguna trama válida | **Cableado:** par RS485 flotando, `A`/`B` invertidos, radio atascada transmitiendo |
| `RX 36  9 tr` | Bytes y tramas válidas | Enlace correcto |

**La distinción vale un viaje.** `RX 0` manda a revisar la radio; `RX ... BASURA` manda a
revisar el cable. Son dos herramientas distintas y dos diagnósticos opuestos.
→ ✅ **ESTO SIGUE SIENDO CIERTO. Lo que caducó es dónde se lee**: hoy sólo llega del **Esclavo**,
por Bluetooth, en su `$ALARM` (`RX:`/`OK:`/`RUIDO:`). Del Maestro **no llega**.

~~La pantalla `PRUEBA ALCANCE` del Maestro añade calidad de enlace en %, tiempo de respuesta y
fallos consecutivos, para **medir** hasta dónde llega la cobertura caminando con el equipo,
en vez de estimarla a ojo.~~ 🛑 **DEROGADO: esa pantalla no se ve, y entrar en ella PARA EL
CRUCE en rojo fijo.** Para medir alcance caminando, use `RF:` y `RTT:` del `$STATUS` desde la
app. Ver `02_LCD/MANUAL_PANTALLA_LCD.md` §5.6, que lleva la medida completa.

---

## 📁 7. Inventario de archivos en `04_Manuales/`

| Archivo | Qué es |
|---|---|
| `MANUAL_EXACTO_RADIOS_E90_DTU.md` | Este documento |
| **`MANUAL_MANDO_4_RELES.md`** | ~~**Mando de 4 relés** — la interfaz que se opera desde el suelo sin ver la pantalla~~ 🛑 **el mando NO SE MONTA (`D-1`, 05/09): el equipo se opera SÓLO POR APP.** El **código** sigue vivo y sigue leyendo `PB9`/`PB13`; el receptor nunca se compró. Lea su cabecera de estado antes que el cuerpo |
| `RF_Setting4.6.exe` | Software oficial de configuración para PC |
| `XCOM V2.6.exe` | Terminal serie/RS485 para monitorear tramas binarias desde un PC |
| `E90-DTU(230SL37)_UserManual_EN_V1.5_fr.pdf` | Datasheet de la variante **230 MHz, 37 dBm** |
| `E90-DTU(433C17)_UserManual_EN_v1.4.pdf` | Datasheet de la variante **433 MHz, 17 dBm** |
| `Manual_de_Senalizacion_Vial.pdf` | Manual oficial del Ministerio de Transporte de Colombia (2024) |
| `校验文件(Hash).exe` · `CRC32 804438E0.txt` | Utilidad de hash del fabricante y su suma. **De procedencia del proveedor, no auditada.** No se necesita para configurar las radios |

> ### ⚠️ Ninguno de los dos datasheets corresponde exactamente a la banda declarada
>
> Los PDF de esta carpeta son de las variantes de **230 MHz** y **433 MHz**, pero toda la
> documentación del proyecto afirma que el enlace opera en **170,0 MHz** (canal `0`) y
> **172,0 MHz** (canal `10`). **No se ha podido resolver esta discrepancia leyendo los
> documentos**, y no debe resolverse a ojo: los rangos de canal, el paso de frecuencia y la
> potencia máxima **son distintos entre variantes**.
>
> **Antes de configurar una radio nueva, lea la etiqueta del equipo físico** y consiga el
> datasheet de esa referencia exacta. Configurar guiándose por el PDF equivocado puede dejar
> la radio en una frecuencia distinta de la que se cree, con el síntoma de siempre: dos
> equipos en ámbar y un fallo de comunicación que parece de firmware.

---

## ❓ 8. Preguntas abiertas

1. **¿Qué variante de E90-DTU está instalada realmente?** Los datasheets guardados aquí son
   de **230 MHz** y **433 MHz**, pero la documentación declara operación en **170/172 MHz**.
   Hasta que alguien lea la etiqueta del equipo físico, **la equivalencia canal↔frecuencia
   de §5 no está verificada contra el fabricante**: se arrastra de la documentación previa
   del proyecto. `MANUAL_HARDWARE.md` (raíz) cita las mismas frecuencias pero **sin
   mencionar números de canal**, así que ni siquiera se contradicen: no se cruzan.
2. **Antenas.** Está en curso conseguir **antenas VHF de la banda correcta**. Mientras tanto,
   cualquier medida de alcance tomada con las antenas actuales **es un piso, no el alcance
   del sistema**.
3. **El PDF de 433 MHz** en esta carpeta puede inducir a error si alguien configura guiándose
   por él. ¿Se archiva en `99_Legacy/`?
