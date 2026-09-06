# 🚦 Manual de Operación y Comportamiento del Sistema (V8.9 Definitiva)

Este manual define el **"Ground Truth"** (la verdad absoluta) de cómo DEBE comportarse el sistema, sirviendo como base para validar que las simulaciones y el código cumplan con la especificación.
Todas las operaciones están alineadas al **Manual de Señalización Vial de Colombia (Resolución 2024 - MinTransporte)**.

**Última revisión:** 31 de agosto de 2026 — **el apartado 6 (cámaras) estaba MAL y se ha corregido.**
Mandaba cablear las cuatro cámaras a `PB9` y `PB13`, que son **los dos canales del mando de relés**, y
además con la polaridad invertida. Es `N-105` de [`roadmap.md`](../roadmap.md). El texto viejo **no se
borra**: queda tachado en su sitio con el motivo, porque una vía descartada que desaparece en
silencio se vuelve a proponer. **Nada de esto ha pasado prueba de banco y este manual no autoriza a
cablear nada** — ver el apartado 6.6.

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
- **Configuración:** La interfaz del menú LCD permite configurar tiempos de despeje de **10 a 90 segundos**, con **piso mínimo de 10 s** por seguridad vial. Lo impone el firmware, no la pantalla: `DESPEJE_SEG_MIN = 10, DESPEJE_SEG_MAX = 90` en `01_Firmware/Maestro/src/modo_automatico.cpp:34`, y `modoAutomatico_fijarTiempos()` **rechaza** (`return false`) cualquier valor fuera del rango, venga del menú o de un comando por Bluetooth. ✅ **MEDIDO EN EL FUENTE.**

> ### ⛔ Este apartado publicó ~~«de 5 a 999 segundos, piso mínimo de 5 s, hasta 16.6 minutos»~~ hasta el 31/08/2026. Era falso por partida triple.
>
> No se borra —una vía descartada que desaparece en silencio se vuelve a proponer— y se explica por qué,
> porque las tres razones son distintas:
>
> 1. 🔴 **El piso publicado era la MITAD del real.** Un operario que configurase **5 s** de despeje
>    creyendo que el manual manda, se encontraría con que el equipo **rechaza el valor** y se queda con
>    el anterior. El despeje es *«el tiempo que garantiza que el tramo quedó vacío antes de dar verde al
>    otro lado»*: **los 10 s no son un número redondo, son el tramo más corto que esta casa ha montado**
>    (comentario de `modo_automatico.cpp:29-31`). Publicar 5 s invita a pedir un despeje que **no da
>    margen**, y esto está en la matriz de seguridad como **SFTY-4**.
> 2. 🔴 **El techo publicado era 11 veces el real.** El máximo es **90 s**, no 999. Quien planifique un
>    túnel de 500 m contando con *«hasta 16.6 minutos»* está planificando sobre un equipo que no existe.
> 3. 🔴 **Y 999 no fue nunca representable.** `despejeSeg` es un **`uint8_t`** (`modo_automatico.cpp:34`,
>    `:37`): el valor más grande que cabe en ese tipo es **255**. La cifra publicada no era una
>    configuración desafortunada — era un número que **ningún firmware podría haber aceptado jamás**, y
>    llevaba meses escrito como si fuera una capacidad del equipo.

---

## 2. Comportamiento en Destello / Intermitente (Bajo Flujo)

Según el Manual de Señalización (2024), si el flujo vehicular baja al 50% o menos durante 4 horas o más (usualmente operación nocturna), el sistema debe pasar a operación intermitente.
- **Funcionamiento:** Un semáforo parpadea en 🟡 **Ámbar** (Precaución - vía principal) y el otro en 🔴 **Rojo** (Pare - vía secundaria), o ambos en Rojo Intermitente para pasos de igual jerarquía.

---

## 3. Comportamiento de la Interfaz y Menú (LCD ST7920)

> ### ⛔ APARTADO DEROGADO EL 05/09 — LA PANTALLA Y LA BOTONERA SE RETIRAN DEL EQUIPO
>
> **`D-17.bis` de [`DECISIONES.md`](../DECISIONES.md), que deroga `D-6`:** *«la pantalla LCD ya no va,
> pues los pines y el equipo lo quitamos»*. **Todo se opera por la app.**
>
> **Y el matiz que hay que sostener, porque decir «la pantalla no existe» sería otra frase falsa: se
> retira del EQUIPO, no del código.** `lcd.cpp` y `menu.cpp` siguen compilando, y `Validacion_LCD`
> sigue dando `271/271` sobre un framebuffer en el PC. Lo que muere es la **INTERFAZ**. Por eso lo de
> abajo se **tacha y se conserva** —convención del repositorio: lo derogado no se borra— y sigue
> describiendo con exactitud lo que el firmware hace cuando ya nadie puede verlo.

