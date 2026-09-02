// ===== src/bluetooth.cpp (MAESTRO) =====
#include "bluetooth.h"
#include "pines.h"
#include "coordinador.h"
#include "demanda.h"
#include "modo_automatico.h"
#include "modo_degradado.h"
#include "semaforo.h"
#include "menu.h"
#include "modos.h"
#include "reloj.h"
#include "identidad.h"
#include <string.h>
#include <stdio.h>

// USART1 REMAPEADO a PB7 (RX) / PB6 (TX). Antes iba en PA9/PA10, y se movio
// porque el modulo Bluetooth de campo se conecta al CONECTOR J17 -posiciones 2
// y 3-, que es enchufable. PA9/PA10 no salen a ninguna bornera de la tarjeta:
// para usarlos hay que soldar en las patas del MAX3485 U2 o del propio micro.
//
// Es el MISMO periferico, no un segundo puerto serie: el STM32F103 permite
// sacar USART1 por PB6/PB7 en vez de PA9/PA10, pero solo por un sitio a la vez.
// Por eso protocolo_setup() dejo de abrir AiBus, que declaraba el mismo USART1
// sobre PA10/PA9 a 115200 -ver el comentario de alli-.
//
// Estos dos pines los tenia lcd.cpp para PSB y RST del display; ninguno de los
// dos llevaba datos, asi que la pantalla sigue funcionando con SCLK/SID/CS.
static HardwareSerial SerialBT(PB7, PB6); // USART1 remapeado: PB7 RX, PB6 TX

static unsigned long tUltimaTelemetria = 0;
static char btBufIn[64];
static uint8_t btIdxIn = 0;

// ---------------------------------------------------------------------------
// A3 - EL REGISTRO DE SILENCIO DEL PUERTO J17.
//
// POR QUE LO LLEVA ESTE MICRO. El ESP32 no puede reportar su propia muerte: el equipo
// que sobrevive al fallo es este, asi que el registro es suyo. Hoy, si J17 se calla, no
// lo apunta nadie, y cuando el tecnico llega lo que paso ya no esta.
//
// QUE MIDEN ESTOS TRES NUMEROS Y QUE NO. ESTA ES LA MITAD IMPORTANTE.
//
// MEDIDO sobre el puente, no supuesto:
//   ESP32_Expansion/src/enlace_stm32.cpp:27-33  "6.4 - SILENCIO NO ES ORDEN [...] aqui
//                                               NO se manda nada"
//   ESP32_Expansion/src/puente.cpp:29-35        lo mismo por el otro lado
//   enlace_escribirLinea() tiene UN solo llamador -puente.cpp:116-, y es el reenvio
//   VERBATIM de lo que mando la app.
// Y sobre la app: no hay un solo envio periodico al firmware; sus dos setInterval son
// el reloj de la cabecera y vigilarEnlace(), que solo ESCUCHA.
//
// O sea que por J17 entra EXACTAMENTE lo que un dedo pulsa en el telefono. De modo que
// esto cuenta SILENCIOS DEL PUERTO, no muertes del puente: mientras nadie use la app el
// puerto esta mudo con el puente perfectamente vivo, y desde aqui "el puente no dice
// nada" y "el puente no esta" NO SE DISTINGUEN -lo dice el propio puente en los dos
// comentarios de arriba-.
//
// Se nombra asi, y no "cortes del puente", porque un contador que prometiera eso seria
// el RF:98% otra vez: un numero con forma de medida. El dia que el ESP32 emita un latido
// propio -AB-1, ESP32_Expansion/src/main.cpp:22-28, abierto y del responsable- estos
// mismos tres numeros pasan a ser el registro de cortes de verdad SIN TOCAR UNA LINEA de
// aqui. Esa es la mitad que si se puede construir hoy.
//
// NO HAY UMBRAL, Y ES LA DECISION DE DISENO. Un silencio se define por sus DOS EXTREMOS
// -la linea que lo abre y la que lo cierra-, nunca por un limite. Un limite seria un
// numero que nadie ha decidido gobernando lo que el tecnico ve, y ademas la precondicion
// para que alguien acabe alimentando este silencio con el de la radio. SFTY6_SILENCIO_MS
// vigila la RADIO y no se nombra en este fichero: son dos silencios y dos instrumentos.
//
// Y ESTO CUENTA; NO ACTUA. De aqui no sale una sola escritura de luz. El ambar
// automatico sigue reservado a los caminos que ya lo tienen -SFTY-6 y el watchdog-: la
// maquina no decide sola operar de un modo que nadie pidio.
//
// SE MIDE POR LINEA COMPLETA, no por byte: el puente entrega lineas y el receptor de
// abajo solo actua sobre lineas. La PRIMERA linea tras el arranque cierra el silencio
// que empezo en el arranque, que es un silencio real y el unico capaz de contestar
// "llevo tres dias sin que nadie hable por aqui".
//
//
// EL ULTIMO SILENCIO NO SE GUARDA, Y NO ES UN OLVIDO. Se publica en el instante en que
// se cierra -que es el unico en que alguien puede estar mirando, ver abajo-, asi que
// entre llamada y llamada no lo lee nadie. MEDIDO: con la variable static puesta,
// arm-none-eabi-nm sobre el .elf NO la encuentra en .bss -el enlazador ya sabia que
// nadie la lee y la habia borrado-. Dejarla declarada seria anunciar un estado que no
// existe: quien lo leyera creeria que se puede consultar mas tarde, y no se puede.
// LIMITE DECLARADO: millis() da la vuelta a los 49,7 dias. La resta sin signo mide bien
// UNA vuelta; un silencio mas largo que eso queda aliasado. Se escribe en vez de
// esconderse.
// ---------------------------------------------------------------------------
static unsigned long tUltimaLineaJ17  = 0;  // millis() de la ultima linea completa
static unsigned long j17Silencios     = 0;  // cerrados: uno por linea. 0 = ninguna aun
static unsigned long j17SilencioMaxMs = 0;  // el mas largo desde el arranque

static uint8_t calcularChecksum(const char* str) {
  uint8_t crc = 0;
  while (*str && *str != '*') {
    crc ^= (uint8_t)(*str);
    str++;
  }
  return crc;
}

