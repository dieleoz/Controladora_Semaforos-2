// ===== 01_Firmware/ESP32_Expansion/src/despachador.cpp =====

#include "despachador.h"
#include "reloj_ds3231.h"
#include "puente.h"
#include <stdio.h>
#include <string.h>

void despachador_observar(const char* linea, bool propagada) {
  if (linea == NULL) return;

  // strstr, NO strncmp contra un prefijo con el PIN dentro.
  //
  // La linea real es "CMD:PIN:1234:SET_RTC:2026-08-31,23:59:59". Buscar el prefijo
  // completo obligaria a escribir "1234" aqui, y entonces el puente TENDRIA el PIN:
  // una segunda copia del contrato que alguien tiene que sincronizar, y el dia que
  // difirieran un comando funcionaria por una puerta y seria rechazado por la otra.
  // Buscando solo "SET_RTC:" el puente transporta el PIN sin conocerlo.
  const char* p = strstr(linea, "SET_RTC:");
  if (p == NULL) return;
  p += 8;

  // MOTIVO 1: la trama no casa el formato. Se contesta ANTES de tocar el bus: una
  // escritura con campos basura seria la escritura a medias que R-5 prohibe.
  FechaHora f;
  if (sscanf(p, "%d-%d-%d,%d:%d:%d",
             &f.anio, &f.mes, &f.dia, &f.hora, &f.minuto, &f.segundo) != 6) {
    puente_emitirPropio("$ERR,NODE:PUENTE,CMD:SET_RTC,DESC:FORMATO_INVALIDO");
    return;
  }

  // 🔴 AQUI ESTA TODO EL PACK esp32_03. El retorno se guarda y CADA rama de abajo lo
  // interroga dentro de su propio `if`. El molde es SET_TIEMPOS del Maestro
  // (bluetooth.cpp:275-294): pregunta dentro del if y tiene un $ERR por cada motivo.
  //
  // La forma prohibida -la que costo N-80- seria:
  //     reloj_ajustar(&f);
  //     puente_emitirPropio("$ACK,...,RESULT:OK");
  // Esa version dice que si con el bus mudo, con la pila agotada y con el modulo
  // desenchufado, y el tecnico se va del poste creyendo que dejo el reloj puesto.
  ResultadoReloj r = reloj_ajustar(&f);

  if (r == RELOJ_ERR_FORMATO) {
    // No lo puede devolver reloj_ajustar() hoy -el formato se decide arriba-, pero el
    // valor existe en el enum y una rama que faltara dejaria ese caso sin respuesta el
    // dia que alguien lo empiece a devolver. Un comando sin contestar se lee como
    // equipo colgado, y el tecnico lo reintenta.
    puente_emitirPropio("$ERR,NODE:PUENTE,CMD:SET_RTC,DESC:FORMATO_INVALIDO");

  } else if (r == RELOJ_ERR_RANGO) {
    // MOTIVO 2. Se reusa el mismo DESC que el motivo 1 a proposito: para quien esta
    // delante del telefono, "el ano dice 3000" y "faltan campos" son el mismo arreglo
    // -reescribir la fecha-, y es lo que ya hace el Maestro en :313-318. Lo que NO se
    // reusa es la RAMA: son dos caminos distintos del firmware y se ven distintos.
    puente_emitirPropio("$ERR,NODE:PUENTE,CMD:SET_RTC,DESC:FORMATO_INVALIDO");

  } else if (r == RELOJ_ERR_SIN_RELOJ) {
    // MOTIVO 3: el bus no contesta. Modulo ausente, mal cableado, SDA y SCL cruzados.
    // Es un arreglo con destornillador, no con teclado, y por eso no se confunde con
    // los de abajo.
    puente_emitirPropio("$ERR,NODE:PUENTE,CMD:SET_RTC,DESC:SIN_RELOJ_NO_RESPONDE");

  } else if (r == RELOJ_ERR_ESCRITURA) {
    // MOTIVO 4: el bus contestaba y la escritura fallo a mitad.
    puente_emitirPropio("$ERR,NODE:PUENTE,CMD:SET_RTC,DESC:ESCRITURA_FALLIDA");

  } else if (r == RELOJ_ERR_NO_QUEDO_PUESTA) {
    // MOTIVO 5: se escribio, el bus dijo que si, y la relectura no coincide. Es el caso
    // que un $ACK sin releer no puede distinguir de un exito.
    puente_emitirPropio("$ERR,NODE:PUENTE,CMD:SET_RTC,DESC:NO_QUEDO_PUESTA");

  } else if (r == RELOJ_ERR_OSF_SIGUE) {
    // MOTIVO 6: la hora entro en los registros y el oscilador NO arranco. Es el equipo
    // que hoy tiene el STM32 con Y2 muerto, con la diferencia de que aqui se dice. Se
    // nombra la pila porque en este modulo si esta medido de que depende el oscilador
    // -N-45 prohibio senalar componentes sin haberlo medido, no nombrarlos nunca-.
    puente_emitirPropio("$ERR,NODE:PUENTE,CMD:SET_RTC,DESC:OSCILADOR_PARADO_CAMBIE_PILA");

  } else if (r != RELOJ_OK) {
    // Cualquier valor del enum que alguien anada manana y no cablee arriba. Sin esta
    // rama caeria por el else y se contestaria OK: un caso nuevo se aprobaria a si
    // mismo, que es justo como se cuelan los defectos en un cambio que "no cambia nada".
    puente_emitirPropio("$ERR,NODE:PUENTE,CMD:SET_RTC,DESC:MOTIVO_NO_CONTEMPLADO");

  } else if (!propagada) {
    // MOTIVO 7, Y ES UN $ACK, NO UN $ERR. La hora entro AQUI y la linea no llego entera
    // al STM32, asi que el equipo se quedo con la suya. Es medio arreglo, y sin esta
    // rama se leeria como entero.
    //
    // LIMITE DECLARADO: `propagada` significa "la linea se puso entera en el cable",
    // no "el STM32 la acepto". Esperar su $ACK obligaria a bloquear el bombeo, y un
    // puente que se para a esperar es un puente que deja de pasar telemetria. El $ACK
    // del propio STM32 sube a la app por su cuenta, y como este va marcado NODE:PUENTE
    // el operario ve las dos respuestas y sabe cual es de quien.
    FechaHora leida;
    if (!reloj_leer(&leida)) {
      puente_emitirPropio("$ERR,NODE:PUENTE,CMD:SET_RTC,DESC:NO_QUEDO_PUESTA");
    } else {
      char p[112];
      snprintf(p, sizeof(p),
               "$ACK,NODE:PUENTE,CMD:SET_RTC,RESULT:HORA_PUESTA_SIN_PROPAGAR,"
               "FECHA:%04d-%02d-%02d,HORA:%02d:%02d:%02d",
               leida.anio, leida.mes, leida.dia,
               leida.hora, leida.minuto, leida.segundo);
      puente_emitirPropio(p);
    }

  } else {
    // LA HORA QUE SE DEVUELVE ES LA QUE EL MODULO TIENE, RELEIDA POR LA BARRERA.
    //
    // No se hace por adorno: es lo que le da un llamador de verdad a reloj_enHora() -a
    // traves de reloj_leer()- y lo que convierte el $ACK en una medida en vez de una
    // promesa. El tecnico ve la hora que quedo dentro, no la que mando.
    //
    // Y SI LA RELECTURA FALLA AQUI, ESTO ES UN $ERR. Entre la escritura confirmada y
    // esta lectura solo cabe que el bus se haya caido o que el OSF haya vuelto: los dos
    // significan que la hora no esta puesta, y contestarlo como exito seria justo la
    // mentira con formato de exito que este fichero existe para no cometer.
    FechaHora leida;
    if (!reloj_leer(&leida)) {
      puente_emitirPropio("$ERR,NODE:PUENTE,CMD:SET_RTC,DESC:NO_QUEDO_PUESTA");
    } else {
      // 🔴 EL LITERAL SE REPITE EN LAS DOS RAMAS A PROPOSITO, Y NO SE SACA A UN
      // COMPOSITOR. Es N-89 exacto: el pack que vigila esta propiedad busca los
      // literales "$ACK / "$ERR DENTRO del bloque de cada rama. Con un
      // responderConHora(resultado) que armara la trama en otro sitio, ninguna rama
      // tendria ya el literal, TODAS pasarian por "no promete nada" -incluidos los
      // controles negativos- y el pack seguiria en verde midiendo nada. Seis lineas
      // repetidas contra un instrumento que deja de medir no es un intercambio.
      char p[112];
      snprintf(p, sizeof(p),
               "$ACK,NODE:PUENTE,CMD:SET_RTC,RESULT:OK,"
               "FECHA:%04d-%02d-%02d,HORA:%02d:%02d:%02d",
               leida.anio, leida.mes, leida.dia,
               leida.hora, leida.minuto, leida.segundo);
      puente_emitirPropio(p);
    }
  }
}
