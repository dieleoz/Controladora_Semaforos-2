// ===== src/coordinador.cpp =====
#include "coordinador.h"
#include "protocolo.h"
#include "reloj.h"
#include "respaldo.h"
#include "semaforo.h"
#include "bluetooth.h"   // N-73: la Caja Negra, que hasta hoy no la llamaba nadie
#include <string.h>
#include <stdio.h>

enum EstadoCoord {
  C_IDLE,
  C_MENU_IDLE,
  C_INICIAL_ESPERA_ESTATICO,
  C_INICIAL_MASTER_A_VERDE,
  C_MASTER_A_ROJO,
  C_ESPERA_ESTATICO_TRAS_MASTER,
  C_ESPERANDO_ACK_GREEN,
  C_ESPERANDO_ACK_RED,
  C_ESPERA_ESTATICO_TRAS_ESCLAVO,
  C_MASTER_A_VERDE,
  C_FALLO
};

enum QuienVerde { QV_NINGUNO, QV_MASTER, QV_ESCLAVO };

static EstadoCoord estadoC = C_IDLE;
static QuienVerde quienVerde = QV_NINGUNO;

// SFTY-4 (Safety Case): Tiempo de Despeje / All-Red
static unsigned long tiempoDespejeMs = 15000;

static bool handshakeOk = false;
static unsigned long tUltimoPing = 0;
static unsigned long tUltimaRxEsclavo = 0;
static bool demandaRemotaPendiente = false;

bool coordinador_hayDemandaRemota() {
  return demandaRemotaPendiente;
}

void coordinador_limpiarDemandaRemota() {
  demandaRemotaPendiente = false;
}

static unsigned long tEsperandoAck = 0;
static unsigned long tRef = 0;
static uint8_t retryCount = 0;

// --- SFTY-14: Telemetria de calidad de enlace (V8.1) -----------------------
// Se mide sobre el latido de 3 s que ya existe. Solo hay UN latido en vuelo a la
// vez, asi que la correspondencia peticion/respuesta es inequivoca: no hace falta
// llevar identificadores ni descartar muestras ambiguas.
static unsigned long tLatidoEnviado = 0;
static bool latidoEnVuelo = false;
static uint8_t respuestaEsperada = 0;  // que comando cierra el latido en vuelo
static uint16_t ventanaLatidos = 0;   // 1 bit por latido: 1 = respondido
static uint8_t muestrasLatido = 0;    // cuantos bits validos hay (tope 10)
static unsigned long rttMedioMs = 0;  // media exponencial del tiempo de respuesta
static int latidosSinRespuesta = 0;

static const uint8_t VENTANA_LATIDOS = 10;

// Registra el resultado de un latido en la ventana deslizante.
static void registrarLatido(bool respondido) {
  ventanaLatidos = (uint16_t)((ventanaLatidos << 1) | (respondido ? 1 : 0));
  if (muestrasLatido < VENTANA_LATIDOS) muestrasLatido++;
  if (respondido) {
    latidosSinRespuesta = 0;
  } else if (latidosSinRespuesta < 999) {
    latidosSinRespuesta++;
  }
}
// OPT-7: Timeout de espera de ACK antes de reintentar la orden.
//
// DECISION DE INGENIERIA (31/07/2026): 3500 ms.
//
// VALIDADO EN CAMPO ese mismo dia con la tasa aerea ya corregida a 2.4 kbps: Automatico,
// Inteligente y Manual, ciclos repetidos, sin una sola caida de comunicacion.
//
// Historia: con la tasa aerea anterior de 0.3 kbps el viaje ida-vuelta GO_GREEN -> ACK_GREEN
// rondaba los 3.0-3.2s y desbordaba estos mismos 3500 ms. El reintento colisionaba entonces con
// el ACK entrante en el bus half-duplex y se caia a C_FALLO: era la "caida de comunicacion al paso
// de ciclos" reportada en campo. La causa raiz era la TASA AEREA, no este valor.
//
// Por que NO se sube "por si acaso", siendo los semaforos MOVILES y la distancia variable:
//   - La distancia NO aumenta la latencia: 600 m son ~2 us de propagacion. El tiempo de viaje lo
//     fija la tasa aerea, no los metros.
//   - Lo que SI crece con la distancia es la PROBABILIDAD DE PERDIDA de la trama.
//   - Contra una trama perdida, esperar mas no sirve: no va a llegar. Solo sirve REPETIR.
//   - Por tanto las palancas correctas para distancia son RF_BURST_COPIES (=3) y el numero de
//     reintentos, no este timeout.
//   - El fallback de orfandad (SFTY6_SILENCIO_MS) acota la ventana TOTAL, porque quien primero
//     llegue manda: menos timeout = mas oportunidades de recuperar antes de ir a estado seguro.
//     Justo lo que conviene en despliegues moviles no caracterizados.
//   - N-71: esa cuenta estuvo MAL tres semanas. Decia "con 3500 ms caben 4 intentos" contra un
//     techo de 12 s, cuando el codigo hace 5 y el peor caso son 3 + 5 x 3,56 = 20,8 s. No cabian
//     ni 3: el ambar por orfandad saltaba antes. Ahora el techo son 25 s y la desigualdad la
//     recalcula costura_09_presupuesto_radio desde estas mismas constantes, en vez de fiarse de
//     este comentario.
const unsigned long TIMEOUT_ACK_MS = 3500;

// Cadencia del latido. N-71: estaba escrita como un 3000 desnudo dentro del if que lo
// dispara -la misma forma exacta del defecto que costo N-69 con el umbral de silencio-.
// Importa que tenga nombre porque NO es un numero suelto: es el primer sumando del
// presupuesto de radio, lo mas tarde que puede arrancar un intercambio desde la ultima
// recepcion, y costura_09_presupuesto_radio lo lee de aqui para recalcular la
// desigualdad contra SFTY6_SILENCIO_MS. Un literal no se puede leer sin adivinar.
const unsigned long LATIDO_MS = 3000;

