# 📋 PROTOCOLO DE AUDITORÍA FUNCIONAL Y ACTA DE CERTIFICACIÓN EN CAMPO

**Versión del protocolo:** V8.7 · **Fecha de emisión:** 1 de Agosto de 2026
**Documento para remisión al Ingeniero Funcional / Auditor de Tránsito**
**Ubicación:** `05_Funcional/3_Protocolo_Pruebas_Rigurosas.md`
**Entorno Auditable:** Ecosistema Semafórico Móvil (Maestro STM32, Esclavo STM32, Repetidor ESP32)
**Normativa Aplicable:** Resolución 2024 del Ministerio de Transporte de Colombia

## 📌 QUÉ AÑADE ESTA REVISIÓN DEL PROTOCOLO

Las Secciones 1 a 6 **no cambiaron de contenido** — siguen certificando lo mismo que la V8.0. Lo
nuevo son cuatro bloques que la versión anterior no cubría en absoluto:

| Sección | Qué certifica | Por qué es nueva |
|---|---|---|
| **5.6** *(modificada)* | El menú ahora tiene **dos niveles** | La lista plana de 4 opciones ya no existe |
| **7** | **AJUSTAR HORA** y **sincronización horaria por radio** | El equipo no tenía reloj |
| **8** | **Secuencias del mando de 4 relés** | No existían |
| **9** | **MODO DEGRADADO** | No existía |
| **10** | Interfaz propia del **Esclavo** | El Esclavo no tenía pantalla |

> **Las Secciones 7 a 10 solo aplican si el firmware cargado es V8.6 o posterior.** Si se está
> certificando la V8.4 que corre hoy en campo, **márquelas como NO APLICA** y no las cuente en el
> total: esas funciones no existen en ese binario.

---

## 🆔 IDENTIFICACIÓN DEL EQUIPO BAJO PRUEBA

> Diligenciar **antes** de empezar. Sin estos datos el acta no es trazable: rondas anteriores se
> ejecutaron con firmware y configuración de radio distintos, y sus resultados no son comparables.

```text
Fecha y hora de inicio de pruebas: ______________________________________
Versión de firmware MAESTRO: __________  Commit: __________  Fecha binario: ________
Versión de firmware ESCLAVO: __________  Commit: __________  Fecha binario: ________
   -> ¿Son la MISMA version en ambas puntas?   [ ] SI   [ ] NO  <- si es NO, DETENER
Nº de serie Maestro: ______________  Nº de serie Esclavo: ______________
Nº de serie Repetidor ESP32: ______________
Air Data Rate configurado en las radios: __________ kbps
Modo de enlace probado:  [ ] Directo (2 radios)   [ ] Repetidor (4 radios)
Receptor de mando de reles instalado:  Maestro [ ] SI [ ] NO   Esclavo [ ] SI [ ] NO
Pila CR2032 instalada (R5 retirado):   Maestro [ ] SI [ ] NO   Esclavo [ ] SI [ ] NO
Auditor responsable: ____________________________________________________
```

> ⚠️ **Si las dos tarjetas no llevan la misma versión de firmware, deténgase aquí.** El cálculo de la
> fase del Modo Degradado tiene que dar **exactamente el mismo resultado** en las dos puntas. Dos
> versiones distintas pueden calcular fases distintas **sin ningún aviso en pantalla**: cada unidad
> mostraría que todo va bien mientras las luces se solapan.

---

## 🚨 PASO 0 — OBLIGATORIO ANTES DE CUALQUIER PRUEBA

- [ ] **0.1 Reconfigurar las radios a `2.4 kbps` de Air Data Rate**, siguiendo `4_Manual_Configuracion_Radios.md`.
  - Aplica a **las 4 radios** (Maestro, Esclavo, Repetidor-Entrada B1, Repetidor-Salida B2).
  - Todas deben quedar con **el mismo** valor, o no enlazarán entre sí.
  - **Si se omite, las Secciones 3, 4, 5 y 6 fallarán** reproduciendo el fallo de comunicación al paso
    de ciclo ya reportado. No es un ajuste opcional ni cosmético.
- [ ] **0.2 Cargar el firmware de esta entrega** en las tres tarjetas (Maestro, Esclavo, Repetidor).
- [ ] **0.3 Verificar canales:** Directo → ambas en canal `0`. Repetidor → Maestro y B1 en canal `0`; B2 y Esclavo en canal `10`.
- [ ] **0.4 Confirmar que las DOS tarjetas llevan la MISMA versión de firmware.** Anotarla en la
  identificación de arriba. Si difieren, **no continúe**: el cálculo de la fase del Modo Degradado
  tiene que dar exactamente el mismo resultado en las dos puntas, y dos versiones distintas pueden
  calcular fases distintas **sin ningún aviso en pantalla**.
- [ ] **0.5 Verificar que la pila `CR2032` está instalada y `R5` retirado** en ambas tarjetas *(solo si
  va a ejecutar las Secciones 7 a 10)*. Con `R5` puesto, la pila queda en paralelo con los 3,3 V:
  **una pila no recargable en esa situación se calienta, se hincha y puede reventar.** Ver
  `2_Manual_Hardware_y_Pruebas.md §5`.

> ⚠️ Las pruebas de las rondas del 30 y 31 de julio se ejecutaron con radios a `0.3 kbps` y firmware
> anterior a esta corrección. **Sus resultados no son comparables con esta ronda.**

---

## ℹ️ COMPORTAMIENTOS ESPERADOS — NO son fallas

Léase antes de empezar. Estos comportamientos son **de diseño**; marcarlos como falla invalidaría el acta.

| Observación | Explicación |
|---|---|
| Al encender el Maestro, **~2 segundos con todas las luces apagadas** | Pantalla de bienvenida (`delay(2000)`). Tras ese lapso el Maestro fija Rojo. *Pendiente de mejora: lo deseable sería arrancar en Rojo.* |
| En Modo Manual, el arranque tarda **5s de Rojo + 4s de Amarillo** antes del Verde | El despeje All-Red mínimo subió de 3s a **5s** por seguridad vial (no configurable por debajo). |
| El Amarillo previo al Verde dura **exactamente 4.0s** | Res. 2024. El paso de Verde a Rojo es **directo, 0s de aviso**. |
| Tras 5 reintentos fallidos, el Maestro **no insiste**: pasa a Ámbar | El fallback de seguridad de 12.0s tiene prioridad sobre los reintentos. Es intencional. |
| En Modo Inteligente, durante el **primer minuto** tras arrancar la pantalla puede mostrar `IA: OK` aunque la cámara no esté conectada | Defecto conocido **N-5**, pendiente. No afecta la seguridad vial. Anotarlo, no bloquear por ello. |
| El **Menú Principal tiene 4 opciones**, y la cuarta es `CONFIGURACION` — no `PRUEBA ALCANCE` | El menú pasó a **dos niveles**. `PRUEBA ALCANCE`, `AJUSTAR HORA` y `MODO DEGRADADO` cuelgan de `CONFIGURACION`. |
| El **Esclavo tiene pantalla y menú propios**, con solo 2 opciones y **sin ajuste de hora** | De diseño. La hora llega por radio; ajustarla a mano en el Esclavo reintroduciría el desfase que ese mecanismo elimina. |
| Al perder el radio, el sistema **sigue yendo a ámbar intermitente** y **no entra solo** en Modo Degradado | De diseño, y es la regla central de SFTY-21. El Degradado **nunca** es automático. |
| Con el **menú abierto**, el mando de relés **no responde a las secuencias** | Requisito de seguridad, no un fallo. Ver Sección 8. |

## 🚫 FUNCIONES NO IMPLEMENTADAS — no deben probarse ni certificarse

