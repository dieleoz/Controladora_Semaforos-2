# 🚦 Manual de Operación y Comportamiento del Sistema (V8.9 Definitiva)

Este manual define el **"Ground Truth"** (la verdad absoluta) de cómo DEBE comportarse el sistema, sirviendo como base para validar que las simulaciones y el código cumplan con la especificación.
Todas las operaciones están alineadas al **Manual de Señalización Vial de Colombia (Resolución 2024 - MinTransporte)**.

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
- **Configuración:** La interfaz del menú LCD permite configurar tiempos de despeje de 5 a 999 segundos (piso mínimo de 5s por seguridad vial, hasta 16.6 minutos) para dar cobertura total a puentes largos o túneles de 500m.

---

## 2. Comportamiento en Destello / Intermitente (Bajo Flujo)

Según el Manual de Señalización (2024), si el flujo vehicular baja al 50% o menos durante 4 horas o más (usualmente operación nocturna), el sistema debe pasar a operación intermitente.
- **Funcionamiento:** Un semáforo parpadea en 🟡 **Ámbar** (Precaución - vía principal) y el otro en 🔴 **Rojo** (Pare - vía secundaria), o ambos en Rojo Intermitente para pasos de igual jerarquía.

---

## 3. Comportamiento de la Interfaz y Menú (LCD ST7920)

El acceso a la pantalla LCD y los botones de configuración es crítico para los operarios.
- **Regla de Oro (Independencia de Red):** El operario DEBE poder acceder al menú de configuración (para elegir modo Manual, Automático o Inteligente, y fijar tiempos) **incluso si las radios están apagadas o no hay comunicación con el esclavo**.
- **Comportamiento en Menú:** En el Menú Principal, si hay comunicación el Maestro mantiene **🔴 ROJO FIJO continuo en ambos semáforos** sin congelar la pantalla. Si no hay comunicación, indica orfandad pasando a Amarillo Intermitente.
- **Arranque Inmediato:** Al seleccionar un modo en el menú, el sistema aplica inmediatamente el tiempo de Despeje All-Red en ambos extremos.

### Prueba de Alcance
Cuarta opción del Menú Principal. Muestra **calidad de enlace en %**, barra gráfica, **tiempo de respuesta** en ms y fallos consecutivos, actualizándose cada 3 segundos. Permite determinar la cobertura real de radio desplazando el equipo, en lugar de estimarla.
Mientras está activa, ambos semáforos permanecen en **🔴 Rojo Fijo** (o Amarillo Intermitente sin enlace), igual que en el Menú. **No arranca ciclos.** Se sale con el Botón 4.

---

## 4. Comportamiento ante Fallas (Fail-Safe & Self-Healing Real)

1. **Pérdida de Comunicación (SFTY-6):** Si se pierde comunicación por más de 12.0 segundos, el sistema entra automáticamente en `C_FALLO` / `S_FALLO` (🟡 **Amarillo Intermitente**). En `C_FALLO`, el Maestro envía `CMD_GO_RED` para obligar al Esclavo a pasar a Rojo o Amarillo Intermitente por timeout.
2. **Auto-Recuperación Autónoma (Self-Healing Real):** Al restablecerse la señal de radio, el sistema **NO requiere reinicio manual**. Limpia automáticamente el registro de duplicados (`protocolo_resetReplayProtection()`), fuerza Rojo Estático (All-Red) de 15 segundos en ambos semáforos para limpiar la vía y reanuda el ciclo lumínico sin intervención técnica.
3. **Cuelgue de Procesador (Ruido EMI):** El Watchdog interno (`IWatchdog` activo a 4.0s) reinicia el procesador ante interferencias severas.

---

## 5. Resiliencia RF: Ráfaga configurable y Ventana Deslizante (SFTY-11)

Para garantizar comunicación inquebrantable en zonas de montaña con alta interferencia:
- **Ráfaga (Burst):** 1 copia de 4 bytes con FEC activo en radios E90-DTU.
- **Ventana Deslizante (Sliding Window):** Procesamiento asíncrono con CRC-8 Maxim (`0x31`).
- **Protección Antirepetida (Replay Protection):** Descarte de duplicados mediante `msgID`.

---

## 6. Integración de 4 Cámaras IA para Demanda Vehicular (AcuSense G2)

Para detección inteligente de flujo vehicular en pasos alternados de obra:
* **Semáforo Maestro:**
  * **Cámara 1 (Aproximación Sentido 1):** Contacto seco `1A`/`1B` en **`PB9`** y `GND` ➔ Demanda Verde Maestro.
  * **Cámara 2 (Monitoreo Obra Sentido 1):** Contacto seco `1A`/`1B` en **`PB13`** y `GND` ➔ Confirma flujo interno.
* **Semáforo Esclavo:**
  * **Cámara 3 (Aproximación Sentido 2):** Contacto seco `1A`/`1B` en **`PB9`** y `GND` ➔ Demanda Verde Esclavo.
  * **Cámara 4 (Monitoreo Obra Sentido 2):** Contacto seco `1A`/`1B` en **`PB13`** y `GND` ➔ Confirma flujo interno.
* **Seguridad:** Cada cambio de sentido exige obligatoriamente los **15 segundos mínimos de All-Red (Todo-Rojo)** antes de habilitar el verde opuesto.

---

## 7. Vocabulario Oficial del Mando a Distancia de 4 Relés (Anti-Colisión N-53)

Para permitir la operación del semáforo a nivel del suelo sin colisionar con la edición de parámetros en pantalla:

| Secuencia | Modo Activado | Confirmación Lumínica |
|---|---|---|
| **`A · B · A`** (≤12s) | 🟢 **Modo Automático** | 2 destellos rojos |
| **`B · A · B`** (≤12s) | 🟡 **Modo Ámbar (Seguro)** | 3 destellos rojos |
| **`B · A · B · A`** (≤18s) | ✋ **Modo Manual (Operario)** | 5 destellos rojos |
| **`A · B · A · B`** (≤18s) | 🕒 **Modo Degradado (Reloj RTC)** | 4 destellos rojos |
| **`A · A · B · B`** (≤18s) | 📷 **Modo Inteligente (Cámaras IA)** | 6 destellos rojos |

* **Inhibición de UI (N-53):** Mientras el operador esté en pantallas de configuración (`AJUSTAR HORA`, `CONFIG_TIEMPOS`), el receptor del mando se inhibe al 100%, permitiendo ajustar números rápidamente con el codillo sin disparar cambios de modo involuntarios.

---

## 8. Módulo Bluetooth para Telemetría y Diagnóstico Móvil (Estándar Baliza)

Para soporte técnico en campo sin escaleras:
* **Conexión Hardware:** Puerto USART1 (`PA9` TX, `PA10` RX), alimentado con 5V/3.3V de la PCB.
* **Telemetría en Vivo:** Emisión periódica de `$STATUS,...` cada 1 segundo con modo, fase de luces, cuenta regresiva, % de señal RF y hora exacta del RTC.
* **Caja Negra de Alarmas:** Registro inmediato de eventos con timestamp (`$ALARM,EVENTO:FALLO_RF_12S...`) para diagnosticar la causa exacta de cualquier caída de radio en obra.
