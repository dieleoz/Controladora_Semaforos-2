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
> >   —el `pinMode` está en `botones_setup()` y la lectura en su barrido de pulsados, en las dos
> >   puntas: `grep -n "pinMode(BOTON1\|pinMode(BOTON2\|digitalRead(todos" 01_Firmware/Maestro/src/botones.cpp 01_Firmware/Esclavo/src/botones.cpp`—. Se
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

> # 🟢 04/09/2026, MÁS TARDE EL MISMO DÍA — LA APP CAMBIÓ DE BARRERA
>
> **Es el cambio que más se nota en la mano del operario, y va aquí arriba porque quien baje
> directamente a un procedimiento de más abajo lo va a encontrar describiendo un teclado que ya no
> aparece.** Todo lo de esta caja está **MEDIDO en el fuente de la app** y **nada se ha ejercido con
> un equipo delante**.
>
> | # | qué cambia | dónde está contado |
> |---|---|---|
> | **1** | 🔴 **El operario ya NO teclea el PIN para dar paso ni para arrancar el ciclo.** En su lugar la app le pregunta **si ha mirado el tramo** | **§4.bis.4**, nueva |
> | **2** | ~~✅ *«NUNCA se pregunta para PARAR —rojo total, ámbar—»*~~ → 🔴 **CADUCADO EL 05/09 (`N-148`): el ÁMBAR SÍ PREGUNTA.** No para nada —deja entrar por las dos puntas a la vez—. Lo que no pregunta es `ROJO TOTAL`, `AMBAR EMERGENCIA` y `VOLVER AL MENÚ` | **§4.bis.4** |
> | **3** | 🟢 **El PIN CADUCA**: 60 s de gracia al guardarse el teléfono, y 5 min sin mandar órdenes. Cierra `AB-9` | **§4.bis.3**, reescrita |
> | **4** | 🆕 **Hay un DIARIO DE ÓRDENES**, aparte de la cinta de tramas. **Es lo que hay que exportar y mandar cuando algo falle** | **§5.4.2.bis**, nueva |
> | **5** | 🆕 **Los rechazos del equipo se traducen** y dicen qué hacer en el poste — **22 + 1 = 23** motivos | **§5.4.2.ter**, nueva |
> | **6** | 🟢 **Los tiempos del ciclo ya no se pierden** al entrar en Automático ni al cortar la luz (`N-133`), **y en Automático no se pueden cambiar** (`N-42`/`N-135`) | **§5.5.1**, corregida |
>
> ## 🔴 El porqué del nº 1, que es lo único que hay que recordar de esta caja
>
> > **El equipo no sabe si quedan vehículos en el tramo, y el operario sí. Un PIN demuestra QUIÉN
> > ERES; no demuestra que hayas MIRADO.**
>
> Cuatro dígitos delante de un cruce parado no protegen a nadie del riesgo real de esa orden —que
> alguien siga dentro del tramo—, y **enseñan a teclear deprisa**. La pregunta que sí protege es la
> que no se puede contestar sin levantar la vista.
>
> ## ⚠️ Y lo que esta caja NO dice
>
> * **El PIN no ha desaparecido de la trama.** Sigue viajando y el firmware lo sigue exigiendo donde
>   siempre; lo que cambió es **a quién se le pregunta en la pantalla**.
> * **`SET_MODO:AMBAR`, `CANCELAR_AMBAR`, `SET_MODO:DEGRADADO` y `TEST_LEDS` SIGUEN pidiendo PIN.**
>   La tabla completa de qué pide qué está en **§4.bis.4**.
> * 🛑 **El error `FORMATO_INVALIDO` del Courier RTC SIGUE SIN DIAGNOSTICAR.** Que la app ahora lo
>   traduzca a lenguaje de obra **no lo explica ni lo repara**: sólo hace legible el síntoma. **No
>   hay causa escrita en ninguna parte, y aquí no se propone ninguna.**

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
│     Los TRES de emergencia salen segun la punta: actualizarEmergencia()  │
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
│     05/09: tampoco PIN para DAR PASO ni para arrancar el ciclo; y el     │
│     AMBAR pide LAS DOS COSAS -PIN y confirmacion de via (N-148)          │
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

## 1.bis 🛑 LO QUE LA APP NO VE NUNCA: IMÁGENES

> **De cada cámara el sistema consume UN CONTACTO SECO. No hay red, no hay imagen, no hay vídeo, y
> no hay analítica en el controlador** — decisión **`D-12`**, en `DECISIONES.md`.

**MEDIDO el 05/09, y son TRES medidas independientes:**

* **En el puente `ESP32`, cero red.** No hay `WiFi`, ni `HTTPClient`, ni servidor web, ni `ONVIF`,
  ni `RTSP`, ni decodificador de imagen en todo el módulo:

  ```bash
  cd 01_Firmware/ESP32_Expansion
  grep -rn -i "WiFi\|HTTPClient\|WebServer\|ONVIF\|RTSP\|esp_camera" src/ include/
  ```

  **Devuelve CERO líneas** (los 16 ficheros del módulo). El `ESP32` sólo hace de puente
  Bluetooth SPP y de reloj `DS3231`.

  > ⚠️ **Y el `grep` va acotado a `src/` e `include/` A PROPÓSITO, porque el que no lo está
  > MIENTE.** Lanzado sobre `01_Firmware/ESP32_Expansion/` entero devuelve **cientos de
  > líneas**: entra en `.pio/build/`, y el `firmware.map` nombra `libesp_wifi.a` porque el
  > enlazador la considera —el Bluetooth del `ESP32` comparte radio con el WiFi—. **Considerar
  > no es enlazar**, y eso se mide en el binario, no en el mapa.

* **Y la medida que cierra la anterior: en el `.elf` no hay UN SOLO símbolo de red definido.**
  Sobre los **10.128 símbolos** del binario del puente:

  ```bash
  C:/.platformio/packages/toolchain-xtensa-esp32/bin/xtensa-esp32-elf-nm.exe \
      01_Firmware/ESP32_Expansion/.pio/build/esp32_expansion/firmware.elf
  ```

  `esp_wifi_init`, `esp_wifi_start`, `esp_wifi_connect`, `esp_netif_init`, `httpd_start`,
  `lwip_socket` y `socket` dan **CERO** en las clases `T`/`t`/`W`/`w`. **Nada de red entra en
  la tarjeta.**

* **En el STM32, la cámara es un `digitalRead`.** Lo único que entra del sensor es el NIVEL de un
  pin, leído por `camara_leerPin()` —una función de una línea— sobre `CAM_DEMANDA_PIN`:

  ```bash
  grep -n "bool camara_leerPin" 01_Firmware/Maestro/src/botones.cpp
  grep -n "CAM_DEMANDA_PIN"     01_Firmware/Maestro/include/pines.h
  ```

**Consecuencia, y va entera porque lo que importa es lo que se pierde:**

| lo que la app SÍ puede enseñar | lo que la app NO puede enseñar — hoy ni con otra APK |
|---|---|
| que hubo **demanda** en una punta, como evento fechado | **quién** la produjo, **qué** había en la calzada, **una foto** |
| el `$EVENT` del cruce y la terna del Diario (`§5.4.2.bis`) | vídeo en directo, grabación, matrícula, conteo por imagen |

