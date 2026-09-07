# PREGUNTAS Y VALIDACIÓN DE DISEÑO FUNCIONAL (V9.0)

Este documento recopila los puntos clave de diseño funcional y operativo acordados y cerrados con el equipo técnico y de concesión vial.

---

## 1. Integración de Cámaras de Visión Artificial (Hikvision AcuSense G2 - APROBADO)

> # 🔴 EL PIN DE ESTE APARTADO ERA FALSO, Y ES EL ERROR MÁS CARO DEL DOCUMENTO
>
> **Este apartado mandaba las cámaras a `PB9` y `PB13`. Esos dos pines son los canales `A` y `B` del
> MANDO DE RELÉS**, no entradas de cámara — y **nunca fueron optoacoplados**. Se corrige aquí y se
> deja el rastro, porque una decisión marcada `CERRADO` es justo la que nadie vuelve a comprobar.
>
> **Lo que pasa si se cablea una cámara a `PB9` o `PB13`:** tres pulsos de tráfico dentro de la
> ventana de 12 s **componen una secuencia de mando** y el semáforo **cambia de modo solo** —a
> Automático con `A·A·A`, a ámbar con `B·B·B`—; cuatro alternos en 18 s lo meten en Degradado. No es
> una conexión inerte.

1. ~~**Topología de Cámaras (CERRADO):** … a las entradas optoacopladas `PB9` y `PB13`.~~
   ⛔ **ANULADO.** El reparto real, **MEDIDO** en `Maestro/include/pines.h` (idéntico en el Esclavo):

   | entrada | pin | bornera | estado |
   |---|---|---|---|
   | `CAM_DEMANDA_PIN` | `PB0` | `J14` — con `R64` 10 kΩ + `C25` 100 nF | ~~✅ **cableable hoy**~~ → 🟠 **07/09: VIVO Y SIN CÁMARA.** `D-2`/`D-3`: **las dos cámaras del cruce van a `J16` p10 y p12, una por poste**, y **`J14`/`PB0` queda libre**. El motivo no es el conector: **`CAM_CIEGA`/`CAM_PEGADA` miran `J16` y no miran `J14`** — una cámara aquí funciona y **nadie sabría que se estropeó**. Reservado a un posible fin de carrera de barrera |
   | `CAM_C_PIN` | `PB14` | `J16` **p10** — sin antirrebote de placa | ~~🟠 **NO cablear hasta `M3`**~~ → ✅ **M3 CERRADA el 03/09 y verificada en banco (paso 21):** `0 V` en reposo y **sin demandas fantasma**, con el cable puesto y sin él |
   | `CAM_D_PIN` | `PB15` | `J16` **p12** — sin antirrebote de placa | ~~🟠 **NO cablear hasta `M3`**~~ → ✅ **M3 cerrada**, `0 V` en reposo (paso 20). ⚠️ **Pero `p12` es el punto del conector MÁS cercano a la red de 12 V —`1,359 mm` de cobre— y `J16` p1 reparte 12 V crudos** |

   > 🔵 **ACTUALIZADO EL 04/09.** La medida `M3` que estas dos filas esperaban **se hizo el 03/09 con
   > multímetro**: las cuatro posiciones llevan pull-down real de `10 kΩ` a masa y `3,3 V` en la
   > posición contigua, así que **el gesto es cerrar contra los `3,3 V`** y la lectura activa en ALTO
   > del firmware es la correcta. Números y procedencia en `17_...md`, revisión del 03-04/09.
   >
   > 🛑 **Lo que sigue siendo obligatorio y NO lo levanta esta medida: tapar `J16` p1 en CADA equipo
   > que se monte**, retirando el pin del cuerpo del conector volante. El banco midió que **las 9
   > salidas** de campo van con `220 Ω` + optoacoplador y **las 5 entradas no llevan nada** —del borne
   > directo a la pata del STM32— (N-120, `17_...md` §3.6). Un contacto de `p1` a `p10` o `p12` mete
   > 12 V en una pata que espera 3,3.

   **Las tres son ACTIVAS EN ALTO**: el contacto seco cierra contra **3,3 V**, no contra masa.
   Contra masa **la cámara no dispara nunca** y no hay síntoma. **`PB9` y `PB13` NO admiten cámara.**
