# ⚙️ Manual de Hardware y Topología Física (V7.6 Definitiva)

Este documento describe la arquitectura física de las placas impresas (PCBs), los componentes electrónicos, y la topología de red utilizada en el ecosistema de semáforos móviles.

---

## 1. Topología de Comunicaciones y Conexión de Radios RS485

Para superar los obstáculos del terreno (montañas, maquinaria, polvo), el sistema utiliza radios industriales **E90-DTU (RS485/232 $\leftrightarrow$ RF)**.

```text
+------------------------+        2 Hilos RS485       +------------------------+
| Tarjeta Semáforo STM32 |                            |     Radio E90-DTU      |
|  (MAX3485 Integrado)   |                            |   (Caja Metalica RF)   |
|                        |                            |                        |
|        Bornera A  ----|----------------------------|----> Pin 485_A         |
|        Bornera B  ----|----------------------------|----> Pin 485_B         |
|                        |                            |      Pin V+ (12-24V)   |
|        GND / V-   ----|----------------------------|----> Pin V-            |
+------------------------+                            +------------------------+
```

### 💡 Reglas Inquebrantables de Cableado:
1. **La PCB del Semáforo ya incluye el integrado MAX3485:** Convierte las señales TTL del STM32 (`PB10`, `PB11`, `PB12`) a bus diferencial RS485.
2. **Conexión de solo 2 hilos de datos hacia la radio E90-DTU:**
   - Bornera **`A`** de la Tarjeta Semáforo $\rightarrow$ Pin **`485_A`** de la radio E90-DTU.
   - Bornera **`B`** de la Tarjeta Semáforo $\rightarrow$ Pin **`485_B`** de la radio E90-DTU.
   - **`V+`** y **`V-`** para alimentación de 12V/24V de la radio.
3. **Autoconmutación TX/RX:** La radio E90-DTU realiza la conmutación de dirección RS485 automáticamente por hardware. NO se requiere cablear pines DE/RE hacia la radio.

---

## 2. Topologías de Red Soportadas (Transparente Automático)

El firmware V7.6 en las placas STM32 es 100% agnóstico a la topología de red instalada:

### Opción A: Modo Directo (2 Radios - Línea de Vista)
- **Maestro:** Conectado por bornera A/B a Radio 1 (Frecuencia 170.0 MHz).
- **Esclavo:** Conectado por bornera A/B a Radio 2 (Frecuencia 170.0 MHz).
- **Operación:** Transmisión directa transparente por aire. No requiere la tarjeta Repetidor ESP32.

### Opción B: Modo Repetidor (4 Radios - Esquinas Ciegas / Montaña)
- **Maestro:** Conectado a Radio 1 (Frecuencia 170.0 MHz).
- **Repetidor ESP32 (Físico Back-to-Back V7.6):**
  - Radio B1 en 170.0 MHz (Habla con el Maestro).
  - Radio B2 en 172.0 MHz (Habla con el Esclavo).
  - El ESP32 reenvía tramas con temporizador de ráfaga inter-byte de 15ms y 2ms de estabilización RS485 en `01_Firmware/Repetidor`.
- **Esclavo:** Conectado a Radio 4 en 172.0 MHz.

### Opción C: Modo Cable Físico Directo (Respaldo si el radio no va)
- **Maestro:** Bornera `RS485_OUT` (Pines A, B, GND).
- **Esclavo:** Bornera `RS485_OUT` (Pines A, B, GND).
- **Conexión:** Cable trenzado directo 3 hilos (A con A, B con B, GND con GND).
- **Operación:** Comunicación cableada transparente sin necesidad de radios.

---

## 3. Integración del Sistema de 4 Cámaras IA (2 por Nodo Semafórico)

Para control vehicular por demanda en pasos alternados de obra móvil con cámaras Hikvision AcuSense:
* **En el Semáforo Maestro (Extremo 1):**
  * **Cámara 1 (Demanda Sentido 1):** Contacto seco `1A`/`1B` conectado a **`PB0`** (Entrada Libre) y **`GND`**.
  * **Cámara 2 (Umbral Tramo 1):** Contacto seco `1A`/`1B` conectado a **`PB8`** (Entrada Libre) y **`GND`**.
