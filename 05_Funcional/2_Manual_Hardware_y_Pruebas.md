# MANUAL DE HARDWARE Y PRUEBAS (Ecosistema Semáforos - V9.0)

Este documento contiene las instrucciones paso a paso para el personal funcional encargado de ensamblar, probar y actualizar el firmware de los semáforos en campo o laboratorio.

## 📌 HARDWARE COMPLETO Y HOMOLOGADO (V9.0)

| Elemento | Estado | Ubicación / Conexión |
|---|---|---|
| **Pila `CR2032` en `VBAT`** | ✅ **Instalada en AMBAS tarjetas** (Maestro y Esclavo) | Alimentación directa RTC (R5 retirado). Ver §5 |
| **Pantalla LCD ST7920** | 🛑 **SE RETIRA (28/08/2026)** | No se lee desde el suelo. Sus pines `PB6`/`PB7` (conector `J17`) pasan al módulo Bluetooth. Ver §3 y §8 |
| **Botonera de `J16`** | 🛑 **SE RETIRA ENTERA (05/09/2026)** | ~~*Se queda en AMBOS, botones 1 a 4*~~ · ~~31/08: **quedan 2 pulsadores** (`PB9` p5 y `PB13` p8, mando `A`/`B`)~~ → **05/09 (`D-1`): tampoco quedan esos dos. `PB9` p5 y `PB13` p8 quedan LIBRES y sin cablear**; **`PB14` p10 y `PB15` p12 pasan a CÁMARAS**. **El código del mando NO se retira** — ver §6. Ver §3 y §6 |
| **Cámaras IA de demanda (1 y 3)** | ✅ **1 en Maestro + 1 en Esclavo** | Contacto seco `1A`/`1B` en `PB0`. Operativas. Ver §7 |
| **Cámaras IA en `J16` (`C` y `D`)** | 🟢 **YA ESTÁN EN EL FIRMWARE (31/08)** | `CAM_C_PIN` = `PB14`, `CAM_D_PIN` = `PB15`. **`INPUT` pelado, activo en ALTO.** ~~⚠️ **No se cablean hasta la medida `M3`**~~ *(`M3` **CERRADA** en el paso 20 del banco del 03/09: el pull-down de 10 kΩ es real, `9,93`/`9,94 kΩ` y `0 V` en reposo)* ni antes de **tapar `J16` p1 (12 V crudos)** — obligatorio desde N-120. Ver §7 |
| **Talanquera de barrera (`J15`)** | ✅ **PROBADA EN COBRE (04/09)** y **bien diseñada** | `PB2` → `R70` 220 Ω → `U15` (`TLP127`, **separa el pin del micro, NO la masa** — §11) → `R72` 220 Ω → puerta de `Q10` (`IRLZ44N`) → `J15`. 🔴 **`J15` p2 está a ~12 V en reposo** (`R73` 1 kΩ + `D29` al riel de 12 V), no a 0 V — §11. ⚠️ El diodo de rueda libre `D30` es un `1N4148` y **queda corto para un motor real** — va a la V2. Ver §10 |
| ~~**Cámaras IA de umbral (2 y 4)**~~ | 🛑 **NO SE INSTALAN EN `PB8`** | **`PB8` NO es entrada de cámara:** alimenta el LED testigo `D5` por `R16` de 1 kΩ. Se renombró a `LED_TESTIGO` (`pines.h:63`) y **`CAM_UMBRAL_PIN` ya no existe en el fuente** — N-64. Ver §7 |
| **Módulo de expansión ESP32** | 🟢 **IDENTIFICADO Y CON FIRMWARE (31/08)** | `ESP32-WROOM-32` clásico (**BT v4.2 BR/EDR → hay SPP**) por **`J17`** p2/p3 = `PB7`/`PB6` (`USART1` **remapeado**), ~~p6 = 3,3 V~~ 🛑 **p6 NO SE CONECTA** (pico de ~500 mA sobre el mismo `LM7805` del STM32 — Manual 1 §8; el módulo lleva **fuente propia**, línea `A5`, **sin pedir**), p7 = GND **(masa común obligatoria)**. **Sustituye al módulo SPP discreto** y trae ~~el reloj `DS3231` con pila propia~~ **el DRIVER del `DS3231`** (`ESP32_Expansion/src/reloj_ds3231.cpp`, `GPIO21`/`GPIO22`); 🛑 **el módulo NO está comprado (`A6`) y `0x68` sigue SIN VERIFICAR** — `contrato.h:185-186` lo dice en el propio fuente. **El firmware existe; la pieza no**, y sin ella la hora sale `--:--:--`, que es correcto. ⛔ **`J16` NO es `J17`: lleva 12 V.** Ver §8 |
| **Mando de 4 Relés Anti-Colisión** | 🛑 **EL HARDWARE NO EXISTE (05/09) · EL CÓDIGO SE QUEDA** | ~~*Cableado en paralelo con `PB9`..`PB15`*~~ · ~~31/08: **SE CONSERVA, solo `A` (`PB9`) y `B` (`PB13`)**~~ → **05/09, `DECISIONES.md` `D-1`: no hay emisor ni receptor. Nada se cablea a `J16` p5 ni p8.** 🛑 **Y el código NO se toca: retirarlo deja ABIERTO el veto de SFTY-21, no inerte.** Las dos mitades van juntas — ver §6 |

> 📱 **El Módulo Bluetooth en el Esclavo resuelve la operación desde el suelo:** Gracias al módulo Bluetooth de diagnóstico estándar Baliza instalado en el Esclavo (`USART1` por `J17`), el operario puede consultar el estado, ver alarmas y operar el Esclavo desde el celular sin necesidad de subir al poste a 5 metros de altura.

---

> # ⛔ 03–04/09/2026 — DOS COSAS QUE PARAN EL TRABAJO. LÉASE ANTES DE ENERGIZAR NADA
>
> **Esto sale de la sesión de banco del 3–4/09, con las tarjetas en la mano.** No es análisis de
> escritorio: el punto 1 se **midió con el multímetro** sobre la tarjeta Maestro, el punto 2 se **leyó
> del `.kicad_pcb`** —el fichero de cobre, no el esquemático— y el punto 3 se **ejercitó sobre la
> tarjeta**.
>
> ### 1. 🛑 LA TARJETA MAESTRO TIENE UN CORTO ENTRE EL RIEL DE 3,3 V Y MASA — **NO LA REENERGICE**
>
> **Medido.** La tarjeta funciona **unos 30 s** al alimentarla, **se calienta y se para**. **No se
> vuelve a energizar «a ver si pasa»**: cada minuto extra con un corto alimentado es lo que convierte
> un componente averiado en varios. **El firmware queda descartado por censo** — no se arregla
> cargando nada.
>
> **Procedimiento completo en §9.** No empiece por desoldar: el primer paso es gratis.
>
> ### 2. 🔴 LAS 5 ENTRADAS DE CAMPO VAN **DESNUDAS** AL PIN DEL STM32, Y `J16` p1 LLEVA 12 V CRUDOS
>
> Leído del `.kicad_pcb` (N-120). La tarjeta **no protege sus entradas como protege sus salidas**:
>
> | | protección en serie |
> |---|---|
> | Las ~~**9 salidas**~~ → **10 salidas** de la placa | ✅ **220 Ω + optoacoplador `TLP127`** — p. ej. `PA0` (`/S1`) → `R19` 220 Ω → `U6` `TLP127` → potencia. 🔴 **Son DIEZ, no nueve: `Q1`–`Q10` y `U6`–`U15`, contados en el `.kicad_pcb` el 05/09.** Censo completo en §11 |
> | Las **5 entradas de campo** (`PB0`, `PB9`, `PB13`, `PB14`, `PB15`) | 🔴 **NADA.** Van directas al pin del micro |
>
> **Consecuencia inmediata y sin discusión:** tapar físicamente `J16` p1 —los 12 V— deja de ser una
> precaución de banco y pasa a ser **OBLIGATORIO en cada equipo, antes de enchufar nada en ese
> conector**. Detalle, cuentas y la propuesta para la V2 en **§7**.
>
> ### 3. ✅ Y lo que SÍ funcionó en cobre, que también es dato
>
> ~~El banco cerró en **24/29**.~~ 🔴 **Corregido el 04/09: ese total NO se publica aquí como hecho,
> porque no cuadra consigo mismo.** `24 / 4 / 1` es la cifra de **la cabecera del informe** —24
> completos, 4 bloqueados por el enlace, 1 abortado por seguridad—, pero **su propia enumeración
> nombra 22 identificadores**, y los siete que faltan —`7`, `10`, `11`, `12`, `13`, `14`, `19`— sí
> están descritos en el cuerpo (como *PARCIAL*, *no logrado* o *BLOQUEADO*).
> `12_Cobertura_de_Pruebas_y_Huecos.md` mide esa discrepancia y **se niega a publicar un total**;
> este manual hace lo mismo. **No se inventa una reconciliación: la decide quien ejecutó la sesión,
> no el repositorio.**
>
> Lo que sí es dato, porque se vio funcionar sobre la tarjeta real: la **carga por SWD al primer
> intento y sin `BOOT0`**, la **radio** —caída a ámbar en ~20 s y vuelta en ~3 s—, la **talanquera de
> `J15`**, la **cámara de `J14`**, la **masa común a 0 V**, y quedó **resuelta la identidad de `J17`**
> (es el UART del módulo ESP32, **no** la pantalla que dice el netlist — ver §8).
>
> **Lo que esto NO es:** un permiso de carga a campo. En campo sigue corriendo la V8.4.

---

> # ✅ 05/09/2026 — **`N-118` CERRADO**, y era DOS COSAS contadas como una
>
> **Léase entero antes de tocar `J16`, y léanse LAS DOS MITADES: con una sola, la conclusión sale al
> revés.**
>
> | | qué se creía | qué es |
> |---|---|---|
> | **1** | *«los `0,6 V` de `MANDO_A`/`MANDO_B` son un defecto de la PLACA»* | ❌ **Falso, y el propio banco dejó el control.** Era el `INPUT_PULLUP` del **firmware viejo** peleando contra los 10 kΩ de la placa. **Corregido en `346ea5f`** |
> | **2** | *«queda pendiente probar el mando en banco»* | ⛔ **Ya no hay qué probar: `D-1`, 05/09 — el mando de A y B se retiró del equipo.** El emisor y el receptor no existen |
>
> 🛑 **Y la mitad que NO se puede leer sola: el CÓDIGO del mando SE QUEDA.** Retirarlo dejaría el
> veto de SFTY-21 **abierto**, no inerte. Está desarrollado en **§6**, y no es opcional.
>
> ### Cómo se cierra la mitad 1, que es una MEDIDA y no un razonamiento
>
> El paso 20 del banco del 03/09 midió **los cuatro** pines de `J16` con la tarjeta energizada, el
> conector vacío y el paquete `617bd00` dentro:
>
> | `J16` | pin | a masa | en reposo | `pinMode` en `617bd00` |
> |---|---|---|---|---|
> | p5 | `PB9` (`MANDO_A`) | `9,92 kΩ` | **`0,6 V`** | `INPUT_PULLUP` |
> | p8 | `PB13` (`MANDO_B`) | `9,92 kΩ` | **`0,6 V`** | `INPUT_PULLUP` |
> | p10 | `PB14` (`CAM_C_PIN`) | `9,93 kΩ` | **`0 V`** | `INPUT` pelado |
> | p12 | `PB15` (`CAM_D_PIN`) | `9,94 kΩ` | **`0 V`** | `INPUT` pelado |
>
> **Mismo cobre, misma resistencia, dos tensiones.** La única variable es el `pinMode`, y está
> medida sobre el binario que estaba dentro de la tarjeta aquel día:
>
> ```text
>   $ git show 617bd00:01_Firmware/Maestro/src/botones.cpp | grep -n "pinMode(BOTON\|pinMode(CAM_C"
>   139:  pinMode(BOTON1, INPUT_PULLUP);
>   140:  pinMode(BOTON2, INPUT_PULLUP);
>   156:  pinMode(CAM_C_PIN, INPUT);
> ```
>
> **Si los `0,6 V` los pusiera la placa, p10 y p12 marcarían lo mismo. Marcaron `0 V`.** El banco
> corrió un control negativo sin saberlo, y por eso *«defecto de placa»* queda **refutado con
> medida**, no con opinión.
>
> 🔴 **Lo que NUNCA se midió, y se dice aunque ya no importe:** nadie ha puesto un voltímetro en
> `J16` p5 **con el binario nuevo dentro**. El paso 29 se abortó por el sobrecalentamiento y la
> Maestro sigue con el corto de N-116. Esa confirmación queda **`SIN VERIFICAR`, y ya no se va a
> tomar** — porque no hay mando que conectar.
>
> ### El defecto histórico — **EN PASADO**, porque ya está arreglado en el fuente
>
> Los cuatro pines de `J16` llevan **10 kΩ a masa** (`R65`–`R68`, medidos `9,92`–`9,94 kΩ` en el
> banco del 03/09) y el conector reparte **3,3 V en la posición de al lado de cada uno** (p4, p7, p9,
> p11), con **una sola masa en todo el conector** (p2). El firmware leía `BOTON1`/`BOTON2` en
> `INPUT_PULLUP` y activo en BAJO: el pull-up interno (30–50 kΩ) contra esos 10 kΩ dejaba el pin en
> **0,55–0,83 V** —el banco midió **`0,6 V`**—, por debajo del `VIL`. Los dos botones estaban
> **clavados en «pulsado»** y **nunca producían un flanco**. Ésa era la causa de que el mando `A`/`B`
> no se pudiera pulsar, y **no era una regresión de este proyecto**: el repositorio de origen trae el
> mismo `== LOW` con el mismo `.kicad_pcb`.
>
> ### El estado de HOY — firmware arreglado, **pendiente de ejercer en tarjeta**
>
> **MEDIDO sobre el fuente el 04/09**, en las **dos** puntas (`Maestro/src/botones.cpp` y
> `Esclavo/src/botones.cpp`):
>
> ```text
>   pinMode(BOTON1, INPUT);   pinMode(BOTON2, INPUT);        <- INPUT PELADO, ya no INPUT_PULLUP
>   bool lecturaCruda = (digitalRead(b.pin) == HIGH);        <- ACTIVO EN ALTO, ya no LOW
>   const bool pulsado = (digitalRead(...) == HIGH);         <- la siembra del arranque, igual
> ```
>
> Los **cuatro** pines de `J16` son ahora eléctricamente idénticos y se leen igual: `INPUT` pelado,
> reposo fijado por el pull-down de 10 kΩ de la placa, **activo en ALTO**.
>
> ### 🔴 LA CONSECUENCIA PRÁCTICA — **05/09: NO HAY GESTO, PORQUE NO HAY MANDO**
>
> | | gesto | vale hoy |
> |---|---|---|
> | ~~Pulso `A`~~ | ~~tocar `J16` **p5** contra **masa** (p2)~~ | 🛑 **NO, y NUNCA se hace** — ver el aviso de abajo |
> | ~~Pulso `B`~~ | ~~tocar `J16` **p8** contra **masa** (p2)~~ | 🛑 **NO** |
> | ~~**Pulso `A`**~~ | ~~tocar un instante **`J16` p5 contra p4** (los 3,3 V de al lado)~~ | ⛔ **CADUCADO 05/09 (`D-1`): no hay mando que pulsar. `p5` queda LIBRE** |
> | ~~**Pulso `B`**~~ | ~~tocar un instante **`J16` p8 contra p7** (los 3,3 V de al lado)~~ | ⛔ **CADUCADO 05/09. `p8` queda LIBRE** |
>
> **Las dos últimas filas se conservan tachadas y no se borran** porque son el gesto correcto para
> los pines que **sí** se cablean —`p10` contra `p9` y `p12` contra `p11`, las cámaras—: sólo hay
> **una** masa en todo `J16` (p2), así que un contacto por señal contra masa nunca pudo ser el
> diseño, y eso vale igual para los cuatro pines.
>
> 🛑 **Y lo que sigue vigente sin cambio, que es lo único de este bloque que protege a alguien: NO SE
> PUENTEA `p5` NI `p8` CONTRA MASA.** Ni con la instrucción vieja, ni «a ver qué pasa», ni para medir
> nada. `p4` es **adyacente** a `p5` y `p7` lo es de `p8`: **un puente corrido una sola posición pone
> el riel de 3,3 V directamente contra masa**, y es el gesto que precedió al calentamiento del paso
> 29 (§9). **Con `p5` y `p8` libres, ahí ya no hay nada que tocar.**
>
> ### 👁️ Y para saber si el equipo OYÓ el pulso no hace falta ningún instrumento
>
> **MEDIDO en `01_Firmware/Maestro/src/mando.cpp:45-47`** (`DESTELLOS_AUTOMATICO = 2`,
> `DESTELLOS_AMBAR = 3`, `DESTELLOS_DEGRADADO = 4`):
>
> ```text
>   A . A . A       (<= 12 s)   ->  2 destellos ROJOS   Automatico
>   B . B . B       (<= 12 s)   ->  3 destellos ROJOS   Ambar intermitente
>   A . B . A . B   (<= 18 s)   ->  4 destellos ROJOS   Modo Degradado
>   rechazado                   ->  ambar rapido de 2 s
> ```
>
> **El firmware confirma con las propias luces, y se ven DESDE EL SUELO: sin app, sin cable y sin
> segunda tarjeta.** Está diseñado así a propósito —quien acciona el mando está a 5 m y sin
> pantalla—, y los destellos son **siempre rojos** porque el rojo nunca significa *pase*: si el
> operario cuenta mal, el peor caso sigue siendo seguro. Un rechazo **no habla el idioma de un
> éxito**: es un ámbar rápido de 2 s.
>
> 🛑 **La trampa, y hay que leerla antes del primer pulso: pruebe DESDE OTRO MODO.** Si el equipo
> **ya está** en el modo que la secuencia pide, `MODO:` **no cambia** —`mando.cpp` entra por la rama
> `if (modoActual_get() == MODO_AUTOMATICO) modoAutomatico_setup();`— y la prueba **no distingue
> nada**. Un `A·A·A` que funcionó perfectamente sobre un equipo ya en Automático no mueve ni un
> campo. **Los destellos, en cambio, se ven siempre.**
>
> 🔵 **Por eso el USB-TTL de `J17` baja de rango PARA EL MANDO.** Sigue siendo el recurso legítimo
> cuando la app no conecta —lo fue en la sesión 1 de banco— pero **no es la forma de verificar el
> mando**: la respuesta son los destellos. Anotar *«no cambió el `MODO:`»* mirando sólo el terminal
> es exactamente cómo un mando sano se apunta como sordo.
>
> ### 🛑 Y lo que sigue SIN MEDIRSE, que hay que decir en voz alta
>
> ~~**Nadie ha medido la tensión de `J16` p5/p8 con el puente a 3,3 V puesto y ESTE firmware
> cargado.** … Las pruebas del mando siguen **sin ejecutar**: *«no hay tarjeta sana con la que
> ejercerlo»*.~~
>
> 🔧 **05/09 — SE TACHA, Y EL MOTIVO CAMBIA POR TERCERA VEZ, QUE ES LO QUE HAY QUE ENTENDER:**
>
> | | por qué no se había probado el mando |
> |---|---|
> | hasta el 04/09 | *«el firmware no puede leer un flanco»* — **cierto, y corregido en `346ea5f`** |
> | el 04/09 | *«no hay tarjeta sana con la que ejercerlo»* — el corto de N-116 |
> | **desde el 05/09** | ⛔ **«no hay mando». `D-1`: el hardware se retiró.** La prueba no se aplaza: **se cancela** |
>
> **Esa medida queda `SIN VERIFICAR` para siempre, y este manual lo declara en vez de dejar la
> casilla abierta.** Una casilla pendiente invita a que alguien vuelva a intentarlo con un cable en
> `J16`; una prueba cancelada, con su motivo escrito, no.
>
> 🔴 **Lo que NO cambia y no depende del mando:** el corto de `N-116` sigue ahí y **la tarjeta Maestro
> no se energiza** (§9). **Un manual corregido no es un permiso de carga, y un firmware arreglado no
> es un firmware probado.** En campo sigue corriendo la **V8.4**.

