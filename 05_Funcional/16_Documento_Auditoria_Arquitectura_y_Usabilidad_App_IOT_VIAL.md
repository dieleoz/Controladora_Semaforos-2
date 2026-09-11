# 📋 INFORME DE AUDITORÍA, ARQUITECTURA Y ESPECIFICACIÓN TÉCNICA · APP IOT-VIAL V9.0

> # 🛑 07/09/2026 — ESTO ES UNA FOTO DEL 28/08, NO LA ESPECIFICACIÓN VIGENTE
>
> **Léase esta caja antes que ninguna otra línea del documento.**
>
> Este fichero es el **informe de una auditoría cerrada el 28 de agosto**. Su título dice
> *«ESPECIFICACIÓN TÉCNICA»* y **no lo es**: nada de lo que hay debajo se ha mantenido al día desde
> esa fecha, y **diez días de decisiones han pasado por encima**.
>
> ## Quién gana sobre cada cosa — y este documento no gana sobre ninguna
>
> | sobre… | manda | aquí |
> |---|---|---|
> | lo **decidido** | 🔴 **`DECISIONES.md`** | *(este fichero **no lo citaba ni una vez**: `grep -c 'DECISIONES.md'` → **0**, medido el 07/09. Es exactamente lo que hace que traiga lo caducado)* |
> | lo que la **app hace** | `05_Funcional/14_Manual_App_Movil_IOT_VIAL.md` y el fuente en `App_Semaforo/` | resumido y **desactualizado** |
> | la **trama** y sus checksums | `05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.md` §4.2 y §4.3 | 🛑 **§3.2 ya dejó de publicar ejemplos propios, y por buen motivo: los tres que traía tenían el checksum mal** |
> | el **hardware medido** | `05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md` | no se toca aquí |
> | las **cifras de los instrumentos** | 🔴 **el acta**: `ls -t evidencia/*_compuerta.txt \| head -1` | 🛑 **todas las de §4 están caducadas — ver §4** |
>
> ## Lo que se ha tachado hoy, y por qué NO se ha borrado
>
> Se corrigen **en el sitio, con su motivo y su fecha**, seis afirmaciones que hoy son falsas:
> los rangos de ciclo de §2.1 y §7 (`D-5`), el `SET_RTC` de §3.1 (`D-15`), la botonera de §1
> (`D-1` / `D-16`), las tres cifras de instrumentos de §4, la casilla de checksum de §7 y las rutas
> de §4.2 y §5. **Nada se borra**: un informe de auditoría es un registro fechado, y una línea que
> desaparece en silencio vuelve a escribirse.
>
> 🟡 **CANDIDATO A ARCHIVARSE, y no lo decide este agente.** Todo lo que este fichero aporta vivo
> está hoy mejor contado en el **14** y en el **10**. Lo que sólo vive aquí es **el registro de la
> auditoría del 28/08** — que es un motivo legítimo para conservarlo, y ninguno para leerlo como
> especificación. **Mover esto a `99_Legacy/` es una decisión del responsable, no una limpieza.**

**Rama Git:** ~~`feat/n69-ajustes-tiempos`~~ → el trabajo vive en **`main-nuevo`** *(07/09; la rama vieja existe y está parada)*  
**Destinatario:** Agente Auditor / QA Lead / Ingeniero de Tráfico y Hardware  
**Fecha:** 28 de Agosto de 2026  
**Sistema:** Controladora de Semáforos Móviles de 3 Estados (Maestro / Esclavo)  
**Entregable APK:** `05_Funcional/IOT_VIAL_Semaforos_2026-08-28_a8e1ceb_SIN_BANCO.apk`  

---

## 1. 📌 RESUMEN EJECUTIVO Y ANTECEDENTES
El sistema original de control semafórico operaba en obra con controles físicos de relé (4 botones) o mediante acceso directo a la consola de la placa STM32. La versión previa de la App presentaba fricción de usabilidad (menús complejos para el operario de obra y falta de separación de roles).