// Reintentos del ciclo antes de darse por caido. N-71: hasta hoy solo existian sobre el
// papel -el ambar por orfandad saltaba a los 12 s, sobre el 2o o 3er reintento- y el
// numero estaba repetido como literal en los dos estados de espera de ACK.
const uint8_t CICLO_MAX_REINTENTOS = 5;

// --- SFTY-23: Sincronizacion horaria por radio (lado Maestro) --------------
//
// Segunda maquina de estados, INDEPENDIENTE de la del ciclo. Se hizo aparte a
// proposito: estas tramas no encienden ni apagan una sola luz, y meterlas en el
// switch de estadoC habria obligado a que cada estado nuevo contemplase que hacer
// con el semaforo. Aqui no hay nada que contemplar.
//
// Comparte el canal half-duplex con el ciclo y con el latido, asi que solo actua
// cuando el bus esta libre; nunca al reves.
enum EstadoSync {
  SY_IDLE,
  SY_ESPERA_ACK_HORA,
  SY_ESPERA_RESP_DELTA,
  SY_ESPERA_ACK_CONFIG
};

static EstadoSync estadoSync = SY_IDLE;
static unsigned long tSyncEnviado = 0;
static uint8_t syncRetry = 0;

// Peticiones pendientes. Se separan del estado porque una peticion sobrevive a que
// el bus este ocupado: se guarda y se atiende cuando queda libre, en vez de
// perderse por haber llegado en mal momento.
static bool pendHora = false;
static bool pendDelta = false;
static bool pendConfig = false;
static uint8_t cfgVerdeSeg = 0;
static uint8_t cfgDespejeSeg = 0;

static bool syncAlgunaVez = false;
static unsigned long tUltimaSyncOk = 0;

// Prueba de que el Esclavo tiene el ciclo del Modo Degradado. Se pone SOLO al
// llegar CMD_ACK_CONFIG y se baja al reencolar la configuracion: mientras el acuse
// no vuelva, el Maestro no puede dar por hecho que el otro extremo sabe con que
// duraciones calcular su fase.
static bool configConfirmada = false;

static int8_t desfaseEsclavo = DELTA_FUERA_DE_RANGO;
static bool hayDesfase = false;
static unsigned long tUltimoDesfase = 0;

// Vigilancia del reloj propio, para detectar que el operario acaba de ponerlo en
// hora. Ver vigilarCambioDeHora().
static uint32_t segDiaVisto = 0;
static unsigned long tSegDiaVisto = 0;
static bool relojVistoEnHora = false;

// Tras agotar los intentos se espera antes de volver a probar, en vez de reintentar
// sin freno: cada intercambio fallido ocupa el canal 7 s y lo unico que consigue
// insistiendo es robarselo al ciclo y al latido.
static bool syncEnBackoff = false;
static unsigned long tFalloSync = 0;

// 3 intentos, y el numero sale de una cuenta, no de un gusto. Durante el intercambio
// se suprime el latido (ver SFTY-13 mas abajo), asi que el Esclavo puede pasar ese
// rato sin emitir nada. El intercambio arranca como muy tarde 3 s despues de la
// ultima respuesta recibida -esa es la cadencia del latido-, luego el peor caso es
// 3 + 3 x 3,56 = 13,7 s sin recibir, por debajo del fallback de orfandad.
//
// N-71: ERAN 2, Y EL UNICO MOTIVO ERA EL TECHO VIEJO. Con 12 s, tres intentos daban
// 13,7 s y una sincronizacion fallida se convertia en una FALSA caida de enlace, o
// sea un ambar sin motivo: perder una sincronizacion cuesta esperar al siguiente
// intento, provocar un ambar espurio cuesta una salida a campo. Al subir el techo a
// 25 s ese motivo desaparece, y dejarlo en 2 con el comentario viejo debajo habria
// sido una restriccion sin causa con una explicacion falsa encima.
//
// LA CUENTA, REHECHA CON LA CUARTA TRAMA (CMD_HORA_D, 01/08/2026). El presupuesto
// aguanta, y conviene ver por que aguanta antes de meter una quinta:
//
//   Lo que la trama extra cuesta en RELOJ DE PARED es solo su paso por el cable al
//   modulo: 4 bytes x 3 copias de rafaga = 12 bytes a 9600 baudios = 12,5 ms, mas el
//   delay(2) de conmutacion del MAX485 y la guarda de 1,2 ms => ~16 ms. El envio
//   entero pasa de ~47 a ~63 ms.
//
//   Peor caso = 3 s (cadencia del latido, lo mas tarde que puede arrancar el
//   intercambio desde la ultima respuesta recibida) + 3 intentos x (~0,06 s de envio
//   + 3,5 s de espera) = 13,7 s, contra el fallback de orfandad de 25 s (N-71).
//   Margen: 11,3 s. Con el techo anterior de 12 s el margen eran 1,87 s con SOLO DOS
//   intentos, y de ahi salia el ambar espurio en cuanto la lluvia tumbaba una trama.
//
//   El TIEMPO DE AIRE no entra en esa suma porque transcurre mientras el Maestro ya
//   esta esperando: lo que hace es comerse la ventana de 3,5 s, no alargarla. Ahi la
//   cuenta es 4 tramas x ~0,13 s de aire (ver SFTY-11 en protocolo.h) = 0,52 s de
//   ida, + 0,2 s de cortesia del Esclavo (SFTY-17) + 0,13 s del ACK = ~0,85 s de
//   ida y vuelta. Antes ~0,72 s. Caben de sobra en 3,5 s.
//
//   Con el techo en 25 s, los tres intentos (13,7 s) caben con 11,3 s de margen.
static const uint8_t SYNC_MAX_INTENTOS = 3;
static const unsigned long BACKOFF_SYNC_MS = 60000UL;

