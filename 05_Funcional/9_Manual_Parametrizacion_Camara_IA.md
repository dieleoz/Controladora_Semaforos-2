# 📷 MANUAL DE CONFIGURACIÓN Y PARAMETRIZACIÓN — CÁMARAS IA ACUSENSE PARA SEMÁFOROS MÓVILES (V9.0)

**Sistema:** Controladora de Semáforos Móviles de 3 Estados (Maestro y Esclavo V9.0)  
**Cámara Certificada:** Hikvision AcuSense Varifocal Motorizada (DS-2CD3643G2-LIZSU o equivalente)  
**Topología del Sistema:** Analítica Deep Learning Embebida (sin PC externo) + contacto seco a la bornera **`J14`** (`PB0`, con antirrebote RC de 1 ms en la placa)  
**Verificación Hardware:** Esquemáticos KiCad `Controladora_Semaforos.kicad_sch`, `pines.h` y `MAPEO_TARJETA_KICAD.md`  
**Normativa Aplicable:** Manual de Señalización Vial de Colombia (Resolución 2024 - MinTransporte)  
**Fecha de Emisión:** 26 de Agosto de 2026

---

> ### 🛑 LAS CÁMARAS 2 Y 4 NO ESTÁN OPERATIVAS EN V9.0
>
> **La cámara de umbral no está en V9.0, y no es solo firmware: no hay dónde conectarla.**
> Medido el 27/08 sobre el esquemático: **`PB8` alimenta el LED testigo `D5` a través de `R16`
> 1 kΩ** — no es una bornera ni una entrada optoacoplada. El manual lo daba por una entrada
> *«en reposo»*, y eso venía de leer un esquemático incompleto (`roadmap.md` N-64).
>
> Para tenerla harían falta **dos cosas**, y ninguna es un `pinMode`: **(1)** una entrada física
> —un hilo desde el pad de `PB8` retirando `R16`/`D5`, o desde uno de los cuatro pines sin
> cablear (`PA11`, `PA12`, `PA15`, `PC13`)— y **(2)** un **comando de radio** que lleve la cuenta
> del tramo al Maestro, que es quien decide (`roadmap.md` N-59). Sin el comando, leer el pin es
> medio camino.
>
> Mientras tanto el despeje se hace **por tiempo** (`cfgDespejeSeg`), que es el criterio
> conservador: la cámara de umbral daría **eficiencia**, no seguridad.


## 1. Arquitectura Autónoma para Semáforos Móviles (Sin Raspberry Pi ni Micro-PC)

La cámara Hikvision AcuSense incorpora un procesador de inteligencia artificial con **clasificador Deep Learning de vehículos vs. humanos**. No se requiere ningún computador externo, switch Ethernet ni direccionamiento IP en obra:

```text
 ┌─────────────────────────────────────────────────────────────────────────────┐
 │                ARQUITECTURA AUTÓNOMA DE DETECCIÓN VEHICULAR                 │
 ├─────────────────────────────────────────────────────────────────────────────┤
 │                                                                             │
 │   [ CÁMARA HIKVISION ACUSENSE (Analítica Embebida) ]                        │
 │           │                                                                 │
 │           ▼ (Detección por Intrusión: Clasificador ☑ Solo Vehículo)         │
 │   [ SALIDA DE ALARMA N/O (Bornes 1A / 1B - Contacto Seco) ]                 │
 │           │                                                                 │
 │           ▼ (2 Hilos directos por cámara)                                  │
 │   [ TARJETA CONTROLADORA STM32 - bornera J14, entrada PB0 ]                 │
 │           │                                                                 │
 │           ▼                                                                 │
 │   [ LÓGICA VIAL: Demanda Vehicular + Despeje Todo-Rojo cfgDespejeSeg ]      │
 │                                                                             │
 └─────────────────────────────────────────────────────────────────────────────┘
```

---

## 2. Asignación de Pines y Distribución de Señales

En cada semáforo móvil (Maestro y Esclavo), los relés de las cámaras se conectan a los **dos únicos pines libres** del microcontrolador:

