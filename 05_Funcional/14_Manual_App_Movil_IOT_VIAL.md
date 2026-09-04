# 📱 MANUAL DE USUARIO Y ARQUITECTURA DE LA APP MÓVIL IOT-VIAL (V9.0)

**Sistema:** Centro de Control, Operación y Diagnóstico Semafórico a Nivel de Suelo  
**Rama Git:** `feat/n69-ajustes-tiempos`  
**Plataforma:** Android Nativo (.APK) y Web Testing PWA  
**Protocolo:** ~~Bluetooth Serial SPP (HC-05 / JDY-31 a 9600 bps) / BLE GATT~~ → **Bluetooth Serial SPP a 9600 bps contra el módulo de expansión `ESP32-WROOM-32`**. Ver el aviso de cabecera  
**Fecha de Actualización:** 28 de Agosto de 2026  
**Última revisión:** 4 de septiembre de 2026 — **banco real del 3–4/09: `N-122`, la app nunca abría el socket Bluetooth**, y **`N-124`, la lista de equipos llevaba `MAC` escritas a mano**. Los dos arreglados, y **obligan a APK nueva**. Ver la cabecera y `§4.bis`  
**Versión de Firmware Compatible:** V8.9 / V9.0 Definitiva — ⚠️ **en campo corre la V8.4**  
**Archivo APK Compilado:** **la APK del 04/09 que acompaña a este paquete** — su nombre exacto y su `md5` estan en `LEEME_PRIMERO.md`, en la raíz del `.zip`, que es el único sitio donde no caducan  
&nbsp;&nbsp;&nbsp;&nbsp; — el nombre lo puede llevar cualquier fichero; el hash no  
&nbsp;&nbsp;&nbsp;&nbsp;🛑 ~~`IOT_VIAL_Semaforos_2026-09-02_617bd00_SIN_BANCO.apk`~~ — **y todas las anteriores: NO CONECTAN** (`N-122`, ver cabecera). No es que les falten funciones: **no abren el socket**  
&nbsp;&nbsp;&nbsp;&nbsp;✏️ *Corregido el 04/09: esta línea citaba `…_2026-09-02_285b18d_…`, **un hash que no existe en ninguna parte**. La APK del 02/09 que sí está en disco es la `617bd00`. Se anota en vez de sustituirse en silencio: una cifra inventada que desaparece sin dejar rastro vuelve a escribirse.*  
**Pestañas:** **2 visibles al operario** (`Tráfico`, `Eventos`) y **5 en modo técnico** — se añaden `Tiempos`, `Técnico` y `Tramas`  

---

> # 🛑 BANCO DEL 3–4/09/2026 — **`N-122`: LA APP NUNCA ABRÍA EL SOCKET BLUETOOTH**
>
> **Es el hallazgo que hay que leer antes que cualquier otra cosa de este manual, porque invalida
> todas las APK anteriores al 04/09.**
>
> ## Qué pasaba
>
> Al pulsar una fila de la lista de equipos, la app **ponía `state.connected = true`**, guardaba la
> `MAC` y llamaba a `subscribe()` — **sin haber llamado NUNCA a `connect()`**. Sin esa llamada **no
> hay socket**: la suscripción no engancha en ningún sitio y **los comandos se van al vacío**.
>
> **La app se pintaba «Enlazado» por haber pulsado una fila.** No por haber conectado.
>
> ## Cómo se veía en el poste, que es lo que costó el banco
>
> El rótulo decía **Enlazado**, el equipo estaba bien, el módulo estaba bien, y **no llegaba ni una
> trama**. Se buscó la avería en el sitio equivocado —radio, cable, módulo, alimentación— porque la
> app afirmaba lo único que era falso. Es la interfaz que **inventa el dato que no tiene**, otra vez:
> el estado de enlace no se dedujo del enlace, se dedujo de un toque en la pantalla.
>
> ## ✅ Arreglado — y el arreglo va DENTRO de la APK
>
> Ahora se llama a **`connect(mac)`**, y **`state.connected` sólo se pone a `true` en su callback de
> éxito**; si la conexión falla, **se dice, y el estado se queda en falso**.
>
> 🛑 **HAY QUE INSTALAR LA APK NUEVA:** **la APK del 04/09 que acompaña a este paquete** — su nombre exacto y su `md5` estan en `LEEME_PRIMERO.md`, en la raíz del `.zip`, que es el único sitio donde no caducan.
> **Es la única APK del 04/09 que existe en
> `05_Funcional/`.** Con la APK anterior la app **NO conecta por bien que funcione el módulo** —
> actualizar el firmware del equipo o cambiar el `ESP32` no arregla nada, porque el defecto está en
> el teléfono.
>
> **Cómo comprobar que la de hoy sí conectó, sin fiarse del rótulo:** una app enlazada de verdad
> recibe **`$STATUS` cada 2 segundos** *(~~cada segundo~~ — la cadencia del STM32 bajo a 2000 ms el 04/09, en las dos puntas)*. Si dice enlazado y la pestaña `Tramas` no se mueve, no está
> enlazada.
>
> ## 🔵 Y esa misma APK trae `N-124`: la lista de equipos ya NO tiene `MAC` escritas a mano
>
> Antes la app llevaba un par de direcciones `MAC` **fijas dentro del código**, así que la lista
> enseñaba equipos que podían no estar delante y **no enseñaba el que sí lo estaba**. Hoy **la lista
> sale del escaneo real del teléfono**: si Android no conoce el módulo, no aparece.
>
> **Eso cambia el orden de lo que hace el técnico, y hay que decirlo entero:**
>
> 1. **EMPAREJAR el `ESP32` en Ajustes de Android** (Bluetooth → `SEM-SIN-MATRICULA`, ver
>    `§4.bis.1`). Empareja **sin PIN**, «Just Works» — `§4.bis.2`.
> 2. **Abrir la app y pulsar «Buscar Módulos Bluetooth».** Sólo entonces sale el equipo en la lista.
>
> 🛑 **Saltarse el paso 1 deja la lista vacía** — y eso **no** es un módulo muerto ni una APK mala.
>
> ## ⚠️ Y el resultado del banco, para que no se lea esto como un cierre
>
> **La cabecera del informe de banco** dice **24 ejercidos · 4 bloqueados por el enlace Bluetooth**
> —o sea, por esto— **· 1 abortado por un incidente de seguridad**, sobre 29 pasos. **Se cita como
> la cifra de la cabecera, no como un hecho: la enumeración del propio informe no cuadra con ella**
> —sus tres cajones nombran 22 identificadores y siete pasos no caen en ninguno—. La discrepancia
> está desglosada en `12_Cobertura_de_Pruebas_y_Huecos.md`, que por eso **no publica ningún total**;
> aquí tampoco se publica uno reconciliado, porque **eso lo decide quien ejecutó la sesión**.
>
> Lo que no depende de la cuenta: como el equipo nunca llegó a operar por falta de app, **la
> regresión del Modo Automático (`N-42`) no se confirmó ni se descartó**. Un paso bloqueado no dice
> nada del firmware.

