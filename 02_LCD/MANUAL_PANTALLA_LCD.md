# 🖥️ Especificación y Mapeo Gráfico de Pantalla LCD ST7920 (V8.7)

**Ubicación:** `02_LCD/MANUAL_PANTALLA_LCD.md`
**Módulo:** Pantalla Gráfica Autónoma ST7920 (128×64 píxeles)
**Librería:** U8g2 (`U8G2_ST7920_128X64_F_SW_SPI`)
**Fecha de revisión:** 1 de agosto de 2026

> ## ⚠️ ESTE DOCUMENTO SE REESCRIBIÓ EL 01/08/2026
>
> La versión anterior describía un **menú plano de 4 opciones** y afirmaba que el Esclavo
> no tenía interfaz. **Las dos cosas son falsas hoy.** Lo que cambió:
>
> | Antes (V8.1) | Hoy (V8.7) |
> |---|---|
> | Menú plano de 4 opciones | **Dos niveles**: 4 opciones en la raíz + 3 en `CONFIGURACION` |
> | Pantallas: bienvenida, menú, config, 3 modos, alcance | **+ AJUSTAR HORA, MODO DEGRADADO, RECHAZADO, ÁMBAR con motivo** |
> | El Esclavo no tenía pantalla | **El Esclavo tiene pantalla, menú y sus propias vistas** |
> | Arnés de validación: 30/30 | **83/83** |
> | Botonera: solo botones físicos | **Mando de 4 relés en paralelo**, operado a ciegas desde el suelo |

---

## 1. Introducción y referencias cruzadas

Este documento especifica el mapeo visual, el cableado y el comportamiento de la interfaz
gráfica sobre la **ST7920 (128×64)** conectada a las controladoras STM32 ("BluePill"),
**tanto en el Maestro como en el Esclavo**.

* **`ARQUITECTURA.map`** — módulo de UI de Capa 2 (Maestro) y Capa 1 (Esclavo).
* **`MANUAL_USUARIO.md`** — estados legales que deben verse: `ROJO`, `VERDE`, `AMARILLO`, `FALLO COM`.
* **`OPTIMIZACIONES.md` / `roadmap.md`** — SFTY-14 (telemetría), SFTY-15 (diagnóstico de línea),
  SFTY-18 (reloj), SFTY-21 (Modo Degradado y mando de relés), SFTY-23 (sincronización por radio).
* **`04_Manuales/MANUAL_MANDO_4_RELES.md`** — la interfaz que se opera **sin ver esta pantalla**.
  Buena parte de las decisiones de layout de aquí se explican por él.
* **`05_Funcional/8_Procedimiento_Modo_Degradado.md`** — el procedimiento de campo. Este
  documento describe **lo que se ve**; aquél, **lo que hay que hacer**.

---

## 2. Hardware y pinout (SPI de 3 hilos)

La ST7920 soporta modo paralelo y modo serie. El firmware la configura en **modo serie**
bajando `PSB` a **LOW**. **El pinout es idéntico en Maestro y Esclavo** (`include/pines.h`
de ambos proyectos), lo que permite intercambiar tarjetas en campo sin recablear.

| Pin LCD ST7920 | Pin STM32 | Función en firmware (`pines.h`) |
|---|---|---|
| **E** | `PB3` | Clock SPI (`LCD_SCLK`) |
| **RS** | `PB4` | Chip Select (`LCD_CS`) |
| **RW** | `PB5` | Datos MOSI (`LCD_SID`) |
| **PSB** | `PB6` | Selección de modo (`LCD_PSB` → LOW = serie) |
| **RST** | `PB7` | Reset (`LCD_RST`) |
| **VCC / GND** | 5 V / GND | Alimentación y tierra común |
| **BLA / BLK** | 5 V / GND | Retroiluminación |

> ### 🔌 La pantalla se inicializa la ÚLTIMA, a propósito
>
> En el Esclavo (`main.cpp`), las luces, la radio, el watchdog y el RTC arrancan **antes**
> de que se toque la LCD. La razón está escrita en el propio código: si la pantalla
> estuviera desconectada o averiada, **el equipo tiene que seguir siendo un semáforo**. La
> pantalla es para diagnosticar el cruce, no una condición para que funcione.
>
> El Maestro conserva un `delay(2000)` en la bienvenida porque lo hace **antes** de armar
> el watchdog. El Esclavo **no puede permitírselo**: allí el watchdog se arma primero, y
> dos segundos de logotipo serían medio periodo del perro guardián gastado sin leer la
> radio. Por eso su bienvenida dura **1.500 ms sostenidos por temporizador**, no por
> `delay()`.

---

## 3. Botonera física y mando de relés

Cuatro botones con pull-up, mismos pines en las dos unidades:

* **PB9 (`BOTON1`, "A" / Arriba)** — sube en el menú, incrementa valores.
* **PB13 (`BOTON2`, "B" / Abajo)** — baja en el menú, decrementa valores.
* **PB14 (`BOTON3`, OK)** — selecciona, confirma, avanza de dígito.
* **PB15 (`BOTON4`, Cancelar)** — aborta y regresa al menú.

