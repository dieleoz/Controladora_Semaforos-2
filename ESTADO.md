# ESTADO — dónde está parado el trabajo HOY (07/09/2026)

> **Este fichero es el estado VIVO.** Lo que está abierto, lo que bloquea y lo que falta medir.
> El *porqué* completo de cada `N-x` vive en [`roadmap.md`](roadmap.md); las decisiones vigentes,
> en [`DECISIONES.md`](DECISIONES.md) — **si un párrafo de aquí contradice una fila de allí, gana
> la fila**. Lo de debajo del separador es histórico y se conserva tachado, no borrado.

**Rama `main-nuevo` · HEAD `c728361`** *(medido con `git rev-parse --short HEAD`. **Un hash
escrito a mano envejece en horas**: antes de fiarse de éste, se vuelve a medir.)*

**En campo sigue la V8.4 (`e303485`, 31/07).** La compuerta en verde dice que los modelos y los
arneses de PC no encuentran nada; **no dice que el firmware funcione sobre la tarjeta**.

> 🔴 **DEPENDENCIA DE ESTA MÁQUINA QUE NO ESTÁ EN EL REPOSITORIO: `D:\toolchain\mingw64`.**
> Es la copia del `gcc` de host **fuera de la ruta con `ñ`** de `Diego.Zuñiga`, cuyo `ld` no abre
> `crt2.o` aunque el fichero exista (N-44). **Si esa carpeta desaparece, los siete arneses que
> compilan C++ real caen a `ABORTADO` a la vez** — y la compuerta lo dice en una línea que nadie
> lee cuando el resumen de arriba parece normal. Ya pasó el 05/09: `13 PASS · 7 ABORTADO`, y no se
> enteró nadie.
>
> **Por eso un `ABORTADO` se lee SIEMPRE, y un número de `PASS` sin su total al lado no significa
> nada.** Esto vive aquí y no en `CLAUDE.md` porque **es estado de una máquina, no una regla**:
> quien clone el repositorio en otro equipo no tiene este problema, tiene el suyo.

---

## ✅ Lo que está CONFIRMADO EN COBRE

**La última cinta es del 05/09 a las 22:19, y cargó `42a52cd`.** Lo demuestra, no lo afirma:

| | evidencia en la cinta |
|---|---|
| **N-42** el Modo Automático mueve las luces | cerrado en la sesión de banco del 04/09 por la noche, con el responsable delante: *«ahí cambia ese amarillo y este a verde. Ahora está funcionando»*. Arreglo en `ceb8cc5` |
| **N-146** el ámbar re-armado | `$ACK,CMD:SET_MODO:AMBAR,RESULT:REARMADO` a las 22:19:40, y el `ESTADO` pasa de `ROJO` a `FALLO COM` |
| **N-149** el Esclavo en la trama | `ESC:AMBAR` y `ESC:ROJO` viajando en todos los `$STATUS` |
| **N-145** la hora del DS3231 | `HORA:22:19:58` — el campo dejó de ser `--:--:--` |
| **N-142** el aviso del ámbar | el `ESC:` sigue al ámbar del Esclavo sin esperar los 25 s |

## 🔴 Lo que se ARREGLÓ DESPUÉS de esa cinta y NO ha pasado por una tarjeta

| | commit | lo que enseñó de paso |
|---|---|---|
| **N-151** `DAR PASO` en un modo sin coordinador trababa el cruce para siempre | `273b315` | el equipo decía que SÍ a una orden que no iba a ejecutar, y se quedaba PEOR que antes de pedirla. Lo vigila `maestro_12_dar_paso_sin_coordinador` |
| **N-152** `CANCELAR_AMBAR` no avisaba al Maestro | `d6ce67e` | **en `MODO_AMBAR` el Maestro estaba SORDO**: un comando copiado de N-142 habría entrado sin lector |
| **N-150** el ciclo no arrancaba tras aplicar tiempos | `414b962` | `ACK_TEXTO` no tenía `SET_TIEMPOS|OK`: el genérico lo pintaba **en verde** |
| **los parsers de la app** | `414b962` | eran **TRES**, y `parseError()` leía por posición con una prueba de un formato que ningún micro emite |
| **la línea falsa de `pines.h`** | — | decía que `BOTON1`/`BOTON2` son `INPUT_PULLUP` activos en BAJO. El fuente hace `INPUT` pelado y lee `== HIGH`. Es la cabecera que todo el mundo lee antes de cablear |

