# 17 — Arquitectura decidida en obra el 28/08/2026, y lo que sigue abierto

**Para:** el funcional y el auditor. **Fecha del documento:** 28 de agosto de 2026; vivo y en uso.

**Esta es la SPEC DE COBRE.** Gana a `DECISIONES.md` y a `CLAUDE.md` en todo lo que sea **hardware
medido** (`CLAUDE.md` §12) — y **no gana en nada mas**: lo decidido lo fija `DECISIONES.md`.

> 🔴 **TOPE: 1.000 LINEAS. HOY NO SE CUMPLE, y se dice aqui para que nadie lo lea como cumplido —
> `wc -l` da la cuenta de verdad.** El motivo, en dos lineas: **un fichero que no se puede leer
> entero no lo lee nadie entero, y entonces cada lector deriva su propia version.** Eso es lo que
> paso aqui: del 28/08 al 12/09 esto llego a **4.182 lineas** y ya solo se citaban trozos.
>
> **Lo que salio NO se ha borrado ni se ha resumido: esta INTEGRO Y LITERAL en
> [`17_hist_Arquitectura.md`](17_hist_Arquitectura.md)**, que publica al final las dos cuentas para
> demostrarlo. Salio **cronica**, **decisiones ya cerradas** y **medidas derogadas** —que se mudan,
> no se borran (`CLAUDE.md` §7.4)—. Se queda **lo que alguien necesita para no equivocarse HOY**.
>
> 🔴 **Y por que se pasa del tope, dicho en vez de disimulado: lo unico que queda por sacar son las
> cuatro decisiones ABIERTAS (§3.6, §3.8, §3.9, §3.10), el censo de cobre de §1.2, las recetas de §A
> y los `SIN VERIFICAR` de §C.** Nada de eso es cronica, y **perder una fila abierta cuesta mucho
> mas que pasarse.**

> 🛑 **LOS TRES AVISOS QUE SE LEEN CON UN DESTORNILLADOR EN LA MANO. Esto es un indice, no la
> medida: cada uno tiene su numero y su fecha en el apartado que se cita.**
>
> 1. **`J16` p1 lleva 12 V CRUDOS** a un conector de senal que va **directo** a la pata del STM32.
>    **Taparlo es OBLIGATORIO en cada equipo que se monte** —no es una cautela de banco— (`D-4`,
>    N-120). Medida y motivo: **§2.1**, **N-120** y la tabla de **§1.7**.
> 2. **`J14` es una ENTRADA del micro** (`PB0`, 3,3 V, sin opto y sin diodo); **la salida de
>    talanquera es `J15`**. Hoy `J14` va **LIBRE y sin cablear** (`D-27`, 11/09) y **mientras el
>    firmware lea `PB0` como demanda ahi no se conecta nada**: **§1.2**, **§1.7.bis**, `D-27`.
> 3. **`J16` p5 y p8 estan VACIOS y el firmware SIGUE leyendo sus flancos** (`A-2`, `D-1`): lo que
>    se cierre ahi contra los 3,3 V del borne contiguo **compone secuencias del mando**, y `A.A.A`
>    entra al Modo Automatico **sin guarda**. **§1.6** y **§1.7**.

> **Como se cita el fuente en este documento: por SIMBOLO y con el `grep` que lo encuentra, nunca
> por numero de linea** —un numero caduca solo, en silencio y con autoridad de dato—. El censo que
> obligo a cambiarlo, y las citas que se cayeron con el, en el historico.

Este documento esta escrito **en ASCII sin acentos**, como el resto de lo que se parsea o se lee en
consola de Windows en este repositorio.

> **Este documento no cambia ningun otro fichero.** Todo lo que otro documento necesita corregir
> esta listado en la seccion B y en el anexo final, y **no se ha tocado**. Habia otros trabajos en
> vuelo sobre el mismo arbol el dia que se escribio.

---

## ✏️ REVISION DEL 11/09/2026 (2.a) — `D-27`: `J14` LIBRE, CUATRO CAMARAS COMPRADAS (solo §1.7.bis y §3.5)

**El responsable, 11/09 (`DECISIONES.md` `D-27`), sobre `D-25`:** (1) **las CUATRO camaras estan
compradas** y el modelo es el de `D-10` (`DS-2CD2683G2-IZS`); (2) **`J14` queda LIBRE, sin
cablear: el fin de carrera NO se instala en este despliegue.** Eso **cierra el conflicto (1) de la
revision de abajo** sin tocar el firmware: el firmware sigue leyendo `PB0` como `CAM_DEMANDA_PIN`,
y con `J14` vacio `R64` (10 kOhm a masa) deja el pin en 0 V —`J14` medido en cobre en los pasos
17-18 del banco del 03-04/09—, asi que no pide nada. **Mientras el firmware lea `PB0` como
demanda, en `J14` no se conecta nada.** (3) La configuracion de cada camara es la del manual del
modelo (`04_Manuales/MANUAL_CONFIGURACION_CAMARAS_IA.md`); este documento no recita valores de
configuracion, asi que no cambia nada aqui por ello. (4) Talanquera por rele a `OPEN` de la
centralita, como en la guia del Sisga — igual que ya decia la revision de abajo.

**Lo que NO cierra `D-27`:** el conflicto (2) de abajo —`D-25` *«las camaras no tocan el ciclo»*
contra el Modo Inteligente, que **si** alarga una fase hasta `TECHO_POR_SUELO`— sigue abierto.

---

## ✏️ REVISION DEL 11/09/2026 — `D-25`: CUATRO CAMARAS, DOS POR POSTE (solo §1.7, §1.7.bis y §3.5)

**El responsable, 11/09:** *«mantener estas conexiones como definitivas»* —las de la guia del Sisga
revisada con el el 10/09—. **En cada poste: camara 1 entre `J16` p9 (3,3 V) y p10 (`CAM_C_PIN`),
camara 2 entre `J16` p11 (3,3 V) y p12 (`CAM_D_PIN`); talanquera en `J15` (p1 12 V, p2 drenador
de `Q10`) por rele a la entrada `OPEN` de la centralita.** Deroga de `D-13` **solo** *«una camara
por poste / `p12` vacio»*. Guia de campo: `05_Funcional/Camaras_Sisga_4x.html` (version del 11/09).

**Lo que el que cablea tiene que saber, MEDIDO en el firmware de `a6980e4`** (el mismo que `b79d904`):

| | medida | simbolo |
|---|---|---|
| las dos camaras de un poste hacen **LO MISMO** | un solo bucle: flanco -> `demanda_solicitar()` + vigilante | `CAM_J16[2]`, `camaras_actualizar()` |
| **ninguna frena la pluma** — baja con un coche debajo | la pluma sube con `(verde && !testLedsActivo) \|\| estado == S_FALLO`; ninguna camara entra. El veto es `A-1.bis`, **sin construir** | `escribirPines()` |
| la pluma **sube tambien con el ambar intermitente**, incluido un poste recien encendido que aun no enlaza | `S_FALLO` desde `C_MENU_IDLE` sin comunicacion, orfandad SFTY-6, Modo Ambar, `AMBAR_EMERGENCIA`, Degradado en ambar | `semaforo_iniciarFallo()` |
| en Automatico/Manual **no cambian ninguna luz**; en Inteligente **solo alargan**, con techo | la demanda solo la leen `modo_inteligente.cpp` y la rama `CMD_DEMANDA`, atendible solo en `MODO_INTELIGENTE` | `demanda_hayLocal()`, `camara_presenciaJ16()`, `TECHO_POR_SUELO` |
| **una camara que nunca dio flanco no la vigila nadie** y la app pinta `CAM: OK — las dos ven` con la primera deteccion de CUALQUIERA (tambien la APK del 10/09) | la exencion se escribio para el `p12` vacio y **no ha cambiado**: pendiente EN FIRMWARE | `vigilante_tick()`, `camara_estado()`, `camHuboFlanco` |

🔴 **Y DOS CONFLICTOS QUE ESTE DOCUMENTO NO RESUELVE, del responsable:** (1) ~~**`J14`**: `A-2` manda
ahi el fin de carrera y el firmware lo lee como camara de demanda (§1.7.bis, fila `CAM_DEMANDA_PIN`)~~
-> 🟢 **CERRADO por `D-27` (11/09): `J14` libre y sin cablear, el fin de carrera no se instala**;
(2) `D-25` dice *«las camaras no tocan el ciclo»* y en el Modo Inteligente **si lo alargan**
(`TECHO_POR_SUELO`). Y una correccion de paso que no es de `D-25`: `p10` y `p12` **si llevan
condensador en el netlist** (`C28`, `C29`), al contrario de lo que decia §1.7.bis.

---

---

## Revisiones anteriores — ninguna se ha borrado, y aqui esta donde vive cada una

Se citan desde fuera **por su fecha** (*«17_…, revision del 04/09 por la tarde»*): la fila se queda
aqui y el texto integro esta en [`17_hist_Arquitectura.md`](17_hist_Arquitectura.md).

| revision | de que iba | donde esta |
|---|---|---|
| **11/09 (2.a) — `D-27`** · **11/09 — `D-25`** | `J14` libre; cuatro camaras, dos por poste | 🟢 **aqui arriba, vivas** |
| **07/09 — el Modo Degradado no puede entrar en ninguna de las dos puntas** | la cadena del reloj medida eslabon a eslabon; `D-20`, `D-26` y `D-28` | historico |
| **05/09 — `D-1` completa y `D-16`** | el mando se va como HARDWARE y su CODIGO se queda; la app queda como unica superficie de mando | historico |
| **03-04/09 — el banco corrio** | 🟢 **lo MEDIDO EN COBRE se queda abajo**; la cuenta del informe y la tabla de veredictos, al historico | **partida** |
| **04/09 (tarde) — tres cambios y una decision** | el minimo del ciclo sube de 1 a 3 min (`N75-1`); N-130 | historico |
| **04/09 (de noche) — cinco cambios, una decision y un hallazgo** | N-134, N-133, N-42, N-135 y la barrera de via de la app | historico |
| **04-05/09 (la noche del banco) — cuatro defectos de calle** | N-142, N-146, N-147, N-149, N-145 y el `BAT:--` de la cinta | historico |
| **31/08 — la revision del responsable** | los diez puntos que corrigio al documento del 28/08 | historico |

## 🔴 REVISION DEL 03-04/09/2026 — EL BANCO CORRIO. Leer esto ~~ANTES QUE TODO LO DEMAS~~ **justo despues del bloque del 05/09**, que lo deroga en dos filas

> **DE ESTA REVISION SE QUEDA AQUI LO MEDIDO EN COBRE, Y NADA MAS.** La discrepancia de la cuenta
> del informe de banco y la tabla «este documento decia / lo que midio el banco» estan integras en
> [`17_hist_Arquitectura.md`](17_hist_Arquitectura.md), en esta misma revision.

### Lo que se midio en `J16`, que es el nucleo de todo lo anterior

**MEDIDO EN COBRE** —multimetro, conector vacio, paso 20 de la Guia, 03/09/2026—:

| `J16` | R a masa (sin energia) | R a `3,3 V` (sin energia) | V contra masa (con energia) |
|---|---|---|---|
| p5 — `MANDO_A` (`PB9`) | **9,92 kOhm** | 11,28 kOhm | 🔴 **0,6 V** |
| p8 — `MANDO_B` (`PB13`) | **9,92 kOhm** | 11,28 kOhm | 🔴 **0,6 V** |
| p10 — Camara (`PB14`) | **9,93 kOhm** | 11,29 kOhm | **0 V** |
| p12 — Camara (`PB15`) | **9,94 kOhm** | 11,31 kOhm | **0 V** |

