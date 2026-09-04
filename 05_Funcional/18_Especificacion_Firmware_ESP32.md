# 18 — Especificación del firmware del ESP32 de expansión

**Fecha de la pasada: 31/08/2026.** `HEAD` en el momento de medir: `cc4ba61`.

Este documento existe porque el firmware del ESP32 **no existe**: ni como código, ni como
especificación. El Manual 17 §C lo deja escrito con todas las letras —*«El firmware del ESP32: **no
existe**»*— y aun así la arquitectura del 28/08 ya reparte funciones sobre él. Lo que sigue es
contra lo que se programa.

> 📌 **Actualización del 31/08, más tarde el mismo día. El párrafo de arriba se queda escrito porque
> era cierto cuando se escribió, y tachar el porqué de un documento hace que nadie entienda su forma.
> Lo que ya no es cierto va aquí, MEDIDO:**
>
> | | estado a 31/08 | MEDIDO en |
> |---|---|---|
> | El firmware del ESP32 de expansión | 🟢 **existe y compila** — `35.6 %` de flash (`1121001` de `3145728` B) | `evidencia/2026-08-31_compuerta.txt:16` · árbol en `01_Firmware/ESP32_Expansion/` (8 `.h` + 8 `.cpp`) |
> | El rol nuevo en la compuerta (§7.1) | 🟢 **dado de alta** | `01_Firmware/compuerta.py:114` `_ROLES = (..., "ESP32_Expansion")`, `:118` la regex, `:97` `RUTAS_MINIMAS_ESPERADAS = 45`, `:692` `compilar("esp32", "ESP32_Expansion")` |
> | Los nueve packs de §7.2 | 🟢 **existen los nueve** | `01_Firmware/Simulaciones/banco/packs/esp32_01…esp32_09` |
> | `BLQ-1` | 🟢 **CERRADO** — ver §6.1 | `ESTADO.md:23` · `roadmap.md:215` (N-107) |
>
> 🛑 **Y lo que NO cambia: nada de esto ha pasado banco, y no hay una sola tarjeta con un ESP32
> conectado a `J17`.** Que compile y que los packs den verde es exactamente lo que `CLAUDE.md` §3
> dice que **no** es un entregable.

> 🛑 **Esto no es un permiso, y no se convierte en uno por estar escrito.**
>
> Nada de lo que hay aquí ha pasado banco. **No hay una sola tarjeta con un ESP32 conectado a
> `J17`.** No hay `DS3231` comprado (`A6`), no hay fuente propia pedida (`A5`), y **no se sabe qué
> chip llegó a obra** (§8.1, `BLQ-1`). Un verde de `compuerta.py` tampoco autoriza nada de esto:
> ese `0` significa que *los modelos y los arneses de PC no encuentran nada*, y **ninguno de ellos
> toca la tarjeta**. `CLAUDE.md` §3: **verde no es entregable.**

---

## 0. Cómo se lee este documento

Tres niveles, la misma escala del Manual 17 §0 y de `MAPEO_TARJETA_KICAD.md` §0. **No se mezclan
nunca.**

| marca | qué significa |
|---|---|
| **MEDIDO** | se abrió el fichero y se leyó, en esta pasada. Va con `fichero:línea`. Se puede repetir |
| **ESCRITO** | lo afirma un documento, un comentario o un acta. Puede ser cierto; **no se comprobó aquí** |
| **SIN VERIFICAR** | nadie lo ha comprobado, ni aquí ni en ningún sitio |

Todo lo que este documento llama **MEDIDO** se midió **sobre ficheros**. Un fichero dice lo que
alguien escribió; una placa dice lo que se fabricó.

### 0.bis Dos avisos sobre el buscador, pagados en esta misma pasada

`CLAUDE.md` §4: *un «no aparece» no es un hallazgo hasta haber descartado al buscador.* Se cobró
dos veces mientras se escribía esto, y las dos van anotadas porque el siguiente agente va a usar
los mismos comandos:

1. **Una lectura del fichero no es una medida.** La primera lectura de
   `Esclavo/src/bluetooth.cpp:78` se transcribió como `SerialBT.begin(115200)` y estuvo a punto de
   publicarse como *«corrección: las dos puntas van a velocidades distintas»*. `grep`, `sed -n
   '78p'` y `od -c` sobre la misma línea dan **`9600`**. Manda la medida, no la lectura:

   ```
   $ sed -n '78p' Esclavo/src/bluetooth.cpp | od -c
   0000000       S   e   r   i   a   l   B   T   .   b   e   g   i   n
   0000020   (   9   6   0   0   )   ;  \n
   ```

2. **`grep -i "OSF"` casa dentro de `MOSFET`.** El censo de *«¿existe ya algo de `DS3231`?»* devolvió
   diez líneas de `pines.h` que no tenían nada que ver. Es el mismo error de forma que el `(segment `
   del `.kicad_pcb`: el buscador respondía y aun así no sabía encontrar. Con `\bOSF\b` y acotando a
   `src/` e `include/` —fuera `.pio/libdeps`, que trae `Wire.h` vendorizado de U8g2— el resultado es
   **cero**, y ése es el dato bueno.

---

## 1. Qué es el ESP32 aquí, y qué no

### 1.1 Lo que está decidido y no se reabre en este documento

| | |
|---|---|
| **El STM32 es el controlador.** El ESP32 **no manda sobre las luces** | `CLAUDE.md` §6 no cambia: solo `semaforo.cpp` escribe pines de luz |
| El ESP32 es **un accesorio colgado de un puerto serie** | si desaparece, el semáforo sigue ciclando |
| Lleva **dos funciones**: puente Bluetooth (sustituye al módulo SPP) y reloj `DS3231` por I²C | Manual 17 §1.3 |
| Enlace por **`J17`**, `GPIO17`→`p2`(`PB7`) y `GPIO16`←`p3`(`PB6`), GND en p7 o p9 | Manual 17 §1.4 |
| **Fuente propia desde 12 V.** No cuelga del 3,3 V de `J17` p6/p8 | Manual 17 §1.5 · línea `A5` de compras, **no pedida** |
| El reloj **no puede ir en el STM32**: no quedan pines | `PB0` = `CAM_DEMANDA_PIN`, `PB8` = `LED_TESTIGO` |

**MEDIDO** — los dos pines que cierran la puerta al reloj en el STM32:

```
01_Firmware/Maestro/include/pines.h:46   #define CAM_DEMANDA_PIN    PB0
01_Firmware/Maestro/include/pines.h:63   #define LED_TESTIGO        PB8   // -> R16 1K -> LED D5
```

`pines.h:59-62` deja escrito además que `PB8` se queda **en alta impedancia a propósito**, y
`pines.h:64-67` enumera los cuatro pines que de verdad quedan libres (`PA11`, `PA12`, `PA15`,
`PC13`) — **ninguno de ellos es un par I²C por hardware**. La línea `PIN-0` está anulada.

### 1.2 La frase que gobierna todo el diseño

> **El ESP32 es la superficie de mando, y no es parte del lazo de seguridad.**

Las dos mitades importan por separado:

- **Es la superficie de mando** porque el Manual 17 §1.6 retira la pantalla y ~~los cuatro
  pulsadores y el mando de relés~~ → **sólo `BOTON3` (`PB14`) y `BOTON4` (`PB15`)**, que son los que
  las cámaras necesitan.
  > 🔵 **CORREGIDO EL 04/09: esta frase llevaba desde el 31/08 diciendo lo contrario que el
  > Manual 17 §1.6**, que **ese mismo día** decidió **CONSERVAR el mando de relés** en los canales
  > `A` (`PB9`) y `B` (`PB13`). Era deriva entre documentos, de la misma clase que las cinco de la
  > segunda pasada.
  >
  > **Lo que NO cambia es la conclusión de este apartado**, y por eso el diseño no se toca: después
  > de la retirada, *toda la operación normal* pasa por la app, y la app pasa por el ESP32. Un ESP32
  > colgado deja el equipo **seguro pero no operable** (§17 3.3).
  >
  > **Lo que cambia es su tamaño:** queda una superficie física de último recurso —`A·A·A`, `B·B·B`
  > y `A·B·A·B`— **en el Maestro y sólo en el Maestro** (§17 2.7: el receptor del Esclavo no se ha
  > comprado). 🔴 **Y no está demostrada:** el mando **no se pudo pulsar** en el banco del
  > 3-4/09 (N-118), el fuente se corrigió el 04/09 y **no se ha cargado en ninguna tarjeta**. Un
  > accesorio no puede apoyarse en una salida de emergencia que nadie ha visto funcionar.
- **No es parte del lazo de seguridad** porque el ciclo, el enclavamiento SFTY-2, el todo-rojo y la
  caída a ámbar de SFTY-6 viven enteros en el STM32 y no leen ni un byte del ESP32.

De ahí sale la regla de rechazo de este documento: **cualquier propuesta que haga que el semáforo
dependa del ESP32 para seguir siendo seguro se rechaza**, por cómoda que sea. Eso incluye —y se
nombra porque ya está propuesta en el Manual 17 §3.2 vía B— el reloj de software en el STM32
disciplinado por el ESP32.

---

## 2. El enlace físico

### 2.1 Pin a pin

| ESP32 | dirección | `J17` | STM32 | pin de `U1` |
|---|---|---|---|---|
| `GPIO17` (TX2) | ⟶ | **p2** | `PB7` — **RX** del micro | 43 |
| `GPIO16` (RX2) | ⟵ | **p3** | `PB6` — **TX** del micro | 42 |
| `GND` | — | p7 **o** p9 | `GND` | — |

**MEDIDO** en el firmware, las dos puntas:

```
01_Firmware/Maestro/src/bluetooth.cpp:28   static HardwareSerial SerialBT(PB7, PB6);
01_Firmware/Esclavo/src/bluetooth.cpp:26   static HardwareSerial SerialBT(PB7, PB6);
```

> ⚠️ **El primer argumento es RX y el segundo es TX** (firma del framework
> `HardwareSerial(rx, tx)`). Es decir: el micro **recibe** por `PB7` y **transmite** por `PB6`. El
> comentario del propio fuente lo confirma —`// USART1 remapeado: PB7 RX, PB6 TX`—, pero el orden
> se invierte solo al leerlo rápido, y un cruce de estos hilos no da error: da silencio.
>
> 📌 **Corrección de deriva:** el Manual 17 §1.4 cita `Maestro/src/bluetooth.cpp:25`. Hoy es **`:28`**.
> El fichero se ha movido tres líneas desde el 28/08. La cita del Esclavo (`:26`) sigue exacta.

### 2.2 Masa común, obligatoria — y la medida que va antes

**MEDIDO** que `J17` no reparte 12 V (Manual 17 §A `M2`), pero la masa común **no es opcional**:
sin ella, la diferencia entre las dos masas entra entera por `PB6`/`PB7`, que son patas del micro
que gobierna el semáforo.

La secuencia es la de `M5` del Manual 17 y **no se salta**: con las dos fuentes encendidas y los
hilos de datos **todavía sin unir**, se mide masa contra masa (**< 50 mV**) y `GPIO17` contra la
masa común (**3,3 V**, reposo alto de una línea serie). Si `GPIO17` en reposo diera **5 V**, el
módulo no es el que se cree y se para antes de conectar.

### 2.3 El nombre del pin 3 sigue en disputa

**ESCRITO** y sin cerrar (Manual 17 §1.4, `MAPEO_TARJETA_KICAD.md` §6.bis, `pines.h:77-84`): la
etiqueta de red del esquemático llama al pin 3 `RS(A0)`; el firmware lo llama `LCD_PSB`. **Los dos
nombres no pueden ser ciertos a la vez.** Con la LCD retirada la duda ya no amenaza a la pantalla,
pero el hilo del ESP32 va a un sitio con dos nombres. Se cierra siguiendo el hilo hasta la pata
rotulada, no leyendo más código. **Dueño: quien monte.** Ver §9 `AB-6`.

---

## 3. El contrato de bytes

Esta sección es el corazón ejecutable del documento. **Todo lo de aquí está MEDIDO** sobre
`01_Firmware/Maestro/src/bluetooth.cpp` y `01_Firmware/Esclavo/src/bluetooth.cpp`, leídos enteros
el 31/08/2026.

### 3.1 Transporte

| | valor | dónde está MEDIDO |
|---|---|---|
| Velocidad | **9600 bps** | `Maestro:70` · `Esclavo:78` — `SerialBT.begin(9600)` |
| Formato | **8N1** | **por defecto del framework**: nadie lo eligió — ver el aviso |
| Periférico | `USART1` remapeado a `PB6`/`PB7` | `Maestro:28` · `Esclavo:26` |
| Caudal efectivo | **960 B/s** (10 bits por byte con arranque y parada) | cuenta |

