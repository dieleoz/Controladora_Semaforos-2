// ===== include/protocolo.h =====
#pragma once
#include <Arduino.h>

// RF Binary Commands
#define CMD_GO_GREEN   0x01
#define CMD_GO_RED     0x02
#define CMD_ACK_GREEN  0x03
#define CMD_PING       0x04
#define CMD_PONG       0x05
#define CMD_ACK_RED    0x06

// --- SFTY-23: Sincronizacion horaria por radio -----------------------------
// La hora se cuadra UNA sola vez en el Maestro y viaja al Esclavo. Ajustar a mano
// las dos puntas dejaba hasta 59 s de desfase el primer dia -casi cuatro veces el
// todo-rojo- y dos pantallas en HH:MM no pueden detectarlo.
//
// El paquete tiene UN solo byte de param, asi que ni la hora ni el dia caben en una
// trama. Se envian CUATRO y el Esclavo las APLICA JUNTAS al recibir la de segundos:
// nunca queda una hora a medias.
//
//   Maestro                          Esclavo
//     CMD_HORA_D (param = 1..31) -->  guarda en buffer   (abre el envio)
//     CMD_HORA_H (param = 0..23) -->  guarda en buffer
//     CMD_HORA_M (param = 0..59) -->  guarda en buffer
//     CMD_HORA_S (param = 0..59) -->  APLICA las CUATRO  --> CMD_ACK_HORA
//
// SI FALTA CUALQUIERA DE LAS CUATRO, no se aplica nada y NO se acusa: el silencio es
// lo que provoca el reintento del envio completo. Aplicar la hora sin el dia dejaria
// a esa punta sincronizada al segundo pero con SU PROPIO calendario, que es justo el
// defecto que CMD_HORA_D viene a cerrar. Media sincronizacion no es media mejora.
//
// CONSECUENCIA DE DESPLIEGUE: el Esclavo EXIGE el dia. Un Maestro que no lo emita
// deja al Esclavo sin poder aplicar ninguna hora. Las dos puntas deben flashearse
// con la misma version.
//
// OBLIGATORIO: el Maestro debe RECALCULAR el valor de segundos en cada
// retransmision. Reenviar el que calculo la primera vez mete el error justo por el
// mecanismo que existe para dar robustez: una trama perdida y reintentada 3,5 s
// despues dejaria al Esclavo 3,5 s atrasado.
// CMD_HORA_D lleva el DIA DEL MES (1..31) y va PRIMERO en la secuencia.
//
// No interesa la fecha real -el equipo no la muestra ni la usa para nada mas-, sino
// que las dos puntas cuenten los dias con el MISMO numero. Sin esto, cada unidad
// sembraba su propio dia 1 al ponerse en hora y los calendarios quedaban
// desacoplados para siempre.
//
// Lo que eso provocaba, detectado por el validador de costura el 01/08/2026: un
// corte de energia el dia en que el calendario de UNA punta pasa de 31 a 1 hace que
// respaldo_horasDesdeSync() declare CADUCADA solo en esa punta. Esa no reanuda y se
// queda en AMBAR, mientras la otra reanuda en fase y sigue dando VERDE. Con los
// calendarios acoplados las dos fallan a la vez y el resultado es simetrico y
// seguro; desacoplados, no.
#define CMD_HORA_D     0x10

#define CMD_HORA_H     0x07
#define CMD_HORA_M     0x08
#define CMD_HORA_S     0x09
#define CMD_ACK_HORA   0x0A

// Medicion de desfase. El Maestro envia su segundo actual (0..59); el Esclavo
// responde con la diferencia contra su propio reloj, EN COMPLEMENTO A DOS dentro
// del byte: se interpreta como int8_t, rango util -127..+127 s.
// Fuera de ese rango debe SATURAR a -128, que significa "fuera de rango", y nunca
// dar la vuelta.
// La medida incluye el tiempo de aire y el retardo de cortesia del Esclavo
// (SFTY-17, 200 ms), asi que trae un sesgo de decimas de segundo. Es irrelevante
// frente a un todo-rojo de 15-30 s y queda escrito para que nadie lo persiga.
#define CMD_DELTA      0x0B
#define CMD_DELTA_RESP 0x0C
#define DELTA_FUERA_DE_RANGO ((int8_t)-128)