> ### ⚠️ Los mismos contactos los acciona un mando de relés desde el suelo
>
> El mando de 4 relés está cableado **en paralelo** con estos cuatro botones: no hay
> entradas dedicadas. Electricamente **son el mismo contacto** (`botones.cpp`). Eso tiene
> tres consecuencias que condicionan el diseño de todas las pantallas:
>
> 1. **La pulsación larga no existe.** El relé da un pulso por flanco: sostenerlo 10 s da
>    un solo pulso. Cualquier pantalla que exigiera "mantener pulsado" sería inservible.
>    Por eso `AJUSTAR HORA` se edita **dígito a dígito** y no como valor completo.
> 2. **Puede llegar una pulsación cuando el equipo no está donde el operario cree.** Por
>    eso `ESTADO` es la primera opción del menú del Esclavo y por eso `AJUSTAR HORA` y
>    `MODO DEGRADADO` están un nivel por debajo en el Maestro.
> 3. **Las secuencias del mando se ignoran con el menú abierto** y en `AJUSTAR HORA`.
>    Ver `04_Manuales/MANUAL_MANDO_4_RELES.md`.

---

## 4. El menú de dos niveles del Maestro

### 4.1 Estructura actual

```text
   MENU PRINCIPAL  (4 opciones)          SUBMENU "CONFIGURACION"  (3 opciones)
   +----------------------------+        +----------------------------+
   |       MODO SEMAFORO        |        |       CONFIGURACION        |
   |----------------------------|        |----------------------------|
   | > MANUAL                   |        | > PRUEBA ALCANCE           |
   |   AUTOMATICO               |   ->   |   AJUSTAR HORA             |
   |   INTELIGENTE              |        |   MODO DEGRADADO           |
   |   CONFIGURACION       -----+        +----------------------------+
   +----------------------------+
```

Definido en `Maestro/src/menu.cpp`. `CONFIGURACION` **no arranca ningún modo**: baja de
nivel y el equipo se queda en el mismo estado seguro del menú principal (Rojo Fijo con
enlace, Ámbar Intermitente sin él). El Botón 4 en el submenú vuelve a la raíz **con el
cursor sobre `CONFIGURACION`**, para que se vea de dónde se viene.

### 4.2 Por qué se partió en dos niveles — el caso concreto

La historia importa porque explica la regla, y la regla es lo que impide repetir el error.

| Versión | Opciones | Interlineado | Última opción cae en |
|---|---|---|---|
| V7.0 | 3 | paso 14 px | `y = 56` — holgado |
| V8.1 | 4 (`+PRUEBA ALCANCE`) | paso 11 px | `y = 61` — ajustado pero válido |
| V8.6 | 5 (`+AJUSTAR HORA`) | paso 9 px | `y = 60` — obligó a comprimir |
| **Propuesta descartada** | **6 (`+MODO DEGRADADO`)** | paso 9 px | **`y = 69` — fuera de los 64 px** |

> ### 🚨 El peligro NO era que la sexta opción no se dibujara
>
> `lcd_dibujarMenu()` tiene una salvaguarda (`if (y > 63) break;`) que impide dibujar
> fuera de pantalla, así que la sexta opción sencillamente no habría aparecido.
>
> **El peligro era que el cursor SÍ podía llegar hasta ella.** `menu_loop()` recorre las
> opciones con aritmética modular sobre la cantidad declarada, no sobre las dibujadas: el
> operario habría bajado una vez más de lo que ve, quedándose sobre una opción invisible
> —y habría pulsado OK sobre ella—. Esa opción invisible era **`MODO DEGRADADO`**, es
> decir, el modo que da verde por reloj sin confirmación del otro extremo.
>
> Una pantalla que no muestra una opción es un defecto cosmético. Una pantalla que permite
> **seleccionar a ciegas** la opción más peligrosa del equipo es un defecto de seguridad.

La causa de fondo no era el número de opciones, sino **mezclar dos cosas distintas en una
lista plana**:

| Grupo | Opciones | Frecuencia de uso |
|---|---|---|
| **Modos de operación** | `MANUAL`, `AUTOMATICO`, `INTELIGENTE` | se eligen a diario |
| **Herramientas y casos especiales** | `PRUEBA ALCANCE`, `AJUSTAR HORA`, `MODO DEGRADADO` | rara vez, y exigen criterio |

Al separarlas, el menú principal vuelve a **4 opciones — exactamente el layout validado en
campo** — y el submenú se queda en 3, que usa ese mismo layout. Ninguno de los dos se
acerca al límite, y una séptima opción futura tampoco obligaría a comprimir nada.

**Y hay un beneficio de seguridad que no es accesorio:** con el mando de relés operando a
ciegas, una ráfaga accidental de pulsos ya **no puede alcanzar** `AJUSTAR HORA` ni
`MODO DEGRADADO`, porque están un nivel por debajo y para bajar hace falta el **Botón 3**,
que es justamente el que las secuencias del mando tienen prohibido usar. La estructura
refuerza el requisito en vez de dejarlo colgando de una sola comprobación en el código.

