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
> relés anti-colisión, sistema de 4 cámaras IA Hikvision y telemetría Bluetooth estándar Baliza—
> está especificado y validado en simuladores/arneses, en la rama `feat/n15-reloj-pantalla-hora`.

> 🔴 **Regresión abierta en banco: el Modo Automático no arranca el ciclo en la rama.** No es
> radio *(`RF:` da porcentaje)* ni botón trabado *(el Botón 3 responde)*. Está en bisección con
> firmware real — entregable en `05_Funcional/bisect_entregable/`, sospechoso `2779d9b`
> *(SFTY-21, mando de relés)*. Detalle en [`roadmap.md`](roadmap.md) §N-42.
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

**Verificación actual** — cifras **copiadas del acta** `evidencia/2026-08-28_compuerta.txt`, que
genera `python 01_Firmware/compuerta.py` en una sola corrida. No se escriben a mano — y desde
**N-62** eso ya no es una promesa: el pack `documentos_01_cifras_del_acta` compara esta tabla
contra la última acta en cada corrida del banco. Cuando se escribió por primera vez, **falló**:
esta tabla publicaba 32 rutas y 86,4 % de flash cuando el acta que ella misma citaba medía 38
rutas y 92,8 %. Las cifras eran del 05/08 y llevaban la palabra *«copiadas»* encima.

| Comprobación | Estado | |
|---|---|---|
| guarda de rutas de los instrumentos | ✅ | 42 rutas parseadas, todas existen |
| banco por packs *(38 packs)* | ✅ | **405/405** — los 38 packs en `PASS`. Subió de 348 con los cuatro packs de las Fases 1 y 2: `app_02_modos_simetricos`, `app_03_sin_ok_mudo`, `esclavo_07_ambar_emergencia` y `maestro_09_test_leds`. **Los cuatro nacieron en rojo** y ninguno se dio por bueno hasta verlo caer con el defecto inyectado en el `.cpp` real |
| compila Maestro / Esclavo / Repetidor | ✅ | **88,4 %** · 64,4 % · 20,6 % — *7.628 B libres en el Maestro tras las Fases 1 y 2* |
| simulador funcional | ✅ | 20/20 |
| simulador de repetidor | ✅ | 10/10 |
| simulador de app y bluetooth | ✅ | 5/5 — **conectado el 27/08**: existía desde el 26/08 y no estaba en el acta |
| **app ejecutada en DOM** | ✅ | **59/59** — carga `index.html` en jsdom, más `app.js` y **los `js/*.js` que el propio HTML declara, en su orden** *(desde N-75: el rewrite sacó el gestor de cruces, el parser NMEA y el Courier a módulos, y el arnés seguía evaluando sólo `app.js`)*, y los **ejercita**: pestañas, modales, ingesta de telemetría, *fuzzing* de 200 tramas corruptas y los botones que mandan comandos. Es el único instrumento que **ejecuta** la app en vez de leerla |
| test funcional de la app | ✅ | **57/57** — también conectado el 27/08. Decía «22/22» a mano y ejecuta 34; su prueba de Courier RTC era una tautología |
| test unitarios de la app | ✅ | **32/32** — seis suites sin DOM: NMEA y *checksums*, generador de comandos y barrera de PIN, validación de `SET_TIEMPOS`, Courier RTC, gestor de cruces y escala de 20 cruces. **Faltaba en esta tabla hasta el 28/08**: el acta lo medía y el README no lo nombraba, así que el auditor no tenía forma de saber que existía |
| arnés de pantalla | ✅ | **271/271** *(Maestro 145/145, Esclavo 126/126)* — +12 al exigir el texto exacto del aviso `>48h` (N-50) |
| arnés del ciclo | ✅ | **29/29** — corre sobre el `ciclo_degradado.h` real compilado, sin espejo en Python |
| arnés del respaldo | ✅ | **conectado por fin** (N-43/N-29) — compila el `calcularSuma()` real; identidad de `respaldo.cpp` entre puntas + prueba de vida |
| arnés del automático | ✅ | **71/71** — compila `coordinador.cpp` + `semaforo.cpp` + `modo_automatico.cpp` **reales** y comprueba SFTY-2 sobre las escrituras de pin |

**15 PASS · 0 FALLA · 0 ABORTADO, de 15 comprobaciones.** — la compuerta sale con código `0`.

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

> ### 📦 El banco son 34 packs — la migración terminó el 05/08, y creció con V9.0
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
> `banco/modelos` — 42 rutas, ninguna escrita a mano.

> ⚠️ **`ABORTADO` no es `PASS`.** Una comprobación que no pudo correr no dice *nada* del firmware.
> Así se perdió la cobertura del Maestro sin que nadie se enterara: `validador_maestro.py` llevaba
> días abortando en silencio tras reescribirse el checksum de `respaldo.cpp`, y desde fuera parecía
> que corría. `compuerta.py` (**N-28**) existe para que esa clase de fallo sea imposible de pasar por
> alto: un único exit code y las tres palabras separadas en el resumen.

