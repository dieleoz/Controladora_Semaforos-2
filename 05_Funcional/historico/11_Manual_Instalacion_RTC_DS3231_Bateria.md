# ⏱️ MANUAL TÉCNICO DE INSTALACIÓN — PILA DEL RELOJ RTC Y PLAN DE CONTINGENCIA DS3231 (V9.0)

**Sistema:** Controladora de Semáforos Móviles de 3 Estados (Maestro y Esclavo V9.0)  
**Hardware Principal:** 🔴 **El reloj del cruce es el `DS3231` con pila del ESP32 de CADA poste** (`DECISIONES.md` **`D-9`**). ~~RTC Interno STM32 (`PC14`/`PC15` cristal `Y2` de 32.768 kHz) + Pila CR2032 en `VBAT`~~ — **corregido el 07/09:** el STM32 **no tiene reloj** (`Y2` confirmado muerto, N-17) y desde `D-15` **ni siquiera contesta a `SET_RTC`**. Lo que sigue describe el RTC interno, y se conserva porque el cuerpo del manual lo explica bien: **es la etiqueta de esta cabecera la que nadie había actualizado.**  
**Plan de Contingencia:** ~~Módulo Externo DS3231 en pines libres `PB0` (SDA) y `PB8` (SCL) por I²C Software~~ ⛔ **ANULADO el 28/08 — y además era FALSO: esos dos pines NO están libres.** El `DS3231` cuelga hoy del **ESP32** (`GPIO21` SDA / `GPIO22` SCL). Ver el aviso de abajo y el apartado 5  
**Propósito:** Sincronización horaria ininterrumpida para Modo Degradado, horario nocturno y Caja Negra  
**Verificación Hardware:** Esquemáticos KiCad `Controladora_Semaforos.kicad_sch`, `pines.h` y `MAPEO_TARJETA_KICAD.md`  
**Fecha de Emisión:** 26 de Agosto de 2026  
**Última revisión: 11 de septiembre de 2026 (tarde)** — 🔴 **`D-26`: LA HORA LA MANDA EL ESP32 DE CADA POSTE** (construida en `68dd2c5`, sin banco): con radio, el poste 2 hace caso al Maestro; **sin radio, toma la de su `DS3231`, que se le pone desde el teléfono en su gabinete**. ~~**`D-20`: LA AUTORIDAD DE LA HORA ES EL ESP32, Y ES UNA SOLA. La app NO pone la hora en el poste 2, nunca.**~~ *(revisión del 07/09, caducada en esa frase)* Ver el apartado final, **«Hay dos relojes por cruce»**, que es donde vive el procedimiento.
*7 de septiembre de 2026 (mañana)* — 🔴 **§4.1 DEROGADA: no se diagnostica el reloj mandando `SET_RTC` al STM32** (`D-15`). La orden vigente es **`CMD:LEER_RTC`** (`D-17`) y la contesta el ESP32.
*Revisión anterior, 31 de Agosto de 2026* — **el apartado 5 (Plan B DS3231) está ANULADO y no se cablea.**
Motivo: mandaba conectar un bus I²C a `PB0` y `PB8` llamándolos *«los dos únicos pines libres de la
placa»*, y **ninguno de los dos lo está**. Corregido también el renglón «Plan de Contingencia» de esta
cabecera y la columna de acción del apartado 4. **Lo demás de este manual —`R5`, la pila `CR2032` en
`VBAT` y la comprobación por pantalla— no se ha tocado y sigue vigente.**

---

## 🛑 AVISO DE SEGURIDAD — LÉASE ANTES DE TOCAR NADA (31/08/2026)

> ## ⛔ EL CABLEADO DEL APARTADO 5 (DS3231 a `PB0`/`PB8`) **NO SE EJECUTA**. NO ES UN «PLAN B» DISPONIBLE: ES UN ERROR DE ESTE MANUAL.

Este documento estaba **intacto desde el commit raíz y sin un solo aviso**, mandando cablear un bus
I²C contra dos pines que **ya tienen dueño en el firmware que corre hoy**.

**MEDIDO EN EL FUENTE** (`grep` sobre `pines.h`, idéntico en las dos puntas):

```
01_Firmware/Maestro/include/pines.h:46   #define CAM_DEMANDA_PIN    PB0  // -> R64 10K + C25 100nF -> bornera J14
01_Firmware/Esclavo/include/pines.h:46   #define CAM_DEMANDA_PIN    PB0  // (linea identica)
01_Firmware/Maestro/include/pines.h:63   #define LED_TESTIGO        PB8  // -> R16 1K -> LED D5. NO es entrada de camara
01_Firmware/Esclavo/include/pines.h:63   #define LED_TESTIGO        PB8  // (linea identica)
```

| pin | lo que dice este manual | lo que hay de verdad | nivel de prueba |
|---|---|---|---|
| `PB0` | «pin libre, `SDA` del I²C software» | **Entrada de la cámara de demanda**, bornera `J14`, con `R64` 10 kΩ de *pull-down* y `C25` de 100 nF de antirrebote | ✅ **MEDIDO EN EL FUENTE** (`pines.h:43-46`) |
| `PB8` | «pin libre, `SCL` del I²C software» | **`LED_TESTIGO`**: sale por `R16` de 1 kΩ al LED `D5` | ✅ **MEDIDO EN EL FUENTE** (`pines.h:50-63`) |

**Qué pasa si alguien sigue el diagrama del apartado 5:**

* En `PB0`, el `C25` de **100 nF** cuelga de la línea de datos. Son **250 veces** el límite de carga
  capacitiva del I²C (`roadmap.md:1899`): el bus **no puede conmutar**. Y mientras tanto ese hilo
  entra a la **entrada de demanda de la cámara**, que es la que pide paso.
* En `PB8`, `R16` de 1 kΩ **fija el nivel de la línea de reloj** contra el LED (`roadmap.md:1931`):
  un `SCL` que no puede subir.
* El resultado no es «un reloj que no funciona»: es **un hilo de un accesorio metido en la entrada
  que solicita verde**, en un equipo que gobierna un cruce.

> 🔴 **Este proyecto ya pagó DOS VECES por publicar un pin como libre sin cruzarlo con `pines.h`, y
> esta es la tercera aparición del mismo error.**
>
> * **N-67** (`roadmap.md:1745`): la entrada de cámara de `PB0` estaba **leída al revés** —`INPUT_PULLUP`
>   contra el *pull-down* de 10 kΩ de la placa deja el pin en 0,66 V, que el micro lee `LOW`—. El
>   firmware habría visto **demanda permanente desde el arranque, sin ninguna cámara conectada**. Salió
>   de ir a escribir *«la cámara se conecta aquí»* y no saber qué había en el otro borne de `J14`.
> * **N-59** (`roadmap.md:2194`): `CAM_UMBRAL_PIN` (`PB8`) se declaraba, se ponía en `INPUT_PULLUP` y
>   **no se leía nunca**, mientras **cuatro documentos** afirmaban que las cámaras 2 y 4 contaban
>   entradas y salidas del tramo. Cuatro manuales describiendo una función que no existía.
>
> **La regla que queda escrita: «pin libre» no es una observación, es una medida contra `pines.h`.**
> Un manual que llama libre a un pin ocupado no se equivoca en una palabra: manda un destornillador
> a una entrada viva.

---

## 1. Arquitectura del Reloj en la Tarjeta Madre STM32

