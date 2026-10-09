# SPEC 4 — La app y el Bluetooth: la unica forma de operar el equipo

**Para:** el tecnico que va al poste, el funcional y el auditor. **Escrita el 12/09/2026.**

> 🔴 **`D-16`: SIN TELEFONO NO HAY FORMA DE OPERAR EL EQUIPO.** No hay pantalla (`D-17.bis`), no hay
> pulsadores (`D-2`: `BOTON3`/`BOTON4` son camaras) y no hay mando (`D-1`). Ni ambar, ni volver a
> automatico, ni parar el cruce. **Es una propiedad DECLARADA del sistema, no una averia** — y es lo
> que obliga a todo lo de abajo: **cada orden tiene que contestar lo que de verdad paso**, porque no
> queda ninguna otra superficie donde el operario pueda comprobarlo.

**Que manda:** este fichero en su materia y `05_Funcional/17_...` en el cobre; `DECISIONES.md` es su indice.
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
| `SET_MODO:` AUTO · MANUAL · AMBAR · MENU · ALCANCE · INTELIGENTE · `MANUAL:CAMBIAR_TURNO` · `SET_TIEMPOS` · `TEST_LEDS` · `DEMANDA` · `REINICIAR_RELOJ` · `FORZAR_ROJO` | `AMBAR_EMERGENCIA` · `CANCELAR_AMBAR` | `LEER_RTC` · `SET_RTC` |

Las dos puntas atienden `SET_MODO:DEG_T`, `SET_DEG_AUTO` y `CONSULTA_DEG_AUTO` (§3.1, §3.2). ⬇️ ~~`SET_MODO:DEGRADADO`
la atienden las DOS~~ → **salio de las dos con `D-46` (`f9cad1f`, 02/10; sin banco)**, y `SOLICITAR_PASO` del
Esclavo con ella: las dos caen en el `else` final de `procesarComando()` y contestan
`$ERR,CMD:DESCONOCIDO,DESC:COMANDO_NO_SOPORTADO` (`..._EN_ESCLAVO` en el Poste 2).

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
| `SET_MODO:MENU` | **no** | **tres**: en `MODO_DEGRADADO` depende del bool de `modo_degradado_pedirSalida()` — `RESULT:SALIENDO_TODO_ROJO` si arranco la salida, `$ERR ... YA_VUELVE_AL_MENU` si ya estaba en marcha. Ninguno es OK: el menu tarda el todo-rojo. Fuera del Degradado, `RESULT:OK`. **Construido (`a4545bb`), sin banco:** sigue siendo la salida INMEDIATA del poste conectado, con el aviso de paleteros; con el Maestro en `MENU` esperando la siembra de `D-29`, revoca el permiso y contesta `RESULT:OK_REANUDACION_CANCELADA`. Hueco sin radio: SPEC 2 §7.quater |
| `SET_MODO:ALCANCE` | **no** | `$ERR ... EN_MARCHA_PARE_EL_MODO` si `modoActual_get()` es `MODO_DEGRADADO`; si no, `RESULT:OK` **incondicional** — ver §7 |
| `SET_MODO:INTELIGENTE` | si | igual que la anterior, y con la misma reserva de §7 |
| ~~`SET_MODO:DEGRADADO`~~ | — | **Salio con `D-46` (`f9cad1f`; sin banco):** `$ERR,CMD:DESCONOCIDO,DESC:COMANDO_NO_SOPORTADO`. Su puerta, `modo_degradado_evaluarEntrada()` con `DEG_RECHAZO`, queda en el fuente sin llamador que la alcance y se conserva |
| `SET_MODO:DEG_T:ahora,inicio,verde,despeje` | si | Construido (`990c278`; `D-35`, SPEC 2 §7.bis). **La unica orden de entrada al Degradado.** Puerta paralela a `modo_degradado_evaluarEntrada()`, sin sync de radio: `$ERR ... DESC:<texto>` (`modo_degradado_textoTestigo()`) por `MDT_FALTA_HORA`, `MDT_AHORA_DESFASADO` (`TOLERANCIA_TESTIGO_S`), `MDT_DESPEJE_RANGO`, `MDT_INICIO_VENCIDO`, `MDT_AMBAR_VIGENTE`, `MDT_EN_VERDE`, `MDT_NO_GUARDADO`; `RESULT:OK` / `RESULT:RENOVADO`. Con salida programada (`D-52`): `$ERR ... DESC:Cancele antes la salida programada` |
| `SET_DEG_AUTO:1` / `SET_DEG_AUTO:0` | si | Construido (`A-15`, SPEC 2 §7.ter (a)). **El `$ACK` sale DIFERIDO**, cuando el `ECO` del otro poste refleja el cambio: `RESULT:ON_EFECTIVO` (el otro tambien apto) / `RESULT:ON_FALTA_EL_OTRO_POSTE` / `RESULT:OFF`. Rechazos: `$ERR ... SIN_ENLACE_CON_EL_OTRO_POSTE` · `EN_DEGRADADO_SALGA_PRIMERO` · `FORMATO_INVALIDO`; sin `ECO` en `DEG_AUTO_ACUSE_MS`, `$ERR ... CAMBIADO_AQUI_SIN_ACUSE_DEL_OTRO_POSTE` (el cambio queda y se sigue publicando). Maestro en `MENU`: siempre ese `$ERR` (SPEC 2 §7.ter (a)). Misma orden y mismos literales en el Esclavo |
| `CONSULTA_DEG_AUTO` | no (es una consulta) | Construido. `RESULT:ESTE_<ON/OFF>_OTRO_<ON/OFF>_APTO_<SI/NO>`: la opcion propia, el ultimo `APTO` oido del otro y el `APTO` propio. Igual en el Esclavo |
| `FORZAR_ROJO` | **no** (y con PIN tambien) | Fuera de Degradado: `coordinador_forzarRojoTotal()`, `RESULT:OK`, 3 s de amarillo y rojo (`D-45`); sobre un rojo+amarillo, rojo directo (`D-53`, HUECO, SPEC 1 §3.3 (3)). En Degradado, **construido el 04/10 (`a4545bb`, `D-51`), sin banco:** rojo FIJO inmediato en ese poste (`DEG_ROJO_SIN_HORA`; tambien en la ventana de `D-29`), sin aviso de paleteros (decidido); respuestas en §3.ter.ter |
| `SET_MODO:DEG_FIN:ahora,salida` · `SET_MODO:DEG_FIN:CANCELAR` | si | Construido el 04/10 (`a4545bb`, `D-52`), sin banco; SPEC 2 §7.quater (c): `salida` = `HH:MM:SS` a la que esta punta sale por su reloj; aviso de paleteros; un `$ERR` por motivo y `$ACK` solo si quedo programada (listas en §3.ter.ter) |
| `CONSULTA_DEG_FIN` | no (es una consulta) | Construido el 04/10, sin banco. `RESULT:NINGUNA` o `RESULT:SALE_HHMMSS_FALTAN_<s>S_<RESPALDADA/SIN_RESPALDO>` (`_SIN_RESPALDO` tambien tras `FORZAR_ROJO` o hora perdida). Molde `CONSULTA_DEG_AUTO`; el contador no cabe en `$STATUS` (SPEC 2 §7.quater (c)). Igual en el Esclavo |
| `MANUAL:CAMBIAR_TURNO` | si | **tres, y el ORDEN importa** (`N-151`): `$ERR ... MODO_SIN_CICLO_SALGA_PRIMERO` si `modoMueveElCoordinador()` es falso; si no, `RESULT:OK` cuando `pedirCambioVerificado()` devuelve true, y `$ERR ... EN_TRANSICION_REINTENTE` cuando no |
| `TEST_LEDS` | si | Solo fuera de servicio (`N-82.bis`): `testLedsAdmitido()` exige modo `MENU` o `ALCANCE`, luz en `S_ROJO` y `ACK_RED` del Esclavo (`coordinador_rojoEsclavoConfirmado()`). `RESULT:STARTING_6S` si quedo armado; `$ERR` `SIN_ENLACE_AMBAR_NO_SE_PRUEBA` en `S_FALLO`, `ESPERANDO_ROJO_DEL_ESCLAVO` sin acuse, `EN_SERVICIO_PASE_A_MENU` en el resto |
| `SET_TIEMPOS:v,r,d` | si | **EL MOLDE. Cuatro respuestas, una por motivo:** `$ERR ... FORMATO_INVALIDO` si el `sscanf` con el `%c` centinela no convierte 3 exactos —dos ordenes pegadas convierten 4—; `$ERR ... EN_MARCHA_PARE_EL_MODO` si `modoAutomatico_enMarcha()`; `$ERR ... RANGO` si `modoAutomatico_fijarTiempos()` devuelve false; `RESULT:OK` **solo** si devolvio true; la app, en DEGRADADO, dice solo «guardados» y que sigue en Degradado (con rojo fijo no da verdes) |
| `REINICIAR_RELOJ` | si | **cuatro, y el `$ACK` NO sale en la rama** (1.49 (a), `622a20b`). ~~depende del bool de `reloj_reiniciarDominioRespaldo()`: `RESULT:CRISTAL_OK_PONGA_LA_HORA` o ...~~ → el bool solo decide si hay algo que verificar. `$ERR ... REPITA_EN_UNOS_SEGUNDOS` si ya hay una orden esperando veredicto —sin tocar nada: reiniciar otra vez abriria otra ventana y dejaria dos ordenes para un veredicto—; `$ERR ... SIGUE_PARADO_VEA_CONSULTA_RELOJ` si el oscilador no arranca, con el `$EVENT` `ORIGEN:RELOJ` de `reportarBitsDelReloj()`; y si arranca, **no contesta todavia**: publica `$EVENT ... ORIGEN:APP_BLUETOOTH, DETALLE:RELOJ_REINICIADO_VERIFICANDO` y deja la orden pendiente. **El veredicto sale despues, una sola vez**, desde `bluetooth_loop()` cuando `reloj_estadoCristal()` deja de decir VIGILANDO: `RESULT:CRISTAL_OK_PONGA_LA_HORA` **solo** con CUENTA, o `$ERR ... ARRANCA_Y_NO_CUENTA_VEA_CONSULTA_RELOJ` con CONGELADO, otra vez con los bits. **Llega unos segundos despues de pulsar**: el plazo es la ventana de vigilancia del cristal y no se copia aqui. La app traduce los tres `DESC` |
| `DEMANDA` | si | **tres**: `$ERR ... SOLO_EN_MODO_INTELIGENTE` fuera del modo —registrar una peticion que ningun ciclo va a mirar es fingirla—; `RESULT:REGISTRADA` si `demanda_solicitar()` devuelve true; `$ERR ... REPITA_EN_UNOS_SEGUNDOS` si devuelve false |