> ⚠️ **Las dos puntas van a 9600, y hay que decir que se comprobó.** El encargo de esta pasada las
> daba a 9600 y **lo son**; se anota porque una primera lectura dijo `115200` en el Esclavo y era
> falsa (§0.bis). Si algún día divergen, el mismo ESP32 no puede servir a las dos sin recompilar.
>
> ⚠️ **`8N1` no es una decisión, es un valor por defecto que nadie escribió.** `SerialBT.begin(9600)`
> se llama con un solo argumento; los 8 bits, sin paridad y un bit de parada los pone el framework.
> **El ESP32 tiene que abrir `Serial2` con `SERIAL_8N1` explícito**, y ese literal es lo único que
> ata las dos puntas: en el STM32 la elección es implícita y no se puede leer de ningún sitio.

### 3.2 La trama de ENTRADA al STM32 (lo que el ESP32 escribe hacia `PB7`)

**MEDIDO** — el bucle receptor, idéntico en las dos puntas (`Maestro:389-400`, `Esclavo:295-306`):

```c
while (SerialBT.available() > 0) {
  char c = (char)SerialBT.read();
  if (c == '\n' || c == '\r') {           // Maestro:391  Esclavo:297
    if (btIdxIn > 0) {
      btBufIn[btIdxIn] = '\0';
      procesarComando(btBufIn);
      btIdxIn = 0;
    }
  } else if (btIdxIn < sizeof(btBufIn) - 1) {   // Maestro:397  Esclavo:303
    btBufIn[btIdxIn++] = c;
  }
}
```

De ahí salen **cuatro reglas duras**, y ninguna es negociable desde el ESP32:

| # | regla | consecuencia si se incumple |
|---|---|---|
| **E-1** | **Terminador obligatorio: `\r` o `\n`.** Cualquiera de los dos vale, y los dos juntos también (el segundo cae con `btIdxIn == 0` y no hace nada) | **sin terminador el despachador no dispara NUNCA.** El comando se queda en el buffer, mudo |
| **E-2** | **63 caracteres útiles como máximo.** `btBufIn[64]` (`Maestro:31`, `Esclavo:29`) con la guarda `btIdxIn < sizeof(btBufIn) - 1` | 🔴 **el exceso se descarta EN SILENCIO**, y lo que llega al despachador es una **línea truncada** que se compara como si estuviera completa |
| **E-3** | **Una línea vacía no es nada.** `if (btIdxIn > 0)` descarta el terminador suelto | inofensivo, pero el ESP32 no debe contarlo como comando entregado |
| **E-4** | **El STM32 NO valida el checksum de entrada** | ver §3.4 |

> 🔴 **E-2 es un límite duro y hay que escribirlo en la especificación del puente, no descubrirlo en
> banco.** **El ESP32 tiene que medir la longitud antes de reenviar y rechazar con `$ERR` propio lo
> que no quepa** — porque el STM32, si se lo mandan, no protesta: trunca y compara.
>
> ⚠️ ~~*«El comando útil más largo que la app compone hoy es `CMD:PIN:1234:SET_RTC:2026-08-31,23:59:59`
> = **41 caracteres**, con 22 de margen»*~~ — **REFUTADA el 31/08. La cadena está bien, la cuenta no,
> y encima no era la cadena que la app componía.** Se deja tachada y no borrada: `CLAUDE.md` §4, *una
> causa que se cae se marca refutada; la que desaparece en silencio vuelve a proponerse.*

**La cuenta de E-2, y de dónde sale cada número. MEDIDO el 31/08 sobre el emisor vivo:**

El emisor es `enviarComandoFirmware()` — `05_Funcional/App_Semaforo/app.js:205-221` —, y la línea que
compone es, literal del fuente:

```js
rawCmd = `CMD:PIN:${pin}:${comando}:${args}\r\n`;   // app.js:218
rawCmd = 'CMD:' + comando + '\r\n';                 // app.js:216, los cuatro SIN_PIN
```

Los caracteres **útiles** son los que llegan al `btBufIn` del STM32: **sin** el `\r\n`, que E-1
consume como terminador y nunca se guarda. Sobre esa definición:

| comando compuesto | útiles | margen sobre 63 | de dónde sale |
|---|---|---|---|
| `CMD:PIN:1234:SET_RTC:2026-08-31,23:59:59` | **40** | **23** | hora en `HH:MM:SS` de 24 h — **es el formato que la app compone HOY**, `horaLocal24()`, `app.js:289-292` |
| `CMD:PIN:1234:SET_RTC:2026-08-31,6:25:00 p. m.` | **45** | **18** | 🔴 el que componía **antes** del arreglo del 31/08: `toLocaleTimeString()` en el locale de campo `es-CO` daba `"6:25:00 p. m."` (`app.js:264-281`) |
| `CMD:PIN:1234:SET_TIEMPOS:255,255,255` | 36 | 27 | `app.js:1252`, con los tres campos en su valor más largo |
| `CMD:PIN:1234:SET_MODO:INTELIGENTE` | 33 | 30 | el `SET_MODO` de literal más largo |
| `CMD:AMBAR_EMERGENCIA` | 20 | 43 | rama sin PIN, `app.js:216` |

> 🔴 **El número que se publica es el de hoy, y hoy vale 40 con 23 de margen. Pero lo que gobierna
> E-2 no es el número: es el CÁLCULO, y por eso se escribe entero.** La cifra se movió **cinco
> caracteres** en una sola tarde sin que nadie tocara el protocolo ni el firmware — la movió el
> **formato de la hora dentro de la app**, y la va a volver a mover cualquier campo que crezca.
> Publicar sólo el número es publicar una foto de un día. Quien necesite la cifra la **recalcula**
> con la tabla de arriba.
>
> ⚠️ **Y el margen no es una garantía en ninguno de los dos casos: es una casualidad de los literales
> de hoy.** El caso de 45 demuestra la forma del riesgo mejor que el de 40: **no hizo falta un
> comando nuevo para comerse 5 de los 23 caracteres de margen, bastó una llamada de JavaScript que
> depende del idioma del teléfono.** Por eso el tope se mide en el puente y no se confía al margen.

### 3.3 La trama de SALIDA del STM32 (lo que el ESP32 lee de `PB6`)

**MEDIDO** — `enviarTramaConCrc()`, `Maestro:43-48`, `Esclavo:51-56`:

```c
static void enviarTramaConCrc(const char* payload) {
  uint8_t crc = calcularChecksum(payload + 1);   // Salta el '$' inicial
  char tramaCompleta[140];
  snprintf(tramaCompleta, sizeof(tramaCompleta), "%s*%02X\r\n", payload, crc);
  SerialBT.print(tramaCompleta);
}
```

Forma de la trama:

```
$<payload>*<XOR8 en 2 hex mayúsculas>\r\n
```

| # | regla | MEDIDO en |
|---|---|---|
| **S-1** | Empieza por `$`, termina por `\r\n`. **Siempre los dos bytes** | `Maestro:46` · `Esclavo:54` |
| **S-2** | El checksum es **XOR-8 sobre el payload SALTANDO el `$` inicial**, y **parando en el `*`** | `calcularChecksum()`, `Maestro:34-41` · `Esclavo:42-49` |
| **S-3** | Se imprime en **una sola llamada** a `print()` — el STM32 no parte tramas por su cuenta | `Maestro:47` · `Esclavo:55` |
| **S-4** | Tamaño máximo en el cable: **132 B** | ver la cuenta abajo |

**La cuenta de S-4, MEDIDA sobre los `snprintf` reales:**

| trama | buffer del payload | tope de payload | **tope en el cable** |
|---|---|---|---|
| `$STATUS` | `payload[128]` (`Maestro:425`, `Esclavo:326`) | 127 | **132 B** |
| `$ALARM` / `$EVENT` | `payload[100]` (`Maestro:81`/`:95`, `Esclavo:89`/`:103`) | 99 | **104 B** |
| `$ERR,...DEGRADADO` | `p[80]` (`Maestro:244`) | 79 | **84 B** |
| literales `$ACK` / `$ERR` | — | el más largo medido: **61** | **66 B** |
| envoltorio | `tramaCompleta[140]` | — | cabe con holgura |

El literal más largo que emite el firmware hoy, medido:
`$ERR,CMD:REINICIAR_RELOJ,DESC:SIGUE_PARADO_VEA_CONSULTA_RELOJ` — 61 B de payload, 66 en el cable.

> ⚠️ **Un borde que el ESP32 no puede arreglar y tiene que tolerar:** `$STATUS` se compone con
> `snprintf` en `payload[128]`. Si `SERIE:`, `MODO:` y `ESTADO:` crecieran, `snprintf` **trunca el
> payload** y el CRC se calcula sobre lo truncado. La trama sale **bien formada y con checksum
> correcto**, pero **cortada a mitad de campo**. El puente no debe descartarla por eso —es válida— y
> el parser de la app tiene que sobrevivir a un último campo incompleto. Hoy el caso largo realista
> mide 119 B de payload, con 8 de margen. **Es poco margen y nadie lo vigila.** Ver §9 `AB-5`.

### 3.4 La asimetría deliberada del checksum

> 🔴 **El STM32 EMITE checksum pero NO VALIDA el de entrada.**

**MEDIDO, y esta parte sí lo estaba:** `procesarComando()` arranca directamente con
`strcmp(cmd, "CMD:FORZAR_ROJO")` (`Maestro:145`) y `strcmp(cmd, "CMD:AMBAR_EMERGENCIA")`
(`Esclavo:130`). **No hay una sola llamada a `calcularChecksum()` en el camino de recepción**, en
ninguna de las dos puntas.

---

#### 🔴 3.4.a REFUTADO — lo que esta sección decía antes, y lo que costó

> ⚠️ **Esta sección llevaba la palabra MEDIDO encima de algo que no se midió, y por eso se tacha en
> vez de borrarse.** Es el caso más puro de `CLAUDE.md` §4 segunda cara —*lo que TÚ reportas también
> es un instrumento*— que ha salido en este repositorio: una afirmación plausible, escrita con dos
> citas de línea, que llegó al documento **con la palabra «medido» encima** y era falsa **en sus dos
> extremos**.

~~*«El `*XX` que la app añade —`nmea_parser.js:20-21`, `formatearComando()`— llega al `strcmp` como
parte de la cadena y hace que el comando no case»*~~, y de ahí ~~*«el ESP32 SÍ valida: comprueba
`$`, `*` y que el XOR-8 case; si no casa, descarta y contesta `$ERR` él mismo»*~~.

**Los dos extremos son falsos. MEDIDO el 31/08, cada uno por separado:**

| # | lo que decía | lo MEDIDO | dónde |
|---|---|---|---|
| **1** | la app añade `*XX` (y `$`) | 🔴 **NO los añade.** El emisor vivo compone literalmente `CMD:PIN:1234:SET_MODO:AUTO\r\n` — sin `$` delante y sin `*XX` detrás | `enviarComandoFirmware()`, `05_Funcional/App_Semaforo/app.js:205-221`; las tres ramas de composición en `:216`, `:218`, `:220` |
| **2** | la función se llama `formatearComando()` | 🔴 **NO EXISTE.** `grep -rn "formatearComando"` sobre toda la app da **cero**. La función es **`formatearTrama()`**, y vive en **`js/nmea_parser.js:19-22`**, no en la raíz | `05_Funcional/App_Semaforo/js/nmea_parser.js:19` |
| **2.bis** | *(implícito: que ese módulo esté en el camino)* | 🔴 **`NMEAParser` no tiene un solo llamador en la app.** Está cargado por `index.html:589` y **nadie lo usa**: los únicos usos vivos están en `tests/`. Su `generarComando()` (`:129-135`) sí produce `$CMD:…*XX`, y con el `$` delante **no casaría con ningún `strcmp`** — pero esa cadena **nunca sale al aire** | `index.html:589` · `js/nmea_parser.js:129-135` |

🔴 **Lo que costó, y va escrito porque es la lección y no la anécdota:** el agente que escribió el
firmware del ESP32 **siguió esta sección al pie de la letra en su primera versión**. Un puente que
exige `$` y checksum a lo que llega de la app **habría descartado el 100 % de los comandos reales**
—ninguno trae `$` ni `*`— y habría contestado un `$ERR` propio a cada uno. Y el equipo, a un
comando que sí llega, contesta `AUTH_FAILED,PIN_INVALIDO`: el motivo manda a depurar el PIN, que no
tiene **nada** que ver. Es la prueba muerta de N-51 del revés: **un instrumento que no aprueba nada
válido**, con documentación encima.

**Es también la forma exacta del corolario de §4:** dos citas con número de línea **parecen** una
medida. Una cita a `nmea_parser.js:20-21` sólo demuestra que alguien abrió ese fichero; **no**
demuestra que ese código corra, y aquí no corre. *Un «no aparece» no es un hallazgo hasta haber
descartado al buscador* tiene su gemelo: **un «sí aparece» no es un hallazgo hasta haber comprobado
que alguien lo llama.**

---

#### 3.4.b Lo que el puente hace HOY, en positivo

**MEDIDO** sobre `01_Firmware/ESP32_Expansion/src/puente.cpp`, que ya existe y compila:

| dirección | ¿hay checksum en el cable? | qué hace el puente |
|---|---|---|
| **app ⟶ STM32** | 🔴 **NO.** La app no lo pone (medida 1 de arriba) | **Los bytes se reenvían VERBATIM**: los mismos que mandó la app, sin quitar nada y **sin añadir nada**, con su `\r\n`. La **única** comprobación de este sentido es el **tope de longitud** (E-2): lo que no cabe se descarta y **se dice**, con un `$ERR` marcado como del puente (`NODE:PUENTE`) |
| **STM32 ⟶ app** | ✅ **SÍ.** Lo pone `enviarTramaConCrc()` (`Maestro:43-48`, `Esclavo:51-56`) | **El puente lo verifica** antes de retransmitir. Una trama con CRC malo es ruido de cable, y el ruido no sube al teléfono. Se cuenta lo descartado (P-3) |

> 🔴 **No se puede validar el checksum de lo que llega de la app, porque la app no lo pone.** No es
> una relajación de SFTY-16 ni una comodidad: **es que no hay nada que validar**, y exigirlo sería
> rechazar el 100 % del tráfico real. La asimetría de esta tabla no es un descuido — es la medida.
>
> ⚠️ **Y por el mismo motivo el puente TAMPOCO añade checksum al salir hacia el STM32.** El STM32
> compara la línea **entera** con `strcmp`: un `*4F` pegado detrás haría que no casara ningún
> comando y **todos** caerían en `$ERR,CMD:DESCONOCIDO`. Es el defecto refutado de §3.4.a,
> reintroducido por el otro lado.

> ~~🟢 **El dato que esta pasada saca de propina:** en el sentido **STM32 ⟶ app**, el puente es **el
> primero de toda la cadena que comprueba el checksum**. La app **no** lo comprueba… **hoy no hay un
> solo checksum verificado en toda la cadena, en ninguna dirección**. El ESP32 está **estrenándola**.~~
>
> 🔴 **REFUTADO el 01/09, y en sus tres extremos. MEDIDO:** la app **sí** comprueba el checksum de
> bajada desde el 31/08. `parseNmeaTelemetry()` empieza por `juzgarTrama(line)`, que llama a
> `NMEAParser.validarTrama()` y **vuelve sin pintar** si no casa; `line.split('*')` **ya no existe en
> el código** —sólo en dos comentarios que describen lo retirado—.
>
> En el sentido **STM32 ⟶ app hay DOS validadores del mismo XOR-8**, el del puente y el de la app, y
> **juzgan igual**: siete casos frontera con veredicto idéntico. El ESP32 **no estrena** la
> comprobación: la **duplica**.
>
> 🔴 **Y lo que sí sigue siendo cierto es la otra mitad, que es la que importa: EN LA SUBIDA NO HAY
> CHECKSUM EN NINGÚN SITIO.** La app no firma, el puente no valida (deliberado, apartado 3.4.b) y
> **ninguna de las dos puntas llama a `calcularChecksum()` en recepción** —está definida y sólo se usa
> al emitir, medido en las dos—. Consecuencia: **un bit cambiado DENTRO del parámetro de
> `SET_TIEMPOS:` o `SET_RTC:` sigue casando con el `strncmp` del prefijo, y el equipo obedece con los
> valores mutilados.** El agujero está razonado en 3.4.b; **que exista sólo ahí no estaba escrito en
> ningún sitio.**

Lo que sí se mantiene de la formulación anterior, porque es correcto y es el principio de diseño:
el puente **valida FORMATO, no comandos**, igual que el firmware del Repetidor en la otra topología
(`05_Funcional/5_Manual_Puente_ESP32.md` §3 — 🔴 **otro equipo, otro firmware: ver el recuadro de
ese manual antes de mezclarlos**). La razón está escrita allí y vale igual aquí: *un puente que
conociera la lista de comandos habría que recompilarlo cada vez que el protocolo crece, y el día
que alguien olvidara hacerlo la función nueva se caería en silencio.*

> ⚠️ **Y el corolario que hay que escribir para que nadie lo «mejore»:** validar el CRC en el ESP32
> **no** convierte al enlace en autenticado. El STM32 sigue aceptando cualquier línea que le
> llegue por `PB7`. Quien pinche un hilo en `J17` p2 manda comandos sin CRC y el equipo obedece.
> El ESP32 es una **puerta**, no una cerradura.

### 3.5 La autenticación es un literal en claro

**MEDIDO**, las dos puntas:

```
01_Firmware/Maestro/src/bluetooth.cpp:166   strncmp(cmd, "CMD:PIN:1234:", 13) == 0
01_Firmware/Esclavo/src/bluetooth.cpp:164   strncmp(cmd, "CMD:PIN:1234:", 13) != 0
```

**El PIN es `1234`, va escrito en el fuente, y viaja en claro por el enlace serie y por el aire
Bluetooth.** Se escribe tal cual, sin maquillarlo:

- **El ESP32 lo TRANSPORTA. No lo mejora, no lo sustituye, no lo almacena y no lo compara.** Un
  puente que validara el PIN sería una segunda copia del contrato que alguien tendría que
  sincronizar — y el día que difieran, un comando funcionaría por una puerta y sería rechazado por
  la otra. Es el mismo razonamiento que el propio `bluetooth.cpp:161-164` da para no partir la
  cadena de comparaciones.
- **Es una limitación conocida, no un defecto que este documento arregle.** Cambiar el esquema de
  autenticación es una decisión con dueño (§9 `AB-7`), toca las dos puntas y la app, y no cabe en
  la especificación de un puente.
- Lo que sí se anota, porque hoy no está escrito en ningún sitio: **tres órdenes del Maestro y una
  del Esclavo entran SIN PIN**, y es deliberado (`Maestro:136-144` y `:154-164`, `Esclavo:110-129`).
  Son `CMD:FORZAR_ROJO`, `CMD:SET_MODO:MENU`, `CMD:SET_MODO:ALCANCE` y `CMD:AMBAR_EMERGENCIA`. El
  puente **no puede exigir PIN a lo que el firmware exime**: si lo hiciera, la caída segura pedida
  delante de un accidente necesitaría recordar una clave.

### 3.6 Censo completo de los comandos que el STM32 atiende hoy

El puente tiene que dejarlos pasar **íntegros**. Se listan para que nadie los deduzca.

#### Maestro — `01_Firmware/Maestro/src/bluetooth.cpp`

| línea | comando (tal como llega) | PIN |
|---|---|---|
| `:145` | `CMD:FORZAR_ROJO` | **no** |
| `:166` | prefijo `CMD:PIN:1234:` → el resto es la *acción* | — |
| `:168-170` | `CMD:SET_MODO:MENU` · `CMD:SET_MODO:ALCANCE` (sin prefijo de PIN) | **no** |
| `:177` | `SET_MODO:AUTO` | sí |
| `:182` | `SET_MODO:MANUAL` | sí |
| `:187` | `SET_MODO:AMBAR` | sí |
| `:191` | `SET_MODO:MENU` | sí (o sin PIN por `:169`) |
| `:212` | `SET_MODO:ALCANCE` | sí (o sin PIN por `:170`) |
| `:223` | `SET_MODO:INTELIGENTE` | sí |
| `:234` | `SET_MODO:DEGRADADO` | sí |
| `:253` | `FORZAR_ROJO` | sí |
| `:257` | `MANUAL:CAMBIAR_TURNO` | sí |
| `:271` | `TEST_LEDS` | sí |
| `:275` | `SET_TIEMPOS:<v>,<r>,<d>` | sí |
| `:295` | `SET_RTC:YYYY-MM-DD,HH:MM:SS` | sí |
| `:330` | `REINICIAR_RELOJ` | sí |
| `:345` | `DEMANDA` | sí |
| `:362` | *(cualquier otra)* → `$ERR,CMD:DESCONOCIDO` | — |

#### Esclavo — `01_Firmware/Esclavo/src/bluetooth.cpp`

| línea | comando | PIN | nota |
|---|---|---|---|
| `:130` | `CMD:AMBAR_EMERGENCIA` | **no** | |
| `:157` | `CMD:FORZAR_ROJO` | **no** | 🔴 **rechazado a propósito**: `$ERR,...DESC:RENOMBRADO_USE_AMBAR_EMERGENCIA` (N-83) |
| `:164` | prefijo `CMD:PIN:1234:` | — | |
| `:171` | `AMBAR_EMERGENCIA` | sí | |
| `:176` | `FORZAR_ROJO` | sí | rechazado igual que `:157` |
| `:184` | `SOLICITAR_PASO` | sí | el Esclavo **pide**, no ordena (SFTY-27) |
| `:202` | `TEST_LEDS` | sí | 🔴 **rechazado a propósito**: encendería un verde en esta punta |
| `:215` | `SET_RTC:...` | sí | |
| `:259` | *(cualquier otra)* → `$ERR,CMD:DESCONOCIDO,...EN_ESCLAVO` | — | |

> 🔴 **Las dos puntas NO atienden la misma lista, y el puente no puede unificarlas.** `TEST_LEDS`
> se acepta en el Maestro (`:271`) y se rechaza con motivo en el Esclavo (`:202`). `FORZAR_ROJO`
> hace rojo de verdad en el Maestro y está renombrado en el Esclavo. Un puente que «normalizara»
> comandos borraría diferencias que son **deliberadas y de seguridad**. El puente transporta bytes.

> 🟠 **`CMD:AMBAR_EMERGENCIA` (`:130` y `:171`) va a dejar de tener UNA respuesta — 31/08/2026.**
>
> El censo de arriba lista lo que el STM32 **atiende**, y eso no cambia. Lo que cambia es lo que
> **contesta**: por decisión del responsable del 31/08 ese comando pasa a salir del Modo Degradado de
> forma ordenada, y entonces **la misma orden tiene cinco respuestas distintas según el estado del
> equipo** —`RESULT:OK`, `RESULT:YA_EN_AMBAR_LATCH_PUESTO`, `RESULT:SALIENDO_TODO_ROJO`, una fila aún
> por decidir y el `$ERR,CMD:AUTH_FAILED` de siempre—.
>
> **La tabla está especificada en un solo sitio y este documento NO la copia:**
> `05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.md` **§4.5**. **Nada de ello ha pasado banco, y
> el firmware de hoy contesta `RESULT:OK` en los cinco casos.**
>
> **Lo que sí es asunto del puente, y son tres cosas:**
>
> 1. **No colapsar respuestas.** Un puente que tradujera cualquier `$ACK,CMD:AMBAR_EMERGENCIA,…` a un
>    «OK» de su propia cosecha borraría justo la distinción que esa tabla existe para crear: el
>    técnico volvería a irse del poste con una confirmación de algo que tarda hasta 90 s en ocurrir.
>    Es §6.2 —**el puente NO ORIGINA**— aplicado a la respuesta y no sólo al comando.
> 2. **No inventar el `RESULT:` que falta.** Si el STM32 no contesta, el puente **no** compone un
>    `$ACK`. Es §6.4 literal: **silencio no es orden**, y tampoco es confirmación.
> 3. **Los literales de `RESULT:` crecen, y el puente no los enumera.** Cualquier tabla de respuestas
>    escrita dentro del ESP32 sería una segunda copia que alguien tendría que sincronizar. El puente
>    **relaya la trama verbatim**, checksum incluido, y quien la interpreta es la app.

### 3.7 Censo completo de las tramas que el STM32 emite

| prefijo | quién la compone | MEDIDO en |
|---|---|---|
| **`$STATUS`** | telemetría periódica, ~~**cada 1000 ms**~~ → **cada 2000 ms** *(bajada el 04/09 por decisión del responsable, en las **dos** puntas del STM32)* | `Maestro:403`, `:427` · `Esclavo:309`, `:328` |
| **`$ACK`** | confirmación de un comando | ~~32 literales en el Maestro~~ → **MEDIDO el 31/08: `grep -c '"$ACK' ` da **17** en el Maestro y **4** en el Esclavo.** El 32 no era el recuento de ninguna de las dos columnas *(se parece a los 33 de sumar `$ACK` + `$ERR` del Maestro, pero eso es otra cosa)* |
| **`$ERR`** | rechazo con motivo | **MEDIDO el 31/08: 16 en el Maestro, 8 en el Esclavo.** ~~«idem, misma cadena»~~ — no son las mismas cifras y la fila lo daba por hecho |
| **`$ALARM`** | `bluetooth_reportarAlarma()` | `Maestro:82` · `Esclavo:90` |
| **`$EVENT`** | `bluetooth_reportarEvento()` — bitácora de quién movió qué | `Maestro:96` · `Esclavo:104` |

