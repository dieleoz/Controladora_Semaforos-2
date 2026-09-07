# 📷 MANUAL DE PARAMETRIZACIÓN, CABLEADO Y TOPOLOGÍA — CÁMARAS IA (HIKVISION AcuSense)

> # 🛑 CABECERA DE ESTADO (07/09/2026) — LÉASE ANTES QUE EL CUERPO
>
> **Este manual se escribió antes de que se comprara la cámara y antes de que se cerrara `M3`.
> Tres cosas suyas están CADUCADAS, y una de ellas bloquea trabajo que ya está desbloqueado.**
>
> | | lo que este manual dice | lo que manda hoy |
> |---|---|---|
> | 🛑 **El modelo** | `DS-2CD3643G2-LIZSU` | **`DS-2CD2683G2-IZS`** — **`D-10`** de [`DECISIONES.md`](../DECISIONES.md) (05/09), *«tiene salida de alarma (`1 in, 1 out, 24 V/1 A`, ficha oficial)»*. La ficha está en esta misma carpeta |
> | 🛑 **«NO CABLEAR hasta `M3`»** *(§0.ter, §2, §3)* | *«`R67`/`R68` sólo están en el netlist y nadie las ha medido»* | ✅ **`M3` CERRADA EL 03/09** — **`D-3`**: medido en cobre con multímetro y conector vacío (paso 20 de la guía de banco), el pull-**down** de 10 kΩ **es real y está en las cuatro posiciones**; `p10` y `p12` dan **0 V en reposo**, y el paso 21 cableó `p10` **sin demandas fantasma**. Fuente que manda: `05_Funcional/17_…` sección **M3** |
> | 🛑 **El PROPÓSITO de las cámaras** | «demanda vehicular»: la cámara **pide verde** | **`D-13`** (05/09) decidió otra cosa, y **no es un matiz**: *«una sola regla, `Intrusion Detection` sobre el BARRIDO DE LA PLUMA —no la zona de espera—»*, y en su apartado *«lo que NO se hace»*: **«el ciclo del semáforo no se toca. Las cámaras no dan ni quitan verde.»** ⚠️ **Ver el aviso de conflicto abierto de abajo: esto NO lo cierra este manual** |
>
> ## 🔴 CONFLICTO ABIERTO, y se anota en vez de resolverse aquí
>
> **`D-13` dice que las cámaras no dan ni quitan verde. El firmware que corre las lleva a
> `demanda_solicitar()`, que es exactamente pedir paso.** Las dos cosas están medidas y las dos son
> ciertas hoy:
>
> ```
> $ grep -n "CAM_J16" 01_Firmware/Maestro/src/botones.cpp
> 118:static const uint8_t CAM_J16[2] = {CAM_C_PIN, CAM_D_PIN};
> ```
>
> Un manual **no decide** cuál gana. Lo que este documento hace es **dejarlo escrito y no fingir que
> ya está resuelto**: mientras siga abierto, **el capítulo de parametrización de abajo describe la
> configuración de DEMANDA, que es la que el firmware ejerce, y NO la de `D-13`.** Quien vaya a
> parametrizar una cámara en campo tiene que preguntar cuál de las dos se monta.
>
> ## ⚠️ Y una fuente que gana a ésta
>
> **`05_Funcional/9_Manual_Parametrizacion_Camara_IA.md` es el manual de campo vigente de la
> cámara** — `D-12` lo asciende a *«entregable principal»*. Este documento cubre el **cableado a la
> tarjeta**; en todo lo que sea **la pantalla de configuración de la cámara**, manda el otro.

