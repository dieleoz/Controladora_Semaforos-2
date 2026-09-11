// ===== 01_Firmware/ESP32_Expansion/src/despachador.cpp =====

#include "despachador.h"
#include "reloj_ds3231.h"
#include "puente.h"
#include "siembra.h"
#include <stdio.h>
#include <string.h>

// D-17: el reloj se puede CONSULTAR sin cambiarlo, y contesta quien lo tiene.
//
// Es consecuencia directa de D-15 -el STM32 no tiene reloj-, y es mejor que sincronizar:
// hasta esta decision la unica forma de leer la hora era mandarla, y con eso se perdia
// justo el dato que se buscaba. No hace falta que los dos relojes se pongan de acuerdo
// solos: hace falta poder ver si lo estan, y esa comparacion solo la puede hacer la app
// visitando los dos postes, porque los dos modulos de expansion no se hablan entre si.
//
// EL COMANDO DE CONSULTA, EN UNA SOLA COPIA.
//
// Lo leen DOS funciones de este fichero -el predicado que veta el reenvio y la rama que
// contesta-, y por eso es una constante y no dos literales iguales: el dia que dos
// copias difirieran, el puente se quedaria una linea que no sabe contestar, o
// contestaria una que ademas viajo al cable. Las dos averias son mudas.
//
// SE COMPARA ENTERA CON strcmp Y NO CON strstr, al reves que SET_RTC. Alli hace falta
// buscar dentro porque el PIN va delante y el puente no puede conocerlo; aqui la linea
// NO LLEVA PIN -es una consulta que no abre paso ni cambia nada- asi que la forma exacta
// del cable es esta y solo esta. Un strstr aceptaria "CMD:PIN:1234:LEER_RTC" y cualquier
// otra cosa con el texto dentro, y entonces el puente se estaria quedando lineas que no
// son suyas.
static const char CMD_LEER_RTC[] = "CMD:LEER_RTC";

// D-20 (11/09): SET_RTC SE BUSCA DENTRO DE LA LINEA, NO SE COMPARA ENTERA.
//
// La linea real es "CMD:PIN:1234:SET_RTC:2026-08-31,23:59:59": el PIN va delante y el
// puente no lo conoce -esp32_09 exige que no aparezca en ningun fuente de aqui-, asi que
// la unica forma de reconocerla sin tenerlo es buscar "SET_RTC:" dentro. Es el mismo
// criterio que esta rama usaba desde el 31/08 para poner el DS3231; lo que cambia es que
// ahora tambien decide que la linea NO CRUZA.
//
// UNA SOLA COPIA, y la usan el predicado y la rama que contesta. Si fueran dos, el dia
// que difirieran el puente se quedaria una linea que no sabe contestar -desaparece sin
// respuesta, y un comando mudo se lee como equipo colgado- o contestaria una que ademas
// viajo al cable, que es lo que D-20 viene a quitar.
static const char* accionSetRtc(const char* linea) {
  return strstr(linea, "SET_RTC:");
}

// D-20 (11/09) - ANTI-SUPLANTACION: LA LINEA DE HORA SOLO LA ORIGINA ESTE MODULO.
//
// El STM32 acepta CMD:HORA_ESP32 SIN PIN -no puede pedirlo: el puente no lo conoce- y
// sobreescribe su hora con ella. Si el telefono pudiera mandarla, cualquiera con
// Bluetooth dictaria la hora del cruce sin clave. Por eso toda linea del telefono que la
// CONTENGA -en cualquier posicion, con o sin PIN delante, con o sin prefijo- se descarta
// aqui y no cruza.
//
// strstr y no strcmp A PROPOSITO, al reves que LEER_RTC: aqui no se trata de reconocer
// una orden del puente, sino de que ninguna linea del telefono llegue al STM32 con ese
// texto dentro. Quedarse de mas solo cuesta un $ERR; quedarse de menos es la puerta.
//
// El texto es el mismo que va en FORMATO_HORA_ESP32 de siembra.cpp; son dos ficheros y
// esp32_13 comprueba que los dos digan lo mismo, porque si la orden cambiara de nombre
// alli y no aqui, esto seguiria guardando una puerta que ya no existe.
static bool suplantaLaSiembra(const char* linea) {
  return strstr(linea, "HORA_ESP32") != NULL;
}

// LO QUE EL PUENTE SE QUEDA: tres criterios, y cada uno lo usa tambien la rama que
// contesta (despachador_atender). esp32_12 los lee de aqui en cada corrida.
bool despachador_esParaElPuente(const char* linea) {
  if (linea == NULL) return false;
  if (strcmp(linea, CMD_LEER_RTC) == 0) return true;
  if (suplantaLaSiembra(linea)) return true;
  if (accionSetRtc(linea) != NULL) return true;
  return false;
}

