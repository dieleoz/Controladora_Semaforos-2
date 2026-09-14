# SPEC 4 — La app y el Bluetooth: la unica forma de operar el equipo

**Para:** el tecnico que va al poste, el funcional y el auditor. **Escrita el 12/09/2026.**

> 🔴 **`D-16`: SIN TELEFONO NO HAY FORMA DE OPERAR EL EQUIPO.** No hay pantalla (`D-17.bis`), no hay
> pulsadores (`D-2`: `BOTON3`/`BOTON4` son camaras) y no hay mando (`D-1`). Ni ambar, ni volver a
> automatico, ni parar el cruce. **Es una propiedad DECLARADA del sistema, no una averia** — y es lo
> que obliga a todo lo de abajo: **cada orden tiene que contestar lo que de verdad paso**, porque no
> queda ninguna otra superficie donde el operario pueda comprobarlo.

**Que manda sobre este fichero:** `DECISIONES.md` en lo decidido y `05_Funcional/17_...` en el cobre.
**Que NO entra aqui:** el ciclo y sus tiempos (SPEC 1), la radio entre postes (SPEC 2), la hora y su
siembra (SPEC 3), el cobre y los conectores (SPEC 5).

**Convencion de este fichero:** se cita el **simbolo**, nunca el numero de linea (`CLAUDE.md` §7.3), y
**no se copia ninguna cifra que este fichero no pueda recalcular** (§14): donde hace falta un numero se
nombra al pack que lo rehace en cada corrida.

---

## 1. El transporte: telefono -> ESP32 -> STM32

Son **tres tramos** y **el del medio no es un cable tonto**.

| tramo | como | quien lo define |
|---|---|---|
| telefono <-> ESP32 | **Bluetooth SPP**, sin cifrar. El rotulo que ve Android lo sirve `transporte_abrir()` | `transporte_app.cpp` |
| ESP32 <-> STM32 | **UART por `J17`**, `ENLACE_BAUDIO` / `ENLACE_FORMATO`, `ENLACE_PIN_TX`/`RX` | `contrato.h` |
| formato de trama | `$TIPO,CLAVE:valor,...` cerrado con `*XX\r\n`, `XX` = XOR-8 del payload **saltando el `$`** | `enviarTramaConCrc()` / `calcularChecksum()` en los dos `bluetooth.cpp` |

**El rotulo SPP se APRENDE.** La serie sale del silicio del STM32 y el ESP32 no puede saberla al arrancar:
`transporte_aprenderRotulo()` la toma del primer `$STATUS` que retransmite y la guarda, y el rotulo bueno
—`ROTULO_PREFIJO` + serie + `M` o `E`— aparece **en la arrancada siguiente**. Hasta entonces se ve
`ROTULO_PROVISIONAL` **a proposito**: un rotulo inventado es peor que uno que admite que no sabe. **No se
re-rotula en caliente**: cambiar el nombre SPP tira la sesion del operario.

**El puente NO filtra la telemetria de subida.** `desdeElEquipo()` valida FORMATO —`trama_valida()`— y
retransmite **toda trama bien formada, la conozca o no**. La lista `PREFIJOS_STM32` existe solo para los
contadores de diagnostico. Un filtro por prefijo se comeria el `$EVENT`, que es la bitacora entera.

**La UNICA trama que el puente modifica** es el sello de hora de `sellarHoraSiFaltaba()`, y va **despues**
de validar el checksum y **recalculandolo**. Su alcance es de SPEC 3.

**Lo que el puente se queda y NO cruza** (`despachador_esParaElPuente()`): `CMD:LEER_RTC`, cualquier
linea que contenga `SET_RTC:`, y cualquier linea que contenga `HORA_ESP32`. Las tres las contesta el
propio puente marcado `NODE:PUENTE`. **O cruza o la atiende el puente, nunca las dos** (`D-15`).

**Los dos limites de linea, y los dos avisan:**

- **Bajada (app -> equipo):** `TRAMA_MAX_UTIL`. Lo que se pase se descarta y **se dice** con
  `$ERR,NODE:PUENTE,CMD:DESCONOCIDO,DESC:LINEA_DEMASIADO_LARGA`. El STM32 truncaria en silencio y
  compararia la linea recortada con `strcmp` como si estuviera entera: por eso el tope se mide en el
  puente.
- **Subida (equipo -> app):** `BUF_ENTRADA_STM32`; lo que desborda se cuenta en
  `puente_descartadasPorLargo()` y no sube.

**El latido.** El puente emite `LATIDO_LINEA` cada `LATIDO_MS` hacia el STM32 para que sus contadores de silencio
de `J17` cuenten **muertes del puente** y no silencios del puerto. **Los dos despachadores tienen una rama
reservada que sale sin actuar y SIN CONTESTAR**, la primera de todas: sin ella el latido caeria en el
`$ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO` del final y la app pintaria un aviso rojo cada dos segundos acusando al
operario de una clave que nadie tecleo. **Y sin telefono conectado no se escribe:** `transporte_escribir()` mira
`hasClient()` y devuelve `0`, porque contar como entregada una trama que `BluetoothSerial` tira dejaria mintiendo
al unico contador que se puede mirar desde fuera.

