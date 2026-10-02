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
| Rojo · ámbar de transición *(hoy, de camino al verde; sale con `D-45`)* · todo-rojo de despeje · ~~destellos y ámbar rápido del mando~~ *(salieron con el mando, 14/09)* | ABAJO |
| Verde de ciclo | ARRIBA |
| **Amarillo de cierre** (`D-45`, §3.2; **sin construir**) | **ARRIBA**: el retardo de bajada cuenta desde el ROJO (SPEC 8 §1) |
| Verde de test de lámparas | **ABAJO** |
| Fallo (ámbar intermitente) | **ARRIBA** |
| Equipo sin energía | ABAJO (nivel de reposo del pin) |

⚠️ **Lo que la pluma NO tiene: realimentación.** No hay final de carrera —el responsable dejó `J14` sin
cablear (`D-27`)—: el equipo sabe *«ordené abrir»*, nunca *«está abierta»*, y no finge lo contrario.

## 3. Los estados de la luz

Cuatro estados, declarados en la cabecera del semáforo (`EstadoSemaforo`): **rojo**, **verde**, **ámbar** y **fallo**.
Lo lee todo el firmware por un solo consultor (`semaforo_estado()`); el equipo se considera **estable** en rojo,
verde y fallo, y no mientras corre la transición.

| estado | lo que se ve | quién lo pone |
|---|---|---|
| Rojo | rojo fijo | forzar rojo · apagar todo (que apaga las tres) |
| Ámbar | ámbar fijo, **sólo de camino al verde** — 🟡 **con `D-45` pasa a ser el amarillo de CIERRE del verde** (§3.2) | el arranque de la transición a verde |
| Verde | verde fijo | forzar verde · el vencimiento de la transición |
| Fallo | **ámbar intermitente** | el arranque de fallo (`semaforo_iniciarFallo()`) |

**Las transiciones son ASIMÉTRICAS, y la asimetría es el corazón de este documento.** **De rojo a verde se pasa por
ámbar**: el arranque de la transición enciende ámbar y anota el instante, y la máquina de luces salta a verde al
cumplirse el plazo. **El fallo parpadea** invirtiendo el ámbar cada medio periodo. Los dos plazos son literales sin
constante con nombre — ver Huecos. Y **la máquina de luces avanza SIN CONDICIÓN en cada vuelta del bucle principal
de las dos puntas**: un cabezal que dependiera de que un modo se acordase puede quedarse a oscuras, y pasó.
🟡 **Esa asimetría es lo que el equipo HACE hoy y la decisión `D-45` la INVIERTE** (§3.2, sin construir): se abre
directo y se cierra por amarillo.

### 3.1 🔴 NO HAY ÁMBAR AL TERMINAR EL VERDE — el semáforo salta de VERDE a ROJO

> 🟡 **Registro de lo que el equipo HACE hoy, y es lo que CAMBIA.** Deja de ser un hueco sin decisión: el
> responsable decidió el 02/10 la secuencia de la norma (`D-45`). Lo que el firmware DEBE hacer está en §3.2 y la
> cola de construcción en §12.10 (`N-174`). Hasta que se cargue, todo lo de este apartado sigue siendo cierto.

**Forzar rojo pone el estado y aplica el rojo en la MISMA llamada, sin estado intermedio.** No es un camino entre
varios: **es el único que existe.** **Ninguna de las dos puntas tiene función de transición a rojo** —el `grep` de
abajo da **cero** en el fuente— y el estado de ámbar lo escribe sólo el arranque de la transición a verde, o sea
**únicamente en el sentido contrario**. Las llamadas a forzar rojo se cuentan por **decenas** (coordinador,
Degradado, radio; ~~mando~~, que salió del firmware el 14/09), varias con el comentario literal `// Directo a rojo`, y **todas hacen lo mismo**.
`grep -rn "TransicionARojo" 01_Firmware --include=*.cpp --include=*.h` *(sin filtro salen los índices binarios del editor, no fuente)*
⚠️ **NO se confunda con lo que sí está documentado, que es LO CONTRARIO.** Lo que se retiró en su día fue la
transición **europea rojo+ámbar → verde**, citando el Manual de Señalización de Colombia: eso habla de cómo se
**ABRE** el verde. **Cerrarlo sin ámbar es otra cosa, y sobre ella** ~~no hay decisión~~ → **decidió `D-45`
(§3.2)**; **ni optimización, ni manual.** El
único rastro es un comentario del Esclavo que dice que devolver a ámbar un verde ya encendido sería *«contra la
Resolución (verde→rojo directo)»*: ⚠️ **una AFIRMACIÓN NORMATIVA SIN VERIFICAR** (`CLAUDE.md` §6) que vive en un
comentario y no en una spec.
🔴 **LA CONSECUENCIA, que es lo que importa: el conductor NO RECIBE NINGÚN AVISO DE QUE EL VERDE SE ACABA.** El paso
se le retira en seco, y **lo que hay en su lugar no es un aviso, es un margen:** el **todo-rojo de despeje** (§5),
que retiene a la otra punta mientras el tramo se vacía. El diseño **no avisa: cierra y espera.** Quien ya entró
queda dentro con el rojo puesto — y de ahí cuelga el hueco de la pluma (§12.1).

