# 📁 DOCUMENTACIÓN FUNCIONAL Y MANUALES DE CAMPO (05_Funcional)

Esta carpeta centraliza los manuales de operación, guía de cableado, protocolo de pruebas y configuraciones de radio para el personal funcional y técnicos en terreno.

**Última revisión de este índice: 7 de septiembre de 2026** *(11/09: tocado sólo para `D-27` —
`J14` libre, cuatro cámaras compradas, la configuración de cámara sale del manual del modelo—)*.
🔴 **Y la regla que hay que leer antes que ningún manual: manda [`DECISIONES.md`](../DECISIONES.md).**
Los documentos de esta carpeta congelan la foto del día en que se escribieron; esa tabla es la que se
mantiene. **Donde un manual y `DECISIONES.md` no digan lo mismo, gana `DECISIONES.md`** — y si la
contradicción es sobre **cobre, conectores o pines**, gana además
[`17_Arquitectura_28-08_y_Decisiones_Abiertas.md`](17_Arquitectura_28-08_y_Decisiones_Abiertas.md),
que es donde se anotan las medidas.

---

> ## 🚨 EMPIECE POR AQUÍ
>
> **0.** **Antes que nada, abra [`DECISIONES.md`](../DECISIONES.md).** Es la tabla que manda sobre
> todos los documentos de esta carpeta: donde un manual y esa tabla no digan lo mismo, **gana la
> tabla**.
> **1.** Reconfigure las **2 radios** —enlace directo, **sin repetidor**— a `2.4 kbps` de Air Data
> Rate → **[`4_Manual_Configuracion_Radios.md`](4_Manual_Configuracion_Radios.md)**
> *(~~«las 4 radios»~~ — **corregido el 07/09**: la configuración vigente son **2 radios en enlace
> directo**, como dice la cabecera de ese mismo manual y `CLAUDE.md` §3. El repetidor está fuera de
> la configuración vigente. ~~⚠️ El CUERPO de `4_Manual` todavía dice «cambiar en las 4 radios» en
> dos sitios: haga caso a su cabecera.~~ ✅ **Los dos sitios se corrigieron en el propio manual el
> 07/09** — ya no hay que hacerle caso a la cabecera contra su cuerpo.)*
> **2.** Cargue el firmware **en las dos tarjetas, la MISMA versión** → [`2_Manual_Hardware_y_Pruebas.md`](2_Manual_Hardware_y_Pruebas.md) §4
> **3.** Ejecute el checklist y firme el acta → **[`3_Protocolo_Pruebas_Rigurosas.md`](3_Protocolo_Pruebas_Rigurosas.md)**
> **4.** 🔧 **Si va a una SESIÓN DE BANCO, el protocolo vigente y ÚNICO es la
> [`Guia_Cableado_y_Pruebas_Banco.html`](Guia_Cableado_y_Pruebas_Banco.html)** — 44 pasos, se abre en
> el navegador, **se rellena y se devuelve en PDF**. *(El `ENCARGO_SESION_BANCO.md` de 29 pasos se
> archivó el 07/09 en [`historico/`](historico/ENCARGO_SESION_BANCO_ARCHIVADO_07-09.md): se conserva
> como **acta** de la 1.ª noche, no como guion. **Un solo encargo vigente.**)*
> ⛔ *(Las dos guías «de 4 cámaras del Sisga» del 10/09 se **RETIRARON el 11/09** a
> [`historico/`](historico/): afirmaban que una cámara protege la pluma y que la pluma solo sube con
> verde, y las dos cosas son falsas en el firmware. ~~La cámara se cablea por el **Manual 9** y la
> decisión vigente `D-13`: **una por poste**.~~ Veredicto completo en `roadmap.md` §3.16.)*
>
> **5.** 📷 **Si va a montar las CÁMARAS o la TALANQUERA del Sisga, la guía de campo es la
> [`Camaras_Sisga_4x.html`](Camaras_Sisga_4x.html) — versión corregida del 11/09**, que sustituye a
> la del 10/09 y empieza por su **fe de erratas**. ✏️ **11/09, `D-25`:** las conexiones de aquella
> guía quedan **definitivas**: **cuatro cámaras, DOS POR POSTE** —cámara 1 entre `J16` p9 (3,3 V) y
> **p10** (`CAM_C_PIN`), cámara 2 entre `J16` p11 (3,3 V) y **p12** (`CAM_D_PIN`)— y la
> **talanquera en `J15`** por relé a la entrada `OPEN` de la centralita. La configuración de cada
> cámara, por el **Manual 9** y `D-13` (que sigue vigente en todo lo demás). Lo que el que cablea
> tiene que saber, medido en el firmware: **las dos cámaras de un poste hacen LO MISMO**, **ninguna
> frena la pluma** (baja con un coche debajo) y **la app dice `OK — las dos ven` en cuanto detecta
> CUALQUIERA de las dos**, aunque la otra no haya detectado nunca: **cada cámara se comprueba con
> el multímetro en su borne** (paso 11 de la guía).
>
> El paso 1 **no es opcional**: es la corrección de la causa raíz del fallo de comunicación que aparecía
> al paso de cada ciclo, en los tres modos, y del repetidor que no enlazaba. Sin él, el firmware nuevo
> no resuelve el síntoma.