> 📌 **Corrección al encargo: son CINCO prefijos, no cuatro.** `$EVENT` faltaba en la lista de
> partida y **no es marginal**. Un puente que filtrara por una lista de cuatro prefijos **se comería
> la bitácora entera** — y sería exactamente la clase de pérdida silenciosa que costó N-73: un
> registro que cuatro documentos describen y que nadie puede mirar cuando hay que diagnosticar un
> fallo de campo.
>
> ⚠️ **Y las dos cifras con que se justificaba estaban mal las dos. Re-MEDIDO el 31/08, segunda
> pasada:**
>
> | | decía | MEDIDO | cómo |
> |---|---|---|---|
> | ramas del despachador del Maestro que emiten `$EVENT` | ~~14~~ | **16** | `grep -n "bluetooth_reportarEvento" Maestro/src/bluetooth.cpp` → 17 líneas, de las que `:87` es **la definición** y no una llamada. Las 16 llamadas van de `:148` a `:355` |
> | dónde lo consume la app | ~~`app.js:814-815`~~ | **`app.js:976-977`** | `:814` no habla de `$EVENT`; la rama real es `else if (header === '$EVENT')` con el comentario *«`$EVENT` es la bitacora del propio equipo -quien movio que y desde donde-»* |
>
> **La conclusión no cambia y por eso el error es instructivo:** las dos cifras estaban mal, las dos
> apuntaban en la misma dirección que la conclusión, y **ninguna de las dos se habría notado nunca**
> porque el lector que está de acuerdo no va a comprobarlas. Es §4 otra vez — *un número con
> `fichero:línea` al lado **parece** medido*, y contar a ojo un `grep` que incluye la definición
> junto a las llamadas es exactamente el fallo de N-73 al revés.

**MEDIDO en la segunda pasada, para que no haya que volver a contarlo:**

```
$ grep -c '"$ACK'  Maestro/src/bluetooth.cpp  ->  17      Esclavo -> 4
$ grep -c '"$ERR'  Maestro/src/bluetooth.cpp  ->  16      Esclavo -> 8
$ grep -n "bluetooth_reportarEvento" Maestro/src/bluetooth.cpp | wc -l  ->  17
                                              (:87 es la definicion; 16 son llamadas)
```

**Plantillas literales, MEDIDAS:**

```
Maestro:82   $ALARM,NODE:MAESTRO,EVENTO:%s,CAUSA:%s,ACCION:%s,HORA:%s
Maestro:96   $EVENT,NODE:MAESTRO,ORIGEN:%s,DETALLE:%s,HORA:%s
Maestro:427  $STATUS,NODE:MAESTRO,SERIE:%s,MODO:%s,ESTADO:%s,T:%lu,RF:%d%%,RTT:%lums,BAT:12.6,HORA:%s
Esclavo:328  $STATUS,NODE:ESCLAVO,SERIE:%s,MODO:SUBORDINADO,ESTADO:%s,T:%lu,RF:98%%,RTT:85ms,BAT:12.6,HORA:%s
```

*(Que `RF:98%`, `RTT:85ms` y `BAT:12.6` sean literales fijos en el Esclavo es telemetría fabricada,
ya levantada en el Manual 17 §2.6. **No es cosa del puente** y no se arregla aquí; se anota para que
nadie crea que el ESP32 los mide.)*

> 🔵 **Y un acoplamiento que hay que dejar escrito, porque desde el 04/09 algo cuelga de él:
> `$STATUS` es la única trama que lleva `SERIE:` y `NODE:` a la vez, y de ahí —y sólo de ahí— sale el
> rótulo Bluetooth del módulo** (§6.5, `src/puente.cpp:205-206`). Es una dependencia **de un campo,
> no de un formato**: quien acorte un campo de `$STATUS` para ganar los 8 B de margen de `AB-5` tiene
> que dejar `SERIE:` y `NODE:` en pie, o el puente deja de aprender su nombre **en silencio** y todos
> los módulos se quedan en `SEM-SIN-MATRICULA`.

### 3.8 El presupuesto de bytes por segundo

A **9600 8N1 = 960 B/s**. La cuenta, con los tamaños medidos de §3.3:

| tráfico | tamaño | ocupación del canal |
|---|---|---|
| ~~`$STATUS` 1/s, caso largo realista~~ → **`$STATUS` 1 cada 2 s** | ~~124 B/s~~ → **62 B/s** | ~~12,9 %~~ → **6,5 %** |
| ~~`$STATUS` 1/s, tope duro~~ → **`$STATUS` 1 cada 2 s, tope duro** | ~~132 B/s~~ → **66 B/s** | ~~13,8 %~~ → **6,9 %** |
| ráfaga `$ACK` + `$EVENT` + `$STATUS` juntos | 340 B | **354 ms de cable** — **NO cambia con la cadencia** |
| comando entrante, tope `E-2` (63 + terminador) | 64 B | **67 ms de cable** — **NO cambia con la cadencia** |

> 🔴 **La bajada a 2000 ms NO divide por dos la ocupación, y esto hay que decirlo porque ya se dijo
> mal.** Se afirmó que dejaría el cable *«por debajo del 30 %»*. **Es falso, y está medido: el peor
> segundo pasa de `528 B` (`55,0 %`) a `462 B` (`48,1 %`) de los 960 B/s.** Sólo el `$STATUS`
> **periódico** se parte por dos; el `$EVENT`, el `$ALARM` y el `$ACK` que caen en ese mismo segundo
> **no escalan con la cadencia** — la cuenta anterior daba por hecho que la ráfaga escalaba, y no
> escala. Las dos primeras filas de la tabla son la parte periódica; **el número que decide es el
> peor segundo, y es `48,1 %`.**

**Lo que esto obliga:**

| # | regla | por qué |
|---|---|---|
| **P-1** | El ESP32 **no puede añadir tráfico periódico propio** hacia el STM32 | el enlace tiene margen, pero el margen es del equipo, no del accesorio |
| **P-2** | Buffer de salida hacia la app de **al menos 512 B** | una ráfaga de 354 ms coincidiendo con un `$STATUS` no puede descartar tramas |
| **P-3** | **Ni un byte se descarta en silencio.** Si un buffer se llena, el ESP32 lo **cuenta y lo dice** | es E-2 otra vez: lo que se descarta callando se lee como que nunca existió |
| **P-4** | 🔴 **El puente NO reduce la cadencia de `$STATUS` ni «agrupa» telemetría** | la app declara enlace perdido a los 5 s sin trama (§4.2). Un puente que agrupe dos `$STATUS` para ahorrar aire hace que la app declare caído a un equipo sano |

---

## 4. Función 1 — El watchdog. **VA PRIMERO**

> ## ✅ 01/09/2026 — YA ESTÁ CONSTRUIDO. Este apartado dejó de ser una especificación
>
> El watchdog del ESP32 **existe, compila y está en la compuerta**. Lo que sigue se conserva porque
> es el porqué y los requisitos que se le exigieron, pero **léalo en pasado**: ya no describe lo que
> hay que hacer, describe lo que se hizo.
>
> **MEDIDO el 02/09**, en `01_Firmware/ESP32_Expansion/` (proyecto PlatformIO real, `esp32dev`):
>
> | | |
> |---|---|
> | qué watchdog | **Task Watchdog Timer del IDF**, con *panic* — **reinicia**, no sólo avisa (`src/vigilante.cpp:24`) |
> | periodo | **2 s** — `ESP32_WDT_MS 2000UL` (`include/contrato.h:102`) |
> | dónde se arma | `setup()`, **antes** del SPP y del I²C (`src/main.cpp:58`) — requisito `W-5` |
> | qué tarea vigila | la que bombea bytes (`esp_task_wdt_add(NULL)`, `src/vigilante.cpp:40`) — `W-1` |
> | dónde se alimenta | una vez por vuelta del `loop()`, al final (`src/main.cpp:89`) |
> | fuera del bucle interior | ✅ `W-3` respetado; el `while` interior está topado en `PUENTE_MAX_ITER 64` |
>
> Lo vigilan dos packs del banco: `esp32_01_watchdog_desigualdad` y `esp32_02_watchdog_alimentado`.
>
> 🟠 **Lo que sigue SIN VERIFICAR, y no se disimula:** `ESP32_ARRANQUE_MS` vale **1500 ms** y lleva su
> propio marcador de que **nadie lo ha medido** (`contrato.h:120-121`,
> `ESP32_ARRANQUE_MEDIDO 0`). El margen de la desigualdad de §4.2 se apoya en ese número: **es un
> supuesto, no una medida**. Es `AB-3`, y se cierra con el módulo en la mano.
>
> 🔴 **Y lo que un watchdog NO resuelve sigue igual de abierto:** rescata al ESP32 **colgado**; no
> hace nada por uno **muerto o desalimentado**, y **SFTY-6 no lo ve** porque mira la radio, no `J17`.
> Es `AB-1`, y es del responsable.

Va primero porque las otras dos funciones cuelgan de que el ESP32 siga vivo, y cuando se escribió
esto **nada garantizaba** que lo estuviera.

### 4.1 Lo MEDIDO

```
01_Firmware/Maestro/src/main.cpp:53    IWatchdog.begin(4000000);   <- 4 s
01_Firmware/Maestro/src/main.cpp:130   IWatchdog.reload();
01_Firmware/Esclavo/src/main.cpp:238   IWatchdog.begin(4000000);   <- 4 s
01_Firmware/Esclavo/src/main.cpp:318   IWatchdog.reload();

01_Firmware/Repetidor/src/   grep -rniE "watchdog|esp_task_wdt|WDT"  ->  CERO coincidencias
```

**Los dos STM32 tienen watchdog a 4 s con refresco en el bucle.** ~~El ESP32 de este proyecto no
tiene ninguno.~~

> ⚠️ **CADUCADO EL 01/09 — y hay que distinguir DOS ESP32, que no son el mismo.**
>
> | | watchdog |
> |---|---|
> | **`01_Firmware/Repetidor/`** — el ESP32 del repetidor | ❌ **sigue sin tener.** `grep -rniE "watchdog\|esp_task_wdt\|WDT"` sobre esa carpeta da **cero** |
> | **`01_Firmware/ESP32_Expansion/`** — el puente de `J17`, el de esta especificación | ✅ **SÍ tiene**, TWDT a 2 s (`src/vigilante.cpp:24`) |
>
> La frase de `OPTIMIZACIONES.md:55` habla del **Repetidor** y sigue siendo cierta para él. **No es
> cierta para el puente**, que es el sujeto de este documento. Confundir los dos es fácil y aquí ya
> pasó: se conserva la distinción escrita.

> 📌 **Corrección de deriva:** el Manual 17 §3.3 cita `Maestro/src/main.cpp:52`. Hoy es **`:53`**.

### 4.2 🔴 La desigualdad — y la corrección que hay que hacerle al enunciado de partida

El encargo de esta pasada pedía escribir esta desigualdad:

```
   periodo del watchdog del ESP32  <  SFTY6_SILENCIO_MS = 25000 ms
```

**MEDIDO:** la constante existe y vale eso, en las dos puntas:

```
01_Firmware/Maestro/include/protocolo.h:149   #define SFTY6_SILENCIO_MS   25000UL
01_Firmware/Esclavo/include/protocolo.h:149   #define SFTY6_SILENCIO_MS   25000UL
```

**Pero la desigualdad, tal cual, tiene la forma correcta y el par de constantes equivocado para el
rol de PUENTE.** Se corrige, y la corrección es el hallazgo más importante de esta sección.

**MEDIDO — qué vigila de verdad SFTY-6:**

```
01_Firmware/Maestro/src/coordinador.cpp:656
    bool tieneComunicacion = (tUltimaRxEsclavo > 0) && (millis() - tUltimaRxEsclavo <= SFTY6_SILENCIO_MS);

01_Firmware/Esclavo/src/main.cpp:555
    if (!degradado_gobiernaLuz() && millis() - tUltimoComando > SFTY6_SILENCIO_MS) {
```

Las dos variables —`tUltimaRxEsclavo` y `tUltimoComando`— se alimentan del **enlace de radio LoRa**
entre Maestro y Esclavo. **Ninguna de las dos lee un solo byte de `SerialBT`.**

De ahí, la consecuencia dura:

> 🔴 **En esta arquitectura el ESP32 es un puente sobre `J17` y NO está en el camino de la radio.
> Un ESP32 colgado NO dispara SFTY-6. No dispara nada. El STM32 sigue ciclando tan tranquilo y
> nadie en el equipo se entera.**
>
> Esto es **peor** que el supuesto de partida, no mejor. El enunciado original —*«si el watchdog es
> lento, el STM32 ya se fue a ámbar antes de que el puente se recupere»*— describe un equipo que
> **al menos reacciona**. El equipo real no reacciona: se queda mudo hacia el teléfono, ciclando,
> y la única señal está en la pantalla del operario.

**Por qué la desigualdad de 25 s sigue teniendo que escribirse igual, con su motivo correcto:**

El precedente del 31/07/2026 es de un ESP32 **que sí estaba en el camino de la radio** —el rol
Repetidor—, y allí colgarse **sí** bloquea el bus y **sí** dispara SFTY-6 a los 25 s:

```
01_Firmware/TROUBLESHOOTING.md:48   "Ocurrio en el repetidor el 31/07/2026: el ESP32 levantaba
                                     DE/RE ante cualquier byte y solo lo bajaba tras 5 ms de
                                     silencio; con ruido continuo [...] ese silencio nunca llegaba."
01_Firmware/TROUBLESHOOTING.md:55   "TX encendido fijo | DE/RE clavado o ruido continuo.
                                     Bus bloqueado en ambos sentidos"
```