**Modelo de Cámara:** ~~Hikvision `DS-2CD3643G2-LIZSU`~~ 🛑 **CADUCADO** → **Hikvision
`DS-2CD2683G2-IZS`** (`D-10`, 05/09; ficha `DS-2CD2683G2-IZS_Datasheet_V5.5.113_20230303.pdf` en
esta carpeta). Lente varifocal motorizado; **`1 alarm in, 1 alarm out`**
**Sistema:** Controladora de Semáforos Móviles de 3 Estados (Control por Demanda Vehicular / Paso Alternado)
**Topología VIGENTE:** 2 Nodos Semafóricos (Maestro y Esclavo) + **cámaras IA de demanda** + Enlace de Control `RS485_OUT`
**Verificación Hardware:** Esquemáticos KiCad `Controladora_Semaforos.kicad_sch`, `pines.h` y `MAPEO_TARJETA_KICAD.md`
**Normativa Aplicable:** Manual de Señalización Vial de Colombia (Resolución 2024 - MinTransporte)
**Fecha de Emisión:** 26 de Agosto de 2026
**Fecha de Corrección:** 2 de Septiembre de 2026 *(ver §0 y §0.ter)*

---

## 0.ter 🟢 QUÉ CAMBIÓ EL 31/08 — ESTE MANUAL SE QUEDÓ CORTO DE ENTRADAS

> **Este manual decía que el firmware lee UNA cámara por poste. Hoy lee TRES.**

Medido el 02/09 sobre el fuente, idéntico en las dos puntas:

| entrada | pin | bornera | antirrebote de placa | estado |
|---|---|---|---|---|
| `CAM_DEMANDA_PIN` | `PB0` | `J14` | ✅ `R64` 10 kΩ + `C25` 100 nF (~1 ms) | ✅ **cableable hoy** |
| `CAM_C_PIN` | `PB14` | `J16` **p10** | ❌ ninguno en la placa — el reposo lo fija `R67` 10 kΩ a masa, ✅ **medida en cobre el 03/09** | ✅ **CABLEABLE** *(~~NO CABLEAR hasta `M3`~~ — `M3` cerrada, `D-3`)*. 👉 **Es la posición de la cámara** |
| `CAM_D_PIN` | `PB15` | `J16` **p12** | ❌ ninguno en la placa — `R68` 10 kΩ a masa, ✅ **medida el 03/09** | ✅ cableable, pero **queda VACÍO**: hay **una cámara por poste** (`D-2`, `D-13`). Y su separación al riel de 12 V es **la peor de las cuatro** (1,36 mm) |

> ⚠️ **Los números de línea de este bloque estaban CADUCADOS los cinco** (decían `pines.h:124-125`,
> `botones.cpp:156-157`, `:126-133`, `:280-281`). **Se cita el símbolo y se publica el `grep` que lo
> encuentra, corrido el 07/09** — un número de línea caduca solo, en silencio y con la autoridad de
> un dato:

```
$ grep -n "define CAM_._PIN" 01_Firmware/Maestro/include/pines.h
148:#define CAM_C_PIN   PB14  // J16 p10 - camara de contacto seco (era BOTON3, "Aceptar")
149:#define CAM_D_PIN   PB15  // J16 p12 - camara de contacto seco (era BOTON4, "Cancelar")

$ grep -n "pinMode(CAM_._PIN" 01_Firmware/Maestro/src/botones.cpp
531:  pinMode(CAM_C_PIN, INPUT);
532:  pinMode(CAM_D_PIN, INPUT);

$ grep -n "^bool botonAceptar\|^bool botonCancelar" 01_Firmware/Maestro/src/botones.cpp
659:bool botonAceptar() { return false; }
660:bool botonCancelar(){ return false; }
```

*(En el Esclavo los mismos símbolos están en `botones.cpp:523-524` y `:645-646`.)*

**Las tres son cámaras de DEMANDA:** las tres acaban en `demanda_solicitar()`. Ninguna mide el
despeje del tramo, que sigue siendo por tiempo (`cfgDespejeSeg`).