---

> ## 📌 EL SISTEMA CAMBIÓ EL 01/08/2026 — LEA ESTO ANTES DE USAR LOS MANUALES
>
> Los manuales se escribieron cuando el equipo tenía **4 opciones de menú en una lista plana y ningún
> modo por reloj**. Ya no es así:
>
> | Novedad | Dónde está documentada |
> |---|---|
> | ~~**Menú en dos niveles** (`CONFIGURACION` como cuarta opción)~~ ⛔ **`D-17.bis` (05/09): LA PANTALLA Y EL MENÚ SE RETIRAN DEL EQUIPO.** *(Del equipo, no del código: `lcd.cpp` compila y `Validacion_LCD` sigue en `271/271` sobre un framebuffer del PC.)* **Y ya no se podía navegar igualmente:** `botonAceptar()`/`botonCancelar()` son `return false;` | `1_Manual_Usuario.md §3` |
> | ~~Pantalla **AJUSTAR HORA** y **sincronización horaria por radio**~~ ⛔ **DOS COSAS CAÍDAS:** la pantalla, por `D-17.bis` *(`MODO_HORA` tenía su único armador en el menú)*; y el reloj, por **`D-9`/`D-15`** — **la hora la lleva el `DS3231` del `ESP32` de cada poste**, el STM32 no tiene ninguno y **ya no contesta a `SET_RTC`**. Se consulta con **`CMD:LEER_RTC`** (`D-17`). 🔴 **07/09 — `D-20` corrige la SEGUNDA mitad: la sincronización por radio NO está caída, está sin destinatario.** Con `D-20`, **la autoridad de la hora es el ESP32 y es UNA sola** —la del poste 1—, y como los dos ESP32 no se hablan, **la hora vuelve a viajar por la radio**: `ESP32-M -> STM32-M -> radio -> STM32-E -> ESP32-E`, con los STM32 de **carteros**. **Y la app NO pone la hora en el poste 2, nunca.** *(Decidida y **SIN CONSTRUIR**.)* | `11_Manual_Instalacion_RTC_DS3231_Bateria.md` §4.1 · `4_Manual_Configuracion_Radios.md` §5 |
> | 🔴 **`D-20` — la hora del POSTE 2 se pone en la PUESTA EN MARCHA, no durante la avería** *(con la radio muerta no hay camino a su reloj; no importa, porque su `DS3231` tiene pila y conserva la hora — **perder la radio no es perder la hora**)* | `1_Manual_Usuario.md §3` · `11_…` (apartado final) |
> | **MODO DEGRADADO** — operación por reloj sin radio | **`8_Procedimiento_Modo_Degradado.md`** |
> | ~~**Mando de 2 canales (`A`/`B`)** y sus secuencias desde el piso~~ ⛔ **`D-1` (05/09): EL MANDO NO EXISTE.** *Su código se conserva a propósito —`mando_ambarLocal()` sostiene el veto de SFTY-21— pero no hay emisor, ni pulsadores, ni receptor RF.* **El equipo se opera SÓLO POR APP** (`D-16`) | `1_Manual_Usuario.md §6` · `2_Manual_Hardware_y_Pruebas.md §6` |
> | ~~El **Esclavo ahora tiene pantalla y menú propios**~~ ⛔ **`D-17.bis`: se retiran del equipo, en las dos puntas.** Lo que el Esclavo tiene hoy son **siete comandos por app**, uno de ellos `SET_MODO:DEGRADADO` (`D-18`) | `1_Manual_Usuario.md §7` |
> | **Pila `CR2032`** del reloj en ambas tarjetas — ⚠️ **es la del `VBAT` del STM32, cuyo cristal `Y2` está MUERTO.** No confundir con la pila del `DS3231` del `ESP32`, que es **otra pila en otro equipo** y puede tener que ser `LIR2032` recargable | `2_Manual_Hardware_y_Pruebas.md §5` · `11_…` §5.2 |