En esta iteración se logró:
1. **Rediseño Ergonómico y Separación de Roles (Operario vs Técnico):**
   * **Modo Operario (0 contraseñas):** ~~Botonera de campo de 4 pulsadores gigantes (Automático, Dar Paso / Alternar, Ámbar Precaución, Rojo Total de Emergencia).~~ → 🔴 **07/09: son SIETE rótulos, no cuatro**, y **tres salen según la punta conectada** — `AUTOMATICO · DAR PASO · AMBAR · ROJO TOTAL · AMBAR EMERGENCIA · RETIRAR AMBAR · VOLVER AL MENU`. Censados sobre `index.html`; la lista viva está en el **Manual 14 §1**.
   * **Modo Técnico / Admin (PIN 1234):** Desbloquea ajustes de tiempos de ciclo (Verde, Rojo, Despeje), Asistente Courier RTC, prueba de potencia de focos (6s) y gestor de cruces.

> # 🔴 07/09 — LO QUE ESTE RESUMEN NO PODÍA DECIR EN AGOSTO, Y HOY MANDA SOBRE TODO LO DEMÁS
>
> **`D-16`: SIN TELÉFONO NO HAY FORMA DE OPERAR EL EQUIPO.** El párrafo de arriba abre diciendo que
> *«el sistema original operaba con controles físicos de relé (4 botones) o acceso directo a la
> consola»*. **Las dos vías se han retirado desde entonces:**
>
> * **el mando de 4 relés** — `D-1` (31/08, hardware confirmado retirado el 05/09);
> * **la pantalla LCD y su menú** — `D-17.bis` (05/09), y con ellos `botonAceptar()`/`botonCancelar()`,
>   que hoy son `return false;`.
>
> **O sea que la app dejó de ser «la que evita la fricción» para ser LA ÚNICA SUPERFICIE DE MANDO.**
> Ni ámbar, ni volver a automático, ni parar el cruce sin un teléfono emparejado y con batería.
> **Este documento describe la app como una mejora de usabilidad; hoy es el equipo entero.** El
> desarrollo completo, con lo que eso obliga a llevar a campo, está en el **Manual 14**, primera
> caja.
2. **Arnés de Transporte Dual para Bluetooth:**
   * Soporte nativo para sockets RFCOMM SPP a 9600 bps en Android vía `cordova-plugin-bluetooth-serial`, contra el módulo de expansión **`ESP32-WROOM-32` clásico** (`BR/EDR`, perfil SPP).

     > 🛑 **Corregido: aquí decía `HC-05 / JDY-31`. El `JDY-31` está PROHIBIDO por su nombre** —es
     > **BLE**, y esta app conecta por **SPP** (Bluetooth clásico): con un `JDY-31` no empareja y
     > habría que rehacer el puente nativo. Ver `04_Manuales/MANUAL_CONFIGURACION_BLUETOOTH.md` §1.
     > **No se compra ni se prueba con un `JDY-31`.**
   * Servidor puente multihilo en Python (`servidor_puente_simulador.py`) para emulación Hardware-in-the-Loop desde el navegador Web.
3. **Batería de Pruebas Automatizadas:**
   * 29 Pruebas Unitarias TDD (100% PASS).
   * Validación Visual E2E con Puppeteer y capturas en alta resolución en `evidencia/`.
   * 5 Suites de estrés en Python (simulación de 6 meses continuos y 50.000 ataques de fuerza bruta).

---

## 2. 🏛️ ARQUITECTURA DEL SISTEMA Y DIAGRAMAS

### 2.1 Arquitectura de Roles
![Arquitectura de 2 Roles](./graficas/grafica_01_arquitectura_roles.png)