### 3.2 Esclavo (poste 2) — `Esclavo/src/bluetooth.cpp`, `procesarComando()`

**`AMBAR_EMERGENCIA` tiene DOS puertas —sin PIN y con PIN— con el MISMO bloque letra por letra.** No se
factoriza a proposito: los packs (`esclavo_07`, `esclavo_08`) leen el bloque de CADA rama, y una respuesta
que viviera en una funcion comun los dejaria midiendo un bloque vacio (`N-89`).

| orden | PIN | que contesta, y **de que depende** |
|---|---|---|
| `AMBAR_EMERGENCIA` | **no** | **seis literales, y ninguno promete de mas.** Con la luz libre (`!degradado_gobiernaLuz()`) el ambar se enciende ya; lo que el `RESULT` anade es **si el aviso al Poste 1 puede haber sido oido**, y eso NO sale de `protocolo_enviarPaquete()` —que es `void`— sino de dos banderas: `enlaceCaidoAnunciado` (esta punta ya declaro que se quedo sin radio) y `avisoAmbarConfirmado` (`D-31`: el Poste 1 acuso un aviso anterior de ESTE ambar). De ahi `OK`, `OK_SIN_RADIO`, `OK_POSTE1_AVISADO` y sus tres gemelos `YA_EN_AMBAR_LATCH_PUESTO...` cuando la luz ya estaba en ambar y lo unico nuevo es el latch. **`SIN_RADIO` gana a `POSTE1_AVISADO`**: el acuse es de hace un rato, la caida es de ahora |
| `AMBAR_EMERGENCIA` (con el Degradado gobernando) | — | `RESULT:SALIENDO_TODO_ROJO` si hay salida iniciada y `RESULT:SALIDA_YA_EN_CURSO` si es una rendicion (`degradado_rendicionEnCurso()`) — **no `OK`**, porque el ambar tarda el todo-rojo entero. Y si la salida en curso termina en ROJO porque la pidio otro: `$ERR ... SALIDA_A_ROJO_EN_CURSO_REPITA`, **sin armar el latch** |
| `CANCELAR_AMBAR` | si | **PIDE PIN al reves que armar**, y es deliberado: quitar el ambar devuelve el cruce a dar verdes, o sea **abre paso**. Depende de `ambarEmergencia`: con latch puesto lo retira, avisa al Poste 1 y contesta `RESULT:RETIRADO` (la rama del mando salio con el, `D-30`). Sin latch, `RESULT:REENVIADO_AL_MAESTRO` si esta punta **sigue** en `S_FALLO` (la red de la trama perdida, `N-152`), y `$ERR ... NO_HAY_AMBAR_VIGENTE` si no |
| ~~`SOLICITAR_PASO`~~ · ~~`SET_MODO:DEGRADADO`~~ | — | **Salieron con `D-46` (`f9cad1f`; sin banco):** `$ERR,CMD:DESCONOCIDO,DESC:COMANDO_NO_SOPORTADO_EN_ESCLAVO`. La app ya no las ofrecia (`D-36`, `D-37`). El paso se da desde el Maestro (Manual y DAR PASO, SPEC 1 §8); al Degradado se entra por el testigo |
| `SET_MODO:DEG_T:ahora,inicio,verde,despeje` | si | Construido (`990c278`; `D-35`, SPEC 2 §7.bis). Misma orden, `RechazoTestigo` propio: `DEG_RECHAZO_T_SIN_HORA` / `_AHORA_DESFASADO` / `_INICIO_VENCIDO` —texto «repita el testigo en el Maestro»— / `_DESPEJE_RANGO` / `_AMBAR_VIGENTE`, o `RESULT:OK` / `RESULT:RENOVADO` |
| `SET_MODO:DEG_FIN:ahora,salida` · `...:CANCELAR` · `CONSULTA_DEG_FIN` | si · si · no | Construidos el 04/10 (`a4545bb`, `D-52`), sin banco. Misma orden y mismos literales que el Maestro (§3.1), con `RechazoSalida` propio; aqui muerde «Salida ya vencida», con el MISMO literal que el Maestro (el consejo lo dice la app). Al llegar la hora llama a `iniciarSalida(false)`: rojo, el despeje y `DEG_INACTIVO`, **sin menu**: sin radio queda en ambar por silencio. `DEG_T` con salida programada se rechaza igual |
| `SET_DEG_AUTO:1` / `SET_DEG_AUTO:0` · `CONSULTA_DEG_AUTO` | si · no | Construidos. Los del Maestro (§3.1); aqui el enlace es una orden de gobierno del Maestro en los ultimos `SFTY6_SILENCIO_MS`, y el `ECO` llega en su `PING` |
| `FORZAR_ROJO` | las dos formas | `$ERR ... RENOMBRADO_USE_AMBAR_EMERGENCIA`. **Se rechaza ensenando el nombre bueno**, no en silencio: quien lo manda tiene una app o un manual anteriores al cambio |
| `TEST_LEDS` | si | `$ERR ... NO_EN_SERVICIO_USE_EL_MAESTRO`, **rechazado a proposito**: la secuencia enciende VERDE sin mirar nada, y ese verde saldria mientras el Maestro da paso al otro sentido |

