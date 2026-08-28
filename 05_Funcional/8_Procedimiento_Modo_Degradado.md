# 🕹️ PROCEDIMIENTO DE CAMPO — MODO DEGRADADO (SFTY-21)

**Documento para el operario de campo y el Ingeniero Funcional**
**Fecha:** 1 de Agosto de 2026 · **Aplica a:** firmware V8.7 (rama `feat/n15-reloj-pantalla-hora`)

---

> ## ⚠️ ESTE MODO NO HA PISADO HARDWARE TODAVÍA
>
> Está construido en las dos puntas y validado en simulador (20/20 funcional, 10/10 repetidor,
> 83/83 de pantalla), pero **no se ha ejercitado sobre tarjetas reales ni en obra**. Hasta que la
> prueba de banco de la Sección 9 del `3_Protocolo_Pruebas_Rigurosas.md` esté firmada, este
> procedimiento **no autoriza operación en vía abierta al tráfico**.

---

## 1. Qué es, y qué NO es

El Modo Degradado hace que las dos unidades **sigan alternando verde y rojo sin radio**, cada una
calculando su fase a partir de la hora de pared. Es un **caso especial de activación manual**, no un
comportamiento automático.

| Situación | Qué hace el equipo |
|---|---|
| Se pierde el radio | 🟡 **Ámbar intermitente en ambas puntas.** Es el comportamiento por defecto y **no cambió** |
| Un operario activa el Degradado en las dos puntas | El cruce vuelve a alternar verde/rojo, gobernado por reloj |
| Nadie lo activa | El equipo se queda en ámbar indefinidamente. **Es correcto** |

> **El equipo NUNCA entra solo.** No hay temporizador, no hay "si pierdes el radio X minutos,
> entra", no hay autorización por adelantado. Y la razón no es prudencia genérica:

```
 ÁMBAR INTERMITENTE  ->  "no estoy controlando esto, decide tú"
                         el conductor llega ALERTA, mira, negocia el paso

 VERDE POR RELOJ     ->  "pasa tranquilo, el otro lado está en rojo"
                         el conductor llega CONFIADO y no mira
```

Sin radio, el Maestro **no puede saber si el Esclavo sigue vivo**: podría estar apagado, colgado, o
haber sido movido a otra obra. Un verde equivocado es **más peligroso que un ámbar ambiguo**, porque
le quita al conductor la precaución que el ámbar le provoca. Por eso el verde solo se da cuando
**una persona verificó las dos puntas con los ojos**.

---

## 2. Requisitos previos — sin esto el firmware lo rechaza

El Degradado **no entra** si falta cualquiera de estas condiciones. No es un aviso en pantalla que se
pueda saltar: es una puerta en firmware.

| # | Condición | Cómo se cumple | Qué muestra la pantalla si falta |
|---|---|---|---|
| 1 | El Maestro tiene el reloj **puesto en hora** | `CONFIGURACION → AJUSTAR HORA` en el **Maestro** | `Falta: reloj sin poner en hora` |
| 2 | Hubo **al menos una sincronización** por radio con el Esclavo | Ocurre sola al confirmar la hora, y cada hora mientras haya enlace | `Falta: nunca hubo sincronizacion RF` |
| 3 | Esa sincronización es **reciente** — menos de **2 h** | Basta con que el radio haya estado vivo hace poco | `Falta: la ultima sync es muy vieja` |
| 4 | Hay una **medida de desfase** contra el Esclavo | La toma el Maestro por radio (`CMD_DELTA`) | `Falta: sin medida de desfase valida` |
| 5 | Ese desfase está **dentro de ±3 s** | — | `Desfase fuera de tolerancia (+-3s)` |

### Por qué la condición 3 existe y no es burocracia

Podría parecer que basta con medir el desfase y comprobar que es pequeño. **No basta**, y el motivo
es un límite real de la medición:

```
   CMD_DELTA transporta SOLO el segundo (0-59).
   La corrección circular resuelve siempre por el camino corto.

   Desfase real de 45 s  ->  se mide como -15 s
   No hay forma de distinguirlos con solo el segundo.
```

Un desfase peligroso **podría leerse como aceptable**. Lo que cierra ese agujero es la frescura: tras
una sincronización correcta el desfase arranca en milisegundos, y con la deriva de estos cristales
harían falta **más de tres días** para acumular los 30 s que provocan la confusión. Con una
sincronización de hace una hora la deriva es de **~0,36 s**: la medida no puede estar equivocada.

> **El desfase es una comprobación de cordura. La garantía es la sincronización reciente.** Invertir
> esa relación —confiar en el número y no en su frescura— reintroduce el fallo.

### ⚠️ La hora se pone UNA sola vez, y solo en el Maestro

**El Esclavo no tiene pantalla de ajuste de hora, y es deliberado.** Ajustar las dos puntas a mano
deja hasta **59 s de desfase el primer día** sin que nadie pueda verlo:

```
   Operario A confirma el Maestro   a las 14:32:10 reales -> el reloj marca 14:32:00
   Operario B confirma el Esclavo   a las 14:32:50 reales -> el reloj marca 14:32:00

   Las dos pantallas muestran 14:32.  Los relojes están a 40 s.
```

Cuarenta segundos es **más del todo-rojo entero**. La hora se cuadra en el Maestro y **viaja por
radio** al Esclavo; así, el día que el radio muera, el desfase arranca en ~0 de verdad y no por
procedimiento.

---

## 3. Procedimiento de ENTRADA

> **Se activa en LAS DOS PUNTAS, por separado, y exige verificación visual de ambas.** Una punta
> sola en Degradado es peor que ninguna: ver Sección 6.

### Paso 1 — Activar en el **MAESTRO**

Desde la pantalla del gabinete:

1. `Botón 4` hasta llegar al **Menú Principal**.
2. Bajar hasta `CONFIGURACION` y entrar con `Botón 3`.
3. Bajar hasta `MODO DEGRADADO` y entrar con `Botón 3`.
4. La pantalla dice `Pulse 3 para entrar` si se cumplen los 5 requisitos, o **el motivo concreto** si
   no. Si aparece un motivo, resuélvalo — no hay forma de forzarlo.
5. `Botón 3` → aparece `CONFIRMAR ENTRADA?` → `Botón 3` otra vez para confirmar.

> **Entrar exige dos pulsaciones; salir, una.** La asimetría es deliberada: salir lleva el equipo
> hacia el estado seguro y no necesita protección. Entrar habilita verdes sin confirmación del otro
> extremo, y eso sí.

**Desde el piso**, con el mando de 4 relés: `A · B · A · B` en menos de 18 segundos.
Confirmación: **4 destellos rojos**. Si en vez de destellos aparece un **ámbar rápido**, la secuencia
fue **rechazada** por alguno de los 5 requisitos — hay que subir a ver cuál.

### Paso 2 — Activar en el **ESCLAVO**

> ## 🪜 HOY HAY QUE SUBIR AL GABINETE
>
> **El Esclavo no tiene receptor de mando de relés** (pendiente **N-19**). La tarjeta ya trae las
> cuatro entradas (`PB9`, `PB13`, `PB14`, `PB15`); falta comprar e instalar el receptor.
>
> **Soporte Bluetooth desde el Suelo (V9.0):** Con el módulo Bluetooth USART1 y la App Móvil
> instalada en el celular del operario, la activación y sincronización del Degradado en el Esclavo
> **se realiza directamente desde el suelo**, sin necesidad de subir al gabinete con escalera.
> Además, la App incluye el **Modo Courier RTC**, que permite capturar la hora y ciclo en el Maestro,
> viajar hasta el Esclavo y aplicar la sincronización compensando automáticamente el tiempo de viaje
> con error inferior a 0.1 s.

