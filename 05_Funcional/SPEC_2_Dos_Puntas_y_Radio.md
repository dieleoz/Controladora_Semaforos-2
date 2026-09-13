# SPEC 2 — LAS DOS PUNTAS Y LA RADIO

> **EL REQUISITO, Y ES EL ÚNICO QUE IMPORTA AQUÍ: LOS DOS POSTES NUNCA DAN VERDE A LA VEZ.** No hay cable
> entre ellos. Lo único que los une es una radio lenta que pierde tramas, y esta especificación es lo que el
> equipo hace **con esa radio sana, degradada y muerta**.

**Qué NO entra:** el ciclo de un poste solo (SPEC 1) · la hora y su siembra (SPEC 3) · la app (SPEC 4) · el
cobre, conectores y radios como aparato (SPEC 5).
**De dónde sale lo que aquí se afirma:** `DECISIONES.md` (vigentes) manda sobre lo decidido; el fuente, sobre
lo que el equipo HACE; los manuales anteriores no son fuente. **Ninguna cifra vive aquí**: se cita el SÍMBOLO
y dónde vive, porque un número copiado a mano nace caducado.

## 1. EL REPARTO — quién decide qué

| | |
|---|---|
| **Poste 1 (Maestro)** | El ÚNICO que ordena verde. Su máquina de estados: `Maestro/src/coordinador.cpp` (`EstadoCoord`) |
| **Poste 2 (Esclavo)** | Obedece y ACUSA. No tiene coordinador: su lado de la radio es la cadena `if/else` de `Esclavo/src/main.cpp`, **único lector de radio** de esa punta |
| **Repetidor** | `Repetidor/src/main.cpp`. Puente transparente con validación de CRC: **no origina ni una trama** |

**La única excepción a «el Maestro ordena» es el Modo Degradado** (§7): allí no hay radio y cada punta decide
su luz por reloj. Es el único modo que enciende un verde **sin confirmación del otro extremo**, y por eso es
el que más condiciones tiene delante.

**Contrato de la radio:** `{Maestro,Esclavo}/include/protocolo.h` — **idéntico byte a byte en las dos
puntas**, y lo compara `costura_01_contratos`. La trama es `RF_Packet`: identificador, comando, **un solo
byte** de parámetro y CRC. Cada envío sale en ráfaga de `RF_BURST_COPIES` copias y el receptor descarta las
repetidas por identificador (`protocolo.cpp`).

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

Del Poste 2 salen por iniciativa propia **tres** tramas —`CMD_DEMANDA`, `CMD_AMBAR_ESCLAVO`,
`CMD_CANCELA_AMBAR_ESCLAVO`—; todo lo demás que emite es respuesta.

### 2.2 El paso de verde a verde, que es donde se mata a alguien

Un cambio de sentido pasa SIEMPRE por todo-rojo, y el todo-rojo se cuenta **desde el acuse del otro lado, no
desde la orden** (`N-162`). El plazo es `tiempoDespejeMs` (`coordinador_configurar()`).

```
Verde Maestro -> C_MASTER_A_ROJO -> C_ESPERA_ESTATICO_TRAS_MASTER  (despeje)
              -> GO_GREEN -> C_ESPERANDO_ACK_GREEN -> ACK_GREEN -> Verde Esclavo
Verde Esclavo -> GO_RED -> C_ESPERANDO_ACK_RED -> ACK_RED
              -> C_ESPERA_ESTATICO_TRAS_ESCLAVO (despeje) -> Verde Maestro
```

**Las cuatro puertas al verde propio del Maestro llevan el mismo veto** (`puedeSostenerVerde()`):
`C_INICIAL_ESPERA_ESTATICO`, `C_ESPERA_ESTATICO_TRAS_ESCLAVO` y las dos ramas de `coordinador_pedirCambio()`
desde `QV_NINGUNO`. **El Maestro no abre su verde contando con que la orden de rojo LLEGÓ, sino con que se
CUMPLIÓ**: `rojoEsclavoConfirmado` se pone en un solo sitio —la llegada de `CMD_ACK_RED`, y nunca con un
`GO_GREEN` en vuelo— y se baja en cada orden de rojo de entrada, al mandar verde y al arrancar. Sin ella, un
solo `GO_RED` perdido con el Esclavo en verde dejaba verde en las dos puntas mientras los latidos le
refrescaban la orfandad.

