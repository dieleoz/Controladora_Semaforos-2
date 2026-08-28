# Diagnostico_LCD — arbol de decision para N-22

Firmware minimo y aislado para la tarjeta del **Esclavo**. Solo hace dos cosas:
parpadear un testigo y pintar rellenos en la LCD. Sin radio, sin reloj, sin
respaldo, sin watchdog, sin menu.

No toca ningun otro proyecto. Se puede cargar y borrar sin consecuencias.

---

## 1. Que cargar

```
cd 01_Firmware/Diagnostico_LCD
C:\.platformio\penv\Scripts\platformio.exe run          # compila
C:\.platformio\penv\Scripts\platformio.exe run -t upload   # carga por SWD
```

La carga usa **`mode=UR`** (el mismo del Esclavo). Si falla con *"Unable to get
core ID"*, **reintente**: es cuestion de acertar la ventana tras soltar el reset.
Si se resiste, mantenga pulsado SW1 al lanzar la carga y sueltelo al ver
*"Download in Progress"*.

Todas las variantes se cambian en **un solo sitio**: el bloque `VARIANTES` al
principio de `src/main.cpp`. **Cambie una sola cada vez.** Si cambia dos y
funciona, no sabra cual era.

---

## 2. Que mirar

Hay **dos** instrumentos, y hay que mirar los dos.

### El testigo (PA0 = ROJO1, salida J3)

Es un pin de salida del semaforo, no un LED de placa. Si no tiene nada conectado
a J3, mida PA0 con el multimetro: debe alternar entre 0 V y 3,3 V.

Nada mas arrancar, el testigo **deletrea la configuracion cargada**:

| Secuencia | Significado |
|---|---|
| rafaga rapida de 10 | arranque de este firmware, ahora mismo |
| grupo 1: 1 / 2 / 3 destellos | PSB = BAJO / ALTO / SUELTO |
| grupo 2: 1 / 2 destellos | RST = controlado / sin controlar |
| grupo 3: 1 / 2 destellos | SPI = normal / lento |
| 2 destellos **largos** | `u8g2.begin()` retorno |

Luego entra en bucle: 1, 2, 3, 4 o 5 destellos cortos anuncian la fase, y durante
la fase el testigo **late** (parpadeo lento y regular).

### La pantalla

Cinco fases de 3 s, en bucle, de **mas visible a menos visible**:

| Fase | Patron | Para que sirve |
|---|---|---|
| 1 | pantalla entera **negra** | se ve aunque el contraste este casi al minimo |
| 2 | pantalla entera **blanca** | junto con la 1 demuestra que el contenido *cambia* |
| 3 | **damero** de 8x8 | ejercita el direccionamiento, no solo el dato |
| 4 | **mitad superior** negra | separa las dos mitades de RAM de la ST7920 |
| 5 | **texto** grande y fino | control de contraste |

---

## 3. Arbol de decision

### Pregunta 0 — ¿parpadea el testigo?

Esta es la pregunta que hasta ahora no se podia responder, y la que estaba
bloqueando el diagnostico: una LCD azul y muda se ve igual con la tarjeta
reiniciandose en bucle que con la tarjeta corriendo perfectamente.

- **NO parpadea nada.**
  El micro **no esta corriendo este firmware**. No busque nada en la pantalla
  todavia. Causas por orden: la carga no llego a completarse (relea la salida del
  programador), alimentacion, o el micro se reinicia antes de `setup()`.
  → Recargue y confirme *"Mass erase successfully achieved"* y
  *"NVM size: 64 KBytes"*. Si dice **128 KBytes (default)** en un chip de 64 KB,
  la carga NO fue buena.

- **Parpadea la rafaga de 10 pero luego se repite la rafaga una y otra vez.**
  Bucle de reinicio. El micro arranca y algo lo reinicia antes de llegar al
  bucle principal. Como aqui no hay watchdog ni radio ni reloj, sospeche de
  alimentacion: mida 3,3 V bajo carga.

