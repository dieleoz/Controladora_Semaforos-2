# SPEC 2 — LAS DOS PUNTAS Y LA RADIO

> **EL REQUISITO, Y ES EL ÚNICO QUE IMPORTA AQUÍ: LOS DOS POSTES NUNCA DAN VERDE A LA VEZ.** No hay cable entre
> ellos. Lo único que los une es una radio lenta que pierde tramas, y esta especificación es lo que el equipo hace
> **con esa radio sana, degradada y muerta**.

**Qué NO entra:** el ciclo de un poste solo (SPEC 1) · la hora (SPEC 3) · la app (SPEC 4) · el cobre y las radios como
aparato (SPEC 5) · el procedimiento de campo y el parche `T-2` (SPEC 6). **De dónde sale:** manda esta spec;
`DECISIONES.md` es su índice para no volver a preguntar, y el fuente dice lo que el equipo HACE; los manuales no son
fuente. **Ninguna cifra vive aquí**: se cita el SÍMBOLO. **Todo lo medido está sobre `ef3504c` con el árbol limpio** —
el firmware lo tocan otros agentes.

## 1. EL REPARTO — quién decide qué

| | |
|---|---|
| **Poste 1 (Maestro)** | El ÚNICO que ordena verde. Su máquina de estados vive en el coordinador (`Maestro/src/coordinador.cpp`) |
| **Poste 2 (Esclavo)** | Obedece y ACUSA. No tiene coordinador: su lado de la radio es una cadena de condiciones en su bucle principal, **único lector de radio** de esa punta |
| **Repetidor** | Puente transparente con validación de CRC: **no origina ni una trama** |

**La única excepción a «el Maestro ordena» es el Modo Degradado** (§7): allí no hay radio y cada punta decide su luz
por reloj. Es el único modo que enciende un verde **sin confirmación del otro extremo**, y por eso es el que más
condiciones tiene delante — y el único que exige un ACUERDO CERRADO ANTES por radio (§8). **Contrato de la radio:** la
cabecera de protocolo, **idéntica byte a byte en las dos puntas** (lo compara `costura_01_contratos`). La trama lleva
identificador, comando, **un solo byte** de parámetro y CRC, y cada envío sale en **ráfaga de varias copias**
(`RF_BURST_COPIES`), con el receptor descartando las repetidas por identificador.

## 2. EL DIÁLOGO DEL CICLO

### 2.1 Los comandos, y quién los emite

| comando | dirección | qué es |
|---|---|---|
| `CMD_GO_GREEN` / `CMD_ACK_GREEN` | M→E / E→M | la única orden que abre paso en el Poste 2, y su «verde encendido y estable» |
| `CMD_GO_RED` / `CMD_ACK_RED` | M→E / E→M | la orden que cierra —también es el latido cuando el rojo no consta— y su «estoy en rojo», que desde `D-45` (`cda33df`) acusa el ROJO ENCENDIDO, tras el amarillo (§2.2.bis) |
| `CMD_PING` / `CMD_PONG` · `CMD_GO_AMBAR` | M→E / E→M | el latido, que mide que el enlace vive · y el ámbar ordenado, siempre detrás de una orden de rojo previa |
| `CMD_AMBAR_ESCLAVO` / `CMD_ACK_AVISO_AMBAR` | E→M / M→E | el Poste 2 avisa de su ámbar de emergencia, y el acuse (`D-31`) |
| `CMD_CANCELA_AMBAR_ESCLAVO` | E→M | el Poste 2 lo retira. **No se acusa, y es la regla** (§6) |
| `CMD_DEMANDA` / `CMD_ACK_DEMANDA` · `CMD_HORA_*` | E→M / M→E | la demanda de cámara y su respuesta, que dice si se atiende · y la hora, que **no toca luz** (SPEC 3) |
| `CMD_CONFIG_*`, `CMD_DELTA*` | M→E | 🔴 **NO son «servicio»: son el ACUERDO que autoriza el Degradado** (§8) |
| `CMD_PRESENTE` | M↔E | §7.ter (c). «Estoy en Degradado y te oigo»; no toca luz, modo ni silencio |

Del Poste 2 salen por iniciativa propia **tres** —la demanda, el aviso de ámbar y su cancelación—; el resto, respuesta.

### 2.2 El paso de verde a verde, que es donde se mata a alguien

Un cambio de sentido pasa SIEMPRE por todo-rojo, contado **desde el acuse del otro lado, no desde la orden**
(`N-162`); el plazo es el despeje que el modo le configuró al coordinador.
`Verde M -> C_MASTER_A_ROJO (amarillo M) -> C_ESPERA_ESTATICO_TRAS_MASTER (despeje) -> GO_GREEN ->
C_ESPERANDO_ACK_GREEN -> ACK_GREEN -> Verde E -> GO_RED -> C_ESPERANDO_ACK_RED (amarillo E) -> ACK_RED ->
C_ESPERA_ESTATICO_TRAS_ESCLAVO -> Verde M` (el amarillo de cada punta, §2.2.bis)
**Las cuatro puertas al verde propio del Maestro llevan el mismo veto de margen** (`puedeSostenerVerde()`): la espera
inicial, la espera tras el verde del Esclavo y las dos ramas de la petición de cambio desde «nadie en verde». **El
Maestro no abre su verde contando con que la orden de rojo LLEGÓ, sino con que se CUMPLIÓ**: la marca de rojo
confirmado se pone en un solo sitio —la llegada del acuse— y se baja en cada orden de rojo, al mandar verde y al
arrancar. Sin ella, una orden de rojo perdida con el Esclavo en verde dejaba verde en las dos puntas.

**El latido tiene DOS formas, y no es un detalle.** Sale cada cierto plazo, **suprimido mientras se espera un acuse**
(`SFTY-13`, colisión en el bus medio dúplex) y mientras corre la sincronización: si el rojo del otro lado **no
consta**, o si el Maestro está en reposo de menú o en fallo → sale **una orden de rojo** y se espera su acuse; en
cualquier otro caso → un latido simple. Es decir: **el reintento del todo-rojo de emergencia es el propio latido** —
sin esto no tenía ninguno. ⚠️ **Y esa supresión hace que el contador de silencio de la otra punta llegue ya
envejecido:** §9 y SPEC 6 D.1.

### 2.2.bis El amarillo de cierre en el diálogo (`D-45`, `N-174`) — CONSTRUIDO en `cda33df`, SIN BANCO

`Verde M -> AMARILLO M (C_MASTER_A_ROJO) -> rojo M -> despeje -> GO_GREEN -> verde E DIRECTO -> ACK_GREEN ->
GO_RED -> AMARILLO E (C_ESPERANDO_ACK_RED) -> rojo E -> ACK_RED -> despeje -> Verde M DIRECTO`

- **Cada punta enciende SU amarillo, en su fichero del semáforo** (`semaforo_forzarRojo()` sobre un verde, SPEC 1
  §3.2). El Poste 1 no ordena «amarillo»: `CMD_GO_RED` significa «cierra». No hay trama nueva para la luz;
  `CMD_GO_AMBAR` sigue siendo el ámbar intermitente.
- **`CMD_ACK_RED` acusa el ROJO ENCENDIDO, no la orden.** En la rama de `CMD_GO_RED` del bucle del Poste 2: tras
  `semaforo_forzarRojo()`, si la luz ya está en `S_ROJO` acusa al momento; si no, arma `ackRojoPendiente`, y al final
  del bucle el acuse sale en la vuelta en que el amarillo acabó en rojo (si acabó en otra cosa —ámbar de emergencia u
  orfandad— se olvida). Una orden de rojo repetida durante el amarillo no lo reinicia ni se acusa todavía; una de
  verde ni lo reabre ni se acusa (`luz != S_AMARILLO`): acabado en rojo, la siguiente repetición se trata como
  siempre.
- **La primera espera del acuse lleva el amarillo dentro:** en `C_ESPERANDO_ACK_RED` el primer plazo es
  `TIMEOUT_ACK_MS + AMARILLO_MS` (6,5 s) y los siguientes `TIMEOUT_ACK_MS` (3,5 s), para que el amarillo no gaste un
  reintento ni parezca radio mala.
- **El todo-rojo empieza cuando la punta que CIERRA está en rojo**: el despeje del Poste 1, desde su rojo estable
  (`C_MASTER_A_ROJO` espera `semaforo_estable()` en rojo, y `coordinador_actualizar()` renueva `tRef` al acabar todo
  amarillo); el del Poste 2, desde el `CMD_ACK_RED`, que ahora es ese mismo instante (y en reposo, sin verde
  asignado, la primera llegada del acuse también renueva `tRef`). **La marca de rojo confirmado** sigue poniéndose
  sólo con el acuse: durante el amarillo del otro poste NO consta su rojo.
- **`CMD_ACK_GREEN` es cierto al recibirse**: el verde se enciende directo (`semaforo_forzarVerde()`) y el acuse sale
  en la misma vuelta.
- **El presupuesto de radio, cerrado subiendo el umbral (`D-48`):** `SFTY6_SILENCIO_MS` pasó de 25 a 28 s. El peor
  caso del ciclo (`PEOR_CASO_CICLO_MS`) es `LATIDO_MS + AMARILLO_MS + CICLO_MAX_REINTENTOS × PASO_RADIO_MS` = 3 + 3 +
  5 × 3,56 = **23,8 s**: el amarillo entra una vez. Libres bajo el techo, **4,2 s**, como antes de `D-45` (§9 (A)).
  ~~**(c)** subir el umbral de silencio: vetado por el responsable~~ → **es la salida que decidió `D-48`**; lo
  vetado sigue siendo BAJARLO (§4). Las otras dos salidas barajadas —**(a)** un reintento menos, **(b)** un acuse
  de recibo inmediato— no se construyeron.
- 🔴 **ABIERTO, sin decidir: el borde de la desigualdad (B) de §9.** El `static_assert` junto a
  `puedeSostenerVerde()` suma `PEOR_CASO_CICLO_MS + TIMEOUT_ACK_MS` = **27,3 s** contra 28, y toma por borde el
  instante en que cae el ROJO de la suelta (`SFTY6_SILENCIO_MS - TIMEOUT_ACK_MS`). Contada desde el ARRANQUE de la
  suelta, que empieza un `AMARILLO_MS` antes, la suma es **30,3 s** y no cabe. El `static_assert` de la
  sincronización (`SYNC_MAX_INTENTOS`) sí mide contra el arranque. Cuál de los dos bordes vale para el ciclo no está
  decidido.
- 🔴 **ABIERTO: `ESC:AMARILLO` no se construyó.** `coordinador_estadoEsclavo()` no tiene amarillo: durante el
  amarillo del Poste 2 (`C_ESPERANDO_ACK_RED` con `quienVerde == QV_ESCLAVO`) publica `ESC:VERDE` hasta el
  `CMD_ACK_RED` (SPEC 4 §6).

### 2.3 Lo que el Poste 2 hace al recibir

Las órdenes de rojo y de verde refrescan el reloj de orfandad; las tramas de servicio y el ámbar ordenado **no lo
refrescan**, a propósito: significan «hay portadora», no «el Maestro está gobernando el cruce». Vetado, **ni se
obedece ni se ACUSA**: acusar un rojo que no se ha encendido dejaría al Maestro dando verde convencido de que aquí hay
rojo — *se guarda lo que ABRE PASO, no lo que lo para*. La orden de verde es **idempotente**: repetida sobre una luz
ya en verde, **re-acusa y no toca la luz**; sobre el amarillo de cierre (`D-45`) ni re-acusa ni reabre (§2.2.bis).

> 🔴 **EL VETO ES DE UN SUJETO Y EN EL FUENTE SON TRES `if`.** **UNO por SUJETO** (`D-8`): la app, la única que
> puede poner hoy ese ámbar —el mando salió del firmware (`D-30`) y su término de la condición con él (SPEC 1 §4)—.
> **TRES por RAMA**, que es lo que hay que guardar: la misma condición guarda, en el bucle principal del Esclavo, la
> rama de la orden de rojo, la de la orden de verde, y una **tercera** que es la recuperación tras fallo. **Esa tercera
> NO es un `else` de la primera:** una orden de rojo vetada arriba entra por ella y volvería a forzar rojo por su
> cuenta. El fuente lo avisa —*«guardar solo una de las dos deja la revocación intacta»*—. Una regla que ENUMERA
> sujetos comprueba que cada sujeto EXISTE, y dónde se ejerce (`CLAUDE.md` §2).
> `grep -c "bluetooth_ambarEmergencia()" 01_Firmware/Esclavo/src/main.cpp`

## 3. LOS REINTENTOS

El plazo de cada acuse y el número de reintentos viven en el coordinador (`SFTY-7`). Agotados: desde la espera del
acuse de **verde** → fallo **con alarma** `FALLO_RF / REINTENTOS_AGOTADOS / CAMBIO_A_ROJO`: esa rama sólo corre con
enlace vivo, y la vuelta del enlace (`SFTY-9`) lleva la luz a **rojo**, no a ámbar —el ámbar, si llega, lo reporta el
silencio con su propia causa— (`D-34`); desde la espera del acuse de **rojo** → fallo **sólo si hay enlace y queda
margen de silencio**, y si no se sigue pidiendo el rojo y la caída la reporta el ámbar por silencio, cuya alarma esta
puerta no puede tapar. **Contra la distancia no sirve esperar, sirve repetir**: sube la **pérdida**, no la latencia, y
la palanca es el número de copias de la ráfaga.

## 4. EL SILENCIO Y EL ÁMBAR DE HUÉRFANO (`SFTY-6`)

El umbral (`SFTY6_SILENCIO_MS`) vive **una sola vez**, en la cabecera de protocolo, porque gobierna las dos puntas: tres copias a mano fue como se desincronizaron (`N-69`).

| punta | ancla del silencio | qué hace al vencer |
|---|---|---|
| Maestro | **la respuesta que le CONTESTARON** | fallo, ámbar intermitente, alarma `FALLO_RF / SILENCIO_…`, y una orden de rojo **en ese mismo instante** |
| Esclavo | **la última orden que RECIBIÓ** | ámbar intermitente, misma alarma |