---

> # 🟢 CAMBIOS DEL 31/08/2026 — LÉASE ANTES DE MONTAR NADA
>
> Cuatro cosas que este manual daba de otra manera. Todo lo de abajo es **MEDIDO sobre el fuente el
> 31/08**; lo superado se tacha con su motivo y no se borra.
>
> ### 1. La arquitectura: el ESP32 es un MÓDULO DE EXPANSIÓN, no un segundo controlador
>
> | | |
> |---|---|
> | **STM32** | **el controlador del cruce.** Sigue siendo quien mueve las luces |
> | **ESP32 por `J17`** | **módulo de expansión.** Aporta el **Bluetooth** —sustituye al módulo SPP discreto— y un **reloj `DS3231`** con pila propia (`GPIO21` `SDA` / `GPIO22` `SCL`, `ESP32_Expansion/include/contrato.h:142-143`) |
> | 🛑 **Lo que el ESP32 NO hace** | **No manda sobre las luces.** Es un puente: traduce y reenvía. La barrera de salidas sigue viviendo en `semaforo.cpp` del STM32 |
>
> **El módulo está identificado y el firmware existe.** Es un `ESP32-WROOM-32` clásico —`Xtensa LX6`
> doble núcleo, `Bluetooth v4.2 BR/EDR + BLE`—, o sea que **hay perfil SPP y la app conecta sin tocar
> una línea**. Su firmware vive en `01_Firmware/ESP32_Expansion/` y **la compuerta lo compila como una
> suite más** (`compila esp32`); la cifra de flash se lee del acta de `evidencia/`.
>
> ### 2. `J16` se parte: ~~dos pulsadores y dos cámaras~~ → **DOS PINES LIBRES Y DOS CÁMARAS**
>
> ```text
>    J16 p5    PB9    BOTON1 / mando A    INPUT pelado,  activo en ALTO   <- LIBRE, sin cablear
>    J16 p8    PB13   BOTON2 / mando B    INPUT pelado,  activo en ALTO   <- LIBRE, sin cablear
>    J16 p10   PB14   CAM_C_PIN           INPUT pelado,  activo en ALTO   <- CAMARA (era "Aceptar")
>    J16 p12   PB15   CAM_D_PIN           INPUT pelado,  activo en ALTO   <- CAMARA (era "Cancelar")
> ```
>
> > 🔴 **05/09 — AQUÍ PONÍA `<- SE QUEDA` EN LAS DOS PRIMERAS FILAS Y CONTRADECÍA A LA TABLA DE
> > BORNERAS DE ESTE MISMO MANUAL (§3), QUE YA DICE «⛔ nada. No se cablea».** Un manual que se
> > desmiente a sí mismo a 330 líneas de distancia manda al instalador a hacer lo que lea primero.
> > **Manda `D-1`: el hardware del mando se retiró (confirmado el 05/09) y su CÓDIGO se queda.**
> >
> > ⚠️ **Y «libre» no es «muerto», que es la parte que un resumen se come.** El firmware **sigue
> > leyendo `p5` y `p8`**: `BOTON1`/`BOTON2` alimentan `botonArriba()`/`botonAbajo()`, que **no**
> > son `return false;` —a diferencia de `botonAceptar()`/`botonCancelar()`, que sí lo son— y
> > tienen llamadores vivos en `menu.cpp` y `modo_hora.cpp` de las dos puntas. **Lo que se cierre
> > ahí contra los 3,3 V entra al firmware y compone secuencias de mando.** Por eso quedan **sin
> > cablear**, no «disponibles».
> >
> > ```
> > grep -n "bool botonAceptar\|bool botonCancelar\|bool botonArriba\|bool botonAbajo" Maestro/src/botones.cpp Esclavo/src/botones.cpp
> > grep -rn "botonArriba\|botonAbajo" Maestro/src Esclavo/src
> > ```
>
> > 🔧 **04/09 — las dos primeras filas se han corregido, y el motivo NO es cosmético.** Decían
> > ~~`INPUT_PULLUP, activo en BAJO`~~, que es lo que el firmware hacía **hasta el 04/09** y lo que
> > dejaba los dos pines clavados en `0,6 V` sin poder dar un flanco. **N-118 los pasó a `INPUT`
> > pelado y activo en ALTO en las dos puntas**, igual que las cámaras. Consecuencia para quien vaya
> > a banco: **el pulso se da contra los 3,3 V del pin de al lado —p5 contra p4, p8 contra p7—, NO
> > contra masa.** Ver el bloque del 04/09 arriba.
>
> **Se conservan los DOS canales del mando a propósito**, no por comodidad: `ambarLocal` —el veto de
> SFTY-21 en el Esclavo— **solo lo arma `B·B·B`**, y `A·A·A` es la única salida de ese ámbar desde el
> piso. `A·A·A`, `B·B·B` y `A·B·A·B` **siguen funcionando enteras**: ninguna usaba `C` ni `D`.
>
> ### 3. `SFTY6_SILENCIO_MS` son **25 s**, no 12
>
> **MEDIDO:** `Maestro/include/protocolo.h:149` y `Esclavo/include/protocolo.h:149` →
> `#define SFTY6_SILENCIO_MS 25000UL`. El valor de 12 s era **el techo de otra cuenta** y dejaba los
> reintentos 4 y 5 del ciclo sin poder ejecutarse jamás (N-71).
>
> > ⚠️ **No lo confunda con `VENTANA_TRIPLE_MS`, que sí vale 12 s y es correcto**
> > (`Maestro/src/mando.cpp:38`, `Esclavo/src/mando.cpp:42`): son los 12 s de la ventana de
> > `A·A·A` / `B·B·B` del mando, **otra magnitud y otro sujeto**. Dos números iguales en documentos
> > distintos no son el mismo número.
>
> ### 4. Y lo que NO cambia
>
> **En campo corre la V8.4.** Nada de lo descrito aquí ha pasado banco. **Un manual corregido no es un
> permiso de carga.**

---

> ## 🔴 CAMBIO DEL 28/08/2026 — LA PANTALLA SE RETIRA Y EL BLUETOOTH OCUPA SUS PINES
>
> **Por qué:** el equipo va montado en alto. Una LCD dentro del gabinete, a 5 m, **no se lee desde
> el suelo**, así que la interfaz de operación pasa a ser **la app por Bluetooth**.
>
> **Los tres cambios de firmware que lo hacen posible, ya aplicados en las DOS puntas y verificados
> leyendo el fuente el 28/08:**
>
> | fichero | qué cambió | por qué |
> |---|---|---|
> | `lcd.cpp` | el constructor de U8g2 recibe **`U8X8_PIN_NONE`** en lugar de `LCD_RST`, y `lcd_setup()` **ya no hace `pinMode`/`digitalWrite` sobre `LCD_PSB`** | así se sueltan **`PB6` y `PB7`**. Ninguno de los dos llevaba datos: `PSB` era un nivel estático y `RST` un pulso de arranque |
> | `bluetooth.cpp` | **`SerialBT(PB7, PB6)`** | el módulo entra por `J17`, que es **enchufable**. `PA9`/`PA10` no salen a ninguna bornera |
> | `protocolo.cpp` | `protocolo_setup()` **ya no llama a `AiBus.begin(115200)`** ni toca `RS485_IN_DE_RE` | `AiBus` estaba declarado sobre **el mismo `USART1`** que `SerialBT`, a 115200 contra 9600. Funcionaba **por accidente de orden**: `bluetooth_setup()` corre después y ganaba, así que el puerto quedaba a 9600 y *«el puerto IA» jamás existió a 115200* |
>
> ⚠️ **`AiBus` y `protocolo_actualizarAI()` NO se han borrado.** Esa función **no la llama nadie** en
> ninguna de las dos puntas —lleva huérfana desde siempre—, y retirarla es un cambio con sentido
> propio: irá en su propio `N-x`, no colado dentro de éste.
>
> ### ⚠️ La pantalla se va; el menú NO se ha ido del binario
>
> **Medido el 28/08:** `lcd.cpp`, `menu.cpp` y `modo_hora.cpp` siguen compilándose, `lcd_setup()`
> sigue llamando a `u8g2.begin()`, y ~~**los cuatro botones de `J16` siguen navegando el menú**~~. Lo
> único que falta es el display.
>
> ~~**Consecuencia para quien monte o mantenga el equipo:** pulsar esos botones —o accionar el mando
> de relés, que va en paralelo con ellos— **mueve un menú a ciegas**, y con los pulsos suficientes se
> llega a `AJUSTAR HORA` y **se confirma una hora que el equipo dará por válida**.~~
>
> > ## ✅ 31/08 — EL RIESGO DE «CONFIRMAR UNA HORA A CIEGAS» SE HA CERRADO POR CONSTRUCCIÓN
> >
> > **MEDIDO** en `Maestro/src/botones.cpp:305-306` y su equivalente del Esclavo:
> >
> > ```
> >   bool botonAceptar() { return false; }
> >   bool botonCancelar(){ return false; }
> > ```
> >
> > `PB14` y `PB15` son cámaras: **ya no hay pin que pueda levantar esos dos flancos**. El menú sigue
> > en el binario y los pulsadores `A`/`B` siguen **moviendo el cursor**, pero **no hay nada que
> > CONFIRME**. La cadena *«ráfaga de pulsos → cursor a `AJUSTAR HORA` → hora inventada dada por
> > válida»* **se corta en el último eslabón**, que era el único peligroso.
> >
> > *(Se devuelven `false` en vez de borrarlas a propósito: tienen veintitantos llamadores en nueve
> > ficheros, y borrarlas convertiría una reasignación de pines en una reescritura del control de
> > flujo de cada modo. Así `git grep botonCancelar` sigue listando en un solo sitio todo lo que la
> > retirada de `C` y `D` se llevó por delante.)*
>
> **Lo que sigue vigente de la instrucción anterior:** **no se accionan los botones del gabinete para
> «probar»**. `A` y `B` siguen siendo entradas reales y **siguen disparando las secuencias del
> mando** —incluido `A·B·A·B`, que pide entrar al Modo Degradado—.

---

> ## ⚠️ ANTES DE PROBAR: reconfigurar las radios a `2.4 kbps`
>
> El cableado descrito aquí es correcto y no cambia. Lo que **sí** debe cambiar es la
> **velocidad aérea (Air Data Rate)** de las radios: de `0.3 kbps` a **`2.4 kbps`**, en todas.
> Ver `4_Manual_Configuracion_Radios.md`. Sin ese ajuste el enlace sigue cayendo al paso de ciclo
> y el modo repetidor no funciona.

---

## 1. CABLEADO DE RADIOS INDUSTRIALES RS485 (E90-DTU)

> [!IMPORTANT]
> **ACLARACIÓN CRÍTICA DE CABLEADO:**
> Las radios industriales **E90-DTU (caja metálica)** utilizan la interfaz estándar **RS485 diferencial (2 hilos)**. **NO BUSQUE PINES TTL (DI, RO, DE, RE) EN LA RADIO**. La tarjeta impresa del semáforo (STM32) **ya incluye el integrado transceptor MAX3485 soldado**, por lo que la conexión se realiza mediante **solo 2 cables de datos** a la bornera de la tarjeta.

### Diagrama de Conexión Físico:
```text
+------------------------------------+        2 Hilos RS485       +------------------------------------+
| Tarjeta Controladora Semáforo STM32|                            |     Radio Industrial E90-DTU       |
|    (Chip MAX3485 Integrado)        |                            |         (Caja Metálica RF)         |
|                                    |                            |                                    |
|   Bornera Verde A  ----------------|----------------------------|----> Pin 485_A                     |
|   Bornera Verde B  ----------------|----------------------------|----> Pin 485_B                     |
|                                    |                            |      Pin V+ (12V - 24V DC)         |
|   Bornera GND / V- ----------------|----------------------------|----> Pin V- (Masa común / Tierra)  |
+------------------------------------+                            +------------------------------------+
```

### Reglas de Cableado RS485 (Directo, NO Cruzado):
1. **Bornera `A` de la Tarjeta Semáforo** $\rightarrow$ se conecta al **`Pin 485_A`** de la radio E90-DTU.
2. **Bornera `B` de la Tarjeta Semáforo** $\rightarrow$ se conecta al **`Pin 485_B`** de la radio E90-DTU.
3. **No se cruzan los hilos A y B:** A diferencia del puerto RS232 o UART TTL (donde TX va con RX), el bus RS485 es paralelo y mide voltaje diferencial ($V_A - V_B$). Si invierte los cables (A con B), la radio no recibirá datos.
4. **Sin control de dirección DE/RE externo:** Las radios E90-DTU conmutan la dirección de transmisión y recepción automáticamente por hardware interno. No requiere cablear pines de control hacia la radio.

---

## 2. Topologías de Red Soportadas

> En ambas topologías, **todas** las radios deben quedar con el mismo Air Data Rate de **`2.4 kbps`**,
> potencia `30 dBm` y `FEC: Enable`. Si una sola queda distinta, no enlazará con las demás.

### Opción 1: Modo Directo (2 Radios - Sin Repetidor)
* **Semáforo Maestro:** Tarjeta STM32 conectada por bornera A/B a la Radio 1 — Canal `0` (`170.0 MHz`).
* **Semáforo Esclavo:** Tarjeta STM32 conectada por bornera A/B a la Radio 2 — Canal `0` (`170.0 MHz`).
* **Funcionamiento:** La señal viaja transparente por aire. No requiere la tarjeta Repetidor ESP32.

> ### ✅ La Opción 1 es la CONFIGURACIÓN VIGENTE
>
> **2 radios en enlace directo, `2.4 kbps`, `M0`/`M1` ambos en OFF durante la operación, sin
> repetidor.** La Opción 2 se documenta porque el firmware del repetidor existe y la compuerta lo
> compila, **no porque haya un repetidor montado**. No monte uno sin reabrir esta decisión.

### Opción 2: Modo Repetidor (4 Radios - Esquinas Ciegas / Montaña) — **NO es el montaje vigente**
* **Semáforo Maestro:** Conectado a la Radio 1 — Canal `0` (`170.0 MHz`).
* **Repetidor Central (ESP32):**
  - Radio B1 en Canal `0` (`170.0 MHz`) — habla con el Maestro.
  - Radio B2 en Canal `10` (`172.0 MHz`) — habla con el Esclavo.
  - La tarjeta ESP32 reenvía los datos de forma asíncrona, liberando el bus RS485 tras 5 ms de silencio.
* **Semáforo Esclavo:** Conectado a la Radio 4 — Canal `10` (`172.0 MHz`).
* **Nota:** en esta topología cada mensaje da **dos saltos de aire por sentido**. Es la razón por la que la velocidad aérea de `0.3 kbps` resultaba insuficiente y quedó derogada.

