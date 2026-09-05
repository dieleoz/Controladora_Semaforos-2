# 🛒 MANUAL 15: LISTA DE COMPRAS DE HARDWARE — QUÉ PEDIR, CUÁNTO Y CUÁNDO

**Sistema:** Controladora de Semáforos Móviles de 3 Estados (V9.0)
**Fecha:** 27 de Agosto de 2026
**Última revisión:** **5 de Septiembre de 2026** — ⛔ **`A9` SALE DE LA LISTA: el mando de relés y
sus pulsadores NO SE COMPRAN**, y **entra `A10`: las microSD de las cámaras**. Detalle en el bloque
rojo de abajo.

**Revisión anterior:** **4 de Septiembre de 2026** — 🔬 **primera revisión con datos de BANCO REAL
(sesión del 03–04/09), y cambia lo que se compra en cuatro sitios:**

* ~~🟢 **`A9` (receptor del mando) se PIDE, y con salida `NO` (normalmente abierto).**~~ ⛔ **CADUCADO
  EL 05/09 POR `D-1`: NO SE COMPRA NINGUNO.** Se conserva tachado con su motivo, y el motivo entero
  se conserva en el bloque `A9`, porque la cadena de razonamiento —una sola masa en `J16`, activo en
  ALTO, `NC` inseguro— **sigue siendo válida para los pines de cámara**, que sí se cablean.
* 🟢 **`A7` (cable y conector de `J16`) se DESBLOQUEA para conectar:** la medida **`M3` está cerrada
  en banco** —pull-down real de 10 kΩ, `p10` 9,93 kΩ y `p12` 9,94 kΩ—. Se compraba ya; **ahora
  además se puede enchufar**, con el pin de 12 V tapado.
* 🆕 **Aparece el bloque `E`: la REVISIÓN V2 DE LA TARJETA.** Dos líneas nuevas que **no son
  decisiones tomadas**, sino cuentas para quien firme la placa: **`E1` resistencias de 2K2** para
  proteger las cinco entradas de campo, y **`E2` un diodo de potencia** que sustituya al `D30`.
* 🛑 **`A5` (la fuente) y `A8` (la placa portadora) siguen EXACTAMENTE IGUAL que el 31/08: sin pedir,
  sin referencia elegida, sin diseñar y sin fabricar.** Cuatro días después no se ha movido nada:
  se escribe para que la fila no envejezca en silencio.

**Revisión anterior:** **31 de Agosto de 2026** — 🟢 **`BLQ-1` CERRADO: el módulo que llegó es un
`ESP32-WROOM-32` clásico y habla SPP, así que `A1′` se DESBLOQUEA.** Con ella cambian tres cosas
más: `A5` pasa a exigir **DC-DC conmutado y no lineal** (con la cuenta de disipación delante),
aparece **`A8`, la placa portadora**, que hasta hoy no era línea de compras, y el **receptor del
mando de relés sube de prioridad**, porque sin él el Esclavo se queda sin ninguna vía de mando. Lo
del 28/08 se conserva entero debajo.
**Revisión anterior:** 28 de Agosto de 2026 (3.ª del día) — 🔄 **la arquitectura cambió en obra: el
`ESP32` SUSTITUYE al módulo Bluetooth SPP, el `DS3231` deja de ir en la placa STM32 y se cuelga del
`ESP32`, y se retiran la pantalla LCD, los cuatro pulsadores y el mando de relés.** Lo que ya no se
compra queda **tachado con su motivo**, no borrado. *(2.ª rev. del 28/08: corrige la bornera de la
talanquera en A4 —decía `J14`, y `J14` es una ENTRADA del micro—; avisa de que el `DS3231` no tiene
driver; arregla las referencias a las secciones del Manual 13. 1.ª rev.: bloque 0 con el estado real
de lo recibido —llegaron ESP32, no `HC-05`—, criterio de compra del módulo Bluetooth y corrección de
`JDY-31` → `JDY-30`.)*
**Para:** el funcional / quien autoriza la compra

---

> [!IMPORTANT]
> # ⛔ 05/09/2026 — DOS CAMBIOS QUE SE EJECUTAN CON DINERO. LÉASE ANTES DE EMITIR NINGUNA ORDEN
>
> ## 1. 🛑 `A9` SALE: **EL MANDO DE 4 RELÉS Y LOS PULSADORES NO SE COMPRAN — Y NO ES UNA PIEZA PENDIENTE**
>
> **`DECISIONES.md` `D-1`, confirmada por el responsable el 05/09:** *«ya no tenemos mandos de A y B,
> sólo la app, los quitamos»*.
>
> **Esto NO es «está pendiente», NO es «se compra después del banco» y NO es «falta decidir la
> referencia». Es una decisión tomada: no se compra, ni ahora ni luego.** Se escribe con estas
> palabras porque la línea `A9` llevaba desde el 31/08 subiendo de prioridad y el 04/09 volvió al
> «se pide»: **quien lea una copia impresa de cualquiera de esas dos revisiones va a pedir dos
> receptores RF que ya no tienen para qué**.
>
> | | qué se compra |
> |---|---|
> | Receptor RF del mando *(línea `A9`)* | ⛔ **NADA. Cero unidades** |
> | Emisor de 4 canales | ⛔ **nada — el que había se retiró con el mando** |
> | Pulsadores de `J16` p5 y p8 | ⛔ **nada. Los pines quedan LIBRES y sin cablear** |
>
> 🛑 **Y LA OTRA MITAD, QUE VA PEGADA A ÉSTA O EL LECTOR SACA LA CONCLUSIÓN CONTRARIA: el CÓDIGO del
> mando NO se retira.** Es firmware, no compras, pero se escribe aquí porque *«ya no compramos el
> mando»* invita directamente a *«pues quitemos su código»*, y eso **abre el veto de SFTY-21 en vez
> de dejarlo inerte**. Con el mando desmontado la bandera simplemente **no se arma nunca**, que es lo
> correcto. El desarrollo está en el **Manual 2 §6**.
>
> ### 📵 Y la consecuencia de operación que esta lista tiene que declarar, porque nace de una compra que NO se hace
>
> **Retirado el mando, la app es la ÚNICA superficie de mando del equipo** (`D-16`). Ni ámbar, ni
> volver a Automático, ni parar el cruce sin un teléfono. **Lo que esta lista puede hacer al respecto
> es una sola cosa, y no cuesta casi nada: que haya un SEGUNDO TERMINAL ya emparejado** —con su cable
> de carga— **antes de que un equipo salga a la calle**. No se pide como electrónica; se pide como
> herramienta, igual que la escalera.
>
> ## 2. 🆕 `A10` ENTRA: **LAS microSD DE LAS CÁMARAS SE COMPRAN**
>
> **Decisión del responsable, 05/09:** *«cada cámara tiene una micro, la metemos»*. Deja de ser la
> pregunta `A-0` en su primera mitad: **se compran, una por cámara — 2 unidades.** Ver la línea
> **`A10`** en el bloque **A**.
>
> ⚠️ **Lo que la decisión NO cierra, y sigue en `A-0` de `DECISIONES.md`:** **capacidad**, **días de
> retención** y **si la grabación va continua o por evento**. Se puede comprar sin cerrarlas —una
> tarjeta grande sirve para las tres respuestas— **pero no se puede configurar la cámara sin ellas**.
>
> ---
>
> # 🔬 04/09/2026 — LO QUE EL BANCO CAMBIÓ EN ESTA LISTA
>
> **Es la primera vez que esta lista se corrige con una tarjeta delante y no con el fuente.** Se lee
> antes que el resto porque **una copia impresa de la revisión del 31/08 se ejecuta con dinero y le
> falta lo de abajo**.
>
> ## 1. ~~🟢 `A9` — se PIDE, con salida `NO`~~ ⛔ **NO SE PIDE (05/09, `D-1`).** El mando **estuvo** sordo, ya no lo está en el fuente, **y ya no está**
>
> 🛑 **Todo este apartado queda como HISTÓRICO: cierra `N-118` y no autoriza ninguna compra.** Se
> conserva entero porque explica por qué durante cuatro días se creyó que había un defecto de placa,
> y esa confusión es la que no debe volver.
>
> **EL DEFECTO, en pasado — es lo que midió el banco el 03–04/09 sobre el firmware `617bd00`:**
> `R65`/`R66` —10 kΩ a masa, el mismo cobre que las entradas de cámara— dejan el pin en **0,6 V**.
> Aquel firmware leía `INPUT_PULLUP` y activo en **BAJO**, así que veía **BAJO permanente** y el
> arranque lo sembraba como **«flanco ya consumido»**: **nunca había transición**. No era que el
> mando funcionara mal — **es que no había gesto que lo activara** (N-118).
>
> **EL ESTADO DE HOY:** 🟢 **el firmware está arreglado en las DOS puntas** (`346ea5f`), verificado
> por lectura el 05/09 sobre los símbolos, no sobre números de línea:
>
> ```text
>   $ grep -n "pinMode(BOTON1, INPUT)" Maestro/src/botones.cpp Esclavo/src/botones.cpp
>   Maestro/src/botones.cpp:160:  pinMode(BOTON1, INPUT);
>   Esclavo/src/botones.cpp:178:  pinMode(BOTON1, INPUT);
>
>   $ grep -n "lecturaCruda = " Maestro/src/botones.cpp Esclavo/src/botones.cpp
>   Maestro/src/botones.cpp:40:  bool lecturaCruda = (digitalRead(b.pin) == HIGH);
>   Esclavo/src/botones.cpp:54:  bool lecturaCruda = (digitalRead(b.pin) == HIGH);
> ```
>
> Exactamente como la cámara en N-67. 🔴 **Y nadie lo volvió a medir con el binario nuevo dentro:**
> el paso 29 se abortó y la Maestro sigue con el corto de N-116. **Esa medida queda `SIN VERIFICAR` y
> ya no se va a tomar** — porque no hay mando.
>
> ⚠️ ~~**Y con el arreglo cambia EL GESTO DE PRUEBA:** cerrar contra los 3,3 V del pin contiguo,
> `p5`–`p4` y `p8`–`p7`.~~ ⛔ **CADUCADO EL 05/09: no hay mando, así que no hay gesto.** **`p5` y `p8`
> quedan libres y NO se les acerca un cable** — ni para probar: `p4` es adyacente a `p5` y un puente
> corrido una posición pone 3,3 V contra masa, que es el candidato del calentamiento del paso 29.
>
> 🟢 ~~**Y de ahí sale una DECISIÓN DE COMPRA que no está tomada: `NO` o `NC`.**~~ → **CADUCADA el
> 04/09 (no era una decisión: la decide el cobre) y AMORTIZADA el 05/09 (no hay compra).** ✅ **Lo que
> del razonamiento sigue vigente y por eso no se borra: los cuatro pines de `J16` son eléctricamente
> idénticos y todos son ACTIVOS EN ALTO** —una sola masa en el conector (`p2`), 3,3 V en cada
> posición contigua—. **Eso gobierna hoy el cableado de las CÁMARAS** (`A7`), que sí se conectan.
>
> ## 2. 🟢 `A7` — `M3` CERRADA: las cámaras de `J16` ya se pueden CONECTAR
>
> ```text
>   M3, paso 20, tarjeta energizada y J16 vacio:
>     J16 p10 (PB14) ->  9,93 kOhm a masa  ->  0 V
>     J16 p12 (PB15) ->  9,94 kOhm a masa  ->  0 V
>   Paso 21: en reposo, CON y SIN el cable puesto, el equipo NO pide paso solo.
> ```
>
> **El pull-down de 10 kΩ es real.** Esta lista decía *«el cable se compra hoy; conectarlo espera a
> `M3`»* — **ya no espera**. La polaridad la decide el cobre: **activo en ALTO**, contacto contra los
> 3,3 V del borne contiguo. Detalle completo en el **Manual 9**.
>
> ## 3. 🛑 EL AVISO QUE ACOMPAÑA A TODO LO QUE SE ENCHUFE EN `J16`
>
> **`J16` p1 lleva 12 V CRUDOS, y las 5 entradas de campo van DESNUDAS al pin del STM32** —sin
> resistencia en serie, sin opto, sin clamp—, mientras que **las 9 salidas de la placa sí llevan
> 220 Ω y optoacoplador**. **Tapar físicamente el pin de 12 V de `J16` es OBLIGATORIO en cada equipo
> antes de cablear.** El 04/09 **una tarjeta Maestro quedó con un cortocircuito de 3,3 V a masa**;
> esto es lo único que hay entre un instalador y que vuelva a pasar.
>
> **Eso no es una compra: es un gesto de montaje.** La compra que lo corrige de verdad es el bloque
> **`E`**, y es de la **revisión V2 de la placa**.
>
> ## 4. ⏸️ Lo que NO se movió en cuatro días, y por eso se escribe
>
> | | estado al 31/08 | estado al **04/09** |
> |---|---|---|
> | **`A5`** fuente DC-DC conmutada | sin pedir, sin referencia elegida | **igual** |
> | **`A8`** placa portadora | sin pedir, sin diseñar, sin dueño | **igual** |
> | **`A1′`** conteo de pines (30/38) y cuántos hay en almacén | sin contar | **igual** |
>
> **Una fila que no cambia no es una fila cerrada.** `A5` y `A8` siguen siendo lo que **BLOQUEA EL
> MONTAJE** del `ESP32`, y las dos se pueden empujar hoy sin esperar a nadie.

---