~~El acceso a la pantalla LCD y los botones de configuración es crítico para los operarios.~~
- ~~**Regla de Oro (Independencia de Red):** El operario DEBE poder acceder al menú de configuración (para elegir modo Manual, Automático o Inteligente, y fijar tiempos) **incluso si las radios están apagadas o no hay comunicación con el esclavo**.~~

  ⛔ **DEROGADA el 05/09 por `D-17.bis`. Es el único requisito de nivel superior que este documento
  deroga, y por eso lleva sustituto MEDIDO en vez de un borrado silencioso.**

  ✅ **LO QUE LA SUSTITUYE, Y SÍ CUBRE EL REQUISITO.** Las tres palancas que la Regla de Oro exigía
  —elegir Manual, Automático o Inteligente, y fijar tiempos— existen por Bluetooth contra el
  Maestro, y **ese enlace es LOCAL**: STM32 ↔ ESP32 por **USART1 (`PB6`/`PB7`, conector `J17`)** y de
  ahí SPP al teléfono. **No pasa por las radios E90**, que es exactamente lo que el requisito pedía.
  El `grep` que lo encuentra, corrido el 05/09 —se cita el símbolo, no el número de línea suelto:

  ```
  $ grep -n 'strcmp(accion, "SET_MODO:\|strncmp(accion, "SET_TIEMPOS:' 01_Firmware/Maestro/src/bluetooth.cpp
  515:  if (strcmp(accion, "SET_MODO:AUTO") == 0) {
  520:  } else if (strcmp(accion, "SET_MODO:MANUAL") == 0) {
  527:  } else if (strcmp(accion, "SET_MODO:AMBAR") == 0) {
  570:  } else if (strcmp(accion, "SET_MODO:MENU") == 0) {
  591:  } else if (strcmp(accion, "SET_MODO:ALCANCE") == 0) {
  602:  } else if (strcmp(accion, "SET_MODO:INTELIGENTE") == 0) {
  613:  } else if (strcmp(accion, "SET_MODO:DEGRADADO") == 0) {
  659:  } else if (strncmp(accion, "SET_TIEMPOS:", 12) == 0) {
  ```

  🔴 **LO QUE EL SUSTITUTO NO CUBRE. Se ESCRIBE, en vez de taparse con la derogación:**

  1. **`AJUSTAR HORA` no tiene `SET_MODO` que lo sustituya: `MODO_HORA` es hoy el único modo
     inalcanzable de los ocho.** Su **único** armador es `modoActual_set(MODO_HORA)` en `menu.cpp`,
     detrás de un `botonAceptar()` que es `return false;`, y en la lista de arriba **no hay
     `SET_MODO:HORA`** — el propio `bluetooth.cpp` lo dice en su comentario: *«la puerta esta tapiada
     por DOS sitios a la vez, y por Bluetooth no existe SET_MODO:HORA»*. ✅ **MEDIDO:**

     ```
     $ grep -rn "modoActual_set(MODO_HORA)" 01_Firmware/Maestro/src/
     01_Firmware/Maestro/src/menu.cpp:129:        case 1:  modoActual_set(MODO_HORA);      break;
     $ grep -n "^bool botonAceptar" 01_Firmware/Maestro/src/botones.cpp
     659:bool botonAceptar() { return false; }
     ```

     **Y no deja al técnico sin reloj, que es lo que importa:** la hora se pone con `SET_RTC:` y se
     lee con `LEER_RTC` **contra el ESP32, que es donde el reloj está** (`D-15`). La rama `SET_RTC:`
     del Maestro se conserva **callada a propósito** —consume la orden para que no salga un segundo
     acuse—, y quien contesta es `ESP32_Expansion/src/despachador.cpp`, con siete finales distintos,
     uno por motivo. Lo que se pierde con la pantalla es el **menú** `AJUSTAR HORA`, no la puesta en
     hora.

  2. **La independencia CAMBIÓ DE SITIO; no se conservó.** La Regla de Oro decía *«no depende de las
     radios»*; el sustituto **depende del teléfono**. Eso no es una avería ni un descuido: es
     **`D-16`** de [`DECISIONES.md`](../DECISIONES.md), ya escrito como **propiedad declarada del
     sistema**. Se anota aquí para que nadie lea esta derogación como si no costara nada: sin
     teléfono emparejado no hay forma de cambiar el modo en el poste.
- **Comportamiento en Menú:** En el Menú Principal, si hay comunicación el Maestro mantiene **🔴 ROJO FIJO continuo en ambos semáforos** sin congelar la pantalla. Si no hay comunicación, indica orfandad pasando a Amarillo Intermitente.
- **Arranque Inmediato:** Al seleccionar un modo en el menú, el sistema aplica inmediatamente el tiempo de Despeje All-Red en ambos extremos.

### Prueba de Alcance
Cuarta opción del Menú Principal. Muestra **calidad de enlace en %**, barra gráfica, **tiempo de respuesta** en ms y fallos consecutivos, actualizándose cada 3 segundos. Permite determinar la cobertura real de radio desplazando el equipo, en lugar de estimarla.
Mientras está activa, ambos semáforos permanecen en **🔴 Rojo Fijo** (o Amarillo Intermitente sin enlace), igual que en el Menú. **No arranca ciclos.** Se sale con el Botón 4.

---

## 4. Comportamiento ante Fallas (Fail-Safe & Self-Healing Real)