Así que hay **dos** cotas, no una, y **manda la más estricta**:

| rol | qué lo vigila | cota | de dónde sale |
|---|---|---|---|
| **Repetidor** (LoRa) | SFTY-6 | `T_wdt + T_arranque < 25000 ms` | `protocolo.h:149` (C++) |
| **Puente** (`J17`/BT) | 🔴 **nada en el equipo** — solo la app | `T_wdt + T_arranque < 5000 ms` | `app.js:1359` (JS) |

**MEDIDO** — la cota del puente, que es la que gobierna:

```
05_Funcional/App_Semaforo/android/app/src/main/assets/public/app.js:1359
    const TIMEOUT_ENLACE_MS = 5000;

app.js:1413   if (Date.now() - state.ultimoStatusMs > TIMEOUT_ENLACE_MS) marcarSinEnlace();
app.js:1406   addEvent('red', 'Enlace perdido: el equipo lleva mas de ' + ...
```

**La desigualdad que va en el firmware y en el pack:**

```
   ESP32_WDT_MS + ESP32_ARRANQUE_MS  <  min( TIMEOUT_ENLACE_MS , SFTY6_SILENCIO_MS )
                                     =  min( 5000 , 25000 )  =  5000 ms
```

y su cota inferior, porque un techo desproporcionado también es un defecto —es el bidireccional de
`costura_09_presupuesto_radio`—:

```
   ESP32_WDT_MS  >  peor caso de una vuelta de bucle con el enlace saturado
```

> 🔴 **Y esto va en un pack que recalcula los cuatro números desde el fuente, NO en un comentario.**
> Es N-71 exacto: allí el umbral de silencio estaba en 12 s mientras el ciclo necesitaba 20,5 s,
> **los reintentos 4 y 5 eran código muerto**, y la relación entre los tres números vivía solo en
> prosa dentro de un comentario. *«Los comentarios no fallan cuando alguien cambia un número: se
> quedan describiendo un equipo que ya no existe, con la autoridad de una cuenta hecha.»* Aquí es
> peor, porque los cuatro números viven en **tres lenguajes distintos** —C++, C++ del ESP32 y
> JavaScript— y no hay nada que los ate.

> ⚠️ **Lo que la desigualdad NO resuelve, y va escrito al lado:** cumplir `< 5000 ms` en el lado
> **serie** no impide que la app pierda la sesión. Un ESP32 que se reinicia **tira la conexión SPP**,
> y volver a emparejar tarda bastante más de 5 s. **SIN VERIFICAR:** nadie ha medido cuánto. Lo
> honesto es que el reinicio será **visible** para el operario, y que la app tiene que reconectar
> sola y decir lo que pasó — no fingir continuidad. Ver §9 `AB-3`.

### 4.3 Dónde se alimenta el watchdog

> **Se alimenta desde la tarea que se puede colgar, no desde una que sigue viva cuando la otra
> muere.**

Un `esp_task_wdt_init()` sin su `esp_task_wdt_reset()` en el sitio correcto es `CAM_UMBRAL_PIN` con
otro nombre: un `pinMode()` sin `digitalRead()`, con documentación encima. Reglas:

| # | regla | por qué |
|---|---|---|
| **W-1** | `esp_task_wdt_add(NULL)` sobre **la tarea que bombea bytes** entre el SPP y `Serial2`, no sobre una tarea de servicio | si se registra la tarea equivocada, el watchdog vigila a un testigo que no se cuelga nunca |
| **W-2** | El `esp_task_wdt_reset()` va **una vez por vuelta del bucle exterior**, después de haber atendido las dos direcciones | |
| **W-3** | 🔴 **El reset NO va dentro del `while (disponible)` interior** | es la forma exacta del fallo del 31/07: **con ruido continuo el bucle interior nunca termina**, y un reset ahí dentro alimentaría al watchdog para siempre mientras el puente no progresa. Un watchdog que un flujo de basura mantiene contento no vigila nada |
| **W-4** | El bucle interior lleva **tope de iteraciones**, no solo condición de disponibilidad | misma razón que W-3, por el otro lado |
| **W-5** | El watchdog se arma en `setup()`, **antes** de abrir el SPP y antes de tocar el I²C | si el `DS3231` cuelga el bus I²C en el arranque, ya hay quien reinicie |

> ⚠️ **Y lo que un watchdog NO cubre, que el Manual 17 §3.3 ya deja escrito y aquí se repite porque
> es la mitad del problema:** un watchdog rescata al ESP32 **colgado**. No hace nada por un ESP32
> **muerto, desalimentado o desenchufado** — y en esa arquitectura, sin pantalla, sin pulsadores y
> sin mando, eso deja el equipo **sin ninguna superficie de mando**. La decisión sigue abierta y
> tiene dueño (§9 `AB-2`).

---

## 5. Función 2 — El reloj `DS3231`

### 5.1 Punto de partida MEDIDO

```
$ grep -rnE "DS3231|Wire\.|0x68|\bOSF\b"  Maestro/{src,include} Esclavo/{src,include} Repetidor/src
   ->  CERO coincidencias
```

*(Con `-i` y sin `\b`, este mismo comando devuelve diez líneas de `pines.h` porque `OSF` casa dentro
de `MOSFET`. Ver §0.bis: el buscador respondía y no sabía encontrar.)*

**No existe driver de `DS3231` en ninguna punta, ni código de I²C propio.** Se escribe entero.

### 5.2 Montaje

| | |
|---|---|
| Bus | I²C: **`GPIO21` SDA · `GPIO22` SCL** |
| Módulo | **`ZS-042`** — **ya trae sus pull-ups: NO se añaden** |
| Alimentación | **pila propia** en el módulo |
| Dirección | `0x68` (estándar del `DS3231`) — **SIN VERIFICAR** sobre el módulo real |

### 5.3 🔴 El bit `OSF` — el chip lo regala, y hay que cogerlo

El `DS3231` mantiene un **oscillator-stop flag** (`OSF`, bit 7 del registro de estado `0x0F`) que se
pone a `1` cuando el oscilador se ha parado en algún momento desde la última vez que alguien lo
limpió — corte de alimentación con la pila agotada, primera puesta en marcha, temperatura fuera de
rango.

> **Es exactamente la lección de SFTY-18, con el bit que el hardware ya da hecho.**
>
> `OPTIMIZACIONES.md:72` lo dice para el reloj del STM32: *«La regla de seguridad no es tener
> reloj, es saber cuándo no se tiene [...] Un reloj sin poner en hora que se cree válido es peor
> que no tener reloj: activaría la operación nocturna a deshora.»* En el STM32 eso costó inventar
> un año marcador y comprobar que sobreviviera. **El `DS3231` lo trae de fábrica y sale gratis.**

| # | regla | |
|---|---|---|
| **R-1** | El `OSF` se lee **en el arranque**, antes de publicar ninguna hora | |
| **R-2** | 🔴 **Una hora con `OSF` puesto se declara NO FIABLE**, aunque los registros traigan una fecha con pinta razonable | un `DS3231` sin pila devuelve una hora perfectamente formada y completamente falsa |
| **R-3** | El `OSF` se limpia **solo después de una escritura de hora confirmada**, nunca en el arranque «para dejarlo limpio» | limpiarlo sin poner la hora es fabricar una autorización |
| **R-4** | Se relee **periódicamente**, no solo al arrancar | la pila se puede agotar con el equipo en marcha |

### 5.4 La hora nace no fiable

> **Tiene que existir una función *«¿tengo hora?»*, y TODA ruta que use la hora la consulta primero.**

Es el equivalente de `reloj_enHora()`, que en el STM32 es la barrera de la que cuelgan SFTY-20 y
SFTY-21 y la entrada al Modo Degradado. En el ESP32:

```
   esp_reloj_enHora()  ->  false  si  (nunca se puso)  ||  (OSF == 1)  ||  (el I2C no contesta)
```

- **Nace `false`.** No hay valor por defecto optimista.
- **Los tres motivos se distinguen** en lo que se contesta: no es lo mismo *«no hay reloj»* que
  *«hay reloj y perdió la hora»* que *«el bus no responde»*. Es lo mismo que el Maestro hace con
  `SIN_CRISTAL_VEA_CONSULTA_RELOJ` frente a `FORMATO_INVALIDO` (`Maestro:308-318`).
- **Y el censo de llamadores es parte del trabajo, no un paso posterior** (N-73): una función
  *«¿tengo hora?»* declarada, documentada y **sin un solo llamador** es la Caja Negra de Alarmas
  otra vez.

### 5.5 🔴 El `$ACK` que mira lo que devolvió la llamada

> **Un `$ACK` que no depende de lo que la llamada devolvió es una mentira con formato de éxito.**

Es literalmente el defecto que se cerró el 28/08 en el STM32 (N-80, `CLAUDE.md` §6): la rama
`SET_RTC` llamaba a `reloj_ajustar()` y mandaba `RESULT:OK` **sin mirar ninguna de las dos
llamadas**, y con `Y2` muerto en hardware ése era el caso **normal**: *el técnico se iba del poste
creyendo que dejó el reloj puesto.*

**Ese defecto no se arregla mudándose de micro. Se muda con él si nadie lo escribe.**

**El molde de cómo se hace bien vive en el mismo fichero: `SET_TIEMPOS`** (`Maestro:275-294`) —
pregunta **dentro** del `if` y tiene **un `$ERR` por cada motivo de rechazo**. Un despachador se
escribe copiándolo.

**Motivos de rechazo que la rama `SET_RTC` del ESP32 tiene que saber distinguir, enumerados:**

| # | motivo | `$ERR` propuesto | cómo se detecta |
|---|---|---|---|
| 1 | La trama no casa el formato `YYYY-MM-DD,HH:MM:SS` | `DESC:FORMATO_INVALIDO` | el `sscanf` no devuelve 6 |
| 2 | Algún campo fuera de rango | `DESC:FORMATO_INVALIDO` | validación por **barrido**, §5.6 |
| 3 | **El bus I²C no contesta** — módulo ausente, mal cableado, SDA/SCL cruzados | `DESC:SIN_RELOJ_NO_RESPONDE` | `endTransmission()` devuelve ≠ 0 |
| 4 | **La escritura I²C falló a mitad** | `DESC:ESCRITURA_FALLIDA` | retorno de la escritura |
| 5 | **La relectura no coincide con lo escrito** | `DESC:NO_QUEDO_PUESTA` | §5.6, relectura |
| 6 | El `OSF` **sigue puesto** tras la escritura | `DESC:OSCILADOR_PARADO_CAMBIE_PILA` | relectura del `0x0F` |
| 7 | La hora entró pero **no se pudo propagar** al STM32 | `$ACK,...,RESULT:HORA_PUESTA_SIN_PROPAGAR` | el `$ACK` del STM32 no llegó |

> ⚠️ **El motivo 7 es un `$ACK`, no un `$ERR`, y la distinción es la que el Maestro ya hace en
> `:325`.** *«Los tres finales son distintos y el operario necesita los tres distintos: no hay con
> qué contar el tiempo; la hora no entró; la hora entró y va camino del Esclavo.»* Aquí son más de
> tres, y siguen teniendo que ser distintos.

> 🔴 **Y el aviso de §8.quater, que aplica el día que esto se construya:** cuando el reloj del
> ESP32 funcione, `reloj_hayCristal()` del Maestro y la rama `SIN_CRISTAL` del Esclavo pasarán a
> describir un equipo que ya no es el que hay. **Las pruebas que hoy exigen ese comportamiento no
> se reescriben en bloque hasta que pasen**: van una por una, y cada una acaba **borrada**,
> **invertida** o **conservada**, anotado.

### 5.6 Escritura atómica y validación por barrido

| # | regla | por qué |
|---|---|---|
| **R-5** | **La terna se escribe ATÓMICA**: o entran hora, minuto y segundo (y la fecha), o no entra nada | una escritura a medias deja el reloj en una hora que nadie pidió, y la deja pareciendo válida |
| **R-6** | **Se valida ANTES de escribir**, con la trama entera en la mano | rechazar a mitad es la escritura a medias de R-5 |
| **R-7** | 🔴 **La validación de rango es por BARRIDO, no por muestra** | comprobar «la hora» y dar por buenos los minutos es exactamente `PESOS_SUMA` (N-51): un número que parece cubrir todos los casos sin haber evaluado ninguno |
| **R-8** | **Se relee después de escribir** y se compara | es lo que hace `ajustarRelojVerificado()` (`Maestro:127-133`). Y **los segundos no se comparan**: entre escribir y releer el RTC puede haber avanzado uno — el comentario del Maestro en `:129-132` explica por qué eso es el lado seguro del error |
| **R-9** | Los rangos viven **en un sitio**, no en dos | `Maestro:277-281` da el motivo: dos copias son un contrato que alguien sincroniza, y el día que difieran una punta deja pasar lo que la otra rechaza |

---

