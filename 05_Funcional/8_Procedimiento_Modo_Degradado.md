# 🕹️ PROCEDIMIENTO DE CAMPO — MODO DEGRADADO (SFTY-21)

**Documento para el operario de campo y el Ingeniero Funcional**
**Fecha:** 1 de Agosto de 2026 · **Aplica a:** firmware V8.7 (rama `feat/n15-reloj-pantalla-hora`)

---

> # 🛑 AVISO DEL 02/09/2026 — ESTE MODO SE OPERA HOY DESDE LA APP, NO DESDE LOS BOTONES
>
> **Los pulsadores 3 y 4 ya no existen.** Sus pines (`J16` p10 y p12) son entradas de cámara, y el
> firmware devuelve *«no pulsado»* de forma permanente para los dos:
>
> ```
>   Maestro/src/botones.cpp:280-281   bool botonAceptar()  { return false; }
>                                     bool botonCancelar(){ return false; }
>   Esclavo/src/botones.cpp:294-295   identico
> ```
>
> **No intente ejecutar ningún paso que diga «pulse Botón 3», «pulse Botón 4» o «navegue al menú».**
> No hay con qué hacerlo, y el equipo no dará ninguna señal de error: simplemente no pasará nada.
>
> ## Cómo se entra y se sale HOY
>
> | Maniobra | Cómo se hace hoy | Evidencia *(re-medida el 04/09)* |
> |---|---|---|
> | Entrar en Degradado | **App:** `CMD:PIN:1234:SET_MODO:DEGRADADO` | `Maestro/src/bluetooth.cpp:501` |
> | Entrar desde el piso | `A · B · A · B` en el mando de relés | canales `A` (`PB9`) y `B` (`PB13`), **se conservan** |
> | Salir a Automático | **App:** `SET_MODO:AUTO`, o `A · A · A` en el mando | `bluetooth.cpp:444` |
> | Salir a Ámbar | **App:** `SET_MODO:AMBAR`, o `B · B · B` en el mando | `bluetooth.cpp:454` |
> | Volver al MENÚ | **App:** `SET_MODO:MENU` | `bluetooth.cpp:458` |
> | ~~Entrar por pantalla~~ | ⛔ **sin actuador** — necesitaba `Botón 4` y dos `Botón 3` | — |
> | ~~Salir por pantalla~~ | ⛔ **sin actuador** — necesitaba `Botón 3` | — |
>
> **El modo NO está inalcanzable, y esto invierte lo que decía la versión anterior de este aviso.**
> Hasta el 28/08 no existía comando de ida ni de vuelta por Bluetooth y esa era la advertencia
> central. Hoy existen los dos. Se deja escrito el cambio y no se borra el motivo: lo que dejó de
> ser cierto es *«el Degradado queda inalcanzable al retirar los botones»*.
>
> ### 🔴 Lo que la salida por app NO hace, y hay que tenerlo en cuenta en obra
>
> - **Se salta el todo-rojo de despedida.** La salida por `Botón 4` forzaba rojo, esperaba la
>   transición y pintaba *«Vea las dos puntas»* (`modo_degradado.cpp:448-462`). La salida por app
>   pasa de un **verde por reloj** directamente al modo nuevo en la iteración siguiente. **Mire las
>   dos puntas usted antes de dar la orden.**
> - **El Esclavo no tiene `SET_MODO`.** Una salida por app **mueve solo el Maestro**, que es el
>   **Riesgo residual nº 2** de la Sección 6.
>
>   > ✏️ **CORREGIDO EL 04/09 — la lista que había aquí era falsa en dos de sus cuatro entradas.**
>   > Decía que el despachador del Esclavo *«atiende `FORZAR_ROJO`, `SOLICITAR_PASO`, `TEST_LEDS` y
>   > `SET_RTC:`»*. **`FORZAR_ROJO` y `TEST_LEDS` están RECHAZADOS a propósito**, y faltaban las dos
>   > órdenes de ámbar, que son justo las que importan en este procedimiento. **Re-censado sobre
>   > `Esclavo/src/bluetooth.cpp`:**
>   >
>   > | orden | línea | qué hace |
>   > |---|---|---|
>   > | `CMD:AMBAR_EMERGENCIA` | `:381` sin PIN · `:468` con PIN | ✅ ámbar + latch que veta la radio |
>   > | `CMD:PIN:1234:CANCELAR_AMBAR` | `:491` | ✅ **retira** el latch. **Con PIN** |
>   > | `CMD:PIN:1234:SOLICITAR_PASO` | `:532` | ✅ pide al Maestro; no ordena |
>   > | `CMD:PIN:1234:SET_RTC:…` | `:563` | ✅ pone la hora |
>   > | ~~`CMD:FORZAR_ROJO`~~ | `:448` · `:524` | 🛑 `$ERR,…,DESC:RENOMBRADO_USE_AMBAR_EMERGENCIA` |
>   > | ~~`CMD:PIN:1234:TEST_LEDS`~~ | `:550` | 🛑 `$ERR,…,DESC:NO_EN_SERVICIO_USE_EL_MAESTRO` |
>   >
>   > **Importa para este documento**: quien leyera la lista vieja y mandara `FORZAR_ROJO` al Esclavo
>   > para pararlo **no habría parado nada**.
>
> - 🟢 **DECISIÓN DEL 04/09: el cruce se opera desde el MAESTRO.** Que el Esclavo no tenga `SET_MODO`
>   **deja de ser una limitación pendiente de cerrar** y pasa a ser cómo se opera este equipo. **Para
>   este procedimiento eso no lo hace más fácil: lo hace más definitivo.** La entrada al Degradado en
>   el Esclavo sigue dependiendo del `A·B·A·B` del mando —y del receptor RF que **no se ha
>   comprado**—, y esa vía ya no espera un comando Bluetooth que la sustituya. Ver la Sección 9.
> - **El mando de relés sigue siendo la única vía que mueve las dos puntas desde el piso**, y sigue
>   necesitando el receptor RF, que **no se ha comprado**.
>
> ### Qué NO se puede ejecutar de este documento
>
> 1. **Todo paso que mencione `Botón 3` o `Botón 4`.** Están tachados abajo y siguen tachados.
> 2. La pantalla LCD **se sigue dibujando** y sirve para *leer* estado; **no sirve para mandar**,
>    porque no hay con qué confirmar una opción.
> 3. El límite duro de 48 h (Sección 4) sigue vigente: el equipo se rinde solo a ámbar
>    (`modo_degradado.cpp:515`). No es un procedimiento — es un tope.

