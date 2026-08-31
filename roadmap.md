# Roadmap — Controladora de Semaforos Moviles de 3 Estados (V9.0)

**Arranca el 31 de Agosto de 2026.** Este fichero lleva **el estado de lo que tenemos**, no una
bitacora. Lo anterior no se pierde —vive en el `git log` de este repositorio y en el remoto
`padre`—, pero no se arrastra aqui: lo que no sirve para decidir hoy, no esta.

> **Como se lee.** Arriba, **lo que hay y lo que esta decidido**. Abajo, los `N-x` de esta sesion,
> que son **el porque** de cada decision con su medida al lado. Un roadmap sin el porque obliga a
> volver a descubrirlo, y en este proyecto eso ya se pago tres veces.

---

## 1. Que hay hoy

| | |
|---|---|
| **En campo** | **V8.4**, commit `e303485` (31/07/2026), validada por el funcional |
| **En el repositorio** | **V9.0** — implementada y compilando. **NO probada en banco** |
| **Compuerta** | ✅ 15 PASS · 0 FALLA · 0 ABORTADO — acta `evidencia/2026-08-31_compuerta.txt` |
| **Flash** | Maestro **88,3 %** (57.880 de 65.536 B, **7.656 B libres**) · Esclavo 64,4 % · Repetidor 20,6 % |
| **Banco** | 445/445 en 39 packs · 271/271 pantalla · 71/71 automatico · 29/29 ciclo · app 32/32 + 61/61 + 58/58 |

> 🛑 **Verde no es entregable.** Ese `0` significa que los modelos y los arneses de PC no encuentran
> nada. **Ninguno toca la tarjeta.** Nada sube a campo sin pasar banco.

> 🔴 **Regresion abierta:** el Modo Automatico no mueve las luces en banco. Es anterior a esta
> arquitectura y no se cierra con ella.

---

## 2. La arquitectura vigente, y POR QUE es esta

**La razon de fondo no es de firmware: este PCB no permite ampliacion.** A diferencia del proyecto
anterior, lo que se desarrollo para ampliarlo **no era fisicamente realizable** —exigia soldar sobre
una placa que no lo admite bien—. No habia de donde sacar pines, y la unica fuente disponible era
retirar funciones. Todo lo que sigue cuelga de ahi.

```
                       fuente propia 12 V (NO sale de la tarjeta)
                                    |
   +--------------------+     +-----v--------------------+
   |   STM32F103C8      |     |         ESP32            |
   |   (controlador)    |     |   (modulo de expansion)  |
   |                    |     |                          |
   |  6 luces  J3-J8    |     |  DS3231  GPIO21 SDA      |
   |  barrera  J15  PB2 |     |          GPIO22 SCL      |
   |  camaras  J16      |     |          (pila propia)   |
   |  LoRa     J12      |     |                          |
   |  mando A/B J16     |     |  Bluetooth (sustituye    |
   |                    |     |   al modulo SPP)         |
   |  PB6 TX == J17 p3 <------ GPIO16 (RX2)              |
   |  PB7 RX == J17 p2 ------> GPIO17 (TX2)              |
   +---------|----------+     +-----------|--------------+
             +-------- masa comun --------+
                       9600 8N1
```

**El STM32 manda sobre las luces; el ESP32 no.** La barrera de salidas de `CLAUDE.md` §6 no cambia.

### El mapa de pines, MEDIDO contra `pines.h` y contra `src/`

| pin | funcion | bornera | estado |
|---|---|---|---|
| `PA0`-`PA5` | 6 luces | `J3`-`J8` | vivo |
| `PA6` `PA7` | peatonal rojo/verde | `J11` `J9` | 🔴 **declarado y MUERTO** en las dos puntas |
| `PB1` | buzzer | `J13` | 🔴 **declarado y MUERTO** en las dos puntas |
| `PB2` | barrera | `J15` | vivo |
| `PB0` | camara de demanda | `J14` | vivo *(solo dentro del Modo Inteligente en el Maestro)* |
| `PB8` | `LED_TESTIGO` | LED `D5` | `INPUT` sin lectura — **deliberado y documentado** |
| `PB9` | **MANDO A** (`BOTON1`) | `J16` p5 | ✅ **SE QUEDA** |
| `PB13` | **MANDO B** (`BOTON2`) | `J16` p8 | ✅ **SE QUEDA** — arma el veto de SFTY-21 |
| `PB14` | `BOTON3` hoy | `J16` p10 | → **camara**, tras retirar C |
| `PB15` | `BOTON4` hoy | `J16` p12 | → **camara**, tras retirar D |
| `PB6` `PB7` | enlace ESP32 | `J17` p3/p2 | vivo — `SerialBT(PB7, PB6)` |
| `PB3` `PB4` `PB5` | SPI de la pantalla | — | **quedan LIBRES al retirar la LCD** |
| `PB10`-`PB12` | radio LoRa | `J12` | vivo |
| `PA8` | desacoplo `U2` | — | vivo, con justificacion caducada (N-95) |
| `PA9` `PA10` | `RS485_IN` | — | declarados y muertos desde N-76 |

**El reloj no cuesta ni un pin del STM32:** el `DS3231` cuelga del ESP32.

---

## 3. Lo decidido, con fecha

| decision | cuando | consecuencia |
|---|---|---|
| **El ESP32 sustituye al modulo SPP dedicado** y se lleva ademas el reloj | 28/08 | ya no se compran `HC-05`/`JDY-30` |
| **Se retira la pantalla LCD** de las dos puntas | 28/08 | libera `PB6`/`PB7` para el Bluetooth y `PB3`/`PB4`/`PB5` de margen, mas ~18,9 KB de flash |
| **El `DS3231` sale del STM32** y cuelga del ESP32 | 28/08 | la linea `PIN-0` queda ANULADA: el I2C ya no vive en el STM32 |
| **El mando de reles se CONSERVA en A y B**; se retiran C y D | **31/08** | `A·A·A`, `B·B·B` y `A·B·A·B` sobreviven, el veto de SFTY-21 no desaparece, y `PB14`/`PB15` quedan para las camaras — **ver N-104** |
| **El modulo es un `ESP32-WROOM-32` clasico: hay SPP** | **31/08** | la app conecta sin tocar el transporte; el apartado 1 del Manual 10 queda intacto; la alimentacion es `12 V -> DC-DC conmutado -> 5 V -> VIN` |
| **Las camaras entran por `J16` p10 y p12** | **31/08** | se leen por el camino de camara (`INPUT` + activo en ALTO), no por el de boton |

---

## 4. Lo que esta ABIERTO, y de quien es

### Del responsable — no se destraban con mas analisis

| # | que | como se cierra |
|---|---|---|
| ~~**BLQ-1**~~ | 🟢 **CERRADO el 31/08.** Es un **`ESP32-WROOM-32` clasico**: `Xtensa LX6 dual-core` y `Bluetooth v4.2 **BR/EDR** + BLE` — hay **SPP**. La app conecta sin tocar el transporte y el apartado 1 del Manual 10 **no se reabre**. Ver **N-107** | — |
| **M3** | 🟠 **La resistencia real de `PB14`/`PB15` en cobre.** Con `pinMode(INPUT)` pelado, sin pull-down real el pin flota y da demandas fantasma | multimetro. Ya **no** es bloqueante de rehacer nada: decide **como se configura la salida de la camara** |
| **A5** | 🔴 **La fuente propia del ESP32 desde 12 V.** No esta pedida y hace falta | comprarla |
| **N75-1** | 🟠 El minimo de tiempo por sentido | es una cifra, y hace falta |
| **APK** | 🔴 Recompilar: la del disco esta caducada y el paquete de entrega **aborta con exit 2** | skill `entregar` §2.bis |

### Tecnico — se puede hacer ya

| # | que | notas |
|---|---|---|
| **T1** | 🔴 **Los documentos peligrosos**: Manual 11 (manda cablear I2C contra la camara y un LED), Manual 10 (manda el modulo equivocado) y el HTML de cableado (viaja en el paquete de entrega) | **daño fisico y dinero.** Va primero |
| **T2** | 🔴 **Blindar los instrumentos antes de tocar firmware**: la etiqueta `# EJERCE` que falta, el `TOTAL_PACKS` que no sabe fallar, el pack del transporte, y el rol del ESP32 en la guarda de rutas | **la compuerta ve lo que se BORRA y no lo que se queda sin sujeto** (N-103) |
| **T3** | 🔴 **Los cinco `MEDIDO` caducados** | hacen reimplementar trabajo ya hecho (N-100) |
| **T4** | 🟠 **Firmware del ESP32**: watchdog primero, luego `DS3231`. No dependen de BLQ-1 | el watchdog con su desigualdad en un pack: periodo **<** `SFTY6_SILENCIO_MS = 25000UL` |
| **T5** | 🟠 **Fases 2 y 3** del firmware STM32 | 🔴 `compilar.ps1` y los stubs de `Validacion_Automatico` **en el mismo commit** que toque `mando.cpp` (N-101) |
| **T6** | 🛑 **BANCO** | sigue siendo EL bloqueante y nada lo sustituye |

---

## 5. El orden de arranque — que se lanza, cuando, y que abre cada puerta

**La regla que fija el orden, y no es de gusto:** hoy la compuerta **ve lo que se BORRA y no ve lo
que se queda sin sujeto** (N-103). Si el firmware se mueve antes que los instrumentos, hay una
ventana en la que decenas de comprobaciones dan verde midiendo codigo que ya no corre. Por eso los
instrumentos van **antes**, no despues.

### Ola A — lo que puede hacer daño hoy 🔴

Va primero porque **no depende de nada** y es lo unico con daño fisico o dinero detras.

| | que | por que aqui |
|---|---|---|
| **A1** | Los cuatro documentos de **N-105**: `MANUAL_USUARIO.md:66-70`, `04_Manuales/MANUAL_INSTALACION_RELOJ_DS3231.md`, `MANUAL_HARDWARE.md:63,66`, `9_Manual_Parametrizacion_Camara_IA.md:64,168` | mandan cablear camaras sobre `PB9`/`PB13` —el mando— y el I2C sobre la entrada de camara y un LED |
| **A2** | El **Manual 17** y **`ESTADO.md`**: llevan la decision anterior *(«se retiran los cuatro pulsadores»)*, y `17_:152-153` manda dejar `p5`/`p8` vacios, que es justo el mando que se conserva | `ESTADO.md:104` ejecutado literal **borra `ambarLocal` y el veto de SFTY-21** |
| **A3** | Los documentos que dan **`FORZAR_ROJO` por valido en el Esclavo** | es el boton de panico: el operario cree que paro el trafico y no paro nada |
| **A4** | Las **cifras sin vigilante**: `MANUAL_USUARIO.md:21` publica despeje de *5 a 999 s* cuando son `DESPEJE_SEG_MIN=10, MAX=90` en un `uint8_t` — 999 nunca fue representable. Y `CERTIFICACION_SW.md` publica 65,0 % de flash cuando son 88,3 % | quien planifique cree tener 23 KB y quedan 7.656 B |

### Ola B — la red, antes de tocar firmware 🔴

| | que | por que aqui |
|---|---|---|
| **B1** | El **`TOTAL_PACKS`** de `documentos_01`: se comprueba como numero suelto y **casa por accidente con el hash `50a5380` del README** | esta demostrado que da falso verde. Se arregla anclandolo a la frase, y se ve caer |
| **B2** | El **rol del ESP32** en `_ROLES` de `compuerta.py`, y su compilacion | sin el, el fuente del ESP32 es **invisible** para la guarda y el acta no tiene una fila donde echarlo de menos |
| **B3** | El **pack de N-106**: que el ambar de la app saque al Esclavo del Degradado | tiene que **fallar** sobre el firmware de hoy antes de que nadie lo arregle |
| **B4** | Un **`documentos_04`** que vigile los manuales que hoy no parsea nadie | `documentos_01` solo mira `README.md` y `ESTADO.md`, y ahi no estan las cifras malas |

### Ola C — firmware

| | que | notas |
|---|---|---|
| **C1** | **Camaras en C y D, LAS DOS PUNTAS EN UN SOLO AGENTE** | dos agentes en paralelo sobre la misma regla es como divergen: es SFTY-2 con el `amarillo = false` de mas, y es N-97. Cierra N-97 de paso unificando como se lee la camara |
| **C2** | **N-106**: la llamada que falta en `Esclavo/src/bluetooth.cpp` | despues de B3, no antes |
| **C3** | **ESP32: watchdog primero, luego `DS3231`** | no dependen de BLQ-1. El watchdog con su desigualdad en un pack: periodo **<** `SFTY6_SILENCIO_MS = 25000UL`, recalculada del C++ |

### Ola D — la pantalla

Sola, con el arbol quieto. Toca `lcd.cpp`, `menu.cpp`, `Validacion_LCD` (271 comprobaciones) y tres
packs, y cada prueba afectada va a **se borra / se invierte / se conserva**, una por una y anotada.
Libera ~18,9 KB y `PB3`/`PB4`/`PB5`.

### Ola E — BANCO

🛑 **Sigue siendo EL bloqueante y nada lo sustituye.** Ni la compuerta en verde, ni los arneses que
compilan C++ real, ni este roadmap.

### Lo que NO desbloquea ningun agente

| | quien | coste |
|---|---|---|
| ~~**BLQ-1** la serigrafia del ESP32~~ | ✅ **cerrado el 31/08** | era `WROOM-32` clasico. Queda una pregunta mucho menor: **30 o 38 pines** de la NodeMCU, para las hembrillas de la placa — pie de rey, y no bloquea firmware |
| **M3** el pull-down real de `PB14`/`PB15` en cobre | funcional | multimetro. Decide **como se configura la camara** y en que pin va cada una: `p10` tiene 4,27 mm contra los 12 V y `p12` solo **1,36 mm** |
| **A5** la fuente propia del ESP32 | responsable | no esta pedida |
| el **receptor del mando** | responsable | nunca se compro: hoy hay firmware y veto, no equipo |
| **recompilar la APK** | responsable | el paquete de entrega aborta con exit 2 |

---

## 6. Los hallazgos de esta sesion — el porque de todo lo de arriba


### 🟢 N-107 — BLQ-1 cerrado: es un `ESP32-WROOM-32` clasico, hay SPP · **CERRADO 31/08**

**La ficha del modulo comprado**, aportada por el responsable, cierra la fila mas bloqueante del
proyecto con **tres confirmaciones independientes**:

```
Microcontrolador ...  ESP32-WROOM-32
CPU ................  Tensilica Xtensa 32-bit LX6, DUAL-CORE
                      el S3 es LX7 y el C3 es RISC-V -> no es ninguno de los dos
Bluetooth ..........  v4.2 BR/EDR and Bluetooth Low Energy (BLE)
                             ^^^^^^ Bluetooth Clasico
```

`BR/EDR` es exactamente el perfil que necesita `createRfcommSocketToServiceRecord`. **La app conecta
sin tocar el transporte**, y el apartado 1 del Manual 10 —congelado por escrito— **no se reabre**.

Y la ficha resolvio de paso dos cosas abiertas: la alimentacion queda en
**`12 V -> DC-DC conmutado -> 5 V -> VIN`** (entrada recomendada 5 V, limite 5,5, con regulador a
bordo), y las E/S a **3,3 V** confirman que el enlace con el STM32 va directo, sin adaptar niveles.

#### Lo que se hizo mal por el camino, que es lo que hay que guardar

**El responsable dijo dos veces que el modulo "ya tiene Bluetooth integrado", y las dos veces se le
contesto con la misma explicacion en vez de con una medida.** La afirmacion era **cierta**; lo que
faltaba era distinguir `BR/EDR` de `BLE`. Pero la forma de resolverlo no era repetir la distincion:
era **buscar el dato**.

Y el dato **no exigia el modulo en la mano**. Se exigio durante toda la sesion *"la serigrafia del
blindaje, 30 segundos"*, cuando **la ficha tecnica del articulo comprado ya lo declaraba**. El
bloqueo se mantuvo mas tiempo del necesario **por no haber preguntado por la referencia de compra**,
que es un dato que el responsable tenia a mano desde el principio.

> **LA LECCION: antes de declarar algo bloqueado por una medida fisica, censa que fuentes escritas
> pueden responderlo ya.** Una serigrafia, una ficha de compra, una factura y un `esptool chip_id`
> contestan la misma pregunta con costes muy distintos, y **la mas cara no es la primera que hay que
> pedir**. Un bloqueo que se puede levantar leyendo no es un bloqueo: es una consulta pendiente.
>
> Y su corolario, que es de trato: **cuando alguien insiste en un hecho que resulta ser cierto,
> repetir la objecion no lo convierte en falso.** La segunda vez que se oye la misma afirmacion es
> la senal de ir a medir, no de explicar mejor.

#### Lo que sigue abierto, y es mucho menor

**Estas NodeMCU vienen en 30 y en 38 pines, con anchos distintos**, y la placa portadora lleva
hembrillas, no la huella del `WROOM-32` —el modulo es de formato protoboard, asi que va enchufado y
es reemplazable sin soldador—. Contar pines y medir el ancho con pie de rey **antes de fabricar**.
No bloquea firmware.

---

### 🔴 N-105 — Cuatro documentos mandan cablear camaras sobre pines que NO son entradas de camara, y uno deja que el trafico cambie el modo del semaforo solo

**Lo encontro la pasada de coherencia del 31/08, y lo verifico el orquestador contra el fuente.**

#### El peor: las camaras sobre los pines del mando

**MEDIDO** — `MANUAL_USUARIO.md:66-70`, cuatro lineas, las dos puntas:

```
* Camara 1 (Aproximacion Sentido 1): Contacto seco 1A/1B en **PB9**  y GND -> Demanda Verde Maestro.
* Camara 2 (Monitoreo Obra Sentido 1): Contacto seco 1A/1B en **PB13** y GND -> Confirma flujo interno.
* Camara 3 (Aproximacion Sentido 2): Contacto seco 1A/1B en **PB9**  y GND -> Demanda Verde Esclavo.
* Camara 4 (Monitoreo Obra Sentido 2): Contacto seco 1A/1B en **PB13** y GND -> Confirma flujo interno.
```

**Contra el fuente, MEDIDO:**

```
botones.cpp:119   if (flanco[0]) mando_registrarPulso(MANDO_A);   // BOTON1 = PB9
botones.cpp:120   if (flanco[1]) mando_registrarPulso(MANDO_B);   // BOTON2 = PB13
mando.cpp:38      static const unsigned long VENTANA_TRIPLE_MS = 12000;
mando.cpp:241-248 A.A.A -> ACC_AUTOMATICO   ·   B.B.B -> ACC_AMBAR
mando.cpp:129-132 case ACC_AMBAR: ambarLocal = true;
```