> # 🔴 LA `CR2032` DE ESTA TARJETA SIGUE SIENDO OBLIGATORIA CON `D-20` — Y YA NO ES POR LA HORA
>
> **Esto se escribe el 07/09 porque `D-20` invita al error contrario**, y el error se comete una
> vez y se paga en el primer corte de luz: *«si la hora se mudó al `DS3231` del ESP32, la pila del
> STM32 ya no hace falta»*. **Es falso.**
>
> Esa misma pila alimenta el **dominio de respaldo** del STM32 (`BKP->DR1..DR10`), y ahí es donde
> viven **la marca de sincronización y el indicador del Modo Degradado** — el cómputo de las 48 h.
> Lo dice el propio fuente, símbolo `respaldo.h`:
>
> > *«Usa los registros de respaldo del STM32 (`BKP->DR1..DR10`), alimentados por LA MISMA pila
> > `CR2032` que ya mantiene el RTC»* · *«Sobreviven al corte porque los `BKP` viven en el dominio
> > de `VBAT`. Sin pila, o con la pila agotada, `respaldo_setup()` encuentra el contenido inválido
> > y borra»*
>
> **Con `D-20` la pila cambia de trabajo, no de necesidad:** deja de mantener una hora que nunca
> avanzó (`Y2` muerto, N-17) y pasa a mantener **la única cosa que dice si el Degradado sigue
> autorizado**. `roadmap.md` §3.4.bis lo pone como precondición con todas las letras: **`CR2032` en
> LAS DOS tarjetas.**
>
> ⚠️ **Y una precisión que ya se escribió mal una vez, el 07/09, y llegó a `DECISIONES.md`: NO se
> diga «el STM32 no tiene ni pila ni cristal».** Tiene las dos y no usa ninguna para la hora:
> `Y1` de 8 MHz está en la placa —el firmware arranca con el **HSI**, el RC interno— y `VBAT`
> **midió 3 V con la tarjeta apagada** (`N-37`) en **al menos una** tarjeta; la otra sigue
> `SIN VERIFICAR`. **Lo muerto es `Y2`**, el cristal de 32.768 kHz del RTC, y sólo ése.
>
> # 🔴 07/09 — `DECISIONES.md` FILA `D-22`: **ESE `Y1` DEJA DE SER UN DATO CURIOSO Y PASA A SER TRABAJO PENDIENTE**
>
> 🛑 **DECIDIDA Y SIN CONSTRUIR. El firmware de hoy sigue arrancando con el HSI y `Y1` sigue sin
> usarse.** Lo de arriba describe el estado real y **no cambia**; esto le pone encima la decisión.
>
> **`D-22`: `Y1` (8 MHz) pasa a ser el reloj de sistema del STM32.** Ya está montado y **no cuesta
> hardware**. El motivo es la precisión de lo que se cuenta con `millis()`: **10.000–25.000 ppm**
> del RC interno contra **20–50 ppm** de un cristal.
>
> ### 🛑 POR QUÉ ESTO IMPORTA **EN ESTE MANUAL**, QUE ES EL DE CAMBIAR PIEZAS DE RELOJ
>
> **Este manual es el que abre alguien con un soldador en la mano.** Con `D-22` sobre la mesa hay
> **dos cristales** en la conversación y **hacen cosas distintas**. Confundirlos es exactamente el
> error que este proyecto ya pagó tres veces:
>
> | | qué es | qué decide | estado |
> |---|---|---|---|
> | **`Y2`** — 32.768 kHz, `PC14`/`PC15` | el del **RTC** (`LSE_CLOCK`) | la hora y la **fase del Modo Degradado** | 🛑 **muerto** (`N-17`), y **es del que trata el resto de este manual** |
> | **`Y1`** — 8 MHz | el **reloj de sistema** (de él saldría `millis()`) | los plazos: `SFTY-6`, watchdog, cómputo de 48 h | 🟢 **montado y sano hasta donde se sabe** — pero ⚠️ **NUNCA SE HA ARRANCADO** |
>
> 🔴 **`D-22` NO ES «reparar el reloj». No arregla `Y2`, no pone la hora y no resucita el RTC.** Si
> usted vino aquí porque el reloj no marca, **`D-22` no es su respuesta** — siga con `D-20` y con el
> `DS3231` del ESP32.
>
> ⚠️ **Y la precondición, que es medible y no opcional: `Y1` nunca se ha seleccionado, y su gemelo
> de la misma placa está muerto.** Que esté soldado no es una medida de que oscile. **Antes de
> cambiar nada de reloj de sistema hay que comprobar que `Y1` arranca** —prueba nueva en
> `2_Manual_Hardware_y_Pruebas.md` §5, con el motivo medido de por qué un fallo ahí deja la tarjeta
> **a oscuras y sin reiniciar**—. **No se toca `Y1` ni sus condensadores en esta visita.**

El diseño de la tarjeta controladora **ya incluye el cristal `Y2` de 32.768 kHz** ruteado a los pines `PC14` y `PC15` del microcontrolador STM32F103C8T6.

Para mantener la hora y fecha exactas durante cortes de energía o traslados en bodega, el microcontrolador conmuta automáticamente al dominio de batería **`VBAT` (Pin 1)**.

```text
 ┌─────────────────────────────────────────────────────────────────────────────┐
 │                 ARQUITECTURA DEL RELOJ EN LA TARJETA MADRE                  │
 ├─────────────────────────────────────────────────────────────────────────────┤
 │                                                                             │
 │    CRISTAL Y2 (32.768 kHz) ──► Pines PC14 / PC15 del STM32 (Oscilador LSE)  │
 │                                                                             │
 │    PORTAPILAS CR2032 (3V)  ──► Pad VBAT (Pin 1 de U1) tras desoldar R5      │
 │                                                                             │
 │    DIAGNOSTICO POR LA APP  ──► $EVENT,ORIGEN:RELOJ  (pestana "Eventos")      │
 │      La pantalla "CONSULTA RELOJ" YA NO SE PUEDE ABRIR.  Ver apartado 4.     │
 │                                                                             │
 └─────────────────────────────────────────────────────────────────────────────┘
```

---

## 2. ⚠️ ADVERTENCIA CRÍTICA: Desoldar Obligatoriamente `R5` antes de Colocar la Pila

En la PCB de fábrica, `R5` es una resistencia puente de **0 Ω** que une el pin `VBAT` con la línea principal de **3.3V**.

> ### 🛑 PELIGRO DE SOBRECALENTAMIENTO Y EXPLOSIÓN:
> Si se conecta una pila **CR2032 (no recargable)** sin retirar previamente la resistencia `R5`, la fuente de 3.3V inyectará corriente continua a la pila cuando el semáforo esté encendido.
> La pila se calentará, se hinchará y **puede reventar dentro del gabinete**.

```text
       ESTADO DE FÁBRICA (SIN PILA)                ESTADO MODIFICADO (CON PILA CR2032)
   ┌───────────────────────────────────┐       ┌───────────────────────────────────┐
   │                                   │       │                                   │
   │   3.3V ───[ R5: 0 Ω ]───► VBAT   │       │   3.3V ───[ X R5 RETIRADA X ]     │
   │                           (Pin 1) │       │                                   │
   │                                   │       │   (+) Pila CR2032 ──► Pad VBAT    │
   │                                   │       │   (-) Pila CR2032 ──► GND         │
   └───────────────────────────────────┘       └───────────────────────────────────┘
```

---

## 3. Procedimiento de Instalación Paso a Paso

### Paso 1: Comprobación con Multímetro
1. Con la tarjeta **totalmente desenergizada**, poner el multímetro en modo continuidad (pito).
2. Colocar una punta en el **Pin 1 de U1 (VBAT)** y la otra en el punto de **3.3V**.
3. Debe pitar continuo (confirmando que `R5` está presente).

### Paso 2: Retiro de `R5` (Desoldadura)
1. Con cautín a 350°C y malla desoldadora o pinzas finas, **desoldar y retirar la resistencia SMD `R5`**.
2. Volver a medir continuidad entre Pin 1 (`VBAT`) y 3.3V: **NO debe pitar**.

### Paso 3: Soldadura del Portapilas CR2032
1. **Cable Positivo (Rojo `+`):** Soldar al pad de `R5` que conecta directamente con `VBAT` (Pin 1).
2. **Cable Negativo (Negro `-`):** Soldar a cualquier punto de masa `GND` confiable:
   * Aleta metálica del regulador `U4` (LM7805).
   * Pin central `GND` del regulador `U4`.
   * Pin `GND` de las borneras RS-485.

### Paso 4: Inserción de la Batería
* Insertar una pila botón **CR2032 de 3.0V (no recargable)** de marca reconocida (Panasonic, Maxell, Sony).

---

## 4. Verificación de Funcionamiento — **HOY SE LEE POR LA APP, NO POR LA PANTALLA**

> # 🛑 `CONSULTA RELOJ` YA NO SE PUEDE ABRIR. No lo intente.
>
> La pantalla **se sigue dibujando**, pero **no se puede llegar a ella**: `CONSULTA RELOJ` vive
> dentro de `CONFIGURACION`, y para entrar hacen falta **dos pulsaciones de *Aceptar*** —una para
> bajar de nivel y otra para entrar en la opción (`Maestro/src/menu.cpp:111`, `:129`)—. Los dos
> pulsadores que lo hacían **ya no existen**:
>
> ```
>   Maestro/src/botones.cpp:305-306   bool botonAceptar()  { return false; }
>                                     bool botonCancelar(){ return false; }
> ```
>
> `J16` p10 y p12 pasaron a ser entradas de cámara el 31/08 —`CAM_C_PIN` y `CAM_D_PIN`—. **La puerta
> está tapiada por los dos lados**, y por Bluetooth no existe ningún comando que abra esa pantalla.