**El pull-down de 10 kOhm que declaraba el netlist es REAL, y esta en las cuatro posiciones**
(`R65`-`R68`, con su `100 nF`). Con `3,3 V` en la posicion de al lado en las cuatro —`J16` p4, p7,
p9 y p11—, el gesto que el conector pide es cerrar el contacto **contra los `3,3 V`**: **entrada
activa en ALTO, para los cuatro pines y sin excepcion.**

> **Las dos conclusiones que salen de esa medida** —el camino de camara correcto y EJERCIDO en el
> paso 21, y el camino de mando invertido en `617bd00` (N-118) y corregido en `346ea5f`— estan en el
> historico, en esta misma revision. **El gesto de prueba que de ahi sale sigue vivo abajo, en los
> dos avisos de `J16` de §1.7.**

### 🟢 D-12 — EL CONTRATO REAL DE LAS CAMARAS: **UN CONTACTO SECO, y nada mas**

> **De cada camara el sistema consume UN CONTACTO SECO. No hay red, no hay imagen, no hay video, y
> no hay analitica en el controlador.** (`DECISIONES.md`, fila **D-12**, 05/09.)

> El desarrollo de `D-12` —el censo de `WiFi`/`RTSP`/`esp_camera` con su control negativo, la
> microSD de la camara (`A-0`) y lo que le hace a §3.3— esta en el historico, en esta misma revision.

### 🔴 N-120 — la tarjeta protege todo lo que sale y no protege nada de lo que entra

**MEDIDO en banco (03-04/09), y coherente con el netlist:** las **9 salidas** de campo van con
**220 Ohm en serie + optoacoplador `TLP127`**; las **5 entradas** de campo van **del borne directo a
la pata del STM32** — sin resistencia en serie, sin opto y sin clamp. Y `J16` p1 reparte **12 V
crudos** en ese mismo conector (§2.1).

**Consecuencia inmediata, y deja de ser una cautela de mesa:** ~~tapar `p1` antes de cablear nada en
`J16`~~ → **tapar `p1` es OBLIGATORIO en cada equipo que se monte**, no solo en el banco. En banco se
hizo **retirando el pin del cuerpo del conector volante** (paso 4), que es mas fiable que la funda
termorretractil porque no se puede deshacer por accidente. Es el metodo que se documenta.

**La decision de diseno que abre —2K2 en serie por entrada, con su cuenta— es de V2, es del
responsable, y esta en §3.6.**

### 🔴 N-116 — la tarjeta Maestro esta fuera de servicio

**MEDIDO despues del incidente del paso 29: hay un corto entre `3,3 V` y `GND`.** La tarjeta arranca,
funciona unos **30 segundos** y se calienta. 🛑 **No se reenergiza** hasta inspeccion tecnica con el
equipo frio.

**El firmware queda descartado como causa, y por CENSO, no por opinion:** ninguna de las **9 salidas**
que el firmware escribe toca un pin de `J16`. Nada de lo que pasa por `escribirPines()` puede llegar
a `p5` ni a `p8`. **Eso no nombra al culpable** —el informe deja la causa abierta, y hace bien—: lo
que hace es **sacar de la lista al unico sospechoso que este repositorio podia haber revisado
leyendo**, que es justo el que se habria revisado.

> **El Bluetooth eran DOS defectos en serie** —N-117 en el ESP32, N-122 en la app, los dos
> arreglados **sin banco**—: el detalle esta en el historico, en esta misma revision.

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
>
> ✅ **ACTUALIZADO EL 04/09: eso ya no es cierto de TODO el documento, y la diferencia importa.** La
> sesion de banco del 03-04/09 dejo las **primeras filas medidas sobre la placa fisica**: `J16`
> p5/p8/p10/p12 (paso 20), `J17` p2/p3 contra las patas 42 y 43 del `U1` (paso 5), `J14` (pasos 17 y
> 18), `J15` (pasos 15 y 16) y las masas del modulo (paso 23). **Esas se marcan `MEDIDO EN COBRE` y
> llevan el numero del multimetro al lado.** Todo lo demas sigue siendo `MEDIDO` sobre ficheros, y no
> se mezclan: la escala no gana un cuarto nivel, gana una **procedencia** que hay que escribir.
>
> **Y la leccion, que este documento se aplica a si mismo:** §2.2 estuvo semanas declarada como
> *"contradiccion irresoluble desde aqui"* y el cobre la resolvio en una tarde con un ohmimetro.
> **Lo que un documento no puede decidir no siempre es indecidible: a veces solo esta esperando a que
> alguien baje al banco.**

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
| 🔴 Rojo peaton / Verde peaton | `PA6` `PA7` | `J11` `J9` | `pines.h:15-16` — **DECLARADOS Y MUERTOS** |
| Barrera (talanquera) | `PB2` | `J15` | `pines.h:31` |
| 🔴 Buzzer | `PB1` | `J13` | `pines.h:20` — **DECLARADO Y MUERTO** |
| Radio LoRa (`USART3`) | `PB10` TX · `PB11` RX · `PB12` DE/~RE | `J12` | `#define RS485_OUT_TX` / `RS485_OUT_RX` / `LORA_DE_RE` en `pines.h` — `grep -n -e RS485_OUT_ -e LORA_DE_RE Maestro/include/pines.h` |
| Camara de demanda | `PB0` | `J14` | `pines.h:46` |
| 🆕 Camaras `C` / `D` | `PB14` `PB15` | `J16` p10 / p12 | `#define CAM_C_PIN` / `CAM_D_PIN` en `pines.h`, ya rotulados *"camara de contacto seco"* — `grep -n "define CAM_._PIN" Maestro/include/pines.h` — ~~**NO cablear hasta `M3`**~~ → ✅ **M3 CERRADA el 03/09: se pueden cablear.** `0 V` en reposo y sin demandas fantasma (paso 21) |

> ## 🔴 TRES DE ESTAS SALIDAS NO EXISTEN MÁS QUE EN LA TABLA (medido el 02/09)
>
> **`ROJO_PEATON` (`PA6`), `VERDE_PEATON` (`PA7`) y el `BUZZER` (`PB1`) están declarados en
> `pines.h` y MUERTOS en las dos puntas**: `grep` de esos tres nombres sobre `Maestro/src` y
> `Esclavo/src` **no devuelve ni un `pinMode`, ni un `digitalWrite`, ni un `digitalRead`**. Sólo
> aparecen en la línea del `#define`.
>
> **Lo que eso significa en obra: si alguien cablea una cabeza peatonal a `J11`/`J9` o un zumbador a
> `J13`, no se enciende nunca y no hay mensaje de error.** El montaje parece bien hecho y el equipo
> parece sano. Es la avería más cara de diagnosticar, porque no hay síntoma que buscar.
>
> **No las venda ni las incluya en una entrega como funciones del equipo.** El hardware está en la
> placa —sus optos y sus MOSFET—; **lo que falta es el firmware**, y no está escrito.
>
> ⚠️ **Y por eso la barrera de salidas de `CLAUDE.md` §6 hay que leerla con cuidado:** dice ocho
> pines de luz y **`escribirPines()` mueve SEIS** —`ROJO1/2`, `AMARILLO1/2`, `VERDE1/2`—. La regla es
> **vacuamente cierta** para los dos peatonales: nadie los escribe porque nadie los escribe. La
> talanquera sí entró dentro de `escribirPines()` el 27/08 (`ESTADO.md`, fila `A2`).

> ## 🔴 AMPLIADO EL 05/09 CON UN CENSO DE COBRE — y son SEIS pines libres, no tres
>
> **El bloque de arriba sigue entero y no se toca: es cierto.** Lo que faltaba es la otra mitad, y
> cambia tres cifras que este documento publica como hechos. Todo lo de abajo sale del
> `.kicad_pcb` (2.158.421 B), del `.kicad_sch` y del desensamblado del `.elf`. **El censo completo,
> con los comandos pegados, está en `05_Funcional/2_Manual_Hardware_y_Pruebas.md` §11.**
>
> ### 1 · Los tres canales muertos están COMPLETOS en la placa
>
> No es que "haya optos y MOSFET": es que `J9`, `J11` y `J13` son **el mismo molde exacto** que la
> talanquera de `J15`, **que sí funcionó en banco el 04/09**. Pin del `U1` → `R` 220 Ω → opto
> `TLP127` → `R` 10 K a masa más `R` 220 Ω a la puerta → `IRLZ44N` de lado bajo → bornera, con
> `1N4148` de rueda libre. Diez cadenas iguales, `Q1`–`Q10` con `U6`–`U15`.
>
> **Encender uno cuesta 16 B de flash**, medidos desensamblando el `.elf`: un `pinMode` con pin
> constante y un `digitalWrite` ocupan 8 B cada uno en Thumb-2. ⚠️ **Ese 16 es un SUELO**: son las
> dos llamadas, y no incluye nada de lo que **decide** el valor.
>
> ### 2 · 🔴 EL BORNE NO ESTÁ A 0 V EN REPOSO: ESTÁ A ~12 V
>
> **Ningún documento de este repositorio lo decía, y decide si esos bornes se pueden enchufar a
> algo.** Nueve de los diez drenadores llevan un **pull-up de 1 kΩ más LED al riel de 12 V**
> (`R23`, `R28`, `R33`, `R38`, `R43`, `R48`, `R53`, `R58`, `R63`, `R73`), y está **en el cobre, no
> en el conector**: no se evita dejando un hilo sin poner. Con el MOSFET abierto el borne sube a
> **~12 V**, con **~10 mA** disponibles por la cuenta `(12 menos ~2 de LED) / 1 kΩ`.
>
> ✅ **Explica una medida de banco que llevaba desde el 04/09 anotada SIN CAUSA:** `J15` daba *«en
> rojo `0 V`, en ámbar `12 V`»*, que es exactamente este circuito con la sonda entre p1 y p2. *(Que
> la sonda estuviera ahí es DEDUCCIÓN a partir de `TALANQUERA_ABRIR = HIGH`, no una lectura del
> informe: `SIN VERIFICAR`.)*
>
> 🔴 **Consecuencia de vocabulario, y no es un matiz: un MOSFET a masa NO es un contacto seco**, y
> aquí ni siquiera es un colector abierto limpio. Este proyecto usa *«contacto seco»* con razón para
> las **entradas** de cámara; **las salidas no lo son**.
>
> 🔴 **Y la excepción, que es un hallazgo nuevo: `J8` (`VERDE2`, `PA5`) NO tiene ese pull-up.** `D21`
> —su LED— tiene el cátodo **sin conectar**, en el esquemático y en el cobre: la red se llama
> `unconnected-(D21-K-Pad1)`, no tiene ni una pista, y su gemelo `D23` sí llega al drenador. Con el
> MOSFET abierto, `J8` p2 **queda FLOTANDO**, no a 12 V. **`SIN VERIFICAR` si es defecto o decisión**
> — no hay una sola nota sobre ello en el repositorio, y se cierra con un multímetro entre `J8` p1 y
> p2 comparado contra `J7`.
>
> ### 3 · 🔴 «El opto aísla galvánicamente» es MEDIO CIERTO, y la mitad que falla importa
>
> Es la frase con la que `2_Manual_Hardware_y_Pruebas.md` concluía que *«la etapa de potencia no
> puede inyectar corriente al micro»*. **Se ha tachado allí con su motivo el 05/09.**
>
> Medido: **hay UNA sola red `GND` en toda la tarjeta**, con **103 pads** y **plano de cobre en las
> dos capas**. En ella están a la vez el **cátodo del LED de cada opto** y la **fuente de cada
> MOSFET**. El opto separa el **pin del micro** del nodo de puerta; **no crea una masa separada**.
> **Cualquier cosa colgada de esos bornes comparte la masa del controlador** — y su riel de 12 V, por
> p1.
>
> Lo que sigue en pie: no hay camino de corriente del drenador a la pata del `U1`. Ése es el mérito
> del diseño, y por eso `J15` sigue siendo *«bien diseñada»* frente a las cinco entradas desnudas de
> §2.1. Lo que deja de poderse decir es que lo colgado esté **aislado del equipo**.
>
> ### 4 · Los pines libres son SEIS: hay que sumar `PB3`, `PB4` y `PB5`
>
> `lcd.cpp` pasó los pines de la pantalla a `U8X8_PIN_NONE`, y la librería **se salta el `pinMode` y
> el `digitalWrite` cuando el pin es `NONE`** (`U8x8lib.cpp`, función `u8x8_gpio_and_delay_arduino()`:
> `if (u8x8->pins[i] != U8X8_PIN_NONE)` y `if (i != U8X8_PIN_NONE)`). **Están hoy en alta impedancia,
> con pista hasta `J17`**: `PB3` a p4 (red `/SCL`), `PB4` a p1 (`/CS`) y `PB5` a p5 (`/SI`). **Son los
> únicos GPIO libres del proyecto con bornera ya cableada.**
>
> **Que dos de ellos sean patas de JTAG no cuesta nada aquí**, y conviene decirlo bien: `pin_function()`
> llama a `pin_DisconnectDebug()`, que en `PinAF_STM32F1.h`, función `pinF1_DisconnectDebug()`, hace
> `__HAL_AFIO_REMAP_SWJ_NOJTAG()` para `PA_15`, `PB_3` y `PB_4` — *«JTAG-DP Disabled and SW-DP
> enabled»*, literal del fuente. **SWD se conserva.** ⚠️ **`PB5` NO es pin de JTAG**: no está en esa
> lista, y decir que los tres lo son es falso.
>
> ⚠️ **Lo que sí cuesta: `J17` es el conector donde vive el ESP32.** Lo que se cuelgue de p1, p4 o p5
> convive con el módulo. **Que eso no le moleste NO está medido: `SIN VERIFICAR`.**
>
> ### 5 · Y la decisión que esto abre, que NO se toma aquí
>
> Gastar uno de los tres canales de `J9`/`J11`/`J13` **cierra la puerta a una cabeza peatonal o a un
> zumbador en esta placa**: no hay más molde libre. **No existe ninguna decisión escrita que renuncie
> a ellos.** Va a **§3.10**, abierta y con dueño.

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
Esclavo/src/bluetooth.cpp   static HardwareSerial SerialBT(PB7, PB6);   <- grep -n "HardwareSerial SerialBT"
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