---

## 2.1 📡 Antenas — la pieza que más alcance gana o pierde

> **Prueba de campo del 31/07/2026:** con las antenas originales el alcance era de **1 cuadra,
> esquina y 2 cuadras más**. Para 1 W a 170 MHz eso es muy poco — en línea de vista deberían ser
> kilómetros. La causa casi segura: **antenas que no son de la banda**.

### El error más caro: comprar en el mercado equivocado

Las antenas genéricas vendidas como "antena LoRa" suelen ser de **433 o 915 MHz**. Puestas en un
radio de 170 MHz **rinden pésimo**: se pierden fácilmente 15–20 dB, que es justo la diferencia entre
tres cuadras y varios kilómetros.

**170 MHz cae en la banda VHF comercial (136–174 MHz)** — la misma de los radios de taxi, seguridad
y vigilancia. **Ahí es donde hay que comprar**, en distribuidores de radiocomunicación profesional,
no en tiendas de electrónica de hobby.

### La física manda el tamaño

A 170 MHz la longitud de onda es de **1,76 m**. No hay forma de hacer una antena eficiente y corta:

| Tipo | Ganancia | Longitud |
|---|---|---|
| Látigo ¼ de onda | ~0 dB | **~42 cm** |
| Fibra de vidrio 1 sección, ⅝ de onda | ~3 dB | **~1 m** |
| Colineal fibra de vidrio 2 secciones | 6–7 dB | **~3,2 m** |
| Yagi 5 elementos | 9,2 dB | boom 1,5–2 m |

> ⚠️ **Desconfía de cualquier antena de 15–20 cm vendida como "170 MHz".** Son helicoidales cargadas
> y su rendimiento es una fracción del de un cuarto de onda real.

### Cómo repartirlas en este sistema

| Equipo | Antena recomendada | Por qué |
|---|---|---|
| **Maestro y Esclavo** (móviles) | Fibra de vidrio **1 sección ⅝ de onda, ~1 m** | Se transportan y reubican. Una de 3,2 m es inmanejable en una unidad móvil |
| **Repetidor** (fijo en poste) | **Yagi 5 elementos**, una apuntando a cada extremo | Está fijo: ahí cabe antena grande y es donde más rinde. La directividad además **reduce el acoplamiento entre las dos radios del poste** |

### 📻 Una sola antena cubre 170 y 172 MHz

**No hacen falta dos modelos distintos.** Las dos frecuencias caen dentro de la banda **VHF comercial
136–174 MHz**, y cualquier antena que cubra ese rango sirve para ambas.

> ⚠️ **Al comprar, verificar el rango que declara la antena.** Muchas antenas "VHF" vienen sintonizadas
> solo para **136–144 MHz** (banda de radioaficionado) y **no cubren 170**. Hay que exigir que el rango
> declarado incluya **148–174 MHz** o **136–174 MHz**.

### ⚠️ Los conectores no coinciden — pídelos con la antena

**Lado del radio — dato verificado en el datasheet oficial** (`E90-DTU(230SL37)_UserManual_EN_V1.5`,
tabla de interfaces, punto 8):

> *"Antenna interface — **SMA-K** interface, external thread, 10 mm, **50 Ω** characteristic impedance"*

`SMA-K` es **SMA hembra**: cuerpo con rosca exterior y contacto central hueco. Lo que se le enrosca
debe ser **SMA macho**.

**Lado de la antena:** las antenas profesionales de VHF traen **`UHF hembra (SO-239)`** —lo más
habitual en fibra de vidrio— o **`N hembra`**. **No conectan directo al radio.**

Cadena completa de conexión:

```
   Antena VHF omnidireccional
   └─ UHF hembra (SO-239)
            │
   Cable coaxial RG-8X
   └─ UHF macho (PL-259) en ambos extremos
            │
   Adaptador o pigtail   UHF hembra → SMA macho
            │
   Radio E90-DTU
   └─ SMA-K (hembra, rosca exterior, 50 Ω)
```

Pide siempre junto con las antenas:

- Adaptador o pigtail **UHF/N → SMA macho**
- Cable coaxial **RG-8X** *(pierde menos que el RG-58 en VHF)* **con los conectores ya instalados de
  fábrica** — un conector mal soldado en obra es una de las causas más frecuentes de pérdida
- Todo de **50 Ω**. El coaxial de 75 Ω de televisión **no sirve**

*Si pides las antenas sin esto, llegan y no se pueden instalar.*

> ⚠️ **El dato del conector está verificado sobre el datasheet de la variante `230SL37`.** Toda la
> familia E90-DTU SL usa la misma caja e interfaz, pero **antes de comprar conviene mirar el conector
> del radio físico** — son diez segundos y evita un pedido equivocado.

### Modelos disponibles en Colombia

El sistema usa **170 y 172 MHz**. La columna de la derecha indica si el modelo cubre **las dos**.

| Referencia | Tipo | Banda | Ganancia | ¿170 y 172? |
|---|---|---|---|---|
| `TX-AB-136-74-FG1` (TXPRO) | Fibra, 1 sección ⅝ | 136–174 MHz | ~3 dB | ✅ Sí |
| `TXAB-136-74-FG2` (TXPRO) | Colineal, 3,2 m | 136–174 MHz | 6,7 dB | ✅ Sí |
| `1490` (TRAM Browning) | Colineal fibra | 144–174 MHz | 6,7 dB | ✅ Sí |
| `HX6-160-70` (Hustler) | Fibra, climas extremos | 160–170 MHz | 6 dB | ❌ **NO — se queda en 170** |
| `MYA-1505K` (PCTEL) | **Yagi 5 elementos** | 150–174 MHz | 9,2 dB | ✅ Sí |
| `MYA-1503K` (PCTEL) | Yagi 3 elementos | 150–174 MHz | 7,1 dB | ✅ Sí |

> ⚠️ **La `HX6-160-70` queda descartada para este sistema.** Su rango termina justo en 170 MHz y
> **no llega a 172**. En el borde de banda el acople empeora, la potencia se refleja hacia el radio en
> lugar de radiarse y **el alcance en 172 sería aún peor que hoy**. Se deja en la tabla precisamente
> para que nadie la pida por descuido.

**Proveedores:** SYSCOM Colombia, Sumitec Comunicaciones, PCredcom.
**Pide por la banda `136-174 MHz`**, no por "antena LoRa".

### Lo que ninguna antena resuelve

En ciudad densa **ninguna antena atraviesa edificios**. Lo que salva las esquinas es:

1. **La altura.** Subir la antena 3 m suele rendir más que duplicar la potencia. Es la palanca
   principal a VHF.
2. **La ubicación del repetidor.** No va a mitad de camino: va donde **vea los dos extremos**,
   normalmente arriba del talud o del poste, no en la vía.

### 🔥 Y una advertencia que ya costó un radio

**Transmitir con la antena en mal estado daña el amplificador de potencia.** Con 1 W bastan unas
horas radiando contra un conector suelto, un coaxial roto o una antena de otra banda.

**Antes de energizar un radio nuevo, revisa su coaxial y su conector.** Si lo montas sobre el cable
que causó la avería anterior, quemas también el radio de reemplazo.

---

## 3. Interfaz Local — 🔴 RETIRADA EL 28/08. QUEDA LA BOTONERA, SIN PANTALLA

**La LCD ST7920 (128×64) ya no se instala.** La configuración de tiempos, los modos y la puesta en
hora se hacen **desde la app por Bluetooth** (§8). Lo que sigue se conserva porque **el hardware de
la botonera sigue en la tarjeta y el menú sigue en el firmware**, y quien monte el equipo tiene que
saberlo.

~~La pantalla LCD (128x64) permite configurar los tiempos de la vía mediante 4 pulsadores físicos:~~

* ~~**Botón 1 (Arriba / +):** Sube el valor en la pantalla o cambia la luz en Modo Manual.~~
* ~~**Botón 2 (Abajo / -):** Baja el valor o cambia la luz en Modo Manual.~~
* ~~**Botón 3 (Aceptar / OK):** Confirma la selección.~~
* ~~**Botón 4 (Cancelar / Menú):** Sube un nivel de menú.~~

> ## ⚠️ ~~LOS BOTONES SIGUEN CONECTADOS Y EL MENÚ SIGUE VIVO — A CIEGAS~~
>
> ~~**Esto no es una nota histórica: es el estado de hoy.** El firmware **no ha perdido el menú**, solo
> ha perdido dónde dibujarlo. Los cuatro pulsadores de `J16` siguen entrando por `PB9`, `PB13`,
> `PB14` y `PB15`, y siguen navegando exactamente el mismo menú de dos niveles de antes.~~
>
> ~~**Lo que eso permite, sin que nadie lo vea:** llegar a `CONFIGURACION → AJUSTAR HORA` y confirmar
> **una hora cualquiera**, que el equipo dará por buena.~~
>
> ### ✅ 31/08 — SUPERADO EN SU MITAD PELIGROSA. QUEDAN DOS PULSADORES, NINGUNO CONFIRMA
>
> **MEDIDO** (`botones.cpp:305-306` (Maestro) / `:316-317` (Esclavo) en ambas puntas): `botonAceptar()` y `botonCancelar()` devuelven
> **`false` siempre**, porque `PB14` y `PB15` **ya no son pulsadores**, son `CAM_C_PIN` y
> `CAM_D_PIN`. El cursor se puede mover con `A`/`B`; **no se puede confirmar nada**, y por tanto
> **no se puede dejar una hora inventada dada por buena**.
>
> ~~**Lo que SÍ sigue en pie, y es la razón de que la instrucción de montaje no cambie:** `PB9` y
> `PB13` son entradas reales, y **`A·B·A·B` sigue pidiendo entrar al Modo Degradado**.~~
>
> ### 🛑 05/09 — LA OTRA MITAD TAMBIÉN SE VA, Y LA INSTRUCCIÓN DE MONTAJE **SÍ** CAMBIA
>
> **`DECISIONES.md` `D-1`:** *«ya no tenemos mandos de A y B, sólo la app, los quitamos»*. **Los dos
> pulsadores y el mando de relés se retiran del equipo.**
>
> **Instrucción de montaje (vigente desde el 05/09):** ~~los dos botones se dejan cableados —los usa
> el mando de relés—, pero nadie los pulsa para «ver qué pasa»~~ → **`J16` p5 y p8 quedan LIBRES: no
> se les cablea nada, ni pulsador ni relé.** No hace falta cartel: no hay nada que pulsar.
>
> 🛑 **Y lo que NO se hace, que es lo único de aquí que puede quemar una tarjeta: no se puentean p5
> ni p8 contra masa** para «ver si el firmware responde». `p4` es adyacente a `p5` y `p7` lo es de
> `p8`; un puente corrido una posición pone el riel de 3,3 V contra masa (§9).
>
> ✅ **Lo que sigue siendo cierto y por eso no se borra:** el firmware **sigue declarando esas dos
> entradas y sigue teniendo su máquina de secuencias**, y eso es deliberado — **§6**. Sin nadie que
> las accione, `A·B·A·B` no se compone nunca y las entradas se quedan en su reposo de `0 V`.

### Pines de `J16` — **repartidos el 31/08, polaridad corregida el 04/09 (N-118)**

| `J16` | Pin del STM32 | 28/08 | Función vigente — **05/09** | Modo del pin | Se acciona |
|---|---|---|---|---|---|
| p5 | `PB9` | 1 — Arriba / `A` | ~~✅ mando `A`~~ → ⚪ **LIBRE.** El mando se retiró (`D-1`) | **`INPUT` pelado, activo en ALTO** | ⛔ **nada. No se cablea** |
| p8 | `PB13` | 2 — Abajo / `B` | ~~✅ mando `B`~~ → ⚪ **LIBRE.** El mando se retiró (`D-1`) | **`INPUT` pelado, activo en ALTO** | ⛔ **nada. No se cablea** |
| p10 | `PB14` | ~~3 — Aceptar / `C`~~ | 🛑 **`CAM_C_PIN` — cámara** | **`INPUT` pelado, activo en ALTO** | p10 contra p9 (3,3 V) |
| p12 | `PB15` | ~~4 — Menú / `D`~~ | 🛑 **`CAM_D_PIN` — cámara** | **`INPUT` pelado, activo en ALTO** | p12 contra p11 (3,3 V) |

**Ambas tarjetas usan los mismos pines** (`Maestro/include/pines.h`, símbolos `BOTON1`, `BOTON2`,
`CAM_C_PIN` y `CAM_D_PIN`; idéntico en el Esclavo):

```text
  $ grep -n "define BOTON1\|define BOTON2\|define CAM_C_PIN\|define CAM_D_PIN" \
        01_Firmware/Maestro/include/pines.h
  146:#define BOTON1      PB9   // J16 p5  - Arriba / mando A
  147:#define BOTON2      PB13  // J16 p8  - Abajo  / mando B
  148:#define CAM_C_PIN   PB14  // J16 p10 - camara de contacto seco (era BOTON3, "Aceptar")
  149:#define CAM_D_PIN   PB15  // J16 p12 - camara de contacto seco (era BOTON4, "Cancelar")
```

~~El mando de relés se cablea **en paralelo con los dos pulsadores que quedan** — no hay entradas
dedicadas para él (ver §6).~~ 🛑 **CADUCADO EL 05/09 (`D-1`): no hay mando de relés que cablear.**
`p5` y `p8` **quedan libres**, y el firmware las sigue declarando como entradas —eso es correcto y no
se toca, ver §6—: **una entrada declarada que nadie acciona se queda en su reposo de `0 V`, fijado
por los 10 kΩ de la placa.**

> 🔧 **Las dos primeras filas cambiaron el 04/09 y la última columna es nueva.** Los cuatro pines de
> `J16` son eléctricamente idénticos —10 kΩ a masa y 3,3 V en la posición de al lado— y desde N-118
> **los cuatro se leen igual**. **El contacto se cierra contra el pin de 3,3 V vecino, nunca contra
> masa**: sólo hay **una** masa en todo el conector (p2), así que un contacto por botón contra masa
> nunca pudo ser el diseño. La instrucción vieja *«toque p5 contra masa»* **no produce nada** con
> este firmware.

> ⛔ **El cambio de polaridad NO es cosmético, y por eso el orden importa.** `R65`–`R68` son 10 kΩ
> **a masa** y `J16` saca 3,3 V en p4, p7, p9 y p11: con `INPUT_PULLUP` el pin se queda en
> 3,3 × 10/50 = **0,66 V**, que el micro lee `LOW` — **entrada permanentemente accionada sin nada
> conectado**. Por eso van en `INPUT` pelado y activo en ALTO.
>
> 🔧 **04/09 (N-118) — y eso ya no vale sólo para p10 y p12: vale para los CUATRO.** El mismo defecto
> que producía *«demanda permanente sin cámara»* en p10/p12 producía *«botón permanentemente
> pulsado»* en p5/p8 — medido en banco a **`0,6 V`**, dentro de la horquilla que predice la cuenta de
> arriba. **`BOTON1` y `BOTON2` pasaron a `INPUT` pelado y activo en ALTO en las dos puntas.**
>
> **Firmware primero; el cableado después. Un commit no protege de un destornillador.** ~~Con el
> firmware viejo todavía dentro, `PB14` sigue siendo *Aceptar* leído **activo en BAJO**: cualquier
> hilo que un instalador enchufe en `J16` p10 **pulsa Aceptar en un equipo que está en la calle**.~~
> *(Caducado el 04/09: `botonAceptar()` devuelve `false` desde el 31/08 y desde N-118 ningún pin de
> `J16` se lee activo en BAJO.)* **Lo que sigue vigente sin cambio: se exige la carga verificada en
> la tarjeta, no el merge**, porque el gesto de accionar esos pines es el opuesto en cada firmware —
> un instalador que trabaje con la instrucción de la versión que no está cargada no acciona nada, o
> acciona lo que no quería. ~~Y **no se cablea cámara a `J16` hasta la medida `M3`**~~ — **`M3` quedó
> CERRADA en el paso 20 del banco del 03/09**: el pull-down de 10 kΩ es real y medido (`9,93` y
> `9,94 kΩ`, `0 V` en reposo).

> 🔴 **04/09 (N-120) — y por debajo de la polaridad hay algo que la polaridad no arregla: `PB14` y
> `PB15` van DESNUDOS al pin del micro.** Leído del `.kicad_pcb`: entre la posición de `J16` y la
> pata del STM32 **no hay ni resistencia en serie ni optoacoplador** — lo que sí tienen las nueve
> salidas de la placa. El `10 kΩ` y el `100 nF` de esas entradas están **en PARALELO a masa**: fijan
> el reposo, que es lo útil contra el pin flotante, pero **no limitan corriente**.
>
> **Con `J16` p1 a 12 V crudos a nueve posiciones de p10, eso convierte un hilo mal puesto en un pin
> del micro a 12 V.** Tapar p1 es **obligatorio**, no recomendable. Las cuentas y la propuesta para
> la V2 están en **§7**; aquí basta con la instrucción de montaje.

### Las dos unidades tienen interfaz, pero menús distintos — 📕 HISTÓRICO (hasta el 28/08)

> **Esta tabla describe el equipo CON pantalla.** Se conserva porque el menú sigue en el binario y
> porque su última fila —*por qué el Esclavo no ajusta hora ni modos*— **sigue siendo la regla
> vigente**, ahora aplicada a la app en vez de a la pantalla.

