# SPEC 8 — CAMARAS Y BARRERA: **que hace el equipo**

**Para quien pregunta que HACE el equipo — un solo lector, y no hace falta que sepa un milimetro de
cobre.** Que hacen las dos camaras, como frenan la barrera, cuando el equipo pide que se revise una
camara, los dos interruptores de administrador, y **que NO protege**. **Fuera:** que hay en cada pin,
la tabla de conectores, la cadena electrica y lo que no se ha medido del hardware — todo eso es
`SPEC_5`. Ademas: el ciclo (SPEC 1), la radio (SPEC 2), la hora (SPEC 3), la app (SPEC 4).

> 🔴 **DE DONDE SALE ESTE FICHERO, Y QUE QUEDO SIN DUENO AL PARTIRLO.** Nace el 14/09/2026 de
> `SPEC_5`, que llevaba **dos lectores dentro** (`roadmap.md` 1.45). El corte es **por lector**: alli
> lo que decide el **COBRE** —lo que hay en un borne y lo que pasa sin firmware dentro—, aqui lo que
> decide el **FIRMWARE**. Partir un documento deja huecos entre las dos mitades, asi que el que ya
> estaba identificado se nombra en vez de dejarlo caer:
>
> **EL FALLO SEGURO EN ESTADO DE AVERIA —que la barrera SUBA con el ambar intermitente— ES DE ESTE
> FICHERO, §5**, porque lo decide una linea de `escribirPines()` y no el cobre. `SPEC_5` §4 deja su
> puntero y se queda con la otra mitad, la que **no** es firmware: **sin energia el pin cae a `LOW`, que
> desde `D-168` es «sin orden de bajar»: la central deja el brazo ARRIBA (8.2).** Las dos mitades se leen juntas o
> ninguna protege a nadie.

**De donde sale cada linea.** Manda esta spec; `DECISIONES.md` es su indice para no volver a preguntar; el
cobre medido, `05_Funcional/17_...` y `SPEC_5`. El firmware se cita **por SIMBOLO**, nunca por linea,
y se remidio contra `{Maestro,Esclavo}/src/{semaforo,botones,modo_inteligente,main}.cpp` el
14/09/2026.

# 1. 🔴 LA CAMARA FRENA LA BARRERA — pero solo a la que YA veia bajar

**`D-33` (14/09/2026) derogo el aviso que habia aqui** *(«ninguna camara protege la pluma: puede bajar
con un coche debajo», medido el 11/09 y cierto hasta ese dia)*. Hoy `escribirPines()` **si** lee la
camara, y lo que hace con ella tiene una sola direccion:

- **La pluma SUBE por la luz y por nada mas.** La camara no la levanta nunca: la rama de «pluma ya
  abajo» la deja abajo **en seco**, sin mirar camara ni reloj. **Una deteccion no puede abrir una
  barrera con la luz en rojo.**
- **La pluma BAJA R segundos DESPUES del rojo:** de fabrica 3 (`PLUMA_RETARDO_BAJADA_MS`, elegido por el
  responsable el 14/09 — no derivado), *«porque suelen pasarse carros en ambar y hay que darle unos
  segundos al conductor»*; desde `N-187` R es un parametro por poste de 3..5 s (§8, sin banco). El retardo no
  depende de ninguna camara: se cumple con las borneras vacias, que es como esta hoy la mayoria de los equipos.
- **Durante el AMARILLO de cierre la pluma SIGUE ARRIBA, y el retardo cuenta desde el ROJO, no desde el amarillo**
  (`D-45`, `N-174`; construido en `cda33df`, sin banco; SPEC 1 §3.2). Es la letra de `D-33` («3 s despues del rojo»)
  y de `D-42` («retardo del rojo sigue en 3 s»): no las reabre. En `escribirPines()` de las dos puntas la condicion
  `luzPideArriba` es `(verde && !testLedsActivo) || estado == S_FALLO || estado == S_AMARILLO`: sin el ultimo
  termino el retardo empezaria en el amarillo y, con los dos a 3 s, la pluma bajaria en el instante del rojo, que es
  lo que `D-33` vino a quitar. El amarillo del test de lamparas no cuenta: alli `estado` sigue en `S_ROJO`. Lo
  ejercen las filas 17, 18 y 19 sobre la salida de motor y `maestro_09_test_leds` por texto. El `static_assert`
  del retardo (hoy `PLUMA_RETARDO_MAX_MS`, §8.3) contra `DESPEJE_SEG_MIN` no cambia, porque el
  amarillo no resta despeje: tras cada verde la pluma queda arriba `AMARILLO_SEG` mas el retardo, y el otro poste
  sigue abriendo un despeje entero despues del rojo.
- **Durante el ROJO+AMARILLO previo a cada verde la pluma no SUBE; sube con el VERDE real** (`D-53`, `N-176`;
  construido el 05/10, sin banco). `luzPideArriba` no gana termino: mira el verde ya enclavado, y en
  `S_ROJO_AMARILLO` no hay verde (SPEC 1 §3.3 (7)). **Excepcion: si llega arriba** (de `S_FALLO`, del retardo de
  bajada o retenida por el veto), es la bajada pendiente: baja como mucho R (`instalacion_retardoPlumaMs()`, §8.3)
  despues de que la luz dejo de pedirla arriba, salvo veto de camara activo. El rojo+amarillo va despues del despeje
  del otro poste y no lo acorta, asi que la bajada y su `static_assert` no cambian. Si el rojo+amarillo acaba en rojo
  (orden de rojo, perdida de radio), la pluma no ha subido.
- **LA PLUMA ES LENTA, y se queda asi (`D-54`, 06/10).** La de El Sisga tarda ~60 s en subir (Marco, sin medir):
  sube con el VERDE real y deja paso a mitad de un verde de 3 min como minimo (`D-5`); no se adelanta, porque la
  abriria en rojo durante el despeje del otro sentido. **Y la camara solo veta el COMIENZO de la bajada:** mandada
  bajar, `escribirPines()` toma la rama de «pluma ya abajo» y no mira camara durante el minuto que dura el recorrido.
- **Pasado el retardo, CUALQUIERA de las dos camaras del poste VETA la bajada** mientras siga viendo
  algo. Sin consenso: con un AND, una camara muerta anularia el veto para siempre.
- **Al soltarse el veto el firmware baja en el instante, pero la camara mantiene su alarma 5 s tras el evento**
  (de fabrica, `D-42`): la pluma baja unos 5 s despues de que el objeto salga. Esos 5 s son CONFIGURACION de la
  camara. Una camara que avisa DESPUES de los R s del rojo encuentra la pluma ya abajo y no la sube.
- **Que ve la camara es CONFIGURACION, no firmware (`D-39`):** las cuatro detectan Vehiculo **y** Humano, con
  sensibilidad 50 %, que se reajusta en campo con el enfoque. Precio aceptado: alguien parado en el barrido deja la
  pluma arriba, y se avisa (§3).
- **El veto es LOCAL y no para el ciclo.** *«Esas barreras son casi de adorno; el que manda es el
  semaforo y su estado»* (responsable, 14/09). El coordinador no lo consulta: el otro poste abre su
  verde igual.

🔴 **LA DIRECCION DEL FALLO ESTA DECIDIDA: ante error, falsa alarma o contacto pegado, LA BARRERA NO
BAJA.** No se le pone tope que acabe bajandola —un tope devuelve el peligro que el veto evita, porque
el firmware **no distingue un rele trabado de un vehiculo parado debajo** (`A-1.bis`)—: se **AVISA**,
y el aviso dice **cuantos segundos lleva retenida**, nunca «camara averiada». **La app lo traduce a lo
accionable** —mirar debajo del brazo y, si no hay nada, revisar el apunte y la configuracion de la camara— (§3). **Una barrera arriba no aplasta a nadie; el precio es que deja de proteger, y por
eso tiene que VERSE.**

⚠️ **Y lo que sigue sin proteger a nadie, escrito en vez de disimulado: una camara muerta DESDE LA
INSTALACION no veta NUNCA.** Sin un solo flanco el firmware no la distingue de una bornera vacia
(§4, `A-6`), asi que ahi la pluma baja como antes de `D-33`. Es el lado seguro para el tramo y el
inseguro para quien este debajo. Lo compensa el paso de instalacion del Manual 9, que obliga a
**provocar una deteccion** delante de la camara.

