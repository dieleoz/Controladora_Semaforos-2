# 17 — Arquitectura decidida en obra el 28/08/2026, y lo que sigue abierto

**Para:** el funcional y el auditor.
**Fecha del documento:** 28 de agosto de 2026.
**Revisado:** 31 de agosto de 2026 (decision del responsable), **04 de septiembre de 2026 (banco
real: el equipo se midio con multimetro los dias 3 y 4)**, **04 de septiembre de 2026, mas tarde el
mismo dia (tres cambios de firmware y una decision del responsable, medidos SOBRE FICHEROS)** y
**04 de septiembre de 2026, de noche (cinco cambios mas, una decision y un hallazgo nuevo, tambien
SOBRE FICHEROS)**. Las cuatro revisiones estan al principio; **la del banco va la primera a
proposito** —es la unica medida en cobre—, y detras las dos del mismo dia en orden. **Nada de lo
superado se ha borrado.**

**Acta de compuerta de referencia:** `evidencia/2026-08-28_compuerta.txt` — `15 PASS | 0 FALLA | 0 ABORTADO`,
HEAD `3733544`, rama `main-nuevo`, **arbol LIMPIO** (lo dice la propia acta).

> 🛑 **Y LA MAS RECIENTE NO ESTA EN VERDE, asi que no se puede usar como referencia de nada.**
> `evidencia/2026-09-04_compuerta.txt`, tal como esta hoy: HEAD `6d075a5`, **arbol CON CAMBIOS SIN
> COMMITEAR**, **`18 PASS | 1 FALLA | 1 ABORTADO`** — banco por packs en `981/998` con **2 packs en
> FALLA**, y el simulador del puente ESP32 **`ABORTADO`**. **Un ABORTADO no dice nada del firmware y
> un `x/y` con `x != y` dice que hay comprobaciones que no cumplen.** El detalle, y el mecanismo por
> el que este documento llego a citar de ese mismo nombre unas cifras que ya no existen, en la
> **revision del 04/09 de noche, punto 5**.

Este documento esta escrito **en ASCII sin acentos**, como el resto de lo que se parsea o se lee en
consola de Windows en este repositorio.

> 🔵 **COMO SE CITA EL FUENTE EN ESTE DOCUMENTO — cambiado el 05/09, y el motivo es una medida, no
> una preferencia de estilo.**
>
> Un censo mecanico —con control negativo: una cita inventada a proposito la marca— conto en este
> documento **272 citas `fichero:linea`** vivas —sin contar las que el propio documento ya lleva
> tachadas—. Pudo juzgar **155** (las demas no nombran ningun simbolo al lado, asi que ninguna
> maquina puede decir si aciertan), y de esas **111 no apuntan a donde dicen**. No es que se escribieran mal: **un numero de linea caduca en cuanto alguien inserta
> veinte lineas encima**, y en este repositorio se insertan todas las semanas. Renumerarlas a mano
> es repetir el mismo trabajo dentro de siete dias — y este documento ya lo hizo una vez: el punto
> 7 de la seccion B renumero tres citas el 31/08 y **las tres vuelven a estar caducadas hoy**.
>
> **Desde esta pasada se cita el SIMBOLO y se deja escrito el `grep` que lo encuentra:** una
> funcion, un `#define`, una constante, el literal de un comando, o la **marca `N-xxx`** que el
> propio fuente lleva en sus comentarios —el firmware de este repositorio las trae, y por eso
> valen como ancla—. Un simbolo sobrevive a que el fichero se reordene por dentro; un numero no.
>
> **Donde el numero siga aportando** —una constante concreta, una fila de tabla— **se deja, pero
> verificado en esta pasada y con esa fecha al lado.** Y una cita que apuntaba a algo que ya no
> existe no es una cita mal numerada: es una **afirmacion falsa**, y va tachada con su motivo. Las
> que aparecieron estan marcadas 🔴 **AFIRMACION FALSA** donde estaban.

> **Este documento no cambia ningun otro fichero.** Todo lo que otro documento necesita corregir
> esta listado en la seccion B y en el anexo final, y **no se ha tocado**. Habia otros trabajos en
> vuelo sobre el mismo arbol el dia que se escribio.

---

## 🔴 REVISION DEL 03-04/09/2026 — EL BANCO CORRIO. Leer esto ANTES que todo lo demas

**Los dias 3 y 4 de septiembre de 2026 este equipo estuvo en un banco, con multimetro, sobre el
paquete V9.0 (commit `617bd00`).** Informe: `evidencia/Informe_Pruebas_Banco_Semaforos_V9.0.pdf`.
Es la primera vez que algo de este documento se contrasta contra **cobre** en vez de contra un
dibujo, y eso **cambia el nivel de varias filas, no solo su contenido**.

**El resultado, sin adornos — y la CUENTA no la publica este documento, que es lo que se corrige el
04/09.** La **cabecera del informe** declara **24 completos · 4 bloqueados por el enlace Bluetooth ·
1 abortado por seguridad**, sobre **29 pasos**. Su **propia enumeracion** nombra `1-6, 8-9, 15-18,
20-24` (**17** identificadores) · `25-28` (**4**) · `29` (**1**): **22 de los 29**. Los **siete que
faltan** —`7`, `10`, `11`, `12`, `13`, `14`, `19`— si estan descritos en el cuerpo del informe (`7`
y `19` como *PARCIAL*, `10` como *no logrado*, `11` y `12-14` como *BLOQUEADO*), pero **la cabecera
no los mete en ningun cajon**. Repartidos como el cuerpo los describe, la cuenta que cuadra a 29 es
**19 / 9 / 1**.

> **Aqui no se publica ninguna de las dos: se publica la DISCREPANCIA.** ~~*"24 completos (pasos
> 1-6, 8-9, 15-18, 20-24), 4 bloqueados y 1 abortado; los pasos 7, 19 y 21 quedaron parciales y se
> cuentan dentro de los 24"*~~ → 🛑 **CADUCADO el 04/09: esa reconciliacion no sale, y no era del
> informe — se escribio aqui.** Los identificadores de esa lista son **17**, no 24; el **paso 21 ya
> estaba dentro** de `20-24`, asi que sumarlo aparte como *parcial* no cuadra nada; y los pasos **10
> a 14 no caian en ningun cajon**. Era un total escrito a mano para que la cabecera pareciera
> reconciliada, y **un total inventado es peor que un hueco declarado**.
>
> **Cual de las dos vale lo decide quien ejecuto la sesion, no este repositorio.** Lo que si se deja
> escrito es **donde esta la diferencia** —los pasos **10 a 14**, el modulo que no se anuncia y todo
> lo que cuelga de el—, porque **un paso que no esta en ningun cajon es un hueco que no deja rastro
> de que falta**. *(Mismo criterio, y misma medida, que
> `12_Cobertura_de_Pruebas_y_Huecos.md` §«La cuenta, y por que este documento no la copia»: ninguna
> linea del informe miente, y la cuenta de su cabecera no sale.)*
>
> **Lo que NO depende de la cuenta y sigue siendo cierto:** los pasos **7 y 19** quedaron
> **parciales** —la parte de cableado y de medida se verifico; la respuesta funcional depende del
> Modo Automatico, que no se pudo seleccionar sin app—, y el **21** si esta en la lista de la
> cabecera: cablea camara, y esa parte se verifico entera.

| # | este documento decia | lo que midio el banco |
|---|---|---|
| **M3** | *"la medida que desbloquea las camaras"*, **pendiente**, y sobre ella colgaba *"mientras esto no se mida, no se cablea camara a `J16`"* | ✅ **CERRADA el 03/09.** Numeros en la tabla de abajo y en la seccion A · M3 |
| **§2.2** | *"contradiccion: el netlist dice activo en ALTO, el firmware dice activo en BAJO, y no se puede decidir desde aqui"* | 🔴 **LA DECIDE EL COBRE (N-118): activo en ALTO los CUATRO.** No era una contradiccion — era **un fuente equivocado** en `A` y `B` |
| **§2.4 y §3.3** | el veto de `mando_ambarLocal()` se queda, y `B·B·B` es la salida fisica de ultimo recurso | 🔴 **EL MANDO `A`/`B` NO SE PUDO PULSAR EN BANCO (N-118)** — con `617bd00`, el pin en `0,6 V` permanente y el firmware leyendo activo en BAJO: no habia transicion que detectar. 🟢 **El fuente ya esta corregido en las dos puntas (`346ea5f`)**; 🔴 **sin ejercer en tarjeta** |
| — | — | 🔴 **NUEVO, N-120: la tarjeta protege sus 9 SALIDAS y no protege NINGUNA de sus 5 ENTRADAS de campo.** Abre una decision de diseno para V2, y es del responsable: **§3.6** |
| — | — | 🔴 **NUEVO, N-116: la tarjeta Maestro esta FUERA DE SERVICIO.** Corto MEDIDO entre `3,3 V` y `GND`. **No se reenergiza** |
| **N-42** | *"el Modo Automatico no mueve las luces"*, regresion abierta | ⚠️ **NI CONFIRMADA NI DESCARTADA.** El equipo nunca llego a operar —falto la app—. Sigue abierta y sigue siendo lo primero de la proxima sesion |
| **§3.1** | *"nadie ha leido la serigrafia del modulo"* — la bloqueante mas barata de la lista | 🟢 **ES un `ESP32-WROOM-32` clasico** (BR/EDR + BLE). El perfil que el SPP de la app necesita existe: **el chip no era la causa** |
| **§1.4** | el nombre del pin 3 de `J17` *"sigue en disputa"* | 🟢 **RESUELTO en banco (paso 5):** continuidad `J17` p3 -> pata 42 y p2 -> pata 43, y **sin 12 V en ninguna posicion**. El netlist quedo viejo; el enlace es el que describe §1.4 |

### Lo que se midio en `J16`, que es el nucleo de todo lo anterior

**MEDIDO EN COBRE** —multimetro, conector vacio, paso 20 de la Guia, 03/09/2026—:

| `J16` | R a masa (sin energia) | R a `3,3 V` (sin energia) | V contra masa (con energia) |
|---|---|---|---|
| p5 — `MANDO_A` (`PB9`) | **9,92 kOhm** | 11,28 kOhm | 🔴 **0,6 V** |
| p8 — `MANDO_B` (`PB13`) | **9,92 kOhm** | 11,28 kOhm | 🔴 **0,6 V** |
| p10 — Camara (`PB14`) | **9,93 kOhm** | 11,29 kOhm | **0 V** |
| p12 — Camara (`PB15`) | **9,94 kOhm** | 11,31 kOhm | **0 V** |

**El pull-down de 10 kOhm que declaraba el netlist es REAL, y esta en las cuatro posiciones**
(`R65`-`R68`, con su `100 nF`). Con `3,3 V` en la posicion de al lado en las cuatro —`J16` p4, p7,
p9 y p11—, el gesto que el conector pide es cerrar el contacto **contra los `3,3 V`**: **entrada
activa en ALTO, para los cuatro pines y sin excepcion.**

**De ahi salen dos conclusiones distintas, una buena y una mala:**

- 🟢 **El camino de camara es correcto, y ademas esta EJERCIDO.** `pinMode(INPUT)` pelado con
  deteccion contra `HIGH` —lo que N-67 dejo, y que **desde N-97 vive en `botones.cpp`, no en
  `modo_inteligente.cpp`**: `camara_leerPin()` y los tres `pinMode(..., INPUT)` de
  `botones_setup()`— es exactamente lo que el cobre pide. El **paso 21** cableo `p10` contra `p11`
  (`3,3 V`) en normalmente abierto y **el equipo no pidio paso solo, ni con el cable puesto ni sin
  el**: **no hay demandas fantasma**. La consecuencia operativa de §2.2 —*"mientras esto no se
  mida, no se cablea camara a `J16`"*— **queda levantada.**
  > ~~`modo_inteligente.cpp:46` y `:25`~~ → 🔴 **AFIRMACION FALSA, corregida el 05/09.** Ahi no hay
  > camara: lo que hay en esas lineas son los comentarios de **N-97** —que explica que el
  > `pinMode(CAM_DEMANDA_PIN, INPUT)` **se fue de esa funcion** porque un modo no es dueno de una
  > entrada fisica— y de **N-135**. El camino de camara se localiza asi, y sale igual en las dos
  > puntas:
  > ```
  > grep -n "camara_leerPin" Maestro/src/botones.cpp Maestro/include/botones.h
  > grep -n "pinMode(CAM_" Maestro/src/botones.cpp Esclavo/src/botones.cpp
  > grep -n "N-97" Maestro/src/modo_inteligente.cpp
  > ```
- 🔴 **El camino de mando ESTABA invertido, y no era un matiz de estilo: EN BANCO NO SE PUDO PULSAR
  (N-118).** El firmware que se llevo —`617bd00`— leia `INPUT_PULLUP` y `== LOW`. Con el pin en
  `0,6 V` en reposo la lectura cruda ya era `LOW` **desde el arranque**, el antirrebote lo sembraba
  como *flanco ya consumido*, y **la transicion que `mando_registrarPulso()` espera no llegaba
  nunca**. El banco lo vio: se puenteo `p5`/`p8` contra masa y **el semaforo no cambio de
  comportamiento** (paso 29).
  > 🟢 **ESTADO DE HOY, 04/09 — el fuente ya no es el defecto.** `346ea5f` deja `pinMode(BOTON1,
  > INPUT)` / `pinMode(BOTON2, INPUT)` pelados y `digitalRead(b.pin) == HIGH` en **las dos puntas**
  > —`botones.cpp`, funcion `botones_setup()` para los `pinMode` y el lector antirrebotado
  > `actualizar()` para la comparacion, con la marca `N-118` encima de los dos—: el mismo arreglo
  > que N-67 le hizo a la camara. Se localiza asi, y **da las dos puntas de una vez**:
  > ```
  > grep -n "pinMode(BOTON[12], INPUT)" Maestro/src/botones.cpp Esclavo/src/botones.cpp
  > grep -n "digitalRead(b.pin) == HIGH" Maestro/src/botones.cpp Esclavo/src/botones.cpp
  > grep -n "N-118" Maestro/src/botones.cpp Esclavo/src/botones.cpp
  > ```
  >
  > 🔴 **Y sigue PENDIENTE de ejercer en tarjeta**, que es lo unico que lo cerraria: **nadie ha
  > visto todavia a este equipo obedecer un `A·A·A`**, y no se prueba sobre la Maestro con el corto
  > (N-116). **Un arreglo en el fuente no es un arreglo cargado.**
  >
  > ⚠️ **Cambia el GESTO DE PRUEBA, y esto es lo que se lleva al banco:** ~~tocar `J16` p5 contra
  > masa~~ → **cerrar `p5` contra `p4` y `p8` contra `p7`, que son los `3,3 V` del pin contiguo**.
  > El gesto viejo es el del paso 29, el que acabo con el Maestro caliente.

### 🟢 D-12 — EL CONTRATO REAL DE LAS CAMARAS: **UN CONTACTO SECO, y nada mas**

> **De cada camara el sistema consume UN CONTACTO SECO. No hay red, no hay imagen, no hay video, y
> no hay analitica en el controlador.** (`DECISIONES.md`, fila **D-12**, 05/09.)

**Va aqui, junto a la medida de `J16`, porque es exactamente lo que esa medida dice y nadie lo
habia escrito:** lo que llega del conector es **un contacto que se cierra o no se cierra**, leido
por `camara_leerPin()` como un nivel alto. Un bit. El firmware no sabe —ni puede saber— si detras
de ese contacto hay una AcuSense, un lazo inductivo o un pulsador. El propio `pines.h` ya lo
rotula asi: *"J16 p10 - camara de contacto seco"*.

**MEDIDO el 05/09, y se publica el control negativo al lado porque §4 obliga a descartar al
buscador antes de creerse un cero:**

| que se busco | en donde | resultado |
|---|---|---|
| `WiFi`, `HTTPClient`, `WebServer`, `esp_camera`, `ONVIF`, `rtsp`, `mqtt`, `lwip` | `01_Firmware/ESP32_Expansion/src` + `include` + `platformio.ini` | **0 en todos** |
| simbolos `esp_wifi_init` / `esp_wifi_start` / `httpd_start` / `lwip_socket` / `esp_camera_init` **enlazados** | `ESP32_Expansion/.pio/build/esp32_expansion/firmware.elf`, con `xtensa-esp32-elf-nm` | **0 definidos** |
| **control negativo** — que el buscador SI encuentra | los mismos ficheros | `BluetoothSerial` **4** · `Serial2` **3** · `Wire` **13** |

```
cd 01_Firmware/ESP32_Expansion
grep -rowE "\bWiFi\b|\bHTTPClient\b|\bWebServer\b|\besp_camera\b|\bONVIF\b|\blwip\b" src include platformio.ini
grep -rowE "\bBluetoothSerial\b|\bSerial2\b|\bWire\b" src include platformio.ini    <-- el control negativo
```

**Las dos consecuencias, y la segunda es la que duele:**

1. 🟢 **Toda la inteligencia vive en la CONFIGURACION DE LA CAMARA**, no en el controlador. El
   `9_Manual_Parametrizacion_Camara_IA.md` deja de ser documento de apoyo y pasa a ser
   **entregable principal**: es donde se decide que significa el contacto.
2. 🟠 **El CONTROLADOR no ve imagen, y por eso D-12 deroga la propuesta de *"imagenes y
   auditoria en la Raspberry o la Nano"*** que **dos revisiones recomendaron el 05/09 heredando la
   idea sin comprobar que existiera camino**.
   > ✏️ **CORREGIDO EL MISMO 05/09, y se deja escrito lo que decia porque el matiz cambia una
   > compra:** ~~*"sin imagenes NO hay soporte de accidentes ni auditoria"*~~ → **la camara SI
   > graba, en su propia microSD** —`DS-2CD2683G2-IZS`, hasta 512 GB por ficha oficial—, asi que
   > **el soporte de accidentes y la auditoria si son posibles: EN LA CAMARA, no en nuestro
   > firmware**. Es una tarjeta que hay que **comprar y configurar**, y **no toca una linea de
   > codigo**. Queda abierta como **`A-0`** en `DECISIONES.md` —que tamano, cuantos dias de
   > retencion, y si graba continuo o por evento—.
   >
   > **La fila de `DECISIONES.md` manda sobre este parrafo**, y este bloque se escribio con la
   > version anterior de esa fila: es exactamente el caso que el encabezado de `DECISIONES.md`
   > describe.

> ⚠️ **Y lo que este apartado NO dice, para que no se lea de mas:** que el sistema consuma un solo
> bit por camara **no es un defecto de la camara**. La Hikvision `DS-2CD2683G2-IZS` (D-10) graba y
> analiza por su cuenta; lo que no hay es **camino desde ella hasta este controlador** para nada
> que no sea el contacto. Si algun dia se quiere imagen, es un equipo nuevo en el armario, no una
> version del firmware.
>
> **Y la consecuencia que decide `A-1`:** con dos camaras y un bit cada una, **el cruce entero
> tiene DOS significados**, no dos canales de informacion. Uno esta tomado —presencia antes de
> bajar la pluma—; el otro es la pregunta abierta.

> 🛑 **Lo que eso le hace a §3.3, y hay que leerlo entero.** La decision del 31/08 —*"se conserva el
> mando en `A` y `B`"*— se eligio porque era la unica salida fisica de ultimo recurso que **ya estaba
> construida**. El cobre dice que **no lo estaba**: `A·A·A`, `B·B·B` y `A·B·A·B` existen en el
> firmware y **no existen en la mano de nadie**. El coste de la decision seguia siendo cero; lo que
> costaba cero era tambien lo que resolvia.
>
> **No es una decision equivocada: es una decision tomada sobre un dato que no se tenia** —y que este
> documento pedia por escrito, en M3—. Ahora se tiene. Lo que hacia falta para que el mando exista de
> verdad era **una linea de `botones.cpp` por punta** (Anexo, punto 5), y ya no es *"si M3 dice"*: M3
> lo dijo.
>
> 🟢 **Y esa linea YA ESTA ESCRITA, en las dos puntas: `346ea5f`.** ~~*"mientras eso no entre"*~~ →
> entro.
>
> 🔴 **Lo que NO ha pasado, y es lo que decide: no se ha cargado ni ejercido en una tarjeta.** Asi
> que **lo que va en el parte sigue siendo lo mismo, por un motivo distinto**: nadie ha visto a este
> equipo obedecer un `A·A·A`, el agujero que §3.3 daba por tapado sigue abierto **hasta la carga
> verificada**, y esa prueba **no se hace sobre la Maestro con el corto** (N-116). *(`CLAUDE.md` §9.bis:
> se exige la carga verificada, no el merge.)*

### 🔴 N-120 — la tarjeta protege todo lo que sale y no protege nada de lo que entra

**MEDIDO en banco (03-04/09), y coherente con el netlist:** las **9 salidas** de campo van con
**220 Ohm en serie + optoacoplador `TLP127`**; las **5 entradas** de campo van **del borne directo a
la pata del STM32** — sin resistencia en serie, sin opto y sin clamp. Y `J16` p1 reparte **12 V
crudos** en ese mismo conector (§2.1).

**Consecuencia inmediata, y deja de ser una cautela de mesa:** ~~tapar `p1` antes de cablear nada en
`J16`~~ → **tapar `p1` es OBLIGATORIO en cada equipo que se monte**, no solo en el banco. En banco se
hizo **retirando el pin del cuerpo del conector volante** (paso 4), que es mas fiable que la funda
termorretractil porque no se puede deshacer por accidente. Es el metodo que se documenta.

**La decision de diseno que abre —2K2 en serie por entrada, con su cuenta— es de V2, es del
responsable, y esta en §3.6.**

### 🔴 N-116 — la tarjeta Maestro esta fuera de servicio

**MEDIDO despues del incidente del paso 29: hay un corto entre `3,3 V` y `GND`.** La tarjeta arranca,
funciona unos **30 segundos** y se calienta. 🛑 **No se reenergiza** hasta inspeccion tecnica con el
equipo frio.

**El firmware queda descartado como causa, y por CENSO, no por opinion:** ninguna de las **9 salidas**
que el firmware escribe toca un pin de `J16`. Nada de lo que pasa por `escribirPines()` puede llegar
a `p5` ni a `p8`. **Eso no nombra al culpable** —el informe deja la causa abierta, y hace bien—: lo
que hace es **sacar de la lista al unico sospechoso que este repositorio podia haber revisado
leyendo**, que es justo el que se habria revisado.

### 🟢 El Bluetooth eran DOS defectos en serie, y los dos estan arreglados

**N-117 y N-122.** El enlace no se establecio en toda la sesion y bloqueo en cascada los pasos 25 a
28, mas la parte funcional de 7, 19 y 21. Eran dos, uno en cada punta del cable:

| # | donde | que pasaba |
|---|---|---|
| **N-117** | ESP32 | **el watchdog se comia su propio arranque** — el modulo no llegaba a anunciarse |
| **N-122** | app | **nunca llamaba a `connect()`** |

**Los dos arreglados, y hay APK nueva.** ⚠️ **Pero eso es firmware y app, no banco: lo medido aqui es
el defecto, no el arreglo.** Que el enlace suba se demuestra repitiendo los pasos 25-28 — no leyendo
esta tabla.

---

## 🔵 REVISION DEL 04/09/2026, MAS TARDE EL MISMO DIA — TRES CAMBIOS Y UNA DECISION

**Esto NO lo trajo el banco.** La revision de arriba se midio con multimetro; esta se midio **sobre
ficheros** —el `.cpp` y el `.h`—, y por eso va aparte y no se mezcla con aquella: es `MEDIDO`, no
`MEDIDO EN COBRE`. **Nada de lo que sigue se ha cargado en una tarjeta.**

| # | que cambia | donde se comprueba | nivel |
|---|---|---|---|
| **1** | **El minimo del ciclo sube de 1 a 3 minutos.** Cierra `N75-1`, abierta desde el 26/08 (§3.4) | `Maestro/include/limites_ciclo.h`, constantes `VERDE_MIN_MIN` / `ROJO_MIN_MIN` / `DESPEJE_SEG_MIN` · `grep -n "VERDE_MIN_MIN" Maestro/include/limites_ciclo.h` | **MEDIDO** |
| **2** | **DECISION DEL RESPONSABLE: el cruce se opera DESDE EL MAESTRO.** Se descarta relevar el mando por radio desde el Esclavo | **§3.7, nueva** | **decision** |
| **3** | **N-130: el acuse de la demanda dice si se va a atender**, y la bandera solo se arma si hay quien la consuma | la marca `N-130` en las cuatro puntas del cambio: `grep -rn "N-130" Maestro/src/coordinador.cpp Maestro/include/protocolo.h Esclavo/src/main.cpp Esclavo/include/protocolo.h` | **MEDIDO** |
| — | 🔴 **ABIERTA, nueva:** el rotulo Bluetooth de un modulo virgen es **el mismo en las dos puntas** hasta la segunda arrancada | **§3.8, nueva** | **MEDIDO** |

### 1 · El minimo del ciclo sube de 1 a 3 minutos

**MEDIDO** en `01_Firmware/Maestro/include/limites_ciclo.h` — `grep -n "VERDE_MIN_MIN"
Maestro/include/limites_ciclo.h`:

```
   static const uint8_t VERDE_MIN_MIN = 3,  VERDE_MIN_MAX = 15;
   static const uint8_t ROJO_MIN_MIN  = 3,  ROJO_MIN_MAX  = 15;
   static const uint8_t DESPEJE_SEG_MIN = 10, DESPEJE_SEG_MAX = 90;
```

> ~~`01_Firmware/Maestro/src/modo_automatico.cpp:51-53`~~ → 🔴 **AFIRMACION FALSA, corregida el
> 05/09: ya no es ese fichero.** Las seis constantes **se mudaron a `include/limites_ciclo.h` por
> N-137**, y este mismo documento lo cuenta unas lineas mas abajo sin haber corregido la cita de
> arriba. En `modo_automatico.cpp` ya solo quedan **usos**, no la declaracion:
> `static int minRojo = ROJO_MIN_MIN, minVerde = VERDE_MIN_MIN, segEstatico = DESPEJE_SEG_MIN;`.

**El porque, con las palabras del responsable: «tres minutos es la minima distancia de seguridad».**
El razonamiento largo esta escrito **en la cabecera de `limites_ciclo.h`** —el bloque *"LOS LIMITES
DEL CICLO VIVEN AQUI, Y EN UN SOLO SITIO"*— y **no se copia aqui**: dos
versiones de un motivo son dos cosas que alguien tiene que sincronizar, y este documento ya sabe
como acaba eso. En una linea: en un paso alternado de un solo carril, un camion pesado tarda entre
5 y 8 s **solo en reaccionar y arrancar**; con un verde de 60 s lo que se produce no es una cola, es
un conductor convencido de que el semaforo esta averiado.

**Y la mitad que decide DONDE vive la guarda, que es lo que la hace firmware y no interfaz.** La app
valida por comodidad, pero **la app no es la unica que puede hablar por `J17`**: cualquier otra cosa
en ese cable —o una APK vieja, que el 04/09 se demostro que sobreviven en los telefonos— puede
mandar `SET_TIEMPOS` con un minuto. La guarda esta en `modoAutomatico_fijarTiempos()`
—`grep -n "bool modoAutomatico_fijarTiempos" Maestro/src/modo_automatico.cpp`— y el rechazo sale
por el literal `$ERR,CMD:SET_TIEMPOS,DESC:RANGO`, que se localiza con
`grep -n "DESC:RANGO" Maestro/src/bluetooth.cpp`. **Una guarda que solo vive en la interfaz es de
cortesia.**

**COSTE ACEPTADO A SABIENDAS, y va escrito porque se paga en la proxima sesion de banco:** ya **no
se puede probar en mesa con ciclos de un minuto**. Un banco cae del lado de esperar tres minutos, no
del lado de dejar un limite de laboratorio suelto en una carretera. **Quien planifique la proxima
visita tiene que contar tres minutos por paso al escribir los tiempos.**

> ✅ **N75-2 se cierra con el mismo cambio, y no por casualidad: al ir a subir el numero aparecio que
> los seis limites estaban escritos a mano en TRES sitios y tres lenguajes.** Hoy los tres coinciden
> y hay un pack que los relee en cada corrida —`app_11_rangos_de_tiempos`, que lee el C++, el
> `enRango(...)` de `app.js` y los `min=`/`max=` de `index.html`—. **MEDIDO** en
> `01_Firmware/Simulaciones/banco/packs/app_11_rangos_de_tiempos.py` — su cabecera y su
> `DESCRIPCION`, que se localizan con
> `grep -n "DESCRIPCION" Simulaciones/banco/packs/app_11_rangos_de_tiempos.py`.
>
> ~~🔴 **Y el hueco que ese pack NO cubre, medido en esta misma pasada: hay un CUARTO sitio, y hoy
> dice lo contrario que el firmware.** `05_Funcional/App_Semaforo/js/config.js:10-17` declara
> `LIMITES_TIEMPO` con `VERDE_MIN_MIN: 1` y `ROJO_MIN_MIN: 1`, bajo el comentario «Rangos de Tiempos
> Permitidos por Firmware» — que hoy es falso. `index.html:718` lo carga~~
>
> → 🔴 **AFIRMACION FALSA, corregida el 05/09. El cuarto sitio YA NO EXISTE: se borro el 04/09**, en
> el mismo dia en que este parrafo lo denunciaba. `js/config.js` conserva **solo el comentario que
> explica la retirada** —*"LOS LIMITES DE TIEMPO SE RETIRAN DE AQUI (04/09/2026). NO SE ACTUALIZAN:
> SE BORRAN"*—, y el censo lo confirma: `grep -rn "LIMITES_TIEMPO" 05_Funcional/App_Semaforo`
> devuelve **solo lineas de comentario**, ninguna declaracion. La cita a `index.html:718` tampoco
> apuntaba a nada: el `<script src="js/config.js">` se localiza con
> `grep -n 'src="js/config.js"' App_Semaforo/www/index.html`.
>
> 🔴 **Y este pie de pagina se demostro a si mismo en HORAS.** Al hacer esta pasada se escribio
> aqui *"esta en la linea 937"*, verificado a mano. **Antes de terminar la misma sesion ya no era
> cierto para las dos copias**: `App_Semaforo/index.html` lo tiene en la **949** y
> `App_Semaforo/www/index.html` en la **937**. Un numero verificado por uno mismo, el mismo dia,
> caducado antes de cerrar el fichero. **Eso es lo que esta pasada viene a arreglar.**
>
> ⚠️ **Lo que de aquel parrafo SIGUE siendo cierto, y por eso no se borra entero:** el resto de
> `IOT_CONFIG` tampoco tiene consumidores —lo dice el propio fichero, *"AVISO APARTE, y no se
> arregla en este commit"*—, asi que el huerfano de los de N-73 **no ha desaparecido: ha
> adelgazado**. Queda en el Anexo, punto 6. **No se toca desde este documento**: la app la lleva
> otro trabajo.

### 2 · La demanda que se acusaba y no se atendia (N-130)

**El defecto, MEDIDO por censo y no por lectura.** El Esclavo contestaba
`$ACK,CMD:SOLICITAR_PASO,RESULT:PEDIDO_AL_MAESTRO` —`grep -n "PEDIDO_AL_MAESTRO"
Esclavo/src/bluetooth.cpp`— y el Maestro armaba `demandaRemotaPendiente` **siempre**. El censo es
este, y lo que importa de el **no son los numeros de linea: es que salga UN SOLO fichero
consumidor**, asi que se deja el comando y no su salida de un dia:

```
   grep -rn "coordinador_hayDemandaRemota\|coordinador_limpiarDemandaRemota" Maestro/src
      ->  coordinador.cpp        las dos DEFINICIONES
      ->  modo_inteligente.cpp   los CUATRO usos      <-- UN SOLO FICHERO CONSUMIDOR
```

**En Modo Manual y en Modo Automatico nadie la lee.** El operario de pie junto al Poste 2 pulsaba,
leia la confirmacion, y el cruce no se movia. Vuelve a pulsar. **Es la barrera de `CLAUDE.md` §6 —un
`$ACK` que no depende de lo que paso— pero repartida entre DOS placas**, y por eso ninguna de las
dos ramas parecia mal escrita leida por separado.

**Lo que hay hoy, MEDIDO:**

*(Las tres filas se localizan por simbolo, no por numero. `grep -rn "N-130" Maestro Esclavo` da las
cuatro puntas del cambio de una vez.)*

