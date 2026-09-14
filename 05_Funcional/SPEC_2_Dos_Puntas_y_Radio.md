# SPEC 2 — LAS DOS PUNTAS Y LA RADIO

> **EL REQUISITO, Y ES EL ÚNICO QUE IMPORTA AQUÍ: LOS DOS POSTES NUNCA DAN VERDE A LA VEZ.** No hay cable entre
> ellos. Lo único que los une es una radio lenta que pierde tramas, y esta especificación es lo que el equipo
> hace **con esa radio sana, degradada y muerta**.

**Qué NO entra:** el ciclo de un poste solo (SPEC 1) · la hora (SPEC 3) · la app (SPEC 4) · el cobre y las
radios como aparato (SPEC 5) · el procedimiento de campo y el parche `T-2` (SPEC 6).
**De dónde sale:** `DECISIONES.md` manda en lo decidido y el fuente en lo que el equipo HACE; los manuales no
son fuente. **Ninguna cifra vive aquí**: se cita el SÍMBOLO, porque un número copiado a mano nace caducado.

## 1. EL REPARTO — quién decide qué

| | |
|---|---|
| **Poste 1 (Maestro)** | El ÚNICO que ordena verde. Su máquina de estados: `Maestro/src/coordinador.cpp` (`EstadoCoord`) |
| **Poste 2 (Esclavo)** | Obedece y ACUSA. No tiene coordinador: su lado de la radio es la cadena `if/else` de `Esclavo/src/main.cpp`, **único lector de radio** de esa punta |
| **Repetidor** | `Repetidor/src/main.cpp`. Puente transparente con validación de CRC: **no origina ni una trama** |

**La única excepción a «el Maestro ordena» es el Modo Degradado** (§7): allí no hay radio y cada punta decide
su luz por reloj. Es el único modo que enciende un verde **sin confirmación del otro extremo**, y por eso es el
que más condiciones tiene delante.

**Contrato de la radio:** `{Maestro,Esclavo}/include/protocolo.h` — **idéntico byte a byte en las dos puntas**,
y lo compara `costura_01_contratos`. La trama es `RF_Packet`: identificador, comando, **un solo byte** de
parámetro y CRC; cada envío sale en ráfaga de `RF_BURST_COPIES` copias y el receptor descarta las repetidas por
identificador (`protocolo.cpp`).

## 2. EL DIÁLOGO DEL CICLO

### 2.1 Los comandos, y quién los emite

| comando | dirección | qué es |
|---|---|---|
| `CMD_GO_GREEN` | M → E | la única orden que abre paso en el Poste 2 |
| `CMD_ACK_GREEN` | E → M | «mi verde está encendido y estable» |
| `CMD_GO_RED` | M → E | la orden que cierra. También es el latido cuando el rojo no consta |
| `CMD_ACK_RED` | E → M | «estoy en rojo» |
| `CMD_PING` / `CMD_PONG` | M → E / E → M | latido: mide que el enlace vive |
| `CMD_GO_AMBAR` | M → E | ámbar ordenado (`N-134`), detrás de un `CMD_GO_RED` previo |
| `CMD_AMBAR_ESCLAVO` | E → M | el Poste 2 avisa de su ámbar de emergencia |
| `CMD_ACK_AVISO_AMBAR` | M → E | el acuse de ese aviso (`D-31`) |
| `CMD_CANCELA_AMBAR_ESCLAVO` | E → M | el Poste 2 lo retira. **No se acusa, y es la regla** (§6) |
| `CMD_DEMANDA` / `CMD_ACK_DEMANDA` | E → M / M → E | demanda de cámara y su respuesta, que dice si se atiende |
| `CMD_HORA_*`, `CMD_DELTA*`, `CMD_CONFIG_*` | M → E | servicio. **No tocan luz.** Ver SPEC 3 |

Del Poste 2 salen por iniciativa propia **tres** tramas —`CMD_DEMANDA`, `CMD_AMBAR_ESCLAVO`, `CMD_CANCELA_AMBAR_ESCLAVO`—; todo lo demás que emite es respuesta.

### 2.2 El paso de verde a verde, que es donde se mata a alguien

