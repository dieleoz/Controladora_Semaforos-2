// ===== include/protocolo.h =====
#pragma once
#include <Arduino.h>

void protocolo_setup();
void protocolo_enviarLinea(const char* linea);
bool protocolo_hayLineaDisponible(char* destino, size_t maxLen);