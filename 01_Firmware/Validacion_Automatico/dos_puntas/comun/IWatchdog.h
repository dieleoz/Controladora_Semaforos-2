// ===== Validacion_Automatico/dos_puntas/comun/IWatchdog.h =====
//
// Sustituto de la clase IWatchdog de STM32duino. Esta aqui porque el bucle REAL del
// Esclavo -que es lo que este arnes ejerce- empieza con IWatchdog.reload(), y sin este
// fichero no compila.
//
// NO EMULA EL PERRO GUARDIAN, Y ESO SE DICE EN VOZ ALTA. Aqui no hay reinicio por
// timeout: SFTY-1 no se mide en este arnes. Lo unico que se guarda es la CUENTA de
// recargas, que el orquestador puede leer para exigir que el bucle de cada punta se
// haya recorrido entero -un bucle que se sale antes de tiempo dejaria de recargar el
// perro y en la tarjeta reiniciaria el equipo-.
#pragma once

class IWatchdogClase {
 public:
  unsigned long recargas = 0;
  unsigned long arranques = 0;
  void begin(unsigned long) { arranques++; }
  void reload() { recargas++; }
};

extern IWatchdogClase IWatchdog;
