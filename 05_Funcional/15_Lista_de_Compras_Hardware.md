# 🛒 MANUAL 15: LISTA DE COMPRAS DE HARDWARE — QUÉ PEDIR, CUÁNTO Y CUÁNDO

**Sistema:** Controladora de Semáforos Móviles de 3 Estados (V9.0)
**Fecha:** 27 de Agosto de 2026
**Última revisión:** 28 de Agosto de 2026 (3.ª del día) — 🔄 **la arquitectura cambió en obra: el
`ESP32` SUSTITUYE al módulo Bluetooth SPP, el `DS3231` deja de ir en la placa STM32 y se cuelga del
`ESP32`, y se retiran la pantalla LCD, los cuatro pulsadores y el mando de relés.** Lo que ya no se
compra queda **tachado con su motivo**, no borrado. *(2.ª rev. del 28/08: corrige la bornera de la
talanquera en A4 —decía `J14`, y `J14` es una ENTRADA del micro—; avisa de que el `DS3231` no tiene
driver; arregla las referencias a las secciones del Manual 13. 1.ª rev.: bloque 0 con el estado real
de lo recibido —llegaron ESP32, no `HC-05`—, criterio de compra del módulo Bluetooth y corrección de
`JDY-31` → `JDY-30`.)*
**Para:** el funcional / quien autoriza la compra

---

> [!CAUTION]
> # 🔄 28/08/2026 — LA ARQUITECTURA CAMBIÓ. ESTO SE LEE ANTES QUE EL RESTO DEL DOCUMENTO
>
> **El `ESP32` pasa a SUSTITUIR al módulo Bluetooth SPP —ya no se compran `HC-05` ni `JDY-30`—, el
> `DS3231` se cuelga del `ESP32` por I²C con pila propia en vez de montarse en la placa STM32, y se
> retiran la pantalla LCD, los cuatro pulsadores y el mando de relés.**
>
> **Todo lo que aparece ~~tachado~~ más abajo es de la arquitectura anterior: no se compra, y lleva
> el motivo escrito al lado.** No se borra a propósito — una línea borrada vuelve a proponerse
> dentro de un mes y nadie recuerda que se descartó; una línea tachada con su porqué, no.

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
> tarjeta (N-63). Por eso hay bloques separados y no una lista sola: **A** lo que se pide, **B** lo
> que espera al banco, **C** lo que ya está en servicio y **D** —nuevo el 28/08— **lo retirado, que
> se escribe en vez de borrarse**.

---

## 0 · ESTADO REAL DE LAS COMPRAS al 28/08 — leer antes de volver a pedir

> **Esta lista describía lo que hay que pedir; le faltaba decir qué llegó.** Sin esa columna se
> vuelve a comprar lo que ya está, y —lo que pasó— se da por cubierta una línea que no lo está.

| Línea | Qué se pidió | Qué hay de verdad hoy | Estado |
|---|---|---|---|
| ~~**A1**~~ Módulo Bluetooth SPP | ~~2 × `HC-05` / `JDY-30`~~ | **nunca llegaron, y ya no se piden** | ⛔ **ANULADA el 28/08.** El `ESP32` los sustituye |
| **A1′** `ESP32` como módulo SPP | — *(línea nueva del 28/08)* | **Llegaron módulos `ESP32`**, referencia **sin confirmar** y **cantidad sin anotar** | 🛑 **BLOQUEADA.** No se compra ni un `ESP32` más hasta leer qué referencia llegó. Ver abajo |
| **A2** Cámaras de demanda | 2 × AcuSense | sin novedad | pendiente *(confirmar si ya hay una en almacén)* |
| **A3** Antenas + coaxiales | 2 + 2 | sin novedad | pendiente |
| **A4** Módulos de 1 relé | 2 | sin novedad | pendiente |
| **A5** Fuente propia del `ESP32` | — *(línea nueva del 28/08)* | **NO se ha pedido, y hace falta** | 🔴 **NO cubierta.** Sin ella el `ESP32` reinicia el STM32 del semáforo |
| **A6** `DS3231` colgado del `ESP32` | — *(sale del bloque B)* | **NO se compró** | pendiente. **Ya no espera al banco**: no va en la placa STM32 |
| **A7** Conexión de cámaras a `J16` | — *(línea nueva del 28/08)* | **NO se ha pedido** | pendiente *(es cable y conector, no electrónica)* |
| ~~**B1–B2**~~ RTC en la placa STM32 | — | **NO se compraron** | ⛔ **ANULADAS.** El RTC se mudó a **A6**, colgado del `ESP32` |
| **B3–B4** Expansor y accesorios | — | **NO se compraron** | correcto: siguen **esperando al banco**, no se piden todavía |

