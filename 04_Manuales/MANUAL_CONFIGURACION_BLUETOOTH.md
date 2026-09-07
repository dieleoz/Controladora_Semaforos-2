# 📱 MANUAL TÉCNICO Y ESPECIFICACIÓN INTEGRAL DE LA APP MÓVIL Y BLUETOOTH (V9.0)

**Sistema:** Controladora de Semáforos Móviles de 3 Estados (Maestro y Esclavo V9.0)  
**Módulo de Diagnóstico:** Módulo Bluetooth Serial SPP / BLE (Estándar Probado en Proyecto Baliza)  
**Software Móvil:** App Android (.apk) con Frontend Reactivo Dark-Theme (Estándar IOT-VIAL)  
**Propósito:** Telemetría en tiempo real, caja negra de alarmas, test de banco, ~~sincronización Courier RTC~~ **puesta en hora del POSTE 1 y consulta del reloj de los dos** *(07/09: el Courier RTC queda **derogado por `D-20`** — ver `§3`, Pantalla 5)* y control desde el suelo con PIN  
**Verificación Hardware:** Esquemáticos KiCad `Controladora_Semaforos.kicad_sch`, `pines.h` y `MAPEO_TARJETA_KICAD.md`  
**Fecha de Emisión:** 26 de Agosto de 2026  
~~**Última revisión:** 31 de agosto de 2026 — ver el aviso de cabecera~~  
**Última revisión:** **7 de septiembre de 2026** — se corrigen contra el fuente **cinco cosas que
describían otro aparato**: el rótulo por `AT+NAME` (es un ESP32, no un `HC-05`), la plantilla del
`$STATUS` (le faltaban cuatro campos y sobraba un `BAT:` inventado), su cadencia (2 s, no 1), el
recuento de finales de `LEER_RTC`, y el diagrama de `PA9`/`PA10`, que ahora **se ve anulado desde
dentro del bloque**. Se añade el dato de campo de `D-16` —desvincular en Android para saltar de
poste—. **Nada se borra: lo superado queda tachado con su motivo y su fecha.**

---

> # 🔴 REVISIÓN DEL 31/08/2026 — LEA ESTO ANTES QUE NADA
>
> Este manual llevaba sin tocarse desde el 26/08 y **describía dos cosas que ya no son ciertas**. Lo
> superado se tacha con su motivo; no se borra, porque una corrección silenciosa se vuelve a proponer
> al mes siguiente.
>
> ## 1. 🛑 `FORZAR_ROJO` YA NO EXISTE EN EL ESCLAVO — Y ERA EL BOTÓN DE PÁNICO
>
> **MEDIDO el 31/08 sobre `01_Firmware/Esclavo/src/bluetooth.cpp`:**
>
> ```
>   Esclavo/src/bluetooth.cpp:157   if (strcmp(cmd, "CMD:FORZAR_ROJO") == 0) {
>   Esclavo/src/bluetooth.cpp:158     enviarTramaConCrc("$ERR,CMD:FORZAR_ROJO,DESC:RENOMBRADO_USE_AMBAR_EMERGENCIA");
>   Esclavo/src/bluetooth.cpp:176   } else if (strcmp(accion, "FORZAR_ROJO") == 0) {     <- la forma CON PIN
>   Esclavo/src/bluetooth.cpp:182     enviarTramaConCrc("$ERR,CMD:FORZAR_ROJO,DESC:RENOMBRADO_USE_AMBAR_EMERGENCIA");
> ```
>
> **Las dos formas —con PIN y sin PIN— contestan `$ERR`. El Esclavo no hace nada.** Este manual lo
> daba por válido en `§4.4` y en la tabla de `§ Qué acepta cada punta`, así que un operario que
> siguiera estas páginas creería haber detenido el cruce **sin haber detenido nada**. Es lo más grave
> que había aquí escrito y por eso encabeza la revisión.
>
> **El comando vigente en el Esclavo es `CMD:AMBAR_EMERGENCIA`** (`bluetooth.cpp:130` sin PIN,
> `:171` con PIN). Y el renombrado no es cosmético: lo que esa punta hace es
> `semaforo_iniciarFallo()` —**ámbar intermitente**—, no rojo. El nombre viejo prometía rojo y hacía
> ámbar, que es casi lo contrario; se corrigió el nombre, no el comportamiento.
>
> **En el MAESTRO `CMD:FORZAR_ROJO` se queda y allí SÍ hace rojo de verdad** (`Maestro/src/bluetooth.cpp:145`).
> Que las dos puntas usen literales distintos es lo correcto, porque hacen cosas distintas.
>
> ## 2. El módulo Bluetooth es hoy un **ESP32 de expansión**, y ya no entra por `PA9`/`PA10`
>
> El `§2` de este manual sigue dibujando el módulo sobre `PA9`/`PA10`. **Ese cableado ha quedado
> obsoleto** (ver el aviso dentro de la propia `§2`), y además el módulo SPP discreto ha sido
> **sustituido por un ESP32 de expansión** que entra por `J17`.
>
> ## 3. Y lo que NO cambia: **nada de esto ha pasado banco**
>
> En campo corre la **V8.4**. Todo lo descrito aquí está validado en simulador y arneses de PC.
> **Un manual corregido no es un permiso de carga.**

---

## 1. Arquitectura de la App — DECISIÓN CONGELADA

> ### 🔒 **Bluetooth Clásico SPP. No BLE. No Web Bluetooth. Y no es negociable sin reabrir este apartado por escrito.**

Esto se congela porque ya se fue por el camino equivocado una vez y hay que dejar constancia de por
qué, o se repetirá.

### Lo que se eligió

```text
 ┌─────────────────────────────────────────────────────────────────────────────┐
 │                    ARQUITECTURA DE LA APP MÓVIL (V9.0)                      │
 ├─────────────────────────────────────────────────────────────────────────────┤
 │ • ENLACE:        Bluetooth CLASICO, perfil SPP.                             │
 │                  UUID 00001101-0000-1000-8000-00805F9B34FB                  │
 │ • MODULO:        ESP32-WROOM-32 clasico (BT v4.2 BR/EDR + BLE) por J17.     │
 │                  Sustituye al modulo SPP discreto. NO HM-10, NO JDY-31.    │
 │ • EMPAREJADO:    Lo hace ANDROID en Ajustes, con PIN 0000 o 1234.           │
 │                  La app NO empareja: solo lista getBondedDevices().         │
 │ • IMPLEMENTACION: Puente NATIVO Android en el proyecto Capacitor            │
 │                  (BluetoothAdapter + createRfcommSocketToServiceRecord).    │
 │                  NO navigator.bluetooth. NO navigator.serial.               │
 │ • CONEXION:      UNA a la vez, explicita. SIN reconexion automatica.        │
 │ • INTERFAZ:      HTML5 + CSS + JS en WebView (Dark Theme IOT-VIAL).         │
 │ • ALMACENAMIENTO: LocalStorage / IndexedDB para logs y cruces.              │
 │ • AUTONOMIA:     100% offline. Opera en montana sin internet ni 4G.         │
 └─────────────────────────────────────────────────────────────────────────────┘
```

### Por qué SPP y no BLE — las tres razones, para que no se vuelva a preguntar

1. **`navigator.bluetooth` (Web Bluetooth) solo habla BLE.** No es que falle con un HC-05: **la API
   no existe para SPP**. La versión anterior de esta app usaba `navigator.bluetooth` y por eso *no
   abría el Bluetooth y no se conectaba a ningún dispositivo*. No era un error de programación: era
   la tecnología equivocada.
2. **La app probada en campo de esta casa es la de Baliza, y usa SPP.** `BluetoothAdapter`,
   `getBondedDevices()`, `createRfcommSocketToServiceRecord(00001101-…)`, con reintento por
   `createInsecureRfcommSocketToServiceRecord`. Está funcionando en la calle. **Se copia ese bloque,
   no se reinventa.**
3. **El técnico ya sabe usarlo.** Empareja en Ajustes de Android con `0000` o `1234` —el PIN del
   módulo, no el del semáforo— y la app le lista lo que ya está emparejado. Cambiar a BLE le cambia
   un flujo que domina, sin darle nada a cambio.

### Lo que queda PROHIBIDO, y por qué

| Prohibido | Motivo |
|---|---|
| `navigator.bluetooth` / Web Bluetooth | No puede ver un HC-05. Es la causa del fallo anterior |
| Módulos **solo BLE** (`HM-10`, `JDY-31`) | Obligarían a rehacer el puente nativo y cambiar el flujo del técnico |

### ✅ 31/08/2026 — el módulo está identificado, y la decisión de arriba se sostiene sin cambios

> **`BLQ-1` está CERRADO.** Si algún documento lo sigue dando por abierto, ese documento está
> caducado.

**MEDIDO:** el módulo del `J17` es un **`ESP32-WROOM-32` clásico** —`Xtensa LX6` de doble núcleo,
**`Bluetooth v4.2 BR/EDR + BLE`**—. `BR/EDR` es Bluetooth **clásico**, así que **hay perfil SPP** y
la app conecta **sin tocar una línea**: el puente nativo de Baliza vale tal cual.

**Y el firmware de ese ESP32 YA EXISTE y compila.** Vive en `01_Firmware/ESP32_Expansion/`
(`main.cpp`, `puente.cpp`, `despachador.cpp`, `enlace_stm32.cpp`, `transporte_app.cpp`,
`reloj_ds3231.cpp`, `trama.cpp`, `vigilante.cpp`) y la compuerta lo compila como una suite más —
**la cifra de flash se lee del acta de `evidencia/`, no de aquí**.

