# 📷 MANUAL DE PARAMETRIZACIÓN, CABLEADO Y TOPOLOGÍA — CÁMARAS IA (HIKVISION AcuSense G2)

**Modelo de Cámara:** Hikvision `DS-2CD3643G2-LIZSU` (Lente Varifocal Motorizado 2.7–13.5 mm)
**Sistema:** Controladora de Semáforos Móviles de 3 Estados (Control por Demanda Vehicular / Paso Alternado)
**Topología VIGENTE:** 2 Nodos Semafóricos (Maestro y Esclavo) + **2 cámaras IA — UNA por poste** + Enlace de Control `RS485_OUT`
**Verificación Hardware:** Esquemáticos KiCad `Controladora_Semaforos.kicad_sch`, `pines.h` y `MAPEO_TARJETA_KICAD.md`
**Normativa Aplicable:** Manual de Señalización Vial de Colombia (Resolución 2024 - MinTransporte)
**Fecha de Emisión:** 26 de Agosto de 2026
**Fecha de Corrección:** 28 de Agosto de 2026 *(ver §0 — este manual salió con dos errores de pin)*

---

## 0. 🛑 CORRECCIÓN DEL 28/08/2026 — QUÉ DECÍA ESTE MANUAL Y POR QUÉ ERA FALSO

> **Esta sección no se borra.** Lo que se corrige en silencio se vuelve a escribir, y la segunda
> vez ya nadie recuerda que se comprobó.

La versión emitida el **26/08/2026** (commit `3d24da6`, único commit de este fichero hasta hoy)
describía **4 cámaras** y asignaba la cámara de demanda al pin **`PB9`**. Ambas cosas son falsas
contra el firmware que corre. Se corrigen aquí, y queda el registro de qué se dijo:

| Lo que decía el manual del 26/08 | Lo MEDIDO sobre el fuente (28/08) |
|---|---|
| **4 cámaras** (2 por poste): demanda + umbral | El firmware lee **1 cámara por poste**. La de umbral no tiene dónde entrar |
| Cámara 1 y 3 (demanda) → pin **`PB9`** | **`PB9` es `BOTON1`** (`pines.h:92`, las dos puntas). La demanda entra por **`PB0`** = `CAM_DEMANDA_PIN` (`pines.h:46`) |
| Cámara 2 y 4 (umbral) → pin **`PB13`** | **`PB13` es `BOTON2`** (`pines.h:93`). No existe `CAM_UMBRAL_PIN` en el firmware Maestro/Esclavo |
| «Optoacoplador `TLP127` con pull-up en `PB9`/`PB13`» | La línea de cámara real (`PB0`) lleva **`R64` 10 kΩ (pull-DOWN) + `C25` 100 nF**, antirrebote ~1 ms, bornera **`J14`**, y es **activa en ALTO** |

**De dónde salió el error:** el manual del 26/08 se escribió contra el firmware de nodo único
`01_Firmware/Semaforos/`, que sí definía dos entradas de cámara (`CAM_DEMANDA_PIN` = `PB0` y
`CAM_UMBRAL_PIN` = `PB8`). Al partir el firmware en Maestro/Esclavo, `CAM_UMBRAL_PIN` **no se
portó**. Y el pin `PB8` que se le atribuía tampoco es una entrada: medido el 27/08 sobre el
esquemático bueno, **`PB8` va por `R16` 1 kΩ a un LED testigo (`D5`)** — es una salida de aviso,
no una bornera (`roadmap.md` N-64, y `pines.h:63` lo deja escrito como `LED_TESTIGO`).

**Distinción de evidencia, que importa:**

| | |
|---|---|
| **MEDIDO** (sobre fuente y esquemático, 27–28/08) | `PB0` = `CAM_DEMANDA_PIN`; `PB9`/`PB13` = botones; `PB8` = LED testigo; polaridad activa en alto; `R64`/`C25`/`J14`; cero llamadores del puerto serie IA |
| **VERIFICADO EN LA PLACA** | **NADA de este manual lo está todavía.** La sesión de banco es su primera comprobación física. Un `.md` correcto contra el fuente sigue siendo papel |

---

## 0.bis 🛑 EL «PUERTO SERIE DE CÁMARA IA» (`AI_CARS` / YOLO / Raspberry Pi) **NO ESTÁ IMPLEMENTADO**

> **Estado: código huérfano. Especificado, escrito, y sin un solo llamador.** No lo describa a
> ningún cliente como una función del equipo.

Medido el 28/08 con `grep` sobre `01_Firmware/`:

