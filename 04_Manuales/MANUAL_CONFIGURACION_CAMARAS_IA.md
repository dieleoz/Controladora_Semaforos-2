# 📷 MANUAL DE PARAMETRIZACIÓN, CABLEADO Y TOPOLOGÍA — CÁMARAS IA (HIKVISION AcuSense)

> # 🛑 CABECERA DE ESTADO (07/09/2026) — LÉASE ANTES QUE EL CUERPO
>
> **Este manual se escribió antes de que se comprara la cámara y antes de que se cerrara `M3`.
> Tres cosas suyas están CADUCADAS, y una de ellas bloquea trabajo que ya está desbloqueado.**
>
> | | lo que este manual dice | lo que manda hoy |
> |---|---|---|
> | 🛑 **El modelo** | `DS-2CD3643G2-LIZSU` | **`DS-2CD2683G2-IZS`** — **`D-10`** de [`DECISIONES.md`](../DECISIONES.md) (05/09), *«tiene salida de alarma (`1 in, 1 out, 24 V/1 A`, ficha oficial)»*. La ficha está en esta misma carpeta |
> | 🛑 **«NO CABLEAR hasta `M3`»** *(§0.ter, §2, §3)* | *«`R67`/`R68` sólo están en el netlist y nadie las ha medido»* | ✅ **`M3` CERRADA EL 03/09** — **`D-3`**: medido en cobre con multímetro y conector vacío (paso 20 de la guía de banco), el pull-**down** de 10 kΩ **es real y está en las cuatro posiciones**; `p10` y `p12` dan **0 V en reposo**, y el paso 21 cableó `p10` **sin demandas fantasma**. Fuente que manda: `05_Funcional/17_…` sección **M3** |
> | 🛑 **El PROPÓSITO de las cámaras** | «demanda vehicular»: la cámara **pide verde** | **`D-13`** (05/09) decidió otra cosa, y **no es un matiz**: *«una sola regla, `Intrusion Detection` sobre el BARRIDO DE LA PLUMA —no la zona de espera—»*, y en su apartado *«lo que NO se hace»*: **«el ciclo del semáforo no se toca. Las cámaras no dan ni quitan verde.»** ⚠️ **Ver el aviso de conflicto abierto de abajo: esto NO lo cierra este manual** |
> | 🛑 **Cuántas cámaras y dónde** *(§0.ter, §1, §2, §3)* | *«una cámara por poste, en `J16` p10; `p12` se deja VACÍO»* | 🔴 **`D-25` (11/09, el responsable: *«mantener estas conexiones como definitivas»*): CUATRO CÁMARAS, DOS POR POSTE.** En cada poste, **cámara 1** entre `J16` **p9** (3,3 V) y **p10** (`PB14`, `CAM_C_PIN`); **cámara 2** entre `J16` **p11** (3,3 V) y **p12** (`PB15`, `CAM_D_PIN`); cada una por el contacto seco `1A`/`1B` de su salida de alarma. **Las cuatro IGUALES** —misma zona y misma configuración, `D-13`—: **no vuelve la «cámara de umbral»** del 26/08. **Talanquera en `J15`**: p1 = 12 V, p2 = drenador de `Q10` (**no es masa**), a la bobina de un relé cuyo contacto va a la entrada `OPEN` de la centralita. `D-25` deroga de `D-13` **sólo** *«una por poste / p12 vacío»*. Lo que el que cablea tiene que saber, medido en el firmware de `648b62f`: ver el recuadro **🔴 11/09** de §0.ter |
>
> ## 🔴 07/09 — LA PREGUNTA QUE DECIDE SI ESTE CABLEADO SIRVE, Y AHORA APUNTA A QUE **NO**
>
> **Todo este manual describe cómo llevar un contacto seco de la cámara a la tarjeta. Eso sólo vale
> si la ANALÍTICA de la cámara puede cerrar ese contacto** — y el papel del modelo comprado apunta a
> que no:
>
> | fuente oficial | qué dice |
> |---|---|
> | **Ficha `DS-2CD2683G2-IZS`, pág. 4, fila *Linkage Method*** | *«Upload to FTP/memory card/NAS, notify surveillance center, trigger recording, trigger capture, send email»* — **cinco, enumerados, y `trigger alarm output` NO está** |
> | **Manual `UD28967B-C`, PDF pág. 79 · impresa 67** | *«Trigger Alarm Output … **This function is only supported by certain models**»* |
>
> ⛔ **Y no vale *«pero la ficha dice `1 output`»***: `Manual Alarm` (un botón del navegador) y
> `Automatic Alarm` (cierre **por horario**) accionan esa salida **sin analítica ninguna** —
> **PDF pág. 80 · impresa 68**—. El borne se explica entero sin el enlace.
>
> 🛑 **Sigue 🔴 `SIN VERIFICAR`, y lo cierra mirar la pantalla en diez minutos** (`Paso 0` de
> `05_Funcional/9_…`, paso 39 de la guía de banco). **Si la casilla no está, este cableado no sirve y
> hay que ir por otro diseño** — decisión del responsable, no de un manual.

> ## 🟢 ~~CONFLICTO ABIERTO~~ — RESUELTO, y la respuesta ya estaba escrita desde el 05/09
>
> ~~**`D-13` dice que las cámaras no dan ni quitan verde. El firmware las lleva a
> `demanda_solicitar()`, que es exactamente pedir paso.**~~ **No hay tal conflicto**, y lo decide
> quién LEE esa demanda, no quién la escribe. Medido el 07/09:
>
> ```
> $ grep -rn "demanda_hayLocal" Maestro/src Maestro/include   (lectores reales)
>     modo_inteligente.cpp:217        <-- el UNICO
>     modo_automatico.cpp   -> cero
>     coordinador.cpp       -> cero
> ```
>
> **En Automático y en Manual las cámaras NO tocan el ciclo** —que es literalmente lo que dice
> `D-13`—. La demanda vive **sólo dentro del Modo Inteligente**, donde `D-19` la acota: **suelo = el
> tiempo que configura el operario, techo = el doble**. Ahí la cámara **sólo SOSTIENE un verde:
> nunca lo adelanta ni lo acorta**, y con la cámara muerta el ciclo vuelve a los tiempos
> configurados — **la ausencia no autoriza nada**, que es la primera de las tres cosas que `D-13`
> declara no negociables.
>
> ⚠️ **11/09 — precisión que falta arriba:** *«sólo SOSTIENE un verde»* es exactamente lo que hace
> `modo_inteligente.cpp` —por debajo del suelo nada; cumplido el suelo, mantiene la fase si en ese
> lado hay tráfico y enfrente nadie pide, hasta `suelo × TECHO_POR_SUELO`—. **Pero eso ES tocar la
> duración del ciclo**, y `D-25` (11/09) repite *«las cámaras no tocan el ciclo»*. Las dos filas y el
> firmware no dicen lo mismo en Inteligente: **conflicto para el responsable, no lo cierra este manual.**
>
> 🔴 **La lección no es del firmware, es de quien escribió esto:** se publicó como conflicto y se le
> llevó al responsable como decisión abierta **sin buscar antes si ya estaba contestada**. Lo
> estaba, en `roadmap_hist`: *«las cámaras no hacen nada en Auto ni en Manual»*.
>
> Lo que sigue debajo se conserva porque **la medida del fuente es correcta** y explica por dónde
> entra el pin:
>
> ```
> $ grep -n "CAM_J16" 01_Firmware/Maestro/src/botones.cpp
> 125:static const uint8_t CAM_J16[2] = {CAM_C_PIN, CAM_D_PIN};
> ```
>
> *(La línea decía `118` y la salida real de hoy es `125`: mismo símbolo, mismo contenido, número
> caducado por el commit `4b2841b`. Ver el bloque 🔴 del §0.ter.)*
>
> Un manual **no decide** cuál gana. Lo que este documento hace es **dejarlo escrito y no fingir que
> ya está resuelto**: mientras siga abierto, **el capítulo de parametrización de abajo describe la
> configuración de DEMANDA, que es la que el firmware ejerce, y NO la de `D-13`.** Quien vaya a
> parametrizar una cámara en campo tiene que preguntar cuál de las dos se monta.
>
> 🟢 **Y una cosa que este manual todavía no decía y ya está construida (07/09): el equipo PUBLICA
> el estado de la cámara.** `camara_estado()` tiene llamador en las dos puntas, dentro del
> `snprintf` del `$STATUS` de `bluetooth.cpp`, y la trama sale con un campo **`CAM:`** con cuatro
> valores: **`OK` · `CIEGA` · `PEGADA` · `?`**. Publica **la peor de las dos cámaras**, no una por
> cada una.
>
> 🛑 **`CAM: ?` (SIN COMPROBAR) es lo NORMAL hasta la primera detección, y es a propósito:** un pin
> que nunca ha dado señal no se vigila —si no, un borne vacío alarmaría—. **Por eso el paso en que
> el instalador provoca una detección de verdad NO es opcional: es lo que ARMA el vigilante.** Una
> instalación que se firma sin ese gesto deja el equipo sin vigilancia de cámara y con aspecto de
> estar bien.
>
> 🔴 **11/09 — CON DOS CÁMARAS POR POSTE (`D-25`) ESE GESTO SE HACE EN CADA UNA, Y LA APP NO SIRVE
> PARA COMPROBARLO.** `camara_estado()` publica la peor **sólo entre las cámaras que ya dieron un
> flanco** (salta la que no, con `camHuboFlanco`), y `vigilante_tick()` tampoco acumula silencio en
> ella. O sea: **con la primera detección de CUALQUIERA de las dos, la app pinta `OK` —*«las dos ven y
> ninguna está pegada»*, APK del 10/09 `b354fe9` y la de hoy— aunque la otra no haya visto nada
> nunca**, y **una segunda cámara muerta desde el día de la instalación no se detecta sola**. Tras
> cada reinicio vuelve a `?` y la vigilancia de silencio queda desarmada hasta la primera detección.
> **Cada cámara se comprueba en SU borne con el multímetro** (punta negra a `J16` p2, roja a p10 o
> p12): **0 V en reposo, 3,3 V con algo en la zona**. Esa exención se escribió porque `p12` iba
> vacío a propósito; con `D-25` pierde su motivo y **queda pendiente de rehacer en el firmware**
> (no aquí). Lo contrario sí se detecta: un contacto que se queda cerrado sale `PEGADA` aunque esa
> cámara no haya dado nunca un flanco (el nivel se siembra al arrancar).
>
> ## ⚠️ Y una fuente que gana a ésta
>
> **`05_Funcional/9_Manual_Parametrizacion_Camara_IA.md` es el manual de campo vigente de la
> cámara** — `D-12` lo asciende a *«entregable principal»*. Este documento cubre el **cableado a la
> tarjeta**; en todo lo que sea **la pantalla de configuración de la cámara**, manda el otro.