---

> ## ⚠️ ESTE MODO NO HA PISADO HARDWARE TODAVÍA
>
> Está construido en las dos puntas y validado en simulador, pero **no se ha ejercitado sobre
> tarjetas reales ni en obra**. Hasta que la prueba de banco de la Sección 9 del
> `3_Protocolo_Pruebas_Rigurosas.md` esté firmada, este procedimiento **no autoriza operación en vía
> abierta al tráfico**.
>
> **Las cifras, COPIADAS del acta `evidencia/2026-09-04_compuerta.txt` tal como está HOY, no
> escritas a mano:**
>
> | instrumento | acta del 04/09 (HEAD `6d075a5`) |
> |---|---|
> | simulador funcional | `9/9 PASS` |
> | simulador de repetidor | `10/10 PASS` |
> | arnés de pantalla | `271/271` *(Maestro 145 + Esclavo 126)* |
> | arnés del Degradado a dos puntas | `18/18` |
> | banco por packs | 🔴 **`981/998` · 67 packs PASS, 2 FALLA** — la fila entera está marcada `FALLA` |
> | simulador del puente ESP32 | 🔴 **`ABORTADO`** — `IndexError: list index out of range` |
> | **resumen de la compuerta** | 🔴 **18 PASS · 1 FALLA · 1 ABORTADO** |
>
> 🛑 **LA COMPUERTA NO ESTÁ EN VERDE, Y ESO CAMBIA CÓMO SE LEE TODO ESTE DOCUMENTO.** La revisión
> anterior publicaba aquí `974/974 · 67 packs PASS` y `20 PASS · 0 FALLA · 0 ABORTADO`, citando el
> **mismo nombre de fichero** con HEAD `624eb37`. Ese acta ya no existe: **el fichero se reescribió
> el mismo día con una corrida posterior** y hoy dice otra cosa. Se conserva escrito el número viejo
> porque el fallo no es la cifra, es el mecanismo: **un acta con la fecha en el nombre se puede
> sobrescribir sin que nada avise**, y un documento que la cita por nombre envejece en silencio.
> *(Es la misma forma que este repositorio ya conoce: un hash en un documento caduca solo.)*
>
> **Y el acta trae además su propio aviso, que se copia entero:** *«el árbol tenía cambios sin
> commitear al medir. Estas cifras NO corresponden exactamente a `6d075a5`»*.
>
> 🛑 **Un `ABORTADO` no dice NADA del firmware** —no es un aprobado— y un `981/998` dice que hay
> **17 comprobaciones que no cumplen**. Hasta que las dos filas rojas se cierren, de este documento
> no se puede afirmar que esté verificado ni el modelo; y ni con la compuerta verde estaría
> verificada la tarjeta, porque **este modo no ha pisado hardware**.

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

> ## 🛑 04/09 — LA PRIMERA VUELTA DE ENERGÍA CON ESTE FIRMWARE BORRA EL RELOJ, ASÍ QUE **NINGUNA** DE LAS CINCO SE CUMPLE
>
> **Y no es un fallo: está diseñado así.** `N-133` mete los tiempos del ciclo automático en el
> respaldo con pila, y para eso **cambia el formato** de ese respaldo: la firma sube de `0x5EB1` a
> **`0x5EB2`** (`Maestro/src/respaldo.cpp:76`, idéntico en el Esclavo). Un equipo que arranca con
> este firmware **no reconoce la firma vieja y borra el respaldo entero** — que es lo correcto:
> leer con esta aritmética unos bytes escritos con otra daría un dato que parece bueno.
>
> **Lo que eso le hace a ESTE procedimiento, que es lo que hay que saber antes de subir:**
>
> | condición | tras la primera arrancada |
> |---|---|
> | 1 · reloj puesto en hora | 🛑 **se perdió** |
> | 2 · hubo al menos una sincronización RF | 🛑 **se perdió** |
> | 3 · esa sincronización es de hace menos de 2 h | 🛑 no aplica: no hay ninguna |
> | 4 y 5 · medida de desfase y su tolerancia | 🛑 no hay medida |
>
> **Es decir: el primer intento de entrar en Degradado tras cargar este firmware VA A SER RECHAZADO,
> y el rechazo es correcto.** El `$ERR` de `SET_MODO:DEGRADADO` dirá cuál falta —contesta el motivo
> concreto, `Maestro/src/bluetooth.cpp:501`—, y **quien no sepa esto va a leerlo como una avería del
> equipo o de la radio.**
>
> ✅ **Qué hacer:** poner la hora (`SET_RTC` desde la app en el **Maestro**), **esperar a que haya
> sincronizado con el Esclavo por radio**, y volver a pedir el modo. **Pasa una sola vez**, en el
> primer arranque tras la carga.
>
> ⚠️ **Y lo que sigue sin sobrevivir a un corte, aunque el respaldo esté conectado: la pertenencia
> al Modo Degradado.** No hay registro de *«esta punta estaba en Degradado»*, así que un corte de
> energía **no lo reanuda** — el equipo vuelve a la espera de selección de modo. Es la dirección
> segura, pero **hay que rehacer la entrada en las dos puntas, con su verificación visual**.

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