> ## 🕐 Y DESDE EL 04–05/09, LA HORA QUE SE VE EN LA APP **NO SALE DE ESTE RELOJ** (`N-145`)
>
> **Este manual describe el RTC de la placa STM32.** Ese reloj **sigue parado** —el cristal `Y2` está
> confirmado muerto (`N-17`)— y ~~`SET_RTC` contra el STM32 sigue contestando
> `$ERR,…,DESC:SIN_CRISTAL_VEA_CONSULTA_RELOJ`. **Eso no ha cambiado.**~~
>
> > 🔴 **TACHADO EL 07/09 — `D-15`: el STM32 ya NO CONTESTA NADA a `SET_RTC`.** Consume la orden en
> > silencio para no dar un segundo acuse a una sola orden. `SIN_CRISTAL_VEA_CONSULTA_RELOJ` **sólo
> > sobrevive en dos comentarios** (`grep` en §4.1). Lo que sí sigue sin cambiar es lo importante:
> > **el `Y2` está muerto y este reloj no cuenta.**
>
> Lo que ha cambiado es **de dónde sale la hora que el operario ve**: el STM32 publica un **hueco
> honesto** (`HORA:--:--:--`) y **el módulo `ESP32` lo rellena al pasar la trama**, con la hora de
> **su propio `DS3231`** —el de la línea `A6`, colgado de `GPIO21`/`GPIO22`— y recalculando el
> checksum.
>
> | | |
> |---|---|
> | **El `DS3231` de A6 no arbitra** | si algún día el STM32 pone una hora de verdad, el módulo **no la toca**: sólo rellena el hueco. El apaño **se apaga solo** cuando deje de hacer falta |
> | **Nunca inventa** | bus mudo, oscilador parado (`OSF`), modo 12 h o registros incoherentes → **el hueco sale como está** |
> | 🛑 **Y por eso, SIN `DS3231` conectado la hora sigue saliendo en blanco** | **eso NO es una avería del apaño: es el apaño negándose a mentir.** Un `DS3231` sin pila entrega una fecha **perfectamente formada y falsa**, que es lo que costó `N-144` |
>
> 🔴 ~~**Nada de esto se ha probado sobre un `DS3231` real: el módulo NO ESTÁ COMPRADO (`A6`)** …
> **`N-145` no se puede dar por probada.**~~
>
> 🛑 **TACHADO EL 07/09 — `DECISIONES.md` `A-5`, resuelta el 05/09 a las 14:12 (`08c9d36`):**
> *«cada `ESP32` tiene un reloj y pila, ya te lo indiqué»*. **Hay DOS `DS3231`, uno por poste, y
> están puestos**; estaba escrito desde el 28/08 en la propia lista de compras. **Consecuencia
> directa: el `HORA:22:19:58` de la cinta del banco ES REAL y `N-145` queda CONFIRMADA EN COBRE.**
> 🔴 **Lo único que sigue `SIN VERIFICAR` es la dirección I²C `0x68` sobre el módulo real.**
>
> ⚠️ **Y el residual, escrito en vez de disimulado: desde la telemetría sola, la app NO PUEDE SABER
> cuál de los dos relojes selló la hora.** Hoy siempre es el del módulo, porque el otro no existe,
> pero **la trama no lo dice**.

### 4.1 ~~✅ Cómo se leen HOY esos mismos bits: por Bluetooth~~

> # 🔴 07/09/2026 — ESTE PROCEDIMIENTO NO SE PUEDE EJECUTAR. **NO MANDE `SET_RTC` AL STM32 PARA DIAGNOSTICAR.**
>
> **`DECISIONES.md` `D-15` + medida sobre el fuente.** El paso 1 de abajo manda `SET_RTC` al STM32 y
> espera un `$ERR` **que esa punta ya no emite**:
>
> ```
> $ grep -rn "SIN_CRISTAL_VEA_CONSULTA_RELOJ" 01_Firmware/Maestro/src 01_Firmware/Maestro/include
> Maestro/src/bluetooth.cpp:367   <- COMENTARIO
> Maestro/include/bluetooth.h:32  <- COMENTARIO
> ```
>
> **Cero emisiones.** La rama `SET_RTC` del Maestro **consume la orden en silencio** —comentario
> literal: *«D-15 — ESTA PUNTA YA NO PONE LA HORA, Y POR ESO NO CONTESTA A LA ORDEN»*— para que no
> caiga en `$ERR,CMD:DESCONOCIDO` y el operario reciba **dos acuses a una sola orden**.
>
> 🛑 **QUÉ LE PASA AL TÉCNICO QUE SIGA LOS TRES PASOS DE ABAJO: no ve el `$ERR`, no ve el `$EVENT`,
> no ve nada** — y concluye *«el cristal `Y2` está muerto»* o *«el equipo no responde»*. **Eso es un
> cambio de pila y de cristal SANOS.** El silencio no es una avería: es `D-15`.
>
> ## ✅ Lo que sí se puede hacer hoy
>
> | quiero… | orden | quién contesta |
> |---|---|---|
> | **saber la hora del cruce** | **`CMD:LEER_RTC`** (`D-17`) — ⚠️ **sin `CMD:PIN:1234:` delante**, el puente compara la línea entera con `strcmp` | el **ESP32**: `$ACK,NODE:PUENTE,CMD:LEER_RTC,RESULT:OK,FECHA:…,HORA:…` o un `$ERR` **por causa** (`OSCILADOR_PARADO_CAMBIE_PILA`, `NUNCA_SE_PUSO_PONGA_LA_HORA`, `SIN_RELOJ_NO_RESPONDE`, …) |
> | **poner la hora** | `CMD:PIN:1234:SET_RTC:…` | el **ESP32**, con `NODE:PUENTE` en el acuse |
>
> 🔵 **Y la pregunta de fondo de este manual ya no la decide `Y2`:** la hora del cruce la lleva el
> **`DS3231` con pila del ESP32 de cada poste** (`D-9`). El RTC interno del STM32 **ya no participa**,
> así que su diagnóstico dejó de estar en el camino crítico.
>
> ## 🟠 LO QUE SÍ SE PERDIÓ, Y ES UNA PREGUNTA PARA EL RESPONSABLE, NO UN ARREGLO
>
> Los **seis bits** (`ON` / `RDY` / `BYP` / `SEL` / `EN` / `CNT`) siguen existiendo:
> `reportarBitsDelReloj()` está vivo en `Maestro/src/bluetooth.cpp` — pero medido el 07/09 le queda
> **UN solo llamador**, y es la rama de **`CMD:PIN:1234:REINICIAR_RELOJ`**.
>
> 🛑 **`REINICIAR_RELOJ` NO ES UNA CONSULTA: BORRA LA HORA Y TODO EL RESPALDO** —ciclo acordado,
> marca de sincronización e indicador del Degradado—. **No se manda para mirar unos bits.**
>
> **O sea: hoy no hay forma NO destructiva de leer esos seis bits.** ¿Hace falta una, o se declara
> que el diagnóstico del `Y2` del STM32 ya no interesa porque el reloj vive en el ESP32? **Lo decide
> el responsable.**
>
> ---
>
> ~~**Lo que decía este apartado, conservado tachado:**~~

~~**Los mismos seis bits que pintaba la pantalla salen ahora en una trama de evento.** No hay que
pedirla con un comando nuevo: **el equipo la manda sola, justo detrás del rechazo**, que es cuando
hay alguien mirando.~~

1. ~~Conéctese al equipo con la app y **mande la hora**: `CMD:PIN:1234:SET_RTC:…`~~
2. ~~Si el equipo la rechaza, contesta **primero** el error y **detrás** los bits:~~

```text
   ~~$ERR,CMD:SET_RTC,DESC:SIN_CRISTAL_VEA_CONSULTA_RELOJ~~
   ~~$EVENT,NODE:MAESTRO,ORIGEN:RELOJ,DETALLE:ON:1 RDY:0 BYP:0 SEL:1 EN:1 CNT:0,HORA:--:--:--~~
```

3. ~~**Léalo en la pestaña `Eventos`** de la app. Es la pestaña 2 y la ven los dos roles.~~

*(~~Re-medido el 05/09: `Maestro/src/bluetooth.cpp:392-421` compone el detalle y `:708` / `:761` lo
emiten, detrás de `SIN_CRISTAL_VEA_CONSULTA_RELOJ` y de `SIGUE_PARADO_VEA_CONSULTA_RELOJ`.~~ 🔴
**Re-medido el 07/09: de esos dos llamadores queda UNO, y es el de `REINICIAR_RELOJ`.** Y las citas
por línea ya se habían renumerado a mano una vez —`:305-333` → `:392-421`— que es justo la cura que
`CLAUDE.md` §4.sexies desaconseja: hoy `reportarBitsDelReloj()` está en la `409`. **Se cita el
símbolo.**)*

