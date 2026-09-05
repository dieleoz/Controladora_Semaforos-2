// ===== src/demanda.cpp =====
#include "demanda.h"

// La ventana de silencio entre demandas. Sale de la medida del contacto seco: el rele
// de la camara AcuSense cierra ~1 s por deteccion, y un coche detras de otro dispara
// pulsos separados por poco mas de un segundo. Sin ventana, una cola entera se
// convierte en una rafaga de peticiones identicas.
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
  return true;
}

bool demanda_hayLocal() {
  // LA MISMA VENTANA, no una segunda constante. Que la vigencia de la demanda y el
  // tiempo que hay que esperar para repetirla sean el mismo numero es lo que impide
  // los dos defectos de escribirlos aparte: un hueco -demanda ya caducada y todavia
  // sin poder pedirla otra vez- o un solape que la alargue sin que nadie lo pida.
  return !primera && (millis() - tUltima) <= SILENCIO_MS;
}

// El unico sitio del que sale este numero hacia fuera. El porque, en demanda.h.
unsigned long demanda_ventanaMs() {
  return SILENCIO_MS;
}
