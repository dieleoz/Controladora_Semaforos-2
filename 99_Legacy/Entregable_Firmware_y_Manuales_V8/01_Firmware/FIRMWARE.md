# Arquitectura del Firmware V7 - Controladora de Semáforos

Este documento desglosa la arquitectura de software del código fuente alojado en `01_Firmware/`, analizando la estructura de proyectosPlatformIO, los módulos principales y el ciclo de vida del sistema.

---

## 1. Estructura de Proyectos PlatformIO (V7 Definitiva)

El firmware está organizado en **3 proyectos PlatformIO independientes**, cada uno con su propio entorno `platformio.ini` y compilación optimizada:

```text
01_Firmware/
├── Maestro/                # Proyecto PlatformIO para STM32F103 (Semáforo Maestro)
│   ├── platformio.ini      # [env:maestro] (genericSTM32F103C8)
│   └── src/                # main.cpp, coordinador.cpp, protocolo.cpp, semaforo.cpp, etc.
├── Esclavo/                # Proyecto PlatformIO para STM32F103 (Semáforo Esclavo)
│   ├── platformio.ini      # [env:esclavo] (genericSTM32F103C8)
│   └── src/                # main.cpp, protocolo.cpp, semaforo.cpp
└── Repetidor/              # Proyecto PlatformIO para ESP32 (Repetidor de Radio RS485/LoRa)
    ├── platformio.ini      # [env:repetidor] (espressif32 / esp32dev)
    ├── include/            # pines_repetidor.h
    └── src/                # main.cpp (Passthrough Asíncrono V7)
```

---

## 2. Mapa de Módulos (C++ V7)

| Módulo | Archivos | Responsabilidad |
|---|---|---|
| **Gestor Maestro** | `Maestro/src/main.cpp` | Orquesta los modos de operación (`MENU`, `MODO_MANUAL`, `MODO_AUTOMATICO`, `MODO_INTELIGENTE`), UI en ST7920 y Watchdog CKS32. |
| **Gestor Esclavo** | `Esclavo/src/main.cpp` | Receptor autónomo de órdenes RF. Responde `0x05 (PONG)` al `0x04 (PING)` cada 1.0s y emite `CMD_ACK_RED` / `CMD_ACK_GREEN` inmediatos. |
| **Coordinador** | `Maestro/src/coordinador.cpp` | Cerebro del ciclo de paso (`QV_MASTER` ↔ `QV_ESCLAVO`). PING rutinario a 1.0s, timeout ACK ampliado a 2.5s por retry y fallback a 3.0s. |
| **Protocolo Binario V7** | `protocolo.cpp` (Maestro y Esclavo) | Implementación de **CRC-8 Maxim (`0x31`)** bit a bit. Retardo `delayMicroseconds(1200)` tras `Bus.flush()` en RS485 y parser de IA (`AI_CARS:XX\n`) con auto-reseteo. |
| **Hardware Abstraction** | `semaforo.cpp/h`, `pines.h` | Control de MOSFETs (`ROJO1`, `VERDE1`, etc.). Enclavamiento anti-verde simultáneo (`SFTY-2`) y secuencia vial de Colombia 2024 (OPT-6). |
| **Repetidor ESP32 V7** | `Repetidor/src/main.cpp` | Reenvío bidireccional asíncrono sub-milisegundo entre `RadioA` y `RadioC` sin `delay(5)` ni impresiones bloqueantes. |

---

## 3. Topología de Red, CRC-8 y Handshake

La comunicación utiliza tramas binarias compactas de 4 bytes (`RF_Packet`):
- `msgID` (1 byte): Identificador incremental de secuencia contra Replay Attacks.
- `command` (1 byte): Comandos (`0x01` GO_GREEN, `0x02` GO_RED, `0x03` ACK_GREEN, `0x04` PING, `0x05` PONG).
- `param` (1 byte): Parámetro de tiempo o estado.
- `crc` (1 byte): Polinomio **CRC-8 Maxim/Dallas (`0x31`)** que descarta cualquier corrupción por ruido RF.

### Handshake y Fallback:
- **Heartbeat:** Maestro envía PING (`0x04`) cada 1.0s. Esclavo responde PONG (`0x05`) de inmediato.
- **Timeout de Caída:** Si pasan 3.0s sin comunicación, ambos nodos entran simultáneamente a `S_FALLO` (ámbar/rojo intermitente).
- **Auto-Healing (Fresh Start):** Al recuperar el enlace, el sistema fuerza Rojo Fijo de Despeje (All-Red) durante el tiempo configurado (hasta 999s) antes de dar paso a cualquier carril.