> ⛔ **AQUÍ DECÍA: *«~~`J16` p10 y p12 NO SE CABLEAN TODAVÍA… `R67`/`R68` sólo están en el netlist y
> nadie las ha medido en la placa. La medida es `M3`, con óhmetro, y no se salta~~»*. CADUCADO — se
> tacha con su motivo y no se borra (07/09).**
>
> **`M3` se cerró el 03/09** (**`D-3`**), y se cerró midiendo justo eso: multímetro, conector vacío,
> paso 20 de la guía de banco. El pull-**down** de **10 kΩ es real y está en las cuatro posiciones**
> (`R65`–`R68`, cada una con su 100 nF); `p10` y `p12` dan **0 V en reposo**; y el **paso 21** cableó
> `p10` contra `p11` en normalmente abierto **sin una sola demanda fantasma**. **El pin no flota.**
>
> La fuente que manda en esto es `05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md`
> sección **M3**, no este manual. **Dejar el bloqueo escrito después de cerrarlo es lo que ya pasó
> con `CLAUDE.md` §9.bis el 05/09**: alguien recita el párrafo caducado con autoridad y nadie va a la
> fuente.

🔴 **Lo que SÍ sigue bloqueando, y no es opcional:**

1. **`J16` p1 lleva 12 V crudos** —sin opto, sin limitadora y sin clamp— a un conector de señal
   directa al micro. **Taparlo es obligatorio en cada equipo que se monte** (`D-4`, N-120), no una
   cautela de banco.
2. **Si sólo se cablea una cámara, va en `p10`.** La separación real sobre cobre contra la red de
   12 V es **4,269 mm en `p10`** y **1,359 mm en `p12`** —el peor de los cuatro—. Un error de una
   posición al enchufar `J16` mete 12 V en un pin de 3,3 V.
3. **No se cablea nada a `p5` ni a `p8`.** Ver el aviso de §2: son los canales del mando, y su
   **código sigue leyéndolos**.

🪜 **Y el orden es asimétrico: el firmware nuevo tiene que estar CARGADO EN LA TARJETA antes de que
nadie enchufe un hilo en `J16`.** Con el firmware viejo dentro, `PB14` todavía es *Aceptar* leído
**activo en BAJO**, y cualquier cosa enchufada en p10 lo pulsa en un equipo que está en la calle. Un
commit no protege de un destornillador: se exige la carga verificada, no el merge.

---

## 0. 🛑 CORRECCIÓN DEL 28/08/2026 — QUÉ DECÍA ESTE MANUAL Y POR QUÉ ERA FALSO

> **Esta sección no se borra.** Lo que se corrige en silencio se vuelve a escribir, y la segunda
> vez ya nadie recuerda que se comprobó.

La versión emitida el **26/08/2026** (commit `3d24da6`, único commit de este fichero hasta hoy)
describía **4 cámaras** y asignaba la cámara de demanda al pin **`PB9`**. Ambas cosas son falsas
contra el firmware que corre. Se corrigen aquí, y queda el registro de qué se dijo:

| Lo que decía el manual del 26/08 | Lo MEDIDO sobre el fuente (28/08) |
|---|---|
| **4 cámaras** (2 por poste): demanda + umbral | El firmware leía **1 cámara por poste**. La de umbral no tiene dónde entrar. *(Al 02/09 son **3 entradas de demanda** por punta — ver §0.ter. La de umbral **sigue sin existir**)* |
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

**Actualización del 02/09: ya no es código huérfano — es código RETIRADO.**

Medido el 02/09 con `grep AiBus` sobre `01_Firmware/Maestro` y `01_Firmware/Esclavo`: **no queda ni
una declaración, ni una definición, ni una llamada.** Lo único que aparece son dos comentarios que
cuentan la historia (`Maestro/src/protocolo.cpp:19`, `Esclavo/src/protocolo.cpp:19`).

Lo que hubo, y por qué se fue:

- `AiBus` era un `HardwareSerial` sobre `(PA10, PA9)` — el **mismo USART1** que usaba `SerialBT`.
  Dos objetos peleándose un periférico a dos velocidades distintas (115200 aquí, 9600 allí).
  Ganaba Bluetooth por orden de arranque, **así que el «puerto IA a 115200» nunca existió**.
