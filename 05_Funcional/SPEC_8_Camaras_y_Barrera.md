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

**De donde sale cada linea.** Lo decidido lo fija `DECISIONES.md`, que **gana a este fichero**; el
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
- **Pasado el retardo, CUALQUIERA de las dos camaras del poste VETA la bajada** mientras siga viendo
  algo. Sin consenso: con un AND, una camara muerta anularia el veto para siempre.
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

# 5. 🔴 LA PLUMA EN AVERIA: **SUBE con el ambar intermitente** — y esa eleccion NO TIENE FILA

**Este apartado es el hueco que dejo la particion, y por eso se nombra en vez de repartirlo:** que la
barrera suba en estado de averia es **conducta** —la escribe una linea de `escribirPines()`— y a la
vez el extremo de la cadena electrica que vive en `SPEC_5` §4. **Es de aqui.**

- **Sube con verde y tambien en `S_FALLO`** —ambar intermitente: orfandad SFTY-6, Modo Ambar,
  `AMBAR_EMERGENCIA`, Degradado en ambar, un poste recien encendido—; **no sube con el verde de un test
  de lamparas** (`!testLedsActivo`, N-82). **La tabla luz -> pluma entera es SPEC 1 §2** y no se repite.
  Es **SFTY-28**.
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

# 6. 🟡 Los dos interruptores de administrador — **DECIDIDOS el 14/09, SIN CONSTRUIR**

Hay dos averias distintas que hoy dejan un poste sin salida, y llevan **dos interruptores
separados en el modo administrador de la app, uno por poste**. No se pueden juntar en uno: lo que
falla no es lo mismo y lo que queda funcionando tampoco.

| el interruptor | cuando se usa | que hace el equipo despues |
|---|---|---|
| **Sacar la camara del veto** | la camara esta averiada o mal apuntada y **no deja bajar la barrera** | la barrera **vuelve a trabajar**: sube con el verde y baja tras el rojo, con su retardo. Lo unico que se pierde es que la camara pueda retenerla |
| **Sacar la barrera de servicio** | la barrera esta **rota de verdad** —brazo partido, actuador atascado— | el brazo se queda **arriba** y deja de obedecer a la luz. El cruce sigue funcionando **solo con el semaforo** |

**Los dos TIENEN QUE sobrevivir a un corte de luz** —y por eso la tarjeta guarda indicadores en una
memoria que mantiene viva su pila; que sigan puestos de verdad es una de las dos cosas que hay que
medir antes de construirlo, abajo— **y los dos se publican**, porque un poste degradado en silencio es exactamente lo que
este documento existe para evitar: quien se conecte tiene que ver que ese poste no esta entero.

⚠️ **No sobra sitio, y se dice porque el trabajo lo va a encontrar: las diez casillas de esa memoria
estan ocupadas.** Los dos interruptores caben **como dos banderas dentro de la casilla de
indicadores**, que si tiene hueco, y eso obliga a rehacer su suma de comprobacion y a **cambiar las
dos puntas a la vez**, porque ese fichero tiene que ser identico en las dos.

🔴 **Y quedan dos cosas por MEDIR antes de construirlo, no despues:** que el interruptor siga puesto
de verdad tras un corte —no basta con que quepa— y **que hace el equipo si se saca la barrera de
servicio con el brazo abajo**. Subir un brazo no aplasta a nadie, pero el equipo arranca siempre con
la barrera abajo, asi que hay un instante que hay que mirar.

🔴 **Lo que NO garantiza «sacar la barrera de servicio»: que el brazo se quede arriba si se va la
luz.** Sin energia la salida cae y el actuador baja por su muelle o por su peso — es el fallo seguro
de la barrera y **el software no lo gobierna**. El interruptor manda mientras haya corriente.

**Y el obstaculo que la app ya midio: el estado de los dos interruptores NO CABE en la trama de hoy**
—`SPEC_4` §7, hueco 10—. **Mientras no se decida como se publican, no se construyen.**

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
| 4 | **Los dos interruptores de §6: DECIDIDOS y SIN CONSTRUIR.** No hay ni una linea de firmware ni de app; lo que hay que medir antes de escribirla esta en §6 |
| 5 | **Que hace el equipo si se saca la barrera de servicio con el brazo abajo** — §6. El equipo arranca siempre con la barrera abajo |
| 6 | **La cinta de campo no ayuda:** 253 tramas del Sisga (10/09) y **las 253 dicen `CAM:?`** — en veinte minutos ninguna camara le dio un flanco al equipo (`SPEC_5` §6) |

*Decisiones recogidas: `D-12`, `D-13` (lo no derogado), `D-25`, `D-27`, `D-33`. Abiertas que nombra
sin resolver: `A-1.bis`, `A-6`. 🔴 **Y una eleccion vial vigente que NO tiene fila y deberia tenerla:
la pluma ARRIBA en `S_FALLO`, del cliente y el PMT el 27/08/2026 (§5).** Lo decidido lo fija
`DECISIONES.md`, que **gana a este fichero**; el cobre, `17_...` y `SPEC_5`. Nacido el 14/09/2026 al
partir `SPEC_5` por lector (`roadmap.md` 1.45).*