---

## 2. Quien atiende cada orden — las dos puntas NO aceptan lo mismo

La app enruta por `NODE:` del `$STATUS`, no por lo que el operario suponga que tiene delante. Las listas
son `SOLO_MAESTRO` y `SOLO_ESCLAVO` en `app.js`, y `app_08_enrutado_por_punta` recalcula el reparto
leyendo **los dos despachadores** y prohibe que una lista reclame una orden que atienden las dos.

| solo el MAESTRO | solo el ESCLAVO | el PUENTE de cada poste |
|---|---|---|
| `SET_MODO:` AUTO · MANUAL · AMBAR · MENU · ALCANCE · INTELIGENTE · DEGRADADO · `MANUAL:CAMBIAR_TURNO` · `SET_TIEMPOS` · `TEST_LEDS` · `DEMANDA` · `REINICIAR_RELOJ` · `FORZAR_ROJO` | `AMBAR_EMERGENCIA` · `CANCELAR_AMBAR` · `SOLICITAR_PASO` · `SET_MODO:DEGRADADO` | `LEER_RTC` · `SET_RTC` |

⚠️ **`SET_MODO:DEGRADADO` la atienden las DOS y no hacen lo mismo:** en el Maestro es el modo del cruce;
en el Esclavo es la llave que `D-18` abrio para esa punta (cierra `A-11`).

⚠️ **`FORZAR_ROJO` no significa lo mismo en las dos puntas, y por eso el Esclavo ya no lo acepta:** alli
prometia rojo y hacia ambar con la talanquera arriba. Lo rechaza **nombrando el literal nuevo**
(`AMBAR_EMERGENCIA`), no en silencio y no como alias.

---

## 3. Las ordenes, y QUE CONTESTA el equipo a cada una

> 🔴 **La regla que gobierna esta tabla (`CLAUDE.md` §2): un `$ACK` que no depende de lo que la llamada
> devolvio es una mentira con formato de exito.** El molde bien hecho es `SET_TIEMPOS`: **pregunta DENTRO
> del `if`** y tiene **un `$ERR` por cada motivo de rechazo**.

### 3.1 Maestro (poste 1) — `Maestro/src/bluetooth.cpp`, `procesarComando()`

| orden | PIN | que contesta, y **de que depende** |
|---|---|---|
| `SET_MODO:AUTO` | si | `RESULT:OK` **incondicional**. La rama llama ella misma a `coordinador_iniciarModo()` —el todo-rojo de entrada—, asi que no depende del flanco de `main.cpp`; las dos llamadas son `void` y no hay rechazo posible |
| `SET_MODO:MANUAL` | si | `RESULT:OK` incondicional, por lo mismo: la rama llama a `coordinador_forzarRojoTotal()` (`N-147`: se entra por el todo-rojo que no programa nada) |
| `SET_MODO:AMBAR` | si | **dos respuestas**, y dependen de `modoActual_get()` leido ANTES de escribirlo: `RESULT:REARMADO` si ya estaba en `MODO_AMBAR` —la rama re-arma con `modo_ambar_setup()`— y `RESULT:OK` si no. `N-146`: sin esto el equipo contestaba OK seis veces sin encender nada |
| `SET_MODO:MENU` | **no** | **tres**: en `MODO_DEGRADADO` depende del bool de `modo_degradado_pedirSalida()` — `RESULT:SALIENDO_TODO_ROJO` si arranco la salida, `$ERR ... YA_VUELVE_AL_MENU` si ya estaba en marcha. **En ninguno de los dos se contesta OK**: el menu tarda el todo-rojo entero. Fuera del Degradado, `RESULT:OK` |
| `SET_MODO:ALCANCE` | **no** | `$ERR ... EN_MARCHA_PARE_EL_MODO` si `modoActual_get()` es `MODO_DEGRADADO`; si no, `RESULT:OK` **incondicional** — ver §7 |
| `SET_MODO:INTELIGENTE` | si | igual que la anterior, y con la misma reserva de §7 |
| `SET_MODO:DEGRADADO` | si | depende del `MotivoDegradado` que devuelve `modo_degradado_evaluarEntrada()`: `$ERR ... DESC:<motivoL1> <motivoL2>` con **los mismos dos textos** que ensenaba el gabinete, o `RESULT:OK` solo si `MDG_OK` |
| `FORZAR_ROJO` | **no** (y con PIN tambien) | `RESULT:OK` incondicional. `coordinador_forzarRojoTotal()` es `void` y no tiene guarda: no hay nada que mirar |
| `MANUAL:CAMBIAR_TURNO` | si | **tres, y el ORDEN importa** (`N-151`): `$ERR ... MODO_SIN_CICLO_SALGA_PRIMERO` si `modoMueveElCoordinador()` es falso; si no, `RESULT:OK` cuando `pedirCambioVerificado()` devuelve true, y `$ERR ... EN_TRANSICION_REINTENTE` cuando no |
| `TEST_LEDS` | si | `RESULT:STARTING_6S` incondicional, **y es correcto**: `semaforo_iniciarTestLeds()` no lleva guarda A PROPOSITO y lo deja escrito — un rechazo mudo dentro de una funcion `void` dejaria este acuse mintiendo. La espera se resuelve en `semaforo_actualizar()` |
| `SET_TIEMPOS:v,r,d` | si | **EL MOLDE. Cuatro respuestas, una por motivo:** `$ERR ... FORMATO_INVALIDO` si el `sscanf` con el `%c` centinela no convierte 3 exactos —dos ordenes pegadas convierten 4—; `$ERR ... EN_MARCHA_PARE_EL_MODO` si `modoAutomatico_enMarcha()`; `$ERR ... RANGO` si `modoAutomatico_fijarTiempos()` devuelve false; `RESULT:OK` **solo** si devolvio true |
| `REINICIAR_RELOJ` | si | depende del bool de `reloj_reiniciarDominioRespaldo()`: `RESULT:CRISTAL_OK_PONGA_LA_HORA` o `$ERR ... SIGUE_PARADO_VEA_CONSULTA_RELOJ`, **y en el fallo emite ademas** el `$EVENT` con `ORIGEN:RELOJ` de `reportarBitsDelReloj()` |
| `DEMANDA` | si | **tres**: `$ERR ... SOLO_EN_MODO_INTELIGENTE` fuera del modo —registrar una peticion que ningun ciclo va a mirar es fingirla—; `RESULT:REGISTRADA` si `demanda_solicitar()` devuelve true; `$ERR ... REPITA_EN_UNOS_SEGUNDOS` si devuelve false |