**Modelo de Cámara:** ~~Hikvision `DS-2CD3643G2-LIZSU`~~ 🛑 **CADUCADO** → **Hikvision
`DS-2CD2683G2-IZS`** (`D-10`, 05/09; ficha `DS-2CD2683G2-IZS_Datasheet_V5.5.113_20230303.pdf` en
esta carpeta). Lente varifocal motorizado; **`1 alarm in, 1 alarm out`**
**Sistema:** Controladora de Semáforos Móviles de 3 Estados (Control por Demanda Vehicular / Paso Alternado)
**Topología VIGENTE:** 2 Nodos Semafóricos (Maestro y Esclavo) + **cámaras IA de demanda** + Enlace de Control `RS485_OUT`
**Verificación Hardware:** Esquemáticos KiCad `Controladora_Semaforos.kicad_sch`, `pines.h` y `MAPEO_TARJETA_KICAD.md`
**Normativa Aplicable:** Manual de Señalización Vial de Colombia (Resolución 2024 - MinTransporte)
**Fecha de Emisión:** 26 de Agosto de 2026
**Fecha de Corrección:** **11 de septiembre de 2026** *(`D-25`: cuatro cámaras, dos por poste — cabecera de estado, §0.ter, §1, §2, §3, §5, §6)* · anterior: 7 de septiembre de 2026 · 2 de septiembre de 2026

---

## 0.ter 🟢 QUÉ CAMBIÓ EL 31/08 — ESTE MANUAL SE QUEDÓ CORTO DE ENTRADAS

> **Este manual decía que el firmware lee UNA cámara por poste. Hoy lee TRES.**

Medido el 02/09 sobre el fuente, idéntico en las dos puntas:

| entrada | pin | bornera | antirrebote de placa | estado |
|---|---|---|---|---|
| `CAM_DEMANDA_PIN` | `PB0` | `J14` | ✅ `R64` 10 kΩ + `C25` 100 nF (~1 ms) | ~~✅ **cableable hoy**~~ 🔴 **11/09: NO lleva cámara.** `A-2` (05/09) la reserva al **fin de carrera** de la pluma, **y el firmware la sigue leyendo como cámara** (`CAM_DEMANDA_PIN`: el Maestro por nivel en Inteligente, el Esclavo por flanco → `CMD_DEMANDA`). **CONFLICTO ABIERTO** — un fin de carrera ahí daría demandas falsas; no lo resuelve este manual |
| `CAM_C_PIN` | `PB14` | `J16` **p10** | ~~❌ ninguno en la placa~~ ⚠️ **11/09: el netlist SÍ trae `C28` 100 nF en paralelo** (`03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md`, fila p10: `R67`.2 · `C28`.1) — sin medir en cobre —; el reposo lo fija `R67` 10 kΩ a masa, ✅ **medida en cobre el 03/09** | ✅ **CABLEABLE** *(~~NO CABLEAR hasta `M3`~~ — `M3` cerrada, `D-3`)*. 👉 **Es la posición de la cámara 1 de cada poste** (`D-25`) |
| `CAM_D_PIN` | `PB15` | `J16` **p12** | ~~❌ ninguno en la placa~~ ⚠️ **11/09: netlist `C29` 100 nF** (misma fuente, fila p12) — `R68` 10 kΩ a masa, ✅ **medida el 03/09** | ✅ cableable, ~~pero **queda VACÍO**: hay **una cámara por poste** (`D-2`, `D-13`)~~ 🔴 **11/09, `D-25`: lleva la CÁMARA 2 de cada poste**, contra los 3,3 V de p11. Y su separación al riel de 12 V es **la peor de las cuatro** (1,36 mm): **`J16` p1 tapado antes de meter un hilo** (`D-4`) |

> ⚠️ **Los números de línea de este bloque estaban CADUCADOS los cinco** (decían `pines.h:124-125`,
> `botones.cpp:156-157`, `:126-133`, `:280-281`). **Se cita el símbolo y se publica el `grep` que lo
> encuentra, corrido el 07/09** — un número de línea caduca solo, en silencio y con la autoridad de
> un dato:

```
$ grep -n "define CAM_._PIN" 01_Firmware/Maestro/include/pines.h
165:#define CAM_C_PIN   PB14  // J16 p10 - camara de contacto seco (era BOTON3, "Aceptar")
166:#define CAM_D_PIN   PB15  // J16 p12 - camara de contacto seco (era BOTON4, "Cancelar")

$ grep -n "pinMode(CAM_._PIN" 01_Firmware/Maestro/src/botones.cpp
538:  pinMode(CAM_C_PIN, INPUT);
539:  pinMode(CAM_D_PIN, INPUT);

$ grep -n "^bool botonAceptar\|^bool botonCancelar" \
      01_Firmware/Maestro/src/botones.cpp 01_Firmware/Esclavo/src/botones.cpp
Maestro/src/botones.cpp:672:bool botonAceptar() { return false; }
Maestro/src/botones.cpp:673:bool botonCancelar(){ return false; }
Esclavo/src/botones.cpp:668:bool botonAceptar() { return false; }
Esclavo/src/botones.cpp:669:bool botonCancelar(){ return false; }
```

*(En el Esclavo el `pinMode` de las dos cámaras está en `botones.cpp:523-524` — esa sí seguía
buena.)*

> ## 🔴 07/09, MÁS TARDE — **ESTE MISMO BLOQUE `grep` YA ESTABA CADUCADO, Y SE PUBLICÓ HOY**
>
> **Es la demostración de §4.sexies de `CLAUDE.md` ocurriendo dentro del propio párrafo que la
> cita.** Lo que había escrito aquí arriba, presentado como *«corrido el 07/09»*, decía
> `pines.h:148-149`, `botones.cpp:531-532` y `:659-660`. **Al volver a correrlo con el árbol de
> firmware LIMPIO** *(`git status` sin una sola modificación en `01_Firmware/`)* **salen `165-166`,
> `538-539` y `672-673`.**
>
> **La causa está en el `git log` y no es un misterio:** el commit `4b2841b` *«anclar las decisiones
> vigentes en el codigo — de 5 marcas `D-x` a 16»* insertó comentarios **por encima** de esos
> símbolos, en el mismo día. **Las citas se verificaron por la mañana y caducaron por la tarde, sin
> que nadie tocara la línea citada.**
>
> 🛑 **Por eso la regla no es «renumerar bien»: es CITAR EL SÍMBOLO.** El número de línea sólo vale
> **fechado a un commit** —`git show <hash>:fichero`— o **pegado como salida literal de un `grep`
> que alguien vuelva a correr**. Los símbolos de este bloque —`CAM_C_PIN`, `CAM_D_PIN`,
> `botonAceptar()`, `botonCancelar()`— **no han cambiado y son lo que hay que buscar.**

**Las tres son cámaras de DEMANDA:** las tres acaban en `demanda_solicitar()`. Ninguna mide el
despeje del tramo, que sigue siendo por tiempo (`cfgDespejeSeg`).

> ⛔ **AQUÍ DECÍA: *«~~`J16` p10 y p12 NO SE CABLEAN TODAVÍA… `R67`/`R68` sólo están en el netlist y
> nadie las ha medido en la placa. La medida es `M3`, con óhmetro, y no se salta~~»*. CADUCADO — se
> tacha con su motivo y no se borra (07/09).**
>
> **`M3` se cerró el 03/09** (**`D-3`**), y se cerró midiendo justo eso: multímetro, conector vacío,
> paso 20 de la guía de banco. El pull-**down** de **10 kΩ es real y está en las cuatro posiciones**
> (`R65`–`R68`, cada una con su 100 nF); `p10` y `p12` dan **0 V en reposo**; y el **paso 21** cableó
> `p10` contra `p11` en normalmente abierto **sin una sola demanda fantasma**. **El pin no flota.**
>
> La fuente que manda en esto es `05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md`
> sección **M3**, no este manual. **Dejar el bloqueo escrito después de cerrarlo es lo que ya pasó
> con `CLAUDE.md` §9.bis el 05/09**: alguien recita el párrafo caducado con autoridad y nadie va a la
> fuente.

🔴 **Lo que SÍ sigue bloqueando, y no es opcional:**

1. **`J16` p1 lleva 12 V crudos** —sin opto, sin limitadora y sin clamp— a un conector de señal
   directa al micro. **Taparlo es obligatorio en cada equipo que se monte** (`D-4`, N-120), no una
   cautela de banco.
