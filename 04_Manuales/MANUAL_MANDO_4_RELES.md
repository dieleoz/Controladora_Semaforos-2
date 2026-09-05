# 🎛️ Manual Técnico del Mando de 4 Relés

**Ubicación:** `04_Manuales/MANUAL_MANDO_4_RELES.md`
**Referencia de diseño:** SFTY-21 (`OPTIMIZACIONES.md`)
**Implementación:** `01_Firmware/Maestro/src/mando.cpp`, `include/mando.h`, `src/botones.cpp`
**Fecha:** 1 de agosto de 2026
~~**Última revisión:** 31 de agosto de 2026 — **el mando NO se retira.**~~
**Última revisión:** **5 de septiembre de 2026** — 🛑 **el mando NO EXISTE COMO HARDWARE. El
CÓDIGO se queda. Las dos cosas a la vez: ver la cabecera de estado.**

---

> # 🛑 ESTADO (05/09/2026) — NO HAY MANDO. EL EQUIPO SE OPERA SÓLO POR APP
>
> **Decisión `D-1` de `DECISIONES.md`**, confirmada por el responsable el 05/09: *«ya no tenemos
> mandos de A y B, sólo la app»*. **Este manual describe un equipo que no se compró y que no se
> va a comprar.** Léalo como registro de diseño, **nunca como instrucción de campo**.
>
> ## Las dos mitades de `D-1`, y hay que sostener las dos
>
> | | |
> |---|---|
> | 🛑 **El HARDWARE se fue** | Nunca se compró receptor de relés (lista de compras rev. 3, 28/08) y ya no se va a comprar. **No hay equipo en servicio que desmontar** |
> | ✅ **El CÓDIGO se queda, y el motivo está MEDIDO** | `mando_ambarLocal()` tiene **cinco llamadas vivas** —tres vetos en `Esclavo/src/main.cpp` y dos decisiones de `CANCELAR_AMBAR` en `Esclavo/src/bluetooth.cpp`— y su veto **es SFTY-21**. Retirar el armador deja esos `if` **siempre verdaderos**: el veto **no queda inerte, queda abierto**. Y el banco caería en **`ABORTADO`, no en rojo**: son **trece packs**, porque los dos modelos leen constantes de `mando.cpp` **en el import** |
>
> ## ⚠️ Y los pines SE SIGUEN LEYENDO. Esto es lo que hay que saber antes de tocar `J16`
>
> Que el pulsador se haya retirado **no deja el camino inerte**. `botones_actualizar()` lee
> `BOTON1` (`PB9`, `J16` p5) y `BOTON2` (`PB13`, p8) en cada vuelta, con `pinMode(…, INPUT)`
> pelado y **activo en ALTO** (N-118, corregido en `346ea5f`), y llama a
> `mando_registrarPulso(MANDO_A/B)` en cada flanco. **`mando.cpp` sigue reconociendo `A·A·A`,
> `B·B·B` y `A·B·A·B`.**
>
> 🔴 **O sea: lo que alguien cierre en `J16` p5 o p8 ENTRA AL FIRMWARE y puede disparar un
> cambio de modo.** La spec da hoy esos dos pines como **LIBRES Y SIN CABLEAR**
> (`05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md`, que **gana a este manual en
> todo lo que sea cobre**). Y `J16` p1 lleva **12 V crudos**: **taparlo es obligatorio en cada
> equipo que se monte** (`D-4`, N-120).
>
> ## 🔴 Lo que la retirada se llevó por delante y NO estaba escrito: §6 se quedó SIN SUJETO
>
> **La §6 de este manual describe una protección —inhibir las secuencias con el menú abierto—
> que hoy no protege de nada, y en el Esclavo NO PUEDE ser cierta jamás.** No es que esté de
> más: es que la guarda dejó de poder dar las dos respuestas. **Está medida y desarrollada en el
> aviso de la propia §6.** Si alguna vez vuelve un pulsador a `J16` p5/p8, esa barrera **no
> vuelve sola**.
>
> ## 🛑 Y `D-16`, que es la consecuencia de esto y va en el manual del operario
>
> **Sin teléfono no hay forma de operar el equipo.** Retirado el mando, la app es la **única**
> superficie de mando: ni ámbar, ni volver a automático, ni parar el cruce. **Es una propiedad
> declarada del sistema, no una avería.**
>
> En el **Esclavo** eso llegó a ser peor —esa punta se quedó sin ninguna vía de modo—, y es el
> hueco `A-11`. **Se está cerrando por Bluetooth el 05/09**: ya tiene `SET_MODO:DEGRADADO` como
> entrada, y la salida va por `AMBAR_EMERGENCIA` / `FORZAR_ROJO`. **La entrada tiene comando
> propio y la salida no** — el censo medido y el porqué de que eso importe están en el aviso de
> §9. **Manda `A-11` en `DECISIONES.md`, no este manual.**

---

> ## 📕 HISTÓRICO — la decisión del 31/08. Superada por `D-1`, se conserva y no se borra
>
> **Su razonamiento sigue siendo el que sostiene la mitad viva de `D-1`** (conservar el código),
> y por eso se lee entero. Lo que caducó es *«va cableado»*, no *«el veto se abre si lo
> borras»*.

