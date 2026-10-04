// ===== src/modo_degradado.cpp =====
#include "modo_degradado.h"
#include "modo_ambar.h"
#include "bluetooth.h"   // D-26 (4): el $EVENT del salto de hora que pasa por rojo
#include "botones.h"
#include "ciclo_degradado.h"
#include "coordinador.h"
#include "menu.h"
#include "modos.h"
#include "reloj.h"
#include "respaldo.h"
#include "semaforo.h"
#include "testigo_flash.h"

// ---------------------------------------------------------------------------
// CONFIGURACION DEL CICLO DEGRADADO
//
// AQUI, Y SOLO AQUI, SE APLICA LA AMPLIACION DEL DESPEJE.
//
// Estas dos constantes son el ciclo degradado COMPLETO y DEFINITIVO. El despeje ya
// viene ampliado al doble del normal (15 s de operacion corriente -> 30 s), y este
// mismo valor es el que:
//
//   a) se envia al Esclavo por CMD_CONFIG_DESPEJE (SFTY-23), y
//   b) se le pasa a ciclo_degradado_fase() en este mismo fichero.
//
// Que sean el mismo simbolo no es comodidad, es la condicion de seguridad: el
// contrato dice que el Maestro amplia ANTES de enviar y que el Esclavo usa el byte
// TAL CUAL, sin escalarlo ni interpretarlo. Si una punta ampliara por su cuenta y la
// otra no, las dos calcularian ciclos de DISTINTA DURACION sobre la misma hora, y eso
// no es un desfase de segundos que el todo-rojo absorba: los verdes se solaparian
// durante MINUTOS. Por eso la ampliacion vive en un unico sitio y no hay ninguna
// variable intermedia donde alguien pueda volver a multiplicar por dos.
//
// POR QUE 30 s DE DESPEJE. En Degradado el todo-rojo ya no es solo el despeje de la
// interseccion: es el colchon que absorbe la DERIVA entre dos cristales de 32.768 kHz
// sin calibrar y a la intemperie, que se separan del orden de 8,6 s al dia en el peor
// caso. Con 30 s de despeje el margen teorico son ~3,5 dias.
//
// EL FACTOR DE SEGURIDAD DE 2 QUE AQUI SE AFIRMABA ES FALSO. Medido el 01/09 ejecutando
// el C++ REAL de las dos puntas a la vez, cada una con su reloj -Validacion_Automatico/
// compilar_degradado.ps1-:
//
//   el cruce aguanta            29 s de desfase entre relojes
//   el equipo puede acumular    20,2 s  (17,2 de deriva en 48 h + 3 de TOLERANCIA_DESFASE_S)
//   MARGEN                       8,8 s  ->  factor 1,44, NO 2
//
// La frontera del sentido malo -Esclavo atrasado- es EXACTAMENTE este despeje de 30 s, y
// el segundo entero con que viaja la hora la deja en 29. D-45: las dos puntas abren su
// verde directo y lo cierran por su amarillo, que se ANADE antes del despeje; el ambar con
// que el Esclavo abria y que favorecia un sentido ya no existe (lo mide el arnes).
//
// Se deja escrito con el numero y no se toca el despeje: subirlo es una decision vial
// -alarga el todo-rojo que ve el conductor- y no la toma el firmware. Lo que si cambia
// es que la desigualdad ya NO vive solo en este comentario: la recalcula desde el C++
// el pack costura_12_margen_deriva, que es lo que N-71 exige. Un comentario no falla
// cuando alguien cambia un numero; se queda describiendo un equipo que ya no existe,
// con la autoridad de una cuenta hecha - que es exactamente lo que paso aqui.
//
// Querer una semana de autonomia obligaria a un todo-rojo de ~90 s, que destroza la
// fluidez del paso. No es una limitacion del diseno, es la fisica de dos cristales
// sin disciplinar. La alternativa real no es alargar el plazo: es ir a arreglar el
// radio.
//
// EL VERDE. Hasta el 13/09 se igualaba al despeje (30 s), y a esa eleccion se le
// atribuian DOS razones que, recontadas, son FALSAS -se dejan escritas y REFUTADAS en
// vez de borradas (CLAUDE.md 7.4), porque una causa que desaparece en silencio vuelve a
// proponerse-:
//
//   "no cabe en el byte"            FALSA. CMD_CONFIG topa en 255 s (ver mas abajo) y
//                                   180 no se acerca al borde.
//   "lo exige el margen de deriva"  FALSA. Ese margen lo pone el DESPEJE, no el verde:
//                                   costura_12_margen_deriva mide que el aguante del
//                                   cruce en el sentido malo es EXACTAMENTE el despeje
//                                   ampliado, y no se mueve ni un segundo al cambiar el
//                                   verde.
//
// La razon que SI sostenia el 30 s era FLUIDEZ -"se pierde fluidez, que es exactamente
// lo que se acepta en un modo degradado"-, y es la que el responsable decidio en contra
// el 13/09: el verde de Degradado pasa a ser el MISMO minimo vial que ya rige para todo
// lo demas (D-5, VERDE_MIN_MIN = 3 min = 180 s). El ciclo pasa de 2*(30+30) = 120 s a
// 2*(180+30) = 420 s, y la espera maxima de quien llega justo despues de su verde pasa
// de 30+30+30 = 90 s a 30+180+30 = 240 s. El despeje no se toca, y el margen de deriva
// no cambia -precisamente porque no depende del verde-.
//
// TOPE DEL BYTE. CMD_CONFIG lleva un solo byte por valor, asi que el ciclo degradado
// esta topado en 255 s por fase. Ni los 180 s de verde ni los 30 de despeje se acercan
// al tope, y ademas el ciclo degradado NO hereda el verde configurado en Modo
// Automatico -que se mide en minutos y si desbordaria-: es fijo y propio de este modo.
// ---------------------------------------------------------------------------
static const uint16_t DEG_VERDE_SEG = 180;
static const uint16_t DEG_DESPEJE_SEG = 30;   // YA AMPLIADO. Ver arriba.

// --- Puerta de entrada -----------------------------------------------------
//
// LA GARANTIA ES LA SINCRONIZACION RECIENTE; EL DESFASE ES SOLO COMPROBACION DE CORDURA. No al reves:
//
//   CMD_DELTA transporta SOLO el segundo (0..59), asi que la correccion circular cae SIEMPRE en +-30 s. El alias no es
//   "los grandes se escapan y los pequenos no": es que TODO MULTIPLO DE 60 s se lee como cero. Un desfase de 60, 120 o
//   3600 s pasa la tolerancia, mientras que 45 s SI se detecta (se lee como -15 s). Ninguna aritmetica distingue 0 de
//   60 con un byte de segundos. (Aqui se afirmaba lo contrario hasta que el validador lo desmintio; se
//   corrige en vez de matizarlo: dos comentarios contradictorios en una funcion de seguridad son peores que uno malo.)
//
//   Lo que impide el alias es que la sincronizacion sea FRESCA: tras una correcta el desfase arranca en milisegundos
//   y con ~100 ppm harian falta mas de tres dias para acumular los 30 s del alias; con una de hace dos horas la
//   deriva es de ~0,7 s.
//
// Confiar en el numero y no en su frescura reintroduce el fallo entero.
//
// 2 h: dos periodos de resincronizacion (el coordinador reintenta cada hora) y la vigencia que el coordinador da a la
// medida de desfase. Un intercambio perdido no debe cerrar la puerta; dos seguidos si.
static const unsigned long SYNC_FRESCA_MS = 7200000UL;

// +-3 s: diez veces por debajo del todo-rojo de 30 s y por encima del sesgo de la medida (tiempo de aire mas el
// retardo de cortesia del Esclavo, SFTY-17). No busca precision: detecta que algo no cuadra.
static const int8_t TOLERANCIA_DESFASE_S = 3;

// --- Limite duro -----------------------------------------------------------
//
// Pasadas 48 h sin sincronizar, el Degradado cae SOLO a ambar intermitente. No es un
// aviso, es un tope: EL ESTADO SEGURO NO PUEDE DEPENDER DE QUE ALGUIEN SE ACUERDE.
// El diseno automatico que precedio a este (SFTY-19) tenia esta regla y al pasar a
// activacion manual se perdio; recuperarla no es opcional.
static const unsigned long LIMITE_DURO_MS = 172800000UL;  // 48 h
static const unsigned long AVISO_LIMITE_MS = 158400000UL; // 44 h: avisa las ultimas 4

// El mismo limite en horas, para la reanudacion tras un corte (la marca de la pila resuelve en horas enteras). Se
// DERIVA del valor de arriba: dos numeros que deben ser el mismo acaban siendo distintos el dia que alguien toca uno.
static const uint32_t LIMITE_DURO_H = LIMITE_DURO_MS / 3600000UL;

