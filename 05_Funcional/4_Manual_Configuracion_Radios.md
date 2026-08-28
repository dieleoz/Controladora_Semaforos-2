# MANUAL DE CONFIGURACIÓN DE RADIOS INDUSTRIALES E90-DTU

Este documento indica los pasos exactos para configurar los parámetros de las radios industriales **EBYTE E90-DTU (RS485/232 $\leftrightarrow$ RF)** usando la computadora y la herramienta `RF_Setting4.6.exe`.

---

> ## ⚠️ CAMBIO OBLIGATORIO EN ESTA VERSIÓN — LEER ANTES DE PROBAR
>
> **La velocidad aérea (Air Data Rate) debe cambiarse de `0.3 kbps` a `2.4 kbps` en LAS CUATRO RADIOS.**
>
> Sin este ajuste el sistema **seguirá cayendo en fallo de comunicación al paso de cada ciclo**, y el
> **modo repetidor no funcionará en absoluto**. No es opcional ni cosmético: es la corrección principal
> de esta entrega. Las radios que no se reconfiguren reproducirán el fallo anterior.
>
> Todas las radios del enlace deben quedar con **el mismo** Air Data Rate, o no se comunicarán entre sí.

---

## 1. ¿Por qué se usa el Software `RF_Setting4.6.exe`?
El firmware del semáforo envía paquetes binarios ultra-cortos de 4 bytes. Configuraremos la radio a su
**MÁXIMA POTENCIA (1W / 30dBm)** y a una velocidad aérea de **2.4 kbps**.

### ¿Por qué ya no se usa 0.3 kbps?

A `0.3 kbps` el aire transporta unos 37 bytes por segundo. Cada orden del Maestro tarda alrededor de
**0.75 segundos por salto de radio**, y en modo repetidor son dos saltos por sentido. El viaje completo
de una orden y su confirmación (`GO_GREEN` → `ACK_GREEN`) se acercaba a **3 segundos**, rozando el
plazo máximo que espera el Maestro. Cualquier retraso lo superaba, el Maestro reintentaba, el reintento
chocaba con la confirmación que venía en camino, y el sistema declaraba fallo de comunicación.

**Ese es el fallo que se observaba justo cuando el Esclavo iba a pasar a verde, en los tres modos.**

A `2.4 kbps` ese mismo viaje baja a unos **0.35 segundos**, dejando un margen de más de 20 veces.

**¿Se pierde alcance?** Muy poco: cerca de 6 dB de sensibilidad. Con **1 W (30 dBm)** y antena en línea
de vista, el enlace especificado es de 6 km, mientras que una obra real mide entre 20 y 500 metros. El
margen sobrante es enorme y el cambio es seguro.

---

## 2. Configuración Paso a Paso (PC con Windows)

### Paso 1: Poner los DIP Switches en Modo Configuración

> 🚨 **CORRECCIÓN IMPORTANTE (31/07/2026).** Este manual indicaba antes `M0=ON, M1=ON` para el modo
> de configuración. **Era incorrecto.** Siguiendo ese dato las cuatro radios quedaron en un modo
> especial donde **la transmisión puede quedar deshabilitada mientras la recepción sigue activa** —
> radios que oyen pero no contestan. Costó un día de campo.

1. En la parte inferior de la caja metálica, mueva **`M0` a `ON` y deje `M1` en `OFF`**.
2. Conecte la radio al PC usando un convertidor USB a RS485 (`A` con `485_A`, `B` with `485_B`).
3. Conecte la fuente de 12V DC a la bornera `V+` y `V-`.

### Paso 2: Ejecutar `RF_Setting4.6.exe`
1. Abra `04_Manuales/RF_Setting4.6.exe`.
2. Seleccione el puerto **COM** del adaptador USB y BaudRate **`9600`**. Presione **`Open Port`**.
3. Haga clic en **`Read Option`**.
4. Configure exactamente estos valores:
   - **Baud Rate:** `9600`
   - **Air Data Rate:** **`2.4 kbps`** ⚠️ **OBLIGATORIO — cambiar en las 4 radios.** Si alguna queda en `0.3 kbps`, no enlazará con las demás y el fallo de comunicación persistirá.
   - **Power:** `30 dBm (1W)` *(Máxima fuerza)*
   - **FEC:** `Enable` *(Corrección de errores activa)*
   - **Transmission Mode:** `Transparent` *(Desactivar Fixed-Point)*
   - **Canal (Channel):**
     - *Modo Directo (2 radios):* Ambas en Canal `0` (170.0 MHz).
     - *Modo Repetidor (4 radios):* Maestro y Repetidor-Entrada en Canal `0`. Repetidor-Salida y Esclavo en Canal `10`.
5. Presione **`Write Option`** para guardar los cambios.

### Paso 3: Retornar a Modo Operación Normal
1. Desconecte la corriente de la radio.
2. Mueva los **DIP Switches `M0` y `M1` a la posición `OFF` (M0=0, M1=0)**.

> ⚠️ **Este es el único modo válido en operación.** Compruébelo en las **cuatro** radios antes de
> cerrar los gabinetes. Si una radio oye pero no contesta, lo primero que se revisa son estos dos
> switches.
3. Conecte la bornera `485_A` y `485_B` a la bornera `A` y `B` del semáforo.

---

## 3. Cambiar entre Modo Directo (2 radios) y Modo Repetidor (4 radios)

