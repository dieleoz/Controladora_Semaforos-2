// ===== include/modo_degradado.h (ESCLAVO) =====
#pragma once
#include <Arduino.h>
#include "ciclo_degradado.h"

// ---------------------------------------------------------------------------
// SFTY-21 — Modo Degradado, lado ESCLAVO.
//
// Sin radio, cada unidad decide su luz por su cuenta a partir de la hora. El
// calculo de la fase NO vive aqui: vive en ciclo_degradado.h, identico en las dos
// puntas. Dos implementaciones que "hacen lo mismo" es como se acaba con verde en
// las dos puntas, asi que este modulo solo aporta el CUANDO (condiciones de
// entrada, limite duro, transiciones) y delega el QUE en la funcion compartida.
//
// ENTRADA SIEMPRE MANUAL, NUNCA AUTOMATICA
// ----------------------------------------
// Perder el radio sigue llevando a ambar intermitente, como hasta hoy. El ambar
// dice "no estoy controlando esto, decide tu" y el conductor llega alerta; un
// verde por reloj dice "pasa tranquilo, el otro lado esta en rojo" y el conductor
// llega confiado. Sin radio, el Esclavo NO puede saber si el Maestro sigue vivo,
// asi que ese verde solo puede darlo una persona que haya verificado las dos
// puntas. Un verde equivocado es mas peligroso que un ambar ambiguo.
//
// LO QUE ESTE MODULO NO HACE
// --------------------------
// No toca la radio y no responde tramas. Si llega algo mientras esta activo, lo
// contesta main.cpp por programarRespuesta() como siempre (SFTY-17).
// ---------------------------------------------------------------------------

enum EstadoDegradado {
  DEG_INACTIVO,   // manda el Maestro (o el ambar por perdida de enlace)
  DEG_ENTRANDO,   // todo-rojo obligatorio ANTES del primer verde por reloj
  DEG_ACTIVO,     // la luz la decide el reloj
  DEG_SALIENDO,   // todo-rojo obligatorio al devolver el mando
  DEG_RENDIDO     // 48 h sin sincronizar: ambar intermitente hasta nueva sync
};

// Por que cada rechazo es un valor distinto y no un simple "no": el operario esta
// en lo alto de un poste y necesita saber QUE le falta, no que algo falla. "Sin
// hora" se arregla sincronizando desde el Maestro; "sin configuracion de ciclo"
// tambien pero con otra orden; "sync caducada" obliga a arreglar el radio. Un
// mensaje generico convierte tres averias distintas en una sola incognita.
enum RechazoDegradado {
  DEG_ACEPTADO,
  DEG_RECHAZO_SIN_HORA,       // reloj_enHora() falso: operaria sobre hora inventada
  DEG_RECHAZO_SIN_CONFIG,     // el Maestro nunca mando la duracion del ciclo
  DEG_RECHAZO_CICLO_NULO,     // la mando, pero con un verde o un despeje en cero
  DEG_RECHAZO_SIN_SYNC,       // el RTC puede estar en hora de un arranque anterior
  DEG_RECHAZO_SYNC_VENCIDA    // la ultima sincronizacion supero el limite duro
};

// --- Sincronizacion horaria: el reloj del limite duro -----------------------
// La llama main.cpp cuando aplica una terna de hora completa (CMD_HORA_S). Es el
// unico punto que reinicia la cuenta de las 48 h, y a proposito: el limite mide
// "cuanto hace que el Maestro me puso en hora", no "cuanto hace que oi algo".
void degradado_registrarSync();
bool degradado_huboSync();
unsigned long degradado_msDesdeSync();

// Latch de caducidad. Se levanta al superar el limite duro y NO se baja hasta una
// sincronizacion nueva. Es un latch y no una comparacion porque millis() da la
// vuelta a los 49,7 dias: una resta sin signo volveria a dar un numero pequeno y
// el equipo se creeria recien sincronizado tras dos meses de deriva.
bool degradado_syncVencida();

