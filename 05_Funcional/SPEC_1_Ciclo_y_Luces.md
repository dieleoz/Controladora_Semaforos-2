# SPEC 1 — El ciclo y las luces de UN poste

**Qué es esto.** Lo que un poste hace con sus tres luces y su pluma: los modos que existen HOY, los
tiempos, las transiciones, la barrera de salidas y el estado seguro. Escrito contra el fuente.
**Qué NO entra.** La coordinación entre los dos postes (SPEC 2) · la hora y el Degradado como acuerdo
de reloj (SPEC 3) · la app (SPEC 4) · el cobre y los pines físicos (SPEC 5).
**De dónde sale.** `DECISIONES.md` (filas vigentes) → `01_Firmware/{Maestro,Esclavo}/{src,include}` →
`OPTIMIZACIONES.md` para las `SFTY-x`. Donde el código y un manual difieren, manda el código.

> 🔴 **NINGUNA CIFRA VIVE AQUÍ** (`CLAUDE.md` §14). Un `.md` no deriva de un `#define`: se nombra la
> constante y dónde vive, y el valor se saca del fuente con el `grep` que acompaña a cada bloque.

## 1. Las salidas que este documento gobierna

| salida | símbolo | dónde se declara |
|---|---|---|
| Rojo, Ámbar, Verde de la cara 1 | `ROJO1` · `AMARILLO1` · `VERDE1` | `{Maestro,Esclavo}/include/pines.h` |
| Rojo, Ámbar, Verde de la cara 2 | `ROJO2` · `AMARILLO2` · `VERDE2` | idem |
| La pluma (talanquera) | `MOTOR_TALANQUERA`, con `TALANQUERA_ABRIR` / `TALANQUERA_CERRAR` | idem |

**Las dos caras de un poste van SIEMPRE juntas.** `escribirPines()` escribe el mismo valor en la cara 1
y en la 2 con la misma orden; no hay ningún camino que las separe. Un poste tiene **un** color, no dos.

⚠️ **`ROJO_PEATON` y `VERDE_PEATON` están declarados en `pines.h` y el firmware NO los conduce**: no
hay `pinMode` ni `digitalWrite` sobre ellos en ninguna de las dos puntas. El firmware mueve **seis**
pines de luz más la pluma, no ocho. Es `CLAUDE.md` §2 (`N-96`) medido otra vez.
`grep -rn "ROJO_PEATON\|VERDE_PEATON" 01_Firmware/{Maestro,Esclavo}/src`

## 2. La barrera de salidas — SFTY-2 y SFTY-28

> **Sólo `semaforo.cpp` escribe pines de luz, y todo pasa por su `escribirPines()` estático.**

Es la propiedad más importante de este documento y la única que ningún modo puede rodear.

1. **`aplicarSalidas()` es la puerta única de la lógica.** Aplica el **enclavamiento SFTY-2** —nunca
   verde y rojo a la vez; **el rojo siempre gana**— y sólo después llama a `escribirPines()`.
2. **`escribirPines()` es la única función que toca un pin de luz** en las dos puntas. Lo censan
   `barrera_01_pines_de_luz` (los ocho pines, peatonales incluidos) y `barrera_02_dos_puntas`.
3. **SFTY-28: la pluma sale por la MISMA puerta que las lámparas.** El `digitalWrite(MOTOR_TALANQUERA,
   …)` vive **dentro** de `escribirPines()`, sobre el mismo `verde` **ya enclavado** por SFTY-2. Si un
   modo pudiera moverla por su cuenta habría pluma arriba con luz en rojo, que es **peor** que no tener
   barrera: el conductor confía en ella. Lo vigila `barrera_03_talanquera`.
4. **Dos excepciones, escritas dentro de la propia condición:** **no** sigue al verde de un test de
   lámparas (`testLedsActivo`, `N-82`) —una prueba de taller no abre la vía—; y **sí sube en `S_FALLO`**
   por decisión del cliente y el PMT, no del firmware. Equipo **vivo** que sabe que falló, abre; equipo
   **muerto**, el pin cae a reposo y la pluma **baja**. Las dos son ciertas y se leen juntas.
