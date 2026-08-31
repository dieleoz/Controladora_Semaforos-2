# ⚡ Matriz de Optimizaciones y Reglas de Seguridad (V8.7)

**Fecha de Revisión:** 31 de Julio de 2026  
**Ecosistema:** Firmware STM32 + Repetidor ESP32 + Protocolo RF 9600 bps  

---

> ## 🔴 28/08/2026 — SE RETIRA LA PANTALLA LCD. QUÉ REGLAS QUEDAN SIN INTERFAZ
>
> **La pantalla no se lee.** El equipo va montado en alto, y una LCD de 128×64 a 5 m dentro del
> gabinete no la mira nadie desde el suelo. **La interfaz de operación pasa a ser la app por
> Bluetooth**, y el módulo entra por el conector `J17`, en los dos pines que deja la pantalla
> (`PB7`/`PB6` = `USART1` remapeado).
>
> **Lo que esto le hace a las reglas de arriba, sin adornos:**
>
> - **SFTY-12 (Navegación de Menú Independiente)** decía que el operario debe poder configurar el
>   equipo *«incluso si las radios están apagadas»*. **La regla sigue viva y su motivo también**,
>   pero **su vía ya no es el menú local**: es el enlace Bluetooth, que es igual de independiente de
>   la radio de largo alcance. Lo que la regla exigía —independencia de red— se conserva; lo que
>   cambia es por dónde entra el operario.
> - **SFTY-18** describe la pantalla **`AJUSTAR HORA`** como *«la única vía para poner el reloj y,
>   por tanto, el requisito previo de SFTY-20 y SFTY-21»*. **Sin pantalla esa vía se cierra.** La
>   sustituye el comando `CMD:PIN:...:SET_RTC:` del Bluetooth, que existe y está en el firmware
>   (`bluetooth.cpp`, ambas puntas). **Todo lo demás de SFTY-18 no cambia**: la hora sigue naciendo
>   declarada no fiable y `reloj_enHora()` sigue siendo la barrera.
> - **SFTY-14 y SFTY-15** publican sus medidas *«en la pantalla PRUEBA ALCANCE»*. Esa pantalla
>   **existe en el firmware y ya no tiene dónde dibujarse.** La calidad de enlace sí sale por
>   telemetría (`RF:` de `$STATUS`); **los contadores de línea de SFTY-15 —`RX 0 - nada llega` /
>   `RX 4k - BASURA`— NO están en la trama `$STATUS`.** Es una capacidad de diagnóstico que se pierde
>   hasta que alguien la lleve a la app, y se anota aquí en vez de darla por trasladada.
>
> ### ⚠️ EL MENÚ NO SE HA BORRADO DEL FIRMWARE, Y ESO ES UN RIESGO NUEVO
>
> **Medido sobre el fuente (28/08):** `lcd.cpp`, `menu.cpp` y `modo_hora.cpp` siguen compilándose;
> `lcd_setup()` sigue llamando a `u8g2.begin()`; y los cuatro botones de `J16` (`PB9`, `PB13`,
> `PB14`, `PB15`) siguen navegando el menú. Lo único que cambia es que **no hay display donde se
> vea el resultado**.
>
> **Consecuencia:** quien pulse esos botones —o accione el mando de relés, que va en paralelo con
> ellos— **navega un menú a ciegas**. Con suficientes pulsos se llega a `CONFIGURACION → AJUSTAR
> HORA` y **se confirma una hora cualquiera que el equipo dará por buena**. Es exactamente el fallo
> que la prueba 8.6 del protocolo existe para descartar, pero **ahora sin el aviso visual que
> permitía detectarlo**.
>
> La inhibición del mando con el menú abierto (SFTY-21) **sigue protegiendo contra las secuencias**;
> **no protege contra la navegación**, porque la navegación es justo lo que el menú abierto sí
> acepta. **Mientras el menú siga en el binario, esto es un riesgo abierto y no una nota
> histórica.**

---

## 📌 Reglas de Seguridad Inquebrantables (SFTY-1 a SFTY-18)

- **SFTY-1:** Watchdog Timer IWDG activo a **4.0s** en Maestro y Esclavo STM32 (`IWatchdog.begin(4000000)`), con refresco obligatorio en `loop()`. *El Repetidor ESP32 no implementa watchdog.*
- **SFTY-2:** Enclavamiento por hardware/software que prohíbe luz Verde y Rojo simultáneas en la misma cara.
- **SFTY-3:** Suma de verificación polinomial **CRC-8 Maxim (`0x31`)** en todos los paquetes RF.
- **SFTY-4:** Lógica de despeje **All-Red (Rojo Fijo en ambos semáforos)** con tiempo configurable de **10 a 90 s**. El piso de **10 s** es inquebrantable por software —`modoAutomatico_fijarTiempos()` hace `return false` fuera de rango, venga del menú o de la radio— así que no es posible configurar despeje nulo. El par vive una sola vez, en `Maestro/src/modo_automatico.cpp:34` (`DESPEJE_SEG_MIN = 10, DESPEJE_SEG_MAX = 90`), y lo releen `documentos_04_cifras_sin_vigilante` y el arnés del automático en cada corrida. *Estuvo publicado aquí como ~~«5 a 999s», con piso de 5s~~ hasta el 31/08/2026: el piso era la mitad del real, el techo once veces el real, y **999 no cabía siquiera en el `uint8_t` que transporta el valor** — ninguna versión del firmware pudo aceptarlo nunca.*
- **SFTY-5:** Transición de luz legal en Colombia (Res. 2024): Verde $\rightarrow$ Rojo Directo (0s); Rojo $\rightarrow$ Amarillo Fijo (4.0s) $\rightarrow$ Verde.
- **SFTY-6:** Timeout de fallback a **25.0s** sin PONG/PING para entrar a **🟡 Amarillo Intermitente** en ambos lados. El umbral vive una sola vez, en `*/include/protocolo.h` (`SFTY6_SILENCIO_MS = 25000UL`, idéntico en las dos puntas). *Estuvo publicado aquí como **12.0s** hasta el 31/08/2026: era el valor anterior a **N-71**, y esa cifra es justamente la que no cabía por encima de los reintentos.*
- **SFTY-7:** Reintento automático de órdenes ACK cada **3.5s** (`TIMEOUT_ACK_MS`), **validado en campo el 31/07/2026** con la tasa aérea a 2.4 kbps. El fallback de seguridad de 25.0s (SFTY-6) es el **techo** de la ventana, y la cuenta que lo sostiene sale del C++: los **5 reintentos** de `CICLO_MAX_REINTENTOS` a 3.5s (≈3.56s con el aire de la ráfaga), más los 3.0s de cadencia del latido, dan **~20,8s de peor caso** bajo un techo de 25.0s. **Con el techo de 12.0s que se publicaba antes de N-71 sólo cabían 2 o 3: los reintentos 4 y 5 eran código muerto**, porque el ámbar por orfandad saltaba primero. La desigualdad la recalcula `costura_09_presupuesto_radio` desde las constantes en cada corrida, en vez de vivir en esta frase. *No se sube "por si acaso" ante mayor distancia: la distancia aumenta la probabilidad de pérdida, no la latencia, y contra una trama perdida sirve repetir, no esperar.*
- **SFTY-8:** Repetidor ESP32 asíncrono. *Desde V8.3 la liberación del bus ya no es por ventana de silencio de 5 ms sino inmediata al terminar cada trama — ver SFTY-16.*
- **SFTY-9:** **Self-Healing Autónomo**: Reconexión de red sin reinicio manual, antecedida por 15s de All-Red de seguridad.
- **SFTY-10:** Ventana deslizante (`memmove`) para rescatar paquetes RF en ambientes con ruido eléctrico o pérdida de bytes.
- **SFTY-11:** Transmisión en ráfaga configurable vía `RF_BURST_COPIES` (`protocolo.h`), **fijada en 3 copias**. El coste de la ráfaga lo determina la **tasa aérea**, no el protocolo: a 2.4 kbps son ~0.13s de aire (despreciable), mientras que a 0.3 kbps eran ~2.2s y ahí sí saturaban el canal. Con la tasa corregida la redundancia vuelve a ser barata, y es **la palanca correcta frente a distancias variables** en equipos móviles.
- **SFTY-12:** **Navegación de Menú Independiente**: Mantiene a Maestro y Esclavo en **🔴 ROJO FIJO CONTINUO** (o Amarillo Intermitente sin coms).
- **SFTY-13:** **Supresión Anti-Colisión de PING**: Suprime el Heartbeat PING durante espera de ACK para evitar colisiones RS485.
- **SFTY-14 (V8.1):** **Telemetría de calidad de enlace**. Sobre el latido de 3 s que ya existe se mide si hubo respuesta y cuánto tardó, en ventana deslizante de 10 latidos. Se expone en los modos de operación (`RF:100% 340ms`) y en la pantalla **PRUEBA ALCANCE**. Solo se acepta como respuesta el comando que corresponde (`PONG` a un `PING`, `ACK_RED` a un `GO_RED`): aceptar cualquier paquete falsearía la medida al alza. No requiere soporte de la radio ni cambios de protocolo.
- **SFTY-15 (V8.3):** **Diagnóstico de línea**. `protocolo.cpp` cuenta bytes recibidos, tramas válidas y tramas descartadas por CRC. La pantalla **PRUEBA ALCANCE** los muestra en su fila inferior, separando tres fallos que antes se veían todos como "no hay comunicación": `RX 0 - nada llega` (cobertura, canal o antena), `RX 4k - BASURA` (llegan bytes pero ninguna trama válida: cableado, línea flotando o radio atascada) y `RX 36 9 tr` (enlace correcto). Los contadores se ponen a cero al entrar a la pantalla.
- **SFTY-16 (V8.3):** **Puente que valida antes de retransmitir**. El repetidor ESP32 dejó de ser un passthrough ciego: ahora reconoce el formato (4 bytes con CRC-8 Maxim) y **solo relaya tramas válidas**. Si el par RS485 de entrada queda flotando, el ruido se descarta dentro del ESP32 y no llega al aire. Antes, ese ruido mantenía la transmisión permanentemente activa y la radio de salida saturaba el canal (fallo de campo del 31/07: LED TX fijo en B2). Además la transmisión solo se activa cuando hay algo real que enviar, no ante el primer byte. Compilar con `-D PUENTE_TRANSPARENTE` revierte al comportamiento anterior.
- **SFTY-17 (V8.4):** **Retardo de cortesía del Esclavo antes de responder** (`RETARDO_RESPUESTA_MS = 200`). En modo repetidor hay una radio intermedia (B2) que acaba de **transmitir** la orden y necesita tiempo para volver a **recepción**. Si el Esclavo contesta de inmediato, su respuesta sale mientras B2 sigue conmutando y **B2 no la oye**: el enlace funciona en un sentido y no vuelve nada. Observado en campo el 31/07 con el contador del puente marcando `C<-Esclavo = 1 byte` en dos minutos mientras la ida fluía. La respuesta se **programa**, no se bloquea el bucle, así que el parpadeo de ámbar y el watchdog siguen atendidos. En enlace directo es inofensivo: el Maestro espera hasta 3.500 ms.
- **SFTY-18 (V8.5):** **Reloj de tiempo real con hora declarada no fiable por defecto**. El Maestro usa el RTC interno del STM32 con el cristal `Y2` de 32.768 kHz que la tarjeta ya traía y una pila CR2032 en `VBAT` (ver [`03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md`](03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md) §4). No ocupa ningún pin: el I²C por hardware está copado por LCD y RS-485, así que un módulo externo habría obligado a I²C por software. **La regla de seguridad no es tener reloj, es saber cuándo no se tiene:** al ajustar la hora se escribe también un año marcador, y al arrancar `reloj_enHora()` solo devuelve `true` si ese marcador sobrevivió. Pila agotada, primera puesta en marcha o dominio de respaldo corrupto ⇒ **`false`**, y toda función que dependa de la hora debe abstenerse. Un reloj sin poner en hora que se cree válido es peor que no tener reloj: activaría la operación nocturna a deshora. **V8.6:** se añade la pantalla **AJUSTAR HORA** —quinta opción del menú— que es la única vía para poner el reloj y, por tanto, el requisito previo de SFTY-20 y SFTY-21. Se edita **dígito a dígito** con el dígito activo subrayado: con un solo botón de subir, poner los minutos como valor completo costaría hasta 59 pulsaciones, y además la edición por dígitos **funciona igual con el mando de relés**, que solo entrega pulsos y no admite repetición por mantener pulsado. Se trabaja sobre una copia y solo se escribe al RTC al confirmar, de modo que entrar por error y salir con el Botón 4 no altera la hora. La pantalla **no arranca ciclos**: mantiene el mismo estado seguro que el menú.

