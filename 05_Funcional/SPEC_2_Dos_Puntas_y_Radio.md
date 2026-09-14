# SPEC 2 — LAS DOS PUNTAS Y LA RADIO

> **EL REQUISITO, Y ES EL ÚNICO QUE IMPORTA AQUÍ: LOS DOS POSTES NUNCA DAN VERDE A LA VEZ.** No hay cable entre
> ellos. Lo único que los une es una radio lenta que pierde tramas, y esta especificación es lo que el equipo hace
> **con esa radio sana, degradada y muerta**.

**Qué NO entra:** el ciclo de un poste solo (SPEC 1) · la hora (SPEC 3) · la app (SPEC 4) · el cobre y las radios como
aparato (SPEC 5) · el procedimiento de campo y el parche `T-2` (SPEC 6). **De dónde sale:** `DECISIONES.md` manda en lo
decidido y el fuente en lo que el equipo HACE; los manuales no son fuente. **Ninguna cifra vive aquí**: se cita el
SÍMBOLO. **Todo lo medido está sobre `ef3504c` con el árbol limpio** — el firmware lo tocan otros agentes.

## 1. EL REPARTO — quién decide qué

| | |
|---|---|
| **Poste 1 (Maestro)** | El ÚNICO que ordena verde. Su máquina de estados: `Maestro/src/coordinador.cpp` (`EstadoCoord`) |
| **Poste 2 (Esclavo)** | Obedece y ACUSA. No tiene coordinador: su lado de la radio es la cadena `if/else` de `Esclavo/src/main.cpp`, **único lector de radio** de esa punta |
| **Repetidor** | `Repetidor/src/main.cpp`. Puente transparente con validación de CRC: **no origina ni una trama** |

**La única excepción a «el Maestro ordena» es el Modo Degradado** (§7): allí no hay radio y cada punta decide su luz
por reloj. Es el único modo que enciende un verde **sin confirmación del otro extremo**, y por eso es el que más
condiciones tiene delante — y el único que exige un ACUERDO CERRADO ANTES por radio (§8). **Contrato de la radio:**
`{Maestro,Esclavo}/include/protocolo.h`, **idéntico byte a byte en las dos puntas** (`costura_01_contratos`). La trama
es `RF_Packet` —identificador, comando, **un solo byte** de parámetro y CRC— y cada envío sale en ráfaga de
`RF_BURST_COPIES` copias, con el receptor descartando las repetidas por identificador.

## 2. EL DIÁLOGO DEL CICLO

### 2.1 Los comandos, y quién los emite

| comando | dirección | qué es |
|---|---|---|
| `CMD_GO_GREEN` / `CMD_ACK_GREEN` | M→E / E→M | la única orden que abre paso en el Poste 2, y su «verde encendido y estable» |
| `CMD_GO_RED` / `CMD_ACK_RED` | M→E / E→M | la orden que cierra —también es el latido cuando el rojo no consta— y su «estoy en rojo» |
| `CMD_PING` / `CMD_PONG` · `CMD_GO_AMBAR` | M→E / E→M | el latido, que mide que el enlace vive · y el ámbar ordenado (`N-134`), detrás de un `CMD_GO_RED` previo |
| `CMD_AMBAR_ESCLAVO` / `CMD_ACK_AVISO_AMBAR` | E→M / M→E | el Poste 2 avisa de su ámbar de emergencia, y el acuse (`D-31`) |
| `CMD_CANCELA_AMBAR_ESCLAVO` | E→M | el Poste 2 lo retira. **No se acusa, y es la regla** (§6) |
| `CMD_DEMANDA` / `CMD_ACK_DEMANDA` · `CMD_HORA_*` | E→M / M→E | la demanda de cámara y su respuesta, que dice si se atiende · y la hora, que **no toca luz** (SPEC 3) |
| `CMD_CONFIG_*`, `CMD_DELTA*` | M→E | 🔴 **NO son «servicio»: son el ACUERDO que autoriza el Degradado** (§8) |

Del Poste 2 salen por iniciativa propia **tres** —`CMD_DEMANDA`, `CMD_AMBAR_ESCLAVO`, `CMD_CANCELA_AMBAR_ESCLAVO`—; el resto es respuesta.

### 2.2 El paso de verde a verde, que es donde se mata a alguien