### 2.3 El latido tiene DOS formas, y no es un detalle

Cada `LATIDO_MS` (`coordinador.cpp`), y **suprimido mientras se espera un ACK** (`SFTY-13`, colisión en el bus
medio dúplex) y mientras corre la sincronización:

- si el rojo del otro lado **no consta** (`quienVerde == QV_NINGUNO && !rojoEsclavoConfirmado`),
  en `C_MENU_IDLE` o en `C_FALLO` → sale **`CMD_GO_RED`** y se espera `CMD_ACK_RED`;
- en cualquier otro caso → sale `CMD_PING` y se espera `CMD_PONG`.

Es decir: **el reintento del todo-rojo de emergencia es el propio latido.** Sin esto, el rojo total —el de
entrar en Manual, el de emergencia— no tenía ningún reintento.

### 2.4 Lo que el Poste 2 hace al recibir

`CMD_GO_RED` y `CMD_GO_GREEN` refrescan `tUltimoComando` (el reloj de orfandad); las tramas de servicio y el
ámbar ordenado **no lo refrescan**, a propósito: significan «hay portadora», no «el Maestro está gobernando el
cruce». **Los dos vetos del ámbar de emergencia (`D-8`) viven en esas dos ramas**, y son `!mando_ambarLocal()
&& !bluetooth_ambarEmergencia()`. Vetado, **ni se obedece ni se ACUSA**: acusar un rojo que no se ha encendido
dejaría al Maestro dando verde convencido de que aquí hay rojo. Los dos se guardan igual porque *se guarda lo
que ABRE PASO, no lo que lo para*.
La orden de verde es **idempotente**: repetida sobre una luz que ya está en ámbar de transición o en verde,
**re-acusa y no toca la luz** (`N-162`). Reiniciar la transición dejaba al Esclavo sin llegar nunca a verde
mientras duraran los reintentos.

## 3. `SFTY-7` — LOS REINTENTOS

`TIMEOUT_ACK_MS` y `CICLO_MAX_REINTENTOS` (`Maestro/src/coordinador.cpp`). Agotados:

- desde `C_ESPERANDO_ACK_GREEN` → `C_FALLO` **con alarma** `FALLO_RF / REINTENTOS_AGOTADOS`;
- desde `C_ESPERANDO_ACK_RED` → `C_FALLO` **sólo si hay enlace y queda margen de silencio**. Si no,
  se sigue pidiendo el rojo y la caída la reporta `SFTY-6`, que es de quien es: esa puerta no lleva
  alarma propia, y por eso no puede taparle la suya a `SFTY-6`.

**Contra la distancia no sirve esperar, sirve repetir**: la distancia sube la **pérdida**, no la latencia. La
palanca correcta es `RF_BURST_COPIES`, no el timeout.

## 4. `SFTY-6` — EL SILENCIO Y EL ÁMBAR DE HUÉRFANO

El umbral es `SFTY6_SILENCIO_MS`, y vive **una sola vez**, en `protocolo.h`, porque gobierna las dos puntas:
tres copias mantenidas a mano fue como se desincronizaron (`N-69`).

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
directo, marca `verdeSoltadoPorMargen` y **sale a `C_IDLE`** —no a una espera de ACK— para no tocar la
cadencia del latido: medido, la otra salida producía **más** ámbares en microcortes que se recuperan. **El
umbral NO se baja**, y es condición del responsable: bajarlo manda el cruce a ámbar cada dos por tres cuando
llueve.

**Vuelta del enlace:** `SFTY-9` reencola la hora y la configuración del ciclo, fuerza rojo, pide `CMD_GO_RED`
y **espera su acuse** antes de contar despeje. Y si lo que hubo fue una suelta por margen, se reanuda por esa
misma puerta en cuanto `puedeSostenerVerde()` vuelve a ser cierto: un todo-rojo de minutos en una vía
alternada no es estado seguro, es donde la gente se pasa el rojo.