// Todo-rojo minimo al entrar y al salir. Coincide con el despeje del ciclo por el
// mismo motivo por el que existe el despeje: es el tiempo que tarda en vaciarse el
// tramo. Entrar o salir mas rapido que eso seria dar por vacio algo que no lo esta.
static const unsigned long ROJO_TRANSICION_MS = (unsigned long)DEG_DESPEJE_SEG * 1000UL;

// ---------------------------------------------------------------------------
// D-35 - EL DEGRADADO CON TESTIGO (SPEC_2 7.bis). Constantes y estado.
//
// TOLERANCIA_TESTIGO_S: el MISMO criterio que TOLERANCIA_DESFASE_S (diez veces por debajo del todo-rojo mas corto y
// por encima del sesgo de una transmision, aqui Bluetooth telefono-poste); constante propia porque la otra es del
// desfase de RADIO. EL DESPEJE TIENE SU PROPIO RANGO, 30..255, no los 10..90 del Automatico: un modo sin camara ni
// radio que lo vigile pide mas margen; el suelo de 30 es el de D-18, asi que SALTO_SIN_ROJO_MAX_S queda del lado
// seguro con cualquier despeje admitido. El verde viaja fijo en 180 (DEG_VERDE_SEG): lo rechaza el despachador por
// formato. LA EDAD SE CUENTA CON LA FECHA DEL DS3231 (reloj_segundosDesde2000()), no con el contador del RTC: sin Y2
// ese contador no cuenta, y HAL_RTC_GetTime puede reescribirlo.
// ---------------------------------------------------------------------------
static const int32_t  TOLERANCIA_TESTIGO_S   = 3;
static const int      TESTIGO_DESPEJE_MIN    = 30;
static const int      TESTIGO_DESPEJE_MAX    = 255;
static const uint32_t TESTIGO_INICIO_MAX_S   = 43200UL;           // 12 h: mas es "ya paso"
// 29/09 (responsable, H9): el testigo YA NO VENCE -se retiro TESTIGO_VIGENCIA_S, los 31 dias
// a ambar-. Queda el aviso: a los 28 dias y despues una vez al dia, sin tocar luz ni modo.
static const uint32_t TESTIGO_AVISO_S        = 28UL * 86400UL;    // 28 dias: solo aviso

static bool testigo = false;            // el Degradado en curso es de testigo
static uint8_t testigoDespeje = DEG_DESPEJE_SEG;
static uint32_t testigoMarcaS = 0;      // s desde 2000 del ultimo testigo aceptado
static uint32_t testigoInicioS = 0;     // s desde 2000 de inicio
static bool entradaTestigoHecha = false;  // setup() no debe rehacer lo que hizo entrarTestigo
static bool reanudarComoTestigo = false;  // la reanudacion pendiente es de testigo
static TestigoFlash testigoLeido;         // lo que leyo la reanudacion de la flash

static uint8_t despejeEnUso() { return testigo ? testigoDespeje : (uint8_t)DEG_DESPEJE_SEG; }

// El todo-rojo de entrada y salida es el despeje EN USO: con testigo, el pedido.
static unsigned long rojoTransicionMs() {
  return testigo ? (unsigned long)testigoDespeje * 1000UL : ROJO_TRANSICION_MS;
}

// a - b por el camino corto del circulo del dia, en (-43200, 43200].
static int32_t difCircular(uint32_t a, uint32_t b) {
  int32_t d = (int32_t)((a + 86400UL - b) % 86400UL);
  return d > 43200L ? d - 86400L : d;
}

// Antiguedad del testigo en segundos; 0xFFFFFFFF si no se puede fechar (sin fecha del
// DS3231, o el reloj retrocedio por debajo de la marca). Ante la duda, se avisa.
static uint32_t edadTestigoS() {
  const uint32_t ahora = reloj_segundosDesde2000();
  if (ahora == 0 || ahora < testigoMarcaS) return 0xFFFFFFFFUL;
  return ahora - testigoMarcaS;
}

static bool testigoEnCurso() { return testigo && modoActual_get() == MODO_DEGRADADO; }

// A los 28 dias de la ultima marca, y despues una vez al dia: pedir renovar el testigo.
static uint32_t diaRenovarAvisado = 0xFFFFFFFFUL;
static void avisarRenovacion() {
  const uint32_t edad = edadTestigoS();
  if (edad == 0xFFFFFFFFUL || edad < TESTIGO_AVISO_S) { diaRenovarAvisado = 0xFFFFFFFFUL; return; }
  const uint32_t dia = (edad - TESTIGO_AVISO_S) / 86400UL;
  if (dia == diaRenovarAvisado) return;
  diaRenovarAvisado = dia;
  bluetooth_reportarAlarma("DEGRADADO", "RENOVAR_TESTIGO", "REPITA_TESTIGO");
}

// ---------------------------------------------------------------------------
// D-26 (4) - UNA HORA QUE SALTA MAS QUE EL MARGEN DEL CRUCE SE APLICA PASANDO POR ROJO.
//
// Desde D-26 la hora de esta punta se re-siembra del DS3231 de su ESP32 cada ~5 min
// TAMBIEN EN DEGRADADO, y la fase del ciclo sale de reloj_segundosDelDia(): cada siembra
// MUEVE LA FASE de golpe lo que el HSI derivo desde la anterior. Hasta hoy esa vuelta
// hacia semaforo_forzarVerde() en la MISMA iteracion si el salto caia en la fase del verde
// de esta punta: un salto hacia delante desde el verde del OTRO poste, por encima del
// despeje, pasaba de su verde al nuestro sin un solo instante de rojo por medio.
//
// EL UMBRAL SALE DEL DESPEJE, NO SE ESCOGE: DEG_DESPEJE_SEG - 1 = el margen del cruce.
//   - Un salto de J segundos enteros solo puede llevar del ultimo segundo del verde del
//     otro poste al primero del nuestro si J >= DEG_DESPEJE_SEG + 1: el despeje es justo
//     el rojo que separa los dos verdes. Por debajo, el salto cae como mucho DENTRO del
//     despeje y el rojo sigue estando en medio.
//   - Y el salto se MIDE en segundos enteros, con un segundo de error por truncado: un
//     salto medido de DEG_DESPEJE_SEG - 1 puede ser uno real de DEG_DESPEJE_SEG, que
//     todavia no salta el despeje. Por eso -1 y no el despeje entero.
//   - Coincide con el desfase que el cruce aguanta medido sobre el C++ de las dos puntas
//     (compilar_degradado.ps1: 29 s con el despeje en 30), y no es casualidad: es la misma
//     frontera vista desde el salto. esp32_13 lo recalcula.
// Un salto menor se aplica directo: es la correccion normal de la deriva y el despeje la
// absorbe igual que absorbe la deriva entre cristales. Mandarlo a rojo pararia el cruce
// en cada siembra.
//
// PASAR POR ROJO ES EL CAMINO QUE YA EXISTE: DEG_ENTRADA_ROJO, con su ROJO_TRANSICION_MS
// completo Y esperando a que la fase deje atras el verde de esta punta, de modo que el
// siguiente verde sea uno entero contado desde su frontera. No hay un camino nuevo.
static const uint32_t SALTO_SIN_ROJO_MAX_S = (uint32_t)DEG_DESPEJE_SEG - 1UL;

// LA OTRA MITAD, Y ES LA QUE HACE QUE EL UMBRAL NO SEA UNA TAPIA: una siembra NORMAL
// -la deriva del HSI en su peor caso durante una cadencia, redondeada hacia arriba, mas el
// segundo del truncado- tiene que quedar POR DEBAJO. Si no, el Degradado pasaria por rojo
// en cada siembra. Las constantes estan en reloj.h, y la cadencia se contrasta con la del
// ESP32 en esp32_13.
static_assert((HORA_ESP32_CADENCIA_MS / 1000UL * HSI_PPM_PEOR + 999999UL) / 1000000UL + 1UL
                  < SALTO_SIN_ROJO_MAX_S,
              "D-26 (4): una siembra normal saltaria mas que el margen y el Degradado "
              "pasaria por rojo cada cadencia");

// La hora de pared de la vuelta anterior y el millis() en que se leyo.
static uint32_t segVisto = 0;
static unsigned long tVisto = 0;

static void anclarHora() {
  segVisto = reloj_segundosDelDia();
  tVisto = millis();
}

// Cuanto se ha movido la hora de pared DE MAS -o de menos- respecto de lo que corrio
// millis() desde la vuelta anterior, por el camino corto del circulo del dia. Re-ancla en
// cada llamada, asi que mide saltos entre dos vueltas y no acumula nada: en marcha normal
// da 0 o 1 (el truncado de los dos segundos enteros).
static uint32_t saltoDeHora() {
  const uint32_t ahora = reloj_segundosDelDia();
  const uint32_t esperado = (segVisto + (uint32_t)((millis() - tVisto) / 1000UL)) % 86400UL;
  uint32_t d = (ahora + 86400UL - esperado) % 86400UL;
  if (d > 43200UL) d = 86400UL - d;
  segVisto = ahora;
  tVisto = millis();
  return d;
}

