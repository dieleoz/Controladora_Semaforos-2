# SPEC 1 — El ciclo y las luces de UN poste

**Qué es esto.** Lo que un poste hace con sus tres luces y su pluma: los modos que existen HOY, los tiempos, las
transiciones, la barrera de salidas y el estado seguro, escrito contra el fuente. **Qué NO entra.** La
coordinación entre los dos postes (SPEC 2) · la hora y el Degradado como acuerdo de reloj (SPEC 3) · la app
(SPEC 4) · el cobre y los pines físicos (SPEC 5). **De dónde sale.** `DECISIONES.md` (filas vigentes) →
`01_Firmware/{Maestro,Esclavo}/{src,include}` → `OPTIMIZACIONES.md` para las `SFTY-x`; donde el código y un
manual difieren, manda el código. 🔴 **NINGUNA CIFRA VIVE AQUÍ** (`CLAUDE.md` §14): se nombra la constante y
dónde vive, y el valor se saca del fuente con el `grep` que acompaña a cada bloque.

## 1. Las salidas que este documento gobierna

| salida | símbolo | dónde se declara |
|---|---|---|
| Rojo, Ámbar, Verde de la cara 1 | `ROJO1` · `AMARILLO1` · `VERDE1` | `{Maestro,Esclavo}/include/pines.h` |
| Rojo, Ámbar, Verde de la cara 2 | `ROJO2` · `AMARILLO2` · `VERDE2` | idem |
| La pluma (talanquera) | `MOTOR_TALANQUERA`, con `TALANQUERA_ABRIR` / `TALANQUERA_CERRAR` | idem |

**Las dos caras de un poste van SIEMPRE juntas.** `escribirPines()` escribe el mismo valor en la cara 1 y
en la 2 con la misma orden; no hay camino que las separe. Un poste tiene **un** color, no dos.
⚠️ **`ROJO_PEATON` y `VERDE_PEATON` están declarados en `pines.h` y el firmware NO los conduce** (ni `pinMode` ni
`digitalWrite`, en ninguna punta): mueve **seis** pines de luz más la pluma, no ocho — `CLAUDE.md` §2 (`N-96`).
`grep -rn "ROJO_PEATON\|VERDE_PEATON" 01_Firmware/{Maestro,Esclavo}/src`

## 2. La barrera de salidas — SFTY-2 y SFTY-28

> **Sólo `semaforo.cpp` escribe pines de luz, y todo pasa por su `escribirPines()` estático.** Es la
> propiedad más importante de este documento y la única que ningún modo puede rodear.

🔴 **EL PRINCIPIO QUE GOBIERNA LA PLUMA, y del que las excepciones de abajo son consecuencia** (responsable,
13/09): *«un semáforo funciona sin pluma; si falla algo, ARRIBA»*. **La pluma REFUERZA la señal; no la
sustituye** —quien regula es la lámpara—. De ahí las dos mitades que cuesta leer juntas: equipo **vivo que
sabe que falló** ABRE —una barrera cerrada que nadie gobierna encierra el corredor de obra—; equipo **muerto**
CIERRA, porque el pin cae a reposo. Y **ninguna avería de la pluma detiene el ciclo**: nadie se entera.

1. **`aplicarSalidas()` es la puerta única de la lógica.** Aplica el **enclavamiento SFTY-2** —nunca verde
   y rojo a la vez; **el rojo siempre gana**— y sólo después llama a `escribirPines()`.
2. **`escribirPines()` es la única función que toca un pin de luz** en las dos puntas; lo censan
   `barrera_01_pines_de_luz` (los ocho pines, peatonales incluidos) y `barrera_02_dos_puntas`.
3. **SFTY-28: la pluma sale por la MISMA puerta que las lámparas.** El `digitalWrite(MOTOR_TALANQUERA,…)`
   vive **dentro** de `escribirPines()`, sobre el mismo `verde` **ya enclavado** por SFTY-2: si un modo
   pudiera moverla por su cuenta habría pluma arriba con luz en rojo, **peor** que no tener barrera —el
   conductor confía en ella—. Lo vigila `barrera_03_talanquera`.