### 4.3 Interlineado adaptativo

`lcd_dibujarMenu()` ajusta el interlineado según la cantidad de opciones. Bajo la línea
separadora (`y = 16`) solo quedan **47 px útiles**:

| Cantidad de opciones | Base | Paso | Líneas base resultantes | Estado |
|---|---|---|---|---|
| **Hasta 4** | **28** | **11 px** | `y = 28, 39, 50, 61` | ✅ **Layout validado en campo y en el arnés** |
| 5 | 24 | 9 px | `y = 24, 33, 42, 51, 60` | Compacto. Solo se usa si de verdad hace falta |
| 6 | 24 | 9 px | la 6.ª caería en `y = 69` | ❌ **Imposible.** Ver §4.2 |

> **Los dos menús actuales del Maestro caen en el caso de 4** (4 opciones en la raíz, 3 en
> el submenú), así que **ambos usan el layout validado en campo**. El caso de 5 está
> implementado y probado geométricamente, pero **hoy no lo ejercita ninguna pantalla del
> producto**: si alguien añade una quinta opción, está entrando en un layout que nunca ha
> estado en un poste.

El título del submenú se centra sobre su **ancho real de texto** (`u8g2_font_7x14_tr` es de
paso fijo, 7 px por carácter, así que la cuenta es exacta). Sin ese centrado calculado, un
título largo se saldría por la derecha y **U8g2 lo recortaría en silencio**.

---

## 5. Mapeo de pantallas del Maestro (`Maestro/src/lcd.cpp`)

### 📺 5.1 Bienvenida (`lcd_dibujarBienvenida`)
2,0 s al encender. **Durante ese lapso las luces permanecen apagadas**; al terminar, el
Maestro fija Rojo.
```text
+--------------------------------+
| Semaforo                       |
| Controlador Pro                |
|          IT Vial SAS           |
+--------------------------------+
```

### 📺 5.2 Menú (`lcd_dibujarMenu`)
Ver §4. La firma admite un **título opcional**; sin título pinta `MODO SEMAFORO`. Se dejó
opcional para no tocar las llamadas ya existentes.

### 📺 5.3 `AJUSTAR HORA` (`lcd_dibujarAjusteHora`) — **nueva**
Única vía para poner el reloj del Maestro (SFTY-18), y **requisito previo del Modo
Degradado**. Se edita **dígito a dígito**, con el dígito activo **subrayado**.
```text
+--------------------------------+
|       AJUSTAR HORA             |
|--------------------------------|
|            14:32               |
|            ^^                  |   <- subrayado bajo el digito activo
| RELOJ SIN PONER EN HORA        |   <- solo si reloj_enHora() == false
| 1=+ 2=- 3=sig 4=salir          |
+--------------------------------+
```

| Botón | Efecto |
|---|---|
| **1** | Incrementa el dígito activo (con vuelta al principio) |
| **2** | Decrementa el dígito activo |
| **3** | Avanza al siguiente dígito; **tras el último, CONFIRMA** |
| **4** | Sale **sin guardar** |

**Por qué dígito a dígito y no valor completo:** con un solo botón de subir, poner los
minutos como valor completo costaría **hasta 59 pulsaciones**. Por dígito son 9 como
máximo, y normalmente dos o tres. Y sobre todo: **funciona igual con el mando de relés**,
que solo entrega pulsos y no admite repetición por mantener pulsado.

**Rangos por dígito**, para que no exista una hora imposible: decena de hora `0..2`, unidad
de hora `0..9` (acotada a `0..3` si la decena es 2), decena de minuto `0..5`, unidad `0..9`.
Al pasar la decena a 2 con una unidad mayor que 3 (p. ej. 19 → 29), la unidad **se recorta**.

**Se trabaja sobre una copia**: el RTC solo se escribe al confirmar, así que entrar por
error y salir con el Botón 4 **no altera la hora**. Al confirmar, los segundos se ponen a
**0** —lo más parecido a poner el reloj contra una referencia externa— y **en el mismo
gesto se sincroniza el Esclavo por radio** (SFTY-23).

> ⚠️ **El aviso `RELOJ SIN PONER EN HORA` es deliberadamente feo.** Mientras el reloj no
> esté validado, ninguna función que dependa de la hora debe activarse. Un reloj sin poner
> que se cree válido es **peor que no tener reloj**: habilitaría el Modo Degradado sobre
> una hora inventada.

Esta pantalla **no arranca ciclos**: mantiene el mismo estado seguro que el menú, y
sostiene el latido de 3 s para que el Esclavo no interprete orfandad mientras el operario
edita.

### 📺 5.4 Configuración de valores (`lcd_dibujarConfigValor`)
Asistente de tiempos de Modo Automático y de Rojo estático en Manual.
```text
+--------------------------------+
|         CONFIGURACION          |
|--------------------------------|
| Tiem. Despeje All-Red          |
|            300 seg             |
| 1/2=+/- 3=OK 4=Menu            |
+--------------------------------+
```