* **Operario de Campo:** No requiere ingresar a menús ni memorizar PINs. Puede reanudar el ciclo autónomo, alternar el sentido de paso respetando el despeje todo-rojo de seguridad, activar ámbar destellante o detener el tráfico en emergencia total.
* **Técnico / Administrador:** Protegido por PIN `1234`. Permite parametrizar tiempos nominales (~~1-15m Verde/Rojo~~ → 🔴 **3-15 min Verde/Rojo desde el 04/09**, `D-5`: *«por debajo, el conductor se convence de que el semáforo está averiado y adelanta en rojo»*. La guarda de verdad **está en el firmware** —`VERDE_MIN_MIN = 3` en `Maestro/include/limites_ciclo.h`— y rechaza con `$ERR,CMD:SET_TIEMPOS,DESC:RANGO`; 10-90s Despeje), realizar pruebas de potencia en MOSFETs y ~~sincronizar relojes RTC DS3231~~ **poner en hora el `DS3231` de ~~DEL POSTE 1, y sólo el del poste 1~~ LOS DOS POSTES** (~~🔴 `D-20`, 07/09: **la app NO pone la hora en el poste 2, nunca** — un `SET_RTC` dirigido al Esclavo **se rechaza**, porque no es una sincronización sino una **segunda fuente**. El poste 2 recibe la hora del Maestro por la cadena `ESP32-M -> STM32-M -> radio -> STM32-E -> ESP32-E`. ⚠️ **SIN CONSTRUIR** — ver §2.2.~~ 🔵 11/09, `D-26`, construida en `68dd2c5` y sin banco: con radio el poste 2 hace caso a la hora del Maestro; **sin radio, a la de su propio `DS3231`**, que el técnico le pone desde el teléfono en su gabinete. **Consultar** los dos relojes con `CMD:LEER_RTC` sí sigue siendo tarea del técnico: leer no escribe).

### 2.2 Flujo del Asistente Courier RTC
![Flujo Courier RTC](./graficas/grafica_02_courier_rtc_flujo.png)

Para obras viales donde la topografía bloquea la señal de radio entre Maestro y Esclavo:
~~$$\text{Hora Inyectada en Esclavo} = \text{Hora Capturada en Maestro} + \Delta t_{\text{traslado}}$$~~
~~El cronómetro en la App contabiliza los segundos de viaje y programa el reloj DS3231 del Esclavo con la hora exacta compensada.~~

> # 🔴 07/09 — LA FÓRMULA DE ARRIBA QUEDA TACHADA ENTERA POR `D-20`, NO SÓLO SU PRIMER TÉRMINO
>
> **Antes se corrigió el término `Hora Capturada en Maestro` (ver abajo). `D-20` va más lejos: quita
> el procedimiento completo.**
>
> > **LA AUTORIDAD DE LA HORA ES EL ESP32, SIEMPRE Y PARA TODO.** La app se la da al **ESP32
> > Maestro**; ése al **ESP32 Esclavo**; y el STM32 de cada punta la recibe **de su propio ESP32**.
> > **Hay UNA sola fuente**, así que **no hay desfase inicial que acotar** — que era la única
> > objeción del arquitecto. ~~🔴 **Consecuencia dura: la app NO pone la hora en el poste 2. Nunca.**
> > Un `SET_RTC` dirigido al Esclavo **se rechaza**: no es una sincronización, **es una segunda
> > fuente**.~~
>
> 🛑 **11/09 — la frase tachada de arriba está CADUCADA** (ya tachada en la propia fila `D-20` el 07/09
> por la noche: *«la barrera es la SOBREESCRITURA»*), **y `D-26` (11/09, construida en `68dd2c5`, sin
> banco) manda lo contrario:** con radio, el poste 2 hace caso a la hora del Maestro; **sin radio, toma
> la de su propio ESP32, que el técnico le pone desde el teléfono en su gabinete**. Lo que se va con el
> Courier es **el viaje cronometrado y su compensación**, no poner la hora en el poste 2.
> `14_Manual_App_Movil_IOT_VIAL.md` §5.3.bis.
>
> **El Courier RTC es, por definición, una segunda fuente**: lleva la hora del teléfono hasta el
> poste 2 y la inyecta allí. Con `D-20`, **ese gesto está prohibido**, y con él se va el $\Delta
> t_{\text{traslado}}$: **no hay traslado que compensar porque no hay nada que trasladar a mano.**
>
> **La topología, que es lo que sustituye a la fórmula:** los dos ESP32 **no se hablan**. El único
> enlace entre postes es la **radio entre los STM32**, así que la hora viaja
>
> ```
> ESP32-M  ->  STM32-M  ->  radio  ->  STM32-E  ->  ESP32-E
> ```
>
> **Los STM32 son CARTEROS de la hora, no dueños.**
>
> 🖼️ **Y por tanto `graficas/grafica_02_courier_rtc_flujo.png` PUBLICA UN FLUJO DESMENTIDO.** La
> imagen sigue en el documento y **no se ha regenerado**: dibuja el Courier —teléfono → poste 2—,
> que es justo lo que `D-20` prohíbe. **Léase la imagen como histórico, no como procedimiento.**
> *(Se deja escrito en vez de corregido a propósito: regenerar gráficas no es trabajo de esta
> revisión, y una imagen borrada en silencio no deja rastro de que estuvo mal.)*
>
> ~~⚠️ **`D-20` ESTÁ DECIDIDA Y SIN CONSTRUIR.** Falta el mando ESP32 → STM32 que siembre la hora —el
> camino físico existe (`ESP32_Expansion/src/enlace_stm32.cpp`), **el mando no**: ese fichero no
> menciona `RTC` ni `hora` ni una vez— y falta el rechazo del `SET_RTC` en el poste 2 — el puente es
> **el mismo firmware en los dos postes** y su despachador **no sabe en cuál está**:
> `grep -c "ESCLAVO\|Esclavo\|esclavo" 01_Firmware/ESP32_Expansion/src/despachador.cpp` → `0`
> (corrido el 07/09). **Hasta que se construya, la barrera es el procedimiento: no se manda `SET_RTC`
> al poste 2.**~~ 🟢 **11/09: el mando existe** (`CMD:HORA_ESP32`, `siembra.cpp`, `68dd2c5`, sin banco) y
> **el rechazo no se construye**: `D-26` manda ponerle la hora al poste 2 cuando cae la radio.
>
> ✅ **Lo que `D-20` NO deroga, para que nadie lo retire de paso:**
> **`CMD:LEER_RTC` (`D-17`) se sigue mandando A LOS DOS POSTES.** `D-20` prohíbe **ESCRIBIR** la hora
> en el poste 2; **leerla no escribe nada**. Con `D-20` construida vale **más** que hoy: es la única
> forma de comprobar que la siembra del Maestro llegó de verdad al reloj del poste 2.
> **Y siguen haciendo falta DOS `DS3231`, uno por poste (`A-5`)**: el del Esclavo es el que conserva
> la hora con su pila cuando se cae la radio. **`D-20` no reduce la compra a uno.**