### 3.3 El puente ESP32 de cada poste — `ESP32_Expansion/src/despachador.cpp`

| orden | PIN | que contesta, y **de que depende** |
|---|---|---|
| `CMD:LEER_RTC` | no (es una consulta: no abre, no para, no cambia) | `$ACK,NODE:PUENTE,...,RESULT:OK` con `FECHA:` y `HORA:` **releidas del chip ahora mismo**, o **siete `$ERR` distintos**, uno por cada valor de `MotivoSinHora` —incluido `BARRERA_INCOHERENTE`, que existe para que un defecto del firmware no se disfrace de averia del modulo—, mas un octavo, `MOTIVO_NO_CONTEMPLADO`, para que nadie anada un valor al enum y deje la consulta muda—. Cada literal vive **dentro** de su rama |
| `SET_RTC:<fecha>,<hora>` | **el PIN viaja y no lo comprueba nadie** — ver §7 | siete motivos, uno por valor de `ResultadoReloj`, y el `$ACK` final **devuelve la hora RELEIDA**, no la que se mando. Hay un octavo caso que es `$ACK` y no `$ERR`: `RESULT:HORA_PUESTA_SIN_PROPAGAR` cuando el DS3231 quedo puesto pero `siembra_ahora()` no pudo poner la linea entera en el cable. El detalle es de SPEC 3 |
| cualquier linea con `HORA_ESP32` dentro | — | `$ERR,NODE:PUENTE,CMD:HORA_ESP32,DESC:LINEA_RESERVADA_AL_PUENTE`, **y va la primera de todas**: esa orden solo la origina el modulo, y el STM32 la acepta sin PIN porque el puente no lo conoce |

---

## 3.bis 🟢 LAS BOTONERAS A/B/C/D ESTAN ELIMINADAS — el equipo se opera SOLO por la app

**El hardware ya no existe: se retiro del equipo el 05/09** —*«se opera solo por app»*— y el
responsable lo reafirma el 14/09: *«eliminamos las botoneras A, B, C y D»*. **No hay superficie
fisica de operacion**: ni botones, ni receptor que comprar, ni pantalla —esa salio del firmware el
13/09—. La guia de campo ya retiro su paso de prueba del mando, y **`J16` p5 y p8 se quedan vacios:
no hay nada que cablear ahi**.

✅ **Y EL PROGRAMA YA NO COMPONE NADA CON ELLOS: el mando salio del firmware el 14/09.** `mando.cpp` y `mando.h` de
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
| *«borrar el menu borra el todo-rojo de las dos puntas»* | **Tampoco.** Ese todo-rojo tiene **tres** llamadores: el menu, el Modo Alcance y el Modo Hora, **y los dos ultimos se alcanzan desde la app**. ⬇️ Con `D-44` el Modo Hora salio y `menu_setup()` **se quedo**: es la puerta que piden `SET_MODO:MENU` y la salida de cada modo |

✅ **Y la condicion que puso el responsable se cumplio: el camino de interceptar las luces SALIO
ENTERO con el mando**, en vez de quedarse dentro sin nadie que lo ejerza. Con el fuera, `semaforo.cpp`
escribe los pines por un solo sitio —`aplicarSalidas()`, donde vive el enclavamiento `SFTY-2`— y la
lista de funciones autorizadas a saltarselo **baja de cinco a una**: cualquier camino nuevo a una
lampara es ahora un rojo del banco en vez de una excepcion ya aprobada por su nombre.

✅ **Y CON `D-44` (`f9cad1f`, 02/10; sin banco) SALIO EL RESTO** (SPEC 1 §12.2): `J16` p5/p8 son `J16_P5_SIN_USO` y
`J16_P8_SIN_USO`, `INPUT` sin lector (`botones_actualizar()` solo llama a `camaras_actualizar()`); salieron
`botonArriba`/`Abajo`/`Aceptar`/`Cancelar`, la navegacion del menu, el Modo Hora y `semaforo_toggle()`.
`menu_setup()` se queda: es el todo-rojo de `SET_MODO:MENU`.

## 3.ter EL DEGRADADO CON TESTIGO EN LA APP — flujo (`D-35`, SPEC 2 §7.bis; construido en `11b57b3`)

**Boton nuevo, en LAS DOS pantallas** —Maestro y Esclavo—, gobernado por `puntaCorrecta()` igual que los demas
(`app.js`, §7 hueco nuevo abajo): «Degradado con testigo». ~~Aparece junto a `SET_MODO:DEGRADADO`~~ → desde `D-46` es
la unica entrada al Degradado: `SET_MODO:DEGRADADO` salio del firmware.

**En el Maestro (primera punta que se toca):**
1. El operario escribe el **traslado** —minutos hasta el Esclavo, por defecto **20**— y el **despeje** —30 a 255 s,
   el mismo limite que el firmware exige (§3.1)—. `verde` no se pide: viaja fijo en **180**.
2. La app manda `SET_RTC:<fecha del telefono>,<hora del telefono>` a ESTA punta, igual que siempre —**no bloquea
   el paso siguiente**: el resultado (`RESULT:OK` o `HORA_PUESTA_SIN_PROPAGAR`) se registra en el Diario, pero
   el testigo ya no depende de el (SPEC 2 §7.bis: la garantia la da `ahora`, no el `SET_RTC` previo).
3. La app LEE la hora del telefono OTRA VEZ, justo antes de enviar —es el `ahora` de la orden—, calcula
   `inicio = ahora + traslado` y manda `SET_MODO:DEG_T:ahora,inicio,180,<despeje>`.
4. **Lo que la app GUARDA, para el paso 2 del Esclavo** (localmente, no en el equipo): `inicio`, `verde`,
   `despeje`, la `SERIE:` del Maestro (del `$STATUS`, `NODE:` distingue de cual poste vino) y el instante en
   que se emitio. Sin este dato el operario tendria que teclear los mismos numeros dos veces, y un numero mal
   repetido en el Esclavo es exactamente lo que este modo NO puede permitirse (§8(e): un ciclo distinto en cada
   punta solapa verdes).

**En el Esclavo (segunda punta, tras desplazarse):**
1. La pantalla ofrece «Aplicar testigo guardado — Maestro `<serie>`, inicio `HH:MM:SS`», con `inicio`, `verde`
   y `despeje` del paso anterior, **no un formulario nuevo**: repetirlos a mano es el error que el guardado
   del paso 4 evita.
2. La app manda `SET_RTC` a esta punta primero, igual que en el Maestro — tampoco bloquea aqui.
3. La app LEE la hora del telefono de nuevo —un `ahora` distinto del que uso el Maestro, mas tarde por el
   traslado— y manda `SET_MODO:DEG_T:ahora,inicio,verde,despeje`: `inicio`, `verde` y `despeje` son los
   GUARDADOS, no se recalculan aqui; `ahora` es fresco.
