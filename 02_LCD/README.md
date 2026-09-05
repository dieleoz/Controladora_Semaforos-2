# 🖥️ Componente de Pantalla LCD ST7920 (128×64)

> # 🛑 LA PANTALLA NO SE MONTA. EL CÓDIGO SÍ SIGUE VIVO — y las dos cosas a la vez
>
> **Estado desde el 05/09/2026.** Esta carpeta describía una pantalla montada y en servicio
> **sin un solo aviso**. Ya no es cierto, y hay que decirlo antes que nada porque **un manual
> que describe algo retirado y no lo dice en la primera pantalla se lee como instrucción**.
>
> | | |
> |---|---|
> | 🛑 **El módulo ST7920 NO SE MONTA** en ninguna punta | Decisión escrita el **28/08/2026** (`roadmap.md`, «Lo decidido, con fecha»: *«Se retira la pantalla LCD de las dos puntas»*), confirmada por el responsable el **05/09**: *«la pantalla LCD ya no va, pues los pines y el equipo lo quitamos»*. **El motivo no fue la pantalla**: el Bluetooth necesitaba exactamente `PB6`/`PB7`, que eran suyos, y este PCB no admite ampliación (N-104) |
> | 🛑 **Los pines NO CONDUCEN.** Está medido, no supuesto | `lcd.cpp` construye el objeto U8g2 con **los cuatro pines en `U8X8_PIN_NONE`**, y `U8x8lib.cpp` pregunta `if (u8x8->pins[i] != U8X8_PIN_NONE)` antes de cada `pinMode` y cada `digitalWrite`. **No queda ni una escritura.** `PB3`/`PB4`/`PB5` están hoy en **alta impedancia** |
> | ✅ **El CÓDIGO NO se retira** | Es la decisión **`D-6`** de `DECISIONES.md` (04/09): *«La pantalla LCD NO se retira — 271 comprobaciones cuelgan de ella»*. `lcd.cpp` y `menu.cpp` **se siguen compilando** y `Validacion_LCD` sigue siendo **una de las 20 filas de la compuerta**, hoy en **271/271** |
> | 🛑 **La BOTONERA tampoco se monta** | `botonAceptar()` y `botonCancelar()` devuelven `false` desde el 31/08; los pulsadores `A`/`B` se retiraron como hardware el 05/09 (**`D-1`**). **Todo se opera por la app** |
>
> ```
> $ grep -n "U8X8_PIN_NONE" Maestro/src/lcd.cpp Esclavo/src/lcd.cpp | grep -i "u8g2("
> Maestro/src/lcd.cpp:74:static U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, U8X8_PIN_NONE, U8X8_PIN_NONE,
> Esclavo/src/lcd.cpp:92:static U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, U8X8_PIN_NONE, U8X8_PIN_NONE,
>
> $ grep -n "bool botonAceptar" Maestro/src/botones.cpp Esclavo/src/botones.cpp
> Maestro/src/botones.cpp:616:bool botonAceptar() { return false; }
> Esclavo/src/botones.cpp:615:bool botonAceptar() { return false; }
> ```
>
> ⚠️ **La distinción de arriba no es una sutileza, y equivocarla cuesta en las dos
> direcciones.** Decir *«la pantalla no existe»* es falso —hay 271 comprobaciones que la
> ejercen, y el símbolo `lcd_dibujarAlcance` sigue teniendo llamador—; decir *«hay pantalla»*
> es peor todavía, porque manda a un técnico a mirar un cristal que no está puesto. **Lo
> correcto es: existe en el código, no se monta en el poste, y no conduce ningún pin.**
>
> 🔴 **Nada de esta carpeta se BORRA.** Es el registro de por qué se puso la pantalla y por qué
> se quitó. Una vía borrada en silencio vuelve a proponerse, y este proyecto ya pagó por ello
> (N-104: *«sin esto, dentro de tres meses alguien vuelve a proponer la placa de expansión y
> nadie recuerda que se probó y no entraba»*).

