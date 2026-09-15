# SPEC 5 — COBRE Y CONECTORES

**Para quien tiene un destornillador en la mano, y para quien compra — UN SOLO LECTOR.** Que hay en
cada pin, que es peligroso, y que declara este proyecto **sin haberlo medido nunca**. 🔴 **Que HACEN
la camara y la barrera —el veto, el retardo de 3 s, el aviso, los dos interruptores— ya NO vive aqui:
es `SPEC_8`.** **Fuera** ademas: el ciclo (SPEC 1), la radio (SPEC 2), la hora (SPEC 3), la app (SPEC 4).

> **EL CORTE CON `SPEC_8`, Y POR QUE ES ESE — se escribe porque partir un documento deja huecos entre
> las dos mitades, y un hueco que nadie nombra no lo cubre nadie.** Aqui vive lo que decide **EL
> COBRE**: lo que hay en un borne y lo que pasa **sin firmware dentro**. Alli vive lo que decide **EL
> FIRMWARE**. Por eso **la pluma sin energia esta AQUI** (§4: cae el pin, cae el MOSFET, baja el
> brazo) y **la pluma que sube con el ambar intermitente esta en `SPEC_8` §5** — esa la decide una
> linea de `escribirPines()`, no el cobre.

**De donde sale cada linea.** Las MEDIDAS de cobre son de `05_Funcional/17_Arquitectura_28-08_y_
Decisiones_Abiertas.md`, **copiadas literales con su fecha** — esa spec **gana a esta** en todo lo
medido. Lo decidido lo fija `DECISIONES.md`. El firmware se cita **por SIMBOLO**, nunca por linea.
**Escala:** `MEDIDO EN COBRE` = multimetro sobre la placa, con su numero y fecha · `MEDIDO` = leido
del fichero · `SIN VERIFICAR` = **nadie lo ha comprobado, ni aqui ni en ningun sitio**.

> 🔴 **Y la linea que manda sobre todo lo demas: no hay ni una fila «VERIFICADO EN LA PLACA» en todo
> el mapeo de la tarjeta** (`MAPEO_TARJETA_KICAD.md` §0 y §9) **salvo las que dejo el banco del
> 03-04/09**: `J16` p5/p8/p10/p12, `J17` p2/p3, `J14`, `J15` y las masas del modulo. Todo lo demas
> es un **dibujo**, no una placa.

# 🛑 PAGINA 1 — LOS AVISOS QUE HIEREN A UNA PERSONA: **TRES DE COBRE, Y EL CUARTO SE MUDO**

## 1. `J16` p1 lleva 12 V CRUDOS, y taparlo es OBLIGATORIO en cada equipo

`J16` es un conector de **senal**, directo a la pata del `U1`. **No hay opto, no hay resistencia en
serie y no hay clamp** entre `p1` y el resto del conector. `MAPEO_TARJETA_KICAD.md` §7: **`J16` es el
unico conector de senal de toda la tarjeta que trae 12 V**. **Un contacto de `p1` a `p10` o `p12`
mete 12 V en una pata que espera 3,3.**

- **`D-4`: se tapa en cada equipo que se monte** — no es cautela de banco (`N-120`). **El metodo que
  se documenta es RETIRAR EL PIN del cuerpo del conector volante** (paso 4 del banco), no la funda
  termorretractil: no se puede deshacer por accidente.
- **El firmware NO PUEDE protegerse de esto.** Esa posicion no llega a ningun pin del micro: ninguna
  linea de codigo se entera de que la puentearon contra una de senal. **La barrera es fisica o no hay
  barrera** (cabecera de `pines.h`, `D-4`).
- **MEDIDO EN COBRE 31/08 — el colchon no existe:** la separacion real red de 12 V / senal es
  **1,359 mm** en `p12` (el peor), **1,405 mm** en `p5`, **1,408 mm** en `p8`, 4,269 mm en `p10`. El
  documento viejo publicaba 10-28 mm: contaba **posiciones**. **Un pad no es una red.**

