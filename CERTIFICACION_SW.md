# 📜 Registro de Validación de Software

**Fecha de esta revisión:** 8 de septiembre de 2026
**Alcance:** STM32F103 (Maestro / Esclavo) + ESP32 (Repetidor) + banco de simulación en Python
**Naturaleza de este documento:** registro interno de validación **en escritorio**.

> ## ⚠️ ESTE DOCUMENTO NO ES UN CERTIFICADO DE APTITUD PARA VÍA PÚBLICA, Y NO AUTORIZA A CARGAR NI A CABLEAR NADA
>
> La certificación funcional la emite el **Ingeniero Funcional / Auditor de Tránsito** tras ejecutar
> las pruebas físicas de `05_Funcional/3_Protocolo_Pruebas_Rigurosas.md` y firmar el acta.
> **A la fecha de esta revisión ese acta NO está firmada.** Sí hubo banco: el 3-4/09 se ejecutó la
> guía de 29 pasos sobre `617bd00` con dos tarjetas cargadas (**24 de 29 pasos**, informe en
> `evidencia/Informe_Pruebas_Banco_Semaforos_V9.0.pdf`) y el 04/09 por la noche hubo una segunda
> sesión. **Ninguna de las dos cubrió la versión que hay hoy en el árbol**, y de ellas salieron los
> defectos que la tabla de abajo lista como abiertos.
>
> **Lo único que aquí se registra es lo que midieron los modelos y los arneses de PC.** Un verde de
> `01_Firmware/compuerta.py` dice exactamente eso: *que los modelos y los arneses de PC no encuentran
> nada*. **No dice que el firmware funcione en la tarjeta** (`CLAUDE.md` §3). Ya ocurrió: con la
> compuerta en verde hubo una regresión de banco en la que el Modo Automático no movía las luces.

---

## 🧾 De dónde salen las cifras de este documento

**Todas las cifras de la tabla siguiente están copiadas del acta
[`evidencia/2026-09-11_compuerta.txt`](evidencia/2026-09-11_compuerta.txt), no escritas a mano.**
Que sigan siendo las del acta más reciente lo comprueba en cada corrida el pack
`documentos_04_cifras_sin_vigilante`, que es lo que impide que este documento envejezca en silencio.

> 🔴 **Lo que ese acta dice de sí misma, y hay que leer antes de firmar nada:**
> su `HEAD` y su rama van en su cabecera —**no se copian aquí**: aquí ponía `f27f1a0` cuando el acta
> citada decía otro— y, si trae **`Arbol: CON CAMBIOS SIN COMMITEAR`**, es porque el acta se escribe
> **antes** del commit que la publica. Un registro de
> validación que se firma sobre un árbol sucio no es reproducible; para que lo sea hay que volver a
> correr la compuerta con el árbol limpio.
>
> ⚠️ **Y este bloque ya envejeció una vez en silencio: citaba `HEAD 0b06f7d` de `main-nuevo` cuando el
> acta llevaba días midiendo otra cosa.** Ningún pack vigila estas tres líneas —`documentos_04` mira
> las **cifras**, no la cabecera—, así que **se comprueban a mano al firmar**. Es el mismo defecto que
> el nombre del fichero de arriba, que sí tiene vigilante y por eso se corrigió solo el 08/09.

---

## ✅ Verificado en escritorio — lo que midió la compuerta

| Verificación | Resultado medido | Método |
|---|---|---|
| Compilación Maestro (STM32F103) | **58400 B de Flash — 89.1 %** de 65536 B (quedan **7136 B**) | `pio run` |
| Compilación Esclavo (STM32F103) | **45280 B de Flash — 69.1 %** de 65536 B | `pio run` |
| Compilación Repetidor (ESP32) | **270497 B de Flash — 20,6 %** de 1310720 B | `pio run` |
| Guarda de rutas de los instrumentos | **64 rutas** parseadas, todas existen | `compuerta.py` |
| Banco de simulación funcional | **9/9 PASS** | `simulador_sistema_v7_6.py` |
| Escenarios de repetidor | **10/10 PASS** | `simulador_repetidor.py` |
| Banco por packs | 🔴 **1251/1258 comprobaciones**, **78 packs** — 77 PASS, **1 FALLA**. **Este documento se FIRMA, así que la frase que había aquí —*«D-14, D-20, D-21, D-22 y D-23 integradas y ancladas»*— se retira por FALSA:** sólo `D-20` y la pieza **B** de `D-21` están construidas. `D-14`, `D-22` y `D-23` son decisiones **vigentes sin construir**, y el rojo las está contando bien | `banco/correr.py` |
| Arnés de pantalla (compila el `lcd.cpp` real) | **271/271** (Maestro 145/145 · Esclavo 126/126) | `Validacion_LCD/compilar.ps1` |
| Arnés del ciclo degradado | **22/22** | `Validacion_Ciclo` |
| Arnés del Modo Automático | **99/99** | `Validacion_Automatico` |
| App — test funcional | **58/58** | suite funcional de la app |
| App — test unitarios | **32/32** | suite unitaria de la app |
| App — ejecutada en DOM | **239/239** | arnés jsdom |
| App — test unitarios TDD | **61/61** | segunda suite unitaria |
| Compilación ESP32 de expansión | **1122973 B — 35,7 %** de 3145728 B | `pio run` |
| Simulador del puente ESP32 | **101/101** | contrato del puente |
| Arnés de las dos puntas | 🔴 **76/77** (G3) | el C++ real de las dos puntas en el mismo proceso |
| Arnés del Degradado a dos puntas | **18/18** | cada punta con su reloj |