> [!IMPORTANT]
> # 🟢 31/08/2026 — `BLQ-1` ESTÁ CERRADO: LA LÍNEA `A1′` SE DESBLOQUEA
>
> **El módulo que llegó a obra es un `ESP32-WROOM-32` clásico.** La ficha del artículo comprado
> —aportada por el responsable— lo declara con **tres confirmaciones independientes**, y las tres
> apuntan al mismo silicio:
>
> ```text
> Microcontrolador ...  ESP32-WROOM-32
> CPU ................  Tensilica Xtensa 32-bit LX6, DUAL-CORE
>                       el S3 es LX7 y el C3 es RISC-V -> no es ninguno de los dos
> Bluetooth ..........  v4.2 BR/EDR and Bluetooth Low Energy (BLE)
>                              ^^^^^^ Bluetooth Clasico
> ```
>
> **`BR/EDR` es Bluetooth Clásico, o sea SPP.** Es exactamente el perfil que necesita
> `createRfcommSocketToServiceRecord` del puente nativo de Android: **la app conecta sin tocar una
> línea de transporte**, y **el apartado 1 del Manual 10 no se reabre**. 📄 **LEÍDO** de la ficha del
> artículo, no medido con `esptool` ni con la serigrafía en la mano — ver `roadmap.md` **N-107** y
> `ESTADO.md` (fila `BLQ-1`).
>
> ### Lo que eso cambia en esta lista, y lo que NO
>
> | | |
> |---|---|
> | ✅ **`A1′` deja de estar 🛑 BLOQUEADA** | El módulo que hay sirve. **La cantidad que hay en almacén sigue sin anotarse** |
> | ✅ **La vía WiFi queda descartada** | No hay que reabrir el Manual 10 §1. Era la rama cara y ya no está sobre la mesa |
> | ✅ **La alimentación queda fijada por la propia ficha** | `12 V → DC-DC **conmutado** → 5 V → `VIN`` (entrada recomendada 5 V, límite 5,5 V, con regulador a bordo). Ver `A5`, que **cambió** con este dato |
> | ✅ **Las E/S del módulo son de 3,3 V** | El enlace con el STM32 por `J17` va **directo, sin adaptador de niveles**. No hay que comprar traductor |
> | 🛑 **NO se compra ni un `ESP32` más todavía** | **El bloqueo cambió de motivo, no desapareció:** ahora es el **conteo de pines**. Ver el aviso de abajo |
>
> ### 🛑 Lo que HOY bloquea comprar más módulos: 30 pines o 38, y no es lo mismo
>
> **Estas NodeMCU vienen en dos formatos, de 30 y de 38 pines, con anchos de placa distintos** — y
> la placa portadora (`A8`) lleva **hembrillas**, no la huella del `WROOM-32`. Un módulo de 38 pines
> no entra en unas hembrillas dimensionadas para 30, y al revés queda con filas al aire.
>
> **Se cuentan los pines y se mide el ancho con pie de rey ANTES de fabricar la portadora.** Es lo
> único que bloquea el taladro, y **no bloquea firmware** — el firmware del ESP32 es el mismo en los
> dos formatos. **Se anota aquí, con fecha y quién lo contó:**
>
> ```text
> Fecha: ____________   Contado por: ____________________
> Pines por fila:  [ ] 15 (placa de 30)   [ ] 19 (placa de 38)
> Ancho de placa medido con pie de rey: ________ mm
> Separacion entre filas (centro a centro): ________ mm
> Cuantos modulos hay en almacen: ______
> ```
>
> > ⚠️ **Y por qué no se compra «uno más por si acaso» antes de contarlos:** si el segundo pedido
> > llega en el otro formato, la portadora sirve para uno de los dos postes. Dos referencias
> > distintas en el mismo cruce es exactamente lo que este documento lleva un mes evitando.
>
> ### La lección de N-107, que se queda escrita porque cuesta dinero repetirla
>
> **El bloqueo se mantuvo más días de los necesarios por exigir la medida más cara.** Se pidió
> durante toda una sesión *«la serigrafía del blindaje, 30 segundos»* cuando **la ficha técnica del
> artículo comprado ya lo declaraba** y estaba a mano desde el principio.
>
> > **Antes de declarar algo bloqueado por una medida física, censa qué fuentes escritas pueden
> > responderlo ya.** Una ficha de compra, una factura, una serigrafía y un `esptool chip_id`
> > contestan la misma pregunta con costes muy distintos, y **la más cara no es la primera que hay
> > que pedir**. Un bloqueo que se levanta leyendo no es un bloqueo: es una consulta pendiente.

> [!CAUTION]
> # 🔄 28/08/2026 — LA ARQUITECTURA CAMBIÓ. ESTO SE LEE ANTES QUE EL RESTO DEL DOCUMENTO
>
> **El `ESP32` pasa a SUSTITUIR al módulo Bluetooth SPP —ya no se compran `HC-05` ni `JDY-30`—, el
> `DS3231` se cuelga del `ESP32` por I²C con pila propia en vez de montarse en la placa STM32, y se
> retiran la pantalla LCD, ~~los cuatro pulsadores y el mando de relés~~.**
>
> > 🔴 **ESA ÚLTIMA FRASE ESTÁ CORREGIDA DESDE EL 31/08 Y NO SE EJECUTA LITERAL.** Por decisión del
> > responsable: **se retiran SÓLO los pulsadores 3 y 4** (`PB14`/`PB15`), y **el MANDO DE RELÉS SE
> > CONSERVA** en los canales A (`PB9`) y B (`PB13`). **Retirar el mando entero no dejaba código
> > inerte: borraba un veto de seguridad** —el que impide que una orden de radio saque al Esclavo de
> > un ámbar que un operario dejó puesto a propósito—. ~~**Y de la corrección sale una línea de compra
> > nueva: `A9`, el receptor RF del mando, que nunca se compró.**~~
> >
> > 🔧 **05/09 — LA CORRECCIÓN SE CORRIGE A SU VEZ, Y CONVIENE VER LAS DOS MITADES POR SEPARADO
> > PORQUE UNA SOBREVIVE Y LA OTRA NO:** el responsable retiró **el hardware** del mando (`D-1`), así
> > que **`A9` queda anulada: cero unidades**. Pero *«retirar el mando entero no dejaba código
> > inerte: borraba un veto de seguridad»* **sigue siendo exactamente cierto**, y es hoy el motivo de
> > que **el código NO se toque** aunque el aparato ya no exista.
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
> se escribe en vez de borrarse**, y **E** —nuevo el **04/09**— **lo que necesita una
> revisión V2 de la placa: cuentas para quien la firme, no compras autorizadas**.

---

## 0 · ESTADO REAL DE LAS COMPRAS al 04/09 — leer antes de volver a pedir

> **Esta lista describía lo que hay que pedir; le faltaba decir qué llegó.** Sin esa columna se
> vuelve a comprar lo que ya está, y —lo que pasó— se da por cubierta una línea que no lo está.

| Línea | Qué se pidió | Qué hay de verdad hoy | Estado |
|---|---|---|---|
| ~~**A1**~~ Módulo Bluetooth SPP | ~~2 × `HC-05` / `JDY-30`~~ | **nunca llegaron, y ya no se piden** | ⛔ **ANULADA el 28/08.** El `ESP32` los sustituye |
| **A1′** `ESP32` como módulo SPP | — *(línea nueva del 28/08)* | **Llegaron módulos `ESP32-WROOM-32` clásicos** —referencia **CONFIRMADA por ficha** el 31/08—, **cantidad todavía sin anotar** | 🟢 **DESBLOQUEADA el 31/08 (`BLQ-1` cerrado).** ~~🛑 BLOQUEADA hasta leer qué referencia llegó~~ — **el motivo desapareció: hay SPP.** Lo que queda antes de comprar **más** es contar pines (30/38), y eso bloquea la portadora `A8`, no el firmware |
| **A2** Cámaras de demanda | 2 × AcuSense | 🟢 **05/09: COMPRADAS, y el modelo ya es un dato: `DS-2CD2683G2-IZS`.** Ficha oficial verificada — **tiene salida de alarma por contacto seco** *(`1 output`, `24 V`/`1 A`)*, así que el cableado a `J16` sigue en pie | 🟢 **CUBIERTA en la compra.** ~~pendiente *(confirmar si ya hay una en almacén)*~~ · 🔴 **Pero NO puesta en marcha:** quedan 3 comprobaciones antes de instalar —enlace analítica→relé, `NO`/`NC` y clasificador—, ver el bloque 📷 y **Manual 9 §8**. ⚠️ **Y arrastra `13 W` por poste y un SOPORTE que nadie ha especificado** |
| **A3** Antenas + coaxiales | 2 + 2 | sin novedad | pendiente |
| **A4** Módulos de 1 relé | 2 | sin novedad | pendiente |
| **A5** Fuente propia del `ESP32` | — *(línea nueva del 28/08)* | **NO se ha pedido, y es LO QUE BLOQUEA EL MONTAJE** | 🔴 **NO cubierta.** Sin ella el `ESP32` reinicia el STM32 del semáforo. **31/08: pasa a exigir DC-DC CONMUTADO, no lineal** — ver `A5`. ⏸️ **04/09: SIN MOVIMIENTO en cuatro días, y ninguna referencia elegida** |
| **A6** `DS3231` colgado del `ESP32` | — *(sale del bloque B)* | ~~**NO se compró**~~ ✅ **YA ESTÁ: el responsable confirmó el 05/09 que CADA ESP32 lleva su reloj con pila propia.** Esta fila llevaba días caducada y produjo una contradicción en `ESTADO.md` | ~~pendiente~~ ✅ **CUBIERTA — son DOS, uno por poste, y están puestos.** ~~**Lo que sigue condicionado a `Y2` es CUÁNTOS**~~ ⛔ **caducado el 05/09 por `D-9` y `D-15`: el STM32 NO tiene reloj y ya no atiende `SET_RTC`, así que las dos puntas necesitan el suyo y las dos lo tienen.** `Y2` ya no decide cuántos. 🔴 Sigue `SIN VERIFICAR` la dirección `0x68` sobre el módulo real — ver `A6` |
| **A7** Conexión de cámaras a `J16` | — *(línea nueva del 28/08)* | **NO se ha pedido** | pendiente *(es cable y conector, no electrónica)*. 🟢 **04/09: `M3` CERRADA en banco — ya no sólo se compra, YA SE PUEDE CONECTAR**, con el p1 de `J16` tapado. Ver `A7` |
| **A8** **Placa portadora del `ESP32`** | — *(línea nueva del **31/08**)* | **NO se ha pedido, NO está diseñada, y NO está decidido quién la diseña ni quién la fabrica** | 🔴 **NO cubierta.** ⏸️ **04/09: SIN MOVIMIENTO — sigue sin dueño.** Ver `A8` |
| ~~**A9**~~ **Receptor RF del mando de relés** | ~~nunca se pidió: el mando iba a retirarse~~ · ~~31/08: SUBE DE PRIORIDAD~~ · ~~04/09: SE COMPRA con salida `NO`~~ | **NUNCA se compró, y ya NO se compra** | ⛔ **ANULADA EL 05/09 (`D-1`).** *«Ya no tenemos mandos de A y B, sólo la app, los quitamos»*. **NO es una pieza pendiente: es una decisión.** Cero unidades. 🛑 **El CÓDIGO del mando NO se retira** — Manual 2 §6. 📵 Consecuencia: **la app es la única vía de mando** (`D-16`) |
| 🆕 **A10** **microSD de las cámaras** | — *(línea nueva del **05/09**)* | **NO se han comprado** | 🟢 **DECIDIDA la compra el 05/09 por el responsable:** *«cada cámara tiene una micro, la metemos»*. **2 unidades, una por cámara.** ⚠️ **Capacidad, retención y continua-o-por-evento siguen abiertas** (`A-0` de `DECISIONES.md`): no bloquean comprar, **sí bloquean configurar**. Ver `A10` |
| ~~**B1–B2**~~ RTC en la placa STM32 | — | **NO se compraron** | ⛔ **ANULADAS.** El RTC se mudó a **A6**, colgado del `ESP32` |
| **B3–B4** Expansor y accesorios | — | **NO se compraron** | correcto: siguen **esperando al banco**, no se piden todavía |
| **E1** Resistencias **2K2** para las 5 entradas de campo | — *(línea nueva del **04/09**)* | **NO se ha pedido: es de la revisión V2 de la placa** | 🆕 **NO es una decisión tomada.** Es una **cuenta** para quien firme la V2. Ver bloque **E** |
| **E2** Diodo de potencia en lugar del **`D30`** | — *(línea nueva del **04/09**)* | **NO se ha pedido: es de la revisión V2 de la placa** | 🆕 **NO es una decisión tomada.** `D30` es un `1N4148` de 200 mA gobernando un `IRLZ44N`: **infradimensionado**. Ver bloque **E** |

### ✅ Antes de comprar un `ESP32` más: NO todos hablan SPP, y el grande es el que no