1. **Pérdida de Comunicación (SFTY-6):** Si se pierde comunicación por más de **25 s de silencio**, el sistema entra automáticamente en `C_FALLO` / `S_FALLO` (🟡 **Amarillo Intermitente**). En `C_FALLO`, el Maestro envía `CMD_GO_RED` para obligar al Esclavo a pasar a Rojo o Amarillo Intermitente por timeout. ✅ **MEDIDO:** `SFTY6_SILENCIO_MS = 25000UL` en `01_Firmware/Maestro/include/protocolo.h:149` y en `01_Firmware/Esclavo/include/protocolo.h:149` — el umbral vive **una sola vez por punta** y las dos líneas son idénticas.
   > ⛔ Este manual publicó ~~12.0 segundos~~ hasta el 31/08/2026. Era el umbral anterior a **N-71**, y no
   > era sólo una cifra vieja: **12 s quedaban por debajo de los ~20,8 s que el ciclo necesita** para
   > agotar sus cinco reintentos, así que **los reintentos 4 y 5 no se ejecutaban nunca** — el ámbar por
   > orfandad saltaba antes. Nada lo delataba, porque irse a ámbar es un comportamiento razonable.
2. **Auto-Recuperación Autónoma (Self-Healing Real):** Al restablecerse la señal de radio, el sistema **NO requiere reinicio manual**. Limpia automáticamente el registro de duplicados (`protocolo_resetReplayProtection()`), fuerza Rojo Estático (All-Red) de 15 segundos en ambos semáforos para limpiar la vía y reanuda el ciclo lumínico sin intervención técnica.
3. **Cuelgue de Procesador (Ruido EMI):** El Watchdog interno (`IWatchdog` activo a 4.0s) reinicia el procesador ante interferencias severas.

---

## 5. Resiliencia RF: Ráfaga configurable y Ventana Deslizante (SFTY-11)

Para garantizar comunicación inquebrantable en zonas de montaña con alta interferencia:
- **Ráfaga (Burst):** 1 copia de 4 bytes con FEC activo en radios E90-DTU.
- **Ventana Deslizante (Sliding Window):** Procesamiento asíncrono con CRC-8 Maxim (`0x31`).
- **Protección Antirepetida (Replay Protection):** Descarte de duplicados mediante `msgID`.

---

## 6. Integración de Cámaras IA para Demanda Vehicular (AcuSense G2)

> ## 🛑 AVISO DE SEGURIDAD — LÉASE ANTES DE CABLEAR NADA (31/08/2026)
>
> ### ⛔ NO SE CABLEA CÁMARA A `PB9` NI A `PB13`. **SON LOS DOS CANALES DEL MANDO DE RELÉS.**
>
> Hasta el 31/08 este apartado mandaba las cuatro cámaras a `PB9` y `PB13`. **`PB9` es `MANDO_A` y
> `PB13` es `MANDO_B`**, y el mando **se conserva** (decisión del 31/08, `N-104`).
>
> **MEDIDO EN EL FUENTE:**
>
> ```
> 01_Firmware/Maestro/src/botones.cpp:119   if (flanco[0]) mando_registrarPulso(MANDO_A);  // BOTON1 = PB9  = J16 p5
> 01_Firmware/Maestro/src/botones.cpp:120   if (flanco[1]) mando_registrarPulso(MANDO_B);  // BOTON2 = PB13 = J16 p8
> 01_Firmware/Maestro/include/pines.h:92-93 #define BOTON1 PB9   ·   #define BOTON2 PB13
> 01_Firmware/Maestro/src/mando.cpp:38      VENTANA_TRIPLE_MS = 12000     (12 s)
> 01_Firmware/Maestro/src/mando.cpp:225-234 A·A·A -> ACC_AUTOMATICO   ·   B·B·B -> ACC_AMBAR
> 01_Firmware/Esclavo/src/mando.cpp:129-132 case ACC_AMBAR: ambarLocal = true;
> ```
>
> **Lo que pasa si alguien sigue el texto viejo:** una cámara enchufada ahí **entrega pulsos**, y
> **tres pulsos dentro de la ventana de 12 s componen una secuencia del mando** (apartado 7 de este
> mismo manual). En `PB9`, `A·A·A` mete el equipo en **Modo Automático**; en `PB13`, `B·B·B` lo manda
> a **Ámbar** y además arma `ambarLocal`, la bandera de la que cuelgan los tres vetos del Esclavo
> (`grep -n "mando_ambarLocal()" 01_Firmware/Esclavo/src/main.cpp` → `:453`, `:476`, `:617`, medidos el 05/09; decían `:406`, `:416`, `:540` y habían caducado), que **desobedecen las órdenes de radio**.
>
> ### 🔴 Dicho en una línea: **el tráfico cambiaría el modo del semáforo solo**, sin que nadie lo pida y sin que nada lo registre como orden.
>
> **Y encima había un SEGUNDO error que TAPABA al primero.** El texto viejo decía *«y `GND`»*. El
> camino de cámara del firmware es `pinMode(INPUT)` pelado y **activo en ALTO**
> (`01_Firmware/Maestro/src/modo_inteligente.cpp:46` y `:25`), y la bornera saca el pin **junto a
> 3,3 V**. Cableada a masa, **la cámara no dispara nunca**: un ensayo de taller la habría dado por
> buena sin llegar a ver el defecto de arriba, que aparecería el día que alguien *«arreglara»* el
> cableado. **Los dos errores se corrigen juntos o el arreglo es peor que el defecto.**

