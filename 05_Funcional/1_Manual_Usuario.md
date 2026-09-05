# 🚦 Manual de Operación y Comportamiento del Sistema (V9.0 Definitiva)

Este manual define el **"Ground Truth"** (la verdad absoluta) de cómo DEBE comportarse el sistema, sirviendo como base para validar que las simulaciones y el código cumplan con la especificación.
Todas las operaciones están alineadas al **Manual de Señalización Vial de Colombia (Resolución 2024 - MinTransporte)**.

---

> ## 🔴 BANCO DEL 3–4/09/2026 — TRES COSAS DE ESTE MANUAL CAMBIAN DE SIGNIFICADO
>
> **Es la primera vez que la tarjeta se ejerce cargada desde el 31/07.** Nada de esto se descubre
> leyendo el fuente: sale de tener el equipo delante.
>
> 1. **Al encender, el equipo se queda EN ESPERA DE SELECCIÓN DE MODO. NO ES UNA AVERÍA.** Con la
>    pantalla y los pulsadores retirados **no queda nada local que arranque un modo**: el equipo
>    espera a que se le ordene uno **desde la app**. Mientras espera muestra **🔴 rojo fijo** si hay
>    comunicación con la otra punta, y **🟡 ámbar intermitente** si no la hay. En banco se
>    interpretó como fallo del equipo, y no lo era. Ver §3.
> 2. **El mando de relés `A`/`B` ESTUVO SORDO EN BANCO (`N-118`), Y EL FIRMWARE YA ESTÁ CORREGIDO —
>    PERO NO SE HA EJERCIDO EN TARJETA.** Hay que separar dos cosas que no son la misma:
>    * **El defecto, en pasado:** `MANDO_A`/`MANDO_B` se leían con `INPUT_PULLUP` y `== LOW`, y con
>      las `R65`/`R66` (10 kΩ a masa) el pin se quedaba en **0,6 V — BAJO permanente**. No había
>      flanco, así que **ninguna de las tres secuencias de §7 era alcanzable**.
>    * **Hoy:** las **dos puntas** leen esos pines con `pinMode(..., INPUT)` **pelado** y
>      `digitalRead(...) == HIGH` —**activo en ALTO**, igual que las cámaras—. **MEDIDO en el
>      fuente:** `Maestro/src/botones.cpp:160-161` y `:223`, `Esclavo/src/botones.cpp:178-179` y
>      `:232`. **Cambia el gesto de prueba:** ya **no** es un cable a masa, es **cerrar contra los
>      3,3 V del pin de al lado** — `J16` **p5 contra p4** para el canal `A`, **p8 contra p7** para
>      el `B`.
>    * ⚠️ **Lo que NO se puede decir todavía:** que el mando funcione. **El firmware corregido no se
>      ha cargado ni ejercido en tarjeta**; lo único medido es el fuente. Hasta esa prueba, **no
>      cuente con respaldo físico**: opere por la app. Ver §7.
> 3. **La app sólo conecta con la APK del 04/09**
>    (**la APK del 04/09 que acompaña a este paquete** — su nombre exacto y su `md5` estan en `LEEME_PRIMERO.md`, en la raíz del `.zip`, que es el único sitio donde no caducan). Hasta esa versión la app **nunca abría el socket
>    Bluetooth**: se pintaba «Enlazado» por haber pulsado una fila de la lista, y los comandos se
>    iban al vacío (`N-122`). **Y esa misma APK trae `N-124`: la lista de equipos ya NO lleva
>    direcciones `MAC` escritas a mano — sale del escaneo real.** Por eso el orden en el poste es
>    **primero EMPAREJAR el `ESP32` en Ajustes de Android, y después pulsar «Buscar Módulos
>    Bluetooth» en la app**. Ver §8 y el Manual 14.
>
> **Y lo que este banco NO pudo contestar, que es lo que hay que saber antes de creer que algo está
> aprobado:** la **cabecera del informe de banco** dice **24 completos · 4 bloqueados por el enlace
> Bluetooth · 1 abortado por un incidente de seguridad**, sobre 29 pasos. **Esa cuenta se cita como
> lo que es —la cifra de la cabecera— y NO se publica aquí como hecho: la propia enumeración del
> informe no cuadra con ella.** Sus tres cajones nombran 22 identificadores y hay siete pasos que no
> caen en ninguno. La discrepancia está medida y desglosada en
> `12_Cobertura_de_Pruebas_y_Huecos.md`, que por el mismo motivo **se niega a publicar un total**.
> **Reconciliarla no es cosa de este manual: lo decide quien ejecutó la sesión.**
>
> Lo que sí es firme, y no depende de esa cuenta: como el equipo nunca llegó a operar por falta de
> app, la regresión del Modo Automático (`N-42`, *«no mueve las luces»*) **no se confirmó NI se
> descartó**. Sigue abierta, y no se cuenta como pasada.

---

> ## 🟢 CAMBIOS DEL 04/09/2026 — CUATRO COSAS QUE CAMBIAN LO QUE SE HACE EN EL POSTE
>
> **Los cuatro están en el firmware y verificados en el fuente.** Ninguno se ha ejercido en tarjeta:
> lo que sigue vigente arriba —**nada sube a campo sin banco pasado**— no lo levanta ninguno de
> ellos.
>
> 1. **EL CICLO MÍNIMO SUBE DE 1 A 3 MINUTOS.** Verde **3–15 min**, rojo **3–15 min**, despeje
>    **10–90 s**. Es una **decisión vial del responsable**: *«tres minutos es la mínima distancia de
>    seguridad»*. **Donde este manual —o cualquier otro— diga «1 a 15» o «mínimo 1 minuto», está
>    obsoleto.** El equipo rechaza lo que quede fuera con `$ERR,CMD:SET_TIEMPOS,DESC:RANGO`. Ver §1.
> 2. **EL CRUCE SE OPERA DESDE EL MAESTRO.** No se hace transparente el mando desde el Esclavo:
>    **para dar paso, cambiar de modo o ajustar tiempos hay que conectarse al poste Maestro.** Desde
>    el Esclavo sólo se puede pedir paso, poner ámbar de emergencia y retirarlo. Ver §9.
> 3. **CÓMO SE DISTINGUE UN POSTE DEL OTRO, QUE AHORA ES CRÍTICO.** El módulo se auto-rotula
>    `SEM-<serie>-M` o `SEM-<serie>-E`… **pero lo aprende del `$STATUS` y lo guarda para la
>    SIGUIENTE arrancada.** **Un módulo recién puesto anuncia `SEM-SIN-MATRICULA`, y los dos postes
>    se llaman igual hasta que se les da una vuelta de energía.** Ver §8.
> 4. **`N-130`: EL EQUIPO YA NO DICE QUE SÍ A LO QUE NO VA A HACER.** Pedir paso desde el Esclavo
>    con el cruce fuera del Modo Inteligente produce el evento
>    `MAESTRO / DEMANDA_NO_ATENDIDA_MODO_ACTUAL`. **Es comportamiento normal, no una avería.** Ver §9.

---

> ## 🟢 04/09/2026, MÁS TARDE EL MISMO DÍA — CINCO COSAS MÁS, Y DOS CAMBIAN LO QUE HACE EL OPERARIO
>
> **Los cinco están MEDIDOS en el fuente y NINGUNO se ha ejercido en tarjeta.** Sigue vigente lo de
> arriba: **nada sube a campo sin banco pasado.**
>
> | # | qué cambia | dónde |
> |---|---|---|
> | **1** | 🔴 **EL ÁMBAR SE ORDENA (`N-134`).** Antes, poner ámbar desde el Maestro dejaba al Esclavo en ROJO y era él quien se iba a ámbar **25 s después, por orfandad**. En banco se veía como *«a veces los dos, a veces sólo el maestro»*. Hoy hay `CMD_GO_AMBAR` (`0x13`); **el rojo previo se queda como intermedio seguro** y **la orfandad de 25 s sigue como red** si la orden se pierde. **Decisión del responsable** | §4 |
> | **2** | 🟢 **LOS TIEMPOS DEL CICLO SOBREVIVEN AL CORTE (`N-133`).** 🛑 **Pero la primera vuelta de energía con este firmware BORRA los tiempos guardados y el reloj** — es correcto y está diseñado así. **Léalo antes de reportarlo como avería** | §3 · §1 |
> | **3** | 🟢 **EL MODO AUTOMÁTICO ARRANCA CORRIENDO, sin cuestionario (`N-42`).** Y su consecuencia: **estando en Automático NO se pueden cambiar los tiempos** — hay que salir, ponerlos y volver a entrar | §1 · §3 |
> | **4** | ✅ **`N-106` CERRADO:** el ámbar de emergencia de la app **sí** saca al Esclavo del Degradado, por el todo-rojo, y contesta **cinco cosas distintas** en vez de `OK` siempre | §9 |
> | **5** | 🔴 **LA APP CAMBIÓ DE BARRERA:** para **dar paso** y para **arrancar el ciclo** ya no se teclea el PIN — la app pregunta **si ha mirado el tramo**. **Nunca pregunta para PARAR.** Y el PIN **caduca** | §3 · Manual 14 |
>
> ### 🔴 El nº 5, dicho entero, porque es lo que cambia en la mano del operario
>
> > **El equipo no sabe si quedan vehículos en el tramo, y el operario sí. Un PIN demuestra QUIÉN
> > ERES; no demuestra que hayas MIRADO.**
>
> * **DAR PASO** y **AUTOMÁTICO** abren un diálogo que pregunta **«¿No quedan vehículos en el
>   tramo?»**, con la lista de lo que hay que mirar, y dos botones: *«Todavía no»* y *«He mirado: el
>   tramo está libre»*.
> * 🛑 **ROJO TOTAL y ÁMBAR no preguntan nada, ni PIN ni tramo.** Es deliberado: **es la dirección
>   segura, y preguntar para parar enseña a decir que sí sin leer.**
> * ⏱️ El «he mirado» **caduca a los 30 s** y **también en cuanto cambian las luces**: el tramo que
>   se miró ya no es el tramo que se va a abrir.
> * 🟢 **El PIN caduca**: a los **60 s** de guardarse el teléfono (con esa gracia, para que consultar
>   un mensaje no cierre la sesión) y a los **5 min sin mandar ninguna orden**. **El técnico lo va a
>   notar**, y no es una desconexión: el enlace sigue.
>
> **El detalle de todo esto —qué órdenes siguen pidiendo PIN, el DIARIO DE ÓRDENES que hay que
> exportar cuando algo falle, y los 23 motivos de rechazo ya traducidos— está en
> `14_Manual_App_Movil_IOT_VIAL.md`, §4.bis.3, §4.bis.4 y §5.4.2.bis.**
>
> ⚠️ **Y lo que NO ha cambiado, para que nadie concluya de más:** el PIN **sigue viajando en la
> trama** y el firmware lo sigue exigiendo donde siempre. Lo que cambió es **a quién se le pregunta
> en la pantalla**.

---

> ## 🔴 CAMBIO DEL 28/08/2026, **AL DÍA EL 31/08/2026** — LA PANTALLA DEJA DE SERVIR, Y EL MENÚ YA NO SE PUEDE EJECUTAR
>
> **El equipo va montado en alto y la pantalla no se lee desde el suelo.** Una LCD de 128×64 dentro
> del gabinete, a 5 m, no la mira nadie: para consultarla hacía falta escalera o canasta, que es
> justo lo que el operario no tiene delante cuando hace falta.
>
> **La interfaz de operación pasa a ser la app por Bluetooth.** El módulo entra por el conector
> **`J17`**, en los mismos pines que dejó libre la pantalla.
>
> > ✏️ **PRECISIÓN DEL 31/08 — «se retira la pantalla» era una frase corta para dos cosas
> > distintas, y sólo una es cierta.** MEDIDO en el fuente:
> >
> > * **El código de la pantalla NO se ha retirado.** `lcd.cpp`, `menu.cpp` y los siete modos siguen
> >   compilándose, y las 271 comprobaciones de su arnés siguen midiéndolos.
> > * **Lo que se hizo es DEJAR DE CONDUCIR SUS PINES.** `PB3`, `PB4` y `PB5` pasan a
> >   `U8X8_PIN_NONE` (`Maestro/src/lcd.cpp:74`): no queda **ni un `pinMode` ni un `digitalWrite`**
> >   sobre ellos, y los tres se quedan en alta impedancia.
> > * **Y hay un motivo eléctrico, no de gusto:** `J17` es **un solo conector**, y ahí es donde
> >   entra ahora el módulo Bluetooth. **No pueden estar los dos enchufados.** Además `PB3` es un
> >   reloj de bus que conmuta en cada bit, pegado al RX/TX del módulo dentro del mismo mazo: es
> >   justo lo que produce corrupción intermitente del enlace serie.
> >
> > **Para el operario el efecto es el mismo —no hay display—**, pero la frase importa para quien
> > lea este manual buscando qué se puede volver a encender: **hacen falta dos cosas, no una**,
> > devolver los pines *y* sacar el módulo de `J17`.
>
> ### Consecuencia operativa: YA NO HAY MENÚ LOCAL
>
> Todo lo que este manual describía como *«en la pantalla»*, *«4 = Menú»*, *«`CONFIGURACION` →
> `AJUSTAR HORA`»* o *«cuarta opción del Menú Principal»* **deja de estar disponible en el equipo**
> y se hace **desde la app**. Los párrafos afectados **no se han borrado**: se han marcado, con lo
> que los sustituye al lado. Borrarlos en silencio dejaría al operario buscando en la app una
> función que aquí figuraba y que quizá aún no exista allí.
>
> ### 🛑 31/08 — **NINGÚN PROCEDIMIENTO DE ESTE MANUAL QUE DIGA «NAVEGUE AL MENÚ» O «PULSE ACEPTAR» SE PUEDE EJECUTAR**
>
> > ⚠️ ~~**LOS BOTONES SIGUEN AHÍ, Y EL MENÚ TAMBIÉN — SOLO QUE A CIEGAS.** Medido sobre el firmware
> > el 28/08: el menú **no se ha retirado del binario**. `lcd.cpp`, `menu.cpp` y `modo_hora.cpp`
> > siguen compilándose, y los cuatro botones del conector `J16` siguen navegándolo. Lo único que
> > falta es el display donde se vería el resultado.~~
> >
> > ~~**Eso significa que pulsar los botones —o accionar el mando de relés, que va cableado en
> > paralelo con ellos— sigue moviendo un menú que nadie ve.** Con los pulsos suficientes se llega a
> > `AJUSTAR HORA` y **se confirma una hora cualquiera que el equipo dará por válida**, sin ningún
> > aviso.~~
> >
> > 🟢 **CADUCADO EL 31/08, y en la dirección segura. El aviso era correcto el 28/08 y hoy es
> > FALSO: ya no se puede confirmar nada.** Se conserva tachado porque describe un riesgo real que
> > existió, y porque quien lo leyera y lo diera por vigente estaría teniendo miedo del peligro
> > equivocado.
>
> **Lo que hay HOY, MEDIDO en el fuente el 31/08:**
>
> | | dónde | qué hace |
> |---|---|---|
> | `botonAceptar()` | `Maestro/src/botones.cpp:305` · igual en el Esclavo | **devuelve `false` SIEMPRE** |
> | `botonCancelar()` | `Maestro/src/botones.cpp:306` · igual en el Esclavo | **devuelve `false` SIEMPRE** |
> | `PB14` (`J16` p10) | `Maestro/include/pines.h:124` | ya no es `BOTON3`: es **`CAM_C_PIN`**, entrada de cámara |
> | `PB15` (`J16` p12) | `Maestro/include/pines.h:125` | ya no es `BOTON4`: es **`CAM_D_PIN`**, entrada de cámara |
>
> **`ACEPTAR` era el botón que EJECUTA y `CANCELAR` el que SALE. Los dos se han quedado sin pin.**
> Los pulsadores 3 y 4 pasaron a ser entradas de cámara, así que:
>
> * **No se puede confirmar nada en el menú** — `menu.cpp:111` pregunta por `botonAceptar()`, que
>   nunca es cierto. Ni una hora, ni un tiempo, ni un modo.
> * **No se puede salir de ningún modo con el botón** — `menu.cpp:148` pregunta por
>   `botonCancelar()`, que nunca es cierto.
> * **El cursor todavía se mueve** —`botonArriba()`/`botonAbajo()` cuelgan de `PB9`/`PB13`, que
>   siguen vivos— pero **no lleva a ninguna parte**, y no hay display donde verlo.
>
> 🛑 **CONSECUENCIA PARA QUIEN USA ESTE MANUAL: cualquier procedimiento de aquí abajo que empiece
> por «navegue al menú», «entre en `CONFIGURACION`», «suba hasta…» o «pulse Aceptar» NO SE PUEDE
> EJECUTAR.** No es que sea incómodo por no haber pantalla: es que **el firmware ya no tiene por
> dónde recibir esa orden**. Se hace desde la app, y donde la app todavía no llega está escrito
> abajo en voz alta, sin darlo por trasladado.
>
> **Y sigue en pie lo único que quedaba de aquel aviso:** **no accione los botones del gabinete «a
> ver qué pasa»**. Ya no pueden confirmar una hora falsa, pero `PB9` y `PB13` son **el mando**, y
> tres pulsos seguidos cambian el modo del semáforo. Ver §7.
>
> 🟠 **MATIZ DEL 04/09, y tiene dos mitades que hay que leer juntas (`N-118`):**
>
> * **En banco, el 3–4/09, esos pulsos NO cambiaban nada.** Con `INPUT_PULLUP` + `== LOW`, las
>   `R65`/`R66` (10 kΩ a masa) dejaban `PB9`/`PB13` en **0,6 V — BAJO permanente**: no había flanco
>   y el reconocedor de secuencias nunca arrancaba.
> * **El firmware de hoy lee esos pines al revés** —`INPUT` pelado y `== HIGH`, **activo en ALTO**,
>   en las dos puntas (`Maestro/src/botones.cpp:160-161`, `Esclavo/src/botones.cpp:178-179`)—, así
>   que **el aviso vuelve a valer en cuanto ese firmware esté cargado**. Y en un equipo cargado los
>   pines quedan **en reposo a 0 V por las mismas `R65`/`R66`**: lo que ahora dispara es **cerrar
>   contra 3,3 V**, no tocar masa.
>
> **Por eso no se tacha:** quien lea sólo la mitad del banco desconfiará de unos botones que el
> firmware nuevo vuelve a atender. **Lo que aún no está medido es la tarjeta.** Ver §7.