### 3.2 🟡 LO QUE DEBE HACER: ROJO – VERDE – AMARILLO – ROJO (`D-45`, `N-174`) — SIN CONSTRUIR

**La norma.** Manual de Señalización Vial 2024, 4.4.3 (pp. 394-395): *«En ningún caso se podrá cambiar de luz verde a
luz roja, o a rojo intermitente, sin que antes aparezca el amarillo»*. 4.4.2 (p. 394) da «Rojo – Verde – Amarillo –
Rojo» entre las secuencias vehiculares. La duración sale sólo del rótulo de la Figura 4-9 (p. 394), *«Duración 3
seg. (velocidad 50 km/h)»*: un mínimo de amarillo en texto **NO CONSTA**. Transcripción y exigibilidad:
`fuentes/md/Manual_Senalizacion_Vial_semaforos.md` §2 y §5. El ámbar SOLO antes del verde que hace hoy no es
ninguna de las secuencias de 4.4.2: la que avisa del verde es «Rojo y Amarillo», los dos encendidos.

1. **Abrir: de rojo a verde DIRECTO.** Sale el ámbar fijo de camino al verde, en las dos puntas y en todos los modos,
   Degradado incluido. El arranque de la transición a verde deja de existir como camino al verde.
2. **Cerrar: de verde a AMARILLO fijo durante `AMARILLO_SEG` y después a ROJO**, en TODOS los caminos que hoy pasan
   de verde a rojo: el cambio del Automático y del Inteligente (§6, §7), `DAR PASO` y `MANUAL:CAMBIAR_TURNO` (§8),
   `FORZAR_ROJO` y `SET_MODO:MENU`, la entrada en un modo con ciclo, la suelta del verde por margen de las dos puntas
   y el backstop de verde máximo del Poste 2 (§9), la orden de rojo por radio al Poste 2 (SPEC 2 §2.2), y en el
   Degradado el fin de la fase propia, el salto de hora, la entrada, las salidas, la caída a rojo sin hora (`D-38`) y
   la caída a ámbar, que hoy pasa por rojo (SPEC 2 §7).
3. **El amarillo va SOLO en la cara**: ni verde+amarillo ni verde+rojo (4.4.3, pp. 394-395). **Se AÑADE antes del
   todo-rojo de despeje y no se le resta** (§5). Y **un cierre empezado no se revierte**: del amarillo de cierre sólo
   se sale a rojo, al vencer, o a ámbar intermitente; ninguna orden de verde lo reabre y una orden de rojo repetida
   no lo reinicia.
4. **La barrera vive en el fichero del semáforo** (§2): **ningún camino pasa de verde a rojo sin el amarillo, lo
   pida quien lo pida**, y no por disciplina de los llamadores. Propuesta para quien construya: forzar rojo sobre un
   verde arranca el cierre; el rojo inmediato queda para los estados que no son verde (rojo, fallo, apagado).
5. 🔴 **Consecuencia: el rojo deja de llegar en la misma llamada.** Todo lo que hoy cuenta desde la orden de rojo
   —el despeje y su instante de inicio en el coordinador, el acuse de rojo del Poste 2, el todo-rojo de entrada del
   Degradado, el rojo previo a su ámbar— cuenta desde el ROJO ENCENDIDO (estado rojo y estable), no desde la llamada.