| | |
|---|---|
| **Qué es** | **Módulo de expansión, no un segundo controlador.** El STM32 sigue siendo el controlador del cruce |
| **Qué aporta** | El **Bluetooth** (sustituye al módulo SPP discreto) y un **reloj `DS3231`** con pila propia, en `GPIO21` (`SDA`) / `GPIO22` (`SCL`) — símbolos **`DS3231_SDA` / `DS3231_SCL`** de `ESP32_Expansion/include/contrato.h`. ⛔ ~~`contrato.h:142-143`~~ **el número estaba mal y por eso se cita el símbolo (07/09)**: hoy son las líneas 189-190, y mañana serán otras |
| **Qué NO hace** | 🛑 **No manda sobre las luces.** Es un puente: traduce y reenvía. La barrera de salidas sigue viviendo en `semaforo.cpp` del STM32 |
| **Se vigila a sí mismo** | ✅ **Tiene watchdog** — `esp_task_wdt` en `ESP32_Expansion/src/vigilante.cpp`, armado en `main.cpp` y alimentado en cuatro puntos del bucle. **Si el puente se cuelga, se reinicia solo y el enlace SPP se cae y vuelve.** Un técnico que vea desaparecer y reaparecer el equipo en la lista **no está viendo una avería de Bluetooth**. ⚠️ **No confundirlo con el `Repetidor`, que es OTRO programa de ESP32 y ése SIGUE SIN WATCHDOG** (medido 07/09: `grep -rniE "watchdog|esp_task_wdt|wdt" 01_Firmware/Repetidor` → **cero**) |
| **Por dónde entra** | `J17`, sobre el `USART1` **remapeado** a `PB6`/`PB7` — ver `§2` |

> ⚠️ **Que el ESP32 traiga también BLE no reabre nada.** La decisión congelada es *usar SPP*, y este
> chip lo tiene. No es una excepción al párrafo de arriba: es la confirmación de que se puede
> cumplir.
| Reconexión automática | El operario camina al Km 24, el teléfono se reengancha solo al Km 12 que sigue en rango, y la pantalla muestra un sistema vivo **que está a 12 km**. No hay forma de que lo note |
| Dos conexiones simultáneas | Físicamente imposible —los postes de una pareja están a cientos de metros y el Bluetooth alcanza 10-15 m— y Baliza ya usa un único socket. **Una a la vez es una propiedad de seguridad, no una limitación** |

### El flujo de campo, que es el de Baliza

```text
  1. Ajustes de Android > Bluetooth > emparejar > PIN 0000 o 1234
  2. Abrir la app > "Buscar" > lista los emparejados (nombre + MAC)
        SEM-7A3F-M   Maestro, Km 12
        SEM-7A3F-E   Esclavo,  Km 12
        SEM-C104-M   Maestro, Km 24
  3. Tocar uno > socket RFCOMM sobre SPP > leer/escribir lineas
```

~~El nombre visible del módulo lo fija el firmware con `AT+NAME`, así que **la lista de Android ya dice
quién es cada equipo antes de conectar**. El técnico lee; no adivina.~~

> # 🛑 CORREGIDO EL 07/09 — `AT+NAME` ES DE UN `HC-05`, Y AQUÍ NO HAY NINGUNO
>
> **Esa frase describía otro aparato.** El módulo del `J17` es un **ESP32** y el rótulo lo pone su
> propio firmware al abrir el perfil SPP, sin comandos `AT` y sin terminal:
>
> ```
> $ grep -n "ROTULO_PREFIJO\|ROTULO_PROVISIONAL" 01_Firmware/ESP32_Expansion/include/contrato.h
> 258:#define ROTULO_PREFIJO        "SEM-"
> 259:#define ROTULO_PROVISIONAL    "SEM-SIN-MATRICULA"
>
> $ grep -n "spp.begin" 01_Firmware/ESP32_Expansion/src/transporte_app.cpp
> 38:  abierto = spp.begin(rotulo);
> ```
>
> ## 🔴 Y LO QUE HAY QUE SABER ANTES DE SUBIR AL POSTE: **EL RÓTULO BUENO NO APARECE EN EL PRIMER ARRANQUE**
>
> El nombre lleva la **serie del STM32**, y esa serie sale del **silicio del micro** — el ESP32 **no
> la puede saber al encender**. Así que el puente la **aprende** del primer `$STATUS` que retransmite
> (`transporte_aprenderRotulo()`, llamado desde `puente.cpp`), la guarda en su memoria no volátil, y
> **la usa en la arrancada SIGUIENTE**. No se re-rotula en caliente, a propósito: cambiar el nombre
> SPP obliga a cerrar el perfil y tiraría la sesión del operario que puede estar dando una orden.
>
> | lo que el técnico ve en la lista de Android | qué significa | qué hace |
> |---|---|---|
> | **`SEM-SIN-MATRICULA`** | módulo **virgen**, o que **nunca ha oído hablar al STM32** | **no es una avería.** Conecte igual: la telemetría funciona. Deje el equipo encendido un minuto y **apague y encienda el ESP32**: en el siguiente arranque ya sale con su nombre |
> | **`SEM-<serie>-M`** / **`SEM-<serie>-E`** | rótulo aprendido: `M` = Maestro, `E` = Esclavo | leer, no adivinar |
> | **dos equipos con el mismo `SEM-SIN-MATRICULA`** | los dos módulos están vírgenes | **no se distinguen por el nombre.** Emparéjelos y arránquelos **de uno en uno** hasta que cada uno tenga el suyo |
>
> ⚠️ **El binario es EL MISMO en las dos puntas** —la letra final la decide el `NODE:` de la trama, no
> una opción de compilación—, así que no existe «el firmware del Maestro» del ESP32. Si alguna vez se
> compilan dos, el día que se crucen los postes **los dos se llamarían igual**.

> ### 🔴 DATO DE CAMPO DE ESTA SEMANA, y no está en ningún otro manual (`D-16`)
>
> **Hubo que DESVINCULAR el Maestro en Ajustes de Android para poder conectarse al Esclavo.** Android
> mantiene el enlace SPP con el primer emparejado y el segundo socket no abre. Con `D-1` retirado el
> mando, **el teléfono es la única superficie de mando del equipo** (`D-16`), así que esto no es una
> molestia de interfaz: es **quedarse sin poder operar el otro poste**.
>
> **Lo que se hace:** *Ajustes de Android > Bluetooth > el equipo que ya no se usa > Desvincular*, y
> luego conectar al otro. **La app no puede arreglarlo**: la decisión congelada de arriba dice
> *«una conexión a la vez, explícita»* y el emparejado **lo hace Android, no la app**.
>
> 🟡 **SIN VERIFICAR: si basta con desconectar en vez de desvincular.** Lo observado es que
> desvincular funcionó; nadie probó lo más barato. Se comprueba en banco antes de escribirlo como
> procedimiento.

### Lo que la app NO puede hacer, aunque el operario lo pida

**Al Esclavo no se le manda nada que abra paso.** Puede leer telemetría, ~~ajustar el reloj~~
**CONSULTAR el reloj sin cambiarlo (`CMD:LEER_RTC`)**, pedir
~~forzar rojo~~ **`AMBAR_EMERGENCIA`** —que es la dirección segura— y **solicitar paso**, que viaja
por radio al Maestro como una demanda: el Maestro decide, aplica el todo-rojo y ordena. Esa asimetría
—**el Esclavo pide y el Maestro decide**— vive escrita en la puerta única por la que entra una
demanda: `Maestro/include/demanda.h` y `Esclavo/include/demanda.h`.

> 🛑 **CORREGIDO EL 07/09 (`D-20`) — «AJUSTAR EL RELOJ» SALE DE ESTA LISTA.**
>
> **LA AUTORIDAD DE LA HORA ES EL ESP32, SIEMPRE Y PARA TODO.** La app le da la hora al **ESP32
> Maestro**; ése al **ESP32 Esclavo**; y el STM32 de cada punta la recibe **de su propio ESP32**.
> **Hay UNA sola fuente**, así que no hay desfase inicial que acotar. **La app NO pone la hora en el
> poste 2. Nunca.** Un `SET_RTC` dirigido al Esclavo **se rechaza**: no es una sincronización, **es
> una segunda fuente**. Fila **`D-20`** de `DECISIONES.md`.
>
> ```
> ESP32-M  ->  STM32-M  ->  radio  ->  STM32-E  ->  ESP32-E      (los STM32 son CARTEROS)
> ```
>
> ✅ **Lo que NO cambia: `CMD:LEER_RTC` se sigue mandando A LOS DOS POSTES** — leer no escribe nada,
> y con `D-20` construida es la única forma de comprobar que la siembra del Maestro llegó de verdad.
>
> 🔴 **DECIDIDA Y SIN CONSTRUIR. Medido el 07/09, corrido antes de publicarlo:**
>
> ```bash
> $ grep -c "ESCLAVO\|Esclavo\|esclavo" 01_Firmware/ESP32_Expansion/src/despachador.cpp
> 0
> ```
>
> El puente es **el mismo firmware en los dos postes** y el fichero que decide qué hacer con un
> `SET_RTC` **no nombra al Esclavo ni una vez**: hoy lo atiende venga por donde venga. **Falta
> también el mando ESP32 → STM32 que siembre la hora en la controladora** (el camino físico existe,
> `enlace_stm32.cpp`; el mando no).
>
> ⚠️ **Mientras `D-20` no esté construida NO hay ninguna otra vía de poner en hora el poste 2, así
> que hoy se sigue visitando y sincronizando allí. Es un ESTADO DECLARADO, no la arquitectura**, y
> muere el día que se construya.
>
> 🔵 **EL POSTE 2 SE PONE EN HORA EN LA PUESTA EN MARCHA, NO DURANTE LA AVERÍA.** Con `D-20` dentro,
> el único camino al reloj del poste 2 pasa por la **radio**, y la radio se cae justo cuando hace
> falta el Modo Degradado — que es el modo que **exige hora**. ✅ **No es un problema: su `DS3231`
> tiene pila y conserva la hora que ya tenía. Perder la radio no es perder la hora.** Lo que obliga
> es a ponerla **antes**: en la puesta en marcha, al cambiar la pila de ese módulo, y tras cualquier
> `OSCILADOR_PARADO_CAMBIE_PILA` leído allí.

