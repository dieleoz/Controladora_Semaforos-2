// ===== src/modo_degradado.cpp (ESCLAVO) =====
#include "modo_degradado.h"
#include "bluetooth.h"      // R-4: bluetooth_ambarEmergencia(), la unica consulta que se le hace
#include "ciclo_degradado.h"
#include "config_ciclo.h"
#include "protocolo.h"
#include "reloj.h"
#include "respaldo.h"
#include "semaforo.h"
#include "testigo_flash.h"

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

// Aviso anticipado: ocho horas es un turno completo, tiempo de programar la subida al gabinete; avisar mas tarde
// seria un adorno.
static const unsigned long AVISO_SIN_SYNC_MS = 40UL * 3600UL * 1000UL;

// Suelo del todo-rojo de entrada y de salida: el normal es el despeje del Maestro, pero un despeje absurdamente corto
// seguiria dejando un paso por rojo de verdad.
static const unsigned long ROJO_MINIMO_MS = 4000UL;

// Cada cuanto se relee el RTC para recalcular la fase: solo cambia en fronteras de segundo y 200 ms evita machacar el
// RTC.
static const unsigned long PERIODO_FASE_MS = 200UL;

// ---------------------------------------------------------------------------
// D-35 - EL DEGRADADO CON TESTIGO (SPEC_2 7.bis). Gemelo del Maestro, con el porque alli: misma tolerancia, despeje
// 30..255, vigencia con la fecha del DS3231. Con testigo la fase usa el ciclo DEL TESTIGO (verde 180, su despeje), no
// el de config_ciclo, que lo manda por radio un Maestro que en este modo no se oye.
// ---------------------------------------------------------------------------
static const int32_t  TOLERANCIA_TESTIGO_S   = 3;
static const int      TESTIGO_DESPEJE_MIN    = 30;
static const int      TESTIGO_DESPEJE_MAX    = 255;
static const uint8_t  TESTIGO_VERDE_SEG      = 180;
static const uint32_t TESTIGO_INICIO_MAX_S   = 43200UL;           // 12 h: mas es "ya paso"
// 29/09 (responsable, H9): el testigo YA NO VENCE -se retiro TESTIGO_VIGENCIA_S-. Queda el aviso.
static const uint32_t TESTIGO_AVISO_S        = 28UL * 86400UL;    // 28 dias: solo aviso

// true desde que entra un testigo hasta que el modo vuelve a DEG_INACTIVO o DEG_RENDIDO:
// la salida tambien cuenta su todo-rojo con el despeje del testigo.
static bool testigo = false;
static uint8_t testigoDespeje = 0;
static uint32_t testigoMarcaS = 0;
static uint32_t testigoInicioS = 0;

static uint8_t cicloVerde()   { return testigo ? TESTIGO_VERDE_SEG : config_verdeSegundos(); }
static uint8_t cicloDespeje() { return testigo ? testigoDespeje : config_despejeSegundos(); }

static int32_t difCircular(uint32_t a, uint32_t b) {
  int32_t d = (int32_t)((a + 86400UL - b) % 86400UL);
  return d > 43200L ? d - 86400L : d;
}

// Antiguedad del testigo; 0xFFFFFFFF si no se puede fechar. Ante la duda, se avisa.
static uint32_t edadTestigoS() {
  const uint32_t ahora = reloj_segundosDesde2000();
  if (ahora == 0 || ahora < testigoMarcaS) return 0xFFFFFFFFUL;
  return ahora - testigoMarcaS;
}

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
// D-29 — LA VENTANA EN LA QUE LA REANUDACION TODAVIA PUEDE DECIDIRSE.
//
// Desde N-162 (11/09) la siembra ya no escribe el RTC hardware: reloj_setup() deja horaValida en false tras CADA
// corte y la PRIMERA puerta de la reanudacion cierra dentro de setup(). La hora -la trae el ESP32 por J17- llega
// DESPUES, en el bucle; si el permiso de la pila se borrara en ese arranque, no quedaria nada que reanudar cuando
// llega.
//
// EL BORDE (CLAUDE.md 7) es el MISMO instante en que bluetooth.cpp da por muda la siembra del ESP32 y publica $ALARM
// EVENTO:HORA_ESP32 (horaEsp32Vigilar(), HORA_ESP32_ESPERA_MAX_MS, tres cadencias): el permiso se conserva mientras
// el firmware cree que la siembra PUEDE llegar, y al tirarse el tecnico ya tiene la alarma que dice por que. Otro
// numero seria un plazo nuevo que nadie recalcularia.
//
// SE CUENTA CON millis() A SECAS: el ESP32 y el STM32 se encienden A LA VEZ y comparten origen (contrato.h; esp32_13
// lo recalcula). Cabe la primera siembra (~1,5 s, contra un puerto que aun no existe) y las que SI se oyen:
// SIEMBRA_REINTENTO_1_MS (10 s), SIEMBRA_REINTENTO_2_MS (60 s) y la cadencia (180 s y 300 s). Se DERIVA del simbolo,
// no se copia el numero: dos numeros que significan lo mismo se separan el dia que alguien toca uno.
static const unsigned long VENTANA_REANUDACION_MS = HORA_ESP32_ESPERA_MAX_MS;