| Función | Estado |
|---|---|
| **Operación intermitente por bajo flujo** (ámbar en vía principal / rojo en secundaria tras 4h con flujo ≤50%), descrita en `1_Manual_Usuario.md §2` | **NO IMPLEMENTADA.** Aplazada por decisión del cliente (31/07): el horario no es igual en todas las obras. Fuera del alcance. |
| **Operación intermitente nocturna por horario** (SFTY-20) | Especificada, **sin construir.** Fuera del alcance. |
| **Watchdog en el Repetidor ESP32** | No implementado. Maestro y Esclavo sí lo tienen (IWDG a 4.0s). |
| **Receptor del mando de relés en el ESCLAVO** (N-19) | **No instalado.** La Sección 8 se ejecuta **solo sobre el Maestro**. En el Esclavo hay que subir al gabinete. |
| **Persistencia del estado del Modo Degradado a un corte de energía** (N-20) | Módulo escrito, **sin conectar.** Un microcorte deja esa punta en ámbar mientras la otra sigue en verde — ver prueba 9.8. |
| **Consumo de la configuración del ciclo recibida por radio** (N-18) | Se **almacena** pero **no se usa** todavía en el cálculo. Ambas puntas emplean los 30/30 s fijos compilados. |

> ## ⚠️ SOBRE EL ALCANCE DE LAS SECCIONES 7 A 10
>
> Todo lo que certifican **está construido y validado en simulador** (20/20 funcional, 10/10
> repetidor, 83/83 de pantalla), pero **nunca se ha ejercitado sobre hardware real**. Esta ronda de
> pruebas **es** su primera verificación física.
>
> Trátelas como pruebas de puesta en marcha, no como confirmación de algo ya conocido: **espere
> encontrar fallos** y anótelos con el detalle que pide la tabla final.

---

## 📝 CÓMO REGISTRAR CADA PRUEBA

Marque una casilla por prueba y anote lo observado. Si algo no cumple, **describa qué pasó y en qué
segundo**; ese dato es lo que permite diagnosticar.

---

## 📑 SECCIÓN 1 — MENÚ PRINCIPAL E INDEPENDENCIA DE RADIO (SFTY-12)

**1.1 Rojo Fijo en Menú con enlace activo**
- *Acción:* Encender Maestro y Esclavo con comunicación activa, sin seleccionar modo.
- *Esperado:* Tras los ~2s de bienvenida, **ambos en 🔴 ROJO FIJO continuo**.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**1.2 Navegación LCD fluida sin radio**
- *Acción:* Con el Maestro en el Menú, apagar la radio del Esclavo. Navegar por las pantallas de configuración.
- *Esperado:* La LCD ST7920 navega con normalidad, **sin congelarse ni trabarse** en ningún momento.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**1.3 Indicación de orfandad en el Menú (regresión reportada)**
- *Acción:* Continuando desde 1.2, esperar sin tocar botones.
- *Esperado:* A los **12.0 s** sin comunicación, **el Maestro pasa a 🟡 AMARILLO INTERMITENTE (~1 Hz)** y el Esclavo también. **Ambos deben parpadear**, no quedarse en Rojo ni apagados.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > *Corresponde a los puntos 3 y 4 de la ronda del 31/07, donde el Maestro se quedaba en Rojo.*

**1.4 Arranque de modo sin ciclos fantasma**
- *Acción:* Ajustar tiempos y presionar Botón 3 (OK) para iniciar un modo.
- *Esperado:* Entra directo al Despeje All-Red, **sin parpadeos ni saltos de estado extraños**.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

---

## 📑 SECCIÓN 2 — PÉRDIDA DE COMUNICACIÓN Y SELF-HEALING (SFTY-6 / SFTY-9)

**2.1 Apagado del Esclavo**
- *Acción:* Con el sistema en Modo Automático, apagar la radio o la batería del Esclavo. Cronometrar.
- *Esperado:* A los **12.0 s** el Maestro pasa a **🟡 AMARILLO INTERMITENTE**.
- Segundos medidos hasta el ámbar: ________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**2.2 Apagado del Maestro**
- *Acción:* Con el sistema corriendo, apagar la radio o la batería del Maestro. Cronometrar.
- *Esperado:* A los **12.0 s** el Esclavo pasa a **🟡 AMARILLO INTERMITENTE**.
- Segundos medidos hasta el ámbar: ________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**2.3 Esclavo en Verde cuando cae el enlace (prueba de seguridad crítica)**
- *Acción:* Esperar a que el **Esclavo esté en Verde**. Entonces desconectar la antena del Maestro.
- *Esperado:* El Esclavo **NO puede quedarse en Verde indefinidamente**. Debe pasar a 🔴 Rojo o a 🟡 Ámbar intermitente en un máximo de **25 s** (SFTY-6).
- Estado final del Esclavo: ____________  Segundos: ________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > *Prueba nueva. Verifica la corrección H-1: un fallo de enlace en un solo sentido dejaba al Esclavo en Verde permanente mientras el Maestro parpadeaba en ámbar.*

**2.4 Self-Healing sin reinicio manual**
- *Acción:* Con ambos en ámbar por falta de señal, reconectar la radio o la antena. **No tocar la alimentación de las tarjetas.**
- *Esperado:* Reconectan solas, **sin reiniciar ni apagar ninguna tarjeta**.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**2.5 Despeje All-Red tras la reconexión**
- *Acción:* Observar las luces inmediatamente después de la reconexión automática.
- *Esperado:* Primero **15.0 s de 🔴 ROJO FIJO en ambos** para vaciar la vía, y solo después se abre un carril.
- Segundos de All-Red medidos: ________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

---

## 📑 SECCIÓN 3 — MODO AUTOMÁTICO (1 min Rojo / 1 min Verde / 15 s Despeje)

> Configuración de la prueba: Rojo **1 min**, Verde **1 min**, Despeje **15 s**.
> Mínimos que el menú ya no permite bajar: fases de **1 minuto**, despeje de **5 segundos**.

**3.1 Secuencia lumínica normativa (Res. 2024)**
- *Esperado:* Rojo → **Amarillo Fijo 4.0 s** → Verde. Y de Verde a Rojo, **directo, sin amarillo**.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**3.2 Arranque y Turno 1 (Verde Maestro)**
- *Esperado:* All-Red 15 s → Maestro 4 s Amarillo → Maestro Verde 60 s, con Esclavo en Rojo.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**3.3 Turno 2 (Verde Esclavo) — PUNTO CRÍTICO DE LA RONDA ANTERIOR**
- *Acción:* Observar con atención el momento exacto en que el **Esclavo pasa a Amarillo y luego a Verde**.
- *Esperado:* Maestro a Rojo → All-Red 15 s → Esclavo 4 s Amarillo → Esclavo Verde 60 s. **Sin fallo de comunicación y sin que el ciclo se reinicie.**
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > *Aquí fallaba en todas las rondas anteriores. Es la prueba que valida la corrección N-1 (tasa aérea + tiempos de espera). Si esta prueba falla, **verifique primero el Paso 0.1** antes de reportar.*

**3.4 Retorno al Maestro sin falso fallo**
- *Esperado:* Terminado el minuto de Verde del Esclavo y los 15 s de despeje, el Maestro toma el Verde limpiamente.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**3.5 Estabilidad en ciclos sucesivos**
- *Acción:* Dejar correr **al menos 5 ciclos completos** (≈ 12 minutos) sin intervenir.
- *Esperado:* Ningún fallo de comunicación, ningún reinicio de ciclo.
- Ciclos completados sin fallo: ________ de 5
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

---

## 📑 SECCIÓN 4 — MODO INTELIGENTE AI (Cámara YOLOv8)

**4.1 Interfaz dedicada**
- *Esperado:* La pantalla muestra `MODO: INTELIGENTE AI` y `IA: OK (Autos: X)` o `IA: Standby (Fallback)`.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > Recuerde el aviso **N-5**: durante el primer minuto tras arrancar puede mostrar `IA: OK` sin cámara.

**4.2 Cede de paso adaptativo**
- *Acción:* Inyectar la trama UART `AI_CARS:X` para simular vehículos.
- *Esperado:* Con el carril en Verde y **0 autos**, adelanta el cambio a partir de los **20 s**. Con autos esperando en Rojo, adelanta el cambio a partir de los **60 s** (mitad del máximo de verde de 2 min).
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**4.3 Fallback por pérdida de cámara**
- *Acción:* Desconectar la cámara / dejar de enviar `AI_CARS`.
- *Esperado:* Transcurridos **60 s** sin datos, la pantalla pasa a `IA: Standby` y el sistema se comporta como Modo Automático con el máximo de verde.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**4.4 Ciclo completo sin caída (punto crítico)**
- *Acción:* Dejar correr un ciclo completo, observando el paso del Esclavo a Verde.
- *Esperado:* Igual que 3.3 — **sin caída de comunicación**.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

