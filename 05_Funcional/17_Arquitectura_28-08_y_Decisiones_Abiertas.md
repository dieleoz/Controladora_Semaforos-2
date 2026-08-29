# 17 — Arquitectura decidida en obra el 28/08/2026, y lo que sigue abierto

**Para:** el funcional y el auditor.
**Fecha del documento:** 28 de agosto de 2026.
**Acta de compuerta de referencia:** `evidencia/2026-08-28_compuerta.txt` — `15 PASS | 0 FALLA | 0 ABORTADO`,
HEAD `3733544`, rama `main-nuevo`, **arbol LIMPIO** (lo dice la propia acta).

Este documento esta escrito **en ASCII sin acentos**, como el resto de lo que se parsea o se lee en
consola de Windows en este repositorio.

> **Este documento no cambia ningun otro fichero.** Todo lo que otro documento necesita corregir
> esta listado en la seccion B y en el anexo final, y **no se ha tocado**. Habia otros trabajos en
> vuelo sobre el mismo arbol el dia que se escribio.

---

## 0. Como se lee este documento

Tres niveles, y no se mezclan nunca. Es la misma escala que usa
`03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md` §0, extendida al firmware.

| marca | que significa |
|---|---|
| **MEDIDO** | se abrio el fichero y se leyo. Va con `fichero:linea` o con el nombre de la red del netlist. Se puede repetir |
| **ESCRITO** | lo afirma un documento, un comentario o un acta. Puede ser cierto; **no se ha comprobado en esta pasada** |
| **SIN VERIFICAR** | nadie lo ha comprobado, ni aqui ni en ningun sitio. Casi todo lo del cobre esta aqui |

> 🔴 **La linea que manda sobre todo lo demas: hoy no existe ni una sola fila «VERIFICADO EN LA
> PLACA» en todo el mapeo de la tarjeta.** Lo dice `MAPEO_TARJETA_KICAD.md` §0 y §9, y sigue siendo
> verdad. Todo lo que este documento llama MEDIDO se midio **sobre ficheros**: el `.cpp`, el `.h`,
> el `.kicad_sch` y el `.kicad_pcb`. Un fichero dice lo que alguien **dibujo o escribio**. Una placa
> dice lo que se **fabrico**, y lo que alguien reparo despues.

---

## 1. La arquitectura decidida en obra

Esto es lo acordado. No se discute aqui; se documenta y se le cuelgan sus consecuencias.

### 1.1 El reparto, en una frase

**El STM32 sigue siendo el controlador del semaforo. El ESP32 es un modulo de expansion colgado de
un puerto serie: aporta reloj y Bluetooth, y no manda sobre las luces.**

```
                       fuente propia 12 V (NO sale de la tarjeta)
                                    |
   +--------------------+     +-----v--------------------+
   |   STM32F103C8      |     |         ESP32            |
   |   (controlador)    |     |   (modulo de expansion)  |
   |                    |     |                          |
   |  8 luces  J3-J9,J11|     |  DS3231  GPIO21 SDA      |
   |  barrera  J15      |     |          GPIO22 SCL      |
   |  buzzer   J13      |     |          (pila propia)   |
   |  camaras  J16      |     |                          |
   |  LoRa     J12      |     |  Bluetooth (sustituye    |
   |           USART3   |     |   al modulo SPP)         |
   |                    |     |                          |
   |  USART1 remapeado  |     |  futuro: WiFi / GPS      |
   |  PB6 TX == J17 p3 <------ GPIO16 (RX2)              |
   |  PB7 RX == J17 p2 ------> GPIO17 (TX2)              |
   +---------|----------+     +-----------|--------------+
             |                            |
             +-------- masa comun --------+
                       9600 8N1
```

### 1.2 Que se queda en el STM32

Todo esta MEDIDO en `01_Firmware/Maestro/include/pines.h`.

| funcion | pin | bornera | linea |
|---|---|---|---|
| Rojo 1 / Amarillo 1 / Verde 1 | `PA0` `PA1` `PA2` | `J3` `J4` `J5` | `pines.h:5-7` |
| Rojo 2 / Amarillo 2 / Verde 2 | `PA3` `PA4` `PA5` | `J6` `J7` `J8` | `pines.h:10-12` |
| Rojo peaton / Verde peaton | `PA6` `PA7` | `J11` `J9` | `pines.h:15-16` |
| Barrera (talanquera) | `PB2` | `J15` | `pines.h:31` |
| Buzzer | `PB1` | `J13` | `pines.h:20` |
| Radio LoRa (`USART3`) | `PB10` TX · `PB11` RX · `PB12` DE/~RE | `J12` | `pines.h:112-114`, `:19` |
| Camara de demanda (hoy) | `PB0` | `J14` | `pines.h:46` |

La barrera de salidas de `CLAUDE.md` §6 no cambia: **solo `semaforo.cpp` escribe pines de luz**, y
la talanquera entro dentro de `escribirPines()` el 27/08 (`ESTADO.md`, fila `A2`).

### 1.3 Que se lleva el ESP32

| funcion | como | estado |
|---|---|---|
| Reloj `DS3231` con pila propia | I2C: `GPIO21` = SDA, `GPIO22` = SCL. El modulo `ZS-042` trae sus pull-ups | **decidido**, sin construir |
| Bluetooth | sustituye al modulo SPP dedicado, que se retira | **decidido**, condicionado a §3.1 |
| WiFi / GPS | futuro | no decidido |

### 1.4 El enlace, pin a pin

**MEDIDO** en el firmware: el `USART1` ya esta remapeado a `PB6`/`PB7` y ya sale por `J17`.

```
01_Firmware/Maestro/src/bluetooth.cpp:25   static HardwareSerial SerialBT(PB7, PB6);
01_Firmware/Esclavo/src/bluetooth.cpp:26   static HardwareSerial SerialBT(PB7, PB6);
01_Firmware/Maestro/include/bluetooth.h:7  "PB6 TX, PB7 RX ... Sale por el conector J17, posiciones 3 y 2"
```

| ESP32 | direccion | `J17` | STM32 | pin del `U1` |
|---|---|---|---|---|
| `GPIO17` (TX2) | ---> | **p2** | `PB7` — **RX** del micro | 43 |
| `GPIO16` (RX2) | <--- | **p3** | `PB6` — **TX** del micro | 42 |
| `GND` | --- | p7 o p9 | `GND` | — |

`9600 8N1`. **Masa comun, obligatoria.** El mapa de `J17` esta MEDIDO en el netlist del
`.kicad_pcb`: `p1=/CS`, `p2=/RST`, `p3=/RS(A0)`, `p4=/SCL`, `p5=/SI`, `p6=/3.3V`, `p7=GND`,
`p8=/3.3V`, `p9=GND`, y `p10`-`p13` sin red.

> ⚠️ **El nombre del pin 3 sigue en disputa y ahora tiene mas dueno que antes.** La etiqueta de red
> del esquematico es `RS(A0)`; el firmware lo llama `LCD_PSB` y lo trata como `PSB`. Los dos nombres
> no pueden ser ciertos a la vez (`MAPEO_TARJETA_KICAD.md` §6.bis, `pines.h:77-84`). **Con la LCD
> retirada la duda deja de amenazar a la pantalla, pero no desaparece:** si esa pata fuera de
> verdad un `RS/A0` de un display, el hilo del ESP32 va a un sitio con otro nombre. Se cierra
> siguiendo el hilo del pin 3 hasta la pata rotulada, no leyendo mas codigo.

### 1.5 La alimentacion: por que el ESP32 NO cuelga de `J17`

**El ESP32 lleva fuente propia desde 12 V.** No se alimenta de los 3,3 V de `J17` p6/p8.

El motivo esta ESCRITO en `05_Funcional/15_Lista_de_Compras_Hardware.md:102-106` y en el
Manual 10: un ESP32 con radio da picos de corriente del orden de **500 mA**, y ese riel de 3,3 V
—el que sale del `U5` LM1117DT-3.3, `MAPEO_TARJETA_KICAD.md` §2— es **el mismo que alimenta al
STM32 que gobierna el semaforo**. Un reset del controlador por una caida de riel provocada por un
periferico de diagnostico es exactamente el reparto de riesgo que no se acepta: el accesorio no
puede tumbar al que manda.

