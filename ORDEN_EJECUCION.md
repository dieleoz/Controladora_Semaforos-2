# Orden Estricto de Ejecución y Pruebas (Protocolo)

Para evitar daños en el hardware y garantizar que las soluciones funcionen en el terreno, todas las optimizaciones (`OPT`) y medidas de seguridad (`SFTY`) deben atravesar obligatoriamente este embudo antes de darse por cerradas:

## Fase 1: Especificación y Cruce
1. **Definir el "Ground Truth":** Todo comportamiento esperado debe estar escrito primero en el `MANUAL_USUARIO.md`. Si no está en el manual, no se programa.
2. **Cruce de Diagnóstico:** Identificar la falla real (ej. el WhatsApp del operario) y documentarla en `OPTIMIZACIONES.md`. Ejemplo claro: Se descubrió que el bug de la interfaz (`OPT-1`) era causado en realidad por el colapso de la memoria por la falta del salto de línea `\n` en la radio (`OPT-5`).

## Fase 2: Simulación Matemática (Dry Run)
1. Antes de tocar el C++ de las placas, se construye un entorno controlado (mocks en Python).
2. Se inyecta la falla reportada (ej. `simulador_ruido.py` para inyectar basura sin `\n`).
3. Se diseña la solución arquitectónica y se prueba en el simulador.
4. **Criterio de Éxito:** ¿La simulación cumple con el `MANUAL_USUARIO.md`? Si la respuesta es Sí, se autoriza pasar a la Fase 3.

## Fase 3: Edición de Firmware
1. **Snapshot/Commit:** Asegurar que el código actual esté guardado en Git (Repositorio `2semaforos_3estados`).
2. **Flasheo de Solución:** Traducir la lógica de la simulación ganadora al C++ de las placas (`coordinador.cpp`, `RepetidorB.ino`, etc.).

## Fase 4: Pruebas de Banco y Rollback
1. **Validación Física:** Compilar e instalar el código en la placa dentro del laboratorio.
2. **Verificación de Estados:** Comprobar que los LEDs y la LCD respondan a los tiempos exactos del Manual.
3. **Decisión de Rollback:** 
   - Si la placa tiene un comportamiento anómalo (ej. se congela o las luces no dan los tiempos), **NO se improvisa**. 
   - Se ejecuta inmediatamente `git checkout` o `git revert` para regresar al estado anterior estable.
   - Se regresa a la Fase 2 para corregir el modelo matemático.
