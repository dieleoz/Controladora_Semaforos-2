# SPEC 2 — LAS DOS PUNTAS Y LA RADIO

> **EL REQUISITO, Y ES EL ÚNICO QUE IMPORTA AQUÍ: LOS DOS POSTES NUNCA DAN VERDE A LA VEZ.** No hay cable entre
> ellos. Lo único que los une es una radio lenta que pierde tramas, y esta especificación es lo que el equipo hace
> **con esa radio sana, degradada y muerta**.

**Qué NO entra:** el ciclo de un poste solo (SPEC 1) · la hora (SPEC 3) · la app (SPEC 4) · el cobre y las radios como
aparato (SPEC 5) · el procedimiento de campo y el parche `T-2` (SPEC 6). **De dónde sale:** las decisiones vigentes del
responsable (`DECISIONES.md`) mandan en lo decidido y el fuente en lo que el equipo HACE; los manuales no son fuente.
**Ninguna cifra vive aquí**: se cita el SÍMBOLO. **Todo lo medido está sobre `ef3504c` con el árbol limpio** — el
firmware lo tocan otros agentes.

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
| `CMD_GO_RED` / `CMD_ACK_RED` | M→E / E→M | la orden que cierra —también es el latido cuando el rojo no consta— y su «estoy en rojo» |
| `CMD_PING` / `CMD_PONG` · `CMD_GO_AMBAR` | M→E / E→M | el latido, que mide que el enlace vive · y el ámbar ordenado, siempre detrás de una orden de rojo previa |
| `CMD_AMBAR_ESCLAVO` / `CMD_ACK_AVISO_AMBAR` | E→M / M→E | el Poste 2 avisa de su ámbar de emergencia, y el acuse (`D-31`) |
| `CMD_CANCELA_AMBAR_ESCLAVO` | E→M | el Poste 2 lo retira. **No se acusa, y es la regla** (§6) |
| `CMD_DEMANDA` / `CMD_ACK_DEMANDA` · `CMD_HORA_*` | E→M / M→E | la demanda de cámara y su respuesta, que dice si se atiende · y la hora, que **no toca luz** (SPEC 3) |
| `CMD_CONFIG_*`, `CMD_DELTA*` | M→E | 🔴 **NO son «servicio»: son el ACUERDO que autoriza el Degradado** (§8) |

Del Poste 2 salen por iniciativa propia **tres** —la demanda, el aviso de ámbar y su cancelación—; el resto, respuesta.

### 2.2 El paso de verde a verde, que es donde se mata a alguien

Un cambio de sentido pasa SIEMPRE por todo-rojo, contado **desde el acuse del otro lado, no desde la orden**
(`N-162`); el plazo es el despeje que el modo le configuró al coordinador.
`Verde M -> C_MASTER_A_ROJO -> C_ESPERA_ESTATICO_TRAS_MASTER (despeje) -> GO_GREEN -> C_ESPERANDO_ACK_GREEN ->
ACK_GREEN -> Verde E -> GO_RED -> C_ESPERANDO_ACK_RED -> ACK_RED -> C_ESPERA_ESTATICO_TRAS_ESCLAVO -> Verde M`
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

### 2.3 Lo que el Poste 2 hace al recibir

Las órdenes de rojo y de verde refrescan el reloj de orfandad; las tramas de servicio y el ámbar ordenado **no lo
refrescan**, a propósito: significan «hay portadora», no «el Maestro está gobernando el cruce». Vetado, **ni se
obedece ni se ACUSA**: acusar un rojo que no se ha encendido dejaría al Maestro dando verde convencido de que aquí hay
rojo — *se guarda lo que ABRE PASO, no lo que lo para*. La orden de verde es **idempotente**: repetida sobre una luz
ya en ámbar de transición o en verde, **re-acusa y no toca la luz**.