> 🔵 **ACTUALIZADO EL 31/08/2026 — DECISION DEL RESPONSABLE.** Esta tabla decia que se retiraban los
> cuatro pulsadores y el mando entero. **Ya no.** Lo tachado se conserva con su motivo, que es como se
> corrige en este repositorio.

| se retira | consecuencia inmediata |
|---|---|
| Pantalla LCD (las dos puntas) | toda la operacion de menu pasa por la app |
| ~~Los cuatro pulsadores (`PB9`, `PB13`, `PB14`, `PB15`)~~ → **solo `BOTON3` (`PB14`) y `BOTON4` (`PB15`)** | libera `J16` **p10 y p12**, que es lo que las camaras necesitan. ~~**rompe la unica salida de modo**~~ → **§2.3 REFUTADA: la salida por app ya existe** |
| ~~Mando de 4 reles~~ → ~~🟢 **SE CONSERVA en los canales A y B**~~ → 🔴 **05/09 (`D-1`): el HARDWARE se retira; el CODIGO se queda** | `MANDO_A` = `BOTON1` = `PB9` = `J16` p5 · `MANDO_B` = `BOTON2` = `PB13` = `J16` p8. **El veto de §2.4 se queda donde esta** — y con el mando desmontado su bandera **no se arma nunca**, que es lo correcto. Los pines siguen leidos activos en ALTO: **lo que entre por `J16` p5/p8 sigue componiendo secuencias** |
| Modulo Bluetooth SPP dedicado (`HC-05`/`JDY-30`) | lo sustituye el ESP32 — ver §3.1 |

> El censo del 31/08 que explica **por que se conservan los DOS canales** del mando, y la linea de
> aquel censo que ya no se puede repetir, estan en el historico, §1.6.

> # 🔴 07/09 — LA FRASE DE ABAJO ES CIERTA Y SE LEE AL REVES DE LO QUE HOY SIGNIFICA
>
> *"`A.A.A`, `B.B.B` y `A.B.A.B` siguen funcionando"* se escribio el 31/08 como **tranquilizador**:
> las camaras entran sin romper el mando. Con `D-1` cumplida —**el hardware del mando se fue**— esa
> misma frase dice otra cosa: **hay tres ordenes vivas colgando de dos bornes VACIOS y expuestos**,
> `J16` p5 (`PB9`) y p8 (`PB13`).
>
> 🔴 **Y la que mas pesa esta MEDIDA HOY EN EL MAESTRO, y no estaba escrita en ninguna parte: la
> secuencia `A.A.A` entra al Modo Automatico SIN NINGUNA GUARDA.**
>
> ```
> $ grep -n -A3 "confirmarYActuar(ACC_AUTOMATICO" 01_Firmware/Maestro/src/mando.cpp
> ```
>
> La rama llama a `modoAutomatico_pedirArranqueDirecto()` y a `modoActual_set(MODO_AUTOMATICO)` —
> **arranca el ciclo, o sea ABRE PASO**—, y el comentario del propio fuente dice que *"no necesita
> proteccion porque el sistema se corrige solo"*. **Ese razonamiento se escribio para un pulsador
> que solo alcanzaba un tecnico subido a la escalera.** Las otras dos secuencias si estan frenadas.
>
> **Consecuencia operativa, y es una regla de montaje, no una cautela: NADA se cablea en `J16` p5 ni
> p8.** Un fin de carrera de talanquera que suba y baje tres veces dentro de la ventana **es una
> secuencia del mando**. Eso es lo que `A-2` de `DECISIONES.md` esta decidiendo, y por eso alli
> figura como decision **de seguridad** y no de reparto de pines.

> El parrafo del 31/08 —*«el mando vive entero en `A` y `B`»*— y su caveat de re-medida estan en el
> historico, §1.6.

### 1.7 Las camaras a `J16`

Los pines que libera la retirada de los pulsadores 3 y 4:

> 🛑 **LA COLUMNA «red» ES EL NOMBRE DE LA RED EN EL NETLIST DE KiCAD, NO EL PAPEL DEL PIN.** Se
> llaman `/Boton1`..`/Boton4` porque asi se bautizaron las pistas cuando la placa se diseno con
> botonera, y **ese nombre no se puede cambiar sin retocar el `.kicad_sch`**. **Lo que decide que
> se cablea ahi es la ultima columna.** `p10` y `p12` **son ENTRADAS DE CAMARA**: quien lea
> `/Boton3` como *«boton 3»* y cablee un pulsador esta cableando en el borne de una camara.

| `J16` | red **(netlist KiCad — nombre heredado, NO es el papel)** | GPIO | uso nuevo · **esto es lo que manda** |
|---|---|---|---|
| p1 | `/12V` | — | 🔴 **12 V crudos. Se tapa** — ver §2.1 y **N-120: es OBLIGATORIO, no una cautela de banco** |
| p2 | `GND` | — | masa |
| p5 | `/Boton1` | `PB9` | ~~**vacio a proposito** (colchon)~~ → ~~🟢 **`MANDO_A`. VA CABLEADO** (31/08)~~ → 🔧 **CADUCADO EL 05/09 (`D-1`): el mando NO se monta, asi que `p5` queda LIBRE Y SIN CABLEAR.** ⚠️ **Pero el firmware SIGUE LEYENDO este pin** —`BOTON1` alimenta `botonArriba()`, con llamadores vivos— o sea que **lo que se cierre aqui contra `p4` mueve cosas dentro**. ~~`0,6 V` en reposo, N-118~~ → **refutado el 05/09: eran del firmware viejo con `INPUT_PULLUP`** |
| p8 | `/Boton2` | `PB13` | ~~**vacio a proposito** (colchon)~~ → ~~🟢 **`MANDO_B`. VA CABLEADO** (31/08)~~ → 🔧 **CADUCADO EL 05/09 (`D-1`): idem `p5` — LIBRE Y SIN CABLEAR, y el firmware sigue leyendolo** (`BOTON2` → `botonAbajo()`) |
| p10 | `/Boton3` | `PB14` | 🎯 **`CAM_C_PIN` — ENTRADA DE CAMARA de DEMANDA.** ~~«Boton 3 / Aceptar»~~ ✅ **cableada y verificada en banco el 03/09** (paso 21). Y desde `4b90f98` ademas **se vigila sola** (§1.7.bis). ✏️ **11/09, `D-25`: la CAMARA 1 de CADA poste**, contacto de alarma entre `p9` (3,3 V) y `p10` |
| p12 | `/Boton4` | `PB15` | 🎯 **`CAM_D_PIN` — entrada de camara**~~, y HOY SE DEJA VACIO.~~ ~~«Boton 4 / Cancelar-Menu»~~ `0 V` en reposo, MEDIDO (paso 20). ~~🔵 **07/09: `D-13` es UNA CAMARA POR POSTE, asi que en cada equipo montado UNO de estos dos pines esta vacio — y el firmware DEPENDE de ello** (la exencion del vigilante, `botones.cpp`: sin esa linea alarmaria `CAM_CIEGA` de una camara que no existe). **El que se cablea es `p10`**, que es el ejercido en banco; **este se queda libre**~~ → ✏️ **11/09, `D-25` (el responsable: *«mantener estas conexiones como definitivas»*): la CAMARA 2 de CADA poste**, contacto de alarma entre `p11` (3,3 V) y `p12`. **Hace LO MISMO que la de `p10`** (`CAM_J16[2] = {CAM_C_PIN, CAM_D_PIN}`, un solo bucle en `camaras_actualizar()`). 🔴 **Lo que el firmware sigue creyendo:** la exencion del vigilante se escribio para un `p12` vacio y **no ha cambiado** (`vigilante_tick()` salta el pin con `!camHuboFlanco[i]`; `camara_estado()` lo salta al publicar `CAM:`): **una camara de `p12` muerta desde la instalacion no la avisa nadie**, y la app pinta `CAM: OK` con la primera deteccion de la de `p10`. Pendiente de rehacer EN FIRMWARE. ⚠️ **`p12` NO se ha cableado nunca en banco** (§3.5) y es el borne **mas cercano a la red de 12 V** (`1,359 mm`, abajo): `p1` tapado (`D-4`) antes de nada |

> 🔴 **`p5`/`p8`: POR QUE NO BASTA CON «el mando se retiro» — MEDIDO EL 05/09, y es la mitad que un
> resumen se come.** De los cuatro getters de boton, **dos estan muertos y dos NO**:
>
> | | hoy | consecuencia |
> |---|---|---|
> | `botonAceptar()` · `botonCancelar()` | **`return false;`** en las dos puntas | **nada de `p10`/`p12` ejecuta ni cancela**. Es lo que libera esos dos pines para las camaras |
> | `botonArriba()` · `botonAbajo()` | 🔴 **VIVOS** — `consumir(0)`/`consumir(1)`, alimentados por `digitalRead(b.pin) == HIGH` sobre `BOTON1`=`PB9` y `BOTON2`=`PB13`, **con llamadores vivos en `menu.cpp` y `modo_hora.cpp` de las dos puntas** | **lo que se cierre en `p5`/`p8` contra los 3,3 V sigue entrando al firmware**, y ademas compone secuencias del mando (§2.4) |
>
> ```
> grep -n "bool botonAceptar\|bool botonCancelar\|bool botonArriba\|bool botonAbajo" Maestro/src/botones.cpp Esclavo/src/botones.cpp
> grep -rn "botonArriba\|botonAbajo" Maestro/src Esclavo/src
> grep -n "pinMode(BOTON[12], INPUT)\|b[12].pin = BOTON[12]" Maestro/src/botones.cpp Esclavo/src/botones.cpp
> ```
>
> **Por eso «los cuatro botones estan fuera» es media verdad y no se escribe asi en ningun sitio:**
> fuera esta el **pulsador** (el plastico), y fuera estan **Aceptar** y **Cancelar** (el codigo).
> **Arriba y Abajo siguen leyendo cobre**, y ese cobre esta a un milimetro y medio de los 12 V de
> `p1` (§1.7 abajo). **`p5` y `p8` se dejan sin cablear, y no se puentean «para probar»** fuera del
> paso 29 de la guia.