4. **Si `ahora` no coincide con el reloj de esta punta** (mas de `TOLERANCIA_TESTIGO_S`), se rechaza con
   `DEG_RECHAZO_T_AHORA_DESFASADO`; la app sugiere repetir el `SET_RTC`. **Si `inicio` ya pasó**, se rechaza con
   `DEG_RECHAZO_T_INICIO_VENCIDO` y la app traduce el motivo tal cual lo devuelve el firmware: «el testigo
   caducó — repita el paso completo en el Maestro». **No ofrece reintentar con el mismo `inicio`**: el traslado
   ya se cumplió (el motivo no lo arregla el reintento, lo arregla un testigo nuevo).

**Testigo con más de 31 días (el limite de vigencia del testigo):** la app no puede saberlo de
antemano —el contador vive en la tarjeta, no en el telefono—; se entera por el `$STATUS`/`$ALARM` igual que
hoy se entera del ámbar por límite duro (`SPEC 6` Parte C), y el texto es el que ya existe para ese aviso,
sin uno nuevo.

## 3.ter.bis EL DEGRADADO AUTOMATICO EN LA APP — construido en `js/deg_auto.js` (`A-15`, SPEC 2 §7.ter)

**Es una ELECCION DE MONTAJE, no un modo que se pone cuando ya cayo la radio** (decidido el 05/10, tras el banco del
funcional: lo intento sin radio y no entendio el rechazo `SIN_ENLACE_CON_EL_OTRO_POSTE`). Construido en
`js/deg_auto.js` y `js/deg_auto_eleccion.js` (`N-176`, sin banco):
- **La tarjeta (pestana Tecnico, las dos pantallas) es una pregunta**: «Si se cae la radio entre los postes:»
  (a) «Ambar intermitente y voy con el testigo» = `SET_DEG_AUTO:0`, la de fabrica y la de carretera (`D-43`);
  (b) «Entran solos en Degradado a los 5 min sin radio» = `SET_DEG_AUTO:1`. Pide PIN; se elige en LOS DOS postes.
- **Sin radio los botones quedan desactivados** y la orden no sale: con `FALLO COM` fuera de `MODO:AMBAR` o `RF:0%`.
  No se bloquea por `APTO_NO`: con la opcion en OFF el apto es siempre NO.
- **Pantalla principal, linea fija encima de la tarjeta del cruce** con lo que dio `CONSULTA_DEG_AUTO`: «Si se
  cae la radio: AMBAR, ir con testigo» o «… DEGRADADO SOLO a los 5 min»; «Los postes no coinciden» en rojo si ESTE
  y OTRO difieren.
- **Recuadro de la eleccion, «Listo para entrar solo»** (06/10, foto de Marco: «SI» con el otro en OFF y no entro): SI
  solo con `ESTE_ON`, `OTRO_ON` y `APTO_SI`, lo mismo que exige el firmware para entrar. Si no, NO con el motivo:
  «este poste en OFF», «el otro poste no esta listo» (su ultimo APTO oido) o «este poste no cumple las
  condiciones». **Debajo, «Que falta» con el PRIMER paso pendiente y la accion** (responsable, 06/10: «no dice que
  hacer»), en este orden: (1) ESTE OFF: «pulse (b) en este poste»; (2) conectado al Maestro con `MODO` distinto de
  `AUTO`/`INTELIGENTE`: «ponga el POSTE 1 en AUTO o INTELIGENTE y espere a que cicle con radio» (`puertaAbierta()`);
  (3) OTRO OFF: desde el Maestro, «vaya al POSTE 2, pulse (b) y espere 10 s»; desde el Esclavo, «el POSTE 1 debe
  estar en AUTO o INTELIGENTE con (b) pulsada»; (4) APTO NO: «ponga la hora y compruebe que hay radio»; todo bien:
  «nada: si la radio cae 5 min, entran solos».
- **Al conectar**, si no consta o no coincide, aviso que pide elegir («Mas tarde» lo pospone hasta la siguiente).
- **Al caer la radio**, cartel segun la eleccion: (a) «quedan en AMBAR y NO arrancan solos: hay que ir con el
  testigo», con boton al testigo; (b) cuenta «aprox. m:ss» desde que la app ve `FALLO COM` (llega tarde respecto al
  reloj del firmware: por eso «aprox.»), y con `AUTO_ENTRADA_INICIO` o `MODO:DEGRADADO`, «En Degradado por reloj».
  La cuenta (b) sale solo con `ESTE_ON`, `OTRO_ON` y `APTO_SI`: es lo que el firmware exige para entrar
  (`degAuto_loop()`: `respaldo_otroApto()` y `puertaAbierta()`, y `entrar()` repite la comprobacion de
  `degAuto_aptoPropio()`; el ECO `respaldo_aptoDado()` no llega a la app). En cualquier otro caso sale el cartel (a),
  con boton al testigo y una linea con el motivo: ESTE y OTRO distintos, «Los postes no tienen la misma eleccion»;
  `ESTE_ON_OTRO_ON` con `APTO_NO`, «Este poste no esta listo para entrar solo». Si la cuenta pasa de 0 en mas de
  60 s sin `MODO:DEGRADADO` ni `AUTO_ENTRADA_INICIO` (cubre el ECO, que la app no ve): «No entraron solos: quedan en
  AMBAR. Hay que ir con el testigo», con boton al testigo.
- Mientras espera el `$ACK` diferido (hasta `DEG_AUTO_ACUSE_MS`) queda «esperando al otro poste», no el valor pedido.

**Eventos y alarmas que traduce, a texto de operario:**
- `DEGRADADO` / `AUTO_ENTRADA_INICIO_HH:MM:SS` — «Sin radio 5 min: modo degradado automatico, rojo hasta HH:MM».
- `DEGRADADO` / `ENLACE_DISPONIBLE` — un **cartel fijo** (`js/carteles_equipo.js`, §6): «LA RADIO VOLVIO entre los
  dos postes. Siguen en Degradado.» y, para volver al ciclo, Volver al menu en el Poste 1. El firmware lo repite
  mientras oiga al otro (SPEC 2 §7.ter (c)): toast y linea solo la primera vez por conexion; despues se refresca la
  hora del ultimo aviso. Lo quitan Entendido, soltar el enlace o un `$STATUS` con otro `MODO`.
- `$ALARM DEGRADADO,CAUSA:AUTO_NO_<codigo>` (codigos en SPEC 2 §7.ter (b); tabla `DegAuto.MOTIVO`) — «No pudo
  entrar solo: <motivo>.» y lo que dice la `ACCION`: con `SIGUE_AMBAR`, «Sigue en ambar»; con `QUEDA_ROJO` (solo el
  Maestro con `GUARDADO`), «Queda en ROJO FIJO en este poste, no en ambar: avise a mantenimiento».
- `$ALARM DEGRADADO,CAUSA:OTRO_EN_DEGRADADO,ACCION:REVISE_OTRO` — «El otro poste esta en degradado y este no: vaya
  al otro poste».
- `$ALARM DEGRADADO,CAUSA:RENOVAR_TESTIGO,ACCION:REPITA_TESTIGO` (28 dias sin renovar) — «vaya a los dos postes y
  repita el Degradado con testigo». Sin vencimiento: la luz no cambia (29/09).

**Y el `$STATUS` del Maestro en Degradado publica `ESC:?`**: la app lo pinta como «desconocido», no como el ultimo
color que vio (SPEC 2 §7.ter (e)).

## 3.ter.ter LA SALIDA PROGRAMADA Y EL ROJO TOTAL EN LA APP — construido el 04/10, sin banco (`D-52`, `D-51`; SPEC 2
§7.quater)