Un cambio de sentido pasa SIEMPRE por todo-rojo, contado **desde el acuse del otro lado, no desde la orden**
(`N-162`); el plazo es `tiempoDespejeMs` (`coordinador_configurar()`).
`Verde M -> C_MASTER_A_ROJO -> C_ESPERA_ESTATICO_TRAS_MASTER (despeje) -> GO_GREEN -> C_ESPERANDO_ACK_GREEN ->
ACK_GREEN -> Verde E -> GO_RED -> C_ESPERANDO_ACK_RED -> ACK_RED -> C_ESPERA_ESTATICO_TRAS_ESCLAVO -> Verde M`
**Las cuatro puertas al verde propio del Maestro llevan el mismo veto** (`puedeSostenerVerde()`):
`C_INICIAL_ESPERA_ESTATICO`, `C_ESPERA_ESTATICO_TRAS_ESCLAVO` y las dos ramas de `coordinador_pedirCambio()` desde
`QV_NINGUNO`. **El Maestro no abre su verde contando con que la orden de rojo LLEGÓ, sino con que se CUMPLIÓ**:
`rojoEsclavoConfirmado` se pone en un solo sitio —la llegada de `CMD_ACK_RED`— y se baja en cada orden de rojo, al
mandar verde y al arrancar. Sin ella, un `GO_RED` perdido con el Esclavo en verde dejaba verde en las dos puntas.

**El latido tiene DOS formas, y no es un detalle.** Cada `LATIDO_MS` (`coordinador.cpp`), **suprimido mientras se espera un ACK** (`SFTY-13`, colisión en el bus medio
dúplex) y mientras corre la sincronización: si el rojo del otro lado **no consta** (`quienVerde == QV_NINGUNO &&
!rojoEsclavoConfirmado`), en `C_MENU_IDLE` o en `C_FALLO` → sale **`CMD_GO_RED`** y se espera `CMD_ACK_RED`; en
cualquier otro caso → `CMD_PING` y se espera `CMD_PONG`. Es decir: **el reintento del todo-rojo de emergencia es el
propio latido** — sin esto no tenía ninguno. ⚠️ **Y esa supresión hace que el contador de silencio de la otra punta
llegue ya envejecido:** §9 y SPEC 6 D.1.

### 2.3 Lo que el Poste 2 hace al recibir

`CMD_GO_RED` y `CMD_GO_GREEN` refrescan `tUltimoComando` (el reloj de orfandad); las tramas de servicio y el ámbar
ordenado **no lo refrescan**, a propósito: significan «hay portadora», no «el Maestro está gobernando el cruce».
Vetado, **ni se obedece ni se ACUSA**: acusar un rojo que no se ha encendido dejaría al Maestro dando verde convencido
de que aquí hay rojo — *se guarda lo que ABRE PASO, no lo que lo para*. La orden de verde es **idempotente**: repetida
sobre una luz ya en ámbar de transición o en verde, **re-acusa y no toca la luz** (`N-162`).

> 🔴 **`D-8` DICE «DOS VETOS» Y EN EL FUENTE HAY TRES `if`. LAS DOS CUENTAS SON CIERTAS Y CUENTAN COSAS DISTINTAS,
> y confundirlas es lo que hace que se lean como una sola.** **DOS por SUJETO**, que es
> como lo escribe `D-8`: el veto del **mando** (`mando_ambarLocal()`) y el de la **app**
> (`bluetooth_ambarEmergencia()`); dos personas distintas pueden poner ese ámbar. **TRES por RAMA**, que es lo que
> hay que guardar: los dos sujetos aparecen **juntos, en la misma condición, en tres `if` de
> `Esclavo/src/main.cpp`** — la rama `CMD_GO_RED`, la rama `CMD_GO_GREEN`, y una **tercera** que es la recuperación
> tras fallo (`semaforo_estado() == S_FALLO && pkt.command == CMD_GO_RED`). **Esa tercera NO es un `else` de la
> primera:** un `GO_RED` vetado arriba entra por ella y volvería a forzar rojo por su cuenta. El fuente lo avisa
> —*«guardar solo una de las dos deja la revocación intacta»*— y `SPEC 5` §3 tiene la cuenta de tres. Es
> `CLAUDE.md` §2: **una regla que ENUMERA sujetos comprueba que cada sujeto EXISTE, y dónde se ejerce.**
> `grep -c "mando_ambarLocal() && !bluetooth_ambarEmergencia" 01_Firmware/Esclavo/src/main.cpp`

## 3. `SFTY-7` — LOS REINTENTOS