> 🛑 **Corregido el 31/08:** decía *«forzar rojo»* y ese literal **ya no funciona en el Esclavo**.
> Ver el aviso de cabecera y `§4.4`. Lo que esa punta hace es **ámbar intermitente**, no rojo.

> 🛑 **Corregido el 01/09 — el puntero mandaba a la regla equivocada.** Decía ~~*«El detalle está en
> `OPTIMIZACIONES.md` § SFTY-27»*~~. **MEDIDO:** el apartado que lleva ese número se titula
> *«SFTY-27 — Matrícula de pareja: quién obedece a quién»* y está marcado **DISEÑO, NO
> IMPLEMENTADO** — no es la asimetría de la demanda. El número `SFTY-27` designa hoy dos reglas
> distintas y **renumerar es decisión del responsable**, así que aquí se hace lo único que no depende
> de esa decisión: **se apunta a donde la regla está escrita de verdad**, que es el fuente.

> **Consecuencia para el operario, que es lo que importa:** da igual en qué extremo esté. Los dos
> postes se operan igual y en la pantalla **no aparecen las palabras «maestro» ni «esclavo»** —
> aparece el sentido de la vía.

---

## 2. Diagrama de Conexión Físico y Desacoplo de Hardware (KiCad)

> # 🔴 31/08 — ESTE APARTADO DESCRIBE EL MONTAJE ANTERIOR. EL VIGENTE ES `J17` / `PB6`-`PB7`
>
> **MEDIDO** sobre el fuente: `static HardwareSerial SerialBT(PB7, PB6);` — **idéntico en las dos
> puntas**. Es el `USART1` **remapeado**, no un segundo puerto serie, y el STM32F103 solo lo saca
> **por un sitio a la vez**.
>
> | `J17` | pin del STM32 | al módulo |
> |---|---|---|
> | **2** | **`PB7`** — `USART1_RX` | `TXD` |
> | **3** | **`PB6`** — `USART1_TX` | `RXD` |
> | **6** | 3,3 V | `VCC` |
> | **7** | GND | `GND` |
>
> **`PA9`/`PA10` sigue siendo válido eléctricamente, pero NO SALE A NINGUNA BORNERA:** habría que
> soldar en las patas del micro o del `MAX3485 U2`. Queda como alternativa de laboratorio.
>
> ⛔ **Y `J16` NO es `J17`.** ~~`J16` es la botonera~~ → **`J16` fue la botonera; hoy es el
> conector de las CÁMARAS** (`p10` = `CAM_C_PIN`, `p12` = `CAM_D_PIN`, `D-2`/`D-3`), con `p5` y
> `p8` **libres de cobre pero todavía leídos por el firmware**. **Su posición 1 lleva 12 V
> crudos** —taparla es obligatorio en cada equipo (`D-4`, N-120)—; el módulo
> es de 3,3 V. Confundirlos lo quema sin aviso previo. El procedimiento para distinguirlos **con
> multímetro** —y el aviso de contar los pines desde el pin 1, porque símbolo y footprint no tienen
> el mismo número de posiciones— está en `05_Funcional/2_Manual_Hardware_y_Pruebas.md §8`. **Este
> manual no lo duplica a propósito.**
>
> Lo que sigue **no se borra**: era correcto para el montaje anterior y el desacoplo de `PA8` que
> describe **el firmware lo sigue haciendo**, aunque ya no sea necesario para el Bluetooth.

~~En la tarjeta controladora del semáforo, el módulo Bluetooth se conecta al puerto **`USART1` (`PA9` TX y `PA10` RX)**:~~

```text
 ##########  DIAGRAMA ANULADO -- NO EJECUTAR ESTE CABLEADO (07/09/2026)  ##########
 #  Ni el modulo ni los pines son los vigentes:                                  #
 #    - HC-05 / JDY-30: NUNCA LLEGARON Y YA NO SE PIDEN (linea A1 de la lista    #
 #      de compras, ANULADA el 28/08). Lo que hay es un ESP32-WROOM-32.          #
 #    - PA9 / PA10: NO SALEN A NINGUNA BORNERA. Habria que soldar en las patas   #
 #      del micro o del MAX3485 U2.                                              #
 #  EL MONTAJE VIGENTE ES J17 / PB6-PB7, en la tabla de arriba.                  #
 ##################################################################################
 ┌─────────────────────────────────────────────────────────────────────────────┐
 │       (SUPERADO) CONEXIÓN DE TELEMETRÍA BLUETOOTH EN TARJETA MADRE          │
 ├─────────────────────────────────────────────────────────────────────────────┤
 │                                                                             │
 │   MÓDULO BLUETOOTH (HC-05 / JDY-30)           TARJETA CONTROLADORA STM32    │
 │   ┌─────────────────────────────┐          ┌────────────────────────────┐   │
 │   │  [ VCC ] (3.6V - 6.0V)      ├────X─────┤► Pin 5V (o 3.3V)           │   │
 │   │  [ GND ] (Tierra)           ├────X─────┤► Pin GND (Tierra común)    │   │
 │   │  [ TXD ] (Transmisión)      ├────X─────┤► Pin PA10 (USART1 RX)      │   │
 │   │  [ RXD ] (Recepción)        ├────X─────┤► Pin PA9  (USART1 TX)      │   │
 │   └─────────────────────────────┘          └────────────────────────────┘   │
 │                                                                             │
 │   ANULADO: se conserva para reconocer una tarjeta ya cableada asi.          │
 └─────────────────────────────────────────────────────────────────────────────┘
```

> 🛑 **Marcado dentro del bloque el 07/09.** El aviso de cabecera de esta `§2` ya decía que el
> apartado está superado, pero **el dibujo no lo llevaba encima**: un diagrama de cableado se lee
> solo, sin el párrafo de arriba, y éste es de los que se ejecutan con un destornillador. La promesa
> de *«lo que sigue no se borra»* se cumple —sigue entero—; lo que faltaba era que **se viera anulado
> desde dentro**.
>
> ⚠️ **Y hay un riesgo de 12 V que no es de este apartado pero se cruza con él:** `J16` p1 lleva
> **12 V crudos** y **se TAPA en cada equipo que se monte** (`D-4`, N-120). El ESP32 es de 3,3 V.
> Confundir `J16` con `J17` lo quema sin aviso previo.

### ⚙️ Desacoplo Eléctrico del Transceptor MAX3485 (~~U3~~ **`U2`**) en Hardware:

> 🔧 **Corrección de chip, 31/08:** este manual decía **`U3`** donde va **`U2`**. Trazado red por red
> sobre `Controladora_Semaforos.kicad_sch`: **`U2`** es el MAX3485 del **`USART1`** (`RO`→`PA10`,
> `~RE`/`DE`→`PA8`, `DI`→`PA9`, par A/B por `J10`); **`U3`** es el del **`USART3`**, el de la **radio
> LoRa** (`PB10`/`PB11`/`PB12`, par A/B por `J12`). Se corrige dejando constancia porque una
> corrección silenciosa se vuelve a proponer al mes siguiente.
>
> ⚠️ **Y el matiz que este manual no decía: `PA8` gobierna A LA VEZ `~RE` (pin 2) y `DE` (pin 3).**
> Ponerlo en `HIGH` apaga el receptor —que es lo que se busca— **pero deja el transmisor
> ENCENDIDO**: `U2` vuelca la telemetría por `J10` de forma permanente y esa línea **no puede recibir
> nunca**. Hoy es inofensivo porque `J10` está vacío; el día que alguien cuelgue algo de `J10` **hay
> que tocar el código, no el cableado** (`01_Firmware/TROUBLESHOOTING.md`, lección del repetidor del
> 31/07/2026).

Dado que ~~`U3`~~ **`U2`** (`MAX3485`) está físicamente conectado a `PA9`/`PA10`/`PA8` en la PCB:
* **Manejo en Firmware:** El pin `PA8` (`RS485_IN_DE_RE`) se configura en **`HIGH` permanente**:
  $$\text{pinMode}(\text{PA8}, \text{OUTPUT});\quad \text{digitalWrite}(\text{PA8}, \text{HIGH});$$
* **Efecto Físico:** Al poner $\text{DE}/\overline{\text{RE}} = 1$, la salida del receptor $\text{RO}$ de `U3` pasa a **Alta Impedancia ($\text{Hi-Z}$)**, liberando completamente el pin `PA10` y evitando cualquier conflicto de corriente con el `TXD` del módulo Bluetooth.

---

## 3. Especificación de Pantallas y Módulos de la App

La App cuenta con **5 pantallas funcionales**, diseñadas con alto contraste para visibilidad bajo sol directo en carretera:

```text
 ┌─────────────────────────────────────────────────────────────────────────────┐
 │ 🚦 STATUS BAR: [● Conectado (👑 MAESTRO P1)] · [RSSI: -62 dBm] · [18:25:00] │
 ├─────────────────────────────────────────────────────────────────────────────┤
 │   [ 🗺️ ESTADO ]   [ 🎮 CONTROL ]   [ 🔔 EVENTOS ]   [ 🔬 TEST ]   [ ⚙️ RTC ] │
 └─────────────────────────────────────────────────────────────────────────────┘
```

### Pantalla 1: ESTADO (Monitoreo en Tiempo Real)
* **Semáforos Duales con Glow Dinámico:** Representación gráfica en vivo de Maestro (Sentido 1) y Esclavo (Sentido 2).
* **Anillo de Cuenta Regresiva SVG:** Círculo animado en tiempo real con indicador gigante en monospace del tiempo restante de verde/rojo (`T:<segundos>`).
* **Métricas Clave:** Calidad de enlace de radio inter-semáforo (%), tiempo de respuesta RTT (ms) y ~~nivel de batería (V)~~ **⛔ la batería NO se mide — ver abajo**.

  > ~~🔴 **AVISO — tres de esos campos NO son medidas.** `RF:` y `RTT:` son **literales** en el Esclavo
  > (`Esclavo/src/bluetooth.cpp:328`: `RF:98%%,RTT:85ms`) y `BAT:12.6` es literal en **las dos
  > puntas**. El `RF:98%` del Esclavo se emite igual **con la antena desconectada**. No se usan para
  > juzgar el enlace ni la batería, y **no se apuntan en un acta como si fueran medidas**.~~
  >
  > # 🛑 ESE AVISO ERA CORRECTO Y HOY ESTÁ CADUCADO (07/09) — EL FIRMWARE YA NO INVENTA CIFRAS
  >
  > **Lo que decía era verdad cuando se midió, y describe un defecto que YA SE ARREGLÓ.** Dejarlo
  > escrito así hace lo contrario de lo que pretendía: manda a desconfiar de un `RF:` del Maestro que
  > hoy **sí se cuenta**, y a fiarse de un `--` leyéndolo como cero.
  >
  > **MEDIDO el 07/09 sobre las plantillas reales, que son la fuente y no un recuerdo:**
  >
  > | campo | Maestro | Esclavo | qué es hoy |
  > |---|---|---|---|
  > | `RF:` | **cifra real** | **`--` fijo** | en el Maestro sale de contar latidos contestados. **Sigue sin ser un RSSI** |
  > | `RTT:` | **cifra real** | **`--` fijo** | ídem |
  > | `T:` | cifra real | **`--` fijo** | cuenta atrás; sólo la lleva quien arbitra el ciclo |
  > | **`BAT:`** | **`--` fijo** | **`--` fijo** | 🔴 **NO HAY MEDIDA DE BATERÍA EN NINGUNA PUNTA.** No existe un solo `analogRead()` en el firmware de las dos |
  >
  > 🟢 **Y el cambio de fondo, que es la lección: `--` significa *«todavía no lo sé»*, y `!` significa
  > *«llegó algo que no puede ser»*.** Ni uno ni otro es un cero. Un tablero que se queda quieto y lo
  > admite es honesto; el `BAT:12.6` de antes **le mentía a alguien de pie delante de un cruce**.
  >
  > ⚠️ **Consecuencia para quien redacta un acta: la batería del gabinete NO se puede apuntar desde la
  > app. Se mide con multímetro o no se apunta.** Detalle en
  > `05_Funcional/2_Manual_Hardware_y_Pruebas.md §8`.

* **Botón de Pánico:** ~~Forzado inmediato de All-Red ante emergencias viales.~~
  **31/08 — depende de la punta, y no es un detalle de interfaz:**
  * **Maestro** → `CMD:FORZAR_ROJO`, sin PIN. **Rojo total de verdad.**
  * **Esclavo** → `CMD:AMBAR_EMERGENCIA`, sin PIN. **Ámbar intermitente.** `FORZAR_ROJO` aquí
    contesta `$ERR` y **no detiene nada**.

### Pantalla 2: CONTROL (Comandos Protegidos por PIN)
* **Selector de Modos:** Automático, Manual, Ámbar de Seguridad y Parada de Emergencia.
* **Control de Tráfico Manual:** Botón táctil para alternar el turno vehicular desde el suelo respetando el tiempo de despeje All-Red normativo.
* **Seguridad Obligatoria:** Requiere validación del PIN de 4 dígitos (`1234`) antes de enviar comandos que alteren las luces.

### Pantalla 3: EVENTOS (Caja Negra y Registro Histórico)
* **Feed de Alarmas:** Registro cronológico de caídas de radio, transiciones de modo y fallas con timestamp del reloj RTC.
* **Compartir por WhatsApp:** Genera automáticamente el mensaje formateado para interventoría con ubicación y nodo.
* **Exportar a CSV:** Descarga el archivo de auditoría para archivo formal.

### Pantalla 4: TEST Y DIAGNÓSTICO EN TALLER
* **Test de Lámparas de 6 Segundos:** Secuencia de prueba de banco (2s Rojo ➔ 2s Amarillo ➔ 2s Verde) ejecutada a través de `semaforo_iniciarTestLeds()` bajo la barrera de seguridad de `semaforo.cpp` con semáforo fuera de servicio o Todo-Rojo controlado.

### Pantalla 5: AJUSTES & ~~ASISTENTE COURIER RTC~~ **RELOJ** *(el Courier queda derogado el 07/09 por `D-20` — ver abajo)*

> 🛑 **CORREGIDO EL 07/09 — a QUIÉN se le pone la hora cambió el 05/09 (`D-15`).**
> **Hay DOS relojes por cruce, uno por poste**, cada uno en su ESP32 con pila propia, y **el STM32
> no aporta ninguno de los dos**. Así que *«el reloj del nodo conectado»* es **el del puente de
> ese poste**, y el acuse llega con **`NODE:PUENTE`**. ~~**La app valida la hora en LAS DOS
> puntas.**~~
>
> > 🔴 **ESA ÚLTIMA FRASE SE ESCRIBIÓ AQUÍ LA MISMA MAÑANA DEL 07/09 Y `D-20` LA INVIERTE ESA
> > TARDE.** Es la mitad de `D-15` que `D-20` deroga, y es la afirmación más peligrosa que tenía
> > este manual porque **llevaba sello de corrección reciente**: se lee como recién medida.
> >
> > **LO VIGENTE: LA AUTORIDAD DE LA HORA ES EL ESP32, SIEMPRE Y PARA TODO.** La app le da la hora
> > al **ESP32 Maestro**; ése al **ESP32 Esclavo**; el STM32 de cada punta la recibe **de su propio
> > ESP32**. **Hay UNA sola fuente.** **La app NO pone la hora en el poste 2. Nunca** — un
> > `SET_RTC` dirigido al Esclavo **se rechaza**: no es una sincronización, **es una segunda
> > fuente**. Fila **`D-20`** de `DECISIONES.md`, 07/09.
> >
> > ```
> > ESP32-M  ->  STM32-M  ->  radio  ->  STM32-E  ->  ESP32-E    (los STM32 son CARTEROS)
> > ```
> >
> > ✅ **Lo que de la frase tachada SÍ sobrevive: la app sigue LEYENDO la hora en las dos puntas**
> > (`CMD:LEER_RTC`, `D-17`). **Validar leyendo, sí; validar escribiendo en el poste 2, no.**
> >
> > 🔴 **DECIDIDA Y SIN CONSTRUIR** —la medida está en `§2`, *«lo que la app NO puede hacer»*—, así
> > que **hoy al poste 2 se le sigue poniendo la hora visitándolo. Es un ESTADO DECLARADO, no la
> > arquitectura.**
>
> 🔧 **Y una precisión de hardware que esta caja tenía en singular y que ya costó cambiar piezas
> sanas: lo muerto es `Y2`, y sólo `Y2`.** ~~*«el STM32 no tiene ninguno (`Y2` muerto)»*~~ se lee
> como *«la controladora no tiene cristal»*, y es falso.
>
> | | qué es | cómo está |
> |---|---|---|
> | **`Y2`** | 32.768 kHz — el del **RTC** del STM32 (`LSE_CLOCK`) | 🛑 **confirmado muerto, `N-17`** |
> | **`Y1`** | 8 MHz — el cristal principal | 🟢 **montado en la placa** *(esquemático: `Value "8MHz"`, `in_bom yes`, `dnp no`)* y **sin avería declarada** |
>
> ⚠️ **Y el dato entero, para que nadie deduzca de más: el firmware NO usa `Y1`.** La tarjeta es
> `genericSTM32F103C8` y el `SystemClock_Config` del núcleo arranca con el **HSI**, el RC interno
> —`RCC_OSCILLATORTYPE_HSI` / `RCC_PLLSOURCE_HSI_DIV2`, en `generic_clock.c` del
> `framework-arduinoststm32`, medido el 07/09—. El reloj de sistema **no depende de ninguno de los
> dos cristales**, y por eso el equipo funciona con `Y2` muerto.
>
> 🔋 **La `CR2032` del STM32 NO sobra por esto.** Con `D-20` ya no es por la hora: alimenta el
> dominio de respaldo (`BKP->DR1..DR10`), donde viven la marca de sincronización y el indicador del
> Degradado — **el cómputo de las 48 h**. Símbolo `respaldo.h`. Sin ella, `respaldo_setup()`
> encuentra el contenido inválido y **borra**.
>
> ✅ **Y hay una herramienta nueva que esta pantalla ~~debe ofrecer~~ YA OFRECE: `CMD:LEER_RTC`
> (`D-17`)** *(comprobado el 07/09: la app tiene las **nueve** respuestas del puente en su tabla de
> traducción, `MOTIVO_NO_CONTEMPLADO` incluida, y calcula el desfase con el teléfono de referencia
> común — `desfaseSeg()`. **Los manuales decían siete; el firmware y la app coinciden en nueve, así
> que el número equivocado era el del papel**)* — leer el
> reloj **sin cambiarlo**, y **enseñar el desfase entre los dos postes**. Es mejor que sincronizar:
> *hasta ahora la única forma de leer el reloj era mandarlo, y con eso se perdía justo el dato que se
> buscaba*. **Los dos ESP32 no se hablan entre sí**, así que la comparación **sólo la puede hacer la
> app visitando los dos postes**.
>
> ✅ **07/09 — ESA ÚLTIMA FRASE SE PARTE EN DOS, Y SÓLO UNA MITAD SIGUE VALIENDO.**
> **«Los dos ESP32 no se hablan entre sí» es CIERTO y `D-20` lo usa de premisa**: por eso la
> **comparación** la tiene que hacer la app visitando los dos postes, y por eso la **hora** sólo
> puede viajar por la radio entre los STM32.
> 🛑 **Lo que NO se sigue de ahí es que la app tenga que PONER la hora en los dos.** Comparar es
> leer; poner la hora es escribir, y `D-20` decide que **eso, en el poste 2, no se hace nunca**. La
> consecuencia que este párrafo extraía era la equivocada.

