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
| `SET_MODO:ALCANCE` | **no** | **tres** (`D-65`, §9.5; construido, sin banco): `$ERR ... EN_MARCHA_PARE_EL_MODO` si `modoActual_get()` es `MODO_DEGRADADO`; `RESULT:SIN_CAMBIO` si ya era `MODO_ALCANCE` —leido ANTES de `modoActual_set()`, molde `N-146`, y no se vuelve a correr nada—; `RESULT:OK` si no |
| `SET_MODO:INTELIGENTE` | si | igual que la anterior: `$ERR` en Degradado, `RESULT:SIN_CAMBIO` si ya era `MODO_INTELIGENTE`, `RESULT:OK` si no (`D-65`, §9.5; construido, sin banco) |
| `SET_NOCHE:n,dias,HHMM,HHMM,A/R` · `SET_NOCHE:n,OFF` · `SET_NOCHE_TR:s` · `CONSULTA_NOCHE` | si · si · si · no | Construido en `N-179` (`D-110`, SPEC 0 §5.10 (h); `modoNoche_orden()`), sin banco. Molde `SET_TIEMPOS`: `$ERR ... FORMATO_INVALIDO` / `RANGO` / `EN_NOCHE_SALGA_PRIMERO` / `FLASH_NO_RELEE`, y `RESULT:OK` solo si lo releido de la flash es lo pedido. La consulta contesta `$ACK,CMD:NOCHE,RESULT:OK,TR:<s>,F1:..,F4:..` (131 caracteres como mucho). Solo el Maestro; el Esclavo, §3.2. ~~`SET_MODO:DEGRADADO`~~ salio con `D-46` (`COMANDO_NO_SOPORTADO`) |
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
| `REINICIOS` | no (es una consulta) | `D-60`, §9.4 (construido, sin banco): `RESULT:OK` con `REIN`, `CAUSA`, `UP` y `ANTES` |

### 3.2 Esclavo (poste 2) — `Esclavo/src/bluetooth.cpp`, `procesarComando()`

**`AMBAR_EMERGENCIA` tiene DOS puertas —sin PIN y con PIN— con el MISMO bloque letra por letra.** No se
factoriza a proposito: los packs (`esclavo_07`, `esclavo_08`) leen el bloque de CADA rama, y una respuesta
que viviera en una funcion comun los dejaria midiendo un bloque vacio (`N-89`).

| orden | PIN | que contesta, y **de que depende** |
|---|---|---|
| `AMBAR_EMERGENCIA` | **no** | **seis literales, y ninguno promete de mas.** Con la luz libre (`!degradado_gobiernaLuz()`) el ambar se enciende ya; lo que el `RESULT` anade es **si el aviso al Poste 1 puede haber sido oido**, y eso NO sale de `protocolo_enviarPaquete()` —que es `void`— sino de dos banderas: `enlaceCaidoAnunciado` (esta punta ya declaro que se quedo sin radio) y `avisoAmbarConfirmado` (`D-31`: el Poste 1 acuso un aviso anterior de ESTE ambar). De ahi `OK`, `OK_SIN_RADIO`, `OK_POSTE1_AVISADO` y sus tres gemelos `YA_EN_AMBAR_LATCH_PUESTO...` cuando la luz ya estaba en ambar y lo unico nuevo es el latch. **`SIN_RADIO` gana a `POSTE1_AVISADO`**: el acuse es de hace un rato, la caida es de ahora |
| `AMBAR_EMERGENCIA` (con el Degradado gobernando) | — | `RESULT:SALIENDO_TODO_ROJO` si hay salida iniciada y `RESULT:SALIDA_YA_EN_CURSO` si es una rendicion (`degradado_rendicionEnCurso()`) — **no `OK`**, porque el ambar tarda el todo-rojo entero. Y si la salida en curso termina en ROJO porque la pidio otro: `$ERR ... SALIDA_A_ROJO_EN_CURSO_REPITA`, **sin armar el latch** |
| `CANCELAR_AMBAR` | si | **PIDE PIN al reves que armar**, y es deliberado: quitar el ambar devuelve el cruce a dar verdes, o sea **abre paso**. Depende de `ambarEmergencia`: con latch puesto lo retira, avisa al Poste 1 y contesta `RESULT:RETIRADO` (la rama del mando salio con el, `D-30`). Sin latch, `RESULT:REENVIADO_AL_MAESTRO` si esta punta **sigue** en `S_FALLO` (la red de la trama perdida, `N-152`), y `$ERR ... NO_HAY_AMBAR_VIGENTE` si no |
| `SET_NOCHE:*` · `SET_NOCHE_TR:*` · `CONSULTA_NOCHE` (y ~~`SOLICITAR_PASO`~~ · ~~`SET_MODO:DEGRADADO`~~, que salieron con `D-46`; la app ya no las ofrecia, `D-36`, `D-37`) | — | Con PIN, `$ERR,CMD:DESCONOCIDO,DESC:COMANDO_NO_SOPORTADO_EN_ESCLAVO`; sin PIN (tambien `CONSULTA_NOCHE`), la puerta del PIN: `AUTH_FAILED`/`PIN_INVALIDO` (`js/noche.js` lo absorbe). Las ordenes de noche solo existen en el Maestro, que manda la franja por radio (`CMD_GO_NOCHE`, SPEC 0 §5.10 (b)); el paso se da desde el Maestro (SPEC 1 §8) y al Degradado se entra por el testigo |
| `SET_MODO:DEG_T:ahora,inicio,verde,despeje` | si | Construido (`990c278`; `D-35`, SPEC 2 §7.bis). Misma orden, `RechazoTestigo` propio: `DEG_RECHAZO_T_SIN_HORA` / `_AHORA_DESFASADO` / `_INICIO_VENCIDO` —texto «repita el testigo en el Maestro»— / `_DESPEJE_RANGO` / `_AMBAR_VIGENTE`, o `RESULT:OK` / `RESULT:RENOVADO` |
| `SET_MODO:DEG_FIN:ahora,salida` · `...:CANCELAR` · `CONSULTA_DEG_FIN` | si · si · no | Construidos el 04/10 (`a4545bb`, `D-52`), sin banco. Misma orden y mismos literales que el Maestro (§3.1), con `RechazoSalida` propio; aqui muerde «Salida ya vencida», con el MISMO literal que el Maestro (el consejo lo dice la app). Al llegar la hora llama a `iniciarSalida(false)`: rojo, el despeje y `DEG_INACTIVO`, **sin menu**: sin radio queda en ambar por silencio. `DEG_T` con salida programada se rechaza igual |
| `SET_DEG_AUTO:1` / `SET_DEG_AUTO:0` · `CONSULTA_DEG_AUTO` | si · no | Construidos. Los del Maestro (§3.1); aqui el enlace es una orden de gobierno del Maestro en los ultimos `SFTY6_SILENCIO_MS`, y el `ECO` llega en su `PING` |
| `FORZAR_ROJO` | las dos formas | `$ERR ... RENOMBRADO_USE_AMBAR_EMERGENCIA`. **Se rechaza ensenando el nombre bueno**, no en silencio: quien lo manda tiene una app o un manual anteriores al cambio |
| `TEST_LEDS` | si | `$ERR ... NO_EN_SERVICIO_USE_EL_MAESTRO`, **rechazado a proposito**: la secuencia enciende VERDE sin mirar nada, y ese verde saldria mientras el Maestro da paso al otro sentido |
| `REINICIOS` | no (es una consulta) | `D-60`, §9.4 (construido, sin banco): igual que en el Maestro, con `NODE:ESCLAVO` |

### 3.3 El puente ESP32 de cada poste — `ESP32_Expansion/src/despachador.cpp`

| orden | PIN | que contesta, y **de que depende** |
|---|---|---|
| `CMD:LEER_RTC` | no (es una consulta: no abre, no para, no cambia) | `$ACK,NODE:PUENTE,...,RESULT:OK` con `FECHA:` y `HORA:` **releidas del chip ahora mismo**, o **siete `$ERR` distintos**, uno por cada valor de `MotivoSinHora` —incluido `BARRERA_INCOHERENTE`, que existe para que un defecto del firmware no se disfrace de averia del modulo—, mas un octavo, `MOTIVO_NO_CONTEMPLADO`, para que nadie anada un valor al enum y deje la consulta muda—. Cada literal vive **dentro** de su rama |
| `SET_RTC:<fecha>,<hora>` | **el PIN viaja y no lo comprueba nadie** — ver §7 | siete motivos, uno por valor de `ResultadoReloj`, y el `$ACK` final **devuelve la hora RELEIDA**, no la que se mando. Hay un octavo caso que es `$ACK` y no `$ERR`: `RESULT:HORA_PUESTA_SIN_PROPAGAR` cuando el DS3231 quedo puesto pero `siembra_ahora()` no pudo poner la linea entera en el cable. El detalle es de SPEC 3 |
| cualquier linea con `HORA_ESP32` dentro | — | `$ERR,NODE:PUENTE,CMD:HORA_ESP32,DESC:LINEA_RESERVADA_AL_PUENTE`, **y va la primera de todas**: esa orden solo la origina el modulo, y el STM32 la acepta sin PIN porque el puente no lo conoce |
| `CMD:LOG,DESDE:<N>` | no (es una consulta: solo mira) | `$ACK,NODE:PUENTE,CMD:LOG,RESULT:OK,DESDE:,HASTA:,ULT:,PRIMERO:` y los contadores (`CONG` de `D-70`, `CRC`, `LARGO`, `ANILLO`, `FLASH`, `MALOS`), y detras hasta 16 `$EVENT,NODE:PUENTE,EVT:LOG` con el texto codificado; `$ERR` `FORMATO_INVALIDO` o `SIN_FLASH`. Se queda en el puente por prefijo (`D-15`). Detalle en §9.8 (`D-74`); construido, sin banco (`D-100`: no) |

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
   el mismo limite que el firmware exige (§3.1)— y el **verde** —«Verde (min)», enteros de 3 a 15, por defecto 3;
   viaja en segundos, minutos x 60 (`D-59`)—. Con el Poste 1 ya en Degradado y otro verde o despeje, antes del PIN
   sale UN solo aviso de paleteros con casilla (§9.9 (e), `D-81`; riesgo aceptado, SPEC 2 §7.bis); un reenvio sin
   acuse repite lo ya enviado, no los campos. Un `FORMATO_INVALIDO` con verde distinto de 180 se lee como firmware
   anterior a `D-59`.
   **`D-78` (construido en `js/testigo.js`, sin telefono):** la app no ve el ciclo que corre en el Poste 1; lo mide
   el acuse. El aviso previo compara con el testigo guardado y sale tambien si no hay guardado o es de otra `SERIE`;
   y si la orden salio con el Poste 1 ya en Degradado (`enCaliente`) y el acuse es `RESULT:OK` —entrada nueva, no
   `RENOVADO`—, la app avisa en rojo (evento y toast): «el POSTE 1 cambio de ciclo en marcha: el POSTE 2 sigue con el
   anterior hasta aceptar este testigo; mantenga el paletero en los dos extremos». Con `RENOVADO` no hay aviso.
2. La app manda `SET_RTC:<fecha del telefono>,<hora del telefono>` a ESTA punta, igual que siempre —**no bloquea
   el paso siguiente**: el resultado (`RESULT:OK` o `HORA_PUESTA_SIN_PROPAGAR`) se registra en el Diario, pero
   el testigo ya no depende de el (SPEC 2 §7.bis: la garantia la da `ahora`, no el `SET_RTC` previo).
3. La app LEE la hora del telefono OTRA VEZ, justo antes de enviar —es el `ahora` de la orden—, calcula
   `inicio = ahora + traslado` y manda `SET_MODO:DEG_T:ahora,inicio,<verde>,<despeje>`.
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
`FORZAR_ROJO|SALIENDO_TODO_ROJO` ya no existe en `ACK_TEXTO` (`js/textos_ack.js`).
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
`app.js` (`CONSULTA_DEG_FIN` en `SIN_PIN`), `js/textos_ack.js` (`ACK_TEXTO`). Copias en `www/` y en la APK.
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
| cabecera | `MAESTRO (POSTE 1)` + «(radio N %)» con `RF:` medido | `ESCLAVO (POSTE 2)` + «(enlazado)» — lo decide `NODE:`, no lo que el operario suponga. En las dos, «(sin radio)» (§9.9 (d), `D-82`; hoy «(sin radio entre postes)») con `FALLO COM` fuera de `MODO:AMBAR` o `RF:0%`, y «(sin enlace)» sin `$STATUS` |
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

