# 📜 Registro de Validación de Software

**Fecha de esta revisión:** 31 de agosto de 2026
**Alcance:** STM32F103 (Maestro / Esclavo) + ESP32 (Repetidor) + banco de simulación en Python
**Naturaleza de este documento:** registro interno de validación **en escritorio**.

> ## ⚠️ ESTE DOCUMENTO NO ES UN CERTIFICADO DE APTITUD PARA VÍA PÚBLICA, Y NO AUTORIZA A CARGAR NI A CABLEAR NADA
>
> La certificación funcional la emite el **Ingeniero Funcional / Auditor de Tránsito** tras ejecutar
> las pruebas físicas de `05_Funcional/3_Protocolo_Pruebas_Rigurosas.md` y firmar el acta.
> **A la fecha de esta revisión ese acta no está firmada, y no hay ninguna prueba de banco completa.**
>
> **Lo único que aquí se registra es lo que midieron los modelos y los arneses de PC.** Un verde de
> `01_Firmware/compuerta.py` dice exactamente eso: *que los modelos y los arneses de PC no encuentran
> nada*. **No dice que el firmware funcione en la tarjeta** (`CLAUDE.md` §3). Ya ocurrió: con la
> compuerta en verde hubo una regresión de banco en la que el Modo Automático no movía las luces.

---

## 🧾 De dónde salen las cifras de este documento

**Todas las cifras de la tabla siguiente están copiadas del acta
[`evidencia/2026-09-02_compuerta.txt`](evidencia/2026-09-02_compuerta.txt), no escritas a mano.**
Que sigan siendo las del acta más reciente lo comprueba en cada corrida el pack
`documentos_04_cifras_sin_vigilante`, que es lo que impide que este documento envejezca en silencio.

> 🔴 **Lo que ese acta dice de sí misma, y hay que leer antes de firmar nada:**
> `HEAD c509b13`, rama `main-nuevo`, y **`Arbol: CON CAMBIOS SIN COMMITEAR`**. El acta lo avisa en su
> última línea: *«estas cifras NO corresponden exactamente a `c509b13`»*. Un registro de validación
> que se firma sobre un árbol sucio no es reproducible; para que lo sea hay que volver a correr la
> compuerta con el árbol limpio.

---

## ✅ Verificado en escritorio — lo que midió la compuerta

| Verificación | Resultado medido | Método |
|---|---|---|
| Compilación Maestro (STM32F103) | **58456 B de Flash — 89.2 %** de 65536 B (quedan **7080 B**) | `pio run` |
| Compilación Esclavo (STM32F103) | **43192 B de Flash — 65.9 %** de 65536 B | `pio run` |
| Compilación Repetidor (ESP32) | **270497 B de Flash — 20,6 %** de 1310720 B | `pio run` |
| Guarda de rutas de los instrumentos | **58 rutas** parseadas, todas existen | `compuerta.py` |
| Banco de simulación funcional | **9/9 PASS** | `simulador_sistema_v7_6.py` |
| Escenarios de repetidor | **10/10 PASS** | `simulador_repetidor.py` |
| Banco por packs | **963/963 comprobaciones**, **66 packs** | `banco/correr.py` |
| Arnés de pantalla (compila el `lcd.cpp` real) | **271/271** (Maestro 145/145 · Esclavo 126/126) | `Validacion_LCD/compilar.ps1` |
| Arnés del ciclo degradado | **22/22** | `Validacion_Ciclo` |
| Arnés del Modo Automático | **71/71** | `Validacion_Automatico` |
| App — test funcional | **58/58** | suite funcional de la app |
| App — test unitarios | **32/32** | suite unitaria de la app |
| App — ejecutada en DOM | **128/128** | arnés jsdom |

> ### ⛔ Cifras que este documento publicó hasta el 31/08/2026 — ANULADAS, conservadas con su motivo
>
> No se borran: una cifra que desaparece en silencio se vuelve a escribir. Las de la izquierda
> llevaban **13 meses** sin que ningún instrumento las mirase, porque
> `documentos_01_cifras_del_acta` sólo parsea `README.md` y `ESTADO.md`. **Un `ABORTADO` grita; un
> hueco no** (`CLAUDE.md` §3).
>
> | publicaba | medido hoy | por qué importa |
> |---|---|---|
> | ~~Maestro: 42.620 B (65,0 %)~~ | **58456 B (89.2 %)** | 🔴 **El error grave.** Quien planificase con el 65 % creería tener **~23 KB libres**; quedan **7656 B**. Con esa cifra se propone estructura que **no cabe** |
> | ~~Esclavo: 15.480 B (23,6 %)~~ | **43192 B (65.9 %)** | Casi el triple de ocupación real |
> | ~~Repetidor: 269.197 B (20,5 %)~~ | **270497 B (20,6 %)** | El acta mide **una** compilación de repetidor, no dos |
> | ~~Compilación Repetidor diagnóstico: 270.257 B (20,6 %)~~ | — | ⛔ **Retirada.** La compuerta no la mide: publicar una fila sin medida detrás la hace leerse como medida |
> | ~~Banco funcional 9/9~~ | **9/9** — las 20 de entonces incluian 11 pruebas que no median nada; se retiraron con su evidencia una a una | |
> | ~~Escenarios de repetidor 8/8~~ | **10/10** | |
> | ~~Validación de pantalla 30/30~~ | **271/271** | |
> | ~~RAM: 3.576 B / 1.752 B / 21.624 B~~ | — | ⛔ **Retiradas: la compuerta NO mide RAM.** Sólo compila y lee el porcentaje de flash. La RAM se mide con `arm-none-eabi-nm` sobre el `.elf` (`CLAUDE.md` §7), y ese número no está en ningún acta |

