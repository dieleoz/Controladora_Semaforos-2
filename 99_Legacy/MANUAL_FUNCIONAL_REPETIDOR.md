# 📘 MANUAL DE DESPLIEGUE FUNCIONAL - SISTEMA DE 4 RADIOS CON REPETIDOR B

Este manual describe el paso a paso para el equipo funcional encargado de las pruebas de campo. Detalla el conexionado, la configuración y el flujo de pruebas para el escenario con obstáculos donde no hay línea de vista directa entre los semáforos, requiriendo un **Repetidor ESP32** en la mitad.

---

## 1. 🏗️ ARQUITECTURA DE 4 RADIOS (Hardware)

El sistema ahora utiliza **4 módulos de radio RF (Lora/RS485)** para saltar un obstáculo:
`[MAESTRO STM32]` <--> `[Radio A]` ~~~(RF)~~~ `[Radio B]` <--> `[REPETIDOR ESP32]` <--> `[Radio C]` ~~~(RF)~~~ `[Radio D]` <--> `[ESCLAVO STM32]`

### 1.1 Topología de Doble Frecuencia (Anti-Eco)
Para evitar que las ondas choquen o generen eco, es **OBLIGATORIO** usar frecuencias distintas en cada "salto", justo como el ingeniero de campo propuso:

* **Salto 1 (Maestro <-> Repetidor):**
  * Radio Maestro (Radio A): Configurar a **170 MHz**.
  * Radio Repetidor Lado A (Radio B1): Configurar a **170 MHz**.
* **Salto 2 (Repetidor <-> Esclavo):**
  * Radio Repetidor Lado C (Radio B2): Configurar a **172 MHz**.
  * Radio Esclavo (Radio D): Configurar a **172 MHz**.

### 1.2 Configuración de los 4 Radios (Air Rate y Serial)
Conecte los 4 radios al PC (usando módulo USB-RS485) y asegúrese de que tengan estos parámetros vitales:
- **Tasa de aire (Baud rate aéreo):** `0.3 kbps` (Crucial para máxima penetración en obstáculos).
- **Tasa serial (Baud rate físico):** `9600 bps` (Para comunicarse con el ESP32/STM32).
- **Modo:** Transparente (Unvarnished).

---

## 2. 🔌 DIAGRAMA DE CABLEADO (PINOUT)

### A. Semáforo Maestro (STM32)
* **Pin DI del RS485 (Transmisión):** Conectar al pin `PB10` (TX) del STM32.
* **Pin RO del RS485 (Recepción):** Conectar al pin `PB11` (RX) del STM32.
* **Pin DE/RE del RS485 (Control):** Conectar al pin `PB12` del STM32.

### B. Repetidor Central (ESP32)
El ESP32 actúa como un puente transparente. Tiene dos radios conectados: uno mira hacia el Maestro y el otro hacia el Esclavo.

* **Radio B1 (hacia el Maestro, 170 MHz):**
  * `Pin DI` del RS485 -> Pin `17` (TX) del ESP32.
  * `Pin RO` del RS485 -> Pin `16` (RX) del ESP32.
  * `Pin DE/RE` del RS485 -> Pin `4` del ESP32.

* **Radio B2 (hacia el Esclavo, 172 MHz):**
  * `Pin DI` del RS485 -> Pin `33` (TX) del ESP32.
  * `Pin RO` del RS485 -> Pin `32` (RX) del ESP32.
  * `Pin DE/RE` del RS485 -> Pin `22` del ESP32.

### C. Semáforo Esclavo (STM32)
* **Pin DI del RS485 (Transmisión):** Conectar al pin `PB10` (TX) del STM32.
* **Pin RO del RS485 (Recepción):** Conectar al pin `PB11` (RX) del STM32.
* **Pin DE/RE del RS485 (Control):** Conectar al pin `PB12` del STM32.

---

## 3. 🚀 CARGA DE FIRMWARE

El paquete contiene los siguientes archivos binarios compilados listos para quemar en las tarjetas.
Para quemar, use la herramienta de flasheo (STM32CubeProgrammer para STM32 y ESP-Flasher para el ESP32):

1. **Maestro:** Grabar `firmware_maestro.bin` en la placa del Semáforo 1.
2. **Esclavo:** Grabar `firmware_esclavo.bin` en la placa del Semáforo 2.
3. **Repetidor:** Grabar `firmware_repetidor.bin` en el módulo ESP32 puente.

---

## 4. 🧪 PROTOCOLO DE PRUEBAS FUNCIONALES EN CAMPO

Una vez instalado el hardware en campo:

1. **Prueba de Encendido y Boot:** 
   * Encienda el Esclavo. Deberá arrancar forzado en luz ROJA de seguridad.
   * Encienda el Repetidor ESP32.
   * Encienda el Maestro. Entrará al Menú LCD.
2. **Prueba de Latencia:** 
   * En el Maestro, seleccione `Modo Manual`.
   * Presione el botón de cambio de estado. 
   * Verifique que el Esclavo cambie en menos de 1 segundo (el paquete de 4 bytes viaja por 4 radios instantáneamente).
3. **Prueba de Obstáculo (El Caso de Uso):**
   * Ubique el Maestro y Esclavo separados por un edificio o cerro que corte la línea de visión directa.
   * Instale el Repetidor ESP32 en un punto alto intermedio con visión a ambas partes (ej: terraza de un edificio central o antena repetidora).
   * Verifique operación en `Modo Inteligente` (ciclos automáticos).
4. **Prueba de Resiliencia (Safety Case P-02):**
   * Desconecte la energía del Repetidor ESP32.
   * El Esclavo dejará de recibir señales. A los 5 segundos debe pasar a `FALLO (Intermitencia Amarilla o Rojo fijo según norma)`.
   * El Maestro detectará la falla al no recibir ACKs y se protegerá.
   * Vuelva a energizar el Repetidor ESP32. El sistema debe reconectarse y sincronizarse automáticamente.
