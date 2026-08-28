# 🚦 Manual de Operación y Comportamiento del Sistema (V9.0 Definitiva)

Este manual define el **"Ground Truth"** (la verdad absoluta) de cómo DEBE comportarse el sistema, sirviendo como base para validar que las simulaciones y el código cumplan con la especificación.
Todas las operaciones están alineadas al **Manual de Señalización Vial de Colombia (Resolución 2024 - MinTransporte)**.

---

> ## 🔴 CAMBIO DEL 28/08/2026 — SE RETIRA LA PANTALLA LCD
>
> **El equipo va montado en alto y la pantalla no se lee desde el suelo.** Una LCD de 128×64 dentro
> del gabinete, a 5 m, no la mira nadie: para consultarla hacía falta escalera o canasta, que es
> justo lo que el operario no tiene delante cuando hace falta.
>
> **La interfaz de operación pasa a ser la app por Bluetooth.** El módulo entra por el conector
> **`J17`**, en los mismos pines que dejó libre la pantalla.
>
> ### Consecuencia operativa: YA NO HAY MENÚ LOCAL
>
> Todo lo que este manual describía como *«en la pantalla»*, *«4 = Menú»*, *«`CONFIGURACION` →
> `AJUSTAR HORA`»* o *«cuarta opción del Menú Principal»* **deja de estar disponible en el equipo**
> y se hace **desde la app**. Los párrafos afectados **no se han borrado**: se han marcado, con lo
> que los sustituye al lado. Borrarlos en silencio dejaría al operario buscando en la app una
> función que aquí figuraba y que quizá aún no exista allí.
>
> ### ⚠️ LOS BOTONES SIGUEN AHÍ, Y EL MENÚ TAMBIÉN — SOLO QUE A CIEGAS
>
> **Medido sobre el firmware el 28/08:** el menú **no se ha retirado del binario**. `lcd.cpp`,
> `menu.cpp` y `modo_hora.cpp` siguen compilándose, y los cuatro botones del conector `J16` siguen
> navegándolo. Lo único que falta es el display donde se vería el resultado.
>
> **Eso significa que pulsar los botones —o accionar el mando de relés, que va cableado en paralelo
> con ellos— sigue moviendo un menú que nadie ve.** Con los pulsos suficientes se llega a
> `AJUSTAR HORA` y **se confirma una hora cualquiera que el equipo dará por válida**, sin ningún
> aviso. **No accione los botones del gabinete «a ver qué pasa».** Toda la operación va por la app.

---

## 1. Comportamiento Físico de las Luces (Secuencia Normativa Colombia)

Para evitar arranques prematuros y dar tiempo de frenado, la secuencia lumínica **debe** operar de la siguiente manera:

1. 🔴 **ROJO FIJO:** Vía cerrada.
2. 🟡 **AMARILLO FIJO:** (Duración estricta de 4.0 segundos avisando el arranque inminente en Maestro y Esclavo).
3. 🟢 **VERDE FIJO:** Vía libre.
4. 🔴 **ROJO FIJO:** (Transición directa desde el verde, 0s de aviso). Vía cerrada.