> **SIN VERIFICAR:** la cifra de 500 mA es de datasheet y de lo escrito en el Manual 15, no medida
> sobre el modulo que llego a obra. **No hace falta medirla para decidir**: la decision es no
> compartir riel, y esa decision no se cae si el pico resulta ser 300 mA.

### 1.6 Que se retira

| se retira | consecuencia inmediata |
|---|---|
| Pantalla LCD (las dos puntas) | toda la operacion pasa por la app |
| Los cuatro pulsadores (`PB9`, `PB13`, `PB14`, `PB15`) | libera `J16`; **rompe la unica salida de modo** — ver §2.3 |
| Mando de 4 reles | **quita un veto que hoy esta activo** — ver §2.4 |
| Modulo Bluetooth SPP dedicado (`HC-05`/`JDY-30`) | lo sustituye el ESP32 — ver §3.1 |

### 1.7 Las camaras a `J16`

Los pines que libera la retirada del mando y de los pulsadores:

| `J16` | red | GPIO | uso nuevo |
|---|---|---|---|
| p1 | `/12V` | — | 🔴 **12 V crudos. Se tapa** — ver §2.1 |
| p2 | `GND` | — | masa |
| p5 | `/Boton1` | `PB9` | **vacio a proposito** (colchon) |
| p8 | `/Boton2` | `PB13` | **vacio a proposito** (colchon) |
| p10 | `/Boton3` | `PB14` | **Camara 2** |
| p12 | `/Boton4` | `PB15` | **Camara 1** |

Dejar `p5` y `p8` sin cablear pone cobre y aire entre los 12 V de `p1` y los hilos de camara.
**MEDIDO**, con el paso del footprint (`Molex_KK-254_AE-6410-16A_1x16_P2.54mm_Vertical`, 16 pads,
tanto en `J16` como en `J17`):

| de `p1` a | posiciones | distancia |
|---|---|---|
| `p5` (`PB9`) | 4 | **10,2 mm** |
| `p10` (`PB14`) | 9 | **22,9 mm** |
| `p12` (`PB15`) | 11 | **27,9 mm** |

---

## 2. Lo que esta arquitectura hereda — ocho hallazgos MEDIDOS

Ninguno de estos ocho es una opinion. Todos se pueden repetir abriendo el fichero que se cita.

### 2.1 🔴 `J16` p1 lleva 12 V crudos a cuatro posiciones del primer pin de GPIO

**MEDIDO** sobre el netlist del `.kicad_pcb` bueno
(`01_Firmware/Controladora_Semaforos/Controladora_Semaforos/Controladora_Semaforos.kicad_pcb`):

```
   J16  pad 1  -> red "/12V"
   J16  pad 5  -> red "/Boton1"   (PB9, U1 pin 46)
```

**No hay opto, no hay resistencia en serie y no hay clamp entre esa posicion y el resto del
conector.** `MAPEO_TARJETA_KICAD.md` §8.2 mide que el aislamiento galvanico de esta tarjeta vive
**solo** en las diez cadenas de potencia (opto `TLP127` + MOSFET) que van a las borneras `J3`-`J9`,
`J11`, `J13`, `J15`. `J16` no es una de ellas: es un conector de senal directo al `U1`.

Y `MAPEO_TARJETA_KICAD.md` §7 lo dice en una linea que conviene no perder: **`J16` es el unico
conector de senal de toda la tarjeta que trae 12 V.**

> **Accion:** tapar `p1` fisicamente antes de cablear nada en `J16` — funda termorretractil sobre
> el pin, o el pin retirado del cuerpo del conector volante. No basta con «no conectarlo»: el
> destornillador, la viruta y el hilo suelto no leen documentacion. Un contacto de `p1` a `p10` o
> `p12` mete 12 V en una pata del `U1` que espera 3,3.

### 2.2 🔴 La polaridad de los pines de boton esta en contradiccion — y es N-67 otra vez

Es el bloqueante del cableado de camaras. **Los dos lados estan MEDIDOS y no pueden ser ciertos a
la vez.**

**Lo que dice la placa** (netlist del `.kicad_pcb`, y valor leido del `.kicad_sch`):

```
   R65 = 10K   pad1 -> GND      pad2 -> /Boton1   (PB9)
   R66 = 10K   pad1 -> GND      pad2 -> /Boton2   (PB13)
   R67 = 10K   pad1 -> GND      pad2 -> /Boton3   (PB14)
   R68 = 10K   pad1 -> GND      pad2 -> /Boton4   (PB15)

   J16  p4 = /3.3V   p5 = /Boton1
        p7 = /3.3V   p8 = /Boton2
        p9 = /3.3V   p10 = /Boton3
        p11 = /3.3V  p12 = /Boton4
```

Eso es **pull-DOWN de 10 kOhm a masa, con 3,3 V en la posicion de al lado**: el gesto previsto es
cerrar el contacto contra los 3,3 V del propio conector, o sea **entrada activa en ALTO**.

**Lo que dice el firmware:**

```
   01_Firmware/Maestro/src/botones.cpp:50-53   pinMode(BOTONn, INPUT_PULLUP);
   01_Firmware/Maestro/src/botones.cpp:19      bool lecturaCruda = (digitalRead(b.pin) == LOW);
   01_Firmware/Esclavo/src/botones.cpp:70-73   pinMode(BOTONn, INPUT_PULLUP);
   01_Firmware/Esclavo/src/botones.cpp:33      bool lecturaCruda = (digitalRead(b.pin) == LOW);
   01_Firmware/Esclavo/src/botones.cpp:16      "Entradas en INPUT_PULLUP y pulsador contra masa: pulsado = LOW"
```

O sea **pull-UP interno, activo en BAJO**. Los dos no pueden ser ciertos.

**Esto ya paso, con la misma resistencia y el mismo valor.** `R64` es tambien 10K a GND, sobre la
red `/Puerta` que llega a `J14` p1, con 3,3 V en `J14` p2. Es la entrada de camara, y la cuenta que
cierra `roadmap.md` N-67 (linea 552) es esta:

> el pull-up interno (~40 kOhm) contra el pull-down de 10 kOhm de la placa deja el pin en
> `3,3 x 10/50 = 0,66 V`, que el micro lee **LOW**. El firmware habria visto **demanda permanente
> desde el arranque, sin ninguna camara conectada**.

La camara se arreglo: `pinMode(CAM_DEMANDA_PIN, INPUT)` y deteccion contra `HIGH`
(`Maestro/src/modo_inteligente.cpp:19-25`, `:44`; `Esclavo/src/main.cpp:288`, `:350`).
**Los cuatro pines de boton, con topologia identica en el netlist, siguen en `INPUT_PULLUP` y
`== LOW`.**

> **Lo que NO se puede concluir desde aqui, y por eso esto es una contradiccion y no un defecto:**
> si esa cuenta describiera la placa fisica, los cuatro botones estarian en LOW permanente y el menu
> no se podria navegar. Y **hay evidencia de banco de que el menu se navega** — todo el protocolo de
> la seccion 5, y N-53. Asi que **o el netlist no describe esta tarjeta, o el pull-up interno real es
> mas fuerte de lo que dice el datasheet, o hay una diferencia entre la placa dibujada y la
> soldada.** Se cierra con multimetro, no con mas lectura: medida **M3** de la seccion A.
>
> 🟡 **Y hay una coincidencia que hay que anotar sin darla por causa.** `botones.cpp:60-64` del
> Maestro documenta N-26, CONFIRMADO EN BANCO el 01/08/2026: *"el Maestro aparecia solo en la
> pantalla de configuracion del Modo Manual sin que nadie tocara nada"*. Ese es exactamente el
> sintoma de un pin en LOW al arrancar. **N-26 se cerro sembrando el estado real del pin**, que
> enmascara el sintoma sin decidir cual de las dos polaridades es la buena. Es una pista, **no una
> medida**, y este documento no la convierte en causa.