// Una vez por hora sobra: la deriva entre dos sincronizaciones tan seguidas queda en
// milisegundos. Lo que se paga por sincronizar mas a menudo es canal, que es el
// recurso escaso, no precision.
static const unsigned long INTERVALO_SYNC_MS = 3600000UL;

// Vigencia de la medida de desfase: el doble del periodo de sincronizacion. Si en dos
// ciclos enteros no se pudo medir, el numero de pantalla ya no describe al equipo.
static const unsigned long VIGENCIA_DESFASE_MS = 7200000UL;

// Envia el cuarteto dia/hora/minuto/segundo. El Esclavo acumula D, H y M en un buffer
// y aplica las CUATRO AL RECIBIR LA DE SEGUNDOS, asi que nunca queda con una fecha y
// hora a medias.
//
// El dia va en la secuencia -y no se queda en cada equipo- porque las dos puntas
// tienen que contar los dias con el MISMO numero. Cuando cada una sembraba el suyo al
// ponerse en hora, un corte de energia el dia en que el calendario de UNA cruzaba de
// 31 a 1 hacia que respaldo_horasDesdeSync() la declarase CADUCADA solo a ella: esa no
// reanudaba y se quedaba en ambar mientras la otra reanudaba en fase y seguia dando
// verde. Ambar contra verde es el fallo asimetrico que hay que evitar; con los
// calendarios acoplados las dos fallan a la vez y el resultado es seguro.
//
// AQUI ESTA LA REGLA CRITICA DE SFTY-23: el reloj se lee DENTRO de esta funcion, que
// es la misma que se invoca en el primer envio y en CADA retransmision. Guardar el
// cuarteto calculado la primera vez y reenviarlo seria un fallo de una sola linea,
// invisible con enlace bueno: una trama perdida y reintentada 3,5 s despues dejaria
// al Esclavo 3,5 s atrasado, y el error entraria justo por el mecanismo que existe
// para dar robustez. Por eso no hay ninguna variable donde guardar la hora a enviar:
// si no existe el sitio, nadie puede reutilizar el valor viejo por descuido.
static void enviarHoraCompleta() {
  // El DIA SE LEE EL PRIMERO, antes que el segundo, y no por gusto: el segundo es el
  // unico testigo de que se cruzo una frontera (ver abajo). Solo delata las lecturas
  // hechas ANTES que el, asi que todo lo que deba quedar cubierto por esa guarda
  // tiene que leerse por delante. Un dia leido DESPUES del segundo podria traer ya la
  // fecha nueva con un segundo 59 de la vieja, y nada lo detectaria.
  uint8_t d = reloj_dia();
  uint8_t h = reloj_hora();
  uint8_t m = reloj_minuto();
  uint8_t s = reloj_segundo();

  // Las cuatro lecturas no son atomicas y el reloj puede cambiar entre medias. Si eso
  // pasa justo despues de leer el minuto, se enviaria HH:59:00 en vez de HH+1:00:00:
  // 60 s de error por unos microsegundos de desfase entre dos lecturas. El segundo
  // recien puesto a cero es la unica pista de que se cruzo esa frontera, asi que en
  // ese caso se releen las cuatro: acabado de cruzar, no puede volver a cruzarse.
  //
  // La misma guarda vale para el dia SIN tocarla, y esa es la razon de que el dia
  // entre por aqui y no por otro sitio: el dia cambia a medianoche, es decir en el
  // instante exacto en que el segundo vale 0. No hay ningun cruce de fecha que esta
  // condicion no vea; enviar la fecha de ayer con la hora de hoy costaria 24 h de
  // error en respaldo_horasDesdeSync(), que es justo lo que este cambio viene a
  // arreglar.
  if (s == 0) {
    d = reloj_dia();
    h = reloj_hora();
    m = reloj_minuto();
    s = reloj_segundo();
  }

  // Las cuatro seguidas y sin espera entre ellas. Son ~63 ms de bucle -cuatro rafagas
  // de 4 bytes a 9600 baudios por el cable al modulo, ~16 ms cada una-, despreciable
  // frente al watchdog de 4 s, y a cambio el cuarteto viaja junto: repartirlo entre
  // iteraciones abriria una ventana para que el ciclo colase una orden en medio y el
  // Esclavo aplicase una hora descabalada.
  protocolo_enviarPaquete(CMD_HORA_D, d);
  protocolo_enviarPaquete(CMD_HORA_H, h);
  protocolo_enviarPaquete(CMD_HORA_M, m);
  protocolo_enviarPaquete(CMD_HORA_S, s);
}

// Mismo motivo que el trio: el segundo se lee AQUI, en el instante del envio. Medir
// el desfase con un segundo caducado inventaria una diferencia que no existe, y la
// medida se registra en el acta de pruebas como si fuera cierta.
static void enviarPeticionDelta() {
  protocolo_enviarPaquete(CMD_DELTA, reloj_segundo());
}

// El Esclavo confirma con un unico CMD_ACK_CONFIG tras recibir las dos, igual que
// hace con la hora: se acusa el conjunto aplicado, no cada trama suelta.
static void enviarConfigCiclo() {
  protocolo_enviarPaquete(CMD_CONFIG_VERDE, cfgVerdeSeg);
  protocolo_enviarPaquete(CMD_CONFIG_DESPEJE, cfgDespejeSeg);
}