### 6.1 ~~El cableado que este manual mandaba hacer~~ — ⛔ ANULADO, CONSERVADO COMO RASTRO

~~Para detección inteligente de flujo vehicular en pasos alternados de obra:~~

* ~~**Semáforo Maestro:**~~
  * ~~**Cámara 1 (Aproximación Sentido 1):** Contacto seco `1A`/`1B` en **`PB9`** y `GND` ➔ Demanda Verde Maestro.~~
  * ~~**Cámara 2 (Monitoreo Obra Sentido 1):** Contacto seco `1A`/`1B` en **`PB13`** y `GND` ➔ Confirma flujo interno.~~
* ~~**Semáforo Esclavo:**~~
  * ~~**Cámara 3 (Aproximación Sentido 2):** Contacto seco `1A`/`1B` en **`PB9`** y `GND` ➔ Demanda Verde Esclavo.~~
  * ~~**Cámara 4 (Monitoreo Obra Sentido 2):** Contacto seco `1A`/`1B` en **`PB13`** y `GND` ➔ Confirma flujo interno.~~

**Por qué estaba mal, pin por pin:**

| pin | lo que decía este manual | lo que hay de verdad | nivel |
|---|---|---|---|
| `PB9` | «Cámara 1 / 3, demanda de verde» | **`BOTON1` = `MANDO_A`** (`J16` p5). Tres pulsos en 12 s = **Modo Automático** | ✅ **MEDIDO** (`pines.h:92`, `botones.cpp:119`, `mando.cpp:225-227`) |
| `PB13` | «Cámara 2 / 4, monitoreo de obra» | **`BOTON2` = `MANDO_B`** (`J16` p8). Tres pulsos en 12 s = **Ámbar + `ambarLocal`** | ✅ **MEDIDO** (`pines.h:93`, `botones.cpp:120`, `mando.cpp:230-234`) |
| el `GND` de las cuatro líneas | «contacto seco contra masa» | El pin es **activo en ALTO** y la bornera lo saca junto a **3,3 V**: contra masa **no dispara jamás** | ✅ **MEDIDO** (`modo_inteligente.cpp:25`, `:46`) |

**Además ya no son cuatro cámaras.** Desde el 28/08 **no existen las cámaras 2 y 4**: `PB8` nunca fue
una entrada —es el `LED_TESTIGO`, ver el apartado 6.4— y el conteo de umbral necesitaría un comando de
radio que el protocolo no tiene (`N-59`, `N-64`). El despeje se hace **por tiempo** (`cfgDespejeSeg`),
que es el criterio conservador: la cámara de umbral daría **eficiencia, no seguridad**.

### 6.2 ✅ Dónde va la cámara HOY, y dónde va DESPUÉS — no es el mismo pin

**Son dos situaciones distintas y confundirlas deja un hilo colgando de una entrada viva.**

| | pin | bornera | estado del firmware |
|---|---|---|---|
| **HOY, lo único que un firmware lee** | **`PB0`** (`CAM_DEMANDA_PIN`) | **`J14`** | ✅ **MEDIDO**: se lee de verdad — `modo_inteligente.cpp:98`, `:136` (Maestro) y `main.cpp:350` (Esclavo). La placa ayuda con `R64` 10 kΩ + `C25` 100 nF = antirrebote de 1 ms (`pines.h:43-46`) |
| **TAMBIÉN HOY** | **`PB14`** = `CAM_C_PIN` (`J16` **p10**) y **`PB15`** = `CAM_D_PIN` (`J16` **p12**) | **`J16`** | ✅ **MEDIDO: ya son cámara.** `pinMode(CAM_C_PIN, INPUT)` / `pinMode(CAM_D_PIN, INPUT)` —**pelado, activo en ALTO**— en las dos puntas. **`p10` es la cámara** (una por poste, verificada en banco); **`p12` queda vacío** |

> ⛔ **LA SEGUNDA FILA DECÍA «~~DESPUÉS de la Fase 3~~» Y «~~NINGÚN firmware los lee como cámara;
> hoy siguen siendo botones~~». Falso desde el 31/08** — el *«después»* ya llegó, y el `pinMode` que
> citaba (`INPUT_PULLUP`) **ya no existe en el fichero**. Se tacha con su motivo porque de esa frase
> colgaba el bloqueo de §6.5. **El estado de hoy, con su `grep`, corrido el 05/09:**
>
> ```
> $ grep -rn "pinMode(CAM_._PIN" 01_Firmware/Maestro/src/ 01_Firmware/Esclavo/src/
> 01_Firmware/Maestro/src/botones.cpp:531:  pinMode(CAM_C_PIN, INPUT);
> 01_Firmware/Maestro/src/botones.cpp:532:  pinMode(CAM_D_PIN, INPUT);
> 01_Firmware/Esclavo/src/botones.cpp:523:  pinMode(CAM_C_PIN, INPUT);
> 01_Firmware/Esclavo/src/botones.cpp:524:  pinMode(CAM_D_PIN, INPUT);
> ```