`TIMEOUT_ACK_MS` y `CICLO_MAX_REINTENTOS` (`Maestro/src/coordinador.cpp`). Agotados: desde `C_ESPERANDO_ACK_GREEN` →
`C_FALLO` **con alarma** `FALLO_RF / REINTENTOS_AGOTADOS`, **y sin mirar el reloj del silencio** —esa asimetría es la
que abre el hueco de §9—; desde `C_ESPERANDO_ACK_RED` → `C_FALLO` **sólo si hay enlace y queda margen de silencio**, y
si no se sigue pidiendo el rojo y la caída la reporta `SFTY-6`, cuya alarma esta puerta no puede tapar. **Contra la
distancia no sirve esperar, sirve repetir**: sube la **pérdida**, no la latencia, y la palanca es `RF_BURST_COPIES`.

## 4. `SFTY-6` — EL SILENCIO Y EL ÁMBAR DE HUÉRFANO

El umbral es `SFTY6_SILENCIO_MS` y vive **una sola vez**, en `protocolo.h`, porque gobierna las dos puntas: tres
copias mantenidas a mano fue como se desincronizaron (`N-69`).

| punta | ancla del silencio | qué hace al vencer |
|---|---|---|
| Maestro | `tUltimaRxEsclavo` — **la respuesta que le CONTESTARON** | `C_FALLO`, ámbar intermitente, alarma `FALLO_RF / SILENCIO_…`, y `CMD_GO_RED` **en ese mismo instante** |
| Esclavo | `tUltimoComando` — **la última orden que RECIBIÓ** | `semaforo_iniciarFallo()`, ámbar intermitente, misma alarma |

> 🔴 **LOS DOS UMBRALES SON EL MISMO NÚMERO Y LOS DOS INSTANTES NO.** Entre «la orden llega al Esclavo» y «su respuesta
> llega al Maestro» hay el retardo de cortesía (`RETARDO_RESPUESTA_MS`, `SFTY-17`) más el viaje de vuelta, y **ese
> desfase era la ventana**: con la dirección Maestro→Esclavo muerta, el Poste 2 se iba a su ámbar **antes** de que el
> Poste 1 apagara su verde — verde contra ámbar con la pluma del otro poste arriba.

**Se cierra soltando el verde propio un margen ANTES** (`N-163`): `puedeSostenerVerde()` adelanta la decisión en
`TIMEOUT_ACK_MS`, que es lo que ya acota el viaje de ida y vuelta. Al soltarlo la punta va a rojo directo, marca
`verdeSoltadoPorMargen` y **sale a `C_IDLE`** —no a una espera de ACK— para no tocar la cadencia del latido: medido, la
otra salida producía **más** ámbares en microcortes que se recuperan. **El umbral NO se baja**, y es condición del
responsable. **Lo que ese margen NO cubre —el verde de la OTRA punta cuando ésta ya cayó— es §9.** **Vuelta del
enlace:** `SFTY-9` reencola la hora y la configuración, fuerza rojo, pide `CMD_GO_RED` y **espera su acuse** antes de
contar despeje; tras una suelta por margen se reanuda por esa misma puerta, porque un todo-rojo de minutos en una vía
alternada no es estado seguro: es donde la gente se pasa el rojo.

## 5. LAS BARRERAS QUE IMPIDEN VERDE + VERDE — la lista entera

1. **El Esclavo no enciende verde sin `CMD_GO_GREEN`** —fuera del Degradado no hay otra vía— y **el Maestro no enciende
   el suyo sin `CMD_ACK_RED` del otro lado** (`rojoEsclavoConfirmado`).
3. **Ninguna punta abre NI SOSTIENE verde sin margen de silencio** — `puedeSostenerVerde()`, en las cuatro puertas de
   apertura y en `coordinador_actualizar()`. ⚠️ **Vigila el verde PROPIO, no el del otro** (§9).
4. **Backstop del Esclavo**: `MAX_VERDE_BACKSTOP_MS` corta un verde eterno mirando **la luz**, no la orden.
5. **En Degradado, la fase la calcula la MISMA función en las dos puntas**: `ciclo_degradado_fase()`, §8 (e).
6. Y la que no es de la radio: **`SFTY-2`** — sólo `semaforo.cpp` escribe pines de luz (SPEC 1 §2).

## 6. EL ÁMBAR DE EMERGENCIA DEL POSTE 2, Y SU ACUSE (`D-31`)