> La ambiguedad de *«Camara 1»* / *«Camara 2»*, la leccion del `grep` sin `-E` cuyo control negativo
> daba cero, y las dos filas tachadas del colchon estan en el historico, §1.7.

**Y el colchon en si tampoco media lo que decia.** Esto es lo que estaba escrito, con el paso del
footprint (`Molex_KK-254_AE-6410-16A_1x16_P2.54mm_Vertical`, 16 pads, tanto en `J16` como en `J17`):

| de `p1` a | posiciones | ~~distancia~~ **distancia entre PADS** |
|---|---|---|
| `p5` (`PB9`) | 4 | ~~10,2 mm~~ |
| `p10` (`PB14`) | 9 | ~~22,9 mm~~ |
| `p12` (`PB15`) | 11 | ~~27,9 mm~~ |

> 🔴 **REFUTADO el 31/08. Esas tres cifras son la distancia entre PADS, y esa NO es la separacion
> real entre los 12 V y la senal.** Los 12 V son una **red**, no un pad: salen de `J16` p1 y recorren
> la placa hasta las diez borneras de potencia con pistas de hasta 1,0 mm. Medido cobre a cobre
> —pads, pistas y vias, respetando capas— en `03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md:576-588`:
>
> | red de 12 V contra | separacion minima real | donde |
> |---|---|---|
> | `/Boton1` (p5) | **1,405 mm** | pista `B.Cu` 1,0 mm <-> pad `J16`.5 |
> | `/Boton2` (p8) | **1,408 mm** | pista `B.Cu` 1,0 mm <-> pad `J16`.8 |
> | `/Boton3` (p10) | 4,269 mm | pista `B.Cu` 1,0 mm <-> pad `J16`.10 |
> | **`/Boton4` (p12)** | **1,359 mm** <- **el peor** | via <-> pista `F.Cu` |
>
> **El orden se INVIERTE.** El colchon daba `p12` por el punto mas alejado de los 12 V —27,9 mm, el
> mas seguro del conector— y por cobre es **el peor de los cuatro**. El margen real no es de 10 mm
> sino de **1,36 mm**, cobre de diseno, sin tolerancia de fabrica ni suciedad ni humedad de un
> armario en la calle.
>
> Es `CLAUDE.md` §4 aplicada a una magnitud: **se midio lo que era facil de contar —posiciones de un
> conector— y se publico como si fuera lo que importaba.** Un pad no es una red. La conclusion
> operativa no cambia de signo pero si de tamano: `p1` se tapa **igual**, ~~y las camaras siguen sin
> cablearse hasta M3 (§2.2)~~.
>
> 🟢 **CADUCADA LA SEGUNDA MITAD EL 03/09, Y SE TACHA EN VEZ DE BORRARSE PORQUE ES LA FRASE QUE
> RESUCITA UN BLOQUEO QUE YA NO EXISTE.** **M3 se hizo** (paso 20 de la Guia de banco) y **cerro**:
> `p10` mide `9,93 kOhm` a masa y `p12` `9,94 kOhm`, los dos a **`0 V` en reposo**, y **`p10` se
> cablo contra `p11` en el paso 21 sin una sola demanda fantasma**. La camara **se cablea**. Lo unico
> que sigue en pie de este recuadro es **`p1`: se tapa, y desde N-120 eso es OBLIGATORIO en cada
> equipo que se monte** —§2.1 y §3.6—, no una cautela de banco.

> 🛑 **LOS DOS AVISOS DE `J16` QUE NO PUEDEN VIVIR EN UNA NOTA AL PIE, PORQUE SE LEEN CON UN
> DESTORNILLADOR EN LA MANO:**
>
> 1. **`J16` p1 lleva 12 V CRUDOS** —sin opto, sin resistencia en serie y sin clamp, contra entradas
>    de 3,3 V que van **desnudas** a la pata del STM32—. **Taparlo es OBLIGATORIO en cada equipo que
>    se monte** (N-120). No es del banco: es del montaje.
> 2. 🔴 **`MANDO_A` y `MANDO_B` NO RESPONDEN.** Medido en banco: `p5` y `p8` en **`0,6 V`
>    permanentes** con `617bd00` dentro, y **no se pudo pulsar el mando** (N-118). El fuente ya esta
>    corregido en las dos puntas (`346ea5f`, `INPUT` pelado y activo en ALTO) y **no se ha cargado en
>    ninguna tarjeta**. **El gesto de prueba es cerrar `p5` contra `p4` y `p8` contra `p7` —los
>    3,3 V del pin contiguo—, NUNCA contra masa:** en todo `J16` hay **una sola masa** (`p2`).
>
---

#### 1.7.bis 🟢 EL CAMINO DE CAMARA, CENSADO ENTERO — **lo que falta es COBRE, no codigo**

> **Esta es la entrada del trabajo siguiente, asi que va medida y no resumida.** Censado contra el
> fuente el 05/09, funcion por funcion. Cada linea se puede repetir abriendo el fichero que se cita.

**LAS TRES ENTRADAS DE CAMARA POR PUNTA, y NO son intercambiables:**

| entrada | pin | conector | ayuda de la placa | quien la declara | quien la lee |
|---|---|---|---|---|---|
| **`CAM_DEMANDA_PIN`** | `PB0` | **`J14`** | 🟢 **`R64` 10 kOhm + `C25` 100 nF — antirrebote RC de 1 ms EN LA PLACA**, escrito encima del `#define` | Maestro: `pinMode(CAM_DEMANDA_PIN, INPUT)` en `botones_setup()` · Esclavo: el mismo `pinMode` en `setup()` de `main.cpp` | Maestro: `camara_leerPin(CAM_DEMANDA_PIN)` en `modoInteligente_loop()` · Esclavo: `digitalRead(CAM_DEMANDA_PIN) == HIGH` en `main.cpp`. ~~🔴 **CONFLICTO ABIERTO (11/09), del responsable — NO se resuelve aqui:** `A-2` (cerrada el 05/09) manda a `J14` el **fin de carrera** de la pluma, y **no lleva camara** (Manual 9);~~ pero el firmware **lo sigue leyendo como camara de demanda** en las dos puntas —Maestro por NIVEL (pide y SOSTIENE fase en Inteligente), Esclavo por FLANCO -> `demanda_solicitar()` -> `CMD_DEMANDA`—. **Un fin de carrera cableado ahi daria demandas falsas cada vez que se mueve la pluma.** 🟢 **`D-27` (11/09): `J14` LIBRE, sin cablear — el fin de carrera NO se instala en este despliegue, y con eso el conflicto se cierra sin tocar el firmware.** Vacio, `R64` lo deja en 0 V (`J14` MEDIDO EN COBRE, pasos 17-18). **Mientras el firmware lea `PB0` como demanda, en `J14` no se conecta nada** |
| **`CAM_C_PIN`** | `PB14` | **`J16` p10** | 🟠 **`R67` 10 kOhm a masa, MEDIDA en cobre: `9,93 kOhm`.** ~~**SIN condensador**~~ ✏️ **11/09: FALSO en el netlist** — `C28` 100 nF entre `/Boton3` y `GND` (`.kicad_pcb`, y `MAPEO_TARJETA_KICAD.md`, fila p10). **El condensador no se ha medido en cobre** | `pinMode(CAM_C_PIN, INPUT)` en `botones_setup()`, **las dos puntas** | `camaras_actualizar()` — su siembra `camaras_sembrar()` — y 🆕 **el vigilante de `D-13` fase 1** (`vigilante_flanco()`, `vigilante_nivel()`, `vigilante_tick()`), **las dos puntas** |
| **`CAM_D_PIN`** | `PB15` | **`J16` p12** | 🟠 **`R68` 10 kOhm a masa, MEDIDA: `9,94 kOhm`.** ~~**SIN condensador**~~ ✏️ **11/09: FALSO en el netlist** — `C29` 100 nF entre `/Boton4` y `GND`; sin medir en cobre. ✏️ **Y desde `D-25` lleva la CAMARA 2 de cada poste** | `pinMode(CAM_D_PIN, INPUT)` en `botones_setup()` | idem |

> ✏️ **11/09 — LA ASIMETRIA DE ANTIRREBOTE QUE ESTA TABLA DABA POR MEDIDA NO ESTA EN EL NETLIST.**
> `R67`+`C28` y `R68`+`C29` son el mismo `10 kOhm` + `100 nF` que `R64`+`C25` en `PB0`: sobre el
> papel, **las tres entradas llevan el mismo RC**. La frase «sin condensador» se repite en el
> comentario de `camara_leerPin()` de `botones.cpp` (*«PB14/PB15 no llevan mas que el 10K de
> R67/R68»*) — **comentario de firmware, no se toca desde aqui**: queda anotado para quien lo
> tenga. Lo que si esta medido en cobre son las resistencias (paso 20); **los condensadores, no**.

**Las seis casillas de las dos ultimas columnas salen de tres `grep`, y ninguno lleva numero:**

```
grep -n "pinMode(CAM_" Maestro/src/botones.cpp Esclavo/src/botones.cpp Esclavo/src/main.cpp
grep -n "camara_leerPin\|digitalRead(CAM_DEMANDA_PIN)" Maestro/src Esclavo/src -r
grep -n "camaras_actualizar\|camaras_sembrar" Maestro/src/botones.cpp Esclavo/src/botones.cpp
```

**Las tres son `INPUT` PELADO y ACTIVAS EN ALTO**, y el gesto es **cerrar el contacto seco contra los
3,3 V del propio conector** — nunca contra masa. La cuenta que lo demuestra esta entera en la
cabecera de `pines.h`, bajo el rotulo *"POR QUE ACTIVO EN ALTO, Y POR QUE ESO NO ES UNA
PREFERENCIA"* — `grep -n "POR QUE ACTIVO EN ALTO" Maestro/include/pines.h`.

> El dibujo de **como convergen** las tres entradas, el `OR` de tres terminos de
> `modoInteligente_loop()`, la ventana de silencio de la demanda y las tres citas por numero de
> linea que se cayeron estan en el historico, §1.7.bis.

> ✅ **CONCLUSION, Y ES LA QUE EL RESPONSABLE NECESITA PARA DECIDIR QUE TOCA AHORA: el firmware de las
> dos camaras de `J16` YA ESTA CONSTRUIDO Y EJERCIDO. Lo que falta es COBRE, no codigo.**
>
> - **Ejercido en banco:** paso 21 del 03/09 — `p10` cableada contra `p11`, **funciono y sin demandas
>   fantasma en reposo**.
> - **Ejercido por instrumento:** `camara_01_demanda` y `camara_02_j16` en el banco por packs.
> - **`M3` cerrada en cobre** (paso 20): las cuatro posiciones con su pull-down real de 10 kOhm.

> El vigilante de `D-13` fase 1 —`CAM_PEGADA`, `CAM_CIEGA` y el contador `camVetos`, que **observa y
> no veta**— y el caducado de `presenciaActual` estan en el historico, §1.7.bis.

