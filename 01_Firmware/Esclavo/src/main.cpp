#include <Arduino.h>
#include "pines.h"
#include "semaforo.h"
#include "protocolo.h"
#include "demanda.h"    // la puerta unica por la que sale una demanda
#include "reloj.h"           // SFTY-23
#include "botones.h"         // N-16
#include "lcd.h"             // N-16
#include "menu.h"            // N-16
#include "mando.h"           // SFTY-21 / N-19: mando de reles
#include "config_ciclo.h"    // SFTY-23: lo que este fichero publica para el Degradado
#include "modo_degradado.h"  // SFTY-21
#include "respaldo.h"        // N-20: lo que sobrevive al corte de energia
#include "bluetooth.h"       // Módulo Bluetooth Serial (USART1)
#include <IWatchdog.h>

static unsigned long tInicioVerdeEsclavo = 0;

// Backstop de verde máximo. 105 min > los 99 min máximos configurables en el Maestro.
static const unsigned long MAX_VERDE_BACKSTOP_MS = 6300000UL;

// ---------------------------------------------------------------------------
// SFTY-17: Retardo de cortesia antes de responder
//
// En modo REPETIDOR hay una radio intermedia (B2) que acaba de TRANSMITIR la
// orden hacia este nodo y necesita tiempo para volver a RECEPCION. Si el Esclavo
// contesta de inmediato, su respuesta sale mientras B2 todavia esta conmutando y
// B2 sencillamente NO LA OYE: el enlace funciona en un sentido y no vuelve nada.
//
// Sintoma en campo (31/07/2026): con repetidor, el contador del puente marcaba
// C<-Esclavo = 1 byte en dos minutos, mientras la ida fluia con normalidad.
//
// En enlace DIRECTO este retardo es inofensivo: el Maestro espera hasta
// TIMEOUT_ACK_MS = 3500 ms, asi que 200 ms no cambian nada.
static const unsigned long RETARDO_RESPUESTA_MS = 200;

// Respuesta programada: se guarda que hay que contestar y cuando, en vez de
// bloquear el bucle con un delay(). Asi el parpadeo de ambar y el watchdog
// siguen atendidos con normalidad.
static uint8_t respuestaPendiente = 0;   // 0 = ninguna
static uint8_t respuestaParam = 0;
static unsigned long tEnviarRespuesta = 0;

// El param se anadio para SFTY-23: CMD_DELTA_RESP no es un ACK seco, lleva dato
// (el desfase medido). Los ACK previos siguen llamando con un solo argumento.
//
// Solo cabe UNA respuesta pendiente: si llega otra orden dentro de la ventana de
// 200 ms, la nueva pisa a la anterior. Se deja asi a proposito. Las tramas de
// SFTY-23 salen del Maestro estando en menu, con el ciclo parado, de modo que no
// compiten con los ACK de luz; y anadir una cola solo para este caso meteria mas
// codigo del que resuelve en el bucle mas critico del equipo.
static void programarRespuesta(uint8_t comando, uint8_t param = 0) {
  respuestaPendiente = comando;
  respuestaParam = param;
  tEnviarRespuesta = millis() + RETARDO_RESPUESTA_MS;
}

static void atenderRespuestaPendiente() {
  if (respuestaPendiente == 0) return;
  if ((long)(millis() - tEnviarRespuesta) < 0) return;
  protocolo_enviarPaquete(respuestaPendiente, respuestaParam);
  respuestaPendiente = 0;
  respuestaParam = 0;
}

// ---------------------------------------------------------------------------
// SFTY-23: Sincronizacion horaria por radio (lado Esclavo)
//
// NADA de lo que hay en este bloque toca las luces ni el backstop de verde
// maximo. El Esclavo puede estar en verde mientras se le pone en hora y no debe
// enterarse: se escribe en el RTC y en dos variables, punto.
//
// Tampoco se refresca tUltimoComando con estas tramas, y es deliberado. Ese
// temporizador significa "el Maestro sigue GOBERNANDO el cruce", no "hay portadora
// en el aire". Si el Maestro se quedase colgado en la pantalla de hora mandando
// solo sincronizaciones, refrescar el temporizador mantendria al Esclavo en su
// ultimo color de forma indefinida en vez de caer a ambar intermitente a los 12 s.
// La caida a ambar es la conducta segura y no se compra con trafico de servicio.
// ---------------------------------------------------------------------------