static void syncAbandonarIntento() {
  // La medida de desfase se suelta al fallar; la hora y la configuracion NO. La
  // medida es diagnostico y se vuelve a encolar sola tras la siguiente hora aplicada,
  // asi que insistir con ella solo le roba canal al latido. Las otras dos son
  // condicion de seguridad del Modo Degradado: siguen pendientes y se reintentan,
  // espaciadas por el backoff.
  if (estadoSync == SY_ESPERA_RESP_DELTA) pendDelta = false;
  estadoSync = SY_IDLE;
  syncEnBackoff = true;
  tFalloSync = millis();
}

// Detecta que el reloj del Maestro ha cambiado por mano del operario.
//
// SFTY-23 dice que poner la hora ES sincronizar, un solo gesto. Pero la pantalla de
// AJUSTAR HORA solo llama a reloj_ajustar(), y el reloj no avisa a nadie de que le
// han escrito. Sin esto, el operario cuadra la hora y el Esclavo sigue con la vieja
// hasta la resincronizacion horaria: hasta 59 minutos creyendo que el gesto surtio
// efecto. Se detecta comparando el reloj contra lo que deberia marcar segun millis().
// Periodo de la vigilancia. Empezo en 1 s, que era desperdicio: el RTC va por su
// cuenta con la pila y no cambia solo, asi que preguntarle mil veces lo que ya se
// sabe solo roba bucle al ciclo y al latido.
//
// Ademas esto dejo de ser el camino principal: modo_hora.cpp llama a
// coordinador_sincronizarHora() en cuanto el operario confirma, que es exacto y no
// heuristico. Lo de aqui es SOLO una red de respaldo para un ajuste que llegase por
// otra via, y para eso 10 minutos de latencia no le cuestan nada a nadie.
//
// La tolerancia de +-3 s aguanta el nuevo periodo sin tocarla: en 600 s, dos bases
// de tiempo del mismo microcontrolador se separan centesimas, no segundos.
static const unsigned long VIGILANCIA_RELOJ_MS = 600000UL;  // 10 min

static void vigilarCambioDeHora() {
  if (millis() - tSegDiaVisto < VIGILANCIA_RELOJ_MS) return;

  // Ancla el calendario propio a enero cada 10 min. El Esclavo ya queda anclado en
  // cada sincronizacion, pero el mes del Maestro avanzaria solo, y el RTC vuelca de
  // 31 a 1 SEGUN LA LONGITUD DEL MES: en febrero el Maestro volcaria en 28 mientras
  // el Esclavo, en enero, sigue en 29. Se separarian hasta la siguiente sync, y un
  // corte en esa ventana reproduce la asimetria ambar-contra-verde.
  //
  // Con las dos puntas en enero, ambas vuelcan en 31 y a la vez. Que el mes sea falso
  // da igual: nadie lo muestra, solo se restan dias, y lo que importa es restar IGUAL.
  reloj_fijarEnero();

  bool enHora = reloj_enHora();

  if (enHora && !relojVistoEnHora) {
    // Paso de no fiable a fiable: es la primera puesta en hora, o el arranque de un
    // equipo con la pila puesta. En ambos casos el Esclavo no tiene esa hora.
    pendHora = true;
  } else if (enHora) {
    uint32_t esperado = (segDiaVisto + (millis() - tSegDiaVisto) / 1000UL) % 86400UL;
    uint32_t actual = reloj_segundosDelDia();
    uint32_t dif = (actual + 86400UL - esperado) % 86400UL;
    // Tolerancia de +-3 s. Absorbe el truncamiento de la division y lo que el bucle
    // se retrase pintando la pantalla, sin dejar pasar un ajuste de verdad: nadie
    // corrige un reloj tres segundos a mano.
    if (dif > 3UL && dif < (86400UL - 3UL)) {
      pendHora = true;
    }
  }

  relojVistoEnHora = enHora;
  segDiaVisto = enHora ? reloj_segundosDelDia() : 0;
  tSegDiaVisto = millis();
}