// Se acerca el limite duro. Existe para que la caida a ambar no sorprenda a nadie:
// el estado seguro no puede depender de que alguien se acuerde, pero avisar con
// margen evita que el cruce se degrade sin que hubiera falta.
bool degradado_avisoLimite();

// --- Gobierno del modo ------------------------------------------------------
// Comprueba las condiciones de entrada SIN entrar. La pantalla la usa para decir
// que falta antes de que el operario pulse, en vez de dejarle intentarlo para
// averiguarlo: quien esta subido a un poste con lluvia necesita saber si merece
// la pena antes de empezar.
RechazoDegradado degradado_comprobar();

// Devuelve DEG_ACEPTADO si el modo arranco, o el motivo del rechazo. Comprueba
// TODAS las condiciones: un reloj sin poner en hora o un ciclo desconocido harian
// operar a ciegas, y ninguna de las dos se nota mirando el semaforo.
RechazoDegradado degradado_entrar();

// Salida ordenada: pasa por todo-rojo antes de devolver el mando. La llama la
// pantalla, y tambien main.cpp cuando vuelve el radio, porque el Maestro manda.
void degradado_salir();

// --- N-20: reanudar tras un corte de energia --------------------------------
// La llama setup() UNA sola vez, al arrancar. Devuelve true si el equipo vuelve al
// Modo Degradado en lugar de al estado normal.
//
// POR QUE REANUDAR ES LO SEGURO, Y NO UNA COMODIDAD
// -------------------------------------------------
// Si el Esclavo cae a ambar tras un microcorte mientras el Maestro sigue dando verde
// por reloj, el cruce queda con una punta en AMBAR -el conductor negocia- contra otra
// en VERDE -el conductor pasa confiado-. Es el riesgo residual n.2 de SFTY-21. CAER A
// AMBAR ES LO QUE CREA EL ESCENARIO PELIGROSO; reanudar en fase es lo que lo evita.
//
// NO ES AUTORIZACION POR ADELANTADO. Se reanuda un modo que UNA PERSONA autorizo y
// cuya autorizacion SIGUE VIGENTE, y eso ultimo se comprueba entero:
//
//   - respaldo_degradadoActivo(): estaba en Degradado cuando se fue la luz
//   - reloj_enHora()            : hay base de tiempo con la que calcular la fase
//   - respaldo_hayCiclo()       : se sabe la duracion del ciclo que hay que seguir
//   - respaldo_horasDesdeSync() : por debajo de 48 h y distinta de CADUCADA
//
// Si falla cualquiera, se arranca normal Y SE BORRA EL INDICADOR, para no reintentarlo
// en cada reinicio: un equipo que reintenta una condicion que no se cumple acaba
// entrando el dia que un dato basura la haga cumplirse por accidente.
//
// Al reanudar se pasa por TODO-ROJO igual que en la entrada normal: se reutiliza
// degradado_entrar(), una sola puerta y un solo criterio.
bool degradado_reanudarTrasCorte();

// En cada vuelta del loop, SIEMPRE, tambien con el modo inactivo: el latch de
// caducidad y el limite duro tienen que correr aunque nadie mire la pantalla.
void degradado_actualizar();

// true mientras el modo es dueno de la luz (entrando, activo o saliendo).
// main.cpp lo consulta para no pelearse con el: durante ese tiempo se suspende la
// caida a ambar por silencio de radio, que es justo lo que este modo sustituye.
bool degradado_gobiernaLuz();

EstadoDegradado degradado_estado();
FaseDegradado   degradado_fase();

// Segundos hasta el proximo cambio de fase, para la cuenta atras de la pantalla.
// Sale de ciclo_degradado_restante(), no de un contador propio.
uint32_t degradado_segundosParaCambio();

const char* degradado_textoEstado();
const char* degradado_textoFase();
const char* degradado_textoRechazo(RechazoDegradado motivo);