### Tiempos de Despeje (All-Red / Rojo Estático)
Cuando se solicita el cambio de vía, el sistema debe entrar en un estado de **ROJO ABSOLUTO**.
- Durante *N* segundos, **ambos semáforos estarán en ROJO**.
- **Variabilidad de Terreno:** Como la obra puede abarcar de 20m a 500m (con radios que alcanzan hasta 6km en línea vista), el tiempo de despeje no puede estar limitado a un valor bajo.
- **Configuración:** ~~La interfaz del menú LCD permite configurar tiempos de despeje de 5 a 999 segundos (piso mínimo de 5s por seguridad vial, hasta 16.6 minutos) para dar cobertura total a puentes largos o túneles de 500m.~~
  > 🔴 **CORREGIDO EL 28/08 — este párrafo daba un rango que la app NO puede pedir.** Sin pantalla,
  > los tiempos se fijan con `CMD:PIN:...:SET_TIEMPOS:<verdeMin>,<rojoMin>,<despejeSeg>`, y los
  > límites duros los impone el firmware, no la interfaz. **Medido en
  > `Maestro/src/modo_automatico.cpp` líneas 30–32:**
  >
  > | | mínimo | máximo |
  > |---|---|---|
  > | Verde | 1 min | 15 min |
  > | Rojo | 1 min | 15 min |
  > | **Despeje (todo-rojo)** | **10 s** | **90 s** |
  >
  > Fuera de rango, el equipo responde `$ERR,CMD:SET_TIEMPOS,DESC:RANGO` y **no cambia nada**.
  > Con el ciclo corriendo responde `$ERR,CMD:SET_TIEMPOS,DESC:EN_MARCHA_PARE_EL_MODO`, porque bajar
  > un tiempo a mitad de fase acortaría un todo-rojo ya empezado.
  >
  > **De dónde salía el «5 a 999 s»:** de la pantalla `CONFIG_TIEMPOS` del menú local, que sí subía
  > hasta 999 (`modo_automatico.cpp` línea 118). **Ese camino ya no tiene display.** El mínimo real
  > tampoco es 5 s sino **10 s**, y el propio fuente dice por qué: *«10 s es lo que tarda en
  > despejarse el tramo más corto que esta casa ha montado»*.
  >
  > ⚠️ **Consecuencia que hay que decir en voz alta:** los túneles y puentes de 500 m que este
  > párrafo prometía cubrir **con 90 s de despeje pueden no quedar cubiertos**. Si una obra los
  > necesita, es un cambio de firmware (subir `DESPEJE_SEG_MAX`), no un ajuste de configuración.

---

## 2. Comportamiento en Destello / Intermitente (Bajo Flujo)

Según el Manual de Señalización (2024), si el flujo vehicular baja al 50% o menos durante 4 horas o más (usualmente operación nocturna), el sistema debe pasar a operación intermitente.
- **Funcionamiento:** Un semáforo parpadea en 🟡 **Ámbar** (Precaución - vía principal) y el otro en 🔴 **Rojo** (Pare - vía secundaria), o ambos en Rojo Intermitente para pasos de igual jerarquía.

---

## 3. Comportamiento de la Interfaz (app por Bluetooth) — 🔴 REESCRITO EL 28/08

**La pantalla LCD ST7920 se retira.** Lo que sigue describe la interfaz vigente; debajo queda lo que
decía antes, tachado, para que se vea qué se perdió y qué lo sustituye.

- **Regla de Oro (Independencia de Red) — SIGUE VIGENTE, cambia la vía.** El operario DEBE poder
  operar el equipo **incluso si las radios están apagadas o no hay comunicación con el Esclavo**. Eso
  se conserva: **el Bluetooth es un enlace corto e independiente de la radio de largo alcance**, así
  que la app entra igual con las radios muertas. Lo que cambia es que el operario entra **desde el
  piso, con el celular**, en vez de subir al gabinete.
- **Comportamiento durante la configuración:** con el equipo parado (sin modo arrancado), ambos
  semáforos se mantienen en **🔴 ROJO FIJO continuo** si hay comunicación, y pasan a **🟡 Amarillo
  Intermitente** si no la hay. **Eso no lo decidía la pantalla, lo decide el coordinador**, y no ha
  cambiado.
- **Arranque:** al ordenar un modo desde la app (`CMD:PIN:1234:SET_MODO:AUTO` / `:MANUAL` /
  `:AMBAR`), el sistema aplica el Despeje All-Red en ambos extremos antes de abrir ningún carril.

### 🛑 Lo que se pierde y NO tiene sustituto todavía

~~**Prueba de Alcance.** Cuarta opción del Menú Principal. Muestra calidad de enlace en %, barra
gráfica, tiempo de respuesta en ms y fallos consecutivos, actualizándose cada 3 segundos.~~

**Medido sobre el firmware el 28/08:** la pantalla `PRUEBA ALCANCE` **sigue existiendo en el
binario** (`modo_alcance.cpp`) y ya no tiene dónde dibujarse. De lo que mostraba:

| dato | ¿sobrevive? | dónde |
|---|---|---|
| Calidad de enlace en % | ✅ sí | campo `RF:` de la trama `$STATUS`, cada segundo |
| Tiempo de respuesta en ms | ✅ sí | campo `RTT:` de `$STATUS` |
| Barra gráfica y fallos consecutivos | ❌ no | eran dibujo de pantalla |
| **Contadores de línea RS-485** (`RX 0 - nada llega` · `RX 4k - BASURA` · `RX 36 9 tr`) | ❌ **NO** | **no viajan en `$STATUS`** |