// Maquina de la sincronizacion. Se ejecuta al FINAL de coordinador_actualizar(), ya
// resuelto el ciclo de esta iteracion, para que ninguna trama de hora se cuele por
// delante de una orden de luz.
static void atenderSincronizacion(bool llego, const RF_Packet* pkt, bool tieneComunicacion) {
  // El ciclo manda sobre el bus. Si la maquina de luces esta esperando su ACK, o el
  // equipo esta en fallo, se abandona el intercambio en curso; la peticion queda
  // pendiente y se atendera cuando el bus quede libre. Sincronizar la hora no puede
  // retrasar un cambio de luz ni un instante: la hora se puede reintentar, un verde
  // simultaneo no se puede deshacer.
  if (estadoC == C_ESPERANDO_ACK_GREEN || estadoC == C_ESPERANDO_ACK_RED ||
      estadoC == C_FALLO) {
    estadoSync = SY_IDLE;
    return;
  }

  // Resincronizacion periodica automatica mientras haya enlace.
  if (syncAlgunaVez && (millis() - tUltimaSyncOk >= INTERVALO_SYNC_MS)) {
    pendHora = true;
  }

  if (estadoSync != SY_IDLE) {
    // Respuesta del Esclavo. Se exige el comando que corresponde al intercambio en
    // curso: dar por buena cualquier trama entrante cerraria una sincronizacion con
    // el PONG del latido y se registraria como aplicada una hora que se perdio.
    if (llego) {
      if (estadoSync == SY_ESPERA_ACK_HORA && pkt->command == CMD_ACK_HORA) {
        tUltimaSyncOk = millis();
        syncAlgunaVez = true;
        pendHora = false;
        estadoSync = SY_IDLE;

        // N-20: la marca se graba AQUI, en el unico punto del firmware donde consta
        // que el Esclavo aplico la hora. Ni al encolar ni al enviar: las dos cosas
        // ocurren tambien cuando la trama se pierde, y anotar entonces una
        // sincronizacion que no llego a producirse dejaria en la pila el aval con el
        // que despues se reanudaria el Modo Degradado.
        //
        // Se guarda el INSTANTE del reloj de pared -dia y segundos del dia-, no un
        // contador de millis(): millis() vuelve a cero en el reinicio, que es
        // precisamente el caso que esto viene a cubrir. Si el reloj no esta en hora,
        // reloj_dia() devuelve 0 y respaldo_marcarSync() descarta la marca por su
        // cuenta; no hay que filtrarlo aqui.
        respaldo_marcarSync(reloj_contadorSegundos());


        // Sincronizar y no medir es suponer. El gate del Modo Degradado pide un
        // numero registrable, no la confianza en que el ACK basta.
        pendDelta = true;
        return;
      }
      if (estadoSync == SY_ESPERA_RESP_DELTA && pkt->command == CMD_DELTA_RESP) {
        // El byte viaja en complemento a dos; interpretarlo como int8_t es lo que le
        // devuelve el signo. Leerlo como uint8_t convertiria un atraso de 1 s en 255.
        desfaseEsclavo = (int8_t)pkt->param;
        // Se guarda tambien el fuera de rango, con su instante: "no se puede medir"
        // es un resultado, y la pantalla debe poder distinguirlo de "no se midio".
        hayDesfase = (desfaseEsclavo != DELTA_FUERA_DE_RANGO);
        tUltimoDesfase = millis();
        pendDelta = false;
        estadoSync = SY_IDLE;
        return;
      }
      if (estadoSync == SY_ESPERA_ACK_CONFIG && pkt->command == CMD_ACK_CONFIG) {
        pendConfig = false;
        // Este acuse es la UNICA prueba de que el Esclavo tiene el ciclo. La puerta
        // del Modo Degradado lo exige: sin el, el Maestro podia aceptar mientras el
        // Esclavo rechazaba por falta de configuracion, dejando verde contra ambar.
        configConfirmada = true;
        estadoSync = SY_IDLE;
        return;
      }
    }

    // Mismo timeout y misma cuenta de reintentos que el ciclo (SFTY-7). No se
    // inventa un valor propio: el viaje ida-vuelta lo fija la tasa aerea, que es la
    // misma para estas tramas que para un GO_GREEN.
    if (millis() - tSyncEnviado > TIMEOUT_ACK_MS) {
      syncRetry++;
      if (syncRetry >= SYNC_MAX_INTENTOS) {
        syncAbandonarIntento();
        return;
      }
      // La retransmision reenvia el intercambio COMPLETO, no solo la ultima trama.
      // Una trama perdida no dice cual se perdio: si fue la de hora, el Esclavo tiene
      // en su buffer una hora vieja que aplicaria con el segundo nuevo. Y al pasar
      // por enviarHoraCompleta() el cuarteto se vuelve a leer del reloj, que es la
      // regla critica de SFTY-23.
      if (estadoSync == SY_ESPERA_ACK_HORA) {
        enviarHoraCompleta();
      } else if (estadoSync == SY_ESPERA_RESP_DELTA) {
        enviarPeticionDelta();
      } else {
        enviarConfigCiclo();
      }
      tSyncEnviado = millis();
    }
    return;  // con un intercambio en vuelo no se arranca otro
  }

  // --- Arranque de un intercambio nuevo ------------------------------------
  if (!pendHora && !pendDelta && !pendConfig) return;

  if (syncEnBackoff) {
    if (millis() - tFalloSync < BACKOFF_SYNC_MS) return;
    syncEnBackoff = false;
  }

  // Sin enlace no se intenta: solo serviria para gastar los tres intentos y entrar en
  // espera. La peticion sigue pendiente y saldra cuando el Esclavo vuelva.
  if (!tieneComunicacion) return;

  // SFTY-13: con el latido en vuelo el bus esta comprometido. Transmitir ahora
  // colisionaria con el PONG entrante en el canal half-duplex, que es exactamente el
  // fallo que costo la "caida de comunicacion al paso de ciclos". Se espera.
  if (latidoEnVuelo) return;

  if (pendHora) {
    // La hora primero: es de lo que dependen el desfase y el propio Modo Degradado.
    if (!reloj_enHora()) {
      pendHora = false;  // sin hora fiable no hay nada que empujar
    } else {
      enviarHoraCompleta();
      estadoSync = SY_ESPERA_ACK_HORA;
      tSyncEnviado = millis();
      syncRetry = 0;
      return;
    }
  }

  if (pendConfig) {
    // La configuracion del ciclo va antes que la medida: es condicion de seguridad,
    // la medida es diagnostico.
    enviarConfigCiclo();
    estadoSync = SY_ESPERA_ACK_CONFIG;
    tSyncEnviado = millis();
    syncRetry = 0;
    return;
  }

  if (pendDelta) {
    if (!reloj_enHora()) {
      pendDelta = false;
    } else {
      enviarPeticionDelta();
      estadoSync = SY_ESPERA_RESP_DELTA;
      tSyncEnviado = millis();
      syncRetry = 0;
    }
  }
}

void coordinador_setup() {
  protocolo_setup();
  semaforo_setup(); 
  estadoC = C_IDLE;
  quienVerde = QV_NINGUNO;
  handshakeOk = false;
  tUltimaRxEsclavo = 0; // Inicializar en 0: no hemos recibido nada del Esclavo aún
}

