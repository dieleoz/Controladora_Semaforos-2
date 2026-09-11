# 📚 Carpeta de Manuales y Herramientas (`04_Manuales`)

Documentación oficial de hardware de radios y de interfaz, datasheets del fabricante y
ejecutables de configuración para PC.

> ## 🛑 AVISO DE ESTADO (05/09/2026) — NO HAY PANTALLA Y NO HAY MANDO
>
> **Los dos documentos principales de esta carpeta describían interfaces físicas que ya no se
> montan.** Los dos llevan ahora su propia cabecera de estado: **léala antes que el cuerpo.**
>
> | | |
> |---|---|
> | 🛑 **La pantalla LCD no se monta** | Decidido el 28/08, confirmado el 05/09. Manda **`D-17.bis`**. **El código sigue compilando** y `Validacion_LCD` sigue dando `271/271` **sobre un framebuffer en el PC**. No es lo mismo *«no existe»* que *«no se monta»*. ⛔ **Aquí se citaba `D-6`, que está DEROGADA — y con el argumento que `DECISIONES.md` marca como FALSO** (*«271 comprobaciones cuelgan de él»* era motivo para no retirar la pantalla, y no lo era: cuelgan del **arnés**, que no necesita la pantalla física). Corregido el 07/09 |
> | 🛑 **El mando de 4 relés no existe como hardware** | **`D-1`**: *«el equipo se opera SÓLO POR APP. Y su CÓDIGO no se toca»*. Nunca se compró receptor. ⚠️ **Pero `J16` p5/p8 SE SIGUEN LEYENDO**: lo que se cierre ahí entra al firmware |
> | 🛑 **§6 del manual de radios queda DEROGADA** | Mandaba diagnosticar el enlace leyendo `PRUEBA ALCANCE` en la pantalla del Maestro. Además de que no hay pantalla, **ese modo para el cruce en rojo fijo** y **en el Maestro el dato no sale por ningún sitio** — sí sale del Esclavo. Queda escrito como **pendiente de firmware** |
> | ⛔ **`J16` p1 lleva 12 V crudos** | **Taparlo es obligatorio en cada equipo que se monte** (`D-4`, N-120) |
>
> ## ⛔ LOS CINCO AVISOS QUE NO SE PUEDEN PERDER — van ANTES de la acción que protegen
>
> **Las cámaras ya están compradas, así que estos documentos dejaron de ser diseño: son lo que
> alguien ejecuta con un destornillador.** Estos cinco van aquí, en la primera página de la
> carpeta, además de en el manual que los desarrolla.
>
> | | |
> |---|---|
> | 1️⃣ **El firmware nuevo va CARGADO Y VERIFICADO en la tarjeta ANTES de que nadie enchufe nada en `J16`** | El orden es **asimétrico**: firmware primero es seguro —el pin queda fijado por su pull-down y **un pin en 0 V no ejecuta nada**—; cableado primero **no lo es**: con el firmware viejo dentro, `PB14` sigue siendo *Aceptar* y cualquier hilo en p10 lo pulsa — 🔴 *11/09, `D-25`:* **y `PB15` sigue siendo *Cancelar*, así que la cámara 2 en p12 lo pulsa igual** (V8.4 `e303485`, el de campo). 🔴 **Un commit no protege de un destornillador: lo que levanta el bloqueo es la CARGA, no el merge** |
> | 2️⃣ **`J16` p1 lleva 12 V CRUDOS** a un conector de señal directa al micro | Sin opto, sin limitadora, sin clamp. **Taparlo es obligatorio en cada equipo que se monte** (`D-4`). Y el margen real de cobre contra esa red es de **1,36 mm** en `p12`, no los 10 mm del conector: ~~**si una cámara es más crítica, va en `p10`** (4,27 mm)~~ → 🔴 **11/09, `D-25`: `p12` lleva la cámara 2 de cada poste, o sea que el peor borne SIEMPRE lleva cable** |
> | 3️⃣ **`J14` es una ENTRADA del micro; la salida de la talanquera es `J15`** | `J14` es `PB0` a 3,3 V **sin opto, sin diodo, sin limitadora** — y **el esquemático la rotula «Puerta»**, que es la trampa. 🔴 **Un relé cableado a `J14` se DESCONECTA antes de energizar.** La pluma va a `J15` |
> | 4️⃣ **El borne NO está a 0 V en reposo: está a ~12 V** | Nueve de las diez cadenas llevan **pull-up de 1 kΩ + LED al riel de 12 V en el COBRE** — no se evita dejando un hilo sin poner. Y *«el opto aísla galvánicamente»* es **medio cierto**: hay **UNA sola red `GND` de 103 pads**, así que **lo que se cuelgue de esos bornes comparte la masa del controlador** |
> | 5️⃣ **`J16` p5 y p8 están VACÍOS y el código SIGUE leyendo sus flancos** | **Nada se cablea ahí** (~~`A-2` sin decidir~~ → `A-2` **cerrada el 05/09**: p5/p8 se quedan como el mando, sin cablear, y el fin de carrera va a `J14` — 🔴 *11/09: y el firmware lee `J14` como cámara, **conflicto abierto**, ver abajo*). Lo que se cierre compone secuencias del mando, y 🔴 **`A·A·A` —la única sin guarda— ARRANCA EL CICLO, o sea abre paso** (medido el 07/09, `MANUAL_MANDO_4_RELES.md` §8) |
>
> 🔴 **11/09 — `D-25`: CUATRO CÁMARAS, DOS POR POSTE, y las conexiones son DEFINITIVAS** (el
> responsable: *«mantener estas conexiones como definitivas»*). En cada poste: **cámara 1** entre
> `J16` p9 (3,3 V) y **p10** (`PB14`, `CAM_C_PIN`); **cámara 2** entre `J16` p11 (3,3 V) y **p12**
> (`PB15`, `CAM_D_PIN`), por el contacto seco `1A`/`1B`. **Talanquera en `J15`**: p1 = 12 V, p2 =
> drenador de `Q10` (**no es masa**), a la bobina de un relé cuyo contacto va a `OPEN` de la
> centralita. Deroga de `D-13` **sólo** *«una por poste / p12 vacío»*. **Lo que el que cablea tiene
> que saber, medido en el firmware de `648b62f`:**
>
> - **Las dos cámaras hacen LO MISMO** (`CAM_J16[2]`, un solo bucle). No hay una «de demanda» y
>   otra «de pluma».
> - **NINGUNA frena la pluma**: `escribirPines()` no lee cámaras; la pluma baja con un coche debajo
>   (el veto es `A-1.bis`, sin construir). **Y sube también con el ámbar intermitente**, no sólo con
>   verde (poste recién encendido sin enlace, radio perdida, Modo Ámbar).
> - **La app pone `CAM: OK` —*«las dos ven…»*— con que detecte UNA**, y una cámara que nunca detectó
>   no la anuncia el equipo. **Cada cámara se comprueba con el multímetro en su borne.**
> - **`J14`: `A-2` lo reserva al fin de carrera, pero el firmware lo lee como `CAM_DEMANDA_PIN`** —
>   un fin de carrera ahí daría demandas falsas—. **CONFLICTO ABIERTO, del responsable.**
>
> La guía de campo de estas cuatro cámaras es `05_Funcional/Camaras_Sisga_4x.html` (corregida el
> 11/09); el detalle, en `MANUAL_CONFIGURACION_CAMARAS_IA.md` y `MANUAL_HARDWARE.md` §3.bis.
>
> 🔴 **Y el sexto, que no es de cableado pero se paga igual: SIN TELÉFONO NO HAY FORMA DE OPERAR EL
> EQUIPO** (`D-16`). Ni ámbar, ni volver a automático, ni parar el cruce. **El teléfono es
> herramienta crítica** —va en la lista de la cuadrilla, con batería y con un segundo terminal
> emparejado—, y el emparejado se hace **antes de subir**.
>
> ## 🆕 REVISIÓN DEL 07/09/2026 — se cotejaron los ocho manuales contra `DECISIONES.md`, `05_Funcional/` y el fuente
>
> | | |
> |---|---|
> | 🔴 **`MANUAL_INSTALACION_RELOJ_DS3231.md` decía que NO hay quien lea el reloj. SÍ lo hay** | Su §8 enseñaba que *«un `DS3231` mudo es lo ESPERADO, no una avería»*. **Hoy sí es una avería:** el driver existe (`01_Firmware/ESP32_Expansion/src/reloj_ds3231.cpp`, `GPIO21`/`GPIO22`) y ese reloj es **el único del cruce** (`D-15`). Se diagnostica con **`CMD:LEER_RTC`** (`D-17`), que dice el motivo |
> | ✅ **`M3` está CERRADA desde el 03/09 — y dos manuales seguían bloqueando por ella** | `MANUAL_CONFIGURACION_CAMARAS_IA.md` y `MANUAL_USUARIO.md` §6.3 mandaban *«no cablear hasta `M3`»*. **`D-3`:** medida en cobre, pull-down de 10 kΩ real en las cuatro posiciones, `p10`/`p12` a 0 V. **Las cámaras se cablean** |
> | 🛑 **La cámara comprada NO es la que el manual nombraba** | Decía `DS-2CD3643G2-LIZSU`; la comprada es **`DS-2CD2683G2-IZS`** (`D-10`), y su ficha está en esta carpeta |
> | 🛑 **El «pulso de 1 s» del relé de la cámara es INVENCIÓN NUESTRA** | `A-7`: Hikvision **no publica ni un valor de `Delay` en 110 páginas**. Se pone al mínimo y **se anota el valor real**. Y el `NO`/`NC` de la **salida** está **`SIN VERIFICAR`** (`D-14`): sólo está documentado para la **entrada** |
> | 🛑 **`MANUAL_USUARIO.md` contradecía al manual de radios de esta misma carpeta** | Publicaba **1 copia** de ráfaga donde el de radios publica **3** desde el 01/08. Medido: `RF_BURST_COPIES 3` en las dos puntas. *Un manual que se contradice con su vecino es peor que uno equivocado de forma consistente* |
> | 🟢 **El Esclavo YA tiene puerta al Modo Degradado** | `SET_MODO:DEGRADADO` por Bluetooth (`D-18`) — símbolo `strcmp(accion, "SET_MODO:DEGRADADO")` en `Esclavo/src/bluetooth.cpp`. Varias tablas decían que *«el mando es el ÚNICO actuador de modo»* en esa punta — con el mando retirado, eso la dejaba **sin ninguna**. ⛔ *(aquí ponía `bluetooth.cpp:675`, escrito **esta misma mañana** y ya caducado por la tarde: hoy es `:694`.)* |
> | 🔴 **`J14` es ENTRADA; la talanquera va a `J15`** | Ningún manual mandaba cablear mal, **pero el esquemático rotula `J14` como «Puerta»**, que es justo la trampa. Queda escrito en `MANUAL_HARDWARE.md` §3.bis |
> | 🛑 **«9 entradas optoacopladas» estaba mal en las tres palabras** | Son **10 cadenas de SALIDA** (`Q1`–`Q10`, `U6`–`U15`); las entradas de campo son **5** y van **del borne directo al micro**. Y *«el opto aísla»* es medio cierto: **hay una sola masa** |
> | ⚠️ **Los números de línea citados caducan en bloque** | Se encontraron citas caducadas en cuatro manuales (`pines.h:92-93`, `botones.cpp:280-281`, `main.cpp:406/416/540`…). **Se cita el símbolo y se publica el `grep`**, no el número |
>
> ## 🔴 SEGUNDA PASADA DEL 07/09 — lo que la primera dejó, y la prueba de que renumerar no cura
>
> | | |
> |---|---|
> | 🔴 **`MANUAL_USUARIO.md` §2 describía un modo que NO EXISTE** | *«si el flujo baja al 50 % durante 4 h el sistema pasa a intermitente»*, en presente y dentro del *«Ground Truth»*. **`reloj_esHorarioNocturno()` tiene CERO llamadores** y el propio firmware dice *«aplazada»* (`N-3`). Es la Caja Negra de `N-73` otra vez |
> | 🔴 **En el MAESTRO, `A·A·A` es la única secuencia del mando que ABRE PASO y la única SIN GUARDA** | `MANUAL_MANDO_4_RELES.md` §8 decía de ella *«Seguro · ninguna protección, y no hace falta»*. Medido: arranca el ciclo sin comprobar nada, mientras `A·B·A·B` sí está validada. 👉 **Nada se cablea en `J16` p5/p8** |
> | 🔴 **`D-14` está DECIDIDA y NO TIENE CAMINO** | *«el controlador cierra un contacto y la cámara graba»*: **el firmware no mueve ninguna salida libre**. `J9`/`J11`/`J13` están fabricadas enteras y **declaradas y muertas** — cero `pinMode`, cero `digitalWrite`. Escrito en `MANUAL_HARDWARE.md` §5.bis.5 |
> | 🔴 **`MANUAL_USUARIO.md` §6.3 publicaba `~0,66 V` donde la medida dio `0 V`** | Y era **la fila que ese apartado manda aplicar**: quien fuera con el multímetro no la reconocería y pararía por la medida que autoriza. Los 0,66 V eran el valor **predicho para `INPUT_PULLUP`**, que ya no existe |
> | ⚠️ **El pin de cámara sobrante de `J16` NO está inerte** | El firmware lee **los dos** (`CAM_J16[2]`) y un flanco en cualquiera llama a `demanda_solicitar()`. **PIDE paso; no ordena** —su único consumidor es el Modo Inteligente— y así queda escrito, sin inflarlo · 🔴 **11/09: con `D-25` ya no hay pin sobrante — `p12` lleva la cámara 2** |
> | ⛔ **26 de 32 citas `fichero:linea` de `MANUAL_USUARIO.md` estaban caducadas —el 81 %—; 13 de 15 en `MANUAL_MANDO_4_RELES.md`; 6 de 9 en `MANUAL_HARDWARE.md`** | **Y dos de ellas se habían renumerado el 05/09 y volvieron a caducar en dos días.** Las movió `4b2841b`, que insertó las marcas `D-x` en los comentarios del firmware — **ni siquiera fue un cambio de comportamiento**. 👉 **El ancla que no caduca ya existe: `grep -rn "D-1" 01_Firmware/`** |
>
> **Manda `DECISIONES.md` (raíz)**, y en todo lo que sea **hardware medido** manda
> `05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md`. Ningún manual de esta carpeta
> gana a esos dos.