**Qué es «respuesta» en el Maestro, y sólo esto** (1.49c, validado sobre `622a20b`): la que **cierra el latido en
vuelo** —el `CMD_PONG` a un latido simple, el `CMD_ACK_RED` a la orden de rojo del latido—, el `CMD_ACK_RED` mientras
espera ese acuse **o en fallo**, y el `CMD_ACK_GREEN` mientras espera el suyo. Todas prueban **las dos direcciones**:
el Esclavo recibió una orden de esta punta. 🔴 **Una trama que no contesta a nada NO sostiene el silencio:** ni la
demanda de cámara (`CMD_DEMANDA`), ni el aviso de ámbar, ni los acuses de hora. Con la bajada Maestro→Esclavo muerta y
la cámara del Poste 2 pidiendo paso, **el Maestro vence su silencio igual que sin demanda** —antes cada demanda se lo
renovaba y sostenía su verde frente al ámbar del otro poste—. El reloj sólo se renueva sin trama en las dos entradas
deliberadas a todo-rojo (el rojo forzado y el arranque de modo), y vuelve a cero al reiniciar la conexión.

> 🔴 **LOS DOS UMBRALES SON EL MISMO NÚMERO Y LOS DOS INSTANTES NO.** Entre «la orden llega al Esclavo» y «su respuesta
> llega al Maestro» hay el retardo de cortesía (`SFTY-17`) más el viaje de vuelta, y **ese desfase era la ventana**:
> con la dirección Maestro→Esclavo muerta, el Poste 2 se iba a su ámbar **antes** de que el Poste 1 apagara su verde
> — verde contra ámbar con la pluma del otro poste arriba.

**Se cierra soltando el verde propio un margen ANTES** (`N-163`): el veto de margen adelanta la decisión en lo que dura
un acuse, que es lo que ya acota el viaje de ida y vuelta. Al soltarlo la punta cierra por su amarillo, marca que soltó
por margen y **sale a reposo** —no a una espera de acuse— para no tocar la cadencia del latido: medido, la otra salida
producía **más** ámbares en microcortes que se recuperan. **El umbral NO se baja**, y es condición del responsable.
**`SFTY6_SILENCIO_MS` = 28 s** (`D-48`; de 25 a 28 en `cda33df`, sin banco), para que quepa el amarillo del Esclavo
(`D-45`) con los cinco reintentos del ciclo; el presupuesto, en §2.2.bis y §9.
**La suelta cierra por el amarillo en las dos puntas y empieza un `AMARILLO_SEG` ANTES** (`D-45`), para que el ROJO
caiga donde caía el rojo directo: un amarillo contra el ámbar intermitente del otro poste es paso abierto. En el
Poste 1, `puedeSostenerVerde()` suma `TIMEOUT_ACK_MS + AMARILLO_MS` (la suelta a 21,5 s de silencio, el rojo a
24,5 s); en el Poste 2, `SUELTA_VERDE_MS = SFTY6_SILENCIO_MS - AVISO_AMBAR_TIMEOUT_MS - AMARILLO_SEG × 1000`, sólo con
la luz en `S_VERDE`.
**Lo que ese margen NO cubre —el verde de la OTRA punta cuando ésta ya cayó— es §9.** **Vuelta del enlace**
(`SFTY-9`): se reencolan la hora y la configuración, se fuerza rojo, se pide el rojo del otro lado y **se espera su
acuse** antes de contar despeje; tras una suelta por margen se reanuda por esa misma puerta, porque un todo-rojo de
minutos en una vía alternada no es estado seguro: es donde la gente se pasa el rojo.

🟢 **Y EL SENTIDO CONTRARIO —el verde del ESCLAVO frente al ámbar del Maestro— SE CIERRA IGUAL** (`D-34`). Las dos
anclas se desfasaban por el otro lado: ~~el Maestro cuenta desde lo último que OYÓ~~ → el Maestro cuenta desde la
última **respuesta** (arriba; hasta 1.49c valía cualquier trama) y el Esclavo refrescaba su silencio con cada
**repetición** de la orden de verde. Hoy, en las dos puntas:

- **Una repetición de la orden de verde no refresca el silencio del Esclavo**: sólo la que encuentra la luz fuera de
  verde y de amarillo, o sea la que abre.
- **El Esclavo suelta su verde un acuse y un `AMARILLO_SEG` ANTES de su silencio**, cierra por su amarillo y lo dice
  en el parámetro de su respuesta al latido; **no lo borra el latido**, sólo una orden de luz.
- **El Maestro no entrega la orden de verde sin una RESPUESTA de la otra punta en el último latido** —en esa espera el
  latido es simple, así que lo que abre la puerta es su `CMD_PONG`; hasta 1.49c valía cualquier trama—, así que el
  desfase entre anclas queda dentro de ese margen. El viaje de ida tiene que caber en la diferencia entre el plazo del acuse y
  la cadencia del latido: **premisa escrita junto a la constante, no medida con repetidor**.
- **Al oír ese aviso con el cruce quieto, el Maestro reanuda por la misma puerta de `N-163`**: rojo, acuse, despeje, y
  su propio verde.
- **Con la subida Esclavo→Maestro muerta y la bajada viva, la suelta del verde del Poste 2 la hace el Poste 1**
  (condición 1 del arquitecto, 02/10; construido, sin banco). Los latidos le renuevan el silencio al Esclavo y su
  propia suelta no actúa. Con el verde en el Poste 2 (`QV_ESCLAVO`, en reposo) y sin respuesta durante
  `SFTY6_SILENCIO_MS - TIMEOUT_ACK_MS - AMARILLO_MS` (21,5 s; `D-50`) —igual que `puedeSostenerVerde()`—, el Poste 1
  **pide su rojo en ese instante** (`CMD_GO_RED` ya; el latido lo repite a su cadencia mientras el rojo no conste),
  marca la suelta por margen y sale a reposo. **No entra en ámbar hasta que vence la ventana** `TIMEOUT_ACK_MS +
  AMARILLO_MS` desde esa orden, que por construcción acaba en el umbral: el ámbar sigue a `SFTY6_SILENCIO_MS` y el
  Poste 2 ya está en rojo. Antes el `GO_RED` salía al entrar en ámbar: un viaje en verde y 3 s de amarillo contra el
  ámbar. **No cubre** la pérdida de esos `GO_RED`: el Poste 2 sigue en verde hasta su orfandad, contada desde la
  última trama que le llegó, como antes. Lo ejerce `G1` del arnés de dos puntas, con el amarillo contado como paso.

**Lo que cuesta**: un microcorte que vuelve justo en ese margen da un rojo y un despeje de más, nunca un ámbar; y con
enlace malo la entrega del verde se retrasa un latido por cada respuesta perdida, sin pasar del umbral de silencio.
Lo ejercen las filas `G12`–`G14` del arnés de dos puntas, con controles negativos vistos fallar. **Sin banco.**

## 5. LAS BARRERAS QUE IMPIDEN VERDE + VERDE — la lista entera

1. **El Esclavo no enciende verde sin la orden de verde del Maestro** — fuera del Degradado no hay otra vía.
2. **El Maestro no enciende el suyo sin el acuse de rojo del otro lado** (la marca de rojo confirmado, §2.2).
3. **Ninguna punta abre NI SOSTIENE verde sin margen de silencio** — el veto de margen, en las cuatro puertas de
   apertura y en cada vuelta del coordinador. ⚠️ **Vigila el verde PROPIO, no el del otro** (§9).
4. **Backstop del Esclavo**: corta un verde eterno mirando **la luz**, no la orden, y lo cierra por el amarillo
   (`D-45`, `cda33df`).
5. **En Degradado, la fase la calcula la MISMA función en las dos puntas**, §8 (e).
6. Y la que no es de la radio: **sólo el fichero del semáforo escribe pines de luz** (`SFTY-2`, SPEC 1 §2).

## 6. EL ÁMBAR DE EMERGENCIA DEL POSTE 2, Y SU ACUSE

**Qué avería cierra:** roto **sólo el transmisor** del Poste 2 —oye pero no habla—, esa punta no puede saberlo (su único
dato de radio es el silencio de lo que RECIBE), el técnico veía un `$ACK` idéntico al de la radio sana y el Poste 1
seguía dando verde hacia un carril cuyo otro extremo está en ámbar. **Armar** (desde la app del Poste 2): ámbar aquí,
cerrojo puesto, aviso al Poste 1 y `$ACK` inmediato al teléfono — **el ámbar no espera al acuse** (`N-130`). **El
Poste 1** anota el aviso, contesta desde su vuelta de coordinador —con el Maestro **ciclando**, que es cuando tiene
permiso para hablar— y se va a modo Ámbar, del que **sólo sale si el ámbar lo pidió el Esclavo** y **sale a
todo-rojo**. *(Literales, SPEC 4 §3.2; el campo, SPEC 6.)*

> 🔴 **EL ACUSE SE ESPERA SÓLO DEL PRIMER AVISO, Y ESO ES LA REGLA, NO UNA SIMPLIFICACIÓN.** El aviso es lo que manda
> al Maestro a modo Ámbar, donde **tiene prohibido transmitir**, así que esperar acuse en cada pulsación diría «nadie
> me oyó» **con la radio sana** cada vez que alguien pulsa dos veces —lo normal en un poste—, y eso enseña al técnico
> a ignorar la única señal que esto existe para darle. **Plazo y reintento:** el plazo del aviso vive en la cabecera
> de protocolo y un `static_assert` del coordinador obliga a que sea el MISMO plazo que el del ciclo; el número de
> reintentos lo **eligió el responsable, no se deriva** (`D-32`). Agotado sale `$ALARM AVISO_RF / SIN_CONFIRMAR /
> AVISE_POSTE_1`, que **dice «no he podido confirmarlo», nunca «el otro poste no se enteró»**. **Cancelar** borra el
> cerrojo y **la memoria del acuse en la misma sentencia** —sin eso el siguiente ámbar diría «el Poste 1 ya lo sabe»
> sobre un acuse que ya no existe—, y **la cancelación NO se acusa**: su camino normal es con el Maestro ya callado en
> modo Ámbar, y su red es que la **segunda pulsación reenvía**.

## 7. EL MODO DEGRADADO

**Sin radio, la luz la decide el reloj** (`SFTY-21`).

> 🔴 **ENTRAR POR PRIMERA VEZ ES MANUAL** (salvo la opción automática de §7.ter, que un técnico activa antes).
> **REANUDAR TRAS UN CORTE NO LO ES — Y AL OPERARIO SE LE AVISA DEL CORTE, NO DE LA REANUDACIÓN.**
> - **La primera entrada sí la pide una persona, y hoy por el testigo** (`D-35`, §7.bis): la app ya no ofrece la de
>   aquí (`D-37`), y `SET_MODO:DEGRADADO` salió de las dos puntas con `D-46` (`f9cad1f`, sin banco): hoy responde
>   `COMANDO_NO_SOPORTADO`. El testigo (`SET_MODO:DEG_T`) es la única orden de entrada.
> - **La reanudación la decide la máquina**, en cada arranque y **sin pulsación ninguna** (`D-29`): cada punta tiene su
>   función de reanudar tras corte, llamada desde el arranque **y desde el bucle** —dos sitios, porque la hora llega
>   segundos después y la decisión queda diferida hasta que cierre su ventana—. El Esclavo entra por **la MISMA puerta
>   del operario**; el Maestro se pone en modo y cae en el reparto común. **No es un camino alternativo: es la entrada
>   de siempre con el permiso recuperado de la pila.**
> - **Lo que el operario NO ve:** un equipo al que se le fue la luz vuelve solo al único modo que da verde sin
>   confirmar con el otro extremo, y **ningún evento dice que REANUDÓ** (HUECO 6). Lo que sí sale: si el corte apagó
>   también el ESP32, la app abre al conectar el cartel de corte (`js/aviso_corte.js`, SPEC 4 §6) con el `MODO:` del
>   `$STATUS`. Si no reanuda, el Poste 1 arranca en **ámbar** (`D-40`, SPEC 1 §4.1).
> - **Lo que sí protege:** el permiso se tira si el equipo ya no está quieto donde lo dejó el arranque —una persona
>   eligió modo, o el modo ya gobierna—, si hay ámbar de emergencia de la app vigente, o si falló cualquier condición de
>   vigencia. Lo que se difiere es **el borrado, no el límite duro**; la ventana, en SPEC 3 §6 y H-3. Y **en las dos
>   puntas la decisión espera además a que el cristal tenga veredicto** (1.49b), dentro de esa misma ventana: SPEC 3 §6.

**Condiciones del Poste 2, y devuelve MOTIVO, no un sí/no** —quien la llame tiene que poder decirle al operario qué le
falta—: sin hora **fiable** (fiable, no sólo puesta) · sin configuración recibida · ciclo nulo · nunca hubo
sincronización · sincronización vencida · ámbar vigente; **los textos que LEE el operario y los tres motivos que no
existen en el Maestro, SPEC 6 A.2.** **Dentro:** todo-rojo de entrada con su suelo mínimo y después verde **sólo** en
la fase propia, con el despeje que mandó el Maestro **ya ampliado en el origen** —si cada punta lo escalara los verdes
se solaparían durante minutos—; **un salto de hora se aplica PASANDO POR ROJO** (`D-26`). **Las cuatro salidas, y
ninguna admite marcha atrás** (el indicador de la pila se baja **al empezar** la salida): el operario, o su
salida programada (§7.quater) · **el regreso
de la radio**, sólo con tramas de **gobierno** · **el límite duro** sin sincronización, con su aviso previo · y la
**la hora no fiable**. **El límite acaba en ámbar y sólo el Esclavo pasa por el despeje** (**SPEC 6 A.4**); **la hora
no fiable acaba en ROJO FIJO en las dos** (`D-38`, `irARojoSinHora()`; SPEC 3 §5). De ninguno de los dos se sale solo.
**La salida del operario en el Poste 1, HOY** (`f6ef03d`; construido, sin banco): `SET_MODO:MENU` y `FORZAR_ROJO`
(`D-51`, versión de la mañana del 04/10) hacen lo mismo, la salida por su todo-rojo (SPEC 4 §3.1). El rojo de
emergencia no arranca el amarillo dentro del modo ni manda `GO_RED` desde el Degradado: el ciclo por reloj volvía a
abrir el verde y ese `GO_RED` sacaba al Esclavo de su Degradado con el Maestro todavía en él.
**DEBE cambiar (responsable, 04/10 tarde; NO CONSTRUIDO, §7.quater):** `FORZAR_ROJO` dentro del Degradado es **rojo
fijo inmediato en ese poste** (`DEG_ROJO_SIN_HORA`, `D-38`) y deja de ser la salida; se sale con la salida programada
(`D-52`) o con `SET_MODO:MENU`, que sigue siendo la salida inmediata del poste conectado.
Al Esclavo lo saca el latido del Maestro ya fuera del modo, si hay radio; sin ella sigue en Degradado (§7.ter, punto 5)
hasta su salida programada (§7.quater).
**Desde `cda33df` (`N-174`, sin banco):** el rojo fijo sobrevive a un corte (`D-47`, `FLAG_ROJO_SIN_HORA`) y
alcanza también al reloj que se congela en marcha (`D-49`), que antes iba a ámbar; los dos, en SPEC 3 §5.
**Todo paso de verde a rojo del Degradado lleva su amarillo** (`D-45`), porque todos pasan por
`semaforo_forzarRojo()`: el fin de la fase propia (lo da la fase, §8 (e)), el salto de hora, la entrada con un verde
encendido, las cuatro salidas, el rojo fijo sin hora y la caída a ámbar (`irAAmbar()`: amarillo, el rojo y, un par de
segundos después, el parpadeo). Mientras dura el amarillo se renueva el reloj de esos todo-rojo (`tEstado`,
`tCambioEstado`), y la puerta del testigo rechaza también durante él. Las dos puntas abren su verde DIRECTO.