> ⚠️ **Colisión de nombres, sin resolver.** Esta pantalla lleva el encabezado
> `CONFIGURACION`, **el mismo texto que la cuarta opción del menú principal y el título del
> submenú**, que son cosas distintas. No es un defecto funcional, pero al describirlo en un
> manual de operario hay que decir cuál es cuál: la del menú es *entrar al submenú de
> herramientas*; ésta es *fijar un tiempo del ciclo*.

### 📺 5.5 Modos en vivo (`lcd_dibujarAutomatico`, `lcd_dibujarInteligente`, `lcd_dibujarManual`)
```text
+--------------------------------+   +--------------------------------+
|        MODO: AUTOMATICO        |   |          MODO: MANUAL          |
|--------------------------------|   |--------------------------------|
|             VERDE              |   |              ROJO              |
|        R:02m V:03m             |   | 1/2=Cambiar 3=Rojo             |
| RF:100% 340ms          4=Menu  |   | RF:100% 340ms          4=Menu  |
+--------------------------------+   +--------------------------------+

+--------------------------------+
|      MODO: INTELIGENTE AI      |
|--------------------------------|
|             VERDE              |
|       IA: OK (Autos: 3)        |     (o "IA: Standby (Fallback)")
| RF:100% 340ms          4=Menu  |
+--------------------------------+
```

### 📺 5.6 `PRUEBA ALCANCE` (`lcd_dibujarAlcance`)
Herramienta de campo para medir hasta dónde hay cobertura real, en vez de estimarla a ojo.
```text
+--------------------------------+   +--------------------------------+
|       PRUEBA ALCANCE           |   |       PRUEBA ALCANCE           |
|--------------------------------|   |--------------------------------|
| 100%                    340ms  |   |         SIN ENLACE             |
| ##########                     |   | Fallos seguidos: 4             |
| Fallos: 0                      |   |                                |
| RX 36  9 tr            4=Menu  |   | RX 4k - BASURA         4=Menu  |
+--------------------------------+   +--------------------------------+
```
Durante los primeros 3 s (sin muestras) muestra `Midiendo enlace... / espere 3 seg`.
La barra tiene **10 segmentos** proporcionales a la calidad.

**No inicia ciclos**: es seguro dejarla puesta. Estado de las luces idéntico al menú
(🔴 Rojo Fijo con enlace, 🟡 Ámbar Intermitente sin él).

### 📺 5.7 `MODO DEGRADADO` (`lcd_dibujarDegradado`) — **nueva**
Es la **única pantalla del equipo que acompaña a un VERDE dado sin confirmación del otro
extremo**. Por eso tiene que decir en todo momento las cuatro cosas que le permiten a un
técnico decidir si sigue confiando en ella.
```text
+--------------------------------+
|  MODO DEGRADADO                |
|--------------------------------|
| VERDE                     29s  |   <- (1) fase actual   (2) cuenta atras
| Paso por el maestro            |   <- que significa esa fase
| Sin sync: 0h03m                |   <- (3) cuanto lleva sin sincronizar
| AVISO: LIMITE 48h      4=Menu  |   <- (4) proximidad al limite duro
+--------------------------------+
```

| Dato | Por qué está ahí |
|---|---|
| **Fase** (`VERDE` / `ROJO`) | Es lo que se compara de un vistazo con la luz de la calle |
| **Cuenta atrás** | Dice si el ciclo corre o se quedó congelado |
| **Horas sin sincronizar** | **El dato que de verdad importa.** Sin radio, es la única medida de cuánto se pueden haber separado los dos relojes |
| **Aviso de límite** | Aparece con **4 h de margen** sobre el límite duro de 48 h |

Textos de detalle según la fase: `Entrando: todo rojo`, `Paso por el maestro`,
`Paso por el esclavo`, `Despeje total`.

> La cuenta atrás sale de `ciclo_degradado_restante()`, **no de un contador propio de la
> pantalla**. Un segundero paralelo acabaría discrepando del cálculo que de verdad manda
> sobre las luces, y entonces la pantalla mentiría justo en el modo donde nadie puede
> contrastarla contra la otra punta.

El `4=Menu` de la derecha **se respeta siempre**, incluso con el aviso de límite puesto:
salir tiene que poder hacerse aunque la pantalla esté dando una alarma. El arnés lo
comprueba (§7).

### 📺 5.8 Rechazo de entrada (`lcd_dibujarDegradadoRechazo`) — **nueva**
No dice "no se puede". **Dice cuál condición falta.**
```text
+--------------------------------+
| DEGRADADO                      |
|--------------------------------|
| RECHAZADO                      |
| Falta: reloj sin               |
| poner en hora                  |
+--------------------------------+
```

| Motivo (`modo_degradado.cpp`) | Línea 1 | Línea 2 |
|---|---|---|
| `MDG_FALTA_HORA` | `Falta: reloj sin` | `poner en hora` |
| `MDG_NUNCA_SYNC` | `Falta: nunca hubo` | `sincronizacion RF` |
| `MDG_SYNC_VIEJA` | `Falta: la ultima` | `sync es muy vieja` |
| `MDG_SIN_DESFASE` | `Falta: sin medida` | `de desfase valida` |
| `MDG_DESFASE_ALTO` | `Desfase fuera de` | `tolerancia (+-3s)` |