---

## 📑 SECCIÓN 5 — MODO MANUAL (Botonera Física)

**5.1 Primer cambio de carril**
- *Acción:* En Modo Manual, presionar **una vez** el Botón 1.
- *Esperado:* Cambio de vía respetando el All-Red configurado (mínimo 5 s) y los 4 s de Amarillo.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**5.2 Cambios sucesivos — PUNTO CRÍTICO DE LA RONDA ANTERIOR**
- *Acción:* Presionar Botón 1 o 2 una **segunda, tercera y cuarta vez**, dejando completar cada cambio.
- *Esperado:* La comunicación se mantiene estable en **todos** los cambios. Antes fallaba a partir del segundo.
- Nº de cambios consecutivos logrados sin fallo: ________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**5.2-bis MEDICIÓN DE CALIDAD DE ENLACE** 📶 *(ya no requiere cronómetro: el equipo lo mide solo)*
- *Acción:* Entrar en **Menú → PRUEBA ALCANCE** y esperar unos 15 segundos a que se estabilice.
- *Anotar lo que muestra la pantalla:*

```text
Enlace DIRECTO (2 radios):     Calidad ______ %     Respuesta ______ ms
Enlace con REPETIDOR (4 radios): Calidad ______ %   Respuesta ______ ms
```

- Resultado: `[ ] MEDIDO  [ ] NO SE PUDO MEDIR` — Observación: ________________________________
- > *Referencia:* se espera **100% de calidad** y **menos de 1000 ms** de respuesta. Si la calidad no llega al 100% o la respuesta pasa de 3000 ms, avise antes de continuar: indicaría que alguna radio no quedó bien configurada.

**5.3 Botón 3 — Rojo total indefinido**
- *Acción:* Presionar Botón 3 (OK) durante el Modo Manual, **en cualquier estado en que estén las luces**.
- *Esperado:* Ambos pasan **de inmediato a 🔴 ROJO FIJO** y **se mantienen así de forma indefinida**. No deben conmutar a Amarillo ni a Verde por sí solos pasados unos segundos.
- Tiempo observado en Rojo sin conmutar: ________ (esperar al menos 60 s)
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**5.4 Reanudación desde Botón 3**
- *Acción:* Estando en Rojo por Botón 3, presionar Botón 1 o Botón 2.
- *Esperado:* Reanuda abriendo el carril correspondiente, respetando el despeje.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**5.5 Botón 4 — Retorno a Menú**
- *Esperado:* Vuelve al Menú Principal y ambos semáforos quedan en Rojo Fijo (con enlace activo).
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**5.6 Menú de DOS NIVELES y pantalla de alcance** *(modificado — el menú cambió)*
- *Acción:* En el Menú Principal, recorrer la lista con Botón 1/2.
- *Esperado:* Se ven **4 opciones completas y legibles**: `MANUAL`, `AUTOMATICO`, `INTELIGENTE`, **`CONFIGURACION`**. Ninguna cortada ni fuera de pantalla. **`PRUEBA ALCANCE` ya NO está en este nivel.**
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- *Acción:* Entrar en `CONFIGURACION` con Botón 3 y recorrer la lista.
- *Esperado:* **3 opciones legibles**: `PRUEBA ALCANCE`, `AJUSTAR HORA`, `MODO DEGRADADO`. **Ambos semáforos siguen en 🔴 Rojo Fijo** (o ámbar sin enlace): **entrar al submenú no arranca ningún ciclo**.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- *Acción:* Pulsar Botón 4 desde el submenú.
- *Esperado:* Vuelve al **Menú Principal** con el cursor **sobre `CONFIGURACION`**. **No arranca ningún modo.**
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- *Acción:* Entrar en `PRUEBA ALCANCE` con Botón 3.
- *Esperado:* Muestra calidad en %, barra gráfica, tiempo de respuesta y fallos. **Ambos semáforos en 🔴 Rojo Fijo**, sin arrancar ciclos. Sale con Botón 4 (vuelve a `CONFIGURACION`).
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > *Por qué se comprueba el layout:* la librería de pantalla **recorta fuera de los 128×64 en silencio, sin error**. Una opción que no quepa **no da fallo: simplemente no se dibuja, pero el cursor sí llega hasta ella.** El operario acabaría seleccionando una opción invisible. Ya ocurrió una vez con la cuarta opción del menú plano.

**5.7 Diagnóstico de línea en pantalla** *(nuevo)*
- *Acción:* En `PRUEBA ALCANCE`, observar la **fila inferior**. Luego apagar la radio del Esclavo y volver a observarla.
- *Esperado:* Con enlace sano muestra `RX <bytes>  <n> tr`. Al apagar el Esclavo pasa a `RX 0 - nada llega` en unos segundos.
- Lo observado con enlace: ______________  Tras apagar el Esclavo: ______________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > *Para qué sirve:* si alguna vez aparece `RX <n> - BASURA` significa que entran datos pero ninguno válido — problema de **cableado, línea RS485 suelta o radio transmitiendo ruido**, no de alcance. Evita desplazamientos al poste con instrumentos.

---

## 📑 SECCIÓN 6 — REPETIDOR ESP32 (topología de 4 radios)

> Requisito previo: Paso 0.1 y 0.3 completados. **La ronda anterior falló íntegramente esta sección**
> por saturación del canal a 0.3 kbps.

**6.0 DIAGNÓSTICO DE CADENA POR LEDs** 🔦 *(hacer esto PRIMERO si el repetidor no enlaza)*

### Cómo se observa

**El Maestro transmite cada 3 segundos, aunque el enlace esté caído.** Estando en fallo sigue
emitiendo una orden de Rojo con esa misma cadencia. Ese pulso periódico es el **metrónomo** que
permite rastrear la cadena precisamente cuando no hay comunicación.

**Qué mirar en cada radio:**
- El LED de **encendido (PWR)** queda **fijo** — ignórelo, no aporta nada.
- Los LEDs de **actividad** (rotulados `TXD` / `RXD` según el modelo) **destellan** cuando pasa
  información. **Son ésos.**
- **No hace falta saber cuál es cuál.** Lo único que se anota es: *¿esta radio destella cada
  3 segundos, sí o no?*

**Procedimiento (unos 5 minutos):**
1. Encienda Maestro, Esclavo y Repetidor. Deje el Maestro en el **Menú Principal**, sin arrancar
   ningún modo. Es indiferente que estén en ámbar por falta de enlace: el pulso sigue saliendo.
2. Párese frente a la **radio del Maestro** y confirme el destello cada 3 s. **Ése es el ritmo de
   referencia**; si aquí no destella nada, no siga: el problema está antes de la radio.
3. Vaya al **poste del repetidor** y observe **B1** durante 15 segundos (unos 5 pulsos).
4. Sin moverse, observe **B2** otros 15 segundos.
5. Vaya a la **radio del Esclavo** y observe otros 15 segundos.

> 💡 Como las radios están separadas, lo más práctico es **grabar 15 segundos de video con el celular
> frente a cada una** y comparar después. También sirve con dos personas y una llamada abierta.

> 💡 *Alternativa con evento provocado:* en **Modo Manual**, cada pulsación del Botón 1 genera una
> transmisión inmediata. Si le resulta difícil seguir el ritmo de 3 s, pulse el botón y observe si
> aparece **un** destello. Correlacionar una pulsación con un destello es más fácil que contar
> intervalos.

### Dónde se detiene la cadena

Anote **hasta dónde llega el parpadeo**. Donde se detenga, ahí está el corte.

```text
        MAESTRO          B1              [ESP32]         B2            ESCLAVO
        ┌──────┐        ┌──────┐                       ┌──────┐        ┌──────┐
        │ Radio│ ~~RF~~>│ Radio│ ──RS485──> ──RS485──> │ Radio│ ~~RF~~>│ Radio│
        └──────┘        └──────┘                       └──────┘        └──────┘
  Parpadea:  [ ]  ──────>  [ ]  ─────────────────────>   [ ]  ───────>   [ ]
```

