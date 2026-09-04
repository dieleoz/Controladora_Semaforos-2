# 📷 MANUAL DE CONFIGURACIÓN Y PARAMETRIZACIÓN — CÁMARAS IA ACUSENSE PARA SEMÁFOROS MÓVILES (V9.0)

**Sistema:** Controladora de Semáforos Móviles de 3 Estados (Maestro y Esclavo V9.0)  
**Cámara Certificada:** Hikvision AcuSense Varifocal Motorizada (DS-2CD3643G2-LIZSU o equivalente)  
**Topología del Sistema:** Analítica Deep Learning Embebida (sin PC externo) + contacto seco a la tarjeta. **Tres entradas de cámara por punta:** la bornera **`J14`** (`PB0`, con antirrebote RC de 1 ms en la placa) y **`J16` p10 / p12** (`PB14`/`PB15`, **sin antirrebote de placa**, y con la medida **`M3` YA CERRADA EN BANCO** el 04/09: **se pueden cablear**)  
**Verificación Hardware:** Esquemáticos KiCad `Controladora_Semaforos.kicad_sch`, `pines.h` y `03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md`  
**Normativa Aplicable:** Manual de Señalización Vial de Colombia (Resolución 2024 - MinTransporte)  
**Fecha de Emisión:** 26 de Agosto de 2026  
**Última revisión:** 4 de septiembre de 2026 — 🟢 **`M3` ESTÁ CERRADA EN BANCO (paso 20): el
pull-down de `J16` p10 y p12 es REAL y de 10 kΩ.** Con eso **desaparece el único bloqueo que quedaba
para cablear cámara a `J16`**, y la **polaridad la decide el cobre: ACTIVO EN ALTO**. Y con el mismo
banco entra un aviso que **no se puede separar de este manual**: `J16` p1 lleva **12 V crudos** y las
cinco entradas de campo van **desnudas al pin del STM32** — ver el bloque 🛑 de la cabecera, que es
lo primero que se lee. Ver apartados 2.1, 4, 6 y 7.

*Revisión anterior (02/09/2026): **`J16` p10 y p12 ya son entradas de cámara en el firmware.** Este
manual las describía como los pulsadores* Aceptar *y* Cancelar *y decía que «ningún firmware los lee
todavía como cámara»; las dos cosas son falsas desde el 31/08. Y se corrigió un segundo punto:
**ninguna cámara de este equipo se configura con Detección de Cruce de Línea** — las tres entradas
son de demanda.*

*Revisión anterior (31/08/2026): corregida la POLARIDAD del cableado. Este manual mandaba llevar el
contacto seco a «`PB0` y `GND`». **La entrada es activa en ALTO**: contra masa la cámara no dispara
nunca. Nada se borra: el texto viejo queda tachado en su sitio con el motivo.*

---

> ## 🛑 ANTES DE ENCHUFAR NADA EN `J16`: TAPAR EL PIN 1, QUE LLEVA 12 V CRUDOS (04/09/2026)
>
> **Este aviso no se separa de este manual y no se resume.** Es lo único que hay entre un instalador
> y una avería que **ya ocurrió**: el 04/09 una tarjeta **Maestro quedó con un cortocircuito de
> 3,3 V a masa** durante la sesión de banco.
>
> ```text
>   J16 p1  ------------------------------->  12 V CRUDOS
>           el unico conector de senal de toda la tarjeta que los trae
>
>   Las 5 ENTRADAS DE CAMPO van DESNUDAS al pin del STM32:
>           sin resistencia en serie, sin optoacoplador, sin clamp
>
>   Las 9 SALIDAS de la placa si van protegidas:
>           220 Ohm + optoacoplador
> ```
>
> **La asimetría es el dato:** lo que sale de la tarjeta está protegido; **lo que entra, no**. Un
> roce de 12 V contra cualquiera de las cinco entradas llega directo a la patilla del micro que
> gobierna el semáforo.
>
> ### 🔧 Lo que se hace, en cada equipo, ANTES de cablear cámara
>
> 1. **Tapar físicamente el pin 1 de `J16`** —tapón, funda termorretráctil o el propio conector con
>    la posición 1 sin terminal—. **Es obligatorio**, no recomendado, y se hace **equipo por equipo**.
> 2. Sólo entonces se llevan los hilos a **p10 / p12**, con retorno por **p2 (`GND`)**.
> 3. **En el p1 no se conecta nada, nunca.**
>
> ⚠️ **Y sigue en pie la confusión que quema módulos:** `J16` y `J17` **comparten footprint y son
> idénticos a la vista**. Multímetro en la posición 1 contra masa **antes** de enchufar: **si da
> ≈ 12 V es `J16`** —ahí va la cámara, con el p1 tapado—; **si no, es `J17`** y ahí va el módulo.
>
> 🔴 **Lo que este aviso NO arregla:** tapar el pin protege del error de cableado, **no** de una
> sobretensión que entre por el hilo de campo. La protección de verdad —**2K2 en serie en las cinco
> entradas**— es una modificación de la **revisión V2 de la placa**, y está anotada como línea de
> compra en `15_Lista_de_Compras_Hardware.md` (bloque **E**). **Hoy no existe en el cobre.**

