# INDICE_CRUZADO — dónde vive cada hecho, y quién se queda colgando si lo tocas

**Escrito el 31/08/2026 · revisado y corregido el 02/09/2026.** Complementa a
[`ARQUITECTURA.map`](ARQUITECTURA.map), que dice *cómo está hecho el equipo*. Este fichero dice
**dónde está escrito cada hecho y quién apunta a él**.

> **Qué cambió en la pasada del 02/09**, para que se lea sin releerlo entero. El árbol se movió mucho
> —el banco pasó de 39 a **66 packs** y la compuerta de 15 a **20** comprobaciones—, y esta revisión
> midió las dos direcciones, no sólo lo que faltaba:
>
> | | |
> |---|---|
> | **Dejaron de ser `SOLO PROSA`** | el margen del Degradado contra la deriva (`costura_12` + el arnés a dos puntas) · el registro de silencio de `J17` (`enlace_02`) · el parte de arranque del puente (`esp32_10`) · *«bien formada no es cierta»* del reloj (`esp32_11`) · el diagnóstico del reloj, que era un **N-73 en marcha** (`reloj_01`) |
> | 🔴 **Decían `VIGILADO` y no era exacto** | el censo de `# EJERCE SFTY-x` (`P-3.ter`: faltaban dos etiquetas **desde antes de escribirlo**) · el PIN, que tiene **medio** vigilante y no ninguno (`P-7`) |
> | ✅ **Estaban abiertos y ya no** | `N-106` (el pack pasó de rojo-a-propósito a **8/8**) · `SFTY-3`/`SFTY-7` (`P-3.bis`) |
> | 🔴 **Lo nuevo, y es lo que buscaba el encargo** | **dos instrumentos existen y no están en la compuerta** — §1.10.bis y `P-10` |

> 🔴 **Por qué existe.** El 31/08 se reescribió la guía de cableado para el técnico y, al sacar lo
> que a él no le servía, **desaparecieron del repositorio entero** la especificación de los puntos de
> prueba de la placa portadora, el requisito de acceso de reflasheo y **«lo que esta placa NO
> lleva»** —una barrera de arquitectura—. Vivían en un solo fichero y **nadie apuntaba a ellas**.
> Es `CLAUDE.md` §3 aplicado a la documentación: **un `ABORTADO` grita; un hueco no.**
>
> Este índice no impide que vuelva a pasar por sí solo. Lo que hace es que, **antes** de reescribir
> un fichero, se pueda leer en treinta segundos qué hechos son los únicos que ese fichero sostiene.

---

## 0. Cómo se lee, y qué NO es

**Este fichero no es un instrumento.** No mide nada, nadie falla por lo que aquí ponga, y ningún pack
lo parsea. **Re-medido el 02/09, y la cifra del 31/08 había que corregirla:**

| medida | 31/08 | 02/09 [MEDIDO] |
|---|---|---|
| `grep -rn "INDICE_CRUZADO"` sobre **`.py`** | cero | **cero** — sigue siendo cierto, y es lo que importa: **ningún pack lo parsea** |
| el mismo sobre **`.md`** | *«cero»* | **tres** — `OPTIMIZACIONES.md:445` y `:1634`, y `roadmap.md:328` |
| control positivo, `ARQUITECTURA.map` | tres | **cuatro** documentos lo nombran, y **cinco ficheros** lo contienen |

> ⚠️ **La frase del 31/08 mezclaba dos cosas en un solo `grep`, y por eso envejeció mal.** *«Ningún
> pack lo parsea»* y *«nadie lo enlaza»* son propiedades distintas: la primera sigue siendo verdad y
> **es la que sostiene el párrafo**; la segunda ya no, y era la que se movía. Se separan aquí para
> que la próxima pasada no tenga que decidir cuál de las dos se estaba midiendo.
>
> **Y el control positivo necesitaba a su vez un control**, que no tenía: `grep "ARQUITECTURA"` sobre
> los `.py` del árbol da **5**, y ninguna es una lectura del mapa. La trampa está en la extensión —
> `flash_01_lastre.py` **sí** parsea un `.map`, pero es `firmware.map`, el del enlazador. Buscar por
> extensión habría dado un falso positivo y con él la conclusión contraria.

**Lo que mide es la compuerta.**

Tres niveles, la misma escala de `ARQUITECTURA.map` y del Manual 17 §0, y no se mezclan:

| marca | significa |
|---|---|
| **[MEDIDO]** | se abrió el fichero en esta pasada y se leyó, o lo dijo un `grep`. Va con `fichero:linea` |
| **[LEÍDO]** | lo afirma un documento y aquí se recoge **sin volver a comprobarlo en el fuente** |
| **[SIN VERIFICAR]** | nadie lo ha comprobado nunca. Casi todo el cobre está aquí |

> ⚠️ **El árbol se estaba moviendo mientras esto se escribía**, y se movió de verdad: entre la
> primera medida y la última entraron seis commits de otros agentes (`e8caef9` … `88b0142`), que
> traen los generadores de trama, cinco manuales, N-108 y el cambio de la pantalla. **Las líneas
> citadas de `app.js`, `lcd.cpp`, `OPTIMIZACIONES.md`, los manuales 2/12/14 y los packs pueden
> haberse desplazado.** Lo que no se mueve es *en qué fichero* vive cada cosa, que es lo que este
> índice contesta.

**Aquí no se publican cifras del acta** —packs, comprobaciones, porcentajes de flash—: se mueven cada
hora y hay packs que las vigilan. La cifra vigente está en **`evidencia/`**, en el acta más reciente.

### Las dos preguntas que este fichero contesta

| pregunta | dónde se contesta |
|---|---|
| **«Quiero saber X: ¿dónde está?»** | **§1 — mapa por tema** |
| **«Voy a reescribir el fichero Y: ¿qué se pierde y quién se queda colgando?»** | **§2 — mapa por fichero** *(y es la que hoy no existía)* |
| **«¿Qué está a un descuido de desaparecer?»** | **§3 — censo de hechos únicos** |
| **«¿Qué se perdió ya?»** | **§4 — huecos abiertos** |

### La columna que decide, y es el juicio de todo el fichero

Un hecho que vive en **un solo documento** no es automáticamente un problema. Lo que lo vuelve
peligroso es que **nada lo relea**:

| | |
|---|---|
| **VIGILADO** | un pack o un arnés **relee el número o el patrón del fuente en cada corrida**. El documento puede envejecer o borrarse: la compuerta se pone roja. **No hace falta duplicarlo** |
| **SOLO PROSA** | vive únicamente como texto. Si el fichero se reescribe, **nadie protesta**. Aquí es donde hay que decidir |

Duplicar un hecho VIGILADO crearía **dos versiones que alguien tendría que sincronizar a mano**, que
es exactamente el defecto que este repositorio lleva un mes cerrando (N-36, N-39, `PESOS_SUMA`,
`cfgVerdeRecibido`). **No se duplica todo.**

---

## 1. MAPA POR TEMA — «quiero saber X, ¿dónde está?»

### 1.1 El reparto STM32 / ESP32

| hecho | dueño del hecho | quién más lo dice | quién lo VIGILA |
|---|---|---|---|
| **El STM32 sigue siendo el controlador; el ESP32 es un accesorio colgado de un puerto serie y NO manda sobre las luces** | `05_Funcional/17_...Decisiones_Abiertas.md` §1.1–1.2 | `ESTADO.md` §4 · `05_Funcional/18_Especificacion_Firmware_ESP32.md` §1.2 · `05_Funcional/5_Manual_Puente_ESP32.md` · `ARQUITECTURA.map` C4 · 7 documentos [MEDIDO] | `esp32_05_no_origina` · `esp32_08_silencio_no_es_orden` |
| **El ESP32 lleva fuente propia desde 12 V y no cuelga del 3,3 V de `J17`** | Manual 17 §1.5 | `05_Funcional/10_...Telemetria.md` §1 · `05_Funcional/15_Lista_de_Compras_Hardware.md` línea `A5` · 6 documentos [MEDIDO] | — **SOLO PROSA** |
| **El firmware del ESP32 EXISTE y compila** | `01_Firmware/ESP32_Expansion/` (`src/` + `include/`, 8 módulos) [MEDIDO] | `05_Funcional/18_...ESP32.md` (la especificación completa) | `compuerta.py` lo compila como rol `ESP32_Expansion` (`compuerta.py:114`, [RE-MEDIDO 02/09]: `_ROLES = ("Maestro", "Esclavo", "Repetidor", "ESP32_Expansion")`) y los packs **`esp32_01`…`esp32_11`** *(eran nueve el 31/08)* |
| **`ESP32_Expansion` es un ROL, distinto del `Repetidor`** | `01_Firmware/compuerta.py:90-118` [MEDIDO] | `05_Funcional/18_...ESP32.md` §7.1 | **la propia guarda de rutas**: un rol que no se declara no se censa |
| **`BLQ-1` cerrado: es un `ESP32-WROOM-32` clásico, `BR/EDR` → hay SPP** | `ESTADO.md` tabla de bloqueantes | `roadmap.md` · `05_Funcional/18_...ESP32.md` §6.1 · `15_Lista_de_Compras...` · 6 documentos [MEDIDO] | — **SOLO PROSA** |

### 1.2 El enlace `J17`, pin a pin

| hecho | dónde | vigilado |
|---|---|---|
| **`J17` reparte UN conector entre dos cosas**: LCD `PB3`/`PB4`/`PB5` → p4/p1/p5, y `USART1` remapeado `PB6` TX / `PB7` RX → p3/p2 | `03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md:349-350` **[MEDIDO — es la fuente]** · §6.bis del mismo | `costura_11_lcd_sin_bus`, `flash_01_lastre` y `enlace_01_transporte` — los tres leen el constructor de `lcd.cpp` **por texto** |
| **La pareja del ESP32**: `GPIO17` (TX2) → `J17` p2 → `PB7` RX del micro · `GPIO16` (RX2) ← `J17` p3 ← `PB6` TX | `01_Firmware/ESP32_Expansion/include/contrato.h` (`ENLACE_PIN_TX`/`ENLACE_PIN_RX`) **[MEDIDO]** | `esp32_09_contrato_de_bytes` |
| **`9600 8N1`, y el formato se elige en el ESP32 porque en el STM32 no se eligió** (`SerialBT.begin(9600)` con un solo argumento) | `contrato.h` §transporte **[MEDIDO]** · gemelo en `Maestro/src/bluetooth.cpp:70` y `Esclavo/src/bluetooth.cpp:78` | `esp32_09_contrato_de_bytes` |
| **Masa común obligatoria** | Manual 17 §1.4 · 10 documentos [MEDIDO] | — **SOLO PROSA** *(pero es medida `M5`)* |
| **El nombre del pin 3 sigue en disputa**: `RS(A0)` en el esquemático contra `LCD_PSB` en `pines.h:88` | `Maestro/include/pines.h:78` y `:88` **[MEDIDO]** · `18_...ESP32.md` §2.3 (`AB-6`) | — **SOLO PROSA** |