### 3.2 Esclavo (poste 2) — `Esclavo/src/bluetooth.cpp`, `procesarComando()`

**`AMBAR_EMERGENCIA` tiene DOS puertas —sin PIN y con PIN— con el MISMO bloque letra por letra.** No se
factoriza a proposito: los packs (`esclavo_07`, `esclavo_08`) leen el bloque de CADA rama, y una respuesta
que viviera en una funcion comun los dejaria midiendo un bloque vacio (`N-89`).

| orden | PIN | que contesta, y **de que depende** |
|---|---|---|
| `AMBAR_EMERGENCIA` | **no** | **seis literales, y ninguno promete de mas.** Con la luz libre (`!degradado_gobiernaLuz()`) el ambar se enciende ya; lo que el `RESULT` anade es **si el aviso al Poste 1 puede haber sido oido**, y eso NO sale de `protocolo_enviarPaquete()` —que es `void`— sino de dos banderas: `enlaceCaidoAnunciado` (esta punta ya declaro que se quedo sin radio) y `avisoAmbarConfirmado` (`D-31`: el Poste 1 acuso un aviso anterior de ESTE ambar). De ahi `OK`, `OK_SIN_RADIO`, `OK_POSTE1_AVISADO` y sus tres gemelos `YA_EN_AMBAR_LATCH_PUESTO...` cuando la luz ya estaba en ambar y lo unico nuevo es el latch. **`SIN_RADIO` gana a `POSTE1_AVISADO`**: el acuse es de hace un rato, la caida es de ahora |
| `AMBAR_EMERGENCIA` (con el Degradado gobernando) | — | `RESULT:SALIENDO_TODO_ROJO` si hay salida iniciada y `RESULT:SALIDA_YA_EN_CURSO` si es una rendicion (`degradado_rendicionEnCurso()`) — **no `OK`**, porque el ambar tarda el todo-rojo entero. Y si la salida en curso termina en ROJO porque la pidio otro: `$ERR ... SALIDA_A_ROJO_EN_CURSO_REPITA`, **sin armar el latch** |
| `CANCELAR_AMBAR` | si | **PIDE PIN al reves que armar**, y es deliberado: quitar el ambar devuelve el cruce a dar verdes, o sea **abre paso**. Depende de `ambarEmergencia`: con latch puesto lo retira y contesta `RESULT:RETIRADO`, o `RESULT:RETIRADO_QUEDA_MANDO` si `mando_ambarLocal()` sigue vetando —contestar OK a secas mandaria al tecnico a esperar un cambio que no va a llegar—. Sin latch, `RESULT:REENVIADO_AL_MAESTRO` si esta punta **sigue** en `S_FALLO` (la red de la trama perdida, `N-152`), y `$ERR ... NO_HAY_AMBAR_VIGENTE` si no |
| `SOLICITAR_PASO` | si | depende del bool de `demanda_solicitar()`: `RESULT:PEDIDO_AL_MAESTRO` o `$ERR ... REPITA_EN_UNOS_SEGUNDOS`. **El Esclavo PIDE; no ordena** (SFTY-27) |
| `SET_MODO:DEGRADADO` | si | depende del `RechazoDegradado` que devuelve `degradado_entrar()` —**no de un bool**—: `$ERR ... DESC:<degradado_textoRechazo(r)>`, un motivo por rama, con la MISMA tabla que ensenaba el gabinete. Si acepto, **`RESULT:YA_ACTIVO`** cuando el modo ya gobernaba (`antesDeg` leido antes de la llamada) y `RESULT:OK` cuando esta pulsacion lo encendio |
| `FORZAR_ROJO` | las dos formas | `$ERR ... RENOMBRADO_USE_AMBAR_EMERGENCIA`. **Se rechaza ensenando el nombre bueno**, no en silencio: quien lo manda tiene una app o un manual anteriores al cambio |
| `TEST_LEDS` | si | `$ERR ... NO_EN_SERVICIO_USE_EL_MAESTRO`, **rechazado a proposito**: la secuencia enciende VERDE sin mirar nada, y ese verde saldria mientras el Maestro da paso al otro sentido |