> **De ninguno hay una sola prueba en cobre. Que la compuerta esté en `20/20` no dice nada de
> eso.**

---

## 🔴 ABIERTO — por orden de lo que duele

**Los tres primeros NO los cierra nadie escribiendo código.** Confundirlos con los que sí es como
se acumula un `20/20` que no acerca una tarjeta (`CLAUDE.md` §2.bis).

1. **`BAT:--`** — falta un **divisor de tensión** y una entrada analógica. La causa está MEDIDA:
   `grep -rn analogRead` sobre las cuatro carpetas da **cero**, y N-108 puso el `--` a propósito
   para que nadie leyera un 12,6 V que era un literal.
2. **`J16` p1 lleva 12 V crudos.** Taparlo es **obligatorio en cada equipo que se monte**
   (N-120), no una cautela de banco.
3. **Matriculación por ID de Bluetooth**, no por nombre y sin hacerlo a mano. Es una **decisión
   de protocolo del responsable** y cuesta bytes: `RF_Packet` son 4 bytes
   `{msgID, command, param, crc}` y **no tiene campo de dirección**; el CRC cubre 3. Meter
   direccionamiento cambia el contrato de la radio en las dos puntas. **Aplazado a después del
   banco por decisión del responsable.**
4. **`buildCommand()`** de esa misma suite sigue siendo copia a mano de `generarComando()`, que
   tiene cero llamadores **a propósito**. Unificarlo mueve la pregunta abierta del `*XX` que
   vigila `simulador_puente_esp32.py`.
5. **Retirar `parseStatus()` de verdad** exige tocar `simulador_app_bluetooth.py` y
   `documentos_03`. Hoy queda como **vista tipada** encima del único partidor, no como un segundo
   parseo — que era el defecto.
6. **`FW-N53`** — la inhibición de secuencias ya está en las dos puntas; falta decidir si se
   redefinen los gestos (hoy Auto es `A·A·A` y Ámbar `B·B·B`). Es **decisión de spec**: cambia el
   Manual 1, el Manual 3 y el adiestramiento del operario.
7. **`APP-APK`** — recompilar la APK. La del disco es de `a8e1ceb` (28/08) y el árbol le pasó
   por encima después.

> ~~**`validateTiempos()` de los unitarios de la app sigue en 1..15 min**~~ — **CERRADO el 07/09**:
> sincronizado a 3..15 min (`VERDE_MIN_MIN = 3`, `limites_ciclo.h`) con comprobaciones en los bordes
> (1 y 2 min rechazados). Suite en 32/32 PASS.