### 1.3 El mapa de pines

**El censo completo, pin a pin, está en `ARQUITECTURA.map` Anexo A.** Aquí solo dónde vive cada dato:

| hecho | dónde vive | vigilado |
|---|---|---|
| **La tabla de `#define`** | `Maestro/include/pines.h` y `Esclavo/include/pines.h`, **byte-idénticos** (md5 `d5d0be19911e0ec9b19ed6e55f799d08`, 31/08) **[MEDIDO]** | `barrera_02_dos_puntas` · `camara_02_j16` |
| **La verificación cruzada PCB ↔ firmware** | `03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md` §6 | — **SOLO PROSA** *(es el único documento de cobre)* |
| **`PB14`/`PB15` son hoy `CAM_C_PIN`/`CAM_D_PIN`**, `INPUT` pelado y activo en ALTO | `pines.h:124-125` · `botones.cpp:145-157` (Maestro) y `:176-177` (Esclavo) **[MEDIDO]** | `camara_02_j16` |
| **`PB9`/`PB13` siguen siendo `BOTON1`/`BOTON2` = mando A/B**, `INPUT_PULLUP`, activo en BAJO | `pines.h:122-123` · `botones.cpp:139-140` **[MEDIDO]** | `camara_02_j16` (etiquetado `EJERCE SFTY-21`) |
| **`ROJO_PEATON` `PA6`, `VERDE_PEATON` `PA7` y `BUZZER` `PB1` están DECLARADOS y MUERTOS** | `CLAUDE.md` §6 (recuadro N-96) · `OPTIMIZACIONES.md:1427` · `ARQUITECTURA.map` C8 | `barrera_01_pines_de_luz` los cubre **como sujetos de la regla, no como pines vivos** — ver el matiz de N-96 |
| **`PB8` = `LED_TESTIGO`, `pinMode(INPUT)` sin lectura, DELIBERADO** | `pines.h:48-63` **[MEDIDO]** | `camara_01_demanda`, con cable trampa |
| **Pines libres sin cablear: `PA11`, `PA12`, `PA15`, `PC13`** | `pines.h:60-61` **[LEÍDO — no comprobado contra el netlist]** | — |

### 1.4 `J16`, las cámaras y la decisión del mando

| hecho | dónde | vigilado |
|---|---|---|
| **El mando de relés SE CONSERVA en A y B** (decisión del responsable, 31/08) | `ESTADO.md` §4 · Manual 17 §1.6 y §3.3 | `camara_02_j16` (inyección: «el `#define BOTON3` reapareciendo») |
| **Se retiran solo los pulsadores 3 y 4**; `botonAceptar()`/`botonCancelar()` quedan **sin sujeto, devolviendo `false` fijo** — no borradas, para que un `grep` siga listando en un sitio todo lo que la retirada se llevó | `Maestro/src/botones.cpp:250-253` y `:280-281` **[MEDIDO]** | `camara_02_j16` |
| **🔴 `J16` p1 lleva 12 V crudos** — el único conector de señal de la tarjeta que los trae | Manual 17 §2.1 · 11 documentos [MEDIDO] | — **SOLO PROSA** |
| **🔴 El margen real contra esos 12 V es de `1,36 mm`, medido sobre cobre** —y el orden se invierte: `p12` es el peor, no el mejor | `MAPEO_TARJETA_KICAD.md:576-588` **(la fuente)** · `ESTADO.md` §1 · 7 documentos | — **SOLO PROSA** |
| **🔴 `M3`: la polaridad de esos pines está en contradicción medida** — netlist pull-**down** (activo ALTO) contra `botones.cpp` `INPUT_PULLUP` + `== LOW` | Manual 17 §2.2 y §A `M3` **(la fuente)** · `CLAUDE.md` §9.bis · 15 documentos | — **SOLO PROSA**, y es **bloqueante de cableado** |
| **El orden es ASIMÉTRICO: firmware cargado ANTES de que nadie enchufe nada** | `CLAUDE.md` §9.bis | — **SOLO PROSA**, y no lo puede vigilar ningún pack: es un procedimiento |

### 1.5 Las reglas `SFTY-x` y su trazabilidad

| hecho | dónde |
|---|---|
| **El catálogo de reglas `SFTY-1` … `SFTY-29`** | `OPTIMIZACIONES.md` — **es el documento propietario y no hay otro** |
| **La trazabilidad regla → código → prueba** | se **levanta buscando** la etiqueta `# EJERCE SFTY-x:` en la cabecera de cada pack (`CLAUDE.md` Convenciones) |
| **Los packs etiquetados hoy** | `grep -rn "EJERCE SFTY" 01_Firmware/Simulaciones/banco/packs/` **[RE-MEDIDO 02/09]** — ver el recuadro de abajo, porque **la lista del 31/08 estaba incompleta el día que se publicó**: <br>**SFTY-2** `barrera_01`, `barrera_02`, **`esclavo_06_no_abre_paso`**, **`maestro_09_test_leds`** · **SFTY-28** `barrera_03`, `maestro_09` · **SFTY-21** `camara_02_j16`, `costura_02`, `costura_06`, **`costura_12_margen_deriva`**, `esclavo_01`, `esclavo_02`, `esclavo_07`, `esclavo_08`, `maestro_01`, `maestro_05` · **SFTY-6** `costura_08`, `costura_09`, `maestro_04` · **SFTY-23** `esclavo_03`, `esclavo_04`, `esclavo_05`, `maestro_04` · **SFTY-18** `esp32_04_osf`, **`esp32_11_bien_formada_no_es_cierta`** |
| **Los que NO llevan etiqueta a propósito** | `enlace_01_transporte` (ninguna de las 29 habla del transporte — `AB-8`) y, desde el 01/09, **`barrera_04_arnes_dos_puntas`**: su cabecera `:39-42` razona por qué *«EJERCE SFTY-2» sería falso* — vigila al **instrumento** que ejerce el enclavamiento, no al enclavamiento. Es la Convención de `CLAUDE.md` bien aplicada: **una fila vacía no miente; una etiqueta de más, sí** |
| **Quién vigila que la tabla no mienta** | `documentos_02_trazabilidad_sfty` |

> ### 🔴 02/09 — LA LISTA DE ETIQUETAS DEL 31/08 ESTABA MAL, Y NO POR HABER ENVEJECIDO
>
> Faltaban dos, y **las dos estaban ahí el 31/08**. No es deriva del árbol: es que el censo no las
> vio. Fechado con `git log -S` sobre el texto literal de cada etiqueta:
>
> | pack | etiqueta que faltaba | desde |
> |---|---|---|
> | **`esclavo_06_no_abre_paso.py:36`** | *«EJERCE SFTY-2: ningún camino del Bluetooth del Esclavo puede producir un verde»* — **el pack no aparecía en la lista en absoluto** | `24276ab`, **28/08** |
> | **`maestro_09_test_leds.py:53`** | *«EJERCE SFTY-2: ningún verde llega a los pines sin pasar por el enclavamiento»* — el pack **sí** estaba listado, pero **sólo por su segunda etiqueta**, `SFTY-28` | `caef8a1`, **28/08** |
>
> **Los dos casos son el mismo error con dos caras, y ninguna es «el árbol se movió»:** uno se cayó
> entero de la lista, y el otro entró con **una** de sus dos etiquetas porque **un pack puede llevar
> más de una y el censo se quedó con la primera**. Un `grep -rl` cuenta ficheros; la propiedad que
> hace falta aquí es *«cuántas reglas ejerce cada pack»*, que es `grep -rn` y **contar líneas, no
> ficheros**.
>
> Es exactamente el error que este mismo documento diagnostica dos recuadros más abajo para el «ocho»
> de `SFTY-27` —*«`8` es el número de FICHEROS de firmware que llevan la etiqueta»*—: **se midió una
> propiedad y se publicó otra**. Que reapareciera aquí, en la sección que denuncia ese fallo, es el
> dato que importa: **no basta con haberlo escrito; hay que volver a medir**.
>
> **Consecuencia sobre `SFTY-2`, que es una regla vial:** su cobertura real no eran dos packs sino
> **cuatro**, y desde el 01/09 son cinco contando el arnés de las dos puntas que `barrera_04` vigila.
> La fila estaba **corta, no falsa** — pero una regla de seguridad que parece cubierta por la mitad
> de lo que la cubre invita a escribir el pack que ya existe.
| 🔴 **`enlace_01_transporte` NO lleva etiqueta A PROPÓSITO**: ninguna de las 29 reglas habla del transporte (`enlace_01_transporte.py` cabecera) **[MEDIDO]**. Asignar un número nuevo es del responsable — `AB-8` | |