## 6. Función 3 — El puente Bluetooth. 🟢 **DESBLOQUEADO: `BLQ-1` cerrado el 31/08**

### 6.1 `BLQ-1` — 🟢 **CERRADO el 31/08: es un `ESP32-WROOM-32` clásico, hay SPP**

> 🟢 **La respuesta llegó, y es la buena.** El módulo es un **`ESP32-WROOM-32` clásico**:
> `Xtensa LX6 dual-core` y `Bluetooth v4.2 **BR/EDR** + BLE`. `BR/EDR` es Bluetooth Clásico, o sea
> **SPP**: `createRfcommSocketToServiceRecord` abre y **la app conecta sin tocar una línea de
> transporte**. **El apartado 1 del Manual 10 NO se reabre.**
>
> **Tres confirmaciones independientes**, que es lo que lo cierra y no una sola: el nombre del
> módulo, el núcleo `LX6` —el `S3` es `LX7` y el `C3` es RISC-V— y el perfil Bluetooth declarado.
>
> **ESCRITO** en `ESTADO.md:23` y `roadmap.md:215` (**N-107**), leídos el 31/08.
>
> ⚠️ **Lo que este cierre NO cierra, y hay que decirlo porque un bloqueo resuelto contagia
> optimismo a los de al lado:** sigue **SIN VERIFICAR** el tiempo de arranque del módulo y el de
> reemparejar el SPP (`AB-3`), sigue sin comprarse el `DS3231` (`A6`) y sin pedirse la fuente
> propia (`A5`), y **no hay una sola tarjeta con un ESP32 conectado a `J17`**. Queda además una
> pregunta menor y no bloqueante: **30 o 38 pines** de la placa DevKitC, para las hembrillas
> (`roadmap.md:204`) — se resuelve con un pie de rey.

**La tabla que sigue se conserva porque es el razonamiento que hizo del chip un bloqueo, y hace
falta para entender por qué la respuesta importaba tanto.** Es §8.quater aplicado a un documento:
esto **se conserva**, no se borra — medía algo que sigue valiendo el día que alguien proponga
cambiar de módulo.

**ESCRITO** en `15_Lista_de_Compras_Hardware.md:102-104` y replicado en
`10_Manual_Modulo_Bluetooth_Telemetria.md:83-84`:

| familia | Bluetooth | ¿la app conecta? | consecuencia |
|---|---|---|---|
| **`ESP32-WROOM-32` · `-32D` · `-32E` · `-32U`** (clásico) | Clásico (BR/EDR) **+ BLE** | ✅ **sí, tal cual** — hay **SPP**, `createRfcommSocketToServiceRecord` abre | **la app conecta sin tocar una línea** |
| **`ESP32-S3` · `ESP32-C3`** | **solo BLE** | ❌ **nunca** — el socket RFCOMM **no abre** | 🛑 **hay que rehacer el transporte de la app ENTERO** |
| **`ESP32-S2`** | **ninguno** (solo WiFi) | ❌ | 🛑 idem, y encima sin radio Bluetooth |

**Por qué es un bloqueo y no un detalle:** el apartado 1 del Manual 10 está **congelado por
escrito** (`10_Manual...:26`, `:146-148`): *«Bluetooth Clásico SPP. No BLE. No Web Bluetooth. Y no
es negociable sin reabrir este apartado por escrito.»* La razón está medida y pagada:
`navigator.bluetooth` **no puede ver un SPP** —la API no existe para ese perfil— y eso ya costó una
versión entera de la app.

> **Cómo se responde, y no admite atajo:** se lee la **serigrafía del blindaje metálico** del
> módulo. `ESP32-WROOM-32E` es una respuesta; `ESP32-S3-WROOM-1` es otra. 🔴 **El rótulo del
> vendedor no distingue** (`10_Manual...:91`, `:100`).
>
> ~~**SIN VERIFICAR:** nadie ha leído la serigrafía de los módulos que llegaron a obra el 28/08~~ →
> 🟢 **RESUELTO el 31/08**, ver el recuadro de §6.1. *(La línea `A1′` de `15_Lista...:198` sigue
> marcada **🛑 BLOQUEADA** en su fichero: esa deriva está anotada en el Anexo y **no se corrige desde
> aquí**.)*
>
> **Era una comprobación de treinta segundos con el módulo en la mano, y decidía si había que
> rehacer el transporte de la app entero. Se hizo, y la respuesta fue la barata.**

**La interfaz estrecha se conserva aunque el bloqueo se haya cerrado.** La capa de transporte SPP
está aislada detrás de `abrir()` / `disponible()` / `leer()` / `escribir()`
(`01_Firmware/ESP32_Expansion/include/transporte_app.h`), y **eso no se deshace ahora que sabemos
que hay SPP**: el aislamiento se puso para que una rama BLE no obligara a reescribir el resto, y
sigue valiendo el día que alguien cambie de módulo. Retirar una barrera porque el problema que
temía no ocurrió es exactamente la clase de simplificación que este repositorio paga después.

### 6.2 El puente **NO ORIGINA**

> **No inventa comandos ni tramas. Todo lo que sale hacia el STM32 procede del buffer de entrada.**

| # | regla | |
|---|---|---|
| **B-1** | Cada byte escrito hacia `Serial2` **procede del buffer de entrada del SPP**. No hay literales de comando en el fuente del puente | |
| **B-2** | El puente **no reintenta** un comando por su cuenta | reintentar `MANUAL:CAMBIAR_TURNO` es pedir dos cambios de turno |
| **B-3** | El puente **no compone `$ACK` ni `$STATUS`** en nombre del equipo | es §3.quinquies: *lo que sustituye a un dato que no se tiene no es una simulación, es decirlo* |
| **B-4** | Los `$ERR` **propios** del puente —CRC malo, línea demasiado larga— **van marcados como suyos** y no se pueden confundir con los del equipo | un `$ERR` del puente que parezca del STM32 manda a diagnosticar el poste equivocado |

**El molde para vigilarlo es `esclavo_06_no_abre_paso`**, y en particular su decisión de método:

> *«La lista blanca se escribe a mano, y eso es deliberado. [...] si el pack leyera los comandos
> del propio fuente, un comando nuevo se aprobaría a sí mismo.»*

Igual aquí: el pack que vigila B-1 lleva **a mano** la lista de literales que el puente tiene
derecho a emitir, y cualquiera que añada uno pasa por ese fichero y justifica por qué no origina.

### 6.3 No parte ni une tramas

| # | regla | por qué |
|---|---|---|
| **B-5** | **Una trama entra entera y sale entera.** El puente no la corta en dos escrituras ni concatena dos en una | el receptor del otro lado delimita por `\r`/`\n` (E-1): una trama partida entrega dos líneas, y las dos son basura |
| **B-6** | **Se valida ANTES de retransmitir**, no después | es SFTY-16 aplicado al puente, y es lo que ya hace el Repetidor (`5_Manual_Puente_ESP32.md` §3): *«ese ruido se descarta dentro del ESP32 y nunca llega al aire»* |
| **B-7** | Una trama que **no cabe** o **no valida** se descarta **contándolo**, y el contador es legible | P-3 otra vez |

### 6.4 🔴 Silencio no es orden

> **Con el TX del ESP32 mudo, ausente o en reposo: ninguna acción.**

Un enlace serie en reposo está **en alto**, y un pin flotante también puede leerse alto. La
diferencia entre *«el puente no dice nada»* y *«el puente no está»* **no es distinguible desde el
STM32**, y por eso el STM32 no debe deducir nada de ninguna de las dos. Concretamente:

- El puente **no tiene un «modo por defecto»** que aplique al arrancar.
- El puente **no manda nada en `setup()`** hacia el STM32.
- **Un byte suelto no es un comando**: sin terminador no dispara (E-1), y eso es una propiedad del
  STM32 que el puente **no debe compensar** añadiendo terminadores por su cuenta.

---

### 6.5 🆕 (04/09) El rótulo del dispositivo — y por qué desde hoy decide algo

**Este apartado no existía, y la ausencia es el hallazgo.** El firmware del puente ya rotula desde
que se escribió, y la especificación **no lo decía en ninguna parte**: `grep -i "rotulo|SEM-"` sobre
este documento antes del 04/09 devolvía **una sola línea**, y era sobre la serigrafía del blindaje
del módulo (§6.1), que es otra cosa. Es `CLAUDE.md` §4 aplicada a un documento: **lo que el fuente
hace y la especificación calla no lo revisa nadie.**

**Lo que hace hoy el puente, MEDIDO:**

| | | MEDIDO en |
|---|---|---|
| El nombre SPP se fija **al abrir el perfil** | `spp.begin(rotulo)` | `src/transporte_app.cpp:37` |
| El rótulo del arranque sale de la **NVS** | `Preferences`, espacio `"puente"`, clave `"rotulo"` | `src/transporte_app.cpp:23-25` |
| Si no hay nada guardado, se anuncia el **provisional** | `SEM-SIN-MATRICULA` | `include/contrato.h:259` · `transporte_app.cpp:30` |
| El bueno se **aprende del `$STATUS`** que ya se retransmite | `SERIE:` + `NODE:` → `SEM-<serie>-M` / `-E` | `src/puente.cpp:205-206` · `transporte_app.cpp:81-104` |
| Y se guarda para la **SIGUIENTE arrancada** | `memoria.putString("rotulo", candidato)` | `src/transporte_app.cpp:113` |
| **No se re-rotula en caliente** | cerrar y reabrir el perfil **tira la sesión** del operario | `src/transporte_app.cpp:109-111` |
| Una escritura **por arrancada**, no por trama | `aprendido[]` corta la repetición: el equipo emite un `$STATUS` cada 2 s y una escritura por trama **se come la NVS en semanas** | `src/transporte_app.cpp:15-16`, `:102-104` |
| Un `NODE:` que no se reconoce **no se rotula a medias** | `return`, **sin valor por defecto** | `src/transporte_app.cpp:97` |
| Un campo que no está o no cabe **no se completa** | `campo()` devuelve `false`; *«un rótulo a medias es un rótulo equivocado y esto lo va a leer un técnico para decidir a qué poste se conecta»* | `src/transporte_app.cpp:64-79` |

**Por qué la letra final la decide el equipo y no una opción de compilación** —está escrito en el
fuente, `transporte_app.cpp:90-93—`: **el mismo binario sirve a las dos puntas.** Un firmware
distinto por punta sería una segunda copia que alguien tendría que sincronizar, y **el día que se
cruzaran los dos postes se llamarían igual**.

**Y encaja con las reglas de §6.2 sin romper ninguna, que es lo que hay que comprobar antes de
aceptar que el puente haga algo más:** `transporte_aprenderRotulo()` se llama **desde el camino de
retransmisión** y *observa*: no altera la trama, no decide si sube, y no origina nada hacia el STM32
(`src/puente.cpp:205-206`). **B-1 y B-5 siguen intactas.**

> 🔵 **Desde el 04/09 esto dejó de ser prolijidad de montaje.** El responsable decidió que
> **el cruce se opera desde el Maestro** (Manual 17 §3.7): no se relevan `SET_MODO` ni
> `MANUAL:CAMBIAR_TURNO` por radio, así que **el operario tiene que saber a qué poste caminar antes
> de caminar**. Lo único que se lo dice **antes de conectar** es este rótulo, en la lista de
> emparejados de Android.
>
> 🔴 **Y con ello se abre `AB-9`, que es de las que no se ven desde el PC:** dos módulos
> vírgenes anuncian **los dos** `SEM-SIN-MATRICULA`, y se diferencian sólo después de haber visto un
> `$STATUS` **y de una vuelta de energía**. Las cuatro opciones, con su coste, están en el Manual 17
> §3.8 y **no se reabren aquí**.

**SIN VERIFICAR, y es la mitad que pesa:** nadie ha visto este rótulo en la lista de emparejados de
un teléfono. **El Bluetooth no subió en el banco del 3-4/09** (N-117 / N-122, arreglados **sin
banco**), y no hay una sola tarjeta con un ESP32 conectado a `J17`.

---

## 7. Los instrumentos que tienen que existir ANTES del código

**Esto no es un apéndice: es la mitad del documento.**

> `CLAUDE.md` §3: *un instrumento que no está en la compuerta no mide nada — y no deja rastro de que
> falta.* Un `ABORTADO` al menos grita; **un hueco no.**

### 7.1 🔴 Primero: sin un rol nuevo, el fuente del ESP32 es INVISIBLE

> 🟢 **HECHO el 31/08 — y el razonamiento se queda entero, porque es el que hay que repetir el día
> que aparezca un quinto rol.** MEDIDO hoy en `01_Firmware/compuerta.py`: `:114` el `_ROLES` ya trae
> `"ESP32_Expansion"`, `:118` la regex lo incluye, `:97` el suelo subió a
> `RUTAS_MINIMAS_ESPERADAS = 45`, y `:692` compila el proyecto (`compilar("esp32",
> "ESP32_Expansion")`). Los nueve packs de §7.2 existen los nueve en
> `banco/packs/esp32_01…esp32_09`. **Lo que sigue describe cómo se hizo, no lo que falta.**