4. **Dos excepciones, dentro de la propia condición:** **no** sigue al verde de un test de lámparas (una
   prueba de taller no abre la vía; `testLedsActivo`, `N-82`); y **sí sube en `S_FALLO`** (el principio).
5. **La bandera `plumaAbierta` se asigna DENTRO del paréntesis de esa condición**, no al lado: el pin y lo que
   se lee de él no pueden discrepar. Y **no sólo se publica** (`semaforo_plumaArriba()`, campo `PLUMA:`): **el
   vigilante de cámaras la usa de reloj** —sólo acumula silencio con la pluma ARRIBA, el *«con el ciclo
   corriendo»* de `D-13`—, y una segunda copia habría que mantenerla a mano: por eso no existe.
6. 🔴 **El pin tiene UN SEGUNDO ESCRITOR, y sólo uno: el cierre de arranque.** `semaforo_setup()` hace su
   `digitalWrite(MOTOR_TALANQUERA, TALANQUERA_CERRAR)` antes de que haya lógica —dejar el pin como quedó
   sería vía abierta sin regular durante todo el arranque— y **repite la bandera a mano**, por ser el
   único camino que no pasa por `escribirPines()`. **Dos escrituras por punta y ninguna más**, las dos en
   `semaforo.cpp`, y la segunda sólo CIERRA. `grep -n MOTOR_TALANQUERA 01_Firmware/*/src/semaforo.cpp`

| luz | pluma |
|---|---|
| Rojo · ámbar de transición · todo-rojo de despeje · destellos y ámbar rápido del mando | ABAJO |
| Verde de ciclo | ARRIBA |
| Verde de test de lámparas | **ABAJO** |
| `S_FALLO` (ámbar intermitente) | **ARRIBA** |
| Equipo sin energía | ABAJO (nivel de reposo del pin) |

⚠️ **Lo que la pluma NO tiene: realimentación.** No hay final de carrera (`D-27` (2) deja `J14` sin
cablear): sabe *«ordené abrir»*, nunca *«está abierta»*, y no finge lo contrario.

## 3. Los estados de la luz

`enum EstadoSemaforo { S_ROJO, S_VERDE, S_AMARILLO, S_FALLO }` — `{Maestro,Esclavo}/include/semaforo.h`. Lo lee
todo el firmware por `semaforo_estado()`; `semaforo_estable()` es cierto en `S_ROJO`, `S_VERDE` y `S_FALLO`, y
falso mientras corre la transición.

| estado | lo que se ve | quién lo pone |
|---|---|---|
| `S_ROJO` | rojo fijo | `semaforo_forzarRojo()` · `semaforo_apagarTodo()` (que apaga las tres) |
| `S_AMARILLO` | ámbar fijo, **sólo de camino al verde** | `semaforo_iniciarTransicionAVerde()` |
| `S_VERDE` | verde fijo | `semaforo_forzarVerde()` · el vencimiento de la transición |
| `S_FALLO` | **ámbar intermitente** | `semaforo_iniciarFallo()` |

**Las transiciones son ASIMÉTRICAS, y la asimetría es el corazón de este documento.** **Rojo → Verde pasa
por ámbar**: `semaforo_iniciarTransicionAVerde()` enciende ámbar y anota el instante, y
`semaforo_actualizar()` salta a verde al cumplirse el plazo. **`S_FALLO` parpadea** invirtiendo el ámbar
cada medio periodo. Los dos plazos son literales sin constante con nombre — ver Huecos. Y
**`semaforo_actualizar()` se llama SIN CONDICIÓN en cada vuelta del `loop()` de las dos puntas**: un cabezal
que dependiera de que un modo se acordase puede quedarse a oscuras indefinidamente, y pasó.

### 3.1 🔴 NO HAY ÁMBAR AL TERMINAR EL VERDE — el semáforo salta de VERDE a ROJO