> 🔴 **`SFTY-27` designa DOS cosas distintas.** *(Anotado el 31/08; **recontado el 01/09**, y las tres
> cifras de la nota anterior eran falsas — ver el recuadro de abajo. `OPTIMIZACIONES.md` no se toca
> desde aquí: lo lleva otro agente.)*
>
> | dónde | qué dice |
> |---|---|
> | `OPTIMIZACIONES.md:1595` | *«SFTY-27 — Matrícula de pareja: quién obedece a quién (DISEÑO, **NO IMPLEMENTADO**)»* |
> | **17 sitios** en firmware, packs, mapa y manuales | *«SFTY-27 — el Esclavo **PIDE** y el Maestro **DECIDE**»* — la asimetría de `demanda_solicitar()`, **implementada y viva** |
>
> **Censo completo [MEDIDO 01/09]**, `grep -rn "SFTY-27"` sobre todo el árbol, cruzado con un segundo
> recuento por fichero (`grep -rc`) que da la misma lista de 19 ficheros — 17 sitios de la regla viva,
> más `OPTIMIZACIONES.md` y `roadmap.md`, que hablan **de** la colisión:
>
> | firmware (8) | `Maestro/src/botones.cpp:71` · `Maestro/src/bluetooth.cpp:483` · `Maestro/include/botones.h:50` · `Maestro/include/demanda.h:8` · y los cuatro gemelos del Esclavo (`botones.cpp:92`, `bluetooth.cpp:456`, `botones.h:69`, `demanda.h:19`) |
> |---|---|
> | **packs (2)** | `camara_02_j16.py:679` · `app_08_enrutado_por_punta.py:52` |
> | **mapa (2)** | `ARQUITECTURA.map:97` · `ARQUITECTURA.map:267` |
> | **manuales (5)** | `04_Manuales/MANUAL_CONFIGURACION_BLUETOOTH.md:152` · `05_Funcional/10_...Telemetria.md:338` · `05_Funcional/18_...ESP32.md:449` · `05_Funcional/1_Manual_Usuario.md:259` · `05_Funcional/3_Protocolo_Pruebas_Rigurosas.md:1048` |
>
> #### 🔴 El «ocho» era de otra cosa, y la nota del 31/08 lo publicó como si fuera esta
>
> Los tres números que traía esa nota —*«9 sitios»*, la lista de 13, y *«ocho de esos sitios remiten
> explícitamente»*— **no coinciden entre sí ni con la medida**, y el error tiene nombre: **`8` es el
> número de FICHEROS DE FIRMWARE que llevan la etiqueta**, que es lo que mide el `grep -rl` citado en
> `OPTIMIZACIONES.md:404`. Se copió como *«ocho sitios que mandan a leer la equivocada»*, que es una
> propiedad distinta y **no se había medido nunca**.
>
> **Lo que sí remite explícitamente a `OPTIMIZACIONES.md § SFTY-27` son CUATRO sitios [MEDIDO 01/09]:**
>
> | `Esclavo/include/demanda.h:19` · `Esclavo/src/bluetooth.cpp:456` | *«Ver OPTIMIZACIONES.md SFTY-27»* |
> |---|---|
> | `04_Manuales/MANUAL_CONFIGURACION_BLUETOOTH.md:152` | *«El detalle está en `OPTIMIZACIONES.md` § SFTY-27»* — **corregido el 01/09** |
> | `05_Funcional/10_...Telemetria.md:337-338` | igual, **partido en dos líneas** — **corregido el 01/09** |
>
> ⚠️ **Y el buscador estuvo a punto de fallar aquí, que es §4 de `CLAUDE.md` en directo.** El cuarto
> puntero **no lo encuentra** un `grep` de `SFTY-27` filtrado por `OPTIMIZACIONES` en la misma línea:
> el salto de línea del Markdown deja `OPTIMIZACIONES.md` en la 337 y `§ SFTY-27` en la 338. Con la
> búsqueda de una línea salen **3**; con `-B2 -A2` salen **4**. Un puntero partido por el ajuste de
> línea es invisible a la búsqueda obvia.
>
> #### Cuál se queda con el `27`, y por qué no es la antigüedad
>
> **La regla viva —*«el Esclavo pide y el Maestro decide»*— es la que debe conservar el número**, y el
> criterio es de coste, no de fecha: está referenciada desde **8 ficheros de firmware y 2 packs**,
> mientras que la *«matrícula de pareja»* **no tiene una sola referencia fuera de `OPTIMIZACIONES.md`**
> [MEDIDO: `grep -rn "SFTY-27"` no la cita en ningún `.cpp`, `.h` ni pack]. Renumerar la matrícula
> toca **prosa de un solo documento**; renumerar la regla viva toca **fuente y banco**. Es diseño sin
> construir contra código que corre en la calle.
>
> **La decisión sigue siendo del responsable** (`AB-8` / `P-3`): asignar el número nuevo a la matrícula
> es un acto de numeración, no de auditoría. Lo que sí se ha hecho el 01/09 es que **los dos punteros
> de manual dejen de engañar**, apuntando a donde la regla vive de verdad en vez de a una sección que
> dice otra cosa. **Los dos punteros del firmware siguen mandando a la sección equivocada** — son de
> otro agente.

> ### 🔴 01/09 — UN NÚMERO DUPLICADO NUNCA ESTÁ SOLO. Censo de las 29 reglas
>
> Se censaron **todas** las `SFTY-x` del árbol y se cruzó cada referencia contra la definición de
> `OPTIMIZACIONES.md`. `SFTY-27` no era la única: **`SFTY-3` y `SFTY-7` están INTERCAMBIADAS en
> `protocolo.cpp`, en las dos puntas.**
>
> | | lo que dice la definición | lo que etiqueta el código |
> |---|---|---|
> | **`SFTY-3`** | *«Suma de verificación polinomial **CRC-8 Maxim (`0x31`)** en todos los paquetes RF»* | `Maestro/src/protocolo.cpp:107` y `Esclavo/src/protocolo.cpp:107` la ponen sobre la **resincronización temporal del búfer** (`if (millis() - lastByteTime > 50) binIdx = 0;`), que no es el CRC |
> | **`SFTY-7`** | *«Reintento automático de órdenes ACK cada 3.5 s (`TIMEOUT_ACK_MS`)»* | `Maestro/src/protocolo.cpp:52` y `Esclavo/src/protocolo.cpp:52` la ponen sobre **el polinomio CRC-8 Maxim**, que es el contenido de `SFTY-3` |
>
> **Sólo `Repetidor/src/main.cpp:37` usa `SFTY-3` para lo que la definición dice.** Las referencias
> correctas de `SFTY-7` viven en `Maestro/include/coordinador.h:54` y `Maestro/src/coordinador.cpp:431`
> — es decir, **cada número designa dos cosas, igual que el 27**, y en el mismo fichero.
>
> **Y la tabla de trazabilidad heredó el error, que es lo que lo mantuvo de pie:** su fila
> `SFTY-7 | */src/protocolo.cpp` apunta al sitio mal etiquetado, no al reintento de ACK. Es
> literalmente el defecto que `OPTIMIZACIONES.md` reconoce dos párrafos más arriba —*«para la segunda
> columna no lo comprueba nadie»*—, y es el mismo caso que la corrección `SFTY-10`/`SFTY-11` de N-6:
> aquella pasada arregló una pareja permutada y **no volvió a barrer el fichero**.
>
> **A diferencia del 27, esto NO necesita decisión del responsable:** no hay que asignar número
> nuevo ni renombrar regla, sólo poner cada etiqueta sobre la línea que le corresponde. Son 4 líneas
> de comentario en firmware, más la fila de la tabla. **No se toca desde aquí**: `01_Firmware/` y
> `OPTIMIZACIONES.md` son de otros agentes.
>
> #### ✅ **CERRADO el 01/09 por el agente de firmware (`4d3b1b9`). Verificado el 02/09, no leído**
>
> El párrafo de arriba se conserva **porque describe lo que había**, no lo que hay. Medido hoy sobre
> los dos `protocolo.cpp`:
>
> | línea | 31/08 | **02/09 [MEDIDO]** |
> |---|---|---|
> | `*/src/protocolo.cpp:52` | `SFTY-3` sobre el polinomio → **etiqueta equivocada** | `// SFTY-3: Polinomio CRC-8 Maxim/Dallas (0x31) bit a bit` → **coincide con la definición** |
> | `*/src/protocolo.cpp:107` | `SFTY-7` sobre la resincronización del búfer | `// SIN REGLA SFTY ASIGNADA, y se dice a proposito: llevaba la etiqueta SFTY-3, que es…` |
>
> **Y la parte que no era obvia: el arreglo correcto dejó una fila VACÍA, no una etiqueta nueva.** La
> resincronización temporal del búfer **no es ninguna de las 29 reglas**, así que ponerle un número
> —cualquiera— habría sido el defecto de la Convención de `CLAUDE.md`: *«una regla que aparece
> cubierta por una prueba que no la ejerce es peor que una fila vacía, porque la vacía no miente»*.
> Se declara sin regla **y se dice por qué**, que es la misma solución que `enlace_01_transporte`.
>
> `grep "SFTY-7"` sobre los dos `protocolo.cpp` da hoy **cero** [MEDIDO]; el número vive donde la
> definición dice, en `Maestro/include/coordinador.h:54` y `Maestro/src/coordinador.cpp:431`.
> **`SFTY-27` sigue abierto** — es el que necesita al responsable, y no se cerró con éste.
>
> #### Reglas definidas que nadie referencia (huérfanas)
>
> | `SFTY-24`, `SFTY-25` | **cero referencias fuera de `OPTIMIZACIONES.md` e `INDICE_CRUZADO.md`** [MEDIDO] |
> |---|---|
>
> **No es un defecto y no se arregla**, por el motivo ya escrito en §5 de este documento: son diseños
> declarados honestamente como *«sin construir»*, y copiarlos a un manual los convertiría en promesas.
> Se deja medido para que la próxima pasada no lo vuelva a levantar como hallazgo.
>
> #### Referencias a reglas que no existen: **ninguna** [MEDIDO]
>
> El censo devuelve dos números por encima del catálogo, y **los dos son legítimos**, no punteros rotos:
>
> - **`SFTY-30`** — `05_Funcional/18_...ESP32.md:992`, y no cita una regla: dice que *«la numeración
>   llega hoy a `SFTY-29`, así que las siguientes empezarían en `SFTY-30`»*.
> - **`SFTY-99`** — `packs/documentos_02_trazabilidad_sfty.py:131`, un número **sintético** dentro del
>   control negativo del propio pack, para comprobar que sabe leer una fila sin packs.
>
> #### Un tercer defecto, ya corregido: la Ventana Deslizante estaba bajo el número de la Ráfaga
>
> `05_Funcional/1_Manual_Usuario.md:233` y `MANUAL_USUARIO.md:82` (raíz) titulan *«Ráfaga configurable
> y Ventana Deslizante **(SFTY-11)**»*. **`SFTY-11` es la ráfaga; la ventana deslizante es `SFTY-10`**
> —la definición lo dice, y el código lo etiqueta bien en `*/src/protocolo.cpp:120`—. Es el mismo
> error que N-6 corrigió **en el firmware** en 2026-08-03 y **no corrigió en los manuales**, que lo
> llevan arrastrando desde entonces. Corregido el 01/09 en `05_Funcional/1_Manual_Usuario.md`;
> **`MANUAL_USUARIO.md` de la raíz sigue mal** — no es de esta pasada.
>
> *(De paso, medido en la misma línea: ese apartado decía **«1 copia»** de ráfaga cuando
> `RF_BURST_COPIES` vale **3** en `*/include/protocolo.h:162`. Corregido junto con la etiqueta, porque
> un técnico escuchando el bus contaría tres tramas y las leería como reenvío por fallo.)*

