# 📡 ESPECIFICACIÓN TÉCNICA DE ANTENAS — Fabricación bajo pedido

**Proyecto:** Controladora de Semáforos Móviles de 3 Estados
**Fecha:** 1 de Agosto de 2026
**Cantidad requerida:** **2 unidades idénticas**

---

## 1. Por qué existe este documento

En la prueba de campo del 31/07/2026 el alcance medido fue de **1 cuadra, esquina y 2 cuadras más**.
Para radios de 1 W a 170 MHz eso es muy poco.

**Causa identificada:** las antenas instaladas eran genéricas de "LoRa", fabricadas para **433 o
915 MHz**, montadas en radios que transmiten en **170 MHz**. Fuera de banda una antena no solo radia
mal: **devuelve la potencia al amplificador del radio**. Es la explicación más probable de la avería
del transmisor de una de las radios.

Este documento existe para que la antena de reemplazo **se especifique por su frecuencia real**, no
por una etiqueta comercial.

---

## 2. Especificación

| Parámetro | Valor exigido |
|---|---|
| **Frecuencia central de sintonía** | **171 MHz** *(punto medio entre los dos canales en uso)* |
| **Ancho de banda útil** | **168 – 174 MHz** con **ROE ≤ 1,5:1** en todo el rango |
| **Frecuencias de operación** | **170 MHz y 172 MHz** — ambas deben quedar dentro del rango de baja ROE |
| Impedancia | **50 Ω** |
| Polarización | **Vertical** |
| Patrón de radiación | **Omnidireccional** en el plano horizontal |
| Tipo constructivo | **Colineal / arreglo de dipolos, alimentada al centro** |
| **Requisito de plano de tierra** | **NINGUNO** — se instala en punta de mástil, sin masa metálica disponible |
| Ganancia | **3 – 4 dB** *(configuración de 2 dipolos)* |
| **Longitud máxima** | **1,5 m** |
| Peso máximo orientativo | 2 kg |
| Potencia admisible | ≥ 10 W *(el sistema transmite 1 W; el margen es holgura, no necesidad)* |
| Montaje | **Abrazadera para mástil de _____ mm de diámetro** *(medir el poste antes de pedir)* |
| **Conector de salida** | **SMA macho**, o `SO-239` **con pigtail a SMA macho incluido** |
| Cantidad | **2 unidades** |

### 2.1 Entregable exigido junto con la antena

> ## 📄 **Reporte de medición de ROE a 170 MHz y 172 MHz**

Es el único documento que demuestra que la antena está donde dice estar. **Sin ese reporte no hay
forma de distinguir una antena correcta de una que solo lo afirma** — y es exactamente el control que
habría evitado el problema original.

Debe permitir verificar la antena **antes** de subirla a 6 metros de altura.

---

## 3. Por qué 2 dipolos y no 4

El fabricante puede construir una versión de **6 dB con 4 dipolos**, pero mide **3 m y pesa 5 kg**.

| Configuración | Ganancia | Longitud | Peso |
|---|---|---|---|
| **2 dipolos** ← recomendada | 3 – 4 dB | ~1,5 m | ~2 kg |
| 4 dipolos | 6 dB | 3 m | 5 kg |

La diferencia real es de **2 – 3 dB por punta**. En una unidad **móvil**, que se transporta y reubica,
eso no compensa el peso en la punta del mástil, la carga de viento ni la fragilidad en el traslado.

> **Excepción:** si el mástil admite el peso con holgura y el equipo **no se baja con frecuencia**,
> la de 4 dipolos aporta ~5 dB al enlace total. Es una decisión de mecánica, no de radio.

---

## 4. Contexto de instalación

```
        ╱▔▔╲   ← antena, punta a ~7,5 m
         ││
    ═════╪═════  6 m — tope del mástil
         ││
       ┌─┴─┐
       │GAB│    5 m — gabinete con la radio E90-DTU
       └───┘
         ││       ~2 m de coaxial: pérdida despreciable
```

**La antena va por encima del gabinete y de cualquier estructura metálica.** A VHF, la altura rinde
más que la potencia: los 5 m de gabinete ya juegan a favor.

---

## 5. Producto de referencia del proveedor

De los nueve modelos del catálogo de Ditelcom, **el único que cumple lo estructural** —sin plano de
tierra, montaje en mástil, arreglo de dipolos— es la **`DAN-OV4D6S`, omnidireccional de 4 dipolos en
corbatín** (6 dB · 3 m · 5 kg). Las `REF-101` y `REF. 102` son de cuarto de onda con montaje pasante:
**exigen lámina metálica como plano de tierra**, que en punta de mástil no existe.

Como el proveedor fabrica bajo pedido, la petición concreta es:

> **«La `DAN-OV4D6S`, pero en versión de 2 dipolos y sintonizada a 171 MHz, con ROE ≤ 1,5:1 entre
> 168 y 174 MHz.»**

