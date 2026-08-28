# 1. Limpiar 01_Firmware moviendo basuras a 99_Legacy
Move-Item -Path 'D:\@Proyect\Controladora_Semaforos\01_Firmware\Programa_Semaforos' -Destination 'D:\@Proyect\Controladora_Semaforos\99_Legacy' -Force -ErrorAction SilentlyContinue
Move-Item -Path 'D:\@Proyect\Controladora_Semaforos\01_Firmware\Serial' -Destination 'D:\@Proyect\Controladora_Semaforos\99_Legacy' -Force -ErrorAction SilentlyContinue
Move-Item -Path 'D:\@Proyect\Controladora_Semaforos\01_Firmware\Simulaciones' -Destination 'D:\@Proyect\Controladora_Semaforos\99_Legacy' -Force -ErrorAction SilentlyContinue
Move-Item -Path 'D:\@Proyect\Controladora_Semaforos\01_Firmware\object_detection_tracking.py' -Destination 'D:\@Proyect\Controladora_Semaforos\99_Legacy' -Force -ErrorAction SilentlyContinue
Move-Item -Path 'D:\@Proyect\Controladora_Semaforos\01_Firmware\deep_sort' -Destination 'D:\@Proyect\Controladora_Semaforos\01_Firmware\Camara' -Force -ErrorAction SilentlyContinue

# 2. Renombrar maestro
Move-Item -Path 'D:\@Proyect\Controladora_Semaforos\01_Firmware\Semaforos\src\main.cpp' -Destination 'D:\@Proyect\Controladora_Semaforos\01_Firmware\Semaforos\src\maestro.cpp' -Force -ErrorAction SilentlyContinue

# 3. Reescribir esclavo.cpp con Protocolo Binario
 = "
#include <Arduino.h>
#include "pines.h"
#include "semaforo.h"
#include "protocolo.h"
#include <IWatchdog.h>

void setup() {
  semaforo_setup();
  protocolo_setup();
  semaforo_forzarRojo();
  IWatchdog.begin(2000000); // 2 segundos
}

void loop() {
  IWatchdog.reload();
  semaforo_actualizar();

  RF_Packet pkt;
  static unsigned long tUltimoComando = millis();

  if (protocolo_hayPaqueteDisponible(&pkt)) {
    tUltimoComando = millis();
    
    // El PING ahora es implícito o con CMD_GO_GREEN/RED
    if (pkt.command == CMD_GO_RED) {
      semaforo_iniciarTransicionARojo();
    } else if (pkt.command == CMD_GO_GREEN) {
      semaforo_iniciarTransicionAVerde();
    }
    
    // Si recuperamos conexion tras un fallo
    if (semaforo_estado() == S_FALLO) {
      semaforo_forzarRojo();
    }
  }

  // Fallback si no hay comunicación del maestro en 5s
  if (millis() - tUltimoComando > 5000) {
    if (semaforo_estado() != S_FALLO) {
      semaforo_iniciarFallo();
    }
  }

  static bool ackRojoEnviado = false, ackVerdeEnviado = false;
  if (semaforo_estable() && semaforo_estado() == S_ROJO && !ackRojoEnviado) {
    protocolo_enviarPaquete(CMD_ACK_RED);
    ackRojoEnviado = true; ackVerdeEnviado = false;
  }
  if (semaforo_estable() && semaforo_estado() == S_VERDE && !ackVerdeEnviado) {
    protocolo_enviarPaquete(CMD_ACK_GREEN);
    ackVerdeEnviado = true; ackRojoEnviado = false;
  }
}
"
 | Out-File -FilePath 'D:\@Proyect\Controladora_Semaforos\01_Firmware\Semaforos\src\esclavo.cpp' -Encoding UTF8
Remove-Item -Path 'D:\@Proyect\Controladora_Semaforos\01_Firmware\Semaforos\src\esclavo.txt' -Force -ErrorAction SilentlyContinue

# 4. Mover y arreglar repetidor.cpp
 = "#include <Arduino.h>
" + (Get-Content 'D:\@Proyect\Controladora_Semaforos\01_Firmware\RepetidorB\src\main.cpp' | Out-String)
 | Out-File -FilePath 'D:\@Proyect\Controladora_Semaforos\01_Firmware\Semaforos\src\repetidor.cpp' -Encoding UTF8
Move-Item -Path 'D:\@Proyect\Controladora_Semaforos\01_Firmware\RepetidorB\src\pines.h' -Destination 'D:\@Proyect\Controladora_Semaforos\01_Firmware\Semaforos\include\pines_repetidor.h' -Force
Remove-Item -Path 'D:\@Proyect\Controladora_Semaforos\01_Firmware\RepetidorB' -Recurse -Force

# Reemplazamos #include "pines.h" por #include "pines_repetidor.h" en repetidor.cpp
(Get-Content 'D:\@Proyect\Controladora_Semaforos\01_Firmware\Semaforos\src\repetidor.cpp') -replace '"pines.h"', '"pines_repetidor.h"' | Set-Content 'D:\@Proyect\Controladora_Semaforos\01_Firmware\Semaforos\src\repetidor.cpp'

# 5. Crear platformio.ini unificado
 = "
[platformio]
default_envs = maestro, esclavo, repetidor

[env:maestro]
platform = ststm32
board = genericSTM32F103C8
framework = arduino
upload_protocol = stlink
upload_command = "C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe" --connect port=swd reset=HWrst --write "$SOURCE" -rst
lib_deps = olikraus/U8g2@^2.35.19
build_src_filter = +<*> -<esclavo.cpp> -<repetidor.cpp>

[env:esclavo]
platform = ststm32
board = genericSTM32F103C8
framework = arduino
upload_protocol = stlink
upload_command = "C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe" --connect port=swd reset=HWrst --write "$SOURCE" -rst
lib_deps = olikraus/U8g2@^2.35.19
build_src_filter = +<esclavo.cpp> +<semaforo.cpp> +<protocolo.cpp> -<maestro.cpp> -<repetidor.cpp> -<lcd.cpp> -<botones.cpp> -<coordinador.cpp> -<menu.cpp> -<modo_*.cpp>

[env:repetidor]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
build_src_filter = +<repetidor.cpp> -<*>
"
 | Out-File -FilePath 'D:\@Proyect\Controladora_Semaforos\01_Firmware\Semaforos\platformio.ini' -Encoding UTF8