> ⛔ **En la fila del arnés que ejecuta la app en el navegador esta tabla publicó `201/201`
> hasta hoy**, y el acta que ella misma citaba medía **235**. `documentos_04` **no vigila esa
> fila** —no está en su tupla `CIFRAS_ACTA`—,
> así que la cifra envejeció en silencio, que es exactamente lo que este documento existe para
> impedir. Las seis filas de abajo se añadieron el 07/09 por el mismo motivo: **el acta las medía y
> este registro no las nombraba.**

> ### ⛔ Cifras que este documento publicó hasta el 31/08/2026 — ANULADAS, conservadas con su motivo
>
> No se borran: una cifra que desaparece en silencio se vuelve a escribir. Las de la izquierda
> llevaban **13 meses** sin que ningún instrumento las mirase, porque
> `documentos_01_cifras_del_acta` sólo parsea `README.md` y `ESTADO.md`. **Un `ABORTADO` grita; un
> hueco no** (`CLAUDE.md` §3).
>
> | publicaba | medido hoy | por qué importa |
> |---|---|---|
> | ~~Maestro: 42.620 B (65,0 %)~~ | **58400 B (89.1 %)** | 🔴 **El error grave.** Quien planificase con el 65 % creería tener **~23 KB libres**; quedan **7136 B**. Con esa cifra se propone estructura que **no cabe** |
> | ~~Esclavo: 15.480 B (23,6 %)~~ | **45280 B (69.1 %)** | Casi el triple de ocupación real |
> | ~~Repetidor: 269.197 B (20,5 %)~~ | **270497 B (20,6 %)** | El acta mide **una** compilación de repetidor, no dos |
> | ~~Compilación Repetidor diagnóstico: 270.257 B (20,6 %)~~ | — | ⛔ **Retirada.** La compuerta no la mide: publicar una fila sin medida detrás la hace leerse como medida |
> | ~~Banco funcional 9/9~~ | **9/9** — las 20 de entonces incluian 11 pruebas que no median nada; se retiraron con su evidencia una a una | |
> | ~~Escenarios de repetidor 8/8~~ | **10/10** | |
> | ~~Validación de pantalla 30/30~~ | **271/271** | |
> | ~~RAM: 3.576 B / 1.752 B / 21.624 B~~ | — | ⛔ **Retiradas: la compuerta NO mide RAM.** Sólo compila y lee el porcentaje de flash. La RAM se mide con `arm-none-eabi-nm` sobre el `.elf` (`CLAUDE.md` §7), y ese número no está en ningún acta |

> El banco por packs y los simuladores son **modelos en Python escritos a mano**: reimplementan lo que
> hace el C++. Un `1230/1230` acredita coherencia del modelo, **no** el comportamiento del firmware sobre
> hardware. Los únicos que compilan C++ real son cuatro arneses, y cada uno tiene su punto ciego
> declarado en `CLAUDE.md` §8.

## ❌ NO verificado

