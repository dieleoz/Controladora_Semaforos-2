# SPEC 1 — El ciclo y las luces de UN poste

**Qué es esto.** Lo que un poste hace con sus tres luces y su pluma: los modos que existen HOY, los tiempos, las
transiciones, la barrera de salidas y el estado seguro, escrito contra el fuente. **Qué NO entra.** La
coordinación entre los dos postes (SPEC 2) · la hora y el Degradado como acuerdo de reloj (SPEC 3) · la app
(SPEC 4) · el cobre y los pines físicos (SPEC 5). **De dónde sale.** Las decisiones vigentes del responsable
(`DECISIONES.md`) → el firmware de las dos puntas → las reglas de seguridad numeradas (`OPTIMIZACIONES.md`); donde
el código y un manual difieren, manda el código. 🔴 **NINGUNA CIFRA VIVE AQUÍ** (`CLAUDE.md` §14): se nombra la
constante y dónde vive, y el valor se saca del fuente con el `grep` que acompaña a cada bloque.

## 1. Las salidas que este documento gobierna

| salida | símbolo | dónde se declara |
|---|---|---|
| Rojo, Ámbar, Verde de la cara 1 | `ROJO1` · `AMARILLO1` · `VERDE1` | `{Maestro,Esclavo}/include/pines.h` |
| Rojo, Ámbar, Verde de la cara 2 | `ROJO2` · `AMARILLO2` · `VERDE2` | idem |
| La pluma (talanquera) | `MOTOR_TALANQUERA`, con `TALANQUERA_ABRIR` / `TALANQUERA_CERRAR` | idem |

**Las dos caras de un poste van SIEMPRE juntas.** La función que escribe los pines (`escribirPines()`) pone el mismo
valor en la cara 1 y en la 2 con la misma orden; no hay camino que las separe. Un poste tiene **un** color, no dos.
⚠️ **Las dos luces de peatón están DECLARADAS en la cabecera de pines y el firmware NO las conduce** (ni configura el
pin ni lo escribe, en ninguna punta): el equipo mueve **seis** pines de luz más la pluma, no ocho. Una regla que
enumera sujetos tiene que comprobar que cada sujeto existe (`N-96`).
`grep -rn "ROJO_PEATON\|VERDE_PEATON" 01_Firmware/{Maestro,Esclavo}/src`

## 2. La barrera de salidas — la puerta única a los pines

> **Sólo el fichero del semáforo escribe pines de luz, y todo pasa por su función de escritura.** Es la
> propiedad más importante de este documento y la única que ningún modo puede rodear (`SFTY-2`).

🔴 **EL PRINCIPIO QUE GOBIERNA LA PLUMA, y del que las excepciones de abajo son consecuencia** (responsable,
13/09): *«un semáforo funciona sin pluma; si falla algo, ARRIBA»*. **La pluma REFUERZA la señal; no la
sustituye** —quien regula es la lámpara—. De ahí las dos mitades que cuesta leer juntas: equipo **vivo que
sabe que falló** ABRE —una barrera cerrada que nadie gobierna encierra el corredor de obra—; equipo **muerto**
CIERRA, porque el pin cae a reposo. Y **ninguna avería de la pluma detiene el ciclo**: nadie se entera.

1. **La puerta única de la lógica es una sola función** (`aplicarSalidas()`). Aplica el **enclavamiento**
   —nunca verde y rojo a la vez; **el rojo siempre gana**— y sólo después escribe los pines.
2. **La función de escritura es la única que toca un pin de luz** en las dos puntas; lo censan
   `barrera_01_pines_de_luz` (los ocho pines, peatonales incluidos) y `barrera_02_dos_puntas`.
3. **La pluma sale por la MISMA puerta que las lámparas** (`SFTY-28`). Su orden de motor vive **dentro** de
   la función de escritura, sobre el mismo verde **ya enclavado**: si un modo pudiera moverla por su cuenta
   habría pluma arriba con luz en rojo, **peor** que no tener barrera —el conductor confía en ella—. Lo
   vigila `barrera_03_talanquera`.
4. **Dos excepciones, dentro de la propia condición:** **no** sigue al verde de un test de lámparas (una
   prueba de taller no abre la vía; `testLedsActivo`, `N-82`); y **sí sube en estado de fallo** (el principio).
5. **La bandera que dice si la pluma está arriba se asigna DENTRO del paréntesis de esa condición**, no al lado: el
   pin y lo que se lee de él no pueden discrepar. Y **no sólo se publica** (campo `PLUMA:` de la app): **el vigilante
   de cámaras la usa de reloj** —sólo acumula silencio con la pluma ARRIBA, el *«con el ciclo corriendo»* de la
   decisión de cámaras—, y una segunda copia habría que mantenerla a mano: por eso no existe.
6. 🔴 **El pin tiene UN SEGUNDO ESCRITOR, y sólo uno: el cierre de arranque.** El arranque del semáforo cierra la
   pluma antes de que haya lógica —dejar el pin como quedó sería vía abierta sin regular durante todo el arranque— y
   **repite la bandera a mano**, por ser el único camino que no pasa por la función de escritura. **Dos escrituras
   por punta y ninguna más**, las dos en el fichero del semáforo, y la segunda sólo CIERRA.
   `grep -n MOTOR_TALANQUERA 01_Firmware/*/src/semaforo.cpp`

| luz | pluma |
|---|---|
| Rojo · todo-rojo de despeje · ~~ámbar de transición~~ *(salió con `D-45`, `cda33df`)* · ~~destellos y ámbar rápido del mando~~ *(salieron con el mando, 14/09)* | ABAJO |
| **Rojo + amarillo de apertura** (`D-53`, §3.3; construido sin banco, `N-176`) | **ABAJO**: sube con el VERDE real |
| Verde de ciclo | ARRIBA |
| **Amarillo de cierre** (`D-45`, §3.2; construido en `cda33df`, sin banco) | **ARRIBA**: `luzPideArriba` lleva `estado == S_AMARILLO`; el retardo de bajada cuenta desde el ROJO (SPEC 8 §1) |
| Verde y amarillo de test de lámparas | **ABAJO** (en el test `estado` sigue en `S_ROJO`) |
| Fallo (ámbar intermitente) | **ARRIBA** |
| Equipo sin energía | ABAJO (nivel de reposo del pin) |

⚠️ **Lo que la pluma NO tiene: realimentación.** No hay final de carrera —el responsable dejó `J14` sin
cablear (`D-27`)—: el equipo sabe *«ordené abrir»*, nunca *«está abierta»*, y no finge lo contrario.

## 3. Los estados de la luz

Cuatro estados, declarados en la cabecera del semáforo (`EstadoSemaforo`): **rojo**, **verde**, **ámbar** y **fallo**.
Lo lee todo el firmware por un solo consultor (`semaforo_estado()`); el equipo se considera **estable** en rojo,
verde y fallo (`semaforo_estable()`), y no mientras dura el amarillo de cierre.

| estado | lo que se ve | quién lo pone |
|---|---|---|
| Rojo | rojo fijo | forzar rojo desde lo que no es verde · el fin del amarillo de cierre · apagar todo (apaga las tres) |
| Ámbar (`S_AMARILLO`) | amarillo fijo, **sólo al CERRAR un verde**, durante `AMARILLO_SEG` (§3.2) | forzar rojo sobre un verde, por `iniciarTransicionARojo()` |
| Verde | verde fijo | forzar verde, **directo** desde rojo (HACE hoy; `D-53` lo deroga, §3.3) |
| Fallo | **ámbar intermitente** | el arranque de fallo (`semaforo_iniciarFallo()`) |
| *Rojo+amarillo (`S_ROJO_AMARILLO`)* | *rojo y amarillo a la vez, `ROJO_AMARILLO_SEG`* | *HUECO (`D-53`, §3.3): forzar verde sobre rojo* |

