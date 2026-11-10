// SPEC_8 6 y 9 (D-95, D-148, D-166, D-169): el DR4 de los interruptores, doblado con memoria para el arnes
// del automatico, que no compila respaldo.cpp (lo mide Validacion_Respaldo, comando BANDERAS).
// Incluido en arnes_automatico.cpp a nivel de fichero, ANTES de vigilarEnclavamiento().
//
// LOS BITS VIENEN DE LA DECISION (D-95, D-166, D-169: SPEC_8 9.4), no del fuente.
static const uint16_t FLAG_VETO_CAMARA_OFF = 0x0200;   // la C
static const uint16_t FLAG_BARRERA_FUERA = 0x0400;
static const uint16_t FLAG_VETO_D_OFF = 0x0800;
static const uint16_t FLAG_MALA1_BAJA = 0x1000;
static const uint16_t FLAG_MALA2_BAJA = 0x2000;
static uint16_t g_dr4 = 0;            // los bits, como los deja el respaldo
static bool g_perdidoAlArrancar = false;
// La excepcion de la invariante va por el NOMBRE (SPEC_8 6.3, CLAUDE.md 9): pluma arriba sin
// verde y fuera de S_FALLO solo si FLAG_BARRERA_FUERA esta puesta EN DR4, la PUSO SU ORDEN
// (SET_BARRERA_SERVICIO:OFF contestado con OK) y escribirPines() lo anoto (D-150). Nunca
// "con rojo": una bandera forzada sin la orden, o quitada con la pluma arriba, se caza.
static bool g_fueraPorOrden = false;

static void ponerBit(uint16_t bit, bool on) { g_dr4 = on ? (uint16_t)(g_dr4 | bit) : (uint16_t)(g_dr4 & ~bit); }
static uint16_t bitVetoArnes(int cam) { return cam == 0 ? FLAG_VETO_CAMARA_OFF : FLAG_VETO_D_OFF; }
void respaldo_guardarVetoCamaraOff(int cam, bool off) { ponerBit(bitVetoArnes(cam), off); }
bool respaldo_vetoCamaraOff(int cam) { return (g_dr4 & bitVetoArnes(cam)) != 0; }
void respaldo_guardarMalaBaja(int n, bool baja) { ponerBit(n == 1 ? FLAG_MALA1_BAJA : FLAG_MALA2_BAJA, baja); }
bool respaldo_malaBaja(int n) { return (g_dr4 & (n == 1 ? FLAG_MALA1_BAJA : FLAG_MALA2_BAJA)) != 0; }
void respaldo_guardarBarreraFuera(bool fuera) { ponerBit(FLAG_BARRERA_FUERA, fuera); }
bool respaldo_barreraFuera() { return (g_dr4 & FLAG_BARRERA_FUERA) != 0; }
bool respaldo_perdidoAlArrancar() { return g_perdidoAlArrancar; }

static bool excepcionBarreraFuera() {
  return (g_dr4 & FLAG_BARRERA_FUERA) != 0 && g_fueraPorOrden && semaforo_plumaFuera();
}

// D-169 (SPEC_8 9.2): la segunda excepcion por NOMBRE. Pluma arriba en rojo porque una camara MALA retiene con su
// MALAx en ARRIBA: escribirPines() lo anota (semaforo_plumaRetenidaMala()) Y el arnes recalcula la regla de la
// SPEC sin pasar por camara_malaRetienePluma(): del estado del vigilante por camara (camara_letra()) y de g_dr4.
static bool excepcionMala() {
  bool hay = false, queda = false;
  for (int i = 0; i < 2; i++) {
    if (g_dr4 & bitVetoArnes(i)) continue;
    if (camara_letra(i) == 'P' || camara_letra(i) == 'C') hay = true; else queda = true;
  }
  const bool arriba = hay && (g_dr4 & (queda ? FLAG_MALA1_BAJA : FLAG_MALA2_BAJA)) == 0;
  return arriba && semaforo_plumaRetenidaMala();
}
