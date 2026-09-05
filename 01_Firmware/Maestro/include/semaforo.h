// ===== include/semaforo.h =====
#pragma once
#include <Arduino.h>

enum EstadoSemaforo { S_ROJO, S_VERDE, S_AMARILLO, S_FALLO };

void semaforo_setup();
void semaforo_apagarTodo();
void semaforo_forzarRojo();
void semaforo_forzarVerde();
void semaforo_iniciarTransicionAVerde();
void semaforo_toggle();
void semaforo_iniciarFallo();
void semaforo_actualizar();
bool semaforo_estable();

EstadoSemaforo semaforo_estado();
const char* semaforo_nombreEstado();

// ---------------------------------------------------------------------------
// SFTY-21 — Senales visibles desde el suelo (V8.7)
//
// El operario maneja el equipo con un mando de reles desde el piso y NO VE LA
// PANTALLA: esta a 5 m, dentro del gabinete. La unica salida que si ve es el propio
// semaforo, asi que la confirmacion de que una secuencia se reconocio tiene que
// darse con las luces.
//
// Se da en DESTELLOS ROJOS CONTABLES, y no en los tres colores, por una razon de
// seguridad y no de estetica: el rojo nunca significa "pase", asi que si el operario
// cuenta mal el peor caso sigue siendo seguro. Destellar verde para confirmar algo
// seria darle a un conductor lejano un permiso que nadie le dio.
//
// NO BLOQUEAN. El watchdog es de 4 s (SFTY-1) y cuatro destellos duran 3,6 s: hacerlo
// con delay() reiniciaria la placa a mitad de la confirmacion.
//
// COMO CONVIVEN CON EL RESTO DEL FIRMWARE
// ---------------------------------------
// Mientras la senal esta activa se interceptan las ESCRITURAS A LOS PINES, no la
// logica. El coordinador y los modos siguen corriendo y actualizando su estado con
// normalidad; simplemente sus salidas se guardan en vez de escribirse, y al terminar
// la senal se vuelcan de golpe.
//
// Se hizo asi tras descartar lo obvio -ignorar las llamadas durante la senal-, que
// tenia un fallo grave: si el coordinador pide la transicion a verde y se le ignora,
// se queda esperando para siempre un semaforo_estado()==S_VERDE que nunca llega, con
// las luces congeladas y sin timeout que lo rescate. Interceptar solo los pines deja
// la maquina de estados intacta y no puede provocar ese bloqueo.
// ---------------------------------------------------------------------------

// n destellos rojos contables. Empieza APAGANDO: sobre un rojo fijo, lo que se ve es
// el hueco, y sin ese primer apagado el primer destello se confundiria con la luz
// que ya estaba encendida.
void semaforo_destellosRojos(uint8_t n);

// Ambar rapido durante ms. Es el "RECHAZADO" del mando: no se parece al ambar
// intermitente de fallo (500 ms) porque va al triple de ritmo, y asi el operario
// distingue "no te he hecho caso" de "me he ido a estado seguro".
void semaforo_ambarRapido(unsigned long ms);

// True mientras hay una senal en curso. El mando espera a que termine antes de
// ejecutar la accion: primero se confirma, despues se actua.
bool semaforo_senalEnCurso();

// Test de lámparas de 6 segundos en taller (2s Rojo -> 2s Amarillo -> 2s Verde)
void semaforo_iniciarTestLeds();
bool semaforo_testLedsEnCurso();

// ---------------------------------------------------------------------------
// N-153 - EL ESTADO DE LA PLUMA, PUBLICADO.
//
// Devuelve lo que escribirPines() escribio la ultima vez en MOTOR_TALANQUERA: true si
// quedo ARRIBA. No recalcula la condicion de SFTY-28 -eso serian dos formulas que
// alguien tendria que mantener iguales-, devuelve la bandera que la propia orden dejo
// puesta.
//
// EXISTE PARA QUE LA APP PUEDA DIBUJARLA. Con D-13 va a haber ratos de LUZ ROJA CON LA
// PLUMA ARRIBA -presencia debajo, la barrera no baja-, y un operario que hoy vea eso
// lo lee como averia. Publicarlo no depende de las camaras y cierra el hueco antes.
// ---------------------------------------------------------------------------
bool semaforo_plumaArriba();