# ESTADO — dónde está parado el trabajo HOY (12/09/2026)

> ## ▶️ PUNTO DE CONTINUACIÓN — 12/09/2026
>
> 🔴 **LO PRIMERO, y no cambia por nada de lo de abajo: NADA DE LO DE HOY HA VISTO UNA TARJETA.**
> Son once horas de PC. Lo que decide si esto funciona es la sesión de banco (`roadmap.md` §0,
> grupo 3) y **la cinta del Esclavo del Sisga, que sigue sin traerse**.
>
> **La compuerta pasa de DOS rojos a UNO**, y el que queda **no es un defecto de firmware**: es
> `decisiones_01_anclas` contando las cinco decisiones tomadas y sin construir (`D-14`, `D-22`,
> `D-23`, `D-25`, `D-27`). De ésas, `D-23` es teclado, `D-22` necesita una tarjeta delante, y
> `D-14`/`D-25`/`D-27` puede que **no necesiten ni una línea de código** y el instrumento les
> exige ancla igual — eso es un defecto del instrumento, y está en la lista.
>
> **Lo que entró hoy en firmware, todo `SIN_BANCO`:** `N-142` —el ámbar de emergencia sin PIN
> **avisa al Maestro**, y el `$ACK` dice si el aviso pudo oírse— · `D-28` —el plazo de la hora a
> **400 s**, y de paso se descubrió que su derivación era una **tautología** que habría dado
> margen cero— · `D-29` —**`N-20` resucitado**: un corte de luz ya no mata la reanudación del
> Degradado— · `N-163` —**`G3` cerrado**: la ventana de verde-contra-ámbar, de 250 ms a **cero**,
> sin tocar el umbral de 25 s—. **Maestro al 89,8 %, quedan 6.656 B.**
>
> **Y el repositorio:** de **1.081 a 549 ficheros versionados** —`99_Legacy/` sale de git con el
> firmware monolítico dentro—, `OPTIMIZACIONES.md` de **2.310 a 954 líneas** con las 29 reglas
> contadas antes y después, y **18 filas nuevas en §0** del roadmap: pendientes que sólo vivían
> en el porqué y que una mudanza al histórico habría enterrado.
>
> **LO QUE ESPERA AL RESPONSABLE — y el 12/09 por la tarde se quedó en NADA.** Las cuatro que había
> se cerraron con él delante, y se dice aquí **para que nadie las vuelva a preguntar**:
> ~~microSD~~ *(tienen SD y graban; lo que falta no es decidir, es **el paso que falta en la guía**,
> y eso es teclado — fila 1.29)* · ~~tres siembras perdidas~~ *(pregunta mal planteada por mí: la hora
> la mandan los ESP32, y lo que se decidía era sólo cuánto aguanta el STM32 con su reloj interno.
> **Se queda en una**, que es lo seguro)* · ~~`A-8`~~ *(**cerrada**: la guía ya dice 24×7 en la regla y
> en la salida, en tres sitios)* · ~~el acuse del aviso de ámbar~~ *(**`D-31`, decidida Y
> ESPECIFICADA entera** — el primer diseño se descartó porque construía algo **peor** que el defecto,
> y las tres correcciones salieron de sus preguntas: acuse sólo del primero, el cancelar en el mismo
> lote, y **un** reintento porque a 8 km lo que sube es la pérdida y el presupuesto de radio sólo deja
> ~4 s)*. **Lo que queda es todo teclado**, en `roadmap.md` §0 grupo (1). Lo único que NO destraba
> nadie escribiendo está en el grupo (3): la sesión de banco y **la cinta del Esclavo del Sisga**.
>
> 1. **En `main` (`68dd2c5`): `D-26`, la hora la manda el ESP32 de cada poste.** El ESP32 siembra a su STM32
>    desde el `DS3231` (al arrancar, tras cada `SET_RTC` bueno y cada ~~300 s~~ **120 s desde el 12/09**, `CMD:HORA_ESP32`); el `SET_RTC` del
>    teléfono lo atiende el puente y ya no cruza; la siembra ya no escribe el RTC del STM32; el Esclavo toma la hora
>    del Maestro con radio y la de su ESP32 sin radio 25 s; en Degradado un salto > 29 s pasa por rojo; y
>    `$ALARM …EVENTO:HORA_ESP32…` (`J17_MUDO`, `SIN_HORA_DEL_ESP32`, `RECHAZADA_FORMATO`). Revisado por el diff por el
>    arquitecto antes de fusionar (**`roadmap.md` §3.16, `N-162`, `H1`..`H9`**). ⚠️ **Sin banco y sin tarjeta.** La
>    foto `wip/d26-hora-esp32-foto-1315` quedó integrada en `a0313fe`: ya no se usa. Los dos rojos de la compuerta
>    siguen siendo reales —`decisiones_01_anclas` y ~~**G3**~~ *(**G3 cerrado el 12/09**, `N-163`: queda UN solo rojo)*—; las cifras, en la última acta.
> 2. ~~🟠 **EN CONSTRUCCIÓN (worktree, NO está en `main`)**~~ 🟢 **CONSTRUIDA y en `main` el 11/09 por la noche, sin banco: `D-21` pieza (1)** —ámbar intermitente en la punta cuya
>    hora caducó, dentro del Degradado—. Es la cura de **`H1`**: con el `J17` del Maestro mudo, al caer la radio el
>    Esclavo adopta su `DS3231` y el Maestro sigue con la hora derivada del HSI → **verde contra verde**. **Bloquea
>    campo.** ~~Se integra por el diff~~ Integrada por el diff (`roadmap.md` §0, fila 1.13); el plazo es ~~320 s~~ **280 s desde el 12/09** —lo bajó la cadencia de ~2 min, y de paso se descubrió que la derivación vieja era una **tautología**: se derivaba de la propia cadencia, así que a 120 s habría dado plazo = cadencia, **margen cero**. Ahora se deriva del **relevo** (`2C + deriva + SFTY6` = 271 s), con dos `static_assert` que lo sujetan— y la app lo
>    enseña (`HORA_ESP32,CAUSA:CADUCADA`). La punta en ámbar **no vuelve sola** (`D-21`).
> 3. **Espera al responsable:** ~~🆕 **la cadencia de la siembra, ~5 min → ~2 min**~~ *(decidida por el responsable
>    el 11/09 por la noche y **CONSTRUIDA el 12/09 en `dca17cd`**: `SIEMBRA_INTERVALO_MS` = 120000 y el plazo
>    rederivado. **Lo que sí queda para el responsable son dos cosas nuevas que salieron al construirla:** «~2 min»
>    no está fijado al segundo en `DECISIONES.md` —los 120 s salen de la medida de la fila 2.10—, y a esa cadencia
>    **tolerar DOS siembras perdidas cabría**, a cambio de bajar el margen entre los dos `DS3231` de 13 s a 7 s)* ·
>    ~~**G3/SFTY-6** (la punta en verde tarda 250 ms en soltar frente a un `S_FALLO`)~~ *(**DECIDIDO y
>    CONSTRUIDO el 12/09**, `N-163`: la ventana pasa de 250 ms a CERO **sin tocar el umbral de 25 s** —lo
>    sujeta un `static_assert` que demuestra que no se recorta el presupuesto de reintentos—, y el criterio
>    del responsable se midió caso por caso sobre 32 cortes: los ámbares no subieron ni uno)*; la
>    referencia del **relé** de `J15`; la **alarma por discrepancia de los dos `DS3231`** —con el matiz de `H3`: el
>    umbral **no puede ser 11 s a secas**, porque el HSI de cada punta mete entre siembras ~~hasta 7,5 s~~ **hasta
>    3 s desde el 12/09** (la cifra la manda la cadencia: `roadmap.md` fila 2.8, y el umbral sube de 11 a 13 s)—;
>    **tachar «sin filtro de objetivo» en `D-13`** (lo derogó `D-27`); y **los `.zip` del 10/09** —cinco en la raíz, y
>    tres (`44967db`, `bd77271`, `fae4b3e`) llevan `Camaras_Sisga_4x.html` de ese día, con las afirmaciones de
>    seguridad retiradas—. Y el texto de los «MESES» de `D-21`/`D-23`, que la medida tumbó (fila 2.7). El resto,
>    `roadmap.md` §0, grupo (2).
> 4. **Siguiente trabajo, ya decidido:** ~~los instrumentos de `D-26` (fila 1.14: ningún arnés compila el reloj del
>    Esclavo)~~ *(hecho el 11/09 por la noche: el Degradado a dos puntas compila el `reloj.cpp` real, 53/53)*; ~~`AMBAR_EMERGENCIA` sin PIN que avise al Maestro (§3.16-A)~~ *(**hecho el 12/09 en `913c29c`**, con los tres instrumentos que lo dejaban pasar arreglados —el de dos puntas ejercía una **transcripción** de la puerta buena escrita en su propio adaptador—; `roadmap.md` §0 fila 1.2. **Sin banco y sin tarjeta**, y **no confirma la causa del DAR PASO del Sisga**: eso lo diría la cinta del Esclavo. Queda viva la vía de `H4`: si muere sólo el transmisor del Esclavo, el aviso no sale, esa punta no puede saberlo y el `$ACK` sale igual que con la radio sana)*; la **APK** recompilada desde `main` —y su
>    texto de `SET_RTC|OK`, que en `68dd2c5` dice *«no hay nada que los sincronice entre sí»*, caducado por `D-20`/`D-26`
>    ~~(al escribir esto hay cambios sin comitear en `app.js` que lo tocan: se integran por el diff)~~ *(integrados en `fd595ea`)*—;
>    portar a los modelos Python la autorrecuperación nueva; los **`.docx`** cuando se cierren las indefiniciones.
> 5. **Lo cerrado el 11/09 NO se reabre desde un documento** (`roadmap.md` §0, recuadro 🔒; `CLAUDE.md` §11.1).