> # ~~✅ DECISIÓN DEL 31/08/2026 — EL MANDO SE CONSERVA EN `A` Y `B`. SE RETIRAN `C` Y `D`~~
>
> **Este manual sale de la lista de documentos falsos.** Lo que sigue vigente se marca; lo superado
> se tacha con su motivo.
>
> | Canal | Pin | Conector | 31/08 |
> |---|---|---|---|
> | **`A`** | `PB9` | `J16` p5 | ✅ **SE CONSERVA** — Arriba / mando A |
> | **`B`** | `PB13` | `J16` p8 | ✅ **SE CONSERVA** — Abajo / mando B |
> | ~~`C`~~ | `PB14` | `J16` p10 | 🛑 **SE RETIRA** → `CAM_C_PIN`, entrada de cámara |
> | ~~`D`~~ | `PB15` | `J16` p12 | 🛑 **SE RETIRA** → `CAM_D_PIN`, entrada de cámara |
>
> **Las tres secuencias siguen funcionando enteras**, porque **ninguna usaba `C` ni `D`**:
> `A·A·A`, `B·B·B` y `A·B·A·B`. **MEDIDO** en `Maestro/src/mando.cpp:11-21` y en
> `Maestro/include/pines.h:91-125` / `Esclavo/include/pines.h:91-125`.
>
> ## 🔑 Y la razón de conservarlo la escribió la §3 de este mismo manual
>
> **Decirlo importa**, porque el documento estuvo a punto de borrarse por obsoleto y **fue su §3 lo
> que sostuvo la decisión**: `B·B·B` es el único gesto que arma `ambarLocal`, la bandera de la que
> cuelgan **tres vetos** en `Esclavo/src/main.cpp` (SFTY-21). Retirar el mando entero **no dejaba
> esos tres `if` inertes: los dejaba siempre-verdaderos**, es decir, **abría** una barrera sin que
> nadie borrara una línea y sin que ningún instrumento se pusiera rojo.
>
> **Se conservan los DOS canales, no solo uno, precisamente por eso:** `A` sin `B` no arma
> `ambarLocal`, y `B` sin `A` deja al operario dentro del ámbar **sin forma de salir desde el piso**
> (`A·A·A` es la salida). Los dos canales son un par, no dos opciones.
>
> ## ✅ 31/08 — y ahora `ambarLocal` tiene además un SEGUNDO armador
>
> **MEDIDO** en `Esclavo/src/main.cpp:406`, `:416` y `:540`: los tres vetos ya no leen una sola
> bandera, sino dos —
> `if (!mando_ambarLocal() && !bluetooth_ambarEmergencia())`.
>
> El segundo es el latch de `CMD:AMBAR_EMERGENCIA` (`Esclavo/src/bluetooth.cpp:130`, `:171`, getter en
> `:268`). O sea: **el ámbar local del Esclavo se puede pedir hoy por dos caminos independientes** —el
> mando por `B·B·B` y la app por Bluetooth—, y **cualquiera de los dos veta las órdenes de radio**.
> El riesgo del punto 3 de la versión anterior de este aviso **está resuelto por construcción**, no
> por promesa.
>
> ## 🛑 Lo que NO cambia
>
> **Sigue sin haber receptor de relés comprado ni instalado en ninguna punta, y sin prueba de banco.**
> Conservar los canales `A` y `B` en el firmware **no pone un actuador en el poste**. Y en campo corre
> la **V8.4**: nada de esto ha pasado banco.
>
> ---
>
> ## 📕 HISTÓRICO — el aviso del 28/08, superado. Se conserva, no se borra
>
> > # ~~🛑 AVISO DEL 28/08/2026 — EL MANDO DE 4 RELÉS SE RETIRA~~
> >
> > ~~El 28/08/2026 se decidió en obra retirar la pantalla LCD, los cuatro pulsadores y el mando de
> > relés. Toda la operación pasa a la App por Bluetooth. Este manual describe un actuador que deja
> > de existir.~~
>
> **Motivo por el que se supera:** la retirada total del mando **no se ejecutó**. El 31/08 se decidió
> partir `J16` —dos posiciones al mando, dos a las cámaras—, y el argumento que lo decidió es el
> `ambarLocal` de arriba. Lo que sí se ejecutó de aquel aviso: **la pantalla LCD se retira** y
> **`C`/`D` dejan de ser pulsadores**.
>
> Lo que sigue del aviso viejo se conserva porque **su punto 3 sigue siendo la mejor explicación
> escrita de por qué el mando no se podía borrar sin más**, y es lo que hay que releer si alguien
> vuelve a proponer retirarlo.
>
> ## 1. Lo que se retira es código y papel, NO equipo en servicio
>
> **VERIFICADO leyendo la cabecera de este mismo manual (`:8-19`, justo debajo):** el receptor de
> relés figura como **❌ no instalado en las dos puntas**, y la prueba de banco como **❌ ninguna**.
> El propio documento lo dice sin rodeos: *"Nada de esto se ha ejercitado con un mando físico
> conectado"*.
>
> | | Maestro | Esclavo |
> |---|---|---|
> | Receptor de relés instalado | ❌ **nunca** | ❌ **nunca** |
> | Prueba de banco | ❌ **ninguna** | ❌ **ninguna** |
>
> **Consecuencia, y conviene decirla porque quita presión a la retirada:** ningún operario ha usado
> nunca este mando. Retirarlo **no cambia lo que hoy se puede hacer en campo** — no había receptor
> que accionar. Lo que se retira es `mando.cpp` en las dos puntas y las páginas que lo describen.
>
> **Lo que NO es inofensivo va en el punto 3.** Que el actuador no existiera no significa que su
> código fuera inerte.
>
> ## 2. Lo que sí desaparece de verdad: la salida de emergencia prometida
>
> La §8 de este manual (`:316-318`) vende `B·B·B` como *"la regla que impide que nadie quede atrapado
> con un semáforo en estado raro a 5 m de altura"*. **Esa salida nunca llegó a existir** (no hay
> receptor) **y ahora tampoco existirá.** El aviso está pegado a la frase, en su sitio, y no solo
> aquí arriba: quien lee la §8 puede no haber leído esta cabecera.
>
> ## 3. 🔴 HALLAZGO TÉCNICO — `mando_ambarLocal()` NO queda inerte al retirar el mando
>
> **Es lo único de este manual que hay que resolver antes de borrar nada.**
>
> ### MEDIDO — `grep` sobre el fuente, 28/08/2026
>
> `ambarLocal` es una bandera del Esclavo que se **arma en un solo sitio**:
>
> ```
>   Esclavo/src/mando.cpp:132    ambarLocal = true;    // dentro de ejecutar(ACC_AMBAR) = B.B.B
> ```
>
> No hay otro. Se apaga en `mando_setup()` (`:100`), en `ACC_OBEDECER` (`:116`) y en `ACC_DEGRADADO`
> (`:147`) — todos también del mando. **Y tiene tres consumidores en `Esclavo/src/main.cpp` que
> VETAN órdenes de radio:**
>
> | Línea *(28/08)* | Código *(28/08)* | Qué veta |
> |---|---|---|
> | `main.cpp:401` | `if (!mando_ambarLocal()) {` | Que un `CMD_GO_RED` del Maestro fuerce rojo y acuse recibo |
> | `main.cpp:408` | `if (!mando_ambarLocal()) {` | Que un `CMD_GO_GREEN` del Maestro **abra paso** |
> | `main.cpp:526` | `if (!mando_ambarLocal() && semaforo_estado() == S_FALLO && pkt.command == CMD_GO_RED)` | Que la recuperación tras `S_FALLO` revoque el ámbar del operario |
>
> > ⚠️ **Estas tres líneas se han movido.** Hoy son `:406`, `:416` y `:540`, y la condición lleva una
> > segunda bandera: `if (!mando_ambarLocal() && !bluetooth_ambarEmergencia())`. **Qué vetan no ha
> > cambiado.** Ver §3.1.
>
> *(Hay un cuarto uso, el sostenedor del ámbar en `mando.cpp:274`, que no es un veto.)*
>
> ### Qué pasa si el mando se va sin tocar estos tres `if`
>
> ```
>   Se retiran mando y pulsadores
>        -> ACC_AMBAR no se alcanza nunca
>        -> ambarLocal se queda en false PARA SIEMPRE  (mando_setup lo pone a false)
>        -> los tres if (!mando_ambarLocal()) son SIEMPRE VERDADEROS
>        -> EL VETO DESAPARECE
> ```
>
> **No es código que quede muerto: es una barrera de seguridad que se abre sola.** Hoy el firmware
> garantiza que *una orden de radio no puede sacar al Esclavo del ámbar local*. Retirado el mando,
> esa garantía no se elimina explícitamente — **se evapora**, porque su condición deja de poder ser
> cierta. Nadie borra una línea y nadie ve un `FALLA`.
>
> Es la forma inversa de la **prueba muerta** que `CLAUDE.md` §3.bis describe: allí una condición
> siempre cierta hacía que una prueba no midiera nada; aquí una condición siempre cierta hace que un
> **veto no vete nada**. El síntoma es el mismo —un `if` que ya no decide— y la señal de alarma
> también: *un `PASS` de algo que nadie ha visto fallar nunca*.
>
> ### ✅ 31/08 — CERRADO, y por la primera de las tres vías que este punto listaba
>
> **La decisión se tomó y fue *«el ámbar local gana un actuador nuevo por Bluetooth — y la bandera se
> conserva tal cual»*.** De hecho conserva las dos: `B·B·B` sigue armándola y
> `CMD:AMBAR_EMERGENCIA` arma la segunda, y los tres `if` leen ambas. **No se retiró en silencio**,
> que era la peor de las tres opciones.
>
> **Y las dos incógnitas que bloqueaban esta decisión están resueltas:** el chip del ESP32 está
> identificado —`ESP32-WROOM-32` clásico, con SPP— y el reloj deja de depender del cristal `Y2`,
> porque el `DS3231` con pila propia vive ahora en el módulo de expansión.
>
> ### ~~Lo que hay que hacer con ello — ESCRITO, no medido~~ *(superado, se conserva)*
>
> ~~**La decisión técnica NO está tomada** (falta confirmar el chip del ESP32 y qué pasa con el cristal
> `Y2`), así que aquí no se propone implementación.~~ Lo que sí queda fijado:
>
> - **`ambarLocal` no se borra junto con `mando.cpp`.** O el ámbar local gana un actuador nuevo por
>   Bluetooth —y la bandera se conserva tal cual—, o se retira **con** sus tres consumidores, de
>   forma explícita y en el mismo commit, dejando escrito que el veto ya no existe.
> - **Retirarla en silencio es la peor de las tres opciones**, porque deja los tres `if` en pie
>   pareciendo que protegen algo.
> - Esto necesita **un pack que lo vigile**. Ver el informe de sesión.
>
> ---

