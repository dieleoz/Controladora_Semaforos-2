# DECISIONES — una fila por decisión, y sólo la VIGENTE

> **Por qué existe este fichero.** El 05/09 se lanzó un agente a retirar el mando de relés
> de las dos puntas sobre una frase dicha de viva voz. La decisión contraria estaba escrita
> desde el 31/08 en `05_Funcional/17_...`, y no se encontró porque **las decisiones de este
> repositorio se añaden sin derogar las anteriores**: `roadmap.md` tiene 4.700 líneas y un
> `grep` devuelve la versión que case primero. Hubo que matar al agente.
>
> **Regla: una decisión vigente vive AQUÍ, en una línea. Todo lo demás apunta a este
> fichero en vez de repetirla.** El porqué largo se queda en el roadmap; lo que manda es
> esta tabla. Si una fila de aquí y un párrafo de allí no coinciden, **gana ésta**, y el
> párrafo está caducado.

**Antes de encargar un cambio de alcance —a un agente o a ti mismo— se lee esta tabla.**
Si el encargo contradice una fila, eso no es una orden: **es una pregunta.**

---

## Vigentes

| # | decisión | fecha | motivo | deroga |
|---|---|---|---|---|
| **D-1** | **El mando de relés SE CONSERVA en los canales `A` y `B`** (`J16` p5 `PB9`, p8 `PB13`). Van cableados, hoy sin usar, **y su código NO se toca** | 31/08, reconfirmada 05/09 | `mando_ambarLocal()` tiene **seis lectores** y su veto es SFTY-21. Retirar el armador de la bandera deja los `if` siempre verdaderos: **el veto no queda inerte, queda abierto**. Y medido: el banco se caería en **ABORTADO**, no en rojo — dos `raise` disparan solos y los dos modelos leen constantes de `mando.cpp` **en el import** | «se retira el mando de 4 relés» (28/08) |
| **D-2** | **`BOTON3` (`PB14`, p10) y `BOTON4` (`PB15`, p12) son las DOS CÁMARAS** | 28/08 | son los pines que las cámaras necesitan y los que el mando no usa | los cuatro pulsadores |
| **D-3** | **`M3` CERRADA: las cámaras se cablean a `J16`** | 03/09 | medido en cobre: pull-down real de 10 kΩ en las cuatro posiciones (`R65`–`R68`), p10 y p12 a **0 V** en reposo, y el paso 21 cableó p10 contra p11 **sin demandas fantasma** | «no se cablea cámara a `J16` hasta M3» |
| **D-4** | **`J16` p1 se TAPA en cada equipo que se monte** | — | lleva **12 V crudos** a un conector de señal directa al micro (N-120) | «cautela de banco» |
| **D-5** | **Mínimo por sentido = 3 minutos** (`VERDE_MIN_MIN = 3`) | 04/09 | por debajo, el conductor se convence de que el semáforo está averiado y adelanta en rojo | `VERDE_MIN_MIN = 1` |
| **D-6** | **La pantalla LCD NO se retira** | 04/09 | 271 comprobaciones cuelgan de ella | «se retira la pantalla» (28/08) y la «Ola D» del roadmap |
| **D-7** | **En Manual, `DAR PASO` alterna rojo/verde como el automático**, disparado por el botón. Termina en **rojo+verde**, no rojo+ámbar. El todo-rojo de despeje **se queda** | 04/09 | el automático también lo hace, y es lo que garantiza que el tramo quedó vacío. Configurable 10–90 s, hoy 15 | «Manual lleva su propio ciclo» |
| **D-8** | **El ámbar de emergencia conserva SUS DOS VETOS** (mando y app) | 04/09 | el banco tumbó **dos veces** la versión sin cerrojo. Al medirlo, el veto **no era** la causa del bloqueo: lo era que esa punta no acusaba | la decisión «(b), sin cerrojo cuando viene de la app» |
| **D-9** | **La hora la pone el DS3231 del ESP32**; el STM32 no tiene reloj (`Y2` muerto, N-17) | 04/09 | el puente rellena el hueco `HORA:--:--:--` al pasar la trama y recalcula el CRC | «la hora la lleva el RTC interno del STM32» |
| **D-10** | **Cámara comprada: Hikvision `DS-2CD2683G2-IZS`** | 05/09 | tiene salida de alarma (`1 in, 1 out, 24 V/1 A`, ficha oficial) | los modelos anteriores de la lista |
| **D-13** | **LAS DOS CÁMARAS LLEVAN LA MISMA CONFIGURACIÓN. Una sola regla: *Intrusion Detection* sobre el BARRIDO DE LA PLUMA —no la zona de espera—. El SIGNIFICADO lo pone el estado del semáforo, no la cámara** | 05/09 | Con **una cámara por poste**, dar «un significado a cada una» no es repartir: es **proteger un poste y el otro no**. Y el estado de la luz es lo único que el controlador tiene y la cámara no sabe, así que **un bit da cinco lecturas** sin gastar significados. Un vehículo que espera correctamente para **antes** de la pluma: no entra en la región | «un significado por cámara»; ~~«vehículo detenido en el tramo»~~; ~~«conteo entradas/salidas»~~ |
| **D-15** | **El reloj lo lleva el ESP32 de cada punta, y es el ÚNICO que contesta a `SET_RTC`. La app valida la hora en LAS DOS** | 05/09 | Hay **dos relojes por cruce** —un DS3231 con pila por ESP32— y el STM32 no tiene ninguno (`Y2` muerto, N-17). Que el STM32 siguiera contestando producía **dos acuses opuestos a una sola orden**, los dos ciertos. Decidido por delegación explícita del responsable | «el STM32 contesta a `SET_RTC`»; el camino de sincronización que sincroniza **el reloj del STM32** |
| **D-11** | **Al aplicar tiempos, la app AVISA y da el botón: NO arranca el ciclo sola** | 05/09 | arrancar el ciclo abre paso, y hacerlo automáticamente se salta la confirmación de vía (§6) | — |
| **D-12** | **De cada cámara el sistema consume UN CONTACTO SECO. No hay red, no hay imagen, no hay vídeo, y no hay analítica en el controlador** | 05/09 | medido: cero `WiFi`, `HTTPClient`, servidor u ONVIF en todo el ESP32; el STM32 sólo lee un pin. **Consecuencia: toda la inteligencia vive en la CONFIGURACIÓN de la cámara**, y el manual de parametrización pasa de documento de apoyo a entregable principal. **Y lo que se pierde: el CONTROLADOR no ve imagen.** ⚠️ **CORREGIDO el 05/09: la cámara SÍ graba en su propia microSD (hasta 512 GB, ficha oficial)**, así que el soporte de accidentes y la auditoría **sí son posibles** — en la cámara, no en nuestro firmware. Es una tarjeta que hay que comprar y configurar, y no toca una línea de código | «imágenes y auditoría en la Raspberry o la Nano», propuesto el 04/09 y recomendado por dos revisiones el 05/09 **sin comprobar que hubiera camino** |

