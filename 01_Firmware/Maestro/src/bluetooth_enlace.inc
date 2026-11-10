// ===== src/bluetooth_enlace.inc (MAESTRO) =====
// Lo que este equipo cuenta por su cuenta: $ALARM y $EVENT, los silencios de J17, la hora del ESP32 y los avisos
// del reloj, con los ayudantes que usa el despachador.
//
// NO SE COMPILA SOLO: lo incluye bluetooth.cpp en su sitio, dentro de su unidad de compilacion, y por eso usa sus
// static. Salio de bluetooth.cpp por el tope de 500 lineas (roadmap, "Partir"), literal; los instrumentos lo leen
// dentro de bluetooth.cpp, en su sitio (banco/fuente.py, expandir_inc; CLAUDE.md 5).
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
  //
  // ACOTADO DONDE SE PRODUCE (05/09). ES N-154 OTRA VEZ, EN PEQUENO Y EN UN BUFFER
  // INTERNO.
  //
  // Aqui ponia tramo[40] con los tres numeros a su tope de TIPO -"%d" de un int son 11
  // caracteres y "%lu" de un unsigned long son 10-, y con los 17 de la parte fija la
  // cuenta daba 52 contra los 39 que un char[40] guarda: TRECE DE MAS. No truncaba en
  // la calle porque los valores reales son cortos, que es la definicion de una barrera
  // que no protege de nada.
  //
  // POR QUE ES MENOS GRAVE QUE EL DE LA TRAMA, Y CONVIENE SABER POR QUE LO ES: lo que
  // se corta es este buffer, no el payload. El $ALARM sale bien formado y con su
  // checksum bueno, asi que la alarma LLEGA; lo que se queda por el camino es el final
  // del tramo -el SINRESP-, que es justo el numero que separa "lleva tres latidos sin
  // contestar" de "lleva doscientos". No se pierde la alarma: se pierde el dato por el
  // que la alarma se anade.
  //
  // LAS TRES COTAS YA ESTABAN DECLARADAS EN coordinador.h Y NINGUNA SE EJERCIA AQUI
  // (2.ter, DECLARAR NO ES EJERCER). Son las mismas que el $STATUS ya compara, y se usa
  // el mismo marcador y con el mismo significado: "--" es "todavia no lo se" y "!" es
  // "llego un valor que no puede ser". Nunca un numero aplastado.
  const int rf = coordinador_calidadEnlace();
  const unsigned long rtt = coordinador_tiempoRespuestaMs();
  const int sinResp = coordinador_latidosSinRespuesta();

  char rfTxt[5];    // "100%"    + NUL, garantizado por la guarda de CALIDAD_ENLACE_MAX
  char rttTxt[8];   // "25000ms" + NUL, por la de RTT_PUBLICABLE_MAX_MS
  char srTxt[4];    // "999"     + NUL, por la de LATIDOS_SIN_RESPUESTA_MAX
  if (rf < 0) {
    strncpy(rfTxt, "--", sizeof(rfTxt));
    strncpy(rttTxt, "--", sizeof(rttTxt));
  } else {
    if (rf > CALIDAD_ENLACE_MAX) {
      strncpy(rfTxt, "!", sizeof(rfTxt));
    } else {
      snprintf(rfTxt, sizeof(rfTxt), "%d%%", rf);
    }
    if (rtt > RTT_PUBLICABLE_MAX_MS) {
      strncpy(rttTxt, "!", sizeof(rttTxt));
    } else {
      snprintf(rttTxt, sizeof(rttTxt), "%lums", rtt);
    }
  }
  // El "< 0" no es adorno: el contador es un int y un negativo no es "no hay dato", es
  // un numero imposible. Se separa del camino normal por lo mismo que en el campo T:
  // del $STATUS -significan cosas distintas y el tecnico tiene que poder verlas
  // distintas-.
  if (sinResp < 0 || sinResp > LATIDOS_SIN_RESPUESTA_MAX) {
    strncpy(srTxt, "!", sizeof(srTxt));
  } else {
    snprintf(srTxt, sizeof(srTxt), "%d", sinResp);
  }

  // EL PEOR CASO DEL TRAMO, POR BUFFER Y NO POR RANGO -lo unico que snprintf garantiza:
  //
  //   parte fija   17   "RF:" + "," + "RTT:" + "," + "SINRESP:"
  //   RF            4   rfTxt[5]
  //   RTT           7   rttTxt[8]
  //   SINRESP       3   srTxt[4]
  //
  //   17 + 14 = 31 caracteres + NUL = 32 B, que es exactamente lo que guarda tramo[32].
  //   Margen CERO, y esta escrito: no es holgura olvidada, es la cuenta cuadrada. Si
  //   alguien ensancha uno de los tres buffers o anade un campo, el pack falla.
  char tramo[32];
  snprintf(tramo, sizeof(tramo), "RF:%s,RTT:%s,SINRESP:%s", rfTxt, rttTxt, srTxt);

  // N-154 CERRADO AQUI (08/09). 139 Y NO 144, Y LA CUENTA VA ESCRITA PORQUE EL MARGEN
  // ES CERO A PROPOSITO.
  //
  // Lo que habia era 144 con una cuenta hecha sobre los VALORES que se pasan hoy -"son
  // 128 B"-, no sobre los BUFFERS. Por buffer, que es lo unico que snprintf garantiza,
  // el peor caso eran 158 caracteres contra los 143 que un char[144] guarda: se perdian
  // los quince ultimos, o sea EL VALOR DE LA HORA. Y con el checksum bueno, porque se
  // calcula sobre lo que quedo: la alarma llegaba entera de aspecto y sin el unico dato
  // por el que existe una Caja Negra.
  //
  // El arreglo NO fue agrandar esto: fue acotar los dos causa[] donde SE PRODUCEN
  // -coordinador.cpp y botones.cpp-, que aportaban 39 caracteres por un numero redondo.
  // Con eso el peor caso baja a 138, y este buffer se dimensiona a esa cuenta:
  //
  //   fijo                          49   "$ALARM,NODE:MAESTRO,EVENTO:" + ",CAUSA:" +
  //                                      "," + ",ACCION:" + ",HORA:"
  //   EVENTO                        10   "CAM_PEGADA", el mas largo de los tres literales
  //   CAUSA                         19   causa[sizeof("CAM_C_CONTACTO_FIJO")] y el
  //                                      literal "REINTENTOS_AGOTADOS" empatan en 19
  //   tramo                         31   tramo[32]
  //   ACCION                        14   "CAMBIO_A_AMBAR"
  //   HORA                          15   horaBuf[16]
  //
  //   49 + 89 = 138 caracteres + NUL = 139 B. MARGEN CERO, y esta escrito: no es holgura
  //   olvidada, es la cuenta cuadrada. Quien rehace esta suma en cada corrida leyendo
  //   los literales de los llamadores es esp32_07_presupuesto_bytes; si alguien anade un
  //   campo o alarga un literal, falla ahi antes de truncar en la calle.
  char payload[139];
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
  // pila, no de flash; el presupuesto de J17 los recalcula. D-60: 128 por REIN; D-92: 144 por AMBAR.
  char payload[144];
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
// AB-1: el silencio por debajo de esto es el reposo normal del latido del puente, no un
// corte. Es LATIDO_MS x 1,5 -contrato.h del ESP32, hoy 2000 ms-, y hay un pack que
// recalcula los dos y falla si divergen: un umbral que no salga del periodo del latido
// deja de significar lo que su nombre dice.
static const unsigned long J17_SILENCIO_MIN_MS = 3000UL;

