# 🚦 Controladora de Semáforos Móviles de 3 Estados (V9.1)

> 🧭 **Las decisiones vigentes viven en [`DECISIONES.md`](DECISIONES.md)** — una fila por
> decision, con su fecha, su motivo y que deroga. *(Aqui se publicaba cuantas hay; se quito el
> 11/09 porque decia 18 y 9 cuando ya eran mas: se cuentan alli.)* Si un parrafo de cualquier
> documento contradice una fila de ahi, **gana la fila**: el parrafo esta caducado. Se lee
> ANTES de encargar un cambio de alcance.

---

## 🔴 Lo que este README no puede decirte, y es lo primero

**La instalación certificada es `e303485` (V8.4, 31/07/2026).** Lo de debajo llegó a una mesa
el 3-4/09 y la noche del 04/09 —banco con dos tarjetas; la última cinta, 05/09 22:19, sobre
`42a52cd`— y 🔴 **el 10/09 llegó por primera vez a una calle: un Maestro (`SERIE:179DB0`)
corrió en El Sisga con firmware V9 `SIN_BANCO`** —el paquete del 08/09 (`7ff7d12`); después
se probó el del 10/09 (`b354fe9`)—. Su cinta y su diario están en `evidencia/` (308 tramas,
308 checksums que casan). Lo que se reportó allí, lo que dice la cinta y la rama que salió de
ahí, validada por el diff, están en `roadmap.md` §3.16 (`N-162`). 🔴 **Con `7ff7d12`, el
Maestro 179DB0 se declara en hora con el reloj parado: no se usa el Modo Degradado allí.**
**Nada de lo arreglado después de la cinta del 05/09 ha pasado un banco.**

| | firmware | instrumento | ratio |
|---|---|---|---|
| 28/08 | 8.895 | 8.898 | 1,00 : 1 |
| 02/09 | **14.976** | **34.532** | **2,31 : 1** |
| 05-06/09 | — | — | **2,74 : 1** *(acumulado)* |

> **La última medida que hay es el `2,74 : 1` del 05-06/09** (`roadmap_hist.md`, sesión del
> arquitecto —el roadmap se partió el 07/09 y esa sesión está en el histórico—). Las dos primeras filas son las únicas con sus cifras absolutas escritas; el
> `2,74` se publicó como ratio y su recuento no quedó anotado, así que **no se le inventa aquí
> un par de números para rellenar la fila**.

**Tres auditorías externas independientes dijeron lo mismo**, y la tercera lo dijo de la
respuesta a la segunda. La regla que salió de ahí vive en `CLAUDE.md` §0 y §8:

> **Un `20/20` sobre decenas de miles de líneas de instrumento que nunca han tocado una tarjeta
> no es un entregable: es una coartada.**

La tabla de abajo es verdad. **Lee lo que mide antes de lo que puntúa.**

---

## 📐 LA SPEC — empieza por aquí, y son seis ficheros

**Escritas el 12–13/09 desde `DECISIONES.md` vigente y desde el fuente, verificando cada afirmación contra el
código.** Sustituyen a ~17.000 líneas de manuales que describían un sistema que ha cambiado cuatro veces.
**Ninguna pasa de 300 líneas a propósito:** un fichero que no se puede leer entero no lo lee nadie entero, y
entonces cada lector deriva su propia versión.

| | qué contesta |
|---|---|
| [`SPEC_1_Ciclo_y_Luces`](05_Funcional/SPEC_1_Ciclo_y_Luces.md) | qué hace **un** poste con sus tres luces y su pluma: modos, tiempos, transiciones, la barrera de salidas |
| [`SPEC_2_Dos_Puntas_y_Radio`](05_Funcional/SPEC_2_Dos_Puntas_y_Radio.md) | 🔴 **lo único de verdad difícil: que los dos postes NUNCA den verde a la vez**, con una radio lenta que pierde tramas y sin cable entre ellos |
| [`SPEC_3_La_Hora`](05_Funcional/SPEC_3_La_Hora.md) | de dónde sale la hora y qué pasa cuando se pierde — **es lo que autoriza el Degradado**, o sea el modo que da verdes sin confirmar con la otra punta |
| [`SPEC_4_App_y_Bluetooth`](05_Funcional/SPEC_4_App_y_Bluetooth.md) | la **única** forma de operar el equipo (`D-16`): qué órdenes hay, qué contesta el equipo a cada una y de qué depende esa respuesta |
| [`SPEC_5_Cobre_Camaras_Pluma`](05_Funcional/SPEC_5_Cobre_Camaras_Pluma.md) | 🛑 **qué hay en cada pin, qué es peligroso y qué no se ha medido nunca.** Se lee con un destornillador en la mano |
| [`SPEC_8_Camaras_y_Barrera`](05_Funcional/SPEC_8_Camaras_y_Barrera.md) | 🛑 **qué HACE el equipo con las cámaras y la barrera**: el veto, el retardo de 3 s, cuándo pide que se revise una cámara, los dos interruptores de administrador, y **qué NO protege**. Es la otra mitad de `SPEC_5`: aquí lo que decide el **firmware**, allí lo que decide el **cobre** |
| [`SPEC_6_Campo_Radio_y_Alarmas`](05_Funcional/SPEC_6_Campo_Radio_y_Alarmas.md) | el **procedimiento de campo** del Degradado, la configuración de radio y antenas, y la tabla completa de alarmas → qué hace el técnico |

