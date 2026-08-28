// ===== src/config_ciclo.cpp (ESCLAVO) =====
//
// FASE 2 DEL PLAN DE ARQUITECTURA — MOVIMIENTO PURO, SIN CAMBIO DE COMPORTAMIENTO.
//
// Esto vivia dentro de src/main.cpp, y el delator de que no era su sitio estaba a la
// vista: config_verdeSegundos() y sus tres hermanas son API PUBLICA -las declara
// config_ciclo.h y las consume el Modo Degradado- y estaban IMPLEMENTADAS EN EL PUNTO
// DE ENTRADA. Un modulo que incluye una cabecera cuya implementacion vive en main.cpp
// no tiene forma de probarse solo.
//
// Y aqui vive el unico fallo abierto del validador del Esclavo, el 30/31: el par
// verde+despeje mezclado entre envios. Enterrado en un fichero de 643 lineas era caro
// de mirar; aqui se lee de una sentada y se ejerce con:
//
//     python 01_Firmware/Simulaciones/banco/correr.py --pack esclavo_03
//
// QUE HACE ESTE MODULO. Guarda la duracion del ciclo que el Maestro anuncia por radio
// (CMD_CONFIG_VERDE / CMD_CONFIG_DESPEJE). NO altera ninguna temporizacion del ciclo
// normal: mientras el Maestro gobierne por radio, las luces las ordena el. Su unico
// consumidor es el Modo Degradado (SFTY-21), que necesita estos dos numeros para
// seguir en fase cuando ya no hay radio a quien obedecer.

#include <Arduino.h>
#include "config_ciclo.h"
#include "respaldo.h"

// El par se toma como una UNIDAD: o valen los dos numeros que llegaron por radio, o no
// vale ninguno. Verde y despeje viajan juntos y describen UN ciclo; mezclar el verde
// de esta sesion con el despeje guardado en la pila daria una duracion total que no
// calculo nadie, y las dos puntas dejarian de coincidir a los pocos minutos. Ese
// solape es exactamente lo que ciclo_degradado.h existe para impedir.
static uint8_t cfgVerdeSeg = 0;
static uint8_t cfgDespejeSeg = 0;
static bool cfgVerdeRecibido = false;
static bool cfgDespejeRecibido = false;

// El 30/31: cfgVerdeRecibido mezclaba DOS preguntas distintas, y por eso no podia
// contestar bien a ninguna.
//
//   "la radio entrego el par"        -> la usan los getters publicos de abajo
//   "hay un VERDE aun sin emparejar" -> la usa verdeDeEsteEnvio() para cerrar el par
//
// Apagar cfgVerdeRecibido al cerrar el par arregla la segunda y ROMPE la primera:
// cfgRadioCompleto() quedaria en false para siempre, los cuatro getters caerian a la
// pila, y si respaldo_guardarCiclo() no llego a guardar -se niega en cuanto un tramo
// es cero- el Esclavo diria que NO tiene ciclo con el par recien recibido Y YA
// ACUSADO. El Maestro lo daria por aplicado y las dos puntas dejarian de coincidir,
// que es justo lo que este fichero existe para impedir.
//
// Son dos banderas porque son dos hechos. La de emparejar se consume; la de "la radio
// hablo" no.
static bool cfgVerdePendiente = false;

static bool cfgRadioCompleto() { return cfgVerdeRecibido && cfgDespejeRecibido; }

// Ventana de vigencia del VERDE a la espera de su DESPEJE, en el mismo envio.
//
// Las dos banderas de arriba eran PEGAJOSAS: decian que ambas tramas llegaron ALGUNA
// VEZ, no que fueran del MISMO par. El validador del Esclavo lo reprodujo el
// 01/08/2026 y el resultado era el que ciclo_degradado.h existe para impedir:
//
//   config (30,20) correcta. El operario cambia a (45,25). Se pierde la trama del
//   VERDE. El Esclavo se queda con (30,25) -verde viejo, despeje nuevo-, LO ACUSA, y
//   el Maestro da la configuracion por buena y no reintenta. El respaldo graba la
//   mezcla, que ademas sobrevive al corte.
//
//   Medido: Maestro 140 s de ciclo, Esclavo 110 s. Primer solape verde-verde en el
//   segundo 165 del dia.
//
// Se trata igual que la hora: el VERDE se aparca con marca de tiempo y solo el DESPEJE
// cierra el par, y solo si el verde es de ESTE envio. Las dos tramas viajan seguidas
// -milisegundos-, asi que 3 s es holgado y a la vez acota la mezcla.
static const unsigned long VENTANA_CONFIG_MS = 3000;
static unsigned long tCfgVerde = 0;

static bool verdeDeEsteEnvio() {
  // PENDIENTE, no "recibido": la ventana de 3 s descarta un VERDE viejo, pero no un
  // VERDE reciente QUE YA CERRO UN PAR. Con dos reconfiguraciones separadas por menos
  // de 3 s y la segunda trama de VERDE perdida, ese verde ya gastado cerraba el
  // segundo par y volvia a colarse la mezcla.
  return cfgVerdePendiente && (millis() - tCfgVerde) <= VENTANA_CONFIG_MS;
}