> 🛑 **Sin imágenes en el controlador NO hay soporte de accidentes ni auditoría por nuestra
> parte.** Un evento dice *«el pin de la cámara estaba activo»* y no dice nada más. Lo que `D-12`
> deja escrito como salida es que **la grabación vive en la microSD de la PROPIA CÁMARA**
> —Hikvision `DS-2CD2683G2-IZS`, `D-10`—: una tarjeta que hay que **comprar y configurar**, y que
> **no toca una línea de este firmware ni de esta app**. Sigue abierta como `A-0` en `DECISIONES.md`.
>
> **Ninguna pantalla, ningún botón y ningún fichero exportado de esta app trae imagen.** Si alguien
> hereda lo contrario de leer *«modo de cámaras»* en `§5.1` o el emoji 📸 de `§4`, **manda este
> apartado**.

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

  > ⚠️ **«Captura» aquí es una CAPTURA DE PANTALLA DEL TELÉFONO, no una imagen de cámara de calle.**
  > En `evidencia/` no hay ni una sola imagen tomada por una cámara del cruce, **y no puede
  > haberla**: ver `§1.bis`. La galería de `§6` es de pantallas de la app y de nada más.

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
1. Conectar al **Poste 1 (Maestro)** y presionar **`📸 Capturar Maestro`**. La app ~~memoriza la hora RTC exacta~~ **anota la hora DEL TELÉFONO** y la fase activa, e inicia el contador de viaje. **No captura ninguna imagen — ver el aviso de debajo.**
2. Trasladarse físicamente al **Poste 2 (Esclavo)** (a pie o en vehículo).
3. Conectar al **Poste 2** y presionar **`🚀 Inyectar en Esclavo`**.
4. La App suma automáticamente los segundos de traslado transcurridos ($\Delta t$) y programa el reloj del Esclavo con la hora compensada, logrando sincronismo total sin radio.

> 🛑 **EL EMOJI 📸 NO CAPTURA NINGUNA IMAGEN, Y VA ESCRITO AQUÍ PARA QUE NADIE LO HEREDE AL
> REVÉS.** «Capturar» aquí es *apuntar una hora en la libreta*, no *hacer una foto*. La app no ve
> imágenes, no las pide y no las puede tener: ver **`§1.bis`**.
>
> **MEDIDO el 05/09 en el fuente de la app:**
>
> ```bash
> grep -n "id=\"btn-courier-capture\"" 05_Funcional/App_Semaforo/www/index.html
> grep -n "capturarMaestro"             05_Funcional/App_Semaforo/www/js/courier_rtc.js
> grep -n "function horaLocal24"        05_Funcional/App_Semaforo/www/app.js
> ```
>
> El manejador de ese botón llama a `CourierRTC.capturarMaestro(hora, fase, cuenta)`, y lo único que
> queda guardado son **cuatro campos**:
>
> | qué guarda | de dónde sale |
> |---|---|
> | `horaStr` — la hora `HH:MM:SS` | **del RELOJ DEL TELÉFONO**: `horaLocal24()`, que es `new Date()` |
> | `faseActual` — la fase de las luces | `state.estadoLuces`, del último `$STATUS` recibido |
> | `tiempoRestanteSeg` — la cuenta atrás | `state.countdown`, del mismo `$STATUS` |
> | `timestampCaptura` — el instante | `Date.now()`, para cronometrar el viaje |
>
> 🔴 **Y de ahí sale una corrección que este apartado necesitaba: la hora capturada NO es «la hora
> RTC del Maestro», es la del teléfono.** El Courier usa el celular **como fuente de tiempo** —que es
> lo que dice la cabecera de este apartado, *«el celular como puente de tiempo»*—, **no** como
> copiadora del reloj del Poste 1: durante todo el procedimiento **al Maestro no se le manda nada**.
> **Si el teléfono va desfasado, el Esclavo queda desfasado igual.**
>
> *(El campo `HORA:` del `$STATUS` —la hora que sí es del equipo— la app lo guarda en `state.hora` y
> lo pinta desde `N-150`, pero el Courier **no lo lee**. `SIN VERIFICAR` si eso es deliberado: no hay
> comentario en el fuente que lo razone.)*

> ⚠️ **31/08 — dos precisiones que este apartado daba por sentadas:**
>
> 1. ~~*«el reloj `DS3231` del Esclavo»*~~ → el `SET_RTC` del Esclavo escribe **el RTC del STM32**
>    (rama `SET_RTC:` de su despachador — `grep -n '"SET_RTC:"' 01_Firmware/Esclavo/src/bluetooth.cpp`).
>    El `DS3231` con pila propia vive **en el módulo de expansión
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
`$STATUS`** que emite la tarjeta. **MEDIDO en `transporte_aprenderRotulo()` y en los dos
`#define` del contrato:**

```bash
grep -n "transporte_aprenderRotulo" 01_Firmware/ESP32_Expansion/src/transporte_app.cpp
grep -n "ROTULO_PREFIJO\|ROTULO_PROVISIONAL" 01_Firmware/ESP32_Expansion/include/contrato.h
```

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
> **El rótulo aprendido se guarda para la SIGUIENTE arrancada, y es deliberado** —el porqué está
> escrito al final de `transporte_aprenderRotulo()`, junto al `memoria.putString("rotulo", ...)`—:
> renombrar el perfil SPP en caliente obliga a cerrarlo y reabrirlo,
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
> nombre Bluetooth. **La app ya lo dice en su propia pantalla de Bluetooth** —
`grep -n "SEM-SIN-MATRICULA" 05_Funcional/App_Semaforo/www/index.html`, el párrafo
`bt-aviso-modulo` del modal, marcado `N-124` en el fuente—.

### 4.bis.2 ⚠️ El emparejamiento es «Just Works»: SIN PIN del sistema operativo

**El `ESP32` empareja sin pedir PIN.** Android enlaza directamente.

> 🔴 **Y aquí está la confusión que se dio en banco, que hay que dejar escrita: el `1234` NO es el
> PIN de emparejamiento.** Es un **PIN de comando dentro de la app** —el que viaja en la trama
> `CMD:PIN:1234:…` (`§5`)— y no lo ve el sistema operativo. Teclearlo en el diálogo de
> emparejamiento de Android no hace nada, porque ese diálogo **no debería aparecer**.
>
> **Si el teléfono pide un PIN de emparejamiento, no está hablando con este módulo.**

### 4.bis.3 ✅ ~~🛑 El PIN de la app NO CADUCA NUNCA — `AB-9`, ABIERTO~~ → **AHORA CADUCA (04/09)**

> 🛑 **LO QUE ESTE APARTADO DECÍA HASTA EL 04/09, conservado tachado porque una barrera que aparece
> sin dejar rastro de que faltaba se vuelve a quitar:**
>
> ~~**Se teclea una vez y el teléfono queda autorizado hasta que se cierra la app.** No hay
> expiración por tiempo, ni bloqueo por inactividad, ni cierre al cambiar de nodo. Si alguien guarda
> el teléfono desbloqueado en el bolsillo, el siguiente que lo coja manda sobre el cruce sin teclear
> nada.~~

**Hoy el permiso caduca por DOS caminos, y el técnico lo va a notar. MEDIDO en las dos constantes
de `App_Semaforo/www/app.js`:**

```bash
grep -n "const PIN_GRACIA_FONDO_MS\|const PIN_INACTIVIDAD_MS" 05_Funcional/App_Semaforo/www/app.js
```

```js
   const PIN_GRACIA_FONDO_MS = 60 * 1000;        // 60 s
   const PIN_INACTIVIDAD_MS  = 5 * 60 * 1000;    // 5 min
```

| se cierra la sesión cuando… | el plazo | dónde |
|---|---|---|
| **la app se va al fondo** —se guarda el teléfono, se cambia de aplicación, se contesta una llamada— **y tarda en volver más de 60 s** | **60 s de gracia** | `PIN_GRACIA_FONDO_MS`, leída por `volverDelFondo()` |
| **pasan 5 minutos sin mandar ni una orden**, con la app delante | **5 min** | `PIN_INACTIVIDAD_MS`, leída por `vigilarAutorizacion()` |