> ⚠️ **La spec dice qué HACE el equipo hoy, no qué debería hacer.** Lo que no cumple una decisión vigente vive
> en el apartado «HUECOS MEDIDOS» de cada una, con la medida pegada. **Y los manuales viejos siguen en el
> árbol a propósito**: se intentó jubilarlos y se paró con medida — hay contenido que ninguna spec cubre
> todavía (la receta de la APK, los ensayos de cámara con instrumento, la trama canónica con checksums de la
> que cuelga un pack). Está contado en `roadmap.md` 1.35(a).

## 🔧 Qué se probó en banco, y qué no

> **El 3 y 4 de Septiembre esto vio una tarjeta.** El funcional ejecutó la guía de 29 pasos
> sobre `617bd00`: **24 pasos completados**, 4 bloqueados por el enlace Bluetooth y 1 abortado
> por un incidente de seguridad. Informe en
> `evidencia/Informe_Pruebas_Banco_Semaforos_V9.0.pdf`.
>
> **Funcionó en cobre:** carga por SWD al primer intento, radio entre puntas con caída a ámbar
> en ~20 s y vuelta sola en ~3 s, talanquera, cámara de demanda y masa común a 0 V.
>
> ✅ **La regresión N-42 —el Modo Automático no movía las luces— está CERRADA EN COBRE.** La
> cerró la sesión de banco de la noche del 04/09, con el responsable delante: *«ahí cambia ese
> amarillo y este a verde. Ahora está funcionando»*. El arreglo es `ceb8cc5` —se retiró el
> asistente entero: una sola puerta, el modo arranca corriendo—. *(Este README publicó
> «regresión abierta» hasta el 07/09; el banco la había cerrado tres días antes.)*
>
> 🔴 **Y aparecieron tres defectos que ninguna cifra de la tabla de abajo podía ver**, porque
> ninguno es una propiedad del código: la tarjeta Maestro tiene un corto de 3,3 V a masa
> (**N-116**), las entradas de campo no tienen ninguna protección mientras las salidas sí
> (**N-120**), y el ESP32 no se anunciaba por Bluetooth (**N-117**). **Los tres pasaron el
> `20/20` sin despeinarlo.**
>
> *(El cuarto de aquella lista, **N-118** —«el mando A/B no responde, 0,6 V en reposo»—, quedó
> **REFUTADO el 05/09**: el propio banco había medido las dos ramas del experimento en la misma
> tabla. Mismo cobre, distinto `pinMode`, distinta tensión. Ver `DECISIONES.md`, cerradas.)*

**Verificación actual** — cifras **copiadas del acta**
[`evidencia/2026-09-15_compuerta.txt`](evidencia/2026-09-15_compuerta.txt), que genera
`python 01_Firmware/compuerta.py` en una sola corrida. No se escriben a mano — y desde **N-62**
eso ya no es una promesa: el pack `documentos_01_cifras_del_acta` compara esta tabla contra la
última acta en cada corrida del banco. Cuando se escribió por primera vez, **falló**: esta tabla
publicaba 32 rutas y 86,4 % de flash cuando el acta que ella misma citaba medía 38 rutas y
92,8 %. Las cifras eran del 05/08 y llevaban la palabra *«copiadas»* encima.

