# ⚡ Matriz de Optimizaciones y Reglas de Seguridad (V8.7)

**Este fichero es el catálogo propietario de las 29 reglas `SFTY-x` y su trazabilidad
regla → código → prueba** (`CLAUDE.md` §12), y no hay otro. **No es la crónica de cómo se descubrió
nada:** las 2.310 líneas que tenía el 12/09/2026 están íntegras en
[`roadmap_hist.md`](roadmap_hist.md), anexo *«`OPTIMIZACIONES.md` íntegro, la foto del 12/09»*.
**Lo decidido manda desde [`DECISIONES.md`](DECISIONES.md); el hardware medido, desde
`05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md`. Los dos GANAN a este fichero.**

**Ecosistema:** Firmware STM32 + Repetidor ESP32 + Radio LoRa E90-DTU.
**Velocidades, que no son la misma:** el puerto serie al módulo va a **9600 bps**
(`Bus.begin(9600)`, `*/src/protocolo.cpp`); la **tasa aérea** es de **2,4 kbps**, que es la que
determina el coste de la ráfaga de SFTY-11 y el peor caso de reintentos de SFTY-7.

---

## 📌 Reglas de Seguridad Inquebrantables (SFTY-1 a SFTY-18)

- **SFTY-1:** Watchdog Timer IWDG activo a **4.0s** en Maestro y Esclavo STM32 (`IWatchdog.begin(4000000)`), con refresco obligatorio en `loop()`. *El Repetidor ESP32 no implementa watchdog.*
- **SFTY-2:** Enclavamiento por hardware/software que prohíbe luz Verde y Rojo simultáneas en la misma cara.
- **SFTY-3:** Suma de verificación polinomial **CRC-8 Maxim (`0x31`)** en todos los paquetes RF.
- **SFTY-4:** Lógica de despeje **All-Red (Rojo Fijo en ambos semáforos)** con tiempo configurable de **10 a 90 s**. El piso de **10 s** es inquebrantable por software —`modoAutomatico_fijarTiempos()` hace `return false` fuera de rango, venga del menú o de la radio— así que no es posible configurar despeje nulo. El par vive una sola vez, en `Maestro/src/modo_automatico.cpp:34` (`DESPEJE_SEG_MIN = 10, DESPEJE_SEG_MAX = 90`), y lo releen `documentos_04_cifras_sin_vigilante` y el arnés del automático en cada corrida. *Estuvo publicado aquí como ~~«5 a 999s», con piso de 5s~~ hasta el 31/08/2026: el piso era la mitad del real, el techo once veces el real, y **999 no cabía siquiera en el `uint8_t` que transporta el valor** — ninguna versión del firmware pudo aceptarlo nunca.*
- **SFTY-5:** Transición de luz legal en Colombia (Res. 2024): Verde $\rightarrow$ Rojo Directo (0s); Rojo $\rightarrow$ Amarillo Fijo (4.0s) $\rightarrow$ Verde.
- **SFTY-6:** Timeout de fallback a **25.0s** sin PONG/PING para entrar a **🟡 Amarillo Intermitente** en ambos lados. El umbral vive una sola vez, en `*/include/protocolo.h` (`SFTY6_SILENCIO_MS = 25000UL`, idéntico en las dos puntas). *Estuvo publicado aquí como **12.0s** hasta el 31/08/2026: era el valor anterior a **N-71**, y esa cifra es justamente la que no cabía por encima de los reintentos.*
- **SFTY-7:** Reintento automático de órdenes ACK cada **3.5s** (`TIMEOUT_ACK_MS`), **validado en campo el 31/07/2026** con la tasa aérea a 2.4 kbps. El fallback de seguridad de 25.0s (SFTY-6) es el **techo** de la ventana, y la cuenta que lo sostiene sale del C++: los **5 reintentos** de `CICLO_MAX_REINTENTOS` a 3.5s (≈3.56s con el aire de la ráfaga), más los 3.0s de cadencia del latido, dan **~20,8s de peor caso** bajo un techo de 25.0s. **Con el techo de 12.0s que se publicaba antes de N-71 sólo cabían 2 o 3: los reintentos 4 y 5 eran código muerto**, porque el ámbar por orfandad saltaba primero. La desigualdad la recalcula `costura_09_presupuesto_radio` desde las constantes en cada corrida, en vez de vivir en esta frase. *No se sube "por si acaso" ante mayor distancia: la distancia aumenta la probabilidad de pérdida, no la latencia, y contra una trama perdida sirve repetir, no esperar.*
- **SFTY-8:** Repetidor ESP32 asíncrono. *Desde V8.3 la liberación del bus ya no es por ventana de silencio de 5 ms sino inmediata al terminar cada trama — ver SFTY-16.*
- **SFTY-9:** **Self-Healing Autónomo**: Reconexión de red sin reinicio manual, antecedida por 15s de All-Red de seguridad.
- **SFTY-10:** Ventana deslizante (`memmove`) para rescatar paquetes RF en ambientes con ruido eléctrico o pérdida de bytes.
- **SFTY-11:** Transmisión en ráfaga configurable vía `RF_BURST_COPIES` (`protocolo.h`), **fijada en 3 copias**. El coste de la ráfaga lo determina la **tasa aérea**, no el protocolo: a 2.4 kbps son ~0.13s de aire (despreciable), mientras que a 0.3 kbps eran ~2.2s y ahí sí saturaban el canal. Con la tasa corregida la redundancia vuelve a ser barata, y es **la palanca correcta frente a distancias variables** en equipos móviles.
- **SFTY-12:** **Navegación de Menú Independiente**: Mantiene a Maestro y Esclavo en **🔴 ROJO FIJO CONTINUO** (o Amarillo Intermitente sin coms).
- **SFTY-13:** **Supresión Anti-Colisión de PING**: Suprime el Heartbeat PING durante espera de ACK para evitar colisiones RS485.
- **SFTY-14 (V8.1):** **Telemetría de calidad de enlace**. Sobre el latido de 3 s que ya existe se mide si hubo respuesta y cuánto tardó, en ventana deslizante de 10 latidos. Se expone en los modos de operación (`RF:100% 340ms`) y en la pantalla **PRUEBA ALCANCE**. Solo se acepta como respuesta el comando que corresponde (`PONG` a un `PING`, `ACK_RED` a un `GO_RED`): aceptar cualquier paquete falsearía la medida al alza. No requiere soporte de la radio ni cambios de protocolo.
- **SFTY-15 (V8.3):** **Diagnóstico de línea**. `protocolo.cpp` cuenta bytes recibidos, tramas válidas y tramas descartadas por CRC. La pantalla **PRUEBA ALCANCE** los muestra en su fila inferior, separando tres fallos que antes se veían todos como "no hay comunicación": `RX 0 - nada llega` (cobertura, canal o antena), `RX 4k - BASURA` (llegan bytes pero ninguna trama válida: cableado, línea flotando o radio atascada) y `RX 36 9 tr` (enlace correcto). Los contadores se ponen a cero al entrar a la pantalla.
- **SFTY-16 (V8.3):** **Puente que valida antes de retransmitir**. El repetidor ESP32 dejó de ser un passthrough ciego: ahora reconoce el formato (4 bytes con CRC-8 Maxim) y **solo relaya tramas válidas**. Si el par RS485 de entrada queda flotando, el ruido se descarta dentro del ESP32 y no llega al aire. Antes, ese ruido mantenía la transmisión permanentemente activa y la radio de salida saturaba el canal (fallo de campo del 31/07: LED TX fijo en B2). Además la transmisión solo se activa cuando hay algo real que enviar, no ante el primer byte. Compilar con `-D PUENTE_TRANSPARENTE` revierte al comportamiento anterior.
- **SFTY-17 (V8.4):** **Retardo de cortesía del Esclavo antes de responder** (`RETARDO_RESPUESTA_MS = 200`). En modo repetidor hay una radio intermedia (B2) que acaba de **transmitir** la orden y necesita tiempo para volver a **recepción**. Si el Esclavo contesta de inmediato, su respuesta sale mientras B2 sigue conmutando y **B2 no la oye**: el enlace funciona en un sentido y no vuelve nada. Observado en campo el 31/07 con el contador del puente marcando `C<-Esclavo = 1 byte` en dos minutos mientras la ida fluía. La respuesta se **programa**, no se bloquea el bucle, así que el parpadeo de ámbar y el watchdog siguen atendidos. En enlace directo es inofensivo: el Maestro espera hasta 3.500 ms.
- **SFTY-18 (V8.5):** **Reloj de tiempo real con hora declarada no fiable por defecto**. El Maestro usa el RTC interno del STM32 con el cristal `Y2` de 32.768 kHz que la tarjeta ya traía y una pila CR2032 en `VBAT` (ver [`03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md`](03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md) §4). No ocupa ningún pin: el I²C por hardware está copado por LCD y RS-485, así que un módulo externo habría obligado a I²C por software. **La regla de seguridad no es tener reloj, es saber cuándo no se tiene:** al ajustar la hora se escribe también un año marcador, y al arrancar `reloj_enHora()` solo devuelve `true` si ese marcador sobrevivió. Pila agotada, primera puesta en marcha o dominio de respaldo corrupto ⇒ **`false`**, y toda función que dependa de la hora debe abstenerse. Un reloj sin poner en hora que se cree válido es peor que no tener reloj: activaría la operación nocturna a deshora. ~~**V8.6:** se añade la pantalla **AJUSTAR HORA** —quinta opción del menú— que es la única vía para poner el reloj y, por tanto, el requisito previo de SFTY-20 y SFTY-21. Se edita **dígito a dígito** con el dígito activo subrayado: con un solo botón de subir, poner los minutos como valor completo costaría hasta 59 pulsaciones, y además la edición por dígitos **funciona igual con el mando de relés**, que solo entrega pulsos y no admite repetición por mantener pulsado. Se trabaja sobre una copia y solo se escribe al RTC al confirmar, de modo que entrar por error y salir con el Botón 4 no altera la hora. La pantalla **no arranca ciclos**: mantiene el mismo estado seguro que el menú.~~