5. **La bandera `plumaAbierta` se asigna DENTRO del paréntesis de esa condición**, no al lado: el pin y
   lo que se publica (`semaforo_plumaArriba()`, campo `PLUMA:` del `$STATUS`) no pueden discrepar.

| luz | pluma |
|---|---|
| Rojo | ABAJO |
| Ámbar de transición | ABAJO |
| Todo-rojo de despeje | ABAJO |
| Destellos / ámbar rápido del mando | ABAJO |
| Verde de ciclo | ARRIBA |
| Verde de test de lámparas | **ABAJO** |
| `S_FALLO` (ámbar intermitente) | **ARRIBA** |
| Equipo sin energía | ABAJO (nivel de reposo del pin) |

⚠️ **Lo que la pluma NO tiene: realimentación.** No hay final de carrera (`D-27` (2) deja `J14` sin
cablear). El firmware sabe *«ordené abrir»*, nunca *«está abierta»*, y no finge lo contrario.

## 3. Los estados de la luz

`enum EstadoSemaforo { S_ROJO, S_VERDE, S_AMARILLO, S_FALLO }` — `{Maestro,Esclavo}/include/semaforo.h`.
Lo lee todo el firmware por `semaforo_estado()`; `semaforo_estable()` es cierto en `S_ROJO`, `S_VERDE`
y `S_FALLO`, y falso mientras corre la transición.

| estado | lo que se ve | quién lo pone |
|---|---|---|
| `S_ROJO` | rojo fijo | `semaforo_forzarRojo()` · `semaforo_apagarTodo()` (que apaga las tres) |
| `S_AMARILLO` | ámbar fijo, **sólo de camino al verde** | `semaforo_iniciarTransicionAVerde()` |
| `S_VERDE` | verde fijo | `semaforo_forzarVerde()` · el vencimiento de la transición |
| `S_FALLO` | **ámbar intermitente** | `semaforo_iniciarFallo()` |

**Las transiciones son ASIMÉTRICAS a propósito** (`OPT-6`, Manual de Señalización de Colombia: no existe
el rojo+ámbar europeo). **Rojo → Verde pasa por ámbar**: `semaforo_iniciarTransicionAVerde()` enciende
ámbar y anota el instante, y `semaforo_actualizar()` salta a verde al cumplirse el plazo. **Verde → Rojo
es directo, sin aviso** (`semaforo_forzarRojo()`). **`S_FALLO` parpadea** invirtiendo el ámbar cada medio
periodo. Los dos plazos son literales sin constante con nombre — ver Huecos.

**`semaforo_actualizar()` se llama SIN CONDICIÓN en cada vuelta del `loop()` de las dos puntas.** No
depende de que un modo se acuerde: un cabezal que dependiera de eso podía quedarse a oscuras
indefinidamente, y pasó.

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

**`MODO_AMBAR` es una salida de emergencia y por eso no tiene condiciones**: funciona desde cualquier
modo en marcha. `DEG_AMBAR`, en cambio, es un estado *interno* de la máquina del Degradado. Comparten
la luz y las dos líneas de motivo, nada más (`modo_ambar.h`).

**Poste 2 (Esclavo) NO tiene `ModoSistema`.** Es subordinado: su luz la ordena el Poste 1 salvo en tres
casos locales, que son los de esta spec — **ámbar de emergencia con cerrojo**
(`bluetooth_ambarEmergencia()`, `CMD:AMBAR_EMERGENCIA`), **Modo Degradado** (`degradado_gobiernaLuz()`) y
**ámbar por orfandad** (SFTY-6, abajo).

🔴 **`D-8` — el ámbar de emergencia conserva sus DOS VETOS**, y en esta punta son la pareja
`if (!mando_ambarLocal() && !bluetooth_ambarEmergencia())` que guarda el `CMD_GO_RED` **y** el
`CMD_GO_GREEN` de `Esclavo/src/main.cpp`. La asimetría es deliberada y es el arreglo entero: **se guarda
lo que ABRE paso, no lo que lo PARA**. Se intentó quitar el cerrojo dos veces y el banco lo tumbó las
dos; la causa del bloqueo era otra.

