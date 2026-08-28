# ===== generar_entrega_v9_0.py =====
# Script para empaquetar el paquete de revision V9.0-rc1.
#
# NO produce una entrega de campo. Produce el paquete que el funcional revisa y con el
# que se prepara la sesion de banco. La V8.9 se archivo en 99_Legacy sin llegar a
# entregarse: su LEEME se leia como un permiso y eso es justo lo que no puede pasar.
import os
import zipfile
import shutil

ZIP_NAME = "Entrega_V9.0-rc1_Firmware_Manuales_App.zip"
BASE_DIR = os.path.dirname(os.path.abspath(__file__))

def crear_paquete_entrega():
    print(f"Empaquetando {ZIP_NAME}...")
    
    with zipfile.ZipFile(os.path.join(BASE_DIR, ZIP_NAME), 'w', zipfile.ZIP_DEFLATED) as zipf:
        # 1. Firmware PlatformIO (Maestro, Esclavo, Repetidor)
        firmware_dirs = ["Maestro", "Esclavo", "Repetidor"]
        for fdir in firmware_dirs:
            full_fdir = os.path.join(BASE_DIR, "01_Firmware", fdir)
            for root, dirs, files in os.walk(full_fdir):
                # Excluir .pio, .vscode, __pycache__, etc.
                if ".pio" in root or ".vscode" in root or "__pycache__" in root:
                    continue
                for file in files:
                    file_path = os.path.join(root, file)
                    rel_path = os.path.relpath(file_path, BASE_DIR)
                    arcname = os.path.join("01_Firmware_PlatformIO", fdir, os.path.relpath(file_path, full_fdir))
                    zipf.write(file_path, arcname)
                    print(f"  + FW: {arcname}")

        # 2. Manuales en formato Word .docx
        funcional_dir = os.path.join(BASE_DIR, "05_Funcional")
        for file in os.listdir(funcional_dir):
            if file.endswith(".docx"):
                file_path = os.path.join(funcional_dir, file)
                arcname = os.path.join("02_Manuales_Docx", file)
                zipf.write(file_path, arcname)
                print(f"  + DOCX: {arcname}")

        # 3. App Movil y Proyecto Android Capacitor
        app_dir = os.path.join(BASE_DIR, "05_Funcional", "App_Semaforo")
        for root, dirs, files in os.walk(app_dir):
            if "node_modules" in root or ".gradle" in root or "build" in root or ".idea" in root:
                continue
            for file in files:
                file_path = os.path.join(root, file)
                arcname = os.path.join("03_App_Movil_Android_y_PWA", os.path.relpath(file_path, app_dir))
                zipf.write(file_path, arcname)
                print(f"  + APP: {arcname}")

        # 4. Archivo binario APK compilado
        apk_path = os.path.join(BASE_DIR, "05_Funcional", "IOT_VIAL_Semaforos_v8.9.apk")
        if os.path.exists(apk_path):
            zipf.write(apk_path, "IOT_VIAL_Semaforos_v8.9.apk")
            zipf.write(apk_path, os.path.join("03_App_Movil_Android_y_PWA", "IOT_VIAL_Semaforos_v8.9.apk"))
            print("  + APK: IOT_VIAL_Semaforos_v8.9.apk")

        # 5. Acta de Compuerta y LEEME
        acta_path = os.path.join(BASE_DIR, "evidencia", "2026-08-26_compuerta.txt")
        if os.path.exists(acta_path):
            zipf.write(acta_path, "ACTA_COMPUERTA_11_SUITES.txt")
            print("  + ACTA_COMPUERTA_11_SUITES.txt")

        leeme_content = """# 📦 V9.0-rc1 — SISTEMA CONTROLADOR DE SEMÁFOROS MÓVILES

## 🛑 NO PROBADO EN BANCO — NO APTO PARA CAMPO

**Lo que va en campo hoy sigue siendo la V8.4 (`e303485`), y no cambia con este paquete.**

Esta versión pasa la compuerta de verificación, y eso significa exactamente una cosa:
*los modelos y los arneses de PC no encuentran nada*. **No significa que el firmware
funcione sobre la tarjeta.** Cuatro funciones de esta versión no se pueden validar sin
hardware delante, y ninguna de las cuatro se ha probado todavía:

| Sin probar en banco | Por qué no basta el PC |
|---|---|
| Cámaras IA en `PB0` / `PB8` | Nadie ha cableado nunca un contacto seco a esos pines |
| `PA8` en HIGH (desacoplo de `U3`) | Cambia el estado eléctrico del `MAX3485`. Es hardware |
| `CMD_DEMANDA` / `CMD_ACK_DEMANDA` | Jamás han cruzado una radio real |
| Bluetooth en `USART1` | Comparte pista con el transceptor `U3` |

**Uso previsto de este paquete:** revisión del funcional y preparación de la sesión de
banco. Cargar en un equipo de calle requiere primero esa sesión, con acta.

---

**Fecha de Generación:** 26 de Agosto de 2026  
**Compuerta de Verificación:** 11/11 PASS (0 Fallas, 0 Abortados) — *sobre modelos y arneses de PC*

---

### 📂 Contenido del Paquete:

1. **`01_Firmware_PlatformIO/`**:
   - `Maestro/`: Firmware C++ STM32 (Modo Inteligente, Cámaras PB0/PB8, Telemetría Bluetooth PA9/PA10, Desacoplo U3 PA8 en HIGH).
   - `Esclavo/`: Firmware C++ STM32 (Demanda remota LoRa Cámara 3, Telemetría Bluetooth, Courier RTC).
   - `Repetidor/`: Firmware C++ ESP32 (Puente validador RF a 2.4 kbps).

2. **`02_Manuales_Docx/`**:
   - Los 12 manuales técnicos y actas en formato Microsoft Word (.docx) 100% actualizados y armonizados sin contradicciones.

3. **`03_App_Movil_Android_y_PWA/`**:
   - PWA Standalone (`index.html`, `style.css`, `app.js`, `manifest.json`, `sw.js`) para instalación inmediata en Chrome Android.
   - Proyecto Nativo Android Studio (`android/`) listo para compilar `app-debug.apk`.

4. **`ACTA_COMPUERTA_11_SUITES.txt`**:
   - Resultado de la compuerta sobre **modelos, arneses de PC y compilación**. No es un
     certificado de funcionamiento en la tarjeta: la compuerta no carga firmware ni mueve
     luces. Comprobar que el `HEAD` del acta coincide con el commit de este paquete antes
     de citarla.

---

### ⚠️ Notas de seguridad para quien instale esto

- **PIN `1234` de fábrica.** Es el único control de acceso a los cambios de modo por
  Bluetooth y viene documentado en los manuales. Cambiarlo antes de operar en vía pública.
- **El `ROJO DE EMERGENCIA` no pide PIN**, y es deliberado: parar el tráfico es la acción
  segura y no debe costar teclear una clave.
- **La APK es una compilación `debug`**, firmada con el almacén de depuración. Sirve para
  probar; no es una versión de distribución.
- **El fichero de la APK conserva el nombre `v8.9`** a propósito: es el binario que se
  compiló, y no se renombra por cosmética. El código de la app no ha cambiado en V9.0;
  recompilarla con la etiqueta nueva queda pendiente (`android/compilar_apk.bat`).
"""
        zipf.writestr("LEEME_ENTREGA_V9.0.md", leeme_content)
        print("  + LEEME_ENTREGA_V9.0.md")

    print(f"\n[OK] Paquete creado con exito: {ZIP_NAME} ({os.path.getsize(ZIP_NAME) / 1024 / 1024:.2f} MB)")

if __name__ == "__main__":
    crear_paquete_entrega()