| Comprobación | Estado | |
|---|---|---|
| guarda de rutas de los instrumentos | ✅ | 60 rutas parseadas, todas existen |
| banco por packs *(79 packs)* | 🔴 **FALLA** | **1409/1410 comprobaciones en 79 packs** — 78 PASS, **1 FALLA**. 🔴 **Y el rojo es CORRECTO, no es una regresión:** es `decisiones_01_anclas` acusando a **`D-22`**, la única decisión vigente sin ancla en el firmware — y no se construye con teclado: **necesita una tarjeta delante**. Las que este rojo contaba antes ya salieron de la lista: `D-23` y `D-33` están **construidas**, `D-14` **no se instala** (la cámara graba el evento sola en su microSD, 12/09) y `D-30` quedó **recortada** al retirarse el LCD (`D-32` (1)). 🔴 **Que esta cuenta suba no dice que el banco se degrade: dice que se está decidiendo más rápido de lo que se construye, y se arregla con teclado, no tocando el instrumento** |
| compila Maestro / Esclavo / Repetidor / ESP32 | ✅ | **63.1 %** · 55.1 % · 20.6 % · 35.7 % — *el Maestro ocupa **41328 de 65536 B**, o sea **24.208 B libres**; el Esclavo, **35976 B**. El salto de sitio lo dio el 13/09 la retirada del LCD (`D-32` (1)), no una optimización* |
| simulador funcional | ✅ | 9/9 — eran 20, y 11 de aquellas no medían nada: se retiraron una a una con su evidencia |
| simulador de repetidor | ✅ | 10/10 |
| compila ESP32 | ✅ | 35.7 % — 1123521 de 3.145.728 B |
| simulador del puente ESP32 | ✅ | **119/119** — las tres puntas: `bluetooth.cpp` compilado, la app en jsdom, y solo el ESP32 modelado |
| simulador de app y bluetooth | ✅ | **12/12** — estuvo en `ABORTADO` unas horas el 05/09: **N-149** le añadió el campo `ESC` al `$STATUS` y el instrumento no supo con qué compararlo. Se enseñó a leerlo el mismo día. Queda escrito porque **mientras duró, todo lo que vigilaba entró sin mirar** (`CLAUDE.md` §1) |
| **app ejecutada en DOM** | ✅ | **302/302** — carga `index.html` en jsdom, más `app.js` y **los `js/*.js` que el propio HTML declara, en su orden**, y los **ejercita**: pestañas, modales, ingesta de telemetría, *fuzzing* de 200 tramas corruptas y los botones que mandan comandos. Es el único instrumento que **ejecuta** la app en vez de leerla |
| test funcional de la app | ✅ | **70/70** — decía «22/22» a mano y ejecuta 34; su prueba de Courier RTC era una tautología |
| test unitarios TDD de la app | ✅ | **69/69** — la **segunda** suite unitaria, que hasta el 01/09 **no estaba en la compuerta**: 23 pruebas verdes que no medían nada. *(Esta fila publicó `55/55` hasta el 07/09: era la cifra del 02/09, y `documentos_01` **no la vigila** — no está en su tupla `CIFRAS`.)* |
| test unitarios de la app | ✅ | **63/63** — seis suites que no cargan el navegador: NMEA y *checksums*, generador de comandos y barrera de PIN, validación de `SET_TIEMPOS`, Courier RTC, gestor de cruces y escala de 20 cruces |
| arnés del ciclo | ✅ | **22/22** — corre sobre el `ciclo_degradado.h` real compilado, sin espejo en Python |
| arnés del respaldo | ✅ | compila el `calcularSuma()` real; identidad de `respaldo.cpp` entre puntas + prueba de vida |
| arnés del Degradado a dos puntas | ✅ | **53/53** — las dos puntas en Degradado **cada una con su reloj**. Entrega **el número**: el cruce aguanta **29 s** de desfase contra los **20,2 s** que el equipo puede acumular en 48 h, o sea factor **1,44** — y no el 2 que afirmaban los comentarios de las dos puntas |
| arnés de las dos puntas | ✅ | **120/120** — 🟢 **`D-34` (15/09): el sentido contrario —verde del Esclavo frente al ámbar del Maestro, hasta 23 s medidos— queda en CERO en `G12`–`G14`, vistas fallar antes del arreglo. Sin banco.** ~~la que cae es **G3** (la punta en verde no suelta antes del silencio de SFTY-6: 250 ms de verde frente a ámbar) y espera una decisión del responsable~~ 🟢 **`G3` CERRADO el 12/09 (`N-163`): la ventana pasó de 250 ms a CERO.** El responsable decidió con una condición que fue el criterio de aceptación —*«que no sea que por microcortes de radio el esclavo se pase a ámbar cada nada»*—, y se midió caso por caso sobre **32 cortes**: los ámbares no subieron ni uno (Esclavo 8→8, Maestro 10→10). **El umbral de 25 s no se tocó**; lo que cambió es cuándo se suelta el verde. Los 180 s de verde en las dos del bloque G están CERRADOS (N-162, 11/09). — el C++ **real de las DOS puntas** ejecutándose en el mismo proceso y el mismo instante: verde simultáneo en **0** instantes *(el total lo imprime el arnés; aquí ponía «de 53.236» y ya no casaba)* |
| arnés del automático | ✅ | **75/75** — compila `coordinador.cpp` + `semaforo.cpp` + `modo_automatico.cpp` + `modo_inteligente.cpp`, `demanda.cpp` y el `botones.cpp` real, y comprueba SFTY-2 sobre las escrituras de pin. Las 16 nuevas son el **Bloque G de `D-33`**: el retardo de bajada de la pluma y el veto de la cámara, ejecutados |

> 🛑 **Aquí había una fila más, `arnés de pantalla`, y NO se actualiza: se RETIRA.** Compilaba los cuatro
> `lcd.cpp`/`menu.cpp` reales contra 131 `.c` de U8g2, y **se fue con el LCD** el 13/09 (`D-32` (1), `17d3a1f`).
> La compuerta baja de **20 filas a 19** y el banco pierde sus **287 comprobaciones** (145 Maestro + 142
> Esclavo): **es una bajada A PROPÓSITO, no una regresión.** Se dice aquí abajo y no en la tabla porque en la
> tabla **sólo caben filas con una medida detrás** — una fila sin medida se lee como medida.


**19 PASS · 1 FALLA · 0 ABORTADO, de 19 comprobaciones — la compuerta sale con código `1`.**

> 🔴 **El `1` es el hallazgo, no una regresión.** Lo acusa `decisiones_01_anclas`: **`D-14`, `D-22` y
> `D-23` están vigentes en `DECISIONES.md` sin una línea que las construya.** `D-14` —*el controlador
> cierra un contacto y la cámara graba*— fue **el argumento de una compra**; `D-22` es poner `Y1`, y
> `D-23` es la pantalla propia del poste 2, **que es de la app y la app no se ha tocado**.
>
> 🛑 **Y ninguna de las tres se apaga poniéndole un ancla:** `D-23` se apaga **construyéndola**
> —un `$EVENT` nuevo en el Esclavo y su pantalla en la app—; `D-14` espera **una decisión del
> responsable** —qué canal de `J9`/`J11`/`J13` se gasta, para siempre—; y `D-22` espera **una
> tarjeta delante**, porque si `Y1` no oscila el `_Error_Handler` es `noreturn` y la tarjeta queda
> a oscuras. **El 07/09 se intentó cerrarlas poniéndoles el ancla en un comentario y la compuerta se
> puso en `20/20` sin que se construyera nada** *(`roadmap.md` §3.10)*. Las tres anclas están
> revertidas y el rojo volvió, que es donde tiene que estar.

