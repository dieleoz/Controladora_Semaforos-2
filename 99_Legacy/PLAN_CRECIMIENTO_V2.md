# 🧠 Manual del Diseñador Funcional (Plan de Crecimiento V2)

Este documento contiene las preguntas arquitectónicas que el **Dueño de Producto (Funcional)** debe responder antes de que el equipo de desarrollo comience a programar la Fase 2 (Visión Artificial y Control Remoto).

La Fase 1 (Hardware actual) ya está asegurada con el *Safety Case* (tiempos All-Red y secuencias normativas). Ahora debemos planear el crecimiento.

---

## 1. Integración de la Cámara AI (YOLO)

El script actual (`object_detection_tracking.py`) procesa video y detecta autos usando YOLOv8. Sin embargo, requiere una computadora para correr (no puede correr en el microcontrolador STM32 ni en el ESP32).

**Preguntas para el Funcional:**
- [ ] **Hardware de Procesamiento:** ¿Dónde se va a ejecutar este script de Python en terreno? ¿Se instalará una Raspberry Pi 4 / 5 o una NVIDIA Jetson en la caja del semáforo Maestro?
- [ ] **Enlace Cámara-STM32:** ¿Cómo se enviará la orden "Hay 10 autos esperando" desde la Raspberry Pi al STM32 Maestro? ¿Usaremos un cable USB-Serial (UART) directo entre la Raspberry y el STM32?
- [ ] **Resiliencia Climatológica (Fallback):** Si la lente de la cámara se cubre de lodo o la niebla ciega al modelo YOLO, ¿cuál es el tiempo máximo que el sistema esperará antes de forzar un cambio de luz por seguridad? (Recomendación: 3 a 5 minutos).

## 2. Acceso Remoto y Configuración (Ergonomía de Hardware)

Actualmente, el LCD y los botones físicos están alojados dentro de la placa, instalada a 6 metros de altura.

**Preguntas para el Funcional:**
- [ ] **Configuración en Piso:** Para evitar que un operario deba subir 6 metros con una escalera en plena obra, ¿se autoriza bajar un cable UTP desde la placa (conectado a los optoacopladores) hasta una botonera física a nivel de suelo?
- [ ] **Consola Inalámbrica (Futuro):** Si se integra la Raspberry Pi para la cámara AI, ¿podríamos usar el WiFi/Bluetooth de esa Raspberry para crear una "Web App" local? Así el operario podría cambiar los tiempos de despeje desde su celular estando parado bajo el semáforo.

## 3. Topología de Red y Modo Degradado (Aislado)

Si el proyecto se despliega en montañas (línea de vista bloqueada) y los radios LoRa pierden conexión permanentemente:

**Preguntas para el Funcional:**
- [ ] **Esclavo Ciego:** ¿Se aprueba la modificación de hardware sugerida en el WhatsApp de agregar una segunda pantalla LCD + Botonera + Módulo Reloj (RTC DS3231) a la placa Esclava? 
- [ ] Esto permitiría que dos operarios se paren en cada extremo del túnel, sincronicen los relojes manualmente a la misma hora exacta, y dejen los semáforos operando solos (ej: minuto par pasa el Maestro, minuto impar pasa el Esclavo).
