# 🕹️ PROCEDIMIENTO DE CAMPO — MODO DEGRADADO (SFTY-21)

**Documento para el operario de campo y el Ingeniero Funcional**
**Fecha:** 1 de Agosto de 2026 · **Última revisión: 7 de septiembre de 2026**
**Aplica a:** firmware V8.7 (rama `feat/n15-reloj-pantalla-hora`)
**Manda sobre este documento:** [`DECISIONES.md`](../DECISIONES.md) — filas **`D-1`**, **`D-16`**,
**`D-17.bis`** y **`D-18`**. Donde este procedimiento y esa tabla no digan lo mismo, **gana la tabla**.

---

> # 🛑 07/09/2026 · LO PRIMERO DE TODO — **HOY EL MODO DEGRADADO NO ENTRA EN NINGUNA DE LAS DOS PUNTAS, Y NO ES UNA AVERÍA**
>
> **Va delante de la orden porque quien la mande va a recibir un `$ERR` y va a concluir que el equipo
> está roto.** No lo está: falta el **requisito 1**, y **hoy no hay camino para cumplirlo**.
>
> **Lo dice el propio firmware, en `Maestro/include/reloj.h`, bajo la marca `D-15`** *(se cita el
> símbolo, no la línea — `grep -n "D-15" 01_Firmware/Maestro/include/reloj.h`)*:
>
> ```text
> CONSECUENCIA MEDIDA, NO DEDUCIDA: reloj_enHora() de esta punta es hoy FALSO SIEMPRE,
> y de esa bandera cuelga la autorizacion del Modo Degradado
> (modo_degradado.cpp: "if (!reloj_enHora()) return MDG_FALTA_HORA;"), la
> sincronizacion horaria por radio (coordinador_sincronizarHora) y la medida de desfase
> (coordinador_medirDesfase). Los tres estan bloqueados
> ```
>
> **La cadena, censada el 07/09. Salida literal, corrida antes de publicarla.** ⚠️ *Tres de las cinco filas son
> COMENTARIOS —los comentarios de este repositorio citan lo que explican, así que el recuento sale
> inflado si no se filtran—; llamada de verdad hay **una**, y la última es la definición:*
>
> ```
> $ grep -rn "reloj_ajustar(" 01_Firmware/Maestro/src/*.cpp
> 01_Firmware/Maestro/src/coordinador.cpp:363:// AJUSTAR HORA solo llama a reloj_ajustar(), y el reloj no avisa a nadie de que le
> 01_Firmware/Maestro/src/modo_hora.cpp:216:      reloj_ajustar(horaActual(), minutoActual(), 0);
> 01_Firmware/Maestro/src/modo_hora.cpp:227:      // N-30: si el ajuste NO prendio -reloj_ajustar() se niega cuando el RTC no
> 01_Firmware/Maestro/src/reloj.cpp:32:// buena, y al apagar y encender volvia a ceros. El motivo es que reloj_ajustar()
> 01_Firmware/Maestro/src/reloj.cpp:280:void reloj_ajustar(uint8_t hora, uint8_t minuto, uint8_t segundo, uint8_t dia) {
>
> $ grep -rn "modoActual_set(MODO_HORA)" 01_Firmware/Maestro/src
> 01_Firmware/Maestro/src/menu.cpp:135:        case 1:  modoActual_set(MODO_HORA);      break;
> ```
>
> 1. El **único** llamador vivo de `reloj_ajustar()` en el Maestro es la pantalla `AJUSTAR HORA`
>    (`modo_hora.cpp`). La rama `SET_RTC` del Bluetooth **se retiró** (`D-15`).
> 2. `MODO_HORA` se arma **sólo** en `menu.cpp`, y el menú se navega con `botonAceptar()`, que es
>    `return false;` desde que `PB14`/`PB15` son cámaras (`D-2`). **Tapiado** (`D-17.bis`).
> 3. Sin hora en el Maestro **no hay sincronización por radio**, así que el Esclavo tampoco la
>    recibe: su `reloj_ajustar()` sólo lo llama el manejador de `CMD_HORA` en `Esclavo/src/main.cpp`.
>
> ## 🔴 07/09 — Y ESTO NO ES UNA CADENA: **EL ESCLAVO ESTÁ BLOQUEADO POR SÍ MISMO, AUNQUE EL MAESTRO SE ARREGLE**
>
> **La cita de arriba es de `Maestro/include/reloj.h` y dice *«de ESTA punta»*.** Leída sola, el punto
> 3 se puede entender como *«si mañana el Maestro tuviera hora, el Esclavo se arreglaría solo»*.
> **No.** El reloj del Esclavo está muerto **por su propio hardware**, y hay que decirlo directo
> porque de esa frase depende que alguien no anote un defecto que no existe.
>
> **Medido sobre `Esclavo/src/reloj.cpp`, salida literal corrida el 07/09 antes de publicarla —
> se citan los SÍMBOLOS, no los números:**
>
> ```
> $ grep -n "arrancarCristal\|LSE_CLOCK\|horaValida =\|bool reloj_enHora" 01_Firmware/Esclavo/src/reloj.cpp
> 33:static bool horaValida = false;
> 63:static bool arrancarCristal() {
> 87:  horaValida = false;
> 91:  if (!arrancarCristal()) return;
> 93:  rtc.setClockSource(STM32RTC::LSE_CLOCK);  // cristal Y2 de 32.768 kHz
> 100:  horaValida = rtc.isConfigured() && (rtc.getYear() >= ANIO_MARCA);
> 119:  rtc.setClockSource(STM32RTC::LSE_CLOCK);
> 122:  horaValida = rtc.isConfigured() && (rtc.getYear() >= ANIO_MARCA);
> 125:bool reloj_enHora() { return horaValida; }
> 217:  horaValida = true;
>
> $ grep -n "DEG_RECHAZO_SIN_HORA" 01_Firmware/Esclavo/src/modo_degradado.cpp
> 197:  if (!reloj_enHora()) return DEG_RECHAZO_SIN_HORA;
> 450:    case DEG_RECHAZO_SIN_HORA:     return "SIN HORA VALIDA";
> ```
>
> ⚠️ **Las cuatro líneas del segundo bloque —`119`, `122` y `217`— son el REINTENTO de N-25 y el
> `reloj_ajustar()`, o sea los DOS caminos por los que `horaValida` podría volverse cierta. Se
> publican enteras a propósito**, porque un `grep` recortado a las líneas que convienen es la misma
> falta que citar una línea caducada: **las dos vuelven a pedir el `LSE`, y las dos se apoyan en
> `rtcOperativo`.** Ninguna es una salida.
>
> **La cadena, dentro de esta punta y sin salir de ella:** `reloj_setup()` sale **antes** de
> `rtc.begin()` si `arrancarCristal()` no ve arrancar el oscilador; `arrancarCristal()` pide el
> `LSE`, que es **`Y2`, el cristal de 32.768 kHz que N-17 dio por no oscilante en banco**; sin él
> `rtcOperativo` se queda en `false`, y con `rtcOperativo` en `false` **ni siquiera `reloj_ajustar()`
> escribe** (`if (!rtcOperativo) return;`). O sea que **`horaValida` no puede volverse cierta por
> ningún camino**, ni por radio, ni por app, ni por reintento de N-25.
>
> 🛑 **Y la consecuencia operativa, escrita como afirmación y no como deducción:**
> **`D-18` está construida —el comando existe, llega y se atiende— y el poste 2 va a contestar que
> NO, correctamente, TODAS las veces.** Su `degradado_comprobar()` abre justamente con esa guarda.
> **Quien pruebe el Modo Degradado del Esclavo sin saber esto lo anotará como defecto del equipo, y
> no lo es.**
>
> ⚠️ **Lo que de esto NO está medido, y va escrito para que nadie lo herede como si lo estuviera:**
> N-17 se diagnosticó sobre **una** tarjeta. El §7.7 del `3_Protocolo_Pruebas_Rigurosas.md` sigue
> diciendo *«hay que hacerlo en las DOS tarjetas: una está diagnosticada, la otra no»*. **El camino
> del código es el mismo en las dos puntas y el bloqueo no depende de cuál sea; lo que sigue
> `SIN VERIFICAR` es si el `Y2` del poste 2 oscila.** Si oscilara, el Esclavo seguiría sin hora
> igual —nadie se la puede mandar hoy—, pero el motivo sería otro.
>
> 🟢 **Lo que lo destraba, y ya está decidido: `DECISIONES.md` fila `D-20` (07/09) — la autoridad de
> la hora pasa al `DS3231` del ESP32, y el STM32 la recibe de su propio módulo.**
> ~~**`D-20` está DECIDIDA y SIN CONSTRUIR:** hoy no corre en ninguna tarjeta.~~ *(11/09: construida
> —extrapolador en `9dd8bbf`, siembra ESP32 → STM32 con las reglas de `D-26` en `68dd2c5`—, **sin banco:
> sigue sin correr en ninguna tarjeta**.)* Ver el bloque *«El poste 2 se pone en hora en la puesta en
> marcha»* más abajo, en la Sección 2.
>
> | punta | lo que va a contestar | motivo del enum |
> |---|---|---|
> | **MAESTRO** | `$ERR,CMD:SET_MODO:DEGRADADO,DESC:Falta: reloj sin poner en hora` | `MDG_FALTA_HORA` |
> | **ESCLAVO** | `$ERR,CMD:SET_MODO:DEGRADADO,DESC:SIN HORA VALIDA` | `DEG_RECHAZO_SIN_HORA` |
>
> ## 🔴 Y LA TRAMPA QUE HACE QUE ESTO PAREZCA OTRA COSA: **LA APP LE VA A ENSEÑAR UNA HORA BUENA**
>
> El campo `HORA:` del `$STATUS` **no lo pone el STM32**. El STM32 emite el hueco `HORA:--:--:--` y
> **el puente ESP32 lo rellena con su propio `DS3231` y recalcula el CRC** (`D-9`, `N-145` —
> `grep -n "HUECO_HORA" 01_Firmware/ESP32_Expansion/src/puente.cpp`). O sea:
>
> 🛑 **Ver `HORA:14:32:10` en la app NO significa que el controlador esté en hora.** Significa que el
> reloj del **accesorio** lo está. El que autoriza el Degradado es el otro, y ése sigue en `false`.
> **No cambie la pila, no cambie el cristal `Y2` y no cargue nada:** este proyecto ya pagó una vez por
> sustituir componentes sanos detrás de un síntoma de reloj.
>
> ## Qué SÍ se puede hacer hoy, y qué NO
>
> | | |
> |---|---|
> | ✅ **Se puede** | parar el cruce: `CMD:PIN:1234:SET_MODO:AMBAR` (Maestro) · `CMD:AMBAR_EMERGENCIA` (Esclavo). **El ámbar por pérdida de radio sigue siendo automático y no depende de nada de esto** |
> | ⛔ **No se puede** | entrar en Degradado, en ninguna punta. **La app no tiene botón que lo arregle** |
> | 🟡 **Quién lo desbloquea** | ~~**el responsable, no este documento.** Es la vía `AB-4` — colgar la hora del STM32 del `DS3231` del puente— y **está abierta**~~ 🟢 **07/09: YA NO ESTÁ ABIERTA. ESA VÍA ES `D-20`, Y ESTÁ DECIDIDA** — la autoridad de la hora es el `DS3231` del ESP32 y el STM32 la recibe de su propio módulo. 🛑 **Pero DECIDIDA no es CONSTRUIDA: hoy no corre en ninguna tarjeta**, así que el `⛔` de la fila de arriba sigue siendo el estado real. Ver `DECISIONES.md` fila `D-20` |
>
> ⚠️ **Todo lo que sigue de aquí abajo describe el modo CORRECTAMENTE y sigue siendo la referencia
> para el día que la puerta se abra.** Lo que no se puede hacer hoy es **ejecutarlo**.

---

> # 🔴 07/09/2026 — LEA ESTO ANTES DE EJECUTAR NADA: LA PREMISA DE ESTE DOCUMENTO CAYÓ
>
> **Este procedimiento se abre en la peor situación del sistema —la radio muerta— así que es el que
> menos puede mentir.** Y hasta hoy mandaba, en cinco sitios distintos, a una vía que no existe.
>
> | lo que este documento repetía | lo que hay en el equipo hoy |
> |---|---|
> | *«El Esclavo no tiene `SET_MODO`»* | **Falso desde el commit `15e8cf3`.** `grep -c "SET_MODO" 01_Firmware/Esclavo/src/bluetooth.cpp` → **10**. La rama viva es `strcmp(accion, "SET_MODO:DEGRADADO")`, con `$ACK,…RESULT:OK`, `$ACK,…RESULT:YA_ACTIVO` y un `$ERR` **por cada motivo** de `degradado_entrar()` |
> | *«para poner el Esclavo en Degradado hace falta el mando y su receptor RF»* | **`D-1`: el mando NO EXISTE.** No hay emisor, no hay pulsadores y el receptor RF **nunca se compró ni se va a comprar**. No es una compra pendiente |
> | *«el cruce se opera desde el Maestro; no viene un `SET_MODO` para el Esclavo»* | **`D-18` (05/09): el Modo Degradado del poste 2 SE PIDE POR APP.** Lo que se retiró fue la llave —el mando—, no la puerta: `degradado_entrar()` ya estaba construido y probado |
>
> ## ✅ EL PROCEDIMIENTO VIGENTE, EN UNA LÍNEA POR PUNTA
>
> 🛑 **Esto es POR DÓNDE se pide, no que hoy entre.** El requisito 1 —reloj del STM32 en hora— **no
> se puede cumplir hoy** y las dos puntas van a rechazar la orden: **lea el recuadro que va antes de
> éste.** Lo de abajo es la referencia para cuando esa puerta se abra.
>
> | punta | entrar en Degradado | salir |
> |---|---|---|
> | **MAESTRO** | **App:** `CMD:PIN:1234:SET_MODO:DEGRADADO` | **App:** `SET_MODO:AUTO` o `SET_MODO:AMBAR` |
> | **ESCLAVO** | **App:** `CMD:PIN:1234:SET_MODO:DEGRADADO` — **misma orden, mismo PIN** (`D-18`) | **App:** `CMD:AMBAR_EMERGENCIA` · el retorno a Automático lo devuelve el Maestro al volver la radio |
>
> 🛑 **Y `D-16`, que es la consecuencia y va escrita aquí porque el operario tiene que saberla antes
> de subir al poste: SIN TELÉFONO NO HAY FORMA DE OPERAR EL EQUIPO.** No es una avería, es una
> propiedad declarada del sistema desde que se retiró el mando. Batería cargada, cable, y conviene un
> segundo terminal emparejado.
>
> ⚠️ **Lo que NO cambia, y por eso vale la pena leer el resto del documento:** los **cinco
> requisitos** de entrada, el todo-rojo obligatorio, el tope de 48 h, el desfase de relojes y los
> riesgos residuales **siguen siendo exactamente los de siempre**. Lo que cambió es **por dónde se
> pide**, no qué hace el equipo.
>
> 🔴 **Y lo que sigue abierto y no lo cierra nadie aquí:** `grep -c reportarEvento
> Esclavo/src/modo_degradado.cpp` → **0** *(re-corrido el 07/09)*. El Esclavo entra en Degradado por
> app y **no publica nada sobre ese estado** más allá del `$ACK` de la orden y del campo `MODO:` de
> **su propio** `$STATUS`.
>
> > ## 🛑 07/09 — Y LA MITAD QUE FALTABA, MEDIDA HOY: **`D-18` NO TIENE CANAL DE VUELTA**
> >
> > **El poste 1 no puede enterarse de que el poste 2 está en Degradado.** Dos medidas:
> >
> > 1. **No hay comando de radio con el que el Esclavo lo anuncie.** El Degradado se pide por
> >    Bluetooth, y el Bluetooth de cada punta es local: **no cruza al otro poste**.
> > 2. **El campo `ESC:` del `$STATUS` del Maestro sólo devuelve COLOR.** Leído hoy de
> >    `coordinador_estadoEsclavo()` (`grep -n "coordinador_estadoEsclavo" 01_Firmware/Maestro/src/coordinador.cpp`):
> >
> >    ```cpp
> >    if (modoActual_get() == MODO_AMBAR) return "AMBAR";
> >    if (estadoC == C_FALLO)             return "?";
> >    return (quienVerde == QV_ESCLAVO) ? "VERDE" : "ROJO";
> >    ```
> >
> >    **Cuatro valores, y ninguno dice `DEGRADADO`.** Además los tres primeros describen lo que el
> >    **Maestro** cree, no lo que el Esclavo hace.
> >
> > **Lo que eso significa en obra, y es lo que hay que llevarse:** para saber en qué modo está cada
> > punta **hay que conectarse a las dos con el teléfono, una por una**. **Ningún tablero del poste 1
> > lo va a decir.** Y por eso la verificación visual de la Sección 3 paso 3 **no es un trámite**: es
> > el único instrumento que ve las dos puntas a la vez.
> >
> > 🟡 **Si el operario necesita ver ese estado de otra forma, es una decisión del responsable** —
> > cuesta un comando de radio nuevo y bytes en una trama que ya está apretada.
>
> ---
>
> **Todo lo que sigue de aquí abajo que hable del mando de relés, del receptor RF o de que el Esclavo
> «no tiene `SET_MODO`» se conserva TACHADO con su motivo, no borrado** — y esa promesa se cumple
> línea a línea, porque un `~~` que falta convierte un archivo histórico en una instrucción.