| | Maestro | Esclavo |
|---|---|---|
| LCD y botonera | ~~✅~~ 🛑 sin LCD desde el 28/08; botonera sí | ~~✅~~ 🛑 igual |
| Menú | 2 niveles: `MANUAL` / `AUTOMATICO` / `INTELIGENTE` / `CONFIGURACION`, y dentro `PRUEBA ALCANCE` / `AJUSTAR HORA` / `MODO DEGRADADO` | 2 opciones: `ESTADO` / `MODO DEGRADADO` |
| Ajuste de hora | ✅ Única vía para poner el reloj | ❌ **A propósito** — la hora llega por radio |
| Modos de operación | ✅ | ❌ **A propósito** — quien manda el ciclo es el Maestro |

> **El Esclavo no ofrece ajuste de hora ni modos de operación, y no es un olvido.** Ajustar la hora en
> las dos puntas a mano deja hasta 59 s de desfase que dos pantallas no pueden detectar; y ofrecer los
> modos en el Esclavo invita a dejar las dos unidades peleando por el ciclo.

### Verificación del Modo Legal (Norma Colombia Res. 2024 - V8.0)
1. Ingrese a **Modo Manual**, **Automático** o **Inteligente**.
2. Al ordenar el paso a Verde:
   - Focos físicos: El estado pasa de **Rojo** $\rightarrow$ **Amarillo Fijo (4.0s)** $\rightarrow$ **Verde** (en Maestro y Esclavo).
3. Al ordenar el paso a Rojo:
   - Focos físicos: El estado pasa de **Verde** $\rightarrow$ **Rojo** DIRECTO (0s de Amarillo).

---

## 4. INSTRUCCIONES PARA ACTUALIZAR EL FIRMWARE (VSCode + PlatformIO)

Siga estos pasos al pie de la letra para compilar y flashear el firmware en las tarjetas.

**Paso 1: Abrir la carpeta específica en VSCode**
1. Abra **VSCode** y haga clic en el ícono de **PlatformIO** en la barra lateral.
2. Haga clic en **"Open Project"**.
3. **CRÍTICO:** Navegue y seleccione ÚNICAMENTE la subcarpeta específica correspondiente:
   - `01_Firmware/Maestro` si va a flashear la tarjeta Maestro STM32.
   - `01_Firmware/Esclavo` si va a flashear la tarjeta Esclavo STM32.
   - **`01_Firmware/ESP32_Expansion` si va a flashear el módulo de expansión del `J17`** *(el del
     Bluetooth y el `DS3231`) — nuevo el 31/08.*
   - `01_Firmware/Repetidor` si va a flashear la tarjeta Repetidor ESP32 *(no es el montaje vigente,
     ver §2)*.

**Paso 2: Conectar y Flashear**
1. Conecte el STM32 (vía ST-Link) o la ESP32 (vía USB) a la computadora.
2. Haga clic en el ícono de **visto bueno (✓ Build)** en la barra azul inferior. Esperar mensaje `[SUCCESS]`.
3. Haga clic en el ícono de la **flecha a la derecha (→ Upload)** para grabar la placa.
4. *(Tip para chips CKS32):* Si el ST-Link falla al conectar, mantenga presionado el botón RESET físico de la tarjeta BluePill al presionar "Upload" y suéltelo justo cuando aparezca "Connecting..." en la consola.

> ⚠️ **Las dos tarjetas deben llevar firmware de la MISMA versión.** El cálculo de la fase del Modo
> Degradado vive en un fichero que es idéntico en los dos proyectos, y las dos puntas tienen que
> calcular exactamente lo mismo a partir de la hora. **Flashear versiones distintas en cada punta
> puede romper la fase sin ningún aviso en pantalla** — cada unidad creería estar en lo correcto.
> Anote la versión y el commit cargados en cada tarjeta.

> ✅ **03/09 — medido en banco: la carga por SWD entró al PRIMER INTENTO y SIN tocar `BOOT0`.** El
> truco del RESET del paso 4 queda como **recurso si falla**, no como parte del procedimiento: si
> hace falta recurrir a él de entrada, lo que hay que sospechar es el cable o la alimentación de la
> tarjeta, no el chip.

---

## 5. 🔋 La pila del RTC (`CR2032` en `VBAT`)

Desde el 01/08/2026 **ambas tarjetas llevan pila**. Es lo que hace que la hora sobreviva a un corte de
energía; sin ella habría que reponerla a mano tras cada apagón, y el Modo Degradado quedaría
inutilizable en la práctica.

### Qué se hizo en la tarjeta

| Elemento | Detalle |
|---|---|
| Cristal | **`Y2` de 32.768 kHz**, ya venía en el diseño, cableado a `PC14`/`PC15` |
| Pila | **`CR2032` NO recargable**, 3 V, soldada al pad de `R5` del lado `VBAT` y a GND |
| **`R5` retirado** | Era un puente de 0 Ω que unía `VBAT` con los 3,3 V |
| Pines ocupados | **Ninguno.** El RTC es interno del STM32 |

> ## ⚠️ POR QUÉ HAY QUE RETIRAR `R5` ANTES DE PONER LA PILA
>
> `R5` une `VBAT` con la fuente de 3,3 V. **Si se conecta la pila sin quitarlo, la `CR2032` queda en
> paralelo con los 3,3 V y la fuente le inyecta corriente.** Una pila **no recargable** en esa
> situación **se calienta, se hincha y puede reventar**.
>
> Verificación previa, con la tarjeta **sin alimentación** y el multímetro en continuidad: una punta
> en el pin 1 de `U1` (`VBAT`) y otra en 3,3 V. **Si pita**, `R5` los une y hay que desoldarlo. **Si
> no pita**, `R5` va a otro sitio: **no continúe** hasta aclararlo.

> ⚠️ **No confundir el tipo de pila.** En `VBAT` de esta tarjeta va **`CR2032` (no recargable)**,
> porque **el STM32 no carga la pila**. La `LIR2032` recargable solo corresponde a los módulos
> externos `DS3231`, que sí traen circuito de carga. **Poner la equivocada en cualquiera de los dos
> casos es peligroso.**

### Detalle completo de la instalación

El procedimiento paso a paso —puntos de GND cómodos para soldar, verificación con multímetro y la
alternativa por módulo `DS3231` si el cristal no arranca— está en
`03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md §4`. **Este manual no lo duplica a propósito:** dos copias
de un procedimiento de soldadura acaban divergiendo.

> 🛑 **NO CUELGUE UN `DS3231` DE LOS PINES DEL STM32 CREYENDO QUE FUNCIONARÁ.** Medido el 28/08 y
> **revalidado el 31/08**: **no hay driver de `DS3231` en ninguna de las dos puntas STM32.** El reloj
> que usa el STM32 es su **RTC interno** con el cristal `Y2` y la pila en `VBAT` (SFTY-18). Un módulo
> `DS3231` soldado hoy al STM32 sería **una pieza de hardware sin una sola línea de software que la
> lea**.
>
> La detección automática de `DS3231` **por parte del STM32** está **diseñada y no construida** —
> `OPTIMIZACIONES.md`, **SFTY-26**, marcada `DISEÑO, NO IMPLEMENTADO`. **Esa alternativa es un plan,
> no un repuesto.**
>
> > ## 🟢 31/08 — PERO EL `DS3231` SÍ EXISTE, EN EL OTRO LADO DEL `J17`
> >
> > **El módulo de expansión ESP32 trae su propio `DS3231` con pila propia**, en `GPIO21` (`SDA`) /
> > `GPIO22` (`SCL`), y **su driver existe y compila**: `01_Firmware/ESP32_Expansion/src/reloj_ds3231.cpp`.
> >
> > **No es lo mismo y la diferencia importa al pedir material:**
> >
> > | | dónde vive | driver |
> > |---|---|---|
> > | RTC interno + `Y2` + `CR2032` en `VBAT` | **STM32** | ✅ `reloj.cpp` |
> > | **`DS3231` + pila propia** | **módulo ESP32 del `J17`** | ✅ `reloj_ds3231.cpp` **(ESP32)** |
> > | `DS3231` colgado del STM32 | — | ❌ **no existe** |
> >
> > ⚠️ **Y la pila NO es la misma:** en `VBAT` del STM32 va **`CR2032` no recargable**; los módulos
> > `DS3231` externos llevan circuito de carga y usan **`LIR2032` recargable**. **Poner la equivocada
> > en cualquiera de los dos casos es peligroso** — ya está dicho arriba y se repite aquí porque ahora
> > hay dos relojes en el equipo y es exactamente donde se confunden.

### Vida útil — no es mantenimiento periódico

Con ~1,4 µA de consumo por `VBAT`, la autonomía teórica supera los **15 años**. El límite real es la
**caducidad de la pila (~10 años)**, no su descarga. No hay que programar sustituciones.

### 🧪 Dos pruebas de banco que faltan — y por qué importan

> **Ninguna de las dos se ha hecho todavía.** Hasta que se hagan, el reloj está construido pero no
> verificado, y todo lo que depende de él va sobre un supuesto.

| Prueba | Qué se comprueba | Por qué importa |
|---|---|---|
| **Contraste contra hora patrón y corte de energía** (N-15) | Que el RTC marca la hora correcta y **la conserva** al desconectar la alimentación | Si la pila no está bien soldada, el equipo pierde la hora en cada apagón y **nadie se entera hasta que el Degradado se rechaza en obra** |
| **Arranque con el cristal `Y2` desconectado o fallido** (N-17) | Que el equipo **bootea igual** y declara la hora como no fiable | Algunos microcontroladores clonados traen mal los condensadores de carga y **el oscilador de 32.768 kHz no arranca**. La rutina de reloj ya se movió detrás del watchdog para que un bloqueo sea un reinicio visible y no un cuelgue mudo con las luces apagadas — pero eso **hay que comprobarlo con la tarjeta en la mano** |

> ⚠️ **Y no se cambia nada antes de leer `CONSULTA RELOJ` (N-37, abierto).** Existe una hipótesis
> razonable —que los condensadores de carga de $Y_2$ estén mal calculados, y que sustituirlos por
> **6 a 10 pF (C0G/NP0)** arregle la oscilación—, pero **hoy no está medida**. Este proyecto ya
> pagó una vez por saltarse este paso: una pantalla acusaba a *"Y2, pila y R5"* sin haber medido
> ninguno de los tres, y mandó a cambiar componentes sanos.
>
> **El orden es: primero la lectura, después la pieza.** ~~`CONFIGURACION` → `AJUSTAR HORA` →
> confirmar. Si la hora queda puesta y no aparece pantalla de error, **el cristal no era el
> problema y no se toca nada**. Si aparece `CONSULTA RELOJ`, solo la línea `Pedido, no oscila`
> señala al cristal; `LSE no se pide`, `Oscila; RTC no atado` y `Oscila y atado a LSE` dicen que
> no era.~~ Con esa lectura en la mano, la sustitución de condensadores pasa a ser una petición
> concreta al funcional, no una conjetura.
>
> 🛑 **`CONSULTA RELOJ` YA NO SE PUEDE ABRIR (medido el 02/09).** La pantalla se sigue dibujando,
> pero está dentro de `CONFIGURACION` y llegar ahí necesita **dos pulsaciones de *Aceptar***
> (`menu.cpp:111`, `:129`); `botonAceptar()` devuelve `false` siempre desde que `PB14`/`PB15` son
> cámaras (`botones.cpp:305-306` (Maestro) / `:316-317` (Esclavo)). **Sus cuatro líneas de diagnóstico tampoco viajan en `$STATUS`.**
>
> ✅ **Pero el diagnóstico NO se ha perdido: se mudó a un `$EVENT`.** Ver el bloque del 01/09, más
> abajo, que es donde está el procedimiento que sí se puede ejecutar hoy.
>
> Lo que **sí** se puede hacer hoy desde la app: poner la hora con
> `CMD:PIN:1234:SET_RTC:YYYY-MM-DD,HH:MM:SS` y leer el campo `HORA:` de `$STATUS`. **Si la hora
> queda puesta y sigue avanzando, el cristal oscila**; si `HORA:` devuelve `--:--:--`, el reloj se
> declara no fiable. Eso distingue *«funciona»* de *«no funciona»*, pero **NO distingue cuál de las
> cuatro causas es** — que era exactamente el punto de aquella pantalla.
>
> ~~**Consecuencia honesta: mientras ese diagnóstico no esté en la app, N-37 no se puede cerrar por
> lectura.**~~ ✅ **Ya está en la app desde el 01/09** — ver el bloque de abajo. Lo que no cambia es
> la regla: **no se sustituyen condensadores por conjetura.** Primero la lectura, después la pieza.
>
> > ### ✅ 31/08 — el instrumento ha mejorado a medias, y conviene saber hasta dónde
> >
> > **MEDIDO** en `Maestro/src/bluetooth.cpp`: `SET_RTC` ya **no contesta `OK` sin mirar**. Tiene
> > **cinco ramas** y dos de ellas son diagnósticas:
> >
> > | Respuesta | Línea | Qué dice |
> > |---|---|---|
> > | `$ERR,CMD:SET_RTC,DESC:SIN_CRISTAL_VEA_CONSULTA_RELOJ` | `:313` | **No hay con qué contar el tiempo.** La hora NO quedó puesta |
> > | `$ACK,CMD:SET_RTC,RESULT:HORA_PUESTA_SIN_PROPAGAR` | `:325` | Entró aquí, **no viajó al Esclavo** |
> > | `$ACK,CMD:SET_RTC,RESULT:OK` | `:327` | Entró y va camino del Esclavo |
> > | `$ERR,CMD:SET_RTC,DESC:FORMATO_INVALIDO` | `:308`, `:318` | La trama no se pudo leer, o cifras fuera de rango |
> >
> > Y existe además `CMD:PIN:1234:REINICIAR_RELOJ` (`:330`), que contesta
> > `CRISTAL_OK_PONGA_LA_HORA` o `SIGUE_PARADO_VEA_CONSULTA_RELOJ`.
> >
> > **Por qué esto importa en el poste:** la versión anterior mandaba `RESULT:OK` **sin mirar
> > ninguna de las dos llamadas**, así que con `Y2` muerto el equipo decía que sí y no ponía la hora
> > — y el técnico se iba creyendo que lo había dejado puesto. Eso ya no pasa.
> >
> > ~~🔴 **Lo que sigue faltando, y no se disimula:** esas respuestas distinguen *«no hay cristal»*
> > de *«sí lo hay»*, pero **siguen sin distinguir cuál de las cuatro causas**. **N-37 sigue sin
> > poderse cerrar por lectura.**~~
> >
> > ### ✅ 01/09 — YA SE PUEDE. Los cuatro diagnósticos llegan a la app
> >
> > **El hueco de arriba está cerrado, y por eso se tacha.** Los seis bits que pintaba `CONSULTA
> > RELOJ` salen ahora en una trama de evento, **detrás de los dos `$ERR` que nombran esa
> > pantalla** — que es justo cuando hay alguien mirando:
> >
> > ```
> >   $ERR,CMD:SET_RTC,DESC:SIN_CRISTAL_VEA_CONSULTA_RELOJ
> >   $EVENT,NODE:MAESTRO,ORIGEN:RELOJ,DETALLE:ON:1 RDY:0 BYP:0 SEL:1 EN:1 CNT:0,HORA:--:--:--
> > ```
> >
> > **Se lee en la pestaña `Eventos` de la app**, y sale con `SIN_CRISTAL_VEA_CONSULTA_RELOJ` y con
> > `SIGUE_PARADO_VEA_CONSULTA_RELOJ` (`Maestro/src/bluetooth.cpp:305-333`, emitido en `:542` y
> > `:577`).
> >
> > | lo que se lee | qué significa |
> > |---|---|
> > | `ON:0` | **`LSE no se pide`** — el oscilador ni siquiera está pedido. Es firmware o dominio de respaldo, **no el cristal** |
> > | `ON:1 RDY:0` | **`Pedido, no oscila`** — aquí **sí** se mira el cristal `Y2` y sus condensadores |
> > | `ON:1 RDY:1 SEL:0` | **`Oscila; RTC no atado`** — el cristal va bien; lo que falla es el enganche |
> > | `ON:1 RDY:1 SEL:1` | **`Oscila y atado a LSE`** — el reloj está sano |
> > | `CNT:--` | **no se pudo leer** el contador. **No es `CNT:0`**: un cero leído es otro diagnóstico |
> >
> > 💡 **Para saber si el contador AVANZA, repita el mismo `SET_RTC` unos segundos después y compare
> > `CNT`.** En esta rama el comando se rechaza **antes** de escribir nada, así que no cuesta.
> >
> > 🔴 **Y esto es lo que decide si se tocan los condensadores: primero la lectura, después la
> > pieza.** Sólo `ON:1 RDY:0` señala al cristal. Con cualquiera de las otras tres, **cambiar `C1`/`C2`
> > es cambiar componentes sanos** — que es exactamente lo que este proyecto ya pagó una vez.

> **Un semáforo no puede depender de un cristal de reloj para encender.** Ésa es la razón de la
> segunda prueba, y es la más fácil de olvidar porque en una tarjeta sana nunca se nota.

---

## 6. 🎛️ El mando de relés — 🛑 **EL HARDWARE NO EXISTE · EL CÓDIGO SE QUEDA (05/09)**