## 7.bis EL DEGRADADO CON TESTIGO — segunda puerta, para cuando la radio no vuelve en semanas (`D-35`)

> 🟢 **CONSTRUIDO en las dos puntas (`990c278`) y en la app (`11b57b3`); probado en banco por el funcional
> (`4a2c73c`).** No toca `modo_degradado_evaluarEntrada()` ni `degradado_entrar()` (`D-18`), que siguen exactamente
> como están — es una puerta NUEVA y PARALELA, para el caso en que el repuesto del radio tarda semanas y las condiciones
> de §8 —sync
> confirmada, desfase medido, ciclo acusado por radio— no se pueden cumplir porque no hay radio que las produzca.

**El comando, en las DOS puntas:** `SET_MODO:DEG_T:ahora,inicio,verde,despeje`
(`HH:MM:SS,HH:MM:SS,v,d`), con PIN. `ahora` es la hora del teléfono EN EL INSTANTE DE ENVIAR — no depende de que
haya habido un `SET_RTC` antes (abajo se explica por qué). `inicio` es la hora de arranque —hora del teléfono más
el traslado, 20 min por defecto—; `verde` viaja fijo en **180**; `despeje` es el que configuró el operario, con
suelo **30** y tope **255** (el byte entero, no los 10–90 de `DESPEJE_SEG_MIN/MAX` del ciclo automático: esta orden
usa SU PROPIO rango, `TESTIGO_DESPEJE_MIN/MAX`, porque un modo sin cámara ni radio que lo vigile pide más margen).
Longitud con PIN: `CMD:PIN:1234:SET_MODO:DEG_T:HH:MM:SS,HH:MM:SS,VVV,DDD` son **53 B** contra los 63 útiles de
`TRAMA_MAX_UTIL` (`ESP32_Expansion/include/contrato.h:64`, gemelo medido de `btBufIn[64]` en las dos puntas,
`Maestro/src/bluetooth.cpp:35` y `Esclavo/src/bluetooth.cpp:32`) — cabe con margen.

**Por qué `ahora` basta y no hace falta tocar el ESP32.** En vez de confiar en CUÁNDO llegó la última siembra,
el STM32 compara directamente `ahora` —la hora que el teléfono acaba de leer, dentro de esta misma orden— contra su
PROPIA hora (`reloj_segundosDelDia()`, ya existe en las dos puntas) y rechaza si difieren más de una tolerancia.
**Eso prueba que el reloj del poste coincide con el del teléfono EN ESE INSTANTE**, se haya hecho o no un `SET_RTC`
antes —la app lo sigue mandando primero, pero ya no es una condición de esta puerta—. No hay pieza nueva en el ESP32.

**La tolerancia: se reutiliza el criterio de `TOLERANCIA_DESFASE_S`, no se inventa uno.** Propuesta
`TOLERANCIA_TESTIGO_S = 3` (mismo valor, constante propia en cada `modo_degradado.cpp`, Maestro y Esclavo — la
original vive sólo en el Maestro, para el desfase de RADIO). Se deriva de la MISMA razón que ya está escrita en
`Maestro/src/modo_degradado.cpp:127`: *diez veces por debajo del todo-rojo más corto (30 s) y varias veces por
encima del sesgo conocido de una transmisión* —allí tiempo de aire de radio, aquí Bluetooth teléfono-poste, del
mismo orden de magnitud—. **Sin decidir por el responsable**, va como propuesta, no como cerrada.

**Maestro — entra en ROJO FIJO hasta `inicio`.** Función propuesta `modo_degradado_evaluarEntradaTestigo(ahora,
inicio, despeje)`, motivo nuevo (no confundir con `MotivoDegradado`, `Maestro/include/modo_degradado.h:32`):
`MDT_FALTA_HORA` (`reloj_horaFiable()` falso: sin una hora propia fiable no hay con qué comparar) ·
`MDT_AHORA_DESFASADO` (`|ahora − reloj_segundosDelDia()| > TOLERANCIA_TESTIGO_S`) · `MDT_DESPEJE_RANGO` (fuera de
30–255) · `MDT_INICIO_VENCIDO` (`inicio` ya pasó al llegar la orden) · `MDT_AMBAR_VIGENTE` (mismo veto que hoy,
`R-4`; **no** lo dispara el ámbar de arranque de `D-40`: `modo_ambar_esDeArranque()`, SPEC 1 §4.1). Aceptada,
fuerza rojo y queda esperando `inicio` con la MISMA máquina de estados (`DEG_ENTRANDO`), no una nueva: al llegar
`inicio` entra por la puerta de siempre, `ciclo_degradado_fase()`.

**Esclavo — la MISMA orden, y una tabla de rechazo distinta (como ya pasa con `D-18`, SPEC 6 A.2).** Motivo nuevo
`RechazoTestigo`: `DEG_RECHAZO_T_SIN_HORA` · `DEG_RECHAZO_T_AHORA_DESFASADO` · `DEG_RECHAZO_T_INICIO_VENCIDO` —
**ésta es la que muerde de verdad**: el operario se desplaza entre postes, y si `inicio` ya pasó cuando llega al
Esclavo, se rechaza con el texto «repita el testigo en el Maestro», exactamente como pide `D-35` — ·
`DEG_RECHAZO_T_DESPEJE_RANGO` · `DEG_RECHAZO_T_AMBAR_VIGENTE`.

**Vigencia y corte de luz — extiende `D-29`, no lo cambia.** Se persiste, en cada punta, un registro NUEVO junto a
los que ya usa `respaldo.cpp` (`respaldo_guardarCiclo()`, `respaldo_marcarSync()`): la hora de `inicio` (absoluta),
`verde`, `despeje`, y la marca del ÚLTIMO testigo aplicado en ESE poste —propuestos `respaldo_guardarTestigo(...)` /
`respaldo_horasDesdeTestigo()`, del mismo molde que `respaldo_marcarSync()`/`respaldo_horasDesdeSync()`, pero en su
PROPIO registro: no comparte el de la sincronización de radio, porque son dos relojes de vencimiento distintos —
**31 días** aquí, **48 h** en `D-18`—. Tras un corte, `degradado_reanudarTrasCorte()` gana una rama: si el permiso
persistido es de testigo, la puerta que comprueba es el **límite de 31 días**, no `LIMITE_SIN_SYNC_MS`; y si el
reinicio cayó ANTES de `inicio`, sigue en rojo fijo hasta esa hora — la reanudación no enciende nada que la entrada
no hubiera encendido ya.

**~~Vencimiento (31 días)~~ — derogado el 29/09 (§7.ter (d)).**
 🔴 Hoy cada punta sale a ámbar en SU marca: dos testigos puestos con
el traslado de por medio dan verde contra ámbar ese traslado. Lo sustituye la regla de medianoche de §7.ter (d).
Repetir el testigo —una vez al mes— reinicia la cuenta. **La deriva entre los DOS DS3231**, que es lo único que
corre sin radio y sin la vigilancia de §8, es del orden de **10 s/mes** (`DECISIONES.md` D-26, motivo: ±2 ppm cada uno)
— **cifra de decisión, no medida en tarjeta** — contra el suelo de **30 s** del
despeje de esta orden: el margen que queda es del orden de 20 s, y **nadie lo ha medido en un banco**.

**Lo que NO cambia, y se dice explícito (`D-35`):** la vuelta de la radio sigue exactamente como hoy —`SFTY-21`,
el latido del Maestro saca al Esclavo del Degradado con tramas de gobierno, arriba en este mismo §7—. ⬇️ ~~y
`SET_MODO:DEGRADADO` (`D-18`) sigue siendo el camino normal~~ → **salió con `D-46` (`f9cad1f`)**: el testigo es hoy
la única entrada por orden, haya o no sincronización de radio.

**Verificación de extremo a extremo en banco (nada de esto se prueba con un pack nuevo — CLAUDE.md §4, simulador
CONGELADO):** con las dos tarjetas reales y el cable de radio entre postes DESCONECTADO, mandar el testigo a las
dos por Bluetooth con el MISMO `verde`/`despeje`, leyendo `ahora` justo antes de cada envío; observar ROJO fijo en
las dos hasta `inicio`, y que al llegar `inicio` alternen sin solaparse (osciloscopio o cronómetro sobre las luces
reales) igual que en §8(e). Repetir cortando la alimentación de una tarjeta mientras espera `inicio`, y otra vez ya
alternando, y comprobar que reanuda sin encender un verde que no le tocaba.

**Ficheros e interfaces que toca:** `Maestro/{src,include}/modo_degradado.cpp,.h` y su
gemelo en `Esclavo/` (puerta nueva, enum nuevo, `TOLERANCIA_TESTIGO_S`) · `{Maestro,Esclavo}/src/bluetooth.cpp`
(rama `SET_MODO:DEG_T`, parsea las DOS horas) · `{Maestro,Esclavo}/{src,include}/respaldo.cpp,.h` (registro
nuevo). **No toca el ESP32** —ni `despachador.cpp` ni `siembra.cpp`— ni `reloj.cpp/.h`: la comparación usa
`reloj_segundosDelDia()`, que ya existe. `ciclo_degradado.h` NO se toca: la fase la calcula la MISMA función.
*(`D-45` sí lo tocó, en `cda33df`, §8 (e.bis): la función ganó el amarillo y el testigo lo hereda sin cambiar su
orden; en el banco de abajo se ve además el amarillo al final de cada verde.)*

**El testigo con el poste YA en Degradado** (la visita mensual, o un Degradado de `D-18` activo).
Pasa las mismas comprobaciones (`ahora`, `inicio`, rangos). Si `verde` y `despeje` son los que el poste ya aplica,
**renueva la cuenta de 31 días y la hora, y sigue alternando**: no vuelve a rojo ni espera `inicio`, porque la fase
es la de pared y no cambia. Si el ciclo es distinto, vuelve a ROJO fijo hasta `inicio` como una entrada nueva.

**El testigo se guarda en la ÚLTIMA PÁGINA DE LA FLASH del STM32, no en la pila.** Medido: los diez registros de
respaldo del F103 están ocupados y el registro mínimo (45 bits) no cabe sin retirar otras garantías. La página se
reserva en `platformio.ini` (`board_upload.maximum_size`) para que el enlazador no pueda poner código encima; se
escribe una vez por testigo, con el poste en ROJO, y lleva su propia suma. `respaldo.cpp` no cambia de formato.

**Reglas de construcción** (medidas sobre el fuente):
- **DR6/DR7:** son de `respaldo.cpp` y de nadie más: el firmware ya no usa la librería STM32duino RTC, que guardaba
  allí su fecha (`N-172`, SPEC 3 §6).
- **Los 31 días se cuentan con la fecha del DS3231** que trae `CMD:HORA_ESP32`, no con `reloj_contadorSegundos()`:
  sin `Y2` ese contador no sirve (desde `N-172` el firmware no lo reescribe nunca). Se permite tocar `reloj.cpp`
  sólo para guardar el día que trae la siembra.
- **`verde` distinto de 180:** rechazo de formato. **Medianoche:** `inicio` vencido si
  `(inicio − reloj) mod 86400 > 12 h`. **A los 28 días sólo avisa;** el ámbar llega a los 31.
- **El cerrojo de 48 h del Esclavo** (`syncVencidaLatch`) no actúa en modo testigo.
- **Ventana aceptada por `D-35`:** si el Esclavo rechaza por `INICIO_VENCIDO` y nadie vuelve al Maestro, a la hora de
  `inicio` el Maestro da verde por reloj contra el ámbar de huérfano del Esclavo. La app lo dice al rechazar: «vuelva
  al Maestro y póngalo en Automático o repita el testigo». **Ratificada el 30/09 tras la prueba de campo (`D-41`).**

**Lo que queda fuera de este apartado:** la pantalla de la app (SPEC 4 §3.ter) y la vista de campo (SPEC 6 A.1.bis).

**Packs que leen por FORMA las funciones de entrada de hoy, y que NO ven la puerta nueva porque tiene otro nombre**
(`grep -rln "modo_degradado_evaluarEntrada\|degradado_entrar" 01_Firmware/Simulaciones/banco/packs`):
`camara_02_j16.py` · `costura_06_reanudacion.py` · `esclavo_01_latch_ambar.py` · `esclavo_06_no_abre_paso.py` ·
`maestro_03_puerta_degradado.py` · `reloj_04_hora_que_caduca.py`. **Ninguno se toca ni se clona**: el simulador está
congelado (§4), y la verificación de esta puerta es de banco, no de pack.

## 7.ter EL DEGRADADO AUTOMÁTICO — entra solo tras 5 min sin radio, si el técnico lo dejó activado (`A-15`)

> **Construido** (`a0d605b`, `1a78873`; módulo `deg_auto.cpp` de las dos puntas), **sin banco**: el arnés de PC
> (bloque `H`) no es una tarjeta. **Deroga, por decisión del responsable:** SPEC 0 §5.7 «no entra en Degradado
> solo», la frase de
> `D-18` «la llave la tiene la app» y la de `D-21` «ni si vuelve a él». **Sigue decidiendo una persona, pero ANTES:**
> el técnico activa la opción con PIN en cada poste. **Con la opción apagada en cualquiera de los dos, el equipo hace
> exactamente lo de hoy** (§4 y §7: ámbar a los 28 s y vuelta sola al ciclo cuando vuelve la radio).
> **En carretera va APAGADA, como sale de fábrica (`D-43`)**: sin radio se usa el testigo (§7.bis) con el traslado
> real (15-20 min) en su campo de la app.