> 🔴 **LA DECISIÓN DICE «DOS VETOS» Y EN EL FUENTE HAY TRES `if`. LAS DOS CUENTAS SON CIERTAS Y CUENTAN COSAS
> DISTINTAS, y confundirlas es lo que hace que se lean como una sola.** **DOS por SUJETO**, que es como lo escribe la
> decisión (`D-8`): el veto del **mando** y el de la **app**; dos personas distintas pueden poner ese ámbar. **TRES
> por RAMA**, que es lo que hay que guardar: los dos sujetos aparecen **juntos, en la misma condición, en tres `if`
> del bucle principal del Esclavo** — la rama de la orden de rojo, la de la orden de verde, y una **tercera** que es
> la recuperación tras fallo. **Esa tercera NO es un `else` de la primera:** una orden de rojo vetada arriba entra por
> ella y volvería a forzar rojo por su cuenta. El fuente lo avisa —*«guardar solo una de las dos deja la revocación
> intacta»*— y `SPEC 5` §3 tiene la cuenta de tres. Una regla que ENUMERA sujetos comprueba que cada sujeto EXISTE, y
> dónde se ejerce (`CLAUDE.md` §2).
> `grep -c "mando_ambarLocal() && !bluetooth_ambarEmergencia" 01_Firmware/Esclavo/src/main.cpp`

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

> 🔴 **LOS DOS UMBRALES SON EL MISMO NÚMERO Y LOS DOS INSTANTES NO.** Entre «la orden llega al Esclavo» y «su respuesta
> llega al Maestro» hay el retardo de cortesía (`SFTY-17`) más el viaje de vuelta, y **ese desfase era la ventana**:
> con la dirección Maestro→Esclavo muerta, el Poste 2 se iba a su ámbar **antes** de que el Poste 1 apagara su verde
> — verde contra ámbar con la pluma del otro poste arriba.

**Se cierra soltando el verde propio un margen ANTES** (`N-163`): el veto de margen adelanta la decisión en lo que dura
un acuse, que es lo que ya acota el viaje de ida y vuelta. Al soltarlo la punta va a rojo directo, marca que soltó por
margen y **sale a reposo** —no a una espera de acuse— para no tocar la cadencia del latido: medido, la otra salida
producía **más** ámbares en microcortes que se recuperan. **El umbral NO se baja**, y es condición del responsable.
**Lo que ese margen NO cubre —el verde de la OTRA punta cuando ésta ya cayó— es §9.** **Vuelta del enlace**
(`SFTY-9`): se reencolan la hora y la configuración, se fuerza rojo, se pide el rojo del otro lado y **se espera su
acuse** antes de contar despeje; tras una suelta por margen se reanuda por esa misma puerta, porque un todo-rojo de
minutos en una vía alternada no es estado seguro: es donde la gente se pasa el rojo.

🟢 **Y EL SENTIDO CONTRARIO —el verde del ESCLAVO frente al ámbar del Maestro— SE CIERRA IGUAL** (`D-34`). Las dos
anclas se desfasaban por el otro lado: el Maestro cuenta desde lo último que OYÓ y el Esclavo refrescaba su silencio
con cada **repetición** de la orden de verde. Hoy, en las dos puntas:

- **Una repetición de la orden de verde no refresca el silencio del Esclavo**: sólo la orden que arranca la transición.
- **El Esclavo suelta su verde un acuse ANTES de su silencio**, va a rojo directo y lo dice en el parámetro de su
  respuesta al latido; **no lo borra el latido**, sólo una orden de luz.
- **El Maestro no entrega la orden de verde sin haber oído a la otra punta en el último latido**, así que el desfase
  entre anclas queda dentro de ese margen. El viaje de ida tiene que caber en la diferencia entre el plazo del acuse y
  la cadencia del latido: **premisa escrita junto a la constante, no medida con repetidor**.
- **Al oír ese aviso con el cruce quieto, el Maestro reanuda por la misma puerta de `N-163`**: rojo, acuse, despeje, y
  su propio verde.

**Lo que cuesta**: un microcorte que vuelve justo en ese margen da un rojo y un despeje de más, nunca un ámbar; y con
enlace malo la entrega del verde se retrasa un latido por cada respuesta perdida, sin pasar del umbral de silencio.
Lo ejercen las filas `G12`–`G14` del arnés de dos puntas, con controles negativos vistos fallar. **Sin banco.**

## 5. LAS BARRERAS QUE IMPIDEN VERDE + VERDE — la lista entera