### 3.3 El puente ESP32 de cada poste — `ESP32_Expansion/src/despachador.cpp`

| orden | PIN | que contesta, y **de que depende** |
|---|---|---|
| `CMD:LEER_RTC` | no (es una consulta: no abre, no para, no cambia) | `$ACK,NODE:PUENTE,...,RESULT:OK` con `FECHA:` y `HORA:` **releidas del chip ahora mismo**, o **siete `$ERR` distintos**, uno por cada valor de `MotivoSinHora` —incluido `BARRERA_INCOHERENTE`, que existe para que un defecto del firmware no se disfrace de averia del modulo, y `MOTIVO_NO_CONTEMPLADO`, para que nadie anada un valor al enum y deje la consulta muda—. Cada literal vive **dentro** de su rama |
| `SET_RTC:<fecha>,<hora>` | **el PIN viaja y no lo comprueba nadie** — ver §7 | siete motivos, uno por valor de `ResultadoReloj`, y el `$ACK` final **devuelve la hora RELEIDA**, no la que se mando. Hay un octavo caso que es `$ACK` y no `$ERR`: `RESULT:HORA_PUESTA_SIN_PROPAGAR` cuando el DS3231 quedo puesto pero `siembra_ahora()` no pudo poner la linea entera en el cable. El detalle es de SPEC 3 |
| cualquier linea con `HORA_ESP32` dentro | — | `$ERR,NODE:PUENTE,CMD:HORA_ESP32,DESC:LINEA_RESERVADA_AL_PUENTE`, **y va la primera de todas**: esa orden solo la origina el modulo, y el STM32 la acepta sin PIN porque el puente no lo conoce |

---

## 3.bis 🟢 LAS BOTONERAS A/B/C/D ESTAN ELIMINADAS — el equipo se opera SOLO por la app

**El hardware ya no existe: se retiro del equipo el 05/09** —*«se opera solo por app»*— y el
responsable lo reafirma el 14/09: *«eliminamos las botoneras A, B, C y D»*. **No hay superficie
fisica de operacion**: ni botones, ni receptor que comprar, ni pantalla —esa salio del firmware el
13/09—. La guia de campo ya retiro su paso de prueba del mando, y **`J16` p5 y p8 se quedan vacios:
no hay nada que cablear ahi**.

✅ **Y EL PROGRAMA YA NO LOS LEE: el codigo salio del firmware el 14/09.** `mando.cpp` y `mando.h` de
las dos puntas estan retirados —681 lineas— y con ellos el reconocedor de secuencias, de modo que
**un puente en `J16` p5 o p8 ya no compone nada**: era el unico riesgo que quedaba vivo, porque
puentear una bornera para probarla es lo primero que hace un instalador y entonces movia el cruce de
verdad. Los bornes siguen vacios y pelados, asi que **la instruccion de no cablearlos sigue en pie**;
lo que cambia es que ya no es lo unico que lo impide.

✅ **Y lo que cuesta sacarlo esta MEDIDO hoy, porque el repositorio lo daba por mas caro de lo que
es** — dos frases que lo frenaban y son falsas:

| lo que se decia | lo medido el 14/09 |
|---|---|
| *«retirar el mando se lleva por delante parte de la barrera de salidas»* | **No.** La bandera que intercepta las luces la arman **dos** funciones y sus **unicos** llamadores son el mando. Sin ellos la bandera se queda en falso para siempre y la guarda que cuelga de ella **no dispara**: el equipo escribe las luces por el camino normal. **No deja ningun veto abierto — deja codigo muerto** |
| *«borrar el menu borra el todo-rojo de las dos puntas»* | **Tampoco.** Ese todo-rojo tiene **tres** llamadores: el menu, el Modo Alcance y el Modo Hora, **y los dos ultimos se alcanzan desde la app** |