> ⚠️ **La pérdida de los contadores de línea no es cosmética.** Eran lo que distinguía *«no hay
> cobertura»* de *«el cable RS-485 está suelto o la radio emite basura»* — dos averías que se ven
> igual desde lejos y se arreglan de forma distinta. Hasta que ese dato llegue a la app, esa
> distinción hay que hacerla con instrumentos en el poste. **Se anota como hueco abierto en lugar de
> darlo por trasladado.**

> 🔴 **Y un aviso sobre el `RF:` del ESCLAVO, medido el 28/08 en
> `Esclavo/src/bluetooth.cpp` línea 215:** la trama del Esclavo emite **`RF:98%` y `RTT:85ms` como
> texto literal**, no como medida. Son constantes escritas dentro del `snprintf`. **El Esclavo no
> está midiendo su enlace: está afirmando un 98 % pase lo que pase**, incluso con la radio
> desconectada. **No use ese número para decidir nada.** El `RF:` del **Maestro** sí sale de la
> telemetría real del latido de 3 s (SFTY-14).

---

## 4. Comportamiento ante Fallas (Fail-Safe & Self-Healing Real)

1. **Pérdida de Comunicación (SFTY-6):** Si se pierde comunicación por más de 12.0 segundos, el sistema entra automáticamente en `C_FALLO` / `S_FALLO` (🟡 **Amarillo Intermitente**). En `C_FALLO`, el Maestro envía `CMD_GO_RED` para obligar al Esclavo a pasar a Rojo o Amarillo Intermitente por timeout.
2. **Auto-Recuperación Autónoma (Self-Healing Real):** Al restablecerse la señal de radio, el sistema **NO requiere reinicio manual**. Limpia automáticamente el registro de duplicados (`protocolo_resetReplayProtection()`), fuerza Rojo Estático (All-Red) de 15 segundos en ambos semáforos para limpiar la vía y reanuda el ciclo lumínico sin intervención técnica.
3. **Cuelgue de Procesador (Ruido EMI):** El Watchdog interno (`IWatchdog` activo a 4.0s) reinicia el procesador ante interferencias severas.

---

## 5. Resiliencia RF: Ráfaga configurable y Ventana Deslizante (SFTY-11)

Para garantizar comunicación inquebrantable en zonas de montaña con alta interferencia:
- **Ráfaga (Burst):** 1 copia de 4 bytes con FEC activo en radios E90-DTU.
- **Ventana Deslizante (Sliding Window):** Procesamiento asíncrono con CRC-8 Maxim (`0x31`).
- **Protección Antirepetida (Replay Protection):** Descarte de duplicados mediante `msgID`.

---

## 6. Integración de Cámaras IA AcuSense para Demanda Vehicular (Modo Inteligente)

Para detección inteligente de flujo vehicular en pasos alternados de obra sin requerir computadores externos en el remolque:
* **Conexión Hardware:** Salida de alarma de relé de la cámara (`1A`/`1B`) al pin **`PB0` (Demanda)** con masa `GND`.
* 🛑 **Las cámaras de umbral (2 y 4) NO se instalan en V9.0, y no hay dónde conectarlas.** Medido el 27/08 sobre el esquemático: el pin `PB8` que los manuales daban por suyo **alimenta un LED testigo (`D5` por `R16` 1 kΩ)** — no es una bornera ni una entrada. El paso alternado lo regulan las cámaras de **demanda** y el **todo-rojo temporizado** (`cfgDespejeSeg`), que es el criterio conservador. Ver Manual 9 y `roadmap.md` N-64.
* **Procesamiento:** La cámara Hikvision AcuSense ejecuta su analítica embebida (Detección de Intrusión con filtro `☑ Solo Vehículo`) y cierra su contacto seco al detectar presencia vehicular.
* **Seguridad:** Cada cambio de sentido respeta el tiempo de **Despeje Todo-Rojo** configurado ~~en pantalla~~ **(28/08: desde la app)** antes de habilitar el verde al sentido con demanda.