2. **Cero Computadores Edge Externos (CERRADO):** Se descarta el uso de Raspberry Pi, Jetson Nano, conversores USB y switches Ethernet. La detección de vehículos corre directamente dentro de la cámara (DSP AcuSense) y entra por pulsos limpios de hardware a la placa. ✅ **Sigue cerrado y sigue siendo cierto.**
3. **Respuesta Adaptativa (CERRADO):** Cada pulso de detección solicita verde para el sentido correspondiente. ~~Se garantiza un piso inquebrantable de **15 segundos mínimos de All-Red (Todo-Rojo)** antes de conmutar de un carril al opuesto.~~ *(Corregido: el pulso entra por `PB0`, `PB14` o `PB15`, ~~`PB9`~~.)*
   🛑 **EL TODO-ROJO DEL MODO INTELIGENTE SON 10 s, NO 15 — corregido el 05/09, y la cita tampoco
   era la que decía.** **MEDIDO:** `modo_inteligente.cpp:25` y `:53` fijan
   `segEstatico = DESPEJE_SEG_MIN`, y `Maestro/include/limites_ciclo.h:56` da
   **`DESPEJE_SEG_MIN = 10`**. ~~`modo_inteligente.cpp:42` (`segEstatico = 15`)~~ — esa línea es hoy
   un comentario sobre polaridad, y el `15` se escribió antes de que N-137 (04/09) centralizara las
   seis constantes en `limites_ciclo.h`.
   ~~**Lo que SÍ son 15 s** es el **verde mínimo** antes de permitir alternancia —
   `modo_inteligente.cpp:100-101`, `if (tiempoActual >= 15000UL)`, `:90`—~~
   🔴 **TACHADO EL 07/09: ESE VERDE MÍNIMO DE 15 s YA NO EXISTE.** Lo retiró `D-19` (05/09), que
   resuelve `A-12`. **Re-corrido hoy el mismo `grep` que sostenía la frase:**

   ```
   $ grep -rn "15000UL" --include=*.cpp 01_Firmware/Maestro
   Maestro/src/modo_inteligente.cpp:62:// Y encima la Regla 1 cortaba el verde a los 15 SEGUNDOS -`tiempoActual >= 15000UL`-,
   Maestro/src/modo_inteligente.cpp:260:        // toda la diferencia con el `tiempoActual >= 15000UL` que habia aqui.
   ```

   **Las dos coincidencias que quedan son COMENTARIOS que cuentan que estuvo ahí.** Hoy el suelo
   del Modo Inteligente lo pone el operario —`modoAutomatico_tiemposCiclo()`— y el techo es
   `sueloMin * TECHO_POR_SUELO` (`D-19`).
   ⚠️ **Y `D-19` está APROBADA CON CONDICIÓN, y la condición NO está cumplida:** *«el doble **si** un
   funcional revisa el manual y este manual de uso es claro»*. **Hasta esa firma, `TECHO_POR_SUELO`
   viaja como POR VALIDAR** — y ésa es una pregunta de este documento, no de firmware.

   **Lo que sí sigue siendo 15 s** es el **despeje por defecto del Modo Manual**,
   `tiempoDespejeMs = 15000` (`Maestro/src/modo_manual.cpp`, símbolo `tiempoDespejeMs`), que es el
   número que se sintió en banco. ⚠️ **No se confunda con el despeje configurable del Modo
   Automático, que va de 10 a 90 s (§2.1).**

   > 🔴 **AMPLIADO EL 04/09 — N-130, y limita este apartado: la demanda SÓLO se atiende en Modo
   > Inteligente.** Censo de llamadores de la bandera `demandaRemotaPendiente`, hecho con `grep` y no
   > leyendo: **un solo fichero** —`modo_inteligente.cpp:87`, `:102`, `:110`, `:124`—. **En Modo
   > Manual y en Modo Automático el pulso entra y nadie lo mira.**
   >
   > Hasta el 04/09 el equipo lo **acusaba igual**: el Esclavo contestaba
   > `$ACK,CMD:SOLICITAR_PASO,RESULT:PEDIDO_AL_MAESTRO`, el operario leía la confirmación y **el cruce
   > no se movía**. Volvía a pulsar. Hoy el acuse **dice cuál de las dos cosas es** —`DEMANDA_ACEPTADA`
   > / `DEMANDA_RECHAZADA`, `protocolo.h:171-172` en las dos puntas— y el Esclavo levanta un evento
   > `MAESTRO / DEMANDA_NO_ATENDIDA_MODO_ACTUAL` (`Esclavo/src/main.cpp:541-543`).
   >
   > ⚠️ **Lo que eso NO cierra, y es una pregunta funcional, no técnica: ¿es aceptable que un botón de
   > demanda no haga nada en dos de los tres modos operativos?** Ver §5.2.
4. ~~**Cobertura Simétrica (CERRADO):** … Cámara 1 y 2 en Maestro; Cámara 3 y 4 en Esclavo.~~
   ⚠️ **La simetría se mantiene, la numeración no.** Cada punta tiene **tres entradas de demanda
   iguales**. **La «cámara de umbral» NO EXISTE**: no hay entrada física para ella ni comando de
   radio que lleve la cuenta del tramo al Maestro. El despeje se hace **por tiempo**
   (`cfgDespejeSeg`), que es el criterio conservador.

---

## 2. Parámetros de Operación Vial y Módulo Bluetooth (APROBADO)