> # ⛔ 05/09/2026 (`DECISIONES.md` `D-1`) — **YA NO HAY MANDO**
>
> *«Ya no tenemos mandos de A y B, sólo la app, los quitamos»* — el responsable, 05/09.
>
> | Canal | Pin | `J16` | 31/08 | **05/09** |
> |---|---|---|---|---|
> | ~~**`A`**~~ | `PB9` | p5 | ~~✅ se conserva~~ | ⛔ **el hardware se retira. Pin LIBRE** |
> | ~~**`B`**~~ | `PB13` | p8 | ~~✅ se conserva~~ | ⛔ **el hardware se retira. Pin LIBRE** |
> | ~~`C`~~ | `PB14` | p10 | 🛑 se retira → cámara | 🛑 **cámara** |
> | ~~`D`~~ | `PB15` | p12 | 🛑 se retira → cámara | 🛑 **cámara** |
>
> ## 🛑 Y AQUÍ VAN LAS DOS MITADES JUNTAS, PORQUE POR SEPARADO SE LEE LO CONTRARIO
>
> | | |
> |---|---|
> | **El HARDWARE se fue** | Emisor y receptor. No se compra ninguno *(Manual 15, línea `A9`)*, no se cablea nada a `J16` p5 ni p8, y **el paso 29 del banco no se repite** |
> | 🛑 **El CÓDIGO se queda, y no se toca** | `mando.cpp` sigue entero en las dos puntas. **Retirarlo NO deja el veto de SFTY-21 inerte: lo deja ABIERTO** |
>
> ### Por qué el código se queda — **está medido, no es prudencia**
>
> El veto de SFTY-21 del Esclavo cuelga de la bandera `ambarLocal`, y **tiene un solo armador**:
>
> ```text
>   $ grep -rn "ambarLocal = true" 01_Firmware/Esclavo/src
>   Esclavo/src/mando.cpp:132:      ambarLocal = true;
>
>   $ grep -rn "mando_ambarLocal()" 01_Firmware/Esclavo/src/main.cpp \
>                                   01_Firmware/Esclavo/src/bluetooth.cpp
>   Esclavo/src/bluetooth.cpp:551:      if (semaforo_estado() == S_FALLO && !mando_ambarLocal()) {
>   Esclavo/src/bluetooth.cpp:562:      if (mando_ambarLocal()) {
>   Esclavo/src/main.cpp:453:      if (!mando_ambarLocal() && !bluetooth_ambarEmergencia()) {
>   Esclavo/src/main.cpp:476:      if (!mando_ambarLocal() && !bluetooth_ambarEmergencia()) {
>   Esclavo/src/main.cpp:617:    if (!mando_ambarLocal() && !bluetooth_ambarEmergencia() &&
> ```
>
> **Todos los lectores la usan para VETAR.** Si se borra la línea que la arma, esos `if` pasan a ser
> **siempre verdaderos**, el veto **desaparece**, y **ningún test falla** — el código que abre el
> agujero está en otro fichero. Con el mando desmontado, en cambio, **la bandera simplemente no se
> arma nunca**, que es exactamente lo correcto: el veto sigue en pie y lo sigue armando la app por
> `bluetooth_ambarEmergencia()`.
>
> 🔴 **Y hay una segunda razón, de instrumento: el banco se caería en `ABORTADO`, no en rojo.** Dos
> `raise` disparan solos y los dos modelos leen constantes de `mando.cpp` **en el import**. Un
> `ABORTADO` no es un `FALLA`: **no dice nada del firmware**, y mientras dura, todo lo que esos packs
> vigilaban entra sin mirar.
>
> ### 📵 La consecuencia de operación, y hay que decirla entera (`D-16`)
>
> **Retirado el mando, la app es la ÚNICA superficie de mando del equipo.** No queda pantalla
> —retirada el 28/08—, no quedan pulsadores —`botonAceptar()` y `botonCancelar()` devuelven `false`
> siempre desde el 31/08— y ahora tampoco queda mando.
>
> 🛑 **SIN TELÉFONO NO HAY FORMA DE OPERAR EL EQUIPO: ni ámbar, ni volver a Automático, ni parar el
> cruce.** Un móvil sin batería, un emparejamiento que falla o dos técnicos con el mismo equipo dejan
> el poste **sin mando de ninguna clase**. No es una avería: **es una propiedad declarada del
> sistema**, y por eso va escrita aquí y no en una nota. **El teléfono es herramienta crítica**:
> batería, cable de carga, y **conviene un segundo terminal ya emparejado**.

~~El operario acciona el equipo **desde el piso**, con un mando de relés cableado en paralelo con los
pulsadores físicos.~~ 🛑 **Caducado el 05/09.** Lo que sigue se conserva **como histórico del
diseño** —porque el código que lo implementa sigue dentro y alguien tendrá que entenderlo— y **no
como instrucción de montaje**: no hay nada que montar ni que comprar.

### Características medidas en campo (01/08/2026) — condicionan el diseño

| Medida | Valor | Consecuencia |
|---|---|---|
| Tipo de señal | **Pulso por flanco**, no se sostiene | **La pulsación larga NO existe.** Sostener el botón 10 s da un solo pulso |
| Retardo por pulsación | **~2 s** | Una ventana de 3 s es inviable; hacen falta 12–18 s para 3–4 pulsos |
| Repetición automática | **No la hay** | Cada pulso exige una pulsación |

Si va a reemplazar o comprar mando, **verifique estas tres características antes de pedirlo**: todo el
diseño de las secuencias y de la pantalla `AJUSTAR HORA` (edición dígito a dígito) descansa sobre
ellas. Un mando que se comporte distinto obliga a rehacer ambas.

> ## 🪜 ~~EL ESCLAVO NO TIENE RECEPTOR DE MANDO (pendiente N-19)~~ → ⛔ **CERRADO POR `D-1`, Y NO COMO SE ESPERABA**
>
> 🛑 **05/09: esto deja de ser una compra pendiente y pasa a ser una LIMITACIÓN ACEPTADA.** No es que
> el receptor siga faltando: es que **ya no se compra ninguno, en ninguna de las dos puntas**
> (`Manual 15`, línea `A9`). **Ni el Maestro ni el Esclavo pueden operarse desde el piso sin
> teléfono** — ver `D-16`, arriba.
>
> ~~**Hoy solo el Maestro puede operarse desde el piso.** La tarjeta del Esclavo ya trae las entradas
> **`PB9` y `PB13`** y su firmware las atiende: **falta únicamente comprar e instalar el
> receptor**.~~ **Lo que sigue siendo cierto de esa frase: las entradas y su firmware siguen ahí.**
> Lo que ya no: que falte comprar algo.
>
> > 🔴 **31/08 — y en el Esclavo esto ha subido de categoría.** Con *Aceptar* mudo y **sin `SET_MODO`
> > por Bluetooth en esa punta** —el Maestro es el único que arbitra el ciclo—, **el mando pasa a ser
> > el único actuador de modo del Esclavo**: entrar al Degradado es `A·B·A·B` y salir es `A·A·A` o
> > `B·B·B` *(en `Esclavo/src/mando.cpp`: `degradado_entrar()` bajo el caso `ACC_DEGRADADO`, y
> > `degradado_salir()` en las dos ramas de salida)*. ~~Mientras
> > el receptor no esté comprado, **esas dos acciones solo se hacen subiendo al gabinete y pulsando
> > `A` y `B` a mano**.~~
>
> ## 🔴 05/09 — Y AL RETIRAR EL MANDO ESO DEJA DE TENER SALIDA. **HAY QUE DECIRLO, NO RESOLVERLO AQUÍ**
>
> **`D-1` retira la única vía que tenía el Esclavo para entrar o salir del Modo Degradado, y no la
> sustituye por otra.** Censadas hoy las cuatro, una por una:
>
> | vía | ¿existe en el Esclavo? | medido |
> |---|---|---|
> | **Mando de relés** (`A·B·A·B`) | ⛔ **el hardware se retiró** | `D-1`, 05/09 |
> | **Pulsadores del gabinete** | ❌ | `botonAceptar()` / `botonCancelar()` devuelven `false` siempre desde el 31/08 |
> | **App por Bluetooth** | ❌ **no existe el comando** | `grep -c "SET_MODO" Esclavo/src/bluetooth.cpp` → **`0`**. Lo que esa punta acepta es `AMBAR_EMERGENCIA`, `CANCELAR_AMBAR`, `SOLICITAR_PASO` y `SET_RTC` |
> | **Orden del Maestro por radio** | ❌ **imposible por diseño** | *el radio muerto es justamente la razón de entrar al Degradado* |
>
> ```text
>   $ grep -c "SET_MODO" 01_Firmware/Esclavo/src/bluetooth.cpp
>   0
> ```
>
> 🛑 **O sea que hoy el Modo Degradado NO SE PUEDE ACTIVAR EN EL ESCLAVO POR NINGUNA VÍA.** Antes del
> 05/09 la respuesta era *«subiendo al gabinete»*; ya no lo es, porque arriba tampoco hay con qué
> —ni pantalla ni botones que confirmen—.
>
> ⚠️ **Este manual NO decide qué se hace con eso, y no propone la solución obvia.** Es una decisión
> vial y del responsable, y las opciones que se le pongan delante son un instrumento: **añadir
> `SET_MODO` al Esclavo tiene consecuencias sobre quién arbitra el ciclo** —hoy sólo el Maestro—, y
> eso no se decide desde una lista de compras. **Lo que este documento hace es dejar el hueco escrito
> con su medida**, para que no se descubra la noche que se caiga la radio.
>
> ✅ **Lo que sí sigue siendo cierto y no cambia:** **mientras el enlace funcione, el sistema
> funciona.** El Degradado es el procedimiento de excepción, no la operación normal.

> ## ⚠️ ~~AL COMPRAR EL RECEPTOR: EXIJA CÓDIGO INDEPENDIENTE DEL MANDO DEL MAESTRO~~
>
> 🛑 **CADUCADO EL 05/09 (`D-1`): no se compra ningún receptor.** ~~Si ambos receptores responden al
> mismo mando —y las dos puntas suelen estar a menos de una cuadra— una sola secuencia metería las
> dos unidades en Modo Degradado a la vez. Pídalo con código o dirección distinta.~~
>
> ✅ **Se conserva tachado porque el RIESGO que describía no era del mando, sino del Degradado, y ése
> sigue vigente:** el procedimiento del Modo Degradado **exige verificación por separado de cada
> punta**, y cualquier vía futura que active las dos a la vez —un mando compartido entonces, un
> comando de radio o un botón de la app mañana— **se salta lo que hace seguro ese modo**. Quien
> diseñe esa función en la app tiene aquí el motivo escrito.

### ~~Verificación del cableado del mando~~ — ⛔ **RETIRADA EL 05/09: no hay mando que cablear**

🛑 **Esta lista completa se retira, no se «deja por si acaso».** Una lista de comprobación sobre
hardware que no existe es una invitación a puentear `J16` con un cable para poder marcarla, y ése es
justo el gesto que precedió al calentamiento del paso 29.

- [ ] ~~Cada relé cierra contra **el mismo pin** que su pulsador correspondiente (`A`→`PB9`, `B`→`PB13`)~~
- [ ] ~~Con el equipo en el menú, accionar `A` mueve el cursor **una sola posición** por pulsación~~
- [ ] ~~Accionar `C` desde el mando **selecciona**, igual que el Botón 3~~ *(ya retirado el 31/08)*
- [ ] ~~Accionar `A·A·A` dentro de la ventana produce **2 destellos rojos**; `B·B·B`, **3**; `A·B·A·B`, **4**~~
- [ ] ~~Los canales `C` y `D` del mando **no producen ningún efecto**~~
- [ ] ~~El mando **no** genera pulsos espurios al energizar el gabinete~~

> ✅ **Lo único que sobrevive de esta lista, y no es una comprobación de mando sino de firmware:** los
> **destellos rojos** siguen existiendo en el código y siguen siendo la forma en que el equipo acusa
> desde el suelo. Hoy **no hay quién los dispare**, pero el mecanismo está y **no se retira**: es el
> mismo `mando.cpp` cuyo veto sostiene SFTY-21. Si algún día vuelve una vía de mando local, la
> confirmación ya está construida y medida.
>
> 🔴 **Y lo que ya no se puede escribir aquí:** *«el mando no genera pulsos espurios al energizar»*.
> **No hay mando, así que no hay nada que lo genere** — pero `p5` y `p8` quedan como **entradas
> desnudas al pin del micro** (N-120, §11) con su reposo fijado a `0 V` por los 10 kΩ. **Dejarlas sin
> cablear es lo que las mantiene calladas.**

---

## 7. 📷 Conexión de las Cámaras (Hikvision AcuSense) — ~~Sistema de 4 Cámaras IA~~

> 🛑 **«4 Cámaras» se tacha el 05/09: EN EL CRUCE HAY DOS, UNA POR POSTE.** El «4» venía de contar
> *bornes de entrada* como si fueran cámaras, y de las dos «cámaras de umbral» que **no existen**
> (`PB8` es un LED testigo — N-64, ver el bloque de abajo). Lo comprado son **2 × `DS-2CD2683G2-IZS`**
> *(`15_Lista_de_Compras_Hardware.md`, fila `A2`)*. Que un manual de montaje diga «4» hace que
> alguien pida el doble de soportes, de cable y de alimentación.

> ### 📜 EL CONTRATO DE LA CÁMARA, PARA QUIEN LA VA A MONTAR (D-12, 05/09/2026)
>
> **De cada cámara este equipo consume UN CONTACTO SECO: dos hilos y un bit.** Eso es todo lo que
> cruza de la cámara al controlador.
>
> * ❌ **No hay red entre la cámara y la tarjeta.** En el poste no va switch, ni router, ni cable de
>   datos. **El `RJ45` de la cámara sólo se usa en taller**, para configurarla, y después no va a
>   ninguna parte. *(Si alguien previó una canalización de datos hasta el gabinete: no hace falta.)*
> * ❌ **La tarjeta no recibe imagen ni vídeo, y no hace analítica.** El STM32 hace `digitalRead()`
>   de un pin y nada más. La clasificación *vehículo / persona* ocurre **dentro de la cámara**.
> * ✅ **Lo que SÍ hay que llevar a cada cámara es ALIMENTACIÓN**, y no es despreciable: **12 VDC,
>   máx. `13 W`** por cámara *(o PoE 802.3at Clase 4, máx. `15 W`)* — ficha, pág. 4. **Dos cámaras =
>   `26 W` que hoy no están en el presupuesto de energía del poste.**
> * ✅ **Y un SOPORTE de fijación por cámara.** Pesa **`1.385 g`** *(ficha, pág. 4)* y mide
>   **`308,5 mm`** de largo: no se cuelga de cualquier sitio, y **nadie lo ha especificado todavía**
>   *(está anotado como hueco en la lista de compras)*.
>
> 🟢 **Lo que la cámara sí puede hacer por su cuenta, para que nadie lo dé por perdido:** admite
> **microSD de hasta 512 GB** *(ficha, pág. 3)* y sabe grabar por evento, así que **el soporte de
> accidentes y la auditoría son posibles EN LA CÁMARA** — sin tocar el firmware. Pero **esas imágenes
> no pasan por el controlador**, y sin red en el poste **se recuperan subiendo a retirar la tarjeta**.
> La tarjeta **no está comprada** y la política de retención **no está definida**. Ver **Manual 9**.
>
> ⚠️ **Una cámara = una salida = UN SIGNIFICADO.** La ficha dice **`1 input, 1 output`** (pág. 3):
> **una sola salida por cámara**, así que cada cámara sólo puede decir **una** cosa. Hoy el
> significado tomado es **«hay vehículo esperando»**; el segundo está **sin decidir** (`A-1` de
> `DECISIONES.md`) y **no lo decide el instalador**.
>
> 🛑 **Y la regla que no se salta al montar:** una cámara **desconectada o sin corriente deja el pin
> en reposo, y el reposo se lee como «no hay nadie»**. El pin **no distingue silencio de vía libre**.
> Por eso una cámara sólo **pide** paso, nunca **autoriza** — y por eso el todo-rojo de despeje es
> **temporizado** y no espera a que ninguna cámara diga que el tramo está vacío.

Para detección vehicular por demanda en obra vial (analítica embebida sin computadores externos):

```text
       CAMARAS HIKVISION ACUSENSE                     TARJETA CONTROLADORA STM32
  ┌─────────────────────────────────┐              ┌───────────────────────────────┐
  │ CAMARA 1 (Demanda Sentido 1)    ┼─ Hilos 1A/1B ┼──► Bornera PB0 (Entrada Libre)│
  │ CAMARA C  (31/08)               ┼─ contacto ───┼──► J16 p10 = PB14  CAM_C_PIN  │ (En Maestro)
  │ CAMARA D  (31/08)               ┼─ contacto ───┼──► J16 p12 = PB15  CAM_D_PIN  │
  ├─────────────────────────────────┤              ├───────────────────────────────┤
  │ CAMARA 3 (Demanda Sentido 2)    ┼─ Hilos 1A/1B ┼──► Bornera PB0 (Entrada Libre)│
  │ CAMARA C / D  (31/08)           ┼─ contacto ───┼──► J16 p10/p12 = PB14/PB15    │ (En Esclavo)
  └─────────────────────────────────┘              └───────────────────────────────┘

       PB8  ->  R16 1K  ->  LED testigo D5.   NO ES ENTRADA DE CAMARA.
```