---

> # 🔴 REVISIÓN DEL 31/08/2026 — TRES CORRECCIONES, Y UNA CAMBIA EL PAPEL DE LA APP
>
> ## 1. 🛑 El `JDY-31` está PROHIBIDO POR NOMBRE, y el módulo real no es ninguno de los dos
>
> La cabecera y la `§2.1` mandaban **`HC-05 / JDY-31`**. El **`JDY-31` es BLE** y está prohibido
> **por su nombre** en `04_Manuales/MANUAL_CONFIGURACION_BLUETOOTH.md §1` —*«Módulos solo BLE
> (`HM-10`, `JDY-31`): obligarían a rehacer el puente nativo y cambiar el flujo del técnico»*—. Que
> apareciera aquí como módulo soportado es una contradicción directa con la decisión congelada.
>
> **El módulo real, identificado:** un **`ESP32-WROOM-32` clásico** —`Xtensa LX6` doble núcleo,
> `Bluetooth v4.2 **BR/EDR** + BLE`—. `BR/EDR` es Bluetooth clásico, o sea **hay perfil SPP y la app
> conecta sin tocar una línea**: el puente nativo de Baliza (`createRfcommSocketToServiceRecord`,
> UUID `00001101-…`) vale tal cual.
>
> **`BLQ-1` está CERRADO.** Si algún documento lo da por abierto, está caducado.
>
> **Y su firmware existe y compila:** `01_Firmware/ESP32_Expansion/`. Ese módulo es una **expansión**
> del STM32 —aporta el Bluetooth y un reloj `DS3231` con pila propia—; **no manda sobre las luces**.
>
> ## 2. 🔴 La app YA NO es un accesorio: es la superficie de mando principal
>
> El diagrama de `§1` y la galería de `§6` describen la app como una *«botonera táctil réplica del
> mando de relés»*. **Se ha invertido la relación:**
>
> | | 28/08 | **31/08** |
> |---|---|---|
> | Superficie de mando | la botonera / el mando de relés | 📱 **la app** |
> | El mando de relés | la referencia que la app imitaba | 🪜 **el último recurso** — y **sin receptor comprado en ninguna punta** |
> | La pantalla LCD | la interfaz local | 🛑 **retirada** |
>
> > 🟠 **AL DÍA EL 04/09 (`N-118`) — y son dos medidas del mismo día que dicen cosas distintas:**
> >
> > * **En banco (3–4/09) el mando ni siquiera era el último recurso.** Con la lectura antigua
> >   —`INPUT_PULLUP` y `== LOW`—, las `R65`/`R66` (10 kΩ a masa) dejan `PB9`/`PB13` en **0,6 V,
> >   BAJO permanente**: **no hay flanco y ninguna de las tres secuencias es alcanzable**.
> > * **En el fuente, ese mismo día, ya está corregido en LAS DOS PUNTAS:** `pinMode(BOTON1/BOTON2,
> >   INPUT)` pelado y `digitalRead(...) == HIGH` —**activo en ALTO**, como las cámaras—
> >   (`Maestro/src/botones.cpp:160-161` y `:223`, `Esclavo/src/botones.cpp:178-179` y `:232`). Se
> >   acciona **cerrando contra los 3,3 V del pin de al lado** (`J16` p5-p4 canal `A`, p8-p7 canal
> >   `B`), **no** con un cable a masa. Y el receptor a comprar deja de ser decisión abierta: es
> >   **NORMALMENTE ABIERTO (`NO`)**.
> >
> > ⚠️ **Pero el arreglo NO se ha cargado ni ejercido en tarjeta.** Mientras no se ejerza, para
> > planificar hay que contar con que **la app es la única vía de mando comprobada** y que **no hay
> > una segunda vía que recoja lo que la app no pueda hacer** — que es lo que da peso a cada defecto
> > de esta app. Ver `1_Manual_Usuario.md` §7 y §9.
>
> **Por qué:** la LCD va a 5 m dentro del gabinete y no se lee desde el suelo; y de los cuatro
> pulsadores **quedan dos** (`A` y `B`), porque `PB14` y `PB15` pasaron a ser entradas de cámara.
> **`botonAceptar()` y `botonCancelar()` devuelven `false`**, así que en el Maestro **no queda ninguna
> forma local de confirmar nada**: entrar y salir de un modo, ajustar tiempos y poner la hora
> **solo se hacen desde la app**.
>
> > ⚠️ **La excepción, y hay que decirla: el ESCLAVO no tiene `SET_MODO` por Bluetooth.** En esa punta
> > el sustituto de los botones **no es la app, es el mando de relés** sobre `A`/`B`: `A·B·A·B` entra
> > al Degradado, `A·A·A` y `B·B·B` salen. Ver `04_Manuales/MANUAL_MANDO_4_RELES.md §5`.
>
> ## 3. La `§5` documentaba **5 comandos**. El Maestro despacha **17 formas**
>
> Ver la sección corregida más abajo.
>
> ## 🟢 4. CAMBIOS DEL 04/09 QUE AFECTAN A LO QUE LA APP OFRECE
>
> **Los tres están verificados en el fuente y ninguno se ha ejercido en tarjeta.**
>
> 1. **El ciclo mínimo sube de 1 a 3 minutos** — verde **3–15 min**, rojo **3–15 min**, despeje
>    10–90 s. **Donde este manual dijera «1-15», estaba obsoleto.** La guarda de verdad está en el
>    firmware; la app valida lo mismo por comodidad. Ver `§5.5.1`.
> 2. **DECISIÓN: el cruce se opera desde el MAESTRO.** La app ya lo enruta y avisa a qué punta va
>    cada orden. Ver `§5.5.2`.
> 3. **`N-130`: el `PEDIDO_AL_MAESTRO` ya no es la última palabra.** Fuera del Modo Inteligente el
>    Maestro rechaza la demanda y llega el evento `MAESTRO / DEMANDA_NO_ATENDIDA_MODO_ACTUAL`.
>    **Es comportamiento normal.** Ver `§5.5.3`.
>
> **Y una cuarta, que no es de comportamiento sino de identificación:** el módulo se auto-rotula
> `SEM-<serie>-M` / `SEM-<serie>-E`, **pero un módulo recién puesto anuncia `SEM-SIN-MATRICULA` y
> los dos postes se llaman igual hasta que se les da una vuelta de energía.** Con la decisión 2
> encima, saber a qué poste se conecta uno deja de ser un detalle. Ver `§4.bis.1`.
>
> ## ⚠️ Y lo que no cambia: **nada de esto ha pasado banco**
>
> En campo corre la **V8.4**. El propio nombre del `.apk` lleva `SIN_BANCO` y hay que leerlo:
> **un manual corregido no es un permiso.**