// Buffer de aplicacion ATOMICA. Ni la hora ni el dia caben en el unico byte de
// param, asi que el envio llega en CUATRO tramas (D, H, M, S); dia, hora y minuto se
// APARCAN AQUI y el RTC no se toca hasta que llega la de segundos. Nunca puede
// quedar una hora a medias: si se escribiera la hora al vuelo y se perdiera la trama
// de minutos, el Esclavo se quedaria con la hora nueva y el minuto viejo, que es
// peor que no sincronizar, porque reloj_enHora() seguiria diciendo true sobre una
// hora falsa.
//
// EL DIA ENTRA EN ESTA MISMA REGLA, no aparte. Aplicar la hora sin el dia dejaria a
// esta punta sincronizada al segundo pero con SU PROPIO calendario, que es
// exactamente el defecto que CMD_HORA_D viene a cerrar: con calendarios
// desacoplados, un corte de energia en la vuelta de 31 a 1 de una de las dos puntas
// deja a esa en AMBAR sin poder reanudar el Degradado mientras la otra reanuda y da
// VERDE. Media sincronizacion aqui no es media mejora, es el fallo asimetrico
// intacto.
static uint8_t bufDia = 0, bufHora = 0, bufMinuto = 0;
static bool tieneDia = false, tieneHora = false, tieneMinuto = false;
static unsigned long tBufHora = 0;

// Caducidad del buffer. Dia, hora y minuto son una FOTO del reloj del Maestro tomada
// en el instante en que armo las tramas; cuanto mas envejecen aqui, mas se alejan de
// la verdad. El caso feo es el cambio de minuto: con H y M capturados en 10:59:58 y
// los segundos llegando ya en las 11:00, se escribiria 10:59:0x, un minuto atrasado.
// El dia caduca CON los demas y no por separado: es parte de la misma foto -en la
// vuelta de medianoche envejece igual que la hora- y separarlo abriria la puerta a
// aplicar un dia de un envio con una hora de otro.
// Las cuatro tramas viajan seguidas (milisegundos), asi que 3 s es holgado para un
// envio normal y a la vez acota cuanto puede envejecer la foto. Pasada la ventana
// se descarta y el Maestro tendra que repetir el envio entero.
static const unsigned long VENTANA_HORA_MS = 3000;

static void olvidarBufferHora() {
  tieneDia = false;
  tieneHora = false;
  tieneMinuto = false;
}

static void caducarBufferHora() {
  if (!tieneDia && !tieneHora && !tieneMinuto) return;
  if (millis() - tBufHora > VENTANA_HORA_MS) olvidarBufferHora();
}

// FASE 2 (03/08/2026): la configuracion del ciclo se mudo a src/config_ciclo.cpp.
//
// Vivia aqui, y el delator de que no era su sitio estaba a la vista: config_
// verdeSegundos() y sus tres hermanas son API PUBLICA -las declara config_ciclo.h y
// las consume el Modo Degradado- y estaban IMPLEMENTADAS EN EL PUNTO DE ENTRADA. Un
// modulo cuya implementacion vive en main.cpp no se puede probar solo.
//
// Ahi vive tambien el unico fallo abierto del validador del Esclavo, el 30/31 -el par
// verde+despeje mezclado entre envios-, que ahora se ejerce con:
//
//     python 01_Firmware/Simulaciones/banco/correr.py --pack esclavo_03

// Desfase entre el segundo que anuncia el Maestro y el nuestro, resuelto por el
// camino CORTO del circulo 0..59.
//
// La resta cruda no vale: con el Maestro en el segundo 1 y nosotros en el 59, da
// 1 - 59 = -58, que se leeria como "el Esclavo va 58 s adelantado", cuando lo que
// pasa es que el Maestro ya cambio de minuto y el Esclavo va 2 s ATRASADO. Al
// cruzar la mitad del circulo hay que corregir: -58 + 60 = +2, que es la respuesta
// correcta. La regla es que por encima de +30 se resta 60 y por debajo de -30 se
// suma 60. Signo: positivo = el Maestro va por delante, el Esclavo esta atrasado.
//
// LIMITE INHERENTE, que conviene tener escrito: mandando solo segundos, el
// resultado vive en +-30 s. Un desfase real de 45 s se mide como -15 s y no hay
// forma de distinguirlos desde aqui. Es aceptable porque esta medida existe para
// verificar una sincronizacion que se acaba de hacer, no para detectar derivas de
// minutos; si el Esclavo llevara minutos de desfase, la solucion no es medirlo, es
// volver a ponerlo en hora.
static int8_t calcularDesfase(uint8_t segundoMaestro) {
  // Sin reloj fiable no hay nada contra lo que comparar. Devolver un numero
  // cualquiera seria lo peor posible: el operario leeria "0 s" en pantalla y se
  // marcharia convencido de que las dos puntas estan cuadradas.
  if (!reloj_enHora()) return DELTA_FUERA_DE_RANGO;

  // Trama corrupta que colo por el CRC, o Maestro con otra idea del contrato.
  if (segundoMaestro > 59) return DELTA_FUERA_DE_RANGO;

  int16_t d = (int16_t)segundoMaestro - (int16_t)reloj_segundo();
  if (d > 30) d -= 60;
  else if (d < -30) d += 60;

  // Saturacion exigida por el contrato. Con la aritmetica de arriba el resultado
  // ya no puede salirse de +-30, asi que esta guarda no puede dispararse hoy; se
  // deja porque el contrato de CMD_DELTA_RESP promete no dar nunca la vuelta y
  // este es el sitio donde se cumple esa promesa, no en la cabeza de quien lo lea.
  if (d > 127 || d < -127) return DELTA_FUERA_DE_RANGO;
  return (int8_t)d;
}

