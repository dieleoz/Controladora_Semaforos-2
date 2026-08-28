# Controladora de Semáforos (Master/Slave) - Vía de 1 Carril Alterno

Este repositorio contiene el firmware para el control de semáforos en situaciones donde solo hay un carril disponible (ej. pasos a nivel, túneles, taludes en obras). El sistema opera en arquitectura Maestro/Esclavo utilizando comunicación por radiofrecuencia (módulos Gamma LoRa a 170MHz mediante interfaz serial y MAX485).

## Arquitectura y "Deber Ser" Lógico

El "Deber Ser" para un sistema de semaforización en un tramo alterno de un solo carril requiere estrictas medidas de seguridad (Fail-Safe) para evitar colisiones:

### 1. Estado Seguro (Fail-Safe) ante pérdida de comunicación
Si el enlace de radio entre el Maestro y el Esclavo falla, el sistema nunca debe permitir el paso simultáneo de vehículos de ambos lados. 
- **Pérdida de Comunicación:** Si el Maestro no recibe respuesta del Esclavo en un tiempo prudencial, ambos semáforos deben pasar automáticamente a **ROJO PARPADEANTE** o **ROJO FIJO** (estado de emergencia) hasta que la comunicación se restablezca.

### 2. Independencia de Interfaz (UI)
El acceso al menú (Pantalla LCD y Botones) debe estar siempre disponible en el Maestro, **independientemente** de si hay conexión por radio o no. La inicialización de la red (Handshake) opera en segundo plano y no debe bloquear al usuario, permitiendo cambiar modos de operación y ajustar tiempos localmente en todo momento.

### 3. Secuencia Alterna (Modo Automático)
Para una vía de un solo carril, el ciclo lógico diseñado es:
1. **Despeje Inicial:** Ambos semáforos en ROJO (permite que los vehículos que ya están transitando salgan del tramo).
2. **Flujo A:** Maestro en VERDE, Esclavo en ROJO.
3. **Transición A:** Maestro en AMARILLO, Esclavo en ROJO.
4. **Despeje Intermedio:** Ambos en ROJO (Tiempo estático configurable según la longitud del tramo).
5. **Flujo B:** Maestro en ROJO, Esclavo en VERDE.
6. **Transición B:** Maestro en ROJO, Esclavo en AMARILLO.
7. **Retorno al ciclo.**

## Hardware Utilizado
- **Microcontrolador**: STM32F103C8 (BluePill)
- **Interfaz de Usuario**: LCD ST7920 128x64, manejado por la librería U8g2 (SPI/Serial)
- **Comunicación**: Radios Gamma LoRa a 170 MHz (conectados a través de un transceiver MAX485 para control de pines DE/RE o conexión serial transparente).

## Modos de Operación
1. **Manual:** El operador avanza el ciclo de los semáforos de manera segura presionando el botón Aceptar.
2. **Automático:** El coordinador cicla los estados automáticamente basado en los tiempos configurados de Verde, Rojo, y Tiempo Estático.
3. **Por Demanda:** El ciclo cambia basado en peticiones externas.

## Configuración y Entorno de Desarrollo (PlatformIO)
- El entorno recomendado para compilar y cargar este firmware es **Visual Studio Code** con la extensión **PlatformIO**.
- El proyecto usa el framework Arduino para STM32 (`ststm32` core).
- Para subir el código, asegúrate de tener conectado un ST-Link o usar el bootloader adecuado según la configuración en `platformio.ini`.
