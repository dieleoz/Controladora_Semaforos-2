// ===== src/modo_degradado.cpp (ESCLAVO) =====
#include "modo_degradado.h"
#include "bluetooth.h"      // R-4: bluetooth_ambarEmergencia(), la unica consulta que se le hace
#include "ciclo_degradado.h"
#include "config_ciclo.h"
#include "mando.h"          // D-29: mando_ambarLocal(), la guarda del camino diferido
#include "protocolo.h"
#include "reloj.h"
#include "respaldo.h"
#include "semaforo.h"

// ---------------------------------------------------------------------------
// SFTY-21 — Modo Degradado, lado Esclavo.
//
// Sin radio, la luz la decide el reloj. La fase la calcula ciclo_degradado_fase()
// -la MISMA funcion que corre en el Maestro-, y este fichero se limita a decidir
// cuando se le hace caso. Ese reparto es el corazon del asunto: si el Esclavo
// recalculara la fase "a su manera", bastaria una diferencia de un segundo en la
// frontera para que las dos puntas se dieran verde a la vez.
// ---------------------------------------------------------------------------

// El limite duro. Pasadas 48 h sin que el Maestro nos ponga en hora, el modo se
// rinde SOLO y cae a ambar intermitente.
//
// No es un aviso, es un tope, y la diferencia importa: el diseno automatico que
// esto sustituye tenia la regla y al pasar a activacion manual se perdio, dejando
// solo "la pantalla pide resincronizar". El estado seguro no puede depender de
// que alguien se acuerde.
//
// De donde salen las 48 h: dos cristales de 32.768 kHz sin calibrar y a la
// intemperie derivan hasta ~8,6 s/dia en el peor caso.
//
// EL FACTOR DE SEGURIDAD 2 QUE AQUI SE AFIRMABA ES FALSO, y la correccion es la misma
// que en el Maestro porque la cuenta era la misma. Medido el 01/09 sobre el C++ REAL de
// las dos puntas ejecutandose a la vez, cada una con su reloj: el cruce aguanta 29 s de
// desfase, el equipo puede acumular 20,2 s en 48 h, MARGEN 8,8 s, factor 1,44.
//
// Alargar el plazo sigue obligando a alargar el todo-rojo -una semana pide ~90 s, que
// destroza la fluidez del paso-, y la alternativa real sigue sin ser estirar el limite:
// es ir a arreglar el radio. Lo que cambia es que ahora hay un instrumento que
// recalcula la desigualdad desde el C++ en cada corrida, en vez de una cuenta escrita
// dentro de un comentario.
static const unsigned long LIMITE_SIN_SYNC_MS = 48UL * 3600UL * 1000UL;

// El mismo limite expresado en horas, que es la unidad en la que el respaldo sabe
// contar a traves de un reinicio. Se deriva del de arriba en vez de escribir un 48
// suelto: dos numeros que significan lo mismo se separan el dia que alguien toca uno.
static const uint32_t LIMITE_SIN_SYNC_H = LIMITE_SIN_SYNC_MS / 3600000UL;

// Aviso anticipado. Ocho horas de margen es un turno completo: da tiempo a
// programar la subida al gabinete en vez de enterarse cuando el cruce ya se
// degrado solo. Avisar mas tarde convertiria el aviso en un adorno.
static const unsigned long AVISO_SIN_SYNC_MS = 40UL * 3600UL * 1000UL;

// Suelo del todo-rojo de entrada y de salida. El valor normal es el despeje que
// mando el Maestro, pero si alguna vez llegara un despeje absurdamente corto, el
// paso por rojo seguiria existiendo de verdad y no solo en el codigo.
static const unsigned long ROJO_MINIMO_MS = 4000UL;

// Cada cuanto se relee el RTC para recalcular la fase. La fase solo puede cambiar
// en fronteras de segundo, asi que 200 ms es de sobra y evita machacar el RTC en
// un bucle que gira miles de veces por segundo.
static const unsigned long PERIODO_FASE_MS = 200UL;

// ---------------------------------------------------------------------------
// D-29 — LA VENTANA EN LA QUE LA REANUDACION TODAVIA PUEDE DECIDIRSE.
//
// Desde N-162 (11/09) la siembra ya no escribe el RTC hardware, asi que reloj_setup()
// deja horaValida en false tras CADA corte y la PRIMERA puerta de la reanudacion cierra
// dentro de setup(). La hora existe -la trae el ESP32 por J17-, pero llega DESPUES, en el
// bucle. Si el permiso de la pila se borrara en ese mismo arranque, cuando la hora llega
// ya no quedaria nada que reanudar.
//
// EL BORDE, Y POR QUE ES ESTE Y NO OTRO (CLAUDE.md 7). Es el MISMO instante en el que
// bluetooth.cpp da por muda la siembra del ESP32 y publica $ALARM EVENTO:HORA_ESP32
// -horaEsp32Vigilar(), HORA_ESP32_ESPERA_MAX_MS, tres cadencias-. O sea que el permiso se
// conserva exactamente mientras el propio firmware considera que la siembra PUEDE llegar,
// ni una vuelta mas, y en el instante en que se tira el tecnico ya tiene la alarma que
// dice por que. Cualquier otro numero seria un plazo nuevo que nadie recalcularia.
//
// SE CUENTA CON millis() A SECAS -tiempo desde el arranque de ESTE micro- porque el ESP32
// y el STM32 se encienden A LA VEZ y comparten ese origen (contrato.h lo dice y esp32_13
// lo recalcula). LO QUE CABE DENTRO, leido del calendario del ESP32 (siembra.cpp): la
// primera siembra sale a los ~1,5 s contra un puerto que aun no existe -este setup() hace
// delay(2000) y reloj_setup() espera hasta ESPERA_LSE_MS antes de bluetooth_setup()-, y
// las que SI se pueden oir son SIEMBRA_REINTENTO_1_MS (10 s), SIEMBRA_REINTENTO_2_MS
// (60 s) y la cadencia (180 s y 300 s). Cuatro oportunidades dentro de la ventana.
//
// SE DERIVA DEL SIMBOLO, NO SE COPIA EL NUMERO, por lo mismo que LIMITE_SIN_SYNC_H se
// deriva de LIMITE_SIN_SYNC_MS: dos numeros que significan lo mismo se separan el dia que
// alguien toca uno.
static const unsigned long VENTANA_REANUDACION_MS = HORA_ESP32_ESPERA_MAX_MS;