> ~~**`MANDO_A`/`MANDO_B` no responden — `0,6 V` en reposo (N-118), y van cableados**~~ —
> **REFUTADO el 05/09** (`d020f3c`), con la medida del propio banco: en `617bd00` —el binario que
> estaba en la tarjeta— `BOTON1/2` iban en `INPUT_PULLUP` y `CAM_C/D_PIN` en `INPUT` pelado; el
> paso 20 midió **9,92–9,94 kΩ en los cuatro pines** y **`0,6 V` sólo en los dos con pull-up**.
> Mismo cobre, distinto `pinMode`, distinta tensión. **El banco había corrido las dos ramas del
> experimento en la misma tabla y nadie lo leyó así.** Y además es moot: **ya no hay mando**
> (`D-1`). La tensión de `J16` p5/p8 queda como **prueba CANCELADA**, no como casilla pendiente:
> una casilla abierta invita a puentear `J16`, que es el gesto que precedió al calentamiento del
> paso 29.
>
> ~~**N-145 no se puede dar por probada: falta comprar el `DS3231`**~~ — **RESUELTO el 05/09 por
> el responsable: cada ESP32 lleva su reloj con pila propia** (`D-9`, `D-15`). Lo que sigue sin
> verificar es la dirección `0x68` sobre el módulo.
>
> ~~**N-148 · la app no pide confirmación de vía al dar ámbar en Manual**~~ — **CERRADO EN SOFTWARE**: `SET_MODO:AMBAR` está en la tabla `VIA_MANIOBRA` de `app.js` y pasa por `confirmarVia()`, igual que `MANUAL:CAMBIAR_TURNO` y `SET_MODO:AUTO`. Y el texto **no dice «se pone en ámbar»** —eso ya lo dice el rótulo del botón—: dice lo que significa en la calzada, que los dos postes quedan en intermitente a la vez. 🔴 **Sin prueba en tarjeta.**
>
> ~~**N-106 · el ámbar de emergencia de la app no saca al Esclavo del Degradado, y aun así se
> contesta `$ACK`**~~ — **CERRADO EN SOFTWARE**: `Esclavo/src/bluetooth.cpp` llama hoy a
> `degradado_salir()` por `salidaDegradadoIniciada()`, que **pregunta la misma guarda que ella
> tiene** —no una parecida— y contesta distinto por rama: `SALIENDO_TODO_ROJO`,
> `SALIDA_YA_EN_CURSO` o `$ERR ... REPITA`. `app_03_sin_ok_mudo` da **18/18**. 🔴 **Sin prueba en
> tarjeta.** *(Este fichero lo publicó como abierto apoyándose en un `grep` de `degradado_salir` que
> ya no daba cero — §4: un cero de `grep` es «mi patrón no encontró», y aquí ni siquiera lo era.)*
>
> ~~**A-12 · el Modo Inteligente corta un verde a los 15 s**~~ — **arreglado**: el
> `tiempoActual >= 15000UL` ya no está en `modo_inteligente.cpp`, y `app_11_rangos_de_tiempos`
> volvió a verde. `DECISIONES.md` todavía lo lista como abierto con la compuerta en rojo; **esa
> fila está caducada** y no se toca desde aquí.

---

## 🛑 BLOQUEANTES

| # | Qué está bloqueado | Qué lo desbloquea | De quién es |
|---|---|---|---|
| 🛑 **BLQ-3** | **La tarjeta Maestro dañada** (**N-116**): se calienta y deja de funcionar a los ~30 s. **El firmware queda descartado por censo**, así que reflashear no lo arregla. **La causa que sostiene el cobre es latch-up**: los 5 pines de bornera van desnudos al die y `J16` p1 lleva 12 V crudos | **Medir el consumo del riel de 3,3 V en frío** con fuente limitada en corriente, antes de energizar. 🛑 **No reenergizar «a ver si pasa»** | **Responsable** |
| 🛑 **BLQ-6** | **Nada de lo arreglado después de la cinta del 05/09 ha pasado por una tarjeta**: N-150, N-151 y N-152 tocan el camino del ámbar y del Modo Manual, o sea **lo que decide qué ve un conductor** | **Una carga y una pasada de los pasos de ámbar, rojo total y `DAR PASO`.** Nada lo sustituye | Banco |
| 🔴 **BLQ-5** | **Todas las tarjetas, no sólo la dañada** (**N-120**): la placa protege sus **9 salidas** con 220 Ω y optoacoplador, y **ninguna de sus 5 entradas de campo** | Revisión de diseño (**2K2 en serie**). **Mientras tanto: tapar el pin de 12 V de `J16` es obligatorio en cada equipo** | **Responsable** |
| 🔴 **BLQ-4** | **La única vía de operación del equipo**: el ESP32 no se anuncia por Bluetooth de forma fiable (**N-117**). Arreglado en el árbol el 04/09, **causa no confirmada en el módulo** | **1º (30 s, gratis): buscar el equipo en la lista del teléfono.** **2º: monitor serie a 115200 sobre el CP2102, ANTES de reflashear** | Técnico |
| **BLQ-2** | 🟠 **El cristal `Y2`.** No oscila en la tarjeta medida (N-17, N-37, medida de banco del 01/08). La mitad de firmware **ya está hecha** (N-80): `SET_RTC` contesta con motivo en vez de mentir | **Diagnosticar el `Y2` de la SEGUNDA tarjeta** para decidir entre reparar el cristal o reloj de software | **Responsable** |
| ~~**BLQ-1**~~ | 🟢 **CERRADO el 31/08 — es un `ESP32-WROOM-32` clásico**, con `BR/EDR` y por tanto SPP | — | — |

---

## 📏 VERIFICACIÓN EN ESCRITORIO — lo que dice la última acta