> **Este fichero es el estado VIVO.** Lo que está abierto, lo que bloquea y lo que falta medir.
> El *porqué* completo de cada `N-x` vive en [`roadmap.md`](roadmap.md); las decisiones vigentes,
> en [`DECISIONES.md`](DECISIONES.md) — **si un párrafo de aquí contradice una fila de allí, gana
> la fila**. Lo de debajo del separador es histórico y se conserva tachado, no borrado.

**Rama de trabajo: ~~`main`~~ `main` EN GITHUB (`origin/main`), que en ESTE clon es la rama local `main-nuevo`** *(medido el 11/09: `git rev-parse --abbrev-ref main-nuevo@{upstream}` → `origin/main`)*. ⚠️ **La rama local `main` de este clon es OTRA y vieja** —sigue a `padre/main`—: un `git checkout main` aquí no lleva al trabajo vigente. *(El 11/09 se fusionaron, por avance rápido y tras validarlas por el
diff, `feat/d20-d23-construccion` y `fix/campo-sisga-rtc-darpaso`, que ya la contenía).* 🔴 **Aquí había un hash de HEAD escrito a mano y estaba
caducado: se ha retirado en vez de sustituirse por otro que caducaría igual.** El HEAD vigente se
mide, no se lee: `git rev-parse --short HEAD`.

**La instalación certificada es la V8.4 (`e303485`, 31/07).** 🔴 **Pero el 10/09 un Maestro
(`SERIE:179DB0`) corrió en El Sisga con firmware V9 `SIN_BANCO`**: cargado el paquete del 08/09
(`7ff7d12`), probado después el del 10/09 (`b354fe9`). La cinta y el diario del Maestro están en
`evidencia/`. Lo primero: 🔴 **no usar el Modo Degradado allí con `7ff7d12`** (el Maestro se declara
en hora con el reloj parado); **avisar al instalador** de que la guía de 4 cámaras que le llegó por
WhatsApp está RETIRADA —ninguna cámara protege la pluma— *(11/09: sus CONEXIONES —cuatro cámaras,
dos por poste, `J16` p9/p10 y p11/p12, talanquera en `J15`— las dejó el responsable como
definitivas en `D-25`; lo que se retira son sus afirmaciones de seguridad, que siguen siendo
falsas)*; **medir la alimentación del ESP32** (~~se reinició 5 veces en 97 s~~ — **al menos 3
reinicios** en 12:18–12:19: la cinta trae 5 partes `EVT:ARRANQUE`, pero el parte se emite una vez
por CONEXIÓN Bluetooth, así que un arranque puede anunciarse dos veces; 2 de los 5 son
`SUBIDA_DE_TENSION`, `roadmap.md` §3.16); y **traer la cinta del Esclavo**. ~~Lo que hay
que construir es que **el ESP32 mande la hora**~~ → 🟢 **construido y en `main` desde `68dd2c5`** (`D-20`/`A-15`, con las reglas de **`D-26`**: siembra ~~cada ~5 min~~ **cada ~2 min desde el 12/09 (`dca17cd`)**, no cada hora) — **sin banco, y con `H1` abierto, que bloquea campo**. Todo en `roadmap.md` §3.16 (`N-162`).
🎯 **11/09, `D-27` — CERRADO por el responsable y alineado en los documentos (sin los `.docx`):**
**las cuatro cámaras están compradas**; **`J14` queda libre y sin cablear** (el fin de carrera no se
instala — se cierra el conflicto con `A-2`); **la configuración de cada cámara es la del manual del
modelo** (`04_Manuales/MANUAL_CONFIGURACION_CAMARAS_IA.md` §4; tabla valor a valor en el Manual 9
§4 Paso 3). Lo que dejó a la vista —la zona, el filtro y el contador de la fase 1, y **la guía del
Sisga, que en su paso 07 sigue diciendo «no filtrar»**— está en `roadmap.md` §0, filas 2.1.bis y
2.1.ter. **`D-25`, `D-26` y `D-27` no se reabren desde un documento** (recuadro 🔒 de `roadmap.md` §0). La compuerta en
verde dice que los modelos y los arneses de PC no encuentran nada; **no dice que el firmware funcione
sobre la tarjeta**.

> 🔴 **Y esta rama no es un retoque: TODO EL RELOJ DE LAS DOS PUNTAS SE REESCRIBIÓ.** El diff contra
> el arranque de la rama son **cientos de líneas** en `Maestro/src/reloj.cpp` y
> `Esclavo/src/reloj.cpp` —los dos ficheros más movidos de la tanda—, y de ahí cuelga
> `reloj_enHora()`, que es **la autorización del Modo Degradado: el único modo que da verde SIN
> confirmar la otra punta.** Antes valía por el RTC sobre `Y2`; ahora vale por una base de software
> sembrada desde fuera. **Es exactamente la clase de cambio que no se juzga en el PC.**

> 🔴 **DEPENDENCIA DE ESTA MÁQUINA QUE NO ESTÁ EN EL REPOSITORIO: `D:\toolchain\mingw64`.**
> Es la copia del `gcc` de host **fuera de la ruta con `ñ`** de `Diego.Zuñiga`, cuyo `ld` no abre
> `crt2.o` aunque el fichero exista (N-44). **Si esa carpeta desaparece, los siete arneses que
> compilan C++ real caen a `ABORTADO` a la vez** — y la compuerta lo dice en una línea que nadie
> lee cuando el resumen de arriba parece normal. Ya pasó el 05/09: `13 PASS · 7 ABORTADO`, y no se
> enteró nadie.
>
> **Por eso un `ABORTADO` se lee SIEMPRE, y un número de `PASS` sin su total al lado no significa
> nada.** Esto vive aquí y no en `CLAUDE.md` porque **es estado de una máquina, no una regla**:
> quien clone el repositorio en otro equipo no tiene este problema, tiene el suyo.