> ## 🟢 31/08 — LAS CÁMARAS `C` Y `D` YA ESTÁN EN EL FIRMWARE
>
> **MEDIDO:** `CAM_C_PIN` = `PB14` y `CAM_D_PIN` = `PB15` (`pines.h:124-125`, idéntico en las dos
> puntas), declarados en `botones.cpp` con **`pinMode(..., INPUT)` pelado** y leídos **activos en
> ALTO**.
>
> ⚠️ ~~**Pero NO se cablea nada a `J16` todavía**, por dos motivos independientes:~~ → 🟢 **QUEDA UN
> MOTIVO, NO DOS. LAS CÁMARAS SE CABLEAN.**
>
> 1. ~~**La medida `M3` sigue pendiente.** Con `INPUT` pelado el pin necesita **resistencia real a
>    masa en la placa**… de `PB14`/`PB15` **solo lo dice el netlist y nadie lo ha medido en cobre**.~~
>    🛑 **CADUCADO EL 03/09 Y SE TACHA CON SU MOTIVO — este bloque era el último de este documento
>    que daba `M3` por pendiente, y contradecía a `:13` y `:539` del propio fichero.**
>    **`M3` ESTÁ CERRADA CON MULTÍMETRO** (paso 20 del banco, 03/09): el pull-down de **10 kΩ es real
>    y está en las CUATRO posiciones** (`R65`–`R68`) — **p5 `9,92 kΩ` / `0,6 V` · p8 `9,92` / `0,6` ·
>    p10 `9,93` / **`0 V`** · p12 `9,94` / **`0 V`**—, y el **paso 21 cableó p10 contra p11 en
>    normalmente abierto y funcionó, SIN demandas fantasma**. La placa soldada **sí** es la del
>    netlist. Lo confirma `Maestro/include/pines.h:123-138`, que lo dejó escrito el mismo día:
>    *«Ya se puede cablear camara a J16»*.
> 2. **`J16` p1 lleva 12 V CRUDOS** —sin opto, sin serie, sin clamp— a nueve posiciones de p10 y once
>    de p12. ~~**Se tapa físicamente antes de enchufar nada.**~~ → **04/09: tapar p1 es OBLIGATORIO en
>    cada equipo, no una precaución de banco.** El motivo del ascenso está en el bloque de abajo
>    (N-120): las entradas no tienen **nada** que limite lo que entra por ellas.
>    🔴 **ÉSTE SIGUE EN PIE Y NO SE RELAJA: es hoy el único requisito previo a cablear `J16`.**
>
> 🔴 ~~**Y lo que `M3` NO levanta, porque no era su sujeto:** `MANDO_A` (`PB9`, p5) y `MANDO_B`
> (`PB13`, p8) **siguen sin responder — `0,6 V` en reposo (N-118)**. Eso es **cobre, no firmware**.
> **El mando SE CONSERVA, va cableado y hoy no se usa.**~~
>
> 🛑 **TACHADO EL 05/09 CON SU MOTIVO, Y ES LA FRASE MÁS EQUIVOCADA QUE TENÍA ESTE DOCUMENTO: decía
> «cobre, no firmware» EN LA MISMA FRASE en que reconocía que el fuente ya estaba corregido.** Las
> dos mitades no podían ser ciertas a la vez, y la falsa es la primera. Se corrige por los dos lados:
>
> 1. **NO era cobre.** El propio paso 20 midió los cuatro pines: `9,92`–`9,94 kΩ` en los cuatro, y
>    **`0,6 V` sólo en los dos que llevaban `INPUT_PULLUP`** (p5, p8) contra **`0 V`** en los dos que
>    ya iban en `INPUT` pelado (p10, p12). **Mismo cobre, distinto `pinMode`, distinta tensión.** El
>    dato estaba en la misma tabla de arriba y nadie lo cruzó. Detalle en el bloque `N-118`.
> 2. **Y `M3` nunca podía «levantarlo», porque desde el 05/09 no hay sujeto:** `D-1` retiró el mando
>    del equipo. **`p5` y `p8` quedan libres y sin cablear.**
>
> 🛑 **Lo que SÍ se conserva de esa frase, y es lo único que importaba de ella: el CÓDIGO del mando
> no se toca.** Ver **§6**.
>
> ~~*(La salida de alarma de la AcuSense es configurable NO/NC, así que se elige qué estado significa
> demanda sin tocar placa ni firmware.)*~~
>
> 🔴 **TACHADO EL 05/09 — `SIN VERIFICAR`, y llevaba desde el 31/08 sosteniendo decisiones.**
> **Ninguna fuente oficial de este modelo lo dice.** Medido sobre los PDF de `04_Manuales/`: la ficha
> dice `1 output (max. 24VDC/24 VAC, 1 A)` **y nada más**; en las **110 páginas** del manual de
> usuario `Normally Open` / `Normally Closed` **no aparecen ni una vez**, y `Alarm Type` sólo está
> documentado para la **ENTRADA** *(pág. 44)*. Lo cierra el **`ENSAYO 1`** del **Manual 9 §6**, con un
> multímetro. **Hasta entonces no se da por elegida la polaridad de la salida.**

> ## 🔴 04/09 (N-120) — LA PLACA PROTEGE SUS SALIDAS Y **NO** PROTEGE SUS ENTRADAS
>
> **Leído del `.kicad_pcb`, red por red.** No es una impresión ni una lectura de esquemático: es el
> fichero de cobre.
>
> ### ~~Las 9 salidas van blindadas~~ → **Las DIEZ salidas llevan opto, y «blindadas» dice de más**
>
> 🔴 **Dos correcciones del 05/09, las dos medidas sobre el mismo fichero de cobre. Se tachan con su
> motivo porque una cifra que desaparece en silencio vuelve a escribirse:** (a) las cadenas de
> potencia son **DIEZ** —`Q1`–`Q10`, `U6`–`U15`—, no nueve; (b) *«blindadas»* es cierto para el **pin
> del micro** y falso para la **masa**. Censo completo, con los comandos, en **§11**.
>
> Cada una lleva **220 Ω en serie + optoacoplador `TLP127`**. Cadena completa de ejemplo:
>
> ```text
>    PA0  (/S1)  ->  R19  220 Ω  ->  U6  TLP127  ->  etapa de potencia
> ```
>
> ~~El opto **aísla galvánicamente**: pase lo que pase en el lado de potencia, no hay camino de
> corriente hacia el micro.~~
>
> 🔴 **TACHADO EL 05/09 CON SU MOTIVO — es MEDIO cierto, y la mitad que falla es la que decide qué
> se puede colgar de esas borneras.** Medido sobre el `.kicad_pcb`: **hay UNA sola red `GND` en toda
> la tarjeta, con 103 pads y un plano de cobre en las DOS capas** (`F.Cu` y `B.Cu`). Esa red incluye
> **el cátodo del LED del opto** (`U6` p3 … `U15` p3) **y la fuente de los diez MOSFET** (`Q1` p3 …
> `Q10` p3). El opto separa el **pin del micro** del nodo de puerta; **no crea una masa separada**.
>
> **Lo que sigue siendo cierto y no se toca:** no hay camino de corriente **desde el drenador hacia
> el pin del `U1`** — el mérito del diseño está ahí y por eso el opto vale. **Lo que deja de poderse
> decir:** que lo que se cuelgue de esas borneras esté aislado del controlador. **No lo está:
> comparte su masa.** Cuentas, comandos de medida y consecuencias en **§11**.
>
> ### Las 5 entradas de campo van DESNUDAS
>
> | entrada | llega desde | protección en serie |
> |---|---|---|
> | `PB0` | `J14.1` | 🔴 **NADA** |
> | `PB9` | `J16.5` | 🔴 **NADA** |
> | `PB13` | `J16.8` | 🔴 **NADA** |
> | `PB14` | `J16.10` | 🔴 **NADA** |
> | `PB15` | `J16.12` | 🔴 **NADA** |
>
> ⚠️ **El `10 kΩ` y el `100 nF` de esas entradas NO son protección: están en PARALELO a masa.** Fijan
> el **nivel de reposo** —que es lo que evita el pin flotante y las demandas fantasma— pero **no
> limitan corriente**. Un componente en paralelo no frena lo que entra por su mismo nudo.
>
> **Qué le hace esto a `M3`:** el `.kicad_pcb` dice que la resistencia a masa de `PB14`/`PB15`
> **existe en cobre**, que era la duda que `M3` tenía que despejar. **No la cierra** —esto sigue
> siendo lectura de fichero y `M3` es punta sobre la placa—, pero cambia lo que `M3` va a buscar: ya
> no *«¿hay pull-down?»* sino *«¿el que hay está bien puesto?»*.
>
> ### ⛔ Lo que obliga HOY, en cada equipo montado
>
> `J16` p1 lleva **12 V crudos** y está a **nueve posiciones de p10** y **once de p12**. Con la
> entrada desnuda, un hilo desplazado no da una lectura rara: **pone 12 V en una pata del STM32**.
>
> **Se tapa físicamente `J16` p1 antes de enchufar nada en ese conector. En todos los equipos, no
> solo en el de banco.**
>
> ### 🟡 La propuesta para la V2 — es una CUENTA, no una decisión tomada
>
> | | valor | contra qué se compara |
> |---|---|---|
> | Resistencia en serie propuesta | **`2K2` por entrada** | — |
> | Corriente de inyección resultante | **3,6 mA** | por debajo de los **5 mA** del datasheet ✅ |
> | Nivel alto que quedaría en el pin | **2,70 V** | por encima de los **2,31 V** de `VIH` ✅ |
> | Si se sube a **`4K7`** | — | 🛑 **ya no leería la cámara** |
>
> **Esto NO está decidido y este manual no lo decide.** Es la cuenta puesta delante de quien firme la
> placa, con las dos cotas —lo que hace falta para que siga leyendo la cámara y lo que hace falta
> para no pasarse del límite del micro— para que la decisión se tome con números y no con criterio.
> Mientras tanto, lo que protege es la cinta sobre p1.

> ## ✅ 31/08 — `PB8` NO ES, NI FUE NUNCA, UNA ENTRADA DE CÁMARA (N-64)
>
> Las ediciones anteriores hablaban de *«Cámara 2 / Cámara 4 de umbral en `PB8`»*. **Corregido:**
> `PB8` va por `R16` de 1 kΩ a un **LED testigo (`D5`)**. Se renombró a **`LED_TESTIGO`**
> (`pines.h:63`) y **el símbolo `CAM_UMBRAL_PIN` ya no existe en el fuente de ninguna de las dos
> puntas** — se comprobó con `grep`, no leyendo.
>
> Esto cierra de paso el hallazgo que circulaba como *«`CAM_UMBRAL_PIN` tiene `pinMode()` y ni un
> `digitalRead()`»*: **ya no hay `pinMode()` que sobre**, porque ya no hay pin de cámara ahí. Si algún
> documento sigue describiendo `PB8` como *«Umbral de tramo»* con función activa, **está caducado**.

* **Salida de Alarma de la Cámara:** Contacto seco libre de potencial (`1A` y `1B` del conector de alarma de la cámara Hikvision, ~~configurado en N/O a 1s~~ **con el pulso ajustado a `1 s` en el campo `Delay`**).
  > 🔴 **05/09: «configurado en N/O» se tacha — `SIN VERIFICAR` que la salida sea `NO`/`NC`
  > configurable en este modelo** *(ver el bloque de arriba y el `ENSAYO 1` del Manual 9 §6)*. El
  > **`Delay`** sí está documentado *(manual de usuario, pág. 68: «the time duration that the alarm
  > output remains after an alarm occurs»)*.
* **Analítica Embebida:** La cámara ejecuta internamente su algoritmo AcuSense (Detección de Intrusión con filtro `☑ Solo Vehículo`), ignorando peatones, ramas y sombras.
* **Seguridad Vial Inquebrantable:** Cada transición de sentido impone automáticamente el tiempo de **Despeje Todo-Rojo (`cfgDespejeSeg`)** configurado en pantalla antes de habilitar el verde con demanda.

---

## 8. 📱 Conexión del Módulo de Expansión — 🔴 REESCRITA EL 28/08 (`J17`) · 🟢 AMPLIADA EL 31/08

Desde el 28/08 esto **no es un accesorio de diagnóstico: es la interfaz del equipo.**

> ### 🟢 31/08 — el módulo es un **ESP32 de expansión**, y sustituye al SPP discreto
>
> **El cableado de `J17` que describe esta sección NO cambia.** Lo que cambia es qué se enchufa ahí.
>
> | | |
> |---|---|
> | **Chip** | **`ESP32-WROOM-32` clásico** — `Xtensa LX6` doble núcleo, **`Bluetooth v4.2 BR/EDR + BLE`**. `BR/EDR` es Bluetooth clásico: **hay SPP** y la app conecta sin tocar una línea |
> | **Qué aporta** | El **Bluetooth** (sustituye al módulo SPP discreto) y un **reloj `DS3231`** en `GPIO21` (`SDA`) / `GPIO22` (`SCL`), **con pila propia** |
> | **Firmware** | 🟢 **existe y compila**: `01_Firmware/ESP32_Expansion/`. La compuerta lo mide como suite propia (`compila esp32`); **la cifra de flash se lee del acta de `evidencia/`, no de este manual** |
> | 🛑 **Qué NO hace** | **No manda sobre las luces.** Es un puente: traduce y reenvía. La barrera de salidas sigue en `semaforo.cpp` del STM32 |
>
> **Consecuencia para el reloj, y es grande:** el `DS3231` del módulo **no cuelga de `PB0`/`PB8` del
> STM32** ni compite con las cámaras por pines. La disputa *«el reloj y las cámaras quieren los mismos
> dos pines»* que describían documentos anteriores **ya no existe**. Y no depende del cristal `Y2`,
> que está confirmado muerto en hardware (N-17).

> # ⛔ PARE. `J16` LLEVA 12 V Y QUEMA EL MÓDULO
>
> **Léase esto ANTES de tocar un solo cable.**
>
> La tarjeta tiene dos conectores de señal que se parecen y están cerca:
>
> | | qué es | **posición 1** |
> |---|---|---|
> | **`J16`** | conector de la **BOTONERA** | 🔴 **`12 V`** |
> | **`J17`** | ~~conector de la **PANTALLA**~~ → **el UART del módulo ESP32** — **el suyo** | `CS` (señal, no alimentación) |
>
> > ✅ **04/09 — la identidad de `J17` quedó RESUELTA en banco, y por eso se tacha lo anterior.**
> > `J17` **es el puerto serie del módulo de expansión ESP32**, no una pantalla. El netlist lo sigue
> > llamando *LCD* porque **quedó desactualizado el día que se retiró el display** (28/08) y nadie
> > volvió a tocarlo. El cableado de esta sección **no cambia** —siempre describió los pines
> > correctos—; lo que cambia es cómo se llama el conector cuando alguien lo busque en el netlist.
>
> **`J16` es el único conector de señal de esta tarjeta que trae 12 V.** El módulo Bluetooth es de
> **3,3 V**. Confundirlos **lo quema**, y no hay aviso previo: se enchufa, se energiza y se acabó.
>
> ### Cómo distinguirlos — se MIDE, no se mira
>
> Con el equipo energizado y el multímetro en tensión continua, **mida la posición 1 del conector
> contra GND**:
>
> - **≈ 12 V → es `J16`. NO es el suyo. Retire la punta y busque el otro conector.**
> - **No hay 12 V → puede ser `J17`.** Confirme entonces que las posiciones 6 y 8 dan **3,3 V**.
>
> ⚠️ **Y cuente los pines DESDE EL PIN 1, no desde el borde del conector.** El símbolo de `J17` en
> el esquema tiene **13 posiciones** y el footprint de la placa tiene **16**: si cuenta desde el
> borde, todo el mapa se desplaza tres posiciones. Lo mismo en `J16` (símbolo 12, footprint 16).
>
> 🔴 **04/09 — y hay un motivo nuevo para contar bien: en estos dos conectores el 3,3 V y la masa
> están ALTERNADOS.** Leído del netlist: `J16` saca **3,3 V en p4, p7, p9 y p11** con **GND en p2**;
> `J17` saca **3,3 V en p6 y p8** con **GND en p7 y p9**. **Un puente corrido UNA sola posición pone
> el riel de 3,3 V contra masa** — que es exactamente el candidato físico del corto de la tarjeta
> Maestro (**§9**). Si va a puentear algo aquí, cuente dos veces y mida antes de energizar.

### El cableado

> 🛑 **ESTE DIAGRAMA MANDABA ALIMENTAR EL MÓDULO DESDE `J17` p6, Y ESO ESTÁ PROHIBIDO — 05/09.**
> El Manual 1 §8 lo retiró el 31/08 y **este manual, que es el que tiene delante quien cablea, no
> se enteró.** Se corrige aquí y se deja el motivo, porque la consecuencia es vial.

```text
       MÓDULO ESP32 DE EXPANSIÓN                      TARJETA CONTROLADORA STM32
  ┌─────────────────────────────────┐              ┌───────────────────────────────┐
  │   [ VIN/5V ] ◄── DC-DC CONMUTADO desde 12 V     NO SE ALIMENTA DE LA TARJETA   │
  │   [ GND ] ──────────────────────┼──────────────┼──► J17 pos. 7   (GND) OBLIGATORIA
  │   [ TXD ] ──────────────────────┼──────────────┼──► J17 pos. 2 = PB7 (USART1 RX)│
  │   [ RXD ] ──────────────────────┼──────────────┼──► J17 pos. 3 = PB6 (USART1 TX)│
  └─────────────────────────────────┘              └───────────────────────────────┘
                                        (USART1 REMAPEADO — ver abajo)
```

