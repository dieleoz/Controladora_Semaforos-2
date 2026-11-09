// SPEC_8 6 (D-95, D-148): el DR4 de los dos interruptores, doblado con memoria para el arnes
// del automatico, que no compila respaldo.cpp (lo mide Validacion_Respaldo, comando BANDERAS).
// Incluido en arnes_automatico.cpp a nivel de fichero, ANTES de vigilarEnclavamiento().
//
// LOS BITS VIENEN DE LA DECISION (D-95), no del fuente: 0x0200 y 0x0400.
static const uint16_t FLAG_VETO_CAMARA_OFF = 0x0200;
static const uint16_t FLAG_BARRERA_FUERA = 0x0400;
static uint16_t g_dr4 = 0;            // los dos bits, como los deja el respaldo
static bool g_perdidoAlArrancar = false;
// La excepcion de la invariante va por el NOMBRE (SPEC_8 6.3, CLAUDE.md 9): pluma arriba sin
// verde y fuera de S_FALLO solo si FLAG_BARRERA_FUERA esta puesta EN DR4, la PUSO SU ORDEN
// (SET_BARRERA_SERVICIO:OFF contestado con OK) y escribirPines() lo anoto (D-150). Nunca
// "con rojo": una bandera forzada sin la orden, o quitada con la pluma arriba, se caza.
static bool g_fueraPorOrden = false;

void respaldo_guardarVetoCamaraOff(bool off) {
  g_dr4 = off ? (uint16_t)(g_dr4 | FLAG_VETO_CAMARA_OFF) : (uint16_t)(g_dr4 & ~FLAG_VETO_CAMARA_OFF);
}
bool respaldo_vetoCamaraOff() { return (g_dr4 & FLAG_VETO_CAMARA_OFF) != 0; }
void respaldo_guardarBarreraFuera(bool fuera) {
  g_dr4 = fuera ? (uint16_t)(g_dr4 | FLAG_BARRERA_FUERA) : (uint16_t)(g_dr4 & ~FLAG_BARRERA_FUERA);
}
bool respaldo_barreraFuera() { return (g_dr4 & FLAG_BARRERA_FUERA) != 0; }
bool respaldo_perdidoAlArrancar() { return g_perdidoAlArrancar; }

static bool excepcionBarreraFuera() {
  return (g_dr4 & FLAG_BARRERA_FUERA) != 0 && g_fueraPorOrden && semaforo_plumaFuera();
}