// Cuanto se queda en pantalla el rechazo antes de volver al menu solo. Suficiente
// para leer dos lineas sin que el equipo se quede indefinidamente en una pantalla que
// no controla nada.
static const unsigned long RECHAZO_MS = 6000;

enum EstadoDeg {
  DEG_RECHAZO,       // no se cumplian las condiciones: se dice cual falta y se vuelve
  DEG_ENTRADA_ROJO,  // todo-rojo obligatorio antes del primer verde
  DEG_ACTIVO,        // ciclando por reloj
  DEG_AMBAR,         // limite duro agotado o reloj sin contar
  DEG_SALIDA_ROJO,   // todo-rojo obligatorio antes de devolver el mando al menu
  DEG_ROJO_SIN_HORA  // D-38: la hora dejo de ser fiable; rojo fijo hasta el operario
};
static const unsigned long AVISO_ROJO_SIN_HORA_MS = 60000UL;   // D-38: repite la $ALARM
static unsigned long tAvisoRojo = 0;

static EstadoDeg estado = DEG_RECHAZO;
static MotivoDegradado motivo = MDG_OK;

// D-52 / D-51 (SPEC_2 7.quater): todo de RAM. Un corte los pierde, salvo salidaS, que va en el registro de la flash.
static uint32_t salidaS = 0;           // s desde 2000 a la que ESTA punta sale del Degradado; 0 = ninguna
static bool salidaSinFecha = false;    // sin fecha del DS3231: salidaS es la hora del dia (24:00 = 86400)
static bool salidaRespaldada = false;  // esta guardada en el registro (un corte la conserva)
static bool rojoPorOrden = false;      // el rojo fijo lo pidio una persona: solo cambia la causa de la alarma
static bool esperaRevocada = false;    // la ventana de D-29 la cerro una orden
static const uint32_t SALIDA_MAX_S = 43200UL;   // 12 h: mas es "ya vencida", como TESTIGO_INICIO_MAX_S
// La UNICA baja de salidaS: toda salida, toda entrada y la cancelacion pasan por aqui.
static void soltarSalida() { salidaS = 0; salidaRespaldada = false; salidaSinFecha = false; }
static bool salidaVencida(uint32_t sal, uint32_t ahora) { return sal != 0 && ahora >= sal; }
static unsigned long tEstado = 0;
static bool ambarArrancado = false;

// N-20: lo pone modo_degradado_reanudarTrasCorte() y lo consume el PRIMER modo_degradado_setup(). Vive aqui y no en
// un parametro porque main.cpp entra a los modos por una tabla sin argumentos; se consume para que una entrada
// posterior no herede el permiso de saltarse la puerta.
static bool reanudacionPendiente = false;

// ---------------------------------------------------------------------------
// D-29 - LA VENTANA EN LA QUE LA REANUDACION TODAVIA PUEDE DECIDIRSE (gemela de la del Esclavo).
//
// reloj_setup() arranca sin hora tras CADA corte (N-162, N-172): la trae el ESP32 por J17 unos segundos DESPUES, y
// borrar el permiso en ese arranque dejaria sin nada que reanudar cuando llega. EL BORDE (CLAUDE.md 7) es el instante
// en que bluetooth.cpp da por muda la siembra y publica $ALARM EVENTO:HORA_ESP32 (HORA_ESP32_ESPERA_MAX_MS, tres
// cadencias), con millis() a secas (ESP32 y STM32 arrancan a la vez, contrato.h). Se DERIVA del simbolo.
static const unsigned long VENTANA_REANUDACION_MS = HORA_ESP32_ESPERA_MAX_MS;

// D-29 - ¿QUEDA ALGO POR DECIDIR DE LA REANUDACION DE ESTE ARRANQUE? Bandera propia, no se deduce de
// reanudacionPendiente (que dice "hay un permiso concedido esperando a setup()"; CLAUDE.md 8). Baja en cuanto la
// decision se toma, en cualquier sentido: la reanudacion sigue siendo UNA por arranque, solo puede tardar unas
// vueltas.
static bool reanudacionPorDecidir = true;

// Declarada aqui porque la puerta la necesita y su cuerpo vive mas abajo, junto al
// resto de la logica del limite duro.
static unsigned long msDesdeSyncEfectivo();

MotivoDegradado modo_degradado_evaluarEntrada() {
  // 1. Reloj propio en hora (SFTY-18). Sin esto no hay nada que calcular: la fase
  //    sale de la hora de pared, y una hora inventada daria una fase inventada.
  //
  //    D-21 (1): Y FIABLE, no solo puesta: la MISMA pregunta que el bucle de abajo. Con la
  //    puerta mirando reloj_enHora() a secas, un equipo con la siembra caducada entraba, el
  //    telefono recibia su $ACK y la primera vuelta del bucle lo mandaba a ambar: un "si" a
  //    una orden que no se iba a cumplir (CLAUDE.md 2). Rechazado aqui, dice por que.
  if (!reloj_horaFiable()) return MDG_FALTA_HORA;

  // 2. Sincronizacion CONFIRMADA por el Esclavo y reciente. Es la condicion que de
  //    verdad sostiene el modo. Ojo con el valor centinela: nunca sincronizado
  //    devuelve 0xFFFFFFFF, no 0; leerlo como "hace 0 ms" seria dejar pasar
  //    precisamente al equipo que jamas hablo con el otro extremo.
  //    OJO CON EL ALIAS DE LA MEDIDA DE DESFASE, que se comprueba mas abajo: no es
//    "los desfases grandes se detectan y los pequenos no". La medida circular lee
//    como CERO todo multiplo de 60 s, asi que 60, 120 o 3600 s pasan la tolerancia
//    mientras 45 s si se detecta -se lee como -15 s-. El comentario anterior ponia
//    45 s de ejemplo y era falso; lo corrigio el validador el 01/08/2026.
//
//    Por eso la frescura de la sincronizacion es la garantia y el desfase solo
//    cordura: con menos de 2 h desde la ultima sync la deriva es de ~0,7 s, y un
//    minuto entero de separacion es imposible.
//
//    Se pide a msDesdeSyncEfectivo() y NO a coordinador_msDesdeUltimaSync().
  //    Aquella devuelve millis() - tUltimaSyncOk, que a los 49,7 dias da la vuelta
  //    y vuelve a numeros pequenos: la puerta se habria abierto sobre una
  //    sincronizacion de mes y medio, con cientos de segundos de deriva frente a un
  //    despeje de 30 s. msDesdeSyncEfectivo() contrasta contra el reloj de pared,
  //    que no se desborda, y se queda con el mayor de los dos.
  unsigned long desdeSync = msDesdeSyncEfectivo();
  if (desdeSync == 0xFFFFFFFFUL) return MDG_NUNCA_SYNC;
  if (desdeSync >= SYNC_FRESCA_MS) return MDG_SYNC_VIEJA;

  // 3. Que el Esclavo TENGA EL CICLO, acusado por el.
  //
  //    Faltaba, y lo detecto el validador de costura el 01/08/2026. El Esclavo si
  //    lo comprobaba por su lado (DEG_RECHAZO_SIN_CONFIG), asi que las dos puntas
  //    daban respuestas distintas a la misma peticion: el Maestro aceptaba y daba
  //    VERDE por reloj, mientras el Esclavo rechazaba, se quedaba en modo normal y
  //    caia a AMBAR por orfandad porque el Maestro ya habia callado.
  //
  //    Verde contra ambar es exactamente el escenario que este modo existe para
  //    evitar. Que una punta acepte lo que la otra rechaza no puede ocurrir.
  if (!coordinador_configConfirmada()) return MDG_SIN_CONFIG;

  // 4. Comprobacion de cordura sobre el desfase medido. Va la ultima porque es la
  //    mas debil de todas, no la mas fuerte.
  if (!coordinador_desfaseValido()) return MDG_SIN_DESFASE;
  int8_t d = coordinador_desfaseEsclavo();
  if (d > TOLERANCIA_DESFASE_S || d < -TOLERANCIA_DESFASE_S) return MDG_DESFASE_ALTO;

  return MDG_OK;
}

