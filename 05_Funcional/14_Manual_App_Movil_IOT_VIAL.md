# 📱 MANUAL DE USUARIO Y ARQUITECTURA DE LA APP MÓVIL IOT-VIAL (V9.0)

**Sistema:** Centro de Control, Operación y Diagnóstico Semafórico a Nivel de Suelo  
**Rama Git:** `feat/n69-ajustes-tiempos`  
**Plataforma:** Android Nativo (.APK) y Web Testing PWA  
**Protocolo:** ~~Bluetooth Serial SPP (HC-05 / JDY-31 a 9600 bps) / BLE GATT~~ → **Bluetooth Serial SPP a 9600 bps contra el módulo de expansión `ESP32-WROOM-32`**. Ver el aviso de cabecera  
**Fecha de Actualización:** 28 de Agosto de 2026  
**Última revisión:** 31 de agosto de 2026  
**Versión de Firmware Compatible:** V8.9 / V9.0 Definitiva — ⚠️ **en campo corre la V8.4**  
**Archivo APK Compilado:** `05_Funcional/IOT_VIAL_Semaforos_2026-08-28_a8e1ceb_SIN_BANCO.apk`  

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
│   • Botonera tactil de campo de 4 botones grandes                       │
│     (31/08: NO es una "replica del mando" - es LA superficie de mando)  │
│   • 1 toque: 🟢 Automatico | ✋ Dar Paso | 🟡 Ambar | 🛑 Caida segura     │
│     OJO: la caida segura NO es el mismo comando en las dos puntas.      │
│          Maestro -> CMD:FORZAR_ROJO      (rojo total)                   │
│          Esclavo -> CMD:AMBAR_EMERGENCIA (ambar). FORZAR_ROJO da $ERR.  │
│   • Cero contrasenas ni formularios complejos para el operario de obra.  │
├──────────────────────────────────────────────────────────────────────────┤
│ 🛡️ MODO TÉCNICO / ADMINISTRADOR (PIN '1234'):                           │
│   • Ajustes de tiempos de ciclo (Verde 1-15m, Rojo 1-15m, Despeje 10-90s)│
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
   *El script sincroniza los assets web con Capacitor (`npx cap sync android`) y ejecuta Gradle `assembleDebug` generando el archivo maestro en `05_Funcional/IOT_VIAL_Semaforos_2026-08-28_a8e1ceb_SIN_BANCO.apk` en ~20 segundos.*

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

### 5.1 Lo que despacha el **MAESTRO** — censado sobre `Maestro/src/bluetooth.cpp` el 31/08

| Trama | Línea | Notas |
|---|---|---|
| `CMD:FORZAR_ROJO` | `:145` | **Sin PIN**, a propósito. Rojo total |
| `CMD:SET_MODO:MENU` | `:168` | **Sin PIN** (alias). No abre paso |
| `CMD:SET_MODO:ALCANCE` | `:168` | **Sin PIN** (alias) |
| `CMD:PIN:1234:SET_MODO:AUTO` | `:177` | |
| `CMD:PIN:1234:SET_MODO:MANUAL` | `:182` | |
| `CMD:PIN:1234:SET_MODO:AMBAR` | `:187` | |
| `CMD:PIN:1234:SET_MODO:MENU` | `:191` | 🆕 **sustituye al botón *Cancelar*** retirado |
| `CMD:PIN:1234:SET_MODO:ALCANCE` | `:212` | 🆕 |
| `CMD:PIN:1234:SET_MODO:INTELIGENTE` | `:223` | 🆕 modo de cámaras |
| `CMD:PIN:1234:SET_MODO:DEGRADADO` | `:234` | 🆕 **contesta el motivo concreto del rechazo** |
| `CMD:PIN:1234:FORZAR_ROJO` | `:253` | Igual que la forma sin PIN |
| `CMD:PIN:1234:MANUAL:CAMBIAR_TURNO` | `:257` | |
| `CMD:PIN:1234:TEST_LEDS` | `:271` | |
| `CMD:PIN:1234:SET_TIEMPOS:v,r,d` | `:275` | Un `$ERR` por motivo: `FORMATO_INVALIDO`, `EN_MARCHA_PARE_EL_MODO`, `RANGO` |
| `CMD:PIN:1234:SET_RTC:YYYY-MM-DD,HH:MM:SS` | `:295` | **Cinco ramas** — ver 5.3 |
| `CMD:PIN:1234:REINICIAR_RELOJ` | `:330` | 🆕 diagnóstico del cristal |
| `CMD:PIN:1234:DEMANDA` | `:345` | 🆕 solo en Modo Inteligente |