1. **El Esclavo no enciende verde sin la orden de verde del Maestro** — fuera del Degradado no hay otra vía.
2. **El Maestro no enciende el suyo sin el acuse de rojo del otro lado** (la marca de rojo confirmado, §2.2).
3. **Ninguna punta abre NI SOSTIENE verde sin margen de silencio** — el veto de margen, en las cuatro puertas de
   apertura y en cada vuelta del coordinador. ⚠️ **Vigila el verde PROPIO, no el del otro** (§9).
4. **Backstop del Esclavo**: corta un verde eterno mirando **la luz**, no la orden.
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

> 🔴 **ENTRAR POR PRIMERA VEZ ES MANUAL. REANUDAR TRAS UN CORTE NO LO ES — Y NADIE SE LO AVISA AL OPERARIO.**
> - **La primera entrada sí la pide una persona:** Poste 2, la app (`D-18`) por su puerta única de entrada; Poste 1,
>   `SET_MODO:DEGRADADO` o el `A.B.A.B` del mando.
> - **La reanudación la decide la máquina**, en cada arranque y **sin pulsación ninguna** (`D-29`): cada punta tiene su
>   función de reanudar tras corte, llamada desde el arranque **y desde el bucle** —dos sitios, porque la hora llega
>   segundos después y la decisión queda diferida hasta que cierre su ventana—. El Esclavo entra por **la MISMA puerta
>   del operario**; el Maestro se pone en modo y cae en el reparto común. **No es un camino alternativo: es la entrada
>   de siempre con el permiso recuperado de la pila.**
> - **Lo que el operario NO ve:** un equipo al que se le fue la luz vuelve solo al único modo que da verde sin
>   confirmar con el otro extremo, y **no hay evento ni literal que lo anuncie** (HUECO 6).
> - **Lo que sí protege:** el permiso se tira si el equipo ya no está quieto donde lo dejó el arranque —una persona
>   eligió modo, o el modo ya gobierna—, si hay ámbar del mando puesto, o si falló cualquier condición de vigencia. Lo
>   que se difiere es **el borrado, no el límite duro**; la ventana, en SPEC 3 §6 y H-3.

**Condiciones del Poste 2, y devuelve MOTIVO, no un sí/no** —quien la llame tiene que poder decirle al operario qué le
falta—: sin hora **fiable** (fiable, no sólo puesta) · sin configuración recibida · ciclo nulo · nunca hubo
sincronización · sincronización vencida · ámbar vigente; **los textos que LEE el operario y los tres motivos que no
existen en el Maestro, SPEC 6 A.2.** **Dentro:** todo-rojo de entrada con su suelo mínimo y después verde **sólo** en
la fase propia, con el despeje que mandó el Maestro **ya ampliado en el origen** —si cada punta lo escalara los verdes
se solaparían durante minutos—; **un salto de hora se aplica PASANDO POR ROJO** (`D-26`). **Las cuatro salidas, y
ninguna admite marcha atrás** (el indicador de la pila se baja **al empezar** la salida): el operario · **el regreso
de la radio**, sólo con tramas de **gobierno** · **el límite duro** sin sincronización, con su aviso previo · y la
salida decidida por fila. **Las dos acaban en ámbar y sólo el Esclavo pasa por el despeje** (**SPEC 6 A.4**); del
estado rendido no se sale solo.

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

**(c) La puerta del Poste 1.** También devuelve **motivo, no sí/no**, y es la MISMA para la app y para el `A.B.A.B`:
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

**(e) El cálculo de fase — la misma función, no dos que «hacen lo mismo».** Vive en una cabecera compartida **que debe
ser idéntica** (lo compara `costura_01_contratos`): **posición = segundos del día módulo el ciclo, con ciclo = 2 ×
(verde + despeje)**, **anclada a la HORA DE PARED y no a un contador propio** —dos equipos encendidos con un minuto de
diferencia arrancarían desfasados un minuto entero—. **El orden lo fija la función y no se negocia: la posición 0 es
el verde del Maestro, o sea que EL POSTE 1 TIENE EL VERDE PRIMERO**, y detrás van despeje, verde del Esclavo y
despeje. **Guarda de medianoche:** el último tramo del día y el primero del siguiente son **siempre despeje**, porque
a las 00:00:00 la posición salta y ese salto puede caer en mitad de un verde y saltarse el todo-rojo. **Y la
configuración imposible tiene respuesta escrita:** con verde o despeje a cero devuelve despeje —todo-rojo—, no un
caso «que no debería pasar».

