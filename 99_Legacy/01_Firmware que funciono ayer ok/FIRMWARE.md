# Arquitectura del Firmware - Controladora de Semáforos

Este documento desglosa la arquitectura de software del código fuente alojado en `01_Firmware/Semaforos/`, analizando cómo está construido, los módulos principales y el ciclo de vida del sistema.

---

## 1. Patrón de Diseño y Ciclo de Vida
El firmware está diseñado bajo un patrón de **Máquina de Estados Finitos (FSM)** no bloqueante. Esto significa que el procesador (STM32) nunca se queda atrapado en bucles de espera (`delay()`), permitiendo que tareas como refrescar la pantalla LCD, leer botones y procesar mensajes de radio ocurran en paralelo de forma fluida.

### Ciclo de vida principal (`main.cpp`):
1. **Setup:** Inicializa pines, pantalla (U8g2) y periféricos (HardwareSerial).
2. **Arranque:** Entra directamente al estado `SISTEMA` y carga el `MENU`. (En iteraciones previas se bloqueaba esperando el radio, lo cual ya fue corregido).
3. **Loop:**
   - Mantiene vivo el hilo de comunicación en segundo plano (`coordinador_actualizar_background()`).
   - Verifica el estado del sistema (`MENU`, `MODO_MANUAL`, `MODO_AUTOMATICO`).
   - Ejecuta el loop del modo correspondiente.

---

## 2. Mapa de Módulos (Archivos Fuente)

| Módulo | Archivos | Responsabilidad |
|---|---|---|
| **Gestor Principal** | `main.cpp` | Orquesta los modos de operación, detecta fallos de radio y arranca el menú. |
| **Menú y UI** | `menu.cpp/h`, `lcd.cpp/h` | Maneja la renderización en el ST7920 (128x64) y la selección de modos. *Totalmente aislado de la lógica de red.* |
| **Coordinador (Red)** | `coordinador.cpp/h` | El "Cerebro" de la sincronización. Maneja la máquina de estados de paso (Verde Maestro ↔ Verde Esclavo). Realiza PING/PONG (Heartbeats) y gestiona timeouts. |
| **Protocolo Físico** | `protocolo.cpp/h` | Abstracción de capa física. Inicializa la USART3 (`PB10/PB11`), manipula el pin `LORA_DE_RE` (`PB0`) para activar el transmisor MAX485 y envía las cadenas de texto (`GO_RED`, `ACK_GREEN`). |
| **Hardware Abstraction** | `semaforo.cpp/h`, `pines.h` | Control de MOSFETs (`ROJO1`, `VERDE1`, etc.). Transiciones de luces y el estado `S_FALLO` (ámbar intermitente). |
| **Modos de Operación** | `modo_automatico.cpp`, `modo_manual.cpp` | Lógica específica de cada modo. El modo automático cuenta tiempos y ordena al coordinador pedir cambio de vía. |

---

## 3. Topología de Red y Handshake

La comunicación utiliza un formato simple basado en texto ASCII (cadenas terminadas en `\n`):

- **Latido (Heartbeat):** El Maestro envía `PING` cada 2 segundos. El esclavo responde `PONG`. Si pasan 5 segundos sin PONG, el Maestro declara `C_FALLO` y pasa a Ámbar Intermitente.
- **Transición de luces:**
  1. Maestro envía `GO_GREEN` al esclavo.
  2. Esclavo pasa sus luces a Verde y responde `ACK_GREEN`.
  3. Maestro espera el `ACK_GREEN` para considerar que la vía contraria fluye.
  *(El mismo flujo aplica para `GO_RED` / `ACK_RED`).*

> **Nota de Optimización:** Este sistema de `ACK` es estricto. Si los radios introducen latencia (ej. cadena de repetidores), el `TIMEOUT_ACK_MS` (actualmente en 4000 ms) debe incrementarse en `coordinador.cpp` para evitar falsos positivos de fallo.

---

## 4. Maestro vs Esclavo (Compilación)
El proyecto es de código único, pero para instanciar a la tarjeta remota (Esclavo):
- La carpeta `src/` contiene un archivo llamado `esclavo.txt`.
- Para grabar el esclavo, se debe copiar el código de `esclavo.txt` y reemplazar el contenido de `main.cpp`.
- **Diferencia lógica:** El esclavo no tiene menú ni toma decisiones de tiempo. Simplemente escucha la USART3, responde PONGs y obedece comandos `GO_RED` o `GO_GREEN`, notificando cuando ya ejecutó la orden. Su única lógica autónoma es entrar en `S_FALLO` si deja de recibir PINGs del maestro.
