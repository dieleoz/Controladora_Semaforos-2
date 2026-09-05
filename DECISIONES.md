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

# 🟡 LO QUE TE TOCA DECIDIR — el índice de treinta segundos

*Actualizado el 05/09 de madrugada. Cada fila se desarrolla más abajo con sus salidas, lo que
bloquea y lo que cuesta.*

| # | en una línea | urgencia | qué está parado mientras tanto |
|---|---|---|---|
| **A-12** | 🔴 **El Modo Inteligente corta un verde a los 15 s**, por debajo de tus 3 minutos, y ninguna guarda lo toca | **la compuerta está en ROJO por esto** | el arreglo (una línea) y **el Modo Inteligente entero**: subir el piso puede dejarlo sin lógica de cámaras |
| ~~**A-11**~~ | ✅ **RESUELTA el 05/09: «esto ya es por app»**. Se le añade `SET_MODO:DEGRADADO` al Esclavo, llamando a `degradado_entrar()` —**la puerta única que YA existe**, con su criterio único—. No se reconstruye la puerta: se le da la llave. ⚠️ **Y arrastra dos cosas medidas que van en el mismo lote:** `degradado_entrar()` devuelve un `RechazoDegradado`, no un `bool` —**cada motivo necesita su `$ERR`**, o es un `$ACK` que miente (§6)—; y **el Esclavo no publica NADA sobre su Degradado** (`grep reportarEvento` en su `modo_degradado.cpp` → **cero**), así que un botón para entrar en un modo que no se puede ver es **media función** | pasa a **`D-18`** |
| **A-1.bis** | **¿Se deroga SFTY-28** («la talanquera sigue a la luz, nunca al revés»**)?** | media — bloquea la fase 2 | **la fase 2 de D-13**: el veto de la pluma. La fase 1 ya está construida |
| **A-13** | **Dónde va el campo `CAM:` del `$STATUS`**: no caben sus 11 caracteres en ninguna de las dos puntas | media | que la app **enseñe el estado de las cámaras**. Los eventos y el contador ya salen por `$EVENT`/`$ALARM` |
| **A-0** | **Cómo se configura la grabación de las microSD** (comprarlas ya está decidido) | baja — no toca firmware | parametrizar la cámara (paso 1 de D-13) |
| **A-7** | **Medir el `Delay` real del relé de la cámara** — el «~1 s» nos lo inventamos y luego nos citamos | baja, pero **hay dos packs en verde contra ese número** | derivar `SILENCIO_MS` de un dato de verdad |
| **A-8** | **Los dos `Arming Schedule` en serie**, sobre un reloj de cámara que no se puede sincronizar | baja | parametrizar la cámara |
| **A-2** | **Qué se pone en `J16` p5 y p8** — 🔴 **y ahora choca con A-11**, ver abajo | media (se ha vuelto una decisión de seguridad) | nada hoy; **pero cualquier cosa que se cablee ahí entra por el reconocedor de secuencias del mando** |
| **A-4** | **Qué pasa con `MENU`** si se replantea la interfaz | baja | nada hoy |
| **A-10** | **El LED `D21` de `VERDE2`** — comprobación de banco, ya no es decisión de firmware | banco | nada |