- `AiBus` — el `HardwareSerial` del puerto — está **declarado** en
  `Maestro/src/protocolo.cpp:7` y `Esclavo/src/protocolo.cpp:7`.
- `protocolo_actualizarAI()`, `protocolo_obtenerAutosEsperandoAI()` y
  `protocolo_obtenerUltimoTiempoAI()` están **declaradas** en `*/include/protocolo.h:186-188`
  y **definidas** en `*/src/protocolo.cpp:51-79`.
- **Ningún fichero de Maestro ni de Esclavo las llama.** La cadena está cerrada sobre sí misma:
  la única lectura de `AiBus` ocurre dentro de la propia función que nadie invoca.
- **Y hoy el puerto ni siquiera se abre:** `protocolo_setup()` ya no llama a `AiBus.begin()`.
  El comentario del propio fuente lo explica: `AiBus` colgaba de `(PA10, PA9)` — el **mismo
  USART1** que usa `SerialBT` —, dos objetos peleándose un periférico a dos velocidades
  distintas (115200 aquí, 9600 allí). Ganaba Bluetooth por orden de arranque, **así que el
  «puerto IA a 115200» nunca existió**.

**Nació vivo y murió en la partición del firmware.** En `01_Firmware/Semaforos/` (nodo único)
`modo_inteligente.cpp:65,81-82` sí llamaba a las tres. Al separar en Maestro/Esclavo la llamada
no se portó, y el **Esclavo ni siquiera tiene `modo_inteligente.cpp`**.

**Y el PC que hablaría ese puerto está descartado por decisión, no por olvido.**
`05_Funcional/6_Preguntas_Diseno_Funcional.md` lo tiene **CERRADO** por escrito:

> *«Cero Computadores Edge Externos (CERRADO): Se descarta el uso de Raspberry Pi, Jetson Nano,
> conversores USB y switches Ethernet. La detección de vehículos corre directamente dentro de la
> cámara (DSP AcuSense) y entra por pulsos limpios de hardware a la placa.»*

**Conclusión operativa:** la analítica de vídeo la hace **la cámara**, y llega al equipo por **un
contacto seco**. No hay ni habrá un enlace serie de conteo de vehículos mientras esa decisión siga
cerrada. El código de `AiBus` se conserva a propósito —retirarlo es un cambio con sentido propio
y va en su propio `N-x`— pero **no es una función del producto**.

---

## 1. Arquitectura Vial y Distribución de las Cámaras (VIGENTE)

Tramo de obra de un solo carril con paso alternado, **dos postes** y **una cámara de demanda por
poste**:

```text
  ══════════════════════════════════════════════════════════════════════════════════════════════
                                  TRAMO DE OBRA (UN SOLO CARRIL)
  ══════════════════════════════════════════════════════════════════════════════════════════════

  [EXTREMO 1: POSTE MAESTRO]                                        [EXTREMO 2: POSTE ESCLAVO]

       ┌────────────────────────┐                                    ┌────────────────────────┐
       │   SEMAFORO MAESTRO     │                                    │    SEMAFORO ESCLAVO    │
       │   🔴 🟡 🟢             │                                    │    🔴 🟡 🟢            │
       └────────────────────────┘                                    └────────────────────────┘
                  │                                                             │
            (CAMARA 1)                                                    (CAMARA 2)
            [ 👁️ ◄── ]                                                    [ ──► 👁️ ]
        Mira la via de aproximacion                            Mira la via de aproximacion
        Contacto seco -> PB0 (J14)                             Contacto seco -> PB0 (J14)
                  │                                                             │
                  ▼                                                             ▼
        SENTIDO 1 (Llegada)                                          SENTIDO 2 (Llegada)
        Carros entrando ────►                                        ◄──── Carros entrando
        hacia la obra                                                hacia la obra

        El DESPEJE del tramo NO lo mide ninguna camara: es POR TIEMPO (cfgDespejeSeg).
```

---

## 2. Asignación y Función por Cámara (VIGENTE)

| Cámara | Ubicación Física | Sentido de Visión | Función en el Sistema Vial | Pin en Tarjeta STM32 |
|---|---|---|---|---|
| **CÁMARA 1** | **Poste Maestro (Extremo 1)** | **SENTIDO 1 (Aproximación):** vehículos que llegan por la vía hacia el Maestro | **Demanda Vehicular Sentido 1:** al detectar vehículo, solicita apertura de **🟢 Verde en Semáforo Maestro** | Pin **`PB0`** — bornera **`J14`**, `R64` 10 kΩ + `C25` 100 nF, **activa en ALTO** |
| **CÁMARA 2** | **Poste Esclavo (Extremo 2)** | **SENTIDO 2 (Aproximación):** vehículos que llegan por la vía hacia el Esclavo | **Demanda Vehicular Sentido 2:** el Esclavo transmite la demanda al Maestro por `RS485_OUT`/radio | Pin **`PB0`** — bornera **`J14`**, idéntico al Maestro |