## 9. LOS DOS RELOJES Y LOS DOS PRESUPUESTOS — todos cierran sobre el umbral de silencio

> 🔴 **EL SILENCIO DE LA RADIO NO SIGNIFICA LO MISMO EN LOS DOS MOMENTOS DEL CICLO.** Al **pasar el testigo** hay que
> ser estricto y reintentar fuerte: ahí es donde alguien puede quedarse en verde mientras el otro también lo tiene.
> Con el cruce **quieto** —una en verde, la otra en rojo, nada que decidir— el mismo silencio **no abre ninguna vía de
> verde contra verde** y se tolera mucho más, con una condición: **no empezar un cambio sin haber recuperado el
> enlace**. **Confundirlos manda el cruce a ámbar por una lluvia cuando no había nada que decidir** (27/08 y 13/09).

**Qué separa hoy el firmware y qué no.** El **ancla** sí está separada, por tres caminos: el reloj de orfandad del
Esclavo contesta *«¿el Maestro GOBIERNA?»* —sólo lo refrescan las órdenes de rojo y de verde—; el aviso de radio al
reloj contesta *«¿LLEGA la radio?»* contando **cualquier** trama válida; y la cuenta de reintentos **es un reloj
propio del cambio**, vivo sólo en las esperas de acuse. **Lo que NO está separado es el NÚMERO:** el umbral de
silencio es uno y gobierna las dos puntas **y las dos preguntas** —el veto del cambio del Maestro lee el MISMO
contador y la MISMA constante que el ámbar por orfandad, desplazada lo que dura un acuse, y en el Esclavo un solo
reloj decide **las dos cosas**—. 🔴 **Y la separación que sí existe está ORDENADA AL REVÉS: el reloj del cambio vence
ANTES que el de la quietud**, y no por accidente: `costura_09` **exige** que el peor caso quepa bajo el techo de
orfandad o los últimos reintentos serían código muerto (`N-71`). **Esa desigualdad abre el HUECO 5.**

**Y sobre el MISMO techo se apoyan dos cuentas que NO se pueden comparar, porque miden contra bordes distintos**
(`CLAUDE.md` §7). **(A) El presupuesto asignable:** lo que queda libre bajo el techo de orfandad **crudo** tras el peor
caso del ciclo —la cadencia del latido más todos los reintentos con su tiempo de cable—, y lo recalcula `costura_09`
leyendo las constantes del C++; *su borde* es que quien primero llega manda, y es el ámbar por orfandad. **(B) La
desigualdad del punto de suelta:** el mismo peor caso **más lo que dura un acuse**; **NO es un presupuesto y su holgura
NO es de nadie**, su borde es el instante en que esta punta suelta el verde, y dice que adelantar la suelta no puede
recortar el presupuesto de reintentos. **No se restan ni comparten holgura**, y un tercer consumidor se mide contra
**(A)**. **El tiempo de aire** —las copias de la ráfaga por la tasa aérea— **está escrito A MANO en el coordinador
como tiempo de cable y NO se deriva de ningún parámetro de la radio** (`ENVIO_TRAMA_MS`); `costura_09` lleva el mismo
valor a la vista y por el mismo motivo. Los parámetros del módulo son de **SPEC 6 §B**, y aquí sólo se exige que las
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
| Silencio → ámbar, y la suelta por margen (§4) | ✅ **fila 18**, bloques G: compila el coordinador y el bucle del Esclavo reales y relee el umbral del C++ |
| Los reintentos (§3) | ✅ **fila 18** (su número, releído) · la desigualdad de §9, `costura_09` por texto |
| Vuelta del enlace · supresión del latido (§2.2) | 🔴 **nadie**, ninguna de las dos |
| Retardo de cortesía · §5.2 rojo confirmado · §5.4 backstop de verde | ✅ **fila 18** las tres |
| §5.3 el veto de margen | ✅ **fila 18, POR SU EFECTO** (G3, G11). Es privada al fichero: ningún arnés puede nombrarla, así que se mide el verde y no la función |
| §8 (e) el cálculo de fase · §7 la reanudación | ✅ la fase, **fila 15** sola —es cabecera pura— y **19** dentro del modo; la reanudación, **18** (Esclavo) y **19** (Maestro) |
| §2.3 los TRES `if` del veto · §6 el acuse del aviso | ✅ **fila 18**: compila el bucle y el Bluetooth del Esclavo REALES; el acuse es su bloque H |
| §8 (c) la puerta del Degradado del Maestro | ✅ **fila 19**: su adaptador llama a las dos consultas del coordinador real |
| §8 (d) el cruce de los dos umbrales de sincronización | 🔴 **NADIE lo cierra** — un pack lo mide y lo publica, pero no falla por él (HUECO 7) |