**Los 60 s de gracia son la mitad que hace esto usable, y por eso van escritos:** mirar un mensaje o
consultar una foto del plano **no** cierra la sesión; guardarse el teléfono, sí. Y *«sin mandar
ninguna orden»* significa exactamente eso —**haber enviado**, no haber tocado la pantalla: el sello
`state.ultimaOrdenMs` se sella en UN SOLO sitio —dentro de `enviarComandoFirmware()`— y el único
que vuelve a tocarlo es `caducarPin()`, que lo borra
(`grep -n "state.ultimaOrdenMs =" 05_Funcional/App_Semaforo/www/app.js` devuelve esas dos y la
lectura de `vigilarAutorizacion()`)—: leer telemetría durante
cinco minutos **no** mantiene abierta la autorización.

**Al caducar, la app baja el rol de `TÉCNICO` a `OPERARIO`** y descarta también la confirmación de
vía que hubiera pendiente — todo dentro de `caducarPin()`
(`grep -n "function caducarPin" 05_Funcional/App_Semaforo/www/app.js`). No es un aviso: es que hay
que volver a teclear.

> ⚠️ **Y lo que hay que decirle al técnico para que no lo lea como una avería:** si vuelve al
> teléfono y la pestaña `Técnico` ya no está, **no se ha desconectado el Bluetooth ni se ha caído el
> equipo**. El enlace sigue; lo que caducó es el permiso. Se vuelve a entrar con los mismos cuatro
> dígitos.

> 🔴 **LO QUE NO ESTÁ VERIFICADO, Y ES PRECISAMENTE LO QUE ESTA BARRERA PROMETE — clasificado `SIN
> VERIFICAR`, no `MEDIDO`.** Los dos caminos cuelgan de **sucesos del navegador**:
> `visibilitychange`, `pagehide` y `pageshow`, y son los tres únicos:
>
> ```bash
> grep -n "addEventListener('visibilitychange'\|'pagehide'\|'pageshow'" \
>      05_Funcional/App_Semaforo/www/app.js
> ```
>
> **No hay un solo `pause` ni `resume` de Cordova**, que es el par que un WebView empaquetado
> emite con seguridad — `grep` de `'pause'`, `'resume'` y `blur` sobre `app.js` da **cero**.
>
> **En un navegador de escritorio eso funciona y se ha ejercido. En la APK, sobre un teléfono real,
> con la pantalla apagada y el móvil en el bolsillo, NADIE LO HA VISTO CADUCAR.** El escenario que
> esta barrera existe para cubrir es **exactamente** ese, y es el único que no se ha probado.
> **Hasta que alguien lo cronometre con el teléfono en la mano, la única barrera demostrada sigue
> siendo quién tiene el teléfono.**
>
> *(Es la forma de `CLAUDE.md` §2.ter: la caducidad está **declarada** y no está **ejercida** en el
> medio donde tiene que valer.)*

### 4.bis.4 🟢 NUEVO EL 04/09 — PARA ABRIR PASO, LA APP YA NO PIDE EL PIN: PREGUNTA SI HA MIRADO EL TRAMO

> **Es el cambio que más se nota en la mano del operario, y el motivo hay que leerlo entero porque
> no es una comodidad: el equipo no sabe si quedan vehículos en el tramo, y el operario sí. Un PIN
> demuestra QUIÉN ERES; no demuestra que hayas mirado.**

~~**Las dos órdenes que ABREN paso ya no piden cuatro dígitos.**~~ → 🔴 **AL DÍA EL 05/09: SON
TRES, Y LA TERCERA ES EL ÁMBAR** (`N-148`). Ver la caja de debajo de la tabla, que es lo que hay
que leer entero.

**MEDIDO el 05/09** — la tabla de maniobras del fuente tiene **tres** entradas y los tres botones
llaman a `confirmarVia()`:

```bash
grep -n "const VIA_MANIOBRA" 05_Funcional/App_Semaforo/www/app.js
grep -n "confirmarVia("      05_Funcional/App_Semaforo/www/app.js
```

*(Comprobado el 05/09: el primero devuelve **una** línea con sus tres claves dentro; el segundo,
**cinco** — la definición, una mención en un comentario y **las TRES llamadas**, una por botón.)*

| botón | orden | ¿pide PIN? | qué pregunta hoy |
|---|---|---|---|
| **DAR PASO** | `MANUAL:CAMBIAR_TURNO` | ❌ no | *«DAR PASO: el sentido que ahora tiene verde va a quedar en rojo y el otro va a arrancar. Mire el tramo entero antes de aceptar.»* |
| **AUTOMÁTICO** | `SET_MODO:AUTO` | ❌ no | *«AUTOMATICO: el equipo va a empezar a dar verdes solo, sin volver a preguntar. Mire el tramo entero antes de aceptar.»* |
| 🆕 **ÁMBAR** | `SET_MODO:AMBAR` | ✅ **sí, y ADEMÁS pregunta** | *«AMBAR: los DOS postes quedan en intermitente a la vez, así que se podrá entrar al corredor por las dos puntas. No es un rojo y no para a nadie. Mire el tramo entero antes de aceptar.»* |

> 🔴 **`N-148` (05/09) — EL ÁMBAR CAMBIÓ DE LADO, Y LO QUE ESTE MANUAL DECÍA ERA FALSO.**
>
> Hasta el 04/09, aquí y en la caja de cabecera estaba escrito que **poner ámbar no pregunta
> nada** porque es *«la dirección segura»*. ~~*«NUNCA se pregunta para PARAR —rojo total,
> ámbar—»*~~ → **caducado**: el ámbar **no para nada**.
>
> **Lo pidió el responsable y se comprobó en el C++, que es lo que lo convierte en un defecto y no
> en una preferencia:** `SET_MODO:AMBAR` llama a `modo_ambar_setup()`
> (`grep -n "modo_ambar_setup" 01_Firmware/Maestro/src/modo_ambar.cpp`), que pone intermitente en
> **ESTA** punta **y manda `CMD_GO_AMBAR` a la otra**. En un carril único con un poste en cada
> extremo, eso **deja entrar por LOS DOS LADOS a la vez**: de todas las órdenes de la botonera es
> **la que más abre paso**, no la que menos. DAR PASO abre un sentido; el ámbar abre los dos.
>
> **Lo que sigue sin preguntar es la EMERGENCIA** —`AMBAR_EMERGENCIA` del Esclavo y `ROJO TOTAL`
> del Maestro—, y ahí el motivo **no** es que no abra paso: es que se da viendo un accidente y una
> pregunta delante sería un paso más entre el operario y la maniobra. **La línea no la traza «abre
> o para»: la traza «hay tiempo de mirar o no lo hay»** — el razonamiento entero está en el
> comentario de la regla 1, dentro de `app.js`, justo encima de `const VIA_MANIOBRA`.

**La pregunta que encabeza el diálogo es «¿No quedan vehículos en el tramo?»**, y debajo lleva la
lista de lo que hay que mirar —**el tramo entero hasta la otra punta**, curvas y cambios de rasante,
vehículos parados o lentos, maquinaria, y gente trabajando o cruzando la calzada—. Los dos botones
son **«Todavía no»** y **«He mirado: el tramo está libre»** — el diálogo entero es el modal
`via-modal` del HTML: `grep -n 'id="via-modal"' 05_Funcional/App_Semaforo/www/index.html`.