> 🔴 **28/08 — QUÉ MIDE DE VERDAD EL MODO INTELIGENTE, Y QUÉ NO.** Medido sobre
> `Maestro/src/modo_inteligente.cpp` y `Esclavo/src/main.cpp`:
>
> - **Es UN contacto seco por poste**, leído en `CAM_DEMANDA_PIN` = `PB0`, activo en alto, por la
>   bornera `J14`, con antirrebote de placa (`R64` 10 K + `C25` 100 nF). **Presencia sí/no, no
>   conteo de vehículos.**
> - El número que el equipo llama *«autos esperando»* es `presenciaActual`, la **suma de dos
>   contactos** —el local y el de la otra punta por radio—, así que **vale 0, 1 o 2 y nada más**.
>   No es una cuenta de coches.
> - **No hay ningún puerto de vídeo ni de cómputo externo.** El fichero
>   `6_Preguntas_Diseno_Funcional.md` lo tiene cerrado por decisión: *«Cero Computadores Edge
>   Externos»*.
>
> **Se escribe aquí porque este manual es el «Ground Truth».** Un técnico que lea *«Autos: 2»*
> creyendo que hay dos vehículos contados está leyendo otra cosa: hay demanda en las dos puntas.

---

## 7. Vocabulario Oficial del Mando a Distancia de 4 Relés (Anti-Colisión N-53)

Para permitir la operación del semáforo a nivel del suelo sin colisionar con la edición de parámetros en pantalla:

| Secuencia | Modo Activado | Confirmación Lumínica |
|---|---|---|
| **`A · B · A`** (≤12s) | 🟢 **Modo Automático** | 2 destellos rojos |
| **`B · A · B`** (≤12s) | 🟡 **Modo Ámbar (Seguro)** | 3 destellos rojos |
| **`B · A · B · A`** (≤18s) | ✋ **Modo Manual (Operario)** | 5 destellos rojos |
| **`A · B · A · B`** (≤18s) | 🕒 **Modo Degradado (Reloj RTC)** | 4 destellos rojos |
| **`A · A · B · B`** (≤18s) | 📷 **Modo Inteligente (Cámaras IA)** | 6 destellos rojos |

* **Inhibición de UI (N-53):** Mientras el equipo esté en pantallas de configuración (`AJUSTAR HORA`, `CONFIG_TIEMPOS`), el receptor del mando se inhibe al 100%, evitando que los pulsos de edición disparen cambios de modo involuntarios.
  > ⚠️ **28/08 — esta protección sigue en el firmware, pero ahora protege un menú que nadie ve.**
  > La inhibición impide que las **secuencias** se reconozcan con el menú abierto; **no impide la
  > navegación**, porque navegar es justo lo que el menú abierto acepta. Sin pantalla, un operario
  > que accione el mando sin saber que el equipo está dentro del menú **no tiene forma de enterarse**:
  > no verá destellos (están inhibidos) y creerá que el mando no funciona, mientras el cursor se
  > mueve a ciegas. **Es un riesgo abierto mientras el menú siga compilado en el binario**, y está
  > anotado también en `OPTIMIZACIONES.md`.

---

## 8. Módulo Bluetooth: LA INTERFAZ DEL EQUIPO (Estándar Baliza) — 🔴 ACTUALIZADO EL 28/08

Desde el 28/08 esto **ya no es un accesorio de soporte: es la única interfaz del equipo.**

### Conexión — el módulo entra por `J17`, en los pines que dejó la pantalla

> ## ⛔ ANTES DE CONECTAR NADA: `J16` LLEVA 12 V Y QUEMA EL MÓDULO
>
> **`J16` y `J17` son dos conectores distintos y se parecen.** `J16` es el de la **botonera** y
> **trae 12 V en su posición 1**; `J17` es el de la pantalla y reparte **3,3 V**. Enchufar el módulo
> Bluetooth en `J16` le mete **12 V a una entrada de 3,3 V: se quema, y no avisa antes**.
>
> **Cómo distinguirlos, y es una medida, no una mirada:** con el equipo energizado, **mida la
> posición 1 contra GND**. **Si hay 12 V, ése es `J16` — no es el suyo.** En `J17` la posición 1 es
> `CS`, una señal de la pantalla, no una alimentación.