> 🟢 **31/08 — ESTA PREGUNTA YA ESTÁ CONTESTADA PARA LO QUE HAY EN ALMACÉN: es un `WROOM-32`
> clásico y habla SPP.** Este apartado **no se tacha**, y no es por costumbre: sigue siendo el
> criterio con el que se pide **el siguiente** módulo, y el error que describe —recibir un `S3`
> por pedir *«el mejor»*— es exactamente igual de posible mañana que ayer. Lo que cambia es que
> **ya no bloquea `A1′`**.

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
> ESP32-WROOM-32  (o -32D / -32E / -32U)  sobre placa DevKitC / NodeMCU
> Bluetooth Clasico BR/EDR + WiFi.  NO S3, NO C3, NO S2, NO C6, NO H2.
> MISMO FORMATO DE PINES que el que ya hay en almacen (30 o 38): CONTARLO ANTES.
> ```
>
> **Los 30 o 38 pines son de la placa, no del chip:** es el mismo `WROOM-32` con SPP en los dos
> casos, pero el de 38 saca más GPIO al conector y **la placa es más ancha**. Los dos sirven
> eléctricamente —lo único que hace falta es que saquen `GPIO16`, `GPIO17`, `GPIO21` y `GPIO22`—,
> pero **no son intercambiables en las hembrillas de la portadora `A8`**. 🛑 **Por eso, y sólo por
> eso, no se compra un módulo más hasta contar los que hay.**

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

> ✅ **CONTESTADO el 31/08 — y por un tercer método que no estaba en esta lista: la FICHA DEL
> ARTÍCULO COMPRADO.** Los dos métodos de arriba siguen siendo válidos y siguen sirviendo como
> comprobante de recepción de un lote nuevo; lo que este documento no había previsto es que **la
> respuesta ya estaba escrita en el papel de la compra**, sin necesidad de tener el módulo delante.
>
> ```text
> Fecha: 31/08/2026     Leido por: el responsable (ficha del articulo comprado)
> Metodo: [ ] serigrafia del blindaje   [ ] esptool chip_id   [X] ficha tecnica de compra
> Referencia leida: ESP32-WROOM-32  (Xtensa LX6 dual-core, BT v4.2 BR/EDR + BLE)
> Cuantos modulos hay en almacen: ______   <-- SIGUE SIN ANOTAR
> Habla SPP:  [X] SI -> sigue A1'   [ ] NO -> ver la fila de WiFi de abajo
> ```
>
> 🛑 **Lo que SIGUE en blanco y todavía importa: cuántos hay.** Se pide un módulo por poste (**2**).
> Si en almacén hay dos, no se compra ninguno; si hay uno, se compra uno **del mismo formato de
> pines** (ver el aviso de 30/38 en la cabecera). **La casilla vacía no se rellena de memoria: se
> cuenta.**

**Salieron ESP32 clásicos — y aun así NO son un cambio gratis:** un ESP32 con WiFi da picos de
**~500 mA**, y la alimentación de la tarjeta (`12 V → LM7805 → LM1117-3.3`) no los da: el `7805`
disiparía 3,5 W sin disipador y, si el riel de 3,3 V se hunde, **se reinicia el STM32 que gobierna
el semáforo**. Van con **fuente propia desde los 12 V y masa común, alimentación NO
compartida** (Manual 10 §1 y §2). Un `HC-05` (~40 mA) sí se alimentaba de la placa; **éste no**.
**Eso es la línea A5, y hoy sigue sin pedirse — es lo que BLOQUEA EL MONTAJE.**

> 🔴 **No se lea el cierre de `BLQ-1` como «lo del ESP32 ya está resuelto».** Lo que se cerró es
> **si el módulo sirve**. Lo que impide montarlo mañana son otras tres cosas, **todas de compra y
> ninguna de firmware**: **`A5`** (la fuente, que además cambió de tipo hoy), **`A8`** (la placa
> portadora, que ni siquiera tiene diseñador asignado) y **el conteo de pines** del módulo. Un
> bloqueante que se cierra **destapa** los que tenía detrás; no los cierra con él.

### 🔄 Decisión de obra del 28/08 — VIGENTE: el `ESP32` SUSTITUYE al módulo SPP

**Ya no se compran `HC-05` ni `JDY-30`. El `ESP32` es el módulo**, y además de la consola por celular
lleva colgado el reloj (A6). ~~Con él se retiran la pantalla LCD, los cuatro pulsadores y el mando de
relés, y las cámaras pasan a los pines que el mando deja libres en `J16` (A7).~~

> 🔴 **CORREGIDO EL 31/08 — esa frase llevaba dos cosas falsas, y una de ellas cambia lo que se
> compra.** Por decisión del responsable:
>
> * ~~**El mando de relés SE CONSERVA**, en los canales **A** y **B**. **Se retiran sólo los
>   pulsadores 3 y 4** (`PB14`/`PB15`), que son los que las cámaras necesitan.~~
>   🔧 **CADUCADO EL 05/09 (`D-1`) — la frase original del 28/08 acabó teniendo razón, pero por otro
>   camino: SE RETIRAN LOS CUATRO PULSADORES Y TAMBIÉN EL MANDO.** Lo que **no** se retira, y es
>   justo lo que la corrección del 31/08 sí acertó a proteger, **es el CÓDIGO** *(Manual 2 §6)*.
>   ✅ **Lo que sigue MEDIDO y no cambia:** los pines de cámara son `CAM_C_PIN` y `CAM_D_PIN`
>   —símbolos de `Maestro/include/pines.h`, `PB14` y `PB15`—, y `PB9`/`PB13` quedan **libres**.
> * **La pantalla no se «retira» en el sentido en que se leía aquí.** Su código sigue compilado;
>   lo que se hizo fue **dejar de conducir sus pines** (`PB3`/`PB4`/`PB5` pasan a
>   `U8X8_PIN_NONE`, `Maestro/src/lcd.cpp:74`) porque el `ESP32` ocupa `J17` y un reloj de SPI de
>   software pegado al RX/TX del módulo corrompe el enlace. **Para la compra el efecto es el
>   mismo** —no se compran pantallas—, pero para el manual del operario no lo es.
> * ~~**Y de ahí sale una línea de compra nueva: `A9`.** El mando que se conserva necesita **su
>   receptor RF**, que **nunca se compró**.~~ ⛔ **CADUCADO EL 05/09 (`D-1`): el mando ya no se
>   conserva como hardware, así que no necesita receptor. `A9` queda anulada, con cero unidades.**

> ~~**Se sigue pidiendo el módulo Bluetooth SPP dedicado (`HC-05` / `JDY-30`). El ESP32 queda como
> alternativa, no como sustituto.**~~ ⛔ **ANULADO el 28/08 por la decisión de obra de arriba.**
> Era la decisión de la 1.ª revisión de ese mismo día, y su motivo era bueno —el módulo dedicado
> deja intacta la decisión congelada del Manual 10 §1 y no obliga a rehacer el puente nativo de
> Android—. **Queda tachado y no borrado** para que quien lo relea dentro de un mes vea que se
> descartó a propósito, y no lo vuelva a proponer como novedad.

**Lo que NO cambia con la decisión nueva, y hay que mirar antes de gastar:**

* Un **ESP32 clásico** haciendo de puente SPP **cabe dentro del Manual 10 §1 sin reabrirlo** —sigue
  siendo Bluetooth Clásico SPP, que es lo que ese apartado congela—, **con dos condiciones: la
  referencia confirmada y su fuente propia**. ~~Las dos siguen sin cumplirse.~~ → **31/08: la
  primera SÍ se cumple** (`BLQ-1` cerrado, es un `WROOM-32`); **la segunda NO** — `A5` sigue sin
  pedirse.
* Ir por **WiFi** —que era lo único que quedaba si los módulos resultaban `S3`, `C3` o `S2`—
  **exigía reabrir por escrito el apartado 1 del Manual 10 antes de comprar y antes de programar**.
  🟢 **31/08: esa rama queda DESCARTADA, no aplazada** — el módulo habla `BR/EDR`. Se conserva
  escrita porque la regla sigue viva para cualquier cambio de transporte futuro, y **no se ablanda**:
  la vez que se cambió de transporte sin escribirlo, el resultado fue una app que no conectaba con
  nada.

> ⚠️ ~~**Y de ahí sale por qué el bloqueo de A1′ no es un trámite:** la decisión del 28/08 sustituye un
> módulo por otro **dando por hecho que el que llegó habla SPP**. Si resulta que no, lo que hay
> encima de la mesa no es una compra pendiente: es **un cambio de transporte** que hay que reabrir
> por escrito. **La misma lectura de serigrafía decide las dos cosas.**~~
>
> ✅ **RESUELTO el 31/08, y en la dirección buena: sí habla SPP.** El párrafo no se borra porque
> **su razonamiento era correcto** —el bloqueo no era un trámite, decidía entre una compra pendiente
> y un cambio de arquitectura—. Lo que estaba mal era **por dónde se buscaba la respuesta**: se
> exigió la serigrafía durante días teniendo la ficha del artículo a mano. Ver la lección de N-107
> en la cabecera.

---

## A · SE PIDE YA — 🟢 **desde el 31/08 NO queda ninguna fila bloqueada por `BLQ-1`**

> ✅ **La única fila que estaba bloqueada era `A1′`, y se desbloqueó el 31/08** con el cierre de
> `BLQ-1`: el módulo que llegó es un `ESP32-WROOM-32` clásico y habla SPP. ~~espera a que alguien
> lea qué referencia llegó (bloque **0**)~~ — **ya se leyó.**
>
> **El resto se pide sin esperar a nada** — y eso incluye `A5` y `A7`, del 28/08, y **`A8`, nueva del
> 31/08**. ⛔ **`A9` SALE del bloque el 05/09 (`D-1`: no se compra)** y ✅ **`A6` sale por lo
> contrario: ya está puesta.** 🆕 **Entra `A10`, las microSD.**
>
> 🛑 **Lo que sí queda condicionado, y no es lo mismo que bloqueado:**
>
> | | qué falta antes | qué NO impide |
> |---|---|---|
> | **`A1′`** | **contar los pines (30 o 38) y medir el ancho** de los módulos que ya hay | no impide usar los que hay ni escribir su firmware; **impide fabricar `A8`** |
> | **`A8`** | **decidir quién la diseña y quién la fabrica** — hoy no está decidido | — |
> | ~~**`A6`**~~ | ~~nada para pedirlo. **Cuántos** y qué reloj queda en el STM32 dependen del `Y2` (`B5`)~~ ⛔ **CADUCADO 05/09: son 2, están puestos, y `Y2` no decide nada** (`D-9`, `D-15`) | — |
> | 🆕 **`A10`** | **nada para comprarlas.** Lo que falta —retención, capacidad, continua o por evento (`A-0`)— **es para configurarlas** | no impide pedir las tarjetas |

| # | Qué | Cant. | Para qué | Especificación en |
|:---:|---|:---:|---|---|
| ~~A1~~ | ~~**Módulo Bluetooth SPP** `HC-05` / `JDY-30`~~ ⛔ **NO SE COMPRA.** El `ESP32` lo sustituye (decisión de obra del 28/08). *La fila no se borra: un hueco se vuelve a proponer, una fila tachada con su motivo no* | ~~2~~ → **0** | — | **Manual 10** §1 *(sigue mandando en el transporte: SPP, no BLE)* |
| **A1′** | **`ESP32` clásico** `WROOM-32` / `-32D` / `-32E` / `-32U` sobre placa **DevKitC / NodeMCU**. Hace de módulo SPP **y** sostiene el `DS3231` de A6. ⛔ **Ni «el más grande» ni «el más nuevo»: eso es un `S3` y no habla SPP — ver el aviso del bloque 0.** ⚠️ **El formato de pines (30 o 38) se CUENTA antes de pedir más y antes de fabricar `A8`** | **2** *(1 por poste)* — 🟢 **DESBLOQUEADA 31/08**, ~~🛑 BLOQUEADA~~. **Descontar lo que haya en almacén: la cantidad recibida sigue sin anotarse** | Consola de servicio por celular en cada poste (evita subir con escalera al Esclavo) **+ el bus I²C del reloj** | **Manual 10** §1 y §2 · `roadmap.md` **N-107** *(la ficha que cierra `BLQ-1`)* |
| A2 | **Cámara IA** Hikvision AcuSense varifocal motorizada. ~~`DS-2CD3643G2-LIZSU` **o equivalente**~~ ⛔ **era un modelo de REFERENCIA** → 🟢 **COMPRADA: `DS-2CD2683G2-IZS` (2,8–12 mm)**, 8 MP bullet AcuSense. **Ficha verificada el 05/09** | **2** *(ver nota)* | Demanda vehicular: una por poste, contacto seco a ~~`PB0`~~ → 🟢 **`J16`, en los pines donde estaban el Botón 3 y el Botón 4** — decidido por el responsable el 05/09, y **corrige lo que esta línea decía**: `PB0` es la bornera `J14`, que queda libre para un posible fin de carrera de barrera. ~~«Son las dos que el firmware lee hoy»~~ — **falso**: hasta el 05/09 el Modo Inteligente leía `PB0` y el vigilante miraba `J16`, o sea que **una mitad estaba ciega con cualquiera de las dos borneras**. ✅ **Tiene salida de alarma: `1 output`, `24 V`/`1 A` máx** — el cableado planeado sigue en pie | **Manual 9** §1.1 |
| A3 | **Antenas VHF y coaxiales** | **2 + 2** | Recuperar alcance: las genéricas de «LoRa» costaban 15–20 dB y dejaban la cobertura en 3 cuadras | **Manual 7** §BOM *(lleva modelo, conectores y adaptadores)* |
| A4 | **Módulo de 1 relé optoacoplado, con jumper `JD-VCC`** | **2** *(1 por poste)* | **La talanquera.** El firmware ya la manda (SFTY-28, 27/08) y la tarjeta ya expone la señal: se conecta a la bornera **`J15`** (red `Motor`, `PB2` → opto `U15` → MOSFET `Q10`). 🔴 **NO a `J14`, que es la ENTRADA de la cámara — ver la fe de erratas de la cabecera.** **No hace falta `PCF8574` ni MOSFET nuevo** | **Manual 13** §3 *(la etapa de potencia y el cuadro `J14`/`J15`)*; el jumper `JD-VCC` y su porqué, en el aviso del bloque **B** de esta misma lista |
| **A5** | 🔴 **Fuente propia para cada `ESP32`: convertidor DC-DC CONMUTADO (*switching*) reductor, `12 V → 5 V`, ≥ 1 A**, con sus borneras y su cable. **CONMUTADO, NO LINEAL — y no es una preferencia: ver la cuenta de abajo.** ~~(un módulo `LM2596` o `MP1584` sirve)~~ ⛔ **retirado el 31/08: ninguna referencia concreta está elegida** | **2** *(1 por `ESP32`)* | **Que el `ESP32` no cuelgue del `LM7805` de la tarjeta.** A 500 mA de pico el `7805` disipa 3,5 W sin disipador, y al hundirse el riel de 3,3 V **se reinicia el STM32 que gobierna el semáforo** | **Manual 10** §1 *(la regla)* — ⚠️ **la pieza sigue sin especificarse en ningún manual: ver el aviso de abajo** |
| **A6** | **Módulo RTC `DS3231` `ZS-042`** con **su propia pila**, colgado del `ESP32` por I²C (`GPIO21` SDA · `GPIO22` SCL) | ~~**1** *(el del Maestro)*~~ → **2** *(1 por poste)* — ✅ **05/09: YA ESTÁN PUESTOS, cada `ESP32` lleva el suyo.** Confirmado por el responsable. **Nada que pedir** | El reloj del equipo, **fuera de la placa STM32**: no hay que modificar la tarjeta ni sacar hilos de `PB0`/`PB8`. 🛑 **Y son DOS porque el STM32 NO tiene reloj** (`Y2` muerto, `D-9`) **y ya no atiende `SET_RTC`** (`D-15`): cada punta mantiene su hora | **Manual 11** *(la pieza)* · ⚠️ **el montaje sobre `ESP32` no está en ningún manual: ver el aviso de abajo** |
| **A7** | **Juego de conexión de las cámaras a `J16`**: conector hembra del footprint de `J16` con sus terminales de crimpar, y cable de 2 hilos apantallado por cámara. 🟢 **04/09: `M3` CERRADA — ya se puede CONECTAR, no sólo comprar.** ⚠️ **Y el juego incluye con qué TAPAR el p1 de 12 V** *(tapón, funda termorretráctil o el conector sin terminal en esa posición)*: no es opcional | **2 juegos** | Llevar el contacto seco de la cámara a los pines que **liberan los pulsadores 3 y 4** ~~el mando~~ (`PB14`/`PB15`), **contra los 3,3 V del borne contiguo** (`p9` para `p10`, `p11` para `p12`). **No hace falta `PCF8574` ni ninguna placa hija** | **Manual 13** §3 *(borneras)* · `03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md` §7 *(el mapa pin a pin de `J16`)* · **Manual 9** *(polaridad, `M3` y el `ENSAYO 4`)* |
| **A8** | 🔴 **PLACA PORTADORA DEL `ESP32`** — **línea nueva del 31/08; hasta hoy esto NO ERA UNA LÍNEA DE COMPRAS y hacía falta igual.** Lleva, como mínimo: **PCB**, **hembrillas** para el módulo (que es de formato protoboard y va enchufado, no soldado), **conectores** de entrada de 12 V y de salida a `J17`, **fusible**, **protección de inversión de polaridad** y **condensadores** de desacoplo y de reserva | **2** *(1 por poste)* — 🛑 **NO SE FABRICA todavía** | Que el `ESP32` y su `A5` y su `A6` sean **un conjunto montable y reemplazable**, en vez de tres módulos sueltos con cables volantes dentro de un armario que vibra en un remolque | 🔴 **`05_Funcional/19_Especificacion_Placa_Portadora_ESP32.md`** *(otro agente la está escribiendo en este mismo árbol — **aquí NO se duplica**: si al leer esto ese fichero no existe todavía, es que ese trabajo no ha entrado, y **se espera a que entre en vez de inventar la especificación aquí**)* |
| ~~**A9**~~ | ⛔ **RECEPTOR RF DEL MANDO DE RELÉS — NO SE PIDE. ANULADA EL 05/09 (`D-1`).** ~~🟢 SE PIDE, contacto seco MOMENTÁNEO, canales A y B, 12 V, salida `NO`~~ · ~~🛑 04/09: falta DECIDIR si `NO` o `NC`~~. **El responsable retiró el mando del equipo: *«ya no tenemos mandos de A y B, sólo la app, los quitamos»*.** **NO es una pieza pendiente ni una compra aplazada: es una decisión.** El razonamiento de `NO`/`NC` se conserva en el bloque de abajo porque **sigue gobernando el cableado de las CÁMARAS** | ~~2~~ → **0** | ⛔ **Nada.** ~~era la única vía de mando del Esclavo~~ → 📵 **eso no se resuelve comprando: pasa a ser `D-16`** — la app es la única superficie de mando, y **el Modo Degradado del Esclavo se queda sin vía de activación** *(Manual 2 §6, con el censo de las cuatro)* | 🛑 **Nada que especificar.** 🔴 **Y lo que NO se toca: el CÓDIGO del mando se queda** — retirarlo abre el veto de SFTY-21. Manual 2 §6 |
| 🆕 **A10** | **Tarjeta microSD para cada cámara** — **`high endurance` / de vigilancia**, no la de un teléfono: se escribe en bucle las 24 h y una tarjeta de consumo se agota. Formato admitido por la cámara: **microSD / SDHC / SDXC** | **2** *(1 por cámara)* | **Que exista soporte de accidentes y auditoría.** El controlador **no ve imagen** (`D-12`) y no la va a ver: las imágenes viven **en la cámara**. Sin tarjeta no hay ninguna, y **no se toca una línea de firmware** | ⚠️ **Capacidad, retención y modo de grabación NO están decididos** (`A-0`). 🔴 **Y la capacidad máxima tiene DOS fuentes que no coinciden:** ver el aviso de abajo |

> ## 📷 05/09 — `A2` YA NO ES GENÉRICA: LA CÁMARA COMPRADA ES UNA `DS-2CD2683G2-IZS`
>
> **Ficha oficial consultada el 05/09:** `DS-2CD2683G2-IZS_Datasheet_V5.5.113_20230303.pdf`,
> descargada de `hikvision.com`. El desglose completo, con el manual de usuario y la guía rápida
> citados página a página, está en **`9_Manual_Parametrizacion_Camara_IA.md` §1.1**.
>
> ✅ **Lo que confirma la compra y NO obliga a cambiar nada de esta lista:**
>
> | | |
> |---|---|
> | **Salida de alarma** | `1 input, **1 output**` — **el contacto seco existe**, y sus bornes son `1A`/`1B`, tal como `A7` y el Manual 9 ya decían |
> | **Régimen** | `24 V DC` / `24 V AC`, `1 A` máx. — **sobra** frente a los `3,3 V` y `330 µA` que le pide `J16` |
> | **Alimentación** | `12 V DC ± 25 %` → **`9` a `15 V`**: **cubre entero el vaivén de una batería de plomo**. No hace falta convertidor para la cámara |
>
> 🛑 **Y las TRES cosas nuevas que esta línea de compra arrastra, y que antes no estaban:**
>
> 1. **`13 W` POR CÁMARA, CONTINUOS** *(`12 V`, `1,08 A`; ficha pág. 4)*. Son **`26 Ah/día` por
>    poste** sólo de cámara — **una cifra que dimensiona batería y panel**, del mismo orden que todo
>    lo demás del equipo junto. **No es una decisión de esta lista: es un número para quien firme la
>    energía**, y hasta hoy no estaba escrito en ninguna parte.
>    ⚠️ **La cámara va DIRECTA A BATERÍA, no al `LM7805` de la tarjeta** — es la misma regla de `A5`.
> 2. **🆕 Falta una línea de compra: EL SOPORTE.** `1.385 g` y `30,8 cm` en lo alto de un poste
>    **móvil** son carga de viento y palanca, y **ningún documento de este proyecto especifica la
>    fijación**. Hikvision lista opcionales (`DS-1275ZJ-S-SUS` de poste vertical, `DS-1260ZJ` y
>    `DS-1280ZJ-S` de caja de conexiones; ficha pág. 5). 🔴 **SIN VERIFICAR cuál aplica: no se elige
>    aquí.**
> 3. ⚠️ **La página oficial del modelo lo marca como `Discontinued`** *(consultada el 05/09)*. **No
>    afecta a las dos unidades ya compradas**, pero **sí a repuestos y a una tercera cámara**: si el
>    diseño va a necesitar más *(la segunda por poste de SFTY-29 sigue sin pedirse)*, **conviene
>    decidirlo antes de que desaparezca del canal**.
>
> 🔴 **Y lo que NO se puede dar por bueno todavía, porque ninguna fuente oficial lo dice** *(detalle y
> cómo se cierra cada uno, en el Manual 9 §7 y §8)*:
>
> | 🔴 `SIN VERIFICAR` | por qué importa aquí |
> |---|---|
> | **Que la analítica pueda ENLAZARSE a la salida de alarma** | **Es el eslabón del que cuelga `A7` entera.** 🟢 **05/09 — MEJORA, leyendo el manual de usuario que ya está en `04_Manuales/`: el camino está documentado entero.** La regla de intrusión remite a `Linkage Method Settings` *(PDF pág. 61 · impresa 49)* y ahí figura **`Trigger Alarm Output`** *(PDF pág. 79 · impresa 67)*. 🔴 **Pero NO se cierra**: debajo sigue la nota *«only supported by certain models»*, y **ninguna fuente dice si esta cámara es uno de ellos**. **Se comprueba en 10 minutos** con la cámara delante — Manual 9 §4 Paso 0 |
> | Que la salida sea **`NO`/`NC` configurable** | La frase *«la salida de la AcuSense es configurable NO/NC»* circula por varios documentos **y no la sostiene ninguna fuente**: `Alarm Type` sólo está documentado para la **entrada**. **Lo mide el `ENSAYO 1`** |
> | **Corriente mínima de conmutación** del contacto | A `330 µA` es *carga seca*, régimen donde un contacto sin oro puede oxidarse con los meses. **Y la línea `E1` —2K2 en serie— la bajaría a `270 µA`: las dos cuentas hay que mirarlas JUNTAS** antes de firmar la V2 |
>
> 🟢 **Ninguna de las tres bloquea la compra —ya está hecha— ni el cableado que `M3` desbloqueó.**
> Bloquean **dar por cerrada la puesta en marcha**, que es distinto.
>
> ### 📜 QUÉ SE ESTÁ COMPRANDO DE VERDAD CON ESTAS DOS CÁMARAS (D-12, 05/09)
>
> **Se compran dos cámaras de 8 MP para que el semáforo reciba de cada una UN BIT.** Dicho así suena
> pobre, y por eso va escrito: es lo que hay, y quien firma la compra tiene que saberlo.
>
> | lo que el sistema **SÍ** obtiene | lo que **NO** obtiene, aunque la cámara lo tenga |
> |---|---|
> | **Un contacto seco por cámara** — dos hilos a `J16`, un bit: *hay vehículo* / *no hay* | ❌ **Ninguna imagen ni vídeo llega al controlador.** No hay red de la cámara a la tarjeta |
> | La **clasificación AcuSense** *(vehículo / persona)* hecha **dentro** de la cámara | ❌ **Ningún cómputo en el controlador**: el STM32 lee un pin. **No se compra ninguna Raspberry, Jetson, NVR ni PC** — y no hacen falta |
> | Robustez de intemperie `IP67`/`IK10`, `-30` a `60 °C` | ❌ **`ONVIF`, `ISAPI`, `SDK` y `RTSP` existen en la cámara y este sistema NO los usa.** Se pagan y no se aprovechan: es el precio de comprar una cámara de catálogo |
>
> 🟢 **PERO OJO CON DARLO POR PERDIDO — corregido el 05/09:** que el controlador no vea imágenes **no
> significa que no haya imágenes**. La cámara admite **microSD/SDHC/SDXC de hasta `512 GB`** *(ficha,
> pág. 3, verificada en el PDF)* y sabe **grabar y capturar por evento** *(ficha, pág. 4, fila
> `Linkage Method`)*. **El soporte de accidentes y la auditoría SÍ son posibles con la cámara ya
> comprada, y sin tocar una línea de firmware.**
>
> 🆕 **Y eso abrió DOS líneas, de las cuales UNA YA ESTÁ DECIDIDA:**
>
> 1. ~~**La tarjeta microSD** — 🔴 **NO comprada, no presupuestada.**~~ 🟢 **DECIDIDA EL 05/09 POR EL
>    RESPONSABLE: *«cada cámara tiene una micro, la metemos»*. SE COMPRAN, 2 unidades — es la línea
>    `A10` del bloque `A`.** Sigue siendo cierto lo que decía la frase vieja sobre **qué** tarjeta:
>    una de resistencia industrial (`high endurance`, pensada para grabación continua) **no es la de
>    un teléfono**.
>    🔴 **`SIN VERIFICAR` — la capacidad máxima tiene dos fuentes y no coinciden:** la **ficha oficial
>    del 03/03/2023** dice **`512 GB`** (p.3, verificada en el PDF); la **recopilación `.docx` del
>    05/09** dice **`256 GB`**, y **ésa NO es fuente del fabricante**. **Se compra mirando la ficha
>    del lote que llegue**, y ante la duda **una de 256 GB entra en las dos lecturas**.
>    ⚠️ **Lo que la decisión NO cierra y sigue en `A-0`:** cuántos **días de retención** hacen falta y
>    si la grabación va **continua o por evento**. De eso sale la capacidad; comprar sin decidirlo se
>    puede, **configurar la cámara no**.
> 2. **Quién y cómo las recupera** — 🔴 **SIGUE SIN DUEÑO.** Sin red en el poste, **las imágenes se
>    sacan subiendo a retirar la tarjeta**. La alternativa *(FTP o NAS)* **exige red hasta el poste,
>    que no existe y no está presupuestada**. **Comprar la tarjeta no resuelve esto**, y conviene no
>    confundir las dos cosas: hay soporte de accidentes **el día que alguien suba a por él**.
>
> 🛑 **Y una decisión que no es de compras y no puede quedarse sin dueño:** cuántos días se guardan
> las imágenes, quién las mira, quién las borra y **qué pasa con la privacidad de quien pase por
> delante**. **Este documento no lo decide** — se anota para que no se descubra después de instalar.
>
> ⚠️ **Por último, la restricción que reparte el recurso escaso: `1 input, 1 output` (ficha, pág. 3).
> UNA salida por cámara = UN significado por cámara.** ~~Con dos cámaras hay **dos significados para
> todo el cruce**: uno ya está tomado *(presencia de vehículo)* y **el otro está sin decidir** (`A-1`
> de `DECISIONES.md`).~~
>
> 🔧 **CADUCADO EL 05/09: `A-1` está CERRADA por `D-13`, y la respuesta invierte el planteamiento.**
> **Las dos cámaras llevan la MISMA configuración** —*Intrusion Detection* sobre el barrido de la
> pluma— y **el significado lo pone el estado del semáforo, no la cámara**. Con una cámara por poste,
> «un significado por cámara» no era repartir: era **proteger un poste y el otro no**. **Un bit da
> cinco lecturas** sin gastar significados.
>
> **Lo que de esta nota sigue vigente, y por eso no se borra:** **comprar una tercera cámara no es la
> respuesta** — y menos con el modelo marcado `Discontinued`. Lo que hoy falta no es un significado
> más: es **la segunda cámara por poste de SFTY-29**, que sigue sin pedirse y sin firmware que la lea.

> 🔌 **Cómo queda montado lo que se pide en A1′, A5 y A6 — para que las tres líneas se lean juntas:**
>
> ```text
>    12 V de la caja
>       |
>       +---> LM7805 -> LM1117-3.3 -> STM32   (la tarjeta, SIN TOCAR)
>       |
>       |   .--- [A8] PLACA PORTADORA -----------------------------------.
>       |   |  fusible + proteccion de polaridad + condensadores          |
>       +---|-> [A5] DC-DC CONMUTADO 12V -> 5V, >=1A --> [A1'] ESP32     |
>           |                                    (en HEMBRILLAS, 5V/VIN) |
>           |                                             |              |
>           |                                             +-- GPIO21 SDA-+--> [A6]
>           |                                             +-- GPIO22 SCL-+    DS3231
>           |                                             +-- GND -------+    con su pila
>           |                                             |              |
>           '---------------------------------------------|--------------'
>                                                         |
>                                                         +-- TX / RX -> J17 p2/p3
>                                                         |              (PB7/PB6)
>                                                         +-- GND ------> J17 p7
>                                                                         MASA COMUN
>
>    MASA COMUN entre las dos ramas.  ALIMENTACION NO COMPARTIDA.
>    De J17 se usan la senal y la masa.  Sus 3,3 V NO alimentan al ESP32.
>    El ESP32 va ENCHUFADO en hembrillas: se reemplaza sin soldador, y por eso
>    hay que saber si es de 30 o de 38 pines ANTES de fabricar la portadora.
>
>    Y EN PARALELO, sin tocar nada de lo anterior:
>
>    ~~12 V ---> [A9] RECEPTOR RF DEL MANDO ---> contacto seco A --> J16 p5  (PB9)~~
>    ~~                                     '--> contacto seco B --> J16 p8  (PB13)~~
>       CADUCADO 05/09 (D-1): NO HAY MANDO.  J16 p5 y p8 quedan LIBRES, sin cablear.
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
> | Que la entrada sea **5 V por `VIN`** *(recomendada 5 V, límite 5,5 V; el módulo lleva su propio regulador a 3,3 V a bordo)* | 📄 **LEÍDO en la ficha del módulo comprado**, 31/08 — N-107 |
> | Que las E/S del módulo son de **3,3 V**, así que el enlace con el STM32 va directo | 📄 **LEÍDO en la misma ficha** — **no hace falta adaptador de niveles**, y por tanto no hay línea de compra para uno |
> | **Qué módulo concreto se compra** —referencia, corriente, si aislado o no, cómo se fija en la caja | 🔴 **SIGUE SIN ESTAR EN NINGÚN MANUAL.** Lo de esta fila es un **mínimo**, no una especificación, y **ninguna referencia está elegida** |
>
> ### 🔴 31/08 — **CONMUTADO, NO LINEAL**, y la razón es una cuenta, no una preferencia
>
> Con la entrada fijada en **5 V** por la ficha del módulo, un regulador **lineal** de 12 V a 5 V
> tiene que tirar 7 V por cada amperio. Y si alguien decide bajar directamente a 3,3 V, peor:
>
> ```text
>   LINEAL 12 V -> 3,3 V, a 500 mA de pico:  (12 - 3,3) x 0,5  =  4,35 W
>   LINEAL 12 V -> 5   V, a 500 mA de pico:  (12 - 5  ) x 0,5  =  3,50 W
>   CONMUTADO 12 V -> 5 V, ~85 % rend.:      perdida  ~ 0,44 W
> ```
>
> **4,35 W** en un encapsulado sin disipador, **dentro de un armario cerrado al sol**, no es una
> cifra de catálogo: es la temperatura a la que ese armario ya estaba antes de meterle nada.
>
> > ⚠️ **Y lo que hace peligrosa a esa disipación es CÓMO falla.** Un lineal caliente **no falla
> > limpio**: entra en limitación térmica, recorta corriente, el `ESP32` se reinicia, vuelve,
> > vuelve a calentar. **Lo que se ve desde fuera es un Bluetooth intermitente**, que es la avería
> > que no se diagnostica nunca porque aparece y desaparece con la hora del día y con el sol. Un
> > conmutado a 0,44 W ni siquiera llega a ese régimen.
>
> **Lo que se pide, escrito para copiar y pegar:**
>
> ```text
> Convertidor DC-DC CONMUTADO (step-down / buck), entrada 12 V nominal, salida 5 V.
> Corriente >= 1 A continuos.  NO lineal, NO AMS1117, NO 7805.
> Con bornera de tornillo o cable soldado.  Salida fija de 5 V, o ajustable AJUSTADA
> Y VERIFICADA CON MULTIMETRO ANTES DE ENCHUFAR EL MODULO.
> ```
>
> 🛑 **El ≥ 1 A no es el consumo: es el margen.** El pico medido de un `ESP32` con WiFi es ~500 mA;
> se pide el doble porque un conmutado al límite de su corriente es el que ondula, y la ondulación
> en el riel del módulo que hace de consola **es otra vez un Bluetooth intermitente**.
>
> ⚠️ **Si el módulo que se consiga es de salida AJUSTABLE, se ajusta y se mide ANTES de conectar el
> `ESP32`.** Vienen de fábrica con el potenciómetro en cualquier sitio, y el límite de `VIN` es
> **5,5 V**: un módulo que salga de la caja a 12 V lo destruye al primer contacto.
>
> **Se apunta el hueco en vez de taparlo con una referencia falsa**, que es la costumbre de este
> documento: el detalle de A5 tiene que acabar en el **Manual 10 §2**, junto al diagrama de conexión
> — y ese diagrama **hoy dibuja un módulo colgado del riel de la placa, que es justo lo que un
> `ESP32` no puede hacer**. **La topología ya está decidida; la referencia, no.**

> 🕐 **Sobre A6 —el reloj—, qué cambió y por qué abarata la obra:**
>
> * **Ya no se monta en la placa STM32.** Cuelga del `ESP32` por I²C (`GPIO21` SDA, `GPIO22` SCL) y
>   con **pila propia**. Eso quita de encima modificar la tarjeta y sacar hilos de `PB0`/`PB8`.
> * **Y por eso sale del bloque B:** el bloque B espera al veredicto del cristal `Y2` porque el RTC
>   iba a ir en la tarjeta. Colgado del `ESP32`, **esa dependencia desaparece para el reloj**.
>   ⏳ **Lo que NO se da por resuelto aquí:** si el diagnóstico del cristal (`B5` de `ESTADO.md`)
>   deja de hacer falta del todo, o solo deja de bloquear esta compra. **Eso lo cierra quien lleva el
>   banco**, no esta lista.
> * 🟠 **31/08 — Y sigue condicionado al `Y2`, aunque no de la forma en que lo estaba.** El
>   bloqueante **`BLQ-2` sigue abierto**: `N-17` y `N-37` midieron en banco que el cristal de
>   32.768 kHz **no oscila en la tarjeta medida**, y **la SEGUNDA tarjeta no se ha diagnosticado**.
>   La distinción, que hay que tener clara antes de pedir:
>
>   🔧 **05/09 — ESTA TABLA ESTÁ CADUCADA EN SUS TRES FILAS, Y SE TACHA CON SU MOTIVO PORQUE ES LA
>   QUE HIZO CREER QUE FALTABA COMPRAR UN RELOJ.** Lo que la deroga son `D-9` y `D-15`: **el STM32 no
>   tiene reloj y ya no atiende `SET_RTC`** — hay **dos relojes por cruce, uno por `ESP32`**, y **ya
>   están puestos** (`A-5`, resuelta el 05/09). **`Y2` no decide nada de esta línea.**
>
>   | | ~~¿lo decide `Y2`?~~ → **05/09** |
>   |---|---|
>   | **Comprar el primer `DS3231`** | ❌ **No, y además ya no hay nada que comprar: están los dos** |
>   | ~~**Si son 1 o 2 módulos**~~ | ⛔ **CADUCADA.** ~~Con el `Y2` del Esclavo sano, esa punta puede seguir tomando la hora del Maestro por radio y **1 basta**~~ → **falso desde `D-15`: el camino que sincronizaba el reloj del STM32 está derogado, y esa punta no tiene de dónde tomar la hora si no es de su propio `DS3231`. Son DOS, y son los que hay** |
>   | ~~**Qué reloj queda dentro del STM32**~~ | ⛔ **CADUCADA: NINGUNO.** ~~Reparar el cristal o pasar a reloj de software disciplinado por el `ESP32`~~ → **`D-9`: el STM32 no tiene reloj, y eso ya no es una avería pendiente sino la arquitectura.** Reparar `Y2` **no está en esta lista de compras** |
>
>   🛑 ~~**Y lo que ya se paga hoy, MEDIDO en el firmware:** con el `Y2` muerto, `CMD:PIN:…:SET_RTC`
>   responde **`$ERR,CMD:SET_RTC,DESC:SIN_CRISTAL…`** y **no pone la hora**.~~
>
>   🔧 **CADUCADO EL 05/09 POR `D-15`, y se comprueba en un `grep`: esa respuesta YA NO EXISTE.**
>   El STM32 **dejó de contestar a `SET_RTC`** —contestaba un segundo acuse a una sola orden—, y ahora
>   sólo deja constancia de que el que acusa es el puente:
>
>   ```text
>     $ grep -rn "SIN_CRISTAL" --include=*.cpp Maestro Esclavo
>     Maestro/src/bluetooth.cpp:309:// SIN_CRISTAL_VEA_CONSULTA_RELOJ y SIGUE_PARADO_VEA_CONSULTA_RELOJ nombran la consulta
>
>     $ grep -rn "SET_RTC_LO_ACUSA_EL_PUENTE" --include=*.cpp Maestro Esclavo
>     Maestro/src/bluetooth.cpp:670:    bluetooth_reportarEvento("APP_BLUETOOTH", "SET_RTC_LO_ACUSA_EL_PUENTE");
>     Esclavo/src/bluetooth.cpp:643:    bluetooth_reportarEvento("APP_BLUETOOTH", "SET_RTC_LO_ACUSA_EL_PUENTE");
>   ```
>
>   **La única coincidencia de `SIN_CRISTAL` que queda es un COMENTARIO.** *(La lección de método, que
>   es la que no caduca: aquella cita iba por número de línea —`:336`, `:268`— y las dos apuntan hoy a
>   otra cosa. `CLAUDE.md` §4.sexies: se cita el símbolo y se publica el `grep`.)*
>
>   ✅ **Lo que de aquel párrafo sigue siendo cierto y por eso no se borra entero:** antes se contestaba
>   `RESULT:OK` **sin haber puesto nada**, y el técnico se iba del poste creyendo que dejó el reloj
>   puesto. **Eso era un `$ACK` que no dependía de lo que la llamada devolvió**, y se arregló.
>
>   > 🟢 **CADUCADA ESA ÚLTIMA FRASE EL 04–05/09, Y SE TACHA EN VEZ DE BORRARSE PORQUE INVIERTE LA
>   > PRIORIDAD DE ESTA LÍNEA.** **El firmware del `ESP32` para este reloj YA ESTÁ ESCRITO, y el de
>   > los dos sentidos:**
>   >
>   > | | dónde | estado |
>   > |---|---|---|
>   > | **Leer y poner en hora el `DS3231`** | `ESP32_Expansion/src/reloj_ds3231.cpp` — completo, con barrera `OSF`, bit 12/24 h y validación por barrido | ✅ **escrito** |
>   > | **Atender `SET_RTC` contra SU reloj**, con las siete ramas colgando de lo que devolvió `reloj_ajustar()` | mismo módulo | ✅ **escrito** |
>   > | 🆕 **Publicar la hora hacia la app** (`N-145`, 05/09): el puente **sella el hueco `HORA:--:--:--`** que el STM32 declara y **recalcula el checksum** | `src/puente.cpp:198-245` | ✅ **escrito** |
>   >
>   > 🛑 **Y por eso `A6` deja de ser «una pieza pendiente» y pasa a ser LA ÚNICA COSA QUE FALTA PARA
>   > PODER PROBAR NADA DE ESTO:**
>   >
>   > * **Sin un `DS3231` en el bus, las tramas seguirán saliendo con `HORA:--:--:--`.** Eso es el
>   >   firmware **negándose a inventar** —cota 3 del sello—, **no el firmware fallando**. Quien lo
>   >   pruebe sin módulo **no puede concluir nada**.
>   > * 🔴 **La dirección I²C `0x68` está SIN VERIFICAR sobre el módulo real.** Es la del datasheet, y
>   >   lo dice el propio fuente: `ESP32_Expansion/include/contrato.h:185-188`.
>   > * 🔴 **`N-145` NO SE PUEDE DAR POR PROBADA** mientras `A6` no se compre y se conecte.
>   >
>   > ~~**La consecuencia de compras, en una línea: `A6` ya no espera a ningún diagnóstico — lo que
>   > espera es a que alguien la pida.**~~ 🟢 **CADUCADO EL 05/09: tampoco espera a que la pidan.
>   > ESTÁN PUESTOS LOS DOS**, y lo confirmó el responsable. **`A6` sale de la lista de pedido.**
>   > 🔴 **Lo que NO se cierra con eso:** `0x68` sigue **`SIN VERIFICAR` sobre el módulo real** —es la
>   > del datasheet, y lo declara el propio fuente en `ESP32_Expansion/include/contrato.h`—, y **nada
>   > de este camino se ha ejercido con un `DS3231` delante**. Que la pieza esté no es que funcione.
> * ~~**Cantidad 1, y el porqué:** el Esclavo **ya toma la hora del Maestro por radio** (`CMD_HORA_*`,
>   SFTY-23), así que no necesita reloj propio.~~
>   ⛔ **CADUCADO EL 05/09, Y ERA LA FILA QUE MÁS ENGAÑABA: son DOS.** El motivo tachado se apoyaba en
>   un camino que **`D-15` derogó** —la sincronización que ponía en hora el reloj **del STM32**—, y en
>   un reloj **que no existe**: `Y2` está muerto y el STM32 **no tiene ninguno** (`D-9`). **Cada punta
>   necesita el suyo, cada `ESP32` lo lleva, y los dos están montados.**
> * ~~🔴 **Sigue sin haber driver, y eso no es una avería.** Ver el aviso del bloque B, que se
>   mantiene entero: **al enchufarlo no dará la hora, porque no hay código que le hable** — ahora en
>   el `ESP32`, cuyo firmware tampoco está escrito.~~
>   → 🟢 **CADUCADO EL 04–05/09: el driver EXISTE** (`ESP32_Expansion/src/reloj_ds3231.cpp`) **y el
>   camino de vuelta hacia la app también** (`N-145`). Lo que **sí** sigue siendo cierto del aviso
>   viejo, y por eso no se borra entero: **al enchufarlo puede no dar la hora igualmente** — un
>   `ZS-042` con la pila equivocada, con el oscilador parado (`OSF`) o en modo 12 h **no entrega hora
>   válida, y el firmware entonces publica el hueco a propósito**. Ver el aviso de recepción de
>   abajo. 🔴 **Y nada de esto se ha ejercido sobre un módulo real: SIN VERIFICAR.**

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
> | ~~Hoy el firmware los declara `BOTON3`/`BOTON4` en `INPUT_PULLUP` (`botones.cpp:52-58`) y **no los lee como cámara**~~ | ⛔ **Falso desde el 31/08:** son `CAM_C_PIN`/`CAM_D_PIN`, `INPUT` pelado, activos en ALTO |
> | ~~Que en la tarjeta física esas vías lleguen donde dice el esquemático~~ | 🟢 **VERIFICADO EN BANCO el 04/09 (`M3`, paso 20):** `p10` **9,93 kΩ** y `p12` **9,94 kΩ** a masa, los dos a **0 V** con energía. **El netlist tenía razón: pull-down real de 10 kΩ** |
> | Que en reposo el equipo **no pida paso solo**, con y sin el cable puesto | 🟢 **VERIFICADO EN BANCO el 04/09 (paso 21):** cero falsas activaciones |
>
> 🟢 **04/09 — Y por eso esta línea cambia de estado: `A7` ya no es sólo «se compra el cable».** Con
> `M3` cerrada **se puede conectar**. La polaridad la decide el cobre: **contacto contra los 3,3 V
> del borne contiguo** (`p9` para `p10`, `p11` para `p12`), retorno de masa por `p2`, **activo en
> ALTO**. El procedimiento completo y el `ENSAYO 4` están en el **Manual 9**, y **no se duplican
> aquí**.
>
> 🔴 **`J16` LLEVA 12 V EN SU POSICIÓN 1** — es el único conector de señal de toda la tarjeta que los
> trae. **El contacto seco de la cámara va a los pines 10 y 12 con retorno por el 2. En el 1 no se
> conecta nada.** Es el mismo aviso por el que el módulo Bluetooth no entra en `J16` (Manual 10
> §2.2), y aquí vale igual.
>
> 🛑 **04/09 — y ese aviso sube de rango: TAPAR FÍSICAMENTE EL PIN 1 ES OBLIGATORIO EN CADA EQUIPO,
> ANTES DE CABLEAR.** No basta con «no conectar nada ahí». Las **5 entradas de campo van desnudas al
> pin del STM32** —sin serie, sin opto, sin clamp—, mientras las **9 salidas sí llevan 220 Ω y
> optoacoplador**; el 04/09 **una tarjeta Maestro quedó con un cortocircuito de 3,3 V a masa**. **Con
> qué taparlo entra en el juego de `A7`** —tapón, funda termorretráctil, o el conector con esa
> posición sin terminal—: cuesta céntimos y es lo único que hay entre un instalador y una avería.
> **La corrección de verdad es de placa, y está en el bloque `E`.**
>
> ⛔ **Y con esta línea el riesgo sube, porque a partir de ahora se enchufa algo en LOS DOS:** el
> `ESP32` en `J17` y las cámaras en `J16`. **Los dos conectores comparten footprint y son idénticos a
> la vista.** Intercambiarlos mete **12 V en el `UART` del `ESP32` y lo quema**. Antes de enchufar
> nada, **multímetro en la posición 1 contra masa: si da ≈ 12 V, ése es `J16` y ahí va la cámara; si
> no, es `J17` y ahí va el módulo.** Es la misma comprobación del Manual 2 §8, con dos cables en vez
> de uno.
>
> 🛑 **Y la regla de este documento manda también aquí: lo que se compra hoy es el cable y el
> conector, no una cámara más.** ~~El firmware **no lee `PB14`/`PB15` como entrada de cámara** —los
> lee como botones—~~ ⛔ **eso dejó de ser cierto el 31/08**, pero **la conclusión no cambia y el
> motivo hay que decirlo bien**: el firmware los lee como **cámaras de DEMANDA** —piden paso—, **no**
> como presencia. **La segunda cámara por poste (SFTY-29, presencia como veto) sigue sin pedirse
> hasta que exista el firmware que la lea así.** La ruta está decidida y **desde el 04/09 el cobre
> también** (`M3`); la función, no escrita.
>
> ⏳ **Y una precisión que no se inventa aquí:** la decisión del 28/08 dice *«las cámaras van a `J16`,
> los pines que libera el mando»*. **La de demanda que ya funciona entra por `PB0` / bornera `J14`,
> está medida y esta lista no la mueve.** Si la intención era **trasladar** también esa, hay que
> decirlo por escrito — cambia el Manual 9, el Manual 13 y el pack `camara_01_demanda`.

> 🧩 **Sobre A8 —la placa portadora—, que hasta el 31/08 no existía como línea de compras:**
>
> **El hueco no era de cantidad: era de categoría.** Esta lista tenía el módulo (`A1′`), su fuente
> (`A5`) y su reloj (`A6`) como tres líneas sueltas, y **nada que dijera sobre qué van montados**.
> Tres módulos con cables volantes dentro de un armario que viaja en un remolque no es un montaje:
> es tres conexiones esperando a soltarse, y la que se suelte va a ser la del enlace con el semáforo.
>
> | qué lleva | por qué está en la lista y no «se ve al montar» |
> |---|---|
> | **PCB** y **hembrillas** | El `ESP32` es de formato protoboard: va **enchufado**, no soldado, para poder cambiarlo en el poste sin soldador |
> | **Conector de entrada 12 V** y **conector de salida a `J17`** | Que el mazo se pueda desconectar entero sin desarmar el conjunto |
> | **Fusible** | El `ESP32` cuelga de los mismos 12 V que el semáforo. Un cortocircuito en el accesorio **no puede llevarse por delante al que gobierna las luces** |
> | **Protección de inversión de polaridad** | Es un montaje que alguien va a conectar subido a una escalera, con guantes, y los dos hilos son del mismo grosor |
> | **Condensadores** de desacoplo y de reserva | Los picos de ~500 mA del WiFi son justo lo que hunde un riel que sólo tiene el conmutado detrás |
>
> 🛑 **Y LO QUE DE VERDAD BLOQUEA ESTA LÍNEA NO ES EL DINERO: NO ESTÁ DECIDIDO QUIÉN LA DISEÑA NI
> QUIÉN LA FABRICA.** Ni el diseño (esquema + huella + fichero de fabricación) ni el proveedor
> (casa de PCB, taller local, o montaje sobre placa perforada) tienen dueño asignado hoy. **Es una
> decisión del responsable**, no un detalle técnico, y **mientras no se tome, `A1′`, `A5` y `A6` se
> pueden comprar pero no se pueden montar.**
>
> ⚠️ **La especificación NO se escribe aquí.** Vive —o va a vivir— en
> **`05_Funcional/19_Especificacion_Placa_Portadora_ESP32.md`**, que **otro agente está redactando
> en este mismo árbol**. Esta lista **referencia y no duplica**: dos copias de una especificación
> son dos versiones que alguien tiene que sincronizar a mano, que es el defecto que este documento
> lleva un mes cerrando. **Si al leer esto ese fichero no existe, ese trabajo no ha entrado todavía
> — y eso es un dato, no un permiso para inventarlo aquí.**
>
> ⏳ **Orden real de esta línea:** *(1)* contar los pines de los módulos que hay → *(2)* decidir
> diseñador y fabricante → *(3)* cerrar `19_…` → *(4)* fabricar. **Los pasos 1 y 2 se pueden hacer
> hoy y no dependen de nadie.**

> 🎛️ **Sobre A9 —el receptor del mando— y sus tres vueltas. ⛔ SE CIERRA EL 05/09: NO SE COMPRA.**
>
> 🛑 **LÉASE ESTO ANTES QUE EL RESTO DEL BLOQUE: todo lo que sigue es HISTÓRICO y no autoriza ninguna
> compra.** Se conserva entero por dos motivos —el razonamiento del *activo en ALTO* sigue
> gobernando el cableado de las cámaras, y una línea que ha cambiado de sentido tres veces no se
> borra en silencio— pero **la conclusión vigente es una sola: cero unidades.**
>
> | | qué se creía |
> |---|---|
> | hasta el **28/08** | el mando se retira entero → el receptor no hace falta *(bloque D)* |
> | el **31/08** | el mando se conserva en A y B → **el receptor SUBE DE PRIORIDAD** |
> | el **04/09** | se pide, con salida `NO` — la decide el cobre |
> | ⛔ el **05/09** | **`D-1`: el mando se retiró del equipo. NO SE COMPRA NINGUNO** |
>
> **El texto original del 31/08, conservado:** ~~el receptor deja de ser un accesorio de una función
> que se iba a quitar y pasa a ser la pieza de la que cuelga la única vía de mando de una de las dos
> puntas.~~ **Eso era cierto, y por eso `D-1` no es gratis: esa vía no se sustituye por otra, se
> pierde** — es `D-16`, y la tabla de abajo, que la medía, sigue siendo el mejor resumen de lo que se
> perdió.
>
> **La cuenta, MEDIDA sobre el fuente el 31/08, y es la que manda:**
>
> | punta | ¿mando por app? | ¿mando por pulsadores? | ~~¿mando por relés?~~ **05/09** |
> |---|---|---|---|
> | **Maestro** | ✅ **sí** — `SET_MODO:AUTO/MANUAL/AMBAR/MENU/ALCANCE/INTELIGENTE/DEGRADADO`, símbolo `SET_MODO` en `Maestro/src/bluetooth.cpp` | ❌ no — `botonAceptar()`/`botonCancelar()` devuelven **`false` siempre** | ⛔ **el hardware ya no existe** |
> | **Esclavo** | 🔴 **NO. No existe ni un solo `SET_MODO`** — `grep -c "SET_MODO" Esclavo/src/bluetooth.cpp` → **`0`** (**MEDIDO** el 31/08, **vuelto a medir el 05/09: sigue en `0`**). Lo que hay es `AMBAR_EMERGENCIA`, `CANCELAR_AMBAR`, `SOLICITAR_PASO` y `SET_RTC` | ❌ no — mismos dos `false`, y sus pines 3 y 4 **son cámaras** desde N-97 | ⛔ **ídem.** ~~🔴 ES LA ÚNICA~~ |
>
> 🔴 **Y así leída, la tabla dice hoy algo que el 31/08 no decía: en el Esclavo las TRES columnas
> están en rojo o en ⛔.** Esa punta **no tiene ninguna vía para entrar o salir del Modo Degradado**,
> y la cuarta —que el Maestro se lo ordene por radio— es imposible por diseño, porque *el radio
> muerto es justamente la razón de entrar al Degradado*. **No se resuelve comprando nada**: está
> escrito, con el censo completo, en el **Manual 2 §6**, y **es una decisión del responsable**, no de
> una lista de compras.
>
> > 🛑 **La consecuencia operativa, dicha entera: sin receptor RF en el Esclavo, entrar o salir del
> > Modo Degradado en esa punta obliga a SUBIR AL GABINETE.** Y ni así, porque lo que había allí
> > arriba era el menú de la pantalla, que hoy **no puede confirmar nada** (`botonAceptar()` es
> > `false`) y **no tiene display** (el `ESP32` ocupa `J17`). El censo llamador a llamador está
> > escrito en el propio firmware, en `Esclavo/src/botones.cpp`, y dice literalmente que en esa
> > punta *«el sustituto no es la app sino EL MANDO DE RELÉS»*.
> >
> > **Eso es exactamente lo que N-19 prometía evitar** —*«el técnico ya no tiene que subir con
> > escalera a 5 metros en el Esclavo»*—. Sin `A9`, esa promesa se cae para el Degradado.
>
> ### 🔬 04/09 — LO QUE EL BANCO MIDIÓ, Y QUE CIERRA ESTA LÍNEA
>
> **EL DEFECTO, EN PASADO — es lo que el banco encontró sobre el firmware `617bd00`:** el mando A/B
> **no se podía pulsar**. No era una avería del emisor ni un cable suelto: **no existía el gesto que
> lo activara**.
>
> ```text
>   R65 / R66  =  10 kOhm a masa   (el mismo cobre que las entradas de camara)
>       -> el pin se queda en 0,6 V
>       -> aquel firmware (INPUT_PULLUP, activo en BAJO) lo leia BAJO, permanente
>       -> el arranque lo sembraba como "flanco ya consumido"
>       -> NUNCA habia transicion.  Ninguna secuencia A.A.A ni B.B.B se componia.
>
>   El cobre pide ACTIVO EN ALTO:
>       contacto contra los 3,3 V de J16 p4  (para MANDO_A, p5 / PB9)
>       contacto contra los 3,3 V de J16 p7  (para MANDO_B, p8 / PB13)
> ```
>
> **EL ESTADO DE HOY, y hay que leerlo separado de lo de arriba:** 🟢 **el firmware está corregido en
> las dos puntas** —`346ea5f`: `pinMode(BOTON1/BOTON2, INPUT)` pelado y `digitalRead(...) == HIGH`,
> en `Maestro/src/botones.cpp:40`, `:160-161` y `Esclavo/src/botones.cpp:54`, `:178-179`—. 🔴 **Y
> está PENDIENTE de ejercer en tarjeta:** nadie ha visto todavía a este equipo obedecer un `A·A·A`,
> y no se prueba sobre la Maestro mientras siga el corto de N-116. **El fuente ya no es el
> bloqueante; la carga verificada sí.**
>
> ⚠️ **El gesto de prueba CAMBIÓ, y es lo que se lleva al banco:** ~~tocar `J16` p5 contra masa con
> un cable suelto~~ → **cerrar `p5` contra `p4` y `p8` contra `p7` (los 3,3 V del pin contiguo)**.
> El gesto viejo es el del paso 29, el que acabó con el Maestro caliente.
>
> **Es el mismo gesto que las cámaras** —`J16` tiene 3,3 V en `p4`, `p7`, `p9` y `p11`, y los cuatro
> pines de señal llevan 10 kΩ a masa + 100 nF—. Que el mando y la cámara pidan lo mismo no es
> casualidad: **es el mismo cobre**.
>
> ### 🟢 ~~LA DECISIÓN QUE HAY QUE TOMAR ANTES DE PEDIR: `NO` o `NC`~~ → **DECIDIDA: `NO`, Y LA DECIDE EL COBRE**
>
> 🛑 ~~**No se toma en esta lista: la lista mide y pone las dos consecuencias delante; la elección es
> del responsable.**~~ → **CADUCADO el 04/09, y se conserva el motivo por el que estuvo bloqueada:
> se creía una preferencia de operación, y no lo es — es una consecuencia de la placa.** La tabla de
> abajo se conserva entera porque es la que lo demuestra, y porque una decisión entre alternativas
> escritas sólo se puede revisar si las alternativas siguen escritas.
>
> **Las dos medidas que la cierran, y ninguna es una opinión:**
>
> 1. **`J16` tiene UN SOLO pin de masa en todo el conector: `p2`** —`MAPEO_TARJETA_KICAD.md:613`,
>    *«no hay ni una resistencia de `J16` a 3,3 V; las cuatro van a masa … hay un solo pin de `GND`
>    en todo `J16` (p2), no uno por botón»*—, mientras que **cada señal tiene 3,3 V en la posición
>    contigua** (`p4`, `p7`, `p9`, `p11`). Un contacto por botón **contra masa** exigiría **una masa
>    por botón**: no las hay. **El conector sólo admite el gesto contra los 3,3 V — activo en ALTO.**
> 2. **`NC` sería inseguro, y lo demuestra la propia tabla de abajo:** un canal caído o un receptor
>    sin alimentación **se leen como pulsación**, y con un vocabulario de secuencias eso compone
>    órdenes que nadie dio. **`NO` deja el fallo en reposo: el mando no manda nada.**
>
> **Y el firmware ya no es un condicional:** lee activo en ALTO en las dos puntas desde `346ea5f`.
>
> **Lo que la tabla comparaba, conservado:**
>
> | salida del receptor | qué hace el contacto | qué ve el pin | consecuencia |
> |---|---|---|---|
> | **`NO`** *(normalmente abierto)* | **abierto** en reposo, **cierra** al pulsar | reposo **0 V** (lo fija `R65`/`R66`); al pulsar sube a **3,3 V** | El pulso produce el **flanco de subida** que el firmware busca. **Un canal caído o un receptor sin alimentación quedan en reposo: el mando no manda nada** |
> | **`NC`** *(normalmente cerrado)* | **cerrado** en reposo, **abre** al pulsar | reposo **3,3 V** permanente; al pulsar cae a **0 V** | Es la lectura invertida. **Un canal caído o un receptor sin alimentación se leen como pulsación**, y con el vocabulario de secuencias eso puede componer una orden que nadie dio |
>
> ⚠️ **El dato que hay que tener delante al elegir, y que no es de compra sino de firmware:** el
> vocabulario del mando son **secuencias** (`A·A·A`, `B·B·B`, `A·B·A·B`) dentro de una ventana de
> **12 s**, y **`B·B·B` arma `ambarLocal`, que veta las órdenes de radio** (`CLAUDE.md` §3.ter). No
> es un pulsador cualquiera: es el que deja una punta en ámbar desobedeciendo a la otra.
>
> 🛑 **Y sigue en pie lo del *latch*:** un receptor con salida **enclavada** no genera tres flancos,
> así que **las secuencias no se reconocen nunca** — eso es independiente de `NO`/`NC`. Se pide
> **momentáneo**.
>
> ```text
> ~~DECISION DEL RESPONSABLE -- SIN TOMAR al 04/09/2026~~
> ~~Salida del receptor:  [ ] NO   [ ] NC~~     <-- CADUCADO: la decide el cobre, y es NO
>
> LO QUE SIGUE SIENDO UNA COMPROBACION DE OBRA, y no la decide esta lista:
> Emisor que hay en obra -- frecuencia: ________  codificacion: ________
> Empareja con el receptor elegido:  [ ] SI  [ ] NO
> ```
>
> **Qué se pide, y qué NO se sabe todavía:**
>
> ```text
> Receptor RF de 4 canales, SALIDAS DE CONTACTO SECO (no de nivel), MOMENTANEO,
> SALIDA NORMALMENTE ABIERTA (NO),
> alimentacion 12 V, emparejado con el emisor de mano que YA EXISTE en obra.
> Se cablean SOLO dos canales:  A -> J16 p5 (PB9)   y   B -> J16 p8 (PB13),
> cerrando CONTRA LOS 3,3 V de J16 p4 y p7  (ACTIVO EN ALTO, medido en banco 04/09).
> NO / NC:  <-- DECIDIDO: NO.  Lo decide el cobre -- una sola masa en el conector (p2),
>               3,3 V en cada posicion contigua, y NC daria pulsacion fantasma.
> Lo que SI falta antes de pedir:  mirar el emisor que ya hay (frecuencia y codificacion).
> ```
>

> | dato | nivel de prueba |
> |---|---|
> | El firmware espera **flancos de contacto** en `PB9`/`PB13`, y el vocabulario es `A·A·A`, `B·B·B` y `A·B·A·B` | ✅ **MEDIDO** en `Maestro/src/mando.cpp:201-238` y `Esclavo/src/mando.cpp:240-248` |
> | La ventana de las secuencias triples es de **12 s** (`VENTANA_TRIPLE_MS`, `mando.cpp:38`) | ✅ **MEDIDO** — **importa para la compra**: un receptor con enclavamiento (*latch*) en vez de pulso momentáneo **no genera tres flancos** y las secuencias no se reconocerán nunca |
> | Que **`R65`/`R66` dejan el pin en 0,6 V**, y que el firmware `617bd00` lo leía BAJO permanente sembrándolo como *flanco ya consumido* — **el mando A/B no se podía pulsar** | ✅ **MEDIDO EN BANCO** el 03–04/09. ⚠️ **Es el DEFECTO, en pasado** |
> | Que el cobre pide **activo en ALTO**: contacto contra los **3,3 V** de `J16` `p4` y `p7` | ✅ **MEDIDO EN BANCO** el 04/09 — mismo cobre que las entradas de cámara |
> | Que **`J16` tiene un solo pin de masa** (`p2`) y 3,3 V en cada posición contigua — por eso el gesto contra masa no es posible en este conector | ✅ **MEDIDO sobre el cobre**, `MAPEO_TARJETA_KICAD.md:613` y §7.bis |
> | Que **el firmware ya lee `INPUT` pelado y `== HIGH` en las dos puntas** | ✅ **MEDIDO POR LECTURA** en `346ea5f` (`Maestro/src/botones.cpp:40`, `:160-161`; `Esclavo/src/botones.cpp:54`, `:178-179`) |
> | Que **el mando responde a un `A·A·A` en la tarjeta** | ⛔ **NUNCA SE EJERCIÓ, Y YA NO SE VA A EJERCER.** Queda **`SIN VERIFICAR` definitivo**: el paso 29 se abortó, la Maestro tiene el corto de N-116 y **el 05/09 se retiró el mando**. No es una casilla pendiente: es una prueba **cancelada** |
> | ~~**Si el receptor se pide con salida `NO` o `NC`**~~ | ⛔ **AMORTIZADO EL 05/09: no hay receptor que pedir.** ✅ Lo que del razonamiento sigue vigente **para las cámaras**: los cuatro pines de `J16` son **activos en ALTO** |
> | ~~**Qué referencia concreta se compra**, y si el emisor que hay en obra empareja con ella~~ | ⛔ **SIN OBJETO (`D-1`).** No hay emisor: se retiró con el mando |
>
> 🛑 ~~**Antes de pedir: MIRAR EL EMISOR QUE YA HAY.**~~ ⛔ **CADUCADO EL 05/09: no hay nada que pedir
> ni emisor que mirar.** ✅ **La regla que lo sostenía sí se conserva, porque vale para toda esta
> lista:** una comprobación de cinco minutos con la pieza en la mano se hace **antes de gastar, no
> después**. Es la que sigue en pie para `A1′` —contar los pines— y para `A10` —mirar la ficha del
> lote de tarjetas que llegue—.

> **Nota sobre la cantidad de cámaras.** El diseño habla de **cuatro** (dos por poste: demanda y
> umbral), pero **el firmware solo lee las de demanda**: las de umbral quedaron retiradas en N-59
> porque el protocolo no tiene comando para mandar la cuenta del tramo al Maestro. **Las de umbral se
> piden cuando exista ese comando** (tarea `C1` de `ESTADO.md`), no antes.
>
> ~~`ESTADO.md` venía pidiendo **3** sin que ningún manual explique por qué —probablemente porque ya
> hay una comprada—. **Eso lo confirma el responsable antes de pedir:** si ya hay una en almacén, son
> 1 o 2; si no hay ninguna, son 2.~~
>
> 🔧 **CADUCADO EL 05/09, y es otra fila que sobrevivió a su propia respuesta: SON DOS Y YA ESTÁN
> COMPRADAS** — `DS-2CD2683G2-IZS`, `DECISIONES.md` `D-10`. **No hay nada que confirmar antes de
> pedir, porque no hay nada que pedir.** Lo que sí falta de `A2` es todo lo que viene **después** de
> la compra: el **soporte de fijación**, las **microSD** (`A10`) y las comprobaciones del **Manual 9
> §8**.
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
> liberan ~~el mando retirado~~ los pulsadores 3 y 4 retirados** (`PB14`/`PB15`, línea A7), **hoy no hay ninguna función pendiente que lo
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
| **Los cuatro pulsadores** *(`J16` p5 `PB9`, p8 `PB13`, p10 `PB14`, p12 `PB15`)* | ⛔ **RETIRADOS LOS CUATRO.** ~~SÓLO los 3 y 4~~ · ~~🟢 31/08: `PB9` y `PB13` NO se retiran, son `MANDO_A`/`MANDO_B`~~ → 🔧 **CADUCADO EL 05/09 (`D-1`): también se van los 1 y 2** | Los pines de los **3 y 4** pasan a las cámaras *(línea `A7`)*. **`p5` y `p8` quedan LIBRES y sin cablear** |
| **Mando de relés / su receptor RF** | ⛔ **RETIRADO — VUELVE A ESTE BLOQUE EL 05/09 (`D-1`).** ~~🟢 31/08: SALE DE ESTE BLOQUE, EL MANDO SE CONSERVA en los canales A y B~~ → **el responsable retiró el hardware: *«ya no tenemos mandos de A y B, sólo la app, los quitamos»***. **Cero unidades, y no es una compra aplazada** | ⚠️ **Nada que cancelar: el receptor nunca se compró.** 🛑 **Y lo que NO se retira: el CÓDIGO del mando** — borrarlo abre el veto de SFTY-21 *(Manual 2 §6)*. 📵 **Lo que sí se pierde es la vía de mando desde el suelo: `D-16`** |
| ~~`HC-05` / `JDY-30`~~ | ⛔ **ANULADO** — línea A1, sustituida por el `ESP32` (A1′) | Nunca llegaron |
| ~~`DS3231` + pila **en la placa STM32**~~ | 🔄 **NO anulado: MOVIDO.** Se compra igual, colgado del `ESP32` — línea **A6** | — |

> ⚠️ **Retirado del EQUIPO no es retirado del FIRMWARE, y confundirlo cuesta una sesión de banco.**
> ~~Medido el 28/08 en el fuente: `main.cpp:45` llama a `lcd_setup()`, que en `lcd.cpp:70` llama a
> `u8g2.begin()`; y `botones.cpp:52-58` declara los cuatro pines en `INPUT_PULLUP` y los lee con
> antirrebote. **El firmware de hoy sigue compilando y ejerciendo pantalla, menú y los cuatro
> botones.**~~
>
> 🟢 **AL DÍA EL 31/08 — el firmware se movió, y en la dirección segura. MEDIDO en el fuente:**
>
> | | 28/08 | **31/08** |
> |---|---|---|
> | `PB14`/`PB15` | `BOTON3`/`BOTON4`, `INPUT_PULLUP`, activos en BAJO | **`CAM_C_PIN`/`CAM_D_PIN`**, `INPUT` pelado, **activos en ALTO** (`pines.h:124-125`, `botones.cpp:156-157`) |
> | `botonAceptar()` / `botonCancelar()` | leían pin y ejecutaban | **devuelven `false` siempre** (`botones.cpp:280-281`) |
> | Pines de la pantalla `PB3`/`PB4`/`PB5` | conducidos por el bus SPI de software | **`U8X8_PIN_NONE`: ni un `pinMode` ni un `digitalWrite`** (`lcd.cpp:74`) |
>
> 🟢 **Y la cuarta fila, que llegó el 04/09 y completa el cuadro: `PB9`/`PB13` (`MANDO_A`/`MANDO_B`).**
> Hasta `617bd00` seguían en `INPUT_PULLUP` y activos en BAJO —el defecto que el banco cobró: el
> mando **no se podía pulsar**—. **Desde `346ea5f` son `INPUT` pelado y activos en ALTO en las dos
> puntas** (`Maestro/src/botones.cpp:40`, `:160-161`; `Esclavo/src/botones.cpp:54`, `:178-179`), o
> sea **los cuatro pines de `J16` leen ya como pide el cobre**. 🔴 **Pendiente de ejercer en
> tarjeta**: el arreglo está en el fuente, no en una carga verificada.
>
> **Y eso cierra el aviso que había aquí en la buena dirección:** ya **no** es cierto que un contacto
> seco de cámara en `PB14` entre como pulsación, porque ese pin ya no es un botón y `botonAceptar()`
> no puede ejecutar nada. ✅ **`A7` deja de estar condicionada al firmware** — ~~lo que sigue delante
> es **la medida `M3`**: de `PB14`/`PB15` **sólo lo dice el netlist y nadie lo ha medido**. **El cable
> se compra hoy; conectarlo espera a `M3`, no al firmware.**~~
>
> 🟢 **04/09 — `M3` CERRADA EN BANCO, y con ella cae el último condicionante de `A7`:** `p10`
> **9,93 kΩ** y `p12` **9,94 kΩ** a masa, los dos a **0 V** con energía (paso 20), y **cero falsas
> activaciones** en reposo con y sin el cable puesto (paso 21). El pull-down real existe: **el pin no
> flota y no hay demandas fantasma. El cable se compra Y se conecta** — con el **p1 de `J16`
> tapado**, que es lo único que queda por delante.
>
> 🛑 **Y la regla de orden no cambia** (`CLAUDE.md` §9.bis): **el firmware nuevo tiene que estar
> CARGADO EN LA TARJETA antes de que nadie enchufe nada en `J16`.** Un commit no protege de un
> destornillador, y **nada de esto ha pasado banco.**

---

## E · REVISIÓN **V2** DE LA TARJETA — 🆕 bloque nuevo del 04/09. **NO se pide hoy, y NO está decidido**

> **Este bloque existe porque el banco encontró dos cosas que no se arreglan comprando un accesorio:
> se arreglan cambiando la placa.** Y por eso **no van en el bloque A**: una línea de `A` se puede
> pedir mañana; éstas dependen de que alguien firme una **revisión V2 del cobre**, y hoy **nadie la
> ha firmado**.
>
> 🛑 **Lo de aquí abajo son CUENTAS, no decisiones.** Se escriben con sus números para que quien
> firme la placa no tenga que rehacerlas — **y para que no se ejecuten como si estuvieran aprobadas.**

| # | Qué | Cant. | La cuenta / el porqué | Estado |
|:---:|---|:---:|---|---|
| **E1** | **Resistencias de 2K2** *(0805, o el formato que use la placa)*, en **serie** con cada una de las **5 entradas de campo** | **5 por tarjeta** *(+ repuestos)* | Ver la cuenta de abajo: **2K2 protege y sigue leyendo; 4K7 ya no lee la cámara** | 🆕 **PROPUESTA, sin decidir.** Es para **quien firme la V2** |
| **E2** | **Diodo de potencia** *(Schottky del orden de **3 A** o más, referencia a elegir)* en lugar del **`D30`** | **1 por tarjeta** *(+ repuestos)* | `D30` es un **`1N4148` de 200 mA** haciendo de **diodo de rueda libre** de una salida de motor gobernada por un **`IRLZ44N`**. **Infradimensionado** | 🆕 **PROPUESTA, sin decidir.** La referencia concreta **la elige quien firme la V2** |

> ### 🧮 `E1` — por qué **2K2** y no otro número
>
> **El problema, medido en banco el 04/09:** las **5 entradas de campo van DESNUDAS al pin del
> STM32** —sin resistencia en serie, sin optoacoplador, sin clamp—, mientras que las **9 salidas de
> la placa sí llevan 220 Ω y optoacoplador**. **`J16` p1 lleva 12 V crudos** a dos posiciones de las
> entradas. **El 04/09 una tarjeta Maestro quedó con un cortocircuito de 3,3 V a masa.**
>
> **La cuenta, que es lo que hace que 2K2 sea el número y no una preferencia:**
>
> ```text
>   CON 2K2 EN SERIE
>     Accidente:  12 V en el pin
>                 corriente de inyeccion = (12 - 3,3) / 2200 = 3,6 mA
>                 el datasheet admite 5 mA  ->  CABE, con margen
>
>     Operacion normal:  contacto cerrado contra 3,3 V
>                 el pin se lee a 2,70 V
>                 VIH del STM32 = 2,31 V   ->  SE LEE ALTO, con margen
>
>   CON 4K7 EN SERIE
>     Protege mas... y el divisor contra los 10K de pull-down deja el pin
>     por debajo de VIH:  YA NO LEERIA LA CAMARA.
> ```
>
> **Las dos condiciones tiran en sentidos contrarios** —cuanto más protege, menos lee— y **2K2 es el
> valor que cumple las dos a la vez**. Ése es todo el contenido de esta línea: **el número está
> acotado por arriba y por abajo, no elegido a ojo.**
>
> ⚠️ **Lo que esta cuenta NO dice, y va escrito al lado para que nadie la lea como una aprobación:**
>
> * **No es una decisión tomada.** Cambia el cobre de las cinco entradas de las dos puntas.
> * **No se ha ejercido en banco con 2K2 montadas**: es una cuenta sobre el datasheet y sobre los
>   valores medidos, **no una medida con la resistencia puesta**.
> * **No sustituye a tapar el pin de 12 V de `J16`**, que es lo que protege **hoy**, en la V1, en
>   cada equipo. Ver `A7` y el **Manual 9**.
>
> ### ⚡ `E2` — por qué el `D30` no vale
>
> Un **`1N4148`** es un diodo de señal de **200 mA**. Puesto como **rueda libre** de una carga
> inductiva conmutada por un **`IRLZ44N`** —un MOSFET de potencia—, tiene que tragarse la corriente
> de la bobina en cada apagado. **Está infradimensionado**, y un diodo de rueda libre que se abre
> deja el pico inductivo contra el MOSFET.
>
> **Lo que hace falta: un diodo de potencia, Schottky del orden de 3 A o más.** 🛑 **La referencia
> concreta —tensión inversa, encapsulado, si va en la V2 o como parche en las tarjetas que ya
> existen— la elige quien firme la placa.** Esta lista **no la inventa**: es la costumbre de este
> documento apuntar el hueco en vez de taparlo con una referencia falsa.

> 🛑 **Y el aviso que cierra este bloque, porque es la trampa evidente:** `E1` y `E2` son baratos —
> resistencias y un diodo—, y por baratos parecen fáciles de aprobar. **Lo que cuestan no es la
> pieza: es una revisión de placa**, con su fabricación, su montaje y su prueba de banco. **Mientras
> la V2 no exista, la V1 se opera con el pin de 12 V tapado y sabiendo que las entradas van
> desnudas.**

---

## Resumen para autorizar — al **05/09/2026**

**Se pide hoy:** 2 antenas VHF con sus 2 coaxiales *(A3)* · 2 módulos de 1 relé con jumper `JD-VCC`
*(A4)* · 🔴 **2 fuentes DC-DC CONMUTADAS 12 V → 5 V ≥ 1 A** *(A5)* · **2 juegos de conector y cable
para `J16`, con qué tapar el p1 de 12 V incluido** *(A7)* · **2 placas portadoras del `ESP32`**
*(A8, y antes hay que decidir quién la diseña)* · 🆕 **2 tarjetas microSD `high endurance`, una por
cámara** *(A10)*.

**Lo que YA NO se pide, y cada uno por un motivo distinto:**

| | por qué sale |
|---|---|
| ~~2 cámaras AcuSense~~ *(A2)* | ✅ **ya compradas** — `DS-2CD2683G2-IZS` (`D-10`). Deja detrás una línea **sin pedir**: el **SOPORTE de fijación** |
| ~~1 módulo `DS3231`~~ *(A6)* | ✅ **ya están, y son DOS** — uno por `ESP32`, con pila propia, confirmado el 05/09 |
| ~~2 receptores RF de mando~~ *(A9)* | ⛔ **DECISIÓN, no aplazamiento: `D-1` retiró el mando del equipo.** Cero unidades |

⛔ **`A9` SALE DEFINITIVAMENTE del «se pide», y ésta es su tercera vuelta — por eso se escribe con la
fecha delante.** ~~🟢 04/09: `A9` VUELVE al «se pide», con la salida ya elegida.~~ **El 05/09 el
responsable retiró el mando**, así que la pregunta ya no es *con qué salida* sino que **no hay
compra**. ✅ **Lo que del razonamiento del 04/09 se conserva, porque gobierna el cableado de las
cámaras que SÍ se conectan:** `J16` tiene **una sola masa** (`p2`) y 3,3 V en cada posición contigua,
así que **las cuatro entradas son activas en ALTO**.

📵 **Y la línea que esta lista no puede pedir como electrónica pero sí como herramienta:** retirado el
mando, **la app es la única superficie de mando** (`D-16`). **Un segundo teléfono ya emparejado, con
su cable de carga**, cuesta menos que cualquier fila de esta tabla y es lo único que hay entre un
móvil descargado y un poste sin mando de ninguna clase.

🆕 **Y lo que NO se pide y no estaba antes: el bloque `E`** —`E1` resistencias de **2K2** para las
cinco entradas de campo, `E2` un **diodo de potencia** en lugar del `D30`—. **Son cuentas para quien
firme la revisión V2 de la placa, no compras autorizadas.**

### 🔴 Lo que HACE FALTA y NO ESTÁ PEDIDO — la lista corta, por orden de lo que bloquea

| # | Qué | Qué bloquea hoy |
|:---:|---|---|
| **A5** | **Fuente DC-DC CONMUTADA 12 V → 5 V, ≥ 1 A** ×2 | 🛑 **EL MONTAJE.** Sin ella el `ESP32` cuelga del `LM7805` y **reinicia el STM32 que gobierna el semáforo**. **Conmutada, no lineal**: un lineal disipa **4,35 W** y en un armario al sol **no falla limpio, falla caliente** |
| **A8** | **Placa portadora del `ESP32`** ×2 | 🛑 **EL MONTAJE**, y además **NO TIENE DUEÑO**: no está decidido quién la diseña ni quién la fabrica. **Decisión del responsable** |
| ~~**A9**~~ | ⛔ **NO SE PIDE (`D-1`, 05/09)** | ~~🛑 LA OPERACIÓN DEL ESCLAVO~~ → 📵 **ya no es una compra que falte: es `D-16`.** Esa punta **no tiene `SET_MODO` por Bluetooth** —`grep -c "SET_MODO" Esclavo/src/bluetooth.cpp` → **`0`**, remedido el 05/09— y sus cuatro pulsadores se han ido. **El Modo Degradado del Esclavo se queda sin vía de activación, y eso no se compra: se decide** *(Manual 2 §6)* |
| ~~**A6**~~ | ✅ **`DS3231` ×2 — YA PUESTOS**, uno por `ESP32`, con pila propia | ~~Se pide ya; **cuántos** lo decide el diagnóstico del `Y2`~~ ⛔ **CADUCADO: `Y2` no decide nada** (`D-9`, `D-15`). 🔴 **Lo que sigue abierto no es la compra sino la prueba:** `0x68` **`SIN VERIFICAR`** sobre el módulo real, y `N-145` sin ejercer |
| 🆕 **A10** | **microSD `high endurance` ×2**, una por cámara | 🟢 **Nada para comprarlas** *(decidido el 05/09)*. Lo que bloquean es **la configuración**: sin decidir retención y continua-o-por-evento no se puede parametrizar la grabación (`A-0`) |
| **A7** | Conector y cable para `J16` ×2 | 🟢 **Nada, ni para comprar ni para conectar: `M3` se cerró en banco el 04/09** —pull-down real de 10 kΩ—. ~~**Conectarlo** espera a la medida **`M3`**~~. **Lo único obligatorio es tapar el p1 de 12 V antes de enchufar** |
| **A2** · **A3** · **A4** | Cámaras, antenas, relés de talanquera | Nada. Siguen pendientes desde el 27/08 |
| **E1** · **E2** | 2K2 en las entradas de campo · diodo de potencia por el `D30` | 🆕 **No bloquean nada hoy porque no se piden: son de la V2 de la placa, y NADIE la ha firmado.** Lo que bloquean es que las 5 entradas siguen **desnudas** — se opera tapando el p1 |

> 🟢 **Y desde el 31/08 ya SÍ se puede usar el Bluetooth que hay.** ~~🛑 **NO se pide hoy ningún
> módulo de Bluetooth — de ninguna clase.** … el `ESP32` está BLOQUEADO hasta que alguien lea qué
> referencia llegó~~ — **se leyó: es un `WROOM-32` clásico, habla SPP, y `A1′` está desbloqueada.**
> Los 2 `HC-05` / `JDY-30` siguen **anulados** por la decisión de obra del 28/08.
>
> ⚠️ **Lo que sigue sin hacerse es CONTAR los que hay**, y si son de 30 o de 38 pines. **No se
> compra un módulo más hasta contarlos** — no porque no sirvan, sino porque un formato distinto no
> entra en las hembrillas de `A8`.

> ⚡ **Las líneas nuevas no son accesorios, y por eso están en el bloque A y no «para luego»:**
> **A5** impide que el `ESP32` hunda el riel y **reinicie el STM32 que gobierna el semáforo**;
> **A6** es el reloj, que deja de tocar la placa; **A7** son cuatro pesos de cable que evitan una
> placa hija entera; **A8** es lo que convierte tres módulos sueltos en algo montable dentro de un
> remolque que vibra; ~~y **A9** es lo único que le queda al Esclavo para que un operario pueda
> mandarle algo desde el suelo~~ ⛔ **05/09: `A9` NO se compra (`D-1`)** — y lo que decía esa frase
> **era cierto**, así que lo que queda no es una compra menos: es **una vía de mando menos**
> (`D-16`). 🆕 **Y entra `A10`, las microSD: son el único soporte de accidentes que va a tener este
> sistema, y no cuestan ni una línea de firmware.**

> 🛑 **NADA DE ESTO ES UN PERMISO PARA MONTAR NI PARA SUBIR A CAMPO.** En la calle corre la **V8.4**
> (`e303485`). Todo lo que este documento describe —cámaras, `ESP32`, reloj, mando— **está sin pasar
> banco**. Esta lista autoriza **comprar**, que es lo único que se puede hacer en paralelo; el
> montaje y la puesta en servicio siguen detrás de la sesión de banco, sin excepción.

**Se pide después del banco, y solo si aparece una función que lo necesite:** el `PCF8574P` con su
placa de expansión (B3, B4). ⚠️ **Sigue sin driver: se compra preparado, no funcionando** — y hoy
**no hay ninguna función pendiente que lo pida**.

**No se pide:** cámaras de umbral ni la segunda cámara por poste *(la ruta ya está —`J16`—, **y desde
el 04/09 el cobre también**; el firmware que las lea como presencia, no)*, `PCF8574` para talanqueras
ni para cámaras, pantalla LCD, **los CUATRO pulsadores de `J16`**, ~~ni mando de relés o su receptor
*(que nunca se compró)*~~ ~~🟢 corregido el 31/08: el receptor del mando SÍ hace falta — es la línea
`A9`~~ ~~🛑 y matizado el 04/09: hace falta, pero NO se pide hasta decidir `NO`/`NC`~~ → ⛔ **05/09,
`D-1`: NI MANDO DE RELÉS NI SU RECEPTOR NI LOS PULSADORES. Y no es «no se pide todavía»: es que no se
va a pedir.** Tampoco se piden **`E1`** ni **`E2`**: son de la **V2** de la placa y no están
decididas. Las talanqueras salen por la salida **`Motor` (bornera `J15`, MOSFET `Q10`)** que **la
tarjeta ya trae**, y las cámaras de demanda por `PB0` (bornera `J14`), que ya se lee.

> 🛑 **Y para que esta línea no vuelva a girar por cuarta vez: lo que se retiró es el HARDWARE.** El
> **código** del mando **se queda entero** en las dos puntas, y no es sentimentalismo: `ambarLocal`
> tiene **un solo armador** —`Esclavo/src/mando.cpp`, `ambarLocal = true;`— y **cinco lectores que lo
> usan para vetar**. Borrarlo no deja el veto de SFTY-21 inerte: **lo deja abierto, y ningún test
> falla.** Desarrollo en el **Manual 2 §6**.

> ✏️ **Corregido el 28/08 (2.ª rev.):** este párrafo decía *«las talanqueras salen por la salida
> `Puerta`»*. **`Puerta` es la red de ENTRADA de la cámara** (`J14`), no la de la pluma. La salida de
> la talanquera es la red **`Motor`** (`J15`). Es la misma errata de la fila A4, y aquí estaba
> repetida con el nombre de la red en vez del de la bornera — que es justo como una corrección se
> deja a medias. Ver la fe de erratas de la cabecera.