**Qué avería cierra:** roto **sólo el transmisor** del Poste 2 —oye pero no habla—, esa punta no puede saberlo (su
único dato de radio es el silencio de lo que RECIBE), el técnico veía un `$ACK` idéntico al de la radio sana y el Poste
1 seguía dando verde hacia un carril cuyo otro extremo está en ámbar. **Armar** (`Esclavo/src/bluetooth.cpp`): ámbar
aquí, latch `ambarEmergencia`, `CMD_AMBAR_ESCLAVO` al Poste 1 y `$ACK` inmediato al teléfono — **el ámbar no espera al
acuse** (`N-130`). **El Poste 1** anota el aviso, contesta `CMD_ACK_AVISO_AMBAR` desde `coordinador_actualizar()` —con
el Maestro **ciclando**, que es cuando tiene permiso para hablar— y se va a `MODO_AMBAR`, del que **sólo sale si el
ámbar lo pidió el Esclavo** y **sale a todo-rojo**. *(Literales, SPEC 4 §3.2; el campo, SPEC 6.)*

> 🔴 **EL ACUSE SE ESPERA SÓLO DEL PRIMER AVISO, Y ESO ES LA REGLA, NO UNA SIMPLIFICACIÓN.** El aviso es lo que manda al
> Maestro a `MODO_AMBAR`, donde **tiene prohibido transmitir** (`SFTY-21`), así que esperar acuse en cada pulsación
> diría «nadie me oyó» **con la radio sana** cada vez que alguien pulsa dos veces —lo normal en un poste—, y eso enseña
> al técnico a ignorar la única señal que esto existe para darle. **Plazo y reintento:** `AVISO_AMBAR_TIMEOUT_MS` (`protocolo.h`), que un `static_assert` de `coordinador.cpp` obliga a
ser el mismo `TIMEOUT_ACK_MS`, y `AVISO_AMBAR_REINTENTOS`, **elegido por `D-32` (3), no derivado**. Agotado sale
`$ALARM AVISO_RF / SIN_CONFIRMAR / AVISE_POSTE_1`, que **dice «no he podido confirmarlo», nunca «el otro poste no se
enteró»**. **Cancelar** borra el latch y **la memoria del acuse en la misma sentencia** —sin eso el siguiente ámbar
diría «el Poste 1 ya lo sabe» sobre un acuse que ya no existe—, y **`CMD_CANCELA_AMBAR_ESCLAVO` NO se acusa**: su
camino normal es con el Maestro ya callado en `MODO_AMBAR`, y su red es que la **segunda pulsación reenvía**.

## 7. `SFTY-21` — EL MODO DEGRADADO

**Sin radio, la luz la decide el reloj.**

> 🔴 **ENTRAR POR PRIMERA VEZ ES MANUAL. REANUDAR TRAS UN CORTE NO LO ES — Y NADIE SE LO AVISA AL OPERARIO.**
> - **La primera entrada sí la pide una persona:** Poste 2, la app (`D-18`) por la puerta única `degradado_entrar()`;
>   Poste 1, `SET_MODO:DEGRADADO` o el `A.B.A.B` del mando.
> - **La reanudación la decide la máquina**, en cada arranque y **sin pulsación ninguna** (`D-29`):
>   `degradado_reanudarTrasCorte()` (Esclavo) y `modo_degradado_reanudarTrasCorte()` (Maestro), llamadas desde
>   `setup()` **y desde el `loop()`** de cada punta —dos sitios, porque desde `N-162` la hora llega segundos después del
>   arranque y la decisión queda diferida hasta `VENTANA_REANUDACION_MS`—. El Esclavo entra por **`degradado_entrar()`,
>   la MISMA puerta del operario**; el Maestro hace `modoActual_set(MODO_DEGRADADO)` y cae en el `switch` común. **No
>   es un camino alternativo: es la entrada de siempre con el permiso recuperado de la pila.**
> - **Lo que el operario NO ve:** un equipo al que se le fue la luz vuelve solo al único modo que da verde sin
>   confirmar con el otro extremo, y **no hay `$EVENT` ni literal que lo anuncie** (HUECO 6).
> - **Lo que sí protege:** el permiso se tira si el equipo ya no está quieto donde lo dejó el arranque —una persona
>   eligió modo, o el modo ya gobierna—, si hay ámbar del mando puesto, o si falló cualquier condición de vigencia. Lo
>   que `D-29` difiere es **el borrado, no el límite duro**; la ventana, en SPEC 3 §6 y H-3.