**Respuestas del firmware (construido el 04/10, `a4545bb`, sin banco).** `FORZAR_ROJO` en Degradado, cada una segun lo
que devolvio `modo_degradado_forzarRojo()`: `RESULT:ROJO_FIJO_EN_ESTE_POSTE` · `RESULT:YA_EN_ROJO_FIJO` ·
`RESULT:SALIDA_YA_EN_CURSO` · `RESULT:OK` (fuera del modo). En la ventana de `D-29` (permiso esperando la siembra)
revoca el permiso y entra en ROJO FIJO, nunca ambar: `OK_REANUDACION_CANCELADA`. `SET_MODO:MENU` en esa ventana:
`OK_REANUDACION_CANCELADA`.
no cruza a nadie.

`SET_MODO:DEG_FIN`, un `$ERR ... DESC:<texto>` por motivo y en este orden: `FORMATO_INVALIDO` · «No esta en
Degradado» · «Ya esta saliendo» · «Falta: reloj sin poner en hora» · «Ahora no coincide» (`TOLERANCIA_TESTIGO_S`) ·
«Salida ya vencida» (mas de 12 h por delante) · «Salida antes del inicio» · «En verde: repita en rojo» · «No se pudo
guardar la salida» · «No hay salida programada» (`CANCELAR` sin nada). `$ACK` solo si quedo programada:
`RESULT:PROGRAMADA` · `REPROGRAMADA` (solo con respaldo en flash) · `PROGRAMADA_SIN_RESPALDO` (sin respaldo: rojo total
o sin registro; un corte la pierde; gana a `REPROGRAMADA`) · `CANCELADA`.

**Lo que HACE la app (`7cd3042`, sin banco ni telefono):** `FORZAR_ROJO` sale sin aviso de paleteros
(`js/aviso_degradado.js`, `aplica()` solo mira `SET_MODO:*`, `SET_DEG_AUTO:0` y las dos órdenes del Esclavo). La clave
`FORZAR_ROJO|SALIENDO_TODO_ROJO` ya no existe en `ACK_TEXTO` (`app.js`).
1. **Texto de `FORZAR_ROJO` en Degradado**, una entrada por `RESULT` (SPEC 4 §3.1): `ROJO_FIJO_EN_ESTE_POSTE` dice
   «ROJO FIJO en ESTE poste. El OTRO POSTE SIGUE ALTERNANDO por su reloj: vaya a él. Se sale con la salida programada o
   con Volver al menu»; `YA_EN_ROJO_FIJO`, «ya estaba en rojo fijo; nada cambia»; `SET_MODO:MENU` con
   `OK_REANUDACION_CANCELADA`, «volvió al menú; el degradado que esperaba la hora NO se reanudará». Desaparece la clave
   `SALIENDO_TODO_ROJO` de esta orden; `SALIDA_YA_EN_CURSO` se queda (ya está saliendo al menú). **Sin aviso de
   paleteros** (decidido, SPEC 2 §7.quater (b)).
2. **Botón «Programar salida del Degradado»** en las DOS pantallas, un bloque por punta según `state.node` (si no
   coincide, aviso y no sale nada), con el mismo flujo
   que el testigo (§3.ter): en el Maestro se escribe la hora de salida (por defecto `ahora` + el traslado, editable) y
   la app manda `SET_MODO:DEG_FIN:ahora,salida` con `ahora` releído justo antes de enviar; **guarda `salida`** (local) y
   en el Esclavo ofrece «Aplicar salida guardada — Maestro `<serie>`, salida `HH:MM:SS`» sin formulario nuevo, con un
   `ahora` fresco. **Lleva el aviso de paleteros** (`aplica()` ya cubre `SET_MODO:DEG_FIN`; el vale se compara por el
   nombre de la orden, no por sus argumentos). Botones «Reprogramar» y «Cancelar salida» (`SET_MODO:DEG_FIN:CANCELAR`).
3. **Tarjeta de estado** (la pestaña Técnico, como `js/deg_auto.js`): pide `CONSULTA_DEG_FIN` al conectar y tras cada
   orden y pinta «sale a HH:MM:SS, faltan N s» contando hacia atrás sola, con la marca «sin respaldo» si el firmware la
   dio. **Si esta app programó el Maestro y no consta el Esclavo, el cartel queda fijo:** «salida programada en el
   Maestro; FALTA el Esclavo» (el firmware no puede saberlo sin radio; riesgo SPEC 2 §7.quater (h).1).
4. **Traduce** cada `$ERR` de `DEG_FIN` (un texto por motivo; «Salida ya vencida» dice «reprograme la salida en el
   Maestro o cancélela», **no ofrece reintentar** con la misma hora), cada `RESULT` (`PROGRAMADA`, `REPROGRAMADA`,
   `PROGRAMADA_SIN_RESPALDO`, `CANCELADA`), `$EVENT DEGRADADO,SALIDA_PROGRAMADA_*` y `$ALARM DEGRADADO,CAUSA:ROJO_TOTAL,
   ACCION:ROJO_FIJO` («rojo total puesto por una persona; el otro poste sigue alternando»), que NO es `ROJO_SIN_HORA`.

**Ficheros (HACE, `7cd3042`).** `js/deg_fin.js` (modulo `DegFin`: botones, tarjeta con cuenta atras, motivos y
`$EVENT`; la tarjeta se arma por JS porque `index.html` no crece) · `js/deg_auto.js` (reparte a `DegFin` desde los
enganches que `app.js` ya llamaba: `enviarFin`, `consultarFin`) · `js/aviso_degradado.js` (el vale del aviso de
paleteros se casa por NOMBRE de orden, no por la trama: `DEG_FIN` lleva un `ahora` distinto en cada envio) ·
`app.js` (entradas de `ACK_TEXTO` y `CONSULTA_DEG_FIN` en `SIN_PIN`). Copias en `www/` y en la APK.
**Limites que HACE y quedan escritos:** un `CANCELADA` que esta app no pidio (otro telefono) no borra el cartel local
«FALTA el Esclavo», aunque la tarjeta si refleja la consulta; y la app mide la ventana de 12 h con el reloj del
telefono y el poste con el suyo, asi que en el borde pueden discrepar: manda el `$ERR` del poste.

**Aviso `$EVENT DEGRADADO ... SYNC:<h>h AVISO:SI|NO VENCIDA:SI|NO` (HACE, sin banco; `deg_auto.js`).** El evento no
dice si el poste tiene testigo y la app no lo deduce de `h`; se publica en todo modo y en las dos puntas. `VENCIDA:SI`
va primero, con o sin `AVISO` (sin testigo y sin fecha el Maestro publica `AVISO:NO VENCIDA:SI`): «sin sincronizacion
de radio valida (48 h o mas, o sin fecha): este poste no da verdes por reloj; la luz que tiene la ve en su estado en
esta pantalla: ambar, o rojo fijo si llega la alarma de reloj parado». No remite a una alarma de la rendicion: el
Maestro emite `LIMITE_48H` pero el Esclavo se rinde sin ninguna (SPEC 6, fila `LIMITE_48H`, solo Maestro), y con el
reloj parado (`D-49`) el poste esta en rojo fijo con la trama diciendo `VENCIDA:SI`. `AVISO:SI` sin `VENCIDA:SI`: un
solo texto, «lleva <h> h sin radio» (sin la cifra si falta `h`), «si este poste tiene testigo, renuevelo en los dos
postes; si no, recupere la radio antes de 48 h». La orden de renovar la da tambien la alarma `RENOVAR_TESTIGO`, que no
sale sin fecha.

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

