# MANUAL DEL PUENTE REPETIDOR ESP32 (Topología de 4 Radios — V8.0)

> # 🛑 EN ESTE PROYECTO HAY **DOS** ESP32 DISTINTOS. ESTE MANUAL ES DE UNO SOLO.
>
> **Léase antes de tocar un cable o teclear un `pio run`.** Los dos son módulos ESP32, los dos van
> en un poste, los dos hacen de «puente» — y **usan los mismos GPIO para cosas distintas**. Cargar
> el firmware de uno en el otro no da error: da un equipo que parece encendido y no hace su trabajo.
>
> | | **ESP32 de EXPANSIÓN** | **ESP32 del REPETIDOR** *(el de este manual)* |
> |---|---|---|
> | **Dónde va** | **uno por poste**, en cada equipo | **un poste intermedio propio**, entre Maestro y Esclavo |
> | **De qué cuelga** | del conector **`J17`** de la tarjeta del semáforo | de **dos radios** E90-DTU (B1 y B2), por RS-485 |
> | **Qué hace** | **puente Bluetooth SPP ⟷ `J17`** (sustituye al módulo SPP) **+ reloj `DS3231`** por I²C. 🔵 **Y desde `N-145` (05/09), UNA cosa más y sólo una: SELLA el hueco `HORA:--:--:--` de las tramas del equipo con la hora de su `DS3231`, y recalcula el checksum** — ver el recuadro de abajo | enlaza **dos radios back-to-back** para salvar una curva ciega o una montaña |
> | **Cuántas radios toca** | **ninguna** | **cuatro** en total en el sistema |
> | **Firmware** | 🟢 `01_Firmware/ESP32_Expansion/` | 🟠 `01_Firmware/Repetidor/` |
> | **Cómo se llama en Bluetooth** | 🔵 **se auto-rotula `SEM-<serie>-M` o `SEM-<serie>-E`** *(y `SEM-SIN-MATRICULA` mientras no lo ha aprendido)* | **no ofrece Bluetooth SPP.** Si busca un `SEM-…` y no sale, puede que tenga delante el otro |
> | **Estado hoy** | firmware escrito y compilando; **sin pasar banco** | **NO DESPLEGADO.** Fuera de la configuración vigente |
> | **Documento** | `05_Funcional/18_Especificacion_Firmware_ESP32.md` | **éste** |
>
> ### 🔵 05/09 (`N-145`) — EL DE EXPANSIÓN DEJÓ DE SER ESTRICTAMENTE «VERBATIM», Y SE DICE CUÁNTO
>
> **Hasta el 05/09 el puente de expansión no cambiaba ni un byte de lo que pasaba.** Ahora cambia
> **una cosa y sólo una**, y conviene tenerlo escrito aquí porque este manual es donde se comparan
> las dos placas:
>
> | | |
> |---|---|
> | **Qué toca** | el literal **`HORA:--:--:--`** de una trama del equipo, y nada más. Si el STM32 puso una hora, **no encuentra el hueco y no hace nada** |
> | **Por qué recalcula el checksum** | la app **sí** valida el XOR-8 en la bajada. Sellar sin recalcular no daría una hora mala: **daría el tablero congelado**, con el síntoma *«el puente se comió la telemetría»* mandando a mirar el cable |
> | **Qué NO hace** | no parte, no une, no filtra, no reordena, **no añade ni quita un byte** y **no origina** ninguna trama. Y **nunca inventa la hora**: si el `DS3231` no es fiable, el hueco sale como está |
>
> 🔴 **Esto NO afecta al ESP32 del REPETIDOR, que es el de este manual: el repetidor sigue validando
> FORMATO y no tocando NADA.** El sello vive en `ESP32_Expansion/src/puente.cpp`, en el otro
> firmware. 🛑 **Y sin un `DS3231` conectado —no está comprado, línea `A6`— la hora seguirá saliendo
> en blanco: eso es el arreglo callándose bien, no fallando.**