void coordinador_reiniciarConexion() {
  handshakeOk = false;
  estadoC = C_IDLE;
  quienVerde = QV_NINGUNO;
  tUltimoPing = 0;
  tUltimaRxEsclavo = 0;

  // SFTY-23: se aborta el intercambio en vuelo -su ACK ya no va a llegar- y se
  // invalida la medida de desfase. Al otro lado puede haber ahora una unidad recien
  // arrancada: seguir mostrando el desfase del enlace anterior seria describir a un
  // equipo con el dato de otro.
  estadoSync = SY_IDLE;
  hayDesfase = false;
  pendHora = true;    // la hora vuelve a empujarse en cuanto el enlace responda
  pendConfig = true;  // y el ciclo con ella: al otro lado puede haber otra unidad
  configConfirmada = false;  // lo confirmado lo confirmo OTRA unidad, quiza
}

bool coordinador_intentarHandshake() {
  coordinador_actualizar();
  return handshakeOk;
}

void coordinador_configurar(unsigned long tiempoDespeje, unsigned long, unsigned long) {
  tiempoDespejeMs = tiempoDespeje;
}

void coordinador_forzarMenu() {
  estadoC = C_MENU_IDLE;
  quienVerde = QV_NINGUNO;
  semaforo_forzarRojo();
  protocolo_resetReplayProtection();
  protocolo_enviarPaquete(CMD_GO_RED);
}

void coordinador_forzarRojoTotal() {
  quienVerde = QV_NINGUNO;
  semaforo_forzarRojo();
  protocolo_resetReplayProtection();
  protocolo_enviarPaquete(CMD_GO_RED);
  tRef = millis();
  tUltimaRxEsclavo = millis(); // Rojo total es intencional, resetear timer para no disparar fallo
  estadoC = C_IDLE; // Queda en Rojo Fijo en ambos semáforos indefinidamente
}

void coordinador_iniciarModo() {
  quienVerde = QV_NINGUNO;
  semaforo_forzarRojo();
  protocolo_resetReplayProtection();
  protocolo_enviarPaquete(CMD_GO_RED);
  tRef = millis();
  tUltimaRxEsclavo = millis();
  estadoC = C_INICIAL_ESPERA_ESTATICO;
}

void coordinador_pedirCambio() {
  if (estadoC != C_IDLE) return;

  switch (quienVerde) {
    case QV_NINGUNO:
      tRef = millis();
      estadoC = C_INICIAL_ESPERA_ESTATICO;
      break;

    case QV_MASTER:
      semaforo_forzarRojo(); // Directo a rojo
      estadoC = C_MASTER_A_ROJO;
      break;

    case QV_ESCLAVO:
      protocolo_enviarPaquete(CMD_GO_RED);
      tEsperandoAck = millis();
      retryCount = 0;
      estadoC = C_ESPERANDO_ACK_RED;
      break;
  }
}