> 🛑 ~~**El mismo camino existe en el Esclavo.**~~ **NO EXISTE, Y ESTE APARTADO NO SE PUEDE
> EJECUTAR EN EL POSTE DEL ESCLAVO. Medido el 05/09 con tres patrones, porque un «no aparece»
> no es un hallazgo hasta descartar al buscador (`CLAUDE.md` §4):**
>
> ```
> grep -rn "reportarBitsDelReloj" Esclavo/src Esclavo/include        -> VACIO
> grep -rn "ORIGEN:RELOJ|RelojDiag|lseOn|VEA_CONSULTA_RELOJ" Esclavo/ -> 1 sola linea, y es
>                                    Esclavo/src/bluetooth.cpp:221   un COMENTARIO, no codigo
> grep -n '"\$ERR,CMD:SET_RTC' Esclavo/src/bluetooth.cpp:672
>       enviarTramaConCrc("$ERR,CMD:SET_RTC,DESC:SIN_CRISTAL");      <- a secas
>
> y en el Maestro si esta:
> Maestro/src/bluetooth.cpp:392  static void reportarBitsDelReloj() {
> Maestro/src/bluetooth.cpp:708  reportarBitsDelReloj();
> Maestro/src/bluetooth.cpp:761  reportarBitsDelReloj();
> ```
>
> **Qué le pasa al técnico que siga estos tres pasos en un Esclavo:** el equipo contesta
> `$ERR,CMD:SET_RTC,DESC:SIN_CRISTAL` —**sin** el sufijo `_VEA_CONSULTA_RELOJ`, o sea que ni
> siquiera nombra la consulta— y **no emite ningún `$EVENT,ORIGEN:RELOJ` detrás**. En la pestaña
> `Eventos` **no aparece nada**. Repetirá el comando, no verá bits, y concluirá que fallan la app
> o el enlace. **Este procedimiento funciona en la mitad de los equipos.**
>
> **Es `CLAUDE.md` §2.ter en su forma más cara: la frase «el mismo camino existe en el Esclavo»
> era una afirmación sobre el código que nadie comprobó**, y encima iba dentro de un paréntesis
> que empieza con la palabra *«Medido»*. Las dos mitades de la frase tenían distinto valor de
> verdad y se publicaron con la misma etiqueta.
>
> **Qué hacer mientras esto no se cierre:** 🔴 **el diagnóstico del reloj se toma en el poste del
> MAESTRO.** Un `SET_RTC` rechazado en el Esclavo **no dice nada** sobre el estado de su `Y2` — y
> eso es exactamente `BLQ-2` (§5.4), el cristal de la segunda tarjeta sin diagnosticar: hoy **no
> hay instrumento para diagnosticarlo por Bluetooth**.
>
> 🟠 **Decisión pendiente, y NO es de este manual: ¿se porta `reportarBitsDelReloj()` al Esclavo**
> —unas 30 líneas, con el Esclavo al 66,3 % de flash (acta del 05/09)— **o se documenta que el
> diagnóstico del reloj es sólo del Maestro? Dueño: el responsable.**

> ⚠️ **`CNT:--` no es `CNT:0`, y la diferencia es el diagnóstico.** `--` significa *no se pudo leer
> el contador* —el periférico no tiene reloj y leerlo sería un fallo de bus—; `0` significa *se leyó
> y vale cero*. Un cero en lugar de los guiones haría indistinguibles dos averías que mandan a
> sitios opuestos.
>
> 💡 ~~**Para saber si el contador AVANZA hacen falta dos lecturas.** Repita el mismo `SET_RTC` al
> cabo de unos segundos y compare el `CNT`: si cambia, el RTC cuenta.~~
> 🛑 **ANULADO EL 07/09 — ESTE CONSEJO NO SE PUEDE EJECUTAR, Y ES EL MISMO CAMINO QUE §4.1 ACABA DE
> DEROGAR:** el STM32 **ya no contesta a `SET_RTC`** (`D-15`), así que ni sale el `$ERR` ni sale el
> `$EVENT` con los bits detrás. **Repetirlo no da dos lecturas: no da ninguna.** La idea sigue siendo
> buena —dos lecturas separadas para ver si el contador avanza— **pero hoy la orden que sirve es
> `CMD:LEER_RTC`, y lee el `DS3231` del `ESP32`, no el `Y2` del STM32.**

### 4.2 ~~Verificación con la pantalla LCD~~ ⛔ NO EJECUTABLE

*Se conserva porque describe qué significa cada diagnóstico, y ese significado sigue valiendo — es
lo que hoy dicen los bits `ON` / `RDY` / `BYP` de la trama de arriba.*

~~En el menú del semáforo, ingresar a **`CONFIGURACION` ➔ `CONSULTA RELOJ`**~~ (`lcd.cpp:488`):

```text
 ┌──────────────────────────────────────┐
 │ CONSULTA RELOJ                       │
 │                                      │
 │ Estado:  Oscilando OK                │
 │ Registro: 18:25:00                   │
 │ Contador: En marcha (+1s)            │
 └──────────────────────────────────────┘
```

### Tabla de Diagnósticos Oficiales del Firmware:

| Mensaje en Pantalla LCD | Causa Técnica | Acción Requerida |
|---|---|---|
| **`Oscilando OK / En hora`** | El oscilador `Y2` y la pila operan perfectamente. | Ninguna. Sistema listo para operación. |
| **`Pedido, no oscila`** | Los condensadores de carga $C_1/C_2$ del cristal no resuenan. | Reemplazar $C_1/C_2$ por 6–10 pF C0G/NP0. ~~o aplicar Plan B (DS3231)~~ ⛔ **el «Plan B» de este manual está ANULADO: no se cablea nada a `PB0`/`PB8`.** Ver apartado 5 |
| **`Parado / Sin bateria`** | Pila agotada o $R_5$ no retirado correctamente. | Medir voltaje en Pin 1 (`VBAT` debe ser > 2.8V). |

> ✅ **RESUELTO EL 01/09 — el hueco que este apartado dejaba abierto ya tiene instrumento.**
>
> La versión anterior decía que *«este procedimiento funciona tal cual mientras la LCD siga
> montada»*, y avisaba de que el día que se retirara la pantalla el apartado se quedaría sin
> instrumento. **Ese día llegó antes de lo previsto y por otro camino:** la pantalla sigue montada y
> el firmware la sigue dibujando (`Maestro/src/main.cpp:46`, `lcd.cpp:483-500`), pero **`CONSULTA
> RELOJ` dejó de ser alcanzable** cuando los pulsadores 3 y 4 pasaron a ser cámaras.
>
> El instrumento nuevo es el `$EVENT,ORIGEN:RELOJ` del apartado 4.1. **No se decidió una pantalla
> nueva: se mandaron los mismos bits por el canal que ya tenía interfaz construida.**

---

## 5. ⛔ ANULADO — «Plan B» del DS3231 sobre `PB0`/`PB8` en el STM32

> ## 🛑 ESTE APARTADO NO DESCRIBE UN MONTAJE DISPONIBLE. DESCRIBE UN CABLEADO QUE NO SE HACE.
>
> **Dos motivos, y el segundo es el grave:**
>
> 1. **La decisión cambió:** el `DS3231` ya no va en el STM32. Cuelga del **ESP32** (§5.2).
> 2. **Y aunque no hubiera cambiado, el cableado de abajo era ERRÓNEO desde el principio:** `PB0` y
>    `PB8` **no están libres** —lo están en este manual, no en la placa—. Ver el aviso de cabecera.
>
> **No se borra**, porque hubo una versión de este manual —**todas, hasta el 31/08**— que lo mandaba
> ejecutar, y porque una vía descartada que desaparece en silencio se vuelve a proponer.

### 5.1 ~~El montaje que este manual mandaba hacer~~ — CONSERVADO SOLO COMO RASTRO

~~Si la tarjeta posee un microcontrolador clonado cuyo oscilador interno `Y2` no lograse oscilar
(`Pedido, no oscila`), se conecta un módulo **DS3231 TCXO externo** a los **dos únicos pines libres de
la placa (`PB0` y `PB8`)**:~~

