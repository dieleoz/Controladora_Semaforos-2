# Arquitectura del Firmware V7.6 Definitivo - Controladora de Semáforos

Este documento desglosa la arquitectura de software del código fuente alojado en `01_Firmware/`, analizando la estructura de proyectos PlatformIO, los módulos principales y el ciclo de vida del sistema.

---

## 1. Estructura de Proyectos PlatformIO (V7.6 Definitiva)

El firmware está organizado en ~~**3**~~ **4** proyectos PlatformIO independientes, cada uno con su propio entorno `platformio.ini` y compilación optimizada:

```text
01_Firmware/
├── Maestro/                # Proyecto PlatformIO para STM32F103 (Semáforo Maestro)
│   ├── platformio.ini      # [env:maestro] (genericSTM32F103C8)
│   └── src/                # main.cpp, coordinador.cpp, protocolo.cpp, semaforo.cpp,
│                           #   modo_*.cpp, mando.cpp, reloj.cpp, respaldo.cpp,
│                           #   bluetooth.cpp, demanda.cpp, identidad.cpp, ... (21 .cpp)
├── Esclavo/                # Proyecto PlatformIO para STM32F103 (Semáforo Esclavo)
│   ├── platformio.ini      # [env:esclavo] (genericSTM32F103C8)
│   └── src/                # main.cpp, protocolo.cpp, semaforo.cpp, mando.cpp,
│                           #   modo_degradado.cpp, bluetooth.cpp, reloj.cpp, ...
├── Repetidor/              # Proyecto PlatformIO para ESP32 (Repetidor de Radio RS485/LoRa)
│   ├── platformio.ini      # [env:repetidor] (espressif32 / esp32dev)
│   ├── include/            # pines_repetidor.h
│   └── src/                # main.cpp (Passthrough Asincrono V7.6)
└── ESP32_Expansion/        # 🆕 Modulo de J17: reloj DS3231 con pila, puente Bluetooth
    ├── platformio.ini      # [env] ESP32
    ├── include/            # contrato.h (LATIDO_MS), reloj_ds3231.h, despachador.h, ...
    └── src/                # main.cpp, puente.cpp, despachador.cpp, reloj_ds3231.cpp
```

> ⚠️ **`lcd.cpp` sigue en las dos puntas pero ya no habla con ninguna pantalla.** Sus tres pines
> están en `U8X8_PIN_NONE` desde el 05/09 y `J17` lo ocupa el `ESP32_Expansion`. Compila, ocupa
> flash y **no mueve un bus**; lo vigila `costura_11_lcd_sin_bus`.

---

## 2. Mapa de Módulos (C++ V7.6)

| Módulo | Archivos | Responsabilidad |
|---|---|---|
| **Gestor Maestro** | `Maestro/src/main.cpp` | Orquesta ~~los modos de operación (`MENU`, `MODO_MANUAL`, `MODO_AUTOMATICO`, `MODO_INTELIGENTE`), UI en ST7920~~ **los OCHO modos de `ModoSistema` (ver debajo)** y Watchdog CKS32. |
| **Gestor Esclavo** | `Esclavo/src/main.cpp` | Receptor autónomo de órdenes RF. Responde `0x05 (PONG)` al `0x04 (PING)` cada 3.0s y emite `CMD_ACK_RED` / `CMD_ACK_GREEN` inmediatos. |
| **Coordinador** | `Maestro/src/coordinador.cpp` | Cerebro del ciclo de paso (`QV_MASTER` ↔ `QV_ESCLAVO`). PING rutinario a 3.0s ~~(0.3k)~~ **(2.4 kbps)**, timeout ACK de ~~5.0s~~ **3.5s**, fallback a ~~12.0s~~ **25.0s** y gestión del Menú Seguro e independizado. |
| **Protocolo Binario V8.0** | `protocolo.cpp` (Maestro y Esclavo) | **CRC-8 Maxim (`0x31`)** bit a bit, **ráfaga configurable `RF_BURST_COPIES` (~~=1~~ **=3**) + Ventana Deslizante**, `msgID` acotado a 1..255, `protocolo_resetReplayProtection()` y retardo `delayMicroseconds(1200)` tras `Bus.flush()`. |
| **Hardware Abstraction** | `semaforo.cpp/h`, `pines.h` | Control de MOSFETs (`ROJO1`, `VERDE1`, etc.). Enclavamiento anti-verde simultáneo (`SFTY-2`) y secuencia vial Colombia 2024 (Rojo $\rightarrow$ Amarillo 4s $\rightarrow$ Verde). |
| **Repetidor ESP32 V7.6** | `Repetidor/src/main.cpp` | Reenvío bidireccional asíncrono sub-milisegundo entre `RadioA` y `RadioC` sin `delay(5)` ni impresiones bloqueantes. |

### 🔴 Los modos son OCHO, no cuatro — y la UI en ST7920 ya no existe (05/09/2026)

La fila del Gestor Maestro nombraba **cuatro de ocho** modos y una interfaz retirada. La lista no se
escribe a mano: sale del `enum`.

```
$ grep -n "MENU\|MODO_" 01_Firmware/Maestro/include/modos.h
18:  MENU,
19:  MODO_MANUAL,
20:  MODO_AUTOMATICO,
21:  MODO_INTELIGENTE,
22:  MODO_ALCANCE,
23:  MODO_HORA,       // SFTY-18: ajuste del reloj. No arranca ciclos.
24:  MODO_DEGRADADO,  // SFTY-21: operacion por reloj, sin radio. Activacion MANUAL.
30:  MODO_AMBAR
```

