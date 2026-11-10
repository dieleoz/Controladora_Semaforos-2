// ===== src/bluetooth.cpp (MAESTRO) =====
#include "bluetooth.h"
#include "pines.h"
#include "coordinador.h"
#include "demanda.h"
#include "modo_automatico.h"
#include "modo_degradado.h"
#include "modo_ambar.h"
#include "semaforo.h"
#include "menu.h"
#include "modos.h"
#include "reloj.h"
#include "identidad.h"
#include "botones.h"       // D-13: camara_estado(), la fuente del campo CAM:
#include "protocolo.h"     // D-23 gemelo: los tres contadores de linea de SFTY-15
#include "version_fw.h"    // el sello del binario, inyectado por platformio.ini
#include "deg_auto.h"      // A-15: SET_DEG_AUTO / CONSULTA_DEG_AUTO
#include "modo_noche.h"    // D-110: SET_NOCHE / SET_NOCHE_TR / CONSULTA_NOCHE
#include "respaldo.h"      // A-15: la opcion propia y el APTO oido del otro, para la consulta
#include "interruptores.h" // SPEC_8 6: SET_VETO_CAMARA y SET_BARRERA_SERVICIO (D-94)
#include "instalacion.h"   // N-187: SET_CAMARA, SET_PLUMA_RETARDO, CONSULTA_INSTALACION
#include <string.h>
#include <stdio.h>

// USART1 REMAPEADO a PB7 (RX) / PB6 (TX). Antes iba en PA9/PA10, y se movio
// porque el modulo Bluetooth de campo se conecta al CONECTOR J17 -posiciones 2
// y 3-, que es enchufable. PA9/PA10 no salen a ninguna bornera de la tarjeta:
// para usarlos hay que soldar en las patas del MAX3485 U2 o del propio micro.
//
// Es el MISMO periferico, no un segundo puerto serie: el STM32F103 permite
// sacar USART1 por PB6/PB7 en vez de PA9/PA10, pero solo por un sitio a la vez.
// Por eso protocolo_setup() dejo de abrir AiBus, que declaraba el mismo USART1
// sobre PA10/PA9 a 115200 -ver el comentario de alli-.
//
// Estos dos pines los tenia lcd.cpp para PSB y RST del display; ninguno de los
// dos llevaba datos, asi que la pantalla sigue funcionando con SCLK/SID/CS.
static HardwareSerial SerialBT(PB7, PB6); // USART1 remapeado: PB7 RX, PB6 TX

static unsigned long tUltimaTelemetria = 0;
static char btBufIn[64];
static uint8_t btIdxIn = 0;

// D-32 (1), 13/09 - LA CADENCIA DEL DIAGNOSTICO DE ENLACE DE ESTA PUNTA.
//
// ES EL GEMELO DECLARADO del DIAG_ENLACE_MS del Esclavo, y el numero es el MISMO a
// proposito: el porque entero -que no sale de los bytes sino de las 30 entradas a las que
// addEvent() de app.js recorta la bitacora- esta escrito una sola vez, en
// Esclavo/src/bluetooth.cpp, y no se repite aqui. Lo que si se dice es que son gemelos:
// dos numeros que significan lo mismo se separan el dia que alguien toca uno, y hay un
// pack que los recalcula y falla si divergen.
static const unsigned long DIAG_ENLACE_MS = 30000UL;
static unsigned long tUltimoDiagEnlace = 0;