> ✅ **CÓMO SE HACE HOY (02/09/2026): desde la app, con PIN.**
>
> ```
>   CMD:PIN:1234:SET_MODO:DEGRADADO
> ```
>
> El firmware atiende esta orden en `Maestro/src/bluetooth.cpp:435`, y **la puerta de los 5
> requisitos sigue delante**: si alguno no se cumple, no entra y contesta el motivo concreto
> (`$ERR,CMD:SET_MODO:DEGRADADO,DESC:...`). **Lea la respuesta** — un `$ERR` aquí no es un fallo de
> la app, es el equipo diciéndole cuál de los cinco requisitos falta.
>
> Los pasos de pantalla que van debajo quedan tachados: **no hay `Botón 3` ni `Botón 4`.** Se
> conservan porque describen la doble confirmación que la puerta sigue exigiendo por dentro.

~~Desde la pantalla del gabinete:~~

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
> ⚠️ **La asimetría se conserva por app, pero no en las dos puntas (medido el 02/09).** En el
> **Maestro** entrar exige PIN (`CMD:PIN:1234:SET_MODO:DEGRADADO`) y la puerta de los 5 requisitos
> sigue delante. Salir por app (`SET_MODO:AUTO`, `SET_MODO:AMBAR`) **se salta el todo-rojo de
> despedida** que hacía el `Botón 3`. En el **Esclavo** no hay `SET_MODO` de ninguna clase: entra
> solo por `A · B · A · B`, y sale a ámbar por `CMD:AMBAR_EMERGENCIA`.

**Desde el piso**, con el mando de relés: `A · B · A · B` en menos de 18 segundos.
Confirmación: **4 destellos rojos**. Si en vez de destellos aparece un **ámbar rápido**, la secuencia
fue **rechazada** por alguno de los 5 requisitos.

> ⚠️ **La secuencia existe en el firmware** (`Maestro/src/mando.cpp:204-214`, `Esclavo/src/mando.cpp:148`)
> **y no se puede ejercer todavía: el receptor RF nunca se compró ni se conectó.** Esta vía **nunca
> ha llegado a existir en campo** — ver `04_Manuales/MANUAL_MANDO_4_RELES.md`.
>
> **Es hoy la única forma de meter el Esclavo en Degradado**, porque su despachador de Bluetooth no
> tiene comando de entrada. Vea el Paso 2.

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

> 🛑 **AVISO AL DÍA (02/09/2026) — AL ESCLAVO NO SE LE PUEDE METER EN DEGRADADO DESDE LA APP.**
>
> Esto **no ha cambiado**, aunque el Maestro sí tenga ya su comando. Medido sobre
> `Esclavo/src/bluetooth.cpp`, su despachador acepta exactamente estas acciones y ninguna más:
>
> ```
>   CMD:AMBAR_EMERGENCIA            :315   (sin PIN)
>   CMD:FORZAR_ROJO                 :382   (sin PIN)
>   CMD:PIN:1234:AMBAR_EMERGENCIA   :402
>   CMD:PIN:1234:CANCELAR_AMBAR     :425
>   CMD:PIN:1234:FORZAR_ROJO        :458
>   CMD:PIN:1234:SOLICITAR_PASO     :466
>   CMD:PIN:1234:TEST_LEDS          :484
>   CMD:PIN:1234:SET_RTC:...        :497
> ```
>
> **Ninguna entra en Degradado.** Las dos únicas puertas de entrada del Esclavo son
> `Esclavo/src/mando.cpp:148` —la secuencia `A · B · A · B` del mando de relés— y
> `Esclavo/src/menu.cpp:227`, que necesita `botonAceptar()` y por tanto **está tapiada**.
>
> ### 🔴 Lo que eso significa en obra, sin adornos
>
> **Para poner el Esclavo en Degradado hace falta el mando de relés, y su receptor RF nunca se
> compró.** Mientras eso siga así, este procedimiento **no se puede completar en las dos puntas** —
> y una sola punta en Degradado es peor que ninguna (Sección 6).
>
> - El punto 2 (pantalla + pulsadores) sigue **sin actuador**: no hay `Botón 3` ni `Botón 4`.
> - *"o activar Modo Degradado"* del punto 1 **sigue sin existir**. La inyección de RTC del punto 1
>   sí existe y se conserva.
> - Lo que el Esclavo **sí** acepta ya desde la app, y antes no, es el **ámbar de emergencia** y su
>   revocación. Es el plan de aborto del Paso 3, no una vía de entrada.

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

> ## ✅ EL PLAN DE ABORTO, AL DÍA (02/09/2026)
>
> Esta línea es lo que se hace **cuando las dos puntas no cuadran con el cruce ya dando verdes por
> reloj**. Sus dos vías viejas eran mando y pulsador. Hoy se hace desde la app, **punta por punta**:
>
> | punta | orden | dónde está |
> |---|---|---|
> | **MAESTRO** | `CMD:PIN:1234:SET_MODO:AMBAR` | `Maestro/src/bluetooth.cpp:388` |
> | **ESCLAVO** | `CMD:AMBAR_EMERGENCIA` *(sin PIN)* o `CMD:PIN:1234:AMBAR_EMERGENCIA` | `Esclavo/src/bluetooth.cpp:315` y `:402` |
>
> **El ámbar del Esclavo NO es instantáneo si el Degradado está gobernando la luz, y eso es
> deliberado.** El equipo sale **por todo-rojo**, no de un verde a un ámbar intermitente: saltar
> directo le daría a quien viene lanzado una señal que invita a negociar el paso creyendo que
> todavía tiene prioridad. Puede tardar **de 10 a 90 s**. Lo verá en la respuesta:
>
> ```
>   $ACK,CMD:AMBAR_EMERGENCIA,RESULT:OK                        <- ambar ya puesto
>   $ACK,CMD:AMBAR_EMERGENCIA,RESULT:SALIENDO_TODO_ROJO        <- va en camino, espere
>   $ACK,CMD:AMBAR_EMERGENCIA,RESULT:SALIDA_YA_EN_CURSO        <- ya estaba saliendo
>   $ACK,CMD:AMBAR_EMERGENCIA,RESULT:YA_EN_AMBAR_LATCH_PUESTO  <- ya estaba en ambar
>   $ERR,CMD:AMBAR_EMERGENCIA,DESC:SALIDA_A_ROJO_EN_CURSO_REPITA  <- REPITA la orden
> ```
>
> 🔴 **`SALIENDO_TODO_ROJO` no es `OK`, y la diferencia importa en la calle: el ámbar todavía no
> está puesto.** No se vaya del cruce hasta verlo con los ojos.
>
> **Para revocar ese ámbar** en el Esclavo: `CMD:PIN:1234:CANCELAR_AMBAR` (`bluetooth.cpp:425`).
> Pide PIN a propósito — poner el ámbar es la acción segura; **quitarlo devuelve el cruce a dar
> verdes**, y eso es lo que el PIN custodia.
>
> **Lo que sigue sin existir:** `CMD:FORZAR_ROJO` en el Esclavo (`bluetooth.cpp:382`) **no es ámbar
> y no saca del Degradado**. No lo use como plan de aborto.

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