**Y cómo se llega a cada uno, que es lo que le falta a quien está delante del equipo:**

| | quién lo arma hoy |
|---|---|
| `MENU` · `MODO_MANUAL` · `MODO_AUTOMATICO` · `MODO_INTELIGENTE` · `MODO_ALCANCE` · `MODO_DEGRADADO` · `MODO_AMBAR` | **Bluetooth** (`SET_MODO:*` en `Maestro/src/bluetooth.cpp`) — **7 de los 8** |
| `MODO_AUTOMATICO` · `MODO_AMBAR` · `MODO_DEGRADADO` | además, el **mando de relés**: `A.A.A`, `B.B.B` y `A.B.A.B` (`Maestro/src/mando.cpp`) |
| 🔴 **`MODO_HORA`** | **NADIE. Es inalcanzable.** |

> 🔴 **`MODO_HORA` no se puede alcanzar, y conviene decirlo aquí porque es el modo que pone el
> reloj.** Tiene **un solo armador**, y cuelga de un botón que hoy devuelve `false`:
>
> ```
> $ grep -r "modoActual_set(MODO_HORA)" 01_Firmware/Maestro 01_Firmware/Esclavo --include=*.cpp
> 01_Firmware/Maestro/src/menu.cpp:        case 1:  modoActual_set(MODO_HORA);      break;
>
> $ grep -r "bool botonAceptar" 01_Firmware/Maestro/src/botones.cpp
> bool botonAceptar() { return false; }
> ```
>
> *(Sin `-n` a propósito: el número de línea de `botonAceptar()` cambió de `539` a `604` entre dos
> greps de esta misma sesión. Se cita el símbolo y el comando que lo encuentra, §4.sexies.)*
>
> Ese único `modoActual_set(MODO_HORA)` vive dentro de `if (botonAceptar())`, así que la rama no
> corre nunca. **El camino
> vivo del reloj es Bluetooth**: `CMD:SET_RTC` y `CMD:LEER_RTC` contra el ESP32 de `J17`, que es
> donde está el reloj con pila. La pantalla `AJUSTAR HORA` ya no es una vía: es código sin puerta.

### ⚠️ Falta un cuarto proyecto: `ESP32_Expansion`

El árbol de §1 lista tres proyectos y en `01_Firmware/` hay **cuatro** que van al equipo:
`Maestro/`, `Esclavo/`, `Repetidor/` y **`ESP32_Expansion/`** —el módulo de `J17` que trae el
reloj DS3231, el puente Bluetooth y el latido `LATIDO_MS` que sostiene los contadores de silencio
de `bluetooth.cpp`—. No es un accesorio: **es donde vive el único camino vivo para poner la hora.**

---

## 3. Topología de Red, CRC-8 y Handshake

La comunicación utiliza tramas binarias compactas de 4 bytes (`RF_Packet`):
- `msgID` (1 byte): Identificador incremental de secuencia contra Replay Attacks.
- `command` (1 byte): Comandos (`0x01` GO_GREEN, `0x02` GO_RED, `0x03` ACK_GREEN, `0x04` PING, `0x05` PONG). ⚠️ **Ésos son 5 de los 21 que hay hoy**: el protocolo llega hasta `0x15`. La lista se relee del C++, no de aquí — `grep "^#define CMD_" Maestro/include/protocolo.h` da además `ACK_RED` (`0x06`), la familia horaria `HORA_H`/`HORA_M`/`HORA_S`/`ACK_HORA`/`HORA_D`, `DELTA`/`DELTA_RESP`, `CONFIG_VERDE`/`CONFIG_DESPEJE`/`ACK_CONFIG`, `DEMANDA`/`ACK_DEMANDA`, `GO_AMBAR`, `AMBAR_ESCLAVO` y `CANCELA_AMBAR_ESCLAVO`.
- `param` (1 byte): Parámetro de tiempo o estado.
- `crc` (1 byte): Polinomio **CRC-8 Maxim/Dallas (`0x31`)** que descarta cualquier corrupción por ruido RF.

### Handshake, Fallback y Self-Healing:
- **Heartbeat:** Maestro envía PING (`0x04`) cada 3.0s a ~~300 bps~~ **2.4 kbps** (Air Data Rate vigente; `M0`/`M1` ambos en OFF). Esclavo responde PONG (`0x05`) tras `RETARDO_RESPUESTA_MS` (SFTY-17).
- **Timeout de Caída:** Si pasan ~~12.0s~~ **25.0s** (`SFTY6_SILENCIO_MS`) sin comunicación durante un modo de operación, ambos nodos entran a `S_FALLO` (amarillo intermitente). **Ese número es el TECHO de la ventana de reintentos, no un ajuste suelto** — N-71: con 12.0s los reintentos 4 y 5 de `CICLO_MAX_REINTENTOS` eran código muerto. Lo recalcula `costura_09_presupuesto_radio` desde el C++ en cada corrida.
- **Auto-Recuperación Real (Self-Healing):** Al recuperar el enlace, `protocolo_resetReplayProtection()` resetea los IDs de duplicados, envía `CMD_GO_RED` y fuerza Rojo Fijo de Despeje (All-Red de 15s) en ambos extremos antes de reanudar el flujo.