> ## 🛑 LOS TRES AVISOS DE COBRE QUE NO PUEDEN FALTAR EN NINGÚN DOCUMENTO DE ESTA CARPETA
>
> *(Añadidos aquí el 07/09 porque este README es el que dice «EMPIECE POR AQUÍ», y los tres hieren a
> una persona o destruyen una tarjeta. La fuente es `DECISIONES.md` y `17_Arquitectura…md`.)*
>
> 1. 🔴 **`J16` p1 lleva 12 V CRUDOS** a un conector de señal **directa** al micro —sin opto, sin
>    serie, sin clamp—. **Taparlo es OBLIGATORIO en cada equipo que se monte** (`D-4`, `N-120`), **no
>    una cautela de banco**. Un contacto de `p1` a `p10` o `p12` mete 12 V en una pata de 3,3 V.
> 2. 🔴 **`J14` es una ENTRADA del micro** (`PB0`, con `R64` + `C25`). **La salida de talanquera es
>    `J15`** (`PB2` → opto `U15` → MOSFET `Q10`). **Un relé de 12 V cableado a `J14` destruye el
>    STM32 que gobierna el semáforo — y además la pluma no se movería.** ⚠️ **Y `J16` y `J17`
>    comparten footprint y son idénticos a la vista:** antes de enchufar nada, **multímetro en la
>    posición 1 contra masa — si da ≈12 V es `J16`.**
> 3. 🔴 **`J16` p5 y p8 están VACÍOS y el código del mando SIGUE leyendo sus flancos** (`D-1`,
>    `A-2`). **Nada se cablea ahí, ni para probar:** cualquier cosa compone secuencias del mando sin
>    que nadie lo pida —`B·B·B` es ámbar local, `A·B·A·B` es entrar en Degradado—, y **`p4` es
>    adyacente a `p5`**, así que un puente corrido una posición pone 3,3 V contra masa: es el
>    candidato del sobrecalentamiento que abortó el paso 29 del banco.
>
> 🛑 **Y la regla de orden, que no es «van en el mismo commit» porque un commit no protege de un
> destornillador: el firmware nuevo tiene que estar CARGADO Y VERIFICADO EN LA TARJETA antes de que
> nadie enchufe nada en `J16`.** Se exige la carga verificada, no el merge.
>
> **La versión que corre en campo hoy es la V8.4**, que **no** incluye nada de lo anterior. Todo lo
> nuevo está construido y validado en simulador, pero **sin prueba de banco ni de campo**. Las
> Secciones 7 a 10 del protocolo de pruebas son su primera verificación física.

> ## ⚠️ SI VA A OPERAR EL MODO DEGRADADO
>
> **Lea completo el [`8_Procedimiento_Modo_Degradado.md`](8_Procedimiento_Modo_Degradado.md) antes de
> tocar nada.** Ese modo da verde **sin confirmación del otro extremo** —con el radio muerto es
> inevitable— y tiene riesgos residuales que el cliente aceptó por escrito. Tres cosas que hay que
> saber de entrada:
>
> - Se activa **en las dos puntas** y **exige verificación visual de ambas**, al entrar **y al salir**.
> - **Se pide POR APP en las dos puntas, con la misma orden:** `CMD:PIN:1234:SET_MODO:DEGRADADO`
>   (`DECISIONES.md` `D-18`). ~~Hoy el Esclavo no tiene receptor de mando: activarlo en esa punta
>   **obliga a subir al gabinete**.~~ 🔴 **Corregido el 07/09: no hay que subir a ningún gabinete, y
>   el mando no existe (`D-1`).**
> - 🛑 **`D-16`: sin teléfono no hay forma de operar el equipo.** No es una avería; es una propiedad
>   declarada del sistema desde que se retiró el mando.
> - **Límite duro de 48 h:** pasado ese tiempo sin resincronizar, el modo **cae solo a ámbar**.

---