#### Las tres reglas de cuándo aparece, y ninguna es evidente

1. 🛑 **NUNCA se pregunta cuando NO HAY TIEMPO DE MIRAR.** ~~*«Poner rojo total, poner ámbar y
   volver al menú no preguntan nada»*~~ → **el ámbar SÍ pregunta desde `N-148`** (ver la caja de
   arriba). Lo que no pregunta es **`ROJO TOTAL`, `AMBAR EMERGENCIA` y `VOLVER AL MENÚ`**. El
   razonamiento entero está en el comentario de esta regla dentro de `app.js`, encima de
   `const VIA_MANIOBRA`: **preguntar donde no hay tiempo de mirar enseña a decir que sí sin leer**,
   y el día que la pregunta llegue en serio ya nadie la lee. Es además el mismo criterio que la app
   usa para el PIN — `FORZAR_ROJO` y `AMBAR_EMERGENCIA` están en la lista **sin PIN** a propósito:
   `grep -n "const SIN_PIN" 05_Funcional/App_Semaforo/www/app.js`.
2. ⚠️ **Se pregunta AUNQUE EL PIN YA ESTÉ PUESTO.** No son dos llaves de la misma puerta, y el
   fuente lo dice con estas palabras, justo encima de `const VIA_VIGENCIA_MS`: *«el técnico que tecleó su clave hace
   diez minutos tampoco ha mirado el tramo»*. Un técnico en modo `TÉCNICO` verá la pregunta igual.
3. ⏱️ **El «he mirado» CADUCA a los 30 s, y también en cuanto cambian las luces**
   (`VIA_VIGENCIA_MS = 30 * 1000`, y las tres condiciones del vale en `viaConfirmadaVigente()` —
   `grep -n "const VIA_VIGENCIA_MS\|function viaConfirmadaVigente" 05_Funcional/App_Semaforo/www/app.js`).
   Confirmar la vía y luego
   entretenerse dos minutos **no vale**: hay que volver a mirar. Y si el cruce cambió de fase entre
   el «he mirado» y la orden, **el vale se cae solo**, porque el tramo que se miró ya no es el tramo
   que se va a abrir.

#### Qué sigue pidiendo PIN — que NO es «nada»

**La confirmación de vía NO ha sustituido al PIN en todas partes.** ~~Censado el 04/09~~ →
**RE-CENSADO EL 05/09**, porque `N-148` movió una fila. Las columnas se leen de dos censos que se
corren solos:

```bash
grep -n "pedirPin(() =>"  05_Funcional/App_Semaforo/www/app.js   # quien pide PIN
grep -n "confirmarVia("   05_Funcional/App_Semaforo/www/app.js   # quien pregunta por el tramo
```

*(Comprobado el 05/09: **cinco** botones piden PIN y **tres** órdenes preguntan por el tramo — y
`SET_MODO:AMBAR` sale en las dos listas, que es la novedad de `N-148`.)*

| orden | ¿pide PIN? | ¿pregunta por el tramo? |
|---|---|---|
| `MANUAL:CAMBIAR_TURNO` (**DAR PASO**) · `SET_MODO:AUTO` (**AUTOMÁTICO**) | ❌ no | ✅ **sí** |
| 🆕 `SET_MODO:AMBAR` (**ÁMBAR**) | ✅ **sí** | ✅ **SÍ — `N-148`, 05/09.** ~~❌ no~~ |
| `FORZAR_ROJO` (**ROJO TOTAL**) · `AMBAR_EMERGENCIA` · `SET_MODO:MENU` · `SET_MODO:ALCANCE` | ❌ no *(lista `SIN_PIN`)* | ❌ no — **no hay tiempo de mirar** |
| `CANCELAR_AMBAR` · `SET_MODO:DEGRADADO` · `TEST_LEDS` · todos los botones de la pestaña Técnico | ✅ **sí** | ❌ no |

> 🔵 **`CANCELAR_AMBAR` es el caso que parece contradictorio y no lo es — y hay que decir contra
> QUÉ «ponerlo» se compara, porque en esta página hay dos ámbares distintos:** *retirar* el ámbar
> pide PIN mientras **`AMBAR_EMERGENCIA` del Esclavo** —el otro ámbar, el de la parada de
> emergencia— no lo pide. Devolver el cruce a dar verdes **abre paso**, y eso es justamente lo que
> el PIN custodia; el razonamiento está en el bloque `R-3` del fuente, encima del manejador de
> `btnOpCancelarAmbar` (`grep -n "btnOpCancelarAmbar" 05_Funcional/App_Semaforo/www/app.js`).
>
> **`SET_MODO:AMBAR` —el MODO ámbar del Maestro— es otra orden y sí pide PIN**, además de preguntar
> por el tramo desde `N-148`.

> ⚠️ **Y una precisión sobre el cable, para que nadie concluya de más: el PIN NO ha desaparecido de
> la trama.** La app sigue construyendo `CMD:PIN:1234:…` dentro de `enviarComandoFirmware()`
(`grep -n "function enviarComandoFirmware" 05_Funcional/App_Semaforo/www/app.js`) y el firmware sigue
> exigiéndolo donde siempre. **Lo que cambió es a quién se le pregunta en la pantalla, no lo que
> viaja por `J17`.** Un `CMD:PIN:` interceptado en el aire vale hoy lo mismo que ayer.

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

> 🔴 **LA COLUMNA DE NÚMEROS DE LÍNEA SE RETIRÓ EL 05/09, Y EL MOTIVO ES QUE NO SE PUEDE MANTENER.**
> Se censaron en agosto, se **re-midieron** el 04/09 con este mismo aviso encima —*«un número de línea
> en un manual caduca solo»*— y el 05/09 volvían a estar corridas: `SET_MODO:INTELIGENTE` se citaba en
> `:490` y vive en `:585`. **Re-numerar a mano es escribir cifras que caducan en una semana.**
>
> **La primera columna YA ES el ancla.** El despachador compara el literal del comando, y ese literal
> no se mueve al crecer el fichero:
>
> ```bash
> # las 20 ramas del despachador del Maestro, en orden:
> grep -n 'strcmp(accion\|strncmp(accion\|strcmp(cmd\|strncmp(cmd' 01_Firmware/Maestro/src/bluetooth.cpp
>
> # una fila cualquiera, por su literal:
> grep -n '"SET_MODO:INTELIGENTE"' 01_Firmware/Maestro/src/bluetooth.cpp
> ```
>
> *(Comprobado el 05/09: el primer `grep` devuelve **20 líneas** — las **18 filas** de la tabla más
> las **2** de reparto: el `strncmp(cmd, "CMD:PIN:1234:", 13)` y el `strncmp(cmd, "CMD:", 4)` que
> abre los alias sin PIN. El segundo devuelve **una**.)*