// ---------------------------------------------------------------------------
// A3 - EL REGISTRO DE SILENCIO DEL PUERTO J17.
// QUE MIDEN ESTOS TRES NUMEROS Y QUE NO. ESTA ES LA MITAD IMPORTANTE.
//
// MEDIDO sobre el puente, no supuesto:
//   ESP32_Expansion/src/enlace_stm32.cpp:27-33  "6.4 - SILENCIO NO ES ORDEN [...] aqui
//                                               NO se manda nada"
//   ESP32_Expansion/src/puente.cpp:29-35        lo mismo por el otro lado
//   enlace_escribirLinea() tiene UN solo llamador -puente.cpp:116-, y es el reenvio
//   VERBATIM de lo que mando la app.
// Y sobre la app: no hay un solo envio periodico al firmware; sus dos setInterval son
// el reloj de la cabecera y vigilarEnlace(), que solo ESCUCHA.
//
// O sea que por J17 entra EXACTAMENTE lo que un dedo pulsa en el telefono. De modo que
// esto cuenta SILENCIOS DEL PUERTO, no muertes del puente: mientras nadie use la app el
// puerto esta mudo con el puente perfectamente vivo, y desde aqui "el puente no dice
// nada" y "el puente no esta" NO SE DISTINGUEN -lo dice el propio puente en los dos
// comentarios de arriba-.
//
// Se nombra asi, y no "cortes del puente", porque un contador que prometiera eso seria
// el RF:98% otra vez: un numero con forma de medida. El dia que el ESP32 emita un latido
// propio -AB-1, ESP32_Expansion/src/main.cpp:22-28, abierto y del responsable- estos
// mismos tres numeros pasan a ser el registro de cortes de verdad SIN TOCAR UNA LINEA de
// aqui. Esa es la mitad que si se puede construir hoy.
//
// NO HAY UMBRAL, Y ES LA DECISION DE DISENO. Un silencio se define por sus DOS EXTREMOS
// -la linea que lo abre y la que lo cierra-, nunca por un limite. Un limite seria un
// numero que nadie ha decidido gobernando lo que el tecnico ve, y ademas la precondicion
// para que alguien acabe alimentando este silencio con el de la radio. SFTY6_SILENCIO_MS
// vigila la RADIO y no se nombra en este fichero: son dos silencios y dos instrumentos.
//
// Y ESTO CUENTA; NO ACTUA. De aqui no sale una sola escritura de luz. El ambar
// automatico sigue reservado a los caminos que ya lo tienen -SFTY-6 y el watchdog-: la
// maquina no decide sola operar de un modo que nadie pidio.
//
// EL ULTIMO SILENCIO NO SE GUARDA, Y NO ES UN OLVIDO. Se publica en el instante en que
// se cierra -que es el unico en que alguien puede estar mirando, ver abajo-, asi que
// entre llamada y llamada no lo lee nadie. MEDIDO: con la variable static puesta,
// arm-none-eabi-nm sobre el .elf NO la encuentra en .bss -el enlazador ya sabia que
// nadie la lee y la habia borrado-. Dejarla declarada seria anunciar un estado que no
// existe: quien lo leyera creeria que se puede consultar mas tarde, y no se puede.
// LIMITE DECLARADO: millis() da la vuelta a los 49,7 dias. La resta sin signo mide bien
// UNA vuelta; un silencio mas largo que eso queda aliasado. Se escribe en vez de
// esconderse.
// ---------------------------------------------------------------------------
static unsigned long tUltimaLineaJ17  = 0;  // millis() de la ultima linea completa
static unsigned long j17Silencios     = 0;  // cerrados: uno por linea. 0 = ninguna aun
static unsigned long j17SilencioMaxMs = 0;  // el mas largo desde el arranque

static uint8_t calcularChecksum(const char* str) {
  uint8_t crc = 0;
  while (*str && *str != '*') {
    crc ^= (uint8_t)(*str);
    str++;
  }
  return crc;
}

static void enviarTramaConCrc(const char* payload) {
  uint8_t crc = calcularChecksum(payload + 1); // Salta el '$' inicial
  // N-108: 160 y no 140. El $ALARM pasa a llevar el ultimo tramo del enlace y su peor
  // caso medido son 128 B de payload; con 140 aqui, el cierre del checksum entraba
  // justo y una CAUSA mas larga habria truncado la trama SIN AVISO -y una trama
  // truncada es una que el otro extremo descarta por checksum, o sea una alarma que
  // desaparece justo cuando hace falta-.
  char tramaCompleta[160];
  snprintf(tramaCompleta, sizeof(tramaCompleta), "%s*%02X\r\n", payload, crc);
  SerialBT.print(tramaCompleta);
}

void bluetooth_setup() {
  // EL CHIP ES U2, NO U3. Aqui ponia "U3" y estaba invertido; se deja escrito para que no
  // se vuelva a poner al reves. Trazado red por red sobre el esquematico bueno
  // (01_Firmware/Controladora_Semaforos/.../Controladora_Semaforos.kicad_sch):
  //
  //   U2 = MAX3485 del USART1: RO(1)->PA10, ~RE(2) y DE(3)->PA8, DI(4)->PA9. Par A/B por J10.
  //   U3 = MAX3485 del USART3, que es el de la radio LoRa: PB11, PB12, PB10. Par A/B por J12.
  //
  // Y FALTA EL MATIZ QUE IMPORTA: PA8 gobierna A LA VEZ ~RE (pin 2) y DE (pin 3) de U2.
  // Ponerlo HIGH apaga el RECEPTOR -que es lo que se busca, porque asi PA10 queda libre
  // para el modulo Bluetooth- pero deja el TRANSMISOR ENCENDIDO. Consecuencia: U2 vuelca
  // la telemetria por J10 de forma permanente y esa linea NO puede recibir nunca. Hoy es
  // inofensivo porque J10 esta vacio; deja de serlo el dia que alguien cuelgue algo de
  // J10, y ese dia hay que tocar este codigo, no el cableado.
  //
  // Es la leccion del repetidor del 31/07/2026, ya escrita en 01_Firmware/TROUBLESHOOTING.md
  // ("RS-485 half-duplex: el control DE/RE"): si un DE/RE se queda permanentemente en alto,
  // esa linea queda bloqueada en AMBOS sentidos.
  pinMode(RS485_IN_DE_RE, OUTPUT);
  digitalWrite(RS485_IN_DE_RE, HIGH); // Apaga el receptor RO de U2 y libera PA10 al modulo Bluetooth
  SerialBT.begin(9600);
}