static void enviarTramaConCrc(const char* payload) {
  uint8_t crc = calcularChecksum(payload + 1); // Salta el '$' inicial
  // N-108: 160 y no 140. El $ALARM pasa a llevar el ultimo tramo del enlace y su peor
  // caso medido son 128 B de payload; con 140 aqui, el cierre del checksum entraba
  // justo y una CAUSA mas larga habria truncado la trama SIN AVISO -y una trama
  // truncada es una que el otro extremo descarta por checksum, o sea una alarma que
  // desaparece justo cuando hace falta-.
  char tramaCompleta[160];
  snprintf(tramaCompleta, sizeof(tramaCompleta), "%s*%02X\r\n", payload, crc);
  SerialBT.print(tramaCompleta);
}

void bluetooth_setup() {
  // EL CHIP ES U2, NO U3. Aqui ponia "U3" y estaba invertido; se deja escrito para que no
  // se vuelva a poner al reves. Trazado red por red sobre el esquematico bueno
  // (01_Firmware/Controladora_Semaforos/.../Controladora_Semaforos.kicad_sch):
  //
  //   U2 = MAX3485 del USART1: RO(1)->PA10, ~RE(2) y DE(3)->PA8, DI(4)->PA9. Par A/B por J10.
  //   U3 = MAX3485 del USART3, que es el de la radio LoRa: PB11, PB12, PB10. Par A/B por J12.
  //
  // Y FALTA EL MATIZ QUE IMPORTA: PA8 gobierna A LA VEZ ~RE (pin 2) y DE (pin 3) de U2.
  // Ponerlo HIGH apaga el RECEPTOR -que es lo que se busca, porque asi PA10 queda libre
  // para el modulo Bluetooth- pero deja el TRANSMISOR ENCENDIDO. Consecuencia: U2 vuelca
  // la telemetria por J10 de forma permanente y esa linea NO puede recibir nunca. Hoy es
  // inofensivo porque J10 esta vacio; deja de serlo el dia que alguien cuelgue algo de
  // J10, y ese dia hay que tocar este codigo, no el cableado.
  //
  // Es la leccion del repetidor del 31/07/2026, ya escrita en 01_Firmware/TROUBLESHOOTING.md
  // ("RS-485 half-duplex: el control DE/RE"): si un DE/RE se queda permanentemente en alto,
  // esa linea queda bloqueada en AMBOS sentidos.
  pinMode(RS485_IN_DE_RE, OUTPUT);
  digitalWrite(RS485_IN_DE_RE, HIGH); // Apaga el receptor RO de U2 y libera PA10 al modulo Bluetooth
  SerialBT.begin(9600);
}

void bluetooth_reportarAlarma(const char* evento, const char* causa, const char* accion) {
  char horaBuf[16];
  if (reloj_enHora()) {
    snprintf(horaBuf, sizeof(horaBuf), "%02u:%02u:%02u", reloj_hora(), reloj_minuto(), reloj_segundo());
  } else {
    strncpy(horaBuf, "--:--:--", sizeof(horaBuf));
  }

  // N-108 - LA ALARMA LLEVA EL ULTIMO TRAMO DEL ENLACE, Y ES LA PREGUNTA DEL CAMPO.
  //
  // Hasta hoy decia QUE se cayo -"CAUSA:SILENCIO_25000ms"- y no DESDE DONDE venia. El
  // reporte del 27/08 -"se va a degradado cada nada cuando llueve"- necesitaba
  // exactamente esto: si el RF venia en 90% y cayo de golpe, es una obstruccion o una
  // antena; si llevaba media hora en 40% con el RTT subiendo, es alcance o lluvia. Los
  // tres numeros ya existian y solo vivian en el $STATUS de 1 Hz, que no se graba.
  //
  // SE COMPONEN AQUI Y NO EN EL LLAMANTE a proposito: los dos sitios que disparan esta
  // alarma estan en coordinador.cpp, y hacerles pasar el tramo obligaria a tocar el
  // fichero que decide el ciclo para anadir telemetria. Este modulo ya sabe preguntarlo.
  //
  // Y se marcan igual que en el $STATUS: sin muestras no hay cifra, hay "--".
  const int rf = coordinador_calidadEnlace();
  char tramo[40];
  if (rf < 0) {
    snprintf(tramo, sizeof(tramo), "RF:--,RTT:--,SINRESP:%d", coordinador_latidosSinRespuesta());
  } else {
    snprintf(tramo, sizeof(tramo), "RF:%d%%,RTT:%lums,SINRESP:%d",
             rf, coordinador_tiempoRespuestaMs(), coordinador_latidosSinRespuesta());
  }

  // 144 y no 100. El peor caso MEDIDO sumando los literales que de verdad se pasan
  // -CAUSA:REINTENTOS_AGOTADOS con RF:100%, RTT de 4 cifras y SINRESP:999- son 128 B, y
  // con 100 la trama se cortaba por la HORA sin que nada lo dijera.
  char payload[144];
  snprintf(payload, sizeof(payload), "$ALARM,NODE:MAESTRO,EVENTO:%s,CAUSA:%s,%s,ACCION:%s,HORA:%s",
           evento, causa, tramo, accion, horaBuf);
  enviarTramaConCrc(payload);
}

void bluetooth_reportarEvento(const char* origen, const char* detalle) {
  char horaBuf[16];
  if (reloj_enHora()) {
    snprintf(horaBuf, sizeof(horaBuf), "%02u:%02u:%02u", reloj_hora(), reloj_minuto(), reloj_segundo());
  } else {
    strncpy(horaBuf, "--:--:--", sizeof(horaBuf));
  }

  // N-114: 112 y no 100. El DETALLE mas largo que hoy entra aqui es el de ORIGEN:RELOJ,
  // y su peor caso POR TIPO son 44 caracteres. Con 100, la trama entera daba 106 de 99:
  // se truncaba POR EL FINAL -o sea por la HORA-, el checksum se calculaba sobre lo que
  // quedo y el otro extremo la habria descartado. Es N-108 letra por letra, y el numero
  // que lo dijo salio de la cuenta que rehace el pack, no de mirarlo. Los 12 B son de
  // pila, no de flash, y el presupuesto de J17 los recalcula solo.
  char payload[112];
  snprintf(payload, sizeof(payload), "$EVENT,NODE:MAESTRO,ORIGEN:%s,DETALLE:%s,HORA:%s",
           origen, detalle, horaBuf);
  enviarTramaConCrc(payload);
}