### 🛑 Antes de comprar un `ESP32` más: NO todos hablan SPP, y el grande es el que no

Un `ESP32` **no es un módulo Bluetooth SPP por definición**. Según la familia de silicio:

| Familia | ¿Habla SPP? |
|---|---|
| `ESP32-WROOM-32` · `-32D` · `-32E` · `-32U` (ESP32 clásico) | ✅ **Sí** — Bluetooth Clásico (BR/EDR): la app conecta tal cual |
| `ESP32-S3` · `ESP32-C3` | ❌ **No** — solo BLE. La app **no conectará nunca** |
| `ESP32-S2` | ❌ **No** — sin radio Bluetooth, solo WiFi |

> ## ⛔ «PIDE EL ESP32 MÁS GRANDE / EL MÁS NUEVO» ES EXACTAMENTE COMO SE ACABA CON UN `S3` SIN SPP
>
> **El `S3` es más nuevo, más rápido, tiene más pines y más memoria que el `WROOM-32` clásico — y no
> habla SPP.** Quien pida «el mejor», «el más grande» o «el más moderno» va a recibir un `S3` o un
> `C3`, y **la app no conectará jamás**: no es un ajuste, no es un driver que falte, es que el
> silicio no lleva Bluetooth Clásico. En este equipo, **el bueno es el viejo**.
>
> **Lo que se pide, escrito para copiar y pegar en el pedido:**
>
> ```text
> ESP32-WROOM-32  (o -32D / -32E / -32U)  sobre placa DevKitC de 38 pines
> Bluetooth Clasico BR/EDR + WiFi.  NO S3, NO C3, NO S2, NO C6, NO H2.
> ```
>
> **Los 38 pines son de la placa, no del chip:** es el mismo `WROOM-32` con SPP, pero saca más GPIO
> al conector — que es lo que hace falta ahora que del `ESP32` cuelga también el reloj de A6. La
> versión de 30 pines sirve igual si es la que hay, siempre que saque `GPIO21` y `GPIO22`.

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

> 🛑 **PENDIENTE, y es lo que BLOQUEA la línea A1′:** hasta que se anote aquí la referencia leída por
> uno de esos dos métodos, **no se sabe si lo que llegó sirve para algo**. No se apunta como cubierto
> ni como descartado. **Y mientras tanto no se compra ni un módulo más de Bluetooth**, ni `ESP32` ni
> nada: comprar antes de saber es como se acaba con dos referencias distintas en el mismo cruce.
>
> **Se anota aquí, con fecha y quién lo leyó:**
>
> ```text
> Fecha: ____________   Leido por: ____________________
> Metodo: [ ] serigrafia del blindaje   [ ] esptool chip_id
> Referencia leida: ______________________________
> Cuantos modulos hay en almacen: ______
> Habla SPP:  [ ] SI -> sigue A1'   [ ] NO -> ver la fila de WiFi de abajo
> ```

**Y aunque salgan ESP32 clásicos, siguen sin ser un cambio gratis:** un ESP32 con WiFi da picos de
**~500 mA**, y la alimentación de la tarjeta (`12 V → LM7805 → LM1117-3.3`) no los da: el `7805`
disiparía 3,5 W sin disipador y, si el riel de 3,3 V se hunde, **se reinicia el STM32 que gobierna
el semáforo**. Si se usan, van con **fuente propia desde los 12 V y masa común, alimentación NO
compartida** (Manual 10 §1 y §2). Un `HC-05` (~40 mA) sí se alimenta de la placa. **Eso es la línea
A5, y hoy no está pedida.**

### 🔄 Decisión de obra del 28/08 — VIGENTE: el `ESP32` SUSTITUYE al módulo SPP

**Ya no se compran `HC-05` ni `JDY-30`. El `ESP32` es el módulo**, y además de la consola por celular
lleva colgado el reloj (A6). Con él se retiran la pantalla LCD, los cuatro pulsadores y el mando de
relés, y las cámaras pasan a los pines que el mando deja libres en `J16` (A7).

> ~~**Se sigue pidiendo el módulo Bluetooth SPP dedicado (`HC-05` / `JDY-30`). El ESP32 queda como
> alternativa, no como sustituto.**~~ ⛔ **ANULADO el 28/08 por la decisión de obra de arriba.**
> Era la decisión de la 1.ª revisión de ese mismo día, y su motivo era bueno —el módulo dedicado
> deja intacta la decisión congelada del Manual 10 §1 y no obliga a rehacer el puente nativo de
> Android—. **Queda tachado y no borrado** para que quien lo relea dentro de un mes vea que se
> descartó a propósito, y no lo vuelva a proponer como novedad.