## 2. `J14` es una ENTRADA del micro. La salida de talanquera es `J15`

**`J14` p1 = `PB0`, entrada de 3,3 V, sin opto y sin diodo** (`R64` 10 kOhm + `C25` 100 nF a la
bornera: antirrebote RC de 1 ms **en la placa**). `J14` p2 son **3,3 V**, el nudo de `J16` p4/p7/p9/p11.

> 🔴 **Un rele cableado a `J14` se desconecta ANTES de energizar nada.** La salida de la talanquera
> es **`J15`** — `PB2` -> `R70` 220 / `R69` 10K -> opto `TLP127` (`U15`) -> MOSFET `IRLZ44N` (`Q10`)
> -> bornera. `J15` p1 = 12 V, `J15` p2 = **drenador de `Q10`**.

- **`D-27` (2), 11/09: `J14` queda LIBRE, SIN CABLEAR** — el fin de carrera **no se instala** aqui.
- ⚠️ **Mientras el firmware lea `PB0` como `CAM_DEMANDA_PIN`, en `J14` no se conecta NADA** — sigue
  leyendolo en las dos puntas (`SPEC_8` §2). Vacio, `R64` lo deja en **0 V** (`J14` **MEDIDO EN
  COBRE**, pasos 17-18 del banco 03-04/09).
- ⚠️ **`J15` p2 NO es masa**, aunque un esquema de guia lo rotulara «GND (Q10)»: con `Q10` abierto
  esta a **~12 V** (`D-25`).

## 3. `J16` p5 y p8 estan VACIOS: el firmware los sigue LEYENDO, ya sin nada que ejecute — y NO se cablean

> 🔴 **Regla de montaje, no cautela: NADA se cablea en `J16` p5 ni p8, y no se puentean «para
> probar».** La regla **sigue en pie**; lo que cambio el 14/09 es **el motivo**, y son tres:
>
> 1. **El firmware que hay DENTRO del poste puede ser anterior a la retirada.** Un equipo sin carga
>    verificada de un firmware posterior al 14/09 **sigue componiendo secuencias con un puente**:
>    `A.A.A` entra al Modo Automatico **sin ninguna guarda** —abre paso—, `B.B.B` pone ambar y
>    `A.B.A.B` mete al poste en Degradado. **Que firmware lleva cada equipo lo dice `ESTADO.md`, no
>    esta spec**, y el instalador no lo puede ver desde la bornera (`CLAUDE.md` §3: se exige la CARGA,
>    no el commit).
> 2. **El pin sigue LEIDO en el firmware de hoy**, aunque ya no ejecute nada (tabla de abajo). Un
>    llamador nuevo de `botonArriba()` o `botonAbajo()` lo vuelve a convertir en una entrada que
>    actua **sin tocar el cobre**. El pin no esta cerrado hasta que salga la lectura, y eso **no esta
>    construido**: es `SPEC_1` §12.2.
> 3. **Es una entrada directa a una pata del micro, sin nada en medio** (§5), **a poco mas de un
>    milimetro de la red de 12 V** (§1).

**Lo que el firmware del arbol HACE con esos dos bornes** — MEDIDO leyendo el fuente de las dos puntas el
15/09/2026; **no ejercido en ninguna tarjeta**:

| | simbolo | que produce un flanco en p5/p8 |
|---|---|---|
| la lectura | `botones_setup()` configura `BOTON1`/`BOTON2` (`PB9`/`PB13`) como entrada pelada; `botones_actualizar()` los antirrebota y deja un flanco por vuelta | nada por si sola |
| ~~las secuencias del mando~~ | ~~`mando_registrarPulso()` desde `botones_actualizar()`~~ — la llamada se corto el 14/09 (`ccca294`) y el modulo del mando salio entero de las dos puntas ese mismo dia (`f57a401`) | **ninguna secuencia**: no queda reconocedor |
| lo que queda consumiendo el flanco | `botonArriba()` / `botonAbajo()`. **Maestro:** los llaman `menu_loop()` y `modo_hora_loop()`. **Esclavo:** ningun llamador | **Maestro en menu:** mueve un cursor interno, y la confirmacion que lo convertiria en un modo (`botonAceptar()`) devuelve `false` siempre. **Modo Hora:** inalcanzable (`SPEC_1` §12.7). **Ni luz, ni pluma, ni trama, ni campo de la app** |