> **Por qué el motivo y no un mensaje genérico:** el técnico está a 5 m de altura con el
> equipo abierto. Necesita saber si tiene que **poner el reloj**, **esperar a que
> sincronice** o **revisar el radio** — son tres viajes distintos. Un "no se puede" lo
> obliga a bajar, adivinar y volver a subir.

Partir el motivo en dos líneas no es estética: es lo que permite que quepan los **18
caracteres** del motivo más largo sin recortarlo, **y un motivo recortado no sirve para
arreglar nada**.

### 📺 5.9 Ámbar con motivo (`lcd_dibujarDegradadoAmbar`) — **nueva**
```text
+--------------------------------+
| AMBAR INTERM.                  |
|--------------------------------|
| Ambar pedido desde             |
| el mando (B.B.B)               |
|                        4=Menu  |
+--------------------------------+
```

| Situación | Línea 1 | Línea 2 |
|---|---|---|
| Límite de 48 h agotado | `Limite 48h sin sync` | `Revise el radio` |
| El reloj dejó de ser fiable | `Reloj no fiable` | `Degradado detenido` |
| Pedido desde el mando | `Ambar pedido desde` | `el mando (B.B.B)` |
| Saliendo del Degradado | `Saliendo: todo rojo` | `Vea las dos puntas` |

> **Por qué el ámbar dice su motivo:** un ámbar mudo obliga a subir al poste a preguntarle
> al equipo qué le pasa. Desde el suelo, un ámbar por límite de 48 h y un ámbar pedido a
> mano se ven **exactamente igual**, y llevan a acciones opuestas.

**Límite duro de la fila:** con `u8g2_font_6x10_tr` dibujada desde `x = 2` caben **20
caracteres** (2 + 20×6 = 122 px). El arnés prueba una línea de 20 para dejar clavado dónde
está el borde.

### 📺 5.10 Pantallas presentes en `lcd.cpp` pero **sin ninguna llamada**

Documentado por honestidad, no como funcionalidad:

| Función | Estado |
|---|---|
| `lcd_dibujarNoDisponible()` | **Código muerto.** Nadie la llama fuera de `lcd.cpp` |
| `lcd_dibujarTextoRecibido()` | **Código muerto.** Depuración de RS485 |
| `lcd_dibujarConfigMinutos()` | **Código muerto.** La reemplazó `lcd_dibujarConfigValor()` |

> No se puede documentar como "pantalla del equipo" algo que el operario nunca verá.
> Ocupan flash en un Maestro que va al **80,2 %** (§8) — su retiro es una decisión de
> firmware, y este documento solo la señala.

---

## 6. 🆕 La pantalla del Esclavo (`Esclavo/src/lcd.cpp`, `menu.cpp`)

> **Esto no estaba documentado en ningún sitio.** Hasta el 01/08/2026 el Esclavo no tenía
> interfaz en firmware (issue **N-16**). Hoy tiene `lcd.cpp`, `botones.cpp`, `menu.cpp` y
> `modo_degradado.cpp` propios.

### 6.1 Qué NO tiene, y por qué

| El Esclavo no tiene | Razón |
|---|---|
| **Pantallas de modo de operación** | El Esclavo **no decide el ciclo**. Una pantalla que ofrezca decidirlo acaba con las dos puntas peleándose por quién manda |
| **`AJUSTAR HORA`** | La hora **llega por radio** (SFTY-23). Ajustarla a mano allí reintroduce exactamente el desfase de hasta 59 s que ese mecanismo existe para eliminar |
| **Línea `RF:xx%`** | El Esclavo **no es quien interroga**, así que no tiene telemetría de calidad. Lo único honesto que puede mostrar son sus contadores de línea en bruto |
| **Modos propios que ofrecer desde el menú** | Solo puede entrar o salir del Modo Degradado; el resto lo decide el Maestro |

> **El mando de 4 relés sí llegó al Esclavo** (`Esclavo/src/mando.cpp`, 01/08/2026): mismas
> secuencias y mismos destellos que el Maestro. **Ninguna de las dos puntas tiene todavía el
> receptor físico instalado.** Ver `04_Manuales/MANUAL_MANDO_4_RELES.md`.

### 6.2 Bienvenida
```text
+--------------------------------+
| Semaforo                       |
| ESCLAVO                        |     1.500 ms, sostenidos por temporizador
|          IT Vial SAS           |     (no con delay: el watchdog ya esta armado)
+--------------------------------+
```

### 6.3 Menú del Esclavo — **2 opciones**
```text
+--------------------------------+
|      MENU ESCLAVO              |
|--------------------------------|
| > ESTADO                       |
|   MODO DEGRADADO               |
|                                |
| MODO DEGRADADO ACTIVO          |   <- pie, solo si el modo gobierna la luz
+--------------------------------+
```

