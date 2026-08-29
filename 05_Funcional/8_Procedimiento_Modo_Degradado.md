# 🕹️ PROCEDIMIENTO DE CAMPO — MODO DEGRADADO (SFTY-21)

**Documento para el operario de campo y el Ingeniero Funcional**
**Fecha:** 1 de Agosto de 2026 · **Aplica a:** firmware V8.7 (rama `feat/n15-reloj-pantalla-hora`)

---

> # 🛑 AVISO DEL 28/08/2026 — ESTE PROCEDIMIENTO YA NO SE PUEDE EJECUTAR
>
> **El 28/08/2026 se decidió en obra retirar la pantalla LCD, los cuatro pulsadores y el mando de
> relés.** Toda la operación pasa a la App por Bluetooth.
>
> **Las dos vías de entrada y las dos de salida de este modo son, todas, botones o secuencias de
> mando.** Al retirarlos, este documento describe maniobras **sin actuador**: no hay con qué
> hacerlas.
>
> | Maniobra | Actuador que la ejecuta hoy | Tras la retirada |
> |---|---|---|
> | Entrar (pantalla) | `Botón 4` → `CONFIGURACION` → `Botón 3` → `Botón 3` | ❌ sin actuador |
> | Entrar (piso) | `A · B · A · B` en el mando | ❌ sin actuador |
> | Salir a Automático | `A · A · A`, o `Botón 3` en pantalla | ❌ sin actuador |
> | Salir a Ámbar | `B · B · B`, o `Botón 3` en pantalla | ❌ sin actuador |
> | Volver al MENÚ desde cualquier modo | `Botón 4` (`botonCancelar()`, `PB15`) | ❌ sin actuador |
>
> ## 🔴 Lo que hay que decir sin adornos
>
> **Hasta que exista el comando de vuelta, retirar los pulsadores deja el Modo Degradado como una
> puerta de un solo sentido.**
>
> Y hoy ni siquiera es una puerta: **tampoco existe el comando de ida.** No hay forma de *entrar*
> al Degradado por Bluetooth, así que retirados los botones este modo queda **inalcanzable** salvo
> por la reanudación automática tras un corte (N-20, `main.cpp:115`) — que es justo el camino que
> **nadie pulsa** y del que **nadie podría sacarlo**.
>
> ### MEDIDO — verificado leyendo el fuente el 28/08/2026
>
> | # | Hallazgo | Evidencia |
> |---|---|---|
> | 1 | `botonCancelar()` **es** el `Botón 4` = `PB15` | `Maestro/include/pines.h:95` (`#define BOTON4 PB15  // Cancelar`) · `Maestro/src/botones.cpp:132` |
> | 2 | `botonCancelar()` es la **única** vuelta al MENÚ desde **todos** los modos del Maestro | `modo_degradado.cpp:443` · `modo_alcance.cpp:50` · `modo_ambar.cpp:42` · `modo_automatico.cpp:80` · `modo_hora.cpp:262` · `modo_inteligente.cpp:65` · `modo_manual.cpp:21` · `menu.cpp:151` |
> | 3 | **No existe comando Bluetooth para ENTRAR** en Degradado | `grep DEGRADADO` sobre los dos `bluetooth.cpp` devuelve **una sola línea**: `Maestro/src/bluetooth.cpp:193`, `case MODO_DEGRADADO: return "DEGRADADO";` — la **cadena de estado de `$STATUS`**, no un comando |
> | 4 | **No existe comando Bluetooth para volver al MENÚ** | `coordinador_forzarMenu()` está declarada en `coordinador.h:7` y definida en `coordinador.cpp:545`. Tiene **tres** llamadores —`menu.cpp:82`, `modo_alcance.cpp:40`, `modo_hora.cpp:104`— y **ninguno** es `bluetooth.cpp` |
> | 5 | En el **Esclavo** no hay **ninguna** vía Bluetooth ni de entrada ni de salida | `Esclavo/src/bluetooth.cpp` solo acepta `FORZAR_ROJO` (`:109`, `:124`), `SOLICITAR_PASO` (`:128`), `TEST_LEDS` (`:146`) y `SET_RTC:` (`:159`) |
>
> ### ⚠️ CORRECCIÓN MEDIDA — el Maestro SÍ tiene dos salidas por Bluetooth, y son incompletas
>
> Se dio por sentado que *"desde Bluetooth no existe ningún comando de vuelta"*. **Medido, es falso
> a medias, y la mitad cierta importa más que la falsa.** Hay dos:
>
> ```
>   CMD:PIN:1234:SET_MODO:AUTO    -> modoActual_set(MODO_AUTOMATICO)   bluetooth.cpp:124
>   CMD:PIN:1234:SET_MODO:AMBAR   -> modoActual_set(MODO_AMBAR)        bluetooth.cpp:134
> ```
>
> Y **sí se atienden estando en Degradado**: `bluetooth_loop()` se llama incondicionalmente en
> `main.cpp:145`, antes del despacho de modos. El indicador de respaldo también se borra bien —
> `main.cpp:196-198` lo hace en el cambio de modo, **por cualquier vía**.
>
> **Lo que esas dos salidas NO hacen, y es el motivo por el que no sustituyen al `Botón 4`:**
>
> - **Se saltan el todo-rojo de despedida.** La salida por `Botón 4` fuerza `semaforo_forzarRojo()`,
>   espera `ROJO_TRANSICION_MS` y pinta `"Vea las dos puntas"` (`modo_degradado.cpp:448-462`). La
>   salida por Bluetooth **no**: se pasa de un **verde por reloj** directamente a
>   `modoAutomatico_setup()` / `modo_ambar_setup()` en la iteración siguiente. Es exactamente lo que
>   el comentario del propio firmware llama *"encadenar dos autoridades sin cerrar el paso en
>   medio"* (`Esclavo/src/mando.cpp:118-121`).
> - **No devuelven al MENÚ.** Sacan del Degradado hacia *otro modo en marcha*, no al estado de
>   reposo.
> - **No existen en el Esclavo** (hallazgo 5). Así que una salida por Bluetooth **solo mueve una
>   punta** — que es el **Riesgo residual nº 2** de la Sección 6, provocado a propósito.
>
> ### ESCRITO — decidido, no medido
>
> - La retirada de LCD, pulsadores y mando es una **decisión de obra del 28/08/2026**.
> - **El reemplazo NO está decidido:** falta confirmar el chip del ESP32 y qué pasa con el cristal
>   `Y2`. Por eso **este aviso no propone un procedimiento nuevo**. No lo invente.
>
> ### Qué se hace mientras tanto
>
> 1. **No ejecute nada de las Secciones 3, 5, 7 ni 8 de este documento.** Los pasos tachados abajo
>    no tienen actuador.
> 2. **No retire los pulsadores de una punta que pueda quedar en Degradado** hasta que exista el
>    comando de vuelta. Es la única salida que hoy funciona en las dos puntas.
> 3. El límite duro de 48 h (Sección 4) **sigue vigente y es hoy la única salida garantizada**: el
>    equipo se rinde solo a ámbar (`modo_degradado.cpp:515`). No es un procedimiento — es un tope.

