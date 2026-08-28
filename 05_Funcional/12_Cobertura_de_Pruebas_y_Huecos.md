# 🧪 COBERTURA DE PRUEBAS Y HUECOS — V9.0

**Fecha:** 26 de Agosto de 2026
**Método:** censo automático de la superficie de entrada del firmware cruzado contra los 20 packs
del banco. **No es una opinión: cada fila se levantó con una búsqueda sobre el código**, y el
comando que la produce está anotado para que cualquiera la repita.

---

## 0. Por qué existe este documento

Los defectos de V9.0 —cámaras en pines de botones, PIN sin validar, radio sin direccionar, Esclavo
que mueve luces por Bluetooth, app incapaz de hablar SPP— **se encontraron uno a uno, preguntando**.
Ninguno salió de una revisión sistemática. Un método que depende de que a alguien se le ocurra la
pregunta correcta **garantiza que quedan defectos**: solo aparecen los que se preguntaron.

Este documento es la pasada que faltaba. Y su primer resultado es un defecto que nadie había visto.

---

## 1. 🔴 Hallazgo de esta pasada: las cámaras 2 y 4 no existen en el firmware

```
$ grep -rn "CAM_UMBRAL_PIN" --include=*.cpp Maestro/src Esclavo/src
Maestro/src/modo_inteligente.cpp:29:  pinMode(CAM_UMBRAL_PIN, INPUT_PULLUP);
Esclavo/src/main.cpp:275:            pinMode(CAM_UMBRAL_PIN, INPUT_PULLUP);
```

**Eso es todo.** El pin se configura como entrada y **no se lee nunca**: no hay un solo
`digitalRead(CAM_UMBRAL_PIN)` en ninguno de los dos microcontroladores. El censo completo de
entradas físicas lo confirma —solo aparece `CAM_DEMANDA_PIN`—.

Mientras tanto, **cuatro documentos afirman lo contrario**: el Manual 9 de cámaras, el Manual 10 de
Bluetooth, `ESTADO.md` y `roadmap.md` describen `PB8` como *"Umbral de tramo"* con función activa.

> **Un `pinMode()` sin `digitalRead()` es exactamente el patrón de la prueba muerta que este
> repositorio ya conoce:** algo que *parece* conectado, que compila, que pasa la compuerta, y que no
> hace nada. Se descubrió censando, no leyendo el manual — porque el manual decía que sí funcionaba.

---

## 2. Superficie de entrada completa, cruzada con el banco

Todo lo que puede meter información al sistema, y qué lo prueba:

| # | Entrada | Canal | Qué acepta | Pack que lo cubre | Estado |
|---|---|---|---|---|---|
| 1 | 4 botones locales | `PB9` `PB13` `PB14` `PB15` | secuencias A/B | `maestro_01_mando` (15), `esclavo_02_inhibicion_menu` (7) | ✅ |
| 2 | Mando de 4 relés | **los mismos 4 pines** | idem | idem | ✅ |
| 3 | Cámara de demanda | `PB0` | binario con antirrebote | **ninguno** | ❌ |
| 4 | Cámara de umbral | `PB8` | **nada: no se lee** | **ninguno** | 🔴 §1 |
| 5 | Radio LoRa | `USART3` | **18** comandos `CMD_*` | `costura_03_comandos` (7 chk) | ⚠️ parcial |
| 6 | Bluetooth Maestro | `USART1` | 7 comandos | **ninguno** | ❌ |
| 7 | Bluetooth Esclavo | `USART1` | 3 comandos | **ninguno** | ❌ |
| 8 | Respaldo en pila | BKP RAM | firma + checksum | `maestro_02_respaldo` (19) | ✅ |
| 9 | Reloj / pila | RTC | hora, validez | `maestro_04_sync_horaria` (11), `esclavo_05_hora_atomica` (7) | ✅ |
| 10 | Corte de energía | — | reanudación | `costura_06_reanudacion` (9) | ✅ |

**El patrón salta a la vista: todo lo anterior a V9.0 está cubierto; todo lo que V9.0 añadió, no.**
Los 20 packs suman `155/155` — exactamente los mismos que antes del Bluetooth, las cámaras y los dos
comandos de radio nuevos.

### 2.bis Los comandos de radio, uno a uno

`protocolo.h` define 18. `costura_03_comandos.py` no se toca desde la Fase 2:

```
CMD_GO_GREEN  CMD_GO_RED  CMD_ACK_GREEN  CMD_PING  CMD_PONG  CMD_ACK_RED
CMD_HORA_D  CMD_HORA_H  CMD_HORA_M  CMD_HORA_S  CMD_ACK_HORA
CMD_DELTA  CMD_DELTA_RESP  CMD_CONFIG_VERDE  CMD_CONFIG_DESPEJE  CMD_ACK_CONFIG
CMD_DEMANDA  CMD_ACK_DEMANDA          <- los dos nuevos, sin cubrir
```

---

## 3. Invariantes de seguridad, y quién intenta romperlos

Un invariante sin una prueba que **intente violarlo** es una intención, no una garantía.

| Invariante | ¿Quién lo intenta romper? |
|---|---|
| Ningún pin de luz se escribe fuera de `semaforo.cpp` | `barrera_01_pines_de_luz` ✅ |
| Nunca verde y rojo a la vez en la misma cabeza (SFTY-2) | `Validacion_Automatico`, sobre pines escritos ✅ |
| **Nunca verde simultáneo en las DOS puntas** | 🔴 **nadie.** `Validacion_Automatico` solo compila el Maestro |
| El despeje nunca baja del configurado | ⚠️ indirecto en `costura_02` |
| No se reanuda el Degradado sin autorización válida | `costura_06`, `maestro_03_puerta_degradado` ✅ |
| Un equipo sin hora no autoriza el Degradado | `maestro_04`, `esclavo_05` ✅ |
| **Un comando de Bluetooth no abre paso sin PIN** | 🔴 **nadie** |
| **Un equipo no obedece a una pareja ajena** | 🔴 **nadie** — y hoy no hay ni direccionamiento |