**`D-53` (`N-176`), construido, sin banco: `ESTADO:ROJO+AMAR`**, 9 caracteres, el tope del campo
(`semaforo_nombreEstado()`, SPEC 1 §3.3 (9)), durante los `ROJO_AMARILLO_SEG` antes de cada verde, en las dos
puntas. Lo que la app HACE: **(1)** una fila `'ROJO+AMAR'` en `ESTADOS` de `app.js`, la unica con dos lamparas
(`lamparas: ['red', 'amber']`), texto «ROJO Y AMARILLO» y la frase de §9.9 (c) «Rojo y amarillo: va a abrir el
verde», sin la cifra: un número que la app no puede recalcular no se escribe en `app.js` (`CLAUDE.md` §14);
**(2)** la frase de la pluma la trata como `ROJO`: sube con el verde (SPEC 8 §1); **(3)** `ESC:` no cambia: dice
`ROJO` hasta el `CMD_ACK_GREEN` (SPEC 2 §2.2.ter);
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

🟢 **4 · CERRADO por `D-65` (§9.5; construido, sin banco): `SET_MODO:ALCANCE` y `SET_MODO:INTELIGENTE`.** Antes
contestaban `RESULT:OK` tras `modoActual_set()` **y nada mas**; entrar vive en `modoAlcance_setup()` y
`modoInteligente_setup()`, que `main.cpp` llama **solo bajo `if (modo != modoAnterior)`**: repetida la orden, OK sin
correr nada. Hoy esa repeticion contesta `RESULT:SIN_CAMBIO`. `AUTO` y `MANUAL` no tenian el defecto: llaman al
coordinador **dentro** de la rama. El par (modo puesto, luz movida por `FORZAR_ROJO`, que no toca `modoActual`) es
alcanzable; leido en `modo_inteligente.cpp`: con el coordinador en `C_IDLE` el modo sigue contando y pide cambio al
cumplir el suelo, asi que no deja el cruce trabado. Lectura del fuente, sin banco ni tarjeta.

⚠️ **5 · El Modo Alcance no tiene superficie.** Todo lo que ese modo produce sale por
`lcd_dibujarAlcance()`, y **el LCD se retiro** (`D-17.bis`). Desde el telefono, `MODO:ALCANCE` ensena el mismo
`RF:`/`RTT:` que cualquier otro modo —vienen de la ventana de latidos del coordinador, **no** de los contadores que ese
modo pone a cero—. Es un modo de medida sin nadie que lea la medida, y es `D-16` en su forma mas simple.

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

🟡 **10 · Los dos interruptores de administrador no existen; como se publican quedo DECIDIDO el 14/10 (`D-97`).**
*Sacar la camara del veto* y *sacar la barrera de servicio*, uno por poste, sobreviviendo a un corte de luz:
la conducta entera es SPEC 8 §6. El obstaculo medido sigue siendo cierto —al `$STATUS` del poste 1 le sobran
3 B de 155 y un campo nuevo no cabe—, y por eso **no entra campo ni byte**: el estado viaja como VALOR de dos
campos que ya tienen su ancho, `PLUMA:FUERA` (5 <= 6) y `CAM:NOVETO` (6 <= 6), en las dos puntas; el recorte
de `HORA:` se queda sin gastar. La app los pinta en ambar con su frase (`D-83`, `D-80`) desde `PLUMA_LEYENDA` y
`CAM_LEYENDA`, que `app_04_valores_de_status` compara con el C++ en los dos sentidos. Las ordenes van tras la
clave de administracion (`D-56`) y el PIN, locales al poste conectado (`D-94`). **Sin construir**: sin tarjeta
con barrera y pila medidas no hay codigo (SPEC 8 §6, `N-178` M12).

⚠️ **11 · Lo que 1.49 dejo a medias en esta spec** (medido sobre `622a20b`). **(a)** ~~`js/avisos_equipo.js` no
tiene entrada para ninguna de las tres causas del limite~~ → cerrado: las traduce (`tests/dom_textos_spec4.js`, caso
4; remedido el 14/10). Lo que faltaba era que salieran UNA sola vez: §9.6 y SPEC 6 C.1.
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

🟢 **13 · 14 · 15 · CERRADOS, construidos:** el testigo (§3.ter, `D-35`; `11b57b3`), el Degradado automatico
(§3.ter.bis, `A-15`; `a0d605b`, `1a78873`; `js/deg_auto.js`, `js/aviso_degradado.js`) y la salida programada con el
rojo total corregido (§3.ter.ter; `7cd3042`; `js/deg_fin.js`). Sin banco ni telefono.

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
| §3.3 **el despachador del ESP32** — `D-15`, `esParaElPuente()`, los siete `$ERR` de `LEER_RTC` | 🔴 **NADIE.** `ESP32_Expansion/src/despachador.cpp` **no lo compila ningun arnes**: solo lo cruza PlatformIO, y la fila 20 **lo modela en Python** (`ARQUITECTURA.map` §5) |
| §1 **`transporte_escribir()` mira `hasClient()`** y no espera con el SPP congestionado (§9.7) | 🟡 **texto** (`esp32_06_no_parte_tramas` bloque 5, 14/10); nadie compila `X:transporte_app.cpp` |
| §9.8 **el registro del puente** (`registro.cpp`, `CMD:LOG,DESDE:<N>`, `LOG:` en el parte) | 🟡 **texto** (`esp32_05` `EMITE_HACIA_LA_APP`, `esp32_10` el parte con `LOG:`, `decisiones_01` las anclas) y ✅ `tests/dom_registro_sesion.js` bloque 9.8 sobre la app real en jsdom; construido, sin banco (`D-100`: no). El C++ del puente sigue sin ejecutarse en el PC |
| §1 **`TRAMA_MAX_UTIL` y `BUF_ENTRADA_STM32`** | 🟡 **texto** (`esp32_06_no_parte_tramas`, `esp32_09_contrato_de_bytes`, `esp32_07_presupuesto_bytes`) |
| §1 **la rama del latido que sale sin contestar** | 🟡 **texto** (`esp32_08_silencio_no_es_orden`, y el censo de la fila 20, que la excluye por nombre) |
| §1 **`sellarHoraSiFaltaba()`**, la unica trama que el puente modifica | 🟡 **texto** (`esp32_13_siembra_de_hora`, `reloj_02_siembra_que_miente`); nadie compila `X:siembra.cpp` |
| §3.1 **las respuestas de `FORZAR_ROJO` en Degradado y las de `SET_MODO:DEG_FIN`** (`D-51` corregida, `D-52`) | 🔴 **Construido el 04/10 (`a4545bb`), sin banco.** Las respuestas las ejecuta el arnes del puente con `bluetooth.cpp` real (arquitecto 04/10, cond. 4); banco: SPEC 2 §7.quater (i) |
**Cuenta: 10 filas y 12 barreras — 7 ejecutadas, 1 sin nadie, 4 vigiladas solo por texto.** 🔴 **El rojo y los
textos del puente son el MISMO hecho: el ESP32 entero (9 `.cpp`) no se ejecuta en el PC en ningun sitio**, asi que todo
lo que esta spec dice del puente —`D-15`, los limites de linea, los siete motivos de `LEER_RTC`— descansa en packs que
leen texto. ⚠️ **Y una refutacion que conviene dejar escrita: `Maestro/src/bluetooth.cpp` SI lo compila alguien** —la
fila 20, junto con el del Esclavo—; lo que no compila nadie es el despachador del **ESP32**, que es otro fichero.

## 9. VERSION SIGUIENTE — decidida del 09/10 al 15/10; cada apartado dice si esta construido

Lo que el equipo debe hacer en la proxima version; lo construido lo dice su cabecera. Detalle y orden en
[`App_Semaforo/ROADMAP_APP.md`](App_Semaforo/ROADMAP_APP.md); las decisiones que lo abren, `D-55`..`D-58`.

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

### 9.2 Dos claves: la de administracion ensena, el PIN del equipo autoriza (`D-56`, `D-57`) — construido, sin telefono

Lo que HACE (contra el fuente, `js/clave_admin.js` y `setRole()`; lo ejerce `tests/dom_claves.js`). Dos estados
que no se mezclan: **rol visible** (clave de administracion de la app) y **autorizacion de ordenes** (PIN del
equipo, §4; `state.pinVerificado`, `CMD:PIN:`). Ninguno arma al otro.
- **Sin clave de administracion** el tecnico ve Trafico (estado, luces de los dos postes, ROJO TOTAL, AMBAR
  EMERGENCIA, la botonera), Eventos, exportar y **«Prueba de focos»**, que pasa a Trafico y sigue pidiendo el PIN.
- **Con la clave** se ven ademas Tiempos, Diagnostico (Degradado: testigo, eleccion del automatico, salida `D-52`;
  hora y Courier; mandos directos) y Depuracion, y la tarjeta que pide la eleccion del automatico al conectar.
  Quien hace Degradado lleva las dos claves (SPEC_6 A.1).
- **El PIN no cambia:** cada orden que lo pide abre el teclado del PIN, tambien con la clave puesta. Tiempos y las
  dos de hora no salen sin el: el emisor las frena (al diario como no salidas) y abre el teclado; tecleado, se
  vuelve a pulsar. FORZAR_ROJO y AMBAR_EMERGENCIA salen sin ninguna clave.
- **La clave**: 4 a 8 cifras; de fabrica `1234`; se guarda en `localStorage` como huella FNV-1a de 32 bits con sal,
  no en claro. El rol dura hasta cerrar la app: no se guarda y no lo baja la caducidad del PIN (§4); lo baja el
  boton de arriba. Con la clave puesta se cambia: la actual y despues la nueva.
- **Olvido** (`D-57`): «He olvidado la clave» pide la clave de soporte; si su huella coincide, pide una nueva. El
  valor de soporte no esta en el repo ni en los manuales: solo su huella. Con 4 cifras la huella no es secreta para
  quien lea el codigo: es barrera de uso, no seguridad del equipo, que sigue siendo el PIN.

### 9.3 El registro de la app no pierde la sesion (`D-55`, app; `ROADMAP_APP.md` 10) — construido, sin telefono

Por que (medido en `evidencia/061020261110/IOTVIAL_4D2007_20261006-110409.txt`): el diario empieza a las 10:03:43
y la cinta a las 10:54:54; se tiraron 1542 tramas por el tope de 300, y el diario ve al menos 63 cambios de `MODO`
o `ESTADO` anteriores a la cinta que en la cinta no estan (cota inferior: el diario solo ve los que caen en los 95 s
de una orden). El `Enlace_*.csv` sale por `<a download>`, que en la APK no hace nada (§7.12).

Lo que DEBE hacer, y HACE desde este cambio (lo ejerce `tests/dom_registro_sesion.js` sobre el codigo real):
1. **Cambios de `$STATUS` aparte de la cinta.** Cada `$STATUS` aceptado cuyo `MODO`, `ESTADO` o `ESC` difiere del
   anterior del mismo `NODE` se guarda en `RegistroCambios` (`js/registro_enlace.js`, tope 2000, en memoria; al
   pasarlo se cuenta lo tirado), con la trama entera y el cambio campo a campo. El primero de cada `NODE` es
   `INICIO`. La cinta sigue en 300.
2. **Copia persistente fuera de CACHE.** «Exportar todo» en la APK copia el `.txt` de CACHE a `Directory.External`
   (`Android/data/com.iotvial.semaforos/files/`, sin permisos, se saca con `adb pull`) antes de abrir la hoja de
   compartir; cancelar la hoja no borra la copia. Si la copia falla, rojo en la bitacora con el motivo; el envio sigue.
3. **Un solo `.txt`.** El `IOTVIAL_<serie>_<fecha>.txt` lleva, tras el diario y la cinta, dos secciones mas,
   separadas por la misma linea de 72 `=`: `IOT-VIAL - CAMBIOS DE ESTADO` y `IOT-VIAL - REGISTRO DEL ENLACE`, esta
   con las filas del CSV de siempre (`Fecha_ISO,Hora_local,Clase,Enlace_pct,RTT_ms,Detalle`). Exportar no dice
   «nada que exportar» si solo hay registro del enlace o cambios.
