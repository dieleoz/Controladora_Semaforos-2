// ===== src/botones_vigilante.inc =====
// EL VIGILANTE DE LAS CAMARAS DE J16 (D-13 fase 1, D-33): su estado, sus dos alarmas, la presencia y el contador
// de vetos de la pluma. IDENTICO EN LAS DOS PUNTAS (camara_02_j16, camara_03_vigilante).
//
// NO SE COMPILA SOLO: lo incluye botones.cpp en su sitio, dentro de su unidad de compilacion. Los static de aqui
// siguen siendo de botones.cpp y J16 sigue teniendo un solo dueno: ni un pinMode ni un digitalRead viven en este
// fichero. Salio de botones.cpp por el tope de 500 lineas (roadmap, "Partir"), literal; los instrumentos lo leen
// dentro de botones.cpp, en su sitio (banco/fuente.py, expandir_inc; CLAUDE.md 5).
// Y LOS CINCO DIGITOS QUE CABEN EN EL CAMPO DE SEGUNDOS DE detalle[] (N-154).
//
// NO es un limite del veto -un veto dura lo que dure-: es lo que el BUFFER admite. Los
// demas %u de este fichero salen de un uint16_t y no pueden dar mas de cinco digitos;
// este sale de un RELOJ, y una pluma retenida 18,2 h ya da seis. Por encima se publica
// "!" en vez de un numero recortado. El buffer no se agranda (CLAUDE.md 10).
static const unsigned long VETO_SEG_PUBLICABLE_MAX = 65535UL;

// EL ORDEN DE LOS VALORES ES LA GRAVEDAD, y por eso el campo CAM: se resuelve con un
// simple mayor-que en vez de con una cadena de ifs que alguien tendria que mantener
// ordenada. "?" pesa mas que "OK" a proposito; el porque, en botones.h.
enum EstadoCamara { CAM_OK, CAM_DESCONOCIDA, CAM_CIEGA, CAM_PEGADA };

static EstadoCamara camEstado[2] = {CAM_DESCONOCIDA, CAM_DESCONOCIDA};
static unsigned long camAltoDesde[2] = {0, 0};
static unsigned long camSinFlancoMs[2] = {0, 0};
static unsigned long camUltimoFlanco[2] = {0, 0};
static bool camHuboFlanco[2] = {false, false};
static unsigned long camTickAnt = 0;
// D-33: la bandera de flanco del contador ya no es la de la pluma -esa transicion dejo
// de ocurrir el dia que el veto existe- sino la del VETO. Sigue habiendo una sola, y con
// ella el cronometro del veto en curso y los avisos ya dados, que son lo que impide que
// el aviso periodico se repita en cada vuelta del loop.
static bool camVetoAnt = false;
static unsigned long camVetoDesde = 0;
static uint16_t camVetoAvisos = 0;
static uint16_t camVetos = 0;

// Los dos nombres que salen al aire, indexados por la MISMA i que CAM_J16[]. Se
// escriben una vez en vez de repetirse dentro de cada snprintf, por el mismo motivo por
// el que CAM_J16[] es una sola tabla: dos listas paralelas acaban desalineadas y la
// alarma sale culpando a la camara de al lado.
static const char* const CAM_NOMBRE[2] = {"CAM_C", "CAM_D"};