1. ~~**Tiempo de Despeje (All-Red):** El firmware admite de **5 a 999 segundos** (hasta 16.6 min); el piso de 5s en menú y 15s en auto-recuperación es inquebrantable por seguridad.~~

   > 🔴 **ANULADO EL 04/09/2026: las tres cifras son falsas, y una de ellas ninguna versión del
   > firmware pudo aceptarla nunca.** **MEDIDO** en `01_Firmware/Maestro/src/modo_automatico.cpp:53`:
   >
   > ```
   >    static const uint8_t DESPEJE_SEG_MIN = 10, DESPEJE_SEG_MAX = 90;
   > ```
   >
   > **El despeje va de 10 a 90 segundos.** El piso real es **10 s**, no 5 —era la mitad—; el techo
   > **90 s**, no 999 —era once veces—, y **999 no cabe en el `uint8_t` que transporta el valor**, así
   > que no fue representable en ninguna versión. `modoAutomatico_fijarTiempos()` hace `return false`
   > fuera de rango **venga del menú o de Bluetooth**, y la orden se rechaza con
   > `$ERR,CMD:SET_TIEMPOS,DESC:RANGO` (`Maestro/src/bluetooth.cpp`, símbolo `SET_TIEMPOS`).
   >
   > 🔧 **CORREGIDO EL 07/09 — LA CITA DE ESTA MEDIDA CADUCÓ, aunque la cifra sea buena.** `N-137`
   > (04/09) **centralizó las seis constantes** y hoy no viven en `modo_automatico.cpp`:
   >
   > ```
   > $ grep -rn "DESPEJE_SEG_MIN =\|VERDE_MIN_MIN =" --include=*.h 01_Firmware/Maestro
   > Maestro/include/limites_ciclo.h:58:static const uint8_t VERDE_MIN_MIN = 3,  VERDE_MIN_MAX = 15;
   > Maestro/include/limites_ciclo.h:60:static const uint8_t DESPEJE_SEG_MIN = 10, DESPEJE_SEG_MAX = 90;
   > ```
   >
   > *(~~Los **15 s** sí existen, pero son otra cosa: son el todo-rojo y el verde mínimo del **Modo
   > Inteligente** —`modo_inteligente.cpp:42` y `:90`—~~ 🔴 **TACHADO EL 07/09: el todo-rojo del Modo
   > Inteligente son 10 s** —`DESPEJE_SEG_MIN`, ver §1.3— **y el verde mínimo de 15 s YA NO EXISTE**:
   > lo retiró `D-19`, y el `15000UL` sólo sobrevive en dos comentarios. El `grep` está en §1.3.)*
   >
   > ⚠️ **El mismo error estuvo publicado en `MANUAL_USUARIO.md` y en `OPTIMIZACIONES.md`, y allí se
   > corrigió el 31/08. Aquí sobrevivió cinco días más.** Es lo que `roadmap.md` `A4` llama *cifras sin
   > vigilante*: una cifra copiada a mano en varios documentos **envejece en cada copia por separado**,
   > y la que nadie vigila es la que se lee en obra.
2. **Módulo Bluetooth en Maestro y Esclavo (CERRADO - Estándar Baliza):** ~~Se conecta a `USART1` (`PA9` TX, `PA10` RX) con alimentación 5V/GND.~~
   🔴 **PINOUT ANULADO.** Se conecta a **`USART1` REMAPEADO — `PB6` TX / `PB7` RX — por el conector
   `J17`** (**MEDIDO**: `Maestro/src/bluetooth.cpp:28`, `static HardwareSerial SerialBT(PB7, PB6)`,
   idéntico en el Esclavo). `USART1` sale por `PA9`/`PA10` **o** por `PB6`/`PB7`, **nunca por los
   dos a la vez**: un montaje en `PA9`/`PA10` exigiría cambiar el firmware de las dos puntas.
   **`PA9`/`PA10` son hoy `RS485_IN`** (el MAX3485 `U2` y la bornera `J10`).
   🔴 **Y la alimentación NO sale de `J17`:** el módulo lleva **fuente propia desde 12 V**. Ver
   `10_Manual_Modulo_Bluetooth_Telemetria.md` §2.3.
   ⚠️ **La *«Caja Negra de caídas de radio»* estaba declarada y documentada, y NO tenía un solo
   llamador.** No la ofrezca como función existente sin comprobar el fuente.
