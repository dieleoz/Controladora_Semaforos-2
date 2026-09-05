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

// ---------------------------------------------------------------------------
// T: del $STATUS -- SEGUNDOS QUE FALTAN PARA QUE TERMINE LA FASE.
//
// Hasta el 04/09 este campo lo calculaba bluetooth.cpp como (millis()/1000) % 60:
// el SEGUNDERO DEL TIEMPO QUE LLEVA ENCENDIDO EL EQUIPO, subiendo de 0 a 59 y
// volviendo a empezar, estuviera el semaforo en verde, en rojo o en despeje. El
// comentario que llevaba encima decia "segundos transcurridos en fase actual" y no
// lo era: la app pintaba fielmente un numero que no significaba nada. Reportado
// desde el banco por el responsable -"el tiempo no esta retrocediendo, sino que
// esta aumentando... deberia empezar en 30 y disminuir"-.
//
// SIN_CUENTA_ATRAS NO ES CERO, Y ESA DIFERENCIA ES TODO EL ARREGLO. Cero significa
// "esta fase se acaba ahora mismo"; SIN_CUENTA_ATRAS significa "en esta fase no hay
// cuenta atras que dar". Quien publique el valor marca el segundo caso con "--", la
// misma convencion que ya llevan RF, RTT, BAT y HORA en esa misma trama. Un cero
// puesto donde no se sabe es el BAT:12.6 de N-108 otra vez: un numero con aspecto de
// medida que impide preguntar.
//
// EL CENSO de en que estados hay numero y en cuales no vive en coordinador.cpp,
// encima de la implementacion: es una propiedad de la maquina de estados, y ahi es
// donde se puede comprobar contra ella en vez de contra este resumen.
// ---------------------------------------------------------------------------
static const int SIN_CUENTA_ATRAS = -1;

int coordinador_segundosRestantesFase();

// --- Demanda vehicular de cámaras IA AcuSense (Sentido 2 / Esclavo) ---
bool coordinador_hayDemandaRemota();
void coordinador_limpiarDemandaRemota();

// N-142: devuelve true UNA vez si el Esclavo aviso de que le pusieron ambar de emergencia
// desde su telefono, y consume el aviso al leerlo -no es un estado, es un suceso-.
//
// Lo consulta main.cpp y no el coordinador, porque la respuesta es un CAMBIO DE MODO y el
// modo no lo decide la maquina del ciclo. Ver el porque completo en coordinador.cpp, en la
// rama de CMD_AMBAR_ESCLAVO.
bool coordinador_hayAmbarDelEsclavo();

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
// N-149: el color que la OTRA punta confirmo, para el campo ESC: del $STATUS.
// Devuelve "ROJO", "VERDE", "AMBAR" o "?" -este ultimo cuando el enlace esta caido y
// por tanto no se sabe-. Nunca devuelve un color por haber mandado una orden: el porque
// completo esta sobre la definicion, en coordinador.cpp.
const char* coordinador_estadoEsclavo();