> 🔴 **Lo que este censo NO dice, y no se da por dicho:** **ninguna camara AcuSense se ha conectado
> nunca a este equipo.** Lo que se cablo en el paso 21 fue **un puente de `p10` a `p11`**, no una
> camara. La salida de la AcuSense es configurable (NO/NC) y **cual de los dos estados significa
> demanda es una decision de parametrizacion que sigue SIN TOMAR** — `9_Manual_Parametrizacion_
> Camara_IA.md`. **SIN VERIFICAR con una camara real en las tres entradas.**
>
> ✏️ **11/09 — y la unica medida de campo que hay tampoco lo cambia:** la cinta del Maestro del
> Sisga (10/09, `evidencia/2026-09-10_Sisga_179DB0_cinta_tramas.txt`) trae **253 tramas con `CAM:`
> y las 253 dicen `CAM:?`** —`grep -o "CAM:[A-Z?]*" … | sort | uniq -c`—: en esos veinte minutos
> **ninguna camara le dio un flanco al equipo**. Con `D-25` son **cuatro** camaras las que faltan
> por ver en cobre, y `p12` —la camara 2 de cada poste— **no se ha cableado nunca**, ni en banco.

---

> 🔴 **Y la consecuencia que nadie debe deshacer por comodidad de montaje: con `MANDO_B` al aire,
> `mando_ambarLocal()` NO SE ARMA NUNCA.** De esa bandera cuelgan **tres vetos** en
> `Esclavo/src/main.cpp` (~~`:406`, `:416`, `:540`~~ → **`:453`, `:476`, `:617`, re-medidos el 05/09;
se citan por el `grep` que los encuentra, §4.sexies: `grep -n "mando_ambarLocal()" Esclavo/src/main.cpp`),
todos de la forma `if (!mando_ambarLocal() &&
> !bluetooth_ambarEmergencia())`. Con la bandera muerta los tres `if` son **siempre verdaderos** y
> **el veto de SFTY-21 desaparece**: una orden de radio le quitaria el ambar a la punta donde un
> operario esta subido al gabinete. **Dejar `p8` sin cablear no deja el mando "inerte": deja el veto
> ABIERTO** — §2.4. Un `J16` montado sin `p5` y `p8` **no es un montaje incompleto, es un montaje
> distinto**, y ningun test lo dice.

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
>
> 🔴 **AMPLIADO EL 04/09 — N-120, y sube de precaucion a obligacion.** El banco confirmo en cobre lo
> que este apartado deducia del netlist, **y encontro que es peor de lo que decia**: no es que `J16`
> no tenga aislamiento, es que **ninguna de las 5 entradas de campo de la tarjeta lo tiene**,
> mientras ~~**las 9 salidas**~~ → **las DIEZ salidas** llevan `220 Ohm` en serie y opto `TLP127`. La
> proteccion de esta placa es **asimetrica**, y esta entera del lado por el que no entra nada.
>
> 🔴 **Dos correcciones del 05/09, medidas sobre el mismo fichero de cobre y tachadas con su motivo:**
> (a) **son DIEZ cadenas, no nueve** —`Q1`-`Q10` con `U6`-`U15`—, que es lo que este mismo documento
> ya decia bien tres parrafos mas arriba; (b) **"aislamiento" dice de mas**: el opto separa el PIN del
> micro del nodo de puerta, pero **hay UNA sola red `GND` en la tarjeta**, con 103 pads y plano en las
> dos capas, y en ella estan el catodo del LED del opto **y** la fuente del MOSFET. **Lo colgado de
> esas borneras comparte la masa del controlador.** Ver el bloque del censo en §1.2 y el detalle en
> `2_Manual_Hardware_y_Pruebas.md` §11.
>
> **Por eso `p1` deja de taparse «en banco» y pasa a taparse EN CADA EQUIPO QUE SE MONTE**, con el
> pin retirado del conector volante — que es como se hizo en el paso 4 y es lo que se documenta,
> porque no se deshace por accidente. **La decision de diseno que esto abre para V2 esta en §3.6**, y
> es del responsable.

### 2.2 La polaridad de los pines de boton — ✅ CERRADA EN COBRE EL 03/09 (M3 / N-118)

**Los numeros estan arriba**, en «Lo que se midio en `J16`»: pull-down real de 10 kOhm en las cuatro posiciones, **activa en ALTO**. El apartado entero —el analisis del 28/08 que motivo la medida, y por que no era una contradiccion sino un fuente equivocado— en el historico, §2.2.

### 2.3 `botonCancelar()` es la unica salida de todos los modos — 🟢 REFUTADA EL 31/08 (N-100)

Hay `SET_MODO:MENU` y seis comandos mas desde `d34cfe2`. El censo de llamadores y el de comandos por punta, en el historico, §2.3.

### 2.4 🔴 Retirar el mando no deja tres `if` inertes: **borra un veto**

**Lo que hay que no perder cabe en una linea, y esta abajo en §1.7.bis:** con el mando desmontado la bandera **no se arma nunca** —el veto queda CERRADO—; **retirando su ARMADOR los `if` se vuelven siempre verdaderos y el veto queda ABIERTO** (SFTY-21). El censo de llamadas, el armador, los trece packs que se irian a `ABORTADO` y el pack que falta escribir: historico, §2.4.

### 2.5 `SET_RTC` puede rechazar en silencio y contestar `RESULT:OK` — ✅ CERRADO EN N-80 (`d34cfe2`)

Y caducado otra vez por `D-15`/`D-26`: **el `SET_RTC` lo contesta el PUENTE, no el STM32.** El analisis y las cinco ramas de entonces, en el historico, §2.5.

### 2.6 🟠 Telemetria fabricada: que campos de `$STATUS` son datos y cuales son texto

Los literales se retiraron (N-108, N-149). La trama de hoy se lee del fuente —`grep -n '"\$STATUS' 01_Firmware/*/src/bluetooth.cpp`—, **no de un documento**: cualquier tabla de campos escrita aqui nace caducada, y este apartado lo demostro dos veces. Historico, §2.6.

### 2.7 🟢 Del mando de reles no se retira equipo en servicio: nunca se compro el receptor

Sigue cierto y ahora es definitivo (`D-1`): no hay que subir a un poste a desmontar nada. Historico, §2.7.

### 2.8 El protocolo de 80 pruebas: 49 dejan de ser ejecutables — ⚠️ CUENTA CADUCADA

Caen **menos** de 49, y el numero nuevo **no se ha recontado seccion por seccion**, asi que no se publica. La tabla del 28/08 y que filas se mueven, en el historico, §2.8.

---
## 3. Decisiones ABIERTAS, con dueno