> 🔴 **Sobre lo tachado de `SFTY-18`: `MODO_HORA` es INALCANZABLE, así que «la única vía para poner
> el reloj» no es ninguna vía.** `modoActual_set(MODO_HORA)` tiene **un solo armador**, dentro de un
> `if (botonAceptar())` en `Maestro/src/menu.cpp`, y `botonAceptar()` es `return false;` desde el
> 31/08 porque su pin es una entrada de cámara. **La regla de seguridad de SFTY-18 no cambia**
> —`reloj_enHora()` sigue siendo quien decide, y toda función que dependa de la hora sigue debiendo
> abstenerse cuando dice `false`—: lo que cambia es **por dónde entra la hora**, que hoy es
> `CMD:SET_RTC:<...>` / `CMD:LEER_RTC` por Bluetooth contra el ESP32 de `J17`
> (`ESP32_Expansion/src/despachador.cpp`, `Maestro/src/bluetooth.cpp`).

---

## 🔗 Trazabilidad requisito → implementación

Cada regla se localiza en el código por su etiqueta `SFTY-x`. **La tercera columna se levanta
BUSCANDO la etiqueta `# EJERCE SFTY-x` en `banco/packs/`, no escribiéndola a mano**, y lo comprueba
`documentos_02_trazabilidad_sfty` **en las dos direcciones**: ninguna etiqueta sin fila, y ninguna
fila citando un pack que no la declare. Correr una sola:
`python 01_Firmware/Simulaciones/banco/correr.py --pack <nombre>`

**Sólo se etiqueta lo que el pack ejerce de verdad.** Una regla que aparece cubierta por una prueba
que no la comprueba es **peor** que una fila vacía, porque la vacía al menos no miente.

| Regla | Dónde vive | Qué lo demuestra |
|---|---|---|
| SFTY-1 | `Maestro/src/main.cpp` · `Esclavo/src/main.cpp` | — |
| SFTY-2 | `Maestro/src/semaforo.cpp` · `Esclavo/src/semaforo.cpp` | ✅ `barrera_01_pines_de_luz` · `barrera_02_dos_puntas` · `esclavo_06_no_abre_paso` · `maestro_09_test_leds` |
| SFTY-3 | `*/src/protocolo.cpp` · `Repetidor/src/main.cpp` | — |
| SFTY-4 | `Maestro/src/coordinador.cpp` · `Maestro/src/modo_automatico.cpp` | — |
| SFTY-5 | `Maestro/src/semaforo.cpp` · `Esclavo/src/semaforo.cpp` | ✅ `Validacion_Automatico/arnes_automatico.cpp` — **arnés C++, invisible para el censo de packs**, ver abajo |
| **SFTY-6** | `*/include/protocolo.h` *(el umbral)* · `Maestro/src/coordinador.cpp` · `Esclavo/src/main.cpp` | ✅ `costura_08_silencio` · `costura_09_presupuesto_radio` · `costura_13_ambar_ordenado` · `maestro_04_sync_horaria` |
| SFTY-7 | `Maestro/include/coordinador.h` · `Maestro/src/coordinador.cpp` | — |
| SFTY-8 | `*/src/protocolo.cpp` | — |
| SFTY-9 | `Maestro/src/coordinador.cpp` · `Maestro/src/main.cpp` | — |
| SFTY-10 | `*/src/protocolo.cpp` | — |
| SFTY-11 | `*/include/protocolo.h` · `*/src/protocolo.cpp` | — |
| SFTY-12 | `Maestro/src/coordinador.cpp` · `Maestro/src/modo_manual.cpp` | — |
| SFTY-13 | `Maestro/src/coordinador.cpp` | — |
| SFTY-14 | `Maestro/src/coordinador.cpp` | — |
| SFTY-15 | `*/src/protocolo.cpp` · `Maestro/src/lcd.cpp` · `Maestro/src/modo_alcance.cpp` | — |
| SFTY-16 | `Repetidor/src/main.cpp` | — |
| SFTY-17 | `Esclavo/src/main.cpp` | — |
| **SFTY-18** | `Maestro/src/reloj.cpp` · `Maestro/include/reloj.h` · `ESP32_Expansion` *(el reloj del puente)* | ✅ `esp32_04_osf` · ✅ `esp32_11_bien_formada_no_es_cierta` |
| **SFTY-19** | **— solo diseño, ver abajo.** La única mención en el código es una advertencia en `reloj.h` aclarando que este modo **no** se apoya en el RTC | — |
| SFTY-20 | `Maestro/include/modo_hora.h` · `ESP32_Expansion/include/reloj_ds3231.h` *(sólo referencias; el modo no existe)* | — |
| **SFTY-21** | `*/src/modo_degradado.cpp` · `*/src/mando.cpp` · `*/include/ciclo_degradado.h` | ✅ `esclavo_01_latch_ambar` · `esclavo_02_inhibicion_menu` · `esclavo_07_ambar_emergencia` · `esclavo_08_ambar_en_degradado` · `maestro_01_mando` · `maestro_05_ciclo_sin_radio` · `costura_02_fase_ciclo` · `costura_06_reanudacion` · `costura_12_margen_deriva` · `costura_13_ambar_ordenado` · `costura_14_cancela_ambar` · `camara_02_j16` |
| SFTY-22 | `Maestro/src/lcd.cpp` · `Maestro/include/modo_ambar.h` *(sólo referencias; la pantalla no existe)* | — |
| **SFTY-23** | `Maestro/src/coordinador.cpp` · `Esclavo/src/config_ciclo.cpp` *(Fase 2)* · `*/src/reloj.cpp` | ✅ `esclavo_03_par_config` · `esclavo_04_desfase` · `esclavo_05_hora_atomica` · `maestro_04_sync_horaria` |
| SFTY-24 | **— solo diseño.** Cero etiquetas en el firmware | — |
| SFTY-25 | **— solo diseño.** Cero etiquetas en el firmware | — |
| SFTY-26 | **— solo diseño.** Cero etiquetas en el firmware | — |
| **SFTY-27** | 🔴 **DOS REGLAS CON EL MISMO NÚMERO — ver el aviso justo debajo.** Este documento lo define como *«matrícula de pareja»*, sin implementar; las **8 etiquetas del firmware** dicen otra cosa | — |
| **SFTY-28** | `*/src/semaforo.cpp` *(dentro de `escribirPines()`)* · `*/include/pines.h` | ✅ `barrera_03_talanquera` · `maestro_09_test_leds` |
| **SFTY-29** | **— solo diseño.** Presencia como veto del todo-rojo y sensor de pluma | — |

> ⚠️ **CÓMO SE LEE UN ✅ DE ESTA TABLA. Sin esto promete más de lo que cubre** *(auditorías del 28/08
> y del 01/09; el desarrollo y las medidas, en el anexo de `roadmap_hist.md`)*:
>
> - **La segunda columna no la comprueba nadie.** Es un resumen curado, no un censo: la fila de
>   `SFTY-21` cita tres ficheros y el `grep` devuelve **35**.
> - **Cuando la segunda columna nombra VARIOS sitios, un ✅ puede estar cubriendo uno solo.** El de
>   `SFTY-18` ejerce el `OSF` del `DS3231` **del puente ESP32**; el año marcador del RTC del STM32 y
>   `reloj_enHora()` —la regla tal como está definida arriba— **no los ejerce ningún pack**.
> - **Los cuatro packs de `SFTY-2` LEEN el C++; ninguno lo ejecuta.** El enclavamiento como tal
>   —*«nunca verde y rojo a la vez»*— sólo lo EJECUTA `Validacion_Automatico/arnes_automatico.cpp`,
>   y **sólo del Maestro**.
> - **El censo mira sólo `banco/packs/`: los arneses que compilan C++ real son invisibles para él.**
>   Una fila vacía significa *«ningún pack la ejerce»*, no *«nada la ejerce»*.
> - **Siete filas vacías seguidas** —`SFTY-3`, `5`, `7`, `8`, `10`, `11` y `15`— apuntan todas a
>   `*/src/protocolo.cpp`. No son siete casillas pendientes: son **un fichero entero sin cobertura**.
>   No se inventa cobertura para taparlas.

---

## 🕹️ SFTY-21 — Modo Degradado por reloj y mando de 4 relés (**IMPLEMENTADO**)

**Estado:** construido en las dos puntas el 01/08/2026. Sustituye y cierra el diseño anterior de
**SFTY-19**, que planteaba entrada *automática*. ⚠️ **Sin prueba de banco:** nada de esto se ha
ejercitado sobre hardware real.

**Parámetros del ciclo degradado:** verde **30 s**, todo-rojo **30 s** —ya ampliado—, ciclo fijo y
propio, que **no hereda** el verde del Modo Automático. Al ser fijo, el tope de 255 s del byte de
`CMD_CONFIG` no puede alcanzarse.

### La decisión de operación

**El controlador no cambia. Solo se añade un modo.** Pérdida de radio ⇒ **ámbar intermitente**,
exactamente como hoy, y **no se toca**. El Modo Degradado es un **caso especial de activación
MANUAL**, confirmado por un operario. **Entrada automática: NUNCA.**

| | Maestro | Esclavo |
|---|---|---|
| **entrar** | app: `SET_MODO:DEGRADADO` → `modo_degradado_evaluarEntrada()` y sólo si da `MDG_OK`, `modoActual_set(MODO_DEGRADADO)` (`Maestro/src/bluetooth.cpp`) · mando: `A·B·A·B` (`Maestro/src/mando.cpp`) | mando: `A·B·A·B` → `degradado_entrar()` (`Esclavo/src/mando.cpp`) |
| **salir** | app: `SET_MODO:MENU` → `modo_degradado_pedirSalida()`, que pasa por el todo-rojo · mando: `A·A·A` (Automático) o `B·B·B` (Ámbar) | app: `AMBAR_EMERGENCIA` → `salidaDegradadoIniciada()` (`Esclavo/src/bluetooth.cpp`) · mando: `A·A·A` (obedecer) o `B·B·B` (ámbar), **las dos por `degradado_salir()`** |