4. **En nativo, a fichero desde el principio.** Con `Filesystem.writeFile` y `appendFile`, la primera anotacion
   de la sesion abre `SESION_<AAAAMMDD-HHMMSS>.txt` en `Directory.External`, y cada trama de la cinta (las dos
   direcciones, el PIN tapado) y cada anotacion del enlace se le anaden por lotes (cada 5 s, 50 lineas o al pasar
   la app a segundo plano). Un fallo de escritura para el registro y se declara en el `.txt` exportado; a los
   20 MB para y lo dice (~64 h: 1830 tramas/h de ~170 B, medidas en la misma cinta). Fuera de la APK no escribe.

Medido el 14/10 en el telefono (`evidencia/2026-10-14_telefono_d55.txt`): `writeFile` y `appendFile` sobre `EXTERNAL`
funcionan, la ruta es la de arriba y adb la lee. Queda sin medir: `copy` al exportar; cuanto se pierde al matar la app
(hasta un lote, 5 s). Los `SESION_*` no se purgan: se borran a mano o por adb. El aviso EN VIVO de un fallo de escritura
necesita una linea en `app.js`.

### 9.4 Reinicios fechados y contador que baja (`N-178` M1; `D-60`..`D-64`) — construido, sin banco ni telefono

Por que (medido): el 06/10 a las 10:43:30 la app escribio «los contadores BAJARON (1789→91) ... SE HA REINICIADO»
(`evidencia/061020261110/Enlace_KM_12_EL_SISGA_2026-10-06.csv`, fila 346). **No hubo reinicio**: a las 10:43:00 la
misma cinta trae `SET_MODO_ALCANCE`, y `modoAlcance_setup()` llama a `protocolo_reiniciarContadores()` (SFTY-15). La
premisa de `ROADMAP_APP.md` 11 queda REFUTADA a medias: lo que faltaba no era fechar un reinicio, era poder decir que
NO lo hubo. El `$STATUS` no admite un campo mas: su comentario en `Maestro/src/bluetooth.cpp` mide margen 2 sobre
`tramaCompleta[160]` y `esp32_07_presupuesto_bytes` lo recalcula. **Nada de esto va en el `$STATUS`.**

Lo que el equipo HACE (las dos puntas, bloques gemelos letra por letra; leido en el fuente el 15/10):
1. **`REIN` en el periodico.** `$EVENT ... ORIGEN:ENLACE_RF,DETALLE:RX:<n> OK:<n> RUIDO:<n> REIN:<r>`; `r` son los
   arranques contados, byte alto de `DR3` (`REG_DESPEJE`) del respaldo, `uint8_t`, da la vuelta en 255: **la senal es
   que CAMBIA, no que sube.** Dentro de la suma y con la misma `FIRMA` (molde de `FLAG_TESTIGO`): un firmware anterior
   lo ignora; `respaldo_guardarCiclo()` conserva el byte alto; `respaldo_borrar()` lo deja en 0 (pila agotada o
   `REINICIAR_RELOJ`: el siguiente arranque cuenta 1, que es un cambio, y es lo correcto) y borra tambien `DR9`/`DR10`,
   los tiempos del ciclo automatico (`D-114`, sin banco; verbo `BORRAR` del arnes de respaldo, visto en rojo). Se
   publica la copia en RAM leida en `respaldo_setup()`. El `det` del periodico es `det[61]` en el Esclavo y `det[76]` en
   el Maestro (`AMBAR:`, `D-92`); el `payload` del `$EVENT`, `[128]` y `[144]`. La cuenta la hace `esp32_07`, no este
   documento. En BKP y no en `testigo_flash`: un bucle de watchdog escribe cada 4 s y la flash se gasta en 10.000 ciclos
   (`D-61`).
2. **La consulta `REINICIOS`**, sin PIN como `VERSION` (solo mira), en las dos puntas:
   `$ACK,CMD:REINICIOS,RESULT:OK,NODE:<MAESTRO|ESCLAVO>,REIN:<r>,CAUSA:<c>,UP:<s>,ANTES:<h>`. `UP`: segundos encendido,
   contador propio de 32 bits (no `millis()/1000`, que da la vuelta a los 49 dias). `ANTES`: horas que llevaba
   encendido la vida anterior, byte alto de `DR2` (`REG_VERDE`), escrito cada hora, satura en 255 (mas de 10 dias);
   0 si murio antes de la primera hora. `CAUSA` sale de `RCC_CSR` leido como PRIMERA linea de `setup()`, antes de
   `IWatchdog.begin()`, y se limpia (`__HAL_RCC_CLEAR_RESET_FLAGS`): `WATCHDOG` (IWDG), `SOFTWARE` (SFT), `CORTE`
   (POR), `PIN` (solo NRST: carga por SWD o pulsador), `OTRA` (WWDG, LPWR o ninguno). Ese ORDEN de prioridad, porque
   en el F1 un POR y un IWDG levantan tambien el flag de PIN (`D-62`). Buffer `char[96]`: 89 caracteres en el peor
   caso, con las cotas de tipo. **La fecha del ultimo arranque la pone la APP**, `hora del telefono - UP`; el equipo
   no la fecha (SPEC_7: puede no tener hora; el telefono siempre la tiene). `HEAP_MIN` no existe: el firmware STM32
   no usa heap (`grep "String|malloc|new "` en `*/src`: 0 lineas); la RAM se mide con `nm` (`CLAUDE.md` §10).
3. **SFTY-15 deja de poner a cero los contadores al entrar en PRUEBA ALCANCE** (`D-63`): `modoAlcance_setup()` no
   llama a `protocolo_reiniciarContadores()`; la funcion queda declarada y pasa a la lista de huerfanas del Maestro en
   `costura_10` (ya estaba en la del Esclavo). Su unico lector era el LCD (`D-44`); la app mide por ventanas de 30 s
   (`D-23`). Desde aqui «un contador baja» vuelve a significar lo que la app ya afirmaba. Modo de fallo buscado
   (`CLAUDE.md` §11.7): ninguno sobre luces; una sesion de antenas (SPEC_6) compara deltas, no absolutos.
4. **La app** (`D-64`; `js/diagnostico_enlace.js`, copia identica en `www/js/`; en `app.js` solo `REINICIOS` en
   `SIN_PIN`, la consulta automatica y el enganche del acuse con el molde `DegAuto.acuse(data)`):
   - `REIN` presente y distinto del anterior del mismo `NODE` → rojo «SE HA REINICIADO (REIN a→b)» aunque ningun
     contador baje, y pide `REINICIOS` sola. El primer periodico con `REIN` de la sesion tambien la pide, una vez.
   - `REIN` igual y un contador baja → rojo «BAJARON SIN REINICIO (REIN:r sigue): o el contador dio la vuelta en
     4294967295 o es un defecto del firmware; reportelo con esta linea». Es la alarma de `ROADMAP_APP.md` 11.
   - sin `REIN` (firmware anterior, como `96c3d32` en campo) y un contador baja → rojo «o se reinicio o entro en
     PRUEBA ALCANCE: este firmware no publica REIN». La app deja de afirmar un reinicio que no midio.
   - acuse de `REINICIOS` → una linea: hora del ultimo arranque (telefono menos `UP`) y hace cuanto; la causa en
     palabras (`WATCHDOG`: el vigilante lo reinicio, el firmware se quedo colgado, SFTY-1 · `CORTE`: se fue la luz o
     se puso la pila · `SOFTWARE`: reinicio pedido por el firmware · `PIN`: pata NRST, carga o pulsador · `OTRA`);
     horas de la vida anterior («mas de 10 dias» si 255) y arranques contados. `$ERR,CMD:DESCONOCIDO` → «este
     firmware no lleva REINICIOS», como hace `deg_auto.js`.

Quien lo ejerce: `test_unitarios_app.js` suite 8 (D-23) con los cuatro casos de arriba y el existente «un contador que
BAJA se lee como reinicio» INVERTIDO (sin `REIN` no se afirma); `esp32_07_presupuesto_bytes` (buffers y peor segundo);
`app_01_comandos` (`REINICIOS` atendido en las dos puntas); `costura_10` (la huerfana nueva del Maestro);
`maestro_02_respaldo` (gemelos y suma). **HUECO hasta banco:** si en la tarjeta `CAUSA` sale siempre `PIN` u `OTRA`,
el core de Arduino limpia `RCC_CSR` antes de `setup()` y hay que leerlo donde el core lo lea. Sin telefono.

### 9.5 El `$ACK` que no cambia nada (`N-178` M2; `D-65`, `D-66`) — construido, sin banco ni telefono

Por que (medido sobre el fuente): `ROADMAP_APP.md` 12 pide «todo `$ACK` lleva `RES`/`APLICA` o es `$ERR`; la orden
que no cambia nada da `MOTIVO:SIN_CAMBIO`». Aqui el campo es `RESULT:` y ya lo lleva todo `$ACK` (`grep '"\$ACK'` sin
`RESULT` en `Maestro/src`, `Esclavo/src` y `ESP32_Expansion/src`: 0 lineas); `MOTIVO:` no existe como campo y `DESC:`
es el del `$ERR`. Lo que queda a medias es el hueco 4 de §7: `SET_MODO:ALCANCE` y `SET_MODO:INTELIGENTE` contestan
`OK` con el modo ya puesto y no corre nada. Es el mismo «si» que `N-146` destapo en el ambar: el operario repite la
orden creyendo que no le hacen caso.

Lo que el equipo HACE (`D-65`):
1. **Maestro, `procesarComando()`, ramas `SET_MODO:ALCANCE` y `SET_MODO:INTELIGENTE`:** `yaEnModo` leido con
   `modoActual_get()` ANTES de `modoActual_set()` (molde `N-146`). En Degradado sigue el `$ERR`; con `yaEnModo`,
   `RESULT:SIN_CAMBIO` y NO se llama a nada (ni `modoActual_set()`, ni el setup, ni `bluetooth_reportarEvento()`:
   repetir una orden no es un evento del cruce); si no, `RESULT:OK` como hoy. Es `$ACK` y no `$ERR` porque el equipo
   esta en el estado pedido; lo que se niega es que haya corrido algo. **Sin re-armar**: un `REARMADO` como el del
   ambar volveria a correr `coordinador_iniciarModo()` (todo-rojo y ciclo desde cero) por una pulsacion repetida; la
   maquina no corre un todo-rojo que nadie pidio (`CLAUDE.md` §2). Quien quiera reiniciar el ciclo pasa por `MENU`.
2. **La app** (`ACK_TEXTO`, `textos_ack.js`; `D-66`): `SET_MODO:ALCANCE|SIN_CAMBIO` y `SET_MODO:INTELIGENTE|SIN_CAMBIO`
   dicen «ya estaba en <modo>: no se movio nada» y el camino para reiniciar (MENU y de nuevo el modo), tono ambar, no
   verde. Las dos entradas `|OK` pasan de `js/textos_spec4.js` a `ACK_TEXTO`, porque `app_10` solo censa esa tabla
   (y `DegAuto.ACUSE`) y con dos `RESULT` por orden exige las dos entradas alli; en `textos_spec4.js` se borran.
   El veredicto `SIN_CAMBIO` del diario (`js/depuracion.js`, dos `$STATUS` iguales) es otra medida y no se toca.

Lo que NO se hace, y por que: `SET_MODO:AUTO`, `MANUAL` y `MENU` repetidos vuelven a correr el todo-rojo DENTRO de
la rama (su `OK` es verdad y tiene efecto); `SET_TIEMPOS` con los mismos valores reescribe el respaldo y su `OK`
dice que esos son los tiempos; `SET_DEG_AUTO` repetido contesta el estado resultante (`ON_EFECTIVO`/`OFF`); el
Esclavo y el puente no tienen ningun `$ACK` incondicional (§3.2, §3.3). Modo de fallo buscado (`CLAUDE.md` §11.7):
ninguno sobre luces; la segunda orden ya no hacia nada, solo deja de decir que si. Un `SIN_CAMBIO` donde el
operario esperaba reiniciar el ciclo se cubre con el texto de la app.

Quien lo ejerce (instrumentos existentes; simulador congelado, `CLAUDE.md` §4):
- `Simulaciones/simulador_puente_esp32.py`, escenario N4 (compila el `bluetooth.cpp` REAL del Maestro): `ALCANCE`
  dos veces, `OK` y despues `SIN_CAMBIO` con `MODO` igual; `INTELIGENTE` (con PIN) dos veces, igual; control:
  `MENU` dos veces, las dos `OK`. Rojo antes de construir: la segunda contestaba `OK`.
- `banco/packs/app_10_ack_con_varios_sies.py`, sin tocar: rojo si el C++ tiene los dos `RESULT` y
  `ACK_TEXTO` no los nombra; verde con las cuatro entradas y textos distintos.