Un cambio de sentido pasa SIEMPRE por todo-rojo, contado **desde el acuse del otro lado, no desde la orden**
(`N-162`). El plazo es `tiempoDespejeMs` (`coordinador_configurar()`).

```
Verde Maestro -> C_MASTER_A_ROJO -> C_ESPERA_ESTATICO_TRAS_MASTER  (despeje)
              -> GO_GREEN -> C_ESPERANDO_ACK_GREEN -> ACK_GREEN -> Verde Esclavo
Verde Esclavo -> GO_RED -> C_ESPERANDO_ACK_RED -> ACK_RED
              -> C_ESPERA_ESTATICO_TRAS_ESCLAVO (despeje) -> Verde Maestro
```

**Las cuatro puertas al verde propio del Maestro llevan el mismo veto** (`puedeSostenerVerde()`):
`C_INICIAL_ESPERA_ESTATICO`, `C_ESPERA_ESTATICO_TRAS_ESCLAVO` y las dos ramas de `coordinador_pedirCambio()`
desde `QV_NINGUNO`. **El Maestro no abre su verde contando con que la orden de rojo LLEGÓ, sino con que se
CUMPLIÓ**: `rojoEsclavoConfirmado` se pone en un solo sitio —la llegada de `CMD_ACK_RED`, nunca con un
`GO_GREEN` en vuelo— y se baja en cada orden de rojo, al mandar verde y al arrancar. Sin ella, un solo
`GO_RED` perdido con el Esclavo en verde dejaba verde en las dos puntas mientras los latidos le refrescaban
la orfandad.

### 2.3 El latido tiene DOS formas, y no es un detalle

Cada `LATIDO_MS` (`coordinador.cpp`), **suprimido mientras se espera un ACK** (`SFTY-13`, colisión en el bus
medio dúplex) y mientras corre la sincronización: si el rojo del otro lado **no consta** (`quienVerde ==
QV_NINGUNO && !rojoEsclavoConfirmado`), en `C_MENU_IDLE` o en `C_FALLO` → sale **`CMD_GO_RED`** y se espera
`CMD_ACK_RED`; en cualquier otro caso → `CMD_PING` y se espera `CMD_PONG`. Es decir: **el reintento del
todo-rojo de emergencia es el propio latido.** Sin esto, el rojo total —el de entrar en Manual, el de
emergencia— no tenía ningún reintento. ⚠️ **Y esa supresión es lo que hace que el contador de silencio de la
otra punta llegue ya envejecido:** ver §9 y SPEC 6 D.1.

### 2.4 Lo que el Poste 2 hace al recibir

`CMD_GO_RED` y `CMD_GO_GREEN` refrescan `tUltimoComando` (el reloj de orfandad); las tramas de servicio y el
ámbar ordenado **no lo refrescan**, a propósito: significan «hay portadora», no «el Maestro está gobernando el
cruce». **Los dos vetos del ámbar de emergencia (`D-8`) viven en esas dos ramas**, y son `!mando_ambarLocal()
&& !bluetooth_ambarEmergencia()`. Vetado, **ni se obedece ni se ACUSA**: acusar un rojo que no se ha encendido
dejaría al Maestro dando verde convencido de que aquí hay rojo — *se guarda lo que ABRE PASO, no lo que lo
para*. La orden de verde es **idempotente**: repetida sobre una luz ya en ámbar de transición o en verde,
**re-acusa y no toca la luz** (`N-162`).

## 3. `SFTY-7` — LOS REINTENTOS

`TIMEOUT_ACK_MS` y `CICLO_MAX_REINTENTOS` (`Maestro/src/coordinador.cpp`). Agotados:

- desde `C_ESPERANDO_ACK_GREEN` → `C_FALLO` **con alarma** `FALLO_RF / REINTENTOS_AGOTADOS`, **y sin mirar el
  reloj del silencio** — la asimetría con la línea de abajo es la que abre el hueco de §9;
- desde `C_ESPERANDO_ACK_RED` → `C_FALLO` **sólo si hay enlace y queda margen de silencio**. Si no, se sigue
  pidiendo el rojo y la caída la reporta `SFTY-6`: esa puerta no lleva alarma propia y no puede taparle la
  suya a `SFTY-6`.