> 🟢 **Y cuando vuelva a `0`, ese verde será más peligroso que el rojo, no menos.** Mientras la
> compuerta sale con `1` nadie la confunde con un permiso; un `0` sí se confunde. Lo que dice es
> exactamente esto:
> *los modelos y los arneses de PC no encuentran nada*. **No dice que el firmware funcione en la
> tarjeta.** El contraejemplo está fechado: los tres defectos que pararon el banco del 3-4/09
> pasaron estas 19 comprobaciones sin despeinarlas. **Verde no es entregable.**

### Por qué la compuerta está hecha así — cuatro lecciones, en una línea cada una

| | |
|---|---|
| **`ABORTADO` no es `PASS`** | una comprobación que no pudo correr no dice *nada* del firmware. Así se perdió la cobertura del Maestro sin que nadie se enterara (**N-28**) |
| **Un `FALLA` contado como `PASS`** | los tres validadores monolíticos imprimían `FALLA` y salían con código `0`. Se retiraron el 05/08 exigiendo que los packs sumaran *exactamente* sus comprobaciones y que el **texto** de cada una coincidiera — Costura `41 = 41`, Maestro `64/67 = 64/67`, Esclavo `31 = 31` (**N-46**) |
| **Un `gcc` que existe no es un `gcc` que enlaza** | dos arneses cayeron a `ABORTADO` de un día para otro con el mismo compilador en el acta: `ld` no abría una ruta con `ñ`. La compuerta **le exige enlazar un `main()` vacío** antes de fiarse (**N-44**) |
| **Un pack puede estar verde sin medir nada** | la prueba 2.8 del checksum hacía `break` sobre una condición siempre cierta y no evaluaba un solo candidato. Al arreglarla apareció un camino **explotable** en `FLAGS`/`SYNC_BAJA` (**N-51**) |

**El detalle de las cuatro, con su medida, está en [`roadmap.md`](roadmap.md).**

> **Un acta no es una frase.** `evidencia/` guarda fecha, hash de HEAD y versión de toolchain de
> cada corrida, así que "20/20" deja de ser un número que envejece en un README y pasa a ser
> algo que el auditor re-corre sobre ese mismo commit. Estado de hoy en
> [`ESTADO.md`](ESTADO.md); reglas permanentes en [`CLAUDE.md`](CLAUDE.md).

### 📦 El banco son 79 packs — y eso NO es una medalla

```
python 01_Firmware/Simulaciones/banco/correr.py --listar
python 01_Firmware/Simulaciones/banco/correr.py --pack esclavo_03   # un fallo, solo, en 1 s
```

**Por qué se migró** — había tanto instrumento como firmware: **8.898 líneas contra 8.895, uno a
uno**. Y los simuladores no ejecutan el C++, lo *reimplementan a mano*: son una segunda copia que
alguien sincroniza, y eso falló cuatro veces en una semana.

⚠️ **Pero `correr.py` NO es `compuerta.py`.** El banco es **una fila de veinte**: un
`x/y` de packs no dice nada de las otras diecinueve, y una de ellas puede estar en `ABORTADO` por
el mismo cambio que acabas de comitear. **Antes de comitear se corre `compuerta.py`, completo.**

### 📦 Qué se le manda al funcional, y por qué son dos paquetes

**Un paquete es una autorización implícita:** quien lo recibe asume que puede instalarlo. Con el
banco sin pasar, eso obliga a separar lo que pide una medida de lo que entrega una versión.

| | |
|---|---|
| **Encargo de banco** | Pide que alguien ponga la tarjeta delante. Lleva los binarios con sus MD5 y los firmware marcados `SIN_VALIDAR` en el propio nombre |
| **Entrega de versión** | Fuente para PlatformIO + manuales + acta. **Sin `.bin`**: se compila del fuente, así lo que se carga se corresponde con lo que se revisa |

El `LEEME_PRIMERO` **no abre con la cifra en verde** — abre diciendo qué corre en campo, que esto
no es eso, y qué sigue roto. El método está en la skill `entregar`.