> 🔴 **CORREGIDO EL 02/09 — el factor de seguridad NO es 2. Es 1,44.**
>
> Este documento decía *«las 48 h dejan factor de seguridad 2 sobre el margen teórico»*. Se midió
> ejecutando el C++ real de las dos puntas a la vez, cada una con su reloj, y la cuenta sale así
> (`Maestro/src/modo_degradado.cpp:39-45`):
>
> | | |
> |---|---|
> | Desfase entre relojes que el cruce **aguanta** | **29 s** |
> | Desfase que el equipo puede **acumular** en 48 h | **20,2 s** *(17,2 s de deriva + 3 s de tolerancia)* |
> | Margen que queda | **8,8 s** → **factor 1,44** |
>
> **Los 48 h no cambian y el despeje de 30 s tampoco** —subirlo alarga el todo-rojo que ve el
> conductor, y esa es una decisión vial, no de firmware—. Lo que cambia es cuánto colchón crea
> usted que tiene: **es la mitad de lo que este documento venía diciendo.** Con 8,8 s de margen, un
> radio caído no se deja para «la semana que viene».
>
> La frontera del sentido malo —**Esclavo atrasado**— es exactamente el despeje de 30 s, y el
> segundo entero con que viaja la hora la deja en 29. El sentido favorable aguanta 35 s porque los
> 4 s de ámbar con que el Esclavo abre su verde protegen **solo en un sentido**.

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

- **Desde el piso:** `A · A · A` en menos de 12 s → **2 destellos rojos**. ✅ **Sigue existiendo.**
- **Desde la app:** `CMD:PIN:1234:SET_MODO:AUTO` — **solo en el Maestro**.
- ~~**Desde la pantalla:** en `CONFIGURACION → MODO DEGRADADO`, `Botón 3` (`3=Salir`).~~ ⛔ sin actuador.

> ✅ **CORREGIDO EL 02/09 — `A · A · A` NO se retiró, y la versión anterior de este documento decía
> que sí.**
>
> El mando de relés **se conserva en sus canales `A` y `B`** (`MANDO_A` = `PB9` = `J16` p5,
> `MANDO_B` = `PB13` = `J16` p8). Lo que se retiró son los pulsadores 3 y 4, que el mando **no
> usaba**. Las tres secuencias siguen enteras en el firmware de las dos puntas:
>
> ```
>   A A A      -> Automatico  Maestro/src/mando.cpp:225-227 · Esclavo/src/mando.cpp
>   B B B      -> Ambar       Maestro/src/mando.cpp:230-234
>   A B A B    -> Degradado   Maestro/src/mando.cpp:204-214 · Esclavo/src/mando.cpp:148
> ```
>
> ⚠️ **Lo que falta no es el firmware: es el receptor RF, que nunca se compró.** ~~Sin él no hay con
> qué generar los pulsos desde el piso. **La secuencia existe y no tiene mando.**~~
>
> 🔧 **Matizado el 04/09 (`N-118`): en BANCO sí hay con qué darlos, con un cable.** Un pulso `A` es
> cerrar un instante **`J16` p5 contra p4**, y un pulso `B`, **p8 contra p7** — los **3,3 V del pin
> contiguo**. 🛑 **NUNCA contra masa:** `J16` tiene **una sola masa en todo el conector** (`p2`), el
> firmware lee estas entradas en `INPUT` pelado y **activo en ALTO** (`346ea5f`), y un cable a masa
> **no produce absolutamente nada** — es además el gesto que precedió al calentamiento del paso 29.
> Lo que sigue faltando es **el receptor**, para darlos por radio desde el piso, y **una tarjeta
> sana** (N-116).
>
> 👁️ **Y no hace falta app ni terminal para saber si el equipo oyó: lo confirma con sus luces**
> (`Maestro/src/mando.cpp:45-47`) — **`A·A·A` → 2 destellos rojos · `B·B·B` → 3 · `A·B·A·B` → 4 ·
> rechazado → ámbar rápido de 2 s**. Se cuentan desde el suelo. ⚠️ **Pruebe DESDE OTRO MODO:** si el
> equipo ya está en el modo que pide la secuencia, `MODO:` no cambia y no se distingue nada; los
> destellos, en cambio, se ven siempre.
>
> **La vía por app no es equivalente a `A·A·A`:** `CMD:PIN:1234:SET_MODO:AUTO`
> (`Maestro/src/bluetooth.cpp:378`) **se salta el todo-rojo de despedida** de
> `modo_degradado.cpp:448-462` y pasa del verde por reloj directo a `modoAutomatico_setup()`. Y **en
> el Esclavo no hay `SET_MODO` de ninguna clase**, así que ejecutar solo el del Maestro produce el
> **Riesgo residual nº 2** (Sección 6) — una punta fuera, la otra dentro.

**Hágalo en las dos unidades**, y luego mire las luces:

```
   A·A·A  ->  2 destellos  ->  esperar ~15 s

     luces CICLANDO  ->  el radio volvió, ya está en automático
     luces en ÁMBAR  ->  sigue muerto; puede volverse al degradado
```

