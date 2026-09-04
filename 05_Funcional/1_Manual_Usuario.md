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
> | `botonAceptar()` | `Maestro/src/botones.cpp:280` · igual en el Esclavo | **devuelve `false` SIEMPRE** |
> | `botonCancelar()` | `Maestro/src/botones.cpp:281` · igual en el Esclavo | **devuelve `false` SIEMPRE** |
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

### Tiempos de Despeje (All-Red / Rojo Estático)
Cuando se solicita el cambio de vía, el sistema debe entrar en un estado de **ROJO ABSOLUTO**.
- Durante *N* segundos, **ambos semáforos estarán en ROJO**.
- **Variabilidad de Terreno:** Como la obra puede abarcar de 20m a 500m (con radios que alcanzan hasta 6km en línea vista), el tiempo de despeje no puede estar limitado a un valor bajo.
- **Configuración:** ~~La interfaz del menú LCD permite configurar tiempos de despeje de 5 a 999 segundos (piso mínimo de 5s por seguridad vial, hasta 16.6 minutos) para dar cobertura total a puentes largos o túneles de 500m.~~
  > 🔴 **CORREGIDO EL 28/08 — este párrafo daba un rango que la app NO puede pedir.** Sin pantalla,
  > los tiempos se fijan con `CMD:PIN:...:SET_TIEMPOS:<verdeMin>,<rojoMin>,<despejeSeg>`, y los
  > límites duros los impone el firmware, no la interfaz. **MEDIDO en
  > `Maestro/src/modo_automatico.cpp` líneas 32–34** *(re-verificado el 31/08; la revisión anterior
  > citaba «30–32», que es donde estaba el comentario, no las constantes)*:
  >
  > | | constante | mínimo | máximo |
  > |---|---|---|---|
  > | Verde | `VERDE_MIN_MIN` / `_MAX` (`:32`) | 1 min | 15 min |
  > | Rojo | `ROJO_MIN_MIN` / `_MAX` (`:33`) | 1 min | 15 min |
  > | **Despeje (todo-rojo)** | **`DESPEJE_SEG_MIN` / `_MAX` (`:34`)** | **10 s** | **90 s** |
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
* **Conexión Hardware:** Salida de alarma de relé de la cámara (`1A`/`1B`) al pin **`PB0` (Demanda)**
  con masa `GND`, por la bornera **`J14`**.

> 🔵 **ACTUALIZADO EL 31/08 — hoy hay TRES entradas de cámara por poste en el firmware, no una.**
> **MEDIDO** en `Maestro/include/pines.h:46, 124-125` y `Maestro/src/botones.cpp:156-157`
> *(idéntico en el Esclavo)*:
>
> | entrada | pin | bornera | cómo se lee | estado |
> |---|---|---|---|---|
> | `CAM_DEMANDA_PIN` | `PB0` | **`J14`** | `INPUT` pelado, **activo en ALTO**, con antirrebote en la placa (`R64` 10 kΩ + `C25` 100 nF) | ✅ **En servicio. Es el único camino de cámara con firmware ya probado** |
> | **`CAM_C_PIN`** | **`PB14`** | **`J16` p10** | `INPUT` pelado, **activo en ALTO** — la misma lectura antirrebotada | 🟠 **Firmware hecho (N-97). SIN CABLEAR** |
> | **`CAM_D_PIN`** | **`PB15`** | **`J16` p12** | igual | 🟠 **Firmware hecho (N-97). SIN CABLEAR** |
>
> **Las tres piden paso por la misma puerta** —`demanda_solicitar()`—, que es donde está escrita la
> diferencia entre **pedir** y **decidir** (SFTY-27). **Una cámara no enciende nada**: sólo pide.
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

**2. En la lista de Bluetooth el equipo se llama `SEM-SIN-MATRICULA`, no como el cruce.**

> 🛑 **Mientras el módulo no haya aprendido la serie del equipo se anuncia como `SEM-SIN-MATRICULA`.**
> Si el técnico busca **`IOT_VIAL`** o **el nombre del cruce**, **no lo encuentra** y concluye que el
> módulo está muerto.
>
> **Y no se renombra en caliente:** aunque el módulo aprenda la serie durante la sesión, **el nombre
> bueno no aparece hasta el arranque siguiente**. Ver un `SEM-SIN-MATRICULA` no significa que la
> serie no haya entrado; significa que el módulo no se ha reiniciado desde que entró.

**3. El emparejamiento NO pide PIN, y el `1234` no es el PIN de emparejamiento.**

> ⚠️ **El `ESP32` empareja por «Just Works»: SIN PIN del sistema operativo.** Android enlaza sin
> preguntar nada. **Si el teléfono le pide un PIN de emparejamiento, no está hablando con este
> módulo.**
>
> 🔴 **El `1234` es un PIN DE COMANDO DENTRO DE LA APP** —el que va en la trama
> `CMD:PIN:1234:…`—, **no el PIN de emparejamiento de Bluetooth.** Se confundieron en banco: se
> tecleó `1234` en el diálogo del sistema operativo esperando que enlazara. Son dos cosas distintas
> y sólo una existe.