**La secuencia de ENTRADA es `A·B·A·B`, no `A·A·A`.** `A·A·A` es *«a ver si volvió el radio»*
—Automático en el Maestro, volver a obedecer en el Esclavo—, y por eso sí sirve para SALIR.

**Por qué manual y no automático.** Sin radio, el Maestro **no puede saber si el Esclavo sigue
vivo**. El ámbar intermitente dice *«no estoy controlando esto, decide tú»* y el conductor llega
ALERTA; un verde por reloj dice *«pasa tranquilo, el otro lado está en rojo»* y el conductor llega
CONFIADO y no mira. **Un verde equivocado es más peligroso que un ámbar ambiguo**, porque le quita
al conductor la precaución que el ámbar le provoca. Con activación manual, el verde se da **porque
una persona verificó las dos puntas**.

**`salidaDegradadoIniciada()` del Esclavo no es un alias de `degradado_salir()`, y esa diferencia es
el molde bueno:** `degradado_salir()` es `void` y **abandona en silencio** desde `DEG_INACTIVO`,
`DEG_SALIENDO` y `DEG_RENDIDO`; el envoltorio pregunta **la misma guarda** antes y devuelve `bool`,
para que el `$ACK` diga lo que de verdad pasó —`SALIENDO_TODO_ROJO`, no `OK`—.

### Procedimiento de puesta en marcha

1. Confirmar que **ambas unidades tienen la hora puesta y coincidente** *(por Bluetooth:
   `CMD:LEER_RTC` en cada punta; se pone con `CMD:SET_RTC:<...>`)*.
2. **Maestro:** `SET_MODO:DEGRADADO` desde la app **o** `A·B·A·B` con el mando. **Esclavo:**
   `A·B·A·B` con el mando, y hoy no hay otra. **Este paso es el único que decide.**
3. Iniciar.
4. **Verificar visualmente que los dos semáforos alternan correctamente — también AL SALIR**, no
   sólo al entrar. Debe constar en el manual del funcional y en el acta de pruebas.

### 🔴 Lo que sigue ABIERTO, y no lo cierra un documento

- **`A·A·A` en el Maestro NO TIENE GUARDA: arranca el ciclo, o sea ABRE PASO** [medido el 07/09].
  `A·B·A·B` sí está bloqueada —la entrada exige reloj en hora y en esa punta nunca lo está— y
  `B·B·B` va a ámbar, que es dirección segura.
- **El Esclavo no tiene camino por app para *ENTRAR* en Degradado.** Sale por `AMBAR_EMERGENCIA` y
  entra sólo por el mando de relés, **cuyo receptor físico no está comprado**. Añadir una puerta a
  un modo que enciende un verde sin confirmación del otro extremo es **decisión del responsable**.
  Consecuencia inmediata: el paso 2 de la puesta en marcha **no es ejecutable en el Esclavo hoy**.
- 🟠 **Asimetría entre puntas, medida y NO tocada:** las dos salidas del Esclavo por mando pasan por
  `degradado_salir()` —o sea por el todo-rojo de despedida—; en el Maestro `A·A·A` hace
  `modoActual_set(MODO_AUTOMATICO)` **sin pasar por `modo_degradado_pedirSalida()`**. No hay
  reanudación fantasma —`main.cpp` borra el indicador de respaldo en el único punto por el que pasan
  todas las salidas—; lo que se salta es el todo-rojo. **Es firmware y es vial.**
- **`D-1`: `J16` p5/p8 se quedan VACÍOS y el código del mando SIGUE leyendo sus flancos.** Eso es lo
  decidido y es correcto —`mando_ambarLocal()` tiene cinco llamadas vivas y su veto es esta regla;
  borrar su armador dejaría los `if` siempre verdaderos, o sea **el veto ABIERTO, no inerte**—. La
  consecuencia operativa es de seguridad: **no se cablea NADA en p5/p8**, porque cualquier contacto
  —un final de carrera, un rebote— compone secuencias que nadie pidió.

### La deriva y el margen

Cristal de 32.768 kHz sin calibrar, a la intemperie: **±30 a 50 ppm**.

| Tiempo sin radio | Desfase entre unidades |
|---|---|
| 1 día | ~2 – 8 s |
| 3 días | ~6 – 25 s |
| 1 semana | ~15 – 60 s |

El **despeje todo-rojo es el colchón que absorbe ese desfase**. **Querer una semana de autonomía
obliga a un todo-rojo de ~90 s**, que destroza la fluidez del paso: no es una limitación del diseño,
es la física de dos cristales sin disciplinar. **La alternativa real no es alargar el plazo: es ir a
arreglar el radio.**

### 🛑 Límite duro: el Degradado debe rendirse solo

**El estado seguro no puede depender de que alguien se acuerde.** Pasadas **48 h** sin
resincronizar, el Degradado **cae solo a ámbar intermitente**. Construido en las dos puntas:
`LIMITE_DURO_MS` en `*/src/modo_degradado.cpp` —**de ahí se lee, nunca de aquí**—, con el estado
`DEG_RENDIDO`, el rechazo `DEG_RECHAZO_SYNC_VENCIDA` y un **latch de caducidad que no se baja hasta
una sincronización nueva**, para que un corte de luz a las 47 h no regale otras 48, que es la trampa
que convierte un límite en un botón de posponer. Lo vigila `costura_05_limite_48h`.

### ⚠️ Riesgos residuales aceptados por el cliente (01/08/2026)

**La decisión de riesgo es del responsable y del cliente, y sigue siendo suya.** Lo que va escrito
aquí es la descripción sobre la que se firmó, corregida el 01/09 porque estaba corta: **una
aceptación de riesgo vale lo que vale la descripción sobre la que se firmó.**

**1. El verde se da sin confirmación del otro extremo.** Con el radio muerto es inevitable. Se
mitiga con activación manual verificada, todo-rojo ampliado y límite duro. **No se elimina.**

**2. Salida asimétrica: que una sola punta abandone el Degradado.** Es el escenario más peligroso y
no tiene solución técnica sin radio. **El disparador NO es una equivocación: es un microcorte.**
Reinicia una punta, esa punta arranca en menú, sin enlace cae a ámbar, y la otra **sigue dando verde
por reloj**; no hace falta que nadie se equivoque en nada, y cuando el técnico se ha ido no hay
quien verifique. **La consecuencia, dicha entera:** el conductor del lado ámbar negocia el paso y
entra, el del lado verde entra confiado y sin mirar, **y se encuentran de frente dentro del tramo.**
Es la única forma en que este equipo puede matar a alguien.

**3. Y una SEGUNDA salida asimétrica, que no necesita ni microcorte ni operario:** las dos puntas
cuentan las 48 h por caminos distintos —el Maestro contrasta con la pila, el Esclavo usa `millis()`
con latch—, **así que no se rinden en el mismo instante**, y en ese hueco una está en ámbar mientras
la otra sigue dando verde por reloj. Es la propiedad que `costura_05_limite_48h` existe para
reproducir. **El límite duro cierra la deriva y abre esto.** Cuánto dura el hueco **es una medida de
banco que nadie ha hecho.**

**Y nadie mide el invariante que lo cerraría:** *«nunca verde en las dos puntas a la vez»* no lo
ejecuta ningún instrumento sobre el C++ real de **ambos** extremos —`Validacion_Automatico` compila
de verdad pero **sólo el Maestro**—.

### 🎛️ El mando de 4 relés — interfaz sin realimentación visual

El operario acciona **desde el piso y sin ver la pantalla**, que está a 5 m dentro del gabinete.
Restricciones medidas en campo (01/08): la señal es **pulso por flanco** —la pulsación larga NO
existe: sostener el botón 10 s da un solo pulso—, hay **~2 s** de retardo por pulsación, y **no hay
repetición automática**.

**La confirmación se da en destellos ROJOS contables**, porque el rojo nunca significa *«pase»*: **si
el operario cuenta mal, el peor caso sigue siendo seguro.** Destellar los tres colores a la vez se
descartó: un conductor lejano podría interpretar el verde.

| Secuencia | Acción | Confirmación |
|---|---|---|
| **`A · A · A`** *(≤ 12 s)* | **AUTOMÁTICO** — "a ver si el radio volvió" | **2** destellos rojos |
| **`B · B · B`** *(≤ 12 s)* | **ÁMBAR intermitente** — salida de emergencia | **3** destellos rojos |
| **`A · B · A · B`** *(≤ 18 s)* | **Entrar a MODO DEGRADADO** | **4** destellos rojos |

**Memotecnia:** `A` es arriba → SUBE al modo normal · `B` es abajo → BAJA al mínimo seguro ·
alternar → modo especial. Se aprende en un minuto, que es el requisito real para alguien que lo usa
de madrugada y bajo lluvia.

Volver a Automático **no necesita protección, porque el propio sistema se corrige**: sin respuesta en 25 s (SFTY-6) se va solo a ÁMBAR INTERMITENTE, que es justo donde se quería estar.

**Las reglas del mando, que son lo que queda:**

- **A ciegas se usan únicamente los botones cuya repetición accidental es inofensiva.** `C`
  EJECUTA; `A` y `B` sólo MUEVEN: con el equipo dejado en el menú, un `C·C·C` desde el piso
  selecciona lo que tenga el cursor y arranca un modo no pedido, mientras que un `A·A·A` sólo sube
  el cursor tres veces. *(Desde el 31/08 es además estructural: `PB14`/`PB15` son entradas de cámara
  y `botonAceptar()`/`botonCancelar()` devuelven `false`, así que `C` y `D` no pulsan nada. Lo
  vigila `camara_02_j16`.)*
- **`A·B·A·B` no se produce nunca navegando:** se sube o se baja, no se zigzaguea. Y si el operario
  se equivoca a mitad, lo único que ha ocurrido es que el cursor se movió.
- 🔒 **Mientras el menú esté abierto, las secuencias del mando NO se reconocen.** Es requisito, no
  afinamiento: una ráfaga accidental con el menú abierto podría llegar a `AJUSTAR HORA` y confirmar
  una hora cualquiera que el equipo daría por buena — **exactamente el veneno que SFTY-18 existe
  para evitar**, y habilitaría el Degradado y la operación nocturna sobre una hora inventada.
