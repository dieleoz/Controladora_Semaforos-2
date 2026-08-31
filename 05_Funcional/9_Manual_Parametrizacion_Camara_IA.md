# 📷 MANUAL DE CONFIGURACIÓN Y PARAMETRIZACIÓN — CÁMARAS IA ACUSENSE PARA SEMÁFOROS MÓVILES (V9.0)

**Sistema:** Controladora de Semáforos Móviles de 3 Estados (Maestro y Esclavo V9.0)  
**Cámara Certificada:** Hikvision AcuSense Varifocal Motorizada (DS-2CD3643G2-LIZSU o equivalente)  
**Topología del Sistema:** Analítica Deep Learning Embebida (sin PC externo) + contacto seco a la bornera **`J14`** (`PB0`, con antirrebote RC de 1 ms en la placa)  
**Verificación Hardware:** Esquemáticos KiCad `Controladora_Semaforos.kicad_sch`, `pines.h` y `03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md`  
**Normativa Aplicable:** Manual de Señalización Vial de Colombia (Resolución 2024 - MinTransporte)  
**Fecha de Emisión:** 26 de Agosto de 2026  
**Última revisión:** 31 de agosto de 2026 — **corregida la POLARIDAD del cableado (`N-105`).** Este
manual mandaba llevar el contacto seco a *«`PB0` y `GND`»* (apartados 2 y 6). **La entrada es activa
en ALTO**: contra masa **la cámara no dispara nunca**. Nada se borra: el texto viejo queda tachado en
su sitio con el motivo.

---

> ## 🛑 AVISO DE POLARIDAD — EL CONTACTO SECO **NO** VA CONTRA `GND` (31/08/2026)
>
> **MEDIDO EN EL FUENTE**, `01_Firmware/Maestro/src/modo_inteligente.cpp` (las dos puntas):
>
> ```
> :25   if (digitalRead(pin) == HIGH) {          // <-- ACTIVO EN ALTO
> :46   pinMode(CAM_DEMANDA_PIN, INPUT);         // <-- INPUT PELADO, sin pull-up
>
> // del mismo fichero, :36-44 (el porque, N-67):
> //   PB0 lleva R64 de 10 kOhm A MASA -pull-DOWN- y C25 de 100 nF tambien a masa;
> //   la bornera J14 saca ese pin JUNTO A 3,3 V. O sea que el contacto seco de la
> //   camara va entre los dos bornes de J14 y CIERRA A 3,3 V.
> ```
>
> El reposo del pin **ya lo fija la placa** con `R64` de 10 kΩ a masa. El contacto de la cámara tiene
> que **cerrar el pin contra los 3,3 V** de la bornera, **no contra masa**.
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
 │            MAPA DE CONEXION FISICA DE CAMARAS  --  CORREGIDO 31/08          │
 ├─────────────────────────────────────────────────────────────────────────────┤
 │                                                                             │
 │ • CAMARA DE DEMANDA, hoy:  Bornes 1A/1B --> J14: pin PB0 CONTRA LOS 3,3 V   │
 │   ~~PB0 y GND~~  <-- ANULADO: la entrada es ACTIVA EN ALTO, contra masa     │
 │                       no dispara nunca.  R64 10K a masa ya fija el reposo.  │
 │   - Detecta si hay vehiculos esperando paso en el carril.                   │
 │                                                                             │
 │ • CAMARA DE DEMANDA, destino decidido (Fase 3):  J16 p10 (PB14) / p12 (PB15)│
 │   - HOY NO SE CABLEA: siguen siendo botones y falta la medida M3.           │
 │                                                                             │
 │ • CAMARA 2 / 4 (Umbral):  NO EXISTE EN V9.0 - PB8 es un LED, no una entrada │
 │                                                                             │
 │ • PB9 (J16 p5) y PB13 (J16 p8):  MANDO DE RELES, canales A y B. SE CONSERVAN│
 │   >>> NUNCA UNA CAMARA AQUI: tres pulsos en 12 s componen una secuencia <<< │
 │                                                                             │
 └─────────────────────────────────────────────────────────────────────────────┘