---

> ## ⚠️ ESTE MODO NO HA PISADO HARDWARE TODAVÍA
>
> Está construido en las dos puntas y validado en simulador (20/20 funcional, 10/10 repetidor,
> 83/83 de pantalla), pero **no se ha ejercitado sobre tarjetas reales ni en obra**. Hasta que la
> prueba de banco de la Sección 9 del `3_Protocolo_Pruebas_Rigurosas.md` esté firmada, este
> procedimiento **no autoriza operación en vía abierta al tráfico**.

---

## 1. Qué es, y qué NO es

El Modo Degradado hace que las dos unidades **sigan alternando verde y rojo sin radio**, cada una
calculando su fase a partir de la hora de pared. Es un **caso especial de activación manual**, no un
comportamiento automático.

| Situación | Qué hace el equipo |
|---|---|
| Se pierde el radio | 🟡 **Ámbar intermitente en ambas puntas.** Es el comportamiento por defecto y **no cambió** |
| Un operario activa el Degradado en las dos puntas | El cruce vuelve a alternar verde/rojo, gobernado por reloj |
| Nadie lo activa | El equipo se queda en ámbar indefinidamente. **Es correcto** |

> **El equipo NUNCA entra solo.** No hay temporizador, no hay "si pierdes el radio X minutos,
> entra", no hay autorización por adelantado. Y la razón no es prudencia genérica:

```
 ÁMBAR INTERMITENTE  ->  "no estoy controlando esto, decide tú"
                         el conductor llega ALERTA, mira, negocia el paso

 VERDE POR RELOJ     ->  "pasa tranquilo, el otro lado está en rojo"
                         el conductor llega CONFIADO y no mira
```