> ### 🔵 04/09 — LA FORMA MÁS BARATA DE SABER QUÉ PLACA TIENE DELANTE, ANTES DE CARGAR NADA
>
> **Desde el 04/09 el ESP32 de EXPANSIÓN dice de qué poste cuelga en su propio nombre Bluetooth.**
> No es una opción de compilación —el mismo binario sirve a las dos puntas—: **lo aprende del campo
> `NODE:` de la trama `$STATUS`** que le manda la tarjeta del semáforo. **MEDIDO en
> `01_Firmware/ESP32_Expansion/src/transporte_app.cpp:80-114`** y `include/contrato.h:258-259`.
>
> | en la lista de Bluetooth del teléfono | qué placa es |
> |---|---|
> | **`SEM-<serie>-M`** | ESP32 de **expansión**, colgado del poste **MAESTRO** |
> | **`SEM-<serie>-E`** | ESP32 de **expansión**, colgado del poste **ESCLAVO** |
> | **`SEM-SIN-MATRICULA`** | ESP32 de **expansión** que **aún no ha aprendido** de qué poste cuelga |
> | *(nada)* | **el REPETIDOR no ofrece SPP**: este manual no habla de Bluetooth con el teléfono |
>
> 🛑 **PERO ESE NOMBRE NO ES FIABLE EL PRIMER DÍA, Y HAY QUE SABERLO ANTES DE SUBIR A UN POSTE.** El
> rótulo aprendido **se guarda para la SIGUIENTE arrancada** *(`transporte_app.cpp:109-113`)*, a
> propósito: renombrar el perfil SPP en caliente obliga a cerrarlo y reabrirlo, o sea a **tirar la
> sesión del operario que puede estar dando una orden al cruce**. **Consecuencia: un módulo recién
> puesto se anuncia `SEM-SIN-MATRICULA`, y con dos módulos nuevos en el mismo frente de obra LOS DOS
> POSTES SE LLAMAN IGUAL hasta que se les da una vuelta de energía.**
>
> **Un nombre no sustituye a mirar el cableado**: el `SEM-…` dice **de qué poste** cuelga la placa,
> **no qué firmware lleva dentro**. Los tres GPIO que colisionan siguen colisionando igual. Para eso
> está la tabla de abajo.
>
> ### 🔴 Los tres GPIO que colisionan — MEDIDO el 31/08
>
> No es un parecido: es **el mismo número de pin sirviendo a dos cosas incompatibles**.
>
> | GPIO | en el **REPETIDOR** *(este manual, §2)* | en el de **EXPANSIÓN** |
> |---|---|---|
> | **`GPIO16`** | RX de la radio B1, **a través de un MAX3485** | **RX del enlace TTL directo a `J17` p2/p3** — sin transceptor |
> | **`GPIO17`** | TX de la radio B1, **a través de un MAX3485** | **TX del mismo enlace TTL** |
> | **`GPIO22`** | **`DE/RE` de la radio B2** (control RS-485) | **`SCL` del bus I²C del `DS3231`** |
>
> **MEDIDO en:** `01_Firmware/Repetidor/include/pines_repetidor.h` (`M1_RX 16`, `M1_TX 17`,
> `M2_DE_RE 22`) contra `01_Firmware/ESP32_Expansion/include/contrato.h`
> (`ENLACE_PIN_RX 16`, `ENLACE_PIN_TX 17`, `DS3231_SCL 22`, `DS3231_SDA 21`).
>
> ### 🛑 Qué pasa si se confunden — y por qué el papel es lo único que lo impide
>
> - **Firmware del Repetidor en una placa de EXPANSIÓN:** el equipo pierde **la única superficie de
>   mando que le queda**. Con la pantalla, los cuatro pulsadores y el mando de relés retirados, toda
>   la operación pasa por la app, y la app pasa por ese ESP32. El semáforo sigue ciclando —el
>   STM32 no depende del accesorio— pero **queda seguro y no operable**, y encima el firmware
>   estaría meneando `GPIO22` como línea de control RS-485 **sobre el `SCL` de un reloj**.
> - **Firmware de Expansión en la placa del REPETIDOR:** ningún `DE/RE` se gobierna, el bus RS-485
>   se queda sin árbitro y el enlace de radio no pasa.
>
> **Los dos firmwares NO son intercambiables**, no comparten pinout y **no se mezclan sus
> directorios**: comparten familia de módulo y nada más. Antes de un `pio run -t upload`, mire de
> qué poste es la placa que tiene delante.
>
> ### ⚠️ Y sobre este manual en concreto
>
> La topología de 4 radios que describe está **fuera de la configuración vigente**: hoy son **2
> radios en enlace directo, sin repetidor** (`CLAUDE.md` §10). Lo que sigue **no es un montaje a
> ejecutar**; es la referencia del día que haya que salvar un obstáculo. Nada de este documento
> aplica al ESP32 de expansión.

