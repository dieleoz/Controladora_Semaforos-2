// ===== src/modo_inteligente.cpp =====
#include "modo_inteligente.h"
#include "pines.h"
#include "botones.h"
#include "semaforo.h"
#include "coordinador.h"
#include "demanda.h"
#include "lcd.h"
#include "menu.h"
#include "modos.h"
#include "protocolo.h"
#include <string.h>
#include "limites_ciclo.h"   // N-137: el minimo vial, no un 2 escrito a mano
#include "modo_automatico.h" // A-12: los tiempos CONFIGURADOS se leen, no se copian

// N-135 OTRA VEZ, EN EL FICHERO DE AL LADO (05/09). Aqui habia:
//
//     enum FaseInt { INT_CORRIENDO };
//     static FaseInt faseI;      ... switch (faseI) { case INT_CORRIENDO: ... }
//
// Un enum de UN SOLO valor no es un estado: es una constante disfrazada, y el switch de
// abajo era un envoltorio que no podia elegir nada. Inerte hoy -no colgaba ninguna guarda
// de el, al contrario que en modo_automatico.cpp, donde costo SET_TIEMPOS entero- pero es
// la misma forma, y se retira antes de que alguien cuelgue una guarda.
//
// Y LO ENCONTRO UNA REVISION EXTERNA, NO EL PACK QUE EXISTE PARA ESTO: maestro_10 censa
// enums de un solo valor "que ademas se COMPARAN" y solo miraba `==`. Un `case` es una
// comparacion.
//
// 🔴 Y AQUI DECIA "el pack se afila en el mismo commit; si no, esto vuelve". NO SE AFILO,
// y la frase se quedo prometiendolo: es exactamente 2.ter -una frase que sostiene un verde
// y que no comprueba nadie-, escrita por quien hizo el cambio y en el mismo commit. La
// encontro una revision externa leyendo, no un test.
//
// Lo que paso, medido: se anadio `case` al detector y se INYECTO el defecto en este .cpp
// para comprobarlo. EL PACK SIGUIO DANDO 12/12. El censo aislado si lo encuentra -el enum
// se detecta y el `case` tambien-, asi que hay una tercera causa sin localizar. Un parche
// que no se ha visto fallar es un adorno que da verde (8.bis), asi que se revirtio.
//
// QUEDA ABIERTO: maestro_10 es el UNICO pack que censa esta forma, y hoy solo mira `==`.
// Un enum de un valor dentro de un switch volveria a entrar sin que nada lo delate.
// N-137 (04/09): AQUI PONIA `maxVerde = 2` MINUTOS, POR DEBAJO DEL MINIMO VIAL.
//
// Este modo configura el coordinador por su cuenta -no pasa por SET_TIEMPOS-, asi que
// la guarda de los 3 minutos no lo tocaba. Y era el modo que la guia de banco
// recomendaba como salida mientras el Automatico estuvo roto: el cruce habria corrido
// con verdes de 2 minutos justo donde el responsable dijo que el minimo son 3.
//
// Los limites salen ahora de limites_ciclo.h, que es el unico sitio donde viven.
//
// =====================================================================================
// A-12 (05/09): ESTE MODO NO LEIA NI UNO DE LOS TIEMPOS QUE CONFIGURA EL OPERARIO.
// =====================================================================================
//
// Aqui habia `static int maxVerde = VERDE_MIN_MIN;` y una sola escritura mas, en el
// setup(), con la misma constante. Nadie mas lo tocaba NUNCA. O sea: el operario
// configuraba 6 minutos por SET_TIEMPOS porque ese tramo es largo, el equipo contestaba
// $ACK, y en cuanto entraba en Inteligente el cruce corria a 3. La app mandaba bien el
// dato; lo tiraba el firmware. Y ninguna guarda lo veia porque este modo NO PASA por
// SET_TIEMPOS: se configuraba el coordinador por su cuenta.
//
// Y encima la Regla 1 cortaba el verde a los 15 SEGUNDOS -`tiempoActual >= 15000UL`-,
// medio minuto por debajo de los 3 minutos que el responsable fijo el 04/09 (D-5).
//
// EL MODELO QUE DECIDIO EL RESPONSABLE, y es el Automatico con UNA diferencia:
//
//   SUELO   el tiempo CONFIGURADO de la fase en curso -el verde configurado cuando esta
//           punta esta en verde, el rojo configurado cuando esta en rojo-. Por debajo
//           de el no cambia nada, y eso es lo que quita a las camaras la capacidad de
//           ACORTAR un verde: por debajo del minimo el conductor se convence de que el
//           semaforo esta averiado y adelanta en rojo, que es D-5 entero.
//   cumplido el suelo, si el OTRO lado pide paso -> cambia, igual que el Automatico.
//   si el otro lado NO pide y en el mio hay trafico -> MANTIENE. Esto es lo unico que
//           aportan las camaras, y solo alarga cuando no hay nadie esperando enfrente.
//   TECHO   el DOBLE del tiempo configurado, y ni un segundo mas.
//
// LA PROPIEDAD QUE HACE QUE ESTO SEA SEGURO, y es la que hay que mirar antes que
// ninguna otra: CON LAS CAMARAS MUERTAS ESTE MODO SE COMPORTA EXACTAMENTE COMO EL
// AUTOMATICO. Si la camara nunca dice "hay coches", miLadoConTrafico es siempre falso,
// la condicion de mantener no se cumple nunca y la fase termina en el suelo -que es la
// duracion del Automatico-. Degrada al comportamiento conocido, no a uno raro.
//
// POR QUE SON DOS NUMEROS Y NO UNO, que es la trampa de este arreglo: `maxVerde` YA
// valia 3 minutos. Subir el piso de la Regla 1 a 3 minutos y dejar el resto habria
// puesto SUELO = TECHO, la comparacion del techo no habria podido dar dos respuestas
// nunca -§3.septies- y las camaras habrian quedado INERTES justo en el unico modo que
// las usa. El suelo sale de la configuracion; el techo se DERIVA de el.
static const uint8_t TECHO_POR_SUELO = 2;