---

## 🟡 Abiertas — y aquí NO se decide por descarte

| # | pregunta | qué falta para poder decidirla |
|---|---|---|
| **A-0** | **¿Se compran las microSD y se activa la grabación en cámara?** | Recupera el uso que el responsable les había encontrado —soporte de accidentes, auditoría— y **no toca el firmware**. Falta: tamaño de tarjeta, días de retención, y si la grabación va continua o por evento |
| **A-1.bis** | 🔴 **El veto de la pluma NO es gratis: contradice SFTY-28 y un arnés armado** | «la talanquera SIGUE al semáforo, nunca al revés» (`barrera_03_talanquera.py`), y `Validacion_Automatico` exige **que no haya pluma arriba sin verde**. Un veto que la deja arriba en rojo **rompe ese invariante**. Se dijo «sin discusión» tres veces hoy: **es falso**. Hace falta derogarlo por escrito, con su excepción en SFTY-28 y el arnés reescrito con control negativo. Y **el veto NO puede llevar tope que fuerce la bajada**: un tope que baja igual devuelve el peligro que el veto evita — tope → **alarma**, no acción |
| **A-1** | **¿Qué significa cada uno de los DOS bits que tenemos?** | Ver **D-12**: no hay red ni imágenes, así que **una cámara = una salida = UN significado**, y con dos cámaras hay **dos significados para todo el cruce**. Uno está tomado (presencia antes de bajar la pluma). El otro está por decidir entre: **invasión en rojo** (ocurre EN la línea de pare, así que el alcance no la limita), **espera prolongada**, **vigilancia de la propia cámara**, o ~~**conteo asimétrico entradas/salidas**~~ **IMPOSIBLE con un bit por cámara**: distinguir entrada de salida exige sentido, y `A<->B` da un pulso sin sentido mientras `A->B` pierde las salidas. Todos como EVENTO, no como control. ~~vehículo detenido en el tramo~~ **DESCARTADA por el responsable: la cámara no ve 2 km de tramo**. Y el conteo sólo vale en la dirección segura —«entraron más de los que salieron» → alargar el todo-rojo—, **nunca** «ya está vacío» → acortar el despeje |
| **A-2** | **¿Qué se pone en `J16` p5 y p8?** | Quedan libres si algún día se retira el mando — hoy **no se retira** (D-1). Idea sin decidir: **fin de carrera de la talanquera**, que hoy es **lazo abierto** |
| ~~**A-3**~~ ✅ | ~~¿A quién le habla el operario con el reloj?~~ **DECIDIDO 05/09, delegado por el responsable: contesta QUIEN TIENE EL RELOJ.** El STM32 deja de atender `SET_RTC` —su `Y2` está muerto y el reloj ya no es suyo—, y el puente, que lleva el DS3231 con pila, es el único que acusa. **Una orden, un acuse.** Y la app **valida la hora en LAS DOS puntas**, no sólo en la conectada. Ver **D-15** |
| **A-4** | **¿Qué pasa con `MENU` si se replantea la interfaz?** | Es el estado «parado» del que depende fijar tiempos: `C_MENU_IDLE` fuerza rojo a las dos puntas |
| ~~**A-5**~~ ✅ | ~~¿Había un DS3231 en el banco?~~ **RESUELTA 05/09: sí — cada ESP32 lleva su reloj con pila propia**, y así estaba escrito desde el 28/08 en la lista de compras. `HORA:22:19:58` es real y **N-145 queda confirmada en cobre**. La contradicción era de la lista (`A6` dice «NO se compró» y está caducada), no del banco. Sigue sin verificar `0x68` sobre el módulo |
| **A-6** | **La vigilancia de la propia cámara — y va PRIMERO** | No compite por el bit: **es la condición previa de todas las demás**, porque con `INPUT` pelado y pull-down **el pin no distingue silencio de vía libre**. «N horas sin flanco con el ciclo corriendo» se calcula sobre cualquier bit. Sólo eventos, sin efecto vial: es **el instrumento del laboratorio**, y cuenta cuántas veces habría actuado un veto **antes** de darle autoridad. La enunció el responsable («lleva 8 días…») y **no existe** |
| **A-7** | 🔴 **El `~1 s` del relé es CIRCULAR: nos lo inventamos y luego nos citamos** | `demanda.cpp` de las dos puntas justifica `SILENCIO_MS = 3000` con *«el relé de la AcuSense cierra ~1 s por detección»*, y `camara_01_demanda.py:39` lo atribuye a **«Manual 9, paso 3»** — que es **una instrucción NUESTRA**, no un dato de Hikvision. **El manual oficial no publica ni un valor de `Delay` en 110 páginas.** La cura: medir el `Delay` real, fijarlo al **mínimo** que admita, y **derivar** `SILENCIO_MS > Delay + rearme` con un pack que relea las dos cifras. Es N-71 otra vez |
| **A-8** | **Los DOS horarios de armado, y la cámara sin NTP** | Hay **dos `Arming Schedule` en serie** —el de la regla y el de la propia salida de alarma— y fuera de cualquiera **el relé no cierra**. Y la cámara **no tiene NTP** (D-12: sin red), así que su horario corre sobre un reloj que **deriva y se pierde en un corte**. La única configuración que no depende de ese reloj es **24×7 en los dos**. Se comprueba de madrugada tras un corte, no en taller |

