// Dobles de D-35 (testigo), incluidos desde arnes_puente.cpp.
// D-35: la rama SET_MODO:DEG_T de bluetooth.cpp. Doble minimo: este simulador no ejerce el
// testigo (se valida en banco); rechaza siempre por hora, que es lo que hace el firmware
// sin fecha del DS3231.
MotivoTestigo modo_degradado_entrarTestigo(uint32_t, uint32_t, int) { return MDT_FALTA_HORA; }
const char* modo_degradado_textoTestigo(MotivoTestigo) { return "Falta: reloj sin poner en hora"; }