**Compuerta: 19 PASS · 1 FALLA · 0 ABORTADO**, o sea que sale con `1` — y ese `1` es el hallazgo de `decisiones_01_anclas`, no una regresión: `D-14` y `D-17` están vigentes en `DECISIONES.md` y no tienen ancla en el firmware. Cifras **copiadas del acta
[`evidencia/2026-09-07_compuerta.txt`](evidencia/2026-09-07_compuerta.txt)**, no escritas a mano —
lo comprueban `documentos_01`, `documentos_04` y `documentos_05` en cada corrida.

| | |
|---|---|
| Flash | Maestro **88.4 %** (**57956** de 65536 B → **7.580 B libres**) · Esclavo **68.5 %** (44912 B) · Repetidor **20.6 %** · ESP32 **35.7 %** |
| Banco por packs | **1230/1230 comprobaciones** en **77 packs** — D-14, D-20, D-21, D-22 y D-23 integradas y ancladas |
| Arneses que compilan C++ real | 271/271 pantalla · **99/99** automático · 22/22 ciclo · **42/42 dos puntas** · **18/18 Degradado a dos puntas** |
| Puente ESP32 | **101/101** |
| App | **235/235** jsdom · 58/58 funcional · 32/32 unitarios · **61/61** TDD |

> 🔴 **El acta se midió sobre HEAD `0b06f7d` y con el árbol CON CAMBIOS SIN COMMITEAR** —lo dice
> su última línea—, así que estas cifras **no corresponden exactamente** a ningún commit. Para que
> sean reproducibles hay que volver a correr la compuerta con el árbol limpio.

🔴 **Y el verde sigue sin ser un entregable — con el contraejemplo delante en vez de como
advertencia.** El banco del 3-4/09 encontró **tres defectos que ninguna línea de instrumento podía
ver**, porque ninguno es una propiedad del fuente. Lo que sí hay que apuntarles: **no fallaron en
nada de lo que sabían mirar**. La medida del desequilibrio —**2,31 a 1** el 02/09, **2,74 a 1**
acumulado el 05-06/09— está en el README y en `roadmap.md`.

⚠️ ~~19 PASS | 1 FALLA | 0 ABORTADO · 1180/1180~~ y ~~18 PASS | 1 FALLA | 1 ABORTADO · 995/1010 en
70 packs~~ — **cifras de corridas intermedias del 05/09**, con el árbol a medias mientras varios
agentes trabajaban a la vez. **Estas cifras se vuelven a copiar del acta en cada corrida; no se
escriben a mano** (N-93).

---

## 🧭 MAPA RÁPIDO DE ARTEFACTOS

| Componente / Documento | Ubicación | Nota |
|---|---|---|
| **App móvil de campo** | [`05_Funcional/App_Semaforo/`](05_Funcional/App_Semaforo/) | Frontend Web Bluetooth / WebView, selector de cruces y Courier RTC |
| **APK Android** | [`05_Funcional/IOT_VIAL_Semaforos_2026-08-28_a8e1ceb_SIN_BANCO.apk`](05_Funcional/IOT_VIAL_Semaforos_2026-08-28_a8e1ceb_SIN_BANCO.apk) | 🔴 **caducada**: el árbol le pasó por encima después. Sigue habiendo que recompilar (`APP-APK`) |
| **Paquete ZIP de entrega de versión** | 🔴 **SIN GENERAR** | No se genera hasta pasar banco. Ver la skill `entregar` |
| **Guía de cableado y banco (HTML)** | [`05_Funcional/Guia_Cableado_y_Pruebas_Banco.html`](05_Funcional/Guia_Cableado_y_Pruebas_Banco.html) | **El documento de conexiones que se entrega**, y el **formulario de vuelta**: se rellena y se devuelve en PDF |
| **Esquemático KiCad bueno** | [`01_Firmware/Controladora_Semaforos/`](01_Firmware/Controladora_Semaforos/) | 649 KB con LCD, botones y el canal del motor, y el `.kicad_pcb` de 2,1 MB. La copia incompleta de `03_Hardware_Tarjeta/KiCad/` **se borró el 27/08** |
| **Informe de banco 3-4/09** | `evidencia/Informe_Pruebas_Banco_Semaforos_V9.0.pdf` | 24 de 29 pasos, sobre `617bd00` |