| Trama | Notas |
|---|---|
| `$LATIDO` | 🆕 **04/09.** No es una orden: es la línea que el puente emite para que el troceador cierre un silencio. **El equipo NO contesta nada, a propósito** — sin esta rama caía en la guarda de PIN y devolvía un `AUTH_FAILED` cada dos segundos |
| `CMD:FORZAR_ROJO` | **Sin PIN**, a propósito. Rojo total |
| `CMD:SET_MODO:MENU` | **Sin PIN** (alias). No abre paso |
| `CMD:SET_MODO:ALCANCE` | **Sin PIN** (alias) |
| `CMD:PIN:1234:SET_MODO:AUTO` | ~~⚠️ **reinicia los tiempos a 1/1/15**~~ → 🟢 **04/09: YA NO.** Respeta los tiempos guardados y los recupera del respaldo (`N-133` + `N-42`). Ver §5.5.1 |
| `CMD:PIN:1234:SET_MODO:MANUAL` | |
| `CMD:PIN:1234:SET_MODO:AMBAR` | **Es un MODO, no el latch de emergencia.** El Maestro no tiene `AMBAR_EMERGENCIA`. 🔴 **05/09: la app pide PIN Y confirmación de vía para él** (`N-148`, §4.bis.4) |
| `CMD:PIN:1234:SET_MODO:MENU` | **sustituye al botón *Cancelar*** retirado. En Degradado sale por todo-rojo |
| `CMD:PIN:1234:SET_MODO:ALCANCE` | |
| `CMD:PIN:1234:SET_MODO:INTELIGENTE` | El modo que atiende la DEMANDA de las cámaras. 🛑 **«Cámara» aquí es UN CONTACTO SECO: el firmware lee el nivel de un pin y nada más. Ni imagen, ni vídeo, ni red — `§1.bis` y `D-12`** |
| `CMD:PIN:1234:SET_MODO:DEGRADADO` | **contesta el motivo concreto del rechazo** |
| `CMD:PIN:1234:FORZAR_ROJO` | Igual que la forma sin PIN |
| `CMD:PIN:1234:MANUAL:CAMBIAR_TURNO` | `$ERR,…,DESC:EN_TRANSICION_REINTENTE` si no está en reposo |
| `CMD:PIN:1234:TEST_LEDS` | |
| `CMD:PIN:1234:SET_TIEMPOS:v,r,d` | Un `$ERR` por motivo: `FORMATO_INVALIDO`, `EN_MARCHA_PARE_EL_MODO`, `RANGO`. **Rangos nuevos del 04/09 en §5.5** |
| `CMD:PIN:1234:SET_RTC:YYYY-MM-DD,HH:MM:SS` | **Cinco ramas** — ver 5.3 |
| `CMD:PIN:1234:REINICIAR_RELOJ` | diagnóstico del cristal |
| `CMD:PIN:1234:DEMANDA` | **sólo en Modo Inteligente**: fuera de él, `$ERR,…,DESC:SOLO_EN_MODO_INTELIGENTE` |

Cualquier otra cosa: `$ERR,CMD:DESCONOCIDO,DESC:COMANDO_NO_SOPORTADO`
(`grep -n "COMANDO_NO_SOPORTADO" 01_Firmware/Maestro/src/bluetooth.cpp`).

> **Los marcados 🆕 son los que la app tiene que exponer y este manual no mencionaba.** Un comando
> del firmware sin interfaz en la app es un modo al que ya no se puede llegar: la LCD se retiró y
> *Aceptar* está mudo.

### 5.2 🛑 Lo que despacha el **ESCLAVO** — y NO es lo mismo

**Censado el 31/08, re-medido el 04/09, y desde el 05/09 SIN COLUMNA DE LÍNEAS** — por lo mismo que
la 5.1: el literal es el ancla y el número caduca solo.

```bash
# las 10 ramas del despachador del Esclavo, en orden:
grep -n 'strcmp(accion\|strncmp(accion\|strcmp(cmd\|strncmp(cmd' 01_Firmware/Esclavo/src/bluetooth.cpp

# una fila cualquiera, por su literal:
grep -n '"CANCELAR_AMBAR"' 01_Firmware/Esclavo/src/bluetooth.cpp
```

*(Comprobado el 05/09: el primer `grep` devuelve **10 líneas** — las **9 filas** de la tabla más la
guarda `strncmp(cmd, "CMD:PIN:1234:", 13) != 0`. El segundo devuelve **una**.)*

| Trama | Qué hace |
|---|---|
| `$LATIDO` | 🆕 **04/09.** Igual que en el Maestro: **no se contesta**, sólo cierra un silencio |
| `CMD:AMBAR_EMERGENCIA` | **Sin PIN.** Ámbar intermitente + latch que **veta las órdenes de radio**. 🟢 **04/09 (`N-106`): y si el Degradado gobierna la luz, SALE de él por el todo-rojo** — el `RESULT` ya no es `OK` siempre, son **cinco**. Ver abajo |
| `CMD:PIN:1234:AMBAR_EMERGENCIA` | Lo mismo, con PIN. **El mismo bloque letra por letra** — lo vigilan `esclavo_07` y `esclavo_08` |
| `CMD:PIN:1234:CANCELAR_AMBAR` | 🆕 **faltaba en este manual.** **Retira** el latch, **con PIN**. `RETIRADO` · `RETIRADO_QUEDA_MANDO` · `$ERR,…,DESC:NO_HAY_AMBAR_VIGENTE` |
| `CMD:PIN:1234:SOLICITAR_PASO` | **Pide**, no ordena. `PEDIDO_AL_MAESTRO` / `$ERR,…,DESC:REPITA_EN_UNOS_SEGUNDOS`. **Y el `PEDIDO_AL_MAESTRO` se puede desmentir después — `N-130`, abajo** |
| `CMD:PIN:1234:SET_RTC:…` | `OK` / `SIN_CRISTAL` / `FORMATO_INVALIDO` |
| ~~`CMD:FORZAR_ROJO`~~ | 🛑 `$ERR,…,DESC:RENOMBRADO_USE_AMBAR_EMERGENCIA` |
| ~~`CMD:PIN:1234:FORZAR_ROJO`~~ | 🛑 mismo `$ERR`. **Las dos formas** |
| ~~`CMD:PIN:1234:TEST_LEDS`~~ | 🛑 `$ERR,…,DESC:NO_EN_SERVICIO_USE_EL_MAESTRO` |

Cualquier otra cosa: `$ERR,CMD:DESCONOCIDO,DESC:COMANDO_NO_SOPORTADO_EN_ESCLAVO`
(`grep -n "COMANDO_NO_SOPORTADO_EN_ESCLAVO" 01_Firmware/Esclavo/src/bluetooth.cpp`).

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

> ✏️ **DISCREPANCIA MEDIDA EL 04/09, y se publica en vez de taparse: el título de este apartado dice
> CINCO ramas y la tabla enseña CUATRO** —y el párrafo de debajo dice *«las cuatro»*—. **No se
> arregla aquí escribiendo una quinta fila a mano**: inventar la que falta es exactamente lo que este
> repositorio castiga. **La lista buena sale de censar la rama `SET_RTC:` del despachador**
> (`grep -n '"SET_RTC:"' 01_Firmware/Maestro/src/bluetooth.cpp`), y ese censo no se ha hecho en
> esta pasada — **`SIN VERIFICAR`**. **Mientras tanto, la regla
> de campo no depende de la cuenta: lea el `RESULT:` o el `DESC:` que llegue, y si no lo reconoce, la
> app se lo enseña en crudo** marcado como *«motivo sin traducir»* (§5.4.2.ter). **Lo que no vale es
> tratar cualquiera de ellas como «enviado».**
>
> 🛑 **Y de las que sí están medidas, una sigue SIN DIAGNOSTICAR: `FORMATO_INVALIDO`.** Aparece en el
> Courier RTC, la app ya lo traduce, y **nadie sabe por qué se produce**. Traducir un síntoma no es
> repararlo, y aquí no se propone ninguna causa.

---

## 5.4 🆕 LO QUE LA APP HACE DESDE EL 01/09 Y ANTES NO — tres cosas que se notan en campo

### 5.4.1 La app **comprueba el checksum y descarta lo corrompido**

Antes cortaba la trama por el `*` y **tiraba el checksum sin mirarlo**: una trama corrompida en el
aire se pintaba como si fuera buena. Hoy lo calcula, lo compara y, si no cuadra, **no pinta nada**
— `NMEAParser.calcularChecksum()` lo calcula y `NMEAParser.validarTrama()` lo compara; la app
descarta lo que esa función rechaza:

```bash
grep -n "calcularChecksum(payload)\|validarTrama" 05_Funcional/App_Semaforo/www/js/nmea_parser.js
grep -n "NMEAParser.validarTrama" 05_Funcional/App_Semaforo/www/app.js
```

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
>
> 🔴 **PERO NO ES LO QUE HAY QUE MANDAR CUANDO ALGO FALLE.** La cinta es **el cable**: trae lo que
> pasó por él, en orden y sin interpretar, **y nada más**. Una orden que la app frenó antes de
> escribir un byte **no aparece aquí**, porque por el cable no pasó. Para reportar una avería se
> exporta **el Diario**, que es lo que cuenta abajo.

### 5.4.2.bis 🆕 EL **DIARIO DE ÓRDENES** — y es LO QUE HAY QUE MANDAR CUANDO ALGO FALLE

**Es un registro nuevo del 04/09, distinto de la cinta y con otra pregunta detrás.** La cinta
contesta *«¿qué salió exactamente por el cable?»*; el Diario contesta **«¿y qué pasó?»**. Están los
dos porque **ninguno de los dos responde a la otra pregunta** — son los dos objetos que abren
`App_Semaforo/www/js/depuracion.js`: `grep -n "const RegistroCrudo\|const DiarioOrdenes" 05_Funcional/App_Semaforo/www/js/depuracion.js`.

**Una línea por ORDEN, con su terna `orden / respuesta / efecto`** — formato literal del fuente
— el bloque de cabecera de `DiarioOrdenes`, en `depuracion.js`:

```text
   18:14:26  ORDEN      CMD:PIN:****:SET_MODO:AUTO
   18:14:26  RESPUESTA  $ACK,CMD:SET_MODO:AUTO,RESULT:OK   (+0,3 s)
             EFECTO     NO CAMBIO NADA: MODO siguio en AUTO y ESTADO en R1_R2
```

**El `EFECTO` es la columna que ningún otro instrumento tenía**, y es la que caza el defecto que
este proyecto persigue por nombre: **un `$ACK` que dice `OK` y no mueve nada.** El Diario compara el
~~`MODO` y el `ESTADO`~~ → 🔴 **05/09: son TRES campos y el tercero es `ESC:`** (`N-149`) — lo que
el Maestro sabe de la OTRA punta, que es justo el dato que faltaba el 04/09 cuando un operario
pulsaba ÁMBAR seis veces por no ver moverse el otro poste. Con `ESC` dentro, la ventana de efecto
publica también *«ESC: ROJO → AMBAR»*. **MEDIDO:** `grep -n "CAMPOS_EFECTO:" 05_Funcional/App_Semaforo/www/js/depuracion.js`. Compara
los `$STATUS` de antes y de después y
lo escribe en la misma línea. Un `RESULT:OK` con un *«NO CAMBIÓ NADA»* al lado es un hallazgo, no
una anécdota.

| dato | valor | dónde |
|---|---|---|
| Órdenes que guarda | **120** | `TOPE` |
| Espera para casar la respuesta | **5 s** | `VENTANA_RESPUESTA_MS` |
| Espera para casar el efecto | **95 s** | `VENTANA_EFECTO_MS` |
| ¿Sobrevive a cerrar la app? | 🛑 **NO** — `PERSISTE: false` | vive en memoria |

> ⚠️ **Las cuatro son propiedades del objeto `DiarioOrdenes`, y `TOPE` y `PERSISTE` existen DOS
> veces en el fichero** — la otra pareja es de `RegistroCrudo`, la cinta, y vale **300**. Al
> buscarlas hay que quedarse con las que salen **después** de `const DiarioOrdenes`:
>
> ```bash
> grep -n "const DiarioOrdenes\|TOPE:\|VENTANA_RESPUESTA_MS:\|VENTANA_EFECTO_MS:\|PERSISTE:" \
>      05_Funcional/App_Semaforo/www/js/depuracion.js
> ```

> 🛑 **Y eso último manda sobre el procedimiento: el Diario NO SE GUARDA SOLO.** Si se cierra la app
> —o caduca la sesión y alguien la reinicia— **se pierde**. **Se exporta ANTES de irse del poste**,
> no al llegar a la oficina.

#### El procedimiento de reporte, en tres pasos

1. **En cuanto algo no cuadre, NO cierre la app.** Vaya a modo Técnico → pestaña `Tramas`.
2. **Exporte el Diario** con su botón. Sale un fichero de texto llamado
   **`Ordenes_<cruce>_AAAA-MM-DD.txt`** (`nombreFicheroDiario()` en `app.js`). Si no se puede guardar fichero, el
   otro botón lo copia al portapapeles.
3. **Adjunte también la cinta** si el problema huele a enlace —ruido, tramas rotas, silencios—:
   sale como **`Tramas_<cruce>_AAAA-MM-DD.txt`** (`nombreFicheroDepuracion()`;
   `grep -n "function nombreFicheroDiario\|function nombreFicheroDepuracion" 05_Funcional/App_Semaforo/www/app.js`).
   **Son dos ficheros distintos
   y no se sustituyen.**

> ✅ **El PIN sale TAPADO con asteriscos en los dos ficheros**, y el propio encabezado del Diario lo
> dice: *«El PIN sale tapado con asteriscos; al equipo se le manda entero»*.
> El tapado está aplicado **por separado en cada registro** —**una definición, `taparPin()`, y TRES
> llamadas: una en la cinta y dos en el Diario**— con este comentario al lado, que explica por qué no basta con hacerlo
> una vez: *«los dos registros se exportan por separado y una barrera que cubre uno deja el otro
> abierto»*. **Un Diario exportado se puede mandar por correo sin repasarlo a mano.**
>
> ```bash
> grep -n "taparPin" 05_Funcional/App_Semaforo/www/js/depuracion.js
> ```
>
> *(Comprobado el 05/09: devuelve **seis** líneas — la definición, sus **tres** llamadas y dos
> menciones en comentarios. Lo que se cuenta son las tres llamadas.)*

> 🔵 **Y la diferencia que hace útil al Diario en el caso peor:** una orden que la app **bloqueó**
> —sin PIN, sin confirmar la vía, o dirigida a la punta equivocada— **sí entra en el Diario**, con
> su motivo, y **no** entra en la cinta —la barrera y su porqué están en `enviarComandoFirmware()`—.
> Es decir: el Diario enseña **las órdenes
> que el operario creyó dar y no salieron**, que es exactamente lo que nadie podía ver antes.

### 5.4.2.ter 🆕 LOS RECHAZOS DEL EQUIPO YA SE TRADUCEN — y dicen qué hacer en el poste

Antes, un `$ERR,CMD:SET_RTC,DESC:SIN_CRISTAL` llegaba a la pantalla tal cual. Hoy la app lo traduce
a lenguaje de obra y **dice qué hacer**.

**CENSADO EL 04/09, y la cuenta se publica desglosada porque no es un solo número:**

| tabla | entradas | qué traduce | dónde |
|---|---|---|---|
| `ERR_MOTIVO` | **22** | el `DESC:` del rechazo | `const ERR_MOTIVO` |
| `ERR_TEXTO` | **1** — el par `CANCELAR_AMBAR` + `NO_HAY_AMBAR_VIGENTE` | pares de `CMD` y `DESC` que sólo tienen sentido juntos | `const ERR_TEXTO` |
| | **23 en total** | | resueltas por `_traducirRechazo()` |