**Consecuencia operativa: mientras esto no se mida, no se cablea camara a `J16`.** Cablear una
camara con la polaridad al reves da **demanda permanente** —un semaforo que pide paso solo— o
**demanda que nunca llega**. Las dos son de calle.

### 2.3 🔴 `botonCancelar()` es la unica salida de todos los modos, y desde Bluetooth no hay vuelta

**MEDIDO** por censo de llamadores de `botonCancelar()` en el Maestro:

| modo | quien sale | linea |
|---|---|---|
| Degradado | `botonCancelar()` | `modo_degradado.cpp:443` |
| Alcance | `botonCancelar()` | `modo_alcance.cpp:50` |
| Ambar | `botonCancelar()` | `modo_ambar.cpp:42` |
| Automatico | `botonCancelar()` | `modo_automatico.cpp:80` |
| Manual | `botonCancelar()` | `modo_manual.cpp:21` |
| Inteligente | `botonCancelar()` | `modo_inteligente.cpp:65` |
| Ajustar hora | `botonCancelar()` | `modo_hora.cpp:262` |
| Menu (subir de nivel) | `botonCancelar()` | `menu.cpp:151` |

**MEDIDO** el juego completo de comandos que atiende el Maestro por Bluetooth
(`Maestro/src/bluetooth.cpp:108-181`):

```
   CMD:FORZAR_ROJO                (sin PIN, deliberado — bluetooth.cpp:99-113)
   CMD:PIN:1234:SET_MODO:AUTO
   CMD:PIN:1234:SET_MODO:MANUAL
   CMD:PIN:1234:SET_MODO:AMBAR
   CMD:PIN:1234:FORZAR_ROJO
   CMD:PIN:1234:MANUAL:CAMBIAR_TURNO
   CMD:PIN:1234:TEST_LEDS
   CMD:PIN:1234:SET_TIEMPOS:<v>,<r>,<d>
   CMD:PIN:1234:SET_RTC:<fecha>,<hora>
```

Y el del Esclavo (`Esclavo/src/bluetooth.cpp:124-168`): `FORZAR_ROJO`, `SOLICITAR_PASO`,
`TEST_LEDS`, `SET_RTC`. **Ningun `SET_MODO`.**

El Maestro tiene **ocho** modos (`bluetooth.cpp:185-196`: `MENU`, `MANUAL`, `AUTO`, `INTELIGENTE`,
`ALCANCE`, `HORA`, `DEGRADADO`, `AMBAR`). Desde Bluetooth se alcanzan **tres**. Y **no hay
`SET_MODO:MENU`**.

> **Consecuencia exacta, y es la que hay que leer despacio:** hoy el operario entra a un modo por
> el menu y sale con el Boton 4. Si se ignoran los pulsadores **antes** de anadir `SET_MODO:MENU`,
> cada modo se convierte en una puerta de un solo sentido: se entra desde el celular y **no se sale
> por ningun sitio**. Lo unico que queda es `FORZAR_ROJO` —que para el trafico pero no devuelve el
> mando— y cortar la energia.
>
> **`SET_MODO:MENU` va ANTES de retirar los pulsadores, no despues.** Y la app tampoco lo tiene:
> **MEDIDO** en `05_Funcional/App_Semaforo/app.js`, la app manda `SET_MODO` con `AUTO`, `MANUAL` y
> `AMBAR` (`app.js:436`, `:452`, `index.html:267`), `MANUAL:CAMBIAR_TURNO` (`:444`), `FORZAR_ROJO`
> (`:461`), `SOLICITAR_PASO` (`index.html:271`), `SET_TIEMPOS` (`:819`), `SET_RTC` (`:865`, `:877`)
> y `TEST_LEDS` (`:900`). Ninguna vuelta al menu.

### 2.4 🔴 Retirar el mando no deja tres `if` inertes: **borra un veto**

**MEDIDO**: `mando_ambarLocal()` (`Esclavo/src/mando.cpp:103`) tiene tres consumidores, todos en
`Esclavo/src/main.cpp`, y **los tres son negados**:

| linea | que veta hoy |
|---|---|
| `401` | `if (!mando_ambarLocal())` antes de obedecer `CMD_GO_RED` |
| `408` | `if (!mando_ambarLocal())` antes de obedecer `CMD_GO_GREEN` |
| `526` | `if (!mando_ambarLocal() && ...)` antes de recuperarse de `S_FALLO` con `CMD_GO_RED` |

El comentario de `main.cpp:396-400` explica el porque: con el ambar pedido desde el piso con
`B·B·B`, el Esclavo **no obedece ni acusa recibo**, para que el Maestro agote reintentos y el
**cruce entero** termine en ambar, que es lo que el operario pidio.

> **Al retirar el mando, `mando_ambarLocal()` pasa a devolver siempre `false` y esos tres `if` se
> vuelven siempre-verdaderos.** El codigo no queda muerto: **queda abierto**. Una orden de radio
> vuelve a poder sacar al Esclavo de un ambar que un operario habia dejado puesto a proposito.
>
> Eso no es un residuo de limpieza: es **SFTY-21 desapareciendo por sustraccion**. Antes de quitar
> el mando hay que decidir **quien hereda ese veto** —un flag de «ambar local vigente» que ponga la
> app, o la decision explicita y escrita de que ese veto ya no existe— y el pack que lo vigile.

### 2.5 🔴 `SET_RTC` puede rechazar en silencio y contestar `RESULT:OK`

**MEDIDO**, y son dos ficheros:

```
   01_Firmware/Maestro/src/reloj.cpp:290       if (!rtcOperativo) return;      <- rechaza y no dice nada
   01_Firmware/Maestro/src/bluetooth.cpp:173   reloj_ajustar(...);
   01_Firmware/Maestro/src/bluetooth.cpp:175   enviarTramaConCrc("$ACK,CMD:SET_RTC,RESULT:OK");
```

`bluetooth.cpp` valida **el formato** de la trama (`sscanf(...) == 6`) y sobre esa validacion
contesta `OK`. **No consulta el resultado de `reloj_ajustar()`, que no devuelve nada.** El rechazo
por falta de oscilador esta escrito con toda intencion en `reloj.cpp:280-290` —y la razon es buena:
escribir una hora que nadie hace avanzar dejaria `horaValida` en `true` y sobre esa mentira el
Maestro empujaria la hora al Esclavo y autorizaria el Modo Degradado—. Lo que falta es **que se
entere el que pregunta**.

**Y no es hipotetico.** `roadmap.md:3376` (N-17) y `roadmap.md:3366` (N-37) cierran, con medida de
banco del 01/08/2026: **el cristal `Y2` no oscila en las tarjetas actuales.** Tres eliminaciones
documentadas: `VBAT` a 3 V con la tarjeta apagada, el reintento de N-25 cada 30 s y `REINICIAR
RELOJ` devolviendo `SIGUE PARADO`.

> **Consecuencia con la nueva arquitectura, y es peor que hoy, no mejor.** Hoy el operario tiene la
> pantalla `CONSULTA RELOJ` para ver que el reloj no arranco. **Sin pantalla, el unico canal es el
> `$ACK`, y el `$ACK` dice `OK`.** La app le confirmara al tecnico que puso la hora, en un equipo en
> el que la hora no se puede poner. El campo `HORA:` de `$STATUS` seguira diciendo `--:--:--`
> (`bluetooth.cpp:230-235`), que es la unica pista, y esta al lado de un `OK`.
>
> El firmware ya tiene el dato: `reloj_hayCristal()` (`reloj.cpp:219`). Lo que falta es que
> `SET_RTC` lo mire antes de contestar.

### 2.6 🟠 Telemetria fabricada: que campos de `$STATUS` son datos y cuales son texto

**MEDIDO** sobre los dos `snprintf`.

**Esclavo** (`Esclavo/src/bluetooth.cpp:215`):