Volver a Automático **no necesita protección**, y por eso la secuencia es corta: si el radio sigue
muerto, el propio sistema se corrige a los ~~12~~ **25 s** (SFTY-6) y cae a ámbar, que es justo donde
se quería estar. **El peor caso de intentar Automático es volver al ámbar.**

> ✏️ **CORREGIDO EL 04/09 — este párrafo publicaba 12 s y son 25 desde `N-71`.** **MEDIDO:**
> `SFTY6_SILENCIO_MS 25000UL`, en `Maestro/include/protocolo.h:149` **y** en
> `Esclavo/include/protocolo.h:149` *(el mismo número en las dos puntas, que es parte de la
> propiedad)*. El manual `1_Manual_Usuario.md §4` ya lo traía bien; **este documento se había quedado
> con el número viejo**, y no es cosmético: quien espere 12 s y no vea ámbar concluirá que el equipo
> no obedeció, cuando lo que le falta es **esperar el doble**. **Cuente 25 s antes de decidir nada.**

### Para irse a ámbar y dejarlo así

- **Desde el piso:** `B · B · B` en menos de 12 s → **3 destellos rojos**. ✅ **Sigue existiendo.**
- `B·B·B` funciona **desde cualquier estado y sin condiciones**. Es la regla que impide que nadie
  quede atrapado con un semáforo en un estado raro a 5 m de altura.
- **Desde la app:** `SET_MODO:AMBAR` en el Maestro · `AMBAR_EMERGENCIA` en el Esclavo.

> ✅ **CORREGIDO EL 02/09 — `B·B·B` tampoco se retiró.** El mando se conserva en `A` y `B`
> (`Maestro/src/mando.cpp:230-234`). Sigue siendo la **salida de emergencia** del sistema, y su
> promesa —*«desde cualquier estado y sin condiciones»*— **sigue siendo cierta en el firmware**. Lo
> que no hay es el **receptor RF**, que nunca se compró: la secuencia existe y no tiene con qué
> generarse desde el piso.
>
> ✅ **Y el ámbar del Esclavo YA NO depende solo del mando.** La versión anterior decía que
> *«el Esclavo no puede irse a ámbar local por orden de nadie»*. Hoy es falso: su despachador de
> Bluetooth atiende **`CMD:AMBAR_EMERGENCIA`** (`Esclavo/src/bluetooth.cpp:315`, sin PIN) y su
> gemelo con PIN (`:402`), y ese ámbar **queda enclavado**: una orden de radio del Maestro no se lo
> quita. Se revoca con `CMD:PIN:1234:CANCELAR_AMBAR` (`:425`), que **sí pide PIN** porque devuelve
> el cruce a dar verdes.
>
> `ambarLocal` —el veto del mando, `Esclavo/src/mando.cpp:132`— sigue armándose solo desde `B·B·B`.
> Son **dos enclavamientos distintos**: el del mando lo pone quien está subido al poste; el de la
> app, quien tiene el teléfono. Ver el manual del mando, §3.1.

> ## 🟢 EL BLOQUE DE ARRIBA ESTÁ CADUCADO DESDE EL 31/08/2026 — y por DOS motivos independientes
>
> Se conserva tachado y no se borra: una casilla que desaparece en silencio se vuelve a proponer.
>
> **(1) EL MANDO DE RELÉS SE CONSERVA**, por decisión del responsable del 31/08, en los canales **A**
> y **B** (`MANDO_A` = `PB9` = `J16` p5, `MANDO_B` = `PB13` = `J16` p8). Se retiran **solo** los
> pulsadores 3 y 4, que son los que las cámaras necesitan. **`A·A·A`, `B·B·B` y `A·B·A·B` siguen
> funcionando**, y con ellos `ambarLocal` y sus tres vetos. El *«sin actuador»* de arriba describía un
> equipo que la decisión del 31/08 ya no va a construir.
>
> **(2) Y aunque no se conservara, el Esclavo SÍ tiene una segunda vía de ámbar** —la tenía ya cuando
> se escribió aquello—: **`CMD:AMBAR_EMERGENCIA` por Bluetooth**, que entra por dos puertas, **con
> PIN y sin PIN** (`Esclavo/src/bluetooth.cpp:315` y `:402`, **MEDIDO el 02/09**; las dos puertas
> llevan el mismo bloque letra por letra, a propósito). Se revoca con `CANCELAR_AMBAR` (`:425`).

### 5.bis 🟠 El ámbar de emergencia desde la app, y la jerarquía de las dos vías

> ✅ **AL DÍA EL 04/09/2026 — `N-106` ESTÁ CERRADO EN EL FUENTE, y este apartado ya NO describe algo
> que el firmware incumple.** Lo que sigue vigente sin cambios es la otra mitad: **NADA DE ESTO HA
> PASADO BANCO.**
>
> ~~*«el firmware de hoy no se comporta como dice este apartado: `CMD:AMBAR_EMERGENCIA` no sale del
> Modo Degradado y contesta `RESULT:OK` pase lo que pase. Es el defecto N-106, abierto»*~~ →
> 🛑 **CADUCADO.** Se tacha en vez de borrarse: quien lo leyera y lo diera por vigente estaría
> desconfiando del botón equivocado, y mandaría a un operario a subir a un poste sin necesidad.
>
> **MEDIDO en el fuente** *(no ejercido en tarjeta)*, `Esclavo/src/bluetooth.cpp`:
>
> * **`salidaDegradadoIniciada()` (`:302-308`)** llama a `degradado_salir()` **y devuelve si la
>   salida arrancó de verdad**. El porqué del envoltorio está escrito en `:293-301` y es la parte
>   reutilizable: `degradado_salir()` es `void` y **abandona en silencio** desde `DEG_INACTIVO` y
>   desde `DEG_SALIENDO`, así que llamarla suelta y contestar `$ACK` detrás **habría sido el mismo
>   «OK mudo»** que este documento denunciaba. Pregunta la **misma** guarda que ella —`DEG_ENTRANDO`
>   o `DEG_ACTIVO`—, no una parecida.
> * **Las dos puertas —sin PIN (`:381`) y con PIN (`:468`)— llevan el mismo bloque letra por letra**,
>   y lo vigilan los packs `esclavo_07` y `esclavo_08`.
>
> **Y el `RESULT` ya no es `OK` pase lo que pase: son cinco respuestas distintas** *(la tabla
> completa con sus `DESC:` sigue viviendo en el Manual 10 §4.5, y aquí sigue sin copiarse — ver más
> abajo el porqué)*.
>
> ⚠️ **Lo que este cierre NO trae, y hay que leerlo antes de cambiar la práctica de campo:** es
> **MEDIDO sobre fichero**, y **nadie lo ha ejercido** ni en tarjeta ni en arnés. La regla de
> `CLAUDE.md` §8.bis —*ver fallar el instrumento antes de fiarse del arreglo*— **no se ha cumplido
> todavía para este camino**. Hasta entonces la instrucción de obra sigue siendo la de siempre y no
> depende de ningún firmware: **verificar las dos puntas con los ojos**.