> ## 🛑 AVISO DE POLARIDAD — EL CONTACTO SECO **NO** VA CONTRA `GND` (31/08/2026)
>
> **MEDIDO EN EL FUENTE**, `01_Firmware/Maestro/src/botones.cpp` (idéntico en las dos puntas). El
> lector de cámara se mudó aquí el 31/08 desde `modo_inteligente.cpp`, porque una entrada física
> existe desde que arranca la tarjeta y no solo mientras un modo está puesto:
>
> ```
> :87   if (digitalRead(pin) == HIGH) {          // <-- ACTIVO EN ALTO
> :155  pinMode(CAM_DEMANDA_PIN, INPUT);         // <-- INPUT PELADO, sin pull-up
> :156  pinMode(CAM_C_PIN, INPUT);               //     las tres, en el arranque
> :157  pinMode(CAM_D_PIN, INPUT);
>
> // el porque, en pines.h:41-46 (N-67):
> //   PB0 lleva R64 de 10 kOhm A MASA -pull-DOWN- y C25 de 100 nF tambien a masa;
> //   la bornera J14 saca ese pin JUNTO A 3,3 V. O sea que el contacto seco de la
> //   camara va entre los dos bornes de J14 y CIERRA A 3,3 V.
> ```
>
> El reposo del pin **ya lo fija la placa** con `R64` de 10 kΩ a masa. El contacto de la cámara tiene
> que **cerrar el pin contra los 3,3 V** de la bornera, **no contra masa**.
>
> ### 🟢 04/09 — Y ahora la polaridad **NO** la decide el fuente: LA DECIDE EL COBRE, y está medido
>
> Hasta hoy esta regla se sostenía en `botones.cpp`. **El banco la midió en la placa** (paso 20):
>
> ```text
>   J16 p10  ->  9,93 kOhm a masa    y  0 V con la tarjeta energizada
>   J16 p12  ->  9,94 kOhm a masa    y  0 V con la tarjeta energizada
>   Los CUATRO pines de J16 son identicos:  10K a masa + 100 nF
>   Y los cuatro tienen 3,3 V en la posicion de al lado:  J16.4 / .7 / .9 / .11
> ```
>
> **Un pull-down real de 10 kΩ y 3,3 V en el borne contiguo describen un solo gesto:** cerrar el
> contacto seco **contra esos 3,3 V**. **ACTIVO EN ALTO**, y ya no por deducción del firmware —que
> además coincide: `INPUT` pelado, `== HIGH`—, sino **por lo que hay soldado en la placa**.
>

> **Qué pasaba si se seguía el texto viejo:** con el contacto entre el pin y `GND`, el pin está en
> `LOW` en reposo **y sigue en `LOW` al detectar**. **La cámara no dispara jamás** y no hay síntoma:
> el equipo se comporta como si no hubiera cámara. Un ensayo de taller lo daría por bueno.
>
> 🔴 **Y eso es exactamente lo que hacía peligroso al error hermano de `MANUAL_USUARIO.md`**, que
> mandaba las cámaras a `PB9`/`PB13` —**los dos canales del mando de relés**—: mientras el cableado
> estuviera *«a `GND`»* la cámara no disparaba y nadie veía nada; el día que alguien **«arreglara» el
> cableado**, tres pulsos de tráfico dentro de la ventana de 12 s empezarían a **componer secuencias
> del mando y a cambiar el modo del semáforo solos**. Los dos errores se corrigen juntos o el arreglo
> es peor que el defecto. Ver `MANUAL_USUARIO.md` §6.

---

> ### 🛑 LAS CÁMARAS 2 Y 4 NO ESTÁN OPERATIVAS EN V9.0
>
> **La cámara de umbral no está en V9.0, y no es solo firmware: no hay dónde conectarla.**
> Medido el 27/08 sobre el esquemático: **`PB8` alimenta el LED testigo `D5` a través de `R16`
> 1 kΩ** — no es una bornera ni una entrada optoacoplada. El manual lo daba por una entrada
> *«en reposo»*, y eso venía de leer un esquemático incompleto (`roadmap.md` N-64).
>
> Para tenerla harían falta **dos cosas**, y ninguna es un `pinMode`: **(1)** una entrada física
> —un hilo desde el pad de `PB8` retirando `R16`/`D5`, o desde uno de los cuatro pines sin
> cablear (`PA11`, `PA12`, `PA15`, `PC13`)— y **(2)** un **comando de radio** que lleve la cuenta
> del tramo al Maestro, que es quien decide (`roadmap.md` N-59). Sin el comando, leer el pin es
> medio camino.
>
> Mientras tanto el despeje se hace **por tiempo** (`cfgDespejeSeg`), que es el criterio
> conservador: la cámara de umbral daría **eficiencia**, no seguridad.


## 1. Arquitectura Autónoma para Semáforos Móviles (Sin Raspberry Pi ni Micro-PC)

La cámara Hikvision AcuSense incorpora un procesador de inteligencia artificial con **clasificador Deep Learning de vehículos vs. humanos**. No se requiere ningún computador externo, switch Ethernet ni direccionamiento IP en obra:

```text
 ┌─────────────────────────────────────────────────────────────────────────────┐
 │                ARQUITECTURA AUTÓNOMA DE DETECCIÓN VEHICULAR                 │
 ├─────────────────────────────────────────────────────────────────────────────┤
 │                                                                             │
 │   [ CÁMARA HIKVISION ACUSENSE (Analítica Embebida) ]                        │
 │           │                                                                 │
 │           ▼ (Detección por Intrusión: Clasificador ☑ Solo Vehículo)         │
 │   [ SALIDA DE ALARMA N/O (Bornes 1A / 1B - Contacto Seco) ]                 │
 │           │                                                                 │
 │           ▼ (2 Hilos directos por cámara)                                  │
 │   [ TARJETA CONTROLADORA STM32 - bornera J14, entrada PB0 ]                 │
 │           │                                                                 │
 │           ▼                                                                 │
 │   [ LÓGICA VIAL: Demanda Vehicular + Despeje Todo-Rojo cfgDespejeSeg ]      │
 │                                                                             │
 └─────────────────────────────────────────────────────────────────────────────┘
```

---

## 2. Asignación de Pines y Distribución de Señales

~~En cada semáforo móvil (Maestro y Esclavo), los relés de las cámaras se conectan a los **dos únicos pines libres** del microcontrolador:~~

⛔ **La frase *«los dos únicos pines libres»* se retira.** Es la formulación que ya costó tres
defectos en este proyecto (`N-59`, `N-67`, `N-105`): **«pin libre» no es una observación, es una
medida contra `pines.h`.** El reparto real es el de la tabla de abajo.

```text
 ┌─────────────────────────────────────────────────────────────────────────────┐
 │       MAPA DE CONEXION FISICA DE CAMARAS  --  AL DIA EL 04/09 (M3 CERRADA)  │
 ├─────────────────────────────────────────────────────────────────────────────┤
 │                                                                             │
 │ !!! J16 p1 LLEVA 12 V CRUDOS.  SE TAPA ANTES DE CABLEAR.  NO SE CONECTA. !!!│
 │                                                                             │
 │ • CAMARA DE DEMANDA, hoy:  Bornes 1A/1B --> J14: pin PB0 CONTRA LOS 3,3 V   │
 │   ~~PB0 y GND~~  <-- ANULADO: la entrada es ACTIVA EN ALTO, contra masa     │
 │                       no dispara nunca.  R64 10K a masa ya fija el reposo.  │
 │   - Detecta si hay vehiculos esperando paso en el carril.                   │
 │                                                                             │
 │ • CAMARAS C y D, EL FIRMWARE YA LAS LEE:  J16 p10 (PB14) / p12 (PB15)      │
 │   - Son camaras de DEMANDA, igual que la de J14. Piden paso, no miden nada. │
 │   ~~NO SE CABLEAN todavia: falta la medida M3~~ <-- M3 CERRADA EN BANCO     │
 │     el 04/09: 9,93 y 9,94 kOhm a masa, los dos a 0 V.  YA SE CABLEAN.       │
 │   - Contacto seco CONTRA LOS 3,3 V del borne de al lado:  p9 para p10,      │
 │     p11 para p12.  Retorno de masa por p2.  ACTIVO EN ALTO.                 │
 │                                                                             │
 │ • CAMARA 2 / 4 (Umbral):  NO EXISTE EN V9.0 - PB8 es un LED, no una entrada │
 │                                                                             │
 │ • PB9 (J16 p5) y PB13 (J16 p8):  MANDO DE RELES, canales A y B. SE CONSERVAN│
 │   >>> NUNCA UNA CAMARA AQUI: tres pulsos en 12 s componen una secuencia <<< │
 │   Sus 3,3 V contiguos son p4 y p7 (mismo cobre que p9/p11: 10K + 100nF).    │
 │                                                                             │
 └─────────────────────────────────────────────────────────────────────────────┘
```

### 2.1 El reparto real, pin por pin