static EstadoDegradado estado = DEG_INACTIVO;
static unsigned long tCambioEstado = 0;
static bool rendicionEnCurso = false;

// POR QUE ESTO ES UNA BANDERA APARTE Y NO SE DEDUCE DE syncVencidaLatch.
//
// rendicionEnCurso contesta "esta salida termina en ambar"; ESTA contesta "por que se
// rindio", que es otra pregunta (CLAUDE.md 8: una variable que contesta a dos preguntas
// no contesta bien a ninguna). Deducirlo del latch seria justo eso: el latch dice "hoy
// hace mas de 48 h de la ultima sync", no "fue eso lo que tumbo el modo" -y despues de
// una rendicion por hora caducada el latch puede levantarse solo con el equipo ya
// rendido, con lo que el rotulo cambiaria de motivo sin que pasara nada.
//
// Se pone en los DOS caminos que rinden, pegada a su guarda, para que anadir un tercero
// obligue a decidir que rotulo lleva.
static bool rendidoPorHora = false;

// Ultima orden de luz que ESTE modulo dio. Se actua solo en los flancos, nunca en
// cada vuelta del bucle, por dos razones: no reiniciar la transicion a verde a
// cada iteracion, y no pisar al backstop de verde maximo de main.cpp. Si el
// backstop cortara a rojo, forzar verde otra vez lo dejaria inservible.
static bool verdeAplicado = false;

static bool huboSyncAlguna = false;
static unsigned long tUltimaSync = 0;
static bool syncVencidaLatch = false;

// D-29 — ¿QUEDA ALGO POR DECIDIR DE LA REANUDACION DE ESTE ARRANQUE?
//
// Contesta a UNA sola pregunta, y por eso no se deduce de ninguna otra bandera
// (CLAUDE.md 8). Vale true desde el arranque y baja EN CUANTO la decision se toma, en
// cualquiera de sus sentidos -se reanudo, se tiro el permiso, o la ventana se cerro-, de
// modo que degradado_reanudarTrasCorte() sigue siendo UNA decision por arranque: lo unico
// que cambia con D-29 es que puede tardar unas vueltas en tomarse.
//
// NO ES COSMETICA. Sin ella la funcion volveria a sembrar tUltimaSync en cada vuelta con
// la antiguedad que dice la pila, y el limite duro de 48 h se mide justo contra ese
// instante: reescribirlo con millis() en cada vuelta congelaria la antiguedad y el tope
// no venceria nunca. Un diferimiento que abriera la segunda puerta seria exactamente lo
// contrario de lo que D-29 autoriza.
static bool reanudacionPorDecidir = true;

static FaseDegradado faseCache = FD_DESPEJE_A;
static unsigned long tFaseCache = 0;

// ---------------------------------------------------------------------------
// D-26 (4) - UNA HORA QUE SALTA MAS QUE EL MARGEN DEL CRUCE SE APLICA PASANDO POR ROJO.
//
// Gemela de la del Maestro (Maestro/src/modo_degradado.cpp, con el porque entero). Aqui
// pesa igual o mas: SIN RADIO esta punta se re-siembra de su propio ESP32 cada ~5 min
// (D-26 (3)), y la PRIMERA siembra tras perder la radio puede traer de golpe todo lo que
// el HSI derivo desde la ultima hora del Maestro. Y la radio del Maestro, si llega con el
// Degradado puesto, tambien mueve la hora (CMD_HORA_S no saca de este modo).
//
// EL UMBRAL SALE DEL DESPEJE QUE MANDO EL MAESTRO -config_despejeSegundos(), el mismo
// numero con el que esta punta calcula la fase- menos el segundo del truncado: la misma
// cuenta que alli con DEG_DESPEJE_SEG, sobre el mismo valor, porque el Maestro lo manda tal
// cual (SFTY-23). esp32_13 comprueba que las dos formulas digan lo mismo.
//
// PASAR POR ROJO ES EL CAMINO QUE YA EXISTE: DEG_ENTRANDO, con rojoObligatorioMs() y la
// espera a que la fase deje atras el verde de esta punta. Y aqui el verde abre por ambar
// (aplicarLuz), asi que "directo" era ademas ambar -> verde sin despeje delante.
static uint32_t segVisto = 0;
static unsigned long tVisto = 0;