6. **Lo que NO cambia, y por qué.** **(a)** De verde a **ámbar intermitente** (fallo: silencio, orfandad, orden de
   ámbar, ámbar de emergencia) sigue directo: 4.4.3 regula el paso a rojo y a rojo intermitente, no éste. La
   entrada en Modo Ámbar del Poste 1 fuerza rojo y arranca el fallo en la misma vuelta: el rojo no se ve y cuenta como
   este caso; si la construcción lo hiciera visible, lleva amarillo antes. **(b)** El test de lámparas (§10) no
   abre ni cierra paso. **(c)** Del fallo, del apagado o del arranque a rojo no hay verde que cerrar.
7. **El estado.** El ámbar fijo de la máquina de luces pasa a ser el amarillo de cierre; el equipo sigue sin ser
   **estable** mientras dura. El `$STATUS` publica `ESTADO:AMARILLO` al FINAL del verde (SPEC 4 §6).
8. **La pluma sigue arriba durante el amarillo** y el retardo de bajada cuenta desde el rojo (§2, SPEC 8 §1).

## 4. Los modos que existen HOY

**Poste 1 (Maestro).** Ocho modos declarados en su cabecera de modos; el estado vive fuera de la pantalla, en
su propio fichero, y se lee y escribe con un par de funciones (`modoActual_get()` / `modoActual_set()`).

| modo | qué hace con las luces | cómo se llega hoy |
|---|---|---|
| Menú | **rojo fijo** en las dos puntas; ámbar intermitente si no hay enlace | `SET_MODO:MENU`; el arranque sólo mientras espera la reanudación del Degradado (§4.1) |
| Manual | todo-rojo al entrar y **ningún cambio programado**: la fase acaba cuando alguien pulsa | `SET_MODO:MANUAL`; también al cancelarse un ámbar del Poste 2 |
| Automático | cicla por tiempo | `SET_MODO:AUTO` ~~; secuencia `A.A.A` del mando~~ *(salió el 14/09)* |
| Inteligente | cicla por tiempo **con suelo y techo**, y las cámaras sólo pueden ALARGAR | `SET_MODO:INTELIGENTE` |
| Alcance | **no arranca ciclos**: mantiene lo que haya (rojo fijo con enlace) | `SET_MODO:ALCANCE` |
| Hora | no toca las luces | 🔴 **INALCANZABLE** — ver Huecos |
| Degradado | todo-rojo de entrada y luego verde/rojo por reloj | `SET_MODO:DEGRADADO` (sale, `D-46`, §12.11); `SET_MODO:DEG_T` (testigo); reanudación tras corte |
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

**Poste 1.** `setup()` de `Maestro/src/main.cpp` elige entre tres salidas, en este orden: **(1)** si
`modo_degradado_reanudarTrasCorte()` concede, vuelve al Degradado (`D-29`); **(2)** si la decisión sigue
pendiente (`respaldo_degradadoActivo()`), espera en **Menú** —rojo fijo con enlace— con `esperaReanudacion`
armada; **(3)** si no, `entrarAmbarDeArranque()`: **MODO_AMBAR entero** con su motivo propio
(`modo_ambar_fijarMotivoDeArranque()`), o sea todo-rojo, orden de ámbar al Poste 2 y ámbar intermitente: **fuera de
servicio**. La espera de (2) acaba en el bucle: si reanuda o alguien elige modo, nada más; si la decisión borra el
indicador y sigue en Menú, pasa al ámbar de arranque. **Se sale con una orden del operario**, como de cualquier
ámbar. El motivo de arranque **es una marca**: `modo_ambar_esDeArranque()` la lee la puerta del testigo para **no
aplicarle el veto R-4** (SPEC 2 §7.bis), que es para el ámbar que pidió una persona; cualquier otro camino al ámbar
fija su motivo y la marca desaparece. El `$STATUS` dice `MODO:AMBAR` en los dos casos: el motivo no se publica.

**Poste 2: NO cambia, y es decisión de diseño** (contra la letra de `D-40`, que nombra a los dos). Reiniciado solo,
arranca en rojo (`semaforo_forzarRojo()` en su `setup()`) y lo recupera el Poste 1 por radio; sin radio, cae a
ámbar por orfandad (§9). Arrancarlo en fallo daría **ámbar contra el verde** de un Poste 1 que siguió ciclando.