> ## ⚠️ ESTADO: FIRMWARE EN LAS DOS PUNTAS · **RECEPTOR FÍSICO SIN COMPRAR**
>
> | | Maestro | Esclavo |
> |---|---|---|
> | Firmware del mando | ✅ `Maestro/src/mando.cpp` | ✅ `Esclavo/src/mando.cpp` *(añadido el 01/08/2026, N-19)* |
> | Receptor de relés instalado | ❌ **no** | ❌ **no** |
> | Prueba de banco | ❌ **ninguna** | ❌ **ninguna** |
>
> **Nada de esto se ha ejercitado con un mando físico conectado.** Las restricciones de
> tiempo de §2 provienen de medidas de campo del 01/08/2026 sobre el relé; el comportamiento
> del conjunto firmware + receptor, no.
>
> **Por qué el firmware entra antes que el receptor:** el relé va en paralelo con los botones
> físicos, así que el firmware no distingue un dedo de un relé. Con las secuencias ya
> cargadas, el día que se instale el receptor es **solo conectarlo** — sin volver a flashear
> y sin subir a 5 m con el equipo en servicio. Al revés obligaría a una intervención más en
> el gabinete **por cada punta**.

---

## 1. Qué es y por qué existe

El operario maneja el equipo desde el suelo con un **mando inalámbrico de 4 relés**
(canales `A`, `B`, `C`, `D`) cuyos contactos están cableados **en paralelo con los cuatro
botones físicos** de la tarjeta:

| Canal del mando | Botón físico | Pin STM32 | `J16` | Función en el menú | 31/08 |
|---|---|---|---|---|---|
| **A** | Botón 1 | `PB9` | p5 | Arriba | ✅ se conserva |
| **B** | Botón 2 | `PB13` | p8 | Abajo | ✅ se conserva |
| ~~**C**~~ | ~~Botón 3~~ | `PB14` | p10 | ~~Aceptar / Ejecutar~~ | 🛑 **→ `CAM_C_PIN`** |
| ~~**D**~~ | ~~Botón 4~~ | `PB15` | p12 | ~~Cancelar / Menú~~ | 🛑 **→ `CAM_D_PIN`** |

**Los pines son los mismos en el Maestro y en el Esclavo**, así que el cableado del receptor es
idéntico en las dos puntas.

> ### 🛑 31/08 — `C` y `D` YA NO SON PULSADORES, Y EL MODO DEL PIN CAMBIA
>
> No es un cambio de nombre. **MEDIDO** en `pines.h:100-101` y `botones.cpp` de ambas puntas:
>
> | | antes (`BOTON3`/`BOTON4`) | ahora (`CAM_C_PIN`/`CAM_D_PIN`) |
> |---|---|---|
> | Modo del pin | `INPUT_PULLUP` | **`INPUT` pelado** |
> | Polaridad | activo en **BAJO** | **activo en ALTO** |
> | Quién lo lee | `botones.cpp` → menú | el camino de cámara |
>
> **La polaridad no es una preferencia, es una cuenta.** `R67` y `R68` son 10 kΩ **a masa** sobre
> `/Boton3` y `/Boton4`, y `J16` saca 3,3 V en p9 y p11 —las posiciones de al lado—: eso es un
> pull-**down** con la tensión a un pin de distancia, y el gesto previsto es cerrar el contacto seco
> de la cámara contra esos 3,3 V. Con `INPUT_PULLUP` el pull-up interno (~40 kΩ) contra ese 10 kΩ
> deja el pin en 3,3 × 10/50 = **0,66 V**, que el micro lee `LOW`: **demanda permanente sin cámara
> conectada, e invertida al cerrarla.**
>
> ⚠️ **NO SE CABLEA CÁMARA A `J16` hasta que se haga la medida `M3`** de
> `05_Funcional/17_Arquitectura...` §2.2 — y el orden es **firmware primero, cableado después**
> (`CLAUDE.md §9.bis`): un pin en `INPUT` no ejecuta nada, mientras que con el firmware viejo dentro
> `PB14` sigue siendo *Aceptar* activo en BAJO y **cualquier hilo enchufado en p10 lo pulsa**.
>
> ⚠️ **Y `J16` p1 lleva 12 V CRUDOS** —sin opto, sin serie, sin clamp— a nueve posiciones de p10 y
> once de p12. **Se tapa físicamente antes de enchufar nada.**

> **No hay entradas dedicadas.** Eléctricamente, el relé y el botón **son el mismo
> contacto**: el firmware no puede distinguir cuál de los dos se accionó, y no lo intenta.
> El antirrebote de `botones.cpp` sirve igual para ambos.

**El problema que resuelve:** la pantalla está a **5 m de altura, dentro del gabinete**. El
operario acciona desde el piso **sin poder verla**.

> ### 🚨 Un menú es inservible a ciegas
>
> No se sabe dónde está el cursor ni si la pulsación entró. Y el fallo no es "el operario se
> equivoca de opción": es que **la pulsación puede llegar cuando el sistema está en un sitio
> distinto del que el operario cree**, y a ciegas eso siempre es posible.
>
> Por eso el mando **no navega el menú**: reconoce **secuencias** y contesta **con las
> luces**.

---

## 2. Restricciones medidas en campo (01/08/2026)

Son medidas, no preferencias. Cualquier diseño que las ignore es papel mojado.

| Medida | Valor | Consecuencia de diseño |
|---|---|---|
| Tipo de señal | **Un pulso por flanco**, no se sostiene | **La pulsación larga NO EXISTE.** Sostener el botón 10 s da un solo pulso |
| Retardo por pulsación | **~2 s** en conmutar | Una ventana de 3 s es inviable; hacen falta **12–18 s** para 3–4 pulsos |
| Repetición automática | **No la hay** | Cada pulso exige una pulsación |

