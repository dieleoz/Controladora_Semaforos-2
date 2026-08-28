# 📱 MANUAL DE USUARIO Y ARQUITECTURA DE LA APP MÓVIL IOT-VIAL (V9.0)

**Sistema:** Centro de Control, Operación y Diagnóstico Semafórico a Nivel de Suelo  
**Rama Git:** `feat/n69-ajustes-tiempos`  
**Plataforma:** Android Nativo (.APK) y Web Testing PWA  
**Protocolo:** Bluetooth Serial SPP (HC-05 / JDY-31 a 9600 bps) / BLE GATT  
**Fecha de Actualización:** 28 de Agosto de 2026  
**Versión de Firmware Compatible:** V8.9 / V9.0 Definitiva  
**Archivo APK Compilado:** `05_Funcional/IOT_VIAL_Semaforos_2026-08-28_a8e1ceb_SIN_BANCO.apk`  

---

## 1. 🌟 INTRODUCCIÓN Y ARQUITECTURA DE 2 ROLES

La aplicación móvil **IOT-VIAL V9.0** elimina la fricción operativa en obra separando claramente la experiencia en **dos perfiles de usuario**:

![Arquitectura de 2 Roles](./graficas/grafica_01_arquitectura_roles.png)

```text
┌──────────────────────────────────────────────────────────────────────────┐
│                   PERFILES DE ACCESO EN APP IOT-VIAL V9.0                │
├──────────────────────────────────────────────────────────────────────────┤
│ 👷 MODO OPERARIO (POR DEFECTO):                                          │
│   • Botonera táctil de campo de 4 botones grandes (estilo mando de relé)│
│   • 1 toque: 🟢 Automático | ✋ Dar Paso | 🟡 Ámbar | 🛑 Rojo Total      │
│   • Cero contraseñas ni formularios complejos para el operario de obra.  │
├──────────────────────────────────────────────────────────────────────────┤
│ 🛡️ MODO TÉCNICO / ADMINISTRADOR (PIN '1234'):                           │
│   • Ajustes de tiempos de ciclo (Verde 1-15m, Rojo 1-15m, Despeje 10-90s)│
│   • Asistente Courier RTC con compensación automática de viaje.          │
│   • Test secuencial de potencia de MOSFETs y focos (6 segundos).         │
│   • Gestor CRUD de frentes de obra y cruces viales en LocalStorage.      │
└──────────────────────────────────────────────────────────────────────────┘
```

---

## 2. 🛠️ STACK TECNOLÓGICO Y PIPELINE DE DESARROLLO

![Stack Tecnológico](./graficas/grafica_03_stack_tecnologico_compilacion.png)

### 2.1 Componentes del Stack
* **Frontend Web:** HTML5 Semántico + Vanilla JavaScript ES6 Modular (sin frameworks pesados para garantizar 60 fps y cero lag en WebView).
* **Diseño & UI/UX:** CSS3 Cyber-Industrial con cabezas semafóricas 3D, lentes Fresnel LED con efecto *glow bloom*, tema oscuro de alto contraste OLED y micro-animaciones.
* **Motor Híbrido Android:** **Capacitor 6.x** + plugin **Cordova Bluetooth Serial** (`cordova-plugin-bluetooth-serial`) para comunicación por sockets RFCOMM SPP nativos con módulos HC-05 y JDY-31.
* **Almacenamiento Local:** `LocalStorage API` para persistencia offline de cruces viales y registros de eventos.

### 2.2 Suite de Pruebas Automatizadas TDD y E2E Visual
* **29 Pruebas Unitarias TDD (100% PASS):** Validan checksum XOR NMEA, parseo de `$STATUS`/`$ALARM`/`$ERR`, generación segura de comandos con PIN, rangos de tiempos de ciclo, lógica Courier RTC y CRUD de cruces.
* **E2E Visual (Puppeteer Core):** Automatiza la apertura de modales, selección de dispositivos, cambio de cruces y captura evidencia gráfica en alta resolución en `evidencia/`.