| pin | qué es | ¿cámara? | nivel |
|---|---|---|---|
| **`PB0`** (`J14`) | `CAM_DEMANDA_PIN`, con `R64` 10 kΩ + `C25` 100 nF (antirrebote 1 ms) | ✅ **Sí** | ✅ **MEDIDO** (`pines.h:43-46`; declarada en `botones.cpp:155`; leída en `modo_inteligente.cpp:86`, `:124` y `Esclavo/src/main.cpp:350`) |
| **`PB8`** | `LED_TESTIGO` → `R16` 1 kΩ → LED `D5` | ❌ **No es entrada de nada** | ✅ **MEDIDO** (`pines.h:63`; `modo_inteligente.cpp:38` lo deja en alta impedancia) |
| **`PB9`** (`J16` p5) | `BOTON1` = **`MANDO_A`** del mando de relés | 🛑 **NUNCA.** `A·A·A` en 12 s = Modo Automático | ✅ **MEDIDO** (`pines.h:122`, `botones.cpp:139`, `mando.cpp:225-227`) |
| **`PB13`** (`J16` p8) | `BOTON2` = **`MANDO_B`** | 🛑 **NUNCA.** `B·B·B` = Ámbar **y arma `ambarLocal`**, que veta las órdenes de radio | ✅ **MEDIDO** (`pines.h:123`, `botones.cpp:140`, `mando.cpp:230-234`, `Esclavo/src/mando.cpp:132`) |
| **`PB14`** (`J16` **p10**) | **`CAM_C_PIN` — entrada de cámara de DEMANDA**, con **`R67` 10 kΩ a masa CONFIRMADA en cobre: 9,93 kΩ** | ✅ **Sí — firmware Y cobre** | ✅ **MEDIDO EN EL FUENTE** (`pines.h:124`; `pinMode(INPUT)` en `botones.cpp:156`; leída por flanco en `botones.cpp:126-133`) **y MEDIDO EN BANCO el 04/09** (`M3`, paso 20: 9,93 kΩ a masa, 0 V con energía) |
| **`PB15`** (`J16` **p12**) | **`CAM_D_PIN` — entrada de cámara de DEMANDA**, con **`R68` 10 kΩ a masa CONFIRMADA en cobre: 9,94 kΩ** | ✅ **Sí — firmware Y cobre** | ✅ **MEDIDO EN EL FUENTE** (`pines.h:125`; `pinMode(INPUT)` en `botones.cpp:157`) **y MEDIDO EN BANCO el 04/09** (`M3`, paso 20: 9,94 kΩ a masa, 0 V con energía) |

> ### ✅ QUÉ CAMBIÓ EL 31/08 EN ESTAS DOS FILAS — Y QUÉ NO
>
> **`PB14` y `PB15` YA NO SON PULSADORES.** Este manual decía que eran `botonAceptar()` y
> `botonCancelar()` y que *«ningún firmware los lee todavía como cámara»*. **Las dos cosas son
> falsas hoy:**
>
> ```
>   Maestro/src/botones.cpp:280-281   bool botonAceptar()  { return false; }
>                                     bool botonCancelar(){ return false; }
>   Maestro/src/botones.cpp:156-157   pinMode(CAM_C_PIN, INPUT);
>                                     pinMode(CAM_D_PIN, INPUT);
>   Maestro/src/botones.cpp:126-133   flanco de subida -> demanda_solicitar()
>   Esclavo/src/botones.cpp:294-295, :176-177, :147-154   identico
> ```
>
> **Son cámaras de DEMANDA**, exactamente como la de `J14`: piden paso. **No son cámaras de umbral**
> y no miden el despeje — el despeje sigue siendo por tiempo (`cfgDespejeSeg`).

> ## 🟢 04/09 — `M3` CERRADA EN BANCO: **`PB14`/`PB15` YA SE CABLEAN**
>
> ~~**AUN ASÍ, `PB14`/`PB15` NO SE CABLEAN TODAVÍA. Queda UN bloqueo, y es de cobre: falta la medida
> `M3` con óhmetro. `R67` y `R68` de 10 kΩ a masa sólo lo dice el netlist, y nadie lo ha medido en
> cobre.**~~ ⛔ **ANULADO: se midió el 04/09 (paso 20 de la sesión de banco) y el netlist tenía
> razón.** El texto viejo se conserva tachado porque **describía bien el riesgo** —un `INPUT` pelado
> sin resistencia real queda flotando y da demandas fantasma en un equipo que gobierna un cruce—; lo
> que ha cambiado es que **ese riesgo está descartado por medida, no por confianza**.
>
> ```text
>   M3, paso 20, tarjeta energizada y J16 vacio:
>     J16 p10 (PB14) ->  9,93 kOhm a masa   ->  0 V
>     J16 p12 (PB15) ->  9,94 kOhm a masa   ->  0 V
>   Los cuatro pines de J16 son identicos: 10K a masa + 100 nF,
>   y los cuatro tienen 3,3 V en la posicion contigua (J16.4 / .7 / .9 / .11).
> ```
>
> **Los tres resultados que `M3` podía dar están resueltos en el bueno:** hay resistencia, es de
> 10 kΩ, y es un **pull-DOWN**. Por tanto **el contacto seco cierra contra los 3,3 V del borne
> contiguo** —`p9` para `p10`, `p11` para `p12`—: **ACTIVO EN ALTO**, que es como el firmware ya lee.
>
> ✅ **Y el paso 21 cerró la otra mitad, que es la que se siente en la calle: NO hay falsa
> activación.** En reposo, **con el cable puesto y sin él**, el equipo **no pide paso por sí solo**.
> Era exactamente el criterio negativo del `ENSAYO 3`, ejercido sobre `J16`.
>
> 🛑 **Lo que NO se ha levantado con `M3`: `J16` p1 sigue llevando 12 V crudos, y taparlo sigue
> siendo obligatorio en cada equipo antes de cablear.** Ver el bloque de la cabecera. La separación
> real sobre cobre contra la red de 12 V es de **4,269 mm** en `p10` y de sólo **1,359 mm** en `p12`
> (**MEDIDO**, `03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md:576-588`): **si una de las dos cámaras es
> más crítica, va en `p10`.**
>
> ### 🪜 Y el orden sigue siendo ASIMÉTRICO — cargar firmware ANTES de tocar un hilo
>
> **Con el firmware nuevo dentro es seguro**: un pin en `INPUT` pelado no ejecuta nada.
> **Con el firmware viejo dentro NO lo es**: `PB14` sigue siendo `botonAceptar()` leído activo en
> BAJO, y cualquier hilo enchufado en `J16` p10 **pulsa *Aceptar* en un equipo que está en la
> calle**. **Exija la carga verificada en la tarjeta antes de que nadie enchufe nada** — un commit
> no protege de un destornillador.

