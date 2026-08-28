# 🛒 MANUAL 15: LISTA DE COMPRAS DE HARDWARE — QUÉ PEDIR, CUÁNTO Y CUÁNDO

**Sistema:** Controladora de Semáforos Móviles de 3 Estados (V9.0)
**Fecha:** 27 de Agosto de 2026
**Última revisión:** 28 de Agosto de 2026 (2.ª del día) — 🔴 **corrige la bornera de la talanquera
en A4: decía `J14`, y `J14` es una ENTRADA del micro**; avisa de que el `DS3231` de B1 **no tiene
driver**; arregla las referencias a las secciones del Manual 13. *(1.ª revisión del 28/08: bloque 0
con el estado real de lo recibido —llegaron ESP32, no `HC-05`—, criterio de compra del módulo
Bluetooth y corrección de `JDY-31` → `JDY-30`.)*
**Para:** el funcional / quien autoriza la compra

---

> [!CAUTION]
> ### 🩹 FE DE ERRATAS DEL 28/08/2026 — 🔴 **LA FILA A4 MANDABA CABLEAR LA TALANQUERA A `J14`**
>
> **Si ya se cableó un módulo de relé de talanquera a `J14` siguiendo la versión anterior de esta
> lista: DESCONECTARLO ANTES DE ENERGIZAR.** No es un error de compra —el relé pedido es el
> correcto—, es un error de **dónde se conecta**.
>
> | | decía la lista | es |
> |---|---|---|
> | Bornera | **`J14`** ❌ | **`J15`** ✅ |
> | Red | `Puerta` ❌ | `Motor` ✅ |
> | Pin | — | `PB2` (`MOTOR_TALANQUERA`) |
>
> * **`J14` es la ENTRADA de la cámara de demanda** (`PB0`, `CAM_DEMANDA_PIN`), con `R64` 10 kΩ y
>   `C25` 100 nF de antirrebote. **Meterle los 12 V de un relé destruye el STM32**, que es el micro
>   que gobierna el semáforo — y además la pluma no se movería, porque `J14` no tiene etapa de
>   salida.
> * **La talanquera SALE por `J15`:** `PB2` → opto `U15` → MOSFET `Q10` → `J15`. Es un canal de
>   potencia propio, ya montado en la placa.
> * **Medido** el 28/08 en `01_Firmware/Maestro/include/pines.h` (líneas 31 y 46). El **Manual 13
>   §3 siempre lo dijo bien**; lo que estaba mal era la §5 de aquel manual y esta fila A4, y las dos
>   se han corregido hoy.
> * **La errata no se borra: queda escrita con su fecha**, porque quien haya cableado siguiendo la
>   versión vieja tiene que poder enterarse.
>
> **De dónde salió el error, para no repetirlo:** la red de `J14` se llama **`Puerta`** en el
> esquemático, y «puerta» suena a talanquera. **No lo es: es por donde la cámara pide paso.**

> ### Por qué existe este documento, y qué NO es
>
> La compra estaba repartida en **siete manuales** (el 4 las radios, el 7 las antenas, el 9 las
> cámaras, el 10 los módulos Bluetooth, el 11 la pila del reloj, el 13 la placa de expansión). Un
> listado repartido es como se olvida una línea, y como se compra dos veces la misma.
>
> **Esto es un índice con cantidades y decisión, no una copia de las especificaciones.** De cada
> línea, la especificación completa —modelo exacto, conector, tolerancias— vive en su manual, y ahí
> se lee antes de pedir. Copiarla aquí crearía una segunda versión que alguien tendría que
> sincronizar a mano, que es exactamente el defecto que este proyecto lleva un mes cerrando.

> ### 🛑 La regla que decide qué entra en esta lista
>
> **No se compra hardware para una función que el firmware no ejecuta.** Se ha pagado dos veces por
> saltársela: `PB8` estuvo en cuatro manuales como *«umbral de tramo»* con el pin **sin leer** (N-59),
> y la placa de expansión iba a llevar chip para dos funciones que **ya están en el cobre** de la
> tarjeta (N-63). Por eso hay tres bloques y no uno.

---

## 0 · ESTADO REAL DE LAS COMPRAS al 28/08 — leer antes de volver a pedir

> **Esta lista describía lo que hay que pedir; le faltaba decir qué llegó.** Sin esa columna se
> vuelve a comprar lo que ya está, y —lo que pasó— se da por cubierta una línea que no lo está.