---

## 3. 📦 GUÍA DE COMPILACIÓN Y CONSTRUCCIÓN DEL APK

### 3.1 Requisitos de Entorno
* **Java Development Kit:** JDK 17 o JDK 21 moderno (`JAVA_HOME`).
* **Android SDK:** `build-tools 34.0.0`, `platforms;android-34` (`ANDROID_HOME`).
* **Node.js:** Versión 18 o superior.

### 3.2 Pasos para Compilar
Desde la carpeta raíz de la App (`05_Funcional/App_Semaforo/`):

1. **Ejecutar Pruebas Unitarias TDD:**
   ```bash
   node tests/test_unitarios.js
   ```
2. **Ejecutar Validación Visual E2E en Navegador:**
   ```bash
   node tests/test_e2e_visual.js
   ```
3. **Compilar el APK:**
   ```bash
   android\compilar_apk.bat
   ```
   *El script sincroniza los assets web con Capacitor (`npx cap sync android`) y ejecuta Gradle `assembleDebug` generando el archivo maestro en `05_Funcional/IOT_VIAL_Semaforos_2026-08-28_a8e1ceb_SIN_BANCO.apk` en ~20 segundos.*

---

## 4. 🔄 ASISTENTE COURIER RTC (SINCRONIZACIÓN SIN RADIO)

Para tramos donde la geografía bloquea el enlace de radio LoRa/RS-485, el **Asistente Courier RTC** permite sincronizar ambos postes utilizando el celular como puente de tiempo:

![Flujo Courier RTC](./graficas/grafica_02_courier_rtc_flujo.png)

### Procedimiento en Campo:
1. Conectar al **Poste 1 (Maestro)** y presionar **`📸 Capturar Maestro`**. La app memoriza la hora RTC exacta y la fase activa e inicia el contador de viaje.
2. Trasladarse físicamente al **Poste 2 (Esclavo)** (a pie o en vehículo).
3. Conectar al **Poste 2** y presionar **`🚀 Inyectar en Esclavo`**.
4. La App suma automáticamente los segundos de traslado transcurridos ($\Delta t$) y programa el reloj DS3231 del Esclavo con la hora compensada, logrando sincronismo total sin radio.

---

## 5. 📡 COMANDOS Y PROTOCOLO NMEA

Todos los comandos emitidos hacia el firmware STM32 viajan en formato NMEA estándar con delimitador `$` y checksum XOR de 2 caracteres:

```text
$CMD,PIN,1234,SET_MODO,AUTO*2E\r\n
$CMD,PIN,1234,SET_MODO,AMBAR*1B\r\n
$CMD,PIN,1234,FORZAR_ROJO*3A\r\n
$CMD,PIN,1234,SET_TIEMPOS,2,2,15*5C\r\n
$CMD,PIN,1234,SET_RTC,18:27:00*41\r\n
```

---

## 6. 📊 EVIDENCIA DE PANTALLAS (GALERÍA V9.0)

| Vista | Captura | Descripción |
|---|---|---|
| **Modo Operario** | `evidencia/01_modo_operario_principal.png` | Botonera táctica de 4 botones y réplica 3D de semáforos |
| **Selector de Cruces** | `evidencia/02_modal_cruces_abierto.png` | Catálogo de frentes de obra y selección inmediata |
| **Modo Técnico** | `evidencia/06_modo_tecnico_activo.png` | Desbloqueo por PIN y pestañas de configuración |
| **Ajustes de Tiempos** | `evidencia/07_tiempos_guardados_exito.png` | Programación de tiempos de verde, rojo y despeje |
| **Courier RTC** | `evidencia/08_courier_rtc_inyectado.png` | Inyección de hora compensada en el nodo Esclavo |

---
**Desarrollado y Validado para el Proyecto Controladora_Semaforos V9.0**