## 📄 Los siete manuales de esta carpeta

> ⛔ **Este índice listaba TRES de los siete, y con dos entradas numeradas `3.` — corregido el
> 07/09.** Los cuatro que faltaban son justamente los que traen las órdenes de taller: el del
> operario, el de hardware, el del reloj y el de las cámaras. **Un índice que no indexa la mitad de
> la carpeta manda a leer lo que ya está indexado.**

| | qué es | estado |
|---|---|---|
| 👉 **[`MANUAL_USUARIO.md`](MANUAL_USUARIO.md)** | El comportamiento del sistema —luces, despeje, fallas, cámaras, mando— y el manual **del operario** | ⚠️ **§2 describe un modo que NO EXISTE** (ver arriba) · el resto, al día contra `DECISIONES.md` |
| 👉 **[`MANUAL_HARDWARE.md`](MANUAL_HARDWARE.md)** | Topología, cableado RS485, pines, borneras y placa base | ✅ al día · 🔴 lleva los avisos de `J14`/`J15`, de los ~12 V en reposo y del hueco de `D-14` |
| 👉 **[`MANUAL_CONFIGURACION_BLUETOOTH.md`](MANUAL_CONFIGURACION_BLUETOOTH.md)** | 🔴 **el manual de la ÚNICA superficie de mando del equipo.** Consecuencia directa de `D-1`, escrita como **`D-16`**: *sin teléfono no hay forma de operar el equipo*. No es una avería: es una propiedad declarada del sistema | ✅ |
| 👉 **[`MANUAL_CONFIGURACION_CAMARAS_IA.md`](MANUAL_CONFIGURACION_CAMARAS_IA.md)** | Parametrización de la cámara comprada (`DS-2CD2683G2-IZS`, `D-10`) e instalación | ~~✅ **es hoy el entregable principal**~~ ⚠️ **11/09: el entregable principal es `05_Funcional/9_Manual_Parametrizacion_Camara_IA.md`** (`D-12`; este mismo manual lo dice en su cabecera: *«una fuente que gana a ésta»*) y la guía de campo es `05_Funcional/Camaras_Sisga_4x.html`. Toda la inteligencia vive en la configuración de la cámara (`D-12`). Corregido el 11/09 a `D-25` (cuatro cámaras, dos por poste) |
| 👉 **[`MANUAL_INSTALACION_RELOJ_DS3231.md`](MANUAL_INSTALACION_RELOJ_DS3231.md)** | El `DS3231` del ESP32 — **el único reloj del cruce** (`D-9`/`D-15`) | ✅ corregido el 07/09: un `DS3231` mudo **sí es una avería** |
| 👉 **[`MANUAL_EXACTO_RADIOS_E90_DTU.md`](MANUAL_EXACTO_RADIOS_E90_DTU.md)** | Radios E90-DTU: DIP `M0`/`M1`, `RF_Setting4.6.exe`, topologías de canal | ✅ vigente · ~~y diagnóstico desde la pantalla del equipo~~ 🛑 **§6 DEROGADA** |
| 👉 **[`MANUAL_MANDO_4_RELES.md`](MANUAL_MANDO_4_RELES.md)** | ~~El mando de 4 relés operado desde el suelo: secuencias, destellos, rechazos y requisitos de compra~~ | 🛑 **NO ES UN MANUAL DE OPERACIÓN.** El mando no se monta (`D-1`). **Se conserva en esta carpeta —no se archiva— por tres motivos medidos:** es un **aviso de cableado vivo** de `J16` p5/p8; es el **motivo escrito** de que `mando.cpp` no se borre (retirar el armador deja el veto de SFTY-21 **abierto**, y trece packs en `ABORTADO`); y **`A-11` salida (b) sigue abierta** proponiendo reponer pulsadores en esos mismos pines. **Su título no describe lo que hace y eso está pendiente de decidir** |