### 1.6 El contrato de bytes del Bluetooth, y las cinco tramas

**El documento propietario del contrato es `01_Firmware/ESP32_Expansion/include/contrato.h`**, y está
escrito así a propósito: *«cada número de este fichero tiene un GEMELO viviendo en otro lenguaje»*
—el C++ del STM32 y el JavaScript de la app—, **y un pack relee las TRES fuentes en cada corrida**.
Ese es el mecanismo, y es el único que no envejece.

| hecho | constante / sitio | vigilado por |
|---|---|---|
| **Baudio y formato del enlace** | `ENLACE_BAUDIO 9600`, `ENLACE_FORMATO SERIAL_8N1` | `esp32_09_contrato_de_bytes` |
| **Tope de línea útil = 63 B**, porque el STM32 **descarta el exceso en silencio** (`btBufIn[64]`, `Maestro/src/bluetooth.cpp:31` y `:397`) | `TRAMA_MAX_UTIL` | `esp32_09_contrato_de_bytes` |
| **Buffers**: entrada app, entrada STM32 (160 B), salida app (512 B, por la ráfaga `P-2` de 340 B) | `BUF_*` | `esp32_07_presupuesto_bytes` |
| **La desigualdad del watchdog**: `ESP32_WDT_MS + ESP32_ARRANQUE_MS < min(TIMEOUT_ENLACE_MS, SFTY6_SILENCIO_MS)` | `contrato.h` §watchdog | `esp32_01_watchdog_desigualdad` · `esp32_02_watchdog_alimentado` |
| 🛑 **`ESP32_ARRANQUE_MS = 1500` está SIN VERIFICAR** y lleva su propia bandera `ESP32_ARRANQUE_MEDIDO 0` para que no se lea como medida | `contrato.h` | `esp32_01` lo **`reportar()`** en cada corrida — `reportar()` no cuenta como comprobación |
| **`W-4`, `P-2`, `P-3`, `R-4`, `R-9`** (las reglas de diseño del puente) | identificadores en `18_...ESP32.md`, **razonados dentro de `contrato.h`** | los packs `esp32_*` |
| **Rangos del `DS3231` en un solo sitio** (`R-9`), y la validación los **barre todos** | `contrato.h` §RTC | `esp32_04_osf` **+ `esp32_11_bien_formada_no_es_cierta` (01/09)** — ver abajo |
| 🔴 **«BIEN FORMADA» NO ES «CIERTA», y el `OSF` sólo cubre una de las puertas** *(nuevo, 01/09)* | `reloj_ds3231.cpp` | ✅ **`esp32_11_bien_formada_no_es_cierta`** (`EJERCE SFTY-18`). `esp32_04` vigila el bit `OSF` y **sigue valiendo**; lo que no podía ver es que `OSF` a cero significa exactamente *«el oscilador no se paró»* y se estaba leyendo como *«el número que vas a devolver es la hora»*. **Eran tres puertas más**, y las tres acababan en **una fecha perfectamente formada y falsa** saliendo por el puente |
| 🟢 **El parte de arranque del puente**: cuando el watchdog muerde, **alguien se entera** | `vigilante.cpp:55-121`, `esp_reset_reason()` y sus once causas **[MEDIDO 02/09]** | ✅ **`esp32_10_parte_de_arranque` (01/09).** No es redundante con los dos del watchdog y el pack lo razona: `esp32_01` mide **el número**, `esp32_02` **el mecanismo**, y los dos **dan por bueno un módulo que se reinicia**. Desde la app, un ESP32 que se reinicia cada dos segundos y uno sano **se ven igual** |

**Las CINCO tramas que el STM32 emite — son cinco, no cuatro:**

```
$STATUS   $ACK   $ERR   $ALARM   $EVENT
```

`01_Firmware/ESP32_Expansion/src/trama.cpp:9-11` **[MEDIDO]**. Censo completo en
`05_Funcional/18_...ESP32.md` §3.7; la trama de entrada, en §3.2.

> 🔴 **`$EVENT` es la que se cae de las listas escritas de memoria** —catorce ramas del Maestro lo
> emiten y la app lo consume—, y un puente que filtrara por cuatro prefijos **se comería la bitácora
> entera**. Por eso **la lista NO es un filtro**: el puente retransmite toda trama bien formada, la
> conozca o no, y valida **formato, no comandos** (`contrato.h`, censo de prefijos).

### 1.7 Los límites del protocolo

| límite | dónde vive **el número** | quién lo relee |
|---|---|---|
| **`SFTY6_SILENCIO_MS = 25000`** (era 12 s antes de N-71) | `*/include/protocolo.h`, idéntico en las dos puntas | `costura_08_silencio` · `costura_09_presupuesto_radio` |
| 🟢 **El margen del Modo Degradado contra la deriva de los dos relojes**: `despeje ampliado > deriva acumulada durante el límite duro`. **Era SOLO PROSA hasta el 01/09** — vivía dentro de un comentario de los dos `.cpp`, que es exactamente cómo se perdió el techo de SFTY-6 en N-71 | se **recalcula desde el C++** | ✅ **PASÓ A VIGILADO (01/09)**: `costura_12_margen_deriva` (`EJERCE SFTY-21`) **+ el arnés del Degradado a dos puntas**, que lo ejecuta. El Degradado es el **único modo sin coordinador**: ahí el choque frontal no lo impide un enclavamiento, sino esa desigualdad |
| 🟢 **El registro de silencio de `J17`**: las dos puntas cuentan `MUDO`/`MAX`/`N` en segundos y lo publican en un `$EVENT`, **sin un solo umbral y sin tocar una luz** | `Maestro/src/bluetooth.cpp:197-222` · `Esclavo/src/bluetooth.cpp:232-257` **[MEDIDO 02/09]** | ✅ **NUEVO Y VIGILADO (01/09)**: `enlace_02_silencio_j17`. La propiedad que vigila es tanto lo que hace como **lo que tiene prohibido**: que no se enganche a `SFTY-6` —*«si el silencio de J17 alimentara al de la radio, un teléfono conectado SALVARÍA al cruce de una caída de radio real»*—. Ver el límite en `§1.8 AB-1` |
| 🔴 **En el poste siguen los 12 s** (N-108, 31/08). Los 25 s están **en la rama** desde el 27/08; el equipo de la calle es la V8.4 (`e303485`). El reporte de campo —*«se va a ámbar a los 12 segundos, por nada»*— **confirma N-71 por el otro lado**, y le cambia el sentido a la sesión de banco: no es solo validar lo nuevo, **es que el síntoma de hoy se arregla con lo que ya está escrito** | `roadmap.md` §N-108 · commit `318d67f` | — ningún pack puede vigilar lo que hay cargado en un poste |
| **La desigualdad techo-vs-reintentos** (5 reintentos a 3,5 s ≈ 20,5–20,8 s bajo un techo de 25 s) | se **recalcula desde el C++** | `costura_09_presupuesto_radio` — **por eso ya no vive en prosa** (la lección de N-71) |
| **Despeje todo-rojo 10–90 s**, piso inquebrantable | `Maestro/src/modo_automatico.cpp:34` | `documentos_04_cifras_sin_vigilante` + el arnés del automático |
| **`RF_BURST_COPIES = 3`** · **CRC-8 Maxim `0x31`** | `protocolo.h` | `costura_01_contratos` |
| **Límite de 48 h del respaldo** | `Maestro` respaldo | `costura_05_limite_48h` |
| **`TIMEOUT_ACK_MS = 3500`** | `protocolo.h` | `costura_09` |
| **Los cuatro límites del automático están escritos DOS veces** —`modo_automatico.cpp:32-34` y a mano en `app.js`— **sin nada que los ate** *(N75-2, abierto)* | `ESTADO.md` N75-2 | 🔴 **nada, sigue siendo el hueco declarado** [RE-MEDIDO 02/09: `grep -l "VERDE_MIN_MIN\|DESPEJE_SEG_MIN\|ROJO_MIN_MIN"` sobre los 66 packs da **`documentos_04`** y **`maestro_08`**, y los dos leen **sólo el C++**. Ninguno cruza contra `app.js`] |
| 🔴 **Y N75-2 tiene un HERMANO que nadie había anotado** *(medido el 02/09)*: `js/config.js` define `IOT_CONFIG` con **PIN, baudio, límites de tiempo y UUIDs BLE**… y **no lo llama nadie**, porque `app.js` lleva su propia copia. No es un hecho único: es **la misma duplicación sin vigilante, en un cuarto sitio** | `app_07_generadores_de_trama.py:94-101`, en su lista `HUERFANOS_CONOCIDOS` | 🟠 **el huérfano SÍ está vigilado** —`app_07` lo lleva censado y una nueva rompería el pack—; **la coherencia de los números duplicados, NO**. Ver `P-8` |
| **El PIN `1234` va en claro en el fuente y en el aire** | `05_Funcional/18_...ESP32.md` §3.5 (`AB-7`) — **único sitio** | ⚠️ **PARTIDO EL 02/09, y las dos mitades no valen lo mismo.** El **número** sí se relee: `app_01_comandos.py:149-157` lo saca del `CMD:PIN:(\d+):` de **los dos `bluetooth.cpp`** y exige que las dos puntas pidan el mismo *(«un técnico que cambie de poste tendría que cambiar de PIN sin que nada se lo diga»)*, y `app_07` lo relee de `js/nmea_parser.js` y `app.js`. **Lo que sigue SOLO PROSA es la DECISIÓN** — que ir en claro es una limitación conocida y aceptada, no un descuido. Ver `P-7`: es justo esa mitad la que decide si se para una entrega |

### 1.8 Decisiones abiertas, con dueño

