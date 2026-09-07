# 🚦 Controladora de Semáforos Móviles de 3 Estados (V9.0)

> 🧭 **Las decisiones vigentes viven en [`DECISIONES.md`](DECISIONES.md)** — una fila por
> decision, con su fecha, su motivo y que deroga. **Dieciocho vigentes, nueve abiertas**
> *(contadas el 07/09 sobre el propio fichero: 19 filas `D-x` con `D-6` derogada, y 10 filas
> `A-x` con `A-11` resuelta)*. Si un parrafo de cualquier documento contradice una fila de
> ahi, **gana la fila**: el parrafo esta caducado. Se lee ANTES de encargar un cambio de
> alcance.

---

## 🔴 Lo que este README no puede decirte, y es lo primero

**En campo corre `e303485` (V8.4, 31/07/2026). Han pasado 38 días y ninguna línea de lo que
hay debajo ha llegado a un poste.** Sí ha llegado a una mesa: el 3-4/09 y la noche del 04/09
hubo banco con dos tarjetas cargadas, y la última cinta —05/09, 22:19— corrió sobre `42a52cd`.
**Todo lo arreglado después de esa cinta no ha visto una tarjeta.**

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
respuesta a la segunda. La regla que salió de ahí vive en `CLAUDE.md` §2.bis:

> **Un `20/20` sobre decenas de miles de líneas de instrumento que nunca han tocado una tarjeta
> no es un entregable: es una coartada.**

La tabla de abajo es verdad. **Lee lo que mide antes de lo que puntúa.**

---

## 📍 De dónde viene este repositorio

> **Este proyecto desciende de `Controladora_Semaforos` @ `50a5380`** (28/08/2026). Allí vive la
> historia completa: 8.900 líneas de firmware validado y dos años de actas en `evidencia/`.
>
> **La historia arranca de cero aquí a propósito, no por descuido.** El repositorio padre pesaba
> **3,47 GB** —dos ZIP de entrega de 2 GB y 1 GB dentro de su historia— y GitHub rechaza de plano
> cualquier fichero de más de 100 MB: esa historia **no se puede empujar**.
>
> **Consecuencia práctica:** si busca por qué una línea del firmware es como es, `git blame`
> **aquí no lo sabe**. Está en el repositorio padre — y sobre todo en
> [`roadmap.md`](roadmap.md), que sí viajó entero y es donde está escrito el *por qué* de cada
> `N-x`.

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
[`evidencia/2026-09-07_compuerta.txt`](evidencia/2026-09-07_compuerta.txt), que genera
`python 01_Firmware/compuerta.py` en una sola corrida. No se escriben a mano — y desde **N-62**
eso ya no es una promesa: el pack `documentos_01_cifras_del_acta` compara esta tabla contra la
última acta en cada corrida del banco. Cuando se escribió por primera vez, **falló**: esta tabla
publicaba 32 rutas y 86,4 % de flash cuando el acta que ella misma citaba medía 38 rutas y
92,8 %. Las cifras eran del 05/08 y llevaban la palabra *«copiadas»* encima.

