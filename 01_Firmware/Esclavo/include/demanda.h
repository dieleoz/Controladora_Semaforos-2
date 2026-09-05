// ===== include/demanda.h =====
#pragma once
#include <Arduino.h>

// LA UNICA PUERTA POR LA QUE SALE UNA DEMANDA DE ESTA PUNTA.
//
// Hay dos origenes -la camara de PB0 y el boton "solicitar paso" de la app- y ambos
// significan lo mismo: hay demanda en este lado. Si cada uno llevase su propio
// temporizador, dos origenes a la vez saturarian el aire con la misma peticion y el
// limite de ritmo de uno no sabria nada del otro.
//
// POR QUE VIVE AQUI Y NO EN protocolo.cpp, que fue el primer sitio donde se puso:
// protocolo.h y protocolo.cpp son CONTRATO COMPARTIDO y deben ser identicos byte a
// byte en las dos puntas -costura_01_contratos lo exige, y cazo este error-. El
// formato de aire lo acuerdan los dos extremos; la ventana de silencio con la que
// ESTA punta decide cuando pedir es politica local, y el Maestro no la necesita.
//
// EL ESCLAVO PIDE; NO ORDENA. Nada de aqui enciende una luz: se manda CMD_DEMANDA y
// el Maestro decide, aplica el todo-rojo y ordena. Ver OPTIMIZACIONES.md SFTY-27.

// Devuelve false si la peticion se descarto por caer dentro de la ventana de silencio,
// para que el llamante pueda decirselo al operario en vez de fingir que se envio.
bool demanda_solicitar();

// ---------------------------------------------------------------------------
// LA VENTANA DE SILENCIO, PUBLICADA. D-13 fase 1, 05/09/2026.
//
// ES UN GETTER, NO UNA SEGUNDA DECLARACION. El numero sigue viviendo -solo- en
// demanda.cpp y no se copia a ningun sitio. El motivo es N-137: cuando un plazo vive
// en una constante Y ADEMAS escrito a mano en otro fichero, el dia que difieran gana
// el que NO lleva encima el comentario que lo explica.
//
// QUIEN LA NECESITA Y PARA QUE. El vigilante de camaras de botones.cpp cuenta cuantas
// veces HABRIA actuado el veto de la pluma de la fase 2, y para eso tiene que decidir
// si una deteccion "sigue en pie" en el instante en que la pluma baja. Esa pregunta ya
// la contesta este plazo -es el que gobierna demanda_solicitar()- y se reutiliza en vez
// de inventar un segundo: dos definiciones de "vigente" que solo la disciplina mantiene
// iguales son el defecto que este repositorio ya paga.
unsigned long demanda_ventanaMs();