**Cuenta: 16 barreras — 11 ejecutadas, 4 sin nadie, 1 partida** (la barrera de salidas, ejecutada en una punta y de
texto en la otra). Los rojos del CRC, de la vuelta del enlace y de la supresión del latido **no son tres casillas
pendientes**: apuntan al fichero de protocolo y al coordinador fuera del ciclo — es el mismo hueco visto tres veces.

## HUECOS MEDIDOS

1. 🟢 **CERRADOS dos:** el número de reintentos del aviso de ámbar está **elegido, no derivado**; y el veto del mando
   sobre la orden de verde **sigue en pie** porque la retirada de la interfaz (`D-30`) se recortó y el mando se queda —lo que
   **no** cierra que `J16` p5/p8 sigan vacíos y leídos por dos caminos (SPEC 5 §3)—.
2. 🔴 **Los dos presupuestos de radio no casan sobre el mismo techo: fila pendiente, no detalle.** §9 los separa por su
   borde; **elegir el modelo sigue sin hacerse**.
3. ⚠️ **El acuse de rojo no lleva a qué orden contesta** —escrito en el propio coordinador—: un acuse retenido en el
   aire y soltado tras una orden de rojo perdida **sería indistinguible del bueno**, y la marca de rojo confirmado
   acota el caso pero no lo cierra. Sin decisión. Y **la rama del latido entrante del Maestro no tiene emisor**: el
   Esclavo sólo contesta latidos y nunca los origina, y el Repetidor no origina tramas — inofensivo para la luz.
4. ⚠️ **La pieza de la decisión del reloj que avisa de pila perdida sigue sin construir: ese bit del RTC no llega al
   STM32** (`D-21`), y el Degradado se autoriza sobre una comprobación que no lo ve (**SPEC 3, H-2**). Y **nada de este
   capítulo ha visto una tarjeta**: el acuse del aviso, el salto de hora por rojo y el evento periódico están en `main`
   **SIN BANCO**, y qué corre en cada equipo lo dice `ESTADO.md`.
5. 🟢 **CERRADO en el fuente (`D-34`, 15/09) — el número se queda.** ~~Nadie mira la luz de la otra punta entre los
   dos vencimientos~~ → medido en el arnés de dos puntas: hasta 23 s de ámbar del Maestro contra verde del Esclavo, y
   el mecanismo no eran los reintentos sino las anclas (`roadmap` 1.39). Lo que el equipo hace ahora está en **§4**;
   las filas `G12`–`G14` lo vigilan. **Sin banco: sigue sin haberse visto en una tarjeta.**
6. 🔴 **LA REANUDACIÓN AUTOMÁTICA NO SE ANUNCIA** (§7). El equipo vuelve solo al único modo que da verde sin confirmar
   con el otro extremo y **el operario no puede enterarse desde el teléfono**: no hay evento ni literal. No es defecto
   de la decisión que mandó reanudar sino de lo que esa decisión no dijo. **Es decisión vial: del responsable.**
7. 🔴 **LAS DOS PUNTAS SE AUTORIZAN CON UMBRALES DE SYNC DISTINTOS Y NADIE LOS CIERRA** (§8 (d)). Remedido el 14/09: no
   hay `static_assert`, las dos constantes viven en proyectos distintos, y el pack que sí las lee **reproduce la
   separación y la publica como residual: mide el hueco, no lo tapa. Es el hueco más grande del acuerdo de §8**, porque
   lo que falla no es la radio: es la puerta.