> **`PB9` y `PB13` son `MANDO_A` y `MANDO_B`.** Una camara enchufada ahi entrega pulsos, y **tres
> pulsos dentro de la ventana de 12 s componen una secuencia del mando**: en `PB9`, `A·A·A` mete el
> equipo en Automatico; en `PB13`, `B·B·B` lo manda a ambar **y arma `ambarLocal`**, que ademas veta
> las ordenes de radio. **El trafico cambiaria el modo del semaforo solo**, sin que nadie lo pida y
> sin que nada lo registre como orden.

**Y hay un segundo error encima del primero, que lo tapa:** el manual dice *"y `GND`"*. El camino de
camara es `pinMode(INPUT)` pelado y **activo en ALTO** (`modo_inteligente.cpp:46`, `:25`), con la
bornera sacando el pin junto a 3,3 V. Cableado a masa, **la camara no dispara nunca** — asi que un
ensayo de taller la aprobaria sin ver el defecto de arriba, y este aparece el dia que alguien
"arregla" el cableado.

#### Los otros tres

| `fichero:linea` | manda cablear | lo que hay |
|---|---|---|
| `04_Manuales/MANUAL_INSTALACION_RELOJ_DS3231.md:43-44`, `:56-57`, `:89`, `:121` | I2C del `DS3231` a `PB0` (SDA) y `PB8` (SCL) | **es la SEGUNDA COPIA del defecto del Manual 11.** `PB0` es `CAM_DEMANDA_PIN` y `PB8` es `LED_TESTIGO` |
| `MANUAL_HARDWARE.md:63`, `:66` | Camara 2 a `PB8`, *"Entrada Libre"* | `PB8` no es entrada: es salida a LED por `R16` 1 K |
| `05_Funcional/9_Manual_Parametrizacion_Camara_IA.md:64`, `:168` | bornera `1A`/`1B` a `PB0` **y `GND`** | mismo error de polaridad: `R64` es pull-**down**, la camara no disparara |

#### Por que aparecio ahora, que es la parte reutilizable

**El Manual 11 se arreglo esta misma manana** (`e1d3720`) y esta copia de `04_Manuales/` **siguio
intacta**. Se arreglo el fichero que alguien nombro, no la propiedad. El censo que habria encontrado
las dos es `grep` del pin, no del nombre del documento.

Y la segunda mitad, que es mas incomoda:

> **La decision de conservar el mando (N-104) convirtio cuatro documentos viejos en un peligro
> vial.** Mientras el plan era retirar los cuatro pulsadores, `PB9`/`PB13` iban a quedar sin dueño y
> un manual que mandara camaras ahi era solo un error de papel. Al conservar A y B, ese mismo texto
> pasa a describir un cableado que **compone ordenes de mando con el trafico**.
>
> **LA LECCION: una decision no solo cambia lo que se construye, cambia lo que SIGNIFICAN los
> documentos que ya estaban escritos.** Al cerrar una decision hay que censar que documentos hablan
> de los pines que toca — y el censo es `grep` del pin, no lectura del indice.

Y el hueco que lo dejo llegar hasta aqui: `documentos_01_cifras_del_acta` **solo vigila `README.md` y
`ESTADO.md`** (MEDIDO, `:224-225`). `MANUAL_HARDWARE.md`, `MANUAL_USUARIO.md` y `CERTIFICACION_SW.md`
**no los parsea nadie**, y es justo donde estan todas las cifras malas. Un `ABORTADO` grita; un hueco
no.

---

### 🔴 N-106 — El ambar de emergencia de la app NO saca al Esclavo del Modo Degradado

**MEDIDO por lectura del fuente. La consecuencia exacta esta razonada, no ejecutada: se marca como
tal y se cierra con el arnes, no con esta nota.**

```
grep -c "degradado" Esclavo/src/bluetooth.cpp   ->   0

Esclavo/src/bluetooth.cpp:130-136
    if (strcmp(cmd, "CMD:AMBAR_EMERGENCIA") == 0) {
      semaforo_iniciarFallo();
      ambarEmergencia = true;
      enviarTramaConCrc("$ACK,CMD:AMBAR_EMERGENCIA,RESULT:OK");
      ...
    }
```

**Quien SI sabe salir del Degradado, censado:**

```
degradado_salir()  <-  Esclavo/src/main.cpp:385   (la puerta automatica)
                   <-  Esclavo/src/mando.cpp:121  (el mando)
                   <-  Esclavo/src/mando.cpp:138  (el mando)
                   <-  Esclavo/src/menu.cpp:215   (la pantalla)
```

**`bluetooth.cpp` no esta en esa lista.** Y `degradado_actualizar()` corre en cada vuelta
(`main.cpp:363`), con `degradado_gobiernaLuz()` decidiendo quien manda sobre la luz (`:383`, `:555`,
`:619`).

> **Es decir: el mando puede sacar al Esclavo del Degradado, la pantalla puede, y la app NO.** El
> `$ACK,RESULT:OK` se manda igual. Es el patron de §6 —un `ACK` que no depende de lo que la llamada
> consiguio— pero esta vez el defecto no esta en el que contesta: **esta en que falta la llamada**.

**Y lo que lo vuelve grave es la conjuncion con las fases:**

- La **pantalla se retira** en la Fase 3, y con ella `menu.cpp:215`.
- Si el mando se hubiera retirado tambien —que era el plan hasta el 31/08— **no habria quedado
  NINGUNA via externa para sacar al Esclavo del Degradado**. Solo la puerta automatica de
  `main.cpp:385`.
- La decision de N-104 lo evito **por accidente**, no porque nadie lo hubiera visto.

**Lo que hay que hacer, y en este orden:** primero el pack que lo ejerza —que hoy no existe: ningun
instrumento comprueba que el ambar de la app saque del Degradado—, verlo **fallar** sobre el firmware
de hoy (§8.bis), y solo entonces añadir la llamada. Al reves, el arreglo entra sin testigo.

Y `05_Funcional/8_Procedimiento_Modo_Degradado.md:474` **llego a la conclusion correcta citando un
comando que no existe**: acerto por el camino equivocado. Se reescribe, no se borra.

> **LA LECCION: un censo de llamadores tiene dos direcciones, y la segunda casi nunca se hace.**
> Preguntar *"¿quien llama a esta funcion?"* encuentra codigo muerto. Preguntar *"¿quien DEBERIA
> llamarla y no lo hace?"* encuentra agujeros — y esa pregunta solo se puede hacer con la lista de
> los que si llaman delante. `mando.cpp` y `menu.cpp` la llaman; `bluetooth.cpp`, que es la unica
> interfaz que va a quedar, no.

---

### 🟢 N-104 — El mando se queda en A y B, las camaras entran por C y D, y la pantalla se fue porque NO HABIA PINES · **DECIDIDO 31/08**

**El porqué de toda la arquitectura del 28/08 no estaba escrito en ningun documento del repositorio.**
Los manuales explican *que* se retira; ninguno decia *por que no habia alternativa*. Lo aporto el
responsable el 31/08 y va aqui, porque sin ello dentro de tres meses alguien vuelve a proponer la
placa de expansion y nadie recuerda que se probo y no entraba.

**La razon de fondo, que no es de firmware:** a diferencia del proyecto anterior, **este PCB no
permite ampliacion**. Lo que se desarrollo para ampliarlo **no era fisicamente realizable**: exigia
soldar sobre una placa que no lo admite bien. Asi que no habia de donde sacar pines, y la unica
fuente disponible era retirar funciones.

#### La cuenta, MEDIDA

```
La pantalla ocupaba CINCO pines   (Maestro/include/pines.h:85-89)
    LCD_SCLK PB3 · LCD_CS PB4 · LCD_SID PB5 · LCD_PSB PB6 · LCD_RST PB7

Los cuatro pulsadores ocupan CUATRO (pines.h:92-95)
    BOTON1 PB9 · BOTON2 PB13 · BOTON3 PB14 · BOTON4 PB15

Lo que hacia falta y no cabia:
    Bluetooth por USART1 remapeado ...  PB6  PB7    <- los suelta la pantalla
    Reloj (en el plan de entonces) ...  PB3  PB4    <- los suelta la pantalla
    Segunda camara ..................   PB14 PB15   <- los sueltan los botones C y D
```

> **La pantalla no se retiro por la pantalla: se retiro porque el Bluetooth necesitaba exactamente
> dos de los cinco pines que ella tenia, y no habia otros.** Por eso N-76 fue lo primero de aquella
> sesion y todo lo demas cayo detras: con la LCD puesta, `PB6`/`PB7` estaban ocupados, no habia
> `USART1`, y sin `USART1` no hay Bluetooth.

**Y una consecuencia que ABARATA la cuenta, posterior a esa decision:** al mudarse el `DS3231` al
ESP32 (`GPIO21`/`GPIO22`, con pila propia), **el reloj dejo de costar pines del STM32**. `PB3`, `PB4`
y `PB5` quedan LIBRES al ejecutar la Fase 3 — margen que el equipo no tenia. Hoy siguen ocupados:
`lcd_setup()` se llama todavia (`Maestro/src/main.cpp:46`, `Esclavo/src/main.cpp:212`) y u8g2 retiene
los tres (`lcd.cpp:29`). `PB6` ya esta suelto desde N-76, que le paso `U8X8_PIN_NONE`.

#### El reparto de los cuatro pulsadores: el mando NO los usa todos

Esto estaba sin medir, y decidia si el mando podia convivir con las camaras.

```
botones.cpp:119   if (flanco[0]) mando_registrarPulso(MANDO_A);   // BOTON1 = PB9  = J16 p5
botones.cpp:120   if (flanco[1]) mando_registrarPulso(MANDO_B);   // BOTON2 = PB13 = J16 p8
botones.cpp:131   bool botonAceptar()  { return consumir(2); }    // BOTON3 = PB14 = J16 p10
botones.cpp:132   bool botonCancelar() { return consumir(3); }    // BOTON4 = PB15 = J16 p12

grep -n "BOTON[1-4]" Maestro/src/mando.cpp   ->   CERO coincidencias
```

**MEDIDO: el mando de reles vive entero en A y B.** No conoce pines —trabaja sobre `MANDO_A` /
`MANDO_B`, que solo alimentan los botones 1 y 2—, y **no toca `PB14` ni `PB15`**, que son Aceptar y
Cancelar del menu de la pantalla que se retira.

#### Por que A y B, y no solo A

Se evaluo dejar cableado un solo canal. **Se rechaza, y la razon es una medida, no una preferencia:**

```
Esclavo/src/mando.cpp:241-242   A·A·A     -> ACC_AUTOMATICO
Esclavo/src/mando.cpp:246-248   B·B·B     -> ACC_AMBAR
Esclavo/src/mando.cpp:219-220   A·B·A·B   -> ACC_DEGRADADO

Esclavo/src/mando.cpp:129-132
    case ACC_AMBAR:
      // Sin condiciones y desde cualquier estado. Es la regla que impide que nadie
      // quede atrapado con un semaforo en estado raro a 5 m de altura.
      ambarLocal = true;      <- EL UNICO sitio donde se pone a true
```

`ambarLocal` es lo que devuelve `mando_ambarLocal()` (`Esclavo/src/mando.cpp:103`), y de el cuelgan
los tres vetos negados del Esclavo (`Esclavo/src/main.cpp:406`, `:416`, `:540`).

> **Sin el canal `B`, `ambarLocal` no se arma jamas**, los tres `if` se vuelven siempre-verdaderos y
> una orden de radio puede sacar al Esclavo de un ambar que un operario dejo puesto a proposito. Es
> **N-79 exacto** —SFTY-21 desapareciendo por sustraccion— solo que por dejar sin cablear un canal en
> vez de por borrar `mando.cpp`.

Y `A`-solo no compra nada: liberaria **tres** entradas cuando solo hacen falta **dos**.

#### La decision, y lo que cierra

**DECIDIDO el 31/08 por el responsable: se conservan `A` (`PB9`) y `B` (`PB13`); se retiran `C`
(`PB14`) y `D` (`PB15`) y esos dos pines pasan a las camaras.**

| queda cerrado | por que |
|---|---|
| **N-79** · el veto que se borraba por sustraccion | `mando_ambarLocal()` se sigue armando por `B·B·B`. SFTY-21 no desaparece |
| **§3.3** del Manual 17 · sin superficie de mando si el ESP32 se cuelga | es la opcion 3 de su tabla —*dejar el mando*— y sale gratis |
| **N-101** · `Validacion_Automatico` abortando | `mando.cpp` **no se borra**, asi que `compilar.ps1:64` sigue enlazando y SFTY-5 conserva su unico instrumento |
| parte de **N-103** | `maestro_01_mando` conserva su sujeto: lo que se pierde baja de ~32 comprobaciones a ~17 —solo lo que muere con la pantalla— |

**Lo que NO cierra, y sigue igual de vivo:**

- 🔴 **La polaridad de `PB14`/`PB15` sigue en contradiccion medida** (N-84). El netlist tiene
  pull-**down** de 10 k con 3,3 V al lado —activo en ALTO— y `botones.cpp:19` lee `== LOW`. **No se
  cablea camara hasta la medida M3**, con multimetro.
- 🔴 **El orden sigue siendo asimetrico** (CLAUDE.md §9.bis): `PB14` es `botonAceptar()`, el que
  EJECUTA. El firmware nuevo tiene que estar **CARGADO EN LA TARJETA** antes de que nadie enchufe
  nada en `J16`.
- 🟠 **El receptor del mando nunca se compro** (Manual 17 §2.7). Lo que se conserva hoy es el
  **firmware y el veto**; para tener mando fisico hay que comprarlo. No contarlo como red de §3.3
  hasta entonces.

> **LA LECCION: un reparto de pines no se decide leyendo los nombres de los `#define`, se decide
> midiendo quien los consume.** Cuatro pulsadores parecian cuatro entradas del mando, y el mando solo
> usaba dos: la diferencia entre creer eso y medirlo son las dos entradas de camara que hacian falta
> y el veto de una regla de seguridad. Y su corolario: **antes de retirar un canal, busca que bandera
> deja de armarse** —el `grep` que importa no es el del pin, es el del `= true`—.

---

### 🔴 N-103 — El censo del instrumental frente a la arquitectura del 28/08: ~347 de 782 comprobaciones se quedan sin sujeto, y la compuerta sabe ver lo que se BORRA pero no lo que se queda sin sujeto

**De donde sale:** del encargo del `05_Funcional/17_...md`, Anexo, punto 8 —*"los packs que ejercen
pantalla, botones y mando quedan sin sujeto. Hay que decidir uno por uno si se borran, se invierten o
se conservan (`CLAUDE.md` §8.quater), con la cuenta comparada antes y despues"*—. Esta es esa decision,
pack a pack, hecha ANTES de tocar el firmware, que es el unico momento en que sirve.

**MEDIDO, corriendo los 38 packs uno a uno** con `python 01_Firmware/Simulaciones/banco/correr.py
--pack <nombre>` y sumando: **411**, la misma cifra del acta. La suma cuadra, asi que las cifras por
pack de abajo no son estimaciones.

---

#### A · Los cuatro packs que se quedan sin sujeto ENTERO — 32 comprobaciones

| pack | hoy | que vigila | destino | por que |
|---|---|---|---|---|
| `maestro_01_mando` | **15** | las tres secuencias `A.B.A.B` / `B.B.B` / `A.A.A`, el barrido de los 254 trenes de 1 a 7 pulsos, el barrido de cadencia 100..10.000 ms, la ventana deslizante y `purgarViejos()` | **SE BORRA — menos dos** | 13 de las 15 no las puede aprobar **ni suspender** ningun firmware sin reles: no hay quien genere un pulso. Es exactamente el residual del alias de `CMD_DELTA` que `CLAUDE.md` §2 mando a `reportar()`. **Pero DOS se conservan mudandolas de pack**, porque no hablan del mando: *"main.cpp llama a `semaforo_actualizar()` SIN CONDICION en el `loop()`"* y *"`modo_automatico.cpp` llama a `coordinador_actualizar()` unicamente dentro de `case CORRIENDO`"*. Esas dos cierran el fallo del cabezal a oscuras y su sitio es `maestro_05` o el arnes del automatico. **Y antes de borrar nada, N-102: ponerle la etiqueta `# EJERCE`** |
| `maestro_06_fuentes_pantalla` | **4** | que `lcd.cpp` dibuje los titulos en `u8g2_font_7x14B_tr` y que el arnes mida sus anchos con **esa misma** fuente | **SE BORRA** | existe por N-39: que el arnes y el firmware no divergan. Sin arnes de pantalla y sin `lcd.cpp` **no hay dos puntas que comparar**; la prueba aprobaria vacia, comparando nada contra nada, que es justo lo que `costura_01` avisa de si mismo para el dia de `lib/Common`. **Dejarlo es peor que borrarlo**: su `control_negativo` seguiria dando verde sobre un fosil |
| `maestro_07_menu_opciones` | **6** | que el array, los textos y la constante de cada menu digan lo mismo, y que la ultima opcion caiga en `y=61`, dentro de los 63 px | **SE BORRA** | los 64 px de alto se van con la pantalla. **La leccion no se pierde** —*"mientras el numero viva en dos sitios, alguien actualizara uno y no el otro"*— : la heredan `app_02_modos_simetricos` y el pack nuevo que ate `VERDE_MIN_MIN` a los limites de `app.js` (N75-2) |
| `esclavo_02_inhibicion_menu` | **7** | con el menu abierto, `B.B.B` no se reconoce; barrido de 85 a 95 s del regreso automatico; el cartel de rechazo caduca a los 6 s sin parar la cuenta | **SE INVIERTE** | 🔴 **es el UNICO pack etiquetado `# EJERCE SFTY-21` cuyo sujeto desaparece entero** (`:14`). La regla que protege —*"dos personas dando ordenes contrarias a la vez es peor que cualquiera de las dos ordenes"*— **no muere con el menu: se muda a dos telefonos sobre el mismo poste**, y hoy eso no lo mide nadie. Se invierte a: mientras un `AMBAR_EMERGENCIA` este puesto, una segunda sesion Bluetooth no lo revoca sin decirlo. Y su fila de `OPTIMIZACIONES.md:128` se toca **en el mismo commit**, o `documentos_02` falla |

---

