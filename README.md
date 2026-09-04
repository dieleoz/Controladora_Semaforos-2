# 🚦 Controladora de Semáforos Móviles de 3 Estados (V9.0)

> ## 📍 De dónde viene este repositorio
>
> **Este proyecto desciende de `Controladora_Semaforos` @ `50a5380`** (28/08/2026). Allí vive la
> historia completa: 8.900 líneas de firmware validado y dos años de actas en `evidencia/`.
>
> **La historia arranca de cero aquí a propósito, no por descuido.** El repositorio padre pesaba
> **3,47 GB** —dos ZIP de entrega de 2 GB y 1 GB dentro de su historia— y GitHub rechaza de plano
> cualquier fichero de más de 100 MB: esa historia **no se puede empujar**. Reescribirla habría
> cambiado todos los hashes, y `50a5380` habría dejado de existir como referencia.
>
> **Consecuencia práctica, y conviene saberla antes de necesitarla:** si busca por qué una línea
> del firmware es como es, `git blame` **aquí no lo sabe**. Está en el repositorio padre — y sobre
> todo en [`roadmap.md`](roadmap.md), que sí viajó entero y es donde está escrito el *por qué* de
> cada `N-x`.


> **La versión que corre en campo es la V8.4** (commit `e303485`, 31/07/2026), validada por el
> funcional. Todo lo de la V8.5 a la V9.0 —reloj, sincronización horaria, Modo Degradado, mando de
> relés anti-colisión, cámaras IA Hikvision y telemetría Bluetooth estándar Baliza— está
> especificado y validado en simuladores/arneses.
>
> 🟢 **Y el 3 y 4 de Septiembre por fin vio una tarjeta.** Aquí ponía *«nada de eso ha visto una
> tarjeta»* y dejó de ser cierto: el funcional ejecutó la guía de 29 pasos sobre hardware real
> (`617bd00`), con **24 completados**, 4 bloqueados por el enlace Bluetooth y 1 abortado por un
> incidente de seguridad. Informe en `evidencia/Informe_Pruebas_Banco_Semaforos_V9.0.pdf`.
>
> **Funcionó en cobre:** carga por SWD al primer intento, radio entre puntas con caída a ámbar en
> ~20 s y vuelta sola en ~3 s, talanquera, cámara de demanda y masa común a 0 V. **No se pudo
> probar** el ciclo vehicular real ni la operación por app — y aparecieron **tres defectos que
> ninguna cifra de la tabla de abajo podía ver**, porque ninguno es una propiedad del código:
> la tarjeta Maestro tiene un corto de 3,3 V a masa (**N-116**), las entradas de campo no tienen
> ninguna protección mientras las salidas sí (**N-120**), y el mando A/B no puede pulsarse
> (**N-118**). **Los tres pasaron el `20/20` sin despeinarlo.**
>
> ⚠️ **Y la arquitectura del equipo cambió en obra el 28/08, así que parte de esa lista ya describe
> un aparato que no se va a montar:** ~~sistema de **4** cámaras IA~~ → **2 cámaras de demanda, una
> por poste**; ~~telemetría por módulo Bluetooth SPP dedicado~~ → **el Bluetooth pasa al ESP32**; y
> **se retira la pantalla LCD de las dos puntas**. El reparto vigente está en
> [🧭 La arquitectura vigente](#-la-arquitectura-vigente-decidida-en-obra-el-2808-y-cerrada-el-3108),
> aquí abajo. **Lo que no cambia es lo de arriba: en campo sigue la V8.4.**

> 🔴 **Regresión abierta en banco: el Modo Automático no arranca el ciclo en la rama.** No es
> radio *(`RF:` da porcentaje)* ni botón trabado *(el Botón 3 responde)*. Detalle en
> [`roadmap.md`](roadmap.md) §N-42.
>
> ⚠️ **El banco del 3-4/09 NO la confirmó ni la descartó**, y la distinción importa: el equipo
> **nunca llegó a operar en Modo Automático** porque la única vía para seleccionarlo es la app, y la
> app no pudo conectar. Se decide repitiendo el paso 7 con enlace, no antes. Un *«no se pudo
> probar»* no es un *«sigue rota»* ni un *«ya está»*.
>
> **Esto es lo que la tabla de abajo NO puede ver.** Los simuladores están en verde sobre ese
> mismo firmware: son modelos escritos a mano que **validan el modelo, no el código**, y ninguno
> ejerce el ciclo automático sobre hardware. Un `20/20` de aquí nunca sustituye una prueba de
> banco.

> ✅ **N-49 (T1 y T2) cerrado en software: el defecto de seguridad vial ya no aparece.** El fechado de la
> sincronización pasa del día-del-mes al **contador del RTC**, monótono y sin vuelta. Antes, al
> volver el contador de días de 31 a 1, el Maestro se rendía a ámbar y **el Esclavo seguía dando
> verde** —24 días al año—. Con T2 (commit `98d9058`), el Esclavo se rinde por el mismo criterio
> de RTC monótono que el Maestro, eliminando la rendición desfasada por `millis()`. Medido contra el C++ real. Ver [`ESTADO.md`](ESTADO.md).

> ### 🔴 Lo que este README no puede decirte, y es lo primero
>
> **En campo corre `e303485` (V8.4, 31/07/2026). Han pasado 33 días y ninguna línea de todo lo
> que hay debajo ha tocado una tarjeta.** El arreglo del defecto que se sufre **hoy** en la calle
> —el equipo se va a ámbar por nada— lleva escrito desde el 27/08 y **sin subir**.
>
> | | firmware | instrumento | ratio |
> |---|---|---|---|
> | 28/08 | 8.895 | 8.898 | 1,00 : 1 |
> | 02/09 | **14.976** | **34.532** | **2,31 : 1** |
>
> **Tres auditorías externas independientes dijeron lo mismo**, y la tercera lo dijo de la
> respuesta a la segunda. La regla que salió de ahí vive en `CLAUDE.md` §2.bis:
>
> > **Un `20/20` sobre 34.532 líneas que nunca han tocado una tarjeta no es un entregable: es una
> > coartada.**
>
> La tabla de abajo es verdad. **Lee lo que mide antes de lo que puntúa.**

**Verificación actual** — cifras **copiadas del acta** `evidencia/2026-09-04_compuerta.txt`, que
genera `python 01_Firmware/compuerta.py` en una sola corrida. No se escriben a mano — y desde
**N-62** eso ya no es una promesa: el pack `documentos_01_cifras_del_acta` compara esta tabla
contra la última acta en cada corrida del banco. Cuando se escribió por primera vez, **falló**:
esta tabla publicaba 32 rutas y 86,4 % de flash cuando el acta que ella misma citaba medía 38
rutas y 92,8 %. Las cifras eran del 05/08 y llevaban la palabra *«copiadas»* encima.

| Comprobación | Estado | |
|---|---|---|
| guarda de rutas de los instrumentos | ✅ | 58 rutas parseadas, todas existen |
| banco por packs *(67 packs)* | 🔴 | **974/974** — **todos en `PASS`**. Los ultimos en entrar vigilan lo que la V9.0 estrena: `enlace_01_transporte` (pines, velocidad, buffer y framing del puerto por el que habla el ESP32, que hasta N-94 no miraba nadie), `camara_02_j16`, los nueve del ESP32 y `costura_11_lcd_sin_bus`. |
| compila Maestro / Esclavo / Repetidor / ESP32 | ✅ | **89.3 %** · 65.9 % · 20,6 % · 35,7 % — *7.040 B libres en el Maestro tras las Fases 1 y 2, `AiBus` y N-90* |
| simulador funcional | ✅ | 9/9 — eran 20, y 11 de aquellas no medían nada: se retiraron una a una con su evidencia |
| simulador de repetidor | ✅ | 10/10 |
| compila ESP32 | ✅ | 35,6 % — 1.121.001 de 3.145.728 B |
| simulador del puente ESP32 | ✅ | **85/85** — las tres puntas: `bluetooth.cpp` compilado, la app en jsdom, y solo el ESP32 modelado |
| simulador de app y bluetooth | ✅ | 5/5 — **conectado el 27/08**: existía desde el 26/08 y no estaba en el acta |
| **app ejecutada en DOM** | ✅ | **128/128** — y lo que mide creció: la app **valida el checksum** desde el 01/09 (`validarTrama()` tenía 4 copias y **cero llamadores**), tiene una **pestaña de depuración** con las tramas en crudo y las rechazadas **con su motivo**, y su barrera de PIN ya no se puede armar con el teclado cerrado. — — carga `index.html` en jsdom, más `app.js` y **los `js/*.js` que el propio HTML declara, en su orden** *(desde N-75: el rewrite sacó el gestor de cruces, el parser NMEA y el Courier a módulos, y el arnés seguía evaluando sólo `app.js`)*, y los **ejercita**: pestañas, modales, ingesta de telemetría, *fuzzing* de 200 tramas corruptas y los botones que mandan comandos. Es el único instrumento que **ejecuta** la app en vez de leerla |
| test funcional de la app | ✅ | **58/58** — también conectado el 27/08. Decía «22/22» a mano y ejecuta 34; su prueba de Courier RTC era una tautología |
| test unitarios TDD de la app | ✅ | **55/55** — la **segunda** suite unitaria, que hasta el 01/09 **no estaba en la compuerta**: 23 pruebas verdes que no medían nada |
| test unitarios de la app | ✅ | **32/32** — seis suites sin DOM: NMEA y *checksums*, generador de comandos y barrera de PIN, validación de `SET_TIEMPOS`, Courier RTC, gestor de cruces y escala de 20 cruces. **Faltaba en esta tabla hasta el 28/08**: el acta lo medía y el README no lo nombraba, así que el auditor no tenía forma de saber que existía |
| arnés de pantalla | ✅ | **271/271** *(Maestro 145/145, Esclavo 126/126)* — +12 al exigir el texto exacto del aviso `>48h` (N-50) |
| arnés del ciclo | ✅ | **22/22** — corre sobre el `ciclo_degradado.h` real compilado, sin espejo en Python |
| arnés del respaldo | ✅ | **conectado por fin** (N-43/N-29) — compila el `calcularSuma()` real; identidad de `respaldo.cpp` entre puntas + prueba de vida |
| arnés del Degradado a dos puntas | ✅ | **18/18** — las dos puntas en Degradado **cada una con su reloj**. Entrega **el número**: el cruce aguanta **29 s** de desfase contra los **20,2 s** que el equipo puede acumular en 48 h, o sea factor **1,44** — y no el 2 que afirmaban los comentarios de las dos puntas |
| arnés de las dos puntas | ✅ | **42/42** — el C++ **real de las DOS puntas** ejecutándose en el mismo proceso y el mismo instante: verde simultáneo en **0 de 53.236 instantes** |
| arnés del automático | ✅ | **71/71** — compila `coordinador.cpp` + `semaforo.cpp` + `modo_automatico.cpp` **reales** y comprueba SFTY-2 sobre las escrituras de pin |

**20 PASS · 0 FALLA · 0 ABORTADO, de 20 comprobaciones — la compuerta sale con código `0`.**

✅ **N-112 cerrado: ya no alterna.** Tres corridas seguidas sobre un árbol idéntico dan lo mismo.
Hacían falta **dos** arreglos, no uno: `documentos_01` y `documentos_04` contaban distinto según
el veredicto del acta, y con uno solo arreglado el punto fijo era falso.

🔴 **Y este README publicó un verde que su propia acta desmentía**, veinticuatro horas después de
escribir que eso no se hace: decía `✅ 829/829, los 59 packs en PASS` citando un acta que decía
`FALLA, 824/829, 57 PASS`. La cifra salió de repetir la corrida hasta que una salió verde — que es
**exactamente** el hábito que N-112 describe. Lo encontró una auditoría externa, no nosotros.

> 🟢 **Y ese verde es más peligroso que el rojo, no menos.** Mientras la compuerta salía con `1`
> nadie la confundía con un permiso; un `0` sí se confunde. Lo que dice es exactamente esto: *los
> modelos y los arneses de PC no encuentran nada*. **No dice que el firmware funcione en la
> tarjeta** — y la prueba está delante: con la compuerta en verde sigue abierta una regresión de
> banco donde el Modo Automático no mueve las luces. **Verde no es entregable.**
>
> ✅ **Los tres validadores monolíticos se retiraron el 05/08 (N-46).** Aquel día la lista pasó de
> **17 comprobaciones a 14**; las de hoy son las de la tabla de arriba. Imprimían `FALLA` y salían con código `0`, así que un fallo real se
> pintaba en verde. Cada uno se retiró demostrando que los packs sumaban **exactamente** sus
> comprobaciones y que el **texto** de cada una coincidía: Costura `41 = 41`, Maestro
> `64/67 = 64/67`, Esclavo `31 = 31`.
>
> ⚠️ **El último `FALLA` que cayó escondía algo peor que un checksum flojo (N-51).** Arrastraba
> *"10 pares ciegos"* que **nunca se midieron** —la prueba usaba unos pesos fijados a `1`, resto
> de un algoritmo anterior, y marcaba los 10 pares posibles sin llamar jamás al checksum—. Y su
> prueba hermana **daba `PASS` sin evaluar un solo candidato**. Al arreglarlas apareció un caso
> **explotable**: permutar `FLAGS` y `SYNC_BAJA` deja la suma intacta y enciende
> `CICLO+SYNC+DEGRADADO`, que un arranque tras corte leería como autorización vigente. Cerrado
> guardando el Horner de 32 bits sin plegar. Detalle en [`roadmap.md`](roadmap.md).
>
> **No queda ningún fallo vial en software.** Los del día-del-mes se cerraron con **N-49**; la
> asimetría de pantalla, con **N-50**; y el residual de ±60 s del protocolo dejó de contarse como
> `FALLA` —ningún firmware puede aprobarlo— para pasar a `reportar()`, con una comprobación en su
> lugar que exige que el agujero sea *exactamente* el que el protocolo obliga.

> **Un `gcc` que existe no es un `gcc` que enlaza (N-44).** Los dos arneses que compilan C++ real
> cayeron de `PASS` a `ABORTADO` de un día para otro **con el mismo compilador en el acta**: `ld` no
> encontraba `crt2.o` ni `libgcc.a` —ficheros que existen y se leen sin problema— porque el
> toolchain estaba instalado bajo una ruta con `ñ`. La compuerta ya no da por bueno el primer `gcc`
> que encuentra: **le exige enlazar un `main()` vacío**. Censar el instrumento no es comprobar que
> mide, que es la misma distinción que separa `PASS` de `ABORTADO`.
>
> **Un acta no es una frase.** `evidencia/` guarda fecha, hash de HEAD y versión de toolchain de
> cada corrida, así que "20/20" deja de ser un número que envejece en un README —**ya pasó**— y
> pasa a ser algo que el auditor re-corre sobre ese mismo commit. Estado de hoy en
> [`ESTADO.md`](ESTADO.md); reglas permanentes en [`CLAUDE.md`](CLAUDE.md).

> ### 📦 Qué se le manda al funcional, y por qué son dos paquetes
>
> **Un paquete es una autorización implícita:** quien lo recibe asume que puede instalarlo. Con el
> banco sin pasar, eso obliga a separar lo que pide una medida de lo que entrega una versión.
>
> | | |
> |---|---|
> | **Encargo de banco** | Pide que alguien ponga la tarjeta delante. Lleva los binarios del bisect con sus MD5 y los firmware marcados `SIN_VALIDAR` en el propio nombre |
> | **Entrega de versión** | Fuente para PlatformIO + manuales + acta. **Sin `.bin`**: se compila del fuente, así lo que se carga se corresponde con lo que se revisa |
>
> El `LEEME_PRIMERO` **no abre con la cifra en verde** — abre diciendo qué corre en campo, que esto
> no es eso, y qué sigue roto. Un lector que empieza por *"100% PASS"* no llega a la línea que
> importa. El método está en la skill `entregar`.

> ### 📦 El banco son 67 packs — la migración terminó el 05/08, y creció con V9.0
>
> *(Este rótulo decía **34** mientras la tabla de arriba y el acta que ella cita decían **38**. Un
> documento que se contradice a sí mismo en dos párrafos es el caso que ya cazó N-62 con las rutas;
> la cifra buena es siempre la del acta.)*
>
> ```
> python 01_Firmware/Simulaciones/banco/correr.py --listar
> python 01_Firmware/Simulaciones/banco/correr.py --pack esclavo_03   # un fallo, solo, en 1 s
> ```
>
> **Por qué** — había tanto instrumento como firmware: **8.898 líneas contra 8.895, uno a uno**. Y
> los simuladores no ejecutan el C++, lo *reimplementan a mano*: son una segunda copia que
> alguien sincroniza. Eso falló **cuatro veces en una semana** (N-36, N-39, N-40 y la propia compuerta).
>
> **Ninguno se retiró por confianza.** Cada uno exigió que los packs sumaran *exactamente* sus
> comprobaciones **y que el texto literal de cada una coincidiera**, no solo el recuento:
>
> | | packs | monolito | |
> |---|---|---|---|
> | Maestro | 64/67 | 64/67 | ✅ retirado |
> | Esclavo | **31/31** | **31/31** | ✅ retirado |
> | Costura | **41/41** | **41/41** | ✅ retirado |
>
> Esa comparación no fue ceremonia: **cazó dos errores reales de la propia migración** — una
> función del modelo cortada a media línea que dejaba de reproducir dos hallazgos, y un arreglo
> aplicado al pack pero no al monolito, que solo se vio porque los totales dejaron de cuadrar.
> Ver [`roadmap.md`](roadmap.md) §V8.8.
>
> **Y al retirarlos cayó la guarda de rutas**, que censaba las tuplas de los monolitos: se quedó
> en 4 y **abortó en vez de aprobar**, que es exactamente su trabajo. Hoy censa `banco/packs` y
> `banco/modelos` — 56 rutas, ninguna escrita a mano.

> ⚠️ **`ABORTADO` no es `PASS`.** Una comprobación que no pudo correr no dice *nada* del firmware.
> Así se perdió la cobertura del Maestro sin que nadie se enterara: `validador_maestro.py` llevaba
> días abortando en silencio tras reescribirse el checksum de `respaldo.cpp`, y desde fuera parecía
> que corría. `compuerta.py` (**N-28**) existe para que esa clase de fallo sea imposible de pasar por
> alto: un único exit code y las tres palabras separadas en el resumen.

> ⚠️ **Nada de esto sustituye la prueba de banco.** Los simuladores son modelos escritos a mano: no
> compilan ni ejecutan el C++, y **validan el modelo, no el código**. El arnés de pantalla sí compila
> los `lcd.cpp` y `menu.cpp` reales, pero contra un framebuffer en el PC, no contra la ST7920.

**Certificado en campo:** 31 de Julio de 2026 *(V8.4, dos radios en enlace directo)*  
**Última actualización del repositorio:** 4 de Septiembre de 2026 *(la cifra vigente y su hash de HEAD están en el acta que cita la tabla de arriba —`evidencia/2026-09-04_compuerta.txt`—, y no se repiten aquí: este pie llevaba `14 PASS` sobre HEAD `2cde016` cuando el acta ya medía otra cosa, y un recuento viejo no se lee como viejo, se lee como medida.)*

> 🟢 **El 3 y 4 de Septiembre esto SÍ pasó por banco, y este pie decía lo contrario hasta hoy.** El
> funcional ejecutó la guía de 29 pasos sobre `617bd00` con dos tarjetas cargadas: **24 completos, 4
> bloqueados por el enlace Bluetooth y 1 abortado por un incidente de seguridad**. Informe en
> `evidencia/Informe_Pruebas_Banco_Semaforos_V9.0.pdf`.
>
> **Lo que el banco NO cerró, y por eso sigue sin ser un entregable de campo:** la regresión **N-42**
> del Modo Automático **no se pudo ni confirmar ni descartar** —el equipo nunca llegó a operar,
> faltaba la app—; el **verde simultáneo** sigue sin ejercerse sobre hardware; y aparecieron **tres
> defectos que ninguna de estas cifras podía ver**: la tarjeta Maestro se calienta y se para a los
> ~30 s (**N-116**), el ESP32 no se anuncia por Bluetooth (**N-117**, arreglado en el árbol y
> pendiente de confirmar en el módulo) y el mando A/B no puede pulsarse (**N-118**). Los tres pasaron
> el `20/20` sin despeinarlo. El detalle está en [`roadmap.md`](roadmap.md) §6.
**Repositorio Oficial:** [`github.com/dieleoz/Controladora_Semaforos-2`](https://github.com/dieleoz/Controladora_Semaforos-2.git) — remoto `origin`. *(Este renglón publicaba `2semaforos_3estados.git`, que es el remoto **`padre`**: `git remote -v` lo dice. Un README que da la dirección del repositorio anterior manda a clonar el árbol anterior.)*  
**Normativa Aplicable:** Resolución 2024 del Ministerio de Transporte de Colombia (Secuencia de Luces y Tiempos de Seguridad Vial)

---

## 🧭 La arquitectura vigente, decidida en obra el 28/08 y cerrada el 31/08

**El documento con el detalle medido —`fichero:linea` en cada afirmación, las cinco decisiones
abiertas con dueño, las cinco medidas de multímetro y el censo de manuales que quedan falsos— es
[`05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md`](05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md).
Aquí va sólo el reparto, porque un README que describe el aparato anterior manda a cablear el
aparato anterior.**

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
| `GPIO17` (TX2) | ➡️ | **p2** | `PB7` — **RX** del micro (pin 43 del `U1`) |
| `GPIO16` (RX2) | ⬅️ | **p3** | `PB6` — **TX** del micro (pin 42 del `U1`) |
| `GND` | — | p7 o p9 | `GND` — **masa común obligatoria** |

`9600 8N1`. El `USART1` ya está remapeado a `PB6`/`PB7` en el firmware de las dos puntas (N-76,
`bluetooth.cpp:25` del Maestro y `:26` del Esclavo) — el puerto y el pinout son los mismos que ya
tenía el módulo SPP; **lo que se enchufa, no**.

> 🔴 **El ESP32 lleva fuente propia desde 12 V. NO cuelga de los 3,3 V de `J17` p6/p8.** Ese riel
> sale del `U5` y es **el mismo que alimenta al STM32 que gobierna el semáforo**: un ESP32 con radio
> da picos de corriente del orden de medio amperio, y un reset del controlador provocado por un
> periférico de diagnóstico es el reparto de riesgo que no se acepta. **El accesorio no puede tumbar
> al que manda.** La fuente (DC-DC 12 V→5 V) **todavía no se ha pedido** — línea `A5` del Manual 15.

### Lo que se retira, y por qué — tachado, no borrado

> **Una vía descartada que desaparece en silencio vuelve a proponerse.** Por eso van tachadas con su
> motivo al lado, y no suprimidas.

| se retira | por qué | consecuencia |
|---|---|---|
| ~~**Pantalla LCD ST7920** (las dos puntas)~~ | **este PCB no permite ampliación** — el proyecto anterior sí, y de ahí venía todo lo que se había desarrollado para ampliarlo. Nada de eso era físicamente realizable soldando sobre una placa que no lo admite. La pantalla ocupaba **cinco pines** —`PB3` `PB4` `PB5` `PB6` `PB7`, `pines.h:85-89`— y el Bluetooth necesita `PB6`/`PB7`: **no había de dónde sacarlos** | toda la operación pasa por la app |
| ~~**Módulo Bluetooth SPP dedicado** (`HC-05` / `JDY-30`)~~ | lo sustituye el ESP32 por el mismo `J17` | **no se compran**: la línea `A1` del Manual 15 está anulada |
| ~~**Botón 3 (`PB14`, `J16` p10) y Botón 4 (`PB15`, `J16` p12)**~~ | sus pines son los que necesitan las cámaras | 🔴 `botonCancelar()` = Botón 4 es **hoy la única salida de los ocho modos**: ver el aviso de abajo |

**Y una vía que se planteó retirar y NO se retira — decidido el 31/08:**

| se conserva | medida que lo sostiene |
|---|---|
| ✅ **El mando de relés, en sus canales A y B** — `MANDO_A` = Botón 1 = `PB9` (`J16` p5) y `MANDO_B` = Botón 2 = `PB13` (`J16` p8), `botones.cpp:119-120` | El mando **nunca usó C ni D**: `grep "BOTON[1-4]" Maestro/src/mando.cpp` da **cero**. Y se conservan **los dos** canales, no uno: el veto de SFTY-21 —`ambarLocal`— **sólo lo arma `B·B·B`** (`Esclavo/src/mando.cpp:246-248` y `:129-132`). Con un solo canal no hay `B·B·B` que dar |

> 🔴 **Por eso conservar A y B no es nostalgia: es lo que impide que SFTY-21 desaparezca por
> sustracción.** `mando_ambarLocal()` tiene **tres consumidores, y los tres vetan**
> (`Esclavo/src/main.cpp:406`, `:416`, `:540`): mientras un operario pidió ámbar local desde el
> suelo, una orden de radio **no** saca a esa punta del ámbar. Si el armador de esa bandera
> desaparece, los tres `if` se vuelven siempre-verdaderos y el veto no queda inerte: **queda
> abierto**.

### Las cámaras pasan a `J16` p10/p12 — y **no se cablean todavía**

| `J16` | red | pin | uso nuevo |
|---|---|---|---|
| p1 | `/12V` | — | 🔴 **12 V crudos. Se tapa físicamente** — es el único conector de señal de la tarjeta que los trae, sin opto ni clamp |
| p5 | `/Boton1` | `PB9` | **mando, canal A** *(se queda)* |
| p8 | `/Boton2` | `PB13` | **mando, canal B** *(se queda)* |
| p10 | `/Boton3` | `PB14` | **Cámara 2** |
| p12 | `/Boton4` | `PB15` | **Cámara 1** |

> 🛑 **No se cablea cámara a `J16` hasta cerrar la medida `M3`.** La polaridad de esos pines está en
> **contradicción medida**: el netlist tiene pull-**down** de 10 kΩ con 3,3 V en la posición de al
> lado —entrada activa en **ALTO**— y `botones.cpp:19` los pone en `INPUT_PULLUP` y lee `== LOW`.
> Los dos no pueden ser ciertos. Es **N-67 otra vez**, y se cierra con multímetro (Manual 17 §A),
> no leyendo más código. Cablear al revés da **demanda permanente** —un semáforo que se pide paso
> solo— o **demanda que nunca llega**. Las dos son de calle.
>
> ⚠️ **Y el orden es asimétrico: primero el firmware cargado, después el destornillador.** Con el
> firmware viejo todavía dentro, `PB14` sigue siendo `botonAceptar()` **leído activo en BAJO**:
> cualquier cosa que un instalador enchufe en `J16` p10 puede pulsar *Aceptar* en un equipo que está
> en la calle. Un commit no protege de un destornillador — se exige la **carga verificada en la
> tarjeta**, no el merge.

### Seis luces gobernadas, no ocho

`pines.h` declara ocho salidas de luz, pero **el semáforo peatonal nunca se conectó a nada**:

| declarado | pin | ¿lo escribe alguien? |
|---|---|---|
| `ROJO1` `AMARILLO1` `VERDE1` / `ROJO2` `AMARILLO2` `VERDE2` | `PA0`-`PA5` | ✅ sí, y **sólo** `semaforo.cpp` |
| `ROJO_PEATON` · `VERDE_PEATON` | `PA6` · `PA7` | 🔴 **ni un `pinMode` ni un `digitalWrite` en ninguna de las dos puntas** |
| `BUZZER` | `PB1` | 🔴 **igual: declarado y nunca escrito** |

Es **hardware pagado y muerto** —tiene opto, MOSFET y bornera propia— y ya está escrito así en
[`OPTIMIZACIONES.md`](OPTIMIZACIONES.md) al lado de la talanquera. Se anota aquí porque *"conserva
las 8 luces"* es la clase de frase que se copia de documento en documento sin que nadie corra el
`grep`: **la barrera de salidas de `CLAUDE.md` §6 custodia ocho nombres; el equipo mueve seis.**

### 🛑 Lo que esta arquitectura NO autoriza

- **Nada de esto ha pasado banco, y la compuerta no lo suple.** No hay una sola fila *«verificado en
  la placa»* en todo el mapeo de la tarjeta: lo que este apartado llama medido se midió **sobre
  ficheros** —el `.cpp`, el `.h`, el `.kicad_sch` y el `.kicad_pcb`—. Un fichero dice lo que alguien
  dibujó; una placa dice lo que se fabricó, y lo que alguien reparó después.
- 🔴 **`SET_MODO:MENU` va ANTES de dejar de atender los pulsadores, no después.** Hoy se entra a un
  modo por el menú y se sale con el Botón 4; por Bluetooth se alcanzan **tres** de los ocho modos y
  no existe la vuelta al menú. Sin ese comando, cada modo es **una puerta de un solo sentido**: se
  entra desde el celular y no se sale más que cortando la energía.
- 🟠 **Sin pantalla, `$STATUS` es el único tablero que existe** — y hoy trae `BAT:12.6` literal en
  las dos puntas *(no hay un solo `analogRead` en `src/`)*, más `RF:98%`, `RTT:85ms` y
  `MODO:SUBORDINADO` fijos en el Esclavo. **Un campo que no se mide se retira o se marca; no se deja
  con aspecto de medida.**
- 🟠 **Qué chip es el ESP32 sigue sin leerse.** `ESP32-WROOM-32*` trae Bluetooth Clásico y la app
  conecta tal cual; `S3`/`C3` son **sólo BLE** y el socket SPP no abre nunca; `S2` no tiene
  Bluetooth. Se responde leyendo la **serigrafía del blindaje metálico** —treinta segundos, y **el
  rótulo del vendedor no distingue**—, y bloquea la compra.
- 🟠 **El ESP32 de este proyecto no tiene watchdog** *(los dos STM32 sí: 4 s)*, y hay precedente
  escrito de uno clavado tumbando el enlace. Con la pantalla retirada, un ESP32 colgado deja el
  equipo **sin ninguna superficie de mando** salvo el mando de relés que se conserva.

**El plano de conexiones que se entrega —`J17`, `J16`, `PB6`/`PB7`, el `DS3231` y el SWD— es
[`05_Funcional/Guia_Cableado_y_Pruebas_Banco.html`](05_Funcional/Guia_Cableado_y_Pruebas_Banco.html).**

---

> ## ✅ SISTEMA OPERATIVO EN CAMPO (1 de Agosto de 2026)
>
> **Con dos radios en enlace directo, el sistema funciona correctamente.**
>
> El fallo de comunicación que se arrastraba desde el 31/07 tenía **tres causas físicas, ninguna de
> firmware**: los DIP switches `M0`/`M1` mal puestos *(error del propio manual, ya corregido)*, la
> tasa aérea a `0.3 kbps` que saturaba el canal half-duplex, y **la radio B1 con el transmisor
> averiado**. Retirada la B1, el enlace opera con normalidad.
>
> **Configuración vigente:** 2 radios, enlace directo, **sin repetidor**.
> **En curso:** antenas VHF `136–174 MHz` para recuperar alcance — las genéricas de "LoRa" costaban
> 15–20 dB y dejaban la cobertura en 3 cuadras. Ver
> [`05_Funcional/2_Manual_Hardware_y_Pruebas.md §2.1`](05_Funcional/2_Manual_Hardware_y_Pruebas.md).
>
> ⚠️ **Sigue vigente:** las radios deben estar a **`2.4 kbps`** de Air Data Rate y con `M0`/`M1`
> **ambos en OFF** durante la operación. Ver
> [`05_Funcional/4_Manual_Configuracion_Radios.md`](05_Funcional/4_Manual_Configuracion_Radios.md).

---

> ## 🔧 Banco del 1 de Agosto de 2026 — tres fallos que solo aparecen con la tarjeta delante
>
> Ninguno se veía en simulación, y los tres se encontraron en cadena: cada arreglo destapó el
> siguiente. La pantalla del **Esclavo** sigue abierta.
>
> ### `N-17` · El arranque ya no depende de que oscile el cristal del reloj ✅
>
> `rtc.begin()` con LSE espera al oscilador **sin límite**. Si el cristal `Y2` de 32.768 kHz no
> arranca — y `MAPEO_TARJETA_KICAD.md §4` ya avisaba de que en micros clonados el condensador de
> carga viene mal calculado — el arranque se queda ahí para siempre.
>
> **El mismo cuelgue daba dos síntomas que no se parecían en nada:** el Maestro quedaba **en bucle
> en la bienvenida** (su `lcd_setup()` va antes), y el Esclavo **en blanco pero con las luces
> funcionando** (el suyo iba después). Uno se leía como problema de pantalla y el otro como de
> arranque; la causa era la misma línea.
>
> Ahora el oscilador se arranca a mano con espera **acotada a 2 s**, por debajo de los 4 s del
> watchdog. Si no responde, **el equipo arranca sin reloj** y quien dependa de la hora se abstiene.
> *Un semáforo no puede depender de un cristal de reloj para encender.*
>
> ### `N-26` · Un botón ya pulsado al encender ya no ejecuta una maniobra ✅
>
> En cuanto `N-17` dejó arrancar al Maestro, este se plantaba solo en la configuración del Modo
> Manual sin que nadie tocara la botonera. `botones_setup()` declaraba los cuatro pines pero
> **nunca los leía**: todo arrancaba en *"ningún botón pulsado"* aunque el pin estuviera en LOW, y
> la primera vuelta del `loop()` lo interpretaba como un flanco.
>
> **El Botón 3 EJECUTA.** Subir o bajar no rompe nada, pero un `ACEPTAR` fantasma arranca un modo
> que nadie pidió — y en un semáforo eso es una maniobra. Además los pulsadores van **en paralelo
> con el mando de relés**: un relé cerrado en reposo o el ruido en los 5 m de cable dejan el pin en
> LOW sin ningún dedo de por medio.
>
> Ahora se siembra el estado real de cada pin: **un botón pulsado al encender es un estado, no una
> pulsación**, y no dispara hasta que se suelte y se vuelva a pulsar.
>
> ### `N-23` · Poner el reloj ya no es lo mismo que sincronizar ✅
>
> Al fijar la hora y entrar al Modo Degradado, este rechazaba con **`Falta: nunca hubo
> sincronización RF`** — con los radios enlazados al 100% y el automático funcionando. El mensaje
> parecía mentir y no lo hacía.
>
> `modo_hora.cpp` llamaba a `coordinador_sincronizarHora()`, que solo **encola** el envío, y en la
> línea siguiente se iba al menú. Y **en el menú nadie llama a `coordinador_actualizar()`**: solo lo
> hacen automático, manual, inteligente, `PRUEBA ALCANCE` y esa misma pantalla. La petición quedaba
> encolada y **las tramas no salían nunca**.
>
> Y no se guardaba nada en **ninguna** de las dos puntas: la marca de sincronización se graba en un
> único punto del firmware, al recibir el `CMD_ACK_HORA` del Esclavo. Sin tramas no hay ACK, y sin
> ACK no hay marca ni en RAM ni en la pila.
>
> Ahora la pantalla **se queda moviendo el coordinador hasta que el Esclavo acusa la hora**, y dice
> cuál de las dos cosas se consiguió: `SINCRONIZADA`, o `SOLO MAESTRO / Esclavo no responde / No
> habrá Degradado`. El reloj del Maestro queda puesto en ambos casos, y la pantalla lo dice para que
> nadie repita el ajuste creyendo que no se guarda nada.
>
> ### `N-37` · El cristal `Y2` está MUERTO ⛔ **CERRADO POR ELIMINACIÓN**
>
> Cada sospechoso cayó con una medida, no por descarte perezoso: **`VBAT` = 3 V con la tarjeta
> APAGADA** (si `R5` siguiera puesto estaría atado al riel de 3,3 V, que apagado es 0 V — prueba de
> golpe que `R5` está fuera y que la pila alimenta), el **reintento de 30 s** de `N-25` (descarta
> *"lento"*), y **`REINICIAR RELOJ`** de `N-31`, que devolvió `SIGUE PARADO` (descarta registros
> sucios del dominio de respaldo tras los arranques colgados de `N-17`).
>
> **Ya no queda software que probar.** ➡️ **`DS3231`** por I²C software en `PB0`/`PB8`. Hacen falta
> **dos**: el Esclavo casi seguro tiene lo mismo y con su pantalla muerta no hay forma de
> comprobarlo. ⚠️ Lleva **LIR2032 recargable**, nunca la CR2032.
>
> Sin reloj no hay sincronización, y sin ella el Degradado rechaza. **Eso no es un defecto: es la
> guarda funcionando.**
>
> ### `N-22` · Pantalla del Esclavo en azul y sin píxeles ⏳ **ABIERTO**
>
> Comparado llamada por llamada contra el Maestro, que sí pinta: `pines.h` es **byte a byte
> idéntico** (`PB3`…`PB7`), el constructor `U8G2_ST7920_128X64_F_SW_SPI` lleva los mismos cuatro
> pines, y `lcd_setup()` y `lcd_dibujarBienvenida()` son iguales hasta la coma. Solo difería el
> **orden**, y ya está igualado — incluido el `delay(2000)` que el Maestro tenía y esta punta no.
>
> **No se espera que eso lo arregle**, y conviene decirlo: la versión anterior tenía `lcd_setup()`
> en última posición — el máximo tiempo posible de reposo antes de tocar el módulo — y también salía
> en blanco. Si los dos extremos fallan igual, el momento de la inicialización no es la variable.
> Se igualó por lo que permite **concluir**: con el camino idéntico, el software queda descartado.
>
> **Dónde mirar:** la luz azul **no prueba que la lógica esté alimentada** — la retroiluminación va
> por `A/K` (19-20) y la lógica por **`VDD` (pin 2)**. Después `PSB`→`PB6` debe medir **0 V** y
> `RST`→`PB7` debe medir **3,3 V**. Si los cuatro miden bien, pasar el arnés completo del Maestro a
> esa tarjeta: módulo y cable juntos, los dos ya probados.
>
> ### Carga por SWD: `mode=UR`, y no se cambia
>
> `HOTPLUG` se engancha al micro **en marcha**. Con un firmware que se cuelga al arrancar, el
> watchdog reinicia cada 4 s **en mitad del borrado**: `failed to erase memory`. El delator es
> `NVM size: 128 KBytes (default)` en un chip de 64 KB. `UR` retiene el micro en reset y funciona en
> los **dos** escenarios; `HOTPLUG` solo en uno.
>
> ⚠️ **Si `UR` falla, reintenta — no cambies el modo.** Enganchar es cuestión de *timing* y puede
> fallar varias veces con `Unable to get core ID`. Eso **no** es falta de cableado: `NRST` está
> conectado, y lo prueba el `DEV_TARGET_HELD_UNDER_RESET` que devolvió uno de los intentos.

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
| **7. Modo Manual - Botón 3 (OK)** | 🔴 ROJO FIJO Continuo | 🔴 ROJO FIJO Continuo | **ROJO FIJO INDEFINIDO** en ambos nodos hasta pulsar Botón 1 o 2. |
| **8. Transición Verde a Rojo** | 🔴 Rojo Directo (0s) | 🔴 Rojo Directo (0s) | Cumplimiento estricto Resolución 2024 (0s de pre-aviso). |
| **9. Transición Rojo a Verde** | 🟡 Amarillo Fijo (4.0s) | 🟡 Amarillo Fijo (4.0s) | 4.0 segundos de aviso previo para despeje de camiones pesados. |
| **10. Prueba de Alcance (V8.1)** | 🔴 ROJO FIJO Continuo | 🔴 ROJO FIJO Continuo | Pantalla de diagnóstico con calidad de enlace y tiempo de respuesta. **No arranca ciclos.** |
| **11. Modo Degradado (V8.7)** | Alterna 🟢/🔴 **por reloj** | Alterna 🔴/🟢 **por reloj** | **Activación MANUAL verificada**, nunca automática. Ciclo de 30 s de verde y **30 s de todo-rojo ampliado**. Cae solo a 🟡 tras **48 h** sin resincronizar. |

> ⚠️ **El umbral de silencio de SFTY-6 son `25 s`, no `12 s`** — `SFTY6_SILENCIO_MS = 25000UL` en
> `protocolo.h:149` de **las dos puntas**. Esta tabla publicaba `12 s` en cinco filas: era el número
> vigente hasta **N-71**, que midió que aquel techo estaba **por debajo** del peor caso del ciclo de
> reintentos —20,5 s—, así que **los reintentos 4 y 5 no podían ejecutarse jamás** y nada lo
> delataba, porque el equipo hacía algo razonable: irse a ámbar. La relación entre esos tres números
> vive ahora en un pack que la recalcula desde el C++, no en un comentario.
>
> ⚠️ **Y los gestos de las filas 2, 3, 7 y 10 son de la botonera y la pantalla, que se retiran.**
> El **comportamiento vial** de las once filas —qué color se ve y durante cuánto— **no cambia**: lo
> que cambia es **quién lo pide**. La fila 7 dice *"hasta pulsar Botón 1 o 2"*: el Botón 3 (`PB14`)
> y el Botón 4 (`PB15`) pasan a **cámaras**, y los Botones 1 y 2 se quedan como canales `A`/`B` del
> mando. **Los comandos que sustituyen a esos gestos todavía no están todos escritos** — sin ellos
> se entra a un modo y no se sale.

> ### ⚠️ El escenario 1 NO cambia: al perder el radio sigue entrándose en ámbar
>
> El **Modo Degradado** (fila 11) **no sustituye** a ese comportamiento: es un **caso especial que un
> operario activa a mano**, tras verificar las dos puntas. La máquina **nunca** decide sola operar sin
> radio.
>
> **Por qué.** Un ámbar dice *"no estoy controlando esto, decide tú"* y el conductor llega **alerta**.
> Un verde dice *"pasa tranquilo, el otro lado está en rojo"* y llega **confiado**. Sin radio nadie
> puede confirmar que la otra punta siga viva, así que **un verde equivocado es más peligroso que un
> ámbar ambiguo**. Con activación manual, el verde deja de darse por suposición y pasa a darse porque
> **una persona verificó ambos extremos**.
>
> El análisis completo, incluidos los riesgos residuales aceptados, está en
> [`OPTIMIZACIONES.md`](OPTIMIZACIONES.md) §SFTY-21.

---

## 🗂️ ~~Estructura del menú (V8.7)~~ — **RETIRADO con la pantalla el 28/08**

> 🛑 **Esto ya no es lo que se monta.** La pantalla LCD se retira de las dos puntas —**este PCB no
> permite ampliación** y sus cinco pines hacían falta para el Bluetooth, ver
> [🧭 La arquitectura vigente](#-la-arquitectura-vigente-decidida-en-obra-el-2808-y-cerrada-el-3108)—,
> y con ella se va el menú entero. **Toda la operación pasa a la app.**
>
> **Se conserva escrito, no se borra**, por tres razones que siguen sirviendo: es la descripción de
> lo que **corre hoy en campo (V8.4)**; el arnés de pantalla sigue midiéndolo en la tabla de arriba
> mientras el código exista; y el criterio de los 64 px es el que explica por qué la app tampoco
> puede esconder una opción donde el cursor sí llega.
>
> 🔴 **Lo que hay que leer antes de dar el menú por prescindible:** `botonCancelar()` es hoy **la
> única salida de los ocho modos**, y desde Bluetooth sólo se alcanzan tres, sin vuelta al menú.
> **`SET_MODO:MENU` va antes de retirar nada**, no después.

Dos niveles, porque una lista plana ya no cabía en los 64 px de alto:

```
   MENÚ PRINCIPAL                 CONFIGURACIÓN
   ┌──────────────────┐           ┌──────────────────┐
   │ > MANUAL         │           │ > PRUEBA ALCANCE │
   │   AUTOMATICO     │    -->    │   AJUSTAR HORA   │
   │   INTELIGENTE    │           │   MODO DEGRADADO │
   │   CONFIGURACION  │           │   REINICIAR RELOJ│
   └──────────────────┘           └──────────────────┘
```

**No fue un apaño de espacio.** Los tres primeros son **modos de operación** que el operario elige a
diario; los cuatro de la derecha, **herramientas y casos especiales** que se tocan rara vez.
Mezclarlos en una lista plana era la causa del problema, no el número de opciones.

`REINICIAR RELOJ` la añadió **N-31** y sólo la busca quien ya sabe que el reloj no arranca. Cabe
sin comprimir nada: con 4 opciones el interlineado sigue siendo el de 11 px validado en campo, y
la última cae en `y=61` de los 63 disponibles. **El límite no es estético** — en la V8.6 una sexta
línea caía en `y=69` y el peligro no era que no se dibujara, sino que **el cursor sí podía navegar
hasta ella**, dejando al operario en una opción invisible.

**El Esclavo también tiene pantalla y menú propios** desde la V8.7 (`ESTADO` y `MODO DEGRADADO`), que
antes no existían en firmware. **Sin ajuste de hora**: la hora llega por radio desde el Maestro.

---

## 🎛️ Mando de relés — operación desde el suelo, **y se conserva** (decidido el 31/08)

El equipo está **a 5 m de altura**: el operario acciona desde abajo **sin ver nada**. Por eso la
confirmación se da en **destellos ROJOS contables** — la única realimentación que le llega desde el
suelo. Retirado el menú, este mando pasa de comodidad a **la última superficie de mando física que
queda si el ESP32 se cuelga**, y por eso se conserva: en sus dos canales `A` y `B`, `PB9` y `PB13`.

**Lo que el firmware hace HOY** — leído de `Maestro/src/mando.cpp:198-236` y de
`Esclavo/src/mando.cpp:213-250`, no de la especificación:

| Secuencia | Acción en el Maestro | Acción en el Esclavo | Confirmación |
|---|---|---|---|
| **`A · A · A`** (≤12 s) | 🟢 **Modo Automático** | obedecer al Maestro *(sale del ámbar local)* | 2 destellos rojos |
| **`B · B · B`** (≤12 s) | 🟡 **Modo Ámbar**, sin condiciones y desde cualquier modo | 🟡 **ámbar local — y desobedece al Maestro a propósito** | 3 destellos rojos |
| **`A · B · A · B`** (≤18 s) | 🕒 **Modo Degradado**, *sólo si la hora está validada* | 🕒 **Modo Degradado** | 4 destellos rojos |

> ⚠️ **La tabla anterior de este README publicaba cinco secuencias —`A·B·A`, `B·A·B`, `B·A·B·A`,
> `A·B·A·B`, `A·A·B·B`— y sólo la de Degradado existe en el firmware.** Aquella era la
> **redefinición anti-colisión propuesta en la spec de V9.0**, escrita en pasado como si estuviera
> hecha; lo que sí se implementó de N-53 es `secuenciasInhibidas()` en las dos puntas. No hay
> secuencia de Manual ni de Inteligente: no existen `DESTELLOS_MANUAL` ni `DESTELLOS_INTELIGENTE`.
> Queda abierto como **`FW-N53`** en [`ESTADO.md`](ESTADO.md) — y es **decisión de spec**, porque
> cambiar los gestos cambia el Manual 1, el Manual 3 y el adiestramiento del operario.

> 🔴 **`B·B·B` no es un modo más: es el veto de SFTY-21.** En el Esclavo arma `ambarLocal`, y esa
> bandera tiene **tres consumidores negados** en `Esclavo/src/main.cpp` (`:406`, `:416`, `:540`):
> mientras el ámbar lo pidió una persona desde el suelo, **una orden de radio no saca a esa punta
> del ámbar**, el Maestro agota reintentos y el cruce entero termina en ámbar — que es exactamente
> lo que se pidió. Es la razón medida por la que se conservan **los dos** canales y no uno.

> 📱 **Y lo que ya no es cierto:** ~~*"gracias al módulo Bluetooth instalado en `USART1`"*~~ — el
> puerto sigue siendo el `USART1`, pero **remapeado a `PB6`/`PB7` con salida por `J17`** (N-76) y
> con un **ESP32** al otro lado en vez del módulo SPP. La consulta de estado, alarmas y operación
> del Esclavo desde el celular (N-19) sigue siendo la vía prevista; **no ha pasado banco**, y lo
> único que hoy la ejercita son arneses que no tocan la tarjeta.

---

## 🗂️ Paquete Oficial de Entregables y Manuales (`05_Funcional/` en Word .docx y Markdown .md)

1. 📄 [`05_Funcional/1_Manual_Usuario.docx`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/1_Manual_Usuario.docx) — Manual de Operación y Comportamiento Vial (Resolución 2024).
2. 📄 [`05_Funcional/2_Manual_Hardware_y_Pruebas.docx`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/2_Manual_Hardware_y_Pruebas.docx) — Manual de Hardware, Borneras y Ensamblaje STM32.
3. 📄 [`05_Funcional/3_Protocolo_Pruebas_Rigurosas.docx`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/3_Protocolo_Pruebas_Rigurosas.docx) — Protocolo de Auditoría Funcional (80 pruebas).
4. 📄 [`05_Funcional/4_Manual_Configuracion_Radios.docx`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/4_Manual_Configuracion_Radios.docx) — Configuración Radios E90-DTU (2.4 kbps / 30 dBm / FEC).
5. 📄 [`05_Funcional/5_Manual_Puente_ESP32.docx`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/5_Manual_Puente_ESP32.docx) — Puente Repetidor ESP32 V7.6 para curvas ciegas.
6. 📄 [`05_Funcional/6_Preguntas_Diseno_Funcional.docx`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/6_Preguntas_Diseno_Funcional.docx) — Decisiones de Diseño Cerradas y Aprobadas.
7. 📄 [`05_Funcional/7_Especificacion_Antenas.docx`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/7_Especificacion_Antenas.docx) — Especificación de Antenas y Línea de Vista.
8. 📄 [`05_Funcional/8_Procedimiento_Modo_Degradado.docx`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/8_Procedimiento_Modo_Degradado.docx) — Operación de Emergencia por Reloj sin Radio.
9. 📄 [`05_Funcional/9_Manual_Parametrizacion_Camara_IA.docx`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/9_Manual_Parametrizacion_Camara_IA.docx) — ~~Sistema de 4 Cámaras IA Hikvision AcuSense~~ → **2 cámaras de demanda**, y el pinout se muda a `J16`. ⚠️ **manual sin corregir.**
10. 📄 [`05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.docx`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.docx) — Módulo Bluetooth y App Móvil Baliza. ⚠️ **manda enchufar un `HC-05` en `J17`, que es donde va el ESP32.** Su apartado 1 —*"Bluetooth Clásico SPP, no BLE"*— sigue **congelado por escrito**: se reabre sólo si el chip no es clásico.
11. 📄 [`05_Funcional/11_Manual_Instalacion_RTC_DS3231_Bateria.docx`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/11_Manual_Instalacion_RTC_DS3231_Bateria.docx) — ~~Instalación Reloj DS3231 TCXO en `PB0`/`PB8`~~ → **el `DS3231` se muda al ESP32** (`GPIO21`/`GPIO22`, pila propia). ⚠️ **manual sin corregir.**
12. 📄 [`05_Funcional/12_Cobertura_de_Pruebas_y_Huecos.docx`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/12_Cobertura_de_Pruebas_y_Huecos.docx) — Qué mide cada instrumento y **qué queda sin medir**.
13. 📄 [`05_Funcional/13_Manual_Modulo_Expansion_I2C_y_Compras.docx`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/13_Manual_Modulo_Expansion_I2C_y_Compras.docx) — ~~Bus I²C en `PB0`/`PB8`: `PCF8574` + `DS3231`~~ y lista de compras. ⚠️ **su §4 entera queda sin sujeto**: el I²C ya no vive en el STM32, así que no hay que sacar bus de `PB0`/`PB8` ni modificar la tarjeta.
14. 📄 [`05_Funcional/14_Manual_App_Movil_IOT_VIAL.docx`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/14_Manual_App_Movil_IOT_VIAL.docx) — Operación de la App IOT-VIAL: SPP, gestor de cruces y Courier RTC.
15. 📄 [`05_Funcional/15_Lista_de_Compras_Hardware.docx`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/15_Lista_de_Compras_Hardware.docx) — **Qué se pide, cuánto y cuándo**: la compra estaba repartida en siete manuales.
16. 📄 [`05_Funcional/16_Documento_Auditoria_Arquitectura_y_Usabilidad_App_IOT_VIAL.docx`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/16_Documento_Auditoria_Arquitectura_y_Usabilidad_App_IOT_VIAL.docx) — Auditoría de arquitectura y usabilidad de la App IOT-VIAL: roles operario/técnico, transporte SPP dual y APK `SIN_BANCO`.
17. 📄 [`05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md`](05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md) — 🧭 **La arquitectura decidida en obra y lo que sigue abierto**: el reparto STM32 / ESP32, ocho hallazgos con su `fichero:linea`, cinco decisiones con dueño, **cinco medidas de multímetro** y el censo de manuales que quedan falsos. **Es el documento que manda sobre las filas 9, 10, 11 y 13 de esta lista.**

*(**La lista se ha desfasado TRES veces por lo mismo, y sigue sin instrumento.** El 27/08 anunciaba 11 manuales de los 14 que existían; el 28/08, 15 de 16 —faltaba el propio informe de auditoría de la app—; y el 31/08, 16 de **17** —faltaba el documento de arquitectura, justo el que dice que media lista ya no es cierta—. Contar los ficheros es `ls 05_Funcional/[0-9]*.md`; que nadie lo haga es lo que hace falta cerrar, igual que se cerró para las cifras del acta.)*

> ⚠️ **Y un aviso sobre los enlaces de esta lista, que no es cosmético:** las entradas 1 a 16 apuntan
> con ruta absoluta a `d:/@Proyect/Controladora_Semaforos/…` — **el directorio del repositorio
> PADRE**, que sigue existiendo en el disco. No dan error: **abren el fichero del otro árbol**, que
> es un fichero distinto. Un enlace roto se nota; uno que abre la versión equivocada, no.

> 🛑 **Del protocolo de 80 pruebas (entrada 3), 49 dejan de ser ejecutables** con la arquitectura de
> arriba —sección a sección, Manual 17 §2.8—: sobreviven 31 en principio y **sólo 16 tal como están
> redactadas hoy**. Se reescribe **el último**, no el primero: es el documento que recoge las
> consecuencias de los otros cuatro. Y va **una prueba por una**, a *borrar, invertir o conservar*,
> anotado — tacharlas en bloque hasta que la cuenta cuadre es ajustar el instrumento hasta que dé
> verde.

---

## 🛠️ Estructura del Ecosistema de Firmware

- [`01_Firmware/Maestro`](01_Firmware/Maestro): **El controlador.** Máquina de estados STM32, barrera de salidas (`semaforo.cpp`, el único que escribe pines de luz), reloj RTC (`reloj.cpp`), Modo Degradado (`modo_degradado.cpp`), mando de relés (`mando.cpp`, canales `A`/`B`) y telemetría Bluetooth (`bluetooth.cpp`, `USART1` **remapeado a `PB6`/`PB7` → `J17`**). ~~UI LCD ST7920~~ — el código sigue en el árbol y **la pantalla se retira**: ver la sección de arquitectura.
- [`01_Firmware/Esclavo`](01_Firmware/Esclavo): Nodo secundario STM32 con ACK por radio RS-485 (`USART3`), fallback de orfandad a **25 s** *(`SFTY6_SILENCIO_MS`, N-71 — decía `12s`)*, ámbar local con veto SFTY-21 (`mando.cpp`) y telemetría Bluetooth (`bluetooth.cpp`). ~~pantalla LCD~~ — igual que el Maestro.
- [`01_Firmware/Repetidor`](01_Firmware/Repetidor): Puente ESP32 que enlaza 2 radios back-to-back (`RadioA`/`RadioC`) de forma transparente. ⚠️ **Fuera de la configuración vigente** —2 radios en enlace directo, sin repetidor— y **sin watchdog**: es el mismo firmware que se quedó clavado tumbando el enlace el 31/07. **No es el ESP32 de expansión** de la arquitectura nueva; hoy tres cosas distintas se llaman «el ESP32» en los papeles.
- [`01_Firmware/Simulaciones/simulador_sistema_v7_6.py`](file:///d:/@Proyect/Controladora_Semaforos/01_Firmware/Simulaciones/simulador_sistema_v7_6.py): Banco de simulación en Python — **20/20 PASS** con barrido de 86.400 segundos.
- [`01_Firmware/Simulaciones/simulador_repetidor.py`](file:///d:/@Proyect/Controladora_Semaforos/01_Firmware/Simulaciones/simulador_repetidor.py): Escenarios de enlace de radio — **10/10 PASS**.
- [`01_Firmware/Validacion_LCD/`](01_Firmware/Validacion_LCD): Validación de pantalla ejecutada en el PC — **271/271 comprobaciones OK**. ⏳ **Mide código que se va a retirar**: cuando la pantalla salga, sus comprobaciones van una por una a *borrar, invertir o conservar* —con la cuenta comparada antes y después—, nunca en bloque.
- [`01_Firmware/compuerta.py`](01_Firmware/compuerta.py): Suite formal de verificación integral — **20 PASS · 0 FALLA · 0 ABORTADO** (Exit code 0). Las cifras están en la tabla de arriba, que se copia del acta; ésta es sólo la puerta.

> 🛑 **Y para cerrar donde se abrió: nada de este README es un permiso.** En campo corre la **V8.4**;
> la V9.0 compila y pasa la compuerta, y **no ha visto una tarjeta**. La arquitectura de arriba está
> decidida en obra y **medida sobre ficheros**, no sobre cobre. La regresión del Modo Automático
> sigue **abierta**. **Nada sube a campo sin pasar banco** — y el banco no lo sustituye ningún
> número verde de esta página.