## 5. LAS BARRERAS QUE IMPIDEN VERDE + VERDE — la lista entera

1. **El Esclavo no enciende verde sin `CMD_GO_GREEN`.** Fuera del Degradado no hay otra vía.
2. **El Maestro no enciende verde sin `CMD_ACK_RED` del otro lado** (`rojoEsclavoConfirmado`).
3. **Ninguna punta abre NI SOSTIENE verde sin margen de silencio** — `puedeSostenerVerde()`, en
   las cuatro puertas de apertura y en `coordinador_actualizar()`.
4. **Backstop del Esclavo**: `MAX_VERDE_BACKSTOP_MS` corta un verde eterno mirando **la luz**, no
   la orden — porque en Degradado el verde no lo ordena nadie por radio.
5. **En Degradado, la fase la calcula la MISMA función en las dos puntas**: `ciclo_degradado_fase()`
   en `{Maestro,Esclavo}/include/ciclo_degradado.h`, que **debe ser idéntico** —dos
   implementaciones que «hacen lo mismo» es como se acaba con verde en las dos—, con su **guarda
   de medianoche**: el último tramo del día y el primero del siguiente son siempre despeje, porque
   el salto de fin de día podía comerse un todo-rojo.
6. Y la que no es de la radio: **`SFTY-2`** — sólo `semaforo.cpp` escribe pines de luz.

## 6. EL ÁMBAR DE EMERGENCIA DEL POSTE 2, Y SU ACUSE (`D-31`)

**Qué avería cierra:** si se rompe **sólo el transmisor** del Poste 2 —oye pero no habla—, esa punta no tiene
forma de saberlo: su único dato de radio es el silencio de lo que RECIBE, y el Maestro le sigue hablando. El
técnico veía un `$ACK` idéntico al de la radio sana, se iba, y el Poste 1 seguía dando verde hacia un carril
cuyo otro extremo está en ámbar. **Y muerde en el caso COMÚN**: la rama principal del ámbar de emergencia es
`if (!degradado_gobiernaLuz())`, o sea operación normal obedeciendo al Maestro.

**Armar** (dos puertas en `Esclavo/src/bluetooth.cpp`, sin PIN y con PIN, con el **mismo bloque letra por
letra**): ámbar aquí, latch `ambarEmergencia` puesto, `CMD_AMBAR_ESCLAVO` al Poste 1, y `$ACK` inmediato al
teléfono. **El ámbar no espera al acuse** — bloquear el bucle por una radio lenta es peor (`N-130`).

**El Poste 1** anota el aviso, **contesta `CMD_ACK_AVISO_AMBAR`** y se va a `MODO_AMBAR`: para de ciclar,
ordena rojo y calla. El acuse promete **una sola cosa: que esta punta oyó**; no que el cruce se pare. Se emite
en `coordinador_actualizar()` —o sea, con el Maestro **ciclando**, que es cuando tiene permiso para hablar—,
nunca en `coordinador_escucharEnAmbar()`.

> 🔴 **EL ACUSE SE ESPERA SÓLO DEL PRIMER AVISO, Y ESO ES LA REGLA, NO UNA SIMPLIFICACIÓN.** El aviso es justo
> lo que manda al Maestro a `MODO_AMBAR`, donde **tiene prohibido transmitir** (`SFTY-21`;
> `coordinador_escucharEnAmbar()` escucha y calla). Esperar acuse en cada pulsación diría «nadie me oyó» **con
> la radio sana** cada vez que alguien pulsa dos veces —que es lo normal en un poste— y eso enseña al técnico
> a ignorar la única señal que esto existe para darle.

**Plazo y reintento:** `AVISO_AMBAR_TIMEOUT_MS` (`protocolo.h`), que un `static_assert` de `coordinador.cpp`
obliga a ser el mismo `TIMEOUT_ACK_MS`; y `AVISO_AMBAR_REINTENTOS`, que **vale TRES desde `D-32` (3), 13/09** y **está ELEGIDO, no
derivado** (§HUECOS, 1). Agotado, sale `$ALARM AVISO_RF / SIN_CONFIRMAR / AVISE_POSTE_1`, y **dice «no he
podido confirmarlo», nunca «el otro poste no se enteró»**: con un reintento de por medio, una trama perdida se
ve igual que un transmisor roto.

