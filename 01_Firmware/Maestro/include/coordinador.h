// ===== include/coordinador.h =====
#pragma once
#include <Arduino.h>
// Los dos se incluyen por las COTAS de abajo, y las cotas se DERIVAN de aqui en vez de
// escribirse a mano: limites_ciclo.h da el techo de la fase larga y protocolo.h el de
// SFTY-6. Un numero copiado a mano en este fichero seria la cuarta copia de N-137.
#include "limites_ciclo.h"
#include "protocolo.h"

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

// --- LA COTA DEL CAMPO T:, Y POR QUE VIVE AQUI Y NO EN EL EMISOR (05/09) ----
//
// EL DEFECTO QUE CIERRA. bluetooth.cpp dimensionaba tTxt PARA EL TIPO -un int con %d
// son 11 caracteres- porque no podia fiarse del rango que promete OTRO modulo, y lo
// dejaba escrito: "un buffer dimensionado por una invariante que vive en otro modulo es
// el que se rompe en silencio el dia que ese modulo cambie". El instinto era bueno y la
// consecuencia era mala: con T:, RF: y RTT: a su tope de tipo el $STATUS pedia 162 B
// contra un techo de 155 -tramaCompleta[160] menos el *XX y el CRLF-, o sea que NO
// CABIA, y una trama truncada no llega a medias: sale bien formada hasta el corte, el
// checksum se calcula sobre lo que quedo y la app la descarta ENTERA. El sintoma en
// campo es "el equipo se callo", que manda a mirar el cable.
//
// LA SALIDA NO ES ENSANCHAR BUFFERS -eso mueve el problema y se come flash que no
// sobra-: es ACOTAR DONDE SE PRODUCE. La cota se declara junto a la funcion que la
// promete, o sea aqui, y el emisor la comprueba antes de imprimir. Asi el buffer se
// puede dimensionar para el RANGO sin fiarse de nadie: lo que sostiene el tamano no es
// una creencia sobre este modulo, es una guarda que el emisor ejecuta.
//
// EL NUMERO SE DERIVA, NO SE ESCRIBE. Las dos funciones que alimentan T: dan:
//   coordinador_segundosRestantesFase()   0..DESPEJE_SEG_MAX (90 s)
//   modoAutomatico_segundosRestantesFase() 0..max(VERDE_MIN_MAX, ROJO_MIN_MAX) x 60
// El segundo es el techo, y sale de limites_ciclo.h. Escribir "900" aqui seria la
// QUINTA copia a mano de un limite del ciclo -N-131, N-133 y N-137 fueron las tres
// primeras-, y el dia que el responsable suba el maximo a 20 min esta cota se mueve
// sola o no se mueve: por eso es una expresion y no un literal.
static const int CUENTA_ATRAS_MAX_SEG =
    (int)(VERDE_MIN_MAX > ROJO_MIN_MAX ? VERDE_MIN_MAX : ROJO_MIN_MAX) * 60;

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

// N-152: y el aviso de que lo RETIRA, con el mismo trato -se consume al leerlo-.
//
// Quien lo consume tiene que comprobar ADEMAS que el ambar vigente sea el del Esclavo
// (modo_ambar_origenEsclavo()): esta funcion dice que el Poste 2 lo pidio, no que se
// deba obedecer. Ver protocolo.h, cabecera de CMD_CANCELA_AMBAR_ESCLAVO.
bool coordinador_hayCancelaAmbarDelEsclavo();

// N-152: escucha la radio SIN hablar, para el unico modo en que el Maestro no la lee.
//
// En MODO_AMBAR nadie llama a coordinador_actualizar(), que es el unico sitio del
// Maestro donde se lee el UART; sin esto la cancelacion del Esclavo llegaria y no la
// leeria nadie. NO responde, NO emite latido, NO toca la maquina de estados y NO
// refresca la telemetria: SFTY-21 pide callar, y callar es no transmitir.
void coordinador_escucharEnAmbar();

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

// La cota de arriba, para quien tenga que imprimirla. El 100 NO es un redondeo ni una
// preferencia: la cuenta es (respondidos * 100) / muestrasLatido con respondidos <=
// muestrasLatido por construccion -respondidos son los bits a 1 de una mascara de
// muestrasLatido bits-, asi que el cociente no puede pasar de 100. El unico valor fuera
// de [0, 100] que la funcion produce hoy es el -1 de "aun sin muestras", y ese tiene su
// propia rama. Se declara para que el emisor pueda dimensionar "100%" -cuatro
// caracteres- en vez de los doce que exige un int con %d.
static const int CALIDAD_ENLACE_MAX = 100;

// Tiempo de respuesta suavizado del ultimo latido, en ms. 0 si no hay muestras.
unsigned long coordinador_tiempoRespuestaMs();

// LA COTA DEL RTT PUBLICABLE, Y NO ES UN NUMERO INVENTADO. Este RTT es la media
// exponencial de millis() - tLatidoEnviado sobre latidos que el Esclavo CONTESTA.
// Por encima de SFTY6_SILENCIO_MS ya no hay enlace que medir: a ese silencio SFTY-6 se
// lleva el cruce a ambar, asi que un RTT mayor no es una latencia alta, es una medida
// que no significa nada -un latido que se quedo en vuelo mientras el ciclo agotaba sus
// reintentos, o un contador que se fue-. Se hereda del techo de orfandad en vez de
// fijar un "9999" propio: son 5 cifras, y el dia que alguien mueva SFTY6_SILENCIO_MS
// esta cota se mueve con el.
static const unsigned long RTT_PUBLICABLE_MAX_MS = SFTY6_SILENCIO_MS;

// Latidos consecutivos sin respuesta. 0 con el enlace sano.
int coordinador_latidosSinRespuesta();

// LA COTA DE ARRIBA, PARA QUIEN TENGA QUE IMPRIMIRLA. El contador NO es libre: quien lo
// incrementa lo para en seco -"else if (latidosSinRespuesta < 999) latidosSinRespuesta++"
// en coordinador.cpp-, asi que tres cifras es todo lo que puede salir de aqui, no las
// once que exige un int con %d. Se declara junto al getter para que el $ALARM pueda
// dimensionar su tramo por el rango que el modulo promete y no por el tipo.
//
// Y ESTE 999 ES UNA COPIA, ASI QUE SE MIDE EN VEZ DE CREERSE (CLAUDE.md 3.bis): el numero
// vive de verdad en la guarda del .cpp, y esp32_07_presupuesto_bytes recalcula la igualdad
// leyendo esa guarda en cada corrida. El dia que alguien mueva uno de los dos sin el otro,
// el pack lo dice -que es lo que una constante repetida sin vigilancia no hace-.
static const int LATIDOS_SIN_RESPUESTA_MAX = 999;

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