✅ **Y la condicion que puso el responsable se cumplio: el camino de interceptar las luces SALIO
ENTERO con el mando**, en vez de quedarse dentro sin nadie que lo ejerza. Con el fuera, `semaforo.cpp`
escribe los pines por un solo sitio —`aplicarSalidas()`, donde vive el enclavamiento `SFTY-2`— y la
lista de funciones autorizadas a saltarselo **baja de cinco a una**: cualquier camino nuevo a una
lampara es ahora un rojo del banco en vez de una excepcion ya aprobada por su nombre.

## 4. El PIN

- La forma del cable es `CMD:PIN:<pin>:<accion>` y la comparacion es **literal en los dos despachadores**.
  La app lo antepone a todo lo que no este en su lista `SIN_PIN`.
- **El criterio, escrito en el firmware:** *el PIN guarda lo que ABRE paso o mueve luces; no lo que las
  para.* Por eso el rojo de emergencia entra sin clave: quien esta viendo el incidente tiene que poder
  pedirlo aunque no se sepa la clave.
- **La segunda puerta de la app no es una clave: es el VALE DE VIA DESPEJADA.** Para las ordenes que abren paso
  desde la botonera de campo, `enviarComandoFirmware()` exige `viaConfirmadaVigente()` en vez del PIN, y el motivo
  no es comodidad: **un PIN demuestra QUIEN eres y no demuestra que hayas MIRADO.** El firmware no cambia —sigue
  exigiendo el PIN y la app se lo sigue poniendo—; cambia a quien se lo pide la app.
- **La app devuelve si la orden llego a salir, y el que llama tiene que mirarlo.** `enviarComandoFirmware()`
  devuelve `bool`: pulsar un boton no es saber que el equipo obedecio, y **ni siquiera es saber que la orden
  salio**. Una orden que no salio se anota en el Diario **y no en la cinta** —por el cable no paso un byte—.

🔴 **Lo que el PIN NO cubre hoy esta medido y esta en §7.**

---

## 5. Las cinco tramas que la app lee

`TIPOS_QUE_LA_APP_LEE` en `app.js` es la lista, y `juzgarTrama()` la unica puerta: valida el XOR-8 y
**devuelve POR QUE** cuando no entra. Una cabecera fuera de la lista **no se pinta y se cuenta nombrandola**.

| trama | campos, en orden | quien la emite |
|---|---|---|
| `$STATUS` | `NODE` `SERIE` `MODO` `ESTADO` `T` `RF` `RTT` `BAT` `HORA` **`ESC`** `PLUMA` `CAM` | Maestro, cada `tUltimaTelemetria` |
| `$STATUS` | `NODE` `SERIE` `MODO` `ESTADO` `T` `RF` `RTT` `BAT` `HORA` `PLUMA` `CAM` | Esclavo — **sin `ESC`**, y la asimetria es deliberada (`N-149`): esta punta no tiene a quien preguntarle por el otro poste |
| `$ACK` | `CMD` `RESULT` (+ `FECHA` `HORA` en los del puente, que ademas llevan `NODE:PUENTE`) | la punta que atendio la orden |
| `$ERR` | `CMD` `DESC` (+ `NODE:PUENTE` si es del ESP32) | idem |
| `$EVENT` | `NODE` `ORIGEN` `DETALLE` `HORA` | las dos puntas. Es el **Diario de Ordenes**: lo que sobrevive al viaje del tecnico |
| `$ALARM` | `NODE` `EVENTO` `CAUSA` + el tramo del enlace + `ACCION` `HORA` | las dos puntas. El tramo es `RF`/`RTT`/`SINRESP` en el Maestro y `RX`/`OK`/`RUIDO` en el Esclavo — **no son los mismos tres numeros**, porque no miden lo mismo |

**Las marcas de ausencia, y significan cosas distintas:** `--` es *«todavia no lo se»* o *«en esta fase no
hay dato»*; `!` es *«llego un valor imposible»*. **Un `0` de relleno esta prohibido**: `T:0` es el ultimo
segundo de la fase y significa lo contrario que `T:--`.

**El `DETALLE` de un `$EVENT` no lleva comas**, y no es estilo: el parser de la app parte por `,` y cada
trozo por su **primer** `:`, asi que una coma dentro convertiria el resto en campos sueltos que el pintor
de `$EVENT` no mira. Los buffers de todas estas tramas los recalcula `esp32_07_presupuesto_bytes` en cada
corrida, **y la cuenta no se copia aqui** (§14).

---

## 6. Que ve el tecnico al conectarse a CADA poste

| | **Poste 1 (Maestro)** | **Poste 2 (Esclavo)** |
|---|---|---|
| rotulo en Android | `<prefijo><serie>-M` | `<prefijo><serie>-E` (o el provisional en la primera arrancada del modulo) |
| cabecera | `MAESTRO (POSTE 1)` | `ESCLAVO (POSTE 2)` — lo decide `NODE:`, no lo que el operario suponga |
| `MODO:` | `MENU` `MANUAL` `AUTO` `INTELIGENTE` `ALCANCE` `HORA` `DEGRADADO` `AMBAR` | `SUBORDINADO` `DEGRADADO` `RENDIDO` — el campo dice si esta punta esta obedeciendo o gobernando |
| `ESTADO:` | `ROJO` `VERDE` `AMARILLO` `FALLO COM` (con espacio) | los mismos cuatro |
| cuenta atras `T:` | numero o `--` | **`--` FIJO** |
| enlace `RF:` `RTT:` | medidos | **`--` FIJOS** |
| bateria `BAT:` | **`--` FIJO** en las dos puntas: el equipo declara que **no la mide** —falta el divisor y la entrada analogica—, y la app lo dice con esas palabras en vez de pintar un numero |
| boton de emergencia | `FORZAR_ROJO` | `AMBAR_EMERGENCIA` + `CANCELAR_AMBAR` |