---

## 1. Comportamiento Físico de las Luces (Secuencia Normativa Colombia)

Para evitar arranques prematuros y dar tiempo de frenado, la secuencia lumínica **debe** operar de la siguiente manera:

1. 🔴 **ROJO FIJO:** Vía cerrada.
2. 🟡 **AMARILLO FIJO:** (Duración estricta de 4.0 segundos avisando el arranque inminente en Maestro y Esclavo).
3. 🟢 **VERDE FIJO:** Vía libre.
4. 🔴 **ROJO FIJO:** (Transición directa desde el verde, 0s de aviso). Vía cerrada.

### 🚧 La talanquera — su reposo y su fallo NO son lo mismo, y hay que leerlos juntos

**MEDIDO el 04/09** sobre los símbolos `MOTOR_TALANQUERA`, `TALANQUERA_ABRIR` y `TALANQUERA_CERRAR`
de `Maestro/include/pines.h` y el único `digitalWrite(MOTOR_TALANQUERA, …)` de
`escribirPines()`, en `Maestro/src/semaforo.cpp` *(idéntico en el Esclavo)*.

> ✏️ **Aquí no van números de línea a propósito.** Ese fichero se está tocando hoy mismo: el
> `digitalWrite` de la pluma pasó de la línea 93 a la 100 **durante la redacción de este apartado**.
> **Un número de línea en un manual caduca solo**; el nombre de la función y del símbolo, no. Se
> busca con `grep -n MOTOR_TALANQUERA`.

**Sale por la bornera `J15`, NUNCA por `J14`.** `J14` es una **ENTRADA** del micro (`PB0`, 3,3 V, sin
opto ni diodo): un relé cableado ahí mete tensión directa a la pata del microcontrolador. 🛑 **Si
alguien ya cableó un relé a `J14`, desconéctelo ANTES de energizar.**

La cadena es un **canal de potencia propio, igual que cada luz**:

```
   PB2 --> R70 220 / R69 10K --> opto TLP127 (U15) --> MOSFET IRLZ44N (Q10) --> bornera J15
```

Son **DIEZ MOSFET y DIEZ optos** en la placa (`Q1..Q10`, `U6..U15`), no nueve: el zumbador tiene el
suyo (`U13`/`Q8`/`J13`) y la talanquera el suyo.

| situación | la pluma | por qué |
|---|---|---|
| **Reposo / arranque** | 🔽 **ABAJO** | **SFTY-28.** En `LOW` el MOSFET no conduce, el motor queda sin energía y la pluma cierra. **Es el fallo seguro** |
| **Sin corriente** | 🔽 **ABAJO** | Lo mismo por otra vía. La especificación de compra pide además **actuador con retorno por muelle o gravedad**, porque eso el software no lo garantiza |
| **Verde** | 🔼 arriba | Da paso |
| **En FALLO (`S_FALLO`, ámbar intermitente)** | 🔼 **ARRIBA** | 🟢 **ES QUERIDO, y está confirmado por el responsable el 04/09/2026.** Es para **no dejar coches atrapados detrás de una pluma que ya no señaliza nada** |

> 🟢 **El `|| estado == S_FALLO` NO es un descuido, y por eso se escribe aquí.** Hasta el 04/09 esa
> decisión vivía dentro de un operador ternario **sin motivo escrito**, y quien la leyera de paso la
> tomaría por un error —contradice el reposo de la línea de abajo— y la *«arreglaría»*.
>
> ⚠️ **La diferencia que se lee mal, dicha en voz alta: FALLO DEL EQUIPO no es CORTE DE CORRIENTE.**
> Con el equipo en ámbar intermitente el micro sigue vivo y **sube** la pluma; sin energía la pluma
> **baja**. Las dos cosas son deliberadas y no se contradicen.

**Verde con la pluma abajo está admitido** —la barrera puede ser **más restrictiva** que la lámpara—;
**pluma arriba sin verde, no**: eso sería una invitación a entrar que nadie autorizó. Es lo único que
el arnés del automático exige en esta dirección.

> **Y sólo `semaforo.cpp` la mueve, dentro de `escribirPines()`**: `grep -n MOTOR_TALANQUERA` sobre
> las dos puntas devuelve **ese `digitalWrite` y el del `setup()`, y nada más**. No hay un segundo
> `digitalWrite` sobre `MOTOR_TALANQUERA` en ninguna parte, ni siquiera en el test de lámparas
> (`N-82`: el verde del test enciende la lámpara, **no abre la barrera**). **Una barrera con dos
> puertas no es una barrera.**

---

### Tiempos de Despeje (All-Red / Rojo Estático)
Cuando se solicita el cambio de vía, el sistema debe entrar en un estado de **ROJO ABSOLUTO**.
- Durante *N* segundos, **ambos semáforos estarán en ROJO**.
- **Variabilidad de Terreno:** Como la obra puede abarcar de 20m a 500m (con radios que alcanzan hasta 6km en línea vista), el tiempo de despeje no puede estar limitado a un valor bajo.
- **Configuración:** ~~La interfaz del menú LCD permite configurar tiempos de despeje de 5 a 999 segundos (piso mínimo de 5s por seguridad vial, hasta 16.6 minutos) para dar cobertura total a puentes largos o túneles de 500m.~~
  > 🔴 **CORREGIDO EL 28/08 — este párrafo daba un rango que la app NO puede pedir.** Sin pantalla,
  > los tiempos se fijan con `CMD:PIN:...:SET_TIEMPOS:<verdeMin>,<rojoMin>,<despejeSeg>`, y los
  > límites duros los impone el firmware, no la interfaz. **MEDIDO en
  > `Maestro/src/modo_automatico.cpp` líneas 51–53** *(re-medido el 04/09: las constantes se
  > movieron al crecer el comentario que las razona; el 31/08 estaban en `32–34`)*:
  >
  > | | constante | mínimo | máximo |
  > |---|---|---|---|
  > | Verde | `VERDE_MIN_MIN` / `_MAX` (`:51`) | **3 min** *(era 1)* | 15 min |
  > | Rojo | `ROJO_MIN_MIN` / `_MAX` (`:52`) | **3 min** *(era 1)* | 15 min |
  > | **Despeje (todo-rojo)** | **`DESPEJE_SEG_MIN` / `_MAX` (`:53`)** | **10 s** | **90 s** |
  >
  > 🔴 **EL MÍNIMO DE VERDE Y ROJO SUBIÓ DE 1 A 3 MINUTOS EL 04/09, Y ES UNA DECISIÓN VIAL DEL
  > RESPONSABLE, NO UN AJUSTE DE INTERFAZ.** El motivo, con sus palabras: **«tres minutos es la
  > mínima distancia de seguridad»**. En un paso alternado de un solo carril un camión pesado tarda
  > entre 5 y 8 s sólo en reaccionar y arrancar; con un verde de 60 s pasan tres o cuatro vehículos
  > antes de cortar a ámbar, y lo que se produce no es una cola: es **un conductor convencido de que
  > el semáforo está averiado, adelantando en rojo contra el sentido que acaba de recibir verde**.
  > El límite de 1 minuto era un valor de **mesa de pruebas** que se quedó abierto para la vía.
  >
  > **La guarda vive en el firmware, y ése es todo el punto.** La app valida lo mismo —para no
  > ofrecer lo que el equipo va a rechazar— pero la app no es la única que habla por `J17`:
  > cualquier otra cosa en ese cable, o una APK vieja de las que sobreviven en los teléfonos, puede
  > mandar `SET_TIEMPOS` con un minuto. **Una guarda que sólo vive en la interfaz es de cortesía.**
  >
  > ⚠️ **Coste declarado, para que no aparezca como sorpresa en banco: ya no se puede probar en mesa
  > con ciclos de 1 minuto.** Se aceptó a sabiendas.
  >
  > 🟢 **LOS DOS AVISOS QUE HABÍA AQUÍ SE CIERRAN EL 04/09 POR LA TARDE (`N-133`, `N-42`,
  > `N-135`). Se conservan tachados, con lo que los sustituye debajo:**
  >
  > ~~*«los valores por defecto del modo siguen siendo 1 min de rojo y 1 min de verde
  > (`modo_automatico.cpp:13` y `:94`), y no pasan por la guarda. Al entrar en Modo Automático,
  > `modoAutomatico_setup()` reescribe los tres valores a `1, 1, 15`, así que unos tiempos aceptados
  > por `SET_TIEMPOS` se pierden al arrancar el modo»*~~
  >
  > ~~*«ese `modoAutomatico_setup()` deja el modo en la fase `CONFIG_ROJO` del asistente de pantalla,
  > que sólo avanza con `botonAceptar()` — y `botonAceptar()` devuelve `false` siempre»*~~
  >
  > **Lo que hay HOY, MEDIDO en `Maestro/src/modo_automatico.cpp`:**
  >
  > | | antes | hoy |
  > |---|---|---|
  > | Valores por defecto | `1, 1, 15` escritos a mano | **los propios mínimos**: `minRojo = ROJO_MIN_MIN, minVerde = VERDE_MIN_MIN, segEstatico = DESPEJE_SEG_MIN` (`:53-54`) → **3 min · 3 min · 10 s** |
  > | Al entrar en Automático | reescribía los tres | **respeta los que haya** y llama a `recuperarTiemposGuardados()` (`:181`) |
  > | Al cortar la luz | se perdían | **sobreviven** en el respaldo con pila (`N-133`) |
  > | El asistente `CONFIG_ROJO` | huérfano, sin botón que lo aceptara | **retirado entero** (`:152-179`) — el modo **arranca corriendo** |
  >
  > 🔴 **Y la causa de `N-42` YA NO ES UNA LECTURA: está escrita en el fuente con su mecanismo
  > completo, y es peor de lo que este manual sospechaba.** No era sólo que las luces no se
  > movieran. `coordinador_actualizar()` vivía **dentro** del `case CORRIENDO`, y `main.cpp`
  > **excluye a `MODO_AUTOMATICO` del respaldo de fondo** —con un comentario que decía *«ya se llama
  > en modo_automatico.cpp»*, **cierto sobre el papel y falso en ejecución**—. Así que en Automático
  > **el Maestro se quedaba MUDO en la radio: ni un `PING`**. El Esclavo, sin oír nada durante
  > `SFTY6_SILENCIO_MS`, **se iba a ámbar por orfandad haciendo lo correcto**, y desde fuera parecía
  > un fallo de comunicaciones. **El Maestro estaba VIVO pero no HABLANDO, que no es lo mismo.**
  >
  > Hoy la llamada al coordinador es **incondicional** (`:206`).
  >
  > ⚠️ **PERO EL ESTADO DE `N-42` NO SE DECLARA CERRADO EN ESTE MANUAL, Y HAY QUE EXPLICAR POR QUÉ.**
  > El comentario del fuente dice *«medido y confirmado en banco el 04/09»*. **Eso es `ESCRITO`, no
  > `MEDIDO`**, y choca de frente con lo que todo el resto de este paquete sostiene: en el banco del
  > 3–4/09 **el equipo nunca llegó a operar** —se queda esperando selección de modo y la app no
  > conectaba (`N-122`)—, y por eso `N-42` *«no se confirmó ni se descartó»*. **Las dos frases no
  > pueden ser ciertas a la vez.**
  >
  > **Lo que este manual publica, que es lo único que puede sostener:** el **arreglo** está `MEDIDO`
  > en el fuente y es coherente con el síntoma. **Si `N-42` llegó a verse en la tarjeta y cuándo lo
  > decide quien ejecutó la sesión**, no este documento. Hasta entonces **sigue contando como
  > abierto**, y sigue siendo lo primero de la próxima visita.
  >
  > ⚠️ **Y hay un techo que no es una decisión sino el tipo de dato: los tres son `uint8_t`.** Un
  > `uint8_t` no pasa de **255**, así que **el «999 s» que este párrafo prometía no fue nunca
  > representable** por este camino — no es que se haya recortado el rango: es que ese número no
  > cabía en la variable. Cualquier documento que siga publicando «hasta 999 s» está describiendo
  > un equipo que no existe.
  >
  > Fuera de rango, el equipo responde `$ERR,CMD:SET_TIEMPOS,DESC:RANGO` y **no cambia nada**.
  > Con el ciclo corriendo responde `$ERR,CMD:SET_TIEMPOS,DESC:EN_MARCHA_PARE_EL_MODO`, porque bajar
  > un tiempo a mitad de fase acortaría un todo-rojo ya empezado.
  >
  > **De dónde salía el «5 a 999 s»:** de la pantalla `CONFIG_TIEMPOS` del menú local, que sí subía
  > hasta 999 en un contador propio de pantalla (`modo_automatico.cpp` línea **119**, `segEstatico`,
  > que es un `int` de menú y no la variable de configuración). **Ese camino ya no tiene display
  > — y desde el 31/08 tampoco tiene forma de confirmar: `botonAceptar()` (línea 121 de ese mismo
  > fichero) devuelve `false` siempre.** El mínimo real
  > tampoco es 5 s sino **10 s**, y el propio fuente dice por qué: *«10 s es lo que tarda en
  > despejarse el tramo más corto que esta casa ha montado»*.
  >
  > ⚠️ **Consecuencia que hay que decir en voz alta:** los túneles y puentes de 500 m que este
  > párrafo prometía cubrir **con 90 s de despeje pueden no quedar cubiertos**. Si una obra los
  > necesita, es un cambio de firmware (subir `DESPEJE_SEG_MAX`), no un ajuste de configuración.