Sin radio, el Maestro **no puede saber si el Esclavo sigue vivo**: podría estar apagado, colgado, o
haber sido movido a otra obra. Un verde equivocado es **más peligroso que un ámbar ambiguo**, porque
le quita al conductor la precaución que el ámbar le provoca. Por eso el verde solo se da cuando
**una persona verificó las dos puntas con los ojos**.

---

## 2. Requisitos previos — sin esto el firmware lo rechaza

El Degradado **no entra** si falta cualquiera de estas condiciones. No es un aviso en pantalla que se
pueda saltar: es una puerta en firmware.

| # | Condición | Cómo se cumple | Qué muestra la pantalla si falta |
|---|---|---|---|
| 1 | El Maestro tiene el reloj **puesto en hora** | `CONFIGURACION → AJUSTAR HORA` en el **Maestro** | `Falta: reloj sin poner en hora` |
| 2 | Hubo **al menos una sincronización** por radio con el Esclavo | Ocurre sola al confirmar la hora, y cada hora mientras haya enlace | `Falta: nunca hubo sincronizacion RF` |
| 3 | Esa sincronización es **reciente** — menos de **2 h** | Basta con que el radio haya estado vivo hace poco | `Falta: la ultima sync es muy vieja` |
| 4 | Hay una **medida de desfase** contra el Esclavo | La toma el Maestro por radio (`CMD_DELTA`) | `Falta: sin medida de desfase valida` |
| 5 | Ese desfase está **dentro de ±3 s** | — | `Desfase fuera de tolerancia (+-3s)` |

### Por qué la condición 3 existe y no es burocracia

Podría parecer que basta con medir el desfase y comprobar que es pequeño. **No basta**, y el motivo
es un límite real de la medición:

```
   CMD_DELTA transporta SOLO el segundo (0-59).
   La corrección circular resuelve siempre por el camino corto.

   Desfase real de 45 s  ->  se mide como -15 s
   No hay forma de distinguirlos con solo el segundo.
```

Un desfase peligroso **podría leerse como aceptable**. Lo que cierra ese agujero es la frescura: tras
una sincronización correcta el desfase arranca en milisegundos, y con la deriva de estos cristales
harían falta **más de tres días** para acumular los 30 s que provocan la confusión. Con una
sincronización de hace una hora la deriva es de **~0,36 s**: la medida no puede estar equivocada.

> **El desfase es una comprobación de cordura. La garantía es la sincronización reciente.** Invertir
> esa relación —confiar en el número y no en su frescura— reintroduce el fallo.

### ⚠️ La hora se pone UNA sola vez, y solo en el Maestro

**El Esclavo no tiene pantalla de ajuste de hora, y es deliberado.** Ajustar las dos puntas a mano
deja hasta **59 s de desfase el primer día** sin que nadie pueda verlo:

```
   Operario A confirma el Maestro   a las 14:32:10 reales -> el reloj marca 14:32:00
   Operario B confirma el Esclavo   a las 14:32:50 reales -> el reloj marca 14:32:00

   Las dos pantallas muestran 14:32.  Los relojes están a 40 s.
```

Cuarenta segundos es **más del todo-rojo entero**. La hora se cuadra en el Maestro y **viaja por
radio** al Esclavo; así, el día que el radio muera, el desfase arranca en ~0 de verdad y no por
procedimiento.

---

## 3. Procedimiento de ENTRADA

> **Se activa en LAS DOS PUNTAS, por separado, y exige verificación visual de ambas.** Una punta
> sola en Degradado es peor que ninguna: ver Sección 6.

### Paso 1 — Activar en el **MAESTRO**

> 🛑 **SIN ACTUADOR desde el 28/08/2026 — las dos vías de este paso se retiran.** No se tacha para
> borrarlo: se conserva porque es la especificación de lo que el reemplazo por Bluetooth tendrá que
> reproducir, incluida la doble confirmación. Ver el aviso de cabecera.

Desde la pantalla del gabinete:

1. ~~`Botón 4` hasta llegar al **Menú Principal**.~~
2. ~~Bajar hasta `CONFIGURACION` y entrar con `Botón 3`.~~
3. ~~Bajar hasta `MODO DEGRADADO` y entrar con `Botón 3`.~~
4. ~~La pantalla dice `Pulse 3 para entrar` si se cumplen los 5 requisitos, o **el motivo concreto** si
   no. Si aparece un motivo, resuélvalo — no hay forma de forzarlo.~~