#### B · Los packs con parte del sujeto fuera — el destino va por COMPROBACION, no por pack

| pack | hoy | caen | destino y por que |
|---|---|---|---|
| `esclavo_01_latch_ambar` | 7 | ~5 | **SE INVIERTE.** El barrido de 15 comandos x 4 estados de partida es de lo mejor que tiene el banco y **sigue valiendo entero**: lo que cambia es **quien arma el latch**, de `mando_ambarLocal()` a `bluetooth_ambarEmergencia()`. Las dos del Maestro cayendo a `C_FALLO` en 17,6 s no tocan el mando: **se conservan tal cual**. 🔴 Y aqui vive N-79: el veto de `Esclavo/src/main.cpp:406`, `:416`, `:540` |
| `esclavo_07_ambar_emergencia` | 16 | ~3 | **SE CONSERVA, con dos filas reescritas.** 13 de 16 son sobre nombres de comando y coherencia `$ACK`/rama —N-83— y no las toca nada. La que dice *"las 3 guardas de main.cpp que respetan el ambar del mando respetan TAMBIEN el pedido por Bluetooth"* **se invierte y sube de rango**: pasa de comparar dos vetos a ser **el unico** veto que queda, y ahi es donde hay que poner el `control_negativo` nuevo |
| `maestro_09_test_leds` | 18 | ~4 | **SE CONSERVA, rebaselinado.** Tres citan *"los cuatro caminos de la senal SFTY-21"* y `senalActiva`, el `static` que **solo pone `mando.cpp`**. No se relajan: se recuentan contra el censo nuevo. 🔴 **Relajar *"las 5 funciones que llaman a `escribirPines()` son las conocidas"* a *"las que haya"* mata el pack**, y es la tentacion exacta que §8.quater castiga |
| `maestro_03_puerta_degradado` | 19 | 1 | **SE BORRA esa una.** La ultima —*"las dos pantallas reciben un indicador de vencimiento y no solo un numero"*— lee `lcd.cpp` de las dos puntas (`packs/maestro_03_puerta_degradado.py:549-550`). Su equivalente honesto es un campo de `$STATUS` que lo diga, y eso ya es Fase 4. **Las otras 18 no se tocan** |
| `costura_06_reanudacion` | 6 | 0 o 1 | **SE CONSERVA.** Solo menciona el menu como *estado de partida* de la punta que arranca. Si `MODO_MENU` sobrevive como estado seguro sin display —que es lo que hace falta para SFTY-12— no cambia nada |
| `app_02_modos_simetricos` | 8 | 0 o 3 | **SE CONSERVA.** Tres listas incluyen el literal `MENU` de `obtenerNombreModo()`. **El pack las relee del C++ en cada corrida**, que es exactamente para lo que se escribio: si `MODO_MENU` desaparece de `ModoSistema`, se ajustan solas |
| `barrera_01_pines_de_luz` | 5 | 0 | **SE CONSERVA ENTERA.** Solo queda historico el comentario sobre los destellos del mando que *interceptan* en vez de rodear |
| `app_01_comandos` | 8 | 0 | **SE CONSERVA.** Los seis comandos de la Fase 1 los vera aparecer solo, porque lee los despachadores del C++ |
| `flash_01_lastre` | 11 | 11 | **SE INVIERTE, y es el mejor negocio de la lista.** Sus 11 comprobaciones son *"u8g2 declarado sin arrastrar I2C ni SPI"*. Retirada la pantalla, **u8g2 no debe estar**, y el pack **ya sabe expresar la forma invertida**: la fila *"Repetidor no tiene pantalla y no arrastra banderas de u8g2"* es literalmente eso. Y vigila los **~18,9 KB estimados** que la Fase 3 promete liberar en el Maestro: sin el, ese numero es una estimacion de nadie (`CLAUDE.md` §7: un delta exige medir los DOS extremos) |

---

#### C · Los tres detectores que se disparan solos — se atienden, NO se silencian

| pack | hoy | que hara | destino |
|---|---|---|---|
| `costura_10_funciones_muertas` | 13 | **FALLA.** Lleva las huerfanas conocidas escritas a mano (`packs/costura_10_funciones_muertas.py:44-68`): **15 en el Maestro, 8 en el Esclavo**. En cuanto `botones_setup()`, `lcd_*` o `menu_*` pierdan su llamador, el censo vera huerfanas **nuevas** fuera de lista | **SE CONSERVA.** 🟢 **Es el UNICO instrumento del banco que caza la Fase 2**, donde el fichero sigue en disco y solo deja de llamarse. La lista se **amplia una por una con su motivo escrito**, jamas se relaja: es un trinquete, y un trinquete que se afloja no es nada |
| `documentos_01_cifras_del_acta` | 51 | **FALLA** el dia que `411`, los `38` packs, el `271/271` de pantalla o **las 15 filas** del acta dejen de coincidir con README y `ESTADO.md` | **SE CONSERVA.** Es la comparacion de totales de `CLAUDE.md` §5, **la unica red para esta clase de deriva**, y ya ha salvado la migracion dos veces. Sus cifras se copian del acta nueva, nunca se escriben a mano |
| `documentos_02_trazabilidad_sfty` | 10 | **FALLA** si desaparece la etiqueta `# EJERCE SFTY-21` de `esclavo_02:14` sin tocar `OPTIMIZACIONES.md:128`; exige coincidencia **exacta en las dos direcciones** | **SE CONSERVA.** Etiqueta y fila van en el **mismo commit**. Ver N-102: su punto ciego es lo que nunca se etiqueto |

**Los 21 restantes no se tocan** —verificado uno a uno—: `app_03`, `barrera_02`, `barrera_03`,
`camara_01`, `costura_01`..`costura_05`, `costura_07`, `costura_08`, `costura_09`, `documentos_03`,
`esclavo_03`..`esclavo_06`, `identidad_01`, `maestro_02`, `maestro_04`, `maestro_05`, `maestro_08`.
`costura_01_contratos` se comprobo expresamente: sus 7 ficheros compartidos son `protocolo`,
`ciclo_degradado`, `respaldo` e `identidad` — **ni `lcd`, ni `menu`, ni `mando`**.

---

#### D · Los cuatro arneses de C++ real y los simuladores

| instrumento | acta | que le pasa |
|---|---|---|
| **`Validacion_LCD`** | **271/271** | **MUERE ENTERO.** `compilar.ps1` compila `lcd.cpp`, `menu.cpp`, `modo_degradado.cpp` y `modos.cpp`; el arnes incluye `lcd.h`, `menu.h` y `botones.h`. Sin `lcd.cpp` no enlaza y `compuerta.py:357` lo marca **ABORTADO**. 🔴 **No se deja abortado ni un dia** (§3.quater: N-75 entro exactamente asi, con dos instrumentos en ABORTADO y cuatro defectos detras). **Y no todo lo suyo es pantalla:** enlaza `modo_degradado.cpp`, que es SFTY-21. Si algo se rescata es un arnes nuevo que compile `modo_degradado.cpp` sin display, no las 271 |
| **`Validacion_Automatico`** | **71/71** | **ABORTA con `mando.cpp`, y se lleva el unico instrumento de SFTY-5.** Es N-101 entero |
| `Validacion_Ciclo` | 29/29 | **INTACTO.** Solo `ciclo_degradado.h`, funcion pura |
| `Validacion_Respaldo` | vivo | **INTACTO.** `respaldo.cpp` + `calcularSuma()` |
| `simulador_sistema_v7_6.py` | 20/20 | **3 de 20 se quedan sin sujeto:** PRUEBA 1 *"Menu con comunicacion -> ambos en ROJO FIJO"*, PRUEBA 2 *"Menu SIN comunicacion -> ambos en AMARILLO PARPADEO"* y PRUEBA 6 *"Modo Manual - Boton 3"*. **Se reescriben por la via Bluetooth**, que es lo que SFTY-12 dice que se conserva: la independencia de la radio, no el menu. 🔴 Y recordar la cuarta cara de N-46: **este simulador escribe `✘ FAIL` y sale con codigo `0`**; lo unico que lo caza es la regla `x == y` sobre su `20/20` |
| `simulador_app_bluetooth.py` | 5/5 | no se toca |
| `simulador_repetidor.py` | 10/10 | **INTACTO** — cero menciones de pantalla, boton o mando |

---

#### E · La cuenta

| | comprobaciones |
|---|---|
| Banco: sujeto que desaparece entero | **32** (15 + 4 + 6 + 7) |
| Banco: se invierten o se rebaselinan | **~46** (`flash_01` 11, `esclavo_01` ~5, `esclavo_07` ~3, `maestro_09` ~4, `maestro_03` 1, `costura_10` 13 en revision) |
| `Validacion_LCD` | **271** |
| `Validacion_Automatico`, BLOQUE D | **44 de 72** llamadas a `comprobar()` |
| Simulador funcional | **3 de 20** |
| **EXPUESTO** | **~347 de las 782** comprobaciones de PC del acta — **~44 %** |

El banco pasaria de `411/411` a **~379**, y el acta de **15 filas a 14** si `Validacion_LCD` se retira.

**Cobertura SFTY-x despues, cruzando las 18 etiquetas `# EJERCE` con `OPTIMIZACIONES.md:109-131`:**

- **SFTY-21:** de 6 packs etiquetados a 5 (cae `esclavo_02`). Pero la mitad que se queda **de verdad**
  descubierta es la del mando, y esa la media `maestro_01_mando` **sin etiqueta** — N-102: **la tabla
  no lo reflejara**.
- **SFTY-2 y SFTY-28:** siguen cubiertas (`barrera_01/02/03`, `esclavo_06`, `maestro_09`), con las
  cuatro filas de `maestro_09` rebaselinadas.
- **SFTY-6 y SFTY-23:** intactas.
- **SFTY-5:** **cero** si `Validacion_Automatico` aborta. N-101.
- **SFTY-12, 14, 15, 18:** tercera columna **ya vacia hoy**, asi que no se pierde cobertura porque no
  la hay — pero su via se muda al Bluetooth y **nada la medira despues tampoco**. La peor es
  **SFTY-15**: sus contadores de linea (`RX 0 - nada llega` / `RX 4k - BASURA`) **no estan en
  `$STATUS`**, asi que esa capacidad de diagnostico **se pierde de verdad, no se traslada**.

---

#### F · El ESP32: dieciseis instrumentos que hacen falta ANTES de la primera linea

**MEDIDO:** cero rutas declaradas, cero packs, cero pasos en `compuerta.py` para el ESP32 de
expansion. Y `compuerta.py:88`:

```
_ROLES = ("Maestro", "Esclavo", "Repetidor")
```

**Tres papeles.** La guarda de rutas censa tuplas `(rol, carpeta, fichero)` y en `:128` completa las
que vienen sin rol probando **solo esos tres**. **Un proyecto nuevo del ESP32 de expansion es
invisible para ella**, y con `RUTAS_MINIMAS_ESPERADAS = 20` (`compuerta.py:86`) el suelo tampoco lo
nota, porque las 43 rutas de hoy siguen ahi.

> 🔴 **Es N-75 con un agravante.** Alli entraron cuatro defectos detras de **dos instrumentos que
> ABORTARON** —o sea, que gritaron y nadie escucho—. Aqui **no hay ninguno que pueda abortar**:
> `CLAUDE.md` §3, literal, *"un `ABORTADO` al menos grita; un hueco no"*.

**Puente serie (`J17` p2/p3, `PB7`/`PB6`, 9600 8N1):**

1. **Costura de tres tramos.** `app_01_comandos` cruza hoy el `.js` contra el C++ de las dos puntas.
   Con el ESP32 en medio hay **una tercera tabla**: todo comando que la app emite el puente lo pasa
   integro, y todo `$ACK`/`$ERR`/`$STATUS` del STM32 llega a la app. Un puente que filtra en silencio
   es N-58 otra vez.
2. **El puente NO origina.** SFTY-2 extendido: censo `grep` de literales de comando en el fuente del
   ESP32 que no procedan del buffer de entrada. El molde existe y es `esclavo_06_no_abre_paso`.
3. **Valida antes de retransmitir**, como SFTY-16 ya obliga al repetidor: ni relaya basura, ni
   **parte ni une** una trama —`$STATUS` lleva su `*XX`—. `documentos_03_trama_status` vigila hoy
   tres copias del contrato (C++, Manual 10, `app.js`); **el ESP32 es la cuarta**.
4. **Presupuesto del enlace.** `costura_09_presupuesto_radio` aplicado al serie: peor caso de
   bytes/segundo contra los 9600 bps, y el buffer del puente por encima de la rafaga. **Recalculado
   del fuente en cada corrida, no escrito en prosa** — N-71.
5. **Silencio no es orden.** Con el TX del ESP32 mudo, ausente o en reposo (medida **M5** del
   Manual 17), ninguna accion. Con `control_negativo`.

**Watchdog — va PRIMERO, como dice la Fase 5:**

6. **Existe y se alimenta.** Un `esp_task_wdt_init()` sin `esp_task_wdt_reset()` es `CAM_UMBRAL_PIN`
   con otro nombre. El censo es `grep` de la declaracion contra las llamadas: `costura_10`.
7. **Y se alimenta desde la tarea que se cuelga**, no desde otra que sigue viva cuando la primera
   muere. Un watchdog alimentado por el vecino no vigila a nadie.
8. **La desigualdad, en un pack.** Periodo del watchdog del ESP32 **<** `SFTY6_SILENCIO_MS = 25000UL`
   (`*/include/protocolo.h:149` en las dos puntas). Si no, el STM32 ya se fue a ambar antes de que el
   puente se recupere solo, y el watchdog no sirve para lo que se puso. **Es N-71 exacto**: un techo
   que hoy vive en prosa.
9. **El STM32 sigue operable sin ESP32.** Ningun camino del STM32 espera bloqueado una respuesta del
   puente. Hoy eso lo garantiza el diseno; **el Manual 17 §3.3 lo deja abierto y ningun instrumento
   lo mide**.

**`DS3231`:**

10. **La hora nace no fiable** — SFTY-18 trasladado: existe una funcion *"tengo hora?"* y **toda**
    ruta que use la hora la consulta **antes**. El molde es la prueba 5 de `maestro_03_puerta_degradado`
    (*"sin reloj en hora la puerta se cierra antes de mirar nada mas"*).
11. **El bit `OSF`.** El `DS3231` trae el *oscillator-stop flag*: se lee al arrancar y una hora con
    `OSF` puesto se declara **no fiable**. Es la leccion del ano marcador de SFTY-18, con el bit que
    el chip ya regala.
12. 🔴 **`$ACK` que mira** — `app_03_sin_ok_mudo` extendido al ESP32. **La rama `SET_RTC` del puente
    no puede contestar `RESULT:OK` sin mirar el retorno de la escritura I2C.** Es literalmente el
    defecto del 28/08 (N-80) **mudandose de micro**: sin este pack entra otra vez, y esta vez sin
    instrumento que lo cace.
13. **Atomicidad**: la hora entra entera o no entra. `esclavo_05_hora_atomica` trasladado a la
    escritura multi-registro por I2C.
14. **Rango y BCD por BARRIDO, no por muestra.** El `DS3231` guarda BCD; ninguna hora invalida
    (mes 13, 31/02) se escribe, y el rechazo es explicito. `esclavo_04_desfase` barre las 3.600
    combinaciones precisamente porque *"los fallos de aritmetica circular viven en el salto de 59 a 0,
    que un muestreo se salta"*.

**Estructural, sin lo cual nada de lo anterior mide:**

15. **Un rol nuevo en `compuerta.py:88`** para el proyecto del ESP32, o la guarda de rutas no ve su
    fuente. Sin esto, mover o renombrar un `.cpp` del ESP32 rompe instrumentos en silencio.
16. **`compuerta.py` tiene que COMPILAR el ESP32 de expansion**, como ya compila el Repetidor
    (`20.6% de 1310720 B` en el acta). `CLAUDE.md` §3, literal: *"un instrumento que no esta en la
    compuerta no mide nada — y no deja rastro de que falta"*.

**Y §8.bis para los dieciseis**: se inyecta el defecto en el fuente real, se exige que **baje la
cuenta y cambie el codigo de salida**, y se restaura verificando con `git diff HEAD` **vacio** — no
con la impresion de haberlo restaurado.

---

#### G · Veredicto sobre la compuerta

**SI se daria cuenta de cuatro cosas** —y conviene decirlo, porque son mejores de lo esperado:

1. **El borrado de ficheros.** `banco/modelos/maestro.py:31` declara `MANDO`, `:48` declara `BOT =
   ("Maestro","src","botones.cpp")`; `banco/modelos/esclavo.py:28-29` declaran `_ESC_MANDO` y
   `_ESC_MENU`; `packs/maestro_06_fuentes_pantalla.py:50` y `packs/maestro_07_menu_opciones.py:46`
   declaran `lcd.cpp` y `menu.cpp`. Borrar cualquiera -> **guarda de rutas ABORTADO, exit `2`, la
   compuerta se para antes de compilar nada**.
2. **Los arneses ausentes.** `compuerta.py:357` y `:626` marcan `ABORTADO` con motivo si falta el
   directorio.
3. **La deriva de cifras.** `documentos_01_cifras_del_acta`, 51 comprobaciones, compara README y
   `ESTADO.md` contra el acta **mas reciente**: `411`, `38` packs, `271/271`, y **las 15 filas**.
   Retirar `Validacion_LCD` de la compuerta baja el acta a 14 y esa comprobacion falla.
4. **La Fase 2.** `costura_10_funciones_muertas` ve a `botones_setup()` perder su llamador **aunque
   `botones.cpp` siga en disco**. Es el unico que lo ve.

**NO se daria cuenta de cinco, y estas son las que importan:**

1. 🔴 **El ESP32 entero.** Sin rol en `_ROLES`, sin pack, sin compilacion. La compuerta saldria con
   **15 PASS y exit `0`** con el firmware del ESP32 sin una sola comprobacion detras, **y el acta no
   tendria una fila donde echarlo de menos**.
2. 🔴 **Un pack en verde midiendo hardware que ya no existe.** Si `mando.cpp` se queda en disco y solo
   se retiran los reles, las **15 comprobaciones** de `maestro_01_mando` siguen en `PASS` sobre
   secuencias que **ningun dedo puede generar**. Es la prueba muerta de N-51 introducida **sin tocar
   el fuente**, y la forma de N-89: un cambio que ningun test delata porque el firmware sigue siendo
   correcto.