| donde | que hace |
|---|---|
| `protocolo.h`, los dos `#define` (identicos en las dos puntas) — `grep -n "DEMANDA_ACEPTADA" Maestro/include/protocolo.h` | `DEMANDA_ACEPTADA 0` / `DEMANDA_RECHAZADA 1` |
| `Maestro/src/coordinador.cpp`, la rama de `CMD_DEMANDA` bajo la marca `N-130` | la bandera **solo se arma si el modo la va a consumir**, y el acuse viaja con el motivo en el byte `param` |
| `Esclavo/src/main.cpp`, la rama `pkt.command == CMD_ACK_DEMANDA` — `grep -n "CMD_ACK_DEMANDA" Esclavo/src/main.cpp` | si llega `DEMANDA_RECHAZADA`, levanta el evento `MAESTRO / DEMANDA_NO_ATENDIDA_MODO_ACTUAL` |

**Dos detalles del diseno que conviene no perder, porque son los que lo hacen barato:**

- **No gasta un codigo de comando ni cambia la trama.** El `param` ya viajaba y ya va cubierto por
  el CRC —`calcularCRC_Bin(&pkt, 3)` incluye `msgID`, `command` y `param`—.
- **`DEMANDA_ACEPTADA` vale `0` a proposito.** `protocolo_enviarPaquete()` pone `param = 0` por
  defecto, asi que un Maestro sin actualizar se sigue leyendo como *aceptada* y se comporta como
  siempre: **si la compatibilidad fallara, fallaria hacia el silencio de hoy y no hacia una alarma
  inventada.**

**Y el molde estaba dentro del mismo sistema:** la rama `DEMANDA` **local** del Maestro ya rechazaba
fuera del Modo Inteligente —`grep -n "SOLO_EN_MODO_INTELIGENTE" Maestro/src/bluetooth.cpp` da la
unica linea que emite `$ERR,CMD:DEMANDA,DESC:SOLO_EN_MODO_INTELIGENTE`—. Lo que faltaba era la
misma regla **al otro lado de la radio**.

> ⚠️ **Lo que N-130 NO cierra, y va escrito al lado:** el rechazo llega a la app como **evento de
> bitacora**, no como `$ERR` —y esta razonado: un `$ERR` habria que casarlo con una orden contestada
> cientos de milisegundos antes, y con dos pulsaciones seguidas la app no sabria a cual corresponde
> — el razonamiento esta escrito dentro de la propia rama `CMD_ACK_DEMANDA` del Esclavo,
> `grep -n "CMD_ACK_DEMANDA" Esclavo/src/main.cpp`—. **Que la app pinte ese evento y el operario lo
> lea NO se ha comprobado.** Sin eso el cierre es medio: se deja de mentir, pero puede no decirse nada.

### 3 · La decision del responsable

**Va desarrollada en §3.7**, que es nueva, con la alternativa descartada, el censo que la respalda,
su consecuencia operativa y su coste. La pregunta que abre —el rotulo— va en **§3.8**, abierta.

> 🔴 **EL COSTE DE FLASH DE ESTOS CAMBIOS NO SE PUBLICA COMO DATO: SE PUBLICA LA
> DISCREPANCIA.** Es `CLAUDE.md` §4 y §4.bis, y el mismo criterio con el que la cabecera trata la
> cuenta del informe de banco.
>
> El acta mas reciente es `evidencia/2026-09-04_compuerta.txt`, `HEAD 624eb37`, y **es ANTERIOR a
> estos cambios**: publica `89.3 %` (`58496` de `65536` B) en el Maestro y `65.9 %` (`43220` B) en el
> Esclavo. El coste se reporto como **`+36` B** en el Maestro y **`+116` B** en el Esclavo.
>
> **Medido aqui sobre el `.elf` construido despues del cambio** —`arm-none-eabi-size -A`, sumando
> las secciones cargables, que es la cuenta que reproduce exactamente las cifras de las actas
> anteriores y que coincide byte a byte con el tamano del `.bin`—:
>
> ```
>    Maestro   58852 B  (89,8 %)      acta 624eb37: 58496 B   ->  +356 B
>    Esclavo   43656 B  (66,6 %)      acta 624eb37: 43220 B   ->  +436 B
> ```
>
> **Las dos parejas no reconcilian, y este documento no elige entre ellas.** La explicacion mas
> probable es que **el acta no sea el extremo bueno del delta** —entre `624eb37` y hoy hay mas de un
> cambio, y §4.bis avisa de que *un delta exige medir los DOS extremos*—; pero eso es una hipotesis,
> no una medida. **La cifra que vale sale de correr la compuerta sobre el arbol de hoy**, y hasta
> entonces aqui no va ningun porcentaje de flash como dato.

---

## 🟢 REVISION DEL 04/09/2026, DE NOCHE — CINCO CAMBIOS, UNA DECISION Y UN HALLAZGO NUEVO

**Tampoco lo trajo el banco.** Se midio **sobre ficheros** —el `.cpp`, el `.h`, el `.js` y el
`.html`—, igual que la revision de la tarde, y por eso va aparte de la del cobre. **Nada de lo que
sigue se ha cargado en una tarjeta ni se ha ejercido con un telefono delante.**

| # | que cambia | donde se comprueba | nivel |
|---|---|---|---|
| **1** | **N-134: EL AMBAR SE ORDENA.** `CMD_GO_AMBAR` (`0x13`) en vez de esperar la orfandad de 25 s. **Decision del responsable** | `#define CMD_GO_AMBAR` en los dos `protocolo.h` · la rama `pkt.command == CMD_GO_AMBAR` de `Esclavo/src/main.cpp` · `grep -rn "CMD_GO_AMBAR" Maestro Esclavo` y `grep -rn "N-134" Maestro Esclavo` | **MEDIDO** |
| **2** | **N-133: los tiempos del ciclo automatico sobreviven al corte.** Entran en el respaldo con pila, `DR9`/`DR10` | `respaldo_guardarTiemposCiclo()` y los `REG_CICLO_RV` / `REG_CICLO_DESPEJE` de `Maestro/src/respaldo.cpp` · `grep -rn "N-133" Maestro` | **MEDIDO** |
| **3** | **N-42: el Modo Automatico arranca corriendo.** Se retira el asistente de tres fases, huerfano desde que `botonAceptar()` devuelve `false` | `modoAutomatico_setup()` de `Maestro/src/modo_automatico.cpp` · `grep -n "N-42" Maestro/src/modo_automatico.cpp` | **MEDIDO** |
| **4** | **N-135: el `enum` de un solo valor.** Ver el bloque propio de abajo — **es el hallazgo, no el cambio** | la cabecera de `Maestro/src/modo_automatico.cpp` y `modoAutomatico_enMarcha()` · `grep -n "N-135" Maestro/src/modo_automatico.cpp Maestro/src/modo_inteligente.cpp` | **MEDIDO** |
| **5** | **N-106 CERRADO:** el ambar de la app sale del Degradado por el todo-rojo, y contesta cinco cosas distintas | las ramas `strcmp(cmd, "CMD:AMBAR_EMERGENCIA")` y `strcmp(accion, "AMBAR_EMERGENCIA")` de `Esclavo/src/bluetooth.cpp` · `grep -n -e "CMD:AMBAR_EMERGENCIA,RESULT" -e "CMD:AMBAR_EMERGENCIA,DESC" Esclavo/src/bluetooth.cpp` da **las cinco respuestas distintas**, repetidas en los dos bloques —el sin PIN y el con PIN— porque son *el mismo bloque letra por letra* | **MEDIDO** |
| **6** | **La app cambia de barrera:** para ABRIR paso pregunta si se ha mirado el tramo, no el PIN. Y el PIN caduca — cierra `AB-9` | `App_Semaforo/www/app.js`: `VIA_VIGENCIA_MS`, `confirmarVia()`, `caducarPin()` y la lista `SIN_PIN` · `grep -n -e VIA_VIGENCIA_MS -e "function confirmarVia" -e "function caducarPin" -e "const SIN_PIN" App_Semaforo/www/app.js` | **MEDIDO** |
| — | 🔴 **ABIERTA, nueva:** `respaldo_borrar()` no limpia los dos registros que N-133 estrena, y los sella como validos. **§3.9** | `respaldo_borrar()` contra la suma que si los incluye — `grep -n -e "void respaldo_borrar" -e REG_CICLO_RV Maestro/src/respaldo.cpp` | **MEDIDO** |

### 1 · N-134 — el ambar se ordena, y el rojo previo NO se toca

**El defecto, en pasado:** poner ambar desde el Maestro dejaba al Esclavo en ROJO. Nadie se lo
decia. Lo que acababa llevandolo a ambar era la **orfandad**: dejaba de oir al Maestro y a los
`SFTY6_SILENCIO_MS` (**25 s** — `grep -n "define SFTY6_SILENCIO_MS" Maestro/include/protocolo.h
Esclavo/include/protocolo.h` da el mismo valor en las dos puntas, verificado el 05/09) se iba solo. Las dos puntas
acababan en ambar **con hasta 25 s de diferencia**, y en banco eso se vio como *"a veces los dos, a
veces solo el maestro"*.

**Las dos mitades del diseno, que son lo que hay que no perder:**

- **El rojo previo se queda como intermedio seguro.** `modo_ambar_setup()` llama a
  `coordinador_forzarRojoTotal()` **primero** —y esa funcion es la que manda `CMD_GO_RED`— y el
  ambar despues. **No se salta de un verde a un ambar intermitente.**
  ```
  grep -n "coordinador_forzarRojoTotal" Maestro/src/modo_ambar.cpp Maestro/src/coordinador.cpp
  ```
- **La orfandad sigue como RED, no como camino.** El Esclavo **no refresca** `tUltimoComando` al
  atender esta orden —el comentario que lo razona esta en la propia rama de `CMD_GO_AMBAR` de
  `Esclavo/src/main.cpp`—, a proposito: si la orden se pierde en el aire, el **fallback de
  `SFTY6_SILENCIO_MS`** se lo lleva a ambar igual. **Las dos vias desembocan en la misma puerta**,
  `semaforo_iniciarFallo()`.
  ```
  grep -n "tUltimoComando\|semaforo_iniciarFallo" Esclavo/src/main.cpp
  ```

**Es el molde correcto y conviene decir por que:** el camino nuevo **no retira** el viejo, lo
adelanta. Un `CMD_GO_AMBAR` perdido degrada al comportamiento de ayer, no al silencio.

### 2 · N-135 — un `enum` de un solo valor cerro la puerta de N-133 el mismo dia

**Va aqui y no en la lista de arriba porque es el hallazgo de metodo, no un cambio de
comportamiento.** Al retirar las tres fases del asistente (N-42) quedo esto:

```
   enum FaseAuto { CORRIENDO };
   static FaseAuto fase;
   bool modoAutomatico_enMarcha() { return fase == CORRIENDO; }
```

Con un solo enumerador la comparacion es **cierta siempre**, y el compilador lo demuestra:
`movs r0, #1` / `bx lr`. **La variable ni se reserva.** El bloque retirado y esta cuenta siguen
escritos en la cabecera del fichero — `grep -n "N-135" Maestro/src/modo_automatico.cpp`.

**Lo que costo, que es la forma que hay que reconocer:** de `enMarcha()` cuelgan las dos guardas de
`SET_TIEMPOS`, asi que el equipo contestaba `$ERR,CMD:SET_TIEMPOS,DESC:EN_MARCHA_PARE_EL_MODO` **a
todo y para siempre**. Y como `modoAutomatico_fijarTiempos()` es el **unico** llamador de
`respaldo_guardarTiemposCiclo()`, **N-133 se quedo con camino de LECTURA y sin camino de
ESCRITURA**: los tiempos no se podian guardar nunca.

> 🔴 **UN ARREGLO CERRO LA PUERTA DEL OTRO EL MISMO DIA, Y NINGUN INSTRUMENTO LO VIO.** Lo encontro
> un agente que fue a comprobar si el paso de banco era ejecutable, y lo encontro **compilando**, no
> leyendo.
>
> **Y el comentario que sostenia el verde lo escribio quien hizo el cambio, en el mismo commit:**
> decia que el `enum` sobrevivia porque *"se lee mejor preguntando por la fase que por una bandera
> suelta"*. Es una **afirmacion sobre el codigo sin comprobar** — exactamente lo que la seccion 2.2
> de este documento ya sabe que hay que medir. Hoy es
> `bool modoAutomatico_enMarcha() { return modoActual_get() == MODO_AUTOMATICO; }`
> —`grep -n "modoAutomatico_enMarcha" Maestro/src/modo_automatico.cpp`—, y `ModoSistema` tiene
> siete valores.

~~**Y queda uno vivo del mismo patron, medido en esta misma pasada:** `Maestro/src/modo_inteligente.cpp:14`
declara `enum FaseInt { INT_CORRIENDO };` y **se compara** en `:40` y `:62`. **No se toca desde este
documento** —no se ha analizado que cuelga de esa comparacion—, pero **es el mismo constructo** y va
al Anexo, punto 12.~~

→ 🔴 **AFIRMACION FALSA desde el 05/09, y se corrige aqui: ese `enum` YA NO EXISTE.** Se retiro en
la pasada de N-135 del dia siguiente. `grep -n "FaseInt" Maestro/src/modo_inteligente.cpp` devuelve
**dos lineas, y las dos son COMENTARIO** —el bloque *"N-135 OTRA VEZ, EN EL FICHERO DE AL LADO
(05/09). Aqui habia: enum FaseInt { INT_CORRIENDO }; ..."*—. No queda declaracion, ni `switch`, ni
comparacion.

> ⚠️ **Y lo que ese comentario deja escrito vale mas que el enum retirado, porque es un hueco del
> instrumento:** el `switch` era **inerte** —no colgaba ninguna guarda de el, al contrario que en
> `modo_automatico.cpp`, donde costo `SET_TIEMPOS` entero—, **y lo encontro una revision externa, no
> el pack que existe para esto**: `maestro_10` censaba enums de un solo valor *"que ademas se
> COMPARAN"* y **solo miraba `==`**. Un `case` tambien es una comparacion. Es `CLAUDE.md`
> §4.quinquies literal: **el instrumento decidio no mirar una frontera y no lo llevaba escrito.**
>
> 🔴 **Y AQUI SALE UN HALLAZGO NUEVO DEL 05/09, QUE NO ES DE ESTE DOCUMENTO PERO SALE DE
> COMPROBARLO.** El comentario que `modo_inteligente.cpp` deja escrito dice *«El pack se afila en el
> mismo commit; si no, esto vuelve»*. **Medido: no se afilo.** El censo de
> `maestro_10_coordinador_alcanzable.py` sigue siendo, letra por letra,
> `if _re.search(r"==\s*%s|%s\s*==" % (_vals[0], _vals[0]), _t):` — **solo `==`, ningun `case`** —
> y es el unico pack del banco que censa esta forma
> (`grep -ln "len(_vals) != 1" Simulaciones/banco/packs/*.py` da **un fichero**). O sea que **un
> `enum` de un solo valor usado en un `switch` volveria a entrar sin que nada lo delate**, que es
> exactamente lo que la frase prometia impedir. Es `CLAUDE.md` §2.ter: **una frase que sostiene un
> verde y que no comprueba nadie.** **No se toca desde este documento** —`01_Firmware/` lo lleva
> otro trabajo—; **el punto 12 del Anexo cambia de sujeto pero NO se cierra.**

### 3 · La barrera de la app cambia de pregunta

**Es decision del responsable y no una preferencia de interfaz.** Para las ordenes que **abren
paso** la app ya **no pide el PIN**: pregunta si el operario **ha mirado el tramo**. El dialogo es
`#via-modal` en `index.html`, con sus dos botones *«Todavia no»* / *«He mirado: el tramo esta
libre»*, y la puerta en el JS es `confirmarVia()`.

```
grep -n 'id="via-modal"' App_Semaforo/www/index.html
grep -n "function confirmarVia\|confirmarVia(" App_Semaforo/www/app.js
```

**El porque, en una linea:** el equipo no sabe si quedan vehiculos en el tramo y el operario si. **Un
PIN demuestra quien eres; no demuestra que hayas mirado.**

**Las tres propiedades que lo hacen una barrera y no un adorno, MEDIDAS** —y la lista de a que
ordenes se aplica **sale de la tabla `VIA_MANIOBRA`, no de esta prosa**:

| | |
|---|---|
| **Solo en lo que ABRE paso** | son **TRES** ordenes y estan en `VIA_MANIOBRA`: `MANUAL:CAMBIAR_TURNO`, `SET_MODO:AUTO` y **`SET_MODO:AMBAR`**. Poner **rojo total** (`FORZAR_ROJO`) y **volver al menu** (`SET_MODO:MENU`) **no preguntan nada** — no estan en esa tabla. Preguntar para PARAR ensena a decir que si sin leer |
| **Se pregunta aunque el PIN este puesto** | no son dos llaves de la misma puerta: el manejador de `btnOpAuto` y el de `btnOpStep` llaman a `confirmarVia()` **sin mirar `state.pinVerificado`**. La lista `SIN_PIN` es otra cosa distinta (`grep -n "const SIN_PIN" App_Semaforo/www/app.js`) |
| **El vale caduca a los 30 s Y al cambiar la fase** | `VIA_VIGENCIA_MS` y `viaConfirmadaVigente()` — `grep -n -e VIA_VIGENCIA_MS -e "function viaConfirmadaVigente" App_Semaforo/www/app.js`. El tramo que se miro ya no es el que se va a abrir |

> ✏️ **CORREGIDO EL 05/09, y no es un matiz de redaccion: ~~«poner ambar no pregunta nada»~~ es
> FALSO para el MODO ambar.** `SET_MODO:AMBAR` **si pasa por `confirmarVia()`**
> (`grep -n "confirmarVia('SET_MODO:AMBAR'" App_Semaforo/www/app.js`), y la app lleva escrito el
> porque, que es vial y no de interfaz: *«los DOS postes quedan en intermitente a la vez, asi que se
> podra entrar al corredor por las dos puntas. No es un rojo y no para a nadie»*. En un carril unico
> eso son dos vehiculos de frente — **abre paso**, y por eso pregunta.
>
> **Lo que si es cierto, y es la distincion que hay que conservar:** el **latch de emergencia**
> `AMBAR_EMERGENCIA` —que es otro comando, no este— **no pregunta** y ademas va **sin PIN**, a
> proposito. Confundir los dos «ambar» es lo que hacia falsa la frase de arriba.
>
> ⚠️ **Y un detalle operativo que no estaba escrito en ningun sitio:** `SET_MODO:AMBAR` es **la
> unica orden de la botonera que pasa por LAS DOS barreras** — primero `pedirPin()`, y **despues**
> `confirmarVia()`. Las otras dos que abren paso llevan **solo** el vale de via. En el poste eso se
> nota: al operario se le piden cuatro digitos **y ademas** una respuesta sobre el tramo.
> `grep -n "btnOpAmber.addEventListener" -A 6 App_Semaforo/www/app.js` lo ensena en seis lineas.

**Y `AB-9` se cierra:** el PIN caduca a los **60 s** de irse la app al fondo (`PIN_GRACIA_FONDO_MS`)
y a los **5 min** sin mandar ordenes (`PIN_INACTIVIDAD_MS`), por `caducarPin()`.

```
grep -n "PIN_GRACIA_FONDO_MS\|PIN_INACTIVIDAD_MS\|function caducarPin" App_Semaforo/www/app.js
```

> 🔴 **PERO SU MITAD MAS IMPORTANTE ES `SIN VERIFICAR`, Y ESO NO SE PUEDE PINTAR DE VERDE.** Los dos
> caminos cuelgan de sucesos del navegador —`visibilitychange`, `pagehide` y `pageshow`, los tres
> registrados junto a `marcarFondo()` / `volverDelFondo()`—. **No hay un solo `pause` ni `resume` de
> Cordova**: `grep -cE "'pause'|'resume'|\bblur\b" App_Semaforo/www/app.js` da **0**, verificado el
> 05/09.
>
> **El escenario que esta barrera existe para cubrir es el telefono guardado en el bolsillo con la
> pantalla apagada, y ese es exactamente el que nadie ha ejercido.** Es `CLAUDE.md` §2.ter en
> limpio: **declarado, no ejercido**, y en el medio donde tiene que valer. **Va al Anexo, punto 13.**

### 4 · Lo que este bloque NO cierra

- **N-42 sigue contando como abierta.** El comentario del fuente
  —`grep -n "confirmado en banco" Maestro/src/modo_automatico.cpp`, sigue ahi el 05/09— dice
  *"EL DEFECTO QUE ESTO CIERRA, medido y confirmado en banco el 04/09"*. **Eso es ESCRITO, no
  MEDIDO**, y **contradice** lo que sostiene el resto de este documento y el informe de banco: el
  equipo **nunca llego a operar** por falta de app (N-122), y por eso la regresion *"no se confirmo
  ni se descarto"*. **Las dos frases no pueden ser ciertas a la vez, y este documento no elige: se
  publica la discrepancia y la decide quien ejecuto la sesion.** Lo que si esta MEDIDO es el
  **arreglo**, y es coherente con el sintoma.
- **Ninguno de los seis se ha ejercido en tarjeta.** N-134 es justo el que se veia mal en banco.
- **El error `FORMATO_INVALIDO` del Courier RTC SIGUE SIN DIAGNOSTICAR.** Que la app ahora **traduzca**
  los rechazos a lenguaje de obra hace legible el sintoma y **no dice nada de la causa**. Aqui no se
  propone ninguna.

### 5 · EL ACTA QUE ESTE DOCUMENTO CITA YA NO DICE LO QUE DICE EL DOCUMENTO

🛑 **Y esto invalida el parrafo de flash de la revision de la tarde.** Aquel bloque cita
`evidencia/2026-09-04_compuerta.txt` con **HEAD `624eb37`**. **Ese acta ya no existe: el fichero se
reescribio el mismo dia con una corrida posterior.** Lo que hay hoy bajo ese nombre:

```
   HEAD    : 6d075a5   rama: main-nuevo
   Arbol   : CON CAMBIOS SIN COMMITEAR
   banco por packs       FALLA      981/998  |  packs: 67 PASS, 2 FALLA, 0 ABORTADO
   simulador puente ESP32 ABORTADO  IndexError: list index out of range
   RESUMEN : 18 PASS | 1 FALLA | 1 ABORTADO
```

**Tres cosas que hay que leer juntas y ninguna es la cifra:**

1. 🔴 **LA COMPUERTA NO ESTA EN VERDE.** Un `981/998` dice que **17 comprobaciones no cumplen**, y
   un `ABORTADO` **no dice nada del firmware** — no es un aprobado.
2. 🔴 **El acta lo mide sobre un arbol sucio**, y lo declara ella misma: *"estas cifras NO
   corresponden exactamente a `6d075a5`"*.
3. 🔴 **El mecanismo, que es lo reutilizable: un acta con la FECHA en el nombre se sobrescribe sin
   que nada avise.** Dos corridas del mismo dia son dos actas distintas con el mismo nombre, y un
   documento que la cita **envejece en silencio**. **Es el hash que caduca solo, aplicado al
   fichero entero.** Los documentos que copiaban de aquella corrida —este y el `8_Procedimiento_...`—
   quedaron publicando `974/974` y `20 PASS · 0 FALLA · 0 ABORTADO` **de un acta que ya no existe**.

**Por eso aqui no se publica ninguna cifra nueva de flash ni de banco.** La que valga sale de correr
la compuerta sobre el arbol de hoy, **con el arbol limpio**, y el mismo criterio de la revision de la
tarde sigue en pie: **se publica la discrepancia, no un numero elegido.**

> 🔵 **Y EL 05/09 ESTE MISMO APARTADO SE DEMOSTRO A SI MISMO, QUE ES LA PARTE QUE VALE.** El bloque de
> arriba cita lo que *"hay hoy bajo ese nombre"* — `981/998`, `FALLA`, un `ABORTADO`—. **Eso ya no es
> lo que hay:** el fichero `evidencia/2026-09-04_compuerta.txt` **se ha vuelto a sobrescribir**, y hoy
> trae `20 PASS | 0 FALLA | 0 ABORTADO` sobre `1025/1025`.
>
> **No se corrige la cita: se deja, con esta nota encima.** Es la prueba de que el mecanismo
> denunciado —**un acta con la fecha en el nombre se sobrescribe sin que nada avise**— vuelve a
> ocurrir en cuestion de horas, y de que **cualquier cifra de acta copiada a un documento envejece en
> silencio, incluida la de este recuadro**. La regla no cambia: **la cifra que vale es la de correr
> la compuerta ahora**, no la que este documento —ni ningun otro— tenga escrita.
>
> 🔴 **Y lo que ese `20/20` NO dice, que es lo unico que importa esta noche: los cuatro defectos de la
> revision del 04-05/09 pasaron por delante de esas mismas 20 comprobaciones sin despeinarlas.** Los
> encontro **una cinta de tramas** y un operario delante del equipo. **Verde no es entregable.**

---

## 🟢 REVISION DEL 04-05/09/2026, LA NOCHE DEL BANCO — CUATRO DEFECTOS DE CALLE, UN CAMPO NUEVO, UN HALLAZGO SIN CAUSA Y UNA DECISION APLAZADA

**Estos SI salen del banco.** Y tres de los cuatro defectos salen de **una cinta de tramas** grabada
con el telefono conectado al Maestro —no de una revision de fuente—, que es la primera vez que este
proyecto encuentra defectos por ese camino. **La cinta es el instrumento; el fuente solo dijo por
que.**

| # | que cambia | donde se comprueba | nivel |
|---|---|---|---|
| **N-142** | **El Esclavo AVISA por radio de su ambar de emergencia.** `CMD_AMBAR_ESCLAVO` (`0x14`). Antes el Maestro no se enteraba y podia seguir dando VERDE **hasta 3 minutos** con el otro lado en ambar | `#define CMD_AMBAR_ESCLAVO` en los dos `protocolo.h` · el `protocolo_enviarPaquete(CMD_AMBAR_ESCLAVO)` del Esclavo · las **dos** ramas que lo reciben en `Maestro/src/coordinador.cpp` · `grep -rn "CMD_AMBAR_ESCLAVO" Maestro Esclavo` | **MEDIDO** en fuente · 🔴 **SIN EJERCER en tarjeta** |
| **N-146** | **`SET_MODO:AMBAR` contestaba `RESULT:OK` y no encendia nada.** Ahora re-arma y contesta **`REARMADO`**, que es distinto de `OK` | `Maestro/src/bluetooth.cpp:487-516` | **MEDIDO** — la cinta y el fuente |
| **N-147** | **En Modo Manual el equipo hacia un ciclo que nadie pidio.** Entraba por la puerta del Automatico | `Maestro/src/modo_manual.cpp:57-81` · `Maestro/src/bluetooth.cpp:475-479` · `Maestro/src/coordinador.cpp:587-617` | **MEDIDO** |
| **N-149** | **Campo `ESC:` en el `$STATUS` del Maestro, con cuatro valores —`ROJO`, `VERDE`, `AMBAR` y `?`—.** Lo que el Maestro sabe del Esclavo, en la trama | `coordinador_estadoEsclavo()` —declarada en `Maestro/include/coordinador.h`, definida en `coordinador.cpp` y consumida por el `snprintf` del `$STATUS`— · `grep -rn "coordinador_estadoEsclavo" Maestro` · el campo en la trama: `grep -n ",ESC:%s" Maestro/src/bluetooth.cpp` | **MEDIDO** |
| **N-145** | **La hora sale del `DS3231` del ESP32.** El puente rellena el hueco `HORA:--:--:--` al pasar la trama y recalcula el checksum | `ESP32_Expansion/src/puente.cpp:198-245`, `:333` | **MEDIDO** en fuente · 🛑 **SIN UN SOLO `DS3231` REAL** — ver el aviso |
| — | 🔴 **HALLAZGO DE CINTA, y aqui NO se le escribe causa:** `BAT:--` en **todas** las tramas | ver el bloque 6 | **MEDIDO** el sintoma |
| — | 🔴 **APLAZADA por el responsable a despues del banco: la MATRICULACION por ID de Bluetooth.** §3.8 | — | decision abierta |

---

### 1 · N-142 — el Esclavo avisa, y LOS DOS VETOS SE QUEDAN

**El defecto, con su ventana medida:** el ambar de emergencia pedido desde el telefono del Esclavo
enganchaba un cerrojo —esa punta deja de obedecer **y de ACUSAR**— y **ademas seguia contestando
`PONG`**, asi que **el enlace le parecia perfecto al Maestro**. Si el Maestro estaba en VERDE, seguia
dandolo **el resto de la fase —hasta 3 minutos con los tiempos de hoy—** con el otro lado en ambar, y
**los dos sentidos podian entrar al carril**. El Maestro solo se enteraba al agotar reintentos en el
cambio siguiente: tarde, y por el camino del fallo.

**El arreglo:** `CMD_AMBAR_ESCLAVO` (`0x14`) sale al armar el latch, **sin esperar acuse y sin
reintento** —igual que `CMD_GO_AMBAR`; el operario esta esperando delante—. El coordinador lo anota y
`main.cpp` lo **consume al leerlo** —es un aviso, no un estado— para entrar en `MODO_AMBAR`. Si ya
estamos en ambar **no se reentra**, y eso no es una optimizacion: `modo_ambar_setup()` manda un
todo-rojo y vuelve a ordenar el ambar, asi que un aviso repetido reiniciaria la secuencia cada vez.

> 🔴 **LA MITAD QUE COSTO MEDIR, Y ES LA QUE HAY QUE NO PERDER: SE IBA A QUITAR EL VETO DEL AMBAR DE
> LA APP, Y EL BANCO LO PARO DOS VECES.**
>
> | version | por que se cayo |
> |---|---|
> | quitar el veto de las tres guardas del Esclavo | `esclavo_07`: *"el ambar de la app dura hasta el siguiente latido del Maestro —unos 3 s— y el operario ve el equipo obedecer y volverse atras solo"* |
> | dejarlo solo en `CMD_GO_GREEN` y abrirlo en `CMD_GO_RED` | con el rojo entrando, el ambar que pidio el operario **se convierte en rojo a los 3 s**. Mas seguro, si; **no es lo que pidio**, y lo ve deshacerse delante |
>
> **Y AL MEDIRLO APARECIO QUE EL VETO NO ERA LA CAUSA DEL BLOQUEO.** La causa es que esa punta **no
> ACUSA**: el Maestro agota reintentos a ciegas, cae a `C_FALLO` y desde ahi rechaza todo. El
> silencio es **deliberado y correcto** —acusar un rojo que no se ha encendido dejaria al Maestro
> dando verde convencido de que aqui hay rojo— pero **obligaba al Maestro a ADIVINAR**. Con el aviso
> ya no adivina: se va a `MODO_AMBAR`, deja de ciclar y **deja de preguntar**, asi que no hay
> reintentos, no hay `C_FALLO` y no hay bloqueo.
>
> **LOS DOS VETOS SE QUEDAN ENTEROS** —el del mando (`mando_ambarLocal()`) y el de la app
> (`bluetooth_ambarEmergencia()`)—. Lo que desaparece **no es el cerrojo: es la ceguera del otro
> extremo.** Es la regla del instrumento (`CLAUDE.md` §4) aplicada a un diseno: **la causa plausible
> y la causa medida no eran la misma, y arreglar la plausible habria quitado la barrera que protege a
> quien esta en la calzada.**

---

### 2 · N-146 — seis ordenes, seis `OK`, y el cruce quieto

**Lo destapo la cinta, y el numero es lo que la hace prueba:** entre las **21:10 y las 21:13** hay
**SEIS** `CMD:PIN:****:SET_MODO:AMBAR` seguidos, los seis con `"$ACK,CMD:SET_MODO:AMBAR,RESULT:OK"`,
y el `$STATUS` de despues diciendo **`MODO:AMBAR,ESTADO:ROJO` durante 47 tramas**. El operario pulso
seis veces porque el cruce no se movia, y el equipo le dijo que si las seis.

**La causa, medida:** entrar en el ambar es trabajo de `modo_ambar_setup()`, y `main.cpp` solo lo
llama **EN EL FLANCO** (`if (modo != modoAnterior)`). Con el modo **ya** en `MODO_AMBAR` no hay
flanco, asi que `modoActual_set()` no hacia nada.

**Y al par (`MODO_AMBAR`, luz en rojo) se llega por un camino normal, no por un fallo:**
`CMD:FORZAR_ROJO` llama a `coordinador_forzarRojoTotal()`, que cambia **la LUZ y no el MODO** —a
proposito: el rojo de emergencia entra **sin PIN** desde cualquier modo—. **Un ROJO TOTAL despues de
un ambar dejaba el boton de ambar muerto para siempre sin decirlo.**

**Se re-arma, y se contesta `RESULT:REARMADO`, distinto de `OK`** (`bluetooth.cpp:512`): son dos
cosas y el diario de ordenes las tiene que poder separar. Es la **barrera de salidas** de `CLAUDE.md`
§6: *un `$ACK` que no depende de lo que la llamada hizo es una mentira con formato de exito* — y aqui
la mentira tapaba **una salida de emergencia**.