```

### 2.1 El reparto real, pin por pin

| pin | qué es | ¿cámara? | nivel |
|---|---|---|---|
| **`PB0`** (`J14`) | `CAM_DEMANDA_PIN`, con `R64` 10 kΩ + `C25` 100 nF (antirrebote 1 ms) | ✅ **Sí, y es la única con firmware hoy** | ✅ **MEDIDO** (`pines.h:43-46`; leída en `modo_inteligente.cpp:98`, `:136` y `Esclavo/src/main.cpp:350`) |
| **`PB8`** | `LED_TESTIGO` → `R16` 1 kΩ → LED `D5` | ❌ **No es entrada de nada** | ✅ **MEDIDO** (`pines.h:63`) |
| **`PB9`** (`J16` p5) | `BOTON1` = **`MANDO_A`** del mando de relés | 🛑 **NUNCA.** `A·A·A` en 12 s = Modo Automático | ✅ **MEDIDO** (`pines.h:92`, `botones.cpp:119`, `mando.cpp:38`, `:225-227`) |
| **`PB13`** (`J16` p8) | `BOTON2` = **`MANDO_B`** | 🛑 **NUNCA.** `B·B·B` = Ámbar **y arma `ambarLocal`**, que veta las órdenes de radio | ✅ **MEDIDO** (`pines.h:93`, `botones.cpp:120`, `mando.cpp:230-234`, `Esclavo/src/mando.cpp:129-132`) |
| **`PB14`** (`J16` **p10**) | Hoy `botonAceptar()` — **el que EJECUTA** | 🟡 **Destino decidido, NO cableable todavía** | ✅ **MEDIDO** hoy (`botones.cpp:52`, `:131`) · 📖 **LEÍDO** el destino (`ESTADO.md:83`) |
| **`PB15`** (`J16` **p12**) | Hoy `botonCancelar()` | 🟡 **Destino decidido, NO cableable todavía** | ✅ **MEDIDO** hoy (`botones.cpp:53`, `:132`) · 📖 **LEÍDO** el destino (`ESTADO.md:83`) |

> 🔴 **`PB14`/`PB15` NO SE CABLEAN HOY.** Cuatro bloqueos, todos vigentes:
> **(1)** falta la medida **M3**, la polaridad de `J16` está en contradicción entre el netlist
> (pull-**down**, activo en ALTO) y `botones.cpp:19` (`INPUT_PULLUP`, activo en BAJO);
> **(2)** el orden es **asimétrico** —`PB14` es `botonAceptar()`, **el que EJECUTA**: el firmware nuevo
> tiene que estar **CARGADO EN LA TARJETA** antes de que nadie enchufe nada, porque un commit no
> protege de un destornillador (`CLAUDE.md` §9.bis);
> **(3)** **`J16` p1 lleva 12 V crudos** y se tapa físicamente antes de cablear — la separación real
> sobre cobre contra la red de 12 V es de **4,269 mm** en `p10` y de sólo **1,359 mm** en `p12`
> (**MEDIDO**, `03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md:576-588`), así que **si una de las dos
> cámaras es más crítica va en `p10`**;
> **(4)** ningún firmware los lee todavía como cámara.

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

#### Para Cámaras 1 y 3 (Demanda de Cola - Aproximación):
1. Ir a **Configuración > Eventos > Evento Inteligente > Detección de Intrusión** (*Intrusion Detection*).
2. Marcar ☑ **Habilitar** (*Enable*).
3. **Dibujar Región:** Dibujar un rectángulo amplio que cubra el **90% del encuadre inferior y central** (zona donde se detendrán los vehículos).
4. **Parámetros:**
   * **Tiempo de Permanencia (*Threshold*):** Configurar en **`1s`**.
   * **Sensibilidad (*Sensitivity*):** Configurar en **`50`**.
5. **Clasificación de Objetivo (*Detection Target*):**
   * ☑ **Vehículo (*Vehicle*)**
   * ☐ **Humano (*Human*)** *(Desmarcado: ignora peatones, ramas, lluvia y sombras)*.

#### Para Cámaras 2 y 4 (Umbral de Cruce de Tramo):
1. Ir a **Configuración > Eventos > Evento Inteligente > Detección de Cruce de Línea** (*Line Crossing Detection*).
2. Marcar ☑ **Habilitar** (*Enable*).
3. Trazar la línea atravesando el carril de salida.
4. **Dirección:** **Bidireccional (`A<->B`)**.
5. **Clasificación de Objetivo:** ☑ **Vehículo** | ☐ **Humano**.

### Paso 4: Configuración de la Salida de Relé (Contacto Seco)

> 🔴 **La salida es configurable `NO`/`NC`, y cuál es la correcta depende de CÓMO esté cableado el pin
> de destino. Las dos configuraciones van escritas aquí, y se elige con una medida — no de memoria.**
>
> | destino | reposo del pin lo fija | cómo se cablea el contacto | configuración |
> |---|---|---|---|
> | **`PB0` / `J14`** (el de hoy) | ✅ **MEDIDO**: `R64` 10 kΩ a masa + `C25` 100 nF (`pines.h:43-46`) | entre el pin y el borne de **3,3 V** de `J14` — **NO contra `GND`** | **`NO`**, pulso **1 s** |
> | **`J16` p10 / p12** (Fase 3) | 🔴 **SIN MEDIR.** `R65`–`R68` de 10 kΩ a `GND` **sólo lo dice el netlist** | **según el resultado de M3** (ver abajo) | **`NO`**, pulso **1 s** |
>
> **Según lo que dé la medida M3** (`05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md`,
> sección M3), con la tarjeta energizada y `J16` vacío:
>
> | lectura en el pin | qué significa | cómo se cablea |
> |---|---|---|
> | **~0,66 V** | pull-**DOWN** de 10 kΩ: el netlist tiene razón, **activo en ALTO** | contacto entre el pin de señal y el pin de **3,3 V** contiguo (`J16` p9 para p10, p11 para p12) |
> | **~3,3 V** | pull-**UP**: el netlist no describe esta placa | contacto contra **`GND`** (`J16` p2) — **y hay que invertir la lectura de cámara en el firmware antes de cablear**, porque hoy lee `== HIGH` |
> | **otra cosa** | ni una ni otra | **no se cablea.** Se anota el número y se para |
>
> 🛑 **`NC` no se usa en ninguno de los dos casos.** Con `NC` el contacto está cerrado en reposo y se
> abre al detectar: el firmware vería **demanda permanente** mientras no pasa nada y **ausencia de
> demanda** justo cuando pasa un vehículo. Es la inversión exacta que ya costó `N-67`.
>
> 🔴 **Y un tercer resultado de M3 que también bloquea: que no haya resistencia ninguna.** El firmware
> pone el pin en `pinMode(INPUT)` **pelado** (`modo_inteligente.cpp:46`), sin pull-up ni pull-down
> internos: si en `PB14`/`PB15` **no hay una resistencia real montada a masa**, el pin queda
> **flotando** y el ruido dispara **demandas fantasma** en un equipo que gobierna un cruce. La medida
> con óhmetro entre el pin y masa —**10 kΩ esperados**— es parte de M3 y no se salta.

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

> 🛑 **Estos tres ensayos son de TALLER y sobre `PB0`/`J14`, que es el único camino con firmware.**
> **No hay ensayo de `J16` p10/p12 en este manual**, y no lo habrá hasta que la medida **M3** cierre la
> polaridad y exista firmware que lea esos pines. Un ensayo que no se puede ejecutar no es una casilla
> pendiente: es la razón por la que no se cablea.

---

## 7. 🛑 Nivel de prueba de este manual — no es un permiso para instalar

| lo que este manual afirma | nivel |
|---|---|
| La entrada de cámara es **activa en ALTO** y no se cablea contra `GND` | ✅ **MEDIDO EN EL FUENTE** (`modo_inteligente.cpp:25`, `:46`, y su razonamiento en `:36-44`) |
| `PB0`/`J14` con `R64` 10 kΩ + `C25` 100 nF es el único camino de cámara con firmware | ✅ **MEDIDO** (`pines.h:43-46`; `modo_inteligente.cpp:98`, `:136`; `Esclavo/src/main.cpp:350`) |
| `PB8` es el `LED_TESTIGO`, no una entrada | ✅ **MEDIDO** (`pines.h:63`) |
| `PB9`/`PB13` son los canales `A` y `B` del mando y **no admiten cámara** | ✅ **MEDIDO** (`pines.h:92-93`, `botones.cpp:119-120`, `mando.cpp:38`, `:225-234`) |
| `PB14`/`PB15` son hoy `botonAceptar()` y `botonCancelar()` | ✅ **MEDIDO** (`botones.cpp:52-53`, `:131-132`) |
| Las separaciones de cobre de `J16` contra los 12 V | ✅ **MEDIDO** sobre el `.kicad_pcb` (`MAPEO_TARJETA_KICAD.md:576-588`) |
| Que las cámaras se muden a `J16` p10/p12 | 📖 **LEÍDO** en las decisiones (`ESTADO.md:83`, `:105`; `roadmap.md` `N-104`). **Decidido, sin construir** |
| Que `R65`–`R68` estén montadas y la polaridad de `J16` sea la del netlist | 🔴 **NO VERIFICADO.** Es la medida **M3**, con multímetro, y está **pendiente** |
| Los parámetros de la AcuSense (zona, umbral 1 s, sensibilidad 50, clasificador) | 📖 **LEÍDO** del manual del fabricante. **No verificado contra una cámara de este proyecto** |

> **Nada de este manual ha pasado prueba de banco**, y **no autoriza a instalar ni a cablear nada**.
> La única forma correcta de verificar el firmware es `01_Firmware/compuerta.py`, y un verde suyo
> **tampoco es un permiso**: dice que los modelos y los arneses de PC no encuentran nada, no que el
> firmware funcione en la tarjeta (`CLAUDE.md` §3).

---
*Manual técnico oficial de integración y configuración de Cámaras IA para Semáforos Móviles V9.0. Polaridad corregida el 31/08 (`N-105`): el contacto seco cierra contra 3,3 V, no contra `GND`.*
