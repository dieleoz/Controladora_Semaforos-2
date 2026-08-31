# 19 — Especificación de la PLACA PORTADORA del ESP32

**Para quien la dibuja y quien la fabrica.**
**Fecha:** 31 de agosto de 2026.

Esta placa **no existe**. No está diseñada, no está fabricada y no está medida. Lo que sigue es la
especificación contra la que se dibuja: **requisitos, cotas y el motivo de cada uno**.

---

> ## 🛑 Esto autoriza a DISEÑAR y a PEDIR. No autoriza a montar nada en la calle.
>
> - **En todo el proyecto no hay hoy ni una sola fila «VERIFICADO EN LA PLACA».** De ésta no puede
>   haberla: la placa ni siquiera está construida. **Todas las filas de este documento son
>   requisitos, no comprobaciones.**
> - **Nada de esto ha pasado banco.** No hay una sola tarjeta de semáforo con un ESP32 conectado
>   a `J17`.
> - **Las cifras de consumo son de ficha técnica, no medidas sobre el módulo que llegó a obra.** Se
>   marcan como tales una por una y no se disfrazan de medida.
> - **El firmware del ESP32 existe y compila, pero no ha corrido nunca sobre un módulo.** Una placa
>   perfecta con un módulo mudo encima sigue siendo un módulo mudo.
>
> **Se escribe ahora, y no cuando la placa exista, por una razón concreta:** una placa fabricada
> contra la decisión equivocada **se tira entera**. Escribir el requisito antes es lo que hace que se
> pueda mandar a fabricar una sola vez.

---

## 0. Cómo se lee este documento

### 0.1 Los tres grados de obligación — **no se mezclan**

| marca | qué significa |
|---|---|
| **REQUISITO** | La placa **no se fabrica** sin esto. Si se omite, se rehace |
| **RECOMENDACIÓN** | Mejora el resultado. Quien fabrique puede resolverlo de otro modo, **anotando cómo** |
| **PENDIENTE DE DECISIÓN** | Falta un dato, una compra o una elección. **Se dice de quién es** |

Cada requisito lleva **identificador** (`MOD-1`, `ALI-3`, …) para que otros documentos lo citen sin
copiarlo.

### 0.2 Los tres grados de prueba

| marca | qué significa |
|---|---|
| **MEDIDO** | Alguien lo abrió y lo leyó — sobre un fichero, un esquemático o el `.kicad_pcb` |
| **CUENTA** | Es aritmética a partir de datos declarados. No es una medida |
| **FICHA** | Lo declara la hoja de datos del fabricante. **No se ha medido sobre el módulo real** |
| **SIN VERIFICAR** | Nadie lo ha comprobado, ni aquí ni en ningún sitio |

**Nada de este documento está medido sobre cobre de esta placa**, porque esta placa no tiene cobre
todavía. Lo que sí está medido es la **tarjeta del semáforo** a la que se conecta.

### 0.3 Procedencia de tres apartados

Los apartados **7 (lo que esta placa NO lleva)**, **8 (puntos de prueba)** y **9 (acceso de
reflasheo)** son **texto recuperado**, no redactado de nuevo: vivían en una única guía que se
reescribió para otro destinatario y desaparecieron con ella. La procedencia exacta está en el
**Anexo A**. Se señalan porque una de las tres —el apartado 7— **no es una instrucción de montaje:
es una barrera de arquitectura**, y quien la relea tiene que saber que no se inventó aquí.

---

## 1. Qué se fabrica

**Dos placas iguales, una por poste** —Maestro y Esclavo—, igual que las dos tarjetas de semáforo son
iguales.

Cada placa sostiene:

1. un **módulo ESP32** enchufado (no soldado),
2. su **fuente propia** desde los 12 V de la caja,
3. un **módulo de reloj `DS3231`** por I²C,
4. y **tres conductores** hacia el conector `J17` de la tarjeta del semáforo.

**La tarjeta del semáforo no se modifica.** No se le añade nada, no se le sacan hilos y no se le
tocan pistas: se le enchufan tres conductores en un conector que ya existe. **Ése es el motivo por
el que esta placa existe** — la tarjeta STM32 no admite ampliación, y por eso el ESP32 y su reloj
viven fuera.