const char* camara_estado() {
  // LA PEOR DE LAS DOS, PERO SOLO ENTRE LAS QUE TIENEN ALGO QUE DECIR.
  //
  // ~~Con UNA CAMARA POR POSTE (D-13) el otro pin esta vacio~~ -> CADUCADO el 11/09: la fila
  // de DECISIONES.md de ese dia sobre las conexiones del Sisga deja DOS camaras por poste,
  // una en cada pin. (Su numero no se escribe aqui A PROPOSITO: decisiones_01 lo contaria
  // como ancla, y esta linea no construye esa decision, solo dice que la regla de abajo
  // perdio su motivo.) La regla se escribio para un pin vacio a proposito; se deja tal cual
  // porque NO se toca aqui -rehacerla es la exencion que esa fila deja pendiente-. Lo cierto
  // es la mecanica: un pin sin camara, o con una muerta desde la instalacion, arranca en
  // CAM_DESCONOCIDA y no sale de ahi nunca -no puede dar un flanco-. Con el mayor-que a
  // secas eso significaba que "?" pesaba mas que "OK" para siempre y EL CAMPO CAM: NO
  // PODIA DECIR "OK" EN NINGUN EQUIPO REAL. Un campo que solo sabe decir una cosa es el
  // enum de un solo valor de 3.septies con otra ropa.
  //
  // La regla es la misma que la del silencio en vigilante_tick(), y por eso se apoya en
  // la MISMA bandera y no en una segunda condicion que alguien tendria que mantener
  // igual: SIN FLANCO NO SE JUZGA EL SILENCIO. Pero el NIVEL si se juzga siempre -PEGADA
  // no necesita flanco, se siembra en camaras_sembrar()-, asi que un contacto trabado en
  // un pin que nunca dio un flanco SIGUE SALIENDO al aire.
  //
  // Y MIENTRAS NINGUNA HAYA DICHO NADA, "?" - no "OK". El "?" honesto de A-6 se conserva
  // entero para el unico instante en que de verdad no se sabe nada: entre el arranque y
  // la primera deteccion. Contestar "OK" ahi seria pintar un dato que no se tiene, que es
  // justo lo que este repositorio le quito a la app.
  //
  // D-166 (SPEC_8 9.5, Q6): Y SOLO ENTRE LAS NO INHIBIDAS. El campo dice lo que actua sobre la pluma; una inhibida
  // CIEGA no tapa a la otra. Con las dos inhibidas no queda ninguna: "?", y interruptores_camNoVeto() dice NOVETO.
  EstadoCamara peor = CAM_DESCONOCIDA;
  bool juzgable = false;
  for (int i = 0; i < 2; i++) {
    if (respaldo_vetoCamaraOff(i) || (!camHuboFlanco[i] && camEstado[i] != CAM_PEGADA)) {
      continue;
    }
    if (!juzgable || camEstado[i] > peor) {
      peor = camEstado[i];
    }
    juzgable = true;
  }
  if (!juzgable) {
    return "?";
  }
  switch (peor) {
    case CAM_PEGADA: return "PEGADA";
    case CAM_CIEGA:  return "CIEGA";
    default:         return "OK";
  }
}

uint16_t camara_vetosPluma() {
  return camVetos;
}

// D-166/D-169 (SPEC_8 9.1): la mascara de los tres consumidores. Lee el estado del vigilante tal cual: una mala se
// recupera sola (RECUPERADA) y vuelve a mirar en ese mismo tick. El porque de cada termino, en botones.h.
bool camara_mira(int i) {
  return !respaldo_vetoCamaraOff(i) && camEstado[i] != CAM_PEGADA && camEstado[i] != CAM_CIEGA;
}

// D-169 (SPEC_8 9.2, Q3): las inhibidas no cuentan ni como malas ni como "queda una": con una inhibida y otra
// mala no queda ninguna que mire y rige MALA2; con las dos inhibidas no hay mala y la pluma baja (D-96).
bool camara_malaRetienePluma() {
  bool hayMala = false, quedaUna = false;
  for (int i = 0; i < 2; i++) {
    if (respaldo_vetoCamaraOff(i)) continue;
    if (camara_mira(i)) {
      quedaUna = true;
    } else {
      hayMala = true;
    }
  }
  return hayMala && !respaldo_malaBaja(quedaUna ? 1 : 2);
}

// D-166: misma regla de juzgable que camara_estado(), por camara y sin mirar la inhibicion.
char camara_letra(int i) {
  if (camEstado[i] == CAM_PEGADA) return 'P';
  if (!camHuboFlanco[i]) return '?';
  return camEstado[i] == CAM_CIEGA ? 'C' : 'O';
}

