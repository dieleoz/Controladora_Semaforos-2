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

// N-143: los segundos que faltan de la fase LARGA -el verde o el rojo de 3 a 15 min-, o
// SIN_CUENTA_ATRAS cuando manda el coordinador (un despeje o una transicion en curso) o
// cuando el equipo no esta en este modo.
//
// La cuenta vive en modo_automatico.cpp y no en el coordinador porque el PLAZO lo pone
// este fichero: coordinador_configurar() recibe los dos tiempos de ciclo SIN NOMBRE y los
// descarta -mirese su firma-, asi que alli solo se conoce el despeje. Reconstruirla en el
// coordinador seria una segunda copia del ciclo escrita a mano, y ademas mentiria en los
// otros dos modos que lo usan: en Manual la fase acaba cuando alguien pulsa, y en
// Inteligente el tiempo configurado es un MAXIMO, no una duracion.
int modoAutomatico_segundosRestantesFase();

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