Ya no existen en el fuente `mando_registrarPulso()`, `mando_ambarLocal()` ni `semaforo_senalEnCurso()`.

> ⬇️ ~~De `mando_ambarLocal()` cuelgan tres vetos en `Esclavo/src/main.cpp`; retirar su armador los dejaria ABIERTOS, y por eso el codigo no se toca~~
> → **MEDIDO el 15/09 sobre los dos commits: ningun veto quedo abierto.** La bandera **solo** se armaba
> desde una secuencia del mando, asi que al cortar los pulsos valia `false` para siempre. Las tres guardas
> del bucle del Esclavo eran «bandera del mando **y** ambar de la app» y quedan en **solo el ambar de la
> app** —la misma respuesta, porque el primer termino ya era siempre verdadero—; la guarda de la
> reanudacion diferida del Degradado y la rama de `CANCELAR_AMBAR` que contestaba «queda el del mando»
> **eran inalcanzables** y salieron. **El veto que sigue vetando es el de la app** (`SPEC_2` §2.3).

## 4. 🔴 LA CAMARA FRENA LA BARRERA — **este aviso se mudo ENTERO a `SPEC_8` §1**

**Lo unico que se afirma de eso desde aqui es cobre: `J16` p10 y p12 son las dos camaras (§2.1) y
`J15` es la pluma (§4). El veto, el retardo de 3 s y el aviso son `SPEC_8`, y no se resumen aqui.**

# 1. El reparto, en una frase

**El STM32 sigue siendo el controlador del semaforo. El ESP32 es un modulo de expansion colgado de un
puerto serie (`J17`, `PB6`/`PB7`, 9600 8N1): aporta reloj `DS3231` y Bluetooth, y NO manda sobre las
luces.** Lleva **fuente propia desde 12 V** y no cuelga de los 3,3 V de `J17` p6/p8: ese riel es el
mismo que alimenta al STM32 del semaforo, y **el accesorio no puede tumbar al que manda** (`17_` §1.5).

# 2. La tabla de conectores — vivo, muerto o libre

**La columna «estado» esta MEDIDA hoy sobre `{Maestro,Esclavo}/include/pines.h` y `src/*.cpp`.**
⚠️ **No existen `camara.cpp` ni `talanquera.cpp`:** la camara vive en `botones.cpp`, la pluma dentro
de `semaforo.cpp`.

| bornera | pin del micro | que es | estado |
|---|---|---|---|
| `J3` `J4` `J5` | `PA0` `PA1` `PA2` | Rojo 1 / Amarillo 1 / Verde 1 | 🟢 **VIVO** — `escribirPines()` |
| `J6` `J7` `J8` | `PA3` `PA4` `PA5` | Rojo 2 / Amarillo 2 / Verde 2 | 🟢 **VIVO** — `escribirPines()` |
| `J11` · `J9` | `PA6` · `PA7` | Rojo peaton · Verde peaton | 🔴 **DECLARADOS Y MUERTOS** |
| `J13` | `PB1` | Buzzer | 🔴 **DECLARADO Y MUERTO** |
| `J15` | `PB2` | **Talanquera** — opto `U15`, MOSFET `Q10` | 🟢 **VIVO** — dentro de `escribirPines()` |
| `J12` | `PB10` TX · `PB11` RX · `PB12` DE/~RE | Radio LoRa (`USART3`) | 🟢 vivo — SPEC 2 |
| `J10` | `PA9` `PA10` · `PA8` DE/~RE | RS485 «IN» / telemetria | 🟡 vacio hoy |
| `J14` | `PB0` | **ENTRADA** — `CAM_DEMANDA_PIN` | 🟡 **LIBRE, sin cablear** (`D-27`) — pero **leido** |
| `J16` | ver abajo | ~~mando `A`/`B`~~ p5/p8 vacios + **las dos camaras** | mitad vivo, mitad libre (y leido, §3) |
| `J17` | `PB3`-`PB7` | LCD retirada -> **ESP32** (p2 = `PB7` RX, p3 = `PB6` TX) | 🟢 vivo |
| `J1` · `J2` | — | alimentacion · **SWD** | — |