void modo_degradado_publicarConfig() {
  // El segundo argumento es el despeje YA AMPLIADO, que es lo que exige el contrato:
  // el Esclavo lo aplica tal cual. Es la MISMA constante que alimenta a
  // ciclo_degradado_fase() unas lineas mas abajo, de modo que no existe forma de que
  // las dos puntas acaben computando ciclos de distinta duracion.
  coordinador_enviarConfigCiclo((uint8_t)DEG_VERDE_SEG, (uint8_t)DEG_DESPEJE_SEG);

  // N-20: y a la pila, con LAS MISMAS DOS CONSTANTES que acaban de salir por radio.
  // Lo que se guarda no es un ajuste del operario -el ciclo degradado es fijo y
  // propio de este modo-, sino la constancia de que ESTE ciclo es el que se acordo
  // con la otra punta. Sirve de aval en el arranque: sin ciclo guardado no consta
  // que las dos unidades computen el mismo horario, y sin eso reanudar seria ir en
  // fase por suposicion.
  respaldo_guardarCiclo((uint8_t)DEG_VERDE_SEG, (uint8_t)DEG_DESPEJE_SEG);
}

// Antiguedad de la ultima sincronizacion, EN MILISEGUNDOS, valida tambien despues de
// un reinicio.
//
// coordinador_msDesdeUltimaSync() cuenta sobre millis(), que arranca de cero en cada
// reset: tras un corte devuelve 0xFFFFFFFF -nunca sincronizado- y el limite duro de
// mas abajo mandaria a ambar al instante, justo al equipo que acaba de reanudar en
// fase. Cuando la RAM no sabe nada se recurre a la marca de reloj de pared que
// sobrevivio en la pila.
//
// El orden importa: la RAM manda cuando existe. Es la medida directa del intercambio
// vivo con el Esclavo; la de la pila es una reconstruccion en horas enteras.
static unsigned long msDesdeSyncEfectivo() {
  unsigned long ms = coordinador_msDesdeUltimaSync();

  // El valor de RAM sale de millis() - tUltimaSyncOk. Esa resta sin signo es
  // correcta hasta 49,7 dias, y pasado ese punto DA LA VUELTA y devuelve un numero
  // pequeno.
  //
  // No es un defecto de pantalla: es la PUERTA del modo. Un equipo encendido 50
  // dias con el radio muerto, alguien manda A·B·A·B desde el suelo, y la puerta lo
  // dejaria pasar creyendo que sincronizo hace minutos. La deriva acumulada en 50
  // dias son unos 7 minutos, doscientas veces el todo-rojo.
  //
  // Se contrasta contra el reloj de pared, que NO da la vuelta, y se toma el MAYOR
  // de los dos. Si millis() se desbordo y dice "10 minutos" mientras la marca de la
  // pila dice "caducada", manda la pila. Tomar el mayor es lo conservador: nunca
  // hace parecer una sincronizacion mas fresca de lo que es.
  const uint32_t horasPila = respaldo_horasDesdeSync(reloj_contadorSegundos());

  if (ms != 0xFFFFFFFFUL) {
    // La pila solo puede SUBIR la antiguedad, NUNCA vetar una medida de RAM valida.
    //
    // La primera version traducia CADUCADA al maximo aunque la RAM tuviera una
    // medida perfecta, y eso creaba un fallo peor que el que cerraba. Lo encontro el
    // validador el 01/08/2026:
    //
    //   respaldo_horasDesdeSync() declara CADUCADA cuando el dia del mes BAJA, y en
    //   Modo Degradado el Maestro calla en la radio, asi que la marca de la pila no
    //   vuelve a refrescarse. Al cruzar fin de mes el Maestro caia a ambar -con la
    //   sincronizacion a UNA HORA de antiguedad frente a un limite de 48- mientras
    //   el Esclavo, que cuenta con millis() puro y latch, seguia dando VERDE.
    //
    //   Ambar en una punta contra verde en la otra, 24 dias al ano. Es el riesgo
    //   residual n.2 de SFTY-21, el que N-20 existe para evitar.
    //
    // CADUCADA significa "no se puede fechar", no "es viejo". Con la RAM sana, esa
    // ignorancia no aporta nada y se ignora; el desbordamiento sigue cubierto porque
    // cuando la pila SI sabe fechar, se toma el mayor de los dos.
    if (horasPila == RESPALDO_SYNC_CADUCADA) return ms;
    const unsigned long msPila = (unsigned long)horasPila * 3600000UL;
    return (msPila > ms) ? msPila : ms;
  }

  uint32_t horas = horasPila;

  // Ante la duda, caducada. respaldo_horasDesdeSync() ya declara CADUCADA todo lo que
  // no puede fechar sin ambiguedad -cambio de mes, reloj movido hacia atras, mas de
  // dos dias-, y aqui eso se traduce en el maximo, que es lo que el limite duro lee
  // como "muy vieja". Traducirlo a un numero pequeno seria autorizar el modo sobre
  // una sincronizacion de antiguedad desconocida.
  if (horas == RESPALDO_SYNC_CADUCADA) return 0xFFFFFFFFUL;
  if (horas >= LIMITE_DURO_H) return 0xFFFFFFFFUL;

  return (unsigned long)horas * 3600000UL;
}

// --- D-32 (1), 13/09: el reloj del limite, expuesto para que alguien lo PUBLIQUE ----
//
// EL PORQUE ENTERO ESTA EN EL HEADER y no se repite. Aqui solo lo que es de este fichero:
// los cuatro se apoyan en msDesdeSyncEfectivo(), que es la MISMA funcion que usan la
// puerta de entrada y el limite duro del bucle, y comparan contra LAS MISMAS DOS
// constantes que estan unas lineas mas arriba. No hay una segunda cuenta que pueda
// separarse de la primera, que es la unica forma de que el aviso siga significando lo que
// dice el dia que alguien mueva el plazo.
//
// 0xFFFFFFFF ES EL CENTINELA QUE ESTA CASA YA USA -"no se puede fechar"-, no un numero
// grande: msDesdeSyncEfectivo() lo devuelve cuando ni la RAM ni la pila saben fechar, y
// aqui se traduce a las dos respuestas que de verdad tocan. Ojo con leerlo como "muy
// viejo" en el sitio equivocado.
bool modo_degradado_huboSync() {
  return msDesdeSyncEfectivo() != 0xFFFFFFFFUL;
}

unsigned long modo_degradado_msDesdeSync() {
  return msDesdeSyncEfectivo();
}

// A-15: la sync del PAR, fresca con el mismo borde que la puerta de D-18.
bool modo_degradado_syncFresca() {
  return msDesdeSyncEfectivo() < SYNC_FRESCA_MS;
}

// SIN FECHA NO HAY AVISO, Y ES DELIBERADO: "nunca sincronizado" no es "se acerca el
// limite", es otra averia -la que la puerta de entrada rechaza con MDG_NUNCA_SYNC- y
// tiene su propio sitio en la trama. Encender el aviso aqui mezclaria las dos y mandaria
// al tecnico a mirar el radio cuando lo que pasa es que este equipo no ha hablado nunca
// con el otro. Es la misma forma que degradado_avisoLimite() del Esclavo, que pregunta
// primero por huboSyncAlguna.
bool modo_degradado_avisoLimite() {
  // D-35: con testigo el limite que manda es el suyo, y es el que se publica (SPEC_4
  // 3.ter: el aviso sale por el MISMO $EVENT, sin texto nuevo). Sin fecha cuenta como vencido.
  if (testigoEnCurso()) return edadTestigoS() >= TESTIGO_AVISO_S;
  const unsigned long ms = msDesdeSyncEfectivo();
  if (ms == 0xFFFFFFFFUL) return false;
  return ms >= AVISO_LIMITE_MS;
}

// AQUI SI CUENTA EL CENTINELA COMO VENCIDO, y es la asimetria con la de arriba: es el
// MISMO borde que aplica el bucle -`if (desdeSync >= LIMITE_DURO_MS) irAAmbar(...)`, y
// 0xFFFFFFFF lo supera-, asi que este getter contesta lo que el modo va a hacer y no una
// opinion paralela. Si contestara false sin fecha, la trama diria "no vencido" del mismo
// equipo que esta a punto de irse a ambar por esta causa.
bool modo_degradado_syncVencida() {
  if (testigoEnCurso()) return false;   // 29/09 (H9): el testigo no vence; el bucle no cae por edad
  return msDesdeSyncEfectivo() >= LIMITE_DURO_MS;
}