### Manuales que siguen describiendo el aparato anterior

| Manual | Qué dice de más | Qué es cierto hoy |
|---|---|---|
| **1 · Usuario** | cámaras en `PB0`/`PB8`, mando y pulsadores | **2 cámaras en `J16` p10/p12**; el mando **no se monta** (`D-1`) y la operación es por app (`D-16`) |
| **3 · Protocolo de pruebas** | ⚠️ la cuenta de «80 pruebas» es la del protocolo **anterior** a la reescritura | **MEDIDO el 01/09**: 75 identificadores únicos, 47 casillas `CUMPLE` y 22 «No se firma» — **47 + 22 = 69, no 75**. 🔴 **No se publica un recuento nuevo porque no sale limpio**: seis pruebas no caen en ningún grupo, y hasta saber por qué cualquier cifra sería inventada |
| **9 · Cámara IA** | contactos en `PB0` (Demanda) y `PB8` (Umbral) | **`PB8` ya no es destino de cámara**; el pinout se muda a `J16` p10/p12 |
| **10 · Bluetooth** | puerto `USART1` en `PA9`/`PA10`, y un módulo SPP dedicado | **`USART1` remapeado a `PB6`/`PB7`, salida por `J17`** (N-76), y **lo sustituye el ESP32** |
| **11 · RTC** | `DS3231` en `PB0`/`PB8` del STM32 | **el `DS3231` vive en el ESP32** (`GPIO21`/`GPIO22`, pila propia) |
| **13 · Expansión I²C** | sacar bus de `PB0`/`PB8` | **su §4 queda sin sujeto**: el I²C ya no vive en el STM32 |

---
---

> ## 📚 DE AQUÍ ABAJO ES HISTÓRICO
>
> **El estado vivo está arriba.** Lo que sigue son las decisiones de la V9.0 con su fecha y su
> motivo: se conserva porque una fila tachada con su motivo no se vuelve a proponer y un hueco sí.
> **El porqué completo, al día, vive en [`roadmap.md`](roadmap.md)** — este fichero no es la
> bitácora.

## 📌 Las cuatro decisiones que reconfiguraron la V9.0

### 1. ~~Sistema de 4 cámaras IA AcuSense~~ → **2 cámaras de demanda**

* **Lo que sigue en pie:** descartar ordenadores externos. La analítica corre dentro del
  procesador AcuSense de las cámaras Hikvision, y el controlador consume **un contacto seco** por
  cámara: no hay red, ni imagen, ni vídeo (`D-12`). Todo cambio de sentido respeta el **Despeje
  Todo-Rojo** (`cfgDespejeSeg`).
* ⚠️ ~~Maestro: Cámara 1 (`PB0`) + Cámara 2 (Umbral, `PB8`); Esclavo: Cámaras 3 y 4~~ — **falso
  desde el 28/08**: son **dos cámaras de demanda, una por poste**, en **`J16` p10 (`PB14`) y p12
  (`PB15`)**.
* ✅ **La medida `M3` está cerrada desde el 03/09 y las cámaras se cablean** (`D-3`): pull-down
  real de 10 kΩ en las cuatro posiciones, `p10` y `p12` a **0 V** en reposo, entrada **activa en
  ALTO** — que es lo que el firmware ya hacía. ~~🔴 No se cablea todavía: polaridad en
  contradicción~~ — **caducado.**
* 🔴 **`J16` p1 lleva 12 V crudos.** Se tapa físicamente antes de cablear nada.
* ~~`p5`/`p8` se dejan vacíos «de colchón»~~ — **REFUTADO el 31/08**: son `MANDO_A` (`PB9`) y
  `MANDO_B` (`PB13`), el firmware **sigue leyéndolos**, y qué se pone ahí es la decisión abierta
  `A-2`. Cualquier cosa que se cierre en esos pines entra por el reconocedor de secuencias del
  mando.

### 2. Telemetría Bluetooth (estándar Baliza)

* ⚠️ ~~el módulo Bluetooth en `USART1` (`PA9` TX, `PA10` RX)~~ — **dos cosas cambiaron y las dos
  están medidas:** (1) **N-76 remapeó `USART1` a `PB6` TX / `PB7` RX**, con salida por `J17`
  p3/p2; (2) el **módulo SPP dedicado se retira y lo sustituye el ESP32**.
