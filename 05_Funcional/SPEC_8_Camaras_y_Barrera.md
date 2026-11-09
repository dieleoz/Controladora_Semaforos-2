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
> puntero y se queda con la otra mitad, la que **no** es firmware: **sin energia el pin cae, el
> MOSFET no conduce y el brazo baja.** Las dos mitades se leen juntas o ninguna protege a nadie.

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
- **La pluma BAJA tres segundos DESPUES del rojo** (`PLUMA_RETARDO_BAJADA_MS`, elegido por el
  responsable el 14/09 — no derivado), *«porque suelen pasarse carros en ambar y hay que darle unos
  segundos al conductor»*. El retardo no depende de ninguna camara: se cumple con las borneras vacias,
  que es como esta hoy la mayoria de los equipos.
- **Durante el AMARILLO de cierre la pluma SIGUE ARRIBA, y el retardo cuenta desde el ROJO, no desde el amarillo**
  (`D-45`, `N-174`; construido en `cda33df`, sin banco; SPEC 1 §3.2). Es la letra de `D-33` («3 s despues del rojo»)
  y de `D-42` («retardo del rojo sigue en 3 s»): no las reabre. En `escribirPines()` de las dos puntas la condicion
  `luzPideArriba` es `(verde && !testLedsActivo) || estado == S_FALLO || estado == S_AMARILLO`: sin el ultimo
  termino el retardo empezaria en el amarillo y, con los dos a 3 s, la pluma bajaria en el instante del rojo, que es
  lo que `D-33` vino a quitar. El amarillo del test de lamparas no cuenta: alli `estado` sigue en `S_ROJO`. Lo
  ejercen las filas 17, 18 y 19 sobre la salida de motor y `maestro_09_test_leds` por texto. El `static_assert` de
  `PLUMA_RETARDO_BAJADA_MS` contra `DESPEJE_SEG_MIN` no cambia, porque el
  amarillo no resta despeje: tras cada verde la pluma queda arriba `AMARILLO_SEG` mas el retardo, y el otro poste
  sigue abriendo un despeje entero despues del rojo.
- **Durante el ROJO+AMARILLO previo a cada verde la pluma no SUBE; sube con el VERDE real** (`D-53`, `N-176`;
  construido el 05/10, sin banco). `luzPideArriba` no gana termino: mira el verde ya enclavado, y en
  `S_ROJO_AMARILLO` no hay verde (SPEC 1 §3.3 (7)). **Excepcion: si llega arriba** (de `S_FALLO`, del retardo de
  bajada o retenida por el veto), es la bajada pendiente: baja como mucho `PLUMA_RETARDO_BAJADA_MS` despues de que
  la luz dejo de pedirla arriba, salvo veto de camara activo. El rojo+amarillo va despues del despeje del otro poste
  y no lo acorta, asi que la bajada y su `static_assert` no cambian. Si el rojo+amarillo acaba en rojo (orden de
  rojo, perdida de radio), la pluma no ha subido.
- **LA PLUMA ES LENTA, y se queda asi (`D-54`, 06/10).** La de El Sisga tarda ~60 s en subir (Marco, sin medir):
  sube con el VERDE real y deja paso a mitad de un verde de 3 min como minimo (`D-5`); no se adelanta, porque la
  abriria en rojo durante el despeje del otro sentido. **Y la camara solo veta el COMIENZO de la bajada:** mandada
  bajar, `escribirPines()` toma la rama de «pluma ya abajo» y no mira camara durante el minuto que dura el recorrido.
- **Pasado el retardo, CUALQUIERA de las dos camaras del poste VETA la bajada** mientras siga viendo
  algo. Sin consenso: con un AND, una camara muerta anularia el veto para siempre.
- **Al soltarse el veto el firmware baja en el instante, pero la camara mantiene su alarma 5 s tras el evento**
  (de fabrica, `D-42`): la pluma baja unos 5 s despues de que el objeto salga. Esos 5 s son CONFIGURACION de la
  camara. Una camara que avisa DESPUES de los 3 s del rojo encuentra la pluma ya abajo y no la sube.
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
> una camara que dispara sin parar ABRE el contacto entre disparo y disparo, asi que su cronometro
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
  la salida cae y el actuador baja por su muelle o por su peso — **es el fallo seguro de la barrera y
  el software no lo gobierna**: `SPEC_5` §4.