**Cancelar** borra el latch y **la memoria del acuse en la misma sentencia**, y para el plazo en vuelo. Armado
y cancelado son la misma máquina: sin ese borrado, el siguiente ámbar diría «el Poste 1 ya lo sabe» apoyándose
en un acuse de un ámbar que ya no existe. Si queda el latch del mando, **no se avisa al Maestro**: esta punta
sigue en ámbar. **`CMD_CANCELA_AMBAR_ESCLAVO` NO se acusa, y la asimetría es deliberada:** su camino normal es
con el Maestro ya callado en `MODO_AMBAR`, donde un acuse le obligaría a hablar justo en el modo en que calla;
su red es que la **segunda pulsación reenvía**. El Maestro sólo sale del ámbar **que pidió el Esclavo**
(`modo_ambar_origenEsclavo()`) y sale **a todo-rojo**, no al ciclo: «ya no retengo el ámbar» no es «ya se
puede pasar».

## 7. `SFTY-21` — EL MODO DEGRADADO

**Sin radio, la luz la decide el reloj.** La entrada es **MANUAL, siempre**: el equipo no entra solo. En el
Poste 2 la llave es la app (`D-18`, `SET_MODO:DEGRADADO`), y **pasa por la puerta única que ya existía**,
`degradado_entrar()`; en el Poste 1 se entra por `modo_degradado_setup()`.

**Condiciones, en `degradado_comprobar()` (`Esclavo/src/modo_degradado.cpp`), y devuelve MOTIVO, no un sí/no**
—quien la llame tiene que poder decirle al operario qué le falta:

| rechazo | qué falta |
|---|---|
| `DEG_RECHAZO_SIN_HORA` | `reloj_horaFiable()` es falso. **Fiable, no sólo puesta** |
| `DEG_RECHAZO_SIN_CONFIG` / `CICLO_NULO` | el Maestro no dejó verde y despeje verificados |
| `DEG_RECHAZO_SIN_SYNC` | no hubo ni una sincronización en esta sesión |
| `DEG_RECHAZO_SYNC_VENCIDA` | la que hubo caducó: el límite duro no es un botón de posponer |
| `DEG_RECHAZO_AMBAR_VIGENTE` | hay un ámbar de emergencia puesto por una persona |

**Dentro:** todo-rojo de entrada (`rojoObligatorioMs()`, con suelo `ROJO_MINIMO_MS`) y después verde **sólo**
en la fase propia. El despeje es el **valor final que mandó el Maestro, ya ampliado en el origen**: si cada
punta lo escalara por su cuenta, los verdes se solaparían durante minutos. **`D-26` (4) — un salto de hora se
aplica PASANDO POR ROJO**: si `saltoDeHora()` supera el margen del cruce, rojo en esa misma vuelta y el
todo-rojo se cuenta de nuevo. Construido en las dos puntas.

**Las cuatro salidas, y ninguna admite marcha atrás** (`iniciarSalida()` baja el indicador de la pila **al
empezar**, no al terminar):

1. el operario;
2. **el regreso de la radio** — sólo con tramas de **gobierno** (`PING`, `GO_RED`, `GO_GREEN`); las de servicio no sacan del modo, porque significan «hay portadora», no «alguien gobierna»;
3. **el límite duro** `LIMITE_SIN_SYNC_MS` sin sincronizar, con aviso previo en `AVISO_SIN_SYNC_MS`;
4. **`D-21` (1): la hora dejó de ser fiable en marcha.**

> **Las dos puntas acaban en ámbar, y sólo el Esclavo pasa por el despeje**, y es lo correcto: el Maestro
> llama a `irAAmbar()` y va directo; el Esclavo llama a `iniciarSalida(true)` —rendición—, que fuerza
> todo-rojo y entra en `DEG_RENDIDO`. Salir de un verde directo a intermitente le dice al que viene lanzado
> que negocie el paso creyendo que aún tiene prioridad.