static void anclarHora() {
  segVisto = reloj_segundosDelDia();
  tVisto = millis();
}

static uint32_t saltoSinRojoMaxS() {
  const uint32_t despeje = config_despejeSegundos();
  return despeje > 0 ? despeje - 1UL : 0UL;
}

// Cuanto se ha movido la hora de pared de mas -o de menos- respecto de lo que corrio
// millis() desde la vuelta anterior, por el camino corto del circulo del dia. Re-ancla en
// cada llamada: mide saltos entre dos vueltas, no acumula, y en marcha normal da 0 o 1.
static uint32_t saltoDeHora() {
  const uint32_t ahora = reloj_segundosDelDia();
  const uint32_t esperado = (segVisto + (uint32_t)((millis() - tVisto) / 1000UL)) % 86400UL;
  uint32_t d = (ahora + 86400UL - esperado) % 86400UL;
  if (d > 43200UL) d = 86400UL - d;
  segVisto = ahora;
  tVisto = millis();
  return d;
}

// ---------------------------------------------------------------------------

// Antiguedad de la ultima sincronizacion, EN MILISEGUNDOS, fiable tambien tras un
// reinicio a medio Degradado. Espejo de msDesdeSyncEfectivo() del Maestro (N-49 T2):
// antes de esto esta punta se rendia con millis() puro durante TODO el
// funcionamiento normal, y solo consultaba la pila UNA VEZ, al arrancar
// (degradado_reanudarTrasCorte()). El Maestro, en cambio, contrasta las dos fuentes
// EN CADA VUELTA. Dos reglas distintas para la misma decision de seguridad acaban
// rindiendose en instantes distintos aunque la fecha ya sea correcta -es la misma
// familia de fallo que N-49 T1 cerro para el mes, aplicada ahora al reloj de
// programa.
//
// El orden es el mismo que en el Maestro: la RAM manda cuando existe -es la medida
// directa de la ultima terna de hora aplicada-; la pila SOLO PUEDE SUBIR la
// antiguedad, nunca vetar una medida de RAM valida.
static unsigned long msDesdeSyncEfectivo() {
  unsigned long ms = huboSyncAlguna ? (millis() - tUltimaSync) : 0xFFFFFFFFUL;
  const uint32_t horasPila = respaldo_horasDesdeSync(reloj_contadorSegundos());

  if (ms != 0xFFFFFFFFUL) {
    // CADUCADA significa "no se puede fechar", no "es viejo". Con la RAM sana esa
    // ignorancia no aporta nada y se ignora; el desbordamiento de millis() (49,7
    // dias) sigue cubierto porque cuando la pila SI sabe fechar se toma el mayor.
    if (horasPila == RESPALDO_SYNC_CADUCADA) return ms;
    const unsigned long msPila = (unsigned long)horasPila * 3600000UL;
    return (msPila > ms) ? msPila : ms;
  }

  // Sin RAM (recien arrancado y sin reanudar), la pila es lo unico que hay. Ante la
  // duda, caducada: CADUCADA o por encima del limite duro se leen igual, como
  // "nunca sincronizado".
  if (horasPila == RESPALDO_SYNC_CADUCADA || horasPila >= LIMITE_SIN_SYNC_H) return 0xFFFFFFFFUL;
  return (unsigned long)horasPila * 3600000UL;
}

static unsigned long rojoObligatorioMs() {
  unsigned long ms = (unsigned long)config_despejeSegundos() * 1000UL;
  return (ms < ROJO_MINIMO_MS) ? ROJO_MINIMO_MS : ms;
}

static FaseDegradado calcularFase() {
  unsigned long ahora = millis();
  if (ahora - tFaseCache >= PERIODO_FASE_MS) {
    tFaseCache = ahora;
    faseCache = ciclo_degradado_fase(reloj_segundosDelDia(),
                                     config_verdeSegundos(),
                                     config_despejeSegundos());
  }
  return faseCache;
}

// Regla completa del modo: en FD_VERDE_ESCLAVO verde, en cualquier otra fase
// rojo. No hay mas casos y no debe haberlos.
static void aplicarLuz(bool verde) {
  if (verde == verdeAplicado) return;
  if (verde) {
    // Misma secuencia que cuando la orden viene del Maestro (CMD_GO_GREEN):
    // rojo -> ambar -> verde. El conductor debe ver siempre lo mismo, sin que
    // importe quien decidio el cambio. Los 4 s de ambar se descuentan de NUESTRO
    // verde, nunca del todo-rojo, asi que el margen de seguridad no encoge.
    semaforo_iniciarTransicionAVerde();
  } else {
    semaforo_forzarRojo();
  }
  verdeAplicado = verde;
}