// Cierra el silencio de J17 que la linea recien recibida acaba de terminar, y lo publica.
//
// SE PUBLICA EN EL INSTANTE EN QUE SE CIERRA PORQUE ES EL UNICO EN QUE SE PUEDE. "Cuanto
// lleva mudo ahora mismo" no es observable desde la app por construccion: la unica forma
// de que alguien este mirando es que acabe de mandar una linea, y esa linea es justo la
// que termina el silencio. Guardar el dato para un comando que lo pida seria guardarlo
// para el unico momento en que ya vale cero.
//
// Por eso sale una linea de bitacora por cada silencio cerrado y no solo por los que
// baten el record: el que le interesa al tecnico que acaba de llegar es el SUYO -"este
// puerto llevaba catorce horas sin que nadie hablara"-, y ese no tiene por que ser el
// mayor. El coste esta medido y cabe: ver el presupuesto de bytes de J17.
//
// EN SEGUNDOS, no en ms. Un corte se cuenta en horas y el ms no aporta nada; en ms el
// peor caso del tipo son diez cifras y la trama se comia el margen del payload -que es
// como N-108 encontro un $ALARM truncandose por la HORA sin que nada lo dijera-.
static void j17RegistrarLinea(unsigned long ahora) {
  const unsigned long silencio = ahora - tUltimaLineaJ17;
  tUltimaLineaJ17 = ahora;
  if (silencio > j17SilencioMaxMs) j17SilencioMaxMs = silencio;
  j17Silencios++;

  char det[48];
  snprintf(det, sizeof(det), "MUDO:%lus,MAX:%lus,N:%lu",
           silencio / 1000UL, j17SilencioMaxMs / 1000UL, j17Silencios);
  bluetooth_reportarEvento("J17", det);
}

// --- Dos envoltorios que devuelven lo que la funcion de abajo no sabe decir -------
//
// Las dos que envuelven son `void` Y ABANDONAN EN SILENCIO si no se cumple su
// precondicion. Desde el despachador eso es invisible: se llama, no pasa nada, y el
// telefono recibe RESULT:OK igual. Aqui se pregunta la MISMA condicion que mira la
// guarda -no una parecida- y se devuelve un bool, que es lo unico que permite mandar
// $ERR en vez de mentir.

// coordinador_pedirCambio() empieza con "if (estadoC != C_IDLE) return;", y
// coordinador_listoParaContar() es literalmente "return estadoC == C_IDLE": no es una
// aproximacion a la guarda, es la guarda leida por la unica puerta publica que hay.
static bool pedirCambioVerificado() {
  if (!coordinador_listoParaContar()) return false;
  coordinador_pedirCambio();
  return true;
}

// reloj_ajustar() empieza con "if (!rtcOperativo) return;" -y reloj_hayCristal()
// devuelve ese mismo rtcOperativo-, y ademas descarta la llamada ENTERA si algun campo
// se sale de rango. Lo primero lo pregunta el llamante, porque tiene motivo propio; lo
// segundo no se puede preguntar antes sin copiar aqui los rangos de reloj.cpp, asi que
// se MIDE despues: se relee la hora que quedo puesta.
//
// El desempate importa cuando el reloj YA estaba en hora: ahi reloj_enHora() sigue
// diciendo true aunque el ajuste se haya descartado entero, y sin releer se contestaria
// OK sobre una hora que no es la que mandaron.
static bool ajustarRelojVerificado(uint8_t h, uint8_t mi, uint8_t s, uint8_t d) {
  reloj_ajustar(h, mi, s, d);
  // Los segundos no se comparan: entre escribir y releer el RTC puede haber avanzado
  // uno. Si el minuto avanza justo en ese hueco esto contesta que no pudo, con la hora
  // bien puesta -es el lado seguro del error, y el operario repite-.
  return reloj_enHora() && reloj_hora() == h && reloj_minuto() == mi;
}