```bash
grep -n "const ERR_MOTIVO\|const ERR_TEXTO\|function _traducirRechazo" 05_Funcional/App_Semaforo/www/app.js
```

*(Las **22** y la **1** se cuentan sobre las claves de primer nivel de cada objeto, re-contadas el
05/09. La cuenta se re-hace, no se copia.)*

> ✏️ **El «23» se desglosa a propósito, y esta nota es el motivo.** Al medirlo aparecieron **tres**
> números distintos que se parecen: las **22** entradas de `ERR_MOTIVO`, las **23** de las dos
> tablas juntas, y un **23** que aparece en un comentario del propio `app.js` —encima de
> `const ERR_TEXTO`, `grep -n "23 literales de DESC" 05_Funcional/App_Semaforo/www/app.js`— refiriéndose a
> otra cosa —los literales `DESC:` que existen **en el firmware**—. **Los tres son ciertos y ninguno
> es el otro.** Publicar «23 motivos» a secas habría sido correcto por casualidad.

**Los seis del reloj son un grupo aparte** —`SIN_RELOJ_NO_RESPONDE`, `ESCRITURA_FALLIDA`,
`NO_QUEDO_PUESTA`, `OSCILADOR_PARADO_CAMBIE_PILA`, `MOTIVO_NO_CONTEMPLADO` y
`SIGUE_PARADO_VEA_CONSULTA_RELOJ`— y ahí hay que leer el aviso de la cabecera de este manual: **el
error `FORMATO_INVALIDO` del Courier RTC sigue SIN DIAGNOSTICAR.** Que la app ahora lo traduzca
**no lo arregla ni lo explica**: sólo hace legible el síntoma.

> ✅ **Y lo que la app hace con un motivo que NO conoce está bien hecho y hay que saberlo:** lo
> enseña **en crudo**, marcado con *«motivo sin traducir en esta versión de la app»*
> (`grep -n "motivo sin traducir" 05_Funcional/App_Semaforo/www/app.js`). **No lo esconde y no se inventa una explicación.** Si ve esa marca, el motivo
> real está en la pestaña `Tramas` y hay que reportarlo tal cual: significa que el firmware ha
> crecido por delante de esta APK.

### 5.4.3 El **PIN ya no se puede armar con el teclado cerrado**

Los botones del teclado siguen existiendo en la página aunque el modal esté oculto, así que
cualquier cosa que disparara una pulsación sobre ellos **autorizaba la sesión sin que la barrera se
hubiera abierto nunca**. Hoy los cuatro caminos que tocan el PIN comprueban primero que el teclado
esté delante del operario —`tecladoPinAbierto()`, **una definición y cuatro llamadas**:
`grep -n "tecladoPinAbierto" 05_Funcional/App_Semaforo/www/app.js—, y
**cerrar el teclado cancela de verdad**: borra los dígitos y la acción pendiente.

> ⚠️ **Esto no lo nota el operario, y por eso se escribe.** Lo que cambia es que un modo técnico
> abierto **significa que alguien tecleó cuatro dígitos**, que es lo que el PIN existía para
> garantizar.
>
> ✅ **AL DÍA EL 04/09.** ~~*«lo que este arreglo NO cubre: ese permiso NO CADUCA (`AB-9`,
> abierto)»*~~ → **`AB-9` está cerrado en el fuente**: el permiso caduca a los **60 s** de guardarse
> el teléfono y a los **5 min** sin mandar órdenes. Ver `§4.bis.3`, **incluido su aviso `SIN
> VERIFICAR`**: la caducidad por segundo plano cuelga de sucesos del navegador y **nadie la ha visto
> disparar en la APK, con la pantalla apagada**, que es el escenario para el que existe.
>
> **Las dos mitades siguen siendo distintas y las dos hacen falta:** este apartado garantiza que
> **alguien tecleó** cuatro dígitos; `§4.bis.3` es **cuánto dura** ese permiso después.

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

*(Medido en `01_Firmware/ESP32_Expansion/src/vigilante.cpp`: el formato es la plantilla que lleva
`EVT:ARRANQUE`, y las **once** causas posibles son los `case` de `nombreCausa()`. Re-contadas el
05/09.)*

```bash
grep -n "EVT:ARRANQUE\|nombreCausa" 01_Firmware/ESP32_Expansion/src/vigilante.cpp
```

> 🔴 **`ARRANQUES:` subiendo entre visitas es el síntoma que ningún otro instrumento da.** Un puente
> que se reinicia solo cada pocos minutos **parece un enlace intermitente**, y se acaba cambiando la
> radio o el cable. Este campo dice que el problema es el módulo.
>
> ⚠️ **La app no le da pantalla propia: sale como un evento más.** Se lee en `Eventos` o en `Tramas`.

---

## 5.5 🟢 LO QUE CAMBIÓ EL 04/09 — TRES COSAS QUE LA APP TIENE QUE REFLEJAR

### 5.5.1 🔴 El ciclo mínimo sube de 1 a 3 minutos, y la guarda de verdad NO es la app

**Rangos vigentes, ~~MEDIDOS en `Maestro/src/modo_automatico.cpp:51-53`~~** → 🔴 **05/09: esa cita
apunta al fichero equivocado.** En `modo_automatico.cpp` sólo se USAN; los seis números viven en
**`Maestro/include/limites_ciclo.h`**, en tres líneas, y ahí es donde se cambian:

```bash
grep -n "VERDE_MIN_MIN\|ROJO_MIN_MIN\|DESPEJE_SEG_MIN" 01_Firmware/Maestro/include/limites_ciclo.h
```

**MEDIDOS el 05/09:**

| | mínimo | máximo |
|---|---|---|
| Verde | **3 min** *(era 1)* | 15 min |
| Rojo | **3 min** *(era 1)* | 15 min |
| Despeje (todo-rojo) | 10 s | 90 s |

**El motivo es una decisión vial del responsable, literal: «tres minutos es la mínima distancia de
seguridad».** En un paso alternado de un solo carril un camión pesado tarda entre 5 y 8 s sólo en
reaccionar y arrancar; con un verde de 60 s pasan tres o cuatro vehículos antes de cortar a ámbar, y
lo que sale de ahí no es una cola: es un conductor convencido de que el semáforo está averiado.

> 🔵 **La app valida lo mismo —`enRango(verde, 3, 15)`, `grep -n "enRango(verde" 05_Funcional/App_Semaforo/www/app.js`—
> pero eso es COMODIDAD, no la
> barrera.** La barrera vive en el firmware y rechaza con `$ERR,CMD:SET_TIEMPOS,DESC:RANGO`. **Y es
> la única que vale:** la app no es la única que habla por `J17`, y **una APK vieja de las que
> sobreviven en los teléfonos puede mandar `SET_TIEMPOS` con un minuto**. Una guarda que sólo vive
> en la interfaz es de cortesía.
>
> **Para quien mantiene la app, la regla que sale de aquí:** el rango de la app se valida contra el
> `.cpp`, y **si algún día divergen, el que manda es el firmware**. Ofrecer un valor que el equipo
> va a rechazar es el defecto que este proyecto ya conoce con otro nombre.

> 🟢 **CERRADO EL 04/09 POR LA TARDE (`N-133` + `N-42`). Este aviso estaba ABIERTO y ya no lo está:**
>
> ~~*«los valores por defecto del modo siguen siendo 1 min / 1 min / 15 s y no pasan por la guarda. `SET_MODO:AUTO` llama a `modoAutomatico_setup()`, que reescribe
> los tres valores a `1, 1, 15`: unos tiempos aceptados con `$ACK` se pierden al arrancar el
> modo»*~~ → 🛑 **CADUCADO.**
>
> **MEDIDO en `Maestro/src/modo_automatico.cpp`, y son tres cambios, no uno:**
>
> ```bash
> grep -n "minRojo = ROJO_MIN_MIN\|recuperarTiemposGuardados\|respaldo_guardarTiemposCiclo" \
>      01_Firmware/Maestro/src/modo_automatico.cpp
> ```
>
> 1. **Los valores por defecto ya no son `1,1,15`: son los propios mínimos** — el inicializador es
>    `static int minRojo = ROJO_MIN_MIN, minVerde = VERDE_MIN_MIN, segEstatico = DESPEJE_SEG_MIN;`,
>    o sea **3 min / 3 min / 10 s**. El `1, 1, 15` sobrevive sólo en el comentario que lo tacha,
>    unas líneas más arriba en el mismo bloque.
> 2. **`modoAutomatico_setup()` YA NO PISA LOS TIEMPOS.** En su lugar llama a
>    `recuperarTiemposGuardados()` y respeta lo que haya.
> 3. **Y ahora sobreviven al corte de energía** (`N-133`): `modoAutomatico_fijarTiempos()` los
>    escribe en el respaldo con pila —es su **único** llamador— y `recuperarTiemposGuardados()` los
>    relee al arrancar, **revalidando el rango aunque el checksum apruebe** — porque un equipo
>    actualizado puede traer guardado
>    un ciclo de 1 minuto perfectamente íntegro, escrito cuando 1 era legal. **Un dato íntegro no es
>    un dato válido.**
>
> ✅ **Consecuencia para la app: lo que la pantalla muestre después de un `SET_MODO:AUTO` ya SÍ es lo
> que el equipo está usando.** La app no tiene que fingir nada ni advertir de nada aquí.
>
> ⚠️ **Sigue siendo MEDIDO sobre fichero. Nadie lo ha cargado en una tarjeta.**

#### 🛑 Y una consecuencia operativa NUEVA que el técnico va a encontrarse: en Automático NO se pueden cambiar los tiempos

**Hay que salir del modo, poner los tiempos, y volver a entrar.** El equipo contesta
`$ERR,CMD:SET_TIEMPOS,DESC:EN_MARCHA_PARE_EL_MODO`, y hay una **segunda guarda dentro del propio
`modoAutomatico_fijarTiempos()`** —su `if (modoAutomatico_enMarcha()) return false;`— para el que
llegue por otro camino:

```bash
grep -n "EN_MARCHA_PARE_EL_MODO"    01_Firmware/Maestro/src/bluetooth.cpp
grep -n "modoAutomatico_enMarcha"   01_Firmware/Maestro/src/modo_automatico.cpp
```

**No es capricho, y el motivo va escrito porque es vial:** la duración se recalcula en cada vuelta a
partir de esas variables, así que **bajar un tiempo a mitad de fase acortaría la fase EN CURSO — y
una de esas fases es el todo-rojo de despeje**. Recortar un despeje ya empezado es abrir el otro
sentido sobre un tramo que todavía no está vacío.

> ⚠️ **Y un detalle que la app enseña mal y hay que conocer al leer la pantalla:** la segunda guarda
> —la de dentro del setter— devuelve `false`, y el despachador traduce **todo** `false` a
> `DESC:RANGO`. **Un rechazo por «el modo está en marcha» que llegue por ese camino se pinta como si
> fuera un rango fuera de límites.** Si el técnico ve `RANGO` con unos tiempos que sabe correctos,
> **lo primero que hay que comprobar es si el Modo Automático está corriendo.**

> 🔵 **De dónde salía todo esto, porque es la parte reutilizable (`N-135`, horas después de
> `N-42`):** al retirar el asistente quedó un `enum FaseAuto { CORRIENDO; }` de **un solo valor**, y
> `modoAutomatico_enMarcha()` lo comparaba. Una comparación con un único enumerador es **cierta
> siempre** —el compilador la reduce a `movs r0, #1`—, así que el equipo contestaba
> `EN_MARCHA_PARE_EL_MODO` **a todo y para siempre**, en todos los modos, desde antes del `setup()`.
> Y como ese setter es el **único** llamador de `respaldo_guardarTiemposCiclo()`, `N-133` se había
> quedado **con camino de lectura y sin camino de escritura**. Hoy es
> `return modoActual_get() == MODO_AUTOMATICO;`, que sí puede dar las dos respuestas.