- `tests/dom_textos_spec4.js` (6 y 14): los dos `|SIN_CAMBIO` entran por el parser real, sin generico, con «ya
  estaba» y «MENU»; la asercion de `ALCANCE|OK` conserva su texto y cita este apartado en vez del hueco 4.
- `decisiones_01_anclas`, sin tocar: `D-65` anclada en `Maestro/src/bluetooth.cpp` (rama `SET_MODO:ALCANCE`).

### 9.6 Alarmas que se ven sin nadie conectado (`N-178` M3; `D-67`..`D-69`) — construido, sin banco

El comportamiento es de SPEC 6 C.1 y alli esta escrito: las dos alarmas del limite del Maestro (`LIMITE_48H`,
`SYNC_SIN_FECHA`) se repiten cada `AVISO_ROJO_SIN_HORA_MS` mientras el poste siga rendido, y el Esclavo publica y
repite `LIMITE_48H` en `DEG_RENDIDO`. **La app no cambia de codigo**: `AvisosEquipo.ALARMA` ya traduce las tres
causas (hueco 11 (a), cerrado) y la tabla no filtra por `NODE`. No hay consulta `ALARMAS` ni `FIN,<n>` (`D-69`):
la app no pide nada al conectar; lo que ve es la repeticion, al minuto como mucho. Prueba de la app:
`tests/dom_textos_spec4.js` caso 4 con `NODE:ESCLAVO` para `LIMITE_48H` (guarda: verde ya).

### 9.7 SPP del puente que no bloquea (`N-178` M4; `D-70`, `D-71`) — decidido y construido el 14/10, sin banco

Por que (medido sobre el fuente y las cintas): `transporte_escribir()` (`ESP32_Expansion/src/transporte_app.cpp`)
llama a `spp.write()` en cuanto hay telefono (`hasClient()`) y detras no hay buffer propio: `esp32_07` P-2 (refutada el
13/09) ya midio que «NINGUN buffer de este firmware acumula una rafaga» y que la cola es la de `BluetoothSerial`. Llena,
se lee en la libreria y no esta medido en tarjeta (`BluetoothSerial.cpp` 3.20017: `write()` -> `xQueueSend(...,
SPP_TX_QUEUE_TIMEOUT 1000)`, cola de 32: hasta 1000 ticks por trama si el telefono no lee); con dos, `loop()` no vuelve,
el perro (`ESP32_WDT_MS` 2000, `panic=true`) reinicia el puente y tira la sesion y la siembra de hora (SPEC 3 §2). La
cabecera `transporte_app.h` promete «menos de n significa buffer lleno, y el que llama tiene que contarlo» y nadie lo
cuenta: `puente.cpp` solo suma `tramasAlaApp` cuando entra entero. Cintas: 28 `EVT:ARRANQUE,CAUSA:OTRO_PERRO` del puente
(`evidencia/011020260934/IOTVIAL_38EB53_20261001-073243.txt` 07:12:38, 07:12:46, 07:24:57, 07:29:41 y 07:32:14;
`051020261239/Enlace_KM_12_EL_SISGA_2026-10-05.csv`, 9). El parte solo sale al conectar (`vigilante_declarar()`), asi
que la cinta no dice CUANDO se reinicio, y todos traen `ARRANQUES:1` (la cuenta RTC no sobrevive a ese perro; fuera de
M4). Que el `write` bloqueado sea la causa NO esta medido: M4 retira ese candidato y deja el contador que lo diria. La
guarda vale en los dos casos: si la libreria no bloquea, sobra la espera y falta el contador; si bloquea, falta todo.

Lo que el equipo HACE (`D-70`, `D-71`; solo `ESP32_Expansion`, fuera del camino de las luces, SPEC 0 §1):
1. **`transporte_app.cpp` registra con `spp.register_callback()`** un observador de eventos SPP (`esp_spp_api.h`)
   que lleva `volatile bool congestionado`: `true` con `ESP_SPP_CONG_EVT` (`param->cong.cong`) o `ESP_SPP_WRITE_EVT`
   (`param->write.cong`) en alto; `false` cuando bajan y en `ESP_SPP_CLOSE_EVT` / `ESP_SPP_SRV_OPEN_EVT` (sesion nueva,
   sin deuda). No toca tramas ni decide nada mas: es un observador, como `transporte_aprenderRotulo()`.
2. **`transporte_escribir()`:** sin telefono, `0` como hoy (no es descarte: no habia a quien dar). Con telefono y
   `congestionado`, NO llama a `spp.write()`: suma `descartadasCongestion`, devuelve `0` y vuelve en el acto. Con
   telefono y sin congestion, `spp.write()` como hoy. `puente.cpp` no cambia: sigue contando solo lo entregado entero.
3. **Sin espera (`D-71`):** ni `delay()`, ni bucle sobre `millis()`, ni `vigilante_alimentar()` dentro (W-3). El
   «hasta 1 s» de `ROADMAP_APP.md` 13 no cabe: `loop()` es UNA tarea que sirve `J17` a 960 B/s y alimenta al perro
   una vez por vuelta con 2000 ms de techo; y esperar no entrega nada: `CONG` significa que la pila y el controlador
   ya absorbieron la rafaga y el telefono dejo de leer. La congestion de milisegundos de un telefono que SI lee no
   llega a `CONG`.
4. **El contador se expone como los otros dos** (`transporte_descartadasPorCongestion()` en `transporte_app.h`, B-7)
   y se publica con el registro del puente de M5 (`D-55`, `LOG,DESDE:<N>`): no en el parte de arranque (`esp32_10`
   mide `VIGILANTE_PARTE_MAX` 144 contra 132 y un campo mas lo rompe) ni en el `$STATUS` (§9.4). Hasta M5 es legible y
   nadie lo publica, igual que `puente_descartadasPorCrc()`; se escribe aqui para que no se lea como hecho.
5. **`transporte_app.h`:** la frase «menos de n significa buffer lleno» pasa a decir lo que hace: «0 con telefono
   conectado = descartada por congestion y contada». Anclas `D-70` y `D-71` en `transporte_app.cpp`.

Lo que NO se hace, y por que: no se toca `BUF_*` de `contrato.h` ni se anade una cola propia (una cola delante de la
cola de la libreria solo retrasa el mismo bloqueo); no se reintenta la trama descartada (`$STATUS` sale cada segundo y
los `$ALARM` se repiten, `D-67`/`D-38`); no se avisa a la app (no lee). Modo de fallo buscado (`CLAUDE.md` §11.7): la
trama que se pierde puede ser un `$ACK` o un `$EVENT` del STM32. Repetida la orden sin acuse: `ALCANCE`/`INTELIGENTE`
dan `SIN_CAMBIO` (§9.5); `AUTO`/`MANUAL`/`MENU` repiten el todo-rojo (seguro); `DEG_T` da `RENOVADO`; `FORZAR_ROJO`
sigue en rojo; `SET_TIEMPOS` y `SET_RTC` reescriben lo mismo. Ninguna orden se vuelve peligrosa al repetirse, y el
`$EVENT` perdido es justo lo que el contador cuenta. La alternativa (bloquear) es un reinicio del puente con la sesion
y la siembra tiradas.

Quien lo ejerce (sin arnes nuevo, simulador congelado, `CLAUDE.md` §4; el ESP32 no se ejecuta en el PC: la fila
`transporte_escribir()` de §8 ya es 🟡 texto):
- `banco/packs/esp32_06_no_parte_tramas.py`, bloque 5 (B-7, el pack que dice «lo descartado se cuenta»): abre
  `("ESP32_Expansion", "src", "transporte_app.cpp")` y `("ESP32_Expansion", "include", "transporte_app.h")` (tuplas
  nuevas, `CLAUDE.md` §5) y verifica (a) `register_callback(` presente y un manejador que nombra `ESP_SPP_CONG_EVT` y
  `ESP_SPP_WRITE_EVT`; (b) en el cuerpo de `transporte_escribir()` la lectura de `congestionado` precede a
  `spp.write(` y hay `descartadasCongestion++`; (c) ese cuerpo no contiene `delay(`, `vTaskDelay(`, `millis()` ni
  `vigilante_alimentar(`; (d) `transporte_descartadasPorCongestion` en la cabecera. **Rojo antes de construir:**
  (a), (b) y (d) caian (`spp.write()` directo, sin callback ni contador); (c) ya era verde: guarda de `D-71`. Cuatro
  comprobaciones mas en `esp32_06`; `esp32_07` P-2 y `esp32_10` no cambian de forma.
- `decisiones_01_anclas`: `D-70` y `D-71` anclados en `transporte_app.cpp`.
- compila `esp32` (compuerta, fila 5): verde con el callback y `#include <esp_spp_api.h>`; flash en la acta.
HUECO hasta banco: que `CONG` llegue con un telefono que deja de leer solo se ve en la tarjeta (app en segundo plano
cinco minutos, cinta despues: el contador por el LOG de M5, o un parte sin `OTRO_PERRO`). Medida de §8: unas 25 lineas
de pack contra 0 en el numerador (`ESP32_Expansion` no cuenta en el): la razon instrumentos/producto sube por eso, y
se declara en vez de esconderse; el arnes que `ROADMAP_APP.md` 13 pedia es mas grande y esta prohibido (§4).

### 9.8 Registro en el puente (`N-178` M5; `D-55`, `D-72`..`D-75`) — construido, sin banco; `D-100` (15/10), no

Por que (medido): §9.3 guarda en el telefono lo que la app VE; lo que pasa sin telefono no lo guarda nadie: el parte
de arranque del puente solo sale al conectar (`vigilante_declarar()`), los `$ALARM` del limite se repiten mientras
duran (`D-67`) pero su primera aparicion y su hora se pierden, y los contadores de descarte del puente
(`puente_descartadasPorCrc()`, `puente_descartadasPorLargo()`, `transporte_descartadasPorCongestion()` de `D-70`)
son legibles y nadie los publica. El STM32 no tiene sitio (`CLAUDE.md` §10); el puente va al 35,7 % de 3 MB (acta
`2026-10-14`) y `huge_app.csv` del framework 3.20017 le da una particion `spiffs` de 0xE0000 B (896 KB) que ningun
fichero del proyecto usa (`grep "SPIFFS|LittleFS|esp_partition" ESP32_Expansion/src`: 0 lineas).

Lo que el equipo HACE (`D-72`..`D-74`; solo `ESP32_Expansion`, fuera del camino de las luces, SPEC 0 §1):
1. **Donde (`D-72`).** `ESP32_Expansion/src/registro.cpp` + `include/registro.h`, nuevos. La particion `spiffs` se usa
   EN CRUDO (`esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, NULL)`), sin
   sistema de ficheros: FIFO circular de registros de `REGISTRO_REG_B` = 128 B, 32 por sector de 4 KB, 224 sectores,
   7168 registros. Ni NVS (un bucle de perro la comeria en dias, `vigilante.h`) ni SPIFFS/LittleFS (su recoleccion de
   basura no tiene cota, y `loop()` alimenta al perro con `ESP32_WDT_MS` 2000 de techo).
2. **El registro (128 B; constantes en `contrato.h`).** `seq` u32 (desde 1, monotono entre arranques; `0xFFFFFFFF` =
   hueco) · `up_ms` u32 (`millis()`) · `fecha` u8[6] (AA MM DD hh mm ss de `reloj_leer()`; todo 0 si
   `!reloj_enHora()`) · `clase` u8 · `len` u8 · `texto[110]` (`REGISTRO_TEXTO_MAX`; lo que no cabe se recorta y el
   ultimo caracter pasa a `~`) · `crc16` u16 (CRC-16/CCITT-FALSE, polinomio 0x1021, inicio 0xFFFF, sobre los 126 bytes
   anteriores). Al leer, un registro con `seq` valido y CRC malo se salta y se cuenta (`MALOS`).
3. **Que se anota (`D-73`).** Desde `desdeElEquipo()` de `puente.cpp`, DESPUES de `trama_valida()` y de
   `sellarHoraSiFaltaba()` y ANTES de `transporte_escribir()`, con o sin telefono. Tres clases: `E`, toda trama valida
   de subida que no empiece por `$STATUS` (`$EVENT`, `$ALARM`, `$ACK`, `$ERR` del STM32) salvo las que contengan
   `REGISTRO_EXCLUIR` = `ORIGEN:ENLACE_RF` (el periodico de `DIAG_ENLACE_MS` 30 s: 120/h, dos dias de registro con
   telemetria que la app ya mide por ventanas, `D-23`) · `S`, el `$STATUS` cuyo `MODO:` o `ESTADO:` difiere del
   anterior (molde `RegistroCambios.CAMPOS` de la app, sin `ESC`; `trama_campo()` sale de `trama.cpp`: el `campo()`
   estatico de `transporte_app.cpp` se muda alli y no se duplica) · `P`, el parte de arranque, anotado en `setup()`
   tras `registro_setup()`. Se guarda el payload sin `$` y sin `*XX`. NADA de bajada: ahi viaja el PIN, y la cinta del
   telefono ya la tiene (§9.3).
