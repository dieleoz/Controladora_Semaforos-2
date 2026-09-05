# 🧪 COBERTURA DE PRUEBAS Y HUECOS — V9.0

**Fecha:** 26 de Agosto de 2026
**Última revisión:** 5 de septiembre de 2026 — **la primera que censa una superficie que este
documento no sabía mirar: la de un componente comprado y no medido** (las cámaras). La revisión
del 4 de septiembre fue la primera con una sesión de banco detrás
**Método:** censo automático de la superficie de entrada del firmware cruzado contra ~~los 20 packs~~
**los packs del banco**. **No es una opinión: cada fila se levantó con una búsqueda sobre el código**,
y el comando que la produce está anotado para que cualquiera la repita.

> 🔴 **Y desde el 04/09 hay que decir dónde acaba ese método, porque el banco lo demostró.** Una
> búsqueda sobre el código encuentra propiedades **del código**. Los tres hallazgos que pararon la
> sesión de banco del 3–4/09 **no son propiedades del código** —una corriente, un tiempo de arranque
> y una resistencia de 10 kΩ en el cobre—, y por eso **ninguno de los tres podía aparecer en este
> censo**, por bien hecho que estuviera. Ver la sección nueva de abajo.

> ## ⚠️ 31/08 — ESTE DOCUMENTO YA NO PUBLICA CIFRAS DEL BANCO. REMITE AL ACTA
>
> Las ediciones anteriores decían *«los 20 packs · `155/155`»*. **Hoy son bastantes más y la cuenta
> se mueve cada hora**, así que escribirla aquí a mano garantiza que envejezca en silencio — que es
> exactamente lo que este documento existe para evitar.
>
> **La cifra vigente se lee del acta más reciente de `evidencia/`** —`evidencia/<AAAA-MM-DD>_compuerta.txt`—,
> que la escribe `01_Firmware/compuerta.py` con la fecha y el hash de `HEAD` encima. **Hay dos packs
> que vigilan que las cifras publicadas salgan del acta**, no de la memoria de nadie.
>
> ```
> python 01_Firmware/compuerta.py                       # 0 PASS | 1 FALLA | 2 ABORTADO
> python 01_Firmware/Simulaciones/banco/correr.py --listar
> ```
>
> 🔴 **Y el acta se lee entera, no su primer número.** `ABORTADO` **no es** `PASS`: una comprobación
> que no pudo correr **no dice nada del firmware**. Un `x/y` con `x != y` significa que hay
> comprobaciones que no cumplen, lo escriba como lo escriba el instrumento y salga con el código que
> salga. ~~**Y el acta del 31/08 no está en verde.**~~ **Caducado el 04/09: las actas del 02/09 y del
> 04/09 sí salen en verde, con todo en `PASS` y ningún `ABORTADO`.**
>
> **Nada de esto es un permiso, y ahora está MEDIDO que no lo es.** En campo corre la **V8.4**, y
> **verde no es entregable**: la compuerta dice que los modelos y los arneses de PC no encuentran
> nada, **no que el firmware funcione en la tarjeta**. El paquete `617bd00` llegó al banco del 3–4/09
> con esa acta en verde debajo —el propio informe lo cita: *«20/20 comprobaciones en PASS … pero
> explícitamente marcada como no probada en banco»*— y **la sesión se paró tres veces**. El verde era
> cierto y no servía de nada para lo que falló.

---

## 🔬 EL BANCO CORRIÓ (3 y 4 de septiembre) — LA PRIMERA VEZ

> **Este documento llevaba desde el 26/08 diciendo qué falta probar. El 3 y 4 de septiembre alguien
> lo probó sobre tarjetas de verdad, y eso cambia el reparto entero de esta página.**

**Qué se ejecutó:** la `Guia_Cableado_y_Pruebas_Banco.html` completa —29 pasos—, sobre una tarjeta
Maestro y una Esclavo con el paquete **`617bd00`**, más un ESP32 de puente. Lo devuelto es un informe
en PDF, y **es la fuente de todo lo que sigue**: `evidencia/Informe_Pruebas_Banco_Semaforos_V9.0.pdf`.

### La cuenta, y por qué este documento no la copia

El informe encabeza con **24 completos · 4 bloqueados por el enlace Bluetooth · 1 abortado por
seguridad**. Sus tres cajones nombran, en cambio, `1-6, 8-9, 15-18, 20-24` (17) · `25-28` (4) ·
`29` (1) — **22 identificadores**. Los **siete que faltan** —`7`, `10`, `11`, `12`, `13`, `14`, `19`—
sí están descritos en el cuerpo: `7` y `19` como *PARCIAL*, `10` como *no logrado*, `11` y `12-14`
como *BLOQUEADO*. Repartidos como el cuerpo los describe, la cuenta que cuadra a 29 es
**19 / 9 / 1**, no 24 / 4 / 1.

> **No se publica aquí ninguna de las dos.** No hay forma de decidirlo desde el repositorio: lo
> decide quien ejecutó la sesión. Lo que sí se deja escrito es **dónde está la diferencia** —los
> pasos 10 a 14, el módulo que no se anuncia y todo lo que cuelga de él—, porque la cabecera **no los
> mete en ninguno de los tres cajones** y un paso que no está en ningún cajón es un hueco que no
> deja rastro de que falta. *(Es la lección de «cierto en cada línea y falso en conjunto» aplicada a
> un parte de trabajo: ninguna línea del informe miente, y la cuenta de arriba no sale.)*

### ✅ Lo que DEJÓ de ser hueco — está medido sobre cobre

| Qué | Lo medido | Paso |
|---|---|---|
| **Carga de firmware por SWD/ST-LINK** en las dos puntas | *«escrito y verificado» al primer intento, sin puente `BOOT0`* | 2 |
| **Radio Maestro↔Esclavo y caída a ámbar** | 3 cortes; a ámbar intermitente en **unos 20 s**, y vuelta sola a rojo fijo en **unos 3 s** al reconectar | 8 |
| **Talanquera `J15`** — y de paso, qué es `J15` | **salida directa de motor** (`MOT+`/`MOT−`), no entrada de disparo. En rojo `0 V`, abajo; en ámbar `12 V`, **sube**; al recuperar enlace vuelve a `0 V` y baja | 15, 16 |
| **Cámara de demanda `J14`** | borne 3,3 V a `3,3 V`, borne «Puerta» a `0 V`; al puentear sube y al soltar vuelve. **Conmuta** | 17, 18 |
| **Cámara cableada en `J16` p10** | cableada a p11 en normalmente-abierto; **en reposo no pide paso, con y sin el cable puesto** | 21 |
| **Masa común del módulo contra la STM32** | `0 V` —umbral exigido `50 mV`— y `GPIO17` a `3,3 V` | 23 |
| **Identidad real de `J17`** | **es el UART del ESP32**, no el LCD del netlist: `USART1` remapeado a `PB6`/`PB7` sale por `J17` pos. 3/2, confirmado por continuidad. El netlist de KiCad quedó desactualizado al retirar la pantalla | 3, 5 |
| **La medida `M3`** — resistencia real de `PB14`/`PB15` en cobre | 🟢 **CERRADA: `9,93 kΩ` y `9,94 kΩ` a masa**, y `0 V` en reposo. Los pines de cámara **sí tienen resistencia real en la placa**; ya no dependen del netlist | 20 |

### 🔴 Lo que SIGUE siendo hueco — ahora con nombre y con motivo

