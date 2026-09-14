// ===== include/modo_degradado.h =====
#pragma once
#include <Arduino.h>

// ---------------------------------------------------------------------------
// SFTY-21 — MODO DEGRADADO (V8.7)
//
// Sin radio, cada unidad decide su luz POR SU CUENTA usando el reloj. Las dos
// calculan la fase con la MISMA funcion compartida, ciclo_degradado_fase() de
// include/ciclo_degradado.h, que es identica en Maestro y Esclavo. No se reimplementa
// el calculo aqui ni en ningun otro sitio: dos implementaciones que "hacen lo mismo"
// es como se acaba con verde en las dos puntas.
//
// ACTIVACION SIEMPRE MANUAL, NUNCA AUTOMATICA. La razon esta en la asimetria de los
// dos avisos:
//
//    AMBAR INTERMITENTE  ->  "no estoy controlando esto, decide tu"
//                            el conductor llega ALERTA, mira y negocia el paso
//    VERDE POR RELOJ     ->  "pasa tranquilo, el otro lado esta en rojo"
//                            el conductor llega CONFIADO y no mira
//
// Sin radio el Maestro no puede saber si el Esclavo sigue vivo: podria estar apagado,
// colgado o haber sido movido. Un verde equivocado es MAS peligroso que un ambar
// ambiguo, porque le quita al conductor la precaucion que el ambar le provoca. Por
// eso el verde no se da por suposicion: se da porque una persona verifico las dos
// puntas y lo habilito.
// ---------------------------------------------------------------------------

// Motivo por el que la entrada se rechaza. MDG_OK significa que se cumplen TODAS las
// condiciones; no hay ninguna que sea opcional o "recomendable".
enum MotivoDegradado {
  MDG_OK,
  MDG_FALTA_HORA,     // reloj_horaFiable() falso: sin hora, o siembra caducada (D-21 (1))
  MDG_NUNCA_SYNC,     // jamas se confirmo una sincronizacion con el Esclavo
  MDG_SYNC_VIEJA,     // la hubo, pero es demasiado antigua para garantizar nada
  MDG_SIN_DESFASE,    // no hay medida de desfase utilizable (SFTY-23)
  MDG_DESFASE_ALTO,   // la hay, y se sale de tolerancia

  // El Esclavo no ha acusado el ciclo, asi que no se sabe con que duraciones
  // calculara su fase. Faltaba, y el Esclavo SI lo comprobaba por su lado: el
  // Maestro aceptaba y daba verde mientras el otro rechazaba y caia a ambar.
  MDG_SIN_CONFIG
};

// Evalua la puerta de entrada. No cambia nada: solo mira.
// Sirve tanto a la entrada por pantalla como a la secuencia A.B.A.B del mando, para
// que las dos vias apliquen EXACTAMENTE el mismo criterio. Tener dos puertas con dos
// criterios distintos seria tener una sola puerta, la mas floja.
MotivoDegradado modo_degradado_evaluarEntrada();

// Textos de pantalla del motivo, en dos lineas de 20 caracteres como maximo.
const char* modo_degradado_motivoL1(MotivoDegradado m);
const char* modo_degradado_motivoL2(MotivoDegradado m);

// Encola por radio la configuracion del ciclo degradado (CMD_CONFIG de SFTY-23).
// Dos relojes en hora dan tiempo comun, pero para ir EN FASE ambas puntas deben
// computar el mismo horario. Se llama al arrancar, mientras el enlace vive: cuando
// el radio muera ya sera tarde para acordarlo.
void modo_degradado_publicarConfig();

// --- El reloj del limite de 48 h, para quien lo tenga que PUBLICAR ----------
//
// D-32 (1), 13/09 - LOS CUATRO GETTERS QUE FALTABAN, Y POR QUE FALTABAN.
//
// Hasta el 13/09 esta punta SI avisaba antes de vencer el limite: el bucle componia
// `const char* aviso = (desdeSync >= AVISO_LIMITE_MS) ? "AVISO: LIMITE 48h" : 0;` y se
// lo pasaba a lcd_dibujarDegradado(). PERO ESO VIVIA DENTRO DE LA FUNCION DE DIBUJO, con
// `desdeSync` como variable LOCAL, asi que al retirar el LCD el aviso se fue entero y
// AVISO_LIMITE_MS se quedo con UN SOLO USO: su declaracion. El Maestro se va hoy a ambar
// al vencer las 48 h sin haber avisado antes -irAAmbar() no emite $EVENT ni $ALARM-, y
// nadie puede verlo venir porque no hay pantalla y el dato no sale por el cable.
//
// ESTOS CUATRO NO SON TELEMETRIA NUEVA: son la MISMA cuenta que el modo ya hacia para si
// mismo, expuesta. El calculo no se mueve de aqui y no se duplica en ningun sitio.
//
// 🔴 Y POR ESO LA COMPARACION VIVE EN EL .cpp Y NO EN QUIEN PREGUNTA. Podria haberse
// publicado solo la antiguedad y dejar que bluetooth.cpp comparase, y habria sido el
// gemelo en otro fichero que este repositorio ya sabe como acaba: el dia que alguien
// mueva AVISO_LIMITE_MS, el aviso seguiria saliendo con el plazo viejo y nada lo diria.
// Quien compara es quien tiene la constante.
//
// SON EL ESPEJO EXACTO DE LOS DEL ESCLAVO -degradado_huboSync(), degradado_msDesdeSync(),
// degradado_avisoLimite() y degradado_syncVencida()- a proposito, para que las dos puntas
// publiquen la MISMA linea y el tecnico compare un poste con el otro sin traducir nada.
// Lo que NO es igual es el plazo, y tampoco se unifica aqui: alli son 40 h de 48 -las
// ultimas 8- y aqui 44 de 48 -las ultimas 4-, cada uno con su motivo escrito junto a su
// constante. Igualarlos seria una decision del responsable, no un aseo.
//
// NINGUNO ES UNA GUARDA: son de solo lectura y no vetan nada. Lo que decide sigue
// decidiendo dentro de modo_degradado.cpp -la puerta de entrada y el limite duro del
// bucle-, y por eso dejarlos sin lector no abriria un veto; lo unico que se perderia
// otra vez es el aviso.