void coordinador_actualizar() {
  semaforo_actualizar();

  RF_Packet pkt;
  bool llego = protocolo_hayPaqueteDisponible(&pkt);

  if (llego) {
    tUltimaRxEsclavo = millis();
    handshakeOk = true;

    // Telemetria: solo cierra el latido la respuesta que le corresponde (PONG a un
    // PING, ACK_RED a un GO_RED). Aceptar cualquier paquete falsearia la medida: si
    // un latido se pierde queda en vuelo 3 s, y un ACK de cambio de ciclo llegado en
    // esa ventana daria un RTT inflado y contaria como exitoso un latido perdido.
    if (latidoEnVuelo && pkt.command == respuestaEsperada) {
      unsigned long rtt = millis() - tLatidoEnviado;
      rttMedioMs = (rttMedioMs == 0) ? rtt : ((rttMedioMs * 3 + rtt) / 4);
      latidoEnVuelo = false;
      registrarLatido(true);
    }

    if (pkt.command == CMD_PING) {
      protocolo_enviarPaquete(CMD_PONG);
    } else if (pkt.command == CMD_DEMANDA) {
      demandaRemotaPendiente = true;
      protocolo_enviarPaquete(CMD_ACK_DEMANDA);
    }
  }

  // OPT-6 & SFTY-12: Heartbeat PING cada 3.0s para mantener vivo el canal RF
  // SFTY-13: SUPRIMIR PING durante espera de ACK para evitar colisión RS485
  // FIX H-1: En C_FALLO se envía CMD_GO_RED (NUNCA PING) para que el Esclavo no quede atrapado en Verde
  //
  // SFTY-23: la supresion se extiende al intercambio de sincronizacion, que espera su
  // propio ACK y sufre la misma colision half-duplex. El enlace no queda a ciegas
  // mientras tanto: la respuesta del Esclavo tambien refresca tUltimaRxEsclavo, y el
  // intercambio esta acotado a SYNC_MAX_INTENTOS, por debajo del fallback de orfandad.
  if (millis() - tUltimoPing > LATIDO_MS
      && estadoC != C_ESPERANDO_ACK_GREEN
      && estadoC != C_ESPERANDO_ACK_RED
      && estadoSync == SY_IDLE) {
    // Telemetria: si el latido anterior seguia en vuelo, nunca fue respondido.
    if (latidoEnVuelo) {
      registrarLatido(false);
    }

    if (estadoC == C_MENU_IDLE || estadoC == C_FALLO) {
      protocolo_enviarPaquete(CMD_GO_RED); // Exige Rojo Fijo en Esclavo durante Menú o Fallo
      respuestaEsperada = CMD_ACK_RED;     // el Esclavo confirma el Rojo
    } else {
      protocolo_enviarPaquete(CMD_PING);
      respuestaEsperada = CMD_PONG;
    }
    tUltimoPing = millis();

    tLatidoEnviado = tUltimoPing;
    latidoEnVuelo = true;
  }

  // SFTY-6 / SFTY-9: Monitoreo de caida (SFTY6_SILENCIO_MS sin recibir) y Auto-Recuperacion
  bool tieneComunicacion = (tUltimaRxEsclavo > 0) && (millis() - tUltimaRxEsclavo <= SFTY6_SILENCIO_MS);

  if (estadoC == C_MENU_IDLE) {
    if (tieneComunicacion) {
      semaforo_forzarRojo(); // TEST 5: Con comunicación en Menú -> Maestro y Esclavo en ROJO FIJO
    } else {
      // TEST 3 y 4: Sin comunicación en Menú -> Maestro y Esclavo en AMARILLO PARPADEO.
      // FIX: la llamada DEBE estar guardada. semaforo_iniciarFallo() hace tCambio = millis()
      // y apaga las salidas; al invocarla en cada iteración del loop reiniciaba sin parar el
      // temporizador de 500ms y semaforo_actualizar() nunca llegaba a conmutar el ámbar,
      // dejando al Maestro sin parpadear (fallo reportado en campo por el funcional).
      if (semaforo_estado() != S_FALLO) {
        semaforo_iniciarFallo();
      }
    }
  } else {
    // En modos de operación activos (Automático, Inteligente, Manual)
    if (!tieneComunicacion) {
      if (tUltimaRxEsclavo > 0 || millis() > SFTY6_SILENCIO_MS) {
        if (estadoC != C_FALLO) {
          // N-73: ver la nota larga en Esclavo/src/main.cpp. La Caja Negra estaba
          // declarada y sin llamar en las dos puntas. Esta es una de las DOS puertas
          // por las que el Maestro cae a ambar, y se distinguen en la causa: por aqui
          // se entra por silencio, y por la de abajo por reintentos agotados. Saber
          // cual de las dos fue es justo lo que el reporte de lluvia necesitaba.
          char causa[40];
          snprintf(causa, sizeof(causa), "SILENCIO_%lums", SFTY6_SILENCIO_MS);
          bluetooth_reportarAlarma("FALLO_RF", causa, "CAMBIO_A_AMBAR");
          estadoC = C_FALLO; // TEST 3: Esclavo apagado / sin comunicación -> Maestro a AMARILLO PARPADEO
        }
      }
    } else if (estadoC == C_FALLO && tieneComunicacion) {
      // SFTY-9: Self-Healing Auto-Recuperación Automática tras restablecer enlace RF
      //
      // SFTY-23: el Esclavo puede haber vuelto por un reinicio, y su reloj no
      // sobrevive a un arranque sin sincronizar. Se encola la hora en cuanto se
      // recupera el enlace, sin esperar al ciclo horario: mientras no la tenga, el
      // Modo Degradado no puede autorizarse.
      pendHora = true;

      // Y EL CICLO TAMBIEN. Detectado por el validador de costura el 01/08/2026:
      // publicarConfig() se llamaba UNA sola vez, en setup(), y al recuperarse el
      // enlace solo se reencolaba la hora.
      //
      // El escenario era este: se sustituye el Esclavo, o se le cambia la pila. El
      // Maestro le reenvia la hora en la siguiente resincronizacion pero NUNCA el
      // ciclo. El operario entra en Degradado y las dos puntas hacen cosas
      // distintas: el Maestro acepta y da VERDE por reloj; el Esclavo rechaza por
      // falta de configuracion, se queda en modo normal y cae a AMBAR por orfandad
      // por orfandad, porque el Maestro ya callo.
      //
      // Verde contra ambar, indefinidamente, y solo se arreglaba reiniciando el
      // Maestro. Un reinicio del Esclavo lo dejaba sin lo que necesita justo cuando
      // mas falta hace.
      pendConfig = true;
      configConfirmada = false;  // hasta que el nuevo acuse llegue, no consta
      quienVerde = QV_NINGUNO;
      semaforo_forzarRojo();
      protocolo_resetReplayProtection();
      protocolo_enviarPaquete(CMD_GO_RED);
      tRef = millis();
      estadoC = C_INICIAL_ESPERA_ESTATICO;
    }
  }

  switch (estadoC) {

    case C_IDLE:
      break;

    case C_MENU_IDLE:
      break;

    case C_INICIAL_ESPERA_ESTATICO:
      if (millis() - tRef >= tiempoDespejeMs) {
        semaforo_iniciarTransicionAVerde(); // Transición Rojo -> Amarillo (4s) -> Verde
        estadoC = C_INICIAL_MASTER_A_VERDE;
      }
      break;

    case C_INICIAL_MASTER_A_VERDE:
      if (semaforo_estable() && semaforo_estado() == S_VERDE) {
        quienVerde = QV_MASTER;
        estadoC = C_IDLE;
      }
      break;

    case C_MASTER_A_ROJO:
      if (semaforo_estable() && semaforo_estado() == S_ROJO) {
        tRef = millis();
        estadoC = C_ESPERA_ESTATICO_TRAS_MASTER;
      }
      break;

    case C_ESPERA_ESTATICO_TRAS_MASTER:
      if (millis() - tRef >= tiempoDespejeMs) {
        protocolo_enviarPaquete(CMD_GO_GREEN);
        tEsperandoAck = millis();
        retryCount = 0;
        estadoC = C_ESPERANDO_ACK_GREEN;
      }
      break;

    case C_ESPERANDO_ACK_GREEN:
      if (llego && pkt.command == CMD_ACK_GREEN) {
        quienVerde = QV_ESCLAVO;
        estadoC = C_IDLE;
      } else if (millis() - tEsperandoAck > TIMEOUT_ACK_MS) {
        retryCount++;
        if (retryCount >= CICLO_MAX_REINTENTOS) {
            // N-71: 5 x TIMEOUT_ACK_MS = 17,5 s. Con el techo de orfandad en 12 s este
            // camino NUNCA se alcanzaba -saltaba el ambar sobre el 2o o 3er reintento- y
            // el comentario decia "12.5s" porque venia de un timeout de 2500 ms retirado
            // el 31/07. Con 25 s los cinco reintentos existen de verdad.
            //
            // N-73: y por eso esta alarma importa. Distinguir "se agotaron los cinco
            // reintentos" de "silencio total" es la diferencia entre un enlace que se
            // degrada -lluvia, distancia, interferencia- y uno que se corta. Hasta hoy
            // las dos caidas se veian igual desde fuera: una luz ambar.
            bluetooth_reportarAlarma("FALLO_RF", "REINTENTOS_AGOTADOS", "CAMBIO_A_AMBAR");
            estadoC = C_FALLO;
        } else {
            protocolo_enviarPaquete(CMD_GO_GREEN);
            tEsperandoAck = millis();
        }
      }
      break;

    case C_ESPERANDO_ACK_RED:
      if (llego && pkt.command == CMD_ACK_RED) {
        tRef = millis();
        estadoC = C_ESPERA_ESTATICO_TRAS_ESCLAVO;
      } else if (millis() - tEsperandoAck > TIMEOUT_ACK_MS) {
        retryCount++;
        if (retryCount >= CICLO_MAX_REINTENTOS) {
            estadoC = C_FALLO;   // ver N-71 en el ACK_GREEN de arriba
        } else {
            protocolo_enviarPaquete(CMD_GO_RED);
            tEsperandoAck = millis();
        }
      }
      break;

    case C_ESPERA_ESTATICO_TRAS_ESCLAVO:
      if (millis() - tRef >= tiempoDespejeMs) {
        semaforo_iniciarTransicionAVerde(); // Transición Rojo -> Amarillo (4s) -> Verde
        estadoC = C_MASTER_A_VERDE;
      }
      break;

    case C_MASTER_A_VERDE:
      if (semaforo_estable() && semaforo_estado() == S_VERDE) {
        quienVerde = QV_MASTER;
        estadoC = C_IDLE;
      }
      break;

    case C_FALLO:
      if (semaforo_estado() != S_FALLO) {
        semaforo_iniciarFallo();
      }
      break;
  }

  // SFTY-23: al final y no antes. El ciclo ya decidio si necesitaba el bus en esta
  // iteracion, asi que la sincronizacion solo ve un estado ya resuelto y nunca le
  // pisa una orden de luz. Ninguna de las dos llamadas bloquea.
  vigilarCambioDeHora();
  atenderSincronizacion(llego, &pkt, tieneComunicacion);
}