| Línea | Qué se pidió | Qué hay de verdad hoy | Estado |
|---|---|---|---|
| **A1** Módulo Bluetooth SPP | 2 × `HC-05` / `JDY-30` | **Llegaron módulos `ESP32`**, referencia **sin confirmar** | 🔴 **NO cubierta.** Ver el aviso de abajo |
| **A2** Cámaras de demanda | 2 × AcuSense | sin novedad | pendiente *(confirmar si ya hay una en almacén)* |
| **A3** Antenas + coaxiales | 2 + 2 | sin novedad | pendiente |
| **A4** Módulos de 1 relé | 2 | sin novedad | pendiente |
| **B1–B4** Bloque de expansión / RTC | — | **NO se compraron** | correcto: el bloque B **espera al banco**, no se pide todavía |

### 🔴 Los ESP32 que llegaron NO sustituyen a los `HC-05` sin comprobarlo antes

Un `ESP32` **no es un módulo Bluetooth SPP por definición**. Según la familia de silicio:

| Familia | ¿Habla SPP? |
|---|---|
| `ESP32-WROOM-32` · `-32D` · `-32E` · `-32U` (ESP32 clásico) | ✅ **Sí** — Bluetooth Clásico (BR/EDR): la app conecta tal cual |
| `ESP32-S3` · `ESP32-C3` | ❌ **No** — solo BLE. La app **no conectará nunca** |
| `ESP32-S2` | ❌ **No** — sin radio Bluetooth, solo WiFi |

**La rotulación del vendedor no distingue las tres:** todas se anuncian *«WiFi + BT · SoC · ISM
2.4G · 802.11»*, y esa cadena dice que hay radio de 2,4 GHz, **no** que haya Bluetooth Clásico.
**No sirve como criterio de compra ni como comprobante de recepción.**

**Cómo se resuelve —dos formas, las dos miran el chip y no el anuncio:**

1. Leer la **serigrafía del blindaje metálico** del módulo (`ESP32-WROOM-32E`, `ESP32-S3-WROOM-1`, …).
2. Preguntárselo al chip, con el módulo por USB:
   ```text
   python "C:/.platformio/packages/tool-esptoolpy/esptool.py" --port COM# chip_id
   ```
   `esptool.py v4.11.0` ya está instalado con PlatformIO en la máquina de trabajo.

> ⏳ **PENDIENTE, y bloquea la decisión de A1:** hasta que se anote aquí la referencia leída por uno
> de esos dos métodos, **no se sabe si lo que llegó sirve para algo**. No se apunta como cubierto ni
> como descartado.

**Y aunque salgan ESP32 clásicos, siguen sin ser un cambio gratis:** un ESP32 con WiFi da picos de
**~500 mA**, y la alimentación de la tarjeta (`12 V → LM7805 → LM1117-3.3`) no los da: el `7805`
disiparía 3,5 W sin disipador y, si el riel de 3,3 V se hunde, **se reinicia el STM32 que gobierna
el semáforo**. Si se usan, van con **fuente propia desde los 12 V y masa común, alimentación NO
compartida** (Manual 10 §1 y §2). Un `HC-05` (~40 mA) sí se alimenta de la placa.

### ✅ Decisión de obra del 28/08

**Se sigue pidiendo el módulo Bluetooth SPP dedicado (`HC-05` / `JDY-30`). El ESP32 queda como
alternativa, no como sustituto.** Motivo: el módulo dedicado mantiene intacta la decisión congelada
del **Manual 10 §1** —SPP, no BLE— y **no obliga a rehacer el puente nativo de Android**.

* Un **ESP32 clásico** haciendo de puente SPP sería aceptable sin tocar el Manual 10, confirmada la
  referencia y con su fuente propia.
* Ir por **WiFi** —que es lo único que queda si los módulos resultan `S3`, `C3` o `S2`— **exige
  reabrir por escrito el apartado 1 del Manual 10 antes de comprar y antes de programar**. Lo exige
  ese mismo manual, y no se ablanda: la vez que se cambió de transporte sin escribirlo, el resultado
  fue una app que no conectaba con nada.

---

## A · SE PIDE YA — no depende de nada