🟡 **DEBE (`D-47`, 02/10; sin construir, `N-174`):** el poste que estaba en rojo fijo por falta de hora (`D-38`,
SPEC 3 §5) arranca tras el corte en **rojo fijo**, no en ámbar; si estaba en verde, antes 3 s de amarillo (`D-45`).

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
  de los tres que es seguridad vial pura** —garantiza que el tramo quedó VACÍO antes de abrir el otro lado— y, **a
  falta de ámbar de cierre (§3.1), es TODA la protección que hay para quien se quedó dentro** *(hoy; con `D-45` el
  aviso es el amarillo y el despeje sigue siendo el margen)*.
- 🟡 **DEBE (`D-45`, sin construir): el amarillo de cierre, `AMARILLO_SEG`, es FIJO y NO CONFIGURABLE.** No entra
  en el fichero de límites ni en `SET_TIEMPOS` ni en el respaldo: lo fija la decisión (3 s, rótulo de la Fig. 4-9 de
  la norma, §3.2). Vive **una sola vez, en una cabecera que las dos puntas comparten idéntica** —propuesta: la de
  protocolo, junto al umbral de silencio, porque entra en el presupuesto de radio (SPEC 2 §4)—, con nombre: no como
  el literal del ámbar de hoy (§12.4).
- **Cómo suma.** El verde y el rojo configurados y el despeje **no se tocan**; el amarillo va detrás de cada verde y
  delante de su despeje. Cada cambio de sentido pasa de *verde → despeje → verde* a *verde → amarillo → despeje →
  verde*, así que **el ciclo completo dura dos `AMARILLO_SEG` más que hoy** (uno por sentido) y el Degradado, igual
  (SPEC 2 §8 (e)). El rango del despeje y su mínimo no cambian.

## 6. Modo Automático — el ciclo por tiempo

El arranque del modo recupera el respaldo, configura el coordinador y le pide iniciar, **empezando SIEMPRE por
todo-rojo y su despeje** — una sola puerta, sin arranque alternativo. En cada vuelta, con el coordinador en reposo,
mide lo transcurrido contra la duración de la fase —el rojo configurado si la luz local está en rojo, el verde si no—
y al vencer pide el cambio. **La cuenta atrás** que ve el operario se publica **por PISO, nunca por redondeo**, y
fuera del modo no publica número: un número congelado es peor que un `--`, porque parece que sigue contando.
🟡 **DEBE (`D-45`):** al vencer el verde, el cambio empieza por el amarillo de la punta que cierra (la propia, o
la orden de rojo al Poste 2); mientras dura el coordinador no está en reposo y la fase no cuenta. El plazo del verde
configurado no absorbe el amarillo.

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
- 🟡 **DEBE (`D-45`):** el suelo y el techo son del VERDE; el amarillo de cierre va después, fuera de los dos, y
  una cámara no lo alarga ni lo acorta.

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
🟡 **DEBE (`D-45`):** `DAR PASO` con un verde encendido lo cierra por su amarillo; durante él `CAMBIAR_TURNO`
contesta `EN_TRANSICION_REINTENTE`, como en el despeje. Sigue terminando en rojo+verde. El comentario del
coordinador que reserva el ámbar *«al paso de rojo a verde, de modo que el par termina en rojo contra verde»* es la
premisa que `D-45` deroga en su primera mitad.

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
**ámbar intermitente** salvo que reanude o espere reanudar el Degradado (§4.1).
🟡 **DEBE (`D-45`):** el backstop de verde máximo cierra por el amarillo, y mide el VERDE: hoy el Poste 2 arranca
esa cuenta también en el ámbar, que con `D-45` es cierre y no apertura. La suelta del verde por margen (SPEC 2 §4)
cierra por el amarillo y **empieza un `AMARILLO_SEG` antes**, para que el ROJO caiga donde hoy cae el rojo directo.
El paso a ámbar intermitente por silencio u orfandad no lleva amarillo (§3.2 (6)).

## 10. Lo que ocupa las luces sin ser un modo

~~**Señales del mando** (`SFTY-21`): destellos rojos y ámbar rápido que INTERCEPTABAN las escrituras a los pines~~
→ **salieron con el mando el 14/09** (`D-30`): ya no queda nada que ocupe las lámparas por encima de la lógica, y la
puerta única escribe siempre lo que decide. **Test de lámparas** (`CMD:TEST_LEDS`): tres fases —rojo, ámbar, verde— que
**entran por la puerta única** y por tanto por el enclavamiento, y **la pluma no sigue a ese verde**. **Solo corre
fuera de servicio** (`N-82.bis`): modo `MENU`, `HORA` o `ALCANCE`, luz en `S_ROJO` **y el rojo del Esclavo
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
**El Poste 2 lo rechaza.**