📖 **LEÍDO** (decisión, no medida): las cámaras se mudan a `J16` p10/p12, los pines que libera la
retirada de los pulsadores **C** y **D** — `ESTADO.md:83`, `:105`, `:119` y `roadmap.md` `N-104`. Los
canales **A** (`PB9`, p5) y **B** (`PB13`, p8) **se conservan para el mando** y **no quedan libres**.

> 🟡 **ABIERTO, y no lo cierra este manual:** cuántas cámaras van por poste. `ESTADO.md:50` dice
> *«dos cámaras de demanda, una por poste»*, y el reparto de pines libera **dos** entradas en cada
> nodo. **Lo decide el responsable**; aquí sólo queda escrito que los pines disponibles son p10 y p12.

### 6.3 🔴 La salida de la cámara es configurable (NO / NC) y hay que elegir DESPUÉS de la medida M3

La salida de alarma de la AcuSense se parametriza como **`NO` (Normalmente Abierto)** o **`NC`
(Normalmente Cerrado)** (`05_Funcional/9_Manual_Parametrizacion_Camara_IA.md`, Paso 4). **Las dos
configuraciones están escritas aquí porque cuál es la correcta depende de una medida que todavía no
se ha hecho.**

| si la medida **M3** dice… | cómo se cablea el contacto seco | configuración de la cámara | encaja con el firmware de hoy |
|---|---|---|---|
| **~0,66 V** en reposo → la placa tiene el *pull-**DOWN*** de 10 kΩ del netlist (`R65`–`R68` a `GND`) | entre el pin de señal y el pin de **3,3 V** contiguo (`J16` p9 para p10, p11 para p12) — **NO contra `GND`** | **`NO`**, pulso de **1 s** | ✅ Sí: `pinMode(INPUT)` + `digitalRead(...) == HIGH` (`modo_inteligente.cpp:25`, `:46`) |
| **~3,3 V** en reposo → *pull-**UP*** y el netlist no describe esta placa | entre el pin de señal y **`GND`** (`J16` p2) | **`NO`**, pulso de **1 s** | ❌ No: habría que **invertir la lectura de la cámara** en las dos puntas antes de cablear |
| **otra cosa** | **no se cablea** | — | se anota el número y **se para** |

* **`NC` no se usa en ninguno de los dos casos.** Con `NC` el contacto está cerrado en reposo y se
  abre al detectar: el firmware vería **demanda permanente** mientras no pasa nada y **ausencia de
  demanda** justo cuando pasa un vehículo. Es la inversión exacta que ya costó `N-67`.
* El detalle de M3 —qué se mide, con qué y qué número se espera **antes** de mirar el multímetro—
  está en `05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md`, sección **M3**.

> ✅ **EL TERCER RESULTADO POSIBLE DE M3 — «que no haya resistencia ninguna» — SE MIDIÓ Y NO SE DIO.**
> ~~De `PB14`/`PB15` sólo lo dice el netlist, y eso es un plano, no una tarjeta: con `pinMode(INPUT)`
> pelado y sin resistencia real montada el pin queda flotando y el ruido dispara demandas fantasma.~~
>
> **Medido el 03/09** —multímetro, conector vacío, paso 20 de la Guía de banco—: el pull-**down** de
> **10 kΩ** (`R65`–`R68`, cada una con su 100 nF) **es real y está en las cuatro posiciones**. `p10`
> (`PB14`) y `p12` (`PB15`) dan **0 V en reposo**, y el **paso 21** cabló `p10` contra `p11` en
> normalmente abierto **sin una sola demanda fantasma**. El pin **no flota**.
>
> `PB0` conserva además su propio reposo por hardware (`R64` 10 kΩ + `C25`, `pines.h`). La fuente de
> estos números es
> [`05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md`](../05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md)
> sección **M3**, que es la que manda en cobre medido.

### 6.4 `PB8` no es una entrada: es el LED testigo

**MEDIDO** — `01_Firmware/Maestro/include/pines.h:63` y `01_Firmware/Esclavo/include/pines.h:63`:

```
#define LED_TESTIGO        PB8  // -> R16 1K -> LED D5. NO es entrada de camara
```

Sale por `R16` de 1 kΩ al LED `D5`. **No es bornera y no es entrada optoacoplada.** El firmware lo
deja a propósito en alta impedancia (`modo_inteligente.cpp:50`). Cuatro manuales llegaron a
describirlo como *«umbral de tramo»* (`N-59`, `N-64`); ninguno se había cruzado contra `pines.h`.

### 6.5 ✅ `M3` CERRADA EL 03/09 — lo que SÍ bloquea el cableado hoy