3. **Mando a Distancia Anti-Colisión (CERRADO - Resolución N-53):** ~~Secuencias con alternancia (`A·B·A` Auto, `B·A·B` Ámbar, `B·A·B·A` Manual, `A·B·A·B` Degradado).~~

   > # ⛔ 07/09/2026 — TODO ESTE PUNTO 3 ES HISTÓRICO. **EL MANDO NO EXISTE, Y AQUÍ ABAJO HABÍA UN GESTO DE BANCO QUE NO SE HACE**
   >
   > **`DECISIONES.md` `D-1` (05/09), confirmada por el responsable:** *«ya no tenemos mandos de A y
   > B, sólo la app, los quitamos»*. **No hay emisor, no hay receptor RF, no hay pulsadores y `J16`
   > p5 y p8 quedan LIBRES Y SIN CABLEAR** (`15_Lista_de_Compras_Hardware.md`, `A9` con **cero
   > unidades**). Lo que **sí** se conserva es el **CÓDIGO** —`mando_ambarLocal()` sostiene el veto
   > de SFTY-21, y borrar su armador lo deja **abierto**, no inerte—.
   >
   > 🛑 **Y LO QUE HAY QUE LEER ANTES DE BAJAR AL BANCO, porque este apartado lo mandaba hacer:**
   > el gesto *«`J16` p5 contra p4 y p8 contra p7»* que se describe más abajo **QUEDA ANULADO**.
   > No se le acerca un cable a `p5` ni a `p8` — ni «a ver qué pasa»: **`p4` es adyacente a `p5`**,
   > y un puente corrido una posición pone el riel de 3,3 V contra masa. **Es el candidato del
   > sobrecalentamiento que abortó el paso 29** del banco del 3–4/09 y dejó una tarjeta Maestro con
   > un corto (`N-116`). Está escrito igual en `12_Cobertura_de_Pruebas_y_Huecos.md` §1 y en
   > `15_Lista_de_Compras_Hardware.md` (bloque `A9`).
   >
   > 🔴 **Y el código del mando SIGUE LEYENDO ESOS DOS PINES** —`botones_actualizar()` llama a
   > `mando_registrarPulso(MANDO_A/B)` en cada flanco—, así que **cualquier cosa que se cablee ahí
   > compone secuencias sin que nadie lo pida** (`DECISIONES.md` `A-2`). **Nada se cablea en `J16`
   > p5 ni p8.**
   >
   > **Lo que sigue debajo se conserva porque describe el vocabulario real del código que se queda,
   > y porque el razonamiento del *activo en ALTO* gobierna hoy el cableado de las CÁMARAS.**

   🔴 **LAS SECUENCIAS DE ESTA LÍNEA NO SON LAS DEL FIRMWARE.** Un operario que las ejecute no
   consigue nada. **MEDIDO** en `Maestro/src/mando.cpp`:

   | secuencia | ventana | qué hace | línea |
   |---|---|---|---|
   | `A · A · A` | ≤ 12 s | **Automático** — 2 destellos rojos | `:225-227` |
   | `B · B · B` | ≤ 12 s | **Ámbar** — 3 destellos rojos | `:230-234` |
   | `A · B · A · B` | ≤ 18 s | **Degradado** — 4 destellos rojos | `:204-214` |

   **No existe secuencia para Modo Manual.** ~~El mando **se conserva** sobre los canales `A` (`PB9`)
   y `B` (`PB13`)~~ ⛔ **CADUCADO EL 05/09 POR `D-1`: se conserva el CÓDIGO, no el mando.** `C` y `D`
   se retiraron porque sus pines pasaron a cámaras, y **ninguna secuencia los usaba**.
   ⚠️ ~~**El receptor RF no se ha comprado en ninguna punta**, así que hoy no hay con qué generarlos
   POR RADIO desde el piso — en banco se generan con un cable, y el gesto está medido (`N-118`,
   `346ea5f`):~~
   ⛔ **TACHADO EL 07/09 — `D-1`: no se va a comprar ningún receptor** (`15_…`, `A9`, cero unidades),
   **y el gesto de banco tampoco se hace.**

   🛑 ~~**`J16` p5 contra p4** (canal `A`) y **`J16` p8 contra p7** (canal `B`) — los **3,3 V del pin
   contiguo—, NUNCA contra masa.**~~ 🔴 **ANULADO EL 07/09. NO SE PUENTEA `p5` NI `p8` CONTRA NADA:**
   `p4` es adyacente a `p5`, un puente corrido una posición pone 3,3 V contra masa, y ése es el
   candidato del sobrecalentamiento del paso 29 (`N-116`). **Esos dos pines quedan libres y sin
   cablear.**

   ✅ **Lo que del razonamiento SÍ sigue vigente, y por eso no se borra: las entradas de `J16` se
   leen en `INPUT` pelado y son ACTIVAS EN ALTO** —`J16` tiene **una sola masa en todo el conector**
   (`p2`) y 3,3 V en cada posición contigua, así que un cable a masa **no produce absolutamente
   nada**—. **Eso es lo que gobierna hoy el cableado de las CÁMARAS** en `p10` y `p12` (`D-2`,
   `D-3`, y `15_…` línea `A7`).

   👁️ **La confirmación no necesita app ni terminal: la dan las propias luces**
   (`Maestro/src/mando.cpp:45-47`) — los destellos rojos de la tabla, y un **ámbar rápido de 2 s**
   si la secuencia se rechaza. ⚠️ **Pruébelo DESDE OTRO MODO:** si el equipo ya está en el modo que
   la secuencia pide, `MODO:` no cambia y la prueba no distingue nada. La inhibición durante la navegación de la LCD sigue en el firmware, pero **ya
   no puede ocurrir**: sin `botonAceptar()` el menú no se abre.

---

4. 🆕 **Mínimo de verde y de rojo: 3 minutos — DECIDIDO EL 04/09/2026.** Sube de 1 a 3 min (los
   techos siguen en 15 min). Motivo del responsable, literal: **«tres minutos es la mínima distancia
   de seguridad»**. **MEDIDO, y la cita re-corrida el 07/09** —~~`Maestro/src/modo_automatico.cpp:51-52`~~,
   que ya no es donde viven: `N-137` las centralizó—:

   ```
   $ grep -rn "VERDE_MIN_MIN =" --include=*.h 01_Firmware/Maestro
   Maestro/include/limites_ciclo.h:58:static const uint8_t VERDE_MIN_MIN = 3,  VERDE_MIN_MAX = 15;
   ```

   **El razonamiento, en una línea:** en un paso alternado de un solo carril un camión pesado tarda
   entre 5 y 8 s **sólo en reaccionar y arrancar**; con un verde de 60 s lo que se produce no es una
   cola, es un conductor convencido de que el semáforo está averiado y adelantando en rojo contra el
   sentido que acaba de recibir verde. El límite de 1 minuto **era un valor de mesa de pruebas** que
   se quedó abierto para la operación en vía.

   🔴 **La guarda vive en el FIRMWARE, no en la app, y eso es la otra mitad de la decisión:** la app
   no es la única que puede hablar por `J17` —cualquier otra cosa en ese cable, o una APK vieja, puede
   mandar `SET_TIEMPOS` con un minuto—. Una guarda que sólo vive en la interfaz **es de cortesía**;
   ésta rechaza con `$ERR,CMD:SET_TIEMPOS,DESC:RANGO` y no la puede saltar nadie.

   ⚠️ **COSTE ACEPTADO A SABIENDAS: ya no se puede probar en mesa con ciclos de un minuto.** Quien
   planifique la próxima sesión de banco tiene que contar **tres minutos por paso** al escribir los
   tiempos. *(Cierra `N75-1`, abierta desde el 26/08. Detalle en `17_...md` §3.4.)*

