// ===== src/modo_degradado.cpp =====
#include "modo_degradado.h"
#include "modo_ambar.h"
#include "botones.h"
#include "ciclo_degradado.h"
#include "coordinador.h"
#include "lcd.h"
#include "menu.h"
#include "modos.h"
#include "reloj.h"
#include "respaldo.h"
#include "semaforo.h"

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
// el segundo entero con que viaja la hora la deja en 29. El sentido favorable aguanta 35
// porque los 4 s de ambar con que el Esclavo abre su verde protegen SOLO en un sentido.
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
// El verde se iguala al despeje: ciclo de 2*(30+30) = 120 s y espera maxima de 90 s
// para quien llega justo despues de que su verde acabe. Se pierde fluidez, que es
// exactamente lo que se acepta en un modo degradado.
//
// TOPE DEL BYTE. CMD_CONFIG lleva un solo byte por valor, asi que el ciclo degradado
// esta topado en 255 s por fase. Estos 30 s no se acercan al tope, y ademas el ciclo
// degradado NO hereda el verde configurado en Modo Automatico -que se mide en minutos
// y si desbordaria-: es fijo y propio de este modo. De ese modo lo que la pantalla
// cuenta es siempre el ciclo que de verdad se esta corriendo, porque la cuenta atras
// sale de estas mismas constantes a traves de ciclo_degradado_restante().
// ---------------------------------------------------------------------------
static const uint16_t DEG_VERDE_SEG = 30;
static const uint16_t DEG_DESPEJE_SEG = 30;   // YA AMPLIADO. Ver arriba.

// --- Puerta de entrada -----------------------------------------------------
//
// LA GARANTIA ES LA SINCRONIZACION RECIENTE; EL DESFASE ES SOLO COMPROBACION DE
// CORDURA. No al reves, y esto no es un matiz de redaccion:
//
//   CMD_DELTA transporta SOLO el segundo (0..59), asi que la correccion circular
//   resuelve siempre por el camino corto y el resultado cae SIEMPRE en +-30 s.
//
//   El alias NO es "los grandes se escapan y los pequenos no": es que TODO MULTIPLO
//   DE 60 s se lee como cero. Un desfase de 60, 120 o 3600 s pasa la tolerancia,
//   mientras que 45 s SI se detecta -se lee como -15 s-. Ninguna aritmetica puede
//   distinguir 0 de 60 con un solo byte de segundos.
//
//   (Aqui se afirmaba lo contrario, con 45 s de ejemplo, hasta que el validador lo
//   desmintio el 01/08/2026. Se corrige en vez de matizarlo abajo: dos comentarios
//   contradictorios en una funcion de seguridad son peores que uno equivocado.)
//
//   Lo que impide que la medida este aliasada es que la sincronizacion sea FRESCA:
//   tras una sincronizacion correcta el desfase arranca en milisegundos, y con una
//   deriva de ~100 ppm harian falta mas de tres dias para acumular los 30 s que
//   provocarian el alias. Con una sincronizacion de hace dos horas la deriva es de
//   ~0,7 s: la medida no puede estar aliasada.
//
// Confiar en el numero y no en su frescura reintroduce el fallo entero.
//
// 2 h: son dos periodos completos de resincronizacion (el coordinador reintenta cada
// hora) y coincide con la vigencia que el propio coordinador da a la medida de
// desfase. Un solo intercambio perdido no debe cerrar la puerta; dos seguidos si,
// porque entonces algo va mal de verdad.
static const unsigned long SYNC_FRESCA_MS = 7200000UL;

// +-3 s. Diez veces por debajo del todo-rojo de 30 s, y varias veces por encima del
// sesgo conocido de la medida (tiempo de aire mas el retardo de cortesia del Esclavo
// de SFTY-17, decimas de segundo). Lo que se busca aqui no es precision: es detectar
// que algo no cuadra.
static const int8_t TOLERANCIA_DESFASE_S = 3;

// --- Limite duro -----------------------------------------------------------
//
// Pasadas 48 h sin sincronizar, el Degradado cae SOLO a ambar intermitente. No es un
// aviso, es un tope: EL ESTADO SEGURO NO PUEDE DEPENDER DE QUE ALGUIEN SE ACUERDE.
// El diseno automatico que precedio a este (SFTY-19) tenia esta regla y al pasar a
// activacion manual se perdio; recuperarla no es opcional.
static const unsigned long LIMITE_DURO_MS = 172800000UL;  // 48 h
static const unsigned long AVISO_LIMITE_MS = 158400000UL; // 44 h: avisa las ultimas 4