**Consecuencia directa sobre la pantalla:** `AJUSTAR HORA` se edita **dígito a dígito** y no
como valor completo. Con un solo botón de subir y sin repetición, poner los minutos como
valor completo costaría **hasta 59 pulsaciones de ~2 s cada una** — casi dos minutos para un
solo campo. Por dígito son 9 como máximo.

---

## 3. Las tres secuencias

**Las secuencias, las ventanas y los destellos son IDÉNTICOS en las dos puntas.** Lo que
cambia es la acción, porque **el Esclavo no tiene modos de operación propios**.

| Secuencia | Ventana | Acción en el **Maestro** | Acción en el **Esclavo** | Confirmación |
|---|---|---|---|---|
| **`A · A · A`** | ≤ **12 s** | **AUTOMÁTICO** — *"a ver si volvió el radio"* | **Vuelve a obedecer** las órdenes del Maestro | **2** destellos rojos |
| **`B · B · B`** | ≤ **12 s** | **ÁMBAR intermitente** | **Ámbar local**, ignorando al Maestro (ver §3.1) | **3** destellos rojos |
| **`A · B · A · B`** | ≤ **18 s** | **Entrar a MODO DEGRADADO** | **Entrar a MODO DEGRADADO** | **4** destellos rojos |
| *(secuencia correcta, condiciones no cumplidas)* | — | **RECHAZADO** | **RECHAZADO** | **Ámbar rápido de 2 s** |

> **Que los destellos signifiquen lo mismo en las dos puntas no es cosmético.** El
> procedimiento del Modo Degradado obliga a activar **cada unidad por separado**, y el mismo
> operario hace las dos. Dos vocabularios de destellos distintos serían una invitación a
> equivocarse en la segunda punta.

### 3.1 ⚠️ En el Esclavo, `B·B·B` desobedece al Maestro a propósito

> ## ✅ 31/08/2026 — ESTA DESOBEDIENCIA SE CONSERVA, Y AHORA TIENE DOS ARMADORES
>
> **Es la barrera que decidió que el mando no se retirara.** Se conserva el aviso anterior debajo,
> tachado, porque su razonamiento es lo que hay que releer si alguien vuelve a proponer borrar
> `mando.cpp`.
>
> **MEDIDO el 31/08 en `Esclavo/src/main.cpp`** — los tres vetos leen hoy **dos** banderas:
>
> ```
>   :406   if (!mando_ambarLocal() && !bluetooth_ambarEmergencia()) {    // CMD_GO_RED
>   :416   if (!mando_ambarLocal() && !bluetooth_ambarEmergencia()) {    // CMD_GO_GREEN  <- abre paso
>   :540   if (!mando_ambarLocal() && !bluetooth_ambarEmergencia() && ...) // tras S_FALLO
> ```
>
> | Bandera | Quién la arma | Sigue viva |
> |---|---|---|
> | `mando_ambarLocal()` | `Esclavo/src/mando.cpp:132`, dentro de `ejecutar(ACC_AMBAR)` = **`B·B·B`** | ✅ `B` es `PB13`, se conserva |
> | `bluetooth_ambarEmergencia()` | `Esclavo/src/bluetooth.cpp:130` y `:171` = **`CMD:AMBAR_EMERGENCIA`** | ✅ **nuevo camino, sin PIN** |
>
> **Que sean dos no es redundancia decorativa:** el mando funciona sin radio ni teléfono, y la app
> funciona sin receptor de relés. Hoy **el receptor no está comprado**, así que en la práctica el
> camino disponible es el de Bluetooth.
>
> > ### 📕 ~~28/08/2026 — ESTA DESOBEDIENCIA ES EL VETO QUE SE PIERDE~~ *(superado, se conserva)*
> >
> > ~~Sin mando ni pulsadores, `ambarLocal` **nunca vuelve a ser `true`**, los tres `if
> > (!mando_ambarLocal())` quedan siempre-verdaderos y **el Maestro recupera la capacidad de abrir
> > paso en el Esclavo en situaciones en las que hoy tiene prohibido hacerlo**. Nadie borra una línea
> > y ningún instrumento se pone rojo.~~
> >
> > **Sigue siendo el aviso correcto para el escenario que describe** —retirar el armador de una
> > bandera **no** la deja inerte: la deja siempre-falsa, y eso **abre** el `if` que la leía—. Lo que
> > cambió es que ese escenario **no se ejecutó**.
> >
> > *(Las líneas `:401`, `:408` y `:526` que citaba se han desplazado a `:406`, `:416` y `:540` al
> > añadirse la segunda bandera.)*

Mientras el ámbar local esté activo, **el Esclavo NO obedece las órdenes de luz del
Maestro** y **tampoco acusa recibo de ellas**. Se queda en ámbar intermitente hasta que
alguien haga `A·A·A`.

Tiene que ser así. `B·B·B` existe para que nadie quede atrapado con un semáforo en estado
raro a 5 m de altura, y **un ámbar que el Maestro pudiera pisar con el siguiente `GO_GREEN`
no serviría de nada**: el operario estaría trabajando bajo una luz que vuelve a dar paso.

> **Y por eso tampoco contesta.** Acusar recibo sin encender la luz sería **mentirle al
> Maestro**, que seguiría dando verde a su lado creyendo que aquí hay rojo — una punta en
> verde confiado contra otra en ámbar es **exactamente la asimetría peligrosa** que todo
> SFTY-21 existe para evitar.
>
> Callando, el Maestro agota sus reintentos, cae a `C_FALLO` en ~12,5 s y **se va también a
> ámbar**. Es el único final correcto: el operario pidió ámbar en una punta y **el cruce
> entero termina en ámbar**.

### Memotecnia

> **`A` es arriba → SUBE al modo normal.**
> **`B` es abajo → BAJA al mínimo seguro.**
> **Alternar → modo especial.**

Se aprende en un minuto, que es el requisito real para alguien que lo usa **de madrugada y
bajo lluvia**.

### De dónde salen las ventanas de 12 s y 18 s

No son un gusto. Con ~2 s por pulsación, tres pulsos son ~6 s y cuatro son ~8 s **en el caso
cómodo**; el doble si el operario duda o el relé tarda. **12 s y 18 s dejan ese margen sin
llegar a ser tan largas como para que dos gestos separados se sumen en una secuencia que
nadie hizo.** Los pulsos más viejos que la ventana se descartan activamente (`purgarViejos()`).

---

## 4. La confirmación: destellos ROJOS contables

El operario no ve la LCD, pero **sí ve el semáforo**. Es la única salida visible desde el
piso.

> ### 🔴 Por qué rojos y no de colores
>
> **El rojo nunca significa "pase".** Si el operario cuenta mal, el peor caso sigue siendo
> seguro. Destellar los tres colores se descartó explícitamente: **un conductor lejano
> podría interpretar el verde.**

| Parámetro | Valor (`semaforo.cpp`) |
|---|---|
| Destello encendido | `DESTELLO_ON_MS = 400 ms` |
| Destello apagado | `DESTELLO_OFF_MS = 400 ms` |
| Hueco inicial | Sí — las luces se apagan antes del primer destello, para que se vea |
| Ámbar de rechazo | `RECHAZO_AMBAR_MS = 2000 ms`, ámbar fijo |

**Primero se confirma, luego se actúa.** La acción queda *pendiente* y se ejecuta cuando
terminan los destellos, nunca antes: ejecutar a medias dejaría la cuenta incompleta y al
operario sin saber si se reconoció.