5. 🆕 **El cruce se opera DESDE EL MAESTRO — DECIDIDO EL 04/09/2026.** Se descartó hacer transparente
   el mando desde el Esclavo: **no se relevan `SET_MODO` ni `MANUAL:CAMBIAR_TURNO` por radio.**

   **RE-MEDIDO el 05/09** por censo de despachadores (`grep -nE 'strcmp\(accion|strncmp\(accion'`):
   el Maestro atiende **14** comandos (~~`:444-664`~~ → **`Maestro/src/bluetooth.cpp:498-763`**) y el
   Esclavo ~~**5**~~ → **6** —`AMBAR_EMERGENCIA` (`:468`), `CANCELAR_AMBAR` (`:506`),
   `SOLICITAR_PASO` (`:588`), **`SET_RTC:` (`:619`)**, más `FORZAR_ROJO` (`:580`) y `TEST_LEDS`
   (`:606`) que rechaza a propósito— (~~`:468-561`~~ → **`Esclavo/src/bluetooth.cpp:468-679`**).
   🛑 **Faltaba `SET_RTC`, y el rango citado cortaba el despachador por la mitad** —dejaba fuera 4 de
   los 6—. Es `CLAUDE.md` §4.quinquies: *el instrumento comparaba contra un borde equivocado*. **Y no
   es cosmético: §5.3 apoyaba en ese «5» una decisión sobre qué queda operable con el Maestro
   caído**, y el sexto comando **pone la hora**, que es la condición de entrada al Degradado
   (`SYNC_FRESCA_MS`, 2 h). **El Esclavo pide; no ordena** (SFTY-27): `SOLICITAR_PASO`
   manda la misma demanda que la cámara y **el Maestro decide, aplica el todo-rojo y ordena**.

   🔴 **CONSECUENCIA OPERATIVA, y es la que hay que llevar a obra: el operario tiene que saber a qué
   poste conectarse ANTES de caminar.** Con dos postes que pueden estar a cientos de metros,
   descubrir en el sitio que el Bluetooth al que se conectó no atiende `SET_MODO` es una caminata
   perdida y un cruce que sigue como estaba. Lo que lo resuelve es el **rótulo Bluetooth**
   —`SEM-<serie>-M` para el Maestro y `SEM-<serie>-E` para el Esclavo—, visible en la lista de
   emparejados **antes de conectar**. **Y ese rótulo tiene una pregunta abierta: §5.1.**

   ⚠️ **Lo que se acepta al decidirlo, y va escrito para que conste:** con el Maestro caído o
   inaccesible, la única superficie que queda en el Esclavo son sus comandos propios —~~cinco~~
   **SIETE, re-medidos el 07/09**, ver el `grep` de §5.3—. ~~**No hay forma de cambiar el modo del
   cruce desde el Esclavo**, ni por Bluetooth ni por radio.~~ 🔵 **MATIZADO EL 07/09 por `D-18`: sí
   hay UNA, y es la que importa en ese escenario — `SET_MODO:DEGRADADO`.** No cambia el modo *del
   cruce*: pone **esa punta** en Degradado, que es el modo diseñado para cuando la otra no contesta.
   Ver §5.3.

---

## 2.1 Modo Degradado — decisiones abiertas (SFTY-21)

Todas estas cifras están **construidas en firmware** y validadas en simulador, pero **no
contrastadas contra la operación real de obra**. Se listan para que el funcional las confirme o las
corrija **antes** de la primera puesta en campo.

1. **Ciclo degradado fijo de verde 30 s / todo-rojo 30 s (ciclo de 120 s).** No hereda el verde
   configurado en Modo Automático, deliberadamente: un verde de 2 min con todo-rojo de 30 s daría un
   ciclo de 5 minutos y nadie espera eso en un paso alternado sin invadir. **¿Son 30/30 aceptables
   para el tramo de obra real, o hace falta ajustarlos por longitud?** *(Bajar el todo-rojo acorta el
   margen de deriva y obligaría a bajar también el límite de 48 h.)*
2. **Límite duro de 48 h sin resincronizar ⇒ caída automática a ámbar.** Sale de un margen teórico de
   ~3,5 días ~~con factor de seguridad 2~~ 🔴 **con factor de seguridad 1,44, no 2.**

   > **MEDIDO el 01/09** ejecutando el C++ real de las dos puntas a la vez, cada una con su reloj
   > (`Maestro/src/modo_degradado.cpp:39-45`): el cruce **aguanta 29 s** de desfase entre relojes y
   > el equipo puede **acumular 20,2 s** en 48 h (17,2 de deriva + 3 de tolerancia). Margen: **8,8
   > s → factor 1,44.** El «2» era una cuenta que nadie había rehecho.

   **¿Se acepta que el equipo se rinda solo a las 48 h**, o la obra necesita una autonomía distinta?
   *Recordar el coste: una semana obligaría a un todo-rojo de ~90 s.* **Y ahora también:** con la
   mitad del colchón que se creía, un radio caído no se deja para la semana siguiente.