> # 🛑 AVISO DEL 02/09/2026 — ESTE MODO SE OPERA HOY DESDE LA APP, NO DESDE LOS BOTONES
>
> **Los pulsadores 3 y 4 ya no existen.** Sus pines (`J16` p10 y p12) son entradas de cámara, y el
> firmware devuelve *«no pulsado»* de forma permanente para los dos:
>
> ```
>   Maestro/src/botones.cpp:305-306   bool botonAceptar()  { return false; }
>                                     bool botonCancelar(){ return false; }
>   Esclavo/src/botones.cpp:316-317   identico
> ```
>
> **No intente ejecutar ningún paso que diga «pulse Botón 3», «pulse Botón 4» o «navegue al menú».**
> No hay con qué hacerlo, y el equipo no dará ninguna señal de error: simplemente no pasará nada.
>
> ## Cómo se entra y se sale HOY
>
> | Maniobra | Cómo se hace hoy | Evidencia *(re-medida el 07/09 — por símbolo, no por línea)* |
> |---|---|---|
> | Entrar en Degradado — **MAESTRO** | **App:** `CMD:PIN:1234:SET_MODO:DEGRADADO` | `grep -n 'SET_MODO:DEGRADADO' Maestro/src/bluetooth.cpp` |
> | Entrar en Degradado — **ESCLAVO** | **App:** `CMD:PIN:1234:SET_MODO:DEGRADADO` — **`D-18`, misma orden** | `grep -n 'SET_MODO:DEGRADADO' Esclavo/src/bluetooth.cpp` → rama con `degradado_entrar()` y `$ERR` por motivo |
> | Salir a Automático | **App:** `SET_MODO:AUTO` *(Maestro)* | `grep -n '"SET_MODO:AUTO"' Maestro/src/bluetooth.cpp` |
> | Salir a Ámbar | **App:** `SET_MODO:AMBAR` *(Maestro)* · `CMD:AMBAR_EMERGENCIA` *(Esclavo)* | `grep -n 'AMBAR_EMERGENCIA' Esclavo/src/bluetooth.cpp` |
> | ~~Entrar desde el piso~~ | ~~`A · B · A · B` en el mando de relés~~ ⛔ **`D-1`: el mando NO EXISTE** — ni emisor, ni pulsadores, ni receptor RF. **La secuencia sigue en el código y no hay con qué generarla** | — |
> | ~~Salir con `A·A·A` / `B·B·B`~~ | ⛔ **mismo motivo (`D-1`)** | — |
> | ~~Volver al MENÚ~~ | ~~**App:** `SET_MODO:MENU`~~ ⛔ **`D-17.bis`: la pantalla y el menú se retiran del equipo** | — |
> | ~~Entrar por pantalla~~ | ⛔ **sin actuador** — necesitaba `Botón 4` y dos `Botón 3` | — |
> | ~~Salir por pantalla~~ | ⛔ **sin actuador** — necesitaba `Botón 3` | — |
>
> 🔴 **Las citas `fichero:línea` que traía esta tabla —y las diez de la tabla «Evidencia (re-medida el
> 04/09)»— estaban CADUCADAS: medidas el 07/09, las diez apuntaban a otro sitio.** Se sustituyen por
> el `grep` del símbolo, que es lo que `CLAUDE.md` §4.sexies exige y lo único que sobrevive a que
> alguien inserte veinte líneas encima.
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
> - ~~**El Esclavo no tiene `SET_MODO`.** Una salida por app **mueve solo el Maestro**, que es el
>   **Riesgo residual nº 2** de la Sección 6.~~
>
>   > 🔴 **TACHADO EL 07/09 — `D-18` + medida: `grep -c "SET_MODO" Esclavo/src/bluetooth.cpp` → 10.**
>   > El Esclavo **sí** atiende `CMD:PIN:1234:SET_MODO:DEGRADADO`.
>   >
>   > ⚠️ **Pero el Riesgo residual nº 2 NO desaparece, y por eso esta línea se tacha a medias:** lo
>   > que el Esclavo no tiene es un `SET_MODO:AUTO`. **Una salida a Automático por app sigue moviendo
>   > solo el Maestro.** Para parar el Esclavo la orden es `CMD:AMBAR_EMERGENCIA`, y el retorno a
>   > Automático lo devuelve el Maestro al volver la radio. **La asimetría de salida sigue siendo el
>   > riesgo; la de entrada ya no.**
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
>   > | **`CMD:PIN:1234:SET_MODO:DEGRADADO`** | `grep -n 'SET_MODO:DEGRADADO' Esclavo/src/bluetooth.cpp` | ✅ **AÑADIDA EL 07/09 — `D-18`.** Faltaba en este censo, y es la orden que da nombre a este documento |
>   > | ~~`CMD:PIN:1234:SET_RTC:…`~~ | `:563` | ~~✅ pone la hora~~ 🔴 **NO. `D-15`: esta punta CONSUME la orden EN SILENCIO** y no contesta nada. Quien pone la hora es el **DS3231 del ESP32**, y quien acusa es el puente |
>   > | ~~`CMD:FORZAR_ROJO`~~ | `:448` · `:524` | 🛑 `$ERR,…,DESC:RENOMBRADO_USE_AMBAR_EMERGENCIA` |
>   > | ~~`CMD:PIN:1234:TEST_LEDS`~~ | `:550` | 🛑 `$ERR,…,DESC:NO_EN_SERVICIO_USE_EL_MAESTRO` |
>   >
>   > **Importa para este documento**: quien leyera la lista vieja y mandara `FORZAR_ROJO` al Esclavo
>   > para pararlo **no habría parado nada**.
>
> - ~~🟢 **DECISIÓN DEL 04/09: el cruce se opera desde el MAESTRO.** Que el Esclavo no tenga
>   `SET_MODO` **deja de ser una limitación pendiente de cerrar**… La entrada al Degradado en el
>   Esclavo sigue dependiendo del `A·B·A·B` del mando —y del receptor RF que **no se ha
>   comprado**—, y esa vía ya no espera un comando Bluetooth que la sustituya.~~
> - ~~**El mando de relés sigue siendo la única vía que mueve las dos puntas desde el piso**, y sigue
>   necesitando el receptor RF, que **no se ha comprado**.~~
>
>   > 🔴 **LOS DOS PUNTOS DE ARRIBA ESTÁN DEROGADOS — 07/09.** La decisión del 04/09 duró **un día**:
>   > el 05/09, `DECISIONES.md` `D-1` retiró el mando entero y **`D-18` abrió la puerta por app** en
>   > el mismo día. Los dos hechos van juntos y por eso este bloque no se parchea, se tacha:
>   >
>   > - **El mando no es «una vía que espera una compra»: no existe** (`D-1`). El receptor RF nunca
>   >   se compró y **no es una compra aplazada**.
>   > - **La entrada del Esclavo al Degradado es `CMD:PIN:1234:SET_MODO:DEGRADADO`** (`D-18`).
>   > - **Y ya no hay «vía desde el piso»**: `D-16` — **sin teléfono no hay forma de operar el
>   >   equipo**. Eso es una propiedad declarada del sistema, no una avería.
>
> ### Qué NO se puede ejecutar de este documento
>
> 1. **Todo paso que mencione `Botón 3` o `Botón 4`.** Están tachados abajo y siguen tachados.
> 2. La pantalla LCD **se sigue dibujando** y sirve para *leer* estado; **no sirve para mandar**,
>    porque no hay con qué confirmar una opción.
> 3. El límite duro de 48 h (Sección 4) sigue vigente: el equipo se rinde solo a ámbar. No es un
>    procedimiento — es un tope. 🔴 **07/09: el `grep` que aquí se publicaba —`LIMITE_DURO_MS` sobre
>    `Esclavo/src/modo_degradado.cpp`— DA CERO, y un cero se lee como «no hay».** El símbolo del
>    Maestro es `LIMITE_DURO_MS`; **el del Esclavo se llama `LIMITE_SIN_SYNC_MS`**, y vale lo mismo:
>    ```
>    $ grep -n "LIMITE_DURO_MS"     01_Firmware/Maestro/src/modo_degradado.cpp | head -1
>    120:static const unsigned long LIMITE_DURO_MS = 172800000UL;  // 48 h
>    $ grep -n "LIMITE_SIN_SYNC_MS" 01_Firmware/Esclavo/src/modo_degradado.cpp | head -1
>    42:static const unsigned long LIMITE_SIN_SYNC_MS = 48UL * 3600UL * 1000UL;
>    ```
>    **Un concepto con dos nombres, uno por punta.** Búsquelo por los dos.
> 4. **Todo paso que diga `A·A·A`, `B·B·B` o `A·B·A·B`.** `D-1`: **el mando no existe.** Van tachados
>    abajo, uno por uno.
>
> ### 🛑 Y UNA COSA QUE NO HAY QUE HACER NUNCA, QUE VALE MÁS QUE TODO EL PROCEDIMIENTO
>
> **NO SE CABLEA NADA A `J16` p5 (`MANDO_A`, `PB9`) NI A `J16` p8 (`MANDO_B`, `PB13`).**
>
> **El código del mando sigue vivo y sigue leyendo esos dos pines.** Sus rutinas están entre las
> llamadas a `degradado_salir()` del Esclavo — censado el 07/09 con
> `grep -rn "degradado_salir" 01_Firmware/Esclavo/src/` —, así que **cerrar contactos ahí devuelve
> una salida del Modo Degradado que nadie autorizó**: el equipo abandonaría el modo por su cuenta,
> en mitad de una radio muerta, y el operario vería el cruce cambiar sin haber pedido nada.
>
> **Hoy no pasa porque no hay nada enchufado**, y **eso** es lo que los mantiene callados: los
> `R65`–`R68` de 10 kΩ a masa fijan el reposo en `0 V`. **Dejarlos vacíos no es descuido: es la
> barrera.** *(Y `J16` p1 lleva **12 V crudos** — se tapa en cada equipo que se monte, N-120.)*
>
> ✅ **Las salidas del Degradado que un operario PUEDE provocar son tres, y sólo tres:**
> **(1)** vuelve el enlace y el Maestro manda `PING` / `GO_RED` / `GO_GREEN` ·
> **(2)** `CMD:AMBAR_EMERGENCIA` desde la app ·
> **(3)** vencen las 48 h y el equipo se rinde solo a ámbar.

---

> ## ⚠️ ESTE MODO NO HA PISADO HARDWARE TODAVÍA
>
> Está construido en las dos puntas y validado en simulador, pero **no se ha ejercitado sobre
> tarjetas reales ni en obra**. Hasta que la prueba de banco de la Sección 9 del
> `3_Protocolo_Pruebas_Rigurosas.md` esté firmada, este procedimiento **no autoriza operación en vía
> abierta al tráfico**.
>
> # 🔴 07/09 — LA TABLA DE CIFRAS QUE HABÍA AQUÍ SE RETIRA, Y POR TERCERA VEZ POR EL MISMO MOTIVO
>
> **Aquí se copiaban siete cifras del acta del 05/09** —`20 PASS · 0 FALLA`, banco `1053/1053` sobre
> 74 packs, puente `93/93`—. **Ninguna de las siete es la de hoy**, y el propio documento ya se había
> advertido a sí mismo dos veces en este mismo párrafo. **La cura no es volver a copiarlas: es dejar
> de copiarlas.**
>
> **La cifra sale del acta y sólo del acta**, y el acta se lee así:
>
> ```
> ls -t evidencia/*_compuerta.txt | head -1        # la más reciente
> ```
>
> ⚠️ **Y el nombre del acta es la FECHA, no la corrida: identifica un DÍA.** Comprobado en vivo el
> 07/09 — el mismo fichero se releyó con **dos HEAD distintos** con minutos de diferencia, porque
> había otra corrida en marcha. **Compare el `HEAD:` de la primera línea del acta contra
> `git log --oneline -1` antes de fiarse de una cifra.**
>
> 🛑 **Lo que sí se deja escrito, porque es lo que cambia la lectura de este documento y no es una
> cifra que caduque en el sentido cómodo: EN LA CORRIDA DEL 07/09 LA COMPUERTA NO ESTABA EN VERDE.**
> El acta de ese día cierra en `19 PASS · 1 FALLA · 0 ABORTADO`, con la fila *banco por packs* en
> `FALLA`. **Si el acta que usted lea sigue con una `FALLA`, este documento no puede afirmar que
> esté verificado ni el modelo** — no ya la tarjeta.
>
> 🛑 **LAS TRES FILAS ROJAS DE LA REVISIÓN ANTERIOR ERAN FALSAS, Y SE TACHAN CON SU MOTIVO — 05/09.**
> Aquí ponía ~~`981/998` · 67 packs PASS, 2 FALLA~~, ~~`ABORTADO` del simulador del puente~~ y
> ~~`18 PASS · 1 FALLA · 1 ABORTADO`~~, citando el acta del 04/09 con HEAD ~~`6d075a5`~~. **Medido:
> ese fichero, hoy, dice `HEAD e0e835d` y `20 PASS | 0 FALLA | 0 ABORTADO`, con el banco en
> `1053/1053` sobre 74 packs.** Ninguna de las tres filas rojas existe en ninguna acta del
> repositorio. Las otras cuatro filas —`9/9`, `10/10`, `271/271`, `18/18`— **sí coincidían**, y eso
> es lo que hacía el error difícil de ver.
>
> ⚠️ **Y este documento se había advertido a sí mismo de esto, en este mismo párrafo:** *«un acta
> con la fecha en el nombre se puede sobrescribir sin que nada avise, y un documento que la cita
> por nombre envejece en silencio»*. **Volvió a pasarle.** Escribir el aviso no es aplicarlo: la
> cura es **citar el HEAD al lado de la cifra y releerla del fichero**, que es lo que hace la tabla
> de arriba.
>
> **Y el acta trae además su propio aviso, que se copia entero:** *«el árbol tenía cambios sin
> commitear al medir. Estas cifras NO corresponden exactamente a `aeb6ce7`»*.
>
> 🔴 **Y aquí va el dato que más vale de esta corrección, porque volvió a pasar MIENTRAS se escribía:
> el fichero `evidencia/2026-09-05_compuerta.txt` se reescribió TRES veces el mismo día**, y en una
> de esas versiones intermedias el HEAD era `c954e74`, en otra `e0e835d`. **Un acta cuyo nombre es
> la fecha no identifica una corrida: identifica un día.** Por eso la tabla de arriba lleva el HEAD
> pegado a la cifra — y por eso, si el HEAD de esta línea no coincide con el del fichero, **manda el
> fichero y esta tabla hay que releerla**, no al revés.
>
> 🔴 **LO QUE NO CAMBIA AL PASAR DE ROJO A VERDE, y es lo único que importa aquí: un `20/20` dice
> que los modelos y los arneses de un PC no encuentran nada. NO dice que el firmware funcione en la
> tarjeta, y ESTE MODO NO HA PISADO HARDWARE.** El párrafo anterior concluía *«hasta que las dos
> filas rojas se cierren, de este documento no se puede afirmar que esté verificado ni el modelo»*
> — ya están cerradas, **y la conclusión operativa es exactamente la misma**: sigue sin estar
> verificada la tarjeta. **Verde no es entregable.** Y `ABORTADO` no es `PASS`, aunque hoy no haya
> ninguno: si aparece uno, este documento vuelve a no poder afirmar nada.

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

> 🔴 **CORREGIDO EL 07/09, Y SON DOS CORRECCIONES.** (1) **La columna decía «Qué muestra la pantalla»
> y la pantalla se retiró del equipo** (`D-17.bis`): hoy ese texto sale en el `DESC:` del `$ERR`, que
> es lo que el operario lee en el teléfono. (2) **En el MAESTRO son SEIS, no cinco.** Faltaba
> `MDG_SIN_CONFIG`, y no es un detalle de redacción: es la condición que impide que una punta acepte
> lo que la otra rechaza. Medido sobre `modo_degradado_evaluarEntrada()` y sobre
> `modo_degradado_motivoL1()`/`motivoL2()`, que es de donde el despachador compone el `DESC:`
> (`grep -n "modo_degradado_motivoL1" 01_Firmware/Maestro/src/bluetooth.cpp`).

### 2.1 · En el **MAESTRO** — seis condiciones