4. **Anillo en RAM y volcado acotado.** `registro_anotar()` solo escribe en un anillo de `REGISTRO_ANILLO` = 8
   registros en RAM (1 KB) y vuelve. `registro_volcar()`, desde `loop()` detras de `puente_bombear()`, escribe COMO
   MAXIMO UN registro por vuelta (`esp_partition_write()`); si ese registro estrena sector, antes lo borra
   (`esp_partition_erase_range()`, 4 KB: pisa los 32 mas viejos). Anillo lleno (ocho anotaciones entre dos vueltas): se
   pisa la mas antigua y se cuenta (`ANILLO`). Escritura fallida: se cuenta (`FLASH`) y el registro se tira, sin
   reintento y SIN consumir `seq` ni ranura: `ULT` y `PRIMERO` solo nombran lo escrito (`D-100`, construido sin banco:
   `registro_volcar()` consume `seq` y ranura solo tras `ESP_OK` de borrado y escritura; `D-114`, construido sin banco
   ni arnes que ejecute el ESP32: tras borrar el sector mas viejo, `PRIMERO` pasa a la cabeza valida del siguiente o a
   0, y toma el `seq` nuevo solo cuando la escritura devolvio `ESP_OK`, no antes). Nada de esto llama a
   `delay()`, `millis()` en bucle ni `vigilante_alimentar()` (`D-71`). **Cuanto dura (calculo, no medido):** 7168
   registros; cada `$ALARM` repetida (`D-67`, cada `AVISO_ROJO_SIN_HORA_MS` = 60 s) gasta 60 por hora: con una, ~5 dias;
   con dos, ~2,5; sin ninguna, meses (solo cambios de `MODO`/`ESTADO` y eventos).
5. **Arranque.** `registro_setup()` va en `setup()` antes de `transporte_abrir()` y NO emite nada (6.4, `esp32_08`):
   lee el `seq` del primer registro de cada sector (224 lecturas de 4 B); el sector vivo es el de `seq` valido mayor,
   el primer hueco dentro es la posicion de escritura y el `seq` anterior es `ULT`; `PRIMERO` es el menor entre las
   cabezas validas; sin ninguna (flash virgen o borrada), sector 0 y `ULT` 0. Sin particion devuelve `false`: no se
   anota nada y la consulta contesta `SIN_FLASH`.
6. **El parte de arranque dice cuanto hay.** `FORMATO_PARTE` gana `,LOG:%lu` (= `ULT`) como ULTIMO campo: `esp32_10`
   recalcula el peor caso, 127 + 5 + 10 = 142 contra `VIGILANTE_PARTE_MAX` 144; la constante no se toca. Es la unica
   puerta por la que la app sabe que este puente lleva registro (punto 8).
7. **La consulta (`D-74`).** Linea `CMD:LOG,DESDE:<N>` (literal de `D-55`), sin PIN (solo mira, como `LEER_RTC` y
   `REINICIOS`). Se la queda el puente (`despachador_esParaElPuente()`, `D-15`) con `static const char CMD_LOG_PREFIJO[]
   = "CMD:LOG,DESDE:"` y `strncmp` de prefijo, NUNCA una ayudante con `strstr`: el simulador del puente lee los `strstr`
   de las ayudantes del predicado como criterios «contiene» y los modela como siembra. `despachador_atender()` delega en
   `registro_atender()`, que contesta, cada `$ERR` dentro de su `if` (molde `SET_TIEMPOS`, `CLAUDE.md` §2):
   - `$ACK,NODE:PUENTE,CMD:LOG,RESULT:OK,DESDE:<a>,HASTA:<b>,ULT:<u>,PRIMERO:<p>,CONG:<x>,CRC:<y>,LARGO:<z>,ANILLO:<w>,`
     `FLASH:<f>,MALOS:<m>`: `a` = max(N, `PRIMERO`) (N = 0 vale 1); `b` = min(a + 15, `ULT`) (`REGISTRO_LOTE_MAX` 16:
     16 x ~170 B = 2,7 KB, la mitad de la cola de 32 paquetes de `BluetoothSerial`); sin nada que dar (a > `ULT`),
     `HASTA:0`. `CONG` es el contador de `D-70`: aqui y solo aqui.
   - detras, un `$EVENT,NODE:PUENTE,EVT:LOG,N:<seq>,UP:<s>,FECHA:<AAAA-MM-DD|->,HORA:<HH:MM:SS|->,C:<E|S|P>,D:<texto>`
     por registro, leido de la flash (no del anillo: una sola verdad; lo pendiente sale en la consulta siguiente). En
     `D:` el texto va con `,` -> `;` y `:` -> `=`: un `$ALARM` guardado que viajara tal cual pisaria `NODE:` y `EVT:` de
     la trama que lo lleva y la app lo leeria como alarma viva. La app deshace la sustitucion al mostrarlo.
   - `$ERR,NODE:PUENTE,CMD:LOG,DESC:FORMATO_INVALIDO` si `DESDE:` no es un entero · `$ERR,NODE:PUENTE,CMD:LOG,DESC:`
     `SIN_FLASH` si `registro_setup()` fallo.
   Ninguna linea de la consulta cruza a `J17`, y la respuesta no se anota (sale por `puente_emitirPropio()`, no entra
   por `desdeElEquipo()`). El `$EVENT` mas largo mide ~56 + 110 = 166 B < `BUF_SALIDA_APP` 512: `BUF_*` no cambian.
8. **La app (`D-75`).** `js/registro_equipo.js` nuevo (copia identica en `www/js/`), `<script>` en `index.html` y
   `www/index.html`, la seccion en `exportar.js`; en `app.js` (y `www/app.js`), neto <= 0 lineas: `'LOG,DESDE'` en
   `SIN_PIN`, un enganche en las ramas `$EVENT`, `$STATUS`, `$ACK` y `$ERR`, y `SIN_FLASH` en la tabla de `$ERR`.
   - **Pide SOLO si el parte trae `LOG:`.** Con un puente anterior la linea cruzaria a `J17` y el STM32 contestaria
     `$ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO` (§1): un aviso rojo por una clave que nadie tecleo.
     `RegistroEquipo.verParte(data)` guarda `ULT` y arma la peticion; sale con el primer `$STATUS` con `SERIE` (molde
     `REINICIOS`, `D-64`): `enviarComandoFirmware('LOG,DESDE', String(n + 1))` -> `CMD:LOG,DESDE:<n+1>`.
   - **Recuerda el ultimo N RECIBIDO por `SERIE`** (`localStorage`, clave `iotvial_log_equipo_v1:<SERIE>`; sin
     almacen, desde 0 cada sesion). `D-100`: el puente emite en orden y solo lo que tiene, y lo que falta no vuelve
     por pedirlo otra vez. `N > recordado` -> recordado = N; si `N > recordado + 1` se anota una vez por conexion
     («hueco en el registro del equipo: faltan a..b») y avanza igual; `N <= recordado` no lo mueve. `PRIMERO` >
     recordado + 1 en el `$ACK`: «el equipo ya no tiene las entradas a..b» y recordado = `PRIMERO` - 1 (sin hueco
     despues). `ULT` < recordado (particion borrada o puente cambiado): recordado = 0 y vuelve a pedir una vez.
     Una sesion que llega a `ULT` deja recordado = `ULT`, y la conexion siguiente no vuelve a bajar nada.
   - **Lotes.** Al llegar el registro `N == HASTA`, si `HASTA < ULT` pide `DESDE:HASTA+1`; tope `LOTES_MAX` = 64 por
     conexion (1024 registros, ~8 s de SPP); agotado, anota «quedan k entradas en el equipo: reconecte». Sin
     temporizador: un lote que no llega entero sigue en la conexion siguiente desde el ultimo RECIBIDO (`D-100`).
   - **Que se ve.** Por registro, una linea gris en Eventos: `Equipo [LOG] N:<n> <FECHA HORA | +UP s, sin hora>
     <E|S|P>: <texto decodificado>`; se guarda en `RegistroEquipo` (memoria, tope 4096) y sale en la seccion nueva
     del `.txt`, `IOT-VIAL - REGISTRO DEL EQUIPO`, detras de `CAMBIOS DE ESTADO` en `Exportar.textoAnexos()`, con los
     contadores del ultimo `$ACK` en su cabecera. NUNCA entra en `AvisoCorte`, `AvisosEquipo`, `AvisoCamaraPluma`,
     `DiagnosticoEnlace` ni en el diario de ordenes: es pasado, no estado.
   - `$ERR CMD:LOG`: `SIN_FLASH` «el puente no encuentra su particion de registro: reportelo»; `FORMATO_INVALIDO`
     «defecto de la app: reportelo con esta linea». Un `DESCONOCIDO` o `AUTH_FAILED` tras un `LOG` es rojo de defecto.

Lo que NO se hace, y por que: no se anota la bajada (PIN); no hay `LOG,BORRAR` ni orden que escriba (la FIFO se pisa
sola, y una orden de borrado es una barrera menos para quien quiera tapar un hecho); no se reintenta una escritura
fallida; no se compacta ni se busca por fecha; no se envia el registro solo (el puente no origina, `esp32_05`: cada
lote responde a una linea). Modo de fallo buscado (`CLAUDE.md` §11.7), ninguna barrera retirada: (a) el borrado de
sector (45 ms tipicos, 400 ms de hoja) para la cache de flash y `CONFIG_UART_ISR_IN_IRAM` no esta puesto en el
framework 3.20017: la UART de `J17` solo retiene su FIFO de 128 B (133 ms a 9600 bps); una trama de subida partida
cae en `trama_valida()` y se cuenta en `CRC`, una vez cada 32 registros como mucho; el `$STATUS` vuelve al segundo y
los `$ALARM` se repiten (`D-67`); la bajada no se pierde (la pila BT la retiene) y el latido de 2000 ms no se ve
afectado. (b) Un lote con el telefono sin leer: `D-70` descarta y cuenta; el lote se repite en la conexion siguiente.
(c) El parte crece a 142: `esp32_10`. (d) `seq` no da la vuelta (2^32: 136 anos a 1/s). (e) El escaneo de arranque
anade 224 lecturas antes de `transporte_abrir()`: dentro de `ESP32_ARRANQUE_MS` 1500 solo por calculo (AB-3 sigue
sin medir). (f) Puente nuevo con app anterior: la app ignora `LOG:` y nunca pide; nada cambia.

Quien lo ejerce (sin arnes nuevo, simulador congelado, `CLAUDE.md` §4; el ESP32 no se ejecuta en el PC):
- `tests/dom_registro_sesion.js`, bloque nuevo «9.8», sobre el codigo real en jsdom (`a.tramas` es lo escrito al
  Bluetooth): (1) parte SIN `LOG:` y `$STATUS` -> `a.tramas` sin `CMD:LOG`; (2) parte con `LOG:7` y `$STATUS` con
  `SERIE` -> exactamente `CMD:LOG,DESDE:1\r\n`; (3) `$ACK ... DESDE:1,HASTA:7,ULT:7,PRIMERO:1,CONG:3,...` y siete
  `EVT:LOG` (uno con `D:ALARM;NODE=MAESTRO;...`) -> siete lineas en Eventos con el texto decodificado
  (`ALARM,NODE:MAESTRO,...`), ningun aviso de alarma ni de corte, y `localStorage` con 7 para esa `SERIE`; (4) parte
  con `LOG:9` -> pide `DESDE:8`; (5) `$ACK` con `ULT:3` y 7 recordado -> recordado 0 y `DESDE:1`; (6) el `.txt`
  exportado trae `REGISTRO DEL EQUIPO` detras de `CAMBIOS DE ESTADO` con `CONG:3`; (7) `D-100`: `$ACK` con
  `PRIMERO:5,ULT:40` y 0 recordado, los 36 `EVT:LOG` -> recordado 40, un solo aviso de «ya no tiene», y en la sesion
  siguiente con `LOG:40` ningun `CMD:LOG` (la `t_registro.js` de QA, en rojo sobre la candidata del 15/10).