Cualquier otra cosa: `$ERR,CMD:DESCONOCIDO,DESC:COMANDO_NO_SOPORTADO` (`:363`).

> **Los marcados 🆕 son los que la app tiene que exponer y este manual no mencionaba.** Un comando
> del firmware sin interfaz en la app es un modo al que ya no se puede llegar: la LCD se retiró y
> *Aceptar* está mudo.

### 5.2 🛑 Lo que despacha el **ESCLAVO** — y NO es lo mismo

| Trama | Línea | Qué hace |
|---|---|---|
| `CMD:AMBAR_EMERGENCIA` | `:130` | **Sin PIN.** Ámbar intermitente + latch que **veta las órdenes de radio** |
| `CMD:PIN:1234:AMBAR_EMERGENCIA` | `:171` | Lo mismo, con PIN |
| `CMD:PIN:1234:SOLICITAR_PASO` | `:184` | **Pide**, no ordena. `PEDIDO_AL_MAESTRO` / `$ERR,…,DESC:REPITA_EN_UNOS_SEGUNDOS` |
| `CMD:PIN:1234:SET_RTC:…` | `:215` | `OK` / `SIN_CRISTAL` / `FORMATO_INVALIDO` |
| ~~`CMD:FORZAR_ROJO`~~ | `:157` | 🛑 `$ERR,…,DESC:RENOMBRADO_USE_AMBAR_EMERGENCIA` |
| ~~`CMD:PIN:1234:FORZAR_ROJO`~~ | `:176` | 🛑 mismo `$ERR`. **Las dos formas** |
| ~~`CMD:PIN:1234:TEST_LEDS`~~ | `:202` | 🛑 `$ERR,…,DESC:NO_EN_SERVICIO_USE_EL_MAESTRO` |

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

## 6. 📊 EVIDENCIA DE PANTALLAS (GALERÍA V9.0)

> ⚠️ **Una captura a un solo ancho no es una prueba de interfaz.** Las de `evidencia/` se hicieron a
> **412 px**, y el corte del botón de la derecha **solo aparece por debajo**: 11 px a 390, **41 px a
> 360** y **81 px a 320**. Un `.png` demuestra que a **ese** ancho se veía bien, y nada más. Al medir
> una pantalla nueva se miden **los cuatro anchos**.

| Vista | Captura | Descripción |
|---|---|---|
| **Modo Operario** | `evidencia/01_modo_operario_principal.png` | Botonera táctil de 4 botones y réplica 3D de semáforos. ~~*«réplica del mando de relés»*~~ → **31/08: es la superficie de mando principal**, no una réplica de nada |
| **Selector de Cruces** | `evidencia/02_modal_cruces_abierto.png` | Catálogo de frentes de obra y selección inmediata |
| **Modo Técnico** | `evidencia/06_modo_tecnico_activo.png` | Desbloqueo por PIN y pestañas de configuración |
| **Ajustes de Tiempos** | `evidencia/07_tiempos_guardados_exito.png` | Programación de tiempos de verde, rojo y despeje |
| **Courier RTC** | `evidencia/08_courier_rtc_inyectado.png` | Inyección de hora compensada en el nodo Esclavo |

---
**Desarrollado y Validado para el Proyecto Controladora_Semaforos V9.0**