// N-16: la bienvenida se sostiene por tiempo, no con delay().
//
// El Maestro se permite un delay(2000) porque lo hace ANTES de armar el
// watchdog. Aqui el watchdog se arma lo primero (ver el razonamiento del cristal
// Y2, mas abajo) y un delay de dos segundos seria medio periodo del perro
// guardian gastado en un logotipo. Peor todavia: durante esos dos segundos el
// equipo no leeria la radio, y este nodo es el que enciende las luces.
static const unsigned long BIENVENIDA_MS = 1500;
static unsigned long tArranque = 0;
static bool interfazArrancada = false;

void setup() {
  // LAS LUCES PRIMERO, SIEMPRE. Pase lo que pase mas abajo, el cruce ya esta en rojo
  // y es un semaforo. Esa parte del orden no se toca.
  semaforo_setup();
  protocolo_setup();
  semaforo_forzarRojo();

  // N-22: LA PANTALLA SUBE AQUI, JUNTO A LAS LUCES. Antes iba la ULTIMA de todo.
  //
  // El argumento para ponerla al final era bueno: si la LCD estuviera desconectada o
  // averiada, el equipo debia seguir siendo un semaforo. Pero N-17 enseno el reverso,
  // y salio caro. Con la pantalla al final, CUALQUIER cosa que se atasque antes deja
  // esta punta MUDA: pantalla en blanco, sin bienvenida, sin un mensaje. Fue
  // exactamente lo que paso -el cristal del reloj no arrancaba- y el sintoma no se
  // parecia en nada a un problema de reloj. En el Maestro, que si arranca la pantalla
  // pronto, el mismo fallo se vio como un bucle en la bienvenida y se diagnostico.
  //
  // El Esclavo es la punta que menos puede permitirse ser muda: el operario esta abajo
  // junto al OTRO gabinete, y si esta unidad no habla nadie se entera de que le pasa.
  //
  // Subirla no sacrifica lo que protegia el orden anterior, porque lo que de verdad
  // protegia -que un fallo de pantalla no apague el cruce- ya lo garantizan las tres
  // lineas de arriba: las luces quedan en rojo ANTES de que se toque la LCD. Y
  // u8g2.begin() sobre este ST7920 va por SW SPI, que solo ESCRIBE: nunca espera
  // respuesta del modulo, asi que una pantalla ausente no puede colgar el arranque.
  // Por eso puede ir incluso antes del watchdog, igual que en el Maestro.
  //
  // botones_setup() la acompana porque solo declara pines y siembra su estado (N-26).
  botones_setup();
  lcd_setup();
  lcd_dibujarBienvenida();

  // N-22: los 2 s que el Maestro SI tenia y esta punta no. Era la ultima diferencia
  // que quedaba entre las dos en todo el camino de la pantalla; el constructor de
  // U8G2, lcd_setup() y lcd_dibujarBienvenida() son identicos hasta la coma.
  //
  // En el Maestro esta desde el principio, con el motivo escrito al lado del watchdog:
  // "despues del delay inicial para evitar loop de reinicio". Va ANTES de armar el
  // IWDG a proposito, que es cuando un delay largo no cuesta nada.
  //
  // NO SE AFIRMA QUE ESTO ARREGLE LA PANTALLA AZUL, y conviene dejarlo dicho: la
  // version anterior tenia lcd_setup() en ULTIMA posicion -o sea, el maximo tiempo
  // posible de reposo tras el encendido antes de tocar el modulo- y tambien salia en
  // blanco. Si el extremo de esperar mucho fallaba igual que el de arrancar pronto,
  // el momento de la inicializacion no es la variable.
  //
  // Se iguala de todas formas por dos razones. Una, cuesta 2 s en el arranque y nada
  // mas. Dos, y es la que importa: deja las dos puntas IDENTICAS en el camino de la
  // pantalla, de modo que si esta sigue azul y en blanco el software queda descartado
  // por completo y la busqueda se puede llevar entera al hardware -VDD del modulo,
  // PSB en PB6, RST en PB7 y el arnes de cables- sin la duda de si quedaba algo por
  // igualar en el firmware.
  delay(2000);

  // SFTY-1: Iniciar Watchdog Timer a 4 segundos
  IWatchdog.begin(4000000);

  // SFTY-18 / SFTY-23: arranca el RTC sobre el cristal Y2 sin borrar la hora
  // guardada. Si la pila esta puesta y el Maestro ya lo sincronizo alguna vez, la
  // hora sobrevivio al apagado y no hay que repetir el proceso.
  //
  // VA DESPUES DEL WATCHDOG A PROPOSITO. rtc.begin() enciende el oscilador LSE, y
  // MAPEO_TARJETA_KICAD.md seccion 4 documenta que en microcontroladores clonados
  // el cristal de 32.768 kHz a veces NO arranca. Si el HAL se quedara esperandolo
  // con el watchdog aun sin armar, la controladora se congelaria con las luces
  // apagadas y sin nada que la rescatase. Armado antes, ese fallo se convierte en
  // un reinicio visible y diagnosticable en vez de un cuelgue mudo.
  //
  // Un semaforo no puede depender de un cristal de reloj para encender.
  //
  // Aqui el argumento pesa aun mas que en el Maestro: el Esclavo no tiene pantalla
  // en la que se vea que algo va mal. Colgado, seria un poste apagado en mitad de
  // la obra sin nadie delante que lo note.
  //
  // PENDIENTE DE BANCO (N-17): probar el arranque con Y2 desconectado y comprobar
  // que el equipo bootea igual, con reloj_enHora() en false.
  reloj_setup();

  // N-20: memoria que sobrevive al corte. VA DESPUES DE reloj_setup() a proposito:
  // comparte con el RTC el dominio de respaldo y la pila CR2032, y aunque
  // respaldo_setup() vuelve a habilitar los relojes de PWR y BKP por su cuenta -para
  // no depender del orden-, leer la firma despues de que el RTC haya terminado de
  // arrancar evita interpretar como corrupcion un dominio que aun se estaba
  // inicializando.
  respaldo_setup();

  // SFTY-21 / N-19: mando de reles. Solo pone a cero sus contadores; los pulsos los
  // vera en botones_actualizar(), asi que no importa que el receptor todavia no este
  // instalado.
  mando_setup();

  // Entradas Digitales de Cámaras IA AcuSense (Contacto Seco N/O)
  // N-67: LA POLARIDAD LA MANDA LA PLACA, Y LA PLACA DICE ACTIVO EN ALTO.
  //
  // Medido sobre el esquematico bueno: PB0 lleva R64 de 10 kOhm A MASA -pull-DOWN- y
  // C25 de 100 nF tambien a masa; la bornera J14 saca ese pin JUNTO A 3,3 V. O sea que
  // el contacto seco de la camara va entre los dos bornes de J14 y CIERRA A 3,3 V.
  //
  // Con INPUT_PULLUP esto no podia funcionar de ninguna manera: el pull-up interno
  // (~40 kOhm) contra los 10 kOhm externos deja el pin en 3,3 x 10/50 = 0,66 V, que es
  // LOW. El firmware habria visto DEMANDA PERMANENTE desde el arranque, sin camara
  // conectada; y al cerrar el contacto el pin sube a 3,3 V y lo habria leido como
  // "no hay demanda". Invertido y siempre activo a la vez.
  //
  // Se deja en INPUT a secas -sin pull-up-: el reposo lo fija el 10k de la placa.
  pinMode(CAM_DEMANDA_PIN, INPUT);
  // N-64: PB8 no es una entrada de camara, es el LED testigo D5 (R16 1K). Se deja en
  // alta impedancia: un pin flotante no puede encenderlo sea cual sea el sentido del
  // diodo, y con INPUT_PULLUP quedaria a medio encender por 40 uA de fuga.
  pinMode(LED_TESTIGO, INPUT);

  // Modulo de radio corta en USART1 REMAPEADO (PB6 TX / PB7 RX a 9600 bps).
  // Sale por el conector J17 -p3 y p2-, que es enchufable; PA9/PA10 no llegan
  // a ninguna bornera. Ver el porque en bluetooth.cpp.
  bluetooth_setup();

  // ---------------------------------------------------------------------------
  // N-20: REANUDAR EL MODO DEGRADADO TRAS UN CORTE.
  //
  // Va aqui, antes de la interfaz y antes de la primera vuelta del bucle, para que
  // el equipo no llegue a mostrar un estado que va a cambiar acto seguido: si esto
  // se hiciera en el loop, el cruce pasaria por su arranque normal -rojo y, doce
  // segundos despues, ambar- antes de reanudar, y ese ambar intermedio es
  // precisamente la asimetria que N-20 viene a evitar.
  //
  // Todas las condiciones y el borrado del indicador cuando alguna falla viven en
  // degradado_reanudarTrasCorte(); aqui no se decide nada. Si devuelve false, el
  // arranque sigue siendo el de siempre y no hay nada mas que hacer.
  degradado_reanudarTrasCorte();

  tArranque = millis();
}