| `J17` | red del esquema | pin del STM32 | al módulo |
|---|---|---|---|
| **2** | `RST` | **`PB7`** — `USART1_RX` | **`TXD`** |
| **3** | `RS(A0)` | **`PB6`** — `USART1_TX` | **`RXD`** |
| ~~**6**~~ | ~~`3,3 V`~~ | — | 🛑 ~~`VCC`~~ **NO SE CONECTA** |
| **7** | `GND` | — | `GND` — **masa común OBLIGATORIA** |

> 🔴 **POR QUÉ `p6` NO SE CONECTA, y por qué esto no es una cautela de banco.** **MEDIDO en el
> `.kicad_pcb`: `J17` p6 y p8 son `/3.3V`, y p7 y p9 son `GND`.** Ese riel de 3,3 V cuelga del
> **mismo `LM7805` que alimenta al STM32 que gobierna el semáforo**. Un `ESP32-WROOM-32` pide
> **picos de ~500 mA** al encender la radio: a esa corriente el 7805 disipa ~3,5 W sin disipador, y
> **si el riel se hunde un instante se reinicia el micro que mueve las luces**.
>
> **El síntoma en campo no se parece a un fallo de alimentación:** parece un cuelgue aleatorio del
> controlador. Por eso el accesorio de diagnóstico **no puede colgar del que gobierna el cruce**.
>
> 🛑 **Si ya se cableó un ESP32 a `J17` p6 siguiendo la versión anterior de este manual:
> DESCONECTE ESE HILO ANTES DE ENERGIZAR.**
>
> ⚠️ **Y lo que esto deja pendiente, dicho para que no se lea como resuelto:** el módulo necesita
> **su propia fuente DC-DC conmutada 12 V → 5 V (≥ 1 A)** — línea **`A5`** de compras, **que NO se
> ha pedido**. Sin ella no se monta el ESP32. La masa común de `p7` **sí** es obligatoria, y su
> medida (paso 23, `0 V` entre masas) es de las pocas de esa sesión que es un resultado y no un plan.

* **Es el `USART1` REMAPEADO, no un segundo puerto serie.** El STM32F103 permite sacar el `USART1`
  por `PB6`/`PB7` en lugar de `PA9`/`PA10`, **pero solo por un sitio a la vez**. El firmware ya está
  ahí: `static HardwareSerial SerialBT(PB7, PB6);` en `bluetooth.cpp`, **idéntico en las dos puntas**.
* **Por qué estos pines quedaron libres:** eran `LCD_PSB` (`PB6`) y `LCD_RST` (`PB7`) de la pantalla,
  y **ninguno de los dos llevaba datos** — uno era un nivel estático y el otro un pulso de arranque.
  Los tres hilos de datos del ST7920 (`SCLK`, `SID`, `CS`) no se tocan.
* **Baudrate:** 9600 bps, 8-N-1.

> ### `PA9`/`PA10`: válido eléctricamente, **NO es el montaje vigente**
>
> El puerto sigue existiendo en esos pines, pero **no salen a ninguna bornera de la tarjeta**. Para
> usarlos hay que **soldar** en las patas del MAX3485 `U2` o del propio micro. **Queda como
> alternativa de laboratorio, no como el cableado de campo.** Las ediciones anteriores de este
> manual daban `PA9` TX / `PA10` RX: era correcto para el montaje anterior y **ha quedado obsoleto,
> no se ha borrado.**

### 🔧 Corrección de chip: `U2` y `U3` estaban INVERTIDOS

> **Este manual decía `MAX3485 U3` donde va `U2`.** Se corrige dejando constancia, porque una
> corrección silenciosa se vuelve a proponer al mes siguiente.

Trazado red por red sobre el esquemático (`Controladora_Semaforos.kicad_sch`):

| chip | qué puerto | pines del micro | par A/B por |
|---|---|---|---|
| **`U2`** | MAX3485 del **`USART1`** | `RO`(1)→`PA10` · `~RE`(2) y `DE`(3)→`PA8` · `DI`(4)→`PA9` | **`J10`** |
| **`U3`** | MAX3485 del **`USART3`** — el de la **radio LoRa** | `PB11` · `PB12` · `PB10` | **`J12`** |

* **Desacoplo de Hardware:** el pin `PA8` (`RS485_IN_DE_RE`) se fija en `HIGH` permanente en el
  firmware. Eso apaga el receptor `RO` del **`U2`** (no del `U3`) y deja libre `PA10`.

> ⚠️ **El matiz que este manual no decía y sí importa: `PA8` gobierna A LA VEZ `~RE` (pin 2) y `DE`
> (pin 3) de `U2`.** Ponerlo en `HIGH` apaga el **receptor** —que es lo que se busca— pero **deja el
> transmisor ENCENDIDO**. Consecuencia: `U2` vuelca la telemetría por **`J10`** de forma permanente,
> y **esa línea no puede recibir nunca**.
>
> **Hoy es inofensivo porque `J10` está vacío.** Deja de serlo el día que alguien cuelgue algo de
> `J10` — y ese día **hay que tocar el código, no el cableado**. Es la lección del repetidor del
> 31/07/2026, ya escrita en `01_Firmware/TROUBLESHOOTING.md`: **un `DE`/`RE` clavado en alto bloquea
> la línea en AMBOS sentidos.**
>
> Nótese además que este desacoplo **ya no es estrictamente necesario para el Bluetooth**, puesto que
> el módulo se ha mudado a `PB6`/`PB7`. **El firmware lo sigue haciendo**, y se documenta tal como
> está en lugar de describir lo que sería razonable.

### Telemetría, caja negra y seguridad

* **Telemetría:** emisión ~~cada 1 s~~ **cada 2 s** de *(cadencia bajada a **2000 ms** el 04/09, decision del responsable, en las DOS puntas — MEDIDO: `Maestro/src/bluetooth.cpp:851`, `Esclavo/src/bluetooth.cpp:768`. Un tecnico que cronometre con «1 segundo» declara caido un enlace sano.)* 
  `$STATUS,NODE:...,SERIE:...,MODO:...,ESTADO:...,T:...,RF:...%,RTT:...ms,BAT:...,HORA:...*XX\r\n`.
* **Caja Negra:** registro instantáneo de caídas de radio con hora del RTC
  (`$ALARM,NODE:...,EVENTO:FALLO_RF,CAUSA:SILENCIO_25000ms,ACCION:CAMBIO_A_AMBAR,HORA:...*XX\r\n`).
  **Censado el 28/08: tiene llamadores reales** — `coordinador.cpp` líneas 683 y 775 en el Maestro,
  `main.cpp` línea 560 en el Esclavo. *(No siempre fue así: la función estuvo documentada en cuatro
  manuales y sin un solo llamador — N-73.)*
* **Seguridad:** los comandos de control exigen PIN de 4 dígitos (`CMD:PIN:1234:...`), **salvo la
  caída segura, que se acepta SIN PIN a propósito**: detener el tráfico no debe depender de recordar
  un código.

  > 🛑 **31/08 — Y EL LITERAL NO ES EL MISMO EN LAS DOS PUNTAS. ES EL BOTÓN DE PÁNICO.**
  >
  > | Punta | Comando sin PIN | Qué hace | Medido en |
  > |---|---|---|---|
  > | **Maestro** | `CMD:FORZAR_ROJO` | **rojo total** | `Maestro/src/bluetooth.cpp:145` |
  > | **Esclavo** | **`CMD:AMBAR_EMERGENCIA`** | **ámbar intermitente** + latch que **veta** las órdenes de radio | `Esclavo/src/bluetooth.cpp:130`, `:171`, `:268` |
  >
  > **En el Esclavo, `FORZAR_ROJO` está RENOMBRADO y NO HACE NADA.** Las dos formas —con PIN y sin
  > PIN— contestan `$ERR,CMD:FORZAR_ROJO,DESC:RENOMBRADO_USE_AMBAR_EMERGENCIA` (`:157`, `:176`).
  > **Un operario que mande el literal viejo creerá haber detenido el cruce sin haber detenido nada.**
  > El nombre viejo prometía rojo y hacía ámbar: se corrigió el nombre, **no el comportamiento**.
  >
  > Que las dos puntas usen literales distintos es lo correcto, **porque hacen cosas distintas**;
  > llamarlas igual era el defecto. Tabla completa de comandos en
  > `04_Manuales/MANUAL_CONFIGURACION_BLUETOOTH.md §4.4` — **17 formas en el Maestro**, no 5 ni 9.

> 🔴 **AVISO SOBRE TRES CAMPOS DE `$STATUS` QUE NO SON MEDIDAS.** Medido sobre `bluetooth.cpp` el
> 28/08:
>
> | campo | Maestro | Esclavo |
> |---|---|---|
> | `RF:` | ✅ real (SFTY-14) | 🔴 **literal `98%`** en el `snprintf` (línea 215) |
> | `RTT:` | ✅ real | 🔴 **literal `85ms`** |
> | `BAT:` | 🔴 **literal `12.6`** | 🔴 **literal `12.6`** |
> | `T:` | ⚠️ `(millis()/1000) % 60` — contador libre 0–59, **no** la cuenta regresiva de la fase | ⚠️ igual |
>
> **Un tablero que rellena con constantes el dato que no tiene miente a quien decide mirándolo.**
> El `RF:98%` del Esclavo se emite igual con la antena desconectada. **No use esos campos para
> juzgar el enlace del Esclavo ni la batería de ninguna de las dos puntas**, y no los apunte en un
> acta como si fueran medidas.

---

## 9. 🩺 EL CORTO DE 3,3 V DE LA TARJETA MAESTRO (N-116) — cómo se diagnostica

> # ⛔ NO REENERGICE LA TARJETA «A VER SI PASA»
>
> **Síntoma medido el 03–04/09, con la tarjeta en la mano:** alimentada, la Maestro **funciona unos
> 30 s**, **se calienta** y **se para**. Hay **continuidad medida entre el riel de 3,3 V y masa**.
>
> Cada reenergizada mete más energía en el punto que ya se está calentando. Un corto que hoy es un
> componente se vuelve tres si se insiste. **La tarjeta se diagnostica en frío.**

**Lo que ya está descartado, para que nadie lo vuelva a intentar:**

* 🛑 **El firmware. Descartado por censo.** Un corto entre riel y masa es cobre o componente: **ningún
  binario lo provoca y ninguno lo arregla.** No gaste una carga en esto.
* 🛑 **Cambiar piezas por sospecha.** Este proyecto ya pagó una vez por ahí —se mandó a cambiar `Y2`,
  la pila y `R5` sin haber medido ninguno de los tres—. Primero la medida, después la pieza.

### La escalera de diagnóstico — de lo gratis a lo caro. No se salta un peldaño

#### Peldaño a) — **gratis, y es el primero:** desenchufe los cinco conectores

**`J14`, `J15`, `J16`, `J17` y `J2`.** Los cinco **sacan el riel de 3,3 V fuera de la placa**. Con la
tarjeta **sin alimentación**, vuelva a medir continuidad entre 3,3 V y masa:

| resultado | qué significa | qué se hace |
|---|---|---|
| **El corto DESAPARECE** | 🟢 **la placa está bien** | el corto está en lo que colgaba de esos conectores — revíselo antes de volver a enchufar |
| **El corto SIGUE** | 🔴 está **dentro** de la placa | pase al peldaño b) |

#### Peldaño b) — si sigue, se sustituye **por orden de coste**, no por corazonada

| orden | qué | referencias |
|---|---|---|
| 1 | cerámicos de desacoplo de 100 nF | `C1` `C2` `C3` `C4` `C10` `C11` |
| 2 | condensador de 10 µF | `C15` |
| 3 | regulador de 3,3 V | `U5` (`LM1117DT-3.3`) |
| 4 | transceptores RS485 | `U2` / `U3` (`MAX3485`) |
| 5 | 🛑 **el STM32 — EL ÚLTIMO** | — |

**El orden no es capricho:** un cerámico en corto es lo más frecuente y lo más barato de sustituir; el
micro es lo más caro, lo que peor se desuelda y lo que menos suele fallar. Empezar por el final es
tirar la tarjeta para arreglar un condensador de céntimos.

#### Peldaño c) — discriminar **SIN DESOLDAR**

**Inyecte 3,3 V con una fuente limitada a ~200 mA** en el riel y **busque qué componente calienta**.
El que conduce es el que se calienta, y así se localiza la pieza antes de tocar el soldador.

⚠️ **La limitación de corriente no es opcional:** es lo único que impide que el diagnóstico se
convierta en la segunda avería.

### 🔎 El candidato físico, leído del netlist

| conector | 3,3 V en | GND en |
|---|---|---|
| **`J16`** | p4, p7, p9, p11 | p2 |
| **`J17`** | p6, p8 | p7, p9 — **alternados** |

En el **paso 29** de la guía de banco se puenteaban `J16` **p5** y **p8** contra masa (**p2**). Y
**`p4` es ADYACENTE a `p5`**, igual que **`p7` lo es de `p8`**: **un puente corrido UNA sola posición
pone el riel de 3,3 V directamente contra masa.**

> **Esto es un candidato, no un veredicto.** No demuestra que fuera lo que ocurrió — el peldaño a) lo
> confirma o lo descarta en un minuto y sin desoldar nada. Se escribe aquí por dos razones: para que
> quien repita el paso 29 lo haga sabiéndolo, y para que la hipótesis quede anotada y no se vuelva a
> proponer dentro de un mes como si fuera nueva.

> 🔧 **04/09 (N-118) — y el gesto nuevo ya no tenía ese extremo.** Con el firmware corregido el pulso
> se daba **p5 contra p4** y **p8 contra p7**: **ninguna de las dos puntas del cable iba a masa**, así
> que el escenario de arriba —un puente corrido una posición desde p2— dejaba de ser posible con la
> instrucción vigente.
>
> ⛔ **05/09 — Y AHORA NO HAY NINGÚN GESTO, QUE ES MEJOR TODAVÍA PARA ESTA HIPÓTESIS.** `D-1` retiró
> el mando: **`J16` p5 y p8 quedan libres y nadie tiene motivo para acercarles un cable.** El paso 29
> **no se repite**. Eso no despeja la hipótesis de este apartado —sigue siendo el candidato de por
> qué se calentó la Maestro aquel día, y el peldaño (a) sigue siendo la forma de confirmarla o
> descartarla sin desoldar—, pero **sí garantiza que no se vuelve a provocar**.
>
> 🛑 **Y no es una autorización para reenergizar nada:** la tarjeta Maestro sigue con el corto de
> N-116 y **no se energiza**.

---

## 10. 🚧 La talanquera de `J15` — **probada en cobre y BIEN DISEÑADA**

> **Se sospechó de ella y la sospecha era FALSA.** Se deja escrito precisamente por eso: una sospecha
> que se cae en silencio vuelve a proponerse al mes siguiente, y la segunda vez ya nadie recuerda que
> se comprobó.

Cadena completa, leída del `.kicad_pcb`:

```text
   PB2  ->  R70 220 Ω  ->  U15  TLP127  ->  R72 220 Ω  ->  puerta de Q10 (IRLZ44N)  ->  J15
                     [ separa el PIN, NO la MASA ]                D30 al riel de 12 V
                                                          R73 1K + D29 LED al riel de 12 V
```

* ~~**`U15` es un `TLP127`: aísla galvánicamente.** La etapa de potencia **no puede inyectar corriente
  al micro**, pase lo que pase del lado del motor.~~ 🔴 **TACHADO EL 05/09: medio cierto.** Lo que
  `U15` separa es **el pin del `U1` del nodo de puerta de `Q10`**, y eso sí protege al micro. Lo que
  **no** hace es crear una masa aparte: **hay una sola red `GND` en la tarjeta**, y en ella están el
  cátodo del LED de `U15` y la fuente de `Q10`. **Un motor colgado de `J15` comparte la masa del
  controlador.** Ver **§11**.
* 🔴 **Y `J15` p2 NO está a 0 V en reposo: está a ~12 V.** Falta en este diagrama, y estaba en el
  cobre desde el primer día: `R73` 1 kΩ + `D29` (LED) van del riel de 12 V al drenador de `Q10`. Es
  lo que explica la medida de banco *«en rojo `0 V`, en ámbar `12 V`»* que hasta hoy estaba anotada
  **sin causa**. Ver **§11**.
* **Aquí la barrera hacia el pin del micro existe y está bien puesta** — es exactamente lo contrario
  de lo que ocurre con las cinco entradas de campo (**§7**).
* **En banco, el 04/09, la talanquera funcionó por `J15`.**

### ⚠️ Pero hay un hallazgo real, y va a la V2: `D30` está infradimensionado

**`D30` es un `1N4148` (200 mA)** haciendo de **diodo de rueda libre** de una salida de **motor**
gobernada por un **`IRLZ44N`**. Un motor real devuelve al abrir bastante más de 200 mA por esa vía.

| | |
|---|---|
| Lo que **NO** pasa | 🟢 **el pin del STM32 no corre peligro** — `U15` separa la pata del micro del nodo de puerta. Ese es el mérito del diseño. 🔴 **Lo que sí llega igual: la masa.** Ver §11 |
| Lo que **SÍ** pasa | 🔴 el retorno inductivo se lleva **`D30`**, y detrás **`Q10`** |