- **Asimetría deliberada: lo seguro fácil, lo peligroso difícil.** El ámbar accidental es molesto,
  no peligroso ⇒ secuencia corta. El Degradado da verde sin confirmar el otro lado ⇒ secuencia
  larga **más validación en firmware**.

> **`B·B·B` devuelve a ámbar desde cualquier estado, sin condiciones.** Es la regla que impide que
> nadie quede atrapado con un semáforo en estado raro a 5 m de altura.

**La red de seguridad real no es la secuencia.** Aunque alguien acierte `A·B·A·B` por casualidad, el
firmware **no entra** si no se cumplen las tres: reloj en hora, medición de desfase reciente y
dentro de tolerancia, y configuración del ciclo sincronizada con el otro lado (SFTY-23). Si falta
alguna ⇒ **ÁMBAR RÁPIDO 2 s y RECHAZADO**. El mando permite reactivar en campo sin grúa, pero **no**
saltarse la puesta a punto.

---

## ⏱️ SFTY-23 — Sincronización horaria por radio (**IMPLEMENTADO**)

**Estado:** construido en las dos puntas el 01/08/2026. **Requisito previo del Modo Degradado**, no
un extra. ⚠️ **Sin prueba de banco.**

**El defecto que corrige.** Ajustar a mano las dos pantallas deja hasta **59 s de desfase el primer
día** —casi cuatro veces el todo-rojo— y **dos pantallas en `HH:MM` no pueden detectarlo**: la tabla
de deriva de SFTY-21 asume que las unidades arrancan en ~0 de desfase, y ajustando a mano esa
premisa es falsa.

### La regla

**La hora se cuadra UNA sola vez, en el Maestro, y el Esclavo nunca se toca a mano.** El Maestro
empuja su hora por radio mientras el enlace vive, de modo que el día que el radio muera el desfase
arranque en ~0 **de verdad**, no por procedimiento. La pila lo hace durable: cada reloj sobrevive
los cortes con su propia CR2032.

**Se dispara** al poner en hora el Maestro —poner en hora *es* sincronizar— y **periódicamente
mientras haya enlace**; con una vez por hora sobra.

### Comandos

**La fuente de verdad es `01_Firmware/*/include/protocolo.h`**, idéntico en ambos proyectos, y el
simulador lee los códigos de ahí en cada ejecución con lectura obligatoria. La tabla siguiente es un
índice, no la fuente:

| Comando | Código | `param` |
|---|---|---|
| `CMD_HORA_H` | `0x07` | hora (0–23) |
| `CMD_HORA_M` | `0x08` | minuto (0–59) |
| `CMD_HORA_S` | `0x09` | segundo (0–59) — **al recibirla, el Esclavo aplica las tres juntas** |
| `CMD_ACK_HORA` | `0x0A` | confirmación del Esclavo tras aplicar la terna |
| `CMD_DELTA` | `0x0B` | segundo actual del Maestro |
| `CMD_DELTA_RESP` | `0x0C` | diferencia medida, complemento a dos (`int8_t`) |
| `CMD_CONFIG_VERDE` | `0x0D` | segundos de verde del ciclo degradado |
| `CMD_CONFIG_DESPEJE` | `0x0E` | segundos de todo-rojo, **ya ampliado** |
| `CMD_ACK_CONFIG` | `0x0F` | confirmación del par de configuración |

**Aplicación atómica:** el Esclavo acumula hora y minuto en un buffer y **sólo escribe el RTC al
llegar la de segundos**; nunca queda una hora a medias. El puente **SFTY-16 valida formato y CRC, no
comandos**, así que las tramas atraviesan el ESP32 sin modificarlo.

> ### ⚠️ Regla obligatoria en los reintentos
>
> **El Maestro debe RECALCULAR el valor de segundos en cada retransmisión, nunca reenviar el que
> calculó la primera vez.** Si la trama se pierde y se reintenta 3,5 s después con el valor viejo,
> el Esclavo queda 3,5 s atrasado — y el error entra justo por el mecanismo que existe para dar
> robustez. Es un fallo de una sola línea que no se ve en pruebas con enlace bueno.

### La validación debe ser una MEDICIÓN, no una inspección ocular

`CMD_DELTA` convierte *«confirmar que la hora coincide»* de mirar dos pantallas a **leer un
número**, registrable en el acta, y **habilita el gate de entrada en firmware**. El desfase medido
incluye el tiempo de aire más el retardo de cortesía del Esclavo (SFTY-17, 200 ms), así que trae un
sesgo de algunas décimas de segundo: frente a un todo-rojo de 15–30 s es irrelevante, y conviene
dejarlo escrito para que nadie persiga ese error. El `param` es de un byte: la diferencia se
transmite **con signo, ±127 s**, y fuera de ese rango debe **saturar y reportarse como "fuera de
rango"**, nunca dar la vuelta.

> ### ⚠️ Límite inherente: la medida sólo alcanza ±30 s, y la regla que cierra el agujero
>
> Como `CMD_DELTA` transporta **sólo el segundo** (0–59), la corrección circular resuelve en el
> sentido corto y el resultado **siempre cae en ±30 s**: un desfase real de 45 s se mide como −15 s,
> y **un desfase peligroso podría leerse como aceptable y pasar la puerta del Modo Degradado.**
>
> **La puerta del Degradado NO puede apoyarse sólo en el desfase medido. Debe exigir las dos
> condiciones a la vez:** (1) una **sincronización correcta reciente** y (2) **desfase medido dentro
> de tolerancia**. La primera es la que hace fiable a la segunda: tras una sincronización correcta
> el desfase arranca en milisegundos, y con ~100 ppm hacen falta **más de tres días** para acumular
> los 30 s del alias. **El desfase es una comprobación de cordura, no la garantía; la garantía es la
> sincronización reciente.** Invertir esa relación reintroduce el fallo.
>
> *Si algún día hiciera falta más rango: añadir una trama con el minuto y calcular sobre
> segundos-dentro-de-la-hora llevaría el alcance a ±30 min. Hoy no es necesario.*

**El ciclo también debe viajar, no sólo la hora.** Dos relojes en hora dan tiempo común, pero para ir
en fase ambas unidades tienen que computar el **mismo horario de fases**. Esa configuración sólo
existe en el Maestro: o se configura a mano en las dos puntas —la misma fuente de error humano que
esto acaba de eliminar para la hora— o **se sincroniza junto con la hora mientras hay enlace**.
`CMD_CONFIG` cubre eso, y **es tan condición de seguridad como la hora**.

**Lo que NO arregla:** la deriva posterior sigue corriendo. Sincronizar pone el desfase a cero en el
momento de perder el radio, pero a partir de ahí crece igual. Por eso el **límite duro** de SFTY-21
sigue siendo necesario.

---

## 📺 SFTY-22 — Pantalla informativa durante el ámbar (MEJORA, **NO IMPLEMENTADO**)

**Estado:** marcada como mejora el 01/08/2026. No es urgente: el sistema funciona.

Al perder el enlace el equipo entra en ámbar intermitente **sin decir por qué**. La propuesta es que
al entrar en ámbar **aparezca sola** una pantalla con la causa (`SIN ENLACE`), el **desde cuándo**
—lo aporta el RTC de SFTY-18—, y los contadores que ya existen: la fila `RX` de SFTY-15
(`RX 0 - nada llega` / `RX 4k - BASURA` / enlace correcto) y la telemetría de SFTY-14
(`RF:100% 340ms`). Casi todo el dato ya está dentro del firmware; lo único que falta es juntarlo y
**la hora**.

### ⚠️ Un diagnóstico no debe alterar lo que diagnostica

**Aparece sola al entrar en ámbar, y ésa es la vía principal.** Es gratis: el equipo ya está
detenido, así que informar no cambia nada. **No debe diseñarse como una opción más del menú**,
porque entrar al menú **detiene el ciclo** (SFTY-12 deja ambas unidades en rojo fijo): se acabaría
diagnosticando un equipo que ya dejó de hacer aquello que se quería diagnosticar.

**Su público es el técnico, no el conductor.** Su valor está en separar **tres averías que hoy se
ven todas igual** —cobertura/antena, cableado/línea flotando, y enlace correcto—, que es la
distinción que costó la jornada completa del 31/07.

---

## 🌙 SFTY-20 — Operación intermitente nocturna (DISEÑO, **NO IMPLEMENTADO**)

**Estado:** especificado el 01/08/2026. Corresponde al pendiente **N-3** y es **para lo que se soldó
la pila**. Se construye, pero **no va a campo** hasta cerrar las antenas y la prueba de banco de la
telemetría. Requisito de origen: `MANUAL_USUARIO.md §2`.

**Disparo por horario**, decidido el 31/07 —el disparo por flujo real exigiría la cámara instalada—.
La franja es **configurable**, porque el horario no es el mismo en todas las obras.

### Reglas de seguridad, no negociables

| # | Regla | Por qué |
|---|---|---|
| 1 | `reloj_enHora() == false` ⇒ **nunca** entrar en modo nocturno | Ya construido en SFTY-18. Un reloj sin poner en hora activaría el modo a deshora |
| 2 | Entrar y salir del modo pasa por el **despeje todo-rojo** (SFTY-4) | No se salta de verde a intermitente |
| 3 | Si el Esclavo no confirma `CMD_GO_NOCHE` ⇒ ámbar en ambos | Se degrada al fallo conocido, no a un estado a medias |

**Un estado propio, aunque se vea igual.** El Maestro en modo nocturno parpadea ámbar, visualmente
idéntico a `S_FALLO`; reutilizar `S_FALLO` haría que la telemetría **reporte una avería que no
existe**. Hoy el enum es `{ S_ROJO, S_VERDE, S_AMARILLO, S_FALLO }`.

### Piezas