// El mismo limite expresado en horas, para la reanudacion tras un corte: lo que
// sobrevive al reinicio es una marca de reloj de pared, y respaldo_horasDesdeSync()
// resuelve en horas enteras. Se DERIVA del valor de arriba en vez de escribir un 48
// aparte, porque dos numeros que deben ser el mismo acaban siendo distintos el dia
// que alguien toca uno solo.
static const uint32_t LIMITE_DURO_H = LIMITE_DURO_MS / 3600000UL;

// Todo-rojo minimo al entrar y al salir. Coincide con el despeje del ciclo por el
// mismo motivo por el que existe el despeje: es el tiempo que tarda en vaciarse el
// tramo. Entrar o salir mas rapido que eso seria dar por vacio algo que no lo esta.
static const unsigned long ROJO_TRANSICION_MS = (unsigned long)DEG_DESPEJE_SEG * 1000UL;

// Cuanto se queda en pantalla el rechazo antes de volver al menu solo. Suficiente
// para leer dos lineas sin que el equipo se quede indefinidamente en una pantalla que
// no controla nada.
static const unsigned long RECHAZO_MS = 6000;

enum EstadoDeg {
  DEG_RECHAZO,       // no se cumplian las condiciones: se dice cual falta y se vuelve
  DEG_ENTRADA_ROJO,  // todo-rojo obligatorio antes del primer verde
  DEG_ACTIVO,        // ciclando por reloj
  DEG_AMBAR,         // limite duro agotado o reloj perdido
  DEG_SALIDA_ROJO    // todo-rojo obligatorio antes de devolver el mando al menu
};

static EstadoDeg estado = DEG_RECHAZO;
static MotivoDegradado motivo = MDG_OK;
static unsigned long tEstado = 0;
static bool ambarArrancado = false;

// N-20: lo pone modo_degradado_reanudarTrasCorte() y lo consume el PRIMER
// modo_degradado_setup() que se ejecute. Vive aqui, y no en un parametro de setup(),
// porque main.cpp entra a los modos por una tabla comun sin argumentos; y se consume
// para que una entrada normal posterior -por pantalla o por A.B.A.B- no herede el
// permiso de saltarse la puerta.
static bool reanudacionPendiente = false;

// Ultimo dibujado, para no repintar sin necesidad: volcar el buffer de 1 KB por SPI
// software bloquea el bucle unas decenas de ms.
static FaseDegradado ultFase = FD_DESPEJE_A;
static uint32_t ultRestante = 0xFFFFFFFFUL;
static EstadoDeg ultEstadoPintado = DEG_ACTIVO;

// Declarada aqui porque la puerta la necesita y su cuerpo vive mas abajo, junto al
// resto de la logica del limite duro.
static unsigned long msDesdeSyncEfectivo();