| # | Condición | Cómo se cumple | `DESC:` del `$ERR` si falta |
|---|---|---|---|
| 1 | El Maestro tiene el reloj **del STM32** puesto en hora — **no el del módulo** | ~~🛑 **HOY NO HAY CÓMO** — ver el recuadro del principio (`D-15`). 🟢 **Lo abre `D-20`, decidida el 07/09 y SIN CONSTRUIR**~~ 🟢 **11/09: lo abre `D-20` con las reglas de `D-26`, construido en `68dd2c5` (sin banco)**: el ESP32 del Maestro le siembra la hora de su `DS3231` al arrancar, tras cada puesta en hora y cada 5 min. Si falta, alarma `HORA_ESP32` a los 15 min | `Falta: reloj sin poner en hora` |
| 2 | Hubo **al menos una sincronización** por radio con el Esclavo | Ocurre sola al confirmar la hora, y cada hora mientras haya enlace. 🔴 **Hoy no ocurre nunca: la dispara `coordinador_sincronizarHora()`, que abre con `if (!reloj_enHora()) return false;` sobre el reloj bloqueado del requisito 1** | `Falta: nunca hubo sincronizacion RF` |
| 3 | Esa sincronización es **reciente** — menos de **2 h** (`SYNC_FRESCA_MS`) | Basta con que el radio haya estado vivo hace poco | `Falta: la ultima sync es muy vieja` |
| 4 | 🆕 **El Esclavo TIENE el ciclo, y lo acusó** (`coordinador_configConfirmada()`) | El Maestro se lo publica al arrancar, con reintentos | `Falta: el esclavo no tiene el ciclo` |
| 5 | Hay una **medida de desfase** contra el Esclavo | La toma el Maestro por radio (`CMD_DELTA`) | `Falta: sin medida de desfase valida` |
| 6 | Ese desfase está **dentro de ±3 s** (`TOLERANCIA_DESFASE_S`) | — | `Desfase fuera de tolerancia (+-3s)` |

> **Por qué la 4 no se puede quitar de la lista**, y el fuente lo razona donde vive: sin ella *«el
> Maestro aceptaba y daba VERDE por reloj, mientras el Esclavo rechazaba, se quedaba en modo normal y
> caía a ÁMBAR por orfandad»*. **Verde contra ámbar es exactamente lo que este modo existe para
> evitar.**

### 2.2 · En el **ESCLAVO** — 🔴 **NO son las mismas, y este documento decía que sí**

**Medido el 07/09 sobre `degradado_entrar()` y `degradado_textoRechazo()`**
(`grep -n "DEG_RECHAZO" 01_Firmware/Esclavo/include/modo_degradado.h`). Son **seis**, y **tres no
existen en el Maestro**:

| # | Condición | `DESC:` del `$ERR` si falta | ¿la tiene el Maestro? |
|---|---|---|---|
| 1 | reloj propio **del STM32** en hora — 🛑 **hoy `false` SIEMPRE en esta punta, y no por culpa del Maestro: ver el recuadro del principio** | `SIN HORA VALIDA` | sí |
| 2 | el Maestro le mandó la **duración del ciclo** | `FALTA CONFIG CICLO` | equivalente (la nº 4) |
| 3 | ese ciclo **no es cero** | `CICLO EN CERO` | ❌ **no** |
| 4 | hubo **alguna sincronización** en esta sesión | `NUNCA SINCRONIZADO` | sí |
| 5 | esa sincronización **no ha vencido las 48 h** | `SYNC CADUCADA >48h` | ❌ **no** — el Maestro exige **2 h**, no 48 |
| 6 | 🔴 **NO hay un ámbar de emergencia puesto por una persona** | `AMBAR EMERG.PUESTO` | ❌ **no** |

> 🛑 **La nº 6 es la que muerde en obra, y no estaba escrita en ninguna parte de este documento.**
> Es la regla `R-4`, **construida** en el firmware: `if (bluetooth_ambarEmergencia()) return
> DEG_RECHAZO_AMBAR_VIGENTE;`.
>
> **El bucle en el que se cae si no se sabe:** el plan de aborto de este mismo documento manda
> `CMD:AMBAR_EMERGENCIA` al Esclavo. **Ese ámbar queda enclavado**, y a partir de ahí el Esclavo
> **rechaza volver a entrar en Degradado** hasta que alguien mande
> `CMD:PIN:1234:CANCELAR_AMBAR`. No es una avería: la máquina **no revoca sola** lo que puso una
> persona, porque debajo de esa luz puede haber alguien trabajando.
>
> ⚠️ **Y el Maestro no comparte esa guarda, a propósito** — su rama del mando ya resolvía lo mismo de
> otra forma—. **Las dos puntas contestan cosas distintas a la misma orden, y hay que leer cada una.**

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
> ~~✅ **Qué hacer:** poner la hora (`SET_RTC` desde la app en el **Maestro**), **esperar a que haya
> sincronizado con el Esclavo por radio**, y volver a pedir el modo. **Pasa una sola vez**, en el
> primer arranque tras la carga.~~
>
> > 🔴 **TACHADO EL 07/09 — `D-15`: ESA ORDEN YA NO LA ATIENDE EL STM32, ASÍ QUE ESE «QUÉ HACER» NO
> > ARREGLA NADA.** `SET_RTC` lo consume y lo acusa el **puente ESP32** (`NODE:PUENTE`), que pone la
> > hora en **su** `DS3231`; el reloj del STM32 —el que autoriza este modo— **se queda igual**. Ver
> > el recuadro del principio del documento. **Lo que aquí decía «pasa una sola vez» hoy es
> > permanente, y no lo cierra ningún procedimiento de campo.**
>
> ~~⚠️ **Y lo que sigue sin sobrevivir a un corte, aunque el respaldo esté conectado: la pertenencia
> al Modo Degradado.** No hay registro de *«esta punta estaba en Degradado»*, así que un corte de
> energía **no lo reanuda** — el equipo vuelve a la espera de selección de modo. Es la dirección
> segura, pero **hay que rehacer la entrada en las dos puntas, con su verificación visual**.~~
>
> > 🔴 **TACHADO EL 07/09 — ERA FALSO, Y EN EL SENTIDO PELIGROSO: EL EQUIPO SÍ REANUDA SOLO.**
> > Medido en las dos puntas, `N-20`:
> >
> > **Salida literal del 07/09** *(cuatro de las ocho filas son comentarios; las llamadas vivas son
> > las de `main.cpp` de cada punta)*:
> >
> > ```
> > $ grep -rn "reanudarTrasCorte" 01_Firmware/Maestro/src 01_Firmware/Esclavo/src
> > 01_Firmware/Maestro/src/main.cpp:95:  const bool reanudarDegradado = modo_degradado_reanudarTrasCorte();
> > 01_Firmware/Maestro/src/modo_degradado.cpp:153:// N-20: lo pone modo_degradado_reanudarTrasCorte() y lo consume el PRIMER
> > 01_Firmware/Maestro/src/modo_degradado.cpp:326:bool modo_degradado_reanudarTrasCorte() {
> > 01_Firmware/Maestro/src/modo_degradado.cpp:409:  // mismo limite de 48 h, en modo_degradado_reanudarTrasCorte().
> > 01_Firmware/Esclavo/src/main.cpp:309:  // degradado_reanudarTrasCorte(); aqui no se decide nada. Si devuelve false, el
> > 01_Firmware/Esclavo/src/main.cpp:311:  degradado_reanudarTrasCorte();
> > 01_Firmware/Esclavo/src/modo_degradado.cpp:87:// (degradado_reanudarTrasCorte()). El Maestro, en cambio, contrasta las dos fuentes
> > 01_Firmware/Esclavo/src/modo_degradado.cpp:301:bool degradado_reanudarTrasCorte() {
> > ```
> >
> > **`respaldo_degradadoActivo()` guarda que esta punta estaba dentro**, y si al arrancar siguen
> > vigentes las tres condiciones —reloj en hora, ciclo acordado en la pila, y sincronización fechable
> > por debajo de las 48 h— **el equipo vuelve al Modo Degradado por su cuenta, sin que nadie lo
> > pida**. Arranca en todo-rojo, igual que en la entrada normal; **no salta a verde**.
> >
> > 🛑 **Lo que eso cambia en obra, y por qué se corrige delante de la acción:** este documento le
> > decía al operario que tras un corte tendría que **rehacer la entrada con su verificación visual**.
> > **No se la van a pedir.** Si una punta se reinició y la otra no, el cruce puede volver a dar
> > verdes por reloj **sin que nadie haya mirado las dos puntas** — que es justo lo que la Sección 3
> > paso 3 existe para impedir. **Tras cualquier corte de energía en Degradado, verifique las dos
> > puntas con los ojos aunque no se lo pidan.**
> >
> > ⚠️ **Y lo que de la frase tachada SÍ era cierto:** si alguna de las tres condiciones no se
> > cumple, el firmware **borra el indicador** y arranca normal — de modo que un corte largo, o un
> > reloj perdido, sí devuelven el equipo a la espera. **Las dos cosas pasan, y no se puede saber cuál
> > desde el suelo sin mirar `MODO:` en el `$STATUS`.**

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

Cuarenta segundos es **más del todo-rojo entero**. ~~La hora se cuadra en el Maestro y **viaja por
radio** al Esclavo~~; así, el día que el radio muera, el desfase arranca en ~0 de verdad y no por
procedimiento.

> ## 🟢 07/09 — EL PRINCIPIO DE ESTE APARTADO SIGUE VIGENTE. EL MECANISMO QUE DESCRIBE, NO
>
> **Lo que este apartado dice y `DECISIONES.md` `D-20` confirma palabra por palabra: UNA SOLA
> FUENTE, Y ESA FUENTE ES EL MAESTRO.** El ejemplo de los 40 s de arriba es exactamente el motivo
> por el que se decidió así, y no se toca:
>
> > *«**El Maestro manda la hora y el Esclavo hace caso siempre. Hay UNA sola fuente**, así que no
> > hay desfase inicial que acotar.»* — `DECISIONES.md`, fila `D-20`, 07/09
>
> 🔴 **Lo que sí cambia es el CAMINO, y por eso la última frase va tachada:** la hora ya no se
> escribe en el RTC del STM32 del Maestro para que viaje al RTC del STM32 del Esclavo. **La
> autoridad es el `DS3231` del ESP32**, y el recorrido decidido es:
>
> ```
>    app  ->  ESP32 Maestro  ->  STM32 Maestro  ->  RADIO  ->  STM32 Esclavo  ->  ESP32 Esclavo
>                (DS3231)         [cartero]                     [cartero]          (DS3231)
> ```
>
> **Los dos ESP32 NO se hablan entre sí.** El único enlace entre postes sigue siendo la radio entre
> los STM32, así que **los STM32 quedan de carteros de la hora, no de dueños**.
>
> ~~🛑 **`D-20` ESTÁ DECIDIDA Y SIN CONSTRUIR. Nada de ese recorrido corre hoy en ninguna tarjeta.**~~
> 🟢 **11/09: construido en `68dd2c5` con las reglas de `D-26`, SIN BANCO — nada de esto ha corrido
> todavía en una tarjeta.** De lo que faltaba según `roadmap.md` §3.4.bis: el extrapolador existe
> (`9dd8bbf`), el mando **ESP32 → STM32** existe (`CMD:HORA_ESP32`, `siembra.cpp`, cada 300 s); **el
> cómputo de las 48 h sigue saliendo del contador crudo del RTC del STM32** (`N-162` `H8`). **Y el último
> salto del dibujo, `STM32 Esclavo -> ESP32 Esclavo`, NO existe**: `D-26` lo deja como mejora.
>
> ~~🔴 **Y la consecuencia dura, que es la que contradice a otra página de ESTE MISMO documento: LA
> APP NO PONE LA HORA EN EL POSTE 2. NUNCA.** Un `SET_RTC` dirigido al Esclavo **se rechaza**: no es
> una sincronización, **es una segunda fuente** — y una segunda fuente es justo el escenario de los
> 40 s de arriba, con otro nombre.~~ 🛑 **CADUCADO** (ya tachado en la propia fila `D-20` el 07/09 por
> la noche: *«la barrera es la SOBREESCRITURA»*). **Lo vigente, `D-26` (3), 11/09: con radio el Esclavo
> hace caso a la hora del Maestro —y la radio la sobrescribe cada 5 min—; SIN radio (25 s), toma la de
> su propio ESP32**, que el técnico le pone desde el teléfono en su gabinete (`D-26` (5)). El escenario de
> los 40 s de arriba **sigue siendo el riesgo** —al perder la radio, el Esclavo salta a lo que tenga su
> `DS3231`— y lo acotan dos cosas: **ponerle la hora a los dos en la puesta en marcha**, y la regla
> (4), abajo.
>
> 🔴 **`D-26` (4) — lo que se ve EN DEGRADADO cuando la hora salta.** Si una hora nueva mueve el reloj
> de una punta **más que el margen del cruce** —el despeje menos 1 s: **29 s** con el despeje de 30 s—,
> esa punta **pasa por ROJO** (vuelve a su todo-rojo de entrada y lo cuenta entero) antes de volver a
> su ciclo, **nunca directo a verde**, y lo deja en el diario: `$EVENT,…,ORIGEN:DEGRADADO,
> DETALLE:SALTO_DE_HORA_POR_ROJO`. Un salto menor —la corrección normal de cada 5 min— se aplica sin
> parar. ⚠️ **Lo que NO cura** (`N-162` `H1`, `roadmap.md` §3.16): tras el rojo, cada punta sigue con
> SU hora; si difieren más que el margen, **el ciclo solapa en cada vuelta**. El caso real: el cable
> interno del Maestro (`J17`) lleva horas mudo —alarma `HORA_ESP32,CAUSA:J17_MUDO`—, su hora corre sin
> corregir y cae la radio. **La protección (`D-21` (1)) está en construcción y no está en este
> firmware.** Detalle para el técnico: `14_Manual_App_Movil_IOT_VIAL.md` §5.3.bis.
>
> ⚠️ **La instrucción del «Paso 2 — Activar en el ESCLAVO» que manda `Ajustes / RTC` →
> `[ 🚀 Inyectar en Esclavo ]` queda derogada por esto, y allí va tachada.** Estaba dentro de este
> mismo documento, numerada, en el procedimiento de emergencia y sin tachar: **decía lo contrario
> que este apartado y nadie lo había cruzado.**

> ## 🛑 LA REGLA ~~NUEVA QUE SALE DE `D-20`~~ DE CAMPO (`D-26`, 11/09): **EL POSTE 2 SE PONE EN HORA EN LA PUESTA EN MARCHA** ~~— NO DURANTE LA AVERÍA~~ **— Y CON LA RADIO CAÍDA, ALLÍ MISMO**
>
> ~~Con `D-20` dentro, el **único** camino al reloj del poste 2 pasa por el Maestro y por **la radio**.
> Y la radio se cae justo cuando hace falta el Modo Degradado, que es el modo que **exige hora**.~~
> *(Caducado por `D-26` (3): sin radio, el poste 2 toma la hora de su propio ESP32.)*
>
> ✅ **El `DS3231` del poste 2 tiene pila y conserva la hora que ya tenía.** *Perder la radio no es
> perder la hora.*
>
> 🛑 **Por eso se pone ANTES** ~~**. Tres momentos, y son los únicos:**~~ **—en estos tres momentos— y
> además cuando salta la alarma de radio (`FALLO_RF`)**, conectándose a ese poste:
>
> | cuándo | por qué |
> |---|---|
> | **en la puesta en marcha** del cruce | es la única visita en la que la radio está garantizada |
> | **al cambiar la `CR2032`** del módulo del poste 2 | el reloj arranca de cero |
> | tras cualquier **`OSCILADOR_PARADO_CAMBIE_PILA`** en ese poste | el `DS3231` avisa de que dejó de contar |
>
> ~~**Si se llega al poste 2 con la radio ya muerta y su reloj perdido, no hay procedimiento de campo
> que lo arregle** — y este documento no se inventa uno. Se anota y se sube con el radio arreglado.~~
> 🔵 **11/09 (`D-26` (5)): SÍ lo hay** — se le pone la hora desde el teléfono conectado a ese poste, y
> sin radio entra. ⚠️ **Lo que esa hora NO arregla:** los rechazos `NUNCA SINCRONIZADO` y `SYNC
> CADUCADA >48h` del Degradado del poste 2 —esa hora no renueva la marca de sincronización, que sólo
> la da el Maestro por radio—. Para eso sigue haciendo falta la radio.

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
> despedida** que hacía el `Botón 3`. ~~En el **Esclavo** no hay `SET_MODO` de ninguna clase: entra
> solo por `A · B · A · B`, y sale a ámbar por `CMD:AMBAR_EMERGENCIA`.~~
>
> > 🔴 **TACHADO EL 07/09 (`D-18`).** El Esclavo entra con **la misma orden y el mismo PIN que el
> > Maestro**: `CMD:PIN:1234:SET_MODO:DEGRADADO`. La asimetría que **sí** se conserva es la de
> > **salida**: el Esclavo no tiene `SET_MODO:AUTO`, y de él se sale a ámbar con
> > `CMD:AMBAR_EMERGENCIA`.