| # | Pieza | Nota |
|---|---|---|
| 1 | Pantalla **AJUSTAR HORA** | ver SFTY-18: hoy la vía viva es `CMD:SET_RTC` por Bluetooth |
| 2 | Pantalla **FRANJA NOCTURNA** | hora de inicio y hora de fin |
| 3 | **Persistencia de la franja** | ⚠️ la pieza que puede descarrilar el resto — ver abajo |
| 4 | Estados de luz nuevos | Maestro ámbar intermitente · Esclavo rojo intermitente |
| 5 | Comando RF `CMD_GO_NOCHE` | `0x07` está libre (`0x01`–`0x06` ocupados) |

### ⚠️ La franja no sobrevive al apagón

`reloj.cpp` guarda hoy la franja **sólo en RAM**: se va la luz y vuelve al valor por defecto,
inaceptable para algo que el operario configura en obra. `STM32duino RTC` **no expone los registros
de respaldo** —comprobado al compilar: `getBackupRegister` no existe en la 1.9.0—; en el STM32F1 son
accesibles directamente (`BKP->DR1..DR10`, previa habilitación de escritura en el dominio de
respaldo), unas diez líneas. **Es el trabajo menos obvio de todo el conjunto y el que más fácil se
pasa por alto al planificar.**

### Cómo se valida — y el requisito previo

La validación debe comprobar que (1) ambas unidades **entran y salen de la franja en fase**, (2) la
transición **pasa por el despeje todo-rojo** y no salta, y (3) con `reloj_enHora() == false`
**nunca entra**, ni al principio ni al cruzar la franja.

> ⚠️ **Requisito previo (N-12).** El simulador tiene el despeje todo-rojo escrito a mano y **no lo
> lee del C++**. Como ese despeje *es* el margen de seguridad al entrar y salir de este modo,
> validar SFTY-20 contra un valor que el modelo no vigila **no demostraría nada**.

⚠️ **`SFTY-20` está declarada `NO IMPLEMENTADA` y el firmware lo confirma:** las cuatro funciones de
la franja nocturna (`reloj_ajustarFranjaNocturna`, `reloj_esHorarioNocturno`, `reloj_inicioNoche`,
`reloj_finNoche`) están **sin llamador**, y así están congeladas en `costura_10_funciones_muertas`.
Obra a medias declarada como tal, no un hallazgo.

---

## 🚧 SFTY-19 — Operación autónoma al perder el radio (DISEÑO, **NO IMPLEMENTADO**)

**Estado:** especificado el 31/07/2026. ❌ **SUSTITUIDA POR SFTY-21 (01/08/2026):** planteaba
**entrada automática** tras N minutos sin enlace y **se descartó en reunión con el cliente** —sin
radio nadie puede confirmar que la otra punta esté viva, y una máquina no debe decidir sola operar a
ciegas—. Lo aprovechable —la deriva entre relojes, el todo-rojo como colchón y las condiciones que
impiden entrar— **se trasladó a SFTY-21**. **El comportamiento actual sigue siendo SFTY-6: sin
enlace ⇒ ámbar intermitente en las dos puntas.** Se conserva el texto porque el análisis de
sincronización *relativa* sigue siendo válido si algún día se retoma.

**Por qué es delicado.** Este sistema regula **un carril alternado**: cuando el Maestro da verde, el
Esclavo **tiene** que estar en rojo. Si las dos unidades cuentan cada una por su lado y se separan
lo suficiente, **hay verde simultáneo en las dos puntas y dos vehículos entran de frente al tramo.**

### Reglas que hacen viable el modo

| # | Regla | Por qué |
|---|---|---|
| 1 | Mientras hay enlace, el Maestro comunica periódicamente **en qué punto del ciclo va** | Da a las dos unidades un origen común |
| 2 | Al perderse el enlace, cada unidad continúa **desde el último punto sincronizado** | La separación arranca en ~0, no en un valor arbitrario |
| 3 | El despeje **todo-rojo** debe superar con holgura la separación acumulada máxima | Es el margen que absorbe la deriva |
| 4 | **Límite duro de tiempo sin enlace** ⇒ ámbar intermitente | Más allá no se puede acotar la deriva **ni saber si la otra punta sigue viva** |
| 5 | Unidad que **arranca o se reinicia sin haber sincronizado nunca** ⇒ ámbar, sin excepción | Sin origen común no hay modo autónomo posible |

**Las reglas 4 y 5 no son opcionales.** Son lo que separa *«modo autónomo»* de *«verde en las dos
puntas»*.

**Hallazgo de diseño: esto NO necesita el RTC.** La sincronización es **relativa** al último mensaje
del Maestro, no a la hora absoluta; basta el contador de milisegundos. El RTC (SFTY-18) sigue siendo
necesario para la operación nocturna, que es otra función. `reloj.h` lleva una advertencia explícita
para que nadie ancle las dos unidades a su reloj de pared.

**Parámetros por decidir:** tiempo máximo sin enlace antes de rendirse a ámbar *(propuesta: 30 min)*
y ciclo que corre en modo autónomo *(propuesta: reusar el verde ya configurado en MODO AUTOMÁTICO)*.

**Criterio de aceptación:** la validación **debe medir la separación entre las dos unidades** a lo
largo de la ventana sin radio y demostrar que el todo-rojo la cubre — no basta con afirmarlo.
Mientras esa medida no exista, este modo **no va a campo**.

---

## 📶 SFTY-24 — Enlace de respaldo por datos móviles entre dos teléfonos (DISEÑO, **NO IMPLEMENTADO**)

**Estado:** propuesto el 26/08/2026. **Nada de esto está en el firmware ni en la app.** El enlace
entre puntas sigue siendo únicamente el radio LoRa E90-DTU, y su pérdida sigue cayendo a ámbar
intermitente (SFTY-6).

**Lo que resolvería, y es real:** saber si la otra punta está viva sin caminar el tramo; juntar
`$ALARM` y `$EVENT` de las dos cajas negras con marca de tiempo para la interventoría; un *courier*
RTC instantáneo en vez del actual, que es el técnico llevando la hora andando; y confirmar una
maniobra antes de hacerla.

### 🛑 La regla, y no es negociable

> **El enlace por datos puede OBSERVAR y puede DOCUMENTAR. No puede AUTORIZAR.**

Ninguna trama que llegue por ese canal puede provocar un verde, acortar un todo-rojo, ni sacar a un
equipo del ámbar. Como mucho puede **pedir** algo que el radio LoRa tendrá que confirmar por su
cuenta, con su CRC y su ACK, exactamente igual que si la petición hubiera venido de un botón. Las
razones son las de SFTY-19, más tres propias:

1. **La cobertura celular es justo lo que no hay donde se usan estos equipos.** Un canal que existe
   **a veces** es peor que uno que no existe nunca: el que no existe no engaña a nadie; el
   intermitente enseña a confiar y falla el día que importa.
2. **Latencia no acotada.** El paso alternado necesita saber **ahora** si el otro lado está en rojo,
   no hace cuatro segundos. Un verde concedido sobre un mensaje retrasado es verde simultáneo en las
   dos puntas.
3. **Cuatro puntos de fallo nuevos, ninguno bajo control del equipo:** dos baterías de teléfono, dos
   personas, dos operadoras y una nube.

**Parámetros por decidir:** transporte (servidor propio contra canal directo entre teléfonos); qué
se sincroniza *(propuesta mínima: sólo `$STATUS`, `$ALARM` y `$EVENT`, de lectura; nada de comandos,
y si algún día los hay, pasan por la radio)*; y **qué ve el técnico cuando el canal se cae**, que
será lo normal *(propuesta: la pantalla del otro extremo se marca **caducada con su antigüedad en
segundos**, nunca se congela mostrando el último valor como si fuera actual — un dato viejo sin
fecha es peor que ningún dato)*.

**Criterio de aceptación:** antes de escribir una línea, **medir la cobertura real en los tramos
donde opera el equipo**, con el teléfono que usa el técnico, a lo largo de una jornada. Y la medida
va al repositorio, no al recuerdo de nadie.

---

## 🏷️ SFTY-25 — Identidad de tramo en la telemetría (RIESGO ABIERTO, **NO IMPLEMENTADO**)

> **Esto no es una mejora futura: es un agujero de hoy**, y aparece en cuanto hay más de un par de
> semáforos en la misma vía, que es el caso de uso real.

**El escenario.** Una vía en obra lleva varios pares Maestro/Esclavo —Km 12, Km 24, Km 31— y **un
solo teléfono** recorriéndolos. La app distingue el **rol** (`NODE:MAESTRO` / `NODE:ESCLAVO`) porque
la trama lo trae, pero **el rol no dice de qué par es ese equipo**, y dos pares distintos emiten hoy
tramas **indistinguibles**.

**Por qué el rol no basta:** el rol no es el sentido, y el sentido no es la instalación. *«Sentido 1
= Sisga → Bogotá»* es una propiedad de **cómo se plantó el poste**, no de qué firmware lleva dentro.
**El fallo concreto:** el técnico está en el Km 24, su teléfono se engancha por Bluetooth al par del
Km 12 —que sigue en rango, o fue el último emparejado—, pulsa **DAR PASO A SENTIDO 1** creyendo que
gobierna el poste que tiene delante, y **abre un verde a 12 km** en un tramo que no está mirando.

### Lo que hace falta

| # | Pieza | Dónde |
|---|---|---|
| 1 | Campo `ID:` en `$STATUS`, `$ALARM` y `$EVENT` con el identificador de tramo | `bluetooth.cpp`, las tres tramas |
| 2 | Ese identificador configurable y guardado en respaldo | `respaldo.cpp` — cambia la `FIRMA` |
| 3 | La app muestra el `ID:` **grande y permanente**, no en un submenú | App móvil |
| 4 | Etiqueta física visible en el poste con el mismo identificador | Procedimiento de instalación |
| 5 | La app **rechaza** un comando si el `ID:` de la conexión no es el que el operario tiene seleccionado | App móvil |

**El punto 4 no es burocracia:** es lo que permite al operario **comparar lo que ve en la pantalla
con lo que tiene delante**. Sin esa comparación, los otros cuatro sólo mueven el error de sitio.

**Criterio de aceptación:** dos pares encendidos a la vez en el banco, un solo teléfono, y demostrar
que **un comando dirigido al par A no llega al par B** — ni siquiera cuando el operario lo intenta a
propósito. Mientras esa prueba no exista, **el manual de la app debe advertir que sólo se opera con
un par encendido a la vez**.

