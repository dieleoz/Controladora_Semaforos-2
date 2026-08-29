// ===== src/modo_hora.cpp =====
#include "modo_hora.h"
#include "botones.h"
#include "coordinador.h"
#include "lcd.h"
#include "menu.h"
#include "modos.h"
#include "reloj.h"

// Se edita DIGITO A DIGITO. Con un solo boton de subir, poner los minutos como
// valor completo costaria hasta 59 pulsaciones; por digito son 9 como maximo, y
// normalmente dos o tres. Ademas funciona igual con el mando de reles, que solo
// entrega pulsos y no permite repeticion por mantener pulsado.
//
//   digito 0 = decena de hora    (0..2)
//   digito 1 = unidad de hora    (0..9, acotada a 0..3 si la decena es 2)
//   digito 2 = decena de minuto  (0..5)
//   digito 3 = unidad de minuto  (0..9)
static uint8_t d[4] = {0, 0, 0, 0};
static uint8_t digito = 0;

// Tope de cada digito, +1. El de la unidad de hora depende de la decena: con 2
// delante solo puede llegar a 3, porque no existe la hora 24.
static uint8_t moduloDe(uint8_t i) {
  switch (i) {
    case 0:  return 3;                   // 0..2
    case 1:  return (d[0] == 2) ? 4 : 10;// 0..3 si es 2x, si no 0..9
    case 2:  return 6;                   // 0..5
    default: return 10;                  // 0..9
  }
}

// Tras cambiar la decena de hora a 2, la unidad puede haber quedado fuera de
// rango (p.ej. 19 -> 29). Se recorta en vez de dejar una hora imposible.
static void normalizar() {
  if (d[0] == 2 && d[1] > 3) d[1] = 3;
}

static uint8_t horaActual()   { return (uint8_t)(d[0] * 10 + d[1]); }
static uint8_t minutoActual() { return (uint8_t)(d[2] * 10 + d[3]); }

// N-23 — CONFIRMAR LA HORA YA NO ES SALIRSE DE LA PANTALLA.
//
// CONFIRMADO EN BANCO EL 01/08/2026. Al pulsar aceptar, esta pantalla llamaba a
// coordinador_sincronizarHora() -que solo ENCOLA el envio- y en la linea siguiente
// se iba al menu. Y en el menu nadie llama a coordinador_actualizar(): solo lo hacen
// automatico, manual, inteligente, PRUEBA ALCANCE y esta misma pantalla.
//
// De modo que la peticion quedaba encolada y LAS TRAMAS NUNCA SALIAN. El operario
// ponia la hora, veia que se guardaba -el reloj del Maestro si quedaba puesto- y a
// continuacion el Modo Degradado lo rechazaba con "Falta: nunca hubo sincronizacion
// RF", sin nada que relacionara una cosa con la otra. Con los radios enlazados al
// 100% y el automatico funcionando, el mensaje parecia mentir.
//
// Y no se guardaba nada en NINGUNA de las dos puntas, que era la otra sospecha: la
// marca de sincronizacion se graba en un unico punto del firmware, al recibir el
// CMD_ACK_HORA del Esclavo. Sin tramas no hay ACK, y sin ACK no hay marca ni en la
// RAM ni en la pila. Las dos hipotesis eran la misma un paso mas arriba.
//
// Ahora la pantalla SE QUEDA moviendo el coordinador hasta que el Esclavo acusa la
// hora, y dice cual de las dos cosas se consiguió. Es la diferencia entre poner el
// reloj y sincronizar, y el operario tiene que verla en el momento, no descubrirla
// tres pantallas despues.
enum FaseHora { FH_EDITANDO, FH_ENVIANDO, FH_RESULTADO };
static FaseHora fase = FH_EDITANDO;
static unsigned long tFase = 0;
static bool envioOk = false;
static bool sinReloj = false;

// Cuanto se espera al ACK. El intercambio lleva sus propios reintentos dentro del
// coordinador; esto solo acota la espera del operario delante de la pantalla.
static const unsigned long ESPERA_ACK_MS = 8000;

// Cuanto se queda el resultado antes de volver solo al menu. Suficiente para leerlo
// desde los 5 m del gabinete, y se puede cortar con cualquier boton.
static const unsigned long RESULTADO_MS = 4000;

// N-45 — cada cuanto se relee el RTC en la pantalla de consulta. El contador del F1
// avanza una vez por segundo: con medio segundo se ve cambiar sin repintar de mas.
static const unsigned long REFRESCO_DIAG_MS = 500;
static unsigned long tDiag = 0;
static bool latidoDiag = false;

