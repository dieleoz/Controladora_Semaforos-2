# TROUBLESHOOTING - Controladora de Semáforos V4

Esta bitácora documenta las fallas esperadas y reportadas en campo, su diagnóstico y solución. Dado que la tarjeta (STM32 + MOSFETs) ya está impresa, las soluciones se enfocan en software o modificaciones externas al hardware.

## 1. Problemas de Comunicación (Radios LoRa)

### Síntoma: Ambos semáforos se quedan en Ámbar Intermitente (S_FALLO)
**Diagnóstico:** El maestro no está recibiendo los `PONG` del esclavo o viceversa durante más de
~~5 segundos~~ **25 segundos** — el umbral de orfandad de SFTY-6, releído del C++:

```
$ grep -n "SFTY6_SILENCIO_MS" 01_Firmware/Maestro/include/protocolo.h
149:#define SFTY6_SILENCIO_MS   25000UL
```

**Posibles Causas:**
1. **Ruido en Repetidor (ESP32):** El repetidor se congeló por leer basura electromagnética (ver GAP OPT-5). *Solución temporal:* Reiniciar el repetidor. *Solución Definitiva:* Actualizar firmware del ESP32 para limpiar buffer por timeout. *(Cerrado desde V8.3 con SFTY-16: el puente valida CRC antes de retransmitir y el ruido ya no sale al aire.)*
2. **Antena desconectada:** Verificar cables coaxiales y conector SMA.
3. ~~**Latencia Excesiva:** Si hay línea de vista perfecta, pero igual falla, subir el `TIMEOUT` en `coordinador.cpp` a 10s (OPT-5).~~

> 🔴 **TACHADO EL 05/09: hoy ese consejo BAJA el umbral y mata dos reintentos.** El umbral vale
> ya **25 s**, así que «subirlo a 10 s» es reducirlo. Y no es un número suelto: es el **techo** de
> la ventana de reintentos, la lección de N-71. La cuenta sale del C++ —`CICLO_MAX_REINTENTOS = 5`
> a `TIMEOUT_ACK_MS = 3500` (`coordinador.cpp`), más los 3,0 s de cadencia del latido, dan
> **~20,8 s de peor caso**—: con el techo en 12 s los reintentos 4 y 5 eran **código muerto**,
> porque el ámbar por orfandad saltaba antes y nadie lo notaba, porque el equipo hacía algo
> razonable. **Ese techo no se toca en campo.** La desigualdad la recalcula
> `costura_09_presupuesto_radio` desde las constantes en cada corrida.
>
> Y contra la distancia la palanca correcta **no es esperar más**: la distancia sube la
> probabilidad de pérdida, no la latencia. Contra una trama perdida sirve **repetir**
> (`RF_BURST_COPIES`, SFTY-11), y antes que nada revisar antena y DIP switches.

## 2. Problemas de Luces (Lógica vs Hardware)

### Síntoma: Se encienden luces fantasmas (Rojo y Verde a la vez) o no se apagan.
**Causa:** MOSFET cruzado / dañado (Cortocircuito en el canal N).
**Diagnóstico en terreno:**
- En el código ya implementamos el **Enclavamiento Lógico (SFTY-2)**. Si ves el Verde y Rojo prendidos al mismo tiempo, **NO es culpa del software ni de la lógica del STM32**. 
- Desconecta la bornera de luces. Mide con multímetro continuidad entre el *Drain* y *Source* del MOSFET (IRLZ44N). Si pita, el MOSFET se quemó por sobrecorriente o corto en el cable del semáforo.
*Solución:* Reemplazar MOSFET en la placa (~~`Q1`-`Q9`~~ → **`Q1`–`Q10`**: son **diez** cadenas de potencia, `Q1`–`Q10` con sus optos `U6`–`U15`, no nueve. Contado sobre el `.kicad_pcb` el 28/08; otros dos documentos decían nueve y se tacharon el 05/09).