---

## 1. 🌟 INTRODUCCIÓN Y ARQUITECTURA DE 2 ROLES

La aplicación móvil **IOT-VIAL V9.0** elimina la fricción operativa en obra separando claramente la experiencia en **dos perfiles de usuario**:

![Arquitectura de 2 Roles](./graficas/grafica_01_arquitectura_roles.png)

```text
┌──────────────────────────────────────────────────────────────────────────┐
│                   PERFILES DE ACCESO EN APP IOT-VIAL V9.0                │
├──────────────────────────────────────────────────────────────────────────┤
│ 👷 MODO OPERARIO (POR DEFECTO):                                          │
│   • Botonera tactil de campo - CENSADA EL 04/09 sobre index.html         │
│     (31/08: NO es una "replica del mando" - es LA superficie de mando)   │
│   • Rotulos LITERALES, y no son cuatro:                                  │
│       AUTOMATICO . DAR PASO . AMBAR . ROJO TOTAL .                       │
│       AMBAR EMERGENCIA . RETIRAR AMBAR . VOLVER AL MENU                  │
│     Los TRES de emergencia se ensenan segun la punta (app.js:1494-1506)  │
│       ROJO TOTAL        se oculta con un ESCLAVO delante                 │
│       AMBAR EMERGENCIA  se oculta con un MAESTRO delante                 │
│       RETIRAR AMBAR     SOLO aparece con un ESCLAVO delante              │
│     Sin punta identificada salen los DOS primeros, con su aviso.         │
│     OJO: la parada de emergencia NO es el mismo comando en las dos       │
│          puntas, y por eso NO comparten rotulo:                          │
│          Maestro -> ROJO TOTAL        = CMD:FORZAR_ROJO      (rojo)      │
│          Esclavo -> AMBAR EMERGENCIA  = CMD:AMBAR_EMERGENCIA (ambar,     │
│                     talanquera ABIERTA). FORZAR_ROJO da $ERR alli.       │
│   • Cero contrasenas ni formularios complejos para el operario de obra.  │
├──────────────────────────────────────────────────────────────────────────┤
│ 🛡️ MODO TÉCNICO / ADMINISTRADOR (PIN '1234'):                           │
│   • Tiempos de ciclo: Verde 3-15m, Rojo 3-15m, Despeje 10-90s            │
│     (04/09: el minimo sube de 1 a 3 min. DECISION VIAL, ver 5.5)         │
│   • Asistente Courier RTC con compensación automática de viaje.          │
│   • Test secuencial de potencia de MOSFETs y focos (6 segundos).         │
│   • Gestor CRUD de frentes de obra y cruces viales en LocalStorage.      │
└──────────────────────────────────────────────────────────────────────────┘
```

---

## 2. 🛠️ STACK TECNOLÓGICO Y PIPELINE DE DESARROLLO

![Stack Tecnológico](./graficas/grafica_03_stack_tecnologico_compilacion.png)

### 2.1 Componentes del Stack
* **Frontend Web:** HTML5 Semántico + Vanilla JavaScript ES6 Modular (sin frameworks pesados para garantizar 60 fps y cero lag en WebView).
* **Diseño & UI/UX:** CSS3 Cyber-Industrial con cabezas semafóricas 3D, lentes Fresnel LED con efecto *glow bloom*, tema oscuro de alto contraste OLED y micro-animaciones.
* **Motor Híbrido Android:** **Capacitor 6.x** + plugin **Cordova Bluetooth Serial** (`cordova-plugin-bluetooth-serial`) para comunicación por sockets RFCOMM SPP nativos con ~~módulos HC-05 y JDY-31~~ **el módulo de expansión `ESP32-WROOM-32` clásico** (`BR/EDR`, perfil SPP, UUID `00001101-0000-1000-8000-00805F9B34FB`).

  > 🛑 **Corregido el 31/08: el `JDY-31` es BLE y está PROHIBIDO POR NOMBRE** en
  > `04_Manuales/MANUAL_CONFIGURACION_BLUETOOTH.md §1`. Aparecía aquí como módulo soportado, en
  > contradicción directa con la decisión congelada de *SPP, no BLE*. **No se compra ni se prueba
  > con un `JDY-31`.**
* **Almacenamiento Local:** `LocalStorage API` para persistencia offline de cruces viales y registros de eventos.

### 2.2 Suite de Pruebas Automatizadas TDD y E2E Visual
* ~~**29 Pruebas Unitarias TDD (100% PASS)**~~ **Pruebas unitarias TDD:** validan checksum XOR NMEA, parseo de `$STATUS`/`$ALARM`/`$ERR`, generación segura de comandos con PIN, rangos de tiempos de ciclo, lógica Courier RTC y CRUD de cruces.

  > ⚠️ **La cifra se retira a propósito, no se actualiza.** Las cuentas de la app se mueven cada vez
  > que se añade un caso, y **una cifra escrita a mano en un manual envejece en silencio**. La app
  > tiene hoy **cuatro instrumentos** en la compuerta —simulador de app y Bluetooth, test funcional,
  > test unitarios y la app ejecutada en DOM—, y **sus números vigentes se leen del acta**:
  > `evidencia/<fecha>_compuerta.txt`, la más reciente. Nunca de aquí.
  >
  > 🔴 **Y hay que leer el acta entera, no el número.** Un `x/y` con `x != y` significa que hay
  > comprobaciones que no cumplen, salga con el código que salga; y un `ABORTADO` **no dice nada del
  > firmware** — no es un aprobado. Los dos instrumentos que ejercen la app ya estuvieron
  > simultáneamente en `ABORTADO` una vez, y detrás entraron cuatro defectos, entre ellos **una app
  > que dejó de oír al equipo y pintaba un estado inventado** y **una barrera de PIN que la propia app
  > abría**.
  >
  > 🔴 **Y `N-122` añade el caso peor, que hay que tener delante al leer cualquier cifra de esta
  > página: los instrumentos estaban en VERDE mientras la app no abría el socket.** El censo de
  > funciones sin llamador llevaba `BluetoothDriver` en su lista de excepciones con un motivo
  > escrito —*«`app.js` habla por `window.bluetoothSerial`, sin pasar por aquí»*—, y ese motivo era
  > **medio cierto**: la app usa `write`, `subscribe` y `list`… **y no usa `connect`**, que es justo
  > la que hace funcionar a las otras tres. **Ningún test podía verlo, porque la excepción decía que
  > no había nada que mirar.**
  >
  > **Una cifra verde de esta app no demuestra que la app hable con el equipo.** Eso sólo lo
  > demuestra una trama recibida, en la pestaña `Tramas`, con el equipo delante.
