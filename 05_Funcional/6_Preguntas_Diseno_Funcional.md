# PREGUNTAS Y VALIDACIÓN DE DISEÑO FUNCIONAL (V9.0)

Este documento recopila los puntos clave de diseño funcional y operativo acordados y cerrados con el equipo técnico y de concesión vial.

---

## 1. Integración de Cámaras de Visión Artificial (Hikvision AcuSense G2 - APROBADO)

1. **Topología de Cámaras (CERRADO):** Se aprueba el sistema autónomo de **4 Cámaras Hikvision AcuSense G2** (2 en Maestro y 2 en Esclavo) mediante **contacto seco directo (`1A`/`1B`)** a las entradas optoacopladas `PB9` y `PB13` de las tarjetas STM32.
2. **Cero Computadores Edge Externos (CERRADO):** Se descarta el uso de Raspberry Pi, Jetson Nano, conversores USB y switches Ethernet. La detección de vehículos corre directamente dentro de la cámara (DSP AcuSense) y entra por pulsos limpios de hardware a la placa.
3. **Respuesta Adaptativa (CERRADO):** Cada pulso de detección en `PB9` solicita verde para el sentido correspondiente. Se garantiza un piso inquebrantable de **15 segundos mínimos de All-Red (Todo-Rojo)** antes de conmutar de un carril al opuesto.
4. **Cobertura Simétrica (CERRADO):** Ambas puntas disponen de detección simétrica (Cámara 1 y 2 en Maestro; Cámara 3 y 4 en Esclavo).

---

## 2. Parámetros de Operación Vial y Módulo Bluetooth (APROBADO)

1. **Tiempo de Despeje (All-Red):** El firmware admite de **5 a 999 segundos** (hasta 16.6 min); el piso de 5s en menú y 15s en auto-recuperación es inquebrantable por seguridad.
2. **Módulo Bluetooth en Maestro y Esclavo (CERRADO - Estándar Baliza):** Se conecta a `USART1` (`PA9` TX, `PA10` RX) con alimentación 5V/GND. Permite telemetría periódica `$STATUS`, Caja Negra de caídas de radio `$ALARM` con timestamp del RTC y control de Modo Manual y Rojo Total desde el suelo sin necesidad de subir a los postes con escaleras (resolviendo el punto N-19 en el Esclavo).
3. **Mando a Distancia Anti-Colisión (CERRADO - Resolución N-53):** Secuencias con alternancia (`A·B·A` Auto, `B·A·B` Ámbar, `B·A·B·A` Manual, `A·B·A·B` Degradado) e inhibición total de secuencias durante la navegación y edición de parámetros en la pantalla LCD.

---

## 2.1 Modo Degradado — decisiones abiertas (SFTY-21)

Todas estas cifras están **construidas en firmware** y validadas en simulador, pero **no
contrastadas contra la operación real de obra**. Se listan para que el funcional las confirme o las
corrija **antes** de la primera puesta en campo.

1. **Ciclo degradado fijo de verde 30 s / todo-rojo 30 s (ciclo de 120 s).** No hereda el verde
   configurado en Modo Automático, deliberadamente: un verde de 2 min con todo-rojo de 30 s daría un
   ciclo de 5 minutos y nadie espera eso en un paso alternado sin invadir. **¿Son 30/30 aceptables
   para el tramo de obra real, o hace falta ajustarlos por longitud?** *(Bajar el todo-rojo acorta el
   margen de deriva y obligaría a bajar también el límite de 48 h.)*
2. **Límite duro de 48 h sin resincronizar ⇒ caída automática a ámbar.** Sale de un margen teórico de
   ~3,5 días con factor de seguridad 2. **¿Se acepta que el equipo se rinda solo a las 48 h**, o la
   obra necesita una autonomía distinta? *Recordar el coste: una semana obligaría a un todo-rojo de
   ~90 s.*