```
        12 V DE LA CAJA (es una BATERIA: no da 12,00 V fijos)
               |
               |  [PRO-1] fusible o polyfuse
               |  [PRO-2] proteccion contra polaridad invertida
               v
 +=========================================================================+
 |                    PLACA PORTADORA  (por construir)                     |
 |                                                                         |
 |  extremo de POTENCIA                          extremo de SENAL          |
 |  ...................                          ...............           |
 |                                                                         |
 |  [ALI-1..8] DC-DC CONMUTADO                                             |
 |     12 V --> 5 V, >= 1 A                                                |
 |          |                                                              |
 |          |  [ALI-7] condensador de reserva JUNTO al modulo              |
 |          v                                                              |
 |   +----------------------------+        +-------------------------+     |
 |   |  MODULO ESP32              |        |  DS3231  ZS-042         |     |
 |   |  sobre DOS TIRAS DE        |        |  pila propia            |     |
 |   |  HEMBRILLAS  [MOD-1]       |        |  pull-ups YA PUESTOS    |     |
 |   |  se enchufa, no se suelda  |        |  32K y SQW al aire      |     |
 |   |                            |        +-------------------------+     |
 |   |  VIN (5V)  <-- de la fuente             ^   ^    ^                  |
 |   |  GPIO21 SDA ----------------------------+   |    |                  |
 |   |  GPIO22 SCL --------------------------------+    |                  |
 |   |  3V3 / GND ---------------------------------------+                 |
 |   |                                                                     |
 |   |  GPIO17 TX2 --o TP1  \                                              |
 |   |  GPIO16 RX2 --o TP2   >  [TP-1..3] puntos de prueba,                |
 |   |  GND        --o TPG  /              CADA UNO CON SU MASA AL LADO    |
 |   |  salida fuente --o TPV                                              |
 |   |                                                                     |
 |   |  [FLA-1] USB del modulo ACCESIBLE con la placa montada              |
 |   +------|---------------------------------------------------------+   |
 +==========|==============================================================+
            |
            |  [ENL-1..5]  3 conductores . 9600 8N1 . los dos lados a 3,3 V
            v
      J17  p2 / p3 / p7 (o p9)      <--- NUNCA J16: J16 p1 lleva 12 V CRUDOS

   NADA DE ESTA PLACA TOCA UNA LUZ, NI LA BARRERA, NI LA CAMARA, NI EL BUZZER.
```

---

## 2. El módulo y su zócalo

### 2.1 Qué módulo es — **ya está resuelto**

**FICHA** (hoja de datos del artículo comprado):

```
Microcontrolador ...  ESP32-WROOM-32          <- el clasico
CPU ................  Tensilica Xtensa 32-bit LX6, DUAL-CORE
Bluetooth ..........  v4.2 BR/EDR + Bluetooth Low Energy (BLE)
                             ^^^^^^ Bluetooth Clasico  ->  HAY SPP
E/S ................  3,3 V
Alimentacion .......  entrada recomendada 5 V (limite 5,5 V), regulador a bordo
```

`BR/EDR` es el perfil que necesita el teléfono para conectarse. **La app conecta sin tocar el
transporte, y las E/S a 3,3 V confirman que el enlace con el STM32 va directo, sin adaptar
niveles** (apartado 5).

> **Lo que esto cierra para el dibujo:** ya no hay duda de familia. Un `ESP32-S3` o un `-C3` no
> tienen ni el mismo pinout ni los mismos números de GPIO, y habrían obligado a otra placa. **Este
> documento se dibuja para el `WROOM-32` clásico y solo para él.**

### 2.2 🔴 Lo único que bloquea el taladro: **30 o 38 pines**

> **`MOD-1` · REQUISITO.** El módulo **no se suelda**. Va sobre **dos tiras de hembrillas**, una por
> fila. La placa **no lleva la huella del `WROOM-32`**: el módulo es de **formato NodeMCU / DevKitC,
> pensado para protoboard**, así que va **enchufado y es reemplazable sin soldador**.

> **`MOD-2` · REQUISITO — se hace ANTES de fabricar.** Estas placas se venden en **30 y en 38 pines,
> con anchos distintos**. Antes de dibujar el zócalo:
>
> 1. **Contar los pines** del módulo que hay en almacén — los de las dos filas **sumados**.
> 2. **Medir con pie de rey la distancia entre las dos filas**, de **centro a centro**.
> 3. Anotar las dos cosas aquí abajo, con fecha y quién las tomó.
>
> ```text
> Fecha: ____________   Medido por: ____________________
> Pines en total (las dos filas):  [ ] 30   [ ] 38   [ ] otro: ____
> Distancia entre filas, centro a centro: ________ mm
> Longitud del modulo: ________ mm
> Serigrafia del blindaje, copiada letra por letra: ____________________
> ```
>
> **SIN VERIFICAR.** Nadie ha contado ni medido todavía. **Es una medida de un minuto con un calibre,
> y es lo único que impide dibujar el zócalo.**

> **`MOD-3` · REQUISITO.** El zócalo tiene que **sacar al menos `GPIO16`, `GPIO17`, `GPIO21`,
> `GPIO22`, `VIN`/`5V`, `3V3` y `GND`**. La versión de 30 pines sirve igual que la de 38 **siempre
> que saque esos**; se comprueba contra el módulo en la mano, no contra un diagrama de internet.

> **`MOD-4` · RECOMENDACIÓN.** Dibujar el zócalo para el ancho medido **y con las hembrillas en tiras
> independientes**, de modo que una diferencia de una décima entre dos unidades del mismo lote no
> impida enchufar el módulo. Si se dibuja para un ancho nominal de catálogo en vez de para el módulo
> medido, un lote distinto no entra.