Este documento detalla la configuración y cableado del puente repetidor con microcontrolador **ESP32** cuando se requiere salvar curvas ciegas o montañas mediante 4 radios industriales E90-DTU.

El ESP32 enlaza **dos radios back-to-back** (B1 y B2) dentro de una topología total de 4 radios.

---

> ## ⚠️ REQUISITO CRÍTICO — LAS 4 RADIOS A `2.4 kbps`
>
> El modo repetidor **no funcionará** con las radios a `0.3 kbps`. En esta topología cada mensaje
> atraviesa **dos saltos de aire por sentido**: a 0.3 kbps el viaje orden→confirmación superaba los
> **3 segundos** y desbordaba el plazo de espera del Maestro, que reintentaba y colisionaba con la
> confirmación entrante en el bus half-duplex.
>
> **Ése era el motivo exacto de que el repetidor no enlazara en las pruebas del 30 y 31 de julio.**
> No era un problema de cableado ni del firmware del ESP32.
>
> Ver `4_Manual_Configuracion_Radios.md`. Las 4 radios deben quedar con el mismo Air Data Rate.

---

## 1. Topología del Sistema Repetidor

```text
+----------------+        Air 170MHz       +-------------------------------+        Air 172MHz       +----------------+
| Semáforo STM32 | <=====================> | Radio B1 <-> ESP32 <-> Radio B2| <=====================> | Semáforo STM32 |
|   (Maestro)    |                         |      (Poste Repetidor)        |                         |   (Esclavo)    |
+----------------+                         +-------------------------------+                         +----------------+
```

1. **Poste Maestro:** STM32 Maestro + Radio 1 en Canal `0` (`170.0 MHz`).
2. **Poste Repetidor Central:**
   - Radio B1 en Canal `0` (`170.0 MHz`) -> Habla con el Maestro.
   - Microcontrolador ESP32 con firmware `01_Firmware/Repetidor`.
   - Radio B2 en Canal `10` (`172.0 MHz`) -> Habla con el Esclavo.
3. **Poste Esclavo:** STM32 Esclavo + Radio 4 en Canal `10` (`172.0 MHz`).

---

## 2. Pinout del ESP32 Repetidor (`01_Firmware/Repetidor/include/pines_repetidor.h`)

> 🔴 **Esta tabla es SOLO del ESP32 del repetidor. El ESP32 de expansión usa tres de estos pines
> para otra cosa** — ver el recuadro del principio. No se cablea una placa con esta tabla sin haber
> confirmado antes cuál de los dos equipos se está montando.

| Conexión | Pin del ESP32 **del repetidor** | Puerto | ⚠️ colisión con el de expansión |
|---|---|---|---|
| **Radio B1 (Entrada) RX** | Pin `16` | UART1 RX | 🔴 allí es **RX del enlace TTL a `J17`** |
| **Radio B1 (Entrada) TX** | Pin `17` | UART1 TX | 🔴 allí es **TX del enlace TTL a `J17`** |
| **Radio B1 DE/RE** | Pin `4` | Control RS485 | — |
| **Radio B2 (Salida) RX** | Pin `32` | UART2 RX | — |
| **Radio B2 (Salida) TX** | Pin `33` | UART2 TX | — |
| **Radio B2 DE/RE** | Pin `22` | Control RS485 | 🔴 allí es **`SCL` del I²C del `DS3231`** |

*Nota:* Las radios E90-DTU en bornera RS485 conmutan automáticamente TX/RX por hardware interno.

> **La diferencia que no se ve en la tabla y decide el cableado:** aquí `GPIO16`/`GPIO17` van a las
> radios **a través de transceptores MAX3485** (§3.0). En el ESP32 de expansión los mismos dos pines
> van **en TTL directo** al conector `J17` del semáforo, **sin transceptor de por medio**. Mismo
> número de pin, dos niveles eléctricos distintos y dos destinos distintos.

---

## 3. Firmware del Repetidor (V8.3) — valida antes de retransmitir