```
"$STATUS,NODE:ESCLAVO,SERIE:%s,MODO:SUBORDINADO,ESTADO:%s,T:%lu,RF:98%%,RTT:85ms,BAT:12.6,HORA:%s"
```

**Maestro** (`Maestro/src/bluetooth.cpp:245`):

```
"$STATUS,NODE:MAESTRO,SERIE:%s,MODO:%s,ESTADO:%s,T:%lu,RF:%d%%,RTT:%lums,BAT:12.6,HORA:%s"
```

| campo | Maestro | Esclavo |
|---|---|---|
| `SERIE` | dato (UID de silicio) | dato |
| `MODO` | dato | 🔴 **literal `SUBORDINADO`** |
| `ESTADO` | dato | dato |
| `T` | 🟠 **`(millis()/1000) % 60`** — `bluetooth.cpp:242` | 🟠 **igual** — `bluetooth.cpp:208` |
| `RF` | dato (`coordinador_calidadEnlace()`) | 🔴 **literal `98%`** |
| `RTT` | dato (`coordinador_tiempoRespuestaMs()`) | 🔴 **literal `85ms`** |
| `BAT` | 🔴 **literal `12.6`** | 🔴 **literal `12.6`** |
| `HORA` | dato, o `--:--:--` | dato, o `--:--:--` |

**`BAT:12.6` es literal en las dos puntas, y no hay ningun `analogRead` en el firmware.**
Comprobado con el buscador descartado antes de reportar (`CLAUDE.md` §4): `grep -rn analogRead`
sobre `01_Firmware/Maestro` y `01_Firmware/Esclavo` solo da coincidencias dentro de
`.pio/build/*/firmware.map` y de los objetos del framework de Arduino — **ni una en `src/` ni en
`include/`**. No hay divisor de tension, no hay canal ADC, no hay medida de bateria. El `12.6` es
una constante escrita a mano.

Y el campo `T:` **no es el tiempo de fase**. Es un contador libre que da la vuelta cada minuto,
independiente de en que fase este el cruce. El comentario de al lado dice *"Cuenta de segundos
transcurridos en fase actual"* — y no lo es.

> **Por que esto sube de prioridad con la nueva arquitectura, no baja.** Con pantalla, la
> telemetria era un canal de comodidad. **Sin pantalla es el unico tablero que existe**, y
> `CLAUDE.md` §3.quinquies ya cobro esta leccion en la app: *"lo que sustituye a un dato que no se
> tiene no es una simulacion: es decirlo"*. Un `98%` fijo en el celular de quien decide sobre el
> trafico es de la misma familia que el simulador que se retiro de la pantalla principal — solo que
> este vive en el firmware.
>
> Un campo que no se mide se retira o se marca. **No se deja con aspecto de medida.**

### 2.7 🟢 Del mando de reles no se retira equipo en servicio: nunca se compro el receptor

**ESCRITO**, y coherente en tres sitios:

- `roadmap.md:3357` (N-19): *"El Maestro tiene mando; el Esclavo no. La tarjeta ya tiene las cuatro
  entradas (`PB9`, `PB13`, `PB14`, `PB15`) — **falta solo el receptor**"*.
- `05_Funcional/8_Procedimiento_Modo_Degradado.md:127-128`: *"El Esclavo no tiene receptor de mando
  de relés (pendiente N-19). La tarjeta ya trae las cuatro entradas; **falta comprar e instalar el
  receptor**"*.
- `04_Manuales/MANUAL_MANDO_4_RELES.md:352`: la advertencia de exigir **codigo independiente por
  unidad** *"al comprarlo"*.

**MEDIDO:** en el protocolo de 80 pruebas, la seccion 8 (mando de 4 reles, 8 pruebas) y la 13
(blindaje del mando, 2 pruebas) **no tienen acta firmada**: son parte de las 48 de V9.0, que es la
tanda que `ESTADO.md` declara *"implementada y compilando. NO probada en banco"*.

> **Lo que se retira del mando es codigo y papel, no equipo montado en la calle.** Es el unico de
> los ocho hallazgos que **abarata** la decision, y por eso conviene tenerlo escrito: nadie tiene
> que subir a un poste a desmontar nada.
>
> **La excepcion es §2.4.** El receptor no existe, pero **el veto que el mando aporta en el
> firmware si existe y esta activo**. Retirar codigo que nunca tuvo hardware detras sigue cambiando
> el comportamiento del equipo.

### 2.8 🔴 El protocolo de 80 pruebas: 49 dejan de ser ejecutables

**MEDIDO** cruzando las 13 secciones de `05_Funcional/3_Protocolo_Pruebas_Rigurosas.md` (recuento
de la propia acta, lineas 799-820) contra lo que se retira.

| seccion | pruebas | caen | sobreviven | por que caen |
|---|---|---|---|---|
| 1 — Menu e independencia de radio | 4 | **4** | 0 | las cuatro se observan en la LCD |
| 2 — Perdida de comunicacion / self-healing | 5 | 0 | 5 | se observan en las luces |
| 3 — Modo Automatico | 5 | 0 | 5 | se observan en las luces |
| 4 — Modo Inteligente | 4 | **1** | 3 | 4.1 es *"interfaz dedicada"* |
| 5 — Modo Manual (botonera fisica) | 10 | **10** | 0 | la seccion **es** la botonera y el menu |
| 6 — Repetidor ESP32 | 4 | 0 | 4 * | |
| 7 — Reloj y AJUSTAR HORA | 11 | **11** | 0 | las once pasan por pantalla o botones |
| 8 — Mando de 4 reles | 8 | **8** | 0 | el mando se retira |
| 9 — Modo Degradado | 12 | **7** | 5 * | 9.2-9.6, 9.10, 9.11 prueban gestos de boton |
| 10 — Interfaz propia del Esclavo | 5 | **5** | 0 | la seccion **es** la LCD del Esclavo |
| 11 — Camaras IA | 4 | **1** | 3 | 11.4 es *"independencia de los botones"* |
| 12 — Bluetooth y telemetria | 6 | 0 | 6 * | |
| 13 — Blindaje del mando (N-53) | 2 | **2** | 0 | mando + pantalla AJUSTAR HORA |
| **TOTAL** | **80** | **49** | **31** | |

Las tres columnas con `*` **no sobreviven tal como estan escritas**:

- **Seccion 6 (4):** el repetidor esta **fuera de la configuracion vigente** — `CLAUDE.md` §10:
  *"2 radios en enlace directo, sin repetidor"*. Ya no eran ejecutables antes de esta decision.
- **Seccion 9 (5):** 9.1, 9.7, 9.12, 9.13 y 9.14 **solo sobreviven si existe una via de entrada al
  Degradado desde la app**, y hoy **no existe ninguna** (§2.3: no hay `SET_MODO:DEGRADADO`).
- **Seccion 12 (6):** 12.1 es *"emparejamiento y enlace inalambrico"* con un `HC-05`; hay que
  reescribirla entera para el ESP32.

> **La cuenta que se lleva el auditor: 49 caen, 31 sobreviven en principio, y solo 16 sobreviven
> tal como estan redactadas hoy.**
>
> ⚠️ **Y aqui va una discrepancia que no se tapa.** A este documento se le dio de partida la cifra
> *"~49 caen, sobreviven ~24"*. **La primera se reprodujo exactamente; la segunda no.** Contando
> seccion por seccion salen 31, o 16 si se descuentan las tres columnas con asterisco. **No hay
> ninguna particion razonable que de 24**, y este documento no escribe un numero que no ha podido
> reproducir. `CLAUDE.md` §4: cuando el instrumento y el razonamiento no coinciden, manda la medida
> — y el instrumento aqui es la tabla de arriba, que se puede recontar en cinco minutos.

---

## 3. Decisiones ABIERTAS, con dueno

Ninguna de estas cinco la puede tomar quien escribe firmware. Van con quien las tiene que firmar.

### 3.1 🔴 Que chip es el ESP32 — **bloquea la compra y bloquea la app**