2. ~~**Si sólo se cablea una cámara, va en `p10`.**~~ 🔴 **11/09, `D-25`: se cablean las DOS —cámara 1
   en `p10`, cámara 2 en `p12`—, así que `p12` deja de poder evitarse.** La separación real sobre
   cobre contra la red de 12 V es **4,269 mm en `p10`** y **1,359 mm en `p12`** —el peor de los
   cuatro—. Un error de una posición al enchufar `J16` mete 12 V en un pin de 3,3 V: **por eso el
   punto 1 no se salta, y el trabajo en `p12` se hace limpio** (sin hilos sueltos ni cobre al aire).
3. **No se cablea nada a `p5` ni a `p8`.** Ver el aviso de §2: son los canales del mando, y su
   **código sigue leyéndolos**.

🪜 **Y el orden es asimétrico: el firmware nuevo tiene que estar CARGADO EN LA TARJETA antes de que
nadie enchufe un hilo en `J16`.** Con el firmware viejo dentro, `PB14` todavía es *Aceptar* leído
**activo en BAJO**, y cualquier cosa enchufada en p10 lo pulsa en un equipo que está en la calle. Un
commit no protege de un destornillador: se exige la carga verificada, no el merge.
🔴 **11/09, `D-25`: y ahora también `p12`** — en cualquier binario anterior a `deeeab4` (el V8.4
`e303485` que corre en campo incluido) **`PB15` es *Cancelar***:
`git show e303485:01_Firmware/Maestro/src/botones.cpp` → `botonAceptar()`/`botonCancelar()` leen
`BOTON3`/`BOTON4` = `PB14`/`PB15`. La cámara 2 cableada con el programa viejo dentro **pulsa
*Cancelar*** igual que la 1 pulsa *Aceptar*.

---

## 0. 🛑 CORRECCIÓN DEL 28/08/2026 — QUÉ DECÍA ESTE MANUAL Y POR QUÉ ERA FALSO

> **Esta sección no se borra.** Lo que se corrige en silencio se vuelve a escribir, y la segunda
> vez ya nadie recuerda que se comprobó.

La versión emitida el **26/08/2026** (commit `3d24da6`, único commit de este fichero hasta hoy)
describía **4 cámaras** y asignaba la cámara de demanda al pin **`PB9`**. Ambas cosas son falsas
contra el firmware que corre. Se corrigen aquí, y queda el registro de qué se dijo:

| Lo que decía el manual del 26/08 | Lo MEDIDO sobre el fuente (28/08) |
|---|---|
| **4 cámaras** (2 por poste): demanda + umbral | El firmware leía **1 cámara por poste**. La de umbral no tiene dónde entrar. *(Al 02/09 son **3 entradas de demanda** por punta — ver §0.ter. La de umbral **sigue sin existir**)* |
| Cámara 1 y 3 (demanda) → pin **`PB9`** | **`PB9` es `BOTON1`** (símbolo `BOTON1` en `pines.h`, las dos puntas). La demanda entra por **`PB0`** = `CAM_DEMANDA_PIN` (`pines.h:46`) |
| Cámara 2 y 4 (umbral) → pin **`PB13`** | **`PB13` es `BOTON2`** (símbolo `BOTON2` en `pines.h`). No existe `CAM_UMBRAL_PIN` en el firmware Maestro/Esclavo |
| «Optoacoplador `TLP127` con pull-up en `PB9`/`PB13`» | La línea de cámara real (`PB0`) lleva **`R64` 10 kΩ (pull-DOWN) + `C25` 100 nF**, antirrebote ~1 ms, bornera **`J14`**, y es **activa en ALTO** |

**De dónde salió el error:** el manual del 26/08 se escribió contra el firmware de nodo único
`01_Firmware/Semaforos/`, que sí definía dos entradas de cámara (`CAM_DEMANDA_PIN` = `PB0` y
`CAM_UMBRAL_PIN` = `PB8`). Al partir el firmware en Maestro/Esclavo, `CAM_UMBRAL_PIN` **no se
portó**. Y el pin `PB8` que se le atribuía tampoco es una entrada: medido el 27/08 sobre el
esquemático bueno, **`PB8` va por `R16` 1 kΩ a un LED testigo (`D5`)** — es una salida de aviso,
no una bornera (`roadmap.md` N-64, y `pines.h:63` lo deja escrito como `LED_TESTIGO`).

**Distinción de evidencia, que importa:**

| | |
|---|---|
| **MEDIDO** (sobre fuente y esquemático, 27–28/08) | `PB0` = `CAM_DEMANDA_PIN`; `PB9`/`PB13` = botones; `PB8` = LED testigo; polaridad activa en alto; `R64`/`C25`/`J14`; cero llamadores del puerto serie IA |
| **VERIFICADO EN LA PLACA** | **NADA de este manual lo está todavía.** La sesión de banco es su primera comprobación física. Un `.md` correcto contra el fuente sigue siendo papel |

---

## 0.bis 🛑 EL «PUERTO SERIE DE CÁMARA IA» (`AI_CARS` / YOLO / Raspberry Pi) **NO ESTÁ IMPLEMENTADO**

> **Estado: código huérfano. Especificado, escrito, y sin un solo llamador.** No lo describa a
> ningún cliente como una función del equipo.

**Actualización del 02/09: ya no es código huérfano — es código RETIRADO.**

Medido el 02/09 con `grep AiBus` sobre `01_Firmware/Maestro` y `01_Firmware/Esclavo`: **no queda ni
una declaración, ni una definición, ni una llamada.** Lo único que aparece son dos comentarios que
cuentan la historia (el bloque `AiBus` de `protocolo.cpp`, las dos puntas).

Lo que hubo, y por qué se fue:

- `AiBus` era un `HardwareSerial` sobre `(PA10, PA9)` — el **mismo USART1** que usaba `SerialBT`.
  Dos objetos peleándose un periférico a dos velocidades distintas (115200 aquí, 9600 allí).
  Ganaba Bluetooth por orden de arranque, **así que el «puerto IA a 115200» nunca existió**.
- Sus tres funciones —`protocolo_actualizarAI()` y sus dos consultas— **no tenían un solo
  llamador**. El enlazador ya las descartaba: retirarlas ahorró 16 B de flash.
- **El objeto sí costaba, y ése era el motivo real de retirarlo:** su constructor cuelga del
  arranque, así que corría en cada encendido y su memoria era permanente. Eran **280 B de RAM por
  punta** —el 5,2 % de la RAM viva del equipo— por un puerto que no se abre.

**Y un dato de cableado que sale de aquí:** `SerialBT` **ya no vive en `PA9`/`PA10`**. Vive en
**`PB6`/`PB7`, USART1 remapeado, conector `J17`** (símbolo `SerialBT` en `Maestro/src/bluetooth.cpp`, idéntico en el
Esclavo — ⛔ la cita `:28` estaba caducada). Ver la tabla de §2.

**Nació vivo y murió en la partición del firmware.** En `01_Firmware/Semaforos/` (nodo único)
`modo_inteligente.cpp:65,81-82` sí llamaba a las tres. Al separar en Maestro/Esclavo la llamada
no se portó, y el **Esclavo ni siquiera tiene `modo_inteligente.cpp`**.

**Y el PC que hablaría ese puerto está descartado por decisión, no por olvido.**
`05_Funcional/6_Preguntas_Diseno_Funcional.md` lo tiene **CERRADO** por escrito:

> *«Cero Computadores Edge Externos (CERRADO): Se descarta el uso de Raspberry Pi, Jetson Nano,
> conversores USB y switches Ethernet. La detección de vehículos corre directamente dentro de la
> cámara (DSP AcuSense) y entra por pulsos limpios de hardware a la placa.»*

**Conclusión operativa:** la analítica de vídeo la hace **la cámara**, y llega al equipo por **un
contacto seco**. No hay ni habrá un enlace serie de conteo de vehículos mientras esa decisión siga
cerrada. **El código de `AiBus` ya se retiró** (02/09) y **no es una función del producto**.

---

## 1. Arquitectura Vial y Distribución de las Cámaras (VIGENTE)

Tramo de obra de un solo carril con paso alternado, **dos postes** y ~~**una cámara de demanda por
poste** — que es el montaje mínimo y el único cableable hoy. Cada punta admite además **dos
entradas más** en `J16`, pendientes de `M3` (§0.ter):~~ 🔴 **11/09, `D-25`: DOS CÁMARAS POR POSTE,
cuatro en el cruce** —cámara 1 en `J16` p9/p10, cámara 2 en `J16` p11/p12—, **las dos iguales y
mirando la misma zona** (`D-13`: el barrido de la pluma). `M3` está cerrada desde el 03/09. **El
diagrama de abajo es el del 07/09 y dibuja UNA por poste mirando la aproximación: las dos cosas
caducaron** (una por `D-25`, la otra por `D-13`); se conserva porque su cableado de `p10` sigue
siendo el de la cámara 1.

```text
  ══════════════════════════════════════════════════════════════════════════════════════════════
                                  TRAMO DE OBRA (UN SOLO CARRIL)
  ══════════════════════════════════════════════════════════════════════════════════════════════

  [EXTREMO 1: POSTE MAESTRO]                                        [EXTREMO 2: POSTE ESCLAVO]

       ┌────────────────────────┐                                    ┌────────────────────────┐
       │   SEMAFORO MAESTRO     │                                    │    SEMAFORO ESCLAVO    │
       │   🔴 🟡 🟢             │                                    │    🔴 🟡 🟢            │
       └────────────────────────┘                                    └────────────────────────┘
                  │                                                             │
            (CAMARA 1)                                                    (CAMARA 2)
            [ 👁️ ◄── ]                                                    [ ──► 👁️ ]
        Mira la via de aproximacion                            Mira la via de aproximacion
        Contacto seco -> PB14 (J16 p10)                        Contacto seco -> PB14 (J16 p10)
        (D-2/D-3, corregido 07/09.  J14/PB0 queda vivo y SIN camara)
        11/09 D-25: en CADA poste va ADEMAS otra camara igual -> PB15 (J16 p12,
        contra p11). Las dos miran el BARRIDO DE LA PLUMA (D-13), no la aproximacion.
        OJO al nombre: aqui "CAMARA 1/2" es Maestro/Esclavo; en D-25 y en la guia
        del Sisga "camara 1/2" son p10/p12 DE CADA POSTE.
                  │                                                             │
                  ▼                                                             ▼
        SENTIDO 1 (Llegada)                                          SENTIDO 2 (Llegada)
        Carros entrando ────►                                        ◄──── Carros entrando
        hacia la obra                                                hacia la obra

        El DESPEJE del tramo NO lo mide ninguna camara: es POR TIEMPO (cfgDespejeSeg).
```