* **E2E Visual (Puppeteer Core):** Automatiza la apertura de modales, selección de dispositivos, cambio de cruces y captura evidencia gráfica en alta resolución en `evidencia/`.

---

## 3. 📦 GUÍA DE COMPILACIÓN Y CONSTRUCCIÓN DEL APK

### 3.1 Requisitos de Entorno
* **Java Development Kit:** JDK 17 o JDK 21 moderno (`JAVA_HOME`).
* **Android SDK:** `build-tools 34.0.0`, `platforms;android-34` (`ANDROID_HOME`).
* **Node.js:** Versión 18 o superior.

### 3.2 Pasos para Compilar
Desde la carpeta raíz de la App (`05_Funcional/App_Semaforo/`):

1. **Ejecutar Pruebas Unitarias TDD:**
   ```bash
   node tests/test_unitarios.js
   ```
2. **Ejecutar Validación Visual E2E en Navegador:**
   ```bash
   node tests/test_e2e_visual.js
   ```
3. **Compilar el APK:**
   ```bash
   android\compilar_apk.bat
   ```
   *El script sincroniza los assets web con Capacitor (`npx cap sync android`) y ejecuta Gradle `assembleDebug` generando el archivo maestro en `05_Funcional/` en ~20 segundos.* **El nombre lleva fecha y hash del árbol** — el vigente es
   **la APK del 04/09 que acompaña a este paquete** — su nombre exacto y su `md5` estan en `LEEME_PRIMERO.md`, en la raíz del `.zip`, que es el único sitio donde no caducan;
   ~~`…_2026-09-02_617bd00_…`~~ y ~~`…_2026-08-28_a8e1ceb_…`~~ **no conectan** (`N-122`, ver
   cabecera).

---

## 4. 🔄 ASISTENTE COURIER RTC (SINCRONIZACIÓN SIN RADIO)

Para tramos donde la geografía bloquea el enlace de radio LoRa/RS-485, el **Asistente Courier RTC** permite sincronizar ambos postes utilizando el celular como puente de tiempo:

![Flujo Courier RTC](./graficas/grafica_02_courier_rtc_flujo.png)

### Procedimiento en Campo:
1. Conectar al **Poste 1 (Maestro)** y presionar **`📸 Capturar Maestro`**. La app memoriza la hora RTC exacta y la fase activa e inicia el contador de viaje.
2. Trasladarse físicamente al **Poste 2 (Esclavo)** (a pie o en vehículo).
3. Conectar al **Poste 2** y presionar **`🚀 Inyectar en Esclavo`**.
4. La App suma automáticamente los segundos de traslado transcurridos ($\Delta t$) y programa el reloj del Esclavo con la hora compensada, logrando sincronismo total sin radio.

> ⚠️ **31/08 — dos precisiones que este apartado daba por sentadas:**
>
> 1. ~~*«el reloj `DS3231` del Esclavo»*~~ → el `SET_RTC` del Esclavo escribe **el RTC del STM32**
>    (`Esclavo/src/bluetooth.cpp:215`). El `DS3231` con pila propia vive **en el módulo de expansión
>    ESP32 del `J17`**, no colgado del STM32: son dos relojes distintos y no se piden igual.
> 2. **El paso 3 puede fallar y hay que mirarlo.** Si el Esclavo contesta
>    `$ERR,CMD:SET_RTC,DESC:SIN_CRISTAL`, **la hora NO quedó inyectada** y el viaje no ha servido de
>    nada. Ver `§5.3`. **No se da el Courier por hecho sin leer la respuesta.**

---

## 4.bis 🔵 EL ENLACE BLUETOOTH — LO QUE NO ESTABA ESCRITO Y COSTÓ EL BANCO DEL 3–4/09

**Los tres puntos de abajo no son teoría: son las tres cosas por las que el técnico se quedó sin
hablar con el equipo teniéndolo delante y encendido.**

> 🔵 **Y antes de los tres, el paso que `N-124` volvió obligatorio:** la lista de equipos de la app
> **sale del escaneo real**, ya no de `MAC` escritas a mano, así que hay que **emparejar el `ESP32`
> en Ajustes de Android PRIMERO** y **después** pulsar **«Buscar Módulos Bluetooth»** en la app. Sin
> ese orden la lista sale vacía. Ver la cabecera.

### 4.bis.1 🛑 Cómo se distingue un poste del otro en la lista — y cuándo NO se distinguen

**El módulo se AUTO-ROTULA, y la letra final dice qué poste es.** No es una opción de compilación:
el mismo binario sirve a las dos puntas y **el nombre lo aprende del campo `NODE:` de la trama
`$STATUS`** que emite la tarjeta. **MEDIDO en
`01_Firmware/ESP32_Expansion/src/transporte_app.cpp:80-114`** y `include/contrato.h:258-259`:

| lo que sale en la lista de Android | qué es |
|---|---|
| **`SEM-<serie>-M`** | el poste **MAESTRO** |
| **`SEM-<serie>-E`** | el poste **ESCLAVO** |
| **`SEM-SIN-MATRICULA`** | el módulo **todavía no ha aprendido** de qué poste cuelga |

> 🔵 **Desde la decisión del 04/09 —el cruce se opera desde el Maestro, `§5.5.2`— esa letra final
> deja de ser un detalle y pasa a ser lo primero que hay que mirar antes de mandar una orden.**