| símbolo | valor | por qué |
|---|---|---|
| `DEG_AUTO_ESPERA_MS` | 300000 (N = 5 min) | `A-15`, el responsable |
| `DEG_AUTO_MARCA_S` | 300 | la marca de inicio, múltiplo de 5 min del día de pared (`A-15`) |
| `DEG_AUTO_ROJO_MIN_S` | 420 | rojo fijo mínimo antes de `inicio`, (b) |
| verde / despeje | 180 / 30 | los del testigo: `DEG_VERDE_SEG`/`TESTIGO_VERDE_SEG` y `TESTIGO_DESPEJE_MIN` |
| `CMD_PRESENTE` | `0x17` | primer código libre de la cabecera de protocolo (la última es `CMD_ACK_AVISO_AMBAR` `0x16`) |
| `DEG_AUTO_APTO` / `DEG_AUTO_ECO` | `0x02` / `0x04` del param | `0x01` es `PONG_VERDE_SOLTADO` (`D-34`) |
| `FLAG_DEG_AUTO` / `FLAG_OTRO_APTO` | bit4 / bit5 de `REG_FLAGS` | libres; bit3 es `FLAG_TESTIGO` |
| `FLAG_RENDIDO` / `FLAG_APTO_DADO` | bit6 / bit7 de `REG_FLAGS` | la rendición sin intercambio sano después, (b); el `ECO` oído del otro, (a) |
| `PRESENTE_S` | 10 | Maestro en `s % 10 == 0`, Esclavo en `s % 10 == 5` (segundo de pared propio) |
| `DEG_AUTO_ACUSE_MS` | 10000 | tres latidos (`LATIDO_MS`) y margen |

**(a) La opción y el acuerdo por radio.** Cada punta publica, mientras hay enlace, si ELLA entraría: el bit `APTO`.
- **`APTO` = «mi puerta automática me aceptaría AHORA»** (`degAuto_aptoPropio()`): la puerta de (b) Y la MISMA
  función que comprueba el testigo al entrar (`modo_degradado_evaluarEntradaTestigo()` /
  `degradado_comprobarTestigo()`), con el `inicio` que usaría la entrada. No hay una lista aparte que pueda divergir.
- **Puerta del Maestro** = `FLAG_DEG_AUTO` · sin `FLAG_RENDIDO` · sin salida manual pendiente (b) · modo `AUTOMATICO`
  o `INTELIGENTE` · `modo_degradado_syncFresca()` (la sync del PAR: el Esclavo la acusó). Viaja en el param de **toda**
  `CMD_PING`, `CMD_GO_GREEN` y `CMD_GO_RED`. En `MENU`, `MANUAL`, `AMBAR` o `ALCANCE` vale 0: hay una persona.
- **Puerta del Esclavo** = `FLAG_DEG_AUTO` · sin `FLAG_RENDIDO` · sin salida manual pendiente · `DEG_INACTIVO`; la
  hora fiable con fecha y el ámbar de emergencia los mira la comprobación del testigo. Viaja en el `CMD_PONG`, junto a
  `PONG_VERDE_SOLTADO`, que el coordinador lee con máscara (`& PONG_VERDE_SOLTADO`, `D-34`). No exige sync propia: la
  del par la trae el `APTO` del Maestro, y tras un reinicio del Esclavo su sync sólo la fecha la pila.
- **La fecha del Esclavo con la radio mandando** (`reloj_fecharDesdeEsp32()`): de la trama `HORA_ESP32` de su ESP32 se
  toma sólo el DÍA, no la hora (`D-26` (3) intacta), anclado a la base de radio por el camino corto; si discrepa de
  esa base en más de 1 h (`FECHA_TOLERANCIA_S` = 3600) no se toca. Sin esto la puerta del Esclavo rechazaba `HORA`.
- **Cada punta guarda el último `APTO` que OYÓ del otro** en `FLAG_OTRO_APTO` (Maestro: de cada `PONG`; Esclavo: de
  cada `PING`/`GO_*`), escribiendo la pila sólo si cambia, y lo devuelve como `ECO` en las mismas tramas. Sin haberlo
  oído nunca, 0. **La opción EFECTIVA de una punta es su `FLAG_DEG_AUTO` Y su `FLAG_OTRO_APTO`**: con la opción en un
  solo poste, ése daría verde por reloj contra el ámbar del otro. **Y además el `ECO` en 1** (`FLAG_APTO_DADO`, en la
  pila): el otro me oyó `APTO`; si no, él no entra y ésta daría verde sola.
- **En el Maestro los bits van en un solo sitio:** `coordinador.cpp` lleva `#define protocolo_enviarPaquete
  degAuto_enviar`, que añade `APTO|ECO` a toda `PING`/`GO_*` del fichero. Los packs que buscan el envío por su nombre
  lo leen con `_alias` (`costura_06`, `costura_10`, `enlace_01`, `maestro_03`).
- **Pila:** los dos bits van en `REG_FLAGS`, dentro de la suma y sin cambiar `FIRMA` (el molde de `FLAG_TESTIGO`).
  Medido: `respaldo_guardarCiclo`, `respaldo_marcarSync` y `respaldo_guardarTestigo` escriben con OR y
  `respaldo_guardarDegradado` limpia sólo bit2 y bit3, así que los preservan; `respaldo_borrar()` los pone a 0 y sólo
  corre con contenido inválido (pila agotada, suma rota), donde 0 = OFF es lo correcto; `REINICIAR_RELOJ` del Maestro
  reinicia el dominio y también apaga. Funciones nuevas, **`respaldo.cpp` idéntico en las dos puntas**:
  `respaldo_guardarDegAuto(bool)` · `respaldo_degAuto()` · `respaldo_guardarOtroApto(bool)` · `respaldo_otroApto()`
  · `respaldo_guardarRendido(bool)` · `respaldo_rendido()` · `respaldo_guardarAptoDado(bool)` · `respaldo_aptoDado()`.
- **`SET_DEG_AUTO:1` / `SET_DEG_AUTO:0`, con PIN, en las dos puntas.** Se rechaza sin enlace (Maestro:
  `tieneComunicacion`; Esclavo: una orden de gobierno en los últimos `SFTY6_SILENCIO_MS`) y en `MODO_DEGRADADO`: la
  otra punta tiene que enterarse, o creería apta a ésta. Aceptada, cambia `FLAG_DEG_AUTO` y **el `$ACK` sale diferido,
  cuando llega el `ECO` que la refleja** (el molde diferido es `REINICIAR_RELOJ`, SPEC 4 §3.1). Literales en SPEC 4.
  **Con el Maestro en `MENU` no hay acuse:** su latido es `GO_RED`/`ACK_RED`, sin `PONG`, y el `ECO` sólo viaja en el
  intercambio `PING`/`PONG`; la orden, en cualquiera de las dos puntas, acaba en `$ERR ...SIN_ACUSE_DEL_OTRO_POSTE`
  a los `DEG_AUTO_ACUSE_MS` con el cambio hecho. Se activa con el Maestro ciclando (`AUTOMATICO`/`INTELIGENTE`).

**(b) La cuenta y la entrada.** Las dos puntas cuentan desde el último intercambio sano, no desde su ámbar:
- **Maestro:** `millis() − tUltimaRespuestaEsclavo ≥ DEG_AUTO_ESPERA_MS` (el ancla de §4; sin respuesta desde el
  arranque, desde el arranque). **No se mira `C_FALLO`**: también llega por reintentos agotados con enlace (§3).
- **Esclavo:** desde el último `CMD_PING` o `CMD_GO_GREEN` recibido (sin ninguno, desde el arranque). **`CMD_GO_RED` no
  cuenta:** el Maestro en `C_FALLO` sólo emite eso (§2.2); si reiniciara la cuenta, con la subida E→M muerta el
  Esclavo empezaría a contar cuando el Maestro ya calló, 300 s tarde. **No depende de `S_FALLO`**: en ese mismo corte
  el Esclavo está en rojo, no en ámbar, y cuenta igual.
- **Con la cuenta cumplida, opción EFECTIVA, `ECO` en 1 y la puerta abierta**, entra por la puerta del testigo
  (`modo_degradado_entrarTestigo()` / `degradado_entrarTestigo()`, §7.bis) sin teléfono: `ahora =
  reloj_segundosDelDia()`, `inicio` = la primera marca múltiplo de `DEG_AUTO_MARCA_S` con `(inicio − ahora) mod 86400 ≥
  DEG_AUTO_ROJO_MIN_S`, verde 180, despeje 30. Aceptada: `$EVENT ORIGEN:DEGRADADO
  DETALLE:AUTO_ENTRADA_INICIO_HH:MM:SS`. **Reinicio en pleno Degradado:** reanuda como el testigo (`FLAG_TESTIGO`
  y flash).
- **Un intento por corte** (`intentoHecho`): un rechazo de la puerta del testigo no entra, se publica UNA vez y no se
  reintenta hasta que la cuenta se rearma con una respuesta sana. `$ALARM DEGRADADO,CAUSA:AUTO_NO_<código>`, con el
  código igual en las dos puntas: `HORA` (sin hora fiable con fecha) · `DESFASE` · `DESPEJE` · `INICIO` · `AMBAR`
  (ámbar de emergencia puesto) · `EN_VERDE` · `GUARDADO` (la flash del testigo falló) · `OK` (respuesta inesperada).
  **`ACCION`:** `SIGUE_AMBAR`, salvo el Maestro con `GUARDADO`: `QUEDA_ROJO`, porque la puerta ya forzó el rojo y dejó
  el `MENU` (rojo fijo). El Esclavo con `GUARDADO` dice `SIGUE_AMBAR`: su rojo dura una vuelta y `main.cpp` lo
  devuelve a `S_FALLO` (arnés, fila `H12`). **Cota:** `CAUSA` ≤ 19 caracteres y `ACCION` ≤ 14, por buffer
  (`CLAUDE.md` §10): el `$ALARM` entero cabe así en su `payload` (Maestro 138, Esclavo 151; pack `esp32_07`).
- **La rendición y la salida manual cierran la puerta hasta un intercambio sano.** Una punta que se rinde en Degradado
  (Maestro al ámbar, Esclavo a `DEG_RENDIDO`) escribe `FLAG_RENDIDO` en la pila, que sobrevive a un reinicio; salir
  del Degradado a mano o tras rendirse levanta además una marca en RAM. Las dos las borra sólo un intercambio sano
  (Maestro: un `PONG`; Esclavo: un `PING` o `GO_GREEN`); la cuenta sola no reabre (arnés, fila `H11`).
- **Por qué 420 s, con cifras del fuente.** Es seguro si cada punta está ya en Degradado (rojo) antes del primer verde
  de la otra. (1) Desfase entre las dos cuentas ≤ **31 s**: el Maestro sólo emite `PING`/`GO_GREEN` con su última
  respuesta a menos de `SFTY6_SILENCIO_MS` (28 s desde `D-48`; eran 25), más un `LATIDO_MS` (3 s); en el otro
  sentido la última respuesta contesta a una orden que el Esclavo oyó. (2) Una punta que se reinicia durante su
  cuenta vuelve a contar desde el arranque: **+300 s**; la hora fiable llega en paralelo (`HORA_ESP32_CADENCIA_MS` =
  120 s del Esclavo). (3) Un
  despeje, **30 s**, para que el rojo del otro dure un todo-rojo entero. (4) El desfase de relojes que el cruce
  aguanta, **29 s** (`compilar_degradado.ps1`, fila D1). Suma **390 s** (387 antes de `D-48`); 420 deja 30 s para
  el arranque del micro. Recalculado a mano sobre `cda33df`, sin instrumento que lo relea.
- **Los dos `inicio` pueden caer en marcas distintas** (cuentas desfasadas o una entrada a cada lado de una marca, o
  de la medianoche) **y es seguro**: la fase la da `ciclo_degradado_fase()` sobre la hora de pared, y la punta que
  arranca una marca más tarde pasa esos 300 s en rojo. Lo único que se exige es la condición de arriba.
- **`D-45` no mueve los 420 s**: el amarillo va detrás de cada verde, y lo exigido es estar en rojo antes del
  primer verde del otro.

**(c) La radio en Degradado: saber que volvió, sin salir.** Trama nueva `CMD_PRESENTE`, param 0, que emite cada punta
en Degradado cada `PRESENTE_S` en su segundo desfasado (medio dúplex). Al oírla en Degradado: `$EVENT ORIGEN:DEGRADADO
DETALLE:ENLACE_DISPONIBLE` al recuperarla (se rearma tras 3 × `PRESENTE_S` sin oírla) y, **en Degradado, otra vez cada
`ENLACE_AVISO_REPETIR_MS` mientras se la oiga** (`oirPresente()` de `deg_auto.cpp`, las dos puntas): quien se conecta
después también se entera; la app lo junta en un cartel fijo (SPEC 4 §3.ter.bis). **No sale del modo:**
la salida sigue siendo el operario (Maestro: `SET_MODO:MENU`, y hoy también `FORZAR_ROJO`, que `D-51` corregida
convierte en rojo fijo, §7.quater; Esclavo: las tramas de gobierno del Maestro ya fuera de Degradado, §7) o, sin
radio, la salida programada de `D-52`.
**`CMD_PRESENTE` no llama a `reloj_notarRadio()`** (dejaría la hora del Esclavo esperando a la radio
en vez de a su ESP32), **ni refresca `tUltimoComando` ni `tUltimaRespuestaEsclavo`, ni pone `handshakeOk`, ni cuenta
para (b), ni saca del Degradado**: en el Esclavo se filtra ANTES de `reloj_notarRadio()`; en el Maestro, al principio
de la rama `if (llego)` de `coordinador_actualizar()`, que hoy da `handshakeOk = true` a cualquier comando que no
conoce. El Maestro en Degradado hoy no lee la radio (`main.cpp` no llama al coordinador): gana un lector propio, como
`coordinador_escucharEnAmbar()`, que consume y descarta todo salvo `CMD_PRESENTE`. **Oída por una punta NO degradada:**
`$ALARM DEGRADADO,CAUSA:OTRO_EN_DEGRADADO,...,ACCION:REVISE_OTRO`, sin tocar la luz. El aviso de (d) sale como
`$ALARM DEGRADADO,CAUSA:RENOVAR_TESTIGO,...,ACCION:REPITA_TESTIGO`.