---

## Cómo se cambia una fila

1. Se escribe la nueva, con **fecha y motivo medido**. Un motivo sin números se deroga de
   palabra: eso es lo que pasó con D-1.
2. La anterior se **tacha aquí y se deja**, no se borra — una decisión que desaparece en
   silencio vuelve a proponerse dentro de un mes.
3. Si la decisión **retira una barrera**, además se censa quién depende de ella antes de
   tocar nada (`CLAUDE.md` §3.ter).

---

## D-13 · El diseño de las cámaras, desarrollado

### En la cámara — **las dos igual**

*Intrusion Detection* sobre **el barrido de la pluma**. `Threshold` al mínimo · `Sensitivity`
alta · `Size Filter` que excluya perros y hojas · **sin filtro de objetivo** (bajo la pluma
importa también una moto o una persona, y además `Detection Target` **no está documentado**
para Intrusión) · `Trigger Alarm Output` **sólo en esta regla** (el enlace es común a todas
las armadas: si Motion o Tampering también lo marcan, el bit deja de significar una cosa) ·
`Delay` al **mínimo** que admita · **los DOS `Arming Schedule` a 24×7** (A-8).

### En el controlador — **un bit, cinco lecturas**

| flanco cuando… | significa | qué hace |
|---|---|---|
| va a **bajar la pluma** | presencia debajo | **no baja**, `$EVENT`, reintenta cada N s. Si persiste → `$ALARM`. **La pluma sigue arriba** — la luz ya está en rojo |
| **rojo** con la pluma abajo | **invasión** | `$EVENT` con hora |
| **verde** | paso normal | cuenta silenciosa: alimenta el vigilante |
| **N horas sin flanco** con el ciclo corriendo | cámara ciega o tapada | `$ALARM` |
| **nivel alto sostenido > T** | cámara pegada | `$ALARM` |