```text
   ##############  DIAGRAMA ANULADO -- NO EJECUTAR ESTE CABLEADO  ##############

       MODULO RTC DS3231 (EXTERNO)                   TARJETA SEMAFORO STM32
   ┌────────────────────────────────────┐         ┌───────────────────────────────┐
   │  [ VCC ]  (Alimentacion 3.3V) ─────┼─────────┼──► Pin 3.3V                   │
   │  [ GND ]  (Tierra / Masa)    ──────┼─────────┼──► Pin GND (Tierra comun)     │
   │  [ SDA ]  (Datos I2C)        ──X───┼─X───────┼─X► Pin PB0   <-- OCUPADO:     │
   │                                    │         │      CAM_DEMANDA_PIN (J14)    │
   │  [ SCL ]  (Reloj I2C)        ──X───┼─X───────┼─X► Pin PB8   <-- OCUPADO:     │
   │                                    │         │      LED_TESTIGO (R16 -> D5)  │
   └────────────────────────────────────┘         └───────────────────────────────┘

   ##############  ANULADO EL 28/08 (decision) Y CORREGIDO EL 31/08 (error)  #####
```

**Por qué el error no se veía:** el manual llamaba `PB0`/`PB8` *«los dos únicos pines libres»* y
**tenía razón en su momento** — la propuesta salió de `N-37` (`roadmap.md:4559`), cuando el cristal `Y2`
se declaró muerto y esos dos pines aún no tenían función. **`V9.0` se los dio a las cámaras**, y la
frase se quedó escrita describiendo una placa que ya no existía. Es exactamente lo que
`CLAUDE.md` §3.bis advierte de los comentarios: no fallan cuando alguien cambia un número, **se quedan
con la autoridad de una cuenta hecha**. El choque quedó anotado como `N-57` (`roadmap.md:2417`):
*«`PB0`/`PB8` están asignados dos veces, y el reloj llegó primero»*.

### 5.2 ✅ Dónde vive HOY el reloj externo: colgado del ESP32, fuera de la tarjeta

📖 **LEÍDO en los documentos de decisión, no medido en hardware** (no hay hardware que medir todavía):

| dato | fuente |
|---|---|
| El `DS3231` cuelga del **ESP32** por I²C: **`GPIO21` = SDA, `GPIO22` = SCL**, con **pila propia**. El módulo `ZS-042` trae sus *pull-ups* | `05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md` §1.3 (línea 89) |
| «El ESP32 es un módulo de expansión colgado de un puerto serie, y **no manda sobre las luces**» | `ESTADO.md:80` |
| La fila `PIN-0` —*«`PB0`/`PB8` van a bus I²C»*— está **⛔ ANULADA**: *«el I²C ya no vive en el STM32 […] `PB0` se queda como cámara de demanda»* | `ESTADO.md:124` |
| ~~El módulo es la línea de compra **`A6`**, y **NO se compró** todavía~~ ⛔ **CADUCADO EL 07/09: `A6` está CUBIERTA — SON DOS Y ESTÁN PUESTOS**, uno por `ESP32` (`A-5`, 05/09) | `05_Funcional/15_Lista_de_Compras_Hardware.md`, línea `A6` *(se cita la línea de la tabla, no el número de renglón: éste ya caducó una vez)* |

```text
 ┌─────────────────────────────────────────────────────────────────────────────┐
 │        DONDE VA EL DS3231 HOY  --  FUERA DE LA TARJETA DEL SEMAFORO         │
 ├─────────────────────────────────────────────────────────────────────────────┤
 │                                                                             │
 │   STM32F103  (gobierna el semaforo)          ESP32  (accesorio, no manda)   │
 │   ┌──────────────────────────┐               ┌──────────────────────────┐   │
 │   │  8 luces, radio, camaras │               │  DS3231  <-- GPIO21 SDA  │   │
 │   │                          │   J17 serie   │          <-- GPIO22 SCL  │   │
 │   │  PB7 (RX) p2  <──────────┼───────────────┼── GPIO17 (TX2)           │   │
 │   │  PB6 (TX) p3  ──────────►┼───────────────┼─► GPIO16 (RX2)           │   │
 │   │  GND      p7/p9 ─────────┼───────────────┼── GND  (masa comun)      │   │
 │   └──────────────────────────┘               └──────────────────────────┘   │
 │        alimentado por la tarjeta               FUENTE PROPIA desde 12 V      │
 │                                                (linea A5 -- NO PEDIDA)      │
 │                                                                             │
 │   9600 8N1.  El ESP32 NO cuelga del 3,3 V de J17 p6/p8: ese riel alimenta   │
 │   al STM32 que gobierna el cruce, y el accesorio no puede tumbar al que     │
 │   manda.  (Manual 10 §1, doc 17 §1.5.)                                      │
 └─────────────────────────────────────────────────────────────────────────────┘
```

* **Tipo de batería en el módulo `DS3231`:** si el módulo incluye circuito de carga activo, usar
  **`LIR2032` (recargable)** o desoldar la resistencia de carga si se usa una `CR2032` estándar.
  *(Esto no cambia con la mudanza al ESP32: es una propiedad del módulo, no de dónde se enchufe.)*
* **Ojo con confundir las dos pilas de este manual:** la del apartado 3 es la **`CR2032` del `VBAT`
  del STM32**, y **nunca es recargable**. La del `DS3231` puede tener que serlo. Son pilas distintas
  en equipos distintos.

### 5.3 ~~🛑 Un `DS3231` conectado hoy se queda MUDO, y eso es lo esperado — no una avería~~ → 🟢 EL DRIVER EXISTE Y ESTÁ LLAMADO. Lo que falta es la PIEZA

> 🛑 **TODO ESTE APARTADO ERA FALSO Y SE TACHA EN VEZ DE BORRARSE — 05/09.** No estaba mal
> escrito: estaba **CADUCADO**. La medida de abajo es real, y era cierta **el 31/08**; el
> `ESP32_Expansion` se escribió después. Publicada sin fecha operativa al lado, una medida
> vieja se lee como el estado de hoy — y ésta le decía a quien está en el poste que el
> **síntoma esperado es el silencio**, que es justo lo que le haría dar por buena una avería.

> ~~**MEDIDO EL 31/08, con `grep` sobre todo `01_Firmware/`:**~~
>
> ```
> grep -rni "ds3231" 01_Firmware --include=*.cpp --include=*.h   ->  0 coincidencias     <-- CIERTO EL 31/08
> grep -rn  "Wire\.|#include <Wire" Maestro/src Esclavo/src      ->  0 coincidencias     <-- SIGUE CIERTO
> ```
>
> ~~**No hay driver de `DS3231` en ninguna punta del STM32.** Y el del ESP32 **tampoco existe**: el
> único fuente del ESP32 en el repositorio es `01_Firmware/Repetidor/src/main.cpp` (un solo fichero,
> 8.348 B) y **no menciona `DS3231`, ni `Wire`, ni `GPIO21`/`GPIO22`**.~~

> **RE-MEDIDO EL 05/09, el mismo comando sobre el mismo árbol:**
>
> ```
> grep -rni "ds3231" 01_Firmware --include=*.cpp --include=*.h | wc -l    ->  57 coincidencias
> wc -l 01_Firmware/ESP32_Expansion/src/reloj_ds3231.cpp                  ->  336 lineas
> wc -l 01_Firmware/ESP32_Expansion/include/reloj_ds3231.h                ->  141 lineas
> grep -n "Wire" 01_Firmware/ESP32_Expansion/src/reloj_ds3231.cpp         ->  #include <Wire.h> (:5)
> ```
>
> **Las dos mitades de aquella frase se caen por separado:**
>
> | lo que decía | lo que se mide hoy |
> |---|---|
> | *«el driver del ESP32 tampoco existe»* | **existe**: `ESP32_Expansion/src/reloj_ds3231.cpp`, 336 líneas |
> | *«el único fuente del ESP32 es `Repetidor/src/main.cpp`»* | **hay dos proyectos ESP32.** `Repetidor` (`platformio.ini`: `platform = espressif32`, `board = esp32dev`) **y** `ESP32_Expansion`, con **8 `.cpp` y 8 `.h`** |
> | *«no hay driver en ninguna punta del STM32»* | **sigue siendo cierto**, y es correcto: el `DS3231` cuelga del ESP32, no del STM32 (§5.2) |
>
> **Y no es un huérfano** —§2.ter de `CLAUDE.md`, declarar no es ejercer—: se censaron los
> llamadores uno a uno y el driver **está cableado al resto del firmware**.
>
> ```
> reloj_setup()    <- ESP32_Expansion/src/main.cpp:161
> reloj_revisar()  <- ESP32_Expansion/src/main.cpp:173
> reloj_leer()     <- ESP32_Expansion/src/despachador.cpp:96, :120 · src/puente.cpp:222
> reloj_ajustar()  <- ESP32_Expansion/src/despachador.cpp:41
> ```
>
> *(De paso, y anotado sin arreglarlo porque el firmware no es de este manual: `reloj_motivo()`
> y `reloj_rangoValido()` **no tienen llamador fuera de su propio `.cpp`**. No cambia nada de lo
> de arriba; queda escrito para que no se cuente como cobertura lo que nadie ejerce.)*