**Certificado en campo:** 31 de Julio de 2026 *(V8.4, dos radios en enlace directo)*
**Última actualización del repositorio:** 11 de Septiembre de 2026 *(la cifra vigente y su hash de
HEAD están en el acta que cita la tabla de arriba, y no se repiten aquí: este pie llevaba
`14 PASS` sobre HEAD `2cde016` cuando el acta ya medía otra cosa, y un recuento viejo no se lee
como viejo, se lee como medida.)*
**Repositorio Oficial:** [`github.com/dieleoz/Controladora_Semaforos-2`](https://github.com/dieleoz/Controladora_Semaforos-2.git) — remoto `origin`.
**Normativa Aplicable:** Resolución 2024 del Ministerio de Transporte de Colombia (Secuencia de Luces y Tiempos de Seguridad Vial)

---

## 🧭 La arquitectura vigente, decidida en obra el 28/08 y cerrada el 31/08

**El documento con el detalle medido —`fichero:linea` en cada afirmación, las decisiones
abiertas con dueño y las medidas de multímetro— es
[`05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md`](05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md),
y las decisiones vivas están en [`DECISIONES.md`](DECISIONES.md). Aquí va sólo el reparto,
porque un README que describe el aparato anterior manda a cablear el aparato anterior.**

### El reparto, en una frase

**El STM32 sigue siendo el controlador del semáforo. El ESP32 es un módulo de expansión colgado de
un puerto serie: aporta reloj y Bluetooth, y NO manda sobre las luces.**

```
                       fuente propia 12 V (NO sale de la tarjeta)
                                    |
   +--------------------+     +-----v--------------------+
   |   STM32F103C8      |     |         ESP32            |
   |   (controlador)    |     |   (modulo de expansion)  |
   |                    |     |                          |
   |  luces    J3-J8    |     |  DS3231  GPIO21 SDA      |
   |  barrera  J15      |     |          GPIO22 SCL      |
   |  camaras  J16 (*)  |     |          (pila propia)   |
   |  LoRa     J12      |     |                          |
   |           USART3   |     |  Bluetooth (sustituye    |
   |                    |     |   al modulo SPP)         |
   |  USART1 remapeado  |     |                          |
   |  PB6 TX == J17 p3 <------ GPIO16 (RX2)              |
   |  PB7 RX == J17 p2 ------> GPIO17 (TX2)              |
   +---------|----------+     +-----------|--------------+
             |                            |
             +-------- masa comun --------+
                       9600 8N1
```

*(\*) Aquí ponía ~~`camaras J14, J16`~~. ✏️ **11/09:** las cámaras van a `J16` —dos por poste, p10 y
p12 (`D-25`)—. `J14` no lleva cámara: ~~`A-2` lo reserva al fin de carrera,~~ **pero el firmware lo sigue
leyendo como cámara de demanda (`CAM_DEMANDA_PIN` = `PB0`)** — ~~conflicto abierto, del responsable~~
→ **`D-27` (11/09): `J14` LIBRE, sin cablear; el fin de carrera no se instala en este despliegue**,
así que en `J14` no se conecta nada (`ARQUITECTURA.map` §7.3).*

### El enlace, pin a pin — y la fuente

| ESP32 | dirección | `J17` | STM32 |
|---|---|---|---|
| `GPIO17` (TX2) | ➡️ | **p2** | `PB7` — **RX** del micro |
| `GPIO16` (RX2) | ⬅️ | **p3** | `PB6` — **TX** del micro |
| `GND` | — | p7 o p9 | `GND` — **masa común obligatoria** |

`9600 8N1`. El `USART1` ya está remapeado a `PB6`/`PB7` en el firmware de las dos puntas (N-76,
`bluetooth.cpp`, `grep -n "SerialBT(PB7, PB6)"`) — el puerto y el pinout son los mismos que ya
tenía el módulo SPP; **lo que se enchufa, no**.

> 🔴 **El ESP32 lleva fuente propia desde 12 V. NO cuelga de los 3,3 V de `J17` p6/p8.** Ese riel
> sale del `U5` y es **el mismo que alimenta al STM32 que gobierna el semáforo**: un ESP32 con radio
> da picos de corriente del orden de medio amperio, y un reset del controlador provocado por un
> periférico de diagnóstico es el reparto de riesgo que no se acepta. **El accesorio no puede tumbar
> al que manda.**

### Lo que se retira, y por qué — tachado, no borrado

> **Una vía descartada que desaparece en silencio vuelve a proponerse.** Por eso van tachadas con su
> motivo al lado, y no suprimidas.

| se retira | por qué | consecuencia |
|---|---|---|
| ~~**Pantalla LCD ST7920** (las dos puntas)~~ | **este PCB no permite ampliación**, y la pantalla ocupaba cinco pines —`PB3` `PB4` `PB5` `PB6` `PB7`— de los que el Bluetooth necesita `PB6`/`PB7`: **no había de dónde sacarlos**. Confirmado por el responsable el 05/09 (`D-17.bis`) | toda la operación pasa por la app. **Se retira del EQUIPO, no del código**: `lcd.cpp` y `menu.cpp` siguen compilando y `Validacion_LCD` sigue dando `271/271` sobre un framebuffer del PC |
| ~~**Módulo Bluetooth SPP dedicado** (`HC-05` / `JDY-30`)~~ | lo sustituye el ESP32 por el mismo `J17` | **no se compran** |
| ~~**Los CUATRO botones**~~ | los pines 3 y 4 son los que necesitan las cámaras, y el 05/09 el responsable retiró la botonera entera: **«todo por app»** | ⚠️ **DOS funciones de botón están muertas y DOS SIGUEN VIVAS, y el reparto no es casual**: `botonAceptar()`/`botonCancelar()` son `return false;` —son los de `PB14`/`PB15`, **los pines que ahora son cámaras**—; pero `botonArriba()`/`botonAbajo()` tienen llamadores vivos y leen `BOTON1`/`BOTON2`, **que son los pines del mando**. 🔴 **Lo que alguien cierre en `J16` p5/p8 contra los 3,3 V SIGUE ENTRANDO al firmware.** *(`Botón 1` y `Botón 2` SON `Mando A` y `Mando B`: los mismos dos pines con dos nombres.)* |
| ~~**El mando de relés** (hardware)~~ | 05/09, el responsable: *«ya no tenemos mandos de A y B, sólo la app»* (`D-1`) | ~~🔴 **El CÓDIGO no se toca**~~ 🛑 **DEROGADO POR `D-30` (12/09): el software TAMBIÉN sale** —el responsable: *«hasta el sw lo quitamos»*—. **Decidido y SIN CONSTRUIR**, y va con su precio medido, que es lo que lo tiene parado: `mando_ambarLocal()` tiene **SEIS** llamadas vivas (no cinco), y de ellas **la que ABRE PASO es el veto del `CMD_GO_GREEN` del Esclavo**; retirar sólo el armador deja ese `if` **ABIERTO, no inerte**. Además `mando.cpp` es el **único escritor de `senalActiva`**, así que el cambio toca **`SFTY-2`**, y después **ningún instrumento vigila la interceptación de las salidas por señal**. ⚠️ Y no cierra el cobre: `J16` p5/p8 siguen vacíos y **el firmware los lee por DOS caminos**, no sólo por el del mando (`CLAUDE.md` §3) |

> 🔴 **Y su consecuencia declarada, que es una propiedad del sistema y no una avería (`D-16`):
> SIN TELÉFONO NO HAY FORMA DE OPERAR EL EQUIPO.** Ni ámbar, ni volver a automático, ni parar el
> cruce. Va escrito en el manual del operario: el teléfono es herramienta crítica.

### Las cámaras van a `J16` p10 y p12 — **dos por poste** (`D-25`, 11/09) — y **ya se cablean**

*(Resumen: la tabla que manda es `05_Funcional/17_…` §1.7. La columna «red» es el nombre heredado
del netlist, **no el papel del pin**.)*

| `J16` | red | pin | uso nuevo |
|---|---|---|---|
| p1 | `/12V` | — | 🔴 **12 V crudos. Se tapa físicamente en CADA equipo que se monte** (N-120) — es el único conector de señal de la tarjeta que los trae, sin opto ni clamp |
| p5 | `/Boton1` | `PB9` | **libre y SIN CABLEAR** (`A-2`, cerrada el 05/09: ~~el fin de carrera va a `J14`~~ — *11/09, `D-27`: y tampoco allí, el fin de carrera no se instala; `J14` queda libre*). 🔴 **El firmware sigue leyendo ese pin** y alimentando el reconocedor de secuencias del mando |
| p8 | `/Boton2` | `PB13` | igual que p5 |
| p10 | `/Boton3` | `PB14` | 🎯 **`CAM_C_PIN` — cámara 1**, contacto de alarma entre p9 (3,3 V) y p10 *(verificada en banco el 03/09)* |
| p12 | `/Boton4` | `PB15` | 🎯 **`CAM_D_PIN` — cámara 2**, contacto de alarma entre p11 (3,3 V) y p12 |

> ✅ **`D-25` (11/09, el responsable): las conexiones de la guía de cámaras del Sisga quedan
> DEFINITIVAS** —4 cámaras, dos por poste— y derogan de `D-13` solo *«una por poste, p12 vacío»*.
> 🔴 **Lo que el que cablea tiene que saber, medido en el firmware de hoy:** las **dos entradas hacen
> LO MISMO** (piden paso; `botones.cpp`, `CAM_J16[2]`), **ninguna protege la pluma** (el veto es
> `A-1.bis`, sin construir), y **una segunda cámara muerta desde la instalación no la detecta nadie**
> (el vigilante salta el pin que nunca dio un flanco): se comprueba cada cámara por separado al
> instalar. Las frases falsas de la guía del 10/09 se corrigen en su versión nueva (`roadmap.md` §3.16)
> → ✏️ **11/09: es [`05_Funcional/Camaras_Sisga_4x.html`](05_Funcional/Camaras_Sisga_4x.html)**, con
> la fe de erratas delante y la talanquera en `J15` al final. ⚠️ **La app** (también la APK del 10/09)
> **pone `CAM: OK — las dos ven` en cuanto detecta CUALQUIERA de las dos cámaras del poste**, aunque la
> otra no haya detectado nunca: por eso **cada cámara se comprueba con el multímetro en su borne**
> (0 V en reposo, 3,3 V con algo en la zona), y no con la app.

> ✅ **La medida `M3` está CERRADA desde el 03/09 y las cámaras se cablean** (`D-3`). Medido en
> cobre —multímetro, conector vacío, paso 20 de la Guía—: el pull-down de **10 kΩ** que declaraba
> el netlist **es real y está en las cuatro posiciones** (`R65`–`R68` con su 100 nF); `p10` y `p12`
> dan **0 V en reposo**, así que la entrada es **activa en ALTO**, que es lo que el firmware ya
> hacía. El paso 21 cableó `p10` contra `p11` en normalmente abierto y funcionó, **sin demandas
> fantasma**.
>
> ~~🛑 No se cablea cámara a `J16` hasta cerrar la medida `M3`~~ — **caducado**: este README lo
> siguió publicando cuatro días después de cerrarse, y el 05/09 se le contestó al responsable con
> ese párrafo. Se conserva tachado porque una vía descartada que desaparece vuelve a proponerse.

> ⚠️ **El orden sigue siendo asimétrico: primero el firmware cargado, después el destornillador.**
> Un commit no protege de un destornillador — se exige la **carga verificada en la tarjeta**, no el
> merge. ✏️ **11/09, con `D-25` vale para los DOS pines:** con un binario anterior a `deeeab4` (la
> V8.4 incluida) `PB14` es `botonAceptar()` y **`PB15` es `botonCancelar()`**, así que lo que se
> cablee en `p10` **o en `p12`** puede actuar como un botón en un equipo que está en la calle.

### Seis luces gobernadas, no ocho

`pines.h` declara ocho salidas de luz, pero **el semáforo peatonal nunca se conectó a nada**:

| declarado | pin | ¿lo escribe alguien? |
|---|---|---|
| `ROJO1` `AMARILLO1` `VERDE1` / `ROJO2` `AMARILLO2` `VERDE2` | `PA0`-`PA5` | ✅ sí, y **sólo** `semaforo.cpp` |
| `ROJO_PEATON` · `VERDE_PEATON` | `PA6` · `PA7` | 🔴 **ni un `pinMode` ni un `digitalWrite` en ninguna de las dos puntas** |
| `BUZZER` | `PB1` | 🔴 **igual: declarado y nunca escrito** |

Es **hardware pagado y muerto** —tiene opto, MOSFET y bornera propia—. Se anota aquí porque
*"conserva las 8 luces"* es la clase de frase que se copia de documento en documento sin que nadie
corra el `grep`: **la barrera de salidas de `CLAUDE.md` §2 custodia ocho nombres; el equipo mueve
seis.**

### 🛑 Lo que esta arquitectura NO autoriza

- **Nada de esto ha pasado banco entero, y la compuerta no lo suple.** Lo que este apartado llama
  medido se midió en su mayoría **sobre ficheros** —el `.cpp`, el `.h`, el `.kicad_sch` y el
  `.kicad_pcb`—. Un fichero dice lo que alguien dibujó; una placa dice lo que se fabricó.
- 🟠 **Sin pantalla, `$STATUS` es el único tablero que existe** — y hoy trae `BAT:--` porque
  **no hay un solo `analogRead` en `src/`**. Un campo que no se mide se retira o se marca; no se
  deja con aspecto de medida.
- 🟠 **El ESP32 de este proyecto no tiene watchdog en el Repetidor** *(los dos STM32 sí: 4 s; el
  ESP32 de expansión también, desde `vigilante.cpp`)*, y hay precedente escrito de uno clavado
  tumbando el enlace. Con la pantalla y el mando retirados, un ESP32 colgado deja el equipo **sin
  ninguna superficie de mando**.

**El plano de conexiones que se entrega —`J17`, `J16`, `PB6`/`PB7`, el `DS3231` y el SWD— es
[`05_Funcional/Guia_Cableado_y_Pruebas_Banco.html`](05_Funcional/Guia_Cableado_y_Pruebas_Banco.html).**

---

## 🗂️ Paquete Oficial de Entregables y Manuales (`05_Funcional/`, en `.md` y `.docx`)

| # | Manual | Estado |
|---|---|---|
| 1 | [`1_Manual_Usuario`](05_Funcional/1_Manual_Usuario.md) | Operación y comportamiento vial (Resolución 2024) |
| 2 | [`2_Manual_Hardware_y_Pruebas`](05_Funcional/2_Manual_Hardware_y_Pruebas.md) | Hardware, borneras y ensamblaje STM32 |
| 3 | [`3_Protocolo_Pruebas_Rigurosas`](05_Funcional/3_Protocolo_Pruebas_Rigurosas.md) | Protocolo de auditoría funcional. ⚠️ **reescrito**: cada prueba lleva su marca `SE REESCRIBE` / `SE RETIRA` / `SE APLAZA` |
| 4 | [`4_Manual_Configuracion_Radios`](05_Funcional/4_Manual_Configuracion_Radios.md) | E90-DTU (2.4 kbps / 30 dBm / FEC) |
| 5 | [`5_Manual_Puente_ESP32`](05_Funcional/5_Manual_Puente_ESP32.md) | Puente repetidor V7.6 — **fuera de la configuración vigente** |
| 6 | [`6_Preguntas_Diseno_Funcional`](05_Funcional/6_Preguntas_Diseno_Funcional.md) | Decisiones de diseño cerradas |
| 7 | [`7_Especificacion_Antenas`](05_Funcional/7_Especificacion_Antenas.md) | Antenas y línea de vista |
| 8 | [`8_Procedimiento_Modo_Degradado`](05_Funcional/8_Procedimiento_Modo_Degradado.md) | Operación de emergencia por reloj sin radio |
| 9 | [`9_Manual_Parametrizacion_Camara_IA`](05_Funcional/9_Manual_Parametrizacion_Camara_IA.md) | **4 cámaras, dos por poste** en `J16` p10/p12 (`D-25`, 11/09) — ~~⚠️ **el manual todavía dice una por poste: pendiente de alinear**~~ ✏️ **alineado el 11/09** (el `.docx` no: se regenera aparte). **Es el entregable principal del diseño de cámaras** (`D-12`): toda la inteligencia vive en la configuración de la cámara. 🎯 **11/09, `D-27`: sus VALORES de configuración salen del manual del modelo, [`04_Manuales/MANUAL_CONFIGURACION_CAMARAS_IA.md`](04_Manuales/MANUAL_CONFIGURACION_CAMARAS_IA.md) §4** —tabla valor a valor en su §4 Paso 3—; las cuatro cámaras están compradas |
| 10 | [`10_Manual_Modulo_Bluetooth_Telemetria`](05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.md) | ⚠️ **manda enchufar un `HC-05` en `J17`, que es donde va el ESP32** |
| 11 | [`11_Manual_Instalacion_RTC_DS3231_Bateria`](05_Funcional/11_Manual_Instalacion_RTC_DS3231_Bateria.md) | ~~`DS3231` en `PB0`/`PB8`~~ → **se muda al ESP32**. ⚠️ **manual sin corregir** |
| 12 | [`12_Cobertura_de_Pruebas_y_Huecos`](05_Funcional/12_Cobertura_de_Pruebas_y_Huecos.md) | Qué mide cada instrumento y **qué queda sin medir** |
| 13 | [`13_Manual_Modulo_Expansion_I2C_y_Compras`](05_Funcional/13_Manual_Modulo_Expansion_I2C_y_Compras.md) | ⚠️ **su §4 queda sin sujeto**: el I²C ya no vive en el STM32 |
| 14 | [`14_Manual_App_Movil_IOT_VIAL`](05_Funcional/14_Manual_App_Movil_IOT_VIAL.md) | Operación de la app: SPP, gestor de cruces y Courier RTC |
| 15 | [`15_Lista_de_Compras_Hardware`](05_Funcional/15_Lista_de_Compras_Hardware.md) | **Qué se pide, cuánto y cuándo** |
| 16 | [`16_Documento_Auditoria_Arquitectura_y_Usabilidad_App`](05_Funcional/16_Documento_Auditoria_Arquitectura_y_Usabilidad_App_IOT_VIAL.md) | Auditoría de arquitectura y usabilidad de la app |
| 17 | [`17_Arquitectura_28-08_y_Decisiones_Abiertas`](05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md) | 🧭 **Manda sobre las filas 9, 10, 11 y 13.** Es donde se anotan las medidas de cobre |
| 18 | [`18_Especificacion_Firmware_ESP32`](05_Funcional/18_Especificacion_Firmware_ESP32.md) | Especificación del firmware del módulo de expansión ESP32 |
| 19 | [`19_Especificacion_Placa_Portadora_ESP32`](05_Funcional/19_Especificacion_Placa_Portadora_ESP32.md) | Especificación de la placa portadora del ESP32 |

*(**La lista se ha desfasado CUATRO veces por lo mismo, y sigue sin instrumento**: el 27/08 anunciaba
11 manuales de los 14 que existían; el 28/08, 15 de 16; el 31/08, 16 de 17; y hasta el 11/09, 17 de
**19**. Contar los ficheros es `ls 05_Funcional/[0-9]*.md`; que nadie lo haga es lo que hace falta
cerrar.)*

---

## 🛠️ Estructura del Ecosistema de Firmware

- [`01_Firmware/Maestro`](01_Firmware/Maestro): **El controlador.** Máquina de estados STM32,
  barrera de salidas (`semaforo.cpp`, el único que escribe pines de luz), Modo Degradado, mando de
  relés (código conservado) y telemetría Bluetooth (`USART1` **remapeado a `PB6`/`PB7` → `J17`**).
- [`01_Firmware/Esclavo`](01_Firmware/Esclavo): Nodo secundario STM32 con ACK por radio
  (`USART3`), fallback de orfandad a **25 s**, ámbar local con veto SFTY-21 y telemetría
  Bluetooth.
- [`01_Firmware/ESP32_Expansion`](01_Firmware/ESP32_Expansion): El módulo de expansión —reloj
  `DS3231` y Bluetooth—, **con watchdog** (`vigilante.cpp`).
- [`01_Firmware/Repetidor`](01_Firmware/Repetidor): Puente ESP32 back-to-back. ⚠️ **Fuera de la
  configuración vigente** y **sin watchdog**: es el mismo firmware que se quedó clavado tumbando el
  enlace el 31/07. **No es el ESP32 de expansión.**
- [`01_Firmware/Simulaciones/`](01_Firmware/Simulaciones): simuladores y `banco/` con sus
  packs *(cuántos, en el acta)*.
- [`01_Firmware/Validacion_LCD/`](01_Firmware/Validacion_LCD) · `_Ciclo/` · `_Respaldo/` ·
  `_Automatico/`: los arneses que compilan C++ real. Qué compila cada uno y su punto ciego:
  [`ARQUITECTURA.map`](ARQUITECTURA.map).
- [`01_Firmware/compuerta.py`](01_Firmware/compuerta.py): **la única forma correcta de
  verificar** — `19 PASS · 1 FALLA · 0 ABORTADO`, exit code 1. Las cifras están en la tabla de
  arriba, que se copia del acta; ésta es sólo la puerta.

> 🛑 **Y para cerrar donde se abrió: nada de este README es un permiso.** En campo corre la
> **V8.4**; la V9.1 compila y **pasa 18 de las 19 filas de la compuerta — la que falla es el censo de decisiones sin construir, y falla con razón** (`D-22` necesita una tarjeta delante). **Lo que hay hoy en el árbol no ha visto una
> tarjeta**. **Nada sube a campo sin pasar banco** — y el banco no lo sustituye ningún número verde
> de esta página.