**Las transiciones son ASIMÉTRICAS, y la asimetría es el corazón de este documento** *(desde `cda33df`, `D-45`; sin
banco)*. **De rojo a verde se pasa directo** (HACE hoy; con `D-53`, por rojo+amarillo, §3.3)**; de verde a rojo, por
amarillo**: forzar rojo sobre un verde enciende el
amarillo y anota el instante, y la máquina de luces (`semaforo_actualizar()`) salta a rojo al cumplirse
`AMARILLO_SEG`. **El fallo parpadea** invirtiendo el ámbar cada medio periodo, con un plazo literal sin nombre — ver
Huecos. Y **la máquina de luces avanza SIN CONDICIÓN en cada vuelta del bucle principal de las dos puntas**: un
cabezal que dependiera de que un modo se acordase puede quedarse a oscuras, y pasó.

### 3.1 🟢 ~~NO HAY ÁMBAR AL TERMINAR EL VERDE~~ → CERRADO por `D-45` en `cda33df` (sin banco)

**Lo que el equipo hacía hasta `cda33df`:** forzar rojo ponía el estado y el rojo en la misma llamada, en todos los
caminos; el ámbar sólo existía de camino al verde (la transición a verde, retirada). **El conductor no recibía
ningún aviso de que el verde se acababa**: lo único entre los dos sentidos era el todo-rojo de despeje (§5).
**Hoy** la función de transición a rojo existe, una por punta, estática en el fichero del semáforo:
`grep -rn "TransicionARojo" 01_Firmware/{Maestro,Esclavo}/src/semaforo.cpp`. El comentario del Esclavo que
llamaba al verde→rojo directo *«la Resolución»* se corrigió en el fuente: era una afirmación normativa sin
verificar (`CLAUDE.md` §6) y la norma pide lo contrario (4.4.3). Lo retirado en su día —la transición **europea
rojo+ámbar → verde**— hablaba de cómo se ABRE el verde, y sigue retirado.

### 3.2 ROJO – VERDE – AMARILLO – ROJO (`D-45`, `N-174`) — CONSTRUIDO en `cda33df`, SIN BANCO

**La norma.** Manual de Señalización Vial 2024, 4.4.3 (pp. 394-395): *«En ningún caso se podrá cambiar de luz verde a
luz roja, o a rojo intermitente, sin que antes aparezca el amarillo»*. 4.4.2 (p. 394) da «Rojo – Verde – Amarillo –
Rojo» entre las secuencias vehiculares. La duración sale sólo del rótulo de la Figura 4-9 (p. 394), *«Duración 3
seg. (velocidad 50 km/h)»*: un mínimo de amarillo en texto **NO CONSTA**. Transcripción y exigibilidad:
`fuentes/md/Manual_Senalizacion_Vial_semaforos.md` §2 y §5. El ámbar SOLO antes del verde que hacía el equipo hasta
`cda33df` no era ninguna de las secuencias de 4.4.2: la que avisa del verde es «Rojo y Amarillo», los dos encendidos.

1. **Abrir: de rojo a verde DIRECTO** (`semaforo_forzarVerde()`), en las dos puntas y en todos los modos, Degradado
   incluido. **Es lo que HACE hoy y lo DEROGA `D-53` (05/10): el verde se abre por ROJO+AMARILLO (§3.3, HUECO).**
   `semaforo_iniciarTransicionAVerde()` ya no está en el fuente; las puertas del coordinador y
   `aplicarLuz()` del Degradado del Poste 2 llaman a `semaforo_forzarVerde()`.
2. **Cerrar: de verde a AMARILLO fijo durante `AMARILLO_SEG` y después a ROJO.** No por disciplina de los
   llamadores: `semaforo_forzarRojo()` sobre `S_VERDE` llama a `iniciarTransicionARojo()` (estática, en el fichero
   del semáforo de cada punta) y vuelve; el rojo lo pone `semaforo_actualizar()` al vencer. Así lo heredan todos los
   caminos de verde a rojo: el cambio del Automático y del Inteligente (§6, §7), `DAR PASO` y
   `MANUAL:CAMBIAR_TURNO` (§8), `FORZAR_ROJO` y `SET_MODO:MENU`, la entrada en un modo con ciclo, la suelta por
   margen de las dos puntas y el backstop de verde máximo del Poste 2 (§9), la orden de rojo por radio al Poste 2
   (SPEC 2 §2.2.bis), y en el Degradado el fin de la fase propia, el salto de hora, la entrada, las salidas, la caída
   a rojo sin hora (`D-38`) y la caída a ámbar (`irAAmbar()`: amarillo, rojo y el parpadeo; SPEC 2 §7).
3. **El amarillo va SOLO en la cara** (`iniciarTransicionARojo()` aplica rojo y verde apagados). **Se AÑADE antes
   del todo-rojo de despeje y no se le resta** (§5). Y **un cierre empezado no se revierte**: con `S_AMARILLO`,
   `semaforo_forzarRojo()` no lo reinicia y `semaforo_forzarVerde()` no lo reabre; de él sólo se sale a rojo, al
   vencer, o a ámbar intermitente (`semaforo_iniciarFallo()` no lleva guarda). Si el test de lámparas termina con el
   cierre en curso, `terminarTestLeds()` repinta el amarillo sin tocar su reloj.
4. **La barrera vive en el fichero del semáforo** (§2): de `S_VERDE` sólo se sale por `iniciarTransicionARojo()` o
   por el fallo. El rojo inmediato queda para lo que no es verde (rojo, fallo, apagado).
5. **El rojo ya no llega en la misma llamada, y todo lo que contaba desde la orden cuenta desde el ROJO ENCENDIDO.**
   En el coordinador, `coordinador_actualizar()` renueva `tRef` en la vuelta en que la luz pasa de `S_AMARILLO` a
   `S_ROJO`, sea cual sea el estado, y `C_MASTER_A_ROJO` espera `semaforo_estable()` en rojo; el acuse de rojo del
   Poste 2 sale al encenderse su rojo (SPEC 2 §2.2.bis); en los dos Degradados, mientras dura el amarillo se renueva
   el reloj de los todo-rojo de entrada, de salida y del rojo previo al ámbar (`tEstado` en el Poste 1,
   `tCambioEstado` en el Poste 2). Un `static_assert` del coordinador exige `DESPEJE_SEG_MIN > AMARILLO_SEG`.
6. **Lo que NO lleva amarillo, y por qué.** **(a)** De verde a **ámbar intermitente** (fallo: silencio, orfandad,
   orden de ámbar, ámbar de emergencia) sigue directo: 4.4.3 regula el paso a rojo y a rojo intermitente, no éste.
   `modo_ambar_setup()` del Poste 1 llama a `coordinador_forzarRojoTotal()` y a `semaforo_iniciarFallo()` en la misma
   llamada: el amarillo que arranca el primero lo pisa el segundo antes de verse. **(b)** El test de lámparas (§10) no
   abre ni cierra paso. **(c)** Del fallo, del apagado o del arranque a rojo no hay verde que cerrar.
7. **El estado.** `S_AMARILLO` es ya sólo el amarillo de cierre, y el equipo no es **estable** mientras dura. El
   `$STATUS` publica `ESTADO:AMARILLO` al FINAL del verde (`semaforo_nombreEstado()`, SPEC 4 §6). 🔴 **Abierto:** el
   campo `ESC:` del Poste 1 no tiene amarillo y sigue diciendo `VERDE` hasta el `CMD_ACK_RED` (SPEC 2 §2.2.bis).
8. **La pluma sigue arriba durante el amarillo** y el retardo de bajada cuenta desde el rojo (§2, SPEC 8 §1).