> **Cambio importante respecto a versiones anteriores.** El puente ya **no reenvía a ciegas**
> cualquier byte que le llegue: reconoce el formato del protocolo (4 bytes con CRC-8) y **solo
> retransmite tramas válidas**.
>
> **Por qué:** si el par RS485 de entrada queda flotando —falta de resistencias de polarización,
> transceptor sin alimentar, cable partido— el receptor lee ruido continuo. El puente anterior lo
> interpretaba como datos infinitos, dejaba la transmisión permanentemente activa y **la radio de
> salida se quedaba radiando basura al aire**, bloqueando el canal. Ése fue el fallo del 31/07:
> LED TX fijo en B2.
>
> Ahora ese ruido se descarta dentro del ESP32 y **nunca llega al aire**. El puente es inmune a
> esta familia de fallos de cableado.

### 📡 El puente valida FORMATO, no comandos — y por eso no hay que tocarlo

Desde la V8.7 el protocolo lleva comandos nuevos (`0x07`–`0x0F`) para la **sincronización horaria** y
la **configuración del ciclo**. **El repetidor no necesita ningún cambio para dejarlos pasar.**

La razón está en cómo valida: comprueba que la trama tenga **4 bytes y CRC-8 Maxim correcto**, y
**no mira qué comando lleva dentro**. Un puente que conociera la lista de comandos habría que
recompilarlo y reflashearlo cada vez que el protocolo crece — y **el día que alguien olvidara hacerlo,
la sincronización horaria se caería en silencio solo en la topología de 4 radios**, que es la más
difícil de diagnosticar.

> **Validar la forma y no el contenido es lo que mantiene al puente fuera del camino crítico.** Es
> deliberado, no un descuido: no hay ninguna versión del repetidor que "soporte" o "no soporte" la
> sincronización horaria.

### ⚠️ El repetidor es irrelevante en Modo Degradado

Conviene decirlo para que nadie lo busque como causa: en **Modo Degradado no hay radio en absoluto**
—ése es justamente el motivo de entrar al modo— así que **el puente no interviene**. Cada unidad
calcula su fase por su cuenta a partir de la hora.

Un repetidor colgado, sin alimentar o mal configurado **no afecta al Modo Degradado**, ni para bien ni
para mal. Lo que sí afecta es la **sincronización previa**: si el enlace nunca funcionó a través del
puente, nunca hubo sincronización, y **el Degradado se rechaza**.

---

## 3.0 RS-485 es half-duplex: por qué el control DE/RE lo es todo

Conviene entender esto, porque explica los dos síntomas que costaron una tarde de campo.

### El bus solo puede tener un emisor a la vez

RS-485 usa **un solo par de hilos para ambos sentidos**. No hay canal de ida y otro de vuelta:
cuando un extremo está excitando la línea, **el otro no puede hablar**. Es como un radioteléfono:
o se transmite, o se escucha, nunca las dos cosas.

Quién habla en cada momento lo decide una señal de control, el **`DE/RE`** del transceptor
MAX3485:

| `DE/RE` | Estado | Qué ocurre |
|---|---|---|
| **ALTO** | Transmisión | El ESP32 excita la línea. **La radio no puede enviarle nada.** |
| **BAJO** | Recepción | El ESP32 escucha. La radio puede hablarle. |

En el puente hay **dos buses independientes**, cada uno con su control: `M1_DE_RE` (GPIO 4) hacia
B1, y `M2_DE_RE` (GPIO 22) hacia B2.

### El fallo del 31/07, explicado

El firmware anterior levantaba `DE/RE` en cuanto aparecía **cualquier** byte y solo lo bajaba tras
5 ms de silencio. Con el par de entrada metiendo ruido continuo, ese silencio **nunca llegaba**:

```
M2_DE_RE  ─────────────────────────────────────────────  siempre ALTO
                    (el ESP32 ocupa el bus permanentemente)
```

De ahí salieron **los dos síntomas a la vez**, que parecían problemas distintos:

1. **LED TX de B2 encendido fijo.** B2 recibía un flujo interminable y lo radiaba sin parar,
   saturando el canal.