**(d) Sin vencimiento (responsable, 29/09; deroga los 31 días de `D-35`).** El Degradado con testigo, manual o
automático, no vence ni cambia de luz por antigüedad. A los 28 días de la última marca de testigo, y después una vez
al día, cada punta manda `$ALARM` a la app pidiendo sincronizar desde el celular (repetir el testigo en los dos postes),
sin tocar luz ni modo. Riesgo aceptado: la deriva de los dos relojes (~10 s/mes, `D-26`, sin medir) contra el despeje.

**(e) Lo que publica.** El `$STATUS` del Maestro en `MODO_DEGRADADO` lleva **`ESC:?`**: `coordinador_estadoEsclavo()`
pregunta el modo antes que `quienVerde`, que hoy sigue publicando lo último conocido. `CONSULTA_DEG_AUTO`, sin PIN,
contesta el estado de la tarjeta de la app (SPEC 4 §3.ter.bis).

**(f) RIESGOS RESIDUALES — no se cierran por radio y los decide el responsable.** **Regla del responsable (29/09):
cambiar el modo de un poste en Degradado sin radio se hace con PALETEROS en los dos extremos, y la app lo avisa con una
confirmacion antes de enviar la orden (SPEC_4 §3.ter.bis).** En todos, una punta da verde por
reloj contra el ámbar de la otra hasta que alguien llega o vuelve la radio (entonces el Maestro fuera de Degradado
saca al Esclavo con su latido, §7).
1. **Durante el corte, el técnico pone el Maestro en `MANUAL`, `AMBAR` o `MENU`**, o el Esclavo en ámbar de emergencia:
   el otro conserva el último `APTO` oído (1) y entra. **Medido** (arnés, fila `H13`, nota que no falla): Maestro a
   `MANUAL` a los 100 s del corte, el Esclavo entra a los 300 s y el Maestro nunca; en los 1.800 s de la ventana del
   arnés, **528 s de verde del Esclavo contra el ámbar del Maestro** (verde contra verde, 0). En campo sigue hasta que
   alguien llega o vuelve la radio. Lo cubre sólo la regla de paleteros: el aviso de la app (`js/aviso_degradado.js`)
   salta con el poste conectado en `MODO:DEGRADADO`, y aquí el Maestro aún no lo está.
2. **Se va la luz del Maestro durante la cuenta:** arranca en ámbar (`D-40`, SPEC 1 §4.1) y no entra; el Esclavo sí.
3. **La flash del testigo falla** (`MDT_NO_GUARDADO`): esa punta no entra (el Maestro va a `MENU`); la otra, sí.
4. **El Esclavo se reinicia en la cuenta con la pila inválida** (sin la opción) o sin hora fiable en 420 s.
5. **`SET_MODO:MENU` en el Maestro sin radio** deja al Esclavo en Degradado: como hoy con `D-18` y `D-35`.
   **El hueco lo cierra la salida programada** (`D-52`, §7.quater, NO CONSTRUIDA): la misma hora en los dos postes,
   cada uno sale por su reloj y sin radio. `SET_MODO:MENU` queda como salida inmediata del poste conectado, con el
   aviso de paleteros; programar la salida en uno solo reabre este mismo riesgo (§7.quater (h)).
6. **Una punta pierde la hora fiable y pasa a ROJO FIJO; la otra sigue alternando por reloj** (`D-38`, construido
   sin banco: `irARojoSinHora()` y `DEG_ROJO_SIN_HORA` en los dos `modo_degradado.cpp`). Peor caso, verde contra rojo:
   ese sentido queda cerrado hasta que llegue alguien. Sale `$ALARM ...EVENTO:DEGRADADO,CAUSA:ROJO_SIN_HORA,...,
   ACCION:ROJO_FIJO` cada `AVISO_ROJO_SIN_HORA_MS`, y si la hora caducó, también la de `HORA_ESP32` con
   `ACCION:CAMBIO_A_ROJO`. Una siembra no lo devuelve: Maestro, `SET_MODO:MENU`; Esclavo, las tramas de gobierno del
   Maestro fuera de Degradado o `AMBAR_EMERGENCIA`. Incluye que el APTO del Maestro caiga por tiempo durante la cuenta
   (`SYNC_FRESCA_MS`); la sync se renueva cada hora, así que es raro. ~~Riesgo abierto: esa punta en rojo
   fijo arranca en ámbar tras un corte~~ → **`D-47` (`cda33df`, sin banco): arranca en el mismo rojo fijo** (SPEC 1
   §4.1); el arranque del Poste 1 no lo ejerce ningún arnés.

**(g) Ficheros e interfaces.** La lógica nueva va en un **módulo pareado** `{Maestro,Esclavo}/{src,include}/
deg_auto.cpp,.h` (los ficheros que tocaría pasan de 500 líneas): `degAuto_loop()` (cuenta, entrada, emisión de
`PRESENTE`, `$ACK` diferido) · bits `APTO|ECO`: `degAuto_enviar(cmd, param)` en el Maestro (el `#define` de (a)) y
`degAuto_paramSaliente(cmd)` en el Esclavo · `degAuto_alRecibir(const RF_Packet*)` (copia, cuenta del Esclavo;
devuelve true si consumió un `PRESENTE`) · `degAuto_escucharEnDegradado()` (lector del Maestro en Degradado) ·
`degAuto_orden(bool)`, `degAuto_veredicto()` y `degAuto_aptoPropio()` (Bluetooth: la consulta la arma
`bluetooth.cpp`). Ganchos de una línea: `coordinador.cpp` (param de
`PING`/`GO_*`, máscara del `PONG`, filtro de `PRESENTE`, `ESC:?`, acceso a `tUltimaRespuestaEsclavo`), `Esclavo/src/
main.cpp` (param del `PONG`, filtro antes de `reloj_notarRadio()`), `Maestro/src/main.cpp` (lector en Degradado),
`{Maestro,Esclavo}/src/bluetooth.cpp`, `respaldo.cpp/.h`, `modo_degradado.cpp` de las dos (vencimiento de (d)) y
`protocolo.h` de las dos (`CMD_PRESENTE`, dos bits: la cabecera sigue idéntica). **Queda fuera:** la salida
automática al volver la radio (el funcional pide que no), los interruptores de cámara y barrera (SPEC 8 §6) y el ESP32.

**(h) Verificación de extremo a extremo (banco, dos tarjetas, luces reales cronometradas en las dos).** En todas:
**nunca verde contra ámbar ni verde contra verde**.
1. Opción ON en los dos, ciclo en `AUTOMATICO`, radio cortada: ámbar a ~28 s, a los 5 min rojo fijo en los dos,
   alternan desde la marca; con la opción ON en uno solo, ámbar indefinido como hoy.
2. Corte sólo M→E y corte sólo E→M (una antena o un TX desconectado): entran las dos y cada una antes del primer verde
   de la otra; en E→M el Esclavo pasa la cuenta en rojo.
3. Reinicio del Esclavo en pleno corte (antes de entrar y ya dentro). Maestro en `MANUAL` durante el corte: es el
   riesgo (f).1, la única fila donde se ESPERA verde contra ámbar; se mide y se anota cuánto dura.
4. `SET_DEG_AUTO:0` sin radio: rechazado. Con radio: `$ACK` sólo tras el `ECO`. Datos asimétricos por bit: cada
   condición de `APTO` falsa por turnos en cada punta, y ninguna entra.
5. Radio de vuelta con los dos en Degradado: `ENLACE_DISPONIBLE` en la app de cada poste, siguen en Degradado; `MENU`
   en el Maestro saca a los dos. `PRESENTE` con un poste no degradado: alarma, la luz no cambia.
6. Entrada a las 23:55 (el `inicio` cruza la medianoche). Y testigo de más de 31 días: sigue alternando (no vence).

**Arnés de PC** (antes del banco, visto en rojo con un defecto inyectado en el `.cpp` real): bloque `H` nuevo en
`Validacion_Automatico/dos_puntas/orquestador_degradado.cpp` (fila 19), con un corte de radio por SENTIDO en el propio
orquestador y mandos nuevos en los adaptadores (`punta_mando("deg_auto", 0|1)`, `punta_mando("deg_auto_apto", 0)`).
Asevera en cada milisegundo de los escenarios 1–4 que no hay verde contra ámbar ni verde contra verde (el detector
`hayVerdeSimultaneo()` y su gemelo verde/ámbar); controles negativos: `DEG_AUTO_ROJO_MIN_S` a 0 y el `APTO` del
Esclavo forzado a 1 deben dar FALLA. El simulador sigue congelado: no hay pack nuevo (`CLAUDE.md` §4).

## 7.quater LA SALIDA PROGRAMADA Y EL ROJO TOTAL DEL DEGRADADO (`D-52`, `D-51` corregida) — NO CONSTRUIDO

> 🔴 **Decidido por el responsable el 04/10 (tarde); nada de este apartado está en el fuente.** Lo que el equipo HACE
> hoy y lo que DEBE hacer van separados en cada punto. Escrito desde el fuente (`Maestro/src/modo_degradado.cpp`,
> `bluetooth.cpp`, `main.cpp`, `Esclavo/src/modo_degradado.cpp`) y el veredicto del arquitecto del 04/10 (condiciones
> 1, 2 y 4). Deroga: de `D-51` la salida por todo-rojo; de §7.ter (f).5 el hueco «sin radio».

**(a) Lo que hace hoy (`f6ef03d`).** Maestro: `modo_degradado_forzarRojo()` en `MODO_DEGRADADO` llama a
`modo_degradado_pedirSalida()` (acuse `SALIENDO_TODO_ROJO` / `SALIDA_YA_EN_CURSO`); fuera del modo, a
`coordinador_forzarRojoTotal()` y `RESULT:OK`. **El rojo total NO revoca el permiso de `D-29`:** con el Maestro en
`MENU` esperando la siembra (`main.cpp`, rama `respaldo_degradadoActivo()`) acusa `OK`, y al llegar la siembra
`modo_degradado_reanudarTrasCorte()` reabre el Degradado y su verde por reloj (arquitecto, cond. 2). **Leído en el
fuente al escribir esto (`menu_setup()` no toca la pila), no estaba en su lista:** `SET_MODO:MENU` con el Maestro ya en
`MENU` tampoco lo borra. Esclavo: no tiene `FORZAR_ROJO` (`$ERR RENOMBRADO_USE_AMBAR_EMERGENCIA`) ni menú; sale con
`AMBAR_EMERGENCIA` (`degradado_salir()`, acaba en ámbar) o con las tramas de gobierno del Maestro. La app manda
`FORZAR_ROJO` sin aviso de paleteros (`aviso_degradado.js`, `aplica()` solo mira `SET_MODO:*`) y dice «no vuelve a
alternar».

**(b) El rojo total (`D-51` corregida). DEBE:**
- **`FORZAR_ROJO` con el Maestro en `MODO_DEGRADADO` = rojo fijo inmediato en ESE poste**, con el estado que ya existe
  (`DEG_ROJO_SIN_HORA`, `D-38`): amarillo de 3 s si estaba en verde (`D-45`) y rojo sostenido. **Ya no es la salida.**
  El otro poste sigue por reloj: verde contra rojo no cruza a nadie (el peor caso de `D-38`, sentido cerrado). «De
  ninguno de los dos se sale solo» (§7): se sale con la salida programada (c) o con `SET_MODO:MENU` (su todo-rojo de
  siempre).
- **Marca `rojoPorOrden`** en ese estado: solo cambia la CAUSA de la alarma, no la luz ni la salida. Publica `$ALARM
  DEGRADADO,CAUSA:ROJO_TOTAL,...,ACCION:ROJO_FIJO`, repetida cada `AVISO_ROJO_SIN_HORA_MS` (recuerda que el otro poste
  alterna). Con `ROJO_SIN_HORA` la app mandaría a mirar el reloj por algo que hizo una persona.
- **Sobrevive a un corte reutilizando la bandera de `D-47`** (`FLAG_ROJO_SIN_HORA`, `respaldo_guardarRojoSinHora()`),
  sin tocar `respaldo.cpp`: arranca en rojo fijo y no reanuda. `rojoPorOrden` es de RAM y NO sobrevive: **tras el corte
  la causa publicada es `ROJO_SIN_HORA`** (`modo_degradado_arrancarEnRojoSinHora()`). Se acepta. Baja el permiso de
  `D-29` (`respaldo_guardarDegradado(false)`, ya lo hace `irARojoSinHora()`).
- **En la ventana de `D-29`** (Maestro en `MENU`, permiso esperando la siembra) **`FORZAR_ROJO` revoca el permiso y
  entra en ROJO FIJO**, en `MODO_DEGRADADO`/`DEG_ROJO_SIN_HORA` con `rojoPorOrden`, como el arranque de `D-47`
  (`modo_degradado_arrancarEnRojoSinHora()`); **nunca `entrarAmbarDeArranque()` ni `C_IDLE` que acaba en ámbar**
  (`main.cpp`: `bluetooth_loop()` corre antes y `esperaReanudacion && !respaldo_degradadoActivo()` llevaría al ámbar).
  Criterio del responsable: **el rojo total nunca deja ámbar.** Se apaga `esperaReanudacion` y `reanudacionPorDecidir`.
  Después de un rojo total **no puede reanudarse el verde por reloj.** `SET_MODO:MENU` en esa ventana también revoca el
  permiso (`respaldo_guardarDegradado(false)`, `reanudacionPorDecidir = false`, fin de `esperaReanudacion`) y se queda
  en `MENU`. `DEG_FIN` en la ventana se rechaza «No esta en Degradado»: solo `FORZAR_ROJO` y `SET_MODO:MENU` revocan.
- **Respuestas de `FORZAR_ROJO` (sin PIN y con PIN, mismas) — cond. 4 del arquitecto.**
  `RESULT:ROJO_FIJO_EN_ESTE_POSTE` (entró en rojo fijo, desde `DEG_ENTRADA_ROJO`, `DEG_ACTIVO`, `DEG_AMBAR` o la ventana
  de `D-29`) · `RESULT:YA_EN_ROJO_FIJO` (ya estaba en `DEG_ROJO_SIN_HORA`, por hora perdida o por otra orden: nada
  cambia) · `RESULT:SALIDA_YA_EN_CURSO` (en `DEG_SALIDA_ROJO` o `DEG_RECHAZO`: ya está en rojo y va al menú) ·
  `RESULT:OK` (fuera del modo y sin permiso: `D-45`, sin cambio). **Desaparece `SALIENDO_TODO_ROJO` de esta orden.**
  **`SET_MODO:MENU`** gana `RESULT:OK_REANUDACION_CANCELADA` (en la ventana, con permiso revocado; `RESULT:OK` sin
  permiso). Cada literal sale de lo que devolvió la función, no de la rama.