| # | Qué | Cant. | Para qué | Especificación en |
|:---:|---|:---:|---|---|
| A1 | **Módulo Bluetooth SPP** `HC-05` / `JDY-30` (Bluetooth **clásico**, no BLE), 9600 bps en modo datos. **Un `ESP32` solo vale si es de la familia clásica — ver el bloque 0** | **2** | Consola de servicio por celular en cada poste. Es lo que evita subir con escalera al Esclavo | **Manual 10** §1 y §2 |
| A2 | **Cámara IA** Hikvision AcuSense varifocal motorizada `DS-2CD3643G2-LIZSU` **o equivalente** | **2** *(ver nota)* | Demanda vehicular: una por poste, contacto seco a `PB0`. **Son las dos que el firmware lee hoy** | **Manual 9** |
| A3 | **Antenas VHF y coaxiales** | **2 + 2** | Recuperar alcance: las genéricas de «LoRa» costaban 15–20 dB y dejaban la cobertura en 3 cuadras | **Manual 7** §BOM *(lleva modelo, conectores y adaptadores)* |
| A4 | **Módulo de 1 relé optoacoplado, con jumper `JD-VCC`** | **2** *(1 por poste)* | **La talanquera.** El firmware ya la manda (SFTY-28, 27/08) y la tarjeta ya expone la señal: se conecta a la bornera **`J15`** (red `Motor`, `PB2` → opto `U15` → MOSFET `Q10`). 🔴 **NO a `J14`, que es la ENTRADA de la cámara — ver la fe de erratas de la cabecera.** **No hace falta `PCF8574` ni MOSFET nuevo** | **Manual 13** §3 *(la etapa de potencia y el cuadro `J14`/`J15`)*; el jumper `JD-VCC` y su porqué, en el aviso del bloque **B** de esta misma lista |

> ✏️ **Corrección en A1 (28/08): decía `JDY-31`, y el `JDY-31` está PROHIBIDO en el Manual 10.** La
> lista pedía un módulo que la decisión congelada excluye por nombre, en la misma línea que dice
> «no BLE». Es el modelo `JDY-30`. **Manda el Manual 10 §1**, que es donde vive la especificación.
>
> 🔧 **Y A1 no acaba en la compra: los dos módulos hay que renombrarlos con `AT+NAME` ANTES de
> instalarlos** (`SEM-<SERIE>-M` y `SEM-<SERIE>-E`), o el técnico verá dos `HC-05` idénticos en la
> lista de Android y no sabrá qué poste es cuál. El procedimiento —modo AT a **38400 bps**, modo
> datos a **9600 bps**— está en el **Manual 10 §1**.

> **Nota sobre la cantidad de cámaras.** El diseño habla de **cuatro** (dos por poste: demanda y
> umbral), pero **el firmware solo lee las de demanda**: las de umbral quedaron retiradas en N-59
> porque el protocolo no tiene comando para mandar la cuenta del tramo al Maestro. **Las de umbral se
> piden cuando exista ese comando** (tarea `C1` de `ESTADO.md`), no antes.
>
> `ESTADO.md` venía pidiendo **3** sin que ningún manual explique por qué —probablemente porque ya hay
> una comprada—. **Eso lo confirma el responsable antes de pedir:** si ya hay una en almacén, son 1 o
> 2; si no hay ninguna, son 2.

---

## B · ESPERA AL BANCO — una sola pregunta lo decide

**La pregunta:** *¿en qué tarjeta está muerto el cristal `Y2` de 32.768 kHz?* Se responde en la sesión
de banco (tarea `B5`), y hasta entonces **no se pide nada de este bloque**:

| si el cristal muerto está en… | qué hace falta |
|---|---|
| el **Esclavo** | **NADA.** Ya toma la hora del Maestro por radio (`CMD_HORA_*`, SFTY-23). Cero pesos |
| el **Maestro** | ahí sí: es quien fija la hora, y necesita fuente propia → todo el bloque de abajo |

| # | Qué | Cant. | Especificación en |
|:---:|---|:---:|---|
| B1 | **Módulo RTC `DS3231` `ZS-042`** 🔴 **sin driver: no dará la hora al enchufarlo** | 1 | **Manual 13** §5 *(la tabla de compras)* y **Manual 11** |
| B2 | **Pila `LIR2032`** (Li-ion 3,6 V **recargable**) | 1 | **Manual 13** §5 — ⚠️ ver aviso abajo |
| B3 | **`PCF8574P`** (DIP-16) + zócalo, si además se quiere expandir | 1 | **Manual 13** §5 — ⚠️ **no** el `PCF8574A` |
| B4 | Optos `PC817`, módulo de relé 12 V, resistencias, borneras, placa perforada | ver manual | **Manual 13** §5 *(qué se pide)* y §6 *(cómo se monta)* |

> ✏️ **Referencias cruzadas corregidas el 28/08 (2.ª rev.).** Estas cuatro filas citaban «§4.1» y
> «§4.2» del Manual 13, y la fila **A4** citaba «§2». Eran los números **anteriores a la
> reestructuración de ese manual**: hoy §4.1 es el censo de pines, §4.2 son las rutas del bus y §2
> es la naturaleza del bus — ninguna de las tres es donde vive lo que se estaba citando. **Las
> piezas se piden en §5; el montaje está en §6; la talanquera, en §3.**
>
> ⚠️ **Y un hueco que se deja anotado en vez de inventarle una sección:** el detalle del **jumper
> `JD-VCC`** (retirarlo, lógica a 3,3 V, bobina a 12 V) **no está en ninguna sección del Manual 13**
> — se comprobó buscándolo. La única descripción que existe es el tercer aviso de aquí abajo. Se
> apunta para que alguien la lleve al Manual 13, no para taparlo con una referencia falsa.