> 🔴 **07/09: LA FÓRMULA DE ARRIBA TIENE MAL EL PRIMER TÉRMINO, y el Manual 14 §4 lo dejó medido el
> 05/09.** *«Hora Capturada en Maestro»* **no es la hora del Maestro: es la hora DEL TELÉFONO**
> (`horaLocal24()`, que es `new Date()`). Durante todo el procedimiento **al Maestro no se le manda
> nada**. El celular se usa **como fuente de tiempo**, no como copiadora del reloj del Poste 1 —
> **si el teléfono va desfasado, el Esclavo queda desfasado igual.**
>
> 🔴 **Y hay dos cosas más que este apartado ya no puede afirmar:**
>
> 1. **`D-15` (05/09): el reloj lo lleva el `DS3231` del ESP32 de cada punta, y es el ÚNICO que
>    contesta a `SET_RTC`.** La controladora consume la orden y **ya no acusa**. ~~Hay **dos relojes
>    por cruce**, no uno que se inyecta desde el otro.~~
>    > 🔴 **07/09 — ESA ÚLTIMA FRASE LA ESCRIBÍ HOY MISMO Y `D-20` LA DEROGA EL MISMO DÍA. Se tacha,
>    > no se borra.** Siguen siendo **dos `DS3231`, uno por poste** (`A-5`, y **la compra no cambia**),
>    > pero **la FUENTE de la hora es UNA SOLA: el ESP32 Maestro.** El del poste 2 **no es una segunda
>    > fuente: es la MEMORIA de la única fuente**, la que conserva la hora con su pila cuando se cae
>    > la radio. Y sí, **es exactamente «uno que se inyecta desde el otro»** — sólo que no a mano con
>    > un teléfono, sino por la cadena
>    > `ESP32-M -> STM32-M -> radio -> STM32-E -> ESP32-E`, con los STM32 de **carteros**.
>    > ⚠️ **SIN CONSTRUIR** — ver el recuadro grande de §2.2.
> 2. **`D-17` (05/09): hay una vía mejor que el Courier para lo que casi siempre se quiere —
>    `CMD:LEER_RTC`, consultar el reloj SIN cambiarlo**, y ver el desfase entre postes. *«No hace
>    falta que los dos relojes se pongan de acuerdo solos: hace falta poder ver si lo están.»* Ver
>    Manual 14 §5.7.
>
> ⚠️ Y lo que sigue abierto tal cual: **el `FORMATO_INVALIDO` del Courier RTC SIGUE SIN
> DIAGNOSTICAR.** Nadie sabe por qué se produce, y aquí no se propone ninguna causa.