Referenciar su propio producto evita explicaciones: solo cambian el **tamaño** y la **frecuencia**.

---

## 6. BOM — Lista de materiales para 2 equipos

| # | Cant. | Descripción | Especificación crítica |
|---|---|---|---|
| 1 | **2** | **Antena omnidireccional de dipolos**, tipo `DAN-OV4D6S` en versión **2 dipolos** | **171 MHz** · ROE ≤ 1,5:1 en 168–174 MHz · 50 Ω · vertical · **sin plano de tierra** · ≤ 1,5 m |
| 2 | **2** | **Cable coaxial RG-8X, 3 m, ensamblado de fábrica** | **`PL-259` (UHF macho) en un extremo y `SMA macho` en el otro** · 50 Ω |
| 3 | **2** | Abrazadera de montaje a mástil de _____ mm | Confirmar si viene incluida con la antena |
| 4 | **1** | Rollo de cinta vulcanizante (autoamalgamante) | Sellado del conector de intemperie |

### 6.1 Pedir el cable con el conector correcto en cada punta

```
   ANTENA ──[SO-239]═════ RG-8X 3 m ═════[SMA macho]── RADIO E90-DTU
              hembra       PL-259 macho                  SMA-K hembra
```

**Un solo cable con la terminación correcta a cada lado.** Comprar cable con `PL-259` en ambos
extremos más un adaptador añade **dos uniones adicionales**, y cada unión es un punto de pérdida y de
entrada de humedad.

*Alternativa si el proveedor solo maneja cable estándar:* 2 × RG-8X de 3 m con `PL-259` en ambos
extremos **más 2 × adaptador `SO-239` hembra → `SMA` macho**.

### 6.2 Por qué 3 metros y no más

El recorrido real entre el gabinete (5 m) y la antena (6 m) es de ~1,5 m. Los 3 m cubren el enrutado
interno del gabinete y dejan margen para rehacer un conector. **No pedir 10 m "por si acaso":** cada
metro adicional es atenuación pura.

### 6.3 Cantidad

Dos antenas cubren el montaje vigente —Maestro y Esclavo en **enlace directo**—. Al reinstalar el
repetidor el sistema vuelve a **4 radios** y harán falta **2 antenas más**: conviene pedir cotización
por 4 aunque se compren 2, por si hay diferencia por volumen.

> ⚠️ **No reinstalar las antenas genéricas antiguas en ningún radio.** Son las que con mayor
> probabilidad dañaron el transmisor de la radio B1.

> ⚠️ **Todo el conjunto debe ser de 50 Ω.** El coaxial de televisión, de 75 Ω, **no sirve**: introduce
> desadaptación aunque el conector encaje mecánicamente.

> ⚠️ **Pedir la antena y el latiguillo al mismo proveedor y armados de fábrica.** Un conector soldado
> en obra es una de las causas más frecuentes de pérdida, y anula el beneficio de una antena buena.

---

## 7. Advertencia de operación

> ⚠️ **Nunca energizar la radio sin la antena conectada.** Transmitir sin carga —o contra una antena
> desadaptada— daña el amplificador de salida. Antes de montar cualquier radio de reemplazo hay que
> revisar el coaxial y el conector, o se quema también el equipo nuevo.

---

## 8. Verificación al recibir

| # | Comprobación | Criterio |
|---|---|---|
| 1 | Reporte de ROE incluido | Presente, con lecturas **en 170 y 172 MHz** |
| 2 | ROE medida | **≤ 1,5:1** en ambas frecuencias |
| 3 | Conector | Enrosca en el `SMA-K` de la radio, directo o por pigtail |
| 4 | Longitud y peso | Dentro de lo especificado |
| 5 | Abrazadera | Compatible con el diámetro del mástil |

Tras la instalación, repetir la **prueba de alcance en campo** con la pantalla `PRUEBA ALCANCE` del
Maestro —ahora en **`Menú Principal → CONFIGURACION → PRUEBA ALCANCE`**, ya no cuelga del menú
principal— y **anotar el resultado** para compararlo contra la medición del 31/07 (1 cuadra, esquina y
2 cuadras). Ver `2_Manual_Hardware_y_Pruebas.md §2.1`.

> 📡 **Una razón más para no aplazar las antenas.** El Modo Degradado exige una sincronización horaria
> de **menos de 2 horas** de antigüedad para poder activarse, y esa sincronización viaja **por el
> mismo enlace de radio**. Con la cobertura actual de 3 cuadras, el enlace se pierde antes de lo que
> debería y **la ventana para activar el Degradado se cierra sin que nadie lo note**. Las antenas
> correctas no solo alargan el alcance: mantienen viva la condición que hace utilizable el modo de
> respaldo. Ver `8_Procedimiento_Modo_Degradado.md §2`.