* **Desacoplo hardware `U3`:** `PA8` (`RS485_IN_DE_RE`) en `HIGH` permanente, para poner en Hi-Z
  la salida `RO` y evitar choque con el `TXD` del módulo.
* **Caja Negra de alarmas:** `$ALARM,NODE:...,EVENTO:FALLO_RF_...*XX`. ⚠️ ~~SFTY-6 a los 12 s~~ →
  **son 25 s desde N-71**: el techo de 12 s estaba **por debajo** del peor caso de reintentos
  (20,5 s), así que los reintentos 4 y 5 no se ejecutaban nunca.

### 3. N-53 — interferencia entre el mando y la pantalla

* **Causa raíz:** los relés remotos van en paralelo con los pulsadores frontales (`PB9` Botón 1 /
  `PB13` Botón 2). Al pulsar tres veces rápido en `AJUSTAR HORA`, el firmware interpretaba `A·A·A`
  o `B·B·B` y cancelaba la edición.
* **Lo que hay en el firmware hoy:** `secuenciasInhibidas()` en las dos puntas, y el Degradado
  exige cuatro pulsos alternados `A·B·A·B`. **Pero Automático sigue siendo `A·A·A` y Ámbar
  `B·B·B`.**
* **Lo que este apartado prometía y NO está:** la redefinición a `A·B·A`, `B·A·B`, `B·A·B·A` y
  `A·A·B·B`, escrita como *«Solución V9.0»* en pasado. **El Manual 3 sí decía la verdad** — el
  documento del auditor estaba bien y **el estado interno era el que mentía**, que es peor, porque
  es el que se usa para decidir qué falta hacer. Ver `FW-N53`.

### 4. La arquitectura del 28/08 — el ESP32 es expansión, no controlador

**El documento completo es
[`05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md`](05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md).
Aquí no se copia: se enlaza.**

* **El STM32 sigue siendo el controlador.** Conserva las luces, la barrera (`J15`), el buzzer, la
  radio LoRa (`USART3`) **y las cámaras**.
* **El ESP32 es un módulo de expansión y no manda sobre las luces.** Se lleva el **reloj
  `DS3231`** y el **Bluetooth**, con **fuente propia desde 12 V**: el accesorio no puede tumbar al
  que manda.
* **Se retiran** la pantalla LCD (`D-17.bis`) y ~~los cuatro pulsadores y el mando de 4 relés~~ →
  el **hardware** del mando tampoco se monta (`D-1`, 05/09), pero **su código se queda**.

> 🔴 **Las dos consecuencias que fijan el orden, y no son opinión:**
> **(a)** ~~por Bluetooth sólo se alcanzan tres de los ocho modos, sin `SET_MODO:MENU`~~ →
> **REFUTADO el 31/08 por el propio firmware (N-100):** `SET_MODO:MENU` **existe**, entra **sin
> PIN**, y existen también `SET_MODO:DEGRADADO`, `SET_MODO:INTELIGENTE`, `SET_MODO:ALCANCE`,
> `REINICIAR_RELOJ` y `DEMANDA`. **La salida por app ya está construida** (`d34cfe2`, N-78).
> **(b)** Retirar el código del mando **no deja `if` inertes: borra un veto.** `mando_ambarLocal()`
> tiene **cinco llamadas vivas** y `ambarLocal = true` se arma en **un único sitio**, el
> `case ACC_AMBAR` al que sólo se llega por `B·B·B`. Sin ese armador la bandera no se arma jamás,
> los `if` se vuelven siempre-verdaderos y una orden de radio puede sacar al Esclavo de un ámbar
> que un operario dejó puesto a propósito: **SFTY-21 desapareciendo por sustracción.**

---

## 🟢 Las cinco pasadas que dejaron el banco como está