> El banco por packs y los simuladores son **modelos en Python escritos a mano**: reimplementan lo que
> hace el C++. Un `963/963` acredita coherencia del modelo, **no** el comportamiento del firmware sobre
> hardware. Los únicos que compilan C++ real son cuatro arneses, y cada uno tiene su punto ciego
> declarado en `CLAUDE.md` §8.

## ❌ NO verificado

| Pendiente | Motivo |
|---|---|
| **Pruebas de banco físico** (Fase 4 de `ORDEN_EJECUCION.md`) | No ejecutadas. **No hay ninguna medición sobre hardware real de esta versión.** |
| **Pruebas de campo** | La última ronda se ejecutó con firmware previo y radios a 0.3 kbps. No comparable. |
| **Modo Inteligente** | El banco de simulación no lo modela. Sin cobertura de prueba. |
| **Telemetría de enlace** | Compilada y revisada, **sin prueba automática**: el arnés de pantalla le inyecta valores para dibujar, no ejecuta la lógica del coordinador. Requiere banco. |
| **Contadores de línea (SFTY-15)** | El dibujado está validado; el conteo real en `protocolo.cpp` no lo ejercita ninguna suite. Requiere banco. |
| **Repetidor ESP32** | El banco lo modela de forma aproximada; no sustituye la prueba física. |
| **RAM de las tres puntas** | Ningún instrumento de la compuerta la mide. Ver el bloque de cifras anuladas. |
| **Cableado de cámaras a `J16`** | 🔴 Bloqueado: falta la medida **M3** (polaridad). `MANUAL_USUARIO.md` §6.5. |

---

## 📋 Reglas de operación — estado de verificación

```text
[S] REG-1: Sin comunicacion -> ambos semaforos en AMARILLO INTERMITENTE (1Hz).
[S] REG-2: Menu -> ROJO FIJO con coms; AMARILLO INTERMITENTE sin coms.
[S] REG-3: Restablecimiento -> Self-Healing autonomo con 15s de All-Red.
[S] REG-4: Apagado de Esclavo -> Maestro a AMARILLO INTERMITENTE tras 25,0 s de silencio (SFTY-6).
[S] REG-5: Apagado de Maestro -> Esclavo a AMARILLO INTERMITENTE tras 25,0 s de silencio (SFTY-6).
[S] REG-6: Modo Automatico -> retorno continuo sin falsos fallos en el paso de ciclo.
[ ] REG-7: Modo Inteligente -> SIN COBERTURA en el banco de simulacion.
[S] REG-8: Modo Manual Boton 3 -> ROJO FIJO INDEFINIDO hasta pulsar Boton 1 o 2.
[ ] REG-9: Repetidor ESP32 -> requiere verificacion fisica.
[S] REG-10: SFTY-13 -> supresion de PING durante espera de ACK.

Leyenda:  [S] verificado en simulacion   [ ] sin verificar   [F] verificado en campo
Ninguna regla esta marcada [F]: no hay pruebas de campo validas para esta version.
```

> ⛔ **REG-4 y REG-5 publicaron ~~12.0s~~ hasta el 31/08/2026.** Era el umbral anterior a **N-71**;
> hoy son **25 s** (`SFTY6_SILENCIO_MS = 25000UL`, `01_Firmware/*/include/protocolo.h:149`, idéntico
> en las dos puntas). Aquel techo de 12 s quedaba **por debajo** del peor caso de reintentos del ciclo
> (~20,8 s): los reintentos 4 y 5 no podían ejecutarse jamás. Un registro de validación que publica el
> umbral viejo describe un equipo que ya no existe.

---

## 🔻 Defectos conocidos abiertos

| Ref | Defecto | Severidad |
|---|---|---|
| **N-106** | El ámbar de emergencia pedido por la app **no saca al Esclavo del Modo Degradado**, y aun así se contesta `$ACK`. `app_03_sin_ok_mudo` lo tiene en rojo a propósito, con el defecto delante | 🔴 Abierto |
| **N-3** | Operación intermitente por bajo flujo (`MANUAL_USUARIO.md §2`) no implementada | Requisito pendiente |
| **N-5** | Modo Inteligente da la cámara por viva el primer minuto tras arrancar | Menor |
| **M3** | Contradicción medida entre el netlist (pull-**down**, activo en ALTO) y `botones.cpp` (`INPUT_PULLUP`, activo en BAJO) en `J16`. **Bloquea el cableado de cámaras** | 🔴 Bloqueante para hardware |
| — | Repetidor ESP32 sin watchdog | Menor |
| — | ~2 s con las luces apagadas al encender el Maestro | A decidir con el funcional |

---

## ✍️ Responsables

```text
Fecha: 31 de agosto de 2026
Lugar: Laboratorio de Control Vial y Desarrollo de Firmware

ESTE REGISTRO NO ES UNA AUTORIZACION DE PUESTA EN SERVICIO.
Firmarlo acredita que se corrio la compuerta en escritorio; nada mas.
La puesta en servicio exige el acta funcional firmada tras banco fisico.

Ingeniero Responsable de Desarrollo
Nombre: _________________________________________________________________
Matricula profesional: ______________________  Firma: ___________________

Estado del repositorio en el momento de emision:
  Rama local:  main-nuevo
  HEAD:        c509b13   -- con cambios SIN COMMITEAR al medir (lo dice el acta)
  Remoto:      https://github.com/dieleoz/2semaforos_3estados.git
  Sincronizado con origin:  [ ] Si   [ ] No -- commits locales sin publicar: ____

Nota: la revision tecnica que origino estas correcciones fue asistida por herramientas
automatizadas. No sustituye la validacion de un ingeniero responsable ni la
certificacion funcional en campo.
```