5. ~~`Botón 3` → aparece `CONFIRMAR ENTRADA?` → `Botón 3` otra vez para confirmar.~~

**Por qué se tacha:** los cinco pasos son pantalla + pulsadores, y ambos se retiran. **La puerta de
los 5 requisitos NO se tacha** — vive en `modo_degradado_evaluarEntrada()` y sigue en el firmware.
Lo que desaparece es la forma de llamar a la puerta, no la puerta.

> **Entrar exige dos pulsaciones; salir, una.** La asimetría es deliberada: salir lleva el equipo
> hacia el estado seguro y no necesita protección. Entrar habilita verdes sin confirmación del otro
> extremo, y eso sí.
>
> ⚠️ **Esta asimetría es hoy exactamente al revés, y es el problema.** Retirados los botones no hay
> **ninguna** pulsación para entrar y **ninguna** para salir; lo único que queda por Bluetooth son
> dos salidas *parciales* en el Maestro (`SET_MODO:AUTO` y `SET_MODO:AMBAR`) que **se saltan el
> todo-rojo de despedida** y **no existen en el Esclavo**. Ver la corrección medida en la cabecera.

~~**Desde el piso**, con el mando de 4 relés: `A · B · A · B` en menos de 18 segundos.
Confirmación: **4 destellos rojos**. Si en vez de destellos aparece un **ámbar rápido**, la secuencia
fue **rechazada** por alguno de los 5 requisitos — hay que subir a ver cuál.~~

**Por qué se tacha:** el mando de 4 relés se retira (28/08/2026). Además, ese receptor **nunca se
compró ni se conectó**, así que esta vía **nunca llegó a existir en campo** — ver
`04_Manuales/MANUAL_MANDO_4_RELES.md:8-19`.

### Paso 2 — Activar en el **ESCLAVO**

> ## 🪜 HOY HAY QUE SUBIR AL GABINETE
>
> **El Esclavo no tiene receptor de mando de relés** (pendiente **N-19**). La tarjeta ya trae las
> cuatro entradas (`PB9`, `PB13`, `PB14`, `PB15`); falta comprar e instalar el receptor.
>
> **Soporte Bluetooth desde el Suelo (V9.0):** Con el módulo Bluetooth USART1 y la App Móvil
> instalada en el celular del operario, ~~la activación y~~ sincronización del Degradado en el Esclavo
> **se realiza directamente desde el suelo**, sin necesidad de subir al gabinete con escalera.
> Además, la App incluye el **Modo Courier RTC**, que permite capturar la hora y ciclo en el Maestro,
> viajar hasta el Esclavo y aplicar la sincronización compensando automáticamente el tiempo de viaje
> con error inferior a 0.1 s.
>
> 🛑 **28/08/2026 — «la activación» se tacha: MEDIDO, no existe.** La **sincronización** sí (es
> `SET_RTC:`, `Esclavo/src/bluetooth.cpp:159`). La **activación del Degradado por Bluetooth en el
> Esclavo nunca se implementó**: ese despachador no tiene ninguna rama de Degradado. Se tacha en vez
> de borrarse para que no se vuelva a dar por hecha dentro de un mes — es la funcionalidad que el
> reemplazo del 28/08 tiene que **construir**, no heredar.

En la App Móvil o en la pantalla del Esclavo:

1. **Vía App Móvil (Desde el suelo):** Conectarse al `📡 ESCLAVO (Poste 2)`, entrar a `Ajustes / RTC` y pulsar `[ 🚀 Inyectar en Esclavo ]` ~~o activar Modo Degradado~~.
2. ~~**Vía Pantalla LCD (Gabinete):** `Botón 4` hasta el menú ➔ `MODO DEGRADADO` ➔ `Botón 3` (`CONFIRMAR ENTRADA`).~~