3. **Antigüedad máxima de la sincronización para poder entrar: 2 h.** Tiene una consecuencia
   operativa poco intuitiva: **si el radio lleva medio día muerto, ya no se puede entrar al
   Degradado.** La ventana para activarlo era mientras el enlace todavía respiraba. **¿Se acepta, o
   hace falta un procedimiento de contingencia para ese caso?**
4. **Tolerancia de desfase de ±3 s.** Diez veces por debajo del todo-rojo de 30 s. ¿Se valida?
5. **Riesgos residuales aceptados el 01/08/2026** — se dan por conocidos, pero conviene que consten
   firmados en el acta:
   - El verde se da **sin confirmación del otro extremo**. Con el radio muerto es inevitable.
   - **Salida asimétrica**: una punta en ámbar contra una punta en verde. Sin solución técnica sin
     radio; la mitigación es **procedimental** (verificación visual de ambas puntas, también al
     salir).
6. ~~**Códigos del mando de relés.** Al comprar el receptor del Esclavo hay que **exigir código
   independiente del mando del Maestro**. Con ambos a menos de una cuadra y el mismo código, una sola
   secuencia metería las dos unidades en Degradado a la vez, **saltándose la verificación por separado
   que justifica todo el diseño**. ¿Está esto en la especificación de compra?~~

   ⛔ **PREGUNTA SIN OBJETO DESDE EL 05/09 (`D-1`): NO SE COMPRA NINGÚN RECEPTOR** —`15_…`, línea
   `A9`, **cero unidades**—. Se tacha en vez de borrarse porque **la propiedad que protegía sigue
   siendo la que justifica el diseño**: entrar en Degradado exige **verificación visual de las dos
   puntas por separado**, y hoy quien la sostiene es el procedimiento, no un código de radio.

   🔵 **Y la pregunta que SUSTITUYE a ésta, porque el camino cambió: `D-18` (05/09) — el Modo
   Degradado del poste 2 se pide POR APP**, con `CMD:PIN:1234:SET_MODO:DEGRADADO`. **Construido y
   medido el 07/09** en `Esclavo/src/bluetooth.cpp` (símbolo `SET_MODO:DEGRADADO`).
   ⚠️ **Con una sola app y un solo teléfono, nada impide que el operario mande las dos puntas sin
   moverse del sitio** — que es exactamente el riesgo que el código independiente evitaba, una capa
   más arriba. **¿Se exige por procedimiento la verificación visual en cada poste antes de mandar la
   orden a la otra punta? Dueño: el responsable.**

---

## 3. Decisiones pendientes abiertas por la auditoría V8.0

1. **Operación intermitente por bajo flujo (`1_Manual_Usuario.md §2`):** el Manual de Señalización 2024 la contempla (ámbar en vía principal, rojo en secundaria, tras 4 h con flujo ≤50%), pero **no está implementada**. ¿Se incorpora al alcance, se difiere, o se retira de la especificación?
2. **Arranque en oscuro:** al encender, el Maestro deja las luces apagadas ~2 s durante la pantalla de bienvenida. ¿Se acepta, o debe arrancar directamente en Rojo?
3. **Watchdog en el Repetidor ESP32:** **sigue sin estar implementado** (`grep` sobre
   `01_Firmware/Repetidor/` da cero). Maestro y Esclavo lo detectan por fail-safe a los 25 s
   (SFTY-6), pero el repetidor requiere corte de energía para recuperarse de un cuelgue. ¿Se añade?

   > ⚠️ **No confundir con el otro ESP32.** El **puente de expansión** (`01_Firmware/ESP32_Expansion/`,
   > el de `J17`) **sí lleva watchdog** desde el 01/09 —Task WDT a 2 s, `src/vigilante.cpp:24`— y
   > además **declara por qué arrancó** al reconectar. Son dos módulos distintos con respuestas
   > opuestas a esta pregunta.
   >
   > 🔴 **Y una pregunta que ese watchdog NO cierra, y que sigue siendo del responsable:** un puente
   > de expansión colgado **no lo detecta nadie en el equipo** —SFTY-6 vigila la radio, no `J17`—, y
   > el watchdog no hace nada por uno **muerto o desalimentado**. ¿Basta con que el único testigo sea
   > la app, o el STM32 tiene que notar el silencio del puente?
4. **Velocidad aérea:** se adopta `2.4 kbps` (antes `0.3 kbps`) con un coste aproximado de 6 dB de sensibilidad. ¿Se valida contra la distancia máxima real de obra prevista?

---

## 4. Decisiones abiertas por la revisión del 01/08/2026

> 🛑 **LAS DOS PRIMERAS PREGUNTAS DE ESTA LISTA YA ESTABAN CONTESTADAS POR EL CÓDIGO, y se tachan
> con su medida — 05/09.** Una pregunta cerrada que sigue en una lista de decisiones abiertas hace
> que alguien **vuelva a decidir algo ya decidido**, y encima sobre un diagnóstico falso. Es
> `CLAUDE.md` §2.quater desde el otro lado: *las opciones que le pones delante al responsable son un
> instrumento*.