// Sincronizacion NUEVA, no una vieja. Se compara la antiguedad que reporta el
// coordinador contra el tiempo que llevamos esperando: si la ultima sync es mas
// reciente que el momento en que pulsamos aceptar, es la nuestra. Mirar solo que
// "haya habido alguna" daria por bueno un intercambio de hace media hora.
static bool syncNuevaConfirmada() {
  const unsigned long desde = coordinador_msDesdeUltimaSync();
  if (desde == 0xFFFFFFFFUL) return false;   // nunca hubo ninguna
  return desde <= (millis() - tFase);
}

static void repintar() {
  // N-24: se le pasa tambien si el oscilador arranco. Sin ese dato la pantalla solo
  // podia decir "sin poner en hora", que manda al operario a teclear la hora contra
  // un RTC parado una y otra vez.
  lcd_dibujarAjusteHora(horaActual(), minutoActual(), digito, reloj_enHora(),
                        reloj_hayCristal());
}

void modo_hora_setup() {
  // Mismo estado seguro que el Menu y que PRUEBA ALCANCE: Rojo Fijo en ambos
  // extremos con enlace, Ambar Intermitente sin el. Esta pantalla NO arranca ciclos.
  coordinador_forzarMenu();

  // Se parte de la hora actual si es fiable. Si el reloj nunca se puso, arrancar
  // en 00:00 es mas honesto que mostrar una hora inventada que parezca valida.
  uint8_t h = 0, m = 0;
  if (reloj_enHora()) {
    h = reloj_hora();
    m = reloj_minuto();
  }
  d[0] = (uint8_t)(h / 10); d[1] = (uint8_t)(h % 10);
  d[2] = (uint8_t)(m / 10); d[3] = (uint8_t)(m % 10);
  digito = 0;

  // N-23: se vuelve a editar SIEMPRE al entrar. Sin esto, una salida por el camino
  // del resultado dejaria la fase colgada y la siguiente entrada a la pantalla
  // arrancaria mostrando el desenlace del ajuste anterior.
  fase = FH_EDITANDO;
  envioOk = false;
  sinReloj = false;

  repintar();
}

