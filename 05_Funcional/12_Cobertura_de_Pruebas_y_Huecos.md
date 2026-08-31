# 🧪 COBERTURA DE PRUEBAS Y HUECOS — V9.0

**Fecha:** 26 de Agosto de 2026
**Última revisión:** 31 de agosto de 2026
**Método:** censo automático de la superficie de entrada del firmware cruzado contra ~~los 20 packs~~
**los packs del banco**. **No es una opinión: cada fila se levantó con una búsqueda sobre el código**,
y el comando que la produce está anotado para que cualquiera la repita.

> ## ⚠️ 31/08 — ESTE DOCUMENTO YA NO PUBLICA CIFRAS DEL BANCO. REMITE AL ACTA
>
> Las ediciones anteriores decían *«los 20 packs · `155/155`»*. **Hoy son bastantes más y la cuenta
> se mueve cada hora**, así que escribirla aquí a mano garantiza que envejezca en silencio — que es
> exactamente lo que este documento existe para evitar.
>
> **La cifra vigente se lee del acta más reciente de `evidencia/`** —`evidencia/<AAAA-MM-DD>_compuerta.txt`—,
> que la escribe `01_Firmware/compuerta.py` con la fecha y el hash de `HEAD` encima. **Hay dos packs
> que vigilan que las cifras publicadas salgan del acta**, no de la memoria de nadie.
>
> ```
> python 01_Firmware/compuerta.py                       # 0 PASS | 1 FALLA | 2 ABORTADO
> python 01_Firmware/Simulaciones/banco/correr.py --listar
> ```
>
> 🔴 **Y el acta se lee entera, no su primer número.** `ABORTADO` **no es** `PASS`: una comprobación
> que no pudo correr **no dice nada del firmware**. Un `x/y` con `x != y` significa que hay
> comprobaciones que no cumplen, lo escriba como lo escriba el instrumento y salga con el código que
> salga. **Y el acta del 31/08 no está en verde.**
>
> **Nada de esto es un permiso.** En campo corre la **V8.4**, y **verde no es entregable**: la
> compuerta dice que los modelos y los arneses de PC no encuentran nada, **no que el firmware funcione
> en la tarjeta**.

---

## 0. Por qué existe este documento

Los defectos de V9.0 —cámaras en pines de botones, PIN sin validar, radio sin direccionar, Esclavo
que mueve luces por Bluetooth, app incapaz de hablar SPP— **se encontraron uno a uno, preguntando**.
Ninguno salió de una revisión sistemática. Un método que depende de que a alguien se le ocurra la
pregunta correcta **garantiza que quedan defectos**: solo aparecen los que se preguntaron.

Este documento es la pasada que faltaba. Y su primer resultado es un defecto que nadie había visto.

---

## 1. ✅ CERRADO (31/08) — `CAM_UMBRAL_PIN` ya no existe, y las cámaras están en otro sitio

> **El hallazgo era correcto y se ha resuelto por la raíz: `PB8` nunca fue una entrada de cámara.**
> Se conserva íntegro debajo, porque el patrón que describe es reutilizable y volverá a aparecer.

**MEDIDO el 31/08 — el censo que lo cierra:**

```
$ grep -rn "CAM_UMBRAL_PIN" --include=*.cpp --include=*.h Maestro Esclavo
   (sin resultados)

$ grep -rn "PB8" Maestro/include/pines.h Esclavo/include/pines.h
   pines.h:63:  #define LED_TESTIGO   PB8   // -> R16 1K -> LED D5. NO es entrada de camara

$ grep -rn "CAM_C_PIN\|CAM_D_PIN" Maestro/include/pines.h
   pines.h:124:  #define CAM_C_PIN   PB14   // J16 p10 - camara de contacto seco
   pines.h:125:  #define CAM_D_PIN   PB15   // J16 p12 - camara de contacto seco
```

| | 26/08 | **31/08** |
|---|---|---|
| `PB8` | *«cámara de umbral»* con `pinMode()` y sin `digitalRead()` | **`LED_TESTIGO`** — LED `D5` por `R16` de 1 kΩ. **No es entrada** (N-64) |
| Cámaras `C` y `D` | no existían | 🟢 **`CAM_C_PIN` = `PB14`, `CAM_D_PIN` = `PB15`**, `INPUT` pelado y **activo en ALTO** |
| Los cuatro documentos que decían *«Umbral de tramo»* | contradecían al fuente | **caducados**: si alguno lo sigue diciendo, está mal |