> ### ⚠️ El orden de las dos opciones NO es casual
>
> `ESTADO` va primero **porque el cursor arranca en la primera opción y esa opción solo
> lee**. Los pulsadores están en paralelo con relés que se accionan sin ver la pantalla:
> si la primera opción fuera `MODO DEGRADADO`, un pulso suelto de ACEPTAR llevaría directo
> a la pantalla que puede cambiar la operación del cruce. Así, **lo peor que hace un pulso
> perdido es mostrar un diagnóstico.**

El **pie `MODO DEGRADADO ACTIVO`** aparece en el propio menú, sin entrar a ninguna
pantalla. Es el dato que cambia el significado de todo lo demás: un técnico que no sepa que
el cruce va por reloj **puede creer que el radio funciona**.

Usa el **mismo interlineado adaptativo** del Maestro (§4.3). Con 2 opciones cae en `y = 28`
y `y = 39`, muy dentro de pantalla; la regla completa se dejó escrita para que quien añada
una tercera no tenga que volver a descubrir por qué existía.

### 6.4 `ESTADO` (`lcd_dibujarEstado`)
```text
+--------------------------------+
| ESTADO                         |
|--------------------------------|
|         14:32:07               |   <- reloj propio, CON SEGUNDOS
| Sync hace: 12h30m              |
| Luz: ROJO                      |
| RX 36  9 tr            4=Menu  |
+--------------------------------+
```

> **Los segundos están a propósito.** Es la pantalla que se contrasta contra el reloj del
> Maestro, y `HH:MM` no permite detectar un desfase de decenas de segundos — que es
> exactamente el defecto que SFTY-23 vino a corregir. Se repinta **una vez por segundo**:
> ni más (cada volcado SPI cuesta decenas de milisegundos que hacen falta para la radio),
> ni menos (un reloj que avanza a saltos no se puede leer contra otro).

**Si el reloj no está en hora se pintan guiones `--:--:--`, no ceros.** Un `00:00:00`
parece una hora, y alguien podría darla por buena.

**Antigüedad de la última sincronización**, acotada a 6 caracteres: `NUNCA`, `59s`, `45m`,
`12h30m`, `>48h`.

> ### 🕐 Por qué `>48h` se recibe como bandera y no se calcula aquí
>
> `millis()` da la vuelta a los **49,7 días** y una resta sin signo volvería a dar un
> número pequeño: la pantalla diría **`12m` tras dos meses sin sincronizar**. Ese número
> sería mentira justo en el escenario más peligroso. Por eso el enclavamiento lo decide
> quien lleva la cuenta y esta función **solo pinta**.

### 6.5 `MODO DEGRADADO` del Esclavo (`lcd_dibujarDegradado`)
```text
+--------------------------------+
| MODO DEGRADADO                 |
|--------------------------------|
| ACTIVO                         |
| VERDE 29s                      |
| [ Sin sync: 12h30m           ] |   <- recuadro si esta cerca del limite
| 3=Salir   4=Menu               |
+--------------------------------+
```

Con el modo detenido, la línea de detalle dice `Pulse 3 para entrar`, o bien el motivo del
impedimento con el encabezado `NO SE PUEDE. FALTA:` (o `RENDIDO 48h. FALTA:` si ya se
rindió).

**El contador contra el límite de 48 h va SIEMPRE**, esté el modo activo o no: es el dato
que decide si se puede entrar, y esconderlo hasta que el modo arranque **obligaría a
intentarlo para averiguarlo**.

> **El aviso de proximidad al límite se recuadra en lugar de escribirse.** La pantalla se
> lee de madrugada, con lluvia y a un metro. Una palabra más en la misma tipografía no se
> ve; un recuadro sí.

### 6.6 Confirmación de entrada
```text
+--------------------------------+
| MODO DEGRADADO                 |
|--------------------------------|
| CONFIRMAR ENTRADA?             |
| Verifique el Maestro           |
| Sin sync: 3m                   |
| 3=SI entrar   4=NO             |
+--------------------------------+
```

**Entrar exige DOS pulsaciones**; **salir es inmediato y sin confirmar**. La asimetría es
deliberada: salir lleva al equipo hacia el estado seguro y no necesita protección, mientras
que entrar habilita verdes sin confirmación del otro extremo. **Retrasar el camino hacia lo
seguro con una pregunta sería proteger al equipo del operario en el sentido equivocado.**

### 6.7 Rechazo (`lcd_dibujarRechazoDegradado`)
```text
+--------------------------------+
| RECHAZADO                      |
|--------------------------------|
| SIN HORA VALIDA                |
| El Degradado NO entro.         |
| Todo sigue igual. 4=Menu       |
+--------------------------------+
```

Motivos: `SIN HORA VALIDA`, `FALTA CONFIG CICLO`, `CICLO EN CERO`, `NUNCA SINCRONIZADO`,
`SYNC CADUCADA >48h`.

El cartel **caduca solo a los 6 s** además de cerrarse con cualquier pulsación: no puede
quedarse puesto tapando el estado real del equipo si nadie vuelve.