> ⚠️ **Lo que esto le pide a la app y a quien lea el diario de ordenes:** `REARMADO` es un **exito**,
> no un rechazo. Una interfaz que solo distinga `RESULT:OK` de `$ERR` lo pintara como error o lo
> ignorara. El literal se localiza con `grep -n "RESULT:REARMADO" Maestro/src/bluetooth.cpp`.

---

### 3 · N-147 — en Manual, el equipo hacia un ciclo que nadie pidio

`modoManual_setup()` llamaba a `coordinador_iniciarModo()`, que es **LA ENTRADA DEL MODO
AUTOMATICO**: deja el coordinador en `C_INICIAL_ESPERA_ESTATICO`, o sea **con un verde ya
programado**. Dos mitades, las dos reportadas desde el banco:

1. **DAR PASO no hacia nada** durante el plazo —`coordinador_pedirCambio()` abre con
   `if (estadoC != C_IDLE) return;`—, y el operario veia `EN_TRANSICION_REINTENTE` con el cruce en
   rojo.
2. **Al vencer el plazo el cruce cambiaba SOLO**, sin que nadie pulsara.

> *"el boton dar paso maestro queda en rojo, pasan 15 seg y ... pasa a ambar intermitente"*.
> **LOS 15 SEGUNDOS SON LITERALES:** `tiempoDespejeMs = 15000` (`Maestro/src/coordinador.cpp:32`). Y
> ese "ambar" era la transicion **rojo -> AMBAR 4 s -> verde** que el propio Maestro arrancaba al
> vencer el plazo.

**Y UN TERCER DEFECTO QUE NADIE HABIA REPORTADO, salido al medir:** el `case QV_NINGUNO` de
`pedirCambio()` **reiniciaba `tRef`**. Con el despeje en 15 s, **quien pulse cada 10 s no ve el verde
NUNCA**, y cada pulsacion contesta `OK`. **Obedecer y no avanzar no deja rastro de averia** — es la
misma forma que N-146, en otro modo.

**Manual entra ahora por `coordinador_forzarRojoTotal()`** —`grep -n "coordinador_forzarRojoTotal"
Maestro/src/modo_manual.cpp Maestro/src/bluetooth.cpp`, y el mismo cambio en la rama
`strcmp(accion, "SET_MODO:MANUAL")`—: **mismo todo-rojo, misma luz, mismo `CMD_GO_RED`, mismo reset
de replay**, pero termina en `C_IDLE` y **sin plazo**. Y el despeje **ya cumplido no se vuelve a
cobrar** —la marca `N-147` dentro de `coordinador_pedirCambio()`:
`grep -n "N-147" Maestro/src/coordinador.cpp`—.

> 🔴 **SFTY-4 NO SE DEBILITA, y esto no es una frase de cortesia: es lo que hay que comprobar antes de
> aceptar el cambio.** Los `case QV_MASTER` y `QV_ESCLAVO` —los que van **de un VERDE a otro**— pasan
> por su rojo y por su `C_ESPERA_ESTATICO_*` **como siempre**. A `QV_NINGUNO` solo se llega con el
> cruce **ya en todo-rojo**, y en Manual ese rojo lleva puesto minutos mientras el operario mira:
> **lo que se deja de cobrar es un despeje ya pagado, no el despeje.**

---

### 4 · N-149 — lo que el Maestro sabe del Esclavo, en la trama

**Pedido por el responsable delante del equipo:** *"cuando me conecto al maestro no me aparecen los
estados del semaforo del esclavo... yo necesito que maestro me traiga los datos del esclavo"*. Y
sobre la alternativa que la app ofrecia —conectarse por Bluetooth al otro poste—: *"tendrias que
caminar 1000 metros hasta el otro lado"*.

```
   $STATUS,NODE:MAESTRO,SERIE:...,MODO:...,ESTADO:...,T:...,RF:...,RTT:...,BAT:--,HORA:...,ESC:ROJO
                                                                                          ^^^^^^^^
```

**LA FUENTE ES `quienVerde`, Y ESA ELECCION ES TODO EL DISENO.** Esa variable **no** se pone por
haber MANDADO una orden: se escribe **AL RECIBIR EL ACUSE** (`case C_ESPERANDO_ACK_GREEN`, dentro del
`if (llego && pkt.command == CMD_ACK_GREEN)`). **Se publica lo que la otra punta confirmo, no lo que
esta quiso.** Publicar la orden seria pintarle al operario un semaforo que quiza no existe, y este
repositorio ya lo pago dos veces: el `BAT:12.6` que era un literal (N-108) y el equipo declarandose
en hora con el reloj parado en ceros (N-144).

| valor | que significa |
|---|---|
| `ROJO` / `VERDE` | lo que el Esclavo **confirmo** por acuse |
| `AMBAR` | `modoActual_get() == MODO_AMBAR` — se mira **antes que nada**, porque en ese modo esta punta ordena el ambar y **se calla a proposito**, asi que `quienVerde` se queda congelado. Es tambien el estado al que N-142 lleva el cruce |
| **`?`** | 🔴 **`estadoC == C_FALLO`: EL ENLACE ESTA CAIDO Y ESTA PUNTA NO LO SABE.** **NO significa "sin medida"** |

> 🔴 **El `?` no es un hueco: es la respuesta correcta.** Y por eso **no** comparte marca con los
> campos que se marcan `--`. Con el enlace caido, *"no se de que color esta la otra punta"* es mas
> util que un color plausible. **La app tiene orden de escribirlo como tal, no de repintar el ultimo
> valor como si fuera de ahora** (`CLAUDE.md` §3.quinquies). El instrumento lo trata igual: en
> `simulador_app_bluetooth.py` el `?` va **en la lista de validos y NO en `SIN_DATO`**, y el modelo
> arranca en `?` porque **antes del primer acuse el Maestro no sabe**.

**SOLO VA EN EL MAESTRO, y la asimetria es deliberada.** El Esclavo **no tiene de donde sacarlo** —no
le pregunta al Maestro y no tiene por que—, asi que un campo simetrico seria **inventarse el dato**,
que es justo lo que este campo existe para no hacer. Un campo que solo pudiera valer `?` no informa
nunca.

> ⚠️ **Y esa asimetria rompio una propiedad que un pack vigilaba,** `documentos_03`: *"las dos puntas
> emiten los mismos campos"*. **Dejo de ser cierta A PROPOSITO**, y se sustituyo por las dos que
> protegia: **(a)** el Esclavo no puede emitir nada que el Maestro no emita, y **(b)** cada campo que
> solo dice el Maestro **tiene que leerse en la app CON RED** — o en el poste del Esclavo llega
> `undefined` y la pantalla lo pinta como si fuera un dato.

**El campo esta documentado en el `10_Manual_Modulo_Bluetooth_Telemetria.md`, con su CRC recalculado
por el propio pack.** Este documento **no lo copia**: lo referencia.

---

### 5 · N-145 — la hora sale del `DS3231` del ESP32, y el puente DEJA DE SER VERBATIM

**El defecto, medido en la cinta del 04/09:** las tramas salen **TODAS** con `HORA:--:--:--`. El
campo lo compone el **STM32**, que es el micro cuyo cristal `Y2` esta **confirmado muerto** (N-17), y
**el unico `DS3231` del equipo cuelga del ESP32**. El equipo tiene la hora en la mano y publica un
hueco **porque quien compone la trama no es quien tiene el reloj**.

**Lo que se midio ANTES de tocar nada, porque cambio el plan:** el ESP32 **ya** leia el `DS3231`
—`reloj_ds3231.cpp` entero, con barrera `OSF`, bit de 12/24 h y validacion por barrido— y **ya**
atendia `SET_RTC` contra **su** reloj. **Lo que faltaba era el camino de VUELTA.**

**El sello, y sus tres cotas** (`ESP32_Expansion/src/puente.cpp:198-245`):

| # | cota | por que |
|---|---|---|
| **1** | **SOLO EL HUECO.** Busca el literal `"HORA:--:--:--"`. Si el STM32 puso una hora —el dia que tenga cristal— esta funcion **no encuentra nada y no hace nada** | **el puente NO ARBITRA entre dos relojes.** El arreglo **se apaga solo** cuando deje de hacer falta, en vez de volverse una segunda fuente de verdad peleando con la primera |
| **2** | **LONGITUD NEUTRA.** `"--:--:--"` y `"HH:MM:SS"` miden **ocho**. Se sella **en sitio** | `largo` sigue valiendo y **el presupuesto de bytes de `esp32_07` no se mueve ni un byte.** No es casualidad: es la razon de sellar en sitio en vez de recomponer la trama |
| **3** | **NUNCA INVENTA.** Pasa por `reloj_leer()`, que lleva `reloj_enHora()` delante y **no tiene variante "damela igual"** | bus mudo, `OSF`, modo 12 h, escritura a medias o registros incoherentes -> **el hueco sale como esta**. Es lo que costo N-144: un `DS3231` sin pila entrega una fecha **perfectamente formada y falsa** |

**Y el checksum se recalcula, que no es opcional:** la app **si** valida el XOR-8 en la bajada, asi
que sellar sin recalcular no daria una hora mala — **daria el tablero congelado**, con el sintoma
*"el puente se comio la telemetria"* mandando a mirar el cable.

> 🛑 **EL AVISO QUE VA CON ESTE ARREGLO, Y ES LO QUE MAS IMPORTA DE TODO EL BLOQUE:**
>
> **SIN UN `DS3231` CONECTADO, LAS TRAMAS SEGUIRAN SALIENDO CON `--:--:--`.** Eso es **el arreglo
> callandose bien, NO el arreglo fallando** — es exactamente la cota 3 funcionando. Quien lo pruebe
> sin modulo **no puede concluir nada** de ver el hueco.
>
> | | estado |
> |---|---|
> | La direccion I²C **`0x68`** | 🔴 **SIN VERIFICAR sobre el modulo real.** Es la del datasheet, y lo dice el propio fuente: `ESP32_Expansion/include/contrato.h:185-188` |
> | El modulo `DS3231` | 🛑 **NO ESTA COMPRADO** — linea **`A6`** de `15_Lista_de_Compras_Hardware.md` |
> | Esta parte de N-145 | 🔴 **NO SE PUEDE DAR POR PROBADA.** Nada de esto ha tocado un `DS3231` real |
>
> **Es `CLAUDE.md` §2.ter en limpio: DECLARADO, NO EJERCIDO** — y la propiedad que falta ejercer es
> **de hardware**, que es justo la clase que ningun pack ve.

**ACOPLAMIENTO NUEVO QUE HAY QUE SABER:** el literal `"HORA:--:--:--"` del STM32 es ahora **carga
estructural**. Si alguien lo cambia o quita el campo `HORA:`, **el sello deja de encontrarlo y la
hora desaparece EN SILENCIO**. No queda al aire: el escenario del `simulador_puente_esp32.py` exige
que el Maestro **real** siga emitiendo ese literal y **falla** si deja de hacerlo.

**RESIDUAL, escrito en vez de disimulado:** desde el `$STATUS` solo, **la app no puede saber cual de
los dos relojes sello la hora**. No se le anadio marca —cuesta contrato y buffer donde hoy no
falla—, y **la cura de verdad es que el STM32 deje de publicar un campo de un reloj que no tiene**.

> 🔵 **Y en la app, el otro extremo del mismo camino:** al cerrar N-145 se midio que `state.hora`
> tenia **un escritor y CERO lectores** —era `CAM_UMBRAL_PIN` en JavaScript—, y que **dos parsers
> escriben dos claves distintas**: `data.hora` en minuscula en `js/nmea_parser.js` y `data.HORA` en
> mayuscula en `app.js`.
>
> 🟢 **CERRADO POR EL AGENTE DE LA APP EN `6282b2a` (05/09), y se re-midio aqui antes de escribirlo:**
> `state.hora` tiene ya lector —la funcion `pintarHoraEquipo()`— y la guarda del escritor pasa a
> `if (data.HORA !== undefined)`. **Este documento no lo cierra: lo constata.**
>
> ```
> grep -n "function pintarHoraEquipo"   App_Semaforo/www/app.js
> grep -n "data.HORA !== undefined"     App_Semaforo/www/app.js
> ```
>
> 🔴 **Y al re-medirlo el 05/09 aparece algo que NO es una cita mal numerada y hay que decirlo:
> las DOS COPIAS de `app.js` YA NO SON LA MISMA.** `App_Semaforo/app.js` y `App_Semaforo/www/app.js`
> tenian el **mismo `md5` esa misma manana** y hoy difieren —**5.197 lineas contra 5.055**—; lo
> mismo con `index.html`. Los dos ficheros llevan el arreglo (`data.HORA !== undefined`), asi que
> **no hay contradiccion de comportamiento**, pero **una cita a `app.js` ya no dice a cual de los
> dos se refiere**, y el que se empaqueta en la APK es el de `www/`. Se comprueba con
> `md5sum App_Semaforo/app.js App_Semaforo/www/app.js`. **No se toca desde este documento** —la app
> la lleva otro trabajo—: **queda anotado como abierto.**
>
> ⚠️ **Lo que sigue en pie y no lo cierra ese commit: las DOS claves distintas siguen existiendo.**
> los dos `case 'HORA': data.hora = v;` de `js/nmea_parser.js` —`grep -n "data.hora" App_Semaforo/www/js/nmea_parser.js` da **dos**— escriben `data.hora`; `app.js` lee `data.HORA`. Hoy no rompe nada
> porque el camino que usa `app.js` es el suyo, **pero son dos parsers del mismo protocolo con
> convenios distintos**, y eso es una trampa esperando a que alguien cambie de camino. **Es de la
> app y no se toca desde aqui.**
>
> 🔴 **Y en cualquier caso: SIN VERIFICAR con un telefono delante y sin un `DS3231` en el bus, lo
> unico que la app puede pintar hoy es el hueco.**

---

### 6 · 🔴 HALLAZGO DE CINTA: `BAT:--` EN TODAS LAS TRAMAS

**Lo medido, y nada mas:** en la cinta del 04/09 **ninguna trama trae una cifra de bateria**. El
campo sale `--` porque **el literal esta escrito asi en el `snprintf` de las dos puntas**
(`Maestro/src/bluetooth.cpp:929`, `Esclavo/src/bluetooth.cpp:791`), y eso fue **deliberado**: N-108
lo cambio de `12.6` a `--` porque **no hay un solo `analogRead()`** en `src/` ni en `include/` de
ninguna de las dos puntas —MEDIDO: `grep -rn analogRead` sobre las cuatro carpetas da **cero**; las
unicas coincidencias del arbol estan dentro de `.pio/` y del framework de Arduino—.

> 🛑 **AQUI NO SE ESCRIBE POR QUE NO SE HA MONTADO LA MEDIDA DE BATERIA, PORQUE NO SE HA MEDIDO**
> (`CLAUDE.md` §4). Lo que hay es: **el campo esta marcado como "sin dato" a proposito, y el equipo
> no tiene ni divisor de tension ni canal ADC declarado.** Si alguien necesita la bateria en campo,
> **eso es una linea de compras y un cambio de firmware, no un defecto que se arregla escribiendo un
> numero.** Y **un campo marcado `--` es lo correcto** mientras no se mida: `CLAUDE.md` §3.quinquies
> —*lo que sustituye a un dato que no se tiene no es una simulacion: es decirlo*—.

---

## 🔵 REVISION DEL 31/08/2026 — leer esto ANTES que el resto

**Este documento se escribio el 28/08 y describia una decision que el responsable REVISO el 31/08.
Nada se ha borrado: lo superado va tachado con su motivo, en su sitio.** Lo que cambia:

| # | el 28/08 decia | el 31/08 |
|---|---|---|
| **1** | se retiran los **cuatro** pulsadores y el **mando de 4 reles** entero | 🟢 **el mando SE CONSERVA en los canales `A` y `B`** (`MANDO_A`=`BOTON1`=`PB9`=`J16` p5 · `MANDO_B`=`BOTON2`=`PB13`=`J16` p8). Se retiran **solo** `BOTON3` (`PB14`, p10) y `BOTON4` (`PB15`, p12), que son los que las camaras necesitan y **los que el mando no usa**. §1.6 |
| **2** | `J16` p5 y p8 **vacios a proposito** como colchon | 🔴 **era la linea mas danina del documento: mandaba dejar sin cablear el mando.** `p5` y `p8` **van cableados**. §1.7 |
| **3** | colchon de **10,2 / 22,9 / 27,9 mm** entre los 12 V y las senales | 🔴 **REFUTADO: eso es distancia entre PADS.** Cobre a cobre son **1,405 / 1,408 / 4,269 / 1,359 mm** (`MAPEO_TARJETA_KICAD.md:576-588`). **El orden se INVIERTE: `p12` es el PEOR punto, no el mejor.** §1.7 y M4 |
| **4** | §3.3 abierta, cinco opciones sin elegir | ✅ **DECIDIDA: opcion 3, «dejar el mando de reles».** Coste cero. §3.3 |
| **5** | §2.3 *"desde Bluetooth no hay vuelta, no hay `SET_MODO:MENU`"* | 🟢 **REFUTADA (N-100): existe** — la rama `strcmp(accion, "SET_MODO:MENU")` de `Maestro/src/bluetooth.cpp`, y con ella `ALCANCE`, `INTELIGENTE`, `DEGRADADO`, `REINICIAR_RELOJ` y `DEMANDA`. **La Fase 1 esta hecha** (`d34cfe2`, N-78) |
| **6** | §2.5 *"`SET_RTC` puede contestar `RESULT:OK` en silencio"* | ✅ **CERRADO en N-80**: cinco ramas —`grep -n "CMD:SET_RTC," Maestro/src/bluetooth.cpp`— |
| **7** | §2.4 citaba ~~`main.cpp:401`, `:408`, `:526`~~, y el 31/08 se **renumeraron** a ~~`:406`, `:416`, `:540`~~ | 🔴 **05/09: LAS SEIS ESTAN CADUCADAS, y es el mejor argumento de por que este documento dejo de citar numeros.** `Maestro/src/main.cpp` tiene **316 lineas**: ninguno de los seis existe. Los tres consumidores se localizan con `grep -n "mando_ambarLocal" Esclavo/src/main.cpp` |
| **8** | el censo de comandos del Esclavo (§2.3) | ⚠️ **le faltaba `CMD:AMBAR_EMERGENCIA`** —`grep -n 'strcmp(cmd, "CMD:AMBAR_EMERGENCIA")' Esclavo/src/bluetooth.cpp`—, que entra **sin PIN** |
| **9** | — | 🔴 **NUEVO, N-106:** ese `AMBAR_EMERGENCIA` **no saca al Esclavo del Modo Degradado**, mientras el `B·B·B` del mando si. **Medido por lectura, NO ejecutado.** `ESTADO.md` §N-106 |
| **10** | — | 🟠 **NUEVO, N-105:** cuatro documentos mandan cablear camaras sobre pines que no son entradas de camara. **En curso por otro agente.** `ESTADO.md` §N-105 |

> 🛑 **Y lo que NO ha cambiado: nada de esto ha pasado banco.** Ni lo del 28/08 ni lo del 31/08. Lo
> marcado MEDIDO se midio **sobre ficheros** — el `.cpp`, el `.h`, el `.kicad_pcb` —, y una revision
> que corrige diez cosas leyendo mas ficheros sigue sin tocar una tarjeta. **La seccion A y la seccion
> C siguen valiendo enteras.**
>
> **Acta vigente al escribir esta revision:** la de fecha 31/08 en `evidencia/` — `15 PASS | 0 FALLA |
> 0 ABORTADO`. Sustituye a la del 28/08 citada en la cabecera de arriba, que se conserva porque es la
> que respalda el texto original.

---

## 0. Como se lee este documento

Tres niveles, y no se mezclan nunca. Es la misma escala que usa
`03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md` §0, extendida al firmware.

| marca | que significa |
|---|---|
| **MEDIDO** | se abrio el fichero y se leyo. Va con `fichero:linea` o con el nombre de la red del netlist. Se puede repetir |
| **ESCRITO** | lo afirma un documento, un comentario o un acta. Puede ser cierto; **no se ha comprobado en esta pasada** |
| **SIN VERIFICAR** | nadie lo ha comprobado, ni aqui ni en ningun sitio. Casi todo lo del cobre esta aqui |

> 🔴 **La linea que manda sobre todo lo demas: hoy no existe ni una sola fila «VERIFICADO EN LA
> PLACA» en todo el mapeo de la tarjeta.** Lo dice `MAPEO_TARJETA_KICAD.md` §0 y §9, y sigue siendo
> verdad. Todo lo que este documento llama MEDIDO se midio **sobre ficheros**: el `.cpp`, el `.h`,
> el `.kicad_sch` y el `.kicad_pcb`. Un fichero dice lo que alguien **dibujo o escribio**. Una placa
> dice lo que se **fabrico**, y lo que alguien reparo despues.
>
> ✅ **ACTUALIZADO EL 04/09: eso ya no es cierto de TODO el documento, y la diferencia importa.** La
> sesion de banco del 03-04/09 dejo las **primeras filas medidas sobre la placa fisica**: `J16`
> p5/p8/p10/p12 (paso 20), `J17` p2/p3 contra las patas 42 y 43 del `U1` (paso 5), `J14` (pasos 17 y
> 18), `J15` (pasos 15 y 16) y las masas del modulo (paso 23). **Esas se marcan `MEDIDO EN COBRE` y
> llevan el numero del multimetro al lado.** Todo lo demas sigue siendo `MEDIDO` sobre ficheros, y no
> se mezclan: la escala no gana un cuarto nivel, gana una **procedencia** que hay que escribir.
>
> **Y la leccion, que este documento se aplica a si mismo:** §2.2 estuvo semanas declarada como
> *"contradiccion irresoluble desde aqui"* y el cobre la resolvio en una tarde con un ohmimetro.
> **Lo que un documento no puede decidir no siempre es indecidible: a veces solo esta esperando a que
> alguien baje al banco.**

---

## 1. La arquitectura decidida en obra

Esto es lo acordado. No se discute aqui; se documenta y se le cuelgan sus consecuencias.

### 1.1 El reparto, en una frase

**El STM32 sigue siendo el controlador del semaforo. El ESP32 es un modulo de expansion colgado de
un puerto serie: aporta reloj y Bluetooth, y no manda sobre las luces.**

```
                       fuente propia 12 V (NO sale de la tarjeta)
                                    |
   +--------------------+     +-----v--------------------+
   |   STM32F103C8      |     |         ESP32            |
   |   (controlador)    |     |   (modulo de expansion)  |
   |                    |     |                          |
   |  8 luces  J3-J9,J11|     |  DS3231  GPIO21 SDA      |
   |  barrera  J15      |     |          GPIO22 SCL      |
   |  buzzer   J13      |     |          (pila propia)   |
   |  camaras  J16      |     |                          |
   |  LoRa     J12      |     |  Bluetooth (sustituye    |
   |           USART3   |     |   al modulo SPP)         |
   |                    |     |                          |
   |  USART1 remapeado  |     |  futuro: WiFi / GPS      |
   |  PB6 TX == J17 p3 <------ GPIO16 (RX2)              |
   |  PB7 RX == J17 p2 ------> GPIO17 (TX2)              |
   +---------|----------+     +-----------|--------------+
             |                            |
             +-------- masa comun --------+
                       9600 8N1
```

### 1.2 Que se queda en el STM32

Todo esta MEDIDO en `01_Firmware/Maestro/include/pines.h`.

| funcion | pin | bornera | linea |
|---|---|---|---|
| Rojo 1 / Amarillo 1 / Verde 1 | `PA0` `PA1` `PA2` | `J3` `J4` `J5` | `pines.h:5-7` |
| Rojo 2 / Amarillo 2 / Verde 2 | `PA3` `PA4` `PA5` | `J6` `J7` `J8` | `pines.h:10-12` |
| 🔴 Rojo peaton / Verde peaton | `PA6` `PA7` | `J11` `J9` | `pines.h:15-16` — **DECLARADOS Y MUERTOS** |
| Barrera (talanquera) | `PB2` | `J15` | `pines.h:31` |
| 🔴 Buzzer | `PB1` | `J13` | `pines.h:20` — **DECLARADO Y MUERTO** |
| Radio LoRa (`USART3`) | `PB10` TX · `PB11` RX · `PB12` DE/~RE | `J12` | `#define RS485_OUT_TX` / `RS485_OUT_RX` / `LORA_DE_RE` en `pines.h` — `grep -n -e RS485_OUT_ -e LORA_DE_RE Maestro/include/pines.h` |
| Camara de demanda | `PB0` | `J14` | `pines.h:46` |
| 🆕 Camaras `C` / `D` | `PB14` `PB15` | `J16` p10 / p12 | `#define CAM_C_PIN` / `CAM_D_PIN` en `pines.h`, ya rotulados *"camara de contacto seco"* — `grep -n "define CAM_._PIN" Maestro/include/pines.h` — ~~**NO cablear hasta `M3`**~~ → ✅ **M3 CERRADA el 03/09: se pueden cablear.** `0 V` en reposo y sin demandas fantasma (paso 21) |

> ## 🔴 TRES DE ESTAS SALIDAS NO EXISTEN MÁS QUE EN LA TABLA (medido el 02/09)
>
> **`ROJO_PEATON` (`PA6`), `VERDE_PEATON` (`PA7`) y el `BUZZER` (`PB1`) están declarados en
> `pines.h` y MUERTOS en las dos puntas**: `grep` de esos tres nombres sobre `Maestro/src` y
> `Esclavo/src` **no devuelve ni un `pinMode`, ni un `digitalWrite`, ni un `digitalRead`**. Sólo
> aparecen en la línea del `#define`.
>
> **Lo que eso significa en obra: si alguien cablea una cabeza peatonal a `J11`/`J9` o un zumbador a
> `J13`, no se enciende nunca y no hay mensaje de error.** El montaje parece bien hecho y el equipo
> parece sano. Es la avería más cara de diagnosticar, porque no hay síntoma que buscar.
>
> **No las venda ni las incluya en una entrega como funciones del equipo.** El hardware está en la
> placa —sus optos y sus MOSFET—; **lo que falta es el firmware**, y no está escrito.
>
> ⚠️ **Y por eso la barrera de salidas de `CLAUDE.md` §6 hay que leerla con cuidado:** dice ocho
> pines de luz y **`escribirPines()` mueve SEIS** —`ROJO1/2`, `AMARILLO1/2`, `VERDE1/2`—. La regla es
> **vacuamente cierta** para los dos peatonales: nadie los escribe porque nadie los escribe. La
> talanquera sí entró dentro de `escribirPines()` el 27/08 (`ESTADO.md`, fila `A2`).

> ## 🔴 AMPLIADO EL 05/09 CON UN CENSO DE COBRE — y son SEIS pines libres, no tres
>
> **El bloque de arriba sigue entero y no se toca: es cierto.** Lo que faltaba es la otra mitad, y
> cambia tres cifras que este documento publica como hechos. Todo lo de abajo sale del
> `.kicad_pcb` (2.158.421 B), del `.kicad_sch` y del desensamblado del `.elf`. **El censo completo,
> con los comandos pegados, está en `05_Funcional/2_Manual_Hardware_y_Pruebas.md` §11.**
>
> ### 1 · Los tres canales muertos están COMPLETOS en la placa
>
> No es que "haya optos y MOSFET": es que `J9`, `J11` y `J13` son **el mismo molde exacto** que la
> talanquera de `J15`, **que sí funcionó en banco el 04/09**. Pin del `U1` → `R` 220 Ω → opto
> `TLP127` → `R` 10 K a masa más `R` 220 Ω a la puerta → `IRLZ44N` de lado bajo → bornera, con
> `1N4148` de rueda libre. Diez cadenas iguales, `Q1`–`Q10` con `U6`–`U15`.
>
> **Encender uno cuesta 16 B de flash**, medidos desensamblando el `.elf`: un `pinMode` con pin
> constante y un `digitalWrite` ocupan 8 B cada uno en Thumb-2. ⚠️ **Ese 16 es un SUELO**: son las
> dos llamadas, y no incluye nada de lo que **decide** el valor.
>
> ### 2 · 🔴 EL BORNE NO ESTÁ A 0 V EN REPOSO: ESTÁ A ~12 V
>
> **Ningún documento de este repositorio lo decía, y decide si esos bornes se pueden enchufar a
> algo.** Nueve de los diez drenadores llevan un **pull-up de 1 kΩ más LED al riel de 12 V**
> (`R23`, `R28`, `R33`, `R38`, `R43`, `R48`, `R53`, `R58`, `R63`, `R73`), y está **en el cobre, no
> en el conector**: no se evita dejando un hilo sin poner. Con el MOSFET abierto el borne sube a
> **~12 V**, con **~10 mA** disponibles por la cuenta `(12 menos ~2 de LED) / 1 kΩ`.
>
> ✅ **Explica una medida de banco que llevaba desde el 04/09 anotada SIN CAUSA:** `J15` daba *«en
> rojo `0 V`, en ámbar `12 V`»*, que es exactamente este circuito con la sonda entre p1 y p2. *(Que
> la sonda estuviera ahí es DEDUCCIÓN a partir de `TALANQUERA_ABRIR = HIGH`, no una lectura del
> informe: `SIN VERIFICAR`.)*
>
> 🔴 **Consecuencia de vocabulario, y no es un matiz: un MOSFET a masa NO es un contacto seco**, y
> aquí ni siquiera es un colector abierto limpio. Este proyecto usa *«contacto seco»* con razón para
> las **entradas** de cámara; **las salidas no lo son**.
>
> 🔴 **Y la excepción, que es un hallazgo nuevo: `J8` (`VERDE2`, `PA5`) NO tiene ese pull-up.** `D21`
> —su LED— tiene el cátodo **sin conectar**, en el esquemático y en el cobre: la red se llama
> `unconnected-(D21-K-Pad1)`, no tiene ni una pista, y su gemelo `D23` sí llega al drenador. Con el
> MOSFET abierto, `J8` p2 **queda FLOTANDO**, no a 12 V. **`SIN VERIFICAR` si es defecto o decisión**
> — no hay una sola nota sobre ello en el repositorio, y se cierra con un multímetro entre `J8` p1 y
> p2 comparado contra `J7`.
>
> ### 3 · 🔴 «El opto aísla galvánicamente» es MEDIO CIERTO, y la mitad que falla importa
>
> Es la frase con la que `2_Manual_Hardware_y_Pruebas.md` concluía que *«la etapa de potencia no
> puede inyectar corriente al micro»*. **Se ha tachado allí con su motivo el 05/09.**
>
> Medido: **hay UNA sola red `GND` en toda la tarjeta**, con **103 pads** y **plano de cobre en las
> dos capas**. En ella están a la vez el **cátodo del LED de cada opto** y la **fuente de cada
> MOSFET**. El opto separa el **pin del micro** del nodo de puerta; **no crea una masa separada**.
> **Cualquier cosa colgada de esos bornes comparte la masa del controlador** — y su riel de 12 V, por
> p1.
>
> Lo que sigue en pie: no hay camino de corriente del drenador a la pata del `U1`. Ése es el mérito
> del diseño, y por eso `J15` sigue siendo *«bien diseñada»* frente a las cinco entradas desnudas de
> §2.1. Lo que deja de poderse decir es que lo colgado esté **aislado del equipo**.
>
> ### 4 · Los pines libres son SEIS: hay que sumar `PB3`, `PB4` y `PB5`
>
> `lcd.cpp` pasó los pines de la pantalla a `U8X8_PIN_NONE`, y la librería **se salta el `pinMode` y
> el `digitalWrite` cuando el pin es `NONE`** (`U8x8lib.cpp`, función `u8x8_gpio_and_delay_arduino()`:
> `if (u8x8->pins[i] != U8X8_PIN_NONE)` y `if (i != U8X8_PIN_NONE)`). **Están hoy en alta impedancia,
> con pista hasta `J17`**: `PB3` a p4 (red `/SCL`), `PB4` a p1 (`/CS`) y `PB5` a p5 (`/SI`). **Son los
> únicos GPIO libres del proyecto con bornera ya cableada.**
>
> **Que dos de ellos sean patas de JTAG no cuesta nada aquí**, y conviene decirlo bien: `pin_function()`
> llama a `pin_DisconnectDebug()`, que en `PinAF_STM32F1.h`, función `pinF1_DisconnectDebug()`, hace
> `__HAL_AFIO_REMAP_SWJ_NOJTAG()` para `PA_15`, `PB_3` y `PB_4` — *«JTAG-DP Disabled and SW-DP
> enabled»*, literal del fuente. **SWD se conserva.** ⚠️ **`PB5` NO es pin de JTAG**: no está en esa
> lista, y decir que los tres lo son es falso.
>
> ⚠️ **Lo que sí cuesta: `J17` es el conector donde vive el ESP32.** Lo que se cuelgue de p1, p4 o p5
> convive con el módulo. **Que eso no le moleste NO está medido: `SIN VERIFICAR`.**
>
> ### 5 · Y la decisión que esto abre, que NO se toma aquí
>
> Gastar uno de los tres canales de `J9`/`J11`/`J13` **cierra la puerta a una cabeza peatonal o a un
> zumbador en esta placa**: no hay más molde libre. **No existe ninguna decisión escrita que renuncie
> a ellos.** Va a **§3.10**, abierta y con dueño.