⚠️ **Lo que NO está cerrado:** el firmware declara los pines, pero **no se cablea cámara a `J16`
hasta la medida `M3`** —con `INPUT` pelado el pin necesita resistencia real a masa en la placa o
queda flotando y el ruido dispara demandas fantasma; de `PB14`/`PB15` **solo lo dice el netlist**—.
Y el orden es **firmware cargado en la tarjeta primero, cableado después**: un commit no protege de
un destornillador.

### 📕 El hallazgo original, tal como se escribió el 26/08 — se conserva

~~```
$ grep -rn "CAM_UMBRAL_PIN" --include=*.cpp Maestro/src Esclavo/src
Maestro/src/modo_inteligente.cpp:29:  pinMode(CAM_UMBRAL_PIN, INPUT_PULLUP);
Esclavo/src/main.cpp:275:            pinMode(CAM_UMBRAL_PIN, INPUT_PULLUP);
```~~

~~**Eso es todo.** El pin se configura como entrada y **no se lee nunca**: no hay un solo
`digitalRead(CAM_UMBRAL_PIN)` en ninguno de los dos microcontroladores.~~

~~Mientras tanto, **cuatro documentos afirman lo contrario**: el Manual 9 de cámaras, el Manual 10 de
Bluetooth, `ESTADO.md` y `roadmap.md` describen `PB8` como *"Umbral de tramo"* con función activa.~~

> **Un `pinMode()` sin `digitalRead()` es exactamente el patrón de la prueba muerta que este
> repositorio ya conoce:** algo que *parece* conectado, que compila, que pasa la compuerta, y que no
> hace nada. Se descubrió censando, no leyendo el manual — porque el manual decía que sí funcionaba.
>
> **Esta frase se conserva sin tachar: es la lección, y no ha caducado.**

---

## 2. Superficie de entrada completa, cruzada con el banco

Todo lo que puede meter información al sistema, y qué lo prueba:

> ⚠️ **Tabla revisada el 31/08.** Las columnas de recuento se retiran: **el número de comprobaciones
> de cada pack se lee del acta**, no de aquí. Lo que esta tabla dice es **qué existe y qué no**.

| # | Entrada | Canal | Qué acepta | Pack que lo cubre | Estado |
|---|---|---|---|---|---|
| 1 | ~~4~~ **2** botones locales | ~~`PB9` `PB13` `PB14` `PB15`~~ **`PB9` `PB13`** | secuencias A/B | `maestro_01_mando`, `esclavo_02_inhibicion_menu` | ✅ |
| 2 | Mando de ~~4~~ **2** relés | **los mismos 2 pines** | idem | idem | ✅ |
| 3 | Cámara de demanda | `PB0` | binario con antirrebote | 🟢 **`camara_01_demanda`** | ✅ *(era ❌)* |
| 4 | ~~Cámara de umbral (`PB8`)~~ | — | **no existe: `PB8` es `LED_TESTIGO`** | — | ✅ **cerrado, §1** |
| 4.bis | 🆕 **Cámaras `C`/`D` en `J16`** | `PB14` `PB15` | contacto seco, **activo en ALTO** | 🟢 **`camara_02_j16`** | ✅ |
| 5 | Radio LoRa | `USART3` | comandos `CMD_*` | `costura_03_comandos` | ⚠️ parcial |
| 6 | Bluetooth Maestro | `USART1` **por `J17`** (`PB6`/`PB7`) | **17 formas** | 🟢 **`app_01_comandos`, `app_02_modos_simetricos`, `app_03_sin_ok_mudo`, `app_04_valores_de_status`, `app_06_formato_de_hora`** | ✅ *(era ❌)* |
| 7 | Bluetooth Esclavo | `USART1` **por `J17`** | 4 aceptados + 3 rechazados | 🟢 **`esclavo_06_no_abre_paso`** + los `app_0x` | ✅ *(era ❌)* |
| 8 | Respaldo en pila | BKP RAM | firma + checksum | `maestro_02_respaldo` | ✅ |
| 9 | Reloj / pila | RTC | hora, validez | `maestro_04_sync_horaria`, `esclavo_05_hora_atomica` | ✅ |
| 10 | Corte de energía | — | reanudación | `costura_06_reanudacion` | ✅ |
| 11 | 🆕 **Reloj `DS3231` del ESP32** | I²C `GPIO21`/`GPIO22` | hora, bit `OSF` | ⚠️ **por censar** — el firmware existe (`ESP32_Expansion/src/reloj_ds3231.cpp`) y **la compuerta lo compila** | ⚠️ |