---

## 3. La alimentación — **conmutada, no lineal**

### 3.1 Fuente propia, y por qué no puede colgar de la tarjeta

> **`ALI-1` · REQUISITO.** La placa se alimenta **de los 12 V de la caja**, con **fuente propia**.
> **`J17` p6 (3,3 V) se deja SIN CONECTAR**, y de `J17` se toman **solo las dos señales y la masa**.

**El motivo, y no es de amperios:** ese riel de 3,3 V es **el mismo que alimenta al STM32 que
gobierna el semáforo**. Si el accesorio tira de él y lo hunde, **se reinicia el controlador del
cruce**. Un reset del semáforo provocado por un periférico de diagnóstico es exactamente el reparto
de riesgo que esta arquitectura no acepta — y seguiría sin aceptarlo aunque el consumo resultara ser
la mitad del declarado.

### 3.2 🔴 Conmutada, y la cuenta es lo que lo decide

> **`ALI-2` · REQUISITO.** El convertidor es **DC-DC CONMUTADO**. **Un regulador lineal
> (`LM7805`, `LM1117`, `AMS1117` y familia) está prohibido en esta posición.**

**CUENTA**, con el pico de **~500 mA** que declara la ficha:

```
lineal     12 V -> 3,3 V :  (12 - 3,3) x 0,5  =  4,35 W  a disipar
lineal     12 V -> 5   V :  (12 - 5,0) x 0,5  =  3,50 W  a disipar

conmutado  12 V -> 5 V, rendimiento supuesto 85 % :
                    salida 5 x 0,5 = 2,50 W  ->  perdidas del orden de 0,4 W
```

**4,35 W en un armario cerrado y al sol no se disipan.** Un lineal ahí **se cuece — y no falla
limpio: falla caliente, cayendo de tensión justo cuando el módulo tira del pico**. El síntoma
aparenta ser software: el módulo «se cuelga a veces», «cuando conecta el celular».

> ⚠️ **Los 500 mA son de FICHA, no medidos sobre el módulo que llegó a obra.** Y el rendimiento del
> 85 % es un supuesto, no una medida. **Se marcan así a propósito.** Pero la decisión **no depende de
> esas cifras**: aunque el pico resultara ser 300 mA, el lineal sigue fuera, porque el motivo es el
> reparto de riesgo del apartado 3.1 y no la aritmética.

### 3.3 La ruta y el margen

> **`ALI-3` · REQUISITO.** La ruta es **`12 V → DC-DC conmutado → 5 V → pata `VIN`/`5V` del
> módulo`**. El regulador de a bordo del módulo hace el `5 → 3,3` (**CUENTA**: disipa
> `(5 − 3,3) × 0,5 = 0,85 W`, que es el reparto que la propia placa DevKitC ya acepta).
>
> **Alternativa válida, y es EXCLUYENTE:** salida a **3,3 V** entrando por la pata `3V3` — que es la
> **salida** del regulador de a bordo. Se hace **solo** si algún día se monta un `WROOM` pelado sin
> regulador propio, y entonces `VIN`/`5V` **queda sin usar**. 🔴 **Nunca las dos a la vez.**

> **`ALI-4` · REQUISITO.** Corriente de salida **≥ 1 A** — al menos el **doble** del pico declarado.

**El porqué del doble, que no es prudencia genérica:** el ESP32 **no consume su media**. El arranque
de la radio es un **escalón** de decenas de milisegundos. Una fuente dimensionada al consumo medio
**cae de tensión justo en ese escalón**, el módulo se reinicia por *brown-out*, y el fallo se
diagnostica durante días como si fuera del programa.

> **`ALI-5` · REQUISITO.** **Rango de entrada holgado por arriba y por abajo.** El riel de 12 V de la
> caja **es una batería, no un 12,00 V fijo**: sube al cargar y baja al descargarse, y **ninguno de
> los dos extremos está medido en esta instalación** (**SIN VERIFICAR** — es la medida `M6`, apartado
> 11). Los módulos de mostrador declaran típicamente de 6–8 V hasta 24–28 V, y eso sobra.

> **`ALI-6` · RECOMENDACIÓN.** Que la fuente sea de **salida fija** o quede **ajustada y fijada**
> (laca, tornillo asegurado). Un potenciómetro de ajuste accesible dentro de un armario es un
> potenciómetro que alguien va a girar.

> **`ALI-7` · REQUISITO.** **Condensador de reserva junto al módulo** — un electrolítico de algunos
> cientos de µF más un cerámico de 100 nF, **físicamente al lado de las hembrillas**, no al lado del
> regulador.
>
> **El porqué:** el hilo desde la fuente tiene inductancia. **El pico no lo sirve el regulador: lo
> sirve el condensador que está al lado.** Un condensador puesto junto al DC-DC y no junto al módulo
> cumple la lista de materiales y no cumple la función. **PENDIENTE DE DECISIÓN:** el valor exacto es
> parte del diseño que falta.