> ### ⚠️ Los motivos de rechazo NO usan los mismos textos en las dos unidades
>
> El Maestro los parte en dos líneas (`Falta: reloj sin` / `poner en hora`); el Esclavo usa
> una sola línea en mayúsculas (`SIN HORA VALIDA`). **Tampoco son el mismo conjunto**: el
> Esclavo tiene `FALTA CONFIG CICLO` y `CICLO EN CERO`, que el Maestro no tiene; el Maestro
> tiene `Desfase fuera de tolerancia`, que el Esclavo no tiene.
>
> Es coherente con el diseño —cada punta comprueba lo que le corresponde— pero **quien
> escriba el instructivo de campo no puede usar una sola tabla para las dos pantallas.**

### 6.8 Diagnóstico de línea (SFTY-15), común a las dos unidades

| Se muestra | Significado | Dónde mirar |
|---|---|---|
| `RX 0 - nada llega` | No entra ni un byte | Cobertura, canal, antena |
| `RX 4k - BASURA` | Entran bytes pero **ninguna trama válida** | Cableado, línea RS485 flotando, radio atascada |
| `RX 36  9 tr` | Bytes y tramas válidas (`tr` = tramas válidas) | Enlace correcto |

Los contadores se abrevian con `k` y `M` (`abreviarCuenta()`) para no desbordar la fila.
**No es cosmético:** un contador de 6 cifras hacía que la línea invadiera el `4=Menu` de la
derecha, y el arnés lo detectó antes de flashear.

En el Maestro, los contadores **se ponen a cero al entrar** a `PRUEBA ALCANCE`, de modo que
lo mostrado corresponde a esa medición y no al acumulado desde el encendido.

---

## 7. 📊 Línea de calidad de enlace (Maestro, pantallas 5.5)

| Se muestra | Significado |
|---|---|
| `RF:---` | Aún no hay muestras (primeros 3 s tras arrancar) |
| `RF:100% 340ms` | 100 % de latidos respondidos; tiempo de respuesta medio 340 ms |
| `RF:SIN ENLACE` | Ninguno de los últimos 10 latidos obtuvo respuesta |

**De dónde sale el dato:** el Maestro ya emite un latido cada 3 s (`PING` en operación,
`GO_RED` en Menú y en Fallo). Se mide si hubo respuesta y cuánto tardó, sobre una ventana
deslizante de los últimos 10 latidos. **No requiere soporte de la radio ni cambios de
protocolo** — es lectura de lo que ya circula.

> ℹ️ La radio E90-DTU **no entrega RSSI** en modo transparente, así que no es posible
> mostrar potencia en dBm. Activar la salida de RSSI en el módulo añadiría un byte a cada
> paquete recibido y **rompería el parser de tramas de 4 bytes**; no se ha hecho.

---

## 8. Arnés de validación de pantalla — **83/83**

`01_Firmware/Validacion_LCD/` · ejecutar con `compilar.ps1` · requiere GCC (MinGW-w64).

### 8.1 Qué es y por qué existe

**U8g2 recorta en silencio lo que cae fuera de pantalla.** Un texto mal colocado no da
error: sencillamente no aparece, y nadie lo nota hasta tener la tarjeta en la mano, en el
poste. El arnés compila **el mismo `lcd.cpp` y el mismo `menu.cpp` que van al firmware**
contra un framebuffer en memoria, de modo que **la validación no puede desviarse de lo que
se compila**.

### 8.2 Qué comprueba

| Bloque | Qué verifica |
|---|---|
| **Calibración del framebuffer** | Dibuja `drawBox(0,40,10,8)` y comprueba que se lee donde toca. Si falla, **aborta**: cualquier medida posterior sería basura |
| **Referencia de campo** | Reconoce como correcto el menú de 3 opciones **que ya funcionaba en terreno**. Si no lo reconociera, **aborta** |
| **Menú principal (4)** | Cuenta las bandas de texto dibujadas; cursor visible sobre la 4.ª opción |
| **Navegación real** | Ejecuta el `menu.cpp` **real** con botonera simulada: recorre las 4 opciones y da la vuelta; comprueba que **el cursor está SIEMPRE sobre una opción dibujada y dentro de pantalla** |
| **Submenú** | Que `CONFIGURACION` **no arranca ningún modo**; que se ven 3 opciones; que el título centrado cabe; que el Botón 4 vuelve a la raíz dejando el cursor sobre `CONFIGURACION` |
| **Indexado de modos** | Que cada posición de cada nivel selecciona el modo correcto, **incluido `MODO_DEGRADADO`** |
| **`PRUEBA ALCANCE`** | Barra proporcional al 100 % / 40 % / sin enlace / midiendo, y un recorrido completo de 8 pasos "alejándose" |
| **SFTY-15** | Los tres diagnósticos de línea, y que con **999.999 bytes / 249.999 tramas** la fila **no invade el `4=Menu`** |
| **`AJUSTAR HORA`** | Que el subrayado cae **bajo el dígito activo y solo bajo ése**, en los 4 dígitos; extremos `00:00` y `23:59`; que el aviso de reloj sin poner **no se pega** a la fila de ayuda |
| **`MODO DEGRADADO`** | 5 casos (incluida cuenta atrás de 3 cifras y 45h59m sin sync): nada se recorta por el borde derecho, las 4 líneas no se solapan, el aviso de límite **no invade el `4=Menu`** |
| **Rechazo y ámbar** | Los 5 motivos de rechazo y los 4 textos de ámbar caben enteros; una línea de **20 caracteres** —el máximo— todavía cabe |
| **Márgenes** | En todas las pantallas: que el contenido no toque ni se pegue al borde inferior |