---

## 2. Asignación y Función por Cámara (VIGENTE)

| Cámara | Ubicación Física | Sentido de Visión | Función en el Sistema Vial | Pin en Tarjeta STM32 |
|---|---|---|---|---|
| **CÁMARA 1** | **Poste Maestro (Extremo 1)** | ~~**SENTIDO 1 (Aproximación):** vehículos que llegan por la vía hacia el Maestro~~ **el barrido de la pluma** (`D-13`) | **Demanda Vehicular Sentido 1:** ~~al detectar vehículo, solicita apertura de **🟢 Verde en Semáforo Maestro**~~ **11/09: pide paso, y esa petición SÓLO la lee el Modo Inteligente** (`demanda_hayLocal()`/`camara_presenciaJ16()` no tienen otro lector); **en Automático y Manual no abre ningún verde** | 🔴 **`PB14` — `J16` p10** *(corregido 07/09)*. ~~Pin `PB0` — bornera `J14`~~ · **11/09, `D-25`: + una segunda cámara igual en `PB15` — `J16` p12** |
| **CÁMARA 2** | **Poste Esclavo (Extremo 2)** | ~~**SENTIDO 2 (Aproximación):** vehículos que llegan por la vía hacia el Esclavo~~ **el barrido de la pluma** (`D-13`) | **Demanda Vehicular Sentido 2:** el Esclavo transmite la demanda al Maestro por `RS485_OUT`/radio — **11/09: y el Maestro sólo la atiende en `MODO_INTELIGENTE`** (rama `CMD_DEMANDA` de `coordinador.cpp`: fuera de ese modo contesta `DEMANDA_RECHAZADA`) | 🔴 **`PB14` — `J16` p10**, idéntico al Maestro *(corregido 07/09)*. ~~Pin `PB0` — bornera `J14`~~ · **11/09, `D-25`: + una segunda cámara igual en `PB15` — `J16` p12** |

> ## 🔴 CORREGIDO EL 07/09 — TODO EL CUERPO DE ESTE MANUAL MANDABA LA CÁMARA A `J14`, Y VA A `J16`
>
> **La cabecera y el §0.ter ya lo decían bien; los apartados 2, 3, 5 y 6 seguían con el destino
> viejo.** Un instalador que abriera este documento por el §3 —el diagrama de cableado— llevaría el
> hilo a `J14`, y **el equipo funcionaría**, que es lo que lo hace peligroso.
>
> | | |
> |---|---|
> | **manda** | **`J16` p10 (`PB14`)** — `DECISIONES.md` **`D-2`** y **`D-3`**, `M3` cerrada en cobre el 03/09 · 🔴 **11/09, `D-25`: y `J16` p12 (`PB15`) para la segunda cámara de cada poste** |
> | **`J14` (`PB0`)** | **sigue vivo en el firmware y NO lleva cámara.** Se conserva libre como candidato a fin de carrera de barrera · 🔴 **11/09: `A-2` ya lo reservó al fin de carrera, pero el firmware lo lee como `CAM_DEMANDA_PIN`** (Maestro por nivel en Inteligente; Esclavo por flanco → `CMD_DEMANDA`): un fin de carrera ahí daría demandas falsas. **CONFLICTO ABIERTO, no lo resuelve este manual** |
>
> 🔴 **Y el motivo que decide no es el antirrebote —que `J14` sí tiene y `J16` no—: es la
> VIGILANCIA.** Las alarmas `CAM_CIEGA` y `CAM_PEGADA` del firmware miran **`J16`** y **no miran
> `J14`**. Una cámara cableada a `J14` funciona… **y el día que se estropee nadie se entera.** Ésa
> es exactamente la avería silenciosa que el vigilante existe para impedir.
>
> **Todo lo que este manual dice de la polaridad sigue valiendo igual** —contacto seco entre el pin
> de señal y el borne de **3,3 V contiguo** (`p9` para `p10`; *11/09, `D-25`:* `p11` para `p12`), **dos hilos, nada a masa**, activo en
> ALTO contra el pull-down de 10 kΩ—. **Lo único que cambia es la bornera.** El paso a paso con el
> destornillador delante está en **`05_Funcional/9_…` §4.bis**, que es el manual de campo vigente.
| **CÁMARA `C`** | ~~cualquiera de los dos postes~~ **11/09, `D-25`: la CÁMARA 1 de CADA poste** | 🔴 **ES LA POSICIÓN DE LA CÁMARA REAL** (`D-2`, `D-3`) | **Demanda Vehicular**, igual que la de `J14`: pide paso | Pin **`PB14`** — **`J16` p10**, contra p9. ✅ **CABLEABLE** *(~~NO CABLEAR hasta `M3`~~: `M3` cerrada el 03/09)*. Separación al riel de 12 V: **4,27 mm** |
| **CÁMARA `D`** ~~*(opcional)*~~ | ~~cualquiera de los dos postes~~ **11/09, `D-25`: la CÁMARA 2 de CADA poste** | ~~libre — sería una demanda más de esa punta~~ **misma zona y misma configuración que la `C`** (`D-13`) | **Demanda Vehicular**, igual — **hace exactamente lo mismo que la `C`** | Pin **`PB15`** — **`J16` p12**, contra p11. ✅ cableable, ~~pero **hoy VACÍO**~~ **y desde `D-25` CABLEADA**. ⚠️ Separación al riel de 12 V: **1,36 mm — la peor de las cuatro** |

> ⚠️ **`C` y `D` no son «cámaras de umbral» ni miden nada distinto.** Son entradas de demanda más,
> ~~por si un poste necesita vigilar dos accesos~~ → **11/09, `D-25`: son las dos cámaras de cada
> poste, mirando la MISMA franja (el barrido de la pluma), con la MISMA configuración.** Lo medido en
> el firmware de `648b62f`, que es lo que el instalador tiene que llevarse:
>
> - **Las dos hacen LO MISMO.** `botones.cpp` recorre `CAM_J16[2] = {CAM_C_PIN, CAM_D_PIN}` en el
>   mismo bucle de `camaras_actualizar()`: flanco → `demanda_solicitar()` + vigilante. Idéntico en
>   las dos puntas. No hay una «de demanda» y otra «de pluma».
> - **NINGUNA protege la pluma.** `escribirPines()` de `semaforo.cpp` sube la pluma con
>   `(verde && !testLedsActivo) || estado == S_FALLO` y no lee ninguna cámara: **la pluma baja con un
>   coche debajo.** El veto es `A-1.bis`, **sin construir**; lo que existe es un contador
>   (`camVetos`, `$EVENT CAMARA_PLUMA`) que observa la bajada ya hecha.
> - **La pluma sube también con el ámbar intermitente (`S_FALLO`)**: Modo Ámbar, radio perdida, y
>   **un poste recién encendido que aún no enlaza con el otro**. No sólo con verde.
> - **En Automático y en Manual no cambian ninguna luz**; en Inteligente sólo alargan una fase,
>   hasta `suelo × TECHO_POR_SUELO`, y nunca la acortan.
> - **La app no distingue una de otra** (ver el recuadro 🔴 11/09 de la cabecera): cada una se
>   comprueba con el multímetro en su borne.
>
> Antirrebote de placa: ver §0.ter (el netlist trae `C28`/`C29`).

> ### 🔴 CUÁNTAS CÁMARAS HAY — decidido, y esta tabla se leía como si fueran cuatro
>
> ~~**Son DOS en total: una por poste** (`D-2` de [`DECISIONES.md`](../DECISIONES.md), 28/08;
> ratificado por `D-13`, 05/09). No son «dos fijas más dos opcionales».~~
>
> 🔴 **11/09 — DEROGADO POR `D-25`: SON CUATRO, DOS POR POSTE, las dos de cada poste en `J16`
> (p10 contra p9, p12 contra p11) y las cuatro IGUALES** —misma zona, misma configuración, `D-13`—.
> Lo que `D-25` deroga de `D-13` es **sólo** *«una por poste / p12 vacío»*. ⚠️ **Y NO es la vuelta
> de las «4 cámaras (2 por poste): demanda + umbral» del 26/08 (§0)**: aquellas eran dos funciones
> distintas; éstas son cuatro veces la misma. La de umbral **sigue sin existir** (§2.bis).
>
> **Que el firmware LEA tres entradas por punta no significa que haya tres cámaras.** Las dos cosas
> son ciertas a la vez y confundirlas es lo que llenó este manual de cámaras que nadie compró:
>
> | | |
> |---|---|
> | **el firmware lee** | 3 entradas por punta — `PB0` (`J14`), `PB14` y `PB15` (`J16`) |
> | **se monta** | ~~**1 cámara por poste**, en `J16` **p10**~~ **11/09, `D-25`: 2 por poste, en `J16` p10 y p12.** `J14` no lleva cámara (`A-2` lo reserva al fin de carrera — y el firmware lo sigue leyendo como cámara: **conflicto abierto**, §0.ter) |
> | **la lista de compras dice** | ~~**2 cámaras**, *«son las dos que el firmware lee hoy»*~~ **decía 2** (`05_Funcional/15_Lista_de_Compras_Hardware.md`, línea `A2`); **con `D-25` hacen falta 4** — la cantidad la lleva ese documento, no éste |
>
> ⛔ **Y aquí había una pregunta abierta —*«cuántas cámaras van por poste, lo decide el
> responsable»*— que YA ESTÁ DECIDIDA** ~~desde `D-2`~~ → **desde el 11/09 por `D-25`: dos.** Se
> tacha para que nadie la vuelva a plantear.

