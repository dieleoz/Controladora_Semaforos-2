// ===== dos_puntas/adaptador_esclavo_instalacion.inc =====
// Lo incluye adaptador_esclavo.cpp donde vivian sus dobles de botones.cpp.
//
// CON ARNES_RELOJ_REAL (compilar_degradado.ps1): los dobles de siempre. NO HAY CAMARAS, y
// es la respuesta correcta para un equipo con las borneras de J16 vacias -el pull-down de
// 10K deja los pines bajos-; ese arnes mide el Degradado, no el veto.
//
// SIN EL (compilar_dos_puntas.ps1, N-187): botones.cpp REAL del Esclavo, con la pagina de
// instalacion modelada con memoria. Asi el bloque I del orquestador ejerce en el Esclavo lo
// que el bloque K de arnes_automatico.cpp ejerce en el Maestro: camara_leerPin() con su NC,
// camaras_sembrar(), la presencia que vetan semaforo.cpp y la demanda, y las ordenes por el
// bluetooth.cpp REAL. Con los pines de camara bajos y la pagina borrada (lo que encuentra
// cualquier otro bloque) presencia es false siempre, como el doble de antes.
// UNA EXCEPCION, escrita: camara_estado() sigue siendo el literal de
// adaptador_esclavo_interruptores.inc (el bloque S elige su valor); el de botones.cpp
// queda renombrado y nadie lo publica en este arnes.
// Lo que hay en cada entrada AL ENCENDER (punta_arrancar): un puente o una camara siguen
// cableados durante el corte de luz. Cero (todo bajo) salvo que el bloque I lo ponga entre
// la carga de la DLL y el arranque ("entrada_al_arrancar", arg = pin << 1 | nivel).
static int g_entradaAlArrancar[64];
#ifdef ARNES_RELOJ_REAL
void botones_setup() {}
void botones_actualizar() {}
bool camara_presenciaJ16(int) { return false; }
bool camara_mira(int) { return true; }   // D-166/D-169: sin camaras ninguna esta inhibida ni mala
bool camara_malaRetienePluma() { return false; }
char camara_letra(int) { return '?'; }
#include "../instalacion_dobles.inc"
bool camara_leerPin(uint8_t pin, bool nc) { return (digitalRead(pin) == HIGH) != nc; }  // N-187: firma real
static long arnesInstEsc_mando(const char*, long) { return PUNTA_DESCONOCIDO; }
#else
// Los pines de J16 que botones.cpp nombra y el sustituto comun de pines.h no tiene. Fuera
// de 0..15: ni luces, ni RS485_IN_DE_RE (13), ni PB7/PB6 (14/15) del bluetooth.
#define CAM_C_PIN 20
#define CAM_D_PIN 21
#define J16_P5_SIN_USO 22
#define J16_P8_SIN_USO 23
#include "instalacion_mandos.inc"
#define camara_estado camara_estado_botonesReal
#include "../../Esclavo/src/botones.cpp"   // NOLINT: el fuente REAL, sin tocar
#undef camara_estado

static long arnesInstEsc_mando(const char* que, long arg) {
  if (!strcmp(que, "cam_pin"))     return arg == 0 ? CAM_C_PIN : CAM_D_PIN;
  if (!strcmp(que, "presencia"))   return camara_presenciaJ16((int)arg) ? 1 : 0;
  if (!strcmp(que, "cmd_demanda")) return (long)CMD_DEMANDA;
  if (!strcmp(que, "entrada_al_arrancar")) { g_entradaAlArrancar[(arg >> 1) & 63] = (int)(arg & 1); return 1; }
  return arnesInstalacion_mando(que, arg);
}
#endif
