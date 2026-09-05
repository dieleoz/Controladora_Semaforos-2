#pragma once
// ===== include/limites_ciclo.h =====
//
// LOS LIMITES DEL CICLO VIVEN AQUI, Y EN UN SOLO SITIO. N-137 (04/09/2026).
//
// EL PORQUE, con las palabras del responsable: TRES MINUTOS ES LA MINIMA DISTANCIA DE
// SEGURIDAD. En un paso alternado de un solo carril, un camion pesado tarda entre 5 y 8 s
// solo en reaccionar y arrancar; con un verde de 60 s pasan tres o cuatro vehiculos antes
// de cortar a ambar. Lo que se produce no es una cola: es un conductor convencido de que
// el semaforo esta averiado, adelantando en rojo contra el sentido que acaba de recibir
// verde. El limite de 1 minuto era un valor de MESA DE PRUEBAS que se quedo abierto para
// la operacion en via.
//
// Y EL DESPEJE es el unico de los tres que es seguridad vial pura: es el tiempo que
// garantiza que el tramo quedo VACIO antes de dar verde al otro lado. Su minimo no sale
// de un numero redondo -10 s es lo que tarda en despejarse el tramo mas corto que esta
// casa ha montado-, y por debajo de eso el margen desaparece.
//
// -------------------------------------------------------------------------------------
// POR QUE ESTE FICHERO EXISTE, que es una leccion que costo TRES veces el mismo dia
// -------------------------------------------------------------------------------------
//
// Estas seis constantes vivian `static` dentro de modo_automatico.cpp, o sea INVISIBLES
// para el resto del firmware. El 04/09 se subio el minimo de 1 a 3 minutos ahi, y en las
// horas siguientes aparecieron TRES agujeros distintos por el mismo motivo -otro fichero
// escribia tiempos de ciclo a mano sin poder ver el limite-:
//
//   N-131  el propio modo_automatico.cpp los repetia en CINCO sitios mas: el
//          inicializador, el reset del setup() y los tres topes del menu, con pisos de
//          1 min, 1 min y 5 s -la MITAD del minimo vial del despeje-.
//   N-133  js/config.js de la app declaraba una CUARTA copia, con VERDE_MIN_MIN: 1, bajo
//          el rotulo "Rangos de Tiempos Permitidos por Firmware", y no la leia NADIE.
//   N-137  modo_inteligente.cpp configuraba el coordinador con `maxVerde = 2` MINUTOS,
//          por debajo del minimo, sin pasar por ninguna guarda. Y era el modo que la
//          guia de banco recomendaba como salida mientras el Automatico estuvo roto.
//
// Ninguna de esas copias llevaba encima el comentario de seguridad. Ese es el patron y
// por eso el fichero existe: cuando un minimo vital vive en una constante Y ADEMAS
// escrito a mano en otro sitio, el dia que difieran gana el que NO lleva el aviso.
//
// La guarda de SET_TIEMPOS es la que rechaza con $ERR,CMD:SET_TIEMPOS,DESC:RANGO, y sigue
// donde estaba. Esto no la sustituye: le quita los caminos por los que se la rodeaba.
//
// COSTE DECLARADO Y ACEPTADO A SABIENDAS: ya no se puede probar en mesa con ciclos de un
// minuto. Un banco cae del lado de esperar tres minutos, no del lado de dejar el limite
// de laboratorio suelto en una carretera.
//
// app_11_rangos_de_tiempos cruza estos seis numeros contra la validacion de la app y los
// min/max del HTML en cada corrida, y censa que ningun .cpp del Maestro escriba un tiempo
// de ciclo fuera de rango.

#include <stdint.h>

static const uint8_t VERDE_MIN_MIN = 3,  VERDE_MIN_MAX = 15;
static const uint8_t ROJO_MIN_MIN  = 3,  ROJO_MIN_MAX  = 15;
static const uint8_t DESPEJE_SEG_MIN = 10, DESPEJE_SEG_MAX = 90;
