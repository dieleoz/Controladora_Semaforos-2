# ESTADO — Dónde está parado el proyecto (V9.0)

**Actualizado:** 28 de Agosto de 2026 · **HEAD:** `f7be2bd` · rama **`main-nuevo`** · remoto `origin` = `github.com/dieleoz/Controladora_Semaforos-2`  
**Repositorio NUEVO, con historia propia desde hoy.** Desciende de `Controladora_Semaforos` @ `50a5380`
(commit raíz `24276ab`), y **la historia arranca de cero porque el padre pesaba 3,47 GB** por dos ZIP de
2 GB y 1 GB que GitHub rechaza: no se podía publicar sin reescribir la historia entera, así que se
publica el árbol y se deja el anterior accesible como remoto **`padre`**.  
**Versión de Especificación y Firmware:** **V9.0 — implementada y compilando. NO probada en banco.**  
**En campo sigue la V8.4 (`e303485`).** La compuerta en verde dice que los modelos y arneses de PC no
encuentran nada; no dice que el firmware funcione sobre la tarjeta. Ver 🛑 más abajo.  
**Compuerta:** ✅ **15 PASS | 0 FALLA | 0 ABORTADO** (Exit code: 0) · Maestro: 88.3% Flash (57.880 de 65.536 B → **7.656 B libres**) · Esclavo: 64.4% Flash · Repetidor: 20.6% Flash · 405/405 en 38 packs · 271/271 pantalla · 71/71 automatico · 29/29 ciclo · app: 32 unitarios + 59 jsdom + 57 funcional · `evidencia/2026-08-28_compuerta.txt`

---

## 🛑 BLOQUEANTES DE HOY — son DOS, y ninguno cuesta firmware

**El banco sigue siendo EL bloqueante del proyecto** (Fase 6, más abajo): nada sube a campo sin pasar
banco. Estos dos son distintos: **bloquean decisiones que hay que tomar antes incluso de llegar al
banco**, y los dos se responden con el equipo en la mano.

| # | Qué está bloqueado | Qué lo desbloquea | De quién es |
|---|---|---|---|
| **BLQ-1** | 🔴 **Qué chip es el ESP32 que llegó a obra.** `ESP32-WROOM-32` / `-32D` / `-32E` / `-32U` (**clásico**) trae Bluetooth Clásico BR/EDR: hay SPP y **la app conecta tal cual, sin tocar una línea**. `ESP32-S3` y `ESP32-C3` son **solo BLE** y el socket RFCOMM **no abre nunca**; `ESP32-S2` no tiene Bluetooth. Y el apartado 1 del Manual 10 está **congelado por escrito** —*«Bluetooth Clásico SPP. No BLE. No Web Bluetooth»*—: si el módulo no es clásico, **su reapertura es obligatoria** y hay que rehacer el transporte de la app entero. Bloquea además la compra (`A1′` del Manual 15, marcada 🛑) | **Leer la serigrafía del blindaje metálico del módulo.** Treinta segundos. **El rótulo del vendedor no distingue** | **Responsable** |
| **BLQ-2** | 🔴 **El cristal `Y2`.** N-17 y N-37 lo cerraron **con medida de banco del 01/08**: no oscila en la tarjeta medida. Consecuencia hoy: `SET_RTC` **rechaza en silencio y contesta `RESULT:OK`** —`reloj.cpp:290` corta con `if (!rtcOperativo) return;` y `bluetooth.cpp:175` valida sólo el formato de la trama—. Sin pantalla, **el `$ACK` es el único canal**, y le confirma al técnico que puso una hora en un equipo donde la hora no se puede poner. El firmware ya tiene el dato: `reloj_hayCristal()` (`reloj.cpp:219`) | Que `SET_RTC` mire `reloj_hayCristal()` antes de contestar **(técnico, barato)** · y **diagnosticar el `Y2` de la SEGUNDA tarjeta** —N-37 midió uno— para decidir reparar hardware o reloj de software **(banco, `B5`)** | Técnica + **Responsable** |

---

## 🧭 MAPA RÁPIDO DE REFERENCIAS Y ARTEFACTOS (Para Auditoría de Agentes)