> 🛑 **Y AQUÍ ESTÁ EL CASO QUE HAY QUE SABER EN OBRA: UN MÓDULO RECIÉN PUESTO ANUNCIA
> `SEM-SIN-MATRICULA`, Y LOS DOS POSTES SE LLAMAN IGUAL HASTA QUE SE LES DA UNA VUELTA DE ENERGÍA.**
>
> **El rótulo aprendido se guarda para la SIGUIENTE arrancada, y es deliberado**
> (`transporte_app.cpp:109-113`): renombrar el perfil SPP en caliente obliga a cerrarlo y reabrirlo,
> o sea **a tirar la sesión del operario que en ese momento puede estar dando una orden al cruce**.
>
> **Con dos módulos nuevos en el mismo frente de obra los dos salen iguales en la lista, y el
> técnico se conecta a ciegas.** Lo que hay que hacer, y va en la puesta en marcha:
>
> 1. Con el módulo montado y la tarjeta encendida, **déjelo un minuto**: le basta con recibir un
>    `$STATUS` —salen cada 2 s— para aprender su matrícula y guardarla.
> 2. **Déle una vuelta de energía al módulo.** En el arranque siguiente ya sale con su `-M` o su `-E`.
> 3. **No cambie el módulo** por anunciarse `SEM-SIN-MATRICULA`, y **no lo busque como `IOT_VIAL`**
>    ni por el nombre del cruce: no se llama así.

* Si el técnico busca **`IOT_VIAL`**, o **el nombre del cruce**, **no lo encuentra** — y lo razonable
  es concluir que el módulo está muerto. No lo está.
* **No se renombra en caliente.** Aunque la serie entre durante la sesión, **el nombre bueno aparece
  en el arranque siguiente**, no antes.

> ⚠️ **Consecuencia para la app y para quien la usa:** un `SEM-SIN-MATRICULA` en la lista **no
> significa «equipo sin configurar»**; puede ser un equipo con su serie ya puesta que todavía no se
> ha reiniciado. La matrícula que vale es la del campo `SERIE:` de la trama `$STATUS`, no la del
> nombre Bluetooth. **La app ya lo dice en su propia pantalla de Bluetooth** (`index.html:639-640`).

### 4.bis.2 ⚠️ El emparejamiento es «Just Works»: SIN PIN del sistema operativo

**El `ESP32` empareja sin pedir PIN.** Android enlaza directamente.

> 🔴 **Y aquí está la confusión que se dio en banco, que hay que dejar escrita: el `1234` NO es el
> PIN de emparejamiento.** Es un **PIN de comando dentro de la app** —el que viaja en la trama
> `CMD:PIN:1234:…` (`§5`)— y no lo ve el sistema operativo. Teclearlo en el diálogo de
> emparejamiento de Android no hace nada, porque ese diálogo **no debería aparecer**.
>
> **Si el teléfono pide un PIN de emparejamiento, no está hablando con este módulo.**

### 4.bis.3 🛑 El PIN de la app NO CADUCA NUNCA — `AB-9`, ABIERTO

**Se teclea una vez y el teléfono queda autorizado hasta que se cierra la app.** No hay expiración
por tiempo, ni bloqueo por inactividad, ni cierre al cambiar de nodo.

> 🔴 **Lo que eso significa:** si alguien **guarda el teléfono desbloqueado en el bolsillo**, el
> siguiente que lo coja **manda sobre el cruce sin teclear nada**. Y con la app como superficie de
> mando principal —y, mientras el arreglo del mando de relés no se ejerza en tarjeta (`N-118`), como
> **única vía comprobada**—, eso es todo el mando del equipo.
>
> ⚠️ **Se escribe como riesgo conocido, NO como algo resuelto.** `§5.4.3` cuenta lo que sí se
> arregló —que el PIN ya no se puede armar con el teclado cerrado—, y eso **no es esto**: aquello
> garantiza que **alguien tecleó** cuatro dígitos; esto es **cuánto dura** ese permiso después.
>
> **Cuánto debe durar una sesión autorizada es decisión del responsable**, porque el coste va en los
> dos sentidos: una sesión que caduca demasiado pronto obliga a teclear el PIN delante de un cruce
> parado, y ese fue exactamente el argumento que hizo que no caducara. Mientras siga así, **la única
> barrera real es quién tiene el teléfono**.

---

## 5. 📡 COMANDOS Y PROTOCOLO — 🔴 REESCRITA EL 31/08: ERAN 5 DE 17

Los comandos hacia el firmware viajan como línea ASCII terminada en `\r\n`, con checksum XOR estilo
NMEA. **La forma es `CMD:PIN:1234:<ACCION>` — con dos puntos**, que es como la parsean las dos puntas
(`strncmp(cmd, "CMD:PIN:1234:", 13)`).

> ⚠️ **La lista anterior documentaba CINCO comandos.** El Maestro despacha **17 formas de trama**
> (15 acciones distintas más dos alias sin PIN), y el Esclavo tiene **su propio juego, que no es el
> mismo**. Se conserva tachada y debajo va la censada.

```text
   (SUPERADO 31/08 - cinco comandos, y con la sintaxis de comas)
   $CMD,PIN,1234,SET_MODO,AUTO*2E\r\n
   $CMD,PIN,1234,SET_MODO,AMBAR*1B\r\n
   $CMD,PIN,1234,FORZAR_ROJO*3A\r\n
   $CMD,PIN,1234,SET_TIEMPOS,2,2,15*5C\r\n
   $CMD,PIN,1234,SET_RTC,18:27:00*41\r\n
```

### 5.1 Lo que despacha el **MAESTRO** — censado el 31/08, **RE-MEDIDO EL 04/09**

> ✏️ **Las líneas de esta tabla y de la 5.2 se re-midieron el 04/09.** Las del censo de agosto ya no
> valían: el fichero creció y los despachadores bajaron unas 270 líneas. **Un número de línea en un
> manual caduca solo**, así que se citan con la fecha de la medida.