~~**Desde el piso**, con el mando de relés: `A · B · A · B` en menos de 18 segundos.
Confirmación: **4 destellos rojos**. Si en vez de destellos aparece un **ámbar rápido**, la secuencia
fue **rechazada** por alguno de los 5 requisitos.~~

> 🔴 **TACHADO EL 07/09 — `D-1`: NO HAY MANDO, NO HAY PISO, NO HAY SECUENCIA QUE PULSAR.** No es que
> falte comprar el receptor: **el mando no existe y no se va a comprar**. La secuencia `A·B·A·B`
> sigue viva en `Esclavo/src/mando.cpp` y en `Maestro/src/mando.cpp` **a propósito** —retirar el
> armador de `mando_ambarLocal()` dejaría el veto de SFTY-21 abierto, no inerte (`D-1`)—, pero
> **nada puede generar esos pulsos**.
>
> ✅ **Lo que se hace en su lugar, y es más corto:** `CMD:PIN:1234:SET_MODO:DEGRADADO` desde la app,
> **en cada una de las dos puntas**. Y si el equipo rechaza la entrada, **el `$ERR` dice cuál de los
> cinco requisitos falta** — que es más de lo que daba el ámbar rápido del mando.

### Paso 2 — Activar en el **ESCLAVO**

> ## ✅ 07/09 — NO HAY QUE SUBIR AL GABINETE. SE HACE DESDE EL SUELO, CON EL TELÉFONO
>
> **`CMD:PIN:1234:SET_MODO:DEGRADADO` al `📡 ESCLAVO (Poste 2)`** — `DECISIONES.md` `D-18`.
> Es la misma orden que en el Maestro. Respuestas posibles, leídas del fuente el 07/09:
>
> | respuesta | qué significa |
> |---|---|
> | `$ACK,CMD:SET_MODO:DEGRADADO,RESULT:OK` | entró. Empieza el **todo-rojo obligatorio** de entrada |
> | `$ACK,CMD:SET_MODO:DEGRADADO,RESULT:YA_ACTIVO` | **ya estaba dentro**: esta pulsación no encendió nada. No repita |
> | `$ERR,CMD:SET_MODO:DEGRADADO,DESC:<motivo>` | **rechazado**, y el motivo dice cuál falta. 🔴 **07/09: NO es «la misma tabla que enseñaba el gabinete» del Maestro — el Esclavo tiene la SUYA, con seis motivos distintos. Ver §2.2**, y en particular `AMBAR EMERG.PUESTO`, que ninguna versión anterior de este documento mencionaba |
>
> 🔴 **Y lo que esta orden NO puede comprobar, escrito para que no se lea como que sí:** que el
> Maestro haya dejado de gobernar. Con el Maestro vivo, su `CMD_PING` cada 3 s saca al Esclavo del
> Degradado, **y el operario ve el equipo obedecer y volverse atrás solo**. Eso no es una avería: es
> que este modo es para cuando la radio está muerta. **Mire el campo `MODO:` del `$STATUS`**, que
> pasa a `DEGRADADO` y vuelve a `SUBORDINADO`.
>
> ~~**El Esclavo no tiene receptor de mando de relés** (pendiente **N-19**). La tarjeta ya trae las
> cuatro entradas (`PB9`, `PB13`, `PB14`, `PB15`); falta comprar e instalar el receptor.~~
> 🔴 **TACHADO EL 07/09 — `D-1`: no falta comprar nada. El mando no existe y no se va a comprar.**
> Y `PB14`/`PB15` **ya no son entradas de mando**: son **las dos cámaras** (`D-2`).
>
> **Soporte Bluetooth desde el Suelo (V9.0):** Con el módulo Bluetooth USART1 y la App Móvil
> instalada en el celular del operario, ~~la activación y~~ sincronización del Degradado en el Esclavo
> **se realiza directamente desde el suelo**, sin necesidad de subir al gabinete con escalera.
> ~~Además, la App incluye el **Modo Courier RTC**, que permite capturar la hora y ciclo en el Maestro,
> viajar hasta el Esclavo y aplicar la sincronización compensando automáticamente el tiempo de viaje
> con error inferior a 0.1 s — corregido el 07/09 a: con resolución de 1 s, que es la del dato de
> entrada, y el error no está medido.~~
>
> > # 🛑 EL **MODO COURIER RTC** SE TACHA ENTERO — 07/09, `DECISIONES.md` FILA `D-20`
> >
> > **No se aplaza: muere.** Y no por una limitación técnica, sino porque es **exactamente lo que
> > `D-20` prohíbe**: el Courier consiste en *«capturar la hora en el poste 1, viajar al poste 2 y
> > aplicarla allí»* — o sea, **poner la hora en el poste 2 desde el teléfono**. Eso es una **segunda
> > fuente**, y `D-20` decidió que hay **una sola**.
> >
> > **La corrección del error de `0,1 s` de más abajo sigue siendo válida como lección** —una cifra
> > huérfana que nadie midió— **y por eso el bloque no se borra.** Lo que cambia es que ya no hace
> > falta medir ese error: **la maniobra entera no se va a hacer.**
> >
> > ⚠️ **Lo que esto NO retira, para que nadie lo borre de paso:** el `DS3231` del ESP32 **del
> > Esclavo sigue haciendo falta** (son dos, uno por poste). Es el que **conserva con pila** la hora
> > que ya tenía cuando se cae la radio. Retirar el Courier no retira el reloj.
>
> > 🔴 **TACHADO EL 07/09 — «ERROR INFERIOR A 0,1 s» ERA UNA CIFRA HUÉRFANA, Y ADEMÁS IMPOSIBLE.**
> > Es `A-7` otra vez —el «~1 s» del relé que nos inventamos y luego nos citamos—, esta vez en el
> > documento con el que un técnico decide si la hora quedó puesta.
> >
> > **(1) No existe en ningún otro sitio del repositorio.** Re-corrido antes de publicarlo:
> >
> > ```
> > $ grep -rn "0,1 s\|0\.1 s" --include=*.md 05_Funcional 04_Manuales
> > 05_Funcional/8_Procedimiento_Modo_Degradado.md:635:> con error inferior a 0.1 s.
> > ```
> >
> > **Una sola línea, y era ésta.** Cero en el firmware, cero en la app: **nadie la midió — se
> > escribió.**
> >
> > **(2) Y no se puede sostener, no sólo está indocumentada.** El Courier trabaja con la hora del
> > campo `HORA:` del `$STATUS`, cuya **resolución es el segundo entero** (`18:25:00`), y
> > `App_Semaforo/js/courier_rtc.js` la parte con `/^(\d{1,2}):(\d{2}):(\d{2})$/` y la aplica con
> > `dateObj.setHours(hh, mm, ss, 0)` — **los milisegundos se fuerzan a 0**. **Un error «inferior a
> > 0,1 s» no se puede medir con un reloj que sólo publica segundos.**
> >
> > ✅ **Lo que sí se puede afirmar, y es lo que queda escrito:** el Courier **compensa el tiempo de
> > traslado**, y su resolución es la del campo `HORA:` — **1 s**. **Cuánto error deja de verdad
> > sigue `SIN MEDIR`**, y no se sustituye un número inventado por otro.
> >
> > ⚠️ **Y hoy esto es teórico por otro motivo:** con `D-15`, quien recibe y acusa la hora es el
> > **`DS3231` del ESP32** ~~de cada poste~~ — **no el STM32**, que es el que autoriza este modo. Ver
> > el recuadro del principio del documento.
> >
> > > ~~🔴 **«DE CADA POSTE» SE TACHA — 07/09, `D-20`.** Esa frase describe **dos fuentes de hora, una
> > > por poste**, que es la arquitectura que `D-20` anula. **Quien recibe y acusa un `SET_RTC` es el
> > > `DS3231` del ESP32 del MAESTRO, y sólo ése.** El del Esclavo **también existe y también hace
> > > falta** —conserva la hora con pila cuando se cae la radio—, pero **la recibe de su propio
> > > STM32**, no del teléfono.~~
> > >
> > > ~~🛑 **Y hoy el firmware todavía hace lo que la frase tachada decía**: el puente es el mismo
> > > binario en los dos postes y atiende el `SET_RTC` venga de donde venga. **`D-20` es una decisión
> > > sin construir; el rechazo del poste 2 hay que escribirlo.**~~
> > >
> > > 🔵 **11/09 — «de cada poste» VUELVE A SER CIERTO (`D-26`, construida en `68dd2c5`, sin banco).**
> > > El `SET_RTC` lo recibe y lo acusa el `DS3231` del ESP32 **de cada poste**; con radio, la
> > > controladora del poste 2 hace caso a la hora del Maestro, y **sin radio toma la de su propio
> > > ESP32**. El `DS3231` del Esclavo **no** la recibe de su STM32 (la radio no lo escribe). **No hay
> > > rechazo que escribir.**
>
> ~~🛑 **28/08/2026 — «la activación» se tacha: MEDIDO, no existe.**~~ 🟢 **REVERTIDO EL 07/09:
> la activación por Bluetooth del Degradado en el Esclavo YA ESTÁ CONSTRUIDA** (`D-18`, 05/09). El
> aviso del 28/08 era cierto el día que se escribió y decía bien lo que había que hacer: *«es la
> funcionalidad que el reemplazo tiene que **construir**»*. **Se construyó.**
>
> ⚠️ **Lo que del aviso del 28/08 sigue siendo cierto y cambió por otro motivo:** la
> **sincronización** por `SET_RTC:` **ya no la hace esta punta** — `D-15`, ver la nota del reloj más
> abajo.

En la App Móvil:

1. **Entrar en Degradado (desde el suelo):** conectarse al `📡 ESCLAVO (Poste 2)` y mandar
   **`CMD:PIN:1234:SET_MODO:DEGRADADO`** (`D-18`).
2. ~~**Poner la hora:** `Ajustes / RTC` → `[ 🚀 Inyectar en Esclavo ]`. ⚠️ **Quien la recibe y la
   acusa es el ESP32 de ese poste, no el STM32** (`D-15`); para **leerla sin cambiarla**,
   `CMD:LEER_RTC` (`D-17`).~~

   > # 🔵 11/09 — `D-26`: LO QUE SE HACE HOY EN ESTE PASO
   >
   > **Si al poste 2 hay que ponerle la hora** —es lo que `D-26` (5) manda cuando salta la alarma de
   > radio—, **se hace con «⏱️ Sincronizar»** (pestaña **Técnico**) conectado a ese poste, **no con el
   > Courier** (`🚀 Inyectar en Esclavo`, que sigue derogado por su viaje cronometrado). Sin radio, su
   > controladora la toma (construido en `68dd2c5`, **sin banco**). ⚠️ **Si se hace con el Degradado ya
   > puesto y la hora salta más de 29 s, esa punta pasa por rojo** antes de volver a su ciclo
   > (`SALTO_DE_HORA_POR_ROJO`, `D-26` (4)). ⚠️ **Y esa hora no renueva la sincronización del Maestro**: no
   > cura `NUNCA SINCRONIZADO` ni `SYNC CADUCADA >48h`. El recuadro de debajo es del 07/09 y **su núcleo
   > está caducado**; se conserva tachado en lo que caducó.
   >
   > # ~~🛑 TACHADO EL 07/09 — `DECISIONES.md` FILA `D-20`: **LA APP NO PONE LA HORA EN EL POSTE 2. NUNCA.**~~
   >
   > **Era una instrucción activa, numerada, dentro del procedimiento de emergencia y sin tachar**, y
   > decía lo contrario que el apartado *«La hora se pone UNA sola vez, y solo en el Maestro»* de la
   > Sección 2 de este mismo documento. **La contradicción vivía dentro del fichero y nadie la había
   > cruzado.** Manda `D-20`, y manda el apartado que ya decía el principio bueno.
   >
   > ~~**Por qué, en una línea:** poner la hora en el poste 2 desde el teléfono **no es una
   > sincronización — es una SEGUNDA FUENTE**. Es exactamente el escenario de los 40 s de desfase que
   > la Sección 2 describe, sólo que con dos `DS3231` en vez de dos pantallas.~~ *(Caducado: `D-26` lo
   > acepta con su cota —la alarma de radio y la regla (4)—.)* Cierra de paso el **rejuvenecimiento** del
   > límite de 48 h, que se conseguía poniendo ese reloj hacia atrás *(11/09: sigue cerrado por otro
   > camino — la hora del ESP32 del poste 2 no renueva la marca de sync)*.
   >
   > ~~✅ **Lo que se hace en su lugar:** la hora se pone **una sola vez y sólo en el poste 1**, y llega
   > al poste 2 por el Maestro y por la radio. **En la puesta en marcha, no durante la avería** — ver
   > la regla al final de la Sección 2.~~
   >
   > ~~🛑 **PERO ESO NO ESTÁ CONSTRUIDO, Y AQUÍ ES DONDE MÁS IMPORTA DECIRLO.** `D-20` está **decidida
   > el 07/09 y sin una línea de firmware detrás**: no existe el mando `ESP32 → STM32` que siembre la
   > hora, y **el puente del poste 2 hoy sigue atendiendo el `SET_RTC` que le llegue**.~~ *(11/09:
   > construido en `68dd2c5`; y que el puente atienda el `SET_RTC` es lo correcto con `D-26`.)* Medido
   > el 07/09 *(crónica)*:
   >
   > ```
   > $ grep -c "ESCLAVO\|Esclavo\|esclavo" 01_Firmware/ESP32_Expansion/src/despachador.cpp
   > 0
   > ```
   >
   > **El puente es el MISMO firmware en los dos postes, y el fichero que decide qué hacer con un
   > `SET_RTC` no nombra al Esclavo ni una vez: hoy lo atiende venga por donde venga.** O sea que el
   > botón sigue ahí y sigue funcionando. **Que no se pulse es hoy una regla de procedimiento, no una
   > barrera del equipo** — y por eso va escrito en el paso, y no sólo en la decisión. *(11/09: vale
   > para el botón del **Courier**; «⏱️ Sincronizar» sí se pulsa — ver el recuadro `D-26` de arriba.)*

3. ✅ **Lo que SÍ se hace en el poste 2, y `D-20` no lo deroga: LEER la hora sin cambiarla** —
   `CMD:LEER_RTC` (`D-17`). **Leer no escribe nada**, así que sigue mandándose **a los dos postes**;
   ~~con `D-20` construida vale más que hoy, porque es la única forma de comprobar que la siembra del
   Maestro llegó de verdad al reloj del poste 2.~~ *(11/09: lee el `DS3231` del ESP32 de ese poste, que
   la radio no escribe; la hora que usa la controladora es el `HORA:` de su `$STATUS`.)*
4. ~~**Vía Pantalla LCD (Gabinete):** `Botón 4` hasta el menú ➔ `MODO DEGRADADO` ➔ `Botón 3`
   (`CONFIRMAR ENTRADA`).~~ ⛔ **`D-17.bis`: la pantalla y el menú se retiran del equipo.**