**`semaforo_forzarRojo()` pone `S_ROJO` y aplica rojo en la MISMA llamada, sin estado intermedio.** No es
un camino entre varios: **es el único que existe.** **Ninguna de las dos puntas tiene función de transición
a rojo** —`grep -rn "TransicionARojo" 01_Firmware` da **cero**— y `S_AMARILLO` lo escribe sólo
`semaforo_iniciarTransicionAVerde()`, o sea **únicamente en el sentido contrario**. Las llamadas a
`semaforo_forzarRojo()` se cuentan por **decenas** (coordinador, Degradado, mando, radio), varias con el
comentario literal `// Directo a rojo`, y **todas hacen lo mismo**.
`grep -rn "semaforo_forzarRojo()" 01_Firmware/{Maestro,Esclavo}/src`
⚠️ **NO se confunda con lo que sí está documentado, que es LO CONTRARIO.** `OPT-6` retiró la transición
**europea rojo+ámbar → verde** citando el Manual de Señalización de Colombia: eso habla de cómo se **ABRE** el
verde. **Cerrarlo sin ámbar es otra cosa, y sobre ella no hay `D-x`, ni `OPT-x`, ni manual.** El único rastro
es un comentario de `N-162` (`Esclavo/src/main.cpp`) que dice que devolver a ámbar un verde ya encendido sería
*«contra la Resolución (verde→rojo directo)»*: ⚠️ **una AFIRMACIÓN NORMATIVA SIN VERIFICAR** (`CLAUDE.md` §6)
que vive en un comentario y no en una spec.
🔴 **LA CONSECUENCIA, que es lo que importa: el conductor NO RECIBE NINGÚN AVISO DE QUE EL VERDE SE ACABA.** El
paso se le retira en seco, y **lo que hay en su lugar no es un aviso, es un margen:** `SFTY-4`, el **todo-rojo de
despeje** (`tiempoDespejeMs`, §5), que retiene a la otra punta mientras el tramo se vacía. El diseño **no avisa:
cierra y espera.** Quien ya entró queda dentro con el rojo puesto — y de ahí cuelga el hueco de la pluma (§12.1).

## 4. Los modos que existen HOY

**Poste 1 (Maestro).** `enum ModoSistema` en `Maestro/include/modos.h`; el estado vive fuera de la
pantalla, en `modos.cpp`, y se lee y escribe con `modoActual_get()` / `modoActual_set()`.

| modo | qué hace con las luces | cómo se llega hoy |
|---|---|---|
| `MENU` | **rojo fijo** (`coordinador_forzarMenu()`, SFTY-12); ámbar intermitente si no hay enlace | arranque, `SET_MODO:MENU` |
| `MODO_MANUAL` | todo-rojo al entrar y **ningún cambio programado**: la fase acaba cuando alguien pulsa | `SET_MODO:MANUAL`; también al cancelarse un ámbar del Poste 2 |
| `MODO_AUTOMATICO` | cicla por tiempo | `SET_MODO:AUTO`; secuencia `A.A.A` del mando |
| `MODO_INTELIGENTE` | cicla por tiempo **con suelo y techo**, y las cámaras sólo pueden ALARGAR | `SET_MODO:INTELIGENTE` |
| `MODO_ALCANCE` | **no arranca ciclos**: mantiene lo que haya (rojo fijo con enlace) | `SET_MODO:ALCANCE` |
| `MODO_HORA` | no toca las luces | 🔴 **INALCANZABLE** — ver Huecos |
| `MODO_DEGRADADO` | todo-rojo de entrada y luego verde/rojo por reloj | `SET_MODO:DEGRADADO`; `A.B.A.B`; reanudación tras corte (`D-29`) |
| `MODO_AMBAR` | **ámbar intermitente pedido a propósito** (`semaforo_iniciarFallo()`) | `SET_MODO:AMBAR`; `B.B.B`; aviso del Poste 2; salida del Degradado |

**`MODO_AMBAR` es una salida de emergencia y por eso no tiene condiciones**: funciona desde cualquier modo en
marcha. `DEG_AMBAR`, en cambio, es un estado *interno* del Degradado: comparten la luz y las dos líneas de motivo
(`modo_ambar.h`). ⚠️ **Y la reanudación tras un corte NO la pide nadie: la decide la máquina** (`D-29`, SPEC 2 §7).
**Poste 2 (Esclavo) NO tiene `ModoSistema`.** Su luz la ordena el Poste 1 salvo en tres casos locales —
**ámbar de emergencia con cerrojo** (`bluetooth_ambarEmergencia()`), **Modo Degradado**
(`degradado_gobiernaLuz()`) y **ámbar por orfandad** (SFTY-6, §9).

