# 📚 Carpeta de Manuales y Herramientas (`04_Manuales`)

Documentación oficial de hardware de radios y de interfaz, datasheets del fabricante y
ejecutables de configuración para PC.

## 📄 Documentos principales

1. 👉 **[`MANUAL_EXACTO_RADIOS_E90_DTU.md`](file:///d:/@Proyect/Controladora_Semaforos/04_Manuales/MANUAL_EXACTO_RADIOS_E90_DTU.md)**
   Configuración de las radios industriales E90-DTU con los DIP switches `M0`/`M1` y
   `RF_Setting4.6.exe`, topologías de canal, y diagnóstico desde la pantalla del equipo.
2. 👉 **[`MANUAL_MANDO_4_RELES.md`](file:///d:/@Proyect/Controladora_Semaforos/04_Manuales/MANUAL_MANDO_4_RELES.md)**
   El mando de 4 relés que se opera **desde el suelo sin ver la pantalla**: secuencias,
   confirmación por destellos rojos, condiciones de rechazo y requisitos de compra.
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