En la App Móvil o en la pantalla del Esclavo:

1. **Vía App Móvil (Desde el suelo):** Conectarse al `📡 ESCLAVO (Poste 2)`, entrar a `Ajustes / RTC` y pulsar `[ 🚀 Inyectar en Esclavo ]` o activar Modo Degradado.
2. **Vía Pantalla LCD (Gabinete):** `Botón 4` hasta el menú ➔ `MODO DEGRADADO` ➔ `Botón 3` (`CONFIRMAR ENTRADA`).

### Paso 3 — VERIFICACIÓN VISUAL DE AMBAS PUNTAS ← **obligatoria**

Con las dos unidades ya en Degradado, **quédese a ver al menos un ciclo completo (120 s)** y
compruebe con los ojos, no en pantalla:

- [ ] Cuando el **Maestro está en verde**, el **Esclavo está en rojo**
- [ ] Entre los dos verdes hay un **todo-rojo largo** (~30 s) con **ambas puntas en rojo**
- [ ] Cuando el **Esclavo está en verde**, el **Maestro está en rojo**
- [ ] **En ningún momento hay verde simultáneo en las dos puntas**

> **Por qué mirar y no fiarse de la pantalla.** Cada unidad muestra la fase que *ella* calcula. Si
> por lo que sea las dos calculan mal —relojes desfasados, configuración distinta, una unidad que se
> reinició— **las dos pantallas dirán que todo va bien** mientras las luces cuentan otra historia.
> La pantalla informa; **las luces son la evidencia**.

Si algo no cuadra: `B · B · B` desde el piso, o `Botón 3` en la pantalla del Degradado, **en las dos
unidades**. Vuelva a ámbar y no insista.

---

## 4. El límite duro de 48 horas

**Pasadas 48 h sin resincronizar, el Degradado cae SOLO a ámbar intermitente.** No es un aviso: es un
tope. A partir de las **44 h** la pantalla muestra `AVISO: LIMITE 48h`, y a las 48 h el equipo se rinde
por su cuenta y muestra `Limite 48h sin sync — Revise el radio`.

### Por qué existe

El colchón que impide el verde simultáneo es el **todo-rojo de 30 s**. Ese colchón se come poco a
poco por la deriva de dos cristales de 32.768 kHz sin calibrar, a la intemperie: **±30 a 50 ppm**.

| Todo-rojo en Degradado | Deriva peor caso | Margen antes de solaparse | Límite adoptado |
|---|---|---|---|
| 15 s *(el de operación normal)* | ~8,6 s/día | ~1,7 días | — *insuficiente* |
| **30 s** ← el que usa este modo | ~8,6 s/día | ~3,5 días | **48 h** |
| 90 s | ~8,6 s/día | ~10 días | 5 días *(a costa de la fluidez)* |

Las 48 h dejan **factor de seguridad 2** sobre el margen teórico.

> **El estado seguro no puede depender de que alguien se acuerde.** Ése es el principio que el resto
> del sistema ya aplica —el fallback de 12 s, el piso de 5 s del despeje— y aquí aplica igual. Un
> modo degradado que dependa de que un operario vuelva a tiempo no es un modo degradado: es una
> apuesta.

> Conviene decirlo sin adornos: **querer una semana de autonomía obligaría a un todo-rojo de ~90 s**,
> que destroza la fluidez del paso. No es una limitación del diseño, es la física de dos cristales sin
> disciplinar. **La alternativa real no es alargar el plazo: es ir a arreglar el radio.**

### Cómo se reinicia la cuenta

Solo con una **sincronización nueva por radio**, que exige que el enlace vuelva. Salir y volver a
entrar al Degradado **no reinicia nada**: el límite mide el tiempo transcurrido desde la última sincronización
real usando el **contador del RTC** (N-49 T1/T2, monótono y sin saltos de fin de mes), no desde la última pulsación.

---

