// ===== src/modo_degradado.cpp (ESCLAVO) =====
#include "modo_degradado.h"
#include "ciclo_degradado.h"
#include "config_ciclo.h"
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
// intemperie derivan hasta ~8,6 s/dia en el peor caso. Con el todo-rojo ampliado
// a 30 s el margen teorico antes de que los verdes se solapen son ~3,5 dias; 48 h
// deja factor de seguridad 2. Alargar el plazo obliga a alargar el todo-rojo -una
// semana pide ~90 s, que destroza la fluidez del paso-. La alternativa real no es
// estirar el limite: es ir a arreglar el radio.
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

static EstadoDegradado estado = DEG_INACTIVO;
static unsigned long tCambioEstado = 0;
static bool rendicionEnCurso = false;

// Ultima orden de luz que ESTE modulo dio. Se actua solo en los flancos, nunca en
// cada vuelta del bucle, por dos razones: no reiniciar la transicion a verde a
// cada iteracion, y no pisar al backstop de verde maximo de main.cpp. Si el
// backstop cortara a rojo, forzar verde otra vez lo dejaria inservible.
static bool verdeAplicado = false;

static bool huboSyncAlguna = false;
static unsigned long tUltimaSync = 0;
static bool syncVencidaLatch = false;

static FaseDegradado faseCache = FD_DESPEJE_A;
static unsigned long tFaseCache = 0;

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
  // modo que ya se habia mandado apagar -por el operario, por el regreso del radio o
  // por el limite de 48 h-, y ninguna de esas tres ordenes admite marcha atras.
  respaldo_guardarDegradado(false);
}

// ---------------------------------------------------------------------------

void degradado_registrarSync() {
  huboSyncAlguna = true;
  tUltimaSync = millis();
  syncVencidaLatch = false;

  // Una sincronizacion nueva rehabilita el modo tras una rendicion: la causa que
  // lo tumbo -deriva desconocida- acaba de desaparecer. No se vuelve a entrar
  // solo, eso sigue siendo decision del operario.
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
  if (!reloj_enHora()) return DEG_RECHAZO_SIN_HORA;

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

  return DEG_ACEPTADO;
}

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
// N-20 — Reanudacion tras un corte. Ver el porque completo en modo_degradado.h.
//
// Solo la llama setup(), y una sola vez.
// ---------------------------------------------------------------------------
bool degradado_reanudarTrasCorte() {
  // Nada que reanudar: ni siquiera se toca el respaldo. Un equipo que nunca entro en
  // Degradado no debe escribir en la pila en cada arranque.
  if (!respaldo_degradadoActivo()) return false;

  bool sigueVigente = reloj_enHora() && respaldo_hayCiclo();

  uint32_t horas = RESPALDO_SYNC_CADUCADA;
  if (sigueVigente) {
    horas = respaldo_horasDesdeSync(reloj_contadorSegundos());
    // Las dos condiciones se comprueban por separado a proposito. CADUCADA no es un
    // numero grande, es "no se cuanto ha pasado": tratarla como una hora mas la
    // colaria por debajo del limite el dia que alguien cambie el orden de la resta.
    sigueVigente = (horas != RESPALDO_SYNC_CADUCADA) && (horas < LIMITE_SIN_SYNC_H);
  }

  if (!sigueVigente) {
    // Arranque normal Y BORRADO DEL INDICADOR. Sin el borrado, cada reinicio
    // reintentaria la misma comprobacion fallida, y un indicador que se queda puesto
    // acabaria disparandose el dia que un dato basura lo haga cuadrar por accidente.
    // La autorizacion caducada no se guarda "por si acaso": se tira.
    respaldo_guardarDegradado(false);
    return false;
  }

  // Se siembra el reloj del limite duro con la antiguedad REAL que el respaldo
  // conoce, no con el instante de arranque. Poner tUltimaSync = millis() regalaria
  // 48 h nuevas en cada corte de luz y convertiria el limite en un boton de posponer.
  //
  // La resta puede quedar por debajo de cero en aritmetica sin signo -millis() vale
  // pocos milisegundos aqui-, y es correcto que lo haga: todas las comparaciones del
  // modulo son de la forma (ahora - tUltimaSync), que con el desbordamiento sin signo
  // sigue dando la diferencia buena.
  huboSyncAlguna = true;
  tUltimaSync = millis() - horas * 3600000UL;
  syncVencidaLatch = false;

  // Y se entra por la MISMA puerta que usa el operario desde la pantalla. Asi el
  // todo-rojo de entrada, el reseteo del filtro de repeticion y la revalidacion de
  // condiciones son identicos: reanudar no es un camino alternativo con reglas
  // propias, es la entrada de siempre con el permiso recuperado de la pila.
  if (degradado_entrar() != DEG_ACEPTADO) {
    huboSyncAlguna = false;
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

  if (syncVencidaLatch && (estado == DEG_ENTRANDO || estado == DEG_ACTIVO)) {
    iniciarSalida(true);
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
          // Rendicion por el limite duro: ambar intermitente, el mismo estado al
          // que lleva la perdida de radio. Se enciende aqui explicitamente en vez
          // de esperar a que main.cpp lo deduzca de su temporizador de 12 s,
          // porque el motivo de esta caida es otro y no debe depender de que ese
          // temporizador este en el valor adecuado.
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

FaseDegradado degradado_fase() { return calcularFase(); }

uint32_t degradado_segundosParaCambio() {
  if (!reloj_enHora()) return 0;
  return ciclo_degradado_restante(reloj_segundosDelDia(),
                                  config_verdeSegundos(),
                                  config_despejeSegundos());
}

const char* degradado_textoEstado() {
  switch (estado) {
    case DEG_INACTIVO: return "INACTIVO";
    case DEG_ENTRANDO: return "ENTRANDO: TODO ROJO";
    case DEG_ACTIVO:   return "ACTIVO (por reloj)";
    case DEG_SALIENDO: return "SALIENDO: TODO ROJO";
    case DEG_RENDIDO:  return "RENDIDO 48h: AMBAR";
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
    default:                       return "";
  }
}