Documentación y especificación de la pantalla gráfica autónoma ST7920 conectada por SPI a
las tarjetas controladoras STM32. ~~**Desde el 01/08/2026 hay pantalla en las dos puntas: el
Maestro y el Esclavo.**~~ → 🛑 **CADUCADO.** Hubo pantalla en el diseño de V8.7; **no se monta
en ningún equipo**. El código de las dos puntas sigue compilando (`D-6`).

👉 **[Ver especificación completa y mapa gráfico (`MANUAL_PANTALLA_LCD.md`)](MANUAL_PANTALLA_LCD.md)**
— lleva su propia cabecera de estado. **Léala antes que el cuerpo del documento.**

## 📋 Resumen del módulo — lo que describe es el DISEÑO, no el equipo montado

* **Controlador:** ST7920 (128×64 píxeles), un módulo por unidad. 🛑 **no se monta**
* **Protocolo:** ~~SPI serie de 3 hilos (`PB3` SCLK, `PB4` CS, `PB5` SID, `PB6` PSB=LOW, `PB7` RST)~~
  🛑 **CADUCADO Y PELIGROSO SI SE COPIA: `PB6`/`PB7` SON HOY EL ENLACE CON EL ESP32**
  (`SerialBT(PB7, PB6)`, USART1 remapeado, conector `J17`, desde N-76). Colgar un LCD de esos
  dos hilos enfrenta dos salidas *push-pull*. `PB3`/`PB4`/`PB5` siguen declarados y **en alta
  impedancia**: son los únicos GPIO libres del proyecto **con pista hasta bornera** (`J17` p4,
  p1 y p5).
* **Librería C++:** U8g2 (`U8G2_ST7920_128X64_F_SW_SPI`). ✅ sigue enlazada, con los pines a `NONE`
* **Implementación:** `01_Firmware/Maestro/src/lcd.cpp` y `01_Firmware/Esclavo/src/lcd.cpp`.
  ✅ **VIGENTE: los dos ficheros se compilan hoy** — no los borre.
* **Validación:** `01_Firmware/Validacion_LCD/` — ~~**83/83**~~ → ✅ **271/271** en el acta del
  05/09 (`MAESTRO 145/145 · ESCLAVO 126/126`). **Es la razón escrita de `D-6`.**

## 🆕 Qué cambió el 01/08/2026 (V8.7) — 📕 HISTÓRICO, se conserva

> 🛑 **Toda esta tabla describe una pantalla que ya no se monta.** Se conserva porque explica
> decisiones de layout que otros documentos citan, y porque el trabajo que describe **sigue
> compilando y sigue midiéndose**.

| | Antes | 01/08/2026 |
|---|---|---|
| Menú del Maestro | plano, 4 opciones | **dos niveles**: 4 en la raíz + 3 en `CONFIGURACION` |
| Pantallas nuevas | — | `AJUSTAR HORA`, `MODO DEGRADADO`, rechazo con motivo, ámbar con motivo |
| Esclavo | **sin interfaz** | menú de 2 opciones (`ESTADO`, `MODO DEGRADADO`) y sus vistas |
| Arnés de validación | 30/30 | 83/83 → **hoy 271/271** |
| Flash del Maestro | 64,2 % | 80,2 % → **hoy 87,5 %** (acta del 05/09) |

**Por qué el menú se partió en dos niveles:** una sexta opción en lista plana caía en
`y = 69`, fuera de los 64 px. Y el peligro no era que no se dibujara —hay salvaguarda—
**sino que el cursor sí podía llegar hasta ella**, dejando al operario seleccionando a
ciegas `MODO DEGRADADO`. Explicado con el caso concreto en §4.2 del manual.

> 🔴 **Y ese menú de dos niveles hoy NO SE PUEDE RECORRER, medido.** Bajar a
> `CONFIGURACION` exige `botonAceptar()`, que devuelve `false` siempre. El cursor sube y baja
> —`botonArriba()`/`botonAbajo()` siguen leyendo `PB9`/`PB13`— pero **no se puede seleccionar
> nada**. En el Esclavo pasa lo mismo y con consecuencia de seguridad: ver §6 del manual.