---

## 3. Filosofía de Movilidad: Configuración ÚNICA en Taller

> ### 🛑 REGLA DE ORO PARA EQUIPOS MÓVILES:
> **La geometría no discrimina; discrimina el clasificador AcuSense.**  
> Al configurar una **zona de intrusión amplia (~90% de la pantalla)** con el filtro `☑ Vehículo`, el semáforo puede trasladarse de kilómetro en la vía **sin necesidad de conectar una laptop, ni reencuadrar el zoom, ni redibujar máscaras de píxeles**.

---

## 4. Procedimiento de Parametrización en Taller (Paso a Paso)

Se realiza **una sola vez en taller** antes de enviar las cámaras a campo:

```
[ PASO 1: Red y Acceso ] ──► [ PASO 2: Óptica y Luz ] ──► [ PASO 3: Analítica AcuSense ] ──► [ PASO 4: Salida Relé ]
```

### Paso 1: Acceso Inicial
1. Conectar la cámara por cable Ethernet a una laptop con el software *Hikvision SADP* o navegador web.
2. Asignar contraseña segura y fijar IP estática de taller (ej. `192.168.1.61` para Cámara 1, `.62` para Cámara 2, etc.).

### Paso 2: Ajuste Óptico y Nocturno
1. Ir a **Configuración > Imagen > Pantalla**:
   * **Zoom:** Ajustar en **Gran Angular Máximo (2.7 mm)** para obtener el campo de visión máximo ($102.4^\circ$).
   * **Enfoque:** Presionar **One-Touch Focus** (Autofoco).
   * **Precintar:** No volver a mover el zoom.
2. Ir a **Configuración > Imagen > Ajustes de Luz Suplementaria**:
   * **Modo de Luz:** Seleccionar **Solo IR (Infrarrojo 850 nm)**.
   * **Luz Blanca:** **DESACTIVADA** (evita deslumbrar de frente a los conductores en carretera).

### Paso 3: Configuración de la Analítica Inteligente

> 🔴 **TODAS LAS CÁMARAS DE ESTE EQUIPO SON DE DEMANDA. Configúrelas todas igual, con el bloque de
> abajo.** Las tres entradas que el firmware lee —`PB0` en `J14`, y `PB14`/`PB15` en `J16`— acaban
> en la misma llamada, `demanda_solicitar()`: **piden paso**. Ninguna mide el despeje del tramo.
>
> **No configure ninguna con *Detección de Cruce de Línea*.** El bloque de *«Cámaras 2 y 4
> (Umbral)»* de más abajo queda tachado: describe una función que no existe en este equipo.

#### Para TODAS las cámaras (Demanda de Cola - Aproximación):
1. Ir a **Configuración > Eventos > Evento Inteligente > Detección de Intrusión** (*Intrusion Detection*).
2. Marcar ☑ **Habilitar** (*Enable*).
3. **Dibujar Región:** Dibujar un rectángulo amplio que cubra el **90% del encuadre inferior y central** (zona donde se detendrán los vehículos).
4. **Parámetros:**
   * **Tiempo de Permanencia (*Threshold*):** Configurar en **`1s`**.
   * **Sensibilidad (*Sensitivity*):** Configurar en **`50`**.
5. **Clasificación de Objetivo (*Detection Target*):**
   * ☑ **Vehículo (*Vehicle*)**
   * ☐ **Humano (*Human*)** *(Desmarcado: ignora peatones, ramas, lluvia y sombras)*.

#### ~~Para Cámaras 2 y 4 (Umbral de Cruce de Tramo):~~ ⛔ NO SE CONFIGURA ASÍ NINGUNA CÁMARA
1. ~~Ir a **Configuración > Eventos > Evento Inteligente > Detección de Cruce de Línea** (*Line Crossing Detection*).~~
2. ~~Marcar ☑ **Habilitar** (*Enable*).~~
3. ~~Trazar la línea atravesando el carril de salida.~~
4. ~~**Dirección:** **Bidireccional (`A<->B`)**.~~
5. ~~**Clasificación de Objetivo:** ☑ **Vehículo** | ☐ **Humano**.~~

> **Por qué se tacha, y no se borra.** La cámara de umbral **no existe en este equipo** —no hay
> entrada física para ella y no hay comando de radio que lleve la cuenta del tramo al Maestro—.
> Configurar una cámara así y llevarla a `J16` p10 o p12 haría que **cada vehículo que SALE del
> tramo pidiera paso**, que es lo contrario de lo que se busca: esos dos pines llaman a
> `demanda_solicitar()` (`Maestro/src/botones.cpp:126-133`).
>
> Se conserva tachado porque describe lo que costaría construirla, si algún día se quiere.