**Lo que NO cambia con la decisión nueva, y hay que mirar antes de gastar:**

* Un **ESP32 clásico** haciendo de puente SPP **cabe dentro del Manual 10 §1 sin reabrirlo** —sigue
  siendo Bluetooth Clásico SPP, que es lo que ese apartado congela—, **con dos condiciones: la
  referencia confirmada y su fuente propia**. Las dos siguen sin cumplirse.
* Ir por **WiFi** —que es lo único que queda si los módulos resultan `S3`, `C3` o `S2`— **exige
  reabrir por escrito el apartado 1 del Manual 10 antes de comprar y antes de programar**. Lo exige
  ese mismo manual, y no se ablanda: la vez que se cambió de transporte sin escribirlo, el resultado
  fue una app que no conectaba con nada.

> ⚠️ **Y de ahí sale por qué el bloqueo de A1′ no es un trámite:** la decisión del 28/08 sustituye un
> módulo por otro **dando por hecho que el que llegó habla SPP**. Si resulta que no, lo que hay
> encima de la mesa no es una compra pendiente: es **un cambio de transporte** que hay que reabrir
> por escrito. **La misma lectura de serigrafía decide las dos cosas.**

---

## A · SE PIDE YA — no depende de nada, salvo la línea marcada 🛑

> **Una sola fila de este bloque está bloqueada, y se marca en vez de sacarla:** `A1′`, el `ESP32`,
> espera a que alguien lea qué referencia llegó (bloque **0**). **El resto se pide sin esperar a
> nada** — y eso incluye `A5`, `A6` y `A7`, que son nuevas del 28/08.

| # | Qué | Cant. | Para qué | Especificación en |
|:---:|---|:---:|---|---|
| ~~A1~~ | ~~**Módulo Bluetooth SPP** `HC-05` / `JDY-30`~~ ⛔ **NO SE COMPRA.** El `ESP32` lo sustituye (decisión de obra del 28/08). *La fila no se borra: un hueco se vuelve a proponer, una fila tachada con su motivo no* | ~~2~~ → **0** | — | **Manual 10** §1 *(sigue mandando en el transporte: SPP, no BLE)* |
| **A1′** | **`ESP32` clásico** `WROOM-32` / `-32D` / `-32E` / `-32U` sobre **placa DevKitC de 38 pines**. Hace de módulo SPP **y** sostiene el `DS3231` de A6. ⛔ **Ni «el más grande» ni «el más nuevo»: eso es un `S3` y no habla SPP — ver el aviso del bloque 0** | 🛑 **BLOQUEADA** | Consola de servicio por celular en cada poste (evita subir con escalera al Esclavo) **+ el bus I²C del reloj** | **Manual 10** §1 y §2 |
| A2 | **Cámara IA** Hikvision AcuSense varifocal motorizada `DS-2CD3643G2-LIZSU` **o equivalente** | **2** *(ver nota)* | Demanda vehicular: una por poste, contacto seco a `PB0`. **Son las dos que el firmware lee hoy** | **Manual 9** |
| A3 | **Antenas VHF y coaxiales** | **2 + 2** | Recuperar alcance: las genéricas de «LoRa» costaban 15–20 dB y dejaban la cobertura en 3 cuadras | **Manual 7** §BOM *(lleva modelo, conectores y adaptadores)* |
| A4 | **Módulo de 1 relé optoacoplado, con jumper `JD-VCC`** | **2** *(1 por poste)* | **La talanquera.** El firmware ya la manda (SFTY-28, 27/08) y la tarjeta ya expone la señal: se conecta a la bornera **`J15`** (red `Motor`, `PB2` → opto `U15` → MOSFET `Q10`). 🔴 **NO a `J14`, que es la ENTRADA de la cámara — ver la fe de erratas de la cabecera.** **No hace falta `PCF8574` ni MOSFET nuevo** | **Manual 13** §3 *(la etapa de potencia y el cuadro `J14`/`J15`)*; el jumper `JD-VCC` y su porqué, en el aviso del bloque **B** de esta misma lista |
| **A5** | **Fuente propia para cada `ESP32`**: convertidor DC-DC reductor **12 V → 5 V, 1 A o más** (un módulo `LM2596` o `MP1584` sirve), con sus borneras y su cable | **2** *(1 por `ESP32`)* | **Que el `ESP32` no cuelgue del `LM7805` de la tarjeta.** A 500 mA de pico el `7805` disipa 3,5 W sin disipador, y al hundirse el riel de 3,3 V **se reinicia el STM32 que gobierna el semáforo** | **Manual 10** §1 *(la regla)* — ⚠️ **la pieza no está especificada en ningún manual todavía: ver el aviso de abajo** |
| **A6** | **Módulo RTC `DS3231` `ZS-042`** con **su propia pila**, colgado del `ESP32` por I²C (`GPIO21` SDA · `GPIO22` SCL) | **1** *(el del Maestro — ver nota)* | El reloj del equipo, **fuera de la placa STM32**: no hay que modificar la tarjeta ni sacar hilos de `PB0`/`PB8` | **Manual 11** *(la pieza)* · ⚠️ **el montaje sobre `ESP32` no está en ningún manual: ver el aviso de abajo** |
| **A7** | **Juego de conexión de las cámaras a `J16`**: conector hembra del footprint de `J16` con sus terminales de crimpar, y cable de 2 hilos apantallado por cámara | **2 juegos** | Llevar el contacto seco de la cámara a los pines que **libera el mando** (`PB14`/`PB15`). **No hace falta `PCF8574` ni ninguna placa hija** | **Manual 13** §3 *(borneras)* y `03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md` §7 *(el mapa pin a pin de `J16`)* |