> 🔴 **TRES SALIDAS NO EXISTEN MAS QUE EN LA TABLA (MEDIDO 02/09, reconfirmado hoy).**
> `grep -rn "ROJO_PEATON\|VERDE_PEATON\|BUZZER" Maestro/src Esclavo/src` devuelve **UNA sola
> coincidencia en las dos puntas, y es un COMENTARIO**. Ni un `pinMode`, ni un `digitalWrite`.
> **Si alguien cablea una cabeza peatonal a `J11`/`J9` o un zumbador a `J13`, no se enciende nunca y
> no hay mensaje de error**: el equipo parece sano — la averia mas cara de diagnosticar. **No se
> venden ni se incluyen en una entrega como funciones del equipo.** El
> cobre **si** esta: tres etapas completas, el mismo molde que `J15`; encender una cuesta **16 B de
> flash** de suelo (desensamblado del `.elf`, 05/09). Gastarlas es **`17_` §3.10, abierta**. 🟢 **Y ya
> NO tienen cuarto pretendiente: `D-32` (4), 13/09, cierra `D-14` para este despliegue** — el ancla vive
> en los dos `pines.h`, justo encima de estos tres pines, porque son sus tres candidatos.

## 2.1 `J16`, posicion por posicion

> 🛑 **La columna «red» es el NOMBRE DE LA RED EN KiCad, NO el papel del pin** — se bautizaron asi
> cuando la placa se diseno con botonera. **Quien lea `/Boton3` como «boton 3» y cablee un pulsador
> esta cableando en el borne de una camara.**

| `J16` | red | GPIO | que es HOY | **MEDIDO EN COBRE, paso 20, 03/09** |
|---|---|---|---|---|
| p1 | `/12V` | — | 🔴 **12 V crudos. SE TAPA** (`D-4`) | 12 V confirmados (pasos 3 y 4) |
| p2 | `GND` | — | **la unica masa del conector** | — |
| p3 · p6 | **sin red** | — | — | — |
| p4 · p7 · p9 · p11 | `3,3 V` | — | el borne contra el que se cierra el contacto | mismo nudo que `J14` p2 |
| p5 | `/Boton1` | `PB9` | 🟡 **LIBRE, y el firmware lo lee** (`botonArriba()`; ~~`MANDO_A`~~) — **no se cablea**, §3 | **9,92 kOhm** a masa · 11,28 kOhm a 3,3 V · **0,6 V** |
| p8 | `/Boton2` | `PB13` | 🟡 **LIBRE, y el firmware lo lee** (`botonAbajo()`; ~~`MANDO_B`~~) — **no se cablea**, §3 | **9,92 kOhm** · 11,28 kOhm · **0,6 V** |
| p10 | `/Boton3` | `PB14` | 🎯 **`CAM_C_PIN` — CAMARA 1 del poste** (`D-25`) | **9,93 kOhm** · 11,29 kOhm · **0 V** |
| p12 | `/Boton4` | `PB15` | 🎯 **`CAM_D_PIN` — CAMARA 2 del poste** (`D-25`) | **9,94 kOhm** · 11,31 kOhm · **0 V** |
| p13-p16 | sin red | — | pads que existen en el cobre y no en el esquema | — |