// EL TECHO NO SE ESCRIBE COMO CONSTANTE PROPIA. Un "el techo son 6 minutos" al lado
// seria un numero mas que alguien tendria que sincronizar con el ciclo -N-137 por
// sexta vez-, y ademas mentiria en cuanto el operario cambiara el tramo.

// Los tiempos configurados, CONGELADOS AL EMPEZAR LA FASE. No se releen en cada vuelta
// y ese es el motivo: la guarda de SET_TIEMPOS mira modoAutomatico_enMarcha(), que solo
// es cierta en MODO_AUTOMATICO, asi que estando en Inteligente el operario SI puede
// cambiar los tiempos. Releerlos aqui dejaria que una configuracion nueva ACORTARA la
// fase que ya esta corriendo -exactamente lo que modo_automatico.cpp prohibe en su
// guarda y por el mismo motivo-. Los tiempos nuevos entran enteros en la fase siguiente.
//
// Arrancan en los MINIMOS VIALES y no en cero: un suelo de cero dejaria el cruce
// alternando sin plazo, y un camino donde eso pueda pasar no se deja abierto aunque hoy
// nadie lo recorra.
static uint8_t verdeFaseMin = VERDE_MIN_MIN, rojoFaseMin = ROJO_MIN_MIN,
               despejeFaseSeg = DESPEJE_SEG_MIN;
static unsigned long tEstadoDesde = 0;
static bool primeraVezCorriendo = true;

// N-97 (31/08): el lector antirrebotado de camara se mudo a botones.cpp, que es el dueno
// de J16 y ahora tambien de las dos camaras nuevas. No se copio: es LA MISMA funcion,
// camara_leerPin(), y la usan las dos puntas. Aqui se llama, no se redefine.

void modoInteligente_setup() {
  // N-97 (31/08): AQUI YA NO SE DECLARA LA CAMARA, Y ESE ERA EL DEFECTO.
  //
  // pinMode(CAM_DEMANDA_PIN, INPUT) vivia en esta funcion, o sea que el pin de la camara
  // solo estaba configurado mientras el equipo estuviera EN Modo Inteligente -mientras el
  // Esclavo lo declaraba en su setup(), siempre-. Un modo no es dueno de una entrada
  // fisica: la entrada existe desde que la tarjeta arranca, la mire quien la mire. Ahora
  // las dos puntas la declaran en el arranque; en esta, en botones_setup().
  //
  // La cuenta que fija la polaridad -activo en ALTO- sigue estando, entera, en pines.h.
  //
  // N-64: PB8 no es una entrada de camara, es el LED testigo D5 (R16 1K). Se deja en
  // alta impedancia: un pin flotante no puede encenderlo sea cual sea el sentido del
  // diodo, y con INPUT_PULLUP quedaria a medio encender por 40 uA de fuga.
  pinMode(LED_TESTIGO, INPUT);

  // A-12: aqui se reescribian los tiempos a los minimos en cada entrada al modo. Ahora
  // se PREGUNTAN, que es lo contrario: los decide el operario y viven en un solo sitio.
  //
  // Y el orden de los argumentos importa aunque el coordinador tire dos de los tres
  // -mirese su firma: `coordinador_configurar(unsigned long, unsigned long, unsigned
  // long)`, los dos ultimos sin nombre-. Antes se pasaba el verde por la posicion del
  // rojo y no se notaba porque los dos valian lo mismo; con tiempos distintos por
  // sentido eso deja de ser inocuo el dia que alguien los use.
  modoAutomatico_tiemposCiclo(&verdeFaseMin, &rojoFaseMin, &despejeFaseSeg);

  coordinador_configurar((unsigned long)despejeFaseSeg * 1000UL,
                          (unsigned long)rojoFaseMin * 60000UL,
                          (unsigned long)verdeFaseMin * 60000UL);
  coordinador_iniciarModo();

  tEstadoDesde = millis();
  primeraVezCorriendo = true;
  lcd_dibujarInteligente(coordinador_nombreEstadoMaster(), 0, true);
}