**4. Ese PIN de la app NO CADUCA. Riesgo conocido y ABIERTO (`AB-9`).**

> 🛑 **Se teclea una vez y el teléfono queda autorizado hasta que se cierra la app.** No hay tiempo
> de expiración, ni bloqueo por inactividad, ni recuento de órdenes: **una sesión desbloqueada lo
> sigue estando dentro del bolsillo**.
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

* **Emisión periódica de `$STATUS,...` cada 1 segundo.** Formato real, leído del firmware:
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

**MEDIDO el 31/08** sobre `Maestro/src/bluetooth.cpp` y `Esclavo/src/bluetooth.cpp`:

| | **MAESTRO** | **ESCLAVO** |
|---|---|---|
| Cambiar de modo desde la app | ✅ **Sí.** `SET_MODO:AUTO` · `MANUAL` · `AMBAR` · `MENU` · `ALCANCE` · `INTELIGENTE` · `DEGRADADO` (`:177+`) | 🛑 **NO. No existe ni un solo `SET_MODO`** — `grep -n "SET_MODO" Esclavo/src/bluetooth.cpp` → **CERO coincidencias** |
| Ámbar de emergencia desde la app | ✅ sí | ✅ sí — `CMD:AMBAR_EMERGENCIA` |
| Pedir paso | ✅ `DEMANDA` | ✅ `SOLICITAR_PASO` *(se lo pide al Maestro)* |
| Poner los tiempos | ✅ `SET_TIEMPOS` | ❌ no — los tiempos los lleva el Maestro |
| `FORZAR_ROJO` | ✅ **sí, y para de verdad** | 🛑 **NO.** Contesta `$ERR,CMD:FORZAR_ROJO,DESC:RENOMBRADO_USE_AMBAR_EMERGENCIA` (`:158`, `:182`) y **no para nada** |
| Menú local / botones | ❌ sin sujeto | ❌ sin sujeto |
| **Mando de relés (A y B)** | 🟠 **No se pudo accionar en banco (`N-118`). Firmware corregido, SIN ejercer en tarjeta** | 🟠 **Igual — y aquí es además la única vía de mando de la punta** |

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

### 🔴 DEFECTO ABIERTO — el ámbar de emergencia de la app NO saca al Esclavo del Modo Degradado

**MEDIDO POR LECTURA** *(nadie lo ha ejercido todavía en banco ni en arnés)*: las dos vías de ámbar
de emergencia **no hacen lo mismo**, y el firmware afirma por escrito que sí.

| vía | ¿saca del Degradado? |
|---|---|
| **Mando, `B·B·B`** | ✅ **sí**, por el todo-rojo de despedida (`Esclavo/src/mando.cpp:138`) |
| **App, `CMD:AMBAR_EMERGENCIA`** | 🛑 **no.** No pregunta por el Degradado ni llama a `degradado_salir()` |

> 🛑 **Lo que eso significa para quien está delante del cruce:** en Modo Degradado, el ámbar de
> emergencia pedido desde el teléfono **puede caerse solo** —el sostenedor del Degradado sigue
> escribiendo luz en su siguiente vuelta— **y encima la app ya habrá mostrado un `RESULT:OK`**.
> Es decir: **el botón de pánico de la app puede no parar el cruce exactamente en el modo donde más
> falta hace**, y decir que sí lo hizo.
>
> ✅ **Mientras esto siga abierto, en el ESCLAVO en Modo Degradado use el MANDO (`B·B·B`), no la
> app.** Y si no hay mando, se sube.
>
> 🟠 **PERO ESA SALIDA NO ESTÁ DEMOSTRADA HOY (`N-118`, §7): en el banco del 3–4/09 el `B·B·B` no se
> pudo accionar**, y subir al gabinete tampoco servía, porque allí arriba era el mismo pin. **El
> firmware ya lo lee activo en ALTO y la salida vuelve en cuanto se cargue y se ejerza** — pero
> hasta esa prueba **hay que contar con que en el Esclavo en Modo Degradado no hay ninguna vía
> comprobada de ámbar de emergencia**: la de la app puede caerse sola y decir que no. **Mientras
> tanto `N-106` es un defecto sin rodeo, no un defecto con rodeo.**
>
> ⚠️ **Está anotado como `N-106` y NO está arreglado.** El arreglo espera decisiones del
> responsable —qué *debe* hacer el ámbar de la app en Degradado: salir ordenado como `B·B·B`, o
> quedarse— porque **es lo que ve un conductor**. Y antes de tocar una línea de firmware hay que
> ver fallar el arnés que lo mide: uno que naciera en verde no mediría nada.

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