| Trama | Línea | Notas |
|---|---|---|
| `$LATIDO` | `:399` | 🆕 **04/09.** No es una orden: es la línea que el puente emite para que el troceador cierre un silencio. **El equipo NO contesta nada, a propósito** — sin esta rama caía en la guarda de PIN y devolvía un `AUTH_FAILED` cada dos segundos |
| `CMD:FORZAR_ROJO` | `:412` | **Sin PIN**, a propósito. Rojo total |
| `CMD:SET_MODO:MENU` | `:436` | **Sin PIN** (alias). No abre paso |
| `CMD:SET_MODO:ALCANCE` | `:437` | **Sin PIN** (alias) |
| `CMD:PIN:1234:SET_MODO:AUTO` | `:444` | ⚠️ ver el aviso de §5.5: **reinicia los tiempos a 1/1/15** |
| `CMD:PIN:1234:SET_MODO:MANUAL` | `:449` | |
| `CMD:PIN:1234:SET_MODO:AMBAR` | `:454` | **Es un MODO, no el latch de emergencia.** El Maestro no tiene `AMBAR_EMERGENCIA` |
| `CMD:PIN:1234:SET_MODO:MENU` | `:458` | **sustituye al botón *Cancelar*** retirado. En Degradado sale por todo-rojo |
| `CMD:PIN:1234:SET_MODO:ALCANCE` | `:479` | |
| `CMD:PIN:1234:SET_MODO:INTELIGENTE` | `:490` | modo de cámaras |
| `CMD:PIN:1234:SET_MODO:DEGRADADO` | `:501` | **contesta el motivo concreto del rechazo** |
| `CMD:PIN:1234:FORZAR_ROJO` | `:520` | Igual que la forma sin PIN |
| `CMD:PIN:1234:MANUAL:CAMBIAR_TURNO` | `:524` | `$ERR,…,DESC:EN_TRANSICION_REINTENTE` si no está en reposo |
| `CMD:PIN:1234:TEST_LEDS` | `:538` | |
| `CMD:PIN:1234:SET_TIEMPOS:v,r,d` | `:542` | Un `$ERR` por motivo: `FORMATO_INVALIDO`, `EN_MARCHA_PARE_EL_MODO`, `RANGO`. **Rangos nuevos del 04/09 en §5.5** |
| `CMD:PIN:1234:SET_RTC:YYYY-MM-DD,HH:MM:SS` | `:578` | **Cinco ramas** — ver 5.3 |
| `CMD:PIN:1234:REINICIAR_RELOJ` | `:625` | diagnóstico del cristal |
| `CMD:PIN:1234:DEMANDA` | `:645` | **sólo en Modo Inteligente**: fuera de él, `$ERR,…,DESC:SOLO_EN_MODO_INTELIGENTE` |

Cualquier otra cosa: `$ERR,CMD:DESCONOCIDO,DESC:COMANDO_NO_SOPORTADO` (`:663`).

> **Los marcados 🆕 son los que la app tiene que exponer y este manual no mencionaba.** Un comando
> del firmware sin interfaz en la app es un modo al que ya no se puede llegar: la LCD se retiró y
> *Aceptar* está mudo.

### 5.2 🛑 Lo que despacha el **ESCLAVO** — y NO es lo mismo

**Censado el 31/08, RE-MEDIDO EL 04/09.**

| Trama | Línea | Qué hace |
|---|---|---|
| `$LATIDO` | `:340` | 🆕 **04/09.** Igual que en el Maestro: **no se contesta**, sólo cierra un silencio |
| `CMD:AMBAR_EMERGENCIA` | `:381` | **Sin PIN.** Ámbar intermitente + latch que **veta las órdenes de radio** |
| `CMD:PIN:1234:AMBAR_EMERGENCIA` | `:468` | Lo mismo, con PIN. **El mismo bloque letra por letra** — lo vigilan `esclavo_07` y `esclavo_08` |
| `CMD:PIN:1234:CANCELAR_AMBAR` | `:491` | 🆕 **faltaba en este manual.** **Retira** el latch, **con PIN**. `RETIRADO` · `RETIRADO_QUEDA_MANDO` · `$ERR,…,DESC:NO_HAY_AMBAR_VIGENTE` |
| `CMD:PIN:1234:SOLICITAR_PASO` | `:532` | **Pide**, no ordena. `PEDIDO_AL_MAESTRO` / `$ERR,…,DESC:REPITA_EN_UNOS_SEGUNDOS`. **Y el `PEDIDO_AL_MAESTRO` se puede desmentir después — `N-130`, abajo** |
| `CMD:PIN:1234:SET_RTC:…` | `:563` | `OK` / `SIN_CRISTAL` / `FORMATO_INVALIDO` |
| ~~`CMD:FORZAR_ROJO`~~ | `:448` | 🛑 `$ERR,…,DESC:RENOMBRADO_USE_AMBAR_EMERGENCIA` |
| ~~`CMD:PIN:1234:FORZAR_ROJO`~~ | `:524` | 🛑 mismo `$ERR`. **Las dos formas** |
| ~~`CMD:PIN:1234:TEST_LEDS`~~ | `:550` | 🛑 `$ERR,…,DESC:NO_EN_SERVICIO_USE_EL_MAESTRO` |

Cualquier otra cosa: `$ERR,CMD:DESCONOCIDO,DESC:COMANDO_NO_SOPORTADO_EN_ESCLAVO` (`:622`).

> ✏️ **`CANCELAR_AMBAR` llevaba desde el 31/08 en el firmware (`R-3`) y NO estaba en esta tabla.**
> La app sí tiene su botón —**«RETIRAR ÁMBAR»**, que sólo aparece con un Esclavo delante—, así que
> el manual describía menos comandos de los que el técnico tiene en la pantalla. **Pide PIN al revés
> que el de poner ámbar, y el motivo está en el firmware:** pedir ámbar es la acción **segura**;
> quitarlo devuelve el cruce a dar verdes, o sea **abre paso**, que es justo lo que el PIN custodia.

> ## 🛑 EL BOTÓN DE PÁNICO NO ES EL MISMO EN LAS DOS PUNTAS
>
> Si la app manda `FORZAR_ROJO` al Esclavo, **el equipo contesta `$ERR` y no hace nada**. El
> operario ve un botón rojo grande, lo pulsa **y el cruce sigue igual**. En el Esclavo el comando es
> **`CMD:AMBAR_EMERGENCIA`** (también sin PIN), y lo que hace es **ámbar**, no rojo.
>
> **La app tiene que mandar uno u otro según el nodo conectado, y decir en pantalla cuál va a
> hacer.** El firmware rechaza el nombre viejo **enseñando el bueno** precisamente porque quien lo
> manda es una app o un manual anteriores al cambio.

### 5.3 `SET_RTC` tiene cinco ramas, y la app debe distinguirlas

