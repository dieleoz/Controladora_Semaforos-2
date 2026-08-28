# 📘 MANUAL DE DESPLIEGUE FUNCIONAL - SISTEMA DIRECTO (2 RADIOS)

Este manual describe el paso a paso para el equipo funcional encargado de las pruebas de campo en el escenario estándar, donde **SÍ HAY LÍNEA DE VISTA** directa entre el Semáforo Maestro y el Semáforo Esclavo (No requiere repetidor ESP32).

---

## 1. 🏗️ ARQUITECTURA (Hardware)

El sistema utiliza **2 módulos de radio RF (Lora/RS485)**:
`[MAESTRO STM32]` <--> `[Radio A]` ~~~(RF Directo)~~~ `[Radio B]` <--> `[ESCLAVO STM32]`

### 1.1 Configuración Previa de Radios
Conecte los 2 radios al PC (usando módulo USB-RS485) y configúrelos idénticos:
- **Tasa de aire (Baud rate aéreo):** `0.3 kbps` (Crucial para máxima penetración y sensibilidad -147 dBm).
- **Tasa serial (Baud rate físico):** `9600 bps`.
- **Canal / Frecuencia:** El mismo para ambos.

---

## 2. 🔌 DIAGRAMA DE CABLEADO (PINOUT CORREGIDO)

> [!WARNING]
> **CORRECCIÓN IMPORTANTE DE PINES:** Los radios deben ir conectados al puerto **RS485 OUT**, no al puerto IN. El puerto IN (PA9/PA10) está reservado para la cámara de Inteligencia Artificial.

### A. Semáforo Maestro (STM32)
* **Pin DI del RS485 (Transmisión):** Conectar al pin `PB10` (TX) del STM32.
* **Pin RO del RS485 (Recepción):** Conectar al pin `PB11` (RX) del STM32.
* **Pin DE/RE del RS485 (Control):** Conectar al pin `PB12` del STM32.

### B. Semáforo Esclavo (STM32)
* **Pin DI del RS485 (Transmisión):** Conectar al pin `PB10` (TX) del STM32.
* **Pin RO del RS485 (Recepción):** Conectar al pin `PB11` (RX) del STM32.
* **Pin DE/RE del RS485 (Control):** Conectar al pin `PB12` del STM32.

---

## 3. 🚀 CARGA DE FIRMWARE

El paquete contiene los binarios compilados listos para quemar en las tarjetas usando *STM32CubeProgrammer*:

1. **Maestro:** Grabar `firmware_maestro.bin` en la placa del Semáforo 1.
2. **Esclavo:** Grabar `firmware_esclavo.bin` en la placa del Semáforo 2.

---

## 4. 🧪 PROTOCOLO DE PRUEBAS FUNCIONALES EN CAMPO

1. **Prueba de Encendido y Boot:** 
   * Encienda el Esclavo. Deberá arrancar forzado en luz ROJA de seguridad.
   * Encienda el Maestro. Ingresará al Menú LCD.
2. **Prueba de Latencia:** 
   * En el Maestro, seleccione `Modo Manual`.
   * Presione el botón de cambio de estado. 
   * Verifique que el Esclavo cambie en menos de 0.5 segundos.
3. **Prueba Automática:**
   * Inicie el `Modo Inteligente` o `Automático`. El sistema debe ciclar perfectamente.
4. **Prueba de Resiliencia (Safety Case P-02):**
   * Desconecte la energía o la antena del Radio Maestro.
   * El Esclavo dejará de recibir el "Heartbeat/Comando". A los 5 segundos debe pasar a `FALLO (Luz Amarilla intermitente o Roja)`.
   * Vuelva a conectarlo. El sistema debe reconectarse y sincronizarse automáticamente.