> 🔴 **`D-8` — ESE ÁMBAR DE EMERGENCIA CONSERVA SUS DOS VETOS, Y EN EL FUENTE SON TRES `if`. LAS DOS CUENTAS
> SON CIERTAS Y CUENTAN COSAS DISTINTAS, y por ahí se cuentan mal.**
> **DOS por SUJETO** —el mando (`mando_ambarLocal()`) y la app (`bluetooth_ambarEmergencia()`), que es como
> lo escribe `D-8`— y **TRES por RAMA**: la condición `!mando_ambarLocal() && !bluetooth_ambarEmergencia()`
> guarda el `CMD_GO_RED`, el `CMD_GO_GREEN` **y la recuperación tras fallo** de `Esclavo/src/main.cpp`, que
> **no es un `else` de la primera**. Es `CLAUDE.md` §2: **una regla que ENUMERA sujetos comprueba que cada
> sujeto EXISTE, y dónde se ejerce.** El detalle y el recuento, **SPEC 2 §2.3**; `SPEC 5` §3 ya decía tres.
> **La asimetría es el arreglo entero: se guarda lo que ABRE paso, no lo que lo PARA.**

## 5. Los tiempos, y de dónde salen

**Los límites del ciclo viven en UN solo sitio: `Maestro/include/limites_ciclo.h`** —`VERDE_MIN_MIN`/`MAX`,
`ROJO_MIN_MIN`/`MAX`, `DESPEJE_SEG_MIN`/`MAX`—. Verde y rojo en **minutos**; el despeje, en **segundos**.
`grep -n "MIN\|MAX" 01_Firmware/Maestro/include/limites_ciclo.h`
- 🔴 **`D-5`: el mínimo por sentido lo fija el responsable** y es un **límite DURO del firmware**, no de la
  interfaz: `modoAutomatico_fijarTiempos()` rechaza con `$ERR,CMD:SET_TIEMPOS,DESC:RANGO`, y no lo salta ni una
  app vieja ni una trama a mano. El porqué vial, en la cabecera de `limites_ciclo.h`.
- **Los valores de arranque salen de esos mismos mínimos**, no de literales, **sobreviven al corte**
  (`respaldo_guardarTiemposCiclo()`) y el rango **se revalida aunque el checksum apruebe**: un dato íntegro no
  es válido. **El dueño de los tres números es `modo_automatico.cpp`** y el resto los **lee**: copiarlos es `N-137`.
- 🔴 **`D-11`: fijar los tiempos NO arranca el ciclo.** Valida, guarda y contesta; no entra en el modo ni
  programa un verde. Y **no se cambian con el ciclo en marcha** (`modoAutomatico_enMarcha()` lo veta):
  acortaría la fase EN CURSO, incluido un todo-rojo ya empezado.
- **El despeje** (todo-rojo entre sentidos) lo guarda el coordinador en `tiempoDespejeMs`, que
  `coordinador_configurar()` fija desde el modo. **Es el único de los tres que es seguridad vial pura** —garantiza
  que el tramo quedó VACÍO antes de abrir el otro lado— y, **a falta de ámbar de cierre (§3.1), es TODA la
  protección que hay para quien se quedó dentro**.

## 6. Modo Automático — el ciclo por tiempo

`modoAutomatico_setup()` recupera el respaldo, configura el coordinador y llama a `coordinador_iniciarModo()`,
que **empieza SIEMPRE por todo-rojo y su despeje** — una sola puerta, sin arranque alternativo. En cada vuelta,
con el coordinador en reposo (`coordinador_listoParaContar()`), mide lo transcurrido contra la duración de la
fase —el rojo configurado si la luz local está en `S_ROJO`, el verde si no— y al vencer llama a
`coordinador_pedirCambio()`. **La cuenta atrás** (`modoAutomatico_segundosRestantesFase()`) se publica **por
PISO, nunca por redondeo**, y fuera del modo devuelve `SIN_CUENTA_ATRAS`: un número congelado es peor que un
`--`, porque parece que sigue contando.