### 3.4 ⚠️ Doble alimentación — el USB y la fuente sobre el mismo riel

> **`ALI-8` · REQUISITO.** El módulo lleva un **`CP2102`** y su conector USB a bordo. Si la placa lo
> alimenta por `VIN` **y alguien enchufa el USB para reflashear**, hay **dos fuentes sobre el mismo
> riel**. El diseño tiene que **elegir el procedimiento** y **serigrafiarlo en la placa**:
>
> - **cortar la entrada de 12 V antes de enchufar el USB**, o
> - **un jumper de aislamiento** que abra el camino de la fuente.
>
> 🔴 **Va serigrafiado en la placa, no en un correo.** Quien vaya a reflashear estará subido a un
> poste, con un portátil en una mano, y no va a tener el correo delante.

---

## 4. Protección de la entrada de 12 V

> **`PRO-1` · REQUISITO.** **Fusible o polyfuse** en la entrada de 12 V.

> **`PRO-2` · REQUISITO.** **Protección contra inversión de polaridad.**

**El motivo:** el 12 V se toma de la misma caja, a mano, con un destornillador y a veces de noche.
**Invertirlo no es una hipótesis remota.** Sin protección, un cambio de dos cables se lleva por
delante la fuente, el módulo y el reloj de una vez.

---

## 5. El enlace a `J17` — **ya está medido; no se rediseña**

> **`ENL-1` · REQUISITO.** Las conexiones son **exactamente** éstas:

```
GPIO17 (TX2 del ESP32)  -->  J17 p2  =  PB7  =  RX del micro    (U1 pin 43)
GPIO16 (RX2 del ESP32)  <--  J17 p3  =  PB6  =  TX del micro    (U1 pin 42)
GND                     ---  J17 p7  o  p9

9600 8N1
```

**MEDIDO** en el firmware de las dos puntas (`Maestro/src/bluetooth.cpp:28`,
`Esclavo/src/bluetooth.cpp:26`, `SerialBT.begin(9600)` en `:70` y `:78`).

> ⚠️ **El cruce TX/RX no da error: da silencio.** El micro **recibe** por `PB7` y **transmite** por
> `PB6`. Un mazo cruzado se comporta exactamente igual que un módulo muerto.

> **`ENL-2` · REQUISITO.** Son **tres** conexiones eléctricas y ninguna más. **`J17` p6 y p8 (3,3 V)
> quedan sin conectar** (`ALI-1`). *(Otros documentos hablan de «cuatro hilos»; las conexiones
> especificadas y medidas son estas tres.)*

> **`ENL-3` · REQUISITO — masa común, y no es opcional.** La masa va **como conductor dedicado, en el
> mismo mazo que los datos**, nunca tomada de la caja por otro camino.
>
> **El porqué:** dos masas que se encuentran por caminos distintos es como aparece una diferencia de
> potencial entre ellas, y **esa diferencia entra entera por `PB6`/`PB7`, que son patas del micro que
> gobierna el semáforo**. Sin masa común los dos hilos de datos no tienen referencia y no comunica
> nada aunque todo lo demás esté bien. **Masa común sí; alimentación compartida no** — son dos cosas
> distintas, y confundirlas es lo que hace que el semáforo se reinicie solo.

> **`ENL-4` · REQUISITO — NO se pone adaptador de niveles**, ni divisor, ni *level shifter*. **Las dos
> puntas son de 3,3 V**: el STM32 y el ESP32 (**FICHA**, apartado 2.1). Un adaptador puesto «por si
> acaso» es una pieza más que puede estar mal, y **un divisor mal calculado en la línea de `RX` del
> micro es peor que no tener nada**.

> **`ENL-5` · REQUISITO.** El mazo se mantiene **corto** y sale por un **conector propio de la placa
> portadora** — no por cables soldados directamente al cobre.

### 5.1 🔴 `J16` no es `J17`, y confundirlos quema la placa entera

**MEDIDO** sobre el `.kicad_pcb` de la tarjeta: **`J16` y `J17` comparten footprint**
—`Molex_KK-254_AE-6410-16A_1x16`, 16 pads los dos— y **a la vista son idénticos**. **`J16` p1 lleva
12 V crudos.**

> **Se distinguen MIDIENDO, no mirando.** Con el multímetro, la posición 1 del conector contra masa:
> **si da ≈ 12 V, ése es `J16`** y ahí no va nada de esta placa; **si no, es `J17`**.
>
> **Enchufar la placa portadora en `J16` la quema en el acto**, y con ella el módulo y el reloj.

---

## 6. El reloj `DS3231`