**Contra la distancia no sirve esperar, sirve repetir**: la distancia sube la **pérdida**, no la latencia. La palanca correcta es `RF_BURST_COPIES`, no el timeout.

## 4. `SFTY-6` — EL SILENCIO Y EL ÁMBAR DE HUÉRFANO

El umbral es `SFTY6_SILENCIO_MS`, y vive **una sola vez**, en `protocolo.h`, porque gobierna las dos puntas: tres copias mantenidas a mano fue como se desincronizaron (`N-69`).

| punta | ancla del silencio | qué hace al vencer |
|---|---|---|
| Maestro | `tUltimaRxEsclavo` — **la respuesta que le CONTESTARON** | `C_FALLO`, ámbar intermitente, alarma `FALLO_RF / SILENCIO_…`, y `CMD_GO_RED` **en ese mismo instante** |
| Esclavo | `tUltimoComando` — **la última orden que RECIBIÓ** | `semaforo_iniciarFallo()`, ámbar intermitente, misma alarma |

> 🔴 **LOS DOS UMBRALES SON EL MISMO NÚMERO Y LOS DOS INSTANTES NO.** Entre «la orden llega al Esclavo» y «su
> respuesta llega al Maestro» hay el retardo de cortesía (`RETARDO_RESPUESTA_MS`, `SFTY-17`) más el viaje de
> vuelta, y **ese desfase era la ventana**: con la dirección Maestro→Esclavo muerta, el Poste 2 se iba a su
> ámbar **antes** de que el Poste 1 apagara su verde. Verde contra ámbar con la pluma del otro poste arriba.

**Se cierra soltando el verde propio un margen ANTES** (`N-163`): `puedeSostenerVerde()` adelanta la decisión
en `TIMEOUT_ACK_MS`, que es lo que ya acota el viaje completo de ida y vuelta. Al soltarlo, la punta va a rojo
directo, marca `verdeSoltadoPorMargen` y **sale a `C_IDLE`** —no a una espera de ACK— para no tocar la cadencia
del latido: medido, la otra salida producía **más** ámbares en microcortes que se recuperan. **El umbral NO se
baja**, y es condición del responsable: bajarlo manda el cruce a ámbar cada dos por tres cuando llueve. **Lo
que ese margen NO cubre —el verde de la OTRA punta cuando ésta ya cayó— es §9.**

**Vuelta del enlace:** `SFTY-9` reencola la hora y la configuración del ciclo, fuerza rojo, pide `CMD_GO_RED` y
**espera su acuse** antes de contar despeje. Si lo que hubo fue una suelta por margen, se reanuda por esa misma
puerta en cuanto `puedeSostenerVerde()` vuelve a ser cierto: un todo-rojo de minutos en una vía alternada no es
estado seguro, es donde la gente se pasa el rojo.

## 5. LAS BARRERAS QUE IMPIDEN VERDE + VERDE — la lista entera

1. **El Esclavo no enciende verde sin `CMD_GO_GREEN`.** Fuera del Degradado no hay otra vía.
2. **El Maestro no enciende verde sin `CMD_ACK_RED` del otro lado** (`rojoEsclavoConfirmado`).
3. **Ninguna punta abre NI SOSTIENE verde sin margen de silencio** — `puedeSostenerVerde()`, en las cuatro
   puertas de apertura y en `coordinador_actualizar()`. ⚠️ **Vigila el verde PROPIO, no el del otro** (§9).
4. **Backstop del Esclavo**: `MAX_VERDE_BACKSTOP_MS` corta un verde eterno mirando **la luz**, no la orden —
   porque en Degradado el verde no lo ordena nadie por radio.
5. **En Degradado, la fase la calcula la MISMA función en las dos puntas**: `ciclo_degradado_fase()` en
   `{Maestro,Esclavo}/include/ciclo_degradado.h`, que **debe ser idéntico** —dos implementaciones que «hacen
   lo mismo» es como se acaba con verde en las dos—, con su **guarda de medianoche**: el último tramo del día
   y el primero del siguiente son siempre despeje.