// Configuracion del ciclo. Dos relojes en hora dan tiempo comun, pero para ir en
// FASE ambas puntas deben computar el mismo horario: sin esto el Esclavo no sabe
// cuanto dura el verde ni el despeje del Modo Degradado.
//
// param = SEGUNDOS, saturado a 255. Es tan condicion de seguridad como la hora.
//
// ---------------------------------------------------------------------------
// QUIEN AMPLIA EL DESPEJE: EL MAESTRO, ANTES DE ENVIARLO.
//
// El byte que viaja es EL VALOR FINAL que ambas puntas deben usar en Modo
// Degradado, con la ampliacion YA APLICADA. El Esclavo lo usa TAL CUAL y no lo
// escala, ni lo duplica, ni lo interpreta.
//
// Esto era ambiguo hasta el 01/08/2026 y es la clase de ambiguedad que mata: si
// una punta ampliara por su cuenta y la otra no, las dos calcularian ciclos de
// DISTINTA DURACION sobre la misma hora. No es un desfase de segundos que el
// todo-rojo absorba: los verdes se solaparian durante MINUTOS.
//
// La regla general: la ampliacion se decide en UN SOLO SITIO -el origen-. Un
// parametro que cada extremo transforma por su cuenta es dos implementaciones de
// una politica, y dos implementaciones acaban divergiendo.
//
// LIMITE CONOCIDO: al caber en un byte, el ciclo degradado queda topado en 255 s
// por fase. El Maestro admite verdes de hasta 99 min en operacion normal, asi que
// por encima de 4 min 15 s el ciclo degradado NO sera el configurado. No es
// inseguro -ambas puntas usan el mismo valor saturado y siguen en fase- pero debe
// constar en el manual del funcional.
// ---------------------------------------------------------------------------
#define CMD_CONFIG_VERDE   0x0D
#define CMD_CONFIG_DESPEJE 0x0E
#define CMD_ACK_CONFIG     0x0F

// Demanda vehicular de cámaras IA AcuSense por radio LoRa

// --- SFTY-6: cuando una punta se rinde a ambar por silencio de radio ---------
//
// N-69: este numero estaba escrito TRES VECES como literal -Esclavo/main.cpp,
// coordinador.cpp dos veces- y gobierna las dos puntas de una regla de seguridad:
// cuanto silencio se aguanta antes de rendirse a ambar intermitente.
//
// Tres copias mantenidas a mano es como se desincronizan las puntas sin que nadie
// se entere: se cambia una, se olvida otra, y el Maestro y el Esclavo se rinden en
// instantes distintos. Aqui vive una sola vez, en el contrato que costura_01 ya
// compara byte a byte entre las dos puntas.
//
// N-71: DE 12 s A 25 s, Y EL PORQUE ES UNA CUENTA, NO UNA HOLGURA "POR SI ACASO".
//
// Este valor NO es independiente: es el TECHO de todo el presupuesto de reintentos
// de radio, porque quien primero llegue manda. Y estaba por DEBAJO de ese
// presupuesto, asi que se disparaba siempre antes de que el ciclo pudiera agotar
// sus propios reintentos:
//
//   coordinador.cpp reintenta 5 veces con TIMEOUT_ACK_MS = 3500 ms. Contando que
//   el intercambio arranca como muy tarde 3 s despues de la ultima recepcion -esa
//   es la cadencia del latido-, el peor caso son 3 + 5 x 3,56 = 20,8 s.
//
//   Con el techo en 12 s, el ambar por orfandad saltaba sobre el segundo o tercer
//   reintento. Los reintentos 4 y 5 NO PODIAN EJECUTARSE NUNCA: eran codigo muerto
//   dentro del mecanismo de recuperacion, y su comentario -"Fallo tras 5 reintentos
//   (12.5s)"- venia de un TIMEOUT_ACK_MS de 2500 ms que dejo de existir el 31/07.
//
// 25 s cubren los 20,8 s del peor caso con 4,2 s de margen, y de paso dejan sitio a
// los 3 intentos de sincronizacion horaria (13,7 s) que antes no cabian.
//
// LO QUE CUESTA SUBIRLO, dicho sin adornos: el cruce puede quedarse hasta 25 s en
// la fase que tuviera cuando cayo el enlace, en vez de 12. NO puede aparecer un
// verde nuevo en ese rato -ninguna punta enciende verde sin el ACK de la otra-, asi
// que lo que se alarga es una espera, no un riesgo de verde simultaneo. A cambio se
// evita el ambar espurio, que segun el reporte de campo del 27/08 aparece "cada
// nada cuando llueve": una lluvia que tumba tramas sueltas consumia el techo antes
// de que los reintentos tuvieran ocasion de recuperarlas.
//
// OJO AL LIMITE SUPERIOR: subirlo mas no es gratis ni indefinido. Es el tiempo que
// una punta sigue con la configuracion vieja creyendo que la otra la acompana.
// costura_09_presupuesto_radio recalcula la desigualdad desde las constantes del
// C++ y falla si alguien toca un reintento o un timeout sin mirar este techo.
#define SFTY6_SILENCIO_MS   25000UL