> 🔌 **Cómo queda montado lo que se pide en A1′, A5 y A6 — para que las tres líneas se lean juntas:**
>
> ```text
>    12 V de la caja
>       |
>       +---> LM7805 -> LM1117-3.3 -> STM32   (la tarjeta, SIN TOCAR)
>       |
>       +---> [A5] DC-DC 12V -> 5V, 1 A ----> [A1'] ESP32 DevKitC (5V / VIN)
>                                                |
>                                                +-- GPIO21 SDA --+
>                                                +-- GPIO22 SCL --+--> [A6] DS3231
>                                                +-- GND ---------+     con su pila
>                                                |
>                                                +-- TX / RX -> J17 p2/p3 (PB7/PB6)
>                                                +-- GND ------> J17 p7   MASA COMUN
>
>    MASA COMUN entre las dos ramas.  ALIMENTACION NO COMPARTIDA.
>    De J17 se usan la senal y la masa.  Sus 3,3 V NO alimentan al ESP32.
> ```
>
> **La masa común no es opcional y la alimentación compartida sí está prohibida**: son dos cosas
> distintas y confundirlas es lo que hace que el semáforo se reinicie solo. Manual 10 §1.
>
> **`J17` es el conector que deja libre la pantalla retirada** *(bloque D)*, y es donde el Manual 10
> §2 manda el módulo desde la revisión del 28/08: `USART1` **remapeado** a `PB7`/`PB6`.
> 🔴 **`J16` NO es `J17`, se parecen y `J16` lleva 12 V**: ese aviso está en el Manual 10 §2.2 y en el
> Manual 2 §8, y vale igual para el `ESP32` que para un `HC-05`.

> ✏️ ~~**Corrección en A1 (28/08): decía `JDY-31`, y el `JDY-31` está PROHIBIDO en el Manual 10.**~~
> **Sin efecto desde la decisión de obra: no se compra ninguno de los dos.** Se deja escrito porque
> sigue siendo cierto —el `JDY-31` es BLE y el Manual 10 §1 lo excluye por nombre— y porque el día
> que alguien reabra la vía del módulo dedicado tiene que encontrarse la corrección hecha.
>
> 🔧 **Lo que SÍ sobrevive de A1, y hay que hacerlo igual con el `ESP32`: cada equipo tiene que
> anunciarse con su matrícula ANTES de subir al poste** (`SEM-<SERIE>-M` y `SEM-<SERIE>-E`), o el
> técnico verá dos dispositivos idénticos en la lista de Android y no sabrá a qué poste se conecta.
> **Lo que cambia es cómo se hace:** en un `HC-05` era un `AT+NAME` a 38400 bps (Manual 10 §1); en un
> `ESP32` **el nombre lo fija su propio firmware**, y ese firmware **todavía no está escrito**.
> ⚠️ **Se anota como hueco, no se da por resuelto:** nadie lo ha hecho, y es lo que separa dos
> equipos distinguibles de dos filas iguales en la pantalla del celular.

> 🔌 **Sobre A5 —la fuente propia—, lo que hay y lo que falta, separado a propósito:**
>
> | dato | nivel de prueba |
> |---|---|
> | Un `ESP32` con WiFi da picos de ~500 mA y el camino `12 V → LM7805 → LM1117-3.3` no los da | 📄 **Escrito y razonado** en el **Manual 10 §1**, con la cuenta de los 3,5 W |
> | Que haya que ponerle fuente propia desde los 12 V, masa común y alimentación **no** compartida | 📄 **Es la regla del Manual 10 §1**, con esas palabras |
> | **Qué módulo concreto se compra** —referencia, corriente, si aislado o no, cómo se fija en la caja | 🔴 **NO ESTÁ EN NINGÚN MANUAL.** Lo de esta fila es un mínimo razonable, no una especificación |
>
> **Se apunta el hueco en vez de taparlo con una referencia falsa**, que es la costumbre de este
> documento: el detalle de A5 tiene que acabar en el **Manual 10 §2**, junto al diagrama de conexión
> — y ese diagrama **hoy dibuja un módulo colgado del riel de la placa, que es justo lo que un
> `ESP32` no puede hacer**.