**La jerarquía, que es decisión del responsable y no una preferencia de diseño:**

| vía | qué es | cuándo se usa |
|---|---|---|
| **La app** (`CMD:AMBAR_EMERGENCIA`) | **la superficie de mando** | siempre que se pueda |
| **El mando, `B·B·B`** | **la vía de último recurso** | cuando no hay teléfono, no hay cobertura, o el ESP32 se colgó |

Que sea la *segunda* no la hace opcional: **es la única que no depende de una radio corta, de una
batería de móvil ni de un accesorio.** Por eso el mando se conserva.

**Y las dos tienen que hacer LO MISMO.** El firmware lo declara por escrito
—`Esclavo/src/bluetooth.cpp:32-39`: *«UNA EMERGENCIA PEDIDA POR BLUETOOTH VALE LO MISMO QUE UNA DEL
MANDO»*— y **hoy no es cierto en Modo Degradado**:

| vía | qué hace hoy en Degradado | ¿sale por el todo-rojo? |
|---|---|---|
| Mando, `B·B·B` | `Esclavo/src/mando.cpp:129-141`: si el Degradado gobierna la luz, `degradado_salir()` | **sí** |
| App, `AMBAR_EMERGENCIA` | ~~`bluetooth.cpp:130-136` y `:171-176`: `semaforo_iniciarFallo()` a secas~~ → ✅ **04/09:** `bluetooth.cpp:402` y `:481` preguntan `salidaDegradadoIniciada()` | ✅ **sí** |

**Decisión del responsable, 31/08:** la vía de la app **sale del Degradado de forma ordenada, igual
que `B·B·B`**, por el todo-rojo de despedida. ✅ **Implementada el 04/09 (`N-106`), MEDIDA en el
fuente y sin ejercer.**

> 🔵 **Y la razón vial va escrita en el propio firmware, que es donde tiene que estar**
> (`Esclavo/src/bluetooth.cpp:368-373`): saltar de un **verde por reloj** directo a ámbar
> intermitente *«le daría a quien ya venía lanzado una señal que invita a negociar el paso mientras
> aún cree tener prioridad»*. Por eso el Degradado entra y sale **siempre** por todo-rojo.

#### Lo que el funcional VE cuando lo pide con el Degradado en marcha

```
   pide AMBAR_EMERGENCIA desde el telefono
        |
        v
   TODO-ROJO de despedida ......... entre 10 y 90 s   <-- NO es un cuelgue
        |                                                 el equipo esta obedeciendo
        v
   AMBAR intermitente, pluma ARRIBA
```

> ⚠️ **Los 10 a 90 s no son un número redondo ni un margen de cortesía.** Salen de
> `Esclavo/src/modo_degradado.cpp:108-111` —`max(cfgDespeje × 1000 ms, 4000 ms)`— y `cfgDespeje` es
> el despeje configurado del cruce, que sólo admite **10 a 90 s** (`Maestro/src/modo_automatico.cpp:34`).
> Es el mismo tiempo que ya cuesta `B·B·B`, y es el margen que garantiza que el tramo quedó vacío.
>
> **Que ese precio se acepte tal cual, se acote o se quite es la fila `R-1` del Manual 10 §4.5.6, y
> la decide el responsable.**

#### Lo que el equipo CONTESTA — no es siempre `OK`, y no se copia aquí

**El Esclavo contesta cosas distintas según el estado en que le llegue la orden**, porque un
`RESULT:OK` sobre un ámbar que todavía tardará 90 s en aparecer es una confirmación de algo que no ha
ocurrido: el técnico se va del poste y el equipo se queda como estaba.

**La tabla completa —los cinco casos, los nombres de `RESULT:` y `DESC:`, y lo que el firmware no
sabe distinguir hoy— vive en un solo sitio:**

📄 **[`10_Manual_Modulo_Bluetooth_Telemetria.md` §4.5](10_Manual_Modulo_Bluetooth_Telemetria.md)**

**Aquí no se copia a propósito.** Duplicarla crearía dos versiones que alguien tendría que
sincronizar a mano, y el día que difieran el técnico de arriba y el de abajo leerían contratos
distintos del mismo comando.

#### 🔴 Riesgo residual nuevo (31/08): entrar en Degradado con el ámbar de la app puesto

**MEDIDO POR LECTURA, y RAZONADO —no ejercido—.** `degradado_entrar()`
(`Esclavo/src/modo_degradado.cpp:212-243`) fuerza todo-rojo en `:224` **sin preguntar por
`bluetooth_ambarEmergencia()`**, y el sostenedor del modo escribe luz por otra puerta —`aplicarLuz()`
desde `degradado_actualizar()`, `main.cpp:363`— **que tampoco lo consulta**.