> **El orden de las tres razones dentro de `escribirPines()` NO es estetico, y por eso se escribe:**
> **1) LA LUZ** —si la luz pide la pluma arriba, arriba, y se olvidan las dos banderas: un veto que
> sobreviviera al verde siguiente seria un veto pegado—; **2) EL RETARDO** —mientras corre, la pluma
> se queda arriba PASE LO QUE PASE, porque es el unico tramo que no depende de que el aparato de
> fuera funcione—; **3) EL VETO**. Una comprobacion que solo mirara el RESULTADO aprobaria este
> firmware con las barreras en el orden equivocado (`CLAUDE.md` §9).

La grabacion en la microSD de cada camara es un respaldo que no toca el equipo (`D-104`): continua, unos 5 dias,
sobrescribiendo lo mas viejo, configurada en el menu de cada camara. El equipo no habla con las camaras por red: solo
lee su rele de presencia.

# 2. 🔴 Las DOS entradas de camara hacen EXACTAMENTE LO MISMO

**MEDIDO hoy en las dos puntas:** `camaras_actualizar()` recorre `CAM_J16[2] = {CAM_C_PIN,
CAM_D_PIN}` **en un solo bucle**: flanco -> `demanda_solicitar()` + `vigilante_flanco()`.
**NO hay una «de demanda» y otra «de presencia», lo diga lo que lo diga cualquier manual.**

**Y hay una TERCERA entrada declarada que no es ninguna de esas dos:** `CAM_DEMANDA_PIN` (`PB0`,
`J14`), que se lee **distinto** — Maestro por **NIVEL** en `modoInteligente_loop()`, Esclavo por
**FLANCO** en `main.cpp` -> `demanda_solicitar()` -> `CMD_DEMANDA` — y **el vigilante NO la vigila**.
Hoy `J14` va **libre** (`D-27`), asi que en reposo no pide nada; **pero el codigo sigue ahi.**

**Que borne es cada una: `SPEC_5` §2.1 y §3.** Aqui no se repite el cobre.

# 3. Cuando el equipo pide que se revise la camara — **basta UNA vez**

**No hay que contar alarmas ni fijar un numero: el criterio es fisico.** Un vehiculo que pasa por
debajo despeja en segundos. Si una camara sigue viendo algo **despues del todo-rojo mas largo que el
equipo admite**, lo que hay debajo ya no es trafico normal: o esta averiada la camara, o esta
apuntando a donde no debe, o hay algo parado ahi.

**Asi que en cuanto ocurre UNA sola vez, el equipo lo publica**, y lo repite mientras la barrera siga
retenida. El aviso dice **cuantos segundos lleva retenida** —que es lo unico que el equipo ha medido
de verdad—; **nunca dice «camara averiada»**, porque este equipo no ve imagen y no puede saberlo.

🟢 **Y la app lo traduce a lo unico accionable** (`aviso_camara_pluma.js`): el aviso de barrera
retenida **abre un cartel** que dice, por este orden, que la barrera esta arriba y no va a bajar
sola, que se mire debajo del brazo antes de tocar nada y, si no hay nada, que se revise el apunte y
la configuracion de la camara. Cuando el equipo informa de que la pluma ya bajo, el cartel **se
queda** y pide revisar la camara antes de irse. El contador de vetos normales **no** lo abre: un
vehiculo despejando es lo corriente.

⚠️ **Y el aviso tarda en salir lo que dura el todo-rojo mas largo que el equipo admite —90 s—, aunque
el cruce este configurado con uno de 10 s.** Se compara contra ese techo a proposito, para que este
aviso **no pueda ser nunca una falsa alarma por tener el ciclo largo**. Afinarlo exigiria que la
barrera preguntara el ciclo EN CURSO, y eso hoy no existe.

⚠️ **Lo que esto cuesta, y se acepta:** un camion cargando o un vehiculo averiado parado bajo la
barrera **tambien** dispara la peticion de ajuste. Es un aviso de mas con la camara sana. Molesta a
quien lo atiende; **no hiere a nadie**, y la alternativa —esperar a que se acumulen varias— seria
elegir un numero que nadie ha medido todavia. *(Decidido el 14/09.)*

> ⚠️ **El aviso de «camara pegada» NO es este y llega tarde para esto: cuelga del NIVEL sostenido, y
> una camara que dispara sin parar SUELTA la presencia entre disparo y disparo, asi que su cronometro
> se reinicia y nunca alarma** — mientras el veto sigue puesto. Las dos alarmas de camara, con su
> texto y lo que hace el tecnico con cada una, son **`SPEC_6`**.

# 4. 🔴 Una segunda camara muerta desde la instalacion NO SE DETECTA SOLA

**MEDIDO:** `vigilante_tick()` salta el plazo `CAM_CIEGA` mientras `camHuboFlanco[i]` es falso, y
`camara_estado()` salta esa misma camara al publicar `CAM:`. **El vigilante no alarma una camara que
NUNCA dio un flanco**, y la app pinta `CAM: OK` con la primera deteccion **de cualquiera de las
dos**. Tras cada reinicio la vigilancia queda desarmada hasta la primera deteccion. **Por que esa exencion
ya no tiene motivo: `SPEC_5` §7.2**, que es donde se sigue reportando, porque el choque nacio del cobre.

**El plazo de `CAM_CIEGA` es de 24 h de PASO ABIERTO sin flanco (`D-24`, `CAM_CIEGA_MS` en `botones.cpp` de las dos
puntas; construido, no ejercido en su tiempo real):** cuenta tiempo con la pluma arriba, no de reloj, y tiene que ser
mayor que `CAM_PEGADA_MS` para que un contacto trabado no se anuncie como camara ciega.

# 5. 🔴 LA PLUMA EN AVERIA: **SUBE con el ambar intermitente** — y esa eleccion NO TIENE FILA

**Este apartado es el hueco que dejo la particion, y por eso se nombra en vez de repartirlo:** que la
barrera suba en estado de averia es **conducta** —la escribe una linea de `escribirPines()`— y a la
vez el extremo de la cadena electrica que vive en `SPEC_5` §4. **Es de aqui.**

- **Sube con verde y tambien en `S_FALLO`** —ambar intermitente: orfandad SFTY-6, Modo Ambar,
  `AMBAR_EMERGENCIA`, Degradado en ambar, un poste recien encendido—; **no sube con el verde de un test
  de lamparas** (`!testLedsActivo`, N-82). **La tabla luz -> pluma entera es SPEC 1 §2** y no se repite.
  Es **SFTY-28**. Desde `D-45` (`cda33df`) sigue arriba tambien durante el amarillo de cierre (§1); el paso de
  verde a ambar intermitente no cambia: la pluma sigue arriba sin interrupcion.
  Y en cualquier luz con la barrera fuera de servicio (`FLAG_BARRERA_FUERA`, `D-96`, §6.3).
- **Por que ARRIBA y no ABAJO, escrito en vez de supuesto.** El ambar intermitente dice que el equipo
  se quedo sin enlace y ya no puede garantizar quien tiene el paso. Caben dos politicas y ninguna es
  obviamente correcta: con la pluma **ABAJO** se cierra la via por completo —y un corredor de obra sin
  salida es su propio peligro—; con la pluma **ARRIBA** se deja pasar a los dos lados con precaucion,
  que es lo que el ambar intermitente significa en la calle.
- 🔴 **Lo del `S_FALLO` lo eligieron el cliente y el PMT el 27/08/2026**, no el firmware — 🔴 **y esa
  eleccion NO tiene fila en `DECISIONES.md`** (SPEC 1 §11). **Vive en un comentario de
  `semaforo.cpp` de las dos puntas y en esta spec, y en ningun sitio mas.** Se escribe aqui porque es
  lo que un agente o un tecnico leeria mal manana: **no es una decision del firmware que se pueda
  cambiar midiendo**; para moverla hace falta una fila del responsable, y si se mueve se cambia
  **AQUI, en el comentario del fuente y en la tabla de SFTY-28**, los tres a la vez.