> ⚠️ **Y el borne NO está a 0 V en reposo, está a ~12 V — LEÍDO DEL COBRE, no medido con
> multímetro.** El pull-up de 1 kΩ + LED al riel de 12 V está en el netlist (`R23`, `R28`, `R33`,
> `R38`, `R43`, `R48`, `R53`, `R58`, `R63`, `R73`) y vive **en el cobre, no en el conector**: no se
> evita dejando un hilo sin poner. Con el MOSFET abierto el borne sube a ~12 V. Medir «0 V =
> apagado» en la bornera lleva a diagnosticar un canal sano como averiado; lo que se mide es la
> **continuidad Drain-Source** con la bornera desconectada, como dice el párrafo de arriba.
> **La excepción es `J8` p2** (`VERDE2`, `PA5`): el cátodo de su LED `D21` está **sin conectar**
> —red `unconnected-(D21-K-Pad1)`, cero pistas—, así que ese borne **flota** en reposo mientras los
> otros nueve suben a 12 V, y ese canal **no tiene indicador luminoso**. Queda `SIN VERIFICAR` si
> es defecto o decisión: se cierra comparando `J8` p1-p2 contra `J7` p1-p2 con el mismo estado de
> luz. *(Y encaja con lo medido en banco el 04/09: `J15` daba «0 V en rojo, 12 V en ámbar» — es
> exactamente este circuito con la sonda entre p1 y p2.)*

### Síntoma: El semáforo se reinicia solo (Se apaga todo y vuelve a prender).
**Diagnóstico:** 
- **Watchdog Timer (SFTY-1):** Desde la V8.0 el perro guardián por hardware está activo a **4.0 segundos** (`IWatchdog.begin(4000000)` en Maestro y Esclavo; el Repetidor ESP32 no lo lleva). Si la tarjeta se resetea sola, significa que el bucle principal se bloqueó más de 4 s: un pico de ruido electromagnético que colgó el procesador, ~~o el bus de la pantalla LCD trabado~~. *Nota: entre la V7.0 y la V7.6 el watchdog estuvo comentado y por tanto inactivo.*

> ⚠️ **La causa «bus de la pantalla trabado» está tachada desde el 05/09: LA PANTALLA SE
> RETIRÓ.** `lcd.cpp` sigue compilando, pero sus tres pines están fijados a `U8X8_PIN_NONE`
> (`Maestro/src/lcd.cpp`, `Esclavo/src/lcd.cpp`) y la librería se salta el `pinMode` y el
> `digitalWrite` cuando el pin es `NONE`: **no hay bus que trabar**. Lo vigila
> `costura_11_lcd_sin_bus`. Con `PB3`/`PB4`/`PB5` en alta impedancia y el ESP32 ocupando `J17`,
> volver a encender la pantalla exige **dos** cosas —devolver los pines Y sacar el módulo del
> conector—, no una.
- **Ventaja:** ¡Este reset salvó el sistema de quedarse congelado en verde para siempre!
- *Solución Definitiva si es recurrente:* Aislar mejor los optoacopladores y separar la fuente de poder de 5V (RS-485) de la de 3.3V (STM32) con filtros capacitivos o ferritas.

## 3. Guía Rápida de Diagnóstico (Leds y Pines)

* **Pin PB12 (LORA_DE_RE):** Si el radio no transmite, mide con osciloscopio o multímetro que este pin suba a 3.3V al momento de enviar, y baje a 0V para escuchar.
* **Transistores MOSFET (IRLZ44N):** Son lógicos directos. Si la salida del STM32 a la compuerta es 3.3V, el semáforo prende. Si es 0V, apaga.
* ~~**Entradas Optoaisladas (TLP127):** Si la demanda de cámaras (PB0/PB8) o botonera no reacciona, medir que haya caída de tensión en el diodo emisor del opto. ¡No puentear nunca la tierra aislada con la tierra digital!~~