> 🕐 **Sobre A6 —el reloj—, qué cambió y por qué abarata la obra:**
>
> * **Ya no se monta en la placa STM32.** Cuelga del `ESP32` por I²C (`GPIO21` SDA, `GPIO22` SCL) y
>   con **pila propia**. Eso quita de encima modificar la tarjeta y sacar hilos de `PB0`/`PB8`.
> * **Y por eso sale del bloque B:** el bloque B espera al veredicto del cristal `Y2` porque el RTC
>   iba a ir en la tarjeta. Colgado del `ESP32`, **esa dependencia desaparece para el reloj**.
>   ⏳ **Lo que NO se da por resuelto aquí:** si el diagnóstico del cristal (`B5` de `ESTADO.md`)
>   deja de hacer falta del todo, o solo deja de bloquear esta compra. **Eso lo cierra quien lleva el
>   banco**, no esta lista.
> * **Cantidad 1, y el porqué:** el Esclavo **ya toma la hora del Maestro por radio**
>   (`CMD_HORA_*`, SFTY-23), así que no necesita reloj propio. Si el responsable quiere que cada
>   poste mantenga hora **sin depender del enlace**, son **2** — es una decisión, no un olvido.
> * 🔴 **Sigue sin haber driver, y eso no es una avería.** Ver el aviso del bloque B, que se mantiene
>   entero: **al enchufarlo no dará la hora, porque no hay código que le hable** — ahora en el
>   `ESP32`, cuyo firmware tampoco está escrito.

> ⚠️ **AVISO DE RECEPCIÓN de A6 — se hace ANTES de darle corriente, no al montar:**
>
> **El `DS3231 ZS-042` se vende muy a menudo con una pila `CR2032` NO recargable puesta encima de un
> circuito de carga.** El módulo está diseñado para una `LIR2032` recargable; con una `CR2032` dentro
> ese circuito **intenta cargar una pila que no admite carga: se calienta, se hincha y puede
> reventar**, con el módulo ya dentro de una caja en un poste.
>
> **Qué se hace al abrir la caja, en este orden:**
>
> ```text
>  1. Mirar la pila que trae puesta.  Rotulo CR2032 -> NO recargable
>                                     Rotulo LIR2032 -> recargable, correcto
>  2. Si es CR2032:  desoldar D1 o R1 del modulo ANTES de energizarlo
>                    (cualquiera de los dos corta el camino de carga)
>  3. Si es LIR2032: no se toca nada.  El circuito de carga se deja intacto
>  4. Solo entonces se le da corriente
> ```
>
> **Esto es un aviso de recepción, no de compra:** no cambia qué se pide, cambia qué se comprueba
> antes de enchufarlo. Y **pila y modificación van juntas** — quien cambie la pila más adelante
> vuelve al paso 1.