## 5. Los tiempos, y de dónde salen

**Los límites del ciclo viven en UN solo sitio: `Maestro/include/limites_ciclo.h`.**
`VERDE_MIN_MIN`/`VERDE_MIN_MAX`, `ROJO_MIN_MIN`/`ROJO_MIN_MAX`, `DESPEJE_SEG_MIN`/`DESPEJE_SEG_MAX`.
Verde y rojo se configuran en **minutos**; el despeje, en **segundos**.
`grep -n "MIN\|MAX" 01_Firmware/Maestro/include/limites_ciclo.h`

- 🔴 **`D-5`: el mínimo por sentido lo fija el responsable** y es un **límite DURO del firmware**, no
  de la interfaz. `modoAutomatico_fijarTiempos()` rechaza fuera de rango con
  `$ERR,CMD:SET_TIEMPOS,DESC:RANGO`; una app vieja o una trama a mano no lo pueden saltar. El porqué
  vial está entero en la cabecera de `limites_ciclo.h`, y **no se copia aquí**.
- **Los valores de arranque salen de esos mismos mínimos**, no de literales. **Sobreviven al corte**
  (`respaldo_guardarTiemposCiclo()` / `recuperarTiemposGuardados()`), y el rango **se revalida aunque el
  checksum apruebe**: un dato íntegro no es un dato válido.
- **El dueño de los tres números es `modo_automatico.cpp`**, y el resto del firmware los **lee** por
  `modoAutomatico_tiemposCiclo()`. No se copian: una segunda copia es `N-137` otra vez.
- 🔴 **`D-11`: fijar los tiempos NO arranca el ciclo.** `modoAutomatico_fijarTiempos()` valida, guarda
  y contesta; no entra en el modo y no programa ningún verde. Y **no se cambian con el ciclo en marcha**
  (`modoAutomatico_enMarcha()` lo veta): bajarlos a mitad de fase acortaría la fase EN CURSO, incluido
  un todo-rojo ya empezado.

**El despeje** (todo-rojo entre sentidos) lo guarda el coordinador en `tiempoDespejeMs`, que
`coordinador_configurar()` fija desde el modo. Es el único de los tres que es seguridad vial pura: es lo
que garantiza que el tramo quedó VACÍO antes de dar verde al otro lado.

## 6. Modo Automático — el ciclo por tiempo

`modoAutomatico_setup()` recupera el respaldo, configura el coordinador y llama a
`coordinador_iniciarModo()`, que **empieza SIEMPRE por todo-rojo y su despeje**. Una sola puerta de
entrada: no hay asistente ni arranque alternativo. En cada vuelta, con el coordinador en reposo
(`coordinador_listoParaContar()`), el modo mide lo transcurrido contra la duración de la fase —el rojo
configurado si la luz local está en `S_ROJO`, el verde configurado si no— y al vencer llama a
`coordinador_pedirCambio()`.

**La cuenta atrás** (`modoAutomatico_segundosRestantesFase()`) se publica **por PISO, nunca por
redondeo**: el operario nunca ve un número mayor que lo que queda. Fuera del modo devuelve
`SIN_CUENTA_ATRAS`: un número congelado es peor que un `--`, porque parece que sigue contando.

## 7. Modo Inteligente — el Automático con suelo y techo

🔴 **`D-19`, y la propiedad que lo hace seguro va primero:** con las cámaras muertas **este modo se
comporta EXACTAMENTE como el Automático**. Si nunca hay detección, la condición de mantener no se cumple
jamás y la fase termina en el suelo. Degrada al comportamiento conocido, no a uno raro.

- **SUELO = el tiempo CONFIGURADO de la fase en curso**, leído con `modoAutomatico_tiemposCiclo()` y
  **congelado al empezar la fase** (releerlo dejaría que una configuración nueva acortase la fase viva).
- **Por debajo del suelo no cambia nada.** Ni una cámara, ni una demanda a mano, ni las dos: **una
  cámara no puede ACORTAR una fase, sólo alargarla.** Es `D-5` dentro de este modo.