| Qué | Estado tras el banco |
|---|---|
| **N-42 — el Modo Automático no mueve las luces** | **NI confirmada NI descartada.** El equipo nunca llegó a operar en Automático: la única vía de selección de modo es la app, y la app no conectó. **Se decide repitiendo el paso 7 con enlace.** ⚠️ *Un «no se pudo probar» no es un «sigue rota» ni un «ya está»: es un `ABORTADO`, y §2 del `CLAUDE.md` dice qué vale eso* |
| **Verde simultáneo en las dos puntas sobre HARDWARE** | **Sigue sin ejercerse.** `barrera_02_dos_puntas` y el arnés de dos puntas lo cierran en PC —y dieron el margen real de `1,44`—, pero **ninguno ha encendido una lámpara**. Lo único que lo cierra es banco |
| **Operación por app · reloj `DS3231` (`SET_RTC`) · barrera de PIN** | **Bloqueados en cascada** (pasos 11-14 y 25-28) porque el módulo **no se anunció en el teléfono**. El hardware está descartado como causa: es un `ESP32-WROOM-32` clásico, con `BR/EDR`, que es justo el perfil que el `SPP` de la app necesita |
| **Paso 29 — mando de relés** | **ABORTADO por seguridad** (el Maestro se sobrecalentó y quedó pendiente de inspección en frío). ~~**Pero la sordera del mando no necesita ese paso para estar diagnosticada, y lo está por el paso 20:** la red de 10 kΩ a masa que la placa trae en esos pines —las `R65`–`R68` de `CLAUDE.md §9.bis`— deja `J16` p5/p8 en **`0,6 V` permanentes**, o sea BAJO fijo. **Nunca hay flanco**, así que el mando está sordo de fábrica~~ 🔧 **CADUCADO el mismo 04/09 por `N-118`, y se tacha en vez de borrarse porque es la frase que resucita el gesto peligroso.** Ese `0,6 V` **lo ponía el firmware**, no el cobre: era el `INPUT_PULLUP` contra esos 10 kΩ. Con `BOTON1`/`BOTON2` en `INPUT` pelado y **activo en ALTO** (`346ea5f`), los 10 kΩ pasan a ser el **reposo correcto**. **El mando NO está sordo de fábrica**; lo que cambia es el gesto: **`p5` contra `p4` y `p8` contra `p7` — los 3,3 V del pin contiguo, NUNCA contra masa** (`J16` tiene una sola masa, `p2`) |
| **La fuente `12 V → 5 V` de la placa del módulo** | **No medida con carga real:** toda la sesión se alimentó por USB. Pendiente antes de campo |

### 🔵 LA SEGUNDA NOCHE (4–5/09) — CUATRO DEFECTOS QUE ENCONTRÓ **UNA CINTA DE TRAMAS**, NO UN PACK

> **Esto es lo que esta página tenía que aprender, y llegó solo.** Ninguno de los cuatro es una
> propiedad del fuente que un pack pudiera ver: **los tres primeros salieron de LEER LA CINTA de lo
> que el equipo contestó**, con el teléfono conectado. **Es un instrumento nuevo, y es el primero en
> mucho tiempo que no es Python.**

| # | qué se encontró | cómo se encontró | estado del arreglo |
|---|---|---|---|
| **N-142** | El ámbar de emergencia del Esclavo **no llegaba al Maestro**, que podía seguir dando VERDE **hasta 3 min** con el otro lado en ámbar | **reportado en banco** por el operario | ✅ firmware escrito · 🔴 **SIN EJERCER en tarjeta** |
| **N-146** | `SET_MODO:AMBAR` contestaba `OK` **seis veces seguidas** y no encendía nada | 🎞️ **la cinta**: 6 órdenes, 6 `OK`, y `MODO:AMBAR,ESTADO:ROJO` durante **47 tramas** | ✅ escrito (`REARMADO`) · 🔴 **SIN EJERCER** |
| **N-147** | En Manual el cruce **cambiaba solo a los 15 s** y DAR PASO se rechazaba. **Y un tercer defecto que nadie reportó**: pulsar DAR PASO **reiniciaba el plazo**, así que pulsando rápido no se veía el verde nunca | reportado + **medido en el `.cpp`** | ✅ escrito · 🔴 **SIN EJERCER** |
| **N-149** | Desde el Maestro **no se veía el estado del Esclavo** | pedido en banco | ✅ campo `ESC:` · 🔴 **SIN EJERCER** |
| **N-145** | La hora salía **en blanco en TODAS las tramas** | 🎞️ **la cinta** | ✅ el puente sella el hueco · 🛑 **NO SE PUEDE PROBAR: no hay `DS3231` comprado (`A6`)** |
| — | **`BAT:--` en todas las tramas** | 🎞️ **la cinta** | 🔴 **NO ES UN DEFECTO A ARREGLAR:** el equipo **no tiene divisor ni entrada analógica**. El `--` es el marcado correcto (`N-108`). **Aquí no se escribe causa de por qué no se montó la medida: no se ha medido** |

> 🔴 **LO QUE ESTA TABLA DICE DE LA COBERTURA, Y ES LO INCÓMODO: los cinco pasaron la compuerta en
> verde.** Ninguno era una propiedad que un modelo de PC pudiera ver — **tres son «el equipo obedece
> y no avanza», que es exactamente lo que no deja rastro**. La compuerta responde *«los modelos y los
> arneses de PC no encuentran nada»*, y esta noche volvió a demostrarse que **eso no es un
> entregable**.
>
> ✅ **Lo que SÍ hicieron los instrumentos, y hay que anotarlo entero:** tres packs **cazaron los
> arreglos** —`esp32_07` tumbó un buffer de 160 que se habría truncado en el último paso;
> `app_03_sin_ok_mudo` acusó la rama nueva del ámbar; `documentos_03` detectó que las dos puntas
> dejaban de emitir los mismos campos— y el `simulador_app_bluetooth` **abortó** ante un campo
> desconocido en vez de ignorarlo. **La puerta se cerró sola** (`CLAUDE.md` §3.quater al revés).

> 🎞️ **EL HUECO DE COBERTURA QUE ESTO DESTAPA, Y NO SE CIERRA CON UN PACK:** **nadie estaba grabando
> las tramas hasta el 04/09.** Los tres defectos de arriba llevaban ahí desde antes, y lo único que
> hizo falta para verlos fue **guardar lo que el equipo contestó y leerlo después**. **Una cinta de
> tramas por sesión de banco debería ser parte del procedimiento**, no una casualidad de esa noche.

---

## 🧪 LO QUE ESTA PÁGINA TIENE QUE APRENDER DEL BANCO

### 1. Los tres defectos que pararon la sesión pasaron la compuerta sin despeinarla

Y no por un fallo de la compuerta: **ninguno de los tres es una propiedad del código fuente.**

| Lo que paró el banco | Qué clase de cosa es | Con qué se mide |
|---|---|---|
| El **sobrecalentamiento del STM32 del Maestro** al puentear `J16` p5/p8 contra masa | **corriente y temperatura** | amperímetro y un dedo — *el informe no afirma causa; pide inspección con la placa fría* |
| El **módulo que no se anuncia** | **cuánto tarda un arranque**, y si el anuncio `SPP` llega a arrancar | un monitor serie y un cronómetro |
| ~~Los **10 kΩ a masa** que dejan el mando sordo~~ → **la POLARIDAD con que el firmware declaraba esos dos pines** (`N-118`) | **no era una resistencia: era un `pinMode()`.** Los 10 kΩ están bien y son los mismos que en las cámaras | se mide en el fuente **y** con un voltímetro sobre el pin, **con el gesto bueno puesto** — `p5`–`p4`, `p8`–`p7` |

> **Ninguno de los tres está en el repositorio.** No hay `grep` que los encuentre, no hay pack que
> los pueda escribir y **no hay `x/y` que baje cuando aparecen**. Este documento se llama *«Cobertura
> de pruebas y huecos»* y hasta hoy sólo censaba **una** de las dos superficies: la del fuente. La
> otra —consumo, tiempo real de arranque, cobre— **no tiene aquí ni una fila**, y es la que se cobró
> la sesión.
>
> Es el corolario del `CLAUDE.md §2.bis` visto desde el otro lado: *un `20/20` sobre instrumento que
> nunca ha tocado una tarjeta no es un entregable*. **Aquí ya no es una advertencia: es el parte de
> una sesión.**

### 2. Dos huecos de instrumento reales, que el banco destapó

**a) El watchdog del puente: verde perfecto sobre la propiedad de al lado.**

`esp32_02_watchdog_alimentado` comprueba el **orden** del armado —el perro antes del `SPP` y antes
del I²C— y la **presencia** del reset en el bucle exterior. `esp32_01_watchdog_desigualdad` recalcula
la desigualdad `ESP32_WDT_MS + ESP32_ARRANQUE_MS < min(TIMEOUT_ENLACE_MS, SFTY6_SILENCIO_MS)` desde
sus tres fuentes. **Lo que ninguno acota es cuánto dura de verdad la ventana de arranque**: los
`1500 ms` de `ESP32_ARRANQUE_MS` son un número escrito en `contrato.h`, no una medida.

> **El pack lo sabe y lo dice** —lleva `ESP32_ARRANQUE_MEDIDO = 0` en el propio C++ y lo saca por
> `reportar()`—, **y ahí está justo el problema: `reportar()` no cuenta**. Un hueco declarado sigue
> siendo un hueco: la compuerta salió con `0`, el número que gobierna la vida del puente nunca se
> midió, y en el banco **el módulo no llegó a anunciarse**. Que el instrumento avise no cierra nada;
> lo cierra el módulo en la mano y un cronómetro.