Registre lo observado en cada radio:

```text
Radio del MAESTRO ..... [ ] destella c/3s   [ ] no destella
Radio B1 (repetidor) .. [ ] destella c/3s   [ ] no destella
Radio B2 (repetidor) .. [ ] destella c/3s   [ ] no destella
Radio del ESCLAVO ..... [ ] destella c/3s   [ ] no destella
```

Marque el **último** punto donde vio parpadeo:

- [ ] **A. No parpadea ni la radio del Maestro** → el problema está antes de la radio: cableado
  `A`/`B` a la tarjeta, o alimentación de la radio.
- [ ] **B. Parpadea el Maestro, pero B1 no recibe** → salto de aire Maestro→B1. Revisar que ambas
  estén en **canal `0`** y con la **misma velocidad aérea (2.4 kbps)**.
- [ ] **C. Parpadea B1, pero B2 no transmite** → el corte está **dentro del repetidor**: ESP32 sin
  alimentar, sin flashear, o falta el transceptor MAX3485 en la placa. Ver 6.0-bis.
- [ ] **D. Parpadea B2, pero el Esclavo no recibe** → **la causa más frecuente.** La radio del
  Esclavo quedó en canal `0` (170 MHz) cuando en modo repetidor debe estar en **canal `10`
  (172 MHz)**, igual que B2.
- [ ] **E. Parpadea toda la cadena pero el semáforo no responde** → el enlace físico está bien;
  el problema es de protocolo. Anotarlo y avisar a desarrollo.

Observación: ______________________________________________________________________

> **Recordatorio de canales en modo repetidor:** Maestro y B1 en canal `0` (170.0 MHz); B2 y
> **Esclavo** en canal `10` (172.0 MHz). En modo repetidor el Maestro y el Esclavo **NO** van en la
> misma frecuencia — ése es justamente el trabajo del puente.

**6.0-bis Firmware de diagnóstico del ESP32** *(solo si marcó la opción C)*

Si el corte parece estar dentro del repetidor, cargue el firmware de diagnóstico, que informa por USB
cuántos bytes llegan de cada lado:

```bash
pio run -e repetidor_diag -t upload     # cargar diagnóstico
pio device monitor -b 115200            # ver el informe (cada 2 s)

pio run -e repetidor -t upload          # volver a producción al terminar
```

Copie aquí dos o tres líneas del informe: ____________________________________________

**6.1 Enlace básico a través del repetidor**
- *Acción:* Interponer el puente ESP32 con las 4 radios ya reconfiguradas.
- *Esperado:* Maestro y Esclavo enlazan y se mantienen en Rojo Fijo en el Menú, sin caer a ámbar.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**6.2 Ausencia de parpadeo errático ("árbol de navidad")**
- *Esperado:* Las tramas viajan sin truncarse; el Esclavo responde de forma fluida, sin encendidos y apagados erráticos de las luces.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**6.3 Ciclo completo con repetidor**
- *Acción:* Ejecutar la Sección 3 completa (Modo Automático, ≥3 ciclos) con el repetidor interpuesto.
- *Esperado:* Mismo comportamiento que en enlace directo, **sin caídas al pasar el Esclavo a Verde**.
- Ciclos completados sin fallo: ________ de 3
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**6.4 Fail-safe del repetidor**
- *Acción:* Desconectar la alimentación del ESP32 durante la marcha; luego restablecerla.
- *Esperado:* A los **12.0 s**, Maestro y Esclavo pasan a 🟡 Ámbar intermitente. Al volver la energía, reconectan solos y ejecutan **15 s de All-Red** antes de reanudar.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

---

## 📑 SECCIÓN 7 — RELOJ, AJUSTAR HORA Y SINCRONIZACIÓN POR RADIO (SFTY-18 / SFTY-23)

> **Requisito previo de todo lo demás.** Sin reloj en hora y sincronizado, el Modo Degradado **no
> entra** — así está construido. Ejecute esta sección **antes** de la 8 y la 9.
>
> Material necesario: **una hora patrón** (celular con hora de red) y forma de **cortar la
> alimentación** de cada tarjeta.

**7.1 El reloj arranca declarándose NO fiable**
- *Acción:* Con una tarjeta **nunca puesta en hora** (o con la pila retirada), encender y entrar en `CONFIGURACION → AJUSTAR HORA`, y en el Esclavo a `ESTADO`.
- *Esperado:* El equipo indica que **la hora no es fiable**. **No debe mostrar una hora cualquiera como si fuera buena.**
- Lo que muestra: ______________________________________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > *Por qué es la primera prueba:* un reloj sin poner en hora que se cree válido **es peor que no tener reloj**. Habilitaría el Modo Degradado con las dos puntas desfasadas, y nadie lo vería.

**7.2 Edición dígito a dígito**
- *Acción:* En `AJUSTAR HORA`, recorrer los dígitos con Botón 3 y modificarlos con Botón 1 / Botón 2.
- *Esperado:* El **dígito activo se ve subrayado**. Botón 1 sube, Botón 2 baja, Botón 3 avanza al siguiente.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**7.3 Salir con Botón 4 NO altera la hora** 🔒
- *Acción:* Anotar la hora que marca el equipo. Entrar en `AJUSTAR HORA`, **cambiar varios dígitos** y salir con **Botón 4**. Volver a entrar.
- *Esperado:* La hora sigue siendo **la anotada al principio**. Los cambios se descartaron.
- Hora antes: ________  Hora después: ________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > *Para qué sirve:* se trabaja sobre una copia y solo se escribe al confirmar, de modo que **entrar por error no rompe la hora del equipo**.

**7.4 La pantalla NO arranca ciclos**
- *Acción:* Permanecer en `AJUSTAR HORA` un par de minutos observando las luces.
- *Esperado:* **🔴 Rojo Fijo en ambas puntas** con enlace (o ámbar sin él). **No arranca ningún ciclo.**
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**7.5 Contraste contra hora patrón** *(prueba de banco pendiente N-15)*
- *Acción:* Poner el Maestro en hora contra un celular con hora de red. Dejarlo **al menos 2 horas** encendido y volver a comparar.
- *Esperado:* La diferencia es de **pocos segundos**. Un cristal sin calibrar deriva ~30–50 ppm, es decir **menos de 1 s en 2 h**.
- Hora patrón: ________  Hora del equipo: ________  Diferencia: ________ s
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**7.6 La hora sobrevive al corte de energía** 🔋 *(valida la pila — prueba de banco pendiente N-15)*
- *Acción:* Con el equipo en hora, **desconectar la alimentación 10 minutos**. Volver a energizar.
- *Esperado:* Al arrancar, el equipo **conserva la hora** y la declara **fiable**. La diferencia contra la hora patrón debe ser de pocos segundos.
- Hora al volver: ________  Diferencia contra patrón: ________ s
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > **Si esta prueba falla, la pila no está haciendo su trabajo.** Revise que `R5` esté retirado y que el positivo esté soldado al pad de `VBAT` (ver `2_Manual_Hardware_y_Pruebas.md §5`). Sin esto, el Modo Degradado se rechaza tras cada apagón y el equipo es inutilizable en obra.
- *Repetir en el **Esclavo**:* Hora al volver: ________  Diferencia: ________ s — `[ ] CUMPLE  [ ] NO CUMPLE`

**7.7 Arranque con el cristal `Y2` desconectado** ⚠️ *(prueba de banco pendiente N-17)*
- *Acción:* Con la tarjeta **desenergizada**, desconectar el cristal `Y2` de 32.768 kHz. Energizar.
- *Esperado:* **El equipo bootea con normalidad**, las luces encienden, y la hora se declara **NO fiable**. **No debe quedarse colgado con las luces apagadas.**
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > *Por qué esta prueba existe:* algunos microcontroladores clonados traen mal calculado el condensador de carga y **el oscilador de 32.768 kHz no arranca**. Si el arranque del reloj colgara el equipo, la tarjeta quedaría muda y a oscuras. **Un semáforo no puede depender de un cristal de reloj para encender.**