static void iniciarSalida(bool rendicion) {
  // Todo-rojo INMEDIATO. Se sale del modo estando en rojo, nunca desde verde
  // directo a otra cosa: si el modo terminara con nuestro carril en verde y la
  // luz saltara a ambar intermitente, quien ya venia lanzado se encontraria con
  // una senal que invita a negociar el paso mientras aun cree tener prioridad.
  semaforo_forzarRojo();
  verdeAplicado = false;
  rendicionEnCurso = rendicion;
  estado = DEG_SALIENDO;
  tCambioEstado = millis();

  // N-20: el indicador se baja AL EMPEZAR la salida, no al terminarla. Si la luz se
  // fuera durante el todo-rojo de despedida, reanudar al volver seria resucitar un
  // modo que ya se habia mandado apagar. Son CUATRO los caminos que llegan hasta aqui,
  // no tres: el operario, el regreso del radio, el limite duro de 48 h y -desde
  // D-21 (1)- la hora que dejo de ser fiable en marcha. Ninguna de las cuatro admite
  // marcha atras, y la lista se enumera entera a proposito: un quinto camino que
  // alguien anada tiene que chocar con ella (CLAUDE.md 2).
  respaldo_guardarDegradado(false);
}

// ---------------------------------------------------------------------------

void degradado_registrarSync() {
  huboSyncAlguna = true;
  tUltimaSync = millis();
  syncVencidaLatch = false;

  // Una sincronizacion nueva rehabilita el modo tras una rendicion, y lo hace sin
  // preguntar POR CUAL de las dos se rindio: si fue el limite duro, la deriva
  // desconocida acaba de medirse; si fue la hora no fiable (D-21 (1)), esta terna la
  // repone. Por eso el DEG_RENDIDO -> DEG_INACTIVO de abajo no mira rendidoPorHora.
  // No se vuelve a entrar solo, eso sigue siendo decision del operario.
  if (estado == DEG_RENDIDO) estado = DEG_INACTIVO;
}

bool degradado_huboSync() { return huboSyncAlguna; }

unsigned long degradado_msDesdeSync() {
  if (!huboSyncAlguna) return 0;
  return msDesdeSyncEfectivo();
}

bool degradado_syncVencida() { return syncVencidaLatch; }

bool degradado_avisoLimite() {
  if (!huboSyncAlguna) return false;
  if (syncVencidaLatch) return true;
  return msDesdeSyncEfectivo() >= AVISO_SIN_SYNC_MS;
}

RechazoDegradado degradado_comprobar() {
  // Las condiciones son OBLIGATORIAS y ninguna se nota mirando el semaforo, que
  // es justo por lo que las comprueba el firmware y no el operario.
  //
  // D-21 (1): la hora tiene que ser FIABLE, no solo estar puesta -la misma pregunta que la
  // guarda de degradado_actualizar()-. Si no, SET_MODO:DEGRADADO contestaba $ACK y el modo
  // se rendia en la vuelta siguiente: un "si" que no se iba a cumplir (CLAUDE.md 2).
  if (!reloj_horaFiable()) return DEG_RECHAZO_SIN_HORA;

  // Sin la duracion del ciclo no hay nada que calcular. El flag de recibido no es
  // lo mismo que el valor: un cero podria ser "el Maestro dijo cero" o "nunca
  // llego nada", y entrar en el segundo caso es operar a ciegas.
  if (!config_verdeRecibido() || !config_despejeRecibido()) return DEG_RECHAZO_SIN_CONFIG;
  if (config_verdeSegundos() == 0 || config_despejeSegundos() == 0) return DEG_RECHAZO_CICLO_NULO;

  // Que el RTC este en hora no prueba que ESTE Maestro nos haya sincronizado: la
  // hora sobrevive al apagado en la pila, asi que podria venir de un ajuste de
  // hace semanas, o de antes de que el equipo se moviera de obra. Sin una
  // sincronizacion recibida en esta sesion no hay base comun demostrable.
  if (!huboSyncAlguna) return DEG_RECHAZO_SIN_SYNC;

  // Y si la que hubo ya caduco, no vale reentrar. Sin esta comprobacion, el modo
  // se rendiria a las 48 h y el operario podria devolverlo al mismo estado con
  // dos pulsaciones, regalandose otras 48 h de deriva sin medir: el limite duro
  // seria un boton de posponer.
  if (syncVencidaLatch) return DEG_RECHAZO_SYNC_VENCIDA;

  // R-4 - CON UN AMBAR DE EMERGENCIA PUESTO, EL DEGRADADO NO ENTRA, Y DICE POR QUE.
  //
  // Entrar arranca con semaforo_forzarRojo() (:224) sin guarda ninguna, y eso saca la
  // luz de S_FALLO. A la vuelta siguiente la revocacion de bluetooth.cpp veia el equipo
  // fuera del ambar y tiraba el latch, con lo que los tres vetos de main.cpp (:406,
  // :416, :540) se apagaban y en la siguiente frontera de fase aplicarLuz() podia dar
  // VERDE POR RELOJ donde una persona habia pedido ambar de precaucion -alguien
  // trabajando bajo la luz, un incidente en el tramo-. El $ACK ya se habia enviado hacia
  // rato y nada se lo decia a nadie.
  //
  // Se rechaza con MOTIVO y no en silencio porque el operario esta subido al poste: un
  // "no" mudo lo manda a buscar una averia que no existe. Y no se deshace la proteccion
  // por su cuenta -la maquina no revoca lo que puso una persona-; quitarla es un acto
  // deliberado, y desde el 31/08 se puede hacer sin subir al gabinete con
  // CMD:PIN:1234:CANCELAR_AMBAR (R-3), asi que nadie queda bloqueado.
  //
  // SOLO SE MIRA EL LATCH DE BLUETOOTH, NO mando_ambarLocal(), y es deliberado: el
  // mando ya resolvio esto de otra forma y la resolvio EXPLICITAMENTE -ejecutar(
  // ACC_DEGRADADO) pone ambarLocal = false antes de llamar aqui (mando.cpp:147)-, o sea
  // que declara que entrar en Degradado revoca su propio ambar. Anadirlo a esta guarda
  // rechazaria el A.B.A.B del mando antes de que llegue a ejecutarse, porque
  // mando_registrarPulso() consulta degradado_comprobar() con la bandera todavia puesta.
  if (bluetooth_ambarEmergencia()) return DEG_RECHAZO_AMBAR_VIGENTE;

  return DEG_ACEPTADO;
}