---

## 2. Comportamiento en Destello / Intermitente (Bajo Flujo)

Según el Manual de Señalización (2024), si el flujo vehicular baja al 50% o menos durante 4 horas o más (usualmente operación nocturna), el sistema debe pasar a operación intermitente.
- **Funcionamiento:** Un semáforo parpadea en 🟡 **Ámbar** (Precaución - vía principal) y el otro en 🔴 **Rojo** (Pare - vía secundaria), o ambos en Rojo Intermitente para pasos de igual jerarquía.

---

## 3. Comportamiento de la Interfaz (app por Bluetooth) — 🔴 REESCRITO EL 28/08

~~**La pantalla LCD ST7920 se retira.**~~ → **La pantalla LCD ST7920 DEJA DE FUNCIONAR: su conector
`J17` lo ocupa el módulo Bluetooth y el firmware ya no conduce sus pines** *(precisión del 31/08 —
ver el recuadro de la cabecera; el efecto para el operario es el mismo, pero la frase corta llevaba
a creer que se había borrado el código, y no)*. Lo que sigue describe la interfaz vigente; debajo
queda lo que decía antes, tachado, para que se vea qué se perdió y qué lo sustituye.

> 🛑 **Y desde el 31/08 hay una segunda mitad que este apartado no decía: la app es la superficie de
> mando PRINCIPAL, y el mando de relés es la de ÚLTIMO RECURSO — pero sólo en el Maestro.** En el
> Esclavo el reparto es el contrario para todo lo que sea cambiar de modo: **allí la app no puede**,
> y el mando es lo único que hay. **Ver §9**, que es donde está la tabla completa.

> ## 🔴 04/09 — LA APP CAMBIÓ DE BARRERA: PARA ABRIR PASO NO PIDE PIN, PREGUNTA SI HA MIRADO EL TRAMO
>
> **Es lo que más se nota en la mano del operario, y el motivo es lo único que hay que recordar:**
>
> > **El equipo no sabe si quedan vehículos en el tramo, y el operario sí. Un PIN demuestra QUIÉN
> > ERES; no demuestra que hayas MIRADO.**
>
> | lo que se pulsa | qué pide la app hoy |
> |---|---|
> | **DAR PASO** · **AUTOMÁTICO** | 🟢 **la confirmación de vía** — *«¿No quedan vehículos en el tramo?»*, con la lista de qué mirar. **Ya no pide PIN** |
> | **ROJO TOTAL** · **ÁMBAR EMERGENCIA** · **VOLVER AL MENÚ** | 🛑 **nada: ni PIN ni pregunta.** Es la **dirección segura** |
> | **ÁMBAR** (modo) · **RETIRAR ÁMBAR** · **DEGRADADO** · `TEST_LEDS` y la pestaña Técnica | 🔵 **el PIN, como siempre** |
>
> **Por qué no se pregunta para PARAR, que es la mitad que se lee mal:** **preguntar para parar
> enseña a decir que sí sin leer**, y el día que la pregunta llegue en serio ya nadie la lee. Poner
> rojo o ámbar no abre paso a nadie: **no hay riesgo que confirmar.**
>
> ⏱️ **El «he mirado» caduca a los 30 s, y también en cuanto cambian las luces.** Confirmar y luego
> entretenerse no vale: **el tramo que se miró ya no es el tramo que se va a abrir.**
>
> 🟢 **Y el PIN, donde sigue haciendo falta, ahora CADUCA:** a los **60 s** de guardarse el teléfono
> —esos 60 s de gracia son para que consultar un mensaje no cierre la sesión— y a los **5 minutos
> sin mandar ninguna orden. El técnico lo va a notar**, y **no es una desconexión**: el enlace
> Bluetooth sigue, lo que caducó es el permiso. Se vuelve a entrar con los mismos cuatro dígitos.
>
> **El detalle completo está en `14_Manual_App_Movil_IOT_VIAL.md` §4.bis.3 y §4.bis.4.**
>
> 🛑 **Y lo que NO está comprobado, dicho aquí y no sólo allí:** la caducidad por guardarse el
> teléfono cuelga de sucesos del navegador y **nadie la ha visto disparar en la APK con la pantalla
> apagada** — que es el único caso para el que existe. **Hasta que alguien lo cronometre, la barrera
> real sigue siendo quién tiene el teléfono.**

- **Regla de Oro (Independencia de Red) — SIGUE VIGENTE, cambia la vía.** El operario DEBE poder
  operar el equipo **incluso si las radios están apagadas o no hay comunicación con el Esclavo**. Eso
  se conserva: **el Bluetooth es un enlace corto e independiente de la radio de largo alcance**, así
  que la app entra igual con las radios muertas. Lo que cambia es que el operario entra **desde el
  piso, con el celular**, en vez de subir al gabinete.
- **Comportamiento durante la configuración:** con el equipo parado (sin modo arrancado), ambos
  semáforos se mantienen en **🔴 ROJO FIJO continuo** si hay comunicación, y pasan a **🟡 Amarillo
  Intermitente** si no la hay. **Eso no lo decidía la pantalla, lo decide el coordinador**, y no ha
  cambiado.

  > 🔴 **VISTO EN BANCO EL 3–4/09, Y HAY QUE DECIRLO CON OTRAS PALABRAS: ESO ES TAMBIÉN LO QUE SE VE
  > AL ENCENDER, Y NO ES UNA AVERÍA.** Retiradas la pantalla y los pulsadores, **al energizar el
  > equipo no arranca ningún modo por su cuenta**: se queda **EN ESPERA DE SELECCIÓN DE MODO** hasta
  > que se le ordene uno desde la app.
  >
  > | lo que se ve al encender | qué significa |
  > |---|---|
  > | 🔴 **Rojo fijo en las dos puntas** | esperando modo, **y hay comunicación con la otra punta** |
  > | 🟡 **Ámbar intermitente** | esperando modo, **y NO hay comunicación con la otra punta** |
  >
  > **Ninguno de los dos es un fallo del equipo.** En banco se leyó el ámbar intermitente como
  > avería y se perdió tiempo buscándola: era el equipo diciendo, correctamente, que nadie le había
  > dicho todavía qué hacer. Lo que sí es un dato a mirar es **cuál de los dos** sale, porque
  > distingue *«falta la orden»* de *«falta la orden Y falta la radio»*.
  >
  > 🛑 **Y su consecuencia dura:** si la app no conecta, **el equipo se queda ahí**. Hoy no hay
  > ninguna otra forma DEMOSTRADA de sacarlo de esa espera: el mando de relés tiene el firmware
  > corregido pero **sin ejercer en tarjeta** — ver §7 (`N-118`) y §8.
- **Arranque:** al ordenar un modo desde la app (`CMD:PIN:1234:SET_MODO:AUTO` / `:MANUAL` /
  `:AMBAR`), el sistema aplica el Despeje All-Red en ambos extremos antes de abrir ningún carril.

  > 🟢 **Y desde el 04/09 el Modo Automático ARRANCA CORRIENDO, sin cuestionario (`N-42`).** El
  > asistente de tres preguntas —`CONFIG_ROJO` → `CONFIG_VERDE` → `CONFIG_ESTATICO`— **se ha
  > retirado entero**: era de la época de la pantalla LCD, su única salida era `botonAceptar()`, y
  > esa función devuelve `false` siempre desde el 31/08. **El equipo entraba en el cuestionario y no
  > salía nunca.** Hoy hay **una sola puerta** y el modo empieza a ciclar con los tiempos que tenga
  > guardados. Ver §1.

### 🛑 PUESTA EN MARCHA — LA PRIMERA VUELTA DE ENERGÍA CON ESTE FIRMWARE BORRA LOS TIEMPOS GUARDADOS

> **Va aquí, en voz alta y antes de que nadie lo vea pasar, porque si no se dice alguien va a
> reportar como avería un comportamiento que está diseñado así.**

**Qué ocurre:** `N-133` mete los tiempos del ciclo automático dentro del respaldo con pila, y para
eso **cambia el FORMATO de ese respaldo**. La firma que lo identifica sube de `0x5EB1` a **`0x5EB2`**
(`Maestro/src/respaldo.cpp:76`, idéntico en el Esclavo). **Un equipo que arranca con este firmware
encuentra la firma vieja, no la reconoce y BORRA el respaldo entero.**

**Qué se pierde en esa primera arrancada, y es más que los tiempos:**

| | |
|---|---|
| los tiempos del ciclo | vuelven a los **mínimos: 3 min de verde, 3 min de rojo, 10 s de despeje** |
| la hora y la autorización de sincronización | se pierden — **hay que volver a poner el reloj** |
| el ciclo guardado del Modo Degradado | se pierde |

**Es CORRECTO y es la dirección segura.** El propio fuente lo razona (`respaldo.cpp:71-81`): dar por
buena una firma vieja significaría leer con esta aritmética unos bytes escritos con otra, y
*«arrancar limpio»* es lo que evita un ciclo que nadie configuró. **Pasa UNA sola vez**, en el primer
arranque tras cargar; a partir de ahí los tiempos sobreviven a los cortes.

> ✅ **Lo que hay que hacer en la puesta en marcha, en este orden:**
>
> 1. Cargar el firmware y **darle la primera vuelta de energía**.
> 2. **Poner la hora** (`SET_RTC` desde la app) — se perdió, y sin ella no se puede entrar en
>    Degradado.
> 3. **Poner los tiempos del ciclo** (`SET_TIEMPOS`) **con el Modo Automático PARADO** — ver el
>    aviso de §1: en marcha el equipo los rechaza.
> 4. 🔴 **LEER LO QUE QUEDÓ, no darlo por hecho.** Entre en Automático y compruebe en la app que los
>    tiempos son los que puso.

> 🔴 **Y el porqué del paso 4, que es un HALLAZGO de esta revisión y no una precaución genérica.**
> `respaldo_borrar()` (`respaldo.cpp:193-201`) pone a cero cinco registros —`DR2` a `DR6`— **y NO
> toca los dos que `N-133` acaba de estrenar**, `DR9` y `DR10`, que son justamente donde viven los
> tiempos del ciclo. Acto seguido `sellar()` calcula el checksum **incluyéndolos** (`:157-158`), o
> sea que **los sella como contenido válido sin haberlos limpiado**.
>
> **En un equipo nuevo o con la pila agotada esos registros valen cero y no pasa nada** —el lector
> se niega a devolver un cero (`:249`) y el equipo cae a los mínimos, que es lo que promete el
> propio header (`respaldo.h:58-61`)—. **El caso que no está cubierto es justo el de esta
> actualización:** una tarjeta con pila buena, cuyo respaldo viejo se declara inválido por la firma,
> puede llevar en `DR9`/`DR10` lo que dejara el arranque anterior. Si esos bytes cayeran dentro de
> `3–15 / 3–15 / 10–90`, la revalidación de rango de `modo_automatico.cpp:110-117` **los daría por
> buenos** y el cruce arrancaría con un ciclo que nadie configuró.
>
> **No se propone aquí un arreglo ni se dictamina la probabilidad de que ocurra: se publica la
> medida.** Es una decisión de firmware —a quién le toca limpiar esos dos registros— y va al
> responsable. **Lo que sí puede hacer quien esté en el poste cuesta diez segundos: mirar los
> tiempos después de la primera arrancada.**

### 🛑 Lo que se pierde y NO tiene sustituto todavía

~~**Prueba de Alcance.** Cuarta opción del Menú Principal. Muestra calidad de enlace en %, barra
gráfica, tiempo de respuesta en ms y fallos consecutivos, actualizándose cada 3 segundos.~~

**Medido sobre el firmware el 28/08:** la pantalla `PRUEBA ALCANCE` **sigue existiendo en el
binario** (`modo_alcance.cpp`) y ya no tiene dónde dibujarse. De lo que mostraba:

| dato | ¿sobrevive? | dónde |
|---|---|---|
| Calidad de enlace en % | ✅ sí | campo `RF:` de la trama `$STATUS`, ~~cada segundo~~ **cada 2 s** *(04/09)* |
| Tiempo de respuesta en ms | ✅ sí | campo `RTT:` de `$STATUS` |
| Barra gráfica y fallos consecutivos | ❌ no | eran dibujo de pantalla |
| **Contadores de línea RS-485** (`RX 0 - nada llega` · `RX 4k - BASURA` · `RX 36 9 tr`) | ❌ **NO** | **no viajan en `$STATUS`** |

> ⚠️ **La pérdida de los contadores de línea no es cosmética.** Eran lo que distinguía *«no hay
> cobertura»* de *«el cable RS-485 está suelto o la radio emite basura»* — dos averías que se ven
> igual desde lejos y se arreglan de forma distinta. Hasta que ese dato llegue a la app, esa
> distinción hay que hacerla con instrumentos en el poste. **Se anota como hueco abierto en lugar de
> darlo por trasladado.**

> 🔴 **Y un aviso sobre el `RF:` del ESCLAVO, medido el 28/08 en
> `Esclavo/src/bluetooth.cpp` línea 215:** la trama del Esclavo emite **`RF:98%` y `RTT:85ms` como
> texto literal**, no como medida. Son constantes escritas dentro del `snprintf`. **El Esclavo no
> está midiendo su enlace: está afirmando un 98 % pase lo que pase**, incluso con la radio
> desconectada. **No use ese número para decidir nada.** El `RF:` del **Maestro** sí sale de la
> telemetría real del latido de 3 s (SFTY-14).

---

## 4. Comportamiento ante Fallas (Fail-Safe & Self-Healing Real)