```text
 ┌─────────────────────────────────────────────────────────────────────────────┐
 │                 MAPA DE CONEXIÓN FÍSICA DE CÁMARAS EN PLACA                 │
 ├─────────────────────────────────────────────────────────────────────────────┤
 │                                                                             │
 │ • CÁMARA 1 / 3 (Demanda de Cola):   Borneras 1A/1B ──► Pin PB0 y GND        │
 │   - Detecta si hay vehículos esperando paso en el carril.                   │
 │                                                                             │
 │ • CAMARA 2 / 4 (Umbral):  NO SE INSTALA - PB8 es un LED, no una entrada     │
 │   - Detecta el paso efectivo de vehículos hacia el tramo de obra.           │
 │                                                                             │
 │ • PUERTO USART1 (PA9 / PA10):       Queda 100% LIBRE para Telemetría BT.    │
 │ • BOTONES LCD (PB9, PB13, PB14, PB15): 100% BLINDADOS para Menús y Mando RF.│
 │                                                                             │
 └─────────────────────────────────────────────────────────────────────────────┘
```

---

## 3. Filosofía de Movilidad: Configuración ÚNICA en Taller

> ### 🛑 REGLA DE ORO PARA EQUIPOS MÓVILES:
> **La geometría no discrimina; discrimina el clasificador AcuSense.**  
> Al configurar una **zona de intrusión amplia (~90% de la pantalla)** con el filtro `☑ Vehículo`, el semáforo puede trasladarse de kilómetro en la vía **sin necesidad de conectar una laptop, ni reencuadrar el zoom, ni redibujar máscaras de píxeles**.

---

## 4. Procedimiento de Parametrización en Taller (Paso a Paso)

Se realiza **una sola vez en taller** antes de enviar las cámaras a campo:

```
[ PASO 1: Red y Acceso ] ──► [ PASO 2: Óptica y Luz ] ──► [ PASO 3: Analítica AcuSense ] ──► [ PASO 4: Salida Relé ]
```

### Paso 1: Acceso Inicial
1. Conectar la cámara por cable Ethernet a una laptop con el software *Hikvision SADP* o navegador web.
2. Asignar contraseña segura y fijar IP estática de taller (ej. `192.168.1.61` para Cámara 1, `.62` para Cámara 2, etc.).

### Paso 2: Ajuste Óptico y Nocturno
1. Ir a **Configuración > Imagen > Pantalla**:
   * **Zoom:** Ajustar en **Gran Angular Máximo (2.7 mm)** para obtener el campo de visión máximo ($102.4^\circ$).
   * **Enfoque:** Presionar **One-Touch Focus** (Autofoco).
   * **Precintar:** No volver a mover el zoom.
2. Ir a **Configuración > Imagen > Ajustes de Luz Suplementaria**:
   * **Modo de Luz:** Seleccionar **Solo IR (Infrarrojo 850 nm)**.
   * **Luz Blanca:** **DESACTIVADA** (evita deslumbrar de frente a los conductores en carretera).

### Paso 3: Configuración de la Analítica Inteligente

#### Para Cámaras 1 y 3 (Demanda de Cola - Aproximación):
1. Ir a **Configuración > Eventos > Evento Inteligente > Detección de Intrusión** (*Intrusion Detection*).
2. Marcar ☑ **Habilitar** (*Enable*).
3. **Dibujar Región:** Dibujar un rectángulo amplio que cubra el **90% del encuadre inferior y central** (zona donde se detendrán los vehículos).
4. **Parámetros:**
   * **Tiempo de Permanencia (*Threshold*):** Configurar en **`1s`**.
   * **Sensibilidad (*Sensitivity*):** Configurar en **`50`**.
5. **Clasificación de Objetivo (*Detection Target*):**
   * ☑ **Vehículo (*Vehicle*)**
   * ☐ **Humano (*Human*)** *(Desmarcado: ignora peatones, ramas, lluvia y sombras)*.