### 📎 Documentación de fabricante y estándares (PDF)

1. `DS-2CD2683G2-IZS_Datasheet_V5.5.113_20230303.pdf` y
   `UD28967B-C_Network-Camera_User-Manual_5.7.20_20240131.PDF` — la cámara comprada (`D-10`).
2. `…UD40284B…Quick_Start_Guide_20241115.pdf` — ⚠️ **sin capa de texto: cualquier búsqueda da cero
   por el FORMATO**, no porque falte el dato. Se mira renderizada.
3. `E90-DTU(230SL37)_UserManual_EN_V1.5_fr.pdf` — datasheet de la variante **230 MHz**.
4. `E90-DTU(433C17)_UserManual_EN_v1.4.pdf` — datasheet de la variante **433 MHz**.
   ⚠️ **Ninguno de los dos corresponde a la banda declarada (170/172 MHz).** Lea la etiqueta
   del equipo físico antes de configurar: ver la advertencia del manual de radios, §7.
5. `Manual_de_Senalizacion_Vial.pdf` — estándar del Ministerio de Transporte de Colombia (2024).
6. ⚠️ `DS-2CD2683G2-IZS_Ficha_Tecnica_y_Configuracion.docx` — **NO es del fabricante** y
   **contradice a la ficha oficial** (dice 256 GB de microSD donde la ficha dice 512). No se cita
   como fuente.