#include "bluetooth_enlace.inc"     // $ALARM, $EVENT, silencios de J17, hora del ESP32 y reloj

#include "bluetooth_ordenes.inc"    // procesarComando(), el despachador

static const char* obtenerNombreModo(ModoSistema m) {
  switch (m) {
    case MENU: return "MENU";
    case MODO_MANUAL: return "MANUAL";
    case MODO_AUTOMATICO: return "AUTO";
    case MODO_INTELIGENTE: return "INTELIGENTE";
    case MODO_ALCANCE: return "ALCANCE";
    case MODO_DEGRADADO: return "DEGRADADO";
    case MODO_AMBAR: return "AMBAR";
    case MODO_NOCHE: return "NOCHE";   // D-110
    default: return "DESCONOCIDO";
  }
}

// ---------------------------------------------------------------------------
// D-32 (1), 13/09 - EL AVISO DEL LIMITE DE 48 h DE ESTA PUNTA. ES EL GEMELO DEL DEL
// ESCLAVO Y SE ESCRIBE COPIANDOLO, NO INVENTANDO OTRO.
//
// QUE SE ROMPIO, medido: hasta hoy el Maestro SI avisaba antes de vencer, pero la
// comparacion vivia DENTRO de la funcion de dibujo del LCD -`(desdeSync >=
// AVISO_LIMITE_MS) ? "AVISO: LIMITE 48h" : 0`, con desdeSync como variable LOCAL-. Se fue
// con la pantalla y AVISO_LIMITE_MS se quedo con UN SOLO USO: su declaracion. Desde
// entonces esta punta se va a ambar al vencer las 48 h sin haber avisado, y ademas en
// SILENCIO: irAAmbar() de modo_degradado.cpp no emite ni $EVENT ni $ALARM.
//
// EL PLAZO NO SE INVENTA: SE CONSERVA EL QUE YA HABIA. Aqui no hay una sola constante de
// tiempo; se pregunta a modo_degradado_avisoLimite(), que es quien compara contra
// AVISO_LIMITE_MS dentro de modo_degradado.cpp. Son 44 h de las 48, o sea las ULTIMAS 4.
// ⚠️ Y NO son las mismas 8 h que el Esclavo: cada punta conserva SU plazo con el motivo
// escrito junto a su constante. Igualarlos seria una decision del responsable, no un aseo
// de este commit.
//
// LOS CUATRO GETTERS SON NUEVOS Y SON EL ESPEJO DE LOS DEL ESCLAVO. La comparacion se
// quedo en modo_degradado.cpp a proposito: traerla aqui habria sido el gemelo en otro
// fichero que este repositorio ya sabe como acaba -el dia que alguien mueva el plazo, el
// aviso seguiria saliendo con el viejo y nada lo diria-. Quien compara es quien tiene la
// constante. ⚠️ Y NO se deriva de coordinador_msDesdeUltimaSync(), que si es publica y
// seria el atajo: devuelve millis() - tUltimaSyncOk y DA LA VUELTA a los 49,7 dias. El
// porque esta escrito en el propio modo_degradado.cpp desde antes de esto.
//
// POR QUE POR FLANCO Y NO PERIODICO A SECAS: identico al Esclavo. Cada $EVENT entra en
// state.events, que addEvent() de app.js RECORTA A 30 ENTRADAS, y un periodico mas a la
// misma cadencia parte por dos la ventana de bitacora en la que el tecnico busca el
// $ALARM. Es N-73 por inundacion. Asi que en reposo esto NO PONE UN SOLO BYTE EN EL CABLE:
// por FLANCO del trio el aviso llega en el instante en que se arma, y mientras SIGUE
// armado se repite a DIAG_ENLACE_MS para el tecnico que llega despues del flanco. El peor
// segundo del presupuesto lo cuenta igual, entero y sin prorratear.
//
// MISMO ORIGEN Y MISMO FORMATO QUE EL ESCLAVO, LETRA POR LETRA, y esa es la mitad del
// valor: el tecnico compara la linea de un poste con la del otro sin traducir nada. El
// ORIGEN DEGRADADO ya existe en esta punta -lo emite modo_degradado.cpp con
// SALTO_DE_HORA_POR_ROJO-, y la app los distingue por el DETALLE.
//
// MARCADORES Y COTAS, la misma cuenta que alli y por los mismos motivos: "--" es "todavia
// no lo se" y nunca un cero de relleno -por eso se pregunta antes a
// modo_degradado_huboSync()-; sin "!" porque la antiguedad no tiene cota contra la que
// salirse y una guarda que ningun dato puede disparar es adorno (CLAUDE.md 6.2);
// syncTxt[12] acotado por TIPO -diez cifras de un unsigned long mas la "h"- porque la
// antiguedad es un contador libre que sigue creciendo pasado el limite; y el separador en
// ESPACIOS y no en comas, porque _camposNmea() de la app parte por ',' y lo de detras se
// convertiria en campos sueltos que el pintor de $EVENT no mira.
//
//   parte fija   21   "SYNC:" + " AVISO:" + " VENCIDA:"
//   SYNC         11   syncTxt[12]
//   AVISO         2   literal "SI"/"NO"
//   VENCIDA       2   literal "SI"/"NO"
//
//   21 + 15 = 36 caracteres + NUL = 37 B, que es lo que guarda det[37]. Que el $EVENT
//   ENTERO quepa luego en su payload no se afirma aqui: lo recalcula
//   esp32_07_presupuesto_bytes leyendo este snprintf y el del emisor.
// ---------------------------------------------------------------------------
static unsigned long tUltimoDiagDegradado = 0;
static int8_t diagDegradadoAnt = -1;   // -1: aun no se ha publicado ninguno