**MEDIDO cuando se escribió esta sección, y sigue siendo la estructura que hay que tocar:**

```
01_Firmware/compuerta.py:88   _ROLES = ("Maestro", "Esclavo", "Repetidor")
01_Firmware/compuerta.py:91-95  _RE_TRIPLE = re.compile(r'["\'](Maestro|Esclavo|Repetidor)["\']...')
01_Firmware/compuerta.py:128    if not any((r, carpeta, fichero) in rutas for r in _ROLES):
```

La guarda de rutas censa las tuplas `("Maestro","src","mando.cpp")` que los instrumentos declaran, y
**solo reconoce esos tres roles**. Un fuente bajo un cuarto directorio **no lo puede nombrar ningún
pack**, así que:

> 🔴 **Con el firmware del ESP32 escrito y sin rol nuevo, la compuerta saldría con `15 PASS`, exit
> `0`, y el ESP32 sin una sola comprobación detrás — SIN UNA FILA DONDE ECHARLO DE MENOS.**
>
> Y ese `0` se lee como permiso. Es la puerta abierta de §3.quater, sin siquiera el `ABORTADO` que
> grita.

**Lo que hay que tocar, y va en el mismo commit que el primer `.cpp` del ESP32:**

1. `compuerta.py:88` — añadir el rol al `_ROLES`.
2. `compuerta.py:91-95` — añadirlo a la alternativa del `_RE_TRIPLE`.
3. `compuerta.py:657` y alrededores — el bloque de compilación, si el ESP32 se compila en la
   compuerta (ver §7.4).
4. `RUTAS_MINIMAS_ESPERADAS` (`compuerta.py:87`) — **sube** con las rutas nuevas. El suelo existe
   justamente para que un censo que se queda ciego lo diga en vez de dar verde.

> ⚠️ **Y el nombre del rol se elige una vez.** `CLAUDE.md` §5: mover o renombrar un `.cpp` rompe un
> instrumento, y la guarda **no vigila contenido que se muda de fichero**. El árbol del ESP32 se
> decide antes de escribir el primer pack, no después.

### 7.2 Los packs, uno a uno, con la propiedad que vigila cada uno

Nueve packs. Cada uno vigila **una** propiedad, cada uno se lee de una sentada, y **cada uno relee
sus constantes del fuente en cada corrida — sin valor por defecto, nunca**.

| # | pack | propiedad que vigila | lee de |
|---|---|---|---|
| **1** | `esp32_01_watchdog_desigualdad` | 🔴 `ESP32_WDT_MS + ESP32_ARRANQUE_MS < min(TIMEOUT_ENLACE_MS, SFTY6_SILENCIO_MS)`, **bidireccional** (que quepa **y** que no lo desborde sin sentido) | los cuatro números, de sus tres fuentes: el `.cpp` del ESP32, `protocolo.h:149` y `app.js:1359` |
| **2** | `esp32_02_watchdog_alimentado` | el `esp_task_wdt_reset()` existe, está **dentro de la tarea registrada** con `esp_task_wdt_add()`, y **NO** está dentro del `while` interior (W-3) | el `.cpp` del puente, por texto |
| **3** | `esp32_03_ack_que_mira` | 🔴 **toda rama que conteste `$ACK` evalúa el retorno de la llamada dentro de su `if`**, y toda vía de rechazo tiene su `$ERR` con motivo propio | el despachador del ESP32 |
| **4** | `esp32_04_osf` | el `OSF` se **lee** en el arranque, **decide** el valor de `esp_reloj_enHora()`, y **solo** se limpia tras una escritura confirmada (R-1…R-4) | el driver del `DS3231` |
| **5** | `esp32_05_no_origina` | 🔴 **ningún literal de comando en el fuente del puente**; todo lo que sale a `Serial2` viene del buffer. **Lista blanca escrita a mano** | el `.cpp` del puente |
| **6** | `esp32_06_no_parte_tramas` | una trama entra entera y sale entera; se valida **antes** de retransmitir (B-5, B-6) | el `.cpp` del puente |
| **7** | `esp32_07_presupuesto_bytes` | la cuenta de §3.8 recalculada: `$STATUS` a la cadencia real **cabe** en 960 B/s con margen, y el puente **no añade** tráfico periódico | la cadencia del C++ del STM32 + el baudio + los tamaños de buffer |
| **8** | `esp32_08_silencio_no_es_orden` | ninguna acción cuelga de la **ausencia** de bytes; no hay envío en `setup()`; no hay modo por defecto | el `.cpp` del puente |
| **9** | `esp32_09_contrato_de_bytes` | 🔴 **el contrato de §3 no ha derivado**: el baudio del ESP32 == el del STM32; el tope de línea del ESP32 == `sizeof(btBufIn)-1`; el terminador que emite ∈ `{\r, \n}`; el XOR-8 salta el `$` | **las DOS puntas a la vez** — es el pack de costura |

> 🔴 **El pack 9 es el más importante de los nueve y el que más fácil se olvida.** Es la clase de
> comprobación que salvó la Fase 2: la **comparación de totales** entre las dos puntas. Hoy
> `btBufIn[64]` y `9600` viven en el C++ del STM32; mañana vivirán **otra vez** en el C++ del ESP32.
> **Dos copias del mismo contrato que alguien tiene que sincronizar** — y este repositorio ya sabe
> cómo acaba eso: N-36, N-39, `cfgVerdeRecibido`, y los cuatro límites de N75-2 escritos dos veces
> «sin nada que los ate».

**Etiquetas `SFTY-x`.** Los packs que ejerzan una regla se marcan con `# EJERCE SFTY-x: <qué>` en su
cabecera, porque la tabla de trazabilidad de `OPTIMIZACIONES.md` se levanta **buscando** esa
etiqueta. El pack 1 ejerce **SFTY-6** por el lado del techo, y el pack 4 ejerce **SFTY-18**. Los
demás necesitarían reglas nuevas: **MEDIDO** que la numeración llega hoy a **SFTY-29**, así que las
siguientes empezarían en `SFTY-30` — pero **asignar un número de SFTY es del responsable, no de
quien escribe el pack**, y hasta entonces no se etiqueta. `CLAUDE.md`: *una regla que aparece
cubierta por una prueba que no la ejerce es peor que una fila vacía, porque la vacía no miente.*

### 7.3 §8.bis para cada uno de los nueve. **Sin excepción**

> **Un arnés que no se ha visto fallar es un adorno que da verde.**

No basta con que el pack tenga `control_negativo` escrito dentro: eso es una etiqueta. Para **cada
uno** de los nueve:

1. Se **inyecta el defecto en el `.cpp` real** del ESP32 —no en un bloque sintético—.
2. Se corre `python 01_Firmware/Simulaciones/banco/correr.py --pack esp32_0N`.
3. Se **exige ver las dos cosas**: que **baje la cuenta** y que **cambie el código de salida**.
4. Se restaura el firmware y se verifica con **`git diff HEAD` vacío** — no con la impresión de
   haberlo restaurado.

**Defecto que inyectar en cada uno, para que el paso 1 no se improvise:**

| pack | defecto a inyectar | qué tiene que pasar |
|---|---|---|
| 1 | subir `ESP32_WDT_MS` por encima de 5000 | FALLA |
| 2 | mover el `esp_task_wdt_reset()` dentro del `while` interior | FALLA |
| 3 | quitar el `if` de una rama y dejar el `$ACK` incondicional | FALLA |
| 4 | ignorar el `OSF` y devolver `true` en `esp_reloj_enHora()` | FALLA |
| 5 | añadir un `Serial2.print("CMD:...")` literal | FALLA |
| 6 | partir una trama en dos `write()` | FALLA |
| 7 | bajar el baudio a 2400 sin tocar la cadencia | FALLA |
| 8 | añadir un envío en `setup()` | FALLA |
| 9 | cambiar el tope de línea del ESP32 a 128 | FALLA |

> 🔴 **Y el aviso de N-89, que aplica a los packs 2, 3, 5, 6 y 8 porque leen bloques POR TEXTO:** al
> tocar la **forma** de un bloque que un pack lee por texto —sacar los literales a un compositor,
> por ejemplo— hay que **volver a comprobar que el pack sigue sabiendo fallar**. Un refactor puede
> apagar un instrumento sin romper un solo test, y el pack seguiría en verde midiendo nada.

### 7.4 Los dos arneses de compilación, y por qué esto no es opcional

`CLAUDE.md` §8 lista cuatro arneses que compilan C++ real. **El ESP32 no tendría ninguno.** Los
packs de §7.2 son **Python que parsea texto**: valen para las propiedades estructurales, y **no
prueban que el código compile ni que haga lo que dice**.

Como mínimo hacen falta dos cosas, y las dos van en la compuerta:

| | qué | punto ciego declarado |
|---|---|---|
| **compilación** | que el proyecto del ESP32 **compile** en la compuerta, como ya hace `compilar("repetidor", "Repetidor")` (`compuerta.py:657`) | compilar no es funcionar |
| **arnés de contrato** | compilar el **parser/validador de tramas real** del ESP32 en el PC y ejercerlo con tramas buenas y malas | no hay radio, no hay SPP, no hay `DS3231`: es el modelo, no la tarjeta |

> ⚠️ **Y aquí hay una trampa de entorno que ya costó dos `ABORTADO` (N-44):** el arnés de contrato
> tiene que **exigir enlazar**, no preguntar si hay compilador. Un `gcc` que responde `--version` y
> compila a `.o` puede no enlazar nada. Y el toolchain del ESP32 vive bajo `C:\.platformio`, **fuera
> de la ruta con `ñ`** — la misma razón por la que los otros dos arneses cayeron de un día para otro
> con el mismo compilador registrado en el acta.

### 7.5 🔴 El precedente: N-75 entró con dos instrumentos en `ABORTADO`. Aquí no hay ninguno

`CLAUDE.md` §3.quater: el rewrite de la app entró con **dos** instrumentos en `ABORTADO` —el banco
entero y el arnés de DOM—, que eran justo **los dos únicos que ejercen la app**. Detrás entraron
**cuatro defectos**: una app que dejó de oír al equipo y pintaba un estado inventado, una barrera de
PIN que la propia app abría, un parser de un protocolo que ninguna punta habla, y comandos del
firmware sin interfaz.

> ~~**Aquí la situación es PEOR, y hay que decirlo así: no hay ningún instrumento que pueda abortar,
> porque no hay ninguno.**~~ → 🟢 **Ya no. MEDIDO el 31/08: el rol está dado de alta, el proyecto
> compila en la compuerta y los nueve packs existen.** El aviso se conserva tachado porque describe
> con exactitud el hueco que había, y **el hueco era real hasta esta misma tarde**. Un `ABORTADO` al
> menos grita. Un hueco no.
>
> ⚠️ **Y lo que el verde de esos nueve packs NO dice, que es lo mismo de siempre:** son Python que
> parsea texto y un compilador que enlaza — **ninguno toca la tarjeta**. `CLAUDE.md` §3: *verde no
> es entregable.*
>
> Y el corolario de aquella vez, aplicado a lo que va a pasar cuando el firmware del ESP32 esté
> escrito: *«un parte de trabajo con seis artefactos que existen y 29 tests que pasan puede ser
> cierto en cada línea y falso en conjunto. El dato que lo habría dicho en diez segundos era el que
> no estaba: la salida de `compuerta.py`».*

### 7.6 Orden de trabajo

**No es una lista de tareas: es una secuencia, y el orden es la mitad del valor.**

1. Decidir el nombre y la forma del árbol del ESP32 (§7.1, aviso).
2. **Tocar `compuerta.py`: rol nuevo, regex, suelo de rutas.** Correr la compuerta: tiene que seguir
   verde con el rol nuevo y **cero rutas**.
3. Escribir el pack **1** (la desigualdad del watchdog) **antes** que el watchdog. Verlo **ABORTAR**
   por no encontrar el fuente — que es lo correcto — y luego verlo **FALLAR** con un valor malo.
4. Escribir el watchdog. Pack 1 en verde. **§8.bis sobre él.**
5. Repetir 3-4 para los packs 2 y 9, luego 5, 6, 8 (el puente), luego 3 y 4 (el reloj), luego 7.
6. Compilación en la compuerta (§7.4).
7. **Dos pasadas completas de `compuerta.py`** — no una: `documentos_01_cifras_del_acta` lee el acta
   **anterior**, y las cifras del README se copian del acta que trae las tres filas de `compila`.

---

## 8. Lo que este documento NO cubre, y nadie debe dar por medido

Se escribe explícito porque **una especificación que no marca sus bordes se lee como un permiso**.
Es la sección C del Manual 17, aplicada aquí.