- 🔴 **Lo que NO garantiza este apartado: que el brazo se quede arriba si se va la luz.** Sin energia
  la salida cae a `LOW` = sin orden de bajar (`D-168`, 8.2); que el brazo suba y se quede lo hace la
  central, no el software, y se acepta con las pruebas 35-40 de la hoja (`SPEC_5` §4).

> 🔴 **Y lo que NO hace: no la levanta nada.** La camara solo puede retrasar o impedir la BAJADA
> (`D-33`, §1). **`A-1.bis` sigue abierto en su otra mitad:** que hacer cuando el veto se
> queda pegado. La respuesta de hoy es **avisar y no bajar nunca**, y esta escrita porque se eligio,
> no porque se midiera que sea la mejor.

# 6. 🟡 Los dos interruptores de administrador — **CONSTRUIDOS SIN BANCO el 09/11 (`N-182`)**

🔴 **La barrera fuera de servicio (6.3) la confirmo ARRIBA el responsable el 09/11 (`D-96`), con su modo de fallo.**

Hay dos averias distintas que hoy dejan un poste sin salida, y llevan **dos interruptores
separados en la app, tras la clave de tecnico (`D-56`, `D-145`), por poste** (el de las camaras, por camara
desde `D-166`: §9). No se pueden juntar en uno: lo que
falla no es lo mismo y lo que queda funcionando tampoco. **No son interruptores fisicos** (`D-94`): no hay
entrada libre decidida —`J14`/`PB0` reservada al fin de carrera (`D-46`), `J16` p5/p8 vacios y sin cablear
(`D-44`)— y un interruptor que el telefono no puede ver puesto es peor que ninguno. Lo que el responsable dejo
abierto el 14/09 lo cerro Fable por delegacion el 14/10 (`D-94`..`D-97`); construido el 09/11 (`N-182`), sin banco:
lo que pide tarjeta con barrera y pila esta en §7 (filas 4, 5 y 7).

| el interruptor | cuando se usa | que hace el equipo despues |
|---|---|---|
| **Sacar la camara del veto** | la camara esta averiada o mal apuntada y **no deja bajar la barrera** | la barrera **vuelve a trabajar**: sube con el verde y baja tras el rojo, con su retardo. Lo unico que se pierde es que la camara pueda retenerla. **Por camara desde `D-166`** (C, D o las dos; §9): la orden sin camara saca a las dos |
| **Sacar la barrera de servicio** | la barrera esta **rota de verdad** —brazo partido, actuador atascado— | el brazo se queda **arriba**, tambien en rojo (6.3, `D-96`), y deja de obedecer a la luz. El cruce sigue funcionando **solo con el semaforo** |

## 6.1 Las ordenes (`D-94`)

- Dos ordenes de la app, **locales al poste conectado**: `SET_VETO_CAMARA:ON|OFF` y `SET_BARRERA_SERVICIO:ON|OFF`.
  `ON` es lo normal (veto puesto, barrera en servicio) y lo que sale de fabrica; `OFF` es el interruptor puesto.
  **No viajan por radio:** el Poste 2 recibe la suya por su propio Bluetooth, como `SET_MODO`. El Maestro no la reenvia.
- Tras la clave de administracion de la app (`D-56`, que las suma a su lista) **y el PIN del equipo**, como toda orden.
  Viven en Ajustes, plegados en «Averias: camaras y barrera» (`D-141`, `D-145`), no en Trafico; antes de enviar `OFF`
  la app dice la consecuencia con
  las palabras de la tabla de arriba y, para la barrera, **«el brazo sube AHORA, tambien con la luz en rojo»**.
- **Molde `SET_TIEMPOS`** (`CLAUDE.md` §2): ya en ese estado, `RESULT:SIN_CAMBIO` sin tocar nada (`D-65`); otro texto,
  `$ERR ... DESC:FORMATO_INVALIDO`; con el respaldo perdido en este arranque, `$ERR ... DESC:RESPALDO_INVALIDO`
  (abajo); si lo releido despues no es lo pedido, `$ERR ... DESC:NO_QUEDO_PUESTO`, que la app traduce
  (`Interruptores.RECHAZOS`: «El equipo NO lo dejo puesto: ... repita la orden y, si vuelve a pasar, avise»; caso I7
  de `tests/dom_interruptores.js` con el literal de `interruptores_orden()`).
  No confundir con `NO_QUEDO_PUESTA` del reloj.
  Literales (`responder()`): `$ACK,CMD:<orden>:<valor>,RESULT:...` y `$ERR,CMD:<orden>:<valor>,DESC:...`; solo
  `FORMATO_INVALIDO` va sin valor: `$ERR,CMD:<orden>,DESC:FORMATO_INVALIDO`.
  Si no, se escribe la bandera, **la orden vuelve a pasar por la puerta** (6.3) y `$ACK,CMD:<orden>,RESULT:OK` solo si
  lo leido DESPUES es lo pedido (`D-149`): el getter de la bandera y, para la barrera, `semaforo_plumaFuera()`
  (6.4). Releer el registro solo no basta: un registro recien escrito devuelve siempre lo escrito.
- **Que es «respaldo invalido» (`D-148`):** `respaldo_valido()` no sirve: `respaldo_setup()` acaba siempre en
  `sellar()`, que pone `contenidoValido = true`, tambien tras `respaldo_borrar()`. El criterio es una bool en RAM que
  `respaldo_setup()` pone ANTES de llamar a `respaldo_borrar()`, leida con `respaldo_perdidoAlArrancar()`: «este
  arranque encontro el respaldo invalido y lo borro» (pila agotada en el ultimo corte, equipo nuevo, pila recien
  cambiada). Con ella, el `OFF` de las dos ordenes sale con ese `$ERR` hasta un arranque con respaldo valido; el `ON`
  se acepta, porque es el lado seguro. Un interruptor que no va a sobrevivir al corte no se pone.
  🔴 **Lo que NO detecta:** una pila que se agota con el equipo encendido. Mientras hay corriente el dominio de respaldo
  vive de `VDD`; el interruptor se pierde en el siguiente corte y vuelve a `ON` (6.2), y el arranque siguiente ya
  rechaza. Tras cambiar la pila, el primer arranque rechaza: se reinicia una vez.
- **Una sola fuente:** las ordenes y `escribirPines()` leen el MISMO getter (`respaldo_vetoCamaraOff(cam)`,
  `respaldo_barreraFuera()`), molde `respaldo_degAuto()`: pasan por `contenidoValido`, asi que antes de
  `respaldo_setup()` (en el Esclavo, `semaforo_setup()` va antes) y con `DR4` corrupto valen `ON`. Sin copia en RAM.
- El `$ACK` queda en el registro del puente (`D-73`): es la huella de quien puso el interruptor y cuando.

## 6.2 Sobreviven al corte: dos banderas en la casilla de indicadores (`D-95`)

Las diez casillas del respaldo estan ocupadas; los dos interruptores caben **como dos banderas libres de `DR4`**
(`REG_FLAGS`: usaba los bits 0..8, `FLAG_CICLO`..`FLAG_ROJO_SIN_HORA`): `FLAG_VETO_CAMARA_OFF` (`0x0200`, la C desde
`D-166`; la D y `MALA1`/`MALA2` en `0x0800`..`0x2000`, §9.4) y
`FLAG_BARRERA_FUERA` (`0x0400`), **dentro de la suma y con la misma `FIRMA`** (molde `FLAG_TESTIGO`, `D-61`): el
formato no cambia y un firmware anterior las ignora. **Las baja solo su orden inversa o `respaldo_borrar()`** (pila
agotada o contenido invalido al arrancar: los dos vuelven a `ON`, el lado seguro); `respaldo_guardarDegradado()` y
`respaldo_guardarTestigo()` no las tocan, porque un Degradado no repara una barrera. El fichero sigue identico en las
dos puntas (`costura_01_contratos` ya compara `respaldo.cpp/.h`). **HUECO en tarjeta, como `D-62`:** que sigan puestas
tras un corte real, con pila; el arnes de dos puntas solo mide que el reinicio simulado las conserve.