**Va a la V2 como cambio de componente**, no como parche de campo: es un diodo, no una decisión de
arquitectura. Mientras tanto la talanquera funciona — lo que no conviene es darla por eterna en cuanto
se le cuelgue un motor grande.

---

## 11. 🧭 EL CENSO DE COBRE DEL 05/09 — lo que las diez borneras de potencia hacen de verdad

> **Todo este apartado sale de UN fichero: `01_Firmware/Controladora_Semaforos/Controladora_Semaforos/Controladora_Semaforos.kicad_pcb`
> (2.158.421 B) y de su `.kicad_sch`.** Es cobre, no esquema de bloques. Los comandos van pegados
> para que cualquiera los repita, y **cada uno se corrió antes de escribirlo**.

### 11.0 ⚠️ Antes de nada: el buscador de este fichero engaña, y ya publicó una mentira

KiCad separa sus tokens con **tabulador y salto de línea**, no con espacio. Buscar la pista con un
espacio detrás da **cero**, y un cero se lee como *«no hay»*:

```text
$ grep -c "(segment " Controladora_Semaforos.kicad_pcb
0                                    <-- FALSO
$ grep -oE "\(segment\b" Controladora_Semaforos.kicad_pcb | wc -l
1447                                 <-- REAL
```

Sobre ese cero se llegó a publicar que el fichero de cobre estaba **VACÍO**. No lo está: **185
huellas, 1.447 pistas, 89 vías, 485 pads y 117 redes**, más un plano de masa. **Quien repita este
censo usa `\b`, nunca un espacio** — y si le sale un cero, mide el fichero por otro camino antes de
escribir que algo falta.

### 11.1 Las DIEZ cadenas de potencia, una por fila

Todas son **el mismo molde**: pin del `U1` → `R` 220 Ω → opto `TLP127` → `R` 10 K a masa + `R`
220 Ω a la puerta → MOSFET `IRLZ44N` de lado bajo → bornera, con `1N4148` de rueda libre al riel de
12 V.

| MOSFET | bornera | pin U1 | GPIO | red | opto | firmware que lo mueve |
|---|---|---|---|---|---|---|
| `Q1` | `J3` | 10 | `PA0` | `/S1` | `U6` | ✅ `ROJO1` |
| `Q2` | `J4` | 11 | `PA1` | `/S2` | `U7` | ✅ `AMARILLO1` |
| `Q3` | `J5` | 12 | `PA2` | `/S3` | `U8` | ✅ `VERDE1` |
| `Q4` | `J6` | 13 | `PA3` | `/S4` | `U9` | ✅ `ROJO2` |
| `Q5` | `J7` | 14 | `PA4` | `/S5` | `U10` | ✅ `AMARILLO2` |
| `Q6` | `J8` | 15 | `PA5` | `/S6` | `U11` | ✅ `VERDE2` |
| `Q7` | `J9` | 17 | `PA7` | `/S8` | `U12` | 🔴 **NADA** — `VERDE_PEATON` |
| `Q9` | `J11` | 16 | `PA6` | `/S7` | `U14` | 🔴 **NADA** — `ROJO_PEATON` |
| `Q8` | `J13` | 19 | `PB1` | `/Buzzer` | `U13` | 🔴 **NADA** — `BUZZER` |
| `Q10` | `J15` | 20 | `PB2` | `/Motor` | `U15` | ✅ `MOTOR_TALANQUERA`, probada en banco el 04/09 |

**Son diez, no nueve.** El *«las 9 salidas»* de §7 y de N-120 se corrige aquí; se tacha allí con su
motivo y no se borra. Los símbolos del firmware están en `Maestro/include/pines.h` y la numeración de
pines del `U1` sale del propio `.kicad_sch` (`STM32F103C8Tx`, LQFP48).

### 11.2 🔴 EL BORNE NO ESTÁ A 0 V EN REPOSO: ESTÁ A ~12 V

**Éste es el hallazgo que cambia lo que se puede enchufar ahí, y no estaba en ningún documento.**

En **nueve** de los diez drenadores hay, **en el cobre**, un **pull-up de 1 kΩ + LED al riel de
12 V**: `R23`, `R28`, `R33`, `R38`, `R43`, `R48`, `R53`, `R58`, `R63`, `R73`, cada uno con su LED
(`D12`, `D13`, `D15`, `D17`, `D19`, `D21`, `D23`, `D25`, `D27`, `D29`).

```text
   +12 V ---[ R 1K ]---|>|--- DRENADOR ------ bornera p2
                      LED         |
                                 [Q] IRLZ44N
                                  |
                                 GND
```

* **MOSFET conduciendo** (el firmware escribe el pin ALTO): el borne cae a **~0 V** y el LED luce.
* **MOSFET abierto** (reposo, o firmware sin escribir el pin): el borne **sube a ~12 V**.

**Esto no se evita dejando un hilo sin poner: está en la placa, no en el conector.**

**La corriente disponible es una cuenta, no una medida:** `(12 V menos Vf del LED, ~2 V) / 1 kΩ` da
**~10 mA**. **La tensión en vacío y esos 10 mA quedan `SIN VERIFICAR` con multímetro** — se miden
entre p2 y masa con la bornera desconectada, y son dos minutos de banco.

> ✅ **Y explica una medida de banco que llevaba desde el 04/09 anotada SIN CAUSA.** El informe
> apuntó de `J15`: *«en rojo `0 V`, en ámbar `12 V`»*. Con la sonda entre **p1 y p2** eso es
> exactamente este circuito: en rojo el firmware cierra la pluma, el pin va BAJO, el MOSFET se abre,
> el drenador sube a 12 V por `R73` y la diferencia p1-p2 se queda en **~0 V**; en ámbar el pin va
> ALTO, el MOSFET conduce, el drenador cae y esa diferencia pasa a **~12 V**. *(La polaridad del
> firmware está en `Maestro/include/pines.h`, `TALANQUERA_ABRIR = HIGH`. **Que la sonda estuviera
> entre p1 y p2 es DEDUCCIÓN de esas dos cosas, no una lectura del informe: `SIN VERIFICAR`.**)*

#### 🔴 Y la excepción, que es un hallazgo nuevo: `J8` (`VERDE2`) NO tiene ese pull-up

`D21` —el LED del canal de `Q6`/`J8`— tiene el **cátodo sin conectar**, en el esquemático y en el
cobre:

```text
$ grep -oE '\(net [0-9]+ "unconnected-\(D21-K-Pad1\)"\)' Controladora_Semaforos.kicad_pcb
(net 47 "unconnected-(D21-K-Pad1)")
$ grep -oE '\(net [0-9]+ "Net-\(D23-K\)"\)' Controladora_Semaforos.kicad_pcb
(net 49 "Net-(D23-K)")            <-- D23, el LED gemelo de J9, SI llega al drenador
```

Y hay **cero pistas** sobre esa red (`net 47`) y **cero hilos** en el `.kicad_sch` en ese pin,
mientras su gemelo `D23` los tiene en los dos extremos. `R48` y `D21` **están montados** y forman un
muñón colgado del riel de 12 V que **no llega a ninguna parte**.

**Consecuencia práctica, y es la única fila de la tabla de 11.1 que se comporta distinta:** con el
MOSFET abierto, **`J8` p2 queda FLOTANDO**, no a 12 V. Y `J8` es `VERDE2` — una de las seis luces
vivas.

> ⚠️ **Lo que este documento NO dice: si esto es un defecto o una decisión.** No hay ninguna nota en
> el repositorio que lo mencione. **`SIN VERIFICAR`** que la placa soldada coincida con el fichero en
> este punto: se comprueba con un multímetro entre `J8` p1 y p2, comparando contra `J7` p1-p2 en el
> mismo estado de luz. **Dos minutos, y es la medida que lo cierra.**

### 11.3 🔴 Un MOSFET a masa NO es un contacto seco

Se escribe aquí porque es la conclusión de obra de los dos apartados anteriores, y porque el
vocabulario de este proyecto usa *«contacto seco»* todo el rato — con razón — **para las ENTRADAS de
cámara**, que sí lo son. **Las salidas no.**

| lo que un contacto seco garantiza | lo que estas borneras dan |
|---|---|
| dos terminales sin referencia a nada | p1 es el **riel de 12 V** de la tarjeta y p2 cuelga del **drenador** |
| ninguna tensión propia | **~12 V en reposo** con ~10 mA disponibles, en nueve de los diez |
| masa independiente del que manda | **la MISMA masa del controlador** — ver 11.4 |
| se puede invertir | **no**: el MOSFET conduce en un solo sentido, y es de lado bajo |

**Ni siquiera es un colector abierto limpio**, porque el pull-up le pone tensión propia. Lo que hay
es una **salida conmutada a masa, con 12 V de reposo y masa común**.

### 11.4 🔴 «El opto aísla galvánicamente» — la mitad que falla

Medido: **hay UNA sola red `GND` en toda la tarjeta**, y ninguna otra que se le parezca.

```text
$ grep -oE '\(net [0-9]+ "[^"]*GND[^"]*"\)' Controladora_Semaforos.kicad_pcb | sort -u
(net 1 "GND")
```

Esa red tiene **103 pads** y un **plano de cobre en las dos capas** (`F.Cu` y `B.Cu`). Dentro están,
a la vez:

* el **cátodo del LED de cada opto** (`U6` p3 hasta `U15` p3) — o sea, el lado que mira al micro;
* la **fuente de cada MOSFET** (`Q1` p3 hasta `Q10` p3) — o sea, el lado de potencia;
* las cuatro patas `VSS` del `U1`, y `J16` p2 y `J17` p7 y p9.

**Qué queda en pie y qué se cae:**

| | |
|---|---|
| ✅ **sigue siendo cierto** | no hay camino de corriente **del drenador a la pata del `U1`**. Ése es el mérito del diseño, y por eso `J15` es *«bien diseñada»* frente a las cinco entradas desnudas de §7 |
| 🔴 **deja de poderse decir** | que lo colgado de esas borneras esté **aislado del controlador**. Comparte su masa, y comparte también su riel de 12 V por p1 |

**En obra esto se traduce en una sola frase:** un retorno de masa sucio de un motor, de una cabeza
peatonal o de un zumbador **entra en la masa del STM32**, aunque el opto esté ahí. El opto protege
al **pin**; no aísla al **equipo**.

### 11.5 Tres canales de potencia completos, fabricados y sin una línea de firmware detrás

`J9` (`VERDE_PEATON`, `PA7`), `J11` (`ROJO_PEATON`, `PA6`) y `J13` (`BUZZER`, `PB1`). Están enteros
en la placa —opto, MOSFET, diodo, LED, bornera— y **el firmware no los toca en ninguna de las dos
puntas**:

```text
$ grep -rn -e ROJO_PEATON -e VERDE_PEATON -e BUZZER Maestro/src Esclavo/src
Maestro/src/main.cpp:35://   ROJO_PEATON y VERDE_PEATON, que estaban sin custodia.

$ grep -rn -e pinMode -e digitalWrite -e digitalRead Maestro/src Esclavo/src \
      | grep -e PEATON -e BUZZER
(sin salida)
```

Ni un `pinMode`, ni un `digitalWrite`. **Si alguien cablea una cabeza peatonal a `J9`/`J11` o un
zumbador a `J13`, no se enciende nunca y no hay mensaje de error.**

**Cuánto cuesta darle vida a uno — medido, no estimado.** Desensamblando el `.elf` construido, un
`pinMode(PIN, OUTPUT)` con pin constante y un `digitalWrite(PIN, valor)` ocupan **8 bytes cada uno**
en Thumb-2:

```text
$ arm-none-eabi-objdump -d Maestro/.pio/build/maestro/firmware.elf
  (dentro del simbolo _Z14semaforo_setupv)
    2101      movs r1, #1            <-- 2 B
    2012      movs r0, #18           <-- 2 B   (MOTOR_TALANQUERA)
    f002 f979 bl   pinMode           <-- 4 B
  (dentro del simbolo _ZL13escribirPinesbbb)
    4629      mov  r1, r5            <-- 2 B
    20c4      movs r0, #196          <-- 2 B
    f002 faae bl   digitalWrite      <-- 4 B
```

O sea **16 B** por canal encendido. ⚠️ **Ese 16 es un SUELO, no el coste de la función.** Es lo que
cuestan las **dos llamadas**; no incluye ni un byte de lo que **decide** el valor —una fase peatonal,
un temporizador, un patrón de zumbido—. **Publicar el 16 sin esta frase sería vender barato algo que
todavía no se ha diseñado.** Y el margen de flash es el que es: el acta de `compuerta.py` del
05/09 deja al Maestro en **86,5 %** (`56656` de `65536` B). **Ese número se lee del acta de
`evidencia/`, no de aquí**: cambia con cada corrida, y una cifra copiada a mano en un manual caduca
sola.

> 🟡 **`SIN VERIFICAR`: que `J9`, `J11` y `J13` estén REALMENTE SOLDADOS en la tarjeta física.**
> El esquemático los marca `in_bom=yes`, `dnp=no`, `on_board=yes` —o sea que el diseño dice que van—,
> pero **nadie los ha mirado en cobre**. Lo más cerca que hay es **`J15`, el gemelo exacto, que sí
> funcionó en banco el 04/09**. Eso hace probable que estén; **no lo demuestra**. Se comprueba a ojo
> y con continuidad, y es lo primero antes de contar con ellos para nada.

> 🟡 **Y lo que este manual NO decide, porque no le toca:** gastar uno de esos tres canales **cierra
> la puerta a una cabeza peatonal o a un zumbador en esta placa** — no hay más molde libre. **No
> existe ninguna decisión escrita que renuncie a ellos.** Queda como pregunta abierta, con dueño, en
> `05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md` §3.10.

### 11.6 Los pines libres son SEIS, no tres — y tres de ellos ya tienen bornera

A `ROJO_PEATON` (`PA6`), `VERDE_PEATON` (`PA7`) y `BUZZER` (`PB1`) hay que sumar **`PB3`, `PB4` y
`PB5`**, los de la pantalla. `lcd.cpp` los pasó a `U8X8_PIN_NONE` cuando el volcado al cable se
retiró, y la librería **se salta el `pinMode` y el `digitalWrite` cuando el pin es `NONE`** — está
leído en `U8x8lib.cpp`, función `u8x8_gpio_and_delay_arduino()`, donde los dos caminos preguntan
`if (u8x8->pins[i] != U8X8_PIN_NONE)` y `if (i != U8X8_PIN_NONE)` antes de tocar nada.

**Quedan en alta impedancia, y con pista hasta una bornera ya montada:**

| GPIO | símbolo | bornera | red del `.kicad_pcb` |
|---|---|---|---|
| `PB3` | `LCD_SCLK` | `J17` p4 | `/SCL` |
| `PB4` | `LCD_CS` | `J17` p1 | `/CS` |
| `PB5` | `LCD_SID` | `J17` p5 | `/SI` |

**Son los únicos GPIO libres del proyecto con bornera ya cableada.** `J9`/`J11`/`J13` traen etapa de
potencia pero no dan una entrada; éstos dan pin desnudo con conector.

> ⚠️ **Lo que cuesta usarlos, y lo que NO cuesta.**
>
> **No cuesta el depurador.** `PB3` y `PB4` son patas de **JTAG** (`JTDO` y `NJTRST`), pero el
> framework las libera conservando SWD: `pin_function()` llama a `pin_DisconnectDebug()`, y en
> `PinAF_STM32F1.h`, función `pinF1_DisconnectDebug()`, eso hace `__HAL_AFIO_REMAP_SWJ_NOJTAG()` para
> `PA_15`, `PB_3` y `PB_4` — *«JTAG-DP Disabled and SW-DP enabled»*, literal del fuente. **La carga
> por SWD sigue funcionando.** *(`PB5` **no** es pin de JTAG: no aparece en esa lista. Donde se haya
> escrito que los tres lo son, es falso.)*
>
> **Sí cuesta la pantalla.** Devolverlos al LCD exige **dos** cosas, no una: reponer los pines en
> `lcd.cpp` **y** sacar el ESP32 de `J17`. Y `J17` p1, p4 y p5 son posiciones **del mismo conector en
> el que vive el ESP32**, así que lo que se cuelgue de ellas convive con él. **Que eso no le moleste
> al módulo NO está medido: `SIN VERIFICAR`.**

### 11.7 Cómo repetir este censo entero

```text
cd 01_Firmware/Controladora_Semaforos/Controladora_Semaforos
wc -c Controladora_Semaforos.kicad_pcb                                # 2158421
grep -oE '\(footprint\b' Controladora_Semaforos.kicad_pcb | wc -l     # 185
grep -oE '\(segment\b'   Controladora_Semaforos.kicad_pcb | wc -l     # 1447
grep -oE '\(via\b'       Controladora_Semaforos.kicad_pcb | wc -l     # 89
grep -oE '\(pad\b'       Controladora_Semaforos.kicad_pcb | wc -l     # 485
grep -oE '\(net [0-9]+ "[^"]*"\)' Controladora_Semaforos.kicad_pcb | sort -u | wc -l   # 117
```

Para las redes por componente hace falta **emparejar paréntesis** (el fichero anida `pad` dentro de
`footprint`); un `grep` de línea suelta **no basta, y da resultados que parecen buenos**. Es la misma
trampa de 11.0, una capa más adentro.