---

## 🔗 Trazabilidad requisito → implementación

Cierra el pendiente **N-6**. Cada regla se localiza en el código por su etiqueta `SFTY-x`; esta tabla
se obtiene **buscando esas etiquetas en los fuentes**, no escribiéndola a mano, así que una regla sin
implementar aparece vacía en vez de aparentar cobertura.

> ### 🆕 Tercera columna (03/08/2026): qué lo DEMUESTRA
>
> *"Dónde vive"* dice que la regla está escrita. **No dice que se cumpla.** La tercera columna
> nombra el pack del banco que la ejerce, y se levanta igual que las otras dos: **buscando** la
> etiqueta `# EJERCE SFTY-x` en `banco/packs/`, no escribiéndola a mano.
>
> **Una fila con la tercera columna vacía es una regla sin evidencia automática**, y se ve de un
> vistazo. Hoy la mayoría lo está: es el trabajo que queda, y es exactamente lo que un auditor
> funcional viene a buscar.
>
> Solo se etiqueta lo que el pack **ejerce de verdad**. Etiquetar de más convertiría esta tabla en
> un adorno — una regla apareciendo cubierta por una prueba que no la comprueba es **peor** que
> una fila vacía, porque la vacía al menos no miente.
>
> Correr una sola: `python 01_Firmware/Simulaciones/banco/correr.py --pack <nombre>`
>
> 🔴 **Y esa promesa estuvo sin instrumento hasta el 27/08 (N-62).** La tabla decía levantarse
> del `grep` y estaba escrita a mano: la fila de `SFTY-2` citaba un solo pack cuando ya había
> **tres** etiquetados —`barrera_02_dos_puntas` y `esclavo_06_no_abre_paso` llevaban días con su
> `# EJERCE` puesto—. Es la misma clase de fallo que las cifras del README: una frase que dice
> *"derivado"* encima de algo derivado una vez y copiado después. Ahora lo comprueba el pack
> **`documentos_02_trazabilidad_sfty`**, y **en las dos direcciones**: ninguna etiqueta sin fila
> y ninguna fila citando un pack que no la declare —la regla con la que se retiraron los tres
> monolitos, aplicada a un documento—.

| Regla | Dónde vive | Qué lo demuestra |
|---|---|---|
| SFTY-1 | `Maestro/src/main.cpp` · `Esclavo/src/main.cpp` | — |
| SFTY-2 | `Maestro/src/semaforo.cpp` · `Esclavo/src/semaforo.cpp` | ✅ `barrera_01_pines_de_luz` · `barrera_02_dos_puntas` · `esclavo_06_no_abre_paso` · `maestro_09_test_leds` |
| SFTY-3 | `*/src/protocolo.cpp` · `Repetidor/src/main.cpp` | — |
| SFTY-4 | `Maestro/src/coordinador.cpp` · `Maestro/src/modo_automatico.cpp` | — |
| SFTY-5 | `Maestro/src/semaforo.cpp` · `Esclavo/src/semaforo.cpp` | ✅ `Validacion_Automatico/arnes_automatico.cpp` — **arnés C++, invisible para el censo de packs**, ver abajo |
| **SFTY-6** | `*/include/protocolo.h` *(el umbral)* · `Maestro/src/coordinador.cpp` · `Esclavo/src/main.cpp` | ✅ `costura_08_silencio` · `costura_09_presupuesto_radio` · `maestro_04_sync_horaria` |
| SFTY-7 | `*/src/protocolo.cpp` | — |
| SFTY-8 | `*/src/protocolo.cpp` | — |
| SFTY-9 | `Maestro/src/coordinador.cpp` · `Maestro/src/main.cpp` | — |
| SFTY-10 | `*/src/protocolo.cpp` | — |
| SFTY-11 | `*/include/protocolo.h` · `*/src/protocolo.cpp` | — |
| SFTY-12 | `Maestro/src/coordinador.cpp` · `Maestro/src/modo_manual.cpp` | — |
| SFTY-13 | `Maestro/src/coordinador.cpp` | — |
| SFTY-14 | `Maestro/src/coordinador.cpp` | — |
| SFTY-15 | `*/src/protocolo.cpp` · `Maestro/src/lcd.cpp` · `Maestro/src/modo_alcance.cpp` | — |
| SFTY-16 | `Repetidor/src/main.cpp` | — |
| SFTY-17 | `Esclavo/src/main.cpp` | — |
| **SFTY-18** | `Maestro/src/reloj.cpp` · `Maestro/include/reloj.h` · `ESP32_Expansion` *(el reloj del puente)* | ✅ `esp32_04_osf` |
| **SFTY-19** | **— solo diseño, ver abajo.** La única mención en el código es una advertencia en `reloj.h` aclarando que este modo **no** se apoya en el RTC | — |
| **SFTY-21** | `*/src/modo_degradado.cpp` · `*/src/mando.cpp` · `*/include/ciclo_degradado.h` | ✅ `esclavo_01_latch_ambar` · `esclavo_02_inhibicion_menu` · `esclavo_07_ambar_emergencia` · `esclavo_08_ambar_en_degradado` · `maestro_01_mando` · `maestro_05_ciclo_sin_radio` · `costura_02_fase_ciclo` · `costura_06_reanudacion` · `camara_02_j16` |
| **SFTY-23** | `Maestro/src/coordinador.cpp` · `Esclavo/src/config_ciclo.cpp` *(Fase 2)* · `*/src/reloj.cpp` | ✅ `esclavo_03_par_config` · `esclavo_04_desfase` · `esclavo_05_hora_atomica` · `maestro_04_sync_horaria` |
| **SFTY-28** | `*/src/semaforo.cpp` *(dentro de `escribirPines()`)* · `*/include/pines.h` | ✅ `barrera_03_talanquera` · `maestro_09_test_leds` |
| **SFTY-29** | **— solo diseño.** Presencia como veto del todo-rojo y sensor de pluma | — |

### Discrepancias corregidas al levantar esta tabla

- **La ventana deslizante estaba etiquetada `SFTY-11` en el código** cuando `SFTY-11` es la ráfaga.
  Corregido a `SFTY-10` en Maestro y Esclavo. Era el caso concreto que describía N-6.
- **SFTY-14 y SFTY-16 no tenían etiqueta** pese a estar implementadas: la telemetría en
  `coordinador.cpp` y la validación del puente en `Repetidor/src/main.cpp`. Añadidas.
- **`reloj.h` afirmaba que `reloj_segundosDelDia()` era la base de SFTY-19**, lo que contradice el
  propio diseño de SFTY-19 (sincronización *relativa*, sin RTC). Reescrito como advertencia explícita
  para que nadie ancle las dos unidades a su reloj de pared.

### 🔴 28/08/2026 — `SFTY-5` era DOS cosas con la misma etiqueta, y la fila estaba mal en las dos columnas

**La etiqueta estaba duplicada.** `SFTY-5` está definida arriba como *«transición de luz legal en
Colombia (Res. 2024)»*, pero en `Maestro/src/protocolo.cpp` y `Esclavo/src/protocolo.cpp` la línea

```cpp
static HardwareSerial AiBus(RS485_IN_RX, RS485_IN_TX); // SFTY-5: Segundo bus UART para IA
```

usaba *«SFTY-5»* para nombrar el **puerto serie de la cámara IA**, que no es una regla de seguridad
ni tiene nada que ver con la transición de luz. Dos cosas distintas con la misma etiqueta, y la tabla
se levanta **buscando la etiqueta**: por eso la fila apuntaba a `*/src/protocolo.cpp`.

**Consecuencia medida sobre el fuente (28/08):**

| | estaba | está |
|---|---|---|
| *Dónde vive* | `*/src/protocolo.cpp` — ruta del comentario impostor | `*/src/semaforo.cpp`, que es donde vive de verdad: `estado == S_AMARILLO && (ahora - tCambio >= 4000)` (línea 258 en el Maestro, 260 en el Esclavo, **idénticas**) |
| *Qué lo demuestra* | vacía | `Validacion_Automatico/arnes_automatico.cpp`, que **relee los 4000 ms del `semaforo.cpp` real** y los ejerce con control negativo |

> **La etiqueta impostora NO se ha borrado del fuente**, y eso es deliberado: `protocolo.cpp` lleva
> hoy un comentario que deja constancia de que esa etiqueta estaba equivocada. Borrarla en silencio
> haría que la próxima persona que la viese volviera a colgarla de la fila de la transición de luz.

### 🕳️ El censo de la tercera columna tiene un hueco, y este caso lo destapó

`documentos_02_trazabilidad_sfty` busca la etiqueta `# EJERCE SFTY-x` **solo dentro de
`banco/packs/`**. Los cuatro arneses que compilan C++ real (§8 de `CLAUDE.md`) son **C++, no packs**,
y por tanto **son invisibles para ese censo**. `SFTY-5` llevaba la tercera columna vacía no porque
nadie la midiera, sino porque **quien la mide no está en el sitio donde se mira**.

> **Es la regla del instrumento aplicada a la propia tabla: un «no aparece» no es un hallazgo hasta
> haber descartado al buscador.** Aquí el buscador era el culpable.
>
> **Esto queda ANOTADO, no arreglado.** El pack está en vuelo en otro proceso y no se toca desde
> aquí. Mientras el censo no mire también los arneses C++, **una fila vacía de esta tabla significa
> «ningún pack la ejerce», no «nada la ejerce»** — y esa diferencia hay que leerla a mano.

### ⚠️ Siete filas vacías, y todas apuntan al mismo fichero

Contadas sobre la tabla de arriba: las filas cuya *«dónde vive»* es `*/src/protocolo.cpp` —**SFTY-3,
5, 7, 8, 10, 11 y 15**— tienen **todas** la tercera columna vacía. Ni un solo pack del banco ejerce
nada de lo que vive en `protocolo.cpp`: ni el CRC-8, ni el reintento de ACK, ni la ventana
deslizante, ni la ráfaga, ni los contadores de diagnóstico de línea.

`CLAUDE.md` advierte del error contrario —*«una regla que aparece cubierta por una prueba que no la
ejerce es peor que una fila vacía, porque la vacía no miente»*—. **Aquí el problema es el inverso, y
también merece quedar escrito:** siete reglas de seguridad seguidas sin evidencia automática **no
son siete casillas pendientes; son un fichero entero sin cobertura**. Una fila vacía suelta se lee
como trabajo por hacer; siete alineadas sobre el mismo `.cpp` señalan **dónde está el agujero**.

**No se inventa cobertura para taparlas.** Se dejan vacías, contadas y con el nombre del fichero
delante, que es lo que un auditor necesita para preguntar por él.

---

## 🕹️ SFTY-21 — Modo Degradado por reloj y mando de 4 relés (**IMPLEMENTADO**)

> **Estado:** especificado y **construido en las dos puntas** el 01/08/2026, en la rama
> `feat/n15-reloj-pantalla-hora`. Sustituye y cierra el diseño anterior de **SFTY-19**, que planteaba
> entrada *automática*.
>
> **Validación:** los tres firmwares compilan sin warnings propios · simulador funcional **20/20** ·
> simulador de repetidor **10/10** · validación de pantalla **83/83**. Maestro 80,2 % de flash,
> Esclavo 59,7 %.
>
> ⚠️ **Sin prueba de banco todavía.** Nada de esto se ha ejercitado sobre hardware real.
>
> 🔴 **Y su commit de implementación es hoy el sospechoso principal de la regresión del Modo
> Automático.** `2779d9b` (01/08 **16:00**) cae en la hora exacta en que el banco sitúa el último
> firmware bueno, y toca **15 ficheros del Maestro**. El mecanismo encaja: el mando de relés
> **intercepta las escrituras de pines de luz** en vez de rodearlas —para no dejar colgado al
> coordinador esperando un `S_VERDE` que no llegaría—, y esa intercepción está justo en el camino
> por el que el ciclo avanza. **No confirmado:** pendiente de bisección con firmware real, ver
> [`roadmap.md`](roadmap.md) §N-42 y `05_Funcional/bisect_entregable/`. Se anota aquí porque una
> regla de seguridad cuya implementación está bajo sospecha no puede figurar como
> `IMPLEMENTADO` a secas.
>
> **Parámetros del ciclo degradado:** verde **30 s**, todo-rojo **30 s** —ya ampliado—, ciclo fijo y
> propio, que **no hereda** el verde del Modo Automático. Al ser fijo, el tope de 255 s del byte de
> `CMD_CONFIG` no puede alcanzarse.
>
> **Pendientes conocidos:** el Esclavo **no tiene mando de relés** (N-19) y el estado del modo **no
> persiste** a un corte de energía (N-20).