> 🔴 **SEGUNDA DEPENDENCIA DE ESTA MÁQUINA, anotada el 07/09 porque compilar la APK obligó a
> encontrarla y no estaba escrita en ningún sitio: NO HAY `java` EN EL `PATH`.** `gradlew` no
> arranca sin un JDK, y el que hay vive **fuera del proyecto y con un nombre que nadie adivina**:
>
> ```
> JAVA_HOME = D:\@Proyect\Baliza\7 sw apk\java-21-openjdk-21.0.4.0.7-1.win.jdk.x86_64
> sdk.dir   = C:/android-sdk        (lo dice android/local.properties, y existe)
> ```
>
> **Es el mismo modo de fallo que `N-44` una capa arriba:** la herramienta existe, no está donde se
> la busca, y `command not found` se lee como *«no se puede compilar»* cuando lo que falta es la
> ruta. **Un `--version` que no responde no prueba que la herramienta no esté** (`CLAUDE.md` §7):
> aquí hicieron falta dos búsquedas en disco para dar con ella. Si `compilar_apk.bat` falla en otra
> máquina, esto es lo primero que hay que mirar.

---

## ✅ Lo que está CONFIRMADO EN COBRE

~~**La última cinta es del 05/09 a las 22:19, y cargó `42a52cd`.**~~ 🔴 **CADUCO desde el 10/09: la
última cinta es la del Sisga** —`evidencia/2026-09-10_Sisga_179DB0_cinta_tramas.txt` y
`…_diario_ordenes.txt`, **sólo del Maestro**, con `7ff7d12` dentro— y lo que ejerce está en
`roadmap.md` §3.16 y §5. **Esta tabla sigue siendo la de la cinta del 05/09** (`42a52cd`), que lo
demuestra, no lo afirma:

| | evidencia en la cinta |
|---|---|
| **N-42** el Modo Automático mueve las luces | cerrado en la sesión de banco del 04/09 por la noche, con el responsable delante: *«ahí cambia ese amarillo y este a verde. Ahora está funcionando»*. Arreglo en `ceb8cc5` |
| **N-146** el ámbar re-armado | `$ACK,CMD:SET_MODO:AMBAR,RESULT:REARMADO` a las 22:19:40, y el `ESTADO` pasa de `ROJO` a `FALLO COM` |
| **N-149** el Esclavo en la trama | `ESC:AMBAR` y `ESC:ROJO` viajando en todos los `$STATUS` |
| **N-145** la hora del DS3231 | `HORA:22:19:58` — el campo dejó de ser `--:--:--` |
| **N-142** el aviso del ámbar | el `ESC:` sigue al ámbar del Esclavo sin esperar los 25 s |

## 🔴 Lo que se ARREGLÓ DESPUÉS de esa cinta y NO ha pasado por una tarjeta

| | commit | lo que enseñó de paso |
|---|---|---|
| **N-151** `DAR PASO` en un modo sin coordinador trababa el cruce para siempre | `273b315` | el equipo decía que SÍ a una orden que no iba a ejecutar, y se quedaba PEOR que antes de pedirla. Lo vigila `maestro_12_dar_paso_sin_coordinador` |
| **N-152** `CANCELAR_AMBAR` no avisaba al Maestro | `d6ce67e` | **en `MODO_AMBAR` el Maestro estaba SORDO**: un comando copiado de N-142 habría entrado sin lector |
| **N-150** el ciclo no arrancaba tras aplicar tiempos | `414b962` | `ACK_TEXTO` no tenía `SET_TIEMPOS|OK`: el genérico lo pintaba **en verde** |
| **los parsers de la app** | `414b962` | eran **TRES**, y `parseError()` leía por posición con una prueba de un formato que ningún micro emite |
| **la línea falsa de `pines.h`** | — | decía que `BOTON1`/`BOTON2` son `INPUT_PULLUP` activos en BAJO. El fuente hace `INPUT` pelado y lee `== HIGH`. Es la cabecera que todo el mundo lee antes de cablear |

> **De ninguno hay una sola prueba en cobre. Que la compuerta esté en `20/20` no dice nada de
> eso.** ⚠️ *11/09: los tres primeros iban dentro de `7ff7d12`, el firmware del Maestro del Sisga
> (`git merge-base --is-ancestor`), y la cinta del 10/09 ejerce **del lado del Maestro y por
> telemetría** `N-150` (el ciclo arranca tras `SET_TIEMPOS:3,3,15`). Ni `N-151` ni `N-152` salen en
> esa cinta —no hay un `DAR PASO` fuera de coordinador ni un `CANCELAR_AMBAR` del Poste 2—, y nada
> de ella ve las luces ni el Esclavo: no se tacha nada hasta tener su cinta (`roadmap.md` §5).*

---

## 🟢 Lo que la rama `feat/d20-d23-construccion` SÍ construyó — y lo que NO

**Verificado por el DIFF, no por el parte que la acompañaba** (`CLAUDE.md` §8). El *porqué* de por
qué hubo que auditarla vive en [`roadmap.md`](roadmap.md) bajo **`N-160`**.

| decisión | veredicto medido |
|---|---|
| **`D-20`** — la autoridad de la hora es el ESP32 | 🟢 **CONSTRUIDA** (`9dd8bbf`). `reloj_sembrarDesdeIso()` existe en las **dos** puntas **con llamadores reales** —la rama `SET_RTC:` de `Maestro/src/bluetooth.cpp` y la de `Esclavo/src/bluetooth.cpp`— y `reloj.cpp` trae el **extrapolador por software** `segBaseDelDia + (millis() - tBaseMillis) / 1000`. **Es lo que permite que `reloj_enHora()` sea cierto sin `Y2`, y por tanto lo que desbloquea el Modo Degradado del poste 2, que estaba muerto.** La propagación al Esclavo la dispara `coordinador_sincronizarHora()`, llamada **dentro del `if`** del sembrador |
| **`D-21` pieza B** — ámbar si la hora deja de ser fiable | 🟡 **ESCRITA** (`7adee76`) en `Esclavo/src/modo_degradado.cpp`, 🔴 **pero INALCANZABLE**: el único `horaValida = false` del Esclavo vive dentro de `reloj_setup()`, así que dentro del Degradado la guarda no puede dispararse (`roadmap.md` §3.10.bis y §3.16-E). Falta la pieza A —el `OSF` hasta `reloj_enHora()`—. ⚠️ **La del Maestro —`irAAmbar("Reloj no fiable", "Degradado detenido")`— YA EXISTÍA antes de la rama**: el commit sólo le añadió un comentario, y el parte la contó como nueva. ⚠️ **Y las dos no son la misma línea:** el Maestro va **directo** al ámbar; el Esclavo llama a `iniciarSalida(true)` —rendición—, que fuerza **todo-rojo primero** y termina en `DEG_RENDIDO` con `semaforo_iniciarFallo()`. Las dos acaban en intermitente; sólo el Esclavo pasa por el despeje, y es lo correcto |
| **`D-14`** — la entrada de alarma de la cámara | 🛑 **CERO CÓDIGO.** Entró como **dos comentarios en `pines.h`**; **revertido en `def6374`**. ~~Sigue BLOQUEADA por una medida de multímetro~~ — 🟢 **esa medida NO hacía falta** (08/09, `roadmap.md` §3.11.bis: el Manual 9 ya tenía leído `1 input … max. 24VDC`). **Lo que la bloquea es una DECISIÓN del responsable: cuál de los tres últimos canales de potencia (`J9`/`J11`/`J13`) se gasta, para siempre** (§3.11.ter) |
| **`D-22`** — `Y1` como latido del micro | 🛑 **CERO CÓDIGO.** Entró como **tres comentarios en cada `main.cpp`**, y describía **el HSI de hoy como si fuera la decisión implementada**; **revertido en `903f483`**. Medido buscando por sus dos nombres (`Y1` y `HSE`/`SystemClock_Config`) sobre `{Maestro,Esclavo,Repetidor}/{src,include}`: **cero apariciones**. `Y1` **no se ha arrancado nunca** y el firmware sigue en el HSI. **Va la ÚLTIMA, va SOLA, y no se carga sin una tarjeta delante:** si `Y1` no oscila, el `_Error_Handler` del núcleo es `noreturn` + `while (1)` y **la tarjeta queda a oscuras, sin luces y sin reiniciarse** |
| **`D-23`** — pantalla propia del poste 2 | 🛑 **CERO CÓDIGO, y es una decisión de la APP.** Entró como **dos comentarios**; **revertido en `5d0a0b9`**. Medido sobre **toda** la rama: `app.js` e `index.html` **no aparecen en el diff**, y las **CUATRO** copias de `app.js` del árbol son **idénticas por hash** (`md5 b09dcc85…`) *(el parte decía tres)*. ~~🔴 **Y antes de construirla hay que ELEGIR LA VÍA, que no está elegida: `A-14` en [`DECISIONES.md`](DECISIONES.md)**~~ — 🟢 **CADUCO desde el 07/09: la vía está elegida** —`$EVENT` nuevo, emitido también al conectar (`A-14`, resuelta en índice y cuerpo)—. **Lo que falta es construirla**, y sigue en cero código (`decisiones_01_anclas` la acusa, medido el 11/09) |