## 🛠️ Herramientas ejecutables para PC (Windows)

* **`RF_Setting4.6.exe`** — programa oficial de Ebyte para leer y escribir parámetros de las
  radios E90-DTU.
* **`XCOM V2.6.exe`** — terminal serie/RS485 para capturar tramas binarias de prueba.
* `校验文件(Hash).exe` y `CRC32 804438E0.txt` — utilidad de hash del proveedor y su suma.
  **No auditada; no se necesita para configurar las radios.**

## ⚙️ Configuración vigente de las radios (01/08/2026)

| Parámetro | Valor |
|---|---|
| Radios en servicio | **2, enlace directo. Sin repetidor** |
| Air Data Rate | **`2.4 kbps`** *(el antiguo `0.3 kbps` está derogado — saturaba el canal)* |
| Canal | `0` (170,0 MHz) en ambas |
| Potencia | `30 dBm` (1 W) · FEC `Enable` · modo `Transparent` |
| **DIP `M0` / `M1` en operación** | **AMBOS en `OFF`** — es el único modo válido |

> ⚠️ **Corrección conservada por su costo:** una versión anterior de esta carpeta indicaba
> `0.3 kbps` como tasa aérea y `M0=ON, M1=ON` como modo de configuración. **Ambos datos eran
> falsos.** El segundo dejó las cuatro radios en un modo donde oían pero no contestaban, y
> costó un día completo de campo. Los detalles están en el manual de radios.