> 🛑 **AVISO 28/08/2026 — el punto 2 se retira, y el punto 1 promete algo que el firmware no tiene.**
>
> - El punto 2 es pantalla + pulsadores: **sin actuador**.
> - **MEDIDO:** *"o activar Modo Degradado"* del punto 1 **no existe**. `Esclavo/src/bluetooth.cpp`
>   acepta exactamente cuatro acciones —`FORZAR_ROJO` (`:109`, `:124`), `SOLICITAR_PASO` (`:128`),
>   `TEST_LEDS` (`:146`) y `SET_RTC:` (`:159`)— y **ninguna** entra ni sale del Degradado. `grep
>   DEGRADADO` sobre ese fichero **no devuelve nada**. La inyección de RTC sí existe y se conserva.
>
> Es la clase de promesa que este repositorio ya pagó con la *«Caja Negra de Alarmas»* (N-73): un
> documento que anuncia como existente una función que nadie llama. Aquí es peor, porque **no está
> ni declarada**.

### Paso 3 — VERIFICACIÓN VISUAL DE AMBAS PUNTAS ← **obligatoria**

Con las dos unidades ya en Degradado, **quédese a ver al menos un ciclo completo (120 s)** y
compruebe con los ojos, no en pantalla:

- [ ] Cuando el **Maestro está en verde**, el **Esclavo está en rojo**
- [ ] Entre los dos verdes hay un **todo-rojo largo** (~30 s) con **ambas puntas en rojo**
- [ ] Cuando el **Esclavo está en verde**, el **Maestro está en rojo**
- [ ] **En ningún momento hay verde simultáneo en las dos puntas**

> **Por qué mirar y no fiarse de la pantalla.** Cada unidad muestra la fase que *ella* calcula. Si
> por lo que sea las dos calculan mal —relojes desfasados, configuración distinta, una unidad que se
> reinició— **las dos pantallas dirán que todo va bien** mientras las luces cuentan otra historia.
> La pantalla informa; **las luces son la evidencia**.

~~Si algo no cuadra: `B · B · B` desde el piso, o `Botón 3` en la pantalla del Degradado, **en las dos
unidades**. Vuelva a ámbar y no insista.~~

> 🛑 **SIN ACTUADOR (28/08/2026) — y este es el tachón más grave del documento.**
>
> Esta línea es el **plan de aborto** de la verificación visual: lo que se hace cuando se descubre
> que las dos puntas no cuadran, con el cruce ya dando verdes por reloj. Sus dos vías son mando y
> pulsador, y las dos se retiran.
>
> **Lo que queda hoy, medido, y por qué no basta:** en el **Maestro**,
> `CMD:PIN:1234:SET_MODO:AMBAR` (`bluetooth.cpp:134`). En el **Esclavo**, `CMD:FORZAR_ROJO`
> (`bluetooth.cpp:109`) — que **no es ámbar y no saca del Degradado**. Es decir: **no hay forma de
> devolver las dos puntas a ámbar por Bluetooth**, que es literalmente lo que este paso ordena
> hacer. La verificación visual sigue siendo obligatoria; lo que ha desaparecido es qué hacer
> cuando falla.

---

## 4. El límite duro de 48 horas

**Pasadas 48 h sin resincronizar, el Degradado cae SOLO a ámbar intermitente.** No es un aviso: es un
tope. A partir de las **44 h** la pantalla muestra `AVISO: LIMITE 48h`, y a las 48 h el equipo se rinde
por su cuenta y muestra `Limite 48h sin sync — Revise el radio`.

### Por qué existe

El colchón que impide el verde simultáneo es el **todo-rojo de 30 s**. Ese colchón se come poco a
poco por la deriva de dos cristales de 32.768 kHz sin calibrar, a la intemperie: **±30 a 50 ppm**.

| Todo-rojo en Degradado | Deriva peor caso | Margen antes de solaparse | Límite adoptado |
|---|---|---|---|
| 15 s *(el de operación normal)* | ~8,6 s/día | ~1,7 días | — *insuficiente* |
| **30 s** ← el que usa este modo | ~8,6 s/día | ~3,5 días | **48 h** |
| 90 s | ~8,6 s/día | ~10 días | 5 días *(a costa de la fluidez)* |

Las 48 h dejan **factor de seguridad 2** sobre el margen teórico.

> **El estado seguro no puede depender de que alguien se acuerde.** Ése es el principio que el resto
> del sistema ya aplica —el fallback de 12 s, el piso de 5 s del despeje— y aquí aplica igual. Un
> modo degradado que dependa de que un operario vuelva a tiempo no es un modo degradado: es una
> apuesta.

> Conviene decirlo sin adornos: **querer una semana de autonomía obligaría a un todo-rojo de ~90 s**,
> que destroza la fluidez del paso. No es una limitación del diseño, es la física de dos cristales sin
> disciplinar. **La alternativa real no es alargar el plazo: es ir a arreglar el radio.**