Consecuencia: si alguien entra en Degradado desde el gabinete mientras hay un ámbar pedido por
teléfono, **la luz sale de ámbar en ese mismo instante**, el latch se revoca solo en la vuelta
siguiente (`bluetooth.cpp:292`), y en la siguiente frontera de fase el cruce puede dar **verde por
reloj** donde alguien había pedido precaución. **El `$ACK` ya se envió, y nada se lo dice a nadie.**

**Esto no lo arregla la decisión del 31/08 por sí sola.** Las tres opciones —rechazar la entrada,
revocar el latch explícitamente, o que el latch vete la luz del Degradado— están escritas con su
consecuencia en el Manual 10 **§4.5.7 (`R-4`)**, y **las decide el responsable**: lo que se elija lo
ve un conductor.

> **Mientras tanto, la regla de campo es la de siempre y sirve exactamente para esto:** *verificar
> con los ojos las dos puntas*, al entrar y al salir. Ver Sección 6, Riesgo 2.

### Checklist de salida

- [ ] Salida ejecutada en el **MAESTRO** — `SET_MODO:AUTO` o `SET_MODO:AMBAR` por app, o `A·A·A` / `B·B·B` con mando. ⚠️ **Por app no hay todo-rojo de despedida**
- [ ] Salida ejecutada en el **ESCLAVO** — `CMD:AMBAR_EMERGENCIA` por app, o `B·B·B` con mando. ⚠️ **No hay salida a Automático en el Esclavo**: por app solo se le puede llevar a ámbar
- [ ] Si salió a ámbar por app en el Esclavo: **`RESULT:OK` visto**, no solo `SALIENDO_TODO_ROJO`
- [ ] **Verificado con los ojos** que **ambas** puntas quedaron en el mismo estado — las dos ciclando
      o las dos en ámbar *(sigue vigente: mirar no necesita actuador)*
- [ ] **Ninguna punta quedó dando verde por reloj mientras la otra parpadea en ámbar** *(sigue vigente)*

> ✅ **Las cinco casillas se pueden marcar hoy** (02/09). En agosto las dos primeras eran
> imposibles, y un checklist con casillas imposibles es una invitación a firmarlo igual.
>
> ⚠️ **La segunda tiene un límite que hay que conocer antes de marcarla:** por app el Esclavo solo
> va a **ámbar**, no a Automático. Si quiere las dos puntas ciclando otra vez, el Esclavo sale del
> Degradado **solo** por el mando (`A·A·A`) o cuando el radio vuelva.

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
| Secuencia de entrada desde el piso | `A · B · A · B` en ≤ 18 s → **4 destellos rojos** | `mando.cpp:204-214` |
| Secuencia a Automático | `A · A · A` en ≤ 12 s → **2 destellos rojos** | `mando.cpp:225-227` |
| Secuencia a Ámbar | `B · B · B` en ≤ 12 s → **3 destellos rojos** | `mando.cpp:230-234` |

> ✅ **CORREGIDO EL 02/09 — las tres secuencias siguen en el firmware.** Una versión anterior de
> este documento las daba por retiradas; el mando **se conserva** en sus canales `A` (`PB9`) y `B`
> (`PB13`). Lo que se retiró son los pulsadores 3 y 4, que estas secuencias **no usan**.
>
> ⚠️ **Pero no se pueden ejercer todavía: el receptor RF nunca se compró.** Es una compra que falta,
> no una función perdida.
>
> Los parámetros de *arriba* —tiempos, tolerancias y el límite de 48 h— viven en el firmware y no
> dependen de ningún botón.

**El ciclo degradado es fijo y propio: no hereda el verde configurado en Modo Automático.** Es
deliberado — un verde de 2 minutos con un todo-rojo de 30 s daría un ciclo de 5 minutos, y nadie
espera cinco minutos en un paso alternado sin invadir.

> 🔵 **Y desde el 04/09 ese argumento es MÁS fuerte, no menos.** El mínimo del Modo Automático
> **subió de 1 a 3 minutos** —verde 3–15 min, rojo 3–15 min
> (`Maestro/src/modo_automatico.cpp:51-53`)—, por decisión vial del responsable: *«tres minutos es
> la mínima distancia de seguridad»*. **Si el Degradado heredase el verde configurado, el ciclo
> mínimo pasaría de los 120 s de ahora a 7 minutos** —2 × (3 min + 30 s)—, y este modo es
> precisamente el que corre **sin confirmación de la otra punta**. Los `30 / 30` fijos se quedan
> como están, y el porqué queda medido en vez de razonado.

---

## 8. Qué hacer si algo va mal

> ✅ **TABLA AL DÍA (02/09/2026).** La columna «Qué hacer» se rehízo: la app tiene hoy órdenes que
> en agosto no existían, incluida la que apaga el Esclavo. **Sin receptor RF, las respuestas por
> mando no se pueden ejercer**; las de app sí.