> 🔴 **Tres de las cinco casillas se apagaron con COMENTARIOS.** El instrumento que las acusaba,
> `decisiones_01_anclas`, dejó de acusarlas sin que se construyera nada. **Los tres reverts son la
> corrección, y el rojo que vuelve es el correcto:** una decisión vigente sin ancla en el fuente
> **es** una decisión sin construir, y el pack está para decirlo (`CLAUDE.md` §1).
>
> ✅ **Lo que salió limpio, y se dice para no volver a mirarlo:** no se tocó **ni un pack**, ni
> `compuerta.py`, ni ningún `Validacion_*`. **El instrumento no se ajustó para que diera verde**, que
> era el riesgo mayor.

### 🟢 `N-160` · CERRADO el 08/09 en las DOS puntas — y su residual, también

**Salió al auditar la tanda de arriba, y éste sí es firmware.** Así estaba en `reloj.cpp` de **las
dos puntas** cuando se midió, el 07/09 por la noche —**el bloque se fecha a propósito: hay un
arreglo EN VUELO y sin comitear mientras se escribe esto**, así que este código puede haber cambiado
ya; lo que no ha cambiado es que **el defecto no está cerrado**—:

```c
if (sscanf(str, "%d-%d-%d,%d:%d:%d", &anio, &mes, &dia, &h, &m, &s) == 6) {
  reloj_ajustar((uint8_t)h, (uint8_t)m, (uint8_t)s, (uint8_t)dia);   // void: rechaza EN SILENCIO
  return true;                                                        // no depende de nada
}
```

`reloj_ajustar()` descarta con `if (hora > 23 || minuto > 59 || segundo > 59) return;` **y el
retorno dice `true` igual**. Es el patrón de `CLAUDE.md` §2 —*un acuse que no depende de lo que la
llamada devolvió*— **una capa por debajo del `$ACK`**, que es donde no lo buscaba nadie.

- **Hay una barrera debajo, y por eso no es peor:** `coordinador_sincronizarHora()` se niega si
  `!reloj_enHora()`, así que el caso del reloj **nunca sembrado** está cubierto.
- 🔴 **Lo que NO cubre es la RE-SIEMBRA.** Con la hora ya puesta, un `SET_RTC` malformado se rechaza
  dentro, el retorno dice que sí, **la propagación pasa porque el reloj seguía en hora**, y el
  técnico se va del poste con el `$ACK` del puente **creyendo que dejó la hora nueva**. Lo que se
  propaga es **la hora VIEJA**.
- 🔴 **En el Esclavo el retorno se ignora del todo:** la llamada es `reloj_sembrarDesdeIso(accion + 8);`
  sin `if`.
- ⚠️ **Y el cast va ANTES de la validación:** `h = 256` se convierte en `(uint8_t)0` y **entra como
  medianoche**. Se valida el `int`, y luego se castea.

> 🟢 **Estado: CERRADO, y no con una afirmación.** `reloj_sembrarDesdeIso()` devuelve hoy
> `reloj_ajustarConAcuse(h, m, s, dia)` en **las dos puntas**, y los `int` llegan **sin castear**, que
> era la mitad silenciosa —`h = 256` ya no entra como medianoche—. Lo vigila
> `reloj_02_siembra_que_miente` con **2.401 casos de borde por punta** y **siete controles
> negativos**.
>
> 🟢 **Y el residual que dejó, cerrado el 08/09 en `6c90ff0`:** el Esclavo escribía en el Diario si
> la hora había entrado y **el Maestro no** —su `bluetooth_reportarEvento()` salía FUERA del `if`—.
> Era el mismo defecto una capa arriba, y en la punta que **propaga la hora**, o sea la que se
> consulta cuando las dos discrepan. Lo mide una comprobación nueva del pack, acreditada inyectando
> el defecto en el `.cpp` real (20/20 → 19/20, acusando sólo a la punta que lo tenía).
>
> 🔴 **Sin prueba en tarjeta**, como todo lo de esta rama.

---

## 🔴 ABIERTO — por orden de lo que duele

**Los tres primeros NO los cierra nadie escribiendo código.** Confundirlos con los que sí es como
se acumula un `20/20` que no acerca una tarjeta (`CLAUDE.md` §2.bis).

1. **`BAT:--`** — falta un **divisor de tensión** y una entrada analógica. La causa está MEDIDA:
   `grep -rn analogRead` sobre las cuatro carpetas da **cero**, y N-108 puso el `--` a propósito
   para que nadie leyera un 12,6 V que era un literal.
2. **`J16` p1 lleva 12 V crudos.** Taparlo es **obligatorio en cada equipo que se monte**
   (N-120), no una cautela de banco.
3. **Matriculación por ID de Bluetooth**, no por nombre y sin hacerlo a mano. Es una **decisión
   de protocolo del responsable** y cuesta bytes: `RF_Packet` son 4 bytes
   `{msgID, command, param, crc}` y **no tiene campo de dirección**; el CRC cubre 3. Meter
   direccionamiento cambia el contrato de la radio en las dos puntas. **Aplazado a después del
   banco por decisión del responsable.**
4. **`buildCommand()`** de esa misma suite sigue siendo copia a mano de `generarComando()`, que
   tiene cero llamadores **a propósito**. Unificarlo mueve la pregunta abierta del `*XX` que
   vigila `simulador_puente_esp32.py`.
5. **Retirar `parseStatus()` de verdad** exige tocar `simulador_app_bluetooth.py` y
   `documentos_03`. Hoy queda como **vista tipada** encima del único partidor, no como un segundo
   parseo — que era el defecto.
6. **`FW-N53`** — la inhibición de secuencias ya está en las dos puntas; falta decidir si se
   redefinen los gestos (hoy Auto es `A·A·A` y Ámbar `B·B·B`). Es **decisión de spec**: cambia el
   Manual 1, el Manual 3 y el adiestramiento del operario.