3. 🔴 **El veto de `mando_ambarLocal()` (N-79).** `esclavo_07` comprueba que las tres guardas
   **consultan** el latch de Bluetooth; **no** comprueba que el latch del mando pueda **armarse**.
   Retirado el armador, `Esclavo/src/main.cpp:406`, `:416` y `:540` se vuelven siempre-verdaderos, la
   compuerta sigue en verde, y SFTY-21 **desaparece por sustraccion**.
4. 🔴 **La perdida de cobertura de SFTY-21 por el mando.** N-102: `maestro_01_mando` no lleva
   `# EJERCE`, asi que `documentos_02` seguira en `10/10` y la tabla no cambiara ni un caracter.
5. 🔴 **`Validacion_Automatico` llevandose SFTY-5.** La compuerta *diria* `ABORTADO` —bien—, pero
   **nada en el repositorio dice que ese arnes es el unico instrumento de SFTY-5** salvo una celda de
   `OPTIMIZACIONES.md:113` que ningun pack cruza, porque la propia tabla admite que los arneses C++
   son *"invisibles para ese censo"*.

> 🔴 **En una linea: LA COMPUERTA SABE VER LO QUE SE BORRA Y NO SABE VER LO QUE SE QUEDA SIN SUJETO.**
>
> Y el orden de las seis fases empuja hacia el segundo caso: **la Fase 2 *ignora* los pulsadores y la
> Fase 3 *retira* la pantalla**. Entre una y otra hay una ventana —dias, quiza semanas— en la que
> **32 comprobaciones estarian en verde midiendo codigo que ya no corre**, con la compuerta en `15
> PASS | 0 FALLA | 0 ABORTADO` y exit `0`. Ese `0` no diria *"el firmware cumple"*: diria *"nadie ha
> preguntado"*.

**Lo que hay que hacer antes de tocar firmware, en este orden:**

| # | que | por que va ahi |
|---|---|---|
| 1 | **Etiquetar `maestro_01_mando`** con `# EJERCE SFTY-21` y actualizar `OPTIMIZACIONES.md:128` a siete packs | N-102: sin esto, el borrado del pack es **silencioso** para el instrumento de trazabilidad. Va primero porque hace visible una cobertura que hoy no se cuenta |
| 2 | **Actualizar `compilar.ps1:64` y los stubs de `arnes_automatico.cpp`** en el **mismo commit** que retire `mando.cpp` | N-101: o SFTY-5 se queda con cobertura **cero**, en ABORTADO |
| 3 | **El pack que herede el veto de `mando_ambarLocal()`**, ANTES de borrar el armador | N-79. Retirar codigo **no es neutro** cuando otros dependen de que una bandera pueda ser CIERTA |
| 4 | **El rol del ESP32 en `_ROLES` y su compilacion en la compuerta**, aunque el pack aun este vacio | para que el hueco **grite** en vez de no dejar rastro |
| 5 | **Anotar el total `411` esperado antes y despues de CADA fase** | es la comparacion de totales de §5, la unica red para esta clase de deriva, y ya ha salvado la migracion dos veces |

**LECCION REUTILIZABLE: retirar hardware es la operacion mas peligrosa que puede sufrir un banco de
pruebas, porque no rompe ningun instrumento — los deja midiendo. La guarda de rutas vigila ficheros
que desaparecen; el compilador vigila simbolos que faltan; ninguno de los dos vigila una comprobacion
que sigue corriendo, sigue pasando y ya no habla de nada. Antes de retirar una pieza, el censo no es
"que se rompe" sino "que se queda sin sujeto", y ese censo hay que hacerlo pack a pack Y arnes a
arnes, con la cuenta anotada antes y despues, porque el unico sintoma que este banco emite ante un
instrumento sin sujeto es un numero que no baja.**

---

*Borrador escrito el 31/08/2026 sobre HEAD `8d76f1e`, arbol LIMPIO, sin modificar ni un fichero del
repositorio. Los 38 packs se corrieron uno a uno y la suma da `411`, la del acta. Lo marcado MEDIDO
se puede repetir abriendo el fichero y la linea que se cita, o corriendo el comando que se pega. Lo
marcado LEIDO viene de un documento y no se ha verificado contra el fuente. Nada de esto autoriza
nada: la sesion de banco sigue siendo el bloqueante.*

---

### 🔴 N-102 — `maestro_01_mando` ejerce SFTY-21 y no lleva la etiqueta `# EJERCE`: 15 comprobaciones que la tabla de trazabilidad nunca conto, y no vera desaparecer

**De donde sale:** de cruzar los packs que se quedan sin sujeto contra la tercera columna de
`OPTIMIZACIONES.md`, para saber que reglas `SFTY-x` pierden cobertura. Uno de los packs afectados no
aparecia en la cuenta, y la razon no era que no ejerciera nada.

**MEDIDO:**

```
grep -c "EJERCE" 01_Firmware/Simulaciones/banco/packs/maestro_01_mando.py    ->  0

01_Firmware/Simulaciones/banco/packs/maestro_01_mando.py:3
    # SECUENCIAS DEL MANDO DE RELES (SFTY-21, mando.cpp)

01_Firmware/Simulaciones/banco/packs/esclavo_02_inhibicion_menu.py:14
    # EJERCE SFTY-21: el mando queda inhibido con el menu abierto.
```

**El pack se titula "SFTY-21" en su primera linea util y no lleva la etiqueta.** Su hermano
`esclavo_02`, que vigila la otra mitad de lo mismo, si la lleva. Las 18 etiquetas `# EJERCE SFTY-x`
que hay hoy en `banco/packs/` son estas —censo completo, `grep -n "EJERCE SFTY" *.py`—:

```
barrera_01_pines_de_luz:34        SFTY-2      esclavo_04_desfase:14             SFTY-23
barrera_02_dos_puntas:50          SFTY-2      esclavo_06_no_abre_paso:36        SFTY-2
barrera_03_talanquera:3           SFTY-28     esclavo_07_ambar_emergencia:34    SFTY-21
costura_02_fase_ciclo:11          SFTY-21     maestro_05_ciclo_sin_radio:14     SFTY-21
costura_06_reanudacion:11         SFTY-21     maestro_09_test_leds:53           SFTY-2
costura_08_silencio:3             SFTY-6      maestro_09_test_leds:54           SFTY-28
costura_09_presupuesto_radio:3    SFTY-6
esclavo_01_latch_ambar:14         SFTY-21
esclavo_02_inhibicion_menu:14     SFTY-21
esclavo_03_par_config:27          SFTY-23
```

Y lo que publica la tabla, `OPTIMIZACIONES.md:128`:

```
| SFTY-21 | ✅ esclavo_01_latch_ambar · esclavo_02_inhibicion_menu · esclavo_07_ambar_emergencia
           · maestro_05_ciclo_sin_radio · costura_02_fase_ciclo · costura_06_reanudacion |
```

Seis packs. **`maestro_01_mando` no esta, y son 15 comprobaciones** —el barrido de los 254 trenes de
1 a 7 pulsos, el barrido de cadencia de 100 a 10.000 ms, la ventana deslizante, la purga de gestos
viejos—: la mitad **del Maestro** de SFTY-21, que es literalmente *"Modo Degradado por reloj y mando
de 4 reles"* (`OPTIMIZACIONES.md:199`).

**Por que esto no es una fila que falta, sino un fallo del instrumento.** `documentos_02_trazabilidad_sfty`
existe porque el 27/08 se descubrio que esa columna estaba **escrita a mano**, y su promesa es que se
levanta *"BUSCANDO la etiqueta `# EJERCE SFTY-x` en `banco/packs/`, no escribiendola a mano"*. Lo
cumple, y lo cumple en las dos direcciones: sus 10 comprobaciones exigen que cada fila cite
**exactamente** los packs etiquetados, que ningun pack citado carezca de etiqueta y que todos existan.
Corrido hoy sale en `10/10`.

> 🔴 **Y aun asi tiene un punto ciego estructural: solo puede ver lo que lleva etiqueta.** Un pack
> que ejerce una regla y no se etiqueta es invisible para el censo, **y por tanto tambien lo es su
> desaparicion**. Cuando `maestro_01_mando` se borre, `documentos_02` seguira en `10/10` y la fila de
> SFTY-21 no cambiara **ni un caracter**. La tabla dira lo mismo el dia antes y el dia despues de
> perder 15 comprobaciones de una regla de seguridad.
>
> Es la forma exacta de N-73 y de `CAM_UMBRAL_PIN` trasladada al instrumental: **no una prueba que no
> mide, sino una medida que no se cuenta**. Y es peor de lo que parece, porque el documento advierte
> encima de si mismo que *"una fila que cita menos packs de los que hay no se ve"*. Se escribio la
> advertencia y el caso estaba debajo.

**Lo que hay que hacer, y en que orden:**

1. **Poner la etiqueta AHORA, antes de borrar nada.** `# EJERCE SFTY-21: las tres secuencias del
   mando de reles y su ventana deslizante.` Con la etiqueta puesta y la fila de `OPTIMIZACIONES.md:128`
   actualizada a **siete** packs, el borrado del pack **si** hara fallar a `documentos_02` y obligara
   a tocar la tabla de forma consciente. Sin ella, el borrado es silencioso.
2. **Es un commit de un solo cambio con sentido propio**, y no va mezclado con la retirada del mando:
   primero se hace visible la cobertura, luego se retira. Al reves no sirve para nada.
3. **Censar si hay mas casos.** El metodo es `grep` de `SFTY-` en las cabeceras de los 38 packs
   contra `grep` de `EJERCE SFTY-`, y comparar. `esclavo_05_hora_atomica` se titula *"APLICACION
   ATOMICA DE LA HORA (SFTY-23)"* y **tampoco lleva etiqueta** —7 comprobaciones mas que la tabla no
   cuenta—. Y `maestro_04_sync_horaria` se titula *"SINCRONIZACION HORARIA POR RADIO (SFTY-23)"*, sin
   etiqueta, 11 comprobaciones. **Son tres, no uno**, y los tres se han encontrado con el mismo
   `grep` de dos lineas. *(MEDIDO sobre las cabeceras; **no** he verificado prueba por prueba que las
   18 comprobaciones de `esclavo_05` y `maestro_04` ejerzan SFTY-23 de verdad, y esa verificacion es
   obligatoria antes de etiquetar: `CLAUDE.md` dice que **solo se etiqueta lo que el pack comprueba
   de verdad**, porque una fila que miente es peor que una vacia.)*

**LECCION REUTILIZABLE: un instrumento que se levanta buscando una etiqueta solo puede ver lo
etiquetado, asi que su punto ciego no son los falsos positivos —esos los caza, y en las dos
direcciones— sino las OMISIONES, que son invisibles por construccion. Y la omision se cobra dos
veces: primero como cobertura que no aparece, y despues, el dia del borrado, como perdida de
cobertura que el instrumento no puede senalar porque nunca la tuvo apuntada. El titulo de un pack no
es su etiqueta: si la cabecera nombra una SFTY-x y la etiqueta no esta, o el titulo miente o falta la
etiqueta, y hay que decidir cual de las dos.**

---

---

### 🔴 N-101 — `Validacion_Automatico` compila `mando.cpp` REAL: retirar el mando no lo hace FALLAR, lo hace ABORTAR, y se lleva el unico instrumento de SFTY-5

**De donde sale:** de preguntarse que arnes queda sin sujeto al retirar el mando de 4 reles, dando
por hecho —como dice `CLAUDE.md` §8— que `Validacion_Automatico` es *"el arnes del ciclo"*. Lo es, y
ademas es otra cosa que no estaba escrita en ningun sitio.

**MEDIDO, abriendo los dos ficheros:**

```
01_Firmware/Validacion_Automatico/compilar.ps1:64
    Compilar-Fuente (Join-Path $MAESTRO 'src\mando.cpp')           'mando.o'

01_Firmware/Validacion_Automatico/arnes_automatico.cpp:446
    mando_setup();                   // N-52: limpia secBoton/pendiente del mando
01_Firmware/Validacion_Automatico/arnes_automatico.cpp:485-486
    if (pulsarA) mando_registrarPulso(MANDO_A);
    if (pulsarB) mando_registrarPulso(MANDO_B);
01_Firmware/Validacion_Automatico/arnes_automatico.cpp:494
    mando_actualizar();
```

Y el reparto de sus comprobaciones, contado sobre las llamadas a `comprobar()`:

```
total de llamadas a comprobar() en arnes_automatico.cpp .......  72
    lineas 1..948   (BLOQUE A + B + C) ........................  28
    lineas 949..fin (BLOQUE D: EL MANDO DE RELES) .............  44
```

`arnes_automatico.cpp:949` abre el bloque asi, literal: `// BLOQUE D: EL MANDO DE RELES (SFTY-21) —
mando.cpp REAL, sobre los PINES`.

*(El acta publica `71/71` y las llamadas a `comprobar()` son 72: una vive en un ayudante o en una
rama que no se ejerce. **La cifra de 44/72 es de sitios de llamada, no de comprobaciones ejecutadas**
— se dice asi a proposito, porque contar lo uno y publicar lo otro es como se cuelan las cifras que
nadie midio.)*

**El propio `compilar.ps1` explica por que esta ahi**, y el motivo es bueno (`compilar.ps1:7-9`):

```
# N-52: mando.cpp se suma aqui. Antes el arnes media los pines de verdad pero
# senalActiva -el static de semaforo.cpp que SOLO pone mando.cpp- nunca se ponia a
# true en este binario, porque mando.cpp no se compilaba.
```

Es decir: **`mando.cpp` se anadio al arnes precisamente porque sin el una rama de `semaforo.cpp`
nunca se ejercia.** La solucion de N-52 es correcta y crea la dependencia que hoy muerde.

**LO QUE NADIE HABIA ESCRITO, y es lo grave.** `OPTIMIZACIONES.md:113`:

```
| SFTY-5 | Maestro/src/semaforo.cpp · Esclavo/src/semaforo.cpp | ✅ Validacion_Automatico/arnes_automatico.cpp
          — **arnes C++, invisible para el censo de packs**, ver abajo |
```

**`Validacion_Automatico` es el UNICO instrumento de SFTY-5** —la transicion de luz legal en
Colombia, Res. 2024: Verde -> Rojo directo, Rojo -> Amarillo fijo 4,0 s -> Verde—. Ningun pack la
ejerce; la propia tabla admite que los arneses C++ son invisibles para el censo de
`documentos_02_trazabilidad_sfty`, que solo busca `# EJERCE SFTY-x` **dentro de `banco/packs/`**.

**La consecuencia, y es la diferencia entre FALLA y ABORTADO otra vez:**

| si se borra `mando.cpp` y no se toca nada mas | que pasa |
|---|---|
| `compilar.ps1:64` no encuentra el fichero | el arnes **no enlaza** |
| `compuerta.py:626` | `anotar("arnes del automatico", ABORTADO, ...)` |
| las 28 comprobaciones de los BLOQUES A/B/C, incluida SFTY-5 al milisegundo | **no corren** |
| el acta | `14 PASS | 0 FALLA | 1 ABORTADO`, exit **2** |

Y antes que eso, `compuerta.py:101` (guarda de rutas) ya habria abortado, porque
`banco/modelos/maestro.py:31` declara `MANDO = ("Maestro", "src", "mando.cpp")` y
`banco/modelos/esclavo.py:28` declara `_ESC_MANDO`. **La compuerta se para en la primera fila y ni
siquiera llega a compilar.** Eso es bueno —grita— pero deja el trabajo a medias con el arbol en
rojo, que es justo la situacion que `CLAUDE.md` §3.quater prohibe apuntar para luego.

> 🔴 **Y hay un modo de romperlo SIN que grite, que es el peligroso.** Si en vez de borrar
> `mando.cpp` se le retiran los llamadores —Fase 2, *"ignorar los pulsadores"*—, el fichero sigue en
> disco, la guarda de rutas no ve nada, el arnes enlaza igual y **sus 44 comprobaciones del BLOQUE D
> siguen en verde ejerciendo un mando que ningun rele puede accionar**. El arnes seguiria midiendo
> de verdad; lo que ya no existiria es el sujeto.

**Lo que este N-x fija, y no es una recomendacion:**

1. **`compilar.ps1:64` y los stubs de `arnes_automatico.cpp` se actualizan EN EL MISMO COMMIT que
   retire `mando.cpp`.** No en el siguiente. `CLAUDE.md` §5 lo dice para los packs que leen por ruta;
   esto es lo mismo para un arnes que **compila** por ruta.
2. **Los BLOQUES A/B/C se preservan enteros.** Son SFTY-5, el ciclo completo y la orfandad SFTY-6, y
   **ninguno de los tres depende del mando**. Si el arnes se toca, se toca el bloque D y solo el.
3. **Antes de dar el arnes por bueno tras el cambio, §8.bis:** se inyecta el defecto que ya se sabe
   que caza —`VERDE1` forzado a `HIGH` por debajo del enclavamiento de `aplicarSalidas()`, que es
   como se conecto: cayo a `25/26`— y se exige que la cuenta baje y el codigo de salida cambie. Un
   arnes recortado que no se ha visto fallar despues del recorte es un arnes nuevo.

**Y una nota de citas que aparecio de paso, MEDIDA:** `05_Funcional/17_...md` §2.4 y `ESTADO.md` §4b
citan los tres consumidores del veto como `Esclavo/src/main.cpp:401`, `:408`, `:526`. **Estan
caducadas.** Medido hoy:

```
Esclavo/src/main.cpp:406   if (!mando_ambarLocal() && !bluetooth_ambarEmergencia()) {
Esclavo/src/main.cpp:416   if (!mando_ambarLocal() && !bluetooth_ambarEmergencia()) {
Esclavo/src/main.cpp:540   if (!mando_ambarLocal() && !bluetooth_ambarEmergencia() &&
Esclavo/src/mando.cpp:103  bool mando_ambarLocal() { return ambarLocal; }
Esclavo/include/mando.h:76 bool mando_ambarLocal();
```

`CLAUDE.md` §3.ter tiene las buenas (`:406`, `:416`, `:540`); los dos documentos de trabajo van tres
commits por detras. Una cita `fichero:linea` que apunta al sitio equivocado no es un error
cosmetico: manda a quien va a ejecutar la Fase 2 a mirar donde no esta.