**El pull-down de 10 kOhm que declaraba el netlist es REAL y esta en las cuatro posiciones**
(`R65`-`R68`, con su 100 nF). Con 3,3 V en la posicion contigua, **el gesto que el conector pide es
cerrar el contacto contra los 3,3 V: entrada activa en ALTO, para los cuatro pines y sin excepcion.**
🛑 **El gesto de prueba es contra el 3,3 V contiguo —`p10` contra `p9`, `p12` contra `p11`— NUNCA contra masa**
*(el banco del 03/09 lo hizo tambien en `p5`/`p8`; en un equipo instalado esos dos no se puentean, §3)*: en todo `J16`
hay **una sola masa** (`p2`). Y **`p1` tapado antes de nada.**

# 3. Las camaras — **el cobre**

**Camara comprada: Hikvision `DS-2CD2683G2-IZS`** (`D-10`), **las CUATRO compradas** (`D-27` (1)),
**dos por poste** (`D-25`, 11/09: *«mantener estas conexiones como definitivas»*).

**El contrato es UN CONTACTO SECO y nada mas** (`D-12`): no hay red, no hay imagen, no hay video y no
hay analitica en el controlador — cero `WiFi`, `HTTPClient`, servidor ni ONVIF en el ESP32; el STM32
solo lee un pin. **Toda la inteligencia vive en la CONFIGURACION de la camara**, la del manual del
modelo comprado (`D-27` (3)). La camara **si graba en su propia microSD** (hasta 512 GB, ficha
oficial): el soporte de accidentes existe **en la camara, no en el firmware**.

**Cableado por poste** (`D-25`), cada camara por el **contacto seco de su salida de alarma** (`1A`/`1B`), que al detectar pone 3,3 V en el pin:

| camara | del borne | al borne |
|---|---|---|
| **camara 1** | `J16` **p9** (3,3 V) | `J16` **p10** (`PB14`, `CAM_C_PIN`) |
| **camara 2** | `J16` **p11** (3,3 V) | `J16` **p12** (`PB15`, `CAM_D_PIN`) |

## 3.1 Las DOS entradas hacen EXACTAMENTE LO MISMO — **`SPEC_8` §2**, con la tercera declarada

## 3.2 Una camara muerta desde la instalacion NO SE DETECTA SOLA — **`SPEC_8` §4** (su choque, §7.2)

## 3.3 Lo que la placa pone y lo que no

Las tres entradas son **`INPUT` PELADO y ACTIVAS EN ALTO**. `R64`/`R67`/`R68` son 10 kOhm a masa;
`C25`/`C28`/`C29`, 100 nF: **sobre el netlist las tres llevan el MISMO RC**. La asimetria publicada
(«`PB14`/`PB15` sin condensador») es **falsa** (11/09), y el comentario de `camara_leerPin()`
**todavia la dice** (§7). **Las resistencias estan medidas en cobre; los condensadores, NO.**

# 4. La talanquera — **la cadena electrica**

> 🔴 **Que HACE la pluma —cuando sube, cuando baja, quien la veta, quien la saca de servicio y que
> pasa en averia— es `SPEC_8`. Aqui solo el cobre por el que sale la orden.**

**Cadena:** `PB2` -> `R70` 220 / `R69` 10K -> opto `TLP127` (`U15`) -> MOSFET `IRLZ44N` (`Q10`) ->
bornera **`J15`** (p1 = 12 V, p2 = drenador). **`D-25` / `D-27` (4): `J15` p1/p2 a la bobina de un
rele, y los contactos del rele a `OPEN`/`COM` de la centralita.** **Lo que el COBRE garantiza, pin a
pin** (la regla completa es SFTY-28, y su tabla luz -> pluma es **SPEC 1 §2**):

- **La orden sale por la MISMA puerta que las luces** — dentro de `escribirPines()`, con el `verde`
  ya enclavado por SFTY-2. Una barrera con dos puertas no es una barrera.
- **Equipo SIN ENERGIA: el pin cae a LOW, el MOSFET no conduce, la pluma BAJA** — el fallo seguro. La
  compra pide **actuador con retorno por muelle o gravedad**: eso el software no lo garantiza.