**Consecuencia para quien esté en el poste — CORREGIDA, y es la contraria de la que decía este
apartado:** ~~si alguien conecta un `DS3231` —donde sea— **hoy no pasa nada**. No hay software que lo
lea. **El síntoma esperado es el silencio**~~. Hoy **sí hay software que lo lee**, y está en el ESP32.
Lo que no hay es **la pieza**:

> 🔴 ~~**`A6` — el módulo `DS3231` NO ESTÁ COMPRADO** … **la pieza no**.~~
>
> 🛑 **TACHADO EL 07/09, Y ES LA TERCERA COPIA DE LA MISMA FRASE EN ESTE FICHERO — `A-5` (05/09):
> LOS DOS MÓDULOS ESTÁN PUESTOS**, uno por `ESP32`, con pila propia. **La pieza SÍ está**, el driver
> también, y **`N-145` está confirmada en cobre**: el `HORA:22:19:58` de la cinta del banco es real.
>
> 🔴 **Lo que SÍ sigue abierto, y no es lo mismo:** la dirección `0x68` está **`SIN VERIFICAR` sobre
> el módulo real** —es la del datasheet, y lo declara el propio fuente en
> `ESP32_Expansion/include/contrato.h`—, y **el camino completo no se ha ejercido en una sesión de
> banco con el enlace Bluetooth arriba**. *(Y sigue valiendo el reverso: si el módulo no entrega hora
> válida —pila, `OSF`, modo 12 h— el firmware publica `--:--:--` **a propósito**. Eso no es avería
> del apaño; el motivo se lee con `CMD:LEER_RTC`.)*

~~`05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md` §1.3 lo clasifica con las dos palabras
exactas: **«decidido, sin construir»**.~~ → 🔧 **Esa clasificación también caducó para el software.**
Hoy el reparto correcto es: **el driver, CONSTRUIDO y llamado · el módulo, SIN COMPRAR · la lectura
sobre chip real, SIN VERIFICAR.**

### 5.4 🟡 Lo que queda ABIERTO, y de quién es

**No lo resuelve este manual.** Se escribe aquí para que no se lea como cerrado:

| abierto | qué falta | dueño |
|---|---|---|
| **`BLQ-2` — el cristal `Y2` de la SEGUNDA tarjeta** | `N-17`/`N-37` midieron **una** tarjeta el 01/08 y el cristal **no oscila** (`roadmap.md:4559`, `4569`). **El otro sigue sin diagnosticar.** Ya **no decide la compra** —el `DS3231` va al ESP32 pase lo que pase—, pero **sí decide el firmware**: reparar el `Y2` (`C1`/`C2` a 6–10 pF C0G/NP0) o reloj de software disciplinado por el ESP32 | **Responsable** — `ESTADO.md:24`, `:261` (prueba de banco `B5`) |
| **Reloj de software colgado del accesorio** | Es la vía alternativa si el cristal no se repara, y **tiene un coste anotado**: cuelga el reloj del semáforo de un módulo accesorio, *«que es justo lo que la arquitectura del 28/08 separa»*, y **si el ESP32 no está, el reloj se va yendo sin que nadie lo vea** | **Responsable** — doc 17 §3.2 (líneas 516-525) |
| ~~**Compra `A6` (`DS3231`)~~ y `A5` (fuente propia del ESP32)** | ~~`A6` **no se ha comprado**~~ ⛔ **CADUCADO EL 07/09: `A6` está CUBIERTA — son DOS, uno por `ESP32`, con pila propia, confirmado por el responsable el 05/09** (`DECISIONES.md` `A-5`). Lo que sigue abierto es **`A5`: no se ha pedido y hace falta** — sin ella el ESP32 reinicia el STM32 que gobierna el semáforo | **Responsable** — `15_Lista_de_Compras_Hardware.md`, líneas `A5` y `A6` |
| ~~**Firmware del `DS3231` en el ESP32**~~ | 🔴 ~~No existe. Y va **detrás del watchdog**: el ESP32 de este proyecto no tiene ninguno, con precedente escrito de uno clavado tumbando el enlace el 31/07~~ **⛔ LAS DOS MITADES SON FALSAS, Y LA §5.3 DE ESTE MISMO MANUAL YA LO MEDÍA DESDE EL 05/09.** *(1)* **El firmware existe**: `ESP32_Expansion/src/reloj_ds3231.cpp`, con `reloj_setup()`, `reloj_revisar()`, `reloj_leer()` y `reloj_ajustar()` **todos con llamador** — censo en §5.3. *(2)* **El ESP32 de expansión SÍ lleva watchdog**, medido el 07/09: `ESP32_Expansion/src/vigilante.cpp` incluye `<esp_task_wdt.h>` y llama a `esp_task_wdt_init()`, `esp_task_wdt_add()` y `esp_task_wdt_reset()`. **El que no lo lleva es el ESP32 del REPETIDOR**, que es otro módulo y otro firmware —`grep -riE "wdt\|watchdog" Repetidor/src Repetidor/include` → **cero**, re-corrido el 07/09—. **Confundirlos manda a buscar la causa al poste equivocado** | ✅ **cerrado** — lo que sigue abierto es ejercerlo sobre un módulo real (`0x68` `SIN VERIFICAR`) |

---

## 6. 🛑 Nivel de prueba de este manual — no es un permiso para instalar

| lo que este manual afirma | nivel |
|---|---|
| `PB0` = cámara de demanda · `PB8` = `LED_TESTIGO` | ✅ **MEDIDO EN EL FUENTE** — símbolos `CAM_DEMANDA_PIN` y `LED_TESTIGO` de `pines.h`, las dos puntas |
| ~~No hay driver de `DS3231` ni I²C en ninguna punta, **ni en el ESP32**~~ | 🔴 **FILA RETIRADA EL 07/09 — ERA EL EJEMPLO DE LIBRO DE «`MEDIDO` CON FECHA CORRECTA Y CONTENIDO CADUCADO».** El `grep` del 31/08 era cierto **ese día y sobre las carpetas que miró**; el módulo `ESP32_Expansion` **entró el 31/08** (`d2427c2`), y la §5.3 de este mismo manual re-midió el 05/09: **57 coincidencias de `ds3231`, 336 líneas de driver y cuatro llamadores.** **La fecha no validaba nada: la frase lleva días siendo falsa con su `MEDIDO` delante.** ✅ **Lo que de la fila SIGUE siendo cierto: no hay driver de `DS3231` ni `Wire` en ninguna punta del STM32** — y es lo correcto (`D-9`) |
| La LCD y `CONSULTA RELOJ` siguen **dibujándose** en el firmware de hoy | ✅ **MEDIDO EN EL FUENTE** — símbolos `lcd_setup()` y `lcd_dibujarConsultaReloj()`. ⚠️ **07/09: `D-17.bis` retira la pantalla del EQUIPO** (no del código), así que dibujarse ya no significa que alguien pueda verlo |
| …pero **`CONSULTA RELOJ` ya no es alcanzable**: necesita dos `botonAceptar()`, que devuelve `false` | ✅ **RE-MEDIDO el 07/09** — símbolos `botonAceptar()` / `botonCancelar()` en `Maestro/src/botones.cpp` y `Esclavo/src/botones.cpp`, los dos `return false;`. *(Las citas por línea de esta fila —`:305-306` / `:316-317`— **están caducadas**: hoy son `:672-673` y `:668-669`, y por eso se cita el símbolo.)* |
| ~~Los seis bits salen hoy por `$EVENT,ORIGEN:RELOJ`, detrás de los dos `$ERR` que nombran esa pantalla~~ | 🔴 **CADUCADO, Y LA §4.1 DE ESTE MANUAL YA LO DECÍA DESDE EL 07/09: los dos `$ERR` YA NO SE EMITEN** (`D-15`). A `reportarBitsDelReloj()` le queda **UN** llamador, y es `REINICIAR_RELOJ` — **que BORRA la hora y todo el respaldo**. **Hoy no hay forma NO destructiva de leer esos seis bits.** Ver §4.1 |
| El `DS3231` va al ESP32 por `GPIO21`/`GPIO22` | ~~📖 **LEÍDO** … **Sin construir y sin hardware que medir**~~ → 🟢 **07/09: CONSTRUIDO Y MONTADO.** Símbolos `DS3231_SDA` / `DS3231_SCL` en `ESP32_Expansion/include/contrato.h`, y **son dos módulos, uno por poste, con pila propia** (`A-5`, 05/09). 🔴 **La dirección `0x68` sigue `SIN VERIFICAR` sobre el módulo real** |
| Que retirar `R5` y montar la `CR2032` funcione en la tarjeta que usted tiene delante | 🔴 **NO VERIFICADO en esa tarjeta.** El procedimiento de los apartados 2-4 es el mismo desde el 26/08 y **la única medida de banco que existe es la del 01/08 sobre UNA tarjeta** |