### Cómo se reinicia la cuenta

Solo con una **sincronización nueva por radio**, que exige que el enlace vuelva. Salir y volver a
entrar al Degradado **no reinicia nada**: el límite mide el tiempo transcurrido desde la última sincronización
real usando el **contador del RTC** (N-49 T1/T2, monótono y sin saltos de fin de mes), no desde la última pulsación.

---

## 5. Procedimiento de SALIDA

> ## ⚠️ LA VERIFICACIÓN VISUAL DE AMBAS PUNTAS ES OBLIGATORIA TAMBIÉN AL SALIR
>
> No solo al entrar. **Salir de una sola punta crea exactamente el escenario que este modo existe
> para evitar** — ver Sección 6.

### Para volver a intentar Automático

Se hace cuando se cree que el radio volvió. El escenario típico es *"dejó de llover, a ver si
enlaza"*.

- ~~**Desde el piso:** `A · A · A` en menos de 12 s → **2 destellos rojos**.~~
- ~~**Desde la pantalla:** en `CONFIGURACION → MODO DEGRADADO`, `Botón 3` (`3=Salir`).~~

> 🛑 **SIN ACTUADOR (28/08/2026).** Las dos vías se retiran.
>
> **Sustituto parcial en el Maestro, medido:** `CMD:PIN:1234:SET_MODO:AUTO`
> (`Maestro/src/bluetooth.cpp:124`). **No es equivalente**: se salta el todo-rojo de despedida de
> `modo_degradado.cpp:448-462` y pasa del verde por reloj directo a `modoAutomatico_setup()`.
> **En el Esclavo no hay sustituto ninguno**, así que ejecutar solo el del Maestro produce el
> **Riesgo residual nº 2** (Sección 6) — una punta fuera, la otra dentro.

~~**Hágalo en las dos unidades**~~ *(hoy imposible: no hay comando en el Esclavo)*, y luego mire las luces:

```
   A·A·A  ->  2 destellos  ->  esperar ~15 s

     luces CICLANDO  ->  el radio volvió, ya está en automático
     luces en ÁMBAR  ->  sigue muerto; puede volverse al degradado
```

Volver a Automático **no necesita protección**, y por eso la secuencia es corta: si el radio sigue
muerto, el propio sistema se corrige a los 12 s (SFTY-6) y cae a ámbar, que es justo donde se quería
estar. **El peor caso de intentar Automático es volver al ámbar.**

### Para irse a ámbar y dejarlo así

- ~~**Desde el piso:** `B · B · B` en menos de 12 s → **3 destellos rojos**.~~
- ~~`B·B·B` funciona **desde cualquier estado y sin condiciones**. Es la regla que impide que nadie
  quede atrapado con un semáforo en un estado raro a 5 m de altura.~~

> 🛑 **SIN ACTUADOR (28/08/2026) — y lo que se pierde no es solo una comodidad.**
>
> `B·B·B` es la **salida de emergencia** del sistema entero. Su promesa —*"desde cualquier estado y
> sin condiciones"*— **deja de ser cierta el día que se retire el mando**, y quien confíe en ella no
> la tiene. El mismo aviso, con más detalle, está pegado a la frase original en
> `04_Manuales/MANUAL_MANDO_4_RELES.md:316-318`.
>
> **MEDIDO — el ámbar del Esclavo desaparece por completo.** `ambarLocal` se arma en un solo sitio,
> `Esclavo/src/mando.cpp:132`, dentro de `ejecutar(ACC_AMBAR)` — la acción de `B·B·B`. No hay otra.
> Retirados mando y pulsadores, **el Esclavo no puede irse a ámbar local por orden de nadie**, y sus
> tres vetos de radio se apagan solos. Ver el hallazgo completo en el manual del mando, §3.1.

### Checklist de salida

- [ ] ~~Secuencia o pulsación ejecutada en el **Maestro**~~ → **sin actuador**; queda `SET_MODO:AUTO` / `SET_MODO:AMBAR` por Bluetooth, **sin todo-rojo de despedida**
- [ ] ~~Secuencia o pulsación ejecutada en el **Esclavo** *(hoy: subiendo al gabinete)*~~ → **sin actuador y SIN SUSTITUTO**: no hay comando Bluetooth de salida en el Esclavo
- [ ] **Verificado con los ojos** que **ambas** puntas quedaron en el mismo estado — las dos ciclando
      o las dos en ámbar *(sigue vigente: mirar no necesita actuador)*
