// ===== include/modo_automatico.h =====
#pragma once
#include <stdint.h>

void modoAutomatico_setup();
void modoAutomatico_loop();

// SFTY-21: arranque sin asistente, para la secuencia A.A.A del mando de reles.
//
// El asistente pide tres confirmaciones en pantalla (rojo, verde y despeje) y desde el
// suelo NO HAY PANTALLA QUE RELLENAR: el operario esta a 5 m y solo puede dar pulsos.
// Un A.A.A que dejara el equipo esperando en la primera pregunta no arrancaria nada, y
// el operario, viendo las luces quietas, concluiria que el radio sigue muerto cuando
// en realidad nadie llego a intentarlo.
//
// Se arranca con los ULTIMOS valores configurados, o con los de fabrica si nadie los
// cambio desde el encendido. Marca la intencion; la aplica el siguiente
// modoAutomatico_setup().
void modoAutomatico_pedirArranqueDirecto();

// N-69: fijar los tiempos del ciclo desde fuera del asistente -hoy, desde el Bluetooth-.
//
// Los limites viven en el .cpp y son DUROS: quien decide si un valor es aceptable es el
// firmware, no la app. Una interfaz bien educada que valide rangos es comodidad; la
// garantia es que el C++ rechace lo que no cabe, porque la app se puede reemplazar por
// otra, por una vieja o por alguien mandando tramas a mano.
//
// Devuelve false y NO cambia nada si algun valor esta fuera de rango.
bool modoAutomatico_fijarTiempos(uint8_t verdeMin, uint8_t rojoMin, uint8_t despejeSeg);

// True mientras el ciclo esta corriendo. Se consulta antes de aceptar tiempos nuevos:
// cambiarlos a mitad de ciclo podria ACORTAR el todo-rojo que ya esta en curso, y ese
// es justo el tiempo que garantiza que el tramo quedo vacio.
bool modoAutomatico_enMarcha();