> 📷 **Sobre A7 —las cámaras en `J16`—, lo medido y lo que falta:**
>
> | dato | nivel de prueba |
> |---|---|
> | `PB14` y `PB15` salen a `J16` **pines 10 y 12**; el pin 2 es `GND` | 📐 **Medido en el esquemático** (`MAPEO_TARJETA_KICAD.md` §7) |
> | Cada uno lleva **antirrebote en la propia placa** (`R65`–`R68` + `C26`–`C29`): un contacto seco entra directo, **sin expansor y sin componentes nuevos** | 📐 **Medido en el esquemático** (mismo §7) |
> | Hoy el firmware los declara `BOTON3`/`BOTON4` en `INPUT_PULLUP` (`botones.cpp:52-58`) y **no los lee como cámara** | ✅ **Medido en el fuente** el 28/08 |
> | Que en la tarjeta física esas vías lleguen donde dice el esquemático | 🔴 **NO VERIFICADO.** Multímetro, cinco minutos, en la sesión de banco |
>
> 🔴 **`J16` LLEVA 12 V EN SU POSICIÓN 1** — es el único conector de señal de toda la tarjeta que los
> trae. **El contacto seco de la cámara va a los pines 10 y 12 con retorno por el 2. En el 1 no se
> conecta nada.** Es el mismo aviso por el que el módulo Bluetooth no entra en `J16` (Manual 10
> §2.2), y aquí vale igual.
>
> ⛔ **Y con esta línea el riesgo sube, porque a partir de ahora se enchufa algo en LOS DOS:** el
> `ESP32` en `J17` y las cámaras en `J16`. **Los dos conectores comparten footprint y son idénticos a
> la vista.** Intercambiarlos mete **12 V en el `UART` del `ESP32` y lo quema**. Antes de enchufar
> nada, **multímetro en la posición 1 contra masa: si da ≈ 12 V, ése es `J16` y ahí va la cámara; si
> no, es `J17` y ahí va el módulo.** Es la misma comprobación del Manual 2 §8, con dos cables en vez
> de uno.
>
> 🛑 **Y la regla de este documento manda también aquí: lo que se compra hoy es el cable y el
> conector, no una cámara más.** El firmware **no lee `PB14`/`PB15` como entrada de cámara** —los lee
> como botones—, así que **la segunda cámara por poste (SFTY-29, presencia como veto) sigue sin
> pedirse hasta que exista el firmware que la lea**. La ruta ya está decidida; la función, no escrita.
>
> ⏳ **Y una precisión que no se inventa aquí:** la decisión del 28/08 dice *«las cámaras van a `J16`,
> los pines que libera el mando»*. **La de demanda que ya funciona entra por `PB0` / bornera `J14`,
> está medida y esta lista no la mueve.** Si la intención era **trasladar** también esa, hay que
> decirlo por escrito — cambia el Manual 9, el Manual 13 y el pack `camara_01_demanda`.

> **Nota sobre la cantidad de cámaras.** El diseño habla de **cuatro** (dos por poste: demanda y
> umbral), pero **el firmware solo lee las de demanda**: las de umbral quedaron retiradas en N-59
> porque el protocolo no tiene comando para mandar la cuenta del tramo al Maestro. **Las de umbral se
> piden cuando exista ese comando** (tarea `C1` de `ESTADO.md`), no antes.
>
> `ESTADO.md` venía pidiendo **3** sin que ningún manual explique por qué —probablemente porque ya hay
> una comprada—. **Eso lo confirma el responsable antes de pedir:** si ya hay una en almacén, son 1 o
> 2; si no hay ninguna, son 2.
>
> 🔄 **Al día del 28/08, y sin cambiar la cantidad:** de las dos cosas que le faltaban a la segunda
> cámara por poste, **una ya está** —**por dónde entra**: `J16`, con los pines que libera el mando,
> línea **A7**— y **la otra sigue faltando**: el firmware que lea ese pin. Con SFTY-29 el dato ya ni
> siquiera necesita comando nuevo *(viaja gratis en el `param` de `CMD_ACK_RED`)*, pero **nadie lo ha
> escrito**. **La cantidad de A2 no sube hasta que exista ese código.**

---

## B · ESPERA AL BANCO — y desde el 28/08 solo queda aquí la expansión

> 🔄 **28/08: este bloque se quedó con la mitad.** El RTC —`B1` y su pila `B2`— **se ha ido a la
> línea A6**, colgado del `ESP32`. Lo que sigue esperando al banco es **solo la expansión**.

**La pregunta:** *¿en qué tarjeta está muerto el cristal `Y2` de 32.768 kHz?* Se responde en la sesión
de banco (tarea `B5`), y hasta entonces **no se pide nada de este bloque**:

| si el cristal muerto está en… | qué hace falta |
|---|---|
| el **Esclavo** | **NADA.** Ya toma la hora del Maestro por radio (`CMD_HORA_*`, SFTY-23). Cero pesos |
| ~~el **Maestro**~~ | ~~ahí sí: es quien fija la hora, y necesita fuente propia → todo el bloque de abajo~~ ⛔ **Esta rama ya no manda en la compra del reloj:** el `DS3231` va colgado del `ESP32` con su pila (**A6**) y **no** en la tarjeta. Lo que el cristal decida sigue importando para el firmware del Maestro, no para pedir el módulo |

| # | Qué | Cant. | Especificación en |
|:---:|---|:---:|---|
| ~~B1~~ | ~~**Módulo RTC `DS3231` `ZS-042`**~~ ⛔ **MOVIDO A A6.** Ya no se monta en la placa STM32: cuelga del `ESP32` por I²C. **Se sigue comprando el mismo módulo, en otro sitio y sin esperar al banco** | ~~1~~ → **ver A6** | **Manual 11** *(la pieza)* |
| ~~B2~~ | ~~**Pila `LIR2032`** (Li-ion 3,6 V **recargable**)~~ ⛔ **MOVIDA A A6**, que es donde vive ahora el reloj. ⚠️ **El aviso de la `CR2032` no se pierde: está en A6 como aviso de recepción** | ~~1~~ → **ver A6** | **Manual 11** |
| B3 | **`PCF8574P`** (DIP-16) + zócalo, si además se quiere expandir | 1 | **Manual 13** §5 — ⚠️ **no** el `PCF8574A` |
| B4 | Optos `PC817`, módulo de relé 12 V, resistencias, borneras, placa perforada | ver manual | **Manual 13** §5 *(qué se pide)* y §6 *(cómo se monta)* |