- [ ] **Ninguna punta quedó dando verde por reloj mientras la otra parpadea en ámbar** *(sigue vigente)*

> ⚠️ **Las dos primeras casillas de este checklist no se pueden marcar hoy.** Un checklist con
> casillas imposibles no es un checklist: es una invitación a firmarlo igual. Se conservan tachadas
> para que el reemplazo sepa qué tiene que volver a hacer marcable.

---

## 6. ⚠️ Riesgos residuales — aceptados por el cliente el 01/08/2026

Están escritos aquí porque **el funcional debe conocerlos antes de firmar**, no después de un
incidente.

### Riesgo 1 — El verde se da sin confirmación del otro extremo

**Con el radio muerto es inevitable.** En operación normal, el Maestro no abre un carril hasta que el
Esclavo confirma que está en rojo. En Degradado esa confirmación **no existe**: cada unidad da verde
porque su reloj dice que le toca.

Se mitiga con activación manual verificada, todo-rojo ampliado a 30 s, límite duro de 48 h y aviso en
pantalla. **No se elimina.** Es el precio de operar sin enlace, y por eso este modo es un caso
especial y no el comportamiento por defecto.

### Riesgo 2 — Salida asimétrica: que una sola punta abandone el Degradado

**Es el escenario más peligroso y no tiene solución técnica sin radio.**

```
   Un microcorte reinicia UNA unidad
        -> arranca en el MENÚ (así lo hace main.cpp)
        -> sin enlace  ->  ÁMBAR          el conductor NEGOCIA el paso
   La otra sigue dando verde por reloj    el conductor pasa CONFIADO
```

Un lado en ámbar contra un lado en verde es **exactamente lo que este modo quiere evitar**: el
conductor del lado en verde entra confiado a un tramo que el otro lado está negociando. Ocurre igual
—sin microcorte de por medio— **si un operario saca del Degradado una sola unidad**.

> **Mitigación procedimental, no técnica: la verificación visual de ambas puntas es obligatoria
> también AL SALIR.** Debe constar en el acta de pruebas (`3_Protocolo_Pruebas_Rigurosas.md`,
> Sección 9).

**Pendiente conocido (N-20):** hoy el estado del Degradado y la marca de sincronización viven en RAM,
así que un microcorte los pierde. El módulo `respaldo.cpp` que los guarda en los registros de
respaldo —alimentados por la misma pila CR2032 ya instalada— **está escrito pero todavía no
conectado**. Mientras no lo esté, **cualquier corte de energía en una punta produce el escenario de
arriba**.

---

## 7. Resumen de parámetros

| Parámetro | Valor | Dónde está |
|---|---|---|
| Verde de cada punta | **30 s** | `DEG_VERDE_SEG` |
| Todo-rojo entre verdes | **30 s** *(ya ampliado — el normal son 15 s)* | `DEG_DESPEJE_SEG` |
| Ciclo completo | **120 s** · espera máxima 90 s | 2 × (30 + 30) |
| Antigüedad máxima de la sincronización para entrar | **2 h** | `SYNC_FRESCA_MS` |
| Tolerancia de desfase para entrar | **±3 s** | `TOLERANCIA_DESFASE_S` |
| Aviso de límite en pantalla | a partir de **44 h** | `AVISO_LIMITE_MS` |
| **Límite duro → ámbar** | **48 h** | `LIMITE_DURO_MS` |
| ~~Secuencia de entrada desde el piso~~ 🛑 | ~~`A · B · A · B` en ≤ 18 s → **4 destellos rojos**~~ | ~~`mando.cpp`~~ |
| ~~Secuencia a Automático~~ 🛑 | ~~`A · A · A` en ≤ 12 s → **2 destellos rojos**~~ | ~~`mando.cpp`~~ |
| ~~Secuencia a Ámbar~~ 🛑 | ~~`B · B · B` en ≤ 12 s → **3 destellos rojos**~~ | ~~`mando.cpp`~~ |

> 🛑 **Las tres últimas filas se retiran (28/08/2026): sin actuador.** Los parámetros de *arriba*
> —tiempos, tolerancias y el límite de 48 h— **siguen todos vigentes**: viven en el firmware y no
> dependen de ningún botón. Lo que se cae son las tres formas de **pedir** un cambio.