#### Para Cámaras 2 y 4 (Umbral de Cruce de Tramo):
1. Ir a **Configuración > Eventos > Evento Inteligente > Detección de Cruce de Línea** (*Line Crossing Detection*).
2. Marcar ☑ **Habilitar** (*Enable*).
3. Trazar la línea atravesando el carril de salida.
4. **Dirección:** **Bidireccional (`A<->B`)**.
5. **Clasificación de Objetivo:** ☑ **Vehículo** | ☐ **Humano**.

### Paso 4: Configuración de la Salida de Relé (Contacto Seco)
1. Ir a **Configuración > Eventos > Salida de Alarma** (*Alarm Output*):
   * **Salida:** `Alarma 1` (Bornes físicos `1A` y `1B`).
   * **Estado por Defecto:** **`NO` (Normally Open / Normalmente Abierto)**.
   * **Duración del Pulso:** **`1s`** (un segundo).
2. Ir a la pestaña **Método de Vinculación (*Linkage Method*)** del evento inteligente configurado y marcar:
   * ☑ **Disparar Salida de Alarma 1** (*Trigger Alarm Output*).
3. **DESARMAR EVENTOS BÁSICOS:** Ir a *Detección de Movimiento básica*, *Sabotaje de Video* y *Excepción* y asegurarse de que la casilla "Disparar Salida de Alarma" esté **desmarcada**. *(Una salida = Un significado único de vehículo)*.
4. Hacer clic en **Guardar** (*Save*).

---

## 5. Dinámica de Control y Seguridad Vial

1. **Llegada de Vehículo al Sentido 1:**
   * La **Cámara 1** detecta el vehículo por intrusión ➔ Cierra el relé `1A`/`1B` en `PB0` del Maestro.
   * El Semáforo Maestro registra la demanda vehicular. Si el sentido opuesto estaba en verde, inicia su cierre: **Verde ➔ Amarillo (4.0s) ➔ Despeje Todo-Rojo (`cfgDespejeSeg`) ➔ Verde Sentido 1**.
2. **Llegada de Vehículo al Sentido 2:**
   * La **Cámara 3** detecta el vehículo por intrusión ➔ Cierra el relé `1A`/`1B` en `PB0` del Esclavo.
   * El Esclavo transmite la demanda al Maestro vía radio LoRa (`RS485_OUT`).
   * El Maestro aplica la transición segura respetando siempre el **Despeje Todo-Rojo** antes de otorgar el verde al Esclavo.
3. **Invariable Vial:** Bajo ninguna circunstancia se omite el tiempo de Todo-Rojo de despeje ni el amarillo normativo de 4.0 segundos.

---

## 6. Protocolo de Pruebas y Validación Rápida en Taller

```text
 ┌─────────────────────────────────────────────────────────────────────────────┐
 │                         BANCO DE PRUEBAS EN TALLER                          │
 ├─────────────────────────────────────────────────────────────────────────────┤
 │                                                                             │
 │ • ENSAYO 1: CONTINUIDAD DEL RELÉ                                            │
 │   - Conectar multímetro en modo continuidad en los bornes 1A y 1B.          │
 │   - Reposo: Circuito abierto (sin pito).                                    │
 │   - Al pasar una maqueta o vehículo: Pita exactamente 1 segundo y se abre.  │
 │                                                                             │
 │ • ENSAYO 2: INMUNIDAD A PEATONES                                            │
 │   - Una persona camina o salta frente al lente.                             │
 │   - Criterio: El relé permanece ABIERTO (cero falsos disparos).             │
 │                                                                             │
 │ • ENSAYO 3: CONMUTACIÓN EN SEMÁFORO                                         │
 │   - Conectar 1A/1B al pin PB0 y GND de la tarjeta STM32.                    │
 │   - Al detectar vehículo: El semáforo atiende la demanda y abre verde tras  │
 │     el despeje de seguridad.                                                │
 │                                                                             │
 └─────────────────────────────────────────────────────────────────────────────┘
```

---
*Manual técnico oficial de integración y configuración de Cámaras IA para Semáforos Móviles V9.0.*
