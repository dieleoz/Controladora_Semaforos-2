// ===== Simulaciones/puente_esp32/arnes_puente_salida.inc =====
//
// D-51 corregida / D-52 (SPEC_2 7.quater (i), cond. 4): los valores de pedirSalida(), forzarRojo() y de las
// funciones de la salida programada los manda el arnes; el bluetooth.cpp REAL compone la trama. Sin la orden
// el sustituto de forzarRojo() solo hace lo de fuera del modo (rojo total, lo que usan N3/F1) y devuelve 0.
// Ordenes (solo Maestro; en el Esclavo, n/a: ahi modo_degradado.cpp es el REAL):
//   FZR <codigo> [<pedirSalida 0/1>]   REV <0/1> (revocarEsperaSiembra)
//   SAL <motivo programar> <motivo cancelar> <salidaS> <respaldada 0/1>   (MotivoSalida = indice del enum de la SPEC)
// ARNES_D52 (lo pone compilar.ps1 cuando modo_degradado.h declara ResultadoRojoTotal): las firmas de la SPEC (g).
// Sin el, solo el sustituto de hoy (uint8_t) y las pruebas P de D-52 salen en rojo por ausencia.
// Incluido desde arnes_puente.cpp.
#if defined(PUNTA_MAESTRO)
static int fzr_codigo = -1;
static bool fzr_salida = true;
bool modo_degradado_pedirSalida() { return fzr_salida; }
#if defined(ARNES_D52)
static bool rev_valor = false;
static int sal_prog = 0, sal_canc = 3;
static uint32_t sal_s = 0;
static bool sal_resp = false;
// Los textos son los de SPEC_2 7.quater (c), en el orden del enum; los cuatro primeros son los RESULT del $ACK.
static const char* const TEXTO_SALIDA[] = {"PROGRAMADA", "REPROGRAMADA", "PROGRAMADA_SIN_RESPALDO", "CANCELADA",
  "No esta en Degradado", "Ya esta saliendo", "Falta: reloj sin poner en hora", "Ahora no coincide",
  "Salida ya vencida", "Salida antes del inicio", "En verde: repita en rojo", "No se pudo guardar la salida",
  "No hay salida programada"};
ResultadoRojoTotal modo_degradado_forzarRojo() {
  if (fzr_codigo >= 0) return (ResultadoRojoTotal)fzr_codigo;
  coordinador_forzarRojoTotal(); return RRT_OK;
}
bool modo_degradado_revocarEsperaSiembra() { return rev_valor; }
MotivoSalida modo_degradado_programarSalida(uint32_t, uint32_t) { return (MotivoSalida)sal_prog; }
MotivoSalida modo_degradado_cancelarSalida() { return (MotivoSalida)sal_canc; }
const char* modo_degradado_textoSalida(MotivoSalida m) { return TEXTO_SALIDA[(int)m]; }
uint32_t modo_degradado_salidaS() { return sal_s; }
bool modo_degradado_salidaRespaldada() { return sal_resp; }
#else
uint8_t modo_degradado_forzarRojo() {
  if (fzr_codigo >= 0) return (uint8_t)fzr_codigo;
  coordinador_forzarRojoTotal(); return 0;
}
#endif
#endif
static bool salida_orden(const char* linea) {
  const bool fzr = strncmp(linea, "FZR ", 4) == 0, rev = strncmp(linea, "REV ", 4) == 0,
             sal = strncmp(linea, "SAL ", 4) == 0;
  if (!fzr && !rev && !sal) return false;
#if defined(PUNTA_MAESTRO)
  int c = -1, s = 1;
  if (fzr) { sscanf(linea + 4, "%d %d", &c, &s); fzr_codigo = c; fzr_salida = (s != 0); }
#if defined(ARNES_D52)
  if (rev) rev_valor = atoi(linea + 4) != 0;
  if (sal) { int a = 0, b = 3, r = 0; unsigned long z = 0;
             sscanf(linea + 4, "%d %d %lu %d", &a, &b, &z, &r);
             sal_prog = a; sal_canc = b; sal_s = (uint32_t)z; sal_resp = (r != 0); }
#endif
  printf("OK %s fzr=%d salida=%d\n", linea, fzr_codigo, (int)fzr_salida);
#else
  printf("OK n/a\n");
#endif
  return true;
}