**Dueno: el responsable.** No es una decision tecnica: es que **la app depende de la respuesta**.

**ESCRITO** en `05_Funcional/15_Lista_de_Compras_Hardware.md:79-83` y replicado en
`05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.md:83-84`:

| familia | Bluetooth | ¿la app conecta? | ¿que pasa con el Manual 10 §1? |
|---|---|---|---|
| `ESP32-WROOM-32` / `-32D` / `-32E` / `-32U` (**clasico**) | Clasico (BR/EDR) + BLE | ✅ **si, tal cual** — `createRfcommSocketToServiceRecord` abre | **intacto** |
| `ESP32-S3` · `ESP32-C3` | **solo BLE** | ❌ **nunca** — el socket RFCOMM no abre | 🛑 **hay que reabrirlo por escrito** |
| `ESP32-S2` | **ninguno** (solo WiFi) | ❌ | 🛑 **hay que reabrirlo por escrito** |

El apartado 1 del Manual 10 esta **congelado por escrito** (`10_Manual...:26`, `:146-148`):
*"Bluetooth Clasico SPP. No BLE. No Web Bluetooth. Y no es negociable sin reabrir este apartado por
escrito."* La razon esta medida y pagada: `navigator.bluetooth` **no puede ver un SPP** —la API no
existe para ese perfil— y eso ya costo una version entera de la app.

> **Como se responde, y no admite atajo (`10_Manual...:91`, `:100`):** se lee la **serigrafia del
> blindaje metalico** del modulo. `ESP32-WROOM-32E` es una respuesta; `ESP32-S3-WROOM-1` es otra.
> **El rotulo del vendedor no distingue.**
>
> **SIN VERIFICAR:** nadie ha leido todavia la serigrafia de los modulos que llegaron a obra el
> 28/08. La lista de compras los registra como *"referencia sin confirmar"*
> (`15_Lista...:69`). **Es una comprobacion de treinta segundos con el modulo en la mano y decide
> si hay que rehacer el transporte de la app entero.**

### 3.2 🟠 El cristal `Y2`: se repara, o el STM32 lleva reloj de software

**Dueno: el responsable**, porque una via cuesta taller y la otra cuesta firmware y flash.

| via | que exige | riesgo |
|---|---|---|
| **A — reparar el hardware** | cambiar `C1`/`C2` del `Y2` por 6-10 pF C0G/NP0 (`MAPEO_TARJETA_KICAD.md` §4) | 🟡 *"hipotesis razonable, sin verificar en esta tarjeta"* — lo dice el propio manual |
| **B — reloj de software en el STM32, disciplinado por el ESP32** | un contador propio que el ESP32 pone en hora reenviando la hora del `DS3231` | 🟠 deriva entre reenvios; **y cuelga el reloj del semaforo del modulo accesorio**, que es justo lo que §1.1 separa |

**Antes de decidir hay una medida pendiente que puede ahorrar la compra entera:** `ESTADO.md` `B5`
— *"Diagnostico de los dos cristales `Y2`. Decide todo el bloque C: si el muerto es el del Esclavo,
no se compra nada"*. **N-37 midio uno**; el otro sigue SIN VERIFICAR.

> **Y hay una consecuencia de la via B que conviene tener escrita antes de elegirla:** el Modo
> Degradado exige reloj (`reloj_enHora()`), y §2.5 muestra que hoy el rechazo por falta de reloj es
> silencioso. Un reloj de software que el ESP32 disciplina significa que **si el ESP32 no esta, el
> reloj se va yendo** — y nadie lo ve, porque el unico tablero es el ESP32.

### 3.3 🔴 Sin pantalla, sin pulsadores y sin mando: como se opera el equipo si el ESP32 se cuelga

**Dueno: el responsable.** Es la decision mas grande de las cinco y la unica sin ninguna via
construida.

**Lo MEDIDO:**

```
   01_Firmware/Maestro/src/main.cpp:52    IWatchdog.begin(4000000);     <- 4 s
   01_Firmware/Esclavo/src/main.cpp:238   IWatchdog.begin(4000000);     <- 4 s
   01_Firmware/Repetidor/src/            grep -rn "watchdog|esp_task_wdt|WDT"  ->  CERO coincidencias
```

**El ESP32 de este proyecto no tiene watchdog.** Lo dice tambien `roadmap.md:2706`, en la casilla
de H-3: *"(El Repetidor ESP32 sigue sin watchdog.)"*, y `MAPEO_TARJETA_KICAD.md` §5 lo lista como
ventaja de portar el repetidor al STM32.

**Y hay precedente escrito de que un ESP32 de este proyecto se queda clavado y tumba el enlace.**
`01_Firmware/TROUBLESHOOTING.md:48`: *"Ocurrio en el repetidor el 31/07/2026: el ESP32 levantaba
DE/RE ante cualquier byte y solo lo bajaba..."*, con el sintoma en la tabla de `:55`: **"DE/RE
clavado o ruido continuo. Bus bloqueado en ambos sentidos"**.

**Que pasa hoy si el enlace muere:** los dos STM32 se van a ambar por SFTY-6 a los
`SFTY6_SILENCIO_MS = 25000UL` (**MEDIDO** en `Maestro/include/protocolo.h:149` y
`Esclavo/include/protocolo.h:149`; son 25 s desde N-71, no los 12 s que aun dicen varios
comentarios y `ESTADO.md`).

> **Eso es seguro. No es operable.**
>
> Un cruce en ambar intermitente no mata a nadie, y por eso SFTY-6 esta bien puesto. Pero con la
> pantalla, los pulsadores y el mando retirados, **un ESP32 colgado deja el equipo sin ninguna
> superficie de mando**: no hay boton que pulsar, no hay menu que navegar, no hay mando desde el
> piso. La unica accion disponible es cortar la energia y volver a darla, y eso lo tiene que hacer
> alguien subiendo al gabinete — que es exactamente el viaje que toda esta arquitectura pretende
> evitar.
>
> **Las opciones, para que la decision se tome entre alternativas escritas y no por eliminacion**
> (`CLAUDE.md` §4: eliminar entre opciones incompletas es adivinar con tabla):
>
> | opcion | coste | que resuelve |
> |---|---|---|
> | Watchdog en el ESP32 (`esp_task_wdt`) | bajo, firmware nuevo del ESP32 | el ESP32 colgado se reinicia solo. **No** cubre el ESP32 muerto o desenchufado |
> | Un solo pulsador de servicio superviviente | un pulsador y un pin; **hereda §2.2** | da una salida fisica de ultimo recurso |
> | Dejar el mando de reles del Maestro | cero — **es no retirarlo** | conserva `B·B·B` y el veto de §2.4 |
> | `J2` (SWD) como consola de servicio | 🔴 **descartado**: `MAPEO_TARJETA_KICAD.md` §7 — `J2` es la unica via de carga de firmware, no se reutiliza | — |
> | Aceptar el ambar como estado final y subir al gabinete | cero | nada; es declarar el riesgo, y entonces va firmado |

### 3.4 🟠 El minimo de tiempo por sentido (N75-1)

**Dueno: el responsable.** Es una cifra, y hace falta.

**MEDIDO:** `01_Firmware/Maestro/src/modo_automatico.cpp:31`

```
   static const uint8_t VERDE_MIN_MIN = 1,  VERDE_MIN_MAX = 15;
```

**El firmware admite 1 minuto.** `ESTADO.md:140` (N75-1) lo dice con todas las letras: *"Se pidio
'minimo de 3 minutos'; **no esta escrito en ninguna parte** — el firmware dice `VERDE_MIN_MIN = 1`
y la app valida exactamente lo mismo. No hay desajuste: hay una decision sin tomar, y su sitio es
el C++"*.

> **Y su hermana, N75-2, que es la que hace que esto vuelva:** los cuatro limites estan escritos
> **dos veces** —en `modo_automatico.cpp:31-33` y a mano en `app.js:734` mas los `min`/`max` del
> formulario— **sin nada que los ate**. Hoy coinciden. El dia que suba el minimo, si nadie ata las
> dos copias, **la app seguira dejando poner 1**. La decision del responsable es el numero; atarlo
> es media hora de trabajo tecnico y va en el mismo commit.

