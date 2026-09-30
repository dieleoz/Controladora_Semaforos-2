// ===== Simulaciones/puente_esp32/arnes_puente_deg_auto.inc =====
//
// A-15 (SPEC_2 7.ter, a0d605b): dobles del Degradado automatico, incluidos desde
// arnes_puente_testigo2.inc (que arnes_puente.cpp incluye en las dos puntas). Sin ellos los
// dos arneses dejaron de ENLAZAR: bluetooth.cpp gano SET_DEG_AUTO, CONSULTA_DEG_AUTO y el
// $ACK diferido, y coordinador.cpp del Maestro envia por degAuto_enviar().
//
// MOLDE: arnes_puente_testigo1.inc y Validacion_Automatico/deg_auto_doble_maestro.cpp. Dobles
// minimos y sin memoria: la opcion esta APAGADA, nada es APTO, SET_DEG_AUTO se rechaza como
// sin enlace y no hay veredicto diferido que publicar. Con eso el param de PING/GO_* sale como
// antes (sin APTO ni ECO) y PRESENTE no se filtra. Ninguna fila de este simulador ejerce A-15:
// QUE NO SE CUENTE COMO CUBIERTO. Lo ejerce el bloque H de orquestador_degradado.cpp.
#include "deg_auto.h"

bool respaldo_degAuto() { return false; }
bool respaldo_otroApto() { return false; }
bool degAuto_aptoPropio() { return false; }
DegAutoOrden degAuto_orden(bool) { return DAO_SIN_ENLACE; }
DegAutoVeredicto degAuto_veredicto() { return DAV_NINGUNO; }

#if defined(PUNTA_MAESTRO)
void degAuto_enviar(uint8_t cmd, uint8_t param) { protocolo_enviarPaquete(cmd, param); }
bool degAuto_alRecibir(const RF_Packet*) { return false; }
#endif