> ## 📷 CÓMO DETECTA VEHÍCULOS EL EQUIPO — CORRECCIÓN DEL 28/08/2026
>
> **Este README anunciaba «integración con visión artificial (YOLOv8)». Se corrige, y queda escrito
> qué decía**, porque lo que se borra en silencio se vuelve a proponer.
>
> **Lo que el firmware hace de verdad hoy: lee CONTACTOS SECOS, y nada más.** La analítica de vídeo
> la hace **el DSP de la propia cámara Hikvision AcuSense**; el equipo solo ve un pulso de 1 segundo.
>
> **Desde el 31/08 son TRES entradas por punta, no una** —este README decía *«lee UN contacto seco
> en `PB0`»*—:
>
> | entrada | pin | bornera | antirrebote de placa | estado |
> |---|---|---|---|---|
> | `CAM_C_PIN` | `PB14` | `J16` p10 | ~~❌ ninguno~~ *(pull-down `R67` 10 kΩ ✅; 11/09: el netlist trae además `C28` 100 nF, sin medir en cobre)* | ~~🔵 **AQUÍ VA LA CÁMARA DEL POSTE 1** (`D-2`/`D-3`)~~ → ✏️ **11/09, `D-25`: la CÁMARA 1 DE CADA POSTE**, contra `p9`. ~~NO cablear hasta `M3`~~ — `M3` cerrada el 03/09 |
> | `CAM_D_PIN` | `PB15` | `J16` p12 | ~~❌ ninguno~~ *(pull-down `R68` 10 kΩ ✅; 11/09: el netlist trae además `C29` 100 nF, sin medir en cobre)* | ~~🔵 **AQUÍ VA LA CÁMARA DEL POSTE 2** (`D-2`/`D-3`)~~ → ✏️ **11/09, `D-25`: la CÁMARA 2 DE CADA POSTE**, contra `p11`. **Hace lo mismo que la de `p10`** (`CAM_J16[2]`, un solo bucle en `camaras_actualizar()`). ⚠️ `p12` es el borne con **menos separación a la red de 12 V** (`1,359 mm`, `17_` §1.7): `p1` tapado, trabajo limpio |
> | `CAM_DEMANDA_PIN` | `PB0` | `J14` | ✅ `R64` 10 kΩ + `C25` 100 nF | ~~✅ **cableable hoy**~~ → 🟠 **VIVO Y SIN CÁMARA.** El firmware lo sigue leyendo, pero **`CAM_CIEGA`/`CAM_PEGADA` NO lo vigilan**: es el único borne sin aviso de avería. ~~Reservado a fin de carrera de barrera (`A-2`). 🔴 **CONFLICTO ABIERTO (11/09), del responsable:** `A-2` manda aquí el fin de carrera, pero~~ el firmware **lee `PB0` como cámara de demanda** —el Maestro por nivel en el Modo Inteligente, el Esclavo por flanco y lo manda por radio—, así que un fin de carrera en `J14` **pediría paso cada vez que se mueve la pluma**. ~~Antes de cablear nada aquí, pregunte al responsable~~ → 🟢 **`D-27` (11/09): `J14` LIBRE, sin cablear — el fin de carrera no se instala en este despliegue. En `J14` no se conecta nada** |
>
> 🛑 **Las dos filas de arriba bloqueaban trabajo YA AUTORIZADO, y se tachan con su motivo — 05/09.**
> `M3` se cerró el **03/09 con multímetro** (paso 20 del banco): pull-down real de **10 kΩ** en las
> **cuatro** posiciones de `J16` (`R65`–`R68`), `p10` **9,93 kΩ / 0 V** y `p12` **9,94 kΩ / 0 V** en
> reposo, y el **paso 21** cableó `p10` contra `p11` **sin demandas fantasma**. La medida ya estaba
> publicada en `17_Arquitectura...md` y en `6_Preguntas_Diseno_Funcional.md` §1.1 **desde el 04/09**:
> lo que faltaba era que alguien cruzara los ficheros. **Este README es el que dice «EMPIECE POR
> AQUÍ», así que su copia caducada pesa más que las otras.**
>
> 🔴 **Y lo que esa medida NO levanta: `J16` p1 reparte 12 V CRUDOS** —sin opto, sin serie, sin
> clamp—. **Taparlo es obligatorio en cada equipo que se monte** (N-120), no una cautela de banco:
> un contacto de `p1` a `p10` o `p12` mete 12 V en una pata de 3,3 V.
>
> **Las tres son de DEMANDA y activas en ALTO**: el contacto cierra contra **3,3 V**, no contra masa.
>
> | | |
> |---|---|
> | **NO hay** visión artificial embarcada en el STM32 | 64 KB de flash; el Maestro ya va al ~86 % |
> | **NO hay** YOLO, Raspberry Pi, Jetson ni PC en obra | Descartado **por decisión escrita** en `6_Preguntas_Diseno_Funcional.md` §2: *«Cero Computadores Edge Externos (CERRADO)»* |
> | **NO existe** el puerto serie de cámara IA (`AI_CARS`) | **`AiBus` y sus tres funciones están RETIRADOS**, no huérfanos: `grep AiBus` sobre las dos puntas sólo devuelve comentarios de historia. Colgaba del mismo USART1 que el Bluetooth, así que *«el puerto IA a 115200» nunca existió*; el enlazador ya descartaba las funciones, pero el objeto costaba **280 B de RAM por punta** en cada arranque |
> | **NO está construida** la cámara de umbral (despeje de tramo) | *Especificado, sin construir.* Falta una entrada física y un comando de radio. El despeje se hace **por tiempo** (`cfgDespejeSeg`), que es el criterio conservador. Ver `9_Manual_Parametrizacion_Camara_IA.md` |
>
> **Y el manual `04_Manuales/MANUAL_CONFIGURACION_CAMARAS_IA.md` salió el 26/08 con dos errores de
> pin**, corregidos el 28/08 y registrados en su §0: asignaba la cámara de demanda a **`PB9`**.
>
> 🔴 **`PB9` y `PB13` son hoy `MANDO_A` y `MANDO_B`, los canales del mando de relés** (`J16` p5 y
> p8), y `PB8` es el **LED testigo `D5`**, no una bornera. **Cablear una cámara a `PB9`/`PB13` no
> inyecta «pulsaciones de menú»: compone SECUENCIAS DE MANDO.** Tres pulsos de tráfico en 12 s hacen
> `A·A·A` o `B·B·B` y **el semáforo cambia de modo solo**; cuatro alternos en 18 s lo meten en
> Degradado.
>
> **Evidencia:** todo lo anterior está **MEDIDO** sobre el fuente y el esquemático (`grep` de las
> llamadas y `pines.h` de las dos puntas). **Ninguna línea está VERIFICADA EN LA PLACA todavía**:
> la sesión de banco es su primera comprobación física.
>
> Referencia de campo vigente: **[`9_Manual_Parametrizacion_Camara_IA.md`](9_Manual_Parametrizacion_Camara_IA.md)**
> y **[`15_Lista_de_Compras_Hardware.md`](15_Lista_de_Compras_Hardware.md)**. ~~**Son 2 cámaras, una
> por poste** (`DECISIONES.md` `D-2` / `D-13`), y **van a `J16`: `p10` en un poste y `p12` en el
> otro** (`D-3`).~~ → ✏️ **11/09, `D-25`: son 4 cámaras, DOS POR POSTE, y van a `J16` —`p10`
> (contra `p9`) y `p12` (contra `p11`) en CADA poste—.** Guía de campo:
> [`Camaras_Sisga_4x.html`](Camaras_Sisga_4x.html).
>
> ~~**2 cámaras es el montaje mínimo y el único cableable hoy** —una por poste, en `J14`—; las dos
> entradas de `J16` de cada punta están en el firmware y **esperan la medida `M3`**.~~
>
> 🔴 **TACHADO EL 07/09 — DOS ERRORES EN UNA FRASE, Y EN EL DOCUMENTO QUE DICE «EMPIECE POR AQUÍ».**
> Mandaba la cámara a `J14` y **resucitaba `M3`**, que este mismo README declara cerrada tres
> párrafos más arriba. Es exactamente lo que él mismo advierte: *«su copia caducada pesa más que las
> otras»*.
>
> **Y el motivo por el que `J16` no es una preferencia de conector:** el vigilante de
> `CAM_CIEGA`/`CAM_PEGADA` **mira `J16` y no mira `J14`**. Una cámara cableada a `J14` **funciona y
> no está vigilada** — nadie se enteraría de que se estropeó. `J14`/`PB0` queda **libre y vivo**,
> ~~reservado a un posible fin de carrera de barrera — 🔴 **con el conflicto abierto de la tabla de
> arriba (11/09): el firmware lo sigue leyendo como cámara.**~~ → **y sin cablear (`D-27`, 11/09):
> el fin de carrera no se instala; el firmware lo sigue leyendo como cámara, así que no se conecta
> nada.**
>
> ⚠️ **Y el vigilante tampoco ve a una cámara que NUNCA haya detectado** (11/09, medido):
> `vigilante_tick()` no acumula silencio en un pin sin flanco y `camara_estado()` lo salta al
> publicar `CAM:`. Esa exención se escribió cuando `p12` iba vacío a propósito; con `D-25` pierde
> su motivo y **queda pendiente en firmware**. Hasta entonces, **una segunda cámara muerta desde el
> día que se monta no la avisa nadie**, y por eso cada cámara se comprueba con el multímetro.