// --- LOS BITS DEL RELOJ, POR EL UNICO CAMINO QUE QUEDA ABIERTO --------------------
//
// N-114 - DOS $ERR DE ESTE FICHERO MANDAN AL TECNICO A UNA PANTALLA TAPIADA.
//
// SIN_CRISTAL_VEA_CONSULTA_RELOJ y SIGUE_PARADO_VEA_CONSULTA_RELOJ nombran la consulta
// de N-45 por su nombre. Esa consulta es MODO_HORA, y MODO_HORA se arma en UN solo
// sitio -menu.cpp, el case 1 de NIVEL_CONFIG-, al que hacen falta DOS botonAceptar():
// uno para bajar al nivel y otro para entrar en la opcion. botonAceptar() es hoy
// "return false" -PB14 y PB15 dejaron de ser pulsadores para ser camaras-, asi que la
// puerta esta tapiada por DOS sitios a la vez, y por Bluetooth no existe SET_MODO:HORA.
// El equipo estaba mandando a leer un instrumento que nadie puede abrir.
//
// Y NO ES UNA CURIOSIDAD: es el instrumento del bloqueante BLQ-2. Sin estos bits,
// SET_RTC distingue "hay reloj" de "no hay reloj" y NO distingue las dos averias que
// mandan a sitios opuestos -lseOn=0, que es firmware o dominio de respaldo, de
// lseOn=1 con lseRdy=0, que es donde SI se mira el cristal-. Es la confusion que ya
// costo cambiar pila, R5 e Y2 tres veces con el hardware sano (N-45).
//
// POR QUE $EVENT Y NO UN COMANDO DE CONSULTA NUEVO. Un comando cuesta una rama mas y,
// sobre todo, EXIGE INTERFAZ: app_01_comandos pone en rojo todo comando que el firmware
// atienda y la app no sepa mandar, con razon -firmware sin interfaz es trabajo que
// nadie puede usar-. El $EVENT ya tiene esa interfaz construida y MEDIDA, no supuesta:
// app.js:755 lo lista en TIPOS_QUE_LA_APP_LEE y app.js:1999-2007 lo pinta con su
// ORIGEN, su DETALLE y su HORA en el REGISTRO DE EVENTOS, que es la pestana 2 y la ven
// los dos roles. Cero lineas de JavaScript para que estos bits lleguen a la pantalla.
//
// SE EMITE DONDE EL FIRMWARE YA MANDA AL TECNICO, y no en un latido periodico ni en el
// arranque: en el arranque del STM32 puede no haber nadie escuchando -el ESP32 y el
// telefono llegan despues- y una trama que nadie oye no es un instrumento. Aqui hay
// alguien mirando por construccion: acaba de recibir el rechazo.
// LIMITE DECLARADO: esto NO es una consulta bajo demanda. Para la segunda visita que
// pide reloj.h -"cnt cambiando entre dos visitas -> el RTC CUENTA"- se repite el mismo
// SET_RTC, que en esta rama se rechaza ANTES de escribir nada y por tanto no cuesta.
//
// EL DETALLE NO LLEVA NI UNA COMA, Y NO ES ESTILO. _camposNmea() de la app parte la
// trama por ',' y cada trozo por su PRIMER ':' (app.js:1798-1810), asi que una coma
// dentro del DETALLE convierte todo lo que va detras en campos sueltos que el pintor de
// $EVENT no mira: los bits saldrian al cable y no llegarian a la pantalla. Seria la
// prueba muerta con forma de telemetria. Se separan con espacio, que es lo que ya hace
// el DESC de SET_MODO:DEGRADADO. Lo vigila reloj_01_consulta_por_bluetooth.
//
// CFG Y ANIO NO VAN, Y ES LO CONTRARIO DE UN OLVIDO. reloj.cpp los saca de rtcOperativo
// -"configurado = rtcOperativo ? rtc.isConfigured() : false"-, y las DOS puertas que
// llaman aqui son justo las que exigen rtcOperativo == false: valdrian 0 y 0 SIEMPRE.
// Publicar dos ceros constantes con forma de medida es el BAT:12.6 otra vez. Los seis
// que quedan son exactamente los que la cabecera de reloj.h usa para razonar.
static void reportarBitsDelReloj() {
  RelojDiag d;
  reloj_diagnostico(&d);

  // Sin RTCEN no se leyo el periferico A PROPOSITO -leer los registros de un periferico
  // sin reloj es un fallo de bus-, y eso se dice con "--" en vez de con un 0: un cero
  // ahi es indistinguible de un contador parado en cero, que es otro diagnostico. Es la
  // misma marca que ya llevan RF, RTT y BAT en el $STATUS.
  //
  // 11 y no 12: un uint32_t no pasa de 4294967295, que son DIEZ cifras mas el cierre.
  // Un byte de mas seria holgura falsa -se paga cuando empuja a recortar el campo que
  // si la necesita-, y este numero entra en la cuenta del payload del $EVENT.
  char cntTxt[11];
  if (d.cntLeido) {
    snprintf(cntTxt, sizeof(cntTxt), "%lu", (unsigned long)d.cnt);
  } else {
    strncpy(cntTxt, "--", sizeof(cntTxt));
  }

  // 48 y no 44. El peor caso POR TIPO son 44 caracteres -cuatro bits, las tres cifras
  // que cabe un uint8_t en RTCSEL y las diez de un uint32_t-. La cuenta NO se deja
  // escrita solo aqui: un comentario no falla cuando alguien alarga un campo. La rehace
  // en cada corrida reloj_01_consulta_por_bluetooth, releyendo este formato, el del
  // $EVENT, los tres buffers y el TIPO de cada campo de RelojDiag.
  char det[48];
  snprintf(det, sizeof(det), "ON:%u RDY:%u BYP:%u SEL:%u EN:%u CNT:%s",
           (unsigned)d.lseOn, (unsigned)d.lseRdy, (unsigned)d.lseByp,
           (unsigned)d.rtcSel, (unsigned)d.rtcEn, cntTxt);
  bluetooth_reportarEvento("RELOJ", det);
}