// false = jamas se pudo fechar una sincronizacion, ni por RAM ni por la marca de la
// pila. Quien publique tiene que preguntar ESTO antes de creerse el numero de abajo:
// publicar una antiguedad sin fecha detras es inventarse un dato.
bool modo_degradado_huboSync();

// Milisegundos desde la ultima sincronizacion, por el mismo camino que usa la puerta de
// entrada: la medida de RAM contrastada contra el reloj de pared de la pila, tomando el
// MAYOR de los dos. NO es coordinador_msDesdeUltimaSync(), que da la vuelta a los 49,7
// dias -el porque entero esta en msDesdeSyncEfectivo()-. Solo tiene sentido si
// modo_degradado_huboSync() dice que si.
unsigned long modo_degradado_msDesdeSync();

// Se acerca el limite duro: quedan AVISO_LIMITE_MS o menos. Existe para que la caida a
// ambar no sorprenda a nadie -el estado seguro no puede depender de que alguien se
// acuerde, pero avisar con margen evita que el cruce se degrade sin que hubiera falta-.
bool modo_degradado_avisoLimite();

// El limite duro YA esta agotado. Es el mismo borde que el bucle aplica para rendirse,
// preguntado desde fuera; no es un latch como el del Esclavo y no le hace falta serlo,
// porque aqui la antiguedad se contrasta contra el reloj de pared y no se desborda.
bool modo_degradado_syncVencida();

// --- Reanudacion tras un corte de energia (N-20) ---------------------------
//
// Se llama UNA vez en el arranque. Devuelve true si el equipo estaba en Modo
// Degradado cuando se fue la luz Y la autorizacion que lo permitio SIGUE VIGENTE,
// en cuyo caso quien llama debe arrancar en MODO_DEGRADADO en vez de en el menu.
//
// POR QUE REANUDAR ES LO SEGURO. Si esta unidad cae a ambar tras un microcorte
// mientras la otra sigue dando verde por reloj, queda una punta en AMBAR -el
// conductor negocia- contra otra en VERDE -el conductor pasa confiado-. Ese es el
// riesgo residual n.2 de SFTY-21. Caer a ambar es lo que CREA el escenario
// peligroso; reanudar en fase lo evita.
//
// Y NO ES AUTORIZACION POR ADELANTADO. No se guarda "si algun dia pierdes el radio,
// entra": se reanuda un modo que UNA PERSONA autorizo antes, y solo mientras esa
// autorizacion siga siendo valida.
//
// TIENE EFECTO: si alguna condicion falla, borra el indicador de la pila antes de
// devolver false, para no reintentar la reanudacion en cada reinicio con una
// autorizacion que ya caduco.
bool modo_degradado_reanudarTrasCorte();

void modo_degradado_setup();
void modo_degradado_loop();

// Pide la salida del modo: arranca el todo-rojo obligatorio y, cumplido, devuelve el
// mando al menu. Es la MISMA puerta que el boton 4 del gabinete, y esa es la razon de
// que exista como funcion: quien quiera sacar al equipo de aqui -la pantalla o el
// Bluetooth- tiene que pasar por ella.
//
// NADIE SALE DE ESTE MODO CON UN modoActual_set(MENU). Saltar al menu directamente se
// come el todo-rojo de ROJO_TRANSICION_MS, y el escenario peligroso de este modo es
// justo que UNA SOLA PUNTA lo abandone: esta unidad caeria a rojo mientras la otra
// sigue dando verde por reloj, sin nadie que haya verificado las dos puntas.
//
// Devuelve false si no habia salida que arrancar -ya se estaba saliendo, o esta puesta
// la pantalla de rechazo, que vuelve sola al menu-. En los dos casos el equipo ya esta
// en rojo y camino del menu, asi que false no es un error: es "no hice nada nuevo".
bool modo_degradado_pedirSalida();

// El ambar intermitente vive en modo_ambar.h desde la Fase 4 (03/08/2026): es un MODO
// DEL SISTEMA -la salida de emergencia de B.B.B-, no una parte del Modo Degradado.