### 1.3 Que se lleva el ESP32

| funcion | como | estado |
|---|---|---|
| Reloj `DS3231` con pila propia | I2C: `GPIO21` = SDA, `GPIO22` = SCL. El modulo `ZS-042` trae sus pull-ups | **decidido**, sin construir |
| Bluetooth | sustituye al modulo SPP dedicado, que se retira | **decidido**, condicionado a §3.1 |
| WiFi / GPS | futuro | no decidido |

### 1.4 El enlace, pin a pin

**MEDIDO** en el firmware: el `USART1` ya esta remapeado a `PB6`/`PB7` y ya sale por `J17`.

```
01_Firmware/Maestro/src/bluetooth.cpp:25   static HardwareSerial SerialBT(PB7, PB6);
Esclavo/src/bluetooth.cpp   static HardwareSerial SerialBT(PB7, PB6);   <- grep -n "HardwareSerial SerialBT"
01_Firmware/Maestro/include/bluetooth.h:7  "PB6 TX, PB7 RX ... Sale por el conector J17, posiciones 3 y 2"
```

| ESP32 | direccion | `J17` | STM32 | pin del `U1` |
|---|---|---|---|---|
| `GPIO17` (TX2) | ---> | **p2** | `PB7` — **RX** del micro | 43 |
| `GPIO16` (RX2) | <--- | **p3** | `PB6` — **TX** del micro | 42 |
| `GND` | --- | p7 o p9 | `GND` | — |

`9600 8N1`. **Masa comun, obligatoria.** El mapa de `J17` esta MEDIDO en el netlist del
`.kicad_pcb`: `p1=/CS`, `p2=/RST`, `p3=/RS(A0)`, `p4=/SCL`, `p5=/SI`, `p6=/3.3V`, `p7=GND`,
`p8=/3.3V`, `p9=GND`, y `p10`-`p13` sin red.

> ⚠️ **El nombre del pin 3 sigue en disputa y ahora tiene mas dueno que antes.** La etiqueta de red
> del esquematico es `RS(A0)`; el firmware lo llama `LCD_PSB` y lo trata como `PSB`. Los dos nombres
> no pueden ser ciertos a la vez (`MAPEO_TARJETA_KICAD.md` §6.bis, `pines.h:77-84`). **Con la LCD
> retirada la duda deja de amenazar a la pantalla, pero no desaparece:** si esa pata fuera de
> verdad un `RS/A0` de un display, el hilo del ESP32 va a un sitio con otro nombre. Se cierra
> siguiendo el hilo del pin 3 hasta la pata rotulada, no leyendo mas codigo.

### 1.5 La alimentacion: por que el ESP32 NO cuelga de `J17`

**El ESP32 lleva fuente propia desde 12 V.** No se alimenta de los 3,3 V de `J17` p6/p8.

El motivo esta ESCRITO en `05_Funcional/15_Lista_de_Compras_Hardware.md:102-106` y en el
Manual 10: un ESP32 con radio da picos de corriente del orden de **500 mA**, y ese riel de 3,3 V
—el que sale del `U5` LM1117DT-3.3, `MAPEO_TARJETA_KICAD.md` §2— es **el mismo que alimenta al
STM32 que gobierna el semaforo**. Un reset del controlador por una caida de riel provocada por un
periferico de diagnostico es exactamente el reparto de riesgo que no se acepta: el accesorio no
puede tumbar al que manda.

> **SIN VERIFICAR:** la cifra de 500 mA es de datasheet y de lo escrito en el Manual 15, no medida
> sobre el modulo que llego a obra. **No hace falta medirla para decidir**: la decision es no
> compartir riel, y esa decision no se cae si el pico resulta ser 300 mA.

### 1.6 Que se retira

> 🔵 **ACTUALIZADO EL 31/08/2026 — DECISION DEL RESPONSABLE.** Esta tabla decia que se retiraban los
> cuatro pulsadores y el mando entero. **Ya no.** Lo tachado se conserva con su motivo, que es como se
> corrige en este repositorio.

| se retira | consecuencia inmediata |
|---|---|
| Pantalla LCD (las dos puntas) | toda la operacion de menu pasa por la app |
| ~~Los cuatro pulsadores (`PB9`, `PB13`, `PB14`, `PB15`)~~ → **solo `BOTON3` (`PB14`) y `BOTON4` (`PB15`)** | libera `J16` **p10 y p12**, que es lo que las camaras necesitan. ~~**rompe la unica salida de modo**~~ → **§2.3 REFUTADA: la salida por app ya existe** |
| ~~Mando de 4 reles~~ → 🟢 **SE CONSERVA en los canales A y B** | `MANDO_A` = `BOTON1` = `PB9` = `J16` p5 · `MANDO_B` = `BOTON2` = `PB13` = `J16` p8. **El veto de §2.4 se queda donde esta** |
| Modulo Bluetooth SPP dedicado (`HC-05`/`JDY-30`) | lo sustituye el ESP32 — ver §3.1 |

**MEDIDO el 31/08, y es el porque de conservar los DOS canales y no uno:**

```
   grep -n "mando_registrarPulso"  Maestro/src/botones.cpp   ->  MANDO_A <- BOTON1(PB9)
                                                                 MANDO_B <- BOTON2(PB13)
   grep -c "BOTON[1-4]"            Maestro/src/mando.cpp     ->  0   el mando NO usa C ni D
   grep -n "ACC_AMBAR"             Esclavo/src/mando.cpp     ->  B.B.B -> ACC_AMBAR, y
                                                                 ambarLocal = true  <- UNICO armador
   grep -c "mando_ambarLocal"      Esclavo/src/main.cpp      ->  3   los tres if negados que vetan
```

> ⚠️ **De aquel censo del 31/08 hay UNA linea que ya no se puede repetir, y se dice en vez de
> borrarla:** decia *"`botonAceptar()` = `BOTON3`(`PB14`) · `botonCancelar()` = `BOTON4`(`PB15`)"*.
> **Hoy las dos funciones devuelven `false` a secas** —`grep -n "bool botonAceptar\|bool
> botonCancelar" Maestro/src/botones.cpp`— y esos dos pines **son camaras**. Lo que sigue en pie es
> lo unico que aquel censo tenia que demostrar: **el mando no los tocaba entonces y no los toca
> ahora.**