### Paso 4: Configuración de la Salida de Relé (Contacto Seco)

> 🟢 **04/09 — LA SALIDA VA EN `NO`, EN LAS TRES ENTRADAS, Y YA NO DEPENDE DE NINGUNA MEDIDA
> PENDIENTE.** La salida de la AcuSense **es configurable `NO`/`NC`**, así que se elige **qué estado
> significa demanda sin tocar placa ni firmware**; lo que decide cuál es la correcta **es el cobre**,
> y el cobre está medido.
>
> | destino | reposo del pin lo fija | cómo se cablea el contacto | configuración |
> |---|---|---|---|
> | **`PB0` / `J14`** (el de hoy) | ✅ **MEDIDO**: `R64` 10 kΩ a masa + `C25` 100 nF (`pines.h:43-46`) | entre el pin y el borne de **3,3 V** de `J14` — **NO contra `GND`** | **`NO`**, pulso **1 s** |
> | **`J16` p10 / p12** | ✅ **MEDIDO EN BANCO el 04/09** (`M3`, paso 20): **9,93 kΩ** y **9,94 kΩ** a masa, los dos a **0 V** con energía. Pull-**DOWN** real de 10 kΩ | entre el pin de señal y el borne de **3,3 V contiguo** (`p9` para `p10`, `p11` para `p12`), retorno de masa por `p2` | **`NO`**, pulso **1 s** |
>
> ~~**Según lo que dé la medida M3**, con la tarjeta energizada y `J16` vacío:~~ ⛔ **Esta tabla de
> tres ramas ya no se ejecuta: `M3` la resolvió en la primera.** Se conserva porque **es el
> procedimiento con el que se comprueba una tarjeta nueva**, y una placa de otro lote no está medida
> por que ésta lo esté.
>
> | lectura en el pin | qué significa | cómo se cablea |
> |---|---|---|
> | **≈ 0 V, y 10 kΩ a masa** ✅ **ES LO QUE DIO** | pull-**DOWN** de 10 kΩ: el netlist tiene razón, **activo en ALTO** | contacto entre el pin de señal y el pin de **3,3 V** contiguo (`J16` p9 para p10, p11 para p12) |
> | ~~**~3,3 V**~~ | pull-**UP**: el netlist no describe esta placa | contacto contra **`GND`** (`J16` p2) — **y habría que invertir la lectura de cámara en el firmware antes de cablear**, porque hoy lee `== HIGH`. **No es el caso de esta tarjeta** |
> | ~~**otra cosa**~~ | ni una ni otra | **no se cablea.** Se anota el número y se para. **No es el caso de esta tarjeta** |
>
> 🛑 **`NC` no se usa en ninguno de los dos casos.** Con `NC` el contacto está cerrado en reposo y se
> abre al detectar: el firmware vería **demanda permanente** mientras no pasa nada y **ausencia de
> demanda** justo cuando pasa un vehículo. Es la inversión exacta que ya costó `N-67`. **Que la
> cámara admita `NC` no lo convierte en una opción: la admite, y aquí se pide `NO`.**
>
> ✅ **El tercer resultado que `M3` podía dar —que no hubiera resistencia ninguna— queda descartado
> por medida.** El firmware pone el pin en `pinMode(INPUT)` **pelado** (`Maestro/src/botones.cpp:155-157`),
> sin pull-up ni pull-down internos, así que **el reposo lo tenía que fijar cobre real**: lo fija,
> son 9,93 y 9,94 kΩ. **Y el paso 21 lo confirmó por el otro lado:** en reposo, con y sin el cable
> puesto, **el equipo no pide paso por sí solo** — cero demandas fantasma.

1. Ir a **Configuración > Eventos > Salida de Alarma** (*Alarm Output*):
   * **Salida:** `Alarma 1` (Bornes físicos `1A` y `1B`).
   * **Estado por Defecto:** **`NO` (Normally Open / Normalmente Abierto)**.
   * **Duración del Pulso:** **`1s`** (un segundo).
2. Ir a la pestaña **Método de Vinculación (*Linkage Method*)** del evento inteligente configurado y marcar:
   * ☑ **Disparar Salida de Alarma 1** (*Trigger Alarm Output*).
3. **DESARMAR EVENTOS BÁSICOS:** Ir a *Detección de Movimiento básica*, *Sabotaje de Video* y *Excepción* y asegurarse de que la casilla "Disparar Salida de Alarma" esté **desmarcada**. *(Una salida = Un significado único de vehículo)*.
4. Hacer clic en **Guardar** (*Save*).

---

## 5. Dinámica de Control y Seguridad Vial

1. **Llegada de Vehículo al Sentido 1:**
   * La **Cámara 1** detecta el vehículo por intrusión ➔ Cierra el relé `1A`/`1B` en `PB0` del Maestro.
   * El Semáforo Maestro registra la demanda vehicular. Si el sentido opuesto estaba en verde, inicia su cierre: **Verde ➔ Amarillo (4.0s) ➔ Despeje Todo-Rojo (`cfgDespejeSeg`) ➔ Verde Sentido 1**.