| conjunto | dónde vive | dueño | vigilado |
|---|---|---|---|
| **`AB-1` … `AB-8`** (ocho decisiones del ESP32, con dueño y con lo que desbloquea cada una) | `05_Funcional/18_Especificacion_Firmware_ESP32.md` **§9** | responsable / quien monte / técnico | 🔴 **nada** — ver §3 |
| **`AB-1`: nada en el equipo vigila al puente** | ídem | responsable | ⚠️ **SIGUE ABIERTO, pero ya no a ciegas (01/09)**. `enlace_02_silencio_j17` vigila un registro que **existe** desde `d44048c`. 🔴 **Y su límite hay que leerlo, porque es lo que impide dar el `AB-1` por cerrado: el silencio se cierra CUANDO LLEGA UNA LÍNEA**, y un puente muerto no manda ninguna. El STM32 **sigue sin poder ver morir al puente en vivo**; lo que gana es la prueba *después*. *«Cuánto lleva mudo ahora mismo» no es observable por construcción* (`bluetooth.cpp:199-202`). El único testigo en vivo sigue siendo la app |
| **Decisiones 3.1 a 3.5** (chip ESP32 ✅ cerrada · `Y2` · superficie de mando ✅ decidida · mínimo por sentido · cámara 1 en `PB0` o `J16`) | `05_Funcional/17_...Decisiones_Abiertas.md` **§3** | responsable | — |
| **`BLQ-2`: el cristal `Y2` de la SEGUNDA tarjeta sin diagnosticar** | `ESTADO.md` · `roadmap.md` · `11_Manual_Instalacion_RTC...` | responsable (banco, `B5`) | — |
| **`N-106`: el ámbar de la app no sacaba al Esclavo del Degradado** | `ESTADO.md` · `roadmap.md` · `OPTIMIZACIONES.md` · 7 documentos | responsable decide **qué debe hacer**; técnico escribe el arnés | ✅ **CERRADO — y el pack está VIGILANDO, no celebrando.** `esclavo_08_ambar_en_degradado` da hoy **8/8** *(corrido solo el 02/09)*, y **nació EN ROJO a propósito** (`829c457`): por eso su verde vale. **Cuatro de sus ocho comprobaciones son controles negativos**, y uno existe justo para que el arreglo no entrara por **una sola** de las dos puertas. Medido en `Esclavo/src/bluetooth.cpp`: las dos puertas del ámbar consultan `degradado_gobiernaLuz()` **dentro de su bloque** (`:316`, `:403`), tienen rama para `degradado_rendicionEnCurso()` (`:340`, `:417`), y hay un **envoltorio** (`:260-273`) que devuelve lo que `degradado_salir()` —que es `void` y abandona en silencio— no sabía decir. 🔴 **Lo cerrado es la incoherencia entre las dos vías, NO la decisión vial**: *qué debe hacer* el ámbar de la app en Degradado sigue siendo del responsable |
| **`N75-1` … `N75-4`** | `ESTADO.md` · `17_...` §3.4 · `roadmap.md` | responsable / técnica | — |
| **`M1` … `M5`, las cinco medidas de multímetro** | `05_Funcional/17_...` **§A** — **es el único sitio con el procedimiento** | quien monte | — **SOLO PROSA** |

### 1.9 Bloqueos y compras

| hecho | dónde |
|---|---|
| **El listado consolidado de compras** | `05_Funcional/15_Lista_de_Compras_Hardware.md` — **documento propietario** |
| **`A5`: la fuente propia del ESP32 (DC-DC 12 V→5 V, ≥1 A). NO SE HA PEDIDO y hace falta** | `15_...` línea `A5` · `ESTADO.md` `COMPRAS` · `10_...Telemetria.md` §1 (la cuenta de los 3,5 W del `LM7805`) — **6 documentos, bien referenciado** |
| **`A6`: `DS3231` `ZS-042`, colgado del ESP32, ya no espera al banco** | `15_...` · `11_...` · `contrato.h` (`DS3231_DIR`, y el aviso de que el módulo **ya trae sus pull-ups**) |
| **`A1′`: el ESP32 sustituye a los módulos SPP; `A1` anulada** | `15_...` — **único sitio con la línea** |
| **🛑 `BANCO` sigue siendo EL bloqueante del proyecto** | `CLAUDE.md` §1 · `ESTADO.md` Fase 6 · el acta de `evidencia/` · todos |
| **La placa portadora en sí no es todavía una línea de compras, y quién la diseña y quién la fabrica NO está decidido** | 🔴 **NINGUNO** — ver §4 |

### 1.10 Qué hace cada instrumento

**La lista autorizada es `01_Firmware/compuerta.py`**: un instrumento que no está ahí no mide nada
(`CLAUDE.md` §3). Las cifras, del acta de `evidencia/`.

| instrumento | qué ejerce | punto ciego |
|---|---|---|
| `compuerta.py` | **la única forma correcta de verificar**; guarda de rutas + compila los cuatro roles | un `0` **no dice que el firmware funcione en la tarjeta** |
| **banco por packs** (`Simulaciones/banco/`) | un fichero corto por propiedad | **Python escrito a mano**: un `PASS` prueba el MODELO |
| `Validacion_LCD` | compila `lcd.cpp`, `menu.cpp`, `modo_degradado.cpp` reales | framebuffer en el PC, **no** la ST7920 |
| `Validacion_Ciclo` | compila `ciclo_degradado.h` | función pura, sin máquina de estados |
| `Validacion_Respaldo` | compila `calcularSuma()`, Horner, `respaldo_horasDesdeSync()` | no ejerce el arranque |
| `Validacion_Automatico` | compila `coordinador` + `semaforo` + `modo_automatico`; **mide SFTY-2 sobre los pines escritos** | **solo el Maestro** — y **ya no es el único**, ver la fila siguiente |
| ✅ **`Validacion_Automatico/dos_puntas/`** (01/09, `compilar_dos_puntas.ps1`) | **el único que ejecuta el C++ real de LAS DOS PUNTAS a la vez.** Compila cada punta en su propia DLL —Maestro y Esclavo definen los **mismos símbolos** y no enlazan juntas—, las carga en un proceso y les da el **mismo `millis()`**: *«verde en las dos a la vez»* tiene por fin un instante donde ocurrir. **Cierra el punto ciego que la fila de arriba declara** | sigue siendo el PC, no la tarjeta. Lo vigila `barrera_04_arnes_dos_puntas` |
| ✅ **arnés del Degradado a dos puntas** (01/09, `compilar_degradado.ps1`, `orquestador_degradado.cpp`) | el **único modo sin coordinador**: ahí el choque no lo impide un enclavamiento sino una desigualdad numérica | ídem. Lo acompaña `costura_12_margen_deriva` |
| `simulador_sistema_v7_6.py` · `simulador_repetidor.py` · `simulador_app_bluetooth.py` | modelos | modelos |
| **DOS** arneses unitarios de la app · arnés DOM · `test_funcional_app.py` | la app real | no es un teléfono. **El segundo unitario —`tests/test_unitarios.js`— existía y NO estaba conectado**; se dio de alta el 02/09 |
| **`simulador_puente_esp32.py`** (31/08) | **el LAZO ENTERO app ↔ ESP32 ↔ STM32**; dos de sus tres puntas son código real | el ESP32 **sigue siendo modelo en Python aunque su C++ ya exista**: el simulador no lo carga |
| **`compilar("esp32", "ESP32_Expansion")`** | que el C++ del ESP32 **compile** — los packs no lo prueban | no ejecuta |

### 🔴 1.10.bis Instrumentos que EXISTEN y NO están en la compuerta [MEDIDO 02/09]

> **La pregunta de `CLAUDE.md` §3 —*«¿hay algún instrumento que exista y no esté conectado?»*— tiene
> hoy dos respuestas**, y la segunda no aparece en ningún censo porque **ningún fichero del árbol la
> nombra**. Se dejan aquí con su comando; **conectarlos o retirarlos no es de este documento.**

| instrumento | quién lo nombra | comando | por qué no entra tal cual |
|---|---|---|---|
| **`05_Funcional/App_Semaforo/tests/test_e2e_visual.js`** | `package.json` (`"test:e2e"`), `README.md` de la app, Manual `14` línea 144, y `16_Documento_Auditoria...` §4.2 — que lo llama **«Validación Visual E2E»** | `node tests/test_e2e_visual.js` (con `python servidor_puente_simulador.py 3000` levantado) | **No afirma nada: es un capturador de pantalla.** No publica ninguna cuenta `x/y`, así que la regla de la cuarta cara de N-46 no tiene qué exigirle. Sólo pone `process.exitCode = 1` si *revienta*, no si la pantalla sale mal |
| 🔴 **`05_Funcional/App_Semaforo/test_e2e_puppeteer.js`** | **NADIE.** `grep -rn` sobre todo el árbol antes de esta pasada: **cero** *(control positivo: el fichero de al lado, `tests/test_e2e_visual.js`, da **4 ficheros vivos** —Manual 14, doc 16, `README.md` de la app y `package.json`— más dos `.zip` de entrega. La búsqueda sabía encontrar; el cero es real)* | `node test_e2e_puppeteer.js` | **Peor: se traga toda excepción y sale con `0`** imprimiendo *«🎉 TODOS LOS CONTROLES Y PANTALLAS FUERON PROBADOS CON ÉXITO»*. Es N-46 literal —fallo pintado de verde— en un fichero que además escribe sus capturas a un directorio de otro IDE (`.gemini/antigravity-ide/brain/<uuid>`) |

> ⚠️ **Y los dos comparten un defecto que `CLAUDE.md` §4.ter ya cobró una vez: miden a `412 px` y
> sólo a `412 px`** —`setViewport({width: 412})` en los dos—, que es **el único de los cuatro anchos
> medidos donde el fallo de recorte no aparecía**. Conectarlos sin más anchos daría un verde con el
> mismo valor que tenían las capturas limpias del `evidencia/`.
>
> **Lo que sí falta y ninguno de los dos cubre:** un arnés de interfaz que **afirme** —anchos,
> contraste, recorte— en vez de fotografiar. Hoy quien mide la app de verdad son los tres que sí
> están en la compuerta: los dos unitarios y el de DOM.

> ✅ **Corregido el 02/09 — y la corrección es la lección.** Este apartado acusaba a
> `compuerta.py:671-676` y `:802-806` de decir que el C++ del ESP32 *«todavía no existe»*. **Medido
> hoy, esas líneas son otra cosa**: `:671-676` es la guarda de `gcc` del arnés del respaldo, y
> `:802-806` el `ABORTADO` del arnés del Degradado a dos puntas. La acusación se quedó apuntando a
> unas líneas que se movieron.
>
> **Lo que sí queda, buscado por texto y no por número: UNA sola aparición, `compuerta.py:950`**,
> dentro del bloque del simulador del puente. **No se toca aquí** — `compuerta.py` no es de este
> encargo. Y la regla que deja: **un defecto se persigue por su texto, no por su línea**; una cita
> numérica a un fichero en vuelo caduca sola, y encima caduca **acusando a un inocente**.

---

## 2. MAPA POR FICHERO — «voy a reescribir Y: ¿qué se pierde?»

**Esta es la tabla que no existía el 31/08 por la mañana.** La columna que importa es la última.