**LECCION REUTILIZABLE: un arnes que COMPILA un `.cpp` depende de el con mas fuerza que un pack que
lo parsea, y esa dependencia no esta en ninguna tabla — hay que leerla del `compilar.ps1`. Antes de
retirar un modulo, el censo no es solo `grep` de sus llamadores en el firmware: es tambien `grep` de
su nombre en los scripts de compilacion del instrumental. Y cuando el modulo que se retira es la
unica pata de un arnes que resulta ser el unico instrumento de una regla SFTY-x, retirarlo no baja
la cobertura: la pone a cero, en ABORTADO, y con la compuerta parada en la primera fila.**

---

---

### 🔴 N-100 — Cinco afirmaciones marcadas MEDIDO fueron refutadas por el firmware el MISMO dia, y siguen publicadas

**De donde sale:** de cruzar cada censo de comandos de los documentos vigentes contra
`bluetooth.cpp` de las dos puntas, en vez de fiarse de su marca `MEDIDO`.

Este no es el fallo de N-98 —dos documentos que se contradicen— ni el de N-99 —un documento que
nadie toco—. Es el tercero y el mas caro de detectar: **documentos que se midieron bien, se
marcaron `MEDIDO` con razon, y quedaron falsos horas despues porque el firmware avanzo.**

**MEDIDO — lo que el firmware hace hoy** (`d34cfe2` N-78 y `caef8a1` N-82/N-83, los dos del 28/08):

```
01_Firmware/Maestro/src/bluetooth.cpp:191   SET_MODO:MENU          ($ACK :209, y rama propia
                                                                    para DEGRADADO en :196-204)
01_Firmware/Maestro/src/bluetooth.cpp:212   SET_MODO:ALCANCE
01_Firmware/Maestro/src/bluetooth.cpp:223   SET_MODO:INTELIGENTE
01_Firmware/Maestro/src/bluetooth.cpp:234   SET_MODO:DEGRADADO     ($ERR motivado :245, $ACK :250)
01_Firmware/Maestro/src/bluetooth.cpp:330   REINICIAR_RELOJ
01_Firmware/Maestro/src/bluetooth.cpp:295-325  SET_RTC con CINCO ramas distintas
01_Firmware/Esclavo/src/bluetooth.cpp:130   CMD:AMBAR_EMERGENCIA   (y :171 con PIN)
01_Firmware/Esclavo/src/bluetooth.cpp:157-158  FORZAR_ROJO -> $ERR,DESC:RENOMBRADO_USE_AMBAR_EMERGENCIA
```

Y la app los manda: `05_Funcional/App_Semaforo/.../app.js:537` (`AMBAR_EMERGENCIA`), `:602`
(`SET_MODO:MENU`), `:668` (`SET_MODO:DEGRADADO`), con la lista sin PIN en `:189`.

**Las cinco afirmaciones refutadas — se marcan REFUTADAS, no se borran:**

**1. REFUTADA** — *"No hay forma de entrar al Degradado por Bluetooth"*

> `05_Funcional/8_Procedimiento_Modo_Degradado.md:30-31` — *"Y hoy ni siquiera es una puerta:
> **tampoco existe el comando de ida.** No hay forma de entrar al Degradado por Bluetooth"*
>
> `:41` — *"**No existe comando Bluetooth para ENTRAR** en Degradado | `grep DEGRADADO` sobre los
> dos `bluetooth.cpp` devuelve **una sola linea** ... la cadena de estado de `$STATUS`, no un
> comando"*
>
> **REFUTADO por `Maestro/src/bluetooth.cpp:234`.** El `grep` era correcto **cuando se corrio**
> —commit `bdcf03d`, 19:00— y dejo de serlo con `d34cfe2`, el mismo dia.

**2. REFUTADA** — *"no hay `SET_MODO:MENU`"*

> `05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md:296` — *"Desde Bluetooth se alcanzan
> **tres** [de ocho modos]. Y **no hay `SET_MODO:MENU`**"*, con la consecuencia de `:300-308`:
> *"cada modo se convierte en una puerta de un solo sentido"*, y el Anexo `:880-881` pidiendolo como
> trabajo pendiente numero 1.
>
> **REFUTADO por `Maestro/src/bluetooth.cpp:191`**, y con mas cuidado del que el Manual 17 pedia:
> la rama trata aparte el caso `MODO_DEGRADADO` (`:196-204`) para no saltarse el todo-rojo de
> despedida.

**3. REFUTADA** — *"ninguno pone al Esclavo en ambar"*

> `04_Manuales/MANUAL_MANDO_4_RELES.md:439` — *"| **Sin sustituto** | **MEDIDO:**
> `Esclavo/src/bluetooth.cpp` acepta `FORZAR_ROJO` (`:109`, `:124`), `SOLICITAR_PASO` (`:128`),
> `TEST_LEDS` (`:146`) y `SET_RTC:` (`:159`). **Ninguno pone al Esclavo en ambar.** `FORZAR_ROJO` es
> rojo, no ambar, y no revoca nada |"*
>
> `05_Funcional/8_Procedimiento_Modo_Degradado.md:473` — *"el Esclavo **no tiene comando**"*
>
> `05_Funcional/17_...md:291-292` — *"Y el del Esclavo (`Esclavo/src/bluetooth.cpp:124-168`):
> `FORZAR_ROJO`, `SOLICITAR_PASO`, `TEST_LEDS`, `SET_RTC`"*
>
> **REFUTADO por `Esclavo/src/bluetooth.cpp:130` y `:171`** (`CMD:AMBAR_EMERGENCIA`), y por
> `:157-158`, donde `FORZAR_ROJO` **ya no se acepta**: contesta
> `$ERR,DESC:RENOMBRADO_USE_AMBAR_EMERGENCIA`. Es N-83, cerrado en `caef8a1` (`roadmap.md:47`).

**Esta tercera es la que mas cuesta**, y conviene decir por que: la fila `:439` del Manual del mando
es **la que sostiene la decision abierta §3.3 del Manual 17** —*"como se opera el equipo si el ESP32
se cuelga"*—. Publica que **no hay sustituto** del `B·B·B` justo cuando el sustituto ya existe y la
app ya tiene su boton. Un responsable que lea esa fila esta decidiendo entre alternativas con una de
ellas tachada por error.

**4. REFUTADA** — *"`SET_RTC` puede rechazar en silencio y contestar `RESULT:OK`"*

> `05_Funcional/17_...md:334` — *"### 2.5 🔴 `SET_RTC` puede rechazar en silencio y contestar
> `RESULT:OK`"*, con `bluetooth.cpp:173-175` citado, y el Anexo `:884-885` pidiendo el arreglo.
>
> **REFUTADO por `Maestro/src/bluetooth.cpp:295-325`**, que tiene hoy **cinco ramas**:
> `FORMATO_INVALIDO`, `SIN_CRISTAL_VEA_CONSULTA_RELOJ` via `reloj_hayCristal()`, rango fuera de
> calendario, `HORA_PUESTA_SIN_PROPAGAR` y `OK`. Es N-80, y **`ESTADO.md` BLQ-2 ya lo declara
> cerrado**, con una frase que vale para toda esta entrada: *"Un bloqueante que ya no bloquea,
> escrito como si bloqueara, cuesta la misma sesion que uno real"*.

**5. REFUTADA** — la cita del acta de referencia del propio Manual 17

> `05_Funcional/17_...md:5-6` — *"Acta de compuerta de referencia: `evidencia/2026-08-28_compuerta.txt`
> — `15 PASS | 0 FALLA | 0 ABORTADO`, HEAD **`3733544`**, arbol LIMPIO (lo dice la propia acta)"*
>
> **REFUTADO:** ese fichero hoy dice `HEAD : 043860a` (lo reescribio `f25fa57`, el mismo dia). La
> cifra sigue siendo correcta; **la cita ya no reproduce**, que es lo unico que este repositorio
> exige de una cita.

---

#### El HTML huerfano: la misma enfermedad, en el documento que se entrega

`05_Funcional/Guia_Cableado_y_Pruebas_Banco.html` — **81.093 bytes, 1.350 lineas, un solo commit
(`24276ab`)**. Nunca se ha tocado. Ademas de llevar la arquitectura anterior entera (N-98: `HC-05`
vigente en `:435`, ESP32 alternativa en `:439`, `DS3231` en `PB3`/`PB4` en `:654`, alimentacion
desde el riel de `J17` en `:435`), **publica cifras de verificacion que no salen de ninguna acta**.

**MEDIDO — su cabecera** (`:279-284`):

```
commit 614065d
compuerta 14 PASS · 1 FALLA · 0 ABORTADO
Maestro 85,8 % · 56.260 B
Esclavo 63,9 % · 41.872 B
arnes de pantalla 271/271
```

**MEDIDO — contra `evidencia/`:**

| cifra del HTML | acta del 31/08 (`8d76f1e`) | acta del 28/08 que el propio HTML dice usar |
|---|---|---|
| `14 PASS · 1 FALLA` | **`15 PASS \| 0 FALLA \| 0 ABORTADO`** | **`15 PASS \| 0 FALLA \| 0 ABORTADO`** |
| Maestro 85,8 % / 56.260 B | **88,3 % / 57.880 B** | 88,3 % / 57.880 B |
| Esclavo 63,9 % / 41.872 B | **64,4 % / 42.176 B** | 64,4 % / 42.176 B |
| 271/271 pantalla | 271/271 ✅ | 271/271 ✅ |

El HTML afirma en `:1282-1283` que sus cifras *"salen del acta `evidencia/2026-08-28_compuerta.txt`"*.
**No salen de ahi.** Esa acta, en cualquier version de su historia —incluida
`git show 24276ab:evidencia/2026-08-28_compuerta.txt`—, dice `15 PASS | 0 FALLA | 0 ABORTADO`. Y el
buscador se descarto antes de reportar (`CLAUDE.md` §4):

```
grep -l "14 PASS" evidencia/*.txt   ->   evidencia/2026-08-03_compuerta.txt   (unico)
                                          y ese acta dice  10 PASS | 2 FALLA | 0 ABORTADO
```

**Ninguna acta del repositorio ha dicho nunca `14 PASS | 1 FALLA`.** La cifra de la cabecera del
entregable es irreproducible.

**Y el commit al que se ancla no existe para quien lo reciba. MEDIDO:**

```
git merge-base --is-ancestor 614065d HEAD   ->   NO
```

`614065d` esta en el object store —viene del repositorio padre— pero **no es alcanzable desde
`main-nuevo`**. El HTML se describe a la vez como *"acta sobre `614065d`"* (`:471`) y como *"El
firmware de esta entrega es POSTERIOR a `614065d`, y todavia sin commitear"* (`:708`).

**Por que esto no es un detalle de un fichero olvidado:** es **el documento de conexiones que se
entrega**. `generar_entrega_v9_0.py:39` lo mete en el paquete:

```
GUIA_HTML = "Guia_Cableado_y_Pruebas_Banco.html"
```

Lo enlaza `ESTADO.md:41` **desde el 31/08** (`8d76f1e`) —antes no lo enlazaba nadie, y el propio
`ESTADO.md` lo dice—. **`README.md` y `roadmap.md` siguen sin enlazarlo**: `grep "Guia_Cableado"`
sobre los dos da **cero**.

🔴 **Y la ficha del indice describe mal el fichero indexado.** `ESTADO.md:41` dice que el HTML
*"Cubre `J17` (ESP32)"*. El HTML cubre `J17` **con un `HC-05`** y llama al ESP32 alternativa
(`:435`, `:439`). El indice y lo indexado se contradicen, asi que el enlace nuevo **no arregla el
problema: lo publica**.

**Que haria falta para cerrarlo.** Tres cosas, y ninguna es reescribir prosa:

1. Que las cifras de la cabecera del HTML **salgan del acta**, como ya exige la regla de
   `CLAUDE.md` §3 para el README —*"las cifras del README se copian del acta, nunca se escriben a
   mano"*—. Hoy el HTML esta fuera de esa regla porque **ningun pack lo parsea**.
2. Que su commit de referencia sea **alcanzable desde la rama que se publica**.
3. Que los censos de comandos de los tres documentos (`MANUAL_MANDO_4_RELES.md:439`,
   `8_Procedimiento...:30-41` y `:473`, `17_...md:276-292`, `:334`, `:462`, Anexo `:880-885`) se
   **releen del `.cpp`**, o se marquen con la fecha y el commit en que se midieron — que es lo
   minimo para que un lector sepa cuanto vale la marca.

> **LECCION REUTILIZABLE: `MEDIDO` es la marca que este repositorio usa para SALTARSE la
> verificacion, asi que un `MEDIDO` caducado cuesta exactamente la misma sesion que un defecto real
> — y ademas llega blindado.** Es la segunda cara de `CLAUDE.md` §4 —*"lo que TU reportas tambien es
> un instrumento"*— con un agravante que no estaba escrito: alli la causa era **plausible y falsa**;
> aqui era **verdadera y se murio de vieja**, en horas, y ninguna revision la habria pillado leyendo
> el documento, porque el documento es correcto en cada linea. **Lo que distingue una medida viva de
> una muerta no es su contenido: es su FECHA junto a la del fichero medido.** Por eso una cita de
> firmware en un documento lleva `fichero:linea` **y el commit en que se leyo**; sin el, no es una
> medida, es una foto sin fecha.
>
> **Corolario, y es la mitad que se olvida: una medida que se cae se marca REFUTADA, no se borra ni
> se "actualiza en silencio".** Las cinco de arriba nacieron bien y describen decisiones que se
> tomaron por ellas —§3.3 del Manual 17 elige entre alternativas contando con que no hay sustituto
> del `B·B·B`—. Si se corrigen sin dejar rastro, la decision queda en pie sin su premisa.

---

## ⚠️ Observacion de metodo — la parte que impide que esto vuelva

**Nueve de los diez puntos de esta auditoria viven en documentos que NINGUN pack parsea.**

**MEDIDO:** el banco son **38 packs** (`ls 01_Firmware/Simulaciones/banco/packs/*.py` -> 39
ficheros, 38 packs mas `__init__.py`), y los unicos que leen documentacion son tres:

```
banco/packs/documentos_01_cifras_del_acta.py
banco/packs/documentos_02_trazabilidad_sfty.py
banco/packs/documentos_03_trama_status.py
```

Entre los tres vigilan `README.md`, `ESTADO.md`, `OPTIMIZACIONES.md` y el Manual 10 — **y nada mas**.
Fuera del alcance de la compuerta quedan: el Manual 11, el Manual 14, el Manual 3, el
`MANUAL_CONFIGURACION_BLUETOOTH.md`, el `MANUAL_MANDO_4_RELES.md`, el
`8_Procedimiento_Modo_Degradado.md`, el `2_Manual_Hardware_y_Pruebas.md`, el propio Manual 17 y **el
HTML de 81 KB que se entrega**. El commit `2e6baf4` lo dejo escrito de su propio documento sin que
nadie sacara la consecuencia: *"Ningun pack parsea este manual: la compuerta no lo ve."*

**Y el dato duro que lo cierra: la compuerta salio `15 PASS | 0 FALLA | 0 ABORTADO` el 31/08
—`evidencia/2026-08-31_compuerta.txt`, HEAD `8d76f1e`, arbol LIMPIO— con las diez contradicciones
dentro del arbol.** Tres decisiones estructurales con dos respuestas opuestas, un diagrama que
manda cablear un bus I2C contra un LED, cinco `MEDIDO` refutados y un entregable con cifras que no
existen: **cero FALLA, exit code `0`.**

No es un defecto de la compuerta. Es `CLAUDE.md` §3 en su forma mas literal, la que ya se pago con
`Validacion_Respaldo` en N-43:

> **"Un instrumento que no esta en la compuerta no mide nada — y no deja rastro de que falta. Un
> `ABORTADO` al menos grita; un hueco no."**

Aqui el hueco son **nueve documentos**, y entre ellos el que lleva el dibujo que alguien sigue con
el destornillador. Un `0` de la compuerta sobre este arbol significa exactamente lo que `CLAUDE.md`
dice que significa —*los modelos y los arneses de PC no encuentran nada*— y ni una palabra sobre si
los papeles que van al funcional, al auditor y al instalador se contradicen entre si.

**La direccion del arreglo, y es la unica que impide que vuelva:** no son diez correcciones de
prosa. Es **un cuarto pack `documentos_04_*` que censa** —no que lea— tres propiedades comprobables
por texto sobre **todo** `05_Funcional/` y `04_Manuales/`:

1. **Unicidad de decision.** La cadena `"Decision de obra del 28/08"` (y sus variantes) no puede
   aparecer con dos direcciones contrarias. Hoy aparece en tres ficheros y significa dos cosas.
2. **Pines citados contra `pines.h`.** Cualquier documento que escriba `PB0`, `PB8`, `PB14`,
   `PB15`, `GPIO21`... junto a una funcion, tiene que coincidir con el `#define` real. Esto solo
   habria cazado N-99 entero, y N-59 y N-64 antes que el.
3. **Censos de comandos contra `bluetooth.cpp`.** Todo documento que publique una lista de comandos
   marcada `MEDIDO` se compara con los `strcmp(accion, ...)` de las dos puntas. Esto caza los cinco
   `MEDIDO` de N-100 el mismo dia en que caducan, que es cuando cuesta un minuto arreglarlos.

Y, como siempre en este repositorio, **el pack no se da por bueno hasta verlo caer** con el defecto
inyectado en el documento real (`CLAUDE.md` §8.bis): se invierte una frase de decision, se cambia un
pin en un manual, se borra un comando de un censo, y se exige que **baje la cuenta y cambie el
codigo de salida**. Un pack de documentos que nadie ha visto fallar es exactamente el adorno que da
verde del que avisa §8.bis — y esta auditoria es la prueba de que hoy hay diez cosas rojas debajo de
un `0`.

---

*Borrador escrito el 31/08/2026 sobre `main-nuevo` @ `8d76f1e`. Todo lo marcado MEDIDO se repite
abriendo el fichero y la linea que se cita, o volviendo a correr el `git` que se transcribe. Nada de
lo marcado LEIDO se ha comprobado contra hardware — y en este documento no hay ninguna afirmacion
sobre el cobre.*

---

### 🔴 N-99 — El Manual 11 manda cablear el bus I2C del reloj a la entrada de camara y a un LED

**De donde sale:** de bajar al detalle del segundo pinout de `DS3231` de N-98, en vez de anotarlo
como *"otro documento desfasado"*.

`05_Funcional/11_Manual_Instalacion_RTC_DS3231_Bateria.md` es **el unico documento del censo cuyo
seguimiento hace dano fisico**, y es tambien uno de los que **nadie ha tocado**: `git log` sobre el
fichero devuelve **un solo commit, `24276ab`**, y `grep "28/08"` sobre el da **cero coincidencias**.
No lleva aviso, no lleva tachado, no lleva fe de erratas. Se lee como vigente porque no hay nada
que diga que no lo es.

