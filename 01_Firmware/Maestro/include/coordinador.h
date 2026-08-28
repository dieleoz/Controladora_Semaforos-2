// ===== include/coordinador.h =====
#pragma once
#include <Arduino.h>

void coordinador_setup();
void coordinador_reiniciarConexion();
void coordinador_forzarMenu();
void coordinador_forzarRojoTotal();
void coordinador_iniciarModo();
bool coordinador_intentarHandshake();
void coordinador_configurar(unsigned long tiempoEstaticoMs, unsigned long minRojoMs, unsigned long minVerdeMs);
void coordinador_pedirCambio();
void coordinador_actualizar();
void coordinador_actualizar_background();
bool coordinador_listoParaContar();
bool coordinador_comunicacionPerdida();
void coordinador_reiniciarConexion();
const char* coordinador_nombreEstadoMaster();

// --- Demanda vehicular de cámaras IA AcuSense (Sentido 2 / Esclavo) ---
bool coordinador_hayDemandaRemota();
void coordinador_limpiarDemandaRemota();

// ---------------------------------------------------------------------------
// Telemetria de calidad de enlace (V8.1)
//
// Se deriva del latido que el Maestro ya emite cada 3 s (PING, o GO_RED cuando
// esta en Menu o en Fallo). No requiere soporte de la radio ni cambios de
// protocolo: solo se mide si llego respuesta y cuanto tardo.
//
// Util para la prueba de alcance en campo: el operario mueve el equipo y ve
// degradarse el porcentaje hasta que se pierde el enlace.
// ---------------------------------------------------------------------------

// Porcentaje de latidos respondidos en la ventana de las ultimas 10 emisiones.
// Devuelve -1 mientras no haya ninguna muestra todavia.
int coordinador_calidadEnlace();

// Tiempo de respuesta suavizado del ultimo latido, en ms. 0 si no hay muestras.
unsigned long coordinador_tiempoRespuestaMs();

// Latidos consecutivos sin respuesta. 0 con el enlace sano.
int coordinador_latidosSinRespuesta();

// ---------------------------------------------------------------------------
// SFTY-23: Sincronizacion horaria por radio (lado MAESTRO)
//
// La hora se cuadra UNA sola vez en el Maestro y viaja al Esclavo. Ajustar a mano
// las dos puntas dejaba hasta 59 s de desfase el primer dia -casi cuatro veces el
// todo-rojo- y dos pantallas en HH:MM no pueden detectarlo.
//
// TODAS estas funciones son NO BLOQUEANTES: encolan el intercambio y devuelven al
// momento. El envio, los reintentos y la espera del ACK los lleva
// coordinador_actualizar() con el mismo patron de SFTY-7 que ya usa el ciclo. Tenian
// que serlo: el bucle alimenta un watchdog de 4 s (SFTY-1) y atiende el parpadeo de
// ambar, y un intercambio con su reintento puede durar 7 s.
//
// Ninguna de ellas altera las luces ni la maquina de estados del ciclo.
// ---------------------------------------------------------------------------

// Encola el envio de la hora del Maestro (tramas H, M y S) al Esclavo.
// Devuelve false -y no envia nada- si el reloj del Maestro no esta en hora:
// empujar una hora no fiable dejaria al Esclavo con una hora inventada, que es peor
// que dejarlo sin hora porque aparenta validez.
// El true significa "peticion aceptada", no "el Esclavo ya la aplico"; para eso esta
// coordinador_msDesdeUltimaSync().
bool coordinador_sincronizarHora();

// Encola una medicion de desfase (CMD_DELTA). Mismo criterio: sin hora fiable en el
// Maestro la diferencia no significa nada, y devuelve false.
bool coordinador_medirDesfase();

// Encola la configuracion del ciclo (verde y despeje, en segundos). Dos relojes en
// hora dan tiempo comun, pero para ir en FASE ambas puntas deben computar el mismo
// horario: es tan condicion de seguridad como la hora.
bool coordinador_enviarConfigCiclo(uint8_t verdeSeg, uint8_t despejeSeg);

// Ultima diferencia medida contra el reloj del Esclavo, en segundos.
// DELTA_FUERA_DE_RANGO (-128) si la ultima medida se salio de +-127 s.
// Consultar SIEMPRE coordinador_desfaseValido() antes de darle credito.
int8_t coordinador_desfaseEsclavo();

// True solo si hay una medicion utilizable: existente, dentro de rango y RECIENTE.
// La vigencia importa: una medida vieja mostrada como si fuera de ahora es justo el
// error que SFTY-23 vino a eliminar.
bool coordinador_desfaseValido();

// True solo si el Esclavo ACUSO la configuracion del ciclo (CMD_ACK_CONFIG).
//
// Es la unica prueba de que el otro extremo sabe con que duraciones calcular su
// fase. Se baja al reencolar la configuracion, porque al otro lado puede haber una
// unidad distinta: lo que confirmo la anterior no dice nada de esta.
//
// La puerta del Modo Degradado lo exige. Sin esta comprobacion el Maestro aceptaba
// mientras el Esclavo rechazaba por falta de configuracion, y quedaba verde en una
// punta contra ambar en la otra.
bool coordinador_configConfirmada();

// Tiempo desde la ultima sincronizacion CONFIRMADA por el Esclavo (ACK_HORA).
// Si nunca hubo ninguna devuelve 0xFFFFFFFF, no cero: quien compare contra un limite
// de antiguedad -el tope de 48 h del Modo Degradado- debe leer "muy vieja", jamas
// "recien hecha".
unsigned long coordinador_msDesdeUltimaSync();