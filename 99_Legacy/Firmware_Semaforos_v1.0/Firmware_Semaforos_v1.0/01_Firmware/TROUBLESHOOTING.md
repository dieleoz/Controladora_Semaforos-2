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
- **Watchdog Timer (SFTY-1):** En la V4 activamos el perro guardián por hardware a 2 segundos. Si la tarjeta se resetea sola (sin tocar el botón), significa que hubo un pico de ruido electromagnético que colgó el procesador, o que el bus I2C de la pantalla LCD se trabó.
- **Ventaja:** ¡Este reset salvó el sistema de quedarse congelado en verde para siempre!
- *Solución Definitiva si es recurrente:* Aislar mejor los optoacopladores y separar la fuente de poder de 5V (RS-485) de la de 3.3V (STM32) con filtros capacitivos o ferritas.

## 3. Guía Rápida de Diagnóstico (Leds y Pines)

* **Pin PB0 (LORA_DE_RE):** Si el radio no transmite, mide con osciloscopio o multímetro que este pin suba a 3.3V al momento de enviar, y baje a 0V para escuchar.
* **Transistores MOSFET (IRLZ44N):** Son lógicos directos. Si la salida del STM32 a la compuerta es 3.3V, el semáforo prende. Si es 0V, apaga.
* **Entradas Optoaisladas (TLP127):** Si el botón peatonal (futuro) no hace nada, medir que haya caída de tensión en el diodo emisor del opto. ¡No puentear nunca la tierra aislada (GND_IN) con la tierra digital (GND_MCU)!