- **La unica orden que sale sin que nadie pulse es `CMD:VERSION`** (sin PIN, no cambia nada): una vez por enlace,
  a los `TIMEOUT_ENLACE_MS` de conectar, solo con telemetria viva y si el CONTROLADOR no ha contestado ya
  (`EVT:VERSION` es el sello del puente). No renueva la inactividad del PIN. El sello de cada `NODE` va en la linea
  `Firmware:` de la cabecera del diario y de la cinta, y **«Exportar todo como archivo»** saca los dos en un
  `IOTVIAL_<serie>_<AAAAMMDD-HHMMSS>.txt` (`js/exportar.js`). En el navegador sale por la hoja de compartir con
  el fichero adjunto o, si no la hay, como descarga. En la APK se escribe en la cache de la app
  (`@capacitor/filesystem`, `Directory.Cache`) y se abre la hoja de compartir de Android con ese fichero
  (`@capacitor/share`, por el `FileProvider` `<applicationId>.fileprovider` y el `<cache-path>` de
  `file_paths.xml`); cerrar la hoja se dice como cancelado, y un fallo, o una APK sin los dos plugins, va en rojo a
  la bitacora con «use Copiar». Sin probar en un telefono (§7.12).

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
| cabecera | `MAESTRO (POSTE 1)` + «(radio N %)» con `RF:` medido | `ESCLAVO (POSTE 2)` + «(enlazado)» — lo decide `NODE:`, no lo que el operario suponga. En las dos, «(sin radio entre postes)» con `FALLO COM` fuera de `MODO:AMBAR` o `RF:0%`, y «(sin enlace)» sin `$STATUS` |
| `MODO:` | `MENU` `MANUAL` `AUTO` `INTELIGENTE` `ALCANCE` `DEGRADADO` `AMBAR` (~~`HORA`~~, `D-44`) | `SUBORDINADO` `DEGRADADO` `RENDIDO` — el campo dice si esta punta esta obedeciendo o gobernando |
| `ESTADO:` | `ROJO` `VERDE` `AMARILLO` `FALLO COM` (con espacio); *`ROJO+AMAR` con `D-53` (HUECO)* | los mismos |
| cuenta atras `T:` | numero o `--` | **`--` FIJO** |
| enlace `RF:` `RTT:` | medidos | **`--` FIJOS** |
| bateria `BAT:` | **`--` FIJO** en las dos puntas: el equipo declara que **no la mide** —falta el divisor y la entrada analogica—, y la app lo dice con esas palabras en vez de pintar un numero |
| boton de emergencia | `FORZAR_ROJO` | `AMBAR_EMERGENCIA` + `CANCELAR_AMBAR` |

**`D-45` (`N-174`; construido en `cda33df`, sin banco) cambio lo que significa `ESTADO:AMARILLO`, no el formato.**
Hasta `cda33df` salia al ABRIR (de rojo a verde); ahora sale al CERRAR, durante `AMARILLO_SEG` al final de cada
verde, y despues `ROJO` (`semaforo_nombreEstado()`; SPEC 1 §3.2). Consecuencias para la app: **(1)** su frase
*«AMARILLO, este poste esta cerrando su paso»* (`app.js`) es ahora cierta; **(2)** con la luz
en `AMARILLO` la app pinta «amarillo de cierre: baja al pasar a rojo» (`app.js`, frase de la pluma): en el amarillo la
pluma esta arriba por diseno (`luzPideArriba`), no por un veto (SPEC 8 §1); **(3)**
`FORZAR_ROJO` contesta `$ACK` al aceptar la orden y el `ROJO` llega tras el amarillo.
🔴 **ABIERTO: `ESC:AMARILLO` no se construyo.** `coordinador_estadoEsclavo()` publica lo ultimo que consta del Poste
2 y dice `VERDE` hasta el `CMD_ACK_RED`, tambien durante su amarillo de cierre (SPEC 2 §2.2.bis). La propuesta
—`ESC:AMARILLO` desde la orden de rojo que cierra un verde del Poste 2 hasta su acuse— sigue **sin decidir**.

**HUECO `D-53` (`N-176`, sin construir): `ESTADO:ROJO+AMAR`**, 9 caracteres, el tope del campo
(`semaforo_nombreEstado()`, SPEC 1 §3.3 (9)), durante los `ROJO_AMARILLO_SEG` antes de cada verde, en las dos
puntas. Hoy la app lo pintaría
«ESTADO NO RECONOCIDO». Lo que debe hacer: **(1)** una fila en `ESTADOS` de `app.js` con las lámparas roja y ámbar
encendidas, texto «ROJO Y AMARILLO» y frase *«ROJO Y AMARILLO, este poste va a abrir su paso»*, sin la cifra: un
número que la app no puede recalcular no se escribe en `app.js` (`CLAUDE.md` §14); **(2)** frase de la pluma: abajo,
*«sube con el verde»* (SPEC 8 §1); **(3)** `ESC:` no cambia: dice `ROJO` hasta el `CMD_ACK_GREEN` (SPEC 2 §2.2.ter);
**(4)** las copias de `app.js` (`metodo.md` §14) llevan la misma fila.

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
Los cuatro campos fijos siguen fijos: **el dato nuevo no va en el `$STATUS`, va en el Diario.** La app lo pinta en un
recuadro propio de la ventana del Poste 2 (`js/diagnostico_enlace.js`, `renderDiagnosticoEnlace()`; §7.3).

🟢 **EN LOS DOS POSTES, LA APP DESTACA EL AVISO DE BARRERA RETENIDA, y no se puede perder con el scroll.** Un solo
`$EVENT` de `ORIGEN:CAMARA_PLUMA` con `DETALLE:VETO_SOSTENIDO_S:` abre un cartel propio
—`js/aviso_camara_pluma.js`, primer hijo de `.app-container`— que vive **fuera de la bitacora y fuera de las
pestanas**, publica los segundos que el equipo mide y manda a **mirar debajo del brazo antes de tocar la camara**;
las repeticiones lo refrescan sin gastar una linea de registro, y el `VETO_ACTUADO_N` del flanco se traduce como lo
normal que es, sin cartel. **No afirma «camara averiada»**: el equipo no ve imagen y no separa un vehiculo parado de
un mal apunte (`D-12`), asi que publica la medida y el juicio queda en quien esta delante del poste. Solo un
`PLUMA:ABAJO` del `$STATUS` le cambia el titulo —el cartel **no se retira solo**, porque lo que la dejo arriba sigue
sin revisarse— y el enlace caido lo borra, que es de un poste.

🟢 **DOS CARTELES MAS, CON EL MISMO MOLDE** (fuera de la bitacora, con Entendido; soltar el enlace los borra):
- **Corte de luz** (`js/aviso_corte.js`). El puente manda al conectar su parte de arranque
  (`$EVENT,NODE:PUENTE,EVT:ARRANQUE,CAUSA:...`); con la causa de subida de tension la app abre «ESTE POSTE SE QUEDO
  SIN LUZ», dice que **no se sabe cuando** —la trama no trae hora y sale en cada conexion, tambien tras el primer
  encendido— y pide revisar el modo, con el poste y el `MODO:` del `$STATUS` de esta conexion. Las demas causas van
  a una linea traducida, sin cartel. Un reinicio del STM32 solo no lo abre: es el arranque del ESP32.
- **Radio de vuelta en Degradado** (`js/carteles_equipo.js`, §3.ter.bis): una tabla de `ORIGEN|DETALLE`; hoy una
  fila, `ENLACE_DISPONIBLE`.