**Mientras hay una confirmación en curso, los pulsos nuevos se ignoran.** El operario está
contando destellos, no pulsando; y si pulsa de nervios, no debe encadenarse una segunda
acción sobre la primera.

### Todo-rojo antes de cualquier acción

Las tres secuencias **detienen el ciclo en las dos puntas antes de que empiece la nueva
acción** (`coordinador_forzarRojoTotal()`). Ninguna transición del mando puede saltar a
verde desde lo que hubiera antes, y la máquina del coordinador queda en un estado definido
en vez de a mitad de un cambio.

### El rechazo NO toca el ciclo en curso

El ámbar rápido de 2 s ocupa las luces esos dos segundos y nada más: **el equipo sigue
haciendo lo que hacía**. El operario pidió un cambio que no se le concedió, y dejarlo en un
estado distinto del que tenía **sería concederle otro distinto del que pidió**.

---

## 5. Por qué SOLO `A` y `B`, y nunca `C` ni `D`

La primera versión de este diseño usaba `C` y `D`, razonando que repetir *arriba* o *abajo*
es normal al navegar y podría disparar una secuencia por accidente. **Ese razonamiento
estaba invertido**, y el cliente lo corrigió el 01/08/2026.

```text
   Equipo dejado en el MENU, y llega C·C·C desde el piso:
     1er C  ->  SELECCIONA lo que tenga el cursor -> arranca un modo que nadie pidio
     2o  C  ->  en Modo Manual, C es ROJO FIJO INDEFINIDO
     3er C  ->  ...

   Mismo caso, llega A·A·A:
     el cursor sube tres veces.  No ocurre NADA.
```

> **`C` ejecuta; `A` y `B` solo mueven.** La regla es: **a ciegas se usan únicamente los
> botones cuya repetición accidental es inofensiva.**

**En el Esclavo la regla se sostiene por el mismo motivo**, con otro ejemplo: allí `C` era el
botón que **confirmaba la entrada al Modo Degradado** y `D` el que navegaba hacia atrás,
mientras que `A` y `B` solo mueven el cursor entre dos opciones — y repetirlos **no hace
absolutamente nada**.

**Y por eso `A·B·A·B` va alternado:** no se produce nunca navegando —se sube o se baja, no
se zigzaguea—. Si el operario se equivoca a mitad de la secuencia, **lo único que ha
ocurrido es que el cursor se movió**.

`botones.cpp` solo entrega al mando los flancos de los botones 1 y 2. Los botones 3 y 4
nunca llegan a `mando_registrarPulso()`.

### ✅ 31/08 — el argumento de esta sección lo ha ratificado el hardware

**Este apartado razonaba que `C` y `D` no debían usarse *a ciegas*. El 31/08 dejaron de existir como
pulsadores**, así que la regla ya no cuelga de una decisión de diseño: **no hay pin detrás**.

**MEDIDO** en `Maestro/src/botones.cpp:280-281` y su equivalente del Esclavo:

```
  bool botonAceptar() { return false; }
  bool botonCancelar(){ return false; }
```

**No se borraron, y la razón está escrita en el fuente:** tienen veintitantos llamadores en nueve
ficheros, y borrarlas convertiría una reasignación de pines en **una reescritura del control de flujo
de cada modo, la salida del Degradado incluida** — que es justo donde se cuelan los errores en un
cambio que no debería cambiar comportamiento. Devolviendo `false`, `git grep botonCancelar` sigue
listando **en una sola lista** todo lo que la retirada de `C` y `D` se llevó por delante.

> ### 🔴 Consecuencia que hay que leer entera: en el ESCLAVO, el mando ~~es ahora~~ **FUE** la ÚNICA vía
>
> > ⚠️ **CADUCADO EL 05/09, EN LAS DOS MITADES, Y EN DIRECCIONES OPUESTAS.** (1) El mando **se
> > retiró como hardware** (`D-1`), así que dejó de poder ser la vía de nadie. (2) Y el Esclavo
> > **ya tiene `SET_MODO:DEGRADADO` por Bluetooth**, añadido ese mismo día —el censo está en el
> > aviso de §9—, así que la premisa de abajo tampoco se sostiene. **Se conserva porque la tabla
> > sigue documentando bien qué se perdió al enmudecer *Aceptar*.**
>
> ~~El Esclavo **no tiene `SET_MODO` por Bluetooth** (`§4.4` del Manual 10: el Maestro es el único que
> arbitra el ciclo). Con *Aceptar* mudo, **el sustituto de esos dos botones en esta punta no es la
> app — es el mando de relés**:~~
>
> | Lo que se perdió | Dónde estaba | Con qué se sustituye |
> |---|---|---|
> | Entrar al Modo Degradado | `Esclavo/src/menu.cpp:227` | **`A·B·A·B`** → `ACC_DEGRADADO` (`mando.cpp:148`) |
> | Salir del Modo Degradado | `Esclavo/src/menu.cpp:215` | **`A·A·A`** → `ACC_OBEDECER` (`:121`) · **`B·B·B`** → `ACC_AMBAR` (`:138`) |
>
> **Esto sube el mando de «conveniencia desde el piso» a «único actuador de modo del Esclavo».** Y
> como el receptor sigue sin comprarse, hoy la vía real de esas dos acciones es **subir al gabinete y
> pulsar `A` y `B` a mano**.
>
> *(En el Maestro sí hay sustituto por app: `SET_MODO:AUTO|MANUAL|AMBAR|MENU|ALCANCE|INTELIGENTE|
> DEGRADADO`, `SET_TIEMPOS`, `MANUAL:CAMBIAR_TURNO`, `SET_RTC` y `REINICIAR_RELOJ` — censados
> llamador a llamador en `Maestro/src/botones.cpp:255-275`.)*

> ### ✅ Y un efecto lateral que va en la dirección buena (SFTY-21)
>
> Con *Aceptar* mudo, **la pantalla del Esclavo no puede bajar del listado**, así que
> `menu_estaAbierto()` es siempre falso y **el mando deja de poder quedarse inhibido por una pantalla
> que alguien olvidó abierta**. Era la preocupación explícita de la `§6` de este manual —*«el síntoma,
> "el mando no responde nunca en el Esclavo", sería indistinguible de un receptor averiado»*—, y ya no
> puede ocurrir por esa causa.

---

## 6. ~~🔒 Las secuencias se ignoran con el menú abierto~~ 🔴 **ESTA PROTECCIÓN SE QUEDÓ SIN SUJETO**

