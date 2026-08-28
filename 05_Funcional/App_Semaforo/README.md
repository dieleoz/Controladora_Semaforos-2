# 🚦 App IOT-VIAL · Centro de Control Semafórico V9.0

Aplicación móvil híbrida y Web PWA para supervisión, control de tráfico a nivel de suelo y parametrización de semáforos móviles de tres estados.

**Rama Git Actual:** `feat/n69-ajustes-tiempos`  
**APK Compilada:** `05_Funcional/IOT_VIAL_Semaforos_2026-08-28_a8e1ceb_SIN_BANCO.apk`  

---

## 🛠️ Stack Tecnológico

* **Frontend:** HTML5 Semántico + Vanilla JavaScript ES6 Modular + CSS3 Cyber-Industrial.
* **Diseño UI/UX:** Óptica 3D de semáforos LED, efecto *glow bloom*, tema oscuro de alto contraste y ergonomía táctica para uso en campo.
* **Motor Móvil:** Capacitor 6.x + `cordova-plugin-bluetooth-serial` (Bluetooth Clásico SPP a 9600 baudios).
* **Testing:**
  * 🧪 **TDD Unitario:** 29 pruebas unitarias automatizadas (`node tests/test_unitarios.js` - 100% PASS).
  * 📸 **E2E Visual:** Automatización con Puppeteer y capturas en alta resolución (`node tests/test_e2e_visual.js`).
* **Compilación:** Gradle 8 + Android SDK 34 + JDK 17/21.

---

## 🏗️ Arquitectura de 2 Roles

1. **👷 Modo Operario (Por Defecto):**
   * Botonera táctica de 4 botones grandes:
     * 🟢 **Automático**
     * ✋ **Dar Paso (Alternar Turno)**
     * 🟡 **Ámbar Precaución**
     * 🛑 **Rojo Total de Emergencia**
   * Sin contraseñas ni opciones complejas.
2. **🛡️ Modo Técnico / Administrador (PIN `1234`):**
   * Ajuste de tiempos de ciclo (Verde 1-15m, Rojo 1-15m, Despeje 10-90s).
   * Asistente Courier RTC con compensación horaria de viaje.
   * Test de potencia de focos y MOSFETs (6 segundos).
   * Gestor de cruces viales y frentes de obra.

---

## 🚀 Comandos Rápidos

```bash
# 1. Ejecutar pruebas unitarias TDD (29 pruebas)
node tests/test_unitarios.js

# 2. Ejecutar validación visual E2E en Chrome
node tests/test_e2e_visual.js

# 3. Generar infografías y diagramas de arquitectura
python ../generar_graficas_arquitectura.py

# 4. Compilar APK Android
android\compilar_apk.bat
```

---

## 📁 Estructura del Módulo

```text
App_Semaforo/
├── index.html                   # Interfaz de usuario con arquitectura de roles
├── style.css                    # Estilos Cyber-Industrial y ópticas 3D
├── app.js                       # Controlador principal de eventos y vistas
├── js/
│   ├── config.js                # Constantes, PIN 1234 y límites de ciclo
│   ├── nmea_parser.js           # Checksum XOR y formateador de tramas NMEA
│   ├── site_manager.js          # CRUD de cruces con LocalStorage
│   ├── courier_rtc.js           # Lógica de compensación horaria Courier RTC
│   └── bluetooth_driver.js      # Abstracción Cordova SPP / Web BLE / Serial
├── tests/
│   ├── test_unitarios.js        # Suite TDD (29 pruebas unitarias)
│   └── test_e2e_visual.js       # Suite E2E con Puppeteer y capturas
├── android/
│   ├── compilar_apk.bat         # Script dinámico de compilación Gradle
│   └── app/                     # Proyecto Android nativo de Capacitor
└── README.md                    # Documentación técnica
```