| Componente / Documento | Ubicación en el Repositorio | Descripción y Función |
|---|---|---|
| **App Móvil de Campo** | [`05_Funcional/App_Semaforo/`](file:///d:/@Proyect/Controladora_Semaforos%202/05_Funcional/App_Semaforo/) | Frontend Web Bluetooth / WebView con UI Baliza IOT-VIAL, Selector de Cruces Viales y Modo Courier RTC. |
| **Manual de Usuario V9.0** | [`05_Funcional/1_Manual_Usuario.md`](file:///d:/@Proyect/Controladora_Semaforos%202/05_Funcional/1_Manual_Usuario.md) / `.docx` | Ground Truth de secuencia lumínica Colombia 2024. ⚠️ ~~cámaras en `PB0` (las de `PB8` van cableadas y en reposo), mando y Bluetooth~~ — **falso desde el 28/08**: las cámaras se mudan a `J16` p10/p12, el mando y los pulsadores se retiran y el Bluetooth pasa al ESP32. **El manual todavía no está corregido** (Manual 17 §B, segundo bloque). |
| **Manual de Hardware V9.0** | [`05_Funcional/2_Manual_Hardware_y_Pruebas.md`](file:///d:/@Proyect/Controladora_Semaforos%202/05_Funcional/2_Manual_Hardware_y_Pruebas.md) / `.docx` | Ensamblaje, cableado RS485 a 2.4kbps, pila RTC en VBAT (R5 retirada) y borneras. |
| **Protocolo de Pruebas (80)** | [`05_Funcional/3_Protocolo_Pruebas_Rigurosas.md`](file:///d:/@Proyect/Controladora_Semaforos%202/05_Funcional/3_Protocolo_Pruebas_Rigurosas.md) / `.docx` | Acta de certificación funcional de **80 pruebas** —contadas, no recordadas: 80 identificadores unicos y 80 lineas `CUMPLE`— ~~(incluye cámaras en `PB0`, BT y N-53)~~. 🔴 **Con la arquitectura del 28/08, 49 de las 80 dejan de ser ejecutables** —sección a sección, Manual 17 §2.8—: sobreviven 31 en principio y **sólo 16 tal como están redactadas hoy**. Se reescribe **la última**, no la primera. |
| **Manual 4 Cámaras IA** | [`05_Funcional/9_Manual_Parametrizacion_Camara_IA.md`](file:///d:/@Proyect/Controladora_Semaforos%202/05_Funcional/9_Manual_Parametrizacion_Camara_IA.md) / `.docx` | Parametrización Hikvision AcuSense G2. ⚠️ ~~contactos `1A`/`1B` en `PB0` (Demanda, **activa**) y `PB8` (Umbral, **en reposo en V9.0**)~~ — **el `PB8` ya no es destino de cámara**: son **2 cámaras de demanda**, y el pinout se muda a `J16` p10/p12 (`PB14`/`PB15`). Manual pendiente. |
| **Manual Bluetooth Baliza** | [`05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.md`](file:///d:/@Proyect/Controladora_Semaforos%202/05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.md) / `.docx` | Telemetría `$STATUS`, Caja Negra `$ALARM`, desacoplo PA8 Hi-Z, Modo Courier RTC. ⚠️ ~~puerto `USART1`~~ *(se leía `PA9`/`PA10`)* → **`USART1` remapeado a `PB6` TX / `PB7` RX, salida por `J17` p3/p2** (N-76). Y el módulo SPP dedicado que este manual manda enchufar **lo sustituye el ESP32** — su apartado 1 sigue congelado: ver **BLQ-1**. |
| **APK Android Binaria** | [`05_Funcional/IOT_VIAL_Semaforos_v8.9.apk`](file:///d:/@Proyect/Controladora_Semaforos%202/05_Funcional/IOT_VIAL_Semaforos_v8.9.apk) | Instalable compilado con Gradle y JDK 17. **El fichero se llama `v8.9`, no `v9.0`** —esta fila apuntaba a un nombre que no existe— y es **anterior a los arreglos de N-62** en `app.js`: hay que recompilarla antes del banco (`APP-APK`). |
| **Paquete ZIP Entrega** | [`Entrega_V9.0-rc1_Firmware_Manuales_App.zip`](file:///d:/@Proyect/Controladora_Semaforos%202/Entrega_V9.0-rc1_Firmware_Manuales_App.zip) | Paquete completo: firmware de los 3 micros, **14 manuales** en `.docx` y `.md`, APK, PWA y actas. |
| **Compilador 1-Click APK** | [`05_Funcional/App_Semaforo/android/compilar_apk.bat`](file:///d:/@Proyect/Controladora_Semaforos%202/05_Funcional/App_Semaforo/android/compilar_apk.bat) | Script batch que compila la APK enlazando el JDK 17 y Android SDK portables. |
| **Esquemático KiCad BUENO** | [`01_Firmware/Controladora_Semaforos/Controladora_Semaforos/`](file:///d:/@Proyect/Controladora_Semaforos%202/01_Firmware/Controladora_Semaforos/Controladora_Semaforos/) | **649 KB con LCD, botones y el canal del motor, y el `.kicad_pcb` de 2,1 MB.** La copia incompleta que había en `03_Hardware_Tarjeta/KiCad/` (451 KB, `.kicad_pcb` vacío) **se borró el 27/08**: midiendo ahí se sacaron conclusiones falsas — ver `roadmap.md` N-64. **Este es el único plano.** |

---

## 📌 HISTORIA Y DECISIONES CLAVE DE LA VERSIÓN V9.0

### 1. ~~Sistema de 4 Cámaras IA AcuSense~~ → **2 cámaras de demanda** (Cero Raspi/Jetson externas)
* **Lo que sigue en pie:** descartar ordenadores externos. La analítica vehicular corre dentro del procesador AcuSense de las cámaras Hikvision. **Seguridad:** todo cambio de sentido respeta el tiempo de **Despeje Todo-Rojo (`cfgDespejeSeg`)**.
* ⚠️ ~~**Conexión:** Maestro: Cámara 1 (Demanda ➔ `PB0`) + Cámara 2 (Umbral ➔ `PB8`). Esclavo: Cámara 3 (`PB0`) + Cámara 4 (`PB8`)~~ — **falso desde el 28/08**. No son cuatro cámaras ni hay pin de umbral: son **dos cámaras de demanda, una por poste**, y su destino pasa a ser **`J16` p10 (`PB14`) y p12 (`PB15`)**, los pines que libera la retirada de los pulsadores y el mando.
* 🔴 **Y no se cablea todavía.** La polaridad de esos cuatro pines está en **contradicción medida**: el netlist tiene pull-**down** de 10 k con 3,3 V al lado (activo en ALTO) y `botones.cpp` los pone en `INPUT_PULLUP` y lee `== LOW`. Es **N-67 otra vez**, y se cierra con multímetro (medida **M3** del Manual 17 §A), no leyendo más código. Cablear al revés da **demanda permanente** o **demanda que nunca llega**: las dos son de calle.
* 🔴 **`J16` p1 lleva 12 V crudos** —el único conector de señal de la tarjeta que los trae, sin opto ni clamp—. Se tapa físicamente antes de cablear nada, y `p5`/`p8` se dejan vacíos a propósito como colchón (**10,2 mm** de `p1` a `p5`, **22,9 mm** a `p10`, **27,9 mm** a `p12`).
* El pack `camara_01_demanda` sigue vigilando que nadie lea `PB8` sin actualizar los manuales.

### 2. Módulo Bluetooth para Telemetría y Diagnóstico (Estándar Baliza)
* ⚠️ ~~**Decisión:** el módulo Bluetooth del proyecto Baliza en el puerto `USART1` (`PA9` TX, `PA10` RX) de ambas tarjetas.~~ **Dos cosas cambiaron y las dos están medidas:** (1) **N-76 remapeó `USART1` a `PB6` TX / `PB7` RX**, que salen por `J17` p3/p2 —`bluetooth.cpp:25` del Maestro y `:26` del Esclavo: `HardwareSerial SerialBT(PB7, PB6)`—; (2) el **módulo SPP dedicado se retira y lo sustituye el ESP32** (28/08). El puerto y el pinout son los mismos; **lo que se enchufa, no**.
* **Desacoplo Hardware U3:** `PA8` (`RS485_IN_DE_RE`) en `HIGH` permanente (pone en $\text{Hi-Z}$ la salida `RO` de `U3` para evitar choque con `TXD` del módulo Bluetooth).
* **Resolución Operativa (N-19 en Esclavo):** El técnico ya no tiene que subir con escalera a 5 metros en el Esclavo; el estado, alarmas y modo manual se operan desde el suelo con el celular.
* **Caja Negra de Alarmas:** Ante caídas de radio se emite `$ALARM,NODE:...,EVENTO:FALLO_RF_...*XX\r\n` con timestamp exacto del RTC. ⚠️ ~~SFTY-6 a los 12s~~ → **son 25 s desde N-71** (`SFTY6_SILENCIO_MS = 25000UL`, `protocolo.h:149` en las dos puntas): el techo de 12 s estaba **por debajo** del peor caso de reintentos (20,5 s) y los reintentos 4 y 5 no se ejecutaban nunca.

### 3. Resolución Definitiva de N-53 (Interferencia Mando vs. Pantalla LCD)
* **Causa Raíz:** Los relés remotos van en paralelo con los pulsadores frontales (`PB9` Botón 1 / `PB13` Botón 2). Al pulsar 3 veces rápido para subir números en `AJUSTAR HORA`, el firmware interpretaba `A·A·A` (Automático) o `B·B·B` (Ámbar) y cancelaba la edición.
* **Lo que hay en el firmware hoy (medido el 27/08, no leído):** `secuenciasInhibidas()` está en las
  dos puntas (`mando.cpp:89` Maestro, `:93` Esclavo) y el Degradado sí exige cuatro pulsos alternados
  `A·B·A·B`. **Pero Automático sigue siendo `A·A·A` y Ámbar `B·B·B`** (`mando.cpp:221-233`).
* **Lo que este apartado prometía y NO está:** la redefinición a `A·B·A` (Auto), `B·A·B` (Ámbar),
  `B·A·B·A` (Manual) y `A·A·B·B` (Inteligente). Estaba escrito como *«Solución V9.0»*, en pasado.
* **El Manual 3 sí dice la verdad** (pruebas 513 y 519: *«accionar `A` tres veces»*). Es decir que el
  documento del auditor estaba bien y **el estado interno era el que mentía** — que es peor, porque es
  el que se usa para decidir qué falta hacer. Ver la fila `FW-N53`.

### 4. 🔵 La arquitectura decidida en obra el 28/08 — el ESP32 es expansión, no controlador

**El documento completo es [`05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md`](file:///d:/@Proyect/Controladora_Semaforos%202/05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md)**
—arquitectura, ocho hallazgos MEDIDOS con su `fichero:linea`, las cinco decisiones abiertas con dueño,
las cinco medidas de multímetro y el censo de documentos que quedan falsos—. **Aquí no se copia: se
enlaza.** Lo que sigue es sólo el reparto, para que se entienda el orden de las fases.

* **El STM32 sigue siendo el controlador del semáforo.** Conserva las **8 luces** (`J3`-`J9`, `J11`), la **barrera** (`PB2`, `J15`), el **buzzer** (`PB1`, `J13`), la **radio LoRa** (`USART3`, `J12`) **y las cámaras**. La barrera de salidas de `CLAUDE.md` §6 no cambia.
* **El ESP32 es un módulo de expansión colgado de un puerto serie, y no manda sobre las luces.** Se lleva el **reloj `DS3231`** (I²C, `GPIO21` SDA / `GPIO22` SCL, con su propia pila) y el **Bluetooth**, **sustituyendo al módulo SPP dedicado**. Enlace por `J17`: `GPIO17`→`p2`=`PB7` RX, `GPIO16`←`p3`=`PB6` TX, `9600 8N1`, **masa común obligatoria**.
* **El ESP32 lleva fuente propia desde 12 V.** No cuelga de los 3,3 V de `J17`: ese riel es el mismo que alimenta al STM32 que gobierna el semáforo, y el accesorio no puede tumbar al que manda.
* **Se retiran:** la **pantalla LCD** de las dos puntas, los **cuatro pulsadores** (`PB9`, `PB13`, `PB14`, `PB15`) y el **mando de 4 relés**. Toda la operación pasa por la app.
* **Las cámaras se mudan a `J16`**: p10 (`PB14`) y p12 (`PB15`), los pines que libera esa retirada.

> 🔴 **Las dos consecuencias que fijan el orden de las fases, y no son opinión:**
> **(a)** `botonCancelar()` es hoy **la única salida de los ocho modos** —censo de llamadores en el
> Manual 17 §2.3— y por Bluetooth sólo se alcanzan **tres** de los ocho, sin `SET_MODO:MENU`. Ignorar
> los pulsadores antes de añadir los comandos convierte cada modo en **una puerta de un solo sentido**.
> **(b)** Retirar el mando **no deja tres `if` inertes: borra un veto**. `mando_ambarLocal()` tiene tres
> consumidores negados en `Esclavo/src/main.cpp` (`:401`, `:408`, `:526`); al desaparecer se vuelven
> siempre-verdaderos y una orden de radio puede sacar al Esclavo de un ámbar que un operario dejó
> puesto a propósito. Es **SFTY-21 desapareciendo por sustracción**, y hay que decidir quién hereda el
> veto **antes** de quitar `mando.cpp`.

---

## 🟡 TAREAS ABIERTAS (HOJA DE RUTA DE EJECUCIÓN)

### El orden vigente desde el 28/08 — seis fases, y el porqué de cada sitio

| Fase | Qué | Por qué va ahí |
|---|---|---|
| **1** | **Los comandos que faltan** en el Maestro: `SET_MODO:DEGRADADO`, `MENU`, `ALCANCE`, `INTELIGENTE`, `REINICIAR_RELOJ` y `DEMANDA` | 🔴 **VAN PRIMERO, antes de tocar los pulsadores.** `botonCancelar()` (`PB15`) es hoy **la única salida de todos los modos**; sin estos comandos, retirar los botones deja al operario dentro de un modo sin forma de salir salvo cortar la energía. Coste **estimado ~930 B** de los **9.276 B libres** del Maestro — se mide antes de escribir, no después (`CLAUDE.md` §7) |
| **2** | **Ignorar los pulsadores** · `FORZAR_ROJO` del Esclavo · `TEST_LEDS` | Sólo cuando la Fase 1 ha construido la salida. Aquí se decide también **quién hereda el veto de `mando_ambarLocal()`** (§4b) |
| **3** | **Cámaras a `J16`** (p10/p12) y **retirar pantalla, menú y `AiBus`** | Libera **~18,9 KB estimados** en el Maestro, que es de donde salen las fases siguientes. Las cámaras **no se cablean hasta la medida M3** (polaridad en contradicción) |
| **4** | **Telemetría honesta** | Sin pantalla, `$STATUS` es **el único tablero que existe**, y hoy trae `BAT:12.6` literal en las dos puntas —**no hay un solo `analogRead` en `src/`**—, más `RF:98%`, `RTT:85ms` y `MODO:SUBORDINADO` fijos en el Esclavo, y un campo `T:` que **no es tiempo de fase**. Un campo que no se mide se retira o se marca; no se deja con aspecto de medida |
| **5** | **ESP32: watchdog primero**, luego `DS3231` y puente Bluetooth | El watchdog va **antes** que las funciones: el ESP32 de este proyecto **no tiene ninguno** (`grep` sobre `Repetidor/src` → cero coincidencias) y hay precedente escrito de uno clavado tumbando el enlace (31/07). Con pantalla, botones y mando retirados, **un ESP32 colgado deja el equipo sin superficie de mando** |
| **6** | **BANCO** | 🛑 **Sigue siendo EL bloqueante y nada lo sustituye.** Ni la compuerta en verde, ni los arneses que compilan C++ real, ni esta hoja de ruta. Nada sube a campo sin pasar banco |

**El detalle fila a fila sigue debajo** —lo que ya está hecho, lo que sigue abierto y lo que se corrigió
el 28/08—. Las filas tachadas **no se borran**: una tachada con su motivo no se vuelve a proponer.

| Tarea | Prioridad | Descripción y Pasos a Seguir |
|---|---|---|
| ~~**FW-BT**~~ | ✅ Hecho | `bluetooth.cpp`/`.h` en ambos micros, `USART1` a 9600 bps, `PA8` en HIGH. **Sin banco.** |
| **FW-N53** | 🟠 Media | **La inhibición ya está** en las dos puntas; lo que falta es la redefinición de secuencias: hoy Auto es `A·A·A` y Ámbar `B·B·B`. Decidir si se cambian —cambia el Manual 1, el Manual 3 y el adiestramiento del operario— o si se cierra N-53 con la inhibición sola y se corrige la spec. **No dejarlo a medias otra vez** |
| ~~**FW-CAM**~~ | ✅ Hecho | Lectura con antirrebote en **`PB0`** y retransmisión por `CMD_DEMANDA` (`0x11`). **`PB8` no se lee** —ver `FW-PB8`—: esta fila decía «`PB0`/`PB8`» y contradecía a la de abajo en la misma tabla. **Sin banco.** ⚠️ **28/08:** `PB0`/`J14` es hoy **el único camino de cámara con firmware probado** (N-67 corregido, `pinMode(INPUT)` y `== HIGH` en las dos puntas, pack `camara_01_demanda`). La mudanza a `J16` es la **Fase 3** y va **después** de la medida M3 |
| **TEST-ARN** | 🟡 Baja | Sigue pendiente, pero **solo si `FW-N53` cambia las secuencias**: hoy el arnés mide las que el firmware tiene. |
| **BANCO** | 🛑 **BLOQUEANTE** | Carga física en tarjetas STM32. **Cuatro funciones nuevas no se pueden validar en PC:** ~~cámaras en `PB0`/`PB8`~~ → **cámaras en `PB0` hoy y en `PB14`/`PB15` (`J16` p12/p10) tras la Fase 3** —nadie ha cableado nunca esos pines—, `PA8` en HIGH (cambia el estado del `MAX3485 U3`), `CMD_DEMANDA` por radio real, y el Bluetooth compartiendo pista con `U3`. Nada de esto va a campo antes. |
| ~~**BANCO-PACKS**~~ | ✅ Hecho | El banco pasó de `155/155` en 20 packs a **`295/295` en 29**: cámaras, identidad, barrera en las dos puntas, el Esclavo que no abre paso y los tres `documentos_*` de N-62. **Corregido de paso lo que decía esta fila:** `costura_03` **sí** cuenta `CMD_ACK_DEMANDA` —11 comandos del Maestro—; el que no ve es **`CMD_DEMANDA`**, porque el Esclavo lo emite en `demanda.cpp:26` y el censo del pack solo mira `main.cpp`. Ver **BANCO-CENSO** |
| ~~**BANCO-CENSO**~~ | ✅ Hecho | **N-65.** Las cuatro listas escritas a mano sustituidas por `fw.fuentes_de(punta, "src")`, que censa el directorio. El Esclavo pasa de 6 a 7 comandos emitidos, con `CMD_DEMANDA` dentro, y el pack sigue en `PASS`: **el Maestro ya lo atendía** — el ciego era el instrumento, no el firmware |
| ~~**FLASH**~~ | ✅ Bajada | Maestro del **93.5 % al 85.6 %** (56 084 / 65 536 B): **9 452 bytes libres**, mas del doble que antes. No se toco una linea de firmware: `U8x8lib.cpp` referenciaba `TwoWire::setClock()` y el enlazador arrastraba `Wire` → el HAL de I2C entero —**5 160 B de flash y 352 de RAM en cada punta, por un bus que el equipo no tiene**—. Causa leida en `firmware.map` y delta medido quitando y poniendo las banderas sobre el mismo arbol. Lo vigila `flash_01_lastre`, que ademas exige quitarlas el dia que entre el `PCF8574`. **Queda margen sin gastar:** `ncenB14` y `ncenB12` se usan una vez cada una y suman 3 894 B (N-70). ⚠️ **Las cifras de esta fila son las del día de N-70 y se conservan como historia:** la vigente es la del acta del 28/08 tras las Fases 1 y 2, N-86 y N-90 — **88.3% (57.880 de 65.536 B), 7.656 B libres** |
| **LLUVIA-RF** | 🟠 Media | Reporte de campo del 27/08: *"se pasa a Modo Degradado cada nada cuando llueve"*. **Medido en el codigo y corregido (N-71):** el techo de orfandad (12 s) estaba por DEBAJO del peor caso de reintentos (20,5 s), asi que los reintentos 4 y 5 no se ejecutaban nunca; ahora el techo son 25 s. **Lo que NO esta medido es la atenuacion por lluvia**: el firmware muestra `calidadPct` en pantalla y esa lectura hay que traerla de campo antes de escribir aqui una causa. Sin ella, N-71 explica el mecanismo, no confirma el disparador. |
| ~~**PIN-0**~~ | ⛔ **ANULADA el 28/08** | ~~`PB0`/`PB8` van a **bus I²C**: `PCF8574` siempre + `DS3231` solo donde el cristal esté muerto~~ — **el I²C ya no vive en el STM32**: el `DS3231` cuelga del ESP32 (`GPIO21`/`GPIO22`) con su propia pila, así que **no hace falta sacar bus de `PB0`/`PB8`** ni modificar la tarjeta. `PB0` se queda como cámara de demanda. Queda por revisar el §4 entero del Manual 13 y `OPTIMIZACIONES.md` § SFTY-26. *La fila no se borra: un hueco se vuelve a proponer, una fila tachada con su motivo no* |
| ~~**FW-ESCLAVO-PIDE**~~ | ✅ Hecho | El Esclavo rechaza `TEST_LEDS` y atiende `SOLICITAR_PASO`, que reusa `CMD_DEMANDA` por la puerta única `demanda_solicitar()`. Pack **`esclavo_06_no_abre_paso`** (9 chk, 2 controles negativos), visto caer a `7/9` con el defecto inyectado en el `.cpp` real. **Sin banco.** |
| ~~**FW-PB8**~~ | ✅ Resuelto | **Retirado de V9.0, no implementado a medias.** El conteo del tramo necesita un comando de radio que el protocolo no tiene; leer el pin sin poder mandar la cuenta al Maestro sería medio camino. Manuales 1, 2 y 9 corregidos: ~~cámaras 2 y 4 **en reposo**~~ → **desde el 28/08 no hay cámaras 2 y 4: son dos cámaras de demanda, una por poste**, y `PB8` deja de ser destino de nada. Pack **`camara_01_demanda`** con cable trampa: el día que alguien lea `PB8`, falla y obliga a actualizar los manuales en el mismo commit |
| **FW-PAIR** | 🟠 En curso | ✅ `SERIE` de 24 bits del UID de silicio, en `$STATUS` y como contrato compartido (pack `identidad_01_serie`, 10 chk). **Falta** el byte `PAIR` en `RF_Packet`, el `SET_PAIR` y el descarte de lo ajeno — eso toca el respaldo (`DR9`), la `FIRMA` y `maestro_02_respaldo` |
| ~~**APP-SPP**~~ | ✅ Hecho | Puente nativo SPP en Capacitor (`64365ab`), spec congelada en el Manual 10 §1. **Sin probar contra un módulo fisico** — eso es `BANCO` |
| **APP-APK** | 🔴 Alta | **Recompilar la APK.** La que hay (`IOT_VIAL_Semaforos_v8.9.apk`, 26/08) es anterior a los tres arreglos de N-62 en `app.js`: el reloj en vivo mostraba `18` en vez de `18:25:00`, el selector ofrecia un PIN que el firmware rechaza siempre, y `SERIE` no se leia. `compilar_apk.bat`, y despues `--pack documentos_03` para que las tres copias sigan siendo la misma |
| **COMPRAS** | 🟠 Media | **Recortado el 27/08 tras cruzar el KiCad con `pines.h`:** las talanqueras y las dos cámaras **no necesitan expansor** —la salida `Puerta` y el pin `PB8` ya están en el cobre—, así que los `PCF8574` solo hacen falta si acaba habiendo bus. El listado consolidado —qué se pide ya, qué espera al banco y qué solo se verifica— es el **Manual 15**, que recoge lo que estaba repartido en siete manuales. Ver también **Manual 13 §0**. ⚠️ **Rehecho el 28/08 y el Manual 15 ya está corregido:** `A1` (**2 módulos SPP**) **anulada** —la sustituye `A1′`, el ESP32, hoy **🛑 bloqueada por BLQ-1**—; el `DS3231` se mueve de `B1` a **`A6`, colgado del ESP32** y ya **no espera al banco**; y aparece **`A5`, la fuente propia del ESP32 (DC-DC 12 V→5 V)**, que **no se ha pedido y hace falta** |
| ~~**SIM-ESTRES**~~ | ✅ Hecho | `simulador_app_bluetooth.py` **conectado a la compuerta** (N-62). Antes de conectarlo se le arregló la prueba 2, que contaba rechazos de PIN y **no comprobaba ninguno**: con la barrera rota a propósito imprimía «100% efectividad» y seguía en 5/5. Visto caer con 49.996 intentos colados |
| ~~**DOCS-DERIVADOS**~~ | ✅ Hecho | N-62: tres packs `documentos_*` (46 comprobaciones) vigilan lo que README, `ESTADO.md`, `OPTIMIZACIONES.md` y el Manual 10 **dicen haber medido**. Nacieron en rojo con 10 defectos reales |

---

## 🟢 CERRADO EN ESTA SESIÓN (27/08/2026) — N-62, la pasada sobre lo que los documentos dicen haber medido

- ✅ **Tres packs nuevos** (`documentos_01_cifras_del_acta`, `documentos_02_trazabilidad_sfty`,
  `documentos_03_trama_status`): **46 comprobaciones**, 6 controles negativos. Al escribirlos
  **cayeron con 10 fallos reales**, que es la única forma de saber que miden.
- ✅ **Cifras del README y de este fichero re-copiadas del acta.** Publicaban 32 rutas y 86,4 %
  de flash contra las 38 y el 92,8 % del acta que ellas mismas citaban.
- ✅ **Tabla de trazabilidad SFTY corregida**: la fila de `SFTY-2` citaba 1 pack de los 3 etiquetados.
- ✅ **Manual 10 sincronizado con el firmware**: `SERIE` documentado y **los dos checksums de
  ejemplo corregidos** (`*4F`→`*42`, `*3B`→`*43`; el apartado 4.1 explica el XOR y los ejemplos
  de debajo lo incumplían).
- ✅ **Tres defectos de la app arreglados** (`app.js`): la hora en vivo se truncaba a `18`, el
  selector ofrecía un PIN `0000` que el firmware rechaza siempre, y se leían `SITE`/`PAIR` que
  nadie emite. Las tres copias (`www/`, assets de Android) vuelven a ser la misma.
- ✅ **`simulador_app_bluetooth.py` conectado a la compuerta**, y con la prueba muerta arreglada.
- ✅ **Compuerta: 15 PASS · 0 FALLA · 0 ABORTADO** en `evidencia/2026-08-28_compuerta.txt`.

## 🟢 CERRADO EN ESTA SESIÓN (28/08/2026) — N-75, la app que no oía al equipo

Un rewrite de la app (`caa09c8`) llego con un informe que decia *"probado y subido con exito"*. La
compuerta daba **12 PASS · 1 FALLA · 2 ABORTADO** contra los 15/0/0 del dia anterior, y **los dos
ABORTADO eran la causa, no el sintoma**: los unicos dos instrumentos que ejercen la app no llegaron
a correr, y detras entraron cuatro defectos.

| lo que estaba roto | como se cerro |
|---|---|
| **La app quedo SORDA.** Perdio el `subscribe()` y el manejador de `$STATUS`/`$ALARM`/`$ERR` enteros: mandaba ordenes y pintaba un estado que se inventaba el propio telefono | Bloque devuelto **literal** de `8d75f4c` |
| **Se autorizaba sola.** `state.correctPin \|\| '1234'` inyectaba un PIN valido en todos los comandos; el modal solo cambiaba el rol de pantalla | PIN por sesion, leido del selector del modal. Unica excepcion: el rojo de emergencia, declarado en `SIN_PIN` |
| **Protocolo imaginario.** `js/nmea_parser.js` leia `FASE`, `BAT1`, `BAT2`, `TELA`… que no emite ninguna punta, con defaults que hacian pasar por sana una trama vacia | Alineado a `bluetooth.cpp:216` y `:50`, sin defaults |
| **Trabajo sin interfaz.** `SOLICITAR_PASO` (N-58), `SET_MODO:MANUAL`, `actualizarCruce()` y `eliminarCruce()` sin llamador | Reconectados con `data-cmd` y botones en la tarjeta |
| **La APK no era la APK.** `..._v9.0.apk` contenia el `app.js` de `8d75f4c` byte a byte — 493 entradas, cero CRC distintos frente a la del 27/08 | Retirada. **Compilada de verdad** y verificada por contenido |

**Y una segunda mitad, la que aparecio al probarla en un telefono con la compuerta ya en verde:**

- **La cabecera no encogia y cortaba media pantalla.** Medido a cuatro anchos: **412 px → 0 px**
  *(el de las capturas)*, 390 → 11, **360 → 41**, 320 → 81. La causa estaba tres bloques por
  encima del sintoma. Arreglado; 0 px en los cuatro.
- **Fuera el simulador del build de campo**, y con el `runLocalTicker()`, que animaba un ciclo
  completo **sin que nadie lo pulsara**. Sin enlace la pantalla ahora se congela y lo dice.
- **Nombres de cruce que rompian la cabecera** — uno de ellos venia **de fabrica**. Default
  acortado, tope de 32 y truncado por CSS, que es la garantia real.
- **Paleta medida, no elegida.** El rojo estaba en 4,9:1 y el gris atenuado en 4,0:1. Corregidos
  con un arnes que recalcula los ratios WCAG del CSS en cada corrida.

**Instrumentos: reapuntados, no aflojados.** Cada comprobacion fue a *borrar, invertir o conservar*,
y las que median de mentira se endurecieron. Vistos caer con el defecto inyectado en el `.js` real:
DOM `59 → 55`, `app_01_comandos` `8/8 → 6/8`, contraste y truncado nombrando el color y el selector.

**Entregable listo:** `05_Funcional/Paquete_App_IOT_VIAL_2026-08-28_a8e1ceb_SIN_BANCO.zip`
— APK verificada por contenido, acta, manuales y un LEEME que **no se lee como un permiso**.

> 🛑 **Nada de esto cambia el bloqueante.** Sigue siendo la sesion de banco: esto compila y
> pasa la compuerta, **no ha visto una tarjeta**.

### 🟡 Lo que esta sesion deja abierto

| # | Qué | De quién es la decisión |
|---|---|---|
| **N75-1** | **El mínimo de tiempo por sentido.** Se pidió *"mínimo de 3 minutos"*; **no está escrito en ninguna parte** — el firmware dice `VERDE_MIN_MIN = 1` y la app valida exactamente lo mismo. No hay desajuste: hay una decisión sin tomar, y su sitio es el C++ | **Responsable** — hace falta el número |
| **N75-2** | **Los cuatro límites están escritos DOS veces** — en `modo_automatico.cpp:31-33` y a mano en `app.js:734` más los `min`/`max` del formulario — **sin nada que los ate**. Hoy coinciden; el día que suba el mínimo, la app seguirá dejando poner 1. Falta un pack que lea los cuatro del `.cpp` | Técnica — media hora |
| **N75-3** | **Modo día de fondo claro.** El contraste WCAG ya está medido y en AAA salvo el rojo, pero a pleno sol el reflejo comprime los ratios y a un tema oscuro le comprime más. Es la única intervención demostrada contra el sol directo | **Responsable** — es diseño |
| **N75-4** | **`prompt()` para crear y renombrar cruces.** Diálogo nativo bloqueante; es lo que colgó una corrida E2E. Funciona, pero no es la forma | Técnica — cuando se quiera |

## 🟢 TAMBIÉN EN ESTA SESIÓN (28/08/2026) — el repositorio nuevo y la arquitectura de obra

- ✅ **Repositorio nuevo con historia propia.** Nace hoy en `24276ab`, desciende de
  `Controladora_Semaforos` @ `50a5380` y publica en `origin` = `Controladora_Semaforos-2`. **La
  historia arranca de cero porque el padre pesaba 3,47 GB** —dos ZIP, de 2 GB y 1 GB, que GitHub
  rechaza—: el anterior queda accesible como remoto **`padre`** y no se pierde nada.
- ✅ **N-76: `USART1` remapeado a `PB6`/`PB7`**, con salida por `J17` p3/p2, y los comentarios que
  seguían diciendo `PA9`/`PA10` corregidos detrás (`50a5380`).
- ✅ **La arquitectura del 28/08 escrita y medida**, en
  [`05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md`](file:///d:/@Proyect/Controladora_Semaforos%202/05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md):
  ocho hallazgos con su `fichero:linea`, cinco decisiones abiertas con dueño, cinco medidas de
  multímetro y el censo de los documentos que quedan falsos. **Este fichero lo enlaza; no lo copia.**
- ✅ **La lista de compras corregida** (`15_Lista_de_Compras_Hardware.md`): pedía `2` módulos SPP
  **el mismo día** en que se decidió sustituirlos por el ESP32.
- ⚠️ **Una discrepancia que no se tapa:** el Manual 17 cita el acta con HEAD `50a5380` *"y el árbol
  con cambios sin commitear"*. **El acta que hay hoy en `evidencia/` dice HEAD `3733544` y árbol
  LIMPIO.** Aquí se copia lo que dice el acta, que es el instrumento.

---

### 📋 EL ORDEN DE LO QUE FALTA (27/08, tras N-62 y N-63)

> ⚠️ **Este bloque es del 27/08 y queda SUBORDINADO al orden de seis fases del 28/08** (arriba, en
> TAREAS ABIERTAS). Se conserva porque su detalle sigue valiendo —sobre todo el bloque **B**, que es la
> **Fase 6**— y porque las filas que la arquitectura nueva vuelve falsas se tachan **con su motivo**, no
> se borran. Donde las dos listas discrepen, **manda la de seis fases**.

El criterio no es el gusto: **primero lo que hace productiva la sesión de banco**, porque el banco es
el bloqueante y cada sesión cuesta cargas por SWD. Después lo barato en flash. Al final lo que solo se
puede decidir con la tarjeta ya medida.

#### A · Taller — se puede hacer HOY, sin tarjeta delante

| # | Qué | Por qué va primero | Coste |
|---|---|---|---|
| ~~**A1**~~ | ✅ **Hecho el 27/08.** `IOT_VIAL_Semaforos_2026-08-27_8668498_SIN_BANCO.apk`, `BUILD SUCCESSFUL`. **Validada por contenido, no por confianza:** se descomprimió la APK y se comprobó sobre el `app.js` de dentro que lleva `indexOf(':')`, que no quedan `data.SITE` ni `data.PAIR`, que lee `data.SERIE`, y sobre el `index.html` que no queda `value="0000"` | ⚠️ **Pesa exactamente lo mismo que la vieja (3 911 388 B) y es otro fichero.** Al banco se va con el **md5** (`f6374a7b…`), nunca con el tamaño — `CLAUDE.md §4.bis` |
| ~~**A2**~~ | ✅ **Hecho el 27/08.** Talanquera dentro de `escribirPines()` en las dos puntas, cerrada al arrancar, con el pack `barrera_03_talanquera` (15 chk) y el invariante en el arnés del automático —visto caer a `68/69` con la pluma forzada a ABRIR en el `.cpp` real— | Queda `B3`: confirmar con multímetro a dónde sale `PB2` | **+24 B** por punta |
| **A3** | **Compras del bloque A del Manual 15**: ~~2 módulos SPP~~ ⛔ **ANULADO el 28/08 — ya no se compran `HC-05` ni `JDY-30`: los sustituye el ESP32** *(línea `A1′`, hoy 🛑 bloqueada por **BLQ-1**: no se compra ni un ESP32 más hasta leer la serigrafía)* · 2 cámaras de demanda *(confirmar si ya hay una en almacén)* · 2 antenas VHF con sus coaxiales · 🔴 **`A5`, la fuente propia de cada ESP32 (DC-DC 12 V→5 V, 1 A), que no se ha pedido y hace falta** | Las cámaras y las antenas no dependen de nada. ~~**`DS3231`/`PCF8574` esperan** al veredicto del cristal (N-63, `B5`)~~ → **el `DS3231` ya no espera**: pasa a `A6`, colgado del ESP32, fuera de la placa. Lo que el cristal decida importa para el **firmware** del Maestro, no para pedir el módulo | — |

#### B · Banco — 🛑 BLOQUEANTE, y en este orden

| # | Qué | Por qué en ese sitio |
|---|---|---|
| **B1** | **Carga ancla**: que arranque y ciclen las luces | La primera carga siempre es el ancla; si falla, se para y se replantea antes de gastar la sesión |
| **B2** | **La regresión N-42** del Modo Automático | Es la única regresión **abierta**. Mientras siga, nada de lo de abajo se puede dar por bueno |
| **B3** | **Multímetro:** que `Puerta` salga del pin `MOTOR_TALANQUERA` y llegue al borne | 5 minutos, y desbloquea A2 de verdad. El fuente dice *«bornera POR CONFIRMAR»* desde el primer día |
| **B4** | Cámaras en `PB0` · `PA8` en HIGH sobre el `MAX3485 U3` · `CMD_DEMANDA` por radio real · SPP contra módulo físico | Las cuatro funciones de V9.0 que **ningún PC puede validar** |
| **B5** | **Diagnóstico de los dos cristales `Y2`** *(**BLQ-2**; N-37 midió uno, el otro sigue sin diagnosticar)* | ~~Decide todo el bloque C: si el muerto es el del Esclavo, **no se compra nada**~~ ⚠️ **Ya no decide la compra:** el `DS3231` va al ESP32 (`A6`) pase lo que pase. **Sigue decidiendo el firmware**: reparar el `Y2` (cambiar `C1`/`C2` por 6-10 pF C0G/NP0) o reloj de software en el STM32 disciplinado por el ESP32 — y esa segunda vía **cuelga el reloj del semáforo del módulo accesorio**, que es justo lo que la arquitectura del 28/08 separa |

#### C · Después del banco, y según lo que diga

| # | Qué | Depende de | Coste |
|---|---|---|---|
| **C1** | ~~Umbral por conteo~~ → **SFTY-29: presencia como veto** | ~~decidido el 27/08: **van las 4 cámaras**~~ ⛔ **REVOCADO el 28/08: van DOS cámaras de demanda, una por poste.** La 2ª entrada por poste deja de estar sobre la mesa, y con ella el sujeto de SFTY-29. **Antes de reabrirlo hay que cerrar M3** (la polaridad de `J16`) y medir el flash | el bit viajaría **gratis** en el `param` de `CMD_ACK_RED`, que hoy va a 0 y nadie lee. Spec completa en `OPTIMIZACIONES.md` §SFTY-29 |
| **C2** | **Reloj `DS3231` por I²C bit-bang** | **solo si B5 dice que el cristal muerto es el del Maestro** | ~800 B + drivers, y sin pines libres si `PB0`/`PB8` se quedan con las cámaras |
| **C3** | **`FW-PAIR`** (byte `PAIR`, `SET_PAIR`, descarte de lo ajeno) | — | el más caro: toca respaldo `DR9`, la `FIRMA` y `maestro_02_respaldo` |
| **C4** | **`FW-N53`**: decidir secuencias | es **decisión de spec**, no código: cambia Manual 1, Manual 3 y el adiestramiento del operario | — |

> **Sobre las tres de C manda el flash:** ~~quedan 4.728 bytes~~ → **el acta del 28/08 mide el Maestro
> al 85.8% (56.260 de 65.536 B): quedan 9.276 B libres.** No caben todas igual. Se mide antes de
> escribir cada una, no después — es la regla de `CLAUDE.md §7`.

#### D · Higiene, cuando se quiera (no bloquea a nadie)

| # | Qué |
|---|---|
| **D1** | ~~**`main` honesto**: llevarle `CLAUDE.md` y una cabecera que diga qué es~~ → **replanteada el 28/08**: este repositorio es nuevo y la rama de trabajo es **`main-nuevo`**, con `CLAUDE.md` y la compuerta dentro desde el commit raíz. Lo que queda de la tarea es el **`main` del repositorio `padre`**, que sigue sin las reglas y cuyo firmware **no es** el de campo (240 líneas por encima del tag `V8.4`) |
| **D2** | **`BANCO-CENSO`**: `costura_03` no ve `CMD_DEMANDA` porque solo censa `main.cpp` |
| **D3** | **Campo**: Courier RTC en sitio y puesta en servicio — **solo con B pasado**, sin excepción |