**b) Una lista de excepciones con motivos que nadie comprueba es una lista de defectos con permiso.**

`app_07_generadores_de_trama` lleva un trinquete de huérfanos con su lista de conocidos, y cada
entrada lleva un **motivo escrito**. `BluetoothDriver` estuvo meses aceptado con éste:
*«`app.js` habla por `window.bluetoothSerial`, sin pasar por aquí»*. Es **medio cierto** — `app.js`
usa `write`, `subscribe` y `list`… y **no usa `connect`**, que es justo la que hace funcionar a las
otras tres. **Resultado: la app nunca abrió un socket**, y se pintaba «Enlazado» por haber pulsado
una fila de la lista (N-122).

> **Un huérfano se acepta por una razón, y una razón es una AFIRMACIÓN SOBRE EL CÓDIGO — o sea, algo
> que se comprueba, no que se escribe.** El pack estaba bien y el trinquete estaba bien: lo que
> nadie medía era **la frase de la excepción**. Se mide al añadir la entrada, y también al heredarla.

---

## 📷 05/09 — EL HUECO QUE ORDENA A TODOS LOS DEMÁS: **NINGUNA CÁMARA HA TOCADO NUNCA ESTE SISTEMA**

> **Se han comprado dos `DS-2CD2683G2-IZS` (`DECISIONES.md` D-10). Este documento lleva desde el
> 26/08 censando la superficie de entrada del firmware, y las cámaras aparecen en él como una
> entrada más. No lo son: son el único sujeto del sistema del que NO EXISTE UNA SOLA MEDIDA.**

Todo lo que este repositorio dice de cámaras se ha ejercido **con un pulsador suelto o con un puente
de cable**. Está escrito en el propio informe del banco del 3–4/09, y es correcto lo que hicieron:

| lo que se probó | con qué | paso |
|---|---|---|
| que el borne de `J14` conmuta | **puenteando dos bornes a mano** | 17 y 18 |
| que `J16` p10 cableado no pide paso solo | **un cable a p11, en normalmente-abierto** | 21 |
| que `PB14`/`PB15` tienen 10 kΩ reales a masa | ohmímetro, sin cámara | 20 |

**Ninguno de los tres necesitaba una cámara y ninguno la tuvo.** Lo que miden es cierto: **el camino
eléctrico existe**. Lo que no dicen —y no pueden decir— es qué manda por él una `DS-2CD2683G2-IZS`.

### 1. Y el modo que las consume es obra DECLARADA, no EJERCIDA — en las dos direcciones a la vez

Es `CLAUDE.md` §2.ter en su forma más completa que se ha visto en este repositorio: **hay firmware
escrito, hay documentación que lo describe, y no hay ni un instrumento ni una tarjeta debajo.**

```
01_Firmware$ grep -rl modo_inteligente Validacion_* compuerta.py
(sin salida - ningun arnes que compile C++ real lo toca)
```

- **Ningún arnés lo compila.** Los cuatro que compilan C++ de verdad —`Validacion_LCD`, `_Ciclo`,
  `_Respaldo`, `_Automatico`— no incluyen `modo_inteligente.cpp`. Lo leen **tres packs, por texto**.
  Es el punto ciego del `CLAUDE.md` §8 en su forma pura: *un `PASS` del modelo no prueba el código*,
  y aquí **ni siquiera hay modelo**: hay lectura de cadenas.
- **Ninguna tarjeta lo ha corrido.** El informe del banco del 3–4/09 lo dice con estas palabras:
  *«nunca apareció la luz verde en ningún poste»* y *«sin cámara de demanda real»*.
- **Y sólo se entra por la app**, que es justo lo que no conectó en esa sesión: la vía del menú está
  muerta —`menu.cpp` cuelga de `botonAceptar()`, que devuelve `false` siempre— y el mando no tiene
  secuencia para él.

> 🔴 **O sea que el Modo Inteligente lleva meses sin que nada lo mire: ni un compilador, ni un
> modelo, ni un pin.** Y no aparece como hueco en ningún recuento, porque **los packs que lo leen por
> texto cuentan como cobertura suya**. Es el `x/y` que no baja: el hueco no deja rastro de que falta.

### 2. Y ese mismo modo VIOLA el mínimo vial, en el fuente, desde antes de comprar la cámara

```
01_Firmware$ grep -rn "15000UL" --include=*.cpp Maestro
Maestro/src/modo_inteligente.cpp:123:        if (tiempoActual >= 15000UL) {

01_Firmware$ grep -rn "VERDE_MIN_MIN =" --include=*.h Maestro
Maestro/include/limites_ciclo.h:54:static const uint8_t VERDE_MIN_MIN = 3,  VERDE_MIN_MAX = 15;
```

**`modo_inteligente.cpp` permite cortar el verde a los 15 segundos** —constante escrita a mano, que
no pasa por `limites_ciclo.h`— mientras **`VERDE_MIN_MIN = 3` MINUTOS**, decidido por el responsable
el 04/09 y razonado en ese mismo fichero: *«un conductor convencido de que el semáforo está averiado,
adelantando en rojo»* (`DECISIONES.md` D-5).

**Con una cámara pegada en «hay presencia», o con cola continua, el Esclavo recibe 15 s de verde por
ciclo y el Maestro tres minutos.** Con las dos cámaras ruidosas el cruce alterna al mínimo
indefinidamente. **Es exactamente el defecto que N-137 cerró en la configuración del coordinador
—`maxVerde = 2` minutos— y que sigue abierto en la guarda de al lado, en la misma función.**

> **Y la señal de método, que es lo que esta página tiene que aprender:** las tres copias que N-131,
> N-133 y N-137 encontraron eran **tiempos escritos a mano fuera del fichero de límites**, y de ahí
> salió el pack `app_11_rangos_de_tiempos`, que **censa TODOS los `.cpp` del Maestro**. Ésta es la
> cuarta copia y **el pack no la ve.**

**Y su borde está medido, no supuesto** — es el propio filtro del pack, en `app_11_rangos_de_tiempos`:

```python
VARS = {"minRojo": (r_min, r_max), "minVerde": (v_min, v_max),
        "segEstatico": (d_min, d_max), "maxVerde": (v_min, v_max)}
for m in re.finditer(r"(%s)\s*(?:=|<|>)\s*(\d+)" % "|".join(VARS), _t):
```

> **El censo busca CUATRO NOMBRES DE VARIABLE.** La guarda de los 15 s no usa ninguno: es
> `if (tiempoActual >= 15000UL)`. **El fichero está dentro del alcance y la línea no**, que es peor
> que quedarse fuera — el pack informa de que ha mirado `modo_inteligente.cpp` y ha mirado justo lo
> que no era. Es `CLAUDE.md` §4.quinquies: *cuando un instrumento compara contra una lista, escribe
> al lado cuál es esa lista y por qué es la correcta*. Aquí la lista es correcta para lo que N-137
> arregló —**`maxVerde = VERDE_MIN_MIN` sigue vigilado**— y ciega para lo que quedó al lado.

### 3. En Automático y en Manual, una detección de cámara HOY NO HACE NADA

Medido con el censo de llamadas —`grep` de la declaración contra los llamadores, que es como se
cuenta esto y no leyendo:

```
01_Firmware$ grep -rn "demanda_hayLocal" --include=*.cpp --include=*.h Maestro Esclavo
Maestro/include/demanda.h:30:bool demanda_hayLocal();
Maestro/src/demanda.cpp:28:bool demanda_hayLocal() {
Maestro/src/modo_inteligente.cpp:119:        bool demandaLocalS1 = camara_leerPin(CAM_DEMANDA_PIN) || demanda_hayLocal();

01_Firmware$ grep -rn "hayDemandaRemota" --include=*.cpp --include=*.h Maestro
Maestro/include/coordinador.h:47:bool coordinador_hayDemandaRemota();
Maestro/src/coordinador.cpp:50:bool coordinador_hayDemandaRemota() {
Maestro/src/modo_inteligente.cpp:120:        bool demandaRemotaS2 = coordinador_hayDemandaRemota();
Maestro/src/modo_inteligente.cpp:157:      int presenciaActual = (camara_leerPin(CAM_DEMANDA_PIN) ? 1 : 0) + ...
```

**Las dos banderas de demanda tienen UN SOLO lector, y es el fichero que nadie ejerce.** El flanco
de una cámara en `J16` entra por `camaras_actualizar()` y llama a `demanda_solicitar()`, que **arma
un temporizador que fuera del Modo Inteligente no lee nadie**.