| Comprobación | Estado | |
|---|---|---|
| guarda de rutas de los instrumentos | ✅ | 64 rutas parseadas, todas existen |
| banco por packs *(77 packs)* | ❌ | **1222/1226 comprobaciones en 77 packs** — las dos que faltan son el hallazgo de `decisiones_01_anclas`: `D-14` y `D-17` están **VIGENTES** en `DECISIONES.md` y **no tienen una sola ancla** en el firmware. El NUMERADOR no lo vigila `documentos_01` a propósito —se estaría midiendo a sí mismo—; de eso se encarga `documentos_05` comparando las copias entre sí |
| compila Maestro / Esclavo / Repetidor / ESP32 | ✅ | **87.6 %** · 63.7 % · 20.6 % · 35.7 % — *el Maestro ocupa **57416 de 65536 B**, o sea **8.120 B libres**; el Esclavo, **41772 B*** |
| simulador funcional | ✅ | 9/9 — eran 20, y 11 de aquellas no medían nada: se retiraron una a una con su evidencia |
| simulador de repetidor | ✅ | 10/10 |
| compila ESP32 | ✅ | 35.7 % — 1122973 de 3.145.728 B |
| simulador del puente ESP32 | ✅ | **101/101** — las tres puntas: `bluetooth.cpp` compilado, la app en jsdom, y solo el ESP32 modelado |
| simulador de app y bluetooth | ✅ | **12/12** — estuvo en `ABORTADO` unas horas el 05/09: **N-149** le añadió el campo `ESC` al `$STATUS` y el instrumento no supo con qué compararlo. Se enseñó a leerlo el mismo día. Queda escrito porque **mientras duró, todo lo que vigilaba entró sin mirar** (`CLAUDE.md` §3.quater) |
| **app ejecutada en DOM** | ✅ | **235/235** — carga `index.html` en jsdom, más `app.js` y **los `js/*.js` que el propio HTML declara, en su orden**, y los **ejercita**: pestañas, modales, ingesta de telemetría, *fuzzing* de 200 tramas corruptas y los botones que mandan comandos. Es el único instrumento que **ejecuta** la app en vez de leerla |
| test funcional de la app | ✅ | **58/58** — decía «22/22» a mano y ejecuta 34; su prueba de Courier RTC era una tautología |
| test unitarios TDD de la app | ✅ | **61/61** — la **segunda** suite unitaria, que hasta el 01/09 **no estaba en la compuerta**: 23 pruebas verdes que no medían nada. *(Esta fila publicó `55/55` hasta el 07/09: era la cifra del 02/09, y `documentos_01` **no la vigila** — no está en su tupla `CIFRAS`.)* |
| test unitarios de la app | ✅ | **32/32** — seis suites que no cargan el navegador: NMEA y *checksums*, generador de comandos y barrera de PIN, validación de `SET_TIEMPOS`, Courier RTC, gestor de cruces y escala de 20 cruces |
| arnés de pantalla | ✅ | **271/271** *(Maestro 145/145, Esclavo 126/126)* — compila el `lcd.cpp` real contra un framebuffer en el PC. **Sigue midiendo aunque la pantalla se retire del equipo**: no necesita la ST7920 |
| arnés del ciclo | ✅ | **22/22** — corre sobre el `ciclo_degradado.h` real compilado, sin espejo en Python |
| arnés del respaldo | ✅ | compila el `calcularSuma()` real; identidad de `respaldo.cpp` entre puntas + prueba de vida |
| arnés del Degradado a dos puntas | ✅ | **18/18** — las dos puntas en Degradado **cada una con su reloj**. Entrega **el número**: el cruce aguanta **29 s** de desfase contra los **20,2 s** que el equipo puede acumular en 48 h, o sea factor **1,44** — y no el 2 que afirmaban los comentarios de las dos puntas |
| arnés de las dos puntas | ✅ | **42/42** — el C++ **real de las DOS puntas** ejecutándose en el mismo proceso y el mismo instante: verde simultáneo en **0 de 53.236 instantes** |
| arnés del automático | ✅ | **99/99** — compila `coordinador.cpp` + `semaforo.cpp` + `modo_automatico.cpp` + `modo_inteligente.cpp`, `demanda.cpp` y el `botones.cpp` real, y comprueba SFTY-2 sobre las escrituras de pin |

**19 PASS · 1 FALLA · 0 ABORTADO, de 20 comprobaciones — la compuerta sale con código `1`.**

> 🔴 **El `1` es el hallazgo, no una regresión.** Lo acusa `decisiones_01_anclas`: `D-14` —*el
> controlador cierra un contacto y la cámara graba*, que fue **el argumento de una compra**— y
> `D-17` —`CMD:LEER_RTC`, **construida** en `ESP32_Expansion/src/despachador.cpp` y **sin marcar**—
> están vigentes en `DECISIONES.md` sin correspondencia en el fuente. La primera se apaga
> **implementándola**; la segunda, **anclando** el código que ya existe.