* **Sincronización Directa de Hora:** Ajuste del reloj `DS3231` **del ESP32 del ~~poste conectado~~ POSTE 1** con la hora del teléfono móvil. ⛔ ~~del nodo conectado~~ — se leía como el reloj del STM32, que va sobre `Y2` y está confirmado muerto (`N-17`). 🛑 **07/09 (`D-20`): «poste conectado» incluía al Esclavo, y ahí está el defecto.** Esta orden es **del POSTE 1**; dirigida al poste 2 **se rechaza** — sería una segunda fuente de hora. ⚠️ **SIN CONSTRUIR:** hoy se sigue pudiendo, y hoy es la única vía que hay.
* 🔍 **Consulta de Hora (`LEER_RTC`) y desfase entre postes:** ✅ **`D-20` NO la toca — se sigue usando en LOS DOS POSTES.** Lee sin escribir, y distingue *«nunca se puso»*, *«oscilador parado, cambie pila»*, *«el bus no responde»* y *«escritura a medias»*. **Es lo que hay que usar antes de dar por muerto un módulo** — y con `D-20` construida será, además, **la única forma de comprobar que la siembra del Maestro llegó al reloj del poste 2**.
* ~~**Modo Asistente Courier RTC (Sincronización Puente sin Radio):**~~ 🛑 **DEROGADO EL 07/09 (`D-20`). Se conserva tachado con su motivo:**
  1. ~~*Paso 1:* Capturar hora y ciclo en el Poste Maestro.~~
  2. ~~*Paso 2:* Viajar hasta el Poste Esclavo (la App cronometra el tiempo de viaje).~~
  3. ~~*Paso 3:* Inyectar en el Esclavo la hora compensada ($\Delta t < 0.1\text{ s}$), permitiendo sincronizar el Modo Degradado sin cables ni radio.~~

  **El motivo: la hora que ese asistente inyectaba en el poste 2 NO era la del Maestro, era la DEL
  TELÉFONO** —durante todo el procedimiento al Maestro no se le manda nada—. Eso no es una
  sincronización: **son dos fuentes de hora**, que es justo lo que `D-20` prohíbe. **La compensación
  del viaje estaba bien; lo que estaba mal era de dónde salía la hora.** 🔴 **El botón sigue en la
  APK: `D-20` decide la autoridad, no ha retirado código. No se use.**

---

## 4. Estructura de Tramas y Protocolo Serie ASCII (Estilo NMEA con Checksum XOR)

Todas las tramas viajan a **9600 baudios (8N1)** y finalizan en `\r\n`.

### 4.1 Cálculo del Checksum NMEA (*XX)
El checksum se calcula aplicando la operación **XOR bit a bit** de todos los bytes contenidos entre el carácter `$` inicial y el asterisco `*` (ambos excluidos), formateado como dos caracteres hexadecimales en mayúsculas (`00` a `FF`).

### 4.2 Telemetría Periódica ($STATUS) — ~~Emitida cada 1 segundo~~ **cada 2 segundos**

> # 🛑 CORREGIDO EL 07/09 — LA PLANTILLA DE ABAJO LE FALTAN CUATRO CAMPOS, Y LA CADENCIA ES OTRA
>
> **Se conserva tachada porque una app o un parser escritos contra ella siguen por ahí**, y hay que
> poder reconocer de dónde salieron. Lo vigente son **las dos plantillas literales del fuente**, que
> no son iguales entre puntas y **por eso se publican las dos**:
>
> ```
> $ grep -n '"\$STATUS,NODE' 01_Firmware/Maestro/src/bluetooth.cpp 01_Firmware/Esclavo/src/bluetooth.cpp
> 01_Firmware/Maestro/src/bluetooth.cpp:1089:  "$STATUS,NODE:MAESTRO,SERIE:%s,MODO:%s,ESTADO:%s,T:%s,RF:%s,RTT:%s,BAT:--,HORA:%s,ESC:%s,PLUMA:%s,CAM:%s"
> 01_Firmware/Esclavo/src/bluetooth.cpp:1037:  "$STATUS,NODE:ESCLAVO,SERIE:%s,MODO:%s,ESTADO:%s,T:--,RF:--,RTT:--,BAT:--,HORA:%s,PLUMA:%s,CAM:%s"
> ```
>
> | campo | qué es | dónde sale |
> |---|---|---|
> | **`SERIE:`** | matrícula del equipo, leída del **silicio del STM32**. Es de donde el puente saca su rótulo SPP | las dos |
> | **`ESC:`** | **qué ve el Maestro del Esclavo.** Asimetría deliberada (N-149): la punta subordinada no publica un juicio sobre quien la manda | **sólo Maestro** |
> | **`PLUMA:`** | `ARRIBA` · `ABAJO` (N-153) | las dos |
> | **`CAM:`** | `OK` · `CIEGA` · `PEGADA` · `?` — **la PEOR de las dos cámaras**, no una por cada una (`A-13`, cerrada el 07/09) | las dos |
>
> 🔴 **Un parser que exija los campos en el orden viejo, o que dé por sentado que las dos puntas
> mandan lo mismo, se rompe con el Esclavo.** No es un campo de más: son **cuatro**, y en el Esclavo
> hay **tres huecos fijos** (`T:` `RF:` `RTT:`) que no existían.
>
> ⏱️ **Y la cadencia son 2000 ms, no 1000** — decisión del responsable del 04/09, con su cuenta al
> lado en el fuente: el único consumidor es `vigilarEnlace()` de la app y su cota son 5000 ms; a
> 1000 ms el peor segundo eran **528 B de los 960 B/s** que caben a 9600 8N1. **Una app que declare
> «equipo caído» a los 3 s ahora se equivoca.**
>
> ⚠️ **`BAT:` está fijado a `--` en el literal**, o sea que ni siquiera es un argumento: **no hay
> nada que medir**. Ver el aviso de la Pantalla 1.

~~$$\text{Formato: }\$STATUS,NODE:\langle N\rangle,MODO:\langle M\rangle,ESTADO:\langle E\rangle,T:\langle S\rangle,RF:\langle R\rangle\%,RTT:\langle T\rangle ms,BAT:\langle V\rangle,HORA:\langle H\rangle*\langle CRC\rangle\backslash r\backslash n$$~~

**Ejemplo Maestro en Modo Automático — ⛔ SUPERADO, ver la plantilla literal de arriba:**
```text
(SUPERADO 07/09 - le faltan SERIE, ESC, PLUMA y CAM, y el BAT es inventado)
$STATUS,NODE:MAESTRO,MODO:AUTO,ESTADO:V1_R2,T:24,RF:98%,RTT:82ms,BAT:12.6,HORA:18:25:00*4F\r\n
```

### 4.3 Trama de Alarma Inmediata ($ALARM) — Emitida ante incidentes
```text
$ALARM,NODE:MAESTRO,EVENTO:FALLO_RF_12S,CAUSA:TIMEOUT_LATIDO,ACCION:CAMBIO_A_AMBAR,HORA:17:54:58*3B\r\n
```

### 4.4 Comandos desde la App hacia la Controladora (Protegidos por PIN)

> ⚠️ **La lista de abajo estaba INCOMPLETA y con un comando que el Esclavo rechaza.** Se conserva
> tachada y debajo va la censada el 31/08. **Ninguna de las dos es un permiso: nada ha pasado banco.**