// ---------------------------------------------------------------------------
// Recepcion de las dos tramas del par.
// ---------------------------------------------------------------------------

void config_rxVerde(uint8_t segundos) {
  // NO se acusa aqui. El Maestro envia VERDE y DESPEJE seguidas y espera UN solo ACK
  // del par, asi que confirmar las dos por separado le devolveria un ACK sobrante.
  //
  // Antes funcionaba, pero POR ACCIDENTE: al caber una sola respuesta pendiente, la de
  // DESPEJE pisaba a la de VERDE dentro de la ventana de cortesia de 200 ms y salia
  // una sola. Bastaba con que las dos tramas se separasen -retransmision, espaciado de
  // rafaga, buferado del repetidor- para que salieran dos. Depender de esa carrera no
  // es un diseno.
  //
  // Se confirma solo con la ULTIMA del par, igual que la hora se aplica solo al llegar
  // la trama de segundos.
  cfgVerdeSeg = segundos;
  cfgVerdeRecibido = true;
  cfgVerdePendiente = true;   // aun sin emparejar: lo consumira su DESPEJE
  tCfgVerde = millis();       // el par tiene que cerrarse dentro de la ventana
}

bool config_rxDespeje(uint8_t segundos) {
  // El par solo se cierra si el VERDE es de ESTE envio. La comprobacion no puede ser
  // "llego alguna vez": esa era pegajosa y aceptaba mezclas.
  //
  // Sin acuse, el Maestro reintenta la pareja entera y cuesta unos segundos. Aceptar
  // un ciclo mezclado deja a las dos puntas calculando duraciones distintas sobre la
  // misma hora, y nadie se entera hasta que los verdes se solapan -medido: segundo 165
  // del dia con (30,20) contra (30,25)-.
  if (!verdeDeEsteEnvio()) {
    cfgVerdePendiente = false;  // lo que hubiera es basura para este par
    return false;               // el silencio provoca el reintento del Maestro
  }

  cfgDespejeSeg = segundos;
  cfgDespejeRecibido = true;

  // El 30/31: el VERDE se CONSUME al cerrar el par. Sin esto seguia sirviendo para
  // cerrar un segundo par cuyo propio VERDE se hubiera perdido, siempre que las dos
  // reconfiguraciones cayeran a menos de 3 s -el ataque 3.2 del pack esclavo_03-.
  //
  // Se apaga la bandera de EMPAREJAR, no la de "la radio hablo": esa segunda sigue en
  // pie y es la que sostiene a los getters. Ver el comentario de las declaraciones.
  cfgVerdePendiente = false;

  // N-20: se guarda el PAR COMPLETO y solo aqui, al cerrarse. Guardar el verde en
  // cuanto llega dejaria en la pila medio ciclo -verde nuevo con despeje viejo-
  // durante los milisegundos que tardase la segunda trama, y si el corte de energia
  // cayera justo ahi, el equipo reanudaria con una duracion de ciclo que el Maestro
  // nunca envio.
  respaldo_guardarCiclo(cfgVerdeSeg, cfgDespejeSeg);

  return true;                  // el llamante acusa el par
}

// ---------------------------------------------------------------------------
// Getters publicos.
//
// El flag "recibido" importa tanto como el valor: un 0 puede ser "el Maestro dijo
// cero" o "nunca llego nada", y el Modo Degradado no puede confundir ambos casos.
//
// N-20: SI EN ESTA SESION NO LLEGO NADA POR RADIO, SE USA LO GUARDADO EN LA PILA. No
// es un adorno: el caso que cubre es justo el que motiva todo N-20 -un corte de
// energia sin radio-. Al volver la luz, el Maestro no puede reenviar la configuracion
// porque el radio es lo que no hay; sin esta via el Esclavo arrancaria sin saber la
// duracion del ciclo, el Modo Degradado no podria reanudarse y esta punta caeria a
// ambar mientras la otra sigue dando verde por reloj.
//
// La radio SIEMPRE gana cuando esta completa: lo guardado es un recuerdo, no una
// fuente de verdad, y en cuanto el Maestro vuelve a hablar manda lo que diga ahora.
// ---------------------------------------------------------------------------

uint8_t config_verdeSegundos()    { return cfgRadioCompleto() ? cfgVerdeSeg   : respaldo_verdeSeg(); }
uint8_t config_despejeSegundos()  { return cfgRadioCompleto() ? cfgDespejeSeg : respaldo_despejeSeg(); }
bool    config_verdeRecibido()    { return cfgRadioCompleto() || respaldo_hayCiclo(); }
bool    config_despejeRecibido()  { return cfgRadioCompleto() || respaldo_hayCiclo(); }