| Pendiente | Motivo |
|---|---|
| **Pruebas de banco físico** (Fase 4 de `ORDEN_EJECUCION.md`) | **Parciales.** El banco del 3-4/09 dejó 24 de 29 pasos verificados sobre `617bd00`; la sesión del 04/09 por la noche **cerró N-42 en cobre**; y la última cinta —05/09, 22:19, sobre `42a52cd`— confirmó N-142, N-145, N-146 y N-149. 🔴 **Nada de lo arreglado DESPUÉS de esa cinta —N-150, N-151, N-152 y los parsers de la app— ha tocado una tarjeta**, y los tres primeros tocan el camino del ámbar y del Modo Manual. |
| **Pruebas de campo** | La última ronda se ejecutó con firmware previo y radios a 0.3 kbps. No comparable. |
| **Modo Inteligente** | El banco de simulación no lo modela. Sin cobertura de prueba. |
| **Telemetría de enlace** | Compilada y revisada, **sin prueba automática**: el arnés de pantalla le inyecta valores para dibujar, no ejecuta la lógica del coordinador. Requiere banco. |
| **Contadores de línea (SFTY-15)** | El dibujado está validado; el conteo real en `protocolo.cpp` no lo ejercita ninguna suite. Requiere banco. |
| **Repetidor ESP32** | El banco lo modela de forma aproximada; no sustituye la prueba física. |
| **RAM de las tres puntas** | Ningún instrumento de la compuerta la mide. Ver el bloque de cifras anuladas. |
| **Cableado de cámaras a `J16`** | 🟢 **Desbloqueado**: `M3` cerrada en cobre el 03/09. Lo que queda es la carga verificada del firmware nuevo **antes** de que nadie enchufe nada, y tapar el pin de 12 V de `J16` p1 (N-120). |

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
[S] REG-8: Modo Manual -> ROJO FIJO INDEFINIDO hasta que la app de paso (la
          botonera ya no se monta; el comportamiento vial no cambia).
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
| ~~**N-106**~~ | ~~El ámbar de emergencia pedido por la app no saca al Esclavo del Modo Degradado, y aun así se contesta `$ACK`; `app_03_sin_ok_mudo` lo tiene en rojo a propósito~~ — 🟢 **CERRADO EN SOFTWARE**: `Esclavo/src/bluetooth.cpp` llama hoy a `degradado_salir()` a través de `salidaDegradadoIniciada()`, que **pregunta la misma guarda que ella tiene** y contesta distinto por rama (`SALIENDO_TODO_ROJO`, `SALIDA_YA_EN_CURSO`, `$ERR ... REPITA`) en vez del OK mudo. `app_03_sin_ok_mudo` da **18/18**. 🔴 **Sin prueba en tarjeta** | 🟢 Cerrado en software |
| ~~**N-145**~~ | ~~El campo `HORA:` lo rellena el STM32, que es el micro sin reloj~~ — 🟢 **CERRADO y confirmado en cobre**: la cinta del 05/09 a las 22:19 trae `HORA:22:19:58`. El reloj lo lleva el `DS3231` del ESP32 de cada punta (`D-9`, `D-15`). Queda sin verificar la dirección `0x68` sobre el módulo | 🟢 Cerrado |
| ~~**N-148**~~ | ~~La app no pide confirmación de vía al dar ámbar en Manual~~ — 🟢 **CERRADO EN SOFTWARE**: `SET_MODO:AMBAR` está en la tabla `VIA_MANIOBRA` de `app.js` y pasa por `confirmarVia()`. 🔴 **Sin prueba en tarjeta** | 🟢 Cerrado en software |
| ~~**N-149**~~ | ~~El `$STATUS` del Maestro no traía ningún campo del Esclavo~~ — 🟢 **CERRADO y confirmado en cobre**: `ESC:AMBAR` y `ESC:ROJO` viajan en todos los `$STATUS` de la cinta del 05/09 | 🟢 Cerrado |
| **BAT** | `BAT:--` en **todas** las tramas de la cinta del 04/09: la batería no se mide nunca. **Sin causa medida** | 🟠 Abierto |
| **Matriculación** | Emparejar Maestro/Esclavo **por ID de Bluetooth y sin intervención manual**. Aplazado a después del banco por decisión del responsable. `RF_Packet` son **4 bytes** `{msgID, command, param, crc}` y **no tiene campo de dirección**; el CRC cubre 3 bytes | 🟠 Aplazado |
| **N-3** | Operación intermitente por bajo flujo (`MANUAL_USUARIO.md §2`) no implementada | Requisito pendiente |
| **N-5** | Modo Inteligente da la cámara por viva el primer minuto tras arrancar | Menor |
| ~~**M3**~~ | ~~Contradicción medida entre el netlist y `botones.cpp` en `J16`; bloquea el cableado de cámaras~~ — 🟢 **CERRADA EN COBRE el 03/09** (`D-3`): pull-down real de 10 kΩ en las cuatro posiciones, `p10`/`p12` a 0 V en reposo, entrada **activa en ALTO** — que es lo que el firmware ya hacía. El paso 21 cableó `p10` contra `p11` sin demandas fantasma | 🟢 Cerrada |
| — | Repetidor ESP32 sin watchdog | Menor |
| — | ~2 s con las luces apagadas al encender el Maestro | A decidir con el funcional |

---

## ✍️ Responsables

```text
Fecha: 7 de septiembre de 2026
Lugar: Laboratorio de Control Vial y Desarrollo de Firmware

ESTE REGISTRO NO ES UNA AUTORIZACION DE PUESTA EN SERVICIO.
Firmarlo acredita que se corrio la compuerta en escritorio; nada mas.
La puesta en servicio exige el acta funcional firmada tras banco fisico.

Ingeniero Responsable de Desarrollo
Nombre: _________________________________________________________________
Matricula profesional: ______________________  Firma: ___________________

Estado del repositorio en el momento de emision:
  Rama local:  main-nuevo
  HEAD del acta: 0b06f7d -- con cambios SIN COMMITEAR al medir (lo dice el acta)
  Remoto:      https://github.com/dieleoz/Controladora_Semaforos-2.git
               (el renglon anterior daba 2semaforos_3estados.git, que es el
                remoto `padre`: manda a clonar el arbol anterior)
  Sincronizado con origin:  [ ] Si   [ ] No -- commits locales sin publicar: ____

Nota: la revision tecnica que origino estas correcciones fue asistida por herramientas
automatizadas. No sustituye la validacion de un ingeniero responsable ni la
certificacion funcional en campo.
```