### La decisión de operación

**El controlador no cambia. Solo se añade un modo.**

| Situación | Comportamiento |
|---|---|
| Pérdida de radio | **Ámbar intermitente**, exactamente como hoy. **No se toca** |
| Modo Degradado | **Caso especial de activación MANUAL**, confirmado por un operario |
| Puesta en marcha del Degradado | **Desde la pantalla**: validar la hora, confirmar el otro extremo, iniciar y verificar |
| Salida del Degradado | Desde la pantalla, **o con `A·A·A` desde el piso** para reintentar Automático |
| Entrada automática | **NUNCA** |

### Por qué manual y no automático

En el diseño anterior el equipo entraba solo al perder el enlace. Se descartó, y la razón de fondo es
la asimetría entre los dos avisos:

```
 ÁMBAR INTERMITENTE  ->  "no estoy controlando esto, decide tú"
                         el conductor llega ALERTA, mira, negocia el paso

 VERDE POR RELOJ     ->  "pasa tranquilo, el otro lado está en rojo"
                         el conductor llega CONFIADO y no mira
```

Sin radio, el Maestro **no puede saber si el Esclavo sigue vivo**: podría estar apagado, colgado o
haber sido movido. Un verde equivocado es **más peligroso que un ámbar ambiguo**, porque le quita al
conductor la precaución que el ámbar le provoca.

Con activación manual, **el verde deja de darse por suposición y pasa a darse porque una persona
verificó las dos puntas**. Y es defendible ante una auditoría: "un técnico validó ambos extremos y
habilitó un modo especial" es un procedimiento; "la máquina decidió operar a ciegas" no lo es.

### Procedimiento de puesta en marcha

1. Confirmar que **ambas unidades tienen la hora puesta y coincidente**
2. Entrar a **MODO DEGRADADO** desde la pantalla, en cada unidad
3. Iniciar
4. **Verificar visualmente que los dos semáforos alternan correctamente**

### La deriva y el margen

Cristal de 32.768 kHz sin calibrar, a la intemperie: **±30 a 50 ppm**.

| Tiempo sin radio | Desfase entre unidades |
|---|---|
| 1 día | ~2 – 8 s |
| 3 días | ~6 – 25 s |
| 1 semana | ~15 – 60 s |

El **despeje todo-rojo es el colchón que absorbe ese desfase**. Con los 15 s actuales el margen dura
entre 2 y 7 días. **En Modo Degradado el todo-rojo debe ampliarse** —del orden del doble— para
sostener semanas. Se pierde fluidez, que es precisamente lo que se acepta en un modo degradado.

### 🛑 Límite duro: el Degradado debe rendirse solo

El diseño automático descartado (SFTY-19) tenía la regla *"pasadas N horas sin enlace ⇒ ámbar"*. **Al
pasar a activación manual esa regla se perdió**, y quedó solo *"la pantalla pide la resincronización"*
—un aviso, no un tope—. Es un error: **el estado seguro no puede depender de que alguien se acuerde**,
que es justamente el principio que el resto del sistema ya aplica.

Con el desfase puesto a cero por SFTY-23 al perder el enlace, el margen es el que da la deriva:

| Todo-rojo en Degradado | Deriva peor caso | Margen antes de solaparse | **Límite duro propuesto** |
|---|---|---|---|
| 15 s *(el normal)* | ~8,6 s/día | ~1,7 días | — *insuficiente* |
| **30 s** | ~8,6 s/día | ~3,5 días | **48 h** |
| 90 s | ~8,6 s/día | ~10 días | 5 días *(a costa de la fluidez)* |

**Propuesta: todo-rojo de 30 s y límite duro de 48 h**, con factor de seguridad 2 sobre el margen
teórico. Pasadas 48 h sin resincronizar, **el Degradado cae solo a ámbar intermitente**.

> Conviene decirlo sin adornos: **querer una semana de autonomía obliga a un todo-rojo de ~90 s**, que
> destroza la fluidez del paso. No es una limitación del diseño, es la física de dos cristales sin
> disciplinar. La alternativa real no es alargar el plazo: es ir a arreglar el radio.

### ⚠️ Riesgos residuales aceptados por el cliente (01/08/2026)

**1. El verde se da sin confirmación del otro extremo.** Con el radio muerto es inevitable. Se mitiga
con activación manual verificada, todo-rojo ampliado, límite duro y aviso en pantalla. **No se
elimina.**

**2. Salida asimétrica: que una sola punta abandone el Degradado.** Es el escenario más peligroso y no
tiene solución técnica sin radio:

```
   Un microcorte reinicia UNA unidad
        -> arranca en MENU (asi lo hace main.cpp)
        -> sin enlace  ->  ÁMBAR          el conductor NEGOCIA el paso
   La otra sigue dando verde por reloj    el conductor pasa CONFIADO
```

Un lado en ámbar contra un lado en verde es **exactamente el escenario que este modo quiere evitar**.
Ocurre igual si un operario saca del Degradado **una sola** unidad con `A·A·A`.

**Mitigación procedimental, no técnica: la verificación visual de ambas puntas es obligatoria también
AL SALIR**, no solo al entrar. Debe constar en el manual del funcional y en el acta de pruebas.

---

### 🎛️ El mando de 4 relés — interfaz sin realimentación visual

El operario maneja la pantalla con un **mando de 4 relés (A, B, C, D)** cableados **en paralelo con
los botones físicos** (`PB9`, `PB13`, `PB14`, `PB15`; no hay entradas dedicadas). Hoy solo navegan el
menú: `A` arriba, `B` abajo, `C` aceptar, `D` menú.

**El problema: la pantalla está a 5 m, dentro del gabinete.** El operario acciona desde el piso **sin
poder verla**. Un menú es inservible a ciegas: no se sabe dónde está el cursor ni si la pulsación
entró.

#### Restricciones medidas en campo (01/08/2026)

| Medida | Valor | Consecuencia de diseño |
|---|---|---|
| Tipo de señal | **Pulso por flanco**, no se sostiene | **La pulsación larga NO es posible.** Sostener el botón 10 s da un solo pulso |
| Retardo por pulsación | **~2 s** | Una ventana de 3 s es inviable; hacen falta 12–15 s para 3–4 pulsos |
| Repetición automática | **No la hay** | Cada pulso exige una pulsación |

#### La única salida visible desde el piso son las luces

El operario no ve la LCD, pero **sí ve el semáforo**. Por eso la confirmación se da en **destellos
ROJOS contables**: el rojo nunca significa "pase", así que **si el operario cuenta mal, el peor caso
sigue siendo seguro**. Destellar los tres colores a la vez se descartó: un conductor lejano podría
interpretar el verde.

#### Configuración propuesta

| Secuencia | Acción | Confirmación |
|---|---|---|
| **`A · A · A`** *(≤ 12 s)* | **AUTOMÁTICO** — "a ver si el radio volvió" | **2** destellos rojos |
| **`B · B · B`** *(≤ 12 s)* | **ÁMBAR intermitente** — salida de emergencia | **3** destellos rojos |
| **`A · B · A · B`** *(≤ 18 s)* | **Entrar a MODO DEGRADADO** | **4** destellos rojos |

#### Memotecnia

> **`A` es arriba → SUBE al modo normal.**
> **`B` es abajo → BAJA al mínimo seguro.**
> **Alternar → modo especial.**

Se aprende en un minuto. Es el requisito real para alguien que lo usa de madrugada y bajo lluvia.

#### La salida también debe poder hacerse desde el piso

Si se puede **entrar** al Modo Degradado desde el suelo pero para **salir** hay que subir, el mando no
sirve. El escenario típico es *"dejó de llover, a ver si el radio volvió"*.

Volver a Automático **no necesita protección**, porque el propio sistema se corrige:

```
   Automático intenta hablar con el otro lado
        -> sin respuesta en 25 s  (SFTY-6)
        -> se va solo a ÁMBAR INTERMITENTE
```

El peor caso de intentar Automático es **volver al ámbar**, que es justo donde se quería estar.

**Y el resultado de la prueba se ve en las luces, sin pantalla:**

```
   A·A·A  ->  2 destellos  ->  esperar ~15 s

     luces CICLANDO  ->  el radio volvió, ya está en automático
     luces en ÁMBAR  ->  sigue muerto; puede volverse al degradado
```

#### Ninguna de las tres deja el equipo en estado peligroso

| Secuencia | Si se dispara por accidente |
|---|---|
| `A·A·A` | Va a automático; sin radio cae a ámbar en 25 s. **Seguro** |
| `B·B·B` | Va a ámbar. **Seguro por definición** |
| `A·B·A·B` | Solo entra **si el reloj está validado**; si no, lo rechaza |

#### Por qué SOLO `A` y `B`, y nunca `C` ni `D`

La primera versión de este diseño usaba `C` y `D`, razonando que repetir *arriba* o *abajo* es normal
al navegar y podría disparar una secuencia por accidente. **Ese razonamiento estaba invertido**, y el
cliente lo corrigió el 01/08/2026.

El riesgo grave no es el falso positivo: es **qué ocurre si la pulsación llega cuando el sistema está
en un sitio distinto del que el operario cree**. Y a ciegas, eso siempre es posible.

```
   Equipo dejado en el MENÚ, y llega C·C·C desde el piso:
     1er C  ->  SELECCIONA lo que tenga el cursor -> arranca un modo no pedido
     2º  C  ->  en Modo Manual, C es ROJO FIJO INDEFINIDO
     3er C  ->  ...

   Mismo caso, llega A·A·A:
     el cursor sube tres veces.  No ocurre NADA.
```

**`C` ejecuta; `A` y `B` solo mueven.** La regla correcta es: **a ciegas se usan únicamente los
botones cuya repetición accidental es inofensiva.**

#### Por qué el degradado va alternado

`A·B·A·B` **no se produce nunca navegando**: se sube o se baja, no se zigzaguea. Y si el operario se
equivoca a mitad de la secuencia, **lo único que ha ocurrido es que el cursor se movió**.

#### 🔒 Requisito: ignorar las secuencias mientras el menú está abierto

Empezó siendo un afinamiento opcional —evitar que un técnico que baja tres veces con `B` dispare el
ámbar, molesto pero inofensivo—. **Al añadirse AJUSTAR HORA como quinta opción del menú pasó a ser
requisito**, porque el riesgo dejó de ser inofensivo:

```
   Ráfaga accidental de pulsos desde el mando, con el menú abierto
        -> el cursor llega a AJUSTAR HORA
        -> unos pulsos más CONFIRMAN una hora cualquiera
        -> el reloj queda MARCADO COMO VÁLIDO con una hora inventada
```

Eso es **exactamente el veneno que SFTY-18 existe para evitar**: no la falta de reloj, sino un reloj
falso que se cree bueno. Y habilitaría el Modo Degradado y la operación nocturna sobre una hora
inventada.

**Regla: mientras el menú esté abierto, las secuencias del mando no se reconocen.** Desde el piso el
operario lo distingue sin ver la pantalla: si las luces están ciclando, el menú no está abierto.

#### Asimetría deliberada: lo seguro fácil, lo peligroso difícil

| Si se dispara por accidente | Consecuencia | Protección |
|---|---|---|
| Ámbar intermitente | El equipo va a seguro. Molesto, no peligroso | Secuencia corta |
| Modo Degradado | Verde sin confirmar el otro lado | Secuencia larga **+ validación en firmware** |

#### La red de seguridad real no es la secuencia

```
   A·B·A·B desde el piso
        |
        +-- ¿El reloj está en hora?
        +-- ¿Hay una medición de desfase reciente y dentro de tolerancia? (SFTY-23)
        +-- ¿La configuración del ciclo se sincronizó con el otro lado? (SFTY-23)
        |
        +-- SÍ  ->  4 destellos rojos  ->  entra a MODO DEGRADADO
        |
        +-- NO  ->  ÁMBAR RÁPIDO 2 s   ->  RECHAZADO
```

**Aunque alguien acierte la secuencia por casualidad, el firmware no entra** si la hora no está
validada. El mando permite reactivar en campo sin grúa, pero **no** saltarse la puesta a punto.

> **`B·B·B` devuelve a ámbar desde cualquier estado, sin condiciones.** Es la regla que impide que
> nadie quede atrapado con un semáforo en estado raro a 5 m de altura.

El menú **sigue funcionando igual** para el técnico que sube: un pulso navega, la secuencia cambia de
modo. No hay que reprogramar lo que ya funciona.

---

## ⏱️ SFTY-23 — Sincronización horaria por radio (**IMPLEMENTADO**)