Lo ejercen las filas 17 (`arnes_automatico.cpp`: rojo a verde directo, cierre por amarillo con el coordinador fuera
de reposo), 18 (dos puntas: el orden de la luz en todo el barrido y el amarillo del Poste 2) y 19 (Degradado), con
las pruebas que celebraban lo contrario invertidas y vistas en rojo con el defecto inyectado (mensaje de `cda33df`).
**Sin acta de compuerta sobre `cda33df` en `evidencia/` y sin banco ni tarjeta.** Las que aseveran «rojo a verde
directo» CELEBRAN lo que `D-53` deroga: se invierten con §3.3 (`CLAUDE.md` §9).

### 3.3 ROJO – ROJO+AMARILLO – VERDE – AMARILLO – ROJO (`D-53`, `N-176`) — CONSTRUIDO EL 05/10, SIN BANCO

Todo este apartado es lo que el equipo DEBE hacer; nada de él está en el fuente de `a5db18e`.
**La norma.** Manual de Señalización Vial 2024, 4.4.2 (p. 394), primera secuencia vehicular: «Rojo – Rojo y Amarillo -
Verde – Amarillo – Rojo»; 4.4.1 d: el rojo y amarillo «advierte [...] que está próximo el inicio del intervalo de
verde»; rótulo de la Figura 4-9: «Rojo - Amarillo Duración 2 seg.» (rótulo de figura, no texto normativo); 4.4.4 cuenta
el rojo-amarillo dentro del tiempo de seguridad. Transcripción: `fuentes/md/Manual_Senalizacion_Vial_semaforos.md` §2.
1. **Abrir: de rojo a ROJO+AMARILLO durante `ROJO_AMARILLO_SEG` y después a VERDE**, en las dos puntas y en todos
   los caminos que hoy llaman a `semaforo_forzarVerde()`: Poste 1, las tres puertas de `coordinador.cpp`
   (`C_INICIAL_ESPERA_ESTATICO`, `C_ESPERA_ESTATICO_TRAS_ESCLAVO` y la de la petición de cambio), `FD_VERDE_MAESTRO`
   de `modo_degradado.cpp` y `terminarTestLeds()`; Poste 2, `CMD_GO_GREEN` de `main.cpp` y `aplicarLuz(true)` de
   `modo_degradado.cpp`. Censo: `grep -rn "semaforo_forzarVerde" 01_Firmware/{Maestro,Esclavo}/src`. Como el
   amarillo, no por disciplina de los llamadores: lo pone el fichero del semáforo.
2. **Un estado nuevo, `S_ROJO_AMARILLO`, en `EstadoSemaforo` de las dos puntas.** `semaforo_forzarVerde()` sobre
   `S_ROJO` o `S_FALLO` lo pone, anota el instante y aplica rojo y amarillo (el enclavamiento lo admite: no hay
   verde); sobre `S_ROJO_AMARILLO` vuelve sin reiniciar el reloj; sobre `S_VERDE` repinta el verde; sobre
   `S_AMARILLO` no hace nada (§3.2 (3)). `semaforo_actualizar()` pasa a `S_VERDE` al vencer `ROJO_AMARILLO_SEG`.
3. **Durante el rojo+amarillo NUNCA hubo verde, y por eso se vuelve a ROJO DIRECTO, sin amarillo.**
   `semaforo_forzarRojo()` sobre `S_ROJO_AMARILLO` pone `S_ROJO` en la misma llamada: `FORZAR_ROJO`,
   `SET_MODO:MENU`, la orden de rojo por radio, la suelta por margen, `D-38`/`D-49` y las salidas del Degradado.
   4.4.3 regula el paso desde el VERDE; un amarillo aquí diría que se cierra un paso que no se abrió.
   `semaforo_iniciarFallo()` sigue sin guarda: del rojo+amarillo al ámbar intermitente, directo.
4. **No es estable** (`semaforo_estable()` falso): los coordinadores esperan el verde real
   (`C_MASTER_A_VERDE`, `C_INICIAL_MASTER_A_VERDE`), y `quienVerde` se pone con él. 🔴 **Para el arquitecto:** si el
   rojo+amarillo acaba en rojo (punto 3) con el coordinador en una de esas dos esperas, la espera no puede quedarse
   colgada esperando un verde que no llegará; quien fuerza el rojo tiene que mover también el estado del coordinador.
5. **Los relojes de fase cuentan desde el VERDE real, en las dos puntas.** El Automático y el Inteligente cuentan con
   `coordinador_listoParaContar()`, que sólo es cierto en `C_IDLE`, al que se llega con el verde propio estable o con
   el `CMD_ACK_GREEN` del Poste 2, que con `D-53` sale con su verde encendido (SPEC 2 §2.2.ter). Un verde de 21 s
   sigue durando 21 s y el ciclo crece un `ROJO_AMARILLO_SEG` por sentido (§5). El suelo y el techo del Inteligente
   son del verde (§7). El backstop del Poste 2 (`tInicioVerdeEsclavo`) ya se arma sólo en `S_VERDE` (§9).
6. **`DAR PASO` y `MANUAL:CAMBIAR_TURNO` (§8):** durante el rojo+amarillo el coordinador no está en `C_IDLE` y la
   orden contesta `EN_TRANSICION_REINTENTE`, como en el despeje y el amarillo.
7. **La pluma sigue ABAJO durante el rojo+amarillo y sube con el verde real** (`D-53`): `luzPideArriba` no gana
   término, porque mira el verde ya enclavado y `S_FALLO`/`S_AMARILLO`. SPEC 8 §1.
8. **El veto de margen al ABRIR el verde propio del Poste 1** (`puedeSostenerVerde()`, SPEC 2 §4) lleva además
   `ROJO_AMARILLO_MS`, para que el verde empiece con el margen de hoy. El umbral de silencio y la suelta de `D-50`,
   abiertos para el arquitecto: SPEC 2 §2.2.ter.
9. **El estado se publica:** `semaforo_nombreEstado()` devuelve `ROJO+AMAR` (9 caracteres, el tope del campo
   `ESTADO` en el presupuesto del `$STATUS`, que hoy marca `FALLO COM`). La app, SPEC 4 §6.
10. **Lo que NO lleva rojo+amarillo.** **(a)** El test de lámparas (§10): rojo, ámbar y verde sueltos, fuera de
   servicio, con la pluma abajo; no abre paso (§3.2 (6)(b)). Si se corta con un rojo+amarillo en curso,
   `terminarTestLeds()` lo repinta sin tocar su reloj, como el amarillo; y como el test sólo corre en `S_ROJO` y se
   corta en cuanto la luz deja el rojo, un verde abierto durante el test pasa por el rojo+amarillo igual.
   🔴 **Para el arquitecto:** su fase ámbar antes del verde es la secuencia que `D-45` retiró de la calle; se deja
   como está por ser de taller. **(b)** El
   paso a ámbar intermitente y la vuelta a rojo de §3.2 (6)(c).
11. **Constante:** `ROJO_AMARILLO_SEG` = 2 s, FIJA y NO CONFIGURABLE como `AMARILLO_SEG`, gemela en `protocolo.h` de
   las dos puntas, junto a él (`costura_01_contratos` compara la cabecera).
12. **Carga:** el Degradado cambia de fases (SPEC 2 §8 (e.ter)); un poste con `D-53` y otro sin él se desfasan. Los
   dos postes se cargan en la misma visita.

## 4. Los modos que existen HOY

**Poste 1 (Maestro).** Siete modos en el enum `ModoSistema` de `modos.h` (`MODO_HORA` salió con `D-44`); el
estado vive fuera de la pantalla, en su propio fichero, y se lee y escribe con un par de funciones
(`modoActual_get()` / `modoActual_set()`).