> ⛔ **ESTE APARTADO SE TITULABA *«~~las cuatro cosas, ninguna opcional~~»* Y DOS DE LAS CUATRO YA NO
> EXISTEN.** Se tachan con su motivo. **La fuente que manda aquí es
> [`05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md`](../05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md)
> sección **M3**, no este manual** — en cobre medido gana la spec.
>
> 1. ~~**Falta la medida M3** (polaridad de `J16`): el netlist dice pull-**down** activo en ALTO y el
>    firmware dice `INPUT_PULLUP` activo en BAJO; mientras la contradicción siga abierta no se cablea
>    cámara a `J16`.~~ ✅ **CERRADA el 03/09, y las dos mitades coinciden ahora.** En cobre
>    (multímetro, conector vacío, paso 20 de la Guía): el pull-**down** de **10 kΩ** es **real y está
>    en las cuatro posiciones**, `p10` y `p12` dan **0 V en reposo**, y el paso 21 cabló `p10` en
>    normalmente abierto **sin demandas fantasma**. En el fuente **ya no queda un solo
>    `pinMode(..., INPUT_PULLUP)`**: los botones se leen `INPUT` pelado y **`== HIGH`**.
>    **Activa en ALTO, los cuatro pines y sin excepción.**
>
> 2. ~~**El orden es ASIMÉTRICO:** `PB14` es `botonAceptar()`, el que EJECUTA, leído activo en BAJO,
>    así que lo que se enchufe en `J16` p10 puede pulsar *Aceptar* en un equipo que está en la
>    calle.~~ ✅ **SIN OBJETO: `botonAceptar()` ya no lee ningún pin** —es `return false;` en las dos
>    puntas— y `PB14` es `CAM_C_PIN`. 🔴 **Pero la REGLA de `CLAUDE.md` §9.bis sigue viva tal cual**
>    para cualquier equipo que aún tenga el firmware viejo dentro: **lo que levanta el bloqueo no es
>    el commit, es la CARGA VERIFICADA en la tarjeta.** Un commit no protege de un destornillador.

**Lo que SÍ bloquea, y ninguna es opcional:**

1. 🔴 **`J16` p1 lleva 12 V crudos**, sin opto, sin limitadora y sin clamp. **Se tapa físicamente
   antes de cablear nada.** Y la separación real sobre cobre **no** es la distancia entre pads
   —MEDIDO en `03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md:576-588`—:

   | red de 12 V contra | separación mínima real |
   |---|---|
   | `/Boton1` (p5) | **1,405 mm** |
   | `/Boton2` (p8) | **1,408 mm** |
   | `/Boton3` (**p10**) | **4,269 mm** |
   | `/Boton4` (**p12**) | **1,359 mm** ← el peor de los cuatro |

   👉 **Consecuencia útil: si una de las dos cámaras es más crítica, va en `p10`** (4,27 mm), **no en
   `p12`** (1,36 mm). Un error de una posición al enchufar `J16` mete **12 V en un pin de 3,3 V**.
2. 🔴 **`J16` p5 y p8 —`MANDO_A`/`MANDO_B`— NO son pines libres, aunque su hardware ya no esté.**
   El **mando de relés se retiró físicamente** (`D-1`); **su código se queda**. `botonArriba()` y
   `botonAbajo()` **siguen vivos** —`consumir(0)` / `consumir(1)`, con llamadores— y leen
   `BOTON1`/`BOTON2`, que **son exactamente esos dos pines**. 🔴 **Libre de cobre no es libre de
   firmware: lo que se cierre en `J16` p5 o p8 SIGUE ENTRANDO.** No se cablea cámara ahí.

   ```
   $ grep -n "^bool botonArriba\|^bool botonAbajo" 01_Firmware/Maestro/src/botones.cpp
   617:bool botonArriba()  { return consumir(0); }
   618:bool botonAbajo()   { return consumir(1); }
   ```

> ⛔ **Aquí había un cuarto bloqueo: *«~~Ningún firmware lee `PB14`/`PB15` como cámara todavía~~»*.**
> **Falso desde el 31/08** — ver §6.2. Se tacha con su motivo y no se borra.

### 6.6 Seguridad vial y nivel de prueba de este apartado

* **Invariante que no cambia con nada de lo anterior:** cada cambio de sentido exige obligatoriamente
  el **Despeje Todo-Rojo** completo antes de habilitar el verde opuesto, y el **amarillo normativo de
  4,0 s**. Bajo ninguna circunstancia se omite ninguno de los dos.