// D-18: ESTA ES LA PUERTA UNICA DEL MODO DEGRADADO DE ESTE POSTE, y desde el 05/09 la
// llave la tiene la app. Se escribe aqui y no solo en el despachador porque el valor de
// la decision esta justamente en que NO se construyo una puerta nueva: quien llame desde
// donde llame vuelve a pasar por estas mismas condiciones. Tres vias con tres criterios
// serian una sola puerta, la mas floja de las tres, y este es el unico modo del firmware
// que enciende un verde sin confirmacion del otro extremo.
//
// Y POR ESO ESTA FUNCION DEVUELVE UN MOTIVO Y NO UN "SI O NO": quien la llame tiene que
// poder decirle al operario que le falta. Un acuse que no mire lo que esto devolvio seria
// una mentira con formato de exito.
RechazoDegradado degradado_entrar() {
  if (estado == DEG_ENTRANDO || estado == DEG_ACTIVO) return DEG_ACEPTADO;

  // Las condiciones se comprueban AQUI otra vez, y no se confia en que la
  // pantalla ya lo hiciera al pintar. Entre el repintado y la pulsacion pasan
  // segundos, y en ese hueco cabe que venza el limite de 48 h.
  RechazoDegradado r = degradado_comprobar();
  if (r != DEG_ACEPTADO) return r;

  // Todo-rojo de entrada. Se entra por rojo pase lo que pase: el equipo puede
  // venir de verde por una orden del Maestro o de ambar intermitente, y saltar de
  // ahi a un verde por reloj seria dar prioridad sin haber cerrado antes el paso.
  semaforo_forzarRojo();
  verdeAplicado = false;

  // Al abandonar el gobierno por radio se limpia el filtro de repeticion, igual
  // que se hace al caer a ambar por silencio: cuando el Maestro vuelva, su
  // contador de msgID habra dado muchas vueltas y no debe comerse su primera
  // trama por coincidir con la ultima que oimos hace horas.
  protocolo_resetReplayProtection();

  estado = DEG_ENTRANDO;
  rendicionEnCurso = false;
  tCambioEstado = millis();
  tFaseCache = millis() - PERIODO_FASE_MS;   // fuerza recalculo en la siguiente vuelta
  anclarHora();                              // D-26 (4): la referencia del salto empieza aqui

  // N-20: queda anotado en la pila que este equipo esta en Degradado. Se escribe al
  // ENTRAR y no cuando el modo lleve un rato: el microcorte que esto cubre puede
  // llegar en el segundo siguiente, y justo el todo-rojo de entrada es el tramo en el
  // que una punta reiniciada y la otra siguiendo el reloj mas se desalinean.
  respaldo_guardarDegradado(true);
  return DEG_ACEPTADO;
}

void degradado_salir() {
  // Desde RENDIDO no hay nada que apagar: ya esta en ambar. Solo se limpia el
  // cartel para que la pantalla deje de anunciar un modo que termino.
  if (estado == DEG_RENDIDO) { estado = DEG_INACTIVO; return; }
  if (estado != DEG_ENTRANDO && estado != DEG_ACTIVO) return;
  iniciarSalida(false);
}