#define CMD_DEMANDA        0x11
#define CMD_ACK_DEMANDA    0x12

// N-134 (04/09): EL AMBAR SE ORDENA, NO SE DEDUCE.
//
// Hasta hoy, poner el cruce en ambar desde el Maestro dejaba al Esclavo en ROJO y era
// el propio Esclavo quien pasaba a ambar 25 s despues, por ORFANDAD (SFTY-6). El
// resultado era el correcto, pero por casualidad afortunada: el ambar del Esclavo no lo
// ordenaba nadie, era el Esclavo rindiendose. Reportado en banco el 04/09 como "a veces
// los dos pasan a ambar, a veces solo el maestro" -depende de cuanto mire uno-, y el
// operario pulsaba tres veces en catorce segundos porque no veia cambiar la otra punta.
//
// Y una casualidad afortunada no es una garantia: el dia que alguien toque
// SFTY6_SILENCIO_MS, el ambar del cruce cambia de tiempo sin que nadie lo relacione.
//
// LO QUE NO SE TOCA, y es decision del responsable del 04/09: la orfandad SE QUEDA como
// red. La orden acelera el caso normal; si la radio se cae justo en ese instante, el
// Esclavo sigue yendo a ambar a los 25 s por su cuenta. Se gana el caso bueno sin
// perder el malo.
//
// EL ROJO PREVIO TAMPOCO SE TOCA: modo_ambar_setup() sigue mandando CMD_GO_RED primero.
// Es el intermedio seguro -si esta orden se pierde, el Esclavo queda PARADO hasta que
// la orfandad lo saque, y parado es la direccion segura-.
#define CMD_GO_AMBAR       0x13

// N-142 (04/09): EL ESCLAVO AVISA DE QUE LE PUSIERON AMBAR DE EMERGENCIA.
//
// El unico paquete que va del Esclavo al Maestro por iniciativa del Esclavo aparte de
// CMD_DEMANDA. Nace de un bloqueo reportado en banco:
//
//   "como me conecte a la aplicacion del Esclavo y le di ambar, esta quedo en ambar
//    intermitente. Si me conecto otra vez al Maestro, esto ya no me recibe nada"
//
// LO QUE PASABA: el ambar de emergencia enganchaba un CERROJO -el Esclavo dejaba de
// obedecer y de ACUSAR las ordenes de radio-, el Maestro agotaba reintentos, caia a
// C_FALLO y rechazaba todo. La unica salida era CANCELAR_AMBAR, que solo acepta el
// Esclavo y con PIN, y para llegar alli hay que desvincular el Maestro en Ajustes de
// Android. Un operario podia dejar el cruce trabado y no poder soltarlo desde el otro
// poste.
//
// Y ANTES DE ESO, LA VENTANA PEOR: el Esclavo avisaba al TELEFONO y no al Maestro, y
// ademas seguia contestando PONG, asi que el enlace le parecia perfecto al Maestro. Si
// el Maestro estaba en VERDE, durante el resto de esa fase -hasta 3 minutos- convivian
// Maestro en verde y Esclavo en ambar, y los dos sentidos podian entrar al carril.
//
// LA DECISION DEL RESPONSABLE (04/09), y son DOS mitades que solo funcionan juntas:
//
//   1. El Esclavo AVISA por radio al enganchar. El Maestro se va a MODO_AMBAR en el
//      acto: deja de ciclar y de dar verdes. El ambar del operario se RESPETA.
//   2. Y el ambar pedido POR LA APP deja de ser un cerrojo frente a la radio. El del
//      MANDO fisico -B.B.B- lo sigue siendo, que es quien de verdad demuestra que hay
//      alguien subido al gabinete.
//
// La segunda sin la primera dejaria el boton inutil -la siguiente orden del Maestro le
// quitaria el ambar en segundos-. La primera sin la segunda deja el bloqueo si el aviso
// se pierde. Juntas: el caso normal lo respeta el Maestro, y si el aviso no llega, la
// siguiente orden recupera el cruce en vez de trabarlo. Es la misma estructura que
// N-134 del reves.
#define CMD_AMBAR_ESCLAVO  0x14