> 🔴 **Y lo que NO hace: no la levanta nada.** La camara solo puede retrasar o impedir la BAJADA
> (`D-33`, §1). **`A-1.bis` sigue abierto en su otra mitad:** que hacer cuando el veto se
> queda pegado. La respuesta de hoy es **avisar y no bajar nunca**, y esta escrita porque se eligio,
> no porque se midiera que sea la mejor.

# 6. 🟡 Los dos interruptores de administrador — **CONSTRUIDOS SIN BANCO el 09/11 (`N-182`)**

🔴 **La barrera fuera de servicio (6.3) la confirmo ARRIBA el responsable el 09/11 (`D-96`), con su modo de fallo.**

Hay dos averias distintas que hoy dejan un poste sin salida, y llevan **dos interruptores
separados en la app, tras la clave de tecnico (`D-56`, `D-145`), uno por poste**. No se pueden juntar en uno: lo que
falla no es lo mismo y lo que queda funcionando tampoco. **No son interruptores fisicos** (`D-94`): no hay
entrada libre decidida —`J14`/`PB0` reservada al fin de carrera (`D-46`), `J16` p5/p8 vacios y sin cablear
(`D-44`)— y un interruptor que el telefono no puede ver puesto es peor que ninguno. Lo que el responsable dejo
abierto el 14/09 lo cerro Fable por delegacion el 14/10 (`D-94`..`D-97`); construido el 09/11 (`N-182`), sin banco:
lo que pide tarjeta con barrera y pila esta en §7 (filas 4, 5 y 7).

| el interruptor | cuando se usa | que hace el equipo despues |
|---|---|---|
| **Sacar la camara del veto** | la camara esta averiada o mal apuntada y **no deja bajar la barrera** | la barrera **vuelve a trabajar**: sube con el verde y baja tras el rojo, con su retardo. Lo unico que se pierde es que la camara pueda retenerla. Saca del veto a **las dos camaras del poste** (una por poste, como se decidio el 14/09; no por camara) |
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
- **Una sola fuente:** las ordenes y `escribirPines()` leen el MISMO getter (`respaldo_vetoCamaraOff()`,
  `respaldo_barreraFuera()`), molde `respaldo_degAuto()`: pasan por `contenidoValido`, asi que antes de
  `respaldo_setup()` (en el Esclavo, `semaforo_setup()` va antes) y con `DR4` corrupto valen `ON`. Sin copia en RAM.
- El `$ACK` queda en el registro del puente (`D-73`): es la huella de quien puso el interruptor y cuando.

## 6.2 Sobreviven al corte: dos banderas en la casilla de indicadores (`D-95`)

Las diez casillas del respaldo estan ocupadas; los dos interruptores caben **como dos banderas libres de `DR4`**
(`REG_FLAGS`: hoy usa los bits 0..8, `FLAG_CICLO`..`FLAG_ROJO_SIN_HORA`): `FLAG_VETO_CAMARA_OFF` (`0x0200`) y
`FLAG_BARRERA_FUERA` (`0x0400`), **dentro de la suma y con la misma `FIRMA`** (molde `FLAG_TESTIGO`, `D-61`): el
formato no cambia y un firmware anterior las ignora. **Las baja solo su orden inversa o `respaldo_borrar()`** (pila
agotada o contenido invalido al arrancar: los dos vuelven a `ON`, el lado seguro); `respaldo_guardarDegradado()` y
`respaldo_guardarTestigo()` no las tocan, porque un Degradado no repara una barrera. El fichero sigue identico en las
dos puntas (`costura_01_contratos` ya compara `respaldo.cpp/.h`). **HUECO en tarjeta, como `D-62`:** que sigan puestas
tras un corte real, con pila; el arnes de dos puntas solo mide que el reinicio simulado las conserve.

## 6.3 Lo que hace el equipo (`D-96`)