6. Y la que no es de la radio: **`SFTY-2`** — sólo `semaforo.cpp` escribe pines de luz.

## 6. EL ÁMBAR DE EMERGENCIA DEL POSTE 2, Y SU ACUSE (`D-31`)

**Qué avería cierra:** si se rompe **sólo el transmisor** del Poste 2 —oye pero no habla—, esa punta no tiene
forma de saberlo (su único dato de radio es el silencio de lo que RECIBE, y el Maestro le sigue hablando), el
técnico veía un `$ACK` idéntico al de la radio sana y el Poste 1 seguía dando verde hacia un carril cuyo otro
extremo está en ámbar. **Y muerde en el caso COMÚN**: la rama principal es `if (!degradado_gobiernaLuz())`.

**Armar** (dos puertas en `Esclavo/src/bluetooth.cpp`, sin PIN y con PIN, con el **mismo bloque letra por letra**): ámbar aquí, latch `ambarEmergencia` puesto, `CMD_AMBAR_ESCLAVO` al Poste 1, y `$ACK` inmediato al
teléfono. **El ámbar no espera al acuse** — bloquear el bucle por una radio lenta es peor (`N-130`).

**El Poste 1** anota el aviso, **contesta `CMD_ACK_AVISO_AMBAR`** y se va a `MODO_AMBAR`: para de ciclar, ordena
rojo y calla. El acuse promete **una sola cosa: que esta punta oyó**; no que el cruce se pare. Se emite en
`coordinador_actualizar()` —con el Maestro **ciclando**, que es cuando tiene permiso para hablar—, nunca en
`coordinador_escucharEnAmbar()`.

> 🔴 **EL ACUSE SE ESPERA SÓLO DEL PRIMER AVISO, Y ESO ES LA REGLA, NO UNA SIMPLIFICACIÓN.** El aviso es lo que
> manda al Maestro a `MODO_AMBAR`, donde **tiene prohibido transmitir** (`SFTY-21`). Esperar acuse en cada
> pulsación diría «nadie me oyó» **con la radio sana** cada vez que alguien pulsa dos veces —lo normal en un
> poste— y eso enseña al técnico a ignorar la única señal que esto existe para darle.

**Plazo y reintento:** `AVISO_AMBAR_TIMEOUT_MS` (`protocolo.h`), que un `static_assert` de `coordinador.cpp`
obliga a ser el mismo `TIMEOUT_ACK_MS`; y `AVISO_AMBAR_REINTENTOS`, **elegido por `D-32` (3), no derivado**
(§HUECOS, 1). Agotado, sale `$ALARM AVISO_RF / SIN_CONFIRMAR / AVISE_POSTE_1`, y **dice «no he podido
confirmarlo», nunca «el otro poste no se enteró»**: con reintentos de por medio, una trama perdida se ve igual
que un transmisor roto.

**Cancelar** borra el latch y **la memoria del acuse en la misma sentencia**, y para el plazo en vuelo: sin ese
borrado, el siguiente ámbar diría «el Poste 1 ya lo sabe» apoyándose en un acuse que ya no existe. Si queda el
latch del mando, **no se avisa al Maestro**: esta punta sigue en ámbar. **`CMD_CANCELA_AMBAR_ESCLAVO` NO se
acusa, y la asimetría es deliberada:** su camino normal es con el Maestro ya callado en `MODO_AMBAR`, donde un
acuse le obligaría a hablar justo en el modo en que calla; su red es que la **segunda pulsación reenvía**. El
Maestro sólo sale del ámbar **que pidió el Esclavo** (`modo_ambar_origenEsclavo()`) y sale **a todo-rojo**.

## 7. `SFTY-21` — EL MODO DEGRADADO

**Sin radio, la luz la decide el reloj.** La entrada es **MANUAL, siempre**: el equipo no entra solo. En el
Poste 2 la llave es la app (`D-18`, `SET_MODO:DEGRADADO`) por la puerta única `degradado_entrar()`; en el
Poste 1, `modo_degradado_setup()`.