### 3.5 🟡 La Camara 1: se queda en `PB0`/`J14`, o se muda a `J16`

**Dueno: quien monte**, con el visto bueno tecnico.

| via | a favor | en contra |
|---|---|---|
| **Quedarse en `PB0` / `J14`** | 🟢 **es el unico camino de camara con firmware probado**: N-67 corregido, `pinMode(INPUT)` y `== HIGH` en las dos puntas, pack `camara_01_demanda` con 14 comprobaciones, y la placa da antirrebote de 1 ms por hardware (`R64` + `C25`) | dos borneras distintas para dos camaras del mismo poste |
| **Mudarla a `J16` p12** | prolijidad de montaje: las dos camaras en el mismo conector | 🔴 **hereda §2.2 sin resolver** y 🔴 **hereda §2.1**: acerca la camara a los 12 V |

> **Este documento no toma la decision, pero deja escrito el sesgo:** mover una funcion que
> **funciona con firmware probado** a un conector cuya polaridad esta en contradiccion medida y que
> reparte 12 V, para ganar prolijidad de montaje, es cambiar riesgo por estetica. Si se muda, se
> muda **despues** de la medida M3 de la seccion A, nunca antes.

---

## A. Las cinco medidas de multimetro, en orden

> 🔴 **El motivo por el que esta seccion existe: hoy no hay ni una fila «VERIFICADO EN LA PLACA» en
> todo el mapeo de la tarjeta.** `MAPEO_TARJETA_KICAD.md` §0 y §9 lo declaran, y sigue siendo cierto
> el 28/08. Todo lo que sabemos del cobre sale de un dibujo.

**Las tres primeras van con la tarjeta SIN ENERGIA. Las dos ultimas con energia, y antes de unir los
dos equipos.** Cada una se anota en `MAPEO_TARJETA_KICAD.md` §9 con la fecha, para que empiece a
haber filas de ese nivel.

### M1 · Cual de los dos conectores es `J16` y cual `J17`

**Por que es la primera:** `J16` y `J17` **comparten footprint** —`Molex_KK-254_AE-6410-16A_1x16`,
16 pads los dos, MEDIDO en el `.kicad_pcb`— y a la vista son identicos. Lo avisa
`10_Manual...:250-256`. **`J16` reparte 12 V; `J17` no.** Confundirlos es meter 12 V donde va el
ESP32.

| que se mide | como | esperado |
|---|---|---|
| p1 del conector contra el borne **positivo** de `J1` | continuidad (pito) | **`J16` PITA · `J17` NO** |
| p7 y p9 contra `GND` | continuidad | **pitan en `J17`** |

🔴 **Si pitan las dos cosas en el mismo conector, se para.** El mapa no describe esta placa y nada
de lo que sigue vale.

### M2 · Que `J17` no tiene 12 V en ninguna posicion, y que p2/p3 son `PB7`/`PB6`

Es la medida que protege al ESP32 y a las patas 42 y 43 del `U1`.

| que se mide | como | esperado |
|---|---|---|
| `J17` p2 al pin **43** del `U1` | continuidad | **pita** (`PB7`) |
| `J17` p3 al pin **42** del `U1` | continuidad | **pita** (`PB6`) |
| cada posicion de `J17` contra `GND`, **con energia** | tension DC | **0 V o 3,3 V, y ni una por encima**. `J17` p6 y p8 = 3,3 V |

🔴 **Si aparece 12 V en cualquier posicion de `J17`, el ESP32 no se enchufa** hasta aclararlo.

### M3 · La polaridad de los pines de boton — **la que desbloquea las camaras**

Es la medida que cierra la contradiccion de §2.2. Sin ella no se cablea camara a `J16`.

Con la tarjeta **sin energia** y **nada enchufado en `J16`**, ohmimetro:

| que se mide | esperado si la placa es la del netlist | esperado si el firmware tiene razon |
|---|---|---|
| `J16` p5 (`PB9`) a `J16` p2 (`GND`) | **10 kOhm** | circuito abierto |
| `J16` p5 a `J16` p4 (`3,3 V`) | circuito abierto | **10 kOhm** |

Repetir en **p8, p10 y p12**. Los cuatro tienen que dar lo mismo; si uno difiere, ese es el
hallazgo.

Y despues, **con energia** y `J16` vacio, tension de p5/p8/p10/p12 contra `GND`:

| lectura | que significa |
|---|---|
| **~0,66 V** | pull-DOWN de 10 k contra el pull-up interno de ~40 k. **El netlist tiene razon y el firmware esta invertido** — es N-67 en los botones |
| **~3,3 V** | pull-UP. El firmware tiene razon y el netlist no describe esta placa |
| otra cosa | ni una ni otra: se anota el numero y se para |

> **`0,66 V` es una cuenta, no una medida:** `3,3 x 10/(10+40)`, con el pull-up interno tipico del
> STM32F103. El umbral `VIL` del micro es `0,3 x VDD = 0,99 V`, asi que `0,66 V` se lee **LOW**. La
> cuenta es de `roadmap.md` N-67 y se reproduce aqui para que el que mide sepa **que numero espera
> antes de mirar la pantalla del multimetro**, que es la unica forma de que la medida signifique
> algo.

### M4 · Los 12 V de `J16` p1, y cuanto hay entre ellos y los pines de camara

Con energia:

| que se mide | esperado |
|---|---|
| `J16` p1 contra `GND` | **12 V** (el rail crudo — sin opto, sin limitadora, sin clamp) |
| `J16` p2 contra `GND` | 0 V |
| `J16` p4/p7/p9/p11 contra `GND` | 3,3 V |

Y **antes de energizar con algo enchufado**, `p1` va tapado (§2.1). La separacion fisica MEDIDA con
el paso de 2,54 mm del footprint: **10,2 mm** a `p5`, **22,9 mm** a `p10`, **27,9 mm** a `p12`.

### M5 · La masa comun del ESP32 y el nivel de reposo de su TX

Con **las dos fuentes encendidas** y **los hilos de datos todavia sin unir**:

| que se mide | como | esperado |
|---|---|---|
| masa del ESP32 contra masa de la tarjeta | tension DC | **< 50 mV** |
| `GPIO17` (TX2) del ESP32 contra la masa comun | tension DC | **3,3 V** (linea serie en reposo alta) |

🔴 **Si hay tension apreciable entre las dos masas, no se unen los datos.** Esa diferencia entra
entera por `PB6`/`PB7`, que son patas del micro que gobierna el semaforo. Y si `GPIO17` en reposo
diera **5 V**, el modulo no es el que se cree que es: se para y se identifica antes de conectar.

> **Lo que estas cinco medidas NO cubren, y va escrito al lado:** ninguna de ellas dice nada del
> `Y2` de cada tarjeta (§3.2, `ESTADO.md` `B5`), ni de a donde sale de verdad `PB2`
> (`ESTADO.md` `B3`: *"que `Puerta` salga del pin `MOTOR_TALANQUERA` y llegue al borne"*), ni del
> nombre real del pin 3 de `J17` (§1.4). Son tres comprobaciones mas, con la tarjeta delante, y
> **no se resuelven con multimetro sino siguiendo hilos y leyendo serigrafias.**

---

## B. Que documentos quedan FALSOS, y en que orden hay que tocarlos

El criterio del orden no es el gusto: **primero lo que hace salir dinero, despues lo que promete una
salida de emergencia que ya no existe, al final el acta que se firma.**

> **Ninguno de estos ficheros se ha tocado al escribir este documento.** Lo que sigue es el censo,
> no el arreglo.

### Orden 1 · 🔴 `05_Funcional/15_Lista_de_Compras_Hardware.md` — **hay dinero a punto de salir**

**Va primero porque es el unico de la lista cuyo dano es irreversible en cuanto alguien pague.**

