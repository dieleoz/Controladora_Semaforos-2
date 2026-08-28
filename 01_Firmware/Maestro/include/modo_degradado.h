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
  MDG_FALTA_HORA,     // reloj_enHora() falso: SFTY-18
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

// El ambar intermitente vive en modo_ambar.h desde la Fase 4 (03/08/2026): es un MODO
// DEL SISTEMA -la salida de emergencia de B.B.B-, no una parte del Modo Degradado.

