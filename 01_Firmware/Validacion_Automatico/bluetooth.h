// ===== Validacion_Automatico/bluetooth.h =====
// Sustituto de bluetooth.h para compilar botones.cpp en el PC.
//
// SOLO LAS DOS PUERTAS QUE botones.cpp USA, y no una copia recortada del header real:
// el de verdad arrastra el protocolo, el reloj, el respaldo y el $STATUS entero, nada de
// lo cual hace falta para medir si el vigilante alarma cuando debe. Si algun dia
// botones.cpp empezara a llamar a una tercera funcion de Bluetooth, esto NO compilaria
// -que es justo lo que se quiere: un arnes que se entera de que el fichero que mide ha
// cambiado de superficie, en vez de uno que la ignora en silencio-.
//
// Las definiciones viven en arnes_automatico.cpp y GUARDAN lo ultimo reportado: es N-73:
// un stub que se limitase a callar dejaria al arnes sin poder distinguir "no alarmo"
// de "alarmo y nadie miro", que es la diferencia entre las cuatro inyecciones de D-13.
#pragma once

void bluetooth_reportarAlarma(const char* evento, const char* causa, const char* accion);
void bluetooth_reportarEvento(const char* tipo, const char* detalle);