- **Esclavo: sin cambio.** No tiene rojo total; su salida inmediata sigue siendo `AMBAR_EMERGENCIA`, que acaba en
  ámbar (asimetría que esta decisión no toca; riesgo (h).7).
- **Aviso de paleteros: `FORZAR_ROJO` NO lo lleva — DECIDIDO** (responsable; el rojo no cruza a nadie, «lo seguro,
  fácil»). El TEXTO de la app sí cambia (SPEC 4 §3.1): «este poste en ROJO FIJO; el OTRO POSTE SIGUE ALTERNANDO, vaya a
  él». `SET_MODO:MENU` y la salida programada SÍ llevan el aviso (`aplica()` cubre cualquier `SET_MODO:*`).
**(c) La orden: `SET_MODO:DEG_FIN:ahora,salida` (`HH:MM:SS,HH:MM:SS`), con PIN, en las DOS puntas.** Nombre: sin choque
(`grep DEG_FIN` en el repo: cero; el prefijo `SET_MODO:DEG_T:` de 15 caracteres difiere en el 14.º). Con PIN son **47
B** de los 63 útiles de `TRAMA_MAX_UTIL`. Cancelar: `SET_MODO:DEG_FIN:CANCELAR`. Consulta, sin PIN (molde
`CONSULTA_DEG_AUTO`, y entra en la lista de órdenes sin PIN de las dos puntas): `CONSULTA_DEG_FIN`.
- **Qué programa:** la hora (del día) en la que ESTA punta sale del Degradado por su reloj y sin radio. La misma en
  cada poste, puesta desde la app, configurable como `inicio` (por defecto `ahora` + el traslado).
- **A la hora `salida`**, en `DEG_ENTRADA_ROJO`, `DEG_ACTIVO` o `DEG_ROJO_SIN_HORA` (con hora fiable): la salida de
  siempre, `modo_degradado_pedirSalida()` en el Maestro y `iniciarSalida(false)` en el Esclavo: rojo (con su amarillo si
  estaba en verde), el despeje en uso (mínimo 30 s), y baja el permiso de la pila AL EMPEZAR. Dispara con
  `reloj_segundosDesde2000() >= salidaS` y `reloj_horaFiable()`; con hora no fiable no dispara (rojo fijo; sale con
  `SET_MODO:MENU` o cuando la hora vuelva). Publica `$EVENT DEGRADADO,SALIDA_PROGRAMADA_EJECUTADA`. El disparo va
  del `switch` de `modo_degradado_loop()`: `DEG_ROJO_SIN_HORA` sale de él por un `return`.
- **Vida de `salidaS` y `rojoPorOrden` (RAM). Se ponen a cero** al ejecutarse CUALQUIER salida (la programada,
  `SET_MODO:MENU`, la rendición o el ámbar del límite, el regreso de la radio) y al ENTRAR (`DEG_T` aceptado o
  renovado, y la entrada automática por `entrarTestigo`). Así tras salir se puede volver a entrar y programar; el
  rechazo de `DEG_T` con salida pendiente solo vale DENTRO del modo.
- **Validaciones, en este orden, un `$ERR ... DESC:<texto>` por motivo** (`MotivoSalida`, `MDF_*`, el mismo en las dos
  puntas; firmas en (g)): `FORMATO_INVALIDO` (`sscanf` con `%c` centinela y horas válidas,
  en `bluetooth.cpp`) · «No esta en Degradado» (otro modo, o `DEG_RECHAZO`/`DEG_AMBAR`/`DEG_RENDIDO`: no gobiernan) ·
  «Ya esta saliendo» (`DEG_SALIDA_ROJO`/`DEG_SALIENDO`) · «Falta: reloj sin poner en hora» (`reloj_horaFiable()`) ·
  «Ahora no coincide» (`|ahora − reloj_segundosDelDia()| > TOLERANCIA_TESTIGO_S`, 3 s, por el camino corto del día) ·
  «Salida ya vencida» (`(salida − reloj) mod 86400 > SALIDA_MAX_S` = 12 h, como `TESTIGO_INICIO_MAX_S`; en el Esclavo es
  la que muerde, con el texto «reprograme la salida en el Maestro») · «Salida antes del inicio» (en la cuenta de
  entrada, `salidaS <= testigoInicioS`) · «En verde: repita en rojo» (la flash solo se escribe con el poste en rojo, y
  se mira la LUZ, `semaforo_estado()`, no la fase) · «No se pudo guardar la salida» (la flash falló: NO queda
  programada; si el borrado salió bien y falló la escritura se pierde TAMBIÉN el registro del testigo, riesgo (h).3).
  `CANCELAR` sin nada programado: «No hay salida programada».
- **`$ACK` solo si quedó programada, y sale de lo que devolvió la llamada** (molde `SET_TIEMPOS`, `CLAUDE.md` §2):
  `RESULT:PROGRAMADA` · `RESULT:REPROGRAMADA` (había otra; esta la sustituye) · `RESULT:PROGRAMADA_SIN_RESPALDO` (en
  rojo total o sin registro de testigo: no hay flash que la guarde, un corte la pierde; (d)) · `RESULT:CANCELADA`.
- **Reprogramar** = mandar otra: pasa las mismas validaciones y sustituye. **Cancelar** pasa por la flash, así que
  también pide el poste en rojo. Las dos llevan el aviso de paleteros: un poste que cancela mientras el otro sale es el
  riesgo (h).1.
- **`SET_MODO:DEG_T` con una salida programada se rechaza** («Cancele antes la salida programada», Maestro y Esclavo):
  ni renueva ni entra de nuevo en silencio sobre una salida que el operario ya fijó.
- **Publicación.** El contador NO cabe en `$STATUS`: su `payload[155]` está a 2 B del borde del cable
  (`bluetooth.cpp`, comentario del `$EVENT` de diagnóstico) y `documentos_03` exige el campo en las dos puntas. Va como
  el resto de lo que pasa pocas veces: `$EVENT DEGRADADO,SALIDA_PROGRAMADA_HH:MM:SS` al aceptarla y `CONSULTA_DEG_FIN` →
  `$ACK,CMD:CONSULTA_DEG_FIN,RESULT:NINGUNA` o `RESULT:SALE_HHMMSS_FALTAN_<s>S_<RESPALDADA|SIN_RESPALDO>` (sin `:`
  dentro del valor, para el parser de la app). La app la pide al conectar y tras cada orden y cuenta hacia atrás sola.

**(d) Corte de luz (el testigo en flash).** El registro de `testigo_flash.h` gana `salidaS` (s desde 2000; 0 = ninguna)
y sube de versión: una página de la versión vieja se lee como ausente, y `-e all` (`CLAUDE.md` §3) la borra en cada
carga. Se escribe con la salida programada, reprogramada o cancelada, **siempre con el poste en rojo**. Tras un corte,
en `modo_degradado_reanudarTrasCorte()` y `reanudarTestigo()`: `salidaS != 0` y `ahora >= salidaS` → **no reanuda** y
borra el permiso (el poste arranca en `MENU`/ámbar de arranque `D-40`; el Esclavo, en orfandad); `ahora < salidaS` →
reanuda como testigo con la salida restaurada en RAM, y sale a su hora. La salida iniciada ya bajó el permiso: un corte
durante su todo-rojo arranca en el menú. **En rojo fijo no hay respaldo** (el permiso está bajado, la flash no se lee):
un corte pierde la salida y el poste queda en rojo fijo hasta `SET_MODO:MENU`; lado seguro, pero el otro habrá salido.
Un Degradado sin registro de testigo (el de `D-18` reanudado; casi inalcanzable desde `D-46`) también la pierde, y SÍ
reanuda: alterna contra el otro, que ya salió.

**(e) Con radio y sin ella.** Cada poste sale a su hora **por su reloj, no por el latido**: con radio, si el Maestro
sale primero, su latido ya fuera del modo puede sacar al Esclavo antes de su hora (§7); el resultado es el mismo.
**Estado final sin radio: los dos en ÁMBAR INTERMITENTE a la vez, leído en el fuente, no medido.** Maestro: `MENU`;
`coordinador_actualizar()` (`C_MENU_IDLE`, sin respuesta del Esclavo en `SFTY6_SILENCIO_MS`) enciende el ámbar. **El
Esclavo no tiene menú:** pasa a `DEG_INACTIVO` y `main.cpp` lo lleva al ámbar por silencio (`tUltimoComando`, que en
Degradado no se refresca). Con radio: los dos terminan en rojo fijo cuando el Maestro (`MENU`) manda `GO_RED`; hasta la
primera respuesta, ámbar. El encargo decía «los dos en menú»: solo el Maestro lo está.

**(f) Relojes desfasados.** Con `δ` = desfase entre los dos relojes al llegar a `salida`, el cruce está cerrado si `δ +
AMARILLO_MS (3 s) < despeje en uso` (≥ 30 s): el que sale primero está en rojo y llega a ámbar `despeje` s después; el
que sale último cierra su verde con 3 s de amarillo. **`δ` lo acota la programación**: cada punta comprueba `ahora`
contra su reloj con ±3 s, luego `δ ≤ 6 s` + la deriva en ≤ 12 h (≈ 0,2 s con ±2 ppm cada uno); hay ~21 s de margen. No
cuenta la deriva mensual de `D-26`. **Escrito desde el fuente, no medido en tarjeta.**

**(g) Ficheros e interfaces.** `{Maestro,Esclavo}/src/modo_degradado.cpp,.h` (`rojoPorOrden`, `salidaS`, la orden de
programar/cancelar/consultar, el disparo en el bucle, la rama de reanudación) · `{Maestro,Esclavo}/src/bluetooth.cpp`
(`SET_MODO:DEG_FIN`, `CONSULTA_DEG_FIN` y su entrada en la lista sin PIN, las respuestas de `FORZAR_ROJO` en las dos
ramas, el rechazo de `DEG_T`) · `{Maestro,Esclavo}/{src,include}/testigo_flash.cpp,.h` (campo y versión) ·
`Maestro/src/main.cpp` (revocar la espera) · `Maestro/src/modo_degradado.cpp` (`modo_degradado_forzarRojo()`: cinco
códigos). App: SPEC 4 §3.1-§3.ter.ter. **No toca** el ESP32, `reloj.cpp`, `respaldo.cpp` ni `ciclo_degradado.h`. Hasta
construirla, `decisiones_01_anclas` ve `D-52` sin ancla: es el rojo que dice «sin construir» (`CLAUDE.md` §1).

**Firmas (las fija la SPEC; el constructor no las elige).** En `modo_degradado.h` de las DOS puntas, salvo lo marcado:
- `enum MotivoSalida : uint8_t { MDF_PROGRAMADA, MDF_REPROGRAMADA, MDF_PROGRAMADA_SIN_RESPALDO, MDF_CANCELADA,
  MDF_NO_EN_DEGRADADO, MDF_YA_SALIENDO, MDF_FALTA_HORA, MDF_AHORA_DESFASADO, MDF_SALIDA_VENCIDA,
  MDF_ANTES_DEL_INICIO, MDF_EN_VERDE, MDF_NO_GUARDADO, MDF_NADA_QUE_CANCELAR };` en el orden de (c).
- `MotivoSalida modo_degradado_programarSalida(uint32_t ahoraDia, uint32_t salidaDia);` (segundos del dia) ·
  `MotivoSalida modo_degradado_cancelarSalida();` · `const char* modo_degradado_textoSalida(MotivoSalida m);` (los
  textos de (c); los cuatro primeros son `$ACK`, el resto `$ERR`) · `uint32_t modo_degradado_salidaS();` (0 = ninguna)
  · `bool modo_degradado_salidaRespaldada();`.
- Solo Maestro: `enum ResultadoRojoTotal : uint8_t { RRT_OK = 0, RRT_ROJO_FIJO = 1, RRT_YA_EN_ROJO_FIJO = 2,
  RRT_SALIDA_EN_CURSO = 3, RRT_REANUDACION_CANCELADA = 4 };` -> `RESULT:OK` · `ROJO_FIJO_EN_ESTE_POSTE` ·
  `YA_EN_ROJO_FIJO` · `SALIDA_YA_EN_CURSO` · `OK_REANUDACION_CANCELADA`, y `ResultadoRojoTotal
  modo_degradado_forzarRojo();` (sustituye al `uint8_t` de hoy).
- Solo Maestro: `bool modo_degradado_revocarEsperaSiembra();` true si habia permiso esperando la siembra (`D-29`) y
  lo bajo. Lo llaman `modo_degradado_forzarRojo()` (y entra en rojo fijo, `RRT_REANUDACION_CANCELADA`) y
  `SET_MODO:MENU` (`RESULT:OK_REANUDACION_CANCELADA` si devolvio true, `RESULT:OK` si no).
- J3 tolera hasta 1000 ms sin rojo en el Maestro: una vuelta de arranque antes de entrar en rojo fijo.

**(h) RIESGOS PARA EL RESPONSABLE — no se resuelven aquí.**
1. **Salida programada en un poste y no en el otro** (o cancelada en uno): el que sale queda en `MENU`/ámbar sin
   radio y el otro sigue con verde por reloj, hasta que alguien llegue: verde contra ámbar, el riesgo de (f).1. El
   firmware no puede saberlo sin radio. Lo mitigan la tarjeta de la app («salida programada en el Maestro, falta el
   Esclavo») y `CONSULTA_DEG_FIN`; no lo cierran.
2. **`SALIDA_VENCIDA` en el segundo poste** (traslado agotado): lo deja sin programar con el primero en cuenta atrás.
   La salida es reprogramar en el primero una hora posterior o cancelarlo (en rojo); la app lo dice al rechazar.
3. **Flash solo con el poste en rojo:** programar, reprogramar o cancelar con el poste en verde rechaza `En verde:
   repita en rojo`; el rojo dura más de 240 s por ciclo (verde del otro más dos despejes), pero el operario debe
   acertarlo. Una segunda página no hace falta: el registro pasa de 7 a 9 medias palabras dentro de su página de 1 KB;
   flash 66,0 % / 58,9 % en el acta del 04/10. **Si el borrado sale bien y falla la escritura se pierde TAMBIÉN el
   registro del testigo** y, tras un corte, no reanuda (ámbar contra el otro, como (f).3); hoy pasa igual al renovar
   el testigo, y esto añade tres órdenes que escriben la flash.