// ---------------------------------------------------------------------------
// N-20 / D-29 — Reanudacion tras un corte. Ver el porque completo en modo_degradado.h.
//
// D-29 (12/09): YA NO SE DECIDE EN UNA SOLA VUELTA. La llaman setup() y, mientras la
// decision siga pendiente, TAMBIEN el bucle: la hora con la que hay que decidir la trae
// el ESP32 despues del arranque. Sigue siendo UNA decision por arranque -lo garantiza
// reanudacionPorDecidir-, y lo unico que se difiere es el BORRADO del permiso, nunca el
// limite duro de 48 h ni la activacion manual de SFTY-21: esto REANUDA un modo que ya
// estaba puesto, no lo enciende.
// ---------------------------------------------------------------------------
bool degradado_reanudarTrasCorte() {
  // D-29: tomada la decision -en el sentido que sea-, las llamadas siguientes del bucle
  // no hacen nada y salen por aqui.
  if (!reanudacionPorDecidir) return false;

  // Nada que reanudar: ni siquiera se toca el respaldo. Un equipo que nunca entro en
  // Degradado no debe escribir en la pila en cada arranque.
  if (!respaldo_degradadoActivo()) { reanudacionPorDecidir = false; return false; }

  // D-29: y si entretanto este modulo dejo de estar quieto -alguien entro al Degradado a
  // mano desde la app, o esta punta ya se rindio-, la reanudacion no pinta nada: manda lo
  // que se hizo DESPUES del arranque. Sin esta guarda la vuelta siguiente reescribiria
  // tUltimaSync con la antiguedad de la pila sobre un modo ya en marcha.
  if (estado != DEG_INACTIVO) { reanudacionPorDecidir = false; return false; }

  // 🔴 D-29 / D-1 — CON UN AMBAR DEL MANDO PUESTO, EL PERMISO SE TIRA Y NO SE REANUDA.
  //
  // ESTA GUARDA LA ABRIO D-29 Y POR ESO LA CIERRA D-29. degradado_comprobar() mira SOLO
  // el cerrojo de Bluetooth y NO mando_ambarLocal(), a proposito y con su motivo escrito
  // alli: con el veto dentro de la puerta, el A.B.A.B del propio mando se rechazaria
  // antes de llegar a ejecutarse. Mientras la reanudacion se decidia dentro de setup() la
  // carrera no existia -no se ha contado ni un pulso todavia-; con la decision diferida
  // hasta VENTANA_REANUDACION_MS, entre el arranque y la siembra caben minutos y en ellos
  // la bandera SI puede armarse. Se pregunta AQUI, en el camino diferido, y no alli: se
  // cierra lo que D-29 abrio sin tocar la puerta unica del modo.
  //
  // 🔴 EL MOTIVO NO ES UN OPERARIO CON EL MANDO EN LA MANO: ESE NO EXISTE. D-1 retiro el
  // hardware el 05/09 y con los pulsadores desmontados esta bandera no se arma nunca. El
  // motivo es EL COBRE: J16 p5 y p8 estan VACIOS y BOTON1/BOTON2 -PB9 y PB13, pines.h-
  // se siguen leyendo como entradas peladas, asi que lo que alguien cablee ahi compone
  // secuencias que nadie pidio (CLAUDE.md 3; A-2 y D-1). Si eso llega a armar el ambar,
  // el equipo esta en ambar intermitente por una orden que el firmware ya obedece en
  // otros cinco sitios, y reanudar por encima seria la maquina revocandola.
  //
  // Y SU MODO DE FALLO ES EL DE HOY -no reanudar-, asi que no puede empeorar nada: sin
  // esta guarda el equipo no reanudaba tampoco, solo que por el borrado que D-29 quita.
  if (mando_ambarLocal()) {
    reanudacionPorDecidir = false;
    respaldo_guardarDegradado(false);
    return false;
  }

  // LA SEGUNDA PUERTA, y desde D-29 se pregunta ENTERA aunque la primera este cerrada.
  // Puede hacerse porque no depende de la hora: es una resta de dos lecturas del contador
  // crudo del RTC (N-49). Y hace falta hacerlo, porque de ella depende que el permiso se
  // pueda conservar: lo que D-29 difiere es el BORRADO, no el limite duro.
  //
  // Las dos condiciones se comprueban por separado a proposito. CADUCADA no es un
  // numero grande, es "no se cuanto ha pasado": tratarla como una hora mas la
  // colaria por debajo del limite el dia que alguien cambie el orden de la resta.
  const uint32_t horas = respaldo_horasDesdeSync(reloj_contadorSegundos());
  const bool syncVigente = (horas != RESPALDO_SYNC_CADUCADA) && (horas < LIMITE_SIN_SYNC_H);

  if (!reloj_enHora() || !respaldo_hayCiclo() || !syncVigente) {
    // D-29 — EL BORRADO SE DIFIERE, Y SOLO POR LO QUE LA SIEMBRA PUEDE ARREGLAR.
    //
    // Se conserva el permiso UNICAMENTE si las cuatro cosas a la vez: lo unico que falta
    // es la hora, el ciclo acordado sigue en la pila, la SEGUNDA puerta esta ABIERTA -o
    // sea que el diferimiento ni la roza- y la siembra del ESP32 todavia puede llegar.
    //
    // Con cualquier otra cosa cerrada se borra hoy igual que antes, y no es rigor de
    // adorno: ninguna siembra arregla un ciclo que no esta guardado ni una marca de sync
    // de hace mas de 48 h, y un contador de RTC parado -el caso "sin cristal" de N-160-
    // devuelve CADUCADA, asi que ese equipo sigue sin reanudar y el permiso se tira en el
    // arranque, sin quedarse puesto seis minutos esperando algo que no puede pasar.
    if (!reloj_enHora() && respaldo_hayCiclo() && syncVigente &&
        millis() < VENTANA_REANUDACION_MS) {
      return false;   // sin borrar: se vuelve a preguntar en la siguiente vuelta del bucle
    }

    // Arranque normal Y BORRADO DEL INDICADOR. Sin el borrado, cada reinicio
    // reintentaria la misma comprobacion fallida, y un indicador que se queda puesto
    // acabaria disparandose el dia que un dato basura lo haga cuadrar por accidente.
    // La autorizacion caducada no se guarda "por si acaso": se tira.
    reanudacionPorDecidir = false;
    respaldo_guardarDegradado(false);
    return false;
  }

  reanudacionPorDecidir = false;

  // Se siembra el reloj del limite duro con la antiguedad REAL que el respaldo
  // conoce, no con el instante de arranque. Poner tUltimaSync = millis() regalaria
  // 48 h nuevas en cada corte de luz y convertiria el limite en un boton de posponer.
  //
  // La resta puede quedar por debajo de cero en aritmetica sin signo -millis() vale
  // pocos milisegundos aqui-, y es correcto que lo haga: todas las comparaciones del
  // modulo son de la forma (ahora - tUltimaSync), que con el desbordamiento sin signo
  // sigue dando la diferencia buena.
  //
  // D-29: SOLO SI NO HAY YA UNA MEDIDA EN RAM. Con la decision diferida puede haber
  // entrado antes una sincronizacion de verdad por radio (CMD_HORA_S), y esa es una
  // medida directa del instante en que las dos puntas volvieron a coincidir; pisarla con
  // la antiguedad de la pila seria envejecerla por una lectura peor. La pila no se
  // ignora: msDesdeSyncEfectivo() se queda con la MAYOR de las dos en cada vuelta.
  const bool sembradaAqui = !huboSyncAlguna;
  if (sembradaAqui) {
    huboSyncAlguna = true;
    tUltimaSync = millis() - horas * 3600000UL;
  }
  syncVencidaLatch = false;

  // Y se entra por la MISMA puerta que usa el operario desde la pantalla. Asi el
  // todo-rojo de entrada, el reseteo del filtro de repeticion y la revalidacion de
  // condiciones son identicos: reanudar no es un camino alternativo con reglas
  // propias, es la entrada de siempre con el permiso recuperado de la pila.
  if (degradado_entrar() != DEG_ACEPTADO) {
    if (sembradaAqui) huboSyncAlguna = false;
    respaldo_guardarDegradado(false);
    return false;
  }
  return true;
}