- **Cumplido el suelo se cambia igual que el Automático**, salvo el único caso que las cámaras aportan:
  hay tráfico en mi lado y **enfrente no espera nadie** → MANTIENE.
- **TECHO = el suelo multiplicado por `TECHO_POR_SUELO`** (`modo_inteligente.cpp`), **saturado** al
  máximo del rango vial (`VERDE_MIN_MAX` / `ROJO_MIN_MAX`). No se escribe como constante propia: se
  deriva, o sería un número más que sincronizar.
- 🔴 **`D-19` está APROBADA CON UNA CONDICIÓN QUE NO ESTÁ CUMPLIDA**: el techo vale *si* un funcional
  revisa el manual de uso y lo da por claro. Hasta esa firma, `TECHO_POR_SUELO` viaja **POR VALIDAR**.

**Las tres entradas de presencia entran por un OR de tres booleanos** —`camara_leerPin(CAM_DEMANDA_PIN)`,
`camara_presenciaJ16()`, `demanda_hayLocal()`— **leídas UNA vez por vuelta y compartidas por la decisión
y la pantalla**. Dos presencias simultáneas valen lo mismo que una. *(Su cobre, en la SPEC 5; el reparto
de significados de las cámaras, en `D-13`.)*

## 8. Modo Manual — DAR PASO

🔴 **`D-7`: en Manual, `DAR PASO` alterna rojo/verde como el automático**, disparado por el operario, y
**el todo-rojo de despeje se queda**. Termina en rojo+verde, no en rojo+ámbar.

`modoManual_setup()` llama a `coordinador_forzarRojoTotal()` — **no** a `coordinador_iniciarModo()`, que
dejaría un verde ya programado. Eso producía las dos mitades del defecto reportado en banco: `DAR PASO`
rechazado mientras corría el plazo, y el cruce cambiando solo al vencer. **En Manual no se programa
ningún cambio: lo pide el operario o no pasa.**

El cambio se pide por `MANUAL:CAMBIAR_TURNO`, y **el acuse depende de lo que la llamada devolvió**
(`CLAUDE.md` §2): `MODO_SIN_CICLO_SALGA_PRIMERO` si el modo no mueve el coordinador,
`EN_TRANSICION_REINTENTE` si hay un despeje o una transición en curso, `OK` sólo si se aceptó. **No se
fuerza: partir un despeje por la mitad es justo lo que no se puede hacer.** Y **este modo ya NO
configura tiempos**: conserva el despeje que haya; para reconfigurar está `SET_TIEMPOS`, con su guarda.

## 9. El estado seguro

**SFTY-6 — el ámbar intermitente por silencio.** El umbral vive **una sola vez**, en
`{Maestro,Esclavo}/include/protocolo.h` como `SFTY6_SILENCIO_MS`, y las dos puntas lo usan:
- el Poste 1 cae a `C_FALLO` y llama a `semaforo_iniciarFallo()` cuando lleva ese silencio sin recibir;
- el Poste 2 hace lo mismo por **orfandad**, y **el Degradado lo veta** (`degradado_gobiernaLuz()`).

**El ámbar ORDENADO usa la misma puerta.** `CMD_GO_AMBAR` llama a `semaforo_iniciarFallo()`, la misma
función por la que se entra por orfandad: no es una luz nueva, es el mismo destino alcanzado por orden en
vez de por temporizador. **La orfandad se queda como red** si esa orden se pierde.

**El backstop de verde máximo del Poste 2** (`MAX_VERDE_BACKSTOP_MS`, `Esclavo/src/main.cpp`) fuerza rojo
si el verde se alarga más de la cuenta. **Mide la LUZ, no la última orden recibida** —las órdenes
repetidas ya no reinician la cuenta—, porque el verde del Degradado no lo ordena nadie por radio.

**SFTY-1 — watchdog.** `IWatchdog.begin(...)` en las dos puntas, refrescado en cada `loop()`, y armado
**antes** de tocar el reloj: un cristal que no arranca se vuelve así un reinicio visible en vez de un
cuelgue mudo con las luces apagadas. Por eso **nada de lo que ocupa las luces bloquea**: los destellos y
el test avanzan por `millis()`, nunca con `delay()`.