### 2.3 Stack Tecnológico y Pipeline
![Stack Tecnológico](./graficas/grafica_03_stack_tecnologico_compilacion.png)

---

## 3. 📡 ESPECIFICACIÓN DEL PROTOCOLO DE COMUNICACIÓN (CONTRATO STM32)

### 3.1 Formato de Tramas Emitidas hacia el Microcontrolador
Todos los comandos viajan en ASCII delimitados por `\r\n`:

```text
CMD:PIN:1234:SET_MODO:AUTO\r\n           -> Activa modo automático
CMD:PIN:1234:SET_MODO:MANUAL\r\n         -> Activa modo manual
CMD:PIN:1234:MANUAL:CAMBIAR_TURNO\r\n    -> Alterna paso al sentido opuesto
CMD:PIN:1234:SET_MODO:AMBAR\r\n          -> Destello de precaución 1 Hz
CMD:PIN:1234:FORZAR_ROJO\r\n             -> Rojo total en ambos sentidos
CMD:FORZAR_ROJO\r\n                      -> Excepción de seguridad (Rojo sin PIN)
CMD:PIN:1234:SET_TIEMPOS:3,3,15\r\n      -> Verde: 3m, Rojo: 3m, Despeje: 15s (minimos vigentes)
   (aqui ponia 2,2,15 y el equipo lo RECHAZA: 2 < VERDE_MIN_MIN = 3 -> $ERR,...,DESC:RANGO)
CMD:PIN:1234:SET_RTC:2026-08-28,14:30:00 -> Sincroniza RTC DS3231
   (05/09, D-15: LA CONTROLADORA YA NO CONTESTA A ESTA ORDEN. La consume para no
    responder "comando desconocido" y deja un evento; EL ACUSE LO DA EL PUENTE,
    que es quien tiene el reloj. Y desde D-17 hay CMD:LEER_RTC, que lo consulta
    SIN cambiarlo -y esa ni siquiera llega a la controladora-.)
CMD:PIN:1234:TEST_LEDS\r\n               -> Secuencia de prueba 6s
```

> 🛑 **Esta lista son OCHO formas y NO es el censo.** El Maestro despacha **veinte ramas** y el
> Esclavo tiene **su propio juego, que no es el mismo** —`FORZAR_ROJO` ni siquiera existe allí: se
> contesta `$ERR,…,DESC:RENOMBRADO_USE_AMBAR_EMERGENCIA`—. **El censo vivo, regenerable con un
> `grep`, está en el Manual 14 §5.1 y §5.2.** Lo de aquí es lo que la app componía el 28/08.
>
> ```bash
> grep -n 'strcmp(accion\|strncmp(accion\|strcmp(cmd\|strncmp(cmd' 01_Firmware/Maestro/src/bluetooth.cpp
> ```
>
> *(Re-corrido el 07/09: **20 líneas** en el Maestro y **11** en el Esclavo.)*