~~**El patrón salta a la vista: todo lo anterior a V9.0 está cubierto; todo lo que V9.0 añadió, no.**
Los 20 packs suman `155/155` — exactamente los mismos que antes del Bluetooth, las cámaras y los dos
comandos de radio nuevos.~~

> ## ✅ 31/08 — EL PATRÓN SE HA INVERTIDO, Y ERA EL PUNTO DE ESTE DOCUMENTO
>
> Las cuatro filas que el 26/08 decían **«ninguno»** —cámara de demanda, Bluetooth del Maestro,
> Bluetooth del Esclavo y el pin de umbral— **hoy tienen pack**, y hay packs nuevos que en aquella
> pasada ni se listaron. **La cifra de comprobaciones no se copia aquí**: se lee del acta.
>
> ⚠️ **Y un pack que existe no es un pack que mida.** Antes de contar uno como cobertura hay que
> haberle roto el firmware a propósito y haberlo visto **bajar la cuenta y cambiar el código de
> salida**. Un arnés que nadie ha visto fallar es un adorno que da verde.

### 2.bis Los comandos de radio, uno a uno

`protocolo.h` define 18. `costura_03_comandos.py` no se toca desde la Fase 2:

```
CMD_GO_GREEN  CMD_GO_RED  CMD_ACK_GREEN  CMD_PING  CMD_PONG  CMD_ACK_RED
CMD_HORA_D  CMD_HORA_H  CMD_HORA_M  CMD_HORA_S  CMD_ACK_HORA
CMD_DELTA  CMD_DELTA_RESP  CMD_CONFIG_VERDE  CMD_CONFIG_DESPEJE  CMD_ACK_CONFIG
CMD_DEMANDA  CMD_ACK_DEMANDA          <- los dos nuevos, sin cubrir
```

---

## 3. Invariantes de seguridad, y quién intenta romperlos

Un invariante sin una prueba que **intente violarlo** es una intención, no una garantía.

| Invariante | ¿Quién lo intenta romper? *(revisado 31/08)* |
|---|---|
| Ningún pin de luz se escribe fuera de `semaforo.cpp` | `barrera_01_pines_de_luz` ✅ — ⚠️ ver el matiz de abajo |
| Nunca verde y rojo a la vez en la misma cabeza (SFTY-2) | `Validacion_Automatico`, sobre pines escritos ✅ |
| ~~**Nunca verde simultáneo en las DOS puntas** — 🔴 nadie~~ | 🟢 **`barrera_02_dos_puntas`** *(era el hueco más caro del documento)* |
| El despeje nunca baja del configurado | ⚠️ indirecto en `costura_02` |
| No se reanuda el Degradado sin autorización válida | `costura_06`, `maestro_03_puerta_degradado` ✅ |
| Un equipo sin hora no autoriza el Degradado | `maestro_04`, `esclavo_05` ✅ |
| ~~**Un comando de Bluetooth no abre paso sin PIN** — 🔴 nadie~~ | 🟢 **`app_01_comandos`, `app_02_modos_simetricos`, `app_03_sin_ok_mudo`** y **`esclavo_06_no_abre_paso`** |
| 🆕 **Un `$ACK` no promete lo que la llamada no devolvió** | 🟢 **`app_03_sin_ok_mudo`** — encontró **dos ramas** que ninguna revisión humana vio |
| 🆕 **La talanquera** | 🟢 `barrera_03_talanquera` |
| **Un equipo no obedece a una pareja ajena** | 🔴 **nadie** — y hoy no hay ni direccionamiento |