| linea | que manda comprar | por que es falso ahora |
|---|---|---|
| `:110`, `:127`, `:213` | **2 modulos Bluetooth SPP `HC-05`/`JDY-30`** | **el modulo SPP se retira.** Lo sustituye el ESP32 |
| `:217-218` | *"Los modulos Bluetooth siguen en la lista aunque hayan llegado ESP32. **Decision de obra del 28/08: se va con el modulo SPP dedicado**"* | 🔴 **es la decision contraria a la del 28/08.** El mismo dia, el mismo documento |
| `:164`, `:222` | **`DS3231` `ZS-042`** *"solo si el cristal muerto es el del Maestro"*, para la placa | el `DS3231` ya no va a la placa: **va al ESP32** por `GPIO21`/`GPIO22`. Cambia de destino, de manual y de criterio de compra |
| `:206` | fila del repetidor ESP32 | hay que separar tres ESP32 distintos en el papel: el **puente de radio** del Manual 5, los **modulos llegados el 28/08** y el **ESP32 de expansion** de este documento. Hoy dos de los tres se llaman igual |

Lo que **sigue siendo cierto** en ese documento y no hay que tocar: las 2 camaras AcuSense de
demanda, las 2 antenas VHF con sus coaxiales, y el aviso de `:180-181` de que **el `DS3231` no tiene
driver en el firmware de hoy** — eso es cierto y se vuelve mas importante, no menos, porque ahora el
driver hay que escribirlo en el ESP32.

### Orden 2 · 🔴 `05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.md` — congelado, y manda enchufar un `HC-05` en `J17`

| linea | que dice | por que es falso ahora |
|---|---|---|
| `:132-135` | *"**Decision de obra del 28/08: se sigue con el modulo SPP dedicado.** Se instala `HC-05`/`JDY-30`, no ESP32"* | 🔴 contraria a la arquitectura de este documento |
| `:221-241` | el cableado del **`HC-05` a `J17`** p2/p3 | **`J17` p2/p3 es donde va el ESP32.** El pinout es correcto; el modulo que manda enchufar, no |
| `:142-146` | la tabla de tres caminos con su columna *"Estado de este apartado 1"* | **hay que decidir la fila y anotarlo**, no elegirla en silencio |

> 🛑 **El apartado 1 esta congelado por escrito (`:26`, `:148`) y su reapertura es OBLIGATORIA si el
> modulo no es un ESP32 clasico.** No es burocracia: el valor de una decision congelada esta en que
> reabrirla cueste. Si los modulos son `S3`, `C3` o `S2`, **hay que rehacer el transporte de la app
> entero** y eso se decide antes de comprar y antes de escribir una linea. Ver §3.1.
>
> **Y hay una via intermedia que el propio manual ya deja abierta (`:145`): un ESP32 clasico
> haciendo de puente SPP deja el apartado 1 INTACTO.** Si la serigrafia dice `WROOM-32*`, esta
> arquitectura entra **sin reabrir nada** — solo hay que confirmar la referencia y la fuente propia.
> Es la razon practica por la que §3.1 va antes que todo lo demas.

### Orden 3 · 🔴 `05_Funcional/8_Procedimiento_Modo_Degradado.md` — sus cuatro vias son botones retirados

**MEDIDO** sobre el documento: la entrada desde el piso es `A · B · A · B` en menos de 18 s
(`:119`, `:295`); la salida a Automatico es `A · A · A` (`:296`); la vuelta a ambar es `B · B · B`
(`:297`); y la entrada por pantalla usa los pulsadores. **Las cuatro desaparecen.**

Y la tabla de averias del `:311-312` manda `B·B·B` como **primera accion** ante los dos escenarios
de riesgo residual —las dos puntas desfasadas, y una en verde con la otra en ambar—. Con el mando
retirado, ese procedimiento **no tiene accion que ejecutar**.

> **Este documento no se puede «actualizar» hasta que exista la via de sustitucion.** Hoy **no hay
> ningun comando de Bluetooth que entre ni salga del Modo Degradado** (§2.3). Reescribirlo antes de
> que exista seria describir un procedimiento que nadie puede ejecutar — que es peor que dejarlo
> desfasado, porque un procedimiento desfasado se nota y uno inventado no.

### Orden 4 · 🔴 `04_Manuales/MANUAL_MANDO_4_RELES.md` — vende `B·B·B` como salida de emergencia

`:316-318`: *"**`B·B·B` devuelve a ambar desde cualquier estado en marcha, sin condiciones.** ... una
salida de emergencia con requisitos no es una salida de emergencia"*.

Es una promesa fuerte y esta bien argumentada. **Con el mando retirado deja de existir**, y su
desaparicion es exactamente §3.3: el equipo se queda sin salida de emergencia fisica.

Ademas `:96-102` documenta el veto de §2.4 —*"En el Esclavo, `B·B·B` desobedece al Maestro a
proposito"*—, que es la parte que **no** queda inerte al retirar el mando.

> **Este manual no se borra: se marca retirado y se conserva.** `CLAUDE.md` §9 del propio mapeo lo
> dice para las correcciones y vale igual aqui — *"no se borra lo que estaba mal"*, y menos lo que
> estaba bien y se retiro. La razon por la que `B·B·B` existia sigue siendo valida el dia que
> alguien pregunte *"¿y si el ESP32 se cuelga?"*.

### Orden 5 · 🔴 `05_Funcional/3_Protocolo_Pruebas_Rigurosas.md` — **el acta que se firma**

Va el ultimo de los urgentes **a proposito**: es el documento que recoge las consecuencias de los
otros cuatro, y reescribirlo antes de que los otros esten decididos garantiza reescribirlo dos
veces.

**49 de sus 80 pruebas dejan de ser ejecutables** (§2.8). El bloque de `RESUMEN DE RESULTADOS`
(`:799-820`) tiene los totales por seccion escritos a mano, y **el `TOTAL ___ / 80` es lo que se
firma**. Un acta con 49 casillas que nadie puede rellenar no es un acta incompleta: es un acta que
invita a rellenarlas igual.

> ⚠️ **Y la trampa concreta que hay que evitar, porque este repositorio ya la tiene escrita
> (`CLAUDE.md` §8.quater):** al reescribirlo, la tentacion es tachar las 49 en bloque hasta que la
> cuenta cuadre. Van una por una, y cada una acaba en uno de tres sitios, **anotado**: **se borra**
> (solo probaba el gesto retirado), **se invierte** (pasa a exigir el comportamiento nuevo por la
> app), o **se conserva** (media otra cosa y sigue valiendo). La 11.4 —*"Inmunidad e Independencia
> de los Botones del Panel LCD"*— es el caso mas claro de **inversion**: hoy exige que las camaras
> no interfieran con los botones; manana tiene que exigir que las camaras **funcionen en los pines
> que eran de los botones**.

### Segundo bloque · lo que tambien queda falso, sin dinero de por medio

| documento | que queda falso |
|---|---|
| `03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md` | **§6, §7:** `J16` deja de ser el conector de botones. **Y ver el recuadro de abajo: su §0 esta desmentido por medida** |
| `05_Funcional/1_Manual_Usuario.md` | toda la operacion por pantalla y botones; camaras en `PB0`/`PB8` |
| `05_Funcional/2_Manual_Hardware_y_Pruebas.md` | ensamblaje, borneras, pila RTC en `VBAT` |
| `05_Funcional/9_Manual_Parametrizacion_Camara_IA.md` | contactos `1A`/`1B` en `PB0` y `PB8` |
| `04_Manuales/MANUAL_CONFIGURACION_CAMARAS_IA.md` | el pinout de camaras |
| `05_Funcional/11_Manual_Instalacion_RTC_DS3231_Bateria.md` | el `DS3231` se muda al ESP32 |
| `05_Funcional/13_Manual_Modulo_Expansion_I2C_y_Compras.md` | su §4 entera: las rutas de bus I2C sobre el STM32 dejan de hacer falta si el I2C vive en el ESP32 |
| `04_Manuales/MANUAL_CONFIGURACION_BLUETOOTH.md` | comandos AT de `HC-05` |
| `05_Funcional/14_Manual_App_Movil_IOT_VIAL.md` | la app pasa de accesorio a **unica interfaz** |
| `ESTADO.md` | §2 dice `USART1` en `PA9`/`PA10` (ya es `PB6`/`PB7`) y *"SFTY-6 a los 12s"* (son **25 s**, `protocolo.h:149`) |
| `OPTIMIZACIONES.md` | la trazabilidad `SFTY-x -> codigo -> prueba`, sobre todo SFTY-21 (§2.4) |
| `CLAUDE.md` | §6 sigue valiendo entera; §10 y el mapa de instrumentos hay que revisarlos |