> ✅ **Y esto NO es un defecto abierto: la mitad remota ya está tapada a propósito y bien.** N-130
> hizo que el Maestro **no arme** `demandaRemotaPendiente` salvo en Modo Inteligente, y que el acuse
> al Esclavo **diga cuál de las dos cosas es** en vez de mentir con un `OK`. Es la barrera de salidas
> del §6 aplicada a través de la radio.
>
> 🔴 **Lo que sigue sin estar tapado es la mitad LOCAL**, y es la que se va a cablear ahora: en
> Automático y en Manual, un coche delante de la cámara del propio poste **arma una bandera que
> nadie lee y no produce ni un evento**. No es peligroso —una cámara **pide**, no ordena— pero
> significa que **cablear las dos cámaras hoy no cambia absolutamente nada en los dos únicos modos
> que se operan**, y eso hay que decirlo antes de que alguien suba a un poste a montarlas.

**Y hay un coste que sí se paga, con la cámara del Esclavo:** `demanda_solicitar()` del Esclavo manda
`CMD_DEMANDA` por radio **en todos los modos** —`demanda.cpp` de esa punta, sin guarda de modo—, en
un canal de `2,4 kbps` semidúplex que también lleva `GO_RED`/`ACK_RED`. *[**SIN MEDIR**: el impacto
probablemente es pequeño, pero es una colisión posible en el instante que más importa. Se escribe
como pregunta, no como hallazgo.]*

### 4. La constante que se apoya en una cifra nuestra — y DOS packs verdes vigilándola (A-7)

```
01_Firmware$ grep -rn "cerrado ~1 s\|cierra ~1 s\|PULSO_RELE_MS = " \
             --include=*.cpp --include=*.py Maestro Esclavo Simulaciones/banco/packs
Maestro/src/botones.cpp:135:// El rele de la camara mantiene el contacto cerrado ~1 s por deteccion: leer el nivel
Maestro/src/demanda.cpp:5:// de la camara AcuSense cierra ~1 s por deteccion, y un coche detras de otro dispara
Esclavo/src/botones.cpp:155:// El rele de la camara mantiene el contacto cerrado ~1 s por deteccion: leer el nivel
Esclavo/src/demanda.cpp:6:// de la camara AcuSense cierra ~1 s por deteccion, y un coche detras de otro dispara
Esclavo/src/main.cpp:342:  // El rele de la camara mantiene el contacto cerrado ~1 s por deteccion; leer el nivel
Simulaciones/banco/packs/camara_01_demanda.py:39:# El rele de la camara AcuSense cierra ~1 s por deteccion (Manual 9, paso 3 de la
Simulaciones/banco/packs/camara_01_demanda.py:43:PULSO_RELE_MS = 1000
Simulaciones/banco/packs/camara_02_j16.py:89:# El rele de la camara AcuSense cierra ~1 s por deteccion (Manual 9). La ventana de
Simulaciones/banco/packs/camara_02_j16.py:92:PULSO_RELE_MS = 1000
```

`DECISIONES.md` **A-7** ya lo tenía escrito citando un sitio. **Son nueve, y la segunda mitad es
peor que la primera:** dos packs del banco comprueban `SILENCIO_MS > PULSO_RELE_MS` y **salen en
verde midiendo contra un `1000` que nadie ha medido nunca**, atribuido en el propio comentario a
*«Manual 9, paso 3 de la parametrización»* — **que es un manual NUESTRO, no de Hikvision**. El
manual del fabricante define el parámetro `Delay` y **no publica ni un valor en 110 páginas**.

> 🔴 **Es la forma exacta que este documento ya conoce: una comprobación verde porque una frase
> escrita al lado la justifica, y la frase no la comprueba nadie** (`CLAUDE.md` §2.ter y §3.bis). La
> diferencia con `BluetoothDriver` es que aquí la frase **no está en una lista de excepciones**:
> está en un comentario y en una constante, que es donde nadie va a buscarla.
>
> **La cura no es un pack más:** es medir el `Delay` real con la cámara delante y **derivar**
> `SILENCIO_MS > Delay + rearme`. Es el ensayo `C-2` de la §14 de `3_Protocolo_Pruebas_Rigurosas.md`.

### 5. Lo que este documento NO censa, y por eso las cámaras se le escapan enteras

**Este documento se llama «Cobertura de pruebas y huecos» y censa UNA superficie: la del fuente.**
El banco del 3–4/09 ya lo demostró con tres defectos que ningún `grep` podía ver. **Las cámaras son
la misma lección, pero completa: de las ocho preguntas que hay que contestarles, NINGUNA es una
propiedad del código.**

| lo que hay que saber de la cámara | qué clase de cosa es | con qué se mide |
|---|---|---|
| si `Trigger Alarm Output` está en el menú de Intrusión | **una casilla de una web**, y el fabricante dice *«only supported by certain models»* | mirarla, 10 minutos |
| qué valores admite el `Delay`, y si hay «Manual» | **una lista desplegable** que el manual no publica | mirarla y copiarla |
| qué hace el contacto mientras el objetivo sigue dentro | **el comportamiento de un relé**, no documentado | multímetro y cronómetro |
| cuánto tarda de la entrada al cierre, de noche y con lluvia | **un tiempo real**, 30 veces | cronómetro |
| si es relé seco y qué hace al arrancar | **una resistencia y un transitorio** | ohmímetro |
| qué espera eléctricamente la entrada de alarma | **una tensión y una corriente** que no están escritas | el diagrama que falta |
| si los dos horarios de armado están puestos, con un reloj que deriva | **una hora real, de madrugada** | volver de noche |
| cuánto consume de verdad con los IR encendidos | **una corriente** | pinza amperimétrica |

**Los ocho ensayos, con su hueco de respuesta, están escritos en la §14 de
`3_Protocolo_Pruebas_Rigurosas.md`** como `C-1` … `C-8`. **No se pueden escribir aquí como packs, y
escribirlos como packs sería exactamente el error que el `CLAUDE.md` §2.bis mide en 2,31 a 1.**

> **El orden importa y sale de ahí: `C-1` va primero porque decide si hay diseño.** Si la casilla no
> está, la cámara **no puede darle un bit al controlador** y se cae entero el reparto de
> `DECISIONES.md` D-13; lo que queda en pie es D-14 —el controlador cierra un contacto y la cámara
> **graba**—, que es la única vía documentada de punta a punta hoy.

### 6. Dos hallazgos del propio repaso de hoy, que son sobre la DOCUMENTACIÓN y no sobre el firmware

**a) «La Quick Start Guide no está en disco» es falso — y la conclusión que se sacaba de ahí, cierta
por otro motivo.**

```
$ ls 04_Manuales/*Quick_Start*
04_Manuales/assets.hikvision.com_prd_normal_all_doc_sm000058893_UD40284B_Baseline_1-3_Series_Multilingual_Quick_Start_Guide_20241115.pdf
```

**Sí está, y tiene 40 páginas.** Lo que pasa es que **no trae el diagrama de cable**: medido página a
página, son **8 páginas de dibujos de montaje mecánico y 32 de textos regulatorios en veinte
idiomas**. Y **ninguna de las 40 tiene capa de texto** —cero caracteres extraíbles en el fichero
entero—, así que **cualquier búsqueda de texto sobre ella devuelve cero por el formato, no por lo
que dice.** Es `CLAUDE.md` §4 en su versión de `MAPEO_TARJETA_KICAD.md`: *el buscador estaba, respondía,
y aun así no sabía encontrar*.

> ✅ **Y al mirarla como imagen apareció lo que sí trae, que cierra una deducción escrita como
> deducción.** Su página impresa 8 lleva una tabla de interfaces que dice, literal: *«ALARM OUT:
> 1A and 1B, 2A and 2B, 3A and 3B are three pairs of alarm outputs»* y *«ALARM IN: IN1 and GND1,
> IN2 and GND2 are two pairs of alarm inputs»*. **`1A` y `1B` son los dos terminales del MISMO
> contacto: es lectura, ya no deducción.** Lo que sigue sin estar en ninguna fuente es la **tensión y
> la corriente** que espera la entrada, y el **material del contacto**.

**b) «La cámara no tiene NTP» es impreciso, y la imprecisión importa porque envejece mal.**