```text
 ┌─────────────────────────────────────────────────────────────────────────────┐
 │ (SUPERADO 31/08 - lista de 9 formas, ver la censada mas abajo)              │
 ├─────────────────────────────────────────────────────────────────────────────┤
 │ • CMD:PIN:1234:SET_MODO:AUTO\r\n            ➔ Pone el cruce en Automático.  │
 │ • CMD:PIN:1234:SET_MODO:MANUAL\r\n          ➔ Pone el cruce en Modo Manual. │
 │ • CMD:PIN:1234:SET_MODO:AMBAR\r\n           ➔ Pone el cruce en Ámbar Seguro.│
 │ • CMD:PIN:1234:FORZAR_ROJO\r\n              ➔ ROJO TOTAL DE EMERGENCIA.     │
 │ • CMD:PIN:1234:MANUAL:CAMBIAR_TURNO\r\n     ➔ Concede turno opuesto.        │
 │ • CMD:PIN:1234:TEST_LEDS\r\n                ➔ Inicia test de lámparas de 6s.│
 │ • CMD:PIN:1234:SET_RTC:YYYY-MM-DD,HH:MM:SS\r\n ➔ Ajusta reloj RTC.          │
 │   SOLO AL POSTE 1. Al poste 2 se rechaza (D-20, 07/09). Ver 5.2.            │
 │ • CMD:PIN:1234:SOLICITAR_PASO\r\n     ➜ Solo ESCLAVO: pide al Maestro. │
 │ • CMD:FORZAR_ROJO\r\n                    ➜ SIN PIN. Ver nota abajo.   │
 └─────────────────────────────────────────────────────────────────────────────┘
```

> # 🛑 EL CENSO DE ABAJO ES DEL 31/08 Y TIENE CUATRO CAMBIOS SIN RECOGER (07/09/2026)
>
> **Las tablas que siguen se conservan** —son correctas en casi todo y su método es bueno—, **pero el
> reloj cambió de dueño el 05/09 y el Esclavo ganó dos comandos.** Léase esto antes que ellas.
>
> | | qué cambió | fuente |
> |---|---|---|
> | 🔴 **`SET_RTC` ya NO lo contesta el STM32** | **`D-15`** (05/09): *«el reloj lo lleva el ESP32 de cada punta, y es el ÚNICO que contesta a `SET_RTC`»*. Las ramas de las **dos** puntas del STM32 **consumen la orden en silencio** —ni `$ACK` ni `$ERR`—, a propósito, para que no salgan **dos acuses opuestos a una sola orden**, los dos ciertos. 👉 **La tabla de «cinco ramas» de abajo describe al PUENTE, no al STM32** | `D-15` · medido 07/09 |
> | 🛑 **Y `SET_RTC` NO SE MANDA AL POSTE 2** | **`D-20`** (07/09), que **deroga la segunda mitad de `D-15`** —~~*«la app valida la hora en LAS DOS»*~~—: **la autoridad de la hora es el ESP32 Maestro y hay UNA sola fuente**. `ESP32-M → STM32-M → radio → STM32-E → ESP32-E`; los STM32 son **carteros**. Un `SET_RTC` dirigido al Esclavo **se rechaza**: no es una sincronización, **es una segunda fuente**. ⚠️ **Lo tiene que rechazar el PUENTE del poste 2 — el silencio del STM32 no basta**, porque no le dice al técnico que su orden no valía. 🔴 **SIN CONSTRUIR**, medido el 07/09: `grep -c "ESCLAVO\|Esclavo\|esclavo" 01_Firmware/ESP32_Expansion/src/despachador.cpp` → **`0`**, el mismo firmware en los dos postes y hoy lo atiende venga por donde venga. **Mientras tanto el poste 2 se sigue poniendo en hora visitándolo: estado declarado, no arquitectura** | `D-20` · medido 07/09 |
> | 🟢 **Existe `CMD:LEER_RTC`** | **`D-17`** (05/09): el reloj **se puede CONSULTAR sin cambiarlo**, y la app enseña **el desfase entre postes**. Lo contesta el puente con ~~siete~~ **NUEVE** finales distintos, uno por motivo *(contados el 07/09 en `despachador.cpp`: un `$ACK,RESULT:OK` y **ocho** `$ERR`)*. *«No hace falta que los dos relojes se pongan de acuerdo solos: hace falta poder ver si lo están.»* | `D-17` · recuento corregido 07/09 |
> | 🟢 **El ESCLAVO ganó `SET_MODO:DEGRADADO`** | **`D-18`** (05/09), que cierra el hueco `A-11`: esa punta se había quedado **sin ninguna puerta** al Modo Degradado al retirarse el mando. Se le dio la llave a una puerta que ya existía | `D-18` · medido 07/09 |
> | 🟢 **Y `CANCELAR_AMBAR`**, que la tabla del Esclavo no listaba | Es la contraria de `AMBAR_EMERGENCIA`, y **lee `mando_ambarLocal()`** para decidir — dos de las cinco llamadas vivas de esa bandera | medido 07/09 |
>
> ✅ **MEDIDO el 07/09** — se citan los símbolos, porque **los números de línea de las tablas de
> abajo están caducados en bloque** (el censo se hizo el 31/08 y el fichero ha crecido).
>
> 🔴 **Y NO SE RENUMERAN, porque esa cura ya falló aquí: los números que esta misma cabecera publicó
> la MAÑANA del 07/09 estaban caducados ANTES DE ACABAR EL DÍA.** Un commit de la tarde
> —`4b2841b`, que ancla las decisiones `D-x` en comentarios del fuente— los movió a todos:
>
> | símbolo | número publicado esta mañana | dónde está esta tarde |
> |---|---|---|
> | `SET_RTC_LO_ACUSA_EL_PUENTE` (Maestro) | `:726` | `:728` |
> | `SET_RTC_LO_ACUSA_EL_PUENTE` (Esclavo) | `:781` | `:808` |
> | `CANCELAR_AMBAR` | `:575` | `:585` |
> | `SET_MODO:DEGRADADO` | `:675` | `:694` |
>
> **Por eso los `grep` de abajo se publican SIN `-n`: lo que hay que poder repetir es la búsqueda, no
> el número.** Corridos el 07/09 por la tarde, salida literal:
>
> ```
> $ grep -h "SET_RTC_LO_ACUSA_EL_PUENTE" 01_Firmware/Maestro/src/bluetooth.cpp 01_Firmware/Esclavo/src/bluetooth.cpp
>     bluetooth_reportarEvento("APP_BLUETOOTH", "SET_RTC_LO_ACUSA_EL_PUENTE");
>     bluetooth_reportarEvento("APP_BLUETOOTH", "SET_RTC_LO_ACUSA_EL_PUENTE");
>
> $ grep -h 'strcmp(accion, "SET_MODO:DEGRADADO")\|strcmp(accion, "CANCELAR_AMBAR")' 01_Firmware/Esclavo/src/bluetooth.cpp
>   } else if (strcmp(accion, "CANCELAR_AMBAR") == 0) {
>   } else if (strcmp(accion, "SET_MODO:DEGRADADO") == 0) {
>
> $ grep -h "CMD_LEER_RTC\[\]" 01_Firmware/ESP32_Expansion/src/despachador.cpp
> static const char CMD_LEER_RTC[] = "CMD:LEER_RTC";
> ```
>
> 🔴 **Consecuencia de operación, y es la que hay que llevarse:** el `$ACK` de `SET_RTC` llega
> **`NODE:PUENTE`**, no `NODE:MAESTRO`. Una app o un técnico que espere el acuse de la punta **creerá
> que el comando se perdió**. *«El STM32 no contesta»* **no es un síntoma: es el diseño.**

#### 📋 Censo del 31/08 — lo que el MAESTRO despacha de verdad

**MEDIDO** con `grep` sobre `01_Firmware/Maestro/src/bluetooth.cpp`, rama por rama. Son **15 acciones
distintas** y **17 formas de trama aceptadas**, porque dos de ellas admiten además una entrada sin
PIN (`:168-170`).

| Forma de trama | Línea | Qué hace / qué contesta |
|---|---|---|
| `CMD:FORZAR_ROJO` | `:145` | **Sin PIN.** Rojo total. `$ACK,CMD:FORZAR_ROJO,RESULT:OK` |
| `CMD:SET_MODO:MENU` | `:168` | **Sin PIN.** Alias de la forma con PIN — no mueve luces a verde |
| `CMD:SET_MODO:ALCANCE` | `:168` | **Sin PIN.** Ídem |
| `CMD:PIN:1234:SET_MODO:AUTO` | `:177` | Modo Automático |
| `CMD:PIN:1234:SET_MODO:MANUAL` | `:182` | Modo Manual |
| `CMD:PIN:1234:SET_MODO:AMBAR` | `:187` | Ámbar de seguridad |
| `CMD:PIN:1234:SET_MODO:MENU` | `:191` | **Vuelve al panel.** Sustituye al botón *Cancelar* retirado. Contesta `RESULT:SALIENDO_TODO_ROJO` o `$ERR,…,DESC:YA_VUELVE_AL_MENU` |
| `CMD:PIN:1234:SET_MODO:ALCANCE` | `:212` | Prueba de alcance. `$ERR,…,DESC:EN_MARCHA_PARE_EL_MODO` si hay un modo corriendo |
| `CMD:PIN:1234:SET_MODO:INTELIGENTE` | `:223` | Modo Inteligente (cámaras). Mismo rechazo |
| `CMD:PIN:1234:SET_MODO:DEGRADADO` | `:234` | Modo Degradado. **Contesta el motivo concreto del rechazo**, no un no seco |
| `CMD:PIN:1234:FORZAR_ROJO` | `:253` | Igual que la forma sin PIN |
| `CMD:PIN:1234:MANUAL:CAMBIAR_TURNO` | `:257` | `$ERR,…,DESC:EN_TRANSICION_REINTENTE` si el coordinador no está en reposo |
| `CMD:PIN:1234:TEST_LEDS` | `:271` | `$ACK,…,RESULT:STARTING_6S` |
| `CMD:PIN:1234:SET_TIEMPOS:v,r,d` | `:275` | **El molde de cómo se contesta bien:** un `$ERR` por cada motivo (`FORMATO_INVALIDO`, `EN_MARCHA_PARE_EL_MODO`, `RANGO`) |
| `CMD:PIN:1234:SET_RTC:YYYY-MM-DD,HH:MM:SS` | `:295` | **Cinco ramas**, ver abajo |
| `CMD:PIN:1234:REINICIAR_RELOJ` | `:330` | `CRISTAL_OK_PONGA_LA_HORA` o `$ERR,…,DESC:SIGUE_PARADO_VEA_CONSULTA_RELOJ` |
| `CMD:PIN:1234:DEMANDA` | `:345` | `REGISTRADA`, o `$ERR` `SOLO_EN_MODO_INTELIGENTE` / `REPITA_EN_UNOS_SEGUNDOS` |