> # 🔴 NO SE REFORZÓ: SE QUEDÓ SIN SUJETO. Y EN EL ESCLAVO YA NO PUEDE SER CIERTA
>
> **Ésta es la consecuencia menos visible de retirar la botonera, y es la única de este
> documento que puede morder a alguien en el futuro.** La regla de abajo describe una
> **interacción entre dos cosas muertas**: un menú que no se puede recorrer y un mando que no
> está montado. Pero las dos mitades no murieron igual, y la diferencia es la que importa.
>
> ## Medido, punta por punta
>
> **MAESTRO — la guarda vive a medias:**
>
> ```
> $ grep -n "static bool secuenciasInhibidas" -A 4 Maestro/src/mando.cpp
> 89:static bool secuenciasInhibidas() {
> 90-  ModoSistema m = modoActual_get();
> 91-  return (m == MENU || m == MODO_HORA);
> 92-}
> ```
>
> * `m == MENU` ✅ **sigue pudiendo ser cierta**: se entra por `SET_MODO:MENU` desde la app.
> * `m == MODO_HORA` 🛑 **no puede ser cierta nunca.** `AJUSTAR HORA` es un modo **al que no se
>   puede entrar**: su única puerta es la opción 1 del submenú, detrás de dos `botonAceptar()`,
>   y **por Bluetooth no existe `SET_MODO:HORA`** — hay siete ramas `strcmp(accion,
>   "SET_MODO…")` en `Maestro/src/bluetooth.cpp` y ninguna es `HORA`. Lo dice el propio
>   firmware: *«El equipo estaba mandando a leer un instrumento que nadie puede abrir»*.
>
> **ESCLAVO — la guarda NO puede ser cierta jamás:**
>
> ```
> $ grep -n "bool menu_estaAbierto" -A 2 Esclavo/src/menu.cpp
> 161:bool menu_estaAbierto() {
> 162:  return pantalla != P_MENU;
> 163:}
> ```
>
> `pantalla` arranca en `P_MENU`, y **la única salida de `P_MENU` es `if (aceptar)`** —
> `botonAceptar()`, que devuelve `false` siempre—. `secuenciasInhibidas()` del Esclavo **es
> exactamente esa llamada**, así que es **permanentemente falsa**.
>
> ## 🔴 Y el firmware ya lo escribió… como efecto lateral BUENO
>
> El bloque sobre `botonAceptar()` en `Esclavo/src/botones.cpp` dice: *«con ACEPTAR mudo, la
> pantalla del Esclavo no puede bajar del listado, así que `menu_estaAbierto()` es siempre falso
> y el mando deja de poder quedarse inhibido por una pantalla que alguien olvidó abierta
> (SFTY-21)»*.
>
> **Es cierto, y es media verdad.** Es cierto que desaparece el riesgo de *inhibición eterna*
> que esta misma §6 avisaba abajo (*«el mando no responde nunca en el Esclavo»*). **La media que
> falta es que la barrera no queda inerte: queda abierta**, exactamente como el veto de
> `mando_ambarLocal()` que `D-1` conserva por escrito. **Es el mismo error en la otra dirección
> —una guarda que ya no puede ser falsa en vez de una bandera que ya no puede ser cierta— y hoy
> no lo ve ningún instrumento**, porque el firmware sigue siendo correcto: sin pulsos que
> inhibir no hay diferencia observable.
>
> ## ⚠️ Cuándo se cobra esto, y qué hay que hacer entonces
>
> **Mientras `J16` p5/p8 estén sin cablear, no hay síntoma.** El día que alguien cierre algo
> ahí —y la salida `(b)` de `A-11` en `DECISIONES.md` propone justo eso: *«volver a poner dos
> pulsadores en `J16` p5 y p8 del Esclavo, CERO firmware»*— **esas secuencias no las inhibe
> nada**, y los pines se siguen leyendo activos en ALTO.
>
> 🔴 **Por eso `A-11` no es sólo «cero firmware»: reponer los pulsadores repone el mando y NO
> repone su inhibición.** Va escrito aquí para que quien evalúe esa salida lo sepa **antes** de
> decidir, y no después.
>
> ## ✅ Lo que ya NO puede pasar, y por qué el riesgo de abajo es histórico
>
> El veneno que esta sección describe —*ráfaga de pulsos → el cursor llega a `AJUSTAR HORA` →
> unos pulsos más CONFIRMAN una hora inventada*— **es hoy imposible**: confirmar exige
> `botonAceptar()`. **El riesgo de SFTY-18 por esta vía está cerrado por construcción.** Lo que
> queda abierto es lo de arriba: la guarda que ya no puede ser cierta.

> ### 📕 HISTÓRICO — la regla original. Se conserva, no se borra

~~**Regla:** mientras haya un cursor capaz de **confirmar** algo, las secuencias del mando
**no se reconocen** (`secuenciasInhibidas()`).~~

| Unidad | Dónde se inhibía | **hoy** |
|---|---|---|
| **Maestro** | En `MENU` y en `AJUSTAR HORA` | ⚠️ **`MENU` sí; `AJUSTAR HORA` es inalcanzable** |
| **Esclavo** | Cuando el menú está abierto (`menu_estaAbierto()`), con **regreso automático al listado por inactividad** | 🔴 **nunca: `menu_estaAbierto()` es siempre `false`** |

> ### ⚠️ El criterio del Maestro NO se puede copiar literalmente al Esclavo
>
> En el Maestro, el menú **es un modo del que se sale**. En el Esclavo **la pantalla no se
> cierra nunca**: tomar "hay algo dibujado" como "el menú está abierto" **dejaría el mando
> muerto SIEMPRE**. Por eso el Esclavo necesita además un **regreso automático al listado
> por inactividad**, que impide que una pantalla olvidada abierta inhiba el mando de forma
> indefinida.
>
> Es el tipo de detalle que se pierde al portar un módulo entre proyectos, y el síntoma
> —"el mando no responde nunca en el Esclavo"— sería indistinguible de un receptor averiado.

Empezó siendo un afinamiento opcional —evitar que un técnico que baja tres veces con `B`
dispare el ámbar, molesto pero inofensivo—. **Al añadirse `AJUSTAR HORA` pasó a ser
requisito**, porque el riesgo dejó de ser inofensivo:

```text
   Rafaga accidental de pulsos con el menu abierto
        -> el cursor llega a AJUSTAR HORA
        -> unos pulsos mas CONFIRMAN una hora cualquiera
        -> el reloj queda MARCADO COMO VALIDO con una hora inventada
```

> Eso es **exactamente el veneno que SFTY-18 existe para evitar**: no la falta de reloj,
> sino **un reloj falso que se cree bueno**. Y habilitaría el propio Modo Degradado sobre
> una hora inventada.

**¿Y no contradice esto la regla de que `B·B·B` funciona desde cualquier estado?** No. En
esas dos pantallas **el equipo YA ESTÁ en estado seguro**: el menú mantiene Rojo Fijo con
enlace y Ámbar Intermitente sin él. **No hay nada de lo que rescatar a nadie.**

**Y desde el piso se distingue sin ver la pantalla: si las luces están ciclando, el menú no
está abierto.**

### ~~Refuerzo por estructura, no solo por código~~ 🛑 **el refuerzo desapareció con el Botón 3**

> 🛑 **El argumento de abajo se apoyaba entero en el Botón 3, y el Botón 3 ya no existe.**
> `PB14` es hoy `CAM_C_PIN`, una entrada de cámara cableada y verificada en banco el 03/09, y
> `botonAceptar()` devuelve `false`. **Ya no hay «un nivel por debajo» al que bajar**, así que
> tampoco hay estructura que refuerce nada: **la protección no cuelga hoy de dos cosas ni de
> una.** Se conserva tachado porque explica por qué el menú se partió en dos niveles.

~~El menú de dos niveles (V8.7) coloca `AJUSTAR HORA` y `MODO DEGRADADO` **un nivel por
debajo**, y para bajar hace falta el **Botón 3 — el único que las secuencias del mando
tienen prohibido usar**. Así el requisito no cuelga de una sola comprobación en el
firmware: **la estructura del menú lo refuerza por su cuenta**.~~

---