### 3.2 Formato de Respuestas y Telemetría Emitida por STM32
> 🛑 **LOS TRES EJEMPLOS QUE HABÍA AQUÍ TENÍAN EL CHECKSUM MAL, Y LA TERCERA TRAMA NO ES LA QUE
> EMITE EL MICRO. Se tachan con su motivo — 05/09.**
>
> ```text
> ~~$ACK,CMD:SET_MODO:AUTO,RESULT:OK*2E\r\n~~                              publicado *2E · real *2F
> ~~$ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO*3A\r\n~~                        publicado *3A · real *5C
> ~~$STATUS,...,RESTANTE:38,TOT:45,BAT:12.6,...,SERIE:M-2026-A1B2*1F\r\n~~ publicado *1F · real *45
> ```
>
> **Recalculado el 05/09 con la regla que este mismo apartado publica dos líneas más abajo** —XOR
> de 8 bits de todo lo que va entre `$` y `*`—: **los tres fallan.** En el apartado que explica cómo
> se calcula el checksum, y con la casilla de §7 marcada afirmando que se validó.
>
> **Y la tercera trama está mal más allá del checksum** (medido contra
> `Maestro/src/bluetooth.cpp:970` e `identidad.cpp:63`):
>
> | lo que ponía | lo que emite el equipo |
> |---|---|
> | `RESTANTE:38,TOT:45` | **no existen**: el campo es `T:` |
> | *(faltaban)* | `HORA:` y **`ESC:`**, que va el último y sólo en el Maestro |
> | `SERIE:M-2026-A1B2` | son **24 bits en hexadecimal** (`& 0xFFFFFFUL`), p. ej. `A3F19C` |
> | `BAT:12.6` | **`BAT:--`** — ver N-108: no hay un solo `analogRead()` en las cuatro carpetas |
>
> 🔴 **Este apartado DEJA DE PUBLICAR EJEMPLOS PROPIOS, y ése es el arreglo de verdad.** Una segunda
> copia a mano de un formato que el firmware ya define es lo que produjo estos tres —es `CLAUDE.md`
> §3.bis: *un dato repetido a mano es una copia que alguien tiene que sincronizar*—. **La trama
> vigente, con sus campos y sus checksums recalculados en cada corrida por
> `documentos_03_trama_status`, vive en `10_Manual_Modulo_Bluetooth_Telemetria.md` §4.2 y §4.3, y
> en ningún otro sitio.**
* **Delimitadores:** Inicia con `$` y finaliza con `*` seguido de 2 caracteres hexadecimales de Checksum XOR de 8 bits.

---

## 4. 🧪 PLAN DE PRUEBAS Y RESULTADOS DE COBERTURA

> # 🔴 07/09 — LAS TRES CIFRAS DE ESTE APARTADO ESTÁN CADUCADAS, Y LA CURA NO ES ACTUALIZARLAS
>
> **Se tachan una a una abajo. Lo que las sustituye no es un número nuevo: es el sitio de donde se
> leen.** El Manual 14 §2.2 ya retiró estas mismas cifras **a propósito** y explica por qué —*«las
> cuentas de la app se mueven cada vez que se añade un caso, y una cifra escrita a mano en un manual
> envejece en silencio»*—. Aquí se hereda ese criterio.
>
> **La fuente única de toda cifra de instrumento es el ACTA de la última corrida:**
>
> ```bash
> ls -t evidencia/*_compuerta.txt | head -1
> ```
>
> 🔴 **Y se lee el acta ENTERA, no el número.** Un `x/y` con `x != y` significa que hay
> comprobaciones que no cumplen, salga con el código que salga; y un `ABORTADO` **no dice nada del
> firmware** — no es un aprobado. **Ninguna de estas cifras, ni las nuevas, dice que la app hable
> con el equipo:** eso sólo lo demuestra una trama recibida, en la pestaña `Tramas`, con el equipo
> delante. `N-122` es el precedente — **los instrumentos estaban en verde mientras la app no abría
> el socket.**

### 4.1 Test Unitarios TDD (`App_Semaforo/tests/test_unitarios.js`)
* **Comando:** `node tests/test_unitarios.js`
* **Resultado:** ~~`29 PASS | 0 FAIL` (100% de Cobertura)~~ → 🔴 **CADUCADO.** El acta del **07/09**
  trae **dos** filas distintas donde este documento veía una: `test unitarios de la app`
  **32 PASS** y `test unitarios TDD de la app` **61 PASS**. **Y el *«100% de Cobertura»* se retira
  sin sustituto: ningún instrumento de este proyecto mide cobertura, y la frase era una afirmación
  que nadie había comprobado.**
* **Módulos Auditados:**
  1. `nmea_parser.js`: Checksum XOR, formateador y parseo de `$STATUS`, `$ALARM`, `$ERR`.
  2. `config.js`: Validación de PIN 1234 y rangos mínimos de seguridad de ciclo.
  3. `courier_rtc.js`: Captura de estado, acumulación de tiempo y compensación horaria.
  4. `site_manager.js`: CRUD completo de frentes de obra en LocalStorage.