// D-29 (12/09): LA LLAMAN setup() Y, MIENTRAS LA DECISION SIGA PENDIENTE, EL BUCLE. La hora con la que decidir la
// trae el ESP32 despues del arranque, asi que el permiso no se tira hasta que esa siembra haya podido llegar. Lo
// diferido es el BORRADO y nada mas: el limite de 48 h manda igual y esto REANUDA un modo ya puesto (SFTY-21 sigue
// siendo manual).
bool modo_degradado_reanudarTrasCorte() {
  // D-29: tomada la decision, las llamadas siguientes salen por aqui; reanudacionPendiente NO se limpia (puede estar
  // concedida y sin consumir por setup(), que corre una vuelta despues).
  if (!reanudacionPorDecidir) return false;

  reanudacionPendiente = false;

  // Sin indicador no hay nada que reanudar. VA PRIMERO (tambien con D-29): un equipo que nunca entro en Degradado no
  // escribe en la pila por pasar por aqui.
  if (!respaldo_degradadoActivo()) { reanudacionPorDecidir = false; return false; }

  // D-29: si el equipo ya no esta donde lo dejo el arranque, la reanudacion se acabo: entre el arranque y la siembra
  // caben minutos y una persona pudo elegir un modo; meterle el Degradado encima seria la maquina revocando a
  // alguien. 🔴 Y ESTA ES, EN ESTA PUNTA, LA GUARDA DEL AMBAR DEL MANDO. Su gemela del Esclavo pregunta por
  // mando_ambarLocal(); aqui NO EXISTE (grep mando_ambarLocal sobre Maestro/{src,include}: cero): el Maestro ejecuta
  // un CAMBIO DE MODO (mando.cpp, ACC_AMBAR) y no levanta cerrojo. El arnes del Degradado compila este fichero SIN
  // mando.cpp.
  if (modoActual_get() != MENU) {
    reanudacionPorDecidir = false;
    respaldo_guardarDegradado(false);
    return false;
  }

  // D-35 - UN DEGRADADO DE TESTIGO SE REANUDA CON SU PROPIA PUERTA: registro integro, fecha del DS3231 no anterior a
  // la marca y hora fiable, sin tope de edad (H9). Si el reinicio cayo antes de inicio, setup() lo deja en rojo hasta
  // esa hora. La fecha llega con la siembra: se espera dentro de la ventana de D-29 sin borrar nada.
  if (respaldo_testigoActivo()) {
    TestigoFlash t;
    const bool hayRegistro = testigoFlash_leer(&t);
    const uint32_t ahora = reloj_segundosDesde2000();
    if (hayRegistro && ahora == 0 && millis() < VENTANA_REANUDACION_MS) return false;
    // D-52 (d): una salida ya vencida durante el corte no reanuda; una pendiente se restaura en setup().
    const bool vigente = hayRegistro && ahora != 0 && reloj_horaFiable() &&
                         ahora >= t.marcaS && !salidaVencida(t.salidaS, ahora);
    reanudacionPorDecidir = false;
    if (!vigente) {
      respaldo_guardarDegradado(false);
      return false;
    }
    testigoLeido = t;
    reanudarComoTestigo = true;
    reanudacionPendiente = true;
    return true;
  }

  // Las tres condiciones que mantienen VIGENTE la autorizacion de antes; se piden TODAS: (1) reloj propio en hora (la
  // fase sale de la hora de pared), (2) ciclo acordado en la pila (constancia de que las dos puntas computan el mismo
  // horario) y (3) sync por debajo de 48 h y FECHABLE (RESPALDO_SYNC_CADUCADA es 0xFFFFFFFF: se comprueba aparte).
  //
  // 1.49(b2) - LA CONDICION 3 NO SE PREGUNTA HASTA QUE EL CRISTAL TENGA VEREDICTO (gemela de la del Esclavo): en la
  // ventana de vigilarCristal() el contador devuelve un numero que nadie ha visto moverse. Medido el 15/09 (bloque
  // G): esta punta reanudaba sobre esa lectura y a los 4,1 s caia a ambar. Se espera CNT_VENTANA_MS mas una vuelta,
  // sin borrar nada, dentro de la ventana de D-29.
  //
  // LA CONDICION 2 SE PREGUNTA ANTES DE ESPERAR: setup() llama a esto ANTES de modo_degradado_publicarConfig(),
  // porque despues respaldo_hayCiclo() es cierto siempre; con la espera delante la condicion 2 no comprobaria nada.
  if (!respaldo_hayCiclo()) {
    reanudacionPorDecidir = false;
    respaldo_guardarDegradado(false);
    return false;
  }
  if (reloj_estadoCristal() == RELOJ_CRISTAL_VIGILANDO && millis() < VENTANA_REANUDACION_MS) {
    return false;   // sin decidir y sin borrar: se vuelve a preguntar en la siguiente vuelta
  }

  const uint32_t horas = respaldo_horasDesdeSync(reloj_contadorSegundos());
  const bool syncVigente = horas != RESPALDO_SYNC_CADUCADA && horas < LIMITE_DURO_H;

  const bool ok = reloj_enHora() && respaldo_hayCiclo() && syncVigente;

  if (!ok) {
    // D-29 - EL BORRADO SE DIFIERE, Y SOLO POR LO QUE LA SIEMBRA PUEDE ARREGLAR: falta solo la hora, el ciclo sigue
    // en la pila, la condicion 3 esta ABIERTA y la siembra aun puede llegar. Con otra cosa cerrada se borra hoy
    // igual, porque ninguna siembra arregla un ciclo no guardado ni una marca de mas de 48 h. La ventana se acota al
    // arranque (VENTANA_REANUDACION_MS desde millis()=0) y se cierra al elegir una persona un modo (la guarda de
    // arriba).
    if (!reloj_enHora() && respaldo_hayCiclo() && syncVigente &&
        millis() < VENTANA_REANUDACION_MS) {
      return false;   // sin borrar: se vuelve a preguntar en la siguiente vuelta del bucle
    }

    // Se BORRA el indicador: dejarlo puesto metaria al equipo solo en Degradado el dia que el reloj se pusiera en
    // hora.
    reanudacionPorDecidir = false;
    respaldo_guardarDegradado(false);
    return false;
  }

  reanudacionPorDecidir = false;
  reanudacionPendiente = true;
  return true;
}

// Fase del instante actual. Aisla la lectura del reloj para que el resto del modulo
// no toque nunca los segundos del dia por su cuenta.
static FaseDegradado faseAhora() {
  // D-35: el despeje es el EN USO (el del testigo, si lo hay); el verde es 180 en los dos.
  return ciclo_degradado_fase(reloj_segundosDelDia(), DEG_VERDE_SEG, despejeEnUso());
}

static void irAAmbar(const char* l1, const char* l2) {
  // N-20: el Degradado se ha ACABADO: el indicador se borra. Reanudar un modo abandonado por inseguro seria revivir
  // la decision contraria, y evita el bucle de un equipo que reintenta en cada reinicio para caer a ambar a los
  // segundos.
  respaldo_guardarDegradado(false);
  respaldo_guardarRendido(true);   // arquitecto 29/09: sin reentrada automatica hasta un PONG
  soltarSalida();                  // D-52: la rendicion es una salida
  rojoPorOrden = false;

  // Se pasa por rojo antes del ambar: nunca se salta de verde a otra cosa sin cerrar
  // el paso primero. D-45: amarillo, ese rojo y, un par de segundos despues, el ambar.
  semaforo_forzarRojo();

  // FASE 4: el motivo se fija por el setter publico de modo_ambar.cpp, que hace
  // exactamente estas dos asignaciones. Antes se escribian los dos static a pelo, y
  // era lo unico que ataba los dos modulos.
  modo_ambar_fijarMotivo(l1, l2);

  ambarArrancado = false;
  estado = DEG_AMBAR;
  tEstado = millis();
}

// D-38 (N-168): sin radio la otra punta sigue alternando por reloj; ambar aqui seria verde
// contra ambar. Rojo fijo, y la pila igual que irAAmbar(): ni se reanuda tras un corte ni
// reentra sola. Se sale por pedirSalida() (boton 4 o SET_MODO:MENU), por su todo-rojo.
// D-51: la causa publicada es ROJO_TOTAL si lo pidio una persona; el resto es identico.
static void avisarRojoFijo() {
  bluetooth_reportarAlarma("DEGRADADO", rojoPorOrden ? "ROJO_TOTAL" : "ROJO_SIN_HORA", "ROJO_FIJO");
}
static void rojoFijo() {
  respaldo_guardarRojoSinHora();   // D-47: DESPUES de bajar el Degradado, que lo borra
  respaldo_guardarRendido(true);
  semaforo_forzarRojo();
  avisarRojoFijo();
  tAvisoRojo = millis();
  estado = DEG_ROJO_SIN_HORA;
  tEstado = millis();
}
static void irARojoSinHora() {
  respaldo_guardarDegradado(false);
  rojoFijo();
}

// D-47 (02/10): arranque tras un corte con el rojo fijo por falta de hora en la pila. El
// modo vuelve en DEG_ROJO_SIN_HORA, no en el ambar de arranque de D-40: el otro poste puede
// seguir alternando por reloj. Se sale como de ese estado hoy: pedirSalida().
void modo_degradado_arrancarEnRojoSinHora() {
  testigo = false;
  motivo = MDG_OK;
  reanudacionPendiente = false;
  reanudacionPorDecidir = false;
  irARojoSinHora();
}