## 5. Procedimiento de SALIDA

> ## ⚠️ LA VERIFICACIÓN VISUAL DE AMBAS PUNTAS ES OBLIGATORIA TAMBIÉN AL SALIR
>
> No solo al entrar. **Salir de una sola punta crea exactamente el escenario que este modo existe
> para evitar** — ver Sección 6.

### Para volver a intentar Automático

Se hace cuando se cree que el radio volvió. El escenario típico es *"dejó de llover, a ver si
enlaza"*.

- **Desde el piso:** `A · A · A` en menos de 12 s → **2 destellos rojos**.
- **Desde la pantalla:** en `CONFIGURACION → MODO DEGRADADO`, `Botón 3` (`3=Salir`).

**Hágalo en las dos unidades**, y luego mire las luces:

```
   A·A·A  ->  2 destellos  ->  esperar ~15 s

     luces CICLANDO  ->  el radio volvió, ya está en automático
     luces en ÁMBAR  ->  sigue muerto; puede volverse al degradado
```

Volver a Automático **no necesita protección**, y por eso la secuencia es corta: si el radio sigue
muerto, el propio sistema se corrige a los 12 s (SFTY-6) y cae a ámbar, que es justo donde se quería
estar. **El peor caso de intentar Automático es volver al ámbar.**

### Para irse a ámbar y dejarlo así

- **Desde el piso:** `B · B · B` en menos de 12 s → **3 destellos rojos**.
- `B·B·B` funciona **desde cualquier estado y sin condiciones**. Es la regla que impide que nadie
  quede atrapado con un semáforo en un estado raro a 5 m de altura.

### Checklist de salida

- [ ] Secuencia o pulsación ejecutada en el **Maestro**
- [ ] Secuencia o pulsación ejecutada en el **Esclavo** *(hoy: subiendo al gabinete)*
- [ ] **Verificado con los ojos** que **ambas** puntas quedaron en el mismo estado — las dos ciclando
      o las dos en ámbar
- [ ] **Ninguna punta quedó dando verde por reloj mientras la otra parpadea en ámbar**

---

## 6. ⚠️ Riesgos residuales — aceptados por el cliente el 01/08/2026

Están escritos aquí porque **el funcional debe conocerlos antes de firmar**, no después de un
incidente.

### Riesgo 1 — El verde se da sin confirmación del otro extremo

**Con el radio muerto es inevitable.** En operación normal, el Maestro no abre un carril hasta que el
Esclavo confirma que está en rojo. En Degradado esa confirmación **no existe**: cada unidad da verde
porque su reloj dice que le toca.

Se mitiga con activación manual verificada, todo-rojo ampliado a 30 s, límite duro de 48 h y aviso en
pantalla. **No se elimina.** Es el precio de operar sin enlace, y por eso este modo es un caso
especial y no el comportamiento por defecto.

### Riesgo 2 — Salida asimétrica: que una sola punta abandone el Degradado

**Es el escenario más peligroso y no tiene solución técnica sin radio.**

```
   Un microcorte reinicia UNA unidad
        -> arranca en el MENÚ (así lo hace main.cpp)
        -> sin enlace  ->  ÁMBAR          el conductor NEGOCIA el paso
   La otra sigue dando verde por reloj    el conductor pasa CONFIADO
```

Un lado en ámbar contra un lado en verde es **exactamente lo que este modo quiere evitar**: el
conductor del lado en verde entra confiado a un tramo que el otro lado está negociando. Ocurre igual
—sin microcorte de por medio— **si un operario saca del Degradado una sola unidad**.

> **Mitigación procedimental, no técnica: la verificación visual de ambas puntas es obligatoria
> también AL SALIR.** Debe constar en el acta de pruebas (`3_Protocolo_Pruebas_Rigurosas.md`,
> Sección 9).