| modo | qué hace con las luces | cómo se llega hoy |
|---|---|---|
| Menú | **rojo fijo** en las dos puntas; ámbar intermitente si no hay enlace. **Sin navegación** (`D-44`): de `menu.cpp` sólo queda `menu_setup()` | `SET_MODO:MENU`; la salida de cada modo; el arranque sólo mientras espera la reanudación del Degradado (§4.1) |
| Manual | todo-rojo al entrar y **ningún cambio programado**: la fase acaba cuando alguien pulsa | `SET_MODO:MANUAL`; también al cancelarse un ámbar del Poste 2 |
| Automático | cicla por tiempo | `SET_MODO:AUTO` ~~; secuencia `A.A.A` del mando~~ *(salió el 14/09)* |
| Inteligente | cicla por tiempo **con suelo y techo**, y las cámaras sólo pueden ALARGAR | `SET_MODO:INTELIGENTE` |
| Alcance | **no arranca ciclos**: mantiene lo que haya (rojo fijo con enlace) | `SET_MODO:ALCANCE` |
| Degradado | todo-rojo de entrada y luego verde/amarillo/rojo por reloj | `SET_MODO:DEG_T` (testigo); reanudación tras corte. ~~`SET_MODO:DEGRADADO`~~ salió (`D-46`, §12.11) |
| Ámbar | **ámbar intermitente**, pedido o de arranque | `SET_MODO:AMBAR`; ~~`B.B.B`;~~ aviso del Poste 2; salida del Degradado; **arranque** (§4.1) |

**El modo Ámbar es una salida de emergencia y por eso no tiene condiciones**: funciona desde cualquier modo en
marcha. El ámbar *dentro* del Degradado, en cambio, es un estado interno de ese modo: comparten la luz y las dos
líneas de motivo. ⚠️ **Y la reanudación tras un corte NO la pide nadie: la decide la máquina** (`D-29`, SPEC 2 §7).

**Poste 2 (Esclavo) NO tiene modos de operación.** Su luz la ordena el Poste 1 salvo en tres casos locales — **ámbar
de emergencia con cerrojo** (pedido desde la app), **Modo Degradado** y **ámbar por orfandad** (§9).

> 🔴 **ESE ÁMBAR DE EMERGENCIA CONSERVA SU VETO, Y EN EL FUENTE SON TRES `if`. LAS DOS CUENTAS SON
> CIERTAS Y CUENTAN COSAS DISTINTAS, y por ahí se cuentan mal** (`D-8`).
> **UNO por SUJETO** —la app, que es hoy la única que puede poner ese ámbar— y **TRES por RAMA**: la misma
> condición guarda la orden de rojo, la orden de verde **y la recuperación tras fallo** del bucle del Esclavo, que
> **no es un `else` de la primera**. ⬇️ ~~DOS por sujeto, el mando y la app, como lo escribe la decisión~~
> → **el sujeto «mando» salió del firmware el 14/09 (`D-30`) y su término de la condición con él**, sin abrir
> ningún veto: sin mando, ese término ya valía siempre lo mismo (SPEC 5 §3). Una regla que ENUMERA sujetos
> comprueba que cada sujeto EXISTE, y dónde se ejerce (`CLAUDE.md` §2). El detalle y el recuento, **SPEC 2 §2.3**.
> **La asimetría es el arreglo entero: se guarda lo que ABRE paso, no lo que lo PARA.**

### 4.1 El arranque tras un corte o un watchdog (`D-40`, construido; sin banco)

**Poste 1.** `setup()` de `Maestro/src/main.cpp` elige entre cuatro salidas, en este orden: **(1)** si
`modo_degradado_reanudarTrasCorte()` concede, vuelve al Degradado (`D-29`); **(2)** si la decisión sigue
pendiente (`respaldo_degradadoActivo()`), espera en **Menú** —rojo fijo con enlace— con `esperaReanudacion`
armada; **(2.bis)** si la pila trae el rojo fijo sin hora (`respaldo_rojoSinHora()`, `D-47`, abajo), vuelve a ese
rojo; **(3)** si no, `entrarAmbarDeArranque()`: **MODO_AMBAR entero** con su motivo propio
(`modo_ambar_fijarMotivoDeArranque()`), o sea todo-rojo, orden de ámbar al Poste 2 y ámbar intermitente: **fuera de
servicio**. La espera de (2) acaba en el bucle: si reanuda o alguien elige modo, nada más; si la decisión borra el
indicador y sigue en Menú, pasa al ámbar de arranque. **Se sale con una orden del operario**, como de cualquier
ámbar. El motivo de arranque **es una marca**: `modo_ambar_esDeArranque()` la lee la puerta del testigo para **no
aplicarle el veto R-4** (SPEC 2 §7.bis), que es para el ámbar que pidió una persona; cualquier otro camino al ámbar
fija su motivo y la marca desaparece. El `$STATUS` dice `MODO:AMBAR` en los dos casos: el motivo no se publica.

**Poste 2: NO cambia, y es decisión de diseño** (contra la letra de `D-40`, que nombra a los dos). Reiniciado solo,
arranca en rojo (`semaforo_forzarRojo()` en su `setup()`) y lo recupera el Poste 1 por radio; sin radio, cae a
ámbar por orfandad (§9). Arrancarlo en fallo daría **ámbar contra el verde** de un Poste 1 que siguió ciclando.

**El rojo fijo por falta de hora sobrevive al corte** (`D-47`; construido en `cda33df`, sin banco). Al caer a ese
rojo (`D-38`, SPEC 3 §5), `irARojoSinHora()` de cada punta pone `FLAG_ROJO_SIN_HORA` (bit8 de `REG_FLAGS`) con
`respaldo_guardarRojoSinHora()`, DESPUÉS de bajar el Degradado, que lo borra; lo bajan también
`respaldo_guardarTestigo()` y toda llamada a `respaldo_guardarDegradado()`, o sea toda entrada y salida del modo.
Al arrancar con la marca puesta: el Poste 1 toma la salida (2.bis) de arriba, **antes** del ámbar de arranque, y
entra en `MODO_DEGRADADO` por `modo_degradado_arrancarEnRojoSinHora()`;
el Poste 2 llama a `degradado_arrancarEnRojoSinHora()` tras su reanudación. Los dos vuelven a `irARojoSinHora()`:
rojo fijo gobernando la luz (en el Poste 2, sin orfandad que lo lleve a ámbar), su `$ALARM ROJO_SIN_HORA` y la
salida de siempre. Si el poste estaba en verde al perder la hora, el cierre lleva su amarillo (§3.2).
🔴 **Abierto: el arranque del Poste 1 en ese rojo no lo ejerce ningún arnés** —`Maestro/src/main.cpp` no lo compila
ninguno, y su único llamador es ese `setup()`—; el del Poste 2 sí (fila
19, `H10` de `orquestador_deg_auto2.inc`).

## 5. Los tiempos, y de dónde salen

**Los límites del ciclo viven en UN solo sitio: `Maestro/include/limites_ciclo.h`** —mínimo y máximo de verde,
de rojo y de despeje—. Verde y rojo en **minutos**; el despeje, en **segundos**.
`grep -n "MIN\|MAX" 01_Firmware/Maestro/include/limites_ciclo.h`
- 🔴 **El mínimo por sentido lo fija el responsable** (`D-5`) y es un **límite DURO del firmware**, no de la
  interfaz: fijar tiempos fuera de rango se rechaza con `$ERR,CMD:SET_TIEMPOS,DESC:RANGO`, y no lo salta ni
  una app vieja ni una trama a mano. El porqué vial, en la cabecera de ese mismo fichero de límites.
- **Los valores de arranque salen de esos mismos mínimos**, no de literales, **sobreviven al corte** (se respaldan) y
  el rango **se revalida aunque el checksum apruebe**: un dato íntegro no es válido. **El dueño de los tres números
  es el Modo Automático** y el resto los **lee**: copiarlos es el defecto de las copias a mano (`N-137`).