void modo_degradado_setup() {
  // N-20: la reanudacion se consume aqui, de una sola vez.
  const bool reanudando = reanudacionPendiente;
  reanudacionPendiente = false;

  // D-35: la entrada por testigo ya dejo el modo en rojo y con su estado; aqui no se
  // re-evalua la puerta de D-18, que la rechazaria por falta de sync de radio.
  if (entradaTestigoHecha) {
    entradaTestigoHecha = false;
    return;
  }
  const bool comoTestigo = reanudando && reanudarComoTestigo;
  reanudarComoTestigo = false;
  testigo = comoTestigo;
  soltarSalida();   // D-52: la entrada no hereda nada; solo el registro de un testigo reanudado
  rojoPorOrden = false;
  if (comoTestigo) {
    testigoDespeje = testigoLeido.despejeSeg;
    testigoMarcaS = testigoLeido.marcaS;
    testigoInicioS = testigoLeido.inicioS;
    salidaS = testigoLeido.salidaS;
    salidaRespaldada = salidaS != 0;
  }

  // Se vuelve a evaluar la puerta AQUI aunque el mando ya la haya evaluado: la entrada por pantalla no pasa por el
  // mando, y una puerta que depende de que la compruebe quien llama no es una puerta. REANUDANDO ES LA UNICA
  // EXCEPCION: la puerta mira la RAM (ultima sync, desfase), que no sobrevive al corte; aplicada tal cual rechazaria
  // SIEMPRE la reanudacion y mandaria la unidad a ambar contra la otra en verde. Lo que sobrevive ya se comprobo, con
  // el mismo limite de 48 h, en modo_degradado_reanudarTrasCorte().
  motivo = reanudando ? MDG_OK : modo_degradado_evaluarEntrada();
  tEstado = millis();

  if (motivo != MDG_OK) {
    estado = DEG_RECHAZO;
    semaforo_forzarRojo();
    // D-46: sin SET_MODO:DEGRADADO no queda entrada que llegue aqui sin reanudar; la puerta
    // se conserva para cualquier modoActual_set(MODO_DEGRADADO) futuro. Ya no hay $ERR.
    return;
  }

  // Todo-rojo en las dos puntas y ciclo detenido. Si el radio aun vive, el Esclavo recibe la orden; si no, lleva su
  // propia cuenta. Desde aqui el Maestro CALLA en la radio (main.cpp no llama al coordinador en este modo): el
  // Degradado se define por no tener radio, y seguir emitiendo dejaria que una orden vieja contradijese la fase por
  // reloj. Si el Esclavo siguiera en modo normal, dejar de hablarle lo manda a ambar por orfandad (SFTY-6), la
  // direccion segura.
  coordinador_forzarRojoTotal();
  semaforo_forzarRojo();

  // N-20: queda constancia en la pila de que este modo CORRE por decision de una persona que verifico las dos puntas:
  // es lo unico que autoriza a reanudarlo tras un corte. Se graba aqui, donde la entrada ya fue aceptada, y tambien
  // al reanudar (un registro de 16 bits sin desgaste evita que la entrada anterior quedara a medias).
  if (comoTestigo) respaldo_guardarTestigo();   // D-35: sigue siendo testigo
  else respaldo_guardarDegradado(true);

  // SE ENTRA POR TODO-ROJO TAMBIEN AL REANUDAR: DEG_ENTRADA_ROJO exige el despeje completo Y que la fase haya dejado
  // atras el verde, asi que el primer verde tras el corte es un verde entero; un equipo que arranca es el que menos
  // sabe de lo que hay en el tramo.
  anclarHora();   // D-26 (4): la referencia del salto de hora empieza aqui
  estado = DEG_ENTRADA_ROJO;
}

bool modo_degradado_pedirSalida() {
  // Ya se esta saliendo, o es la pantalla de rechazo (vuelve sola al menu): no hay ciclo que parar y reiniciar el
  // todo-rojo solo alargaria la espera de quien ya salio.
  if (estado == DEG_SALIDA_ROJO || estado == DEG_RECHAZO) return false;

  // N-20: el indicador se borra AL PULSAR, no al llegar al menu 30 s despues: es esa persona revocando la
  // autorizacion, y si la luz se fuera durante el todo-rojo de salida el equipo debe arrancar en el menu.
  respaldo_guardarDegradado(false);
  soltarSalida();   // D-52: toda salida (programada, MENU, regreso de la radio) cierra la salida y el rojo por orden
  rojoPorOrden = false;

  semaforo_forzarRojo();
  estado = DEG_SALIDA_ROJO;
  tEstado = millis();
  return true;
}

// D-51 corregida (SPEC_2 7.quater (b)): en Degradado, ROJO FIJO inmediato en ESTE poste (el otro sigue por reloj); no
// es la salida. Solo cambia la causa de la alarma (rojoPorOrden); sobrevive al corte con la bandera de D-47.
ResultadoRojoTotal modo_degradado_forzarRojo() {
  if (modoActual_get() != MODO_DEGRADADO) {
    if (!modo_degradado_revocarEsperaSiembra()) {
      coordinador_forzarRojoTotal();
      return RRT_OK;
    }
    // Ventana de D-29: rojo fijo como el arranque de D-47, nunca ambar. setup() no rehace nada.
    rojoPorOrden = true;
    entradaTestigoHecha = true;
    modoActual_set(MODO_DEGRADADO);
    modo_degradado_arrancarEnRojoSinHora();
    return RRT_REANUDACION_CANCELADA;
  }
  if (estado == DEG_ROJO_SIN_HORA) return RRT_YA_EN_ROJO_FIJO;
  if (estado == DEG_SALIDA_ROJO || estado == DEG_RECHAZO) return RRT_SALIDA_EN_CURSO;
  respaldo_guardarDegradado(false);
  rojoPorOrden = true;
  rojoFijo();
  return RRT_ROJO_FIJO;
}

// D-52: la ventana de D-29 (permiso esperando la siembra, Maestro en MENU) la cierran FORZAR_ROJO y SET_MODO:MENU.
bool modo_degradado_revocarEsperaSiembra() {
  if (!reanudacionPorDecidir || !respaldo_degradadoActivo()) return false;
  reanudacionPorDecidir = false;
  reanudacionPendiente = false;
  esperaRevocada = true;
  respaldo_guardarDegradado(false);
  return true;
}
bool modo_degradado_esperaRevocada() { return esperaRevocada; }

// ---------------------------------------------------------------------------
// D-35 - LA PUERTA DEL TESTIGO. Paralela a modo_degradado_evaluarEntrada(), que no se toca.
// El orden de los motivos es el de SPEC_2 7.bis para el Maestro.
// ---------------------------------------------------------------------------
MotivoTestigo modo_degradado_evaluarEntradaTestigo(uint32_t ahora, uint32_t inicio, int despeje) {
  if (!reloj_horaFiable() || reloj_segundosDesde2000() == 0) return MDT_FALTA_HORA;
  const uint32_t reloj = reloj_segundosDelDia();
  const int32_t d = difCircular(ahora, reloj);   // por el camino corto: 23:59:59 vs 00:00:01 = 2 s
  if (d > TOLERANCIA_TESTIGO_S || d < -TOLERANCIA_TESTIGO_S) return MDT_AHORA_DESFASADO;
  if (despeje < TESTIGO_DESPEJE_MIN || despeje > TESTIGO_DESPEJE_MAX) return MDT_DESPEJE_RANGO;
  if ((inicio + 86400UL - reloj) % 86400UL > TESTIGO_INICIO_MAX_S) return MDT_INICIO_VENCIDO;
  // R-4 en esta punta: el ambar de emergencia es MODO_AMBAR; el de arranque (D-40) no lo es.
  if (modoActual_get() == MODO_AMBAR && !modo_ambar_esDeArranque()) return MDT_AMBAR_VIGENTE;
  return MDT_OK;
}

// Escribe la flash y lo publica con lo que tardo el borrado (medida para el banco).
static bool guardarTestigoFlash(uint32_t marcaS, uint32_t inicioS, uint8_t despeje, uint32_t salida) {
  TestigoFlash t;
  t.marcaS = marcaS;
  t.inicioS = inicioS;
  t.verdeSeg = (uint8_t)DEG_VERDE_SEG;
  t.despejeSeg = despeje;
  t.salidaS = salida;
  uint32_t us = 0;
  const bool ok = testigoFlash_escribir(&t, &us);
  char det[40];
  snprintf(det, sizeof(det), "FLASH_%s_BORRADO_US:%lu", ok ? "OK" : "FALLO", (unsigned long)us);
  bluetooth_reportarEvento("TESTIGO", det);
  return ok;
}