1. **Pérdida de Comunicación (SFTY-6):** Si se pierde comunicación por más de ~~12.0~~ **25,0
   segundos**, el sistema entra automáticamente en `C_FALLO` / `S_FALLO` (🟡 **Amarillo
   Intermitente**). En `C_FALLO`, el Maestro envía `CMD_GO_RED` para obligar al Esclavo a pasar a
   Rojo o Amarillo Intermitente por timeout.

   > 🔴 **CORREGIDO EL 31/08 — este manual publicaba 12 s, y son 25 s desde N-71.** **MEDIDO:**
   > `SFTY6_SILENCIO_MS 25000UL`, en `Maestro/include/protocolo.h:149` **y** en
   > `Esclavo/include/protocolo.h:149` *(el mismo número en las dos puntas, que es parte de la
   > propiedad)*; se usa en `Maestro/src/coordinador.cpp:656` y `Esclavo/src/main.cpp:555`.
   >
   > **Y el porqué del cambio importa para el operario, no sólo para el que programa:** el techo
   > de 12 s estaba **por debajo** del peor caso del ciclo de reintentos, que necesita hasta
   > **20,5 s** para agotar los cinco. Con 12 s, **los reintentos 4 y 5 no se ejecutaban jamás**:
   > el equipo se iba a ámbar antes de haber terminado de intentar hablar. Nadie lo notaba porque
   > irse a ámbar es una reacción razonable — pero era ámbar de más, no ámbar necesario.
   >
   > ⚠️ **NO CONFUNDIR CON LOS OTROS 12 s DE ESTE MANUAL.** La ventana de las secuencias del mando
   > (§7) **sí es de 12 s y es correcta**: `VENTANA_TRIPLE_MS = 12000`, `Maestro/src/mando.cpp:38`
   > y `Esclavo/src/mando.cpp:42`. Son dos números distintos que valían lo mismo por casualidad, y
   > **sólo uno de los dos cambió.**
   >
   > **Consecuencia práctica en obra:** un corte de radio de, por ejemplo, 15 segundos **ya no
   > manda el cruce a ámbar**. Si alguien había aprendido que «a los 12 segundos se pone en
   > ámbar», el equipo de hoy aguanta **más del doble** antes de hacerlo.

   > ## 🟢 `N-134` (04/09) — EL ÁMBAR SE ORDENA. LOS 25 s PASAN DE SER EL CAMINO A SER LA RED
   >
   > **El defecto, en pasado, y es el que se veía en banco como *«a veces se van los dos, a veces
   > sólo el maestro»*:** al poner ámbar desde el Maestro, el Esclavo **se quedaba en ROJO**. Nadie
   > se lo decía. Lo que acababa llevándolo a ámbar era **la orfandad**: dejaba de oír al Maestro y,
   > pasados los 25 s de `SFTY-6`, se iba solo. **Las dos puntas acababan en ámbar, pero con hasta
   > 25 segundos de diferencia** — y el operario que miraba desde un extremo veía un cruce a medias
   > y concluía, razonablemente, que el equipo no le había obedecido.
   >
   > **Lo que hay hoy, MEDIDO:** existe una orden explícita, **`CMD_GO_AMBAR` = `0x13`**
   > (`Maestro/include/protocolo.h:174`, **el mismo número en el `Esclavo`**). El Maestro la manda
   > al entrar en ámbar (`Maestro/src/modo_ambar.cpp:57`) y el Esclavo la atiende
   > (`Esclavo/src/main.cpp:394`). **Decisión del responsable.**
   >
   > **Y las dos mitades que hacen que esto no sea sólo «un comando más»:**
   >
   > 1. 🔴 **EL ROJO PREVIO SE QUEDA, y es el intermedio seguro.** `modo_ambar_setup()` sigue
   >    mandando **`CMD_GO_RED` primero** (`modo_ambar.cpp:36` → `coordinador_forzarRojoTotal()`,
   >    `coordinador.cpp:554-562`) y sólo después el ámbar. **No se salta de un verde a un ámbar
   >    intermitente**: al que ya venía lanzado eso le da una señal que invita a negociar el paso
   >    mientras aún cree tener prioridad.
   > 2. ✅ **LA ORFANDAD DE 25 s SIGUE AHÍ, Y AHORA ES LA RED.** El Esclavo **no refresca** su
   >    contador de silencio al recibir esta orden (`main.cpp:404-407`), a propósito: **si la orden
   >    se pierde en el aire, el equipo se va a ámbar igual**, por el camino de siempre
   >    (`main.cpp:596-599`). Las dos vías desembocan en la misma puerta, `semaforo_iniciarFallo()`.
   >
   > **Lo que cambia para quien está en el cruce:** el ámbar en la punta remota **llega en seguida
   > en vez de hasta 25 segundos después**. Si tarda esos 25 s, ya no es lo normal — **es que la
   > orden no llegó**, y eso es un dato sobre la radio.
   >
   > ⚠️ **MEDIDO sobre el fuente. No se ha ejercido en tarjeta**, y era justamente en banco donde
   > se veía el síntoma.
2. **Auto-Recuperación Autónoma (Self-Healing Real):** Al restablecerse la señal de radio, el sistema **NO requiere reinicio manual**. Limpia automáticamente el registro de duplicados (`protocolo_resetReplayProtection()`), fuerza Rojo Estático (All-Red) de 15 segundos en ambos semáforos para limpiar la vía y reanuda el ciclo lumínico sin intervención técnica.
3. **Cuelgue de Procesador (Ruido EMI):** El Watchdog interno (`IWatchdog` activo a 4.0s) reinicia el procesador ante interferencias severas.

---

## 5. Resiliencia RF: Ráfaga configurable (SFTY-11) y Ventana Deslizante (SFTY-10)

Para garantizar comunicación inquebrantable en zonas de montaña con alta interferencia:
- **Ráfaga (Burst) — SFTY-11:** cada orden de 4 bytes sale **3 veces seguidas**, con FEC activo en
  las radios E90-DTU. Al escuchar el bus verá tres tramas idénticas por cada orden: es lo esperado,
  no un reenvío por fallo. *(`RF_BURST_COPIES = 3`, en `include/protocolo.h` de las dos puntas.)*
- **Suma de verificación CRC-8 Maxim (`0x31`) — SFTY-3:** todo paquete cuya suma no cuadre se
  descarta sin ejecutarse.
- **Ventana Deslizante (Sliding Window) — SFTY-10:** cuando la suma no cuadra, el búfer se desplaza
  un byte y se reintenta, para reengancharse a la copia siguiente de la ráfaga sin desalinearse.
- **Protección Antirepetida (Replay Protection):** descarte de duplicados mediante `msgID`.

---

## 6. Integración de Cámaras IA AcuSense para Demanda Vehicular (Modo Inteligente)

Para detección inteligente de flujo vehicular en pasos alternados de obra sin requerir computadores externos en el remolque:
* **Conexión Hardware:** Salida de alarma de relé de la cámara (`1A`/`1B`) a la bornera **`J14`**:
  un hilo a **`p1`** (`/Puerta` → **`PB0`**) y el otro a **`p2`** (**`3,3 V`** del propio conector).

> 🛑 **AQUÍ PONÍA ~~«con masa `GND`, por la bornera `J14`»~~ Y ERA FALSO EN LAS DOS MITADES — 05/09.**
>
> **MEDIDO EN COBRE**, extrayendo las redes de los pads de `J14` del `.kicad_pcb` real (2.091.460 B),
> no del esquemático:
>
> ```
> ===== J14   (Conn_01x02)
>    p1  -> /Puerta      (= PB0, la entrada de demanda)
>    p2  -> /3.3V
> ```
>
> **`J14` tiene DOS posiciones y NINGUNA es masa.** No hay `GND` en ese conector, así que la
> instrucción no se podía ni ejecutar. Y si el instalador buscaba la masa en otro punto de la placa
> y cerraba contra ella, **la demanda no se produce nunca**: el firmware lee ese pin **activo en
> ALTO** —`Esclavo/src/main.cpp:350`, `bool demandaCamaraActual = (digitalRead(CAM_DEMANDA_PIN) ==
> HIGH);`, y `Maestro` por `camara_leerPin()` en `botones.cpp:105`—.
>
> **El gesto correcto es el mismo que en `J16`: cerrar el contacto seco contra los 3,3 V del pin de
> al lado.** La placa lo tiene previsto —`R64` de 10 kΩ **a masa** más `C25` de 100 nF, o sea un
> pull-down con antirrebote de 1 ms (`pines.h:43-46`)—, que es justo lo que hace falta para que el
> reposo sea `0 V` y el cierre a 3,3 V sea la detección.
>
> ⚠️ **Es el mismo error de polaridad que N-118, en la bornera que sí se cablea hoy.** La salida de
> la AcuSense es configurable (NO/NC), así que se elige qué estado significa demanda **sin tocar
> placa ni firmware** — pero el hilo va a `p1` y `p2`, no a una masa.

> 🛑 **CORREGIDO EL 04/09 — SON DOS CÁMARAS EN EL CRUCE, UNA POR POSTE. NO TRES POR TARJETA.**
>
> **Aquí ponía ~~«hoy hay TRES entradas de cámara por poste»~~ y se leía como tres cámaras que
> cablear.** Lo que hay son **tres bornes de entrada por tarjeta que piden LO MISMO: paso para ese
> poste**. Y como el cruce tiene **dos sentidos, un poste por sentido**, **las cámaras del cruce son
> DOS: una por poste.**
>
> **MEDIDO el 04/09** en `Maestro/include/pines.h:43-46` y `:136-137`,
> `Maestro/src/botones.cpp:116` y `:144-152`, `Maestro/src/modo_inteligente.cpp:86` y `:124`,
> `Esclavo/src/main.cpp:348-354` y `Esclavo/src/botones.cpp:136`, `:164-172`:
>
> | borne | pin | qué trae la placa | cómo lo lee el programa | estado |
> |---|---|---|---|---|
> | **`J14`** | `PB0` | 🟢 **`R64` 10 kΩ + `C25` 100 nF → antirrebote por hardware de ~1 ms** | **Maestro: por NIVEL** (`modo_inteligente.cpp:86`, `:124`) · **Esclavo: por FLANCO**, y ahí sí llama a `demanda_solicitar()` (`main.cpp:352`) | ✅ **Es el borne donde va la cámara** |
> | `J16` p10 | `PB14` | 🟠 `R67` 10 kΩ a masa, **SIN condensador** | **por FLANCO** → `demanda_solicitar()`, en las dos puntas | 🟠 **entrada equivalente disponible. NO se le monta cámara hoy** |
> | `J16` p12 | `PB15` | 🟠 `R68` 10 kΩ a masa, **SIN condensador** | igual | 🟠 igual |
>
> **🔵 LA CÁMARA VA A `J14`, Y EL MOTIVO NO ES PREFERENCIA: es el único de los tres bornes que la
> placa protege.** Un relé de cámara **rebota** al cerrar. En `J14` el condensador se lo come antes
> de que el micro lo vea. En `p10`/`p12` no hay condensador y la lectura es **por flanco**: lo único
> que hay delante es la **ventana de silencio de 3 s** de `demanda_solicitar()`
> (`demanda.cpp:8`, `:19`), que lo tapa casi siempre. **«Casi» no es una garantía**, y esa ventana es
> **una sola por poste**: un rebote la consume y la petición del coche que viene detrás se pierde.
>
> **`p10` y `p12` NO se retiran de este manual y no son un error:** son entradas reales, medidas en
> banco (`M3`, 03/09: **p10 = 9,93 kΩ**, **p12 = 9,94 kΩ** a masa), y **son el sitio previsto si
> algún día hace falta una segunda entrada en un poste**.
>
> ✏️ **Y una precisión que el censo del 04/09 obligó a escribir, porque la frase anterior era falsa
> en una punta:** aquí ponía *~~«las tres piden paso por la misma puerta —`demanda_solicitar()`»~~*.
> **En el ESCLAVO es cierto para los tres bornes. En el MAESTRO no lo es para `J14`**, que se lee
> **por nivel** dentro de `modo_inteligente.cpp` y **no llama a `demanda_solicitar()`** —`grep` de la
> función devuelve `bluetooth.cpp:653`, `botones.cpp:148` y su definición, y ninguno pasa
> `CAM_DEMANDA_PIN`—. **El efecto operativo es el mismo** —la petición entra igual en el mismo `OR`
> del Modo Inteligente— pero **las dos puntas no lo hacen igual**, y eso se anota en vez de
> redondearse. *No se propone aquí ningún cambio de firmware: es una asimetría medida, no un defecto
> diagnosticado.*
>
> **Lo que sí vale para los tres bornes y para las dos puntas, y es lo que importa en la calle: una
> cámara PIDE, no decide** (SFTY-27). **No enciende nada.**
>
> 🛑 **ANTES DE CABLEAR CÁMARA A `J16`, uno de los dos motivos SIGUE EN PIE y el otro está CERRADO:**
>
> 1. **SIGUE EN PIE — `J16` LLEVA 12 V CRUDOS EN SU POSICIÓN 1**, el único conector de señal de la
>    tarjeta que los trae, sin opto ni protección. **Se tapa físicamente antes de cablear nada**, y
>    el margen real en cobre hasta el pin de al lado es de **1,36 mm**, medido sobre pistas y vías.
>    **Esto NO se relaja: la medida `M3` no lo toca.**
> 2. ~~**Falta la medida `M3`:** con `INPUT` pelado, el pin necesita **resistencia real a masa en la
>    placa** o queda flotando y el ruido dispara **demandas fantasma**. `PB0` la tiene declarada;
>    de `PB14`/`PB15` **sólo lo dice el netlist y nadie lo ha comprobado con multímetro**.~~
>
> 🟢 **CADUCADO EL 03–04/09: `M3` ESTÁ CERRADA CON MULTÍMETRO, Y EL CABLEADO DE CÁMARA A `J16` ESTÁ
> DESBLOQUEADO.** Se tacha con motivo en vez de borrarse: este manual fue el último que la seguía
> dando por pendiente, contradiciendo a otros cuatro que ya la daban por cerrada, y una duda que
> desaparece en silencio se vuelve a plantear.
>
> **Las cuatro medidas de `J16` con energía, sobre la tarjeta:**
>
> | posición | señal | a masa | en reposo |
> |---|---|---|---|
> | **p5** | `MANDO_A` (`PB9`) | **9,92 kΩ** | **0,6 V** |
> | **p8** | `MANDO_B` (`PB13`) | **9,92 kΩ** | **0,6 V** |
> | **p10** | `CAM_C_PIN` (`PB14`) | **9,93 kΩ** | **0 V** |
> | **p12** | `CAM_D_PIN` (`PB15`) | **9,94 kΩ** | **0 V** |
>
> **El netlist tenía razón: el pull-down real de 10 kΩ existe en el cobre**, así que `PB14`/`PB15`
> **no quedan flotando** y no hay demandas fantasma por ese camino.
>
> **Y el orden no es negociable** (`CLAUDE.md` §9.bis): **el firmware nuevo tiene que estar CARGADO
> EN LA TARJETA antes de que nadie enchufe nada en `J16`.** Con el firmware viejo dentro, `PB14`
> todavía es `botonAceptar()` leído activo en BAJO, y cualquier cosa enchufada ahí **pulsa
> «Aceptar» en un equipo que está en la calle**.
* 🛑 **Las cámaras de umbral (2 y 4) NO se instalan en V9.0, y no hay dónde conectarlas.** Medido el 27/08 sobre el esquemático: el pin `PB8` que los manuales daban por suyo **alimenta un LED testigo (`D5` por `R16` 1 kΩ)** — no es una bornera ni una entrada. El paso alternado lo regulan las cámaras de **demanda** y el **todo-rojo temporizado** (`cfgDespejeSeg`), que es el criterio conservador. Ver Manual 9 y `roadmap.md` N-64.
* **Procesamiento:** La cámara Hikvision AcuSense ejecuta su analítica embebida (Detección de Intrusión con filtro `☑ Solo Vehículo`) y cierra su contacto seco al detectar presencia vehicular.
* **Seguridad:** Cada cambio de sentido respeta el tiempo de **Despeje Todo-Rojo** configurado ~~en pantalla~~ **(28/08: desde la app)** antes de habilitar el verde al sentido con demanda.

> 🔴 **28/08 — QUÉ MIDE DE VERDAD EL MODO INTELIGENTE, Y QUÉ NO.** Medido sobre
> `Maestro/src/modo_inteligente.cpp` y `Esclavo/src/main.cpp`:
>
> - **Es UN contacto seco por poste**, leído en `CAM_DEMANDA_PIN` = `PB0`, activo en alto, por la
>   bornera `J14`, con antirrebote de placa (`R64` 10 K + `C25` 100 nF). **Presencia sí/no, no
>   conteo de vehículos.**
> - El número que el equipo llama *«autos esperando»* es `presenciaActual`, la **suma de dos
>   contactos** —el local y el de la otra punta por radio—, así que **vale 0, 1 o 2 y nada más**.
>   No es una cuenta de coches.
> - **No hay ningún puerto de vídeo ni de cómputo externo.** El fichero
>   `6_Preguntas_Diseno_Funcional.md` lo tiene cerrado por decisión: *«Cero Computadores Edge
>   Externos»*.
> - ✅ **31/08 — y sigue valiendo «0, 1 o 2» aunque ahora haya tres entradas de cámara.**
>   **MEDIDO** en `Maestro/src/modo_inteligente.cpp:124`: ese número suma **sólo `PB0` y la demanda
>   remota**. Las cámaras `C` y `D` de `J16` **entran por otra puerta** —un flanco que llama a
>   `demanda_solicitar()`, `botones.cpp:128-130`— y **no suben ese contador**. Es decir que
>   **«Autos: 2» sigue siendo el máximo** y una demanda por `J16` **no se ve reflejada ahí**. Se
>   escribe porque lo contrario es lo que un lector supondría solo.
>
> **Se escribe aquí porque este manual es el «Ground Truth».** Un técnico que lea *«Autos: 2»*
> creyendo que hay dos vehículos contados está leyendo otra cosa: hay demanda en las dos puntas.