3. **Antigüedad máxima de la sincronización para poder entrar: 2 h.** Tiene una consecuencia
   operativa poco intuitiva: **si el radio lleva medio día muerto, ya no se puede entrar al
   Degradado.** La ventana para activarlo era mientras el enlace todavía respiraba. **¿Se acepta, o
   hace falta un procedimiento de contingencia para ese caso?**
4. **Tolerancia de desfase de ±3 s.** Diez veces por debajo del todo-rojo de 30 s. ¿Se valida?
5. **Riesgos residuales aceptados el 01/08/2026** — se dan por conocidos, pero conviene que consten
   firmados en el acta:
   - El verde se da **sin confirmación del otro extremo**. Con el radio muerto es inevitable.
   - **Salida asimétrica**: una punta en ámbar contra una punta en verde. Sin solución técnica sin
     radio; la mitigación es **procedimental** (verificación visual de ambas puntas, también al
     salir).
6. **Códigos del mando de relés.** Al comprar el receptor del Esclavo hay que **exigir código
   independiente del mando del Maestro**. Con ambos a menos de una cuadra y el mismo código, una sola
   secuencia metería las dos unidades en Degradado a la vez, **saltándose la verificación por separado
   que justifica todo el diseño**. ¿Está esto en la especificación de compra?

---

## 3. Decisiones pendientes abiertas por la auditoría V8.0

1. **Operación intermitente por bajo flujo (`1_Manual_Usuario.md §2`):** el Manual de Señalización 2024 la contempla (ámbar en vía principal, rojo en secundaria, tras 4 h con flujo ≤50%), pero **no está implementada**. ¿Se incorpora al alcance, se difiere, o se retira de la especificación?
2. **Arranque en oscuro:** al encender, el Maestro deja las luces apagadas ~2 s durante la pantalla de bienvenida. ¿Se acepta, o debe arrancar directamente en Rojo?
3. **Watchdog en el Repetidor ESP32:** no está implementado. Maestro y Esclavo lo detectan por fail-safe a los 25 s (SFTY-6), pero el repetidor requiere corte de energía para recuperarse de un cuelgue. ¿Se añade?
4. **Velocidad aérea:** se adopta `2.4 kbps` (antes `0.3 kbps`) con un coste aproximado de 6 dB de sensibilidad. ¿Se valida contra la distancia máxima real de obra prevista?

---

## 4. Decisiones abiertas por la revisión del 01/08/2026

1. **Persistencia del estado del Modo Degradado (N-20).** Hoy vive en RAM: un microcorte deja esa
   punta en ámbar mientras la otra sigue dando verde por reloj — **que es exactamente el riesgo
   residual nº 2**. El módulo que lo guardaría en los registros de respaldo, alimentados por la misma
   pila ya instalada, **está escrito pero sin conectar**. ¿Se prioriza antes de ir a campo?
   ⚠️ *No confundir con autorización por adelantado ("si pierdes el radio X minutos, entra"): eso es
   entrada automática con pasos extra y sigue descartado.*
2. **Configuración del ciclo recibida pero no consumida (N-18).** El Esclavo **almacena** la
   configuración que le llega por radio, pero el cálculo del ciclo degradado todavía usa los 30/30
   fijos compilados. Mientras las dos puntas lleven la misma versión de firmware coinciden — **pero
   nadie vigila que la lleven**. ¿Se cierra esto antes de campo, o basta con la comprobación de
   versión en el acta de pruebas?
3. **Margen de flash del Maestro (N-21).** Tras el Modo Degradado va al **80,2 %**. Lo pendiente
   —persistencia conectada, pantalla informativa en ámbar y modo nocturno— lo dejaría sobre el
   **85 %**: ajustado pero viable. **Una función grande más ya no cabría.** ¿Se acota el alcance
   funcional, o se evalúa el cambio de microcontrolador con su verificación chip a chip?
4. **Prueba de banco del reloj (N-15 / N-17).** Sigue sin hacerse: ni contraste contra hora patrón, ni
   conservación tras corte de energía, ni arranque con el cristal desconectado. **Todo lo que depende
   de la hora va hoy sobre un supuesto.** ¿Se agenda antes de la certificación?