| lo que este apartado afirma | nivel |
|---|---|
| `PB9` = `MANDO_A`, `PB13` = `MANDO_B`, y tres pulsos en 12 s componen secuencia | ✅ **MEDIDO EN EL FUENTE** (`pines.h:92-93`, `botones.cpp:119-120`, `mando.cpp:38`, `:225-234`) |
| La cámara es activa en ALTO y no se cablea contra `GND` | ✅ **MEDIDO EN EL FUENTE** (`modo_inteligente.cpp:25`, `:46`) |
| `PB0`/`J14` es hoy el único camino de cámara con firmware | ✅ **MEDIDO** (`modo_inteligente.cpp:98`, `:136`; `Esclavo/src/main.cpp:350`) |
| ~~`PB14`/`PB15` son hoy `botonAceptar()` y `botonCancelar()`~~ → **`PB14` = `CAM_C_PIN` y `PB15` = `CAM_D_PIN`; `botonAceptar()`/`botonCancelar()` son `return false;`** | ✅ **MEDIDO** (`#define CAM_C_PIN`/`CAM_D_PIN` en `pines.h`; `^bool botonAceptar` en `botones.cpp`). ⛔ **La fila anterior fue FALSA con un «MEDIDO» al lado desde el 31/08** |
| `botonArriba()`/`botonAbajo()` **siguen vivos** y leen `BOTON1`/`BOTON2`, **los pines del mando** | ✅ **MEDIDO** (`^bool botonArriba` en `botones.cpp`, con llamadores en `menu.cpp` y `modo_hora.cpp`). 🔴 **Libre de cobre no es libre de firmware** |
| Las distancias de cobre de `J16` contra los 12 V | ✅ **MEDIDO** sobre el `.kicad_pcb` (`MAPEO_TARJETA_KICAD.md:576-588`) |
| ~~Que las cámaras se muden a `J16` p10/p12~~ → **ya mudadas** | ✅ **MEDIDO** (`pinMode(CAM_C_PIN, INPUT)` en las dos puntas). Ya no es *«decidido, sin construir»* |
| ~~Que `R65`–`R68` estén realmente montadas y la polaridad sea la del netlist~~ | ✅ **MEDIDO EN COBRE el 03/09** (**M3 CERRADA**, paso 20): 10 kΩ a masa en las cuatro posiciones, 0 V en reposo, **activa en ALTO**. Fuente: `05_Funcional/17_…` sección **M3** |

> **Nada de este apartado ha pasado prueba de banco**, y **no autoriza a instalar ni a cablear nada**.
> La única forma correcta de verificar el firmware es `01_Firmware/compuerta.py`, y un verde suyo
> **tampoco es un permiso**: dice que los modelos y los arneses de PC no encuentran nada, no que el
> firmware funcione en la tarjeta (`CLAUDE.md` §3).

---

## 7. Vocabulario Oficial del Mando a Distancia de Relés (Anti-Colisión N-53)

Para permitir la operación del semáforo a nivel del suelo sin colisionar con la edición de parámetros en pantalla.

> 🛑 **ESTA TABLA ESTABA MAL EN CUATRO DE SUS CINCO FILAS, y se corrige el 31/08.** No es un detalle
> de redacción: es el vocabulario que decide **qué pulsos componen una orden**, y es justo lo que hace
> peligroso el defecto del apartado 6. Lo de abajo está **MEDIDO** sobre
> `01_Firmware/Maestro/src/mando.cpp` (el Esclavo es idéntico).

### 7.1 ✅ El vocabulario REAL, medido en el fuente

| Secuencia | Ventana | Modo Activado | Confirmación Lumínica | dónde está medido |
|---|---|---|---|---|
| **`A · A · A`** | ≤ **12 s** | 🟢 **Modo Automático** | **2** destellos rojos | `mando.cpp:225-227`, `:45` |
| **`B · B · B`** | ≤ **12 s** | 🟡 **Modo Ámbar (Seguro)** | **3** destellos rojos | `mando.cpp:230-234`, `:46` |
| **`A · B · A · B`** | ≤ **18 s** | 🕒 **Modo Degradado (Reloj)** | **4** destellos rojos | `mando.cpp:204-214`, `:47` |

**Y no hay más.** El repertorio completo son **tres acciones**, no cinco:
`enum AccionMando { ACC_NINGUNA, ACC_AUTOMATICO, ACC_AMBAR, ACC_DEGRADADO };` (`mando.cpp:53`).

### 7.2 ~~La tabla anterior~~ — ⛔ ANULADA, conservada con el motivo de cada fila

| ~~Secuencia~~ | ~~Modo~~ | por qué era falsa |
|---|---|---|
| ~~`A · B · A` (≤12s)~~ | ~~🟢 Modo Automático~~ | ⛔ **Es `A · A · A`.** `A·B·A` no dispara nada |
| ~~`B · A · B` (≤12s)~~ | ~~🟡 Modo Ámbar~~ | ⛔ **Es `B · B · B`.** `B·A·B` no dispara nada |
| ~~`B · A · B · A` (≤18s)~~ | ~~✋ Modo Manual, 5 destellos~~ | ⛔ **NO EXISTE.** No hay `ACC_MANUAL` en el `enum` |
| `A · B · A · B` (≤18s) | 🕒 Modo Degradado, 4 destellos | ✅ **la única fila que era correcta** |
| ~~`A · A · B · B` (≤18s)~~ | ~~📷 Modo Inteligente, 6 destellos~~ | ⛔ **NO EXISTE.** No hay `ACC_INTELIGENTE` en el `enum` |

### 7.3 Lo que el mando comprueba además de la secuencia

* **La red de seguridad real del Degradado no es la secuencia, es la validación.** Aunque alguien
  acierte `A·B·A·B` por casualidad, el firmware **no entra** si la hora no está validada:
  `modo_degradado_evaluarEntrada() == MDG_OK` (`mando.cpp:213`). El mando permite reactivar en campo
  sin grúas, **pero no saltarse la puesta a punto**.