- Sus tres funciones —`protocolo_actualizarAI()` y sus dos consultas— **no tenían un solo
  llamador**. El enlazador ya las descartaba: retirarlas ahorró 16 B de flash.
- **El objeto sí costaba, y ése era el motivo real de retirarlo:** su constructor cuelga del
  arranque, así que corría en cada encendido y su memoria era permanente. Eran **280 B de RAM por
  punta** —el 5,2 % de la RAM viva del equipo— por un puerto que no se abre.

**Y un dato de cableado que sale de aquí:** `SerialBT` **ya no vive en `PA9`/`PA10`**. Vive en
**`PB6`/`PB7`, USART1 remapeado, conector `J17`** (`Maestro/src/bluetooth.cpp:28`, idéntico en el
Esclavo). Ver la tabla de §2.

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
cerrada. **El código de `AiBus` ya se retiró** (02/09) y **no es una función del producto**.

---

## 1. Arquitectura Vial y Distribución de las Cámaras (VIGENTE)

Tramo de obra de un solo carril con paso alternado, **dos postes** y **una cámara de demanda por
poste** — que es el montaje mínimo y el único cableable hoy. Cada punta admite además **dos
entradas más** en `J16`, pendientes de `M3` (§0.ter):

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
| **CÁMARA `C`** | cualquiera de los dos postes | 🔴 **ES LA POSICIÓN DE LA CÁMARA REAL** (`D-2`, `D-3`) | **Demanda Vehicular**, igual que la de `J14`: pide paso | Pin **`PB14`** — **`J16` p10**. ✅ **CABLEABLE** *(~~NO CABLEAR hasta `M3`~~: `M3` cerrada el 03/09)*. Separación al riel de 12 V: **4,27 mm** |
| **CÁMARA `D`** *(opcional)* | cualquiera de los dos postes | libre — sería una demanda más de esa punta | **Demanda Vehicular**, igual | Pin **`PB15`** — **`J16` p12**. ✅ cableable, pero **hoy VACÍO**. ⚠️ Separación al riel de 12 V: **1,36 mm — la peor de las cuatro** |

> ⚠️ **`C` y `D` no son «cámaras de umbral» ni miden nada distinto.** Son entradas de demanda más,
> por si un poste necesita vigilar dos accesos. Sin antirrebote de placa: ver §0.ter.

> ### 🔴 CUÁNTAS CÁMARAS HAY — decidido, y esta tabla se leía como si fueran cuatro
>
> **Son DOS en total: una por poste** (`D-2` de [`DECISIONES.md`](../DECISIONES.md), 28/08;
> ratificado por `D-13`, 05/09). No son «dos fijas más dos opcionales».
>
> **Que el firmware LEA tres entradas por punta no significa que haya tres cámaras.** Las dos cosas
> son ciertas a la vez y confundirlas es lo que llenó este manual de cámaras que nadie compró:
>
> | | |
> |---|---|
> | **el firmware lee** | 3 entradas por punta — `PB0` (`J14`), `PB14` y `PB15` (`J16`) |
> | **se monta** | **1 cámara por poste**, en `J16` **p10** |
> | **la lista de compras dice** | **2 cámaras**, *«son las dos que el firmware lee hoy»* (`05_Funcional/15_Lista_de_Compras_Hardware.md`) |
>
> ⛔ **Y aquí había una pregunta abierta —*«cuántas cámaras van por poste, lo decide el
> responsable»*— que YA ESTÁ DECIDIDA** desde `D-2`. Se tacha para que nadie la vuelva a plantear.

> ⚠️ **Numeración:** el manual del 26/08 llamaba «Cámara 3» a la del Esclavo, porque contaba
> cuatro. Con dos cámaras, la del Esclavo es la **Cámara 2**. Si encuentra rotulado *«CAM 3»* en
> una caja o en un plano viejo, es esta misma.

### Pines que NO son entradas de cámara — no los cablee