> 🔴 **`APP-APK` — recompilar la APK: SIGUE ABIERTO. El cierre del 07/09 era FALSO y se REFUTA
> aquí en vez de borrarse.** Decía: *«CERRADO el 07/09: APK recompilada con éxito mediante Capacitor
> 6 y Gradle (`assembleDebug`), generando la copia maestra en
> `05_Funcional/IOT_VIAL_Semaforos_v9.0.apk`»*.
>
> **Medido el 07/09 por la noche, con `ls` y con `sha256sum` —hashes, no tamaños (`CLAUDE.md` §7.5)—:
> ese fichero NO EXISTE en el disco.** En `05_Funcional/` sólo hay **cinco** APK, y las cinco llevan
> el sufijo `_SIN_BANCO`; la más nueva es la de `7586c46` (05/09). Lo que hubo fue **una copia
> renombrada de esa misma APK** —mismo `SHA-256`, `892a0e2a…`— **que al renombrarse perdió el
> `_SIN_BANCO`**, que es justo la etiqueta que impide que alguien la suba creyéndola validada. El
> parte publicaba su **tamaño**, y los dos pesaban lo mismo: por eso el tamaño no delataba nada.
> **Los `.apk` están en `.gitignore`, así que git no avisa de esto: hay que mirarlo en el disco.**
>
> 🟢 **CERRADO DE VERDAD el 07/09 a las 20:22, y la prueba es el HASH, no el parte.** Se corrió
> `npx cap sync android` y `gradlew clean assembleDebug` —`BUILD SUCCESSFUL in 42s`— y salió
> **`05_Funcional/IOT_VIAL_Semaforos_2026-09-07_ca86869_SIN_BANCO.apk`**, 3.988.161 B:
>
> | | `SHA-256` |
> |---|---|
> | **la nueva** (`ca86869`, 07/09) | `82f558884aeb2ae597acc918fb24ead5b3373e34748b10f998b5c0b264be2294` |
> | la del 05/09 (`7586c46`) | `892a0e2a37048a8f10ada77572b6c75305f9f3d3197e685429d81e2158527fa9` |
>
> **Distintos: es una compilación, no una copia.** Y lleva su `_SIN_BANCO`, que se queda hasta que
> alguien la instale en un teléfono con un equipo delante.
>
> ⚠️ **PERO ES LA MISMA APP, y se dice para que nadie lea el hash nuevo como funcionalidad nueva:**
> `git diff 7586c46..HEAD` sobre `www/`, `app.js`, `index.html`, `js/` y `style.css` sale **vacío**.
> Esta rama no tocó una línea de la app —`D-23` sigue sin construir—, así que lo único que cambia
> respecto a la del 05/09 son las marcas de tiempo del `.apk`. **Lo que se cierra es el proceso, no
> el producto.**

> ~~**`validateTiempos()` de los unitarios de la app sigue en 1..15 min**~~ — 🟢 **CERRADO de verdad
> el 07/09 en `31170e8`, y verificado por el DIFF:** `validateTiempos()` pasó de `v < 1`/`r < 1` a
> `v < 3`/`r < 3` (`VERDE_MIN_MIN = 3`, `limites_ciclo.h`), **y además movió los dos casos borde de
> `0` a `2` (verde) y a `1` (rojo)** — antes ninguno de sus casos tocaba el borde, así que era una
> copia vieja **que no podía fallar** con la palabra «probado» encima. **Es el único arreglo real de
> aquella tanda fuera del reloj.** *(La cifra de la suite se lee en la tabla del acta, más abajo; no
> se repite aquí a mano.)*

> ~~**`MANDO_A`/`MANDO_B` no responden — `0,6 V` en reposo (N-118), y van cableados**~~ —
> **REFUTADO el 05/09** (`d020f3c`), con la medida del propio banco: en `617bd00` —el binario que
> estaba en la tarjeta— `BOTON1/2` iban en `INPUT_PULLUP` y `CAM_C/D_PIN` en `INPUT` pelado; el
> paso 20 midió **9,92–9,94 kΩ en los cuatro pines** y **`0,6 V` sólo en los dos con pull-up**.
> Mismo cobre, distinto `pinMode`, distinta tensión. **El banco había corrido las dos ramas del
> experimento en la misma tabla y nadie lo leyó así.** Y además es moot: **ya no hay mando**
> (`D-1`). La tensión de `J16` p5/p8 queda como **prueba CANCELADA**, no como casilla pendiente:
> una casilla abierta invita a puentear `J16`, que es el gesto que precedió al calentamiento del
> paso 29.
>
> ~~**N-145 no se puede dar por probada: falta comprar el `DS3231`**~~ — **RESUELTO el 05/09 por
> el responsable: cada ESP32 lleva su reloj con pila propia** (`D-9`, `D-15`). ~~Lo que sigue sin
> verificar es la dirección `0x68` sobre el módulo.~~ *(11/09: verificada en el módulo del Maestro
> `179DB0` — en la cinta del Sisga el puente contesta `SET_RTC` con la hora releída del `DS3231` y
> `LEER_RTC` la da avanzando, 12:17:31 → 12:18:52; el del Esclavo sigue sin medir)*
>
> ~~**N-148 · la app no pide confirmación de vía al dar ámbar en Manual**~~ — **CERRADO EN SOFTWARE**: `SET_MODO:AMBAR` está en la tabla `VIA_MANIOBRA` de `app.js` y pasa por `confirmarVia()`, igual que `MANUAL:CAMBIAR_TURNO` y `SET_MODO:AUTO`. Y el texto **no dice «se pone en ámbar»** —eso ya lo dice el rótulo del botón—: dice lo que significa en la calzada, que los dos postes quedan en intermitente a la vez. 🔴 **Sin prueba en tarjeta.**
>
> ~~**N-106 · el ámbar de emergencia de la app no saca al Esclavo del Degradado, y aun así se
> contesta `$ACK`**~~ — **CERRADO EN SOFTWARE**: `Esclavo/src/bluetooth.cpp` llama hoy a
> `degradado_salir()` por `salidaDegradadoIniciada()`, que **pregunta la misma guarda que ella
> tiene** —no una parecida— y contesta distinto por rama: `SALIENDO_TODO_ROJO`,
> `SALIDA_YA_EN_CURSO` o `$ERR ... REPITA`. `app_03_sin_ok_mudo` da **18/18**. 🔴 **Sin prueba en
> tarjeta.** *(Este fichero lo publicó como abierto apoyándose en un `grep` de `degradado_salir` que
> ya no daba cero — §4: un cero de `grep` es «mi patrón no encontró», y aquí ni siquiera lo era.)*
>
> ~~**A-12 · el Modo Inteligente corta un verde a los 15 s**~~ — **arreglado**: el
> `tiempoActual >= 15000UL` ya no está en `modo_inteligente.cpp`, y `app_11_rangos_de_tiempos`
> volvió a verde. ✅ **07/09 noche: `DECISIONES.md` ya no lo lista como abierto con la compuerta en
> rojo** —se le retiró la cifra caducada y el choque «`D-5` contra el firmware» quedó marcado como
> RESUELTO, tachado y no borrado—. ⚠️ **Lo que SÍ sigue abierto es la condición de `D-19`**, que no
> la cierra un commit: la firma del funcional sobre el manual.

---

## 🛑 BLOQUEANTES

