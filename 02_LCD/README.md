# 🖥️ Componente de Pantalla LCD ST7920 (128×64)

Documentación y especificación de la pantalla gráfica autónoma ST7920 conectada por SPI a
las tarjetas controladoras STM32. **Desde el 01/08/2026 hay pantalla en las dos puntas: el
Maestro y el Esclavo.**

👉 **[Ver especificación completa y mapa gráfico (`MANUAL_PANTALLA_LCD.md`)](file:///d:/@Proyect/Controladora_Semaforos/02_LCD/MANUAL_PANTALLA_LCD.md)**

## 📋 Resumen del módulo

* **Controlador:** ST7920 (128×64 píxeles), un módulo por unidad.
* **Protocolo:** SPI serie de 3 hilos (`PB3` SCLK, `PB4` CS, `PB5` SID, `PB6` PSB=LOW, `PB7` RST).
  **Mismo pinout en Maestro y Esclavo**, para poder intercambiar tarjetas en campo.
* **Librería C++:** U8g2 (`U8G2_ST7920_128X64_F_SW_SPI`).
* **Implementación:** `01_Firmware/Maestro/src/lcd.cpp` y `01_Firmware/Esclavo/src/lcd.cpp`.
* **Validación:** `01_Firmware/Validacion_LCD/` — **83/83** comprobaciones.

## 🆕 Qué cambió el 01/08/2026 (V8.7)

| | Antes | Hoy |
|---|---|---|
| Menú del Maestro | plano, 4 opciones | **dos niveles**: 4 en la raíz + 3 en `CONFIGURACION` |
| Pantallas nuevas | — | `AJUSTAR HORA`, `MODO DEGRADADO`, rechazo con motivo, ámbar con motivo |
| Esclavo | **sin interfaz** | menú de 2 opciones (`ESTADO`, `MODO DEGRADADO`) y sus vistas |
| Arnés de validación | 30/30 | **83/83** |
| Flash del Maestro | 64,2 % | **80,2 %** — ver la advertencia N-21 en el manual |

**Por qué el menú se partió en dos niveles:** una sexta opción en lista plana caía en
`y = 69`, fuera de los 64 px. Y el peligro no era que no se dibujara —hay salvaguarda—
**sino que el cursor sí podía llegar hasta ella**, dejando al operario seleccionando a
ciegas `MODO DEGRADADO`. Explicado con el caso concreto en §4.2 del manual.

## ⚠️ Limitaciones conocidas

* **Las pantallas del Esclavo no las valida el arnés.** El gancho está puesto, falta una
  línea en `compilar.ps1`.
* **Ninguna de las pantallas nuevas se ha visto sobre una ST7920 real.** Sin prueba de banco.
* Tres funciones de dibujo en `Maestro/src/lcd.cpp` son **código muerto**: nadie las llama.

## 🔗 Documentos relacionados

* **`04_Manuales/MANUAL_MANDO_4_RELES.md`** — la interfaz que se opera **sin ver esta
  pantalla**. Explica por qué el menú está estructurado como está.
* **`05_Funcional/8_Procedimiento_Modo_Degradado.md`** — el procedimiento de campo.