---

## 🔌 SFTY-26 — Expansor I2C para acabar con la disputa por los dos pines libres (DISEÑO, **NO IMPLEMENTADO**)

**Estado:** decidido el 26/08/2026 como salida de `N-57`. **No es una elección entre reloj y
cámaras: son las dos, sobre el mismo bus, y el firmware detecta qué hay montado.**

**El problema, en una línea.** La placa tiene **dos pines libres** y **tres cosas** los quieren: el
`DS3231` necesita dos (`SDA`, `SCL`) porque `N-37` cerró en banco con el cristal `Y2` **muerto**, y
las cámaras necesitan al menos uno. No caben **como pines sueltos**; sí caben **como bus**.

**La salida:** `PB0` y `PB8` dejan de ser dos entradas y pasan a ser **un bus I2C** del que cuelga
una placa hija, **con el cableado idéntico en todas las unidades**. El **`PCF8574` (`0x20`) va
siempre** —así cualquier entrada futura entra por el expansor sin volver a abrir esta discusión—; el
**`DS3231` (`0x68`, más `0x57` de su EEPROM) va sólo donde el cristal esté muerto**. No colisionan.

**Un solo firmware para los dos casos: se detecta al arrancar.** El micro escanea el bus: si
responde `0x68` la fuente de hora es el `DS3231` externo, si no, el cristal `Y2` interno; si
responde `0x20` las cámaras se leen por el expansor, si no, por `PB0` directo. **Una tarjeta con el
cristal sano funciona sin placa hija; si el cristal falla, se le enchufa el módulo y arranca
usándolo, sin recompilar nada.**

### Y AVISA. Un respaldo silencioso sería el defecto, no la solución

Esto es lo que `N-12` dejó escrito: *un valor por defecto silencioso derrota el propósito*. **La
fuente de hora nunca se elige en silencio:** `RELOJ: INTERNO` / `DS3231` / `SIN FUENTE` en la
pantalla de estado, `CLK:INT` / `CLK:EXT` / `CLK:NONE` en `$STATUS`, y un `$ALARM` en la caja negra
al detectar que el cristal interno **acepta la hora y no avanza**.

**La detección de «cristal muerto» no es *«no está en hora»*** —eso le pasa a una unidad sana recién
encendida—: **es que la hora no AVANCE**, y se mide comparando dos lecturas separadas. Confundir las
dos cosas hace comprar módulos que no hacen falta. El censo se construye solo del uso normal de la
app, que ya manda la hora del teléfono al conectar y puede compararla contra `HORA:` en cada
`$STATUS`; y **funciona sobre unidades con el LCD muerto**, que es el agujero que `N-37` no pudo
cubrir en el Esclavo.

### Lo que hay que medir ANTES de comprometerlo

**El I2C es por software**, no por hardware: los dos puertos nativos están ocupados —`I2C1`
(`PB6`/`PB7`) lo tiene el **Bluetooth** desde N-76 (`USART1` remapeado, conector `J17`), y
`PB10`/`PB11` el RS-485—.

| Riesgo | Cómo se mide |
|---|---|
| Una transacción I2C larga acerca el `IWDG` a su ventana | Instrumentar el margen del watchdog, no estimarlo |
| Latencia de detección de cámara | El contacto de relé dura ~1 s: el sondeo tiene que caber con holgura |

**Ninguno de los dos se resuelve desde el PC. Van a banco.** *(El tercer riesgo de la versión
anterior —«la lectura del expansor compite con el refresco del LCD»— dejó de existir el 31/08: los
cuatro argumentos de pin de `u8g2` son `U8X8_PIN_NONE` y no hay refresco que se vuelque al cable.)*

> 🔴 **Y un choque que este apartado no resuelve: propone `PB0` como `SDA`, pero `PB0` es hoy
> `CAM_DEMANDA_PIN`** —la cámara de demanda, con su RC de 1 ms en la bornera `J14`, leída por nivel
> en el Maestro y por flanco en el Esclavo—. Convertirlo en media línea de bus **retira una entrada
> que está en uso**; el apartado da eso por gratis porque se escribió cuando `PB0` era *«un pin
> libre en disputa»*. **Ya no lo es.**

### Tres avisos eléctricos que van al manual, no al aire

1. **`VCC` a 3,3 V. NUNCA a 5 V.** En el STM32F103 `PB0` tiene entrada analógica y **no es tolerante
   a 5 V**. Los módulos llevan sus pull-up hacia `VCC`: con `VCC` a 5 V esas pull-up meterían 5 V en
   `PB0` por el bus. **Verificar en el datasheet antes de energizar**, y corregir el Manual 11, que
   hoy dice *"3.3V (o 5V)"*.
2. **Dos módulos = dos juegos de pull-up en paralelo.** ~4,7 k cada uno quedan en ~2,3 k. A la
   velocidad baja de un I2C por software no debería estorbar, **pero es una medida de banco**: si el
   bus no arranca, retirar las pull-up de uno de los dos módulos es lo primero.
3. **El `ZS-042` trae diodo y resistencia de carga.** Con `LIR2032` recargable, bien; con `CR2032`
   no recargable hay que **levantar `D1` o `R1`**. No confundirla con la `CR2032` de `VBAT` de la
   placa madre, que es otra pila y otra regla (`R5`).

**Criterio de aceptación:** censo de las unidades **antes** de comprar nada, y las medidas de la
tabla en banco antes de dar el expansor por bueno.

---

## 🔗 SFTY-27 — Matrícula de pareja: quién obedece a quién (DISEÑO, **NO IMPLEMENTADO**)

> # 🔴 SI HAS LLEGADO AQUÍ SIGUIENDO UN PUNTERO DESDE EL FIRMWARE, ESTA NO ES LA REGLA QUE BUSCAS
>
> **`SFTY-27` designa DOS reglas distintas, y CUATRO sitios mandan a leer la equivocada.**
>
> | | |
> |---|---|
> | **Lo que dice este apartado** | matrícula de pareja: `SERIE`, `PAIR`, `SITIO`/`SENTIDO`. **DISEÑO, no implementado** |
> | **Lo que dicen las 8 etiquetas del firmware** | *«el Esclavo PIDE y el Maestro DECIDE»* — la asimetría de `demanda_solicitar()`. **Implementada y viva** |
>
> Los ocho sitios son `Maestro/src/bluetooth.cpp` · `Maestro/src/botones.cpp` ·
> `Maestro/include/botones.h` · `Maestro/include/demanda.h` y sus cuatro gemelos del Esclavo
> [MEDIDO 01/09], más dos packs y tres manuales. **Renumerar es del responsable** (`AB-8` / `P-3` de
> `INDICE_CRUZADO.md`): son 13 sitios a la vez, y un número de regla a medio cambiar es peor que uno
> duplicado. **Hasta entonces este aviso es lo que impide que el puntero engañe, y no se quita sin
> cerrar `AB-8`.**

**Estado:** diseñado el 26/08/2026. **Nada está implementado.** Hoy `RF_Packet` no lleva
direccionamiento de ningún tipo —`{msgID, command, param, crc}`—, así que dos parejas dentro del
alcance de la radio (**1 a 3 km**, justo la distancia a la que conviven dos frentes de obra) **se
mandan órdenes entre sí**.

### Las tres piezas

1. **`SERIE` — la identidad, y no se puede editar.** Derivada del **UID de 96 bits que el STM32F103
   trae grabado de fábrica** (`0x1FFFF7E8`), reducida a 4 hex: `7A3F`. Única, no falsificable, sin
   reloj, sin escribir memoria y sin base de datos de fábrica. *(Se descartó acuñar el ID con
   fecha+hora al arrancar: en un equipo recién salido de taller `reloj_enHora()` es **falso**, así
   que todas las unidades nacerían con el mismo sello.)*
2. **`PAIR` — el filtro, 2 bytes en la trama de radio.** El Maestro **nace** con `PAIR` = su `SERIE`;
   no hay nada que configurarle, nunca. El Esclavo nace en blanco (`0000` = sin adoptar) y **no
   obedece a nadie**, así que se queda en ámbar. La pregunta que se hace la radio no es *«quién me
   habla»* sino *«esto es de mi pareja»*.
3. **`SITIO` y `SENTIDO` — las etiquetas, y ésas SÍ se editan.** El poste se muda cada día; su
   etiqueta tiene que poder cambiar. **Lo que no puede cambiar es quién es, no dónde está.**

**La matrícula se hace por Bluetooth, no por radio**, y el motivo es el alcance: la radio LoRa llega
a 1–3 km y encontraría Maestros de otros frentes de obra; el Bluetooth llega a 10–15 m. **El alcance
corto, que para telemetría es una limitación, aquí es la garantía:** es físicamente imposible
matricular por error con un Maestro que está a 12 km. La app lee el `PAIR` del Maestro y lo escribe
en el otro poste con `SET_PAIR`; **nadie teclea el código** — un dedazo crearía dos parejas con el
mismo número, que es el único escenario peligroso. **La app no da por bueno su propio envío:** relee
la trama del equipo y compara lo que quedó escrito.

**El Esclavo pide; no ordena.** Una ORDEN (*«ponte en verde»*) el Esclavo la ejecutaría: **NO**. Una
PETICIÓN (*«hay demanda aquí»*) viaja al Maestro, que decide, aplica el todo-rojo y ordena: **SÍ**.
El mecanismo ya existe —`CMD_DEMANDA` (`0x11`), añadido para la cámara 3—: un botón del funcional es
exactamente lo mismo que un coche detectado. Con dos funcionales, uno en cada extremo, **el Maestro
serializa**: ninguno concede nada, los dos piden.

### Todos los fallos acaban en ámbar

| Qué sale mal | Dónde acaba |
|---|---|
| Esclavo sin matricular | No obedece a nadie: **ámbar**. En pantalla, `SIN ADOPTAR` |
| Matriculado con el Maestro equivocado | El Maestro correcto se queda sin Esclavo: **ámbar** |
| Dos parejas en la misma vía | Códigos distintos por construcción: **se ignoran** |
| App conectada al poste que no toca | El Esclavo no acepta órdenes de tráfico **de nadie** |