### 5.5.2 🟢 DECISIÓN: el cruce se opera desde el MAESTRO

**No se va a hacer transparente el mando desde el Esclavo.** La asimetría de `§5.1` y `§5.2` deja de
ser una limitación pendiente y pasa a ser **cómo se opera este equipo**.

| desde el **MAESTRO** | desde el **ESCLAVO** |
|---|---|
| cambiar de modo · dar paso · ajustar tiempos · `TEST_LEDS` · rojo total | **sólo** `SOLICITAR_PASO`, `AMBAR_EMERGENCIA`, `CANCELAR_AMBAR` y `SET_RTC` |

**La app ya lo enruta y no hace falta tocarla:** las dos listas `SOLO_MAESTRO` y `SOLO_ESCLAVO`, y
`puntaCorrecta()`, que avisa a qué punta va la orden en vez de mandarla al vacío.

```bash
grep -n "const SOLO_MAESTRO\|const SOLO_ESCLAVO\|function puntaCorrecta" 05_Funcional/App_Semaforo/www/app.js
```

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

**MEDIDO** en las dos puntas — el Maestro decide con `modoActual_get() == MODO_INTELIGENTE` y manda
`DEMANDA_ACEPTADA` / `DEMANDA_RECHAZADA`; el Esclavo lo convierte en el evento:

```bash
grep -n "DEMANDA_ACEPTADA"                 01_Firmware/Maestro/src/coordinador.cpp
grep -n "DEMANDA_NO_ATENDIDA_MODO_ACTUAL"  01_Firmware/Esclavo/src/main.cpp
``` **Antes esa rama estaba vacía: la app decía «pedido al Maestro» y no pasaba nada.**

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

> 🛑 **Y lo que estas cinco filas NO son: imágenes de cámara.** Son capturas de la PANTALLA DEL
> TELÉFONO, hechas por el arnés E2E con Puppeteer. **De las cámaras del cruce esta app no recibe
> ninguna imagen — sólo el nivel de un pin: `§1.bis` y `D-12`.** La galería no las mezcla y no debe
> mezclarlas nunca.

| Vista | Captura (de PANTALLA) | Descripción |
|---|---|---|
| **Modo Operario** | `evidencia/01_modo_operario_principal.png` | Botonera táctil y réplica 3D de semáforos. ~~*«réplica del mando de relés»*~~ → **31/08: es la superficie de mando principal**, no una réplica de nada. ⚠️ **04/09: la captura enseña ~~4~~ botones y la botonera de hoy tiene SIETE rótulos, tres de ellos condicionados a la punta (§1). La captura está caducada, no la botonera** |
| **Selector de Cruces** | `evidencia/02_modal_cruces_abierto.png` | Catálogo de frentes de obra y selección inmediata |
| **Modo Técnico** | `evidencia/06_modo_tecnico_activo.png` | Desbloqueo por PIN y pestañas de configuración |
| **Ajustes de Tiempos** | `evidencia/07_tiempos_guardados_exito.png` | Programación de tiempos de verde, rojo y despeje |
| **Courier RTC** | `evidencia/08_courier_rtc_inyectado.png` | Inyección de hora compensada en el nodo Esclavo |

---
**Desarrollado y Validado para el Proyecto Controladora_Semaforos V9.0**