---

## 📄 Índice de Manuales y Documentos Core:

1. 📘 **[1_Manual_Usuario.md](1_Manual_Usuario.md)** / **`1_Manual_Usuario.docx`**  
   Manual de operación y secuencia de luces seguras bajo la **Resolución 2024 de MinTransporte Colombia**.
   Incluye el ~~**menú de dos niveles**, **AJUSTAR HORA**~~ *(⛔ `D-17.bis`: la pantalla y el menú se retiran del equipo)*, ~~el **mando de 4 relés**~~ *(⛔ `D-1`: **el mando no existe**; su código se conserva por SFTY-21)* y el **menú propio del Esclavo**. **Lo vigente de este manual es la operación por app** y el apartado de cámaras (§6, corregido el 07/09).
2. 📘 **[2_Manual_Hardware_y_Pruebas.md](2_Manual_Hardware_y_Pruebas.md)** / **`2_Manual_Hardware_y_Pruebas.docx`**  
   Guía de ensamblaje, cableado de borneras RS485 `485_A` / `485_B` (A a A, B a B) y flasheo en PlatformIO.
   Incluye la **pila `CR2032` del reloj** (§5) y el **mando de relés** (§6). 🔴 **07/09:** ~~con la advertencia de que **el Esclavo no tiene receptor**~~ → **`D-1`: NO HAY MANDO en ninguna punta.** Y **§5 está derogada en su parte de diagnóstico**: `D-15` — el STM32 ya no contesta a `SET_RTC`; se consulta con **`CMD:LEER_RTC`** (`D-17`).