Ninguna de estas ~~cinco~~ ~~seis~~ ~~nueve~~ **diez** la puede tomar quien escribe firmware. Van con
quien las tiene que firmar. *(La sexta —§3.6, N-120— la trajo el banco del 03-04/09 y no existia el
28/08. La §3.7 y la §3.8 salieron de la revision del 04/09 por la tarde; la **§3.9**, de la de la

### 3.1 Que chip es el ESP32 — 🟢 CERRADA EL 31/08 (`BLQ-1` / N-107). NO BLOQUEA NADA

Es un **`ESP32-WROOM-32` clasico** (BR/EDR + BLE), confirmado por ficha y por el socket SPP que abrio el banco del 3-4/09. La tabla de familias —la que hay que mirar el dia que llegue un modulo de otra procedencia— y lo que sigue bloqueando el montaje (linea `A5`, la fuente propia), en el historico, §3.1.

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

### 3.3 Sin pantalla, sin pulsadores y sin mando: como se opera el equipo si el ESP32 se cuelga — **CONTESTADA EL 05/09 POR `D-16`: NO SE OPERA**

**Es una propiedad DECLARADA de este sistema, no una averia:** un ESP32 colgado, muerto o desenchufado deja esa punta **sin ninguna superficie de mando**, y cortar y devolver la energia no lo arregla. Las cinco opciones con su coste —que es lo que permite revisar la decision— en el historico, §3.3.

### 3.4 El minimo de tiempo por sentido (N75-1) — ✅ DECIDIDA EL 04/09: **3 minutos**

Vive en el C++ (`VERDE_MIN_MIN` / `ROJO_MIN_MIN`, `Maestro/include/limites_ciclo.h`) y **no en este documento**. El motivo del responsable, el coste de banco aceptado y el cierre de N75-2, en el historico, §3.4.

### 3.5 La Camara 1: se queda en `PB0`/`J14`, o se muda a `J16` — ✅ CERRADA (05/09 hacia `J16`; `D-25` y `D-27` el 11/09)

**Lo unico de este apartado que no caduca es su aviso electrico, y esta arriba en §1.7:** `p12` es el punto del conector **mas cercano a la red de 12 V** y **ninguna entrada de campo lleva proteccion en serie** (§3.6). El desarrollo —incluida la pregunta mal hecha que se le llego a plantear al responsable sobre una premisa sin medir— en el historico, §3.5.

### 3.6 🔴 NUEVA (04/09) — N-120: las entradas de campo no tienen ninguna proteccion, y hay que decidir si V2 se la pone

**Dueno: el responsable.** Es una decision de diseno de placa: cuesta rediseno y no la desbloquea
ninguna medida mas. Y es la unica de este apartado que **no** existia el 28/08: la trajo el banco.

**MEDIDO en el banco del 03-04/09, y es una asimetria, no un olvido puntual:**

| | cuantas | que llevan en medio |
|---|---|---|
| **Salidas** de campo | **9** | `220 Ohm` en serie **+ optoacoplador `TLP127`** |
| **Entradas** de campo | **5** | 🔴 **nada.** Del borne **directo** a la pata del STM32 |

Y en el mismo conector de dos de esas entradas, `J16` p1 reparte **12 V crudos** (§2.1).

> **Lo que eso significa en la calle, en una frase:** la tarjeta esta blindada contra lo que ella
> hace y desnuda contra lo que le hacen. Cualquier tension que aparezca en un borne de entrada —un
> cruce de hilos en el armario, un cable de camara que roza `p1`, una descarga por la linea de una
> camara que vive fuera del gabinete— entra **entera** en una pata del micro que gobierna el
> semaforo. **La proteccion de esta placa esta toda del lado por el que no entra nada.**

**La propuesta tecnica, con su cuenta hecha para que se pueda discutir el numero y no la
sensacion — `2K2` en serie por entrada:**

| | con `2K2` | por que ese numero y no otro |
|---|---|---|
| **Corriente inyectada** con 12 V en el borne | **3,6 mA** | por debajo de los **5 mA** por pata que admite el datasheet del STM32F103: el diodo de clamp la aguanta en vez de morirse |
| **Nivel ALTO que ve el micro** con la camara cerrada | **2,70 V** | el `2K2` forma divisor con el pull-down de **10 kOhm** de la placa: `3,3 x 10/12,2` |
| **Umbral `VIH`** del micro | **2,31 V** | `0,7 x VDD`. Los `2,70 V` quedan **por encima**, con margen |
| **Con `4K7` en vez de `2K2`** | 🔴 **`2,24 V` — por DEBAJO de `VIH`** | **ya no leeria la camara.** No es *"mas proteccion, mejor"*: hay un techo, y esta cerca |

**Esa ultima fila es la razon por la que esto es una decision y no una obviedad:** el margen entre
*proteger* y *dejar de leer* es de un salto de valor de resistencia. La cuenta la fija el pull-down
de 10 kOhm que la placa ya trae —el mismo que M3 midio—, asi que **cambiar uno obliga a recalcular
el otro**.

> **Lo que este documento NO decide, y va escrito:**
> - **Si esto entra en V2 o se queda como esta.** Es coste de rediseno de placa contra un riesgo que
>   nadie ha visto materializarse todavia. El dato en contra de esperar es que **ya hay una tarjeta
>   fuera de servicio** (N-116) y la causa esta abierta.
> - **SIN VERIFICAR: el `2K2` no se ha probado en ninguna tarjeta.** La cuenta es aritmetica sobre el
>   datasheet y sobre el pull-down medido; **no es una medida**. Antes de mandar a fabricar, se monta
>   sobre una entrada y se comprueba que la camara sigue leyendose.
>
> **Y lo que SI esta decidido y no espera a V2:** ~~tapar `p1` es una precaucion de banco~~ →
> **tapar `p1` es obligatorio en cada equipo**, retirando el pin del conector volante. Eso no cuesta
> rediseno, no espera a nadie y ya se hizo una vez (paso 4).

### 3.7 ✅ DECIDIDA (04/09) — el cruce se opera DESDE EL MAESTRO

**El Esclavo PIDE; no ordena.** Y la consecuencia es operativa: **el operario tiene que saber a que poste conectarse ANTES de caminar**, y lo unico que se lo dice es el rotulo Bluetooth — que es lo que §3.8 deja abierto. El censo de despachadores por punta, la alternativa descartada y su motivo, en el historico, §3.7.

### 3.8 🔴 NUEVA Y ABIERTA (04/09) — un modulo virgen se llama igual en las dos puntas

**Dueno: el responsable**, porque lo que decide es lo que hace un tecnico de pie delante de dos
postes, y §3.7 acaba de apoyar en ello una decision de operacion.

**MEDIDO** en `01_Firmware/ESP32_Expansion/`:

```
   include/contrato.h:259          #define ROTULO_PROVISIONAL  "SEM-SIN-MATRICULA"
   src/transporte_app.cpp:23-31    el rotulo del arranque sale de la NVS; si no hay nada, el provisional
   src/transporte_app.cpp:37       spp.begin(rotulo)     <- el nombre se fija AL ABRIR el perfil
   src/transporte_app.cpp:105-113  el aprendido se GUARDA, y no se re-rotula en caliente
   src/puente.cpp:205-206          se aprende de paso, del $STATUS que ya se retransmite
```

**Como funciona, y por que esta bien pensado —esta parte no se discute:** la serie sale del **silicio
del STM32** (`identidad_serie()` lee el UID del micro), asi que **el ESP32 no puede saberla al
arrancar**. El puente la aprende del `$STATUS` que retransmite —sin tocarlo—, junto con el `NODE:`,
compone `SEM-<serie>-M` o `SEM-<serie>-E` y lo **guarda para la SIGUIENTE arrancada**. **No se
re-rotula en caliente a proposito**: cambiar el nombre SPP obliga a cerrar y reabrir el perfil, o sea
**a tirar la sesion del operario que en ese momento puede estar dando una orden al cruce**. Y un
`NODE:` que no se reconoce **no se rotula a medias**: `return`, sin valor por defecto
(`transporte_app.cpp:97`).

**LO QUE ABRE, y es la pregunta:** un modulo virgen —recien montado, con la NVS borrada, o con un
STM32 que todavia no ha hablado— anuncia **`SEM-SIN-MATRICULA`**. Con **dos** modulos virgenes, uno
por poste, **los dos postes se llaman exactamente igual** hasta que cada uno haya visto un `$STATUS`
**y se le haya cortado la energia**. Y §3.7 acaba de decidir que **el rotulo es lo que le dice al
operario a que poste caminar**.

> **Que hay que decidir, con las opciones escritas para que no se elija por eliminacion**
> (`CLAUDE.md` §4: *eliminar entre opciones incompletas es adivinar con tabla*):
>
> | opcion | coste | que resuelve |
> |---|---|---|
> | **Dejarlo y cubrirlo por procedimiento** — el montador da una vuelta de energia a cada modulo antes de irse y lo firma en el acta de puesta en marcha | cero firmware | resuelve el caso del montaje; **no** resuelve la NVS borrada en campo ni el modulo de repuesto que alguien enchufa un martes |
> | **Provisional distinto por modulo** — p. ej. con los ultimos bytes del MAC del propio ESP32, que si conoce al arrancar | bajo, firmware del ESP32 | **dos virgenes dejan de llamarse igual**; el rotulo sigue sin decir cual es el Maestro |
> | **Re-rotular en caliente** al aprender la matricula | bajo en lineas, 🔴 **alto en calle** | 🛑 **tira la sesion del operario que esta dando una orden.** Es justo lo que `transporte_app.cpp:109-111` evita a proposito |
> | **Que lo diga la app** en vez del rotulo: conectar a cualquiera y leer `NODE:` del primer `$STATUS` | cero firmware; la app ya recibe el campo | 🔴 **no resuelve el problema de §3.7**, que es saberlo **antes de caminar** |
>
> **Este documento no elige.** Lo que si deja escrito es que la primera opcion **es una decision**,
> no el estado por defecto: si nadie la firma, lo que hay es dos postes con el mismo nombre y nadie
> avisado.

**SIN VERIFICAR, y es la mitad que mas pesa:** **nadie ha visto este rotulo en la lista de
emparejados de un telefono.** No hay una sola tarjeta con un ESP32 conectado a `J17`, y el Bluetooth
**no subio en toda la sesion de banco del 3-4/09** (N-117 / N-122, arreglados **sin banco**).

---

> 🔴 **AMPLIADA EL 04-05/09 — EL RESPONSABLE DESCARTO LAS CUATRO OPCIONES DE ARRIBA Y APLAZO LA
> DECISION A DESPUES DEL BANCO.**
>
> Su frase sobre como se matricula hoy —mirando los **NOMBRES** de los Bluetooth— fue ***"es pura
> mierda"***, y lo que pidio son **dos** requisitos, no uno:
>
> | # | requisito |
> |---|---|
> | **1** | La matriculacion se hace por **ID de Bluetooth** (la direccion del modulo), **no por su nombre** |
> | **2** | **Sin intervencion manual.** Nadie teclea, nadie empareja a mano, nadie lee una etiqueta |
>
> **Esto no es una quinta fila de la tabla de arriba: la tumba entera.** Las cuatro opciones giraban
> sobre **como rotular**, y el requisito dice que **el rotulo no es la identidad**. La opcion 1
> —"cubrirlo por procedimiento"— es exactamente la intervencion manual que el requisito 2 prohibe.
>
> 🛑 **APLAZADO POR EL RESPONSABLE A DESPUES DEL BANCO. AQUI NO SE DISENA NADA**, y esa es la
> decision correcta esta noche: hay gente en banco y esto no bloquea ninguno de los pasos.
>
> **EL DATO DURO QUE CONDICIONA EL DISENO, Y VA ESCRITO AHORA PARA QUE NO SE PROPONGA LO IMPOSIBLE
> DENTRO DE UN MES** — MEDIDO en `Maestro/include/protocolo.h:242-248`, identico en el Esclavo:
>
> ```
>    struct RF_Packet {
>        uint8_t msgID;
>        uint8_t command;
>        uint8_t param;
>        uint8_t crc;
>    };
> ```
>
> **Son CUATRO BYTES Y NO HAY CAMPO DE DIRECCION.** El `crc` cubre **los tres anteriores**. O sea:
>
> - **el enlace de radio no sabe a quien va dirigido un paquete** — hoy funciona porque hay
>   exactamente **dos** radios en enlace directo (`CLAUDE.md` §10);
> - **una matricula que viaje por radio no cabe** sin cambiar la trama, y cambiar la trama toca
>   **las dos puntas, el CRC, la proteccion de replay y todos los packs que la modelan**;
> - **la identidad que hoy existe de verdad es la del STM32** —`identidad_serie()` lee el UID del
>   silicio— y el ESP32 **no la conoce hasta oir un `$STATUS`**.
>
> **Lo que esto NO dice:** no dice que haya que ampliar `RF_Packet`, ni que el ID de Bluetooth deba
> viajar por radio. **Dice cual es el terreno**, para que quien lo disene no descubra el limite
> despues de escribirlo. **SIN VERIFICAR:** nadie ha comprobado que la pila SPP del ESP32 exponga la
> direccion del **remoto** conectado por la interfaz que usa `transporte_app.cpp`; es la primera
> medida que hace falta y **no se ha hecho**.

### 3.9 🔴 NUEVA Y ABIERTA (04/09, de noche) — el borrado del respaldo no limpia los dos registros que N-133 estrena

**Dueno: quien decida el firmware**, y va aqui y no en el Anexo porque **lo que hay que decidir no
es como se arregla sino quien tiene la responsabilidad de limpiar** — el borrador o el lector.

**MEDIDO** en `01_Firmware/Maestro/src/respaldo.cpp` *(el fichero es identico en el Esclavo)*:

```
   :157-158   calcularSuma()      s = s*31 + leerReg(REG_CICLO_RV);
                                  s = s*31 + leerReg(REG_CICLO_DESPEJE);     <- SI entran en la suma
   :193-201   respaldo_borrar()   DR2, DR3, DR4, DR5, DR6  a cero
                                  DR9 y DR10  NO SE TOCAN                    <- y luego sellar()
```

**O sea: los dos registros nuevos entran en el checksum pero NO en el borrado, y `sellar()` los
firma como contenido valido sin haberlos limpiado.**

**Por que importa justo AHORA, y no antes:** `respaldo_borrar()` corre desde `respaldo_setup():185`
**cuando el contenido se declara invalido** — que es exactamente lo que la subida de firma de N-133
(`0x5EB1` -> `0x5EB2`) provoca en **la primera arrancada de cada equipo actualizado**. Es su caso de
uso principal, no un borde.

| escenario | que pasa hoy |
|---|---|
| equipo nuevo o pila agotada | `DR9`/`DR10` valen cero; `respaldo_tiemposCiclo()` se niega a devolver un cero (`:249`) y el equipo cae a sus minimos. ✅ **correcto** |
| **equipo ACTUALIZADO, pila buena** | `DR9`/`DR10` llevan lo que dejara el arranque anterior. Se **sellan como validos** y el lector los devuelve. 🔴 **el caso sin cubrir** |

**Lo unico que hoy lo frena es una segunda barrera en otro fichero:** la revalidacion de rango de
`modo_automatico.cpp:110-117` (`3..15 / 3..15 / 10..90`). **Un par de bytes que cayera dentro de las
tres ventanas pasaria**, y el cruce arrancaria con un ciclo que nadie configuro.

> 🔴 **Y esto contradice por escrito lo que el propio header promete**, `Maestro/include/respaldo.h:58-61`:
> *"respaldo_setup() encuentra el contenido invalido y borra: entonces respaldo_tiemposCiclo()
> devuelve false y el equipo arranca con sus minimos, que es la direccion segura"*. **La frase
> describe un borrado que no ocurre.** Es una afirmacion sobre el codigo, escrita al lado del
> codigo, que **nadie comprobo** — la misma forma de N-122.

> **Que hay que decidir, con las opciones escritas para que no se elija por eliminacion:**
>
> | opcion | coste | que resuelve |
> |---|---|---|
> | **Anadir `DR9`/`DR10` a `respaldo_borrar()`** | dos lineas | cierra el hueco donde nacio. Es el simetrico de lo que ya hace con los otros cinco |
> | **Que el lector exija ademas una bandera propia**, como `respaldo_hayCiclo()` hace con `FLAG_CICLO` | una bandera mas en `DR4` | distingue *"no configurado"* de *"borrado a medias"*, que hoy no se distinguen |
> | **Dejarlo y confiar en la revalidacion de rango** | cero | 🔴 **apoya una propiedad de seguridad en un fichero que no sabe que la sostiene**, y el header seguiria mintiendo |
>
> **Este documento no elige.** Lo que si deja escrito es que **la tercera es una decision**, no el
> estado por defecto.

**Y hay una segunda mitad, del mismo censo, mas pequena pero de la misma familia:** el comentario de
`respaldo.cpp:228` justifica el rechazo del cero diciendo que si no *"`respaldo_hayTiemposCiclo()`
mentiria"*. **Esa funcion no existe en el arbol.** La analoga que si existe es `respaldo_hayCiclo()`
(`:254`), y es la del **Degradado**, no la del automatico. Un comentario que nombra una funcion
inexistente es una nota que ya no se puede comprobar leyendo — y es como se llega a documentar una
Caja Negra que nadie llama.

> ⚠️ **Nada de esto se ha ejercido: es MEDIDO sobre fichero.** Y **no hay ningun pack que mire el
> borrado**: `Validacion_Respaldo` compila `calcularSuma()` y Horner, y su punto ciego declarado es
> justamente que **no ejerce el arranque** (`CLAUDE.md` §8). **Antes de tocar una linea aqui va el
> arnes, visto fallar** — uno que naciera en verde no mediria nada.

---

---

### 3.10 🔴 NUEVA Y ABIERTA (05/09) — hay TRES canales de potencia libres, y gastar uno cierra una puerta

**El censo de cobre del 05/09** (`2_Manual_Hardware_y_Pruebas.md` §11) encontro que `J9`
(`VERDE_PEATON`, `PA7`), `J11` (`ROJO_PEATON`, `PA6`) y `J13` (`BUZZER`, `PB1`) no son "pines
sueltos": son **tres etapas de potencia completas y fabricadas**, identicas a la de la talanquera de
`J15` **que si funciono en banco el 04/09**. Opto `TLP127`, MOSFET `IRLZ44N`, diodo de rueda libre,
LED testigo y bornera. Encender uno cuesta **16 B de flash** de suelo, medidos por desensamblado.

**Lo que hay que decidir no es tecnico, es de alcance:** son **los tres unicos canales de potencia
libres de esta placa**. El que se gaste **ya no esta** para lo que llevaba escrito encima.

| si se gasta en… | lo que se pierde a cambio |
|---|---|
| `J9` / `J11` — la pareja peatonal | **la cabeza peatonal de este cruce**, que es para lo que estan rotulados desde el dia uno |
| `J13` — el zumbador | **el aviso acustico**, que es la unica salida no visual del equipo |
| nada (se dejan como estan) | cero coste, y **tres canales fabricados sin usar** en una placa que ya no tiene mas moldes libres |

> # 🔴 AÑADIDO EL 07/09 — ESTA DECISION YA TIENE UN CUARTO PRETENDIENTE, Y ES `D-14`
>
> **`D-14` es una fila VIGENTE de `DECISIONES.md`** —*"la ENTRADA de alarma de la camara (grabar
> cuando el controlador cierra un contacto) es la via que NO depende de la casilla bloqueada"*—, y
> **para cerrar ese contacto hace falta una salida.** Las unicas libres de esta placa son
> **exactamente estas tres**. O sea que `D-14` no es un tema aparte: **es un cuarto candidato a
> gastar uno de los tres canales**, y nadie lo habia escrito en esta tabla.
>
> 🔴 **Y lo primero que hay que saber, medido hoy por separado: `D-14` NO ESTA IMPLEMENTADA. Cero
> lineas, en las dos puntas.**
>
> ```
> $ grep -rn "ROJO_PEATON\|VERDE_PEATON\|BUZZER" 01_Firmware/Maestro/src/*.cpp 01_Firmware/Esclavo/src/*.cpp
> 01_Firmware/Maestro/src/main.cpp:35://   ROJO_PEATON y VERDE_PEATON, que estaban sin custodia.
> ```
>
> **Una sola coincidencia en las dos puntas, y es un COMENTARIO.** Sin `pinMode`, sin
> `digitalWrite`, sin un llamador. Fuera de `semaforo.cpp`, la unica salida que el Maestro mueve es
> la direccion del RS485. **La decision esta tomada, el cobre fabricado, y no hay una linea que
> cierre ese contacto** — y era el argumento de una compra.
>
> ⚠️ **Y antes de que nadie tire un cable de ahi a la entrada de alarma de la camara, hay dos cosas
> `SIN VERIFICAR` a los dos lados y ninguna es menor:**
>
> | lado | lo que no se sabe |
> |---|---|
> | **nuestro borne** | esta a **~12 V en reposo**, no a 0 V (pull-up de 1 kOhm + LED al riel de 12 V, en el cobre), y **el opto NO crea masa separada**: hay **una sola red `GND`**, asi que lo que se cuelgue comparte la masa del controlador |
> | **la camara** | que espera electricamente su `ALARM IN` — **sin tension, sin corriente**, y las palabras *"dry contact"* y *"relay"* **no aparecen en las 110 paginas** del manual del fabricante (`D-14`, apartado de `SIN VERIFICAR`) |
>
> **Conectar una salida que en reposo esta a 12 V, con masa comun, a una entrada cuyo regimen no
> conocemos, no es un cableado: es un ensayo.** Va con la camara delante y con el multimetro, y
> **antes** de escribir la linea de firmware — no despues.