| | |
|---|---|
| **Nada de esto ha pasado banco** | y **ni un verde de `compuerta.py` lo autoriza**: ese `0` dice que los modelos y los arneses de PC no encuentran nada, y **ninguno toca la tarjeta** |
| ~~**El firmware del ESP32 no existe**~~ | 🟢 **existe y compila al `35.6 %`** (`evidencia/2026-08-31_compuerta.txt:16`). Lo que **no** cambia: **compilar no es funcionar**, y **no ha pasado banco** |
| ~~**El chip que llegó a obra**~~ | 🟢 **`BLQ-1` CERRADO el 31/08**: `ESP32-WROOM-32` clásico, hay SPP (§6.1, `ESTADO.md:23`, N-107) |
| **Que el enlace `J17` funcione** | `13_Manual...:99`: *«tampoco está verificado en banco el enlace Bluetooth sobre `J17` p2/p3: la compuerta pasó, y la compuerta no toca la tarjeta»* |
| **El `DS3231`** | **no se ha comprado** (`A6`). Su dirección `0x68`, sus pull-ups y su `OSF` son datasheet, no medida sobre el módulo real |
| **La fuente propia (`A5`)** | **no está pedida**. Sin ella no se conecta nada: el ESP32 tumbaría el riel del STM32 |
| **El pico de 500 mA del ESP32** | ESCRITO en el Manual 15, **no medido** sobre el módulo real |
| **El tiempo de arranque del ESP32 y el de reemparejar SPP** | **SIN VERIFICAR**. `ESP32_ARRANQUE_MS` es hoy un hueco en la desigualdad de §4.2, y hay que **medirlo**, no estimarlo |
| **El nombre real del pin 3 de `J17`** | sigue en disputa (§2.3) |
| 🔴 **El rótulo del dispositivo** | el puente lo compone y lo guarda (§6.5, **MEDIDO** en el fuente), y **nadie lo ha visto nunca en la lista de emparejados de un teléfono**. Desde el 04/09 el Manual 17 §3.7 apoya en él una decisión operativa —a qué poste camina el operario—, y §3.8 deja abierto que **dos módulos vírgenes se llaman igual** hasta una vuelta de energía (`AB-9`) |
| **Que un watchdog resuelva §17 3.3** | **no lo resuelve**. Cubre el ESP32 colgado; **no** el muerto ni el desenchufado |
| **La regresión N-42** | el Modo Automático no mueve las luces en banco y **sigue abierta** (`ESTADO.md` `B2`). Es anterior a esta arquitectura y **no se cierra con ella** |
| **El `Y2` de la segunda tarjeta** | N-37 midió uno; el otro sigue sin diagnosticar (`ESTADO.md` `B5`) |

> 🔴 **Y el borde que más fácil se cruza al leer esto:** que el ESP32 lleve reloj **no arregla** el
> `Y2` de los STM32. Son dos relojes distintos. Mientras `reloj_enHora()` del STM32 devuelva `false`,
> el Modo Degradado y todo lo que cuelga de SFTY-20/21 siguen igual de bloqueados que hoy. La
> propuesta de puentearlo —Manual 17 §3.2 vía B, reloj de software disciplinado por el ESP32—
> **cuelga el reloj del semáforo del módulo accesorio**, que es justo lo que §1.2 separa. Está
> abierta y tiene dueño (§9 `AB-4`).

---

## 9. Lo que queda ABIERTO, con dueño

**Ninguna de éstas la puede tomar quien escribe firmware.** Van con quien las firma. Lo abierto se
deja abierto: no se inventa una decisión para que el documento parezca cerrado.

| id | qué está abierto | dueño | qué desbloquea |
|---|---|---|---|
| ~~**`BLQ-1`**~~ | 🟢 **CERRADO el 31/08.** Es un **`ESP32-WROOM-32` clásico**: `Xtensa LX6 dual-core`, `Bluetooth v4.2 BR/EDR + BLE`. **Hay SPP**, la app conecta sin tocar el transporte y **el Manual 10 §1 no se reabre** (§6.1, `ESTADO.md:23`, `roadmap.md:215` N-107). Queda una pregunta menor y no bloqueante: **30 o 38 pines** de la DevKitC, para las hembrillas | ~~el responsable~~ **resuelto** | desbloqueó la compra (`A1′`) y §6 entera |
| **`AB-1`** | 🔴 **Nada en el equipo vigila al puente.** MEDIDO en §4.2: SFTY-6 mira la radio, no `J17`. Un ESP32 colgado es **invisible** para el STM32. ¿Se acepta que el único testigo sea la app, o el STM32 tiene que notar el silencio del puente? | **el responsable** *(es una decisión vial: cambia lo que el equipo hace cuando se queda sin operador)* | el alcance del watchdog y si hace falta un SFTY nuevo |
| ~~**`AB-2`**~~ | ✅ **DECIDIDA el 31/08 y la mitad que faltaba el 04/09.** ~~*Cómo se opera el equipo si el ESP32 se cuelga, sin pantalla, sin pulsadores y sin mando*~~ → el mando **se queda** en los canales `A` y `B` (Manual 17 §3.3, opción 3), y el cruce **se opera desde el Maestro** (§3.7). 🔴 **Lo que sigue abierto no es la decisión, es su demostración:** el mando **no se pudo pulsar en banco** (N-118), el fuente se corrigió el 04/09 y **no se ha cargado en ninguna tarjeta**. El watchdog sigue cubriendo el colgado y **no** el muerto ni el desenchufado | ~~el responsable~~ **decidida; falta la carga verificada** | ya no bloquea el alcance de §6; **sí** bloquea que se pueda vender como salida de emergencia |
| **`AB-9`** | 🔴 **NUEVA (04/09): dos módulos vírgenes se anuncian con el MISMO nombre.** El rótulo bueno se aprende del `$STATUS` y entra **en la siguiente arrancada** (§6.5); hasta entonces las dos puntas dicen `SEM-SIN-MATRICULA`. Y `AB-2` acaba de convertir ese rótulo en **lo que le dice al operario a qué poste caminar** (§17 3.7). ¿Se cubre por procedimiento —una vuelta de energía a cada módulo antes de irse, firmada en el acta— o el firmware da un provisional distinto por módulo? Las cuatro opciones, en el Manual 17 §3.8 | **el responsable** | si hay que tocar el firmware del puente antes de la primera puesta en marcha |
| **`AB-3`** | 🟠 **`ESP32_ARRANQUE_MS` y el tiempo de reemparejar SPP: SIN VERIFICAR.** Son el hueco de la desigualdad de §4.2, y **se miden con el módulo en la mano**, no se estiman | **quien monte**, con visto bueno técnico | el número concreto del watchdog, y qué tiene que decirle la app al operario tras un reinicio |
| **`AB-4`** | 🟠 **El `Y2`: se repara, o el STM32 lleva reloj de software disciplinado por el ESP32.** La vía B **cuelga el reloj del semáforo del accesorio** — contra §1.2. Antes hay una medida pendiente que puede ahorrar la compra entera (`ESTADO.md` `B5`) | **el responsable** | si el `DS3231` del ESP32 basta o hay que tocar el STM32 |
| **`AB-5`** | 🟡 **`$STATUS` tiene 8 B de margen** antes de que `snprintf` trunque el payload (§3.3), y **nada lo vigila**. ¿Se acorta un campo, se sube el buffer, o se añade un pack que mida el margen? | técnico | evita una trama cortada a mitad de campo el día que crezca un literal |
| **`AB-6`** | 🟡 **El nombre real del pin 3 de `J17`**: `RS(A0)` en el esquemático contra `LCD_PSB` en el firmware. Se cierra **siguiendo el hilo**, no leyendo más código | **quien monte** | el cableado del ESP32 |
| **`AB-7`** | 🟡 **El PIN `1234` en claro en el fuente y en el aire** (§3.5). Es una limitación conocida. Cambiar el esquema toca las dos puntas y la app, y **no cabe en la especificación de un puente** | **el responsable** | nada de este documento; se anota para que no se dé por resuelto |
| **`AB-8`** | 🟡 **Los números de `SFTY-x` de los packs nuevos.** La numeración llega hoy a `SFTY-29`. **Asignar uno es del responsable**, y hasta entonces los packs no se etiquetan | **el responsable** | la tabla de trazabilidad de `OPTIMIZACIONES.md` |

---

## Anexo · Lo que otros ficheros necesitan y este documento NO ha hecho

**Ninguno se ha tocado.** Es la lista de trabajo, no el trabajo.

| fichero | qué le falta |
|---|---|
| ~~`01_Firmware/compuerta.py`~~ | 🟢 **hecho el 31/08**: `:114` el rol, `:118` la regex, `:97` el suelo (45), `:692` la compilación |
| ~~`01_Firmware/Simulaciones/banco/packs/`~~ | 🟢 **hecho el 31/08**: existen `esp32_01` … `esp32_09` |
| `05_Funcional/15_Lista_de_Compras_Hardware.md` | 🟠 `:198` marca la línea `A1′` **🛑 BLOQUEADA por `BLQ-1`**, y `BLQ-1` está **cerrado** desde el 31/08. Deriva pendiente |
| `05_Funcional/App_Semaforo/js/nmea_parser.js` | 🟠 **módulo sin un solo llamador** (§3.4.a): cargado por `index.html:589` y usado sólo en `tests/`. Habla un protocolo (`$…*XX`) que **ninguna punta de este sistema habla**. Es N-73 en JavaScript, y ya indujo un defecto de firmware. ¿Se retira o se documenta como muerto? **No se decide desde aquí** |
| `OPTIMIZACIONES.md` | `:55` — SFTY-1 dice *«El Repetidor ESP32 no implementa watchdog»*. Cuando lo implemente, esa frase queda falsa |
| `05_Funcional/17_...md` §1.4 | cita `Maestro/src/bluetooth.cpp:25`; hoy es `:28` |
| `05_Funcional/17_...md` §3.3 | cita `Maestro/src/main.cpp:52`; hoy es `:53` |
| ~~`05_Funcional/5_Manual_Puente_ESP32.md`~~ | 🟢 **corregido el 31/08**: lleva recuadro que separa los **dos ESP32** y avisa de la **colisión de GPIO** (`GPIO16`/`17`/`22` sirven a cosas distintas en cada rol). Sigue describiendo sólo el rol Repetidor, que es su asunto |
| `05_Funcional/10_...Bluetooth...md` | congelado en SPP, y sigue mandando enchufar un `HC-05` en `J17` (Manual 17 §B, Orden 2) |
| `ESTADO.md` / `roadmap.md` | `BLQ-1` y `AB-1`…`AB-8` no están anotados como abiertos con dueño |

---

*Documento escrito el 31/08/2026 sobre `HEAD = cc4ba61`. Banco medido antes y después de esta
pasada: **445/445, 39 packs, 0 FALLA, 0 ABORTADO**. Este documento no toca código ni instrumentos.*

---

### Segunda pasada — 31/08/2026, más tarde el mismo día

**Qué se corrigió, y por qué la lista importa más que las correcciones:**

| § | qué estaba mal | clase de error |
|---|---|---|
| **§3.4** | 🔴 **`formatearComando()` no existe y la app no añade `*XX`.** Sobre esa frase falsa —**con la palabra MEDIDO encima**— la sección ordenaba al ESP32 validar `$`, `*` y XOR-8. El firmware lo obedeció en su primera versión y **habría descartado el 100 % de los comandos reales** | `CLAUDE.md` §4 segunda cara: **lo que TÚ reportas también es un instrumento** |
| **§3 `E-2`** | la cifra de `41 caracteres` no cuadraba: la cadena mide **40**, y la que la app componía de verdad en el locale de campo medía **45** | una foto de un día publicada como constante |
| **§3.7** | `$EVENT`: se decía **14** ramas y `app.js:814-815`. MEDIDO: **16** ramas y `app.js:976-977` | dos cifras que apoyaban una conclusión correcta, y que **nadie iba a comprobar por eso mismo** |
| **§6, §9** | `BLQ-1` dado por abierto; está **cerrado** desde el 31/08 | deriva entre documentos |
| **cabecera, §7.1, §7.5, §8, Anexo** | el firmware, el rol de la compuerta y los nueve packs se daban por inexistentes; **existen los tres** | deriva |

**Lo que sí se re-midió y estaba bien** —se anota porque una lista de errores sin la lista de
aciertos no dice si la revisión fue exhaustiva o afortunada—: los baudios (`9600` en las dos
puntas), `SerialBT(PB7, PB6)` en `Maestro:28` y `Esclavo:26`, `btBufIn[64]`, `payload[128]`,
`payload[100]`, `p[80]`, `tramaCompleta[140]`, los recuentos `$ACK` 17/4 y `$ERR` 16/8,
`IWatchdog.begin(4000000)` en las dos puntas, `SFTY6_SILENCIO_MS 25000UL`,
`TIMEOUT_ENLACE_MS = 5000` y que la numeración `SFTY-x` llega a **29**.

**Ninguna afirmación se borró: las cuatro están tachadas con lo medido al lado.** *Una causa que se
cae se marca refutada; la que desaparece en silencio vuelve a proponerse, y la segunda vez ya nadie
recuerda que se comprobó.*

🛑 **Y lo que esta segunda pasada NO cambia: nada ha pasado banco.** Este documento sigue sin tocar
código ni instrumentos.