// La alarma de una camara. Las dos que hay salen por esta puerta para que el peor caso
// de bytes sea UNO SOLO y se pueda medir: el $ALARM lleva ademas el tramo del enlace y
// la hora, y N-108 costo una trama truncada por no haber hecho antes esta cuenta. El
// pack la rehace leyendo estos literales y el snprintf de bluetooth.cpp.
//
// ACCION NO ES RELLENO: es la medida de seguridad vial EJECUTADA. ~~La fase 1 no ejecuta ninguna~~ -> desde
// D-169 si (SPEC_8 9.3): la camara mala deja de vetar y de pedir verde (IGNORA_CAMARA) o, con su MALAx en ARRIBA,
// la pluma se queda arriba en el rojo (PLUMA_ARRIBA); una inhibida ya estaba ignorada (NINGUNA). Se lee DESPUES
// de poner camEstado[i], con el parametro de ese instante. Escribir otra cosa seria el $ACK que no mira lo que
// devolvio la llamada (CLAUDE.md 2), en la caja negra. La MAS LARGA VA PRIMERO: el pack del vigilante lee la
// primera de las tres para el presupuesto de bytes de N-108.
static void camara_alarmar(int i, const char* evento, const char* motivo) {
  // N-154: la cota sale de los literales que de verdad entran, no de un numero redondo.
  // Este buffer va ENTERO dentro del payload del $ALARM, y con char[28] aportaba 27 al
  // peor caso -por buffer, que es lo unico que snprintf garantiza-. El peor real es
  // CAM_NOMBRE[i] + '_' + el motivo mas largo: "CAM_C" + "_" + "CONTACTO_FIJO" = 19.
  // No se deriva con un sizeof compuesto porque el motivo llega por parametro: quien
  // rehace la cuenta en cada corrida leyendo estos literales es el pack del presupuesto
  // de bytes, y si alguien anade un motivo mas largo, falla ahi.
  char causa[sizeof("CAM_C_CONTACTO_FIJO")];
  snprintf(causa, sizeof(causa), "%s_%s", CAM_NOMBRE[i], motivo);
  // Sin llaves a proposito: el pack del presupuesto de bytes busca causa[] en el MISMO bloque que la llamada.
  const bool inhibida = respaldo_vetoCamaraOff(i);
  if (!inhibida && !camara_malaRetienePluma()) bluetooth_reportarAlarma(evento, causa, "IGNORA_CAMARA");
  else if (!inhibida) bluetooth_reportarAlarma(evento, causa, "PLUMA_ARRIBA");
  else bluetooth_reportarAlarma(evento, causa, "NINGUNA");
}

// Una alarma que no se cierra nunca deja al operario sin saber si aquello se arreglo:
// las dos vuelven a OK por su prueba contraria, y las dos lo dicen.
static void camara_recuperada(int i) {
  char detalle[24];
  snprintf(detalle, sizeof(detalle), "%s_RECUPERADA", CAM_NOMBRE[i]);
  bluetooth_reportarEvento("CAMARA", detalle);
}

// UN FLANCO ES LA UNICA PRUEBA POSITIVA DE QUE LA CAMARA VE. Reinicia su cronometro de
// silencio y, si estaba anunciada CIEGA, la devuelve a OK con su evento.
//
// AQUI EL ESTADO NO PUEDE SER PEGADA, y por eso no se pregunta por el: un flanco es una
// subida, o sea que el contacto tuvo que ABRIRSE antes, y abrirse es justo lo que cierra
// PEGADA en vigilante_nivel(). Preguntarlo seria una guarda que no puede ser falsa
// (CLAUDE.md 3.septies).
static void vigilante_flanco(int i, unsigned long ahora) {
  camUltimoFlanco[i] = ahora;
  camHuboFlanco[i] = true;
  camAltoDesde[i] = ahora;
  camSinFlancoMs[i] = 0;
  if (camEstado[i] == CAM_CIEGA) {
    camara_recuperada(i);
  }
  camEstado[i] = CAM_OK;
}