> **Estado:** especificado y **construido en las dos puntas** el 01/08/2026, tras una auditoría que
> detectó que el procedimiento de SFTY-21 era insuficiente. **Requisito previo del Modo Degradado**,
> no un extra.
>
> ⚠️ **Sin prueba de banco.** Nada de esto se ha ejercitado sobre hardware real.

### El defecto que corrige

SFTY-21 pedía *"confirmar que ambas unidades tienen la hora puesta y coincidente"* mirando las dos
pantallas. **Eso no funciona**, y la aritmética lo demuestra:

```
   Operario A confirma el Maestro   a las 14:32:10 reales -> el reloj marca 14:32:00
   Operario B confirma el Esclavo   a las 14:32:50 reales -> el reloj marca 14:32:00

   Las dos pantallas muestran 14:32.  Los relojes están a 40 s.
```

**Hasta 59 s de desfase el primer día** — casi cuatro veces el todo-rojo de 15 s — y **dos pantallas
en `HH:MM` no pueden detectarlo**. La tabla de deriva de SFTY-21 asume que las unidades arrancan en
~0 de desfase; ajustando a mano, esa premisa es falsa.

### La regla

**La hora se cuadra UNA sola vez, en el Maestro, y el Esclavo nunca se toca a mano.** El Maestro
empuja su hora por radio mientras el enlace vive. Así, el día que el radio muera, el desfase arranca
en ~0 **de verdad**, no por procedimiento.

Y la pila es lo que lo hace durable: sincronizados una vez, **cada reloj sobrevive los cortes de
energía con su propia CR2032**, y la única deriva que queda es la posterior a la pérdida del enlace.

### Comandos nuevos

El paquete RF son 4 bytes con **un solo `param`**, así que la hora no cabe en una trama. Se usan
varias, con el mismo patrón de reintentos que ya existe (SFTY-7). Libres desde `0x07`:

| Comando | Código | `param` |
|---|---|---|
| `CMD_HORA_H` | `0x07` | hora (0–23) |
| `CMD_HORA_M` | `0x08` | minuto (0–59) |
| `CMD_HORA_S` | `0x09` | segundo (0–59) — **al recibirla, el Esclavo aplica las tres juntas** |
| `CMD_ACK_HORA` | `0x0A` | confirmación del Esclavo tras aplicar la terna |
| `CMD_DELTA` | `0x0B` | segundo actual del Maestro |
| `CMD_DELTA_RESP` | `0x0C` | diferencia medida, complemento a dos (`int8_t`) |
| `CMD_CONFIG_VERDE` | `0x0D` | segundos de verde del ciclo degradado |
| `CMD_CONFIG_DESPEJE` | `0x0E` | segundos de todo-rojo, **ya ampliado** |
| `CMD_ACK_CONFIG` | `0x0F` | confirmación del par de configuración |

> ⚠️ **Esta tabla estuvo mal hasta el 01/08/2026** y lo detectó una auditoría de manuales. Decía
> `CMD_DELTA = 0x0A` y `CMD_CONFIG = 0x0B`, que era el **boceto** escrito antes de cerrar
> `protocolo.h`; al ampliarse el contrato con los ACK y el par de configuración, la especificación no
> se actualizó. **Quien implementara contra ella habría usado códigos equivocados.**
>
> **La fuente de verdad es `01_Firmware/*/include/protocolo.h`**, verificado idéntico en ambos
> proyectos. El simulador lee los códigos de ahí en cada ejecución, con lectura obligatoria, para que
> una divergencia como ésta no pueda volver a pasar inadvertida.

El Esclavo **acumula hora y minuto en un buffer y solo escribe el RTC al llegar la de segundos**:
aplicación atómica, nunca queda una hora a medias. `reloj_ajustar()` ya acepta segundos, así que del
lado del reloj no hay que tocar nada. Y el puente **SFTY-16 valida formato y CRC, no comandos**, de
modo que las tramas nuevas atraviesan el ESP32 sin modificarlo.

> ### ⚠️ Regla obligatoria en los reintentos
>
> **El Maestro debe RECALCULAR el valor de segundos en cada retransmisión, nunca reenviar el que
> calculó la primera vez.** Si la trama se pierde y se reintenta 3,5 s después con el valor viejo, el
> Esclavo queda 3,5 s atrasado — y el error entra justo por el mecanismo que existe para dar robustez.
>
> Es un fallo de una sola línea que no se ve en pruebas con enlace bueno.

**Cuándo se dispara:** al confirmar en **AJUSTAR HORA** del Maestro —poner en hora *es* sincronizar,
un solo gesto— y **periódicamente mientras haya enlace**. Con una vez por hora sobra: la deriva entre
sincronizaciones queda en milisegundos.

### La validación debe ser una MEDICIÓN, no una inspección ocular

`CMD_DELTA` convierte el paso 1 del procedimiento de SFTY-21 —*"confirmar que la hora coincide"*— de
mirar dos pantallas a **leer un número**:

```
   ┌──────────────────────────────┐
   │ Desfase Esclavo:      +1 s   │
   │ Ultima sincronizacion: 14:32 │
   └──────────────────────────────┘
```

Registrable en el acta del protocolo de pruebas, y **habilita el gate de entrada en firmware**: el
Modo Degradado solo se permite si hay una medición de desfase **dentro de tolerancia** y **reciente**.
Eso da contenido real a la condición *"¿se sincronizó con el otro lado alguna vez?"*.

**Precisión de la medida.** El desfase medido incluye el tiempo de aire más el retardo de cortesía del
Esclavo (SFTY-17, 200 ms), así que trae un sesgo de **algunas décimas de segundo**. Frente a un
todo-rojo de 15–30 s es irrelevante, y conviene dejarlo escrito para que nadie persiga ese error.
El `param` es de un byte: la diferencia se transmite **con signo, ±127 s**, y fuera de ese rango debe
**saturar y reportarse como "fuera de rango"**, nunca dar la vuelta.

> ### ⚠️ Límite inherente: la medida solo alcanza ±30 s
>
> Detectado al implementar el lado Esclavo (01/08/2026). Como `CMD_DELTA` transporta **solo el
> segundo** (0–59), la corrección circular resuelve en el sentido corto y el resultado **siempre cae
> en ±30 s**. Consecuencia:
>
> ```
>    Desfase real de 45 s  ->  se mide como -15 s
>    No hay forma de distinguirlos con solo el segundo.
> ```
>
> **Un desfase peligroso podría leerse como aceptable y pasar la puerta del Modo Degradado.**
>
> ### La regla que cierra el agujero
>
> **La puerta del Degradado NO puede apoyarse solo en el desfase medido.** Debe exigir **las dos
> condiciones a la vez**:
>
> 1. **Una sincronización correcta reciente** *(propuesta: menos de N horas)*
> 2. **Desfase medido dentro de tolerancia**
>
> La primera es la que hace fiable a la segunda: tras una sincronización correcta el desfase arranca
> en milisegundos, y con una deriva de ~100 ppm hace falta **más de tres días** para acumular los
> 30 s que provocarían el alias. Con una sincronización de hace una hora, la deriva es de **~0,36 s**:
> la medida no puede estar aliasada.
>
> **El desfase es una comprobación de cordura, no la garantía.** La garantía es la sincronización
> reciente. Invertir esa relación —confiar en el número y no en su frescura— reintroduce el fallo.
>
> *Alternativa si algún día hiciera falta rango mayor: añadir una trama con el minuto y calcular
> sobre segundos-dentro-de-la-hora, lo que llevaría el alcance a ±30 min. Hoy no es necesario.*

### El ciclo también debe viajar, no solo la hora

Dos relojes en hora dan **tiempo común**, pero para ir en fase ambas unidades tienen que computar el
**mismo horario de fases**: quién está en verde en cada instante, cuánto dura y cuánto es el todo-rojo
ampliado. Hoy esa configuración **solo existe en el Maestro** y viaja por radio en operación normal.

Sin radio, o se configura a mano en las dos puntas —otra fuente de error humano, la misma que este
documento acaba de eliminar para la hora— o **se sincroniza junto con la hora mientras hay enlace**.
`CMD_CONFIG` cubre eso. **Es tan condición de seguridad como la hora**, y faltaba en SFTY-21.

### Lo que la sincronización NO arregla

**La deriva posterior sigue corriendo.** Sincronizar pone el desfase a cero en el momento de perder el
radio, pero a partir de ahí crece igual. Por eso el **límite duro** de SFTY-21 sigue siendo necesario.

---

## 📺 SFTY-22 — Pantalla informativa durante el ámbar (MEJORA, **NO IMPLEMENTADO**)

> **Estado:** marcada como mejora el 01/08/2026. No es urgente: el sistema funciona.

Hoy, al perder el enlace, el equipo entra en ámbar intermitente **sin decir por qué**. Quien sube a
revisarlo se encuentra un ámbar mudo y empieza a preguntar: ¿desde cuándo?, ¿alguien tocó algo?,
¿es el radio o la configuración?

La propuesta es que, **al entrar en ámbar por pérdida de enlace, aparezca sola** una pantalla de
diagnóstico:

```
   +------------------------------+
   | SIN ENLACE                   |  <- por qué está en ámbar
   | Desde: 08:32   Hace: 2h 14m  |  <- lo aporta el RTC (SFTY-18)
   |                              |
   | RX 0 - nada llega            |  <- SFTY-15, ya existe
   | Ultimo RF: 100%  340ms       |  <- SFTY-14, ya existe
   +------------------------------+
```

**Casi todo el dato ya está dentro del firmware** desde el 31/07. Lo único que falta es juntarlo en
una pantalla y **la hora**: sin RTC solo se puede decir "hace un rato", no "se cayó a las 08:32".

### ⚠️ Un diagnóstico no debe alterar lo que diagnostica

Esta pantalla **aparece sola al entrar en ámbar**, y ésa es la vía principal. Es gratis: el equipo ya
está detenido, así que informar no cambia nada.

**No debe diseñarse como una opción más del menú**, porque entrar al menú **detiene el ciclo**
(SFTY-12 deja ambas unidades en rojo fijo). Se acabaría diagnosticando un equipo que ya dejó de hacer
aquello que se quería diagnosticar.

| Vía | Cuándo | ¿Altera el estado? |
|---|---|---|
| **Aparece sola en el ámbar** ← principal | Al perderse el enlace | **No.** El equipo ya estaba detenido |
| Buscada desde el menú | Cuando el técnico quiera | **Sí**, detiene el ciclo, como todo el menú |

Conviene recordar además que **la telemetría ya está visible durante la operación normal**: SFTY-14
muestra `RF:100% 340ms` en las pantallas de los modos, sin detener nada. Para saber si el enlace está
sano **hoy no hace falta entrar al menú**. Lo que falta es el *por qué* y el *desde cuándo* una vez
que ya se cayó, y eso es lo que aporta el reloj.

### Su público es el técnico, no el conductor

Con la pantalla a **5 m dentro del gabinete, nadie la lee de paso.** Su valor está en el momento del
diagnóstico: los tres mensajes de la fila `RX` separan **tres averías que hoy se ven todas igual**
—`nada llega` (cobertura, canal o antena), `BASURA` (cableado, línea flotando o radio atascada) y
enlace correcto—. Es exactamente la distinción que costó la jornada completa del 31/07.

**No habría evitado la avería, pero habría acortado mucho el camino hasta encontrarla.**

---

## 🌙 SFTY-20 — Operación intermitente nocturna (DISEÑO, **NO IMPLEMENTADO**)

> **Estado:** especificado el 01/08/2026. Corresponde al pendiente **N-3** y es **para lo que se soldó
> la pila**. Prioridad 4 en el orden de trabajo del roadmap: se construye, pero **no va a campo** hasta
> cerrar las antenas y la prueba de banco de la telemetría.

Requisito de origen: `MANUAL_USUARIO.md §2`. **Disparo por horario**, decidido el 31/07 — el disparo
por flujo real exigiría la cámara instalada, que hoy no lo está. La franja es **configurable**, porque
el horario no es el mismo en todas las obras.

### 🚫 El menú está lleno, y falla en silencio

```c
int y = 28 + i * 11;
if (y > 63) break;   // salvaguarda
```

Una quinta opción con el paso de 11 px cae en **`y = 72`**. La salvaguarda impide dibujarla, **pero el
cursor sí puede navegar hasta ella**: el operario llegaría a una opción invisible. Un fallo silencioso
es peor que uno ruidoso.