## 7. Modo Inteligente — el Automático con suelo y techo

🔴 **`D-19`, y la propiedad que lo hace seguro va primero:** con las cámaras muertas **se comporta EXACTAMENTE
como el Automático** —sin detección la condición de mantener no se cumple jamás y la fase termina en el suelo—:
degrada al comportamiento conocido, no a uno raro.
- **SUELO = el tiempo CONFIGURADO de la fase en curso** (`modoAutomatico_tiemposCiclo()`), **congelado al
  empezar la fase**: releerlo dejaría que una configuración nueva acortase la fase viva. **Por debajo del
  suelo no cambia nada** —ni una cámara, ni una demanda a mano, ni las dos—: **una cámara no puede ACORTAR
  una fase, sólo alargarla.** Es `D-5` dentro de este modo.
- **Cumplido el suelo se cambia igual que el Automático**, salvo el único caso que las cámaras aportan:
  tráfico en mi lado y **enfrente nadie esperando** → MANTIENE.
- **TECHO = el suelo por `TECHO_POR_SUELO`** (`modo_inteligente.cpp`), **saturado** al máximo del rango vial
  (`VERDE_MIN_MAX`/`ROJO_MIN_MAX`); se deriva, no se escribe. 🔴 **`D-19` está APROBADA CON UNA CONDICIÓN SIN
  CUMPLIR** —vale *si* un funcional revisa el manual—, así que viaja **POR VALIDAR**.
- **Las tres entradas de presencia son un OR de tres booleanos** —`camara_leerPin(CAM_DEMANDA_PIN)`,
  `camara_presenciaJ16()`, `demanda_hayLocal()`—, **leídas UNA vez por vuelta** *(cobre, SPEC 5; `D-13`)*.

## 8. Modo Manual — DAR PASO

🔴 **`D-7`: en Manual, `DAR PASO` alterna rojo/verde como el automático**, disparado por el operario, y **el
todo-rojo de despeje se queda**. Termina en rojo+verde, no en rojo+ámbar. `modoManual_setup()` llama a
`coordinador_forzarRojoTotal()` — **no** a `coordinador_iniciarModo()`, que dejaría un verde ya programado
(las dos mitades del defecto de banco: `DAR PASO` rechazado mientras corría el plazo, y el cruce cambiando
solo al vencer). **Aquí no se programa ningún cambio: lo pide el operario o no pasa.** El cambio se pide por
`MANUAL:CAMBIAR_TURNO` y **el acuse depende de lo que la llamada devolvió** (`CLAUDE.md` §2):
`MODO_SIN_CICLO_SALGA_PRIMERO`, `EN_TRANSICION_REINTENTE` si hay un despeje o una transición en curso, y `OK`
sólo si se aceptó. **No se fuerza: partir un despeje por la mitad es justo lo que no se puede hacer.** Y **este
modo ya NO configura tiempos**: conserva el despeje que haya.

## 9. El estado seguro

**SFTY-6 — el ámbar intermitente por silencio.** El umbral vive **una sola vez**, en
`{Maestro,Esclavo}/include/protocolo.h` como `SFTY6_SILENCIO_MS`: el Poste 1 cae a `C_FALLO` y llama a
`semaforo_iniciarFallo()` tras ese silencio; el Poste 2 hace lo mismo por **orfandad**, y **el Degradado lo veta**
(`degradado_gobiernaLuz()`). **El ámbar ORDENADO usa la misma puerta** —`CMD_GO_AMBAR` llama a
`semaforo_iniciarFallo()`—, y **la orfandad se queda como red** si esa orden se pierde.
**El backstop de verde máximo del Poste 2** (`MAX_VERDE_BACKSTOP_MS`, `Esclavo/src/main.cpp`) fuerza rojo si
el verde se alarga más de la cuenta. **Mide la LUZ, no la última orden recibida** —las órdenes repetidas ya
no reinician la cuenta—, porque el verde del Degradado no lo ordena nadie por radio.
**SFTY-1 — watchdog:** `IWatchdog.begin(...)` en las dos puntas, refrescado en cada `loop()` y armado **antes**
de tocar el reloj, para que un cristal que no arranca sea un reinicio visible y no un cuelgue mudo a oscuras.
Por eso **nada de lo que ocupa las luces bloquea**: destellos y test avanzan por `millis()`. **Arranque:** el
Poste 2 pone **las luces primero, siempre**; el Poste 1 no — ver Huecos.

