// ===== src/demanda.cpp =====
#include "demanda.h"
#include "protocolo.h"

// La ventana de silencio entre demandas. Sale de la medida del contacto seco: el rele
// de la camara AcuSense cierra ~1 s por deteccion, y un coche detras de otro dispara
// pulsos separados por poco mas de un segundo. Sin ventana, una cola entera se
// convierte en una rafaga de tramas identicas sobre un canal de 2.4 kbps.
static const unsigned long SILENCIO_MS = 3000;

static unsigned long tUltima = 0;
static bool primera = true;

bool demanda_solicitar() {
  const unsigned long ahora = millis();

  // La primera demanda tras el arranque no espera. millis() vale ~0 y la resta contra
  // un tUltima tambien en 0 daria "dentro de la ventana", tragandose justo la peticion
  // del primer coche que llega a un equipo recien encendido.
  if (!primera && (ahora - tUltima) <= SILENCIO_MS) {
    return false;
  }

  primera = false;
  tUltima = ahora;
  protocolo_enviarPaquete(CMD_DEMANDA);
  return true;
}