2. **Sin datos de vuelta.** Y éste es el menos evidente: como el ESP32 tenía el bus ocupado el
   100% del tiempo, **B2 no podía devolverle nada**. La respuesta del Esclavo llegaba a B2 y ahí
   moría, contra un bus permanentemente tomado. *No es que el Esclavo callara: es que su respuesta
   no tenía por dónde entrar.*

### Cómo lo maneja el firmware V8.3

`DE/RE` se levanta **únicamente durante los milisegundos que dura enviar una trama válida**, y se
baja enseguida:

```
M2_DE_RE  ▁▁▁▁▁▁▁█▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁█▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
                 ↑ 4 ms          ↑ 4 ms
          el bus queda libre el 99,9% del tiempo
```

Una trama de 4 bytes a 9600 baudios ocupa ~4,2 ms, y solo hay una cada 3 segundos. El camino de
vuelta queda despejado prácticamente siempre.

### Qué mirar en campo

| Síntoma | Qué significa |
|---|---|
| LED TX de una radio **encendido fijo** | Su `DE/RE` está permanentemente activo, o le entra ruido continuo. **El bus de ese lado está bloqueado en ambos sentidos.** |
| LED TX **destella brevemente cada 3 s** | Correcto |
| Llega en un sentido pero **no vuelve nada** | Sospechar del `DE/RE` de ese tramo antes que de la radio del otro extremo |

> ⚠️ **No hace falta cablear `DE/RE` hacia las radios E90-DTU**: ellas conmutan solas por hardware.
> Las señales `M1_DE_RE` y `M2_DE_RE` controlan los **transceptores MAX3485 de la placa del
> repetidor**, no las radios.

---

## 3.1 Detalle técnico

El firmware en `01_Firmware/Repetidor/src/main.cpp` procesa cada sentido de forma independiente:

1. Lee los bytes que le entrega la radio de entrada y los va acumulando en un búfer de 4 bytes.
2. Al completar 4 bytes, comprueba el **CRC-8 Maxim (`0x31`)**.
   - **CRC correcto** → levanta el `DE/RE` de la radio opuesta, emite la trama, espera a que salga
     el último bit y **baja el `DE/RE` de inmediato**.
   - **CRC incorrecto** → descarta un byte, desplaza el búfer y reintenta el enganche (ventana
     deslizante). Así recupera la trama aunque venga precedida de ruido.

**La línea de transmisión se ocupa únicamente durante la trama** (~4,2 ms a 9600 baudios), no
durante ventanas de silencio. Es lo que mantiene libre el camino de vuelta.

### Informe por consola — siempre activo

El firmware **de producción** imprime por USB (115200) un encabezado al arrancar y un informe de
contadores cada 2 segundos.

> **Esto no era así antes, y costó una tarde de campo.** El informe solo existía en el entorno
> `repetidor_diag`, pero `pio run -t upload` sube el entorno **por defecto**, que era el de
> producción y era mudo. La consola mostraba solo el arranque de la ROM del ESP32 y se concluía
> "no hay datos de flujo", cuando en realidad nunca se había cargado el firmware que informa.

**Primera comprobación en campo:** si tras el arranque de la ROM **no aparece el encabezado**, el
firmware no está corriendo.

```
==================================================
 REPETIDOR ESP32  V8.3  -  puente con validacion CRC
==================================================
[    12s]  A<-Maestro:     36 b /    9 val /     0 desc   |   C<-Esclavo:     36 b /    9 val /     0 desc
```

**El ESP32 del repetidor no lleva watchdog.** Si se cuelga, Maestro y Esclavo lo detectan por su
cuenta y pasan a Ámbar intermitente a los 25 s (fail-safe, SFTY-6), pero el repetidor requiere corte
de energía para recuperarse.

> ⚠️ **Esta frase es del ESP32 DEL REPETIDOR, y no se puede trasladar al otro.** El ESP32 de
> expansión **sí** lleva watchdog (`01_Firmware/ESP32_Expansion/src/vigilante.cpp`, sobre
> `esp_task_wdt_*`), y además **su caída no la detecta nadie en el equipo**: SFTY-6 vigila el enlace
> de radio, no el conector `J17`, así que un ESP32 de expansión colgado **no dispara nada** y el
> STM32 sigue ciclando sin enterarse. Son dos situaciones opuestas —aquí el sistema **sí** reacciona,
> allí **no**— y confundirlas manda a buscar la causa al poste equivocado. Ver
> `18_Especificacion_Firmware_ESP32.md` §4.2 y `AB-1`.