| `J17` | red | va a | al módulo Bluetooth |
|---|---|---|---|
| **p2** | `RST` → `PB7` | `USART1_RX` del STM32 | **`TXD`** del módulo |
| **p3** | `RS(A0)` → `PB6` | `USART1_TX` del STM32 | **`RXD`** del módulo |
| **p6** | `3,3 V` | alimentación | `VCC` |
| **p7** | `GND` | masa | `GND` |

* **Es el `USART1` REMAPEADO.** El STM32F103 puede sacar el `USART1` por `PB6`/`PB7` en vez de
  `PA9`/`PA10`, **pero solo por un sitio a la vez**. El firmware ya está en esa posición: declara
  `SerialBT(PB7, PB6)` en `bluetooth.cpp`, en las dos puntas.
* **Baudrate:** 9600 bps, 8-N-1.
* **`PA9`/`PA10` sigue siendo válido eléctricamente, pero NO es el montaje vigente.** Esos dos pines
  **no salen a ninguna bornera de la tarjeta**: para usarlos hay que **soldar** en las patas del
  MAX3485 `U2` o del propio micro. Queda como alternativa de laboratorio, no como el cableado de
  campo. *(Este manual decía antes `PA9` TX / `PA10` RX; era correcto para el montaje anterior y ha
  quedado obsoleto, no borrado.)*

> ⚠️ **Todo lo anterior está MEDIDO EN EL ESQUEMÁTICO** (`03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md`
> §7) **y en el fuente del firmware. NADA de esto se ha comprobado con multímetro sobre el cobre.**
> El propio mapeo advierte además de que el símbolo de `J17` tiene 13 posiciones y el footprint 16:
> **al contar pines sobre la tarjeta física hay que contar desde el pin 1, no desde el borde del
> conector.** La verificación de continuidad con la tarjeta delante sigue pendiente.

### Telemetría en vivo — y qué campos NO son medidas

* **Emisión periódica de `$STATUS,...` cada 1 segundo.** Formato real, leído del firmware:
  `$STATUS,NODE:...,SERIE:...,MODO:...,ESTADO:...,T:...,RF:...%,RTT:...ms,BAT:...,HORA:...*XX`

> 🔴 **Tres campos de esa trama NO son medidas, y hay que saberlo antes de decidir con ellos.**
> Medido el 28/08 sobre `bluetooth.cpp` en las dos puntas:
>
> | campo | Maestro | Esclavo |
> |---|---|---|
> | `RF:` | ✅ **real** — telemetría del latido de 3 s (SFTY-14) | 🔴 **literal `98%`** escrito en el `snprintf` |
> | `RTT:` | ✅ **real** | 🔴 **literal `85ms`** |
> | `BAT:` | 🔴 **literal `12.6`** | 🔴 **literal `12.6`** |
> | `T:` | ⚠️ **no es la cuenta regresiva**: es `(millis()/1000) % 60`, un contador libre de 0 a 59 | ⚠️ igual |
>
> **Este manual prometía antes «cuenta regresiva» y «% de señal RF».** La cuenta regresiva **no
> existe en la trama**, y el `RF:` del Esclavo **afirma un 98 % aunque la radio esté desconectada**.
> Un tablero que inventa el dato que no tiene es peor que uno que se calla: quien decide sobre el
> tráfico mirándolo cree estar viendo el enlace. **Se deja escrito en vez de corregirse en silencio,
> porque el arreglo es de firmware y no de manual.**
* **Caja Negra de Alarmas:** Registro inmediato de eventos con timestamp (`$ALARM,EVENTO:FALLO_RF,CAUSA:SILENCIO_25000ms...` —**el nombre del evento ya no lleva el número dentro**: el umbral va en la causa, para que no quede mintiendo el día que se ajuste) para diagnosticar la causa exacta de cualquier caída de radio en obra.
* **Operación Multicruce (Un solo celular para la vía):** La App permite gestionar toda la carretera con un selector de cruces viales (Km 12, Km 24, etc.) y detecta automáticamente si está conectada a `👑 MAESTRO (Poste 1)` o `📡 ESCLAVO (Poste 2)`.
* **Modo Asistente Courier RTC (Sincronización Puente sin Radio):** Si no hay enlace de radio entre postes, el técnico captura hora y ciclo en el Maestro, viaja en vehículo al Esclavo, y la App inyecta la sincronización compensando automáticamente el tiempo de viaje con su reloj interno de alta precisión ($\Delta t < 0.1\text{ s}$).