## ⚠️ Limitaciones conocidas — REVISADAS EL 05/09

* ~~**Las pantallas del Esclavo no las valida el arnés.** El gancho está puesto, falta una
  línea en `compilar.ps1`.~~ → ✅ **RESUELTO.** El acta del 05/09 publica
  `ESCLAVO 126/126`: el arnés **sí valida las dos puntas** desde entonces.
* **Ninguna de las pantallas nuevas se ha visto sobre una ST7920 real.** Sin prueba de banco.
  → 🛑 **Y ya no se verá: no se monta ninguna.** La limitación deja de ser un pendiente y pasa
  a ser una propiedad permanente del sistema.
* ~~Tres funciones de dibujo en `Maestro/src/lcd.cpp` son **código muerto**: nadie las llama.~~
  → ⚠️ **La cifra envejeció.** Hoy el censo de huérfanas del banco
  (`banco/packs/costura_10_funciones_muertas.py`) anota `lcd_dibujarConfigValor` como huérfana
  del Maestro, con su motivo medido; y `lcd_dibujarNoDisponible`, `lcd_dibujarTextoRecibido` y
  `lcd_dibujarConfigMinutos` **ya no están en `lcd.cpp`: sólo quedan sus declaraciones en
  `lcd.h`**. La cuenta vive en el pack, no aquí.

## 🔴 Lo que la retirada de la pantalla dejó sin cerrar, y no lo dice ningún otro documento

1. **`AJUSTAR HORA` (`MODO_HORA`) es un modo AL QUE NO SE PUEDE ENTRAR.** Su única puerta es
   la opción 1 del submenú `CONFIGURACION`, detrás de dos `botonAceptar()`, y **por Bluetooth
   no existe `SET_MODO:HORA`** — lo dice el propio firmware en `Maestro/src/bluetooth.cpp`:
   *«por Bluetooth no existe SET_MODO:HORA. El equipo estaba mandando a leer un instrumento
   que nadie puede abrir»*. El sustituto para poner el reloj sí existe y es `SET_RTC:` (más
   `REINICIAR_RELOJ`), por app.
   ```
   $ grep -n 'strcmp(accion, "SET_MODO' Maestro/src/bluetooth.cpp
   515:  if (strcmp(accion, "SET_MODO:AUTO") == 0) {
   520:  } else if (strcmp(accion, "SET_MODO:MANUAL") == 0) {
   527:  } else if (strcmp(accion, "SET_MODO:AMBAR") == 0) {
   570:  } else if (strcmp(accion, "SET_MODO:MENU") == 0) {
   591:  } else if (strcmp(accion, "SET_MODO:ALCANCE") == 0) {
   602:  } else if (strcmp(accion, "SET_MODO:INTELIGENTE") == 0) {
   613:  } else if (strcmp(accion, "SET_MODO:DEGRADADO") == 0) {
   ```
   **Siete ramas, y ninguna es `HORA`.** El buscador sabe encontrarlas: por eso el cero vale.
2. **`PRUEBA ALCANCE` es hoy un modo que PARA EL CRUCE y no entrega nada.** Ver la §6 de
   `04_Manuales/MANUAL_EXACTO_RADIOS_E90_DTU.md`, donde queda escrito como pendiente de
   firmware.

## 🔗 Documentos relacionados

* **`DECISIONES.md`** (raíz) — **`D-1`** (el mando no existe: sólo la app) y **`D-6`** (la
  pantalla **no** se retira del código). **Gana a este documento en todo lo que decida.**
* **`05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md`** — **gana a cualquier
  manual en todo lo que sea hardware medido**: pines, borneras, cobre.
* **`04_Manuales/MANUAL_MANDO_4_RELES.md`** — ~~la interfaz que se opera **sin ver esta
  pantalla**~~ → ⚠️ **el mando tampoco se monta** (`D-1`). El **código** sigue vivo y sigue
  leyendo `PB9`/`PB13`; el receptor de relés nunca se compró.
* **`05_Funcional/8_Procedimiento_Modo_Degradado.md`** — el procedimiento de campo.