## 11. Decisiones vigentes que gobiernan este documento

El mínimo por sentido · `DAR PASO` en Manual · los vetos del ámbar de emergencia (§4) · que aplicar tiempos no
arranque el ciclo · el suelo y el techo del Inteligente, **con condición sin cumplir** · la reanudación del Degradado,
que **no es manual** · y la retirada de la interfaz vieja (`D-30`), ~~recortada el 13/09 a que salga sólo el LCD y el mando se quede (`D-32`)~~
→ **reafirmada entera el 14/09**: el LCD y el mando ya salieron; faltan la lectura de p5/p8 y el menú (§12.2). ⬇️ Y ~~`A-1.bis` abierta~~ → **el 14/09 el responsable SÍ deroga la mitad de «nunca al revés» de la
regla de la pluma** (`D-33`). La pluma **sigue a la luz para SUBIR**; lo que la cámara puede hacer es **retener la
bajada**, nunca provocar una subida. La conducta entera, en **SPEC 5 pág. 1 §4**.
🔴 **DOS CONDUCTAS DE LA PLUMA Y LA LUZ SIN FILA QUE LAS RESPALDE**, y son de las que hieren a alguien: **(a)** que la
pluma **suba en fallo** (§2), elegido *«por el cliente y el PMT el 27/08/2026»* en una frase que vive sólo en el
fuente y en SPEC 5 §4; y **(b)** que el verde **cierre sin ámbar** (§3.1) → **(b) ya tiene fila: `D-45` (02/10),
amarillo de cierre, sin construir (§3.2)**. **Son decisiones viales: las decide él, no
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
   el daño que describía **era real y se componía con §3.1** —el verde cierra SIN ÁMBAR, así que el coche que
   entró legalmente seguía dentro cuando caía el rojo y la pluma bajaba sobre él—. 🔴 **§3.1 SIGUE ABIERTO:
   el verde sigue cerrando sin ámbar.** Lo que cambia es que ahora hay tres segundos y un veto de cámara entre
   ese coche y el brazo; lo que no cambia es que **el conductor no recibe aviso de que el verde se acaba**.
   🟡 *(02/10: el aviso está decidido, `D-45`; sin construir, §12.10.)*
   ⚠️ **Y el contador que medía esto cambió de significado con la obra:** ya no dice «habría actuado» —esa
   transición dejó de ocurrir el día que el veto existe— sino **cuántos vetos ACTUARON de verdad**.
2. 🔴 **LA RETIRADA DE LA INTERFAZ VIEJA (`D-30`, `D-44`): el LCD y el mando YA SALIERON; la lectura de p5/p8
   y el menú NO.** **Lo que el equipo HACE hoy** *(registro 1, medido el 15/09/2026 contra el fuente de las dos
   puntas)*: **el LCD salió el 13/09** y **el mando A/B/C/D salió entero el 14/09** —primero el corte de sus pulsos
   (`ccca294`), después el módulo, sus lectores y la señal que interceptaba las luces (`f57a401`)—. Ya no existen
   `mando_ambarLocal()` ni `semaforo_senalEnCurso()`. ⬇️ ~~retirarlo toca la barrera de salidas o abre vetos~~ →
   **medido: no abrió ninguno** (SPEC 5 §3). 🔴 **LO QUE FALTA — COLA DE TRABAJO DEL FIRMWARE** *(registro 2)*: **(a)**
   la **lectura de `BOTON1`/`BOTON2`** (`J16` p5/p8), que `D-30` retira con el resto y **sigue viva**:
   `botones_actualizar()` la antirrebota y `botonArriba()`/`botonAbajo()` entregan el flanco. Hoy sólo lo consumen
   `menu_loop()` y `modo_hora_loop()` del Poste 1 y **no mueve nada visible**, pero un llamador nuevo lo vuelve a hacer
   actuar sin tocar el cobre. **(b)** El **menú del Poste 1** (`Maestro/src/menu.cpp`): su navegación no llega a ningún
   modo, pero `menu_setup()` es la puerta del todo-rojo de las dos puntas que pide `SET_MODO:MENU`, y ese camino se
   conserva antes de retirarlo. 🟡 **DEBE (`D-44`, 02/10; sin construir): sale todo eso** —la lectura de p5/p8,
   `botonAceptar`/`botonCancelar` y sus ramas, la navegación del menú, el Modo Hora (§12.7) y `semaforo_toggle()`
   (§12.8)—; **`menu_setup()` se queda**. Cobre y regla de montaje, **SPEC 5 §3**; lo que ve el operador, **SPEC 4
   §3.bis**.