> **Este documento NO elige, y no por prudencia: porque no hay ninguna decision escrita que renuncie
> a ellos.** `DECISIONES.md` no tiene ni una fila sobre los peatonales ni sobre el zumbador. Lo que
> se ha escrito hasta hoy —en este mismo documento y en el Manual 2— es que **estan MUERTOS en el
> firmware**, que es una descripcion del estado, **no una renuncia**. Confundir las dos cosas es como
> se derogan decisiones de palabra (`CLAUDE.md` §2.quinquies).

**Lo que hace falta para poder decidirla:**

1. 🟡 **`SIN VERIFICAR`: que `J9`, `J11` y `J13` esten REALMENTE SOLDADOS.** El esquematico los marca
   `in_bom=yes`, `dnp=no`, `on_board=yes`, pero **nadie los ha mirado en cobre**. Lo mas cerca que hay
   es `J15`, el gemelo, que si funciono. Eso lo hace probable; **no lo demuestra**. Es una inspeccion
   a ojo mas continuidad, y va **antes** que la decision.
2. **Si el cruce lleva o no paso peatonal.** Es una pregunta de obra, no de firmware, y la contesta el
   responsable.
3. **Que quede claro que lo que se cuelgue ahi comparte la masa del controlador** —hay una sola red
   `GND` en la tarjeta— **y que el borne esta a ~12 V en reposo**, no a 0 V. Las dos cosas cambian que
   se puede conectar. Detalle en `2_Manual_Hardware_y_Pruebas.md` §11.2 y §11.4.

> ⚠️ **Y una cuarta cosa que NO es parte de esta decision pero se cruza con ella:** los **otros tres**
> pines libres —`PB3`, `PB4`, `PB5`, los del LCD— **no** traen etapa de potencia, pero **si traen
> bornera ya cableada** (`J17` p4, p1 y p5). Si lo que hace falta es una **entrada** o una senal de
> nivel logico, esos son el sitio y **no cuestan ninguno de los tres canales**. Si lo que hace falta
> es **mandar 12 V a algo**, no sirven. Son dos preguntas distintas y conviene no mezclarlas.

---

## A. Las cinco medidas de multimetro, en orden

> 🔴 **El motivo por el que esta seccion existe: hoy no hay ni una fila «VERIFICADO EN LA PLACA» en
> todo el mapeo de la tarjeta.** `MAPEO_TARJETA_KICAD.md` §0 y §9 lo declaran, y sigue siendo cierto
> el 28/08. Todo lo que sabemos del cobre sale de un dibujo.
>
> ✅ **ACTUALIZADO EL 04/09: cuatro de las cinco se ejecutaron en el banco del 03-04/09.**
>
> | | estado | donde |
> |---|---|---|
> | **M1** — cual es `J16` y cual `J17` | ✅ **HECHA** (paso 3): `J16` p1 da 12 V, `J17` p1 no | informe §3.2 |
> | **M2** — `J17` sin 12 V, y p2/p3 = `PB7`/`PB6` | ✅ **HECHA** (paso 5): continuidad a las patas 43 y 42, **ni un pin por encima de 3,3 V** | informe §3.2 |
> | **M3** — la polaridad de los pines de boton | ✅ **HECHA, y es la que mas cambio** (paso 20) — ver abajo | informe §3.7 |
> | **M4** — los 12 V de `J16` p1 | ✅ **HECHA** (pasos 3 y 4): 12 V confirmados, y `p1` **retirado del conector volante** | informe §3.2 |
> | **M5** — masa comun del ESP32 y reposo de su TX | ✅ **HECHA** (paso 23): **0 V** entre masas —por debajo del umbral de 50 mV— y `GPIO17` en **3,3 V**, no 5 V | informe §3.8 |
>
> **Las cinco se anotan en `MAPEO_TARJETA_KICAD.md` §9 con su fecha, que es lo que esta seccion pedia
> desde el 28/08.** Ese fichero **no lo toca este documento** — va en la lista de la seccion B.

**Las tres primeras van con la tarjeta SIN ENERGIA. Las dos ultimas con energia, y antes de unir los
dos equipos.** Cada una se anota en `MAPEO_TARJETA_KICAD.md` §9 con la fecha, para que empiece a
haber filas de ese nivel.

> 🔵 **LAS CINCO RECETAS DE MULTIMETRO SE MUDARON AL HISTORICO; SUS RESULTADOS SE QUEDAN ARRIBA, EN
> ESTA MISMA TABLA.** Se sacaron porque las cinco estan **HECHAS** y su numero ya esta publicado
> aqui — pero **una medida no se borra por haberla hecho una vez: hay una segunda tarjeta, y habra
> mas**, asi que **quien tenga una tarjeta nueva delante abre el historico y las repite**.

| medida | que cierra | receta |
|---|---|---|
| **M1** | cual de los dos conectores es `J16` y cual `J17` — comparten footprint y **confundirlos es meter 12 V donde va el ESP32** | historico, §A · M1 |
| **M2** | que `J17` no tiene 12 V en ninguna posicion, y que p2/p3 son `PB7`/`PB6` | historico, §A · M2 |
| **M3** | la polaridad de los cuatro pines de `J16` — **sus numeros estan arriba**, en la revision del 03-04/09 | historico, §A · M3 |
| **M4** | los 12 V de `J16` p1 | historico, §A · M4 |
| **M5** | la masa comun del ESP32 y el nivel de reposo de su TX | historico, §A · M5 |

> ⚠️ **Y lo que M3 dejo pendiente, que no caduca:** el voltaje de `p5`/`p8` **con el puente puesto**
> —el dato que se perdio con el incidente de N-116— sigue sin tomarse, y **no sobre la tarjeta
> Maestro** mientras siga con el corto.

---
## B. Que documentos quedan FALSOS, y en que orden hay que tocarlos

El criterio del orden no es el gusto: **primero lo que hace salir dinero, despues lo que promete una
salida de emergencia que ya no existe, al final el acta que se firma.**

> **Ninguno de estos ficheros se ha tocado al escribir este documento.** Lo que sigue es el censo,
> no el arreglo.

> **EL CENSO DE DOCUMENTOS FALSOS SE MUDO ENTERO AL HISTORICO, §B**, y con el las cinco fichas del
> orden. Se saco porque es una **lista de alcance**, y una lista de alcance es una afirmacion sobre
> el arbol que caduca igual que una cifra (`CLAUDE.md` §14): la de abajo se escribio el 28/08 y
> desde entonces `D-1`, `D-12`, `D-13`, `D-15`, `D-16`, `D-18`, `D-20`, `D-25`, `D-26` y `D-27` han
> movido casi todas sus filas. **Se RECUENTA con `grep` antes de ejecutarla; leerla es creersela.**

| orden | documento | donde esta la ficha |
|---|---|---|
| **1** | `05_Funcional/15_Lista_de_Compras_Hardware.md` — hay dinero a punto de salir | historico, §B · Orden 1 |
| **2** | `05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.md` — congelado, y manda enchufar un `HC-05` en `J17` | historico, §B · Orden 2 |
| **3** | `05_Funcional/8_Procedimiento_Modo_Degradado.md` | historico, §B · Orden 3 |
| **4** | `04_Manuales/MANUAL_MANDO_4_RELES.md` — describe un accionador que no esta | historico, §B · Orden 4 |
| **5** | `05_Funcional/3_Protocolo_Pruebas_Rigurosas.md` — el acta que se firma | historico, §B · Orden 5 |
| — | el segundo bloque (doce documentos mas, sin dinero de por medio) | historico, §B |