- `decisiones_01_anclas`: `D-72`, `D-73` y `D-74` anclados en `registro.cpp` (`D-74` tambien en `despachador.cpp`);
  rojo hasta que `D-100` lo este.
- `esp32_05_no_origina`: filas nuevas en `EMITE_HACIA_LA_APP` para `$ACK,NODE:PUENTE,CMD:LOG,RESULT:OK,`,
  `$EVENT,NODE:PUENTE,EVT:LOG,` y los dos `$ERR`; su censo de `src/` ve `registro.cpp` solo y exige que no toque
  `enlace_escribirLinea()`. `esp32_10` recalcula 142 <= 144. `esp32_06` bloque 5, `esp32_07`, `esp32_08` y `app_10`
  no cambian de forma; si `app_01_comandos` cae por `SIN_PIN`, se le da `CMD_LOG_PREFIJO` en la forma que ya lee: el
  pack no se toca. Simulador del puente (fila 5): `CMD:LOG,DESDE:` entra solo como orden reclamada (lee las
  `static const char` del predicado); su cuenta no baja de 156/156.
- compila `esp32`: verde con `#include <esp_partition.h>`; flash y RAM en el acta.
HUECO hasta banco: (a) y (e) se miden en la tarjeta (`CRC` del `$ACK` antes y despues de 33 anotaciones; el parte
tras un corte); `FECHA:`/`HORA:` con el DS3231 en hora; el lote con la app en segundo plano. Medida de §8: ~350 lineas
de producto en `ESP32_Expansion` (no cuentan en el numerador, §9.7) contra ~80 de test DOM y ~10 de pack: no sube.

### 9.9 La pantalla del operario (`N-178` M7; `D-80`..`D-83`) — decidido el 14/10, NO construido

Fuente: `ROADMAP_APP.md` 2-3 y la revision del 09/10; lo que la app DEBE hacer. Decidido por Fable por delegacion del
responsable, 14/10, y `D-101` el 15/10. Sin arnes nuevo (`CLAUDE.md` §4): `tests/dom_*.js` sobre el `app.js` real;
`www/` identica.

**(a) Dos zonas, y la jerga solo en una (`D-80`).** Zona del operario: la cabecera, Trafico (`#tab-estado` entero,
con la ventana del Poste 2 y la reja de telemetria), el toast y todo modal o cartel (`.modal-overlay`,
`js/aviso_corte.js`, `js/aviso_camara_pluma.js`, `js/carteles_equipo.js`, la hoja de conexion). Zona tecnica: las
pestanas con clave (`admin-only`: Tiempos, Diagnostico, Depuracion) y Eventos. Eventos queda en la tecnica a
proposito: cada linea lleva la orden y el motivo LITERAL del equipo porque es lo que se copia a la hoja de banco y al
reporte (`CLAUDE.md` §7.2: el sintoma trae el numero); la frase de «Que falta:» va al lado, no en su lugar.
Jerga = lo que casa con `JERGA` (una sola expresion, en el test, copiada aqui):
`\bRSSI\b|\bRTT\b|\bMAC\b|\bNODE\b|\$STATUS|\$EVENT|\$ACK|\$ERR|\bCMD:|\bD-\d{1,3}\b|\bJ1[4-7]\b|OTRO_PERRO|_MUDO|`
`SET_MODO|SET_DEG_AUTO|SET_RTC|no viaja en esta trama|ESP32|STM32|\b(RF|BAT|RTT|T):--`.
`MAESTRO` y `ESCLAVO` no son jerga: §6 los pone en la cabecera junto a POSTE 1 / POSTE 2. Lo que cambia, medido
sobre el fuente de hoy (cada uno con su texto nuevo; el literal del equipo sigue en Eventos):
1. `#phase-desc` entre la conexion y el primer `$STATUS`: «EQUIPO SIN IDENTIFICAR - esperando su primer dato».
2. `pintarAjeno()` contra el Esclavo: «SIN DATOS · se ve desde el POSTE 1» (hoy «no viaja en esta trama»).
3. Reja de telemetria: la etiqueta `RTT` pasa a «respuesta»; la nota de la barra se queda en su SITIO (el comentario
   de `index.html` explica por que) y dice «% de latidos contestados entre postes. No es la potencia de la antena.»;
   «CÁMARAS J16» pasa a «CÁMARAS» (el conector esta en la guia de montaje y en SPEC 5).
4. Ventana del Poste 2: la lista «Si publica / NO publica» en palabras del operario, sin `D-23`, sin `<code>` de
   campos de trama y sin `$EVENT`; el titulo del recuadro «Radio de este poste · contadores de linea»; el vacio dice
   «no esta llegando su medida». El recuadro «Ultimos movimientos de este enlace» (`#diag-log`: las 12 ultimas
   lineas de Eventos, literales, con «Caja Negra», `RTT:`, «MAC real») es Eventos acercado, y pasa con su cabecera a
   la zona tecnica: clase `admin-only`, en su sitio (`D-101`). Ni se traduce (seria la segunda copia que `app.js`
   rechaza en su comentario) ni se exime del test; el operario tiene Eventos a una pestana, sin clave.
5. Toasts de `js/avisos_equipo.js` (`HORA_ESP32|J17_MUDO`, `|RECHAZADA_FORMATO`, `|SIN_HORA_DEL_ESP32`) y el de
   `app.js` «revise J17»: el toast dice la consecuencia y el paso del operario («Este poste no tiene hora fiable:
   avise al tecnico»); el conector y el circuito siguen en la linea de Eventos, que no cambia. Oraculo (`D-101`):
   `J17_MUDO` y `RECHAZADA_FORMATO` «Este poste no tiene hora fiable: avise al tecnico (detalle en Eventos)»;
   `SIN_HORA_DEL_ESP32` «Este poste no tiene hora: pongasela desde el telefono (Tecnico > Sincronizar)». Las pruebas
   que exigen «revise el circuito ESP32-STM32» en el toast (`test_unitarios_app.js` «Avisos D-26» J17_MUDO, 4.o
   aserto; `test_dom_execution.js` 15.2, ultimo aserto) celebran el defecto (`CLAUDE.md` §9) y se INVIERTEN: el toast
   no casa con `JERGA` y contiene «avise al tecnico»; los asertos sobre `texto` (REVISE EL CIRCUITO...) se CONSERVAN.
6. El aviso de paleteros nombra la maniobra con el rotulo del boton pulsado (`<strong>` de `_boton`, o la orden si no
   hay boton): «Maniobra: Modo Manual.»; `SET_MODO:...` sale de la vista del operario y sigue en Eventos.
7. Hoja de conexion (`js/lista_equipos.js`): el detalle dice «Dirección <mac>», no «MAC:».
8. Traduccion de `ARRANQUE` con `CAUSA:OTRO_PERRO` (`js/aviso_corte.js`): el toast, si lo hay, sin la sigla.

**(b) Un color, una accion; gris es «sin dato» (`D-83`).** Rojo: hay que actuar (alarma, parada, orden que no
salio). Ambar: mirar antes de seguir (precaucion, aviso, acuse que no cambio nada). Verde: en orden, da paso. Gris
(`--text-muted`): sin dato medido. Cyan se queda para los mandos de ciclo (DAR PASO, RETIRAR AMBAR) y la informacion
sin accion (acuse OK, orden enviada): retirarlo es un rediseno sin medir en el telefono (contraste 6,52:1 del 09/10)
y `style.css` ya razona por que DAR PASO no puede parecerse a una parada. Lo que cambia: ningun valor de Trafico que
diga `--`, `-- V`, `SIN DATOS`, `NO SE SABE` o «Sin datos del equipo» lleva clase ni color rojo, ambar, verde o cyan
(hoy `#rf-quality` y `#cam-estado` nacen `cyan`, y el anillo sin estado cae a `red` en `updateCountdownRing()`:
pasa a una clase `sindato` gris en `.ring-fill` y `.ring-num`). `js/testigo.js` «Lleve el MISMO testigo...» sigue
en rojo: es una accion.

**(c) Una frase por estado.** `#phase-desc` = `<POSTE>: <frase> · <otro poste>`; la frase es UNA por `ESTADO:`,
de `ESTADOS` (`app.js`), y esta tabla es el oraculo del test: `ROJO` «ROJO, este poste no da paso» · `ROJO+AMAR` «Rojo y
amarillo: va a abrir el verde» · `VERDE` «VERDE, este poste da paso» · `AMARILLO` «AMARILLO, este poste está cerrando su
paso» · `FALLO COM` «ámbar y pluma arriba: pasar con precaución», y con `MODO:AMBAR` la frase por ORIGEN de §9.11
(`D-93`), que decide `_fraseDeEstadoLuces()`. Sin estado: «SIN ENLACE - sin datos del equipo», «EQUIPO SIN IDENTIFICAR -
esperando su primer dato», «ESTADO no reconocido: "<literal>"». Ninguna otra funcion escribe `#phase-desc`; ninguna
frase pasa de 60 caracteres sin el otro poste (lo que ya exige `tests/dom_cabecera_fase.js` para FALLO COM).

**(d) Cabecera legible a 360 px (`D-82`).** jsdom no mide pixeles y el recorte del 06/10 no se reprodujo en
simulado (`ROADMAP_APP.md` 3): no se toca el reparto de `style.css` (N-75, §4.ter) ni las pastillas en emoji a
<= 400 px. Lo que si se acota, y se puede medir en jsdom: `#rssi-text` nunca pasa de 16 caracteres, para que quepa
junto a «MAESTRO (POSTE 1)» (17) con la fuente mono de reserva, sin red. «(sin radio entre postes)» (24) pasa a
«(sin radio)»; «(radio N %)», «(enlazado)» y «(sin enlace)» se quedan. Vocabulario fijo: «sin enlace» = el telefono
no oye al poste; «sin radio» = los postes no se oyen entre si (§6).

**(e) Un solo aviso al cambiar el verde con el Degradado en marcha (`D-81`), SIN retirar la barrera.** Hoy son dos:
`Testigo._pulsar()` abre `Dialogo.confirmar(...)` («Paletero puesto: enviar», un toque) y despues, al enviar,
`AvisoDegradado.permite()` abre el de paleteros (casilla + «Cambiar este poste»). Lo que DEBE hacer: en el caso
`_cambioEnCaliente()`, `_pulsar()` no abre el `Dialogo`: pide a `AvisoDegradado` el MISMO modal
(`#aviso-deg-modal`, casilla `#chk-aviso-deg` y boton) con titulo «Cambio de tiempos con el Degradado en marcha» y
cuerpo con lo que exigian los dos: «Hasta que el POSTE 2 acepte el MISMO testigo, los dos postes llevan ciclos
distintos y puede haber verde en los dos sentidos. Sin radio el otro poste no se entera y sigue dando verdes por su
reloj. Ponga PALETEROS en los dos extremos ANTES de seguir y lleve el testigo al POSTE 2 antes de las HH:MM:SS.»
Confirmar (casilla marcada) deja el vale de un solo uso para `SET_MODO:DEG_T` (`VIGENCIA_MS`, 30 s) y llama a
`_pulsar(punta, true)`: PIN, `SET_RTC`, `_enviarTestigo()`; `permite()` consume el vale y NO abre un segundo modal.
Si el vale caduco (PIN lento mas los 10 s de `ESPERA_RTC_MS`), `permite()` vuelve a preguntar: la barrera nunca
afloja, solo deja de preguntar dos veces lo mismo. Cancelar no envia nada y lo anota. Modo de fallo buscado (§11.7):
el vale se casa por nombre `SET_MODO:DEG_T` y en 30 s no cabe otra orden de ese nombre desde otra pantalla; `DEG_FIN`
y el resto siguen con su aviso propio. Con los mismos verde y despeje que el guardado no hay cambio en caliente: el
aviso de paleteros sale al enviar, como hoy (un aviso tambien).