3. 📘 **[3_Protocolo_Pruebas_Rigurosas.md](3_Protocolo_Pruebas_Rigurosas.md)** / **`3_Protocolo_Pruebas_Rigurosas.docx`**  
   Checklist obligatorio de pruebas de laboratorio y campo para certificar el equipo antes de puesta en marcha.
   ~~**68 pruebas**, con las Secciones **7 (reloj y sincronización)**, **8 (mando)**, **9 (Modo Degradado)** y **10 (interfaz del Esclavo)** nuevas.~~
   🔴 **AQUÍ NO VA UN TOTAL, y no es un descuido: es lo que el propio documento exige — 05/09.**
   ~~La cifra «68» **no aparece en el Protocolo**: `grep -c "68" 3_Protocolo_Pruebas_Rigurosas.md` da
   **0**, con y sin frontera de palabra.~~
   🔴 **ESA EVIDENCIA CADUCÓ EN 48 h — re-corrida el 07/09: `9` sin `-w` y `3` con `-w`.** Los
   nuevos aciertos son referencias de página del manual de la cámara, no un total de pruebas. **La
   conclusión sigue en pie —aquí no va un total—; lo que murió es el `grep` que la sostenía**, y por
   eso se tacha en vez de renumerarse (`CLAUDE.md` §4.sexies: *todo `grep` que se publique se corre
   antes*). Lo que ese documento publica es el reparto de los **82**
   identificadores de la revisión anterior (49 reescritas · 12 aplazadas · 21 retiradas · 4 nuevas),
   y **se niega expresamente a publicar un total nuevo** porque el denominador lo recorta quien
   ejecute la sesión. **Este README publicaba justo la cifra que el documento se niega a inventar.**
   ⚠️ **Y la Sección 10 (interfaz del Esclavo) está RETIRADA entera, no es nueva.** Las nuevas
   vigentes son la **7 (reloj)**, la **8 (mando)** y la **9 (Modo Degradado)**.
