// ===== include/demanda.h =====
#pragma once
#include <Arduino.h>

// LA UNICA PUERTA POR LA QUE ENTRA UNA DEMANDA PEDIDA A MANO EN ESTA PUNTA.
//
// Es la copia simetrica de Esclavo/include/demanda.h, y la simetria se rompe justo
// donde SFTY-27 dice que se rompe: EL ESCLAVO PIDE Y EL MAESTRO DECIDE. Alli
// demanda_solicitar() manda CMD_DEMANDA por radio porque el que decide es el otro;
// aqui no sale nada al aire, porque el que decide es este.
//
// Lo que hace es levantar el MISMO bit que levantaria la camara de PB0, para que
// modo_inteligente.cpp lo trate exactamente igual: con su minimo de verde y con su tope
// de verde maximo.
//
// POR QUE NO LLAMA A coordinador_pedirCambio(). Esa funcion se salta los dos, y ademas
// falla en silencio -su primera linea es "if (estadoC != C_IDLE) return;"-. Un
// RESULT:OK encima de eso le diria al operario que se hizo algo que no se hizo.

// Devuelve false si la peticion se descarto por caer dentro de la ventana de silencio,
// para que el llamante pueda decirselo al operario en vez de fingir que se registro.
bool demanda_solicitar();

// True mientras la ultima demanda pedida a mano sigue en pie.
//
// CADUCA SOLA, y dura lo mismo que la ventana de silencio. Un bit que se quedara
// encendido seria una demanda local permanente, y la regla de modo_inteligente.cpp es
// "cede el paso si hay demanda remota Y NO hay cola local": con la cola local siempre
// puesta, el Maestro no le cederia el paso al otro sentido nunca mas.
bool demanda_hayLocal();