// EL NIVEL SOSTENIDO. Es la unica de las dos alarmas que puede dispararse SIN que la
// camara haya dado nunca un flanco -un contacto que ya venia cerrado al encender-, y
// por eso su cronometro se siembra en camaras_sembrar() y no aqui: sin esa siembra, el
// rele trabado desde ANTES del arranque seria precisamente el unico que no se detecta.
// Es N-26 aplicado al vigilante.
static void vigilante_nivel(int i, bool alto, unsigned long ahora) {
  if (!alto) {
    if (camEstado[i] == CAM_PEGADA) {
      camEstado[i] = CAM_OK;
      camara_recuperada(i);
    }
    return;
  }
  if ((ahora - camAltoDesde[i]) >= CAM_PEGADA_MS && camEstado[i] != CAM_PEGADA) {
    camEstado[i] = CAM_PEGADA;
    camara_alarmar(i, "CAM_PEGADA", "CONTACTO_FIJO");
  }
}

// QUE ES "HAY PRESENCIA" EN UNA CAMARA DE J16. UNA SOLA DEFINICION, Y AQUI.
//
// ~~Lo contestan dos consumidores... el contador de vetos de la fase 2 -abajo, en
// vigilante_tick()- y, en el Maestro, camara_presenciaJ16()~~ -> ACTUALIZADO POR D-33
// (14/09/2026). El contador ya no pregunta aqui: lee la decision que tomo escribirPines()
// (ver vigilante_tick()). El unico llamador es camara_presenciaJ16(), y por el pasan
// ahora DOS preguntas distintas hechas por motivos distintos:
//
//   EL MODO INTELIGENTE, solo en el Maestro: "hay cola AHORA" para sostener una fase.
//   EL VETO DE LA PLUMA, en las dos puntas: "hay alguien debajo" para no bajar la
//   barrera (D-33). Es la razon de que el getter haya dejado de ser del Maestro solo.
//
// Escribir la condicion dos veces seria el defecto que este repositorio ya paga en otros
// sitios: dos formulas que solo la disciplina mantiene iguales. Y ahora pesa mas que
// ayer, porque una de las dos copias decidiria si una barrera fisica baja sobre alguien.
// El pack camara_03_vigilante comprueba que sigue habiendo UNA.
//
// SON DOS TERMINOS Y HACEN FALTA LOS DOS:
//
//   camAnt[i] - EL NIVEL de esta misma vuelta. Contesta al contacto que esta cerrado
//        AHORA. Lo lee camaras_actualizar() una vez por iteracion; no se relee aqui,
//        porque una segunda lectura seria otro instante y ademas arrastra el delay(5).
//
//   el flanco todavia vigente - EL PULSO. El rele de la camara cierra ~1 s por deteccion
//        y ABRE entre coche y coche: con solo el nivel, una cola de vehiculos se leeria
//        como huecos de "no hay nadie" cada vez que el contacto se abre, y quien decide
//        el semaforo muestrea justo en un hueco tarde o temprano. Medido: sin este
//        termino el verde con trafico continuo en J16 terminaba en el suelo (Bloque F).
//
// Y "VIGENTE" NO SE INVENTA AQUI: es demanda_ventanaMs(), el UNICO plazo que este
// firmware ya tiene para "esta demanda sigue en pie". Un numero propio seria el decimo
// "~1 s del rele" de A-7. La diferencia con demanda_hayLocal(), que usa ese mismo plazo,
// es que camUltimoFlanco[] SI se renueva en cada flanco -vigilante_flanco()- mientras que
// la ventana de demanda.cpp no se prolonga: por eso aquella deja huecos y esta no.
//
// camHuboFlanco[] es N-26: sin el, con millis() casi en cero contra un camUltimoFlanco[]
// tambien en cero la resta cae DENTRO de la ventana, y el arranque regalaria una
// presencia que nadie provoco.
static bool camara_presencia(int i, unsigned long ahora) {
  return camAnt[i]
         || (camHuboFlanco[i] && (ahora - camUltimoFlanco[i]) <= demanda_ventanaMs());
}