> ### 🗂️ ESTRUCTURA DEFINITIVA DEL MENÚ (01/08/2026)
>
> Al llegar el Modo Degradado harían falta **seis** opciones, y con el interlineado
> compactado la sexta caería en `y = 69` — fuera de los 64 px otra vez, con el mismo agravante: la
> salvaguarda no la dibujaría **pero el cursor sí llegaría hasta ella**.
>
> **La solución no es comprimir más píxeles: es un submenú.**
>
> ```
>    MENÚ PRINCIPAL                 CONFIGURACIÓN
>    ┌──────────────────┐           ┌──────────────────┐
>    │ > MANUAL         │           │ > PRUEBA ALCANCE │
>    │   AUTOMATICO     │    -->    │   AJUSTAR HORA   │
>    │   INTELIGENTE    │           │   MODO DEGRADADO │
>    │   CONFIGURACION  │           └──────────────────┘
>    └──────────────────┘
>         4 opciones                     3 opciones
> ```
>
> **El menú principal vuelve a 4 opciones**, que es exactamente el layout validado en campo y con
> 30/30 en el arnés. No hay que tocar `lcd_dibujarMenu()`: con 3 o con 4 opciones ambos menús caen en
> el caso de siempre (base 28, paso 11).
>
> **Por qué es la división correcta, y no un apaño de espacio.** `MANUAL`, `AUTOMATICO` e
> `INTELIGENTE` son **modos de operación** —lo que el operario elige a diario—. `PRUEBA ALCANCE`,
> `AJUSTAR HORA` y `MODO DEGRADADO` son **herramientas y casos especiales** que se tocan rara vez.
> **Mezclarlos en una lista plana era la causa del problema, no el número de opciones.** Comprimir
> píxeles habría tapado el síntoma dejando la causa intacta.
>
> **Y un beneficio de seguridad que no se buscaba:** con el mando de relés operando **a ciegas**, una
> ráfaga accidental de pulsos **ya no puede alcanzar el Modo Degradado ni AJUSTAR HORA** — están un
> nivel más abajo. Refuerza por estructura el requisito de *"ignorar secuencias con el menú abierto"*,
> y sale gratis.
>
> El Botón 4 desde el submenú **vuelve al menú principal**, no sale a un modo. El submenú mantiene el
> mismo estado seguro: Rojo Fijo con enlace, Ámbar sin él. **No arranca ciclos.**

---

> ### ✅ RESUELTO EN V8.6 — y de otra forma que la propuesta aquí
>
> Este apartado proponía **no tocar el menú** y acceder por pulsación larga del Botón 4. **Se
> descartó**, porque el análisis de partida era incompleto: daba por fijo el arranque en `y = 28` y
> concluía que a 9 px la quinta caía en `y = 64`, un píxel fuera.
>
> **Moviendo también la base**, sí cabe: `lcd_dibujarMenu()` usa ahora **base 24 y paso 9** cuando hay
> 5 opciones ⇒ `y = 24, 33, 42, 51, 60`. Con 4 o menos conserva **exactamente** el layout validado en
> campo (base 28, paso 11). Confirmado en el arnés: **42/42**.
>
> **Lo que se pierde y hay que compensar.** La pulsación larga existía para que *un gesto deliberado
> impidiese cambiar el reloj sin querer*. Como opción de menú, **una ráfaga accidental de pulsos desde
> el mando de relés puede entrar a AJUSTAR HORA y, con cuatro pulsos más, confirmar una hora errónea
> marcada como válida** — justo el veneno que SFTY-18 quiere evitar.
>
> Por eso **ignorar las secuencias del mando mientras el menú está abierto deja de ser un
> "afinamiento pendiente" y pasa a ser requisito** (ver SFTY-21).

### Piezas

| # | Pieza | Nota |
|---|---|---|
| 1 | Pantalla **AJUSTAR HORA** | `HH:MM` · B1 = + · B2 = − · B3 = siguiente · B4 = salir |
| 2 | Pantalla **FRANJA NOCTURNA** | hora de inicio y hora de fin |
| 3 | **Persistencia de la franja** | ⚠️ ver abajo — la pieza que puede descarrilar el resto |
| 4 | Estados de luz nuevos | Maestro ámbar intermitente · Esclavo rojo intermitente |
| 5 | Comando RF `CMD_GO_NOCHE` | `0x07` está libre (`0x01`–`0x06` ocupados) |

### ⚠️ La franja no sobrevive al apagón

`reloj.cpp` guarda hoy la franja **solo en RAM**: se va la luz y vuelve al valor por defecto. Es
inaceptable para algo que el operario configura en obra.

`STM32duino RTC` **no expone los registros de respaldo** — comprobado al compilar, `getBackupRegister`
no existe en la versión 1.9.0. En el STM32F1 son accesibles directamente (`BKP->DR1..DR10`, previa
habilitación de escritura en el dominio de respaldo), unas diez líneas. **Es el trabajo menos obvio de
todo el conjunto y el que más fácil se pasa por alto al planificar.**

### Reglas de seguridad, no negociables

| # | Regla | Por qué |
|---|---|---|
| 1 | `reloj_enHora() == false` ⇒ **nunca** entrar en modo nocturno | Ya construido en SFTY-18. Un reloj sin poner en hora activaría el modo a deshora |
| 2 | Entrar y salir del modo pasa por el **despeje todo-rojo** (SFTY-4) | No se salta de verde a intermitente |
| 3 | Si el Esclavo no confirma `CMD_GO_NOCHE` ⇒ ámbar en ambos | Se degrada al fallo conocido, no a un estado a medias |

### Un estado propio, aunque se vea igual

El Maestro en modo nocturno parpadea ámbar, **visualmente idéntico a `S_FALLO`**. Aun así necesita
estado propio: reutilizar `S_FALLO` haría que la telemetría y los diagnósticos **reporten una avería
que no existe**. Hoy el enum es `{ S_ROJO, S_VERDE, S_AMARILLO, S_FALLO }`.

### Cómo se valida — y el requisito previo

`simulador_sistema_v7_6.py` ya tiene **eje de tiempo** (`avanzar_simulacion(duracion_s, dt=0.1)`, con
`current_time` en todas las máquinas de estado) y modela las luces como estados con nombre. Simular
este modo es añadir un reloj al modelo y una prueba que compruebe:

1. Ambas unidades **entran y salen de la franja en fase**.
2. La transición **pasa por el despeje todo-rojo**, no salta.
3. Con `reloj_enHora() == false`, **nunca entra** — ni al principio ni al cruzar la franja.

> ⚠️ **Requisito previo (N-12).** El simulador tiene el despeje todo-rojo escrito a mano como `15.0`,
> y **no lo lee del C++**. Como ese despeje *es* el margen de seguridad al entrar y salir de este
> modo, validar SFTY-20 contra un valor que el modelo no vigila no demostraría nada. Hay que extender
> primero la lectura anti-deriva del bloque 0.

---

## 🚧 SFTY-19 — Operación autónoma al perder el radio (DISEÑO, **NO IMPLEMENTADO**)

> **Estado:** especificado el 31/07/2026, pendiente de construir, simular y validar con el funcional.
> **Nada de esto está en el firmware todavía.** El comportamiento actual sigue siendo SFTY-6:
> sin enlace ⇒ ámbar intermitente en las dos puntas.
>
> ## ❌ SUSTITUIDA POR SFTY-21 (01/08/2026)
>
> Esta regla planteaba **entrada automática** al modo autónomo tras N minutos sin enlace. **Se
> descartó en reunión con el cliente.** El motivo es el de siempre: sin radio nadie puede confirmar
> que la otra punta esté viva, y una máquina no debe decidir sola operar a ciegas.
>
> **Lo aprovechable de este diseño —la deriva entre relojes, el despeje todo-rojo como colchón y las
> condiciones que impiden entrar— se trasladó a SFTY-21**, ahora con activación manual verificada.
>
> Se conserva el texto por dos razones: deja constancia de **por qué se descartó la vía automática**,
> y el análisis de sincronización relativa sigue siendo válido si algún día se retoma.

### El problema que resuelve

Con el radio averiado, hoy el sistema entra en ámbar intermitente. Es correcto y es el *fail-safe*
estándar — devuelve la decisión al conductor — pero deja el paso de obra sin regular. Se pide que
el equipo **siga alternando verde y rojo por su cuenta** durante una pérdida de enlace acotada.

### Por qué es delicado

Este sistema regula **un carril alternado**. Cuando el Maestro da verde, el Esclavo **tiene** que estar
en rojo. Si las dos unidades cuentan cada una por su lado y se separan lo suficiente, **hay verde
simultáneo en las dos puntas y dos vehículos entran de frente al tramo.** No es un defecto cosmético:
es la única forma en que este equipo puede matar a alguien.

### Reglas que hacen viable el modo

| # | Regla | Por qué |
|---|---|---|
| 1 | Mientras hay enlace, el Maestro comunica periódicamente **en qué punto del ciclo va** | Da a las dos unidades un origen común |
| 2 | Al perderse el enlace, cada unidad continúa **desde el último punto sincronizado** | La separación arranca en ~0, no en un valor arbitrario |
| 3 | El despeje **todo-rojo** debe superar con holgura la separación acumulada máxima | Es el margen que absorbe la deriva |
| 4 | **Límite duro de tiempo sin enlace** ⇒ ámbar intermitente | Más allá no se puede acotar la deriva **ni saber si la otra punta sigue viva** |
| 5 | Unidad que **arranca o se reinicia sin haber sincronizado nunca** ⇒ ámbar, sin excepción | Sin origen común no hay modo autónomo posible |

Las reglas 4 y 5 **no son opcionales**. Son lo que separa "modo autónomo" de "verde en las dos puntas".

### Hallazgo de diseño: esto NO necesita el RTC

La sincronización es **relativa** al último mensaje del Maestro, no a la hora absoluta. Para eso basta
el contador de milisegundos del micro. Consecuencias prácticas:

- **No hace falta pila en la tarjeta del Esclavo.** El Esclavo no necesita saber la hora.
- El RTC (SFTY-18) sigue siendo necesario para la **operación intermitente nocturna**, que es otra
  función distinta y quedó **aplazada** a petición del cliente: el horario no es igual en todas las obras.

### Parámetros por decidir

- **Tiempo máximo sin enlace antes de rendirse a ámbar.** Propuesta: **30 min** — suficiente para que
  el funcional haga la prueba desconectando una antena, y corto para que un radio muerto de verdad no
  deje el cruce operando a ciegas toda la noche.
- **Ciclo que corre en modo autónomo.** Propuesta: reusar el tiempo de verde ya configurado en
  **MODO AUTOMÁTICO**, en vez de introducir un parámetro nuevo.

### Criterio de aceptación

El simulador ya modela pérdida de enlace. La validación **debe medir la separación entre las dos
unidades** a lo largo de la ventana sin radio y demostrar que el todo-rojo la cubre — no basta con
afirmarlo. Mientras esa medida no exista, este modo **no va a campo**.

---

## 📶 SFTY-24 — Enlace de respaldo por datos moviles entre dos telefonos (DISENO, **NO IMPLEMENTADO**)

> **Estado:** propuesto el 26/08/2026 a raiz de una observacion de operacion. **Nada de esto esta en
> el firmware ni en la app.** El enlace entre puntas sigue siendo unicamente el radio LoRa E90-DTU, y
> la perdida de ese enlace sigue cayendo a ambar intermitente (SFTY-6).

### De donde sale la idea

La app de campo ya corre en un telefono que, fuera de la montana, **tiene datos**. Hoy ese canal no
se usa para nada: la app habla Bluetooth con el poste que tiene delante y se acaba ahi. La pregunta
es si **dos telefonos, uno en cada punta, podrian hablarse por datos** y darle al sistema un segundo
camino entre extremos que no dependa del radio.

### Lo que resolveria, y es real

| | |
|---|---|
| **Saber si la otra punta esta viva** | Hoy, con el radio caido, la unica forma de saber que hace el otro extremo es caminar el tramo. Un tramo de obra puede tener cientos de metros y trafico dentro |
| **Diagnostico de las dos cajas negras en un sitio** | `$ALARM` y `$EVENT` de ambos postes juntos, con marca de tiempo, para la interventoria |
| **Courier RTC instantaneo** | El Modo Courier ya existe y es la version *sneakernet* de esto: el tecnico lleva la hora andando. Por datos seria inmediato |
| **Confirmar una maniobra antes de hacerla** | El operario del Km 12 ve en su pantalla que el otro extremo esta en rojo **antes** de pedir el cambio |

### 🛑 La regla, y no es negociable

> **El enlace por datos puede OBSERVAR y puede DOCUMENTAR. No puede AUTORIZAR.**

Ninguna trama que llegue por ese canal puede provocar un verde, acortar un todo-rojo, ni sacar a un
equipo del ambar. Como mucho puede **pedir** algo que el radio LoRa tendra que confirmar por su
cuenta, con su CRC y su ACK, exactamente igual que si la peticion hubiera venido de un boton.