Medido sobre las 110 páginas del manual: **`NTP` aparece 7 veces**, todas en *Time Settings*
(PDF pág. 87 / impresa 75). **La cámara SÍ trae cliente NTP.** Lo que no tiene en este diseño es
**red a ningún sitio** (`DECISIONES.md` D-12), así que el NTP no le sirve de nada y la hora entra una
sola vez, desde el portátil del que la configura, con *«Sync. with computer time»* — y a partir de
ahí **deriva sola**. **La conclusión de A-8 no cambia —24×7 en los dos horarios es la única
configuración que no depende de ese reloj— pero el motivo hay que escribirlo bien**, porque una
frase falsa sobre el producto es la que alguien usará dentro de tres meses para descartar una opción
que sí existía.

> **Y el contraste de fuentes que sale de aquí, y vale para las tres:** el manual de usuario que hay
> en disco es la versión **`5.7.20`**; los menús que se van a mirar pueden ser de otra. **Que no
> coincidan no es un contratiempo: es un hallazgo**, y por eso el `C-1` de la §14 empieza anotando la
> versión de firmware de cada cámara antes de tocar nada.

---

## 0. Por qué existe este documento

Los defectos de V9.0 —cámaras en pines de botones, PIN sin validar, radio sin direccionar, Esclavo
que mueve luces por Bluetooth, app incapaz de hablar SPP— **se encontraron uno a uno, preguntando**.
Ninguno salió de una revisión sistemática. Un método que depende de que a alguien se le ocurra la
pregunta correcta **garantiza que quedan defectos**: solo aparecen los que se preguntaron.

Este documento es la pasada que faltaba. Y su primer resultado es un defecto que nadie había visto.

---

## 1. ✅ CERRADO (31/08) — `CAM_UMBRAL_PIN` ya no existe, y las cámaras están en otro sitio

> **El hallazgo era correcto y se ha resuelto por la raíz: `PB8` nunca fue una entrada de cámara.**
> Se conserva íntegro debajo, porque el patrón que describe es reutilizable y volverá a aparecer.

**MEDIDO el 31/08 — el censo que lo cierra:**

```
$ grep -rn "CAM_UMBRAL_PIN" --include=*.cpp --include=*.h Maestro Esclavo
   (sin resultados)

$ grep -rn "PB8" Maestro/include/pines.h Esclavo/include/pines.h
   pines.h:63:  #define LED_TESTIGO   PB8   // -> R16 1K -> LED D5. NO es entrada de camara

$ grep -rn "CAM_C_PIN\|CAM_D_PIN" Maestro/include/pines.h
   pines.h:124:  #define CAM_C_PIN   PB14   // J16 p10 - camara de contacto seco
   pines.h:125:  #define CAM_D_PIN   PB15   // J16 p12 - camara de contacto seco
```

| | 26/08 | **31/08** |
|---|---|---|
| `PB8` | *«cámara de umbral»* con `pinMode()` y sin `digitalRead()` | **`LED_TESTIGO`** — LED `D5` por `R16` de 1 kΩ. **No es entrada** (N-64) |
| Cámaras `C` y `D` | no existían | 🟢 **`CAM_C_PIN` = `PB14`, `CAM_D_PIN` = `PB15`**, `INPUT` pelado y **activo en ALTO** |
| Los cuatro documentos que decían *«Umbral de tramo»* | contradecían al fuente | **caducados**: si alguno lo sigue diciendo, está mal |

~~⚠️ **Lo que NO está cerrado:** el firmware declara los pines, pero **no se cablea cámara a `J16`
hasta la medida `M3`** —con `INPUT` pelado el pin necesita resistencia real a masa en la placa o
queda flotando y el ruido dispara demandas fantasma; de `PB14`/`PB15` **solo lo dice el netlist**—.~~

> 🟢 **`M3` CERRADA en el banco del 3–4/09 (paso 20 de la Guía), y con números.** `PB14` y `PB15`
> —`J16` p10 y p12— miden **`9,93 kΩ` y `9,94 kΩ` contra masa** y están a **`0 V` en reposo**: la
> resistencia real existe en el cobre y ya no depende de que el netlist tenga razón. Con eso se
> cableó la cámara del p10 (paso 21) y **en reposo el equipo no pidió paso solo, con el cable puesto
> y sin él**.
>
> **Lo que se conserva sin tachar, porque no ha caducado:** el orden es **firmware cargado en la
> tarjeta primero, cableado después**. Un commit no protege de un destornillador. En esta sesión se
> cumplió — el paso 2 fue antes que el 21.
>
> ⚠️ **Y la misma medida cerró `M3` y abrió otra cosa:** los pines vecinos del **mando**, `J16` p5 y
> p8, dan esos mismos ~10 kΩ a masa **y `0,6 V` en reposo**. ~~Ahí la resistencia no ayuda: gana al
> pull-up interno, el pin queda en BAJO permanente y **nunca hay flanco que detectar**.~~
>
> 🔧 **Corregido el 04/09 (`N-118`), y la diferencia importa porque cambia el gesto de banco:** ese
> `0,6 V` **no lo ponía el cobre, lo ponía el `INPUT_PULLUP` del firmware** contra esos 10 kΩ. Los
> cuatro pines de `J16` son eléctricamente idénticos y ahora se leen igual —`INPUT` pelado, **activo
> en ALTO**, `346ea5f`—, así que esos 10 kΩ **son el reposo, no el estorbo**.
>
> 🛑 **El gesto para pulsar el mando es `p5` contra `p4` y `p8` contra `p7`** —los 3,3 V del pin
> contiguo—, **nunca contra masa**: en todo `J16` hay **una sola masa** (`p2`), y un cable a masa
> con este firmware **no produce absolutamente nada**. Es además el gesto que precedió al
> calentamiento del paso 29.

### 📕 El hallazgo original, tal como se escribió el 26/08 — se conserva

~~```
$ grep -rn "CAM_UMBRAL_PIN" --include=*.cpp Maestro/src Esclavo/src
Maestro/src/modo_inteligente.cpp:29:  pinMode(CAM_UMBRAL_PIN, INPUT_PULLUP);
Esclavo/src/main.cpp:275:            pinMode(CAM_UMBRAL_PIN, INPUT_PULLUP);
```~~

~~**Eso es todo.** El pin se configura como entrada y **no se lee nunca**: no hay un solo
`digitalRead(CAM_UMBRAL_PIN)` en ninguno de los dos microcontroladores.~~

~~Mientras tanto, **cuatro documentos afirman lo contrario**: el Manual 9 de cámaras, el Manual 10 de
Bluetooth, `ESTADO.md` y `roadmap.md` describen `PB8` como *"Umbral de tramo"* con función activa.~~

> **Un `pinMode()` sin `digitalRead()` es exactamente el patrón de la prueba muerta que este
> repositorio ya conoce:** algo que *parece* conectado, que compila, que pasa la compuerta, y que no
> hace nada. Se descubrió censando, no leyendo el manual — porque el manual decía que sí funcionaba.
>
> **Esta frase se conserva sin tachar: es la lección, y no ha caducado.**

---

## 2. Superficie de entrada completa, cruzada con el banco

Todo lo que puede meter información al sistema, y qué lo prueba:

> ⚠️ **Tabla revisada el 31/08.** Las columnas de recuento se retiran: **el número de comprobaciones
> de cada pack se lee del acta**, no de aquí. Lo que esta tabla dice es **qué existe y qué no**.
>
> 🔬 **Columna nueva el 04/09: «banco».** Un `✅` en la columna de packs dice que **hay un instrumento
> de PC**; la columna de banco dice si esa entrada **se ha ejercido sobre una tarjeta**. Son cosas
> distintas y hasta el 3–4/09 esta tabla sólo sabía decir la primera.