> **`RTC-1` · REQUISITO.** Módulo **`DS3231` `ZS-042`**, I²C al ESP32: **`GPIO21` = SDA**,
> **`GPIO22` = SCL**, más `3V3` y `GND`. **Cuatro conexiones cortas dentro de la propia placa
> portadora**: no salen de ella y **no llegan a la tarjeta del semáforo**.

> **`RTC-2` · REQUISITO.** **El módulo ya trae sus pull-ups: NO se añaden.** Un segundo juego en
> paralelo baja la resistencia efectiva y carga el bus **sin que nadie lo note hasta que el bus falla
> de forma intermitente**, que es el fallo más caro de diagnosticar. `32K` y `SQW` quedan **al aire**.

> **`RTC-3` · REQUISITO.** La huella y las cuatro conexiones **se dibujan en las DOS placas**, aunque
> hoy solo haga falta un reloj. **Añadirlas después obliga a rehacer la placa**; dejarlas puestas y
> sin poblar no cuesta nada.

> **`RTC-4` · REQUISITO de recepción, antes de dar corriente.** El `ZS-042` lleva **circuito de
> carga** pensado para una **`LIR2032` recargable**, y **se vende muy a menudo con una `CR2032` NO
> recargable ya puesta encima**. Con una `CR2032` dentro, ese circuito intenta cargar una pila que no
> admite carga: **se calienta, se hincha y puede reventar**, con el módulo ya dentro de una caja en
> un poste.
>
> ```text
>  1. Mirar la pila que trae puesta.  Rotulo CR2032  -> NO recargable
>                                     Rotulo LIR2032 -> recargable, correcto
>  2. Si es CR2032: desoldar D1 o R1 del modulo ANTES de energizarlo
>                   (cualquiera de los dos corta el camino de carga)
>  3. Si es LIR2032: no se toca nada
>  4. Solo entonces se le da corriente
> ```

**El consumo del reloj es de microamperios: no entra en el dimensionado de la fuente** (`ALI-4`).

> ⚠️ **Lo que no cambia por tener sitio en la placa:** un reloj montado hoy **se queda mudo**, y eso
> es lo esperado, no una avería. **No se reporta como fallo, no se devuelve al mostrador y no se
> busca el defecto en la soldadura.**

---

## 7. 🔴 Lo que esta placa NO lleva — **es la barrera, no una lista de omisiones**

> **Este apartado es del mismo rango que la barrera de salidas del firmware.** No describe lo que se
> quedó fuera por falta de sitio: describe **lo que esta placa tiene prohibido llevar**, y el motivo
> por el que un semáforo sigue siendo seguro con ella enchufada.

> **`NO-1` · REQUISITO — nada que escriba sobre las luces.** Ni relés de lámpara, ni salidas de
> potencia, ni un conductor hacia `J3`–`J9` o `J11`, ni hacia **`J15`** (la barrera), ni hacia
> **`J14`** (la cámara), ni hacia **`J13`** (el buzzer). **Ni un conductor a `J16`** —ni para señal
> ni para tomar de ahí los 12 V—.

**El porqué, que no cambia porque haya una placa nueva:** **solo `semaforo.cpp` escribe pines de
luz**, y todo pasa por su `escribirPines()`. Ésa es la barrera de salidas del firmware, y **una placa
accesoria no la reabre**. El ESP32 **no manda sobre el semáforo: PIDE**, por el puerto serie, y el
STM32 **acepta o rechaza** con su `$ACK` o su `$ERR`.

> **`NO-2` · REQUISITO — nada que reinicie al STM32.** Ni una línea de *reset*, ni un watchdog
> externo sobre la tarjeta. **Los dos STM32 ya tienen el suyo, a 4 s**, y **un accesorio con
> capacidad de reiniciar al controlador del semáforo es exactamente el reparto que esta arquitectura
> evita**.

> **`NO-3` · REQUISITO — sin batería propia para el ESP32.** Si se va la energía de la caja, el
> semáforo se apaga: **mantener vivo el módulo de diagnóstico mientras el cruce está muerto no
> resuelve nada** y añade una fuente más dentro del armario. **Lo único que sobrevive al corte es el
> reloj, con su pila** (`RTC-4`), que es para lo que está.

> **`NO-4` · REQUISITO — la placa no toma señal de ningún otro conector de la tarjeta.** Su única
> conexión con el semáforo son las tres del apartado 5.

> 🔴 **Por qué esta lista tiene que estar escrita, y por qué se revisa antes de mandar a fabricar:**
> **lo que una placa nueva NO lleva no deja rastro en el cobre.** Un hueco no grita. Dentro de un año
> nadie sabrá si faltó o si se decidió — y la primera persona que necesite «una salidita más» la
> añadirá creyendo que nadie lo había pensado.

---

## 8. Puntos de prueba — **requisito de diseño, no comodidad**

> **La placa se monta dentro de un armario, en un poste.** Las dos cosas que habrá que hacerle con el
> tiempo —**medirle el TX** y **volver a programarlo**— hay que diseñarlas **ahora**.