### 8.3 La regla de validación adoptada

> **Todo instrumento de prueba debe reconocer primero como correcto lo que ya funciona en
> campo. Si no lo hace, el equivocado es el instrumento.**
>
> Por eso el arnés arranca contrastando el menú de 3 opciones validado en terreno y aborta
> si no lo reconoce. Aplicar esta regla evitó dos veces "corregir" código sano guiándose
> por una medición rota.

### 8.4 ⚠️ Lo que el arnés NO cubre

| Limitación | Consecuencia |
|---|---|
| **No valida ninguna pantalla del Esclavo.** `compilar.ps1` solo enlaza `Maestro/src/lcd.cpp` y `Maestro/src/menu.cpp` | Las 5 vistas del Esclavo (§6) **nunca han pasado por comprobación geométrica**. El gancho `LCD_VALIDACION_NATIVA` ya está puesto en `Esclavo/src/lcd.cpp`: añadirlo es cuestión de una línea en `compilar.ps1` |
| No ejercita la lógica del coordinador | La telemetría (SFTY-14) y los contadores (SFTY-15) se **inyectan a mano**. El arnés valida el **dibujo**, no el dato |
| No hay prueba de banco de las pantallas nuevas | `AJUSTAR HORA`, `MODO DEGRADADO`, rechazo y ámbar **no se han visto sobre una ST7920 real**. Contraste de la pantalla, ángulo de visión y legibilidad bajo sol **no están verificados** |
| El layout de **5 opciones** no lo usa ninguna pantalla real | Está probado geométricamente en el código pero **nunca ha estado en un poste** |

---

## 9. Consumo de recursos

| Recurso | V7.6 | V8.1 | **V8.7 (01/08/2026)** |
|---|---|---|---|
| Buffer de video en RAM | 1.024 B | 1.024 B | **1.024 B** (no cambia: `_F_` es full buffer) |
| Flash del **Maestro** | 36,3 KB (55,5 %) | 42,1 KB (64,2 %) | **52.540 B — 80,2 %** |
| Flash del **Esclavo** | — *(sin interfaz)* | — *(sin interfaz)* | **59,7 %** |

> ### ⚠️ El margen de flash del Maestro es la restricción real del proyecto (N-21)
>
> Al **80,2 %** de 65.536 B, lo pendiente —`respaldo.cpp` conectado, SFTY-22 y el modo
> nocturno— lo dejaría sobre el **85 %**: ajustado pero viable. **Una función grande más ya
> no cabría**, y eso incluye cualquier pantalla nueva.
>
> Hay una salida conocida y tiene letra pequeña: el `STM32F103C8` está especificado con
> 64 KB pero el silicio **suele** traer 128 KB físicos. Es práctica común **pero no
> documentada por ST**. En un equipo de seguridad vial exige verificarlo **chip a chip**,
> porque si un lote viene con 64 KB reales **el firmware se corrompe en silencio** al pasar
> del límite. **No tocar todavía.**
>
> Las tres pantallas muertas de §5.10 son la primera reserva de flash que se puede liberar
> sin perder ninguna función.

*Cifras tomadas de `roadmap.md` y `OPTIMIZACIONES.md` (compilación del 01/08/2026). No se
recompiló para redactar este documento.*

---

## 10. Preguntas abiertas sobre esta pantalla

Se dejan escritas porque una limitación documentada vale más que una suposición cómoda.

1. **¿Se validará el Esclavo con el arnés?** El gancho está puesto y cuesta una línea. Hoy
   sus 5 vistas son las únicas del sistema sin comprobación geométrica.
2. **¿Qué versión es "la de la pantalla"?** El código rotula estos cambios como **V8.7**;
   el encabezado de `roadmap.md` sigue en **V8.5**. Este manual adopta **V8.7** por seguir
   al código, pero la numeración debería unificarse.
3. **La colisión de nombres `CONFIGURACION`** (§5.4) — ¿se renombra el encabezado del
   asistente de tiempos, o se documenta la ambigüedad en el manual de operario?
4. **Formatos de "sin sync" distintos entre puntas** — el Maestro imprime `0h03m` siempre;
   el Esclavo distingue `NUNCA`, `45m`, `12h30m` y `>48h`. El Maestro **no tiene el
   enclavamiento `>48h` en la pantalla**, así que ante un desbordamiento de `millis()`
   mostraría un número pequeño y falso, que es justo lo que el Esclavo evita a propósito.
   **Merece revisarse en firmware.**
