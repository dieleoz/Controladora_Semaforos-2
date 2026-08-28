# TROUBLESHOOTING - Controladora de Semáforos V4

Esta bitácora documenta las fallas esperadas y reportadas en campo, su diagnóstico y solución. Dado que la tarjeta (STM32 + MOSFETs) ya está impresa, las soluciones se enfocan en software o modificaciones externas al hardware.

## 1. Problemas de Comunicación (Radios LoRa)

### Síntoma: Ambos semáforos se quedan en Ámbar Intermitente (S_FALLO)
**Diagnóstico:** El maestro no está recibiendo los `PONG` del esclavo o viceversa por más de 5 segundos.
**Posibles Causas:**
1. **Ruido en Repetidor (ESP32):** El repetidor se congeló por leer basura electromagnética (ver GAP OPT-5). *Solución temporal:* Reiniciar el repetidor. *Solución Definitiva:* Actualizar firmware del ESP32 para limpiar buffer por timeout.
2. **Antena desconectada:** Verificar cables coaxiales y conector SMA.
3. **Latencia Excesiva:** Si hay línea de vista perfecta, pero igual falla, subir el `TIMEOUT` en `coordinador.cpp` a 10s (OPT-5).

## 2. Problemas de Luces (Lógica vs Hardware)

### Síntoma: Se encienden luces fantasmas (Rojo y Verde a la vez) o no se apagan.
**Causa:** MOSFET cruzado / dañado (Cortocircuito en el canal N).
**Diagnóstico en terreno:**
- En el código ya implementamos el **Enclavamiento Lógico (SFTY-2)**. Si ves el Verde y Rojo prendidos al mismo tiempo, **NO es culpa del software ni de la lógica del STM32**. 
- Desconecta la bornera de luces. Mide con multímetro continuidad entre el *Drain* y *Source* del MOSFET (IRLZ44N). Si pita, el MOSFET se quemó por sobrecorriente o corto en el cable del semáforo.
*Solución:* Reemplazar MOSFET en la placa (Q1-Q9).

### Síntoma: El semáforo se reinicia solo (Se apaga todo y vuelve a prender).
**Diagnóstico:** 
- **Watchdog Timer (SFTY-1):** Desde la V8.0 el perro guardián por hardware está activo a **4.0 segundos** (`IWatchdog.begin(4000000)` en Maestro y Esclavo; el Repetidor ESP32 no lo lleva). Si la tarjeta se resetea sola, significa que el bucle principal se bloqueó más de 4 s: un pico de ruido electromagnético que colgó el procesador, o el bus de la pantalla LCD trabado. *Nota: entre la V7.0 y la V7.6 el watchdog estuvo comentado y por tanto inactivo.*
- **Ventaja:** ¡Este reset salvó el sistema de quedarse congelado en verde para siempre!
- *Solución Definitiva si es recurrente:* Aislar mejor los optoacopladores y separar la fuente de poder de 5V (RS-485) de la de 3.3V (STM32) con filtros capacitivos o ferritas.

## 3. Guía Rápida de Diagnóstico (Leds y Pines)

* **Pin PB12 (LORA_DE_RE):** Si el radio no transmite, mide con osciloscopio o multímetro que este pin suba a 3.3V al momento de enviar, y baje a 0V para escuchar.
* **Transistores MOSFET (IRLZ44N):** Son lógicos directos. Si la salida del STM32 a la compuerta es 3.3V, el semáforo prende. Si es 0V, apaga.
* **Entradas Optoaisladas (TLP127):** Si la demanda de cámaras (PB0/PB8) o botonera no reacciona, medir que haya caída de tensión en el diodo emisor del opto. ¡No puentear nunca la tierra aislada con la tierra digital!

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