**7.8 Poner en hora SINCRONIZA en el mismo gesto** 📡
- *Acción:* Con **enlace de radio activo**, confirmar la hora en el **Maestro**. Ir enseguida a la pantalla `ESTADO` del **Esclavo**.
- *Esperado:* El Esclavo muestra **la misma hora, con segundos**, y una **sincronización reciente**. **Nadie tocó el Esclavo.**
- Hora Maestro: ______:______:______   Hora Esclavo: ______:______:______
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**7.9 Medición del desfase — es un NÚMERO, no una impresión** 📏
- *Acción:* En el Maestro, leer el **desfase medido** contra el Esclavo y la marca de última sincronización.
- *Anotar:*

```text
Desfase Esclavo medido: ________ s     Ultima sincronizacion: ______:______
```

- *Esperado:* Desfase **dentro de ±3 s** tras una sincronización reciente.
- Resultado: `[ ] MEDIDO  [ ] NO SE PUDO MEDIR` — Observación: ________________________________
- > *Por qué se anota y no se mira:* confirmar la hora *"mirando las dos pantallas"* **no detecta hasta 59 s de desfase**, porque dos relojes separados 40 s muestran el mismo `14:32`. Cuarenta segundos es **más que el todo-rojo entero**. Este número es la única validación real, y por eso queda registrado en el acta.
- > *Sesgo conocido, no lo persiga:* la medida incluye el tiempo de aire más el retardo de cortesía del Esclavo (200 ms), así que trae **algunas décimas de segundo** de sesgo. Frente a un todo-rojo de 30 s es irrelevante.

**7.10 Resincronización periódica**
- *Acción:* Dejar el sistema con enlace activo **más de una hora** sin tocar nada. Volver a leer la marca de sincronización.
- *Esperado:* Se ha actualizado sola. El sistema resincroniza **cada hora mientras hay enlace**.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**7.11 El Esclavo NO ofrece ajuste de hora**
- *Acción:* Recorrer completo el menú del Esclavo.
- *Esperado:* **Solo 2 opciones**: `ESTADO` y `MODO DEGRADADO`. **No existe `AJUSTAR HORA`.**
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > *No es una carencia: es la regla.* La hora se cuadra **una sola vez en el Maestro** y viaja por radio. Ajustarla a mano en el Esclavo reintroduciría exactamente el desfase que ese mecanismo elimina.

---

## 📑 SECCIÓN 8 — MANDO DE 4 RELÉS Y SECUENCIAS (SFTY-21)

> **Solo aplica al MAESTRO.** El Esclavo **no tiene receptor de mando** (N-19).
>
> Estas pruebas se hacen **desde el piso**, sin ver la pantalla: es la condición real de uso. Necesita
> una segunda persona arriba solo para las pruebas que exigen leer la LCD.
>
> ⚠️ **Recuerde el ritmo del mando:** cada pulsación tarda **~2 s** en conmutar. No lo acelere.

**8.1 El mando navega el menú igual que la botonera**
- *Acción:* Con el equipo en el Menú Principal, accionar `A`, `B`, `C` y `D` uno a uno.
- *Esperado:* `A` sube el cursor, `B` lo baja, `C` selecciona, `D` sube un nivel — **una sola posición por pulsación**, sin saltos ni rebotes.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**8.2 `A · A · A` → AUTOMÁTICO, con 2 destellos rojos**
- *Acción:* Con el equipo **fuera del menú** (ciclando o en ámbar), accionar `A` tres veces en **menos de 12 s**.
- *Esperado:* **2 destellos ROJOS contables**, y luego el equipo intenta Modo Automático.
- Destellos contados: ________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**8.3 `B · B · B` → ÁMBAR, con 3 destellos rojos**
- *Acción:* Accionar `B` tres veces en **menos de 12 s**.
- *Esperado:* **3 destellos ROJOS** y el equipo pasa a 🟡 Ámbar intermitente.
- Destellos contados: ________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**8.4 `B · B · B` funciona DESDE CUALQUIER ESTADO** 🛟
- *Acción:* Repetir 8.3 desde **cada uno** de estos estados: Modo Manual, Modo Automático a mitad de verde, Modo Inteligente, y Modo Degradado activo.
- *Esperado:* En **todos** los casos va a ámbar, **sin condiciones ni rechazos**.
- Estados probados con éxito: ______ de 4
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > *Por qué se prueba desde todos:* es la regla que **impide que nadie quede atrapado** con un semáforo en un estado raro a 5 m de altura y sin escalera a mano. Si falla desde algún estado, es un hallazgo de seguridad.

**8.5 Los destellos son ROJOS, nunca verdes** 🔴
- *Acción:* Observar las tres confirmaciones **desde lejos**, como lo haría un conductor que se acerca.
- *Esperado:* **Solo destella el ROJO.** En ningún momento se enciende el verde ni los tres colores a la vez.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > *Por qué importa:* el operario cuenta destellos **sin ver la pantalla**. Si cuenta mal, el peor caso debe seguir siendo seguro — y **el rojo nunca significa "pase"**. Un destello verde podría ser interpretado por un conductor lejano como autorización de paso.

**8.6 CON EL MENÚ ABIERTO, LAS SECUENCIAS SE IGNORAN** 🔒 *(prueba de seguridad crítica)*
- *Acción:* Dejar el equipo **con el menú abierto**. Desde el piso, ejecutar `B·B·B`, luego `A·A·A`, luego `A·B·A·B`.
- *Esperado:* **Ninguna secuencia se reconoce.** No hay destellos, no cambia de modo, no entra en Degradado. El cursor simplemente se mueve.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > **Por qué es crítica, con el caso concreto:** con el menú abierto, una ráfaga accidental de pulsos podría llevar el cursor hasta `CONFIGURACION → AJUSTAR HORA` y, con unos pulsos más, **confirmar una hora cualquiera que el equipo daría por válida**. A partir de ahí el Modo Degradado entraría sobre una hora inventada, con las dos puntas desfasadas y nadie enterado. Es exactamente el fallo que el diseño del reloj existe para evitar. **Si esta prueba no cumple, es un hallazgo de seguridad vial: RECHAZAR.**

**8.7 Ráfagas accidentales no dejan el equipo en estado peligroso**
- *Acción:* Con el equipo **fuera del menú**, accionar `A` y `B` de forma desordenada durante un minuto (simulando un pulso espurio o un operario confundido).
- *Esperado:* El equipo termina en **Automático, en Ámbar, o donde estaba** — nunca en un estado peligroso, y **nunca en Degradado si los requisitos no se cumplen**.
- Estado final: ______________________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**8.8 `C` y `D` NO forman secuencias**
- *Acción:* Con el equipo fuera del menú, accionar `C·C·C` y luego `D·D·D`.
- *Esperado:* **Ninguna secuencia se dispara.** Solo se usan `A` y `B`.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

---

## 📑 SECCIÓN 9 — MODO DEGRADADO (SFTY-21)

> ## ⚠️ ANTES DE EMPEZAR ESTA SECCIÓN
>
> **1. Lea completo el `8_Procedimiento_Modo_Degradado.md`.** No ejecute estas pruebas sin conocer los
> riesgos residuales de su Sección 6.
>
> **2. Estas pruebas se hacen con el tramo CERRADO al tráfico.** Varias de ellas provocan
> deliberadamente el escenario de verde en una punta y ámbar en la otra.
>
> **3. La Sección 7 debe estar CUMPLE** — sin reloj en hora y sincronizado, el modo no entra.
>
> Material necesario: forma de **cortar el radio** (desconectar antena o apagar la radio) y **dos
> observadores**, uno en cada punta, con forma de comunicarse.

**9.1 Sin radio, el comportamiento por defecto SIGUE siendo el ámbar** ✅
- *Acción:* Con el sistema en Modo Automático, desconectar la antena del Maestro. Esperar **5 minutos sin tocar nada**.
- *Esperado:* A los **25 s** ambas puntas van a 🟡 Ámbar intermitente **y se quedan ahí**. **El equipo NO entra solo en Modo Degradado, ni a los 5 minutos ni nunca.**
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > *Es la prueba más importante de la sección.* Si el equipo entrara solo, estaría dando verde **sin poder saber si la otra punta sigue viva** — podría estar apagada, colgada o movida a otra obra. **Si esta prueba no cumple: RECHAZAR.**

