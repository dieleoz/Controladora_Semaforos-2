# ⏱️ MANUAL TÉCNICO DE INSTALACIÓN — RELOJ RTC DS3231 Y BATERÍA (ESTÁNDAR BALIZA)

**Sistema:** Controladora de Semáforos Móviles de 3 Estados (Maestro y Esclavo V9.0)  
**Módulo RTC:** Módulo de Alta Precisión DS3231 TCXO (Mismo estándar probado en Proyecto Baliza)  
**Propósito:** Sincronización horaria de precisión absoluta (±2 ppm) para Modo Degradado y Caja Negra  
**Verificación Hardware:** Esquemáticos KiCad `Controladora_Semaforos.kicad_sch`, `pines.h` y `MAPEO_TARJETA_KICAD.md`  
**Fecha de Emisión:** 26 de Agosto de 2026

---

## 1. ¿Por qué se instala el Módulo Externo DS3231?

En las placas con microcontroladores STM32/CKS32 clonados, los cristales internos $Y_2$ (32.768 kHz) presentan problemas de oscilación o derivas térmicas de hasta ~17 segundos cada 48 horas.

El módulo **DS3231 (mismo equipo de Baliza)** resuelve esto definitivamente:
1. **Compensación Térmica Activa (TCXO):** Deriva **menos de 0.5 segundos cada 48 horas** en cualquier clima (-40°C a +85°C).
2. **Cero Fallos de Arranque:** Se comunica por bus digital I²C en menos de 2 ms al energizar.
3. **Independencia Eléctrica:** Mantiene la fecha y hora exacta funcionando durante más de 8 años con su batería de respaldo, incluso si el semáforo está totalmente apagado en bodega.

```
┌────────────────────────────────────────────────────────────────────────┐
│                   GABINETE DEL SEMÁFORO (MAESTRO O ESCLAVO)           │
│                                                                        │
│   ┌────────────────────────┐              ┌────────────────────────┐   │
│   │ TARJETA STM32 SEMÁFORO │◄── 4 Hilos ─►│ MÓDULO RTC DS3231      │   │
│   │ (Pines PB0 / PB8)      │  (VCC/GND/   │ (Con Batería LIR2032)  │   │
│   │                        │   SDA/SCL)   │ (Estándar Baliza)      │   │
│   └────────────────────────┘              └────────────────────────┘   │
└────────────────────────────────────────────────────────────────────────┘
```

---

## 2. Diagrama de Cableado Pin a Pin en la Placa STM32

Dado que los puertos I²C por hardware nativo de la placa están ocupados (`PB6`/`PB7` para la pantalla LCD y `PB10`/`PB11` para el bus RS-485), el módulo DS3231 se conecta mediante **I²C por Software** a los **pines libres `PB0` y `PB8`**:

```text
       MÓDULO RTC DS3231 (DE BALIZA)                 TARJETA SEMÁFORO STM32
  ┌────────────────────────────────────┐         ┌───────────────────────────────┐
  │  [ VCC ]  (Alimentación 3.3V) ─────┼─────────┼──► Pin 3.3V (o 5V)            │
  │  [ GND ]  (Tierra / Masa)    ──────┼─────────┼──► Pin GND (Tierra común)     │
  │  [ SDA ]  (Datos I2C)        ──────┼─────────┼──► Pin PB0 (I2C Soft SDA)     │
  │  [ SCL ]  (Reloj I2C)        ──────┼─────────┼──► Pin PB8 (I2C Soft SCL)     │
  │  [ SQW ]  (No conectar)      ──────┼── (NC)  │                               │
  │  [ 32K ]  (No conectar)      ──────┼── (NC)  │                               │
  └────────────────────────────────────┘         └───────────────────────────────┘
```

### Tabla de Conexión Física:

| Pin Módulo DS3231 | Color Recomendado | Pin en Tarjeta STM32 | Función |
|---|---|---|---|
| **`VCC`** | 🔴 Rojo | **`3.3V`** (o `5V`) | Alimentación de la lógica del módulo |
| **`GND`** | ⚫ Negro | **`GND`** | Masa común de referencia |
| **`SDA`** | 🟢 Verde / Azul | **`PB0`** | Línea de Datos bidireccional I²C |
| **`SCL`** | 🟡 Amarillo / Blanco | **`PB8`** | Línea de Reloj de sincronización I²C |

---

## 3. ⚠️ REGLA CRÍTICA SOBRE LA PILA / BATERÍA (Evitar Daños)