Metodología de la columna «quién apunta»: se contó, para cada documento, **cuántos otros ficheros
nombran su nombre de fichero**, separando documentos de fuentes (`.py`, `.cpp`, `.h`, `.js`).
**[MEDIDO 31/08]**

> ⚠️ **Los números de esta tabla son los de ANTES de que existiera este índice, y hay que leerlos
> así.** Al nombrar aquí a los huérfanos, este fichero se ha convertido en **su único puntero
> entrante** —comprobado al volver a correr el censo—. **Eso no los arregla:** un índice que nadie
> abre no encuentra un documento mejor que el silencio. Los enlaces que hacen falta van en
> `ESTADO.md` y en `README.md`, que es donde la gente entra. **Anotado, no hecho: no son ficheros de
> este encargo.**

| fichero | quién apunta a él | qué hechos son **los únicos** que sostiene | riesgo |
|---|---|---|---|
| **`05_Funcional/18_Especificacion_Firmware_ESP32.md`** | **1 documento** (`5_Manual_Puente_ESP32.md`) + 2 fuentes | **`AB-1`…`AB-8` con su dueño** · el PIN en claro (`AB-7`) · «el puente NO ORIGINA» · «no parte ni une tramas» · la asimetría del checksum · «la hora nace no fiable» · escritura atómica · el censo de comandos y de tramas del STM32 · el presupuesto de bytes/s | 🔴 **EL MÁS ALTO DE TODO EL ÁRBOL.** Máxima densidad de hechos únicos, mínimo número de punteros. Su propio §9 ya avisa: *«`ESTADO.md`/`roadmap.md`: `BLQ-1` y `AB-1`…`AB-8` no están anotados como abiertos con dueño»* |
| **`05_Funcional/17_...Decisiones_Abiertas.md`** | 10 documentos + 1 fuente | **`M1`…`M5`, las cinco medidas de multímetro con su procedimiento** · el censo §B de documentos que quedan falsos · §2.8 (49 de las 80 pruebas dejan de ser ejecutables) | 🟠 **Bien referenciado**, pero `M1`–`M5` viven solo aquí |
| **`03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md`** | 19 documentos + 9 fuentes | **TODO el cobre**: reparto de `J17`, `R65`–`R68`, el margen de `1,36 mm` sobre cobre, `PB8`→`R16`→`D5`, `PB2`→`U15`→`Q10`→`J15` | 🔴 **Es el único documento de cobre del proyecto.** Muy apuntado, pero **sin copia**: si se equivoca, se equivocan los 19 |
| **`OPTIMIZACIONES.md`** | 13 documentos + 16 fuentes | **el catálogo `SFTY-1`…`SFTY-29`**, y en exclusiva `SFTY-19`, `SFTY-24`, `SFTY-25` | 🟠 documento propietario; la trazabilidad **sí** la vigila `documentos_02` |
| **`01_Firmware/ESP32_Expansion/include/contrato.h`** | los packs `esp32_*` y todo `src/` del ESP32 | los números del contrato **y su porqué** | 🟢 **el modelo de cómo se hace bien**: hecho único, en su dueño, **releído por packs en cada corrida** |
| **`ESTADO.md`** | 19 documentos | el estado de HOY, los bloqueantes, las seis fases | 🟢 muy apuntado y con `documentos_01` vigilando sus cifras |
| **`05_Funcional/15_Lista_de_Compras_Hardware.md`** | 11 documentos | **la línea `A1′`** | 🟢 documento propietario |
| **`05_Funcional/Guia_Cableado_y_Pruebas_Banco.html`** | 3 documentos (`ESTADO.md`, `README.md`, `roadmap.md`) | ~~los 23 pasos~~ → **los 29 pasos** de campo con `HAZ / COMPRUEBA / TIENES QUE VER / ANOTA` *(**MEDIDO el 01/09** sobre el fichero, por cinco caminos que dan lo mismo: 29 `<div class="fila">`, 29 `id="fN"`, 29 `<div class="fn">`, 29 `<p class="ft">` y 29 `name="pN"`, numerados 1…29 sin huecos. `81f31f7` insertó seis pasos nuevos —el montaje de mesa— y **corrió los del 9 en adelante seis sitios**; los `.md` escritos contra la numeración vieja no se tocaron)* | 🔴 **YA COBRÓ SU RIESGO EL 31/08** — ver §4. Sostenía cinco hechos de arquitectura que no eran de campo, y al reescribirlo para el técnico se fueron |
| **`05_Funcional/12_Cobertura_de_Pruebas_y_Huecos.md`** | **CERO** *(antes de este indice)* | el censo de cobertura y sus huecos | 🔴 **HUÉRFANO TOTAL**: nadie lo enlaza, ni un documento ni una fuente. Es el problema de los 81 KB de la guía, otra vez |
| **`05_Funcional/16_Documento_Auditoria_...App_IOT_VIAL.md`** | **CERO** *(sigue en cero: este indice no lo nombra por su nombre de fichero completo)* | la auditoría de arquitectura y usabilidad de la app | 🔴 **HUÉRFANO TOTAL** |
| **`05_Funcional/LEEME_PRIMERO_APP.md`** | **CERO** *(antes de este indice)* | lo primero que debería leer quien recibe la app | 🔴 **HUÉRFANO TOTAL** — y su nombre dice que es lo primero que hay que leer |
| **`01_Firmware/FIRMWARE.md`** | **CERO** *(antes de este indice)* | — | 🟠 huérfano total |
| **`05_Funcional/ENCARGO_SESION_BANCO.md`** | 1, y es un **histórico cerrado** | el encargo de la sesión de banco, que es **el bloqueante del proyecto** | 🔴 el documento del bloqueante, colgando de un fichero archivado |
| **`ORDEN_EJECUCION.md`** | 1 (`CERTIFICACION_SW.md`) | — | 🟠 del 28/07; **probablemente caducado, y nadie lo va a echar de menos** |
| **`ARQUITECTURA.map`** | **4 documentos** [RE-MEDIDO 02/09]: `02_LCD/MANUAL_PANTALLA_LCD.md:29`, `05_Funcional/README.md:135`, `OPTIMIZACIONES.md:427` **(nuevo)** y este índice. 🔴 **`roadmap.md:142` YA NO** — el roadmap se reescribió el 31/08 y perdió la referencia | el censo de pines en un sitio, los dos anexos de asimetría | 🟢 **ningún pack lo parsea** [RE-MEDIDO 02/09 con su control: `grep "ARQUITECTURA"` sobre `.py` da 5 y ninguna lo lee; `flash_01_lastre` sí parsea un `.map`, pero es `firmware.map`], así que reescribirlo no rompe la compuerta — 🔴 **y ésa es también su enfermedad: la pasada del 02/09 verificó sus 60 citas `fichero:linea` y encontró seis apuntando a código que se había movido.** Las **rutas** aguantaron las 36, porque ésas sí tienen vigilante: la guarda de rutas. **Lo que nadie relee, envejece sin avisar** |
| **`04_Manuales/MANUAL_MANDO_4_RELES.md`** · **`05_Funcional/8_Procedimiento_Modo_Degradado.md`** | 9 cada uno | las secuencias `A·A·A`, `B·B·B`, `A·B·A·B` **desde el punto de vista del operario** | 🟢 el firmware lo vigila `maestro_01_mando` |

---

## 3. 🔴 CENSO DE HECHOS QUE VIVEN EN UN SOLO SITIO

### 3.1 Método, y el descarte del buscador

Se barrieron **75 ficheros** de documentación (`.md`, `.html`, `.map`, `.txt`) de todo el árbol,
excluyendo `99_Legacy`, `evidencia/` y `node_modules`. Para cada hecho se contó **en cuántos ficheros
distintos** aparece su término característico.

**Control positivo — la búsqueda sabía encontrar más de uno** *(si estos hubieran dado 1, el censo no
valdría nada)*:

| término | ficheros |
|---|---|
| `SFTY-6` | **19** |
| `escribirPines` | **8** |
| `PB0` | **24** |
| `J17` | **23** |
| `compuerta.py` | **14** |
| `Modo Degradado` | **33** |
| `Y2` | **21** |

**Control negativo:** un término inventado (`ZZQQXX_NO_EXISTE_NUNCA`) → **0 ficheros**. La búsqueda
distingue el cero del uno.

**Y el aviso de `CLAUDE.md` §4 aplicado al formato**, porque aquí también muerde: buscar
`"placa portadora"` con `grep -i` sobre `.md` da 1. Se repitió sobre **todos** los ficheros de texto
del árbol, incluidos `.py`, `.h`, `.cpp`, `.js`, y **sigue dando los mismos tres ficheros**, dos de
ellos por «portadora» en el sentido de **portadora de radio** (`Esclavo/src/main.cpp:74`,
`simulador_repetidor.py:55`). El cero es real.

### 3.2 🔴 HECHOS ÚNICOS **PELIGROSOS** — hay que duplicarlos o referenciarlos YA

> **Criterio:** barrera de arquitectura, límite de seguridad o requisito de diseño que **nada relee**
> y que, si desaparece, **nadie echa de menos**.