**9.2 Rechazo con el reloj sin poner en hora**
- *Acción:* Con una unidad **sin hora fiable**, intentar entrar al Modo Degradado desde la pantalla.
- *Esperado:* **Lo rechaza**, y dice el motivo: `Falta: reloj sin poner en hora`.
- Mensaje mostrado: ______________________________________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**9.3 Rechazo sin sincronización previa por radio**
- *Acción:* Con la hora puesta pero **sin que haya habido nunca sincronización** con la otra punta, intentar entrar.
- *Esperado:* **Rechazado**, con motivo `Falta: nunca hubo sincronizacion RF`.
- Mensaje mostrado: ______________________________________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**9.4 Rechazo con sincronización caducada (más de 2 h)**
- *Acción:* Sincronizar, luego dejar el radio caído **más de 2 horas**. Intentar entrar.
- *Esperado:* **Rechazado**, con motivo `Falta: la ultima sync es muy vieja`.
- Horas transcurridas: ________  Mensaje: ______________________________________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > *Por qué no basta con medir el desfase:* la medida de desfase transporta **solo el segundo**, así que un desfase real de 45 s **se lee como −15 s**. Un desfase peligroso podría pasar por aceptable. Lo que cierra ese agujero es **la frescura**: tras una sincronización de hace una hora la deriva es de ~0,36 s y la medida no puede estar equivocada. **El desfase es una comprobación de cordura; la garantía es la sincronización reciente.**

**9.5 Rechazo desde el piso: ámbar rápido en vez de destellos**
- *Acción:* Con alguno de los requisitos sin cumplir, ejecutar `A·B·A·B` desde el mando.
- *Esperado:* **NO hay 4 destellos rojos.** Aparece un **ámbar rápido** que significa *rechazado*, y el equipo **no entra**.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > *Por qué se distingue así:* los destellos rojos significan **"hecho"**. Un rechazo no puede confirmarse con el mismo lenguaje que un éxito, o el operario se va creyendo que el modo quedó activo.

**9.6 Entrada correcta en las DOS puntas y verificación visual** 👁️
- *Acción:* Cumplidos los requisitos, cortar el radio y entrar al Degradado **en el Maestro y en el Esclavo** siguiendo el `8_Procedimiento_Modo_Degradado.md §3`.
- *Anotar cómo se activó cada punta:*

```text
MAESTRO: [ ] desde la pantalla   [ ] con A.B.A.B desde el piso  -> destellos contados: ____
ESCLAVO: [ ] desde la pantalla (subiendo al gabinete)   [ ] N/A
   Tiempo que tomó subir al gabinete del Esclavo: ________ min
```

- *Esperado:* **Doble confirmación** en cada punta (`Botón 3` → `CONFIRMAR ENTRADA?` → `Botón 3`).
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**9.7 VERIFICACIÓN VISUAL DEL CICLO — al menos 2 ciclos completos** 👁️👁️
- *Acción:* Con las dos puntas en Degradado, **observar dos ciclos completos (4 minutos)** con un observador en cada punta.
- *Marcar cada punto con lo observado en las LUCES, no en la pantalla:*

```text
[ ] Maestro VERDE  <->  Esclavo ROJO           (verificado a las ______)
[ ] Todo-rojo de ~30 s con AMBAS en rojo       (medido: ______ s)
[ ] Esclavo VERDE  <->  Maestro ROJO           (verificado a las ______)
[ ] Todo-rojo de ~30 s con AMBAS en rojo       (medido: ______ s)
[ ] EN NINGUN MOMENTO hubo verde en las dos puntas a la vez
```

- Duración medida del ciclo completo: ________ s *(esperado ~120 s)*
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > **Por qué mirar las luces y no la pantalla.** Cada unidad muestra la fase que *ella* calcula. Si las dos calculan mal —relojes desfasados, versiones de firmware distintas, una unidad reiniciada— **las dos pantallas dirán que todo va bien** mientras las luces cuentan otra historia. **La pantalla informa; las luces son la evidencia.**
- > **Verde simultáneo en las dos puntas es la única forma en que este equipo puede matar a alguien.** Si lo observa, corte el modo con `B·B·B` de inmediato y **RECHACE**.

**9.8 Corte de energía en UNA punta — riesgo residual nº 2** ⚠️ *(documentar, no necesariamente rechazar)*
- *Acción:* Con las dos puntas en Degradado, **cortar y restituir la alimentación de una sola unidad**.
- *Esperado hoy:* la unidad reiniciada **arranca en el menú, sin enlace, y cae a ÁMBAR**, mientras **la otra sigue dando verde por reloj**.
- Lo observado: ______________________________________
- Resultado: `[ ] SE REPRODUJO  [ ] NO SE REPRODUJO` — Observación: ________________________________
- > **Esto es el riesgo residual nº 2, conocido y aceptado por el cliente el 01/08/2026.** No es un defecto nuevo: es la consecuencia de que el estado del modo viva en RAM (pendiente **N-20**, módulo escrito y sin conectar). Un lado en ámbar contra un lado en verde es **exactamente el escenario que este modo quiere evitar**: el conductor del lado en verde entra confiado a un tramo que el otro lado está negociando.
- > **La mitigación es procedimental, no técnica:** verificación visual de ambas puntas, también al salir. Esta prueba existe para que el funcional **vea el escenario con sus ojos antes de firmar**, no para descubrirlo en obra.

**9.9 Salida asimétrica provocada** ⚠️
- *Acción:* Con las dos puntas en Degradado, sacar del modo **una sola** unidad con `A·A·A`.
- *Esperado:* Se reproduce el mismo escenario que 9.8 — una punta en ámbar, la otra en verde.
- Resultado: `[ ] SE REPRODUJO  [ ] NO SE REPRODUJO` — Observación: ________________________________
- > *Para qué sirve provocarlo:* demuestra por qué **la verificación visual de ambas puntas es obligatoria también AL SALIR**, no solo al entrar. Es la razón de que ese paso esté en el procedimiento y en esta acta.

**9.10 Salida a Automático desde el piso**
- *Acción:* Con el radio **restablecido**, ejecutar `A·A·A` en **ambas** unidades.
- *Esperado:* 2 destellos rojos, y a los ~15 s **las luces vuelven a ciclar** en modo Automático.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**9.11 Salida a Automático con el radio TODAVÍA muerto**
- *Acción:* Con el radio aún desconectado, ejecutar `A·A·A` en ambas unidades.
- *Esperado:* 2 destellos, intenta Automático, **y a los 25 s cae solo a 🟡 Ámbar** en ambas.
- Segundos hasta el ámbar: ________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > *Por qué esta prueba tranquiliza:* **el peor caso de intentar Automático es volver al ámbar**, que es justo donde se quería estar. Por eso volver a Automático no necesita protección y su secuencia es corta.

**9.12 LÍMITE DURO DE 48 h — el modo se rinde solo** ⏱️
- *Acción:* Dejar el Modo Degradado activo **sin radio** y sin resincronizar. Observar a las **44 h** y a las **48 h**.
- *Esperado:*
  - A partir de las **44 h**: la pantalla muestra `AVISO: LIMITE 48h`.
  - A las **48 h**: el modo **cae solo a 🟡 ámbar intermitente**, con el mensaje `Limite 48h sin sync — Revise el radio`. **Sin que nadie intervenga.**
- Hora de entrada al modo: ________  Hora del aviso: ________  Hora de la caída a ámbar: ________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > **Por qué existe el límite, con el número concreto:** el colchón que impide el verde simultáneo es el todo-rojo de 30 s, y se lo come la deriva de dos cristales sin calibrar a la intemperie (**±30 a 50 ppm**, ~8,6 s de separación por día). Con 30 s de todo-rojo el margen teórico son **~3,5 días**; las 48 h dejan **factor de seguridad 2**.
- > **Y por qué es un tope y no un aviso:** *el estado seguro no puede depender de que alguien se acuerde.* Es el mismo principio del fallback de 25 s y del piso de 5 s del despeje. Un modo degradado que dependa de que un operario vuelva a tiempo no es un modo degradado: es una apuesta.
- > *Si necesita más autonomía, la respuesta no es alargar el plazo:* una semana obligaría a un todo-rojo de ~90 s, que destroza la fluidez del paso. **La alternativa real es ir a arreglar el radio.**