⚠️ **Los cuatro campos fijos del Esclavo —`T`, `RF`, `RTT`, `BAT`— NO SE RETIRAN, y el motivo es de la
app, no del firmware.** La app escribe `state.countdown` dentro de `if (data.T !== undefined)` y
`state.battery` dentro de `if (data.BAT !== undefined)`, y **nadie los limpia al cambiar de poste**:
medido, los unicos sitios que los escriben son esas dos ramas. Quitar los campos del `$STATUS` del
Esclavo dejaria la pantalla del poste 2 **pintando la cuenta atras y la bateria del poste 1**, que es peor
que un `--`. El campo que llega marcado es un dato; el campo que no llega deja la pantalla mintiendo.

🟢 **EL POSTE 2 DICE COMO VE EL SU ENLACE, y hasta el 12/09 no lo decia:** los tres contadores
—`protocolo_bytesRecibidos()`, `protocolo_tramasValidas()`, `protocolo_tramasDescartadas()`— **solo salian dentro
del `$ALARM`, o sea cuando el enlace YA se habia caido**. `D-32` (2) eligio la via de `D-23` —**`$EVENT`
periodico**, no un aviso ESP32 -> STM32—: `Esclavo/src/bluetooth.cpp` publica `$EVENT ... ORIGEN:ENLACE_RF,
DETALLE:RX:<n> OK:<n> RUIDO:<n>` cada `DIAG_ENLACE_MS`, con su propio reloj y **fuera del `if` del `$STATUS`**.
Los cuatro campos fijos siguen fijos: **el dato nuevo no va en el `$STATUS`, va en el Diario.** La PANTALLA sigue
abierta: §7.3.

---

## 7. HUECOS MEDIDOS

> Lo que sigue **no cumple una decision vigente, o no lo cubre nadie**. Cada linea dice como se midio.

🔴 **1 · El PIN `1234` viaja en claro y cuatro ordenes no lo piden** (`roadmap.md` 1.20). `correctPin`
es un literal en `app.js` y `CMD:PIN:1234:` es un literal en **los dos** `bluetooth.cpp`; el transporte es
SPP **sin cifrar**. Recontadas hoy sobre el fuente, las ordenes que **cambian algo** y entran sin PIN son
**cuatro**: `FORZAR_ROJO` y `SET_MODO:MENU` y `SET_MODO:ALCANCE` (Maestro) y `AMBAR_EMERGENCIA`
(Esclavo). Las otras dos ramas que preceden a la guarda de PIN **no cuentan**: la del latido no contesta
ni actua, y la de `HORA_ESP32` el puente la tira si viene del telefono. **`SET_MODO:MENU` no es inocuo**:
su `menu_setup()` llama al todo-rojo de las dos puntas. La exencion de `FORZAR_ROJO` y `AMBAR_EMERGENCIA`
esta razonada en el fuente y es defendible; **lo que no esta escrito en ningun sitio vivo es que las
cuatro juntas son la superficie sin clave de un equipo cuyo unico mando es el telefono.**

🔴 **2 · `SET_RTC` con un PIN falso pone la hora** (`roadmap.md` 2.11). El puente no conoce el PIN y el
STM32 ya no ve la linea: **no hay quien lo rechace**. Esta **abierto POR DECISION** —`D-26` (1), riesgo
aceptado por el responsable— y por eso no es un defecto; se escribe aqui porque **una excepcion que nadie
cuenta es indistinguible de un olvido** (`CLAUDE.md` §6).

🔴 **3 · `D-23`: EL FIRMWARE YA ESTA; LA PANTALLA DE LA APP NO — y la app es de quien era la decision.**
`D-32` (2) resolvio el mecanismo: como el STM32 **no puede saber** que un telefono se conecto —el unico que lo ve
es el ESP32— y traer ese aviso seria **la tercera orden que el accesorio origina hacia el micro**
(`esp32_05_no_origina` la condiciona **por escrito** a una fila de `DECISIONES.md`), el responsable eligio
**`$EVENT` periodico a cadencia baja**, que no toca ninguna barrera, y esta construido. **LO QUE SIGUE ABIERTO,
medido sobre el arbol:** `D-23` pedia *«una pantalla para Esclavo, diferente de lo que hoy hace la app»* y **la app
no tiene ni una linea de ella** — `grep -n "ENLACE_RF" app.js` da **cero en las cuatro copias**. El dato **si llega
al tecnico**, pero como una linea mas del REGISTRO DE EVENTOS: `$EVENT` lo pinta el camino generico de `app.js`,
que no distingue este `ORIGEN` de ningun otro. **Es media decision construida, y la mitad que falta es la que la
fila nombra.**