**Sin puntos de prueba, medir el TX del ESP32 en reposo obliga a pinchar en el propio conector con el
equipo montado**: punta fina sobre un pad de 2,54 mm, junto a la masa y junto a otra señal, con la
placa energizada. **Eso no es una medida: es un cortocircuito esperando a que a alguien le tiemble
la mano.**

> **`TP-1` · REQUISITO.** Cuatro puntos de prueba, y **el tercero no es un extra: es lo que hace que
> los otros sirvan.**

| punto | qué es | para qué |
|---|---|---|
| **`TP1`** | `GPIO17` · **TX2** del ESP32 | Es la medida **`M5`**: en reposo debe dar **3,3 V** (línea serie en reposo alta). 🔴 **Si diera 5 V, el módulo no es el que se cree que es — y se para antes de conectar nada** |
| **`TP2`** | `GPIO16` · **RX2** | Ver que el conductor que viene del micro está donde se cree |
| **`TPG`** | **masa, al lado de los otros dos** | **Una medida de tensión necesita dos puntas.** Un punto de prueba **sin su masa al lado no es un punto de prueba**: obliga a buscar masa en otro sitio con la placa energizada, que es **como se resbala una punta** |
| **`TPV`** | **salida de la fuente, y su masa al lado** | Para **`M6`**: medir la salida **en vacío y con el módulo transmitiendo**, **sin desmontar nada** |

> **`TP-2` · REQUISITO.** **Pads accesibles con punta de multímetro con la placa montada** — no vías
> tapadas por el módulo, no pads debajo de un conector, no puntos que obliguen a desenchufar el
> módulo para llegar a ellos.

> **`TP-3` · REQUISITO.** **Rotulados en la serigrafía** — `TP1`, `TP2`, `TPG`, `TPV`, junto al pad.
> **Si no está rotulado, dentro de seis meses nadie sabrá cuál es cuál**, y la placa vuelve a
> medirse siguiendo pistas con una lupa.

---

## 9. Acceso de reflasheo sin desoldar — **dos casos**

> **`FLA-1` · REQUISITO — caso A, el que aplica al módulo que hay: es una placa NodeMCU / DevKitC y
> ya trae su conector USB.** El requisito es **mecánico**: que ese conector quede **accesible con la
> placa montada en la caja** — no tapado por el conector de 12 V, ni por el mazo hacia `J17`, ni
> contra una pared del armario. **Se comprueba al dibujar la placa, no después.**

> **`FLA-2` · REQUISITO — caso B, si alguna vez se monta un `WROOM` pelado.** No hay USB, y hay que
> sacar a una **tira de prueba**: **`EN`, `IO0`, `TX0` (`GPIO1`), `RX0` (`GPIO3`), `3V3` y `GND`**.
> 🔴 **Y no se reutilizan `GPIO16`/`GPIO17` para esto**: están ocupados por el enlace con el semáforo.

> **`FLA-3` · REQUISITO — la precaución sobre `EN` e `IO0`.** Son los dos pines que meten al módulo en
> **modo de descarga**. **Una tira accesible con esos dos pines es una tira que un destornillador
> puede pisar**, y entonces el módulo se queda **en modo de programación, mudo, sin que nada lo
> indique**. Van donde no se pisen, y **si llevan pulsador, que no sobresalga**.

> **`FLA-4` · REQUISITO — el procedimiento de las dos fuentes, porque es donde se rompe.** **Antes de
> enchufar el USB se corta la entrada de 12 V** (o se abre el jumper de aislamiento de `ALI-8`).
> **Serigrafiado en la placa.**

**El módulo se enchufa y se puede sacar** (`MOD-1`), pero eso **no sustituye a este apartado**:
sacarlo para reprogramarlo obliga a volver a enchufarlo bien —y las hembrillas se desgastan—, y en un
poste, de noche, es exactamente cuando se dobla un pin.

---

## 10. Regla de trazado — **separar los 12 V de las señales**

> **`TRZ-1` · REQUISITO.** **La entrada de 12 V, el fusible, la protección de polaridad y el
> regulador van en UN EXTREMO de la placa. Las señales de 3,3 V, el zócalo del módulo, el reloj y el
> conector hacia `J17` van en el OTRO.**

**De dónde sale esta regla, que no es de manual sino de una medida:** en la tarjeta del semáforo —que
**ya está fabricada**— la separación real entre la red de 12 V y una señal de botón es de
**1,359 mm** en su punto peor (**MEDIDO** cobre a cobre sobre el `.kicad_pcb`: vía contra pista
`F.Cu`; los otros tres casos dan 1,405 / 1,408 / 4,269 mm). **Eso es cobre de diseño**, sin la
tolerancia de fábrica, sin la suciedad y sin la humedad de un armario en la calle.

**Ahí ya no se puede cambiar. Aquí sí.** En una placa que se dibuja de cero esa separación **se
elige, y es gratis**. Regalarla es la diferencia entre una placa nueva y una placa nueva con el mismo
problema.

