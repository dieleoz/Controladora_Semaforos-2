// ===== Validacion_Automatico/deg_auto_doble_maestro.cpp =====
//
// A-15 (SPEC_2 7.ter): DOBLE de Maestro/src/deg_auto.cpp, solo las dos funciones que llama
// coordinador.cpp. Lo enlazan compilar.ps1 y compilar_dos_puntas.ps1, cuyo Maestro no
// compila el Degradado: el deg_auto.cpp real arrastraria modo_degradado.cpp, respaldo.cpp
// y reloj.cpp. Con este doble el param de PING/GO_* sale como antes (sin APTO ni ECO) y
// PRESENTE no se filtra. QUE NO SE CUENTE COMO CUBIERTO: el Degradado automatico no lo
// ejerce ningun arnes; lo compila de verdad compilar_degradado.ps1, sin escenarios.
#include "deg_auto.h"

void degAuto_enviar(uint8_t cmd, uint8_t param) { protocolo_enviarPaquete(cmd, param); }
bool degAuto_alRecibir(const RF_Packet*) { return false; }