**Ninguna salida va a verde.** Eso es lo que hace que una matrícula equivocada no pueda hacer daño:
sólo puede dejar sin servicio.

**Consecuencia operativa que va al manual de mantenimiento:** sustituir un **Esclavo** es trivial;
sustituir un **Maestro** obliga a re-matricular, porque su `PAIR` es su identidad y el repuesto es
otro chip. Es un gesto de un minuto, **pero si no está en el manual alguien se encontrará un Esclavo
en ámbar sin saber por qué**. La re-matrícula debe ser posible pero **deliberada**: PIN,
confirmación y registro en la caja negra con fecha.

**Criterio de aceptación:** pack `costura_08_pareja` con su control negativo —una trama con `PAIR`
ajeno **debe** ser descartada, y el pack tiene que haberse visto fallar con el filtro desactivado a
propósito—. Y en banco: **dos parejas encendidas a la vez**, demostrando que una orden dirigida a la
pareja A no la ejecuta la B.

---

## 👁️ SFTY-29 — Presencia en el tramo: un VETO, nunca un atajo (**DISEÑO**, no implementado)

**Estado:** especificado el 27/08/2026. Sustituye a la idea de *«contar vehículos»* que arrastraban
los manuales bajo el nombre de *«cámara de umbral»* (N-59), retirada de V9.0 por cara y frágil.

> 🔴 **TODA ESTA REGLA CUELGA DE UN BIT, Y EL BIT NO ESTÁ ACREDITADO (`N-159`).** Con la ficha del
> modelo ya comprado (`D-10`) delante, la fila `Linkage Method` enumera cinco métodos y
> **`Trigger Alarm Output` NO está**; el manual genérico lo da *«only supported by certain models»*.
>
> 🟢 **12/09 — HAY MEDIDA DE CAMPO DEL 10/09, Y ES POSITIVA:** una cámara del Maestro con
> `Intrusion Detection` y `Trigger Alarm Output` dio **0 V en reposo y 3,3 V al detectar** en el
> borne (`roadmap.md` §3.8, `N-159`). ⚠️ **Pero NO se lee como cerrada: no tiene ACTA** —ni captura
> de la lista de casillas, ni número de serie, ni firmware de la cámara— **y cubre UNA de las
> cuatro**. El estado correcto, y el mismo en todos los documentos: **«confirmado en UNA cámara el
> 10/09, sin acta; el `ENSAYO 0` se repite en las otras tres»**.
>
> 🔴 **Y se retira el razonamiento con el que esto se daba por negativo:** *«no está en la lista de
> `Linkage Method`»* **NO significa** *«el equipo no puede»*. Esa misma ficha omite la entrada de
> alarma en su fila `Basic Event` mientras declara `Alarm: 1 input, 1 output`, y el manual tiene un
> capítulo entero `Set Alarm Input`: **las filas de evento de esa ficha no son un censo exhaustivo**
> (`CLAUDE.md` §7.1). Lo cierra el `ENSAYO 0` de `roadmap.md` §3.8 en las tres cámaras que faltan,
> **con la lista de casillas copiada literal**, que es lo que la medida del 10/09 no recogió.
>
> 🔴 **Y su fase de grabación (`D-14`) NO EXISTE EN EL FIRMWARE.** *«El controlador cierra un
> contacto y la cámara graba»*: **cero anclas en las dos puntas**, medido por separado por dos
> agentes el 07/09. Fuera de `semaforo.cpp` los únicos `digitalWrite` reales del Maestro son la
> dirección del RS485 y la del LoRa: **no hay contacto que cerrar.** Lo vigila
> `decisiones_01_anclas`, **y el banco está en rojo por ello a propósito**.

### La distinción que lo cambia todo

Las cámaras 2 y 4 no cuentan: **detectan presencia**. Y esa presencia no autoriza a ir más deprisa,
sólo puede decir *«todavía no»*.

| | conteo *(descartado)* | **presencia** *(esto)* |
|---|---|---|
| qué viaja por radio | un mensaje por cada coche que entra y otro por cada uno que sale | **un bit**: sigue ocupado / libre |
| qué autoriza | **ACORTAR** el todo-rojo | **RETRASAR** el verde |
| si la detección falla | se acorta un despeje que no debía acortarse → **dos vehículos de frente** | se cae al temporizador de siempre → **nada peor que hoy** |
| si detecta de más | — | se espera un poco más |

**Los dos modos de fallo caen del lado seguro.** Por eso esta regla no debilita el todo-rojo: lo
refuerza, y por eso el conteo se descarta y esta versión no.

**Y la objeción de la radio desaparece.** El enlace va a 2,4 kbps y es semidúplex; contar vehículos
obligaba a decenas de mensajes por minuto compitiendo con el `CMD_GO_RED`. **Un bit no compite con
nada**, y no hace falta trama nueva: el `param` de `CMD_ACK_RED` viaja hoy a 0 y **el Maestro no lo
lee** [medido el 27/08], y llega **exactamente en el instante que importa**: cuando el Esclavo
confirma que ya está en rojo y empieza a correr el todo-rojo. **Coste en aire: cero.**

**La máquina:** `param` bit0 = 0 (tramo libre) ⇒ todo-rojo normal y al acabar, VERDE. `param` bit0 = 1
(aún ocupado) ⇒ **EXTIENDE** el todo-rojo; si se libera antes del tope, VERDE; si se llega al tope,
**VERDE IGUAL** más `$ALARM`.

> 🔴 **EL TOPE NO ES UN DETALLE: ES LA REGLA.** Barro en el lente, un camión aparcado en el punto de
> vigilancia o un sensor pegado en activo, y **el cruce se congela para siempre** esperando que se
> libere. Un enclavamiento sin tope no es más seguro: es un semáforo colgado, con cola en las dos
> puntas y nadie entendiendo por qué. La regla es **extender hasta un máximo configurable; al
> llegar, cambiar igual y levantar alarma** en la caja negra
> (`$ALARM,EVENTO:PRESENCIA_PEGADA,...`).

**La segunda función, que es local y no toca la radio: no bajar la pluma sobre un vehículo.** El
mismo sensor, leído por el propio poste, impide bajar la barrera mientras haya algo debajo. Aquí el
fallo seguro va al revés que arriba: un sensor **pegado en ACTIVO** deja la pluma sin bajar —la
barrera deja de proteger pero **la luz sigue regulando**, con alarma: aceptable—; un sensor **pegado
en INACTIVO** baja la pluma sobre un coche: **no lo es**. Por eso el sensor va **normalmente
cerrado** si el modelo lo permite: **un cable cortado se lee como "hay algo"**.

### Dónde entra físicamente

**Decidido el 27/08: la entrada es `PA11` (pad 32 del `U1`).** Se midió el cobre del `.kicad_pcb` y
lo dice el propio enrutador: `PA11`, `PA12`, `PA15` y `PC13` salen como
`unconnected-(U1-PA11-Pad32)` y equivalentes —**pads sin una sola pista**—, mientras los ocupados
aparecen con su red. **Por qué `PA11` y no otro:** es tolerante a 5 V, **no depende de que el JTAG
esté desactivado** —a diferencia de `PA15`— y no vive en el dominio de respaldo con sus
limitaciones, como `PC13`. `PA12` queda de reserva.

**Cómo se cabla:** pad 32 (`PA11`) con hilo AWG30 fijado con kapton a una vía muerta de `J16` y de
ahí a bornera. **Sensor NORMALMENTE CERRADO a masa:** en reposo el contacto está cerrado y el pin a
masa (LOW); con vehículo el contacto ABRE y el pin sube por el pull-up interno (HIGH = presencia).
**Un cable cortado se lee como "hay algo"** — el todo-rojo se extiende y la pluma no baja: el fallo
del cableado cae del lado seguro, y el TOPE impide que un sensor averiado congele el cruce.

> ⚠️ **Lo que YA NO hace falta un hilo a un pad para tener: entrada eléctrica.** El reparto de `J16`
> (N-97) dio a las dos puntas **dos entradas de cámara** ya configuradas y leídas en cada vuelta:
> `CAM_C_PIN`/`PB14` (`J16` p10) y `CAM_D_PIN`/`PB15` (`J16` p12), `INPUT` pelado y activas en ALTO
> (`*/include/pines.h`), que entran por `demanda_solicitar()` **por flanco en las dos puntas**, con
> siembra del nivel al arrancar. Con `PB0` en `J14` son **tres**. Lo que falta no es una entrada:
> es que **una de ellas signifique PRESENCIA en vez de DEMANDA**, y **son cosas opuestas** —una
> demanda **pide** el verde, una presencia **lo retrasa**—. Reusar una entrada de demanda como
> presencia sin cambiar quién la lee sería pedir paso justo cuando hay que negarlo. `PA11` sigue
> siendo la elección correcta para una entrada **con bornera propia**.
>
> ⚠️ **Y el veto al cableado de `J16` está DEROGADO por `D-25` y `D-27` (11/09):** las cuatro cámaras
> van a `J16` —cámara 1 entre p9 (3,3 V) y p10, cámara 2 entre p11 (3,3 V) y p12, conmutando **hacia
> +3,3 V y nunca contra masa**—, está cerrado por el responsable y las guías ya se lo mandan al
> instalador. La polaridad se resolvió: `pinMode(..., INPUT)` pelado y `digitalRead() == HIGH`, con
> 9,93/9,94 kΩ a masa y 0 V en reposo **medidos en cobre el 03/09**. De aquella familia de medidas
> queda **`M4` y sólo eso**: nadie ha puesto un multímetro en **p9**, y sigue marcado ⬜ banco en
> `MAPEO_TARJETA_KICAD.md`. Es una comprobación de banco, **no un veto al cableado**.

### Lo que hay que medir ANTES de escribir una línea

- **El flash.** El bit y la máquina de extensión son pocas decenas de bytes; la alarma y el texto de
  pantalla, más. **Se compila con y sin, y se compara el acta** (`ls -t evidencia/*_compuerta.txt`).
  Sin esa cifra, cualquier estimación de aquí es una intuición — y **un número de flash escrito en
  prosa caduca cada vez que alguien compila**.