4. **Un corte entre programar y `salida`** se cubre con la flash; en rojo total no (d). Y un fallo de la flash deja
   el poste sin salida con un `$ERR` que el operario puede no leer.
5. **Rojo total y luego salida:** funciona (de `DEG_ROJO_SIN_HORA` por `pedirSalida()`), pero sale al menú sin radio
   → ámbar; si el otro poste no salió, ese ámbar es de nuevo (h).1.
6. **`SET_MODO:MENU` y `FORZAR_ROJO` en la ventana de `D-29`** revocan el permiso (decidido por el responsable): el
   primero cambia lo que hoy hace; el `$ACK` de MENU lo dice (`OK_REANUDACION_CANCELADA`).
7. **Asimetría del rojo inmediato:** Maestro = rojo fijo; Esclavo = `AMBAR_EMERGENCIA` (sale por todo-rojo y acaba en
   ámbar). Un técnico que para los dos con el botón de cada pantalla deja uno en rojo y otro en ámbar.
8. **Sin cuenta atrás en `$STATUS`** (el cable no admite un campo más): solo `CONSULTA_DEG_FIN` y el `$EVENT`.
9. **Entra «su cuenta de entrada» solo como `DEG_ENTRADA_ROJO`/`DEG_ENTRANDO`** (esperando `inicio`). La cuenta de
   300-420 s del automático (§7.ter) no es Degradado todavía: se rechaza con «No esta en Degradado».

**(i) Verificación de extremo a extremo.** Banco, dos tarjetas, radio desconectada: (1) testigo y, ya alternando,
`SET_MODO:DEG_FIN` a las dos con la misma `salida` y `ahora` leído antes de cada envío (aviso de paleteros primero);
cronometrar las luces: a `salida` rojo en las dos (amarillo si venían en verde), tras el despeje ámbar intermitente en
las dos, **nunca verde contra ámbar ni verde contra verde**. (2) Corte de una tarjeta antes de `salida` (reanuda y sale
a la hora) y otro después (no reanuda). (3) `FORZAR_ROJO` en el Maestro alternando: rojo fijo, el Esclavo sigue; luego
`DEG_FIN` o `SET_MODO:MENU`. (4) Maestro en `MENU` esperando la siembra + `FORZAR_ROJO` + siembra: rojo fijo, ni verde
ni ámbar. **Presupuesto:** las líneas de prueba nuevas no superan a las de producto nuevas (el trinquete de
instrumentos/producto no sube, `CLAUDE.md` §4). **Dónde viven:** el bloque `J` en un `.inc` NUEVO incluido por
`orquestador_degradado.cpp`, y los casos `P` en un `.inc` NUEVO incluido por `arnes_puente.cpp` (patrón de
`arnes_puente_testigo*.inc`): los dos pasan de 500 líneas y no pueden crecer; el `#include` neto es 1 línea y se
compensa en el mismo fichero.
**Pruebas que deben verse en ROJO antes del código** (no existen; valores de esta SPEC, ninguno sale del código):
- *Arnés del Degradado* (fila 19, bloque `J`; ampliación de un arnés existente, no un pack nuevo; simulador
  congelado): **J1** `FORZAR_ROJO` en el verde del Maestro: acuse = `ROJO_FIJO_EN_ESTE_POSTE`, 0 ms de verde en 600 s,
  modo sigue `MODO_DEGRADADO`, el Esclavo sigue alternando. **J2** corte tras J1: arranca en rojo fijo, no reanuda.
  **J3** ventana de `D-29`: Maestro en `MENU` con permiso + `FORZAR_ROJO` + siembra → **la LUZ del Maestro, rojo fijo
  sostenido en 1.800 s, 0 ms de ámbar y 0 ms de verde**, modo `MODO_DEGRADADO` y `respaldo_degradado == 0`; control sin
  la orden: reanuda (`G0`). **J4** salida con `δ` = 0, ±6 s: ningún ms de verde-verde ni verde-ámbar entre
  `salida − 60 s` y `salida + despeje + 60 s`; estado final de las dos = ámbar. **J5** corte antes de `salida`: reanuda
  y sale a la hora; corte después: no reanuda. **J6** reprogramar sale a la hora nueva; cancelar no sale; cancelar sin
  nada, `DEG_T` con salida y salida con la luz en verde: su motivo; **tras salir (programada, `MENU` o rendición) se
  puede volver a entrar con `DEG_T` y programar otra salida**, y el rechazo de `DEG_T` no persiste. **Controles
  negativos**, con el defecto inyectado en el `.cpp` real y restaurado por hash: quitar la baja del permiso en
  `modo_degradado_forzarRojo()`, quitar el disparo de la salida, `salidaS` sin leer en la reanudación, `salidaS` sin
  poner a cero al salir: las cuatro dan FALLA. **Invertir B8**, que hoy EXIGE `acuse == 1` y `modo != MODO_DEGRADADO`
  (`CLAUDE.md` §9: se invierte, no se borra). *La medida de la salida en un solo poste* (riesgo (h).1) **no es una
  prueba**: no entra en el recuento de las que aseveran.
- *Arnés del puente* (`bluetooth.cpp` REAL de las dos puntas): deja de tener `pedirSalida()` fijo a `true` y una copia
  de `forzarRojo()` (cond. 4): los valores los manda el arnés y asevera la trama literal. **P1** cada código de
  `modo_degradado_forzarRojo()` → su `RESULT:` de (b), con y sin PIN, ninguno `SALIENDO_TODO_ROJO`; y `SET_MODO:MENU` en
  la ventana → `OK_REANUDACION_CANCELADA`, sin permiso → `OK`. **P2** `DEG_FIN`: 5 entradas de formato malo
  (`25:00:00`, campo de menos, byte de sobra, `CANCELAR` con basura, sin PIN → `AUTH_FAILED`) y cada motivo de (c) → su
  `DESC` exacto; `PROGRAMADA`/`REPROGRAMADA`/`PROGRAMADA_SIN_RESPALDO`/`CANCELADA` solo con lo que devolvió el stub;
  `CONSULTA_DEG_FIN` sin PIN, las dos formas. **P3** Esclavo: lo mismo con `RechazoSalida`, y `FORZAR_ROJO` sigue en
  `RENOMBRADO_USE_AMBAR_EMERGENCIA`. **P4** `DEG_T` con salida → «Cancele antes la salida programada».
- *Test de la app* (jsdom): **A1** `FORZAR_ROJO|ROJO_FIJO_EN_ESTE_POSTE` dice «este poste» y «el otro sigue alternando»
  y no «no vuelve a alternar» ni «menu» (la prueba afirma que la clave existe, o sería ABORTADO). **A2** con el botón
  nuevo (afirma que existe) el aviso de paleteros se pide UNA vez: el segundo clic tras confirmar, con un `ahora`
  distinto, casa el vale por el nombre de la orden y no vuelve a pedirlo. **A3** el botón «Programar salida» existe en
  las dos pantallas, manda `SET_MODO:DEG_FIN:ahora,salida` con `ahora` releído en cada envío y reutiliza la `salida`
  guardada en el Esclavo. **A4** traduce cada `RESULT`/`DESC` de `DEG_FIN`, `CAUSA:ROJO_TOTAL`,
  `OK_REANUDACION_CANCELADA` de `MENU` y `CONSULTA_DEG_FIN` (cuenta atrás y tarjeta «falta el otro poste»);
  `SALIDA_VENCIDA` dice reprogramar en el Maestro, no reintentar.

## 8. CÓMO SE PONEN DE ACUERDO LAS DOS PUNTAS SIN RADIO

> **El Degradado da verde sin confirmar con el otro extremo, pero no a ciegas: lo hace sobre un acuerdo cerrado
> ANTES, con la radio viva.** La publicación de configuración se llama al arrancar y su cabecera dice por qué:
> *«cuando el radio muera ya será tarde para acordarlo»*.

**(a) El reparto del ciclo.** El coordinador manda **seguidas** las dos tramas de configuración —verde y despeje—; el
Poste 2 las guarda y **acusa el CONJUNTO con un solo acuse** — se acusa lo aplicado, no cada trama suelta. **Y el flag
de recibido no es el valor:** «nunca llegó» y «llegó un cero» son motivos distintos, porque un cero puede ser «el
Maestro dijo cero» o «no llegó nada» y entrar en el segundo caso es operar a ciegas.

**(b) La medida de desfase** (`SFTY-23`). La petición manda el **segundo del Maestro leído DENTRO de la función del
envío**: no existe variable donde guardarlo, para que una retransmisión no reenvíe un segundo caducado. El Poste 2
contesta la diferencia en **complemento a dos en un byte** y la puerta la compara contra su tolerancia. ⚠️ **Es la
condición MÁS DÉBIL y por eso va la última:** la medida es **circular** y **lee como cero todo múltiplo de 60 s** —60,
120 o 3600 s pasan la tolerancia mientras 45 s sí se detecta—, así que lo que sostiene el modo es la **frescura** de la
sincronización, no este número. Al fallar un intento **la medida se suelta y la hora y la configuración NO**: es
diagnóstico contra seguridad.

**(c) La puerta del Poste 1** (`modo_degradado_evaluarEntrada()`). También devuelve **motivo, no sí/no**; la llamaba
`SET_MODO:DEGRADADO`, que salió con `D-46` (`f9cad1f`): hoy no la alcanza ningún camino y se conserva. Sus motivos:
falta hora · nunca hubo sincronización · sincronización vieja · **falta configuración** —que el ESCLAVO haya acusado el
ciclo; **se añadió porque faltaba**, y sin él el Maestro daba verde por reloj mientras el Esclavo rechazaba y caía a
ámbar por orfandad— · falta desfase · desfase alto.

> 🔴 **(d) LOS DOS UMBRALES DE SINCRONIZACIÓN SON DISTINTOS POR PUNTA: LA MISMA ORDEN PUEDE ENTRAR EN UNA Y SER
> RECHAZADA EN LA OTRA.** El Poste 1 exige **frescura** (`SYNC_FRESCA_MS`); el Poste 2 sólo exige que **haya habido
> alguna** y que **no haya vencido el límite duro** (`LIMITE_SIN_SYNC_MS`). **La frescura es un plazo mucho más
> corto**, así que entre los dos el Poste 2 acepta `SET_MODO:DEGRADADO` y el Poste 1 lo rechaza por sincronización
> vieja: **una punta en Degradado dando verde por reloj y la otra en su ciclo normal**, que se irá a ámbar por
> orfandad — el mismo escenario que la falta de configuración cierra en el sentido contrario, **abierto en éste**.
> **El firmware no cruza esos dos números: no hay `static_assert` que los relacione, y viven en proyectos
> distintos.** ⬇️ ~~ni pack~~ → **medido el 14/09: el pack `costura_05_limite_48h` SÍ los lee los dos y reproduce la
> separación**, pero **certifica que el hueco sigue ahí; no lo cierra ni hace fallar a nadie** (HUECO 7).
> `grep -n "SYNC_FRESCA_MS\|LIMITE_SIN_SYNC_MS" 01_Firmware/{Maestro,Esclavo}/src/modo_degradado.cpp`
> ⬇️ **Desde `D-46` (`f9cad1f`) ninguna orden llega a estas dos puertas** —`SET_MODO:DEGRADADO` salió de las dos
> puntas—; los dos números siguen sin cruzarse y la del Poste 2 (`degradado_entrar()`) la sigue usando su reanudación.

**(e) El cálculo de fase — la misma función, no dos que «hacen lo mismo».** Vive en una cabecera compartida **que debe
ser idéntica** (lo compara `costura_01_contratos`): **posición = segundos del día módulo el ciclo, con ciclo = 2 ×
(verde + `AMARILLO_SEG` + despeje)** (desde `D-45`, (e.bis)), **anclada a la HORA DE PARED y no a un contador
propio** —dos equipos encendidos con un minuto de diferencia arrancarían desfasados un minuto entero—. **El orden lo
fija la función y no se negocia: la posición 0 es
el verde del Maestro, o sea que EL POSTE 1 TIENE EL VERDE PRIMERO**, y detrás van su amarillo, despeje, verde del
Esclavo, su amarillo y despeje. **Guarda de medianoche:** el último tramo del día y el primero del siguiente son
**siempre despeje**, porque a las 00:00:00 la posición salta y ese salto puede caer en mitad de un verde y saltarse el
todo-rojo. **Y la
configuración imposible tiene respuesta escrita:** con verde o despeje a cero devuelve despeje —todo-rojo—, no un
caso «que no debería pasar».

> **(e.bis) El amarillo en la fase (`D-45`, `N-174`) — CONSTRUIDO en `cda33df`, SIN BANCO.**
> - **Seis fases por punta, en este orden** (`FaseDegradado`): `FD_VERDE_MAESTRO`, `FD_AMARILLO_MAESTRO`,
>   `FD_DESPEJE_A`, `FD_VERDE_ESCLAVO`, `FD_AMARILLO_ESCLAVO`, `FD_DESPEJE_B`. **Ciclo = 2 × (verde + `AMARILLO_SEG`
>   + despeje)**: el verde y el despeje no se tocan. Cada punta enciende su amarillo en la fase suya —el Poste 2 aplica
>   `aplicarLuz(calcularFase() == FD_VERDE_ESCLAVO)`, y el `false` llega como `semaforo_forzarRojo()` sobre un verde—;
>   la otra sigue en rojo. `ciclo_degradado_faseCruda()` da la fase de una posición sin guardas.
> - **Guarda de medianoche, con amarillo.** El último tramo del día y el primero del siguiente siguen siendo
>   despeje. En los `AMARILLO_SEG` previos al tramo final, la punta que estaba en verde al empezar esa ventana está en
>   su amarillo y un verde que empezaría dentro no se enciende; un amarillo cuyo verde acabó dentro del tramo inicial
>   sale como despeje. «Venía de verde» se calcula sobre la hora de pared, no se recuerda.
>   `ciclo_degradado_restante()` cuenta las mismas fronteras. Lo ejerce la fila 15 (`arnes_ciclo.cpp`: todo verde
>   acaba en el amarillo de su punta, de `AMARILLO_SEG` enteros, y éste en despeje).
> - **El margen de desfase entre relojes es el despeje en los dos sentidos.** Antes el sentido favorable aguantaba
>   más por el ámbar con que abría el Poste 2; sin él, la fila 19 (bloque C, `C3`/`C4`) asevera que con el Esclavo
>   atrasado el cruce rompe exactamente en el despeje ampliado y con el Esclavo adelantado sólo un segundo más (el
>   residuo de la hora en segundos enteros), contando el amarillo del otro como paso abierto. `costura_12` se
>   invirtió en el mismo commit. **Sin acta de compuerta sobre `cda33df`.**
> - 🔴 **Las dos puntas tienen que llevar `D-45` A LA VEZ.** Un Poste 1 con la fase nueva y un Poste 2 con la anterior
>   calculan ciclos de duración distinta sobre la misma hora y se desfasan en cada vuelta: **verde contra verde** al
>   cabo de pocas vueltas. La cabecera idéntica (`costura_01_contratos`) lo vigila en el árbol, no en los equipos:
>   ninguna trama compara la versión cargada. **Se cargan las dos en la misma visita**, y en campo manda `ESTADO.md`.