### Orden de ejecución — y las dos primeras fases NO tocan el ciclo

| | qué | depende de |
|---|---|---|
| **1** | instalar · activar · IP · **microSD y grabación** | 🔴 **comprar las tarjetas** (A-0). Cero firmware, valor inmediato |
| **2** | **mirar la casilla `Trigger Alarm Output`** | 10 min con la cámara delante. **Decide todo lo demás** |
| **3** | medir el **`Delay`** y el tiempo de respuesta | de ahí se **deriva** `SILENCIO_MS > Delay + rearme` (A-7) |
| **4** | firmware fase 1: **el vigilante, los eventos Y SU PANTALLA** | **cero efecto vial.** Es el contador que dice si el veto merece la pena — y por eso **no se construye sin dónde leerlo**: hoy `camara` aparece **cero veces** en `app.js` y la talanquera sólo sale en textos que explican botones, **nunca como estado**. Un contador que nadie lee es lo que este repositorio lleva pagando |
| **5** | firmware fase 2: **el veto de la pluma** | **sólo si la fase 4 da números que lo justifiquen**, y con la derogación de SFTY-28 escrita (A-1.bis) |

### Lo que el equipo tiene que PUBLICAR, y hoy no publica

Sin esto la fase 1 no puede decidir la fase 5, y la fase 5 sería invisible para el operario.
Dos campos en el `$STATUS` del Maestro, con la misma disciplina que `ESC:` —incluido su `?`
honesto—:

| campo | valores | por qué |
|---|---|---|
| **`PLUMA:`** | `ARRIBA` · `ABAJO` | Es un elemento físico que se mueve y **la app no lo enseña**. Con D-13 pasa a haber ratos con **luz roja y pluma arriba**, que hoy un operario lee como avería. **No depende de las cámaras: se puede publicar ya** |
| **`CAM:`** | `OK` · `CIEGA` · `PEGADA` · `?` | Sale del vigilante. Sin él, «no llega bit» y «no hay nadie» son indistinguibles |

Y en la app: la **pluma dibujada en el cruce**, el **estado de cada cámara**, y el **contador
de la fase 1** («el veto habría actuado N veces»). Los eventos ya caen solos en el registro.

> **Por qué esto no se había hecho:** se esperaba que las cámaras alimentaran el Modo
> Inteligente, y ese modo no se usa. **No es que se olvidara dibujarlo: es que no había dato
> que dibujar.** Con D-13 lo hay, y por primera vez llega a los modos que se operan.