4. 📘 **[4_Manual_Configuracion_Radios.md](4_Manual_Configuracion_Radios.md)** / **`4_Manual_Configuracion_Radios.docx`**  
   Configuración de radios industriales **E90-DTU** con `RF_Setting4.6.exe` y DIP switches `M0`/`M1`.
5. 📘 **[5_Manual_Puente_ESP32.md](5_Manual_Puente_ESP32.md)** / **`5_Manual_Puente_ESP32.docx`**  
   Instrucciones para la instalación del puente repetidor con ESP32 (Modo 4 Radios para curvas ciegas).
6. 📘 **[6_Preguntas_Diseno_Funcional.md](6_Preguntas_Diseno_Funcional.md)** / **`6_Preguntas_Diseno_Funcional.docx`**  
   Cuestionario y parámetros de diseño de obra, y la **detección vehicular por contacto seco**.
   Contiene la decisión **CERRADA** de *«Cero Computadores Edge Externos»* (§2): la analítica corre
   **dentro de la cámara**, y al equipo llega **un pulso de hardware** — no vídeo ni datos.
7. 📡 **[7_Especificacion_Antenas.md](7_Especificacion_Antenas.md)** / **`7_Especificacion_Antenas.docx`**  
   **Especificación para fabricación de antenas bajo pedido.** Documento para entregar al proveedor:
   sintonía a **171 MHz**, ROE ≤ 1,5:1 en 168–174 MHz, sin plano de tierra y con **reporte de medición
   de ROE exigido como entregable**. Resuelve la causa del alcance de 3 cuadras medido el 31/07.
8. 🕹️ **[8_Procedimiento_Modo_Degradado.md](8_Procedimiento_Modo_Degradado.md)** / **`8_Procedimiento_Modo_Degradado.docx`**  
   **Procedimiento de campo del MODO DEGRADADO.** Requisitos previos, activación **en las dos puntas**
   con verificación visual de ambas, el **límite duro de 48 h**, la salida —también verificada en
   ambas puntas— y los **riesgos residuales aceptados por el cliente**. **Obligatorio leerlo antes de
   operar ese modo.**

> # 🔴 07/09 — ESTE ÍNDICE LLEGABA HASTA EL 8, Y EN LA CARPETA HAY **19** DOCUMENTOS NUMERADOS
>
> **Medido:** `ls -1 05_Funcional/[0-9]*.md | wc -l` → **19**. Los once que faltaban aquí incluyen
> **los tres que hoy más mandan**, y uno de ellos gana a este README en cobre:

9. 📷 **[9_Manual_Parametrizacion_Camara_IA.md](9_Manual_Parametrizacion_Camara_IA.md)** —
   **entregable principal desde `D-12`**: sin red y sin analítica en el controlador, **toda la
   inteligencia vive en la CONFIGURACIÓN de la cámara.** Es lo que se lleva delante de la cámara.
   🎯 **11/09, `D-27`:** los **valores** de esa configuración son los del manual del modelo
   comprado, [`../04_Manuales/MANUAL_CONFIGURACION_CAMARAS_IA.md`](../04_Manuales/MANUAL_CONFIGURACION_CAMARAS_IA.md)
   §4 —la tabla valor a valor está en el §4 Paso 3 del Manual 9—, y **las cuatro cámaras están
   compradas**. ⚠️ **La guía [`Camaras_Sisga_4x.html`](Camaras_Sisga_4x.html), paso 07, todavía dice
   «objetivo: no filtrar»; con `D-27` es ☑ Vehículo · ☐ Humano si la casilla existe — manda `D-27`
   (y el Manual 9, ya alineado); la guía está caducada en ese punto hasta que se corrija.**
10. 📱 **[10_Manual_Modulo_Bluetooth_Telemetria.md](10_Manual_Modulo_Bluetooth_Telemetria.md)** —
    el transporte SPP y la alimentación del módulo. **§1 congela el transporte: SPP, no BLE.**
11. ⏱️ **[11_Manual_Instalacion_RTC_DS3231_Bateria.md](11_Manual_Instalacion_RTC_DS3231_Bateria.md)** —
    pila del RTC y el `DS3231`. ⚠️ **§4.1 derogada por `D-15`: no se diagnostica el reloj mandando
    `SET_RTC` al STM32.** La orden vigente es **`CMD:LEER_RTC`** (`D-17`).