> **`TRZ-2` · RECOMENDACIÓN.** Que la separación mínima entre la red de 12 V y cualquier red de
> 3,3 V **no baje de los 2 mm** en ningún punto —pads, pistas y vías incluidos, respetando capas—, y
> que el fabricante lo verifique con la comprobación de reglas de diseño antes de enviar. Si el
> tamaño de la placa no lo permite, **se agranda la placa**: es lo más barato de todo lo que hay en
> este documento.

---

## 11. La medida `M6` — y por qué hoy no se puede hacer

`M6` es la medida que cierra el dimensionado de la fuente. **Hoy no se puede rellenar: la placa no
existe.**

| qué se mide | dónde | esperado |
|---|---|---|
| Tensión real del riel de 12 V de la caja, **cargada y descargada** | borne de entrada | dentro del rango de `ALI-5` |
| Salida de la fuente **en vacío** | `TPV` contra su masa | la nominal elegida (5 V) |
| Salida de la fuente **con el módulo transmitiendo** | `TPV` contra su masa | **sin caída apreciable** — es lo que valida `ALI-4` y `ALI-7` |
| Temperatura del regulador **a los 30 minutos** | al tacto o con sonda | tibio, no quemando |
| `TP1` contra `TPG` **en reposo** | punto de prueba | **3,3 V** · 🔴 **si da 5 V, se para** |

> ⚠️ **La hoja de `M6` tiene que admitir tres respuestas, no dos.** Mientras la placa no exista, la
> respuesta correcta es **«NO SE PUDO MEDIR»** con el motivo *«placa no construida»*. **Una hoja que
> solo admite bien/mal obliga a inventar.**

---

## 12. Lo que falta COMPRAR o DECIDIR

### 12.1 Compras

| # | qué | cant. | estado |
|---|---|---|---|
| **`A5`** | **Fuente DC-DC conmutada 12 V → 5 V, ≥ 1 A**, con sus borneras y su cable | **2** (1 por placa) | 🛒 **NO PEDIDA.** Requisitos en el apartado 3. 🔴 **La referencia NO está elegida** |
| **`A6`** | **Módulo RTC `DS3231` `ZS-042`** con su pila | **1** *(ver 12.2)* | 🛒 **NO COMPRADO** |
| — | **La placa portadora en sí**: circuito impreso o placa de prototipo, hembrillas, conectores, fusible o polyfuse, protección de polaridad, condensadores de reserva y cableado | **2** | 🛒 **No es todavía una línea de compras.** No hay presupuesto ni proveedor |

> 🔴 **Ninguna referencia concreta de este documento es una elección.** Los `LM2596` y `MP1584` que
> aparecen en la lista de compras son **prueba de que la pieza existe**, no una decisión tomada.
> **Aquí están los requisitos —conmutado, ≥ 1 A, rango de entrada holgado, salida de 5 V—; la
> referencia la elige quien compra, con el apartado 3 delante.**

### 12.2 Decisiones pendientes, con dueño

| # | qué falta | quién lo decide | qué bloquea |
|---|---|---|---|
| **`PP-D1`** | **30 o 38 pines, y el ancho entre filas** del módulo que hay en almacén (`MOD-2`) | quien tenga el módulo y un pie de rey | 🔴 **El zócalo, y por tanto el taladro.** Es lo único que impide dibujar. **Un minuto de calibre** |
| **`PP-D2`** | **Referencia concreta de la fuente** — corriente, si aislada o no, cómo se fija en la caja | quien compra | El pedido de `A5` y la huella mecánica de la fuente en la placa |
| **`PP-D3`** | **Valor del condensador de reserva** (`ALI-7`) | quien diseña | Nada más; se cierra al dibujar |
| **`PP-D4`** | **Qué mecanismo de doble alimentación** se adopta: corte de la entrada de 12 V o jumper de aislamiento (`ALI-8`) | quien diseña | El cobre **y** el texto de la serigrafía |
| **`PP-D5`** | **Cuántos relojes**: 1 o 2. Hoy solo el Maestro necesita reloj propio —el Esclavo toma la hora del Maestro por radio—, pero eso depende de en qué tarjeta está muerto el cristal `Y2`, **que sigue sin diagnosticarse** | el responsable, con el resultado del banco | La **compra** de `A6`, **no** el dibujo: la huella se dibuja en las dos placas de todos modos (`RTC-3`) |
| **`PP-D6`** | **Quién diseña la placa y quién la fabrica** | el responsable | Todo lo demás |

---

## 13. Lo que este documento NO mide, y nadie debe dar por medido

*Se escribe explícito porque una especificación que no marca sus bordes se lee como un permiso.*