**9.13 Salir y volver a entrar NO reinicia la cuenta de 48 h**
- *Acción:* Con el modo llevando ya varias horas activo y sin radio, salir con `B·B·B` y volver a entrar.
- *Esperado:* La cuenta **sigue donde iba**. Solo una **sincronización nueva por radio** la reinicia.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > *Por qué:* el límite mide **el tiempo desde la última sincronización real**, no desde la última pulsación. Si bastara con salir y entrar, cualquiera podría estirar el plazo indefinidamente sin haber corregido nada — y la deriva seguiría corriendo igual.

**9.14 Comportamiento en el cambio de medianoche**
- *Acción:* Dejar el Modo Degradado corriendo **a través de las 00:00**, con observadores en ambas puntas.
- *Esperado:* Al cruzar la medianoche **ambas puntas quedan en todo-rojo**. **No debe saltarse el despeje ni darse verde sin todo-rojo previo.**
- Lo observado entre 23:58 y 00:02: ______________________________________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > *Por qué se prueba específicamente:* a las 00:00:00 el contador del día vuelve a cero, y si la duración del ciclo no divide exactamente al día —casi nunca lo hace— **la posición del ciclo salta**. Las dos unidades saltan igual y a la vez, así que no se desincronizan; el problema es otro y es peor: **ese salto podría caer en mitad de un verde y saltarse el despeje**, dando verde a la otra punta sin todo-rojo de por medio. El firmware fuerza todo-rojo alrededor de la medianoche precisamente por eso, y aquí se comprueba que lo hace.

---

## 📑 SECCIÓN 10 — INTERFAZ PROPIA DEL ESCLAVO (N-16)

> El Esclavo tenía pantalla en hardware desde antes, pero **su firmware no la usaba**. Desde la V8.7
> sí. Esta sección certifica esa interfaz.

**10.1 Menú de 2 opciones, con `ESTADO` primero**
- *Acción:* En el Esclavo, pulsar Botón 4 hasta el menú y recorrerlo.
- *Esperado:* **Exactamente 2 opciones legibles**: `ESTADO` (primera) y `MODO DEGRADADO` (segunda). **No hay modos de operación ni ajuste de hora.**
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > *`ESTADO` va primero a propósito:* con el mando operando a ciegas, lo peor que puede hacer un pulso de aceptar perdido es **mostrar un diagnóstico**. Si la primera opción fuera `MODO DEGRADADO`, un pulso suelto llevaría directo a la pantalla que habilita verdes sin confirmación del otro extremo.

**10.2 La pantalla `ESTADO` muestra lo que hace falta**
- *Acción:* Entrar en `ESTADO`.
- *Esperado:* Hora **con segundos** y si es fiable · antigüedad de la última sincronización · contadores de línea RS-485 · estado actual de la luz. **Se refresca una vez por segundo.**
- Lo que muestra: ______________________________________
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > *Los segundos no son un adorno:* `HH:MM` no sirve para validar nada — dos relojes a 40 s de distancia muestran el mismo `14:32`. Ésta es la comprobación de respaldo cuando el radio ya murió y no hay medida de desfase disponible.

**10.3 El menú del Esclavo NO detiene nada que estuviera funcionando**
- *Acción:* Con el sistema en Modo Automático y enlace activo, entrar al menú del Esclavo y navegarlo.
- *Esperado:* Las luces se comportan igual que cuando se entra al menú del Maestro: **🔴 Rojo Fijo continuo** en ambas puntas.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**10.4 Aviso de Modo Degradado activo en el pie del menú**
- *Acción:* Con el Modo Degradado gobernando la luz en el Esclavo, volver a su menú.
- *Esperado:* El pie del menú muestra **`MODO DEGRADADO ACTIVO`**, sin necesidad de entrar a ninguna pantalla.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > *Por qué está en el pie y no dentro de una pantalla:* es el dato que **cambia el significado de todo lo demás**. Un técnico que suba a revisar y no sepa que el cruce va por reloj **puede creer que el radio funciona** y dar por buena una avería que sigue ahí.

**10.5 Entrada al Degradado desde el Esclavo con doble confirmación**
- *Acción:* En `MODO DEGRADADO` del Esclavo, pulsar Botón 3.
- *Esperado:* Aparece `CONFIRMAR ENTRADA?` con el aviso `Verifique el Maestro`. **Hace falta un segundo Botón 3** para entrar. Botón 4 cancela.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________
- > *La asimetría es deliberada:* **entrar exige dos pulsaciones; salir, una.** Salir lleva el equipo hacia el estado seguro y no necesita protección; entrar habilita verdes sin confirmación del otro extremo, y eso sí. **Lo peligroso se hace difícil; lo seguro, fácil.**

---

## 11. 📷 VALIDACIÓN DE CÁMARAS IA ACUSENSE (MODO INTELIGENTE)

Esta sección certifica la integración de las cámaras de detección vehicular por contacto seco de relé (`1A`/`1B`) conectadas a los pines libres de la tarjeta controladora: `PB0` (Demanda) y `PB8` (Umbral).

**11.1 Demanda vehicular en Sentido 1 (Cámara 1 en Maestro)**
- *Acción:* Simular o hacer cruzar un vehículo frente a la Cámara 1 (Maestro) estando el semáforo en Rojo o en reposo.
- *Esperado:* Cierre de contacto `1A`/`1B` en `PB0` del Maestro. El Maestro registra la demanda, aplica el tiempo configurado de **Despeje Todo-Rojo (`cfgDespejeSeg`)** y conmuta a **🟢 Verde Maestro**.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**11.2 Demanda vehicular en Sentido 2 (Cámara 3 en Esclavo)**
- *Acción:* Simular o hacer cruzar un vehículo frente a la Cámara 3 (Esclavo).
- *Esperado:* Cierre de contacto `1A`/`1B` en `PB0` del Esclavo. El Esclavo transmite la demanda al Maestro por `RS485_OUT`, el Maestro aplica **Todo-Rojo de Despeje** y conmuta a **🟢 Verde Esclavo**.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**11.3 Inmunidad a Peatones, Sombras y Lluvia (Filtro AcuSense)**
- *Acción:* Una persona camina, salta o agita los brazos frente al lente de la cámara en el carril.
- *Esperado:* El filtro AcuSense (*Solo Vehículo*) ignora a la persona: el relé **permanece abierto** y las luces no se alteran.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**11.4 Inmunidad e Independencia de los Botones del Panel LCD**
- *Acción:* Verificar que las activaciones en `PB0` y `PB8` **NO afecten ni interactúen** con las entradas de los botones frontales (`PB9`, `PB13`, `PB14`, `PB15`).
- *Esperado:* Los menús y botones del LCD permanecen 100% estables e independientes.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

---

## 12. 📱 MÓDULO BLUETOOTH DE TELEMETRÍA Y DIAGNÓSTICO (BALIZA)

Esta sección certifica la consola inalámbrica de servicio por Bluetooth en el puerto `USART1` (`PA9` TX, `PA10` RX).