~~La tercera fila es la que mata, y es la que este repositorio ya sabe que no mide.~~

> ## ✅ 31/08 — quedan DOS avisos, y no son el mismo que había
>
> **1. El direccionamiento de pareja sigue sin nadie.** Es hoy el único `🔴` de la tabla.
>
> **2. 🔴 `barrera_01_pines_de_luz` mide menos de lo que su nombre promete.** La regla enumera **ocho**
> pines de luz; `escribirPines()` mueve **seis**. `ROJO_PEATON` (`PA6`), `VERDE_PEATON` (`PA7`) y el
> `BUZZER` (`PB1`) están **declarados en `pines.h` y muertos en las dos puntas** —sin `pinMode`, sin
> `digitalRead`, sin `digitalWrite`—, así que la regla era **vacuamente cierta** para dos de sus ocho
> sujetos. El pack **no podía detectarlo**: solo mide *fugas hacia fuera*, acepta `len(luces) >= 6`
> sobre una lista que devuelve 8, y su `control_negativo` **nunca ejerció un pin peatonal**.
>
> > **Una regla de seguridad que enumera sujetos tiene que comprobar que cada sujeto existe**, no solo
> > que nadie la rodea. Es la misma familia que `CAM_UMBRAL_PIN` de la §1: un `✅` en la columna de la
> > izquierda **no dice qué mide el pack**, solo que hay uno.

---

## 4. Plan de pruebas — qué escribir, y en qué orden

Cada pack se conecta a `compuerta.py` **solo después de haberlo visto fallar** con un defecto
inyectado a propósito en el `.cpp` real (`CLAUDE.md §8.bis`).

| # | Pack | Qué debe romper | Control negativo obligatorio | **31/08** |
|---|---|---|---|---|
| 1 | ~~`bluetooth_01_autorizacion`~~ | que un comando sin PIN válido abra paso | trama sin PIN → rechazada | ✅ **hecho** como `app_01`/`app_02`/`app_03` |
| 2 | ~~`bluetooth_02_esclavo_no_abre`~~ | que el Esclavo ejecute algo que encienda un verde | `TEST_LEDS` en servicio → rechazado | ✅ **hecho** como `esclavo_06_no_abre_paso` |
| 3 | `costura_08_pareja` | que un equipo atienda una trama de otra pareja | `PAIR` ajeno → descartada | 🔴 **sigue pendiente** |
| 4 | ~~`camara_01_demanda`~~ | antirrebote y que la demanda no acorte el despeje | demanda continua → el despeje **no** baja | ✅ **hecho** — y hay además `camara_02_j16` |
| 5 | `costura_03` (ampliar) | que `0x11`/`0x12` cuadren entre las dos puntas | *(el total se lee del acta)* | ⚠️ por confirmar |
| 6 | ~~`barrera_02_dos_puntas`~~ | **verde simultáneo en las dos puntas** | forzar verde en ambos → debe saltar | ✅ **hecho** |

~~El **6** es el que falta desde siempre y el más caro.~~ **El 6 está escrito.** Lo que queda de este
plan es **el 3** —direccionamiento de pareja— y **confirmar el 5**.

> ⚠️ **Y la §5 del control negativo del punto 5 se corrige:** *«totales `7 → 9`»* era una cifra
> escrita a mano. **Un control negativo se expresa como una propiedad, no como un número que este
> documento tendría que ir persiguiendo**; el recuento vive en el acta.

> ## 🔴 Lo que esta lista NO cubre y hay que añadirle (31/08)
>
> | Qué vigilar | Por qué |
> |---|---|
> | **Que `mando_ambarLocal()` siga teniendo armador** | El veto de SFTY-21 no se rompe borrando un `if`: se rompe borrando **quien pone la bandera a `true`**. Los tres consumidores quedan siempre-verdaderos y **ningún test falla**. Hoy tiene dos armadores —`B·B·B` y `CMD:AMBAR_EMERGENCIA`—; el pack debe exigir que **al menos uno** exista |
> | **Que los pines enumerados por una regla EXISTAN** | Ver §3: `barrera_01` acepta `>= 6` sobre una lista de 8 |
> | **Que los literales que un pack busca por texto sigan donde los busca** | Un refactor puede **apagar un instrumento sin romper un solo test**: si los `"$ACK`/`"$ERR` se mudan de fichero, `app_03` pasa a decir *«ninguna rama promete nada»* y **queda en verde midiendo nada** |
> | **El módulo de expansión ESP32** | Compila en la compuerta, pero su superficie —el despachador, el puente y el `DS3231`— **no está en esta tabla**. Un instrumento que no está en la compuerta no mide nada, **y un hueco no deja rastro de que falta** |

