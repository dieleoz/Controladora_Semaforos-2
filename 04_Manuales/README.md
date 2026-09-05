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
