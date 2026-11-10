// ===== src/bluetooth_enlace.inc (ESCLAVO) =====
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

  // N-108 - LA ALARMA LLEVA EL ULTIMO TRAMO, Y EN ESTA PUNTA ES OTRO. Y ESO ES EL DATO.
  //
  // El Maestro puede contar latidos porque el los MANDA: sabe cuantos salieron y cuantos
  // volvieron. Esta punta solo CONTESTA, asi que no tiene ni ventana de latidos ni ida y
  // vuelta que cronometrar. Lo que si tiene, y es real, son los tres contadores de
  // SFTY-15 (protocolo.cpp:101-103), que separan las tres averias que desde el suelo se
  // ven todas igual -"no hay comunicacion"-:
  //
  //   RX en 0                 no llega NADA: cobertura, canal o antena
  //   RX alto y OK en 0       llega BASURA: cableado, linea flotando, radio atascada
  //   los dos suben, RUIDO alto  enlace marginal por distancia o lluvia
  //
  // Ese reparto es exactamente "desde donde se cayo" visto desde este extremo, que es el
  // que se queda sin radio a 5 m de altura y el que hasta hoy no tenia un solo dato.
  //
  // NO SE PUBLICA UN RF% AQUI, Y NO ES POR PEREZA. El unico cociente que se podria formar
  // -OK / (OK + RUIDO)- no es una tasa de perdida, y lo dice el propio protocolo.cpp:120
  // en su nota: "un solo byte de ruido puede provocar varios descartes seguidos mientras
  // la ventana se desplaza. El contador mide RUIDO, no tramas perdidas una a una". Ademas
  // OK cuenta las 3 copias de cada rafaga (SFTY-11), asi que ni el numerador es de
  // mensajes. Un porcentaje sobre eso seria el RF:98% de antes con otra forma: peor, de
  // hecho, porque se moveria y por eso nadie sospecharia de el.
  // ACOTADO DONDE SE PRODUCE (05/09), Y AQUI LA COTA ES EL TIPO -- QUE ES LA RESPUESTA
  // HONESTA, NO LA COMODA.
  //
  // El gemelo del Maestro pedia 52 caracteres en un tramo[40]; este pedia 44 en un
  // tramo[44], que guarda 43. UNO de mas, y nadie lo habia mirado nunca. La cuenta, por
  // buffer y no por rango:
  //
  //   parte fija   14   "RX:" + "," + "OK:" + "," + "RUIDO:"
  //   RX           10   "%lu" de un unsigned long: 4294967295
  //   OK           10
  //   RUIDO        10
  //
  //   14 + 30 = 44 caracteres + NUL = 45 B.
  //
  // AQUI NO HAY COTA QUE DECLARAR, Y ESA ES LA DIFERENCIA CON EL MAESTRO. Alla los tres
  // numeros son medidas con techo -un porcentaje, un RTT por encima del cual ya no hay
  // enlace, un contador que se para en 999- y por eso el arreglo fue una guarda. Estos
  // tres son CONTADORES LIBRES de protocolo.cpp: cuentan bytes y tramas desde el
  // arranque y nadie los detiene. Con 960 B/s el de bytes llega a diez cifras en unos
  // 51 dias, o sea dentro de la vida de un poste. Inventarles un techo para poder
  // estrechar el buffer seria una cota escrita en vez de medida, y el numero real se
  // publicaria como "!" mintiendo sobre que paso.
  //
  // O sea que lo que se acota es el BUFFER a lo que el tipo puede escribir, y se deja
  // dicho por que no hay nada mas que apretar. El coste es 1 B de pila.
  char tramo[45];
  snprintf(tramo, sizeof(tramo), "RX:%lu,OK:%lu,RUIDO:%lu",
           protocolo_bytesRecibidos(), protocolo_tramasValidas(),
           protocolo_tramasDescartadas());

  // Se anota que la caida YA se anuncio, para que la vuelta tenga con que compararse.
  // No decide nada: la decision es de SFTY-6, en main.cpp; aqui solo se toma nota.
  //
  // D-26 (5), 11/09: SOLO SI LA ALARMA ES LA DE LA RADIO. Esta funcion la llaman tambien
  // la camara (botones.cpp) y desde hoy la hora del ESP32, y con la bandera puesta por
  // cualquiera la siguiente trama de radio -con la radio sana llega en segundos- sacaba un
  // $EVENT ENLACE_RF RECUPERADO de una caida que nunca hubo. Con la alarma de la hora
  // repitiendose cada tres cadencias habria sido un falso "radio recuperada" cada 15 min
  // en el diario de este poste. Mismo literal que main.cpp, porque es el mismo evento.
  if (strcmp(evento, "FALLO_RF") == 0) enlaceCaidoAnunciado = true;

  // N-154 CERRADO AQUI (08/09). 152 Y NO 144, Y ES LA UNICA PUNTA DONDE EL BUFFER CRECE.
  //
  // Por buffer -lo unico que snprintf garantiza- el peor caso eran 171 caracteres contra
  // los 143 que un char[144] guarda: se perdian los veintiocho ultimos, o sea el VALOR de
  // la HORA y parte de ACCION, y con el checksum bueno porque se calcula sobre lo que
  // quedo. La alarma llegaba con aspecto de intacta y sin el dato por el que existe una
  // Caja Negra.
  //
  // Primero se acoto donde se PRODUCE, que es lo que manda: los dos causa[] de main.cpp y
  // botones.cpp aportaban 39 caracteres por un numero redondo y bajan a 19. Eso deja el
  // peor caso en 151, y al Maestro le basto para caber en 139. AQUI NO, y el motivo es
  // real y no se puede apretar: el tramo de ESTA punta son 44 caracteres contra los 31
  // del Maestro, porque lleva TRES contadores de protocolo con %lu -numeros libres, sin
  // rango que prometer, acotados a su tope de TIPO, ver la nota de tramo[45] arriba-.
  //
  //   fijo                          49   "$ALARM,NODE:ESCLAVO,EVENTO:" + ",CAUSA:" +
  //                                      "," + ",ACCION:" + ",HORA:"
  //   EVENTO                        10   "CAM_PEGADA"
  //   CAUSA                         19   causa[sizeof("CAM_C_CONTACTO_FIJO")] y el
  //                                      literal "REINTENTOS_AGOTADOS", empatados
  //   tramo                         44   tramo[45]
  //   ACCION                        14   "CAMBIO_A_AMBAR"
  //   HORA                          15   horaBuf[16]
  //
  //   49 + 102 = 151 caracteres + NUL = 152 B. MARGEN CERO y escrito. Los 8 B son de
  //   PILA, no de flash. Y cabe aguas abajo: tramaCompleta[160] guarda 159 y la trama
  //   entera son 151 + "*XX\r\n" = 156.
  //
  // Agrandar un buffer NO es la cura de N-154 y aqui no lo es tampoco: la cura fue acotar
  // los causa[]. Esto es dimensionar a la cota DERIVADA lo que ya no se puede reducir.
  char payload[152];
  snprintf(payload, sizeof(payload), "$ALARM,NODE:ESCLAVO,EVENTO:%s,CAUSA:%s,%s,ACCION:%s,HORA:%s",
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

  // N-114: 112 y no 100, IGUAL QUE EL MAESTRO Y A PROPOSITO. Alli el numero lo obliga
  // el DETALLE de ORIGEN:RELOJ -los bits del RTC, 44 caracteres en su peor caso por
  // tipo-, y esta punta todavia no lo emite porque su reloj.h no declara
  // reloj_diagnostico(). Se sube igual: el dia que lo declare, este emisor tiene que
  // ser el mismo bloque letra por letra, y dos buffers distintos en la misma funcion de
  // las dos puntas son la divergencia que nadie recuerda. D-60: 128 por el REIN de ENLACE_RF.
  char payload[128];
  snprintf(payload, sizeof(payload), "$EVENT,NODE:ESCLAVO,ORIGEN:%s,DETALLE:%s,HORA:%s",
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
// Gemela de la del Maestro, con el porque entero alli. Aqui pesa mas: SIN RADIO la hora de
// esta punta sale SOLO de su ESP32 (D-26 (3)), asi que un J17 suelto la deja extrapolando
// con el HSI -hasta 90 s por hora- justo cuando no hay otra fuente. Y CON radio tambien
// hace falta: el circuito roto hoy es la punta sin hora el dia que la radio caiga.
//
// Una HORA_ESP32 IGNORADA porque manda la radio CUENTA como buena para esta alarma: lo que
// vigila es si la linea LLEGA entera por J17, no si se aplica. Con la radio mandando no se
// parsea -no se aplica-, asi que un formato roto con la radio viva no se ve aqui; se vera en
// la primera siembra sin radio, que es cuando importa.
enum HoraEsp32Estado : uint8_t { HE_SIN_NOTICIAS = 0, HE_IGNORADA, HE_SEMBRADA, HE_ALARMA };
static HoraEsp32Estado horaEsp32Estado = HE_SIN_NOTICIAS;
static bool horaEsp32Llego = false;        // buzon: lo levanta la rama, lo baja el bucle
static bool horaEsp32Rechazada = false;    // la ultima HORA_ESP32 que se parseo no servia
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

// --- El envoltorio que devuelve lo que degradado_salir() no sabe decir ------------
//
// degradado_salir() es `void` Y ABANDONA EN SILENCIO: desde DEG_INACTIVO y desde
// DEG_SALIENDO no hace nada, y desde DEG_RENDIDO solo baja el cartel. Llamarla suelta y
// contestar $ACK detras es el OK mudo que este repositorio persigue -es literalmente lo
// que el banco rechazo del primer intento de N-106-.
//
// Aqui se pregunta la MISMA guarda que ella tiene, no una parecida: modo_degradado.cpp
// arranca la salida en iniciarSalida(false) solo si el estado es DEG_ENTRANDO o
// DEG_ACTIVO. Es el mismo patron que pedirCambioVerificado() del Maestro.
static bool salidaDegradadoIniciada() {
  const EstadoDegradado e = degradado_estado();
  if (e != DEG_ENTRANDO && e != DEG_ACTIVO && e != DEG_ROJO_SIN_HORA) return false;   // D-38
  degradado_salir();
  return true;
}

// --- A-11: EL MODO QUE ESTA PUNTA PUBLICA, QUE HASTA HOY ERA UN LITERAL FIJO -------
//
// El campo MODO: del $STATUS decia SUBORDINADO SIEMPRE, escrito dentro de la plantilla
// del snprintf. Era cierto mientras esta punta no tuviera ningun modo propio, y dejo de
// serlo el 05/09: con el Degradado (hoy por testigo, D-46) el Esclavo puede gobernar su luz
// por reloj y la unica pantalla que lo decia -el menu del LCD- se retiro.
//
// UN BOTON PARA ENTRAR EN UN MODO QUE NO SE PUEDE VER ES MEDIA FUNCION. Y hay tres
// preguntas concretas que el operario no podia contestar desde el telefono: si esta
// punta esta en Degradado, si se rindio a ambar, y si lo que ve en el cruce lo manda el
// Maestro o el reloj de aqui.
//
// POR QUE TRES LITERALES Y NO CINCO, uno por estado del enum:
//
//   DEG_ENTRANDO / DEG_ACTIVO / DEG_SALIENDO -> "DEGRADADO". Son los tres en los que
//   degradado_gobiernaLuz() es cierta, o sea en los que la luz NO la manda el Maestro,
//   que es lo que este campo contesta. En que tramo del modo va se lee en ESTADO:, que
//   ya viaja al lado y dice ROJO en los dos todo-rojos.
//
//   DEG_RENDIDO -> "RENDIDO", y va aparte porque NO es lo mismo: el modo TERMINO SOLO
//   y la luz quedo en ambar intermitente. Pintarlo como DEGRADADO diria que el cruce
//   sigue operando por reloj cuando ya no opera.
//
//   Desde D-38 solo lleva ahi el limite de 48 h. La hora no fiable va a DEG_ROJO_SIN_HORA,
//   que publica DEGRADADO (la luz, roja, la sigue gobernando el modo) y su $ALARM.
//
// EL COSTE EN BYTES ES CERO, Y ESTA MEDIDO POR BUFFER (CLAUDE.md 7.bis): el literal mas
// largo que puede salir de aqui es "SUBORDINADO", 11 caracteres, que son exactamente
// los que ocupaba dentro de la plantilla. El peor $STATUS se queda en los mismos 133
// caracteres de antes. "DEGRADADO" son 9 y "RENDIDO" 7: los dos casos nuevos son mas
// cortos que el que ya estaba, asi que el peor caso no lo fijan ellos.
//
// EL `default:` NO SE PUEDE PEDIR COMO ORDEN, y lo comprueba app_02_modos_simetricos:
// "DESCONOCIDO" es lo que la telemetria dice cuando le llega un valor del enum que no
// sabe nombrar, no un modo al que se pueda ir.
static const char* obtenerNombreModo(EstadoDegradado e) {
  switch (e) {
    case DEG_INACTIVO: return "SUBORDINADO";
    case DEG_ENTRANDO: return "DEGRADADO";
    case DEG_ACTIVO:   return "DEGRADADO";
    case DEG_SALIENDO: return "DEGRADADO";
    case DEG_ROJO_SIN_HORA: return "DEGRADADO";   // D-38: la luz la sigue gobernando el modo
    case DEG_RENDIDO:  return "RENDIDO";
    default:           return "DESCONOCIDO";
  }
}