| # | hecho | vive en | por qué es peligroso | qué hacer |
|---|---|---|---|---|
| **P-1** | 🔴 **La especificación entera de la placa portadora**: qué lleva, qué NO lleva, los puntos de prueba, el acceso de reflasheo, la regla de trazado y la medida `M6` | **NINGÚN FICHERO.** `grep` sobre todo el árbol: **cero** | **Ya se perdió.** Detalle completo en §4 | **Su propio documento** — el responsable lo va a encargar aparte |
| **P-2** | 🔴 **`AB-1` … `AB-8`: ocho decisiones abiertas, cada una con su dueño y con lo que desbloquea** | `18_...ESP32.md` §9, **y nada más** (`AB-2`…`AB-8` dan exactamente **1** fichero; `AB-1` da 2) | Ese fichero tiene **un solo puntero documental**. Si se reescribe o se archiva, **ocho decisiones con dueño desaparecen sin dejar rastro** — y su propio §9 ya avisa de que `ESTADO.md` no las tiene | **Referenciarlas desde `ESTADO.md`** en la tabla de bloqueantes, con el enlace al §9. **Con enlace, no copiadas**: la copia haría dos versiones |
| **P-3** | 🔴 **`SFTY-27` designa dos reglas distintas** (§1.5) — **17 sitios** usan la viva; ~~ocho~~ **CUATRO** mandan explícitamente al lector al sitio equivocado *(recontado el 01/09: el «ocho» era el número de FICHEROS de firmware con la etiqueta, copiado como si fuera el de punteros)* | ya está escrita: `OPTIMIZACIONES.md` §`SFTY-27` y su tabla, §1.5 de aquí | Una regla vial implementada aparece bajo un número marcado *«NO IMPLEMENTADO»*. Nadie lo ve porque **hay que leer los dos sitios a la vez** | **Decisión del responsable** (`AB-8`): el `27` se queda con la regla viva —está en 8 fuentes y 2 packs; la matrícula no está en ninguno— y la matrícula toma número nuevo. Los 2 punteros de manual ya no engañan (01/09); **quedan los 2 del firmware** |
| **P-3.bis** | ✅ **CERRADO el 01/09** (`4d3b1b9`). Era: *«`SFTY-3` y `SFTY-7` están INTERCAMBIADAS en `*/src/protocolo.cpp:52` y `:107`, en las dos puntas»* | §1.5 de aquí | *(era)* El CRC llevaba el número del reintento de ACK: el mismo defecto que el 27, y la misma clase de sitio que N-6 ya corrigió una vez | **HECHO, y verificado el 02/09 midiendo, no leyendo el commit:** `:52` etiqueta hoy el polinomio como `SFTY-3` —que es lo que dice la definición— y `:107` declara **«SIN REGLA SFTY ASIGNADA»** con su porqué. `grep "SFTY-7"` sobre los dos `protocolo.cpp` → **cero**. 🟢 **Lo reutilizable: el arreglo correcto dejó una fila VACÍA, no una etiqueta nueva** — la resincronización del búfer no es ninguna de las 29, y ponerle un número habría sido *«una regla cubierta por una prueba que no la ejerce»* |
| **P-3.ter** | 🔴 **La lista de packs con `# EJERCE SFTY-x` de §1.5 estaba INCOMPLETA el día que se publicó** — y no por deriva: `esclavo_06` (SFTY-2) faltaba entero y `maestro_09` entró con **una** de sus **dos** etiquetas, las dos presentes desde el 28/08 | §1.5, corregido el 02/09 | Es el error del «ocho» de `SFTY-27` **repetido dentro de la sección que lo denuncia**: se midió *ficheros* (`grep -rl`) y se publicó *reglas ejercidas*. La cobertura real de `SFTY-2` —una regla **vial**— parecía la mitad de lo que es | **HECHO.** Y la regla que deja: **una etiqueta por línea, no por fichero** (`grep -rn` y contar líneas). Un pack puede ejercer más de una regla, y `documentos_02` levanta la tabla de trazabilidad **de esas líneas** |
| **P-4** | 🔴 **Las cinco medidas de multímetro `M1`…`M5`, con su procedimiento** | `17_...` §A, y solo ahí el **procedimiento** | `M3` es **bloqueante de cableado** y se cita en 15 documentos… pero **cómo se hace** está en uno. Reescribir el 17 deja quince punteros a un procedimiento que ya no existe | **Referenciar desde la guía de campo**, que es quien va a ejecutarlas |
| **P-5** | 🔴 **`05_Funcional/12_Cobertura_de_Pruebas_y_Huecos.md` es HUÉRFANO TOTAL** — y es, literalmente, el documento que dice **qué NO se está midiendo** | 0 punteros entrantes | Es el mismo defecto que los 81 KB de guía que no enlazaba nadie, aplicado al fichero cuyo tema **es** los huecos | **Enlazarlo desde `ESTADO.md` y desde `README.md`** |
| **P-6** | 🔴 **`05_Funcional/ENCARGO_SESION_BANCO.md`**: el encargo de la sesión de banco, que es **EL bloqueante del proyecto**, colgando solo de un histórico cerrado | 1 puntero, archivado | El día que se organice el banco, el documento que lo describe no se encuentra por ningún camino vivo | **Enlazarlo desde la Fase 6 de `ESTADO.md`** |
| **P-7** | 🟠 **El PIN `1234` va en claro en el fuente y en el aire** | `18_...ESP32.md` §3.5 | Es una **limitación de seguridad conocida y aceptada**. Si el único sitio que lo dice desaparece, el día que alguien la encuentre no sabrá si es descuido o decisión — y esa diferencia decide si se para una entrega | **SIGUE ABIERTO, y el 02/09 se precisó por qué.** El **número** sí lo relee un pack (`app_01_comandos.py:149-157`, de los dos `bluetooth.cpp`; `app_07` de `js/`), así que **no es un hecho sin vigilante — es un hecho con MEDIO vigilante**. Lo que ningún pack puede comprobar es que ir en claro sea una **decisión**: eso es prosa por naturaleza, y por eso es justo la mitad que hay que anotar **donde vive la barrera de PIN**, o en `OPTIMIZACIONES.md` |
| **P-8** | 🟠 **Los cuatro límites del automático están escritos dos veces y nada los ata** (`N75-2`) | `ESTADO.md` lo declara abierto | No es un hecho único: es un hecho **duplicado sin vigilante**, que es la otra cara. Hoy coinciden; el día que suba el mínimo, la app seguirá dejando poner 1 | **SIGUE ABIERTO** [re-medido 02/09: los dos packs que leen esas constantes, `documentos_04` y `maestro_08`, leen **sólo el C++**]. **Un pack que lea los cuatro del `.cpp` y los cruce con `app.js`** — media hora, ya presupuestada |
| **P-8.bis** | 🟠 **`js/config.js` (`IOT_CONFIG`) es la MISMA duplicación en un cuarto sitio**: define PIN, baudio, límites de tiempo y UUIDs BLE, **y no lo llama nadie** porque `app.js` lleva su propia copia | `app_07_generadores_de_trama.py:94-101`, lista `HUERFANOS_CONOCIDOS` | Un módulo huérfano de constantes es peor que la duplicación sola: **parece la fuente y no lo es.** Quien lo edite creyendo cambiar el PIN no cambiará nada, y no habrá error | **Ya está medido y el huérfano vigilado** —una función que gane llamador y siga en la lista rompe `app_07`—. Lo que falta es lo de `P-8`: **atar los números**, o **retirar el módulo**. Lo que no puede quedarse es a medias |
| **P-9** | 🟠 **`LEEME_PRIMERO_APP.md` y `16_Documento_Auditoria...` son huérfanos totales** | 0 punteros | Uno de ellos se llama *«léeme primero»* y no hay ningún camino que lleve a él | **Enlazarlos o retirarlos**; lo que no puede quedarse es a medias |
| **P-10** | 🔴 **DOS instrumentos existen y no están en la compuerta** — `tests/test_e2e_visual.js` (4 ficheros lo nombran, uno de ellos lo llama *«Validación Visual E2E»*) y **`test_e2e_puppeteer.js`, que no lo nombra NADIE** | §1.10.bis de aquí, desde el 02/09. Antes: **en ninguna parte** | Es `CLAUDE.md` §3 en su forma más cara: **un hueco no grita.** Y el de puppeteer es peor que un hueco — **se traga toda excepción, sale con `0` e imprime *«TODOS LOS CONTROLES FUERON PROBADOS CON ÉXITO»***, que es N-46 literal esperando a que alguien lo conecte. Los dos miden a **412 px y sólo 412 px**, el único ancho donde el fallo de §4.ter no aparecía | **NO conectarlos tal cual**: no afirman nada, son capturadores. Decidir entre **retirarlos** o **convertir uno en un arnés que afirme** —anchos, contraste, recorte— y entonces sí darlo de alta, con la inyección de §8.bis delante. **`compuerta.py` no es de este documento** |

### 3.3 🟢 HECHOS ÚNICOS **CORRECTOS** — así debe ser, NO se duplican

> **Criterio:** vive en su documento propietario **y** un instrumento lo relee del fuente en cada
> corrida, o los demás documentos lo referencian en vez de copiarlo. **Duplicarlos crearía dos
> versiones que alguien tendría que sincronizar a mano** — el defecto que este repositorio lleva un
> mes cerrando.

| hecho | vive solo en | por qué está BIEN así |
|---|---|---|
| **Todo el contrato de bytes** (baudio, `TRAMA_MAX_UTIL`, buffers, la desigualdad del watchdog, los rangos del RTC) | `ESP32_Expansion/include/contrato.h` | **Es el ejemplo a seguir.** El propio fichero explica que cada número tiene un gemelo en otro lenguaje y que **`esp32_01` y `esp32_09` releen las tres fuentes en cada corrida**. Un cambio sin su gemelo pone la compuerta roja |
| **`W-4`, `P-2`, `P-3`, `R-4`, `R-9`** — las reglas de diseño del puente | `18_...ESP32.md` como identificadores | Cada una está **razonada dentro de `contrato.h`** junto a su constante, y la vigila su pack. El documento explica; el fuente manda |
| **«El puente NO ORIGINA»**, **«no parte ni une tramas»**, **«silencio no es orden»** | `18_...ESP32.md` §6.2–6.4 | Las tres son **barreras de arquitectura**… y las tres las vigila un pack (`esp32_05`, `esp32_06`, `esp32_08`). **El documento puede desaparecer y la barrera sigue de pie.** Es la diferencia exacta con P-1 |
| **La asimetría deliberada del checksum** | `18_...ESP32.md` §3.4 | Vigilada por `esp32_09_contrato_de_bytes` |
| **`SFTY-19`, `SFTY-24`, `SFTY-25`** (diseños **no implementados**) | `OPTIMIZACIONES.md` | Es su documento propietario, están declarados honestamente como *«sin construir»*, y no hay código que vigilar. **Copiarlos a un manual los convertiría en promesas** — que es lo que costó la Caja Negra de N-73 |
| **`SFTY6_SILENCIO_MS`, `DESPEJE_SEG_MIN/MAX`, `RF_BURST_COPIES`, `TIMEOUT_ACK_MS`** | un `.h` cada uno | **Sin valor por defecto y releídos en cada corrida.** La lección de N-71 fue justo la contraria: sacarlos de la prosa |
| **La línea `A1′` de compras** | `15_Lista_de_Compras_Hardware.md` | Documento propietario del dinero, con 11 punteros entrantes |
| **`costura_11_lcd_sin_bus`, `camara_02_j16`, los **once** `esp32_*`** citados en un solo documento | `OPTIMIZACIONES.md` / `18_...ESP32.md` | **El pack es el hecho.** Está en `compuerta.py`, corre y sabe fallar; que un documento lo mencione una vez o diez no cambia lo que mide. *(Eran nueve el 31/08; `esp32_10` y `esp32_11` entraron el 01/09)* |
| 🟢 **Los siete packs nuevos del 01–02/09 no están nombrados en `ARQUITECTURA.map` ni lo estaban aquí, Y ESO NO ES UN DEFECTO DE ELLOS** | sus propios `.py` | **Es la misma razón de la fila de arriba, y conviene decirla porque parece un hueco y no lo es.** `barrera_04`, `costura_12`, `documentos_05`, `enlace_02`, `esp32_10`, `esp32_11` y `reloj_01` corren en cada corrida y **saben fallar**; que un índice los nombre no cambia lo que miden. Se les ha dado fila arriba **para el lector**, no para sostenerlos. 🔴 **La asimetría que sí importa: lo que un índice sostiene de verdad son los hechos que NADIE relee** — y ésos son los de §3.2 |
| **El censo de pines completo** | `ARQUITECTURA.map` Anexo A | Es un índice, no una fuente: **cada fila lleva su `fichero:linea`**, y quien dude va al `.cpp`. Duplicarlo sería fabricar una segunda copia del `pines.h` |

