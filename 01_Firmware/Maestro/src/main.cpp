// ===== src/main.cpp (MAESTRO) =====
#include <Arduino.h>
#include "pines.h"
#include "botones.h"
#include "coordinador.h"
#include "lcd.h"
#include "menu.h"
#include "modos.h"
#include "modo_manual.h"
#include "modo_automatico.h"
#include "modo_inteligente.h"
#include "modo_alcance.h"
#include "modo_hora.h"
#include "modo_degradado.h"
#include "modo_ambar.h"
#include "mando.h"
#include "reloj.h"
#include "respaldo.h"
#include "semaforo.h"
#include "bluetooth.h"
#include <IWatchdog.h> // SFTY-1: Watchdog Timer

// FASE 1 DEL PLAN DE ARQUITECTURA: aqui vivia el ultimo resto de la vieja maquina de
// estados HANDSHAKE/SISTEMA -el enum, estadoGlobal, tBlink, amarilloOn y
// iniciarParpadeoFallo()-. Todo ello estaba MUERTO: la funcion era static y no la
// llamaba nadie, tBlink y amarilloOn solo se escribian dentro de ella, y estadoGlobal
// se asignaba sin que nadie lo leyera nunca.
//
// No se borra por prolijidad. iniciarParpadeoFallo() escribia ROJO1/ROJO2/VERDE1/
// VERDE2 con digitalWrite() DIRECTO, y era la unica escritura a pines de luz fuera de
// semaforo.cpp en todo el firmware: la unica fuga de la barrera de salidas. Cerrarla
// permite exigir la regla completa, que ahora vigila el validador:
//
//   NINGUN pin de luz se escribe fuera de semaforo.cpp. Los OCHO, incluidos
//   ROJO_PEATON y VERDE_PEATON, que estaban sin custodia.
//
// La barrera ya existia -todo pasa por escribirPines(), y los destellos del mando
// INTERCEPTAN las escrituras en vez de rodearlas-. Lo que faltaba era poder decir que
// no tiene excepciones, y poder demostrarlo.

static ModoSistema modoAnterior;

void setup() {
  botones_setup();
  coordinador_setup();
  lcd_setup();

  // Inicializar UI
  lcd_dibujarBienvenida();
  delay(2000);

  // SFTY-1: Iniciar Watchdog Timer a 4 segundos (despues del delay inicial para evitar loop de reinicio)
  IWatchdog.begin(4000000);

  // SFTY-18: arranca el RTC sobre el cristal Y2 sin borrar la hora guardada. Si la
  // pila esta puesta y ya se ajusto alguna vez, la hora sobrevivio al apagado.
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
  // PENDIENTE DE BANCO (N-17): probar el arranque con Y2 desconectado y comprobar
  // que el equipo bootea igual, con reloj_enHora() en false.
  reloj_setup();

  // N-20: DESPUES de reloj_setup(), y el orden no es estetico. Las dos cosas viven en
  // el mismo dominio alimentado por la pila CR2032, y respaldo_setup() valida la firma
  // y la suma de comprobacion de unos registros cuyo reloj de periferico enciende
  // aquel. Al reves, sobre un dominio a medio arrancar, un contenido bueno podria
  // leerse como corrupto y borrarse: se perderia la marca de sincronizacion justo en
  // el arranque que viene a rescatarla.
  respaldo_setup();

  // SFTY-21: el mando de reles queda armado desde el arranque. No depende de ningún
  // modo: es la única interfaz que el operario tiene desde el suelo.
  mando_setup();

  // Modulo de radio corta en USART1 REMAPEADO (PB6 TX / PB7 RX a 9600 bps).
  // Sale por el conector J17 -p3 y p2-, que es enchufable; PA9/PA10 no llegan
  // a ninguna bornera. Ver el porque en bluetooth.cpp.
  bluetooth_setup();

  // N-20: la decision de reanudar se consulta AQUI, ANTES de publicar la
  // configuracion, y el orden es de fondo. modo_degradado_publicarConfig() guarda el
  // ciclo en la pila, asi que despues de llamarla respaldo_hayCiclo() seria cierto
  // SIEMPRE y esa condicion dejaria de comprobar nada. Consultada antes, dice lo que
  // de verdad interesa: si esta unidad llego a acordar un ciclo con la otra punta
  // ANTES del corte. La respuesta se guarda y se aplica unas lineas mas abajo, ya con
  // la configuracion publicada.
  const bool reanudarDegradado = modo_degradado_reanudarTrasCorte();

  // SFTY-23: se encola la configuración del ciclo degradado AHORA, mientras el enlace
  // vive. Queda pendiente hasta que el Esclavo la confirme, con los reintentos que ya
  // lleva el coordinador. Dejarlo para el momento de entrar en Degradado sería tarde:
  // para entonces el radio puede estar muerto, y sin acordar el ciclo las dos puntas
  // computarían horarios distintos sobre la misma hora.
  modo_degradado_publicarConfig();

  if (reanudarDegradado) {
    // N-20: el equipo estaba en Modo Degradado cuando se fue la luz y la autorizacion
    // sigue vigente, asi que VUELVE a el en vez de arrancar en el menu.
    //
    // Arrancar en el menu dejaria esta punta en rojo fijo o en ambar mientras la otra
    // -que quiza no se reinicio- sigue dando verde por reloj: una punta en AMBAR, que
    // el conductor negocia, contra otra en VERDE, que el conductor cruza confiado. Ese
    // es el riesgo residual n.2 de SFTY-21, y caer a ambar es precisamente lo que lo
    // crea. Reanudar en fase lo evita.
    //
    // No se salta a verde: modo_degradado_setup() arranca en todo-rojo igual que en la
    // entrada normal.
    modoActual_set(MODO_DEGRADADO);
    modo_degradado_setup();
    modoAnterior = MODO_DEGRADADO;
  } else {
    // Eliminamos el bloqueo inicial por Handshake.
    // Arrancamos directamente en el menú.
    modoActual_set(MENU);
    menu_setup();
    modoAnterior = MENU;
  }
}