## 10. Lo que ocupa las luces sin ser un modo

**Señales del mando (SFTY-21).** `semaforo_destellosRojos(n)` y `semaforo_ambarRapido(ms)` **INTERCEPTAN las
escrituras a los pines, no la lógica**: mientras `senalActiva`, `aplicarSalidas()` guarda en `ultR`/`ultA`/
`ultV` **ya saneados por SFTY-2** y no escribe; al terminar se vuelca la última decisión real (ignorar las
llamadas dejaría esperando para siempre a quien aguardase un `S_VERDE`). **La confirmación va en DESTELLOS
ROJOS contables, nunca en verde** —el rojo nunca significa «pase», así que si el operario cuenta mal el peor
caso sigue siendo seguro—; el **rechazo** es
ámbar rápido, a otro ritmo que el de fallo. **Test de lámparas** (`semaforo_iniciarTestLeds()`,
`CMD:TEST_LEDS`): tres fases —rojo, ámbar, verde— que **entran por `aplicarSalidas()`** y por tanto por el
enclavamiento, y **la pluma no sigue a ese verde**. Con una señal del mando en curso **el test espera y se
rearma**: ni se abandona (el `$ACK` ya salió) ni corre por debajo. **El Poste 2 lo rechaza.**

## 11. Decisiones vigentes que gobiernan este documento

`D-5` (mínimo por sentido) · `D-7` (DAR PASO en Manual) · `D-8` (los vetos del ámbar de emergencia, §4) ·
`D-11` (aplicar tiempos no arranca el ciclo) · `D-19` (suelo y techo del Inteligente, **con condición sin
cumplir**) · `D-29` (la reanudación del Degradado, que **no es manual**) · `D-30` **recortada por `D-32` (1):
sale SÓLO el LCD y el mando SE QUEDA** · y ~~`A-1.bis` abierta~~ → **`D-33` (14/09): SÍ se deroga SFTY-28,
y sólo en su mitad de «nunca al revés».** La pluma **sigue a la luz para SUBIR**; lo que la cámara puede hacer
es **retener la bajada**, nunca provocar una subida. La conducta entera, en **SPEC 5 pág. 1 §4**.
🔴 **TRES CONDUCTAS DE LA PLUMA Y LA LUZ SIN FILA QUE LAS RESPALDE**, y son de las que hieren a alguien:
**(a)** que la pluma **suba en `S_FALLO`** (§2), elegido *«por el cliente y el PMT el 27/08/2026»* en una frase
que vive sólo en el fuente y en SPEC 5 §4; y **(b)** que el verde **cierre sin ámbar** (§3.1). **Son decisiones
viales: las decide él, no el firmware ni esta spec.** ⬇️ ~~**(c)** que la pluma baje sin retardo~~ → **ya tiene
fila: `D-33`, y está construida.** Bajan DOS, no tres.

## 12. HUECOS MEDIDOS

Medido el 12/09/2026; **1, 2 y 6 remedidos el 13/09/2026 sobre `bfef121`**. Cada uno trae con qué reproducirlo.
1. 🟢 **CERRADO EL 14/09 — ~~LA PLUMA BAJA EN EL MISMO INSTANTE DEL ROJO~~.** Hoy **baja 3 segundos
   después del rojo**, y **no baja en absoluto mientras una cámara vea algo debajo**. La regla que lo pidió,
   del responsable el 13/09: *«sólo baja segundos DESPUÉS del rojo, porque suelen pasarse carros y hay que
   darle tiempo al conductor a pasar»*. Es **el cambio que sube la versión a `V9.1`**, y la conducta entera
   vive en **SPEC 5 pág. 1 §4** — aquí sólo el puntero. ⬇️ *lo que este hueco decía, y por qué se conserva:*
   el daño que describía **era real y se componía con §3.1** —el verde cierra SIN ÁMBAR, así que el coche que
   entró legalmente seguía dentro cuando caía el rojo y la pluma bajaba sobre él—. 🔴 **§3.1 SIGUE ABIERTO:
   el verde sigue cerrando sin ámbar.** Lo que cambia es que ahora hay tres segundos y un veto de cámara entre
   ese coche y el brazo; lo que no cambia es que **el conductor no recibe aviso de que el verde se acaba**.
   ⚠️ **Y el contador que medía esto cambió de significado con la obra:** ya no dice «habría actuado» —esa
   transición dejó de ocurrir el día que el veto existe— sino **cuántos vetos ACTUARON de verdad**.