🔴 **4 · Dos `$ACK` que no dependen de lo que la orden hizo: `SET_MODO:ALCANCE` y `SET_MODO:INTELIGENTE`.**
Las dos ramas contestan `RESULT:OK` despues de `modoActual_set()` **y nada mas**. El trabajo de entrar en
el modo vive en `modoAlcance_setup()` y `modoInteligente_setup()`, y `Maestro/src/main.cpp` los llama
**solo bajo `if (modo != modoAnterior)`**: repetida la orden con el modo ya puesto, el equipo contesta OK
y **no corre nada**. Es la **misma forma** que `N-146` reparo en `SET_MODO:AMBAR` —capturando `yaEnModo`
y contestando `REARMADO`— y que el Esclavo reparo en su `SET_MODO:DEGRADADO` —capturando `antesDeg` y contestando
`YA_ACTIVO`—; las dos unicas ramas de modo que **no** tienen la forma son `AUTO` y `MANUAL`, que son justo las que
llaman al coordinador **dentro** de la rama. **Lo que cada una se deja, medido:** en `ALCANCE`,
`modoAlcance_setup()` es el **unico llamador** de `protocolo_reiniciarContadores()` en el Maestro, o sea que **la
puesta a cero de SFTY-15 no ocurre en la segunda pulsacion**; en `INTELIGENTE`, no se re-ejecuta
`coordinador_iniciarModo()`. **El par peligroso es alcanzable por el mismo camino que `N-146`:**
`CMD:FORZAR_ROJO` entra **sin PIN desde cualquier modo**, mueve la luz y **no toca `modoActual`**. ⚠️ **Lo que NO
se afirma:** no esta medido en banco ni en tarjeta, es lectura del fuente, y **no deja el cruce trabado**.

⚠️ **5 · El Modo Alcance no tiene superficie.** Todo lo que ese modo produce sale por
`lcd_dibujarAlcance()`, y **el LCD se retiro** (`D-17.bis`). Desde el telefono, `MODO:ALCANCE` ensena el
mismo `RF:`/`RTT:` que cualquier otro modo —vienen de la ventana de latidos del coordinador, **no** de los
contadores que ese modo pone a cero—. Es un modo de medida sin nadie que lea la medida, y es `D-16` en su
forma mas simple.

⚠️ **6 · `menu.cpp` NO ES SOLO PANTALLA, y por eso `D-30` toca a esta spec** (`D-32` (1) la recorta: sale solo el
LCD — SPEC 1 §12.2). `menu_setup()` llama a `coordinador_forzarMenu()`, el todo-rojo de las dos puntas,
**alcanzable HOY desde la app** con `SET_MODO:MENU` (§3.1, y es una de las cuatro ordenes sin PIN del hueco 1).
**Si `menu.cpp` cae con el LCD, ese todo-rojo necesita otra puerta EN EL MISMO COMMIT.**

⚠️ **7 · Una ventana de ~1 s por conexion en la que una orden de Maestro puede salir contra un Esclavo.**
`SOLO_MAESTRO` pregunta `=== 'ESCLAVO'`, asi que con `state.node` en `null` —entre que el socket abre y
llega el primer `$STATUS`— la orden sale. El firmware la rechaza con `$ERR,CMD:DESCONOCIDO`, **asi que no
mueve una luz**; lo que se pierde es el aviso claro al operario. Esta **abierta a proposito**: cerrarla
rompe el arnes del puente, que pulsa antes del primer `$STATUS`.

⚠️ **8 · Desde el `$STATUS` la app no puede saber que reloj sello la `HORA:`.** Hoy siempre es el DS3231
del puente, porque el otro no existe, **pero la trama no lo dice**. Residual declarado en
`sellarHoraSiFaltaba()`; su cierre es de SPEC 3.

✅ **9 · La app destaca el aviso de barrera retenida, y ya no se puede perder con el scroll.** Un solo
`$EVENT` de `ORIGEN:CAMARA_PLUMA` con `DETALLE:VETO_SOSTENIDO_S:` abre un cartel propio
—`js/aviso_camara_pluma.js`, primer hijo de `.app-container`— que vive **fuera de la bitacora de 30
entradas y fuera de las cinco pestanas**, publica los segundos que el equipo mide y manda a **mirar
debajo del brazo antes de tocar la camara**; las repeticiones lo refrescan sin gastar una linea de
registro, y el `VETO_ACTUADO_N` del flanco se traduce como lo normal que es, sin cartel. **No afirma
«camara averiada»**: el equipo no ve imagen y no separa un vehiculo parado de un mal apunte (`D-12`),
asi que publica la medida y el juicio queda en quien esta delante del poste. Solo un `PLUMA:ABAJO` del
`$STATUS` le cambia el titulo —el cartel **no se retira solo**, porque lo que la dejo arriba sigue sin
revisarse— y el enlace caido lo borra, que es de un poste.