- 🔴 **Fijar los tiempos NO arranca el ciclo** (`D-11`). Valida, guarda y contesta; no entra en el modo ni programa un
  verde. Y **no se cambian con el ciclo en marcha**: acortaría la fase EN CURSO, incluido un todo-rojo ya empezado.
- **El despeje** (todo-rojo entre sentidos) lo guarda el coordinador y lo fija el modo al configurarse. **Es el único
  de los tres que es seguridad vial pura** —garantiza que el tramo quedó VACÍO antes de abrir el otro lado—. El aviso al
  conductor es el amarillo de cierre (§3.2); el despeje sigue siendo el margen para quien se quedó dentro.
- **El amarillo de cierre, `AMARILLO_SEG` = 3 s, es FIJO y NO CONFIGURABLE** (`D-45`; `cda33df`, sin banco). No
  entra en `limites_ciclo.h`, ni en `SET_TIEMPOS`, ni en el respaldo: lo fija la decisión (rótulo de la Fig. 4-9 de
  la norma, §3.2). Vive **una sola vez, en `protocolo.h`**, idéntica en las dos puntas (`costura_01_contratos`), junto
  a `SFTY6_SILENCIO_MS`, porque entra en el presupuesto de radio (SPEC 2 §4, §9) y en la fase del Degradado.
- **Cómo suma.** El verde y el rojo configurados y el despeje **no se tocan**; el amarillo va detrás de cada verde y
  delante de su despeje. Cada cambio de sentido es *verde → amarillo → despeje → verde*, así que **el ciclo completo
  dura dos `AMARILLO_SEG` más que antes de `D-45`** (uno por sentido) y el Degradado, igual (SPEC 2 §8 (e)). El rango
  del despeje y su mínimo no cambian; el mínimo tiene que cubrir el amarillo (`static_assert(DESPEJE_SEG_MIN >
  AMARILLO_SEG)` en el coordinador).
- **HUECO (`D-53`, §3.3): el rojo+amarillo, `ROJO_AMARILLO_SEG` = 2 s, FIJO, en `protocolo.h`, fuera de
  `limites_ciclo.h`, de `SET_TIEMPOS` y del respaldo.** Se SUMA, no sale del verde: cada cambio de sentido pasa a ser
  *verde → amarillo → despeje → rojo+amarillo → verde*, y el ciclo dura dos `ROJO_AMARILLO_SEG` más (uno por
  sentido), también en el Degradado (SPEC 2 §8 (e.ter)). Va DESPUÉS del despeje: el todo-rojo puro no se acorta.

## 6. Modo Automático — el ciclo por tiempo

El arranque del modo recupera el respaldo, configura el coordinador y le pide iniciar, **empezando SIEMPRE por
todo-rojo y su despeje** — una sola puerta, sin arranque alternativo. En cada vuelta, con el coordinador en reposo,
mide lo transcurrido contra la duración de la fase —el rojo configurado si la luz local está en rojo, el verde si no—
y al vencer pide el cambio. **La cuenta atrás** que ve el operario se publica **por PISO, nunca por redondeo**, y
fuera del modo no publica número: un número congelado es peor que un `--`, porque parece que sigue contando.
**El cierre (`D-45`, `cda33df`):** al vencer el verde, `coordinador_pedirCambio()` empieza por el amarillo de la
punta que cierra —la propia (`C_MASTER_A_ROJO`) o la orden de rojo al Poste 2 (`C_ESPERANDO_ACK_RED`)—; mientras
dura, `coordinador_listoParaContar()` es falso y la fase no cuenta. El plazo del verde configurado no absorbe el
amarillo: el amarillo empieza cuando el verde ya venció.

## 7. Modo Inteligente — el Automático con suelo y techo

🔴 **La propiedad que lo hace seguro va primero** (`D-19`): con las cámaras muertas **se comporta EXACTAMENTE
como el Automático** —sin detección la condición de mantener no se cumple jamás y la fase termina en el suelo—:
degrada al comportamiento conocido, no a uno raro.
- **SUELO = el tiempo CONFIGURADO de la fase en curso**, **congelado al empezar la fase**: releerlo dejaría que una
  configuración nueva acortase la fase viva. **Por debajo del suelo no cambia nada** —ni una cámara, ni una demanda a
  mano, ni las dos—: **una cámara no puede ACORTAR una fase, sólo alargarla.** Es el mínimo por sentido, aquí dentro.
- **Cumplido el suelo se cambia igual que el Automático**, salvo el único caso que las cámaras aportan: tráfico en mi
  lado y **enfrente nadie esperando** → MANTIENE.
- **TECHO = el suelo multiplicado por un factor** (`TECHO_POR_SUELO`), **saturado** al máximo del rango vial; se
  deriva, no se escribe. 🔴 **Esta decisión está APROBADA CON UNA CONDICIÓN SIN CUMPLIR** —vale *si* un funcional
  revisa el manual—, así que viaja **POR VALIDAR**.
- **Las tres entradas de presencia son un OR de tres booleanos** —la cámara de demanda, la presencia del conector
  `J16` y la demanda local—, **leídas UNA vez por vuelta** *(cobre, SPEC 5)*.
- **El suelo y el techo son del VERDE** (`D-45`): el amarillo de cierre empieza con `coordinador_pedirCambio()`,
  ya fuera de los dos, y corre con `coordinador_listoParaContar()` en falso, donde el modo no mira las cámaras.

## 8. Modo Manual — DAR PASO

🔴 **En Manual, `DAR PASO` alterna rojo/verde como el automático** (`D-7`), disparado por el operario, y **el
todo-rojo de despeje se queda**. Termina en rojo+verde, no en rojo+ámbar. El arranque del modo fuerza el
todo-rojo — **no** inicia un ciclo, que dejaría un verde ya programado (las dos mitades del defecto de banco:
`DAR PASO` rechazado mientras corría el plazo, y el cruce cambiando solo al vencer). **Aquí no se programa
ningún cambio: lo pide el operario o no pasa.** El cambio se pide por `MANUAL:CAMBIAR_TURNO` y **el acuse
depende de lo que la llamada devolvió** (`CLAUDE.md` §2): `MODO_SIN_CICLO_SALGA_PRIMERO`,
`EN_TRANSICION_REINTENTE` si hay un despeje o una transición en curso, y `OK` sólo si se aceptó. **No se
fuerza: partir un despeje por la mitad es justo lo que no se puede hacer.** Y **este modo ya NO configura
tiempos**: conserva el despeje que haya.
**Con `D-45` (`cda33df`, sin banco):** `DAR PASO` con un verde encendido lo cierra por su amarillo; durante él el
coordinador no está en `C_IDLE` y `CAMBIAR_TURNO` contesta `EN_TRANSICION_REINTENTE`, como en el despeje. Sigue
terminando en rojo+verde: el verde se abre directo (HACE hoy; con `D-53` se abre por rojo+amarillo, §3.3 (6),
HUECO). El comentario de `coordinador_pedirCambio()` se reescribió, y el de
la cabecera de `modo_manual.cpp` ya no habla de «los 4 s de ambar».

## 9. El estado seguro