**Condiciones del Poste 2, en `degradado_comprobar()`, y devuelve MOTIVO, no un sí/no** —quien la llame tiene que poder
decirle al operario qué le falta—: `DEG_RECHAZO_SIN_HORA` (`reloj_horaFiable()` falso: **fiable, no sólo puesta**) ·
`SIN_CONFIG` / `CICLO_NULO` · `SIN_SYNC` · `SYNC_VENCIDA` · `AMBAR_VIGENTE`; **los textos que LEE el operario y los
tres motivos que no existen en el Maestro, SPEC 6 A.2.** **Dentro:** todo-rojo de entrada (`rojoObligatorioMs()`, suelo
`ROJO_MINIMO_MS`) y después verde **sólo** en la fase propia, con el despeje que mandó el Maestro **ya ampliado en el
origen** —si cada punta lo escalara los verdes se solaparían durante minutos—; **`D-26` (4): un salto de hora se aplica
PASANDO POR ROJO.** **Las cuatro salidas, y ninguna admite marcha atrás** (`iniciarSalida()` baja el indicador de la
pila **al empezar**): el operario · **el regreso de la radio**, sólo con tramas de **gobierno** · **el límite duro**
`LIMITE_SIN_SYNC_MS`, con aviso en `AVISO_SIN_SYNC_MS` · **`D-21` (1)**. **Las dos acaban en ámbar y sólo el Esclavo
pasa por el despeje** (**SPEC 6 A.4**); de `DEG_RENDIDO` no se sale solo.

## 8. CÓMO SE PONEN DE ACUERDO LAS DOS PUNTAS SIN RADIO

> **El Degradado da verde sin confirmar con el otro extremo, pero no a ciegas: lo hace sobre un acuerdo cerrado
> ANTES, con la radio viva.** `modo_degradado_publicarConfig()` se llama al arrancar y su cabecera dice por qué:
> *«cuando el radio muera ya será tarde para acordarlo»*.

**(a) El reparto del ciclo — `CMD_CONFIG_VERDE` y `CMD_CONFIG_DESPEJE`.** `enviarConfigCiclo()` (`coordinador.cpp`) las
manda **seguidas**, con `cfgVerdeSeg` y `cfgDespejeSeg`; el Poste 2 las guarda en `config_ciclo.cpp` y **acusa el
CONJUNTO con un solo `CMD_ACK_CONFIG`** — se acusa lo aplicado, no cada trama suelta. **Y el flag de recibido no es el
valor:** `SIN_CONFIG` (nunca llegó) y `CICLO_NULO` (llegó un cero) son motivos distintos, porque un cero puede ser «el
Maestro dijo cero» o «no llegó nada» y entrar en el segundo caso es operar a ciegas.

**(b) La medida de desfase — `CMD_DELTA` / `CMD_DELTA_RESP` (`SFTY-23`).** `enviarPeticionDelta()` manda el **segundo
del Maestro leído DENTRO de la función del envío**: no existe variable donde guardarlo, para que una retransmisión no
reenvíe un segundo caducado. El Poste 2 contesta la diferencia en **complemento a dos en un byte** y la puerta la
compara contra `TOLERANCIA_DESFASE_S`. ⚠️ **Es la condición MÁS DÉBIL y por eso va la última:** la medida es
**circular** y **lee como cero todo múltiplo de 60 s** —60, 120 o 3600 s pasan la tolerancia mientras 45 s sí se
detecta—, así que lo que sostiene el modo es la **frescura** de la sync, no este número. Al fallar un intento **la
medida se suelta y la hora y la configuración NO** (`syncAbandonarIntento()`): diagnóstico contra seguridad.

**(c) La puerta del Poste 1 — `MotivoDegradado`.** `modo_degradado_evaluarEntrada()` devuelve **motivo, no sí/no**, y es
la MISMA para la app y para el `A.B.A.B`: `MDG_FALTA_HORA` · `MDG_NUNCA_SYNC` · `MDG_SYNC_VIEJA`
(`msDesdeSyncEfectivo() >= SYNC_FRESCA_MS`) · `MDG_SIN_CONFIG` (`coordinador_configConfirmada()`: **que el ESCLAVO haya
acusado el ciclo** — **se añadió porque faltaba**, y sin él el Maestro daba verde por reloj mientras el Esclavo
rechazaba y caía a ámbar por orfandad) · `MDG_SIN_DESFASE` · `MDG_DESFASE_ALTO`.