---

## 5. Qué NO cubre este documento

- ~~**La app móvil no tiene ninguna prueba**, de ningún tipo. Ni unitaria, ni de integración.~~
  ✅ **Superado el 31/08:** la app tiene hoy **cuatro instrumentos en la compuerta** —simulador de app
  y Bluetooth, test funcional, test unitarios y **la app ejecutada en DOM**—, más los packs `app_0x`
  que la cruzan contra el firmware. **Sus cifras se leen del acta.**

  > 🔴 **Pero el aviso que sustituye a aquel es más incómodo:** esos instrumentos ya estuvieron
  > **los dos en `ABORTADO` a la vez** —un pack buscaba una rama en `app.js` cuando el parser se
  > había mudado a `js/`, y el arnés de DOM reventaba con un `TypeError` sobre una pestaña que ya no
  > existía—, y eran justo **los dos únicos que ejercen la app**. Detrás entraron cuatro defectos.
  > **Mientras un instrumento está abortado, todo lo que vigilaba entra sin mirar**: un `ABORTADO` no
  > se apunta para luego, se arregla antes de mirar nada más.

- 🆕 **Este documento no mide la interfaz.** El contraste WCAG y el comportamiento a distintos anchos
  tienen su propio arnés; y **una captura a un solo ancho no es una prueba de interfaz** — las de
  `evidencia/` se hicieron a 412 px, el único de cuatro donde cierto corte no aparecía.

- **Nada de esto sustituye la prueba de banco.** Un pack demuestra que el modelo y el código
  coinciden; no que la tarjeta encienda una bombilla. **En campo corre la V8.4.**


---

## 6. ✅ CERRADO (31/08) — ~~`PB0`/`PB8` están asignados dos veces~~ · **el reloj se fue del STM32**

> # 🟢 LA DISPUTA POR LOS PINES YA NO EXISTE, Y NO SE RESOLVIÓ CON UN EXPANSOR
>
> **Toda esta sección se sostenía sobre una premisa que hoy es falsa:** que el `DS3231` tenía que
> colgar de `PB0`/`PB8` del STM32. **Ya no.**
>
> | | 26/08 | **31/08 — MEDIDO** |
> |---|---|---|
> | Dónde vive el `DS3231` | *«en `PB0`/`PB8` del STM32, hacen falta DOS»* | **en el módulo de expansión ESP32**, `GPIO21` (`SDA`) / `GPIO22` (`SCL`), **con pila propia** |
> | Driver | *«no hay»* | 🟢 **existe y compila**: `01_Firmware/ESP32_Expansion/src/reloj_ds3231.cpp` |
> | `PB8` | *«cámara de umbral»* | **`LED_TESTIGO`** — LED `D5` por `R16` 1 kΩ. **No es entrada** |
> | `PB0` | disputado | **`CAM_DEMANDA_PIN`, sin disputa** |
> | Cámaras `C`/`D` | sin pines | **`PB14`/`PB15`** (`J16` p10/p12), liberados por el mando |
>
> **La cuenta que bloqueaba —«1 + 2 = 3 > 2»— ya no se plantea:** el reloj no pide pines del STM32 y
> las cámaras tienen tres entradas (`PB0`, `PB14`, `PB15`).
>
> ### 🛑 Y por tanto la propuesta del `PCF8574` QUEDA RETIRADA
>
> No porque fuera mala —resolvía bien el problema que había—, sino porque **el problema desapareció**.
> Montarlo hoy añadiría un segundo esclavo a un bus I²C por software **sin necesidad**, y con él el
> riesgo que la propia sección anotaba: *«hay que comprobar que la latencia de leer el expansor no
> compite con el `IWDG` ni con el bit-bang del LCD»*. **Ese riesgo ya no hace falta correrlo.**
>
> ### 📌 Lo que de esta sección SIGUE EN PIE
>
> - **`Y2` está muerto** (N-37, banco del 01/08). El STM32 no puede fiarse de su cristal, y por eso
>   el reloj bueno vive ahora en el ESP32. **No es un plan: el firmware existe.**
> - **Sin reloj no hay Modo Degradado**, y SFTY-18 lo prohíbe con razón.
> - **La lección de método, que es lo que no caduca:** un documento fechado **antes** de una medida
>   de banco no contradice esa medida — **está sin actualizar**, y hay que corregirlo antes de que
>   alguien lo cite como si valiera. Es exactamente lo que le acaba de pasar a esta sección.
>
> **Lo que sigue se conserva sin borrar, como histórico del razonamiento.**