> ~~🛑 **AVISO AL DÍA (02/09/2026) — AL ESCLAVO NO SE LE PUEDE METER EN DEGRADADO DESDE LA APP.**~~
>
> 🔴 **TACHADO ENTERO EL 07/09. ERA LA FRASE MÁS PELIGROSA DE ESTE DOCUMENTO** — en mayúsculas, en
> el procedimiento de emergencia, negando la vía que existe y mandando a una que no existe.
> **Es falsa desde el commit `15e8cf3`** (`D-18`), no desde una fecha vaga: ese commit es la medida
> que la tacha. Gana `D-18` y gana el `grep`:
>
> ```
> $ grep -c "SET_MODO" 01_Firmware/Esclavo/src/bluetooth.cpp
> 10
> ```
>
> ~~Medido sobre `Esclavo/src/bluetooth.cpp`, su despachador acepta exactamente estas acciones y
> ninguna más:~~
>
> ```
>   ~~CMD:AMBAR_EMERGENCIA            :315   (sin PIN)~~
>   ~~CMD:FORZAR_ROJO                 :382   (sin PIN)~~
>   ~~CMD:PIN:1234:AMBAR_EMERGENCIA   :402~~
>   ~~CMD:PIN:1234:CANCELAR_AMBAR     :425~~
>   ~~CMD:PIN:1234:FORZAR_ROJO        :458~~
>   ~~CMD:PIN:1234:SOLICITAR_PASO     :466~~
>   ~~CMD:PIN:1234:TEST_LEDS          :484~~
>   ~~CMD:PIN:1234:SET_RTC:...        :497~~
> ```
>
> **Ese censo tenía DOS defectos a la vez, y por eso se conserva:** le faltaba
> `SET_MODO:DEGRADADO`, y **sus ocho números de línea están hoy caducados** — el mismo censo se
> repitió con `:315/:382/:402…`, luego con `:381/:448/:468…`, y ninguna de las dos tandas apunta ya
> a su sitio. **Por eso este documento pasa a citar el símbolo y el `grep`, no la línea**
> (`CLAUDE.md` §4.sexies).
>
> ~~Las dos únicas puertas de entrada del Esclavo son `Esclavo/src/mando.cpp:148` —la secuencia
> `A · B · A · B` del mando de relés— y `Esclavo/src/menu.cpp` *(`grep -n "degradado_entrar" 01_Firmware/Esclavo/src/menu.cpp`)*, que necesita `botonAceptar()`
> y por tanto **está tapiada**.~~
>
> ✅ **Las puertas de `degradado_entrar()`, censadas el 07/09 con
> `grep -rn "degradado_entrar" 01_Firmware/Esclavo/src/`:** `bluetooth.cpp` **(la de la app, la que
> se usa)**, `mando.cpp` (sin actuador, `D-1`) y `menu.cpp` (tapiada, `D-17.bis`). **La puerta
> siempre estuvo construida; lo que faltaba era la llave, y `D-18` se la dio a la app.**
>
> ### 🔴 Lo que eso significa en obra, sin adornos
>
> ~~**Para poner el Esclavo en Degradado hace falta el mando de relés, y su receptor RF nunca se
> compró.** Mientras eso siga así, este procedimiento **no se puede completar en las dos puntas** —
> y una sola punta en Degradado es peor que ninguna (Sección 6).~~
>
> ✅ **07/09: el procedimiento SÍ tiene VÍA en las dos puntas, y con la misma orden.** Lo que hace
> falta para pedirlo es **el teléfono** (`D-16`), no una escalera ni una compra.
>
> 🛑 **Pero «tiene vía» no es «entra», y la diferencia se paga en el poste.** Hoy las dos puntas
> **rechazan** la orden por el requisito 1 —reloj del STM32 sin poner en hora, sin camino para
> ponerlo (`D-15`)—. **Ver el recuadro del principio del documento.** La llave está puesta; la
> cerradura sigue trabada por el otro lado.
>
> - El punto 3 (pantalla + pulsadores) sigue **sin actuador**: no hay `Botón 3` ni `Botón 4`.
> - Lo que el Esclavo **sí** acepta desde la app es la **entrada en Degradado** (`D-18`), el **ámbar
>   de emergencia** y su revocación.

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
tope. ~~A partir de las **44 h** la pantalla muestra `AVISO: LIMITE 48h`~~, y a las 48 h el equipo se
rinde por su cuenta y muestra `Limite 48h sin sync — Revise el radio`.

> 🔴 **CORREGIDO EL 07/09 — EL AVISO NO LLEGA A LA MISMA HORA EN LAS DOS PUNTAS, Y ESO NO ESTABA
> ESCRITO EN NINGÚN SITIO.** Medido sobre el C++, y se cita el **símbolo** porque los números de
> línea caducan solos:
>
> ```
> grep -rn "AVISO_LIMITE_MS"   01_Firmware/Maestro/src/modo_degradado.cpp
>   static const unsigned long AVISO_LIMITE_MS  = 158400000UL;   // 44 h
> grep -rn "AVISO_SIN_SYNC_MS" 01_Firmware/Esclavo/src/modo_degradado.cpp
>   static const unsigned long AVISO_SIN_SYNC_MS = 40UL * 3600UL * 1000UL;   // 40 h
> ```
>
> | punta | constante | avisa a las | margen que deja hasta el tope |
> |---|---|---|---|
> | **MAESTRO** | `AVISO_LIMITE_MS` | **44 h** | 4 h |
> | **ESCLAVO** | `AVISO_SIN_SYNC_MS` | **40 h** | 8 h |
>
> **El tope duro son 48 h en las dos.** Lo que cambia es cuándo avisa cada una, y **este documento
> publicaba sólo el número del Maestro para las dos puntas**.
>
> 🔴 **Y LO QUE NINGUNA DE LAS DOS CONSTANTES ARREGLA, medido el 07/09: EL PLAZO NO SE PUEDE
> CONSULTAR. En el POSTE 2, ni el aviso ni la cuenta salen del equipo.** El firmware del Esclavo
> **sí sabe** cuánto hace de la última sincronización y si el plazo venció —`degradado_msDesdeSync()`,
> `degradado_syncVencida()`, `degradado_avisoLimite()`—, y **el único llamador de los tres es
> `Esclavo/src/menu.cpp`**, la pantalla del gabinete que `D-17.bis` retira del equipo. **No viajan en
> ninguna trama y no hay comando para pedirlos.**
>
> **Así que el plazo se lleva a mano:** al dejar un poste en Degradado **se anota la hora** y se
> vuelve **antes** de las 48 h. Lo único que el técnico verá por sí solo es el **resultado** cuando ya
> se cumplió: `MODO:RENDIDO` y la luz en ámbar. 🆕 **Es el hueco que `DECISIONES.md` `D-23` viene a
> cerrar — decidida el 07/09 y SIN CONSTRUIR**; el censo de lo que haría falta está en
> `14_Manual_App_Movil_IOT_VIAL.md` §5.8.

> 🛑 **Por qué importa en obra y no es un detalle:** quien espere el aviso del Esclavo a las 44 h lo
> verá a las **40** y concluirá que el equipo va mal; y al revés, quien crea que el Maestro avisa a
> las 40 dará por perdida una ventana que todavía tiene. **Cuente 40 h para el Esclavo y 44 para el
> Maestro**, o más simple: **al primer aviso de cualquiera de las dos puntas, quedan al menos 4 h.**
>
> ~~⚠️ **La pantalla que mostraba `AVISO: LIMITE 48h` ya no se puede leer en el equipo**
> (`D-17.bis`): lo que hay que mirar es el `$STATUS` en la app.~~
>
> > # 🔴 TACHADO EL 07/09 — **EL AVISO NO ESTÁ EN EL `$STATUS`. HOY NO HAY NINGUNA FORMA DE VERLO**
> >
> > La primera mitad era cierta; **la segunda mandaba a mirar donde no está**, que es peor que no
> > decir nada. Medido el 07/09, por los dos lados:
> >
> > **1. Los getters del aviso tienen UN SOLO llamador, y es la pantalla retirada:**
> >
> > ```
> > $ grep -rn "degradado_syncVencida\|degradado_avisoLimite" 01_Firmware/Esclavo/src 01_Firmware/Esclavo/include
> > 01_Firmware/Esclavo/src/menu.cpp:86:                    degradado_syncVencida(),
> > 01_Firmware/Esclavo/src/menu.cpp:117:                       degradado_syncVencida(), degradado_avisoLimite(),
> > 01_Firmware/Esclavo/src/menu.cpp:129:                       degradado_syncVencida(), degradado_avisoLimite(),
> > 01_Firmware/Esclavo/src/modo_degradado.cpp:186:bool degradado_syncVencida() { return syncVencidaLatch; }
> > 01_Firmware/Esclavo/src/modo_degradado.cpp:188:bool degradado_avisoLimite() {
> > 01_Firmware/Esclavo/include/modo_degradado.h:65:bool degradado_syncVencida();
> > 01_Firmware/Esclavo/include/modo_degradado.h:70:bool degradado_avisoLimite();
> > ```
> >
> > **`menu.cpp` y nada más.** Y `menu.cpp` es la interfaz que `D-17.bis` retira del equipo.
> >
> > **2. El `$STATUS` no lleva ningún campo de antigüedad de sincronización.** Plantillas reales,
> > leídas del `snprintf` de cada punta el 07/09:
> >
> > ```
> > Maestro: $STATUS,NODE:MAESTRO,SERIE:%s,MODO:%s,ESTADO:%s,T:%s,RF:%s,RTT:%s,BAT:--,HORA:%s,ESC:%s,PLUMA:%s,CAM:%s
> > Esclavo: $STATUS,NODE:ESCLAVO,SERIE:%s,MODO:%s,ESTADO:%s,T:--,RF:--,RTT:--,BAT:--,HORA:%s,PLUMA:%s,CAM:%s
> > ```
> >
> > **Ni «horas desde la última sync», ni «quedan N horas», ni el aviso.** El campo `T:` es un
> > contador libre 0–59, no una cuenta atrás del límite.
> >
> > ## 🛑 Lo único que el operario SÍ puede ver, y llega TARDE
> >
> > | qué | dónde | cuándo |
> > |---|---|---|
> > | que el Esclavo **ya se rindió** | `MODO:RENDIDO` en su `$STATUS` | **a las 48 h, cuando ya ocurrió** |
> > | que el Esclavo está dentro del modo | `MODO:DEGRADADO` | mientras dura |
> > | 🔴 **el aviso de las 40 h / 44 h** | **EN NINGÚN SITIO** | — |
> >
> > **En obra esto significa: el técnico que sube al poste 2 no tiene el dato ni sustituto por app.**
> > La única forma de saber cuánto queda es **contar las horas desde que se entró en el modo**, a
> > mano y por fuera del equipo. **Anótelo en el parte al entrar.**
> >
> > 🟡 **Que esto se arregle —publicar la antigüedad en el `$STATUS`— es una decisión del
> > responsable, no de este documento:** cuesta bytes de trama, y la cota del `$STATUS` del Maestro
> > ya está apretada.

### Por qué existe

El colchón que impide el verde simultáneo es el **todo-rojo de 30 s**. Ese colchón se come poco a
poco por la deriva de dos cristales de 32.768 kHz sin calibrar, a la intemperie: **±30 a 50 ppm**.

> ## ⚠️ 07/09 — ESTA CUENTA SE APOYA EN UN OSCILADOR QUE NO OSCILA. NO SE RECALCULA AQUÍ; SE SEÑALA
>
> **«La deriva de DOS cristales de 32.768 kHz»** son los dos `Y2`, uno por punta — **el mismo cristal
> que N-17 dio por muerto** y del que cuelga el requisito 1 de la Sección 2. Es decir: **toda la
> tabla de abajo, los `±30 a 50 ppm` y el factor 1,44 describen un equipo cuyos dos relojes hoy no
> cuentan.**
>
> 🛑 **Eso no vuelve falsa la cuenta, y por eso no se tacha:** describe correctamente el equipo tal
> como fue diseñado, y **si los dos `Y2` arrancaran mañana la aritmética sería exactamente ésta**.
> Lo que hay que saber al leerla es que **es una cuenta sobre un oscilador que no está oscilando**,
> igual que el requisito 1 es una puerta que hoy nadie puede abrir.
>
> 🟢 **`D-20` cambia el sujeto de la cuenta, y no está construida.** Con la hora sembrada desde el
> `DS3231` de cada ESP32, quienes derivan pasan a ser esos dos módulos y no los dos `Y2`.
> **`roadmap.md` §3.4.bis publica el presupuesto derivado por el arquitecto —≈2,8 s contra los 29 s
> que aguanta el cruce, factor ≈10—, y aquí NO se copia como si ya rigiera:** es una cuenta sobre un
> mecanismo que todavía no existe. **La cifra vigente para el equipo de hoy sigue siendo la de abajo.**
> *(Y una cuenta metida en una nota es justo lo que `CLAUDE.md` manda llevar a un pack (N-71); ese
> pack tampoco existe todavía.)*

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

> 🔴 **07/09 — `D-20` NO CAMBIA LA REGLA, PERO SÍ DE DÓNDE SALE EL NÚMERO, Y ESA ES LA MITAD QUE SE
> OLVIDA.** La radio **sigue siendo el camino entre postes** y una sincronización nueva sigue siendo
> lo único que reinicia la cuenta. Lo que cambia es que **ese «contador del RTC» es hoy el del RTC
> del STM32** —el que `D-20` jubila—. **`roadmap.md` §3.4.bis lo lista como la fila 3 de lo que hay
> que construir:** *«las 48 h salen hoy del contador crudo del RTC; tienen que venir del ESP32 o se
> pierden en el primer corte»*.
>
> ⚠️ **No se puede afirmar aquí cómo quedará ese cómputo: no está construido, y este documento no
> escribe hipótesis como si fueran reparaciones.** Lo que sí se puede afirmar es lo de hoy: **el
> límite de 48 h se apoya en un contador que sólo avanza si el `Y2` oscila** — el mismo cristal del
> requisito 1.
>
> 🛑 **Y lo que `D-20` sí obliga a decir aquí, porque es de campo:** el rejuvenecimiento de esta
> cuenta poniendo un reloj hacia atrás desde el teléfono **deja de ser posible por diseño** — ~~la app
> no pone la hora en el poste 2~~ *(11/09, `D-26`: la app sí pone la hora en el poste 2, pero **esa hora
> no renueva** la marca de sincronización —la rama `CMD:HORA_ESP32:` del Esclavo no llama a
> `degradado_registrarSync()` ni a `respaldo_marcarSync()`, medido en `68dd2c5`—, así que la rendija
> sigue cerrada)*. El límite de 48 h no tiene botón de posponer, y esa era la última rendija que
> quedaba.

---

## 5. Procedimiento de SALIDA

> ## ⚠️ LA VERIFICACIÓN VISUAL DE AMBAS PUNTAS ES OBLIGATORIA TAMBIÉN AL SALIR
>
> No solo al entrar. **Salir de una sola punta crea exactamente el escenario que este modo existe
> para evitar** — ver Sección 6.

### Para volver a intentar Automático

Se hace cuando se cree que el radio volvió. El escenario típico es *"dejó de llover, a ver si
enlaza"*.

- **Desde la app:** `CMD:PIN:1234:SET_MODO:AUTO` — **solo en el Maestro**. Al Esclavo lo devuelve el
  propio Maestro en cuanto vuelve el radio (su `CMD_PING` llama a `degradado_salir()`).
- ~~**Desde el piso:** `A · A · A` en menos de 12 s → **2 destellos rojos**.~~ ⛔ **`D-1`: no hay
  mando con qué darlo.** La secuencia sigue en el firmware; el emisor no existe.
- ~~**Desde la pantalla:** en `CONFIGURACION → MODO DEGRADADO`, `Botón 3` (`3=Salir`).~~ ⛔ sin actuador.

> 📕 **HISTÓRICO DEL 02/09 — se conserva por el CÓDIGO, no como instrucción** *(🔴 07/09: el ✅ que
> encabezaba este bloque se retira — leía como un permiso, y `D-1` retiró el aparato: ver el tachado
> que sigue al bloque)*. **`A · A · A` NO se retiró del firmware, y la versión anterior de este documento decía
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
> ⚠️ ~~**Lo que falta no es el firmware: es el receptor RF, que nunca se compró.**~~ ~~Sin él no hay
> con qué generar los pulsos desde el piso. **La secuencia existe y no tiene mando.**~~
>
> > 🔴 **07/09 — `D-1`: NO «FALTA» EL RECEPTOR. EL MANDO NO EXISTE Y NO SE VA A COMPRAR.** La
> > diferencia importa: *«falta»* pone una compra en la lista y deja al operario esperándola;
> > `D-1` cierra la vía. **El firmware del mando SE CONSERVA a propósito** —`mando_ambarLocal()`
> > tiene cinco llamadas vivas y su veto es SFTY-21; retirar el armador dejaría los `if` siempre
> > verdaderos y **el veto abierto, no inerte**—. O sea: **código vivo, actuador inexistente.**
>
> ~~🔧 **Matizado el 04/09 (`N-118`): en BANCO sí hay con qué darlos, con un cable.** Un pulso `A` es
> cerrar un instante **`J16` p5 contra p4**, y un pulso `B`, **p8 contra p7** — los **3,3 V del pin
> contiguo**. … Lo que sigue faltando es **el receptor**, para darlos por radio desde el piso, y
> **una tarjeta sana** (N-116).~~
>
> > # 🛑 TACHADO ENTERO EL 07/09 — **ESTE PÁRRAFO MANDABA HACER LO QUE LA CABECERA DE ESTE MISMO DOCUMENTO PROHÍBE**
> >
> > Arriba, en el aviso *«UNA COSA QUE NO HAY QUE HACER NUNCA»*, este documento dice:
> > **«NO SE CABLEA NADA A `J16` p5 NI A `J16` p8»**. Y aquí abajo daba **la instrucción exacta para
> > cerrar contactos en esos dos pines**, a 500 líneas de distancia. **Un procedimiento que se
> > desmiente a sí mismo manda al técnico a hacer lo que lea primero.** Manda la cabecera y manda
> > `DECISIONES.md` `D-1`.
> >
> > 🔴 **Y lo que hace que esto no sea una contradicción de redacción sino un riesgo vial — medido el
> > 07/09 en `Maestro/src/mando.cpp`, en el `case ACC_AUTOMATICO` de `ejecutar()`:**
> >
> > ```
> > $ grep -n "pedirArranqueDirecto\|modoActual_set(MODO_AUTOMATICO)" 01_Firmware/Maestro/src/mando.cpp
> > 118:      modoAutomatico_pedirArranqueDirecto();
> > 122:        modoActual_set(MODO_AUTOMATICO);
> > ```
> >
> > **`A·A·A` entra al Modo Automático SIN GUARDA NINGUNA: arranca el ciclo, o sea ABRE PASO.** El
> > asistente de configuración que antes se interponía **ya no existe** —`N-42`;
> > `modoAutomatico_pedirArranqueDirecto()` es hoy un cuerpo vacío—, así que tres contactos en `p5`
> > dentro de la ventana de 12 s **dan verde en un cruce**. Eso se salta la confirmación de vía que
> > `DECISIONES.md` `D-11` exige a la app *(«al aplicar tiempos, la app AVISA y da el botón: NO
> > arranca el ciclo sola»)*.
> >
> > **Y `A·B·A·B` no es más inocente: pide entrar en el modo que da verde sin confirmar la otra
> > punta.**
> >
> > ⚠️ **Lo que del párrafo tachado sigue siendo cierto y por eso no se borra:** si algún día alguien
> > tiene que tocar esos pines, el contacto va **contra los 3,3 V del pin contiguo (p4 / p7), NUNCA
> > contra masa** — `J16` tiene **una sola masa en todo el conector** (`p2`), y `p4` es **adyacente**
> > a `p5`: **un puente corrido una posición pone el riel de 3,3 V contra masa**, que es el gesto que
> > precedió al calentamiento del paso 29. **Pero hoy la instrucción es que no se toca.**
>
> 👁️ **Cómo confirma el equipo que oyó un pulso — se conserva porque el mecanismo sigue en el
> firmware, NO como instrucción de prueba.** `grep -n "DESTELLOS_" 01_Firmware/Maestro/src/mando.cpp`
> → **`A·A·A` → 2 destellos rojos · `B·B·B` → 3 · `A·B·A·B` → 4 · rechazado → ámbar rápido de 2 s**.
> 🛑 **Hoy no hay quién dispare esas secuencias y nadie debe fabricarlas con un cable** (`D-1`).
>
> **La vía por app no es equivalente a `A·A·A`:** `CMD:PIN:1234:SET_MODO:AUTO`
> (`Maestro/src/bluetooth.cpp:378`) **se salta el todo-rojo de despedida** de
> `modo_degradado.cpp:448-462` y pasa del verde por reloj directo a `modoAutomatico_setup()`. Y ~~**en
> el Esclavo no hay `SET_MODO` de ninguna clase**~~ **en el Esclavo no hay `SET_MODO:AUTO`**
> *(la entrada sí existe: `SET_MODO:DEGRADADO`, `D-18` — corregido el 07/09)*, así que ejecutar solo
> el del Maestro produce el **Riesgo residual nº 2** (Sección 6) — una punta fuera, la otra dentro.