Quien lo ejerce (rojo hoy, todos sobre el `app.js` real):
- `tests/dom_usabilidad.js`, bloque nuevo «J (D-80)»: clona `body`, quita `.admin-only`, `#tab-eventos`, `script`,
  `style`, y pasa `JERGA` sobre el texto en seis fotos: (1) tras conectar y antes del primer `$STATUS`; (2) Maestro
  `AUTO`; (3) Maestro `FALLO COM`, `RF:0`, `ESC:?`; (4) Esclavo `SUBORDINADO` con la ventana del Poste 2 y un
  `$EVENT ... ORIGEN:ENLACE_RF`; (5) `$ALARM,NODE:MAESTRO,EVENTO:HORA_ESP32,CAUSA:J17_MUDO,...` y el toast; (6) en
  `DEGRADADO`, `SET_MODO:MANUAL` con PIN y el `#aviso-deg-modal` abierto. Falla nombrando el trozo que casa.
  Bloque C ampliado (`D-83`): `#rf-quality`, `#cam-estado`, `#cd-num`, `#bat-voltage` sin `$STATUS` y con `--`: sin
  clase ni color rojo/ambar/verde/cyan.
- `tests/dom_cabecera_fase.js`: `(sin radio)` en vez de `(sin radio entre postes)`, y `#rssi-text.length <= 16` en
  cada paso; y la tabla (c): por cada `ESTADO:` inyectado, `#phase-desc` empieza por `<POSTE>: <frase>`.
- `tests/dom_textos_spec4.js` bloque 15 (14/10 (1)), INVERTIDO: en `DEGRADADO` con verde 6 se abre
  `#aviso-deg-modal` (no `#dialogo-modal`) con los cuatro trozos del cuerpo y `a.tramas` vacio; Cancelar no envia;
  casilla + confirmar -> PIN -> `$ACK SET_RTC` -> el testigo sale con 360 y el modal NO vuelve a abrirse; con los
  mismos tiempos que el guardado, al pulsar no hay modal y se abre el PIN.
- `tests/dom_aviso_degradado.js` (1): `#aviso-deg-orden` dice «Modo Manual» y no `SET_MODO`.
Ficheros: `index.html`, `app.js`, `style.css`, `js/testigo.js`, `js/aviso_degradado.js`, `js/avisos_equipo.js`,
`js/lista_equipos.js`, `js/aviso_corte.js`, y sus copias en `www/`. `app.js` no crece en neto. Medida de §8: ~60
lineas de producto contra ~70 de test DOM; la razon instrumentos/producto no sube porque el test sustituye prosa de
manual, no firmware. Sin medir hasta el telefono: contraste del gris `sindato` a pleno sol y la cabecera con fuente
de reserva sin red.

### 9.10 Practica sin poste (`N-178` M9; `D-58`, `D-86`..`D-89`) — construido, sin telefono

Fuente: `D-58` y `ROADMAP_APP.md` 14. Lo que sigue es lo que la app HACE (`js/practica*.js`). Decidido por Fable
por delegacion del responsable, 14/10. Es producto, en `js/` (copia identica en `www/js/`): el simulador de PC del
repo sigue CONGELADO (`CLAUDE.md` §4) y no se toca. La barrera que esto roza es la de `N-75`: un tablero que pinta
un cruce que no existe. Por eso la practica no pinta nada por su cuenta: entra por el cable falso y la app la trata
como a un equipo, con la franja SIMULADO encima de todo.

**(a) Donde esta y como se entra (`D-86`).** Un boton «Practicar sin poste» (`#btn-practica`) en Diagnostico
(`#tab-diag`, `admin-only`): sin la clave de administracion (§9.2) no se ve ni entra (`Practica.entrar()` sin
`body.admin` no hace nada). Entrar con un enlace abierto (`state.connected`, leido por `window.appEnlazada()`) se
rechaza con toast y una linea en Eventos: la practica no desconecta a nadie. Sin pestana nueva: `test_dom_execution.js`
cuenta cinco pestanas y sigue contandolas.
- Al entrar, `Practica` guarda `window.bluetoothSerial` y lo sustituye por `PuertoPractica` (`isEnabled`, `list`,
  `connect`, `subscribe`, `write`, `disconnect`, `isConnected`), pone `body.simulado`, muestra `#franja-simulado` y
  abre el modal de equipos (`#btnDevice`): la flota se lista, se elige y se enlaza por el MISMO camino que un equipo
  real (`abrirEnlace()`, `subscribe`, `parseNmeaTelemetry()`, `juzgarTrama()`); ningun widget se pinta por otro.
- `#franja-simulado` es fija arriba, por encima de modales y toasts, magenta (color que no esta en la paleta de
  estados de `D-83`): «SIMULADO - practica sin poste: ningun equipo real», con el boton «Salir» (`#btn-franja-salir`).
  Salir pasa por `desconectarEquipo()` (el `disconnect` del puerto falso), restaura el `bluetoothSerial` original y
  quita franja y clase. Mientras dura, `bt.list()` no toca la radio: el original no se llama.
- Ningun id ni literal de la practica contiene `btn-sim`, `sim-btn-`, `field-sim-bar`, `runLocalTicker`,
  «SIMULADOR DE PRUEBAS» ni «DEMO EN VIVO»: `test_funcional_app.py` los busca en `index.html` y en `js/*.js`.

**(b) La flota y sus cintas (`D-87`).** Una cinta por equipo en `js/practica_cinta_<n>.js` (<= 500 lineas cada
una, cabecera con la ruta de `evidencia/` de origen, el firmware que llevaba el equipo y el tramo de horas). Cada
linea es una trama `<--` copiada LITERAL de la cinta, con su checksum: la app la vuelve a juzgar. Ritmo: una linea
por segundo en el orden de la cinta, y vuelve a empezar al acabar. Flota inicial, cuatro equipos con rotulo real
`SEM-<serie>-M`/`-E` y direccion `SIM:<n>`:
1. Poste 1 sin radio con el 2: `evidencia/061020261110/Tramas_KM_12_EL_SISGA_2026-10-06.txt` (d810f81, posterior
   a `D-53`; `MENU`/`AMBAR`, `ESTADO:FALLO COM`).
2. Poste 1 en ciclo: `evidencia/021020260914/Tramas_KM_12_EL_SISGA_2026-10-02.txt` (d3606be; `INTELIGENTE`,
   `DEGRADADO`, `VERDE`/`ROJO`/`AMARILLO`). Es la unica anterior a `D-53` que se queda: no hay cinta con `ROJO+AMAR`
   crudo; la primera que lo traiga la sustituye.
3. Poste 2 en Degradado con testigo: `evidencia/2026-09-29_campo_testigo/IOTVIAL_38EB53_20260929-183728.txt`
   (8974932, `NODE:ESCLAVO`, `MODO:DEGRADADO`).
4. Poste 2 con alarma de radio: `evidencia/011020260934/IOTVIAL_38EB53_20261001-073243.txt` (`$ALARM FALLO_RF`,
   `CAMBIO_A_AMBAR`).
La cinta `2026-09-29_web_con_poste_simulado/` NO vale: viene de un poste simulado.

**(c) Las ordenes (`D-88`).** `write()` del puerto falso deja la orden en el diario y en la cinta como siempre
(`-->`), y contesta a los 100 ms con el primer `$ACK` o `$ERR` de la cinta de ese equipo cuyo `CMD:` coincide,
literal y con su checksum. Sin respuesta grabada, silencio: lo que el tecnico ve es el camino sin acuse de la app,
no un acuse inventado. El PIN se pide igual (§4) y viaja igual. Ninguna trama de practica sale de un literal escrito
a mano: `juzgarTrama()` rechazaria un checksum recalculado a mano y la propiedad de `app_04` (la app no habla el
idioma de un simulador) se conserva porque todo lo que entra lo emitio un firmware.

**(d) Marcas y registros (`D-89`).** Con `Practica.activa()`: `Exportar.nombreFichero()` da
`IOTVIAL_SIMULADO_<serie>_...txt`; la cabecera de la cinta (`Exportar.textoCinta()`) y la del diario
(`js/depuracion.js`, «IOT-VIAL - DIARIO DE ORDENES») llevan como primera linea `Simulado: SI - practica sin poste,
no vale como evidencia`; `RegistroSesion.anotar()` (`Directory.External`, `D-55`) no escribe nada durante la
practica, y al entrar se anota en Eventos. Un `.txt` con esa marca no se acepta en la hoja de banco ni en
`evidencia/`. Lo que la practica no ensena, y se dice en la franja de Eventos al entrar: el tiempo real de respuesta
del equipo, el emparejado de Android y la radio entre postes.

**Pruebas (vistas en rojo antes de construir).** `tests/dom_claves.js`, bloque «Practica»: (1) `#btn-practica`
existe en `#tab-diag` y sin clave la practica no entra (`bluetoothSerial` identico, franja oculta); (2) con el
enlace del arnes abierto, entrar se rechaza; (3) desconectado y con clave: franja visible, `body.simulado`, el
modal lista >= 4 filas de `Practica.FLOTA`, y al pulsar la primera `#node-name` pasa a `MAESTRO` con `a.tramas` del
mock original VACIO (nada llego al puerto real); (4) la `VERSION` que la app pide al enlazar recibe el `$ACK` de la
cinta y Eventos ensena su `FW:`; (5) Salir devuelve el `bluetoothSerial` original y quita franja y clase; (6)
campo nuevo: cada `CAMPO:` del literal `$STATUS` de `Maestro/src/bluetooth.cpp` esta en las cintas del Maestro, y
cada uno del de `Esclavo/src/bluetooth.cpp` en las del Esclavo (un campo nuevo en el firmware deja esto en rojo
hasta renovar la cinta). `tests/dom_exportar.js`: en practica, con plugins, `Filesystem.appendFile` no se llama y el
nombre y la cabecera llevan SIMULADO. `test_funcional_app.py` sigue en verde con los literales de (a).
Ficheros: `index.html`, `style.css`, `app.js` (solo `window.appEnlazada`), `js/practica.js`,
`js/practica_cinta_1..4.js`, `js/exportar.js`, `js/depuracion.js`, los dos tests, y las copias en `www/`.
`app.js`, `index.html`, `style.css` y `js/depuracion.js` no crecen en neto. Medida de §8: ~150 lineas de producto
(sin contar las cintas, que son datos copiados) contra ~70 de test DOM; la razon instrumentos/producto no sube.
Sin medir hasta el telefono: que la franja se lea sobre el modal del PIN y que el bucle de 300 tramas no caliente.

### 9.11 El origen del ambar del Poste 1 (`N-178` M11; SFTY-22; `D-91`..`D-93`) — construido, sin banco ni telefono

Fuente: SFTY-22 (`OPTIMIZACIONES.md`), escrita el 01/08 para un LCD que salio del equipo (`D-17.bis`). Decidido por
Fable por delegacion del responsable, 14/10. Lo que SFTY-22 pedia, medido contra el fuente de hoy (`D-91`):

| pedia | donde esta hoy |
|---|---|
| aparecer sola al entrar en ambar, sin pasar por el menu | `#phase-desc` cambia con el `$STATUS` de 1 Hz; la app no pide nada al equipo |
| la causa (`SIN ENLACE`) | `$ALARM` con `CAUSA` y el tramo del enlace (`N-108`), repetida mientras dure (`D-67`, `D-68`) |
| desde cuando | `HORA` de `$ALARM`/`$EVENT`; sin telefono, el LOG del puente (`D-73`, clase `S` al cambiar `MODO`) |
| la fila `RX` de SFTY-15 y la telemetria de SFTY-14 | el periodico `ENLACE_RF` (`D-23`, `D-32` (1)) en `js/diagnostico_enlace.js`; `RF`/`RTT` en la reja |
| separar las tres averias del 31/07 | `RX`/`OK`/`RUIDO` y sus transiciones a la bitacora (`DiagnosticoEnlace.ver()`) |

**Lo que faltaba (hueco medido, y es lo que cierra este apartado).** `MODO_AMBAR` del Poste 1 tiene tres origenes
que el firmware ya distingue y de los que cuelga una barrera: `modo_ambar_esDeArranque()` exime del veto R-4 (§4.1 de
SPEC 1) y `modo_ambar_origenEsclavo()` gobierna la cancelacion (`N-152`). Son la app (`SET_MODO:AMBAR`), el Poste 2
por radio (`CMD_AMBAR_ESCLAVO`, `N-142`) y el arranque tras un corte o un watchdog (`D-40`). El `$STATUS` dice
`MODO:AMBAR` en los tres y `_fraseDeEstadoLuces()` traduce los tres como «ámbar pedido por el operario»: tras un
corte, el operario lee que alguien lo pidio. Las dos lineas de `modo_ambar_fijarMotivo()` (`ambarL1`/`ambarL2`) no
tienen lector desde `D-44`: eran la pantalla.