Cualquier otra cosa cae en `$ERR,CMD:DESCONOCIDO,DESC:COMANDO_NO_SOPORTADO` (`:363`).

#### 🕐 `SET_RTC` tiene ~~CINCO~~ **siete** ramas, y ninguna miente — **pero las contesta EL PUENTE**

> 🛑 **Corregido el 07/09.** La lección de abajo —*«un `$ACK` que no depende de lo que devolvió la
> llamada es una mentira con formato de éxito»*— **sigue siendo exactamente correcta y es lo mejor de
> este apartado**. Lo que caducó es **quién** la aplica: desde `D-15` el reloj vive en el ESP32 y es
> **el puente** quien contesta, con `NODE:PUENTE`. La tabla siguiente se conserva por su método;
> **los literales vigentes son los del puente**, y son más:
>
> | respuesta del PUENTE | significa |
> |---|---|
> | `$ACK,NODE:PUENTE,CMD:SET_RTC,RESULT:OK` | entró y quedó puesta |
> | `$ACK,NODE:PUENTE,CMD:SET_RTC,RESULT:HORA_PUESTA_SIN_PROPAGAR` | entró aquí, no viajó |
> | `$ERR,…,DESC:FORMATO_INVALIDO` | la trama no se pudo leer, o las cifras están fuera de rango |
> | `$ERR,…,DESC:SIN_RELOJ_NO_RESPONDE` | **el bus I²C está mudo.** No hay con qué contar el tiempo |
> | `$ERR,…,DESC:ESCRITURA_FALLIDA` · `NO_QUEDO_PUESTA` | se intentó y no se pudo verificar |
> | `$ERR,…,DESC:OSCILADOR_PARADO_CAMBIE_PILA` | el módulo está; la pila no |
> | `$ERR,…,DESC:MOTIVO_NO_CONTEMPLADO` | rama de cierre: **no se inventa un OK** |
>
> ✅ **Y su gemelo de sólo lectura, `CMD:LEER_RTC` (`D-17`)**, contesta `RESULT:OK` o uno de ~~seis~~
> **OCHO** `$ERR` distintos: `NUNCA_SE_PUSO_PONGA_LA_HORA`, `OSCILADOR_PARADO_CAMBIE_PILA`,
> `SIN_RELOJ_NO_RESPONDE`, `ESCRITURA_A_MEDIAS_REPITA_SET_RTC`, `MODO_12H_PONGA_LA_HORA`,
> `REGISTROS_INCOHERENTES`, `BARRERA_INCOHERENTE` y **`MOTIVO_NO_CONTEMPLADO`**. **Es la herramienta
> de diagnóstico del reloj**, y es la que hay que usar antes de devolver un `DS3231` por mudo.
>
> 🔴 **Corregido el 07/09, y el que faltaba es justo el que más importa.** El recuento *«seis»* salió
> de contar las filas de una tabla, no de contar las ramas del fuente:
> `REGISTROS_INCOHERENTES` y `BARRERA_INCOHERENTE` **son dos**, no una con barra en medio, y
> **`MOTIVO_NO_CONTEMPLADO` no estaba listado en ninguna parte**. Ése es **la rama de cierre**: la que
> impide que un caso que nadie previó salga disfrazado de `OK`. Un manual que no la lista deja al
> técnico sin saber qué hacer **cuando la lea en el teléfono**, que es exactamente el momento en que
> algo raro está pasando. *(Qué hacer: no es del reloj — es del firmware. Se anota la trama literal y
> se reporta; **no se cambia la pila ni el módulo por esta respuesta**.)*

##### 📕 La tabla del 31/08, conservada por su método *(los literales son los de entonces, del STM32)*

Un `$ACK` que no depende de lo que devolvió la llamada **es una mentira con formato de éxito**. Este
comando llegó a contestar `RESULT:OK` sin mirar nada; hoy contesta lo que pasó:

| Respuesta | Línea | Significa |
|---|---|---|
| `$ERR,CMD:SET_RTC,DESC:FORMATO_INVALIDO` | `:308`, `:318` | La trama no se pudo leer, o las cifras están fuera de rango |
| `$ERR,CMD:SET_RTC,DESC:SIN_CRISTAL_VEA_CONSULTA_RELOJ` | `:313` | **No hay con qué contar el tiempo.** El reloj no quedó puesto |
| `$ACK,CMD:SET_RTC,RESULT:HORA_PUESTA_SIN_PROPAGAR` | `:325` | La hora entró aquí, pero **no viajó al Esclavo** |
| `$ACK,CMD:SET_RTC,RESULT:OK` | `:327` | La hora entró y va camino del Esclavo |

> **Por qué esto importa en el poste:** con el cristal `Y2` confirmado muerto en hardware (N-17), la
> versión anterior decía que sí y **no ponía la hora** — y el técnico se iba del poste creyendo que
> lo había dejado puesto.

#### 🛑 Y lo que el ESCLAVO acepta, que NO es lo mismo

**MEDIDO** sobre `01_Firmware/Esclavo/src/bluetooth.cpp`:

| Forma de trama | Línea | Qué hace |
|---|---|---|
| `CMD:AMBAR_EMERGENCIA` | `:130` | **Sin PIN.** Ámbar intermitente + latch. `$ACK,…,RESULT:OK` |
| `CMD:PIN:1234:AMBAR_EMERGENCIA` | `:171` | Lo mismo, con PIN. Las dos entradas hacen lo mismo |
| `CMD:PIN:1234:SOLICITAR_PASO` | `:184` | **Pide**, no ordena. `PEDIDO_AL_MAESTRO` o `$ERR,…,DESC:REPITA_EN_UNOS_SEGUNDOS` |
| ~~`CMD:PIN:1234:SET_RTC:…`~~ | ~~`:215`~~ | 🛑 **CADUCADO (`D-15`, 05/09): esta punta ya NO contesta.** Consume la orden en silencio; **acusa el puente** con `NODE:PUENTE` |
| 🟢 **`CMD:PIN:1234:SET_MODO:DEGRADADO`** | **`:675`** | **NUEVO (`D-18`, 05/09).** Es la **única** puerta al Modo Degradado que le queda a esta punta: el menú es inalcanzable y el mando no existe. Llama a `degradado_entrar()`, **la puerta que ya estaba construida y probada** (`18/18` en el arnés de dos puntas), y **contesta el motivo concreto del rechazo**, no un no seco |
| 🟢 **`CMD:PIN:1234:CANCELAR_AMBAR`** | **`:575`** | La contraria de `AMBAR_EMERGENCIA`. **Lee `mando_ambarLocal()` para decidir**: si un operario dejó ámbar local puesto con el mando, esta orden **no lo saca de ahí** (SFTY-21) |
| ~~`CMD:FORZAR_ROJO`~~ | `:157` | 🛑 **RECHAZADO** — `$ERR,CMD:FORZAR_ROJO,DESC:RENOMBRADO_USE_AMBAR_EMERGENCIA` |
| ~~`CMD:PIN:1234:FORZAR_ROJO`~~ | `:176` | 🛑 **RECHAZADO** — mismo `$ERR`. **Las dos formas.** |
| ~~`CMD:PIN:1234:TEST_LEDS`~~ | `:202` | 🛑 **RECHAZADO** — `$ERR,…,DESC:NO_EN_SERVICIO_USE_EL_MAESTRO` |

Cualquier otra cosa: `$ERR,CMD:DESCONOCIDO,DESC:COMANDO_NO_SOPORTADO_EN_ESCLAVO` (`:260`).

> **El rechazo de `FORZAR_ROJO` se hace ENSEÑANDO EL NOMBRE BUENO, no en silencio.** Quien manda ese
> literal es alguien con una app o un manual anteriores al cambio —este manual, hasta hoy—, y lo que
> necesita no es enterarse de que el comando no existe, sino **de cómo se llama ahora**.

### 🛑 El ROJO DE EMERGENCIA no pide PIN, y es deliberado

`mando.cpp` lo dejó escrito hace meses para el mando de relés: **«asimetría deliberada: lo seguro,
fácil; lo peligroso, difícil»**. Detener el tráfico es la acción **segura** —el equipo cae a
todo-rojo—, así que ponerle una clave delante solo retrasa a quien está viendo el incidente.

**En el Maestro** se aceptan **las dos formas**: `CMD:FORZAR_ROJO` y `CMD:PIN:1234:FORZAR_ROJO` hacen
lo mismo. El PIN sigue guardando lo que **abre** paso o mueve luces.

> 🛑 **EN EL ESCLAVO NO — y era el error de pánico de este manual.** Allí las dos formas contestan
> `$ERR,CMD:FORZAR_ROJO,DESC:RENOMBRADO_USE_AMBAR_EMERGENCIA` y **no pasa nada**. La asimetría
> deliberada sigue viva en esa punta, pero **con otro literal**: `CMD:AMBAR_EMERGENCIA`, que también
> se acepta **sin PIN**, por la misma razón —quien está viendo el incidente tiene que poder pedir la
> caída segura aunque no se sepa el PIN—.