> **Nada de esto ha pasado prueba de banco completa**, y este documento **no autoriza a instalar
> nada**. La única forma correcta de verificar el firmware es `01_Firmware/compuerta.py`, y un verde
> suyo **tampoco es un permiso**: dice que los modelos y los arneses de PC no encuentran nada, no que
> el firmware funcione en la tarjeta (`CLAUDE.md` §3).

---
*Manual técnico oficial de instalación de pila RTC V9.0. El «Plan B» del `DS3231` sobre `PB0`/`PB8` está ANULADO — apartado 5.*

---

## 🔴 HAY DOS RELOJES POR CRUCE — y quién manda ya está decidido (`D-20`, `D-26`) ~~, pero NO construido~~ y construido en `68dd2c5` (11/09), sin banco

> 🔵 **11/09 — `D-26`: LA HORA LA MANDA EL ESP32 DE CADA POSTE, y este apartado se corrige hacia
> ella.** Cada ESP32 le pasa la hora de su `DS3231` a su controladora al arrancar, justo después de
> ponerla y cada 5 minutos. **Con radio, el poste 2 hace caso a la del Maestro; SIN radio (25 s), toma
> la de su propio `DS3231`** —o la que el técnico le ponga desde el teléfono en su gabinete—. Por eso
> **a los dos postes se les pone la hora**, y **con la radio caída se va al poste 2 a ponérsela**. La
> prohibición de más abajo ya estaba tachada en la propia fila `D-20` el 07/09 por la noche. Qué ve y
> qué hace el técnico, con cada alarma: `14_Manual_App_Movil_IOT_VIAL.md` §5.3.bis.

Cada ESP32 lleva **su** DS3231 con **su** pila. **Hoy no existe ningún camino que los ponga de
acuerdo** *(11/09: sigue siendo cierto para los dos `DS3231` —la radio lleva la hora de controladora
a controladora, no escribe el `DS3231` del poste 2—)*: los dos ESP32 no se hablan —sin WiFi, sin ESP-NOW, sin radio: sus únicos objetos
de entrada/salida son su propio STM32 y el teléfono—, y la única sincronización horaria del
equipo va por LoRa **entre los dos STM32**, cuyos relojes están muertos (`Y2`, N-17).

> # 🔴 `D-20` (07/09) — LA AUTORIDAD DE LA HORA ES EL ESP32, Y ES UNA SOLA
>
> Fila **`D-20`** de [`DECISIONES.md`](../DECISIONES.md), decidida por el responsable el 07/09.
> Dice tres cosas, y la tercera es la que cambia este apartado:
>
> 1. **La autoridad de la hora es el ESP32, siempre y para todo. Al STM32 no se le pregunta
>    nunca.** La app se la da al **ESP32 Maestro**; ése al **ESP32 Esclavo**; y el STM32 de cada
>    punta la recibe **de su propio ESP32**.
> 2. **El Maestro manda la hora y el Esclavo hace caso** ~~**siempre**~~ *(con radio; sin radio, a
>    su propio ESP32 — `D-26` (3))*. ~~Hay **una sola fuente**, así que no hay desfase inicial que
>    acotar.~~
> 3. ~~🔴 **La app NO pone la hora en el poste 2. Nunca.** Un `SET_RTC` dirigido al Esclavo
>    **se rechaza**: no es una sincronización, **es una segunda fuente**.~~ 🛑 **Caducado** (tachado en
>    la propia fila `D-20` el 07/09 por la noche; `D-26` (5), 11/09): **al poste 2 se le pone la hora
>    desde el teléfono, y sin radio entra.**
>
> **La topología, porque explica el resto:** los dos ESP32 **no se hablan**, así que la hora
> viaja `ESP32-M -> STM32-M -> radio -> STM32-E` ~~`-> ESP32-E`~~ *(el último salto no existe: `D-26` lo
> deja como mejora)*. **Los STM32 quedan de CARTEROS de la hora, no de dueños.**
>
> ⚠️ ~~**Y AQUÍ ESTÁ LO QUE NO SE PUEDE LEER COMO HECHO: `D-20` está DECIDIDA Y SIN CONSTRUIR.**~~
> *(11/09: construida en `68dd2c5`, sin banco.)* Medido en el fuente el 07/09 *(crónica)*, y se puede reproducir:
>
> ```
> $ cd 01_Firmware/ESP32_Expansion
> $ grep -c "ESCLAVO\|Esclavo\|esclavo" src/despachador.cpp
> 0
> ```
>
> **El puente es el MISMO firmware en los dos postes, y el fichero que decide qué hacer con un
> `SET_RTC` no nombra al Esclavo ni una vez**: hoy lo atiende venga por donde venga.
>
> *(Y no vale decir «pero el puente sí sabe quién es»: lo aprende —`transporte_aprenderRotulo()`,
> con **un solo llamador**, `puente.cpp`— **para rotular el Bluetooth en la lista de Android**, y
> ese dato **no llega al despachador**. ~~Saber el rol para rechazar la orden es trabajo pendiente,
> no una propiedad del equipo que usted tiene delante.~~ *(11/09: no hace falta — con `D-26` no se
> rechaza nada; decide la controladora del poste 2.)*)*
>
> ~~Lo mismo con la siembra `ESP32 -> STM32`: el camino físico existe (`enlace_stm32.cpp`), **el
> mando que la siembra no**.~~ *(11/09: el mando existe desde `68dd2c5` — `CMD:HORA_ESP32`,
> `siembra.cpp`.)* Lo que falta está en `roadmap.md` §0 (filas 1.5, 1.13 y 1.14).

> # 🔴 07/09 — `DECISIONES.md` FILA `D-21`: **UNA PILA AGOTADA EN ESE `DS3231` YA NO ES SÓLO «SE PIERDE LA HORA». ES UN CRUCE QUE DEBERÍA IRSE A ÁMBAR**
>
> 🛑 **DECIDIDA Y SIN CONSTRUIR.** El equipo de hoy **no hace nada de esto**. Va aquí porque este es
> el manual de **la pila de ese reloj**, y desde `D-21` esa pila tiene una consecuencia vial que
> antes no tenía escrita en ningún sitio.
>
> **El escenario exacto, en palabras del responsable:** *«si la pila se apaga y queda en una hora
> fija de una fecha pasada… debería pasar a ámbar int., ¿no? Y lo mismo el Maestro»*.
>
> ⚠️ **Lo que hay que entender, y es lo contrario de lo que sugiere el sentido común: un `DS3231`
> sin pila NO se queda «sin hora». Se queda con UNA hora, perfectamente formada y falsa** — día,
> mes, hora, minuto y segundo, todos plausibles. **Ese es el caso peligroso**, porque el caso «sin
> hora» ya está cubierto (el Modo Degradado no entra) y éste **entra y reparte verdes**.
>
> **La respuesta decidida: la punta que tiene la hora mentirosa pasa a 🟡 ÁMBAR INTERMITENTE, y se
> publica** para que la app lo enseñe. `D-21` no inventa la detección —el `DS3231` levanta su bit
> **`OSF`** al pararse y el puente **ya lo lee y ya lo declara**, símbolo `reloj_ds3231.cpp`,
> regla `R-2`—: **lo que falta es que esa declaración llegue hasta las luces**.
>
> ✅ **Y medido el 07/09, porque cambia lo que cuesta: el MAESTRO YA TIENE LA REACCIÓN ESCRITA.**
> Dentro de su bucle de Degradado hay un `irAAmbar("Reloj no fiable", "Degradado detenido")` con su
> porqué al lado —*«el reloj puede dejar de ser fiable en marcha (pila agotada)»*—.
> `grep -n "irAAmbar(" 01_Firmware/Maestro/src/modo_degradado.cpp`. **El ESCLAVO no la tiene**:
> `grep -c "irAAmbar" 01_Firmware/Esclavo/src/modo_degradado.cpp` → **0**.
>
> 🔴 **Lo que falta, entonces, no es «construir el ámbar»: es que el `OSF` DE ESTE MÓDULO llegue a
> esa bandera.** ~~Hoy `reloj_enHora()` mira el **RTC del STM32** —cristal `Y2`, muerto—, **no este
> `DS3231`**. Son dos relojes distintos y no se hablan. **Y como esa bandera es falsa siempre, el
> ámbar del Maestro no se ejecuta nunca.**~~ *(11/09, `68dd2c5`: la bandera ya se enciende con la
> siembra de su ESP32; pero con el `OSF` puesto **el ESP32 no siembra** y la controladora **se queda con
> su hora y la bandera en `true`** — lo único que sale, a los 15 min, es
> `$ALARM …EVENTO:HORA_ESP32,CAUSA:SIN_HORA_DEL_ESP32…`. El ámbar por hora no fiable, `D-21` (1), está en
> construcción fuera de `main`.)* Así que para el técnico que está en el poste **la conclusión
> práctica no cambia: hoy el equipo avisa, pero no reacciona.**
>
> ### Lo que `D-21` cambia en el trabajo de este manual, HOY y sin firmware nuevo
>
> | | antes | desde `D-21` |
> |---|---|---|
> | la pila del `DS3231` | consumible: si se agota, se pierde la hora y se vuelve a poner | 🔴 **pieza de seguridad**: mientras esté agotada, esa punta **autoriza verdes con una hora parada** |
> | cuándo se cambia | cuando se note que falta la hora | **en visita programada, ANTES de que se agote, en los dos postes** |
> | qué se comprueba antes de autorizar un Degradado | nada en particular | **leer la hora de las dos puntas con `CMD:LEER_RTC` (`D-17`) y contrastarla con un reloj de fuera.** Si una devuelve una fecha que no es la de hoy, **no se entra en Degradado en ese cruce** |
>
> 🛑 **Y la asimetría que no se puede evitar y hay que conocer: en Modo Degradado NO HAY RADIO.**
> Cada punta decide sola, y lo normal es que se agote **una** pila: se puede ver **un poste en
> ámbar y el otro en verde**. Es el `Riesgo residual nº 3` de
> `8_Procedimiento_Modo_Degradado.md` §6 — variante nueva del `Riesgo 2`, **declarado sin solución
> técnica sin radio**.
>
> ⚠️ **Mientras tanto, el único detector es el técnico**, y por eso está escrito en un manual y no
> sólo en la decisión.