* **El Ámbar entra sin condiciones y desde cualquier modo en marcha** (`mando.cpp:230-234`): es la
  regla que impide que nadie quede atrapado con un semáforo en estado raro a 5 m de altura. Además
  arma **`ambarLocal`**, la bandera de la que cuelgan los tres vetos del Esclavo
  (los tres `if (!mando_ambarLocal() && ...)` de `Esclavo/src/main.cpp` — `:453`, `:476`, `:617`, medidos el 05/09): mientras un operario dejó ámbar local puesto, **una
  orden de radio no lo saca de ahí**. Es una desobediencia deliberada, no un fallo.
* **Los destellos son SIEMPRE ROJOS** y contables desde el suelo (`mando.cpp:41-44`): el rojo nunca
  significa *«pase»*, así que si el operario cuenta mal, **el peor caso sigue siendo seguro**.
* **Inhibición de UI (N-53):** mientras el operador esté en pantallas de configuración
  (`AJUSTAR HORA`, `CONFIG_TIEMPOS`), el receptor del mando **se inhibe al 100%**
  (`mando.cpp:89`, `:180`), permitiendo ajustar números con el codillo sin disparar cambios de modo
  involuntarios.
* **Sólo los botones 1 y 2 alimentan el mando.** El 3 **ejecuta** y el 4 sale: si formaran parte de
  alguna secuencia, repetirlos a ciegas podría arrancar un modo que nadie pidió (`botones.cpp:115-120`).

### 7.4 🟢 El mando SE CONSERVA — y por eso el apartado 6 importa

**DECIDIDO el 31/08** (`roadmap.md` `N-104`): se conservan los canales **`A`** (`PB9`, `J16` p5) y
**`B`** (`PB13`, `J16` p8); se retiran **`C`** (`PB14`, p10) y **`D`** (`PB15`, p12), **y esos dos
pines pasan a las cámaras**.

* **Por qué los dos canales y no sólo `A`:** `B·B·B` es **el único sitio donde se arma `ambarLocal`**
  (`Esclavo/src/mando.cpp:129-132`, **MEDIDO**). Sin el canal `B` esa bandera no se armaría jamás, los
  tres `if` que la niegan se volverían siempre-verdaderos y **el veto desaparecería** — una regla de
  seguridad perdida por sustracción. Y `A`-solo tampoco compraba nada: liberaría **tres** entradas
  cuando sólo hacen falta **dos**.
* 🟠 **El receptor físico del mando nunca se compró.** Lo que se conserva hoy es el **firmware y el
  veto**; para tener mando físico hay que comprarlo.
* 📄 **Coherente con `ESTADO.md`, verificado el 31/08:** `ESTADO.md:83-84` ya recoge esta decisión —se
  retiran **sólo** los pulsadores 3 y 4, y *«EL MANDO DE RELÉS SE CONSERVA, en los canales A y B»*—.
  La redacción anterior del **28/08**, que retiraba los cuatro pulsadores y el mando entero, queda
  **tachada allí con su motivo**. Si alguien encuentra todavía la versión vieja en otro documento, la
  vigente es ésta: `roadmap.md` `N-104`.

---

## 8. Módulo Bluetooth para Telemetría y Diagnóstico Móvil (Estándar Baliza)

Para soporte técnico en campo sin escaleras:
* **Conexión Hardware:** Puerto **USART1 REMAPEADO a `PB6` (TX) / `PB7` (RX)**, conector **`J17`**, alimentado con 5V/3.3V de la PCB. ✅ **MEDIDO:** `static HardwareSerial SerialBT(PB7, PB6);` en `01_Firmware/Maestro/src/bluetooth.cpp:28`.
  > ⛔ Este manual publicó ~~«USART1 (`PA9` TX, `PA10` RX)»~~ hasta el 31/08/2026. Es el sitio donde
  > estuvo **antes** de `N-76`, y dejarlo escrito manda al técnico a soldar el módulo Bluetooth al
  > conector equivocado.
* **Telemetría en Vivo:** Emisión periódica de `$STATUS,...` cada 1 segundo con modo, fase de luces, cuenta regresiva, % de señal RF y hora exacta del RTC.
* **Caja Negra de Alarmas:** Registro inmediato de eventos con timestamp (`$ALARM,NODE:MAESTRO,EVENTO:FALLO_RF,CAUSA:SILENCIO_25000ms,ACCION:CAMBIO_A_AMBAR,HORA:...`) para diagnosticar la causa exacta de cualquier caída de radio en obra.
  > ⛔ El ejemplo decía ~~`$ALARM,EVENTO:FALLO_RF_12S...`~~ hasta el 31/08/2026. El propio firmware ya
  > había retirado ese literal con su motivo escrito al lado —`Esclavo/src/main.cpp:573` y
  > `*/include/bluetooth.h:18-19`: *«el numero quedo mintiendo al subir el umbral a 25 s»*—, pero el
  > manual se quedó con la versión vieja. La causa **no lleva el número pegado al nombre del evento**:
  > se compone en tiempo de ejecución desde `SFTY6_SILENCIO_MS`, y por eso no puede envejecer.