void modo_hora_loop() {
  // N-23: ENVIANDO. Aqui esta la razon de ser de toda esta fase: se sigue llamando a
  // coordinador_actualizar(), que es lo que de verdad saca las tramas al aire. Antes
  // se salia al menu en este punto y el envio se quedaba encolado para siempre.
  if (fase == FH_ENVIANDO) {
    coordinador_actualizar();

    if (syncNuevaConfirmada()) {
      envioOk = true;
      fase = FH_RESULTADO;
      tFase = millis();
      lcd_dibujarSyncHora(false, true);
      return;
    }

    // Se agoto la espera. No es un fallo del ajuste: el reloj del Maestro quedo
    // puesto igual. Lo que no hay es sincronizacion, y por eso el Modo Degradado
    // seguira rechazando. La pantalla lo dice con esas palabras.
    if (millis() - tFase >= ESPERA_ACK_MS) {
      envioOk = false;
      fase = FH_RESULTADO;
      tFase = millis();
      lcd_dibujarSyncHora(false, false);
    }
    return;
  }

  // N-23: RESULTADO. Se sostiene para poder leerlo desde el suelo y se puede cortar
  // con cualquier boton. El coordinador se sigue moviendo para no dejar al Esclavo
  // sin latido y que se vaya a ambar por orfandad mientras se lee la pantalla.
  if (fase == FH_RESULTADO) {
    coordinador_actualizar();

    // N-45: LA CONSULTA SE RELEE. Una sola lectura del contador no distingue un RTC
    // parado de uno que avanza -que es justo lo que hay que averiguar-, asi que se
    // repinta y lo que se mira es si el numero CAMBIA. El RTC del F1 cuenta una vez
    // por segundo; medio segundo de refresco basta para verlo sin cargar el bus de
    // la pantalla mas de lo necesario.
    if (sinReloj && millis() - tDiag >= REFRESCO_DIAG_MS) {
      tDiag = millis();
      latidoDiag = !latidoDiag;
      RelojDiag diag;
      reloj_diagnostico(&diag);
      lcd_dibujarDiagnosticoReloj(diag, latidoDiag);
    }

    // N-45: la consulta del reloj NO se va sola. Los otros resultados son un
    // desenlace de una linea que se lee de un vistazo; esta son cuatro lineas de
    // numeros que hay que APUNTAR desde los 5 m del gabinete, y 4 s no dan. Se
    // queda hasta que alguien pulse. Es seguro: el coordinador se sigue moviendo
    // aqui abajo, asi que el Esclavo no ve orfandad, y el estado de luces es el
    // mismo Rojo Fijo del menu.
    const bool pulsado = botonArriba() || botonAbajo() || botonAceptar() || botonCancelar();
    if (pulsado || (!sinReloj && millis() - tFase >= RESULTADO_MS)) {
      fase = FH_EDITANDO;
      modoActual_set(MENU);
      menu_setup();
    }
    return;
  }

  bool cambio = false;

  // Boton 1: incrementa el digito activo, con vuelta al principio.
  if (botonArriba()) {
    d[digito] = (uint8_t)((d[digito] + 1) % moduloDe(digito));
    normalizar();
    cambio = true;
  }

  // Boton 2: decrementa. Se suma el modulo antes de restar para no bajar de cero
  // en aritmetica sin signo, donde 0 - 1 daria 255.
  if (botonAbajo()) {
    uint8_t mod = moduloDe(digito);
    d[digito] = (uint8_t)((d[digito] + mod - 1) % mod);
    normalizar();
    cambio = true;
  }

  // Boton 3: avanza al siguiente digito y, tras el ultimo, CONFIRMA.
  if (botonAceptar()) {
    if (digito < 3) {
      digito++;
      cambio = true;
    } else {
      // Los segundos se ponen a cero: el operario ajusta hora y minuto, y arrancar
      // el minuto en su segundo 0 es lo mas parecido a poner el reloj en hora
      // contra una referencia externa.
      reloj_ajustar(horaActual(), minutoActual(), 0);

      // SFTY-23: poner en hora ES sincronizar. Un solo gesto del operario deja las
      // dos puntas iguales, y por eso el Esclavo nunca se ajusta a mano: hacerlo
      // dejaria hasta 59 s de desfase entre relojes que dos pantallas HH:MM no
      // pueden detectar.
      //
      // La llamada va aqui, explicita, y no se deja al vigilante de reloj que el
      // coordinador mantiene: aquel es una red de respaldo que compara el RTC
      // contra millis() una vez por segundo, y depender de un heuristico para algo
      // que aqui se sabe con certeza seria elegir lo fragil pudiendo ser exacto.
      // N-30: si el ajuste NO prendio -reloj_ajustar() se niega cuando el RTC no
      // cuenta, N-24- no hay nada que enviar y coordinador_sincronizarHora() tampoco
      // haria nada. Entrar en ENVIANDO acabaria en "Esclavo no responde" tras 8 s de
      // espera en vacio, mandando al tecnico a revisar la otra punta, la radio y las
      // antenas por un fallo que esta en la tarjeta que tiene delante.
      // N-45: aqui es donde el operario descubre que no se le deja poner la hora, y
      // por tanto es donde tiene que ver POR QUE. Antes salia "Revisa Y2, pila y R5"
      // -una conclusion escrita a mano, sin haber medido ninguna de las tres cosas- y
      // mando a cambiar los tres componentes con el hardware sano. Ahora se ensenan
      // los bits del RTC y decide quien mira.
      if (!reloj_enHora()) {
        sinReloj = true;
        fase = FH_RESULTADO;
        tFase = millis();
        tDiag = millis();
        latidoDiag = false;
        RelojDiag diag;
        reloj_diagnostico(&diag);
        lcd_dibujarDiagnosticoReloj(diag, latidoDiag);
        return;
      }
      sinReloj = false;

      coordinador_sincronizarHora();

      // NO SE SALE AQUI (N-23). Se pasa a esperar el ACK moviendo el coordinador
      // desde el bucle de abajo, que es lo unico que hace que las tramas salgan.
      fase = FH_ENVIANDO;
      tFase = millis();
      lcd_dibujarSyncHora(true, false);
      return;
    }
  }

  // Boton 4: sale SIN guardar. Se trabaja sobre una copia y solo se escribe al RTC
  // al confirmar, asi que entrar por error no altera el reloj.
  if (botonCancelar()) {
    modoActual_set(MENU);
    menu_setup();
    return;
  }

  // Mantiene vivo el latido de 3 s mientras se esta en esta pantalla, para que el
  // Esclavo no interprete orfandad y se vaya a Ambar por estar el operario aqui.
  coordinador_actualizar();

  if (cambio) repintar();
}