| Pin | Lo que realmente es | Referencia |
|---|---|---|
| **`PB9`** (`J16` p5) | **`BOTON1` = `MANDO_A`**, canal `A` del mando de relés | símbolos `BOTON1` en `pines.h`, `MANDO_A` en `mando.cpp` |
| **`PB13`** (`J16` p8) | **`BOTON2` = `MANDO_B`**, canal `B` del mando de relés | símbolos `BOTON2` en `pines.h`, `MANDO_B` en `mando.cpp` |
| **`PB8`** | **`LED_TESTIGO`** — `R16` 1 kΩ → LED `D5`. **Salida, no bornera** | `pines.h:63` |
| `PA9` / `PA10` | `RS485_IN` — el MAX3485 `U2` y la bornera `J10`. **NO es el Bluetooth** | `pines.h:127-139` |
| `PB6` / `PB7` (`J17`) | **USART1 remapeado — aquí sí está el Bluetooth / ESP32** | `bluetooth.cpp:28` |

> ### ⚠️ 05/09: EL MANDO SE RETIRÓ COMO HARDWARE, Y ESO **NO** HACE SEGUROS ESTOS DOS PINES
>
> **`D-1` retiró el equipo, no el código** —y lo dice por escrito: *«el CÓDIGO no se toca»*—.
> `botones_actualizar()` sigue leyendo `BOTON1`/`BOTON2` en cada vuelta, con `pinMode(…, INPUT)`
> pelado y **activo en ALTO**, y sigue llamando a `mando_registrarPulso(MANDO_A/B)` en cada
> flanco. `mando.cpp` sigue reconociendo `A·A·A`, `B·B·B` y `A·B·A·B`.
>
> **O sea que el aviso de abajo NO caduca: vale hoy exactamente igual, y ahora sin ningún
> pulsador legítimo compitiendo por el pin.** La spec da `J16` p5 y p8 como **libres y sin
> cablear** (`05_Funcional/17_…`) — *libres de cobre*, **no** *libres de firmware*.

🔴 **Conectar un relé de cámara a `PB9` o `PB13` no inyecta «pulsaciones de menú»: compone
SECUENCIAS DE MANDO.** Tres pulsos de tráfico dentro de la ventana de 12 s hacen `A·A·A` o `B·B·B`,
y eso **cambia el modo del semáforo solo**: a Automático o a ámbar. Cuatro alternos en 18 s
(`A·B·A·B`) lo meten en **Modo Degradado**. No es una conexión inerte: es un semáforo que se
reconfigura con el tráfico.

⚠️ **`PA9`/`PA10` cambió de significado el 28/08 y este manual llevaba el dato viejo.** Decía que
eran *«hoy el bus de Bluetooth»*. Son `RS485_IN`; el Bluetooth se mudó a `PB6`/`PB7` (`J17`).
Tampoco son entrada de cámara.

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
 │  NO CABLEAR: PB9 = MANDO A · PB13 = MANDO B  │      │  NO CABLEAR: PB9 = MANDO A · PB13 = MANDO B  │
 │              (J16 p5 y p8 - secuencias)      │      │              (J16 p5 y p8 - secuencias)      │
 │  NO CABLEAR: PB8 = LED testigo D5 (salida)   │      │  NO CABLEAR: PB8 = LED testigo D5 (salida)   │
 │                                              │      │                                              │
 │  CAMARA -> J16 p10 = PB14  (UNA por poste)   │      │  CAMARA -> J16 p10 = PB14  (UNA por poste)   │
 │    M3 CERRADA 03/09: YA SE CABLEA.           │      │    M3 CERRADA 03/09: YA SE CABLEA.           │
 │    Contacto contra los 3,3 V de p9.          │      │    Contacto contra los 3,3 V de p9.          │
 │  J16 p12 = PB15 -> VACIO (D-2: una camara).  │      │  J16 p12 = PB15 -> VACIO (D-2: una camara).  │
 │  J16 p1 lleva 12 V CRUDOS: TAPARLO SIEMPRE.  │      │  J16 p1 lleva 12 V CRUDOS: TAPARLO SIEMPRE.  │
 │    p10 esta a 4,27 mm de los 12 V; p12 a     │      │    p10 esta a 4,27 mm de los 12 V; p12 a     │
 │    1,36 mm, que es el peor de los cuatro.    │      │    1,36 mm, que es el peor de los cuatro.    │
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

