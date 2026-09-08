// ===== src/demanda.cpp =====
#include "demanda.h"
#include "protocolo.h"

// La ventana de silencio entre demandas.
//
// N-160 - DE DONDE SALE ESTE NUMERO, CORREGIDO: es una decision de TRAFICO, no una
// medida. Aqui ponia "sale de la medida del contacto seco: el rele de la camara
// AcuSense cierra ~1 s por deteccion" -y ESA MEDIDA NUNCA SE TOMO-. El ~1 s se supuso,
// se escribio como si estuviera medido, y despues dos packs se citaron a el. Un
// comentario que se inventa su procedencia es peor que uno que calla: el siguiente lo
// lee como dato y no vuelve a preguntarse de donde salio.
//
// LO QUE ESTE NUMERO SI DEFIENDE, y no depende de ningun rele: que una COLA de coches
// no se convierta en una rafaga de tramas identicas sobre un canal de 2.4 kbps. Tres
// segundos es el hueco por debajo del cual dos coches se tratan como el mismo evento.
// Se sube o se baja mirando el canal y el trafico, no la ficha de la camara.
//
// Y EL TIEMPO QUE EL RELE AGUANTE CERRADO NO INTERVIENE, por una razon de diseno que
// esta una capa mas abajo: botones.cpp lee el FLANCO, no el nivel. Con nivel, un rele
// pegado un segundo repetiria la peticion en cada vuelta del bucle; con flanco, una
// deteccion es una peticion dure lo que dure el contacto.
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

// El unico sitio del que sale este numero hacia fuera. El porque, en demanda.h.
unsigned long demanda_ventanaMs() {
  return SILENCIO_MS;
}