void coordinador_actualizar_background() {
  coordinador_actualizar();
}

bool coordinador_listoParaContar() {
  return estadoC == C_IDLE;
}

bool coordinador_comunicacionPerdida() {
  return estadoC == C_FALLO;
}

const char* coordinador_nombreEstadoMaster() {
  return semaforo_nombreEstado();
}

// --- SFTY-14: Telemetria de calidad de enlace (V8.1) -----------------------

int coordinador_calidadEnlace() {
  if (muestrasLatido == 0) return -1; // aun sin muestras
  uint16_t mascara = (uint16_t)((1u << muestrasLatido) - 1u);
  uint16_t bits = (uint16_t)(ventanaLatidos & mascara);
  uint8_t respondidos = 0;
  while (bits) {            // conteo de bits a 1
    respondidos += (uint8_t)(bits & 1u);
    bits >>= 1;
  }
  return (int)((respondidos * 100u) / muestrasLatido);
}

unsigned long coordinador_tiempoRespuestaMs() {
  return rttMedioMs;
}

int coordinador_latidosSinRespuesta() {
  return latidosSinRespuesta;
}

// --- SFTY-23: Sincronizacion horaria por radio -----------------------------

bool coordinador_sincronizarHora() {
  // Sin hora fiable no se envia NADA. Empujar la hora por defecto del RTC dejaria al
  // Esclavo con una hora inventada que aparenta validez, y sobre esa apariencia se
  // autorizaria despues el Modo Degradado. Es peor que no tener hora.
  if (!reloj_enHora()) return false;
  pendHora = true;
  return true;
}

bool coordinador_medirDesfase() {
  // La diferencia contra un reloj que no esta en hora no mide nada, y acabaria
  // anotada en el acta de pruebas como si fuera un dato.
  if (!reloj_enHora()) return false;
  pendDelta = true;
  return true;
}

bool coordinador_enviarConfigCiclo(uint8_t verdeSeg, uint8_t despejeSeg) {
  cfgVerdeSeg = verdeSeg;
  cfgDespejeSeg = despejeSeg;
  pendConfig = true;
  return true;
}

int8_t coordinador_desfaseEsclavo() {
  return desfaseEsclavo;
}

bool coordinador_desfaseValido() {
  if (!hayDesfase) return false;
  // La antiguedad forma parte de la validez. Un desfase de hace horas presentado como
  // el de ahora es exactamente el fallo que SFTY-23 vino a eliminar, solo que con
  // aspecto de medicion en vez de aspecto de inspeccion ocular.
  return (millis() - tUltimoDesfase) < VIGENCIA_DESFASE_MS;
}

bool coordinador_configConfirmada() { return configConfirmada; }

unsigned long coordinador_msDesdeUltimaSync() {
  // Nunca sincronizado se reporta como el maximo, no como cero: quien compare contra
  // un limite de antiguedad tiene que ver "muy vieja". Devolver cero haria pasar el
  // gate del Modo Degradado a un equipo que jamas hablo con el otro extremo.
  if (!syncAlgunaVez) return 0xFFFFFFFFUL;
  return millis() - tUltimaSyncOk;
}