- **La polaridad del sensor**, como en N-67: la manda la placa, no el gusto de nadie.

### El pack que lo vigilará, y la comprobación que nadie escribe

`presencia_01_veto` tendrá que exigir, sobre el C++ real:

1. Que la presencia **sólo extienda**: que no exista ningún camino donde acorte el todo-rojo. Es la
   propiedad central, y se mide sobre las escrituras de pin como SFTY-2.
2. Que el bit se lea del `param` de `CMD_ACK_RED` y de ningún otro sitio.
3. **Que el tope funcione** — y éste es el control negativo que se olvida siempre: con la presencia
   forzada a activa **para siempre**, el cruce **tiene que cambiar igual** al llegar al máximo y
   emitir la alarma. Un pack que sólo pruebe el caso bueno estaría certificando el cuelgue.
4. Que la pluma no baje con presencia, y que un sensor ausente no la deje bajar sola.

**Lo que esta regla NO hace:** no cuenta vehículos, no acorta el despeje, no sustituye al
temporizador y **no se apoya en la radio para nada crítico** —si el bit no llega, el Maestro hace
exactamente lo que hace hoy—.

---

## 🚧 SFTY-28 — Talanquera acoplada al estado del semáforo (**IMPLEMENTADA la regla; abiertas las decisiones de operación**)

> ⏸️ **PENDIENTE ANOTADO EL 05/09/2026 — NO EJECUTADO AQUÍ, A PROPÓSITO.** El responsable dijo que
> *«la barrera puede no bajar y el semáforo cambia igual»*, lo que **derogaría** el sentido único de
> esta regla. Pero la derogación formal es **`A-1.bis` de [`DECISIONES.md`](DECISIONES.md)**, y va
> con la fase 2 de `D-13`, en otro lote. **Esta sección no se reescribe todavía**: una frase nueva no
> deroga una decisión escrita, y con más razón porque el cambio **retira una barrera** —el veto de la
> pluma—, que es la dirección en la que un malentendido no se nota hasta que alguien está en la
> calzada.

**Estado:** anotado el 26/08/2026 y **construido el 27/08** en las dos puntas. La orden sale de
`escribirPines()` —**la misma puerta que las lámparas**— y sigue al `verde` YA enclavado; el arranque
la deja cerrada; **el nivel de reposo del pin es el de CERRAR**, de modo que un equipo apagado no
deja la vía abierta. Coste: **+24 bytes por punta**, sin drivers ni bus.

**Lo que lo mide, en dos planos distintos:** el arnés del automático vigila en CADA tick de los nueve
bloques que la pluma nunca esté arriba con los dos verdes apagados —sobre lo que el `semaforo.cpp`
real escribió en el pin, con su control negativo al lado—, y el pack `barrera_03_talanquera` fija la
estructura que el arnés no puede ver: que **ningún otro** `.cpp` de las dos puntas escriba ese pin
—censando el directorio, no una lista—, que la orden viva **dentro de** `escribirPines()` y no en una
función suelta, y que las dos puntas lo hagan igual. Visto caer con la pluma forzada a ABRIR en el
`.cpp` real.

### 🛑 La regla, y es la misma que gobierna todo lo demás aquí

> **La talanquera SIGUE al semáforo. Nunca lo manda, y nunca lo contradice.**

| Luz | Talanquera |
|---|---|
| 🔴 Rojo | **BAJADA** |
| 🟡 Ámbar (transición) | **BAJADA** — sólo sube cuando el verde está confirmado |
| 🟢 Verde | **SUBIDA** |
| Todo-rojo de despeje | **BAJADA en las dos puntas** |
| Ámbar intermitente (SFTY-6, sin enlace) | ✅ **SUBIDA** — decidido por el cliente el 27/08/2026: sin enlace se deja pasar con precaución, que es lo que ese ámbar significa en la calle. Cerrar la vía dejaría un corredor de obra sin salida |

**Y la consecuencia estructural, que es lo que la hace segura: la orden de la talanquera sale del
mismo sitio que la luz, `semaforo.cpp`, y de ningún otro.** Es `§2` de `CLAUDE.md` extendida: si un
modo pudiera mover la barrera por su cuenta, tendríamos una barrera abierta con la luz en rojo, que
es **peor** que no tener barrera — porque el conductor confía en ella.

**Para qué sirve una talanquera aquí:** en un paso alternado la barrera física hace lo que la luz no
puede, **detener al que no mira**. En obra con maquinaria pesada, de noche, o con conductores que ya
han visto veinte semáforos de obra esa semana, el rojo se salta. Una barrera, no.

### ⚠️ Lo que sigue ABIERTO, y no lo decide el firmware

1. **¿Qué hace al perder el enlace?** Hoy la tabla dice SUBIDA por decisión del cliente, pero
   *«cerrar la vía por completo»* frente a *«dejar pasar con precaución»* sigue siendo decisión del
   cliente y del PMT. Cambiarla es **una línea** en `escribirPines()`, y el día que se cambie hay
   que actualizar la tabla de arriba y el Manual 1.
2. **¿Qué hace al cortarse la energía?** Una talanquera que se queda arriba con el equipo muerto es
   una vía abierta sin regulación. **Esto no se resuelve en software:** se resuelve eligiendo un
   actuador que caiga por gravedad o con muelle de retorno. **Va en la especificación de compra.**
3. **¿Y si no llega a bajar?** Un final de carrera convierte *«ordené bajar»* en *«está bajada»*, que
   no es lo mismo. Sin realimentación el firmware no puede saberlo y **no debe fingir que sí**: si no
   hay final de carrera, el manual tiene que decir que la barrera es una ayuda visual y que **quien
   regula sigue siendo la luz**.
4. 🔴 **A dónde sale `PB2` en el cobre sigue por confirmar con multímetro.** El firmware ya escribe
   el pin; que ese pin mueva un motor es lo que falta comprobar. *(El puntero que este documento
   llevaba —«tarea `B3` de `ESTADO.md`»— está roto: `B3` ya no existe allí.)*

### 🔌 En qué salida se acciona — MEDIDO sobre el esquemático

```
   pines.h   #define MOTOR_TALANQUERA   PB2   // J15
```

Medido sobre el plano bueno (`01_Firmware/Controladora_Semaforos/.../*.kicad_sch`, 649.224 B): son
**DIEZ MOSFET y DIEZ optos** (`Q1..Q10`, `U6..U15`), y los conectores llegan a `J17`. **No hay
disputa de drivers: el buzzer tiene `Q8` y la talanquera `Q10`.**

| red | por dónde pasa |
|---|---|
| `S1`…`S8` | R 220 + R 10K → opto `TLP127` → MOSFET `IRLZ44N` → bornera. Ocho canales idénticos |
| `Buzzer` (`PB1`) | `R55`+`R54` → opto `U13` → MOSFET `Q8` → bornera `J13` con 12 V |
| `Motor` (**`PB2`**) | su propio canal completo: `R70`+`R69` → opto **`U15`** → MOSFET **`Q10`** → bornera **`J15`** |
| `Puerta` (`PB0`) | `R64` 10 kΩ + `C25` 100 nF → bornera `J14`. Es un **RC de 1 ms: una ENTRADA con antirrebote por hardware**, donde el firmware lee la cámara de demanda |
| `PB8` | `R16` 1 kΩ → **LED `D5`**. Es un indicador, no una entrada optoacoplada de cámara |
| `PA11`, `PA12`, `PA15`, `PC13` | **sin cable**: los únicos pines realmente libres, y sin bornera |

**`MOTOR_TALANQUERA = PB2` es exacto**, no hace falta mover nada, no hace falta módulo de relé
externo y no hay que sacrificar el buzzer. **`J15` existe** —`J1`..`J17` en el plano bueno—: la lista
`J1..J14` y `J16` que este documento llegó a publicar era del plano incompleto.

### ⚠️ Y un MOSFET no basta para una talanquera

Un `IRLZ44N` de canal N conmuta **encendido/apagado** contra masa; un motor de barrera necesita **dos
sentidos**, y eso no sale de un solo transistor.

| Opción | Salidas que consume | Nota |
|---|---|---|
| **Barrera con controlador propio** que acepte contacto seco de apertura | **1** | Es el caso industrial habitual y **el único que cabe hoy**. El controlador de la barrera se ocupa del motor, los finales de carrera y la seguridad antiaplastamiento |
| Dos salidas de pulso, abrir y cerrar | 2 | No caben sin tocar la placa |
| Puente en H externo | 1-2 + placa auxiliar | Se asume el control del motor, y con él la responsabilidad de no aplastar a nadie |

**La primera es la única que cabe en la placa actual, y además es la correcta:** dejar el motor y su
seguridad al equipo diseñado para eso, y que el semáforo sólo diga *abre* o *cierra*. **Lo caro no es
el código: son las tres decisiones de arriba y el actuador.**

**Criterio de aceptación:** pack que barra **todos** los estados del semáforo y exija que la
talanquera nunca esté arriba con el verde apagado —con su control negativo, forzando el caso
contrario—. Y en banco: **cortar la energía con la barrera arriba y comprobar que baja sola.**

> 🔴 **N-96, y vive aquí porque lo destapó este apartado: `ROJO_PEATON` (`PA6`), `VERDE_PEATON`
> (`PA7`) y el `BUZZER` (`PB1`) son hardware pagado y MUERTO.** Están declarados en
> `*/include/pines.h` y **no tienen ni un `pinMode`, ni un `digitalWrite`, ni un `digitalRead` en
> ninguna de las dos puntas**; el censo devuelve **cero** llamadas. Su parte incómoda no es el
> hardware muerto: es que **`barrera_01_pines_de_luz` no podía detectarlo** —aceptaba
> `len(luces) >= 6` sobre una lista que devuelve 8, y su control negativo nunca ejerció un pin
> peatonal—. **Una regla de seguridad que ENUMERA sujetos tiene que comprobar que cada sujeto
> existe.**