1. ~~**Persistencia del estado del Modo Degradado (N-20).** Hoy vive en RAM… El módulo que lo
   guardaría en los registros de respaldo **está escrito pero sin conectar**. ¿Se prioriza antes de
   ir a campo?~~
   ✅ **CERRADA EN CÓDIGO. Censo de llamadores con `grep`, no lectura:**
   `respaldo_guardarDegradado()` tiene **cinco puntos de escritura** —`modo_degradado.cpp:356`,
   `:377`, `:442`, `:464` y `Maestro/src/main.cpp:281`— y `modo_degradado_reanudarTrasCorte()`
   (`modo_degradado.cpp:326`) **se llama en el arranque**, `Maestro/src/main.cpp:95`. **Está
   conectado en las dos direcciones**, escritura y lectura.
   🔴 **Lo que sigue abierto es la PRUEBA, no el módulo: el microcorte con reanudación NO se ha
   ejercido en tarjeta.** El riesgo residual nº 2 no desaparece por estar el código escrito.
   ⚠️ *No confundir con autorización por adelantado ("si pierdes el radio X minutos, entra"): eso es
   entrada automática con pasos extra y sigue descartado.*
2. ~~**Configuración del ciclo recibida pero no consumida (N-18).** El Esclavo **almacena** la
   configuración… pero el cálculo del ciclo degradado todavía usa los 30/30 fijos compilados…
   **nadie vigila que la lleven**.~~
   ✅ **CERRADA EN CÓDIGO, y las dos mitades de la pregunta se caen por separado.**
   **(a) El Esclavo SÍ la consume:** `Esclavo/src/modo_degradado.cpp:117` (`rojoObligatorioMs()`),
   `:126-127` (`calcularFase()`) y `:415-416`. **No queda ningún 30/30 compilado en ese fichero**
   —medido con dos patrones, para descartar al buscador—.
   **(b) Y sí hay quien lo vigila:** `:202-203` **rechaza la entrada** al modo si no ha llegado la
   configuración — `DEG_RECHAZO_SIN_CONFIG` y `DEG_RECHAZO_CICLO_NULO`.
   Las dos puntas corren 30/30 **porque el Maestro se los manda**, no por coincidencia de versión:
   `Maestro/src/modo_degradado.cpp:249` envía `DEG_VERDE_SEG` / `DEG_DESPEJE_SEG` (`:74-75`).
3. **Margen de flash del Maestro (N-21).** ~~Tras el Modo Degradado va al **80,2 %**. Lo pendiente
   —persistencia conectada, pantalla informativa en ámbar y modo nocturno— lo dejaría sobre el
   **85 %**: ajustado pero viable.~~ **Una función grande más ya no cabría.** ¿Se acota el alcance
   funcional, o se evalúa el cambio de microcontrolador con su verificación chip a chip?

   > ⚠️ **CIFRA CADUCADA, y la pregunta no está respondida: está más apretada.**
   > ~~El acta de la compuerta del 04/09 —`evidencia/2026-09-04_compuerta.txt`, `HEAD 624eb37`—
   > publica **`89,3 %`** (`58496` de `65536` B), o sea **`7.040` B libres**.~~
   >
   > 🛑 **ESA ACTA YA NO EXISTE, y las cuatro cifras eran falsas — 05/09.** El fichero **se
   > reescribió sobre el mismo nombre** (`d2a510f`, *«cifras del acta con el arbol quieto»*), así que
   > lo que hoy publica es otra cosa: **`HEAD e0e835d` · `86,3 %` · `56588` de `65536` B ·
   > `8.948` B libres**. Ni el hash ni el porcentaje ni los dos recuentos coincidían.
   >
   > **Otros dos documentos de esta misma carpeta ya lo habían avisado** —`17_Arquitectura…:433` y
   > `8_Procedimiento_Modo_Degradado.md:106`, los dos con la frase *«ese acta ya no existe»*—: **este
   > era el único que seguía citándolo como vigente.** La información estaba; faltó cruzar ficheros.
   >
   > 🔴 **Y la cura no es escribir aquí el número nuevo, porque volverá a caducar: la cifra vigente
   > se lee del acta más reciente de `evidencia/`, con su HEAD al lado.** El acta del 05/09 lleva
   > `HEAD c954e74`. El **`85 %`** que este punto daba como escenario de llegada **ya se pasó
   > igualmente**, y lo que lo listaba como pendiente sigue pendiente.
   > 🔴 **Y el árbol de hoy tiene cambios posteriores a esa acta sin medir por la compuerta**: la cifra
   > que valga sale de una corrida nueva, no de aquí (`17_...md`, revisión del 04/09 por la tarde).
4. **Prueba de banco del reloj (N-15 / N-17).** Sigue sin hacerse: ni contraste contra hora patrón, ni
   conservación tras corte de energía, ni arranque con el cristal desconectado. **Todo lo que depende
   de la hora va hoy sobre un supuesto.** ¿Se agenda antes de la certificación?

---

## 5. Decisiones abiertas por la sesión del 04/09/2026