2. **Llegada de Vehículo al Sentido 2:**
   * La **Cámara 3** detecta el vehículo por intrusión ➔ Cierra el relé `1A`/`1B` en `PB0` del Esclavo.
   * El Esclavo transmite la demanda al Maestro vía radio LoRa (`RS485_OUT`).
   * El Maestro aplica la transición segura respetando siempre el **Despeje Todo-Rojo** antes de otorgar el verde al Esclavo.
3. **Invariable Vial:** Bajo ninguna circunstancia se omite el tiempo de Todo-Rojo de despeje ni el amarillo normativo de 4.0 segundos.

---

## 6. Protocolo de Pruebas y Validación Rápida en Taller

```text
 ┌─────────────────────────────────────────────────────────────────────────────┐
 │                         BANCO DE PRUEBAS EN TALLER                          │
 ├─────────────────────────────────────────────────────────────────────────────┤
 │                                                                             │
 │ • ENSAYO 1: CONTINUIDAD DEL RELÉ                                            │
 │   - Conectar multímetro en modo continuidad en los bornes 1A y 1B.          │
 │   - Reposo: Circuito abierto (sin pito).                                    │
 │   - Al pasar una maqueta o vehículo: Pita exactamente 1 segundo y se abre.  │
 │                                                                             │
 │ • ENSAYO 2: INMUNIDAD A PEATONES                                            │
 │   - Una persona camina o salta frente al lente.                             │
 │   - Criterio: El relé permanece ABIERTO (cero falsos disparos).             │
 │                                                                             │
 │ • ENSAYO 3: CONMUTACION EN SEMAFORO                                         │
 │   ~~- Conectar 1A/1B al pin PB0 y GND de la tarjeta STM32.~~  <-- ANULADO   │
 │   - Conectar 1A/1B entre el pin PB0 y el borne de 3,3 V de J14.             │
 │     LA ENTRADA ES ACTIVA EN ALTO: contra GND no dispara nunca, y el         │
 │     ensayo saldria "sin deteccion" sin que nada este roto.                  │
 │   - Al detectar vehiculo: El semaforo atiende la demanda y abre verde tras  │
 │     el despeje de seguridad.                                                │
 │   - CRITERIO NEGATIVO, obligatorio: con el contacto ABIERTO y nadie delante │
 │     de la camara, el equipo NO debe registrar demanda. Si la registra, el   │
 │     pin esta flotando o la polaridad no es la que se cree: se para.         │
 │                                                                             │
 └─────────────────────────────────────────────────────────────────────────────┘
```

```text
 ┌─────────────────────────────────────────────────────────────────────────────┐
 │        ENSAYO 4: LO MISMO SOBRE J16 p10 / p12  --  YA SE PUEDE HACER        │
 ├─────────────────────────────────────────────────────────────────────────────┤
 │                                                                             │
 │ 0. TAPAR EL PIN 1 DE J16 (12 V CRUDOS). Sin esto no se sigue.               │
 │ 1. Firmware nuevo YA CARGADO Y VERIFICADO en la tarjeta. Ver el orden       │
 │    asimetrico del apartado 2.1: primero firmware, despues hilo.             │
 │ 2. Ohmimetro entre el pin de senal y masa, con la tarjeta SIN energia:      │
 │    se esperan ~10 kOhm.   MEDIDO 04/09: p10 = 9,93k   p12 = 9,94k           │
 │ 3. Con energia y J16 vacio: el pin en reposo debe estar a 0 V.              │
 │    MEDIDO 04/09: los dos a 0 V.                                             │
 │ 4. Contacto seco 1A/1B entre el pin de senal y el borne de 3,3 V contiguo   │
 │    (p9 para p10, p11 para p12). Retorno de masa por p2.                     │
 │ 5. Al detectar vehiculo: el equipo registra demanda y abre verde tras el    │
 │    despeje de seguridad.                                                    │
 │ 6. CRITERIO NEGATIVO, obligatorio y con MAS peso que en J14: en reposo,     │
 │    CON el cable puesto y SIN el, el equipo NO debe pedir paso solo.         │
 │    VERIFICADO EN BANCO el 04/09 (paso 21): no hay falsa activacion.         │
 │                                                                             │
 └─────────────────────────────────────────────────────────────────────────────┘
```