### 4.2 Validación Visual E2E (`App_Semaforo/tests/test_e2e_visual.js`)
* **Comando:** `node tests/test_e2e_visual.js`
* **Entorno:** Google Chrome automatizado con Puppeteer.
* **Evidencias Generadas en ~~`evidencia/`~~ → 🔴 `evidencia/old/` (07/09: las ocho rutas de abajo
  estaban ROTAS; los ficheros se movieron):**
  * `01_modo_operario_principal.png`: Dashboard táctil con ~~4~~ botones y réplica 3D. ⚠️ **la botonera de hoy tiene SIETE rótulos: la captura está caducada, no la botonera**
  * `02_modal_cruces_abierto.png`: Listado de frentes de obra.
  * `03_cruce_cambiado_exito.png`: Selección y conmutación de cruce.
  * `04_modal_bluetooth_abierto.png`: Escaneo de dispositivos Bluetooth.
  * `05_nodo_esclavo_conectado.png`: Enlace y telemetría de nodo Esclavo.
  * `06_modo_tecnico_activo.png`: Desbloqueo mediante PIN 1234.
  * `07_tiempos_guardados_exito.png`: Formulario de tiempos validado y guardado.
  * `08_courier_rtc_inyectado.png`: Proceso de inyección horaria compensada.

> ⚠️ **Y las ocho se hicieron a un solo ancho —412 px—, que es el único de los cuatro medidos donde
> el corte del botón de la derecha NO aparece** (11 px a 390, **41 a 360**, **81 a 320**). **Un
> `.png` demuestra que a ESE ancho se veía bien, y nada más.** Manual 14 §6.

### 4.3 Pruebas de Estrés Firmware C++ (`01_Firmware/Simulaciones/simulador_app_bluetooth.py`)
* **Comando:** `python 01_Firmware/Simulaciones/simulador_app_bluetooth.py`
* **Resultado:** ~~`5/5 Suites PASS` (8.640 tramas en 180 días simulados, 50.000 intentos de PIN
  rechazados, 10.000 tramas basura descartadas por fuzzing)~~ → 🔴 **CADUCADO.** El acta del
  **07/09** publica ese instrumento como **`12/12 comprobaciones PASS`** *(fila `simulador de app y
  bluetooth`)*: cambió el número **y la unidad** —ya no cuenta «suites»—, así que actualizar el
  `5` habría dado una cifra con la etiqueta equivocada.

> 🔴 **Y este instrumento tiene un antecedente que hay que leer antes de usar su verde: el 05/09
> estuvo en `ABORTADO`** por emitir el firmware un campo `ESC:` que él no sabía comparar. **Abortó
> bien** —la alternativa, saltarse lo desconocido, habría dado verde midiendo un campo menos, en
> silencio y para siempre—. Un `ABORTADO` **no es un aprobado**: es una puerta abierta mientras dura.

---

## 5. 🔄 PUENTE DE SIMULACIÓN HARDWARE-IN-THE-LOOP (HIL)
* **Script:** `05_Funcional/App_Semaforo/servidor_puente_simulador.py` *(07/09: el fichero existe.
  Se retira el enlace `file:///d:/@Proyect/Controladora_Semaforos/…` que llevaba encima — **apuntaba
  a una carpeta que no es ésta**, le faltaba el ` 2` del nombre, y un enlace absoluto a la máquina
  de quien escribió el documento no le sirve a nadie más)*
* **Servidor:** Multihilo en `http://localhost:3000/`.
* **Endpoints:**
  * `POST /api/cmd`: Recibe los comandos emitidos por la App Web y los procesa en la máquina de estados del STM32.
  * `GET /api/status_json`: Emite telemetría periódica en vivo hacia la interfaz.
  * `GET /api/telemetria`: Emite la trama NMEA pura `$STATUS` con checksum XOR.

---

## 6. 📦 GUÍA DE REPRODUCCIÓN Y COMPILACIÓN

```bash
# 1. Clonar o cambiar a la rama
# 07/09: la rama de abajo EXISTE y esta PARADA. El trabajo vive en main-nuevo.
git checkout main-nuevo        # <-- la buena
# git checkout feat/n69-ajustes-tiempos   <-- 28/08, se conserva por registro

# 2. Ejecutar suite TDD
node 05_Funcional/App_Semaforo/tests/test_unitarios.js

# 3. Ejecutar servidor puente y pruebas E2E
python 05_Funcional/App_Semaforo/servidor_puente_simulador.py 3000
node 05_Funcional/App_Semaforo/tests/test_e2e_visual.js

# 4. Compilar APK Android final
cd 05_Funcional/App_Semaforo
android\compilar_apk.bat
```