Las razones son las mismas por las que SFTY-19 descarto la entrada automatica, mas dos propias:

1. **La cobertura celular es justo lo que no hay donde se usan estos equipos.** El manual de la app
   lo dice como virtud —*"100% Offline, opera en montana sin internet ni 4G"*—, y tiene razon. Un
   canal que existe **a veces** es peor que uno que no existe nunca: el que no existe no engana a
   nadie, el intermitente ensena a confiar y falla el dia que importa.
2. **Latencia no acotada.** Datos moviles admiten segundos de retardo, reintentos y caidas
   silenciosas. El paso alternado necesita saber **ahora** si el otro lado esta en rojo, no hace
   cuatro segundos. Un verde concedido sobre un mensaje retrasado es verde simultaneo en las dos
   puntas, que es la unica forma en que este equipo puede matar a alguien.
3. **Cuatro puntos de fallo nuevos, ninguno bajo control del equipo:** dos baterias de telefono, dos
   personas, dos operadoras y una nube.

### Parametros por decidir

- **Transporte.** Un servidor propio implica infraestructura y cuenta que alguien paga; un canal
  directo entre telefonos evita el servidor pero complica el emparejamiento en campo. Sin decidir.
- **Que se sincroniza.** Propuesta minima: solo `$STATUS`, `$ALARM` y `$EVENT`, de lectura. Nada de
  comandos en la primera version — y si algun dia los hay, pasan por la radio.
- **Que ve el tecnico cuando el canal se cae**, que sera lo normal. Propuesta: la pantalla del otro
  extremo se marca **caducada con su antiguedad en segundos**, nunca se congela mostrando el ultimo
  valor como si fuera actual. Un dato viejo sin fecha es peor que ningun dato.

### Criterio de aceptacion

Antes de escribir una linea: **medir la cobertura real en los tramos donde opera el equipo**, con el
telefono que usa el tecnico, a lo largo de una jornada. Si la cobertura no esta, esta funcion es una
pantalla bonita que no se enciende cuando hace falta. Y la medida va al repositorio, no al recuerdo
de nadie.

---

## 🏷️ SFTY-25 — Identidad de tramo en la telemetria (RIESGO ABIERTO, **NO IMPLEMENTADO**)

> **Esto no es una mejora futura: es un agujero de hoy**, y aparece en cuanto hay mas de un par de
> semaforos en la misma via, que es el caso de uso real.

### El escenario

Una via en obra lleva **varios pares Maestro/Esclavo** —Km 12, Km 24, Km 31— y **un solo telefono**
recorriendolos. La app ya distingue el **rol** (`NODE:MAESTRO` / `NODE:ESCLAVO`) porque la trama lo
trae. Pero el rol no dice **de que par** es ese equipo.

### Por que el rol no basta

**El rol no es el sentido, y el sentido no es la instalacion.** Un Maestro puede estar en el extremo
norte de un tramo y en el sur del siguiente; "Sentido 1 = Sisga -> Bogota" es una propiedad de **como
se planto el poste**, no de que firmware lleva dentro. Dos pares distintos emiten hoy tramas
**indistinguibles**:

```
   $STATUS,NODE:MAESTRO,MODO:MANUAL,ESTADO:...   <- ¿el del Km 12 o el del Km 24?
```

El fallo concreto: el tecnico esta parado en el Km 24, su telefono se engancha por Bluetooth al par
del Km 12 —que sigue en rango, o fue el ultimo emparejado—, pulsa **DAR PASO A SENTIDO 1** creyendo
que gobierna el poste que tiene delante, y **abre un verde a 12 km de distancia** en un tramo que no
esta mirando.

### Lo que hace falta

| # | Pieza | Donde |
|---|---|---|
| 1 | Campo `ID:` en `$STATUS`, `$ALARM` y `$EVENT` con el identificador de tramo | `bluetooth.cpp`, las tres tramas |
| 2 | Ese identificador configurable desde el menu del LCD y guardado en respaldo | `menu.cpp`, `respaldo.cpp` — cambia la `FIRMA` |
| 3 | La app muestra el `ID:` **grande y permanente**, no en un submenu | App movil |
| 4 | Etiqueta fisica visible en el poste con el mismo identificador | Procedimiento de instalacion |
| 5 | La app **rechaza** un comando si el `ID:` de la conexion no es el que el operario tiene seleccionado | App movil |

El punto 4 no es burocracia: es lo que permite al operario **comparar lo que ve en la pantalla con lo
que tiene delante**. Sin esa comparacion, los otros cuatro puntos solo mueven el error de sitio.

### Criterio de aceptacion

Dos pares encendidos a la vez en el banco, un solo telefono, y **demostrar que un comando dirigido al
par A no llega al par B** — ni siquiera cuando el operario lo intenta a proposito. Mientras esa
prueba no exista, el manual de la app debe advertir que **solo se opera con un par encendido a la
vez**.


---

## 🔌 SFTY-26 — Expansor I2C para acabar con la disputa por los dos pines libres (DISENO, **NO IMPLEMENTADO**)

> **Estado:** decidido el 26/08/2026 como salida de `N-57`. **No es una eleccion entre reloj y
> camaras: son las dos, sobre el mismo bus, y el firmware detecta que hay montado.**

### El problema, en una linea

La placa tiene **dos pines libres** y **tres cosas** los quieren: el `DS3231` necesita dos (`SDA`,
`SCL`) porque `N-37` cerro en banco con el cristal `Y2` **muerto**, y las camaras necesitan al menos
uno. No caben **como pines sueltos**. Si caben **como bus**.

### La salida: una placa hija de expansion, siempre la misma

`PB0` y `PB8` dejan de ser "dos entradas" y pasan a ser **un bus I2C**. De ahi cuelga una placa hija
con dos modulos, y **el cableado es identico en todas las unidades** — como el del modulo Bluetooth:
un solo diagrama, una sola forma de conectarlo, sin variantes que el instalador tenga que decidir.

```
   PB0 (SDA) --+--------------+---------------
   PB8 (SCL) --+--+        +--+--+
                  |        |     |
              [DS3231]   [PCF8574]  --> 8 entradas digitales
               0x68        0x20         (camaras 1..4, y sobran 4)
             se monta      SIEMPRE
             SOLO si el    montado
             Y2 esta
             muerto
```

**El `PCF8574` va siempre.** Es lo que hace que las camaras entren por el bus y que `PB0`/`PB8` dejen
de estar en disputa para siempre: cualquier entrada futura entra por el expansor sin volver a abrir
esta discusion.

**El `DS3231` va solo donde haga falta.** Y ahi esta la gracia del diseño: **no hay que saberlo de
antemano.**

### Un solo firmware para los dos casos: se detecta al arrancar

Al arrancar, el micro **escanea el bus** y decide su fuente de hora. Nada de compilar dos versiones
ni de decidir por unidad antes de fabricar:

```
   arranque
     |
     +-- responde 0x68 en el bus ?  --SI-->  RELOJ = DS3231 externo
     |                              --NO-->  RELOJ = cristal Y2 interno
     |
     +-- responde 0x20 en el bus ?  --SI-->  camaras leidas por el expansor
                                    --NO-->  camaras leidas por PB0 directo (compatibilidad)
```

Se programa todo una vez. Una tarjeta con el cristal sano funciona sin placa hija; si el cristal
falla, **se le enchufa el modulo y arranca usandolo, sin recompilar nada**.

### Y AVISA. Un respaldo silencioso seria el defecto, no la solucion

Esto es lo que `N-12` dejo escrito en este repositorio: *un valor por defecto silencioso derrota el
proposito*. Asi que la fuente de hora **nunca se elige en silencio**:

| Donde | Que dice |
|---|---|
| LCD, pantalla de estado | `RELOJ: INTERNO` · `RELOJ: DS3231` · `RELOJ: SIN FUENTE` |
| Telemetria `$STATUS` | campo `CLK:INT` / `CLK:EXT` / `CLK:NONE` |
| Caja negra | `$ALARM` al detectar que el cristal interno **acepta la hora y no avanza** |
| App | avisa al tecnico y lo registra: **el censo se construye solo** |

La deteccion de "cristal muerto" no es *"no esta en hora"* —eso le pasa a una unidad sana recien
encendida—: es **que la hora no AVANCE**. Se mide comparando dos lecturas separadas, que es
exactamente lo que hizo `N-37` a mano y lo que la app puede hacer sola en cada conexion.

### Lo que hay que medir ANTES de comprometerlo

**El I2C es por software**, no por hardware: los dos puertos nativos estan ocupados (`PB6`/`PB7` la
LCD, `PB10`/`PB11` el RS-485). Meter dos esclavos en un bus bit-bang, en un micro que ya hace
bit-bang del LCD y lleva el `IWDG` armado, **no es gratis**:

| Riesgo | Como se mide |
|---|---|
| La lectura del expansor compite con el refresco del LCD | Peor caso del ciclo de `loop()` con las dos cosas activas |
| Una transaccion I2C larga acerca el `IWDG` a su ventana | Instrumentar el margen del watchdog, no estimarlo |
| Latencia de deteccion de camara | El contacto de rele dura ~1 s: el sondeo tiene que caber con holgura |

**Ninguno de los tres se resuelve desde el PC.** Van a banco.

### Que se pide, y como se conecta

Los dos son modulos comerciales de breakout, de los que se compran hechos:

| | Modulo | Direccion I2C | Cuantos |
|---|---|---|---|
| Reloj | **`DS3231` (placa `ZS-042`)**, el azul de siempre | `0x68` (+ `0x57` de su EEPROM `AT24C32`) | uno por tarjeta **con el cristal muerto** |
| Expansor | **`PCF8574`** breakout de 8 E/S, con `A0`-`A2` a GND | `0x20` | uno por tarjeta, **siempre** |

No colisionan: `0x68`, `0x57` y `0x20` son direcciones distintas. Y los dos cuelgan de los mismos
dos hilos:

```
       TARJETA STM32                 PLACA HIJA DE EXPANSION
  +---------------------+        +----------------------------------+
  |  3.3 V  ------------+--------+--> VCC   [DS3231]   [PCF8574]     |
  |  GND    ------------+--------+--> GND      0x68       0x20       |
  |  PB0    ------------+--------+--> SDA  (los dos en paralelo)     |
  |  PB8    ------------+--------+--> SCL  (los dos en paralelo)     |
  +---------------------+        |                                   |
                                 |  PCF8574 P0 <-- Camara 1 rele 1A  |
                                 |  PCF8574 P1 <-- Camara 2 rele 1A  |
                                 |  (P2..P7 libres)      1B --> GND  |
                                 +----------------------------------+
```

### Tres avisos electricos que van al manual, no al aire

1. **`VCC` a 3,3 V. NUNCA a 5 V.** En el STM32F103, `PB0` es un pin con entrada analogica y **no es
   tolerante a 5 V** (los `FT` son otros). Los modulos suelen llevar sus resistencias de pull-up
   hacia `VCC`: si `VCC` fuese 5 V, esas pull-ups meterian 5 V en `PB0` por el bus. **Verificar en el
   datasheet del micro antes de energizar**, y corregir el Manual 11, que hoy dice *"3.3V (o 5V)"*.
2. **Dos modulos = dos juegos de pull-up en paralelo.** ~4,7 k cada uno quedan en ~2,3 k. A la
   velocidad baja de un I2C por software no deberia estorbar, pero **es una medida de banco**, no una
   suposicion: si el bus no arranca, retirar las pull-up de uno de los dos modulos es lo primero.
3. **El `ZS-042` trae diodo y resistencia de carga.** Con `LIR2032` recargable, bien. Con `CR2032` no
   recargable hay que **levantar `D1` o `R1`**. Ya esta en el Manual 11 y sigue valiendo — y no
   confundirla con la `CR2032` de `VBAT` de la placa madre, que es otra pila y otra regla (`R5`).

### El censo se construye solo, como en Baliza

No hace falta una pantalla de diagnostico ni un procedimiento aparte. **La app de Baliza ya hace lo
que se necesita: al arrancar toma la hora del celular y se la manda al controlador.** Con eso el
diagnostico sale gratis del uso normal:

```
  Al conectar:   la app manda la hora del telefono   (SET_RTC)
  En cada $STATUS: compara HORA: contra la hora del telefono

     desfase ~0                  -->  cristal SANO
     desfase que CRECE           -->  cristal MUERTO o fuera de tolerancia
     no acepta el ajuste         -->  no arranca: revisar pila / R5 / N-45
```