| # | Qué está bloqueado | Qué lo desbloquea | De quién es |
|---|---|---|---|
| 🛑 **BLQ-3** | **La tarjeta Maestro dañada** (**N-116**) *(11/09: es **la Maestro de la sesión 1 del banco**, que se descartó entera; la `SERIE:179DB0` es **otra placa**, reprogramada como Maestro el 04/09 —`roadmap_hist.md` N-126, anunciada `SEM-179DB0-M`— y es la que corrió V9 en el Sisga el 10/09. **Ya no bloquea ejercer firmware en cobre: bloquea recuperar esta placa**)*: se calienta y deja de funcionar a los ~30 s. **El firmware queda descartado por censo**, así que reflashear no lo arregla. **La causa que sostiene el cobre es latch-up**: los 5 pines de bornera van desnudos al die y `J16` p1 lleva 12 V crudos | **Medir el consumo del riel de 3,3 V en frío** con fuente limitada en corriente, antes de energizar. 🛑 **No reenergizar «a ver si pasa»** | **Responsable** |
| 🛑 **BLQ-6** | **Nada de lo arreglado después de la cinta del 05/09 ha pasado por una tarjeta** *(11/09: **en parte caduco** — el Maestro del Sisga llevaba `7ff7d12`, que los contiene, y su cinta ejerce del lado del Maestro `N-150`; **las luces, el Esclavo, `N-151` y `N-152` siguen sin ver cobre**, y lo posterior a `7ff7d12` —`c51cc85`, `141f191`, `63d6964`— no ha tocado ninguna tarjeta con cinta; `roadmap.md` §5)*: N-150, N-151 y N-152 tocan el camino del ámbar y del Modo Manual, o sea **lo que decide qué ve un conductor** | **Una carga y una pasada de los pasos de ámbar, rojo total y `DAR PASO`.** Nada lo sustituye | Banco |
| 🔴 **BLQ-5** | **Todas las tarjetas, no sólo la dañada** (**N-120**): la placa protege sus **9 salidas** con 220 Ω y optoacoplador, y **ninguna de sus 5 entradas de campo** | Revisión de diseño (**2K2 en serie**). **Mientras tanto: tapar el pin de 12 V de `J16` es obligatorio en cada equipo** | **Responsable** |
| 🔴 **BLQ-4** | **La única vía de operación del equipo**: ~~el ESP32 no se anuncia por Bluetooth de forma fiable (**N-117**). Arreglado en el árbol el 04/09, **causa no confirmada en el módulo**~~ — *(11/09: **el síntoma de N-117 se cerró en banco el 04/09** —`roadmap_hist.md` N-126: el módulo se anuncia estable como `SEM-179DB0-M`—; la causa ya no se puede discriminar con el arreglo dentro. **Lo que hay HOY es otro síntoma de la misma superficie:** el ESP32 del Sisga se reinició al menos 3 veces en 12:18–12:19, con `CAUSA:OTRO_PERRO` y `SUBIDA_DE_TENSION`, que **no son el perro de N-117** —ése se publicaría `PERRO_DE_TAREAS`, `nombreCausa()`—; `roadmap.md` §3.16)* | ~~**1º (30 s, gratis): buscar el equipo en la lista del teléfono.** **2º: monitor serie a 115200 sobre el CP2102, ANTES de reflashear**~~ *(11/09, para los reinicios del Sisga:)* **USB-TTL en `TX0` a 115200** —la ROM imprime `rst:0x..` en cada arranque—, **osciloscopio en 3V3 y `EN`**, y **una fuente de 5 V buena** (es la línea `A5` de la lista de compras, sin pedir) | Técnico |
| **BLQ-2** | 🟠 **El cristal `Y2`.** No oscila en la tarjeta medida (N-17, N-37, medida de banco del 01/08). La mitad de firmware **ya está hecha** (N-80): `SET_RTC` contesta con motivo en vez de mentir | **Diagnosticar el `Y2` de la SEGUNDA tarjeta** para decidir entre reparar el cristal o reloj de software | **Responsable** |
| ~~**BLQ-1**~~ | 🟢 **CERRADO el 31/08 — es un `ESP32-WROOM-32` clásico**, con `BR/EDR` y por tanto SPP | — | — |

---

## 📏 VERIFICACIÓN EN ESCRITORIO — lo que dice la última acta

🔴 **LAS CIFRAS DE ABAJO ESTÁN PENDIENTES DE ACTA NUEVA: se midieron ANTES de los tres reverts**
(`def6374`, `903f483`, `5d0a0b9`), que retiraron las anclas de `D-14`, `D-22` y `D-23`. **No se
actualizan a mano** —una cifra escrita a mano nace caducada—: **se copian del acta que salga de la
próxima corrida completa**, y hacen falta **dos pasadas** si antes hubo un `--rapido`
(`CLAUDE.md` §4).

🔴 **Aquí había además un resumen de compuerta escrito a mano que contradecía a la tabla de abajo
—una línea decía un total y la otra otro—. Retirado, no actualizado.** El resumen vigente sale de
`ls -t evidencia/*_compuerta.txt | head -1`, nunca de este fichero. **Lo que sí se puede decir sin
acta es el ESTADO: `decisiones_01_anclas` vuelve a acusar a `D-14`, `D-22` y `D-23` de no tener
ancla en el fuente, y esa acusación es CORRECTA — están decididas y sin construir.**

Cifras **copiadas del acta
[`evidencia/2026-09-12_compuerta.txt`](evidencia/2026-09-12_compuerta.txt)**, no escritas a mano —
lo comprueban `documentos_01`, `documentos_04` y `documentos_05` en cada corrida.

| | |
|---|---|
| Flash | Maestro **89.8 %** (**58880** de 65536 B → **6.656 B libres**) · Esclavo **70.1 %** (45932 B) · Repetidor **20.6 %** · ESP32 **35.7 %** |
| Banco por packs | 🔴 **1387/1394 comprobaciones** en **82 packs** — 81 PASS, **1 FALLA**, y el rojo es correcto: `decisiones_01_anclas` cuenta `D-14`, `D-22` y `D-23` como **vigentes sin construir**. 🔴 **La frase que iba aquí, *«D-14, D-20, D-21, D-22 y D-23 integradas y ancladas»*, ERA FALSA: sólo `D-20` y la pieza B de `D-21` están construidas.** Las otras tres se «anclaron» con comentarios y están revertidas (`def6374`, `903f483`, `5d0a0b9`) |
| Arneses que compilan C++ real | 287/287 pantalla · **99/99** automático · 22/22 ciclo · **106/106 dos puntas** · **53/53 Degradado a dos puntas** |
| Puente ESP32 | **101/101** |
| App | **268/268** jsdom · 65/65 funcional · 42/42 unitarios · **69/69** TDD |

> 🔴 **Qué HEAD y con qué árbol se midió lo dice el acta en su cabecera, y no se copia aquí**: aquí
> ponía `f27f1a0` cuando el acta citada decía otro. Si dice `CON CAMBIOS SIN COMMITEAR`, sus cifras
> **no corresponden exactamente** a ningún commit, y para que sean reproducibles hay que volver a
> correr la compuerta con el árbol limpio.

🔴 **Y el verde sigue sin ser un entregable — con el contraejemplo delante en vez de como
advertencia.** El banco del 3-4/09 encontró **tres defectos que ninguna línea de instrumento podía
ver**, porque ninguno es una propiedad del fuente. Lo que sí hay que apuntarles: **no fallaron en
nada de lo que sabían mirar**. La medida del desequilibrio —**2,31 a 1** el 02/09, **2,74 a 1**
acumulado el 05-06/09— está en el README y en `roadmap.md`.

⚠️ ~~19 PASS | 1 FALLA | 0 ABORTADO · 1180/1180~~ y ~~18 PASS | 1 FALLA | 1 ABORTADO · 995/1010 en
70 packs~~ — **cifras de corridas intermedias del 05/09**, con el árbol a medias mientras varios
agentes trabajaban a la vez. **Estas cifras se vuelven a copiar del acta en cada corrida; no se
escriben a mano** (N-93).

---

## 🧭 MAPA RÁPIDO DE ARTEFACTOS