**MEDIDO — lo que el manual manda:**

```
05_Funcional/11_Manual_Instalacion_RTC_DS3231_Bateria.md:5
  "Plan de Contingencia: Modulo Externo DS3231 en pines libres PB0 (SDA) y PB8 (SCL)
   por I2C Software"

05_Funcional/11_Manual_Instalacion_RTC_DS3231_Bateria.md:111
  "│  [ SDA ]  (Datos I2C)   ──────┼─────────┼──► Pin PB0 (I2C Soft SDA)     │"

05_Funcional/11_Manual_Instalacion_RTC_DS3231_Bateria.md:112
  "│  [ SCL ]  (Reloj I2C)   ──────┼─────────┼──► Pin PB8 (I2C Soft SCL)     │"
```

**No es una frase suelta en una cabecera: es un diagrama ASCII de conexion**, del tipo que alguien
sigue con el destornillador en la mano. Y la linea `:5` lo llama *"pines libres"*.

**MEDIDO — lo que esos dos pines son de verdad:**

```
01_Firmware/Maestro/include/pines.h:46
  #define CAM_DEMANDA_PIN    PB0   // -> R64 10K + C25 100nF -> bornera J14 (antirrebote 1 ms)

01_Firmware/Maestro/include/pines.h:63
  #define LED_TESTIGO        PB8   // -> R16 1K -> LED D5. NO es entrada de camara
```

Ninguno de los dos esta libre:

- **`PB0` es la entrada de camara de demanda**, con `R64` 10 kOhm de **pull-DOWN** y `C25` 100 nF
  hasta la bornera `J14`. Un SDA bit-bang sobre ese pin no solo no va a hablar I2C contra un RC de
  1 ms: **mueve la linea que el firmware lee como demanda vehicular**. El efecto no es un reloj que
  no da la hora — es un semaforo que pide paso solo, o que deja de pedirlo.
- **`PB8` es un LED testigo** (`D5` a traves de `R16` 1 kOhm). Es una **salida de aviso, no una
  bornera**, y el propio `pines.h:50-62` explica que se deja en alta impedancia **a proposito**
  porque el sentido del LED no esta trazado. Un SCL contra ese nudo es sacar corriente a un diodo.

**Y ya esta pagado antes en este proyecto, dos veces, con estos mismos dos pines:**

- **N-59 / N-64:** `PB8` estuvo en **cuatro manuales** como *"umbral de tramo"* con el pin **sin
  leer**. `pines.h:53` lo deja escrito: *"Durante meses cuatro manuales lo describieron como
  'umbral de tramo' (N-59) y el firmware le hacia un pinMode que no servia para nada"*.
- **N-67:** la contradiccion de polaridad sobre la linea de camara —el pull-down de 10 kOhm de la
  placa contra el pull-up interno— dejaba el pin en `0,66 V`, que el micro lee **LOW**, o sea
  **demanda permanente desde el arranque sin ninguna camara conectada**. `PB0` es exactamente esa
  linea.

**El resto del repositorio ya lo sabe, y ahi esta la señal:** `13_Manual_Modulo_Expansion_I2C_y_Compras.md`
—el manual hermano, del mismo tema— lleva **fe de erratas fechada el 28/08** justo sobre esto
(`:69-90`: *"LA FILA DE `PB8` DE LA SECCION 4 ERA FALSA"*), y en `:145-146` descarta `PB0` y `PB8`
uno por uno con su motivo. Los dos documentos hablan del mismo bus, del mismo chip y de los mismos
dos pines; **uno se corrigio y el otro no**, y el que no se corrigio es el que trae el dibujo.

**Refutada, no borrada, la premisa que sostenia el diagrama:** *"`PB0`/`PB8` son los unicos pines
libres"* es falsa, y lo dice el propio Manual 13 en `:89` —*"con esa fila cayo tambien su premisa:
`PB0`/`PB8` no eran «los unicos pines libres»"*—, con el censo en su §4.1 y las rutas B y D vivas
en §4.2. Se anota aqui para que no vuelva a proponerse.

**Que haria falta para cerrarlo.** Con la arquitectura del 28/08 el bus I2C **ya no vive en el
STM32**: el `DS3231` cuelga del ESP32 por `GPIO21`/`GPIO22` con pila propia
(`17_...md:89`, `15_Lista...:203`). Asi que el arreglo **no es reelegir ruta**: es marcar el §
entero del Manual 11 como **plan de contingencia retirado**, con su motivo, y dejar en pie lo que
sigue siendo cierto de ese manual —la pila `CR2032` en `VBAT` con `R5` desoldada, `:36`-`:67`, y el
aviso de la `CR2032` sobre circuito de carga, `:114`—. Y **no se borra el diagrama: se tacha**, o
alguien volvera a proponer `PB0`/`PB8` dentro de un mes.

> **LECCION REUTILIZABLE: "pin libre" es una afirmacion sobre el firmware Y sobre el cobre, y un
> manual no la puede sostener sola.** Los dos pines de este diagrama estaban tomados —uno por una
> entrada con RC de placa, otro por un LED con su resistencia—, y las dos cosas se comprueban en
> **una linea de `pines.h` y una del esquematico**. **Corolario de censo: cuando dos manuales
> describen el mismo bus, el mismo chip y los mismos pines, la fe de erratas de uno es una alarma
> sobre el otro.** El Manual 13 se corrigio el 28/08 y el 11 no; nadie cruzo los dos, y el que se
> quedo sin corregir es precisamente el que lleva el dibujo que alguien sigue con el
> destornillador. **Un aviso que solo se pone en el documento que se estaba mirando no es un
> aviso: es un parche.**

---

---

### 🔴 N-98 — Tres decisiones estructurales tienen DOS respuestas opuestas vivas, y las dos dicen "decidido el 28/08"

**De donde sale:** de auditar los cinco "Ordenes" de la seccion B de
`05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md` documento por documento, en vez de
darla por vigente.

El repositorio ya tuvo esta clase de fallo una vez y esta escrito en el propio Manual 17
(`:725`): *"es la decision contraria a la del 28/08. El mismo dia, el mismo documento"*. Se
arreglo **una** copia. **Quedaron dos, y aparecieron dos contradicciones mas de la misma familia.**

---

#### 1. Que se enchufa en `J17` — dos contra uno, y el que pierde lleva el dibujo

**MEDIDO**, tres ficheros abiertos:

```
05_Funcional/15_Lista_de_Compras_Hardware.md:159
    "### 🔄 Decision de obra del 28/08 - VIGENTE: el ESP32 SUSTITUYE al modulo SPP"

05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.md:132-134
    "### ✅ Decision de obra del 28/08: se sigue con el modulo SPP dedicado"
    "Se instala HC-05 / JDY-30, no ESP32."

05_Funcional/Guia_Cableado_y_Pruebas_Banco.html:435,439
    "Ese es el modulo vigente [HC-05 / JDY-30], y es lo que pide la lista de compras"
    "El ESP32 queda como ALTERNATIVA, no como sustituto, y solo entra si no llegan los HC-05"
```

**La frase del `:439` del HTML es literalmente la que el commit `2e6baf4` califico de *"lo contrario
de lo decidido"* al arreglar la lista de compras.** Sigue viva, palabra por palabra, en otro fichero.

**Por que el Manual 10 es el peor sitio donde dejarlo, y no es cuestion de gusto:**

- Es el unico documento de la entrega **con dibujo de conexion del modulo** (`:286`:
  `MODULO BLUETOOTH (HC-05 / JDY-30)   TARJETA -- CONECTOR J17`). El pinout es correcto; el modulo
  que manda enchufar, no.
- Su apartado 1 esta **congelado por escrito** (`:26`, `:148`), y una decision congelada solo vale
  si reabrirla cuesta un documento. Hoy la reabre otro fichero, en silencio.
- Su tabla de tres caminos (`:144-146`) sigue **sin fila decidida**.
- `git log -- 05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.md` devuelve **un solo commit,
  `24276ab`, el raiz**. Nadie lo ha tocado desde que se creo el repositorio nuevo.

#### 2. Donde va el `DS3231` — tres documentos, tres pinouts distintos

**MEDIDO:**

```
05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md:89
    DS3231 sobre el ESP32:  GPIO21 = SDA, GPIO22 = SCL, pila propia   <- replicado en
    05_Funcional/15_Lista_de_Compras_Hardware.md:203 y en ESTADO.md seccion 4

05_Funcional/11_Manual_Instalacion_RTC_DS3231_Bateria.md:5
    "Modulo Externo DS3231 en pines libres PB0 (SDA) y PB8 (SCL) por I2C Software"

05_Funcional/Guia_Cableado_y_Pruebas_Banco.html:654
    "🕐 Reloj DS3231 - pines 40 y 39 - PB4  PB3 - SDA y SCL si se monta el reloj - J17 p1 y p4"
```

Tres destinos distintos para la misma pieza, ninguno marcado como caducado. El del Manual 11 ademas
hace dano fisico y tiene entrada propia: **N-99**.

#### 3. Los cuatro pulsadores: se retiran o se quedan

**MEDIDO:**

```
05_Funcional/17_...md:140
    "Los cuatro pulsadores (PB9, PB13, PB14, PB15) | libera J16"     <- se retiran

ESTADO.md seccion 4
    "Se retiran: la pantalla LCD de las dos puntas, los cuatro pulsadores ... y el mando"

05_Funcional/2_Manual_Hardware_y_Pruebas.md:11
    "| Botonera de 4 pulsadores | ✅ Se queda en AMBOS | Botones 1 a 4 (PB9, PB13, PB14,
     PB15) por el conector J16 |"
```

El `2_Manual` **si** recogio la retirada de la pantalla en la fila de al lado (`:10`), asi que no es
un documento sin tocar: es una tabla de estado **medio actualizada**, que es peor, porque su fila
buena avala a la mala.

---

#### El hallazgo estructural que lo explica: **el censo de la seccion B nacio desfasado**

Esto es lo que convierte tres erratas en un problema de metodo. **MEDIDO** con `git log`:

```
bdcf03d  28/08 19:00  docs: aviso en las dos maniobras que se quedan sin actuador   (Ordenes 3 y 4)
2e6baf4  28/08 19:03  docs: la lista de compras pedia lo contrario de lo decidido   (Orden 1)
3733544  28/08  --    docs: el documento del funcional para la arquitectura del 28/08 (Manual 17)
```

Los dos commits que reparan tres de los cinco Ordenes son **anteriores** al commit del propio
Manual 17. Y sin embargo el Manual 17 dice, en `:716`:

> **"Ninguno de estos ficheros se ha tocado al escribir este documento. Lo que sigue es el censo, no
> el arreglo."**

**Esa frase ya era falsa en el instante en que se escribio.** El propio documento lo avisa a medias
en `:11-13` —*"Habia otros trabajos en vuelo sobre el mismo arbol el dia que se escribio"*—, pero
avisar de que puede haber deriva no es lo mismo que medirla: la lista se publico como censo y se
lee como censo.

**Es `CLAUDE.md` §8.quinquies en directo, en su segunda forma.** Alli el dano de dos agentes sobre
el mismo arbol fue **la historia** (`ff6bd19` con un solo fichero, el acta). Aqui el dano es
distinto y peor: **un documento de arquitectura publica una lista de trabajo pendiente que ya
estaba hecha en parte**. Nadie escribio nada falso a proposito; simplemente ninguno de los dos
agentes miro el arbol del otro antes de publicar un censo.

**Consecuencia practica, y es la que cuesta la sesion:** hoy la seccion B **no se puede usar como
lista de trabajo**. Dos de sus cinco Ordenes (1 y 4) estan hechos, uno (3) esta hecho a medias y
ademas caducado, y solo dos (2 y 5) siguen enteros. Quien la coja de arriba abajo empieza por
reabrir la lista de compras, que es justo el documento que ya esta bien.

**Estado real de los cinco Ordenes, verificado fichero por fichero:**

| Orden | Documento | Estado | Evidencia |
|---|---|---|---|
| 1 | `15_Lista_de_Compras_Hardware.md` | ✅ **YA REPARADO** | `:85` A1 anulada, `:159` decision invertida, `:91` A6, `:361` B1 movido, `:413` repetidor separado |
| 2 | `10_Manual_Modulo_Bluetooth_Telemetria.md` | 🔴 **SIGUE FALSO, intacto** | un solo commit (`24276ab`); `:132`, `:134`, `:139`, `:144-146`, `:286` |
| 3 | `8_Procedimiento_Modo_Degradado.md` | 🟠 **PARCIAL y caducado** | avisos en `:8`, `:187`, `:200`, `:217`, `:276`, `:342`, `:369`, `:452`, `:464`; pero `:30-31`, `:41` y `:473` refutados por firmware — ver **N-100** |
| 4 | `04_Manuales/MANUAL_MANDO_4_RELES.md` | 🟠 **PARCIAL**, una fila hoy falsa | cabecera y tachado de `B·B·B` en `:425` correctos; `:439` refutado — ver **N-100** |
| 5 | `3_Protocolo_Pruebas_Rigurosas.md` | 🔴 **SIGUE FALSO, intacto** | un solo commit; `grep "28/08"` da **cero**; sigue emitido como V8.7 del 01/08 (`:3`) con el Repetidor en el entorno auditable (`:6`) |

**Y el segundo bloque, tambien verificado:** reparados `MAPEO_TARJETA_KICAD.md` (`02d913d`, y
`grep "vacio"` da cero), `1_Manual_Usuario.md` (`:8`, `:85`, `:153`) y `ESTADO.md` (`:37`, `:56`,
`:59`). Parciales `2_Manual_Hardware_y_Pruebas.md`, `9_Manual_Parametrizacion_Camara_IA.md`,
`04_Manuales/MANUAL_CONFIGURACION_CAMARAS_IA.md` y `13_Manual_Modulo_Expansion_I2C_y_Compras.md`
—cuya §4 sigue eligiendo rutas de I2C **sobre el STM32** (`:142-159`), que es lo que la arquitectura
del ESP32 deja sin objeto—. Intactos y falsos `11_Manual_Instalacion_RTC_DS3231_Bateria.md`
(**N-99**), `04_Manuales/MANUAL_CONFIGURACION_BLUETOOTH.md` y
`14_Manual_App_Movil_IOT_VIAL.md` —este ultimo manda `HC-05 / JDY-31` en `:6` y `:45`, y el
`JDY-31` esta **prohibido por nombre** en el Manual 10 §1 por ser BLE (`10_Manual...:39`)—.

**Y una fila que quedo mal en el unico documento que si vigila la compuerta:** `OPTIMIZACIONES.md:60`
publica *"SFTY-6: Timeout de fallback a **12.0s**"* y `:61` deriva de ahi la cuenta de reintentos de
SFTY-7. **MEDIDO:** `Maestro/include/protocolo.h:149` y `Esclavo/include/protocolo.h:149` =
`#define SFTY6_SILENCIO_MS 25000UL`. `ESTADO.md:59` ya lo corrigio con tachado; `OPTIMIZACIONES.md`,
que es **la tabla de trazabilidad `SFTY-x -> codigo -> prueba`**, no. Es N-71 volviendo al sitio
donde mas duele.

**Que haria falta para cerrarlo.** No es reescribir los documentos: es **decidir y anotar la fila**
del apartado 1 del Manual 10 —que su propia tabla `:144-146` deja abierta—, y despues propagar esa
unica decision a los otros dos ficheros con tachado y motivo, como ya se hizo bien en la lista de
compras. Y un pack que ate la cadena literal *"Decision de obra del 28/08"* a **una sola
direccion** en todo `05_Funcional/`: hoy la cadena aparece en tres ficheros y significa dos cosas
contrarias.

> **LECCION REUTILIZABLE: una decision no esta tomada mientras exista una copia que diga lo
> contrario, y arreglar UNA copia es lo que hace que el error se vuelva invisible.** Mientras los
> tres documentos estaban desfasados, cualquiera que abriera dos notaba el desfase. Arreglado uno,
> los otros dos quedan **avalados por su propia coherencia entre si** —y ademas fechados el mismo
> dia y marcados "decidido"—, asi que quien abra el equivocado no comete un error detectable:
> implementa fielmente la decision contraria con un documento que le da la razon. **Corolario de
> proceso: un censo de "que documentos quedan falsos" caduca en horas si otro agente esta tocando
> el arbol, asi que se levanta con `git log` sobre cada fichero en el momento de usarlo, no en el
> momento de escribirlo.**

---

---

### 🔴 N-97 — La camara de demanda no es la misma entrada en las dos puntas: en el Maestro vive dentro del Modo Inteligente y en el Esclavo vive siempre

**De donde sale:** del mismo censo de N-96, al comparar punta contra punta en vez de leer cada una
por separado. Es la familia del `amarillo = false` de mas de SFTY-2 (CLAUDE.md §3.ter): dos puntas
que dicen implementar lo mismo y no lo implementan igual.

---

#### 1. Lo MEDIDO — el mismo `#define`, dos entradas distintas

`pines.h` es identico en las dos puntas (`md5 8791a4c1f9afbe5e0e55adad2959b3eb`, ver N-96), asi que
la diferencia no esta en la declaracion:

```
Maestro/include/pines.h:46   #define CAM_DEMANDA_PIN  PB0   // R64 10K + C25 100nF -> J14
Esclavo/include/pines.h:46   #define CAM_DEMANDA_PIN  PB0   // identico
```

**Donde se configura y donde se lee, MEDIDO:**

| | Maestro | Esclavo |
|---|---|---|
| `pinMode(CAM_DEMANDA_PIN, INPUT)` | `modo_inteligente.cpp:46` — **dentro de `modoInteligente_setup()`** | `main.cpp:288` — **en `setup()`** |
| quien llama a eso | `main.cpp:205`, `case MODO_INTELIGENTE:` del `switch` de cambio de modo. **Unico llamador** (censado con `grep`, no leyendo) | nadie: es el `setup()` del arranque |
| lecturas | `modo_inteligente.cpp:98` y `:136`, las dos dentro del `loop` de ese modo | `main.cpp:350`, en el `loop()` principal |
| como se lee | **nivel**, con antirrebote software de 5 ms: `leerPinCamara()`, `modo_inteligente.cpp:21-30` | **flanco**, sin antirrebote software: `main.cpp:347-354`, `demandaCamaraActual && !demandaCamaraAnt` |