> 🟢 **04/09 — el `ENSAYO 4` ya no es un hueco: se puede ejecutar, y sus dos primeros pasos ya se
> ejecutaron en banco.**
>
> ~~**No hay ensayo de `J16` p10/p12 en este manual, y falta escribirlo… Hasta que `M3` se haga, no
> se cablea cámara a `J16`.**~~ ⛔ **ANULADO el 04/09: `M3` está hecha (paso 20) y el ensayo está
> escrito arriba.** El texto viejo se conserva porque su razón era buena —*un ensayo que no se puede
> ejecutar no es una casilla pendiente: es la razón por la que no se cablea*— y hoy se aplica al
> revés: **ya se puede ejecutar, luego ya se puede cablear.**
>
> ⚠️ **Y el `ENSAYO 4` NO es igual que el `ENSAYO 3`, aunque se parezcan:** `PB0` lleva un antirrebote
> RC en la placa (`R64` + `C25`, ~1 ms) y **`PB14`/`PB15` no llevan ninguno** — su único filtro son
> los 5 ms por software de `botones.cpp:87-93`. Por eso el criterio negativo pesa **más** ahí, y por
> eso se ejerce **con el cable puesto**, que es cuando entra el ruido: un cable de campo es una
> antena. **El paso 21 lo ejerció así y no hubo falsa activación.**
>
> 🛑 **Lo que ni el `ENSAYO 4` ni `M3` cubren:** la entrada sigue yendo **desnuda al pin del STM32**
> —sin serie, sin opto, sin clamp—. Un ensayo limpio en banco no protege de una sobretensión por el
> hilo de campo. Ver el bloque de la cabecera y la línea de la **V2** en la lista de compras.

---

## 7. 🛑 Nivel de prueba de este manual — no es un permiso para instalar

| lo que este manual afirma | nivel |
|---|---|
| La entrada de cámara es **activa en ALTO** y no se cablea contra `GND` | ✅ **MEDIDO EN EL FUENTE** (`Maestro/src/botones.cpp:87-93`, y su razonamiento en `pines.h:103-109`) |
| `PB0`/`J14` con `R64` 10 kΩ + `C25` 100 nF es una entrada de cámara con firmware | ✅ **MEDIDO** (`pines.h:43-46`; `botones.cpp:155`; `modo_inteligente.cpp:86`, `:124`; `Esclavo/src/main.cpp:350`) |
| `PB8` es el `LED_TESTIGO`, no una entrada | ✅ **MEDIDO** (`pines.h:63`) |
| `PB9`/`PB13` son los canales `A` y `B` del mando y **no admiten cámara** | ✅ **MEDIDO** (`pines.h:122-123`, `botones.cpp:139-140`, `mando.cpp:225-234`) |
| **`PB14`/`PB15` son `CAM_C_PIN`/`CAM_D_PIN`, entradas de cámara de DEMANDA** | ✅ **MEDIDO el 02/09** (`pines.h:124-125`, `botones.cpp:156-157`, `:126-133`). **`botonAceptar()`/`botonCancelar()` devuelven `false` siempre** (`botones.cpp:280-281`) |
| Las separaciones de cobre de `J16` contra los 12 V | ✅ **MEDIDO** sobre el `.kicad_pcb` (`MAPEO_TARJETA_KICAD.md:576-588`) |
| Que el **firmware** de `J16` p10/p12 esté escrito | ✅ **HECHO Y MEDIDO** |
| ~~Que `R65`–`R68` estén montadas y la polaridad de `J16` sea la del netlist~~ | 🟢 **MEDIDO EN BANCO el 04/09 — `M3` CERRADA** (paso 20): `p10` **9,93 kΩ**, `p12` **9,94 kΩ** a masa, los dos a **0 V** con energía. Pull-**DOWN** real ⇒ **activo en ALTO**. ~~🔴 NO VERIFICADO, pendiente~~ |
| Que en reposo el equipo **no pida paso solo**, con y sin el cable puesto | ✅ **MEDIDO EN BANCO el 04/09** (paso 21): **cero falsas activaciones** |
| Que las cinco entradas de campo van **desnudas** al pin del STM32 y que `J16` p1 lleva **12 V crudos** | ✅ **MEDIDO EN BANCO el 04/09.** Es un **defecto de diseño de la V1**, no una advertencia de manual: se mitiga tapando el p1, y se corrige en la **V2** |
| Los parámetros de la AcuSense (zona, umbral 1 s, sensibilidad 50, clasificador) | 📖 **LEÍDO** del manual del fabricante. **No verificado contra una cámara de este proyecto** |

> 🟢 **Lo que SÍ ha pasado banco de este manual, y sólo eso:** la medida `M3` (paso 20) y el ensayo
> de falsa activación (paso 21). **Con eso, cablear cámara a `J16` deja de estar bloqueado.**
>
> 🛑 **Lo que sigue SIN pasar banco:** la cámara AcuSense **real** contra este equipo —zona, umbral,
> clasificador, pulso de 1 s—, y el ciclo completo con demanda por `J16`. **Este manual no autoriza a
> instalar**: autoriza a **cablear** lo que `M3` desbloqueó, con el pin de 12 V tapado y con el
> firmware nuevo ya cargado en la tarjeta.
>
> La única forma correcta de verificar el firmware es `01_Firmware/compuerta.py`, y un verde suyo
> **no es un permiso**: dice que los modelos y los arneses de PC no encuentran nada, no que el
> firmware funcione en la tarjeta (`CLAUDE.md` §3).

---
*Manual técnico oficial de integración y configuración de Cámaras IA para Semáforos Móviles V9.0. Polaridad corregida el 31/08 (`N-105`): el contacto seco cierra contra 3,3 V, no contra `GND`. **Confirmada en cobre el 04/09 con el cierre de `M3`**, que además levanta el bloqueo para cablear `J16` p10/p12 — con el pin 1 de esa misma bornera **tapado**.*