---

## 7. Vocabulario Oficial del Mando a Distancia de 4 Relés (Anti-Colisión N-53)

Para permitir la operación del semáforo a nivel del suelo sin colisionar con la edición de parámetros en pantalla:

> ## 🔴 CORREGIDO EL 31/08 — LA TABLA QUE HABÍA AQUÍ NO ERA LA DEL EQUIPO
>
> **Lo que este apartado publicaba era la REDEFINICIÓN PROPUESTA de V9.0, escrita en pasado como si
> estuviera implementada. Nunca se implementó.** Un operario que aprendiera aquella tabla accionaría
> secuencias que el equipo **no reconoce**, concluiría que el mando está averiado, y **la única de
> las cinco que sí existe la habría aprendido mal** — `A·B·A·B` es Degradado en las dos tablas, y es
> la única coincidencia.
>
> **Se conserva tachada, no borrada:** una propuesta que desaparece en silencio vuelve a proponerse
> como novedad, y la segunda vez ya nadie recuerda que no llegó a escribirse.
>
> | ~~Secuencia~~ | ~~Modo Activado~~ | ~~Confirmación Lumínica~~ |
> |---|---|---|
> | ~~**`A · B · A`** (≤12s)~~ | ~~🟢 Modo Automático~~ | ⛔ **NO EXISTE** |
> | ~~**`B · A · B`** (≤12s)~~ | ~~🟡 Modo Ámbar (Seguro)~~ | ⛔ **NO EXISTE** |
> | ~~**`B · A · B · A`** (≤18s)~~ | ~~✋ Modo Manual (Operario)~~ | ⛔ **NO EXISTE — y no hay ninguna secuencia de mando para el Modo Manual** |
> | ~~**`A · A · B · B`** (≤18s)~~ | ~~📷 Modo Inteligente (Cámaras IA)~~ | ⛔ **NO EXISTE — y no hay ninguna secuencia de mando para el Modo Inteligente** |

> ## 🟠 04/09 — **`N-118`: EL MANDO ESTUVO SORDO EN BANCO. EL FIRMWARE YA ESTÁ ARREGLADO Y FALTA EJERCERLO EN TARJETA**
>
> **El vocabulario de abajo siempre estuvo bien escrito y compilado.** Lo que faltaba era forma de
> pronunciarlo, y eso es lo que ha cambiado. Son dos cosas distintas y hay que leerlas por separado.
>
> ### 1. El defecto, en pasado — por qué el mando estuvo sordo
>
> **MEDIDO en banco el 3–4/09:** `MANDO_A` (`PB9`) y `MANDO_B` (`PB13`) llevan **`R65`/`R66`, 10 kΩ
> a masa**, y con ellas el pin se queda en **0,6 V**. El firmware de entonces los leía con
> `INPUT_PULLUP` y `== LOW`, así que **0,6 V era BAJO permanente**: el reconocedor de secuencias
> cuenta **flancos**, y un pin que nunca sube no da flancos. Las tres secuencias eran
> **inalcanzables**, no «poco fiables» — no llegaba ni el primer pulso, y no había destellos de
> confirmación que mirar porque no se reconocía nada.
>
> ### 2. El estado de hoy — firmware corregido en las DOS puntas
>
> **MEDIDO en el fuente:** los dos pines pasan a **`pinMode(BOTON1/BOTON2, INPUT)` pelado** y la
> lectura a **`digitalRead(...) == HIGH`**, o sea **ACTIVO EN ALTO**, exactamente como ya se leen
> las cámaras. `Maestro/src/botones.cpp:160-161` y `:223`; `Esclavo/src/botones.cpp:178-179` y
> `:232`. **Las `R65`/`R66` dejan de ser el problema y pasan a ser el reposo**: fijan el pin a 0 V
> cuando nadie acciona, que es justo lo que quiere una entrada activa en alto.
>
> 🔵 **Y CAMBIA EL GESTO CON EL QUE SE PRUEBA. Esto es lo que hay que llevar al poste:**
>
> | | ~~antes (activo en BAJO)~~ | **hoy (activo en ALTO)** |
> |---|---|---|
> | Canal `A` | ~~puentear `J16` p5 a masa~~ | **cerrar `J16` p5 contra p4 (3,3 V)** |
> | Canal `B` | ~~puentear `J16` p8 a masa~~ | **cerrar `J16` p8 contra p7 (3,3 V)** |
>
> 🛑 **Y por eso el receptor RF del mando YA NO ES UNA DECISIÓN ABIERTA: se compra NORMALMENTE
> ABIERTO (`NO`).** En reposo el contacto queda abierto y `R65`/`R66` mantienen el pin en 0 V; al
> accionar, cierra contra los 3,3 V y produce el **flanco de subida** que el firmware busca. Un
> receptor `NC` tendría el pin en alto permanente —el equipo leyendo pulsación continua—, y además
> un canal caído o un receptor sin alimentación quedarían en reposo con `NO`, que es la dirección
> segura: **el mando no manda nada en vez de mandar solo.**
>
> ### ⚠️ Lo que NO se puede decir todavía
>
> **Que el mando funcione.** Lo medido es el fuente, no la tarjeta: **el firmware corregido no se ha
> cargado ni se ha ejercido en banco**, y `N-118` no se cierra con una lectura de código. Hasta esa
> prueba, y para todo lo que se planifique desde hoy:
>
> * **No cuente con respaldo físico operativo.** La app por Bluetooth sigue siendo la vía de mando
>   con la que se opera.
> * 🔴 **En el ESCLAVO eso sigue cerrando el círculo, porque allí la app no manda de modo** (§9):
>   mientras el arreglo no se ejerza, **no hay forma demostrada de cambiarle el modo al Esclavo** —
>   ni entrar ni salir del Degradado, ni quitar un `ambarLocal` ya puesto.

### ✅ EL VOCABULARIO REAL — MEDIDO en `Maestro/src/mando.cpp:201-238` y `Esclavo/src/mando.cpp` (31/08)

**Son TRES secuencias, y no hay más.** *(Escritas en el firmware. **En el banco del 3–4/09 ninguna se
pudo accionar**; el firmware de hoy ya lee los pines activo en ALTO y **falta ejercerlo en tarjeta** —
ver el recuadro `N-118` de arriba, con el gesto de prueba que cambió.)*

| Secuencia | Modo Activado | Confirmación Lumínica | Dónde está |
|---|---|---|---|
| **`A · A · A`** (≤ 12 s) | 🟢 **Modo Automático** | **2 destellos rojos** | `mando.cpp:226-228` · `DESTELLOS_AUTOMATICO = 2` (`:45`) |
| **`B · B · B`** (≤ 12 s) | 🟡 **Modo Ámbar (Seguro)** | **3 destellos rojos** | `mando.cpp:230-235` · `DESTELLOS_AMBAR = 3` (`:46`) |
| **`A · B · A · B`** (≤ 18 s) | 🕒 **Modo Degradado (Reloj RTC)** | **4 destellos rojos** | `mando.cpp:203-219` · `DESTELLOS_DEGRADADO = 4` (`:47`) |

* **Las ventanas son dos constantes distintas y las dos están medidas:** `VENTANA_TRIPLE_MS = 12000`
  (12 s, `mando.cpp:38`) para las de tres pulsos, y `VENTANA_CUADRUPLE_MS` (18 s) para `A·B·A·B`.
  **El reloj cuenta desde el PRIMER pulso de la secuencia, no desde el último.**
* **`A·B·A·B` se comprueba ANTES que las otras dos, y no hay ambigüedad:** sus tres últimos pulsos
  son `B·A·B`, que no es ni `A·A·A` ni `B·B·B`.
* 🛑 **`A·B·A·B` puede ser RECHAZADO aunque se accione bien.** El Degradado exige que la hora esté
  validada: si no lo está, el equipo **rechaza** en vez de entrar (`mando.cpp:214-218`). **La red de
  seguridad no es la secuencia: es esa comprobación.** El mando permite reactivar en campo sin
  grúas, pero **no saltarse la puesta a punto**.
* ✅ **`B·B·B` no tiene condiciones y funciona desde cualquier modo en marcha.** Es deliberado, y el
  fuente lo dice: *«es la regla que impide que nadie quede atrapado con un semáforo en estado raro a
  5 m de altura»*.
* ⚠️ **Un error a mitad de secuencia no hace nada.** Si el operario se equivoca, lo único que ha
  hecho es no activar el modo; no hay estado intermedio que deshacer.
* 👁️ **La confirmación no necesita app, ni cable, ni una segunda persona: son las propias luces.**
  Los destellos de la tabla se cuentan **desde el suelo**, que es donde está quien acciona el mando.
  Un **ámbar rápido de 2 s** en vez de destellos significa **rechazado**, no *«no me oyó»*.
* 🛑 **Y al PROBARLO, hágalo DESDE OTRO MODO.** Si el equipo ya está en el modo que la secuencia
  pide, `MODO:` **no cambia** —el firmware entra por la rama
  `if (modoActual_get() == MODO_AUTOMATICO) modoAutomatico_setup();`— y quien mire la app o el
  terminal **no distingue nada**, aunque el mando haya funcionado perfectamente. **Cuente los
  destellos: ésos se ven siempre.**

> 🛑 **Y una advertencia de operación que sale de la misma medida: `A` y `B` son los MISMOS PINES
> que los pulsadores 1 y 2 del gabinete** (`MANDO_A` = `PB9` = `J16` p5, `MANDO_B` = `PB13` = `J16`
> p8). El mando de relés va cableado **en paralelo** con ellos. **Pulsar tres veces seguidas el
> botón 1 del gabinete cambia el modo del semáforo exactamente igual que hacerlo desde el suelo.**
>
> 🟠 **MATIZ DEL 04/09 (`N-118`): en el banco del 3–4/09 no cambiaba nada, ni desde el gabinete ni
> desde el suelo** — el mismo pin, el mismo camino, y ese camino clavado en BAJO por la lectura
> `INPUT_PULLUP` + `== LOW` contra `R65`/`R66`. **El firmware de hoy lo lee activo en ALTO y el
> efecto vuelve en cuanto esté cargado**; lo que no ha cambiado nunca es el reparto de pines, que
> es lo que hay que conocer para no accionar el mando sin querer desde el gabinete.

> 🔴 **LO QUE NO TIENE MANDO, dicho entero porque es lo que se echa de menos en obra:** **no hay
> secuencia para el Modo Manual ni para el Modo Inteligente.** Esos dos se ordenan **sólo desde la
> app**, y **sólo en el Maestro** (`Maestro/src/bluetooth.cpp:177+`). Ver §9.

### ⚠️ Las tres secuencias son las mismas en los dos postes, pero `A·A·A` NO hace lo mismo

**MEDIDO** en `Esclavo/src/mando.cpp:238-249` el 31/08. Es una diferencia que el operario tiene que
saber, porque el gesto es idéntico y el resultado no:

| Secuencia | En el **MAESTRO** | En el **ESCLAVO** |
|---|---|---|
| **`A · A · A`** | 🟢 **Arranca el Modo Automático** — el Maestro decide el ciclo | 🔵 **`OBEDECER`: devuelve el mando al Maestro.** No arranca ningún ciclo propio. Si estaba en Degradado sale ordenado por el todo-rojo; si no, **se queda en ROJO** esperando la primera orden por radio |
| **`B · B · B`** | 🟡 Ámbar de seguridad | 🟡 Ámbar de seguridad — **igual**, y además **marca el ámbar como LOCAL** *(ver el aviso de abajo)* |
| **`A · B · A · B`** | 🕒 Entra en Degradado *(si la hora está validada)* | 🕒 Entra en Degradado *(si además hay configuración de ciclo y sincronización vigente — `degradado_comprobar()`, `mando.cpp:229`)* |

> 🛑 **El `B·B·B` del Esclavo hace algo más de lo que se ve, y es una protección deliberada.**
> Marca ese ámbar como **puesto por un operario en el sitio** (`ambarLocal`, `Esclavo/src/mando.cpp:132`),
> y mientras esa marca esté puesta **una orden que llegue por radio NO saca a ese poste del ámbar**
> (los tres `if` de `Esclavo/src/main.cpp:406`, `:416` y `:540`). Es desobediencia a propósito: si
> alguien dejó el poste en ámbar porque hay gente trabajando delante, el Maestro no se lo quita.
>
> **Para quitarlo hay que ir al poste y hacer `A·A·A`** — que es lo único que baja esa marca
> (`mando.cpp:116`). **No hay forma de quitarla desde la app**, ni desde el Maestro.
>
> ⚠️ **04/09 (`N-118`) — en banco esa marca NI SE PONÍA NI SE QUITABA**, porque `B·B·B` y `A·A·A` no
> se podían accionar. Era un empate que no tranquilizaba, **y con el firmware ya corregido el empate
> se deshace en los dos sentidos a la vez**: en cuanto el arreglo se cargue y se ejerza, `B·B·B`
> vuelve a poner la marca y `A·A·A` vuelve a ser lo único que la baja. **Sigue sin haber forma de
> bajarla desde la app**, así que la protección y su cerradura siguen siendo la misma pieza y siguen
> exigiendo ir al poste.

* **Inhibición de UI (N-53):** Mientras el equipo esté en pantallas de configuración (`AJUSTAR HORA`, `CONFIG_TIEMPOS`), el receptor del mando se inhibe al 100%, evitando que los pulsos de edición disparen cambios de modo involuntarios.
  > ⚠️ ~~**28/08 — esta protección sigue en el firmware, pero ahora protege un menú que nadie ve.**
  > La inhibición impide que las **secuencias** se reconozcan con el menú abierto; **no impide la
  > navegación**, porque navegar es justo lo que el menú abierto acepta. Sin pantalla, un operario
  > que accione el mando sin saber que el equipo está dentro del menú **no tiene forma de enterarse**:
  > no verá destellos (están inhibidos) y creerá que el mando no funciona, mientras el cursor se
  > mueve a ciegas. **Es un riesgo abierto mientras el menú siga compilado en el binario**, y está
  > anotado también en `OPTIMIZACIONES.md`.~~
  >
  > 🟢 **CERRADO EL 31/08 por efecto lateral, y en la dirección buena.** El riesgo era real y ya no
  > existe: **con `ACEPTAR` mudo, la pantalla no puede bajar del listado**, así que
  > `menu_estaAbierto()` **es siempre falso** y **el mando no se puede quedar inhibido por un menú
  > que alguien dejó abierto**. Está escrito y razonado en el propio firmware, junto a las
  > definiciones de `botonAceptar()`/`botonCancelar()` (`Maestro/src/botones.cpp`, cabecera del
  > bloque «SIN SUJETO»).
  >
  > **Se conserva tachado**: describe con exactitud un riesgo que estuvo abierto tres días, y quien
  > lo leyera hoy como vigente estaría desconfiando del mando por el motivo equivocado.
  >
  > ⚠️ **Lo que sí sigue vigente de N-53, y no ha cambiado:** el mando va **en paralelo** con los
  > pulsadores 1 y 2 del gabinete, así que una ráfaga de pulsaciones sigue siendo una secuencia. La
  > protección que hoy vale no es la inhibición sino la **ventana de tiempo** (12 s / 18 s) y la
  > **confirmación por destellos**.

---

## 8. Módulo Bluetooth: LA INTERFAZ DEL EQUIPO (Estándar Baliza) — 🔴 ACTUALIZADO EL 28/08

Desde el 28/08 esto **ya no es un accesorio de soporte: es la única interfaz del equipo.**

### Conexión — el módulo entra por `J17`, en los pines que dejó la pantalla