2. 🔴 **`D-30` está VIGENTE, RECORTADA por `D-32` (1) y sin ancla en el fuente**: esta spec ya no puede decir que
   el mando salga, porque es el **único escritor de `senalActiva`**, o sea SFTY-2 (§10). Su cobre, **SPEC 5 §3**.
3. 🔴 **DOS HUECOS DE LUZ DEL DEGRADADO** *(el modo entero, SPEC 3)*. **(a)** `DEG_VERDE_SEG` **no pasa por
   `limites_ciclo.h`** y queda **muy por debajo** del mínimo de `D-5`, y **si `D-5` alcanza al Degradado no está
   escrito en ninguna parte** (`N-137` otra vez).
   **(b) Las dos puntas ABREN su verde DISTINTO:** Poste 1 con `semaforo_forzarVerde()` —**sin ámbar**—, Poste 2
   con `semaforo_iniciarTransicionAVerde()`; no es cosmético, porque el Maestro apoya su cuenta de margen en el
   ámbar con que el Esclavo abre. ⚠️ **No confundir con la asimetría de la SALIDA a ámbar, que sí está razonada.**
   `grep -n "DEG_VERDE_SEG\|forzarVerde" 01_Firmware/{Maestro,Esclavo}/src/modo_degradado.cpp`
4. ⚠️ **Los dos plazos de la máquina de luces son LITERALES sin nombre**, dentro de `semaforo_actualizar()`: el
   ámbar de transición y el medio periodo del parpadeo de `S_FALLO`. No se pueden citar, ni vigilar por símbolo,
   ni releer desde un pack. `grep -n "ahora - tCambio >=" 01_Firmware/Maestro/src/semaforo.cpp`
5. ⚠️ **El Poste 1 arranca A OSCURAS y el Poste 2 no.** `coordinador_setup()` llama a `semaforo_setup()` —que
   **apaga las tres luces**— y el primer rojo no llega hasta `menu_setup()` → `coordinador_forzarMenu()`, tras
   arrancar reloj, respaldo, mando y Bluetooth, con un `delay(2000)` en medio y el cabezal apagado. ⚠️ **Ese
   `delay(2000)` ya NO es la bienvenida del LCD**: sobrevive porque su motivo escrito es el watchdog.
6. ⚠️ **`MAX_VERDE_BACKSTOP_MS` está dimensionado contra un máximo que ya no existe.** Su comentario lo justifica
   *«por encima del máximo configurable (99 min)»* y el máximo real hoy es `VERDE_MIN_MAX`: protege en la
   dirección segura, pero mucho más laxo de lo que su razón pide — una razón caducada (`CLAUDE.md` §6).
7. ⚠️ **`MODO_HORA` es inalcanzable, y de ningún modo se sale por botón.** `botonAceptar()` y `botonCancelar()`
   devuelven `false` **siempre** en las dos puntas desde que sus pines pasaron a ser cámaras: **todas** las ramas
   `if (botonCancelar())` son código muerto, sólo se sale por app o por mando, y no hay `SET_MODO:HORA`.
   `grep -n "bool boton" 01_Firmware/{Maestro,Esclavo}/src/botones.cpp`
8. ⚠️ **`semaforo_toggle()` no tiene ningún llamador** y **contiene la única transición `S_FALLO → verde`** del
   firmware: hoy inerte, saca del ámbar intermitente sin pasar por rojo.