- 🔴 **Y por eso «sacar la barrera de servicio» (`SPEC_8` §6) NO garantiza que el brazo se quede
  arriba si se va la luz:** sin energia la salida cae y el actuador baja por su muelle o por su peso.
  **El interruptor manda mientras haya corriente**; el cobre manda cuando no la hay.

⚠️ **MEDIDO EN COBRE (banco 04/09):** `J15` dio *«en rojo 0 V, en ambar 12 V»*. La causa se explico
el 05/09: **nueve de los diez drenadores llevan pull-up de 1 kOhm + LED al riel de 12 V** (`R23`,
`R28`, `R33`, `R38`, `R43`, `R48`, `R53`, `R58`, `R63`, `R73`), **en el cobre, no en el conector**.
**El borne de potencia en reposo esta a ~12 V, no a 0 V**, con ~10 mA por la cuenta `(12-2)/1k`.

> 🔴 **Un MOSFET a masa NO es un contacto seco.** El proyecto usa «contacto seco» con razon para las
> **entradas** de camara; **las salidas no lo son.** Y **el opto NO crea masa separada**: hay **UNA
> sola red `GND`** (103 pads, plano en las dos capas), con el catodo del LED del opto y la fuente del
> MOSFET dentro. **Lo colgado de esas borneras comparte la masa del controlador** — y sus 12 V.

# 5. La proteccion de esta placa es ASIMETRICA — **MEDIDO en el banco del 03-04/09**

| | cuantas | que llevan en medio |
|---|---|---|
| **Salidas** de campo | **9** *(⚠️ `17_` se corrige a si misma: son **DIEZ** cadenas, `Q1`-`Q10` con `U6`-`U15` — §7)* | `220 Ohm` en serie **+ opto `TLP127`** |
| **Entradas** de campo | **5** | 🔴 **nada.** Del borne **directo** a la pata del STM32 |

> **La tarjeta esta blindada contra lo que ella hace y desnuda contra lo que le hacen.** Cualquier
> tension que aparezca en un borne de entrada —un cruce de hilos en el armario, un cable de camara
> que roza `p1`, una descarga por la linea de una camara que vive fuera del gabinete— entra **entera**
> en una pata del micro que gobierna el semaforo.

**La propuesta para V2 —`2K2` en serie por entrada— es de `17_` §3.6, es del responsable, y NO se
decide aqui.** Su cuenta: con 12 V en el borne, **3,6 mA** (por debajo de los 5 mA/pata del
datasheet); nivel ALTO con la camara cerrada, **2,70 V** contra un `VIH` de **2,31 V**. 🔴 **Con
`4K7`: 2,24 V, POR DEBAJO de `VIH` — ya no leeria la camara.** No es «mas proteccion, mejor»: **hay
un techo y esta cerca**, y lo fija el pull-down de 10 kOhm que la placa ya trae.

# 6. `SIN VERIFICAR` — lo que el proyecto declara sin haberlo medido nunca

**La tabla se conserva ENTERA y con su numeracion**, porque se cita desde fuera por numero de fila.
Lo que no esta medido de la **conducta** —no del cobre— es `SPEC_8` §7.