**Pendiente conocido (N-20):** hoy el estado del Degradado y la marca de sincronización viven en RAM,
así que un microcorte los pierde. El módulo `respaldo.cpp` que los guarda en los registros de
respaldo —alimentados por la misma pila CR2032 ya instalada— **está escrito pero todavía no
conectado**. Mientras no lo esté, **cualquier corte de energía en una punta produce el escenario de
arriba**.

---

## 7. Resumen de parámetros

| Parámetro | Valor | Dónde está |
|---|---|---|
| Verde de cada punta | **30 s** | `DEG_VERDE_SEG` |
| Todo-rojo entre verdes | **30 s** *(ya ampliado — el normal son 15 s)* | `DEG_DESPEJE_SEG` |
| Ciclo completo | **120 s** · espera máxima 90 s | 2 × (30 + 30) |
| Antigüedad máxima de la sincronización para entrar | **2 h** | `SYNC_FRESCA_MS` |
| Tolerancia de desfase para entrar | **±3 s** | `TOLERANCIA_DESFASE_S` |
| Aviso de límite en pantalla | a partir de **44 h** | `AVISO_LIMITE_MS` |
| **Límite duro → ámbar** | **48 h** | `LIMITE_DURO_MS` |
| Secuencia de entrada desde el piso | `A · B · A · B` en ≤ 18 s → **4 destellos rojos** | `mando.cpp` |
| Secuencia a Automático | `A · A · A` en ≤ 12 s → **2 destellos rojos** | `mando.cpp` |
| Secuencia a Ámbar | `B · B · B` en ≤ 12 s → **3 destellos rojos** | `mando.cpp` |

**El ciclo degradado es fijo y propio: no hereda el verde configurado en Modo Automático.** Es
deliberado — un verde de 2 minutos con un todo-rojo de 30 s daría un ciclo de 5 minutos, y nadie
espera cinco minutos en un paso alternado sin invadir.

---

## 8. Qué hacer si algo va mal

| Síntoma | Causa probable | Qué hacer |
|---|---|---|
| La secuencia `A·B·A·B` responde con **ámbar rápido** en vez de 4 destellos | Falta alguno de los 5 requisitos | Subir al gabinete y leer el motivo en `CONFIGURACION → MODO DEGRADADO` |
| La secuencia **no responde nada** | El menú está abierto: **las secuencias se ignoran con el menú abierto** | Desde el piso se distingue: si las luces están ciclando, el menú **no** está abierto. Subir y salir del menú con `Botón 4` |
| Las dos puntas **ciclan pero desfasadas** | Relojes separados, o una unidad se reinició | `B·B·B` en **ambas** y volver a ámbar. No corregir a ojo |
| Una punta en **verde** y la otra en **ámbar** | Riesgo residual nº 2 — salida asimétrica | **`B·B·B` inmediatamente en la punta que da verde.** Es lo primero, antes de diagnosticar nada |
| Pantalla: `Limite 48h sin sync` | Se agotó el límite duro | Es correcto. Hay que **arreglar el radio**, no reactivar el modo |

---

## 9. Lo que este procedimiento NO cubre

Escrito aquí porque una limitación documentada vale más que una promesa:

- **No hay prueba de banco ni de campo todavía.** Todo lo anterior está validado en simulador.
- **El Esclavo no tiene receptor de mando** (N-19). Todo lo que este documento dice del mando aplica
  **solo al Maestro**.
- **El estado no sobrevive a un corte de energía** (N-20). `respaldo.cpp` está escrito pero sin
  conectar.
- **La configuración del ciclo se sincroniza pero todavía no se consume** en el cálculo del ciclo
  (N-18): hoy ambas puntas usan los 30/30 fijos compilados. Mientras los dos firmwares sean de la
  misma versión, coinciden — **pero flashear versiones distintas en cada punta rompería la fase sin
  aviso**.
- **El RTC no se ha contrastado contra hora patrón** ni se ha comprobado que conserve la hora tras
  desconectar la alimentación (N-15, N-17).