**Condiciones, en `degradado_comprobar()` (`Esclavo/src/modo_degradado.cpp`), y devuelve MOTIVO, no un sí/no**
—quien la llame tiene que poder decirle al operario qué le falta—: `DEG_RECHAZO_SIN_HORA` (`reloj_horaFiable()`
es falso: **fiable, no sólo puesta**) · `SIN_CONFIG` / `CICLO_NULO` (el Maestro no dejó verde y despeje
verificados) · `SIN_SYNC` (ninguna en esta sesión) · `SYNC_VENCIDA` (el límite duro no es un botón de
posponer) · `AMBAR_VIGENTE` (lo puso una persona). **Lo que el operario LEE por cada uno, y los tres motivos
que no existen en el Maestro, están en SPEC 6 A.2 y no se repiten aquí.**

**Dentro:** todo-rojo de entrada (`rojoObligatorioMs()`, suelo `ROJO_MINIMO_MS`) y después verde **sólo** en la
fase propia. El despeje es el **valor final que mandó el Maestro, ya ampliado en el origen**: si cada punta lo
escalara por su cuenta, los verdes se solaparían durante minutos. **`D-26` (4) — un salto de hora se aplica
PASANDO POR ROJO**: si `saltoDeHora()` supera el margen del cruce, rojo en esa vuelta y el todo-rojo se cuenta
de nuevo.

**Las cuatro salidas, y ninguna admite marcha atrás** (`iniciarSalida()` baja el indicador de la pila **al
empezar**, no al terminar): el operario · **el regreso de la radio**, sólo con tramas de **gobierno** (`PING`,
`GO_RED`, `GO_GREEN`) —las de servicio significan «hay portadora», no «alguien gobierna»— · **el límite duro**
`LIMITE_SIN_SYNC_MS` sin sincronizar, con aviso previo en `AVISO_SIN_SYNC_MS` · **`D-21` (1): la hora dejó de
ser fiable en marcha.**

> **Las dos puntas acaban en ámbar, y sólo el Esclavo pasa por el despeje**, y es lo correcto: el Maestro
> llama a `irAAmbar()` y va directo; el Esclavo llama a `iniciarSalida(true)` —rendición—, que fuerza
> todo-rojo y entra en `DEG_RENDIDO`. El porqué de ese despeje está en **SPEC 6 A.4**.

**De `DEG_RENDIDO` no se sale solo**: hace falta una orden, o una sincronización nueva
(`degradado_registrarSync()`), y reentrar sigue siendo decisión del operario. **`D-29`:** un corte de luz ya no
mata la reanudación, y **el límite duro —la segunda puerta— no se toca**; la ventana y su alcance real están
en **SPEC 3 §6 y H-3**.

## 8. EL PRESUPUESTO DE RADIO — Y CUÁL ES EL BORDE DE CADA UNO

> 🔴 **EN ESTE REPOSITORIO HAY DOS CUENTAS DE RADIO Y NO SE PUEDEN COMPARAR: MIDEN COSAS DISTINTAS CONTRA
> BORDES DISTINTOS.** Confundirlas ya pasó una vez. Cada una se escribe con su borde al lado (`CLAUDE.md` §7).

**(A) El presupuesto de verdad — `PRESUPUESTO_LIBRE_MS`.** Lo que queda libre bajo el techo de orfandad
**crudo**, `SFTY6_SILENCIO_MS`, tras el peor caso del ciclo: la cadencia del latido más `CICLO_MAX_REINTENTOS`
pasos, cada uno con su tiempo de cable. **Es lo único asignable**, y lo recalcula
`costura_09_presupuesto_radio` en cada corrida leyendo las constantes del C++. *Por qué ese borde:* quien
primero llegue manda, y el que llega primero es el ámbar por orfandad — si el presupuesto lo desborda, los
últimos reintentos son código muerto, que es lo que pasaba antes de `N-71`.

**(B) La desigualdad del punto de suelta — el `static_assert` de `N-163`.** El mismo peor caso **más un
`TIMEOUT_ACK_MS`** contra `SFTY6_SILENCIO_MS`. **NO es un presupuesto y su holgura NO es de nadie:** su borde
es el punto en que esta punta **suelta el verde**, `SFTY6_SILENCIO_MS − TIMEOUT_ACK_MS`, y dice que adelantar
la suelta no puede recortar el presupuesto de reintentos, o con lluvia el cruce se iría a ámbar de más. **Las
dos son legítimas: no se restan ni comparten holgura**; un tercer consumidor se mide contra **(A)**.