| # | Entrada | Canal | Qué acepta | Pack que lo cubre | Estado | 🔬 Banco 3–4/09 |
|---|---|---|---|---|---|---|
| 1 | ~~4~~ **2** botones locales | ~~`PB9` `PB13` `PB14` `PB15`~~ **`PB9` `PB13`** | secuencias A/B | `maestro_01_mando`, `esclavo_02_inhibicion_menu` | ✅ | 🔴 **ABORTADO** — paso 29, ver abajo |
| 2 | Mando de ~~4~~ **2** relés | **los mismos 2 pines** | idem | idem | ✅ | 🔴 **SIN EJERCER.** ~~sordo en cobre: p5/p8 a `0,6 V` fijos, sin flanco posible~~ → **el cobre estaba bien; era el `pinMode()` (`N-118`, `346ea5f`)**. Gesto: **p5–p4 y p8–p7**, nunca a masa. Confirmación: **destellos rojos**, contables desde el suelo |
| 3 | Cámara de demanda | `PB0` | binario con antirrebote | 🟢 **`camara_01_demanda`** | ✅ *(era ❌)* | 🟡 **media**: `J14` conmuta (pasos 17-18); la respuesta del semáforo, **pendiente** (paso 19) |
| 4 | ~~Cámara de umbral (`PB8`)~~ | — | **no existe: `PB8` es `LED_TESTIGO`** | — | ✅ **cerrado, §1** | — |
| 4.bis | 🆕 **Cámaras `C`/`D` en `J16`** | `PB14` `PB15` | contacto seco, **activo en ALTO** | 🟢 **`camara_02_j16`** | ✅ | 🟢 **`M3` cerrada** (`9,93`/`9,94 kΩ`) y cableada **sin falsa activación** (pasos 20-21) |
| 5 | Radio LoRa | `USART3` | comandos `CMD_*` | `costura_03_comandos` | ⚠️ parcial | 🟢 **enlace y caída a ámbar ejercidos** en 3 cortes (paso 8) |
| 6 | Bluetooth Maestro | `USART1` **por `J17`** (`PB6`/`PB7`) | **17 formas** | 🟢 **`app_01_comandos`, `app_02_modos_simetricos`, `app_03_sin_ok_mudo`, `app_04_valores_de_status`, `app_06_formato_de_hora`** | ✅ *(era ❌)* | 🔴 **el enlace NUNCA se estableció**. `J17` sí quedó confirmado como el UART del ESP32 (pasos 3 y 5) |
| 7 | Bluetooth Esclavo | `USART1` **por `J17`** | 4 aceptados + 3 rechazados | 🟢 **`esclavo_06_no_abre_paso`** + los `app_0x` | ✅ *(era ❌)* | 🔴 ídem |
| 8 | Respaldo en pila | BKP RAM | firma + checksum | `maestro_02_respaldo` | ✅ | ⬜ no ejercido |
| 9 | Reloj / pila | RTC | hora, validez | `maestro_04_sync_horaria`, `esclavo_05_hora_atomica` | ✅ | ⬜ no ejercido |
| 10 | Corte de energía | — | reanudación | `costura_06_reanudacion` | ✅ | ⬜ no ejercido |
| 11 | 🆕 **Reloj `DS3231` del ESP32** | I²C `GPIO21`/`GPIO22` | hora, bit `OSF` | ⚠️ **por censar** — el firmware existe (`ESP32_Expansion/src/reloj_ds3231.cpp`) y **la compuerta lo compila** | ⚠️ | 🔴 **BLOQUEADO** (paso 27): **la única vía de leer o poner esa hora es `SET_RTC` desde la app**, y la app no conectó |

> 🔴 **Léase la columna nueva entera antes de sacar conclusiones, porque dice algo incómodo:** en
> toda la superficie de entrada hay **dos filas con banco en verde** —las cámaras de `J16` y el
> enlace de radio— **y una a medias**. Cinco filas están en rojo y tres sin ejercer. Y **ninguna de
> las verdes es un modo de operación**: todo lo que mueve luces —Automático, Inteligente, Degradado,
> Manual— sigue con la columna de packs en verde y **la de banco vacía**.

~~**El patrón salta a la vista: todo lo anterior a V9.0 está cubierto; todo lo que V9.0 añadió, no.**
Los 20 packs suman `155/155` — exactamente los mismos que antes del Bluetooth, las cámaras y los dos
comandos de radio nuevos.~~

> ## ✅ 31/08 — EL PATRÓN SE HA INVERTIDO, Y ERA EL PUNTO DE ESTE DOCUMENTO
>
> Las cuatro filas que el 26/08 decían **«ninguno»** —cámara de demanda, Bluetooth del Maestro,
> Bluetooth del Esclavo y el pin de umbral— **hoy tienen pack**, y hay packs nuevos que en aquella
> pasada ni se listaron. **La cifra de comprobaciones no se copia aquí**: se lee del acta.
>
> ⚠️ **Y un pack que existe no es un pack que mida.** Antes de contar uno como cobertura hay que
> haberle roto el firmware a propósito y haberlo visto **bajar la cuenta y cambiar el código de
> salida**. Un arnés que nadie ha visto fallar es un adorno que da verde.

### 2.bis Los comandos de radio, uno a uno

~~`protocolo.h` define 18.~~ → **`protocolo.h` define 21** (**MEDIDO el 05/09**:
`grep -cE "^#define CMD_" Maestro/include/protocolo.h` → `21`).

```
CMD_GO_GREEN  CMD_GO_RED  CMD_ACK_GREEN  CMD_PING  CMD_PONG  CMD_ACK_RED
CMD_HORA_D  CMD_HORA_H  CMD_HORA_M  CMD_HORA_S  CMD_ACK_HORA
CMD_DELTA  CMD_DELTA_RESP  CMD_CONFIG_VERDE  CMD_CONFIG_DESPEJE  CMD_ACK_CONFIG
CMD_DEMANDA  CMD_ACK_DEMANDA           <- ya cubiertos por costura_12_acuse_de_demanda
CMD_GO_AMBAR              (0x13)       <- N-134, cubierto por costura_13_ambar_ordenado
CMD_AMBAR_ESCLAVO         (0x14)       <- N-142
CMD_CANCELA_AMBAR_ESCLAVO (0x15)       <- N-152, cubierto por costura_14_cancela_ambar
```

> 🛑 **FALTABAN TRES, Y SON JUSTO LOS DEL ÁMBAR — corregido el 05/09.** La lista se quedó en 18
> mientras N-134, N-142 y N-152 añadían `0x13`, `0x14` y `0x15`. **Es el defecto que este documento
> existe para no cometer:** un comando que no está en ningún cajón es **un hueco sin rastro** —y
> §3 de `CLAUDE.md` lo dice en una línea: *un instrumento que no está en la compuerta no mide nada,
> y no deja rastro de que falta*—.
>
> ⚠️ **Y `CMD_ACK_DEMANDA` ya no está «sin cubrir»**: lo mide `costura_12_acuse_de_demanda`. La
> etiqueta *«los dos nuevos, sin cubrir»* se heredó de la edición en que lo estaban.

---

## 3. Invariantes de seguridad, y quién intenta romperlos

Un invariante sin una prueba que **intente violarlo** es una intención, no una garantía.

| Invariante | ¿Quién lo intenta romper? *(revisado 31/08)* |
|---|---|
| Ningún pin de luz se escribe fuera de `semaforo.cpp` | `barrera_01_pines_de_luz` ✅ — ⚠️ ver el matiz de abajo |
| Nunca verde y rojo a la vez en la misma cabeza (SFTY-2) | `Validacion_Automatico`, sobre pines escritos ✅ |
| ~~**Nunca verde simultáneo en las DOS puntas** — 🔴 nadie~~ | 🟢 **`barrera_02_dos_puntas`** *(era el hueco más caro del documento)* |
| El despeje nunca baja del configurado | ⚠️ indirecto en `costura_02` |
| No se reanuda el Degradado sin autorización válida | `costura_06`, `maestro_03_puerta_degradado` ✅ |
| Un equipo sin hora no autoriza el Degradado | `maestro_04`, `esclavo_05` ✅ |
| ~~**Un comando de Bluetooth no abre paso sin PIN** — 🔴 nadie~~ | 🟢 **`app_01_comandos`, `app_02_modos_simetricos`, `app_03_sin_ok_mudo`** y **`esclavo_06_no_abre_paso`** |
| 🆕 **Un `$ACK` no promete lo que la llamada no devolvió** | 🟢 **`app_03_sin_ok_mudo`** — encontró **dos ramas** que ninguna revisión humana vio |
| 🆕 **La talanquera** | 🟢 `barrera_03_talanquera` |
| **Un equipo no obedece a una pareja ajena** | 🔴 **nadie** — y hoy no hay ni direccionamiento |

~~La tercera fila es la que mata, y es la que este repositorio ya sabe que no mide.~~

> ## ✅ 31/08 — quedan DOS avisos, y no son el mismo que había
>
> **1. El direccionamiento de pareja sigue sin nadie.** Es hoy el único `🔴` de la tabla.
>
> **2. 🔴 `barrera_01_pines_de_luz` mide menos de lo que su nombre promete.** La regla enumera **ocho**
> pines de luz; `escribirPines()` mueve **seis**. `ROJO_PEATON` (`PA6`), `VERDE_PEATON` (`PA7`) y el
> `BUZZER` (`PB1`) están **declarados en `pines.h` y muertos en las dos puntas** —sin `pinMode`, sin
> `digitalRead`, sin `digitalWrite`—, así que la regla era **vacuamente cierta** para dos de sus ocho
> sujetos. El pack **no podía detectarlo**: solo mide *fugas hacia fuera*, acepta `len(luces) >= 6`
> sobre una lista que devuelve 8, y su `control_negativo` **nunca ejerció un pin peatonal**.
>
> > **Una regla de seguridad que enumera sujetos tiene que comprobar que cada sujeto existe**, no solo
> > que nadie la rodea. Es la misma familia que `CAM_UMBRAL_PIN` de la §1: un `✅` en la columna de la
> > izquierda **no dice qué mide el pack**, solo que hay uno.