**De `DEG_RENDIDO` no se sale solo**: hace falta una orden, o una sincronización nueva
(`degradado_registrarSync()`). Reentrar sigue siendo decisión del operario. **`D-29`:** un corte de luz ya no
mata la reanudación — el permiso de la pila se conserva hasta que la primera siembra del arranque pueda llegar
(`VENTANA_REANUDACION_MS`, derivada del mismo instante en que el firmware da por muda la siembra). La
**segunda puerta —el límite duro— no se toca**, y con el cristal `Y2` muerto es ella la que cierra: `D-29`
sólo alcanza a las tarjetas cuyo `Y2` oscila.

## 8. EL PRESUPUESTO DE RADIO — Y CUÁL ES EL BORDE DE CADA UNO

> 🔴 **EN ESTE REPOSITORIO HAY DOS CUENTAS DE RADIO Y NO SE PUEDEN COMPARAR: MIDEN COSAS DISTINTAS CONTRA
> BORDES DISTINTOS.** Confundirlas ya pasó una vez. Cada una se escribe con su borde al lado (`CLAUDE.md` §7).

**(A) El presupuesto de verdad — `PRESUPUESTO_LIBRE_MS`.** Lo que queda libre por debajo del techo de orfandad
**crudo**, `SFTY6_SILENCIO_MS`, tras el peor caso del ciclo: la cadencia del latido más `CICLO_MAX_REINTENTOS`
pasos, cada uno con su tiempo de cable. **Es lo único asignable**: quien quiera gastar aire tiene que caber
aquí. Lo recalcula `costura_09_presupuesto_radio` en cada corrida leyendo las cuatro constantes del C++. *Por
qué ese borde:* quien primero llegue manda, y el que llega primero es el ámbar por orfandad — si el
presupuesto lo desborda, los últimos reintentos son código muerto, que es lo que pasaba antes de `N-71`.

**(B) La desigualdad del punto de suelta — el `static_assert` de `N-163`.** Compara el mismo peor caso **más
un `TIMEOUT_ACK_MS`** contra `SFTY6_SILENCIO_MS`. **NO es un presupuesto y su holgura NO es de nadie.** Su
borde es otro: el punto en que esta punta **suelta el verde**, `SFTY6_SILENCIO_MS − TIMEOUT_ACK_MS`. *Por qué
ese borde:* adelantar la suelta no puede recortar el presupuesto de reintentos, o con lluvia el cruce se iría
a ámbar de más.
**Las dos son legítimas: no se restan, no se comparan y no comparten holgura.** Un tercer consumidor de aire
se mide contra **(A)**, nunca contra **(B)**.

**El aire:** `RF_BURST_COPIES` copias por envío y la tasa aérea fijan el tiempo de cable de cada trama; ese
coste está a la vista en `coordinador.cpp` (`ENVIO_TRAMA_MS`) y en `costura_09`, y **no se lee de ninguna
constante del firmware porque no existe como tal**. Los parámetros del módulo son de SPEC 5; esta spec sólo
exige que **las dos puntas y el repetidor estén configurados igual**, o el CRC no casa y el síntoma es «el
equipo se calló».

## HUECOS MEDIDOS