**El aire:** `RF_BURST_COPIES` copias por envío y la tasa aérea fijan el tiempo de cable de cada trama; ese
coste está a la vista en `coordinador.cpp` (`ENVIO_TRAMA_MS`) y en `costura_09`, y **no se lee de ninguna
constante del firmware porque no existe como tal**. Los parámetros del módulo son de **SPEC 6 §B**; aquí sólo
se exige que **las dos puntas y el repetidor estén configurados igual**, o el CRC no casa.

## 9. LOS DOS RELOJES: EL DEL CAMBIO Y EL DE LA QUIETUD

> 🔴 **EL SILENCIO DE LA RADIO NO SIGNIFICA LO MISMO EN LOS DOS MOMENTOS DEL CICLO.** Al **pasar el testigo**
> hay que ser estricto y reintentar fuerte: ahí es donde alguien puede quedarse en verde mientras el otro
> también lo tiene. Con el cruce **quieto** —una en verde, la otra en rojo, nada que decidir— el mismo
> silencio **no abre ninguna vía de verde contra verde** y se tolera mucho más, con una sola condición: **no
> empezar un cambio sin haber recuperado el enlace**. **Confundirlos es lo que manda el cruce a ámbar por una
> lluvia cuando no había nada que decidir** (reportes del 27/08 y del 13/09).

**Qué separa hoy el firmware y qué no — la carencia no es la que parece.** El **ancla** sí está separada, por
tres caminos: `tUltimoComando` (`Esclavo/src/main.cpp`) contesta *«¿el Maestro GOBIERNA?»* —sólo lo refrescan
`CMD_GO_RED` y `CMD_GO_GREEN`, §2.4—; `reloj_notarRadio()` / `reloj_radioManda()` (`Esclavo/include/reloj.h`)
contesta *«¿LLEGA la radio?»* contando **cualquier** trama válida; y `retryCount` contra
`CICLO_MAX_REINTENTOS` **es un reloj propio del cambio**, armado al mandar la orden y vivo sólo en las dos
esperas de ACK. **Lo que NO está separado es el NÚMERO:** `SFTY6_SILENCIO_MS` es uno y gobierna las dos puntas
**y las dos preguntas**. El veto del cambio del Maestro —`puedeSostenerVerde()`— **lee el MISMO contador
`tUltimaRxEsclavo` y la MISMA constante** que el ámbar por orfandad, desplazada un `TIMEOUT_ACK_MS`: no es otro
reloj, es el mismo leído un margen antes; y en el Esclavo un solo `tUltimoComando` decide **las dos cosas**.

> 🔴 **Y LA SEPARACIÓN QUE SÍ EXISTE ESTÁ ORDENADA AL REVÉS: EL RELOJ DEL CAMBIO VENCE ANTES QUE EL DE LA
> QUIETUD**, y no por accidente — `costura_09_presupuesto_radio` **exige** que el peor caso del ciclo quepa bajo
> el techo de orfandad, porque un reintento inalcanzable es código muerto (`N-71`)—. **Esa desigualdad correcta
> es la que abre un hueco:** agotados los reintentos, el Maestro se va a ámbar intermitente con `FALLO_RF /
> REINTENTOS_AGOTADOS` mientras la otra punta —si el `GO_GREEN` llegó y su acuse no volvió— **sigue en verde**
> hasta que venza SU silencio. **Dura la diferencia entre los dos plazos, que se recalcula de las constantes.**
> `N-163` cerró el sentido contrario; **éste no lo cierra nadie** (HUECO 9).

**Y por eso la tolerancia de quietud no es un número suelto:** `SFTY6_SILENCIO_MS` fija además el punto de
suelta del verde y el techo de `PRESUPUESTO_LIBRE_MS` (§8), sujetos por `static_assert`, y **subirlo sin mover
`CICLO_MAX_REINTENTOS` × `TIMEOUT_ACK_MS` ensancha el hueco de arriba** — el compromiso que `T-2` tuvo que
resolver sobre la versión de campo: **SPEC 6 PARTE D.**