## 9. LOS DOS RELOJES Y LOS DOS PRESUPUESTOS — todos cierran sobre el umbral de silencio

> 🔴 **EL SILENCIO DE LA RADIO NO SIGNIFICA LO MISMO EN LOS DOS MOMENTOS DEL CICLO.** Al **pasar el testigo** hay que
> ser estricto y reintentar fuerte: ahí es donde alguien puede quedarse en verde mientras el otro también lo tiene.
> Con el cruce **quieto** —una en verde, la otra en rojo, nada que decidir— el mismo silencio **no abre ninguna vía de
> verde contra verde** y se tolera mucho más, con una condición: **no empezar un cambio sin haber recuperado el
> enlace**. **Confundirlos manda el cruce a ámbar por una lluvia cuando no había nada que decidir** (27/08 y 13/09).

**Qué separa hoy el firmware y qué no.** El **ancla** sí está separada, por tres caminos: el reloj de orfandad del
Esclavo contesta *«¿el Maestro GOBIERNA?»* —sólo lo refrescan las órdenes de rojo y de verde—; el aviso de radio al
reloj contesta *«¿LLEGA la radio?»* contando **cualquier** trama válida; y la cuenta de reintentos **es un reloj
propio del cambio**, vivo sólo en las esperas de acuse. **En el Maestro el silencio contesta sólo «¿me CONTESTA?»**
—lo renueva una respuesta (§4), no cualquier trama— y la pregunta «¿llega radio?» **no tiene reloj propio** en esa
punta: los cinco lectores del reloj viejo preguntaban lo otro. **Lo que NO está separado es el NÚMERO:** el umbral de
silencio es uno y gobierna las dos puntas **y las dos preguntas** —el veto del cambio del Maestro lee el MISMO
contador y la MISMA constante que el ámbar por orfandad, desplazada lo que dura un acuse, y en el Esclavo un solo
reloj decide **las dos cosas**—. 🔴 **Y la separación que sí existe está ORDENADA AL REVÉS: el reloj del cambio vence
ANTES que el de la quietud**, y no por accidente: `costura_09` **exige** que el peor caso quepa bajo el techo de
orfandad o los últimos reintentos serían código muerto (`N-71`). **Esa desigualdad abre el HUECO 5.**

**Y sobre el MISMO techo se apoyan dos cuentas que NO se pueden comparar, porque miden contra bordes distintos**
(`CLAUDE.md` §7). **(A) El presupuesto asignable:** lo que queda libre bajo el techo de orfandad **crudo** tras el peor
caso del ciclo —la cadencia del latido, el amarillo de cierre del Esclavo (`D-45`) y todos los reintentos con su
tiempo de cable: 23,8 s de 28, libres 4,2 s (§2.2.bis)—, y lo recalcula `costura_09` leyendo las constantes del
C++; *su borde* es que quien primero llega manda, y es el ámbar por orfandad. **(B) La desigualdad del punto de
suelta:** el mismo peor caso **más lo que dura un acuse** (27,3 s de 28; 🔴 su borde está
abierto: el rojo de la suelta o su arranque, §2.2.bis); **NO es un presupuesto y su holgura
NO es de nadie**, su borde es el instante en que esta punta suelta el verde, y dice que adelantar la suelta no puede
recortar el presupuesto de reintentos. **No se restan ni comparten holgura**, y un tercer consumidor se mide contra
**(A)**. **Contra el borde de (B) se mide también el intercambio de hora** (1.49c): mientras corre, el latido está
suprimido y sus acuses **no renuevan** el silencio del Maestro, así que la cadencia del latido más todos sus intentos
tienen que caber antes del punto de suelta del verde —si no, un intercambio con lluvia soltaría el verde con el
Esclavo contestando—. Lo exige un `static_assert` junto a `SYNC_MAX_INTENTOS`, que desde `D-45` mide contra el
ARRANQUE de la suelta (un `AMARILLO_MS` antes del rojo), y `costura_09` lo recalcula. **El tiempo de aire** —las
copias de la ráfaga por la tasa aérea— **está escrito A MANO en el coordinador como tiempo de cable y NO se deriva
de ningún parámetro de la radio** (`ENVIO_TRAMA_MS`); `costura_09` lleva el mismo valor a la vista y por el mismo
motivo. Los parámetros del módulo son de **SPEC 6 §B**, y aquí sólo se exige que las
dos puntas y el repetidor estén configurados igual o el CRC no casa. **Subir el umbral de silencio sin mover los
reintentos por su plazo ensancha el HUECO 5** — el compromiso que `T-2` resolvió en campo: **SPEC 6 D.**

## 10. QUIÉN EJERCE CADA BARRERA DE ESTE DOCUMENTO

> **Una spec puede describir barreras que ningún compilador ejerce, con UNA condición: que cada barrera lleve escrito QUIÉN
> la ejerce.** El criterio es `CLAUDE.md` §6.3 — **¿algún arnés COMPILA ese `.cpp`?**; si sólo lo lee por texto no ve un
> defecto del TIEMPO. Filas = compuerta; reparto de `.cpp`, `ARQUITECTURA.map` §4-5. Medido sobre `ef3504c`.

| barrera | quién la EJERCE hoy |
|---|---|
| Sólo el fichero del semáforo escribe pines | **fila 17**, y **sólo el Maestro** · el Esclavo, texto (SPEC 1 §13) |
| El CRC de cada trama (`SFTY-3`) | 🔴 **NADIE.** El fichero de protocolo de las **dos** puntas sólo lo cruza PlatformIO |
| Silencio → ámbar, y la suelta por margen (§4) | ✅ **fila 18**, bloques G: compila el coordinador y el bucle del Esclavo reales y relee el umbral del C++. Que la demanda **no** sostenga el silencio del Maestro lo ejerce `G3D` (1.49c), con control negativo visto fallar |
| Los reintentos (§3) | ✅ **fila 18** (su número, releído) · la desigualdad de §9, `costura_09` por texto |
| Vuelta del enlace · supresión del latido (§2.2) | 🔴 **nadie**, ninguna de las dos |
| Retardo de cortesía · §5.2 rojo confirmado · §5.4 backstop de verde | ✅ **fila 18** las tres |
| §5.3 el veto de margen | ✅ **fila 18, POR SU EFECTO** (G3, G11). Es privada al fichero: ningún arnés puede nombrarla, así que se mide el verde y no la función |
| §8 (e) el cálculo de fase · §7 la reanudación | ✅ la fase, **fila 15** sola —es cabecera pura— y **19** dentro del modo; la reanudación, **18** (Esclavo) y **19** (Maestro) |
| §2.3 los TRES `if` del veto · §6 el acuse del aviso | ✅ **fila 18**: compila el bucle y el Bluetooth del Esclavo REALES; el acuse es su bloque H |
| §8 (c) la puerta del Degradado del Maestro | ✅ **fila 19**: su adaptador llama a las dos consultas del coordinador real |
| §8 (d) el cruce de los dos umbrales de sincronización | 🔴 **NADIE lo cierra** — un pack lo mide y lo publica, pero no falla por él (HUECO 7) |
| §2.2.bis y §8 (e.bis) el amarillo de cierre en la radio y en la fase (`D-45`) | ✅ **filas 18 y 19** (el orden de la luz en las dos puntas, el amarillo del Poste 2 ante la orden de rojo, la suelta adelantada, la fase de seis tramos dentro del modo) y **15** (la fase pura); invertidas en `cda33df`. El presupuesto, `costura_09` por texto. Sin acta de compuerta sobre `cda33df` |
| §7.quater la salida programada y el rojo total del Degradado (`D-52`, `D-51` corregida) | 🔴 **NADIE: no construido.** Las pruebas de §7.quater (i) deben verse en rojo antes del código; el `B8` de hoy exige lo contrario |

**Cuenta: ~~16 barreras — 11 ejecutadas, 4 sin nadie, 1 partida~~ 17 barreras — 12 ejecutadas, 4 sin nadie, 1
partida** *(02/10: entra `D-45`, ejecutada desde `cda33df`)* (la barrera de salidas, ejecutada en una punta y de
texto en la otra). Los rojos del CRC, de la vuelta del enlace y de la supresión del latido **no son tres casillas
pendientes**: apuntan al fichero de protocolo y al coordinador fuera del ciclo — es el mismo hueco visto tres veces.

## HUECOS MEDIDOS

1. 🟢 **CERRADOS dos:** el número de reintentos del aviso de ámbar está **elegido, no derivado**; y el veto sobre la
   orden de verde es hoy sólo el de la app (§2.3): el mando salió (`D-30`). **No** cierra que `J16` p5/p8 sigan vacíos
   y leídos (SPEC 5 §3) hasta `D-44` (SPEC 1 §12.2).
2. 🔴 **Los dos presupuestos de radio no casan sobre el mismo techo: fila pendiente, no detalle.** §9 los separa por su
   borde; **elegir el modelo sigue sin hacerse**.
3. ⚠️ **El acuse de rojo no lleva a qué orden contesta** —escrito en el propio coordinador—: un acuse retenido en el
   aire y soltado tras una orden de rojo perdida **sería indistinguible del bueno**, y la marca de rojo confirmado
   acota el caso pero no lo cierra. Sin decisión. Y **la rama del latido entrante del Maestro no tiene emisor**: el
   Esclavo sólo contesta latidos y nunca los origina, y el Repetidor no origina tramas — inofensivo para la luz.
4. ⚠️ **La pieza de la decisión del reloj que avisa de pila perdida sigue sin construir: ese bit del RTC no llega al
   STM32** (`D-21`), y el Degradado se autoriza sobre una comprobación que no lo ve (**SPEC 3, H-2**). Y **nada de este
   capítulo ha visto una tarjeta**: el acuse del aviso, el salto de hora por rojo, el evento periódico y el ancla de
   respuesta del silencio del Maestro (§4, 1.49c) están en `main` **SIN BANCO**, y qué corre en cada equipo lo dice `ESTADO.md`.
5. 🟢 **CERRADO en el fuente (`D-34`, 15/09) — el número se queda.** ~~Nadie mira la luz de la otra punta entre los
   dos vencimientos~~ → medido en el arnés de dos puntas: hasta 23 s de ámbar del Maestro contra verde del Esclavo, y
   el mecanismo no eran los reintentos sino las anclas (`roadmap` 1.39). Lo que el equipo hace ahora está en **§4**;
   las filas `G12`–`G14` lo vigilan. **Sin banco: sigue sin haberse visto en una tarjeta.**
6. 🟡 **LA REANUDACIÓN AUTOMÁTICA NO SE ANUNCIA; EL CORTE, SÍ, Y SÓLO SI APAGÓ EL ESP32** (§7). La app abre el
   cartel de corte con el parte de arranque del puente (`js/aviso_corte.js`, causa de subida de tensión) y el
   `MODO:` del `$STATUS`; **no lo abre** un reinicio del STM32 solo (watchdog), y el parte no trae la hora del corte.
   Ningún evento dice «reanudó». No es defecto de la decisión que mandó reanudar sino de lo que esa decisión no dijo.
   **Es decisión vial: del responsable.**
7. 🔴 **LAS DOS PUNTAS SE AUTORIZAN CON UMBRALES DE SYNC DISTINTOS Y NADIE LOS CIERRA** (§8 (d)). Remedido el 14/09: no
   hay `static_assert`, las dos constantes viven en proyectos distintos, y el pack que sí las lee **reproduce la
   separación y la publica como residual: mide el hueco, no lo tapa. Es el hueco más grande del acuerdo de §8**, porque
   lo que falla no es la radio: es la puerta.
8. 🟢 ~~**EL DEGRADADO CON TESTIGO (§7.bis, `D-35`) NO EXISTE: CERO CÓDIGO.**~~ → construido (`990c278`) y probado en
   banco (`4a2c73c`). Su vencimiento de 31 días está quitado (`1a78873`, §7.ter (d)).
9. 🔴 **EL DEGRADADO AUTOMÁTICO (§7.ter, `A-15`) ESTÁ CONSTRUIDO Y SIN BANCO** (`a0d605b`, `1a78873`): sólo lo mide el
   arnés de PC. Sus riesgos residuales, §7.ter (f), son decisión del responsable; (f).1 está medido.
10. 🟡 **EL AMARILLO DE CIERRE (`D-45`, `N-174`) ESTÁ CONSTRUIDO EN `cda33df`, SIN BANCO** (§2.2.bis, §4, §7,
   §8 (e.bis)), con el umbral a 28 s (`D-48`) como salida al presupuesto de radio. 🔴 **Abiertos:** el borde de la
   desigualdad (B) (27,3 s contra el rojo de la suelta, 30,3 s contra su arranque; §2.2.bis, §9), `ESC:AMARILLO`
   sin construir (§2.2.bis). El arranque en rojo fijo sin hora (`D-47`) lo ejercen `F2.5`, `F2.6` y `F3.5` del
   arnés del Degradado (SPEC 3 §5). Símbolos e instrumentos, SPEC 1 §12.10.
11. 🔴 **LA SALIDA PROGRAMADA (`D-52`) Y EL ROJO TOTAL CORREGIDO (`D-51`) NO ESTÁN CONSTRUIDOS** (§7.quater, 04/10). Hoy
   `FORZAR_ROJO` en Degradado sale del modo y no revoca el permiso de `D-29` (ventana de la siembra); sin radio no hay
   salida simultánea (§7.ter (f).5). Los riesgos nuevos, §7.quater (h), son del responsable.