| Respuesta | Qué mostrar al técnico |
|---|---|
| `$ACK,CMD:SET_RTC,RESULT:OK` | Hora puesta y propagada al Esclavo |
| `$ACK,CMD:SET_RTC,RESULT:HORA_PUESTA_SIN_PROPAGAR` | **Puesta aquí, NO viajó al otro poste** |
| `$ERR,CMD:SET_RTC,DESC:SIN_CRISTAL_VEA_CONSULTA_RELOJ` | 🛑 **La hora NO quedó puesta** |
| `$ERR,CMD:SET_RTC,DESC:FORMATO_INVALIDO` | Trama ilegible o cifras fuera de rango |

> **Tratar las cuatro como «enviado» es el defecto que este comando ya tuvo:** contestaba `RESULT:OK`
> sin mirar ninguna de las dos llamadas, y con el cristal `Y2` muerto el técnico se iba del poste
> creyendo que había dejado el reloj puesto. **Un `$ACK` que no depende de lo que devolvió la llamada
> es una mentira con formato de éxito** — y una app que no lee la diferencia la reintroduce en la
> pantalla.

---

## 5.4 🆕 LO QUE LA APP HACE DESDE EL 01/09 Y ANTES NO — tres cosas que se notan en campo

### 5.4.1 La app **comprueba el checksum y descarta lo corrompido**

Antes cortaba la trama por el `*` y **tiraba el checksum sin mirarlo**: una trama corrompida en el
aire se pintaba como si fuera buena. Hoy lo calcula, lo compara y, si no cuadra, **no pinta nada**
(`js/nmea_parser.js:23-29` lo calcula, `:71-81` lo compara; `app.js:1820-1826` la descarta).

También descarta lo que **no tiene forma** de trama —sin `$` o sin `*`— y las cabeceras que no
conoce. Las que sí lee son cinco: `$STATUS`, `$ALARM`, `$ACK`, `$EVENT` y `$ERR`.

> 🔴 **Consecuencia práctica, y es la que hay que saber en el poste: una pantalla que se queda
> quieta ya no significa «el equipo no contesta».** Puede significar **«contesta y llega roto»** —
> radio con ruido, cable flojo, un `J17` mal enchufado—. Los dos casos se ven distintos en la
> pestaña `Tramas`, y **son averías diferentes**. No cambie el equipo antes de mirarla.

### 5.4.2 La pestaña **`Tramas`** — las tramas en crudo, tal como llegan

Es la quinta pestaña, con icono 🧪, y **solo se ve en modo TÉCNICO**: se llega tocando el botón de
rol, tecleando el PIN, y luego `Tramas`.

Muestra cada línea recibida **tal cual**, marcando las rechazadas con su motivo, más los contadores,
un filtro *Sólo rechazadas* y la posibilidad de **guardar el registro en un fichero** o copiarlo.

> 💡 **Es el instrumento que hay que abrir cuando algo no cuadra**, y también donde se leen los
> `$EVENT` en bruto — el `ORIGEN:RELOJ` con los bits del reloj, entre ellos.

### 5.4.3 El **PIN ya no se puede armar con el teclado cerrado**

Los botones del teclado siguen existiendo en la página aunque el modal esté oculto, así que
cualquier cosa que disparara una pulsación sobre ellos **autorizaba la sesión sin que la barrera se
hubiera abierto nunca**. Hoy los cuatro caminos que tocan el PIN comprueban primero que el teclado
esté delante del operario (`app.js:2554-2556`, aplicado en `:2561`, `:2574`, `:2582` y `:2592`), y
**cerrar el teclado cancela de verdad**: borra los dígitos y la acción pendiente.

> ⚠️ **Esto no lo nota el operario, y por eso se escribe.** Lo que cambia es que un modo técnico
> abierto **significa que alguien tecleó cuatro dígitos**, que es lo que el PIN existía para
> garantizar.
>
> 🛑 **Y lo que este arreglo NO cubre, para que no se lea como si la barrera estuviera cerrada:
> ese permiso NO CADUCA (`AB-9`, abierto).** Garantizar que alguien tecleó el PIN **no es lo mismo
> que** garantizar que quien manda ahora es quien lo tecleó. Ver `§4.bis.3`.

### 5.4.4 El puente **dice por qué arrancó** al reconectar

Cuando el teléfono se empareja con el ESP32, el puente manda **una** trama contando su último
reinicio. Vuelve a mandarla en cada reconexión:

```text
   $EVENT,NODE:PUENTE,EVT:ARRANQUE,CAUSA:PERRO_DE_TAREAS,ARRANQUES:3,PERRO:ARMADO,WDT_MS:2000*XX
```

| campo | qué dice |
|---|---|
| `CAUSA:` | por qué se reinició. `SUBIDA_DE_TENSION` es un encendido normal; **`PERRO_DE_TAREAS`, `EXCEPCION_O_PANICO` o `TENSION_BAJA` no lo son** |
| `ARRANQUES:` | cuántas veces ha arrancado **desde la última subida de tensión**, no desde siempre |
| `PERRO:` | `ARMADO` si el watchdog está vigilando; **`SIN_ARMAR` es un problema** |

*(Medido en `01_Firmware/ESP32_Expansion/src/vigilante.cpp:90-91` el formato, `:103-118` las once
causas posibles, `:170-174` el rearme por reconexión.)*

> 🔴 **`ARRANQUES:` subiendo entre visitas es el síntoma que ningún otro instrumento da.** Un puente
> que se reinicia solo cada pocos minutos **parece un enlace intermitente**, y se acaba cambiando la
> radio o el cable. Este campo dice que el problema es el módulo.
>
> ⚠️ **La app no le da pantalla propia: sale como un evento más.** Se lee en `Eventos` o en `Tramas`.

---

## 5.5 🟢 LO QUE CAMBIÓ EL 04/09 — TRES COSAS QUE LA APP TIENE QUE REFLEJAR

### 5.5.1 🔴 El ciclo mínimo sube de 1 a 3 minutos, y la guarda de verdad NO es la app

**Rangos vigentes, MEDIDOS en `Maestro/src/modo_automatico.cpp:51-53`:**

| | mínimo | máximo |
|---|---|---|
| Verde | **3 min** *(era 1)* | 15 min |
| Rojo | **3 min** *(era 1)* | 15 min |
| Despeje (todo-rojo) | 10 s | 90 s |

**El motivo es una decisión vial del responsable, literal: «tres minutos es la mínima distancia de
seguridad».** En un paso alternado de un solo carril un camión pesado tarda entre 5 y 8 s sólo en
reaccionar y arrancar; con un verde de 60 s pasan tres o cuatro vehículos antes de cortar a ámbar, y
lo que sale de ahí no es una cola: es un conductor convencido de que el semáforo está averiado.