---

## 3.2 ⛔ NO unir las dos radios del repetidor con un cable directo

Puede parecer que dos radios conectadas entre sí bornera con bornera (`485_A`↔`485_A`,
`485_B`↔`485_B`) harían de puente solas, sin ESP32. **No funciona, y conviene entender por qué.**

RS-485 usa **un solo par de hilos para ambos sentidos**, así que en cada instante solo puede hablar
uno. Las radios E90-DTU conmutan la dirección **automáticamente, cada una por su cuenta**: deciden
tomar la línea cuando tienen algo que enviar.

**El problema es que ninguna sabe de la existencia de la otra.** No hay árbitro, no hay turnos. En la
práctica una de las dos acapara la línea y la otra queda muda — con lo que el enlace funciona en un
sentido y no en el contrario.

> Probado en campo el 31/07/2026: con el cable directo, la ida fluía y el retorno nunca llegaba.

**El ESP32 existe precisamente para arbitrar**: levanta la línea hacia la radio de salida solo
durante los milisegundos que dura una trama válida, y la suelta enseguida para que la otra pueda
contestar. Esa es su función, y no es opcional.

---

## 3.3 🔍 Cómo saber si el transmisor de una radio está muerto

Un receptor capta señal aunque su antena esté floja. **Un transmisor con la antena en mal estado no
radia prácticamente nada**: la potencia se refleja y se queda dentro. El síntoma es engañoso —
*el LED indica que transmite, pero nadie la oye*.

### La prueba de intercambio de frecuencias

Es la que resolvió el caso del 31/07 y no requiere instrumentos:

1. Anota qué radio está en cada banda y qué falla.
2. **Intercambia las asignaciones de frecuencia** entre los dos extremos del repetidor.
3. Vuelve a observar.

| Lo que se observa | Conclusión |
|---|---|
| El fallo **se queda con el mismo destino** | Problema del enlace hacia ese punto |
| El fallo **sigue a la misma radio**, aunque cambie de banda y de destino | **El transmisor de esa radio está muerto** |

**Caso real:** B1 recibía bien en las dos configuraciones, pero **nadie oía sus transmisiones** — ni el
Maestro ni el Esclavo, en bandas distintas. B2, en cambio, sí logró transmitir. **Factor común: B1.**

### Qué revisar antes de dar el radio por perdido

1. **Conector SMA** — apretado, sin el pin central hundido, sin rosca cruzada
2. **Cable coaxial** — continuidad, sin dobleces cerrados ni conector flojo
3. **Antena de la banda correcta** — una de otra banda presenta una carga pésima
4. **Intercambiar el radio sospechoso por uno sano.** Si el fallo se muda, confirmado

> ⚠️ **Transmitir con la antena en mal estado daña el amplificador de potencia.** Con 1 W, unas horas
> radiando contra un conector suelto o un coaxial roto bastan para inutilizar el radio.
>
> **Al montar un radio de reemplazo, revisa primero el coaxial y el conector.** Si vuelves a montarlo
> sobre el cable que causó la avería, quemas también el nuevo.

---

## 4. Diagnóstico rápido si el repetidor no enlaza

### Paso 1 — Seguir la cadena de LEDs (no requiere herramientas)

**El Maestro transmite cada 3 segundos aunque el enlace esté caído**: estando en fallo sigue emitiendo
una orden de Rojo con esa cadencia. Ese pulso periódico es el metrónomo para rastrear la cadena.

En cada radio, **ignore el LED de encendido (PWR), que queda fijo**, y observe los LEDs de actividad
(`TXD` / `RXD`), que destellan al pasar información. **No hace falta saber cuál es cuál**: lo único
que se registra es si esa radio destella cada 3 segundos o no.

Observe cada radio durante ~15 segundos (unos 5 pulsos). Como están separadas, lo práctico es grabar
un video corto con el celular frente a cada una y comparar después.

Anote **hasta dónde llega el parpadeo**:

```text
  MAESTRO  ~~RF~~>  B1  ──RS485──> [ESP32] ──RS485──>  B2  ~~RF~~>  ESCLAVO
     [ ]              [ ]                                [ ]           [ ]
```