La clave es **medir si la hora avanza, no si esta puesta**: `reloj_enHora() == false` significa "no
esta en hora", que tambien le pasa a una unidad sana recien encendida. Confundir las dos cosas hace
comprar modulos que no hacen falta.

**Y funciona sobre unidades con el LCD muerto**, que es justo el agujero que `N-37` no pudo cubrir en
el Esclavo: *"con su pantalla muerta (N-22) no hay forma de comprobarlo desde el menu"*.

Cada tecnico que use la app en campo alimenta el censo sin proponerselo. Cuando haya numeros, se
decide cuantos expansores y cuantos `DS3231` se compran.

### Criterio de aceptacion

Censo de las unidades **antes** de comprar nada, y las tres medidas de la tabla en banco antes de dar
el expansor por bueno.

---

## 🔗 SFTY-27 — Matricula de pareja: quien obedece a quien (DISENO, **NO IMPLEMENTADO**)

> **Estado:** disenado el 26/08/2026. **Nada esta implementado.** Hoy `RF_Packet` no lleva
> direccionamiento de ningun tipo —`{msgID, command, param, crc}`—, asi que dos parejas dentro del
> alcance de la radio (**1 a 3 km**, justo la distancia a la que conviven dos frentes de obra) **se
> mandan ordenes entre si**.

### Las tres piezas

**1. `SERIE` — la identidad, y no se puede editar.**
Derivada del **UID de 96 bits que el STM32F103 trae grabado de fabrica** (`0x1FFFF7E8`), reducida a
4 hex: `7A3F`. Unica, no falsificable, sin reloj, sin escribir memoria y **sin base de datos de
fabrica**. El firmware hoy **no la usa**.

> Se descarto acuñar el ID con fecha+hora al arrancar: en un equipo recien salido de taller
> `reloj_enHora()` es **falso**, asi que todas las unidades nacerian con el mismo sello.

**2. `PAIR` — el filtro, 2 bytes en la trama de radio.**
El Maestro **nace** con `PAIR` = su `SERIE`; no hay nada que configurarle, nunca. El Esclavo nace en
blanco (`0000` = sin adoptar) y **no obedece a nadie**, asi que se queda en ambar. La pregunta que se
hace la radio no es *"quien me habla"* sino *"esto es de mi pareja"*, y para eso basta un codigo
comun en los dos.

**3. `SITIO` y `SENTIDO` — las etiquetas, y esas SI se editan.**
El poste se muda cada dia; su etiqueta tiene que poder cambiar. Confundir identidad con etiqueta es
el error: **lo que no puede cambiar es quien es, no donde esta.**

### La matricula se hace por Bluetooth, no por radio

El motivo es el alcance:

```
  RADIO LoRa   1 - 3 km    <- "buscar" encontraria Maestros de OTROS frentes de obra
  BLUETOOTH    10 - 15 m   <- solo encuentra lo que tienes AL LADO
  CABLE A/B    2 nodos     <- literalmente no hay nadie mas en el bus
```

**El alcance corto del Bluetooth, que para telemetria es una limitacion, aqui es la garantia:** es
fisicamente imposible matricular por error con un Maestro que esta a 12 km. La app lee el `PAIR` del
Maestro, el tecnico camina al otro poste, y la app lo escribe con `SET_PAIR`. **Nadie teclea el
codigo** — si un humano lo escribe, un dedazo crea dos parejas con el mismo numero, que es el unico
escenario de todos que si es peligroso.

Variante de taller, mas robusta todavia: unir `A`/`B` de las dos borneras `RS485_OUT`. En un bus de
dos nodos no existe un tercero que pueda contestar.

**La app no da por bueno su propio envio:** relee la trama del equipo y compara lo que quedo escrito.
Y el LCD del Esclavo muestra `MI MAESTRO: 7A3F` de forma permanente, para poder verificar la
matricula **sin telefono**, mirando los dos postes.

### El Esclavo pide; no ordena

El operario del PMT se coloca en el extremo que haga falta, y **no tiene por que saber que es un
maestro**. Eso se resuelve sin darle mando al Esclavo:

| | |
|---|---|
| **ORDEN** — *"ponte en verde"* | el Esclavo la ejecutaria. NO |
| **PETICION** — *"hay demanda aqui"* | viaja al Maestro, que decide, aplica el todo-rojo y ordena. SI |

**El mecanismo ya existe:** `CMD_DEMANDA` (`0x11`) se añadio para la camara 3. Un boton del funcional
es exactamente lo mismo que un coche detectado: **una demanda**. No hace falta protocolo nuevo; hace
falta darle un segundo origen.

Con dos funcionales, uno en cada extremo, **el Maestro serializa**: ninguno concede nada, los dos
piden. Que pulsen a la vez tiene que ser aburrido, y asi lo es.

Y en la interfaz **no aparece "MAESTRO" ni "ESCLAVO"**: aparece el sentido que el equipo lleva
guardado. El boton dice **"solicitar paso por este lado"**, porque el funcional esta mirando el poste.

### Todos los fallos acaban en ambar

| Que sale mal | Donde acaba |
|---|---|
| Esclavo sin matricular | No obedece a nadie: **ambar**. En pantalla, `SIN ADOPTAR` |
| Matriculado con el Maestro equivocado | El Maestro correcto se queda sin Esclavo: **ambar** |
| Dos parejas en la misma via | Codigos distintos por construccion: **se ignoran** |
| App conectada al poste que no toca | El Esclavo no acepta ordenes de trafico **de nadie** |

**Ninguna salida va a verde.** Eso es lo que hace que una matricula equivocada no pueda hacer daño:
solo puede dejar sin servicio.

### Consecuencia operativa que va al manual de mantenimiento

Sustituir un **Esclavo** es trivial. Sustituir un **Maestro** obliga a re-matricular, porque su
`PAIR` es su identidad y el repuesto es otro chip. Es un gesto de un minuto con los dos equipos
delante, **pero si no esta en el manual alguien se encontrara un Esclavo en ambar sin saber por que**.
La pantalla debe decirlo: `SIN ENLACE - MI MAESTRO ES 7A3F`.

La re-matricula debe ser posible pero **deliberada**: PIN, confirmacion en pantalla y registro en la
caja negra con fecha. Es mantenimiento, no un ajuste.

### Criterio de aceptacion

Pack `costura_08_pareja` con su control negativo: una trama con `PAIR` ajeno **debe** ser descartada,
y el pack tiene que haberse visto fallar con el filtro desactivado a proposito. Y en banco: **dos
parejas encendidas a la vez**, demostrando que una orden dirigida a la pareja A no la ejecuta la B.

---

---

## 👁️ SFTY-29 — Presencia en el tramo: un VETO, nunca un atajo (**DISEÑO**, no implementado)

> **Estado:** especificado el 27/08/2026. Sustituye a la idea de *"contar vehiculos"* que arrastraban
> los manuales bajo el nombre de *"camara de umbral"* (N-59), y que se retiro de V9.0 por cara y
> fragil. **Esto es otra cosa, y es mejor.**

### La distincion que lo cambia todo

Las camaras 2 y 4 no cuentan: **detectan presencia**. Y esa presencia no autoriza a ir mas deprisa,
solo puede decir *"todavia no"*.

| | conteo *(descartado)* | **presencia** *(esto)* |
|---|---|---|
| que viaja por radio | un mensaje por cada coche que entra y otro por cada uno que sale | **un bit**: sigue ocupado / libre |
| que autoriza | **ACORTAR** el todo-rojo | **RETRASAR** el verde |
| si la deteccion falla | se acorta un despeje que no debia acortarse -> **dos vehiculos de frente** | se cae al temporizador de siempre -> **nada peor que hoy** |
| si detecta de mas | — | se espera un poco mas |

**Los dos modos de fallo caen del lado seguro.** Por eso esta regla no debilita el todo-rojo: lo
refuerza. Y por eso el conteo se descarta y esta version no.

### Y la objecion de la radio desaparece

El enlace va a **2,4 kbps y es semiduplex**: por ese unico canal viajan las ordenes que mueven las
luces, sus acuses, el latido y la hora. Contar vehiculos obligaba a decenas de mensajes por minuto
en hora punta, compitiendo con el `CMD_GO_RED`. **Un bit no compite con nada** — y ademas no hace
falta ninguna trama nueva:

```
   struct RF_Packet { uint8_t msgID; uint8_t command; uint8_t param; uint8_t crc; };
                                                       ^^^^^
   MEDIDO el 27/08: el Esclavo manda CMD_ACK_RED con programarRespuesta(CMD_ACK_RED),
   o sea param = 0 por defecto, y el Maestro (coordinador.cpp:743) NO LO LEE.
   El byte esta libre, y llega EXACTAMENTE en el instante que importa: cuando el
   Esclavo confirma que ya esta en rojo y empieza a correr el todo-rojo.
```

**Coste en aire: cero.** Cero comandos nuevos, cero bytes nuevos, cero cambios en el formato.

### La maquina, y su unico peligro real

```
   El Maestro recibe CMD_ACK_RED
        |
        +-- param bit0 = 0  (tramo libre)  --> todo-rojo normal, y al acabar, VERDE
        |
        +-- param bit0 = 1  (aun ocupado)  --> EXTIENDE el todo-rojo
                                               |
                                               +-- se libera antes del tope --> VERDE
                                               |
                                               +-- se llega al TOPE ---------> VERDE IGUAL
                                                                               + $ALARM
```

> 🔴 **EL TOPE NO ES UN DETALLE: ES LA REGLA.** Barro en el lente, un camion aparcado en el punto de
> vigilancia o un sensor pegado en activo, y **el cruce se congela para siempre** esperando que se
> libere. Un enclavamiento sin tope no es mas seguro: es un semaforo colgado, con cola en las dos
> puntas y nadie entendiendo por que.
>
> La regla es: **extender hasta un maximo configurable; al llegar, cambiar igual y levantar alarma**
> en la caja negra (`$ALARM,EVENTO:PRESENCIA_PEGADA,...`). El operador se entera de que hay un sensor
> averiado **sin que el cruce deje de regular**.

### La segunda funcion, que es local y no toca la radio

**No bajar la pluma sobre un vehiculo.** El mismo sensor, leido por el propio poste, impide bajar la
barrera mientras haya algo debajo — igual que la fotocelula de un porton. No interviene el Maestro, no
viaja nada por radio, y es la funcion que evita el dano material y la reclamacion.

Aqui el fallo seguro va al reves que arriba, y conviene tenerlo claro:

| el sensor se queda… | que pasa | es seguro? |
|---|---|---|
| **pegado en ACTIVO** | la pluma **no baja** | ✅ si: la barrera deja de proteger, pero **la luz sigue regulando**. Con alarma |
| **pegado en INACTIVO** | la pluma baja sobre un coche | ❌ no. Por eso el sensor va **normalmente cerrado** si el modelo lo permite: un cable cortado se lee como "hay algo" |

### Donde entra fisicamente

Cada tarjeta necesita **una segunda entrada** —la primera la ocupa la camara de demanda en `J14`—.
Tres caminos, y **lo decide la pregunta 5 del acta de banco**:

**Decidido el 27/08: la entrada es `PA11` (pad 32 del `U1`).**

Se midio el cobre del `.kicad_pcb`, y lo dice el propio enrutador: `PA11`, `PA12`, `PA15` y `PC13`
salen como `unconnected-(U1-PA11-Pad32)` y equivalentes — **pads sin una sola pista**. En la misma
pasada los ocupados aparecen con su red (`PB0 -> /Puerta`, `PB2 -> /Motor`, `PB1 -> /Buzzer`), asi que
el metodo esta validado y no es una suposicion.

**Por que `PA11` y no otro:** es tolerante a 5 V, **no depende de que el JTAG este desactivado** -a
diferencia de `PA15`- y no vive en el dominio de respaldo con sus limitaciones, como `PC13`. `PA12`
queda de reserva por si el pad de `PA11` resulta inaccesible en la tarjeta real.

**Como se cablea, y por que asi:**

```
   pad 32 (PA11) --[ hilo AWG30, fijado con kapton ]--> via muerta de J16 --> bornera
                                                          (cual, lo dice el multimetro)

   Sensor NORMALMENTE CERRADO a masa. En reposo: contacto cerrado, pin a masa (LOW).
   Con vehiculo: el contacto ABRE y el pin sube por el pull-up interno (HIGH = presencia).
```

**Un cable cortado se lee como "hay algo"** — el todo-rojo se extiende y la pluma no baja. El fallo
del cableado cae del lado seguro, y el TOPE de mas abajo impide que un sensor averiado congele el
cruce. Esa combinacion -NC mas tope- es lo que hace que esta entrada no pueda empeorar nada.