void degradado_actualizar() {
  const unsigned long ahora = millis();

  // El limite duro se vigila SIEMPRE, tambien con el modo apagado. Asi el latch
  // ya esta puesto cuando alguien intente entrar, en vez de dejar que entre y
  // rendirse un instante despues.
  if (huboSyncAlguna && !syncVencidaLatch && msDesdeSyncEfectivo() >= LIMITE_SIN_SYNC_MS) {
    syncVencidaLatch = true;
  }

  // D-21: Si la hora deja de ser fiable en marcha (pila agotada o reloj invalido),
  // se responde con ambar intermitente (rendicion) en vez de seguir dando verdes con hora falsa.
  //
  // D-21 (1), 11/09: HASTA HOY INALCANZABLE -en esta punta horaValida solo baja en
  // reloj_setup()-. Ahora pregunta si la hora puede decidir una luz (reloj_horaFiable(),
  // reloj.h). La alarma, con el molde de las de HORA_ESP32, solo para la caducidad -detras
  // de reloj_enHora(), igual que en el Maestro, donde la hora si se puede borrar en marcha-.
  // El camino es el que ya habia: rendicion, todo-rojo el despeje entero y despues
  // DEG_RENDIDO con el ambar de semaforo.cpp. Una siembra fresca NO devuelve el modo: de
  // DEG_RENDIDO se sale por una orden (D-21).
  if (!reloj_horaFiable() && (estado == DEG_ENTRANDO || estado == DEG_ACTIVO)) {
    if (reloj_enHora()) {
      bluetooth_reportarAlarma("HORA_ESP32", "CADUCADA", "CAMBIO_A_AMBAR");
    }
    rendidoPorHora = true;   // el rotulo de la pantalla dice el motivo, no "48h"
    iniciarSalida(true);
    return;
  }

  if (syncVencidaLatch && (estado == DEG_ENTRANDO || estado == DEG_ACTIVO)) {
    rendidoPorHora = false;
    iniciarSalida(true);
    return;
  }

  // D-26 (4): ANTES de decidir la luz. Un salto mayor que el margen devuelve a
  // DEG_ENTRANDO: rojo en esta misma vuelta, el todo-rojo contado de nuevo desde el salto y
  // la fase recalculada ya con la hora nueva.
  if ((estado == DEG_ENTRANDO || estado == DEG_ACTIVO) && saltoDeHora() > saltoSinRojoMaxS()) {
    semaforo_forzarRojo();
    verdeAplicado = false;
    estado = DEG_ENTRANDO;
    tCambioEstado = ahora;
    tFaseCache = ahora - PERIODO_FASE_MS;
    bluetooth_reportarEvento("DEGRADADO", "SALTO_DE_HORA_POR_ROJO");
  }

  switch (estado) {
    case DEG_ENTRANDO:
      // Se abandona el todo-rojo de entrada solo con DOS condiciones a la vez:
      //
      //   1. Que haya transcurrido un despeje completo. Es el margen que absorbe
      //      la deriva entre los dos relojes, y recortarlo aqui seria recortarlo
      //      justo en la transicion menos vigilada.
      //   2. Que la fase actual NO sea nuestro verde. Engancharse a mitad de un
      //      verde en curso daria una luz de duracion desconocida; asi el primer
      //      verde del modo empieza siempre en su frontera, como los demas.
      if ((ahora - tCambioEstado) >= rojoObligatorioMs() &&
          calcularFase() != FD_VERDE_ESCLAVO) {
        estado = DEG_ACTIVO;
        tCambioEstado = ahora;
      }
      break;

    case DEG_ACTIVO:
      aplicarLuz(calcularFase() == FD_VERDE_ESCLAVO);
      break;

    case DEG_SALIENDO:
      if ((ahora - tCambioEstado) >= rojoObligatorioMs()) {
        if (rendicionEnCurso) {
          // Rendicion -por CUALQUIERA de sus dos causas, que aqui ya no se
          // distinguen-: ambar intermitente, el mismo estado al que lleva la perdida
          // de radio. Este tramo es comun a las dos guardas de arriba, la del limite
          // duro y la de la hora no fiable; cual fue lo dice rendidoPorHora, y solo
          // lo necesita el rotulo. Se enciende aqui explicitamente en vez de esperar
          // a que main.cpp lo deduzca de su temporizador de 12 s, porque el motivo de
          // esta caida es otro y no debe depender de que ese temporizador este en el
          // valor adecuado.
          estado = DEG_RENDIDO;
          semaforo_iniciarFallo();
          protocolo_resetReplayProtection();
        } else {
          estado = DEG_INACTIVO;
        }
        tCambioEstado = ahora;
      }
      break;

    default:
      break;
  }
}