void modoInteligente_loop() {
  if (botonCancelar()) {
    modoActual_set(MENU);
    menu_setup();
    return;
  }

  {
      coordinador_actualizar();

      // ==============================================================
      // PRESENCIA VEHICULAR: LAS TRES ENTRADAS, Y POR DONDE ENTRA CADA UNA
      // ==============================================================
      // Se leen UNA SOLA VEZ POR VUELTA y las comparte todo el modo -la decision y la
      // pantalla-. Antes la pantalla se hacia su propia lectura unas lineas mas abajo, y
      // ademas solo miraba J14: el operario podia ver "0 presencia" mientras el modo
      // estaba manteniendo el verde por una deteccion. Dos formulas de "hay coches" que
      // solo la disciplina mantiene iguales son el defecto que este fichero ya paga en
      // otros sitios; ahora lo que se dibuja es LO MISMO que decide.
      //
      // De paso ahorra una llamada a camara_leerPin() por vuelta, y esa llamada no es
      // gratis: lleva delay(5) dentro cuando el pin esta alto.
      //
      // ---- SENTIDO 1 (esta punta) -------------------------------------------------
      //   J14 / CAM_DEMANDA_PIN (PB0)  -> se lee AQUI, POR NIVEL. Es la entrada que el
      //        responsable dejo abierta para un posible fin de carrera de barrera; no se
      //        retira.
      //   J16 / CAM_C_PIN y CAM_D_PIN  -> POR NIVEL, con camara_presenciaJ16(), que
      //        devuelve lo que camaras_actualizar() acaba de leer en esta misma vuelta.
      //        Es la camara que el responsable decidio instalar el 05/09, y desde aqui
      //        entra por DOS caminos que contestan a dos preguntas distintas:
      //          - PEDIR PASO, por flanco: camaras_actualizar() llama a
      //            demanda_solicitar(), que sale por demanda_hayLocal() -el tercer
      //            termino de este OR-. Ese camino ya existia y no se toca.
      //          - SOSTENER LA FASE, por nivel: este termino. Hacia falta porque la
      //            ventana de demanda_hayLocal() son 3 s y NO se prolonga con cada
      //            deteccion, asi que entre dos peticiones aceptadas queda un hueco en el
      //            que "hay cola" contesta que no; y basta que el muestreo caiga en un
      //            hueco para que la fase termine en el suelo. Sin este termino la camara
      //            de J16 llegaba al modo y casi no podia alargar nada. Medido en el
      //            Bloque F de Validacion_Automatico, no razonado.
      //        Es el mismo reparto que el firmware ya hacia con J14 -el Maestro la lee
      //        POR NIVEL porque el Maestro DECIDE (SFTY-27)-, aplicado a las nuevas.
      //   la app / DEMANDA por Bluetooth -> por la misma puerta, demanda_hayLocal(). Un
      //        atajo propio se saltaria el suelo y el techo de la fase.
      //
      // QUE PASA SI J14 Y J16 DAN SENAL A LA VEZ: nada distinto. Esto es un OR de tres
      // booleanos, asi que dos presencias simultaneas valen exactamente lo mismo que una
      // -no se suman, no se cuentan, no hay doble efecto, y el techo de la fase sigue
      // siendo el mismo-. Lo unico que de verdad comparten es la ventana de silencio de
      // demanda.cpp: si J16 dispara dentro de los 3 s de una peticion anterior,
      // demanda_solicitar() la descarta COMO PETICION NUEVA -no se pierde la presencia,
      // que ya la sostiene el nivel- y la ventana NO se prolonga: cuenta desde la primera.
      //
      // ---- SENTIDO 2 (la otra punta) ----------------------------------------------
      //   La camara del Esclavo manda CMD_DEMANDA por radio -> demanda remota.
      const bool demandaLocalS1  = camara_leerPin(CAM_DEMANDA_PIN)
                                   || camara_presenciaJ16()
                                   || demanda_hayLocal();
      const bool demandaRemotaS2 = coordinador_hayDemandaRemota();

      if (coordinador_listoParaContar()) {
        if (primeraVezCorriendo) {
          tEstadoDesde = millis();
          primeraVezCorriendo = false;
          // Empieza una fase: aqui y solo aqui se congelan los tiempos. El porque, en
          // la cabecera de verdeFaseMin.
          modoAutomatico_tiemposCiclo(&verdeFaseMin, &rojoFaseMin, &despejeFaseSeg);
        }

        // EL SUELO ES EL DE LA FASE EN CURSO. Cuando esta punta esta en ROJO la fase que
        // corre es el VERDE del sentido 2, y su plazo es el rojo configurado de aqui; es
        // el mismo reparto que hace el Automatico en su `duracion`.
        const bool enRojo = (semaforo_estado() == S_ROJO);
        const uint8_t sueloMin = enRojo ? rojoFaseMin : verdeFaseMin;
        const uint8_t topeMin  = enRojo ? ROJO_MIN_MAX : VERDE_MIN_MAX;

        // EL TECHO SE DERIVA, Y SE SATURA AL MAXIMO DEL RANGO VIAL. Con 8 minutos
        // configurados el doble son 16 y el rango llega a 15: se recorta. Dejarlo salir
        // pondria al cruce en un plazo que la propia guarda de SET_TIEMPOS rechazaria si
        // alguien intentara configurarlo a mano.
        //
        // En el tope del rango -15 minutos configurados- el techo COINCIDE con el suelo
        // y este modo deja de poder alargar. Eso no es la guarda muerta de §3.septies:
        // es la saturacion haciendo su trabajo en un solo punto del rango, y en ese
        // punto lo correcto es comportarse como el Automatico.
        unsigned int techoMin = (unsigned int)sueloMin * TECHO_POR_SUELO;
        if (techoMin > topeMin) techoMin = topeMin;

        const unsigned long sueloMs = (unsigned long)sueloMin * 60000UL;
        const unsigned long techoMs = (unsigned long)techoMin * 60000UL;
        const unsigned long tiempoActual = millis() - tEstadoDesde;

        bool forzarCambio = false;

        // Quien pide paso es el lado que NO lo tiene, y quien puede alargar es el que si.
        const bool otroLadoPide     = enRojo ? demandaLocalS1  : demandaRemotaS2;
        const bool miLadoConTrafico = enRojo ? demandaRemotaS2 : demandaLocalS1;

        // POR DEBAJO DEL SUELO NO CAMBIA NADA. Ni una camara, ni una demanda a mano, ni
        // las dos: una camara no puede ACORTAR una fase, solo alargarla. Esta linea es
        // toda la diferencia con el `tiempoActual >= 15000UL` que habia aqui.
        if (tiempoActual >= sueloMs) {
          // Cumplido el suelo se cambia, IGUAL QUE EL AUTOMATICO, salvo el unico caso
          // que las camaras aportan: en mi lado hay trafico y enfrente no espera nadie.
          // Con las camaras mudas las dos banderas son falsas, esto vale siempre `true`
          // y el modo es el Automatico exacto. Esa es la propiedad de la cabecera.
          forzarCambio = !(miLadoConTrafico && !otroLadoPide);
        }

        // Y el techo cierra por arriba: mantener no es indefinido.
        if (tiempoActual >= techoMs) {
          forzarCambio = true;
        }

        if (forzarCambio) {
          // LA DEMANDA REMOTA SE SATISFACE CUANDO EL SENTIDO 2 RECIBE EL VERDE, o sea
          // cuando esta punta SALE de su verde. Limpiarla al salir del rojo la borraria
          // sin haberla atendido, y la camara del Esclavo dispara por DETECCION -no
          // mientras alguien espera-, asi que nadie volveria a levantarla y el coche
          // parado se quedaria sin turno. Antes se limpiaba tambien en el tope, que es
          // justo el caso en que el otro lado lleva mas rato esperando.
          if (!enRojo) coordinador_limpiarDemandaRemota();
          coordinador_pedirCambio();
          tEstadoDesde = millis();
        }
      } else {
        primeraVezCorriendo = true;
      }

      static const char* estadoAnt = "";
      static int presenciaAnt = -1;
      const char* actual = coordinador_nombreEstadoMaster();

      // LO QUE SE DIBUJA ES LO QUE DECIDE, y no una segunda lectura. Aqui se llamaba otra
      // vez a camara_leerPin(CAM_DEMANDA_PIN), o sea que la pantalla contaba SOLO la
      // camara de J14: una deteccion de J16 -que es donde el responsable decidio poner la
      // camara el 05/09- alargaba la fase sin aparecer en el contador de presencia. El
      // operario veia un cruce que no cambiaba y un cero al lado.
      int presenciaActual = (demandaLocalS1 ? 1 : 0) + (demandaRemotaS2 ? 1 : 0);

      if (strcmp(actual, estadoAnt) != 0 || presenciaActual != presenciaAnt) {
        lcd_dibujarInteligente(actual, presenciaActual, true);
        estadoAnt = actual;
        presenciaAnt = presenciaActual;
      }
  }
}