> 🔴 **(d) LOS DOS UMBRALES DE SINCRONIZACIÓN SON DISTINTOS POR PUNTA: LA MISMA ORDEN PUEDE ENTRAR EN UNA Y SER
> RECHAZADA EN LA OTRA.** El Poste 1 exige **frescura** —`SYNC_FRESCA_MS`, `Maestro/src/modo_degradado.cpp`—; el Poste 2
> sólo exige que **haya habido alguna** (`huboSyncAlguna`) y que **no haya vencido el límite duro**
> (`syncVencidaLatch`, armado en `LIMITE_SIN_SYNC_MS`). **`SYNC_FRESCA_MS` es mucho más corto**, así que entre los dos
> plazos el Poste 2 acepta `SET_MODO:DEGRADADO` y el Poste 1 lo rechaza con `MDG_SYNC_VIEJA`: **una punta en Degradado
> dando verde por reloj y la otra en su ciclo normal**, que se irá a ámbar por orfandad — el mismo escenario que
> `MDG_SIN_CONFIG` cierra en el sentido contrario, **abierto en éste**. **Nadie cruza esos dos números** —ni
> `static_assert` ni pack—, y viven en proyectos distintos (HUECO 7).
> `grep -n "SYNC_FRESCA_MS\|LIMITE_SIN_SYNC_MS" 01_Firmware/{Maestro,Esclavo}/src/modo_degradado.cpp`

**(e) El cálculo de fase — la misma función, no dos que «hacen lo mismo».** `ciclo_degradado_fase()` vive en
`{Maestro,Esclavo}/include/ciclo_degradado.h`, **que debe ser idéntico** (lo compara `costura_01_contratos`):
**`posicion = segundos_del_dia mod ciclo`, con `ciclo = 2 × (verde + despeje)`**, **anclado a la HORA DE PARED y no a un
contador propio** —dos equipos encendidos con un minuto de diferencia arrancarían desfasados un minuto entero—. **El
orden lo fija la función y no se negocia: la posición 0 es `FD_VERDE_MAESTRO`, o sea que EL POSTE 1 TIENE EL VERDE
PRIMERO**, y detrás van `FD_DESPEJE_A`, `FD_VERDE_ESCLAVO` y `FD_DESPEJE_B`. **Guarda de medianoche:** el último tramo
del día y el primero del siguiente son **siempre despeje**, porque a las 00:00:00 la posición salta y ese salto puede
caer en mitad de un verde y saltarse el todo-rojo. **Y la configuración imposible tiene respuesta escrita:** con verde
o despeje a cero devuelve `FD_DESPEJE_A` —todo-rojo—, no un caso «que no debería pasar».

## 9. LOS DOS RELOJES Y LOS DOS PRESUPUESTOS — todos cierran sobre `SFTY6_SILENCIO_MS`

> 🔴 **EL SILENCIO DE LA RADIO NO SIGNIFICA LO MISMO EN LOS DOS MOMENTOS DEL CICLO.** Al **pasar el testigo** hay que ser
> estricto y reintentar fuerte: ahí es donde alguien puede quedarse en verde mientras el otro también lo tiene. Con el
> cruce **quieto** —una en verde, la otra en rojo, nada que decidir— el mismo silencio **no abre ninguna vía de verde
> contra verde** y se tolera mucho más, con una condición: **no empezar un cambio sin haber recuperado el enlace**.
> **Confundirlos manda el cruce a ámbar por una lluvia cuando no había nada que decidir** (27/08 y 13/09).

**Qué separa hoy el firmware y qué no.** El **ancla** sí está separada, por tres caminos: `tUltimoComando`
(`Esclavo/src/main.cpp`) contesta *«¿el Maestro GOBIERNA?»* —sólo lo refrescan `CMD_GO_RED` y `CMD_GO_GREEN`—;
`reloj_notarRadio()` / `reloj_radioManda()` contesta *«¿LLEGA la radio?»* contando **cualquier** trama válida; y
`retryCount` contra `CICLO_MAX_REINTENTOS` **es un reloj propio del cambio**, vivo sólo en las esperas de ACK. **Lo
que NO está separado es el NÚMERO:** `SFTY6_SILENCIO_MS` es uno y gobierna las dos puntas **y las dos preguntas** —el
veto del cambio del Maestro lee el MISMO contador y la MISMA constante que el ámbar por orfandad, desplazada un
`TIMEOUT_ACK_MS`, y en el Esclavo un solo `tUltimoComando` decide **las dos cosas**—. 🔴 **Y la separación que sí
existe está ORDENADA AL REVÉS: el reloj del cambio vence ANTES que el de la quietud**, y no por accidente: `costura_09`
**exige** que el peor caso quepa bajo el techo de orfandad o los últimos reintentos serían código muerto (`N-71`).
**Esa desigualdad correcta es la que abre el HUECO 5.**

