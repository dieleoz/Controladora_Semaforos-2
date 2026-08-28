# SOLICITUD DE VALIDACIÓN EN BANCO — HARDWARE CONTROLADORA DE SEMÁFOROS

Equipo: Controladora de Semáforos v1.0 — STM32F103C8Tx
Propósito: Cerrar las verificaciones de hardware y asegurar el correcto funcionamiento de la electrónica antes de iniciar el desarrollo del firmware final.

---

## A. MAPEO DE ENTRADAS OPTOACOPLADAS (Prioridad 1)

El esquemático muestra 9 entradas mediante optoacopladores TLP127 (U6 a U14).

**Procedimiento:** 
1. Inyectar señal (ej. 12V/24V o GND dependiendo de la configuración de la bornera) en cada entrada de la tarjeta.
2. Verificar con multímetro en los pines del STM32 (o corriendo un firmware de test básico) si la señal llega correctamente.
3. **Pregunta clave:** ¿Las entradas en el STM32 se leen como LOW (0) cuando están activas, o como HIGH (1)? Esto es vital para configurar los `pull-up` o `pull-down` en el firmware.

| Bornera de Entrada | Función (Ej. Botón, Sincronismo) | Pin STM32 asignado | Nivel Activo (LOW/HIGH) | Verificado por |
|---|---|---|---|---|
| IN 1 | | | | |
| IN 2 | | | | |
| ... | | | | |
| IN 9 | | | | |

## B. MAPEO DE SALIDAS MOSFET (Prioridad 1)

El esquemático muestra 9 salidas de potencia con MOSFETs IRLZ44N (Q1 a Q9), suficientes para 3 semáforos (Rojo, Amarillo, Verde).

**Procedimiento:**
1. Cargar un firmware de prueba que ponga en HIGH secuencialmente cada uno de los 9 pines del STM32 asignados a los MOSFET.
2. Medir en la bornera de salida que el voltaje conmute correctamente.

| Pin STM32 | Función Asignada (Ej. Semáforo 1 - Rojo) | Conmuta Correctamente (Sí/No) | Verificado por |
|---|---|---|---|
| | | | |
| | | | |

## C. PRUEBA DE ARRANQUE Y ESTADOS FLOTANTES (Prioridad 2)

Durante el reinicio o encendido, los pines del STM32 quedan en alta impedancia (flotantes) antes de que el código `setup()` los configure como salidas.

| Verificación | Resultado Esperado | Resultado Real |
|---|---|---|
| Al conectar la energía a la placa (antes de que arranque el código), ¿las luces de los semáforos destellan o se encienden solas un instante? | **NO** (Los MOSFET deberían tener resistencias pull-down en sus compuertas por hardware). | |
| ¿Los pines BOOT0 y BOOT1 del STM32 tienen los jumpers/resistencias correctas para arrancar siempre desde la Flash principal? | Arranca de inmediato al dar energía. | |

## D. COMUNICACIÓN RS-485 (Prioridad 2)

El diseño incluye 2 transceptores RS-485 (MAX3485: U2 y U3).

**Procedimiento:**
1. Identificar qué pines del STM32 controlan el **TX**, **RX** y los pines de dirección **DE/RE** de cada MAX3485.
2. Cargar un código que envíe caracteres por UART y active el pin DE (Data Enable) simultáneamente.
3. Verificar con un osciloscopio o conversor USB-RS485 en un PC si la trama llega correctamente a la bornera.

| Puerto | Pines STM32 (TX, RX, DE/RE) | Prueba de Transmisión (OK/Falla) | Prueba de Recepción (OK/Falla) |
|---|---|---|---|
| RS-485 A (U2) | | | |
| RS-485 B (U3) | | | |

## E. ALIMENTACIÓN Y REGULACIÓN (Prioridad 3)

| Verificación | Medición | Resultado |
|---|---|---|
| Voltaje en salida del LM7805 (U4) | Debe ser ~5.0V | |
| Voltaje en salida del LM1117-3.3 (U5) | Debe ser ~3.3V | |

---

## Al terminar

1. Registrar los resultados de A y B en el archivo `MAPEO_TARJETA_KICAD.md` para completar la tabla de asignación de pines.
2. Reportar cualquier fallo grave (ej. luces que parpadean al arrancar) para actualizar el PCB antes de la fabricación masiva.