---
## C. Lo que este documento NO mide, y nadie debe dar por medido

Se escribe explicito porque un documento de arquitectura que no marca sus bordes se lee como un
permiso.

| | |
|---|---|
| ~~**Nada del cobre**~~ → **casi nada del cobre** | ✅ **el banco del 03-04/09 dejo las primeras filas medidas**: `J16` p5/p8/p10/p12, `J17` p2/p3 y sus tensiones, `J14`, `J15` y las masas del modulo. **Todo lo demas de esta tarjeta sigue siendo netlist y esquematico** — ⚠️ **y el censo del 05/09 NO cambia esto: es lectura del `.kicad_pcb`, no punta sobre la placa.** Lo que si hace es decir **que** hay que medir, y esta abajo en cuatro filas nuevas |
| ~~**El chip que llego a obra**~~ | ✅ **CERRADO: `ESP32-WROOM-32` clasico**, BR/EDR + BLE. Era lo mas barato y lo mas bloqueante, y ya no bloquea |
| **El pico de 500 mA del ESP32** | ESCRITO en el Manual 15, no medido sobre el modulo real. 🔴 **Y sigue sin medirse por un motivo nuevo: en banco el modulo se alimento por USB, no por la fuente `12 V -> 5 V` de la placa definitiva** (paso 22, parcial) |
| ~~**Que el enlace `J17` funcione**~~ | ✅ **el enlace fisico SI:** continuidad a las patas 42/43, masa comun por debajo de 50 mV, `GPIO17` en 3,3 V y el montaje definitivo encendido sin calentamiento ni reinicios (pasos 5, 23 y 24). 🔴 **Lo que sigue sin verificarse es que por ese enlace hable alguien**: el Bluetooth no subio en toda la sesion (N-117 / N-122, arreglados **sin banco**) |
| **El `Y2` de la segunda tarjeta** | N-37 midio uno. El otro sigue sin diagnosticar (`ESTADO.md` `B5`). 🔴 **Y el paso 27 —«el reloj conserva la hora»— quedo BLOQUEADO**: la unica via de consultar el `DS3231` es `SET_RTC` por Bluetooth, que no subio |
| ~~**Que las camaras funcionen en `PB14`/`PB15`**~~ | 🟡 **a medias, y hay que decir cual mitad.** ✅ **el cableado si**: `p10` cablado contra `p11` en normalmente abierto, `0 V` en reposo, **sin demandas fantasma con cable y sin el** (pasos 20 y 21). 🔴 **La concesion de paso NO**: depende del Modo Automatico, que no se pudo seleccionar sin app |
| **El firmware del ESP32** | ~~**no existe**~~ → **existe, compila y se cargo sin errores** (`01_Firmware/ESP32_Expansion/`), con su `DS3231` por `GPIO21`/`GPIO22`. 🔴 **Lo que no esta demostrado es que funcione**: el modulo no se anuncio de forma fiable en el telefono en toda la sesion |
| **La regresion N-42** | el Modo Automatico no mueve las luces en banco, y **sigue abierta**. 🔴 **El banco del 03-04/09 NI la confirmo NI la descarto** —el equipo nunca llego a operar, porque se queda esperando seleccion de modo y la app no conecto—. **Un ABORTADO no es un PASS**: sigue siendo lo primero de la proxima sesion |
| 🔴 **La causa de N-116** | la tarjeta Maestro tiene un corto entre `3,3 V` y `GND` **medido**, y **la causa esta abierta**. El firmware queda descartado por censo —ninguna de las salidas de potencia toca un pin de `J16`; ~~9~~ **son 10, censadas el 05/09**—, y eso **no nombra a nadie mas** |
| 🔴 **Que el mando `A`/`B` funcione con la polaridad corregida** | ~~la correccion de `botones.cpp` **no esta escrita ni cargada**~~ → 🟢 **escrita el 04/09 en `346ea5f`, las dos puntas.** 🔴 **NO cargada y NO ejercida**, y el unico intento de pulsarlo acabo en el incidente de N-116. **Nadie ha visto nunca a este equipo obedecer un `A·A·A`.** El gesto de la proxima prueba es `p5`-`p4` y `p8`-`p7`, **no contra masa**, y no sobre la Maestro |

| 🔴 **Que el minimo de 3 minutos aguante en la tarjeta** | el cambio esta **MEDIDO en el fuente** (`Maestro/include/limites_ciclo.h`, constante `VERDE_MIN_MIN`) y **no se ha cargado en ninguna tarjeta**. Trae ademas un coste de banco declarado: **ya no hay ciclos de un minuto para probar en mesa**, y la proxima visita tiene que contar tres minutos por paso |
| 🔴 **Que N-130 se VEA desde la app** | el rechazo llega como evento `MAESTRO / DEMANDA_NO_ATENDIDA_MODO_ACTUAL` (`Esclavo/src/main.cpp:542`), no como `$ERR`. **Que la app lo pinte y el operario lo lea NO se ha comprobado**, y sin eso el cierre es medio: se deja de mentir, pero puede no decirse nada |
| 🔴 **El coste de flash de los cambios del 04/09 por la tarde** | **no medido por la compuerta.** El acta de `624eb37` es anterior a ellos, y la lectura directa del `.elf` **no reconcilia** con el delta reportado. La discrepancia esta publicada en la revision del 04/09 (tarde); **el numero bueno sale de correr la compuerta sobre el arbol de hoy** |
| 🔴 **El rotulo Bluetooth** | **nadie lo ha visto en un telefono.** §3.7 apoya en el una decision operativa —a que poste camina el operario— y §3.8 deja abierto que **dos modulos virgenes se llaman igual** hasta una vuelta de energia |
| 🔴 **Que el ambar ordenado (N-134) llegue a la otra punta** | el comando existe y las dos puntas lo comparten (`protocolo.h:174`), **MEDIDO sobre fichero**. **Nadie ha visto salir el `CMD_GO_AMBAR` de un Maestro ni entrar en un Esclavo**, y era en banco donde se veia el sintoma que este cambio arregla —*"a veces los dos, a veces solo el maestro"*—. La red de la orfandad de 25 s sigue puesta, asi que un fallo degrada al comportamiento de ayer |
| 🔴 **Que la primera arrancada tras N-133 deje los tiempos en los minimos** | la subida de firma esta **MEDIDA** (`respaldo.cpp:76`), pero `respaldo_borrar()` **no limpia `DR9`/`DR10`** y los sella como validos (**§3.9**). En un equipo actualizado con pila buena **el resultado depende de bytes que nadie ha leido**. **Se mira en el poste, no se supone** |
| 🔴 **Que el PIN de la app caduque EN LA APK** | los dos plazos estan **MEDIDOS** en el fuente (`PIN_GRACIA_FONDO_MS` y `PIN_INACTIVIDAD_MS` de `App_Semaforo/www/app.js`), pero cuelgan de `visibilitychange` / `pagehide` y **no hay `pause` de Cordova**. **Nadie lo ha visto caducar con el telefono en el bolsillo y la pantalla apagada**, que es el unico escenario para el que existe. **SIN VERIFICAR** |
| 🔴 **Que N-106 salga de verdad por el todo-rojo** | el camino esta **MEDIDO** (`Esclavo/src/bluetooth.cpp:293-308`) y **no se ha ejercido ni en tarjeta ni en arnes**. `CLAUDE.md` §8.bis pide ver fallar el instrumento antes de fiarse, y para este camino **no se ha hecho** |
| 🔴 **La causa del `FORMATO_INVALIDO` del Courier RTC** | **SIN DIAGNOSTICAR.** La app lo traduce a lenguaje de obra desde el 04/09, y eso **hace legible el sintoma sin decir nada de la causa**. Aqui no se propone ninguna |
| 🔴 **Que `J9`, `J11` y `J13` esten SOLDADOS en la tarjeta** | 🆕 **05/09.** El esquematico los marca `in_bom=yes`, `dnp=no`, `on_board=yes`, y el `.kicad_pcb` trae sus huellas, sus optos (`U12`, `U13`, `U14`) y sus MOSFET (`Q7`, `Q8`, `Q9`). **Nadie los ha mirado en cobre.** Lo mas cerca es `J15`, el gemelo exacto, que si funciono en banco el 04/09 — eso los hace probables, **no ciertos**. Es una inspeccion a ojo mas continuidad, y va antes de §3.10 |
| 🔴 **Los ~12 V de reposo y los ~10 mA de las borneras de potencia** | 🆕 **05/09. Leidos del cobre, NO medidos con multimetro.** El pull-up de 1 kOhm mas LED al riel de 12 V esta en el netlist (`R23`, `R28`, `R33`, `R38`, `R43`, `R48`, `R53`, `R58`, `R63`, `R73`) y la corriente sale de una **cuenta**, no de una sonda. Se cierra en dos minutos midiendo p2 contra masa con la bornera desconectada |
| 🔴 **Si el `D21` sin conectar de `J8` es defecto o decision** | 🆕 **05/09.** El catodo del LED del canal de `VERDE2` esta **sin conectar** en el esquematico y en el cobre (red `unconnected-(D21-K-Pad1)`, cero pistas), asi que `J8` p2 **flota** en reposo mientras los otros nueve suben a 12 V. **No hay ni una nota en el repositorio sobre ello.** Se cierra comparando `J8` p1-p2 contra `J7` p1-p2 con el mismo estado de luz |
| 🔴 **Que `PB3`/`PB4`/`PB5` se puedan usar con el ESP32 puesto** | 🆕 **05/09.** Estan libres y en alta impedancia, con pista a `J17` p4, p1 y p5 — **el mismo conector donde vive el modulo**. Que lo que se cuelgue de esas tres posiciones no le moleste **no esta medido**. Lo que si esta leido del fuente es que usarlos **no cuesta el SWD**: `pinF1_DisconnectDebug()` hace `NOJTAG`, no `SWJ_DISABLE` |

> 🛑 **La compuerta del 28/08 salio con `15 PASS | 0 FALLA | 0 ABORTADO` y eso no autoriza nada de
> este documento.** Lo dice el acta y lo dice `CLAUDE.md` §3: ese `0` significa que *los modelos y
> los arneses de PC no encuentran nada*. **Ninguno de ellos toca la tarjeta**, ninguno tiene
> bornera, y ninguno sabe si el cobre de la tarjeta es el del plano. **Verde no
> es entregable.**

---

---

## Anexo · Cambios que otros ficheros necesitan y que este documento NO ha hecho

**Los catorce puntos —con los que ya estan hechos tachados en su sitio, que es como se dice aqui que
una tarea no ha desaparecido en silencio— estan integros en el historico, «Anexo».** Los que siguen
ABIERTOS y no son de este documento, para que no se pierdan en la mudanza: el **pack que exija que
`ACC_AMBAR` siga siendo el unico armador de `ambarLocal`** (punto 2), el **pack que ate la polaridad
de los cuatro pines de boton al netlist** (punto 9), que **`maestro_10` mire tambien el `case` y no
solo el `==`** (punto 12), que **la caducidad del PIN se ejerza EN LA APK y no en un navegador**
(punto 13), y el **pack que ate el borrado del respaldo a su checksum** (punto 14, §3.9).

---

*Escrito el 28/08/2026. Revisado el 31/08, el 04/09, el 05/09, el 07/09 y el 11/09/2026, y **partido
el 12/09/2026** en este fichero y en [`17_hist_Arquitectura.md`](17_hist_Arquitectura.md), que trae
la cuenta que demuestra que no se perdio nada. Lo marcado **MEDIDO** se repite abriendo el fichero
que se cita; **MEDIDO EN COBRE**, con un multimetro y trae el numero que dio; **ESCRITO** tiene su
fuente al lado; **SIN VERIFICAR** no lo ha comprobado nadie, ni aqui ni en ningun otro sitio.*
