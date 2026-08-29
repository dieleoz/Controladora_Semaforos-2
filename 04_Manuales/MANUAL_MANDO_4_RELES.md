# 🎛️ Manual Técnico del Mando de 4 Relés

**Ubicación:** `04_Manuales/MANUAL_MANDO_4_RELES.md`
**Referencia de diseño:** SFTY-21 (`OPTIMIZACIONES.md`)
**Implementación:** `01_Firmware/Maestro/src/mando.cpp`, `include/mando.h`, `src/botones.cpp`
**Fecha:** 1 de agosto de 2026

> # 🛑 AVISO DEL 28/08/2026 — EL MANDO DE 4 RELÉS SE RETIRA
>
> **El 28/08/2026 se decidió en obra retirar la pantalla LCD, los cuatro pulsadores y el mando de
> relés.** Toda la operación pasa a la App por Bluetooth. Este manual describe un actuador que deja
> de existir.
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
> | Línea | Código | Qué veta |
> |---|---|---|
> | `main.cpp:401` | `if (!mando_ambarLocal()) {` | Que un `CMD_GO_RED` del Maestro fuerce rojo y acuse recibo |
> | `main.cpp:408` | `if (!mando_ambarLocal()) {` | Que un `CMD_GO_GREEN` del Maestro **abra paso** |
> | `main.cpp:526` | `if (!mando_ambarLocal() && semaforo_estado() == S_FALLO && pkt.command == CMD_GO_RED)` | Que la recuperación tras `S_FALLO` revoque el ámbar del operario |
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
> ### Lo que hay que hacer con ello — ESCRITO, no medido
>
> **La decisión técnica NO está tomada** (falta confirmar el chip del ESP32 y qué pasa con el cristal
> `Y2`), así que aquí no se propone implementación. Lo que sí queda fijado:
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

| Canal del mando | Botón físico | Pin STM32 | Función en el menú |
|---|---|---|---|
| **A** | Botón 1 | `PB9` | Arriba |
| **B** | Botón 2 | `PB13` | Abajo |
| **C** | Botón 3 | `PB14` | Aceptar / Ejecutar |
| **D** | Botón 4 | `PB15` | Cancelar / Menú |

**Los cuatro pines son los mismos en el Maestro y en el Esclavo**, así que el cableado del
receptor es idéntico en las dos puntas.

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

> ## 🛑 28/08/2026 — ESTA DESOBEDIENCIA ES EL VETO QUE SE PIERDE
>
> **Esta sección describe la única barrera de seguridad que la retirada del mando rompe de verdad,
> y la rompe en silencio.** Todo lo demás de este manual es papel; esto es un `if` que deja de
> decidir.
>
> **MEDIDO:** la bandera que sostiene esta desobediencia es `ambarLocal`, y se arma en **un solo
> sitio** — `Esclavo/src/mando.cpp:132`, dentro de `ejecutar(ACC_AMBAR)`, la acción de `B·B·B`. Sus
> tres vetos viven en `Esclavo/src/main.cpp:401` (`CMD_GO_RED`), `:408` (`CMD_GO_GREEN`) y `:526`
> (recuperación tras `S_FALLO`).
>
> Sin mando ni pulsadores, `ambarLocal` **nunca vuelve a ser `true`**, los tres `if
> (!mando_ambarLocal())` quedan siempre-verdaderos y **el Maestro recupera la capacidad de abrir
> paso en el Esclavo en situaciones en las que hoy tiene prohibido hacerlo**. Nadie borra una línea
> y ningún instrumento se pone rojo.
>
> **No borre `mando.cpp` del Esclavo sin decidir antes qué pasa con estos tres `if`.** Desarrollo
> completo en el punto 3 del aviso de cabecera.

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

**En el Esclavo la regla se sostiene por el mismo motivo**, con otro ejemplo: allí `C` es el
botón que **confirma la entrada al Modo Degradado** y `D` el que navega hacia atrás,
mientras que `A` y `B` solo mueven el cursor entre dos opciones — y repetirlos **no hace
absolutamente nada**.

**Y por eso `A·B·A·B` va alternado:** no se produce nunca navegando —se sube o se baja, no
se zigzaguea—. Si el operario se equivoca a mitad de la secuencia, **lo único que ha
ocurrido es que el cursor se movió**.

`botones.cpp` solo entrega al mando los flancos de los botones 1 y 2. Los botones 3 y 4
nunca llegan a `mando_registrarPulso()`.