~~**Hágalo en las dos unidades**, y luego mire las luces:~~ 🔴 **07/09: la secuencia de abajo no se
puede ejecutar (`D-1`).** La maniobra vigente es `CMD:PIN:1234:SET_MODO:AUTO` en el Maestro; **el
diagrama se conserva porque los tiempos y los criterios de lectura de las luces siguen siendo los
mismos.**

```
   ~~A·A·A~~ SET_MODO:AUTO  ->  esperar ~15 s

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

- **Desde la app:** `CMD:PIN:1234:SET_MODO:AMBAR` en el Maestro · **`CMD:AMBAR_EMERGENCIA`** en el
  Esclavo *(esta última **no pide PIN**, justamente porque es la que para el cruce)*.
- ~~**Desde el piso:** `B · B · B` en menos de 12 s → **3 destellos rojos**.~~ ⛔ **`D-1`: no hay
  mando.** La secuencia sigue en el firmware y nada puede generarla.
- 🔴 **Y la consecuencia hay que leerla entera, porque es `D-16`: SIN TELÉFONO NO HAY SALIDA DE
  EMERGENCIA.** Lo que antes prometía `B·B·B` —*«desde cualquier estado, sin condiciones, sin
  depender de una batería de móvil»*— **hoy depende del teléfono**. No es una avería: es una
  propiedad declarada del sistema desde que se retiró el mando. **Batería, cable, y un segundo
  terminal emparejado.**

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
> `ambarLocal` —el veto del mando; `grep -n "ambarLocal = true" 01_Firmware/Esclavo/src/mando.cpp`,
> **por símbolo: el `:132` que se citaba aquí está caducado**— sigue armándose solo desde `B·B·B`.
> Son **dos enclavamientos distintos**: el del mando lo pone quien está subido al poste; el de la
> app, quien tiene el teléfono. Ver el manual del mando, §3.1.

> ## 🟢 EL BLOQUE DE ARRIBA ESTÁ CADUCADO DESDE EL 31/08/2026 — y por DOS motivos independientes
>
> Se conserva tachado y no se borra: una casilla que desaparece en silencio se vuelve a proponer.
>
> **(1) EL CÓDIGO DEL MANDO SE CONSERVA**, por decisión del responsable del 31/08, en los canales
> **A** y **B** (`MANDO_A` = `PB9` = `J16` p5, `MANDO_B` = `PB13` = `J16` p8). Se retiran **solo** los
> pulsadores 3 y 4, que son los que las cámaras necesitan. ~~**`A·A·A`, `B·B·B` y `A·B·A·B` siguen
> funcionando**~~, y con ellos `ambarLocal` y sus tres vetos. El *«sin actuador»* de arriba describía
> un equipo que la decisión del 31/08 ya no va a construir.
>
> > 🔴 **MATIZADO EL 07/09 — `D-1`, y es la distinción que sostiene todo este documento: SE CONSERVA
> > EL CÓDIGO, NO EL APARATO.** El 05/09 el responsable confirmó el hardware retirado: *«ya no
> > tenemos mandos de A y B, sólo la app»*. Las tres secuencias **compilan y se reconocerían si
> > alguien cerrara los contactos**, y por eso `ambarLocal` y sus vetos siguen bien construidos —
> > **pero no hay emisor, no hay pulsadores y el receptor RF nunca se compró.** *«Siguen
> > funcionando»* es cierto del firmware y falso del equipo, y en un procedimiento de campo lo que
> > cuenta es el equipo.
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
| **La app** (`CMD:AMBAR_EMERGENCIA`) | **la ÚNICA superficie de mando** (`D-16`) | **siempre** |
| ~~**El mando, `B·B·B`**~~ | ~~**la vía de último recurso**~~ | ~~cuando no hay teléfono, no hay cobertura, o el ESP32 se colgó~~ ⛔ **`D-1`: no existe** |

~~Que sea la *segunda* no la hace opcional: **es la única que no depende de una radio corta, de una
batería de móvil ni de un accesorio.** Por eso el mando se conserva.~~

> 🔴 **TACHADO EL 07/09, Y ES EL PÁRRAFO QUE MÁS FALTA HACE LEER DE ESTE DOCUMENTO.** El argumento
> era bueno —una vía que no dependiera del teléfono— **y el aparato que lo cumplía se retiró**
> (`D-1`, 05/09). Lo que queda escrito en `DECISIONES.md` `D-16` es la consecuencia, sin
> maquillarla:
>
> 🛑 **SIN TELÉFONO NO HAY FORMA DE OPERAR EL EQUIPO. Ni ámbar, ni volver a automático, ni parar el
> cruce.** No es una avería y no hay segunda vía esperando en un cajón. **Es una propiedad declarada
> del sistema**, y por eso el teléfono es **herramienta crítica**: batería, cable, y conviene un
> segundo terminal ya emparejado antes de salir a obra.
>
> *(El código del mando se conserva por SFTY-21 — ver el matiz de arriba —, no como vía de reserva.)*

**Y las dos tienen que hacer LO MISMO.** El firmware lo declara por escrito
—`Esclavo/src/bluetooth.cpp:32-39`: *«UNA EMERGENCIA PEDIDA POR BLUETOOTH VALE LO MISMO QUE UNA DEL
MANDO»*— y **hoy no es cierto en Modo Degradado**:

| vía | qué hace hoy en Degradado | ¿sale por el todo-rojo? |
|---|---|---|
| Mando, `B·B·B` | `grep -n "degradado_salir" 01_Firmware/Esclavo/src/mando.cpp` — si el Degradado gobierna la luz, `degradado_salir()`. *(Por símbolo: el `:129-141` que se citaba aquí está caducado)* ⛔ **`D-1`: sin actuador** | **sí** |
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

#### ~~🔴 Riesgo residual nuevo (31/08): entrar en Degradado con el ámbar de la app puesto~~ → ✅ **CERRADO Y CONSTRUIDO**

> # ✅ 07/09 — `R-4` ESTÁ RESUELTA EN EL FIRMWARE. **ESTE APARTADO DESCRIBÍA UN RIESGO ABIERTO QUE YA NO LO ESTÁ**
>
> Se conserva tachado y no se borra —quien lo leyera y lo diera por vigente estaría desconfiando del
> botón equivocado—, pero **la salida elegida fue «rechazar la entrada, con motivo», y está en el
> `.cpp`.** Medido el 07/09 dentro de `degradado_entrar()`:
>
> ```
> $ grep -n "DEG_RECHAZO_AMBAR_VIGENTE" 01_Firmware/Esclavo/src/modo_degradado.cpp 01_Firmware/Esclavo/include/modo_degradado.h
> 01_Firmware/Esclavo/src/modo_degradado.cpp:239:  if (bluetooth_ambarEmergencia()) return DEG_RECHAZO_AMBAR_VIGENTE;
> 01_Firmware/Esclavo/src/modo_degradado.cpp:458:    case DEG_RECHAZO_AMBAR_VIGENTE: return "AMBAR EMERG.PUESTO";
> 01_Firmware/Esclavo/include/modo_degradado.h:50:  DEG_RECHAZO_AMBAR_VIGENTE   // R-4: hay un ambar de emergencia puesto por una persona
> ```
>
> **Con un ámbar de emergencia puesto, el Degradado NO entra y lo dice:**
> `$ERR,CMD:SET_MODO:DEGRADADO,DESC:AMBAR EMERG.PUESTO`. **La máquina no revoca lo que puso una
> persona** —puede haber alguien trabajando bajo esa luz—; quitarlo es un acto deliberado y se hace
> desde el suelo con `CMD:PIN:1234:CANCELAR_AMBAR`.
>
> ⚠️ **Y la guarda mira SÓLO el latch de Bluetooth, no `mando_ambarLocal()`, a propósito:** la rama
> del mando ya resolvía lo mismo de otra forma —`ejecutar(ACC_DEGRADADO)` pone `ambarLocal = false`
> **antes** de llamar—, y añadirla aquí habría rechazado el `A·B·A·B` antes de que llegara a
> ejecutarse.
>
> 🔴 **Lo que NO se puede leer de este cierre: que esté ejercido.** Es **MEDIDO sobre fichero**;
> nadie lo ha ejercido en tarjeta. **Y el Maestro no tiene esta guarda** — ver §2.2.

~~**MEDIDO POR LECTURA, y RAZONADO —no ejercido—.** `degradado_entrar()`
(`Esclavo/src/modo_degradado.cpp:212-243`) fuerza todo-rojo en `:224` **sin preguntar por
`bluetooth_ambarEmergencia()`**, y el sostenedor del modo escribe luz por otra puerta —`aplicarLuz()`
desde `degradado_actualizar()`, `main.cpp:363`— **que tampoco lo consulta**.~~

~~Consecuencia: si alguien entra en Degradado desde el gabinete mientras hay un ámbar pedido por
teléfono, **la luz sale de ámbar en ese mismo instante**, el latch se revoca solo en la vuelta
siguiente (`bluetooth.cpp:292`), y en la siguiente frontera de fase el cruce puede dar **verde por
reloj** donde alguien había pedido precaución. **El `$ACK` ya se envió, y nada se lo dice a nadie.**~~

~~**Esto no lo arregla la decisión del 31/08 por sí sola.** Las tres opciones —rechazar la entrada,
revocar el latch explícitamente, o que el latch vete la luz del Degradado— están escritas con su
consecuencia en el Manual 10 **§4.5.7 (`R-4`)**, y **las decide el responsable**: lo que se elija lo
ve un conductor.~~

> **La regla de campo no cambia y sirve exactamente para esto:** *verificar con los ojos las dos
> puntas*, al entrar y al salir. Ver Sección 6, Riesgo 2.
>
> 🛑 **Y el bucle de obra que este cierre introduce, escrito aquí porque nadie lo espera:** si usted
> aborta con `CMD:AMBAR_EMERGENCIA` en el Esclavo y luego quiere volver a entrar en Degradado, **la
> orden le va a ser rechazada** hasta que mande `CMD:PIN:1234:CANCELAR_AMBAR`. **No es una avería.**

### Checklist de salida

- [ ] Salida ejecutada en el **MAESTRO** — `SET_MODO:AUTO` o `SET_MODO:AMBAR` por app ~~, o `A·A·A` / `B·B·B` con mando~~ *(⛔ `D-1`)*. ⚠️ **Por app no hay todo-rojo de despedida**
- [ ] Salida ejecutada en el **ESCLAVO** — `CMD:AMBAR_EMERGENCIA` por app ~~, o `B·B·B` con mando~~ *(⛔ `D-1`)*. ⚠️ **No hay salida a Automático en el Esclavo**: por app solo se le puede llevar a ámbar
- [ ] Si salió a ámbar por app en el Esclavo: **`RESULT:OK` visto**, no solo `SALIENDO_TODO_ROJO`
- [ ] **Verificado con los ojos** que **ambas** puntas quedaron en el mismo estado — las dos ciclando
      o las dos en ámbar *(sigue vigente: mirar no necesita actuador)*
- [ ] **Ninguna punta quedó dando verde por reloj mientras la otra parpadea en ámbar** *(sigue vigente)*

> ✅ **Las cinco casillas se pueden marcar hoy** (02/09). En agosto las dos primeras eran
> imposibles, y un checklist con casillas imposibles es una invitación a firmarlo igual.
>
> ⚠️ **La segunda tiene un límite que hay que conocer antes de marcarla:** por app el Esclavo solo
> va a **ámbar**, no a Automático. Si quiere las dos puntas ciclando otra vez, el Esclavo sale del
> Degradado ~~**solo** por el mando (`A·A·A`) o~~ **cuando el radio vuelva** — el `CMD_PING` del
> Maestro llama a `degradado_salir()` en esta punta. 🔴 **07/09 (`D-1`): la vía del mando ya no
> existe, así que ésa es la ÚNICA.** Y la entrada, en cambio, sí se pide desde la app en las dos
> puntas (`D-18`).

---

## 6. ⚠️ Riesgos residuales — aceptados por el cliente el 01/08/2026

Están escritos aquí porque **el funcional debe conocerlos antes de firmar**, no después de un
incidente.

> 🔴 **07/09 — SON TRES, Y EL TERCERO NO LO HA ACEPTADO NADIE TODAVÍA.** El título de esta sección
> dice *«aceptados por el cliente el 01/08/2026»*, y desde hoy eso sólo es cierto para el `Riesgo 1`
> y el `Riesgo 2`. **El `Riesgo 3` —`DECISIONES.md` fila `D-21`, la hora que miente— se añade el
> 07/09 y llega SIN esa firma**: es una variante nueva del `Riesgo 2`, con causa propia. Se marca
> así para que nadie lo lea como ya aceptado por venir debajo del mismo título.

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

> 🔴 **CORREGIDO EL 07/09 — EL DIAGRAMA DESCRIBE **UNA** DE LAS DOS RAMAS DEL ARRANQUE, Y ADEMÁS
> NOMBRA UNA PANTALLA QUE YA NO EXISTE.** Medido sobre `Maestro/src/main.cpp`, alrededor de
> `modo_degradado_reanudarTrasCorte()`:
>
> | rama | cuándo | qué hace la unidad reiniciada |
> |---|---|---|
> | `if (reanudarDegradado)` | el respaldo dice que estaba en Degradado **y** siguen vigentes las tres condiciones | `modoActual_set(MODO_DEGRADADO)` + `modo_degradado_setup()` — **vuelve al modo, en todo-rojo** |
> | `else` | cualquier otro caso | `modoActual_set(MENU)` — es el diagrama de arriba |
>
> **Los dos escenarios son peligrosos y por motivos opuestos, y hay que conocer los dos:**
>
> - **Rama `else` (la del diagrama):** una punta en ámbar contra otra en verde. Es el riesgo clásico.
> - 🔴 **Rama de reanudación:** la unidad **vuelve a dar verdes por reloj sin que nadie haya
>   verificado las dos puntas**. Si la otra unidad NO se reinició y sigue en fase, no pasa nada; si
>   se reinició y cayó por la rama `else`, se produce **exactamente el mismo cruce ámbar-contra-verde**
>   — sólo que ahora **puede ser la reanudada la que dé el verde**.
>
> ⚠️ **Y `MENU` ya no es una pantalla que alguien vea** (`D-17.bis`): es el estado interno de reposo
> que fuerza rojo. **El equipo no muestra nada; hay que leer `MODO:` en el `$STATUS`.**
>
> 🛑 **La mitigación no cambia y ahora tiene una razón más: verificar las dos puntas con los ojos,
> también DESPUÉS DE CUALQUIER CORTE DE ENERGÍA.** El equipo no va a pedirlo.

Un lado en ámbar contra un lado en verde es **exactamente lo que este modo quiere evitar**: el
conductor del lado en verde entra confiado a un tramo que el otro lado está negociando. Ocurre igual
—sin microcorte de por medio— **si un operario saca del Degradado una sola unidad**.

> **Mitigación procedimental, no técnica: la verificación visual de ambas puntas es obligatoria
> también AL SALIR.** Debe constar en el acta de pruebas (`3_Protocolo_Pruebas_Rigurosas.md`,
> Sección 9).

~~**Pendiente conocido (N-20):** hoy el estado del Degradado y la marca de sincronización viven en RAM,
así que un microcorte los pierde.~~ El módulo `respaldo.cpp` que los guarda en los registros de
respaldo —**alimentados por la misma pila `CR2032` ya instalada**— ~~**está escrito pero todavía no
conectado**. Mientras no lo esté, **cualquier corte de energía en una punta produce el escenario de
arriba**.~~

> 🔴 **07/09 — LA PARTE TACHADA ES LA TERCERA COPIA DE LA MISMA FRASE CADUCADA** *(las otras dos
> están en la Sección 2 y en la Sección 9, ya corregidas con su `grep`)*: **`respaldo.cpp` SÍ está
> conectado** —`respaldo_setup()` tiene llamador vivo en el `main.cpp` de las dos puntas— y el
> equipo **reanuda solo** el Modo Degradado tras un corte. Se conserva tachada y no borrada porque
> es el mismo error repetido en tres sitios, y eso es el hallazgo.
>
> ## ✅ LO QUE DE ESTE PÁRRAFO NO SÓLO SIGUE VIGENTE, SINO QUE `D-20` REFUERZA: **LA `CR2032` DEL STM32 SIGUE SIENDO OBLIGATORIA**
>
> **Y ya no es por la hora.** `D-20` le quita al STM32 la autoridad del reloj, así que es fácil
> concluir *«entonces la pila del STM32 ya no hace falta»*. **Es falso, y sería un error caro:** esa
> misma `CR2032` alimenta el **dominio de respaldo** (`BKP->DR1..DR10`), donde viven la marca de
> sincronización y el indicador del Degradado — o sea **el cómputo de las 48 h y la reanudación tras
> corte**. Lo dice el propio fuente, símbolo `respaldo.h`:
>
> > *«alimentados por LA MISMA pila `CR2032` que ya mantiene el RTC»* · *«Sin pila… `respaldo_setup()`
> > encuentra el contenido inválido y borra»*
>
> 🛑 **Sin esa pila, el equipo arranca con el respaldo borrado y el Degradado no puede reanudar
> nunca.** Son **dos pilas por poste y las dos hacen falta**: la `CR2032` del STM32 (el respaldo) y
> la del módulo `DS3231` del ESP32 (la hora).
>
> 🔴 **Y lo que NO se escribe aquí, porque es falso y ya se escribió mal una vez hoy en otro
> documento:** *«el STM32 no tiene ni pila ni cristal»*. **Tiene las dos y no usa ninguna:** `Y1` de
> 8 MHz está en la placa y el firmware arranca con el **HSI**, y `VBAT` midió **3 V con la tarjeta
> apagada** (N-37) en **al menos una** tarjeta —la otra sigue `SIN VERIFICAR`—. **Lo muerto es `Y2`,
> y sólo `Y2`.**
>
> ✅ **Por eso el aviso del principio de este documento se mantiene palabra por palabra: no cambie la
> pila, no cambie el cristal `Y2` y no cargue nada.** Que `D-20` mude el reloj **no autoriza a
> quitarle piezas a la tarjeta**.

### Riesgo 3 — 🔴 **UNA HORA QUE MIENTE: el reloj se para en una fecha pasada y el equipo sigue dando verdes con toda confianza** ➕ **NUEVO (07/09)** — `DECISIONES.md` fila **`D-21`**

> # 🛑 ESTE RIESGO ESTÁ **DECIDIDO Y NO CONSTRUIDO**. LO QUE SIGUE DESCRIBE LO QUE EL EQUIPO **DEBE** HACER, NO LO QUE HACE HOY
>
> **Si usted lee este apartado buscando *«qué hará el equipo»*, la respuesta de hoy es: nada.** Se
> escribe aquí porque `D-21` abre un riesgo que esta sección no tenía en ningún renglón, y porque
> un riesgo sin escribir no se mitiga ni con el procedimiento.
>
> ⚠️ **PERO NO por la razón evidente, y la diferencia decide lo que cuesta arreglarlo:** el
> **Maestro SÍ tiene escrita** la reacción *«reloj no fiable → ámbar»* dentro de su bucle de
> Degradado. Lo que pasa es que **cuelga del reloj equivocado** —el RTC del STM32, no el `DS3231`
> cuya pila se agota— y que **hoy no se ejecuta nunca**, porque `reloj_enHora()` es falso siempre.
> **El Esclavo, ése sí, no la tiene en absoluto.** Medido el 07/09; el detalle y los `grep` están
> más abajo, en *«la detección ya existe en el chip»*.

**El escenario, que es distinto de los otros dos:** la pila del módulo `DS3231` se agota y el reloj
**queda clavado en una fecha pasada**. **Eso no es quedarse sin hora.** Un `DS3231` sin pila
devuelve una fecha **perfectamente formada** —día, mes, hora, minuto, segundo, todos plausibles—, y
un Degradado que cuelgue de ella **repartirá verdes con toda confianza sobre una hora que no
avanza**.

| | qué hace el equipo | qué ve el conductor |
|---|---|---|
| **sin hora** *(el caso que ya está cubierto)* | `reloj_enHora()` es falso, el Degradado **no entra** | nada anómalo: no se llega a entrar en el modo |
| 🔴 **hora que miente** *(éste)* | **nada lo distingue del caso bueno hoy** | **verde**, y el verde le dice *«pase tranquilo»* |

**La respuesta decidida: `ÁMBAR INTERMITENTE` en la punta que tiene la hora mentirosa, y se
publica** para que la app lo enseñe.

> **Por qué ámbar y no rojo, y es la doctrina que este mismo documento ya tiene escrita:** el ámbar
> dice *«no estoy controlando esto, decida usted»* y el conductor **llega alerta**. Un rojo fijo en
> las dos puntas para el cruce entero por un fallo de reloj; un verde por reloj **le quita la
> precaución al que llega**. Entre las tres, la única que degrada hacia la atención del conductor
> es el ámbar.

#### ⚠️ La asimetría NO se puede evitar, y por eso este riesgo vive **aquí** y no en otro apartado

**En Degradado NO HAY RADIO — es la definición del modo.** No hay forma de ordenar *«ámbar en las
dos»*: **cada punta decide sola**, con su propio reloj, y que las dos coincidan **sólo pasa si las
dos pierden la hora a la vez**. Lo normal es que se agote **una** pila.

```
   La pila del DS3231 del poste 1 se agota
        -> su hora queda clavada en una fecha pasada
        -> D-21: esa punta pasa a AMBAR INTERMITENTE      el conductor NEGOCIA el paso
   El poste 2 tiene su reloj sano y sigue en fase          el conductor pasa CONFIADO