static void procesarComando(const char* cmd) {
  // SFTY - EL ROJO DE EMERGENCIA NO PIDE PIN, Y ES DELIBERADO.
  //
  // mando.cpp ya lo dejo escrito para el mando de reles: "lo seguro, facil; lo
  // peligroso, dificil". Detener el trafico es la accion SEGURA -el equipo cae a
  // todo-rojo-, asi que ponerle una clave delante solo retrasa a quien esta viendo
  // el incidente. El PIN guarda lo que ABRE paso o mueve luces; no lo que las para.
  //
  // Se acepta tambien la forma con PIN mas abajo: la app la envia asi y el manual
  // la documenta. Las dos entradas hacen lo mismo.
  if (strcmp(cmd, "CMD:FORZAR_ROJO") == 0) {
    coordinador_forzarRojoTotal();
    enviarTramaConCrc("$ACK,CMD:FORZAR_ROJO,RESULT:OK");
    bluetooth_reportarEvento("APP_BLUETOOTH", "FORZAR_ROJO_SIN_PIN");
    return;
  }

  // Validación estricta de PIN de 4 dígitos (1234)
  //
  // OTRAS DOS ORDENES ENTRAN SIN PIN, POR EL MISMO CRITERIO DE ARRIBA: ni MENU ni
  // ALCANCE abren paso -el primero deja el equipo en la pantalla, sin ciclo; el segundo
  // en rojo fijo-, y el PIN guarda lo que ABRE, no lo que para. Se aceptan tambien con
  // PIN, igual que FORZAR_ROJO: la app antepone la clave a todo lo que no este en su
  // lista SIN_PIN, y una orden que solo se aceptara sin PIN seria inalcanzable desde el
  // celular.
  //
  // Lo que se mueve es DONDE EMPIEZA LA ACCION, no la cadena de comparaciones: las dos
  // formas caen en el mismo strcmp de mas abajo. Una segunda cadena para las ordenes sin
  // PIN serian dos contratos que alguien tendria que sincronizar, y el dia que uno se
  // quede atras el comando funciona por una puerta y contesta DESCONOCIDO por la otra.
  const char* accion;
  if (strncmp(cmd, "CMD:PIN:1234:", 13) == 0) {
    accion = cmd + 13;
  } else if (strncmp(cmd, "CMD:", 4) == 0 &&
             (strcmp(cmd + 4, "SET_MODO:MENU") == 0 ||
              strcmp(cmd + 4, "SET_MODO:ALCANCE") == 0)) {
    accion = cmd + 4;
  } else {
    enviarTramaConCrc("$ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO");
    return;
  }

  if (strcmp(accion, "SET_MODO:AUTO") == 0) {
    modoActual_set(MODO_AUTOMATICO);
    coordinador_iniciarModo();
    enviarTramaConCrc("$ACK,CMD:SET_MODO:AUTO,RESULT:OK");
    bluetooth_reportarEvento("APP_BLUETOOTH", "SET_MODO_AUTO");
  } else if (strcmp(accion, "SET_MODO:MANUAL") == 0) {
    modoActual_set(MODO_MANUAL);
    coordinador_iniciarModo();
    enviarTramaConCrc("$ACK,CMD:SET_MODO:MANUAL,RESULT:OK");
    bluetooth_reportarEvento("APP_BLUETOOTH", "SET_MODO_MANUAL");
  } else if (strcmp(accion, "SET_MODO:AMBAR") == 0) {
    modoActual_set(MODO_AMBAR);
    enviarTramaConCrc("$ACK,CMD:SET_MODO:AMBAR,RESULT:OK");
    bluetooth_reportarEvento("APP_BLUETOOTH", "SET_MODO_AMBAR");
  } else if (strcmp(accion, "SET_MODO:MENU") == 0) {
    // EN DEGRADADO NO SE SALTA AL MENU. Se pide la salida, que es el todo-rojo de 30 s
    // de ROJO_TRANSICION_MS. Sin el, esta unidad cae a rojo mientras la otra sigue
    // dando verde por reloj, y el escenario peligroso de ese modo es exactamente que
    // UNA SOLA PUNTA lo abandone. Es la MISMA puerta que el boton 4 del gabinete.
    if (modoActual_get() == MODO_DEGRADADO) {
      // El bool distingue haber arrancado la salida de encontrarla ya en marcha, y en
      // ninguno de los dos casos se contesta OK: el menu tarda todavia el todo-rojo
      // entero en llegar, y decir OK seria dar por hecho un cambio que no ha ocurrido.
      if (modo_degradado_pedirSalida()) {
        enviarTramaConCrc("$ACK,CMD:SET_MODO:MENU,RESULT:SALIENDO_TODO_ROJO");
        bluetooth_reportarEvento("APP_BLUETOOTH", "SALIDA_DEGRADADO_PEDIDA");
      } else {
        enviarTramaConCrc("$ERR,CMD:SET_MODO:MENU,DESC:YA_VUELVE_AL_MENU");
      }
    } else {
      modoActual_set(MENU);
      menu_setup();
      enviarTramaConCrc("$ACK,CMD:SET_MODO:MENU,RESULT:OK");
      bluetooth_reportarEvento("APP_BLUETOOTH", "SET_MODO_MENU");
    }
  } else if (strcmp(accion, "SET_MODO:ALCANCE") == 0) {
    // Del Degradado se sale por su puerta y no por aqui. El rojo fijo del alcance es
    // seguro por si mismo, pero llegar a el saltandose el todo-rojo deja a la otra
    // punta dando verde sin que nadie haya verificado las dos.
    if (modoActual_get() == MODO_DEGRADADO) {
      enviarTramaConCrc("$ERR,CMD:SET_MODO:ALCANCE,DESC:EN_MARCHA_PARE_EL_MODO");
    } else {
      modoActual_set(MODO_ALCANCE);
      enviarTramaConCrc("$ACK,CMD:SET_MODO:ALCANCE,RESULT:OK");
      bluetooth_reportarEvento("APP_BLUETOOTH", "SET_MODO_ALCANCE");
    }
  } else if (strcmp(accion, "SET_MODO:INTELIGENTE") == 0) {
    // PIDE PIN porque arranca un ciclo que DA VERDES, y por eso mismo no se entra desde
    // el Degradado: la salida de aquel obliga a un todo-rojo de 30 s en las dos puntas,
    // y ponerse a ciclar sin cumplirlo deja a la otra unidad en su ciclo por reloj.
    if (modoActual_get() == MODO_DEGRADADO) {
      enviarTramaConCrc("$ERR,CMD:SET_MODO:INTELIGENTE,DESC:EN_MARCHA_PARE_EL_MODO");
    } else {
      modoActual_set(MODO_INTELIGENTE);
      enviarTramaConCrc("$ACK,CMD:SET_MODO:INTELIGENTE,RESULT:OK");
      bluetooth_reportarEvento("APP_BLUETOOTH", "SET_MODO_INTELIGENTE");
    }
  } else if (strcmp(accion, "SET_MODO:DEGRADADO") == 0) {
    // LA PUERTA ES LA MISMA QUE LA DE LA PANTALLA Y LA DEL MANDO. Tres vias de entrada
    // con tres criterios serian una sola puerta: la mas floja de las tres. Y esta es la
    // unica del firmware que enciende un verde sin confirmacion del otro extremo.
    const MotivoDegradado m = modo_degradado_evaluarEntrada();
    if (m != MDG_OK) {
      // El motivo se compone con los MISMOS dos textos que ensena el gabinete. Una
      // tabla propia para el celular seria una tercera que alguien tendria que
      // sincronizar, y el dia que difieran el tecnico de arriba y el de abajo leerian
      // causas distintas del mismo rechazo.
      char p[80];
      snprintf(p, sizeof(p), "$ERR,CMD:SET_MODO:DEGRADADO,DESC:%s %s",
               modo_degradado_motivoL1(m), modo_degradado_motivoL2(m));
      enviarTramaConCrc(p);
    } else {
      modoActual_set(MODO_DEGRADADO);
      enviarTramaConCrc("$ACK,CMD:SET_MODO:DEGRADADO,RESULT:OK");
      bluetooth_reportarEvento("APP_BLUETOOTH", "SET_MODO_DEGRADADO");
    }
  } else if (strcmp(accion, "FORZAR_ROJO") == 0) {
    coordinador_forzarRojoTotal();
    enviarTramaConCrc("$ACK,CMD:FORZAR_ROJO,RESULT:OK");
    bluetooth_reportarEvento("APP_BLUETOOTH", "FORZAR_ROJO_TOTAL");
  } else if (strcmp(accion, "MANUAL:CAMBIAR_TURNO") == 0) {
    // Antes se mandaba OK pasara lo que pasara. coordinador_pedirCambio() abandona en
    // silencio si el coordinador no esta en reposo -esta a mitad de una transicion, que
    // incluye un todo-rojo de despeje en curso-, asi que el operario recibia la
    // confirmacion, no cambiaba nada, y volvia a pulsar creyendo que no le hacian caso.
    //
    // No se fuerza el cambio: el rechazo es correcto -partir un despeje por la mitad es
    // justo lo que no se puede hacer-. Lo que faltaba era DECIRLO.
    if (pedirCambioVerificado()) {
      enviarTramaConCrc("$ACK,CMD:CAMBIAR_TURNO,RESULT:OK");
      bluetooth_reportarEvento("APP_BLUETOOTH", "CAMBIAR_TURNO_MANUAL");
    } else {
      enviarTramaConCrc("$ERR,CMD:CAMBIAR_TURNO,DESC:EN_TRANSICION_REINTENTE");
    }
  } else if (strcmp(accion, "TEST_LEDS") == 0) {
    semaforo_iniciarTestLeds();
    enviarTramaConCrc("$ACK,CMD:TEST_LEDS,RESULT:STARTING_6S");
    bluetooth_reportarEvento("APP_BLUETOOTH", "TEST_LEDS_INICIADO");
  } else if (strncmp(accion, "SET_TIEMPOS:", 12) == 0) {
    // N-69: los tiempos del ciclo, desde el celular en vez de subiendo a la pantalla.
    //
    // Los limites NO se comprueban aqui: los tiene modoAutomatico_fijarTiempos(), que
    // es quien conoce el ciclo. Este sitio solo traduce texto a numeros. Repetir los
    // rangos en los dos lados seria una segunda copia que alguien tendria que
    // sincronizar -y el dia que difieran, la app dejaria pasar lo que el ciclo rechaza-.
    //
    // EL %c DE MAS NO SE LEE NUNCA: ESTA PARA QUE sscanf DELATE LO QUE SOBRA.
    //
    // sscanf cuenta CAMPOS CONVERTIDOS y no exige que la cadena se acabe: se lleva sus
    // tres numeros y se desentiende de lo que venga pegado detras. Con dos ordenes en el
    // mismo buffer -"8,8,50CMD:FORZAR_ROJO", que es lo que queda cuando una trama entra a
    // medias y la siguiente se le concatena- esto convertia 3 y entraba como si fuera una
    // trama limpia: el equipo ejecutaba la VIEJA, contestaba OK con el nombre de la vieja,
    // y la que el operario acababa de pulsar -que puede ser el rojo de emergencia- se
    // perdia sin un solo aviso.
    //
    // Con el %c detras la cuenta separa los dos casos: una cadena que se acaba convierte
    // 3 -el %c se queda sin nada que leer-, y una con basura pegada convierte 4. Se exige
    // el 3 EXACTO, y todo lo demas se rechaza por formato: ante dos ordenes pegadas la
    // respuesta segura es no ejecutar ninguna y decirlo, no adivinar cual era.
    int v = 0, r = 0, d = 0;
    char sobra = 0;
    if (sscanf(accion + 12, "%d,%d,%d%c", &v, &r, &d, &sobra) != 3) {
      enviarTramaConCrc("$ERR,CMD:SET_TIEMPOS,DESC:FORMATO_INVALIDO");
    } else if (modoAutomatico_enMarcha()) {
      // Con el ciclo corriendo no se tocan: bajar un tiempo a mitad de fase acortaria
      // la fase EN CURSO, y una de esas fases es el todo-rojo de despeje.
      enviarTramaConCrc("$ERR,CMD:SET_TIEMPOS,DESC:EN_MARCHA_PARE_EL_MODO");
    } else if (!modoAutomatico_fijarTiempos((uint8_t)v, (uint8_t)r, (uint8_t)d)) {
      enviarTramaConCrc("$ERR,CMD:SET_TIEMPOS,DESC:RANGO");
    } else {
      enviarTramaConCrc("$ACK,CMD:SET_TIEMPOS,RESULT:OK");
      bluetooth_reportarEvento("APP_BLUETOOTH", "TIEMPOS_CAMBIADOS");
    }
  } else if (strncmp(accion, "SET_RTC:", 8) == 0) {
    // CMD:PIN:1234:SET_RTC:YYYY-MM-DD,HH:MM:SS
    //
    // ESTE COMANDO CONTESTABA OK SIN HABER PUESTO NADA, y con N-17 confirmado en
    // hardware -el cristal Y2 que no oscila- ese era el caso NORMAL, no el raro: sin
    // oscilador reloj_ajustar() se abstiene a proposito -escribir dejaria una hora en
    // un contador que nadie hace avanzar-, y aqui se mandaba la confirmacion igual. El
    // tecnico se iba del poste creyendo que dejo el reloj puesto.
    //
    // Los tres finales son distintos y el operario necesita los tres distintos: no hay
    // con que contar el tiempo; la hora no entro; la hora entro y va camino del Esclavo.
    //
    // El %c del final es el mismo cierre que lleva SET_TIEMPOS, y por el mismo motivo:
    // sin el, "...,18:25:00CMD:FORZAR_ROJO" -dos ordenes pegadas en el buffer- convierte
    // sus 6 campos, pone la hora y contesta OK, tirando la orden de emergencia que venia
    // detras. Aqui ademas la basura entra en un comando que ESCRIBE en el RTC: se rechaza
    // antes de tocar nada.
    int y, mo, d, h, mi, s;
    char sobra = 0;
    if (sscanf(accion + 8, "%d-%d-%d,%d:%d:%d%c", &y, &mo, &d, &h, &mi, &s, &sobra) != 6) {
      enviarTramaConCrc("$ERR,CMD:SET_RTC,DESC:FORMATO_INVALIDO");
    } else if (!reloj_hayCristal()) {
      // No se nombra ninguna pieza: N-45 quito "Es Y2: toca hardware" de la pantalla
      // por senalar un componente sin haberlo medido, y con Y2 nuevo seguia diciendo lo
      // mismo. Lo que el micro VE lo cuenta CONSULTA RELOJ.
      enviarTramaConCrc("$ERR,CMD:SET_RTC,DESC:SIN_CRISTAL_VEA_CONSULTA_RELOJ");
      // Y aqui van los bits que esa consulta ensenaba, porque su pantalla ya no se
      // puede abrir. DETRAS del $ERR y no delante: el $ERR es la respuesta a la orden y
      // el diagnostico la acompana, igual que j17RegistrarLinea() se anota despues de
      // despachar y no antes.
      reportarBitsDelReloj();
    } else if (!ajustarRelojVerificado((uint8_t)h, (uint8_t)mi, (uint8_t)s, (uint8_t)d)) {
      // El ajuste se descarto ENTERO por un campo fuera de rango. Los rangos siguen
      // viviendo en reloj.cpp, que es quien conoce el calendario; aqui solo se relee lo
      // que quedo puesto y se reusa el motivo que ya existe para una trama que no sirve.
      enviarTramaConCrc("$ERR,CMD:SET_RTC,DESC:FORMATO_INVALIDO");
    } else if (!coordinador_sincronizarHora()) {
      // HOY ESTE CAMINO NO PUEDE OCURRIR: sincronizarHora() solo se niega si
      // !reloj_enHora(), que la linea de arriba acaba de comprobar. Se deja porque su
      // bool existe y no se tira, y porque es lo unico que se enterara el dia que a esa
      // funcion le anadan otra precondicion -la hora quedaria puesta aqui y el Esclavo
      // no la recibiria, que es medio arreglo y se lee como entero-.
      enviarTramaConCrc("$ACK,CMD:SET_RTC,RESULT:HORA_PUESTA_SIN_PROPAGAR");
    } else {
      enviarTramaConCrc("$ACK,CMD:SET_RTC,RESULT:OK");
      bluetooth_reportarEvento("APP_BLUETOOTH", "RTC_AJUSTADO_Y_SYNC");
    }
  } else if (strcmp(accion, "REINICIAR_RELOJ") == 0) {
    // N-31. PIDE PIN porque BORRA LA HORA Y TODO EL RESPALDO -ciclo acordado, marca de
    // sincronizacion e indicador del Degradado-, o sea la autorizacion de la que cuelga
    // el unico modo que da verde sin el otro extremo.
    //
    // El bool no se ignora: dice si el oscilador arranco tras el reinicio. Contestar OK
    // en los dos casos mandaria al tecnico a poner la hora en un equipo que no puede
    // contarla. Y no se nombra ninguna pieza -N-45 quito "Es Y2: toca hardware" de la
    // pantalla por afirmar sin haber medido-: lo que sigue lo dice CONSULTA RELOJ.
    if (reloj_reiniciarDominioRespaldo()) {
      enviarTramaConCrc("$ACK,CMD:REINICIAR_RELOJ,RESULT:CRISTAL_OK_PONGA_LA_HORA");
      bluetooth_reportarEvento("APP_BLUETOOTH", "RELOJ_REINICIADO");
    } else {
      enviarTramaConCrc("$ERR,CMD:REINICIAR_RELOJ,DESC:SIGUE_PARADO_VEA_CONSULTA_RELOJ");
      // El mismo motivo que en SET_RTC: este $ERR nombra una consulta que no se puede
      // abrir. Y aqui el dato vale mas todavia, porque el dominio de respaldo ACABA de
      // reiniciarse: los bits dicen si el oscilador se quedo sin pedir -lseOn=0, que no
      // es el cristal- o pedido y sin arrancar -lseOn=1, lseRdy=0, que si lo es-.
      reportarBitsDelReloj();
    }
  } else if (strcmp(accion, "DEMANDA") == 0) {
    // NO ES SOLICITAR_PASO, y la diferencia no es de nombre: alli el Esclavo PIDE a
    // este equipo (SFTY-27), y aqui el que decide ya es este. La peticion entra por la
    // misma puerta que la camara y la gobierna modo_inteligente.cpp.
    if (modoActual_get() != MODO_INTELIGENTE) {
      // Fuera del Modo Inteligente nadie lee la demanda: registrarla y contestar OK
      // seria apuntar una peticion que ningun ciclo va a mirar.
      enviarTramaConCrc("$ERR,CMD:DEMANDA,DESC:SOLO_EN_MODO_INTELIGENTE");
    } else if (demanda_solicitar()) {
      enviarTramaConCrc("$ACK,CMD:DEMANDA,RESULT:REGISTRADA");
      bluetooth_reportarEvento("APP_BLUETOOTH", "DEMANDA_LOCAL");
    } else {
      // No se finge una peticion que se descarto: si el operario no sabe que su
      // pulsacion cayo en la ventana de silencio, volvera a pulsar creyendo que no le
      // hacen caso.
      enviarTramaConCrc("$ERR,CMD:DEMANDA,DESC:REPITA_EN_UNOS_SEGUNDOS");
    }
  } else {
    enviarTramaConCrc("$ERR,CMD:DESCONOCIDO,DESC:COMANDO_NO_SOPORTADO");
  }
}