3. 🔴 **LOS HUECOS DE LUZ DEL DEGRADADO** *(el modo entero, SPEC 3)*. **(a)** El verde del Degradado
   (`DEG_VERDE_SEG`) **no pasa por el fichero de límites del ciclo**: es una constante propia del modo. ⬇️ ~~y queda
   muy por debajo del mínimo por sentido, y si ese mínimo alcanza al Degradado no está escrito en ninguna parte~~ →
   **REFUTADO, medido el 14/09: vale exactamente el mínimo por sentido** (subido de 30 s a 180 s el 13/09) **y el
   motivo SÍ está escrito**, en la cabecera del propio modo. **Lo que queda abierto es que nadie cruza los dos
   números:** el día que el mínimo se mueva, el verde del Degradado no le sigue, y ningún instrumento lo dirá.
   **(b) Las dos puntas ABREN su verde DISTINTO:** el Poste 1 fuerza el verde —**sin ámbar**—, el Poste 2 pasa por la
   transición con ámbar; no es cosmético, porque el Maestro apoya su cuenta de margen en el ámbar con que el Esclavo
   abre. ⚠️ **No confundir con la asimetría de la SALIDA a ámbar, que sí está razonada.** 🟡 **Con `D-45` esta (b)
   se cierra sola: las dos abren directo y las dos cierran por amarillo** (SPEC 2 §8 (e)).
   `grep -n "DEG_VERDE_SEG\|forzarVerde" 01_Firmware/{Maestro,Esclavo}/src/modo_degradado.cpp`
4. ⚠️ **Los dos plazos de la máquina de luces son LITERALES sin nombre**, dentro de la función que la avanza:
   el ámbar de transición y el medio periodo del parpadeo de fallo. No se pueden citar, ni vigilar por
   símbolo, ni releer desde un pack. `grep -n "ahora - tCambio >=" 01_Firmware/Maestro/src/semaforo.cpp`
   🟡 Con `D-45` el del ámbar sale y entra el del amarillo de cierre, ya con nombre (`AMARILLO_SEG`, §5); el del
   parpadeo sigue literal.
5. ⚠️ **El Poste 1 arranca A OSCURAS y el Poste 2 no.** El arranque del coordinador arranca el semáforo —que **apaga
   las tres luces**— y la primera luz no llega hasta el todo-rojo del ámbar de arranque o del menú (§4.1), tras
   arrancar reloj, respaldo, ~~mando y~~ Bluetooth, con una espera de dos segundos en medio y el cabezal apagado.
   ⚠️ **Esa espera ya NO es la bienvenida del LCD**: sobrevive porque su motivo escrito es el watchdog.
6. ⚠️ **El backstop de verde máximo está dimensionado contra un máximo que ya no existe.** Su comentario lo justifica
   *«por encima del máximo configurable (99 min)»* y el máximo real hoy es el del rango vial: protege en la dirección
   segura, pero mucho más laxo de lo que su razón pide — una razón caducada (`CLAUDE.md` §6).
7. ⚠️ **El Modo Hora es inalcanzable, y de ningún modo se sale por botón.** Los botones de aceptar y cancelar
   devuelven `false` **siempre** en las dos puntas desde que sus pines pasaron a ser cámaras: **todas** las
   ramas que los consultan son código muerto, sólo se sale por app ~~o por mando~~, y no hay `SET_MODO:HORA`.
   `grep -n "bool boton" 01_Firmware/{Maestro,Esclavo}/src/botones.cpp`
8. ⚠️ **El conmutador de luz** (`semaforo_toggle()`) **no tiene ningún llamador** y **contiene la única
   transición de fallo a verde** del firmware: hoy inerte, saca del ámbar intermitente sin pasar por rojo.