static EstadoDegradado estado = DEG_INACTIVO;
static unsigned long tCambioEstado = 0;
static bool rendicionEnCurso = false;

// D-52 (SPEC_2 7.quater): la salida programada, de RAM; un corte la conserva solo si va en el registro de la flash.
static uint32_t salidaS = 0;           // s desde 2000 a la que ESTA punta sale del Degradado; 0 = ninguna
static bool salidaSinFecha = false;    // sin fecha del DS3231: salidaS es la hora del dia (24:00 = 86400)
static bool salidaRespaldada = false;  // esta guardada en el registro
static const uint32_t SALIDA_MAX_S = 43200UL;   // 12 h: mas es "ya vencida"
static void soltarSalida() { salidaS = 0; salidaRespaldada = false; salidaSinFecha = false; }   // la unica baja
static bool salidaVencida(uint32_t sal, uint32_t ahora) { return sal != 0 && ahora >= sal; }

// Ultima orden de luz que ESTE modulo dio. Se actua solo en los flancos, nunca en
// cada vuelta del bucle, por dos razones: no reiniciar la transicion a verde a
// cada iteracion, y no pisar al backstop de verde maximo de main.cpp. Si el
// backstop cortara a rojo, forzar verde otra vez lo dejaria inservible.
static bool verdeAplicado = false;

static bool huboSyncAlguna = false;
static unsigned long tUltimaSync = 0;
static bool syncVencidaLatch = false;

// 1.49(b1) - ¿LA MEDIDA DE RAM ES PROPIA, O ES LA MARCA DE LA PILA COPIADA EN RAM? huboSyncAlguna dice "hay una
// antiguedad en RAM"; ESTA dice "de donde salio" (otra bandera, CLAUDE.md 8). La pone SOLO la reanudacion tras corte
// -tUltimaSync = millis() - horas de la pila- y la baja SOLO una sincronizacion de verdad
// (degradado_registrarSync()). Hace falta porque msDesdeSyncEfectivo() ignora una pila CADUCADA "porque la RAM esta
// sana", y una RAM sembrada de la pila no midio nada: si la pila ya no la puede fechar (contador parado, 1.22) la
// copia tampoco. Medido el 15/09 (bloque G): el Maestro, que tras un corte solo tiene la pila, caia a ambar y esta
// punta seguia en verde con su copia: dos reglas para la misma marca dan verde contra ambar. Con la bandera las dos
// contestan lo mismo con el mismo dato.
static bool syncDesdePila = false;

// D-29 - ¿QUEDA ALGO POR DECIDIR DE LA REANUDACION DE ESTE ARRANQUE? Una sola pregunta, sin deducirse de otra bandera
// (CLAUDE.md 8): true desde el arranque, baja EN CUANTO la decision se toma (se reanudo, se tiro el permiso o la
// ventana se cerro). NO ES COSMETICA: sin ella la funcion resembraria tUltimaSync en cada vuelta con la antiguedad de
// la pila, y el limite de 48 h se mide contra ese instante: congelaria la antiguedad y el tope no vencera nunca.
static bool reanudacionPorDecidir = true;

static FaseDegradado faseCache = FD_DESPEJE_A;
static unsigned long tFaseCache = 0;

// ---------------------------------------------------------------------------
// D-26 (4) - UNA HORA QUE SALTA MAS QUE EL MARGEN DEL CRUCE SE APLICA PASANDO POR ROJO.
//
// Gemela de la del Maestro (con el porque entero). Aqui pesa igual o mas: SIN RADIO esta punta se re-siembra de su
// ESP32 cada ~5 min (D-26 (3)), y la PRIMERA siembra tras perder la radio puede traer de golpe lo que el HSI derivo
// desde la ultima hora del Maestro; y CMD_HORA_S del Maestro tambien mueve la hora (no saca de este modo).
//
// EL UMBRAL SALE DEL DESPEJE QUE MANDO EL MAESTRO -config_despejeSegundos(), el numero con el que se calcula la fase-
// menos el segundo del truncado: la misma cuenta que alli sobre el mismo valor (SFTY-23); esp32_13 comprueba que las
// dos formulas digan lo mismo. PASAR POR ROJO ES EL CAMINO QUE YA EXISTE: DEG_ENTRANDO, con rojoObligatorioMs() y la
// espera a que la fase deje atras el verde de esta punta (aqui el verde abre por ambar, asi que "directo" era ambar
// -> verde).
static uint32_t segVisto = 0;
static unsigned long tVisto = 0;

static void anclarHora() {
  segVisto = reloj_segundosDelDia();
  tVisto = millis();
}

