## Movido el 14/10 (`D-99`)

Salen LITERALES del fichero vivo, por cerradas o por cronica, para que baje del tope de 1.000 lineas: §2.3, §2.5,
§2.7, §2.8, §3.1, §3.2, §3.4, §3.5, §3.7, §3.8 (contestada en `SPEC_4` §6), §B y el Anexo. Alli queda una fila
por cada una en «Revisiones anteriores».

### 2.3 `botonCancelar()` es la unica salida de todos los modos — 🟢 REFUTADA EL 31/08 (N-100)

Hay `SET_MODO:MENU` y seis comandos mas desde `d34cfe2`. El censo de llamadores y el de comandos por punta, en el historico, §2.3.

### 2.5 `SET_RTC` puede rechazar en silencio y contestar `RESULT:OK` — ✅ CERRADO EN N-80 (`d34cfe2`)

Y caducado otra vez por `D-15`/`D-26`: **el `SET_RTC` lo contesta el PUENTE, no el STM32.** El analisis y las cinco ramas de entonces, en el historico, §2.5.

### 2.7 🟢 Del mando de reles no se retira equipo en servicio: nunca se compro el receptor

Sigue cierto y ahora es definitivo (`D-1`): no hay que subir a un poste a desmontar nada. Historico, §2.7.

### 2.8 El protocolo de 80 pruebas: 49 dejan de ser ejecutables — ⚠️ CUENTA CADUCADA

Caen **menos** de 49, y el numero nuevo **no se ha recontado seccion por seccion**, asi que no se publica. La tabla del 28/08 y que filas se mueven, en el historico, §2.8.

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

### 3.4 El minimo de tiempo por sentido (N75-1) — ✅ DECIDIDA EL 04/09: **3 minutos**

Vive en el C++ (`VERDE_MIN_MIN` / `ROJO_MIN_MIN`, `Maestro/include/limites_ciclo.h`) y **no en este documento**. El motivo del responsable, el coste de banco aceptado y el cierre de N75-2, en el historico, §3.4.

### 3.5 La Camara 1: se queda en `PB0`/`J14`, o se muda a `J16` — ✅ CERRADA (05/09 hacia `J16`; `D-25` y `D-27` el 11/09)

**Lo unico de este apartado que no caduca es su aviso electrico, y esta arriba en §1.7:** `p12` es el punto del conector **mas cercano a la red de 12 V** y **ninguna entrada de campo lleva proteccion en serie** (§3.6). El desarrollo —incluida la pregunta mal hecha que se le llego a plantear al responsable sobre una premisa sin medir— en el historico, §3.5.

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
| **1** | `05_Funcional/historico/15_Lista_de_Compras_Hardware.md` — hay dinero a punto de salir | historico, §B · Orden 1 |
| **2** | `05_Funcional/historico/10_Manual_Modulo_Bluetooth_Telemetria.md` — congelado, y manda enchufar un `HC-05` en `J17` | historico, §B · Orden 2 |
| **3** | `05_Funcional/historico/8_Procedimiento_Modo_Degradado.md` | historico, §B · Orden 3 |
| **4** | `05_Funcional/historico/MANUAL_MANDO_4_RELES.md` — describe un accionador que no esta | historico, §B · Orden 4 |
| **5** | `05_Funcional/historico/3_Protocolo_Pruebas_Rigurosas.md` — el acta que se firma | historico, §B · Orden 5 |
| — | el segundo bloque (doce documentos mas, sin dinero de por medio) | historico, §B |

## Anexo · Cambios que otros ficheros necesitan y que este documento NO ha hecho

**Los catorce puntos —con los que ya estan hechos tachados en su sitio, que es como se dice aqui que
una tarea no ha desaparecido en silencio— estan integros en el historico, «Anexo».** Los que siguen
ABIERTOS y no son de este documento, para que no se pierdan en la mudanza: el **pack que exija que
`ACC_AMBAR` siga siendo el unico armador de `ambarLocal`** (punto 2), el **pack que ate la polaridad
de los cuatro pines de boton al netlist** (punto 9), que **`maestro_10` mire tambien el `case` y no
solo el `==`** (punto 12), que **la caducidad del PIN se ejerza EN LA APK y no en un navegador**
(punto 13), y el **pack que ate el borrado del respaldo a su checksum** (punto 14, §3.9).


### Y el 14/10 (`D-99`), con las mismas dos cuentas

| | lineas |
|---|---|
| `17_Arquitectura_28-08_y_Decisiones_Abiertas.md` antes | **1137** |
| salen de alli (166 de texto movido aqui literal y 15 en blanco o `---` de junta) | **181** |
| entran alli (una fila por apartado en «Revisiones anteriores») | **12** |
| `17_Arquitectura_28-08_y_Decisiones_Abiertas.md` despues | **968** (= 1137 - 181 + 12) |
| `17_hist_Arquitectura.md` (este) antes | **3359** |
| `17_hist_Arquitectura.md` (este) despues | **3554** (= 3359 + 166 movidas + 29 de rotulos y esta cuenta) |