> ⚠️ **Numeración:** el manual del 26/08 llamaba «Cámara 3» a la del Esclavo, porque contaba
> cuatro. Con dos cámaras, la del Esclavo es la **Cámara 2**. Si encuentra rotulado *«CAM 3»* en
> una caja o en un plano viejo, es esta misma.

### Pines que NO son entradas de cámara — no los cablee

| Pin | Lo que realmente es | Referencia |
|---|---|---|
| **`PB9`** | **`BOTON1`** (Arriba) del menú LCD | `pines.h:92` |
| **`PB13`** | **`BOTON2`** (Abajo) del menú LCD | `pines.h:93` |
| **`PB8`** | **`LED_TESTIGO`** — `R16` 1 kΩ → LED `D5`. **Salida, no bornera** | `pines.h:63` |
| `PA9` / `PA10` | USART1 — hoy es el bus de **Bluetooth**, no el puerto IA | `protocolo.cpp:19-45` |

Conectar un relé de cámara a `PB9` o `PB13` **inyecta pulsaciones de menú fantasma** en el
equipo. No es una conexión inerte: es una avería.

---

## 2.bis Cámara de UMBRAL (tramo de obra) — **ESPECIFICADO, SIN CONSTRUIR**

> **No se borra del papel: se marca honestamente.** Si mañana se quiere, esto es lo que costaría.

La cámara de umbral —la que confirmaría el **despeje efectivo** del tramo mirando los vehículos
que ya cruzaron— está **diseñada y documentada, y no existe en el equipo**. Faltan **dos** cosas,
y ninguna es un `pinMode()`:

1. **Una entrada física.** `PB8` no sirve: es el LED testigo. Habría que sacar un hilo del pad de
   `PB8` retirando `R16`/`D5`, o cablear uno de los cuatro pines libres (`PA11`, `PA12`, `PA15`,
   `PC13`) a una bornera con su red de antirrebote.
2. **Un comando de radio** que lleve la cuenta del tramo desde el Esclavo hasta el Maestro, que
   es quien decide (`roadmap.md` N-59). Sin ese comando, leer el pin es medio camino — y un pin
   leído que no llega a la lógica es exactamente la clase de función huérfana que este manual
   acaba de corregir en §0.bis.

**Mientras tanto el despeje se hace POR TIEMPO** (`cfgDespejeSeg`), que es el criterio
conservador: la cámara de umbral daría **eficiencia**, no seguridad. El equipo es seguro sin ella.

---

## 3. Diagrama Eléctrico de Cableado (VIGENTE)

Cada cámara conecta su salida de contacto seco de relé (**Bornera `ALARM`: pines `1A` y `1B`**)
a la bornera **`J14`** de la tarjeta STM32 de su propio poste. La comunicación entre postes viaja
por **`RS485_OUT`**:

```text
 ┌──────────────────────────────────────────────┐      ┌──────────────────────────────────────────────┐
 │            NODO 1: SEMAFORO MAESTRO          │      │            NODO 2: SEMAFORO ESCLAVO          │
 ├──────────────────────────────────────────────┤      ├──────────────────────────────────────────────┤
 │                                              │      │                                              │
 │  CAMARA 1 (Demanda Sentido 1)                │      │  CAMARA 2 (Demanda Sentido 2)                │
 │    Rele [ 1A ] ───► J14 / PB0  (activa ALTO) │      │    Rele [ 1A ] ───► J14 / PB0  (activa ALTO) │
 │    Rele [ 1B ] ───► 3.3V del propio J14      │      │    Rele [ 1B ] ───► 3.3V del propio J14      │
 │      (R64 10K a GND = pull-DOWN + C25 100nF) │      │      (R64 10K a GND = pull-DOWN + C25 100nF) │
 │                                              │      │                                              │
 │  NO CABLEAR: PB9 = BOTON1 · PB13 = BOTON2    │      │  NO CABLEAR: PB9 = BOTON1 · PB13 = BOTON2    │
 │  NO CABLEAR: PB8 = LED testigo D5 (salida)   │      │  NO CABLEAR: PB8 = LED testigo D5 (salida)   │
 │                                              │      │                                              │
 │  BORNERA RS485_OUT (Control Inter-Poste)     │      │  BORNERA RS485_OUT (Control Inter-Poste)     │
 │    [ A   ] ══════════════════════════════════╪══════╪═══════════════════════════════► [ A   ]      │
 │    [ B   ] ═══════ (Cable Trenzado o Radio) ═╪══════╪═══════════════════════════════► [ B   ]      │
 │    [ GND ] ══════════════════════════════════╪══════╪═══════════════════════════════► [ GND ]      │
 └──────────────────────────────────────────────┘      └──────────────────────────────────────────────┘
```