### Procedimiento — ~~HOY, hasta que `D-20` esté construida~~ el de `D-26` (11/09)

> ~~🛑 **Léase esto antes que los pasos.** `D-20` prohíbe poner la hora en el poste 2, **y la vía
> que la sustituye todavía no existe**. … Cuando `D-20` esté construida, el paso 4 desaparece.~~
> 🔵 **11/09 (`D-26`): el paso 4 NO desaparece — es el procedimiento.** Se ponen en hora **los dos**
> `DS3231`: el del poste 2 es el que su controladora usa cuando no hay radio.

1. Conéctese al **poste 1** —**el Maestro, y es el que manda la hora** (`D-20`)—. Espere a que
   el tablero identifique el equipo (`MAESTRO` o `ESCLAVO`): hasta entonces la app no sabe a
   cuál atribuir la lectura y **se niega a anotar**.
2. Mande la hora. El puente contesta con la hora **releída del chip**, no con la que usted
   mandó.
3. La app anota el poste y **avisa de cuál falta**.
4. ~~⚠️ **PASO TEMPORAL, y muere con `D-20`.**~~ Vaya al **poste 2** y repita **sin cerrar la app**.
   ~~*(Cuando el Maestro siembre la hora por radio, este paso no sólo sobra: el equipo lo
   rechazará, porque una segunda fuente es exactamente lo que `D-20` prohíbe.)*~~ *(11/09, `D-26`:
   no se rechaza. Con radio su controladora sigue con la del Maestro; la de este `DS3231` es la que
   tomará al perder la radio.)*
5. Con los dos vistos, la app publica el **desfase medido entre los dos relojes**. Es el
   único sitio donde ese número existe.

> # 🔴 EL POSTE 2 SE PONE EN HORA EN LA PUESTA EN MARCHA — ~~NO DURANTE LA AVERÍA~~ Y TAMBIÉN CON LA RADIO CAÍDA (`D-26`)
>
> ~~**Esto vale hoy y valdrá más todavía cuando `D-20` esté construida**, y es lo que nadie había
> escrito en ningún manual hasta el 07/09.~~
>
> ~~Con `D-20` dentro, **el único camino hacia el reloj del poste 2 pasa por el Maestro y por la
> radio**. Y la radio se cae justo cuando hace falta el Modo Degradado — que es el modo que
> **exige hora**. O sea: **con la radio muerta no se le puede poner en hora.**~~ 🔵 **11/09
> (`D-26`): con la radio muerta SÍ se le puede poner en hora** —su controladora toma la de su propio
> ESP32—, y **es lo que se hace cuando salta la alarma de radio**.
>
> ✅ **Su `DS3231` tiene pila y conserva la hora que ya tenía.** *Perder la radio no es perder la
> hora.* Por eso se pone también **antes**:
>
> - **En la puesta en marcha del cruce**, con los dos postes sanos y la radio viva.
> - **Después de cambiar la `CR2032`** de cualquiera de los dos módulos.
> - **Después de cualquier `$ERR ... DESC:OSCILADOR_PARADO_CAMBIE_PILA`** en el poste 2.
>
> ~~🛑 **Lo que NO se hace es subir al poste 2 con la avería encima a poner la hora.** Si se llega
> ahí, el fallo ya ocurrió antes: en la puesta en marcha.~~ *(Caducado por `D-26` (5): es justo lo
> que se hace.)*

⚠️ **`RESULT:HORA_PUESTA_SIN_PROPAGAR` no habla del otro poste.** Significa que la hora entró
en el DS3231 y la orden *(desde `D-26`: la línea `CMD:HORA_ESP32` que la lleva)* **no llegó a la
controladora de ese mismo armario** por el cable interno `J17`. El reloj está bien; lo que falla es
el enlace de dentro. Si se repite, la controladora lo dice sola a los 15 min: `$ALARM
…EVENTO:HORA_ESP32,CAUSA:J17_MUDO…` (revisar el circuito ESP32–controladora de ese gabinete).

⚠️ **Si la app se cierra entre poste y poste, el registro se pierde** y hay que rehacer los
dos. La app lo dirá —*«no consta en esta sesión»*— en vez de dar por bueno lo que no vio.


### 🔎 Y desde el 05/09 se puede CONSULTAR sin cambiar nada

1. Conéctese con la app y pulse **🔎 Consultar reloj (no cambia nada)**. Manda `CMD:LEER_RTC`,
   **sin PIN y sin escribir**: es la lectura que antes no existía — **hasta hoy había que
   CAMBIAR la hora para poder verla**.
   - `RESULT:OK` con `FECHA:`/`HORA:` → ésa es la hora que el DS3231 **tiene ahora mismo**.
   - `$ERR ... DESC:` → **no hay hora que enseñar**, y el motivo dice cuál de las ocho averías
     es. Tabla completa en el Manual 10, §4.1.
   - **`NODE:PUENTE` significa que contesta el ESP32, no el micro del semáforo.** El `Y2` del
     STM32 sigue muerto (N-17) y **ya no atiende `SET_RTC`** (D-15): una orden, un acuse.
2. **Repita la consulta en el OTRO poste.** Cada punta lleva **su propio DS3231 con pila** y
   nada los sincroniza, así que **un poste en hora no es un cruce en hora**. Con las dos
   lecturas en la misma sesión, la app publica en el registro **el desfase entre los dos
   relojes** — restando el error de cada uno contra el celular, de modo que el tiempo de la
   caminata no cuenta. **Ése es el único número que dice si el cruce está en hora, y hasta hoy
   no lo calculaba nadie.**

> ✅ **`D-20` NO deroga este apartado, y conviene decirlo para que nadie lo borre de paso.**
> ~~`D-20` prohíbe **ESCRIBIR** la hora en el poste 2 —eso sería una segunda fuente—;~~ **leerla no
> escribe nada**. `CMD:LEER_RTC` (`D-17`) se sigue mandando **a los dos postes**, ~~y con `D-20`
> construida vale **más** que hoy, no menos: es la única forma de comprobar que la siembra del
> Maestro llegó de verdad al reloj del poste 2.~~ *(11/09, `D-26`: la radio no escribe el `DS3231` del
> poste 2, así que el desfase entre los dos es **el salto que dará la hora del poste 2 al perder la
> radio**.)* **Mirar sigue siendo la única comprobación que hay.**