> 🟢 **Y un hallazgo MEDIDO sobre el mapeo, que va aqui porque cambia el peso de la evidencia de
> este documento:**
>
> `MAPEO_TARJETA_KICAD.md` §0 afirma: *"El `.kicad_pcb` de este proyecto **esta vacio**, asi que
> entre el esquematico y la tarjeta que hay encima de la mesa no existe ningun artefacto que las
> ate"*. **Es falso para el fichero bueno.** Censo del
> `01_Firmware/Controladora_Semaforos/Controladora_Semaforos/Controladora_Semaforos.kicad_pcb`
> (2 158 421 bytes):
>
> | | |
> |---|---|
> | `(footprint` | **185** |
> | `(segment` | **1 447** |
> | `(via` | **89** |
> | `(zone` | **2** |
>
> **Es una placa ruteada, no un fichero vacio.** Los `.kicad_pcb` vacios de verdad —**78 bytes**—
> estan en `99_Legacy/Controladora_Semaforos-backups/`. Es la regla del instrumento otra vez: **el
> buscador miro el fichero equivocado**, igual que en N-64. Y `ESTADO.md` **ya lo sabe** —su mapa de
> artefactos dice *"649 KB ... y el `.kicad_pcb` de 2,1 MB"*—: los dos documentos se contradicen
> hoy.
>
> **Lo que esto NO significa:** que el netlist describa la placa que hay sobre la mesa. Un `.pcb`
> ruteado dice lo que se **envio a fabricar**; sigue sin decir lo que alguien **reparo despues**, ni
> si la unidad de banco salio de esa tirada. **La seccion A sigue siendo obligatoria.** Lo que
> cambia es que las medidas de §2.1 y §2.2 dejan de salir de un dibujo y salen de un **netlist
> ruteado**, que es un escalon mas arriba — y por eso la contradiccion de §2.2 pesa mas, no menos.

---

## C. Lo que este documento NO mide, y nadie debe dar por medido

Se escribe explicito porque un documento de arquitectura que no marca sus bordes se lee como un
permiso.

| | |
|---|---|
| **Nada del cobre** | ni una fila «VERIFICADO EN LA PLACA». Todo el hardware de aqui es netlist y esquematico |
| **El chip que llego a obra** | nadie ha leido la serigrafia (§3.1). **Es lo mas barato y lo mas bloqueante de la lista** |
| **El pico de 500 mA del ESP32** | ESCRITO en el Manual 15, no medido sobre el modulo real |
| **Que el enlace `J17` funcione** | `13_Manual...:99` lo dice mejor de lo que se puede decir aqui: *"tampoco esta verificado en banco el enlace Bluetooth sobre `J17` p2/p3: la compuerta paso, y la compuerta no toca la tarjeta"* |
| **El `Y2` de la segunda tarjeta** | N-37 midio uno. El otro sigue sin diagnosticar (`ESTADO.md` `B5`) |
| **Que las camaras funcionen en `PB14`/`PB15`** | esos dos pines **nunca han tenido una camara conectada**. `ESTADO.md`, fila `BANCO`: *"nadie ha cableado nunca esos pines"* |
| **El firmware del ESP32** | **no existe**. No hay driver de `DS3231` en ninguna punta —medido el 28/08 con `grep -rniE "DS3231\|Wire\.\|0x68"`, `13_Manual...:56`— ni codigo de ESP32 para esta funcion |
| **La regresion N-42** | el Modo Automatico no mueve las luces en banco, y **sigue abierta** (`ESTADO.md` `B2`). Es anterior a esta arquitectura y no se cierra con ella |

> 🛑 **La compuerta del 28/08 salio con `15 PASS | 0 FALLA | 0 ABORTADO` y eso no autoriza nada de
> este documento.** Lo dice el acta y lo dice `CLAUDE.md` §3: ese `0` significa que *los modelos y
> los arneses de PC no encuentran nada*. **Ninguno de ellos toca la tarjeta**, ninguno tiene
> bornera, y ninguno sabe si el cobre de la tarjeta es el del plano. **Verde no
> es entregable.**

---

## Anexo · Cambios que otros ficheros necesitan y que este documento NO ha hecho

Ninguno de estos se ha tocado. Es la lista de trabajo, no el trabajo.

**Firmware — y estos tres van antes de retirar nada:**

1. **`SET_MODO:MENU`** en `Maestro/src/bluetooth.cpp`, y su boton en la app. **Antes** de ignorar
   los pulsadores (§2.3).
2. **Quien hereda el veto de `mando_ambarLocal()`** en `Esclavo/src/main.cpp:401`, `:408`, `:526`.
   **Antes** de retirar `mando.cpp` (§2.4).
3. **`SET_RTC` tiene que mirar `reloj_hayCristal()`** antes de contestar `RESULT:OK`
   (`bluetooth.cpp:175`, `reloj.cpp:290`) (§2.5).
4. **La telemetria fabricada:** `BAT:12.6` en las dos puntas, y `RF:98%` / `RTT:85ms` /
   `MODO:SUBORDINADO` en el Esclavo. Se retiran o se marcan; no se dejan con aspecto de medida
   (§2.6). Y el campo `T:` no es tiempo de fase — el comentario de `bluetooth.cpp:241` dice que si.
5. **La polaridad de `botones.cpp`** en las dos puntas, si M3 dice que el netlist tiene razon
   (§2.2). Con su pack, como N-67 tuvo el suyo.
6. **`VERDE_MIN_MIN`** (`modo_automatico.cpp:31`) cuando llegue el numero, **atado a la app** en el
   mismo commit (N75-1 y N75-2, §3.4).
7. **Comentarios que ya mienten:** `Esclavo/src/main.cpp:399` dice *"cae a C_FALLO en ~12,5 s"* con
   `SFTY6_SILENCIO_MS` en 25 000. Es lo que `CLAUDE.md` avisa de los comentarios que sobreviven a un
   numero.

**Instrumentos** — y esta es la parte que impide que todo esto vuelva:

8. **Los packs que ejercen pantalla, botones y mando quedan sin sujeto.** Hay que decidir uno por
   uno si se borran, se invierten o se conservan (`CLAUDE.md` §8.quater), **con la cuenta comparada
   antes y despues**, que es la unica red para esta clase de deriva (§5 de `CLAUDE.md`).
9. **Un pack que ate la polaridad de los cuatro pines de boton al netlist**, como
   `camara_01_demanda` ato la de `PB0`.
10. **Un pack que ate `VERDE_MIN_MIN` del `.cpp` a los limites de `app.js`** (N75-2).
11. **Los tres packs `documentos_*`** vigilan lo que README, `ESTADO.md`, `OPTIMIZACIONES.md` y el
    Manual 10 **dicen haber medido**. Cambiar el Manual 10 sin mirarlos deja el banco en rojo — o,
    peor, en verde vigilando una frase que ya no esta.

**Documentos:** los de la seccion B, en el orden de la seccion B.

---

*Escrito el 28/08/2026. Todo lo marcado MEDIDO se puede repetir abriendo el fichero y la linea que
se cita. Lo marcado ESCRITO tiene su fuente al lado. Lo marcado SIN VERIFICAR no lo ha comprobado
nadie — ni aqui ni en ningun otro sitio de este repositorio.*