> ## 🔬 04/09 — Y HAY UN TERCER AVISO, QUE ES EL DE LA COLUMNA QUE FALTA
>
> **Ninguno de los invariantes de esta tabla se ha ejercido sobre una tarjeta.** La columna dice
> *«¿quién lo intenta romper?»* y todas las respuestas son instrumentos de PC. El banco del 3–4/09
> no llegó a ninguno: el equipo nunca entró en un modo de operación.
>
> El que más pesa es el que ya estaba señalado como *«el hueco más caro del documento»*: **el verde
> simultáneo en las dos puntas**. Hoy tiene pack —`barrera_02_dos_puntas`— y tiene arnés de dos
> puntas compilando el C++ real de las dos, que es de donde salió el margen verdadero (**1,44**, no
> el 2 que afirmaban los comentarios). **Lo que no tiene es una lámpara encendida.** Y es
> literalmente la única forma en que este equipo puede matar a alguien.
>
> > **La lección de método, dicha con el número delante:** aquel `🔴 nadie` de la fila se cerró
> > escribiendo un instrumento, y eso estuvo bien porque **contestaba una pregunta abierta**. Pero
> > un instrumento que cierra la fila **no cierra la propiedad**. La fila y la propiedad se parecen
> > tanto que se confunden, y por eso ahora hay dos columnas.

---

## 4. Plan de pruebas — qué escribir, y en qué orden

Cada pack se conecta a `compuerta.py` **solo después de haberlo visto fallar** con un defecto
inyectado a propósito en el `.cpp` real (`CLAUDE.md §8.bis`).

| # | Pack | Qué debe romper | Control negativo obligatorio | **31/08** |
|---|---|---|---|---|
| 1 | ~~`bluetooth_01_autorizacion`~~ | que un comando sin PIN válido abra paso | trama sin PIN → rechazada | ✅ **hecho** como `app_01`/`app_02`/`app_03` |
| 2 | ~~`bluetooth_02_esclavo_no_abre`~~ | que el Esclavo ejecute algo que encienda un verde | `TEST_LEDS` en servicio → rechazado | ✅ **hecho** como `esclavo_06_no_abre_paso` |
| 3 | `costura_08_pareja` | que un equipo atienda una trama de otra pareja | `PAIR` ajeno → descartada | 🔴 **sigue pendiente** |
| 4 | ~~`camara_01_demanda`~~ | antirrebote y que la demanda no acorte el despeje | demanda continua → el despeje **no** baja | ✅ **hecho** — y hay además `camara_02_j16` |
| 5 | `costura_03` (ampliar) | que `0x11`/`0x12` cuadren entre las dos puntas | *(el total se lee del acta)* | ⚠️ por confirmar |
| 6 | ~~`barrera_02_dos_puntas`~~ | **verde simultáneo en las dos puntas** | forzar verde en ambos → debe saltar | ✅ **hecho** |

~~El **6** es el que falta desde siempre y el más caro.~~ **El 6 está escrito.** Lo que queda de este
plan es **el 3** —direccionamiento de pareja— y **confirmar el 5**.

> ⚠️ **Y la §5 del control negativo del punto 5 se corrige:** *«totales `7 → 9`»* era una cifra
> escrita a mano. **Un control negativo se expresa como una propiedad, no como un número que este
> documento tendría que ir persiguiendo**; el recuento vive en el acta.

> ## 🔴 Lo que esta lista NO cubre y hay que añadirle (31/08)
>
> | Qué vigilar | Por qué |
> |---|---|
> | **Que `mando_ambarLocal()` siga teniendo armador** | El veto de SFTY-21 no se rompe borrando un `if`: se rompe borrando **quien pone la bandera a `true`**. Los tres consumidores quedan siempre-verdaderos y **ningún test falla**. Hoy tiene dos armadores —`B·B·B` y `CMD:AMBAR_EMERGENCIA`—; el pack debe exigir que **al menos uno** exista |
> | **Que los pines enumerados por una regla EXISTAN** | Ver §3: `barrera_01` acepta `>= 6` sobre una lista de 8 |
> | **Que los literales que un pack busca por texto sigan donde los busca** | Un refactor puede **apagar un instrumento sin romper un solo test**: si los `"$ACK`/`"$ERR` se mudan de fichero, `app_03` pasa a decir *«ninguna rama promete nada»* y **queda en verde midiendo nada** |
> | **El módulo de expansión ESP32** | Compila en la compuerta, pero su superficie —el despachador, el puente y el `DS3231`— **no está en esta tabla**. Un instrumento que no está en la compuerta no mide nada, **y un hueco no deja rastro de que falta** |

> ## 🔴 Y lo que el BANCO añadió a esa lista el 04/09 — dos huecos de instrumento medidos
>
> | Qué vigilar | Por qué, con lo que costó |
> |---|---|
> | **Cuánto dura DE VERDAD la ventana de arranque del ESP32** | `esp32_02` comprueba el **orden** del armado del watchdog y la **presencia** del reset; `esp32_01` recalcula la desigualdad con `ESP32_ARRANQUE_MS`. Pero esos `1500 ms` son **un número escrito en `contrato.h`**, no una medida — el propio contrato lo declara con `ESP32_ARRANQUE_MEDIDO = 0` y el pack lo saca por `reportar()`, **que no cuenta**. Verde perfecto sobre la propiedad de al lado. En el banco, **el módulo no llegó a anunciarse**, y *cuánto tarda un arranque* no se lee del C++ con ningún `grep` |
> | **El MOTIVO de cada excepción de la lista de huérfanos** | `BluetoothDriver` estuvo meses aceptado en `HUERFANOS_CONOCIDOS` con un motivo **medio cierto**: *«`app.js` habla por `window.bluetoothSerial`»* — usa `write`, `subscribe` y `list`, y **no usa `connect`**, que es la que hace funcionar a las otras tres. **La app nunca abrió un socket** (N-122). El pack estaba bien; lo que nadie medía era **la frase**. Un huérfano se acepta por una razón, y una razón es **una afirmación sobre el código: se comprueba, no se escribe** |
> | **La superficie que NO es código** | Corriente, temperatura, tiempo de arranque real, resistencias del cobre. Los tres defectos que pararon el banco viven ahí y **ningún pack posible los alcanza**. No es una fila que se cierre escribiendo Python: se cierra con un óhmetro, un amperímetro y un cronómetro, y su sitio es la Guía de banco — **no esta tabla** |

---

## 5. Qué NO cubre este documento

- ~~**La app móvil no tiene ninguna prueba**, de ningún tipo. Ni unitaria, ni de integración.~~
  ✅ **Superado el 31/08:** la app tiene hoy **cuatro instrumentos en la compuerta** —simulador de app
  y Bluetooth, test funcional, test unitarios y **la app ejecutada en DOM**—, más los packs `app_0x`
  que la cruzan contra el firmware. **Sus cifras se leen del acta.**

  > 🔴 **Pero el aviso que sustituye a aquel es más incómodo:** esos instrumentos ya estuvieron
  > **los dos en `ABORTADO` a la vez** —un pack buscaba una rama en `app.js` cuando el parser se
  > había mudado a `js/`, y el arnés de DOM reventaba con un `TypeError` sobre una pestaña que ya no
  > existía—, y eran justo **los dos únicos que ejercen la app**. Detrás entraron cuatro defectos.
  > **Mientras un instrumento está abortado, todo lo que vigilaba entra sin mirar**: un `ABORTADO` no
  > se apunta para luego, se arregla antes de mirar nada más.

- 🆕 **Este documento no mide la interfaz.** El contraste WCAG y el comportamiento a distintos anchos
  tienen su propio arnés; y **una captura a un solo ancho no es una prueba de interfaz** — las de
  `evidencia/` se hicieron a 412 px, el único de cuatro donde cierto corte no aparecía.

- ~~**Nada de esto sustituye la prueba de banco.**~~ **Sigue siendo cierto, y desde el 04/09 ya no
  hace falta creerlo: está medido.** Un pack demuestra que el modelo y el código coinciden; no que la
  tarjeta encienda una bombilla. El paquete `617bd00` fue al banco con la compuerta entera en verde y
  **la sesión se paró tres veces, por tres cosas que ningún pack podía ver**. **En campo corre la
  V8.4.**