// EL RELOJ DEL VIGILANTE, y el unico sitio del fichero donde se mira la pluma.
//
// EL SILENCIO SE ACUMULA SOLO CON LA PLUMA ARRIBA, y ese es el "con el ciclo corriendo"
// de D-13: con la pluma abajo nadie puede cruzar el barrido y una camara callada no
// esta fallando (D-96: con la barrera fuera de servicio esta arriba tambien en rojo y el
// silencio corre, porque se cruza igual). Se lee semaforo_plumaArriba(), que devuelve LO
// QUE escribirPines() dejo puesto en el pin y no una segunda formula de SFTY-28 que
// alguien tendria que mantener igual a la primera.
static void vigilante_tick(unsigned long ahora) {
  const bool arriba = semaforo_plumaArriba();
  const unsigned long dt = ahora - camTickAnt;
  camTickAnt = ahora;

  if (arriba) {
    for (int i = 0; i < 2; i++) {
      // UN PIN QUE NUNCA HA DADO UN FLANCO NO SE VIGILA - Y ESE ES EL CASO NORMAL.
      //
      // ~~HAY UNA CAMARA POR POSTE (D-13, y las compras son 2 unidades para 2 postes), asi
      // que en todos los equipos que se monten UNO DE ESTOS DOS PINES ESTA VACIO~~ ->
      // CADUCADO el 11/09: DECISIONES.md deja ese dia DOS camaras por poste (p10 y p12 de
      // J16), las cuatro compradas. ESTA EXENCION PIERDE ASI SU MOTIVO y queda pendiente de
      // rehacer (CLAUDE.md 6: una excepcion es una afirmacion sobre el codigo). Sin el numero
      // de la fila, a proposito: decisiones_01 lo contaria como ancla y esto no la construye.
      // No se toca aqui. Lo que sigue describe lo que el codigo HACE, que no ha cambiado: con el
      // pull-down de 10K de la placa un pin vacio lee bajo para siempre: nunca da un
      // flanco. Sin esta linea acumulaba silencio como si fuera una camara y, cumplido el
      // plazo, emitia $ALARM CAM_CIEGA DE UNA CAMARA QUE NO EXISTE -el manual manda
      // entonces al tecnico a mirar una bornera vacia-. Y ademas CIEGA pesa mas que OK,
      // asi que ese pin tapaba en el campo CAM: el estado de la camara que si esta.
      //
      // LO QUE ESTA ELECCION CUESTA, ESCRITO EN VEZ DE DISIMULADO: asi NO se detecta una
      // camara que este muerta DESDE EL DIA DE LA INSTALACION. Nunca dio un flanco, luego
      // el firmware no la distingue de un conector vacio; es A-6 letra por letra -el pin
      // no distingue silencio de via libre-, y no hay bit que lo arregle.
      //
      // LO QUE LO COMPENSA NO ES FIRMWARE, Y ESTA MEDIDO: el paso de instalacion del
      // Manual 9 hace que el instalador PROVOQUE una deteccion delante de la camara y
      // compruebe que el equipo la acusa. Ese es el primer flanco. O sea que el vigilante
      // empieza a vigilar EXACTAMENTE cuando el instalador ha demostrado que la camara
      // ve, y ni un segundo antes.
      //
      // Y vale igual despues de cada reinicio: camHuboFlanco[] arranca en false, asi que
      // tras un corte la vigilancia de silencio queda desarmada hasta la primera
      // deteccion. Es deliberado -al encender no sabemos nada de esta camara-, pero deja
      // un hueco real: una camara que muera en el mismo corte que reinicia al equipo no
      // se anuncia. Va anotado como residual, no disimulado.
      //
      // ESTO NO TOCA LA OTRA ALARMA. Un contacto ya cerrado desde antes del arranque
      // tampoco da flancos, y ese SI se detecta: PEGADA cuelga del NIVEL, su cronometro
      // se siembra en camaras_sembrar() y vive en vigilante_nivel(), que no pasa por
      // aqui. Un pin vacio no puede dispararla -lee bajo siempre-, que es justo la
      // diferencia entre las dos.
      if (!camHuboFlanco[i]) {
        continue;
      }
      camSinFlancoMs[i] += dt;
      if (camSinFlancoMs[i] >= CAM_CIEGA_MS
          && camEstado[i] != CAM_CIEGA && camEstado[i] != CAM_PEGADA) {
        camEstado[i] = CAM_CIEGA;
        camara_alarmar(i, "CAM_CIEGA", "SIN_FLANCO");
      }
    }
  }

  // EL CONTADOR. YA NO DICE "HABRIA": DICE "ACTUO" (D-33, 14/09/2026).
  //
  // ~~Se mira en el instante en que la pluma ACABA DE BAJAR... Esto OBSERVA una
  // transicion ya hecha~~ -> CADUCADO POR D-33, y no por gusto: CON EL VETO CONSTRUIDO
  // ESA TRANSICION YA NO OCURRE. Si el disparo se hubiera quedado donde estaba, el
  // contador se habria callado para siempre EXACTAMENTE en el caso que vino a medir, y
  // un contador a cero se lee como "esto no pasa nunca", que es la conclusion CONTRARIA
  // a la verdad. Se muda el DISPARO; el contador, su saturacion y su $EVENT se conservan.
  //
  // SE CUENTA EL FLANCO DE SUBIDA, NO EL NIVEL: un veto que dura tres minutos es UN veto,
  // no uno por vuelta del loop. Contar el nivel llenaria el aire de tramas identicas y
  // haria que el numero midiera la velocidad del bucle en vez de los coches.
  //
  // ~~"Hay presencia" se pregunta a camara_presencia()...~~ Ya no se pregunta aqui: la
  // hace escribirPines(), que es quien decide, y aqui solo se lee LO QUE DECIDIO
  // -semaforo_plumaVetada()-. Preguntarlo otra vez seria una segunda copia de la
  // condicion del veto, muestreada en otro instante, y podria contar un veto que no hubo
  // o callarse uno que si. camara_presencia() sigue siendo la definicion unica y sigue
  // teniendo consumidor -camara_presenciaJ16()-, que es por donde la contestan tanto el
  // Modo Inteligente (Maestro) como el veto de la pluma (las dos puntas).
  const bool vetada = semaforo_plumaVetada();
  if (vetada && !camVetoAnt) {
    camVetoDesde = ahora;
    camVetoAvisos = 0;
    if (camVetos < 65535) {
      camVetos++;
    }
    char detalle[sizeof("VETO_SOSTENIDO_MIN:65535")];
    snprintf(detalle, sizeof(detalle), "VETO_ACTUADO_N:%u", camara_vetosPluma());
    bluetooth_reportarEvento("CAMARA_PLUMA", detalle);
  }

  // -------------------------------------------------------------------------------
  // EL VETO QUE NO SE ACABA - D-33, AMPLIACION DEL RESPONSABLE (14/09/2026)
  // -------------------------------------------------------------------------------
  //
  // EL SENTIDO DEL FALLO ESTA DECIDIDO Y ES ESTE: "si la camara da error, asumo que
  // NUNCA BAJA LA BARRERA por error o falsa alarma. Se informa a la app para pedir
  // AJUSTE DE CAMARA, y la barrera nunca baja". Una barrera arriba no aplasta a nadie;
  // el precio es que deja de proteger, Y POR ESO TIENE QUE VERSE. Esto es ese verse.
  //
  // NO HAY TOPE QUE BAJE LA PLUMA, Y NO ES UN OLVIDO (A-1.bis): un tope que baja igual
  // devuelve el peligro que el veto evita, porque este firmware NO PUEDE distinguir un
  // rele trabado de un vehiculo parado debajo -dan el mismo nivel-. Tope es AVISO.
  //
  // POR QUE CAM_PEGADA NO BASTA, medido: CAM_PEGADA cuelga del NIVEL sostenido, y una
  // camara que dispara sin parar -mal ajustada, vegetacion, lluvia- ABRE el contacto
  // entre disparo y disparo. Su cronometro se reinicia en cada flanco, asi que nunca
  // alarma; pero camara_presencia() SI la ve continua -el flanco vigente dura
  // demanda_ventanaMs()-, o sea que el veto no se suelta y nadie se entera. Ese es el
  // hueco exacto que tapa este aviso, y por eso es de la PLUMA y no de una camara.
  //
  // EL PLAZO ES EL DEL PELIGRO, NO EL DEL DIAGNOSTICO, Y ESA DISTINCION ES TODO.
  //
  // Se escribio primero con CAM_PEGADA_MS (20 min) "porque ya existia", y es el numero
  // EQUIVOCADO: aquel plazo contesta a "rele trabado o vehiculo parado", que es una
  // pregunta de DIAGNOSTICO, y llega veinte minutos tarde. El peligro de una pluma
  // retenida no se mide en el contacto: se mide contra el TODO-ROJO DE DESPEJE, porque
  // en cuanto el despeje vence la otra punta puede abrir su verde y entonces el tramo
  // tiene dos bocas -una por luz, otra por barrera-.
  //
  // EL BORDE, ESCRITO AL LADO Y POR QUE ES EL CORRECTO (CLAUDE.md 7): el despeje es
  // CONFIGURABLE entre DESPEJE_SEG_MIN y DESPEJE_SEG_MAX, y este fichero no tiene el
  // valor en curso -vive en coordinador.cpp en el Maestro y llega por radio en el
  // Esclavo-. Se toma el MAXIMO del rango: un veto que dura mas que el despeje MAS LARGO
  // que el equipo admite ha vencido al despeje EN CUALQUIER configuracion, asi que este
  // aviso NUNCA puede ser un falso positivo por tener el despeje largo. El precio esta
  // declarado: con el despeje al minimo el aviso llega tarde -90 s en vez de 10-, y la
  // unica forma de afinarlo es preguntarle al coordinador el despeje EN CURSO, que es
  // un getter que este encargo no puede escribir.
  //
  // LO QUE EL AVISO DICE Y LO QUE NO: dice cuantos segundos lleva la pluma retenida, que
  // es lo unico MEDIDO. NO dice "camara averiada": vetar mucho puede ser una camara mal
  // ajustada O un cruce con mucho trafico, y este micro no ve imagen (D-12) y no puede
  // separarlos. Quien traduce esto a "revise el ajuste de la camara" es la app.
  //
  // ES PERIODICO A PROPOSITO. Un aviso unico se pierde si la sesion de Bluetooth empieza
  // despues y deja al operario con una barrera arriba sin explicacion; repetirlo es lo
  // que hace que la condicion se pueda descubrir entrando tarde.
  if (vetada) {
    const unsigned long retenida = ahora - camVetoDesde;
    const uint16_t debidos = (uint16_t)(retenida / VETO_SOSTENIDO_MS);
    if (debidos > camVetoAvisos) {
      camVetoAvisos = debidos;
      // N-154: snprintf NO avisa de que recorta. Con seis digitos dejaria
      // "VETO_SOSTENIDO_S:10368" cuando lleva 1.036.800 s: un numero BIEN FORMADO Y
      // FALSO, que es peor que no publicarlo, porque la app lo dibuja igual. Fuera de
      // cota va "!", que en este protocolo significa "llego imposible" y nunca "--",
      // que significa "todavia no lo se" (bluetooth.cpp).
      const unsigned long segs = retenida / 1000UL;
      char detalle[sizeof("VETO_SOSTENIDO_S:65535")];
      if (segs > VETO_SEG_PUBLICABLE_MAX) {
        snprintf(detalle, sizeof(detalle), "VETO_SOSTENIDO_S:!");
      } else {
        snprintf(detalle, sizeof(detalle), "VETO_SOSTENIDO_S:%u", (unsigned)segs);
      }
      bluetooth_reportarEvento("CAMARA_PLUMA", detalle);
    }
  }
  camVetoAnt = vetada;
}