void despachador_atender(const char* linea) {
  if (linea == NULL) return;

  // =================================================================================
  // D-20 - LA SUPLANTACION VA LA PRIMERA
  // =================================================================================
  // Antes que la consulta y antes que SET_RTC: una linea que traiga las dos cosas
  // -"CMD:PIN:1234:SET_RTC:HORA_ESP32..."- se trata como lo que puede hacer dano, no
  // como lo que parece. Y se CONTESTA: una orden que desaparece sin respuesta se lee
  // como equipo colgado y se repite.
  if (suplantaLaSiembra(linea)) {
    puente_emitirPropio("$ERR,NODE:PUENTE,CMD:HORA_ESP32,DESC:LINEA_RESERVADA_AL_PUENTE");
    return;
  }

  // =================================================================================
  // LEER_RTC - LA CONSULTA QUE NO ESCRIBE NADA
  // =================================================================================
  // POR QUE EXISTE. Hasta hoy la unica forma de LEER la hora del DS3231 era MANDARLA:
  // el $ACK de SET_RTC devuelve FECHA:/HORA: releidas del chip. O sea que comprobar si
  // el cruce esta en hora obligaba a cambiarle la hora a un equipo que esta en la calle,
  // y con eso se pierde justo el dato que se buscaba -cuanto se habia desviado-.
  //
  // A-9 dice que hay DOS relojes por cruce y que nada los sincroniza. Esto no los
  // sincroniza tampoco: los hace VISIBLES, que es lo que se pidio. Cero efecto vial: no
  // toca el reloj, no toca una luz y no pide PIN.
  //
  // POR QUE NO PIDE PIN, MEDIDO CONTRA EL CRITERIO QUE YA ESTA ESCRITO. El Maestro lo
  // dice en su despachador: "El PIN guarda lo que ABRE paso o mueve luces; no lo que las
  // para" -y por eso FORZAR_ROJO, SET_MODO:MENU y SET_MODO:ALCANCE entran sin clave-.
  // Una consulta no abre, no para y no cambia: no hay nada que custodiar. Y hay una
  // razon de campo encima: si el reloj esta mal, el tecnico tiene que poder MIRARLO
  // antes de decidir si toca algo, no despues de haber tecleado la llave que lo cambia.
  //
  // 🔴 Y LO QUE DE VERDAD IMPORTA AQUI ES LO QUE CONTESTA CUANDO NO SABE.
  //
  // Un DS3231 sin pila entrega una fecha PERFECTAMENTE FORMADA y completamente falsa; el
  // OSF es lo unico que las distingue, y no es el unico camino -esta el modo 12 h, la
  // escritura a medias y unos registros que no componen fecha-. N-144 es el precedente
  // con nombre: aquel dia el equipo se declaro EN HORA con el reloj parado en ceros y
  // publico HORA:00:00:00. Aqui la unica puerta de salida de una hora es reloj_leer(),
  // que lleva reloj_enHora() delante y NO TIENE variante "damela igual". Si esa puerta
  // dice que no, sale un $ERR CON EL MOTIVO -no un hueco, no un cero, no la ultima que
  // se vio-, y los motivos son los del enum MotivoSinHora, uno por rama.
  //
  // LAS RAMAS SON SIETE Y CADA UNA LLEVA SU LITERAL DENTRO. Es N-89 literal: el pack que
  // vigila esta propiedad busca "$ACK / "$ERR DENTRO del bloque de cada rama. Un
  // responderMotivo(m) que armara la trama en otro sitio dejaria a TODAS las ramas sin
  // literal, todas pasarian por "no promete nada" y el pack seguiria en verde midiendo
  // nada. Se repiten los prefijos a proposito.
  //
  // La condicion es la MISMA comparacion que el predicado -strcmp contra la constante-,
  // no una llamada al predicado: desde D-20 el predicado se queda tres cosas y esta rama
  // es solo la de la consulta.
  if (strcmp(linea, CMD_LEER_RTC) == 0) {
    FechaHora leida;
    if (reloj_leer(&leida)) {
      // LA HORA QUE SALE ES LA QUE EL MODULO TIENE, RELEIDA AHORA MISMO. No hay copia
      // en RAM que envejezca: reloj_leer() vuelve a hablar por el I2C en cada llamada,
      // asi que un bus que se cayo hace un minuto se ve AQUI y no en la corrida
      // siguiente. El formato es el MISMO que el $ACK de SET_RTC -FECHA: y HORA:- para
      // que la app lo anote con el mismo lector y no haya dos parsers de la misma cosa.
      char p[112];
      snprintf(p, sizeof(p),
               "$ACK,NODE:PUENTE,CMD:LEER_RTC,RESULT:OK,"
               "FECHA:%04d-%02d-%02d,HORA:%02d:%02d:%02d",
               leida.anio, leida.mes, leida.dia,
               leida.hora, leida.minuto, leida.segundo);
      puente_emitirPropio(p);

    } else {
      // SE PREGUNTA POR EL MOTIVO UNA SOLA VEZ. Llamar a reloj_motivo() dentro de cada
      // `if` seria releer una variable que puede cambiar entre ramas -reloj_leer() ya
      // la escribio- y entonces dos ramas podrian contestar por dos motivos distintos
      // de la misma consulta.
      MotivoSinHora m = reloj_motivo();

      if (m == SIN_HORA_NUNCA_SE_PUSO) {
        // El caso de un modulo virgen o recien arrancado sin que nadie le haya puesto
        // la hora. El arreglo es teclado, no destornillador, y por eso lo dice.
        puente_emitirPropio("$ERR,NODE:PUENTE,CMD:LEER_RTC,DESC:NUNCA_SE_PUSO_PONGA_LA_HORA");

      } else if (m == SIN_HORA_OSCILADOR_PARADO) {
        // OSF == 1. El literal es el MISMO que usa SET_RTC para este motivo, y se repite
        // a proposito: para quien esta delante del poste es el mismo arreglo, y dos
        // textos distintos para una sola averia se leen como dos averias.
        puente_emitirPropio("$ERR,NODE:PUENTE,CMD:LEER_RTC,DESC:OSCILADOR_PARADO_CAMBIE_PILA");

      } else if (m == SIN_HORA_BUS_MUDO) {
        // El bus no contesta: modulo ausente, mal cableado, SDA y SCL cruzados. Mismo
        // literal que SET_RTC, mismo motivo: es un arreglo con destornillador.
        puente_emitirPropio("$ERR,NODE:PUENTE,CMD:LEER_RTC,DESC:SIN_RELOJ_NO_RESPONDE");

      } else if (m == SIN_HORA_ESCRITURA_A_MEDIAS) {
        // Alguien toco el bus y la escritura no quedo verificada. La duda SE PEGA a
        // proposito -reloj_ds3231.cpp lo razona- y solo la levanta un SET_RTC entero.
        puente_emitirPropio("$ERR,NODE:PUENTE,CMD:LEER_RTC,DESC:ESCRITURA_A_MEDIAS_REPITA_SET_RTC");

      } else if (m == SIN_HORA_FORMATO_12H) {
        // El registro de horas esta en modo 12 h. Es hasta DOCE HORAS de error entregado
        // sin una sola senal de alarma, con el oscilador perfectamente sano. No se le
        // reescribe el bit al chip -seria cambiarle la hora a un equipo de la calle sin
        // que nadie lo pida-: un SET_RTC lo arregla de paso, y eso es lo que se dice.
        puente_emitirPropio("$ERR,NODE:PUENTE,CMD:LEER_RTC,DESC:MODO_12H_PONGA_LA_HORA");

      } else if (m == SIN_HORA_REGISTROS_INCOHERENTES) {
        // 🔴 ESTE DESC NO NOMBRA EL ARREGLO, Y ES DELIBERADO. Los otros seis si lo
        // nombran porque tienen uno solo. Aqui hay DOS -repetir el SET_RTC si fue una
        // escritura cortada, o cambiar el modulo si el chip o el bus devuelven basura- y
        // este firmware no puede distinguirlos. Escribir uno de los dos seria mandar al
        // tecnico a una reparacion elegida a cara o cruz con formato de diagnostico.
        puente_emitirPropio("$ERR,NODE:PUENTE,CMD:LEER_RTC,DESC:REGISTROS_INCOHERENTES");

      } else if (m == SIN_HORA_NINGUNO) {
        // LA CONTRADICCION, Y TIENE RAMA PROPIA PORQUE NO ES "UN MOTIVO MAS".
        //
        // reloj_leer() dijo que no Y la barrera dice que hay hora fiable. Hoy no hay
        // camino que llegue aqui -toda salida negativa de reloj_leer() escribe un motivo
        // antes de volver-, y precisamente por eso: si algun dia se alcanza, lo que esta
        // roto es la barrera, no el reloj, y meterlo en el cajon de "no contemplado"
        // esconderia un defecto del firmware detras de una averia del modulo.
        puente_emitirPropio("$ERR,NODE:PUENTE,CMD:LEER_RTC,DESC:BARRERA_INCOHERENTE");

      } else {
        // Cualquier valor que alguien anada manana al enum y no cablee arriba. Sin esta
        // rama caeria por el else de un `if` que no existe y la consulta se quedaria SIN
        // CONTESTAR: un comando mudo se lee como equipo colgado y el tecnico lo repite.
        puente_emitirPropio("$ERR,NODE:PUENTE,CMD:LEER_RTC,DESC:MOTIVO_NO_CONTEMPLADO");
      }
    }
    return;
  }

  // =================================================================================
  // SET_RTC - D-20 (11/09): SE ATIENDE AQUI Y YA NO SIGUE VIAJE
  // =================================================================================
  // Hasta el 11/09 esta rama ponia el DS3231 y la MISMA linea, con los bytes del
  // telefono, seguia hacia el STM32, que sembraba su hora con ellos. Eso tenia dos
  // defectos medidos (roadmap 3.16-C): el STM32 sembraba y propagaba al Esclavo una hora
  // que el DS3231 acababa de RECHAZAR -ano fuera de rango, OSF-, y el telefono recibia a
  // la vez un $ERR del puente y un "sembrada" del equipo. D-20: la autoridad de la hora
  // es el ESP32. El STM32 recibe la hora RELEIDA del DS3231 por siembra_ahora(), y solo
  // si el DS3231 la acepto.
  //
  // 🔴 Y LO QUE ESO CUESTA, DICHO Y NO ESCONDIDO: el PIN de esta linea ya no lo comprueba
  // NADIE. Antes lo miraba la guarda de PIN del STM32; ahora la linea no llega alli, y el
  // puente no puede mirarlo porque no lo conoce (esp32_09). O sea que con D-20 la hora
  // del CONTROLADOR se puede poner sin PIN escribiendo "SET_RTC:..." por Bluetooth.
  // RIESGO ACEPTADO por el responsable el 11/09 (D-26 (1)): "hace falta tener la app, y la
  // app no la maneja cualquiera que este en la via". esp32_13 lo deja escrito como nota
  // que cita D-26, no como acusacion: el dia que esa fila cambie, la nota tiene a quien
  // apuntar.
  const char* p = accionSetRtc(linea);
  if (p == NULL) {
    // EL PREDICADO SE QUEDO UNA LINEA QUE NINGUNA RAMA RECONOCE. Hoy no hay camino: los
    // tres criterios del predicado tienen su rama arriba. Si alguien anade un cuarto y
    // no le escribe rama, la linea no cruza y sin esto desapareceria sin respuesta.
    puente_emitirPropio("$ERR,NODE:PUENTE,CMD:DESCONOCIDO,DESC:RECLAMADA_SIN_RAMA");
    return;
  }
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

  } else {
    // RELOJ_OK. LA HORA QUE SE DEVUELVE ES LA QUE EL MODULO TIENE, RELEIDA POR LA BARRERA.
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

    } else if (!siembra_ahora()) {
      // D-20 (11/09): LA SIEMBRA INMEDIATA, Y SOLO AQUI -con el DS3231 puesto Y releido-.
      // Una hora que el DS3231 rechazo no llega nunca a esta linea, que es lo que cierra
      // 3.16-C: el STM32 ya no puede quedarse con una hora que el reloj dijo que no.
      //
      // MOTIVO 7, Y ES UN $ACK, NO UN $ERR. La hora entro en el DS3231 y la linea
      // CMD:HORA_ESP32 NO salio entera hacia el STM32 -o la barrera se cayo entre esta
      // relectura y la de siembra_ahora()-, asi que el equipo se quedo con la suya hasta
      // la siembra siguiente. Es medio arreglo, y sin esta rama se leeria como entero.
      //
      // LIMITE DECLARADO: "propagar" es "la linea HORA_ESP32 se puso entera en el
      // cable", no "el STM32 la acepto". Esperar su acuse obligaria a bloquear el bombeo,
      // y un puente que se para a esperar es un puente que deja de pasar telemetria.
      char p[112];
      snprintf(p, sizeof(p),
               "$ACK,NODE:PUENTE,CMD:SET_RTC,RESULT:HORA_PUESTA_SIN_PROPAGAR,"
               "FECHA:%04d-%02d-%02d,HORA:%02d:%02d:%02d",
               leida.anio, leida.mes, leida.dia,
               leida.hora, leida.minuto, leida.segundo);
      puente_emitirPropio(p);

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