### 📋 Qué acepta cada punta, y no es lo mismo — **corregida el 31/08**

| Comando | Maestro | Esclavo | Por qué |
|---|---|---|---|
| `SET_MODO:AUTO` · `MANUAL` · `AMBAR` | ✅ con PIN | ❌ | El Maestro es el único que arbitra el ciclo |
| `SET_MODO:MENU` · `ALCANCE` | ✅ con PIN **y sin PIN** | ❌ | No abren paso: vuelven al panel o arrancan una prueba |
| `SET_MODO:INTELIGENTE` · `DEGRADADO` | ✅ con PIN | ❌ | idem — y el Degradado además pasa su propia puerta |
| `MANUAL:CAMBIAR_TURNO` | ✅ con PIN | ❌ | idem |
| `SET_TIEMPOS:v,r,d` | ✅ con PIN | ❌ | La configuración del ciclo la arbitra el Maestro |
| **`SOLICITAR_PASO`** | — | ✅ **con PIN** | **Pide**, no ordena: manda `CMD_DEMANDA` por radio y decide el Maestro |
| **`DEMANDA`** | ✅ con PIN | — | Equivalente al `SOLICITAR_PASO` del otro extremo, sobre el Maestro |
| ~~`FORZAR_ROJO`~~ **en el Esclavo** | ✅ **sin PIN**, hace rojo | 🛑 **RECHAZADO** — `RENOMBRADO_USE_AMBAR_EMERGENCIA` | Prometía rojo y hacía ámbar. **Se corrigió el nombre, no el comportamiento** |
| **`AMBAR_EMERGENCIA`** | ❌ | ✅ **sin PIN** *(y con PIN)* | **Éste es el botón de pánico del Esclavo.** Dirección segura |
| ~~`SET_RTC:…`~~ | 🛑 **ninguna de las dos contesta** — lo acusa el **PUENTE** (`D-15`) | 🛑 **Y AL POSTE 2 NO SE MANDA** (`D-20`, 07/09) | El reloj **vive en el ESP32**. Que el STM32 contestara producía **dos acuses opuestos a una sola orden**, los dos ciertos. 🔴 **07/09: eso resolvía el ACUSE, no la AUTORIDAD.** `D-20` decide que la hora del poste 2 **sale del Poste 1 por radio**, así que un `SET_RTC` dirigido al Esclavo **es una segunda fuente y se rechaza** — y **lo tiene que rechazar su PUENTE**, no consumirlo callado el STM32. ⚠️ **SIN CONSTRUIR**, ver `§2` |
| 🟢 **`LEER_RTC`** | — *(lo contesta el puente)* | — *(ídem)* | **`D-17`:** consulta el reloj **sin cambiarlo**, con 7 finales distintos. La app compara los **dos postes** y enseña el desfase |
| 🟢 **`SET_MODO:DEGRADADO`** | ✅ con PIN | ✅ **con PIN — NUEVO (`D-18`)** | Antes sólo el Maestro. Es la única puerta que le queda al Esclavo |
| 🟢 **`CANCELAR_AMBAR`** | ❌ | ✅ con PIN | Contraria de `AMBAR_EMERGENCIA`; **respeta el veto de `ambarLocal`** |
| `REINICIAR_RELOJ` | ✅ con PIN | ❌ | Diagnóstico del cristal `Y2` del STM32. ⚠️ **Con `Y2` muerto (N-17) y el reloj mudado al ESP32, su utilidad hoy es de diagnóstico histórico**: para el reloj que se usa, la herramienta es `LEER_RTC` |
| `TEST_LEDS` | ✅ con PIN | 🛑 **RECHAZADO** | Ver abajo |

### 🛑 Por qué el Esclavo rechaza `TEST_LEDS`

El test enciende 6 s de secuencia —rojo, ámbar y **verde**— sin mirar el estado del ciclo. Lanzado
sobre un Esclavo en servicio, ese verde sale **mientras el Maestro está dando paso al otro sentido**:
dos vehículos entrando de frente al tramo.

Y lo que costó ver: **conectarse al Esclavo *correcto* era igual de peligroso.** El fallo no es
equivocarse de poste —eso lo arreglaría una matrícula—, es que esa punta acepte mover luces. Por eso
la guarda no vive en la app: una app se actualiza, se instala otra, se usa una vieja. Vive en el
firmware, y la vigila el pack `esclavo_06_no_abre_paso`.

El Esclavo contesta `$ERR,CMD:TEST_LEDS,DESC:NO_EN_SERVICIO_USE_EL_MAESTRO` — un motivo legible, no
un silencio que el técnico leería como equipo colgado. **El test de lámparas se hace desde el
Maestro.**

> **Volverá al Esclavo** cuando exista un estado `FUERA DE SERVICIO` que el propio equipo conozca, no
> una promesa del manual.

### ✋ `SOLICITAR_PASO`: el funcional trabaja desde cualquier extremo

El PMT coloca a un funcional en el extremo que haga falta, y ese funcional **no tiene por qué saber
cuál de los dos postes es el Maestro**:

```text
   ORDEN     "ponte en verde"    -> el Esclavo la ejecutaria          NO
   PETICION  "hay demanda aqui"  -> viaja al Maestro por radio,
                                    el Maestro decide, aplica el
                                    todo-rojo y ordena                SI
```

Es **la misma demanda que manda la cámara**: un botón del funcional y un coche detectado significan
lo mismo. Con dos funcionales, uno en cada extremo, el Maestro **serializa** — ninguno concede nada,
los dos piden. Que pulsen a la vez tiene que ser aburrido, y así lo es.

Si la petición cae en la ventana de silencio de 3 s, el equipo contesta
`$ERR,CMD:SOLICITAR_PASO,DESC:REPITA_EN_UNOS_SEGUNDOS`. **No se finge un envío que no ocurrió:** si el
operario no lo sabe, vuelve a pulsar creyendo que no le hacen caso.


---

## 5. Operación en Corredor Vial (Un solo celular para múltiples cruces)

En un proyecto de carretera con múltiples cruces (ej. Km 12, Km 24, Km 38), el técnico opera toda la concesión con **un solo teléfono móvil**:

### 5.1 Identificación de Nodos y Roles
1. **👑 Nodo Maestro (Poste 1):** Controla el ciclo global, programas de verde/despeje y radio hacia el Esclavo.
2. **📡 Nodo Esclavo (Poste 2):** Opera subordinado al Maestro y reporta diagnóstico local de batería, focos y recepción RF.

### 5.2 ~~Modo Courier RTC (Sincronización Puente sin Radio)~~ — 🛑 **DEROGADO POR `D-20` (07/09)**

> 🔴 **Segunda copia del mismo procedimiento en este manual, y las dos quedan derogadas.** La otra
> está en `§3`, Pantalla 5.
>
> **NO ERA UNA SINCRONIZACIÓN: ERA UNA SEGUNDA FUENTE DE HORA.** La hora que inyectaba en el poste 2
> **no era la del Maestro, era la DEL TELÉFONO** — al Maestro no se le manda nada en todo el
> procedimiento. **La compensación del viaje era correcta; lo que estaba mal era de dónde salía la
> hora.**
>
> **Lo vigente (`D-20`, `DECISIONES.md`, 07/09): LA AUTORIDAD DE LA HORA ES EL ESP32.** La app se la
> da al **ESP32 Maestro**; ése al **ESP32 Esclavo**; el STM32 de cada punta la recibe **de su propio
> ESP32**. **Hay UNA sola fuente**, y **la app NO pone la hora en el poste 2. Nunca.**
>
> ```
> ESP32-M  ->  STM32-M  ->  radio  ->  STM32-E  ->  ESP32-E      (los STM32 son CARTEROS)
> ```
>
> 🔴 **DECIDIDA Y SIN CONSTRUIR** — la medida está en `§2`. **Mientras tanto, y sólo mientras tanto,
> al poste 2 se le sigue poniendo la hora visitándolo: ESTADO DECLARADO, no arquitectura.**
>
> 🔵 **Y la regla que este apartado del corredor vial necesita: EL POSTE 2 SE PONE EN HORA EN LA
> PUESTA EN MARCHA, NO DURANTE LA AVERÍA.** El único camino al reloj del poste 2 pasará por la
> radio, y la radio se cae justo cuando hace falta el Modo Degradado — que es el modo que **exige
> hora**. ✅ **Su `DS3231` tiene pila y conserva la hora: perder la radio no es perder la hora.**
>
> 🔴 **El botón `[ 🚀 Inyectar en Esclavo ]` SIGUE EN LA APK.** `D-20` decide la autoridad, no ha
> retirado código. **No se use.**

**Se conserva tachado, con su motivo y su fecha, para que no se vuelva a proponer:**

~~Cuando no hay cobertura de radio entre los dos postes:~~
1. ~~El técnico abre la App junto al **Maestro (Poste 1)** y toca **`[ 📸 Capturar Maestro ]`** (memoriza hora y ciclo).~~
2. ~~El técnico viaja en moto/vehículo hasta el **Esclavo (Poste 2)**; la App cronometra el viaje con su reloj interno de alta resolución.~~
3. ~~Al conectarse al Esclavo, toca **`[ 🚀 Inyectar en Esclavo ]`**; la App inyecta la hora exacta:~~  
   ~~$\text{Hora Inyectada} = \text{Hora Capturada} + \text{Tiempo de Viaje}$~~
   ~~logrando un desfase $\Delta t < 0.1\text{ s}$ para el Modo Degradado sin cables ni escaleras.~~