> ⚠️ **Correccion del 27/08:** una version anterior decia que `J16` llevaba la pantalla y los botones.
> **Medido: `J16` es el conector de la BOTONERA** -cuatro botones con su 3,3 V y los 12 V- y le sobran
> dos o tres vias muertas. La pantalla va por otro sitio. La conclusion no cambia -hay vias libres y
> hay pines libres-, pero el dato si.

| alternativa | coste | deja libre |
|---|---|---|
| **`PA11` + hilo a via muerta de `J16`** *(elegida)* | un hilo | `PA12`, `PA15`, `PC13` |
| pad de `PB8`, retirando `R16` y el LED `D5` | un hilo + 2 componentes | los cuatro pines libres |
| placa hija con `PCF8574` | chip + bus bit-bang (~800 B) + un modo de fallo nuevo | **6 E/S** para el reloj y lo que venga |

### Lo que hay que medir ANTES de escribir una linea

- **El flash.** El bit y la maquina de extension son pocas decenas de bytes; la alarma y el texto de
  pantalla, mas. Quedan **4.684 bytes** en el Maestro: se compila con y sin, y se compara el acta. Sin
  esa cifra, cualquier estimacion de aqui es una intuicion.
- **La polaridad del sensor**, como en N-67: la manda la placa, no el gusto de nadie.

### El pack que lo vigilara, y la comprobacion que nadie escribe

`presencia_01_veto` tendra que exigir, sobre el C++ real:

1. Que la presencia **solo extienda**: que no exista ningun camino donde acorte el todo-rojo. Es la
   propiedad central, y se mide sobre las escrituras de pin como SFTY-2.
2. Que el bit se lea del `param` de `CMD_ACK_RED` y de ningun otro sitio.
3. **Que el tope funcione** — y este es el control negativo que se olvida siempre: con la presencia
   forzada a activa **para siempre**, el cruce **tiene que cambiar igual** al llegar al maximo y
   emitir la alarma. Un pack que solo pruebe el caso bueno estaria certificando el cuelgue.
4. Que la pluma no baje con presencia, y que un sensor ausente no la deje bajar sola.

### Lo que esta regla NO hace

No cuenta vehiculos, no acorta el despeje, no sustituye al temporizador y **no se apoya en la radio
para nada critico**: si el bit no llega, el Maestro hace exactamente lo que hace hoy. Esa es la razon
de que se pueda añadir sin volver a discutir el todo-rojo entero.

---

## 🚧 SFTY-28 — Talanquera acoplada al estado del semaforo (**IMPLEMENTADA la regla; abiertas las decisiones de operacion**)

> **Estado:** anotado el 26/08/2026 y **construido el 27/08** en las dos puntas.
>
> **Lo que ya corre:** la orden sale de `escribirPines()` —la misma puerta que las lamparas— y
> sigue al `verde` YA enclavado; el arranque la deja cerrada; el nivel de reposo del pin es el
> de CERRAR, de modo que un equipo apagado no deja la via abierta. **Coste: +24 bytes por
> punta**, sin drivers ni bus.
>
> **Lo que lo mide, en dos planos distintos:** el arnes del automatico vigila en CADA tick de
> los nueve bloques que la pluma nunca este arriba con los dos verdes apagados —sobre lo que el
> `semaforo.cpp` real escribio en el pin, con su control negativo al lado—, y el pack
> `barrera_03_talanquera` fija la estructura que el arnes no puede ver: que **ningun otro**
> `.cpp` de las dos puntas escriba ese pin —censando el directorio, no una lista—, que la orden
> viva dentro de `escribirPines()` y no en una funcion suelta, y que las dos puntas lo hagan
> igual. Visto caer a `68/69` con la pluma forzada a ABRIR en el `.cpp` real.
>
> 🟡 **Lo que sigue abierto, y NO lo decide el firmware:** que hace la pluma con el ambar
> intermitente de SFTY-6 (sin enlace). Hoy queda **abajo** —es lo conservador: solo sube con
> verde confirmado—, pero *«cerrar la via por completo»* frente a *«dejar pasar con
> precaucion»* es decision del cliente y del PMT. Cambiarla es **una linea** en
> `escribirPines()`, y el dia que se cambie hay que actualizar la tabla de abajo y el Manual 1.
>
> 🔴 **Y sigue sin resolverse el hardware:** son 10 demandas para 9 MOSFET —el buzzer y la
> talanquera se disputan el unico driver libre—, y a donde sale `PB2` de verdad esta **por
> confirmar con multimetro** (tarea `B3` de `ESTADO.md`). El firmware ya escribe el pin; que
> ese pin mueva un motor es lo que falta comprobar.

### Lo que ya existe y no se usa

```
   pines.h:21   #define MOTOR_TALANQUERA   PB2   // J15
```

Ese pin esta declarado en las dos puntas, tiene **MOSFET de potencia y bornera propia**, y
**ninguna linea del firmware lo escribe** — comprobado con el censo de `pinMode()`: no aparece.
Es hardware pagado y muerto, igual que el semaforo peatonal (`PA6`/`PA7`) y el buzzer (`PB1`).

### Para que sirve una talanquera aqui

En un paso alternado la barrera fisica hace lo que la luz no puede: **detener al que no mira**. En
obra con maquinaria pesada, o de noche, o con conductores que ya han visto veinte semaforos de obra
esa semana, el rojo se salta. Una barrera, no.

### 🛑 La regla, y es la misma que gobierna todo lo demas aqui

> **La talanquera SIGUE al semaforo. Nunca lo manda, y nunca lo contradice.**

| Luz | Talanquera |
|---|---|
| 🔴 Rojo | **BAJADA** |
| 🟡 Ambar (transicion) | **BAJADA** — solo sube cuando el verde esta confirmado |
| 🟢 Verde | **SUBIDA** |
| Todo-rojo de despeje | **BAJADA en las dos puntas** |
| Ambar intermitente (SFTY-6, sin enlace) | ✅ **SUBIDA** — decidido por el cliente el 27/08/2026: sin enlace se deja pasar con precaucion, que es lo que ese ambar significa en la calle. Cerrar la via dejaria un corredor de obra sin salida |

Y la consecuencia estructural, que es lo que la hace segura: **la orden de la talanquera sale del
mismo sitio que la luz, `semaforo.cpp`, y de ningun otro.** Es `§6` extendida: si un modo pudiera
mover la barrera por su cuenta, tendriamos una barrera abierta con la luz en rojo, que es peor que
no tener barrera — porque el conductor confia en ella.

### ⚠️ Las tres preguntas que hay que cerrar antes de escribir una linea

1. **¿Que hace al perder el enlace?** SFTY-6 cae a ambar intermitente, que significa *"pasa con
   precaucion"*. Una barrera **bajada** ahi cierra la via por completo; **subida**, deja pasar a los
   dos lados a la vez. Ninguna de las dos es obviamente correcta y la decision es del cliente y del
   PMT, no del firmware.
2. **¿Que hace al cortarse la energia?** Una talanquera que se queda arriba con el equipo muerto es
   una via abierta sin regulacion. Esto **no se resuelve en software**: se resuelve eligiendo un
   actuador que caiga por gravedad o con muelle de retorno. Va en la especificacion de compra.
3. **¿Y si no llega a bajar?** Un final de carrera que confirme la posicion convierte "ordene bajar"
   en "esta bajada", que no es lo mismo. Sin realimentacion, el firmware no puede saberlo y **no debe
   fingir que si**. Si no hay final de carrera, el manual tiene que decir que la barrera es una ayuda
   visual y que **quien regula sigue siendo la luz**.

### 🔌 En que salida se acciona — MEDIDO sobre el esquematico, y hay un problema

```
   MOSFET de potencia en la placa:  9 x IRLZ44N  (Q1..Q9)
   Pines de luz en pines.h:         8            (ROJO/AMARILLO/VERDE 1 y 2 + 2 peatonales)
   -> queda UN driver de potencia libre
```

> ### ✅ RESUELTO el 27/08 — y CORREGIDO el mismo dia, porque la primera medida uso el plano malo
>
> ⚠️ **Primera version (equivocada):** *"son 10 demandas para 9 MOSFET; la talanquera se queda sin
> etapa y `J15` no existe"*. Salio de `03_Hardware_Tarjeta/KiCad/*.kicad_sch`, que esta **incompleto**
> —sin LCD, sin botones y sin el canal del motor— y cuyo `.kicad_pcb` **esta vacio (78 bytes)**.
>
> **Medido sobre el plano bueno** (`01_Firmware/Controladora_Semaforos/.../*.kicad_sch`, 649 KB):
> **son DIEZ MOSFET y DIEZ optos** (`Q1..Q10`, `U6..U15`). No falta ninguno:
>
> | red | por donde pasa |
> |---|---|
> | `S1`…`S8` | R 220 + R 10K -> opto `TLP127` -> MOSFET `IRLZ44N` -> bornera. Ocho canales identicos |
> | `Buzzer` (`PB1`) | **el mismo camino**: `R55`+`R54` -> opto `U13` -> MOSFET `Q8` -> bornera `J13` con 12 V |
> | `Motor` (**`PB2`**) | **su propio canal completo**: `R70`+`R69` -> opto **`U15`** -> MOSFET **`Q10`** -> bornera **`J15`** |
> | `Puerta` (`PB0`) | `R64` 10 kOhm + `C25` 100 nF -> bornera `J14`. Es un **RC de 1 ms: una ENTRADA con antirrebote por hardware**, que es donde el firmware lee la camara de demanda |
> | `PB8` | `R16` 1 kOhm -> **LED `D5`**. Es un indicador, no una entrada optoacoplada de camara |
> | `PB3..PB7` · `PB9`,`PB13..PB15` | la pantalla ST7920 (`SCL`,`CS`,`SI`,`RS(A0)`,`RST`) y `Boton1..4` |
> | `PA11`, `PA12`, `PA15`, `PC13` | **sin cable**: los unicos pines realmente libres, y sin bornera |
>
> **Conclusion: la talanquera ya esta donde tiene que estar.** `MOTOR_TALANQUERA = PB2` es exacto, no
> hace falta mover nada, no hace falta modulo de rele externo y **no hay que sacrificar el buzzer**.
> Lo unico que sigue sin entrada fisica es la **segunda camara** (la de umbral): `PB8` alimenta un LED
> y los cuatro pines libres no tienen bornera. Eso es un hilo, no un chip.
>
> 🔴 **Y la leccion, que costo una conclusion equivocada publicada:** habia **dos esquematicos** en el
> repositorio y `ESTADO.md` apuntaba al incompleto. *Un "no aparece" no es un hallazgo hasta haber
> descartado al buscador* — aqui el buscador leia el plano que no era. **Dos copias de un plano son
> peores que ninguna**, y hay que dejar una sola.

> 🔴 **Y un dato falso en el fuente, encontrado al medir:** `pines.h` dice
> `#define MOTOR_TALANQUERA PB2 // J15`, pero **la bornera `J15` no existe**. El esquematico tiene
> `J1`..`J14` y `J16`. Quien vaya a cablear buscando `J15` no la va a encontrar. Hay que trazar a
> donde sale `PB2` de verdad —probablemente por el conector de 12 vias `J16`— y corregir el
> comentario.

### ⚠️ Y un MOSFET no basta para una talanquera

Un IRLZ44N de canal N conmuta **encendido/apagado** contra masa. Un motor de barrera necesita **dos
sentidos**, y eso no sale de un solo transistor. Las tres salidas posibles, de menos a mas trabajo:

| Opcion | Salidas que consume | Nota |
|---|---|---|
| **Barrera con controlador propio** que acepte contacto seco de apertura | **1** | Es el caso industrial habitual y el unico que cabe hoy. El controlador de la barrera se ocupa del motor, los finales de carrera y la seguridad antiaplastamiento |
| Dos salidas de pulso, abrir y cerrar | 2 | No caben sin tocar la placa |
| Puente en H externo | 1-2 + placa auxiliar | Se asume el control del motor, y con el la responsabilidad de no aplastar a nadie |

**La primera es la unica que cabe en la placa actual, y ademas es la correcta**: dejar el motor y su
seguridad al equipo que esta disenado para eso, y que el semaforo solo diga *abre* o *cierra*.

### Coste

Un pin ya declarado y **el ultimo MOSFET libre —si el buzzer no se lo lleva—**. En flash, unas pocas lineas
dentro de `escribirPines()`. Lo caro no es el codigo: son las tres decisiones de arriba y el
actuador.

### Criterio de aceptacion

Pack que barra **todos** los estados del semaforo y exija que la talanquera nunca este arriba con el
verde apagado — con su control negativo, forzando el caso contrario. Y en banco: cortar la energia
con la barrera arriba y comprobar que baja sola.