> ## ⛔ ANTES DE CONECTAR NADA: `J16` LLEVA 12 V Y QUEMA EL MÓDULO
>
> **`J16` y `J17` son dos conectores distintos y se parecen.** `J16` es el de la **botonera** y
> **trae 12 V en su posición 1**; `J17` es el de la pantalla y reparte **3,3 V**. Enchufar el módulo
> Bluetooth en `J16` le mete **12 V a una entrada de 3,3 V: se quema, y no avisa antes**.
>
> **Cómo distinguirlos, y es una medida, no una mirada:** con el equipo energizado, **mida la
> posición 1 contra GND**. **Si hay 12 V, ése es `J16` — no es el suyo.** En `J17` la posición 1 es
> `CS`, una señal de la pantalla, no una alimentación.

| `J17` | red | va a | al módulo Bluetooth |
|---|---|---|---|
| **p2** | `RST` → `PB7` | `USART1_RX` del STM32 | **`TXD`** del módulo |
| **p3** | `RS(A0)` → `PB6` | `USART1_TX` del STM32 | **`RXD`** del módulo |
| ~~**p6**~~ | ~~`3,3 V`~~ | ~~alimentación~~ | 🛑 **NO SE CONECTA** — ver abajo |
| **p7** | `GND` | masa | `GND` — **obligatoria** |

> ## 🔴 CORREGIDO EL 31/08 — **LA FILA `p6` MANDABA ALIMENTAR EL MÓDULO DESDE LA TARJETA, Y CON EL MÓDULO DE HOY ESO REINICIA EL SEMÁFORO**
>
> **Si ya se cableó un `ESP32` a `J17` p6 siguiendo la versión anterior: DESCONECTE ESE HILO ANTES
> DE ENERGIZAR.** No es un error de qué módulo, es un error de **de dónde sale su corriente**.
>
> **El módulo que va en `J17` desde el 28/08 ya no es un `HC-05`: es un `ESP32`** —y `BLQ-1` cerró
> el 31/08 confirmando que el que llegó es un `ESP32-WROOM-32` clásico, con SPP—. La diferencia que
> importa aquí es el consumo:
>
> | | consumo de pico | ¿se puede alimentar de `J17` p6? |
> |---|---|---|
> | `HC-05` / `JDY-30` *(el módulo de antes)* | ~40 mA | ✅ sí — para eso estaba escrita esta fila |
> | **`ESP32-WROOM-32`** *(el de hoy)* | **~500 mA** | 🛑 **NO** |
>
> **Por qué no, y es una cuenta:** los 3,3 V de `J17` salen del `LM1117-3.3`, que cuelga del
> `LM7805`, **que es el mismo riel que alimenta al STM32 que gobierna el semáforo**. A 500 mA el
> `7805` disipa 3,5 W sin disipador; y si el riel de 3,3 V se hunde un instante, **se reinicia el
> micro que mueve las luces**. El accesorio no puede tumbar al que manda.
>
> ✅ **Cómo va: `12 V → convertidor DC-DC CONMUTADO → 5 V → `VIN`/`5V` del módulo`.** La ficha del
> módulo comprado da entrada recomendada **5 V**, límite **5,5 V**, con regulador a 3,3 V a bordo.
> **La fuente es una compra aparte que HOY NO ESTÁ PEDIDA** — línea `A5` del Manual 15.
>
> 🛑 **Y las dos cosas que se confunden y no son la misma:**
>
> * **La MASA COMÚN entre `J17` p7 y el módulo es OBLIGATORIA.** Sin ella el enlace serie no tiene
>   referencia y no funciona.
> * **La ALIMENTACIÓN COMPARTIDA está PROHIBIDA.** De `J17` se usan **la señal y la masa, y nada
>   más**.
>
> **Confundir esas dos frases es lo que hace que un semáforo se reinicie solo.** Manual 10 §1.
>
> ⚠️ **Las E/S del módulo son de 3,3 V**, igual que las del STM32 (LEÍDO en la ficha, 31/08): el
> enlace `p2`/`p3` va **directo, sin adaptador de niveles**.

* **Es el `USART1` REMAPEADO.** El STM32F103 puede sacar el `USART1` por `PB6`/`PB7` en vez de
  `PA9`/`PA10`, **pero solo por un sitio a la vez**. El firmware ya está en esa posición: declara
  `SerialBT(PB7, PB6)` en `bluetooth.cpp`, en las dos puntas.
* **Baudrate:** 9600 bps, 8-N-1.
* **`PA9`/`PA10` sigue siendo válido eléctricamente, pero NO es el montaje vigente.** Esos dos pines
  **no salen a ninguna bornera de la tarjeta**: para usarlos hay que **soldar** en las patas del
  MAX3485 `U2` o del propio micro. Queda como alternativa de laboratorio, no como el cableado de
  campo. *(Este manual decía antes `PA9` TX / `PA10` RX; era correcto para el montaje anterior y ha
  quedado obsoleto, no borrado.)*

> ⚠️ **Todo lo anterior está MEDIDO EN EL ESQUEMÁTICO** (`03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md`
> §7) **y en el fuente del firmware. NADA de esto se ha comprobado con multímetro sobre el cobre.**
> El propio mapeo advierte además de que el símbolo de `J17` tiene 13 posiciones y el footprint 16:
> **al contar pines sobre la tarjeta física hay que contar desde el pin 1, no desde el borde del
> conector.** La verificación de continuidad con la tarjeta delante sigue pendiente.

### 🔵 CÓMO SE ENLAZA EL TELÉFONO — MEDIDO EN BANCO EL 3–4/09

**Los tres tropiezos de abajo costaron horas de banco, y ninguno era una avería del equipo.**

**1. La APK tiene que ser la del 04/09.** El fichero es
**la APK del 04/09 que acompaña a este paquete** — su nombre exacto y su `md5` estan en `LEEME_PRIMERO.md`, en la raíz del `.zip`, que es el único sitio donde no caducan.
**Compruebe el `md5` antes de instalar**, porque
el nombre lo puede llevar cualquier fichero y el hash no.

> 🔴 **Con cualquier APK anterior la app NO CONECTA, por bien que funcione el módulo (`N-122`).**
> Hasta esa versión, al tocar una fila de la lista de equipos la app **se daba por enlazada sin
> haber abierto nunca el socket Bluetooth**: guardaba la dirección, se pintaba **«Enlazado»** y se
> ponía a escuchar un canal que no existía. **Los comandos se iban al vacío y el equipo no contestaba
> nunca.** Está arreglado —ahora la app abre el socket y sólo se declara enlazada si el socket
> abrió—, **pero el arreglo va dentro de la APK: no se hereda actualizando el equipo.**
>
> **Cómo distinguirlo en el poste, que es lo que hace falta:** con la APK vieja la app dice
> «Enlazado» **y no llega ni una trama**. Una app enlazada de verdad recibe `$STATUS` ~~**cada
> segundo**~~ **cada 2 segundos** *(cadencia bajada el 04/09)*. Si el rótulo dice enlazado y la pantalla no se mueve, sospeche de la APK antes que del
> equipo.

> 🔵 **Y esa misma APK trae un segundo cambio que ALTERA EL ORDEN DE LO QUE HACE EL TÉCNICO
> (`N-124`): la lista de equipos ya NO lleva direcciones `MAC` escritas a mano.** Antes la app
> traía un par de `MAC` fijas dentro; hoy **la lista sale del escaneo real del teléfono**, así que
> **un equipo que Android no conozca no aparece**.
>
> **Los dos pasos, y en este orden:**
>
> 1. **EMPAREJAR el `ESP32` en Ajustes de Android** *(Bluetooth → dispositivos disponibles →
>    `SEM-SIN-MATRICULA`, ver el punto 2)*. Empareja sin PIN, «Just Works» — punto 3.
> 2. **Abrir la app y pulsar «Buscar Módulos Bluetooth».** Sólo entonces sale el equipo en la lista,
>    y sólo entonces se puede tocar la fila para conectar.
>
> 🛑 **Si se salta el paso 1, la lista sale vacía y no hay nada que pulsar** — y eso **no** es un
> módulo muerto ni una APK mala. Es la app diciendo, correctamente, que el teléfono todavía no
> conoce a ese equipo.

**2. Cómo se distingue un poste del otro en la lista de Bluetooth — y el caso en que NO se
distinguen.**

> 🔵 **El módulo se auto-rotula, y la letra final dice QUÉ POSTE ES.** No es una opción de
> compilación: el mismo binario sirve a las dos puntas y **el nombre lo aprende del campo `NODE:` de
> la trama `$STATUS`** que emite la tarjeta. **MEDIDO en
> `01_Firmware/ESP32_Expansion/src/transporte_app.cpp:80-114`** y
> `include/contrato.h:258-259`:
>
> | lo que sale en la lista de Android | qué es |
> |---|---|
> | **`SEM-<serie>-M`** | el poste **MAESTRO** |
> | **`SEM-<serie>-E`** | el poste **ESCLAVO** |
> | **`SEM-SIN-MATRICULA`** | el módulo **todavía no ha aprendido** de qué poste cuelga |
>
> **Desde la decisión del 04/09 —el cruce se opera desde el Maestro, §9— esa letra final deja de ser
> un detalle y pasa a ser lo primero que hay que mirar antes de mandar una orden.**

> 🛑 **Y AQUÍ ESTÁ EL CASO QUE HAY QUE SABER EN OBRA: UN MÓDULO RECIÉN PUESTO ANUNCIA
> `SEM-SIN-MATRICULA`, Y LOS DOS POSTES SE LLAMAN IGUAL HASTA QUE SE LES DA UNA VUELTA DE ENERGÍA.**
>
> **El rótulo aprendido se guarda para la SIGUIENTE arrancada, y es deliberado** *(`transporte_app.cpp:109-113`)*:
> cambiar el nombre SPP en caliente obliga a cerrar y reabrir el perfil Bluetooth, o sea **a tirar
> la sesión del operario que en ese momento puede estar dando una orden al cruce**. Se prefirió no
> renombrar en caliente.
>
> **Consecuencia real, y no es teórica: con dos módulos nuevos en el mismo frente de obra, los dos
> aparecen en la lista como `SEM-SIN-MATRICULA` y el técnico se conecta a ciegas** — sin saber si
> tiene delante el Maestro, que es el que manda, o el Esclavo, que casi no.
>
> **Qué hacer, y va en la puesta en marcha:**
>
> 1. **Con el módulo montado y la tarjeta encendida, déjelo un minuto**: le basta con recibir un
>    `$STATUS`, que sale cada 2 s, para aprender su matrícula y guardarla.
> 2. **Déle una vuelta de energía al módulo** —quitar y devolver tensión—. **En el arranque
>    siguiente ya sale con su nombre bueno**, con su `-M` o su `-E`.
> 3. **No dé el módulo por muerto ni lo cambie** por anunciarse `SEM-SIN-MATRICULA`. Y si busca
>    `IOT_VIAL` o el nombre del cruce, **no lo va a encontrar**: no se llama así.
>
> ⚠️ **Mientras no se haya reiniciado, la matrícula que vale es la del campo `SERIE:` de la trama
> `$STATUS` que la app enseña — no la del nombre Bluetooth.** Ver un `SEM-SIN-MATRICULA` **no**
> significa «equipo sin configurar»: puede ser un equipo con su serie ya puesta que todavía no se ha
> reiniciado.

**3. El emparejamiento NO pide PIN, y el `1234` no es el PIN de emparejamiento.**

> ⚠️ **El `ESP32` empareja por «Just Works»: SIN PIN del sistema operativo.** Android enlaza sin
> preguntar nada. **Si el teléfono le pide un PIN de emparejamiento, no está hablando con este
> módulo.**
>
> 🔴 **El `1234` es un PIN DE COMANDO DENTRO DE LA APP** —el que va en la trama
> `CMD:PIN:1234:…`—, **no el PIN de emparejamiento de Bluetooth.** Se confundieron en banco: se
> tecleó `1234` en el diálogo del sistema operativo esperando que enlazara. Son dos cosas distintas
> y sólo una existe.

**4. ~~Ese PIN de la app NO CADUCA. Riesgo conocido y ABIERTO (`AB-9`).~~** → 🟢 **EL PIN SÍ CADUCA
DESDE EL 04/09. Se tacha en vez de borrarse, porque quien leyera esto como vigente dejaría el
teléfono desbloqueado creyendo que da igual.**

> 🛑 ~~**Se teclea una vez y el teléfono queda autorizado hasta que se cierra la app.** No hay tiempo
> de expiración, ni bloqueo por inactividad, ni recuento de órdenes.~~
>
> **MEDIDO en `App_Semaforo/app.js`:** el PIN caduca **a los 60 s de irse la app al fondo**
> (`PIN_GRACIA_FONDO_MS = 60 * 1000`, `:1918`, aplicado en `:1959`) y **a los 5 min sin mandar
> ninguna orden** (`PIN_INACTIVIDAD_MS = 5 * 60 * 1000`, `:1919`, aplicado en `:1980-1981`).
>
> 🔴 **Este bloque contradecía a la cabecera de este mismo manual y a su §3**, que ya lo decían bien
> desde el 04/09 (`:104-105` y `:164-167`). Era la corrección que no llegó hasta aquí.
>
> ⚠️ **Y lo que SÍ sigue abierto de `AB-9`, que es la mitad que se conserva:** la caducidad por irse
> al fondo **cuelga de sucesos del navegador** (`visibilitychange` / `pagehide` / `pageshow`) y
> **nadie la ha visto disparar en la APK con la pantalla apagada**. **Hasta que alguien lo
> cronometre en el teléfono, la barrera real sigue siendo quién tiene el aparato en la mano.**
>
> **Lo que eso significa en obra:** si alguien deja el teléfono desbloqueado y otra persona lo coge,
> **manda sobre el cruce sin teclear nada** — y mientras el mando de relés no se haya visto entrar
> en tarjeta (§7, `N-118`), la app es la única vía de mando comprobada: eso es todo el mando del
> equipo.
>
> ⚠️ **Se escribe como riesgo conocido, no como algo resuelto.** Cuánto debe durar una sesión
> autorizada —y si debe caducar por tiempo, por inactividad o al cambiar de nodo— **es decisión del
> responsable**, porque el coste de equivocarse cae en los dos sentidos: una sesión que caduca
> demasiado pronto obliga a teclear cuatro dígitos delante de un cruce parado. Mientras tanto, la
> única barrera real es **quién tiene el teléfono**.

### Telemetría en vivo — y qué campos NO son medidas

* **Emisión periódica de `$STATUS,...` ~~cada 1 segundo~~ cada 2 segundos.** *(cadencia bajada a **2000 ms** el 04/09, decision del responsable, en las DOS puntas — MEDIDO: `Maestro/src/bluetooth.cpp:851`, `Esclavo/src/bluetooth.cpp:768`. Un tecnico que cronometre con «1 segundo» declara caido un enlace sano.)* Formato real, leído del firmware:
  `$STATUS,NODE:...,SERIE:...,MODO:...,ESTADO:...,T:...,RF:...%,RTT:...ms,BAT:...,HORA:...*XX`