**Arranque.** El Poste 2 pone **las luces primero, siempre**: `semaforo_setup()` y `semaforo_forzarRojo()`
son lo primero de su `setup()`. El Poste 1 no — ver Huecos.

## 10. Lo que ocupa las luces sin ser un modo

**Señales del mando (SFTY-21).** `semaforo_destellosRojos(n)` y `semaforo_ambarRapido(ms)`
**INTERCEPTAN las escrituras a los pines, no la lógica**: mientras `senalActiva`, `aplicarSalidas()`
guarda en `ultR`/`ultA`/`ultV` **ya saneados por SFTY-2** y no escribe; al terminar se vuelca la última
decisión real. Se descartó ignorar las llamadas porque dejaba a quien esperase un `S_VERDE`
esperándolo para siempre, con las luces congeladas y sin timeout que lo rescatase. **La confirmación va
en DESTELLOS ROJOS contables, nunca en verde**: el rojo nunca significa «pase», así que si el operario
cuenta mal el peor caso sigue siendo seguro. El **rechazo** es ámbar rápido, a otro ritmo que el de fallo.

**Test de lámparas** (`semaforo_iniciarTestLeds()`, `CMD:TEST_LEDS`): tres fases de igual duración
—rojo, ámbar, verde— que **entran por `aplicarSalidas()`**, así que pasan por el enclavamiento; **la
pluma no sigue a ese verde**. Con una señal del mando en curso **el test espera y se rearma**: no se
abandona (el `$ACK` ya salió) ni corre por debajo (gastaría sus segundos sin encender nada y un técnico
leería tres lámparas fundidas). **El Poste 2 lo rechaza** con `NO_EN_SERVICIO_USE_EL_MAESTRO`, y por eso
`semaforo_iniciarTestLeds()` **no tiene llamador allí**.

## 11. Decisiones vigentes que gobiernan este documento

`D-5` (mínimo por sentido) · `D-7` (DAR PASO en Manual) · `D-8` (los dos vetos del ámbar de emergencia)
· `D-11` (aplicar tiempos no arranca el ciclo) · `D-19` (suelo y techo del Inteligente, **con condición
sin cumplir**) · `D-30` (LCD y mando salen del firmware, **sin construir**) · y `A-1.bis` **abierta**: si
se deroga SFTY-28 para que una cámara pueda vetar la bajada de la pluma. **Mientras `A-1.bis` esté
abierta, ninguna cámara interviene en `escribirPines()`** — es un hueco, no un comportamiento.

## 12. HUECOS MEDIDOS

Medido el 12/09/2026 sobre el árbol de trabajo. Cada uno trae con qué se reproduce.

1. 🔴 **`D-30` está VIGENTE y NO ESTÁ CONSTRUIDA.** Decide retirar `lcd.cpp`, `menu.cpp` y `mando.cpp`
   de las dos puntas y la lectura de flancos de `BOTON1`/`BOTON2`. Los seis ficheros siguen ahí y no hay
   **ni una** marca `D-30` en el fuente —`grep -rho "D-[0-9]*" 01_Firmware/{Maestro,Esclavo}/{src,include} | sort | uniq -c`
   la da en **cero** mientras otras decisiones tienen decenas—. Siguen vivos `A.A.A`, `B.B.B` y
   `A.B.A.B`, que **cambian de modo**; y `A.A.A` arranca el ciclo, o sea **abre paso**.
