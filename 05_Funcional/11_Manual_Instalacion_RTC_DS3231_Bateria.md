# ⏱️ MANUAL TÉCNICO DE INSTALACIÓN — PILA DEL RELOJ RTC Y PLAN DE CONTINGENCIA DS3231 (V9.0)

**Sistema:** Controladora de Semáforos Móviles de 3 Estados (Maestro y Esclavo V9.0)  
**Hardware Principal:** RTC Interno STM32 (`PC14`/`PC15` cristal `Y2` de 32.768 kHz) + Pila CR2032 en `VBAT`  
**Plan de Contingencia:** Módulo Externo DS3231 en pines libres `PB0` (SDA) y `PB8` (SCL) por I²C Software  
**Propósito:** Sincronización horaria ininterrumpida para Modo Degradado, horario nocturno y Caja Negra  
**Verificación Hardware:** Esquemáticos KiCad `Controladora_Semaforos.kicad_sch`, `pines.h` y `MAPEO_TARJETA_KICAD.md`  
**Fecha de Emisión:** 26 de Agosto de 2026

---

## 1. Arquitectura del Reloj en la Tarjeta Madre STM32

El diseño de la tarjeta controladora **ya incluye el cristal `Y2` de 32.768 kHz** ruteado a los pines `PC14` y `PC15` del microcontrolador STM32F103C8T6.

Para mantener la hora y fecha exactas durante cortes de energía o traslados en bodega, el microcontrolador conmuta automáticamente al dominio de batería **`VBAT` (Pin 1)**.

```text
 ┌─────────────────────────────────────────────────────────────────────────────┐
 │                 ARQUITECTURA DEL RELOJ EN LA TARJETA MADRE                  │
 ├─────────────────────────────────────────────────────────────────────────────┤
 │                                                                             │
 │    CRISTAL Y2 (32.768 kHz) ──► Pines PC14 / PC15 del STM32 (Oscilador LSE)  │
 │                                                                             │
 │    PORTAPILAS CR2032 (3V)  ──► Pad VBAT (Pin 1 de U1) tras desoldar R5      │
 │                                                                             │
 │    DIAGNÓSTICO EN PANTALLA ──► Menú "CONSULTA RELOJ" (Oscilando OK / En Hora)│
 │                                                                             │
 └─────────────────────────────────────────────────────────────────────────────┘
```

---

## 2. ⚠️ ADVERTENCIA CRÍTICA: Desoldar Obligatoriamente `R5` antes de Colocar la Pila

En la PCB de fábrica, `R5` es una resistencia puente de **0 Ω** que une el pin `VBAT` con la línea principal de **3.3V**.

> ### 🛑 PELIGRO DE SOBRECALENTAMIENTO Y EXPLOSIÓN:
> Si se conecta una pila **CR2032 (no recargable)** sin retirar previamente la resistencia `R5`, la fuente de 3.3V inyectará corriente continua a la pila cuando el semáforo esté encendido.
> La pila se calentará, se hinchará y **puede reventar dentro del gabinete**.

```text
       ESTADO DE FÁBRICA (SIN PILA)                ESTADO MODIFICADO (CON PILA CR2032)
   ┌───────────────────────────────────┐       ┌───────────────────────────────────┐
   │                                   │       │                                   │
   │   3.3V ───[ R5: 0 Ω ]───► VBAT   │       │   3.3V ───[ X R5 RETIRADA X ]     │
   │                           (Pin 1) │       │                                   │
   │                                   │       │   (+) Pila CR2032 ──► Pad VBAT    │
   │                                   │       │   (-) Pila CR2032 ──► GND         │
   └───────────────────────────────────┘       └───────────────────────────────────┘
```

---

## 3. Procedimiento de Instalación Paso a Paso

### Paso 1: Comprobación con Multímetro
1. Con la tarjeta **totalmente desenergizada**, poner el multímetro en modo continuidad (pito).
2. Colocar una punta en el **Pin 1 de U1 (VBAT)** y la otra en el punto de **3.3V**.
3. Debe pitar continuo (confirmando que `R5` está presente).

### Paso 2: Retiro de `R5` (Desoldadura)
1. Con cautín a 350°C y malla desoldadora o pinzas finas, **desoldar y retirar la resistencia SMD `R5`**.
2. Volver a medir continuidad entre Pin 1 (`VBAT`) y 3.3V: **NO debe pitar**.

### Paso 3: Soldadura del Portapilas CR2032
1. **Cable Positivo (Rojo `+`):** Soldar al pad de `R5` que conecta directamente con `VBAT` (Pin 1).
2. **Cable Negativo (Negro `-`):** Soldar a cualquier punto de masa `GND` confiable:
   * Aleta metálica del regulador `U4` (LM7805).
   * Pin central `GND` del regulador `U4`.
   * Pin `GND` de las borneras RS-485.

### Paso 4: Inserción de la Batería
* Insertar una pila botón **CR2032 de 3.0V (no recargable)** de marca reconocida (Panasonic, Maxell, Sony).

---

## 4. Verificación de Funcionamiento con la Pantalla LCD

En el menú del semáforo, ingresar a **`CONFIGURACION` ➔ `CONSULTA RELOJ`** (`lcd.cpp:421`):

```text
 ┌──────────────────────────────────────┐
 │ CONSULTA RELOJ                       │
 │                                      │
 │ Estado:  Oscilando OK                │
 │ Registro: 18:25:00                   │
 │ Contador: En marcha (+1s)            │
 └──────────────────────────────────────┘
```

### Tabla de Diagnósticos Oficiales del Firmware:

| Mensaje en Pantalla LCD | Causa Técnica | Acción Requerida |
|---|---|---|
| **`Oscilando OK / En hora`** | El oscilador `Y2` y la pila operan perfectamente. | Ninguna. Sistema listo para operación. |
| **`Pedido, no oscila`** | Los condensadores de carga $C_1/C_2$ del cristal no resuenan. | Reemplazar $C_1/C_2$ por 6–10 pF o aplicar Plan B (DS3231). |
| **`Parado / Sin bateria`** | Pila agotada o $R_5$ no retirado correctamente. | Medir voltaje en Pin 1 (`VBAT` debe ser > 2.8V). |

---

## 5. Anexo Técnico: Plan B de Contingencia con Módulo Externo DS3231

Si la tarjeta posee un microcontrolador clonado cuyo oscilador interno `Y2` no lograse oscilar (`Pedido, no oscila`), se conecta un módulo **DS3231 TCXO externo** a los **dos únicos pines libres de la placa (`PB0` y `PB8`)**:

```text
       MÓDULO RTC DS3231 (EXTERNO)                   TARJETA SEMÁFORO STM32
   ┌────────────────────────────────────┐         ┌───────────────────────────────┐
   │  [ VCC ]  (Alimentación 3.3V) ─────┼─────────┼──► Pin 3.3V                   │
   │  [ GND ]  (Tierra / Masa)    ──────┼─────────┼──► Pin GND (Tierra común)     │
   │  [ SDA ]  (Datos I2C)        ──────┼─────────┼──► Pin PB0 (I2C Soft SDA)     │
   │  [ SCL ]  (Reloj I2C)        ──────┼─────────┼──► Pin PB8 (I2C Soft SCL)     │
   └────────────────────────────────────┘         └───────────────────────────────┘
```

* **Tipo de Batería en el Módulo DS3231:** Si el módulo incluye circuito de carga activo, usar **LIR2032 (recargable)** o desoldar la resistencia de carga si se usa una CR2032 estándar.

---
*Manual técnico oficial de instalación de pila RTC y contingencia DS3231 V9.0.*