### Paso 3: Configurar la Salida de Relé (N/O ~~a 1 Segundo~~ **al mínimo que admita**)

> ## 🔴 DOS AVISOS QUE ESTE PASO NO LLEVABA, y los dos son sobre cifras que nos inventamos
>
> **1. El «`1 s`» ES CIRCULAR: sale de un manual NUESTRO, no de Hikvision.** Es **`A-7`** de
> [`DECISIONES.md`](../DECISIONES.md), y está medido: *«el manual oficial no publica ni un valor de
> `Delay` en 110 páginas»* — sólo la definición del campo. Peor: el `~1 s` vive hoy en **nueve
> sitios** —cinco comentarios de firmware y **dos packs con `PULSO_RELE_MS = 1000`** que salen
> **verdes contra un número que nadie ha medido**. *El instrumento certifica la invención.*
>
> 👉 **Lo que se hace en su lugar:** poner el `Delay` **al mínimo que la cámara admita**, **anotar
> el valor real que ofrezca** y medir el tiempo de cierre con un cronómetro. De ahí se **deriva**
> `SILENCIO_MS > Delay + rearme`, en vez de citarnos. Hay hueco para esa medida en la guía de banco
> (pasos 39–40).
>
> **2. El desplegable `NO`/`NC` está documentado para la ENTRADA de alarma, NO para la salida.**
> **`D-14`**, verificado sobre el manual de usuario `UD28967B-C` v5.7.20: la **salida** sólo expone
> `No.`, `Name` y `Delay` (p. 68); *Normally Open / Normally Closed* **no aparece ni una vez en las
> 110 páginas**, y de la **entrada** el manual documenta el valor `NO` y **`NC` no aparece nunca**.
> Así que *«se configura como `NO`»* queda **`SIN VERIFICAR`**: puede que no haya nada que elegir.
>
> 👉 **Consecuencia práctica, y es buena:** si la salida es `NO` de fábrica, es la que el firmware
> necesita —entrada activa en ALTO contra el pull-down de 10 kΩ— y no hay que tocar nada. **Si
> resultara ser `NC` y no se pudiera cambiar, NO se cablea**: el firmware vería demanda permanente
> en reposo y ausencia justo al pasar el vehículo. Es la inversión que ya costó `N-67`. **Se anota
> el hallazgo y se para.**
>
> 🔴 **Y la casilla que decide si este paso sirve para algo sigue abierta:** que la **analítica**
> pueda accionar el relé. El manual dice de `Trigger Alarm Output` que *«only supported by certain
> models»*. **Se mira con la cámara delante antes que nada** (paso 39 de la guía de banco). Si no se
> puede enlazar, el camino de `J16` no sirve y hay que ir por otro diseño — y existe una vía que
> **no** depende de esa casilla: la **entrada** de alarma de la cámara (`D-14`), que es el camino
> contrario, *el controlador le dice a la cámara que grabe*.

1. Ir a **Configuración > Eventos > Salida de Alarma** (*Alarm Output*):
   * **Estado por Defecto:** **`NO`** (*Normally Open*) — 🟡 **`SIN VERIFICAR` que sea elegible**, ver arriba.
   * **Duración del Pulso (`Delay`):** **el MÍNIMO que ofrezca el desplegable.** ⛔ ~~`1s`~~ — cifra
     inventada por nosotros (`A-7`). **Anote el valor real que aparezca**, que es el dato que falta.
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
  > 🟡 **Que `1A`+`1B` sean los dos extremos de UN contacto es DEDUCCIÓN, no lectura de diagrama**
  > (`D-14`). La *Quick Start Guide* dice que *«`1A` and `1B` … are three pairs of alarm outputs»* y
  > la ficha dice `1 input, 1 output`, luego a la nuestra le tocan `1A`+`1B`. **Lo que no existe en
  > ninguna de las dos fuentes es diagrama de cable ni régimen eléctrico** — ni tensión, ni
  > corriente, ni las palabras *dry contact* o *relay* en 110 páginas. **Compruébelo con el óhmetro
  > antes de dar tensión, no después.**