> ⚠️ **Numeración:** el manual del 26/08 llamaba «Cámara 3» a la del Esclavo, porque contaba
> cuatro. Con dos cámaras, la del Esclavo es la **Cámara 2**. Si encuentra rotulado *«CAM 3»* en
> una caja o en un plano viejo, es esta misma.
>
> 🔴 **11/09 — Y OJO, QUE CON `D-25` «CÁMARA 2» QUIERE DECIR OTRA COSA.** En este manual *«Cámara
> 1/2»* es **la del Maestro / la del Esclavo**. En `D-25` y en `05_Funcional/Camaras_Sisga_4x.html`
> *«cámara 1/2»* es **`p10` / `p12` de CADA poste**. Lo que no se confunde es la constante:
> **`CAM_C` = `J16` p10, `CAM_D` = `J16` p12**, en cualquiera de los dos postes. Ante la duda,
> se nombra el borne.

### Pines que NO son entradas de cámara — no los cablee

| Pin | Lo que realmente es | Referencia |
|---|---|---|
| **`PB9`** (`J16` p5) | **`BOTON1` = `MANDO_A`**, canal `A` del mando de relés | símbolos `BOTON1` en `pines.h`, `MANDO_A` en `mando.cpp` |
| **`PB13`** (`J16` p8) | **`BOTON2` = `MANDO_B`**, canal `B` del mando de relés | símbolos `BOTON2` en `pines.h`, `MANDO_B` en `mando.cpp` |
| **`PB8`** | **`LED_TESTIGO`** — `R16` 1 kΩ → LED `D5`. **Salida, no bornera** | símbolo `LED_TESTIGO` (`pines.h:63`, cita verificada el 07/09) |
| `PA9` / `PA10` | `RS485_IN` — el MAX3485 `U2` y la bornera `J10`. **NO es el Bluetooth** | símbolos `RS485_IN_RX`/`RS485_IN_TX` en `pines.h` |
| `PB6` / `PB7` (`J17`) | **USART1 remapeado — aquí sí está el Bluetooth / ESP32** | símbolo `SerialBT` en `bluetooth.cpp` (⛔ la cita `:28` estaba caducada: esa línea es hoy un comentario del `lcd.cpp`) |

> ### ⚠️ 05/09: EL MANDO SE RETIRÓ COMO HARDWARE, Y ESO **NO** HACE SEGUROS ESTOS DOS PINES
>
> **`D-1` retiró el equipo, no el código** —y lo dice por escrito: *«el CÓDIGO no se toca»*—.
> `botones_actualizar()` sigue leyendo `BOTON1`/`BOTON2` en cada vuelta, con `pinMode(…, INPUT)`
> pelado y **activo en ALTO**, y sigue llamando a `mando_registrarPulso(MANDO_A/B)` en cada
> flanco. `mando.cpp` sigue reconociendo `A·A·A`, `B·B·B` y `A·B·A·B`.
>
> **O sea que el aviso de abajo NO caduca: vale hoy exactamente igual, y ahora sin ningún
> pulsador legítimo compitiendo por el pin.** La spec da `J16` p5 y p8 como **libres y sin
> cablear** (`05_Funcional/17_…`) — *libres de cobre*, **no** *libres de firmware*.

🔴 **Conectar un relé de cámara a `PB9` o `PB13` no inyecta «pulsaciones de menú»: compone
SECUENCIAS DE MANDO.** Tres pulsos de tráfico dentro de la ventana de 12 s hacen `A·A·A` o `B·B·B`,
y eso **cambia el modo del semáforo solo**: a Automático o a ámbar. Cuatro alternos en 18 s
(`A·B·A·B`) lo meten en **Modo Degradado**. No es una conexión inerte: es un semáforo que se
reconfigura con el tráfico.

⚠️ **`PA9`/`PA10` cambió de significado el 28/08 y este manual llevaba el dato viejo.** Decía que
eran *«hoy el bus de Bluetooth»*. Son `RS485_IN`; el Bluetooth se mudó a `PB6`/`PB7` (`J17`).
Tampoco son entrada de cámara.

---

## 2.bis Cámara de UMBRAL (tramo de obra) — **ESPECIFICADO, SIN CONSTRUIR**

> **No se borra del papel: se marca honestamente.** Si mañana se quiere, esto es lo que costaría.

La cámara de umbral —la que confirmaría el **despeje efectivo** del tramo mirando los vehículos
que ya cruzaron— está **diseñada y documentada, y no existe en el equipo**. Faltan **dos** cosas,
y ninguna es un `pinMode()`:

1. **Una entrada física.** `PB8` no sirve: es el LED testigo. Habría que sacar un hilo del pad de
   `PB8` retirando `R16`/`D5`, o cablear uno de los cuatro pines libres (`PA11`, `PA12`, `PA15`,
   `PC13`) a una bornera con su red de antirrebote.
2. **Un comando de radio** que lleve la cuenta del tramo desde el Esclavo hasta el Maestro, que
   es quien decide (`roadmap.md` N-59). Sin ese comando, leer el pin es medio camino — y un pin
   leído que no llega a la lógica es exactamente la clase de función huérfana que este manual
   acaba de corregir en §0.bis.

**Mientras tanto el despeje se hace POR TIEMPO** (`cfgDespejeSeg`), que es el criterio
conservador: la cámara de umbral daría **eficiencia**, no seguridad. El equipo es seguro sin ella.

---

## 3. Diagrama Eléctrico de Cableado (VIGENTE)

Cada cámara conecta su salida de contacto seco de relé (**Bornera `ALARM`: pines `1A` y `1B`**)
a la bornera ~~**`J14`**~~ 🔴 **`J16` p10** *(corregido 07/09 — `D-2`/`D-3`; ver el bloque 🔴 del §2)* de la tarjeta STM32 de su propio poste. 🔴 **11/09, `D-25`: la cámara 1 de cada poste a `J16` p10 (contra p9) y la cámara 2 a `J16` p12 (contra p11).** El diagrama de abajo es del 07/09: sus líneas *«UNA por poste»* y *«p12 → VACIO»* **caducaron** (se anotan dentro). La comunicación entre postes viaja
por **`RS485_OUT`**:

```text
 ┌──────────────────────────────────────────────┐      ┌──────────────────────────────────────────────┐
 │            NODO 1: SEMAFORO MAESTRO          │      │            NODO 2: SEMAFORO ESCLAVO          │
 ├──────────────────────────────────────────────┤      ├──────────────────────────────────────────────┤
 │                                              │      │                                              │
 │  CAMARA 1 (Demanda Sentido 1)                │      │  CAMARA 2 (Demanda Sentido 2)                │
 │    Rele [ 1A ] ───► J16 p10 / PB14 (act.ALTO)│      │    Rele [ 1A ] ───► J16 p10 / PB14 (act.ALTO)│
 │    Rele [ 1B ] ───► J16 p9  (3,3 V contiguo)  │      │    Rele [ 1B ] ───► J16 p9  (3,3 V contiguo)  │
 │      (R67 10K a GND = pull-DOWN, medido 9,93K)│      │      (R67 10K a GND = pull-DOWN, medido 9,94K)│
 │      CORREGIDO 07/09: NO va a J14. Ver §2.    │      │      CORREGIDO 07/09: NO va a J14. Ver §2.    │
 │      (R64 10K a GND = pull-DOWN + C25 100nF) │      │      (R64 10K a GND = pull-DOWN + C25 100nF) │
 │                                              │      │                                              │
 │  NO CABLEAR: PB9 = MANDO A · PB13 = MANDO B  │      │  NO CABLEAR: PB9 = MANDO A · PB13 = MANDO B  │
 │              (J16 p5 y p8 - secuencias)      │      │              (J16 p5 y p8 - secuencias)      │
 │  NO CABLEAR: PB8 = LED testigo D5 (salida)   │      │  NO CABLEAR: PB8 = LED testigo D5 (salida)   │
 │                                              │      │                                              │
 │  CAMARA 1 -> J16 p10 = PB14 (11/09, D-25:    │      │  CAMARA 1 -> J16 p10 = PB14 (11/09, D-25:    │
 │    YA NO es "UNA por poste": son DOS)        │      │    YA NO es "UNA por poste": son DOS)        │
 │    M3 CERRADA 03/09: YA SE CABLEA.           │      │    M3 CERRADA 03/09: YA SE CABLEA.           │
 │    Contacto contra los 3,3 V de p9.          │      │    Contacto contra los 3,3 V de p9.          │
 │  CAMARA 2 -> J16 p12 = PB15, contra p11.     │      │  CAMARA 2 -> J16 p12 = PB15, contra p11.     │
 │    ANTES DECIA: "p12 VACIO (D-2: una         │      │    ANTES DECIA: "p12 VACIO (D-2: una         │
 │    camara)" -- DEROGADO 11/09 por D-25.      │      │    camara)" -- DEROGADO 11/09 por D-25.      │
 │    Hace LO MISMO que la 1. NINGUNA de las    │      │    Hace LO MISMO que la 1. NINGUNA de las    │
 │    dos frena la pluma (A-1.bis sin hacer).   │      │    dos frena la pluma (A-1.bis sin hacer).   │
 │  J16 p1 lleva 12 V CRUDOS: TAPARLO SIEMPRE.  │      │  J16 p1 lleva 12 V CRUDOS: TAPARLO SIEMPRE.  │
 │    p10 esta a 4,27 mm de los 12 V; p12 a     │      │    p10 esta a 4,27 mm de los 12 V; p12 a     │
 │    1,36 mm, que es el peor de los cuatro.    │      │    1,36 mm, que es el peor de los cuatro.    │
 │                                              │      │                                              │
 │  BORNERA RS485_OUT (Control Inter-Poste)     │      │  BORNERA RS485_OUT (Control Inter-Poste)     │
 │    [ A   ] ══════════════════════════════════╪══════╪═══════════════════════════════► [ A   ]      │
 │    [ B   ] ═══════ (Cable Trenzado o Radio) ═╪══════╪═══════════════════════════════► [ B   ]      │
 │    [ GND ] ══════════════════════════════════╪══════╪═══════════════════════════════► [ GND ]      │
 └──────────────────────────────────────────────┘      └──────────────────────────────────────────────┘
```