static const char* obtenerNombreModo(ModoSistema m) {
  switch (m) {
    case MENU: return "MENU";
    case MODO_MANUAL: return "MANUAL";
    case MODO_AUTOMATICO: return "AUTO";
    case MODO_INTELIGENTE: return "INTELIGENTE";
    case MODO_ALCANCE: return "ALCANCE";
    case MODO_HORA: return "HORA";
    case MODO_DEGRADADO: return "DEGRADADO";
    case MODO_AMBAR: return "AMBAR";
    default: return "DESCONOCIDO";
  }
}

bool bluetooth_testLedsActivo() {
  return semaforo_testLedsEnCurso();
}

void bluetooth_loop() {
  const unsigned long ahora = millis();

  // 1. Recepción de Comandos desde la App Móvil
  while (SerialBT.available() > 0) {
    char c = (char)SerialBT.read();
    if (c == '\n' || c == '\r') {
      if (btIdxIn > 0) {
        btBufIn[btIdxIn] = '\0';
        procesarComando(btBufIn);
        btIdxIn = 0;
        // A3: DESPUES de despachar, no antes. La linea que llega puede ser el rojo de
        // emergencia, y anteponerle una trama de bitacora le mete el tiempo de cable de
        // esa trama por delante. El registro es diagnostico; la orden, no.
        //
        // Y se anota la linea RECIBIDA, se haya reconocido o no: quien mide el silencio
        // del puerto es el puerto. Una linea que el despachador rechaza por desconocida
        // sigue siendo prueba de que el puente esta vivo y hablando.
        j17RegistrarLinea(ahora);
      }
    } else if (btIdxIn < sizeof(btBufIn) - 1) {
      btBufIn[btIdxIn++] = c;
    }
  }

  // 2. Emisión periódica de Telemetría cada 1000ms ($STATUS,...)
  if (ahora - tUltimaTelemetria >= 1000) {
    tUltimaTelemetria = ahora;

    const char* modoStr = obtenerNombreModo(modoActual_get());
    const char* estadoStr = coordinador_nombreEstadoMaster();

    // N-108 - UN CAMPO QUE NO SE MIDE SE MARCA; NO SE APLASTA A UN NUMERO.
    //
    // Aqui ponia "if (rfCalidad < 0) rfCalidad = 0;". El -1 de calidadEnlace() significa
    // AUN NO HAY MUESTRAS -no ha cerrado ni un latido-, y convertirlo en 0 publicaba
    // "RF:0%", que en el tablero se lee como enlace caido: un equipo sano recien
    // arrancado se veia igual que una radio muerta. Y rttMedioMs vale 0 en ese mismo
    // instante, o sea "RTT:0ms", que se lee como un enlace perfecto. Los dos numeros
    // mentian a la vez y en direcciones opuestas.
    //
    // Con "--" el tablero dice lo unico cierto: todavia no se sabe. Es la misma regla
    // que ya se aplica a HORA cuando el reloj no esta en hora.
    const int rfCalidad = coordinador_calidadEnlace();
    // 13 y no 8: calidadEnlace() devuelve 0..100 por construccion, pero eso lo sabe el
    // coordinador, no este fichero -y no lo sabe el compilador, que avisa con -Wall-. Un
    // buffer dimensionado por una invariante que vive en OTRO modulo es el que se rompe
    // en silencio el dia que ese modulo cambie; se dimensiona para el tipo, que es lo
    // unico que este lado puede garantizar.
    char rfTxt[13];
    char rttTxt[16];
    if (rfCalidad < 0) {
      strncpy(rfTxt, "--", sizeof(rfTxt));
      strncpy(rttTxt, "--", sizeof(rttTxt));
    } else {
      snprintf(rfTxt, sizeof(rfTxt), "%d%%", rfCalidad);
      snprintf(rttTxt, sizeof(rttTxt), "%lums", coordinador_tiempoRespuestaMs());
    }

    char horaBuf[16];
    if (reloj_enHora()) {
      snprintf(horaBuf, sizeof(horaBuf), "%02u:%02u:%02u", reloj_hora(), reloj_minuto(), reloj_segundo());
    } else {
      strncpy(horaBuf, "--:--:--", sizeof(horaBuf));
    }

    // Cuenta de segundos transcurridos en fase actual (T:)
    unsigned long tFaseSeg = (ahora / 1000UL) % 60UL;

    char serieTxt[7];
    identidad_texto(serieTxt);

    // N-108 - BAT SE MARCA, EN LAS DOS PUNTAS. El 12.6 era un LITERAL: no hay un solo
    // analogRead() en Maestro/src, Maestro/include, Esclavo/src ni Esclavo/include
    // -MEDIDO: grep sin una coincidencia-. Un tablero que ensena 12,6 V fijos no esta
    // informando de la bateria, esta impidiendo que nadie pregunte por ella: el tecnico
    // ve el numero bueno y descarta la alimentacion como causa sin haberla mirado.
    // Vuelve a haber cifra cuando haya divisor y una entrada analogica que lo lea.
    char payload[128];
    snprintf(payload, sizeof(payload),
             "$STATUS,NODE:MAESTRO,SERIE:%s,MODO:%s,ESTADO:%s,T:%lu,RF:%s,RTT:%s,BAT:--,HORA:%s",
             serieTxt, modoStr, estadoStr, tFaseSeg, rfTxt, rttTxt, horaBuf);

    enviarTramaConCrc(payload);

    // N-108 - UN $EVENT EN CADA CAMBIO DE ESTADO DEL ENLACE, CON SU VALOR.
    //
    // Hasta hoy la caida del radio solo dejaba rastro cuando ya era total: el $ALARM de
    // SFTY-6. Lo que el campo pregunta -"desde cuando venia mal"- no lo contestaba nadie,
    // porque el RF% solo existe en el $STATUS de 1 Hz, que no se graba en ningun sitio.
    //
    // LOS TRES ESTADOS SALEN DEL FIRMWARE, NO DE UN UMBRAL INVENTADO. C_FALLO ya existe y
    // lo publica coordinador_comunicacionPerdida(); "sin medida" es el -1 de
    // calidadEnlace(). Poner aqui un "RF < 70% = degradado" seria una constante que nadie
    // ha decidido gobernando lo que el tecnico ve, asi que el numero VIAJA EN EL DETALLE y
    // el juicio lo hace quien lo lee.
    {
      static int8_t enlaceAnt = -2;  // -2: aun no se ha publicado ninguno
      const int8_t enlaceAhora = coordinador_comunicacionPerdida() ? 0
                              : (rfCalidad < 0 ? 1 : 2);
      if (enlaceAhora != enlaceAnt) {
        enlaceAnt = enlaceAhora;
        char det[28];
        snprintf(det, sizeof(det), "%s_RF:%s",
                 enlaceAhora == 0 ? "PERDIDO" : (enlaceAhora == 1 ? "SIN_MEDIDA" : "OK"),
                 rfTxt);
        bluetooth_reportarEvento("ENLACE_RF", det);
      }
    }
  }
}