### Reglas Eléctricas:

1. **Contacto seco, pero la POLARIDAD DE LA SEÑAL SÍ IMPORTA.** Los bornes `1A`/`1B` del relé son
   libres de tensión y entre ellos no hay polaridad; **pero la entrada `PB0` es ACTIVA EN ALTO**
   contra el pull-down de 10 kΩ de la placa. El contacto debe **cerrar `PB0` a 3,3 V**. Cablearlo
   a GND deja la entrada leyendo demanda continua sin que pase ningún vehículo (`N-67`).
2. **Antirrebote:** la placa filtra ~1 ms con `R64`/`C25`, y el firmware añade **5 ms** por
   software. No hace falta condensador externo.
3. **Alimentación 12 V DC:** la cámara de cada poste se alimenta de la batería de 12 V del propio
   semáforo móvil.
4. **Independencia de buses:** el enlace entre postes (`RS485_OUT`) no comparte nada con la señal
   de cámara — son un par diferencial y un contacto seco, sin puntos en común.
5. **Ventana de silencio de 3 s:** el firmware ignora demandas repetidas dentro de los 3 s
   siguientes a una aceptada (`demanda.cpp`). Una cola de coches no satura el canal de 2,4 kbps.
   **No es un fallo de la cámara** si el segundo coche no dispara una trama.

---

## 4. Parametrización de la Cámara en 3 Pasos

Sin Internet, sin routers y sin software de monitoreo. Se hace **una vez en taller** y queda:

```
┌────────────────────────────────────────────────────────────────────────┐
│   CONFIGURACION DE LA CAMARA DE DEMANDA (una por poste)                │
├────────────────────────────────────────────────────────────────────────┤
│ 1. Zoom y Foco Motorizado: encuadrar el carril de parada               │
│ 2. Dibujar la zona / linea de deteccion (Filtro: solo Vehiculo)        │
│ 3. Salida de Alarma: modo N/O a 1 s (se configura una vez y queda)     │
└────────────────────────────────────────────────────────────────────────┘
```

### Paso 1: Encuadre Óptico Motorizado (Lente 2.7–13.5 mm)

1. Conectar la laptop al puerto Ethernet de la cámara e ingresar a `http://192.168.1.64`
   (Usuario: `admin`).
2. En la pestaña **Live View**:
   * Usar **Zoom `+` / Zoom `-`** hasta encuadrar la **zona de parada del carril**.
   * Presionar **One-Touch Focus**. La cámara ajusta la nitidez con su motor interno.
   * **Precintar:** no volver a mover el zoom.

### Paso 2: Dibujar la Detección (Máscara de Demanda)

1. Ir a **Configuración > Eventos > Evento Inteligente > Detección de Cruce de Línea**
   (*Line Crossing Detection*) — o **Detección de Intrusión** con región amplia, si prefiere que
   el equipo se pueda trasladar de obra sin reencuadrar.
2. Marcar ☑ **Habilitar** (*Enable*).
3. Trazar la línea atravesando el carril (o dibujar la región sobre la zona de parada).
4. En **Clasificación de Objetivo** (*Detection Target*):
   * ☑ **Vehículo** (*Vehicle*)
   * ☐ **Humano** (*Human*) — *desmarcado: inmunidad a peatones, ramas y sombras.*

### Paso 3: Configurar la Salida de Relé (N/O a 1 Segundo)

1. Ir a **Configuración > Eventos > Salida de Alarma** (*Alarm Output*):
   * **Estado por Defecto:** **`NO`** (*Normally Open*).
   * **Duración del Pulso:** **`1s`**.
2. En la pestaña del evento inteligente, **Método de Vinculación** (*Linkage Method*):
   * ☑ **Disparar Salida de Alarma 1**.
3. **Desarmar los eventos básicos:** en *Detección de Movimiento básica*, *Sabotaje de Vídeo* y
   *Excepción*, dejar **desmarcada** la casilla «Disparar Salida de Alarma».
   *(Una salida = un único significado: «hay vehículo».)*
4. **Guardar**.