**El ámbar intermitente por silencio** (`SFTY-6`). El umbral vive **una sola vez**, en la cabecera del protocolo que
comparten las dos puntas: el Poste 1 cae a fallo tras ese silencio; el Poste 2 hace lo mismo por **orfandad**, y **el
Degradado lo veta**. **El ámbar ORDENADO usa la misma puerta** —la orden de ámbar por radio entra por el mismo
arranque de fallo—, y **la orfandad se queda como red** si esa orden se pierde.
**El backstop de verde máximo del Poste 2** (`MAX_VERDE_BACKSTOP_MS`) fuerza rojo si el verde se alarga más de la
cuenta. **Mide la LUZ, no la última orden recibida** —las órdenes repetidas ya no reinician la cuenta—, porque el
verde del Degradado no lo ordena nadie por radio.
**El watchdog** (`SFTY-1`) arranca en las dos puntas, se refresca en cada vuelta del bucle y se arma **antes** de
tocar el reloj, para que un cristal que no arranca sea un reinicio visible y no un cuelgue mudo a oscuras. Por eso
**nada de lo que ocupa las luces bloquea**: ~~destellos y~~ el test avanza por el reloj de milisegundos. **Arranque:** el
Poste 2 pone **las luces primero, siempre**; el Poste 1 no — ver Huecos. Tras el arranque, el Poste 1 queda en
**ámbar intermitente** salvo que reanude, espere reanudar el Degradado o vuelva al rojo fijo sin hora (§4.1).
**Con `D-45` (`cda33df`, sin banco):** el backstop cierra por el amarillo y mide sólo el VERDE (`tInicioVerdeEsclavo`
se arma al pasar la luz a `S_VERDE`, ya no a `S_AMARILLO`). La suelta del verde por margen (SPEC 2 §4) cierra por el
amarillo y **empieza un `AMARILLO_SEG` antes** —`SUELTA_VERDE_MS` en el Poste 2, `puedeSostenerVerde()` en el
Poste 1—, para que el ROJO caiga donde caía el rojo directo. El paso a ámbar intermitente por silencio u orfandad no
lleva amarillo (§3.2 (6)).

## 10. Lo que ocupa las luces sin ser un modo

~~**Señales del mando** (`SFTY-21`): destellos rojos y ámbar rápido que INTERCEPTABAN las escrituras a los pines~~
→ **salieron con el mando el 14/09** (`D-30`): ya no queda nada que ocupe las lámparas por encima de la lógica, y la
puerta única escribe siempre lo que decide. **Test de lámparas** (`CMD:TEST_LEDS`): tres fases —rojo, ámbar, verde— que
**entran por la puerta única** y por tanto por el enclavamiento, y **la pluma no sigue a ese verde**. **Solo corre
fuera de servicio** (`N-82.bis`): modo `MENU` o `ALCANCE` (~~`HORA`~~ salió con `D-44`), luz en `S_ROJO`
**y el rojo del Esclavo
confirmado** —su `ACK_RED` a la orden de rojo de esa entrada al menú (`coordinador_rojoEsclavoConfirmado()`)—; en
cualquier otro caso `TEST_LEDS` se rechaza con `$ERR` (`ESPERANDO_ROJO_DEL_ESCLAVO` mientras no llega el acuse),
porque el enclavamiento no conoce el otro poste y ese verde saldría contra el paso del otro sentido. La condición se
pregunta en cada vuelta: si el equipo sale de fuera de servicio, el coordinador deja `C_MENU_IDLE` o se pierde el
enlace (`S_FALLO`), el test se corta en la vuelta siguiente; al acabar o cortarse, la lámpara vuelve a la luz de su
`estado` por su propio setter —no a un rojo fijo—, así que luz, `ESTADO:` y `PLUMA:` dicen lo mismo. Mientras el test
corre, `C_MENU_IDLE` no vuelve a forzar el rojo; en la vuelta siguiente a su final, sí. Medido el 28/09 en el arnés
del automático: en `MENU` con el acuse, las tres fases a 2 s cada una y el final en rojo; en `AUTO`, cero muestras
de verde del Maestro con el Esclavo en verde. ⚠️ **Residual, no construido:** si alguien pulsa el ámbar de
emergencia del Esclavo durante el test, el Maestro solo lo corta cuando lo sabe por radio y pasa a `MODO_AMBAR`;
en esa ventana la fase verde del Maestro puede coincidir con el ámbar intermitente del Esclavo.
~~Con una señal del mando en curso el test espera y se rearma~~ → **corre siempre entero**: la espera se fue con la señal.
**El Poste 2 lo rechaza** (`NO_EN_SERVICIO_USE_EL_MAESTRO`): su test salió con `D-44` (`testLedsActivo` vale `false`).

## 11. Decisiones vigentes que gobiernan este documento

El mínimo por sentido · `DAR PASO` en Manual · los vetos del ámbar de emergencia (§4) · que aplicar tiempos no
arranque el ciclo · el suelo y el techo del Inteligente, **con condición sin cumplir** · la reanudación del Degradado,
que **no es manual** · y la retirada de la interfaz vieja (`D-30`), ~~recortada el 13/09 a que salga sólo el LCD y el mando se quede (`D-32`)~~
→ **reafirmada entera el 14/09**: el LCD y el mando salieron; la lectura de p5/p8 y el menú, con `D-44` (§12.2).
⬇️ Y ~~`A-1.bis` abierta~~ → **el 14/09 el responsable SÍ deroga la mitad de «nunca al revés» de la
regla de la pluma** (`D-33`). La pluma **sigue a la luz para SUBIR**; lo que la cámara puede hacer es **retener la
bajada**, nunca provocar una subida. La conducta entera, en **SPEC 5 pág. 1 §4**.
🔴 **DOS CONDUCTAS DE LA PLUMA Y LA LUZ SIN FILA QUE LAS RESPALDE**, y son de las que hieren a alguien: **(a)** que la
pluma **suba en fallo** (§2), elegido *«por el cliente y el PMT el 27/08/2026»* en una frase que vive sólo en el
fuente y en SPEC 5 §4; y **(b)** que el verde **cierre sin ámbar** (§3.1) → **(b) ya tiene fila: `D-45` (02/10),
amarillo de cierre, construido en `cda33df` y sin banco (§3.2)**. **Son decisiones viales: las decide él, no
el firmware ni esta spec.** ⬇️ ~~**(c)** que la pluma baje sin retardo~~ → **ya tiene fila y está construida.** Bajan
DOS, no tres.

## 12. HUECOS MEDIDOS

Medido el 12/09/2026; **1, 2 y 6 remedidos el 13/09/2026 sobre `bfef121`; 2 y 3 remedidos el 14/09/2026.**
Cada uno trae con qué reproducirlo.
1. 🟢 **CERRADO EL 14/09 — ~~LA PLUMA BAJA EN EL MISMO INSTANTE DEL ROJO~~.** Hoy **baja 3 segundos
   después del rojo**, y **no baja en absoluto mientras una cámara vea algo debajo**. La regla que lo pidió,
   del responsable el 13/09: *«sólo baja segundos DESPUÉS del rojo, porque suelen pasarse carros y hay que
   darle tiempo al conductor a pasar»*. Es **el cambio que sube la versión a `V9.1`**, y la conducta entera
   vive en **SPEC 5 pág. 1 §4** — aquí sólo el puntero. ⬇️ *lo que este hueco decía, y por qué se conserva:*
   el daño que describía **era real y se componía con §3.1** —el verde cerraba SIN ÁMBAR, así que el coche que
   entró legalmente seguía dentro cuando caía el rojo y la pluma bajaba sobre él—. **§3.1 se cerró en `cda33df`
   (`D-45`, sin banco):** el conductor recibe ahora el amarillo, la pluma sigue arriba durante él, y después del
   rojo quedan los tres segundos y el veto de cámara (SPEC 8 §1).
   ⚠️ **Y el contador que medía esto cambió de significado con la obra:** ya no dice «habría actuado» —esa
   transición dejó de ocurrir el día que el veto existe— sino **cuántos vetos ACTUARON de verdad**.