static void diagDegradadoPublicar(unsigned long ahora) {
  const bool sabeFechar = modo_degradado_huboSync();
  const bool aviso      = modo_degradado_avisoLimite();
  const bool vencida    = modo_degradado_syncVencida();

  // El trio en un solo entero para poder comparar flancos sin tres banderas estaticas.
  // No contesta a dos preguntas distintas: contesta a UNA -"que se publico la ultima
  // vez"- y por eso puede ser una sola variable.
  const int8_t ahoraEstado = (int8_t)((sabeFechar ? 4 : 0) | (aviso ? 2 : 0) | (vencida ? 1 : 0));

  // LA PRIMERA VUELTA SOLO ANOTA, Y ES EL MISMO LIMITE DECLARADO QUE LLEVA ESCRITO EL
  // PERIODICO DE D-23: al arrancar el STM32 puede no haber nadie escuchando -el ESP32 y
  // el telefono llegan despues- y una trama que nadie oye no es un instrumento. No se
  // pierde nada: si el aviso ya viene armado de un arranque anterior, la repeticion de
  // abajo lo publica a los DIAG_ENLACE_MS, que es cuando ya puede oirse.
  if (diagDegradadoAnt < 0) {
    diagDegradadoAnt = ahoraEstado;
    return;
  }

  const bool cambio = (ahoraEstado != diagDegradadoAnt);
  const bool tocaRepetir = aviso && (ahora - tUltimoDiagDegradado >= DIAG_ENLACE_MS);
  if (!cambio && !tocaRepetir) return;

  diagDegradadoAnt = ahoraEstado;
  tUltimoDiagDegradado = ahora;

  char syncTxt[12];
  if (!sabeFechar) {
    strncpy(syncTxt, "--", sizeof(syncTxt));
  } else {
    snprintf(syncTxt, sizeof(syncTxt), "%luh", modo_degradado_msDesdeSync() / 3600000UL);
  }

  char det[37];
  snprintf(det, sizeof(det), "SYNC:%s AVISO:%s VENCIDA:%s",
           syncTxt, aviso ? "SI" : "NO", vencida ? "SI" : "NO");
  bluetooth_reportarEvento("DEGRADADO", det);
}

// D-92 (SPEC_4 9.11): el origen del ambar del Poste 1, de las marcas que ya gobiernan el veto
// R-4 y la cancelacion N-152 (sin estado nuevo). Solo literales: esp32_07 lo acota por su return.
static const char* ambarOrigenTxt() {
  return modoActual_get() != MODO_AMBAR ? "" : (modo_ambar_esDeArranque() ? " AMBAR:ARRANQUE"
         : (modo_ambar_origenEsclavo() ? " AMBAR:POSTE2" : " AMBAR:APP"));
}

#include "bluetooth_bucle.inc"     // bluetooth_loop(): puerto, telemetria y diagnosticos