## 7. La red de seguridad real no es la secuencia

```text
   A·B·A·B desde el piso
        |
        +-- ¿El reloj esta en hora?
        +-- ¿Hubo sincronizacion por radio alguna vez?
        +-- ¿La ultima sincronizacion es reciente (< 48 h)?
        +-- ¿Hay medicion de desfase, y dentro de tolerancia (±3 s)?
        |
        +-- SI  ->  4 destellos rojos  ->  entra a MODO DEGRADADO
        |
        +-- NO  ->  AMBAR RAPIDO 2 s   ->  RECHAZADO
```

**Aunque alguien acierte `A·B·A·B` por casualidad, el firmware no entra si las condiciones
no se cumplen.** El mando permite reactivar en campo sin grúas, pero **no** saltarse la
puesta a punto.

> **Se usa la misma función que la entrada por pantalla** (`modo_degradado_evaluarEntrada()`):
> **una sola puerta, un solo criterio.** Dos comprobaciones equivalentes en dos sitios acaban
> divergiendo, y la que se quede atrás será la que autorice lo que la otra rechazaba.

Y el Modo Degradado **vuelve a evaluar la puerta al arrancar**, aunque el mando ya la haya
evaluado: la entrada por pantalla no pasa por el mando, y **una puerta que dependa de que la
compruebe quien llama no es una puerta**.

### ⚠️ El rechazo desde el piso no dice POR QUÉ

El ámbar rápido de 2 s significa "rechazado", pero **no dice cuál de las cuatro condiciones
falta** — eso solo se lee en la pantalla. **Si la entrada se rechaza, hay que subir.** Es
una limitación aceptada: con solo las luces como salida no hay forma de codificar cinco
motivos distintos sin arriesgar que se cuenten mal.

---

## 8. Asimetría deliberada: lo seguro fácil, lo peligroso difícil

| Si se dispara por accidente | Consecuencia | Protección |
|---|---|---|
| `A·A·A` → Automático | Sin radio cae a ámbar solo en 12 s (SFTY-6). **Seguro** | Ninguna, y no hace falta |
| `B·B·B` → Ámbar | El equipo va a seguro. Molesto, no peligroso | Secuencia corta, **sin condiciones** |
| `A·B·A·B` → Degradado | **Verde sin confirmar el otro lado** | Secuencia larga alternada **+ validación en firmware** |

**`A·A·A` no necesita protección porque el sistema se corrige solo.** El peor caso de
intentar Automático es **volver al ámbar, que es justo donde se estaba**. Y arranca
**directo, sin el asistente de configuración**: desde el suelo no hay pantalla que rellenar.

**Y el resultado se ve desde el piso, sin pantalla:**

```text
   A·A·A  ->  2 destellos  ->  esperar ~15 s

     luces CICLANDO  ->  el radio volvio, ya esta en automatico
     luces en AMBAR  ->  sigue muerto; puede volverse al degradado
```

**`B·B·B` devuelve a ámbar desde cualquier estado en marcha, sin condiciones.** Es la regla
que impide que nadie quede atrapado con un semáforo en estado raro a 5 m de altura. **Una
salida de emergencia con requisitos no es una salida de emergencia.**

> # ✅ 31/08/2026 — LA FRASE DE ARRIBA VUELVE A ESTAR VIGENTE, Y AHORA TIENE SUSTITUTO
>
> **Se destacha.** El 28/08 se tachó porque el mando se retiraba entero; el 31/08 el mando **se
> conserva en `A` y `B`**, y `B·B·B` es exactamente una de las dos secuencias que sobreviven.
>
> ## 🔴 Y hay que corregir una afirmación que este manual publicaba como MEDIDA y era FALSA
>
> El aviso del 28/08 decía, con la palabra *«MEDIDO»* encima:
>
> > ~~**Sin sustituto** — MEDIDO: `Esclavo/src/bluetooth.cpp` acepta `FORZAR_ROJO` (`:109`, `:124`),
> > `SOLICITAR_PASO` (`:128`), `TEST_LEDS` (`:146`) y `SET_RTC:` (`:159`). **Ninguno pone al Esclavo
> > en ámbar.** `FORZAR_ROJO` es rojo, no ámbar, y no revoca nada~~
>
> **Es falso en las dos mitades, y se refuta con la medida — no se borra** (`CLAUDE.md §4`: una
> refutación también es un instrumento, y tachar exige el mismo rigor que afirmar).
>
> **MEDIDO el 31/08 sobre `01_Firmware/Esclavo/src/bluetooth.cpp`:**
>
> ```
>   :130   if (strcmp(cmd, "CMD:AMBAR_EMERGENCIA") == 0) {      <- SIN PIN
>   :131     semaforo_iniciarFallo();
>   :132     ambarEmergencia = true;                            <- el latch
>   :133     enviarTramaConCrc("$ACK,CMD:AMBAR_EMERGENCIA,RESULT:OK");
>   :171   } else if (strcmp(accion, "AMBAR_EMERGENCIA") == 0)  <- la forma CON PIN
>   :268   bool bluetooth_ambarEmergencia() { ... }             <- lo leen los tres vetos
> ```
>
> | Lo que decía | Lo que mide el fuente |
> |---|---|
> | *«ninguno pone al Esclavo en ámbar»* | **`CMD:AMBAR_EMERGENCIA` sí lo pone**, y se acepta **sin PIN** |
> | *«`FORZAR_ROJO` es rojo, no ámbar»* | **`FORZAR_ROJO` ya no es nada en el Esclavo**: contesta `$ERR,…,DESC:RENOMBRADO_USE_AMBAR_EMERGENCIA` por las dos formas (`:157`, `:176`). Era el **nombre** lo que estaba mal: aquella rama ya hacía ámbar |
> | *«no revoca nada»* | **Sí revoca:** `bluetooth_ambarEmergencia()` entra en los tres vetos de `main.cpp:406`, `:416`, `:540`, junto a `mando_ambarLocal()` |
>
> ## Estado real de la salida de emergencia del Esclavo, hoy
>
> | Vía | Estado |
> |---|---|
> | **`B·B·B` desde el mando** | ✅ **en el firmware**, 🛑 **sin receptor comprado**. Ejecutable subiendo al gabinete, no desde el piso |
> | **`CMD:AMBAR_EMERGENCIA` desde la app** | ✅ **en el firmware**, sin PIN, con latch y con veto |
> | Prueba de banco de cualquiera de las dos | ❌ **ninguna** |
>
> **La frase de arriba sigue siendo el criterio con el que se juzga el sustituto**, y el sustituto por
> Bluetooth lo cumple *en el fuente*: desde cualquier estado, sin condiciones y sin PIN. **Lo que
> falta es la tarjeta.** No se lea esto como permiso.

---

## 9. ⚠️ La salida también debe poder hacerse desde el piso