| # | lo que nadie ha comprobado |
|---|---|
| 1 | **Ninguna camara AcuSense se ha conectado NUNCA a este equipo.** Lo que se cablo en el paso 21 fue **un puente de `p10` a `p11`**, no una camara |
| 2 | **Cual de los dos estados de la salida de la camara (NO/NC) significa demanda.** Es parametrizacion, y sigue **SIN TOMAR** |
| 3 | **`p12` no se ha cableado NUNCA**, ni en banco — y es el borne **mas cercano** a la red de 12 V (1,359 mm) |
| 4 | **La concesion de paso por camara.** El cableado si (paso 21, sin demandas fantasma); que el equipo **conceda** depende del Modo Automatico, que no se pudo seleccionar sin app |
| 5 | **Que `J9`, `J11` y `J13` esten REALMENTE SOLDADOS.** El esquematico los da `in_bom=yes`, `dnp=no`, `on_board=yes`; **nadie los ha mirado en cobre.** `J15`, el gemelo, si funciono: eso los hace **probables, no ciertos** |
| 6 | **Los ~12 V de reposo y los ~10 mA de las borneras de potencia.** Leidos del cobre, **no medidos con multimetro.** Se cierra en dos minutos midiendo p2 contra masa con la bornera desconectada |
| 7 | **Si el `D21` sin conectar de `J8` (`VERDE2`) es defecto o decision.** Su catodo esta sin conectar en esquema y cobre (red `unconnected-(D21-K-Pad1)`, cero pistas): **`J8` p2 FLOTA** en reposo mientras los otros nueve suben a 12 V. **No hay ni una nota en el repositorio.** Se cierra comparando `J8` p1-p2 contra `J7` p1-p2 |
| 8 | **Los condensadores `C25`/`C28`/`C29`.** En el netlist si; **en cobre, sin medir** — solo las resistencias (paso 20) |
| 9 | **El voltaje de `p5`/`p8` con el puente puesto.** El dato se perdio con el incidente de N-116 y **no se retoma sobre la Maestro** mientras siga con el corto |
| 10 | ~~**Que el mando `A`/`B` funcione con la polaridad corregida.**~~ → **sin sujeto desde el 14/09: el mando salio del firmware** (§3). Lo que sigue sin verificar es lo contrario y es de campo: **que firmware lleva cada equipo instalado**, porque con uno anterior un puente en p5/p8 **si** compone secuencias (`ESTADO.md`) |
| 11 | **La causa de N-116** — la tarjeta Maestro tiene un corto entre 3,3 V y `GND` **medido**, arranca ~30 s y se calienta. 🛑 **No se reenergiza.** El firmware queda descartado **por censo** (ninguna salida toca un pin de `J16`); eso **no nombra a nadie mas** |
| 12 | **El `2K2` de §5: no se ha probado en ninguna tarjeta.** Es aritmetica sobre el datasheet y sobre el pull-down medido, **no una medida** |
| 13 | **Que `PB3`/`PB4`/`PB5` se puedan usar con el ESP32 puesto.** Libres y en alta impedancia, con pista a `J17` p4/p1/p5 — **el mismo conector donde vive el modulo**. Usarlos **no cuesta el SWD** (`pinF1_DisconnectDebug()` hace `NOJTAG`, no `SWJ_DISABLE`) |
| 14 | **El nombre de `J17` p3.** La red del esquematico se rotula `RS(A0)` y el firmware lo llama `LCD_PSB`: **los dos no pueden ser ciertos a la vez.** Se cierra siguiendo el hilo hasta la pata rotulada, no leyendo mas codigo |
| 15 | **El pico de 500 mA del ESP32.** Es ficha y Manual 15, no medida — y en banco el modulo se alimento **por USB**, no por la fuente 12 V -> 5 V definitiva |
| 16 | **El `Y2` de la segunda tarjeta.** N-37 midio uno; el otro sigue sin diagnosticar |
| 17 | **Que por el enlace `J17` hable alguien.** El enlace fisico si (patas 42/43, masa comun < 50 mV, `GPIO17` a 3,3 V); **el Bluetooth no subio en toda la sesion de banco** |
| 18 | **Que espera electricamente el `ALARM IN` de la camara** (`D-14`): **sin tension, sin corriente**, y las palabras *«dry contact»* y *«relay»* **no aparecen en las 110 paginas** del manual del fabricante. Conectar una salida que en reposo esta a 12 V, con masa comun, a una entrada cuyo regimen no conocemos **no es un cableado: es un ensayo** — con multimetro y **antes** de escribir firmware. 🟢 **13/09: no urge, y por eso sigue aqui y no en la pagina 1.** `D-32` (4) decide que **`D-14` NO SE INSTALA en este despliegue** —medido sobre `Camaras_Sisga_4x.html`, la guia que sigue el instalador: solo cablea la SALIDA de alarma (`1A`/`1B`), nunca la entrada—. **La decision no borra la capacidad: dice que hoy no se monta**, igual que `D-27` (2) con `J14`. El dia que se instale, este ensayo va primero |
| 19 | **Que la sonda de la medida de `J15`** *(«en rojo 0 V, en ambar 12 V»)* estuviera entre p1 y p2. Es **deduccion** a partir de `TALANQUERA_ABRIR = HIGH`, no una lectura del informe |
| 20 | **Casi todo el resto del cobre de esta tarjeta.** El censo del 05/09 es lectura del `.kicad_pcb`, **no punta sobre la placa** |