9. 🟡 ~~EL ARRANQUE ENTRA EN MENÚ Y DEBE ENTRAR EN ÁMBAR (`D-40`, `N-169`)~~ → **construido en el Poste 1**
   (§4.1), **sin banco ni tarjeta**. El Poste 2 no cambia por diseño (§4.1). **Riesgo abierto, decidido por `D-47`
   (02/10) y sin construir (`N-174`, §4.1):** un poste en rojo fijo por hora perdida en Degradado (`D-38`,
   SPEC 3 §5) que sufre un corte ya no reanuda y arranca en ámbar mientras el otro sigue alternando por reloj: **verde
   contra ámbar** hasta que llegue alguien.
10. 🔴 **EL AMARILLO DE CIERRE (`D-45`, `N-174`): DECIDIDO EL 02/10, SIN CONSTRUIR** *(registro 2, cola del
   firmware; el comportamiento, §3.2 y SPEC 2 §2.2, §4, §7, §8 (e); la pluma, SPEC 8 §1)*. Censo de 02/10 sobre
   `0518bfe`, por símbolo:
   - **Semáforo, las dos puntas** (`semaforo.cpp/.h`): la transición a verde y su literal de ámbar en la función que
     avanza la máquina (`semaforo_iniciarTransicionAVerde()`, `semaforo_actualizar()`) salen; el cierre es nuevo
     (propuesta: `semaforo_iniciarTransicionARojo()`, el nombre que el `grep` de §3.1 busca); `semaforo_forzarRojo()`
     y `semaforo_forzarVerde()` sobre un cierre en curso (§3.2 (3)(4)); `escribirPines()` (la condición
     `luzPideArriba` gana el amarillo de cierre); `terminarTestLeds()` (su rama de ámbar reanuda la transición a
     verde); `semaforo_toggle()`, sin llamador.
   - **Coordinador del Poste 1**: `coordinador_pedirCambio()` (rama `QV_MASTER`, `// Directo a rojo`, y la de
     `QV_NINGUNO`), los casos `C_INICIAL_ESPERA_ESTATICO` y `C_ESPERA_ESTATICO_TRAS_ESCLAVO` (abren por la
     transición), `C_MASTER_A_ROJO` (ya espera el rojo estable), `C_ESPERANDO_ACK_RED` y `TIMEOUT_ACK_MS`,
     `coordinador_forzarMenu()`, `coordinador_forzarRojoTotal()` e `coordinador_iniciarModo()` (el instante de inicio
     del rojo), el bloque de suelta por margen con `puedeSostenerVerde()` y su `static_assert`,
     `coordinador_estadoEsclavo()` (`ESC:`). Y `modo_ambar_setup()` (§3.2 (6)).
   - **Bucle del Poste 2** (`Esclavo/src/main.cpp`): las ramas de `CMD_GO_RED` (`// Directo a rojo`, `ackRojoEnviado`)
     y de `CMD_GO_GREEN` (su idempotencia mira el ámbar), `SUELTA_VERDE_MS`, `MAX_VERDE_BACKSTOP_MS` y
     `tInicioVerdeEsclavo`.
   - **Degradado**: `ciclo_degradado.h` entero (`FaseDegradado`, `ciclo_degradado_fase()`,
     `ciclo_degradado_restante()`), y en los dos `modo_degradado.cpp` la luz por fase, el salto de hora
     (`SALTO_SIN_ROJO_MAX_S`), `irAAmbar()`, `irARojoSinHora()`, la salida, y `aplicarLuz()` e `iniciarSalida()` del
     Poste 2.
   - **La pluma**: `PLUMA_RETARDO_BAJADA_MS` y su `static_assert` contra `DESPEJE_SEG_MIN` **no cambian** (SPEC 8 §1).
   - **Instrumentos que hoy CELEBRAN lo que `D-45` deroga** (`CLAUDE.md` §9: se reparten, invierten, conservan o
     borran uno por uno; el simulador está congelado y no gana packs): `Validacion_Automatico/arnes_automatico.cpp`
     (bloque A, *«Rojo->Verde pasa SIEMPRE por AMARILLO»*, y el control negativo del despeje a cero) ·
     `dos_puntas/orquestador.cpp` (F4 exige cero pasos de verde a ámbar en el Poste 2, *«la Resolucion da
     verde->rojo directo»*; cuenta la apertura del Poste 1 desde el ámbar) · `dos_puntas/orquestador_degradado.cpp`
     y `adaptador_esclavo.cpp` · `Validacion_Ciclo/arnes_ciclo.cpp` (de verde se pasa a despeje) · los modelos
     `banco/modelos/esclavo.py` y `costura.py` (leen el literal del ámbar previo al verde) · los packs
     `costura_12_margen_deriva` (el ámbar que abre el verde del Poste 2 como margen), `maestro_09_test_leds`,
     `barrera_02_dos_puntas`, `barrera_04_arnes_dos_puntas`, `camara_03_vigilante`, `costura_06_reanudacion`,
     `esclavo_03_par_config`, `esclavo_06_no_abre_paso`, `app_04_valores_de_status` · y en la app, la frase de la
     pluma con la luz en ÁMBAR (`App_Semaforo/app.js` y su copia de `www/`; SPEC 4 §6).
   - **Comentarios del fuente que llevan la premisa derogada** (`CLAUDE.md` §11.7): el de `coordinador_pedirCambio()`
     (ámbar reservado al paso de rojo a verde), el del Poste 2 *«contra la Resolución (verde→rojo directo)»* y el de
     `Maestro/src/modo_degradado.cpp` sobre los 4 s de ámbar con que abre el Poste 2.