> 🔴 **TACHADO EL 05/09/2026. Las tres afirmaciones de esa línea son falsas, y las tres mandan
> a alguien con un multímetro a buscar algo que no está.** Se tacha en vez de borrarse porque la
> frase lleva meses en la guía de campo y volverá a proponerse si desaparece en silencio.
>
> **1. `PB8` no es entrada de cámara desde el 27/08.** Es un LED testigo:
>
> ```
> $ grep -n "PB8" 01_Firmware/Maestro/include/pines.h
> 50:// Medido el 27/08 sobre el esquematico bueno: PB8 va por R16 1K a un LED (D5). Es un
> 60:// pin: un hilo -pad de PB8 retirando R16/D5, o uno de los cuatro pines sin cablear
> 63:#define LED_TESTIGO        PB8  // -> R16 1K -> LED D5. NO es entrada de camara
> ```
>
> La cámara de demanda entra por **`PB0`** (`CAM_DEMANDA_PIN`, bornera `J14`) y por
> **`PB14`** (`CAM_C_PIN`, `J16` p10, la que se cablea hoy — una por poste):
> `grep -n "CAM_DEMANDA_PIN\|CAM_C_PIN" 01_Firmware/Maestro/include/pines.h`.
>
> **2. NO HAY OPTO EN LA ENTRADA. No hay nada.** Los diez `TLP127` (`U6`–`U15`) están en la
> etapa de **POTENCIA**, entre el GPIO y las borneras de luz — no entre la bornera de entrada y
> el micro. Trazado pista a pista sobre el `.kicad_pcb`
> ([`03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md`](../03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md)
> §7.bis.1): cada red de `J16` tiene **exactamente cuatro pads** —la bornera, la pata de `U1`, y
> `R65`/`C26` colgando **de lado contra `GND`**, que es un antirrebote en paralelo—. **Entre la
> bornera y la pata del micro no hay resistencia en serie, ni divisor, ni diodo, ni opto.**
> Buscar una caída de tensión en un diodo emisor que no existe es tiempo perdido en lo alto de
> un poste.
>
> **3. NO HAY «TIERRA AISLADA» QUE PUENTEAR.** Medido sobre el `.kicad_pcb`: hay **UNA sola red
> `GND` en toda la tarjeta**, con **103 pads** y plano de cobre en las dos capas
> ([`05_Funcional/2_Manual_Hardware_y_Pruebas.md`](../05_Funcional/2_Manual_Hardware_y_Pruebas.md)).
> El opto separa el **pin del micro** del nodo de puerta del MOSFET; **no crea una masa
> separada**: el cátodo de su LED y la fuente del MOSFET están los dos en esa única `GND`.
> Cualquier cosa colgada de esas borneras **comparte la masa del controlador**.

* **Entradas de cámara y mando (`J16`, `J14`) — cómo se miden de verdad:** son GPIO **pelados**
  (`pinMode(..., INPUT)`), **activos en ALTO**, con un pull-**DOWN** de 10 kΩ en placa
  (`R64`–`R68`) y 3,3 V disponibles en la posición contigua del conector. El gesto que pide el
  conector es **cerrar el contacto seco contra esos 3,3 V**, nunca contra masa: en todo `J16` hay
  **una sola masa** (`p2`).

  | qué se mide | conector vacío | contacto cerrado |
  |---|---|---|
  | tensión del pin a `GND` | **0 V** | **3,3 V** |
  | resistencia del pin a `GND` | **~10 kΩ** *(9,92–9,94 kΩ medidos, paso 20 del 03/09)* | — |

  Si el pin no está a 0 V con el conector vacío, el sospechoso es el pull-down (`R64`–`R68`), no
  un opto. *(Y ojo con el histórico: `0,6 V` en reposo no es un defecto de placa — es lo que da
  un `INPUT_PULLUP` interno de ~40 kΩ contra ese 10 kΩ. Los `0,6 V` de N-118 se midieron con un
  binario que aún ponía `INPUT_PULLUP`; el firmware de hoy pone `INPUT` pelado y da 0 V.)*