## 6.3 Lo que hace el equipo (`D-96`)

- **Camara fuera del veto:** `escribirPines()` no pregunta por ESA camara (§9.2): con las dos fuera `plumaVetada` no
  vale nunca `true` y la pluma baja al cumplirse R (§8.3), como con las borneras vacias. El retardo no
  cambia (`D-33`, `D-42`). El vigilante de las camaras sigue contando y avisando (`CAM_PEGADA`, `CAM_CIEGA`, `D-24`):
  asi se sabe cuando la camara volvio a dar flancos y se puede devolver el veto.
- 🔴 **Barrera fuera de servicio: ARRIBA, confirmado por el responsable el 09/11 (`D-96`).** `luzPideArriba` gana el
  termino del interruptor y el pin va a `ABRIR`, tambien con la luz en rojo. **Modo de fallo aceptado:** rojo aqui y
  verde en la otra punta con la pluma arriba = el carril unico con las dos bocas abiertas, sin tope de tiempo,
  sobreviviendo al corte (`D-95`) y sin `$ALARM` (solo el aviso de arranque, `D-152`); caso tipico, se repara el brazo
  y nadie quita el interruptor. No es la politica de `S_FALLO` (alli ninguna punta tiene verde). Deroga, SOLO para
  `FLAG_BARRERA_FUERA`, la premisa «pluma arriba sin verde fuera de `S_FALLO`» de `semaforo.cpp`, `botones.cpp` y
  `OPTIMIZACIONES.md` SFTY-28 («una barrera abierta con la luz en rojo ... es peor que no tener barrera»).
  **En el mismo cambio que la construye:** los comentarios de las dos puntas, la tabla de SFTY-28 y
  esta seccion; las pruebas que exigen lo contrario se reparten o invierten una a una (`CLAUDE.md` §9) con la excepcion
  **por el NOMBRE** `FLAG_BARRERA_FUERA`, nunca ensanchando a «con rojo»; control negativo: bandera forzada sin la orden
  o quitada con la pluma arriba en rojo, y `violacionesTalanquera > 0`. Cada una vista en rojo.
- **La orden vuelve a pasar por la puerta (`D-149`).** Con la luz en `S_ROJO`
  estable nada llama a `escribirPines()`: la unica reentrada del bucle es `if (plumaCierrePendiente)
  aplicarSalidas(ultR, ultA, ultV)`, y la rama `luzPideArriba` deja `plumaCierrePendiente = false`. Sin reentrada,
  `OFF` no mueve el pin hasta el siguiente cambio de luz (horas en rojo fijo, `D-38`) y `ON` deja la pluma arriba sin
  razon nombrada. Por eso, tras escribir la bandera, la orden llama a `semaforo_reaplicarSalidas()`, que llama
  a `aplicarSalidas(ultR, ultA, ultV)` —sin un segundo `digitalWrite` (SFTY-2)— y el `$ACK` lee despues. Con `ON` en
  rojo la pluma baja a R (§8.3), o la retiene el veto. Prueba: C++ real en las dos puntas, `S_ROJO`
  estable, vista en rojo sobre el fuente de hoy.
- Lo que §6 pedia medir: la segunda cosa, decidida ARRIBA (`D-96`, 09/11); la primera, el HUECO de 6.2.

## 6.4 Se publican sin crecer la trama (`D-97`)

El `$STATUS` no gana campo ni byte (le quedan 3 B, `D-60`): el estado viaja **como valor de dos campos que ya existen
y ya tienen su ancho** (cota por buffer, `N-154`), en las dos puntas:
- **`PLUMA:FUERA`** (5 <= 6 de `ARRIBA`; brazo arriba tambien en rojo, 6.3). **Una sola fuente
  (`D-150`):** `escribirPines()` anota en la misma asignacion que el pin si va por el interruptor (molde `N-153`), y el
  `$STATUS` y el `$ACK` leen esa anotacion con `semaforo_plumaFuera()`, nunca la bandera del respaldo: si no, el
  `$STATUS` diria `FUERA` con el pin en `CERRAR` (antes de la reentrada de 6.3, o entre `semaforo_setup()` y
  `respaldo_setup()`). El arnes compara `PLUMA:` con el pin real (`discrepanciasPluma`) tambien con la bandera puesta.
- **`CAM:NOVETO`** (6 <= 6 de `PEGADA`) **solo con alguna camara fuera Y `camara_estado()` en `OK` o `?` (`D-151`)**;
  si diria `PEGADA` o `CIEGA`, se publica eso aunque el veto este fuera. Desde `D-166`, `camara_estado()` es la peor
  de las camaras NO inhibidas (§9.5). Asi el campo sigue diciendo lo que 6.3 promete
  (cuando la camara vuelve a dar flancos): las alarmas de camara son de una vez (`$ALARM` al entrar, `$EVENT
  CAMARA_RECUPERADA` al salir) y el campo es lo unico que permanece. `NOVETO` se lee «camara sana y sin veto: devolver
  el veto». La prueba ejerce las dos lecturas con datos asimetricos (C en `PEGADA` y D en `OK`, y al reves).
- **Sin `$ALARM` repetida;** el `$STATUS` lo dice a 1 Hz desde que alguien se conecta y el `$ACK` esta en el registro.
  **Sin telefono (`D-152`):** si al arrancar sigue puesta alguna de las dos banderas, el parte de arranque lo dice con
  un `$EVENT` de una sola vez, que queda en el registro del puente (`D-73`); la prueba, en el arnes de dos puntas con
  reinicio simulado, vista en rojo. El `$EVENT` es `ORIGEN:INTERRUPTOR` con `DETALLE:ARRANQUE_CAM_NOVETO`,
  `ARRANQUE_BARRERA_FUERA` o `ARRANQUE_CAM_NOVETO_Y_BARRERA_FUERA` (`interruptores_avisoArranque()`, llamada en
  `main.cpp` tras leer el respaldo); la app lo pinta en ambar con `Interruptores.evento()`.

La app pinta los dos en ambar (`D-83`, mirar): la pluma, «BARRERA FUERA DE SERVICIO · arriba tambien en rojo, hasta
ponerla en servicio» (`PLUMA_LEYENDA`, `js/luces.js`); la camara, «SIN VETO» con «camara sana y sin veto: devolver el
veto» (`CAM_LEYENDA`). Con cualquiera de los dos, «Averias» de Ajustes se abre con el `<summary>` en ambar
(`js/pantalla_operario.js`). `app_04_valores_de_status` compara los literales del C++ con `PLUMA_LEYENDA` y
`CAM_LEYENDA` en los dos sentidos.

🔴 **Lo que NO garantiza «sacar la barrera de servicio»: que el brazo se quede arriba si se va la
luz.** Hoy, sin energia la salida cae y el brazo baja; el requisito del 10/11 (arriba, `SPEC_5` §4,
8.2) es de la central y esta sin construir. El interruptor manda mientras haya corriente.

**El obstaculo que la app midio** —el estado no cabia en la trama, `SPEC_4` §7 hueco 10— **queda cerrado por 6.4**.

# 7. `SIN VERIFICAR` — lo que este documento afirma **sin haberlo visto nunca**

**Escala:** `MEDIDO` = leido del fuente · `SIN VERIFICAR` = nadie lo ha comprobado, ni aqui ni en
ningun sitio. **Lo no verificado del HARDWARE va entero en `SPEC_5` §6** —y de sus veinte filas, las
que muerden a este documento son la **1** (ninguna camara se ha conectado NUNCA a este equipo), la
**2** (cual de los dos estados de la salida de la camara, NO/NC, significa demanda: el firmware admite los dos desde
`N-187`, §8; cual lleva cada camara instalada, **sin tomar**) y
la **4** (la concesion de paso por camara nunca se probo)—. Aqui, lo que falta de la CONDUCTA:

| # | lo que nadie ha comprobado |
|---|---|
| 1 | **NINGUNA DE ESTAS CONDUCTAS SE HA VISTO EN UNA TARJETA.** El retardo de 3 s y el veto son del 14/09, los parametros de §8 del 09/11, y estan **escritos y no cargados**; la instalacion **certificada** es la `V8.4`, anterior a todo ello. 🛑 **Que firmware hay hoy en cada equipo lo dice `ESTADO.md`, no este fichero** (`CLAUDE.md` §0.2) |
| 2 | **Que una camara real cierre el contacto el tiempo suficiente para vetar.** El veto se apoya en `camara_presenciaJ16()`, cuya ventana la fija `demanda_ventanaMs()`; **con un contacto mas corto que la ventana el veto se sostiene, con uno mas largo tambien — y ninguno se ha medido con una camara puesta** |
| 3 | **Que el aviso de pluma retenida llegue al telefono.** Sale por el mismo camino que el resto de `$EVENT` (`SPEC_6`), y **el Bluetooth no subio en toda la sesion de banco** (`SPEC_5` §6.17) |
| 4 | **Los dos interruptores de §6: construidos el 09/11 (`interruptores.cpp` en las dos puntas, `js/interruptores.js`) y sin banco.** Tras un corte con la barrera fuera, la pluma no baja: el arranque la deja arriba (`D-168`) y la reentrada del cierre la anota `FUERA` en el primer tick con la bandera leida: lo mide el arnes de dos puntas (S3) |
| 5 | **Que las dos banderas de §6.2 sigan puestas tras un corte REAL, con pila** — HUECO en tarjeta, como `D-62`; el arnes de dos puntas solo simula el reinicio |
| 6 | **La cinta de campo no ayuda:** 253 tramas del Sisga (10/09) y **las 253 dicen `CAM:?`** — en veinte minutos ninguna camara le dio un flanco al equipo (`SPEC_5` §6) |
| 7 | **El recorrido real de la pluma:** ~60 s de subida segun Marco (06/10), sin cronometrar; la bajada, sin dato |

# 8. 🟡 Parametros de instalacion de camaras y barrera — **CONSTRUIDO, SIN BANCO (`D-161`..`D-165`)**

**Lo que pidio el responsable el 09/11:** «que no sea firmware sino un parametro desde la app, en modo
administracion, para camaras y barrera; asi evitamos ir a campo por algo que hoy no controlamos». Este apartado lo
cumple el firmware desde `N-187` (09/11): modulos `instalacion.{h,cpp}` e `instalacion_flash.{h,cpp}`, identicos en las
dos puntas. **Sin banco; queda para la tarjeta** (lo que QA y arquitectura no pueden cerrar en PC): (1) la escritura
real de la pagina de `0x0800F400` (borrar, programar y releer; en PC la dobla `instalacion_modelo.inc`); (2) `NC` con
el rebote de un contacto real, contra los 5 ms de `camara_leerPin()`; (3) la radio durante el borrado: si borrar la
pagina hace perder una trama o un latido, sin medir.
**Regla de carga:** cada parametro sale de fabrica con el valor que hoy esta fijo en el firmware, asi que la primera
carga no cambia nada de lo que ve un conductor. Pero la carga por SWD borra entera la flash (`-e all`, `CLAUDE.md` §3):
**cada carga devuelve el poste a fabrica (NA, NA, 3 s)**. Un poste con una camara `NC` queda declarado `NA` y ve
presencia permanente (lado seguro: la pluma no baja en rojo y avisa). Tras cada carga se envia `CONSULTA_INSTALACION`
y se reenvia la configuracion que tenia el poste antes de cargar. (Se numera §8 porque §7 ya existe y
los numeros de apartado no se tocan, `CLAUDE.md` cabecera.)

## 8.1 Los parametros, y los dos candidatos que NO entran

| parametro | fabrica (= hoy) | rango | que cambia en el fuente |
|---|---|---|---|
| contacto de la camara C (`J16` p10) | `NA` | `NA` / `NC` | la lectura de `CAM_C_PIN`, en un solo sitio (8.3) |
| contacto de la camara D (`J16` p12) | `NA` | `NA` / `NC` | la de `CAM_D_PIN`, igual |
| retardo de bajada tras el rojo | 3 s | 3..5 s, entero (P1, P3 decididas 09/11) | `PLUMA_RETARDO_BAJADA_MS` pasa a valor de fabrica |
| sentido de `J15` | activa = BAJAR (`D-168`) | **fijo al compilar, no es parametro** | `pines.h` (8.2) |
| tipo de salida | nivel | **fijo, no es parametro** | nada (8.2) |
| pluma en ambar intermitente / `S_FALLO` | ARRIBA | **fijo salvo fila (P2)** | nada (8.2) |

## 8.2 Por que tres candidatos se quedan fijos (`D-164`) — la premisa contraria esta en el fuente

- **`J15` «activa = BAJAR»: SI, desde el 10/11** (responsable: «en reposo o sin energia, pluma arriba»; deroga el
  «NO» de `D-164`; requisito de cobre en `SPEC_5` §4). **Lo que HACE el equipo (`D-168`, construido sin banco):**
  (1) `TALANQUERA_CERRAR` = `HIGH`, `TALANQUERA_ABRIR` = `LOW`, el nivel de reset (`pines.h`, las dos puntas);
  (2) polaridad **fijada al compilar**: un binario por polaridad, declarada en la respuesta de version
  (`$ACK,CMD:VERSION,...,FW:<hash>,J15:BAJAR`; la vieja, `J15:SUBIR`; `$STATUS` no crece, `D-60`), y `ESTADO.md` la
  lleva por poste. De fabrica, la NUEVA; la vieja, solo para un equipo cuya central no cumpla, se compila con
  `TALANQUERA_POLARIDAD_VIEJA` (la orden, en `pines.h`). Un binario de antes del 10/11 contesta sin `J15:`, y la app
  lo dice como «activa = subir». **Nunca parametro:** `DR4` con pila agotada vuelve
  a fabrica en silencio y la pagina `0x0800F400` la borran `-e all` y el cambio de `VERSION`; cualquier fabrica
  invierte la pluma con uno de los dos tipos de central; (3) `semaforo_setup()` escribe `TALANQUERA_ABRIR` y deja
  `plumaAbierta = true` en las dos puntas: el arranque NO baja la pluma, y la primera bajada pasa por R y por el veto
  (8.4); `camaras_sembrar()` corre antes del primer tick que pueda bajar (Maestro: `botones_setup()` va antes que
  `coordinador_setup()`; Esclavo: R es >= 3 s y el bucle empieza tras los 2 s del arranque). Ejercido: I6 de dos
  puntas, el arranque del bloque A de «automatico» y `barrera_03_talanquera`.
  **Modo de fallo que trae:** cada reset, carga o cuelgue SUBE la pluma, tambien en rojo (aceptado: la barrera es
  complemento); `Q10` u opto en corto la deja ABAJO fija con verde, en silencio (`SPEC_5` §4, sin aceptar ni medir);
  y la mezcla de firmware y central la invierte sin que nadie salvo el que mira el brazo lo note.
  **Por eso firmware y central cambian en la MISMA visita, con `J15` suelta entre medias** (para este cambio NO vale
  «firmware primero» de `CLAUDE.md` §3): `J15` suelta; central a «sin senal = subir» y comprobar que sube; carga SWD;
  leer la polaridad por la app; reconectar `J15`; casillas: verde = arriba, rojo tras R sin nadie = abajo, quitar
  `J15` = sube, reset = sube y vuelve a bajar tras R. El puente H del banco se reconfigura en la misma sesion.
- **Salida por PULSO: NO en esta version.** `J15` es UNA salida con dos estados (1 = bajar y 0 = subir, `D-168`). Un
  pulso necesita un tercer estado (motor parado) o una central paso a paso, y una paso a paso que pierda un pulso
  queda **invertida para siempre** (arriba en rojo, abajo en verde) sin que el firmware lo sepa: no hay fin de carrera
  leido (`J14`/`PB0` reservada, `D-46`; libre, `D-27`). El puente H del funcional (10 s, con su final de carrera) es
  de NIVEL y ya funciona con el firmware de hoy. Pulso = version siguiente, con segunda salida (`J13`, `BUZZER`, sin
  uso en `src`; cobre sin medir) o con fin de carrera leido; los tiempos de subida y bajada (1..60 s) van con el.