2. 🟢 **CERRADO POR `D-44` (`f9cad1f`, 02/10; sin banco) — LA RETIRADA DE LA INTERFAZ VIEJA (`D-30`).** El LCD
   salió el 13/09, el mando A/B/C/D el 14/09 (`ccca294`, `f57a401`) y el resto con `D-44`. **Lo que el equipo HACE**
   *(registro 1, contra el fuente de las dos puntas)*: `J16` p5/p8 (`PB9`/`PB13`) son `J16_P5_SIN_USO` y
   `J16_P8_SIN_USO`, declarados `INPUT` en `botones_setup()` y **sin lector**: `botones_actualizar()` sólo llama a
   `camaras_actualizar()`. Ya no existen `botonArriba()`/`botonAbajo()`/`botonAceptar()`/`botonCancelar()`, la
   navegación del menú (`menu_loop()`), el Modo Hora (§12.7) ni `semaforo_toggle()` (§12.8). **`menu_setup()` se
   queda**: es la puerta del todo-rojo de las dos puntas (`coordinador_forzarMenu()`) que piden `SET_MODO:MENU`, la
   salida de cada modo y el arranque (§4.1). Cobre y regla de montaje, **SPEC 5 §3**; lo que ve el operador, **SPEC 4
   §3.bis**.
3. 🔴 **LOS HUECOS DE LUZ DEL DEGRADADO** *(el modo entero, SPEC 3)*. **(a)** El verde del Degradado
   (`DEG_VERDE_SEG`) **no pasa por el fichero de límites del ciclo**: es una constante propia del modo. ⬇️ ~~y queda
   muy por debajo del mínimo por sentido, y si ese mínimo alcanza al Degradado no está escrito en ninguna parte~~ →
   **REFUTADO, medido el 14/09: vale exactamente el mínimo por sentido** (subido de 30 s a 180 s el 13/09) **y el
   motivo SÍ está escrito**, en la cabecera del propio modo. **Lo que queda abierto es que nadie cruza los dos
   números:** el día que el mínimo se mueva, el verde del Degradado no le sigue, y ningún instrumento lo dirá.
   🟢 ~~**(b) Las dos puntas ABREN su verde DISTINTO**~~ → **cerrado con `D-45` en `cda33df`:** las dos abren directo
   (`semaforo_forzarVerde()`) y las dos cierran por su amarillo (SPEC 2 §8 (e.bis)). El margen de desfase que se
   apoyaba en el ámbar con que abría el Esclavo es ahora el despeje en los dos sentidos (fila 19, `C3`/`C4`).
   `D-53` cambia la apertura de las dos a rojo+amarillo, también la misma en las dos (§3.3, HUECO).
   ⚠️ **No confundir con la asimetría de la SALIDA a ámbar, que sí está razonada.**
   `grep -n "DEG_VERDE_SEG\|forzarVerde" 01_Firmware/{Maestro,Esclavo}/src/modo_degradado.cpp`
4. ⚠️ **Queda UN plazo literal sin nombre en la máquina de luces**: el medio periodo del parpadeo de fallo. No se
   puede citar, ni vigilar por símbolo, ni releer desde un pack. El otro, el ámbar de transición, salió con `D-45`
   (`cda33df`): el amarillo de cierre se mide con `AMARILLO_SEG` (§5).
   `grep -n "ahora - tCambio >=" 01_Firmware/{Maestro,Esclavo}/src/semaforo.cpp`
5. ⚠️ **El Poste 1 arranca A OSCURAS y el Poste 2 no.** El arranque del coordinador arranca el semáforo —que **apaga
   las tres luces**— y la primera luz no llega hasta el todo-rojo del ámbar de arranque o del menú (§4.1), tras
   arrancar reloj, respaldo, ~~mando y~~ Bluetooth, con una espera de dos segundos en medio y el cabezal apagado.
   ⚠️ **Esa espera ya NO es la bienvenida del LCD**: sobrevive porque su motivo escrito es el watchdog.
6. ⚠️ **El backstop de verde máximo está dimensionado contra un máximo que ya no existe.** Su comentario lo justifica
   *«por encima del máximo configurable (99 min)»* y el máximo real hoy es el del rango vial: protege en la dirección
   segura, pero mucho más laxo de lo que su razón pide — una razón caducada (`CLAUDE.md` §6).
7. 🟢 **CERRADO POR `D-44` (`f9cad1f`) — ~~el Modo Hora es inalcanzable~~.** Salió entero: `modo_hora.cpp` y
   `modo_hora.h` borrados y `MODO_HORA` fuera del enum. Nunca tuvo `SET_MODO:HORA`; su único armador era la navegación
   del menú. El andamio de `SFTY-20` (horario nocturno) vive en `reloj.*` (`reloj_esHorarioNocturno()`).
8. 🟢 **CERRADO POR `D-44` (`f9cad1f`) — ~~`semaforo_toggle()` sin llamador~~.** Salió de las dos puntas.
9. 🟡 ~~EL ARRANQUE ENTRA EN MENÚ Y DEBE ENTRAR EN ÁMBAR (`D-40`, `N-169`)~~ → **construido en el Poste 1**
   (§4.1), **sin banco ni tarjeta**. El Poste 2 no cambia por diseño (§4.1). ~~Riesgo abierto: un poste
   en rojo fijo por hora perdida que sufre un corte arranca en ámbar, verde contra ámbar~~ → **cerrado por `D-47` en
   `cda33df`, sin banco** (§4.1): vuelve al rojo fijo. 🔴 **Abierto:** el arranque del Poste 1 en ese rojo no lo
   ejerce ningún arnés; el del Poste 2, la fila 19.
10. 🟡 **EL AMARILLO DE CIERRE (`D-45`, `N-174`): CONSTRUIDO EN `cda33df`, SIN BANCO NI TARJETA** *(el
   comportamiento, §3.2 y SPEC 2 §2.2.bis, §4, §7, §8 (e.bis); la pluma, SPEC 8 §1)*. Lo que hizo, por símbolo:
   - **Semáforo, las dos puntas**: sale `semaforo_iniciarTransicionAVerde()` y su literal de 4 s; entra
     `iniciarTransicionARojo()` (estática); `semaforo_forzarRojo()` y `semaforo_forzarVerde()` respetan el cierre en
     curso; `luzPideArriba` gana `S_AMARILLO`; `terminarTestLeds()` repinta el cierre; `semaforo_toggle()` sólo
     alterna rojo y verde.
   - **Coordinador del Poste 1**: las tres puertas al verde propio abren con `semaforo_forzarVerde()`; `QV_MASTER`
     cierra por el amarillo y `C_MASTER_A_ROJO` espera el rojo estable; `tRef` se renueva al acabar el amarillo y al
     llegar el `CMD_ACK_RED` en reposo; `C_ESPERANDO_ACK_RED` espera la primera vez `TIMEOUT_ACK_MS + AMARILLO_MS`;
     `puedeSostenerVerde()` y sus `static_assert` llevan `AMARILLO_MS` (SPEC 2 §4, §9).
   - **Bucle del Poste 2**: `CMD_GO_RED` acusa el rojo encendido (`ackRojoPendiente`); `CMD_GO_GREEN` ni reabre ni
     acusa sobre el amarillo; `SUELTA_VERDE_MS` resta `AMARILLO_SEG`; `tInicioVerdeEsclavo` sólo en `S_VERDE`.
   - **Degradado**: `ciclo_degradado.h` en seis fases (`FD_AMARILLO_MAESTRO`, `FD_AMARILLO_ESCLAVO`,
     `ciclo_degradado_faseCruda()`), `aplicarLuz()` del Poste 2 abre directo, los todo-rojo cuentan desde el rojo, y
     el testigo se rechaza también durante el amarillo (`MDT_EN_VERDE`, `DEG_RECHAZO_T_EN_VERDE`).
   - **La pluma**: `PLUMA_RETARDO_BAJADA_MS` y su `static_assert` contra `DESPEJE_SEG_MIN` no cambian (SPEC 8 §1).
   - **Instrumentos invertidos en el mismo commit** (`CLAUDE.md` §9): `arnes_automatico.cpp`, `orquestador.cpp`,
     `orquestador_degradado.cpp`, `orquestador_deg_auto2.inc`, `arnes_ciclo.cpp`, los modelos `esclavo.py` y
     `costura.py` y los packs `barrera_04`, `camara_03`, `costura_09`, `costura_10`, `costura_12`, `esclavo_01`,
     `esp32_13` y `maestro_09`. **No tocados** y que el censo del 02/10 nombraba: `barrera_02_dos_puntas`,
     `costura_06_reanudacion`, `esclavo_03_par_config`, `esclavo_06_no_abre_paso`, `app_04_valores_de_status` y
     `adaptador_esclavo.cpp` (sólo un comentario); su revisión no se ha hecho aquí.
   - 🔴 **LO QUE QUEDA ABIERTO:** **(1)** el borde de la desigualdad (B) del presupuesto de radio (SPEC 2 §2.2.bis,
     §9); **(2)** `ESC:AMARILLO` no se construyó: `ESC:` dice `VERDE` hasta el `CMD_ACK_RED` (SPEC 4 §6). Cerrados
     después: la app dice con la luz en `AMARILLO` «amarillo de cierre: baja al pasar a rojo», y la cabecera de
     `modo_manual.cpp` y los *«25 s»* ya no están en el fuente.