El mando vive **entero** en `A` y `B`. Los pines que las camaras necesitan —`PB14`, `PB15`— son los
dos que el mando **no toca**: el propio `botones.cpp` lo deja escrito encima de los dos
`mando_registrarPulso()` (*"Solo hay A (Boton 1) y B (Boton 2), que son justo los dos que el mando
necesita. El 3 EJECUTABA y el 4 salia..."*) — `grep -n "mando_registrarPulso" Maestro/src/botones.cpp`. Asi que las camaras
entran **sin** que `ambarLocal` deje de armarse: `A.A.A`, `B.B.B` y `A.B.A.B` siguen funcionando.

> ⚠️ **Las lineas de arriba se midieron el 31/08 sobre el arbol de ese momento, y ESE MISMO DIA otro
> agente esta llevando esta decision al firmware** —`pines.h` renombra ya `BOTON3`/`BOTON4` a
> `CAM_C_PIN`/`CAM_D_PIN`—. Las de `mando.cpp` y `main.cpp` no las toca ese trabajo; las de
> `botones.cpp` y `pines.h` **se van a mover**. Cuando ese trabajo entre, esta lista se re-mide: es
> `CLAUDE.md` §5, y es el mismo motivo por el que §2.4 tenia tres lineas caducadas.
>
> **Nada de esto ha pasado banco.** Es lectura de fuente, como todo lo que este documento marca
> MEDIDO. Ver la seccion C.

### 1.7 Las camaras a `J16`

Los pines que libera la retirada de los pulsadores 3 y 4:

| `J16` | red | GPIO | uso nuevo |
|---|---|---|---|
| p1 | `/12V` | — | 🔴 **12 V crudos. Se tapa** — ver §2.1 y **N-120: es OBLIGATORIO, no una cautela de banco** |
| p2 | `GND` | — | masa |
| p5 | `/Boton1` | `PB9` | ~~**vacio a proposito** (colchon)~~ → 🟢 **`MANDO_A`. VA CABLEADO** (31/08) → 🔴 **y hoy no responde: `0,6 V` en reposo, N-118** |
| p8 | `/Boton2` | `PB13` | ~~**vacio a proposito** (colchon)~~ → 🟢 **`MANDO_B`. VA CABLEADO** (31/08) → 🔴 **idem N-118** |
| p10 | `/Boton3` | `PB14` | **`CAM_C_PIN`** — entrada de camara de DEMANDA. ✅ **cableada y verificada en banco el 03/09** (paso 21) |
| p12 | `/Boton4` | `PB15` | **`CAM_D_PIN`** — entrada de camara de DEMANDA. `0 V` en reposo, MEDIDO (paso 20) |

> ⚠️ **AQUI PONIA ~~«Camara 2»~~ y ~~«Camara 1»~~, Y ERA UNA AMBIGUEDAD DE VERDAD, NO DE ESTILO.** El
> firmware y el `9_Manual_Parametrizacion_Camara_IA.md` usan esos numeros **para otra cosa**:
> `modo_inteligente.cpp:91-92` llama **«Camara 1»** a la que entra por **`PB0` del Maestro** y
> **«Camara 3»** a la de `PB0` del **Esclavo**. Las de `J16` **no tienen numero: son `C` y `D`**. Con
> los numeros de esta tabla, *"mover la Camara 1 a `J16` p12"* (§3.5) se leia como una tautologia. **A
> partir de aqui este documento las nombra por su constante**, que es lo unico que no se puede
> confundir.

> 🔴 **Las dos filas tachadas eran las lineas mas daninas de este documento: mandaban dejar sin
> cablear justo el mando que la decision del 31/08 conserva.** Un `J16` montado segun la tabla
> anterior deja `MANDO_A` y `MANDO_B` al aire, y con `B` al aire `ambarLocal` no se arma nunca
> (§2.4). El colchon habria costado el veto de SFTY-21 sin que ningun test lo dijera.

**Y el colchon en si tampoco media lo que decia.** Esto es lo que estaba escrito, con el paso del
footprint (`Molex_KK-254_AE-6410-16A_1x16_P2.54mm_Vertical`, 16 pads, tanto en `J16` como en `J17`):

| de `p1` a | posiciones | ~~distancia~~ **distancia entre PADS** |
|---|---|---|
| `p5` (`PB9`) | 4 | ~~10,2 mm~~ |
| `p10` (`PB14`) | 9 | ~~22,9 mm~~ |
| `p12` (`PB15`) | 11 | ~~27,9 mm~~ |

> 🔴 **REFUTADO el 31/08. Esas tres cifras son la distancia entre PADS, y esa NO es la separacion
> real entre los 12 V y la senal.** Los 12 V son una **red**, no un pad: salen de `J16` p1 y recorren
> la placa hasta las diez borneras de potencia con pistas de hasta 1,0 mm. Medido cobre a cobre
> —pads, pistas y vias, respetando capas— en `03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md:576-588`:
>
> | red de 12 V contra | separacion minima real | donde |
> |---|---|---|
> | `/Boton1` (p5) | **1,405 mm** | pista `B.Cu` 1,0 mm <-> pad `J16`.5 |
> | `/Boton2` (p8) | **1,408 mm** | pista `B.Cu` 1,0 mm <-> pad `J16`.8 |
> | `/Boton3` (p10) | 4,269 mm | pista `B.Cu` 1,0 mm <-> pad `J16`.10 |
> | **`/Boton4` (p12)** | **1,359 mm** <- **el peor** | via <-> pista `F.Cu` |
>
> **El orden se INVIERTE.** El colchon daba `p12` por el punto mas alejado de los 12 V —27,9 mm, el
> mas seguro del conector— y por cobre es **el peor de los cuatro**. El margen real no es de 10 mm
> sino de **1,36 mm**, cobre de diseno, sin tolerancia de fabrica ni suciedad ni humedad de un
> armario en la calle.
>
> Es `CLAUDE.md` §4 aplicada a una magnitud: **se midio lo que era facil de contar —posiciones de un
> conector— y se publico como si fuera lo que importaba.** Un pad no es una red. La conclusion
> operativa no cambia de signo pero si de tamano: `p1` se tapa **igual**, ~~y las camaras siguen sin
> cablearse hasta M3 (§2.2)~~.
>
> 🟢 **CADUCADA LA SEGUNDA MITAD EL 03/09, Y SE TACHA EN VEZ DE BORRARSE PORQUE ES LA FRASE QUE
> RESUCITA UN BLOQUEO QUE YA NO EXISTE.** **M3 se hizo** (paso 20 de la Guia de banco) y **cerro**:
> `p10` mide `9,93 kOhm` a masa y `p12` `9,94 kOhm`, los dos a **`0 V` en reposo**, y **`p10` se
> cablo contra `p11` en el paso 21 sin una sola demanda fantasma**. La camara **se cablea**. Lo unico
> que sigue en pie de este recuadro es **`p1`: se tapa, y desde N-120 eso es OBLIGATORIO en cada
> equipo que se monte** —§2.1 y §3.6—, no una cautela de banco.

> 🛑 **LOS DOS AVISOS DE `J16` QUE NO PUEDEN VIVIR EN UNA NOTA AL PIE, PORQUE SE LEEN CON UN
> DESTORNILLADOR EN LA MANO:**
>
> 1. **`J16` p1 lleva 12 V CRUDOS** —sin opto, sin resistencia en serie y sin clamp, contra entradas
>    de 3,3 V que van **desnudas** a la pata del STM32—. **Taparlo es OBLIGATORIO en cada equipo que
>    se monte** (N-120). No es del banco: es del montaje.
> 2. 🔴 **`MANDO_A` y `MANDO_B` NO RESPONDEN.** Medido en banco: `p5` y `p8` en **`0,6 V`
>    permanentes** con `617bd00` dentro, y **no se pudo pulsar el mando** (N-118). El fuente ya esta
>    corregido en las dos puntas (`346ea5f`, `INPUT` pelado y activo en ALTO) y **no se ha cargado en
>    ninguna tarjeta**. **El gesto de prueba es cerrar `p5` contra `p4` y `p8` contra `p7` —los
>    3,3 V del pin contiguo—, NUNCA contra masa:** en todo `J16` hay **una sola masa** (`p2`).
>
---

#### 1.7.bis 🟢 EL CAMINO DE CAMARA, CENSADO ENTERO — **lo que falta es COBRE, no codigo**

> **Esta es la entrada del trabajo siguiente, asi que va medida y no resumida.** Censado contra el
> fuente el 05/09, funcion por funcion. Cada linea se puede repetir abriendo el fichero que se cita.

**LAS TRES ENTRADAS DE CAMARA POR PUNTA, y NO son intercambiables:**

| entrada | pin | conector | ayuda de la placa | quien la declara | quien la lee |
|---|---|---|---|---|---|
| **`CAM_DEMANDA_PIN`** | `PB0` | **`J14`** | 🟢 **`R64` 10 kOhm + `C25` 100 nF — antirrebote RC de 1 ms EN LA PLACA**, escrito encima del `#define` | Maestro: `pinMode(CAM_DEMANDA_PIN, INPUT)` en `botones_setup()` · Esclavo: el mismo `pinMode` en `setup()` de `main.cpp` | Maestro: `camara_leerPin(CAM_DEMANDA_PIN)` en `modoInteligente_loop()` · Esclavo: `digitalRead(CAM_DEMANDA_PIN) == HIGH` en `main.cpp` |
| **`CAM_C_PIN`** | `PB14` | **`J16` p10** | 🟠 **`R67` 10 kOhm a masa, MEDIDA en cobre: `9,93 kOhm`. SIN condensador** | `pinMode(CAM_C_PIN, INPUT)` en `botones_setup()`, **las dos puntas** | `camaras_actualizar()` — y su siembra `camaras_sembrar()`, **las dos puntas** |
| **`CAM_D_PIN`** | `PB15` | **`J16` p12** | 🟠 **`R68` 10 kOhm a masa, MEDIDA: `9,94 kOhm`. SIN condensador** | `pinMode(CAM_D_PIN, INPUT)` en `botones_setup()` | idem |

**Las seis casillas de las dos ultimas columnas salen de tres `grep`, y ninguno lleva numero:**

```
grep -n "pinMode(CAM_" Maestro/src/botones.cpp Esclavo/src/botones.cpp Esclavo/src/main.cpp
grep -n "camara_leerPin\|digitalRead(CAM_DEMANDA_PIN)" Maestro/src Esclavo/src -r
grep -n "camaras_actualizar\|camaras_sembrar" Maestro/src/botones.cpp Esclavo/src/botones.cpp
```

**Las tres son `INPUT` PELADO y ACTIVAS EN ALTO**, y el gesto es **cerrar el contacto seco contra los
3,3 V del propio conector** — nunca contra masa. La cuenta que lo demuestra esta entera en la
cabecera de `pines.h`, bajo el rotulo *"POR QUE ACTIVO EN ALTO, Y POR QUE ESO NO ES UNA
PREFERENCIA"* — `grep -n "POR QUE ACTIVO EN ALTO" Maestro/include/pines.h`.

**COMO CONVERGEN, que es lo que hay que saber antes de tocar nada:**

```
   J16 p10 (PB14) --+
                    +--> camaras_actualizar()  --FLANCO DE SUBIDA-->  demanda_solicitar()
   J16 p12 (PB15) --+     botones.cpp:144-152                          demanda.cpp:13
                                                                             |
                                                                    demanda_hayLocal()
                                                                             |
   J14 (PB0) ------------------------------------------------> camara_leerPin(CAM_DEMANDA_PIN)
                                                                             |
                                                       modo_inteligente.cpp:97:
                                             camara_leerPin(CAM_DEMANDA_PIN) || demanda_hayLocal()
```

**Las tres entran por la MISMA puerta**, y esa puerta lleva la ventana de silencio de **3 s**
(`SILENCIO_MS`, `demanda.cpp:8`) que impide que una cola de coches se convierta en una rafaga de
peticiones identicas. **La demanda pedida a mano por Bluetooth entra por ese mismo `OR`**, a
proposito: asi se le aplican los dos limites del ciclo en vez de saltarselos.

**Y las de `J16` se toman POR FLANCO, no por nivel** —el rele de la AcuSense cierra ~1 s por
deteccion; leer el nivel repetiria la peticion en cada vuelta del `loop` durante todo ese segundo—,
con su siembra de arranque (`camaras_sembrar()`, `botones.cpp:129-135`): **un contacto YA CERRADO al
encender no es una deteccion, es un estado**. Es N-26 aplicado a la camara.

> ✅ **CONCLUSION, Y ES LA QUE EL RESPONSABLE NECESITA PARA DECIDIR QUE TOCA AHORA: el firmware de las
> dos camaras de `J16` YA ESTA CONSTRUIDO Y EJERCIDO. Lo que falta es COBRE, no codigo.**
>
> - **Ejercido en banco:** paso 21 del 03/09 — `p10` cableada contra `p11`, **funciono y sin demandas
>   fantasma en reposo**.
> - **Ejercido por instrumento:** `camara_01_demanda` y `camara_02_j16` en el banco por packs.
> - **`M3` cerrada en cobre** (paso 20): las cuatro posiciones con su pull-down real de 10 kOhm.
>
> 🛑 **Lo que sigue siendo condicion de montaje, no de firmware:** tapar `J16` **p1** —12 V crudos,
> N-120— **antes** de enchufar nada, y **cargar el firmware nuevo ANTES de que nadie toque `J16`**
> (`CLAUDE.md` §9.bis: un commit no protege de un destornillador).

> ⚠️ **UN SOLO SITIO DONDE LAS TRES NO CONVERGEN, Y NO ES UN DEFECTO — SE ESCRIBE CON LA MEDIDA AL
> LADO PARA QUE NADIE VAYA A «ARREGLARLO»:** `modo_inteligente.cpp` calcula `presenciaActual`
> —`grep -n "presenciaActual" Maestro/src/modo_inteligente.cpp`— mirando **solo**
> `CAM_DEMANDA_PIN` y la demanda remota, **sin** `demanda_hayLocal()`. O sea que las dos de `J16`
> **no cuentan ahi**. **Medido: ese numero solo alimenta `lcd_dibujarInteligente()` — es el
> contador de presencia de la PANTALLA, y la pantalla se retira** (§1.6). No decide
> ninguna luz ni ninguna orden. **Si algun dia ese contador se publica en la telemetria, entonces si
> hay que meter `demanda_hayLocal()` — y no antes.**

> 🔴 **Lo que este censo NO dice, y no se da por dicho:** **ninguna camara AcuSense se ha conectado
> nunca a este equipo.** Lo que se cablo en el paso 21 fue **un puente de `p10` a `p11`**, no una
> camara. La salida de la AcuSense es configurable (NO/NC) y **cual de los dos estados significa
> demanda es una decision de parametrizacion que sigue SIN TOMAR** — `9_Manual_Parametrizacion_
> Camara_IA.md`. **SIN VERIFICAR con una camara real en las tres entradas.**

---

> 🔴 **Y la consecuencia que nadie debe deshacer por comodidad de montaje: con `MANDO_B` al aire,
> `mando_ambarLocal()` NO SE ARMA NUNCA.** De esa bandera cuelgan **tres vetos** en
> `Esclavo/src/main.cpp` (`:406`, `:416`, `:540`), todos de la forma `if (!mando_ambarLocal() &&
> !bluetooth_ambarEmergencia())`. Con la bandera muerta los tres `if` son **siempre verdaderos** y
> **el veto de SFTY-21 desaparece**: una orden de radio le quitaria el ambar a la punta donde un
> operario esta subido al gabinete. **Dejar `p8` sin cablear no deja el mando "inerte": deja el veto
> ABIERTO** — §2.4. Un `J16` montado sin `p5` y `p8` **no es un montaje incompleto, es un montaje
> distinto**, y ningun test lo dice.

---

## 2. Lo que esta arquitectura hereda — ocho hallazgos MEDIDOS

Ninguno de estos ocho es una opinion. Todos se pueden repetir abriendo el fichero que se cita.

### 2.1 🔴 `J16` p1 lleva 12 V crudos a cuatro posiciones del primer pin de GPIO

**MEDIDO** sobre el netlist del `.kicad_pcb` bueno
(`01_Firmware/Controladora_Semaforos/Controladora_Semaforos/Controladora_Semaforos.kicad_pcb`):

```
   J16  pad 1  -> red "/12V"
   J16  pad 5  -> red "/Boton1"   (PB9, U1 pin 46)
```

**No hay opto, no hay resistencia en serie y no hay clamp entre esa posicion y el resto del
conector.** `MAPEO_TARJETA_KICAD.md` §8.2 mide que el aislamiento galvanico de esta tarjeta vive
**solo** en las diez cadenas de potencia (opto `TLP127` + MOSFET) que van a las borneras `J3`-`J9`,
`J11`, `J13`, `J15`. `J16` no es una de ellas: es un conector de senal directo al `U1`.

Y `MAPEO_TARJETA_KICAD.md` §7 lo dice en una linea que conviene no perder: **`J16` es el unico
conector de senal de toda la tarjeta que trae 12 V.**

> **Accion:** tapar `p1` fisicamente antes de cablear nada en `J16` — funda termorretractil sobre
> el pin, o el pin retirado del cuerpo del conector volante. No basta con «no conectarlo»: el
> destornillador, la viruta y el hilo suelto no leen documentacion. Un contacto de `p1` a `p10` o
> `p12` mete 12 V en una pata del `U1` que espera 3,3.
>
> 🔴 **AMPLIADO EL 04/09 — N-120, y sube de precaucion a obligacion.** El banco confirmo en cobre lo
> que este apartado deducia del netlist, **y encontro que es peor de lo que decia**: no es que `J16`
> no tenga aislamiento, es que **ninguna de las 5 entradas de campo de la tarjeta lo tiene**,
> mientras ~~**las 9 salidas**~~ → **las DIEZ salidas** llevan `220 Ohm` en serie y opto `TLP127`. La
> proteccion de esta placa es **asimetrica**, y esta entera del lado por el que no entra nada.
>
> 🔴 **Dos correcciones del 05/09, medidas sobre el mismo fichero de cobre y tachadas con su motivo:**
> (a) **son DIEZ cadenas, no nueve** —`Q1`-`Q10` con `U6`-`U15`—, que es lo que este mismo documento
> ya decia bien tres parrafos mas arriba; (b) **"aislamiento" dice de mas**: el opto separa el PIN del
> micro del nodo de puerta, pero **hay UNA sola red `GND` en la tarjeta**, con 103 pads y plano en las
> dos capas, y en ella estan el catodo del LED del opto **y** la fuente del MOSFET. **Lo colgado de
> esas borneras comparte la masa del controlador.** Ver el bloque del censo en §1.2 y el detalle en
> `2_Manual_Hardware_y_Pruebas.md` §11.
>
> **Por eso `p1` deja de taparse «en banco» y pasa a taparse EN CADA EQUIPO QUE SE MONTE**, con el
> pin retirado del conector volante — que es como se hizo en el paso 4 y es lo que se documenta,
> porque no se deshace por accidente. **La decision de diseno que esto abre para V2 esta en §3.6**, y
> es del responsable.

### 2.2 ~~🔴 La polaridad de los pines de boton esta en contradiccion~~ → ✅ **CERRADA EN COBRE EL 03/09 (M3 / N-118): NO era una contradiccion**

> ✅ **CERRADA POR MEDIDA, y se conserva entera porque el analisis es el que motivo la medida.** Este
> apartado decia que los dos lados —netlist y firmware— *"no pueden ser ciertos a la vez"* y que
> **no se podia decidir desde aqui**. Era correcto, y la salida que proponia —**M3, con multimetro**—
> es exactamente la que se ejecuto.
>
> **MEDIDO EN COBRE el 03/09/2026, paso 20 de la Guia, conector vacio:**
>
> ```
>    J16 p5  (MANDO_A, PB9)    9,92 kOhm a masa   11,28 kOhm a 3,3 V    0,6 V con energia
>    J16 p8  (MANDO_B, PB13)   9,92 kOhm a masa   11,28 kOhm a 3,3 V    0,6 V con energia
>    J16 p10 (camara, PB14)    9,93 kOhm a masa   11,29 kOhm a 3,3 V    0   V con energia
>    J16 p12 (camara, PB15)    9,94 kOhm a masa   11,31 kOhm a 3,3 V    0   V con energia
> ```
>
> 🔴 **El netlist tenia razon, y la tiene en las CUATRO posiciones.** El pull-down de 10 kOhm es
> real, las cuatro posiciones son identicas —10K a masa mas `100 nF`— y las cuatro llevan `3,3 V` en
> la posicion de al lado (`J16` p4, p7, p9, p11). **El gesto que el conector pide es cerrar contra
> los `3,3 V`: entrada activa en ALTO, sin excepcion.**
>
> **Asi que no habia dos afirmaciones incompatibles sobre la placa: habia un fuente equivocado.**
> `botones.cpp` **estaba** invertido en `A` y `B`, en las dos puntas. La camara ya se corrigio en
> N-67 y por eso el camino de camara sale bien; el camino de mando **no se habia corregido**, y es
> N-67 otra vez — con la diferencia de que ahora esta **medido**, no deducido. 🟢 **Corregido el
> 04/09 en `346ea5f`, en las dos puntas** —`INPUT` pelado y `== HIGH`—; 🔴 **sin ejercer en
> tarjeta**.
>
> **Y la cuenta de `0,66 V` que este apartado reprodujo de N-67 se confirma en la punta del
> multimetro: `0,6 V`.** No es una coincidencia amable — es la unica parte de todo este documento que
> se puede comparar contra un numero real, y coincide.
>
> ⚠️ **Lo que la medida NO dice, y va escrito al lado:** no dice por que el menu se navegaba en las
> pruebas de banco de agosto. El parrafo de mas abajo levanta esa objecion —*"si esa cuenta
> describiera la placa fisica, los cuatro botones estarian en LOW permanente y el menu no se podria
> navegar"*— y la medida **no la contesta**. La pista que si esta escrita es la coincidencia con
> N-26, mas abajo: *"el Maestro aparecia solo en la pantalla de configuracion del Modo Manual"*, que
> **es el sintoma exacto de un pin en LOW al arrancar**, y que se cerro **sembrando el estado real
> del pin** — es decir, enmascarando el sintoma. Eso deja de ser *"una pista"* y pasa a ser la
> explicacion mas probable, pero **no esta comprobada**: se marca SIN VERIFICAR y no se convierte en
> causa.
>
> **Consecuencias, y son dos en direcciones opuestas — estan desarrolladas en la revision del
> 03-04/09 de la cabecera:** ✅ la camara **se cablea** (paso 21, sin demandas fantasma) · 🔴 el
> mando `A`/`B` **no se pudo pulsar en banco** con `617bd00` (N-118), y con el se cayo la salida
> fisica de ultimo recurso que §3.3 daba por construida. 🟢 **El fuente se corrigio el 04/09
> (`346ea5f`, las dos puntas); 🔴 la salida de ultimo recurso NO vuelve a estar construida hasta la
> carga verificada en tarjeta.**

**Lo que sigue debajo es el texto del 28/08, conservado, y HAY QUE LEERLO EN PASADO.** Su conclusion
—*"esto es una contradiccion y no un defecto"*— **ya no vale**: era un defecto, estaba en
`botones.cpp`, y **se corrigio el 04/09 en `346ea5f`**. Las citas de fuente que vienen a
continuacion describen el firmware de `617bd00` —el que fue al banco—, **no el de hoy**.

Es el bloqueante del cableado de camaras. **Los dos lados estan MEDIDOS y no pueden ser ciertos a
la vez.**

**Lo que dice la placa** (netlist del `.kicad_pcb`, y valor leido del `.kicad_sch`):

```
   R65 = 10K   pad1 -> GND      pad2 -> /Boton1   (PB9)
   R66 = 10K   pad1 -> GND      pad2 -> /Boton2   (PB13)
   R67 = 10K   pad1 -> GND      pad2 -> /Boton3   (PB14)
   R68 = 10K   pad1 -> GND      pad2 -> /Boton4   (PB15)

   J16  p4 = /3.3V   p5 = /Boton1
        p7 = /3.3V   p8 = /Boton2
        p9 = /3.3V   p10 = /Boton3
        p11 = /3.3V  p12 = /Boton4
```

Eso es **pull-DOWN de 10 kOhm a masa, con 3,3 V en la posicion de al lado**: el gesto previsto es
cerrar el contacto contra los 3,3 V del propio conector, o sea **entrada activa en ALTO**.

**Lo que decia el firmware — `617bd00`, el que fue al banco. ~~Lo que dice el firmware~~:**

```
   Maestro/src/botones.cpp   pinMode(BOTONn, INPUT_PULLUP);
   Maestro/src/botones.cpp   bool lecturaCruda = (digitalRead(b.pin) == LOW);
   Esclavo/src/botones.cpp   pinMode(BOTONn, INPUT_PULLUP);
   Esclavo/src/botones.cpp   bool lecturaCruda = (digitalRead(b.pin) == LOW);
   Esclavo/src/botones.cpp   "Entradas en INPUT_PULLUP y pulsador contra masa: pulsado = LOW"
```

*(Sin numeros de linea a proposito: esto describe `617bd00`, un arbol que ya no es el de hoy, y
un numero de aquel arbol no se puede verificar contra este. Lo que sigue verificandose es la
AUSENCIA del `pinMode`, no la de la palabra —los comentarios que explican la retirada SI la
nombran—: `grep -c "pinMode.*INPUT_PULLUP" Maestro/src/botones.cpp Esclavo/src/botones.cpp` da
**0 y 0**, 05/09.)*

O sea **pull-UP interno, activo en BAJO**. Los dos no podian ser ciertos.

> 🟢 **AL DIA EL 04/09 — este bloque describe `617bd00`, y el fuente ya no dice eso.** En `346ea5f`
> las cuatro lineas de arriba son:
>
> ```
>    grep -n "pinMode(BOTON[12], INPUT)"       Maestro/src/botones.cpp Esclavo/src/botones.cpp
>    grep -n "digitalRead(b.pin) == HIGH"      Maestro/src/botones.cpp Esclavo/src/botones.cpp
>    grep -c "pinMode.*INPUT_PULLUP"           Maestro/src/botones.cpp Esclavo/src/botones.cpp   -> 0 y 0
> ```
>
> **El fuente y el netlist ya coinciden.** 🔴 **Lo que falta es la tarjeta**: sin carga verificada
> esto es una lectura, no una prueba.

**Esto ya paso, con la misma resistencia y el mismo valor.** `R64` es tambien 10K a GND, sobre la
red `/Puerta` que llega a `J14` p1, con 3,3 V en `J14` p2. Es la entrada de camara, y la cuenta que
cierra `roadmap.md` N-67 (linea 552) es esta:

> el pull-up interno (~40 kOhm) contra el pull-down de 10 kOhm de la placa deja el pin en
> `3,3 x 10/50 = 0,66 V`, que el micro lee **LOW**. El firmware habria visto **demanda permanente
> desde el arranque, sin ninguna camara conectada**.

La camara se arreglo: `pinMode(CAM_DEMANDA_PIN, INPUT)` y deteccion contra `HIGH` — y **desde
N-97 el `pinMode` del Maestro vive en `botones_setup()`, no en `modoInteligente_setup()`**:
`grep -rn "CAM_DEMANDA_PIN" Maestro/src Esclavo/src`.
~~**Los cuatro pines de boton, con topologia identica en el netlist, siguen en `INPUT_PULLUP` y
`== LOW`.**~~ → 🟢 **CADUCADO el 04/09: ya no queda ninguno.** `PB14`/`PB15` pasaron a camara en
N-67/31-08, y `PB9`/`PB13` en `346ea5f`. **Los cuatro leen `INPUT` pelado y `== HIGH`**, que es lo
que pide el cobre. **Pendiente de ejercer en tarjeta.**

> **Lo que NO se puede concluir desde aqui, y por eso esto es una contradiccion y no un defecto:**
> si esa cuenta describiera la placa fisica, los cuatro botones estarian en LOW permanente y el menu
> no se podria navegar. Y **hay evidencia de banco de que el menu se navega** — todo el protocolo de
> la seccion 5, y N-53. Asi que **o el netlist no describe esta tarjeta, o el pull-up interno real es
> mas fuerte de lo que dice el datasheet, o hay una diferencia entre la placa dibujada y la
> soldada.** Se cierra con multimetro, no con mas lectura: medida **M3** de la seccion A.
>
> 🟡 **Y hay una coincidencia que hay que anotar sin darla por causa.** `botones.cpp:60-64` del
> Maestro documenta N-26, CONFIRMADO EN BANCO el 01/08/2026: *"el Maestro aparecia solo en la
> pantalla de configuracion del Modo Manual sin que nadie tocara nada"*. Ese es exactamente el
> sintoma de un pin en LOW al arrancar. **N-26 se cerro sembrando el estado real del pin**, que
> enmascara el sintoma sin decidir cual de las dos polaridades es la buena. Es una pista, **no una
> medida**, y este documento no la convierte en causa.

~~**Consecuencia operativa: mientras esto no se mida, no se cablea camara a `J16`.**~~ → ✅ **SE
MIDIO (M3, 03/09) y el bloqueo se levanta: la camara se cablea.** Cablear una camara con la
polaridad al reves da **demanda permanente** —un semaforo que pide paso solo— o **demanda que nunca
llega**; las dos son de calle, y por eso la medida iba antes que el cable. **El paso 21 la cablo
(`p10` contra `p11`, normalmente abierto) y no hubo demanda fantasma ni con el cable ni sin el.**

> 🔴 **Y la consecuencia se muda de sitio, no desaparece: el que esta con la polaridad al reves es el
> MANDO.** `A`/`B` no dan *"pulsacion permanente"* de forma visible —dan algo peor, **silencio**: el
> pin nace en `LOW`, el antirrebote lo da por consumido y ninguna pulsacion produce ya un flanco.
> **Una entrada de seguridad que no puede fallar ruidosamente es la que hay que vigilar.**

### 2.3 ~~🔴 `botonCancelar()` es la unica salida de todos los modos, y desde Bluetooth no hay vuelta~~ → 🟢 **REFUTADA EL 31/08 (N-100)**

> 🟢 **REFUTADA, y no se borra.** El censo de llamadores de `botonCancelar()` de mas abajo **sigue
> siendo correcto**: `botonCancelar()` es efectivamente la unica salida *fisica*. Lo que era falso —y
> se marca aqui, en la cabecera, para que nadie lo lea antes que la correccion— es la **segunda
> mitad**: *"desde Bluetooth no hay vuelta"* y *"no hay `SET_MODO:MENU`"*.
>
> **MEDIDO el 31/08 sobre `01_Firmware/Maestro/src/bluetooth.cpp`:**
>
> ```
>    :191   SET_MODO:MENU          <- EXISTE. Y entra SIN PIN: :169-170
>    :196   ... y en Degradado no salta al menu: pide la salida por el todo-rojo
>    :201   $ACK,CMD:SET_MODO:MENU,RESULT:SALIENDO_TODO_ROJO
>    :204   $ERR,CMD:SET_MODO:MENU,DESC:YA_VUELVE_AL_MENU
>    :212   SET_MODO:ALCANCE       :223  SET_MODO:INTELIGENTE
>    :234   SET_MODO:DEGRADADO     :245  $ERR motivado      :250  $ACK,RESULT:OK
>    :330   REINICIAR_RELOJ        :345  DEMANDA
> ```
>
> Los seis comandos que este documento pedia en su Anexo entraron en **`d34cfe2` (N-78)**. La
> consecuencia que la seccion anunciaba —*"cada modo se convierte en una puerta de un solo
> sentido"*— **ya no se sostiene**, y el orden de fases que colgaba de ella queda satisfecho, no
> derogado: la razon por la que la salida iba primero era buena, y por eso se construyo.
>
> **Y la decision del 31/08 la refuerza por el otro lado:** con el mando conservado en `A` y `B`,
> `A.A.A` sigue siendo una salida a Automatico **sin app y sin pantalla**. Hay dos vias, no una.

**Lo que sigue debajo se conserva como estaba escrito el 28/08.** Vale entero para el censo de
llamadores; su parrafo de consecuencia esta refutado por el recuadro de arriba.

**MEDIDO** por censo de llamadores de `botonCancelar()` en el Maestro:

| modo | quien sale | linea |
|---|---|---|
| Degradado | `botonCancelar()` | `modo_degradado.cpp` |
| Alcance | `botonCancelar()` | `modo_alcance.cpp:50` |
| Ambar | `botonCancelar()` | `modo_ambar.cpp` |
| Automatico | `botonCancelar()` | `modo_automatico.cpp` |
| Manual | `botonCancelar()` | `modo_manual.cpp` |
| Inteligente | `botonCancelar()` | `modo_inteligente.cpp` |
| Ajustar hora | `botonCancelar()` | `modo_hora.cpp:262` |
| Menu (subir de nivel) | `botonCancelar()` | `menu.cpp` |

**MEDIDO** el juego completo de comandos que atiende el Maestro por Bluetooth
(`Maestro/src/bluetooth.cpp:108-181`):

```
   CMD:FORZAR_ROJO                (sin PIN, deliberado — bluetooth.cpp:99-113)
   CMD:PIN:1234:SET_MODO:AUTO
   CMD:PIN:1234:SET_MODO:MANUAL
   CMD:PIN:1234:SET_MODO:AMBAR
   CMD:PIN:1234:FORZAR_ROJO
   CMD:PIN:1234:MANUAL:CAMBIAR_TURNO
   CMD:PIN:1234:TEST_LEDS
   CMD:PIN:1234:SET_TIEMPOS:<v>,<r>,<d>
   CMD:PIN:1234:SET_RTC:<fecha>,<hora>
```

~~Y el del Esclavo (`Esclavo/src/bluetooth.cpp:124-168`): `FORZAR_ROJO`, `SOLICITAR_PASO`,
`TEST_LEDS`, `SET_RTC`. **Ningun `SET_MODO`.**~~

> 🔴 **Este censo del Esclavo estaba INCOMPLETO, y le faltaba el comando mas importante de los
> cinco.** MEDIDO el 31/08 sobre `Esclavo/src/bluetooth.cpp`:
>
> ```
>    :130   CMD:AMBAR_EMERGENCIA        <- SIN PIN, y el censo del 28/08 NO lo tenia
>    :157   CMD:FORZAR_ROJO             -> $ERR: RENOMBRADO_USE_AMBAR_EMERGENCIA (N-83)
>    :171   PIN + AMBAR_EMERGENCIA      :176  PIN + FORZAR_ROJO -> el mismo $ERR
>    :184   PIN + SOLICITAR_PASO        :202  PIN + TEST_LEDS   :215  PIN + SET_RTC
> ```
>
> Sigue siendo cierto que **no hay ningun `SET_MODO` en el Esclavo**. Lo que era falso es la lista:
> `FORZAR_ROJO` ya no hace lo que este documento suponia —se rechaza ensenando el nombre bueno— y
> `AMBAR_EMERGENCIA` es una entrada sin PIN que el censo no vio. Un censo al que le falta una
> puerta no es un censo incompleto: es un censo que **afirma** que esa puerta no existe.
>
> 🔴 **Y ese comando tiene un defecto abierto: N-106.** `grep -in degradado
> Esclavo/src/bluetooth.cpp` da **CERO** — el ambar de emergencia de la app **no llama a
> `degradado_salir()`**, mientras el `B.B.B` del mando si (`grep -n "degradado_salir" Esclavo/src/mando.cpp Maestro/src/mando.cpp`). Las dos vias que el
> propio fuente declara equivalentes (`bluetooth.cpp:32-39`) **no lo son en Degradado**. Detalle,
> consecuencia razonada y el arnes que hay que ver fallar: `ESTADO.md`, seccion **N-106**.

El Maestro tiene **ocho** modos (`grep -n 'strcmp(accion, "SET_MODO:' Maestro/src/bluetooth.cpp`: `MENU`, `MANUAL`, `AUTO`, `INTELIGENTE`,
`ALCANCE`, `HORA`, `DEGRADADO`, `AMBAR`). ~~Desde Bluetooth se alcanzan **tres**. Y **no hay
`SET_MODO:MENU`**.~~ → **REFUTADO (N-100): hoy se alcanzan `AUTO`, `MANUAL`, `AMBAR`, `MENU`,
`ALCANCE`, `INTELIGENTE` y `DEGRADADO` — siete de los ocho** (`HORA` no, y `SET_RTC` la sustituye).

> ~~**Consecuencia exacta, y es la que hay que leer despacio:** hoy el operario entra a un modo por
> el menu y sale con el Boton 4. Si se ignoran los pulsadores **antes** de anadir `SET_MODO:MENU`,
> cada modo se convierte en una puerta de un solo sentido: se entra desde el celular y **no se sale
> por ningun sitio**. Lo unico que queda es `FORZAR_ROJO` —que para el trafico pero no devuelve el
> mando— y cortar la energia.~~
>
> 🟢 **REFUTADO el 31/08, y se conserva porque explica por que la Fase 1 iba primero.** El
> razonamiento era correcto **y se atendio**: la rama `strcmp(accion, "SET_MODO:MENU")` de `bluetooth.cpp` existe desde
> `d34cfe2`. La puerta de un solo sentido no llego a existir.
>
> **Lo que sigue en pie es la parte de la app**, y **se ha vuelto a contar el 05/09 porque la de
> antes era del 28/08 y `caef8a1` toco la app despues** —que es justo lo que este parrafo pedia—.
> El censo no se hace por numero de linea sino por el **unico emisor** que la app tiene:
>
> ```
> grep -n "enviarComandoFirmware(" App_Semaforo/www/app.js      ->  20 llamadas
> grep -n "SOLO_MAESTRO\|SOLO_ESCLAVO"  App_Semaforo/www/app.js  ->  las dos listas de enrutado
> ```
>
> Salen `SET_MODO` con `AUTO`, `MANUAL` y `AMBAR`, `MANUAL:CAMBIAR_TURNO`, `FORZAR_ROJO`,
> `SOLICITAR_PASO`, `SET_TIEMPOS`, `SET_RTC` y `TEST_LEDS`. Con el firmware ya sirviendo siete
> modos, lo que falta comprobar es **si la app tiene boton para cada uno**, que es otra pregunta.

### 2.4 🔴 Retirar el mando no deja tres `if` inertes: **borra un veto**

> 🔵 **ACTUALIZADO EL 31/08: la decision del responsable es CONSERVAR el mando en `A` y `B`, asi que
> el veto NO se retira y no hay que decidir quien lo hereda.** Este apartado se conserva entero —es
> el analisis que justifica la decision— con dos correcciones dentro: las tres lineas citadas
> **estaban caducadas**, y falta el dato de donde se arma la bandera.
>
> 🔴 **Y UN TERCER DATO, DEL 04/09, QUE NO CAMBIA EL CODIGO PERO SI LO QUE SIGNIFICA (N-118).** El
> veto sigue **entero** en el firmware: `ACC_AMBAR` sigue siendo el unico armador y los tres `if`
> siguen negados. Lo que el banco midio es que **nadie podia llegar a `ACC_AMBAR`** con `617bd00`,
> porque `B·B·B` no se podia pulsar con `PB13` a `0,6 V` en reposo y el pin leido activo en BAJO.
>
> **`mando_ambarLocal()` devolvia siempre `false` — que es exactamente el escenario que este
> apartado describe como peligroso**, y se llego a el **sin retirar una sola linea**. Es la
> version silenciosa de lo que se temia: los tres `if` eran siempre-verdaderos, y ningun `git diff`
> lo delataba. **La barrera no se borro: se quedo sin quien la arme.**
>
> 🟢 **Y esto es lo que cambia con `346ea5f`: el armador vuelve a ser alcanzable EN EL FUENTE**
> —`PB13` en `INPUT` pelado y `== HIGH`, las dos puntas—. 🔴 **En la tarjeta, sin verificar.** Hasta
> que una carga verificada demuestre que `B·B·B` arma `ACC_AMBAR`, **lo que hay escrito arriba sigue
> siendo la descripcion del equipo real**, y el pack que exija que `ACC_AMBAR` es el unico armador
> (punto 8 del Anexo) sigue sin escribirse.

**MEDIDO**: `mando_ambarLocal()` (`Esclavo/src/mando.cpp:103`) tiene tres consumidores, todos en
`Esclavo/src/main.cpp`, y **los tres son negados**:

| ~~linea (28/08)~~ | **linea real, MEDIDA el 31/08** | que veta hoy |
|---|---|---|
| ~~`401`~~ | **`406`** | `if (!mando_ambarLocal() && !bluetooth_ambarEmergencia())` antes de obedecer `CMD_GO_RED` |
| ~~`408`~~ | **`416`** | idem antes de obedecer `CMD_GO_GREEN` |
| ~~`526`~~ | **`540`** | `if (!mando_ambarLocal() && !bluetooth_ambarEmergencia() && ...)` antes de recuperarse de `S_FALLO` |

> **Las tres cifras viejas no son un detalle de estilo: son la direccion que alguien abre para
> comprobar.** Una linea caducada en un documento que se lee como MEDIDO manda al lector a un sitio
> que no dice lo que promete, y este repositorio ya pago eso (`CLAUDE.md` §5). Se corrigen y se deja
> lo que decian, para que quien tenga una copia vieja sepa que ya no vale.

**Y falta el dato que decide la forma de la decision: DONDE se arma la bandera.** MEDIDO el 31/08:

```
   Esclavo/src/mando.cpp:132     case ACC_AMBAR:  ambarLocal = true;   <- UNICO sitio
   Esclavo/src/mando.cpp:246-248  B.B.B -> confirmarYActuar(ACC_AMBAR, ...)
```

`ambarLocal` se arma en **un solo sitio**, y a ese sitio solo se llega por `B.B.B`. Por eso el veto
no depende del mando *en general* sino **del canal `B` en particular**: conservar `A` y retirar `B`
lo borraria igual que retirarlo entero, y conservar `B` lo salva entero. Es la razon medida por la
que la decision del 31/08 conserva **los dos** canales.

El comentario que lo explica esta en **`Esclavo/src/main.cpp`, bajo la etiqueta `SFTY-21`**
—`grep -n "SFTY-21: con el ambar pedido desde el mando" Esclavo/src/main.cpp`—: con el ambar
pedido desde el piso con `B·B·B`, el Esclavo **no obedece ni acusa recibo**, para que el Maestro agote reintentos y el
**cruce entero** termine en ambar, que es lo que el operario pidio.

> **Al retirar el mando, `mando_ambarLocal()` pasa a devolver siempre `false` y esos tres `if` se
> vuelven siempre-verdaderos.** El codigo no queda muerto: **queda abierto**. Una orden de radio
> vuelve a poder sacar al Esclavo de un ambar que un operario habia dejado puesto a proposito.
>
> Eso no es un residuo de limpieza: es **SFTY-21 desapareciendo por sustraccion**. ~~Antes de quitar
> el mando hay que decidir **quien hereda ese veto** —un flag de «ambar local vigente» que ponga la
> app, o la decision explicita y escrita de que ese veto ya no existe— y el pack que lo vigile.~~
>
> 🟢 **RESUELTO EL 31/08 POR DECISION, NO POR CODIGO: el mando se conserva en `A` y `B`, asi que el
> armador se queda y no hay nada que heredar.** Los tres `if` siguen vetando de verdad. Es la salida
> mas barata de las que habia sobre la mesa —cuesta cero bytes de flash y cero lineas— y es tambien
> la unica que no exige escribir una barrera nueva y verla fallar antes de fiarse de ella.
>
> **Lo que si conviene escribir, aunque ya no sea urgente:** un pack que exija que `ACC_AMBAR`
> (`grep -n "ACC_AMBAR" Esclavo/src/mando.cpp`) siga siendo el **unico** armador de `ambarLocal` y que los tres consumidores
> sigan siendo tres y negados. Hoy esa propiedad vive en la lectura de un documento, y los
> documentos no fallan cuando alguien borra una linea (`CLAUDE.md` §3.bis, N-71).
>
> ⚠️ **Y queda un segundo veto, el de la app, con un defecto abierto: N-106.** `bluetooth_ambarEmergencia()`
> acompana a `mando_ambarLocal()` en los tres `if`, pero su armador —la rama `strcmp(cmd, "CMD:AMBAR_EMERGENCIA")` de `Esclavo/src/bluetooth.cpp`,
> `:171`— **no sale del Modo Degradado** como si hace `ACC_AMBAR`. Ver `ESTADO.md` §N-106: medido por
> lectura, **no ejecutado**, y se cierra con un arnes visto fallar.

### 2.5 ~~🔴 `SET_RTC` puede rechazar en silencio y contestar `RESULT:OK`~~ → ✅ **CERRADO EN N-80 (`d34cfe2`)**

> ✅ **REFUTADO el 31/08: el defecto que describe este apartado ya no existe.** Se conserva entero
> porque el analisis es correcto y es el que motivo el arreglo — y porque una causa que desaparece en
> silencio se vuelve a proponer (`CLAUDE.md` §4).
>
> **MEDIDO el 31/08, y revalidado el 05/09 con `grep -c "CMD:SET_RTC," Maestro/src/bluetooth.cpp` → **5**: hoy `SET_RTC` tiene CINCO
> ramas y ninguna contesta sin mirar.**
>
> ```
>    :306   sscanf(...) != 6                  -> $ERR,CMD:SET_RTC,DESC:FORMATO_INVALIDO
>    :309   !reloj_hayCristal()               -> $ERR,DESC:SIN_CRISTAL_VEA_CONSULTA_RELOJ
>    :314   !ajustarRelojVerificado(...)      -> $ERR,DESC:FORMATO_INVALIDO
>    :319   !coordinador_sincronizarHora()    -> $ACK,RESULT:HORA_PUESTA_SIN_PROPAGAR
>    :325   todo bien                         -> $ACK,RESULT:OK
> ```
>
> El comentario de `:297-305` recoge el porque con las mismas palabras que este apartado. Y el propio
> fuente declara en `:320-323` que la cuarta rama **hoy no puede ocurrir** y por que se deja puesta —
> que es como se anota un camino que existe pero no se ejerce, en vez de borrarlo o de fingir que se
> prueba.
>
> Sigue en pie la consecuencia de fondo, que no es de firmware: **con `Y2` muerto (N-17, medida de
> banco del 01/08) y sin pantalla, el unico canal es el `$ACK`.** La diferencia es que ahora el `$ACK`
> dice la verdad.

**Lo que sigue es el texto del 28/08, conservado.**

**MEDIDO**, y son dos ficheros:

```
   01_Firmware/Maestro/src/reloj.cpp:290       if (!rtcOperativo) return;      <- rechaza y no dice nada
   01_Firmware/Maestro/src/bluetooth.cpp:173   reloj_ajustar(...);
   01_Firmware/Maestro/src/bluetooth.cpp:175   enviarTramaConCrc("$ACK,CMD:SET_RTC,RESULT:OK");
```

*(Esas tres lineas son las de `3733544`, y **por eso llevan numero: una cita fechada a un commit
concreto es legitima** — se verifica con `git show 3733544:...`, no contra el arbol de hoy. Lo que
no vale es citar por numero el arbol de HOY. Tras `d34cfe2` el bloque de `SET_RTC` se localiza con
`grep -n "CMD:SET_RTC," Maestro/src/bluetooth.cpp` — ver arriba.)*

`bluetooth.cpp` valida **el formato** de la trama (`sscanf(...) == 6`) y sobre esa validacion
contesta `OK`. **No consulta el resultado de `reloj_ajustar()`, que no devuelve nada.** El rechazo
por falta de oscilador esta escrito con toda intencion en `reloj.cpp`, dentro de `reloj_hayCristal()` y su comentario —`grep -n "reloj_hayCristal" Maestro/src/reloj.cpp`—, y la razon es buena:
escribir una hora que nadie hace avanzar dejaria `horaValida` en `true` y sobre esa mentira el
Maestro empujaria la hora al Esclavo y autorizaria el Modo Degradado—. Lo que falta es **que se
entere el que pregunta**.

**Y no es hipotetico.** Las entradas **N-17** y **N-37** del `roadmap.md` —`grep -n "N-17\|N-37"
roadmap.md`, las dos siguen vivas el 05/09— cierran, con medida de
banco del 01/08/2026: **el cristal `Y2` no oscila en las tarjetas actuales.** Tres eliminaciones
documentadas: `VBAT` a 3 V con la tarjeta apagada, el reintento de N-25 cada 30 s y `REINICIAR
RELOJ` devolviendo `SIGUE PARADO`.

> **Consecuencia con la nueva arquitectura, y es peor que hoy, no mejor.** Hoy el operario tiene la
> pantalla `CONSULTA RELOJ` para ver que el reloj no arranco. **Sin pantalla, el unico canal es el
> `$ACK`, y el `$ACK` dice `OK`.** La app le confirmara al tecnico que puso la hora, en un equipo en
> el que la hora no se puede poner. El campo `HORA:` de `$STATUS` seguira diciendo `--:--:--`
> (`bluetooth.cpp:230-235`), que es la unica pista, y esta al lado de un `OK`.
>
> ~~El firmware ya tiene el dato: `reloj_hayCristal()` (`reloj.cpp:219`). Lo que falta es que
> `SET_RTC` lo mire antes de contestar.~~ → ✅ **HECHO en `d34cfe2` (N-80): la rama `SET_RTC` de `bluetooth.cpp` lo
> mira.** Ver el recuadro de la cabecera de este apartado.

### 2.6 🟠 Telemetria fabricada: que campos de `$STATUS` son datos y cuales son texto

**MEDIDO EL 28/08** sobre los dos `snprintf`. ⚠️ **Los dos han cambiado desde entonces —N-108,
N-139/N-143 y N-149—: lo de aqui abajo es el estado VIEJO, y el de hoy esta en el recuadro que sigue
a la tabla.** No se borra porque es lo que este apartado consiguio que se arreglara.

**Esclavo** (`Esclavo/src/bluetooth.cpp:215`):

```
"$STATUS,NODE:ESCLAVO,SERIE:%s,MODO:SUBORDINADO,ESTADO:%s,T:%lu,RF:98%%,RTT:85ms,BAT:12.6,HORA:%s"
```

**Maestro** (`Maestro/src/bluetooth.cpp:245`):

```
"$STATUS,NODE:MAESTRO,SERIE:%s,MODO:%s,ESTADO:%s,T:%lu,RF:%d%%,RTT:%lums,BAT:12.6,HORA:%s"
```

| campo | Maestro | Esclavo |
|---|---|---|
| `SERIE` | dato (UID de silicio) | dato |
| `MODO` | dato | 🔴 **literal `SUBORDINADO`** |
| `ESTADO` | dato | dato |
| `T` | 🟠 **`(millis()/1000) % 60`** | 🟠 **igual** — *(las dos casillas describen `617bd00`; **hoy no queda ni un `millis()/1000` en el `$STATUS` de ninguna de las dos puntas** — verificado el 05/09 —, y lo que hay es el recuadro de abajo)* |
| `RF` | dato (`coordinador_calidadEnlace()`) | 🔴 **literal `98%`** |
| `RTT` | dato (`coordinador_tiempoRespuestaMs()`) | 🔴 **literal `85ms`** |
| `BAT` | 🔴 **literal `12.6`** | 🔴 **literal `12.6`** |
| `HORA` | dato, o `--:--:--` | dato, o `--:--:--` |

> 🟢 **ESTA TABLA ESTA CADUCADA DESDE N-108 (04/09) Y N-149 (05/09), Y SE TACHA EN VEZ DE
> REESCRIBIRSE PORQUE EL CAMBIO ES LA NOTICIA.** Los `snprintf` de hoy, releidos:
>
> ```
>    Maestro/src/bluetooth.cpp:929
>      $STATUS,NODE:MAESTRO,SERIE:%s,MODO:%s,ESTADO:%s,T:%s,RF:%s,RTT:%s,BAT:--,HORA:%s,ESC:%s
>    Esclavo/src/bluetooth.cpp:791
>      $STATUS,NODE:ESCLAVO,SERIE:%s,MODO:SUBORDINADO,ESTADO:%s,T:--,RF:--,RTT:--,BAT:--,HORA:%s
> ```
>
> | campo | Maestro, hoy | Esclavo, hoy |
> |---|---|---|
> | `T` | 🟢 **dato**: `coordinador_segundosRestantesFase()` y, si no hay, `modoAutomatico_segundosRestantesFase()`; **`--` cuando no hay cuenta atras que dar** (N-139/N-143) | 🟢 **`--` fijo, y es lo correcto**: el Esclavo es SUBORDINADO y sus ordenes **no llevan duracion**. Inventar la cuenta seria adivinar cuando el Maestro mandara la siguiente |
> | `RF` / `RTT` | dato | 🟢 **`--`** — ~~literales `98%` y `85ms`~~ |
> | `BAT` | 🟢 **`--`** — ~~literal `12.6`~~ | 🟢 **`--`** — ~~idem~~ |
> | `ESC` | 🆕 **dato** (N-149): lo que el Esclavo **confirmo por acuse**, o `?` con el enlace caido | **no lo emite, a proposito** — no tiene de donde sacarlo |
>
> **Lo que este apartado pedia —*«un campo que no se mide se retira o se marca; no se deja con
> aspecto de medida»*— ESTA HECHO en las dos puntas.** Lo que **no** esta hecho es medir la bateria:
> `BAT` sigue sin cifra porque **no hay divisor ni canal ADC**, y eso es una linea de compras, no un
> defecto de firmware. Ver el bloque 6 de la revision del 04-05/09.
>
> ⚠️ **Y la excepcion que existe y no se publica, para que no se pierda:** en Modo Degradado el
> Esclavo **si** conoce su fase —la calcula por reloj, `degradado_segundosParaCambio()`—, y aun asi
> no la publica: **el Maestro no expone el getter equivalente**, y encender una sola punta dejaria al
> operario con un numero en un poste y `--` en el otro **para el mismo ciclo**. Cuando el Maestro
> exponga el suyo, las dos se encienden **en el mismo commit**.

**`BAT:12.6` es literal en las dos puntas, y no hay ningun `analogRead` en el firmware.**
Comprobado con el buscador descartado antes de reportar (`CLAUDE.md` §4): `grep -rn analogRead`
sobre `01_Firmware/Maestro` y `01_Firmware/Esclavo` solo da coincidencias dentro de
`.pio/build/*/firmware.map` y de los objetos del framework de Arduino — **ni una en `src/` ni en
`include/`**. No hay divisor de tension, no hay canal ADC, no hay medida de bateria. El `12.6` es
una constante escrita a mano.

Y el campo `T:` **no es el tiempo de fase**. Es un contador libre que da la vuelta cada minuto,
independiente de en que fase este el cruce. El comentario de al lado dice *"Cuenta de segundos
transcurridos en fase actual"* — y no lo es.

> **Por que esto sube de prioridad con la nueva arquitectura, no baja.** Con pantalla, la
> telemetria era un canal de comodidad. **Sin pantalla es el unico tablero que existe**, y
> `CLAUDE.md` §3.quinquies ya cobro esta leccion en la app: *"lo que sustituye a un dato que no se
> tiene no es una simulacion: es decirlo"*. Un `98%` fijo en el celular de quien decide sobre el
> trafico es de la misma familia que el simulador que se retiro de la pantalla principal — solo que
> este vive en el firmware.
>
> Un campo que no se mide se retira o se marca. **No se deja con aspecto de medida.**

### 2.7 🟢 Del mando de reles no se retira equipo en servicio: nunca se compro el receptor

**ESCRITO**, y coherente en tres sitios:

- ~~`roadmap.md:3357` (N-19)~~ → 🔴 **AFIRMACION FALSA desde el 05/09, y el motivo es
  instructivo: `grep -c "N-19" roadmap.md` da CERO.** La entrada **ya no esta en el roadmap** —se
  fue al historico en `b327550`, *"docs(roadmap): arranca hoy - estado del proyecto, no bitacora"*—,
  asi que la cita no apuntaba a otra linea: apuntaba a un fichero que **ya no contiene el texto**.
  *(Control: `N-73` da 4 y `N-118` da 14 en ese mismo fichero, o sea que el buscador ve.)* El texto
  sobrevive donde se sigue usando: `04_Manuales/MANUAL_MANDO_4_RELES.md` y `ESTADO.md` —
  `grep -rn "N-19" 04_Manuales ESTADO.md OPTIMIZACIONES.md`. Decia: *"El Maestro tiene mando; el
  Esclavo no. La tarjeta ya tiene las cuatro entradas (`PB9`, `PB13`, `PB14`, `PB15`) — **falta
  solo el receptor**"*.
- `05_Funcional/8_Procedimiento_Modo_Degradado.md`, en dos sitios —`grep -n "N-19"
  05_Funcional/8_Procedimiento_Modo_Degradado.md`—: *"El Esclavo no tiene receptor de mando de
  relés (pendiente N-19). La tarjeta ya trae las cuatro entradas; **falta comprar e instalar el
  receptor**"*.
- `04_Manuales/MANUAL_MANDO_4_RELES.md`, el apartado del codigo por punta: la advertencia de exigir **codigo independiente por
  unidad** *"al comprarlo"*.

**MEDIDO:** en el protocolo de 80 pruebas, la seccion 8 (mando de 4 reles, 8 pruebas) y la 13
(blindaje del mando, 2 pruebas) **no tienen acta firmada**: son parte de las 48 de V9.0, que es la
tanda que `ESTADO.md` declara *"implementada y compilando. NO probada en banco"*.

> **Lo que se retira del mando es codigo y papel, no equipo montado en la calle.** Es el unico de
> los ocho hallazgos que **abarata** la decision, y por eso conviene tenerlo escrito: nadie tiene
> que subir a un poste a desmontar nada.
>
> **La excepcion es §2.4.** El receptor no existe, pero **el veto que el mando aporta en el
> firmware si existe y esta activo**. Retirar codigo que nunca tuvo hardware detras sigue cambiando
> el comportamiento del equipo.

### 2.8 🔴 El protocolo de 80 pruebas: 49 dejan de ser ejecutables

> ⚠️ **ESTA CUENTA QUEDA CADUCADA EL 31/08, y en la direccion buena: caen MENOS de 49.** No se
> publica aqui el numero nuevo porque **no se ha recontado seccion por seccion**, y este documento no
> escribe una cifra que no haya podido reproducir (`CLAUDE.md` §4, que es la misma regla por la que
> se rechazo el «~24» del 28/08). Lo que si esta medido es **que filas se mueven y por que**:
>
> | seccion | decia | por que se mueve |
> |---|---|---|
> | 8 — Mando de 4 reles (8) | caen **8** | el mando **se conserva** en `A` y `B` (§1.6). `A.A.A` y `B.B.B` siguen existiendo |
> | 13 — Blindaje del mando N-53 (2) | caen **2** | el mando se conserva; lo que cae es la mitad de *pantalla AJUSTAR HORA* |
> | 9 — Modo Degradado (12) | caen 7, sobreviven 5 con asterisco | el asterisco **se retira**: `SET_MODO:DEGRADADO` existe —`grep -n 'strcmp(accion, "SET_MODO:DEGRADADO")' Maestro/src/bluetooth.cpp`—, y `A.B.A.B` tambien |
> | 5 — Modo Manual (10) · 7 — Reloj (11) · 10 — Interfaz del Esclavo (5) | caen | **no se mueven**: son pantalla y menu, y eso si se retira |
>
> **Recontarlo es trabajo de la reescritura del Manual 3 (Orden 5 de la seccion B), una prueba por
> una, anotando en cual de los tres sitios cae cada una.** La tabla de abajo se conserva como estaba
> el 28/08: era correcta con la decision de entonces.

**MEDIDO** cruzando las 13 secciones de `05_Funcional/3_Protocolo_Pruebas_Rigurosas.md` (recuento
de la propia acta, lineas 799-820) contra lo que se retira.

| seccion | pruebas | caen | sobreviven | por que caen |
|---|---|---|---|---|
| 1 — Menu e independencia de radio | 4 | **4** | 0 | las cuatro se observan en la LCD |
| 2 — Perdida de comunicacion / self-healing | 5 | 0 | 5 | se observan en las luces |
| 3 — Modo Automatico | 5 | 0 | 5 | se observan en las luces |
| 4 — Modo Inteligente | 4 | **1** | 3 | 4.1 es *"interfaz dedicada"* |
| 5 — Modo Manual (botonera fisica) | 10 | **10** | 0 | la seccion **es** la botonera y el menu |
| 6 — Repetidor ESP32 | 4 | 0 | 4 * | |
| 7 — Reloj y AJUSTAR HORA | 11 | **11** | 0 | las once pasan por pantalla o botones |
| 8 — Mando de 4 reles | 8 | **8** | 0 | el mando se retira |
| 9 — Modo Degradado | 12 | **7** | 5 * | 9.2-9.6, 9.10, 9.11 prueban gestos de boton |
| 10 — Interfaz propia del Esclavo | 5 | **5** | 0 | la seccion **es** la LCD del Esclavo |
| 11 — Camaras IA | 4 | **1** | 3 | 11.4 es *"independencia de los botones"* |
| 12 — Bluetooth y telemetria | 6 | 0 | 6 * | |
| 13 — Blindaje del mando (N-53) | 2 | **2** | 0 | mando + pantalla AJUSTAR HORA |
| **TOTAL** | **80** | **49** | **31** | |

Las tres columnas con `*` **no sobreviven tal como estan escritas**:

- **Seccion 6 (4):** el repetidor esta **fuera de la configuracion vigente** — `CLAUDE.md` §10:
  *"2 radios en enlace directo, sin repetidor"*. Ya no eran ejecutables antes de esta decision.
- **Seccion 9 (5):** 9.1, 9.7, 9.12, 9.13 y 9.14 **solo sobreviven si existe una via de entrada al
  Degradado desde la app**, ~~y hoy **no existe ninguna** (§2.3: no hay `SET_MODO:DEGRADADO`)~~ →
  🟢 **REFUTADO el 31/08 (N-100): la via EXISTE.** MEDIDO en `Maestro/src/bluetooth.cpp:234`
  (`SET_MODO:DEGRADADO`), con `$ERR` motivado en `:245` y `$ACK,RESULT:OK` en `:250`. **Estas cinco
  sobreviven**, y hay que reescribir el *gesto* de cada una —de `A·B·A·B` a un comando— sin tocar lo
  que miden. **Y con el mando conservado en `A` y `B` (§1.6), el gesto `A·B·A·B` tampoco desaparece:
  las cinco se pueden ejecutar por las dos vias.**
- **Seccion 12 (6):** 12.1 es *"emparejamiento y enlace inalambrico"* con un `HC-05`; hay que
  reescribirla entera para el ESP32.

> **La cuenta que se lleva el auditor: 49 caen, 31 sobreviven en principio, y solo 16 sobreviven
> tal como estan redactadas hoy.**
>
> ⚠️ **Y aqui va una discrepancia que no se tapa.** A este documento se le dio de partida la cifra
> *"~49 caen, sobreviven ~24"*. **La primera se reprodujo exactamente; la segunda no.** Contando
> seccion por seccion salen 31, o 16 si se descuentan las tres columnas con asterisco. **No hay
> ninguna particion razonable que de 24**, y este documento no escribe un numero que no ha podido
> reproducir. `CLAUDE.md` §4: cuando el instrumento y el razonamiento no coinciden, manda la medida
> — y el instrumento aqui es la tabla de arriba, que se puede recontar en cinco minutos.

---

## 3. Decisiones ABIERTAS, con dueno

Ninguna de estas ~~cinco~~ ~~seis~~ **nueve** la puede tomar quien escribe firmware. Van con quien
las tiene que firmar. *(La sexta —§3.6, N-120— la trajo el banco del 03-04/09 y no existia el 28/08.
La §3.7 y la §3.8 salieron de la revision del 04/09 por la tarde; la **§3.9**, de la de la noche.)*

> ✏️ **Tres de las nueve ya estan DECIDIDAS —§3.3, §3.4 y §3.7— y se quedan en esta lista a
> proposito:** una decision entre alternativas solo se puede revisar si las alternativas siguen
> escritas.

### 3.1 🔴 Que chip es el ESP32 — **bloquea la compra y bloquea la app**

**Dueno: el responsable.** No es una decision tecnica: es que **la app depende de la respuesta**.

**ESCRITO** en `05_Funcional/15_Lista_de_Compras_Hardware.md:79-83` y replicado en
`05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.md:83-84`:

| familia | Bluetooth | ¿la app conecta? | ¿que pasa con el Manual 10 §1? |
|---|---|---|---|
| `ESP32-WROOM-32` / `-32D` / `-32E` / `-32U` (**clasico**) | Clasico (BR/EDR) + BLE | ✅ **si, tal cual** — `createRfcommSocketToServiceRecord` abre | **intacto** |
| `ESP32-S3` · `ESP32-C3` | **solo BLE** | ❌ **nunca** — el socket RFCOMM no abre | 🛑 **hay que reabrirlo por escrito** |
| `ESP32-S2` | **ninguno** (solo WiFi) | ❌ | 🛑 **hay que reabrirlo por escrito** |

El apartado 1 del Manual 10 esta **congelado por escrito** (`10_Manual...:26`, `:146-148`):
*"Bluetooth Clasico SPP. No BLE. No Web Bluetooth. Y no es negociable sin reabrir este apartado por
escrito."* La razon esta medida y pagada: `navigator.bluetooth` **no puede ver un SPP** —la API no
existe para ese perfil— y eso ya costo una version entera de la app.

> **Como se responde, y no admite atajo (`10_Manual...:91`, `:100`):** se lee la **serigrafia del
> blindaje metalico** del modulo. `ESP32-WROOM-32E` es una respuesta; `ESP32-S3-WROOM-1` es otra.
> **El rotulo del vendedor no distingue.**
>
> **SIN VERIFICAR:** nadie ha leido todavia la serigrafia de los modulos que llegaron a obra el
> 28/08. La lista de compras los registra como *"referencia sin confirmar"*
> (`15_Lista...:69`). **Es una comprobacion de treinta segundos con el modulo en la mano y decide
> si hay que rehacer el transporte de la app entero.**

### 3.2 🟠 El cristal `Y2`: se repara, o el STM32 lleva reloj de software

**Dueno: el responsable**, porque una via cuesta taller y la otra cuesta firmware y flash.

| via | que exige | riesgo |
|---|---|---|
| **A — reparar el hardware** | cambiar `C1`/`C2` del `Y2` por 6-10 pF C0G/NP0 (`MAPEO_TARJETA_KICAD.md` §4) | 🟡 *"hipotesis razonable, sin verificar en esta tarjeta"* — lo dice el propio manual |
| **B — reloj de software en el STM32, disciplinado por el ESP32** | un contador propio que el ESP32 pone en hora reenviando la hora del `DS3231` | 🟠 deriva entre reenvios; **y cuelga el reloj del semaforo del modulo accesorio**, que es justo lo que §1.1 separa |

**Antes de decidir hay una medida pendiente que puede ahorrar la compra entera:** `ESTADO.md` `B5`
— *"Diagnostico de los dos cristales `Y2`. Decide todo el bloque C: si el muerto es el del Esclavo,
no se compra nada"*. **N-37 midio uno**; el otro sigue SIN VERIFICAR.

> **Y hay una consecuencia de la via B que conviene tener escrita antes de elegirla:** el Modo
> Degradado exige reloj (`reloj_enHora()`), y §2.5 muestra que hoy el rechazo por falta de reloj es
> silencioso. Un reloj de software que el ESP32 disciplina significa que **si el ESP32 no esta, el
> reloj se va yendo** — y nadie lo ve, porque el unico tablero es el ESP32.

### 3.3 ✅ ~~🔴~~ Sin pantalla, sin pulsadores ~~y sin mando~~: como se opera el equipo si el ESP32 se cuelga — **DECIDIDA EL 31/08**

> 🔴 **AVISO DEL 04/09, ANTES DE LEER NADA DE ESTE APARTADO: la opcion elegida NO ESTABA
> CONSTRUIDA, Y TODAVIA NO ESTA DEMOSTRADA.** El banco midio `J16` p5 y p8 en **`0,6 V`
> permanentes** (M3, N-118) y con `617bd00` el mando `A`/`B` **no se pudo pulsar**: el pin nacia en
> `LOW`, el antirrebote lo daba por consumido y no habia flanco que detectar. Se puenteo en el paso
> 29 y **el semaforo no se inmuto**.
>
> **Asi que el agujero que este apartado daba por tapado sigue abierto.** La decision no era mala
> —era la unica opcion que no costaba nada—, pero **se tomo creyendo que ya estaba construida, y no
> lo estaba**. Hacian falta **dos lineas de `botones.cpp`, una por punta** (Anexo, punto 5), su pack
> visto fallar antes, y una carga verificada.
>
> 🟢 **Las dos lineas YA ESTAN: `346ea5f`, las dos puntas en `INPUT` pelado y `== HIGH`.** 🔴 **Falta
> el resto, y es lo que decide: el pack visto fallar y la carga verificada en tarjeta.** Mientras no
> se ejerza, **la fila que este apartado retiro de la mesa el 31/08 —*aceptar el ambar como estado
> final y subir al gabinete*— sigue siendo lo que de hecho esta pasando**, sin que nadie la haya
> firmado. **Un arreglo en el fuente no tapa un agujero de calle: lo tapa una carga verificada**
> (`CLAUDE.md` §9.bis).
>
> ⚠️ **Y cuando se ejerza, el gesto es OTRO:** ~~puentear `p5`/`p8` contra masa~~ → **cerrar `p5`
> contra `p4` y `p8` contra `p7`** (los `3,3 V` contiguos), y **no sobre la tarjeta Maestro** con el
> corto de N-116.
>
> ✅ **DECIDIDA POR EL RESPONSABLE EL 31/08: se elige la opcion 3 de la tabla de abajo — DEJAR EL
> MANDO DE RELES**, en los canales `A` y `B`. La tabla se conserva entera: una decision entre
> alternativas escritas solo se puede revisar si las alternativas siguen escritas.
>
> **Que resuelve, exactamente:** un ESP32 colgado —o muerto, o desenchufado, que es lo que el
> watchdog **no** cubre— deja el equipo con `A.A.A` (volver a Automatico), `B.B.B` (ambar desde
> cualquier estado) y `A.B.A.B` (Degradado) desde el piso, sin escalera y sin pantalla. **El equipo
> deja de quedarse sin ninguna superficie de mando**, que era el agujero que abria este apartado.
>
> **Que NO resuelve, y va escrito al lado:** el mando esta en el **Maestro**; el Esclavo tiene las
> cuatro entradas pero **no tiene receptor comprado** (§2.7). Asi que la salida fisica de ultimo
> recurso existe **en una punta de las dos**, y en el Esclavo sigue existiendo solo como cobre. Si se
> quiere en las dos, hay que comprar el segundo receptor — es la linea de N-19 que lleva abierta
> desde el principio.
>
> **Y las otras opciones no quedan derogadas por esta:** el **watchdog del ESP32** (opcion 1) sigue
> siendo barato y sigue siendo la Fase 5, y ahora es *complemento*, no sustituto. Lo que esta
> decision retira de la mesa es la ultima fila —*aceptar el ambar como estado final y subir al
> gabinete*—, que era la que habia que firmar.
>
> **Coste de la decision: cero bytes y cero lineas** — es **no retirar** codigo que ya esta y ya
> compila. **Y no ha pasado banco**, como nada de este documento.

**Dueno: ~~el responsable~~ DECIDIDA (31/08).** Era la decision mas grande de las cinco y la unica
sin ninguna via construida; la via elegida es la unica que ya estaba construida.

**Lo MEDIDO:**

```
   01_Firmware/Maestro/src/main.cpp:52    IWatchdog.begin(4000000);     <- 4 s
   01_Firmware/Esclavo/src/main.cpp:238   IWatchdog.begin(4000000);     <- 4 s
   01_Firmware/Repetidor/src/            grep -rn "watchdog|esp_task_wdt|WDT"  ->  CERO coincidencias
```

**El ESP32 de este proyecto no tiene watchdog.** ~~Lo dice tambien `roadmap.md:2706`, en la casilla
de H-3: *"(El Repetidor ESP32 sigue sin watchdog.)"*~~ → 🔴 **AFIRMACION FALSA desde el 05/09:
esa frase ya no esta en el `roadmap.md`** —`grep -c "sigue sin watchdog" roadmap.md` da **0**, y el
control es que `N-17` en ese mismo fichero da **5**—. La entrada se fue al historico con
`b327550`. Lo que **si** sigue en pie, y es lo que sostiene el parrafo, es la medida sobre el
propio fuente del repetidor. Y `MAPEO_TARJETA_KICAD.md` §5 lo lista como
ventaja de portar el repetidor al STM32.

**Y hay precedente escrito de que un ESP32 de este proyecto se queda clavado y tumba el enlace.**
`01_Firmware/TROUBLESHOOTING.md:48`: *"Ocurrio en el repetidor el 31/07/2026: el ESP32 levantaba
DE/RE ante cualquier byte y solo lo bajaba..."*, con el sintoma en la tabla de `:55`: **"DE/RE
clavado o ruido continuo. Bus bloqueado en ambos sentidos"**.

**Que pasa hoy si el enlace muere:** los dos STM32 se van a ambar por SFTY-6 a los
`SFTY6_SILENCIO_MS = 25000UL` (**MEDIDO** en `Maestro/include/protocolo.h:149` y
`Esclavo/include/protocolo.h:149`; son 25 s desde N-71, no los 12 s que aun dicen varios
comentarios y `ESTADO.md`).

> **Eso es seguro. No es operable.**
>
> Un cruce en ambar intermitente no mata a nadie, y por eso SFTY-6 esta bien puesto. Pero con la
> pantalla, los pulsadores y el mando retirados, **un ESP32 colgado deja el equipo sin ninguna
> superficie de mando**: no hay boton que pulsar, no hay menu que navegar, no hay mando desde el
> piso. La unica accion disponible es cortar la energia y volver a darla, y eso lo tiene que hacer
> alguien subiendo al gabinete — que es exactamente el viaje que toda esta arquitectura pretende
> evitar.
>
> **Las opciones, para que la decision se tome entre alternativas escritas y no por eliminacion**
> (`CLAUDE.md` §4: eliminar entre opciones incompletas es adivinar con tabla):
>
> | opcion | coste | que resuelve |
> |---|---|---|
> | Watchdog en el ESP32 (`esp_task_wdt`) | bajo, firmware nuevo del ESP32 | el ESP32 colgado se reinicia solo. **No** cubre el ESP32 muerto o desenchufado — 🟡 **sigue viva como complemento (Fase 5)** |
> | Un solo pulsador de servicio superviviente | un pulsador y un pin; **hereda §2.2** | da una salida fisica de ultimo recurso — 🟡 innecesaria si el mando se queda |
> | **✅ ELEGIDA (31/08) — Dejar el mando de reles del Maestro** | cero — **es no retirarlo** | conserva `A·A·A`, `B·B·B`, `A·B·A·B` y el veto de §2.4 |
> | `J2` (SWD) como consola de servicio | 🔴 **descartado**: `MAPEO_TARJETA_KICAD.md` §7 — `J2` es la unica via de carga de firmware, no se reutiliza | — |
> | ~~Aceptar el ambar como estado final y subir al gabinete~~ | cero | ⛔ **retirada de la mesa el 31/08**: era la que habia que firmar, y la decision fue no firmarla |

### 3.4 ✅ ~~🟠~~ El minimo de tiempo por sentido (N75-1) — **DECIDIDA EL 04/09/2026**

**Dueno: ~~el responsable~~ DECIDIDA (04/09).** Era una cifra y hacia falta; llego.

> ✅ **DECIDIDA POR EL RESPONSABLE EL 04/09/2026: el minimo de verde y de rojo sube de 1 a 3
> minutos.** Su motivo, literal: **«tres minutos es la minima distancia de seguridad»**. El
> desarrollo esta en la revision del 04/09 (tarde) de la cabecera, y largo en
> `modo_automatico.cpp:22-50`.
>
> **MEDIDO** en `01_Firmware/Maestro/include/limites_ciclo.h` (N-137 los mudo ahi desde
> `modo_automatico.cpp`) — `grep -n "VERDE_MIN_MIN" Maestro/include/limites_ciclo.h`:
>
> ```
>    static const uint8_t VERDE_MIN_MIN = 3,  VERDE_MIN_MAX = 15;
>    static const uint8_t ROJO_MIN_MIN  = 3,  ROJO_MIN_MAX  = 15;
> ```
>
> **Y va en el C++, no en la app**, que es la otra mitad de la decision: la app puede quedarse vieja
> y **no es la unica que puede hablar por `J17`**. El firmware rechaza con
> `$ERR,CMD:SET_TIEMPOS,DESC:RANGO` (`grep -n "DESC:RANGO" Maestro/src/bluetooth.cpp`), y el rango lo comprueba
> `modoAutomatico_fijarTiempos()` (`:57-60`), no el despachador — que solo traduce texto a numeros.
>
> ⚠️ **Coste aceptado a sabiendas: ya no se puede probar en mesa con ciclos de un minuto.**
>
> 🔴 **Y lo que la decision NO trae: nadie ha ejercido esto en una tarjeta.** Es MEDIDO sobre
> fichero, como casi todo este documento.

**Lo que decia este apartado hasta el 04/09, conservado porque una decision solo se puede revisar si
la pregunta sigue escrita:**

~~**MEDIDO:** `01_Firmware/Maestro/src/modo_automatico.cpp:31`~~

```
   static const uint8_t VERDE_MIN_MIN = 1,  VERDE_MIN_MAX = 15;      <- CADUCADO el 04/09
```

~~**El firmware admite 1 minuto.** `ESTADO.md:140` (N75-1) lo dice con todas las letras: *"Se pidio
'minimo de 3 minutos'; **no esta escrito en ninguna parte** — el firmware dice `VERDE_MIN_MIN = 1`
y la app valida exactamente lo mismo. No hay desajuste: hay una decision sin tomar, y su sitio es
el C++"*.~~ → **la decision se tomo, y su sitio era exactamente ese.**

> ✅ **Y su hermana, N75-2, se cierra con el mismo cambio.** Decia esto: *"los cuatro limites estan
> escritos **dos veces** —en `modo_automatico.cpp:31-33` y a mano en `app.js` mas los `min`/`max`
> del formulario— **sin nada que los ate**. El dia que suba el minimo, si nadie ata las dos copias,
> la app seguira dejando poner 1"*. **Eran tres sitios, no dos** —el `.cpp`, el `enRango(...)` de
> `app.js` y los `min=`/`max=` de `index.html`—, hoy los tres dicen `3`, y **hay un pack que los
> relee del fuente en cada corrida**: `app_11_rangos_de_tiempos`. Eso es lo que la nota vieja pedia
> —*"atarlo es media hora de trabajo tecnico y va en el mismo commit"*— y es lo que se hizo.
>
> 🔴 **Con un hueco medido el mismo dia: hay un CUARTO sitio y el pack no lo mira.**
> `App_Semaforo/js/config.js:10-17` declara `LIMITES_TIEMPO` con `VERDE_MIN_MIN: 1` y
> `ROJO_MIN_MIN: 1`, bajo el comentario *«Rangos de Tiempos Permitidos por Firmware»*. Esta
> ~~**cargado** por `index.html:718` y **sin un solo consumidor** fuera de `tests/`~~ → 🔴 **05/09:
> `LIMITES_TIEMPO` YA NO EXISTE** (borrado el 04/09; queda solo el comentario que lo explica). El
> `<script src="js/config.js">` sigue ahi, pero **no en ~~`:718`~~ y tampoco en un numero fijo**:
> hoy es la `949` en `App_Semaforo/index.html` y la `937` en `App_Semaforo/www/index.html`. El
> ancla es `grep -n 'src="js/config.js"' App_Semaforo/www/index.html`. Lo que sigue abierto es el
> **resto** de `IOT_CONFIG`, que tampoco tiene consumidores. Anexo, punto 6.

### 3.5 🟡 La Camara 1: se queda en `PB0`/`J14`, o se muda a `J16`

**Dueno: quien monte**, con el visto bueno tecnico.

> ⚠️ **PRIMERO, QUE «CAMARA 1» ES, PORQUE ESTE DOCUMENTO USABA EL NUMERO PARA DOS COSAS.** Aqui
> **«Camara 1» es la unidad AcuSense que hoy entra por `PB0` / bornera `J14` del MAESTRO** —el mismo
> nombre que le da `modo_inteligente.cpp:91`, y la del Esclavo es la «Camara 3»—. **No es
> `CAM_D_PIN`.** La tabla de §1.7 llegó a rotular `p10`/`p12` como «Camara 2»/«Camara 1» y **eso
> hacia esta seccion ilegible**; ya esta tachado alli.
>
> 🔵 **Y con §1.7.bis delante, esta decision cambia de naturaleza: NO ES UNA DECISION DE FIRMWARE.**
> `CAM_C_PIN` y `CAM_D_PIN` **ya son entradas de camara construidas y ejercidas**; `PB0` tambien.
> Mudar la Camara 1 de `J14` a `J16` p12 **no toca una linea de codigo**: es mover un cable de una
> bornera a otra. Lo que se decide es **riesgo de montaje**, y por eso el sesgo de abajo sigue
> valiendo entero.

| via | a favor | en contra |
|---|---|---|
| **Quedarse en `PB0` / `J14`** | 🟢 **es el unico camino de camara con firmware probado**: N-67 corregido, `pinMode(INPUT)` y `== HIGH` en las dos puntas, pack `camara_01_demanda` con 14 comprobaciones, y la placa da antirrebote de 1 ms por hardware (`R64` + `C25`) | dos borneras distintas para dos camaras del mismo poste |
| **Mudarla a `J16` p12** | prolijidad de montaje: las dos camaras en el mismo conector. 🟢 **Y desde §1.7.bis, `p12` ya tiene firmware construido y ejercido: la mudanza no es codigo, es un cable** | ~~🔴 **hereda §2.2 sin resolver**~~ → 🟢 **§2.2 CERRADA en cobre el 03/09 (M3/N-118): activo en ALTO, que es como el firmware de camara ya lee.** Este contra **desaparece**. Y 🔴 **hereda §2.1**, que NO desaparece: acerca la camara a los 12 V. **Y desde el 31/08 se sabe cuanto: `p12` (`/Boton4`) es el punto del conector MAS cercano a la red de 12 V —`1,359 mm` de cobre a cobre, `MAPEO_TARJETA_KICAD.md:576-588`—, no el mas lejano como decia el colchon de §1.7** |

> **Este documento no toma la decision, pero deja escrito el sesgo:** mover una funcion que
> **funciona con firmware probado** a un conector cuya polaridad esta en contradiccion medida y que
> reparte 12 V, para ganar prolijidad de montaje, es cambiar riesgo por estetica. ~~Si se muda, se
> muda **despues** de la medida M3 de la seccion A, nunca antes.~~
>
> 🔵 **ACTUALIZADO EL 04/09: M3 ya se hizo, y el sesgo cambia de forma — no de signo.** La polaridad
> ya no esta *"en contradiccion"*: el cobre dice **activo en ALTO**, que es como el firmware de
> camara ya lee (N-67). **Y `J16` p10 se cablo en banco y funciono** (paso 21), asi que la mudanza
> deja de ser un salto a ciegas.
>
> **Lo que NO ha cambiado, y es la mitad que decide:** `p12` sigue siendo el punto del conector **mas
> cercano a la red de 12 V** —`1,359 mm` de cobre—, `J16` p1 sigue repartiendo 12 V crudos, y
> **ninguna entrada de campo tiene proteccion en serie** (§3.6, N-120). `J14` no reparte 12 V y
> ademas trae antirrebote por hardware. **El sesgo sigue siendo quedarse en `PB0`/`J14`** — ahora por
> el riesgo de los 12 V, no por la polaridad.

### 3.6 🔴 NUEVA (04/09) — N-120: las entradas de campo no tienen ninguna proteccion, y hay que decidir si V2 se la pone

**Dueno: el responsable.** Es una decision de diseno de placa: cuesta rediseno y no la desbloquea
ninguna medida mas. Y es la unica de este apartado que **no** existia el 28/08: la trajo el banco.

**MEDIDO en el banco del 03-04/09, y es una asimetria, no un olvido puntual:**

| | cuantas | que llevan en medio |
|---|---|---|
| **Salidas** de campo | **9** | `220 Ohm` en serie **+ optoacoplador `TLP127`** |
| **Entradas** de campo | **5** | 🔴 **nada.** Del borne **directo** a la pata del STM32 |

Y en el mismo conector de dos de esas entradas, `J16` p1 reparte **12 V crudos** (§2.1).

> **Lo que eso significa en la calle, en una frase:** la tarjeta esta blindada contra lo que ella
> hace y desnuda contra lo que le hacen. Cualquier tension que aparezca en un borne de entrada —un
> cruce de hilos en el armario, un cable de camara que roza `p1`, una descarga por la linea de una
> camara que vive fuera del gabinete— entra **entera** en una pata del micro que gobierna el
> semaforo. **La proteccion de esta placa esta toda del lado por el que no entra nada.**

**La propuesta tecnica, con su cuenta hecha para que se pueda discutir el numero y no la
sensacion — `2K2` en serie por entrada:**

| | con `2K2` | por que ese numero y no otro |
|---|---|---|
| **Corriente inyectada** con 12 V en el borne | **3,6 mA** | por debajo de los **5 mA** por pata que admite el datasheet del STM32F103: el diodo de clamp la aguanta en vez de morirse |
| **Nivel ALTO que ve el micro** con la camara cerrada | **2,70 V** | el `2K2` forma divisor con el pull-down de **10 kOhm** de la placa: `3,3 x 10/12,2` |
| **Umbral `VIH`** del micro | **2,31 V** | `0,7 x VDD`. Los `2,70 V` quedan **por encima**, con margen |
| **Con `4K7` en vez de `2K2`** | 🔴 **`2,24 V` — por DEBAJO de `VIH`** | **ya no leeria la camara.** No es *"mas proteccion, mejor"*: hay un techo, y esta cerca |

**Esa ultima fila es la razon por la que esto es una decision y no una obviedad:** el margen entre
*proteger* y *dejar de leer* es de un salto de valor de resistencia. La cuenta la fija el pull-down
de 10 kOhm que la placa ya trae —el mismo que M3 midio—, asi que **cambiar uno obliga a recalcular
el otro**.

> **Lo que este documento NO decide, y va escrito:**
> - **Si esto entra en V2 o se queda como esta.** Es coste de rediseno de placa contra un riesgo que
>   nadie ha visto materializarse todavia. El dato en contra de esperar es que **ya hay una tarjeta
>   fuera de servicio** (N-116) y la causa esta abierta.
> - **SIN VERIFICAR: el `2K2` no se ha probado en ninguna tarjeta.** La cuenta es aritmetica sobre el
>   datasheet y sobre el pull-down medido; **no es una medida**. Antes de mandar a fabricar, se monta
>   sobre una entrada y se comprueba que la camara sigue leyendose.
>
> **Y lo que SI esta decidido y no espera a V2:** ~~tapar `p1` es una precaucion de banco~~ →
> **tapar `p1` es obligatorio en cada equipo**, retirando el pin del conector volante. Eso no cuesta
> rediseno, no espera a nadie y ya se hizo una vez (paso 4).

### 3.7 ✅ NUEVA Y DECIDIDA (04/09) — el cruce se opera DESDE EL MAESTRO

**Dueno: el responsable. DECIDIDA el 04/09/2026.** Es la segunda decision que este documento recoge
ya tomada, y va aqui —en el apartado de decisiones— por el mismo motivo que §3.3: **una decision
entre alternativas solo se puede revisar si las alternativas siguen escritas.**

**La alternativa que se descarto:** hacer **transparente** el mando desde el Esclavo, o sea que esa
punta aceptara `SET_MODO` y `MANUAL:CAMBIAR_TURNO` y los **relevara por radio** al Maestro, de forma
que el operario pudiera operar el cruce entero desde cualquiera de los dos postes.

**Se descarta. El cruce se opera desde el Maestro.**

**Lo que hay hoy, MEDIDO por censo de despachadores** (`grep -n 'strcmp(accion, "'`):

| punta | comandos que atiende | fichero |
|---|---|---|
| **Maestro** | `SET_MODO:AUTO` · `:MANUAL` · `:AMBAR` · `:MENU` · `:ALCANCE` · `:INTELIGENTE` · `:DEGRADADO` · `FORZAR_ROJO` · `MANUAL:CAMBIAR_TURNO` · `TEST_LEDS` · `SET_TIEMPOS` · `SET_RTC` · `REINICIAR_RELOJ` · `DEMANDA` — **14** | `Maestro/src/bluetooth.cpp:444-664` |
| **Esclavo** | `AMBAR_EMERGENCIA` · `CANCELAR_AMBAR` · `SOLICITAR_PASO`, mas `FORZAR_ROJO` y `TEST_LEDS` que **rechaza a proposito** — **5** | `Esclavo/src/bluetooth.cpp:468-561` |

**El Esclavo PIDE; no ordena** —`SOLICITAR_PASO` manda la misma demanda que la camara y **el Maestro
decide, aplica el todo-rojo y ordena**: `Esclavo/src/bluetooth.cpp:533-539`, SFTY-27—. **No hay ni
una linea que releve un `SET_MODO` por radio**, y esta decision dice que no la va a haber.

**Por que se descarta, y no es por coste de codigo:** relevar el mando por radio pone **una segunda
puerta a la maquina de estados del cruce** al otro lado de un enlace de `2.4 kbps` que el propio
equipo declara caido a los 25 s (SFTY-6). Cada orden relevada seria un `$ACK` que hay que casar con
algo que ocurre en **otra placa** — que es exactamente el defecto que N-130 acaba de cerrar en la
unica trama donde ya existia. **Una punta que pide y otra que decide es una asimetria deliberada, no
una carencia.**

> 🔴 **LA CONSECUENCIA, y es operativa: el operario tiene que saber a que poste conectarse
> ANTES de caminar.** Con dos postes que pueden estar a cientos de metros, descubrir en el sitio que
> el Bluetooth al que se conecto no atiende `SET_MODO` es una caminata perdida y un cruce que sigue
> como estaba.
>
> **Lo unico que se lo dice antes de conectar es el ROTULO Bluetooth** —`SEM-<serie>-M` para el
> Maestro y `SEM-<serie>-E` para el Esclavo—, que aparece en la lista de emparejados de Android
> **sin abrir la app**: `ESP32_Expansion/include/contrato.h:241-259`. **Asi que el rotulo deja de ser
> prolijidad de montaje y pasa a ser parte de esta decision.** Como funciona esta en el Manual 18
> §6.5; **lo que abre, en §3.8, y esta ABIERTO.**

**Lo que la app ya hace, MEDIDO** (`grep -n "SOLO_MAESTRO\|SOLO_ESCLAVO" App_Semaforo/www/app.js`): lleva la lista `SOLO_MAESTRO`
—`SET_MODO`, `MANUAL:CAMBIAR_TURNO`, `SET_TIEMPOS`, `TEST_LEDS`, `DEMANDA`, `REINICIAR_RELOJ`,
`FORZAR_ROJO`— y **avisa a que punta va cada orden** en vez de mandarla al cable contra la punta
equivocada. **La decision no obliga a tocar la app: la app ya la implementaba.**

**COSTE DE LA DECISION: cero bytes y cero lineas** — es **no construir** lo descartado.

> ⚠️ **Y el coste que SI se paga, que es de calle y por eso va escrito:** con el Maestro caido,
> inaccesible o fuera de servicio —hoy hay una tarjeta Maestro asi, N-116—, la unica superficie que
> queda en el Esclavo son sus cinco comandos: **ambar de emergencia, cancelarlo y pedir paso**. **No
> hay forma de cambiar el modo del cruce desde el Esclavo, ni por Bluetooth ni por radio.** Eso es
> lo que se ha decidido aceptar.
>
> **Y se suma a lo que §3.3 ya dejaba escrito:** el mando de reles —la otra superficie de ultimo
> recurso— **tambien vive solo en el Maestro**, porque el receptor del Esclavo no se ha comprado
> (§2.7). Las dos vias de ultimo recurso estan en la misma punta.

### 3.8 🔴 NUEVA Y ABIERTA (04/09) — un modulo virgen se llama igual en las dos puntas

**Dueno: el responsable**, porque lo que decide es lo que hace un tecnico de pie delante de dos
postes, y §3.7 acaba de apoyar en ello una decision de operacion.

**MEDIDO** en `01_Firmware/ESP32_Expansion/`:

```
   include/contrato.h:259          #define ROTULO_PROVISIONAL  "SEM-SIN-MATRICULA"
   src/transporte_app.cpp:23-31    el rotulo del arranque sale de la NVS; si no hay nada, el provisional
   src/transporte_app.cpp:37       spp.begin(rotulo)     <- el nombre se fija AL ABRIR el perfil
   src/transporte_app.cpp:105-113  el aprendido se GUARDA, y no se re-rotula en caliente
   src/puente.cpp:205-206          se aprende de paso, del $STATUS que ya se retransmite
```

**Como funciona, y por que esta bien pensado —esta parte no se discute:** la serie sale del **silicio
del STM32** (`identidad_serie()` lee el UID del micro), asi que **el ESP32 no puede saberla al
arrancar**. El puente la aprende del `$STATUS` que retransmite —sin tocarlo—, junto con el `NODE:`,
compone `SEM-<serie>-M` o `SEM-<serie>-E` y lo **guarda para la SIGUIENTE arrancada**. **No se
re-rotula en caliente a proposito**: cambiar el nombre SPP obliga a cerrar y reabrir el perfil, o sea
**a tirar la sesion del operario que en ese momento puede estar dando una orden al cruce**. Y un
`NODE:` que no se reconoce **no se rotula a medias**: `return`, sin valor por defecto
(`transporte_app.cpp:97`).

**LO QUE ABRE, y es la pregunta:** un modulo virgen —recien montado, con la NVS borrada, o con un
STM32 que todavia no ha hablado— anuncia **`SEM-SIN-MATRICULA`**. Con **dos** modulos virgenes, uno
por poste, **los dos postes se llaman exactamente igual** hasta que cada uno haya visto un `$STATUS`
**y se le haya cortado la energia**. Y §3.7 acaba de decidir que **el rotulo es lo que le dice al
operario a que poste caminar**.

> **Que hay que decidir, con las opciones escritas para que no se elija por eliminacion**
> (`CLAUDE.md` §4: *eliminar entre opciones incompletas es adivinar con tabla*):
>
> | opcion | coste | que resuelve |
> |---|---|---|
> | **Dejarlo y cubrirlo por procedimiento** — el montador da una vuelta de energia a cada modulo antes de irse y lo firma en el acta de puesta en marcha | cero firmware | resuelve el caso del montaje; **no** resuelve la NVS borrada en campo ni el modulo de repuesto que alguien enchufa un martes |
> | **Provisional distinto por modulo** — p. ej. con los ultimos bytes del MAC del propio ESP32, que si conoce al arrancar | bajo, firmware del ESP32 | **dos virgenes dejan de llamarse igual**; el rotulo sigue sin decir cual es el Maestro |
> | **Re-rotular en caliente** al aprender la matricula | bajo en lineas, 🔴 **alto en calle** | 🛑 **tira la sesion del operario que esta dando una orden.** Es justo lo que `transporte_app.cpp:109-111` evita a proposito |
> | **Que lo diga la app** en vez del rotulo: conectar a cualquiera y leer `NODE:` del primer `$STATUS` | cero firmware; la app ya recibe el campo | 🔴 **no resuelve el problema de §3.7**, que es saberlo **antes de caminar** |
>
> **Este documento no elige.** Lo que si deja escrito es que la primera opcion **es una decision**,
> no el estado por defecto: si nadie la firma, lo que hay es dos postes con el mismo nombre y nadie
> avisado.

**SIN VERIFICAR, y es la mitad que mas pesa:** **nadie ha visto este rotulo en la lista de
emparejados de un telefono.** No hay una sola tarjeta con un ESP32 conectado a `J17`, y el Bluetooth
**no subio en toda la sesion de banco del 3-4/09** (N-117 / N-122, arreglados **sin banco**).

---

> 🔴 **AMPLIADA EL 04-05/09 — EL RESPONSABLE DESCARTO LAS CUATRO OPCIONES DE ARRIBA Y APLAZO LA
> DECISION A DESPUES DEL BANCO.**
>
> Su frase sobre como se matricula hoy —mirando los **NOMBRES** de los Bluetooth— fue ***"es pura
> mierda"***, y lo que pidio son **dos** requisitos, no uno:
>
> | # | requisito |
> |---|---|
> | **1** | La matriculacion se hace por **ID de Bluetooth** (la direccion del modulo), **no por su nombre** |
> | **2** | **Sin intervencion manual.** Nadie teclea, nadie empareja a mano, nadie lee una etiqueta |
>
> **Esto no es una quinta fila de la tabla de arriba: la tumba entera.** Las cuatro opciones giraban
> sobre **como rotular**, y el requisito dice que **el rotulo no es la identidad**. La opcion 1
> —"cubrirlo por procedimiento"— es exactamente la intervencion manual que el requisito 2 prohibe.
>
> 🛑 **APLAZADO POR EL RESPONSABLE A DESPUES DEL BANCO. AQUI NO SE DISENA NADA**, y esa es la
> decision correcta esta noche: hay gente en banco y esto no bloquea ninguno de los pasos.
>
> **EL DATO DURO QUE CONDICIONA EL DISENO, Y VA ESCRITO AHORA PARA QUE NO SE PROPONGA LO IMPOSIBLE
> DENTRO DE UN MES** — MEDIDO en `Maestro/include/protocolo.h:242-248`, identico en el Esclavo:
>
> ```
>    struct RF_Packet {
>        uint8_t msgID;
>        uint8_t command;
>        uint8_t param;
>        uint8_t crc;
>    };
> ```
>
> **Son CUATRO BYTES Y NO HAY CAMPO DE DIRECCION.** El `crc` cubre **los tres anteriores**. O sea:
>
> - **el enlace de radio no sabe a quien va dirigido un paquete** — hoy funciona porque hay
>   exactamente **dos** radios en enlace directo (`CLAUDE.md` §10);
> - **una matricula que viaje por radio no cabe** sin cambiar la trama, y cambiar la trama toca
>   **las dos puntas, el CRC, la proteccion de replay y todos los packs que la modelan**;
> - **la identidad que hoy existe de verdad es la del STM32** —`identidad_serie()` lee el UID del
>   silicio— y el ESP32 **no la conoce hasta oir un `$STATUS`**.
>
> **Lo que esto NO dice:** no dice que haya que ampliar `RF_Packet`, ni que el ID de Bluetooth deba
> viajar por radio. **Dice cual es el terreno**, para que quien lo disene no descubra el limite
> despues de escribirlo. **SIN VERIFICAR:** nadie ha comprobado que la pila SPP del ESP32 exponga la
> direccion del **remoto** conectado por la interfaz que usa `transporte_app.cpp`; es la primera
> medida que hace falta y **no se ha hecho**.

### 3.9 🔴 NUEVA Y ABIERTA (04/09, de noche) — el borrado del respaldo no limpia los dos registros que N-133 estrena

**Dueno: quien decida el firmware**, y va aqui y no en el Anexo porque **lo que hay que decidir no
es como se arregla sino quien tiene la responsabilidad de limpiar** — el borrador o el lector.

**MEDIDO** en `01_Firmware/Maestro/src/respaldo.cpp` *(el fichero es identico en el Esclavo)*:

```
   :157-158   calcularSuma()      s = s*31 + leerReg(REG_CICLO_RV);
                                  s = s*31 + leerReg(REG_CICLO_DESPEJE);     <- SI entran en la suma
   :193-201   respaldo_borrar()   DR2, DR3, DR4, DR5, DR6  a cero
                                  DR9 y DR10  NO SE TOCAN                    <- y luego sellar()
```

**O sea: los dos registros nuevos entran en el checksum pero NO en el borrado, y `sellar()` los
firma como contenido valido sin haberlos limpiado.**

**Por que importa justo AHORA, y no antes:** `respaldo_borrar()` corre desde `respaldo_setup():185`
**cuando el contenido se declara invalido** — que es exactamente lo que la subida de firma de N-133
(`0x5EB1` -> `0x5EB2`) provoca en **la primera arrancada de cada equipo actualizado**. Es su caso de
uso principal, no un borde.

| escenario | que pasa hoy |
|---|---|
| equipo nuevo o pila agotada | `DR9`/`DR10` valen cero; `respaldo_tiemposCiclo()` se niega a devolver un cero (`:249`) y el equipo cae a sus minimos. ✅ **correcto** |
| **equipo ACTUALIZADO, pila buena** | `DR9`/`DR10` llevan lo que dejara el arranque anterior. Se **sellan como validos** y el lector los devuelve. 🔴 **el caso sin cubrir** |

**Lo unico que hoy lo frena es una segunda barrera en otro fichero:** la revalidacion de rango de
`modo_automatico.cpp:110-117` (`3..15 / 3..15 / 10..90`). **Un par de bytes que cayera dentro de las
tres ventanas pasaria**, y el cruce arrancaria con un ciclo que nadie configuro.

> 🔴 **Y esto contradice por escrito lo que el propio header promete**, `Maestro/include/respaldo.h:58-61`:
> *"respaldo_setup() encuentra el contenido invalido y borra: entonces respaldo_tiemposCiclo()
> devuelve false y el equipo arranca con sus minimos, que es la direccion segura"*. **La frase
> describe un borrado que no ocurre.** Es una afirmacion sobre el codigo, escrita al lado del
> codigo, que **nadie comprobo** — la misma forma de N-122.

> **Que hay que decidir, con las opciones escritas para que no se elija por eliminacion:**
>
> | opcion | coste | que resuelve |
> |---|---|---|
> | **Anadir `DR9`/`DR10` a `respaldo_borrar()`** | dos lineas | cierra el hueco donde nacio. Es el simetrico de lo que ya hace con los otros cinco |
> | **Que el lector exija ademas una bandera propia**, como `respaldo_hayCiclo()` hace con `FLAG_CICLO` | una bandera mas en `DR4` | distingue *"no configurado"* de *"borrado a medias"*, que hoy no se distinguen |
> | **Dejarlo y confiar en la revalidacion de rango** | cero | 🔴 **apoya una propiedad de seguridad en un fichero que no sabe que la sostiene**, y el header seguiria mintiendo |
>
> **Este documento no elige.** Lo que si deja escrito es que **la tercera es una decision**, no el
> estado por defecto.

**Y hay una segunda mitad, del mismo censo, mas pequena pero de la misma familia:** el comentario de
`respaldo.cpp:228` justifica el rechazo del cero diciendo que si no *"`respaldo_hayTiemposCiclo()`
mentiria"*. **Esa funcion no existe en el arbol.** La analoga que si existe es `respaldo_hayCiclo()`
(`:254`), y es la del **Degradado**, no la del automatico. Un comentario que nombra una funcion
inexistente es una nota que ya no se puede comprobar leyendo — y es como se llega a documentar una
Caja Negra que nadie llama.

> ⚠️ **Nada de esto se ha ejercido: es MEDIDO sobre fichero.** Y **no hay ningun pack que mire el
> borrado**: `Validacion_Respaldo` compila `calcularSuma()` y Horner, y su punto ciego declarado es
> justamente que **no ejerce el arranque** (`CLAUDE.md` §8). **Antes de tocar una linea aqui va el
> arnes, visto fallar** — uno que naciera en verde no mediria nada.

---

---

### 3.10 🔴 NUEVA Y ABIERTA (05/09) — hay TRES canales de potencia libres, y gastar uno cierra una puerta

**El censo de cobre del 05/09** (`2_Manual_Hardware_y_Pruebas.md` §11) encontro que `J9`
(`VERDE_PEATON`, `PA7`), `J11` (`ROJO_PEATON`, `PA6`) y `J13` (`BUZZER`, `PB1`) no son "pines
sueltos": son **tres etapas de potencia completas y fabricadas**, identicas a la de la talanquera de
`J15` **que si funciono en banco el 04/09**. Opto `TLP127`, MOSFET `IRLZ44N`, diodo de rueda libre,
LED testigo y bornera. Encender uno cuesta **16 B de flash** de suelo, medidos por desensamblado.

**Lo que hay que decidir no es tecnico, es de alcance:** son **los tres unicos canales de potencia
libres de esta placa**. El que se gaste **ya no esta** para lo que llevaba escrito encima.

| si se gasta en… | lo que se pierde a cambio |
|---|---|
| `J9` / `J11` — la pareja peatonal | **la cabeza peatonal de este cruce**, que es para lo que estan rotulados desde el dia uno |
| `J13` — el zumbador | **el aviso acustico**, que es la unica salida no visual del equipo |
| nada (se dejan como estan) | cero coste, y **tres canales fabricados sin usar** en una placa que ya no tiene mas moldes libres |

> **Este documento NO elige, y no por prudencia: porque no hay ninguna decision escrita que renuncie
> a ellos.** `DECISIONES.md` no tiene ni una fila sobre los peatonales ni sobre el zumbador. Lo que
> se ha escrito hasta hoy —en este mismo documento y en el Manual 2— es que **estan MUERTOS en el
> firmware**, que es una descripcion del estado, **no una renuncia**. Confundir las dos cosas es como
> se derogan decisiones de palabra (`CLAUDE.md` §2.quinquies).

**Lo que hace falta para poder decidirla:**

1. 🟡 **`SIN VERIFICAR`: que `J9`, `J11` y `J13` esten REALMENTE SOLDADOS.** El esquematico los marca
   `in_bom=yes`, `dnp=no`, `on_board=yes`, pero **nadie los ha mirado en cobre**. Lo mas cerca que hay
   es `J15`, el gemelo, que si funciono. Eso lo hace probable; **no lo demuestra**. Es una inspeccion
   a ojo mas continuidad, y va **antes** que la decision.
2. **Si el cruce lleva o no paso peatonal.** Es una pregunta de obra, no de firmware, y la contesta el
   responsable.
3. **Que quede claro que lo que se cuelgue ahi comparte la masa del controlador** —hay una sola red
   `GND` en la tarjeta— **y que el borne esta a ~12 V en reposo**, no a 0 V. Las dos cosas cambian que
   se puede conectar. Detalle en `2_Manual_Hardware_y_Pruebas.md` §11.2 y §11.4.

> ⚠️ **Y una cuarta cosa que NO es parte de esta decision pero se cruza con ella:** los **otros tres**
> pines libres —`PB3`, `PB4`, `PB5`, los del LCD— **no** traen etapa de potencia, pero **si traen
> bornera ya cableada** (`J17` p4, p1 y p5). Si lo que hace falta es una **entrada** o una senal de
> nivel logico, esos son el sitio y **no cuestan ninguno de los tres canales**. Si lo que hace falta
> es **mandar 12 V a algo**, no sirven. Son dos preguntas distintas y conviene no mezclarlas.

---

## A. Las cinco medidas de multimetro, en orden

> 🔴 **El motivo por el que esta seccion existe: hoy no hay ni una fila «VERIFICADO EN LA PLACA» en
> todo el mapeo de la tarjeta.** `MAPEO_TARJETA_KICAD.md` §0 y §9 lo declaran, y sigue siendo cierto
> el 28/08. Todo lo que sabemos del cobre sale de un dibujo.
>
> ✅ **ACTUALIZADO EL 04/09: cuatro de las cinco se ejecutaron en el banco del 03-04/09.**
>
> | | estado | donde |
> |---|---|---|
> | **M1** — cual es `J16` y cual `J17` | ✅ **HECHA** (paso 3): `J16` p1 da 12 V, `J17` p1 no | informe §3.2 |
> | **M2** — `J17` sin 12 V, y p2/p3 = `PB7`/`PB6` | ✅ **HECHA** (paso 5): continuidad a las patas 43 y 42, **ni un pin por encima de 3,3 V** | informe §3.2 |
> | **M3** — la polaridad de los pines de boton | ✅ **HECHA, y es la que mas cambio** (paso 20) — ver abajo | informe §3.7 |
> | **M4** — los 12 V de `J16` p1 | ✅ **HECHA** (pasos 3 y 4): 12 V confirmados, y `p1` **retirado del conector volante** | informe §3.2 |
> | **M5** — masa comun del ESP32 y reposo de su TX | ✅ **HECHA** (paso 23): **0 V** entre masas —por debajo del umbral de 50 mV— y `GPIO17` en **3,3 V**, no 5 V | informe §3.8 |
>
> **Las cinco se anotan en `MAPEO_TARJETA_KICAD.md` §9 con su fecha, que es lo que esta seccion pedia
> desde el 28/08.** Ese fichero **no lo toca este documento** — va en la lista de la seccion B.

**Las tres primeras van con la tarjeta SIN ENERGIA. Las dos ultimas con energia, y antes de unir los
dos equipos.** Cada una se anota en `MAPEO_TARJETA_KICAD.md` §9 con la fecha, para que empiece a
haber filas de ese nivel.

### M1 · Cual de los dos conectores es `J16` y cual `J17`

**Por que es la primera:** `J16` y `J17` **comparten footprint** —`Molex_KK-254_AE-6410-16A_1x16`,
16 pads los dos, MEDIDO en el `.kicad_pcb`— y a la vista son identicos. Lo avisa
`10_Manual...:250-256`. **`J16` reparte 12 V; `J17` no.** Confundirlos es meter 12 V donde va el
ESP32.

| que se mide | como | esperado |
|---|---|---|
| p1 del conector contra el borne **positivo** de `J1` | continuidad (pito) | **`J16` PITA · `J17` NO** |
| p7 y p9 contra `GND` | continuidad | **pitan en `J17`** |

🔴 **Si pitan las dos cosas en el mismo conector, se para.** El mapa no describe esta placa y nada
de lo que sigue vale.

### M2 · Que `J17` no tiene 12 V en ninguna posicion, y que p2/p3 son `PB7`/`PB6`

Es la medida que protege al ESP32 y a las patas 42 y 43 del `U1`.

| que se mide | como | esperado |
|---|---|---|
| `J17` p2 al pin **43** del `U1` | continuidad | **pita** (`PB7`) |
| `J17` p3 al pin **42** del `U1` | continuidad | **pita** (`PB6`) |
| cada posicion de `J17` contra `GND`, **con energia** | tension DC | **0 V o 3,3 V, y ni una por encima**. `J17` p6 y p8 = 3,3 V |

🔴 **Si aparece 12 V en cualquier posicion de `J17`, el ESP32 no se enchufa** hasta aclararlo.

### M3 · La polaridad de los pines de boton — ~~**la que desbloquea las camaras**~~ ✅ **HECHA EL 03/09. RESULTADO ABAJO**

> ✅ **EJECUTADA en el paso 20 de la Guia, el 03/09/2026. El resultado es la primera columna: «la
> placa es la del netlist».** La receta que sigue se conserva entera —no se borra una medida por
> haberla hecho una vez: hay una segunda tarjeta, y habra mas—.
>
> **MEDIDO EN COBRE, conector vacio:**
>
> | `J16` | R a masa | R a `3,3 V` | V contra masa (con energia) | veredicto |
> |---|---|---|---|---|
> | p5 (`PB9`) | **9,92 kOhm** | 11,28 kOhm | 🔴 **0,6 V** | **el netlist tiene razon** |
> | p8 (`PB13`) | **9,92 kOhm** | 11,28 kOhm | 🔴 **0,6 V** | idem |
> | p10 (`PB14`) | **9,93 kOhm** | 11,29 kOhm | **0 V** | idem |
> | p12 (`PB15`) | **9,94 kOhm** | 11,31 kOhm | **0 V** | idem |
>
> **Los cuatro dieron lo mismo, que es lo que esta seccion pedia comprobar** —y aqui *«lo mismo»* no
> es el resultado tranquilizador que parece: significa que **la placa trata igual a los cuatro** y
> por tanto el firmware **no** puede leer dos de ellos al reves de los otros dos. Los `0,6 V` de p5 y
> p8 contra los `0 V` de p10 y p12 eran la huella del pull-up interno que `botones.cpp` activaba **y
> que no debia activar**. 🟢 **Ya no lo activa: `346ea5f` deja los cuatro en `INPUT` pelado**, asi
> que una repeticion de M3 sobre una tarjeta con firmware de hoy **debe dar `0 V` en las cuatro
> posiciones**. Es la comprobacion mas barata de que el arreglo entro, y **todavia no se ha hecho**.
>
> **La fila «~0,66 V» de la tabla de interpretacion de mas abajo es la que se cumplio.** Ver §2.2 y
> la revision del 03-04/09 de la cabecera para lo que eso decide: 🟢 camara desbloqueada · 🔴 mando
> `A`/`B` inoperante (N-118).
>
> ⚠️ **Lo que M3 NO midio, y no se da por medido:** el voltaje de p5/p8 **con el puente puesto**. Ese
> dato es el que faltaba del paso 29 y se perdio con el incidente de N-116. Sigue pendiente, **y no
> sobre la tarjeta Maestro** mientras siga con el corto.

Es la medida que cierra la contradiccion de §2.2. Sin ella no se cablea camara a `J16`.

Con la tarjeta **sin energia** y **nada enchufado en `J16`**, ohmimetro:

| que se mide | esperado si la placa es la del netlist | esperado si el firmware tiene razon |
|---|---|---|
| `J16` p5 (`PB9`) a `J16` p2 (`GND`) | **10 kOhm** | circuito abierto |
| `J16` p5 a `J16` p4 (`3,3 V`) | circuito abierto | **10 kOhm** |

Repetir en **p8, p10 y p12**. Los cuatro tienen que dar lo mismo; si uno difiere, ese es el
hallazgo.

Y despues, **con energia** y `J16` vacio, tension de p5/p8/p10/p12 contra `GND`:

| lectura | que significa |
|---|---|
| **~0,66 V** | pull-DOWN de 10 k contra el pull-up interno de ~40 k. **El netlist tiene razon y el firmware esta invertido** — es N-67 en los botones |
| **~3,3 V** | pull-UP. El firmware tiene razon y el netlist no describe esta placa |
| otra cosa | ni una ni otra: se anota el numero y se para |

> **`0,66 V` es una cuenta, no una medida:** `3,3 x 10/(10+40)`, con el pull-up interno tipico del
> STM32F103. El umbral `VIL` del micro es `0,3 x VDD = 0,99 V`, asi que `0,66 V` se lee **LOW**. La
> cuenta es de `roadmap.md` N-67 y se reproduce aqui para que el que mide sepa **que numero espera
> antes de mirar la pantalla del multimetro**, que es la unica forma de que la medida signifique
> algo.

### M4 · Los 12 V de `J16` p1, y cuanto hay entre ellos y los pines de camara

Con energia:

| que se mide | esperado |
|---|---|
| `J16` p1 contra `GND` | **12 V** (el rail crudo — sin opto, sin limitadora, sin clamp) |
| `J16` p2 contra `GND` | 0 V |
| `J16` p4/p7/p9/p11 contra `GND` | 3,3 V |

Y **antes de energizar con algo enchufado**, `p1` va tapado (§2.1). ~~La separacion fisica MEDIDA con
el paso de 2,54 mm del footprint: **10,2 mm** a `p5`, **22,9 mm** a `p10`, **27,9 mm** a `p12`.~~

> 🔴 **REFUTADO el 31/08: eso es la distancia entre PADS del conector, no la separacion entre las
> redes.** Cobre a cobre —pistas y vias incluidas, `MAPEO_TARJETA_KICAD.md:576-588`— la red de 12 V
> pasa a **1,405 mm** de `/Boton1`, **1,408 mm** de `/Boton2`, **4,269 mm** de `/Boton3` y **1,359 mm**
> de `/Boton4`. **`p12` es el peor punto del conector, no el mejor.** Ver §1.7.
>
> **Consecuencia para esta medida M4:** el multimetro entre `p1` y los otros pines **no puede ver
> esto** —mide continuidad y tension, no distancia—, asi que M4 no lo confirma ni lo desmiente. Lo
> que M4 sigue haciendo es lo suyo: confirmar que `p1` trae 12 V de verdad. El margen de 1,36 mm es
> una razon **mas** para tapar `p1`, no menos.

### M5 · La masa comun del ESP32 y el nivel de reposo de su TX

Con **las dos fuentes encendidas** y **los hilos de datos todavia sin unir**:

| que se mide | como | esperado |
|---|---|---|
| masa del ESP32 contra masa de la tarjeta | tension DC | **< 50 mV** |
| `GPIO17` (TX2) del ESP32 contra la masa comun | tension DC | **3,3 V** (linea serie en reposo alta) |

🔴 **Si hay tension apreciable entre las dos masas, no se unen los datos.** Esa diferencia entra
entera por `PB6`/`PB7`, que son patas del micro que gobierna el semaforo. Y si `GPIO17` en reposo
diera **5 V**, el modulo no es el que se cree que es: se para y se identifica antes de conectar.

> **Lo que estas cinco medidas NO cubren, y va escrito al lado:** ninguna de ellas dice nada del
> `Y2` de cada tarjeta (§3.2, `ESTADO.md` `B5`), ni de a donde sale de verdad `PB2`
> (`ESTADO.md` `B3`: *"que `Puerta` salga del pin `MOTOR_TALANQUERA` y llegue al borne"*), ni del
> nombre real del pin 3 de `J17` (§1.4). Son tres comprobaciones mas, con la tarjeta delante, y
> **no se resuelven con multimetro sino siguiendo hilos y leyendo serigrafias.**

---

## B. Que documentos quedan FALSOS, y en que orden hay que tocarlos

El criterio del orden no es el gusto: **primero lo que hace salir dinero, despues lo que promete una
salida de emergencia que ya no existe, al final el acta que se firma.**

> **Ninguno de estos ficheros se ha tocado al escribir este documento.** Lo que sigue es el censo,
> no el arreglo.

### Orden 1 · 🔴 `05_Funcional/15_Lista_de_Compras_Hardware.md` — **hay dinero a punto de salir**

**Va primero porque es el unico de la lista cuyo dano es irreversible en cuanto alguien pague.**

| linea | que manda comprar | por que es falso ahora |
|---|---|---|
| `:110`, `:127`, `:213` | **2 modulos Bluetooth SPP `HC-05`/`JDY-30`** | **el modulo SPP se retira.** Lo sustituye el ESP32 |
| `:217-218` | *"Los modulos Bluetooth siguen en la lista aunque hayan llegado ESP32. **Decision de obra del 28/08: se va con el modulo SPP dedicado**"* | 🔴 **es la decision contraria a la del 28/08.** El mismo dia, el mismo documento |
| `:164`, `:222` | **`DS3231` `ZS-042`** *"solo si el cristal muerto es el del Maestro"*, para la placa | el `DS3231` ya no va a la placa: **va al ESP32** por `GPIO21`/`GPIO22`. Cambia de destino, de manual y de criterio de compra |
| `:206` | fila del repetidor ESP32 | hay que separar tres ESP32 distintos en el papel: el **puente de radio** del Manual 5, los **modulos llegados el 28/08** y el **ESP32 de expansion** de este documento. Hoy dos de los tres se llaman igual |

Lo que **sigue siendo cierto** en ese documento y no hay que tocar: las 2 camaras AcuSense de
demanda, las 2 antenas VHF con sus coaxiales, y el aviso de `:180-181` de que **el `DS3231` no tiene
driver en el firmware de hoy** — eso es cierto y se vuelve mas importante, no menos, porque ahora el
driver hay que escribirlo en el ESP32.

### Orden 2 · 🔴 `05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.md` — congelado, y manda enchufar un `HC-05` en `J17`

| linea | que dice | por que es falso ahora |
|---|---|---|
| `:132-135` | *"**Decision de obra del 28/08: se sigue con el modulo SPP dedicado.** Se instala `HC-05`/`JDY-30`, no ESP32"* | 🔴 contraria a la arquitectura de este documento |
| `:221-241` | el cableado del **`HC-05` a `J17`** p2/p3 | **`J17` p2/p3 es donde va el ESP32.** El pinout es correcto; el modulo que manda enchufar, no |
| `:142-146` | la tabla de tres caminos con su columna *"Estado de este apartado 1"* | **hay que decidir la fila y anotarlo**, no elegirla en silencio |

> 🛑 **El apartado 1 esta congelado por escrito (`:26`, `:148`) y su reapertura es OBLIGATORIA si el
> modulo no es un ESP32 clasico.** No es burocracia: el valor de una decision congelada esta en que
> reabrirla cueste. Si los modulos son `S3`, `C3` o `S2`, **hay que rehacer el transporte de la app
> entero** y eso se decide antes de comprar y antes de escribir una linea. Ver §3.1.
>
> **Y hay una via intermedia que el propio manual ya deja abierta (`:145`): un ESP32 clasico
> haciendo de puente SPP deja el apartado 1 INTACTO.** Si la serigrafia dice `WROOM-32*`, esta
> arquitectura entra **sin reabrir nada** — solo hay que confirmar la referencia y la fuente propia.
> Es la razon practica por la que §3.1 va antes que todo lo demas.

### Orden 3 · ~~🔴~~ 🟡 `05_Funcional/8_Procedimiento_Modo_Degradado.md` — ~~sus cuatro vias son botones retirados~~ **tres de sus cuatro vias SOBREVIVEN**

> 🟢 **REBAJADO EL 31/08.** Este apartado daba las cuatro vias por muertas. Con el mando conservado
> en `A` y `B` (§1.6), **solo cae la cuarta** —la entrada por pantalla—. `A·B·A·B`, `A·A·A` y `B·B·B`
> se ejecutan igual que hoy, y ademas ahora hay una **quinta** via nueva que este documento negaba:
> `SET_MODO:DEGRADADO` por Bluetooth (`grep -n 'strcmp(accion, "SET_MODO:DEGRADADO")' Maestro/src/bluetooth.cpp`, MEDIDO el 31/08 y revalidado el 05/09).
>
> **La tabla de averias de `:311-312` vuelve a tener accion que ejecutar**, que era el agujero grave
> de esta ficha: `B·B·B` sigue siendo la primera accion ante los dos escenarios de riesgo residual.
>
> Lo que sigue debajo se conserva como estaba escrito el 28/08.

**MEDIDO** sobre el documento: la entrada desde el piso es `A · B · A · B` en menos de 18 s
(`:119`, `:295`); la salida a Automatico es `A · A · A` (`:296`); la vuelta a ambar es `B · B · B`
(`:297`); y la entrada por pantalla usa los pulsadores. **Las cuatro desaparecen.**

Y la tabla de averias del `:311-312` manda `B·B·B` como **primera accion** ante los dos escenarios
de riesgo residual —las dos puntas desfasadas, y una en verde con la otra en ambar—. Con el mando
retirado, ese procedimiento **no tiene accion que ejecutar**.

> ~~**Este documento no se puede «actualizar» hasta que exista la via de sustitucion.** Hoy **no hay
> ningun comando de Bluetooth que entre ni salga del Modo Degradado** (§2.3). Reescribirlo antes de
> que exista seria describir un procedimiento que nadie puede ejecutar — que es peor que dejarlo
> desfasado, porque un procedimiento desfasado se nota y uno inventado no.~~
>
> 🟢 **REFUTADO el 31/08 y el bloqueo se levanta, por partida doble.** La via de sustitucion **existe
> en firmware** —las ramas `strcmp(accion, "SET_MODO:DEGRADADO")`, entrada, y `strcmp(accion, "SET_MODO:MENU")` de `bluetooth.cpp`, que
> en Degradado pide la salida por el todo-rojo (`:196-205`)— **y ademas las vias originales de mando
> no se han ido**. Este documento ya se puede actualizar: lo que hay que reescribir es **una** de sus
> cuatro vias, no las cuatro.
>
> **La cautela que sigue en pie, y que se hereda tal cual:** nada de esto ha pasado banco, asi que el
> procedimiento reescrito describe lo que el firmware **dice** hacer, no lo que se ha visto hacer. Se
> marca asi dentro del propio documento.

### Orden 4 · ~~🔴~~ 🟢 `04_Manuales/MANUAL_MANDO_4_RELES.md` — ~~vende `B·B·B` como salida de emergencia~~ **SALE DE LA LISTA DE FALSOS**

> 🟢 **RETIRADO DE ESTA LISTA EL 31/08: este manual vuelve a ser VIGENTE.** El mando se conserva en
> `A` y `B` (§1.6), asi que `A·A·A`, `B·B·B` y `A·B·A·B` siguen existiendo y el manual sigue
> describiendo el equipo. **No hay que marcarlo retirado.**
>
> **Lo unico que hay que corregir dentro, y es poco:** el manual describe **cuatro** canales; el
> equipo va a tener **dos**. Los canales `C` y `D` del receptor quedan sin destino porque `PB14` y
> `PB15` pasan a camaras. Ninguna secuencia documentada los usa —MEDIDO: `grep "BOTON[1-4]"
> Maestro/src/mando.cpp` da CERO—, asi que el cambio es de inventario, no de procedimiento.
>
> Y la advertencia de `:352` —exigir **codigo independiente por unidad** al comprarlo— **sube de
> importancia**, no baja: si el mando es ahora la salida de ultimo recurso (§3.3), un mando que abre
> dos cruces es un fallo mas caro que antes.
>
> **Lo que sigue debajo es el analisis del 28/08, conservado. Su premisa —«con el mando retirado»—
> ya no se cumple.**

~~`:316-318`: *"**`B·B·B` devuelve a ambar desde cualquier estado en marcha, sin condiciones.** ... una
salida de emergencia con requisitos no es una salida de emergencia"*.~~

~~Es una promesa fuerte y esta bien argumentada. **Con el mando retirado deja de existir**, y su
desaparicion es exactamente §3.3: el equipo se queda sin salida de emergencia fisica.~~ → **La
promesa se cumple y se queda.** Y el argumento del manual es, literalmente, el que gano la decision
de §3.3.

Ademas `:96-102` documenta el veto de §2.4 —*"En el Esclavo, `B·B·B` desobedece al Maestro a
proposito"*—, que es la parte que **no** queda inerte al retirar el mando. **Sigue vigente y sigue
activo** —los tres `if` negados: `grep -n "mando_ambarLocal" Esclavo/src/main.cpp`—.

> ~~**Este manual no se borra: se marca retirado y se conserva.**~~ → **Ni se borra ni se marca
> retirado: se corrige el numero de canales y se queda.** La razon por la que `B·B·B` existia sigue
> siendo valida — y el 31/08 alguien pregunto *"¿y si el ESP32 se cuelga?"* y la respuesta fue
> quedarse con `B·B·B`.

### Orden 5 · 🔴 `05_Funcional/3_Protocolo_Pruebas_Rigurosas.md` — **el acta que se firma**

Va el ultimo de los urgentes **a proposito**: es el documento que recoge las consecuencias de los
otros cuatro, y reescribirlo antes de que los otros esten decididos garantiza reescribirlo dos
veces.

**49 de sus 80 pruebas dejan de ser ejecutables** (§2.8). El bloque de `RESUMEN DE RESULTADOS`
(`:799-820`) tiene los totales por seccion escritos a mano, y **el `TOTAL ___ / 80` es lo que se
firma**. Un acta con 49 casillas que nadie puede rellenar no es un acta incompleta: es un acta que
invita a rellenarlas igual.

> ⚠️ **Y la trampa concreta que hay que evitar, porque este repositorio ya la tiene escrita
> (`CLAUDE.md` §8.quater):** al reescribirlo, la tentacion es tachar las 49 en bloque hasta que la
> cuenta cuadre. Van una por una, y cada una acaba en uno de tres sitios, **anotado**: **se borra**
> (solo probaba el gesto retirado), **se invierte** (pasa a exigir el comportamiento nuevo por la
> app), o **se conserva** (media otra cosa y sigue valiendo). La 11.4 —*"Inmunidad e Independencia
> de los Botones del Panel LCD"*— es el caso mas claro de **inversion**: hoy exige que las camaras
> no interfieran con los botones; manana tiene que exigir que las camaras **funcionen en los pines
> que eran de los botones**.

### Segundo bloque · lo que tambien queda falso, sin dinero de por medio

| documento | que queda falso |
|---|---|
| `03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md` | **§6, §7:** `J16` deja de ser el conector de botones. **Y ver el recuadro de abajo: su §0 esta desmentido por medida** |
| `05_Funcional/1_Manual_Usuario.md` | toda la operacion por pantalla y botones; camaras en `PB0`/`PB8` |
| `05_Funcional/2_Manual_Hardware_y_Pruebas.md` | ensamblaje, borneras, pila RTC en `VBAT` |
| `05_Funcional/9_Manual_Parametrizacion_Camara_IA.md` | contactos `1A`/`1B` en `PB0` y `PB8` |
| `04_Manuales/MANUAL_CONFIGURACION_CAMARAS_IA.md` | el pinout de camaras |
| `05_Funcional/11_Manual_Instalacion_RTC_DS3231_Bateria.md` | el `DS3231` se muda al ESP32 |
| `05_Funcional/13_Manual_Modulo_Expansion_I2C_y_Compras.md` | su §4 entera: las rutas de bus I2C sobre el STM32 dejan de hacer falta si el I2C vive en el ESP32 |
| `04_Manuales/MANUAL_CONFIGURACION_BLUETOOTH.md` | comandos AT de `HC-05` |
| `05_Funcional/14_Manual_App_Movil_IOT_VIAL.md` | la app pasa de accesorio a **unica interfaz** |
| `ESTADO.md` | §2 dice `USART1` en `PA9`/`PA10` (ya es `PB6`/`PB7`) y *"SFTY-6 a los 12s"* (son **25 s**: `grep -n "define SFTY6_SILENCIO_MS" Maestro/include/protocolo.h`) |
| `OPTIMIZACIONES.md` | la trazabilidad `SFTY-x -> codigo -> prueba`, sobre todo SFTY-21 (§2.4) |
| `CLAUDE.md` | §6 sigue valiendo entera; §10 y el mapa de instrumentos hay que revisarlos |

> 🟢 **Y un hallazgo MEDIDO sobre el mapeo, que va aqui porque cambia el peso de la evidencia de
> este documento:**
>
> `MAPEO_TARJETA_KICAD.md` §0 afirma: *"El `.kicad_pcb` de este proyecto **esta vacio**, asi que
> entre el esquematico y la tarjeta que hay encima de la mesa no existe ningun artefacto que las
> ate"*. **Es falso para el fichero bueno.** Censo del
> `01_Firmware/Controladora_Semaforos/Controladora_Semaforos/Controladora_Semaforos.kicad_pcb`
> (2 158 421 bytes):
>
> | | |
> |---|---|
> | `(footprint` | **185** |
> | `(segment` | **1 447** |
> | `(via` | **89** |
> | `(zone` | **2** |
>
> **Es una placa ruteada, no un fichero vacio.** Los `.kicad_pcb` vacios de verdad —**78 bytes**—
> estan en `99_Legacy/Controladora_Semaforos-backups/`. Es la regla del instrumento otra vez: **el
> buscador miro el fichero equivocado**, igual que en N-64. Y `ESTADO.md` **ya lo sabe** —su mapa de
> artefactos dice *"649 KB ... y el `.kicad_pcb` de 2,1 MB"*—: los dos documentos se contradicen
> hoy.
>
> **Lo que esto NO significa:** que el netlist describa la placa que hay sobre la mesa. Un `.pcb`
> ruteado dice lo que se **envio a fabricar**; sigue sin decir lo que alguien **reparo despues**, ni
> si la unidad de banco salio de esa tirada. **La seccion A sigue siendo obligatoria.** Lo que
> cambia es que las medidas de §2.1 y §2.2 dejan de salir de un dibujo y salen de un **netlist
> ruteado**, que es un escalon mas arriba — y por eso la contradiccion de §2.2 pesa mas, no menos.

---

## C. Lo que este documento NO mide, y nadie debe dar por medido

Se escribe explicito porque un documento de arquitectura que no marca sus bordes se lee como un
permiso.

| | |
|---|---|
| ~~**Nada del cobre**~~ → **casi nada del cobre** | ✅ **el banco del 03-04/09 dejo las primeras filas medidas**: `J16` p5/p8/p10/p12, `J17` p2/p3 y sus tensiones, `J14`, `J15` y las masas del modulo. **Todo lo demas de esta tarjeta sigue siendo netlist y esquematico** — ⚠️ **y el censo del 05/09 NO cambia esto: es lectura del `.kicad_pcb`, no punta sobre la placa.** Lo que si hace es decir **que** hay que medir, y esta abajo en cuatro filas nuevas |
| ~~**El chip que llego a obra**~~ | ✅ **CERRADO: `ESP32-WROOM-32` clasico**, BR/EDR + BLE. Era lo mas barato y lo mas bloqueante, y ya no bloquea |
| **El pico de 500 mA del ESP32** | ESCRITO en el Manual 15, no medido sobre el modulo real. 🔴 **Y sigue sin medirse por un motivo nuevo: en banco el modulo se alimento por USB, no por la fuente `12 V -> 5 V` de la placa definitiva** (paso 22, parcial) |
| ~~**Que el enlace `J17` funcione**~~ | ✅ **el enlace fisico SI:** continuidad a las patas 42/43, masa comun por debajo de 50 mV, `GPIO17` en 3,3 V y el montaje definitivo encendido sin calentamiento ni reinicios (pasos 5, 23 y 24). 🔴 **Lo que sigue sin verificarse es que por ese enlace hable alguien**: el Bluetooth no subio en toda la sesion (N-117 / N-122, arreglados **sin banco**) |
| **El `Y2` de la segunda tarjeta** | N-37 midio uno. El otro sigue sin diagnosticar (`ESTADO.md` `B5`). 🔴 **Y el paso 27 —«el reloj conserva la hora»— quedo BLOQUEADO**: la unica via de consultar el `DS3231` es `SET_RTC` por Bluetooth, que no subio |
| ~~**Que las camaras funcionen en `PB14`/`PB15`**~~ | 🟡 **a medias, y hay que decir cual mitad.** ✅ **el cableado si**: `p10` cablado contra `p11` en normalmente abierto, `0 V` en reposo, **sin demandas fantasma con cable y sin el** (pasos 20 y 21). 🔴 **La concesion de paso NO**: depende del Modo Automatico, que no se pudo seleccionar sin app |
| **El firmware del ESP32** | ~~**no existe**~~ → **existe, compila y se cargo sin errores** (`01_Firmware/ESP32_Expansion/`), con su `DS3231` por `GPIO21`/`GPIO22`. 🔴 **Lo que no esta demostrado es que funcione**: el modulo no se anuncio de forma fiable en el telefono en toda la sesion |
| **La regresion N-42** | el Modo Automatico no mueve las luces en banco, y **sigue abierta**. 🔴 **El banco del 03-04/09 NI la confirmo NI la descarto** —el equipo nunca llego a operar, porque se queda esperando seleccion de modo y la app no conecto—. **Un ABORTADO no es un PASS**: sigue siendo lo primero de la proxima sesion |
| 🔴 **La causa de N-116** | la tarjeta Maestro tiene un corto entre `3,3 V` y `GND` **medido**, y **la causa esta abierta**. El firmware queda descartado por censo —ninguna de las salidas de potencia toca un pin de `J16`; ~~9~~ **son 10, censadas el 05/09**—, y eso **no nombra a nadie mas** |
| 🔴 **Que el mando `A`/`B` funcione con la polaridad corregida** | ~~la correccion de `botones.cpp` **no esta escrita ni cargada**~~ → 🟢 **escrita el 04/09 en `346ea5f`, las dos puntas.** 🔴 **NO cargada y NO ejercida**, y el unico intento de pulsarlo acabo en el incidente de N-116. **Nadie ha visto nunca a este equipo obedecer un `A·A·A`.** El gesto de la proxima prueba es `p5`-`p4` y `p8`-`p7`, **no contra masa**, y no sobre la Maestro |

| 🔴 **Que el minimo de 3 minutos aguante en la tarjeta** | el cambio esta **MEDIDO en el fuente** (`Maestro/include/limites_ciclo.h`, constante `VERDE_MIN_MIN`) y **no se ha cargado en ninguna tarjeta**. Trae ademas un coste de banco declarado: **ya no hay ciclos de un minuto para probar en mesa**, y la proxima visita tiene que contar tres minutos por paso |
| 🔴 **Que N-130 se VEA desde la app** | el rechazo llega como evento `MAESTRO / DEMANDA_NO_ATENDIDA_MODO_ACTUAL` (`Esclavo/src/main.cpp:542`), no como `$ERR`. **Que la app lo pinte y el operario lo lea NO se ha comprobado**, y sin eso el cierre es medio: se deja de mentir, pero puede no decirse nada |
| 🔴 **El coste de flash de los cambios del 04/09 por la tarde** | **no medido por la compuerta.** El acta de `624eb37` es anterior a ellos, y la lectura directa del `.elf` **no reconcilia** con el delta reportado. La discrepancia esta publicada en la revision del 04/09 (tarde); **el numero bueno sale de correr la compuerta sobre el arbol de hoy** |
| 🔴 **El rotulo Bluetooth** | **nadie lo ha visto en un telefono.** §3.7 apoya en el una decision operativa —a que poste camina el operario— y §3.8 deja abierto que **dos modulos virgenes se llaman igual** hasta una vuelta de energia |
| 🔴 **Que el ambar ordenado (N-134) llegue a la otra punta** | el comando existe y las dos puntas lo comparten (`protocolo.h:174`), **MEDIDO sobre fichero**. **Nadie ha visto salir el `CMD_GO_AMBAR` de un Maestro ni entrar en un Esclavo**, y era en banco donde se veia el sintoma que este cambio arregla —*"a veces los dos, a veces solo el maestro"*—. La red de la orfandad de 25 s sigue puesta, asi que un fallo degrada al comportamiento de ayer |
| 🔴 **Que la primera arrancada tras N-133 deje los tiempos en los minimos** | la subida de firma esta **MEDIDA** (`respaldo.cpp:76`), pero `respaldo_borrar()` **no limpia `DR9`/`DR10`** y los sella como validos (**§3.9**). En un equipo actualizado con pila buena **el resultado depende de bytes que nadie ha leido**. **Se mira en el poste, no se supone** |
| 🔴 **Que el PIN de la app caduque EN LA APK** | los dos plazos estan **MEDIDOS** en el fuente (`PIN_GRACIA_FONDO_MS` y `PIN_INACTIVIDAD_MS` de `App_Semaforo/www/app.js`), pero cuelgan de `visibilitychange` / `pagehide` y **no hay `pause` de Cordova**. **Nadie lo ha visto caducar con el telefono en el bolsillo y la pantalla apagada**, que es el unico escenario para el que existe. **SIN VERIFICAR** |
| 🔴 **Que N-106 salga de verdad por el todo-rojo** | el camino esta **MEDIDO** (`Esclavo/src/bluetooth.cpp:293-308`) y **no se ha ejercido ni en tarjeta ni en arnes**. `CLAUDE.md` §8.bis pide ver fallar el instrumento antes de fiarse, y para este camino **no se ha hecho** |
| 🔴 **La causa del `FORMATO_INVALIDO` del Courier RTC** | **SIN DIAGNOSTICAR.** La app lo traduce a lenguaje de obra desde el 04/09, y eso **hace legible el sintoma sin decir nada de la causa**. Aqui no se propone ninguna |
| 🔴 **Que `J9`, `J11` y `J13` esten SOLDADOS en la tarjeta** | 🆕 **05/09.** El esquematico los marca `in_bom=yes`, `dnp=no`, `on_board=yes`, y el `.kicad_pcb` trae sus huellas, sus optos (`U12`, `U13`, `U14`) y sus MOSFET (`Q7`, `Q8`, `Q9`). **Nadie los ha mirado en cobre.** Lo mas cerca es `J15`, el gemelo exacto, que si funciono en banco el 04/09 — eso los hace probables, **no ciertos**. Es una inspeccion a ojo mas continuidad, y va antes de §3.10 |
| 🔴 **Los ~12 V de reposo y los ~10 mA de las borneras de potencia** | 🆕 **05/09. Leidos del cobre, NO medidos con multimetro.** El pull-up de 1 kOhm mas LED al riel de 12 V esta en el netlist (`R23`, `R28`, `R33`, `R38`, `R43`, `R48`, `R53`, `R58`, `R63`, `R73`) y la corriente sale de una **cuenta**, no de una sonda. Se cierra en dos minutos midiendo p2 contra masa con la bornera desconectada |
| 🔴 **Si el `D21` sin conectar de `J8` es defecto o decision** | 🆕 **05/09.** El catodo del LED del canal de `VERDE2` esta **sin conectar** en el esquematico y en el cobre (red `unconnected-(D21-K-Pad1)`, cero pistas), asi que `J8` p2 **flota** en reposo mientras los otros nueve suben a 12 V. **No hay ni una nota en el repositorio sobre ello.** Se cierra comparando `J8` p1-p2 contra `J7` p1-p2 con el mismo estado de luz |
| 🔴 **Que `PB3`/`PB4`/`PB5` se puedan usar con el ESP32 puesto** | 🆕 **05/09.** Estan libres y en alta impedancia, con pista a `J17` p4, p1 y p5 — **el mismo conector donde vive el modulo**. Que lo que se cuelgue de esas tres posiciones no le moleste **no esta medido**. Lo que si esta leido del fuente es que usarlos **no cuesta el SWD**: `pinF1_DisconnectDebug()` hace `NOJTAG`, no `SWJ_DISABLE` |

> 🛑 **La compuerta del 28/08 salio con `15 PASS | 0 FALLA | 0 ABORTADO` y eso no autoriza nada de
> este documento.** Lo dice el acta y lo dice `CLAUDE.md` §3: ese `0` significa que *los modelos y
> los arneses de PC no encuentran nada*. **Ninguno de ellos toca la tarjeta**, ninguno tiene
> bornera, y ninguno sabe si el cobre de la tarjeta es el del plano. **Verde no
> es entregable.**

---

## Anexo · Cambios que otros ficheros necesitan y que este documento NO ha hecho

Ninguno de estos se ha tocado. Es la lista de trabajo, no el trabajo.

**Firmware — ~~y estos tres van antes de retirar nada~~ → los tres primeros YA NO ESTAN PENDIENTES.
Se conservan tachados: una tarea que desaparece en silencio se vuelve a pedir.**

1. ~~**`SET_MODO:MENU`** en `Maestro/src/bluetooth.cpp`, y su boton en la app. **Antes** de ignorar
   los pulsadores (§2.3).~~ → ✅ **HECHO en `d34cfe2` (N-78)**, con los otros cinco:
   las ramas `strcmp(accion, "SET_MODO:...")` de `bluetooth.cpp` —`MENU`, `ALCANCE`, `INTELIGENTE`, `DEGRADADO`— y
   `REINICIAR_RELOJ`, `:345` `DEMANDA` (MEDIDO el 31/08). 🟡 **Lo que queda es la mitad de la app:
   comprobar que hay boton para cada uno** — `ESTADO.md`, fila `APP-APK`.
2. ~~**Quien hereda el veto de `mando_ambarLocal()`** en `Esclavo/src/main.cpp:401`, `:408`, `:526`.
   **Antes** de retirar `mando.cpp` (§2.4).~~ → ✅ **RESUELTO POR DECISION el 31/08: no se retira
   `mando.cpp`.** El mando se conserva en `A` y `B`, el armador `ACC_AMBAR` (`grep -n "ACC_AMBAR" Esclavo/src/mando.cpp`) se
   queda y **no hay veto que heredar**. *(De paso: las tres lineas citadas estaban caducadas — son
   `:406`, `:416`, `:540`.)* 🟡 **Queda un pack** que exija que `ACC_AMBAR` siga siendo el unico
   armador y los consumidores sigan siendo tres y negados.
3. ~~**`SET_RTC` tiene que mirar `reloj_hayCristal()`** antes de contestar `RESULT:OK`
   (la rama `SET_RTC` de `bluetooth.cpp` y `reloj_hayCristal()` de `reloj.cpp`) (§2.5).~~ → ✅ **HECHO en `d34cfe2` (N-80)**:
   `bluetooth.cpp:309`. Cinco ramas, `:295-328`. Ver §2.5.
3.bis ~~🔴 **NUEVO (31/08) — N-106: el ambar de emergencia de la app no sale del Degradado en el
   Esclavo.** Las dos ramas `AMBAR_EMERGENCIA` de `Esclavo/src/bluetooth.cpp` —la sin PIN y la con PIN— arman el latch y llaman a
   `semaforo_iniciarFallo()`, pero **no** a `degradado_salir()`.~~ → ✅ **HECHO el 04/09.**
   `salidaDegradadoIniciada()` (`Esclavo/src/bluetooth.cpp:302-308`) preguntada en las **dos**
   puertas, `:402` y `:481`. **Y el envoltorio es la parte que hay que no perder:**
   `degradado_salir()` es `void` y **abandona en silencio** desde `DEG_INACTIVO` y `DEG_SALIENDO`,
   asi que llamarla suelta y contestar `$ACK` detras habria sido **el mismo OK mudo** que N-106
   denunciaba; por eso el envoltorio pregunta **la misma guarda que ella**, no una parecida
   (`:293-301`). El `RESULT` pasa de uno a **cinco**.
   > 🔴 **ESTE PUNTO NO SE CIERRA DEL TODO, y se deja abierto a proposito con la mitad que falta:**
   > el arreglo entro **sin el arnes visto fallar** que este mismo punto pedia (`CLAUDE.md` §8.bis).
   > Con el arreglo ya dentro, la unica forma honesta de exigirlo es **inyectar el
   > `semaforo_iniciarFallo()` a secas en el `.cpp` real y comprobar que un pack baja la cuenta y
   > cambia el codigo de salida**. Sin eso, entro sin testigo.
3.ter 🔴 **NUEVO (04/09) — `respaldo_borrar()` no limpia `DR9`/`DR10` y los sella como validos.**
   Detalle, escenarios y las tres opciones en **§3.9**. Va **antes** de dar por buena la puesta en
   marcha de ningun equipo actualizado, porque su caso de uso principal **es** esa primera arrancada.
3.quater ✅ ~~**NUEVO (04/09) — el `enum` de un solo valor que queda vivo.**
   `Maestro/src/modo_inteligente.cpp:14` declara `enum FaseInt { INT_CORRIENDO };` y **lo compara**
   en `:40` y `:62`.~~ → **CERRADO EL 05/09: el `enum` se retiro.**
   `grep -n "FaseInt" Maestro/src/modo_inteligente.cpp` devuelve **dos lineas y las dos son
   comentario** —el bloque *"N-135 OTRA VEZ, EN EL FICHERO DE AL LADO"*—. Era **inerte**: no
   colgaba ninguna guarda de el.
   > 🔴 **PERO LO QUE ESTE PUNTO ABRIA NO SE CIERRA CON EL, Y CAMBIA DE SUJETO:** lo encontro
   > una revision externa y **no el pack que existe para esto**. El censo de
   > `maestro_10_coordinador_alcanzable.py` sigue mirando **solo `==`** —
   > `if _re.search(r"==\s*%s|%s\s*==" % ...)` — y **un `case` tambien es una comparacion**. El
   > comentario del fuente afirma que *"el pack se afila en el mismo commit"*; **medido el 05/09: no
   > se afilo**, y es el unico pack que censa esta forma
   > (`grep -ln "len(_vals) != 1" Simulaciones/banco/packs/*.py` → **1 fichero**). **Queda
   > ABIERTO**, y no es firmware: es el vigilante.
4. ✅ ~~**La telemetria fabricada:** `BAT:12.6` en las dos puntas, y `RF:98%` / `RTT:85ms` en el
   Esclavo. Se retiran o se marcan; no se dejan con aspecto de medida (§2.6). Y el campo `T:` no es
   tiempo de fase — el comentario de `bluetooth.cpp:241` dice que si.~~ → **HECHO, y las dos
   mitades por separado (05/09):**
   - 🟢 **Los literales se retiraron** por N-108: hoy los dos `$STATUS` llevan `BAT:--`, y el del
     Esclavo ademas `RF:--` y `T:--`. `grep -n "BAT:--" Maestro/src/bluetooth.cpp
     Esclavo/src/bluetooth.cpp` da **una linea por punta**. Detalle en §2.6.
   - 🔴 **La frase del campo `T:` era una AFIRMACION FALSA, y por partida doble.** Hoy `T:` **SI**
     es tiempo de fase en el Maestro —sale de `coordinador_segundosRestantesFase()` y, si no hay,
     de `modoAutomatico_segundosRestantesFase()`, con `--` cuando no hay cuenta que dar—, y el
     comentario que se citaba **no habla de eso**: la linea que se citaba trata del umbral de
     silencio de `LATIDO_MS`. `grep -n "segundosRestantesFase" Maestro/src/bluetooth.cpp`.
   - 🟠 **Lo unico que sobrevive del punto es `MODO:SUBORDINADO`** en el Esclavo, que **sigue
     siendo un literal** —y esta razonado en el propio fuente, no es un descuido—:
     `grep -n "MODO:SUBORDINADO" Esclavo/src/bluetooth.cpp`.
5. **La polaridad de `botones.cpp`** en las dos puntas, ~~si M3 dice que el netlist tiene razon~~
   (§2.2). Con su pack, como N-67 tuvo el suyo. 🔴 **Y desde el 31/08 esto SUBE de prioridad, no
   baja:** con los pulsadores 3 y 4 retirados, `PB9` y `PB13` dejan de ser *"pines que se van"* y
   pasan a ser **las dos entradas del unico mando fisico que queda** (§3.3). ~~Si M3 dice que el
   netlist tiene razon, el mando esta leyendo al reves **la superficie de ultimo recurso del
   equipo**.~~
   > 🔴 **YA NO ES UN CONDICIONAL — 04/09. M3 lo dijo: el netlist tiene razon, y el mando estaba
   > leyendo al reves la superficie de ultimo recurso del equipo (N-118).** Esto paso de *"punto 5 de
   > un anexo"* a **el arreglo de firmware con nombre y sitio**: `pinMode(INPUT)` pelado y deteccion
   > contra `HIGH` para `MANDO_A` y `MANDO_B`, en las **dos** puntas, exactamente como se hizo con la
   > camara en N-67.
   >
   > 🟢 **EL FUENTE YA ESTA: `346ea5f`** —`Maestro/src/botones.cpp:40`, `:160-161`;
   > `Esclavo/src/botones.cpp:54`, `:178-179`—. **Este punto NO se cierra con eso**, y se deja
   > abierto a proposito con las dos mitades que faltan:
   >
   > 1. 🔴 **El pack, visto fallar sobre el firmware de ANTES** (`CLAUDE.md` §8.bis). Con el arreglo
   >    ya dentro, la unica forma honesta de exigirlo es **inyectar la polaridad vieja en el `.cpp`
   >    real y comprobar que el pack baja la cuenta y cambia el codigo de salida**. Sin eso el
   >    arreglo entro sin testigo, que es exactamente lo que este punto pedia evitar.
   > 2. 🔴 **La carga verificada en tarjeta.** Y **no en una tarjeta con un corto** (N-116): se
   >    prueba sobre la que esta sana, y con el gesto nuevo —`p5` contra `p4`, `p8` contra `p7`—,
   >    **no contra masa**.
6. ~~**`VERDE_MIN_MIN`** (`modo_automatico.cpp:31`) cuando llegue el numero, **atado a la app** en
   el mismo commit (N75-1 y N75-2, §3.4).~~ → ✅ **HECHO el 04/09.** El numero llego —**3 minutos**—
     y entro atado —y **desde N-137 la constante vive en `Maestro/include/limites_ciclo.h`**, no en
     `modo_automatico.cpp`—: `app.js` con `enRango(verde, 3, 15)`, `index.html`
   con `min="3"`, y el pack **`app_11_rangos_de_tiempos`** releyendo los tres del fuente en cada
   corrida.
   > 🔴 **Queda un CUARTO sitio que ese pack no mira, y hoy dice lo contrario que el
   > firmware:** `05_Funcional/App_Semaforo/js/config.js:10-17` — `LIMITES_TIEMPO` con
   > `VERDE_MIN_MIN: 1` y `ROJO_MIN_MIN: 1`, bajo el comentario *«Rangos de Tiempos Permitidos por
   > Firmware»*. ~~`index.html:718` lo carga~~ —el `<script>` se encuentra con `grep -n 'src="js/config.js"' App_Semaforo/www/index.html`— y **no lo consume nadie fuera de `tests/`** (`grep` de
   > `IOT_CONFIG` y de `LIMITES_TIEMPO` sobre la app). Es un huerfano de los de N-73 **con una cifra
   > caducada dentro**: o se borra, o se ata al pack. **No se toca desde este documento.**
7. **Comentarios que ya mienten:** un comentario de `Esclavo/src/main.cpp` —`grep -n "12,5 s"
   Esclavo/src/main.cpp`, sigue ahi el 05/09— dice *"cae a C_FALLO en ~12,5 s"* con
   `SFTY6_SILENCIO_MS` en 25 000. Es lo que `CLAUDE.md` avisa de los comentarios que sobreviven a un
   numero.

**Instrumentos** — y esta es la parte que impide que todo esto vuelva:

8. ~~**Los packs que ejercen pantalla, botones y mando quedan sin sujeto.**~~ → **corregido el
   31/08: los del MANDO conservan su sujeto entero** (`A.A.A`, `B.B.B`, `A.B.A.B`, `ACC_AMBAR` y el
   veto de §2.4 siguen existiendo). Se quedan sin sujeto los de **pantalla y menu**, y **solo la
   parte de `botones.cpp` que mira los flancos 3 y 4** — los flancos 1 y 2 siguen alimentando a
   `mando_registrarPulso()` (`grep -n "mando_registrarPulso" Maestro/src/botones.cpp`). Hay que decidir uno por uno si se borran, se
   invierten o se conservan (`CLAUDE.md` §8.quater), **con la cuenta comparada antes y despues**,
   que es la unica red para esta clase de deriva (§5 de `CLAUDE.md`). **Retirar de mas aqui es
   exactamente el error que la decision del 31/08 evita en el firmware.**
9. **Un pack que ate la polaridad de los cuatro pines de boton al netlist**, como
   `camara_01_demanda` ato la de `PB0`.
10. **Un pack que ate `VERDE_MIN_MIN` del `.cpp` a los limites de `app.js`** (N75-2).
11. **Los tres packs `documentos_*`** vigilan lo que README, `ESTADO.md`, `OPTIMIZACIONES.md` y el
    Manual 10 **dicen haber medido**. Cambiar el Manual 10 sin mirarlos deja el banco en rojo — o,
    peor, en verde vigilando una frase que ya no esta.
12. 🔴 **NUEVO (04/09) — un pack que censE los `enum` de un solo valor QUE ADEMAS SE COMPARAN.**
    N-135 costo el camino de escritura de N-133 y **ningun instrumento lo vio**: `maestro_10` censaba
    funciones que devuelven un **literal** (`return false;`), y esta devolvia una **comparacion**. La
    forma era distinta y la consecuencia identica. Es un **trinquete, no un absoluto** —un `enum` de
    un valor que nadie compara puede ser legitimo—, y su primer sujeto vivo esta en
    `modo_inteligente.cpp`. 🔴 **05/09: el `enum` YA NO EXISTE** (retirado por N-135); lo que
      queda abierto es que el pack `maestro_10` **sigue mirando solo `==` y no `case`**.
13. 🔴 **NUEVO (04/09) — que la caducidad del PIN se ejerza EN LA APK, no en un navegador.** Es la
    unica mitad `SIN VERIFICAR` de la barrera nueva de la app, y es justo la que importa: los dos
    plazos cuelgan de `visibilitychange` / `pagehide` (`grep -n "visibilitychange\|pagehide" App_Semaforo/www/app.js`) y **no hay `pause` de
    Cordova**. **Se cronometra con un telefono en el bolsillo y la pantalla apagada.** Un arnes de
    DOM en un navegador de escritorio **no puede contestar esta pregunta**, y darlo por medido ahi
    seria declarar sin ejercer.
14. 🔴 **NUEVO (04/09) — un pack que ate el borrado del respaldo a su checksum.** La regla es
    mecanica y por eso se puede vigilar: **todo registro que entre en `calcularSuma()` tiene que
    salir en `respaldo_borrar()`.** Hoy son diez contra cinco (§3.9). Es la clase de desigualdad que
    `CLAUDE.md` avisa que **no puede vivir en un comentario**.

**Documentos:** los de la seccion B, en el orden de la seccion B.

---

*Escrito el 28/08/2026. Revisado el 31/08/2026 y el 04/09/2026. Todo lo marcado MEDIDO se puede
repetir abriendo el fichero y la linea que se cita; lo marcado **MEDIDO EN COBRE** se puede repetir
con un multimetro, y trae el numero que dio. Lo marcado ESCRITO tiene su fuente al lado. Lo marcado
SIN VERIFICAR no lo ha comprobado nadie — ni aqui ni en ningun otro sitio de este repositorio.*

> **Y la unica frase de este documento que el 04/09 vale mas que el 28/08:** durante meses lo que
> sabiamos del cobre salia de un dibujo, y este documento lo decia en cada pagina. **Tres dias de
> banco cerraron cuatro medidas, resolvieron una contradiccion que se daba por indecidible, abrieron
> una decision de diseno que nadie habia planteado y dejaron una tarjeta fuera de servicio.**
> Ninguna de esas cuatro cosas la habria producido otra pasada de lectura.