- **Pluma en ambar intermitente como parametro por poste: NO sin fila.** §5: ARRIBA lo eligieron el cliente y el PMT
  el 27/08, y «para moverla hace falta una fila del responsable». Un parametro por poste la pasaria al tecnico. P2.
- **Y lo que tampoco se reabre:** `D-54` descarto el 06/10 un «anticipo variable desde la APK» de la SUBIDA; esto es
  la BAJADA tras el rojo y la subida sigue al verde real (`D-53`). `D-45`: en amarillo de cierre la pluma sigue arriba.

## 8.3 Lo que hace el equipo con cada parametro (`D-162`, `D-163`)

- **Contacto NC.** El cobre no cambia: `INPUT` pelado, pull-down R67/R68, contacto contra los 3,3 V de p9/p11
  (`pines.h`, «por que activo en alto»: eso es el NIVEL ELECTRICO y sigue cierto). Con `NC` el contacto esta cerrado
  en reposo (pin alto) y abre al detectar (pin bajo): **presencia = pin bajo**. La inversion va en UN sitio:
  `camara_leerPin(pin, nc)` (`botones.cpp`), que sigue siendo el unico `digitalRead` de camara DE `J16` (dentro de
  `botones.cpp` lo exige `camara_02_j16`); `camaras_sembrar()` y `camaras_actualizar()` la llaman con
  `instalacion_camaraNC(i)` en las dos puntas. `J14` no entra en el parametro: el Maestro lee `CAM_DEMANDA_PIN`
  (`modo_inteligente.cpp`) por `camara_leerPin()` con `nc = false`, y el Esclavo con su propio
  `digitalRead(CAM_DEMANDA_PIN) == HIGH` en `main.cpp`, anterior a `N-187`; las dos, activo en alto como hoy. El
  antirrebote de 5 ms confirma el nivel ACTIVO (con `NC` un rebote bajo no pasa sin filtrar). `camara_02_j16` sigue
  viendo `== HIGH` y no ve el `!= nc`: la inversion la prueba el bloque K del arnes
  automatico, no el pack. Asi veto, demanda, Modo Inteligente y vigilante ven la misma presencia logica.
- **NC aporta algo:** un cable cortado da presencia permanente, o sea el lado de §1 (la pluma no baja y se avisa).
- **Retardo.** `escribirPines()` compara `millis() - tPlumaCierrePedido` contra el getter del parametro, leido en cada
  tick (`instalacion_retardoPlumaMs()`, sin copia en RAM, molde `D-149`). Una sola cota en `instalacion.h`
  (`PLUMA_RETARDO_MIN_MS` 3000, `PLUMA_RETARDO_MAX_MS` 5000; `PLUMA_RETARDO_BAJADA_MS` 3000 de fabrica) para el getter,
  la orden y el `static_assert`: `PLUMA_RETARDO_MAX_MS * 2 <=
  DESPEJE_SEG_MIN * 1000` (Esclavo: `DESPEJE_MIN_HEREDADO_SEG`), o sea 5 s con 10 s; la orden rechaza lo de fuera.
  Cambiado durante una bajada en curso, vale desde ese tick: nunca baja de 3 s ni pasa de 5.
- **Leer la pagina mal = fabrica ENTERA.** Pagina borrada, firma, version o suma que no casan, un retardo fuera de
  3..5 o un contacto distinto de 0/1: los getters devuelven NA, NA y 3 s, que es el equipo de hoy.

## 8.4 La tabla de estados — luz x parametro -> `J15` y pluma (validada contra `escribirPines()` de `63e4c63`)

`J15`: 1 = `TALANQUERA_ABRIR`, 0 = `CERRAR` (simbolos: hoy ABRIR = `HIGH`; con 8.2, ABRIR = `LOW`). R = retardo
(3..5 s). «Ve» = presencia logica de C o D tras aplicar `NA`/`NC`. Ningun parametro entra en `luzPideArriba`: solo en
la rama de bajada.

| luz / causa | NA o NC | R | `J15` | pluma | caso ya aceptado |
|---|---|---|---|---|---|
| VERDE real | no cuenta | no cuenta | 1 | arriba | SFTY-28 |
| AMARILLO de cierre (`S_AMARILLO`) | no cuenta | no cuenta | 1 | arriba | `D-45` |
| ROJO, primeros R s | no cuenta | 3..5 | 1 | arriba | `D-33` (hoy 3 s) |
| ROJO tras R, ve | presencia logica | - | 1 | arriba, avisa a 90 s | `D-33`, §3 |
| ROJO tras R, no ve | presencia logica | - | 0 | baja | `D-33` |
| ROJO con pluma ya abajo | no cuenta | no cuenta | 0 | abajo | §1 («en seco») |
| ROJO+AMARILLO (`S_ROJO_AMARILLO`) | como ROJO | como ROJO | 0 | no sube | `D-53` |
| test de lamparas (verde, `S_ROJO`) | como ROJO | como ROJO | 0 | no sube | N-82 |
| `S_FALLO`, `S_AMBAR_INT` | no cuenta | no cuenta | 1 | arriba | §5, cliente y PMT 27/08 (P2) |
| `S_ROJO_INT` (noche) | no cuenta | no cuenta | 1 | arriba | `D-110` |
| barrera fuera (`FLAG_BARRERA_FUERA`) | no cuenta | no cuenta | 1 | arriba | `D-96` |
| camara fuera del veto (las dos) | ignorada | 3..5 | 0 tras R | baja | `D-96`; una sola o camara mala: §9.3 |
| camara mala (`CAM_PEGADA`/`CAM_CIEGA`), `MALAx=ARRIBA` (fabrica) | ignorada | no cuenta | 1 | arriba, `$ALARM` | `D-169`, §9.3 E2-E4 |
| arranque, reset, corte, SWD | no cuenta | no cuenta | 1 (`LOW`) | arriba con la central viva; corte general, lo que haga la central (`SPEC_5` §4, pruebas 36-37); 1.a bajada tras R y veto | `D-168` |

**Las cuatro comprobaciones, con lo que las garantiza:**
1. **Pluma arriba en rojo solo en los casos ya aceptados.** Los parametros no tocan `luzPideArriba` ni la rama «ya
   abajo»; cambian R (acotado a 5 s por la orden y el `static_assert` de la cota), que pin es presencia y, con
   `D-169`, la camara mala con `MALAx=ARRIBA` (`plumaRetenidaMala`, solo en la rama de bajada: no sube una pluma abajo).
   🔴 **Lo nuevo:** un contacto mal declarado entra por el caso de veto (`D-33`, «ante error no baja y avisa»): ver 8.5.
2. **Nunca baja sobre el amarillo ni dentro de R.** `S_AMARILLO` esta en `luzPideArriba`; R cuenta desde el rojo y su
   minimo es 3 s (P3).
3. **Ningun pulso activo mas de su tiempo:** no hay pulso (8.2). En nivel, el tiempo de motor lo corta el final de
   carrera de la central o del puente H.
4. **El reposo sin energia no depende de ningun parametro:** la polaridad de `J15` es de compilacion (8.2). El
   reposo es arriba (`D-168`), con el arranque escribiendo ABRIR y `plumaAbierta = true`.

## 8.5 El contacto mal declarado — se detecta

Declarar `NC` una camara `NA`, al reves, o `NC` con la bornera vacia da **presencia logica permanente**. Lo que pasa:
la pluma no baja en el rojo (lado `D-33`), el Modo Inteligente lleva el verde a su techo y no hay flancos. Tres
detectores, del mas rapido al mas lento:
1. **El `$ACK` publica la presencia logica en el momento de la orden** (`VE:C0D0`, 8.6). La app dice «con nadie
   delante debe salir 0»; si sale 1, el contacto esta mal declarado o hay algo delante.
2. **El aviso de pluma retenida** de §3 a los 90 s (`VETO_SOSTENIDO_MS`), en el primer rojo.
3. **`CAM_PEGADA`** a los 20 min (`CAM_PEGADA_MS`): su cronometro se siembra en `camaras_sembrar()` con el nivel ya
   invertido, asi que un nivel permanente desde el cambio tambien alarma. Por eso, **cuando el contacto releido de la
   flash cambia**, la orden re-siembra las dos camaras: `camaras_sembrar()`, publica, reinicia TODO su estado (flanco,
   ultimo flanco, silencio, estado y cronometros), sin flanco fantasma ni demanda regalada, y el `VE:` del `$ACK` se
   lee despues. Una orden que no cambia el contacto (`SET_PLUMA_RETARDO`) no re-siembra: no borra una `CAM_PEGADA`
   anunciada ni reinicia el aviso de 90 s.