---

## 6. 🔒 Las secuencias se ignoran con el menú abierto

**Regla:** mientras haya un cursor capaz de **confirmar** algo, las secuencias del mando
**no se reconocen** (`secuenciasInhibidas()`).

| Unidad | Dónde se inhibe |
|---|---|
| **Maestro** | En `MENU` y en `AJUSTAR HORA` |
| **Esclavo** | Cuando el menú está abierto (`menu_estaAbierto()`), con **regreso automático al listado por inactividad** |

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

### Refuerzo por estructura, no solo por código

El menú de dos niveles (V8.7) coloca `AJUSTAR HORA` y `MODO DEGRADADO` **un nivel por
debajo**, y para bajar hace falta el **Botón 3 — el único que las secuencias del mando
tienen prohibido usar**. Así el requisito no cuelga de una sola comprobación en el
firmware: **la estructura del menú lo refuerza por su cuenta**.

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

> ~~**`B·B·B` devuelve a ámbar desde cualquier estado en marcha, sin condiciones.** Es la regla
> que impide que nadie quede atrapado con un semáforo en estado raro a 5 m de altura. **Una
> salida de emergencia con requisitos no es una salida de emergencia.**~~

> # 🛑 28/08/2026 — ESTA SALIDA DE EMERGENCIA NO EXISTE. NO CUENTE CON ELLA.
>
> **Un operario que confíe en esta frase no tiene la salida que promete.** El aviso va pegado aquí,
> y no solo en la cabecera, porque quien lee esta página puede no haber leído aquélla — y lo que
> esta frase ofrece es precisamente aquello sobre lo que alguien decide subirse a un poste.
>
> | | |
> |---|---|
> | **Nunca existió** | El receptor de relés **no se compró ni se conectó en ninguna punta**, y no hay prueba de banco (`:8-19` de este manual). Sin receptor, `B·B·B` no se puede accionar desde el piso: **jamás fue ejecutable en campo** |
> | **Y deja de existir** | El mando y los cuatro pulsadores se retiran (28/08/2026). También se va la vía por botón |
> | **Sin sustituto** | **MEDIDO:** `Esclavo/src/bluetooth.cpp` acepta `FORZAR_ROJO` (`:109`, `:124`), `SOLICITAR_PASO` (`:128`), `TEST_LEDS` (`:146`) y `SET_RTC:` (`:159`). **Ninguno pone al Esclavo en ámbar.** `FORZAR_ROJO` es rojo, no ámbar, y no revoca nada |
>
> **La frase tachada sigue siendo un buen requisito — por eso se tacha y no se borra.** *"Una salida
> de emergencia con requisitos no es una salida de emergencia"* es exactamente el criterio que el
> reemplazo por Bluetooth tiene que cumplir. Hoy no lo cumple nadie: **el sistema se ha quedado sin
> salida de emergencia, con requisitos o sin ellos.**
>
> ⚠️ **Y hay una consecuencia de firmware que no se ve desde aquí:** `B·B·B` era el único sitio que
> armaba `ambarLocal`, la bandera de la que cuelgan **tres vetos** en `Esclavo/src/main.cpp`
> (`:401`, `:408`, `:526`). Al retirarla, esos vetos se vuelven siempre-verdaderos y **desaparecen
> sin que nadie borre una línea.** Ver el punto 3 del aviso de cabecera.

---

## 9. ⚠️ La salida también debe poder hacerse desde el piso

Si se puede **entrar** al Modo Degradado desde el suelo pero para **salir** hay que subir, el
mando no sirve. El escenario típico es *"dejó de llover, a ver si volvió el radio"*: `A·A·A`.

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
| 🛑 **El mando entero se retira (28/08/2026)** | Decisión de obra. Todo este manual pasa a describir un actuador inexistente. **Nunca hubo receptor, así que no cambia lo que se puede hacer hoy en campo** |
| 🛑 **Al retirarlo, el veto de `ambarLocal` desaparece solo** | **MEDIDO.** Tres `if` en `Esclavo/src/main.cpp` (`:401`, `:408`, `:526`) se vuelven siempre-verdaderos. **Es lo único de esta retirada que toca seguridad, y hay que resolverlo explícitamente.** Ver punto 3 de la cabecera |
| 🛑 **El sistema se queda sin salida de emergencia** | `B·B·B` era la única *"desde cualquier estado y sin condiciones"*. **No hay sustituto por Bluetooth en el Esclavo.** Ver §8 |

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