**Y el rojo fijo por hora perdida** (`D-38`, SPEC 3 §5) se traduce en `js/avisos_equipo.js`: `ROJO_SIN_HORA` dice
que ese poste esta en rojo fijo, que el otro puede seguir alternando y como salir; `HORA_ESP32` / `CADUCADA` dice
que dejo de alternar y **no vuelve solo**. El sello `EVT:VERSION` sale solo en su linea «Version cargada», sin una
segunda linea vacia en Eventos.

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

🟢 **3 · `D-23`: FIRMWARE Y APP CONSTRUIDOS (sin telefono ni banco).** `D-32` (2) resolvio el mecanismo: como
el STM32 **no puede saber** que un telefono se conecto —el unico que lo ve es el ESP32— y traer ese aviso seria **la
tercera orden que el accesorio origina hacia el micro** (`esp32_05_no_origina` la condiciona **por escrito** a una fila
de `DECISIONES.md`), el responsable eligio **`$EVENT` periodico a cadencia baja**, que no toca ninguna barrera. En la
app, `js/diagnostico_enlace.js` lee ese `$EVENT` y `renderDiagnosticoEnlace()` lo pinta en un recuadro de la ventana
del Poste 2 (contadores acumulados y su diferencia con la muestra anterior), **sin semaforo de color ni veredicto**: el
umbral que diria «esto va mal» no lo ha decidido nadie. `app.js` no nombra `ENLACE_RF`; lo nombra el modulo.

🔴 **4 · Dos `$ACK` que no dependen de lo que la orden hizo: `SET_MODO:ALCANCE` y `SET_MODO:INTELIGENTE`.**
Las dos ramas contestan `RESULT:OK` despues de `modoActual_set()` **y nada mas**. El trabajo de entrar en
el modo vive en `modoAlcance_setup()` y `modoInteligente_setup()`, y `Maestro/src/main.cpp` los llama
**solo bajo `if (modo != modoAnterior)`**: repetida la orden con el modo ya puesto, el equipo contesta OK
y **no corre nada**. Es la **misma forma** que `N-146` reparo en `SET_MODO:AMBAR` —capturando `yaEnModo`
y contestando `REARMADO`— y que el Esclavo reparo en su `SET_MODO:DEGRADADO` —capturando `antesDeg` y contestando
`YA_ACTIVO`; esa orden salio con `D-46`—;
las dos unicas ramas de modo que **no** tienen la forma son `AUTO` y `MANUAL`, que son justo las que
llaman al coordinador **dentro** de la rama. **Lo que cada una se deja, medido:** en `ALCANCE`,
`modoAlcance_setup()` es el **unico llamador** de `protocolo_reiniciarContadores()` en el Maestro, o sea que **la
puesta a cero de SFTY-15 no ocurre en la segunda pulsacion**; en `INTELIGENTE`, no se re-ejecuta
`coordinador_iniciarModo()`. **El par peligroso es alcanzable por el mismo camino que `N-146`:**
`CMD:FORZAR_ROJO` entra **sin PIN desde cualquier modo**, mueve la luz y **no toca `modoActual`** (salvo en
Degradado, donde es rojo fijo en ese poste, §3.1). ⚠️ **Lo que NO
se afirma:** no esta medido en banco ni en tarjeta, es lectura del fuente, y **no deja el cruce trabado**.

⚠️ **5 · El Modo Alcance no tiene superficie.** Todo lo que ese modo produce sale por
`lcd_dibujarAlcance()`, y **el LCD se retiro** (`D-17.bis`). Desde el telefono, `MODO:ALCANCE` ensena el
mismo `RF:`/`RTT:` que cualquier otro modo —vienen de la ventana de latidos del coordinador, **no** de los
contadores que ese modo pone a cero—. Es un modo de medida sin nadie que lea la medida, y es `D-16` en su
forma mas simple.

🟢 **6 · CERRADO POR `D-44` (`f9cad1f`) — `menu.cpp` NO ERA SOLO PANTALLA.** `menu_setup()` llama a
`coordinador_forzarMenu()`, el todo-rojo de las dos puntas, alcanzable desde la app con `SET_MODO:MENU` (§3.1, una de
las cuatro ordenes sin PIN del hueco 1). `D-44` saco la navegacion y **dejo `menu_setup()`** con ese nombre: la puerta
no se movio.

⚠️ **7 · Una ventana de ~1 s por conexion en la que una orden de Maestro puede salir contra un Esclavo.**
`SOLO_MAESTRO` pregunta `=== 'ESCLAVO'`, asi que con `state.node` en `null` —entre que el socket abre y
llega el primer `$STATUS`— la orden sale. El firmware la rechaza con `$ERR,CMD:DESCONOCIDO`, **asi que no
mueve una luz**; lo que se pierde es el aviso claro al operario. Esta **abierta a proposito**: cerrarla
rompe el arnes del puente, que pulsa antes del primer `$STATUS`.

⚠️ **8 · Desde el `$STATUS` la app no puede saber que reloj sello la `HORA:`.** Hoy siempre es el DS3231
del puente, porque el otro no existe, **pero la trama no lo dice**. Residual declarado en
`sellarHoraSiFaltaba()`; su cierre es de SPEC 3.

🟢 **9 · CERRADO — construido en `f57a401`.** Ya no es cola de trabajo: lo que la app HACE con el aviso de
barrera retenida esta descrito en §6. El numero se queda para que las citas de fuera no queden cojas.

🟡 **10 · Los dos interruptores de administrador no existen, y su estado NO CABE en la trama de hoy.**
Decididos el 14/09 (SPEC 5 §4.1), y se llaman **los dos interruptores de administrador**: *sacar la
camara del veto* y *sacar la barrera de servicio*, uno por poste, sobreviviendo a un corte de luz. 🔴 **Y el obstaculo esta MEDIDO antes de construirlos, no
despues:** la trama de estado del poste 1 va llena — **le sobran 3 bytes de 155, y un campo nuevo no
cabe en 3 bytes**—, asi que **el estado de los dos interruptores no puede viajar ahi**. Hay una salida
ya medida que liberaria otros 3 recortando el campo de la hora, **y aun asi habria que comprobar que
alcanza**. Como se publican es una decision pendiente: un aviso propio, o ese recorte. **Mientras no se
decida, no se construyen**: un interruptor que el tecnico no puede ver encendido es peor que no
tenerlo, porque deja un poste degradado sin que nadie lo sepa.

⚠️ **11 · Lo que 1.49 dejo a medias en esta spec** (medido sobre `622a20b`). **(a)** El Maestro emite
`$ALARM DEGRADADO` con tres causas al caer por su limite (SPEC 6 PARTE C) y **`js/avisos_equipo.js` no tiene
entrada para ninguna de las tres** (de ese evento solo traduce `ROJO_SIN_HORA`): `traducirAlarma()` devuelve `null`
y la alarma se pinta en crudo, justo la que dice si mirar la radio o el reloj.
**(b)** El acuse diferido de `REINICIAR_RELOJ` (§3.1) **no lo recorre ningun
arnes en el tiempo**: el doble del arnes del puente acepta la orden que fija el veredicto del cristal, pero
ningun guion la manda; lo que hay son packs de texto sobre la forma de la rama (`reloj_01`, `app_08`).

⚠️ **12 · «Exportar todo como archivo» en la APK: construido, SIN PROBAR EN UN TELEFONO.** Hecho: `@capacitor/android`
6.2.1 (su WebView no registra `DownloadListener`, asi que `<a download>` no hace nada) con `@capacitor/filesystem`
6.0.4 y `@capacitor/share` 6.0.4, versiones fijadas en `package.json`; `npx cap sync android` los da de alta en
`capacitor.settings.gradle` y `app/capacitor.build.gradle`. Lo que se ha medido en el PC es la app contra dobles
de los dos plugins (`tests/test_unitarios.js` §12 y `test_dom_execution.js`). **Queda sin medir:** que la APK
compile con los dos modulos; que el puente publique los dos en `window.Capacitor.Plugins`; que el `FileProvider`
acepte la ruta de la cache; y que WhatsApp reciba el `.txt` como adjunto y no como texto. Se cierra con la APK
instalada: exportar, enviarlo a un chat y abrir el fichero recibido. Hasta entonces, si falla, se copia el texto.