> 💡 **Operación Autónoma:** se desconecta la laptop. La cámara opera de forma continua con la
> batería del semáforo. No queda ningún PC en obra.

---

## 5. Dinámica de Control y Seguridad Vial (Resolución 2024)

1. **Llegada de Vehículo al Sentido 1 (lado Maestro):**
   * **Cámara 1** detecta el vehículo ➔ cierra su relé ➔ **`PB0` del Maestro pasa a ALTO**.
   * El Maestro cierra el Semáforo Esclavo a Rojo, ejecuta el **Todo-Rojo de despeje**
     (`cfgDespejeSeg`) para vaciar la vía, y abre **🟢 Verde en el Semáforo Maestro**.
2. **Llegada de Vehículo al Sentido 2 (lado Esclavo):**
   * **Cámara 2** detecta el vehículo ➔ **`PB0` del Esclavo pasa a ALTO**.
   * El Esclavo emite `CMD_DEMANDA` al Maestro por radio/`RS485_OUT` (con su ventana de 3 s).
   * El Maestro cierra el Semáforo 1 a Rojo, aplica el Todo-Rojo, y otorga **🟢 Verde al
     Semáforo Esclavo**.
3. **Despeje del tramo de obra:** **por tiempo** (`cfgDespejeSeg`). Ninguna cámara lo mide —ver
   §2.bis—. Es el criterio conservador y es el que corre hoy.

> ⚠️ **El Maestro es quien decide siempre.** La cámara del Esclavo **pide**; no abre nada por su
> cuenta.

---

## 6. Protocolo de Pruebas y Validación en Campo

Antes de abrir el paso vehicular en el tramo de obra:

```
[ ENSAYO 1: Continuidad con Multimetro ] ──► [ ENSAYO 2: Demanda Sentido 1 (Maestro) ] ──► [ ENSAYO 3: Demanda Sentido 2 (Esclavo) ]
```

### 🧪 Ensayo 1: Continuidad del Relé (las 2 cámaras)

* Medir con multímetro en modo continuidad entre `1A` y `1B` de cada cámara.
* **Reposo:** circuito abierto (sin pito).
* **Al cruzar un vehículo:** pita durante **~1 segundo** y se abre solo.

### 🧪 Ensayo 2: Demanda y Conmutación Sentido 1 (Maestro)

* Acercar un vehículo frente a la **Cámara 1**.
* **Criterio de Aceptación:** el Maestro recibe el pulso en **`PB0`** y ejecuta la secuencia de
  transición legal hasta **🟢 Verde Maestro**, manteniendo el Esclavo en Rojo.
* **Criterio negativo, obligatorio:** pulsar los cuatro botones del menú **no** debe generar
  ninguna demanda. Si al pulsar Arriba/Abajo el equipo pide paso, **el relé está cableado a
  `PB9`/`PB13`** — el error de §0.

### 🧪 Ensayo 3: Demanda y Conmutación Sentido 2 (Esclavo)

* Acercar un vehículo frente a la **Cámara 2** (la del Esclavo).
* **Criterio de Aceptación:** el Esclavo recibe el pulso en **`PB0`**, lo transmite, el Maestro
  aplica el Todo-Rojo y otorga **🟢 Verde Esclavo**, pasando él a Rojo.
* **Segundo vehículo dentro de 3 s:** puede no generar trama nueva. Es la ventana de silencio,
  **no un fallo**.

---

## 7. Referencias cruzadas

| Documento | Qué aporta |
|---|---|
| `05_Funcional/9_Manual_Parametrizacion_Camara_IA.md` | **Manual de campo vigente** de la cámara. Ya trae `PB0`/`J14` correctos y el aviso de las cámaras 2 y 4 |
| `05_Funcional/15_Lista_de_Compras_Hardware.md` | Cantidades reales: **2 cámaras**, «son las dos que el firmware lee hoy» |
| `05_Funcional/6_Preguntas_Diseno_Funcional.md` | Decisión **CERRADA** de «Cero Computadores Edge Externos» |
| `roadmap.md` N-59, N-64, N-67 | Origen de la cámara de umbral, del hallazgo de `PB8` y de la polaridad activa en alto |

---
*Manual técnico de instalación, topología y cableado para semáforos móviles.*
*Emitido 26/08/2026 · **Corregido 28/08/2026** (§0: pines `PB9`/`PB13` erróneos y 4 cámaras → 2).*
*Todo lo de este documento está **MEDIDO sobre el fuente y el esquemático**, y **ninguna línea
está VERIFICADA EN LA PLACA**: la sesión de banco es su primera comprobación física.*