**1. 🔴 `AVISO_AMBAR_REINTENTOS` está ELEGIDO, no derivado — y CUATRO sitios del fuente siguen diciendo lo
contrario.** Medido el 12/09; escrito en `D-31` y en `roadmap.md` 1.31. Los dos `static_assert` de
`coordinador.cpp` cobran el **coste unitario** del modelo serie con el **recuento** del modelo paralelo: bajo
el modelo del párrafo que los precede —las esperas del aviso corren en paralelo y sólo se serializa el cable—
caben decenas; bajo el modelo serie completo no cabe ninguno y el valor tendría que ser cero, que anula `D-31`
entera. **«Con dos no compila y con cero tampoco» no demuestra nada**: las dos mitades salen del mismo híbrido.
**El techo de orfandad AGUANTA y no hay defecto vivo**; lo que se cae es la justificación. La restricción real
no es el aire: es **cuánto tarda el Poste 2 en declarar que el otro no contesta**, y ese equilibrio —menos
falsas alarmas contra una alarma verdadera más tardía— **era del responsable y YA ESTÁ CONTESTADO: `D-32` (3), 13/09
— «mas reintentos, 14 o mas» → `AVISO_AMBAR_REINTENTOS` = 3, o sea 14 s. CONSTRUIDO ese día**, y con él la
guarda rehecha contra el modelo que el firmware implementa (una sola, `(1+N) × 2 × ENVIO_TRAMA_MS`, vista
fallar en 35). Este hueco queda CERRADO. **Corregido en `Maestro/src/coordinador.cpp`; SIN corregir** en la cabecera de
`AVISO_AMBAR_REINTENTOS` de `protocolo.h` (*«UNO, Y EL NUMERO ESTA DERIVADO, NO ELEGIDO»* — y el fichero es
idéntico en las **dos** puntas, así que la frase está dos veces) y en **dos** párrafos de
`Esclavo/src/bluetooth.cpp`. Nada de esto lo ve la compuerta.

**2. 🔴 Los dos presupuestos de radio no casan sobre el mismo techo: fila pendiente, no detalle.** Escrito en
`D-31` (3) y en `coordinador.cpp`. §8 los separa por su borde; **elegir el modelo sigue sin hacerse**.

**3. 🔴 `D-30` está decidida y NO construida, y muerde exactamente aquí.** El veto `!mando_ambarLocal()` de la
rama `CMD_GO_GREEN` del Esclavo es **la única de las seis llamadas vivas que ABRE PASO**: retirar el armador
sin resolver los seis lectores no deja ese `if` inerte, **lo deja abierto** y el `GO_GREEN` se obedece. Con el
mando desmontado la bandera no se arma nunca y hoy la guarda deja pasar: el riesgo es del trabajo pendiente.

**4. ⚠️ El `CMD_ACK_RED` no lleva a qué `CMD_GO_RED` contesta** —escrito en el propio `coordinador.cpp`—: un
acuse retenido en el aire más allá del último `GO_GREEN` y soltado tras una orden de rojo perdida **sería
indistinguible del bueno**. `rojoEsclavoConfirmado` acota el caso —no se pone con un `GO_GREEN` en vuelo— pero
no lo cierra. Residual conocido, sin decisión.

**5. ⚠️ La rama `CMD_PING` del Maestro no tiene emisor.** Censado el 12/09 sobre
`{Maestro,Esclavo,Repetidor}/src`: el Esclavo emite `CMD_PONG` y **nunca** `CMD_PING`, y el Repetidor no
origina tramas. El `protocolo_enviarPaquete(CMD_PONG)` de `coordinador_actualizar()` contesta a algo que hoy no
manda nadie —§6, declarado y no ejercido—. Inofensivo para la luz; se deja escrito en vez de tocarse.

**6. ⚠️ `D-21` pieza (A) sigue sin construir en el STM32.** `OSF` no aparece en
`{Maestro,Esclavo}/{src,include}` fuera de un comentario de `Maestro/include/reloj.h` que dice exactamente eso
(`grep OSF` a secas cuenta `MOSFET`: filtrarlo); el bit **sí** se interpreta en el ESP32
(`ESP32_Expansion/src/reloj_ds3231.cpp`). Toca aquí por su consecuencia: el Degradado se autoriza sobre
`reloj_horaFiable()`, que no ve el `OSF`. El detalle es de SPEC 3.

**7. ⚠️ Ningún arnés EJECUTA `Esclavo/src/reloj.cpp`**, y la frontera de silencio que decide si la hora la
manda la radio se mide **por texto** (`D-26`). Un pack de texto no ve un defecto del tiempo.

**8. ⚠️ Nada de este capítulo ha visto una tarjeta.** `D-31` y `D-26` están en `main` **sin banco**. Qué
firmware corre en cada equipo lo dice `ESTADO.md`, no este fichero.