### Reglas Eléctricas:

1. **Contacto seco, pero la POLARIDAD DE LA SEÑAL SÍ IMPORTA.** Los bornes `1A`/`1B` del relé son
   libres de tensión y entre ellos no hay polaridad; **pero la entrada `PB14` es ACTIVA EN ALTO**
   contra el pull-down de 10 kΩ de la placa (`R67`, medido **9,93 kΩ** en banco). El contacto debe
   **cerrar `PB14` (`J16` p10) contra los 3,3 V de `J16` p9**. Cablearlo
   a GND deja la entrada leyendo demanda continua sin que pase ningún vehículo (`N-67`).
   **11/09, `D-25`: y la cámara 2 cierra `PB15` (`J16` p12) contra los 3,3 V de `J16` p11**, con el
   mismo pull-down (`R68`, medido **9,94 kΩ**). Dos hilos por cámara; nada a masa.
2. **Antirrebote:** ~~la placa filtra ~1 ms con `R64`/`C25`~~ ⚠️ **11/09: `R64`/`C25` son de `J14`
   (`PB0`), no de `J16`.** En `J16` el netlist pone `R67`+`C28` (p10) y `R68`+`C29` (p12), 10 kΩ y
   100 nF (`03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md`; el condensador no se ha medido en cobre).
   El firmware añade **5 ms** por software (`camara_leerPin()`). No hace falta condensador externo.
3. **Alimentación 12 V DC:** ~~la cámara~~ **las dos cámaras** *(11/09, `D-25`)* de cada poste se
   alimentan de la batería de 12 V del propio semáforo móvil — **nunca del regulador de la
   tarjeta**. El consumo por poste es **el doble** del de una cámara: la cuenta, con la cifra de
   ficha, está en `05_Funcional/9_…` §1.1.2.
4. **Independencia de buses:** el enlace entre postes (`RS485_OUT`) no comparte nada con la señal
   de cámara — son un par diferencial y un contacto seco, sin puntos en común.
5. **Ventana de silencio de 3 s:** el firmware ignora demandas repetidas dentro de los 3 s
   siguientes a una aceptada (`demanda.cpp`). Una cola de coches no satura el canal de 2,4 kbps.
   **No es un fallo de la cámara** si el segundo coche no dispara una trama.

---

## 4. Parametrización de la Cámara en 3 Pasos

Sin Internet, sin routers y sin software de monitoreo. Se hace **una vez en taller** y queda:

```
┌────────────────────────────────────────────────────────────────────────┐
│   CONFIGURACION DE LA CAMARA DE DEMANDA (~~una por poste~~ 11/09,     │
│   D-25: las CUATRO, dos por poste, TODAS con esta misma configuracion) │
├────────────────────────────────────────────────────────────────────────┤
│ 1. Zoom y Foco Motorizado: encuadrar el carril de parada               │
│ 2. Dibujar la zona / linea de deteccion (Filtro: solo Vehiculo)        │
│ 3. Salida de Alarma: modo N/O a 1 s (se configura una vez y queda)     │
└────────────────────────────────────────────────────────────────────────┘
```

### Paso 1: Encuadre Óptico Motorizado (Lente ~~2.7–13.5 mm~~ **2,8 – 12 mm**)

> ⛔ **CORREGIDO EL 07/09: `2.7–13.5 mm` NO ES ESTA CÁMARA.** Es la óptica del modelo de referencia
> que este manual traía antes de la compra. La `DS-2CD2683G2-IZS` lleva **`2,8` a `12 mm`**, `F1.6`,
> iris fijo, con **FOV horizontal de `108°` a `30°`** — ficha `DS-2CD2683G2-IZS_Datasheet_V5.5.113`,
> **pág. 2**, filas *Lens Type* y *Focal Length & FOV*. No cambia el gesto; cambia el número que
> alguien podría usar para calcular un encuadre.

1. Conectar la laptop al puerto Ethernet de la cámara e ingresar a `http://192.168.1.64`
   (Usuario: `admin`).
2. En la pestaña **Live View**:
   * Usar **Zoom `+` / Zoom `-`** hasta encuadrar la **zona de parada del carril**.
   * Presionar **One-Touch Focus**. La cámara ajusta la nitidez con su motor interno.
   * **Precintar:** no volver a mover el zoom.

### Paso 2: Dibujar la Detección (Máscara de Demanda)

> ## 🛑 CORREGIDO EL 07/09 — ESTE PASO MANDABA LA ANALÍTICA EQUIVOCADA
>
> **Decía *Detección de Cruce de Línea* «o Intrusión, si prefiere». No es una preferencia: es la
> diferencia entre detectar al que ESPERA y al que YA SE FUE.** La elección está razonada sobre las
> definiciones literales del fabricante en
> **`05_Funcional/9_Manual_Parametrizacion_Camara_IA.md` §1.1.4**, que es el manual de campo
> vigente (`D-12`) y **gana a éste en todo lo que sea la pantalla de la cámara**:
>
> | | qué dice el manual oficial que detecta | ¿sirve? |
> |---|---|---|
> | **Detección de Intrusión** | *«objects entering and **loitering** in a predefined virtual region»*, y su `Threshold` es *«the time of the object **loitering** in the region»* | ✅ **SÍ. Es la única que mide PERMANENCIA** |
> | *Cruce de Línea* | *«objects **crossing** a predefined virtual line»* | 🛑 **NO.** Dispara con el vehículo **que ya pasó** |
>
> **PDF `UD28967B-C` págs. 60-62 · impresas 48-50.**
>
> 🔴 **Y el clasificador NO está donde este paso lo ponía.** `Detection Target` (☑ Vehículo /
> ☐ Humano) **está documentado en *Line Crossing* y NO en `Set Intrusion Detection`**, cuyas reglas
> publicadas son sólo `Sensitivity`, `Threshold` y el filtro de tamaño (PDF 60-61 · impresas 48-49).
> La ficha del modelo **sí** dice que la Perimeter Protection *«supports human and vehicle targets
> classification»* (pág. 4). **Las dos fuentes oficiales no dicen lo mismo: se mira en pantalla.**

1. Ir a **`Configuration → Event → Smart Event → Intrusion Detection`**
   *(Configuración → Evento → Evento Inteligente → **Detección de Intrusión**)* — **PDF pág. 60 ·
   impresa 48**. En algunos modelos la ruta es **`VCA → Smart Event → Intrusion Detection`**: **el
   manual oficial da las dos**, así que si una no existe se prueba la otra.
   > ⚠️ **Y una condición previa fácil de pasar por alto:** *«For certain device models, you need to
   > enable the smart event function on **VCA Resource** page first»* (misma página). Si la opción
   > no aparece o sale en gris, habilitarla ahí primero.
2. Marcar ☑ **Habilitar** (*Enable*).
3. **Dibujar la región** sobre la zona donde el vehículo **se detiene a esperar** *(`Draw Area`:
   clic en la vista en vivo para los vértices, clic derecho para cerrar — PDF pág. 67 · impresa 55)*.
   🔴 **Antes de dibujar, pregunte qué tiene que mirar esta cámara**: `D-13` decide *barrido de la
   pluma* y el firmware ejerce *demanda de aproximación*. **Las dos están vivas y una cámara sólo
   lleva una regla.** Ver §4 Paso 3 del manual de campo.
4. **`Size Filter`:** se dibujan con el ratón un tamaño **mínimo** y uno **máximo** de objetivo
   *(PDF pág. 68 · impresa 56)* — no son campos numéricos.
5. En **Clasificación de Objetivo** (*Detection Target*), **si la casilla existe**:
   * ☑ **Vehículo** (*Vehicle*)
   * ☐ **Humano** (*Human*) — *desmarcado: inmunidad a peatones, ramas y sombras.*
   * 🔴 **Si NO existe** —que es lo que el papel deja esperar en Intrusión— **se anota y se avisa**:
     sin filtro, una persona parada levanta el bit. **Se compensa subiendo el mínimo del
     `Size Filter`, y la decisión de fondo NO la toma el técnico.**
6. **`Arming Schedule`: 24 × 7.** Fuera de la programación horaria **la cámara no dispara** — un
   cruce que deja de aceptar demanda de madrugada es un fallo que ningún ensayo de taller de día
   encuentra.

### Paso 3: Configurar la Salida de Relé (N/O ~~a 1 Segundo~~ **al mínimo que admita**)