> ### 🔴 DESDE EL PISO, YA NO. Y el requisito se mudó al teléfono, no desapareció
>
> **Este requisito sigue siendo correcto**, y decirlo es más útil que tacharlo. Lo que cambió es
> quién lo cumple: **desde el suelo ya no hay forma** —el mando se retiró (`D-1`) y la pantalla
> no se monta—, así que **las dos vías físicas que esta sección da por supuestas se fueron a la
> vez**. Es el hueco `A-11` de `DECISIONES.md`.
>
> ✅ **Se está cerrando por Bluetooth. Censo del despachador del Esclavo a fecha de esta revisión
> (05/09):**
>
> ```
> $ grep -n 'strcmp(accion, "' Esclavo/src/bluetooth.cpp
> 537:  if (strcmp(accion, "AMBAR_EMERGENCIA") == 0) {
> 575:  } else if (strcmp(accion, "CANCELAR_AMBAR") == 0) {
> 649:  } else if (strcmp(accion, "FORZAR_ROJO") == 0) {
> 657:  } else if (strcmp(accion, "SOLICITAR_PASO") == 0) {
> 675:  } else if (strcmp(accion, "SET_MODO:DEGRADADO") == 0) {
> 744:  } else if (strcmp(accion, "TEST_LEDS") == 0) {
> ```
>
> **`SET_MODO:DEGRADADO` es la ENTRADA**, y es de hoy. **La SALIDA no tiene comando propio**:
> sale por `AMBAR_EMERGENCIA` y por `FORZAR_ROJO`, que pasan por el envoltorio
> `salidaDegradadoIniciada()` — bien hecho, porque **repregunta la misma guarda** que
> `degradado_salir()` en vez de contestar `$ACK` a ciegas.
>
> 🔴 **Y el aviso de esta §9 sigue aplicando palabra por palabra, sólo que al teléfono:** *«si se
> puede entrar desde el suelo pero para salir hay que subir, el mando no sirve»*. **La entrada
> tiene comando propio y la salida no**, así que la asimetría que esta sección describe —una
> punta fuera del Degradado y la otra dentro— **se sigue evitando por la disciplina del
> operario, no por el diseño de la interfaz.** Quien cierre `A-11` que lea esto antes.
>
> ⚠️ **Esto se movió mientras se redactaba esta revisión.** Vuelva a correr ese `grep` antes de
> fiarse del párrafo, y consulte `A-11` en `DECISIONES.md`, que es quien manda. **Este manual no
> cierra `A-11`.** Lo que sí aporta, y va en el aviso de §6: la salida `(b)` de `A-11` —reponer
> dos pulsadores en `J16` p5/p8— **repone el mando pero NO repone su inhibición**.

~~Si se puede **entrar** al Modo Degradado desde el suelo pero para **salir** hay que subir, el
mando no sirve.~~ **Sigue siendo verdad, y por eso `A-11` está abierta.** El escenario típico
es *"dejó de llover, a ver si volvió el radio"*: `A·A·A`.

> ### 🚨 Riesgo: salida asimétrica
>
> **Sacar del Degradado UNA SOLA punta con `A·A·A` crea el escenario más peligroso del
> sistema:**
>
> ```text
>    Una unidad sale del Degradado -> sin enlace -> AMBAR   el conductor NEGOCIA el paso
>    La otra sigue dando verde por reloj                    el conductor pasa CONFIADO
> ```
>
> Un lado en ámbar contra un lado en verde es **exactamente lo que este modo quiere
> evitar**. No tiene solución técnica sin radio.
>
> **Mitigación procedimental, no técnica: la verificación visual de ambas puntas es
> obligatoria también AL SALIR**, no solo al entrar. Debe constar en el acta de pruebas.
> Ver `05_Funcional/8_Procedimiento_Modo_Degradado.md`.

---

## 10. Requisitos de compra del receptor

| Requisito | Por qué |
|---|---|
| **4 canales de salida por relé, contacto seco** | Se cablean en paralelo con los botones. No debe inyectar tensión |
| **Modo biestable NO** — pulso por flanco | Es lo que el firmware espera y lo que se midió en campo |
| ⚠️ **Código independiente por unidad** | **Crítico.** Si el receptor del Esclavo responde al mismo mando que el del Maestro —y las dos puntas suelen estar a menos de una cuadra—, **una sola secuencia actuaría sobre ambas**. Eso convierte cualquier gesto del operario en un cambio simultáneo no verificado en las dos puntas, que es justo lo contrario de la verificación independiente que el procedimiento exige |

---

## 11. Limitaciones conocidas

| Limitación | Estado |
|---|---|
| **No hay receptor de relés instalado en ninguna punta** | El firmware está listo en ambas (N-19 cerrado del lado software). Falta comprar e instalar el hardware |
| **Sin prueba de banco** | Nada se ha ejercitado con un mando físico conectado |
| **El rechazo no indica el motivo desde el piso** | Aceptado. Hay que subir a leer la pantalla |
| **No hay realimentación de "pulso recibido"** | El operario no sabe si un pulso entró hasta completar la secuencia. Consecuencia directa de tener solo las luces como salida |
| **El estado del Degradado no persiste a un corte** | Pendiente **N-20**. Un microcorte puede crear la salida asimétrica de §9 sin que nadie toque el mando |
| ~~🛑 **El mando entero se retira (28/08/2026)**~~ | ✅ **SUPERADO el 31/08.** Se conserva en `A` (`PB9`) y `B` (`PB13`); se retiran `C` y `D`, que pasan a cámaras. **Las tres secuencias siguen funcionando.** Ver cabecera |
| ~~🛑 **Al retirarlo, el veto de `ambarLocal` desaparece solo**~~ | ✅ **RESUELTO.** No se retira el armador (`B·B·B` sigue), y además los tres `if` —`main.cpp:406`, `:416`, `:540`— leen ahora **dos** banderas: `mando_ambarLocal()` **y** `bluetooth_ambarEmergencia()` |
| ~~🛑 **El sistema se queda sin salida de emergencia**~~ | ⚠️ **CORREGIDO — la afirmación era falsa.** `CMD:AMBAR_EMERGENCIA` (`Esclavo/src/bluetooth.cpp:130`, sin PIN) **sí** pone al Esclavo en ámbar y **sí** veta las órdenes de radio. Ver §8. **Lo que sigue faltando es el receptor y la prueba de banco**, no el sustituto |
| 🛑 **`C` y `D` cambian de modo de pin y de polaridad** | `INPUT_PULLUP` activo en BAJO → **`INPUT` pelado activo en ALTO**. **No se cablea cámara a `J16` hasta la medida `M3`**, y el firmware nuevo va **cargado en la tarjeta** antes de que nadie enchufe nada (`CLAUDE.md §9.bis`) |
| 🛑 **En el Esclavo el mando pasa a ser el ÚNICO actuador de modo** | Con *Aceptar* mudo y sin `SET_MODO` por Bluetooth en esa punta, entrar y salir del Degradado **solo se hace con `A·B·A·B` / `A·A·A` / `B·B·B`**. Ver §5 |

---

## 12. Preguntas abiertas

1. **¿Cuánto tarda realmente el relé?** Los ~2 s son una medida de campo con el mando
   disponible ese día. Con otro modelo, las ventanas de 12 s y 18 s podrían quedar cortas o
   largas. **Debe re-medirse con el receptor que finalmente se compre.**
2. **¿Se ven los destellos de 400 ms a la distancia de trabajo?** No se ha verificado en el
   poste, ni de día ni de noche. Si no se distinguen, la única realimentación del sistema
   deja de existir.
3. **¿Qué pasa si el receptor rebota?** Un relé que chatee podría generar varios flancos por
   pulsación. El antirrebote de `botones.cpp` filtra 30 ms y exige 200 ms entre flancos, pero
   **eso está dimensionado para un pulsador, no para un relé mecánico**.