---

### 📕 HISTÓRICO — el hallazgo del 26/08, superado

Esta pasada cruzó **todos** los documentos que nombran `PB0` o `PB8`. El resultado no es una errata:

| Documento | Dice que `PB0`/`PB8` son… | Fecha |
|---|---|---|
| `roadmap.md` **N-37** | **`SDA`/`SCL` del `DS3231`** — *"los únicos pines libres"* | **cerrado en banco el 01/08/2026** |
| Manual 11 · `MANUAL_INSTALACION_RELOJ_DS3231.md` | `SDA`/`SCL` del `DS3231` | V9.0 |
| Manual 9 · Manual 2 · `MAPEO:179` · `ESTADO.md` | **cámaras IA** (demanda y umbral) | V9.0 |

**No caben las dos cosas.** Y la que llegó primero tiene evidencia de banco detrás.

### Lo que dice N-37, y por qué no es negociable

> *"**CERRADO POR ELIMINACIÓN: el cristal `Y2` está MUERTO. Banco del 01/08/2026.**"*

Con tres eliminaciones medidas —`VBAT` a 3 V con la tarjeta apagada, el reintento de `N-25`, y
`REINICIAR RELOJ` devolviendo `SIGUE PARADO`—. Y su conclusión: *"Ya no queda software que probar.
**Salida: `DS3231` por I²C software en `PB0`/`PB8`.** Hacen falta **DOS**"*.

**El `MAPEO §4` dice lo contrario** —*"que el cristal sea el culpable no está medido"*— pero su
cabecera lo fecha: **«Última revisión: 31 de julio de 2026»**, un día **antes** de la medida de
banco. No es una contradicción: es un documento sin actualizar, y hay que corregirlo antes de que
alguien lo cite como si valiera.

### Por qué esto invalida la arquitectura de V9.0

1. El firmware sigue en `STM32RTC` sobre el cristal muerto (`reloj.cpp:109`, `LSE_CLOCK`). **No hay
   driver `DS3231`.**
2. V9.0 acaba de darle a las cámaras **los dos pines que el reloj necesita**.
3. Sin reloj no hay Modo Degradado: `SFTY-18` lo prohíbe, y con razón.

### Y la salida que aparece al mirarlo entero

De los dos pines de cámara, **`PB8` no se lee nunca** (§1). O sea que la demanda real necesita **un
solo pin**, no dos. Y aun así 1 + 2 = 3 > 2.

**Un expansor `PCF8574` en el mismo bus I²C lo resuelve:** cuelga de `PB0`/`PB8` junto al `DS3231`,
cuesta céntimos, y aporta **8 entradas digitales** para las cámaras — con lo que sobran seis.

```
   PB0 (SDA) ──┬─────────────┬───────────────
   PB8 (SCL) ──┴──┐       ┌──┴──┐
                  │       │     │
              [DS3231]  [PCF8574]  ← 8 entradas: camaras 1..4 y margen
```

**Ventaja adicional que no es menor:** deja de haber pines libres en disputa. Cualquier entrada
futura entra por el expansor sin volver a abrir esta discusión.

**Riesgo a medir, no a suponer:** el I²C es por software y ahora tendría dos esclavos; hay que
comprobar que la latencia de leer el expansor no compite con el `IWDG` ni con el bit-bang del LCD.
Eso se mide en banco, no aquí.