static uint32_t saltoSinRojoMaxS() {
  uint32_t despeje = config_despejeSegundos();
  // D-35: con testigo, el despeje en uso es el suyo (config_ciclo puede no tenerlo: sin el,
  // el umbral seria 0 y cada siembra mandaria a rojo).
  if (testigo) despeje = testigoDespeje;
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

// Antiguedad de la ultima sincronizacion, EN MILISEGUNDOS, fiable tambien tras un reinicio a medio Degradado. Espejo
// de msDesdeSyncEfectivo() del Maestro (N-49 T2): antes esta punta se rendia con millis() puro y consultaba la pila
// UNA vez, al arrancar, mientras el Maestro contrasta las dos fuentes EN CADA VUELTA; dos reglas para la misma
// decision de seguridad rinden en instantes distintos aunque la fecha sea correcta (la familia de fallo que N-49 T1
// cerro para el mes). Orden como en el Maestro: la RAM manda cuando existe (medida directa); la pila SOLO PUEDE SUBIR
// la antiguedad.
static unsigned long msDesdeSyncEfectivo() {
  unsigned long ms = huboSyncAlguna ? (millis() - tUltimaSync) : 0xFFFFFFFFUL;
  const uint32_t horasPila = respaldo_horasDesdeSync(reloj_contadorSegundos());

  if (ms != 0xFFFFFFFFUL) {
    // CADUCADA significa "no se puede fechar", no "es viejo". Con la RAM sana esa
    // ignorancia no aporta nada y se ignora; el desbordamiento de millis() (49,7
    // dias) sigue cubierto porque cuando la pila SI sabe fechar se toma el mayor.
    //
    // 1.49(b1): "sana" quiere decir MEDIDA AQUI. Si lo que hay en RAM es la marca de la pila
    // copiada al reanudar, la pila que ya no la fecha es la unica fuente que tenia, y se
    // contesta lo mismo que sin RAM: nunca sincronizado. Ver syncDesdePila arriba.
    if (horasPila == RESPALDO_SYNC_CADUCADA) return syncDesdePila ? 0xFFFFFFFFUL : ms;
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
  unsigned long ms = (unsigned long)cicloDespeje() * 1000UL;   // D-35: el despeje en uso
  return (ms < ROJO_MINIMO_MS) ? ROJO_MINIMO_MS : ms;
}

static FaseDegradado calcularFase() {
  unsigned long ahora = millis();
  if (ahora - tFaseCache >= PERIODO_FASE_MS) {
    tFaseCache = ahora;
    faseCache = ciclo_degradado_fase(reloj_segundosDelDia(),
                                     cicloVerde(),      // D-35: el ciclo en uso
                                     cicloDespeje());
  }
  return faseCache;
}

// Regla completa del modo: en FD_VERDE_ESCLAVO verde, en cualquier otra fase
// rojo. No hay mas casos y no debe haberlos.
static void aplicarLuz(bool verde) {
  if (verde == verdeAplicado) return;
  if (verde) {
    // Misma secuencia que cuando la orden viene del Maestro (CMD_GO_GREEN): D-45, de rojo
    // a verde DIRECTO. El conductor ve siempre lo mismo, decida quien decida el cambio.
    // El cierre lo pone la fase FD_AMARILLO_ESCLAVO por el rojo de abajo (semaforo.cpp).
    semaforo_forzarVerde();
  } else {
    semaforo_forzarRojo();
  }
  verdeAplicado = verde;
}

static void iniciarSalida(bool rendicion) {
  // Todo-rojo INMEDIATO: se sale del modo en rojo, nunca desde verde directo a otra cosa; quien venia lanzado no debe
  // encontrar una senal que invita a negociar mientras aun cree tener prioridad.
  semaforo_forzarRojo();
  verdeAplicado = false;
  rendicionEnCurso = rendicion;
  estado = DEG_SALIENDO;
  tCambioEstado = millis();

  // N-20: el indicador se baja AL EMPEZAR la salida, no al terminarla: si la luz se fuera durante el todo-rojo de
  // despedida, reanudar seria resucitar un modo mandado apagar. Son CUATRO los caminos hasta aqui: el operario, el
  // regreso del radio, el limite de 48 h y -desde D-21 (1)- la hora que dejo de ser fiable. Ninguno admite marcha
  // atras, y la lista se enumera entera a proposito: un quinto camino que alguien anada tiene que chocar con ella
  // (CLAUDE.md 2).
  respaldo_guardarDegradado(false);
  soltarSalida();   // D-52: toda salida cierra la salida programada
}

// D-38 (N-168): gemela de la del Maestro. GOBIERNA la luz (degradado_gobiernaLuz) para que la
// orfandad de main.cpp no la lleve a ambar; se sale por degradado_salir() -tramas de gobierno
// del Maestro o AMBAR_EMERGENCIA- con su todo-rojo. La pila, como la rendicion.
static const unsigned long AVISO_ROJO_SIN_HORA_MS = 60000UL;   // repite la $ALARM
static unsigned long tAvisoRojo = 0;
static void irARojoSinHora() {
  semaforo_forzarRojo();
  verdeAplicado = false;
  rendicionEnCurso = false;
  respaldo_guardarDegradado(false);
  respaldo_guardarRojoSinHora();   // D-47: DESPUES de bajar el Degradado, que lo borra
  respaldo_guardarRendido(true);
  bluetooth_reportarAlarma("DEGRADADO", "ROJO_SIN_HORA", "ROJO_FIJO");
  tAvisoRojo = millis();
  estado = DEG_ROJO_SIN_HORA;
  tCambioEstado = millis();
}

// D-47 (02/10): arranque tras un corte con el rojo fijo por falta de hora en la pila: rojo
// fijo gobernando la luz, para que la orfandad no lo lleve a ambar. Se sale como hoy.
void degradado_arrancarEnRojoSinHora() {
  testigo = false;
  reanudacionPorDecidir = false;
  irARojoSinHora();
}

// ---------------------------------------------------------------------------

void degradado_registrarSync() {
  huboSyncAlguna = true;
  tUltimaSync = millis();
  syncVencidaLatch = false;
  syncDesdePila = false;   // 1.49(b1): desde aqui la RAM es una medida propia

  // Una sincronizacion nueva rehabilita el modo tras la rendicion de 48 h: la deriva
  // desconocida acaba de medirse. No se vuelve a entrar solo: lo decide el operario.
  if (estado == DEG_RENDIDO) estado = DEG_INACTIVO;
}

bool degradado_huboSync() { return huboSyncAlguna; }

unsigned long degradado_msDesdeSync() {
  if (!huboSyncAlguna) return 0;
  return msDesdeSyncEfectivo();
}

// D-35: con testigo en curso, los dos publican la cuenta del testigo (28 aviso; no vence, H9),
// que es la que manda en este modo; el $EVENT es el mismo, sin texto nuevo (SPEC_4 3.ter).
bool degradado_syncVencida() {
  if (testigo && degradado_gobiernaLuz()) return false;   // 29/09 (H9): el testigo no vence
  return syncVencidaLatch;
}

bool degradado_avisoLimite() {
  if (testigo && degradado_gobiernaLuz()) return edadTestigoS() >= TESTIGO_AVISO_S;
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

  // R-4 - CON UN AMBAR DE EMERGENCIA PUESTO, EL DEGRADADO NO ENTRA, Y DICE POR QUE. Entrar arranca con
  // semaforo_forzarRojo() sin guarda y eso saca la luz de S_FALLO; la revocacion de bluetooth.cpp veria el equipo
  // fuera del ambar y tiraria el latch, los vetos de main.cpp se apagarian y aplicarLuz() podria dar VERDE POR RELOJ
  // donde una persona pidio ambar de precaucion (alguien trabajando bajo la luz, un incidente en el tramo), con el
  // $ACK ya enviado. Se rechaza con MOTIVO y no en silencio, porque el operario esta subido al poste y un "no" mudo
  // lo manda a buscar una averia que no existe; y no se deshace la proteccion por su cuenta (la maquina no revoca lo
  // que puso una persona): quitarla es un acto deliberado, y desde el 31/08 se hace sin subir al gabinete con
  // CMD:PIN:1234:CANCELAR_AMBAR (R-3). SOLO HAY UN LATCH QUE MIRAR, Y ES EL DE BLUETOOTH.
  if (bluetooth_ambarEmergencia()) return DEG_RECHAZO_AMBAR_VIGENTE;

  return DEG_ACEPTADO;
}

// D-18: ESTA ES LA PUERTA UNICA DEL MODO DEGRADADO DE ESTE POSTE, y desde el 05/09 la llave la tiene la app. Se
// escribe aqui y no solo en el despachador porque el valor esta en que NO se construyo una puerta nueva: quien llame
// desde donde llame vuelve a pasar por estas condiciones (tres vias con tres criterios serian una sola puerta, la mas
// floja). Devuelve un MOTIVO y no un "si o no": quien la llame debe poder decirle al operario que le falta, y un
// acuse que no mire lo que esto devolvio seria una mentira con formato de exito.
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

  soltarSalida();   // D-52: la entrada no hereda salida
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
  if (estado != DEG_ENTRANDO && estado != DEG_ACTIVO && estado != DEG_ROJO_SIN_HORA) return;
  iniciarSalida(false);   // D-38: desde el rojo fijo tambien, con el todo-rojo entero
}

// ---------------------------------------------------------------------------
// D-35 - LA ENTRADA POR TESTIGO. Copia la de degradado_entrar() (todo-rojo, filtro de
// repeticion, DEG_ENTRANDO, ancla de hora) sin tocarla: aquella es la puerta de D-18 y la
// leen por su forma varios packs. Lo que cambia es el permiso en la pila y el ciclo.
// ---------------------------------------------------------------------------
static void entrarTestigoPorRojo(uint32_t marcaS, uint32_t inicioS, uint8_t despeje) {
  testigo = true;
  testigoDespeje = despeje;
  testigoMarcaS = marcaS;
  testigoInicioS = inicioS;
  soltarSalida();   // D-52: entrada nueva; reanudarTestigo() restaura la del registro despues
  semaforo_forzarRojo();
  verdeAplicado = false;
  protocolo_resetReplayProtection();
  estado = DEG_ENTRANDO;
  rendicionEnCurso = false;
  tCambioEstado = millis();
  tFaseCache = millis() - PERIODO_FASE_MS;
  anclarHora();
  respaldo_guardarTestigo();
  reanudacionPorDecidir = false;   // D-29: una entrada cierra la reanudacion pendiente
}

// Tras un corte: vigente si hay registro integro, fecha del DS3231 no anterior a la marca y
// hora fiable, sin tope de edad (H9). Si el reinicio cayo antes de inicio, DEG_ENTRANDO lo deja en rojo hasta esa hora.
static bool reanudarTestigo() {
  TestigoFlash t;
  const bool hayRegistro = testigoFlash_leer(&t);
  const uint32_t ahora = reloj_segundosDesde2000();
  if (hayRegistro && ahora == 0 && millis() < VENTANA_REANUDACION_MS) return false;
  reanudacionPorDecidir = false;
  // D-52 (d): una salida vencida durante el corte no reanuda; una pendiente se restaura y sale a su hora.
  const bool vigente = hayRegistro && ahora != 0 && reloj_horaFiable() &&
                       ahora >= t.marcaS && !salidaVencida(t.salidaS, ahora);
  if (!vigente) {
    respaldo_guardarDegradado(false);
    return false;
  }
  entrarTestigoPorRojo(t.marcaS, t.inicioS, t.despejeSeg);
  salidaS = t.salidaS;
  salidaRespaldada = salidaS != 0;
  return true;
}

// Escribe la flash con la luz en rojo y lo publica con lo que tardo el borrado.
static bool guardarTestigoFlash(uint32_t marcaS, uint32_t inicioS, uint8_t despeje, uint32_t salida) {
  TestigoFlash t;
  t.marcaS = marcaS;
  t.inicioS = inicioS;
  t.verdeSeg = TESTIGO_VERDE_SEG;
  t.despejeSeg = despeje;
  t.salidaS = salida;
  uint32_t us = 0;
  const bool ok = testigoFlash_escribir(&t, &us);
  char det[40];
  snprintf(det, sizeof(det), "FLASH_%s_BORRADO_US:%lu", ok ? "OK" : "FALLO", (unsigned long)us);
  bluetooth_reportarEvento("TESTIGO", det);
  return ok;
}

// El orden de los motivos es el de SPEC_2 7.bis para el Esclavo.
RechazoTestigo degradado_comprobarTestigo(uint32_t ahora, uint32_t inicio, int despeje) {
  if (!reloj_horaFiable() || reloj_segundosDesde2000() == 0) return DEG_RECHAZO_T_SIN_HORA;
  const uint32_t reloj = reloj_segundosDelDia();
  const int32_t d = difCircular(ahora, reloj);
  if (d > TOLERANCIA_TESTIGO_S || d < -TOLERANCIA_TESTIGO_S) return DEG_RECHAZO_T_AHORA_DESFASADO;
  // LA QUE MUERDE: el operario llega del Maestro y el traslado ya se agoto. No se reintenta
  // con el mismo inicio; se repite el testigo entero en el Maestro (D-35).
  if ((inicio + 86400UL - reloj) % 86400UL > TESTIGO_INICIO_MAX_S) return DEG_RECHAZO_T_INICIO_VENCIDO;
  if (despeje < TESTIGO_DESPEJE_MIN || despeje > TESTIGO_DESPEJE_MAX) return DEG_RECHAZO_T_DESPEJE_RANGO;
  if (bluetooth_ambarEmergencia()) return DEG_RECHAZO_T_AMBAR_VIGENTE;   // R-4, el mismo veto
  return DEG_T_ACEPTADO;
}

RechazoTestigo degradado_entrarTestigo(uint32_t ahora, uint32_t inicio, int despeje) {
  const RechazoTestigo r = degradado_comprobarTestigo(ahora, inicio, despeje);
  if (r != DEG_T_ACEPTADO) return r;

  const uint32_t reloj = reloj_segundosDelDia();
  const uint32_t ahoraS = reloj_segundosDesde2000();
  const uint32_t inicioS = ahoraS + (inicio + 86400UL - reloj) % 86400UL;

  // Ya alternando con este mismo ciclo (testigo, o D-18 con verde 180 y ese despeje):
  // renueva y sigue, sin volver a rojo. La flash solo se escribe con esta punta en rojo.
  if (estado == DEG_ACTIVO && cicloVerde() == TESTIGO_VERDE_SEG &&
      cicloDespeje() == (uint8_t)despeje) {
    if (verdeAplicado || semaforo_estado() == S_AMARILLO) return DEG_RECHAZO_T_EN_VERDE;
    soltarSalida();   // D-52: renovar es entrar de nuevo
    if (!guardarTestigoFlash(ahoraS, ahoraS, (uint8_t)despeje, 0)) return DEG_RECHAZO_T_NO_GUARDADO;
    testigo = true;
    testigoDespeje = (uint8_t)despeje;
    testigoMarcaS = ahoraS;
    testigoInicioS = ahoraS;
    respaldo_guardarTestigo();
    return DEG_T_RENOVADO;
  }

  // Entrada nueva (o ciclo distinto): rojo YA, y despues la flash.
  semaforo_forzarRojo();
  verdeAplicado = false;
  soltarSalida();   // D-52
  if (!guardarTestigoFlash(ahoraS, inicioS, (uint8_t)despeje, 0)) {
    // Sin registro no se entra; si ya gobernaba, sale por su todo-rojo.
    if (estado == DEG_ENTRANDO || estado == DEG_ACTIVO) iniciarSalida(false);
    return DEG_RECHAZO_T_NO_GUARDADO;
  }
  entrarTestigoPorRojo(ahoraS, inicioS, (uint8_t)despeje);
  return DEG_T_ACEPTADO;
}

const char* degradado_textoRechazoTestigo(RechazoTestigo r) {
  switch (r) {
    case DEG_RECHAZO_T_SIN_HORA:        return "Falta: reloj sin poner en hora";
    case DEG_RECHAZO_T_AHORA_DESFASADO: return "Ahora no coincide";
    case DEG_RECHAZO_T_INICIO_VENCIDO:  return "Inicio ya vencido: repita el testigo en el Maestro";
    case DEG_RECHAZO_T_DESPEJE_RANGO:   return "Despeje fuera de rango (30-255)";
    case DEG_RECHAZO_T_AMBAR_VIGENTE:   return "Ambar de emergencia puesto";
    case DEG_RECHAZO_T_EN_VERDE:        return "En verde: repita en rojo";
    case DEG_RECHAZO_T_NO_GUARDADO:     return "No se pudo guardar el testigo";
    default:                            return "";
  }
}

// D-52 (SPEC_2 7.quater (c)): donde se puede ordenar una salida. MDF_PROGRAMADA = si, gobierna.
static bool guardarSalida(uint32_t s) { return guardarTestigoFlash(testigoMarcaS, testigoInicioS, testigoDespeje, s); }
static MotivoSalida dondeEstoy() {
  if (estado == DEG_INACTIVO || estado == DEG_RENDIDO) return MDF_NO_EN_DEGRADADO;
  return estado == DEG_SALIENDO ? MDF_YA_SALIENDO : MDF_PROGRAMADA;
}

MotivoSalida modo_degradado_programarSalida(uint32_t ahoraDia, uint32_t salidaDia) {
  const MotivoSalida donde = dondeEstoy();
  if (donde != MDF_PROGRAMADA) return donde;
  if (!reloj_horaFiable()) return MDF_FALTA_HORA;
  const uint32_t reloj = reloj_segundosDelDia();
  const int32_t d = difCircular(ahoraDia, reloj);
  if (d > TOLERANCIA_TESTIGO_S || d < -TOLERANCIA_TESTIGO_S) return MDF_AHORA_DESFASADO;
  const uint32_t falta = (salidaDia + 86400UL - reloj) % 86400UL;
  if (falta > SALIDA_MAX_S) return MDF_SALIDA_VENCIDA;   // en el Esclavo es la que muerde: reprograme en el Maestro
  const uint32_t ahoraS = reloj_segundosDesde2000();   // 0 = sin fecha: la salida se pasa a absoluta al llegar
  const uint32_t absS = ahoraS != 0 ? ahoraS + falta : salidaDia != 0 ? salidaDia : 86400UL;
  if (estado == DEG_ENTRANDO && testigo && ahoraS != 0 && absS <= testigoInicioS) return MDF_ANTES_DEL_INICIO;
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
    case MDF_SALIDA_VENCIDA:           return "Salida ya vencida: reprograme la salida en el Maestro";
    case MDF_ANTES_DEL_INICIO:         return "Salida antes del inicio";
    case MDF_EN_VERDE:                 return "En verde: repita en rojo";
    case MDF_NO_GUARDADO:              return "No se pudo guardar la salida";
    default:                           return "No hay salida programada";
  }
}