bool degradado_gobiernaLuz() {
  return estado == DEG_ENTRANDO || estado == DEG_ACTIVO || estado == DEG_SALIENDO;
}

EstadoDegradado degradado_estado() { return estado; }

// R-2. El porque completo esta en modo_degradado.h: DEG_SALIENDO no distingue la salida
// normal -termina en rojo- de la rendicion -termina en ambar-, y el despachador de
// Bluetooth necesita esa diferencia para contestar la verdad. Contesta "esta salida
// acaba en ambar" y NO por que: vale igual para las dos causas -limite duro y hora no
// fiable-, porque las dos pasan por el mismo iniciarSalida(true). Quien necesite el
// motivo pregunta a degradado_rendidoPorHora(), que es otra bandera a proposito.
bool degradado_rendicionEnCurso() { return rendicionEnCurso; }

// D-21 (1). El porque de que sea una bandera propia esta arriba, donde se declara.
bool degradado_rendidoPorHora() { return rendidoPorHora; }

FaseDegradado degradado_fase() { return calcularFase(); }

uint32_t degradado_segundosParaCambio() {
  if (!reloj_enHora()) return 0;
  return ciclo_degradado_restante(reloj_segundosDelDia(),
                                  config_verdeSegundos(),
                                  config_despejeSegundos());
}

// EL ROTULO DICE EL MOTIVO DE LA RENDICION, Y HAY DOS.
//
// "RENDIDO 48h" a secas MENTIA desde D-21 (1): la punta tambien se rinde cuando la hora
// deja de ser fiable, y entonces el plazo de 48 h no se ha agotado ni tiene nada que
// ver. Quien lea "48h" sale a revisar el radio; la hora caducada se arregla mirando el
// J17 y la siembra del ESP32, que es otra averia y otro viaje.
//
// Los dos caben en la linea: 19 y 18 caracteres contra los 20 que la 6x10 admite desde
// x=2 dejando una celda libre -el criterio de arnes_esclavo.cpp, que mide esta linea-.
const char* degradado_textoEstado() {
  switch (estado) {
    case DEG_INACTIVO: return "INACTIVO";
    case DEG_ENTRANDO: return "ENTRANDO: TODO ROJO";
    case DEG_ACTIVO:   return "ACTIVO (por reloj)";
    case DEG_SALIENDO: return "SALIENDO: TODO ROJO";
    case DEG_RENDIDO:  return rendidoPorHora ? "RENDIDO HORA: AMBAR"
                                             : "RENDIDO 48h: AMBAR";
  }
  return "";
}

const char* degradado_textoFase() {
  switch (calcularFase()) {
    case FD_VERDE_MAESTRO: return "Verde Maestro";
    case FD_VERDE_ESCLAVO: return "VERDE AQUI";
    default:               return "Todo rojo";
  }
}

const char* degradado_textoRechazo(RechazoDegradado motivo) {
  switch (motivo) {
    case DEG_RECHAZO_SIN_HORA:     return "SIN HORA VALIDA";
    case DEG_RECHAZO_SIN_CONFIG:   return "FALTA CONFIG CICLO";
    case DEG_RECHAZO_CICLO_NULO:   return "CICLO EN CERO";
    case DEG_RECHAZO_SIN_SYNC:     return "NUNCA SINCRONIZADO";
    case DEG_RECHAZO_SYNC_VENCIDA: return "SYNC CADUCADA >48h";
    // R-4. 18 caracteres EXACTOS, que es el techo que menu.cpp:107 declara para este
    // texto -"los 18 caracteres del motivo mas largo"- y el que ya ocupan otros tres.
    // Un motivo recortado no sirve para arreglar nada, que es justo lo que dice alli.
    case DEG_RECHAZO_AMBAR_VIGENTE: return "AMBAR EMERG.PUESTO";
    default:                       return "";
  }
}