// N-130: EL PARAM DE CMD_ACK_DEMANDA DICE SI LA DEMANDA SE VA A ATENDER O NO.
//
// Hasta el 04/09 el Maestro acusaba la demanda SIEMPRE y armaba su bandera SIEMPRE,
// y esa bandera la lee UN SOLO fichero -modo_inteligente.cpp-. En Manual y en
// Automatico el Esclavo contestaba a la app "$ACK ... PEDIDO_AL_MAESTRO", el operario
// leia la confirmacion, y el cruce no se movia. Es §6 -un acuse que no depende de lo
// que paso- pero repartido entre DOS placas: por separado ninguna rama parecia mal
// escrita, y por eso no lo vio nadie.
//
// Se resuelve SIN gastar un codigo de comando ni tocar la trama: el param ya viaja y
// ya va cubierto por el CRC -calcularCRC_Bin(&pkt, 3) incluye msgID, command y param-.
//
// ACEPTADA vale 0 A PROPOSITO: protocolo_enviarPaquete() pone param=0 por defecto, asi
// que un Maestro con firmware viejo -que no conoce estos motivos- sigue leyendose como
// "aceptada" y se comporta como siempre. El valor nuevo es el que dice que NO, que es
// el que hay que anunciar; si la compatibilidad fallara, fallaria hacia el silencio de
// hoy y no hacia una alarma inventada.
#define DEMANDA_ACEPTADA   0
#define DEMANDA_RECHAZADA  1

// SFTY-11: Copias por ráfaga (Burst). Ver nota en protocolo_enviarPaquete().
// A la tasa aérea de operación (2.4 kbps) cada copia de 4 bytes cuesta ~0.04s de aire
// (x2 con FEC): 3 copias son ~0.13s, despreciable frente a ciclos de 15s y 60s.
// A la tasa anterior (0.3 kbps) las mismas 3 copias costaban ~2.2s y saturaban el canal
// half-duplex; ése era el problema, no la redundancia en sí.
// Se mantiene en 3 porque los equipos son MÓVILES y la distancia de despliegue es
// desconocida: a mayor distancia sube la pérdida de tramas y la redundancia es la que paga.
// DEBE ser idéntico en Maestro y Esclavo.
#define RF_BURST_COPIES 3

#pragma pack(push, 1)
struct RF_Packet {
    uint8_t msgID;
    uint8_t command;
    uint8_t param;
    uint8_t crc;
};
#pragma pack(pop)

void protocolo_setup();
void protocolo_enviarPaquete(uint8_t cmd, uint8_t param = 0);
bool protocolo_hayPaqueteDisponible(RF_Packet* destino);
void protocolo_resetReplayProtection();

// SFTY-15: Diagnostico de linea. Distingue "no llega nada" de "llega basura",
// que en pantalla se veian igual ("fallo de comunicacion").
unsigned long protocolo_bytesRecibidos();
unsigned long protocolo_tramasValidas();
unsigned long protocolo_tramasDescartadas();
void protocolo_reiniciarContadores();