static void j17RegistrarLinea(unsigned long ahora) {
  const unsigned long silencio = ahora - tUltimaLineaJ17;
  tUltimaLineaJ17 = ahora;
  if (silencio > j17SilencioMaxMs) j17SilencioMaxMs = silencio;
  j17Silencios++;

  // AB-1 - AQUI HABIA "SIN UMBRAL, Y ES LA DECISION DE DISENO". SE INVIERTE, Y EL
  // MOTIVO NO ES QUE AQUELLA FUERA MALA: ES QUE EL LATIDO CAMBIO LO QUE MIDE ESTO.
  //
  // Sin latido, cada linea que llegaba era un dedo en la app y un silencio era un dato
  // en si mismo: "este puerto llevaba catorce horas sin que nadie hablara". Publicar
  // todos era correcto, y poner un umbral habria sido un numero que nadie habia decidido
  // gobernando lo que ve el tecnico.
  //
  // Con latido cada LATIDO_MS el puerto habla SOLO, asi que un silencio de un latido no
  // significa absolutamente nada: es el reposo. A 2 s serian 1.800 lineas identicas por
  // hora en la misma bitacora donde hay que encontrar el fallo de campo. Es N-73 por
  // INUNDACION en vez de por filtro: el registro existe y deja de poder leerse.
  //
  // EL UMBRAL NO ES ARBITRARIO Y POR ESO SE PUEDE ESCRIBIR. Sale del periodo del latido,
  // no del gusto de nadie: LATIDO_MS x 1,5. Por debajo esta el hueco normal -un latido-;
  // por encima, algo se perdio. Con LATIDO_MS = 2000 el corte se pone en 3000 ms, y un
  // ciclo de perro entero -2000 de watchdog mas el arranque- cae siempre por encima.
  //
  // ⚠️ ES UN GEMELO EN OTRO LENGUAJE, y este repositorio ya sabe como acaban: si alguien
  // cambia LATIDO_MS en el contrato.h del ESP32 y no toca este numero, el umbral deja de
  // significar lo que dice. Hay un pack que recalcula los dos y falla si divergen.
  //
  // LOS CONTADORES SIGUEN CONTANDO TODO: lo que se acota es lo que se PUBLICA. j17Silencios
  // y j17SilencioMaxMs se actualizan arriba con cada linea, latido incluido, porque son
  // los que contestan "cuanto llevaba caido" cuando el tecnico llegue.
  if (silencio >= J17_SILENCIO_MIN_MS) {
    char det[48];
    snprintf(det, sizeof(det), "MUDO:%lus,MAX:%lus,N:%lu",
             silencio / 1000UL, j17SilencioMaxMs / 1000UL, j17Silencios);
    bluetooth_reportarEvento("J17", det);
  }
}