void loop() {
  // SFTY-1: Alimentar al perro guardián. Si el loop se traba por > 4s, la placa se reinicia.
  IWatchdog.reload();

  // N-25: reintento en segundo plano del cristal del reloj. Va al principio del bucle
  // y no cuesta nada -sale por la primera linea si el RTC ya cuenta, y si no, lee un
  // flag cada 30 s-. Si el oscilador arranco tarde, el equipo lo ADOPTA aqui sin
  // reiniciar: el arranque no puede esperarlo (N-17), pero condenarlo tras 2 s seria
  // confundir un cristal lento con uno muerto.
  reloj_actualizar();


  // SFTY-21: los flancos de los cuatro botones se detectan aquí, UNA vez por
  // iteración y antes de atender ningún modo. El mando de reles los ve desde dentro de
  // botones.cpp, de modo que una secuencia desde el suelo se reconoce esté el equipo
  // en la pantalla que esté, y no solo en las que casualmente leen ese botón.
  botones_actualizar();

  // Telemetría periódica y comandos por Bluetooth en USART1
  bluetooth_loop();

  // La maquina de luces se refresca SIEMPRE, en todos los modos y sin excepcion.
  //
  // Antes dependia de que alguien llamase a coordinador_actualizar(), y habia
  // caminos donde nadie lo hacia: main.cpp excluye MODO_AUTOMATICO del refresco de
  // fondo y modo_automatico.cpp solo llama al coordinador en su estado CORRIENDO,
  // de modo que en las pantallas de configuracion del asistente la maquina quedaba
  // congelada.
  //
  // La consecuencia la encontro el validador el 01/08/2026 y era grave: una
  // secuencia del mando desde el suelo apaga las seis salidas para arrancar los
  // destellos de confirmacion, y sin nadie que avance la senal el cabezal se
  // quedaba A OSCURAS INDEFINIDAMENTE. Peor aun, el mando quedaba sordo -se ignora
  // todo pulso mientras hay senal en curso-, asi que desde el suelo no habia forma
  // de recuperarlo: habia que subir al gabinete.
  //
  // Un semaforo apagado no avisa de nada. Que las luces dependan de que un modo se
  // acuerde de refrescarlas es una dependencia que no debe existir: es una llamada
  // barata, idempotente -avanza por millis()- y aqui cubre todos los caminos.
  semaforo_actualizar();

  // El coordinador maneja su propia auto-recuperación (SFTY-9)
  // No necesitamos reiniciar su estado desde aquí, ya que eso rompe el C_FALLO.

  // Siempre actualizamos el estado del coordinador para mantener vivos los PINGs/PONGs
  // y las secuencias de semáforo (si no estamos en manual).
  // Nota: En modo_automatico.cpp ya se llama a coordinador_actualizar(), 
  // pero para que el handshake en background funcione, lo llamamos aquí si no estamos en auto.
  //
  // SFTY-21: el Degradado y el Ámbar de emergencia quedan FUERA de esta llamada, y no
  // por ahorro: en esos dos modos el Maestro CALLA en la radio a propósito.
  //   - El Degradado se define por no tener radio. Seguir emitiendo abriría la puerta
  //     a que una orden vieja contradijese la fase calculada por reloj.
  //   - Y si el otro extremo siguiera en modo normal, dejar de hablarle lo manda a
  //     ambar por orfandad (SFTY-6), que es la direccion segura.
  // OJO: LA FRASE QUE SEGUIA AQUI DESCRIBIA UN FIRMWARE ANTERIOR (corregido 04/09).
  // Decia que estos modos llaman a semaforo_actualizar() por su cuenta "que es lo que
  // aqui se perderia". Medido: la linea 167 de este mismo fichero ya la llama SIN
  // condicion, asi que no se perderia nada. La exclusion sigue siendo correcta, pero
  // por el OTRO motivo -callar en la radio a proposito-, que es el de arriba. Se anota
  // porque una cuenta dentro de un comentario envejece con la autoridad de un dato.
  // Lo encontro maestro_10_coordinador_alcanzable.
  // Los dos modos llaman a semaforo_actualizar() por su cuenta, que es lo que aquí se
  // perdería.
  ModoSistema modo = modoActual_get();
  if (modo != MODO_AUTOMATICO && modo != MODO_DEGRADADO && modo != MODO_AMBAR) {
     coordinador_actualizar_background();
  }

  if (modo != modoAnterior) {
    // N-20: SALIR del Degradado por CUALQUIER via borra el indicador de la pila, y por
    // eso se hace aqui y no dentro de cada destino. Del Degradado se sale por al menos
    // cuatro caminos -boton 4, A.A.A a Automatico, B.B.B a Ambar, y el limite de 48 h-,
    // y basta olvidar uno para que el equipo reanude despues un modo del que ya habia
    // salido. Este punto es el unico por el que pasan todos: si el modo cambio, el
    // Degradado se acabo. Un borrado de mas solo cuesta una escritura de 16 bits; un
    // borrado de menos cuesta un verde sin confirmar.
    if (modoAnterior == MODO_DEGRADADO) {
      respaldo_guardarDegradado(false);
    }

    switch (modo) {
      case MODO_MANUAL:      modoManual_setup();      break;
      case MODO_AUTOMATICO:  modoAutomatico_setup();  break;
      case MODO_INTELIGENTE: modoInteligente_setup(); break;
      case MODO_ALCANCE:     modoAlcance_setup();     break;
      case MODO_HORA:        modo_hora_setup();       break;
      case MODO_DEGRADADO:   modo_degradado_setup();  break;
      case MODO_AMBAR:       modo_ambar_setup();      break;
      case MENU:             menu_setup();            break;
    }
    modoAnterior = modo;
  }

  switch (modo) {
    case MENU:            menu_loop();            break;
    case MODO_MANUAL:     modoManual_loop();      break;
    case MODO_AUTOMATICO: modoAutomatico_loop();  break;
    case MODO_INTELIGENTE:modoInteligente_loop(); break;
    case MODO_ALCANCE:    modoAlcance_loop();     break;
    case MODO_HORA:       modo_hora_loop();       break;
    case MODO_DEGRADADO:  modo_degradado_loop();  break;
    case MODO_AMBAR:      modo_ambar_loop();      break;
  }

  // SFTY-21: al FINAL. La acción del mando (cambio de modo) se aplica cuando la
  // confirmación de destellos ha terminado, nunca a mitad: primero se confirma, luego
  // se actúa. Ponerlo aquí también garantiza que el cambio de modo lo recoja la
  // siguiente iteración por el camino normal, con su setup(), y no a mitad del loop
  // de un modo que ya no es el activo.
  mando_actualizar();
}