MotivoTestigo modo_degradado_entrarTestigo(uint32_t ahora, uint32_t inicio, int despeje) {
  const MotivoTestigo m = modo_degradado_evaluarEntradaTestigo(ahora, inicio, despeje);
  if (m != MDT_OK) return m;

  const uint32_t reloj = reloj_segundosDelDia();
  const uint32_t ahoraS = reloj_segundosDesde2000();
  const uint32_t inicioS = ahoraS + (inicio + 86400UL - reloj) % 86400UL;
  const bool enModo = modoActual_get() == MODO_DEGRADADO;

  // YA ALTERNANDO CON ESTE MISMO CICLO: renueva la cuenta y sigue, sin volver a rojo ni
  // esperar inicio -la fase es la de pared y no cambia-. Vale tambien sobre un D-18 activo
  // (su despeje es 30). La flash solo se escribe con esta punta en rojo; en su verde se
  // rechaza y se repite en rojo. Lo guardado lleva inicio = ahora: ya empezo.
  if (enModo && estado == DEG_ACTIVO && despejeEnUso() == (uint8_t)despeje) {
    if (faseAhora() == FD_VERDE_MAESTRO || semaforo_estado() == S_AMARILLO) return MDT_EN_VERDE;
    soltarSalida();   // D-52: renovar es entrar de nuevo
    rojoPorOrden = false;
    if (!guardarTestigoFlash(ahoraS, ahoraS, (uint8_t)despeje, 0)) return MDT_NO_GUARDADO;
    testigo = true;
    testigoDespeje = (uint8_t)despeje;
    testigoMarcaS = ahoraS;
    testigoInicioS = ahoraS;
    respaldo_guardarTestigo();
    return MDT_RENOVADO;
  }

  // ENTRADA NUEVA (o ciclo distinto): rojo YA, y despues la flash.
  coordinador_forzarRojoTotal();
  semaforo_forzarRojo();
  soltarSalida();   // D-52: entrada nueva, sin salida ni rojo por orden heredados
  rojoPorOrden = false;
  if (!guardarTestigoFlash(ahoraS, inicioS, (uint8_t)despeje, 0)) {
    // Sin registro no hay reanudacion posible: no se entra. Si ya estaba en Degradado sale
    // por su todo-rojo; si no, al menu, que es rojo fijo.
    if (enModo) modo_degradado_pedirSalida();
    else modoActual_set(MENU);
    return MDT_NO_GUARDADO;
  }
  testigo = true;
  testigoDespeje = (uint8_t)despeje;
  testigoMarcaS = ahoraS;
  testigoInicioS = inicioS;
  respaldo_guardarTestigo();
  motivo = MDG_OK;
  reanudacionPendiente = false;
  reanudacionPorDecidir = false;   // D-29: una entrada nueva cierra la reanudacion pendiente
  anclarHora();
  estado = DEG_ENTRADA_ROJO;
  tEstado = millis();
  if (!enModo) {
    entradaTestigoHecha = true;    // el setup() que dispara el cambio de modo no rehace nada
    modoActual_set(MODO_DEGRADADO);
  }
  return MDT_OK;
}

const char* modo_degradado_textoTestigo(MotivoTestigo m) {
  switch (m) {
    case MDT_FALTA_HORA:      return "Falta: reloj sin poner en hora";
    case MDT_AHORA_DESFASADO: return "Ahora no coincide";
    case MDT_DESPEJE_RANGO:   return "Despeje fuera de rango (30-255)";
    case MDT_INICIO_VENCIDO:  return "Inicio ya vencido";
    case MDT_AMBAR_VIGENTE:   return "Ambar de emergencia puesto";
    case MDT_EN_VERDE:        return "En verde: repita en rojo";
    case MDT_NO_GUARDADO:     return "No se pudo guardar el testigo";
    default:                  return "";
  }
}

// D-52 (SPEC_2 7.quater (c)): donde se puede ordenar una salida. MDF_PROGRAMADA = si, gobierna.
static bool guardarSalida(uint32_t s) { return guardarTestigoFlash(testigoMarcaS, testigoInicioS, testigoDespeje, s); }
static MotivoSalida dondeEstoy() {
  if (modoActual_get() != MODO_DEGRADADO || estado == DEG_RECHAZO || estado == DEG_AMBAR) return MDF_NO_EN_DEGRADADO;
  return estado == DEG_SALIDA_ROJO ? MDF_YA_SALIENDO : MDF_PROGRAMADA;
}

MotivoSalida modo_degradado_programarSalida(uint32_t ahoraDia, uint32_t salidaDia) {
  const MotivoSalida donde = dondeEstoy();
  if (donde != MDF_PROGRAMADA) return donde;
  if (!reloj_horaFiable()) return MDF_FALTA_HORA;
  const uint32_t reloj = reloj_segundosDelDia();
  const int32_t d = difCircular(ahoraDia, reloj);
  if (d > TOLERANCIA_TESTIGO_S || d < -TOLERANCIA_TESTIGO_S) return MDF_AHORA_DESFASADO;
  const uint32_t falta = (salidaDia + 86400UL - reloj) % 86400UL;
  if (falta > SALIDA_MAX_S) return MDF_SALIDA_VENCIDA;
  const uint32_t ahoraS = reloj_segundosDesde2000();   // 0 = sin fecha: la salida se pasa a absoluta al llegar
  const uint32_t absS = ahoraS != 0 ? ahoraS + falta : salidaDia != 0 ? salidaDia : 86400UL;
  if (estado == DEG_ENTRADA_ROJO && testigo && ahoraS != 0 && absS <= testigoInicioS) return MDF_ANTES_DEL_INICIO;
  if (semaforo_estado() != S_ROJO) return MDF_EN_VERDE;   // la flash solo se escribe en rojo: se mira la LUZ
  const bool habia = salidaS != 0;
  const bool resp = testigo && ahoraS != 0 && estado != DEG_ROJO_SIN_HORA;   // en rojo fijo no hay respaldo
  if (resp && !guardarSalida(absS)) return MDF_NO_GUARDADO;
  salidaS = absS;
  salidaSinFecha = ahoraS == 0;
  salidaRespaldada = resp;
  return habia ? MDF_REPROGRAMADA : resp ? MDF_PROGRAMADA : MDF_PROGRAMADA_SIN_RESPALDO;
}

MotivoSalida modo_degradado_cancelarSalida() {
  const MotivoSalida donde = dondeEstoy();
  if (donde != MDF_PROGRAMADA) return donde;
  if (salidaS == 0) return MDF_NADA_QUE_CANCELAR;
  if (semaforo_estado() != S_ROJO) return MDF_EN_VERDE;
  if (salidaRespaldada && !guardarSalida(0)) return MDF_NO_GUARDADO;
  soltarSalida();
  return MDF_CANCELADA;
}

const char* modo_degradado_textoSalida(MotivoSalida m) {
  switch (m) {
    case MDF_PROGRAMADA:               return "PROGRAMADA";
    case MDF_REPROGRAMADA:             return "REPROGRAMADA";
    case MDF_PROGRAMADA_SIN_RESPALDO:  return "PROGRAMADA_SIN_RESPALDO";
    case MDF_CANCELADA:                return "CANCELADA";
    case MDF_NO_EN_DEGRADADO:          return "No esta en Degradado";
    case MDF_YA_SALIENDO:              return "Ya esta saliendo";
    case MDF_FALTA_HORA:               return "Falta: reloj sin poner en hora";
    case MDF_AHORA_DESFASADO:          return "Ahora no coincide";
    case MDF_SALIDA_VENCIDA:           return "Salida ya vencida";
    case MDF_ANTES_DEL_INICIO:         return "Salida antes del inicio";
    case MDF_EN_VERDE:                 return "En verde: repita en rojo";
    case MDF_NO_GUARDADO:              return "No se pudo guardar la salida";
    default:                           return "No hay salida programada";
  }
}

uint32_t modo_degradado_salidaS() { return modoActual_get() == MODO_DEGRADADO ? salidaS : 0; }
bool modo_degradado_salidaRespaldada() { return salidaRespaldada; }

