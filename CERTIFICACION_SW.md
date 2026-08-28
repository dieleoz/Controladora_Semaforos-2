# 📜 Registro de Validación de Software (V8.0)

**Fecha:** 31 de Julio de 2026
**Alcance:** STM32F103 (Maestro / Esclavo) + ESP32 (Repetidor) + banco de simulación en Python
**Naturaleza de este documento:** registro interno de validación de software.

> ## ⚠️ ESTE DOCUMENTO NO ES UN CERTIFICADO DE APTITUD PARA VÍA PÚBLICA
>
> La certificación funcional la emite el **Ingeniero Funcional / Auditor de Tránsito** tras ejecutar
> las pruebas físicas de `05_Funcional/3_Protocolo_Pruebas_Rigurosas.md` y firmar el acta.
> **A la fecha de emisión ese acta no está firmada.** Lo que aquí se registra es únicamente lo que se
> verificó en escritorio.

---

## ✅ Verificado empíricamente

| Verificación | Resultado | Método |
|---|---|---|
| Compilación Maestro (STM32F103) | SUCCESS — 42.620 B Flash (65,0%), 3.576 B RAM | `pio run` |
| Compilación Esclavo (STM32F103) | SUCCESS — 15.480 B Flash (23,6%), 1.752 B RAM | `pio run` |
| Compilación Repetidor (ESP32) | SUCCESS — 269.197 B Flash (20,5%), 21.624 B RAM | `pio run` |
| Compilación Repetidor diagnóstico | SUCCESS — 270.257 B Flash (20,6%) | `pio run -e repetidor_diag` |
| Banco de simulación funcional | **9/9 PASS** desde cualquier directorio de trabajo | `simulador_sistema_v7_6.py` |
| Escenarios de repetidor | **8/8 PASS** | `simulador_repetidor.py` |
| **Validación de pantalla en PC** | **30/30** — compila el mismo `lcd.cpp` y `menu.cpp` del firmware | `Validacion_LCD/compilar.ps1` |
| Coherencia modelo ↔ firmware | El simulador lee `RF_BURST_COPIES` y `TIMEOUT_ACK_MS` del C++ en cada ejecución | Bloque 0 del simulador |

## ❌ NO verificado

| Pendiente | Motivo |
|---|---|
| **Pruebas de banco físico** (Fase 4 de `ORDEN_EJECUCION.md`) | No ejecutadas. No hay medición sobre hardware real. |
| **Pruebas de campo** | Última ronda (31/07, 9:03) se ejecutó con firmware previo y radios a 0.3 kbps. No comparable. |
| **Eficacia real de la corrección N-1** | Requiere reconfigurar las radios a 2.4 kbps y repetir el ciclo en campo. |
| **Modo Inteligente** | El banco de simulación no lo modela. Sin cobertura de prueba. |
| **Telemetría de enlace (V8.1)** | Compilada y revisada, pero **sin prueba automática**: el arnés de pantalla le inyecta valores para dibujar, no ejecuta la lógica del coordinador. Requiere banco (checklist 5.6). |
| **Contadores de línea (V8.3, SFTY-15)** | Igual: el dibujado está validado en los tres formatos, pero el conteo real en `protocolo.cpp` no lo ejercita ninguna suite. Requiere banco. |
| **Repetidor ESP32** | El banco lo modela de forma aproximada; no sustituye la prueba física. |

> El banco de simulación es un **modelo en Python**: no compila ni ejecuta el C++. Un `9/9 PASS`
> acredita coherencia de la máquina de estados, **no** el comportamiento del firmware sobre hardware.

---

## 📋 Reglas de operación — estado de verificación

```text
[S] REG-1: Sin comunicación -> ambos semáforos en AMARILLO INTERMITENTE (1Hz).
[S] REG-2: Menú -> ROJO FIJO con coms; AMARILLO INTERMITENTE sin coms (corregido en V8.0).
[S] REG-3: Restablecimiento -> Self-Healing autónomo con 15s de All-Red.
[S] REG-4: Apagado de Esclavo -> Maestro a AMARILLO INTERMITENTE tras 12.0s.
[S] REG-5: Apagado de Maestro -> Esclavo a AMARILLO INTERMITENTE tras 12.0s.
[S] REG-6: Modo Automático -> retorno continuo sin falsos fallos en el paso de ciclo.
[ ] REG-7: Modo Inteligente -> SIN COBERTURA en el banco de simulación.
[S] REG-8: Modo Manual Botón 3 -> ROJO FIJO INDEFINIDO hasta pulsar Botón 1 o 2.
[ ] REG-9: Repetidor ESP32 -> requiere verificación física.
[S] REG-10: SFTY-13 -> supresión de PING durante espera de ACK.

Leyenda:  [S] verificado en simulación   [ ] sin verificar   [F] verificado en campo
Ninguna regla está marcada [F]: no hay pruebas de campo válidas para esta versión.
```

---

## 🔻 Defectos conocidos abiertos

| Ref | Defecto | Severidad |
|---|---|---|
| **N-1** | Requiere reconfigurar las 4 radios a 2.4 kbps. **Sin esta acción en campo, el fallo de comunicación persiste.** | Bloqueante en campo |
| **N-3** | Operación intermitente por bajo flujo (`MANUAL_USUARIO.md §2`) no implementada | Requisito pendiente |
| **N-5** | Modo Inteligente da la cámara por viva el primer minuto tras arrancar | Menor |
| **N-6** | Numeración `SFTY-x` inconsistente entre documentación y comentarios del código | Trazabilidad |
| — | Repetidor ESP32 sin watchdog | Menor |
| — | ~2 s con las luces apagadas al encender el Maestro | A decidir con el funcional |

---

## ✍️ Responsables

```text
Fecha: 31 de Julio de 2026
Lugar: Laboratorio de Control Vial y Desarrollo de Firmware

Ingeniero Responsable de Desarrollo
Nombre: _________________________________________________________________
Matrícula profesional: ______________________  Firma: ___________________

Estado del repositorio en el momento de emisión:
  Rama local:  main
  Remoto:      https://github.com/dieleoz/2semaforos_3estados.git
  Sincronizado con origin/main:  [ ] Sí   [ ] No — commits locales sin publicar: ____

Nota: la revisión técnica que originó las correcciones H-1..H-6 y N-1 fue asistida por
herramientas automatizadas. No sustituye la validación de un ingeniero responsable ni la
certificación funcional en campo.
```