> 🔴 **Y dos filas vivas de este mismo fichero se contradicen entre sí — ver
> [«Filas que chocan»](#filas-que-chocan) al final.** Es exactamente el fallo que este
> fichero existe para impedir, y esta vez lo cazó el fichero.

---

## Vigentes

| # | decisión | fecha | motivo | deroga |
|---|---|---|---|---|
| **D-1** | 🔴 **EL MANDO DE RELÉS NO EXISTE: el equipo se opera SÓLO POR APP. Y su CÓDIGO no se toca.** Las dos cosas a la vez | 31/08 · hardware confirmado retirado el **05/09**: *«ya no tenemos mandos de A y B, sólo la app»* | **El hardware se fue** (lista de compras rev. 3, 28/08). **El código se queda**, y el motivo está medido: `mando_ambarLocal()` tiene **CINCO llamadas vivas** —tres vetos en `Esclavo/src/main.cpp` y dos decisiones de `CANCELAR_AMBAR` en `Esclavo/src/bluetooth.cpp`— y su veto es SFTY-21; retirar el armador deja los `if` siempre verdaderos —**el veto no queda inerte, queda abierto**— y además **el banco se caería en ABORTADO, no en rojo**: son **TRECE packs** (los dos modelos leen constantes de `mando.cpp` **en el import**). Con el mando desmontado la bandera simplemente **no se arma nunca**, que es lo correcto | «se retira el mando de 4 relés» (28/08, que se leía como retirar también el código) |
| **D-2** | **`BOTON3` (`PB14`, p10) y `BOTON4` (`PB15`, p12) son las DOS CÁMARAS** | 28/08 | son los pines que las cámaras necesitan y los que el mando no usa | los cuatro pulsadores |
| **D-3** | **`M3` CERRADA: las cámaras se cablean a `J16`** | 03/09 | medido en cobre: pull-down real de 10 kΩ en las cuatro posiciones (`R65`–`R68`), p10 y p12 a **0 V** en reposo, y el paso 21 cableó p10 contra p11 **sin demandas fantasma** | «no se cablea cámara a `J16` hasta M3» |
| **D-4** | **`J16` p1 se TAPA en cada equipo que se monte** | — | lleva **12 V crudos** a un conector de señal directa al micro (N-120) | «cautela de banco» |
| **D-5** | **Mínimo por sentido = 3 minutos** (`VERDE_MIN_MIN = 3`) | 04/09 | por debajo, el conductor se convence de que el semáforo está averiado y adelanta en rojo | `VERDE_MIN_MIN = 1` |
| ~~**D-6**~~ | ~~**La pantalla LCD NO se retira**~~ — 🔴 **DEROGADA el 05/09 por el responsable**: *«la pantalla LCD ya no va, pues los pines y el equipo lo quitamos»*. **Se retira del EQUIPO. El `lcd.cpp` sigue compilando y `Validacion_LCD` sigue siendo una de las 20 filas de la compuerta con `271/271`**: el arnés mide un framebuffer en el PC, no una ST7920, así que **no se cae al no montar la pantalla**. Lo que sí muere es el MENÚ como interfaz —y con él `MODO_HORA`, cuyo único armador vive ahí—. Ver `D-18` | ~~04/09~~ · **derogada 05/09** | el motivo original —*«271 comprobaciones cuelgan de ella»*— **era falso como argumento para no retirarla**: cuelgan del arnés, que no necesita la pantalla física | «se retira la pantalla» (28/08) y la «Ola D» del roadmap |
| **D-7** | **En Manual, `DAR PASO` alterna rojo/verde como el automático**, disparado por el botón. Termina en **rojo+verde**, no rojo+ámbar. El todo-rojo de despeje **se queda** | 04/09 | el automático también lo hace, y es lo que garantiza que el tramo quedó vacío. Configurable 10–90 s, hoy 15 | «Manual lleva su propio ciclo» |
| **D-8** | **El ámbar de emergencia conserva SUS DOS VETOS** (mando y app) | 04/09 | el banco tumbó **dos veces** la versión sin cerrojo. Al medirlo, el veto **no era** la causa del bloqueo: lo era que esa punta no acusaba | la decisión «(b), sin cerrojo cuando viene de la app» |
| **D-9** | **La hora la pone el DS3231 del ESP32**; el STM32 no tiene reloj (`Y2` muerto, N-17) | 04/09 | el puente rellena el hueco `HORA:--:--:--` al pasar la trama y recalcula el CRC | «la hora la lleva el RTC interno del STM32» |
| **D-10** | **Cámara comprada: Hikvision `DS-2CD2683G2-IZS`** | 05/09 | tiene salida de alarma (`1 in, 1 out, 24 V/1 A`, ficha oficial) | los modelos anteriores de la lista |
| **D-11** | **Al aplicar tiempos, la app AVISA y da el botón: NO arranca el ciclo sola** | 05/09 | arrancar el ciclo abre paso, y hacerlo automáticamente se salta la confirmación de vía (§6) | — |
| **D-12** | **De cada cámara el sistema consume UN CONTACTO SECO. No hay red, no hay imagen, no hay vídeo, y no hay analítica en el controlador** | 05/09 | medido: cero `WiFi`, `HTTPClient`, servidor u ONVIF en todo el ESP32; el STM32 sólo lee un pin. **Consecuencia: toda la inteligencia vive en la CONFIGURACIÓN de la cámara**, y el manual de parametrización pasa de documento de apoyo a entregable principal. **Y lo que se pierde: el CONTROLADOR no ve imagen.** ⚠️ **CORREGIDO el 05/09: la cámara SÍ graba en su propia microSD (hasta 512 GB, ficha oficial)**, así que el soporte de accidentes y la auditoría **sí son posibles** — en la cámara, no en nuestro firmware | «imágenes y auditoría en la Raspberry o la Nano», propuesto el 04/09 y recomendado por dos revisiones el 05/09 **sin comprobar que hubiera camino** |
| **D-13** | **LAS DOS CÁMARAS LLEVAN LA MISMA CONFIGURACIÓN. Una sola regla: *Intrusion Detection* sobre el BARRIDO DE LA PLUMA —no la zona de espera—. El SIGNIFICADO lo pone el estado del semáforo, no la cámara** | 05/09 · **fase 1 CONSTRUIDA el 05/09 en `4b90f98`** | Con **una cámara por poste**, dar «un significado a cada una» no es repartir: es **proteger un poste y el otro no**. Y el estado de la luz es lo único que el controlador tiene y la cámara no sabe, así que **un bit da cinco lecturas** sin gastar significados | «un significado por cámara»; ~~«vehículo detenido en el tramo»~~; ~~«conteo entradas/salidas»~~ |
| **D-14** | **La ENTRADA de alarma de la cámara (grabar cuando el controlador cierra un contacto) es la vía que NO depende de la casilla bloqueada** | 05/09 | verificado en el manual del fabricante: *Trigger Recording* **no lleva** la nota «only supported by certain models» que sí acompaña a `Trigger Alarm Output`. Desarrollo completo [más abajo](#d-14--la-entrada-de-alarma--la-vía-que-no-depende-de-la-casilla-bloqueada) | «sólo se mira la salida de la cámara» |
| **D-15** | **El reloj lo lleva el ESP32 de cada punta, y es el ÚNICO que contesta a `SET_RTC`. La app valida la hora en LAS DOS** | 05/09 · construido en `5e4076d` | Hay **dos relojes por cruce** —un DS3231 con pila por ESP32— y el STM32 no tiene ninguno (`Y2` muerto, N-17). Que el STM32 siguiera contestando producía **dos acuses opuestos a una sola orden**, los dos ciertos. Decidido por delegación explícita del responsable | «el STM32 contesta a `SET_RTC`»; el camino de sincronización que sincroniza **el reloj del STM32** |
| **D-17.bis** | **LA PANTALLA LCD Y EL MENÚJ SE RETIRAN DEL EQUIPO. Todo se opera por la app** | 05/09 | Deroga `D-6`. ⚠️ **Y el matiz que hay que sostener, porque decir «la pantalla no existe» sería otra frase falsa: se retira del EQUIPO, no del código.** `lcd.cpp` compila, `menu.cpp` compila, y `Validacion_LCD` sigue dando `271/271` sobre un framebuffer en el PC. Lo que muere es la INTERFAZ, y con ella `MODO_HORA` —único armador en `menu.cpp`, detrás de un `botonAceptar()` que es `return false;`— y `MODO_ALCANCE`, cuya única salida es `lcd_dibujarAlcance()`. 🔴 **Y NO se retira `botonArriba()`/`botonAbajo()`: siguen vivos y leen `BOTON1`/`BOTON2`, que son los pines del mando** | ~~`D-6`~~ · ~~«el menú es la única vía sin radios»~~ (Regla de Oro del `MANUAL_USUARIO`, derogada: la app por Bluetooth **tampoco depende de las radios**) |
| **D-18** | **El Modo Degradado del poste 2 se pide POR APP** | 05/09 | Resuelve `A-11`. La puerta —`degradado_entrar()`— ya estaba construida y probada (`18/18` en el arnés de dos puntas); lo que se retiró fue la llave, que era el mando. 🔴 **Y cambia quién arbitra el ciclo**, que es el motivo de que no lo cerrara un agente por su cuenta: el Degradado es **el único modo que da verde sin confirmar la otra punta**. Se mide y se escribe qué hace el Maestro mientras el Esclavo está dentro | ~~la salida (b): dos pulsadores físicos en el Esclavo~~ — contradecía `D-1` y competía con `A-2` por `J16` p5/p8 |
| **D-16** | 🔴 **SIN TELÉFONO NO HAY FORMA DE OPERAR EL EQUIPO. Es una propiedad DECLARADA del sistema, no una avería** | 05/09 | Consecuencia directa de **D-1**: retirado el mando, **la app es la única superficie de mando**. Ni ámbar, ni volver a automático, ni parar el cruce. Y no es teórico: esta semana hubo que **desvincular el Maestro en Ajustes de Android** para poder conectarse al Esclavo. **Va escrito en el manual del operario**: el teléfono es herramienta crítica —batería, cable, y conviene un segundo terminal emparejado—. ⚠️ **Y en el Esclavo es peor de lo que decía esta fila: ver A-11** | la idea de que «siempre queda el mando desde el suelo» |
| **D-17** | **`CMD:LEER_RTC` — el reloj se puede CONSULTAR sin cambiarlo**, en Maestro, Esclavo y teléfono, y la app enseña el **desfase entre postes** | 05/09 · construido en `5846cee` | Es la respuesta del responsable a `A-9`, y **es mejor que sincronizar**: hasta hoy la única forma de leer el reloj era mandarlo, y con eso se perdía justo el dato que se buscaba. **No hace falta que los dos relojes se pongan de acuerdo solos: hace falta poder ver si lo están.** Los dos ESP32 no se hablan, así que la comparación sólo la puede hacer la app visitando los dos postes | «hay que sincronizar los dos relojes entre sí» |

---

# 🟡 Abiertas — y aquí NO se decide por descarte

Para cada una: **qué se decide · qué pasa con cada salida · qué está bloqueado · cuánto cuesta.**
Donde el coste no está medido, lo dice.

---

## 🔴 A-12 · El Modo Inteligente corta un verde a los 15 segundos

**La compuerta está en `19 PASS · 1 FALLA · 0 ABORTADO` por esta línea, y eso es correcto: es el
hallazgo, no una regresión.** Lo acusa `app_11_rangos_de_tiempos` (13/14), reparado en `2d17678`
—antes **no juzgaba ninguna línea** y salía verde por vacío—.

**Lo medido, y se puede reproducir en diez segundos:**

```
$ grep -n "15000" 01_Firmware/Maestro/src/modo_inteligente.cpp
123:        if (tiempoActual >= 15000UL) {

$ grep -n "VERDE_MIN_MIN" 01_Firmware/Maestro/include/limites_ciclo.h
54:static const uint8_t VERDE_MIN_MIN = 3,  VERDE_MIN_MAX = 15;
```

Esa comparación, dentro de `modoInteligente_loop()`, decide cuándo se puede **cortar un verde en
marcha**. **No pasa por `SET_TIEMPOS` ni por el menú**, así que ninguna de las dos guardas del
mínimo vial la toca. Y está en el **único modo que usa las cámaras**: con una cámara pegada en «hay
presencia», esa punta recibiría verdes de 15 s ciclo tras ciclo mientras la otra corre a 3 minutos.
**Estaba escrito antes de comprar las cámaras: el hardware nuevo no lo trae, lo encuentra.**

### 🔴 Y lo que hace que esto NO sea un arreglo mecánico — medido, no razonado

`maxVerde` en ese modo **no es configurable**: vale `VERDE_MIN_MIN` y no se le asigna otra cosa en
todo el fichero.

```
$ grep -n "maxVerde\s*=" 01_Firmware/Maestro/src/modo_inteligente.cpp
49:static int maxVerde = VERDE_MIN_MIN, segEstatico = DESPEJE_SEG_MIN;
75:  maxVerde = VERDE_MIN_MIN;
```

O sea que la **Regla 2** del mismo bucle —`if (tiempoActual >= duracionMaxima)`, con
`duracionMaxima = maxVerde * 60000`— **ya dispara a los 3 minutos exactos**, y siempre.

> **Consecuencia: subir el piso de la Regla 1 a los 3 minutos la deja SIN PODER DECIDIR NADA.**
> Cualquier instante en que la Regla 1 pudiera cortar por demanda, la Regla 2 ya ha cortado por
> tiempo. El Modo Inteligente se convierte en **un alternador fijo de 3 minutos**, y las cámaras
> dejan de tener el menor efecto sobre el ciclo. Es §3.septies otra vez: una guarda que ya no puede
> dar las dos respuestas.

**Por eso la pregunta no es «¿subimos el 15 a 180?». Son dos números y hay que dar los dos:**

| salida | qué queda | qué se pierde |
|---|---|---|
| **(a)** piso = 3 min, techo = 3 min *(el arreglo de una línea)* | cumple `D-5` con la letra | **el Modo Inteligente deja de ser inteligente**: alternador fijo de 3 min, las cámaras no cortan nunca. Nadie lo notaría en verde |
| **(b)** piso = 3 min y **techo mayor** (p. ej. 6–15 min) | cumple `D-5` **y** la demanda sigue pudiendo cortar, entre el minuto 3 y el techo | un vehículo que llega en el minuto 1 espera hasta el 3. Es lo que `D-5` dice que hay que hacer |
| **(c)** dejarlo en 15 s | el modo alterna por demanda desde el primer segundo | **contradice `D-5` a la cara**, y con una cámara pegada el cruce alterna al mínimo indefinidamente. La compuerta se queda en rojo |
| **(d)** retirar el Modo Inteligente | el problema desaparece | se tira el único modo que usa las cámaras, con las cámaras ya compradas |

**Qué está bloqueado:** el arreglo, y con él **volver la compuerta a verde**. Mientras tanto no se
puede distinguir «la compuerta está roja por esto» de «la compuerta está rota».

**Cuánto cuesta:** el cambio **es de una línea** y `app_11` está **verificado por inyección** (sabe
fallar con 15 000 dentro de la puerta y sabe estar verde con 180 000). **El coste en flash NO está
medido** — sustituir un literal por una constante ya enlazada no debería costar bytes, pero eso es
una expectativa, no una medida. **Lo que sí es seguro es que no es mecánico: es vial, y lo decides
tú.**

---

## 🔴 A-11 · El Modo Degradado del Esclavo se quedó sin ninguna puerta

**Salió al censar `D-16`, y ningún documento lo decía.** El modo existe, está construido, tiene su
propio arnés (`arnes del Degradado a dos puntas`, 18/18) — y **nadie puede pedirlo en esa punta.**

**Las tres puertas, medidas una a una:**

| puerta | estado hoy | la medida |
|---|---|---|
| **Bluetooth `SET_MODO`** | **NO EXISTE en el Esclavo** | `grep -c "SET_MODO" 01_Firmware/Esclavo/src/bluetooth.cpp` → **0**. En el Maestro hay **8** (`ALCANCE`, `AMBAR`, `AUTO`, `DEGRADADO`, `HORA`, `INTELIGENTE`, `MANUAL`, `MENU`) |
| **el menú de la pantalla** | **INALCANZABLE en las dos puntas** | `menu.cpp` sí llama a `degradado_entrar()`, pero se navega con `botonAceptar()`/`botonCancelar()`, y los dos son `return false;` (`Esclavo/src/botones.cpp:550-551`, `Maestro/src/botones.cpp:539-540`) desde que `BOTON3`/`BOTON4` pasaron a ser cámaras (`D-2`) |
| **el mando, secuencia `A.B.A.B`** | 🔴 **el CÓDIGO SIGUE VIVO Y SIGUE LEYENDO LOS PINES; lo que falta es el hardware** | `botones_actualizar()` llama a `mando_registrarPulso(MANDO_A/B)` en cada flanco de `BOTON1`(`PB9`, `J16` p5) y `BOTON2`(`PB13`, p8), y `mando.cpp` reconoce `A.B.A.B` → `confirmarYActuar(ACC_DEGRADADO)`. **Los pulsadores se retiraron (`D-1`)** |
| **la radio** | **no vale por definición** | su muerte es justo la razón de entrar al Degradado |

> El propio comentario del firmware lo dice, y lleva ahí desde el 31/08:
> *«en esta punta el mando pasa a ser la ÚNICA forma de entrar o salir del Degradado sin la app»*
> (`Esclavo/src/botones.cpp`, bloque SFTY-21). **La app nunca llegó, y el mando se fue.**

**Las salidas, con lo que cada una cambia:**

| salida | qué cuesta | qué cambia de fondo |
|---|---|---|
| **(a)** añadir `SET_MODO` al `bluetooth.cpp` del Esclavo | firmware en la punta subordinada + app + packs. **No medido en bytes** | 🔴 **cambia quién arbitra el ciclo.** Hoy el Esclavo es `MODO:SUBORDINADO` y no decide; darle un `SET_MODO` le da autoridad propia, con el Maestro pudiendo ordenarle otra cosa a la vez. **Por eso no lo cierra un agente** |
| **(b)** volver a poner **dos pulsadores** en `J16` p5 y p8 **del Esclavo** | **CERO firmware** — el camino ya está construido y los pines se leen | 🔴 **contradice `D-1`** («sólo la app») y **compite con `A-2`** por esos mismos dos pines. Y **el cobre con el binario nuevo no está medido**: `d020f3c` dejó esa medida escrita como *prueba cancelada*, no como casilla pendiente |
| **(c)** dejarlo así | cero | **el Esclavo no puede entrar en Degradado nunca.** Es una función terminada, probada y sin usuario posible |

**Qué está bloqueado mientras tanto:** el uso real del Degradado en un corte de radio, que es
precisamente el escenario para el que se construyó.

---

## A-1.bis · El veto de la pluma NO es gratis: contradice SFTY-28 y un arnés armado

**Qué se decide:** si se **deroga por escrito** la regla *«la talanquera SIGUE al semáforo, nunca al
revés»* (SFTY-28) para permitir que una cámara **impida que la pluma baje** cuando hay algo debajo.

**Lo que hay hoy, medido:** SFTY-28 vive en `*/src/semaforo.cpp` dentro de `escribirPines()`, y la
vigilan **dos packs** (`barrera_03_talanquera`, `maestro_09_test_leds`, tabla de `OPTIMIZACIONES.md`).
Además `Validacion_Automatico` exige **que no haya pluma arriba sin verde**. Un veto que la deja
arriba en rojo **rompe ese invariante**.

| salida | consecuencia |
|---|---|
| **derogar** | hay que reescribir SFTY-28 con su excepción **y** el arnés con control negativo, y aceptar ratos de **luz roja con pluma arriba** — que un operario lee hoy como avería (por eso `PLUMA:` se publica ya, N-153) |
| **no derogar** | **la fase 2 de `D-13` no se construye.** La fase 1 —el contador que dice cuántas veces habría actuado el veto— sí, y ya está |

🔴 **Se dijo «sin discusión» tres veces el 05/09: es falso.** Y **el veto NO puede llevar tope que
fuerce la bajada**: un tope que baja igual devuelve el peligro que el veto evita — tope →
**alarma**, no acción.

**Coste:** no medido. Lo que sí está medido es que la fase 1 no lo necesita: `4b90f98` observa la
transición ya hecha (`camVetos++` + `$EVENT`) sin entrar en `escribirPines()`.

---

## A-13 · Dónde va el campo `CAM:` del `$STATUS` *(hay un agente midiéndolo ahora)*

**No cabe, y está medido por buffer —no por rango— en las dos puntas** (`e43a8e7`, N-154):

| punta | plantilla | libre | `,CAM:PEGADA` pide | falta |
|---|---|---|---|---|
| **Maestro** | 141 B de `payload[144]` | **3 B** | **11 B** | 8 B |
| **Esclavo** | 123 B de `payload[128]` | **5 B** | **11 B** | 6 B |

*(11 = la coma + `CAM:` + `PEGADA`, el más largo de los cuatro valores que devuelve
`camara_estado()`: `OK` · `CIEGA` · `PEGADA` · `?`.)*

🔴 **Y una corrección a lo que el propio comentario del fuente ofrece como alternativa: acotar
`HORA:` NO ALCANZA POR SÍ SOLO.** Validar hora/minuto/segundo baja ese campo de 11 a 8 y **devuelve
3 B** — Maestro quedaría en 6 libres y Esclavo en 8, **y hacen falta 11 en las dos**. La única
salida medida que llega es **subir `payload` hacia los 155 B** que impone de techo real
`tramaCompleta[160]`; las dos cosas juntas dan holgura de sobra.

| salida | qué cuesta |
|---|---|
| **subir `payload` a 155** | RAM de pila, **no medida en bytes**. Es el techo real, no un número inventado |
| **acotar `HORA:` además** | recupera 3 B más y quita un valor imposible de la trama |
| **acotar `HORA:` sola** | **no llega.** Medido arriba |
| **no publicar `CAM:`** | «no llega bit» y «no hay nadie» siguen siendo indistinguibles para el operario |

**Qué está bloqueado:** que la app enseñe el **estado** de cada cámara. **Los eventos y el contador
del veto NO están bloqueados**: salen ya por `$EVENT`/`$ALARM` —`camara_alarmar()` y
`VETO_HABRIA_ACTUADO_N:`— y la app los registra (lee `$STATUS`, `$ALARM`, `$ACK`, `$EVENT`, `$ERR`).

⚠️ **Consecuencia hoy: `camara_estado()` está declarada y SIN NINGÚN LLAMADOR** en las dos puntas.
Es un huérfano **deliberado y con su motivo medido**, anotado en `costura_10_funciones_muertas` y
**re-medido en cada corrida** por `camara_03_vigilante` —que exige que deje de ser excepción en
cuanto el `$STATUS` publique `CAM:`—. Es la forma correcta de dejar obra a medias (§3.bis), pero
**es obra a medias**.

---

## A-0 · La configuración de la grabación en microSD

✅ **La COMPRA ya está decidida**, el 05/09 por el responsable: *«cada cámara tiene una micro, la
metemos»* — **2 unidades `high endurance`**, entra como `A10` en la lista de compras
(`05_Funcional/15_Lista_de_Compras_Hardware.md`).

**Lo que sigue abierto es la configuración, y no bloquea comprar:** capacidad, días de retención, y
si la grabación va **continua o por evento**. Recupera el uso que le habías encontrado —soporte de
accidentes y auditoría— y **no toca una línea de firmware**.

⚠️ **La capacidad máxima tiene dos fuentes que no coinciden:** la ficha oficial del 03/03/2023 dice
**512 GB**; una recopilación `.docx` que **no es del fabricante** dice 256 GB.

---

## A-7 · 🔴 El `~1 s` del relé es CIRCULAR: nos lo inventamos y luego nos citamos

`demanda.cpp` de las dos puntas justifica `SILENCIO_MS = 3000` con *«el relé de la AcuSense cierra
~1 s por detección»*, y un pack lo atribuye a **«Manual 9, paso 3»** — que es **una instrucción
NUESTRA**, no un dato de Hikvision. **El manual oficial no publica ni un valor de `Delay` en 110
páginas** (confirmado en el repaso de `D-14`).

🔴 **Y es peor de lo escrito: el «~1 s» vive en NUEVE sitios** —cinco comentarios de firmware y
**DOS PACKS con `PULSO_RELE_MS = 1000`** que comprueban `SILENCIO_MS > PULSO_RELE_MS` y **salen
VERDES contra un número que nadie ha medido**. El instrumento certifica la invención.

**La cura:** medir el `Delay` real con la cámara delante (paso 3 de `D-13`, y **ya hay hueco para
él en la guía de banco**, pasos 39–40, `2d17678`), fijarlo al **mínimo** que admita, y **derivar**
`SILENCIO_MS > Delay + rearme` con un pack que relea las dos cifras. Es N-71 otra vez.

---

## A-8 · Los DOS horarios de armado, y el reloj de la cámara sin sincronizar

Hay **dos `Arming Schedule` en serie** —el de la regla y el de la propia salida de alarma— y fuera
de cualquiera de los dos **el relé no cierra**. Y la cámara **no PUEDE USAR NTP** —el cliente
existe, `NTP` sale 7 veces en el manual; lo que no hay es **red** (`D-12`)—, así que su horario corre
sobre un reloj que **deriva y se pierde en un corte**.

**La única configuración que no depende de ese reloj es `24×7` en los dos.** Se comprueba de
madrugada tras un corte, no en taller.

---

## A-2 · Qué se pone en `J16` p5 y p8 — 🔴 y ahora es una decisión de SEGURIDAD

**La premisa cambió el 05/09 y la fila no se había actualizado.** Decía *«quedan libres si algún día
se retira el mando — hoy no se retira»*. Hoy: **el mando físico se retiró** (`D-1`), así que los dos
pines están **libres en el cobre** — pero **el firmware los sigue leyendo**, y por eso el hueco
**no es un hueco neutro**.

🔴 **Lo medido, y no estaba escrito en ninguna parte:** `botones_actualizar()` alimenta el
reconocedor de secuencias del mando con **cada flanco** de esos dos pines:

```
$ grep -n "mando_registrarPulso" 01_Firmware/Esclavo/src/botones.cpp
491:  if (flanco[0]) mando_registrarPulso(MANDO_A);
492:  if (flanco[1]) mando_registrarPulso(MANDO_B);
```

Y en `Esclavo/src/mando.cpp` esas secuencias ejecutan: `A.A.A` → **volver a obedecer al Maestro**,
`B.B.B` → **ámbar local con su cerrojo SFTY-21**, `A.B.A.B` → **entrar en Modo Degradado**.

> **O sea: cualquier cosa que se cablee a `J16` p5/p8 —un fin de carrera de talanquera, la idea que
> estaba anotada aquí— compone secuencias del mando sin que nadie lo pida.** Una pluma que sube y
> baja tres veces seguidas dentro de la ventana es `B.B.B`. Esto **no está medido en banco**; lo que
> está medido es que el código que lo haría **está vivo y lee esos pines**.

**Las salidas:**

| salida | consecuencia |
|---|---|
| **dejarlos vacíos** | no pasa nada. Es lo que hay hoy |
| **fin de carrera de la talanquera** (la idea original) | cierra el lazo abierto de la pluma **y** puede disparar el mando. Exigiría **desarmar el reconocedor**, que es tocar el código que `D-1` dice que no se toca |
| **dos pulsadores en el Esclavo** | es la salida **(b)** de `A-11`, y cuesta **cero firmware** — pero contradice `D-1` |

---

## A-4 · Qué pasa con `MENU` si se replantea la interfaz

Es el estado «parado» del que depende fijar tiempos: `C_MENU_IDLE` fuerza rojo a las dos puntas.

⚠️ **Contexto que ha cambiado y hay que tener delante al decidirlo:** el menú **ya no se puede
navegar** en ninguna de las dos puntas —`botonAceptar()`/`botonCancelar()` son `return false;`—,
así que hoy `MENU` sólo se alcanza por `SET_MODO:MENU` desde la app, y sólo en el Maestro.

---

## A-10 · El LED `D21` de `VERDE2` con una pata al aire

✅ **Resuelto el alcance por el responsable: NO ES PROBLEMA DE FIRMWARE.** *«El firmware lo enciende,
así que no es problema; es una revisión o confirmación del funcional en el `.html`.»* El firmware
manda esa salida igual que las otras nueve —`escribirPines()` no distingue—, así que lo único que
puede faltar es el **indicador**, no la luz.

Pasa a ser una **comprobación de banco** (paso 7.bis de la guía, `2d17678`): mirar si ese LED
enciende con `VERDE2` activo. **Si no enciende, es cobre y hay que saberlo antes de fabricar más.**

---

# ✅ Cerradas — se tachan con su commit, no se borran

| # | qué era | cómo se cerró |
|---|---|---|
| ~~**A-1**~~ | ~~¿Qué significa cada uno de los dos bits?~~ | **CERRADA por `D-13`** (`108d882`): las dos cámaras llevan la misma configuración y el significado lo pone el estado del semáforo. Un bit, cinco lecturas |
| ~~**A-3**~~ | ~~¿A quién le habla el operario con el reloj?~~ | **DECIDIDO 05/09, delegado por el responsable: contesta QUIEN TIENE EL RELOJ.** Ver `D-15`, construido en `5e4076d`. Una orden, un acuse; y la app valida la hora en **las dos** puntas |
| ~~**A-5**~~ | ~~¿Había un DS3231 en el banco?~~ | **RESUELTA 05/09** (`08c9d36`): sí — cada ESP32 lleva su reloj con pila propia, y así estaba escrito desde el 28/08 en la lista de compras. `HORA:22:19:58` es real y **N-145 queda confirmada en cobre**. ⚠️ Sigue **sin verificar** `0x68` sobre el módulo |
| ~~**A-6**~~ | ~~La vigilancia de la propia cámara — «la enunció el responsable y NO EXISTE»~~ | 🟢 **CONSTRUIDA el 05/09 en `4b90f98`** (fase 1 de `D-13`, N-157), en **las dos puntas y con cero efecto vial**: la única función vial en las 238 líneas nuevas es `semaforo_plumaArriba()`, **y se lee**. `CAM_PEGADA_MS = 20 min` **derivado** del techo del ciclo y recalculado del C++ por su pack (N-71); `CAM_CIEGA_MS = 6 h **de paso abierto**`, que **no sale de ninguna constante del firmware y así está escrito** —cuánto tarda el siguiente vehículo es propiedad de la carretera, y fabricarle una derivación sería `A-7` otra vez—. Y el contador `camVetos` **observa** la transición en vez de vetarla. 🔴 **Lo que NO debe leerse como aprobado: `CAM_CIEGA` a su valor de producción son 6 h, no ejecutables en una sesión de banco.** El camino está comprobado en su **forma**, no en su **tiempo**. Y su lectura en pantalla depende de `A-13` |
| ~~**A-9**~~ | ~~Dos relojes por cruce y nada los sincroniza~~ | **RESUELTA 05/09 por el responsable y CONSTRUIDA en `5846cee`** (ver `D-17`). Sigue en pie el aviso para `AB-4`: el día que el Degradado cuelgue del DS3231, **el desfase inicial no tiene cota** — pero ahora al menos **se mide** |
| ~~**N-118**~~ | ~~`MANDO_A`/`MANDO_B` no responden: 0,6 V en reposo, «defecto de placa»~~ | 🟢 **REFUTADO el 05/09 en `d020f3c`, con la medida del propio banco.** En `617bd00` —**el binario que estaba en la tarjeta durante aquel banco**— `BOTON1/2` iban en `INPUT_PULLUP` y `CAM_C/D_PIN` en `INPUT` pelado. El paso 20 midió **9,92–9,94 kΩ en los cuatro pines**, y **0,6 V sólo en los dos con pull-up y 0 V en los dos sin él**: mismo cobre, distinto `pinMode`, distinta tensión. **El banco había corrido las dos ramas del experimento en la misma tabla y nadie lo leyó así.** Y además es moot: **ya no hay mando** (`D-1`). ⚠️ La tensión de `J16` p5/p8 con el binario nuevo queda escrita como **prueba CANCELADA**, no como casilla pendiente: una casilla abierta invita a puentear `J16`, que es el gesto que precedió al calentamiento del paso 29 |

---

<a id="filas-que-chocan"></a>

# 🔴 Filas que chocan — lo que este fichero existe para cazar

**Tres pares de decisiones vivas se contradicen entre sí. Ninguna es un descuido de redacción: las
tres son decisiones reales que compiten.**

### 1. `D-5` contra el firmware que corre — y la compuerta ya lo dice

`D-5` fija el mínimo vial en **3 minutos** (04/09). `modo_inteligente.cpp` corta un verde a los
**15 segundos**, y ninguna de las dos guardas del mínimo pasa por ahí. **No es una fila contra otra
fila: es una fila contra el código, y el instrumento está en rojo por ello.** → **`A-12`**.

### 2. `A-2` contra `A-11`(b) — los dos quieren `J16` p5 y p8, y por motivos opuestos

`A-2` propone poner ahí **un fin de carrera de talanquera**. La salida más barata de `A-11` es
poner ahí **dos pulsadores** para devolverle al Esclavo la puerta del Degradado. **No caben los
dos**, y hay algo peor: **el firmware sigue leyendo esos pines y alimentando el reconocedor de
secuencias del mando**, así que el fin de carrera de `A-2` **dispararía las órdenes del mando por
accidente** —`B.B.B` es ámbar local, `A.B.A.B` es entrar en Degradado—. Decidir `A-2` sin mirar
`A-11` deja el cruce obedeciendo a una pluma.

### 3. `D-13` fase 4 contra lo que se construyó anoche

La tabla de ejecución de `D-13` dice, de la fase 1: *«el vigilante, los eventos **Y SU PANTALLA**…
no se construye sin dónde leerlo: un contador que nadie lee es lo que este repositorio lleva
pagando»*. **Se construyó igual** (`4b90f98`), y `camara_estado()` **quedó sin un solo llamador**
porque el `CAM:` no cabe en el `$STATUS` (`A-13`).

**No es tan grave como suena, y la diferencia importa:** *los eventos y el contador SÍ tienen dónde
leerse* —salen por `$EVENT`/`$ALARM` y la app los registra—. **Lo que no tiene pantalla es el
ESTADO** de cada cámara. O sea que la fase 1 cumple su propósito —dar el número que decide la fase
2— y lo que falta es el semáforo de salud de la cámara. **Aun así, la regla estaba escrita y se
saltó; queda anotado aquí en vez de arreglarse en silencio.**

---

## Cómo se cambia una fila

1. Se escribe la nueva, con **fecha y motivo medido**. Un motivo sin números se deroga de
   palabra: eso es lo que pasó con D-1.
2. La anterior se **tacha aquí y se deja**, no se borra — una decisión que desaparece en
   silencio vuelve a proponerse dentro de un mes.
3. Si la decisión **retira una barrera**, además se censa quién depende de ella antes de
   tocar nada (`CLAUDE.md` §3.ter).
4. **Se cierra con el commit que lo demuestra**, no con una afirmación (Convenciones de
   `CLAUDE.md`).

---

## D-13 · El diseño de las cámaras, desarrollado

### En la cámara — **las dos igual**

*Intrusion Detection* sobre **el barrido de la pluma**. `Threshold` al mínimo · `Sensitivity`
alta · `Size Filter` que excluya perros y hojas · **sin filtro de objetivo** (bajo la pluma
importa también una moto o una persona, y además `Detection Target` **no está documentado**
para Intrusión) · `Trigger Alarm Output` **sólo en esta regla** (el enlace es común a todas
las armadas: si Motion o Tampering también lo marcan, el bit deja de significar una cosa) ·
`Delay` al **mínimo** que admita (A-7) · **los DOS `Arming Schedule` a 24×7** (A-8).

### En el controlador — **un bit, cinco lecturas**

| flanco cuando… | significa | qué hace | estado |
|---|---|---|---|
| va a **bajar la pluma** | presencia debajo | **fase 2:** no baja, `$EVENT`, reintenta. **Fase 1 (hoy):** la transición se **observa**, `camVetos++` y `$EVENT` | 🟢 observado (`4b90f98`) · el veto espera a `A-1.bis` |
| **rojo** con la pluma abajo | **invasión** | `$EVENT` con hora | 🟢 |
| **verde** | paso normal | cuenta silenciosa: alimenta el vigilante | 🟢 |
| **N horas sin flanco** con el paso abierto | cámara ciega o tapada | `$ALARM` `CAM_CIEGA` a las **6 h de pluma arriba** | 🟢 construido · **no ejercido en su tiempo real** |
| **nivel alto sostenido > T** | cámara pegada | `$ALARM` `CAM_PEGADA` a los **20 min** | 🟢 |

### Orden de ejecución — y las dos primeras fases NO tocan el ciclo

| | qué | depende de | estado |
|---|---|---|---|
| **1** | instalar · activar · IP · **microSD y grabación** | 🔴 configurar la grabación (`A-0`) — **las tarjetas ya están decididas** | pendiente de campo |
| **2** | **mirar la casilla `Trigger Alarm Output`** | 10 min con la cámara delante. **Decide todo lo demás** | 🟢 **paso 39 de la guía de banco** (`2d17678`) |
| **3** | medir el **`Delay`** y el tiempo de respuesta | de ahí se **deriva** `SILENCIO_MS > Delay + rearme` (`A-7`) | 🟢 **paso 40 de la guía** (`2d17678`) |
| **4** | firmware fase 1: **el vigilante, los eventos Y SU PANTALLA** | **cero efecto vial** | 🟢 **CONSTRUIDA** (`4b90f98`) · ⚠️ **la pantalla del ESTADO falta**: `A-13` |
| **5** | firmware fase 2: **el veto de la pluma** | **sólo si la fase 4 da números que lo justifiquen**, y con la derogación de SFTY-28 escrita (`A-1.bis`) | ⛔ bloqueada |

### Lo que el equipo tiene que PUBLICAR

| campo | valores | estado |
|---|---|---|
| **`PLUMA:`** | `ARRIBA` · `ABAJO` | 🟢 **publicado en las dos puntas y dibujado en la app** (N-153, `79ef5a6`). Con `D-13` habrá ratos de **luz roja y pluma arriba**, que hoy un operario lee como avería |
| **`CAM:`** | `OK` · `CIEGA` · `PEGADA` · `?` | 🔴 **no cabe: `A-13`.** `camara_estado()` está escrita y sin llamador. Sin él, «no llega bit» y «no hay nadie» son indistinguibles |

Y en la app: el **estado de cada cámara** y el **contador de la fase 1** («el veto habría actuado N
veces») — este último **ya llega**, dentro del `$EVENT` con `VETO_HABRIA_ACTUADO_N:`.

> **Por qué esto no se había hecho:** se esperaba que las cámaras alimentaran el Modo
> Inteligente, y ese modo no se usa. **No es que se olvidara dibujarlo: es que no había dato
> que dibujar.** Con D-13 lo hay, y por primera vez llega a los modos que se operan.
>
> ⚠️ **Y `A-12` es la otra mitad de esa frase:** el Modo Inteligente **es el único modo que usa las
> cámaras**, y su límite de 15 s lleva ahí desde antes de comprarlas.

### Lo que NO se hace, y no es negociable

- **Nada que AUTORICE por ausencia** — acortar el despeje, dar verde antes. Una cámara
  desconectada lee «no hay nadie».
- **Ningún tope que fuerce la bajada** de la pluma: devolvería el peligro que el veto evita.
  Tope → **alarma**, nunca acción.
- **El ciclo del semáforo no se toca.** Las cámaras no dan ni quitan verde.

---

<a id="d-14--la-entrada-de-alarma--la-vía-que-no-depende-de-la-casilla-bloqueada"></a>

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
Quick Start Guide for cable connection**»* (PDF p.56 / impresa 44).

✅ **CORREGIDO el 05/09: la Quick Start Guide SÍ está en disco** —`04_Manuales/...UD40284B...
Quick_Start_Guide_20241115.pdf`, 40 páginas—. Lo que pasa es que **no tiene capa de texto**:
cero caracteres extraíbles en las 40, así que **cualquier búsqueda da cero POR EL FORMATO**
(§4 aplicada a un fichero, igual que el `.kicad_pcb`). **Al renderizarla como imagen y
mirarla, la página 8 dice literal:**

> *«`1A` and `1B`, `2A` and `2B`, `3A` and `3B` are **three pairs of alarm outputs**»*
> *«`IN1` and `GND1`, `IN2` and `GND2` are **two pairs of alarm inputs**»*

🟢 **Así que `1A`+`1B` = UN contacto ya es LECTURA, no deducción.** Nuestra cámara tiene
`1 input, 1 output`, luego le corresponden `1A`+`1B` (salida) e `IN1`+`GND1` (entrada). Y su
propio aviso: *«The interface varies with the models»*.

**Lo que la Quick Start Guide NO trae es diagrama de cable ni régimen eléctrico** — es un
folleto genérico de 8 páginas de montaje y 32 de textos legales. La delegación del manual
**acaba en un callejón sin salida**.

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