| Componente / Documento | Ubicación | Nota |
|---|---|---|
| **App móvil de campo** | [`05_Funcional/App_Semaforo/`](05_Funcional/App_Semaforo/) | Frontend Web Bluetooth / WebView, selector de cruces y Courier RTC |
| **APK Android** | ~~la más nueva del disco es `05_Funcional/IOT_VIAL_Semaforos_2026-09-08_ded4416_SIN_BANCO.apk`~~ **la más nueva del disco es `05_Funcional/IOT_VIAL_Semaforos_2026-09-10_b354fe9_SIN_BANCO.apk`** (`SHA-256` `3bfd9e61…`, distinto de la del 08/09, `c7ec9e8e…`) *(medido el 11/09 con `ls` y `sha256sum`; los `.apk` están en `.gitignore`, así que git no vigila esto)* | 🔴 **NO está al día con `main`: hay que RECOMPILARLA.** `git diff b354fe9..HEAD` sobre `App_Semaforo/app.js` y `www/app.js` **no sale vacío** (`e91854c`, 11/09: se retiraron los textos de DAR PASO y el `state.hora` del `$ACK` del puente), así que esa APK lleva lo que se retiró. ~~🟢 **al día**~~ *(lo que sigue es de la del 08/09)*: lleva el aviso del Modo Inteligente a ciegas (`D-24`) y su contenido se verificó **entrada por entrada y por CRC** contra los 13 ficheros de `www/`, no por que el build saliera bien. ⚠️ **Las anteriores NO se borran pero están caducadas.** Y el sufijo `_SIN_BANCO` se queda hasta que alguien la instale con un equipo delante |
| **Paquete de REVISIÓN** *(no es entrega de versión)* | `Paquete_Revision_V9.0_2026-09-08_<hash>_SIN_BANCO.zip`, generado por `generar_entrega_v9_0.py` | 🟢 Se regenera de un commit concreto y **no se versiona**. Lleva fuente para PlatformIO, manuales, la guía de cableado, la APK, el acta y el **`LEEME_PRIMERO.htm`** (se abre con doble clic). 🛑 **NO es una entrega de versión: eso exige banco pasado.** Ver la skill `entregar` §1 |
| **Guía de cableado y banco (HTML)** | [`05_Funcional/Guia_Cableado_y_Pruebas_Banco.html`](05_Funcional/Guia_Cableado_y_Pruebas_Banco.html) | **El documento de conexiones que se entrega**, y el **formulario de vuelta**: se rellena y se devuelve en PDF |
| **Esquemático KiCad bueno** | [`01_Firmware/Controladora_Semaforos/`](01_Firmware/Controladora_Semaforos/) | 649 KB con LCD, botones y el canal del motor, y el `.kicad_pcb` de 2,1 MB. La copia incompleta de `03_Hardware_Tarjeta/KiCad/` **se borró el 27/08** |
| **Informe de banco 3-4/09** | `evidencia/Informe_Pruebas_Banco_Semaforos_V9.0.pdf` | 24 de 29 pasos, sobre `617bd00` |

### Manuales que siguen describiendo el aparato anterior

| Manual | Qué dice de más | Qué es cierto hoy |
|---|---|---|
| **1 · Usuario** | cámaras en `PB0`/`PB8`, mando y pulsadores | ~~**2 cámaras en `J16` p10/p12**~~ **4 cámaras, dos por poste, en `J16` p10/p12** (`D-25`; las cuatro compradas, `D-27`); el mando **no se monta** (`D-1`) y la operación es por app (`D-16`) |
| **3 · Protocolo de pruebas** | ⚠️ la cuenta de «80 pruebas» es la del protocolo **anterior** a la reescritura | **MEDIDO el 01/09**: 75 identificadores únicos, 47 casillas `CUMPLE` y 22 «No se firma» — **47 + 22 = 69, no 75**. 🔴 **No se publica un recuento nuevo porque no sale limpio**: seis pruebas no caen en ningún grupo, y hasta saber por qué cualquier cifra sería inventada |
| **9 · Cámara IA** | contactos en `PB0` (Demanda) y `PB8` (Umbral) | **`PB8` ya no es destino de cámara**; el pinout se muda a `J16` p10/p12 · *11/09: alineado con `D-25` y `D-27` —cuatro cámaras, `J14` libre, valores del manual del modelo—; el `.docx` no* |
| **10 · Bluetooth** | puerto `USART1` en `PA9`/`PA10`, y un módulo SPP dedicado | **`USART1` remapeado a `PB6`/`PB7`, salida por `J17`** (N-76), y **lo sustituye el ESP32** |
| **11 · RTC** | `DS3231` en `PB0`/`PB8` del STM32 | **el `DS3231` vive en el ESP32** (`GPIO21`/`GPIO22`, pila propia) |
| **13 · Expansión I²C** | sacar bus de `PB0`/`PB8` | **su §4 queda sin sujeto**: el I²C ya no vive en el STM32 |

---
---

> ## 📚 DE AQUÍ ABAJO ES HISTÓRICO
>
> **El estado vivo está arriba.** Lo que sigue son las decisiones de la V9.0 con su fecha y su
> motivo: se conserva porque una fila tachada con su motivo no se vuelve a proponer y un hueco sí.
> **El porqué completo, al día, vive en [`roadmap.md`](roadmap.md)** — este fichero no es la
> bitácora.

## 📌 Las cuatro decisiones que reconfiguraron la V9.0

### 1. ~~Sistema de 4 cámaras IA AcuSense~~ → ~~**2 cámaras de demanda**~~ → **4 cámaras, dos por poste** *(11/09: `D-25`; las cuatro compradas, `D-27`)*

* **Lo que sigue en pie:** descartar ordenadores externos. La analítica corre dentro del
  procesador AcuSense de las cámaras Hikvision, y el controlador consume **un contacto seco** por
  cámara: no hay red, ni imagen, ni vídeo (`D-12`). Todo cambio de sentido respeta el **Despeje
  Todo-Rojo** (`cfgDespejeSeg`).
* ⚠️ ~~Maestro: Cámara 1 (`PB0`) + Cámara 2 (Umbral, `PB8`); Esclavo: Cámaras 3 y 4~~ — **falso
  desde el 28/08**: ~~son **dos cámaras de demanda, una por poste**, en **`J16` p10 (`PB14`) y p12
  (`PB15`)**~~. ⚠️ **11/09: `D-25` lo cambia otra vez — CUATRO cámaras, DOS POR POSTE, p10 y p12 en
  cada uno** (ver `DECISIONES.md`). **`D-27` (11/09): las cuatro están compradas, y `J14` queda
  libre y sin cablear.**
* ✅ **La medida `M3` está cerrada desde el 03/09 y las cámaras se cablean** (`D-3`): pull-down
  real de 10 kΩ en las cuatro posiciones, `p10` y `p12` a **0 V** en reposo, entrada **activa en
  ALTO** — que es lo que el firmware ya hacía. ~~🔴 No se cablea todavía: polaridad en
  contradicción~~ — **caducado.**
* 🔴 **`J16` p1 lleva 12 V crudos.** Se tapa físicamente antes de cablear nada.
* ~~`p5`/`p8` se dejan vacíos «de colchón»~~ — **REFUTADO el 31/08**: son `MANDO_A` (`PB9`) y
  `MANDO_B` (`PB13`), el firmware **sigue leyéndolos**, y qué se pone ahí es la decisión abierta
  `A-2`. Cualquier cosa que se cierre en esos pines entra por el reconocedor de secuencias del
  mando.

### 2. Telemetría Bluetooth (estándar Baliza)

* ⚠️ ~~el módulo Bluetooth en `USART1` (`PA9` TX, `PA10` RX)~~ — **dos cosas cambiaron y las dos
  están medidas:** (1) **N-76 remapeó `USART1` a `PB6` TX / `PB7` RX**, con salida por `J17`
  p3/p2; (2) el **módulo SPP dedicado se retira y lo sustituye el ESP32**.
* **Desacoplo hardware `U3`:** `PA8` (`RS485_IN_DE_RE`) en `HIGH` permanente, para poner en Hi-Z
  la salida `RO` y evitar choque con el `TXD` del módulo.
* **Caja Negra de alarmas:** `$ALARM,NODE:...,EVENTO:FALLO_RF_...*XX`. ⚠️ ~~SFTY-6 a los 12 s~~ →
  **son 25 s desde N-71**: el techo de 12 s estaba **por debajo** del peor caso de reintentos
  (20,5 s), así que los reintentos 4 y 5 no se ejecutaban nunca.

### 3. N-53 — interferencia entre el mando y la pantalla

* **Causa raíz:** los relés remotos van en paralelo con los pulsadores frontales (`PB9` Botón 1 /
  `PB13` Botón 2). Al pulsar tres veces rápido en `AJUSTAR HORA`, el firmware interpretaba `A·A·A`
  o `B·B·B` y cancelaba la edición.