> ## 🔴 DOS AVISOS QUE ESTE PASO NO LLEVABA, y los dos son sobre cifras que nos inventamos
>
> **1. El «`1 s`» ES CIRCULAR: sale de un manual NUESTRO, no de Hikvision.** Es **`A-7`** de
> [`DECISIONES.md`](../DECISIONES.md), y está medido: *«el manual oficial no publica ni un valor de
> `Delay` en 110 páginas»* — sólo la definición del campo. Peor: el `~1 s` vive hoy en **nueve
> sitios** —cinco comentarios de firmware y **dos packs con `PULSO_RELE_MS = 1000`** que salen
> **verdes contra un número que nadie ha medido**. *El instrumento certifica la invención.*
>
> 👉 **Lo que se hace en su lugar:** poner el `Delay` **al mínimo que la cámara admita**, **anotar
> el valor real que ofrezca** y medir el tiempo de cierre con un cronómetro. De ahí se **deriva**
> `SILENCIO_MS > Delay + rearme`, en vez de citarnos. Hay hueco para esa medida en la guía de banco
> (pasos 39–40).
>
> **2. El desplegable `Alarm Type` está documentado para la ENTRADA de alarma, NO para la salida.**
> Verificado sobre el manual de usuario `UD28967B-C` v5.7.20: la **salida** sólo expone
> `No.`, `Name` y `Delay` (impresa 68 · PDF 80); *Normally Open / Normally Closed* **no aparece ni
> una vez en las 110 páginas**. Así que *«se configura como `NO`»* queda **`SIN VERIFICAR`**: puede
> que no haya nada que elegir.
>
> > ### 🔴 CORREGIDO EL 07/09 — AQUÍ HABÍA UNA LECTURA MAL HECHA DE LA FUENTE
> >
> > Decía: ~~*«de la entrada el manual documenta el valor `NO`»*~~, citando la pág. impresa 44
> > (PDF 57): *«Select **Alarm Input NO.** and Alarm Type from the dropdown list»*.
> >
> > **`NO.` con punto es *Number*, no *Normally Open*.** Lo demuestra el mismo documento: la
> > pantalla de la **salida** usa el mismo campo escrito `Alarm Output **No.**` — *«Select the alarm
> > output No. according to the alarm interface connected»* (**PDF 80 · impresa 68**), donde nadie
> > leería «salida normalmente abierta». **Esa página documenta el NOMBRE del desplegable y ningún
> > valor suyo.**
> >
> > 🟢 **Y hay UNA aparición real de `NO` como valor en las 110 páginas, en un sitio donde nadie
> > había mirado** — sale al buscar `alarm type` **insensible a mayúsculas**, no sólo el rótulo:
> > *«the alarm input `A<-1` … its **alarm type is always NO**»*, **PDF pág. 99 · impresa 87**, en
> > *Road Traffic → Set Vehicle Detection*. Es de la **ENTRADA**, en un modo que este proyecto **no
> > usa**, y dice *always* — o sea que ahí **no se elige**. **`NC` sigue en CERO apariciones.**
> >
> > **La conclusión no cambia y por eso se corrige igual:** la polaridad de la **salida** sigue
> > `SIN VERIFICAR` y la cierra el óhmetro. Lo que se arregla es la **prueba**, que era falsa —
> > una excepción sostenida por una frase mal leída es un defecto con permiso (`CLAUDE.md` §2.ter).
>
> 👉 **Consecuencia práctica, y es buena:** si la salida es `NO` de fábrica, es la que el firmware
> necesita —entrada activa en ALTO contra el pull-down de 10 kΩ— y no hay que tocar nada. **Si
> resultara ser `NC` y no se pudiera cambiar, NO se cablea**: el firmware vería demanda permanente
> en reposo y ausencia justo al pasar el vehículo. Es la inversión que ya costó `N-67`. **Se anota
> el hallazgo y se para.**
>
> 🔴 **Y la casilla que decide si este paso sirve para algo sigue abierta:** que la **analítica**
> pueda accionar el relé. El manual dice de `Trigger Alarm Output` que *«only supported by certain
> models»*. **Se mira con la cámara delante antes que nada** (paso 39 de la guía de banco).
>
> ### 🔴 07/09 — Y AHORA HAY UN DATO DEL MODELO COMPRADO QUE APUNTA A QUE **NO**
>
> La nota *«only supported by certain models»* es una condición **sobre el modelo**, y **la ficha ES
> el documento del modelo**. Contesta enumerando:
>
> ```text
>   DS-2CD2683G2-IZS_Datasheet_V5.5.113_20230303.pdf, pag. 4, seccion "General":
>     Linkage Method
>       Upload to FTP/memory card/NAS, notify surveillance center,
>       trigger recording, trigger capture, send email
> ```
>
> **Cinco enlaces, lista cerrada, y `trigger alarm output` NO está** — mientras `trigger recording`
> y `trigger capture` sí. ⛔ **Y no vale el consuelo de *«pero tiene `1 output`»***: el propio manual
> documenta `Manual Alarm` (un botón del navegador) y `Automatic Alarm` (cierre **por horario**),
> **PDF pág. 80 · impresa 68**, que accionan esa salida **sin analítica ninguna**. El borne se
> explica entero sin el enlace.
>
> 🛑 **Sigue `SIN VERIFICAR` —el papel no lo cierra en positivo— pero lo esperable es que la casilla
> no esté.** Si no se puede enlazar, **el camino de `J16` no sirve y hay que ir por otro diseño**
> (relé de NVR, evento de red), que es una decisión del responsable.
>
> 🔻 **Lo que NO muere con ella:** la cámara **sigue grabando el evento en su microSD** —
> `Trigger Recording` **sí** está en la lista de la ficha, y el `Record Schedule` admite el tipo
> `Event` (PDF pág. 48 · impresa 36)—.
>
> ⛔ **Y lo que este manual decía como relevo y NO lo es:** ~~*«existe una vía que no depende de esa
> casilla: la entrada de alarma de la cámara (`D-14`)»*~~. `D-14` es una **decisión de diseño sin
> firmware**: medido el 07/09, **fuera de `semaforo.cpp` el controlador sólo escribe la dirección
> del RS485 y de la radio**, y los tres canales de potencia libres —`ROJO_PEATON`, `VERDE_PEATON`,
> `BUZZER`— están **declarados y muertos en las dos puntas**. **El controlador no tiene hoy con qué
> cerrar un contacto hacia la cámara.** Describirlo como disponible sería la «Caja Negra de
> Alarmas» otra vez: cuatro manuales y ni un llamador.

1. Ir a **Configuración > Eventos > Salida de Alarma** (*Alarm Output*):
   * **Estado por Defecto:** **`NO`** (*Normally Open*) — 🟡 **`SIN VERIFICAR` que sea elegible**, ver arriba.
   * **Duración del Pulso (`Delay`):** **el MÍNIMO que ofrezca el desplegable.** ⛔ ~~`1s`~~ — cifra
     inventada por nosotros (`A-7`). **Anote el valor real que aparezca**, que es el dato que falta.
2. En la pestaña del evento inteligente, **Método de Vinculación** (*Linkage Method*):
   * ☑ **Disparar Salida de Alarma 1**.
3. **Desarmar los eventos básicos:** en *Detección de Movimiento básica*, *Sabotaje de Vídeo* y
   *Excepción*, dejar **desmarcada** la casilla «Disparar Salida de Alarma».
   *(Una salida = un único significado: «hay vehículo».)*
4. **Guardar**.

> 💡 **Operación Autónoma:** se desconecta la laptop. La cámara opera de forma continua con la
> batería del semáforo. No queda ningún PC en obra.

---

## 5. Dinámica de Control y Seguridad Vial (Resolución 2024)

> 🔴 **11/09 — LOS PUNTOS 1 Y 2 DESCRIBEN UN EQUIPO QUE NO EXISTE, y se tachan con la medida al
> lado.** Dicen que la cámara **abre** el verde de su lado. **Medido en `648b62f`:** la demanda de
> cámara sólo la lee `modo_inteligente.cpp` (`demanda_hayLocal()`, `camara_presenciaJ16()`, y
> `CMD_DEMANDA` del Esclavo sólo se atiende en `MODO_INTELIGENTE`). **En Automático y en Manual la
> cámara no cambia ninguna luz.** En Inteligente, **por debajo del suelo (el tiempo configurado) no
> cambia nada**; cumplido el suelo la fase cambia igual que en Automático, salvo que en ese lado
> haya tráfico y enfrente nadie pida: entonces **se alarga, hasta `suelo × TECHO_POR_SUELO`**. **La
> cámara nunca adelanta ni acorta un verde.** Y con `D-25` (dos por poste, en `p10` y `p12`) las dos
> entran por la misma puerta y hacen lo mismo.

1. **Llegada de Vehículo al Sentido 1 (lado Maestro):**
   * **Cámara 1** detecta el vehículo ➔ cierra su relé ➔ **`PB14` (`J16` p10) del Maestro pasa a ALTO**
     *(11/09: o `PB15` (`J16` p12), la cámara 2 de ese poste — `D-25`)*.
   * ~~El Maestro cierra el Semáforo Esclavo a Rojo, ejecuta el **Todo-Rojo de despeje**
     (`cfgDespejeSeg`) para vaciar la vía, y abre **🟢 Verde en el Semáforo Maestro**.~~ → **11/09:
     ver el recuadro — sólo en Inteligente, y sólo alarga.**
2. **Llegada de Vehículo al Sentido 2 (lado Esclavo):**
   * **Cámara 2** detecta el vehículo ➔ **`PB14` (`J16` p10) del Esclavo pasa a ALTO**
     *(11/09: o `PB15` (`J16` p12) — `D-25`)*.
   * El Esclavo emite `CMD_DEMANDA` al Maestro por radio/`RS485_OUT` (con su ventana de 3 s).
   * ~~El Maestro cierra el Semáforo 1 a Rojo, aplica el Todo-Rojo, y otorga **🟢 Verde al
     Semáforo Esclavo**.~~ → **11/09: el Maestro sólo la atiende en Inteligente; fuera de él
     contesta `DEMANDA_RECHAZADA` y no mueve nada.**