2. 🔴 **En Modo Degradado el verde de cada sentido lo fija `DEG_VERDE_SEG`, en `modo_degradado.cpp`, y
   NO pasa por `limites_ciclo.h`.** Está en **segundos** y `VERDE_MIN_MIN` en **minutos**; conviértalos y
   compárelos: el verde del Degradado queda **muy por debajo** del mínimo vial de `D-5`. El fuente lo
   dice —*«el ciclo degradado NO hereda el verde configurado en Modo Automático»*— y da su razón (el tope
   de un byte de `CMD_CONFIG` y el margen de deriva). **Es la forma exacta de `N-137`**: un modo que
   configura el ciclo por su cuenta, por debajo del mínimo, sin guarda que lo vea. **Si el alcance de
   `D-5` cubre o no al Degradado no está escrito en ninguna parte, y no lo decide el firmware.**
   `grep -n "DEG_VERDE_SEG\|DEG_DESPEJE_SEG" 01_Firmware/Maestro/src/modo_degradado.cpp`
3. 🔴 **Las dos puntas abren el verde del Degradado de forma DISTINTA.** El Poste 1 usa
   `semaforo_forzarVerde()` —salto directo, **sin ámbar**— y el Poste 2 usa
   `semaforo_iniciarTransicionAVerde()` —con ámbar—. No es cosmético: el propio `modo_degradado.cpp` del
   Maestro apoya su cuenta de margen en que *«los 4 s de ámbar con que el Esclavo abre su verde protegen
   SÓLO en un sentido»*. Además contradice la secuencia que el Manual 1 declara normativa (rojo → ámbar
   → verde). `grep -n "semaforo_forzarVerde\|semaforo_iniciarTransicionAVerde" 01_Firmware/{Maestro,Esclavo}/src/modo_degradado.cpp`
4. ⚠️ **Los dos plazos de la máquina de luces son LITERALES sin nombre**, dentro de
   `semaforo_actualizar()`: el ámbar de transición y el medio periodo del parpadeo de `S_FALLO`. No se
   pueden citar, ni vigilar por símbolo, ni releer desde un pack. Todo lo demás de este documento tiene
   constante con nombre. `grep -n "ahora - tCambio >=" 01_Firmware/Maestro/src/semaforo.cpp`
5. ⚠️ **El Poste 1 arranca A OSCURAS y el Poste 2 no.** En el Maestro, `coordinador_setup()` llama a
   `semaforo_setup()` —que **apaga las tres luces**— y el primer rojo no llega hasta `menu_setup()` →
   `coordinador_forzarMenu()`, después de la bienvenida y de arrancar reloj, respaldo, mando y
   Bluetooth, con un `delay(2000)` en medio y el cabezal apagado. *«Un semáforo apagado no avisa de
   nada»* es del propio `main.cpp`.
6. ⚠️ **`MAX_VERDE_BACKSTOP_MS` está dimensionado contra un máximo que ya no existe.** Su comentario lo
   justifica *«por encima del máximo configurable (99 min)»*, y el máximo real hoy es `VERDE_MIN_MAX`,
   mucho menor. Protege en la dirección segura, pero es varias veces más laxo de lo que su propia razón
   pide: una excepción cuya razón caducó (`CLAUDE.md` §6).
7. ⚠️ **`MODO_HORA` es inalcanzable, y de ningún modo se sale por botón.** `botonAceptar()` y
   `botonCancelar()` devuelven `false` **siempre** en las dos puntas desde que sus pines pasaron a ser
   cámaras, así que **todas** las ramas `if (botonCancelar())` de los `modo_*_loop()` son código muerto:
   sólo se sale por app o por mando. Y no existe `SET_MODO:HORA` por Bluetooth.
   `grep -n "bool boton" 01_Firmware/{Maestro,Esclavo}/src/botones.cpp`
8. ⚠️ **`semaforo_toggle()` no tiene ningún llamador** en ninguna de las dos puntas, y **contiene la
   única transición `S_FALLO → verde`** del firmware. Hoy es inerte; el día que alguien la llame, saca
   del ámbar intermitente sin pasar por rojo. `grep -rn "semaforo_toggle" 01_Firmware/{Maestro,Esclavo}`
9. ⚠️ **El enclavamiento SFTY-2 sólo lo EJECUTA un arnés, y sólo del Maestro**
   (`Validacion_Automatico/arnes_automatico.cpp`). Los cuatro packs que la tabla de `OPTIMIZACIONES.md`
   pone en su fila **leen** el C++; ninguno lo corre. Lo dice esa misma tabla en su aviso de lectura.