// ---------------------------------------------------------------------------
// D-26 (5) - LA SEGUNDA ALARMA: LA HORA DEL ESP32 NO LLEGA, O LLEGA Y NO SIRVE.
//
// Son dos averias distintas de la de la radio y por eso otra alarma (D-26): esta es de la
// MISMA placa -el ESP32 del gabinete, su DS3231 y el cable J17- y lo que pide es "revisar
// el circuito", no ir al otro poste. Sin ella, un J17 suelto dejaba a este micro
// extrapolando con el HSI -hasta 90 s por hora- sin que nadie lo supiera.
//
// TRES CAUSAS, CADA UNA CON SU LITERAL EN SU RAMA (N-89: el pack que las mira las busca
// dentro de la rama que decide, no en un compositor):
//   RECHAZADA_FORMATO   llego una HORA_ESP32 que el sembrador tiro -forma o rango-. Se
//                       dice en el acto, en la rama del despachador.
//   J17_MUDO            no llega NADA por J17, ni el latido: el cable o el ESP32 muerto.
//   SIN_HORA_DEL_ESP32  el latido si llega y la hora no: el DS3231 del ESP32 no tiene hora
//                       fiable (no siembra sin ella), o el ESP32 lleva un firmware sin
//                       siembra. Se distingue de la anterior con el registro de J17 de
//                       arriba y su mismo umbral: sin eso las dos mandarian a mirar lo
//                       mismo y son dos arreglos distintos.
// ACCION es lo que el equipo HACE, como en las otras alarmas: sigue con su hora.
//
// SE REPITE cada HORA_ESP32_ESPERA_MAX_MS mientras dure (reloj.h). No es el rechazo que
// nadie puede apagar (CLAUDE.md 1): se apaga arreglando el circuito, y la siguiente siembra
// buena lo dice en el diario. Cuatro por hora como mucho.
//
// LA RAMA DEL DESPACHADOR NO LLAMA A millis(): deja su veredicto en estas variables y el
// reloj lo pone bluetooth_loop(), que ya lo tiene. Asi la rama sigue siendo lo que
// reloj_02 le exige -todo lo que hace, gobernado por lo que devolvio la siembra-.
enum HoraEsp32Estado : uint8_t { HE_SIN_NOTICIAS = 0, HE_SEMBRADA, HE_ALARMA };
static HoraEsp32Estado horaEsp32Estado = HE_SIN_NOTICIAS;
static bool horaEsp32Llego = false;        // buzon: lo levanta la rama, lo baja el bucle
static bool horaEsp32Rechazada = false;    // la ultima HORA_ESP32 que llego no servia
static unsigned long tUltimaHoraEsp32 = 0; // ultima buena, o ultima alarma repetida

