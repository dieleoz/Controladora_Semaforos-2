// ===== 01_Firmware/ESP32_Expansion/include/reloj_ds3231.h =====
//
// EL RELOJ. LA HORA NACE NO FIABLE.
//
// Es SFTY-18 con el bit que el hardware regala. OPTIMIZACIONES.md:72 lo dice para el
// reloj del STM32: "La regla de seguridad no es tener reloj, es saber cuando no se
// tiene [...] Un reloj sin poner en hora que se cree valido es peor que no tener
// reloj: activaria la operacion nocturna a deshora." En el STM32 eso costo inventar un
// ano marcador y comprobar que sobreviviera; el DS3231 lo trae de fabrica.
//
// EL BIT OSF -bit 7 del registro 0x0F- se pone a 1 en cuanto el oscilador se para en
// algun momento: corte con la pila agotada, primera puesta en marcha, temperatura fuera
// de rango. Un DS3231 sin pila devuelve una hora PERFECTAMENTE FORMADA y completamente
// falsa; el OSF es lo unico que distingue las dos.
//
// 🔴 Y EL BORDE QUE MAS FACIL SE CRUZA AL LEER ESTO: que el ESP32 lleve reloj NO
// arregla el Y2 de los STM32. Son dos relojes distintos. Mientras reloj_enHora() del
// STM32 devuelva false, el Modo Degradado y todo lo que cuelga de SFTY-20/21 siguen
// igual de bloqueados que hoy. Colgar el reloj del semaforo de este modulo accesorio
// es la via B del Manual 17 3.2, esta abierta como AB-4 y tiene dueno.

#ifndef RELOJ_DS3231_H
#define RELOJ_DS3231_H

#include <Arduino.h>

// Por que hay SIETE resultados y no un bool.
//
// "Los tres finales son distintos y el operario necesita los tres distintos: no hay con
// que contar el tiempo; la hora no entro; la hora entro y va camino del Esclavo"
// (Maestro/src/bluetooth.cpp:305). Aqui son mas de tres y siguen teniendo que ser
// distintos: un tecnico que recibe "no se pudo" no sabe si cambiar la pila, revisar el
// cableado del I2C o repetir el comando bien escrito.
enum ResultadoReloj {
  RELOJ_OK = 0,
  RELOJ_ERR_FORMATO,          // el sscanf no devolvio 6 campos
  RELOJ_ERR_RANGO,            // algun campo fuera de rango (validado POR BARRIDO)
  RELOJ_ERR_SIN_RELOJ,        // el bus no contesta: modulo ausente, SDA/SCL cruzados
  RELOJ_ERR_ESCRITURA,        // la escritura I2C fallo a mitad
  RELOJ_ERR_NO_QUEDO_PUESTA,  // la relectura no coincide con lo escrito
  RELOJ_ERR_OSF_SIGUE         // el OSF sigue puesto tras escribir: pila agotada
};

// Los tres motivos por los que la hora no es fiable, que NO son el mismo problema.
enum MotivoSinHora {
  SIN_HORA_NINGUNO = 0,       // hay hora fiable
  SIN_HORA_NUNCA_SE_PUSO,
  SIN_HORA_OSCILADOR_PARADO,  // OSF == 1
  SIN_HORA_BUS_MUDO           // el I2C no responde
};

struct FechaHora {
  int anio, mes, dia, hora, minuto, segundo;
};

// R-1: el OSF se lee EN EL ARRANQUE, antes de publicar ninguna hora.
void reloj_setup();

// R-4: relee periodicamente. La pila se puede agotar con el equipo en marcha, y una
// hora que dejo de ser fiable a las tres de la manana no avisa sola.
void reloj_revisar();

// LA BARRERA. Nace en false y toda ruta que use la hora la consulta primero.
//
//   false  si  (nunca se puso) || (OSF == 1) || (el I2C no contesta)
//
// N-73: una funcion "tengo hora?" declarada, documentada y SIN UN SOLO LLAMADOR es la
// Caja Negra de Alarmas otra vez. El censo de llamadores es parte de escribirla, y hay
// un pack que lo hace.
bool reloj_enHora();
MotivoSinHora reloj_motivo();

// Copia la hora SOLO si es fiable. Devuelve false en caso contrario, y esa es toda la
// barrera: no hay una variante "damela igual".
bool reloj_leer(FechaHora* fh);

// Ajusta el reloj y DEVUELVE QUE PASO.
//
// 🔴 Este bool es la razon de existir de este fichero. El defecto que se cerro el 28/08
// en el STM32 (N-80) fue una rama SET_RTC que llamaba a reloj_ajustar() y mandaba
// RESULT:OK SIN MIRAR LO QUE DEVOLVIO. Con Y2 muerto en hardware, ese era el caso
// NORMAL: el tecnico se iba del poste creyendo que dejo el reloj puesto.
//
// Ese defecto no se arregla mudandose de micro; se muda con el si nadie lo escribe.
ResultadoReloj reloj_ajustar(const FechaHora* fh);

// R-6/R-7: valida ANTES de escribir, con la trama entera en la mano, BARRIENDO los seis
// campos. Comprobar "la hora" y dar por buenos los minutos es exactamente PESOS_SUMA
// (N-51): un numero que parece cubrir todos los casos sin haber evaluado ninguno.
bool reloj_rangoValido(const FechaHora* fh);

#endif // RELOJ_DS3231_H