**12.1 Emparejamiento y enlace inalámbrico**
- *Acción:* Conectar el celular vía Bluetooth con PIN `1234` al semáforo a 10 metros de distancia.
- *Esperado:* Enlace establecido en < 5 segundos. El LED del módulo Bluetooth pasa de parpadeo a **encendido fijo**.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**12.2 Telemetría periódica en vivo ($STATUS)**
- *Acción:* Observar la pantalla de la App en el celular.
- *Esperado:* Recepción continua cada 1 segundo exacto de la trama `$STATUS,MODO:...,ESTADO:...,RF:...%,RTT:...ms,HORA:...`.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**12.3 Caja Negra de Alarmas con Hora Exacta RTC ($ALARM)**
- *Acción:* Provocar una caída de radio desconectando la antena del Esclavo durante 12.0 segundos.
- *Esperado:* Ambos semáforos caen a Ámbar Intermitente (SFTY-6) y la App del celular recibe inmediatamente la trama con fecha y hora: `$ALARM,EVENTO:FALLO_RF,CAUSA:SILENCIO_25000ms,ACCION:CAMBIO_A_AMBAR,HORA:...`.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**12.4 Maniobra de Modo Manual y Rojo Total desde el celular**
- *Acción:* Enviar desde la App el comando `CMD:SET_MODO:MANUAL` y luego `CMD:FORZAR_ROJO`.
- *Esperado:* El semáforo conmuta a Modo Manual y al recibir el forzado aplica **🔴🔴 ROJO TOTAL INMEDIATO** en ambos extremos.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**12.5 Selector de Cruces Viales en el Corredor (Multicruce con 1 celular)**
- *Acción:* Abrir la App en el corredor vial y alternar entre Cruce Km 12 (El Sisga) y Cruce Km 24 (Machetá), seleccionando nodo Maestro o Esclavo.
- *Esperado:* La App adapta su interfaz al rol conectado (`👑 MAESTRO (Poste 1)` o `📡 ESCLAVO (Poste 2)`) mostrando las métricas y controles pertinentes.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**12.6 Sincronización Puente Móvil (Modo Courier RTC sin Radio)**
- *Acción:* En el Maestro, presionar `[ 📸 Capturar Maestro ]`. Desplazarse 3 minutos hasta el Esclavo y presionar `[ 🚀 Inyectar en Esclavo ]`.
- *Esperado:* La App calcula el tiempo transcurrido de viaje y programa el RTC DS3231 del Esclavo con la hora exacta compensada ($\Delta t < 0.1\text{ s}$ respecto al Maestro).
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

---

## 13. 🛡️ BLINDAJE DEL MANDO ANTI-COLISIÓN (RESOLUCIÓN N-53)

**13.1 Estrés con codillo en pantalla AJUSTAR HORA (N-53)**
- *Acción:* Entrar en el menú a `AJUSTAR HORA` y presionar **15 veces seguidas y rápido el Botón 1 (Arriba / PB9)**.
- *Esperado:* Los minutos van subiendo correlativamente. El semáforo **NO destella en rojo, NO se sale de la pantalla y NO salta a Automático**.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

**13.2 Nuevas secuencias oficiales del mando**
- *Acción:* Probar desde el suelo las nuevas combinaciones: `A·B·A` (Auto - 2 destellos), `B·A·B` (Ámbar - 3 destellos), `B·A·B·A` (Manual - 5 destellos), `A·B·A·B` (Degradado - 4 destellos).
- *Esperado:* Cada secuencia ejecuta su modo correspondiente con su conteo exacto de destellos rojos y Todo-Rojo preventivo de 15s.
- Resultado: `[ ] CUMPLE  [ ] NO CUMPLE` — Observación: ________________________________

---

## 📊 RESUMEN DE RESULTADOS

```text
NUCLEO YA CERTIFICADO EN V8.0  (aplica a cualquier version de firmware)
Seccion 1 — Menu e independencia de radio ...........  ___ / 4  CUMPLE
Seccion 2 — Perdida de comunicacion y Self-Healing ..  ___ / 5  CUMPLE
Seccion 3 — Modo Automatico .........................  ___ / 5  CUMPLE
Seccion 4 — Modo Inteligente AI .....................  ___ / 4  CUMPLE
Seccion 5 — Modo Manual y menu de dos niveles .......  ___ / 10 CUMPLE
Seccion 6 — Repetidor ESP32 .........................  ___ / 4  CUMPLE
                                                       ------------------
                                        SUBTOTAL       ___ / 32 CUMPLE

FUNCIONES V9.0 (Cámaras IA, Bluetooth, Mando Anti-Colisión y Modo Degradado)
Seccion 7 — Reloj, AJUSTAR HORA y sincronizacion ....  ___ / 11 CUMPLE
Seccion 8 — Mando de 4 reles y secuencias ...........  ___ / 8  CUMPLE
Seccion 9 — Modo Degradado ..........................  ___ / 12 CUMPLE
Seccion 10 — Interfaz propia del Esclavo ............  ___ / 5  CUMPLE
Seccion 11 — Sistema de 4 Cámaras IA AcuSense .......  ___ / 4  CUMPLE
Seccion 12 — Módulo Bluetooth y Telemetría ..........  ___ / 6  CUMPLE
Seccion 13 — Blindaje Mando Anti-Colisión (N-53) ....  ___ / 2  CUMPLE
                                                       ------------------
                                        SUBTOTAL       ___ / 48 CUMPLE

                                        TOTAL          ___ / 80 CUMPLE



DATOS MEDIDOS  (no cuentan como PASS/FALLA: son registro para el acta)

  Calidad de enlace (5.2-bis), en pantalla PRUEBA ALCANCE:
     Directo ...... calidad ____%   respuesta ______ ms
     Repetidor .... calidad ____%   respuesta ______ ms

  Reloj (7.5 / 7.6):
     Deriva contra hora patron en 2 h ....... ______ s
     Diferencia tras corte de energia ....... ______ s  (Maestro)
                                              ______ s  (Esclavo)

  Sincronizacion horaria (7.9):
     Desfase Esclavo medido ................. ______ s   (tolerancia +-3 s)
     Marca de ultima sincronizacion ......... ______:______

  Modo Degradado (9.7 / 9.12):
     Duracion del ciclo completo ............ ______ s   (esperado ~120 s)
     Todo-rojo medido ....................... ______ s   (esperado ~30 s)
     Horas hasta la caida automatica ........ ______ h   (esperado 48 h)


ESCENARIOS DE RIESGO RESIDUAL  (se documentan, no se puntuan)

  9.8  Corte de energia en una sola punta ..... [ ] SE REPRODUJO  [ ] NO
  9.9  Salida asimetrica provocada ............ [ ] SE REPRODUJO  [ ] NO

  El funcional declara haber OBSERVADO ambos escenarios y conocer que la
  mitigacion es PROCEDIMENTAL, no tecnica: verificacion visual de las dos
  puntas, obligatoria tambien AL SALIR del Modo Degradado.

  Firma del funcional sobre este punto: ____________________________________
```

### Pruebas NO CUMPLE — detalle para el equipo de desarrollo

| Nº de prueba | Qué se observó | Segundo / momento exacto | Modo y topología |
|---|---|---|---|
|  |  |  |  |
|  |  |  |  |
|  |  |  |  |

---

## ✍️ ACTA DE CERTIFICACIÓN FUNCIONAL

```text
Fecha de Auditoría: _____ / _____ / 2026        Hora inicio: ______  Hora fin: ______
Lugar / Tramo de Obra: __________________________________________________________
Versión de firmware certificada: ________   Air Data Rate verificado: ________ kbps

ALCANCE CERTIFICADO:
  [ ] Solo nucleo (Secciones 1-6) ......... 32 pruebas
  [ ] Nucleo + funciones nuevas (1-10) .... 68 pruebas

DICTAMEN:
  [ ] APROBADO ..................... todas las pruebas del alcance en CUMPLE
  [ ] APROBADO CON OBSERVACIONES ... sin hallazgos de seguridad vial; detallar arriba
  [ ] RECHAZADO .................... uno o más hallazgos de seguridad vial

DICTAMEN ESPECIFICO SOBRE EL MODO DEGRADADO  (marcar solo si se ejecuto la Seccion 9):
  [ ] APTO para operacion en campo con el procedimiento de 8_Procedimiento_Modo_Degradado.md
  [ ] APTO CON RESTRICCION ... detallar: ______________________________________
  [ ] NO APTO ................ no debe operarse en via abierta al trafico

Se deja constancia de que NO forman parte del alcance de esta certificación las funciones
listadas como "NO IMPLEMENTADAS" al inicio de este documento, y en particular:
  - el receptor de mando de reles del ESCLAVO, no instalado (N-19): activar el Modo
    Degradado en esa punta exige subir al gabinete;
  - la persistencia del estado del Modo Degradado ante corte de energia (N-20).


Ingeniero Funcional / Auditor de Tránsito
Nombre: _________________________________________________________________________
Cargo / Empresa: ________________________________________________________________
Matrícula profesional: __________________________  Firma: _______________________


Ingeniero Responsable de Desarrollo
Nombre: _________________________________________________________________________
Matrícula profesional: __________________________  Firma: _______________________
```