11. 🔴 **SALEN DOS ÓRDENES (`D-46`, 02/10): DECIDIDO, SIN CONSTRUIR** *(registro 2)*. `SOLICITAR_PASO` del Poste 2 y
   `SET_MODO:DEGRADADO` del Poste 1 siguen hoy en los dos `procesarComando()` (SPEC 4 §3.1-§3.2); la app ya no las
   ofrece (`D-36`, `D-37`): el paso se da con Manual y DAR PASO (§8) y el Degradado con el testigo (SPEC 2 §7.bis).
   **Se quedan** `J14`/`PB0` (reservada al fin de carrera), el andamio `SFTY-20` y los pines de peatón y zumbador.

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
| **Menú en rojo fijo** (`SFTY-12`) · **el test de lámparas** (§10) | 🔴 **NADIE las dos.** El forzado de menú y el arranque del test **no aparecen en ningún arnés**; el menú sólo lo compila la fila 14, y `maestro_09_test_leds` lee texto |
| ~~**Los destellos INTERCEPTAN** la escritura de pines (§10)~~ | ~~fila 17~~ → **sin sujeto desde el 14/09**: la interceptación salió con el mando (§10). No cuenta abajo |
| **El backstop de verde máximo** (§9) · **el rango duro de tiempos** (`D-5`) · **no reconfigurar en marcha** (`D-11`, §5) | ✅ el backstop, **fila 18**; el rango y la reconfiguración, **fila 17**, y el rango también la 18 |
| **`DAR PASO` en Manual** (`D-7`, §8) | 🟡 **PARTIDA:** el acuse de `MANUAL:CAMBIAR_TURNO` sí (**fila 18**), pero **el Modo Manual del Maestro no lo compila ningún arnés** — sólo PlatformIO |
| **Los vetos del ámbar del Poste 2** (`D-8`, §4) | ✅ **fila 18**, que compila el bucle y el Bluetooth del Esclavo REALES. La cuenta, SPEC 2 §2.3 |
| **El suelo y el techo del Inteligente** (`D-19`, §7) | ✅ **fila 17** (el factor de techo y las tres entradas de presencia) |
| **El amarillo de cierre** (`D-45`, §3.2) | 🔴 **NADIE: sin construir.** Peor: las filas 17 y 18 hoy EXIGEN lo contrario (§12.10) |

**Cuenta: ~~13 barreras — 8 ejecutadas~~ ~~12 barreras — 7 ejecutadas, 2 sin nadie, 3 partidas~~ 13 barreras — 7
ejecutadas, 3 sin nadie, 3 partidas** *(15/09: sale la interceptación; 02/10: entra el amarillo de cierre, que
es rojo por no construido)*. Los otros dos rojos **no son casillas sueltas**: el menú y
el test de lámparas son **los dos caminos a los pines de luz que ningún arnés recorre**, y el segundo es justo el que
enciende VERDE sin mirar nada. Y de las tres partidas la más cara es el enclavamiento: **el del ESCLAVO —la punta que
obedece— no lo ejecuta nadie.**