> 🔴 **PRIMERO, LO QUE NO ES UNA AVERÍA:** el **`DS3231` no tiene driver en el firmware**. Medido el
> 28/08: `grep -rniE "DS3231|Wire\.|0x68"` sobre `01_Firmware/Maestro/{src,include}` y
> `01_Firmware/Esclavo/{src,include}` da **cero coincidencias de código**. **Al enchufarlo no dará
> la hora: no hay código que le hable. Eso no es una avería, ni del módulo ni del montaje** — no se
> devuelve al mostrador ni se busca el fallo en la soldadura. Se compra para tenerlo cuando llegue
> el firmware V9.0 (`roadmap.md` N-54 / N-55). **Lo mismo vale para el `PCF8574` de B3.**

> 🔴 **Tres avisos que cambian lo que se compra, no solo cómo se monta:**
>
> - **`PCF8574` sí, `PCF8574A` no.** Misma patilla, dirección base `0x38` en vez de `0x20`. Si llega
>   el «A», el firmware no lo encuentra y parece que la placa está mal soldada.
> - **Pila `LIR2032` recargable, con el circuito de carga del módulo intacto.** Si en la tienda solo
>   hay `CR2032`, sirve — pero entonces **hay que desoldar `D1` o `R1`**, porque una `CR2032` no es
>   recargable y cargarla es como se hincha o ventea. **Pila y modificación van juntas.**
> - **Módulo de relé con jumper `JD-VCC`.** Sin él no hay aislamiento y no se puede alimentar la
>   lógica a 3,3 V: se destruye el expansor al primer montaje. Si el que venden no lo trae, hace falta
>   además un transistor `2N2222` y una resistencia de 1 kΩ.

---

## C · YA EN SERVICIO — no se pide, se verifica

| Qué | Estado | Dónde se comprueba |
|---|---|---|
| **2 radios `E90-DTU`** en enlace directo, `2.4 kbps`, `M0`/`M1` en OFF | en servicio desde el 01/08 | **Manual 4** |
| Radio **B1** | **averiada y retirada** (transmisor). Si se quiere repuesto, es la misma referencia | `roadmap.md` |
| Repetidor ESP32 | **fuera de la configuración vigente** (enlace directo, sin repetidor). ⚠️ **No confundir con los ESP32 llegados el 28/08**: aquel es el puente de radio del Manual 5, y no libera la línea A1 | **Manual 5** |
| Pila de `VBAT` en la tarjeta, con `R5` retirada | montada | **Manual 11** *(es OTRA pila distinta de la B2)* |

---

## Resumen para autorizar

**Se pide hoy:** 2 módulos Bluetooth SPP `HC-05` / `JDY-30` · 2 cámaras AcuSense de demanda
*(confirmar si ya hay una)* · 2 antenas VHF con sus 2 coaxiales · 2 módulos de 1 relé con jumper
`JD-VCC`.

> **Los módulos Bluetooth siguen en la lista aunque hayan llegado ESP32.** Decisión de obra del
> 28/08: se va con el módulo SPP dedicado. Los ESP32 solo dejarían de hacer falta pedir los `HC-05`
> si se confirma que son de la **familia clásica** (serigrafía o `chip_id`) **y** se les monta
> **fuente propia de 12 V** — y ninguna de las dos cosas está hecha. Ver el bloque **0**.

**Se pide después del banco, y solo si el cristal muerto es el del Maestro:** el `DS3231` con su
`LIR2032`, y el `PCF8574P` con su placa si se decide expandir. ⚠️ **Ninguna de las dos piezas tiene
driver todavía: se compran preparadas, no funcionando.**

**No se pide:** cámaras de umbral, `PCF8574` para talanqueras ni para cámaras. Las talanqueras salen
por la salida **`Motor` (bornera `J15`, MOSFET `Q10`)** que **la tarjeta ya trae**, y las cámaras de
demanda por `PB0` (bornera `J14`), que ya se lee.

> ✏️ **Corregido el 28/08 (2.ª rev.):** este párrafo decía *«las talanqueras salen por la salida
> `Puerta`»*. **`Puerta` es la red de ENTRADA de la cámara** (`J14`), no la de la pluma. La salida de
> la talanquera es la red **`Motor`** (`J15`). Es la misma errata de la fila A4, y aquí estaba
> repetida con el nombre de la red en vez del de la bornera — que es justo como una corrección se
> deja a medias. Ver la fe de erratas de la cabecera.