- 🆕 **Este documento no mide el hardware, y ahora se sabe cuánto pesa eso.** Consumo, temperatura,
  tiempos reales de arranque y resistencias del cobre **no tienen aquí ni una fila** y no la pueden
  tener: no están en el repositorio. Quien busque esa cobertura la tiene en la
  `Guia_Cableado_y_Pruebas_Banco.html` y en el informe devuelto,
  `evidencia/Informe_Pruebas_Banco_Semaforos_V9.0.pdf`. **Un censo del fuente que se lea como censo
  del equipo es exactamente el error que esta página existe para no cometer.**

- 🆕 **Y desde el 05/09, tampoco mide los COMPONENTES COMPRADOS.** Las dos cámaras
  `DS-2CD2683G2-IZS` son hoy el único sujeto del sistema del que **no existe una sola medida**, y las
  ocho preguntas que hay que hacerles **no son propiedades del código**: una casilla de un menú web,
  una lista desplegable, la duración de un pulso, un tiempo de respuesta, una corriente, un reloj que
  deriva. **Aquí no puede haber una fila para ninguna.** Están escritas como ensayos ejecutables, con
  su hueco de respuesta, en la **§14 de `3_Protocolo_Pruebas_Rigurosas.md`** (`C-1` … `C-8`).
  **Escribirlas como packs sería la industria de sustitución que el `CLAUDE.md` §2.bis mide en 2,31
  a 1: un instrumento nuevo que certifica otra vez lo ya certificado, en vez de contestar la pregunta
  abierta.**


---

## 6. ✅ CERRADO (31/08) — ~~`PB0`/`PB8` están asignados dos veces~~ · **el reloj se fue del STM32**

> # 🟢 LA DISPUTA POR LOS PINES YA NO EXISTE, Y NO SE RESOLVIÓ CON UN EXPANSOR
>
> **Toda esta sección se sostenía sobre una premisa que hoy es falsa:** que el `DS3231` tenía que
> colgar de `PB0`/`PB8` del STM32. **Ya no.**
>
> | | 26/08 | **31/08 — MEDIDO** |
> |---|---|---|
> | Dónde vive el `DS3231` | *«en `PB0`/`PB8` del STM32, hacen falta DOS»* | **en el módulo de expansión ESP32**, `GPIO21` (`SDA`) / `GPIO22` (`SCL`), **con pila propia** |
> | Driver | *«no hay»* | 🟢 **existe y compila**: `01_Firmware/ESP32_Expansion/src/reloj_ds3231.cpp` |
> | `PB8` | *«cámara de umbral»* | **`LED_TESTIGO`** — LED `D5` por `R16` 1 kΩ. **No es entrada** |
> | `PB0` | disputado | **`CAM_DEMANDA_PIN`, sin disputa** |
> | Cámaras `C`/`D` | sin pines | **`PB14`/`PB15`** (`J16` p10/p12), liberados por el mando |
>
> **La cuenta que bloqueaba —«1 + 2 = 3 > 2»— ya no se plantea:** el reloj no pide pines del STM32 y
> las cámaras tienen tres entradas (`PB0`, `PB14`, `PB15`).
>
> ### 🛑 Y por tanto la propuesta del `PCF8574` QUEDA RETIRADA
>
> No porque fuera mala —resolvía bien el problema que había—, sino porque **el problema desapareció**.
> Montarlo hoy añadiría un segundo esclavo a un bus I²C por software **sin necesidad**, y con él el
> riesgo que la propia sección anotaba: *«hay que comprobar que la latencia de leer el expansor no
> compite con el `IWDG` ni con el bit-bang del LCD»*. **Ese riesgo ya no hace falta correrlo.**
>
> ### 📌 Lo que de esta sección SIGUE EN PIE
>
> - **`Y2` está muerto** (N-37, banco del 01/08). El STM32 no puede fiarse de su cristal, y por eso
>   el reloj bueno vive ahora en el ESP32. **No es un plan: el firmware existe.**
> - 🆕 **Y desde el 3–4/09 tampoco es un plan la placa: existe y está armada** —fuente conmutada
>   `12 V → 5 V`, `DS3231` por I²C en `GPIO21`/`GPIO22` y salida a `J17`—, con la masa común medida
>   a `0 V` contra la STM32 (paso 23). **Lo que falta son dos medidas, y las dos son de banco:** la
>   fuente **nunca se midió con carga real de 12 V** (todo se alimentó por USB), y **la hora del
>   `DS3231` sólo se puede leer o poner con `SET_RTC` desde la app** — o sea que mientras el enlace
>   no exista, ese reloj **no es verificable desde fuera por ninguna vía**.
> - **Sin reloj no hay Modo Degradado**, y SFTY-18 lo prohíbe con razón.
> - **La lección de método, que es lo que no caduca:** un documento fechado **antes** de una medida
>   de banco no contradice esa medida — **está sin actualizar**, y hay que corregirlo antes de que
>   alguien lo cite como si valiera. Es exactamente lo que le acaba de pasar a esta sección.
>
> **Lo que sigue se conserva sin borrar, como histórico del razonamiento.**

---

### 📕 HISTÓRICO — el hallazgo del 26/08, superado

Esta pasada cruzó **todos** los documentos que nombran `PB0` o `PB8`. El resultado no es una errata:

| Documento | Dice que `PB0`/`PB8` son… | Fecha |
|---|---|---|
| `roadmap.md` **N-37** | **`SDA`/`SCL` del `DS3231`** — *"los únicos pines libres"* | **cerrado en banco el 01/08/2026** |
| Manual 11 · `MANUAL_INSTALACION_RELOJ_DS3231.md` | `SDA`/`SCL` del `DS3231` | V9.0 |
| Manual 9 · Manual 2 · `MAPEO:179` · `ESTADO.md` | **cámaras IA** (demanda y umbral) | V9.0 |

**No caben las dos cosas.** Y la que llegó primero tiene evidencia de banco detrás.

### Lo que dice N-37, y por qué no es negociable

> *"**CERRADO POR ELIMINACIÓN: el cristal `Y2` está MUERTO. Banco del 01/08/2026.**"*

Con tres eliminaciones medidas —`VBAT` a 3 V con la tarjeta apagada, el reintento de `N-25`, y
`REINICIAR RELOJ` devolviendo `SIGUE PARADO`—. Y su conclusión: *"Ya no queda software que probar.
**Salida: `DS3231` por I²C software en `PB0`/`PB8`.** Hacen falta **DOS**"*.

**El `MAPEO §4` dice lo contrario** —*"que el cristal sea el culpable no está medido"*— pero su
cabecera lo fecha: **«Última revisión: 31 de julio de 2026»**, un día **antes** de la medida de
banco. No es una contradicción: es un documento sin actualizar, y hay que corregirlo antes de que
alguien lo cite como si valiera.

### Por qué esto invalida la arquitectura de V9.0

1. El firmware sigue en `STM32RTC` sobre el cristal muerto (`reloj.cpp:109`, `LSE_CLOCK`). **No hay
   driver `DS3231`.**
2. V9.0 acaba de darle a las cámaras **los dos pines que el reloj necesita**.
3. Sin reloj no hay Modo Degradado: `SFTY-18` lo prohíbe, y con razón.

### Y la salida que aparece al mirarlo entero

De los dos pines de cámara, **`PB8` no se lee nunca** (§1). O sea que la demanda real necesita **un
solo pin**, no dos. Y aun así 1 + 2 = 3 > 2.

**Un expansor `PCF8574` en el mismo bus I²C lo resuelve:** cuelga de `PB0`/`PB8` junto al `DS3231`,
cuesta céntimos, y aporta **8 entradas digitales** para las cámaras — con lo que sobran seis.

```
   PB0 (SDA) ──┬─────────────┬───────────────
   PB8 (SCL) ──┴──┐       ┌──┴──┐
                  │       │     │
              [DS3231]  [PCF8574]  ← 8 entradas: camaras 1..4 y margen
```

**Ventaja adicional que no es menor:** deja de haber pines libres en disputa. Cualquier entrada
futura entra por el expansor sin volver a abrir esta discusión.

**Riesgo a medir, no a suponer:** el I²C es por software y ahora tendría dos esclavos; hay que
comprobar que la latencia de leer el expansor no compite con el `IWDG` ni con el bit-bang del LCD.
Eso se mide en banco, no aquí.