**El ciclo degradado es fijo y propio: no hereda el verde configurado en Modo Automático.** Es
deliberado — un verde de 2 minutos con un todo-rojo de 30 s daría un ciclo de 5 minutos, y nadie
espera cinco minutos en un paso alternado sin invadir.

---

## 8. Qué hacer si algo va mal

> 🛑 **AVISO 28/08/2026 — la columna «Qué hacer» de esta tabla se queda casi entera sin actuador.**
> Las filas tachadas describen respuestas que **ya no se pueden ejecutar**. Se conservan porque son
> la lista de emergencias que el reemplazo por Bluetooth **tiene que saber atender**; hasta
> entonces, la respuesta real a las tres primeras es **ir al gabinete**.

| Síntoma | Causa probable | Qué hacer |
|---|---|---|
| ~~La secuencia `A·B·A·B` responde con **ámbar rápido** en vez de 4 destellos~~ 🛑 *(sin mando: el síntoma ya no ocurre)* | Falta alguno de los 5 requisitos | ~~Subir al gabinete y leer el motivo en `CONFIGURACION → MODO DEGRADADO`~~ — **sin pantalla.** El motivo solo se podrá leer cuando la App lo publique |
| ~~La secuencia **no responde nada**~~ 🛑 *(sin mando: no aplica)* | ~~El menú está abierto~~ — **sin menú, la inhibición de secuencias deja de existir** | ~~Subir y salir del menú con `Botón 4`~~ |
| Las dos puntas **ciclan pero desfasadas** | Relojes separados, o una unidad se reinició | ~~`B·B·B` en **ambas** y volver a ámbar.~~ 🛑 **SIN ACTUADOR.** Hoy: `SET_MODO:AMBAR` por Bluetooth **solo en el Maestro**; el Esclavo **no tiene comando**. **No corregir a ojo** *(sigue vigente)* |
| Una punta en **verde** y la otra en **ámbar** | Riesgo residual nº 2 — salida asimétrica | ~~**`B·B·B` inmediatamente en la punta que da verde.**~~ 🛑 **SIN ACTUADOR — y esta es la fila que más duele.** Si la punta en verde es el **Esclavo**, hoy **no hay ninguna orden que lo apague**: lo único disponible es `CMD:FORZAR_ROJO` (`Esclavo/src/bluetooth.cpp:109`), que **no saca del Degradado** — el ciclo por reloj volverá a dar verde en la siguiente fase |
| Pantalla: `Limite 48h sin sync` | Se agotó el límite duro | Es correcto. Hay que **arreglar el radio**, no reactivar el modo. *(Sigue vigente: el tope es del firmware, `modo_degradado.cpp:515`, y no necesita actuador. Con los botones retirados es **la única salida garantizada** del Degradado.)* |

---

## 9. Lo que este procedimiento NO cubre

Escrito aquí porque una limitación documentada vale más que una promesa:

- **No hay prueba de banco ni de campo todavía.** Todo lo anterior está validado en simulador.
- **El Esclavo no tiene receptor de mando** (N-19). Todo lo que este documento dice del mando aplica
  **solo al Maestro**.
- **El estado no sobrevive a un corte de energía** (N-20). `respaldo.cpp` está escrito pero sin
  conectar.
- **La configuración del ciclo se sincroniza pero todavía no se consume** en el cálculo del ciclo
  (N-18): hoy ambas puntas usan los 30/30 fijos compilados. Mientras los dos firmwares sean de la
  misma versión, coinciden — **pero flashear versiones distintas en cada punta rompería la fase sin
  aviso**.
- **El RTC no se ha contrastado contra hora patrón** ni se ha comprobado que conserve la hora tras
  desconectar la alimentación (N-15, N-17).
- 🛑 **Y desde el 28/08/2026, lo mayor: no cubre cómo se ENTRA ni cómo se SALE de este modo.** Las
  cuatro vías que documentaba eran botones y secuencias de mando, y las tres se retiran. **MEDIDO:**
  no existe comando Bluetooth de entrada en ninguna punta, ni de salida en el Esclavo; en el Maestro
  las dos salidas que hay (`SET_MODO:AUTO`, `SET_MODO:AMBAR`) se saltan el todo-rojo de despedida.
  Ver el aviso de cabecera. **Este documento describe hoy un modo al que no se puede llamar y del
  que no se puede volver.**