---

## 4. 🔴 HUECOS ABIERTOS — lo que YA se perdió

**Confirmado con `grep` sobre todo el árbol el 31/08, con el control positivo de §3.1 delante.**
Estos cinco bloques **no tienen copia en ningún fichero del repositorio**. Se recogen aquí **solo
para que no se vuelvan a perder**; su sitio definitivo es **la especificación de la placa portadora,
que no existe todavía** y que el responsable encarga aparte. **Aquí no se escriben en ningún manual:
no son ficheros de este encargo.**

Todos vivían en `05_Funcional/Guia_Cableado_y_Pruebas_Banco.html`, añadidos en `f10f4d4` y retirados
en `fa66710` al reescribir la guía **para el técnico**. La retirada fue **correcta de destinatario**
—un funcional que va a cablear no necesita una barrera de arquitectura— y **equivocada de
procedimiento**: no había otro sitio donde ponerlas.

### H-1 · 🔴 «Lo que esta placa NO lleva» — **es una barrera de arquitectura, del rango de §6**

Lo que decía, recuperado literal del commit `fa66710`:

* **Nada que escriba sobre las luces.** Ni relés de lámpara, ni salidas de potencia, ni un hilo hacia
  `J3`–`J9` o `J11`, ni hacia **`J15`** (la barrera), ni hacia **`J14`** (la cámara), ni hacia
  **`J13`** (el buzzer). **Ni un hilo a `J16`** —ni para señal ni para tomar de ahí los 12 V—.
* **El porqué, que no cambia porque haya una placa nueva:** solo `semaforo.cpp` escribe pines de luz,
  y todo pasa por su `escribirPines()`. **Una placa accesoria no reabre la barrera de salidas.**
  El ESP32 **no manda sobre el semáforo: PIDE**, por el puerto serie, y el STM32 acepta o rechaza con
  su `$ACK` o su `$ERR`.
* **Tampoco lleva nada que reinicie al STM32.** Ni una línea de *reset*, ni un watchdog externo sobre
  la tarjeta: los dos STM32 ya tienen el suyo a 4 s, y **un accesorio con capacidad de reiniciar al
  controlador del semáforo es exactamente el reparto que esta arquitectura evita.**
* **Y no lleva batería propia para el ESP32.** Si se va la energía de la caja, el semáforo se apaga:
  mantener vivo el módulo de diagnóstico mientras el cruce está muerto no resuelve nada y añade una
  fuente más dentro del armario. **Lo único que sobrevive al corte es el reloj, con su pila.**
* Y la frase que explica por qué la lista tenía que estar escrita: *«lo que una placa nueva **no**
  lleva no deja rastro en el cobre: un hueco no grita, y dentro de un año nadie sabrá si faltó o si
  se decidió»*.

> **Es del mismo rango que la barrera de salidas de `CLAUDE.md` §6**, y hoy no está en ningún sitio.

### H-2 · 🔴 Los puntos de prueba — **requisito de diseño, no comodidad**

**El módulo va SOLDADO**, y por eso las dos cosas que habrá que hacerle con el tiempo —medirle el TX
y volver a programarlo— **hay que diseñarlas ahora**, o se acaban haciendo con una punta de soldador
sobre una placa montada dentro de un armario.

| punto | qué es | para qué |
|---|---|---|
| **`TP1`** | `GPIO17` · **TX2** del ESP32 | **Es la medida `M5`**: en reposo debe dar **3,3 V** (línea serie en reposo alta). 🔴 **Si diera 5 V, el módulo no es el que se cree que es** |
| **`TP2`** | `GPIO16` · **RX2** | ver que el hilo que viene del micro está donde se cree |
| **`TPG`** | **masa, al lado de los otros dos** | **Una medida de tensión necesita dos puntas.** Un punto de prueba sin su masa al lado **no es un punto de prueba**: obliga a buscar masa en otro sitio con la placa energizada, que es como se resbala una punta |
| **`TPV`** | salida de la fuente y su masa | para **`M6`**: medir la salida en vacío y con el módulo transmitiendo, **sin desmontar nada** |

**Pads accesibles con punta de multímetro con la placa montada** —no vías tapadas por el módulo, no
pads debajo de un conector—, **rotulados en la serigrafía**. *«Si no está rotulado, dentro de seis
meses nadie sabrá cuál es cuál.»*

### H-3 · 🔴 El acceso de reflasheo sin desoldar — **son DOS casos, y dependen del chip**

* **Si el módulo es una placa DevKitC**, ya trae su **conector USB**: el requisito es **mecánico**
  —que ese conector quede **accesible con la placa montada en la caja**, no tapado por el conector de
  12 V, ni por el mazo hacia `J17`, ni contra una pared del armario—. **Se comprueba al dibujar la
  placa, no después.**
* **Si es un `WROOM` pelado**, no hay USB y hay que sacar a una tira de prueba: **`EN`, `IO0`,
  `TX0` (`GPIO1`), `RX0` (`GPIO3`), `3V3` y `GND`**. 🔴 **Y no se reutilizan `GPIO16`/`GPIO17`**:
  están ocupados por el enlace con el semáforo.
* 🔴 **La precaución, que es la de `J16` p1 aplicada aquí:** `EN` e `IO0` son los que meten al módulo
  en modo de descarga. **Una tira accesible con esos dos pines es una tira que un destornillador
  puede pisar**, y el módulo se quedaría en modo de programación —mudo— sin que nada lo indique. Van
  donde no se pisen, y **si llevan pulsador, que no sobresalga**.
* **El procedimiento de las dos fuentes, porque es donde se rompe:** **antes de enchufar el USB se
  corta la entrada de 12 V** (o se abre el jumper de aislamiento). **Serigrafiado en la placa.**

### H-4 · 🟠 La medida `M6` y la regla de trazado

* **`M6`** —riel de 12 V, salida de la fuente bajo pico, temperatura del regulador a los 30 min—
  **no aparece hoy en ningún fichero** (`grep "M6"` → cero, con `M3` dando 15 como control positivo).
  Iba encabezada por el aviso de que **hoy no se puede rellenar**: la respuesta correcta es marcar
  *«NO SE PUDO MEDIR»* con motivo *«placa no construida»*. **Una hoja que solo admite bien/mal fuerza
  a inventar.**
* **La regla de trazado**, que sale de un hallazgo medido: en la tarjeta actual el margen contra los
  12 V es de `1,36 mm` **y ya está fabricada**; en una placa que se dibuja de cero **eso se elige**.
  **Entrada de 12 V y regulador en un extremo; señales de 3,3 V y conector a `J17` en el otro.**

### H-5 · 🔴 Que la placa portadora **existe como objeto pendiente**

`grep -i "portadora"` sobre **todo** el árbol devuelve **dos frases de prosa en `roadmap.md`**
(`:149` y `:257`), y una de ellas es precisamente la nota de que estos requisitos se perdieron. **No
hay un solo documento que diga que hay una placa que construir**, ni que
**quién la diseña y quién la fabrica no está decidido**, ni que **no es todavía una línea de
compras**.

> 🛑 **Y `BLQ-1`, aunque esté cerrado para la app, dejó dicho algo que sigue vigente: la huella del
> cobre.** Un `ESP32-WROOM-32` y un `ESP32-S3-WROOM-1` tienen **footprint y pinout distintos**. Ese
> razonamiento tampoco tiene copia. *(Con `BLQ-1` cerrado el 31/08 la respuesta ya se conoce; lo que
> se pierde es el **porqué**, que es lo que impide volver a equivocarse.)*

---

## 5. Lo que este índice NO cubre, y nadie debe dar por cubierto

* **El cobre.** No hay una sola fila *«VERIFICADO EN LA PLACA»* en todo el proyecto, y este fichero
  no inaugura la primera. Todo lo que aquí se llama **[MEDIDO]** se midió sobre **ficheros**.
* **El banco.** V9.0 no ha estado nunca en una tarjeta. **Nada de lo que este índice ordena lo
  sustituye.**
* **Las cifras.** Aquí no hay ninguna del acta, a propósito. La vigente está en `evidencia/`.
* **Los ficheros en vuelo.** Seis agentes escribían mientras esto se medía. Las **rutas** son
  estables; las **líneas** de `app.js`, `lcd.cpp`, `OPTIMIZACIONES.md`, los manuales 2/12/14 y los
  packs pueden haberse movido.
* 🔴 **Las citas `fichero:linea`, y esto lo cobró la pasada del 02/09.** Se verificaron una a una las
  del hermano `ARQUITECTURA.map`: **las 36 rutas aguantaron** —tienen vigilante, la guarda de rutas
  de la compuerta— y **seis citas de línea apuntaban a código que se había movido**, una de ellas
  *acusando de caducado un comentario que ya no estaba ahí*. Las de **este** fichero no se han
  revisado enteras y **hay que leerlas con la misma desconfianza**. Lo que no se mueve es *en qué
  fichero* vive cada cosa, que es lo que este índice contesta; el número de línea es una comodidad,
  no un dato vigilado.
* **Este fichero no se vigila solo.** No hay ningún pack que compruebe que sigue siendo cierto, y
  **eso es deliberado**: un pack que exigiera que cada hecho esté en N ficheros forzaría a duplicar,
  que es el defecto contrario. Lo que sí se puede escribir algún día, y sería barato, es un pack que
  vigile **una sola propiedad**: que **ningún documento de `05_Funcional/` tenga cero punteros
  entrantes**. Ese sí es un trinquete —lo que falla es un huérfano **nuevo**—, y habría cazado los
  81 KB de la guía de cableado y los cuatro huérfanos de §2.