- **Parpadea la firma completa y luego SE QUEDA QUIETO.**
  El micro se cuelga **dentro de `u8g2.begin()`** (los 2 destellos largos no
  llegaron). Eso apunta a la linea de **RST**.
  → Vaya a la **Pregunta 2**.

- **Parpadea, da los 2 destellos largos y sigue latiendo en bucle.**
  El micro corre y esta enviando a la pantalla. **La tarjeta esta sana.** El
  fallo esta en la pantalla, su cableado o sus niveles.
  → Vaya a la **Pregunta 1**.

- **El latido SE PARA en mitad de una fase.**
  Se cuelga enviando datos a la pantalla. Sospeche de un corto en SCLK, SID o CS
  contra masa o entre si. Mida continuidad de PB3, PB4 y PB5 contra masa con el
  modulo desconectado.

### Pregunta 1 — ¿se ve algo en pantalla?

- **Se ven las fases 1 a 4 (rellenos) pero NO la 5 (texto).**
  **La pantalla funciona.** Lo que falla es el **contraste**: el potenciometro
  del modulo o la tension de V0. Esto explicaria N-22 entero, porque
  `lcd_dibujarBienvenida()` solo pinta texto fino y con el contraste bajo un
  texto fino es invisible mientras un relleno todavia se distingue.
  → Ajuste el potenciometro del modulo mientras corre la fase 5.

- **Se ve el negro y el blanco, pero el damero sale corrido, torcido o repetido.**
  El dato llega y el **direccionamiento** esta mal. No es un problema de niveles.

- **Se ve la mitad de arriba y no la de abajo (o al reves).**
  Fallo en el recorrido de las dos mitades de RAM, o una linea de datos del panel.

- **No se ve ABSOLUTAMENTE NADA, ni siquiera el negro total.**
  El controlador no esta procesando comandos. → **Pregunta 2**.

### Pregunta 2 — aislar el sospechoso

Pruebe **en este orden**, una variante cada vez, recompilando y recargando:

#### 2a. `VARIANTE_LENTA 1` — la hipotesis de niveles

Es la variante mas probable y la que hay que probar primero.

Con el modulo alimentado a **5 V**, el V_IH garantizado del ST7920 es
0,7·VDD = **3,5 V**, y el STM32 entrega **3,3 V**. Esta fuera de especificacion
**en las dos puntas**: que el Maestro pinte demuestra que *aquel* chip tolera, no
que el diseno sea correcto. Un margen asi falla de forma sucia — unas unidades si
y otras no — y depende de los flancos.

- **Empieza a pintar (aunque refresque despacio):**
  **CONFIRMADO: el problema son los umbrales**, no el cableado ni el firmware.
  La entrada del ST7920 no alcanza sus 3,5 V a velocidad normal y solo discrimina
  los bits cuando se le da tiempo.
  **La solucion NO es de software.** Es una de estas dos:
  1. Alimentar el modulo a **3,3 V** en lugar de 5 V (baja el V_IH a 2,31 V), o
  2. Poner un **adaptador de nivel** en SCLK, SID y CS.
  Dejar el firmware lento seria tapar el sintoma: seguiria funcionando al borde.

- **Sigue sin pintar:** los niveles quedan descartados como causa **unica**.
  Siga con 2b. Si a `RETARDO_US 20` no cambia nada, pruebe **100** antes de
  descartar del todo.

> **Por que no `setBusClock()`:** no sirve aqui. `setBusClock()` guarda un valor
> que solo leen los transportes por **hardware** (SPI/I2C del periferico). Este
> display usa `u8x8_byte_arduino_4wire_sw_spi`, que mueve los pines con
> `digitalWrite()` y **nunca consulta ese valor**. Llamarlo compila, no da error
> y no cambia nada — es una trampa comoda para perder una tarde concluyendo que
> "con reloj lento tampoco pinta". Lo que si funciona, y es lo que hace esta
> variante, es **sustituir el callback de GPIO y retardo** (`gpio_and_delay_cb`)
> para insertar un retardo real tras cada cambio de pin.