> 🔵 **La app valida lo mismo —`app.js:2500`, `enRango(verde, 3, 15)`— pero eso es COMODIDAD, no la
> barrera.** La barrera vive en el firmware y rechaza con `$ERR,CMD:SET_TIEMPOS,DESC:RANGO`. **Y es
> la única que vale:** la app no es la única que habla por `J17`, y **una APK vieja de las que
> sobreviven en los teléfonos puede mandar `SET_TIEMPOS` con un minuto**. Una guarda que sólo vive
> en la interfaz es de cortesía.
>
> **Para quien mantiene la app, la regla que sale de aquí:** el rango de la app se valida contra el
> `.cpp`, y **si algún día divergen, el que manda es el firmware**. Ofrecer un valor que el equipo
> va a rechazar es el defecto que este proyecto ya conoce con otro nombre.

> 🛑 **Y un aviso ABIERTO, medido el 04/09 y SIN DIAGNOSTICAR, que afecta a lo que la app enseña:**
> los valores por defecto del modo siguen siendo **1 min / 1 min / 15 s** (`modo_automatico.cpp:13`
> y `:94`) y **no pasan por la guarda** —`modoAutomatico_fijarTiempos()` sólo se ejerce desde
> `SET_TIEMPOS`—. **`SET_MODO:AUTO` llama a `modoAutomatico_setup()`, que reescribe los tres valores
> a `1, 1, 15`**: unos tiempos aceptados con `$ACK` **se pierden al arrancar el modo**.
> *No se propone aquí un arreglo, y la app no debe fingir uno: mientras esto siga así, lo que la
> pantalla muestre después de un `SET_MODO:AUTO` puede no ser lo que el equipo está usando.*

### 5.5.2 🟢 DECISIÓN: el cruce se opera desde el MAESTRO

**No se va a hacer transparente el mando desde el Esclavo.** La asimetría de `§5.1` y `§5.2` deja de
ser una limitación pendiente y pasa a ser **cómo se opera este equipo**.

| desde el **MAESTRO** | desde el **ESCLAVO** |
|---|---|
| cambiar de modo · dar paso · ajustar tiempos · `TEST_LEDS` · rojo total | **sólo** `SOLICITAR_PASO`, `AMBAR_EMERGENCIA`, `CANCELAR_AMBAR` y `SET_RTC` |

**La app ya lo enruta y no hace falta tocarla:** `SOLO_MAESTRO` y `SOLO_ESCLAVO` en `app.js:2051-2057`,
y `puntaCorrecta()` en `:2078` avisa a qué punta va la orden en vez de mandarla al vacío.

> ⚠️ **Lo que la app NO puede hacer por el operario: llevarle al otro poste.** El Bluetooth alcanza
> el equipo que tiene delante. Si hace falta cambiar el modo estando en el Poste 2, hay que
> desplazarse. **Por eso `§4.bis.1` deja de ser un detalle de lista: hay que saber a qué poste se
> está conectando.**

### 5.5.3 🟢 `N-130` — el equipo ya no dice que sí a lo que no va a hacer

**Es comportamiento normal, no una avería.** Si se pulsa **«Solicitar Paso» desde el ESCLAVO** y el
cruce **no está en Modo Inteligente**, el Maestro rechaza la demanda y el Esclavo lo publica como
evento:

```text
   MAESTRO / DEMANDA_NO_ATENDIDA_MODO_ACTUAL
```

**Por qué llega como EVENTO y no como `$ERR`:** el Esclavo contesta a la app **antes** de saber la
respuesta del Maestro —no puede esperar: bloquear su bucle por una radio de 2,4 kbps es peor—, así
que **el primer acuse sigue siendo `$ACK,CMD:SOLICITAR_PASO,RESULT:PEDIDO_AL_MAESTRO`**. Un `$ERR`
tardío habría que casarlo con una orden contestada hace cientos de milisegundos, y con dos
pulsaciones seguidas la app no sabría a cuál corresponde. **Un evento fechado en la bitácora sí se
lee.**

**MEDIDO:** `Maestro/src/coordinador.cpp:636-641` (decide con `modoActual_get() == MODO_INTELIGENTE`
y manda `DEMANDA_ACEPTADA` / `DEMANDA_RECHAZADA`) y `Esclavo/src/main.cpp:541-543` (lo convierte en
el evento). **Antes esa rama estaba vacía: la app decía «pedido al Maestro» y no pasaba nada.**

> ⚠️ **Para quien lea la app en campo: el `PEDIDO_AL_MAESTRO` ya no es la última palabra.** Hay que
> mirar `Eventos` unos segundos después. Si sale `DEMANDA_NO_ATENDIDA_MODO_ACTUAL`, **no es un fallo
> del enlace ni del Esclavo**: es que el cruce no está en Modo Inteligente, y ese modo se pone
> **desde el Maestro**.

---

## 6. 📊 EVIDENCIA DE PANTALLAS (GALERÍA V9.0)

> ⚠️ **Una captura a un solo ancho no es una prueba de interfaz.** Las de `evidencia/` se hicieron a
> **412 px**, y el corte del botón de la derecha **solo aparece por debajo**: 11 px a 390, **41 px a
> 360** y **81 px a 320**. Un `.png` demuestra que a **ese** ancho se veía bien, y nada más. Al medir
> una pantalla nueva se miden **los cuatro anchos**.

| Vista | Captura | Descripción |
|---|---|---|
| **Modo Operario** | `evidencia/01_modo_operario_principal.png` | Botonera táctil y réplica 3D de semáforos. ~~*«réplica del mando de relés»*~~ → **31/08: es la superficie de mando principal**, no una réplica de nada. ⚠️ **04/09: la captura enseña ~~4~~ botones y la botonera de hoy tiene SIETE rótulos, tres de ellos condicionados a la punta (§1). La captura está caducada, no la botonera** |
| **Selector de Cruces** | `evidencia/02_modal_cruces_abierto.png` | Catálogo de frentes de obra y selección inmediata |
| **Modo Técnico** | `evidencia/06_modo_tecnico_activo.png` | Desbloqueo por PIN y pestañas de configuración |
| **Ajustes de Tiempos** | `evidencia/07_tiempos_guardados_exito.png` | Programación de tiempos de verde, rojo y despeje |
| **Courier RTC** | `evidencia/08_courier_rtc_inyectado.png` | Inyección de hora compensada en el nodo Esclavo |

---
**Desarrollado y Validado para el Proyecto Controladora_Semaforos V9.0**