- **Camara fuera del veto:** `escribirPines()` no pregunta a `camara_presenciaJ16()`: `plumaVetada` no vale nunca
  `true` y la pluma baja al cumplirse `PLUMA_RETARDO_BAJADA_MS`, como con las borneras vacias. El retardo de 3 s no
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
  rojo la pluma baja a `PLUMA_RETARDO_BAJADA_MS`, o la retiene el veto. Prueba: C++ real en las dos puntas, `S_ROJO`
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
- **`CAM:NOVETO`** (6 <= 6 de `PEGADA`) **solo con el veto fuera Y `camara_estado()` en `OK` o `?` (`D-151`)**; si
  diria `PEGADA` o `CIEGA`, se publica eso aunque el veto este fuera. Asi el campo sigue diciendo lo que 6.3 promete
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
luz.** Sin energia la salida cae y el actuador baja por su muelle o por su peso — es el fallo seguro
de la barrera y **el software no lo gobierna**. El interruptor manda mientras haya corriente.

**El obstaculo que la app midio** —el estado no cabia en la trama, `SPEC_4` §7 hueco 10— **queda cerrado por 6.4**.

# 7. `SIN VERIFICAR` — lo que este documento afirma **sin haberlo visto nunca**

**Escala:** `MEDIDO` = leido del fuente · `SIN VERIFICAR` = nadie lo ha comprobado, ni aqui ni en
ningun sitio. **Lo no verificado del HARDWARE va entero en `SPEC_5` §6** —y de sus veinte filas, las
que muerden a este documento son la **1** (ninguna camara se ha conectado NUNCA a este equipo), la
**2** (cual de los dos estados de la salida de la camara, NO/NC, significa demanda: **sin tomar**) y
la **4** (la concesion de paso por camara nunca se probo)—. Aqui, lo que falta de la CONDUCTA:

| # | lo que nadie ha comprobado |
|---|---|
| 1 | **NINGUNA DE ESTAS CONDUCTAS SE HA VISTO EN UNA TARJETA.** El retardo de 3 s y el veto son del 14/09 y estan **escritos y no cargados**; la instalacion **certificada** es la `V8.4`, anterior a las dos. 🛑 **Que firmware hay hoy en cada equipo lo dice `ESTADO.md`, no este fichero** (`CLAUDE.md` §0.2) |
| 2 | **Que una camara real cierre el contacto el tiempo suficiente para vetar.** El veto se apoya en `camara_presenciaJ16()`, cuya ventana la fija `demanda_ventanaMs()`; **con un contacto mas corto que la ventana el veto se sostiene, con uno mas largo tambien — y ninguno se ha medido con una camara puesta** |
| 3 | **Que el aviso de pluma retenida llegue al telefono.** Sale por el mismo camino que el resto de `$EVENT` (`SPEC_6`), y **el Bluetooth no subio en toda la sesion de banco** (`SPEC_5` §6.17) |
| 4 | **Los dos interruptores de §6: construidos el 09/11 (`interruptores.cpp` en las dos puntas, `js/interruptores.js`) y sin banco.** Tras un corte con la barrera fuera, la pluma vuelve a subir en la siguiente escritura de la luz, no al arrancar: lo mide el arnes de dos puntas (S3), sin fila que lo decida |
| 5 | **Que las dos banderas de §6.2 sigan puestas tras un corte REAL, con pila** — HUECO en tarjeta, como `D-62`; el arnes de dos puntas solo simula el reinicio |
| 6 | **La cinta de campo no ayuda:** 253 tramas del Sisga (10/09) y **las 253 dicen `CAM:?`** — en veinte minutos ninguna camara le dio un flanco al equipo (`SPEC_5` §6) |
| 7 | **El recorrido real de la pluma:** ~60 s de subida segun Marco (06/10), sin cronometrar; la bajada, sin dato |

*Decisiones recogidas: `D-12`, `D-13` (lo no derogado), `D-24`, `D-25`, `D-27`, `D-33`, `D-39`, `D-42`, `D-54`,
`D-94`..`D-97` y `D-148`..`D-152` (§6, construido sin banco). Abiertas
que nombra sin resolver: `A-1.bis`, `A-6`. 🔴 **Y una eleccion vial vigente que NO tiene fila y deberia tenerla:
la pluma ARRIBA en `S_FALLO`, del cliente y el PMT el 27/08/2026 (§5).** Manda esta spec;
`DECISIONES.md` es su indice; el cobre, `17_...` y `SPEC_5`. Nacido el 14/09/2026 al
partir `SPEC_5` por lector (`roadmap.md` 1.45).*