**Y sobre el MISMO techo se apoyan dos cuentas que NO se pueden comparar, porque miden contra bordes distintos**
(`CLAUDE.md` §7). **(A) El presupuesto asignable — `PRESUPUESTO_LIBRE_MS`:** lo que queda libre bajo el techo de
orfandad **crudo** tras el peor caso del ciclo —la cadencia del latido más `CICLO_MAX_REINTENTOS` pasos con su tiempo
de cable—, y lo recalcula `costura_09` leyendo las constantes del C++; *su borde* es que quien primero llega manda, y
es el ámbar por orfandad. **(B) La desigualdad del punto de suelta — el `static_assert` de `N-163`:** el mismo peor
caso **más un `TIMEOUT_ACK_MS`**; **NO es un presupuesto y su holgura NO es de nadie**, su borde es el instante en que
esta punta suelta el verde, y dice que adelantar la suelta no puede recortar el presupuesto de reintentos. **No se
restan ni comparten holgura**, y un tercer consumidor se mide contra **(A)**. **El aire** —`RF_BURST_COPIES` copias
por la tasa aérea— **no se lee de ninguna constante del firmware porque no existe como tal** (`ENVIO_TRAMA_MS`); los
parámetros del módulo son de **SPEC 6 §B**, y aquí sólo se exige que las dos puntas y el repetidor estén configurados
igual o el CRC no casa. **Subir `SFTY6_SILENCIO_MS` sin mover `CICLO_MAX_REINTENTOS` × `TIMEOUT_ACK_MS` ensancha el
HUECO 5** — el compromiso que `T-2` resolvió sobre la versión de campo: **SPEC 6 D.**

## 10. QUIÉN EJERCE CADA BARRERA DE ESTE DOCUMENTO

> **Una spec puede describir barreras que ningún compilador ejerce, con UNA condición: que cada barrera lleve escrito QUIÉN
> la ejerce.** El criterio es `CLAUDE.md` §6.3 — **¿algún arnés COMPILA ese `.cpp`?**; si sólo lo lee por texto no ve un
> defecto del TIEMPO. Filas = compuerta; reparto de `.cpp`, `ARQUITECTURA.map` §4-5. Medido sobre `ef3504c`.

| barrera | quién la EJERCE hoy |
|---|---|
| `SFTY-2` — sólo `semaforo.cpp` escribe pines | **fila 17**, y **sólo el Maestro** · el Esclavo, texto (SPEC 1 §13) |
| `SFTY-3` — CRC-8 de cada trama | 🔴 **NADIE.** `*/src/protocolo.cpp` de las **dos** puntas sólo lo cruza PlatformIO |
| `SFTY-6` — silencio → ámbar, y la suelta por margen | ✅ **fila 18**, bloques G: compila `coordinador.cpp` y `Esclavo/src/main.cpp` reales y relee `SFTY6_SILENCIO_MS` del C++ |
| `SFTY-7` — reintentos | ✅ **fila 18** (`CICLO_MAX_REINTENTOS` releído) · la desigualdad de §9, `costura_09` por texto |
| `SFTY-9` vuelta del enlace · `SFTY-13` supresión del latido | 🔴 **nadie**, ninguna de las dos |
| `SFTY-17` retardo de cortesía · §5.2 `rojoEsclavoConfirmado` · §5.4 `MAX_VERDE_BACKSTOP_MS` | ✅ **fila 18** las tres (`RETARDO_RESPUESTA_MS` entre ellas) |
| §5.3 `puedeSostenerVerde()` | ✅ **fila 18, POR SU EFECTO** (G3, G11). Es `static`: ningún arnés puede nombrarla, así que se mide el verde y no la función |
| §8 (e) `ciclo_degradado_fase()` · §7 la reanudación (`D-29`) | ✅ la fase, **fila 15** sola —es cabecera pura— y **19** dentro del modo; la reanudación, **18** (Esclavo) y **19** (Maestro) |
| §2.3 `D-8`, los TRES `if` · §6 `D-31`, el acuse | ✅ **fila 18**: compila `Esclavo/src/{main,bluetooth}.cpp` REALES; `D-31` es su bloque H |
| §8 (c) la puerta del Degradado del Maestro | ✅ **fila 19**: `adaptador_maestro_deg.cpp` llama a `coordinador_configConfirmada()` y `coordinador_desfaseValido()` sobre el `coordinador.cpp` real |
| §8 (d) el cruce `SYNC_FRESCA_MS` ↔ `LIMITE_SIN_SYNC_MS` | 🔴 **NADIE** (HUECO 7) |