**Y el cambio no suelta un veto en curso:** con `semaforo_plumaVetada()` cierto, `SET_CAMARA` contesta
`$ERR ... DESC:PLUMA_VETADA`. Para corregir una camara que retiene la pluma: `SET_VETO_CAMARA:OFF` (§6, baja tras R),
cambiar el contacto, comprobar `VE:0` y devolver el veto.

## 8.6 Donde se guarda, la orden y lo que publica

- **Pagina de flash propia, molde `testigo_flash.cpp`/`noche_flash.cpp`:** las diez casillas estan ocupadas (§6.2;
  `DR4` tenia 5 bits libres, no estaba lleno: errata de esta frase, corregida con `D-166`). Pagina de 1 KB en
  `0x0800F400` en las dos puntas (fichero identico). `board_upload.maximum_size` = 62464 en los dos `platformio.ini`
  deja las TRES ultimas paginas fuera del enlazador: `0x0800F400` instalacion, `0x0800F800` noche (`noche_flash`,
  solo el Maestro; en el Esclavo queda reservada sin uso) y `0x0800FC00` testigo (`testigo_flash`); las tres las borra
  `-e all` en cada carga. Formato: firma,
  version, `ncC`, `ncD`, retardo en s, suma de 32 bits; valor fuera de rango = fabrica. Se borra y escribe solo al
  aceptar una orden (pocas en la vida del equipo).
- **Ordenes, locales al poste** (no viajan por radio, como §6.1), tras la clave de administracion (`D-56`) y el PIN:
  `SET_CAMARA:C:NA|NC`, `SET_CAMARA:D:NA|NC` (sin coma: la coma parte el campo `CMD` del `$ACK`),
  `SET_PLUMA_RETARDO:<s>` (una o dos cifras); y `CONSULTA_INSTALACION` sin PIN (en la lista sin PIN del firmware, de
  `app.js` y del simulador). Las atiende `instalacion_orden()`, llamada desde `bluetooth.cpp`.
- **Molde `SET_TIEMPOS` (`CLAUDE.md` §2):** igual que lo guardado, `RESULT:SIN_CAMBIO`; texto que no casa,
  `DESC:FORMATO_INVALIDO`; retardo fuera de 3..5, `DESC:FUERA_DE_RANGO`; veto en curso, `DESC:PLUMA_VETADA`; se escribe
  la pagina, se re-siembra y se reentra por `semaforo_reaplicarSalidas()` (`D-149`), y `RESULT:OK` solo si lo releido
  de la FLASH despues es lo pedido; si no, `DESC:NO_QUEDO_PUESTO`. Cada `$ACK` lleva `VE:C<0|1>D<0|1>`; el `$ERR` no.
  El `CMD` repite la orden con su valor (molde `interruptores.cpp`), salvo `FORMATO_INVALIDO`, que va sin valor:
  `$ACK,CMD:SET_CAMARA:C:NC,RESULT:OK,VE:C0D0`, `$ERR,CMD:SET_PLUMA_RETARDO:6,DESC:FUERA_DE_RANGO`. `PLUMA_VETADA`
  solo rechaza `SET_CAMARA`; el retardo se acepta con veto y rige desde ese tick. Cota del buffer: la trama mas larga
  es la de `CONSULTA_INSTALACION`, 57 B en 64; si no cabe, `$ERR,CMD:INSTALACION,DESC:NO_CABE` (`N-154`).
- **Lo que publica:** el `$STATUS` no crece (3 B, `D-60`). `CONSULTA_INSTALACION` contesta
  `RESULT:C_NA_D_NA_R3,VE:C0D0`; la app la pide al abrir Ajustes > «Instalacion: camaras y barrera» (plegado, como
  «Averias»). El `$ACK` queda en el registro del puente (`D-73`).
- **Simulador de practica de la app:** contesta las tres ordenes y la consulta con los mismos literales, sobre un
  estado por poste (fabrica NA, NA, 3); no simula camara (`VE:C0D0` siempre) ni el retardo, igual que hoy `plumaCam()`.
  `js/simulador.js` esta en 500 lineas: va en `js/sim_instalacion.js`; la pantalla, en `js/instalacion.js`, con el
  rango 3..5 en un solo sitio (`Instalacion.RETARDO`), cruzado contra `instalacion.h` por `tests/dom_simulador.js`.
  Con un firmware anterior la consulta vuelve como `$ERR,CMD:AUTH_FAILED` (sin repetir el `CMD`): la app lo toma por
  la consulta, y dice que ese poste no tiene la funcion, solo si fue lo ultimo que salio del panel y dentro de
  `Instalacion.PLAZO_MS` (3 s, elegido sin medir; `rechazo()` de `js/instalacion.js`); un `DESCONOCIDO` no se le
  atribuye.

## 8.7 Lo que decide el responsable (P1..P4), con el modo de fallo y la recomendacion

- **P1. ¿Cuando baja?** Solo tras el ROJO con R ajustable; o en amarillo, «4 s antes» del rojo, o en ambar.
  Modo de fallo de bajar en amarillo o antes: cae sobre quien entro con verde y sigue bajo el barrido, que es lo que
  `D-33` vino a quitar, y contradice `D-45`. **Decidido el 09/11: solo tras el rojo, 3..5 s; nunca en amarillo
  ni en ambar.**
- **P2. Pluma en ambar intermitente / `S_FALLO`:** ARRIBA fija, o parametro ARRIBA/ABAJO. Modo de fallo de ABAJO:
  cierra el corredor sin enlace (§5); por poste, una boca abierta y otra cerrada con el mismo ambar.
  **Decidido: ARRIBA fija (cliente y PMT, 27/08); la fila que §5 pide es `D-164`.**
- **P3. Rango de R:** por debajo de 2 s se acerca a «baja en el instante del rojo»; por encima de 5 s pasa de la mitad
  del despeje minimo (`static_assert` de `D-33`). **Decidido el 09/11: 3..5 s (`D-163`)**: con 2 s el brazo cae ~7
  veces mas sobre quien cruza al final del amarillo (simulacion de dos modelos y arbitro, `roadmap.md` `N-187`).
- **P4. `J15` de nivel; pulso a la version siguiente (`D-164`, vigente). Sentido: decidido el 09/11 «activa =
  subir» y central «sin senal = bajar»; DEROGADO el 10/11 por el responsable: J15 activa = BAJAR, polaridad fijada al
  compilar (un binario por polaridad, declarada en la version), arranque que no baja la pluma, central «sin senal =
  subir» y firmware y central en la MISMA visita con `J15` suelta (8.2, `SPEC_5` §4).** Modo de fallo: sube con cada
  corte o reinicio (aceptado) y baja fija con `Q10` en corto (sin aceptar). **Construido sin banco (`D-168`).**

**P1..P4 decididas el 09/11** (simulacion de dos modelos y arbitro): P1 y P4 como se recomienda, P2 ARRIBA fija,
P3 3..5 s. Revision de arquitectura antes de construir: APTO CON CONDICIONES (C1..C5, cerradas en este apartado).
🔴 **Sin banco.**

# 9. 🟡 Camaras degradadas: veto POR CAMARA y camara mala — **CONSTRUIDO, SIN BANCO (`D-166`, `D-169`)**

**Principio del responsable (10/11, `D-169`): ninguna averia de camara o barrera deja el paso semaforico
inoperante; la barrera es complemento.** Con una camara mala el equipo alarma y sigue funcionando. Lo que sigue esta
leido del fuente de las dos puntas (identico: `botones_vigilante.inc`, `interruptores.cpp`, `respaldo.cpp`).

## 9.1 Definiciones