---

## 7. 🔍 CHECKLIST PARA EL AGENTE AUDITOR

> 🔴 **07/09 — DOS CASILLAS SE DESMARCAN, Y UNA DE ELLAS LA DESMIENTE ESTE MISMO DOCUMENTO.**
> Una casilla marcada es una afirmación sobre el código; dejar marcada una que el propio fichero
> refuta tres apartados más arriba es `CLAUDE.md` §6 exacto: **la excepción que sostiene el verde es
> el instrumento de verdad, y no la comprueba nadie.**

- [x] Verificar que el perfil Operario no solicita PIN para maniobras de tráfico de campo.
- [x] Verificar que el perfil Técnico exige PIN `1234` para modificar tiempos y memorias.
- [ ] ~~Verificar que la validación de tiempos rechaza valores menores a 1 min (verde/rojo)~~ y menores a 10 seg (despeje todo-rojo). → 🔴 **DESMARCADA: el mínimo ya no es 1 min, son 3** (`D-5`, 04/09). Una casilla marcada sobre el umbral viejo **certifica que el equipo acepta un ciclo que hoy es ilegal**. Vuelve a marcarse cuando se verifique contra `VERDE_MIN_MIN = 3`.
- [ ] ~~Validar que el checksum NMEA XOR coincide exactamente con la rutina de firmware STM32.~~ → 🔴 **DESMARCADA, Y LA PRUEBA ESTÁ EN §3.2 DE ESTE FICHERO:** los **tres** checksums que este documento publicaba estaban **mal los tres** (`*2E` por `*2F`, `*3A` por `*5C`, `*1F` por `*45`), recalculados el 05/09 con la regla que el propio apartado explica. **Esta casilla llevaba marcada desde el 28/08 afirmando lo contrario, en el mismo documento que traía la refutación.** No se vuelve a marcar aquí: la trama y sus checksums viven en `10_Manual_Modulo_Bluetooth_Telemetria.md` §4.2–§4.3, **recalculados en cada corrida por `documentos_03_trama_status`** — y una casilla a mano no puede competir con un pack.
- [x] Validar que el script `servidor_puente_simulador.py` responde en multihilo sin bloqueos.
- [x] APK compilada el 28/08 sobre `4c3f1a3` (`BUILD SUCCESSFUL`, 499 entradas) y **verificada por contenido**: sus 9 ficheros web son identicos byte a byte a los del repositorio. Sigue **SIN BANCO**.

> # 🛑 07/09 — Y LO QUE ESTA CHECKLIST NO PUEDE SEGUIR DEJANDO ENTENDER: LA APK DE ESTE DOCUMENTO NO CONECTA
>
> **La `IOT_VIAL_Semaforos_2026-08-28_a8e1ceb_SIN_BANCO.apk` de la cabecera existe en disco y NO
> SIRVE.** `N-122`, hallado en el banco real del 3–4/09: **la app nunca llamaba a `connect()`** —se
> pintaba *«Enlazado»* por haber pulsado una fila de la lista— así que **ninguna APK anterior al
> 04/09 abre el socket**, por bien que funcione el módulo.
>
> **Ese defecto pasó por debajo de las siete casillas de arriba y por debajo de las tres cifras
> verdes de §4**, porque el censo de funciones sin llamador llevaba `BluetoothDriver` en su lista de
> excepciones con un motivo escrito que era **medio cierto**. **Ningún test podía verlo: la
> excepción decía que no había nada que mirar.**
>
> 🔴 **La lección, y es la que este informe deja como su parte más útil hoy:** *un informe de
> auditoría con siete casillas marcadas y tres cifras en verde puede ser **cierto en cada línea y
> falso en conjunto**.* La APK vigente y su `md5` viven en el `LEEME_PRIMERO.md` de la raíz del
> `.zip`, que es el único sitio donde no caducan. **Y nada de esto ha pasado banco: en campo corre
> la V8.4.**