3. **Despeje del tramo de obra:** **por tiempo** (`cfgDespejeSeg`). Ninguna cámara lo mide —ver
   §2.bis—. Es el criterio conservador y es el que corre hoy.
4. 🔴 **11/09 — La pluma (talanquera, `J15`) NO la frena ninguna cámara.** `escribirPines()` de
   `semaforo.cpp`, las dos puntas: `(verde && !testLedsActivo) || estado == S_FALLO`. **Baja al
   acabar el verde aunque la cámara vea un coche debajo** (el veto es `A-1.bis`, sin construir), y
   **sube también con el ámbar intermitente** (`S_FALLO`: Modo Ámbar, radio perdida, poste recién
   encendido sin enlace). `J15`: p1 = 12 V, p2 = drenador de `Q10` —**no es masa**—, a la bobina de
   un relé cuyo contacto va a la entrada `OPEN` de la centralita (`D-25`).

> ⚠️ **El Maestro es quien decide siempre.** La cámara del Esclavo **pide**; no abre nada por su
> cuenta.

---

## 6. Protocolo de Pruebas y Validación en Campo

Antes de abrir el paso vehicular en el tramo de obra:

```
[ ENSAYO 1: Continuidad con Multimetro ] ──► [ ENSAYO 2: Demanda Sentido 1 (Maestro) ] ──► [ ENSAYO 3: Demanda Sentido 2 (Esclavo) ]
```

### 🧪 Ensayo 1: Continuidad del Relé (~~las 2 cámaras~~ **las 4 cámaras** — 11/09, `D-25`)

* Medir con multímetro en modo continuidad entre `1A` y `1B` de cada cámara.
  > 🟡 **Que `1A`+`1B` sean los dos extremos de UN contacto es DEDUCCIÓN, no lectura de diagrama**
  > (`D-14`). La *Quick Start Guide* dice que *«`1A` and `1B` … are three pairs of alarm outputs»* y
  > la ficha dice `1 input, 1 output`, luego a la nuestra le tocan `1A`+`1B`. **Lo que no existe en
  > ninguna de las dos fuentes es diagrama de cable ni régimen eléctrico** — ni tensión, ni
  > corriente, ni las palabras *dry contact* o *relay* en 110 páginas. **Compruébelo con el óhmetro
  > antes de dar tensión, no después.**
* **Reposo:** circuito abierto (sin pito).
* **Al cruzar un vehículo:** pita y se abre solo. ⛔ **Aquí decía *«~~durante ~1 segundo~~»*: esa
  cifra es NUESTRA, no de Hikvision (`A-7`).** **Cronométrelo y anote el número** — es justo el dato
  que falta para derivar `SILENCIO_MS`. **Un valor distinto de 1 s no es un fallo de la cámara.**

### 🧪 Ensayo 2: Demanda y Conmutación Sentido 1 (Maestro)

> 🔴 **11/09 — LOS CRITERIOS DE ACEPTACIÓN DE LOS ENSAYOS 2 Y 3 NO SE PUEDEN CUMPLIR, y no por
> avería:** piden que la detección **abra el verde**, y eso no lo hace ninguna cámara (§5, recuadro
> del 11/09). **Lo que se comprueba en su lugar, cámara por cámara (las dos de cada poste,
> `D-25`):** multímetro en tensión continua, punta negra a `J16` p2, roja en el borne de esa cámara
> (`p10` la 1, `p12` la 2): **0 V en reposo y 3,3 V con alguien en la zona**. ⚠️ **La app no sirve
> para esto:** pone `CAM: OK` —*«las dos ven y ninguna está pegada»*— con la primera detección de
> **cualquiera** de las dos, y sigue en `OK` aunque la otra no haya detectado nunca.

* Acercar un vehículo frente a la **Cámara 1**.
* ~~**Criterio de Aceptación:** el Maestro recibe el pulso en **`PB14`** y ejecuta la secuencia de
  transición legal hasta **🟢 Verde Maestro**, manteniendo el Esclavo en Rojo.~~ → **11/09: el
  pulso llega a `PB14` (3,3 V en p10) — y a `PB15` (p12) con la cámara 2 —; la luz no se mueve por
  ello fuera del Modo Inteligente.**
* **Criterio negativo, obligatorio:** **con el contacto de la cámara ABIERTO y nadie delante del
  lente, el equipo NO debe registrar demanda.** Si la registra, el pin está flotando o la polaridad
  no es la que se cree: se para y se anota.
* **Segundo criterio negativo, y hoy es MÁS importante, no menos:** con `J16` p5 y p8 **vacíos**,
  el semáforo **no debe cambiar de modo solo** mientras la cámara dispara. Si cambia, **el relé está
  cableado a `PB9`/`PB13`** — el error de §0 — porque **ese código sigue vivo y sigue leyendo esos
  pines** aunque el mando ya no se monte (`D-1`).

> ⚠️ **La versión anterior de este ensayo decía *«pulsar los cuatro botones del menú»*. Ya no hay
> botones ni menú que pulsar.** `botonAceptar()` y `botonCancelar()` devuelven `false` siempre en las
> **dos** puntas, y desde el 05/09 **la pantalla tampoco se monta** (`D-17.bis`). Un criterio
> negativo que no se puede ejercer no prueba nada. ✅ **Se cita el símbolo, porque los números de
> línea que había aquí —`:280-281` y `:294-295`— ya habían caducado:**
>
> ```
> $ grep -n "^bool botonAceptar" 01_Firmware/Maestro/src/botones.cpp 01_Firmware/Esclavo/src/botones.cpp
> 01_Firmware/Maestro/src/botones.cpp:659:bool botonAceptar() { return false; }
> 01_Firmware/Esclavo/src/botones.cpp:645:bool botonAceptar() { return false; }
> ```
>
> ⛔ **Y aquí decía *«~~quedan dos: `A` en `J16` p5 y `B` en p8~~»*, que se leía como que hay dos
> pulsadores montados. NO LOS HAY** (`D-1`, 05/09): *«ya no tenemos mandos de A y B, sólo la app»*.
> Lo que queda en p5/p8 es **el código que los lee**, no un aparato.

### 🧪 Ensayo 3: Demanda y Conmutación Sentido 2 (Esclavo)

* Acercar un vehículo frente a la **Cámara 2** (la del Esclavo).
* ~~**Criterio de Aceptación:** el Esclavo recibe el pulso en **`PB14`**, lo transmite, el Maestro
  aplica el Todo-Rojo y otorga **🟢 Verde Esclavo**, pasando él a Rojo.~~ → **11/09: el pulso
  llega a `PB14` (p10) o `PB15` (p12) del Esclavo y sale `CMD_DEMANDA`; el Maestro sólo lo atiende
  en Inteligente. Se acepta con el multímetro en el borne de cada cámara, como en el Ensayo 2.**
* **Segundo vehículo dentro de 3 s:** puede no generar trama nueva. Es la ventana de silencio,
  **no un fallo**.

---

## 7. Referencias cruzadas

| Documento | Qué aporta |
|---|---|
| `05_Funcional/9_Manual_Parametrizacion_Camara_IA.md` | 🔴 **MANDA SOBRE ÉSTE en todo lo que sea la cámara** (`D-12`). Trae el destino correcto (`J16` p10), el paso a paso del cableado con el destornillador delante (§4.bis) y la tabla de qué está `MEDIDO` y qué `SIN VERIFICAR` (§7) |
| `DECISIONES.md` | La tabla de decisiones vigentes. **Si una fila de allí y un párrafo de aquí no coinciden, gana la fila** |
| `05_Funcional/15_Lista_de_Compras_Hardware.md` | Cantidades reales: ~~**2 cámaras**, «son las dos que el firmware lee hoy»~~ **11/09: con `D-25` son 4** (dos por poste); la cantidad la lleva ese documento |
| `05_Funcional/Camaras_Sisga_4x.html` | 🆕 **11/09:** la guía de campo de las 4 cámaras de `D-25`, corregida (qué hace cada cámara, la pluma, la comprobación con multímetro) |
| `05_Funcional/6_Preguntas_Diseno_Funcional.md` | Decisión **CERRADA** de «Cero Computadores Edge Externos» |
| `roadmap.md` N-59, N-64, N-67 | Origen de la cámara de umbral, del hallazgo de `PB8` y de la polaridad activa en alto |

---
*Manual técnico de instalación, topología y cableado para semáforos móviles.*
*Emitido 26/08/2026 · **Corregido 28/08/2026** (§0: pines `PB9`/`PB13` erróneos y 4 cámaras → 2) ·
**Corregido 02/09/2026** (§0.ter: `J16` p10/p12 ya son entradas de cámara; `PA9`/`PA10` no son el
Bluetooth; el criterio negativo del Ensayo 2 ya no se podía ejercer; `AiBus` está retirado, no
huérfano) · **Corregido 11/09/2026** (`D-25`: cuatro cámaras iguales, dos por poste en `J16`
p10/p12 —no la vuelta de «demanda + umbral»—; ninguna frena la pluma; la app no distingue una
cámara de otra; `J14` es conflicto abierto con `A-2`; §5 y los Ensayos 2-3 pedían un verde que la
cámara no abre).*
*Todo lo de este documento está **MEDIDO sobre el fuente y el esquemático**, y **ninguna línea
está VERIFICADA EN LA PLACA**: la sesión de banco es su primera comprobación física.*