**Consecuencia:** en el Maestro, en Modo Manual, Automatico, Alcance, Hora, Degradado, Ambar o en el
Menu, **el pin `PB0` ni se configura ni se lee**. La camara del Maestro no existe fuera de un modo.
En el Esclavo la camara vive en todos los modos, porque su lectura esta en el `loop()` principal.

Y ni siquiera filtran igual: el Maestro exige nivel alto estable 5 ms (mas el RC de 1 ms de la
placa); el Esclavo se fia **solo** del RC de 1 ms y cuenta flancos. Con un rele de camara que rebote
mas de 1 ms —que es justo lo que el comentario del Maestro dice que puede pasar,
`modo_inteligente.cpp:22-24`— las dos puntas cuentan distinto el mismo gesto.

---

#### 2. Las citas enfrentadas: los documentos hablan de "las camaras" como una sola cosa

> `ESTADO.md:79` — *"…la **radio LoRa** (`USART3`, `J12`) **y las camaras**."*

> `ESTADO.md:117`, fila `FW-CAM` — *"`PB0`/`J14` es hoy **el unico camino de camara con firmware
> probado** (N-67 corregido, `pinMode(INPUT)` y `== HIGH` en **las dos puntas**, pack
> `camara_01_demanda`)."*

> `05_Funcional/17_...md:245-246` — *"La camara se arreglo: `pinMode(CAM_DEMANDA_PIN, INPUT)` y
> deteccion contra `HIGH` (`Maestro/src/modo_inteligente.cpp:19-25`, `:44`; `Esclavo/src/main.cpp:288`,
> `:350`)."*

**Contra lo MEDIDO:** las dos citas son **exactas en lo que afirman** —la polaridad si es `INPUT` y
`== HIGH` en las dos puntas, N-67 se cerro bien— y **el propio Manual 17 imprime la asimetria sin
verla**: cita `modo_inteligente.cpp` para una punta y `main.cpp` para la otra, en la misma linea, sin
que a nadie le llame la atencion que la misma entrada viva en un modo en un lado y en el arranque en
el otro.

Lo que ningun documento dice, y es lo que importa: **que en el Maestro esa entrada esta apagada en
siete de los ocho modos.** Un documento que dice *"las camaras"* describe un sistema simetrico que no
existe.

---

#### 3. Por que esto no lo caza el pack de camara

`banco/packs/camara_01_demanda.py` vigila lo que N-64 y N-67 dejaron: que `PB8` no vuelva a llamarse
`CAM_UMBRAL_PIN` (`:49-68`), que nadie lea `PB8` (`:84-86`), y la polaridad del `pinMode` de
`CAM_DEMANDA_PIN` (`:106`, `re.findall(r"pinMode\s*\(\s*CAM_DEMANDA_PIN\s*,\s*(\w+)\s*\)", codigo)`).

**Busca el `pinMode` por texto, en el codigo de la punta.** Lo encuentra igual este dentro de
`setup()` o dentro de `modoInteligente_setup()`: **el pack no tiene forma de saber quien llama a la
funcion que lo contiene.** Vigila la polaridad, que es lo que le pidieron; no vigila el alcance.

Es exactamente el punto ciego que `barrera_02_dos_puntas.py:5-6` describe para la otra regla —*"este
vigila lo otro, que faltaba: que `semaforo.cpp` DIGA LO MISMO en las dos puntas"*—. Para la barrera
de luz existe ese segundo pack. **Para la camara no existe.**

---

#### 4. Y de paso: hay un CUARTO `pines.h` que no compila ni censa nadie

MEDIDO. `01_Firmware/Semaforos/` es un proyecto completo en el arbol activo —`platformio.ini`,
`src/`, `include/`, `test/`, `compile_commands.json`— con su propio `include/pines.h`, que tambien
declara `BUZZER`. Y:

```
01_Firmware/compuerta.py:88    _ROLES = ("Maestro", "Esclavo", "Repetidor")
01_Firmware/compuerta.py:655   compilar("maestro",   "Maestro")
01_Firmware/compuerta.py:656   compilar("esclavo",   "Esclavo")
01_Firmware/compuerta.py:657   compilar("repetidor", "Repetidor")
```

**`Semaforos` no esta.** No se compila, la guarda de rutas no lo censa, y ningun pack lo lee. No es
un `ABORTADO` —nadie intento medirlo y fallo—: es un **hueco**, la clase que no deja rastro
(CLAUDE.md §3). Si es legado, su sitio es `99_Legacy/`, donde ya viven tres copias suyas; si esta
vivo, le falta un papel en `_ROLES`. Hoy no es ninguna de las dos cosas, y un `pines.h` sin vigilar
en el arbol activo es la clase de fichero que alguien acaba editando por error creyendo que es el
bueno.

*(No se ha tocado nada: solo se mide y se anota.)*

---

> **LECCION REUTILIZABLE: dos puntas pueden pasar el mismo pack con el mismo texto y no tener la
> misma entrada, porque un pack que busca una llamada por texto no sabe QUIEN llama a la funcion que
> la contiene. El alcance de un `pinMode` —arranque incondicional, o dentro del `setup()` de un modo—
> no se ve leyendo la linea: se ve censando los llamadores con `grep`, que es un segundo censo y hay
> que hacerlo aparte. Y su corolario para los documentos: cuando una nota cita `fichero_A.cpp` para
> una punta y `fichero_B.cpp` para la otra en la misma frase, esa asimetria de rutas ya esta escrita
> delante de quien la publica — se comprueba antes de resumir las dos como "las camaras".**

---

### 🔴 N-96 — La barrera de salidas dice gobernar OCHO pines de luz y gobierna SEIS: `ROJO_PEATON`, `VERDE_PEATON` y `BUZZER` estan declarados y muertos, y el pack que respalda la regla es vacuamente cierto sobre ellos

**De donde sale:** de censar la superficie de entrada y salida del firmware pin a pin y cruzarla con
lo que los documentos prometen (CLAUDE.md §3.ter). No lo pregunto nadie.

---

#### 1. El descarte del buscador, primero — porque sin el esto no es un hallazgo

Un *"no aparece"* no vale en este repositorio hasta haber descartado al buscador (CLAUDE.md §4).
Tres controles, y los tres son la parte reutilizable de este N-x:

**Control 1 — los dos `pines.h` son el MISMO fichero, asi que basta censar una lista.** MEDIDO:

```
md5sum 01_Firmware/Maestro/include/pines.h  ->  8791a4c1f9afbe5e0e55adad2959b3eb
md5sum 01_Firmware/Esclavo/include/pines.h  ->  8791a4c1f9afbe5e0e55adad2959b3eb
```

**27 `#define` con pin fisico en cada punta**, identicos. Cualquier asimetria que aparezca despues
no puede venir de la declaracion: viene del uso.

**Control 2 — se busco por macro Y por literal de pin crudo.** Buscar solo `BUZZER` seria confiar
en que nadie escribio `digitalWrite(PB1, ...)` a pelo. Se hicieron las dos pasadas sobre
`src/` + `include/` de las dos puntas, recursivo, `.cpp` y `.h`, excluyendo `.pio/` (libreria
ajena) y `.cache/`:

```
grep -rn "\bBUZZER\b"  Maestro/src Maestro/include Esclavo/src Esclavo/include   ->  0
grep -rn "\bPB1\b"     (los mismos directorios)                                 ->  solo pines.h:20
grep -rn "\bROJO_PEATON\b|\bVERDE_PEATON\b"                                     ->  solo un COMENTARIO
grep -rn "\bPA6\b|\bPA7\b"                                                      ->  solo pines.h:15-16
```

**Control 3 — el buscador SI sabe encontrar.** El mismo patron sobre un pin vivo:

```
grep -rc "\bROJO1\b" Maestro/src/semaforo.cpp   ->  2   (pinMode :193 y digitalWrite :49)
```

Y ampliado al repositorio entero —sin `.pio/`, sin `.git/`— `BUZZER` aparece en **6 `pines.h`,
5 `.kicad_pcb` y `roadmap.md`**, y en **ningun `.cpp` de ningun proyecto**. La palabra existe, el
grep la encuentra donde esta, y no esta en el firmware.

---

#### 2. Lo MEDIDO: `escribirPines()` escribe seis, no ocho

`Maestro/src/semaforo.cpp:48-54` y `Esclavo/src/semaforo.cpp:48-54`, la funcion entera:

```c
digitalWrite(ROJO1, rojo);          digitalWrite(ROJO2, rojo);
digitalWrite(AMARILLO1, amarillo);  digitalWrite(AMARILLO2, amarillo);
digitalWrite(VERDE1, verde);        digitalWrite(VERDE2, verde);
```

**Seis.** Y `semaforo_setup()` (`:193-198` en las dos puntas) hace `pinMode` a esos mismos seis.

| pin | nombre | bornera | `pinMode` | leido | escrito | veredicto |
|---|---|---|---|---|---|---|
| `PA6` | `ROJO_PEATON` | `J11` | **NO** | **NO** | **NO** | 🔴 declarado y muerto |
| `PA7` | `VERDE_PEATON` | `J9` | **NO** | **NO** | **NO** | 🔴 declarado y muerto |
| `PB1` | `BUZZER` | `J13` | **NO** | **NO** | **NO** | 🔴 declarado y muerto |

Los tres, iguales en las dos puntas. `ROJO_PEATON` y `VERDE_PEATON` aparecen **una sola vez** fuera
de `pines.h` en todo el firmware, y es dentro de un comentario: `Maestro/src/main.cpp:35`. `BUZZER`
no aparece ni ahi.

---

#### 3. Las citas enfrentadas — tres documentos, y no pueden ser ciertos a la vez

**Lo que dice la regla permanente:**

> `CLAUDE.md` §6 — *"**Solo `semaforo.cpp` escribe pines de luz.** Los ocho: `ROJO1/2`,
> `AMARILLO1/2`, `VERDE1/2`, `ROJO_PEATON`, `VERDE_PEATON`. Todo pasa por su `escribirPines()`
> estatico"*

**Lo que dice el propio fuente, con la palabra CUSTODIA dentro:**

> `Maestro/src/main.cpp:34-35` — *"NINGUN pin de luz se escribe fuera de semaforo.cpp. Los OCHO,
> incluidos ROJO_PEATON y VERDE_PEATON, que estaban sin custodia."*

**Contra lo MEDIDO:** `semaforo.cpp:48-54` escribe seis. `PA6` y `PA7` no pasan por
`escribirPines()` porque **no pasan por ningun sitio**.

La frase de §6 es **literalmente cierta y vacuamente cierta a la vez**: ningun pin de luz se escribe
fuera de `semaforo.cpp`, en efecto — dos de ellos tampoco dentro. La palabra *"custodia"* sugiere
que hay algo vigilado; lo que hay es un pin que nadie toca.

**Y sobre el buzzer, dos documentos lo dan por vivo y uno lo declara muerto:**

> `ESTADO.md:79` — *"El STM32 sigue siendo el controlador del semaforo. Conserva las **8 luces**
> (`J3`-`J9`, `J11`), la **barrera** (`PB2`, `J15`), el **buzzer** (`PB1`, `J13`), la **radio LoRa**
> (`USART3`, `J12`) y las camaras."*

> `05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md:68-78`, seccion **1.2 Que se queda en
> el STM32**, encabezada en `:70` por *"Todo esta MEDIDO en `01_Firmware/Maestro/include/pines.h`"* —
> `:76` *"| Rojo peaton / Verde peaton | `PA6` `PA7` | `J11` `J9` | `pines.h:15-16` |"* · `:78`
> *"| Buzzer | `PB1` | `J13` | `pines.h:20` |"*

> `OPTIMIZACIONES.md:1427` — *"Es hardware pagado y muerto, igual que el semaforo peatonal
> (`PA6`/`PA7`) y el buzzer (`PB1`)."*

**El que coincide con la medida es `OPTIMIZACIONES.md`.** El hardware muerto no es el hallazgo: ya
estaba escrito bien en un sitio. El hallazgo es que otros dos documentos lo publican como funcion
conservada.

> ⚠️ **Y el "MEDIDO" del Manual 17 §1.2 es lo que lo hace peor, no mejor.** Lo medido es que la
> linea existe en `pines.h` — cierto, repetible, y **no es lo que el titulo de la tabla promete**.
> La tabla se llama *"Que se queda en el STM32"*, que un lector entiende como *funcion que el equipo
> conserva*. Tres de sus siete filas —peatonal, buzzer, y la camara del Maestro de N-97— son
> declaraciones sin firmware detras. **Una marca MEDIDO responde a la pregunta que se le hizo, no a
> la que el lector cree que se le hizo.**

---

#### 4. Lo grave: el pack que respalda la regla NO PUEDE detectarlo

`01_Firmware/Simulaciones/banco/packs/barrera_01_pines_de_luz.py`, MEDIDO linea a linea:

- **Solo mide propiedades negativas.** `:81-97` recorre los `.cpp` de cada punta saltandose
  `PERMITIDOS = ("semaforo.cpp",)` (`:52`) y busca *fugas* hacia fuera. **En ningun sitio comprueba
  que un pin de luz declarado sea escrito DENTRO.** Un pin que nadie escribe pasa la barrera por
  definicion: no puede fugarse lo que no se mueve.
- **La guarda de recuento acepta la perdida.** `:74` — `len(luces) >= 6`. Su propio regex (`:44`,
  `^(ROJO|AMARILLO|VERDE)`) devuelve **8** al correrlo sobre `pines.h` — MEDIDO, ejecutado. **Si
  alguien borrara manana los dos `#define` peatonales, el pack seguiria en verde**, y el mensaje de
  exito (`:75-76`) seguiria diciendo *"todos entran bajo custodia"*.
- **El control negativo nunca ha ejercido un peatonal.** `:102-108` inyecta la fuga sobre
  `luces_m[0]`, que es `ROJO1`: un pin **vivo**. El control demuestra que el regex casa; no demuestra
  que la regla sepa distinguir un pin gobernado de uno abandonado.