| | |
|---|---|
| **Toda esta placa** | **No existe.** Ni una fila «VERIFICADO». No hay cobre, no hay montaje y no hay medida |
| **El pico de 500 mA del ESP32** | **FICHA**, no medido sobre el módulo que llegó a obra |
| **El rendimiento del 85 % del conmutador** | **Supuesto** de la cuenta de 3.2. Tampoco medido |
| **La tensión real del riel de 12 V** | **SIN VERIFICAR.** Es `M6`, y hoy no se puede hacer |
| **Que el enlace por `J17` funcione** | **SIN VERIFICAR.** Nunca se ha conectado un ESP32 a `J17` en ninguna tarjeta |
| **El cobre de la tarjeta del semáforo** | Todo lo que se sabe de ella sale del esquemático y del `.kicad_pcb`. **Ni una fila «VERIFICADO EN LA PLACA»** — un fichero dice lo que alguien dibujó; una placa dice lo que se fabricó |
| **Los 30 o 38 pines del módulo** | **SIN VERIFICAR.** Nadie los ha contado (`PP-D1`) |
| **El firmware del ESP32 sobre hardware** | Compila, y no ha corrido nunca sobre un módulo. **No hay driver de `DS3231` funcionando en ninguna punta** |

---

## Anexo A · De dónde sale cada cosa

| apartado | procedencia |
|---|---|
| **2.1** el módulo, `LX6` dual-core, `BR/EDR`, 5 V / 5,5 V, E/S 3,3 V | **FICHA** del artículo comprado, aportada por el responsable el 31/08 |
| **2.2** 30 o 38 pines, hembrillas, formato protoboard | `roadmap.md` — *«lo que sigue abierto»* de la ficha del módulo |
| **3** alimentación, los 500 mA, la regla de no compartir riel | `05_Funcional/15_Lista_de_Compras_Hardware.md` línea `A5` y `10_Manual_Modulo_Bluetooth_Telemetria.md` §1 |
| **5** el enlace pin a pin, `9600 8N1`, `PB7`/`PB6`, `U1` 42/43 | `05_Funcional/18_Especificacion_Firmware_ESP32.md` §2.1 y §3.1, **MEDIDO** sobre `bluetooth.cpp` de las dos puntas |
| **5.1** `J16`/`J17` mismo footprint, `J16` p1 con 12 V | `05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md` §A `M1`, **MEDIDO** sobre el `.kicad_pcb` |
| **6** el `DS3231`, sus pull-ups y el aviso de la pila | `05_Funcional/15_Lista_de_Compras_Hardware.md` línea `A6` |
| **8** `TP1` como medida `M5` (3,3 V en reposo, 5 V ⇒ parar) | `05_Funcional/17_...` §A `M5` |
| **10** la separación de **1,359 mm** | `03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md`, **MEDIDO** cobre a cobre sobre el `.kicad_pcb` |
| **7, 8 y 9** *(los tres apartados recuperados)* | **Texto original recuperado** de la versión anterior de `05_Funcional/Guia_Cableado_y_Pruebas_Banco.html` (commit `f10f4d4`), retirado al reescribir la guía para el técnico de campo (commit `fa66710`) |

> **Sobre los tres apartados recuperados, y por qué importa decirlo:** se dieron por perdidos porque
> una búsqueda sobre los ficheros del árbol no los encontró — **y era cierto que no estaban en
> ninguno**. Estaban en el **historial**, que la búsqueda no miraba. **Aquí no se han reconstruido de
> memoria: se han recuperado literales y se les ha actualizado una premisa que caducó** —el texto
> original suponía el módulo **soldado**, y hoy va **enchufado sobre hembrillas** (`MOD-1`)—. El
> requisito no cambia; **el motivo, sí**, y va escrito en el apartado 8.

---

## Anexo B · Referencias que faltan y que este documento NO ha hecho

*Ninguno de estos ficheros se ha tocado. Es la lista de trabajo, no el trabajo.*

| fichero | qué le falta |
|---|---|
| `05_Funcional/15_Lista_de_Compras_Hardware.md` | La línea `A5` dice de sí misma que *«qué módulo concreto se compra no está en ningún manual»*. **Ya lo está**: apuntar de `A5` y de `A6` a este documento |
| `05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md` | Su §A lista cinco medidas de multímetro. **`M6` vive aquí** y no está en esa lista |
| `05_Funcional/18_Especificacion_Firmware_ESP32.md` | Su §2 describe el enlace físico y no dice que el módulo va sobre una placa que hay que construir |
| `05_Funcional/Guia_Cableado_y_Pruebas_Banco.html` | Su paso del pie de rey mide para esta placa; el técnico anota el dato que cierra `PP-D1` |
| `05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.md` | Su diagrama de conexión dibuja un módulo colgado del riel de la tarjeta, **que es justo lo que `ALI-1` prohíbe** |
| `03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md` | La separación de 1,359 mm se cita aquí como origen de `TRZ-1` |
| `ESTADO.md` y `README.md` | **No hay hoy ningún camino que lleve a este documento** |