**(a) El equipo (`D-92`).** `Maestro/src/bluetooth.cpp`, bloque `D-32` (1): el `DETALLE` del periodico `ENLACE_RF`
termina en ` AMBAR:<o>` solo mientras `modoActual_get() == MODO_AMBAR`; `o` es `ARRANQUE` si
`modo_ambar_esDeArranque()`, si no `POSTE2` si `modo_ambar_origenEsclavo()`, si no `APP`. Fuera de `MODO_AMBAR` el
campo no va: el formato de hoy no cambia. Formato `"RX:%lu OK:%lu RUIDO:%lu REIN:%u%s"`, con el `%s` desde una
funcion estatica del mismo fichero que solo devuelve literales (`" AMBAR:ARRANQUE"`, 15, el mas largo), para que
`esp32_07_presupuesto_bytes` lo acote siguiendo sus `return` y no por estimacion. `det[76]` (44 + 16 + 15 + NUL;
era `det[61]`) y `payload[144]` en `bluetooth_reportarEvento()` (era `[128]`): por BUFFER el `$EVENT` mide
42 + 9 + 75 + 15 = 141 <= 143, y 141 + 5 <= 159 de `tramaCompleta[160]`; la cuenta la rehace el pack, no este
documento. El Esclavo no cambia: su ambar no tiene estas marcas (abajo). Nada en el `$STATUS` (margen 2, `D-60`), ni
`$EVENT` nuevo, ni repeticion propia: el periodico de 30 s ya es la via del telefono que llega tarde (`REIN`, `D-64`)
y `REGISTRO_EXCLUIR` lo deja fuera del LOG; el flanco ya queda en el LOG por el `$STATUS` de clase `S`, el `$ACK` de
`SET_MODO:AMBAR` y el parte de arranque. La APK anterior con este firmware pierde el periodico SIEMPRE, no solo en
ambar (`REIN:` ya rompe su `PATRON`; medido por QA el 15/10): pareja obligatoria, §9.13 (`D-102`). Ancla `D-92` en
el bloque. `bluetooth.cpp` no crece en neto.

**(b) La app (`D-93`).** `js/diagnostico_enlace.js`: `PATRON` admite `(?:\s+AMBAR:(APP|POSTE2|ARRANQUE))?` al final;
`leer()` devuelve `ambar` (`null` si no viene); `ver()` lo guarda y, si cambia respecto al anterior, anade a `avisos`
una linea «ámbar de este poste: <frase>» (una por cambio, nunca por repeticion); `olvidarAmbar()` lo borra;
`fraseAmbar()` devuelve la frase por origen. `app.js`: `_fraseDeEstadoLuces()` con `FALLO COM` y `MODO:AMBAR`
devuelve `DiagnosticoEnlace.fraseAmbar()` (sin el modulo, la frase de `null`); al pintar un `$STATUS` con `MODO`
distinto de `AMBAR` llama a `olvidarAmbar()`, para que un episodio nuevo no herede el origen del anterior;
`olvidar()` ya corre al desconectar. Oraculo de §9.9 (c), frases de <= 60 caracteres sin el otro poste:
`null` «ámbar en este poste (origen sin dato): pasar con precaución» · `APP` «ámbar pedido por el operario: pasar
con precaución» · `POSTE2` «ámbar pedido desde el POSTE 2: pasar con precaución» · `ARRANQUE` «ámbar de arranque
(poste reiniciado): pasar con precaución». La app afirma el origen solo cuando lo ha leido (`CLAUDE.md` §2: un dato
sin medir no se publica con forma de medida). Copias `www/js/` identicas. `app.js` no crece en neto.

**(c) Prueba, vista en rojo antes del codigo.** `tests/dom_cabecera_fase.js`, bloque (4) nuevo, sobre el `app.js`
real en jsdom: `MODO:AMBAR` + `FALLO COM` sin periodico → frase con «sin dato» y sin «operario»; periodico
`$EVENT,NODE:MAESTRO,ORIGEN:ENLACE_RF,DETALLE:RX:10 OK:5 RUIDO:0 REIN:1 AMBAR:APP,HORA:14:31:30` → /operario/;
`AMBAR:POSTE2` → /POSTE 2/; `AMBAR:ARRANQUE` → /reiniciado/; las cuatro <= 60 y con /precauci/; el mismo periodico
dos veces → `state.events` crece 1 y no 2; `MODO:AUTO` y otra vez `MODO:AMBAR` sin periodico → «sin dato». Antes del
codigo, en rojo: «sin dato», `POSTE2`, `ARRANQUE` (la app decia «operario» siempre) y la cuenta de la bitacora
(el periodico con `AMBAR:` no casaba `PATRON` y caia como `$EVENT` suelto). El firmware no se ejecuta en ningun arnes
(`adaptador_maestro.cpp` sustituye `bluetooth_reportarEvento()` por un vacio): lo acotan la compilacion y
`esp32_07`; la trama real la lee Marco en banco (`CLAUDE.md` §0.3).

**Queda fuera (version siguiente):** el origen del ambar del Poste 2 (orfandad SFTY-6, `CMD_GO_AMBAR`, latch
SFTY-21, `DEG_RENDIDO` de `D-68`): sus marcas no viven en un solo sitio y su `$STATUS` no dice `MODO:AMBAR`.

Ficheros: `Maestro/src/bluetooth.cpp`, `App_Semaforo/js/diagnostico_enlace.js`, `App_Semaforo/app.js`,
`App_Semaforo/tests/dom_cabecera_fase.js` y las copias en `www/`. Medida de §8: ~25 lineas de producto contra ~15 de
test; la razon instrumentos/producto no sube.

### 9.12 `app.js` en modulos, por tandas y con salida identica (`N-178` M13; `D-98`) — construido 15/10, sin telefono

Fuente: `ROADMAP_APP.md` 8. Decidido por Fable por delegacion del responsable, 14/10. Medido sobre el arbol del 14/10:
`app.js` es UN cierre (`document.addEventListener('DOMContentLoaded', () => { ... })`) de 5860 lineas con 240 nombres
dentro (156 `const`, 82 `function`, 2 `let`); nada suyo es global. Los arneses jsdom cargan los modulos leyendo las
`<script src="js/...">` de `index.html` en su orden (`test_dom_execution.js`, `modulos`), y `tests/dom_*.js` montan la
app por el mismo cargador: un modulo nuevo entra sin tocar ningun test.

**El molde de corte (vale para todas las tandas).** Un modulo es un fichero `js/<zona>.js` de 500 lineas como maximo,
cargado por `index.html` ANTES de `app.js` y despues de los modulos que use. El bloque que sale de `app.js` se mueve
LITERAL (mismas lineas, dos espacios menos de sangria; el `diff` sin sangria queda vacio), con sus comentarios. Dos
formas y ninguna mas: (a) una tabla o funcion pura va a columna 0 con el MISMO nombre de simbolo que anclan los packs
y la SPEC (`ACK_TEXTO` sigue llamandose `ACK_TEXTO`) y acaba en `module.exports`, molde `js/textos_spec4.js`; (b) lo
que necesita estado o funciones de `app.js` las recibe por `X.instalar(dep)` con cada dependencia nombrada, molde
`js/diario_vista.js`, y `app.js` conserva un envoltorio con el nombre de siempre (`renderDiario()`), que es lo que leen
los packs por ruta. Prohibido: leer un nombre del cierre de `app.js` como global (en el telefono no existe: el cierre
no publica nada, y en jsdom solo se publica lo de columna 0 de los modulos), y un modulo que declare un nombre que ya
declara otro a columna 0 (`SyntaxError` al cargar en el navegador, y la app no arranca).

**Salida identica, medida y no declarada.** Antes y despues de cada tanda, sobre el mismo arbol: `test_dom_execution`
(hoy 668 PASS | 0), `tests/test_unitarios.js` (85), `test_unitarios_app.js` (67), `test_funcional_app.py` (70/70) y el
banco por packs con el MISMO total por pack; `documentos_03_trama_status` verde con `www/` y, si existe, el `public/`
de Android copiados. Una cifra que baja, un pack que cambia de total o una ruta nueva sin su tupla en el mismo commit
(`CLAUDE.md` §5) deshacen la tanda: dos intentos y se revierte (`N-178`).

**Tanda 1 (esta version): las tablas de acuse y rechazo.** `js/textos_ack.js` recibe `ACK_TEXTO` (330 lineas, cero
nombres del cierre, cero modulos); `js/textos_err.js` recibe `ERR_TEXTO` y `ERR_MOTIVO` (unas 360, cero nombres del
cierre) y exporta `{ ERR_TEXTO, ERR_MOTIVO }`. `_traducirRechazo(data)` se QUEDA en `app.js`: usa `state.node`,
`DegAuto`, `DiagnosticoEnlace`, `Testigo` y `RegistroEquipo`, y es el consumidor que `app_10` mide en `app.js`
(`const dicho = ACK_TEXTO[clave] || ...`). `index.html` carga los dos detras de `js/textos_spec4.js`. Instrumento que
cambia de ruta en el mismo commit: `app_10_ack_con_varios_sies` lee `ACK_TEXTO` de `js/textos_ack.js` y `ERR_TEXTO`
de `js/textos_err.js` (`_tabla()` busca `const <nombre> = {` y cuenta llaves: sirve a columna 0), y el consumidor sigue
en `APP_JS`; mismo total de comprobaciones. Citas que cambian al construir: §3.ter.ter (dos veces «`ACK_TEXTO`
(`app.js`)»), §9.5 punto 2, y los comentarios de `js/testigo.js` (linea 11) y `js/avisos_equipo.js` (linea 5) que
dicen «de `app.js`».

**Prueba, en `tests/test_unitarios.js` (fila «test unitarios TDD de la app»), vista en rojo antes del corte:**
(1) `index.html` carga `js/textos_ack.js` y `js/textos_err.js` ANTES de `app.js` (molde de la linea de
`avisos_equipo.js`); (2) `app.js` ya no declara `const ACK_TEXTO`, `const ERR_TEXTO` ni `const ERR_MOTIVO`; (3)
`require('../js/textos_ack.js')['SET_MODO:ALCANCE|OK'].texto` es una cadena y `require('../js/textos_err.js')` trae
las dos tablas; (4) la guarda del molde: ningun nombre declarado a dos espacios en `app.js` (`const|let|function|class`)
aparece como identificador en un modulo partido salvo que el modulo lo declare o lo reciba como `dep.<nombre>`. Medido
el 15/10: antes del corte 85 PASS | 5 FALLAS, despues 90 | 0; (4) en rojo con `state.node` inyectado en `ERR_MOTIVO`,
restaurado por hash (`CLAUDE.md` §6). Lo demas, igual antes y despues: jsdom 683 | 1 (fallo previo de `D-59`).

**Orden de las tandas:** 2.a la pintura de luces, construida el 15/10 (`N-178` N5): tablas a `js/luces_tablas.js`,
pintores a `js/luces.js` (`Luces.instalar(dep)`, 14 nombres del cierre; `renderLights`, `pintarPluma`, `pintarCamaras`
quedan de envoltorio en `app.js`), `app_04` lee los tres ficheros; TDD 87|4 antes, 91|0 despues; 3.a depuracion (241
lineas, 13 nombres) con `D-72`..`D-75`; 4.a el enlace y los equipos (648 lineas, 21 nombres); la ULTIMA, los modales y
el PIN (189 lineas): es la barrera de §4 y se mueve solo con `dom_claves.js` delante y `app_01`/`app_05` recontados.

### 9.13 Firmware y APK del MISMO paquete, y la APK primero (`D-102`) — decidido el 15/10, regla de campo

La app nueva lee el firmware nuevo y el anterior (`96c3d32`: sin `REIN:`, `AMBAR:` ni `LOG:`; §9.4 (4), §9.8 (f),
§9.11 (b)). La APK anterior con el firmware nuevo, NO: su `PATRON` del periodico `ENLACE_RF` acaba en `RUIDO:` y con
`REIN:` no casa nunca; el diagnostico de enlace enmudece y cada 30 s cae una linea a la bitacora de 30 entradas. Regla:
el firmware de un paquete no se carga en un poste sin la APK del MISMO paquete (`D-84`/`D-85`) en TODOS los celulares
que lo manejan, y en ese orden: primero la APK (lee los dos firmwares), despues el firmware; en ningun momento un
celular con la APK anterior frente a un poste con la nueva. La hoja de campo lo dice en su paso 1 con la huella sha256
de la APK. Lo ejerce `test_unitarios_app.js` suite 8 (periodico sin `REIN` y sin `AMBAR` leido por la app nueva: ya
existe); la APK anterior no se prueba, porque no se construye contra ella.