> 🔴 **Tres campos de esa trama NO son medidas, y hay que saberlo antes de decidir con ellos.**
> Medido el 28/08 sobre `bluetooth.cpp` en las dos puntas:
>
> | campo | Maestro | Esclavo |
> |---|---|---|
> | `RF:` | ✅ **real** — telemetría del latido de 3 s (SFTY-14) | 🔴 **literal `98%`** escrito en el `snprintf` |
> | `RTT:` | ✅ **real** | 🔴 **literal `85ms`** |
> | `BAT:` | 🔴 **literal `12.6`** | 🔴 **literal `12.6`** |
> | `T:` | ⚠️ **no es la cuenta regresiva**: es `(millis()/1000) % 60`, un contador libre de 0 a 59 | ⚠️ igual |
>
> **Este manual prometía antes «cuenta regresiva» y «% de señal RF».** La cuenta regresiva **no
> existe en la trama**, y el `RF:` del Esclavo **afirma un 98 % aunque la radio esté desconectada**.
> Un tablero que inventa el dato que no tiene es peor que uno que se calla: quien decide sobre el
> tráfico mirándolo cree estar viendo el enlace. **Se deja escrito en vez de corregirse en silencio,
> porque el arreglo es de firmware y no de manual.**
* **Caja Negra de Alarmas:** Registro inmediato de eventos con timestamp (`$ALARM,EVENTO:FALLO_RF,CAUSA:SILENCIO_25000ms...` —**el nombre del evento ya no lleva el número dentro**: el umbral va en la causa, para que no quede mintiendo el día que se ajuste) para diagnosticar la causa exacta de cualquier caída de radio en obra.
* **Operación Multicruce (Un solo celular para la vía):** La App permite gestionar toda la carretera con un selector de cruces viales (Km 12, Km 24, etc.) y detecta automáticamente si está conectada a `👑 MAESTRO (Poste 1)` o `📡 ESCLAVO (Poste 2)`.
* **Modo Asistente Courier RTC (Sincronización Puente sin Radio):** Si no hay enlace de radio entre postes, el técnico captura hora y ciclo en el Maestro, viaja en vehículo al Esclavo, y la App inyecta la sincronización compensando automáticamente el tiempo de viaje con su reloj interno de alta precisión ($\Delta t < 0.1\text{ s}$).

  > 🔴 **31/08 — LEA ESTO ANTES DE HACER UN VIAJE DE COURIER: HOY PUEDE VOLVER SIN HABER PUESTO LA
  > HORA, Y AHORA EL EQUIPO SÍ LO DICE.**
  >
  > El cristal de reloj `Y2` de 32.768 kHz **no oscila en la tarjeta medida en banco** (N-17 y N-37,
  > medida del 01/08). **La segunda tarjeta no está diagnosticada.** Sin oscilador no hay nada que
  > haga avanzar los segundos, así que **el equipo se niega a aceptar la hora en vez de guardar una
  > que nadie va a mover**.
  >
  > **Las tres respuestas posibles de `CMD:PIN:…:SET_RTC:…`, MEDIDAS en el fuente, y qué hacer con
  > cada una:**
  >
  > | respuesta | qué pasó | qué hace el técnico |
  > |---|---|---|
  > | `$ACK,CMD:SET_RTC,RESULT:OK` | La hora **entró** | Listo. Puede bajarse del poste |
  > | `$ERR,CMD:SET_RTC,DESC:SIN_CRISTAL_VEA_CONSULTA_RELOJ` *(Maestro)* · `DESC:SIN_CRISTAL` *(Esclavo)* | 🛑 **No hay con qué contar el tiempo. La hora NO entró** | **El viaje no sirvió.** Es avería de hardware. **Antes de bajarse, mande el mismo comando una segunda vez** y apunte lo que dice la trama de abajo — es el dato que decide qué pieza se toca |
  > | `$ERR,CMD:SET_RTC,DESC:FORMATO_INVALIDO` | La app mandó algo que no se entiende | Reintentar |
  >
  > > ### 🛑 `CONSULTA RELOJ` NO SE PUEDE ABRIR — el mensaje nombra una pantalla tapiada
  > >
  > > Ese `$ERR` le manda a `CONSULTA RELOJ`, y **esa pantalla ya no es alcanzable**: está dentro de
  > > `CONFIGURACION` y llegar ahí necesita dos pulsaciones de *Aceptar*, que hoy no existen.
  > >
  > > **✅ Pero el equipo manda esos mismos datos SOLO, justo detrás del rechazo.** Búsquelos en la
  > > pestaña **`Eventos`** de la app:
  > >
  > > ```
  > >   $EVENT,NODE:MAESTRO,ORIGEN:RELOJ,DETALLE:ON:1 RDY:0 BYP:0 SEL:1 EN:1 CNT:0,HORA:--:--:--
  > > ```
  > >
  > > | lo que lea | qué significa |
  > > |---|---|
  > > | `ON:0` | el oscilador **ni se pide**. **No es el cristal** — no lo cambie |
  > > | `ON:1 RDY:0` | **pedido y no oscila.** Aquí **sí** se mira `Y2` y sus condensadores |
  > > | `ON:1 RDY:1 SEL:0` | oscila bien; lo que falla es el enganche. **No es el cristal** |
  > > | `CNT:--` | no se pudo **leer** el contador. **No es lo mismo que `CNT:0`** |
  > >
  > > 🔴 **Apunte estos seis números antes de bajarse del poste.** Sin ellos, arriba sólo se sabe
  > > *«el reloj no va»*, y este proyecto ya mandó cambiar pila, resistencia y cristal **tres veces
  > > con el hardware sano** por no tener este dato.
  > >
  > > 💡 **Mande el comando dos veces con unos segundos de diferencia y compare el `CNT`:** si cambia,
  > > el reloj cuenta. No cuesta nada — en este caso el comando se rechaza **antes** de escribir.
  >
  > > ⚠️ **Y por qué esto se escribe con tanto detalle: hasta hace poco ese comando contestaba
  > > `RESULT:OK` sin haber puesto nada.** Con el `Y2` muerto ése era el caso **normal**, no el
  > > raro — **el técnico se iba del poste creyendo que había dejado el reloj puesto**. Está
  > > arreglado, pero **el reloj sigue sin poder ponerse**: lo que se arregló es que ahora el equipo
  > > lo dice. Un `$ACK` que no depende de lo que la llamada devolvió es una mentira con formato de
  > > éxito.
  >
  > 🕒 **Lo que lo destraba de verdad es un reloj `DS3231` externo colgado del módulo `ESP32`** —
  > línea `A6` del Manual 15—, **que ni está comprado ni tiene todavía firmware que le hable**.

---

## 9. 🔴 QUÉ SE PUEDE MANDAR A CADA POSTE — y por qué no es lo mismo en los dos (31/08)

> **Este apartado no existía, y es el que un operario necesita antes de subirse a un poste.** La app
> **no** manda lo mismo en el Maestro que en el Esclavo, y el manual daba a entender que sí.

> ## 🟢 DECISIÓN DEL 04/09/2026 — **EL CRUCE SE OPERA DESDE EL MAESTRO**
>
> **No se va a hacer transparente el mando desde el Esclavo.** La asimetría de la tabla de abajo
> deja de ser una limitación pendiente de cerrar y pasa a ser **cómo se opera este equipo**.
>
> **Lo que eso significa para el operario, que es lo único que hay que recordar de este apartado:**
> para **dar paso, cambiar de modo o ajustar tiempos hay que conectarse al poste MAESTRO**. Desde el
> Esclavo sólo se puede **pedir paso** (`SOLICITAR_PASO`), **poner ámbar de emergencia**,
> **retirarlo** (`CANCELAR_AMBAR`) y **el intento de forzar rojo, que esa punta rechaza**.
>
> ⚠️ **Consecuencia de obra que hay que planificar antes de subir:** si se llega al Poste 2 y hace
> falta cambiar el modo, **hay que desplazarse al Poste 1**. La app lo dice —cuando la orden no es
> de esta punta avisa a cuál va— pero **no lo hace por usted**: el enlace Bluetooth alcanza el poste
> que tiene delante, no el otro.
>
> 🔵 **Por eso la §8 de este manual pasa a ser crítica: hay que saber A QUÉ POSTE se está
> conectando.** El módulo se auto-rotula `SEM-<serie>-M` (Maestro) o `SEM-<serie>-E` (Esclavo), y
> hay un caso en el que **los dos se llaman igual**. Está escrito allí.

**MEDIDO el 31/08 y RE-CENSADO EL 04/09** sobre `Maestro/src/bluetooth.cpp` y
`Esclavo/src/bluetooth.cpp`:

| | **MAESTRO** | **ESCLAVO** |
|---|---|---|
| Cambiar de modo desde la app | ✅ **Sí.** `SET_MODO:AUTO` · `MANUAL` · `AMBAR` · `MENU` · `ALCANCE` · `INTELIGENTE` · `DEGRADADO` (`:444+`) | 🛑 **NO. No existe ni un solo `SET_MODO`** — `grep -n "SET_MODO" Esclavo/src/bluetooth.cpp` → **CERO coincidencias** |
| Ámbar de emergencia desde la app | 🛑 **NO existe `AMBAR_EMERGENCIA` en el Maestro** — re-censado el 04/09. Lo que hay es `SET_MODO:AMBAR` (**con PIN**, y es un **modo**, no un latch) y `FORZAR_ROJO`. 🔵 **05/09 (N-146): si el equipo YA estaba en modo ámbar, esa orden RE-ARMA y contesta `RESULT:REARMADO` en vez de `OK`** | ✅ sí — `CMD:AMBAR_EMERGENCIA` (`:381` sin PIN, `:468` con PIN). Arma un **latch** que veta las órdenes de radio. 🟢 **05/09 (N-142): y AHORA AVISA AL MAESTRO por radio (`CMD_AMBAR_ESCLAVO`), que se va a modo ámbar en el acto.** Antes el Maestro podía seguir dando VERDE **hasta 3 minutos** con este lado en ámbar |
| Retirar el ámbar de emergencia | ❌ no aplica: no hay latch que retirar | ✅ `CANCELAR_AMBAR` (`:491`, **con PIN**). Contesta `RETIRADO` o `RETIRADO_QUEDA_MANDO` |
| Pedir paso | ✅ `DEMANDA` (`:645`, **sólo en Modo Inteligente**) | ✅ `SOLICITAR_PASO` (`:532`) *(se lo pide al Maestro)* — ver `N-130` abajo |
| Poner los tiempos | ✅ `SET_TIEMPOS` (`:542`) | ❌ no — los tiempos los lleva el Maestro |
| `FORZAR_ROJO` | ✅ **sí, y para de verdad** (`:412` sin PIN, `:520` con PIN) | 🛑 **NO.** Contesta `$ERR,CMD:FORZAR_ROJO,DESC:RENOMBRADO_USE_AMBAR_EMERGENCIA` (`:448`, `:524`) y **no para nada** |
| `TEST_LEDS` | ✅ sí (`:538`) | 🛑 **rechazado a propósito** (`:550`): `NO_EN_SERVICIO_USE_EL_MAESTRO` |
| Menú local / botones | ❌ sin sujeto | ❌ sin sujeto |
| **Mando de relés (A y B)** | 🟠 **No se pudo accionar en banco (`N-118`). Firmware corregido, SIN ejercer en tarjeta** | 🟠 **Igual — y aquí es además la única vía de mando de la punta** |

> ✏️ **CORRECCIÓN DEL 04/09 — la fila del ámbar de emergencia decía «✅ sí» en las DOS puntas y era
> falsa en la del Maestro.** `grep -n "AMBAR_EMERGENCIA" Maestro/src/bluetooth.cpp` → **cero
> coincidencias**. La confusión venía de que el operario ve un botón de ámbar en la app estando
> delante de cualquiera de los dos postes; lo que **manda** al Maestro es otra cosa —`SET_MODO:AMBAR`,
> un modo con PIN— y **no arma el latch que veta las órdenes de radio**. La app ya lo enruta bien
> (`SOLO_ESCLAVO` en `app.js:2057`); lo que estaba mal era este manual.

> ### 🟢 `N-130` (04/09) — EL EQUIPO YA NO DICE QUE SÍ A LO QUE NO VA A HACER
>
> **Esto es COMPORTAMIENTO NORMAL, no una avería, y hay que reconocerlo en obra.**
>
> Si el técnico pulsa **«Solicitar Paso» desde el ESCLAVO** y el cruce **no está en Modo
> Inteligente**, el Maestro **rechaza la demanda** y el Esclavo lo dice en la bitácora con el evento:
>
> ```
>   MAESTRO / DEMANDA_NO_ATENDIDA_MODO_ACTUAL
> ```
>
> **Antes decía «pedido al Maestro» y no pasaba nada.** El Esclavo tiene que contestar a la app
> antes de saber la respuesta del Maestro —bloquear su bucle esperando una radio de 2,4 kbps es
> peor—, así que **el primer acuse sigue siendo `PEDIDO_AL_MAESTRO`** y el desmentido llega después,
> como **evento fechado**. **MEDIDO:** `Maestro/src/coordinador.cpp:636-641` decide con
> `modoActual_get() == MODO_INTELIGENTE` y manda `DEMANDA_ACEPTADA` o `DEMANDA_RECHAZADA`;
> `Esclavo/src/main.cpp:541-543` lo convierte en el evento.
>
> **Qué hacer si sale:** no es un fallo del Esclavo ni del enlace. **Es que el cruce no está en Modo
> Inteligente**, y ese modo se pone **desde el Maestro** (decisión de arriba). El mismo criterio
> gobierna el botón `DEMANDA` del propio Maestro: fuera del Modo Inteligente contesta
> `$ERR,CMD:DEMANDA,DESC:SOLO_EN_MODO_INTELIGENTE`.

> 🟠 **ESA FILA HA CAMBIADO DOS VECES Y HAY QUE LEER LAS DOS.** **04/09, medido en banco:** con la
> lectura antigua (`INPUT_PULLUP` + `== LOW`), las `R65`/`R66` (10 kΩ a masa) dejaban `PB9` y `PB13`
> en **0,6 V — BAJO permanente**, así que **nunca había flanco y ninguna secuencia se reconocía**, en
> ninguna de las dos puntas. **Ese mismo día, medido en el fuente:** las dos puntas pasan a `INPUT`
> pelado y `== HIGH` —**activo en ALTO**—, y el gesto de accionamiento pasa a ser **cerrar contra los
> 3,3 V del pin de al lado** (`J16` p5-p4 canal `A`, p8-p7 canal `B`). **El arreglo no se ha cargado
> ni ejercido en tarjeta, así que la fila no se pone en verde.** Ver §7.

### 🛑 La consecuencia, dicha en voz alta

**En el Esclavo, entrar o salir del Modo Degradado NO se puede hacer desde la app.** Sólo con el
mando de relés (`A·B·A·B` para entrar, `A·A·A` o `B·B·B` para salir). Y el **receptor RF de ese
mando NUNCA SE COMPRÓ** *(línea `A9` del Manual 15)*.

> 🛑 **Y AL 04/09 SIGUEN FALTANDO LAS DOS COSAS, aunque una haya dejado de estar abierta.**
>
> * **Qué receptor comprar YA NO es una decisión abierta: NORMALMENTE ABIERTO (`NO`).** Con el
>   firmware de hoy leyendo activo en ALTO, un `NO` deja el pin en 0 V en reposo —lo fijan
>   `R65`/`R66`— y produce el flanco de subida al cerrar. Un `NC` dejaría el pin en alto
>   permanente. Y con `NO`, un canal caído o un receptor sin alimentación quedan en reposo: **el
>   mando no manda nada en vez de mandar solo.** Sigue **sin comprarse**.
> * **Y el mando tampoco se ha visto entrar ni cableado** (`N-118`, §7): el firmware está corregido
>   pero **no ejercido en tarjeta**. Comprar el receptor no cierra por sí solo esta casilla; hace
>   falta la prueba de banco del mando con el firmware nuevo dentro.
>
> **Sumado a que el Esclavo no tiene `SET_MODO` por Bluetooth (fila 1 de esta tabla), HOY NO HAY
> NINGUNA FORMA DEMOSTRADA DE ORDENARLE UN MODO AL ESCLAVO ESTANDO DELANTE DE ÉL:** ni la app, ni el
> mando, ni los botones, ni la pantalla. Lo único que el Esclavo sigue aceptando de la app es lo que
> ya dice esta tabla —ámbar de emergencia, solicitar paso y `SET_RTC`—, que **no es cambiar de
> modo**.
>
> **Lo que sí sigue funcionando es el Esclavo obedeciendo al Maestro por radio**, que es su trabajo
> normal. Lo que ha desaparecido es **la vía local**: el técnico que está junto al poste 2 **no puede
> decidir nada allí**, tiene que ir al Maestro. Con el enlace de radio caído, tampoco desde allí.