void loop() {
  // SFTY-1: Watchdog reload
  IWatchdog.reload();

  // N-25: reintento en segundo plano del cristal del reloj. Va al principio del bucle
  // y no cuesta nada -sale por la primera linea si el RTC ya cuenta, y si no, lee un
  // flag cada 30 s-. Si el oscilador arranco tarde, el equipo lo ADOPTA aqui sin
  // reiniciar: el arranque no puede esperarlo (N-17), pero condenarlo tras 2 s seria
  // confundir un cristal lento con uno muerto.
  reloj_actualizar();


  // SFTY-21: los cuatro flancos se detectan AQUI, una sola vez y antes que nada.
  // Dentro, botones.cpp le ensena los pulsos de A y B al mando sin consumirlos.
  //
  // Tiene que ir antes de menu_loop() y antes de cualquier cosa que lea un boton: si
  // la deteccion siguiera repartida dentro de cada botonX(), las pulsaciones que
  // ninguna pantalla consulta no existirian para nadie, y el operario del piso habria
  // pulsado tres veces sin que el equipo contase ninguna.
  botones_actualizar();

  // Telemetría periódica y comandos por Bluetooth en USART1
  bluetooth_loop();

  // Camara 3 (PB0): el FLANCO es lo que pide paso, no el nivel.
  //
  // El rele de la camara mantiene el contacto cerrado ~1 s por deteccion; leer el nivel
  // repetiria la peticion en cada vuelta del loop durante todo ese segundo. La ventana
  // de silencio y el envio viven en demanda_solicitar(), que es la unica
  // puerta: por ahi entra tambien el boton de la app, y asi los dos origenes comparten
  // el mismo limite de ritmo en vez de llevar cada uno el suyo.
  static bool demandaCamaraAnt = false;
  // N-67: activo en ALTO. El contacto de la camara cierra a 3,3 V contra el
  // pull-down de 10k de la placa; leer LOW daria demanda continua sin camara.
  bool demandaCamaraActual = (digitalRead(CAM_DEMANDA_PIN) == HIGH);
  if (demandaCamaraActual && !demandaCamaraAnt) {
    demanda_solicitar();
  }
  demandaCamaraAnt = demandaCamaraActual;

  semaforo_actualizar();
  atenderRespuestaPendiente();   // SFTY-17
  caducarBufferHora();           // SFTY-23: un envio a medias no envejece en RAM

  // SFTY-21: el ciclo por reloj y el limite duro de 48 h corren en cada vuelta,
  // haya o no alguien mirando la pantalla. Con el modo inactivo esto no toca
  // nada; su unico efecto es mantener viva la cuenta atras del limite.
  degradado_actualizar();

  RF_Packet pkt;
  static unsigned long tUltimoComando = millis();
  static bool ackRojoEnviado = false, ackVerdeEnviado = false;

  if (protocolo_hayPaqueteDisponible(&pkt)) {
    // SFTY-21: SI VUELVE EL RADIO, EL MAESTRO MANDA.
    //
    // El Modo Degradado existe para cubrir la ausencia del Maestro; en cuanto el
    // Maestro vuelve a gobernar, sobra, y mantenerlo seria tener dos autoridades
    // decidiendo la misma luz. Se sale por la via ordenada -todo-rojo primero-,
    // no apagando el modo en seco.
    //
    // Solo cuentan las tramas de GOBIERNO. Las de servicio (hora, configuracion,
    // delta) NO sacan del Degradado, por el mismo motivo por el que no refrescan
    // tUltimoComando: significan "hay portadora en el aire", no "el Maestro esta
    // gobernando el cruce". Un Maestro colgado en su pantalla de hora mandaria
    // sincronizaciones sin ordenar una sola luz, y salir del Degradado por eso
    // dejaria el cruce en ambar creyendo que alguien lo controla.
    if (degradado_gobiernaLuz() &&
        (pkt.command == CMD_PING || pkt.command == CMD_GO_RED || pkt.command == CMD_GO_GREEN)) {
      degradado_salir();
    }

    // Si recibimos un PING durante estado de fallo local, no resincronizar timer para permitir timeout si el maestro cayó
    if (pkt.command == CMD_PING) {
      if (semaforo_estado() != S_FALLO) {
        tUltimoComando = millis();
      }
      programarRespuesta(CMD_PONG); // SFTY-17: se responde tras el retardo de cortesia
    } else if (pkt.command == CMD_GO_RED) {
      tUltimoComando = millis();
      // SFTY-21: con el ambar pedido desde el mando (B.B.B) no se obedece NI SE
      // ACUSA RECIBO. Ver mando.h: acusar sin encender la luz dejaria al Maestro
      // dando verde a su lado convencido de que aqui hay rojo. Callando, agota sus
      // reintentos, cae a C_FALLO en ~12,5 s y el cruce entero termina en ambar,
      // que es lo que el operario pidio.
      if (!mando_ambarLocal()) {
        semaforo_forzarRojo(); // Directo a rojo
        ackRojoEnviado = true;
        programarRespuesta(CMD_ACK_RED);
      }
    } else if (pkt.command == CMD_GO_GREEN) {
      tUltimoComando = millis();
      if (!mando_ambarLocal()) {
        semaforo_iniciarTransicionAVerde(); // Transición Rojo -> Amarillo -> Verde
        // El backstop de verde maximo ya no se rearma aqui: lo hace el vigilante
        // del final del bucle, que mira la LUZ en vez de la orden. Ver alli el
        // motivo -el verde del Modo Degradado no lo ordena nadie por radio-.
        ackVerdeEnviado = false;
        programarRespuesta(CMD_ACK_GREEN);
      }

    // --- SFTY-23: sincronizacion horaria y configuracion del ciclo -----------
    // Ninguna de estas ramas toca las luces ni refresca tUltimoComando (ver el
    // razonamiento en el bloque de arriba).
    } else if (pkt.command == CMD_HORA_D) {
      // El DIA abre el envio (ver protocolo.h). Se aprovecha eso para ABRIR TAMBIEN
      // EL BUFFER: lo que quedase de un intento anterior es basura de otra foto del
      // reloj, y mezclarla con las cifras que vienen ahora daria una hora que el
      // Maestro nunca envio. Con el envio entero repitiendose en cada reintento, no
      // se pierde nada tirandolo.
      //
      // Solo 1..31. Un 0 no es un dia y por radio significaria "no toques la fecha",
      // que es justo lo que este mensaje existe para NO hacer.
      if (pkt.param >= 1 && pkt.param <= 31) {
        olvidarBufferHora();
        bufDia = pkt.param;
        tieneDia = true;
        tBufHora = millis();   // la ventana de validez cuenta desde la primera cifra
      }
    } else if (pkt.command == CMD_HORA_H) {
      if (pkt.param <= 23) {
        bufHora = pkt.param;
        tieneHora = true;
        // La marca de tiempo la pone la PRIMERA cifra que llega del envio. Refrescarla
        // en cada trama alargaria la ventana trama a trama y la foto podria envejecer
        // mas de los 3 s que esto acota. Si el dia se perdio, la hora hace de primera.
        if (!tieneDia) tBufHora = millis();
      }
    } else if (pkt.command == CMD_HORA_M) {
      if (pkt.param <= 59) {
        bufMinuto = pkt.param;
        tieneMinuto = true;
        if (!tieneDia && !tieneHora) tBufHora = millis();
      }
    } else if (pkt.command == CMD_HORA_S) {
      // Aqui, y solo aqui, se escribe el RTC, con las cuatro cifras a la vez.
      // Si falta cualquiera de las tres anteriores -el DIA incluido- se DESCARTA el
      // envio entero y no se contesta: sin ACK el Maestro repetira, y repetir cuesta
      // 3,5 s mientras que dejar el reloj con una hora inventada, o con la hora buena
      // y el calendario propio, no se detecta nunca. El silencio es la respuesta
      // correcta a una orden incompleta.
      if (tieneDia && tieneHora && tieneMinuto && pkt.param <= 59) {
        reloj_ajustar(bufHora, bufMinuto, pkt.param, bufDia);

        // SFTY-21: aqui, y solo aqui, se reinicia la cuenta del limite duro de
        // 48 h. Es el unico momento en que este nodo puede afirmar que su reloj
        // vuelve a coincidir con el del Maestro; oir tramas no lo demuestra, y
        // apoyar el limite en el trafico convertiria "sigo en hora" en "sigo
        // oyendo ruido", que es exactamente la confusion que el limite evita.
        degradado_registrarSync();

        // N-20: la misma marca, pero en la pila. Se guarda el INSTANTE y no un
        // contador, porque un contador de milisegundos se pierde al reiniciar, que
        // es justo el caso que esto viene a cubrir. Va DESPUES de reloj_ajustar()
        // para que reloj_dia() y reloj_segundosDelDia() devuelvan ya la hora nueva:
        // sellar la marca con la hora vieja falsearia su antiguedad, y sobre esa
        // antiguedad se decide luego si se puede reanudar el Modo Degradado.
        //
        // Desde CMD_HORA_D ese reloj_dia() es el DIA DEL MAESTRO, no uno sembrado
        // aqui: las dos puntas sellan su marca con el mismo numero y la antiguedad
        // les caduca a la vez.
        respaldo_marcarSync(reloj_contadorSegundos());

        programarRespuesta(CMD_ACK_HORA);   // SFTY-17: sale tras el retardo de cortesia
      }
      // Se vacia el buffer se haya aplicado o no, y se vacian LAS CUATRO cifras
      // juntas. Un envio solo sirve UNA vez: si el ACK se perdiera y el Maestro
      // reenviara los segundos a secas, aplicarlos sobre un dia, una hora y un minuto
      // ya viejos meteria el error de un minuto entero en el momento justo de un
      // cambio de minuto -y de un dia entero en el de medianoche-. Sin ACK, el
      // Maestro repite el envio completo y vuelve a estar todo fresco.
      olvidarBufferHora();

    } else if (pkt.command == CMD_DELTA) {
      // Se responde SIEMPRE, incluso sin hora fiable: en ese caso el valor es
      // DELTA_FUERA_DE_RANGO. Callar dejaria al Maestro sin distinguir "el Esclavo
      // no esta en hora" de "el Esclavo no me oye", que son dos averias distintas.
      int8_t d = calcularDesfase(pkt.param);
      programarRespuesta(CMD_DELTA_RESP, (uint8_t)d);   // complemento a dos en el byte

    } else if (pkt.command == CMD_CONFIG_VERDE) {
      // NO se acusa aqui. El Maestro envia VERDE y DESPEJE seguidas y espera UN
      // solo ACK del par, asi que confirmar las dos por separado le devolveria un
      // ACK sobrante.
      //
      // Antes funcionaba, pero POR ACCIDENTE: al caber una sola respuesta
      // pendiente, la de DESPEJE pisaba a la de VERDE dentro de la ventana de
      // cortesia de 200 ms y salia una sola. Bastaba con que las dos tramas se
      // separasen -retransmision, espaciado de rafaga, buferado del repetidor-
      // para que salieran dos. Depender de esa carrera no es un diseno.
      //
      // Se confirma solo con la ULTIMA del par, igual que la hora se aplica solo
      // al llegar la trama de segundos.
      config_rxVerde(pkt.param);
    } else if (pkt.command == CMD_CONFIG_DESPEJE) {
      // El par solo se cierra si el VERDE es de ESTE envio; config_rxDespeje() lo
      // decide y devuelve si hay que acusarlo. Cuando NO se cierra, el silencio es
      // deliberado: es lo que provoca el reintento del Maestro.
      if (config_rxDespeje(pkt.param)) {
        programarRespuesta(CMD_ACK_CONFIG);   // cierra el par
      }
    } else if (pkt.command == CMD_ACK_DEMANDA) {
      // Acuse de recibo de demanda recibido desde el Maestro
    }


    // Si recuperamos conexion tras un fallo.
    // SFTY-21: no con el ambar pedido desde el mando. Ese ambar no es una perdida de
    // enlace de la que haya que recuperarse, es una orden vigente del operario, y
    // solo A.A.A la revoca.
    if (!mando_ambarLocal() && semaforo_estado() == S_FALLO && pkt.command == CMD_GO_RED) {
      semaforo_forzarRojo();
      protocolo_resetReplayProtection();
    }
  }

  // Fallback unificado si no hay comunicacion del maestro en SFTY6_SILENCIO_MS
  //
  // SFTY-21: suspendido mientras el Modo Degradado gobierna la luz. No es una
  // excepcion al estado seguro, es el mismo estado seguro por otra via: el ambar
  // se da porque nadie decide, y aqui decide el reloj con la configuracion que el
  // Maestro dejo, verificada por un operario en las dos puntas. Sin esta guarda
  // los dos mecanismos se pisarian doce segundos despues de entrar, y el ambar
  // ganaria siempre porque el silencio del radio es permanente.
  if (!degradado_gobiernaLuz() && millis() - tUltimoComando > SFTY6_SILENCIO_MS) {
    if (semaforo_estado() != S_FALLO) {
      semaforo_iniciarFallo();
      protocolo_resetReplayProtection();
      // N-73: la Caja Negra existia y NO LA LLAMABA NADIE.
      //
      // bluetooth_reportarAlarma() estaba declarada, definida y documentada en las
      // dos puntas -y anunciada en cuatro manuales como "registro inmediato de
      // eventos para diagnosticar la causa exacta de cualquier caida de radio en
      // obra"- sin un solo sitio que la invocara. Es la misma forma que N-63: un
      // pinMode() sin digitalRead(), pero con documentacion encima.
      //
      // Se conecta AQUI porque este es el instante que el tecnico ve y no puede
      // fechar: el momento exacto en que la luz se va a ambar. El reporte de campo
      // del 27/08 -"se va a degradado cada nada cuando llueve"- no se pudo
      // diagnosticar por esto: no habia registro que mirar.
      //
      // El nombre del evento NO lleva el numero dentro. El ejemplo del header decia
      // "FALLO_RF_12S", y ese literal habria quedado mintiendo el dia que el umbral
      // paso a 25 s (N-71). El umbral va en la causa, no en el nombre.
      char causa[40];
      snprintf(causa, sizeof(causa), "SILENCIO_%lums", SFTY6_SILENCIO_MS);
      bluetooth_reportarAlarma("FALLO_RF", causa, "CAMBIO_A_AMBAR");
    }
  }

  // Rearme del backstop: se cuenta desde que la LUZ arranca hacia verde, sin
  // mirar quien lo pidio.
  //
  // Antes se anotaba al recibir CMD_GO_GREEN, y eso deja de valer con el Modo
  // Degradado: alli el verde no lo ordena nadie por radio. Un backstop anclado a
  // la ultima orden recibida estaria midiendo un verde que empezo horas antes y
  // cortaria el primer verde por reloj a los pocos milisegundos de encenderlo.
  //
  // Ademas es mas estricto que la version anterior: ordenes de verde repetidas ya
  // no reinician la cuenta, asi que el limite mide el verde de verdad y no el
  // tiempo desde la ultima trama.
  {
    static EstadoSemaforo estadoLuzAnt = S_ROJO;
    EstadoSemaforo luzAhora = semaforo_estado();
    if (luzAhora != estadoLuzAnt) {
      if (luzAhora == S_AMARILLO || luzAhora == S_VERDE) tInicioVerdeEsclavo = millis();
      estadoLuzAnt = luzAhora;
    }
  }

  // FIX H-1: Guardia de Verde Máximo de Seguridad (backstop de último recurso).
  // CORRECCIÓN DE REGRESIÓN: estaba en 180000 (3 min), pero modo_automatico.cpp permite
  // configurar hasta 99 minutos de verde. Con verdes > 3 min el Esclavo cortaba a Rojo por su
  // cuenta, sin avisar al Maestro (el auto-ACK_RED se eliminó en H-6), dejando su carril
  // cerrado el resto del ciclo mientras el Maestro lo seguía contando como verde.
  // Se fija por encima del máximo configurable (99 min) para que nunca actúe sobre una
  // configuración legítima. La protección real de H-1 son las otras dos vías: el Maestro
  // emite CMD_GO_RED estando en C_FALLO, y este nodo cae a ámbar a los 12s sin recibir nada.
  if (semaforo_estado() == S_VERDE && (millis() - tInicioVerdeEsclavo > MAX_VERDE_BACKSTOP_MS)) {
    semaforo_forzarRojo();
  }

  // SFTY-17: el ACK de verde se emite cuando la luz ya esta estable en verde.
  //
  // Se calla mientras manda el Modo Degradado. Ese verde no lo pidio el Maestro,
  // asi que confirmarselo seria contestar a una pregunta que nadie hizo: si el
  // radio reapareciera en mitad de un verde por reloj, el Maestro recibiria el
  // ACK de una orden que no dio y podria darla por cumplida.
  if (!degradado_gobiernaLuz() &&
      semaforo_estable() && semaforo_estado() == S_VERDE && !ackVerdeEnviado) {
    programarRespuesta(CMD_ACK_GREEN);   // SFTY-17
    ackVerdeEnviado = true; ackRojoEnviado = false;
  }

  // N-16: interfaz, siempre al final del bucle.
  //
  // El volcado del framebuffer a la ST7920 por SPI de software cuesta decenas de
  // milisegundos, asi que va DESPUES de atender la radio y las luces: si alguna
  // vuelta se alarga, lo que se retrasa es el repintado de una pantalla y no una
  // orden de semaforo. El repintado periodico esta acotado a una vez por segundo
  // dentro de menu.cpp, muy por debajo de los 4 s del watchdog.
  if (!interfazArrancada) {
    // La bienvenida se retira por tiempo transcurrido, sin bloquear: durante esos
    // 1,5 s el equipo ya esta escuchando la radio y obedeciendo ordenes.
    if (millis() - tArranque >= BIENVENIDA_MS) {
      interfazArrancada = true;
      menu_setup();
    }
  } else {
    menu_loop();
  }

  // SFTY-21: el mando va AL FINAL, y por dos motivos.
  //
  // Uno: la accion se ejecuta cuando los destellos de confirmacion han terminado, y
  // quien los hace avanzar es semaforo_actualizar(), al principio de esta misma
  // vuelta. Consultarlo aqui hace que la accion salga en la iteracion en la que la
  // senal se apaga, sin un ciclo de espera de por medio.
  //
  // Dos: cualquier cosa que el mando cambie -salir del Degradado, encender el ambar-
  // queda como ultima palabra de la vuelta, sin que la logica de radio de mas arriba
  // la pise antes de llegar a los pines.
  mando_actualizar();
}