12. 🧪 **[12_Cobertura_de_Pruebas_y_Huecos.md](12_Cobertura_de_Pruebas_y_Huecos.md)** — qué está
    cubierto y qué no. **No publica cifras: remite al acta de `evidencia/`.**
13. 🔌 **[13_Manual_Modulo_Expansion_I2C_y_Compras.md](13_Manual_Modulo_Expansion_I2C_y_Compras.md)** —
    🛑 **PRE-IMPLEMENTACIÓN. El bus I²C sobre el STM32 NO SE MONTA.** Lo vigente de él es **§3 y la
    fe de erratas `J14`/`J15`** —riesgo eléctrico real— y **§4.1**, el censo de pines.
14. 📱 **[14_Manual_App_Movil_IOT_VIAL.md](14_Manual_App_Movil_IOT_VIAL.md)** — la app. **Y desde
    `D-16`, la única superficie de mando del equipo.**
15. 🛒 **[15_Lista_de_Compras_Hardware.md](15_Lista_de_Compras_Hardware.md)** — 💰 **el que se
    ejecuta con DINERO: cada fila es una compra.**
16. 📋 **[16_Documento_Auditoria_Arquitectura_y_Usabilidad_App_IOT_VIAL.md](16_Documento_Auditoria_Arquitectura_y_Usabilidad_App_IOT_VIAL.md)**
17. 🏗️ **[17_Arquitectura_28-08_y_Decisiones_Abiertas.md](17_Arquitectura_28-08_y_Decisiones_Abiertas.md)** —
    🔴 **GANA A TODO LO DEMÁS DE ESTA CARPETA EN HARDWARE MEDIDO.** Es donde se anotan las medidas de
    cobre, con su fecha, su instrumento y el firmware que había dentro.
18. 🧩 **[18_Especificacion_Firmware_ESP32.md](18_Especificacion_Firmware_ESP32.md)** — el ESP32 de
    **EXPANSIÓN** *(no el del repetidor del doc 5)*. 🟢 **Su firmware existe:**
    `01_Firmware/ESP32_Expansion/`, con rótulo Bluetooth y watchdog.
19. 🔧 **[19_Especificacion_Placa_Portadora_ESP32.md](19_Especificacion_Placa_Portadora_ESP32.md)** —
    la portadora de la línea `A8`.

---

## 🔄 Regeneración de los entregables Word

Los `.docx` **se generan desde los `.md`**, que son la única fuente de verdad:

```bash
python 05_Funcional/convertir_a_word.py        # regenera TODOS los .md numerados de la carpeta
python 05_Funcional/convertir_a_word.py 3 8    # solo los numerados 3 y 8
```

> ~~regenera los 8~~ 🔧 **CORREGIDO EL 07/09: son 19.** El script **descubre solos los `.md`
> numerados** (`sorted(os.listdir(DIR))`), así que **no hay que tocarlo al añadir uno** — lo que
> caducaba era el número escrito aquí a mano. **Y por eso este README no vuelve a publicar la
> cuenta**: se mide con `ls -1 05_Funcional/[0-9]*.md | wc -l`.

> **No edite los `.docx` a mano:** cualquier cambio se pierde al regenerarlos. Edite el `.md`
> correspondiente y vuelva a ejecutar el script.
>
> *Archivo obsoleto:* `2_Manual_Hardware.docx` es un remanente del nombre anterior del documento 2.
> El vigente es `2_Manual_Hardware_y_Pruebas.docx`.

---

## 🔗 Matriz de Referencia Cruzada:
- **`ARQUITECTURA.map` $\leftrightarrow$ `05_Funcional`:** Mapeo directo entre la topología de red RS485/LoRa y las instrucciones físicas de conexión.
- **`README.md` & `roadmap.md` $\leftrightarrow$ `05_Funcional`:** Garantiza el cumplimiento de las fases de desarrollo (Fase 1 Lógica Core, Fase 2 «AI Edge», Fase 3 Protocolo Binario V7 y Fase 4 Pruebas Físicas).
  > *Nota (28/08/2026):* el nombre **«Fase 2 AI Edge»** es histórico y **no describe lo construido**.
  > Lo que quedó de esa fase es la lectura de **contactos secos** —`PB0` en `J14`, y `PB14`/`PB15`
  > en `J16` desde el 31/08— ver el aviso de cámaras más arriba. No hay computador edge ni
  > inferencia en el microcontrolador.