> 🛑 **Y `B3` tiene ahora menos motivos que ayer, no más.** El expansor entraba para dar entradas y
> salidas que faltaban; entre lo que la tarjeta ya trae en el cobre (N-63) y **los dos pines que
> libera el mando retirado** (`PB14`/`PB15`, línea A7), **hoy no hay ninguna función pendiente que lo
> necesite**. No se tacha porque el bus podría hacer falta si aparece una, pero **quien lo pida tiene
> que decir para qué**: es la regla del principio de esta lista.

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

> 🔴 **PRIMERO, LO QUE NO ES UNA AVERÍA — y sigue valiendo con el reloj mudado a A6:** el **`DS3231`
> no tiene driver en ninguna punta**. Medido el 28/08: `grep -rniE "DS3231|Wire\.|0x68"` sobre
> `01_Firmware/Maestro/{src,include}` y `01_Firmware/Esclavo/{src,include}` da **cero coincidencias
> de código**. **Al enchufarlo no dará la hora: no hay código que le hable. Eso no es una avería, ni
> del módulo ni del montaje** — no se devuelve al mostrador ni se busca el fallo en la soldadura.
>
> ⚠️ **Y colgarlo del `ESP32` no lo arregla, lo mueve:** ahora el código que falta es **el del
> `ESP32`, que tampoco existe todavía**. Se compra para tenerlo cuando llegue ese firmware
> (`roadmap.md` N-54 / N-55). **Lo mismo vale para el `PCF8574` de B3.**

> 🔴 **Tres avisos que cambian lo que se compra, no solo cómo se monta:**
>
> - **`PCF8574` sí, `PCF8574A` no.** Misma patilla, dirección base `0x38` en vez de `0x20`. Si llega
>   el «A», el firmware no lo encuentra y parece que la placa está mal soldada.
> - ~~**Pila `LIR2032` recargable, con el circuito de carga del módulo intacto.**~~ ➡️ **MOVIDO A
>   A6** con el reloj, y ampliado allí como **aviso de recepción**: el `ZS-042` suele venir con una
>   `CR2032` **no recargable** ya puesta sobre el circuito de carga, y eso se resuelve **antes de
>   energizarlo**, desoldando `D1` o `R1`. **Pila y modificación siguen yendo juntas.**
> - **Módulo de relé con jumper `JD-VCC`.** Sin él no hay aislamiento y no se puede alimentar la
>   lógica a 3,3 V: se destruye el expansor al primer montaje. Si el que venden no lo trae, hace falta
>   además un transistor `2N2222` y una resistencia de 1 kΩ.

---

## C · YA EN SERVICIO — no se pide, se verifica

| Qué | Estado | Dónde se comprueba |
|---|---|---|
| **2 radios `E90-DTU`** en enlace directo, `2.4 kbps`, `M0`/`M1` en OFF | en servicio desde el 01/08 | **Manual 4** |
| Radio **B1** | **averiada y retirada** (transmisor). Si se quiere repuesto, es la misma referencia | `roadmap.md` |
| Repetidor ESP32 | **fuera de la configuración vigente** (enlace directo, sin repetidor). ⚠️ **No confundir con los ESP32 llegados el 28/08**: aquel es el puente de radio del Manual 5, y **no** cubre la línea A1′ | **Manual 5** |
| Pila de `VBAT` en la tarjeta, con `R5` retirada | montada | **Manual 11** *(es OTRA pila distinta de la de A6)* |

---

## D · RETIRADO POR LA DECISIÓN DEL 28/08 — no se compra, y si estaba pedido se cancela

> **Este bloque existe para que un «no se compra» tenga dónde vivir.** Sin él, lo retirado
> desaparece de la lista y a la semana siguiente vuelve como si fuera nuevo.