void modo_degradado_loop() {
  // Aqui, y no en el coordinador: en este modo no se llama al coordinador, y sin esto
  // ni el ambar parpadearia ni se animarian los destellos del mando.
  semaforo_actualizar();
  // D-45: los todo-rojo de este modo -entrada, salida, el rojo previo al ambar- cuentan desde
  // el ROJO ENCENDIDO: mientras dure un amarillo de cierre, su reloj no empieza.
  if (semaforo_estado() == S_AMARILLO) tEstado = millis();

  // D-44: aqui se leia el boton 4 (salir). Del Degradado se sale por la app con
  // SET_MODO:MENU, que llama a modo_degradado_pedirSalida(): la misma salida por todo-rojo.

  // D-52: a la hora de la salida programada, la salida de siempre (rojo, despeje, baja el permiso); con la hora no
  // fiable no dispara. Antes del switch: DEG_ROJO_SIN_HORA sale de el por un return.
  if (salidaSinFecha && reloj_segundosDesde2000() != 0) {   // llego la fecha: lo que falta, sobre la hora absoluta
    const uint32_t falta = (salidaS % 86400UL + 86400UL - reloj_segundosDelDia()) % 86400UL;
    salidaS = reloj_segundosDesde2000() + (falta > SALIDA_MAX_S ? 0 : falta);   // pasada: sale ya
    salidaSinFecha = false;
  }
  if (salidaS != 0 && (estado == DEG_ENTRADA_ROJO || estado == DEG_ACTIVO || estado == DEG_ROJO_SIN_HORA) &&
      reloj_horaFiable() && reloj_segundosDesde2000() >= salidaS) {
    bluetooth_reportarEvento("DEGRADADO", "SALIDA_PROGRAMADA_EJECUTADA");
    modo_degradado_pedirSalida();
    return;
  }

  switch (estado) {

    case DEG_RECHAZO:
      // Vuelve solo al menu. Que el equipo se quede parado en una pantalla de error
      // esperando a que alguien la lea es como se pierden las obras de vista.
      if (millis() - tEstado >= RECHAZO_MS) {
        modoActual_set(MENU);
        menu_setup();
      }
      return;

    case DEG_SALIDA_ROJO:
      if (millis() - tEstado >= rojoTransicionMs()) {   // D-35: el despeje en uso
        modoActual_set(MENU);
        menu_setup();
      }
      return;

    case DEG_AMBAR:
      // Rojo un par de segundos y despues ambar. El salto directo desde un verde a un
      // ambar intermitente le diria al conductor "negocie usted" en el mismo instante
      // en que le estabamos diciendo "pase": primero se cierra, luego se avisa.
      if (!ambarArrancado && millis() - tEstado >= 2000) {
        semaforo_iniciarFallo();
        ambarArrancado = true;
      }
      return;

    case DEG_ROJO_SIN_HORA:   // D-38: rojo cada vuelta y el aviso cada minuto; sale el operario
      semaforo_forzarRojo();
      if (millis() - tAvisoRojo >= AVISO_ROJO_SIN_HORA_MS) {
        tAvisoRojo = millis();
        avisarRojoFijo();
      }
      return;

    default:
      break;
  }

  // --- A partir de aqui, DEG_ENTRADA_ROJO o DEG_ACTIVO ---------------------

  // D-21: el reloj puede dejar de ser fiable en marcha. Sin hora no hay fase que calcular, y seguir dando verdes con
  // la ultima que se recuerde seria inventar. D-38 (30/09): la punta que pierde la fiabilidad pasa a ROJO FIJO, no a
  // ambar. D-21 (1), 11/09: la guarda solo veia una hora BORRADA (reloj_enHora()); con el J17 mudo la hora seguia
  // "valida" sobre el HSI y la otra punta, sembrada de su DS3231, se separaba en minutos (H1). Ahora pregunta si la
  // hora PUEDE DECIDIR UNA LUZ (sembrada hace menos de HORA_CADUCA_MS, reloj.h). La alarma solo para la caducidad,
  // detras de reloj_enHora(): con la hora borrada por REINICIAR_RELOJ el que la borro ya tiene su $ACK. D-38:
  // irARojoSinHora(). NO SE REANUDA SOLO al volver una siembra: se abandona por la salida del operario (D-21).
  if (!reloj_horaFiable()) {
    if (reloj_enHora()) {
      bluetooth_reportarAlarma("HORA_ESP32", "CADUCADA", "CAMBIO_A_ROJO");
    }
    irARojoSinHora();
    return;
  }

  // LIMITE DURO. En cada iteracion y por delante de cualquier decision de luz: agotado el plazo, ya no importa la
  // fase. N-20: la antiguedad sale de msDesdeSyncEfectivo(), que tras un reinicio cae a la marca de la pila: se
  // cuenta desde la sincronizacion de VERDAD, no desde el arranque, o un corte a las 47 h regalaria 48 h mas.
  // 1.49(b3) - LA CAIDA SE DICE, CON SU CAUSA VERDADERA. msDesdeSyncEfectivo() devuelve 0xFFFFFFFF tambien cuando la
  // marca NO SE PUEDE FECHAR: RELOJ_NO_CUENTA (contador parado: D-49, ROJO FIJO como D-38; SIN_CRISTAL no entra
  // aqui), SYNC_SIN_FECHA (cuenta, pero la pila dice CADUCADA) y LIMITE_48H (el plazo de verdad). La mas concreta
  // primero; se publica UNA vez. Los rotulos no nombran ninguna pieza, por lo mismo que N-45 quito "Es Y2: toca
  // hardware". D-35: CON TESTIGO NO HAY TOPE DE 48 h: mide la sync de radio, que este modo existe para no necesitar.
  // Y desde el 29/09 (responsable, H9) tampoco el de 31 dias: solo el aviso.
  if (testigo) {
    avisarRenovacion();   // 29/09: a los 28 dias y despues una vez al dia
  } else {
  unsigned long desdeSync = msDesdeSyncEfectivo();
  if (desdeSync >= LIMITE_DURO_MS) {
    if (reloj_estadoCristal() == RELOJ_CRISTAL_CONGELADO) {
      // D-49 (02/10): ROJO FIJO como D-38, no ambar: el otro poste puede seguir alternando.
      bluetooth_reportarAlarma("DEGRADADO", "RELOJ_NO_CUENTA", "CAMBIO_A_ROJO");
      irARojoSinHora();
    } else if (respaldo_horasDesdeSync(reloj_contadorSegundos()) == RESPALDO_SYNC_CADUCADA &&
               desdeSync == 0xFFFFFFFFUL) {
      bluetooth_reportarAlarma("DEGRADADO", "SYNC_SIN_FECHA", "CAMBIO_A_AMBAR");
      irAAmbar("Sync sin fecha", "Sincronice de nuevo");
    } else {
      bluetooth_reportarAlarma("DEGRADADO", "LIMITE_48H", "CAMBIO_A_AMBAR");
      irAAmbar("Limite 48h sin sync", "Revise el radio");
    }
    return;
  }
  }   // D-35: fin de la rama sin testigo

  // D-26 (4): ANTES de calcular la fase y de decidir la luz. Un salto mayor que el margen
  // -una siembra del ESP32 tras mucha deriva, o un DS3231 puesto con otra hora- vuelve a
  // DEG_ENTRADA_ROJO: rojo YA, en esta misma vuelta, y el verde solo vuelve tras
  // ROJO_TRANSICION_MS y en su frontera. Tambien si ya estaba entrando: el todo-rojo se
  // cuenta de nuevo desde el salto, porque lo que habia contado era con otra hora.
  if (saltoDeHora() > SALTO_SIN_ROJO_MAX_S) {
    semaforo_forzarRojo();
    estado = DEG_ENTRADA_ROJO;
    tEstado = millis();
    bluetooth_reportarEvento("DEGRADADO", "SALTO_DE_HORA_POR_ROJO");
  }

  FaseDegradado fase = faseAhora();

  if (estado == DEG_ENTRADA_ROJO) {
    // Condiciones para arrancar, todas a la vez: 1. el todo-rojo completo (se entra desde un estado cualquiera,
    // tambien un verde, y el tramo tiene que vaciarse); 2. NO estar dentro de un verde del Maestro (se daria paso sin
    // el despeje que le precede: el primer verde es un verde entero desde su principio); 3. D-35, con testigo, haber
    // llegado a inicio (el tiempo para ir al otro poste; hasta entonces, rojo fijo), con el todo-rojo del TESTIGO.
    if (millis() - tEstado >= ROJO_TRANSICION_MS && fase != FD_VERDE_MAESTRO &&
        (!testigo || (millis() - tEstado >= rojoTransicionMs() &&
                      reloj_segundosDesde2000() >= testigoInicioS))) {
      estado = DEG_ACTIVO;
    }
    semaforo_forzarRojo();
  } else {
    // DEG_ACTIVO. La luz se deriva de la fase EN CADA ITERACION, no solo en los
    // cambios: asi, si una senal del mando ocupo las salidas un momento, al terminar
    // se vuelve a lo que manda el reloj sin necesidad de detectar nada.
    //
    // Verde SOLO en FD_VERDE_MAESTRO. En cualquier otra fase, rojo. Esta es la unica
    // linea del firmware que enciende un verde sin confirmacion del otro extremo, y
    // por eso no admite ni un caso mas.
    if (fase == FD_VERDE_MAESTRO) {
      semaforo_forzarVerde();
    } else {
      semaforo_forzarRojo();
    }
  }

  // --- Pantalla: RETIRADA POR D-32 (1) el 13/09: aqui se pintaba la fase y la cuenta atras; no decidia ninguna luz.
  // El aviso de las 48 h lo publica bluetooth.cpp en el $EVENT ORIGEN:DEGRADADO con AVISO_LIMITE_MS. Se pierde que
  // ciclo_degradado_restante() y degradado_segundosParaCambio() no tienen llamador: la cuenta atras no se publica.
}