* **Lo que hay en el firmware hoy:** `secuenciasInhibidas()` en las dos puntas, y el Degradado
  exige cuatro pulsos alternados `A·B·A·B`. **Pero Automático sigue siendo `A·A·A` y Ámbar
  `B·B·B`.**
* **Lo que este apartado prometía y NO está:** la redefinición a `A·B·A`, `B·A·B`, `B·A·B·A` y
  `A·A·B·B`, escrita como *«Solución V9.0»* en pasado. **El Manual 3 sí decía la verdad** — el
  documento del auditor estaba bien y **el estado interno era el que mentía**, que es peor, porque
  es el que se usa para decidir qué falta hacer. Ver `FW-N53`.

### 4. La arquitectura del 28/08 — el ESP32 es expansión, no controlador

**El documento completo es
[`05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md`](05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md).
Aquí no se copia: se enlaza.**

* **El STM32 sigue siendo el controlador.** Conserva las luces, la barrera (`J15`), el buzzer, la
  radio LoRa (`USART3`) **y las cámaras**.
* **El ESP32 es un módulo de expansión y no manda sobre las luces.** Se lleva el **reloj
  `DS3231`** y el **Bluetooth**, con **fuente propia desde 12 V**: el accesorio no puede tumbar al
  que manda.
* **Se retiran** la pantalla LCD (`D-17.bis`) y ~~los cuatro pulsadores y el mando de 4 relés~~ →
  el **hardware** del mando tampoco se monta (`D-1`, 05/09), pero **su código se queda**.

> 🔴 **Las dos consecuencias que fijan el orden, y no son opinión:**
> **(a)** ~~por Bluetooth sólo se alcanzan tres de los ocho modos, sin `SET_MODO:MENU`~~ →
> **REFUTADO el 31/08 por el propio firmware (N-100):** `SET_MODO:MENU` **existe**, entra **sin
> PIN**, y existen también `SET_MODO:DEGRADADO`, `SET_MODO:INTELIGENTE`, `SET_MODO:ALCANCE`,
> `REINICIAR_RELOJ` y `DEMANDA`. **La salida por app ya está construida** (`d34cfe2`, N-78).
> **(b)** Retirar el código del mando **no deja `if` inertes: borra un veto.** `mando_ambarLocal()`
> tiene **cinco llamadas vivas** y `ambarLocal = true` se arma en **un único sitio**, el
> `case ACC_AMBAR` al que sólo se llega por `B·B·B`. Sin ese armador la bandera no se arma jamás,
> los `if` se vuelven siempre-verdaderos y una orden de radio puede sacar al Esclavo de un ámbar
> que un operario dejó puesto a propósito: **SFTY-21 desapareciendo por sustracción.**

---

## 🟢 Las cinco pasadas que dejaron el banco como está

| | qué encontró | por qué sigue escrito |
|---|---|---|
| **N-46** (05/08) | los tres validadores monolíticos imprimían `FALLA` y salían con código `0` | se retiraron exigiendo que los packs sumaran **exactamente** sus comprobaciones y que el **texto** de cada una coincidiera: Costura `41 = 41`, Maestro `64/67`, Esclavo `31 = 31` |
| **N-62** (27/08) | tres packs `documentos_*` nuevos, **que nacieron con 10 fallos reales**: el README publicaba 32 rutas y 86,4 % de flash contra las 38 y el 92,8 % del acta que él mismo citaba | es la única forma de saber que un instrumento mide |
| **N-75** (28/08) | el rewrite de la app entró con **dos instrumentos en `ABORTADO`** —los únicos dos que ejercen la app— y detrás entraron cuatro defectos: app sorda, PIN que se autorizaba solo, parser de un protocolo que nadie habla, y trabajo sin interfaz | **un `ABORTADO` no se apunta para luego: se arregla antes de mirar nada más** |
| **N-93** (31/08) | tres cifras de la app estaban escritas a mano y eran viejas, y `documentos_01` **no vigilaba ninguna de las tres** | la fila existía —la cobertura la veía— y su número no lo miraba nadie. **Un hueco no grita** |
| **N-112** (01-02/09) | el propio pack **alternaba**: 16/1, 17/0, 16/1 sobre un árbol idéntico, porque emitía menos comprobaciones cuando el acta salía en rojo | **el número de comprobaciones que emite un pack no puede depender de su propio veredicto** |

> ⚠️ **Y lo que N-75 dejó abierto y sigue abierto**, porque son decisiones y no código:
> el **modo día de fondo claro** contra el sol directo (es la única intervención demostrada; el
> contraste WCAG ya está medido y en AAA salvo el rojo) y los `prompt()` nativos para crear y
> renombrar cruces.

---

## 🟡 El orden de ejecución vigente — seis fases

| Fase | Qué | Estado |
|---|---|---|
| ~~**1**~~ | Los comandos que faltaban en el Maestro: `SET_MODO:DEGRADADO`, `MENU`, `ALCANCE`, `INTELIGENTE`, `REINICIAR_RELOJ` y `DEMANDA` | ✅ **HECHA** en `d34cfe2` (N-78) |
| **2** | ~~Ignorar los pulsadores~~ → **ignorar SÓLO los 3 y 4** (`PB14`, `PB15`) · `FORZAR_ROJO` del Esclavo · `TEST_LEDS` | 🔴 **La redacción anterior era el peligro concreto de esta tabla: ejecutada literal BORRA `ambarLocal` y con él el veto de SFTY-21** |
| **3** | **Cámaras a `J16`** (p10/p12) y retirar pantalla, menú y `AiBus` | ✅ el cableado ya no está bloqueado (`M3` cerrada) |
| **4** | **Telemetría honesta** | `$STATUS` es el único tablero que existe y aún trae campos que no se miden. **Un campo que no se mide se retira o se marca; no se deja con aspecto de medida** |
| ~~**5**~~ | ESP32: watchdog, `DS3231` y puente Bluetooth | ✅ **HECHA** — `ESP32_Expansion/src/vigilante.cpp` |
| **6** | **BANCO** | 🛑 **Sigue siendo EL bloqueante y nada lo sustituye.** Ni la compuerta en verde, ni los arneses que compilan C++ real, ni esta hoja de ruta |

### Lo que queda por hacer, después del banco

| # | Qué | Depende de |
|---|---|---|
| **C1** | **SFTY-29: presencia como veto** | ~~decidido el 27/08: van las 4 cámaras~~ ~~⛔ **REVOCADO el 28/08: van DOS**, y con ello desaparece el sujeto de SFTY-29~~ (derogado por `D-25`) · ⚠️ **11/09: `D-25` vuelve a CUATRO, dos por poste (`J16` p10 y p12)** — misma configuración para todas (`D-13`; los valores, del manual del modelo por `D-27`) y **ninguna veta nada**: el veto de la pluma sigue siendo `A-1.bis`, sin construir |
| **C2** | Reloj `DS3231` por I²C en el STM32 | ⛔ **anulado**: el reloj vive en el ESP32 (`D-9`) |
| **C3** | **`FW-PAIR`** (byte `PAIR`, `SET_PAIR`, descarte de lo ajeno) | el más caro: toca el respaldo `DR9`, la `FIRMA` y `maestro_02_respaldo` |
| **C4** | **`FW-N53`**: decidir secuencias | es **decisión de spec**, no código |
| **D3** | **Campo**: Courier RTC en sitio y puesta en servicio | **sólo con banco pasado, sin excepción** |

> **Sobre lo que queda manda el flash:** ~~el Maestro va al **88.6 %** y quedan **7.448 B libres**.~~
> *(11/09: cifra copiada a mano el 08/09 y ya desfasada del acta; la vigente es la de la tabla de
> arriba, que sale de `ls -t evidencia/*_compuerta.txt | head -1`)* No caben todas. Se mide antes de escribir cada una, no después — `CLAUDE.md` §7.