> **Sin ese receptor, la única forma de entrar o salir del Degradado en el Esclavo es SUBIR AL
> GABINETE** — y ni así, porque allí arriba lo que había era el menú de la pantalla, que hoy **no
> tiene display y no puede confirmar nada**.
>
> **Eso deshace justo lo que se había ganado:** *«el técnico ya no tiene que subir con escalera a
> 5 metros en el Esclavo»*. Para el estado y las alarmas sigue siendo cierto; **para el Degradado,
> hoy no.**

### ✅ ~~🔴 DEFECTO ABIERTO~~ — el ámbar de emergencia de la app **YA** saca al Esclavo del Modo Degradado (`N-106`, cerrado el 04/09)

> 🛑 **LO QUE ESTE APARTADO DECÍA, conservado tachado porque manda a un operario a un poste y hay
> que poder ver qué se le decía antes:**
>
> ~~**MEDIDO POR LECTURA:** las dos vías de ámbar de emergencia **no hacen lo mismo**. Mando `B·B·B`
> ✅ sale por el todo-rojo; app `CMD:AMBAR_EMERGENCIA` 🛑 **no** —no pregunta por el Degradado ni
> llama a `degradado_salir()`—, así que el ámbar pedido desde el teléfono **puede caerse solo** y la
> app ya habrá mostrado un `RESULT:OK`.~~
>
> ~~✅ **Mientras esto siga abierto, en el ESCLAVO en Modo Degradado use el MANDO (`B·B·B`), no la
> app.**~~ → 🛑 **ESA INSTRUCCIÓN YA NO VALE, y mantenerla sería lo peligroso**: mandaba a usar la
> única vía que **no se puede accionar** —el receptor RF del mando nunca se compró y `N-118` sigue
> sin ejercerse—, en lugar de la que sí existe.

**Lo que hay HOY, MEDIDO en `Esclavo/src/bluetooth.cpp`** *(en el fuente; no ejercido en tarjeta)*:

| vía | ¿saca del Degradado? |
|---|---|
| **Mando, `B·B·B`** | ✅ sí, por el todo-rojo de despedida (`Esclavo/src/mando.cpp:129-141`) |
| **App, `CMD:AMBAR_EMERGENCIA`** | ✅ **sí** — `salidaDegradadoIniciada()` (`:302-308`), preguntada en las **dos** puertas: sin PIN (`:402`) y con PIN (`:481`) |

**Y el `RESULT:OK` ya no sale pase lo que pase: hay cinco respuestas y cada una dice algo distinto.**

| lo que contesta | qué significa para quien está delante |
|---|---|
| `RESULT:OK` | el ámbar **ya está encendido** |
| `RESULT:YA_EN_AMBAR_LATCH_PUESTO` | ya estaba en ámbar; **lo que esta orden ha añadido es la protección**, no la luz |
| `RESULT:SALIENDO_TODO_ROJO` | 🔴 **va en camino y TARDA de 10 a 90 s** — el todo-rojo de despedida. **No es un cuelgue** |
| `RESULT:SALIDA_YA_EN_CURSO` | el equipo ya se estaba rindiendo por el límite de 48 h; el ámbar llegará solo |
| `$ERR,…,DESC:SALIDA_A_ROJO_EN_CURSO_REPITA` | 🛑 **NO se ha armado nada.** Otro pidió una salida que acaba en ROJO. Mientras dura, el cruce está en todo-rojo —**más seguro que el ámbar que se pide**—. **Repita al terminar** |

> ⏱️ **La regla de campo que sale de esta tabla, y es la única que hay que recordar: `SALIENDO_TODO_ROJO`
> significa *va en camino*, NO *ya está*. No se vaya del cruce hasta ver el ámbar con los ojos.**
>
> 🔵 **El porqué de que tarde, que no es lentitud sino diseño:** saltar de un **verde por reloj**
> directo a ámbar intermitente le daría a quien ya venía lanzado una señal que invita a negociar el
> paso **mientras aún cree tener prioridad**. Por eso el Degradado entra y sale **siempre** por
> todo-rojo, y por eso cuesta lo mismo que el `B·B·B`.

> ⚠️ **Y lo que este cierre NO trae, para que no se lea como un permiso.** Es **MEDIDO sobre
> fichero**: **nadie lo ha ejercido**, ni en tarjeta ni en arnés. `CLAUDE.md` §8.bis pide **ver
> fallar el instrumento antes de fiarse del arreglo**, y para este camino eso **no se ha hecho
> todavía**.
>
> **Mientras tanto, la instrucción de obra es la que no depende de ningún firmware y ya estaba
> escrita: verificar las dos puntas con los ojos, al entrar y al salir.**

---

---

## 10. 🟢 LO QUE CAMBIÓ LA NOCHE DEL 04–05/09 — cuatro comportamientos que se ven desde la calle

> **Los cuatro salen de la sesión de banco**, y tres de ellos de **una cinta de tramas** grabada con
> el teléfono conectado al Maestro. **Ninguno estaba en el firmware que se llevó al banco.**

### 10.1 🔴 El ámbar del Poste 2 ya no deja al Poste 1 dando verde (`N-142`)

**Lo que pasaba, y es lo más grave de los cuatro:** si un operario ponía **ámbar de emergencia desde
la app en el Esclavo**, el Maestro **no se enteraba** — y encima el Esclavo **seguía contestando al
latido**, así que **el enlace le parecía perfecto**. Con el Maestro en VERDE, seguía dándolo **el
resto de esa fase, hasta 3 minutos con los tiempos de hoy**, con el otro lado en ámbar
intermitente: **los dos sentidos podían entrar al carril**.

**Lo que hace ahora:** el Esclavo **avisa por radio** al enganchar el ámbar, y el Maestro **se va a
modo ámbar en el acto**. Deja de ciclar, deja de dar verdes y **deja de preguntar**.

> ✅ **Y esto también deshace el bloqueo que se vio en banco** —*"si me conecto otra vez al Maestro,
> esto ya no me recibe nada"*—. La causa **no era el veto del ámbar**, como se pensó al principio: era
> que esa punta **no acusa recibo** mientras está en ámbar, así que el Maestro agotaba reintentos a
> ciegas y caía a fallo. Con el aviso ya no adivina.
>
> 🛑 **LOS DOS VETOS DEL ÁMBAR SE QUEDAN, y esto no se toca por comodidad:** mientras hay ámbar
> pedido —por el mando físico `B·B·B` **o** por la app— **una orden de radio no le quita el ámbar a
> esa punta**. Se probó a quitarlo y **el banco lo paró dos veces**: sin el veto, el ámbar que pidió
> el operario **se deshace solo a los ~3 s delante de él**.

### 10.2 🔴 El botón de ámbar podía quedar muerto sin decirlo (`N-146`)

**Lo que pasaba:** entre las **21:10 y las 21:13** del 04/09 el operario pulsó **seis veces** «poner
ámbar», el equipo contestó **`OK` las seis**, y el cruce siguió **en rojo durante 47 tramas
seguidas**.

**La causa:** el equipo entra en ámbar **sólo al cambiar de modo**. Si ya estaba en modo ámbar y
alguien le había dado **ROJO TOTAL** —que cambia la **luz** y no el **modo**, a propósito, porque el
rojo de emergencia entra sin PIN—, la orden de ámbar **no tenía nada que cambiar**: no hacía nada, y
decía que sí.

**Lo que hace ahora:** **re-arma el ámbar**, y contesta **`REARMADO`** en vez de `OK`.

> ⚠️ **Para quien lea el diario de órdenes: `REARMADO` es un ÉXITO, no un error.** Significa *"ya
> estaba en ámbar y lo he vuelto a encender"*. Son dos cosas distintas y por eso se dicen distinto.

### 10.3 🔴 En Modo Manual el cruce cambiaba solo (`N-147`)

**Lo que pasaba, reportado desde el banco:** *«el botón dar paso maestro queda en rojo, pasan 15 seg
y … pasa a ámbar intermitente»*.

| # | mitad del defecto |
|---|---|
| **1** | **DAR PASO se rechazaba** durante el plazo, y el cruce se quedaba en rojo |
| **2** | **al vencer el plazo, el cruce cambiaba SOLO**, sin que nadie pulsara |

**La causa:** el Modo Manual **entraba por la puerta del Modo Automático**, que deja un verde ya
programado. **Los 15 segundos son literales** — es el tiempo de despeje del cruce.

**Y un tercer defecto que nadie había reportado, salido al medir:** **cada pulsación de DAR PASO
reiniciaba el plazo**. Quien pulsara cada 10 segundos **no veía el verde nunca**, y cada pulsación
contestaba `OK`. **Obedecer y no avanzar no deja rastro de avería.**

**Lo que hace ahora:** en Manual el equipo **se pone en todo-rojo y se queda quieto**. *«El operador
en manual no debería llevar un ciclo, sino que, como está ahí parado viéndolo, que se cambie de
inmediato»* — decisión del responsable.

> 🛑 **LO QUE NO SE HA DEBILITADO, Y HAY QUE SABERLO ANTES DE PROBARLO: el despeje sigue entero
> cuando hay algo que despejar.** Un cambio **de verde a verde** —el caso normal del cruce— sigue
> pasando por **su rojo y su despeje completos** (`SFTY-4`). Lo único que se dejó de cobrar es un
> despeje **ya pagado**: en Manual el cruce lleva minutos en rojo mientras el operario mira, y
> hacerle esperar 15 s más no vacía nada que no estuviera vacío.

### 10.4 🟢 Desde el Maestro se ve el semáforo del Esclavo (`N-149`)

**Pedido en banco:** *«cuando me conecto al maestro no me aparecen los estados del semáforo del
esclavo… yo necesito que maestro me traiga los datos del esclavo»*. Y sobre la alternativa de
conectarse al otro poste: *«tendrías que caminar 1000 metros hasta el otro lado»*.

La telemetría del **Maestro** lleva ahora un campo con el estado del **Esclavo**:

| valor | qué significa |
|---|---|
| **ROJO** / **VERDE** | lo que el Esclavo **confirmó** — no lo que el Maestro le ordenó |
| **ÁMBAR** | el cruce está en modo ámbar |
| **?** | 🔴 **el enlace está caído y esta punta no sabe de qué color está la otra.** **NO** significa «sin medida» |

> 🔵 **Por qué se publica lo confirmado y no lo ordenado:** una orden puede perderse en el aire. Un
> tablero que pinte el color que el Maestro **quiso** le enseña al operario un semáforo que quizá no
> existe. Este equipo ya pagó eso dos veces —los `12,6 V` de batería que eran un número escrito a
> mano, y el equipo declarándose «en hora» con el reloj parado en ceros—.
>
> ⚠️ **El campo sólo lo emite el MAESTRO.** El Esclavo no lo emite porque **no tiene de dónde
> sacarlo**: no le pregunta al Maestro y no tiene por qué. Un campo que sólo pudiera valer `?` no
> informaría nunca.

### 10.5 🕐 La hora: de dónde sale ahora, y por qué puede seguir saliendo en blanco (`N-145`)

El campo de **hora** de la telemetría lo compone el **STM32**, que es justo el micro **sin reloj**
—su cristal `Y2` está confirmado muerto—. El único reloj del equipo (`DS3231`) **cuelga del módulo
`ESP32`**. Desde ahora **el módulo rellena ese hueco al pasar la trama** y recalcula su checksum.

> 🛑 **AVISO, Y ES LA PARTE QUE HAY QUE LEER ANTES DE PROBARLO:**
>
> **SIN UN `DS3231` CONECTADO, LA HORA SEGUIRÁ SALIENDO EN BLANCO (`--:--:--`).** Eso **no es un
> fallo**: es el módulo negándose a inventar una hora que no tiene. Un `DS3231` sin pila entrega una
> fecha **perfectamente formada y falsa**, y este equipo prefiere el hueco.
>
> | | estado |
> |---|---|
> | El módulo `DS3231` | 🛑 **NO ESTÁ COMPRADO** (línea `A6` de la lista de compras) |
> | Su dirección en el bus | 🔴 **SIN VERIFICAR sobre el módulo real** |
> | Esta parte | 🔴 **NO SE PUEDE DAR POR PROBADA.** Nada de esto ha tocado un `DS3231` real |

### 10.6 🔴 La batería NO se mide, y el equipo lo dice

En **todas** las tramas de la cinta del 04/09 el campo de batería sale **`--`**. **Eso es
deliberado**: el equipo **no tiene ni divisor de tensión ni entrada analógica** para medirla, y antes
publicaba un `12.6` fijo escrito a mano que hacía que nadie preguntara por la alimentación.

> **Un campo marcado `--` es lo correcto mientras no se mida.** Si hace falta la batería en campo,
> **eso es una pieza que comprar y firmware que escribir** — no un número que rellenar.

### 10.7 📱 Lo que cambió en la APP la misma noche, y aquí sólo se apunta

**No se detalla en este manual** —vive en `14_Manual_App_Movil_IOT_VIAL.md`, que lo lleva otra
mano—, pero un operario lo va a ver y conviene que no le sorprenda:

| | |
|---|---|
| 🆕 **Al pedir ÁMBAR, la app pregunta si ha mirado el corredor** (`N-148`) | Pedido en banco: *«cuando le das en manual en ámbar, debería salirte un aviso diciendo: ¿está usted seguro que no hay vehículos en el corredor?»*. **Y el motivo por el que antes no preguntaba era falso**: se creía que el ámbar «es la dirección segura», y **medido en el `.cpp` no lo es** — el ámbar deja intermitente **en las DOS puntas**, o sea que en un carril único **se entra por los dos lados a la vez**. Es la orden que **más abre paso** de la botonera |
| 🆕 **El estado del Poste 2 se enciende en la consola del Poste 1** (`N-149`, ver §10.4) | y sus cuatro salidas **no se colapsan**: el color se pinta; el `?` sale como *«SIN DATO DEL POSTE 2»* **con la causa al lado**; un equipo con firmware anterior sale como *«este equipo no lo publica»*; y un valor que no se reconoce sale **en crudo, sin adivinar** |
| 🟢 **La hora del equipo se pinta** — antes la app la recibía y la tiraba | 🔴 **y seguirá saliendo en blanco mientras no haya `DS3231`**, ver §10.5 |

---

> ## 🛑 ESTE MANUAL NO ES UN PERMISO
>
> **En la calle corre la V8.4** (commit `e303485`). Todo lo que este documento describe como *«hoy»*
> —las cámaras de `J16`, el `ESP32` en `J17`, el mando conservado en A y B, los botones sin sujeto,
> los 25 s de SFTY-6— **está en el árbol de trabajo y NO HA PASADO PRUEBA DE BANCO**.
>
> Que la compuerta de verificación salga en verde significa que los modelos y los arneses de PC no
> encuentran nada. **No dice que el firmware funcione sobre la tarjeta.** Las actas están en
> `evidencia/`, con su fecha y el hash del árbol que midieron.
>
> 🔴 **Y el banco del 3–4/09 tampoco es ese permiso.** Sobre 29 pasos previstos, **la cabecera del
> informe** dice **24 ejercidos · 4 bloqueados por el enlace Bluetooth** —el equipo no llegó a
> recibir órdenes— **· 1 abortado por un incidente de seguridad**. **Se cita como la cifra de la
> cabecera y no como un hecho: la enumeración del propio informe no cuadra con ella**, y la
> discrepancia está desglosada en `12_Cobertura_de_Pruebas_y_Huecos.md`, que por eso **no publica
> ningún total**. Aquí tampoco se publica uno reconciliado. Lo que sí vale sin depender de la
> cuenta: **un paso bloqueado no dice nada del firmware** — no es un aprobado ni un suspenso.
>
> **La consecuencia concreta:** como el equipo nunca llegó a operar, **la regresión del Modo
> Automático (`N-42`) no se confirmó ni se descartó.** El defecto por el que se fue a banco sigue
> exactamente igual de abierto que antes de ir.
>
> **Nada de esto sube a campo sin pasar banco.** No es una preferencia de proceso: un semáforo que
> falla mal mata a alguien.

