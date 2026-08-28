# ESTADO — Dónde está parado el proyecto (V9.0)

**Actualizado:** 28 de Agosto de 2026 · **HEAD:** `40bb853` · rama `main` *(la rama `feat/n69-ajustes-tiempos` apunta al mismo commit)*  
**Versión de Especificación y Firmware:** **V9.0 — implementada y compilando. NO probada en banco.**  
**En campo sigue la V8.4 (`e303485`).** La compuerta en verde dice que los modelos y arneses de PC no
encuentran nada; no dice que el firmware funcione sobre la tarjeta. Ver 🛑 más abajo.  
**Compuerta:** ✅ **15 PASS | 0 FALLA | 0 ABORTADO** (Exit code: 0) · Maestro: 85.8% Flash · Esclavo: 63.9% Flash · Repetidor: 20.6% Flash · 348/348 en 34 packs · 271/271 pantalla · 71/71 automatico · app: 32 unitarios + 59 jsdom + 57 funcional · `evidencia/2026-08-28_compuerta.txt`

---

## 🧭 MAPA RÁPIDO DE REFERENCIAS Y ARTEFACTOS (Para Auditoría de Agentes)

| Componente / Documento | Ubicación en el Repositorio | Descripción y Función |
|---|---|---|
| **App Móvil de Campo** | [`05_Funcional/App_Semaforo/`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/App_Semaforo/) | Frontend Web Bluetooth / WebView con UI Baliza IOT-VIAL, Selector de Cruces Viales y Modo Courier RTC. |
| **Manual de Usuario V9.0** | [`05_Funcional/1_Manual_Usuario.md`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/1_Manual_Usuario.md) / `.docx` | Ground Truth de secuencia lumínica Colombia 2024, cámaras en `PB0` (las de `PB8` van cableadas y en reposo), mando y Bluetooth. |
| **Manual de Hardware V9.0** | [`05_Funcional/2_Manual_Hardware_y_Pruebas.md`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/2_Manual_Hardware_y_Pruebas.md) / `.docx` | Ensamblaje, cableado RS485 a 2.4kbps, pila RTC en VBAT (R5 retirada) y borneras. |
| **Protocolo de Pruebas (80)** | [`05_Funcional/3_Protocolo_Pruebas_Rigurosas.md`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/3_Protocolo_Pruebas_Rigurosas.md) / `.docx` | Acta de certificación funcional de **80 pruebas** —contadas, no recordadas: 80 identificadores unicos y 80 lineas `CUMPLE`— (incluye cámaras en `PB0`, BT y N-53). |
| **Manual 4 Cámaras IA** | [`05_Funcional/9_Manual_Parametrizacion_Camara_IA.md`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/9_Manual_Parametrizacion_Camara_IA.md) / `.docx` | Parametrización Hikvision AcuSense G2, contactos `1A`/`1B` en `PB0` (Demanda, **activa**) y `PB8` (Umbral, **en reposo en V9.0**). |
| **Manual Bluetooth Baliza** | [`05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.md`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.md) / `.docx` | Telemetría `$STATUS`, Caja Negra `$ALARM`, desacoplo PA8 Hi-Z, Modo Courier RTC y puerto `USART1`. |
| **APK Android Binaria** | [`05_Funcional/IOT_VIAL_Semaforos_v8.9.apk`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/IOT_VIAL_Semaforos_v8.9.apk) | Instalable compilado con Gradle y JDK 17. **El fichero se llama `v8.9`, no `v9.0`** —esta fila apuntaba a un nombre que no existe— y es **anterior a los arreglos de N-62** en `app.js`: hay que recompilarla antes del banco (`APP-APK`). |
| **Paquete ZIP Entrega** | [`Entrega_V9.0-rc1_Firmware_Manuales_App.zip`](file:///d:/@Proyect/Controladora_Semaforos/Entrega_V9.0-rc1_Firmware_Manuales_App.zip) | Paquete completo: firmware de los 3 micros, **14 manuales** en `.docx` y `.md`, APK, PWA y actas. |
| **Compilador 1-Click APK** | [`05_Funcional/App_Semaforo/android/compilar_apk.bat`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/App_Semaforo/android/compilar_apk.bat) | Script batch que compila la APK enlazando el JDK 17 y Android SDK portables. |
| **Esquemático KiCad BUENO** | [`01_Firmware/Controladora_Semaforos/Controladora_Semaforos/`](file:///d:/@Proyect/Controladora_Semaforos/01_Firmware/Controladora_Semaforos/Controladora_Semaforos/) | **649 KB con LCD, botones y el canal del motor, y el `.kicad_pcb` de 2,1 MB.** La copia incompleta que había en `03_Hardware_Tarjeta/KiCad/` (451 KB, `.kicad_pcb` vacío) **se borró el 27/08**: midiendo ahí se sacaron conclusiones falsas — ver `roadmap.md` N-64. **Este es el único plano.** |

---

## 📌 HISTORIA Y DECISIONES CLAVE DE LA VERSIÓN V9.0

### 1. Sistema de 4 Cámaras IA AcuSense (Cero Raspi/Jetson externas)
* **Decisión:** Descartar ordenadores externos. La analítica vehicular corre dentro del procesador AcuSense de las cámaras Hikvision.
* **Conexión:** Salidas de relé `1A`/`1B` conectadas directamente a los pines libres de la placa STM32:
  * Maestro: Cámara 1 (Demanda Cola ➔ `PB0`) **activa** + Cámara 2 (Umbral Tramo ➔ `PB8`) **en reposo**.
  * Esclavo: Cámara 3 (Demanda Cola ➔ `PB0`) **activa** + Cámara 4 (Umbral Tramo ➔ `PB8`) **en reposo**.
  * ⚠️ **El umbral se retiró de V9.0** (`FW-PB8`): el firmware declara `PB8` y **no lo lee**. Lo vigila el pack `camara_01_demanda`, que falla el día que alguien lo lea sin actualizar los manuales.
* **Seguridad:** Todo cambio de sentido respeta el tiempo de **Despeje Todo-Rojo (`cfgDespejeSeg`)**.

### 2. Módulo Bluetooth para Telemetría y Diagnóstico (Estándar Baliza)
* **Decisión:** Integrar el mismo módulo Bluetooth probado en el proyecto Baliza en el puerto `USART1` (`PA9` TX, `PA10` RX) de ambas tarjetas.
* **Desacoplo Hardware U3:** `PA8` (`RS485_IN_DE_RE`) en `HIGH` permanente (pone en $\text{Hi-Z}$ la salida `RO` de `U3` para evitar choque con `TXD` del módulo Bluetooth).
* **Resolución Operativa (N-19 en Esclavo):** El técnico ya no tiene que subir con escalera a 5 metros en el Esclavo; el estado, alarmas y modo manual se operan desde el suelo con el celular.
* **Caja Negra de Alarmas:** Ante caídas de radio (SFTY-6 a los 12s), se emite `$ALARM,NODE:...,EVENTO:FALLO_RF_12S...*XX\r\n` con timestamp exacto del RTC.

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

---

## 🟡 TAREAS ABIERTAS (HOJA DE RUTA DE EJECUCIÓN)

| Tarea | Prioridad | Descripción y Pasos a Seguir |
|---|---|---|
| ~~**FW-BT**~~ | ✅ Hecho | `bluetooth.cpp`/`.h` en ambos micros, `USART1` a 9600 bps, `PA8` en HIGH. **Sin banco.** |
| **FW-N53** | 🟠 Media | **La inhibición ya está** en las dos puntas; lo que falta es la redefinición de secuencias: hoy Auto es `A·A·A` y Ámbar `B·B·B`. Decidir si se cambian —cambia el Manual 1, el Manual 3 y el adiestramiento del operario— o si se cierra N-53 con la inhibición sola y se corrige la spec. **No dejarlo a medias otra vez** |
| ~~**FW-CAM**~~ | ✅ Hecho | Lectura con antirrebote en **`PB0`** y retransmisión por `CMD_DEMANDA` (`0x11`). **`PB8` no se lee** —ver `FW-PB8`—: esta fila decía «`PB0`/`PB8`» y contradecía a la de abajo en la misma tabla. **Sin banco.** |
| **TEST-ARN** | 🟡 Baja | Sigue pendiente, pero **solo si `FW-N53` cambia las secuencias**: hoy el arnés mide las que el firmware tiene. |
| **BANCO** | 🛑 **BLOQUEANTE** | Carga física en tarjetas STM32. **Cuatro funciones nuevas no se pueden validar en PC:** cámaras en `PB0`/`PB8` (nadie ha cableado nunca esos pines), `PA8` en HIGH (cambia el estado del `MAX3485 U3`), `CMD_DEMANDA` por radio real, y el Bluetooth compartiendo pista con `U3`. Nada de esto va a campo antes. |
| ~~**BANCO-PACKS**~~ | ✅ Hecho | El banco pasó de `155/155` en 20 packs a **`295/295` en 29**: cámaras, identidad, barrera en las dos puntas, el Esclavo que no abre paso y los tres `documentos_*` de N-62. **Corregido de paso lo que decía esta fila:** `costura_03` **sí** cuenta `CMD_ACK_DEMANDA` —11 comandos del Maestro—; el que no ve es **`CMD_DEMANDA`**, porque el Esclavo lo emite en `demanda.cpp:26` y el censo del pack solo mira `main.cpp`. Ver **BANCO-CENSO** |
| ~~**BANCO-CENSO**~~ | ✅ Hecho | **N-65.** Las cuatro listas escritas a mano sustituidas por `fw.fuentes_de(punta, "src")`, que censa el directorio. El Esclavo pasa de 6 a 7 comandos emitidos, con `CMD_DEMANDA` dentro, y el pack sigue en `PASS`: **el Maestro ya lo atendía** — el ciego era el instrumento, no el firmware |
| ~~**FLASH**~~ | ✅ Bajada | Maestro del **93.5 % al 85.6 %** (56 084 / 65 536 B): **9 452 bytes libres**, mas del doble que antes. No se toco una linea de firmware: `U8x8lib.cpp` referenciaba `TwoWire::setClock()` y el enlazador arrastraba `Wire` → el HAL de I2C entero —**5 160 B de flash y 352 de RAM en cada punta, por un bus que el equipo no tiene**—. Causa leida en `firmware.map` y delta medido quitando y poniendo las banderas sobre el mismo arbol. Lo vigila `flash_01_lastre`, que ademas exige quitarlas el dia que entre el `PCF8574`. **Queda margen sin gastar:** `ncenB14` y `ncenB12` se usan una vez cada una y suman 3 894 B (N-70). |
| **LLUVIA-RF** | 🟠 Media | Reporte de campo del 27/08: *"se pasa a Modo Degradado cada nada cuando llueve"*. **Medido en el codigo y corregido (N-71):** el techo de orfandad (12 s) estaba por DEBAJO del peor caso de reintentos (20,5 s), asi que los reintentos 4 y 5 no se ejecutaban nunca; ahora el techo son 25 s. **Lo que NO esta medido es la atenuacion por lluvia**: el firmware muestra `calidadPct` en pantalla y esa lectura hay que traerla de campo antes de escribir aqui una causa. Sin ella, N-71 explica el mecanismo, no confirma el disparador. |
| **PIN-0** | ✅ Decidido | `PB0`/`PB8` van a **bus I²C**: `PCF8574` siempre + `DS3231` solo donde el cristal esté muerto. Un firmware detecta cuál hay y **lo anuncia**. Ver `OPTIMIZACIONES.md` § SFTY-26 y Manual 13 |
| ~~**FW-ESCLAVO-PIDE**~~ | ✅ Hecho | El Esclavo rechaza `TEST_LEDS` y atiende `SOLICITAR_PASO`, que reusa `CMD_DEMANDA` por la puerta única `demanda_solicitar()`. Pack **`esclavo_06_no_abre_paso`** (9 chk, 2 controles negativos), visto caer a `7/9` con el defecto inyectado en el `.cpp` real. **Sin banco.** |
| ~~**FW-PB8**~~ | ✅ Resuelto | **Retirado de V9.0, no implementado a medias.** El conteo del tramo necesita un comando de radio que el protocolo no tiene; leer el pin sin poder mandar la cuenta al Maestro sería medio camino. Manuales 1, 2 y 9 corregidos: cámaras 2 y 4 **en reposo**. Pack **`camara_01_demanda`** con cable trampa: el día que alguien lea `PB8`, falla y obliga a actualizar los manuales en el mismo commit |
| **FW-PAIR** | 🟠 En curso | ✅ `SERIE` de 24 bits del UID de silicio, en `$STATUS` y como contrato compartido (pack `identidad_01_serie`, 10 chk). **Falta** el byte `PAIR` en `RF_Packet`, el `SET_PAIR` y el descarte de lo ajeno — eso toca el respaldo (`DR9`), la `FIRMA` y `maestro_02_respaldo` |
| ~~**APP-SPP**~~ | ✅ Hecho | Puente nativo SPP en Capacitor (`64365ab`), spec congelada en el Manual 10 §1. **Sin probar contra un módulo fisico** — eso es `BANCO` |
| **APP-APK** | 🔴 Alta | **Recompilar la APK.** La que hay (`IOT_VIAL_Semaforos_v8.9.apk`, 26/08) es anterior a los tres arreglos de N-62 en `app.js`: el reloj en vivo mostraba `18` en vez de `18:25:00`, el selector ofrecia un PIN que el firmware rechaza siempre, y `SERIE` no se leia. `compilar_apk.bat`, y despues `--pack documentos_03` para que las tres copias sigan siendo la misma |
| **COMPRAS** | 🟠 Media | **Recortado el 27/08 tras cruzar el KiCad con `pines.h`:** las talanqueras y las dos cámaras **no necesitan expansor** —la salida `Puerta` y el pin `PB8` ya están en el cobre—, así que los `PCF8574` solo hacen falta si acaba habiendo bus. El listado consolidado —qué se pide ya, qué espera al banco y qué solo se verifica— es el **Manual 15**, que recoge lo que estaba repartido en siete manuales. Ver también **Manual 13 §0** |
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

---

### 📋 EL ORDEN DE LO QUE FALTA (27/08, tras N-62 y N-63)

El criterio no es el gusto: **primero lo que hace productiva la sesión de banco**, porque el banco es
el bloqueante y cada sesión cuesta cargas por SWD. Después lo barato en flash. Al final lo que solo se
puede decidir con la tarjeta ya medida.

#### A · Taller — se puede hacer HOY, sin tarjeta delante

| # | Qué | Por qué va primero | Coste |
|---|---|---|---|
| ~~**A1**~~ | ✅ **Hecho el 27/08.** `IOT_VIAL_Semaforos_2026-08-27_8668498_SIN_BANCO.apk`, `BUILD SUCCESSFUL`. **Validada por contenido, no por confianza:** se descomprimió la APK y se comprobó sobre el `app.js` de dentro que lleva `indexOf(':')`, que no quedan `data.SITE` ni `data.PAIR`, que lee `data.SERIE`, y sobre el `index.html` que no queda `value="0000"` | ⚠️ **Pesa exactamente lo mismo que la vieja (3 911 388 B) y es otro fichero.** Al banco se va con el **md5** (`f6374a7b…`), nunca con el tamaño — `CLAUDE.md §4.bis` |
| ~~**A2**~~ | ✅ **Hecho el 27/08.** Talanquera dentro de `escribirPines()` en las dos puntas, cerrada al arrancar, con el pack `barrera_03_talanquera` (15 chk) y el invariante en el arnés del automático —visto caer a `68/69` con la pluma forzada a ABRIR en el `.cpp` real— | Queda `B3`: confirmar con multímetro a dónde sale `PB2` | **+24 B** por punta |
| **A3** | **Compras del bloque A del Manual 15**: 2 módulos SPP · 2 cámaras de demanda *(confirmar si ya hay una en almacén)* · 2 antenas VHF con sus coaxiales | Es lo único que no depende de nada. **`DS3231`/`PCF8574` esperan** al veredicto del cristal (N-63, `B5`) | — |

#### B · Banco — 🛑 BLOQUEANTE, y en este orden

| # | Qué | Por qué en ese sitio |
|---|---|---|
| **B1** | **Carga ancla**: que arranque y ciclen las luces | La primera carga siempre es el ancla; si falla, se para y se replantea antes de gastar la sesión |
| **B2** | **La regresión N-42** del Modo Automático | Es la única regresión **abierta**. Mientras siga, nada de lo de abajo se puede dar por bueno |
| **B3** | **Multímetro:** que `Puerta` salga del pin `MOTOR_TALANQUERA` y llegue al borne | 5 minutos, y desbloquea A2 de verdad. El fuente dice *«bornera POR CONFIRMAR»* desde el primer día |
| **B4** | Cámaras en `PB0` · `PA8` en HIGH sobre el `MAX3485 U3` · `CMD_DEMANDA` por radio real · SPP contra módulo físico | Las cuatro funciones de V9.0 que **ningún PC puede validar** |
| **B5** | **Diagnóstico de los dos cristales `Y2`** | Decide todo el bloque C: si el muerto es el del Esclavo, **no se compra nada** |

#### C · Después del banco, y según lo que diga

| # | Qué | Depende de | Coste |
|---|---|---|---|
| **C1** | ~~Umbral por conteo~~ → **SFTY-29: presencia como veto** | decidido el 27/08: **van las 4 cámaras**. Falta la 2ª entrada por poste —la decide la pregunta 5 del banco— y medir el flash | el bit viaja **gratis** en el `param` de `CMD_ACK_RED`, que hoy va a 0 y nadie lee. Spec completa en `OPTIMIZACIONES.md` §SFTY-29 |
| **C2** | **Reloj `DS3231` por I²C bit-bang** | **solo si B5 dice que el cristal muerto es el del Maestro** | ~800 B + drivers, y sin pines libres si `PB0`/`PB8` se quedan con las cámaras |
| **C3** | **`FW-PAIR`** (byte `PAIR`, `SET_PAIR`, descarte de lo ajeno) | — | el más caro: toca respaldo `DR9`, la `FIRMA` y `maestro_02_respaldo` |
| **C4** | **`FW-N53`**: decidir secuencias | es **decisión de spec**, no código: cambia Manual 1, Manual 3 y el adiestramiento del operario | — |

> **Sobre las tres de C manda el flash: quedan 4.728 bytes.** No caben todas. Se mide antes de
> escribir cada una, no después — es la regla de `CLAUDE.md §7` y hoy el margen es del 7 %.

#### D · Higiene, cuando se quiera (no bloquea a nadie)

| # | Qué |
|---|---|
| **D1** | **`main` honesto**: llevarle `CLAUDE.md` y una cabecera que diga qué es —hoy no tiene ni las reglas ni la compuerta, y su firmware **no es** el de campo (240 líneas por encima del tag `V8.4`) |
| **D2** | **`BANCO-CENSO`**: `costura_03` no ve `CMD_DEMANDA` porque solo censa `main.cpp` |
| **D3** | **Campo**: Courier RTC en sitio y puesta en servicio — **solo con B pasado**, sin excepción |