#### 2b. `VARIANTE_PSB PSB_SUELTO` — el strap del modulo

Muchos modulos traen PSB puenteado en la placa. Si el modulo lo ata a VCC y el
STM32 lo fuerza a LOW, los dos se pelean y la linea puede quedarse en tierra de
nadie, con el controlador en **modo paralelo** esperando datos que nunca llegan
por el puerto serie. Esa averia da exactamente una pantalla encendida y muda.

Esta variante deja el pin del STM32 como **entrada** (alta impedancia): no pelea
con nadie y manda el strap.

- **CON EL MULTIMETRO, en esta variante, mida PSB (PB6) contra masa.**
  - Marca ~5 V o ~3,3 V → **hay strap a VCC**: el modo serie nunca se
    selecciono. Hay que quitar el puente del modulo (o cortar la pista).
  - Marca ~0 V → el strap esta a masa o no hay strap; PSB no es la causa.
- **Empieza a pintar** → confirmado, era el strap y el micro estorbaba.

#### 2c. `VARIANTE_SIN_RST 1` — la linea de reset

Si RST esta en corto a masa — pista danada, soldadura puenteada, cable
pellizcado — el ST7920 queda **retenido en reset para siempre**: alimentado, con
la retroiluminacion encendida, y sin procesar ni un comando. Sintoma identico al
de N-22.

Con esta variante se pasa `U8X8_PIN_NONE` y U8g2 no toca RST en absoluto; el
modulo arranca con su propio circuito de reset interno.

- **Empieza a pintar** → el problema es la linea **PB7**, no la pantalla.
  Confirmelo con el multimetro: continuidad de PB7 contra masa.
- **El micro se colgaba dentro de `begin()` y con esta variante ya no** → misma
  conclusion.

#### 2d. `VARIANTE_PSB PSB_ALTO` — control negativo

Fuerza modo **paralelo**, que es absurdo a proposito. No espere que funcione.

- **Se ve ALGO distinto** — basura, lineas, cualquier cosa — → el chip **esta
  vivo y escuchando**. El fallo esta en el modo o en el dato, no en la
  alimentacion ni en el panel. Es una noticia buena.
- **Se ve exactamente lo mismo que siempre (nada)** → el controlador no responde
  a ningun estimulo. Sospeche de alimentacion del modulo o del propio panel.

---

## 4. Resumen en una tabla

| Lo que ve | Conclusion |
|---|---|
| Testigo quieto, nunca parpadea | El micro no corre. No es la pantalla. |
| Rafaga de 10 repitiendose | Bucle de reinicio. Mire alimentacion. |
| Firma completa, luego quieto | Se cuelga en `begin()`. Mire RST (2c). |
| Late en bucle, pantalla muda | Tarjeta sana. Es la pantalla (2a → 2b → 2c). |
| Latido se para en una fase | Corto en SCLK/SID/CS. |
| Rellenos si, texto no | **Contraste.** Ajuste el potenciometro. |
| Damero corrido o repetido | Direccionamiento, no niveles. |
| Pinta solo con `VARIANTE_LENTA` | **Umbrales.** Alimente el modulo a 3,3 V. |
| Pinta solo con `PSB_SUELTO` | Strap de PSB en el modulo. |
| Pinta solo con `SIN_RST` | Linea PB7 en corto. |

---

## 5. Lo que este firmware NO puede decidir

Que las dos pantallas fisicas funcionen probadas en el Maestro **no descarta los
niveles**: descarta los paneles. Si la hipotesis de umbrales es correcta, lo que
difiere entre las dos puntas no es la pantalla ni el firmware, sino el ejemplar
concreto del STM32 y su tension de salida real bajo carga.

**Mida, con el osciloscopio si lo tiene a mano, la tension de SCLK (PB3) en alto
durante la fase 1, con el modulo conectado.** Si no llega a 3,5 V — y no va a
llegar, porque el STM32 no da mas de 3,3 V — el diseno esta fuera de
especificacion en las dos puntas, y el Maestro esta funcionando de prestado.