- **Inhibida a mano** (`D-166`): bit por camara en `DR4`, puesto por `SET_VETO_CAMARA:C|D:OFF`. El firmware la ignora
  del todo: **ni veto ni demanda**. La ve igual el vigilante (`CAM_PEGADA`/`CAM_CIEGA`) y el `VE:` de `SET_CAMARA`:
  la mascara va en tres sitios por punta y solo ahi —el veto de `escribirPines()`, `demandaLocalS1`
  (`modo_inteligente.cpp`, Maestro) y el flanco que llama a `demanda_solicitar()` (`camaras_actualizar()`; en el
  Esclavo es el `CMD_DEMANDA`)—, nunca en `camara_presencia()` ni en `camara_presenciaJ16()`.
- **Mala detectada:** `camEstado` en `CAM_PEGADA` o `CAM_CIEGA` (§3). La pone y la quita el vigilante solo
  (`$ALARM` al entrar, `$EVENT ..._RECUPERADA` al salir). **Vive en RAM: un corte la olvida** y la `CIEGA` tarda otras
  24 h de pluma arriba en volver a detectarse (Q5, aceptado); lo que dura es la inhibicion, que si se guarda.
- **Mira** = ni inhibida ni mala (`camara_mira(i)`, `botones.h`). Una `?` (sin flanco nunca) mira: no se puede saber.
- **Veto = OR de las que miran.** Una mala detectada deja de vetar y de pedir verde (Q4).

## 9.2 El parametro `MALA1` / `MALA2` (`D-169`)

`MALA1` rige si hay al menos una mala y queda una que mira; `MALA2`, si hay al menos una mala y no queda ninguna
(dos malas, o una inhibida y otra mala: Q3). Valores `BAJA` (la mala se ignora; baja tras R si las que miran no ven)
o `ARRIBA` («no hace nada»: la pluma se queda arriba en el rojo sin tope). **Fabrica: `ARRIBA` y `ARRIBA`.**
`ARRIBA` entra en la RAMA DE BAJADA de `escribirPines()`, no en `luzPideArriba` (Q2): sube con el verde siguiente y no
levanta una pluma ya abajo; se anota en `semaforo_plumaRetenidaMala()`, no en `plumaVetada` (no cuenta como veto, no
lanza el aviso de 90 s ni el `PLUMA_VETADA` de `SET_CAMARA`). Dos inhibidas sin mala: baja tras R (`D-96`), sin
leer `MALA2`. El Modo Inteligente no cambia de modo sin camaras (Q7, `CLAUDE.md` §2): sin demanda cae al suelo, que son
los tiempos de Automatico.

## 9.3 La tabla de estados — ROJO tras R, pluma arriba al entrar en rojo (el resto de luces, como §8.4)

| estado | pluma | alarma / `CAM:` | demanda |
|---|---|---|---|
| E1 dos miran | baja si ninguna ve; retenida si alguna ve (`D-33`) | `CAM:` la peor | las dos |
| E2 una mala, otra mira | `MALA1=BAJA`: la buena retiene si ve, si no baja. `MALA1=ARRIBA`: arriba | `$ALARM` con `ACCION:IGNORA_CAMARA` o `PLUMA_ARRIBA` | solo la buena |
| E3 una inhibida, otra mira | la buena retiene si ve, si no baja | aviso de arranque; `CAM:NOVETO` si la otra esta sana | solo la buena |
| E3b una inhibida, otra mala | `MALA2` | la `$ALARM` de la mala | ninguna camara |
| E4 dos malas | `MALA2` | dos `$ALARM` | ninguna camara |
| E5 dos inhibidas | baja tras R (`D-96`) | aviso de arranque; `CAM:NOVETO` | ninguna camara |

Una camara inhibida que se declara mala alarma con `ACCION:NINGUNA`: ya estaba ignorada. La `ACCION:` se fija al
alarmar con el parametro de ese instante (`camara_alarmar()`).

## 9.4 Donde se guarda

`DR4`, molde §6.2 (misma `FIRMA`, dentro de la suma, solo los baja su orden o `respaldo_borrar()`): `0x0200` C inhibida
(el bit de `D-94`), `0x0800` D inhibida, `0x1000` `MALA1=BAJA`, `0x2000` `MALA2=BAJA`. **Bit a 0 = fabrica** (veto
puesto, `ARRIBA`): con pila agotada todo vuelve a fabrica. Quedan libres `0x4000` y `0x8000`. Un equipo de campo con
`0x0200` puesto («las dos fuera» con el firmware anterior) lo lee como «C fuera, D con veto»: mas veto, el lado
seguro; el aviso de arranque lo dice por camara.

## 9.5 Ordenes y publicacion (molde `SET_TIEMPOS`, `interruptores.cpp`)

- `SET_VETO_CAMARA:ON|OFF` (las dos, como antes), `SET_VETO_CAMARA:C:ON|OFF`, `SET_VETO_CAMARA:D:ON|OFF`;
  `SET_CAMARA_MALA:1|2:BAJA|ARRIBA` (sin coma: parte el campo `CMD`). Locales, tras clave de administracion y PIN.
  `SIN_CAMBIO` si ya esta asi; `FORMATO_INVALIDO` sin valor en el `CMD`; `RESPALDO_INVALIDO` para el valor que no es de
  fabrica (`OFF`, `BAJA`) con el respaldo perdido al arrancar (`D-148`); reentrada por `semaforo_reaplicarSalidas()` y
  `RESULT:OK` solo si lo releido es lo pedido; si no, `NO_QUEDO_PUESTO` (`D-149`). La mas larga sigue siendo la de la
  barrera, 56 B en `respuesta[64]`.
- `CONSULTA_VETO`, sin PIN: `$ACK,CMD:CONSULTA_VETO,RESULT:C1O_D0P_M1A_M2A` (45 B): por camara, `1` con veto y `0`
  inhibida, y su estado `O` (OK), `?`, `C` (CIEGA) o `P` (PEGADA); `M1`/`M2`, `A` (ARRIBA) o `B` (BAJA).
- `$STATUS` sin crecer (`D-60`): `CAM:` es la peor de las camaras NO inhibidas (Q6); `NOVETO` si hay alguna inhibida y
  las demas estan `OK` o `?`, y con las dos inhibidas.
- Aviso de arranque (`D-152`): `ARRANQUE_CAM_NOVETO` (las dos), `ARRANQUE_CAM_NOVETO_C` o `ARRANQUE_CAM_NOVETO_D`, con
  `_Y_BARRERA_FUERA` detras si toca; todos llevan el literal `CAM_NOVETO` que busca la app. Un parametro `MALAx=BAJA`
  no es un interruptor puesto: no avisa.
- **App:** Ajustes > «Averias: camaras y barrera» con un interruptor por camara (C, D) ademas del de las dos, y los
  dos selectores `MALA1`/`MALA2`; cada acuse dice que queda y «Que falta». El simulador de Practica contesta las
  mismas ordenes con los mismos literales (`js/sim_interruptores.js`).

## 9.6 Lo que no se ha visto

🔴 **Sin banco ni tarjeta.** El arnes del automatico (bloque I) y el de dos puntas (bloque I, Esclavo real) ejercen con
datos asimetricos —C inhibida y D ve, y al reves— el veto y la demanda, y `MALA1`/`MALA2` con una `CAM_PEGADA`; la
`CIEGA` real (24 h) no se ha ejercido nunca (`D-24`). Que los bits sigan tras un corte con pila: HUECO de §6.2.

*Decisiones recogidas: `D-12`, `D-13` (lo no derogado), `D-24`, `D-25`, `D-27`, `D-33`, `D-39`, `D-42`, `D-54`,
`D-94`..`D-97`, `D-148`..`D-152` (§6, construido sin banco), `D-161`..`D-165` (§8, construido sin banco) y
`D-166`, `D-169` (§9, construido sin banco). Abiertas
que nombra sin resolver: `A-1.bis`, `A-6`. La pluma ARRIBA en `S_FALLO`,
del cliente y el PMT el 27/08/2026 (§5), tiene fila desde el 09/11: `D-164`. Manda esta spec;
`DECISIONES.md` es su indice; el cobre, `17_...` y `SPEC_5`. Nacido el 14/09/2026 al
partir `SPEC_5` por lector (`roadmap.md` 1.45).*