Existen dos tipos de pilas de botón y no deben confundirse:

```text
  ┌─────────────────────────────────────┬─────────────────────────────────────┐
  │ TIPO 1: PILA RECARGABLE (LIR2032)   │ TIPO 2: PILA NO RECARGABLE (CR2032) │
  │ Voltaje: 3.6V - 4.2V                │ Voltaje: 3.0V                       │
  │ ✅ Apta para módulo DS3231 estándar  │ ⚠️ Requiere anular diodo de carga   │
  └─────────────────────────────────────┴─────────────────────────────────────┘
```

### ¿Cómo proceder según la pila que tenga en taller?

* **Opción A (Recomendada - Con Batería Recargable LIR2032):**
  * Colocar la batería **LIR2032 (3.6V)** en el portapilas del módulo DS3231.
  * El circuito de carga integrado del módulo mantendrá la batería cargada automáticamente mientras el semáforo esté encendido.
* **Opción B (Si solo tiene Pila Común CR2032 no recargable):**
  * Los módulos comerciales DS3231 (ZS-042) traen un diodo y una resistencia de 200 $\Omega$ que intentan cargar la pila.
  * **Acción obligatoria:** Desoldar o levantar con el cautín el diodo SMD marcado como `D1` o la resistencia `R1` del módulo DS3231. Con esto, el módulo funcionará como lector pasivo de la pila CR2032 durante 8 años sin peligro de sobrecargarla.

---

## 4. Guía de Instalación y Soldadura en Taller (Paso a Paso)

1. **Fijación Mecánica:**
   * Montar el módulo DS3231 en un separador plástico o cinta doble faz espumada dentro del gabinete del semáforo, cerca de la placa principal STM32.
2. **Soldadura / Conexión de Cables:**
   * Conectar los 4 cables flexibles (24 AWG) desde el módulo DS3231 hacia la bornera o pines de cabecera de la placa (`3.3V`, `GND`, `PB0`, `PB8`).
3. **Verificación con Multímetro (Antes de Energizar):**
   * Medir continuidad: `GND` del módulo debe tener continuidad ($0.0\ \Omega$) con el `GND` general de la placa.
   * Confirmar que no haya cortocircuito entre `3.3V` y `GND`.
4. **Verificación con Tarjeta Apagada:**
   * Con el semáforo desconectado de la fuente de 12V/24V, medir con el multímetro entre el pin positivo (+) de la pila del DS3231 y `GND`:
   * **Lectura esperada:** Entre **`3.0 V` y `3.6 V`**.

---

## 5. Protocolo de Validación en Banco de Pruebas (3 Minutos)

Una vez cableado el módulo DS3231 en la placa:

1. **Encendido:**
   * Energizar la tarjeta STM32 ➔ El LED de encendido del módulo DS3231 debe iluminar fijo.
2. **Puesta en Hora:**
   * Desde la pantalla LCD, ingresar a **`CONFIGURACION > AJUSTAR HORA`**.
   * Fijar la hora actual (ej. `18:00:00`) y presionar **Guardar / Confirmar**.
   * **Criterio de Aceptación:** La pantalla guarda exitosamente y **NO aparece la pantalla de error `CONSULTA RELOJ`**.
3. **Prueba de Corte de Energía (Memoria No Volátil):**
   * Desconectar totalmente la tarjeta de la fuente de 12V/24V durante **2 minutos**.
   * Volver a energizar la tarjeta.
   * Entrar a la pantalla de estado o consultar por Bluetooth:
   * **Criterio de Aceptación:** La hora debe marcar exactamente `18:02:00` (el reloj continuó avanzando en la oscuridad).

---

## 6. Diagnóstico de Fallas (Troubleshooting)

| Síntoma | Causa Probable | Solución |
|---|---|---|
| **La pantalla muestra `CONSULTA RELOJ: I2C No Responde`** | Cables SDA o SCL invertidos o sueltos. | Verificar que `SDA` esté en `PB0` y `SCL` esté en `PB8`. |
| **El reloj pierde la hora cada vez que se apaga el semáforo** | Pila agotada o mal colocada en el módulo. | Medir la pila con multímetro (>3.0V) y verificar que el polo positivo (+) quede hacia arriba. |
| **La hora no avanza (se queda congelada)** | Módulo DS3231 dañado o sin cristal activo. | Reemplazar el módulo DS3231 por otra unidad de repuesto de Baliza. |

---
*Manual técnico de instalación y cableado del módulo RTC DS3231 para semáforos móviles V9.0.*