**Cuenta: 16 barreras — 11 ejecutadas, 4 sin nadie, 1 partida** (`SFTY-2`, ejecutada en una punta y de texto en la otra).
Los rojos de `SFTY-3`, `SFTY-9` y `SFTY-13` **no son tres casillas pendientes**: apuntan a `protocolo.cpp` y al
`coordinador.cpp` fuera del ciclo — es el mismo hueco visto tres veces.

## HUECOS MEDIDOS

1. 🟢 **CERRADOS dos:** `AVISO_AMBAR_REINTENTOS` está **elegido, no derivado** (`D-32` (3)); y el veto
   `!mando_ambarLocal()` del `CMD_GO_GREEN` **sigue en pie** porque `D-32` (1) recorta `D-30` y el mando se queda —lo
   que **no** cierra que `J16` p5/p8 sigan vacíos y leídos por dos caminos (SPEC 5 §3)—.
2. 🔴 **Los dos presupuestos de radio no casan sobre el mismo techo: fila pendiente, no detalle** (`D-31` (3) y
   `coordinador.cpp`). §9 los separa por su borde; **elegir el modelo sigue sin hacerse**.
3. ⚠️ **El `CMD_ACK_RED` no lleva a qué `CMD_GO_RED` contesta** —escrito en el propio `coordinador.cpp`—: un acuse
   retenido en el aire y soltado tras una orden de rojo perdida **sería indistinguible del bueno**, y
   `rojoEsclavoConfirmado` acota el caso pero no lo cierra. Sin decisión. Y **la rama `CMD_PING` del Maestro no tiene
   emisor**: el Esclavo emite `CMD_PONG` y nunca `CMD_PING`, y el Repetidor no origina tramas — inofensivo para la luz.
4. ⚠️ **`D-21` pieza (A) sigue sin construir: el `OSF` no llega al STM32**, y el Degradado se autoriza sobre
   `reloj_horaFiable()`, que no ve ese bit (**SPEC 3, H-2**). Y **nada de este capítulo ha visto una tarjeta**: `D-31`,
   `D-26` y el `$EVENT` de `D-32` (2) están en `main` **SIN BANCO**, y qué corre en cada equipo lo dice `ESTADO.md`.
5. 🔴 **NADIE MIRA LA LUZ DE LA OTRA PUNTA ENTRE LOS DOS VENCIMIENTOS** (§9). Agotados los reintentos, el Maestro se va
   a ámbar con `FALLO_RF / REINTENTOS_AGOTADOS` mientras la otra punta —si el `GO_GREEN` llegó y su acuse no volvió—
   **sigue en verde** hasta que venza SU silencio; dura la diferencia entre los dos plazos, que se recalcula de las
   constantes. `N-163` cerró el sentido contrario y **éste no lo cierra nadie**: `costura_09` sólo comprueba que los
   reintentos quepan, y `puedeSostenerVerde()` sólo protege el verde de ESTA punta. **No ejercido en banco.**
6. 🔴 **LA REANUDACIÓN AUTOMÁTICA NO SE ANUNCIA** (§7). El equipo vuelve solo al único modo que da verde sin confirmar
   con el otro extremo y **el operario no puede enterarse desde el teléfono**: no hay `$EVENT` ni literal. No es defecto
   de `D-29` —que decidió reanudar— sino de lo que `D-29` no dijo. **Es decisión vial: del responsable.**
7. 🔴 **LAS DOS PUNTAS SE AUTORIZAN CON UMBRALES DE SYNC DISTINTOS Y NADIE LOS CRUZA** (§8 (d)). Medido sobre
   `ef3504c`. Ni `static_assert` ni pack, y las dos constantes viven en proyectos distintos. **Es el hueco más grande
   del acuerdo de §8**, porque lo que falla no es la radio: es la puerta.