> ⛔ **ANTES DE ENCHUFAR NADA EN `J16`: `p1` LLEVA 12 V CRUDOS** —sin opto, sin resistencia en
> serie, sin clamp— a nueve posiciones de `p10`. Y la separación real en cobre entre la red de
> 12 V y las de botón baja a **1,359 mm** (§7.bis.2 del mapeo), no a los 10 mm del paso del
> conector. **Tapar `p1` físicamente es obligatorio en cada equipo que se monte** (N-120), no una
> cautela de banco: un error de una posición mete 12 V en un pin de 3,3 V sin nada en medio.

---

## 🔌 RS-485 half-duplex: el control DE/RE

RS-485 usa **un solo par de hilos para ambos sentidos**. Mientras un extremo excita la línea, el
otro **no puede hablar**. Quién habla lo decide la señal DE/RE del transceptor MAX3485.

**Síntoma clásico:** si un DE/RE se queda permanentemente en alto, esa línea queda bloqueada en
**ambos** sentidos. Se manifiesta como dos problemas que parecen distintos:

- El LED **TX de la radio queda encendido fijo** (recibe un flujo interminable y lo radia al aire).
- **No hay datos de vuelta** — el otro extremo no puede meter nada en un bus permanentemente tomado.

Ocurrió en el repetidor el 31/07/2026: el ESP32 levantaba DE/RE ante cualquier byte y solo lo
bajaba tras 5 ms de silencio; con ruido continuo en la línea de entrada, ese silencio nunca llegaba.

**Desde V8.3** el puente solo levanta DE/RE durante los ~4 ms que dura emitir una trama válida.

| Lo que se ve | Qué significa |
|---|---|
| TX **encendido fijo** | DE/RE clavado o ruido continuo. Bus bloqueado en ambos sentidos |
| TX **destella cada 3 s** | Correcto |
| Llega en un sentido y **no vuelve nada** | Sospechar del DE/RE de ese tramo, no de la radio del otro extremo |

---

## 📡 Enlace de radio que funciona en un solo sentido

La propagación es **recíproca**: si A alcanza a B, la pérdida del camino es idéntica en sentido
contrario. **Un enlace no puede fallar solo en una dirección por causas de propagación.** Solo dos
cosas rompen esa simetría: **potencia de transmisión distinta** o **un receptor/transmisor averiado**.

### Prueba de intercambio de frecuencias

Resolvió el caso del 31/07/2026 sin instrumentos:

1. Anotar qué radio está en cada banda y qué falla exactamente.
2. **Intercambiar las asignaciones de frecuencia** entre los extremos.
3. Volver a observar.

| Observación | Conclusión |
|---|---|
| El fallo se queda con el mismo **destino** | Problema del enlace hacia ese punto |
| El fallo sigue a la misma **radio**, aunque cambie de banda y destino | **Transmisor de esa radio averiado** |

**Caso real:** una radio recibía correctamente en ambas configuraciones, pero **nadie oía sus
transmisiones** — ni el Maestro ni el Esclavo, en bandas distintas. La otra radio del repetidor sí
transmitía. Factor común identificado en una sola prueba.

### Por qué suele ser la antena, no el radio

Un receptor capta algo aunque la antena esté floja. **Un transmisor con la antena en mal estado no
radia casi nada**: la potencia se refleja. De ahí el sintoma enganoso de "el LED dice que transmite
pero nadie lo oye".

Revisar en este orden: **conector SMA**, **cable coaxial**, **antena de la banda correcta**, y por
ultimo intercambiar el radio por uno sano.

> **Transmitir con la antena en mal estado dana el amplificador de potencia.** Con 1 W bastan unas
> horas. Al montar un reemplazo, revisar primero el coaxial: si se monta sobre el cable que causo la
> averia, se quema tambien el radio nuevo.

### Antes de nada: los DIP switches

En operacion, **M0 y M1 van SIEMPRE los dos en OFF**. Cualquier otra combinacion mete la radio en un
modo especial donde la transmision puede quedar deshabilitada mientras la recepcion sigue activa.
Es la causa mas frecuente de un enlace de un solo sentido, y se comprueba en diez segundos.