> ⚠️ **Nada de esto sustituye la prueba de banco.** Los simuladores son modelos escritos a mano: no
> compilan ni ejecutan el C++, y **validan el modelo, no el código**. El arnés de pantalla sí compila
> los `lcd.cpp` y `menu.cpp` reales, pero contra un framebuffer en el PC, no contra la ST7920.

**Certificado en campo:** 31 de Julio de 2026 *(V8.4, dos radios en enlace directo)*  
**Última actualización del repositorio:** 28 de Agosto de 2026 *(la cifra vigente y su hash de HEAD están en el acta que cita la tabla de arriba —`evidencia/2026-08-28_compuerta.txt`—, y no se repiten aquí: este pie llevaba `14 PASS` sobre HEAD `2cde016` cuando el acta ya medía otra cosa, y un recuento viejo no se lee como viejo, se lee como medida. **Nada de esto ha pasado banco**: siguen abiertas la regresión N-42 del Modo Automático, el cristal N-37, y las cuatro funciones de V9.0 que ningún PC puede validar)*
**Repositorio Oficial:** [https://github.com/dieleoz/2semaforos_3estados.git](https://github.com/dieleoz/2semaforos_3estados.git)  
**Normativa Aplicable:** Resolución 2024 del Ministerio de Transporte de Colombia (Secuencia de Luces y Tiempos de Seguridad Vial)

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
| **1. Sin Comunicación / Pérdida de Enlace** | 🟡 Amarillo Intermitente (1Hz) | 🟡 Amarillo Intermitente (1Hz) | Entrada a fallo de seguridad tras **12.0s** sin PONG/PING. |
| **2. Menú Principal (Con comunicación)** | 🔴 ROJO FIJO Continuo | 🔴 ROJO FIJO Continuo | Menú LCD 100% fluida con re-refuerzo de Rojo Fijo. |
| **3. Menú Principal (Sin comunicación)** | 🟡 Amarillo Intermitente (1Hz) | 🟡 Amarillo Intermitente (1Hz) | Detección de orfandad en Menú a los 12.0s. |
| **4. Apagado del Esclavo** | 🟡 Amarillo Intermitente (12s) | Off / Sin Batería | Maestro detecta orfandad a los 12s y entra a fallo de seguridad. |
| **5. Apagado del Maestro** | Off / Sin Batería | 🟡 Amarillo Intermitente (12s) | Esclavo detecta orfandad a los 12s y entra a fallo de seguridad. |
| **6. Restablecimiento (Self-Healing)** | 🔴 Rojo Fijo (15s All-Red) | 🔴 Rojo Fijo (15s All-Red) | **RECONEXIÓN AUTÓNOMA SIN REINICIAR NINGUNA TARJETA**. |
| **7. Modo Manual - Botón 3 (OK)** | 🔴 ROJO FIJO Continuo | 🔴 ROJO FIJO Continuo | **ROJO FIJO INDEFINIDO** en ambos nodos hasta pulsar Botón 1 o 2. |
| **8. Transición Verde a Rojo** | 🔴 Rojo Directo (0s) | 🔴 Rojo Directo (0s) | Cumplimiento estricto Resolución 2024 (0s de pre-aviso). |
| **9. Transición Rojo a Verde** | 🟡 Amarillo Fijo (4.0s) | 🟡 Amarillo Fijo (4.0s) | 4.0 segundos de aviso previo para despeje de camiones pesados. |
| **10. Prueba de Alcance (V8.1)** | 🔴 ROJO FIJO Continuo | 🔴 ROJO FIJO Continuo | Pantalla de diagnóstico con calidad de enlace y tiempo de respuesta. **No arranca ciclos.** |
| **11. Modo Degradado (V8.7)** | Alterna 🟢/🔴 **por reloj** | Alterna 🔴/🟢 **por reloj** | **Activación MANUAL verificada**, nunca automática. Ciclo de 30 s de verde y **30 s de todo-rojo ampliado**. Cae solo a 🟡 tras **48 h** sin resincronizar. |

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

## 🗂️ Estructura del menú (V8.7)

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

## 🎛️ Mando de 4 relés — operación desde el suelo (V9.0 Anti-Colisión N-53)

La pantalla está **a 5 m dentro del gabinete**: el operario acciona desde abajo **sin verla**. Por eso
la confirmación se da en **destellos ROJOS contables** y se utilizan secuencias con alternancia
para evitar colisiones con la edición de parámetros en la pantalla LCD:

| Secuencia | Acción | Confirmación Lumínica |
|---|---|---|
| **`A · B · A`** (≤12s) | 🟢 **Modo Automático** — *"ciclo programado estándar"* | 2 destellos rojos |
| **`B · A · B`** (≤12s) | 🟡 **Modo Ámbar** — *"salida de seguridad / precaución"* | 3 destellos rojos |
| **`B · A · B · A`** (≤18s) | ✋ **Modo Manual** — *"operario habilita turnos"* | 5 destellos rojos |
| **`A · B · A · B`** (≤18s) | 🕒 **Modo Degradado** — *"reloj RTC sincronizado"* | 4 destellos rojos |
| **`A · A · B · B`** (≤18s) | 📷 **Modo Inteligente** — *"demanda 4 Cámaras IA"* | 6 destellos rojos |

> 📱 **Operación desde el Suelo en el Esclavo (Baliza):** Gracias al módulo Bluetooth instalado en `USART1`,
> el operario puede consultar el estado, ver alarmas y operar el Esclavo desde el celular sin necesidad
> de subir al poste a 5 metros de altura (resolviendo el pendiente N-19).

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
9. 📄 [`05_Funcional/9_Manual_Parametrizacion_Camara_IA.docx`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/9_Manual_Parametrizacion_Camara_IA.docx) — Sistema de 4 Cámaras IA Hikvision AcuSense.
10. 📄 [`05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.docx`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.docx) — Módulo Bluetooth y App Móvil Baliza.
11. 📄 [`05_Funcional/11_Manual_Instalacion_RTC_DS3231_Bateria.docx`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/11_Manual_Instalacion_RTC_DS3231_Bateria.docx) — Instalación Reloj DS3231 TCXO en `PB0`/`PB8`.
12. 📄 [`05_Funcional/12_Cobertura_de_Pruebas_y_Huecos.docx`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/12_Cobertura_de_Pruebas_y_Huecos.docx) — Qué mide cada instrumento y **qué queda sin medir**.
13. 📄 [`05_Funcional/13_Manual_Modulo_Expansion_I2C_y_Compras.docx`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/13_Manual_Modulo_Expansion_I2C_y_Compras.docx) — Bus I²C en `PB0`/`PB8`: `PCF8574` + `DS3231` y lista de compras.
14. 📄 [`05_Funcional/14_Manual_App_Movil_IOT_VIAL.docx`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/14_Manual_App_Movil_IOT_VIAL.docx) — Operación de la App IOT-VIAL: SPP, gestor de cruces y Courier RTC.
15. 📄 [`05_Funcional/15_Lista_de_Compras_Hardware.docx`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/15_Lista_de_Compras_Hardware.docx) — **Qué se pide, cuánto y cuándo**: la compra estaba repartida en siete manuales.
16. 📄 [`05_Funcional/16_Documento_Auditoria_Arquitectura_y_Usabilidad_App_IOT_VIAL.docx`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/16_Documento_Auditoria_Arquitectura_y_Usabilidad_App_IOT_VIAL.docx) — Auditoría de arquitectura y usabilidad de la App IOT-VIAL: roles operario/técnico, transporte SPP dual y APK `SIN_BANCO`.

*(**La lista se ha desfasado dos veces por lo mismo, y sigue sin instrumento.** El 27/08 anunciaba 11 manuales de los 14 que existían; el 28/08, 15 de 16 —faltaba el propio informe de auditoría de la app—. Contar los ficheros es `ls 05_Funcional/[0-9]*.md`; que nadie lo haga es lo que hace falta cerrar, igual que se cerró para las cifras del acta.)*

---

## 🛠️ Estructura del Ecosistema de Firmware

- [`01_Firmware/Maestro`](file:///d:/@Proyect/Controladora_Semaforos/01_Firmware/Maestro): Control de máquina de estados STM32, UI LCD ST7920, reloj RTC (`reloj.cpp`), Modo Degradado (`modo_degradado.cpp`), mando de relés anti-colisión (`mando.cpp`) y telemetría Bluetooth (`bluetooth.cpp` en `USART1`).
- [`01_Firmware/Esclavo`](file:///d:/@Proyect/Controladora_Semaforos/01_Firmware/Esclavo): Nodo secundario STM32 con ACK por radio RS-485 (`USART3`), fallback de orfandad a 12s, pantalla LCD y telemetría Bluetooth (`bluetooth.cpp`).
- [`01_Firmware/Repetidor`](file:///d:/@Proyect/Controladora_Semaforos/01_Firmware/Repetidor): Puente ESP32 que enlaza 2 radios back-to-back (`RadioA`/`RadioC`) de forma transparente.
- [`01_Firmware/Simulaciones/simulador_sistema_v7_6.py`](file:///d:/@Proyect/Controladora_Semaforos/01_Firmware/Simulaciones/simulador_sistema_v7_6.py): Banco de simulación en Python — **20/20 PASS** con barrido de 86.400 segundos.
- [`01_Firmware/Simulaciones/simulador_repetidor.py`](file:///d:/@Proyect/Controladora_Semaforos/01_Firmware/Simulaciones/simulador_repetidor.py): Escenarios de enlace de radio — **10/10 PASS**.
- [`01_Firmware/Validacion_LCD/`](file:///d:/@Proyect/Controladora_Semaforos/01_Firmware/Validacion_LCD): Validación de pantalla ejecutada en el PC — **271/271 comprobaciones OK**.
- [`01_Firmware/compuerta.py`](file:///d:/@Proyect/Controladora_Semaforos/01_Firmware/compuerta.py): Suite formal de verificación integral — **15 PASS · 0 FALLA · 0 ABORTADO** (Exit code 0). Las cifras están en la tabla de arriba, que se copia del acta; ésta es sólo la puerta.