> 🟢 **Y cuando vuelva a `0`, ese verde será más peligroso que el rojo, no menos.** Mientras la
> compuerta sale con `1` nadie la confunde con un permiso; un `0` sí se confunde. Lo que dice es
> exactamente esto:
> *los modelos y los arneses de PC no encuentran nada*. **No dice que el firmware funcione en la
> tarjeta.** El contraejemplo está fechado: los tres defectos que pararon el banco del 3-4/09
> pasaron estas 20 comprobaciones sin despeinarlas. **Verde no es entregable.**

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

### 📦 El banco son 77 packs — y eso NO es una medalla

```
python 01_Firmware/Simulaciones/banco/correr.py --listar
python 01_Firmware/Simulaciones/banco/correr.py --pack esclavo_03   # un fallo, solo, en 1 s
```

**Por qué se migró** — había tanto instrumento como firmware: **8.898 líneas contra 8.895, uno a
uno**. Y los simuladores no ejecutan el C++, lo *reimplementan a mano*: son una segunda copia que
alguien sincroniza, y eso falló cuatro veces en una semana.

⚠️ **Pero `correr.py` NO es `compuerta.py`.** El banco es **una fila de veinte**: un
`1222/1226` no dice nada de las otras diecinueve, y una de ellas puede estar en `ABORTADO` por
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
**Última actualización del repositorio:** 7 de Septiembre de 2026 *(la cifra vigente y su hash de
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
   |  camaras  J14, J16 |     |          (pila propia)   |
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
| ~~**El mando de relés** (hardware)~~ | 05/09, el responsable: *«ya no tenemos mandos de A y B, sólo la app»* (`D-1`) | 🔴 **El CÓDIGO no se toca, y el motivo está medido:** `mando_ambarLocal()` tiene **cinco llamadas vivas** —tres vetos en `Esclavo/src/main.cpp` y dos decisiones de `CANCELAR_AMBAR` en su `bluetooth.cpp`— y su veto es **SFTY-21**. Retirar el armador deja esos `if` siempre verdaderos: **el veto no queda inerte, queda ABIERTO.** Con el mando desmontado la bandera simplemente no se arma nunca, que es lo correcto |

> 🔴 **Y su consecuencia declarada, que es una propiedad del sistema y no una avería (`D-16`):
> SIN TELÉFONO NO HAY FORMA DE OPERAR EL EQUIPO.** Ni ámbar, ni volver a automático, ni parar el
> cruce. Va escrito en el manual del operario: el teléfono es herramienta crítica.

### Las cámaras van a `J16` p10/p12 — y **ya se cablean**

| `J16` | red | pin | uso nuevo |
|---|---|---|---|
| p1 | `/12V` | — | 🔴 **12 V crudos. Se tapa físicamente en CADA equipo que se monte** (N-120) — es el único conector de señal de la tarjeta que los trae, sin opto ni clamp |
| p5 | `/Boton1` | `PB9` | **sin asignar — decisión abierta `A-2`**, y es de seguridad: el firmware sigue leyendo ese pin y alimentando el reconocedor de secuencias del mando |
| p8 | `/Boton2` | `PB13` | igual que p5 |
| p10 | `/Boton3` | `PB14` | **Cámara 2** *(verificada en banco)* |
| p12 | `/Boton4` | `PB15` | **Cámara 1** |

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
> merge.

### Seis luces gobernadas, no ocho

`pines.h` declara ocho salidas de luz, pero **el semáforo peatonal nunca se conectó a nada**:

| declarado | pin | ¿lo escribe alguien? |
|---|---|---|
| `ROJO1` `AMARILLO1` `VERDE1` / `ROJO2` `AMARILLO2` `VERDE2` | `PA0`-`PA5` | ✅ sí, y **sólo** `semaforo.cpp` |
| `ROJO_PEATON` · `VERDE_PEATON` | `PA6` · `PA7` | 🔴 **ni un `pinMode` ni un `digitalWrite` en ninguna de las dos puntas** |
| `BUZZER` | `PB1` | 🔴 **igual: declarado y nunca escrito** |

Es **hardware pagado y muerto** —tiene opto, MOSFET y bornera propia—. Se anota aquí porque
*"conserva las 8 luces"* es la clase de frase que se copia de documento en documento sin que nadie
corra el `grep`: **la barrera de salidas de `CLAUDE.md` §6 custodia ocho nombres; el equipo mueve
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

## ✅ Sistema operativo en campo (1 de Agosto de 2026)

**Con dos radios en enlace directo, el sistema funciona correctamente.** El fallo de comunicación
que se arrastraba desde el 31/07 tenía **tres causas físicas, ninguna de firmware**: los DIP
switches `M0`/`M1` mal puestos, la tasa aérea a `0.3 kbps` que saturaba el canal half-duplex, y
**la radio B1 con el transmisor averiado**.

⚠️ **Sigue vigente:** 2 radios, enlace directo, **sin repetidor**, a **`2.4 kbps`** de Air Data
Rate y con `M0`/`M1` **ambos en OFF** durante la operación. Ver
[`05_Funcional/4_Manual_Configuracion_Radios.md`](05_Funcional/4_Manual_Configuracion_Radios.md).

---

## 🔧 Lo que dejó el banco del 1 de Agosto — cuatro hallazgos que siguen informando decisiones

| | qué era | estado hoy |
|---|---|---|
| **N-17** | `rtc.begin()` con LSE esperaba al oscilador **sin límite**: si el cristal `Y2` no arranca, el equipo se queda en el arranque para siempre | ✅ espera acotada a 2 s, por debajo de los 4 s del watchdog. *Un semáforo no puede depender de un cristal de reloj para encender* |
| **N-26** | `botones_setup()` declaraba los pines y **nunca los leía**: un botón ya pulsado al encender se leía como flanco y arrancaba un modo que nadie pidió | ✅ se siembra el estado real de cada pin |
| **N-23** | poner la hora no era sincronizar: `coordinador_sincronizarHora()` sólo **encola**, y en el menú nadie movía el coordinador | ✅ cerrado — y hoy el reloj vive en el ESP32 (`D-9`, `D-15`) |
| **N-37** | **el cristal `Y2` está MUERTO**, cerrado por eliminación con tres medidas | ⛔ sigue siendo el motivo de que el reloj sea un `DS3231` colgado del ESP32. Falta diagnosticar el `Y2` de la segunda tarjeta (**BLQ-2**) |

### Carga por SWD: `mode=UR`, y no se cambia

`HOTPLUG` se engancha al micro **en marcha**. Con un firmware que se cuelga al arrancar, el
watchdog reinicia cada 4 s **en mitad del borrado**: `failed to erase memory`. El delator es
`NVM size: 128 KBytes (default)` en un chip de 64 KB.

⚠️ **Si `UR` falla, reintenta — no cambies el modo.** Enganchar es cuestión de *timing* y puede
fallar varias veces con `Unable to get core ID`. Eso **no** es falta de cableado.

---

## 📌 Resumen de Reglas de Color y Seguridad Vial (V8.0 → V8.7)

| Escenario de Operación | Estado Semáforo Maestro | Estado Semáforo Esclavo | Comportamiento del Sistema |
|---|---|---|---|
| **1. Sin Comunicación / Pérdida de Enlace** | 🟡 Amarillo Intermitente (1Hz) | 🟡 Amarillo Intermitente (1Hz) | Entrada a fallo de seguridad tras **25 s** sin PONG/PING. |
| **2. Menú Principal (Con comunicación)** | 🔴 ROJO FIJO Continuo | 🔴 ROJO FIJO Continuo | ~~Menú LCD~~ con re-refuerzo de Rojo Fijo. **Con la LCD retirada este escenario pasa a la app** — el rojo fijo no cambia. |
| **3. Menú Principal (Sin comunicación)** | 🟡 Amarillo Intermitente (1Hz) | 🟡 Amarillo Intermitente (1Hz) | Detección de orfandad ~~en Menú~~ a los **25 s**. |
| **4. Apagado del Esclavo** | 🟡 Amarillo Intermitente (25 s) | Off / Sin Batería | Maestro detecta orfandad a los **25 s** y entra a fallo de seguridad. |
| **5. Apagado del Maestro** | Off / Sin Batería | 🟡 Amarillo Intermitente (25 s) | Esclavo detecta orfandad a los **25 s** y entra a fallo de seguridad. |
| **6. Restablecimiento (Self-Healing)** | 🔴 Rojo Fijo (15s All-Red) | 🔴 Rojo Fijo (15s All-Red) | **RECONEXIÓN AUTÓNOMA SIN REINICIAR NINGUNA TARJETA**. |
| **7. Modo Manual** | 🔴 ROJO FIJO Continuo | 🔴 ROJO FIJO Continuo | **ROJO FIJO INDEFINIDO** hasta que la app dé paso. Con `DAR PASO` alterna rojo/verde como el automático y termina en **rojo+verde**, conservando el todo-rojo de despeje (`D-7`). |
| **8. Transición Verde a Rojo** | 🔴 Rojo Directo (0s) | 🔴 Rojo Directo (0s) | Cumplimiento estricto Resolución 2024 (0s de pre-aviso). |
| **9. Transición Rojo a Verde** | 🟡 Amarillo Fijo (4.0s) | 🟡 Amarillo Fijo (4.0s) | 4.0 segundos de aviso previo para despeje de camiones pesados. |
| **10. Prueba de Alcance (V8.1)** | 🔴 ROJO FIJO Continuo | 🔴 ROJO FIJO Continuo | Diagnóstico con calidad de enlace y tiempo de respuesta. **No arranca ciclos.** |
| **11. Modo Degradado (V8.7)** | Alterna 🟢/🔴 **por reloj** | Alterna 🔴/🟢 **por reloj** | **Activación MANUAL verificada**, nunca automática. Ciclo de 30 s de verde y **30 s de todo-rojo ampliado**. Cae solo a 🟡 tras **48 h** sin resincronizar. |

> ⚠️ **El umbral de silencio de SFTY-6 son `25 s`, no `12 s`** — `SFTY6_SILENCIO_MS = 25000UL` en
> el `protocolo.h` de **las dos puntas**. Esta tabla publicaba `12 s` en cinco filas: era el número
> vigente hasta **N-71**, que midió que aquel techo estaba **por debajo** del peor caso del ciclo de
> reintentos —20,5 s—, así que **los reintentos 4 y 5 no podían ejecutarse jamás** y nada lo
> delataba, porque el equipo hacía algo razonable: irse a ámbar.
>
> ⚠️ **Los gestos de las filas 2, 3, 7 y 10 eran de la botonera y la pantalla, que ya no se
> montan.** El **comportamiento vial** de las once filas —qué color se ve y durante cuánto— **no
> cambia**: lo que cambia es **quién lo pide**, y hoy lo pide la app.

> ### ⚠️ El escenario 1 NO cambia: al perder el radio sigue entrándose en ámbar
>
> El **Modo Degradado** (fila 11) **no sustituye** a ese comportamiento: es un **caso especial que un
> operario activa a mano**, tras verificar las dos puntas. La máquina **nunca** decide sola operar sin
> radio.
>
> **Por qué.** Un ámbar dice *"no estoy controlando esto, decide tú"* y el conductor llega **alerta**.
> Un verde dice *"pasa tranquilo, el otro lado está en rojo"* y llega **confiado**. Sin radio nadie
> puede confirmar que la otra punta siga viva, así que **un verde equivocado es más peligroso que un
> ámbar ambiguo**. El análisis completo, incluidos los riesgos residuales aceptados, está en
> [`OPTIMIZACIONES.md`](OPTIMIZACIONES.md) §SFTY-21.

---

## 🗂️ ~~Estructura del menú (V8.7)~~ — **RETIRADO con la pantalla**

> 🛑 **Esto ya no es lo que se monta** (`D-17.bis`, 05/09). La pantalla LCD se retira de las dos
> puntas y con ella el menú entero: **toda la operación pasa a la app**. Lo que muere es la
> INTERFAZ, y con ella `MODO_HORA` —cuyo único armador vive en `menu.cpp`, detrás de un
> `botonAceptar()` que es `return false;`— y `MODO_ALCANCE`.
>
> **Lo que sí se conserva escrito**, porque sigue explicando decisiones de hoy: el menú era de dos
> niveles porque una lista plana no cabía en los 64 px de alto, y **el límite no era estético** —en
> la V8.6 una sexta línea caía en `y=69` y el peligro no era que no se dibujara, sino que **el
> cursor sí podía navegar hasta ella**, dejando al operario en una opción invisible—. Es el mismo
> criterio que impide que la app esconda una opción donde el dedo sí llega.
>
> ✅ **Y lo que este README publicó como imposible y era falso:** ~~por Bluetooth sólo se alcanzan
> tres de los ocho modos y no existe la vuelta al menú~~ — **REFUTADO el 05/09: son SIETE de
> ocho.** `SET_MODO:MENU` existe en `Maestro/src/bluetooth.cpp`, entra **sin PIN** y sale de todos
> los modos; en Degradado pide la salida ordenada. El único inalcanzable era `MODO_HORA`.
> `ESTADO.md` lo llevaba corregido desde el 31/08 (N-100) **y este README no**, así que durante seis
> días el documento de entrada hacía parecer imposible el «todo por app» que ya estaba hecho.

---

## 🎛️ ~~Mando de relés~~ — el HARDWARE se retiró, el CÓDIGO se queda

> 🟡 **`D-1` (05/09):** *«ya no tenemos mandos de A y B, sólo la app»*. El mando **no se monta**.
> **Su código no se toca**, y el motivo está medido, no razonado: `mando_ambarLocal()` tiene
> **cinco llamadas vivas** y su veto es **SFTY-21**; borrar el armador deja esos `if` siempre
> verdaderos y **el veto queda ABIERTO, no inerte**. Con el mando desmontado la bandera no se arma
> nunca, que es exactamente lo correcto.

**Lo que el firmware hace HOY** — leído de `mando.cpp` de las dos puntas, no de la especificación:

| Secuencia | Acción en el Maestro | Acción en el Esclavo | Confirmación |
|---|---|---|---|
| **`A · A · A`** (≤12 s) | 🟢 **Modo Automático** | obedecer al Maestro *(sale del ámbar local)* | 2 destellos rojos |
| **`B · B · B`** (≤12 s) | 🟡 **Modo Ámbar**, sin condiciones y desde cualquier modo | 🟡 **ámbar local — y desobedece al Maestro a propósito** | 3 destellos rojos |
| **`A · B · A · B`** (≤18 s) | 🕒 **Modo Degradado**, *sólo si la hora está validada* | 🕒 **Modo Degradado** | 4 destellos rojos |

> ⚠️ **La tabla anterior de este README publicaba cinco secuencias y sólo la de Degradado existe en
> el firmware.** Aquella era la **redefinición anti-colisión propuesta en la spec de V9.0**, escrita
> en pasado como si estuviera hecha; lo que sí se implementó de N-53 es `secuenciasInhibidas()` en
> las dos puntas. Queda abierto como **`FW-N53`** en [`ESTADO.md`](ESTADO.md), y es **decisión de
> spec**: cambiar los gestos cambia el Manual 1, el Manual 3 y el adiestramiento del operario.

> 🔴 **Y ese código vivo es la razón de que `J16` p5/p8 sea hoy una decisión de SEGURIDAD (`A-2`):
> cualquier cosa que se cierre ahí entra por el reconocedor de secuencias.** Un fin de carrera de
> talanquera dispararía `B·B·B` (ámbar local) o `A·B·A·B` (Degradado) por accidente.

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
| 9 | [`9_Manual_Parametrizacion_Camara_IA`](05_Funcional/9_Manual_Parametrizacion_Camara_IA.md) | ~~4 cámaras~~ → **2 cámaras de demanda** en `J16`. **Es el entregable principal del diseño de cámaras** (`D-12`): toda la inteligencia vive en la configuración de la cámara |
| 10 | [`10_Manual_Modulo_Bluetooth_Telemetria`](05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.md) | ⚠️ **manda enchufar un `HC-05` en `J17`, que es donde va el ESP32** |
| 11 | [`11_Manual_Instalacion_RTC_DS3231_Bateria`](05_Funcional/11_Manual_Instalacion_RTC_DS3231_Bateria.md) | ~~`DS3231` en `PB0`/`PB8`~~ → **se muda al ESP32**. ⚠️ **manual sin corregir** |
| 12 | [`12_Cobertura_de_Pruebas_y_Huecos`](05_Funcional/12_Cobertura_de_Pruebas_y_Huecos.md) | Qué mide cada instrumento y **qué queda sin medir** |
| 13 | [`13_Manual_Modulo_Expansion_I2C_y_Compras`](05_Funcional/13_Manual_Modulo_Expansion_I2C_y_Compras.md) | ⚠️ **su §4 queda sin sujeto**: el I²C ya no vive en el STM32 |
| 14 | [`14_Manual_App_Movil_IOT_VIAL`](05_Funcional/14_Manual_App_Movil_IOT_VIAL.md) | Operación de la app: SPP, gestor de cruces y Courier RTC |
| 15 | [`15_Lista_de_Compras_Hardware`](05_Funcional/15_Lista_de_Compras_Hardware.md) | **Qué se pide, cuánto y cuándo** |
| 16 | [`16_Documento_Auditoria_Arquitectura_y_Usabilidad_App`](05_Funcional/16_Documento_Auditoria_Arquitectura_y_Usabilidad_App_IOT_VIAL.md) | Auditoría de arquitectura y usabilidad de la app |
| 17 | [`17_Arquitectura_28-08_y_Decisiones_Abiertas`](05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md) | 🧭 **Manda sobre las filas 9, 10, 11 y 13.** Es donde se anotan las medidas de cobre |

*(**La lista se ha desfasado TRES veces por lo mismo, y sigue sin instrumento**: el 27/08 anunciaba
11 manuales de los 14 que existían; el 28/08, 15 de 16; y el 31/08, 16 de **17**. Contar los
ficheros es `ls 05_Funcional/[0-9]*.md`; que nadie lo haga es lo que hace falta cerrar.)*

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
- [`01_Firmware/Simulaciones/`](01_Firmware/Simulaciones): simuladores y `banco/` con los **76
  packs**.
- [`01_Firmware/Validacion_LCD/`](01_Firmware/Validacion_LCD) · `_Ciclo/` · `_Respaldo/` ·
  `_Automatico/`: los arneses que compilan C++ real. Cada uno tiene su punto ciego declarado en
  `CLAUDE.md` §8.
- [`01_Firmware/compuerta.py`](01_Firmware/compuerta.py): **la única forma correcta de
  verificar** — `19 PASS · 1 FALLA · 0 ABORTADO`, exit code 1. Las cifras están en la tabla de
  arriba, que se copia del acta; ésta es sólo la puerta.

> 🛑 **Y para cerrar donde se abrió: nada de este README es un permiso.** En campo corre la
> **V8.4**; la V9.0 compila, pasa la compuerta y **lo que hay hoy en el árbol no ha visto una
> tarjeta**. **Nada sube a campo sin pasar banco** — y el banco no lo sustituye ningún número verde
> de esta página.