MotivoDegradado modo_degradado_evaluarEntrada() {
  // 1. Reloj propio en hora (SFTY-18). Sin esto no hay nada que calcular: la fase
  //    sale de la hora de pared, y una hora inventada daria una fase inventada.
  if (!reloj_enHora()) return MDG_FALTA_HORA;

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

const char* modo_degradado_motivoL1(MotivoDegradado m) {
  switch (m) {
    case MDG_FALTA_HORA:   return "Falta: reloj sin";
    case MDG_NUNCA_SYNC:   return "Falta: nunca hubo";
    case MDG_SYNC_VIEJA:   return "Falta: la ultima";
    case MDG_SIN_CONFIG:   return "Falta: el esclavo";
    case MDG_SIN_DESFASE:  return "Falta: sin medida";
    case MDG_DESFASE_ALTO: return "Desfase fuera de";
    default:               return "";
  }
}

const char* modo_degradado_motivoL2(MotivoDegradado m) {
  switch (m) {
    case MDG_FALTA_HORA:   return "poner en hora";
    case MDG_NUNCA_SYNC:   return "sincronizacion RF";
    case MDG_SYNC_VIEJA:   return "sync es muy vieja";
    case MDG_SIN_CONFIG:   return "no tiene el ciclo";
    case MDG_SIN_DESFASE:  return "de desfase valida";
    case MDG_DESFASE_ALTO: return "tolerancia (+-3s)";
    default:               return "";
  }
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

bool modo_degradado_reanudarTrasCorte() {
  reanudacionPendiente = false;

  // Sin indicador no hay nada que reanudar: el equipo no estaba en Degradado, o ya
  // salio de el por su propio pie. Arranque normal, y sin tocar la pila.
  if (!respaldo_degradadoActivo()) return false;

  // Las tres condiciones que mantienen VIGENTE la autorizacion de antes. Se piden
  // TODAS, igual que en la puerta de entrada normal: no hay ninguna recomendable.
  //
  //   1. Reloj propio en hora. La fase sale de la hora de pared; sin ella no hay nada
  //      que calcular y reanudar seria inventar una fase.
  //   2. Ciclo acordado en la pila. Es la constancia de que las dos puntas computan
  //      el mismo horario. Sin eso, ir en fase seria una suposicion.
  //   3. Sincronizacion por debajo del limite duro de 48 h y FECHABLE. El centinela
  //      se comprueba aparte del numero: RESPALDO_SYNC_CADUCADA es 0xFFFFFFFF, y
  //      compararlo solo contra el limite lo dejaria pasar por "muchisimas horas"
  //      -que ya falla-, pero escribirlo explicito es lo que impide que un futuro
  //      cambio de signo en la comparacion lo convierta en "reciente".
  const uint32_t horas = respaldo_horasDesdeSync(reloj_contadorSegundos());

  const bool ok = reloj_enHora() && respaldo_hayCiclo() &&
                  horas != RESPALDO_SYNC_CADUCADA && horas < LIMITE_DURO_H;

  if (!ok) {
    // Se BORRA el indicador antes de arrancar normal. Si se dejara puesto, cada
    // reinicio volveria a intentar la reanudacion sobre una autorizacion que ya
    // caduco, y el dia que el operario pusiera el reloj en hora por otro motivo el
    // equipo se meteria solo en Degradado sin que nadie lo hubiera pedido. Eso si
    // seria entrada automatica.
    respaldo_guardarDegradado(false);
    return false;
  }

  reanudacionPendiente = true;
  return true;
}

// Fase del instante actual. Aisla la lectura del reloj para que el resto del modulo
// no toque nunca los segundos del dia por su cuenta.
static FaseDegradado faseAhora() {
  return ciclo_degradado_fase(reloj_segundosDelDia(), DEG_VERDE_SEG, DEG_DESPEJE_SEG);
}

static void irAAmbar(const char* l1, const char* l2) {
  // N-20: el Degradado se ha ACABADO, asi que el indicador se borra. Llegar aqui
  // significa que el modo se declaro insostenible -limite de 48 h agotado o reloj
  // perdido-, y reanudar despues de un corte un modo que ya se habia abandonado por
  // inseguro seria revivir la decision contraria a la que se tomo. Ademas evita el
  // bucle de un equipo que reintenta la reanudacion en cada reinicio para volver a
  // caer al ambar a los pocos segundos.
  respaldo_guardarDegradado(false);

  // Se pasa por rojo antes del ambar: nunca se salta de verde a otra cosa sin cerrar
  // el paso primero. El ambar se enciende un par de segundos despues, ya en DEG_AMBAR.
  semaforo_forzarRojo();

  // FASE 4: el motivo se fija por el setter publico de modo_ambar.cpp, que hace
  // exactamente estas dos asignaciones. Antes se escribian los dos static a pelo, y
  // era lo unico que ataba los dos modulos.
  modo_ambar_fijarMotivo(l1, l2);

  ambarArrancado = false;
  estado = DEG_AMBAR;
  tEstado = millis();
  lcd_dibujarDegradadoAmbar(l1, l2);
}

void modo_degradado_setup() {
  // N-20: la reanudacion se consume aqui, de una sola vez.
  const bool reanudando = reanudacionPendiente;
  reanudacionPendiente = false;

  // Se vuelve a evaluar la puerta AQUI aunque el mando ya la haya evaluado. La
  // entrada por pantalla no pasa por el mando, y una puerta que dependa de que la
  // compruebe quien llama no es una puerta.
  //
  // REANUDANDO ES LA UNICA EXCEPCION, y no es un atajo. Esta puerta mira la RAM
  // -coordinador_msDesdeUltimaSync() y la medida de desfase-, y la RAM no sobrevive
  // al corte: recien arrancado siempre diria MDG_NUNCA_SYNC, incluso en el equipo que
  // sincronizo hace diez minutos. Aplicarla tal cual no seria "ser estricto", seria
  // rechazar SIEMPRE la reanudacion y mandar la unidad a ambar contra una otra punta
  // en verde. Las condiciones que si sobreviven al corte ya se comprobaron, con el
  // mismo limite de 48 h, en modo_degradado_reanudarTrasCorte().
  motivo = reanudando ? MDG_OK : modo_degradado_evaluarEntrada();
  tEstado = millis();

  if (motivo != MDG_OK) {
    estado = DEG_RECHAZO;
    semaforo_forzarRojo();
    lcd_dibujarDegradadoRechazo(modo_degradado_motivoL1(motivo),
                                modo_degradado_motivoL2(motivo));
    return;
  }

  // Todo-rojo en las dos puntas y ciclo detenido. Si el radio todavia vive, el
  // Esclavo recibe la orden; si ya no, no cambia nada porque el Esclavo lleva su
  // propia cuenta.
  //
  // A partir de aqui el Maestro CALLA en la radio: main.cpp deja de llamar al
  // coordinador en este modo. Es deliberado. El Degradado se define por no tener
  // radio, y seguir emitiendo abriria la puerta a que una orden vieja contradijese
  // la fase calculada por reloj. Ademas, si el Esclavo siguiera en modo normal, dejar
  // de hablarle lo manda a ambar por orfandad (SFTY-6), que es la direccion
  // segura.
  coordinador_forzarRojoTotal();
  semaforo_forzarRojo();

  // N-20: queda constancia en la pila de que este modo esta CORRIENDO, y de que lo
  // corre por decision de una persona que verifico las dos puntas. Es lo unico que
  // despues autoriza a reanudarlo tras un corte. Se graba aqui y no al pulsar el
  // boton: aqui es donde la entrada ya ha sido aceptada.
  //
  // Al reanudar tambien se reescribe, aunque ya estuviera puesto. Cuesta un registro
  // de 16 bits sin desgaste y evita el caso de que la entrada anterior quedara a
  // medias.
  respaldo_guardarDegradado(true);

  // SE ENTRA POR TODO-ROJO TAMBIEN AL REANUDAR. No se salta a verde desde el arranque
  // ni aunque la fase que toque sea la del verde del Maestro: DEG_ENTRADA_ROJO exige
  // cumplir el despeje completo Y que la fase haya dejado atras el verde, de modo que
  // el primer verde tras el corte sea un verde entero contado desde su principio. Un
  // equipo que arranca es justo el que menos sabe de lo que hay en el tramo.
  estado = DEG_ENTRADA_ROJO;
  ultRestante = 0xFFFFFFFFUL;
  ultEstadoPintado = DEG_RECHAZO;  // fuerza el primer repintado
}

bool modo_degradado_pedirSalida() {
  // Ya se esta saliendo, o es la pantalla de rechazo -que vuelve sola al menu en
  // RECHAZO_MS y ya tiene el equipo en rojo-. En ninguno de los dos casos hay ciclo
  // que parar, y reiniciar el todo-rojo solo alargaria la espera de quien ya salio.
  if (estado == DEG_SALIDA_ROJO || estado == DEG_RECHAZO) return false;

  // N-20: el indicador se borra AL PULSAR, no al llegar al menu 30 s despues. Lo que
  // sostiene la reanudacion es que la autorizacion de una persona SIGA VIGENTE, y
  // este boton es esa persona revocandola. Si la luz se fuera durante el todo-rojo
  // de salida, el equipo debe arrancar en el menu: el operario ya decidio salir.
  respaldo_guardarDegradado(false);

  semaforo_forzarRojo();
  estado = DEG_SALIDA_ROJO;
  tEstado = millis();
  // El texto es corto a proposito: con la fuente 6x10 caben 20 caracteres por linea
  // y U8g2 recorta en silencio lo que sobre. El arnes lo comprueba.
  lcd_dibujarDegradadoAmbar("Saliendo: todo rojo", "Vea las dos puntas");
  return true;
}

void modo_degradado_loop() {
  // Aqui, y no en el coordinador: en este modo no se llama al coordinador, y sin esto
  // ni el ambar parpadearia ni se animarian los destellos del mando.
  semaforo_actualizar();

  // El flanco se lee UNA sola vez y se guarda. Consultarlo dos veces lo consumiria en
  // la primera y la segunda comprobacion no lo veria nunca: el boton parece que no
  // responde y nadie entiende por que.
  const bool salir = botonCancelar();

  // Boton 4: salir. Se pasa por todo-rojo ANTES de devolver el mando al menu, en los
  // dos sentidos de la transicion. La verificacion visual de las dos puntas es
  // obligatoria tambien AL SALIR, no solo al entrar: el escenario peligroso de este
  // modo es que una sola punta lo abandone.
  //
  // El cuerpo se mudo a modo_degradado_pedirSalida() sin tocarlo, porque el despachador
  // de Bluetooth necesita ESTA salida y no otra: dos formas de abandonar el modo serian
  // dos criterios, y en la calle mandaria el mas flojo.
  if (salir && modo_degradado_pedirSalida()) {
    return;
  }

  switch (estado) {

    case DEG_RECHAZO:
      // Vuelve solo al menu. Que el equipo se quede parado en una pantalla de error
      // esperando a que alguien la lea es como se pierden las obras de vista.
      if (salir || millis() - tEstado >= RECHAZO_MS) {
        modoActual_set(MENU);
        menu_setup();
      }
      return;

    case DEG_SALIDA_ROJO:
      if (millis() - tEstado >= ROJO_TRANSICION_MS) {
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

    default:
      break;
  }

  // --- A partir de aqui, DEG_ENTRADA_ROJO o DEG_ACTIVO ---------------------

  // El reloj puede dejar de ser fiable en marcha (pila agotada). Sin hora no hay fase
  // que calcular, y seguir dando verdes con la ultima que se recuerde seria inventar.
  if (!reloj_enHora()) {
    irAAmbar("Reloj no fiable", "Degradado detenido");
    return;
  }

  // LIMITE DURO. Se comprueba en cada iteracion y por delante de cualquier decision de
  // luz: agotado el plazo, ya no importa que fase toque.
  //
  // N-20: la antiguedad se pide a msDesdeSyncEfectivo(), que tras un reinicio cae a la
  // marca guardada en la pila. El limite se sigue contando desde la sincronizacion de
  // VERDAD, no desde el arranque: si no, un corte de luz a las 47 h regalaria 48 h mas
  // de Degradado sobre una hora que lleva dos dias sin cuadrarse, y el tope dejaria de
  // ser un tope.
  unsigned long desdeSync = msDesdeSyncEfectivo();
  if (desdeSync >= LIMITE_DURO_MS) {
    irAAmbar("Limite 48h sin sync", "Revise el radio");
    return;
  }

  FaseDegradado fase = faseAhora();

  if (estado == DEG_ENTRADA_ROJO) {
    // Dos condiciones para arrancar, y las dos hacen falta:
    //
    //   1. Haber cumplido el todo-rojo completo. Se entra desde un estado cualquiera
    //      -incluido un verde del modo anterior- y el tramo tiene que vaciarse.
    //   2. NO estar dentro de un verde del Maestro. Si se entrase a mitad de un verde
    //      se daria paso sin el despeje que le precede, que es justo lo que el
    //      despeje existe para evitar. Esperando a que la fase pase de largo, el
    //      primer verde que se de sera un verde entero contado desde su principio.
    if (millis() - tEstado >= ROJO_TRANSICION_MS && fase != FD_VERDE_MAESTRO) {
      estado = DEG_ACTIVO;
      ultEstadoPintado = DEG_ENTRADA_ROJO;  // fuerza repintado al cambiar de estado
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

  // --- Pantalla ------------------------------------------------------------
  uint32_t restante = ciclo_degradado_restante(reloj_segundosDelDia(),
                                               DEG_VERDE_SEG, DEG_DESPEJE_SEG);
  if (fase == ultFase && restante == ultRestante && estado == ultEstadoPintado) return;
  ultFase = fase;
  ultRestante = restante;
  ultEstadoPintado = estado;

  const char* textoFase;
  const char* detalle;
  if (estado == DEG_ENTRADA_ROJO) {
    textoFase = "ROJO";
    detalle = "Entrando: todo rojo";
  } else if (fase == FD_VERDE_MAESTRO) {
    textoFase = "VERDE";
    detalle = "Paso por el maestro";
  } else if (fase == FD_VERDE_ESCLAVO) {
    textoFase = "ROJO";
    detalle = "Paso por el esclavo";
  } else {
    textoFase = "ROJO";
    detalle = "Despeje total";
  }

  bool syncVencida = (desdeSync >= LIMITE_DURO_MS);

  // Aviso al acercarse al limite. Se avisa con 4 h de margen: es tiempo de sobra para
  // programar una visita, y no tanto como para que el aviso se vuelva paisaje.
  const char* aviso = (desdeSync >= AVISO_LIMITE_MS) ? "AVISO: LIMITE 48h" : 0;

  lcd_dibujarDegradado(textoFase, detalle, (unsigned long)restante,
                       desdeSync / 60000UL, syncVencida, aviso);
}