```

**Es el `Riesgo 2` otra vez, por una causa nueva.** El `Riesgo 2` fue aceptado por el cliente el
**01/08/2026** y está declarado **sin solución técnica sin radio**; `D-21` **no lo empeora ni lo
arregla: le añade un disparador que antes no estaba en la lista** —hasta hoy la asimetría sólo
podía nacer de un microcorte o de un operario, y desde `D-21` puede nacer **de una pila**.

> 🛑 **Y por eso la mitigación del `Riesgo 2` se aplica entera, palabra por palabra, también a
> éste: verificación visual de LAS DOS PUNTAS.** El equipo no la va a pedir.

#### 🟢 La detección YA EXISTE en el chip. Lo que falta es que llegue a las luces

**No hay que inventar nada.** El `DS3231` levanta su bit **`OSF`** (*oscillator-stop flag*) en
cuanto el oscilador se para, y el firmware del puente **ya lo lee y ya lo declara**:

```
$ grep -n "R-2" 01_Firmware/ESP32_Expansion/src/reloj_ds3231.cpp
```

> *«una hora con `OSF` puesto se declara NO FIABLE aunque los registros traigan valores
> plausibles»* — símbolo `reloj_ds3231.cpp`, regla `R-2` de
> `18_Especificacion_Firmware_ESP32.md` §5.3

🔴 **Lo que NO existe es el camino de esa declaración hasta las luces.** Hoy el `OSF` muere en el
puente: **el STM32 no lo recibe.**

> # ✅ CORREGIDO HORAS DESPUÉS, EL MISMO 07/09 — **MEDIA `D-21` YA ESTÁ CONSTRUIDA**
>
> **Aquí se había escrito ~~*«no hay quien lo convierta en ámbar»*~~ y ~~*«está sin empezar»*~~. Es
> falso, y se conserva tachado con su motivo:** salió de suponer la ausencia en vez de medirla.
>
> **Medido, y el `grep` corrido antes de publicarlo:**
>
> ```
> $ grep -n "irAAmbar(" 01_Firmware/Maestro/src/modo_degradado.cpp
> $ grep -c "irAAmbar"  01_Firmware/Esclavo/src/modo_degradado.cpp
> ```
>
> **El MAESTRO ya lo hace.** Dentro de su bucle de Degradado, con este comentario al lado:
>
> > *«El reloj puede dejar de ser fiable en marcha (pila agotada). Sin hora no hay fase que
> > calcular, y seguir dando verdes con la ultima que se recuerde seria inventar»*
> > — y llama a `irAAmbar("Reloj no fiable", "Degradado detenido")`, **pasando por rojo antes del
> > ámbar**, como manda `SFTY`.
>
> **Entonces, ¿qué falta de verdad?** Tres cosas, y las tres son más pequeñas que «construirlo»:
>
> | # | falta | detalle |
> |---|---|---|
> | **A** | 🔴 el `OSF` del `DS3231` **no llega** a `reloj_enHora()` | esa bandera es el RTC del STM32 sobre `Y2`; **el reloj cuya pila se agota es el `DS3231` del ESP32**. Son dos relojes y hoy **no se hablan** |
> | **B** | 🔴 el **ESCLAVO no tiene** la comprobación en su bucle | `irAAmbar` **no existe** en su `modo_degradado.cpp`; sólo mira el reloj **al entrar** y **al reanudar tras corte**. Dentro del modo, esa punta **ya no vuelve a mirarlo** |
> | **C** | 🔴 **no se publica** | ni el `$STATUS` ni la app tienen hoy dónde decir *«el reloj de este poste no es fiable»* |
>
> 🆕 **07/09 — LA PIEZA `C` YA TIENE DUEÑO, Y NO ES FIRMWARE: `DECISIONES.md` fila `D-23`.**
> *«La app necesita una pantalla propia del poste 2»* —el diagnóstico de esa punta **cuando el
> teléfono se conecta por Bluetooth directamente a ella**, no a través del Poste 1—. Y con eso la
> propia fila `D-21` cierra la duda de si además hay que **salir** del modo: **no hace falta que el
> equipo lo decida solo, porque el Maestro reenvía la hora periódicamente y para que esa hora se
> vuelva mentirosa tendrían que pasar MESES. Lo que hace falta es poder DIAGNOSTICARLO cuando
> alguien vaya.**
>
> 🛑 **`D-23` está DECIDIDA Y SIN CONSTRUIR — ni una línea, ni en la app ni en el firmware.** Lo
> que SÍ está medido —qué publica y qué acepta hoy esa punta por Bluetooth, y qué datos ya calcula
> sin que nadie pueda leerlos— está en `05_Funcional/14_Manual_App_Movil_IOT_VIAL.md` **§5.8**.
>
> ⚠️ **Y no releva de nada de lo de abajo:** `D-23` hace **visible** el fallo; **las piezas `A` y
> `B` siguen siendo las que hacen que el equipo REACCIONE.** Una pantalla no pone un ámbar.

> ⚠️ **Y aun así el operario NO debe esperar nada hoy, y por dos motivos distintos:** (1) el
> Esclavo directamente no lo hace, y (2) **el ámbar del Maestro no se ejecuta nunca**, porque
> `reloj_enHora()` es **falso siempre** —`Y2` muerto, `N-17`—, así que el Degradado **no entra** y
> ese bucle **no corre**. Es `CLAUDE.md` §2.ter en estado puro: **DECLARADO y no EJERCIDO.**
>
> 🟢 **Y la consecuencia buena, que es la que hay que llevarse a la reunión: `D-20` cierra la
> pieza A sin proponérselo.** En cuanto el STM32 reciba la hora de su propio ESP32,
> `reloj_enHora()` pasa a ser la bandera correcta y **la reacción del Maestro se enciende sola**.
> `D-21` se queda entonces en **propagar el `OSF`** y **portar la guarda al Esclavo**.
>
> ⚠️ **Y una asimetría que la propia fila `D-21` no anticipa:** se dictó como *«…y lo mismo el
> Maestro»*, dando por hecho que al Maestro había que añadírselo. **Es al revés: el que lo tiene es
> el Maestro, y el que no, el Esclavo.**

#### Lo que hay que hacer HOY, mientras `D-21` no esté construida

1. **La pila del módulo `DS3231` es consumible con fecha, no mantenimiento eventual.** Se cambia en
   visita programada, en los dos postes, **antes** de que se agote.
2. **Antes de autorizar un Degradado, se lee la hora de las dos puntas con `CMD:LEER_RTC`** (`D-17`)
   y **se contrasta con un reloj de fuera**. Una fecha pasada plausible es exactamente lo que este
   riesgo describe, y hoy **el único detector es el técnico**.
3. **Si una punta devuelve una fecha que no es la de hoy: no se entra en Degradado en ese cruce.**
   Se cambia la pila y se pone en hora primero.

---

### 🔴 `D-22` y este documento — **el cristal `Y1` NO es el que decide los 29 s, y hay que decirlo antes de que alguien lo lea al revés**

> **`DECISIONES.md` fila `D-22` (07/09): `Y1` (8 MHz) pasa a ser el reloj de sistema del STM32.**
> Hoy el firmware arranca con el **HSI**, el RC interno del chip. 🛑 **DECIDIDA Y SIN CONSTRUIR.**

**Por qué se escribe aquí:** la fila `D-22` justifica el cambio comparando el error del HSI *«frente
a un margen de cruce de 29 s»*, y **esos 29 s son los de este documento**. Quien lea las dos cosas
seguidas concluirá que arrancar `Y1` amplía el margen del Degradado. **Medido el 07/09, y no es
así.**

| | qué oscilador lo cuenta | medido en |
|---|---|---|
| **La FASE del Degradado** —de quién es el verde y cuándo— | 🔴 **el RTC del STM32, sobre `LSE_CLOCK`, o sea el cristal `Y2` de 32.768 kHz** | `ciclo_degradado_fase(reloj_segundosDelDia(), …)` en el `modo_degradado.cpp` de **las dos puntas**, y `rtc.setClockSource(STM32RTC::LSE_CLOCK)` en los dos `reloj.cpp` |
| **Los 29 s de margen** | el mismo: es la deriva **entre los dos `Y2`** | comentario de cabecera de `modo_degradado.cpp`: *«el colchon que absorbe la DERIVA entre dos cristales de 32.768 kHz»* |
| `millis()` — temporizadores de transición, caché de fase, y **el cómputo de las 48 h del Esclavo** | 🟢 **el reloj de sistema: HOY el HSI, y con `D-22` sería `Y1`** | `tUltimaSync = millis()` en `Esclavo/src/modo_degradado.cpp` |

```
$ grep -rn "ciclo_degradado_fase(" 01_Firmware/Maestro/src 01_Firmware/Esclavo/src
$ grep -n "setClockSource" 01_Firmware/Maestro/src/reloj.cpp 01_Firmware/Esclavo/src/reloj.cpp
```

> 🛑 **Conclusión, y es la que hay que llevarse: `D-22` no compra ni un segundo del margen de 29 s
> mientras la fase la siga contando el `Y2`.** Lo que sí compra, y nadie lo tenía escrito:

- ✅ **El límite duro de 48 h del ESCLAVO se cuenta con `millis()`**, o sea con el HSI. A
  **10.000–25.000 ppm**, ese límite de 48 h puede caer **entre ~29 minutos y ~1,2 horas antes o
  después** de las 48 h reales *(cuenta: `48 h × 0,010` y `48 h × 0,025`; no es una medida de
  banco)*. Con `Y1` a 20–50 ppm el mismo error baja a **entre 3,5 y 8,6 segundos**.
- ✅ **Todos los plazos de `millis()` heredan ese error**: el techo de silencio de `SFTY-6`
  (25 s → ±0,25 a ±0,63 s), la ventana de reintentos de 20,5 s, y el watchdog de 4 s.

> ⚠️ **Y la pregunta que esto abre y que este documento NO puede contestar** —queda en el informe
> de la sesión, no aquí—: **`Y2` está muerto (N-17), así que hoy `reloj_segundosDelDia()` no cuenta
> nada y el Degradado no entra en ninguna punta.** Cuando `D-20` se construya, **el STM32 tendrá que
> contar la hora sembrada por su ESP32 con ALGO**, y si ese algo es `millis()` —la vía *«reloj de
> software disciplinado por el ESP32»* que `11_Manual_Instalacion_RTC_DS3231_Bateria.md` §5.4 deja
> abierta como `BLQ-2`—, **entonces `Y1` SÍ pasa a decidir los 29 s y la justificación de `D-22` es
> exacta.** Depende de una decisión que todavía no está tomada.

---

## 7. Resumen de parámetros

| Parámetro | Valor | Dónde está |
|---|---|---|
| Verde de cada punta | **30 s** | `DEG_VERDE_SEG` |
| Todo-rojo entre verdes | **30 s** *(ya ampliado — el normal son 15 s)* | `DEG_DESPEJE_SEG` |
| Ciclo completo | **120 s** · espera máxima 90 s | 2 × (30 + 30) |
| Antigüedad máxima de la sincronización para entrar | **2 h** | `SYNC_FRESCA_MS` |
| Tolerancia de desfase para entrar | **±3 s** | `TOLERANCIA_DESFASE_S` |
| Aviso de límite — **MAESTRO** | a partir de **44 h** — 🔴 **HOY NO HAY DÓNDE LEERLO** *(§4)* | `AVISO_LIMITE_MS` |
| Aviso de límite — **ESCLAVO** | a partir de **40 h** *(🔴 no es el mismo número — añadido el 07/09)* — 🔴 **tampoco hay dónde leerlo** | `AVISO_SIN_SYNC_MS` |
| **Límite duro → ámbar** | **48 h** *(se ve DESPUÉS, como `MODO:RENDIDO`)* | `LIMITE_DURO_MS` **(Maestro)** · `LIMITE_SIN_SYNC_MS` **(Esclavo)** — 🔴 **dos nombres, un concepto** |
| 🔴 **Requisito 1 · reloj en hora — MAESTRO** | **HOY NO SE PUEDE CUMPLIR.** El único llamador vivo de `reloj_ajustar()` es la pantalla `AJUSTAR HORA`, y el menú está tapiado (`D-15`, `D-17.bis`) | `reloj_enHora()` de `Maestro/src/reloj.cpp` |
| 🔴 **Requisito 1 · reloj en hora — ESCLAVO** *(añadido el 07/09: esta tabla decía `reloj_enHora()` **sin distinguir punta**, y las dos puntas están bloqueadas por motivos DISTINTOS)* | **HOY NO SE PUEDE CUMPLIR, y no por culpa del Maestro.** `reloj_setup()` sale antes de `rtc.begin()` si el `LSE` —el cristal `Y2`— no arranca; sin él ni `reloj_ajustar()` escribe. **La bandera es `false` siempre en esta punta** | `reloj_enHora()` de `Esclavo/src/reloj.cpp` · guarda en `degradado_comprobar()` |
| 🟢 **Lo que abre las dos** | `DECISIONES.md` fila **`D-20`** (07/09): la autoridad de la hora pasa al `DS3231` del ESP32 y el STM32 la recibe de su propio módulo. ~~🛑 **DECIDIDA y SIN CONSTRUIR**~~ 🟢 **construida** (`9dd8bbf` + `68dd2c5`, reglas de `D-26`), **sin banco** | `roadmap.md` §3.4.bis y §0 |
| **Entrada por app** *(la vigente, `D-18`)* | `CMD:PIN:1234:SET_MODO:DEGRADADO` — **en las dos puntas** | `grep -n 'SET_MODO:DEGRADADO' Maestro/src/bluetooth.cpp Esclavo/src/bluetooth.cpp` |
| ~~Secuencia de entrada desde el piso~~ | ~~`A · B · A · B` en ≤ 18 s → **4 destellos rojos**~~ | ~~`mando.cpp:204-214`~~ |
| ~~Secuencia a Automático~~ | ~~`A · A · A` en ≤ 12 s → **2 destellos rojos**~~ | ~~`mando.cpp:225-227`~~ |
| ~~Secuencia a Ámbar~~ | ~~`B · B · B` en ≤ 12 s → **3 destellos rojos**~~ | ~~`mando.cpp:230-234`~~ |

> ✅ **CORREGIDO EL 02/09 — las tres secuencias siguen en el firmware.** Una versión anterior de
> este documento las daba por retiradas; **el CÓDIGO se conserva** en sus canales `A` (`PB9`) y `B`
> (`PB13`). Lo que se retiró son los pulsadores 3 y 4, que estas secuencias **no usan**.
>
> ~~⚠️ **Pero no se pueden ejercer todavía: el receptor RF nunca se compró.** Es una compra que
> falta, no una función perdida.~~
>
> 🔴 **07/09 — `D-1`: NO ES UNA COMPRA QUE FALTA. EL MANDO NO EXISTE.** Las tres filas se tachan
> porque **ninguna de las tres se puede ejecutar en el equipo de campo**, y una tabla de referencia
> que las lista sin tachar hace que alguien las busque en obra. **La entrada vigente es la fila de
> arriba, por app y en las dos puntas** (`D-18`).
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

> ✅ **TABLA AL DÍA (07/09/2026).** La columna «Qué hacer» se rehízo: la app tiene hoy órdenes que
> en agosto no existían, incluida la que **entra en Degradado en el Esclavo** (`D-18`) y la que lo
> apaga. ~~**Sin receptor RF, las respuestas por mando no se pueden ejercer**~~ → **`D-1`: no hay
> mando. Todas las respuestas de esta tabla son por app.**

| Síntoma | Causa probable | Qué hacer |
|---|---|---|
| **El Esclavo rechaza `SET_MODO:DEGRADADO`** con un `$ERR` | Falta alguno de los ~~5~~ **6** requisitos de **su** lista (§2.2 — no son los del Maestro) | **Lea el `DESC:` — dice cuál falta.** ~~*«Sin hora»* se arregla sincronizando~~ ~~🛑 **07/09: `SIN HORA VALIDA` NO SE ARREGLA HOY DE NINGUNA FORMA, Y ESTA CASILLA MANDABA A INTENTARLO.** Es el requisito 1 y el reloj de esa punta está bloqueado por su propio cristal: **no hay sincronización, orden ni pila que lo cambie**. Lo destraba `D-20`, decidida el 07/09 y **sin construir**. **Anótelo como NO PROBADO, no como avería**~~ 🔵 **11/09 (`D-26`, `68dd2c5`, sin banco): *«sin hora»* se arregla poniéndole la hora a ESE poste** («⏱️ Sincronizar»), o esperando la siembra de su ESP32 (≤ 5 min); si no llega, mire la alarma `HORA_ESP32` · *«sync caducada»* obliga a arreglar el radio · `AMBAR EMERG.PUESTO` se retira con `CMD:PIN:1234:CANCELAR_AMBAR` |
| **El Esclavo entra y a los pocos segundos vuelve solo a subordinado** | ✅ **No es una avería:** el Maestro sigue vivo y su `CMD_PING` cada 3 s lo saca. Este modo es para cuando la radio está **muerta** | Compruebe primero que el enlace de radio esté realmente caído |
| ~~La secuencia `A·B·A·B` responde con **ámbar rápido** en vez de 4 destellos~~ | ~~Falta alguno de los 5 requisitos~~ | ⛔ **`D-1`: no hay mando.** Entre **desde la app** con `CMD:PIN:1234:SET_MODO:DEGRADADO` |
| ~~La secuencia **no responde nada**~~ | ~~Sin receptor RF no hay quien genere los pulsos~~ | ⛔ **`D-1`: no hay mando.** Use la app |
| Las dos puntas **ciclan pero desfasadas** | Relojes separados, o una unidad se reinició | **Ámbar en las dos, y no corrija a ojo.** Maestro: `CMD:PIN:1234:SET_MODO:AMBAR`. Esclavo: `CMD:AMBAR_EMERGENCIA` |
| 🔴 **Una punta en verde y la otra en ámbar** | Riesgo residual nº 2 — salida asimétrica | **Apague la punta que da verde, ya.** Si es el **Esclavo**: `CMD:AMBAR_EMERGENCIA` (`Esclavo/src/bluetooth.cpp:381`, **no pide PIN** justamente por esto). Si es el **Maestro**: `CMD:PIN:1234:SET_MODO:AMBAR`. **`CMD:FORZAR_ROJO` NO sirve en el Esclavo** (`:448`): esa punta lo **rechaza** con `RENOMBRADO_USE_AMBAR_EMERGENCIA`, así que no para nada y el ciclo por reloj volverá a dar verde en la fase siguiente |
| 🟡 **Puse ámbar de emergencia en el Esclavo y ya no hace falta** | — | `CMD:PIN:1234:CANCELAR_AMBAR`. **Pide PIN al revés que el de poner**, porque quitarlo devuelve el cruce a dar verdes. Contesta `RETIRADO`. 🔵 **`RETIRADO_QUEDA_MANDO` no se puede ver hoy** (`D-1`: no hay mando que arme `ambarLocal`); si apareciera, la luz seguiría vetada y **eso sería el hallazgo**, no la respuesta esperada |
| ❓ **Estoy delante de un poste y no sé si es el Maestro o el Esclavo** | Módulo Bluetooth recién puesto | El módulo se auto-rotula **`SEM-<serie>-M`** o **`SEM-<serie>-E`**. **Si se anuncia `SEM-SIN-MATRICULA` todavía no lo ha aprendido, y con dos módulos nuevos LOS DOS SE LLAMAN IGUAL.** Déjelo un minuto encendido y **déle una vuelta de energía**: el nombre bueno sale en el arranque siguiente. Mientras tanto, la punta la dice el campo `NODE:` del `$STATUS` en la app, no el nombre Bluetooth |
| ~~Pantalla: `Limite 48h sin sync`~~ → **`MODO:RENDIDO` en el `$STATUS` del Esclavo** | Se agotó el límite duro de 48 h y la punta se rindió sola a ámbar | Es correcto. Hay que **arreglar el radio**, no reactivar el modo — y el firmware **no deja reentrar**: contesta `$ERR,…,DESC:SYNC CADUCADA >48h`. 🔴 **07/09: la «pantalla» de esta fila ya no existe** (`D-17.bis`); lo que sí se ve es el literal **`RENDIDO`** del campo `MODO:`, que el firmware separa de `DEGRADADO` a propósito *(«pintarlo como DEGRADADO diría que el cruce sigue operando por reloj cuando ya no opera»)*. El tope no necesita actuador |

> ⏱️ **En la fila roja, cuente con que el ámbar del Esclavo puede tardar de 10 a 90 s**: sale por
> todo-rojo a propósito. `RESULT:SALIENDO_TODO_ROJO` significa *va en camino*, no *ya está*. **No se
> vaya del cruce hasta verlo.**

---

## 9. Lo que este procedimiento NO cubre

Escrito aquí porque una limitación documentada vale más que una promesa:

- **No hay prueba de banco ni de campo todavía.** Todo lo anterior está validado en simulador.
- ~~**El Esclavo no tiene receptor de mando** (N-19). Todo lo que este documento dice del mando
  aplica **solo al Maestro**.~~ → 🔴 **07/09 (`D-1`): NINGUNA punta tiene mando.** No hay emisor, no
  hay pulsadores y el receptor RF nunca se compró. **Todo lo que este documento dice del mando
  describe código vivo sin actuador**, y por eso va tachado allí donde era una instrucción.
- 🛑 **`D-16`: SIN TELÉFONO NO HAY FORMA DE OPERAR EL EQUIPO.** Es la limitación mayor de este
  procedimiento y va aquí, en el apartado de lo que no se cubre, porque no la resuelve el firmware.
- 🛑 **`D-23` (07/09): EL DIAGNÓSTICO DEL POSTE 2 POR SÍ MISMO NO EXISTE TODAVÍA.** Está decidido
  —*«la app, cuando se conecta a ese lado del Esclavo vía Bluetooth, podrá diagnosticarlo»*— y **no
  hay ni una línea escrita**. Mientras tanto, quien suba al Poste 2 tiene **exactamente** esto: el
  `$STATUS` cada 2 s (`MODO` · `ESTADO` · `HORA` · `PLUMA` · `CAM`, con `T`/`RF`/`RTT`/`BAT` en `--`
  porque esa punta **no puede** medirlos), los `$EVENT` y `$ALARM` que el equipo emite por su cuenta,
  y **una sola consulta**: `CMD:LEER_RTC`. **No hay forma de preguntarle cuánto le queda a las 48 h.**
  Censo completo en `14_Manual_App_Movil_IOT_VIAL.md` §5.8.
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
  > ~~🔴 **Y lo que sigue SIN sobrevivir, que es lo que este procedimiento necesitaba:** no hay
  > registro de *«esta punta estaba en Degradado»*. Un corte de energía en Degradado **no lo
  > reanuda**: el equipo vuelve a la espera de selección de modo. Eso es correcto y es la dirección
  > segura —reanudar solo un verde por reloj sin que nadie mire las dos puntas es exactamente lo que
  > este modo existe para evitar—, pero **hay que saberlo antes de irse del cruce**.~~
  >
  > 🔴 **TACHADO EL 07/09 — ES FALSO, Y ERA LA SEGUNDA COPIA DE LA MISMA FRASE FALSA** (la otra
  > estaba en la Sección 2, donde va la corrección larga y su `grep`). **`respaldo_degradadoActivo()`
  > SÍ registra que esta punta estaba dentro**, y `modo_degradado_reanudarTrasCorte()` /
  > `degradado_reanudarTrasCorte()` **tienen llamador vivo en el `main.cpp` de las dos puntas**:
  > el equipo **reanuda el Modo Degradado solo**, arrancando en todo-rojo, si al volver la energía
  > siguen vigentes reloj en hora + ciclo en la pila + sincronización fechable por debajo de 48 h.
  >
  > 🛑 **Y el registro que falta es OTRO, más pequeño y menos grave:** falta la fila que dijera
  > *«esta punta reanudó sola»*. **Nadie avisa al operario de que la reanudación ocurrió**, así que
  > la única forma de saberlo es mirar `MODO:` en el `$STATUS`. **La verificación visual de las dos
  > puntas tras un corte no la pide el equipo: la tiene que recordar la persona.**
- **La configuración del ciclo se sincroniza pero todavía no se consume** en el cálculo del ciclo
  (N-18): hoy ambas puntas usan los 30/30 fijos compilados. Mientras los dos firmwares sean de la
  misma versión, coinciden — **pero flashear versiones distintas en cada punta rompería la fase sin
  aviso**.
- **El RTC no se ha contrastado contra hora patrón** ni se ha comprobado que conserve la hora tras
  desconectar la alimentación (N-15, N-17).
- ⚠️ **Lo que sigue abierto en la entrada y la salida (medido el 02/09):**
  - ~~**El Esclavo no tiene comando Bluetooth de entrada** en Degradado. Su única puerta es la
    secuencia `A · B · A · B` del mando, y **el receptor RF no se ha comprado**. Mientras siga así,
    este procedimiento no se puede completar en las dos puntas.~~

    > ~~🟢 **Y al 04/09 esto CAMBIA DE NATURALEZA, no de estado.** … **no viene un `SET_MODO` para
    > el Esclavo**… **El bloqueo es el mismo; lo que se sabe hoy es que no se va a resolver por
    > software.**~~
    >
    > 🟢 **CERRADO EL 05/09 Y TACHADO EL 07/09 — `DECISIONES.md` `D-18`: SÍ SE RESOLVIÓ POR
    > SOFTWARE.** *«No viene un `SET_MODO` para el Esclavo»* aguantó **un día**. La orden es
    > `CMD:PIN:1234:SET_MODO:DEGRADADO`, la misma que en el Maestro, y **este procedimiento SÍ se
    > completa en las dos puntas**.
    >
    > **Lo que se conserva de este párrafo, porque es la lección y no el dato:** una ausencia
    > publicada se convierte en una decisión del responsable. Aquí se le dijo *«no hay puerta»*
    > cuando la puerta —`degradado_entrar()`— llevaba meses construida y probada; **lo que faltaba
    > era la llave**. `CLAUDE.md` §4, quinta cara.
    >
    > 🔴 **Y lo que SÍ sigue abierto de esto, medido el 07/09:**
    > `grep -c reportarEvento 01_Firmware/Esclavo/src/modo_degradado.cpp` → **0**. El Esclavo entra
    > por app y **no publica nada propio sobre ese estado**: sólo el `$ACK` de la orden y el campo
    > `MODO:` del `$STATUS`. **Si el operario necesita verlo de otra forma, lo decide el
    > responsable.**
  - Las salidas por app del Maestro (`SET_MODO:AUTO`, `SET_MODO:AMBAR`) **se saltan el todo-rojo de
    despedida** que sí hacía el `Botón 3`.
  - El ámbar de emergencia del Esclavo **sí existe ya** por app y **sale por todo-rojo**, tardando
    de 10 a 90 s. Ver el aviso de cabecera y la Sección 8.