### Lo que NO se hace, y no es negociable

- **Nada que AUTORICE por ausencia** — acortar el despeje, dar verde antes. Una cámara
  desconectada lee «no hay nadie».
- **Ningún tope que fuerce la bajada** de la pluma: devolvería el peligro que el veto evita.
  Tope → **alarma**, nunca acción.
- **El ciclo del semáforo no se toca.** Las cámaras no dan ni quitan verde.

---

## D-14 · LA ENTRADA DE ALARMA — la vía que NO depende de la casilla bloqueada

**Verificado en el manual del fabricante, y sin la coletilla que bloquea la otra vía.**

Hasta ahora sólo se miraba la **salida** de la cámara. La **entrada** abre el canal contrario:
**el controlador le dice algo a la cámara**.

| qué dice el manual | dónde |
|---|---|
| *Record Schedule*, tipo **Alarm**: *«When alarm input is enabled and trigger recording is selected as linkage method, the video is recorded after receiving alarm signal from external alarm input device»* | PDF p.48 / impresa 36 |
| *Trigger Recording*: *«the device records the video about the detected alarm event»* — **SIN nota de modelos** | PDF p.82 / impresa 70 |
| Y la nota *«only supported by certain models»* acompaña **sólo** a `Trigger Alarm Output`, `Flashing Light` y `Audible Warning` | PDF p.79, 82, 83 |

> 🟢 **Consecuencia: «el controlador cierra un contacto en ROJO → la cámara graba» NO depende
> de `Trigger Alarm Output`.** Es la única vía documentada de punta a punta hoy.

### Y trae algo que no habíamos pedido: la cámara sabe hacer la AND

El *Record Schedule* admite **`Motion & Alarm`** — graba **sólo si hay movimiento Y señal en la
entrada— y también `Motion | Alarm`** (PDF p.48 / impresa 36).

**Con el contacto cerrado en rojo, `Motion & Alarm` graba exactamente «algo se movió mientras
el carril estaba cerrado».** Es la invasión, filtrada **por la cámara**, sin necesitar que la
analítica accione ningún relé. La AND que íbamos a hacer en el controlador **ya la hace ella**.

### 🔴 Lo que falta, y es documentación que NO tenemos

**El manual DELEGA el cableado**: *«Make sure the external alarm device is connected. **See
Quick Start Guide for cable connection**»* (PDF p.56 / impresa 44). **La Quick Start Guide no
está en disco.**

Por eso `ALARM IN`, `G`, `1A`, `1B` quedan **`SIN VERIFICAR`**: no hay diagrama de cable en
ninguna de las dos fuentes. Lo único que acota es que la ficha dice **`1 input, 1 output`**
(p.3): si `1A` y `1B` fueran dos salidas, diría «2 outputs». **Que sean los dos terminales de
un único contacto es DEDUCCIÓN, no lectura de diagrama.** Y de `G` no se sabe si es común de
la entrada, de la salida o de las dos.

**Hace falta la Quick Start Guide antes de cablear nada.**

### Otros `SIN VERIFICAR` que este repaso deja cerrados como tales

- **`Delay` de la salida: CERO números en 110 páginas.** Sólo la definición. **A-7 confirmada:
  el «~1 s» no sale de Hikvision, sale de un manual nuestro.**
- **Qué espera eléctricamente la entrada**: sin tensión, sin corriente, y las palabras *«dry
  contact»* y *«relay»* **no aparecen** en las 110 páginas.
- **`Alarm Type` de la entrada**: el manual documenta el valor **`NO`**; **`NC` no aparece
  nunca** → sin verificar que sea configurable.
- **Corriente mínima de conmutación** de la salida, y si es relé mecánico o de estado sólido.
- Y **dos horarios en serie otra vez** (el de la entrada y el del *Record Schedule*), igual
  que A-8.

### ⚠️ La recopilación `.docx` contradice a la ficha oficial

Dice **256 GB** de microSD donde la ficha del 03/03/2023 dice **512 GB** (p.3), y recomienda
**NTP**, que choca con D-12 (la cámara no tiene red hacia nuestro sistema). **No es fuente del
fabricante y no se cita como tal.**