> 🔴 **La unica medida de campo que hay no lo mejora:** la cinta del Maestro del Sisga (10/09,
> `evidencia/2026-09-10_Sisga_179DB0_cinta_tramas.txt`) trae **253 tramas con `CAM:` y las 253 dicen
> `CAM:?`**: en esos veinte minutos **ninguna camara le dio un flanco al equipo**. Con `D-25` son
> **cuatro** las que faltan por ver en cobre.

# 7. Choques que esta spec REPORTA y no resuelve

Manda la medida de cobre; el choque se escribe, no se arregla desde aqui (`CLAUDE.md` §12).

1. **`D-27` deja `J14` libre y el firmware sigue leyendo `PB0` como `CAM_DEMANDA_PIN`** en las dos puntas
   (aviso 2). Inofensivo con el borne vacio; **deja de serlo el dia que alguien cablee ahi.**
2. **La exencion del vigilante se escribio PORQUE `p12` iba vacio a proposito, y `D-25` le quito el
   motivo** (`CLAUDE.md` §6: una excepcion es una AFIRMACION sobre el codigo, y esta caduco). Sigue en
   el firmware: **una camara 2 muerta desde la instalacion no la avisa nadie** (`SPEC_8` §4). Pendiente EN
   FIRMWARE, no aqui.
3. **El comentario de `camara_leerPin()` en `botones.cpp` de las dos puntas dice que `PB14`/`PB15`
   «no llevan mas que el 10K de `R67`/`R68`».** El netlist trae `C28` y `C29` (100 nF). **Gana el
   cobre**; el comentario esta caducado y **no se toca desde esta spec**.
4. **`17_` se contradice en la cuenta de salidas de campo: «9» en tres sitios vivos y «DIEZ» en uno**
   (05/09). `pines.h` es explicito: **DIEZ MOSFET y DIEZ optos, `Q1`-`Q10` con `U6`-`U15`.** La
   asimetria del §5 no cambia de signo.
5. **`D-25` dice «las camaras no tocan el ciclo» y en Modo Inteligente SI alargan una fase** (con
   techo, `TECHO_POR_SUELO`). Es el conflicto (2) que `D-27` **no** cerro. El ciclo es SPEC 1.
6. **`CLAUDE.md` §2 enumera ocho pines de luz y `escribirPines()` mueve SEIS** mas la talanquera: la
   regla es **vacuamente cierta** para los dos peatonales (`N-96`; §2 de arriba y SPEC 1 §1).

*Decisiones recogidas: `D-1`, `D-2`, `D-3`, `D-4`, `D-10`, `D-12`, `D-13` (lo no derogado), `D-14`,
`D-25`, `D-27`, ~~`D-32` (1)~~ (derogada por `D-30`, reafirmada el 14/09), `D-32` (4), `D-30`. Abiertas que nombra sin resolver: `A-2`, `17_` §3.6 y §3.10. Las MEDIDAS
son de `17_Arquitectura_28-08_y_Decisiones_Abiertas.md`, que **gana a este fichero**; lo decidido, de
`DECISIONES.md`, que **gana a los dos**. El firmware se remidio contra `{Maestro,Esclavo}/include/
pines.h` y `src/{semaforo,botones,main}.cpp` el 12/09/2026; **§3, contra `botones.cpp`, `menu.cpp`,
`modo_hora.cpp` y `main.cpp` de las dos puntas el 15/09/2026**. **La conducta de camara y barrera se
partio a `SPEC_8` el 14/09/2026** (`roadmap.md` 1.45).*