uint32_t modo_degradado_salidaS() { return salidaS; }
bool modo_degradado_salidaRespaldada() { return salidaRespaldada; }

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

  // D-35 - UN TESTIGO SE REANUDA CON SU PROPIA PUERTA (el porque, en el gemelo del Maestro):
  // la fecha del DS3231, sin tope de edad (H9), sin CNT ni la sync de radio. Se espera la fecha de la
  // siembra dentro de la ventana de D-29, sin borrar nada.
  if (respaldo_testigoActivo()) return reanudarTestigo();

  // 1.49(b2) - LA SEGUNDA PUERTA NO SE PREGUNTA HASTA QUE EL CRISTAL TENGA VEREDICTO. Se fecha con
  // reloj_contadorSegundos(), que durante la ventana de vigilarCristal() devuelve un numero que nadie ha visto
  // moverse: con un Y2 que arranca y no cuenta la resta de la pila sale "0 h". Medido el 15/09 (bloque G): con la
  // siembra del ESP32 dentro de esa ventana esta punta reanudaba y daba 273 s de verde por reloj frente al ambar del
  // Maestro. Pasada la ventana, un contador parado devuelve 0 (CADUCADA) y la puerta cierra sola (N-160, D-29). No
  // choca con D-29: espera como mucho CNT_VENTANA_MS mas una vuelta, sin borrar nada, dentro de
  // VENTANA_REANUDACION_MS, que la acota por si el veredicto no llegara: pasado el plazo se decide con lo que haya.
  if (reloj_estadoCristal() == RELOJ_CRISTAL_VIGILANDO && millis() < VENTANA_REANUDACION_MS) {
    return false;   // sin decidir y sin borrar: se vuelve a preguntar en la siguiente vuelta
  }

  // LA SEGUNDA PUERTA, y desde D-29 se pregunta ENTERA aunque la primera este cerrada: no depende de la hora (es una
  // resta de dos lecturas del contador crudo, N-49) y de ella depende que el permiso se pueda conservar. Las
  // condiciones van por separado: CADUCADA no es un numero grande sino "no se cuanto ha pasado", y tratarla como una
  // hora mas la colaria por debajo del limite el dia que alguien cambie el orden de la resta.
  const uint32_t horas = respaldo_horasDesdeSync(reloj_contadorSegundos());
  const bool syncVigente = (horas != RESPALDO_SYNC_CADUCADA) && (horas < LIMITE_SIN_SYNC_H);

  if (!reloj_enHora() || !respaldo_hayCiclo() || !syncVigente) {
    // D-29 - EL BORRADO SE DIFIERE, Y SOLO POR LO QUE LA SIEMBRA PUEDE ARREGLAR: falta solo la hora, el ciclo sigue
    // en la pila, la SEGUNDA puerta esta ABIERTA y la siembra aun puede llegar. Con otra cosa cerrada se borra hoy
    // igual: ninguna siembra arregla un ciclo no guardado ni una marca de mas de 48 h, y un contador parado (N-160,
    // "sin cristal") devuelve CADUCADA, asi que ese equipo no se queda seis minutos con el permiso puesto esperando
    // algo imposible.
    if (!reloj_enHora() && respaldo_hayCiclo() && syncVigente &&
        millis() < VENTANA_REANUDACION_MS) {
      return false;   // sin borrar: se vuelve a preguntar en la siguiente vuelta del bucle
    }

    // Arranque normal Y BORRADO DEL INDICADOR: sin el, cada reinicio reintentaria la comprobacion fallida y un dato
    // basura podria hacerla cuadrar por accidente. La autorizacion caducada no se guarda "por si acaso": se tira.
    reanudacionPorDecidir = false;
    respaldo_guardarDegradado(false);
    return false;
  }

  reanudacionPorDecidir = false;

  // Se siembra el reloj del limite duro con la antiguedad REAL de la pila, no con el arranque: tUltimaSync = millis()
  // regalaria 48 h nuevas en cada corte y el limite seria un boton de posponer. La resta puede quedar bajo cero en
  // aritmetica sin signo y es correcto: todas las comparaciones son (ahora - tUltimaSync).
  //
  // D-29: SOLO SI NO HAY YA UNA MEDIDA EN RAM. Con la decision diferida pudo entrar antes una sync de verdad por
  // radio (CMD_HORA_S), medida directa del instante en que las dos puntas coincidieron; pisarla con la pila seria
  // envejecerla por una lectura peor. La pila no se ignora: msDesdeSyncEfectivo() se queda con la MAYOR de las dos en
  // cada vuelta.
  const bool sembradaAqui = !huboSyncAlguna;
  if (sembradaAqui) {
    huboSyncAlguna = true;
    tUltimaSync = millis() - horas * 3600000UL;
    syncDesdePila = true;   // 1.49(b1): es la marca de la pila, no una medida de esta RAM
  }
  syncVencidaLatch = false;

  // Y se entra por la MISMA puerta que usa el operario desde la pantalla. Asi el
  // todo-rojo de entrada, el reseteo del filtro de repeticion y la revalidacion de
  // condiciones son identicos: reanudar no es un camino alternativo con reglas
  // propias, es la entrada de siempre con el permiso recuperado de la pila.
  if (degradado_entrar() != DEG_ACEPTADO) {
    if (sembradaAqui) { huboSyncAlguna = false; syncDesdePila = false; }
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
  // D-45: los todo-rojo de entrada y de salida cuentan desde el ROJO ENCENDIDO.
  if (semaforo_estado() == S_AMARILLO) tCambioEstado = ahora;

  // D-21: si la hora deja de ser fiable en marcha no se siguen dando verdes con hora falsa. D-38 (30/09): ROJO FIJO,
  // no ambar (irARojoSinHora). D-21 (1), 11/09: HASTA HOY INALCANZABLE (horaValida solo baja en reloj_setup()); ahora
  // pregunta si la hora puede decidir una luz (reloj_horaFiable(), reloj.h). La alarma, con el molde de HORA_ESP32,
  // solo para la caducidad -detras de reloj_enHora(), como en el Maestro-. Una siembra fresca NO devuelve el modo: de
  // DEG_ROJO_SIN_HORA se sale por una orden (D-21).
  if (!reloj_horaFiable() && (estado == DEG_ENTRANDO || estado == DEG_ACTIVO)) {
    if (reloj_enHora()) {
      bluetooth_reportarAlarma("HORA_ESP32", "CADUCADA", "CAMBIO_A_ROJO");
    }
    irARojoSinHora();
    return;
  }

  // D-35: EL CERROJO DE 48 h NO ACTUA EN MODO TESTIGO -mide la sync de radio, que este modo
  // existe para no necesitar-; y desde el 29/09 (H9) el testigo no tiene tope: solo el aviso.
  if (!testigo && syncVencidaLatch && (estado == DEG_ENTRANDO || estado == DEG_ACTIVO)) {
    // D-49 (02/10): si lo que vencio es un reloj que dejo de contar -la marca de la pila ya
    // no se puede fechar-, ROJO FIJO como D-38, no la rendicion a ambar: el otro poste puede
    // seguir alternando por reloj.
    if (reloj_estadoCristal() == RELOJ_CRISTAL_CONGELADO) {
      irARojoSinHora();
      return;
    }
    iniciarSalida(true);
    return;
  }
  if (testigo && (estado == DEG_ENTRANDO || estado == DEG_ACTIVO)) {
    avisarRenovacion();   // 29/09: a los 28 dias y despues una vez al dia
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

  // D-52: a la hora de la salida programada, la salida de siempre (rojo, despeje, baja el permiso); con la hora no
  // fiable no dispara. Antes del switch: la hora perdida ya llevo ENTRANDO/ACTIVO a ROJO_SIN_HORA.
  if (salidaSinFecha && reloj_segundosDesde2000() != 0) {   // llego la fecha: lo que falta, sobre la hora absoluta
    const uint32_t falta = (salidaS % 86400UL + 86400UL - reloj_segundosDelDia()) % 86400UL;
    salidaS = reloj_segundosDesde2000() + (falta > SALIDA_MAX_S ? 0 : falta);   // pasada: sale ya
    salidaSinFecha = false;
  }
  if (salidaS != 0 && (estado == DEG_ENTRANDO || estado == DEG_ACTIVO || estado == DEG_ROJO_SIN_HORA) &&
      reloj_horaFiable() && reloj_segundosDesde2000() >= salidaS) {
    bluetooth_reportarEvento("DEGRADADO", "SALIDA_PROGRAMADA_EJECUTADA");
    iniciarSalida(false);
    return;
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
      //   3. D-35: con testigo, haber llegado a inicio; hasta entonces, rojo fijo.
      if ((ahora - tCambioEstado) >= rojoObligatorioMs() &&
          calcularFase() != FD_VERDE_ESCLAVO &&
          (!testigo || reloj_segundosDesde2000() >= testigoInicioS)) {
        estado = DEG_ACTIVO;
        tCambioEstado = ahora;
      }
      break;

    case DEG_ACTIVO:
      aplicarLuz(calcularFase() == FD_VERDE_ESCLAVO);
      break;

    case DEG_ROJO_SIN_HORA:   // D-38: rojo sostenido y el aviso cada minuto
      if (semaforo_estado() != S_ROJO) semaforo_forzarRojo();
      if (ahora - tAvisoRojo >= AVISO_ROJO_SIN_HORA_MS) {
        tAvisoRojo = ahora;
        bluetooth_reportarAlarma("DEGRADADO", "ROJO_SIN_HORA", "ROJO_FIJO");
      }
      break;

    case DEG_SALIENDO:
      if ((ahora - tCambioEstado) >= rojoObligatorioMs()) {
        if (rendicionEnCurso) {
          // Rendicion por el limite de 48 h (la hora va a rojo fijo desde D-38): ambar
          // intermitente, encendido aqui y no por el temporizador de 12 s de main.cpp.
          estado = DEG_RENDIDO;
          respaldo_guardarRendido(true);   // arquitecto 29/09: sin reentrada automatica hasta un PING
          semaforo_iniciarFallo();
          protocolo_resetReplayProtection();
        } else {
          estado = DEG_INACTIVO;
        }
        testigo = false;   // D-35: el todo-rojo de salida ya se conto con su despeje
        tCambioEstado = ahora;
      }
      break;

    default:
      break;
  }
}

bool degradado_gobiernaLuz() {
  return estado == DEG_ENTRANDO || estado == DEG_ACTIVO || estado == DEG_SALIENDO ||
         estado == DEG_ROJO_SIN_HORA;
}

EstadoDegradado degradado_estado() { return estado; }

// R-2. El porque completo esta en modo_degradado.h: DEG_SALIENDO no distingue la salida
// normal -termina en rojo- de la rendicion -termina en ambar-, y el despachador de
// Bluetooth necesita esa diferencia para contestar la verdad: "esta salida acaba en
// ambar". Desde D-38 solo la pide el limite de 48 h; la hora va a DEG_ROJO_SIN_HORA.
bool degradado_rendicionEnCurso() { return rendicionEnCurso; }

FaseDegradado degradado_fase() { return calcularFase(); }

// D-44/D-46: aqui vivian los rotulos de la pantalla (estado, fase) y la tabla de rechazo de
// SET_MODO:DEGRADADO; salieron sin lector con el menu y con la orden.