| Qué | Estado de la compra | Qué pasa con lo que ya hay |
|---|---|---|
| **Pantalla LCD** `ST7920` (conector `J17`) | ⛔ **RETIRADA.** No se compran repuestos ni unidades nuevas | Las que estén montadas se quedan donde están. **No se compra una de recambio si una muere.** Su conector **`J17` es el que ocupa ahora el módulo de A1′** (`USART1` remapeado a `PB7`/`PB6`, Manual 10 §2) |
| **Los cuatro pulsadores** (`J16`: `PB9`, `PB13`, `PB14`, `PB15`) | ⛔ **RETIRADOS** | Sus **dos últimos pines pasan a las cámaras** — es la línea **A7** |
| **Mando de relés / su receptor RF** | ⛔ **RETIRADO.** ⚠️ **El receptor NUNCA se compró**, así que aquí no hay nada que cancelar: no llegó a estar en ninguna línea de esta lista | — |
| ~~`HC-05` / `JDY-30`~~ | ⛔ **ANULADO** — línea A1, sustituida por el `ESP32` (A1′) | Nunca llegaron |
| ~~`DS3231` + pila **en la placa STM32**~~ | 🔄 **NO anulado: MOVIDO.** Se compra igual, colgado del `ESP32` — línea **A6** | — |

> ⚠️ **Retirado del EQUIPO no es retirado del FIRMWARE, y confundirlo cuesta una sesión de banco.**
> Medido el 28/08 en el fuente: `main.cpp:45` llama a `lcd_setup()`, que en `lcd.cpp:70` llama a
> `u8g2.begin()`; y `botones.cpp:52-58` declara los cuatro pines en `INPUT_PULLUP` y los lee con
> antirrebote. **El firmware de hoy sigue compilando y ejerciendo pantalla, menú y los cuatro
> botones.**
>
> **Para la compra da igual —no se pide nada—, pero para A7 no:** mientras `PB14`/`PB15` sean botones
> en el código, **un contacto seco de cámara ahí dentro entra como una pulsación**, y el `Boton 3`
> *ejecuta* (lo dice el comentario de N-26 en ese mismo fichero: un `3` fantasma arranca un modo que
> nadie pidió, y en un semáforo eso es una maniobra). **El cable se puede comprar hoy; conectarlo
> espera al firmware.**

---

## Resumen para autorizar

**Se pide hoy:** 2 cámaras AcuSense de demanda *(confirmar si ya hay una)* · 2 antenas VHF con sus 2
coaxiales · 2 módulos de 1 relé con jumper `JD-VCC` · **2 fuentes DC-DC 12 V → 5 V ≥ 1 A** (A5) ·
**1 módulo `DS3231` `ZS-042` con su pila** (A6) · **2 juegos de conector y cable para `J16`** (A7).

> 🛑 **Y NO se pide hoy ningún módulo de Bluetooth — de ninguna clase.** ~~Los 2 `HC-05` / `JDY-30`~~
> quedaron **anulados** por la decisión de obra del 28/08: el `ESP32` los sustituye. Y **el `ESP32`
> está BLOQUEADO** hasta que alguien lea qué referencia llegó, por serigrafía o por `chip_id`, y lo
> anote en el bloque **0**. Comprar antes de esa lectura es como se acaba con un `S3` que **no habla
> SPP** y con dos referencias distintas en el mismo cruce.

> ⚡ **Las tres líneas nuevas no son accesorios, y por eso están arriba y no «para luego»:**
> **A5** es lo que impide que el `ESP32` hunda el riel y **reinicie el STM32 que gobierna el
> semáforo**; **A6** es el reloj, que deja de tocar la placa; **A7** son cuatro pesos de cable que
> evitan una placa hija entera.

**Se pide después del banco, y solo si aparece una función que lo necesite:** el `PCF8574P` con su
placa de expansión (B3, B4). ⚠️ **Sigue sin driver: se compra preparado, no funcionando** — y hoy
**no hay ninguna función pendiente que lo pida**.

**No se pide:** cámaras de umbral ni la segunda cámara por poste *(la ruta ya está —`J16`—, el
firmware que las lea no)*, `PCF8574` para talanqueras ni para cámaras, pantalla LCD, pulsadores, ni
mando de relés o su receptor *(que nunca se compró)*. Las talanqueras salen por la salida
**`Motor` (bornera `J15`, MOSFET `Q10`)** que **la tarjeta ya trae**, y las cámaras de demanda por
`PB0` (bornera `J14`), que ya se lee.

> ✏️ **Corregido el 28/08 (2.ª rev.):** este párrafo decía *«las talanqueras salen por la salida
> `Puerta`»*. **`Puerta` es la red de ENTRADA de la cámara** (`J14`), no la de la pluma. La salida de
> la talanquera es la red **`Motor`** (`J15`). Es la misma errata de la fila A4, y aquí estaba
> repetida con el nombre de la red en vez del de la bornera — que es justo como una corrección se
> deja a medias. Ver la fe de erratas de la cabecera.