11. 🟢 **CERRADO POR `D-46` (`f9cad1f`, 02/10; sin banco) — SALEN DOS ÓRDENES.** `SOLICITAR_PASO` del Poste 2 y
   `SET_MODO:DEGRADADO` de las dos puntas ya no tienen rama en `procesarComando()`: caen en el `else` final y
   responden `$ERR,CMD:DESCONOCIDO,DESC:COMANDO_NO_SOPORTADO` (`..._EN_ESCLAVO` en el Poste 2). El paso se da con
   Manual y DAR PASO (§8) y el Degradado se pide con el testigo `SET_MODO:DEG_T` (SPEC 2 §7.bis). La puerta
   `modo_degradado_evaluarEntrada()` y su `DEG_RECHAZO` del Poste 1 quedan inalcanzables y se conservan. **Se
   quedan** `J14`/`PB0` (reservada al fin de carrera), el andamio `SFTY-20` y los pines de peatón y zumbador.
12. 🔴 **EL ROJO+AMARILLO ANTES DE CADA VERDE (`D-53`, `N-176`): CONSTRUIDO EL 05/10, SIN BANCO.** Antes el equipo abria
   hoy de rojo a verde directo en todos los caminos (§3.2 (1)); lo que debe hacer, §3.3, con la radio en SPEC 2
   §2.2.ter, la fase del Degradado en SPEC 2 §8 (e.ter), la app en SPEC 4 §6 y la pluma en SPEC 8 §1. Abiertos para
   el arquitecto: §3.3 (4) (la espera del coordinador si el rojo+amarillo acaba en rojo) y (10)(a) (el test).
   Cerrados por el arquitecto: (4) ningun estado del coordinador queda colgado; (10)(a) de taller, no se objeta.

## 13. QUIÉN EJERCE CADA BARRERA DE ESTE DOCUMENTO

> **Una spec puede describir barreras que ningún compilador ejerce, con UNA condición: que cada barrera lleve
> escrito QUIÉN la ejerce.** El criterio es `CLAUDE.md` §6.3 — **¿algún arnés COMPILA ese `.cpp`?**; si sólo lo lee
> por texto no ve un defecto del TIEMPO, y es *vigilada por texto*, no *ejecutada*. Filas = las de la compuerta;
> reparto de `.cpp`, `ARQUITECTURA.map` §4-5. Medido sobre `ef3504c`.

| barrera | quién la EJERCE hoy |
|---|---|
| **El enclavamiento** — nunca verde y rojo a la vez (`SFTY-2`) | 🟡 **PARTIDA. Sólo la fila 17** (`arnes_automatico.cpp`) **y sólo del Maestro.** Los cuatro packs de su fila en `OPTIMIZACIONES.md` LEEN el C++; el semáforo del Esclavo lo COMPILAN las filas 18, 19 y 20, pero ninguna afirma «nunca verde y rojo a la vez» |
| **La pluma dentro de la puerta única** (`SFTY-28`) | ✅ **filas 17, 18 y 19** (miran la salida de motor; la 17 además el publicador de pluma arriba) · `barrera_03_talanquera` por texto |
| **El todo-rojo de despeje** (`SFTY-4`) · **el ámbar por silencio y orfandad** (§9) | ✅ el despeje, **filas 17 y 18**, que corren el ciclo sobre el coordinador real; el silencio, **fila 18** (detalle en SPEC 2 §9) |
| **El watchdog** (`SFTY-1`, §9) | 🟡 **PARTIDA: el reinicio no lo ejerce nadie.** El bucle del Maestro sólo lo cruza PlatformIO; la fila 18 cuenta las recargas del bucle del Esclavo contra un watchdog **simulado** |
| **Menú en rojo fijo** (`SFTY-12`) · **el test de lámparas** (§10) | 🔴 **NADIE las dos.** El forzado de menú y el arranque del test **no aparecen en ningún arnés**; ~~el menú sólo lo compila la fila 14~~ → `menu.cpp` (desde `D-44`, sólo `menu_setup()`) no lo compila nadie: los arneses lo sustituyen por un `menu_setup()` propio, y `maestro_09_test_leds` lee texto |
| ~~**Los destellos INTERCEPTAN** la escritura de pines (§10)~~ | ~~fila 17~~ → **sin sujeto desde el 14/09**: la interceptación salió con el mando (§10). No cuenta abajo |
| **El backstop de verde máximo** (§9) · **el rango duro de tiempos** (`D-5`) · **no reconfigurar en marcha** (`D-11`, §5) | ✅ el backstop, **fila 18**; el rango y la reconfiguración, **fila 17**, y el rango también la 18 |
| **`DAR PASO` en Manual** (`D-7`, §8) | 🟡 **PARTIDA:** el acuse de `MANUAL:CAMBIAR_TURNO` sí (**fila 18**), pero **el Modo Manual del Maestro no lo compila ningún arnés** — sólo PlatformIO |
| **Los vetos del ámbar del Poste 2** (`D-8`, §4) | ✅ **fila 18**, que compila el bucle y el Bluetooth del Esclavo REALES. La cuenta, SPEC 2 §2.3 |
| **El suelo y el techo del Inteligente** (`D-19`, §7) | ✅ **fila 17** (el factor de techo y las tres entradas de presencia) |
| **El amarillo de cierre** (`D-45`, §3.2) | ✅ **filas 17, 18 y 19** (rojo a verde directo, cierre por amarillo, el orden de la luz en las dos puntas y la fase del Degradado), con las pruebas que exigían lo contrario invertidas en `cda33df` · fila 15, la fase pura. Sin acta de compuerta sobre `cda33df` |
| **El rojo+amarillo antes de cada verde** (`D-53`, §3.3) | ✅ **filas 15, 17, 18 y 19 invertidas** (`N-176`), vistas en rojo; sin acta de compuerta |

**Cuenta: 14 barreras — 9 ejecutadas, 2 sin nadie, 3 partidas** *(05/10: entra el rojo+amarillo, construido)*.
Los otros dos rojos **no son casillas sueltas**: el menú y
el test de lámparas son **los dos caminos a los pines de luz que ningún arnés recorre**, y el segundo es justo el que
enciende VERDE sin mirar nada. Y de las tres partidas la más cara es el enclavamiento: **el del ESCLAVO —la punta que
obedece— no lo ejecuta nadie.**