🟡 **10 · Los dos interruptores de administrador no existen, y su estado NO CABE en la trama de hoy.**
Decididos el 14/09 (SPEC 5 §4.1), y se llaman **los dos interruptores de administrador**: *sacar la
camara del veto* y *sacar la barrera de servicio*, uno por poste, sobreviviendo a un corte de luz. 🔴 **Y el obstaculo esta MEDIDO antes de construirlos, no
despues:** la trama de estado del poste 1 va llena — **le sobran 3 bytes de 155, y un campo nuevo no
cabe en 3 bytes**—, asi que **el estado de los dos interruptores no puede viajar ahi**. Hay una salida
ya medida que liberaria otros 3 recortando el campo de la hora, **y aun asi habria que comprobar que
alcanza**. Como se publican es una decision pendiente: un aviso propio, o ese recorte. **Mientras no se
decida, no se construyen**: un interruptor que el tecnico no puede ver encendido es peor que no
tenerlo, porque deja un poste degradado sin que nadie lo sepa.
---

## 8. QUIEN EJERCE CADA BARRERA DE ESTE DOCUMENTO

> **Una spec puede describir barreras que ningun compilador ejerce, con UNA condicion: que cada barrera lleve
> escrito QUIEN la ejerce.** El criterio es `CLAUDE.md` §6.3 — **¿algun arnes COMPILA ese `.cpp`?**; si solo lo
> lee por texto no ve un defecto del TIEMPO, y es *vigilada por texto*, no *ejecutada*. Filas = las de la
> compuerta; reparto de `.cpp` por arnes, `ARQUITECTURA.map` §4-5. **Medido sobre `ef3504c`.**

| barrera | quien la EJERCE hoy |
|---|---|
| §4 **el PIN del firmware** (`CMD:PIN:<pin>:<accion>`) | ✅ **fila 20**: `Simulaciones/puente_esp32/compilar.ps1` **enlaza el `bluetooth.cpp` REAL de las DOS puntas** y le teclea ordenes con el prefijo releido del fuente · **fila 18** ademas en el Esclavo (bloque H) |
| §3 **el `$ACK` depende de lo que la llamada devolvio** | ✅ **filas 20 y 18**, sobre esos mismos `bluetooth.cpp` reales: la 20 compara rama por rama lo que contesta el fuente |
| §4 **el PIN de la app** y §2 **el enrutado por punta** | ✅ **filas 9 a 12**, y la **12** corre `app.js` entero en jsdom inyectando `$STATUS` reales |
| §5 **`juzgarTrama()`, el XOR-8 y las cinco tramas** | ✅ **fila 12**, que inyecta tramas corruptas contra el `app.js` real |
| §4 **el VALE DE VIA (`viaConfirmadaVigente()`)** y el `bool` de `enviarComandoFirmware()` | ✅ **fila 12** |
| §3.3 **el despachador del ESP32** — `D-15`, `esParaElPuente()`, los siete `$ERR` de `LEER_RTC` | 🔴 **NADIE.** `ESP32_Expansion/src/despachador.cpp` **no lo compila ningun arnes**: solo lo cruza PlatformIO, y la fila 20 **lo modela en Python** (`ARQUITECTURA.map` §3.7, nota 3) |
| §1 **`transporte_escribir()` mira `hasClient()`** | 🔴 **NADIE.** `X:transporte_app.cpp`, igual que el anterior |
| §1 **`TRAMA_MAX_UTIL` y `BUF_ENTRADA_STM32`** | 🟡 **texto** (`esp32_06_no_parte_tramas`, `esp32_09_contrato_de_bytes`, `esp32_07_presupuesto_bytes`) |
| §1 **la rama del latido que sale sin contestar** | 🟡 **texto** (`esp32_08_silencio_no_es_orden`, y el censo de la fila 20, que la excluye por nombre) |
| §1 **`sellarHoraSiFaltaba()`**, la unica trama que el puente modifica | 🟡 **texto** (`esp32_13_siembra_de_hora`, `reloj_02_siembra_que_miente`); nadie compila `X:siembra.cpp` |

**Cuenta: 10 filas y 12 barreras — 7 ejecutadas, 2 sin nadie, 3 vigiladas solo por texto.** 🔴 **Los dos rojos son
el MISMO hecho: el ESP32 entero (9 `.cpp`) no se ejecuta en el PC en ningun sitio**, asi que todo lo que esta spec
dice del puente —`D-15`, los limites de linea, los siete motivos de `LEER_RTC`— descansa en packs que leen texto.
⚠️ **Y una refutacion que conviene dejar escrita: `Maestro/src/bluetooth.cpp` SI lo compila alguien** —la fila 20,
junto con el del Esclavo—; lo que no compila nadie es el despachador del **ESP32**, que es otro fichero.