static void horaEsp32Vigilar(unsigned long ahora) {
  if (horaEsp32Llego) {
    horaEsp32Llego = false;
    tUltimaHoraEsp32 = ahora;
  }
  if (ahora - tUltimaHoraEsp32 <= HORA_ESP32_ESPERA_MAX_MS) return;

  tUltimaHoraEsp32 = ahora;   // la siguiente, dentro de otra espera entera
  horaEsp32Estado = HE_ALARMA;
  if (horaEsp32Rechazada) {
    bluetooth_reportarAlarma("HORA_ESP32", "RECHAZADA_FORMATO", "SIGUE_SU_HORA");
  } else if (ahora - tUltimaLineaJ17 >= J17_SILENCIO_MIN_MS) {
    bluetooth_reportarAlarma("HORA_ESP32", "J17_MUDO", "SIGUE_SU_HORA");
  } else {
    bluetooth_reportarAlarma("HORA_ESP32", "SIN_HORA_DEL_ESP32", "SIGUE_SU_HORA");
  }
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
// N-151 (05/09): DAR PASO EN UN MODO DONDE EL COORDINADOR NO CORRE DEJABA EL CRUCE
// TRABADO PARA SIEMPRE. Lo destapo una cinta, y el sintoma no se parecia a la causa.
//
// EN LA CINTA: el equipo en MODO:AMBAR, y tres MANUAL:CAMBIAR_TURNO seguidos en 40 s
// contestados "$ERR,CMD:CAMBIAR_TURNO,DESC:EN_TRANSICION_REINTENTE". El operario leia
// "el cruce esta cambiando de fase, repita al terminar" y el cambio no terminaba nunca.
//
// POR QUE: main.cpp EXCLUYE a MODO_AMBAR y a MODO_DEGRADADO del refresco de fondo -en
// esos dos modos el Maestro calla en la radio A PROPOSITO- y sus loop() no llaman al
// coordinador. O sea que alli la maquina del coordinador ESTA CONGELADA.
//
// La PRIMERA pulsacion si entraba -estadoC valia C_IDLE- y dejaba el coordinador en un
// estado de transicion. Y ese estado YA NO AVANZA, porque no hay quien lo haga avanzar.
// A partir de ahi, todas las demas caen en el "if (estadoC != C_IDLE) return;" de
// pedirCambio() y contestan EN_TRANSICION_REINTENTE HASTA QUE ALGUIEN CAMBIE DE MODO.
// El mensaje era ademas mentiroso: no habia ninguna transicion en curso.
//
// Es la barrera de salidas (CLAUDE.md 6) en su forma mas cara: el equipo dijo que SI a
// una orden que no iba a ejecutar, y encima se quedo peor que antes de pedirla. La
// guarda de abajo se pregunta lo unico que decide -si en este modo alguien mueve el
// coordinador- y contesta con un motivo que es verdad.
static bool modoMueveElCoordinador() {
  // La lista es la MISMA de main.cpp, y por eso se escribe aqui su porque en vez de
  // repetir la condicion: si manana un tercer modo deja de llamar al coordinador, este
  // sitio hay que tocarlo. Lo vigila maestro_12_dar_paso_sin_coordinador.
  const ModoSistema m = modoActual_get();
  return m != MODO_AMBAR && m != MODO_DEGRADADO;
}

static bool pedirCambioVerificado() {
  if (!coordinador_listoParaContar()) return false;
  coordinador_pedirCambio();
  return true;
}

// --- LOS BITS DEL RELOJ, POR EL UNICO CAMINO QUE QUEDA ABIERTO --------------------
//
// N-114 - DOS $ERR DE ESTE FICHERO MANDAN AL TECNICO A UNA PANTALLA TAPIADA.
//
// SIN_CRISTAL_VEA_CONSULTA_RELOJ y SIGUE_PARADO_VEA_CONSULTA_RELOJ nombran la consulta
// de N-45 por su nombre. Esa consulta era MODO_HORA, y D-44 la retiro del firmware con su
// unico armador (la navegacion del menu); por Bluetooth no existio nunca SET_MODO:HORA.
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
// LIMITE DECLARADO: esto NO es una consulta bajo demanda. La segunda visita que pide
// reloj.h -"cnt cambiando entre dos visitas -> el RTC CUENTA"- se hacia repitiendo
// SET_RTC; desde D-15 esa rama ya no emitia los bits, y desde el 11/09 SET_RTC ni
// siquiera llega a esta punta -es del puente-. La unica puerta que queda es
// REINICIAR_RELOJ, y esa BORRA el respaldo: la segunda visita no tiene camino inocuo.
//
// EL DETALLE NO LLEVA NI UNA COMA, Y NO ES ESTILO. _camposNmea() de la app parte la
// trama por ',' y cada trozo por su PRIMER ':' (app.js:1798-1810), asi que una coma
// dentro del DETALLE convierte todo lo que va detras en campos sueltos que el pintor de
// $EVENT no mira: los bits saldrian al cable y no llegarian a la pantalla. Seria la
// prueba muerta con forma de telemetria. Se separan con espacio, que es lo que ya hace
// el DESC de SET_MODO:DEG_T. Lo vigila reloj_01_consulta_por_bluetooth.
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

// 1.49(a) - LA ORDEN REINICIAR_RELOJ ESPERANDO SU VEREDICTO. La pone la rama del despachador
// cuando el oscilador arranca tras el reinicio, y la baja atenderVeredictoReloj() en la
// vuelta en que reloj_estadoCristal() deja de decir VIGILANDO. Una sola bandera y una sola
// respuesta por orden: el porque, en la rama.
static bool pendienteReloj = false;

// El veredicto, desde bluetooth_loop(). Dos salidas y ninguna mas, las dos con su trama:
//   CUENTA     -> el $ACK que antes salia en el acto, ahora con la medida detras.
//   CONGELADO  -> $ERR con la marca VEA_CONSULTA_RELOJ Y LOS BITS en la misma rama, como la
//                 otra puerta (reloj_01): el oscilador arranco y el contador no se movio, y
//                 los bits -RDY en 1, CNT quieto- son justo los que lo dicen.
//   SIN_CRISTAL -> 1.49b3: el $ERR de "no arranco", con sus bits. Ver la rama.
// Mientras sea VIGILANDO no contesta nada: la ventana la cierra vigilarCristal() en cada
// reloj_actualizar(), asi que esto no espera mas de CNT_VENTANA_MS y una vuelta.
static void atenderVeredictoReloj() {
  if (!pendienteReloj) return;
  const EstadoCristal e = reloj_estadoCristal();
  if (e == RELOJ_CRISTAL_CUENTA) {
    pendienteReloj = false;
    enviarTramaConCrc("$ACK,CMD:REINICIAR_RELOJ,RESULT:CRISTAL_OK_PONGA_LA_HORA");
  } else if (e == RELOJ_CRISTAL_CONGELADO) {
    pendienteReloj = false;
    enviarTramaConCrc("$ERR,CMD:REINICIAR_RELOJ,DESC:ARRANCA_Y_NO_CUENTA_VEA_CONSULTA_RELOJ");
    reportarBitsDelReloj();
  } else if (e == RELOJ_CRISTAL_SIN_CRISTAL) {
    // 1.49b3: con la orden pendiente el oscilador YA arranco (la rama solo la deja pendiente
    // si reloj_reiniciarDominioRespaldo() dio true), y hoy nadie baja rtcOperativo sin el
    // cerrojo: esto no deberia verse. Si se ve, NO es un OK ni se queda sin respuesta: es el
    // mismo $ERR que la rama da cuando el oscilador no arranca, con sus bits.
    pendienteReloj = false;
    enviarTramaConCrc("$ERR,CMD:REINICIAR_RELOJ,DESC:SIGUE_PARADO_VEA_CONSULTA_RELOJ");
    reportarBitsDelReloj();
  }
}