La tercera fila es la que mata, y es la que este repositorio ya sabe que no mide: `CLAUDE.md §8` lo
dice del arnés del automático — *"solo el Maestro: verde simultáneo en las dos puntas no se mide
ahí"*.

---

## 4. Plan de pruebas — qué escribir, y en qué orden

Cada pack se conecta a `compuerta.py` **solo después de haberlo visto fallar** con un defecto
inyectado a propósito en el `.cpp` real (`CLAUDE.md §8.bis`).

| # | Pack | Qué debe romper | Control negativo obligatorio |
|---|---|---|---|
| 1 | `bluetooth_01_autorizacion` | que un comando sin PIN válido abra paso | trama sin PIN → debe ser rechazada |
| 2 | `bluetooth_02_esclavo_no_abre` | que el Esclavo ejecute algo que encienda un verde | `TEST_LEDS` en servicio → rechazado |
| 3 | `costura_08_pareja` | que un equipo atienda una trama de otra pareja | `PAIR` ajeno → descartada |
| 4 | `camara_01_demanda` | antirrebote y que la demanda no acorte el despeje | demanda continua → el despeje **no** baja |
| 5 | `costura_03` (ampliar) | que `0x11`/`0x12` cuadren entre las dos puntas | totales `7 → 9` |
| 6 | `barrera_02_dos_puntas` | **verde simultáneo en las dos puntas** | forzar verde en ambos → debe saltar |

El **6** es el que falta desde siempre y el más caro: exige un arnés que compile los dos firmwares a
la vez. Es también el único que mide la propiedad que puede matar a alguien.

---

## 5. Qué NO cubre este documento

- **La app móvil no tiene ninguna prueba**, de ningún tipo. Ni unitaria, ni de integración.
- **Nada de esto sustituye la prueba de banco.** Un pack demuestra que el modelo y el código
  coinciden; no que la tarjeta encienda una bombilla.


---

## 6. 🔴🔴 Hallazgo mayor: `PB0`/`PB8` están asignados dos veces, y el reloj llegó primero

Esta pasada cruzó **todos** los documentos que nombran `PB0` o `PB8`. El resultado no es una errata:

| Documento | Dice que `PB0`/`PB8` son… | Fecha |
|---|---|---|
| `roadmap.md` **N-37** | **`SDA`/`SCL` del `DS3231`** — *"los únicos pines libres"* | **cerrado en banco el 01/08/2026** |
| Manual 11 · `MANUAL_INSTALACION_RELOJ_DS3231.md` | `SDA`/`SCL` del `DS3231` | V9.0 |
| Manual 9 · Manual 2 · `MAPEO:179` · `ESTADO.md` | **cámaras IA** (demanda y umbral) | V9.0 |

**No caben las dos cosas.** Y la que llegó primero tiene evidencia de banco detrás.

### Lo que dice N-37, y por qué no es negociable

> *"**CERRADO POR ELIMINACIÓN: el cristal `Y2` está MUERTO. Banco del 01/08/2026.**"*

Con tres eliminaciones medidas —`VBAT` a 3 V con la tarjeta apagada, el reintento de `N-25`, y
`REINICIAR RELOJ` devolviendo `SIGUE PARADO`—. Y su conclusión: *"Ya no queda software que probar.
**Salida: `DS3231` por I²C software en `PB0`/`PB8`.** Hacen falta **DOS**"*.

**El `MAPEO §4` dice lo contrario** —*"que el cristal sea el culpable no está medido"*— pero su
cabecera lo fecha: **«Última revisión: 31 de julio de 2026»**, un día **antes** de la medida de
banco. No es una contradicción: es un documento sin actualizar, y hay que corregirlo antes de que
alguien lo cite como si valiera.

### Por qué esto invalida la arquitectura de V9.0

1. El firmware sigue en `STM32RTC` sobre el cristal muerto (`reloj.cpp:109`, `LSE_CLOCK`). **No hay
   driver `DS3231`.**
2. V9.0 acaba de darle a las cámaras **los dos pines que el reloj necesita**.
3. Sin reloj no hay Modo Degradado: `SFTY-18` lo prohíbe, y con razón.

### Y la salida que aparece al mirarlo entero

De los dos pines de cámara, **`PB8` no se lee nunca** (§1). O sea que la demanda real necesita **un
solo pin**, no dos. Y aun así 1 + 2 = 3 > 2.

**Un expansor `PCF8574` en el mismo bus I²C lo resuelve:** cuelga de `PB0`/`PB8` junto al `DS3231`,
cuesta céntimos, y aporta **8 entradas digitales** para las cámaras — con lo que sobran seis.

```
   PB0 (SDA) ──┬─────────────┬───────────────
   PB8 (SCL) ──┴──┐       ┌──┴──┐
                  │       │     │
              [DS3231]  [PCF8574]  ← 8 entradas: camaras 1..4 y margen
```

**Ventaja adicional que no es menor:** deja de haber pines libres en disputa. Cualquier entrada
futura entra por el expansor sin volver a abrir esta discusión.

**Riesgo a medir, no a suponer:** el I²C es por software y ahora tendría dos esclavos; hay que
comprobar que la latencia de leer el expansor no compite con el `IWDG` ni con el bit-bang del LCD.
Eso se mide en banco, no aquí.