| Último punto con parpadeo | Dónde está el corte | Qué revisar |
|---|---|---|
| Nada, ni el Maestro | Antes de la radio | Cableado `A`/`B` a la tarjeta; alimentación de la radio |
| Solo el Maestro | Aire Maestro → B1 | Ambas en canal `0` y misma velocidad aérea |
| Hasta B1 | **Dentro del repetidor** | ESP32 alimentado, flasheado, y con MAX3485 en la placa → Paso 2 |
| Hasta B2 | Aire B2 → Esclavo | **Causa más frecuente:** la radio del Esclavo quedó en canal `0`; en modo repetidor debe estar en **canal `10`** |
| Toda la cadena | Enlace físico correcto | Problema de protocolo; reportar a desarrollo |

> ⚠️ **En modo repetidor el Maestro y el Esclavo NO van en la misma frecuencia.** Maestro y B1 en
> canal `0` (170.0 MHz); B2 y Esclavo en canal `10` (172.0 MHz). Confundir esto es el error más común
> al pasar de 2 radios a 4: se cambia B1/B2 y se olvida la radio del Esclavo.

### Paso 2 — Firmware de diagnóstico del ESP32

Si el corte parece estar dentro del repetidor, hay un segundo entorno de compilación que informa por
USB cuántos bytes llegan de cada lado, cada 2 segundos:

> 🛑 **ANTES DE TECLEAR ESTO: confirme que el ESP32 que tiene enchufado es el del POSTE REPETIDOR.**
>
> Estos tres comandos se lanzan desde `01_Firmware/Repetidor/` y cargan el firmware **del
> repetidor**. Si el módulo conectado es un **ESP32 de expansión** —el que cuelga de `J17` en un
> poste de semáforo—, esta carga **le deja el equipo sin la única interfaz que le queda**: sin
> pantalla, sin pulsadores y sin mando, toda la operación pasa por la app, y la app pasa por ese
> ESP32. El semáforo seguiría ciclando, pero **nadie podría mandarle nada**.
>
> El síntoma es engañoso: el módulo enciende, el LED de alimentación queda fijo y por consola
> aparece el encabezado del repetidor. **Parece que funciona.**
>
> **Cómo se distingue en treinta segundos, sin abrir un fichero:** un ESP32 **de expansión** tiene
> un cable de datos hacia el **conector `J17` de la tarjeta del semáforo** y, si el reloj está
> montado, un módulo `DS3231` (`ZS-042`) colgado de `GPIO21`/`GPIO22`. Un ESP32 **del repetidor**
> tiene **dos radios** y sus transceptores MAX3485, y **ningún** cable hacia una tarjeta de
> semáforo.
>
> **Firmware de expansión** — otro directorio, otro entorno, y **no es intercambiable**:
> ```bash
> pio run -e esp32_expansion -t upload    # desde 01_Firmware/ESP32_Expansion/
> ```

```bash
pio run -e repetidor_diag -t upload     # cargar el diagnóstico
pio device monitor -b 115200            # ver el informe

pio run -e repetidor -t upload          # volver a producción al terminar
```

El informe indica directamente si el tráfico del Maestro llega al puente y si el Esclavo responde,
lo que separa un fallo de radio de un fallo del puente.

### Otros síntomas

| Síntoma | Causa probable | Acción |
|---|---|---|
| Enlaza pero cae al pasar el Esclavo a Verde | Alguna radio sin reconfigurar a `2.4 kbps` | Verificar las 4 una por una |
| Parpadeo errático ("árbol de navidad") | Tramas fragmentadas | Confirmar `2.4 kbps` y canales `0` / `10` |
| Ambos extremos en Ámbar permanente | ESP32 sin alimentación o colgado | Verificar 5 V del ESP32; cortar y restablecer energía |
| UART de B1/B2 distinto de 9600 | El ESP32 les habla a 9600 fijo | Revisar `Baud Rate` (no confundir con Air Data Rate) |
| Una radio **recibe pero nadie la oye** | Transmisor o antena en mal estado | Ver §3.3 — prueba de intercambio de frecuencias |
| Una radio **oye pero no contesta** | **DIP switches `M0`/`M1` fuera de OFF/OFF** | Ver `4_Manual_Configuracion_Radios.md`. Es la causa más frecuente |