| | qué encontró | por qué sigue escrito |
|---|---|---|
| **N-46** (05/08) | los tres validadores monolíticos imprimían `FALLA` y salían con código `0` | se retiraron exigiendo que los packs sumaran **exactamente** sus comprobaciones y que el **texto** de cada una coincidiera: Costura `41 = 41`, Maestro `64/67`, Esclavo `31 = 31` |
| **N-62** (27/08) | tres packs `documentos_*` nuevos, **que nacieron con 10 fallos reales**: el README publicaba 32 rutas y 86,4 % de flash contra las 38 y el 92,8 % del acta que él mismo citaba | es la única forma de saber que un instrumento mide |
| **N-75** (28/08) | el rewrite de la app entró con **dos instrumentos en `ABORTADO`** —los únicos dos que ejercen la app— y detrás entraron cuatro defectos: app sorda, PIN que se autorizaba solo, parser de un protocolo que nadie habla, y trabajo sin interfaz | **un `ABORTADO` no se apunta para luego: se arregla antes de mirar nada más** |
| **N-93** (31/08) | tres cifras de la app estaban escritas a mano y eran viejas, y `documentos_01` **no vigilaba ninguna de las tres** | la fila existía —la cobertura la veía— y su número no lo miraba nadie. **Un hueco no grita** |
| **N-112** (01-02/09) | el propio pack **alternaba**: 16/1, 17/0, 16/1 sobre un árbol idéntico, porque emitía menos comprobaciones cuando el acta salía en rojo | **el número de comprobaciones que emite un pack no puede depender de su propio veredicto** |

> ⚠️ **Y lo que N-75 dejó abierto y sigue abierto**, porque son decisiones y no código:
> el **modo día de fondo claro** contra el sol directo (es la única intervención demostrada; el
> contraste WCAG ya está medido y en AAA salvo el rojo) y los `prompt()` nativos para crear y
> renombrar cruces.

---

## 🟡 El orden de ejecución vigente — seis fases

| Fase | Qué | Estado |
|---|---|---|
| ~~**1**~~ | Los comandos que faltaban en el Maestro: `SET_MODO:DEGRADADO`, `MENU`, `ALCANCE`, `INTELIGENTE`, `REINICIAR_RELOJ` y `DEMANDA` | ✅ **HECHA** en `d34cfe2` (N-78) |
| **2** | ~~Ignorar los pulsadores~~ → **ignorar SÓLO los 3 y 4** (`PB14`, `PB15`) · `FORZAR_ROJO` del Esclavo · `TEST_LEDS` | 🔴 **La redacción anterior era el peligro concreto de esta tabla: ejecutada literal BORRA `ambarLocal` y con él el veto de SFTY-21** |
| **3** | **Cámaras a `J16`** (p10/p12) y retirar pantalla, menú y `AiBus` | ✅ el cableado ya no está bloqueado (`M3` cerrada) |
| **4** | **Telemetría honesta** | `$STATUS` es el único tablero que existe y aún trae campos que no se miden. **Un campo que no se mide se retira o se marca; no se deja con aspecto de medida** |
| ~~**5**~~ | ESP32: watchdog, `DS3231` y puente Bluetooth | ✅ **HECHA** — `ESP32_Expansion/src/vigilante.cpp` |
| **6** | **BANCO** | 🛑 **Sigue siendo EL bloqueante y nada lo sustituye.** Ni la compuerta en verde, ni los arneses que compilan C++ real, ni esta hoja de ruta |

### Lo que queda por hacer, después del banco

| # | Qué | Depende de |
|---|---|---|
| **C1** | **SFTY-29: presencia como veto** | ~~decidido el 27/08: van las 4 cámaras~~ ⛔ **REVOCADO el 28/08: van DOS**, y con ello desaparece el sujeto de SFTY-29 |
| **C2** | Reloj `DS3231` por I²C en el STM32 | ⛔ **anulado**: el reloj vive en el ESP32 (`D-9`) |
| **C3** | **`FW-PAIR`** (byte `PAIR`, `SET_PAIR`, descarte de lo ajeno) | el más caro: toca el respaldo `DR9`, la `FIRMA` y `maestro_02_respaldo` |
| **C4** | **`FW-N53`**: decidir secuencias | es **decisión de spec**, no código |
| **D3** | **Campo**: Courier RTC en sitio y puesta en servicio | **sólo con banco pasado, sin excepción** |

> **Sobre lo que queda manda el flash:** el Maestro va al **88.4 %** y quedan **7.580 B libres**.
> No caben todas. Se mide antes de escribir cada una, no después — `CLAUDE.md` §7.