Las tres salen de los cambios de esa fecha —el mínimo de 3 minutos (§2.4), operar desde el Maestro
(§2.5) y N-130 (§1.3)—. **Ninguna la puede tomar quien escribe firmware.**

1. 🔴 **El rótulo Bluetooth de un módulo virgen es el MISMO en las dos puntas.** El ESP32 **no puede
   saber la serie al arrancar** —sale del silicio del STM32, `identidad_serie()` lee el UID del
   micro—, así que la aprende del `$STATUS` que retransmite y **la usa a partir de la SIGUIENTE
   arrancada** (`ESP32_Expansion/src/transporte_app.cpp:23-31`, `:105-113`). **No se re-rotula en
   caliente a propósito**: cambiar el nombre SPP obliga a cerrar el perfil, o sea **a tirar la sesión
   del operario que está dando una orden al cruce**. **MEDIDO** en `include/contrato.h:259`: el
   provisional es **`SEM-SIN-MATRICULA`**.

   **Con dos módulos vírgenes, los dos postes se llaman exactamente igual** hasta que cada uno haya
   visto un `$STATUS` **y se le haya cortado la energía** — y §2.5 acaba de apoyar en ese rótulo la
   decisión de a qué poste camina el operario.

   **¿Se cubre por procedimiento** —una vuelta de energía a cada módulo antes de irse, firmada en el
   acta de puesta en marcha— **, o el firmware debe dar un provisional distinto por módulo?** Las
   cuatro opciones, con su coste, en `17_...md` §3.8. **Dueño: el responsable.**

   ⚠️ **SIN VERIFICAR:** nadie ha visto este rótulo en un teléfono. El Bluetooth **no subió en el
   banco del 3-4/09**, y no hay una sola tarjeta con un ESP32 conectado a `J17`.

2. 🟠 **Un botón de demanda que no hace nada en dos de los tres modos.** Desde el 04/09 el equipo ya
   **no miente** —el acuse dice si se va a atender, N-130—, pero **el botón sigue estando** en la app
   en Modo Manual y en Modo Automático, y lo único que produce es una línea en la bitácora. **¿Se
   deja así, se oculta el botón mientras el equipo no esté en Modo Inteligente, o la demanda debe
   hacer algo en los otros modos?** Lo segundo es interfaz; lo tercero es alcance funcional y toca el
   ciclo. **Dueño: el responsable.**

   ⚠️ **SIN VERIFICAR, y decide si N-130 está cerrado o a medias:** que la app **pinte** ese evento y
   el operario lo lea **no se ha comprobado**. Si no lo pinta, el resultado es que se dejó de mentir
   y no se dice nada — que es mejor, pero no es lo mismo que avisar.

3. 🟡 **Sin Maestro no hay cambio de modo.** Consecuencia directa de §2.5, y va aquí para que se
   firme y no se descubra en obra: con el Maestro caído, inaccesible o fuera de servicio —hoy hay una
   tarjeta Maestro así, N-116—, la única superficie que queda en el Esclavo son sus comandos propios.

   🔧 **RE-MEDIDO EL 07/09, Y LA CUENTA HA CAMBIADO DOS VECES: SON SIETE, NO CINCO NI SEIS.** Este
   punto decía «cinco» mientras §2.5 de este mismo documento ya había corregido a «seis» — una
   contradicción dentro del propio fichero. Y desde entonces `D-18` añadió el séptimo:

   ```
   $ grep -nE 'strcmp\(accion|strncmp\(accion' 01_Firmware/Esclavo/src/bluetooth.cpp
   547:  if (strcmp(accion, "AMBAR_EMERGENCIA") == 0) {
   585:  } else if (strcmp(accion, "CANCELAR_AMBAR") == 0) {
   668:  } else if (strcmp(accion, "FORZAR_ROJO") == 0) {
   676:  } else if (strcmp(accion, "SOLICITAR_PASO") == 0) {
   694:  } else if (strcmp(accion, "SET_MODO:DEGRADADO") == 0) {
   771:  } else if (strcmp(accion, "TEST_LEDS") == 0) {
   784:  } else if (strncmp(accion, "SET_RTC:", 8) == 0) {
   ```

   🔵 **Y el séptimo cambia la respuesta de esta pregunta: `SET_MODO:DEGRADADO` (`D-18`, 05/09).**
   Con el Maestro caído, el Esclavo **ya no se queda sin ninguna vía de cambio de modo**: se le puede
   pedir el Degradado por app, que es justamente el modo para cuando la otra punta no contesta.

   ~~Y el mando de relés, la otra vía de último recurso, **también vive sólo en el Maestro**: el
   receptor del Esclavo no se ha comprado (N-19).~~ ⛔ **CADUCADO EL 05/09 (`D-1`): no hay mando en
   ninguna de las dos puntas, y no se va a comprar.**

   🛑 **Lo que la pregunta pasa a ser, y sigue siendo del responsable: `D-16` — SIN TELÉFONO NO HAY
   FORMA DE OPERAR EL EQUIPO.** No es una avería: es una propiedad declarada del sistema desde que se
   retiró el mando. **¿Se acepta, y qué procedimiento de contingencia se escribe** —segundo terminal
   emparejado, cable de carga, quién lo lleva— **para el día que el móvil llegue descargado a un
   poste?** **Dueño: el responsable.**