🟢 **13 · ~~EL DEGRADADO CON TESTIGO (§3.ter, `D-35`) ES CERO CÓDIGO EN LA APP~~** → construido (`11b57b3`).

🟢 **14 · ~~EL DEGRADADO AUTOMATICO (§3.ter.bis, `A-15`) ES CERO CODIGO~~** → construido (`a0d605b`, `1a78873`;
app en `js/deg_auto.js` y `js/aviso_degradado.js`), sin banco.

🟢 **15 · ~~LA SALIDA PROGRAMADA Y EL ROJO TOTAL CORREGIDO NO TIENEN CODIGO EN LA APP~~** → construido (`7cd3042`,
§3.ter.ter; `js/deg_fin.js`), sin banco ni telefono.

---

## 8. QUIEN EJERCE CADA BARRERA DE ESTE DOCUMENTO

> **Una spec puede describir barreras que ningun compilador ejerce, con UNA condicion: que cada barrera lleve
> escrito QUIEN la ejerce.** El criterio es `CLAUDE.md` §6.3 — **¿algun arnes COMPILA ese `.cpp`?**; si solo lo
> lee por texto no ve un defecto del TIEMPO, y es *vigilada por texto*, no *ejecutada*. Filas = las de la
> compuerta; reparto de `.cpp` por arnes, `ARQUITECTURA.map` §4-5. **Medido sobre `ef3504c`.**

| barrera | quien la EJERCE hoy |
|---|---|
| §4 **el PIN del firmware** (`CMD:PIN:<pin>:<accion>`) | ✅ **fila 20**: `Simulaciones/puente_esp32/compilar.ps1` **enlaza el `bluetooth.cpp` REAL de las DOS puntas** y le teclea ordenes con el prefijo releido del fuente · **fila 18** ademas en el Esclavo (bloque H) |
| §3 **el `$ACK` depende de lo que la llamada devolvio** | ✅ **filas 20 y 18**, sobre esos mismos `bluetooth.cpp` reales: la 20 compara rama por rama lo que contesta el fuente. ⚠️ **Salvo el `$ACK` diferido de `REINICIAR_RELOJ`**, que sale fuera de la rama y solo lo vigila texto (hueco 11) |
| §4 **el PIN de la app** y §2 **el enrutado por punta** | ✅ **filas 9 a 12**, y la **12** corre `app.js` entero en jsdom inyectando `$STATUS` reales |
| §5 **`juzgarTrama()`, el XOR-8 y las cinco tramas** | ✅ **fila 12**, que inyecta tramas corruptas contra el `app.js` real |
| §4 **el VALE DE VIA (`viaConfirmadaVigente()`)** y el `bool` de `enviarComandoFirmware()` | ✅ **fila 12** |
| §3.3 **el despachador del ESP32** — `D-15`, `esParaElPuente()`, los siete `$ERR` de `LEER_RTC` | 🔴 **NADIE.** `ESP32_Expansion/src/despachador.cpp` **no lo compila ningun arnes**: solo lo cruza PlatformIO, y la fila 20 **lo modela en Python** (`ARQUITECTURA.map` §3.7, nota 3) |
| §1 **`transporte_escribir()` mira `hasClient()`** | 🔴 **NADIE.** `X:transporte_app.cpp`, igual que el anterior |
| §1 **`TRAMA_MAX_UTIL` y `BUF_ENTRADA_STM32`** | 🟡 **texto** (`esp32_06_no_parte_tramas`, `esp32_09_contrato_de_bytes`, `esp32_07_presupuesto_bytes`) |
| §1 **la rama del latido que sale sin contestar** | 🟡 **texto** (`esp32_08_silencio_no_es_orden`, y el censo de la fila 20, que la excluye por nombre) |
| §1 **`sellarHoraSiFaltaba()`**, la unica trama que el puente modifica | 🟡 **texto** (`esp32_13_siembra_de_hora`, `reloj_02_siembra_que_miente`); nadie compila `X:siembra.cpp` |
| §3.1 **las respuestas de `FORZAR_ROJO` en Degradado y las de `SET_MODO:DEG_FIN`** (`D-51` corregida, `D-52`) | 🔴 **Construido el 04/10 (`a4545bb`), sin banco.** Las respuestas las ejecuta el arnes del puente con `bluetooth.cpp` real (arquitecto 04/10, cond. 4); banco: SPEC 2 §7.quater (i) |

**Cuenta: 10 filas y 12 barreras — 7 ejecutadas, 2 sin nadie, 3 vigiladas solo por texto.** 🔴 **Los dos rojos son
el MISMO hecho: el ESP32 entero (9 `.cpp`) no se ejecuta en el PC en ningun sitio**, asi que todo lo que esta spec
dice del puente —`D-15`, los limites de linea, los siete motivos de `LEER_RTC`— descansa en packs que leen texto.
⚠️ **Y una refutacion que conviene dejar escrita: `Maestro/src/bluetooth.cpp` SI lo compila alguien** —la fila 20,
junto con el del Esclavo—; lo que no compila nadie es el despachador del **ESP32**, que es otro fichero.

## 9. VERSION SIGUIENTE — decidido el 09/10, NO construido

Lo que el equipo DEBE hacer en la proxima version de la app; hoy no lo hace. Detalle y orden en
[`App_Semaforo/ROADMAP_APP.md`](App_Semaforo/ROADMAP_APP.md). `D-55`: registro en el equipo (ESP32 del puente) y las
cuatro mejoras del registro de la app del 06/10. `D-56`: tiempos, Degradado, depuracion y hora, solo con la clave de
administracion. `D-57`: clave de soporte para recuperarla, fuera del repo, como huella con sal. `D-58`: modo de
practica sin poste con varios equipos simulados, marcado SIMULADO, nunca con un equipo conectado y tras clave.

### 9.1 Las preguntas las hace la app, no el navegador (`ROADMAP_APP.md` 5) — construido, sin telefono

Lo que HACE (contra el fuente; lo ejerce `tests/dom_dialogos.js` sobre el `app.js` real): ninguna pregunta sale
por `prompt`, `confirm` o `alert` del navegador: el nativo sale en el idioma del sistema (en ingles en la APK de
Pasos Peatonales), bloquea el hilo (colgo una corrida E2E, `N75-4`) y en jsdom no se contesta.
Las seis que hay (vaciar la bitacora del enlace, la cinta de tramas y el diario de ordenes; crear un cruce, nombre y
ubicacion; renombrarlo) salen por un dialogo propio en espanol (`js/dialogo.js`, `#dialogo-modal`):
- el titulo dice QUE se borra o que se pide, y el boton nombra la accion («Vaciar», «Guardar»), no «Aceptar» ni
  «OK»; Cancelar a la izquierda y la accion a la derecha, como en el aviso de via;
- Cancelar, la ✕ o tocar fuera no borran ni guardan nada; un nombre vacio no crea ni renombra;
- crear un cruce son dos preguntas seguidas, con los campos VACIOS (el `prompt` proponia nombre y PR al azar con
  `Math.random()`: un valor inventado que se guarda con un toque); sin nombre no se guarda; cancelar la segunda lo
  guarda sin ubicacion, como el `prompt`.