El pack cumple exactamente lo que promete su `DESCRIPCION` (`:37`, *"ningun pin de luz se escribe
fuera de semaforo.cpp"*). Lo que sobra es la lectura de `CLAUDE.md` §6 y de `main.cpp:34-35`, que
convierten esa propiedad negativa en *"los ocho estan gobernados"*.

Es el patron de N-51 con otra cara: **un `PASS` de algo que nadie ha visto fallar nunca**, y un
numero —`>= 6`— que no coincide con el que el fichero declara.

---

#### 5. Lo que hay que decidir, y no lo decide el firmware

Las tres salidas tienen canal de potencia completo en la placa (`OPTIMIZACIONES.md:1486` para el
buzzer: `R55`+`R54` -> opto `U13` -> MOSFET `Q8` -> bornera `J13`). Que se implementen o no es
decision de operacion. **Lo que no es opcional es que los documentos digan cual de las dos cosas
es.** Hoy hay tres respuestas publicadas y solo una coincide con el codigo.

---

> **LECCION REUTILIZABLE: una regla de seguridad enunciada en NEGATIVO —"nadie escribe X fuera de
> aqui"— es vacuamente cierta sobre todo sujeto que nadie escribe, y su pack no puede notar la
> diferencia entre un pin gobernado y uno abandonado. Cuando la regla se resume como "los N estan
> bajo custodia", hace falta la mitad POSITIVA: que cada sujeto declarado aparezca dentro de la
> puerta unica. Y el sintoma que lo delata sin leer el pack es una comparacion de recuento con
> holgura —`>= 6` sobre una lista que declara 8—: una guarda que acepta menos sujetos de los que
> existe no esta contando, esta permitiendo.**

---

---

### 🟠 N-95 — `PA8` sobrevivio a su motivo, y el comentario que lo justifica describe un equipo que ya no existe

**De donde sale:** de la pregunta *"el desacoplo de `PA8`, ¿sigue siendo una barrera viva o quedo
como resto de la epoca de `PA9`/`PA10`?"*. Se midio antes de leer lo que dijera ningun documento, y
salieron **dos** afirmaciones del fuente que el propio fuente contradice.

**MEDIDO — netlist del `.kicad_pcb`, extraido por huella:**

```
   U2 pad1 (RO)   -> red /PA10   ->  U1 pad 31  (PA10)
   U2 pad2 (~RE)  -> red /PA8   \
   U2 pad3 (DE)   -> red /PA8   /->  U1 pad 29  (PA8)    <- un solo nivel manda sobre las dos mitades
   U2 pad4 (DI)   -> red /PA9    ->  U1 pad 30  (PA9)
   U2 pad6/pad7   -> J10 pin1 / pin2

   U3 pad1 -> /PB11 · pad2,pad3 -> /PB12 · pad4 -> /PB10 · pad6,pad7 -> J12
```

Confirmado de paso que la correccion `U3` -> `U2` que arrastran los comentarios **es la buena**: `U3`
es el de la radio LoRa y no toca el `USART1`. Eso ya no esta en disputa.

**MEDIDO — firmware, identico en las dos puntas** (`Maestro/src/bluetooth.cpp:68-69`,
`Esclavo/src/bluetooth.cpp:76-77`):

```cpp
pinMode(RS485_IN_DE_RE, OUTPUT);
digitalWrite(RS485_IN_DE_RE, HIGH); // Apaga el receptor RO de U2 y libera PA10 al modulo Bluetooth
```

#### Refutacion 1 — el motivo escrito encima de la linea es falso

> **REFUTADO.** `Maestro/src/bluetooth.cpp:69` afirma:
> *"Apaga el receptor RO de U2 y **libera PA10 al modulo Bluetooth**"*.
> Y `Maestro/include/pines.h:109` lo repite en el `#define`:
> *"HIGH: apaga el receptor de U2 y **libera PA10** (el TX de U2 queda activo)"*.
>
> **Contra:** `Maestro/src/bluetooth.cpp:28` — `static HardwareSerial SerialBT(PB7, PB6);`
>
> **El modulo Bluetooth no esta en `PA10` desde N-76. No hay nada que liberar.** El beneficio se fue
> con el remapeo a `J17`; el coste se quedo. La linea sobrevivio a su numero y el comentario le
> quedo encima, **con la autoridad de una cuenta hecha**.

#### Refutacion 2 — el efecto que el fuente describe ya no puede ocurrir

Esta es la que no estaba vista, y es la peor de las dos porque **es la que alguien usaria para
decidir si toca `J10`**.

> **REFUTADO.** `Maestro/src/bluetooth.cpp:60-61` y `Maestro/include/pines.h:105-106` afirman:
> *"U2 vuelca **la telemetria** por J10 de forma permanente"* / *"con PA8 en HIGH ... **J10 emite la
> telemetria** de forma permanente y no puede recibir nunca"*.
>
> **Contra, MEDIDO:** el `DI` de `U2` es `PA9`, y sobre todo `01_Firmware/` (excluyendo
> `Simulaciones/`) **no hay un solo `pinMode` ni `digitalWrite` sobre `PA9` ni `PA10`**:
>
> ```
> $ grep -rn "RS485_IN_TX\|RS485_IN_RX" --include=*.cpp --include=*.h 01_Firmware/
> Maestro/include/pines.h:98   #define RS485_IN_RX     PA10      <- solo el #define
> Maestro/include/pines.h:99   #define RS485_IN_TX     PA9       <- solo el #define
> Maestro/src/protocolo.cpp:20 // (RS485_IN_RX, RS485_IN_TX) = ...  <- solo un comentario
> Esclavo/include/pines.h:98   ...  (identico)
> Esclavo/src/protocolo.cpp:20 ...  (identico)
> ```
>
> `protocolo_setup()` retiro la apertura de `AiBus` (N-76) y N-86 retiro el objeto entero
> (`Maestro/src/protocolo.cpp:16-46`), y el `USART1` se fue a `PB6`/`PB7`. **El `DI` de `U2` es una
> entrada flotante.** Por `J10` no sale telemetria: sale **un nivel indeterminado**. La frase describe
> el equipo de antes de N-76 y **se ha copiado a dos ficheros**.

*(El Manual 10 §2.5 —`05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.md:361-430`— ya reclasifico
`PA8` como **"RESIDUO PENDIENTE DE REVISAR"** el 28/08 y observa lo del `DI` sin gobierno en `:420-427`.
**LEIDO**, y coincide con lo medido aqui de forma independiente. Lo que ese manual no cubre, y es lo
que este N-x anade, es que **los comentarios del fuente siguen diciendo lo contrario** en cuatro
sitios: `bluetooth.cpp:60-61` y `:69` y `pines.h:105-106` y `:109`, por duplicado en las dos puntas.
Un manual corregido y un fuente sin corregir es peor que los dos mal: quien lee el `.cpp` no sabe que
hay un manual que lo desmiente.)*

#### El matiz que impide el arreglo obvio

La tentacion, leidas las dos refutaciones, es borrar el `digitalWrite`. **Es la respuesta
equivocada.**

| `PA8` | receptor (`RO` -> `PA10`) | transmisor (`DI` <- `PA9`, salida a `J10`) |
|:---:|---|---|
| `LOW` | escuchando: `U2` **conduce** `PA10` desde fuera | apagado |
| **`HIGH`** *(hoy)* | Hi-Z — `PA10` libre, **que ya no le hace falta a nadie** | **encendido permanente sobre el par A/B de `J10`, con el `DI` flotando** |
| **sin configurar** *(si se borra la linea)* | `~RE` y `DE` **flotan**: el `MAX3485` queda en estado indefinido | idem |

`PA8` **tiene que quedar en un nivel determinista**; lo que hay que decidir es **cual**. Borrar la
linea deja las dos mitades del transceptor al aire, que es peor que cualquiera de los dos niveles.

> **La pregunta abierta no es "se borra o se queda": es "HIGH o LOW", y hoy `HIGH` no tiene ningun
> argumento a favor.** Con `HIGH`, el transmisor de `U2` esta permanentemente tomado sobre `J10` con
> una entrada flotante detras — que es literalmente el fallo del repetidor del 31/07/2026 ya escrito
> en `01_Firmware/TROUBLESHOOTING.md:48` y `:55`: *"si un DE/RE se queda permanentemente en alto, esa
> linea queda bloqueada en ambos sentidos"*. Alli fue una averia; aqui dejo de ser una decision y
> volvio a ser lo que era alli.

**A quien bloquea y a quien no:**

- **Al enlace del ESP32: a NADIE.** `U2` no toca `PB6` ni `PB7` en ninguna de sus cuatro patas
  (MEDIDO arriba). Esto **no** es un bloqueante de N-94 ni del firmware del ESP32.
- **A `J10`: si.** El puerto RS-485 de `J10` no esta libre, esta **tomado y sordo**. Y `J10` es
  precisamente el segundo puerto del que depende la idea de portar el repetidor a esta misma placa
  (`03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md` §5, que lo lista como *"hoy esta vacio, asi que el
  segundo puerto no necesita ni un hilo nuevo"* — **LEIDO, y es optimista**: el hilo no hace falta, el
  cambio de firmware si).
- **Que nivel presenta hoy `J10` en la placa: SIN VERIFICAR.** No se ha medido nunca, y no se afirma
  aqui. Es una de las cosas que la sesion de banco tiene que mirar.

**Como se cierra:** su propio `N-x`, con su pack, no colado dentro de otro cambio. Y la parte que
**si se puede hacer ya y no toca comportamiento**: corregir los cuatro comentarios refutados
—`bluetooth.cpp:60-61`, `bluetooth.cpp:69`, `pines.h:105-106`, `pines.h:109`, en las dos puntas—,
porque hoy apuntan a quien venga a medir en la direccion equivocada.

> ⚠️ **Y no viajan solos: hay una tercera afirmacion caduca del mismo remapeo.**
> `Maestro/include/reloj.h:10` sigue diciendo *"el I2C por hardware esta copado: **PB6/PB7 los usa la
> LCD**"*, y `pines.h:88-89` sigue definiendo `LCD_PSB PB6` / `LCD_RST PB7`. **MEDIDO:** el consumidor
> real de esos dos pines es `SerialBT` desde N-76, y `lcd.cpp:29` construye el `U8G2` con
> `U8X8_PIN_NONE` y solo `SCLK`/`SID`/`CS` — **cero llamadas a `LCD_PSB` y `LCD_RST`**. Los `#define`
> no rompen nada porque nadie los usa; el comentario de `reloj.h` si engana, porque **razona una
> decision de arquitectura** —por que el RTC no va por I2C— sobre un hecho que dejo de ser cierto.

> **La regla que queda: cuando un cambio mueve un periferico de pines, lo que hay que censar no son
> los `#define` que se quedan huerfanos —esos son inertes— sino los COMENTARIOS QUE EXPLICAN POR QUE.**
> Un `#define` sin llamadores no hace dano; una frase que justifica una decision con un hecho caducado
> **sigue tomando decisiones**, porque el siguiente la lee y no vuelve a medir. Aqui fueron cinco
> frases en cuatro ficheros y las dos puntas, todas correctas el dia que se escribieron, todas falsas
> hoy, y **todas con la palabra que las blinda: `MEDIDO`, `en la PCB`, `trazado red por red`**. El
> censo de un remapeo es `grep` del nombre del pin viejo —`PA9`, `PA10`— **en los comentarios**, no
> solo en el codigo.

---

### 🔴 N-94 — El transporte del enlace `J17` no lo vigila ningun pack, y el contrato de bytes que el ESP32 tiene que cumplir no esta escrito en ningun sitio

**De donde sale:** de validar si §1.4 del Manual 17 —la tabla pin a pin del enlace ESP32 <-> STM32—
es implementable exactamente como esta escrita. **Lo es: las siete filas coinciden.** Lo que aparecio
al comprobarlo es lo de siempre en este repositorio: la cifra estaba bien y **nadie la vigilaba**.

**Primero, lo que SI esta bien, porque un hallazgo empieza descartando al buscador (CLAUDE.md §4).**
`§1.4` se verifico contra tres fuentes independientes y las tres casan:

```
MEDIDO — fuente:
  Maestro/src/bluetooth.cpp:28   static HardwareSerial SerialBT(PB7, PB6);
  Esclavo/src/bluetooth.cpp:26   static HardwareSerial SerialBT(PB7, PB6);
  Maestro/src/bluetooth.cpp:70   SerialBT.begin(9600);
  Esclavo/src/bluetooth.cpp:78   SerialBT.begin(9600);

MEDIDO — framework, NO el comentario del .cpp:
  C:/.platformio/.../cores/arduino/HardwareSerial.h:111
      HardwareSerial(uint32_t _rx, uint32_t _tx, ...)      <- el PRIMER argumento es RX
  C:/.platformio/.../cores/arduino/HardwareSerial.h:116-119
      void begin(unsigned long baud) { begin(baud, SERIAL_8N1); }

MEDIDO — netlist del .kicad_pcb (185 huellas, extraido balanceando parentesis, no con grep de token):
  J17 pad 2 -> /RST        U1 pad 43 -> /RST        (LQFP48 pad 43 = PB7)
  J17 pad 3 -> /RS(A0)     U1 pad 42 -> /RS(A0)     (LQFP48 pad 42 = PB6)
  J17 pad 7 -> GND         J17 pad 9 -> GND
  J17 pad 1 = /CS · 4 = /SCL · 5 = /SI · 6 = /3.3V · 8 = /3.3V · 10-13 sin red
```

`GPIO17` (TX2) -> `J17` p2 -> `PB7` **RX** del micro; `GPIO16` (RX2) <- `J17` p3 -> `PB6` **TX**.
**El cruce esta bien puesto en el documento y bien puesto en el cobre.** Una sola cautela de
etiqueta: **`8N1` no lo elige nadie en este repositorio** — sale del valor por defecto de
`HardwareSerial::begin(baud)`. Es cierto, pero es una herencia de libreria, no una decision escrita.

> 🔴 **Y aqui esta el hallazgo: NADA de lo anterior lo comprueba el banco.** Censo sobre los **38
> packs** de `01_Firmware/Simulaciones/banco/packs/`:
>
> ```
> $ ls packs/*.py | grep -v __init__ | wc -l
> 38
> $ grep -rln "SerialBT\|HardwareSerial\|PB6\|PB7\|J17\|begin(9600)\|USART1" packs/*.py modelos/*.py
> packs/flash_01_lastre.py
> ```
>
> **Un solo fichero, y no cuenta.** `flash_01_lastre.py:180` contiene la cadena
> `" el I2C por hardware esta copado: PB6/PB7 los usa la LCD "` **dentro de un
> `control_negativo`**, como texto sintetico para demostrar que una mencion en prosa no es un uso del
> bus. No lee el firmware: **es una cadena literal escrita dentro del pack**.
>
> **Siete packs SI abren `bluetooth.cpp`** —`app_01_comandos`, `app_02_modos_simetricos`,
> `app_03_sin_ok_mudo`, `documentos_03_trama_status`, `esclavo_06_no_abre_paso`,
> `esclavo_07_ambar_emergencia`, `maestro_08_set_tiempos`— y **ninguno de los siete mira el
> transporte**: leen comandos del despachador, la cadena de formato de `$STATUS`, las ramas de
> `$ACK`/`$ERR` y los limites de `SET_TIEMPOS`. Todos miran **lo que se dice**; ninguno mira **por
> donde y a que velocidad se dice**.

**Que significa en la practica:** hoy alguien puede escribir `SerialBT(PA10, PA9)` o
`SerialBT.begin(115200)` en las dos puntas y **la compuerta sale en verde, el banco da `411/411` y el
acta lo firma**. El equipo se queda mudo en la calle y ningun instrumento lo dijo. Es exactamente el
hueco de CLAUDE.md §3: *un instrumento que no esta en la compuerta no mide nada — y no deja rastro de
que falta*. **Un `ABORTADO` grita; un hueco no.** Y este hueco es especialmente caro ahora, porque
N-76 acaba de mover ese puerto y el 28/08 acaba de decidir que por ahi entra el ESP32: **es el unico
cable entre el controlador del semaforo y su unica superficie de mando futura.**

#### El contrato de bytes, que tampoco esta escrito en ningun sitio

`§1.4` da los pines y la velocidad, y ahi se acaba. **Todo lo demas que el firmware del ESP32 tiene
que cumplir esta implicito en el `.cpp` y no aparece en ningun documento.** Deducido del fuente,
MEDIDO linea por linea, identico en las dos puntas:

| regla | de donde sale | consecuencia si el ESP32 no la respeta |
|---|---|---|
| **Terminador `\r` o `\n` obligatorio** | `Maestro/src/bluetooth.cpp:391` (`if (c == '\n' \|\| c == '\r')`) · `Esclavo/src/bluetooth.cpp:297` | sin terminador **`procesarComando()` no se llama nunca**. El equipo no contesta y no hay error: el comando se queda en el buffer |
| **Maximo 63 caracteres utiles antes del terminador** | `btBufIn[64]` (`Maestro:31`, `Esclavo:29`) con la guarda `btIdxIn < sizeof(btBufIn) - 1` (`Maestro:397`, `Esclavo:303`) | **descarte SILENCIOSO del caracter 64 en adelante**, y el despachador recibe una linea truncada que casara con un `strcmp` equivocado o con ninguno |
| **El STM32 NO valida el checksum de entrada** | `procesarComando()` empieza en `Maestro/src/bluetooth.cpp:135` con `strcmp(cmd, "CMD:FORZAR_ROJO")` — no hay lectura de `*XX` en ninguna rama | el ESP32 **puede** mandar comandos sin checksum, pero tambien significa que **el enlace no tiene deteccion de error en el sentido ESP32 -> STM32**. Un byte corrompido no se rechaza: se compara |
| **El STM32 SI lo emite, siempre** | `enviarTramaConCrc()`, `Maestro/src/bluetooth.cpp:42-48`: `snprintf(tramaCompleta, ..., "%s*%02X\r\n", payload, crc)` | el ESP32 **tiene que verificarlo** en el sentido STM32 -> ESP32, o se estara fiando de tramas sin comprobar |
| **XOR-8 saltando el `$` inicial** | `calcularChecksum()`, `Maestro/src/bluetooth.cpp:33-40`, invocado como `calcularChecksum(payload + 1)` en `:43`. Recorre hasta `'\0'` **o hasta `'*'`** | un ESP32 que calcule el XOR incluyendo el `$` rechazara **todas** las tramas buenas |
| **Telemetria no solicitada cada 1000 ms** | `Maestro/src/bluetooth.cpp:403` (`if (ahora - tUltimaTelemetria >= 1000)`) | el enlace **no es pregunta-respuesta**: llegan `$STATUS` sin pedirlos, mas `$ALARM` y `$EVENT` asincronos. Un parser que espere respuesta a su comando leera un `$STATUS` como respuesta |
| **La autenticacion es un literal en claro** | `strncmp(cmd, "CMD:PIN:1234:", 13)`, `Maestro/src/bluetooth.cpp:166` | no cambia por el ESP32, pero el ESP32 pasa a ser **quien lo transporta**, y eso es una decision de seguridad que hereda |

> ⚠️ **Correccion de una cifra propia, dentro de este mismo N-x.** El primer informe de esta
> validacion publico *"maximo 62 caracteres"*. Es **falso**: la guarda es `btIdxIn < 63`, asi que se
> escriben los indices `0..62` —**63 caracteres**— y el `'\0'` cae en el `63`. Se corrige aqui en vez
> de en silencio, porque **una cifra que desaparece vuelve a proponerse** (CLAUDE.md §4). Y la
> leccion de fondo es la de siempre: la cuenta se hace sobre la condicion del `if`, no sobre el
> tamano del array.

**Que hace falta para cerrarlo.** Tres cosas, y las tres antes de soldar el primer hilo:

1. **Un pack que fije el transporte** — que relea del C++ los pines de la declaracion de `SerialBT`,
   la velocidad de `begin()` y que las **dos puntas** sean identicas (`bluetooth.cpp:24` del Esclavo
   ya declara esa simetria como intencion: *"IDENTICO AL MAESTRO A PROPOSITO"*, y hoy nada la
   comprueba). **Sin valor por defecto**, releido en cada corrida.
2. **Y verlo fallar antes de conectarlo** (CLAUDE.md §8.bis): se inyecta `SerialBT(PA10, PA9)` en el
   `.cpp` real, se exige que **baje la cuenta y cambie el codigo de salida**, y se restaura
   verificando con `git diff HEAD` vacio.
3. **Escribir el contrato de bytes** —las siete filas de arriba— en `§1.4` del Manual 17 o en el
   Manual 10, porque hoy quien escriba el firmware del ESP32 tiene que **deducirlo leyendo
   `bluetooth.cpp`**, y las dos reglas que muerden —el terminador y el descarte silencioso a los 63
   caracteres— fallan **sin sintoma**: no hay error, no hay `$ERR`, no hay nada. El equipo
   simplemente no contesta.

**Y un dato que NO es un bloqueante pero conviene tener escrito:** al STM32 **le da exactamente igual
que hay al otro lado**. Se grepearon `AT+`, `HC-05`, `HC05`, `JDY`, `SPP`, `pairing` y `emparej` sobre
`Maestro/src`, `Maestro/include`, `Esclavo/src` y `Esclavo/include` —y el buscador **si encuentra
cosas**, cinco aciertos de `emparej` y tres de `115200`, asi que sabia buscar—: **cero comandos AT,
cero secuencia de configuracion, cero pin de KEY/EN**. La unica mencion a `AT+NAME` esta en un
comentario de documentacion, `Maestro/include/identidad.h:42`. `bluetooth_setup()`
(`Maestro/src/bluetooth.cpp:66-71`) hace tres cosas y ninguna es del modulo. **Sustituir el `HC-05`
por un ESP32 no cuesta ni una linea de firmware del STM32** — cuesta exactamente el pack que no
existe.

> **La regla que queda: un contrato que solo vive dentro de un `.cpp` no es un contrato — es una
> arqueologia que el siguiente tiene que hacer.** Y su otra mitad, que es la que este repositorio ya
> conoce con otro disfraz: **el transporte de un enlace se vigila con un pack, igual que su
> contenido.** Siete packs leen lo que dice `bluetooth.cpp` y ninguno por donde lo dice; el dia que
> alguien mueva el puerto, los siete seguiran en verde midiendo el mensaje de un equipo mudo.

---

---

## 7. Lo anterior

**Este roadmap arranca el 31/08/2026.** El historico del proyecto —los `N-1` a `N-93`, las versiones
V8.0 a V8.9 y las auditorias de agosto— **no se ha borrado**: vive en el `git log` de este
repositorio y en el remoto `padre` (`git log padre/main -- roadmap.md`). No se arrastra aqui porque
un roadmap que crece por acumulacion deja de servir para decidir, que es para lo unico que existe.

**Lo que si se conserva y no se pierde nunca son las reglas**: viven en `CLAUDE.md`, que es el
fichero que se lee solo en cada sesion. Si un `N-x` de los viejos dejo una regla, esa regla esta
alli — y si no lo esta, es que no la dejo.