> **El firmware de las tarjetas NO cambia.** Maestro y Esclavo son agnósticos a la topología: no hay que
> recompilar ni reflashear las STM32 al pasar de 2 a 4 radios ni al revés. **Lo único que cambia es el
> CANAL de las radios.**

### Tabla de canales según topología

| Equipo | Modo Directo (2 radios) | Modo Repetidor (4 radios) |
|---|---|---|
| Radio del **Maestro** | Canal `0` — 170.0 MHz | Canal `0` — 170.0 MHz |
| Radio **B1** (entrada del repetidor) | — no se usa | Canal `0` — 170.0 MHz |
| Radio **B2** (salida del repetidor) | — no se usa | Canal `10` — 172.0 MHz |
| Radio del **Esclavo** | Canal `0` — 170.0 MHz | Canal `10` — 172.0 MHz |

### ⚠️ Al RETIRAR el repetidor (pasar de 4 radios a 2)

**No basta con desconectar el ESP32.** Si lo retira sin más, el Maestro queda en canal `0` y el Esclavo
en canal `10`: **frecuencias distintas, y no se comunicarán**. Ambos entrarán en Amarillo Intermitente
a los 25 s (SFTY-6).

- [ ] Reconfigurar la radio del **Esclavo** de canal `10` a canal `0`.
- [ ] Verificar que ambas quedan también en `2.4 kbps`, `30 dBm` y `FEC: Enable`.

### ⚠️ Al INSTALAR el repetidor (pasar de 2 radios a 4)

- [ ] Reconfigurar la radio del **Esclavo** de canal `0` a canal `10`.
- [ ] Configurar **B1** en canal `0` y **B2** en canal `10`.
- [ ] Flashear el ESP32 con `01_Firmware/Repetidor` (ver `5_Manual_Puente_ESP32.md`).

> **Por qué dos canales:** el repetidor escucha en una frecuencia y retransmite en otra. Si las cuatro
> radios estuvieran en el mismo canal, el repetidor se oiría a sí mismo y realimentaría el enlace.

### 📌 Verificar el modelo exacto de radio

Los canales y su frecuencia dependen del modelo. Las tablas anteriores asumen **paso de 200 kHz desde
170.0 MHz** (canal 10 = 170.0 + 10 × 0.2 = 172.0 MHz).

> Los datasheets incluidos en `04_Manuales/` corresponden a los modelos **E90-DTU(230SL37)** y
> **E90-DTU(433C17)**, que operan en 230 MHz y 433 MHz — **ninguno en 170 MHz**. Antes de la puesta en
> marcha, **confirme la referencia impresa en la caja metálica** de sus radios y contraste el mapa de
> canales con su datasheet. Si el modelo real es otro, la correspondencia canal↔frecuencia puede diferir.

---

## 4. Lista de Verificación antes de Probar en Campo

Repita el procedimiento completo en cada radio y marque:

- [ ] Radio **Maestro** — Air Data Rate en `2.4 kbps`, potencia `30 dBm`, FEC `Enable`, canal `0`
- [ ] Radio **Esclavo** — Air Data Rate en `2.4 kbps`, potencia `30 dBm`, FEC `Enable`, canal `0` (directo) o `10` (repetidor)
- [ ] Radio **Repetidor-Entrada (B1)** — Air Data Rate en `2.4 kbps`, canal `0` *(solo en modo repetidor)*
- [ ] Radio **Repetidor-Salida (B2)** — Air Data Rate en `2.4 kbps`, canal `10` *(solo en modo repetidor)*
- [ ] Las **cuatro** radios tienen el **mismo** Air Data Rate
- [ ] Los DIP Switches `M0`/`M1` quedaron en `OFF` en todas

> Si tras el cambio el enlace sigue cayendo, **no** vuelva a bajar la velocidad aérea: anótelo y
> repórtelo. Bajarla reintroduce exactamente el fallo que este ajuste corrige.

---

## 5. La sincronización horaria NO exige tocar las radios

Desde la V8.7 el Maestro envía **la hora y la configuración del ciclo** al Esclavo por el mismo enlace
(comandos `0x07`–`0x0F`), al confirmar la hora y luego **una vez por hora** mientras haya enlace.

**No hay nada que reconfigurar en la radio por esto:**

| Duda razonable | Respuesta |
|---|---|
| ¿Hace falta más ancho de banda? | **No.** Son las mismas tramas de 4 bytes de siempre, unas pocas al confirmar la hora y una tanda por hora |
| ¿Hay que cambiar el Air Data Rate? | **No.** Sigue en `2.4 kbps` |
| ¿El repetidor las deja pasar? | **Sí.** El puente valida **formato y CRC, no comandos**, así que las tramas nuevas lo atraviesan sin modificarlo |
| ¿Cambia algo en modo directo o con repetidor? | **No.** Las radios siguen siendo agnósticas al contenido |

> ⚠️ **Lo que sí importa para la sincronización es que el enlace esté SANO, no rápido.** El desfase
> medido incluye el tiempo de aire, así que un enlace al límite de cobertura mete ruido en la medida.
> Si la pantalla `PRUEBA ALCANCE` no marca cerca del 100 %, **sincronice antes de alejar el equipo**,
> no después.

> ⚠️ **Y una consecuencia operativa que conviene tener presente:** el Modo Degradado exige una
> sincronización de **menos de 2 horas** de antigüedad para poder activarse. Si el radio lleva medio
> día muerto, **ya no se puede entrar al Degradado** — la ventana para activarlo era mientras el
> enlace todavía respiraba. Ver `8_Procedimiento_Modo_Degradado.md`.