* **En el Semáforo Esclavo (Extremo 2):**
  * **Cámara 3 (Demanda Sentido 2):** Contacto seco `1A`/`1B` conectado a **`PB0`** (Entrada Libre) y **`GND`**.
  * **Cámara 4 (Umbral Tramo 2):** Contacto seco `1A`/`1B` conectado a **`PB8`** (Entrada Libre) y **`GND`**.
* **Independencia de Buses:** Los pulsos de detección entran por las entradas dedicadas `PB0` y `PB8` sin interferir con los botones del panel LCD (`PB9`, `PB13`, `PB14`, `PB15`) ni con el bus de sincronización `RS485_OUT`. Detalle completo en [`05_Funcional/9_Manual_Parametrizacion_Camara_IA.md`](05_Funcional/9_Manual_Parametrizacion_Camara_IA.md).

---

## 4. Módulo Bluetooth para Telemetría y Diagnóstico Móvil

Para soporte técnico, caja negra de alarmas y monitoreo desde el suelo sin subir al poste (estándar probado en proyecto Baliza):
* **Pines de Conexión:** Puerto USART1 del STM32 (`PA9` TX ➔ `RXD` BT, `PA10` RX ➔ `TXD` BT).
* **Desacoplo Eléctrico:** Pin `PA8` forzado en `HIGH` permanente para poner en alta impedancia ($\text{Hi-Z}$) el transceptor `MAX3485 U3`.
* **Alimentación:** `5V` (o `3.3V`) y `GND` de la tarjeta controladora.
* **Funcionalidad:** Envío continuo de telemetría (estado de luces, modo, cuenta regresiva `T:`, calidad de señal RF, % de paquetes y motivos de alarma con hora exacta de RTC) hacia la App en celular con comandos protegidos por PIN (`1234`). Detalle completo en [`05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.md`](05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.md).

---

## 5. Placa Base (Mainboard STM32)

La tarjeta principal de control que gobierna las luces del semáforo.

### Componentes Clave:
- **Cerebro:** Microcontrolador **STM32F103C8T6** ("Blue Pill" / CKS32F103).
- **Pantalla y UI:** LCD ST7920 (128x64 píxeles) conectada por SPI de 3 hilos. 4 botones físicos de navegación.
- **Salidas de Potencia:** Transistores MOSFET N-Channel (**IRLZ44N**) a 12V/24V para lámparas LED (Rojo, Amarillo, Verde).
- **Protección Galvánica:** 9 Entradas Optoacopladas para aislamiento eléctrico de botoneras y sensores.
- **Transceptor Integrado:** Chip MAX485 (Half-Duplex) conectado a la bornera RS485 `A` / `B`.
- **Watchdog:** Inicialización IWDG por registros directos adaptada a CKS32F103 (refresco en `loop()`).

---

## 6. Placa Repetidora (ESP32 V7.6)

Ubicada en la carpeta [`01_Firmware/Repetidor`](file:///d:/@Proyect/Controladora_Semaforos/01_Firmware/Repetidor).

### Componentes Clave:
- **Cerebro:** Microcontrolador **ESP32 Dev Module**.
- **Puertos Seriales:** `RadioA` (UART1: RX=16, TX=17, DE/RE=4) y `RadioC` (UART2: RX=32, TX=33, DE/RE=22).
- **Firmware V7.6:** Passthrough asíncrono con `delay(2)` y 15ms de ventana de ráfaga que previene la fragmentación de tramas binarias.

---

## 7. Guía de Pruebas y Carga de Firmware en PlatformIO

### 5.1 Carga del Firmware a los STM32 y ESP32:
En PlatformIO (VSCode), usar el botón **"Open Project"** y abrir de forma independiente la carpeta específica del nodo a programar:
1. `01_Firmware/Maestro` para la tarjeta Maestro STM32.
2. `01_Firmware/Esclavo` para la tarjeta Esclavo STM32.
3. `01_Firmware/Repetidor` para la tarjeta Repetidor ESP32.

> **Tip para Chips Clones CKS32F103:** Si el ST-Link rechaza la primera carga, mantener presionado el botón RESET de la tarjeta BluePill al hacer clic en "Upload" y soltarlo en el instante en que la consola indique "Connecting...".