| Síntoma | Causa probable | Qué hacer |
|---|---|---|
| La secuencia `A·B·A·B` responde con **ámbar rápido** en vez de 4 destellos | Falta alguno de los 5 requisitos | Repita la entrada **desde la app** con `CMD:PIN:1234:SET_MODO:DEGRADADO`: el `$ERR` dice **cuál** de los cinco falta. La pantalla ya no se puede navegar |
| La secuencia **no responde nada** | Sin receptor RF no hay quien genere los pulsos | Use la app. La inhibición por menú abierto ya no ocurre: el menú no se puede abrir |
| Las dos puntas **ciclan pero desfasadas** | Relojes separados, o una unidad se reinició | **Ámbar en las dos, y no corrija a ojo.** Maestro: `CMD:PIN:1234:SET_MODO:AMBAR`. Esclavo: `CMD:AMBAR_EMERGENCIA` |
| 🔴 **Una punta en verde y la otra en ámbar** | Riesgo residual nº 2 — salida asimétrica | **Apague la punta que da verde, ya.** Si es el **Esclavo**: `CMD:AMBAR_EMERGENCIA` (`Esclavo/src/bluetooth.cpp:381`, **no pide PIN** justamente por esto). Si es el **Maestro**: `CMD:PIN:1234:SET_MODO:AMBAR`. **`CMD:FORZAR_ROJO` NO sirve en el Esclavo** (`:448`): esa punta lo **rechaza** con `RENOMBRADO_USE_AMBAR_EMERGENCIA`, así que no para nada y el ciclo por reloj volverá a dar verde en la fase siguiente |
| 🟡 **Puse ámbar de emergencia en el Esclavo y ya no hace falta** | — | `CMD:PIN:1234:CANCELAR_AMBAR` (`:491`). **Pide PIN al revés que el de poner**, porque quitarlo devuelve el cruce a dar verdes. Contesta `RETIRADO`, o `RETIRADO_QUEDA_MANDO` si además hay un ámbar pedido desde el mando: en ese caso **la luz sigue vetada** hasta que alguien haga `A·A·A` |
| ❓ **Estoy delante de un poste y no sé si es el Maestro o el Esclavo** | Módulo Bluetooth recién puesto | El módulo se auto-rotula **`SEM-<serie>-M`** o **`SEM-<serie>-E`**. **Si se anuncia `SEM-SIN-MATRICULA` todavía no lo ha aprendido, y con dos módulos nuevos LOS DOS SE LLAMAN IGUAL.** Déjelo un minuto encendido y **déle una vuelta de energía**: el nombre bueno sale en el arranque siguiente. Mientras tanto, la punta la dice el campo `NODE:` del `$STATUS` en la app, no el nombre Bluetooth |
| Pantalla: `Limite 48h sin sync` | Se agotó el límite duro | Es correcto. Hay que **arreglar el radio**, no reactivar el modo. El tope es del firmware (`modo_degradado.cpp:515`) y no necesita actuador |

> ⏱️ **En la fila roja, cuente con que el ámbar del Esclavo puede tardar de 10 a 90 s**: sale por
> todo-rojo a propósito. `RESULT:SALIENDO_TODO_ROJO` significa *va en camino*, no *ya está*. **No se
> vaya del cruce hasta verlo.**

---

## 9. Lo que este procedimiento NO cubre

Escrito aquí porque una limitación documentada vale más que una promesa:

- **No hay prueba de banco ni de campo todavía.** Todo lo anterior está validado en simulador.
- **El Esclavo no tiene receptor de mando** (N-19). Todo lo que este documento dice del mando aplica
  **solo al Maestro**.
- ~~**El estado no sobrevive a un corte de energía** (N-20). `respaldo.cpp` está escrito pero sin
  conectar.~~ → 🛑 **CADUCADO EL 04/09: `respaldo.cpp` SÍ está conectado.** **MEDIDO:**
  `respaldo_setup()` se llama en las dos puntas —`Maestro/src/main.cpp:77` y
  `Esclavo/src/main.cpp:267`—. Lo que sobrevive al corte no es *«el estado»* en general, y por eso
  la frase corta se sustituye por la lista:

  | qué | registro | unidad |
  |---|---|---|
  | verde y despeje del **ciclo Degradado** | `DR2` / `DR3` | **segundos** |
  | banderas y sello de la sincronización | `DR4`–`DR8` | — |
  | 🆕 **rojo, verde y despeje del ciclo AUTOMÁTICO** (`N-133`, 04/09) | `DR9` / `DR10` | **minutos** el rojo y el verde · **segundos** el despeje |

  > ⚠️ **OJO A LA UNIDAD, y el propio fuente avisa** (`Maestro/include/respaldo.h:54-56`): las dos
  > parejas **no son lo mismo**. Las del Degradado van en segundos; las del Automático, el rojo y el
  > verde van en **minutos**. Confundirlas al leer un valor guardado da un ciclo 60 veces más largo
  > o más corto del que alguien configuró.
  >
  > 🔴 **Y lo que sigue SIN sobrevivir, que es lo que este procedimiento necesitaba:** no hay
  > registro de *«esta punta estaba en Degradado»*. Un corte de energía en Degradado **no lo
  > reanuda**: el equipo vuelve a la espera de selección de modo. Eso es correcto y es la dirección
  > segura —reanudar solo un verde por reloj sin que nadie mire las dos puntas es exactamente lo que
  > este modo existe para evitar—, pero **hay que saberlo antes de irse del cruce**.
- **La configuración del ciclo se sincroniza pero todavía no se consume** en el cálculo del ciclo
  (N-18): hoy ambas puntas usan los 30/30 fijos compilados. Mientras los dos firmwares sean de la
  misma versión, coinciden — **pero flashear versiones distintas en cada punta rompería la fase sin
  aviso**.
- **El RTC no se ha contrastado contra hora patrón** ni se ha comprobado que conserve la hora tras
  desconectar la alimentación (N-15, N-17).
- ⚠️ **Lo que sigue abierto en la entrada y la salida (medido el 02/09):**
  - **El Esclavo no tiene comando Bluetooth de entrada** en Degradado. Su única puerta es la
    secuencia `A · B · A · B` del mando, y **el receptor RF no se ha comprado**. Mientras siga así,
    este procedimiento no se puede completar en las dos puntas.

    > 🟢 **Y al 04/09 esto CAMBIA DE NATURALEZA, no de estado.** La decisión del responsable es que
    > **el cruce se opera desde el Maestro** y **no se hace transparente el mando desde el Esclavo**:
    > o sea que **no viene un `SET_MODO` para el Esclavo**. Lo que era *«falta el comando»* pasa a
    > ser *«la vía es el mando de relés, y hay que comprar su receptor»*. **El bloqueo es el mismo;
    > lo que se sabe hoy es que no se va a resolver por software.**
  - Las salidas por app del Maestro (`SET_MODO:AUTO`, `SET_MODO:AMBAR`) **se saltan el todo-rojo de
    despedida** que sí hacía el `Botón 3`.
  - El ámbar de emergencia del Esclavo **sí existe ya** por app y **sale por todo-rojo**, tardando
    de 10 a 90 s. Ver el aviso de cabecera y la Sección 8.