## 13. QUIÉN EJERCE CADA BARRERA DE ESTE DOCUMENTO

> **Una spec puede describir barreras que ningún compilador ejerce, con UNA condición: que cada barrera lleve
> escrito QUIÉN la ejerce.** El criterio es `CLAUDE.md` §6.3 — **¿algún arnés COMPILA ese `.cpp`?**; si sólo lo lee
> por texto no ve un defecto del TIEMPO, y es *vigilada por texto*, no *ejecutada*. Filas = las de la compuerta;
> reparto de `.cpp`, `ARQUITECTURA.map` §4-5. Medido sobre `ef3504c`.

| barrera | quién la EJERCE hoy |
|---|---|
| **SFTY-2** — el enclavamiento de `aplicarSalidas()` | 🟡 **PARTIDA. Sólo la fila 17** (`arnes_automatico.cpp`) **y sólo del Maestro.** Los cuatro packs de su fila en `OPTIMIZACIONES.md` LEEN el C++; el `semaforo.cpp` del Esclavo lo COMPILAN las filas 18, 19 y 20, pero ninguna afirma «nunca verde y rojo a la vez» |
| **SFTY-28** — la pluma dentro de `escribirPines()` | ✅ **filas 17, 18 y 19** (miran `MOTOR_TALANQUERA`; la 17 además `semaforo_plumaArriba()`) · `barrera_03_talanquera` por texto |
| **SFTY-4** el todo-rojo de despeje · **SFTY-6** ámbar por silencio y orfandad (§9) | ✅ el despeje, **filas 17 y 18**, que corren el ciclo sobre el `coordinador.cpp` real; `SFTY-6`, **fila 18** (detalle en SPEC 2 §9) |
| **SFTY-1** — watchdog (§9) | 🟡 **PARTIDA: el reinicio no lo ejerce nadie.** `M:main.cpp` sólo lo cruza PlatformIO; la fila 18 cuenta las recargas del `loop()` del Esclavo contra un `IWatchdog` **simulado** |
| **SFTY-12** — menú en rojo fijo · **el test de lámparas** (§10) | 🔴 **NADIE las dos.** `coordinador_forzarMenu()` y `semaforo_iniciarTestLeds()` **no aparecen en ningún arnés**; `menu.cpp` sólo lo compila la fila 14, y `maestro_09_test_leds` lee texto |
| **SFTY-21** — los destellos INTERCEPTAN (`senalActiva`) | ✅ **fila 17** (`semaforo_destellosRojos()`), y `mando.cpp` es su único escritor vivo — es el motivo de `D-32` (1) |
| **`MAX_VERDE_BACKSTOP_MS`** (§9) · **`D-5`** rango duro · **`D-11`** no reconfigurar en marcha (§5) | ✅ el backstop, **fila 18**; `D-5` y `D-11`, **fila 17** (`modoAutomatico_fijarTiempos()`, `modoAutomatico_enMarcha()`), y `D-5` también la 18 |
| **`D-7`** — `DAR PASO` en Manual (§8) | 🟡 **PARTIDA:** el acuse de `MANUAL:CAMBIAR_TURNO` sí (**fila 18**, `pedirCambioVerificado()`), pero **`M:modo_manual.cpp` no lo compila ningún arnés** — sólo PlatformIO |
| **`D-8`** — los vetos del ámbar del Poste 2 (§4) | ✅ **fila 18**, que compila `Esclavo/src/{main,bluetooth}.cpp` REALES. La cuenta, SPEC 2 §2.3 |
| **`D-19`** — suelo y techo del Inteligente (§7) | ✅ **fila 17** (`TECHO_POR_SUELO` y las tres entradas de presencia) |

**Cuenta: 13 barreras — 8 ejecutadas, 2 sin nadie, 3 partidas.** Los dos rojos **no son casillas sueltas**:
`menu.cpp` y el test de lámparas son **los dos caminos a los pines de luz que ningún arnés recorre**, y el
segundo es justo el que enciende VERDE sin mirar nada. Y de las tres partidas la más cara es `SFTY-2`: **el
enclavamiento del ESCLAVO —la punta que obedece— no lo ejecuta nadie.**
