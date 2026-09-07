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
> | 🛑 **La pantalla LCD no se monta** | Decidido el 28/08, confirmado el 05/09. **El código sigue compilando** (`D-6`: 271 comprobaciones cuelgan de él). No es lo mismo *«no existe»* que *«no se monta»* |
> | 🛑 **El mando de 4 relés no existe como hardware** | **`D-1`**: *«el equipo se opera SÓLO POR APP. Y su CÓDIGO no se toca»*. Nunca se compró receptor. ⚠️ **Pero `J16` p5/p8 SE SIGUEN LEYENDO**: lo que se cierre ahí entra al firmware |
> | 🛑 **§6 del manual de radios queda DEROGADA** | Mandaba diagnosticar el enlace leyendo `PRUEBA ALCANCE` en la pantalla del Maestro. Además de que no hay pantalla, **ese modo para el cruce en rojo fijo** y **en el Maestro el dato no sale por ningún sitio** — sí sale del Esclavo. Queda escrito como **pendiente de firmware** |
> | ⛔ **`J16` p1 lleva 12 V crudos** | **Taparlo es obligatorio en cada equipo que se monte** (`D-4`, N-120) |
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
> | 🟢 **El Esclavo YA tiene puerta al Modo Degradado** | `SET_MODO:DEGRADADO` por Bluetooth (`D-18`, `Esclavo/src/bluetooth.cpp:675`). Varias tablas decían que *«el mando es el ÚNICO actuador de modo»* en esa punta — con el mando retirado, eso la dejaba **sin ninguna** |
> | 🔴 **`J14` es ENTRADA; la talanquera va a `J15`** | Ningún manual mandaba cablear mal, **pero el esquemático rotula `J14` como «Puerta»**, que es justo la trampa. Queda escrito en `MANUAL_HARDWARE.md` §3.bis |
> | 🛑 **«9 entradas optoacopladas» estaba mal en las tres palabras** | Son **10 cadenas de SALIDA** (`Q1`–`Q10`, `U6`–`U15`); las entradas de campo son **5** y van **del borne directo al micro**. Y *«el opto aísla»* es medio cierto: **hay una sola masa** |
> | ⚠️ **Los números de línea citados caducan en bloque** | Se encontraron citas caducadas en cuatro manuales (`pines.h:92-93`, `botones.cpp:280-281`, `main.cpp:406/416/540`…). **Se cita el símbolo y se publica el `grep`**, no el número |
>
> **Manda `DECISIONES.md` (raíz)**, y en todo lo que sea **hardware medido** manda
> `05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md`. Ningún manual de esta carpeta
> gana a esos dos.

## 📄 Documentos principales

1. 👉 **[`MANUAL_EXACTO_RADIOS_E90_DTU.md`](MANUAL_EXACTO_RADIOS_E90_DTU.md)**
   Configuración de las radios industriales E90-DTU con los DIP switches `M0`/`M1` y
   `RF_Setting4.6.exe`, y topologías de canal. ✅ **Todo eso sigue vigente.**
   ~~y diagnóstico desde la pantalla del equipo.~~ 🛑 **§6 DEROGADA** — ver el aviso de arriba.
2. 👉 **[`MANUAL_MANDO_4_RELES.md`](MANUAL_MANDO_4_RELES.md)**
   ~~El mando de 4 relés que se opera **desde el suelo sin ver la pantalla**: secuencias,
   confirmación por destellos rojos, condiciones de rechazo y requisitos de compra.~~
   🛑 **HISTÓRICO / DISEÑO: el mando no se monta (`D-1`).** Se conserva porque es el motivo
   escrito de por qué su **código** no se borra —el veto de SFTY-21 no quedaría inerte, quedaría
   **abierto**— y porque documenta una barrera que **se quedó sin sujeto** (su §6).
3. 👉 **[`MANUAL_CONFIGURACION_BLUETOOTH.md`](MANUAL_CONFIGURACION_BLUETOOTH.md)** — 🔴 **es hoy
   el manual de la ÚNICA superficie de mando del equipo.** Consecuencia directa de `D-1`, y
   escrita como **`D-16`**: *sin teléfono no hay forma de operar el equipo*. No es una avería:
   es una propiedad declarada del sistema.
3. `E90-DTU(230SL37)_UserManual_EN_V1.5_fr.pdf` — datasheet de la variante **230 MHz**.
4. `E90-DTU(433C17)_UserManual_EN_v1.4.pdf` — datasheet de la variante **433 MHz**.
   ⚠️ **Ninguno de los dos corresponde a la banda declarada (170/172 MHz).** Lea la etiqueta
   del equipo físico antes de configurar: ver la advertencia del manual de radios, §7.
5. `Manual_de_Senalizacion_Vial.pdf` — estándar del Ministerio de Transporte de Colombia (2024).

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