* **Reposo:** circuito abierto (sin pito).
* **Al cruzar un vehículo:** pita y se abre solo. ⛔ **Aquí decía *«~~durante ~1 segundo~~»*: esa
  cifra es NUESTRA, no de Hikvision (`A-7`).** **Cronométrelo y anote el número** — es justo el dato
  que falta para derivar `SILENCIO_MS`. **Un valor distinto de 1 s no es un fallo de la cámara.**

### 🧪 Ensayo 2: Demanda y Conmutación Sentido 1 (Maestro)

* Acercar un vehículo frente a la **Cámara 1**.
* **Criterio de Aceptación:** el Maestro recibe el pulso en **`PB0`** y ejecuta la secuencia de
  transición legal hasta **🟢 Verde Maestro**, manteniendo el Esclavo en Rojo.
* **Criterio negativo, obligatorio:** **con el contacto de la cámara ABIERTO y nadie delante del
  lente, el equipo NO debe registrar demanda.** Si la registra, el pin está flotando o la polaridad
  no es la que se cree: se para y se anota.
* **Segundo criterio negativo, y hoy es MÁS importante, no menos:** con `J16` p5 y p8 **vacíos**,
  el semáforo **no debe cambiar de modo solo** mientras la cámara dispara. Si cambia, **el relé está
  cableado a `PB9`/`PB13`** — el error de §0 — porque **ese código sigue vivo y sigue leyendo esos
  pines** aunque el mando ya no se monte (`D-1`).

> ⚠️ **La versión anterior de este ensayo decía *«pulsar los cuatro botones del menú»*. Ya no hay
> botones ni menú que pulsar.** `botonAceptar()` y `botonCancelar()` devuelven `false` siempre en las
> **dos** puntas, y desde el 05/09 **la pantalla tampoco se monta** (`D-17.bis`). Un criterio
> negativo que no se puede ejercer no prueba nada. ✅ **Se cita el símbolo, porque los números de
> línea que había aquí —`:280-281` y `:294-295`— ya habían caducado:**
>
> ```
> $ grep -n "^bool botonAceptar" 01_Firmware/Maestro/src/botones.cpp 01_Firmware/Esclavo/src/botones.cpp
> 01_Firmware/Maestro/src/botones.cpp:659:bool botonAceptar() { return false; }
> 01_Firmware/Esclavo/src/botones.cpp:645:bool botonAceptar() { return false; }
> ```
>
> ⛔ **Y aquí decía *«~~quedan dos: `A` en `J16` p5 y `B` en p8~~»*, que se leía como que hay dos
> pulsadores montados. NO LOS HAY** (`D-1`, 05/09): *«ya no tenemos mandos de A y B, sólo la app»*.
> Lo que queda en p5/p8 es **el código que los lee**, no un aparato.

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
*Emitido 26/08/2026 · **Corregido 28/08/2026** (§0: pines `PB9`/`PB13` erróneos y 4 cámaras → 2) ·
**Corregido 02/09/2026** (§0.ter: `J16` p10/p12 ya son entradas de cámara; `PA9`/`PA10` no son el
Bluetooth; el criterio negativo del Ensayo 2 ya no se podía ejercer; `AiBus` está retirado, no
huérfano).*
*Todo lo de este documento está **MEDIDO sobre el fuente y el esquemático**, y **ninguna línea
está VERIFICADA EN LA PLACA**: la sesión de banco es su primera comprobación física.*