## HUECOS MEDIDOS

**1. 🟢 CERRADO — `AVISO_AMBAR_REINTENTOS` está ELEGIDO, no derivado.** La restricción nunca fue el aire, sino
**cuánto tarda el Poste 2 en declarar que el otro no contesta**, y ese equilibrio era del responsable: lo
contestó `D-32` (3), con la guarda rehecha contra el modelo que el firmware implementa. Los cuatro sitios que
decían «derivado» están corregidos. **Nada de esto lo ve la compuerta: se remide con `grep`.**

**2. 🔴 Los dos presupuestos de radio no casan sobre el mismo techo: fila pendiente, no detalle.** Escrito en
`D-31` (3) y en `coordinador.cpp`. §8 los separa por su borde; **elegir el modelo sigue sin hacerse**.

**3. 🟢 EL RIESGO QUE ESTE HUECO DESCRIBÍA YA NO EXISTE — y lo cerró una DECISIÓN, no un commit.** Decía que al
retirar `mando.cpp` el veto `!mando_ambarLocal()` de la rama `CMD_GO_GREEN` del Esclavo —**la única de las seis
llamadas vivas que ABRE PASO**— quedaría **ABIERTO, no inerte**. 🔴 **`D-32` (1) recorta `D-30`: sale sólo el
LCD y el mando SE QUEDA**, así que el veto sigue en pie y con el mando desmontado queda **CERRADA**. ⚠️ **Lo que
NO cierra: `J16` p5/p8 siguen vacíos y leídos por dos caminos** — SPEC 5 §3, regla de montaje.

**4. ⚠️ El `CMD_ACK_RED` no lleva a qué `CMD_GO_RED` contesta** —escrito en el propio `coordinador.cpp`—: un
acuse retenido en el aire más allá del último `GO_GREEN` y soltado tras una orden de rojo perdida **sería
indistinguible del bueno**. `rojoEsclavoConfirmado` acota el caso pero no lo cierra. Sin decisión.

**5. ⚠️ La rama `CMD_PING` del Maestro no tiene emisor.** Censado el 12/09 sobre
`{Maestro,Esclavo,Repetidor}/src`: el Esclavo emite `CMD_PONG` y **nunca** `CMD_PING`, y el Repetidor no origina
tramas —§6, declarado y no ejercido—. Inofensivo para la luz; se deja escrito en vez de tocarse.

**6. ⚠️ `D-21` pieza (A) sigue sin construir: el `OSF` no llega al STM32.** Toca aquí por su consecuencia —el
Degradado se autoriza sobre `reloj_horaFiable()`, que no ve ese bit—. **La medida está en SPEC 3, H-2**.

**7. 🔴 REFUTADO — y se deja escrito en vez de borrarse.** Esta spec publicaba que **ningún arnés EJECUTA
`Esclavo/src/reloj.cpp`**. Falso, medido sobre `606ba78`: `Validacion_Automatico/compilar_degradado.ps1`
**enlaza el `reloj.cpp` REAL de las dos puntas** con `-DARNES_RELOJ_REAL`. Lo que sí sigue sin ejercicio es
otra cosa, y está en **SPEC 3, H-1**.

**8. ⚠️ Nada de este capítulo ha visto una tarjeta.** `D-31`, `D-26` y el `$EVENT` de `D-32` (2) están en `main` **SIN BANCO**; qué firmware corre en cada equipo lo dice `ESTADO.md`, no este fichero.

**9. 🔴 NADIE MIRA LA LUZ DE LA OTRA PUNTA ENTRE LOS DOS VENCIMIENTOS** (§9). `costura_09` comprueba que los
reintentos **caben** bajo el techo de orfandad; **ninguna prueba mira qué luz tiene el Esclavo en el tramo que
esa desigualdad crea**, y `puedeSostenerVerde()` sólo protege el verde de ESTA punta. Derivado de las constantes
del C++ y **no ejercido en banco**: se escribe como hueco, no como comportamiento medido.
