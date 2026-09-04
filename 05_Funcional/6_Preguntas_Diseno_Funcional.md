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
   | `CAM_DEMANDA_PIN` | `PB0` | `J14` — con `R64` 10 kΩ + `C25` 100 nF | ✅ **cableable hoy** |
   | `CAM_C_PIN` | `PB14` | `J16` **p10** — sin antirrebote de placa | 🟠 firmware listo, **NO cablear hasta `M3`** |
   | `CAM_D_PIN` | `PB15` | `J16` **p12** — sin antirrebote de placa | 🟠 firmware listo, **NO cablear hasta `M3`** |

   **Las tres son ACTIVAS EN ALTO**: el contacto seco cierra contra **3,3 V**, no contra masa.
   Contra masa **la cámara no dispara nunca** y no hay síntoma. **`PB9` y `PB13` NO admiten cámara.**
2. **Cero Computadores Edge Externos (CERRADO):** Se descarta el uso de Raspberry Pi, Jetson Nano, conversores USB y switches Ethernet. La detección de vehículos corre directamente dentro de la cámara (DSP AcuSense) y entra por pulsos limpios de hardware a la placa. ✅ **Sigue cerrado y sigue siendo cierto.**
3. **Respuesta Adaptativa (CERRADO):** Cada pulso de detección solicita verde para el sentido correspondiente. Se garantiza un piso inquebrantable de **15 segundos mínimos de All-Red (Todo-Rojo)** antes de conmutar de un carril al opuesto. *(Corregido: el pulso entra por `PB0`, `PB14` o `PB15`, ~~`PB9`~~.)*
4. ~~**Cobertura Simétrica (CERRADO):** … Cámara 1 y 2 en Maestro; Cámara 3 y 4 en Esclavo.~~
   ⚠️ **La simetría se mantiene, la numeración no.** Cada punta tiene **tres entradas de demanda
   iguales**. **La «cámara de umbral» NO EXISTE**: no hay entrada física para ella ni comando de
   radio que lleve la cuenta del tramo al Maestro. El despeje se hace **por tiempo**
   (`cfgDespejeSeg`), que es el criterio conservador.

---

## 2. Parámetros de Operación Vial y Módulo Bluetooth (APROBADO)

1. **Tiempo de Despeje (All-Red):** El firmware admite de **5 a 999 segundos** (hasta 16.6 min); el piso de 5s en menú y 15s en auto-recuperación es inquebrantable por seguridad.
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
   🔴 **LAS SECUENCIAS DE ESTA LÍNEA NO SON LAS DEL FIRMWARE.** Un operario que las ejecute no
   consigue nada. **MEDIDO** en `Maestro/src/mando.cpp`:

   | secuencia | ventana | qué hace | línea |
   |---|---|---|---|
   | `A · A · A` | ≤ 12 s | **Automático** — 2 destellos rojos | `:225-227` |
   | `B · B · B` | ≤ 12 s | **Ámbar** — 3 destellos rojos | `:230-234` |
   | `A · B · A · B` | ≤ 18 s | **Degradado** — 4 destellos rojos | `:204-214` |

   **No existe secuencia para Modo Manual.** El mando **se conserva** sobre los canales `A` (`PB9`)
   y `B` (`PB13`); `C` y `D` se retiraron porque sus pines pasaron a cámaras, y **ninguna secuencia
   los usaba**. ⚠️ **El receptor RF no se ha comprado en ninguna punta**, así que ~~hoy no hay con qué
   generar los pulsos~~ **hoy no hay con qué generarlos POR RADIO desde el piso** — en banco se
   generan con un cable, y el gesto está medido (`N-118`, `346ea5f`):

   🛑 **`J16` p5 contra p4** (canal `A`) y **`J16` p8 contra p7** (canal `B`) — los **3,3 V del pin
   contiguo—, NUNCA contra masa.** Estas entradas se leen en `INPUT` pelado y **activas en ALTO**,
   igual que las cámaras de la tabla de arriba; `J16` tiene **una sola masa en todo el conector**
   (`p2`) y un cable a ella **no produce absolutamente nada**.

   👁️ **La confirmación no necesita app ni terminal: la dan las propias luces**
   (`Maestro/src/mando.cpp:45-47`) — los destellos rojos de la tabla, y un **ámbar rápido de 2 s**
   si la secuencia se rechaza. ⚠️ **Pruébelo DESDE OTRO MODO:** si el equipo ya está en el modo que
   la secuencia pide, `MODO:` no cambia y la prueba no distingue nada. La inhibición durante la navegación de la LCD sigue en el firmware, pero **ya
   no puede ocurrir**: sin `botonAceptar()` el menú no se abre.

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
6. **Códigos del mando de relés.** Al comprar el receptor del Esclavo hay que **exigir código
   independiente del mando del Maestro**. Con ambos a menos de una cuadra y el mismo código, una sola
   secuencia metería las dos unidades en Degradado a la vez, **saltándose la verificación por separado
   que justifica todo el diseño**. ¿Está esto en la especificación de compra?

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

1. **Persistencia del estado del Modo Degradado (N-20).** Hoy vive en RAM: un microcorte deja esa
   punta en ámbar mientras la otra sigue dando verde por reloj — **que es exactamente el riesgo
   residual nº 2**. El módulo que lo guardaría en los registros de respaldo, alimentados por la misma
   pila ya instalada, **está escrito pero sin conectar**. ¿Se prioriza antes de ir a campo?
   ⚠️ *No confundir con autorización por adelantado ("si pierdes el radio X minutos, entra"): eso es
   entrada automática con pasos extra y sigue descartado.*
2. **Configuración del ciclo recibida pero no consumida (N-18).** El Esclavo **almacena** la
   configuración que le llega por radio, pero el cálculo del ciclo degradado todavía usa los 30/30
   fijos compilados. Mientras las dos puntas lleven la misma versión de firmware coinciden — **pero
   nadie vigila que la lleven**. ¿Se cierra esto antes de campo, o basta con la comprobación de
   versión en el acta de pruebas?
3. **Margen de flash del Maestro (N-21).** Tras el Modo Degradado va al **80,2 %**. Lo pendiente
   —persistencia conectada, pantalla informativa en ámbar y modo nocturno— lo dejaría sobre el
   **85 %**: ajustado pero viable. **Una función grande más ya no cabría.** ¿Se acota el alcance
   funcional, o se evalúa el cambio de microcontrolador con su verificación chip a chip?
4. **Prueba de banco del reloj (N-15 / N-17).** Sigue sin hacerse: ni contraste contra hora patrón, ni
   conservación tras corte de energía, ni arranque con el cristal desconectado. **Todo lo que depende
   de la hora va hoy sobre un supuesto.** ¿Se agenda antes de la certificación?
