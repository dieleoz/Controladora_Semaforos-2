// ===== src/bluetooth_bucle.inc (MAESTRO) =====
// bluetooth_loop(): la lectura del puerto, la telemetria $STATUS y los diagnosticos periodicos.
//
// NO SE COMPILA SOLO: lo incluye bluetooth.cpp en su sitio, dentro de su unidad de compilacion, y por eso usa sus
// static (btBufIn, enviarTramaConCrc, procesarComando...). Salio de bluetooth.cpp por el tope de 500 lineas
// (roadmap, "Partir"), literal; los instrumentos lo leen dentro de bluetooth.cpp (banco/fuente.py, expandir_inc).
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

  // D-26 (5): despues de despachar, para que una HORA_ESP32 que acaba de llegar cuente ya.
  horaEsp32Vigilar(ahora);

  // 1.49(a): el veredicto de un REINICIAR_RELOJ pendiente, si ya lo hay. Detras del
  // despachador para que una orden que llega en esta vuelta no se conteste en la misma: el
  // cristal recien adoptado todavia no ha podido contar.
  atenderVeredictoReloj();

  // A-15: el $ACK diferido de SET_DEG_AUTO, una respuesta por veredicto.
  switch (degAuto_veredicto()) {
    case DAV_ON_EFECTIVO: enviarTramaConCrc("$ACK,CMD:SET_DEG_AUTO,RESULT:ON_EFECTIVO"); break;
    case DAV_ON_FALTA: enviarTramaConCrc("$ACK,CMD:SET_DEG_AUTO,RESULT:ON_FALTA_EL_OTRO_POSTE"); break;
    case DAV_OFF: enviarTramaConCrc("$ACK,CMD:SET_DEG_AUTO,RESULT:OFF"); break;
    case DAV_SIN_ACUSE:
      enviarTramaConCrc("$ERR,CMD:SET_DEG_AUTO,DESC:CAMBIADO_AQUI_SIN_ACUSE_DEL_OTRO_POSTE");
      break;
    default: break;
  }

  // 2. Emision periodica de telemetria cada 2000 ms ($STATUS,...)
  //
  // POR QUE 2000 Y NO 1000 -decision del responsable, 04/09-. El unico consumidor del
  // $STATUS es vigilarEnlace() de la app, y su cota son 5000 ms: a 1000 ms se emitia
  // cinco veces mas rapido de lo que nadie necesita. Y el cable no era gratis: medido
  // por esp32_07_presupuesto_bytes, el peor segundo eran 528 B de los 960 B/s que
  // caben a 9600 8N1 -el 55,0%-; a 2000 ms son 462 B, el 48,1%.
  //
  // NO baja al 30%, y el numero se deja escrito porque la cuenta a ojo se equivoca
  // aqui: de los cuatro sumandos del peor segundo -el $STATUS periodico mas el
  // $EVENT, el $ALARM y el $ACK que coinciden con el- SOLO el periodico se parte
  // por dos. La rafaga no escala con la cadencia. Medido, no supuesto.
  //
  // EL COSTE VA DECLARADO, NO ESCONDIDO: el tablero del operario refresca la mitad de
  // rapido. Eso es lo que se paga por el margen de cable, y lo decidio el responsable.
  //
  // POR QUE SIGUE ESCRITO A PELO Y NO EN UNA CONSTANTE CON NOMBRE, que es lo que este
  // repositorio pide: hay DOS instrumentos que leen este numero por TEXTO, con el patron
  // `ahora - tUltimaTelemetria >= (\d+)` -banco/packs/esp32_07_presupuesto_bytes.py y
  // Simulaciones/simulador_puente_esp32.py-. Un identificador en su sitio no casa con
  // (\d+): los dos quedarian ABORTADOS, y un ABORTADO deja pasar sin mirar todo lo que
  // vigilaba. Si algun dia se saca a un nombre, los dos lectores se cambian en el MISMO
  // commit; mientras tanto, las dos puntas van a la vez porque el pack lo exige.
  if (ahora - tUltimaTelemetria >= 2000) {
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
    const unsigned long rttMs = coordinador_tiempoRespuestaMs();

    // AQUI PONIA rfTxt[13] Y rttTxt[16], DIMENSIONADOS PARA EL TIPO, Y ESE ERA EL
    // DEFECTO (05/09). El razonamiento escrito era bueno -"un buffer dimensionado por
    // una invariante que vive en OTRO modulo es el que se rompe en silencio el dia que
    // ese modulo cambie"- y su conclusion era mala: sumando los tres campos de rango
    // ajeno a su tope de TIPO, este $STATUS pedia mas de lo que cabe en payload[144], y
    // ademas mas que el techo de 155 B que impone tramaCompleta[160]. No es una holgura
    // que se agota: es una trama que NO CABE, y una trama truncada se descarta ENTERA
    // por checksum. Hoy no truncaba porque los valores REALES son cortos, que es la
    // definicion de una barrera que no protege de nada.
    //
    // LO QUE CAMBIA NO ES EL TAMANO: ES DE QUE SE FIA. Ya no se fia de una invariante
    // que vive en otro fichero; se fia de la GUARDA que hay tres lineas mas abajo, que
    // corre en esta misma funcion y en cada emision. La cota la declara el modulo que la
    // promete -coordinador.h, junto a la funcion- y aqui se COMPRUEBA antes de imprimir.
    //
    // Y ACOTAR NO ES RECORTAR EN SILENCIO: si el valor se sale de su rango declarado se
    // publica "!", que no es un numero ni es "--". "--" ya significa "todavia no lo se"
    // -no hay muestras, no hay hora, no hay cuenta atras- y aplastar ahi un valor
    // imposible lo escondería entre los huecos normales. "!" dice lo unico cierto: llego
    // un valor que no puede ser, y eso es un HALLAZGO. El parser de la app devuelve tal
    // cual lo que no entiende (nmea_parser.js, _numeroOMarca), asi que llega a quien
    // pinta sin convertirse en un cero.
    char rfTxt[5];    // "100%" + NUL, garantizado por la guarda de CALIDAD_ENLACE_MAX
    char rttTxt[8];   // "25000ms" + NUL, garantizado por la de RTT_PUBLICABLE_MAX_MS
    if (rfCalidad < 0) {
      strncpy(rfTxt, "--", sizeof(rfTxt));
      strncpy(rttTxt, "--", sizeof(rttTxt));
    } else {
      if (rfCalidad > CALIDAD_ENLACE_MAX) {
        strncpy(rfTxt, "!", sizeof(rfTxt));
      } else {
        snprintf(rfTxt, sizeof(rfTxt), "%d%%", rfCalidad);
      }
      if (rttMs > RTT_PUBLICABLE_MAX_MS) {
        strncpy(rttTxt, "!", sizeof(rttTxt));
      } else {
        snprintf(rttTxt, sizeof(rttTxt), "%lums", rttMs);
      }
    }

    // 12 y no 16, y los 4 B NO eran gratis. %02u sobre los uint8_t de reloj.h no pasa de
    // tres cifras por campo, asi que lo mas largo que este snprintf puede escribir son
    // 11 caracteres ("255:255:255", con el reloj diciendo cualquier cosa). El pack del
    // presupuesto acota cada campo por SU BUFFER -que es lo unico que snprintf garantiza
    // sin modelar tipos-, asi que un buffer holgado no es prudencia: es margen que se
    // resta del payload y que la cuenta tiene que dar por gastado.
    // D-9: LA HORA NO ES DE ESTA PUNTA, Y POR ESO ESTE CAMPO SALE VACIO CASI SIEMPRE.
    // El micro no tiene reloj con pila -su cristal de respaldo esta muerto en cobre-, de
    // modo que lo unico honesto aqui es publicar el hueco y dejar que lo rellene, al
    // pasar la trama, el aparato que si lleva el reloj. Inventar un numero en este campo
    // seria peor que dejarlo vacio: el operario no tendria como saber que lo que lee no
    // lo cuenta nadie.
    char horaBuf[12];
    if (reloj_enHora()) {
      snprintf(horaBuf, sizeof(horaBuf), "%02u:%02u:%02u", reloj_hora(), reloj_minuto(), reloj_segundo());
    } else {
      strncpy(horaBuf, "--:--:--", sizeof(horaBuf));
    }

    // T: SEGUNDOS QUE FALTAN PARA QUE TERMINE LA FASE (04/09).
    //
    // AQUI PONIA `(ahora / 1000UL) % 60UL` CON EL COMENTARIO "segundos transcurridos
    // en fase actual" ENCIMA. No era eso: `ahora` es millis(), asi que el campo era el
    // SEGUNDERO DEL TIEMPO ENCENDIDO, de 0 a 59 y vuelta a empezar, dijera lo que
    // dijera el semaforo. Es la forma exacta de un campo DECLARADO que nadie ejercia:
    // el nombre y el comentario prometian una cosa y la cuenta hacia otra, y como el
    // numero se movia, parecia vivo. Reportado desde el banco: "el tiempo no esta
    // retrocediendo, sino que esta aumentando... deberia empezar en 30 y disminuir".
    //
    // EL DUENO DEL PLAZO ES EL COORDINADOR, y contesta -1 cuando en esa fase no hay
    // cuenta atras que dar. Ese caso se marca "--", igual que RF, RTT, BAT y HORA: es
    // la convencion que ya tiene esta trama, no una nueva. La lista completa de en que
    // estados no hay numero -y por que- esta sobre coordinador_segundosRestantesFase().
    //
    // OCUPACION DEL CABLE: SIN CAMBIO. El campo media 1 o 2 caracteres (0..59) y sigue
    // midiendo 1 o 2: "--" son dos, y el mayor despeje que admite el firmware son 90 s
    // (DESPEJE_SEG_MAX en limites_ciclo.h) o 99 s por el Manual, o sea dos digitos. El
    // 48,1% del peor segundo que mide esp32_07_presupuesto_bytes no se mueve.
    //
    // EL BUFFER SE DIMENSIONA PARA EL RANGO, Y LO QUE LO SOSTIENE ES LA GUARDA DE ABAJO
    // (05/09). Aqui ponia tTxt[12] "para el TIPO -un int con %d cabe en 11 caracteres-,
    // no para el rango que hoy garantiza otro modulo". Ese razonamiento sumado a los de
    // RF: y RTT: daba un $STATUS que no cabe ni en payload[144] ni en el techo de 155 B
    // de tramaCompleta[160]: el porque entero esta sobre CUENTA_ATRAS_MAX_SEG, en
    // coordinador.h. La cota la promete el modulo que produce el numero y aqui se
    // comprueba; fuera de rango se publica "!", que no es "--" -eso ya significa "en
    // esta fase no hay cuenta atras"- sino "llego un valor imposible".
    char tTxt[4];
    // N-143 (04/09): SE PREGUNTA PRIMERO AL COORDINADOR Y DESPUES AL MODO, Y EL ORDEN
    // NO ES ARBITRARIO.
    //
    // El coordinador sabe los despejes y las transiciones -las fases CORTAS-; el modo
    // sabe la fase LARGA, que es la de 3 a 15 minutos y la unica que el operario mira de
    // verdad. Con N-139 solo se publicaba la primera, asi que la mayor parte del tiempo
    // el contador decia "--". El responsable lo dijo dos veces: "debe ser un contador
    // decreciente, de saber cuanto tiempo falta para el cambio".
    //
    // El coordinador va PRIMERO porque cuando el tiene cuenta -un todo-rojo en curso- esa
    // es la que manda: el modo esta esperando a que termine y su plazo largo todavia no
    // ha empezado a correr. Preguntar al reves publicaria el numero de una fase que aun
    // no ha comenzado.
    int faseRestanteSeg = coordinador_segundosRestantesFase();
    if (faseRestanteSeg == SIN_CUENTA_ATRAS) {
      // Y el modo solo contesta si es el suyo: fuera del Automatico devuelve
      // SIN_CUENTA_ATRAS y aqui se queda el "--", que es lo correcto -en Manual la fase
      // acaba cuando alguien pulsa, y en Inteligente el tiempo es un maximo-.
      faseRestanteSeg = modoAutomatico_segundosRestantesFase();
    }
    if (faseRestanteSeg == SIN_CUENTA_ATRAS) {
      strncpy(tTxt, "--", sizeof(tTxt));
    } else if (faseRestanteSeg < 0 || faseRestanteSeg > CUENTA_ATRAS_MAX_SEG) {
      // El "< 0" no es redundante con la rama de arriba: SIN_CUENTA_ATRAS es UN valor
      // (-1), y cualquier otro negativo -una resta que se fue, un contador sin
      // inicializar- no es "no hay cuenta", es un numero imposible. Se separan porque
      // significan cosas distintas y porque el operario tiene que poder distinguirlas.
      strncpy(tTxt, "!", sizeof(tTxt));
    } else {
      snprintf(tTxt, sizeof(tTxt), "%d", faseRestanteSeg);
    }

    char serieTxt[7];
    identidad_texto(serieTxt);

    // N-108 - BAT SE MARCA, EN LAS DOS PUNTAS. El 12.6 era un LITERAL: no hay un solo
    // analogRead() en Maestro/src, Maestro/include, Esclavo/src ni Esclavo/include
    // -MEDIDO: grep sin una coincidencia-. Un tablero que ensena 12,6 V fijos no esta
    // informando de la bateria, esta impidiendo que nadie pregunte por ella: el tecnico
    // ve el numero bueno y descarta la alimentacion como causa sin haberla mirado.
    // Vuelve a haber cifra cuando haya divisor y una entrada analogica que lo lea.
    // N-149: ESC: SOLO VA EN EL $STATUS DEL MAESTRO, y esa asimetria es deliberada.
    //
    // Conectado al Maestro la app es la CONSOLA DE OPERACION y ensena el cruce entero;
    // conectada al Esclavo es DIAGNOSTICO y no opera. El Esclavo no sabe nada del
    // Maestro -no le pregunta y no tiene por que-, asi que anadirle un campo simetrico
    // seria inventarse el dato, que es justo lo que este campo existe para no hacer.
    //
    // N-153: PLUMA: - EL ESTADO DE LA TALANQUERA, QUE ES UN ELEMENTO QUE SE MUEVE Y
    // HASTA HOY NO SALIA DEL MICRO.
    //
    // El valor NO se recalcula aqui: se pregunta a semaforo.cpp, que es el unico que
    // decide (SFTY-28) y que deja anotado lo que acaba de escribir en el pin. Una
    // segunda copia de la condicion en este fichero seria la que se queda vieja cuando
    // la de alla cambie -y D-13 la va a cambiar-, y eso ya se ha pagado tres veces.
    //
    // VA EN LAS DOS PUNTAS, y no es una simetria de oficio: las dos placas son la MISMA
    // y las dos tienen su talanquera en PB2 -pines.h:31, "opto U15 -> MOSFET Q10 ->
    // bornera J15" en las dos-, asi que cada punta publica LA SUYA. No es el caso de
    // ESC:, que es asimetrico porque el Esclavo no tiene de donde sacar el dato del
    // otro poste; aqui cada equipo tiene el suyo delante.
    //
    // D-13 fase 1 (05/09): CAM: - LO QUE EL VIGILANTE DE LAS CAMARAS SABE, QUE HASTA
    // HOY NO SALIA DEL MICRO.
    //
    // camara_estado() publica LA PEOR de las dos camaras y no una por cada una, y eso
    // no es ahorro de bytes: el operario no tiene que hacer el maximo de dos campos
    // para saber si puede fiarse de la deteccion. El orden de gravedad vive en el enum
    // de botones.cpp -OK < ? < CIEGA < PEGADA-, en un solo sitio.
    //
    // "?" NO ES "OK", Y ESA ES LA MITAD QUE MAS IMPORTA. Es el estado de arranque:
    // todavia no ha pasado nada por esos pines, asi que no se sabe si la camara ve.
    // Por eso pesa MAS que OK en el enum y por eso el campo lo publica en vez de
    // colapsarlo: "no lo se" y "esta bien" mandan al tecnico a sitios opuestos, y esta
    // punta no puede inventarse el que le falta.
    //
    // VA EN LAS DOS PUNTAS, como PLUMA: y por lo mismo -las dos placas son la misma y
    // las dos tienen sus dos camaras en J16-, no como ESC:, que es asimetrico.
    //
    // EL BUFFER SUBE A 155, Y EL PEOR CASO CABE (05/09).
    //
    // La primera version del campo ESC: puso 160 "por si acaso" y el banco la tumbo:
    // esp32_07_presupuesto_bytes exige tramaCompleta >= payload + 5 -el *XX y el CRLF que
    // enviarTramaConCrc anade despues-, y tramaCompleta mide 160. Un payload de 160 se
    // habria truncado en el ULTIMO paso, saliendo al cable bien formado hasta la mitad.
    // Es la regla del instrumento (CLAUDE.md 4) contra la cuenta a ojo.
    //
    // 🛑 PERO ESA COMPROBACION ERA SOLO LA MITAD, Y LA QUE FALTABA ES LA QUE IMPORTA:
    // "el envoltorio cabe el payload" es una cota entre DOS BUFFERS. Que el CONTENIDO
    // quepa en el payload no lo comprobaba nadie, y NO CABIA -ni antes de PLUMA:-.
    // esp32_07 lo mide ahora, y por eso esta cuenta no es un comentario decorativo: si
    // deja de cuadrar, el pack falla.
    //
    // EL PEOR CASO SE ACOTA POR BUFFER, NO POR RANGO NI POR TIPO. Es lo unico que
    // snprintf garantiza sin creerse a nadie: en un char x[N] no entran mas de N-1
    // caracteres, pase lo que pase aguas arriba. Parte fija de la plantilla: 83.
    //
    //   SERIE   6   serieTxt[7]  -- identidad_texto() escribe 6 hex y el NUL
    //   MODO   11   literal: el mas largo de obtenerNombreModo() (INTELIGENTE/DESCONOCIDO)
    //   ESTADO  9   literal: "FALLO COM", el mas largo de semaforo_nombreEstado()
    //   T       3   tTxt[4]      -- "900" es el techo, y lo sostiene la guarda de
    //                               CUENTA_ATRAS_MAX_SEG, no una creencia sobre el modo
    //   RF      4   rfTxt[5]     -- "100%", sostenido por la guarda de CALIDAD_ENLACE_MAX
    //   RTT     7   rttTxt[8]    -- "25000ms", por la de RTT_PUBLICABLE_MAX_MS
    //   HORA   11   horaBuf[12]  -- tres uint8_t con %02u: "255:255:255" es el tope
    //   ESC     5   literal: "VERDE", el mas largo de coordinador_estadoEsclavo()
    //   PLUMA   6   literal: "ARRIBA"
    //   CAM     6   literal: "PEGADA", el mas largo de camara_estado()
    //
    //   83 + 68 = 151 caracteres + NUL = 152 B. En payload[155] quedan 3 B, y con el
    //   envoltorio son 156 B de los 160 de tramaCompleta.
    //
    // ANTES DE ACOTAR, LA MISMA CUENTA DABA 168 CARACTERES -25 de mas-, porque tTxt,
    // rfTxt, rttTxt y horaBuf estaban dimensionados para el tipo (12, 13, 16 y 16).
    // No truncaba en la calle porque los valores reales son cortos; o sea que lo que
    // impedia el fallo era la suerte, no el codigo.
    //
    // D-13 (05/09): EL CAMPO CAM: ENTRA, Y EL BUFFER SUBE A 155 -- QUE ES EL TECHO, NO
    // UN "POR SI ACASO".
    //
    // El apunte de N-154 dejaba escritas las DOS salidas para hacerle sitio, y decia que
    // no se decidiera a ojo. Medidas las dos, con el peor caso por buffer:
    //
    //   (a) acotar HORA:   valida hora/minuto/segundo y baja el campo de 11 a 8, o sea
    //                      horaBuf[12] -> [9]. Devuelve EXACTAMENTE 3 B: el peor caso
    //                      pasa de 151 a 148 y payload[144] guarda 143. SIGUE SIN CABER
    //                      POR 5 B. No es que sea peor opcion: es que NO ALCANZA SOLA.
    //   (b) payload -> 155  el peor caso son 151 y quedan 3 B. Cabe.
    //
    // 155 NO ES UN NUMERO HOLGADO ELEGIDO A OJO: es el maximo que tramaCompleta[160]
    // admite -payload mas el cierre "*XX" con su CR y su LF-, y esp32_07_presupuesto_bytes lo
    // exige en la misma corrida (bufTrama >= bufStatus + 5, que con 155 queda en 160 >= 160, sin holgura
    // que gastar). Al ponerlo en el techo, las dos cotas del pack se vuelven la MISMA y
    // la unica que puede fallar es la que importa: que el CONTENIDO quepa.
    //
    // LO QUE CUESTA, DICHO Y NO ESCONDIDO: el peor segundo del presupuesto de J17 se
    // calcula sobre la CAPACIDAD del buffer, no sobre el contenido, asi que sube del
    // 48,1% al 52,3% de los 960 B/s. Sigue cabiendo con mas de 450 B/s de sobra.
    //
    // Y (a) NO SE GASTA HOY A PROPOSITO: es un lever que no pierde valor esperando -los
    // 3 B seguiran ahi- y hoy no compra nada, porque el que decide es (b). El dia que
    // haga falta un campo mas, esa es la carta que queda.
    char payload[155];
    snprintf(payload, sizeof(payload),
             "$STATUS,NODE:MAESTRO,SERIE:%s,MODO:%s,ESTADO:%s,T:%s,RF:%s,RTT:%s,BAT:--,HORA:%s,ESC:%s,PLUMA:%s,CAM:%s",
             serieTxt, modoStr, estadoStr, tTxt, rfTxt, rttTxt, horaBuf,
             coordinador_estadoEsclavo(),
             semaforo_plumaFuera() ? "FUERA" : semaforo_plumaArriba() ? "ARRIBA" : "ABAJO",   // D-97/D-150
             interruptores_camNoVeto() ? "NOVETO" : camara_estado());                        // D-151

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

  // ---------------------------------------------------------------------------
  // D-32 (1), 13/09 - EL POSTE 1 TAMBIEN DICE COMO VE EL SU ENLACE. ES EL GEMELO DEL
  // PERIODICO DE D-23, Y SE ESCRIBE COPIANDO AQUEL, NO INVENTANDO OTRO.
  //
  // QUE SE ROMPIO, medido: protocolo_bytesRecibidos() y protocolo_tramasValidas() -los dos
  // contadores de SFTY-15 que separan "no llega nada" de "llega basura"- se quedaron SIN
  // NINGUN LECTOR en esta punta al retirar el LCD. Su unico lector era refrescarSiCambio()
  // de modo_alcance.cpp, que los pintaba en la pantalla de PRUEBA ALCANCE.
  // costura_10_funciones_muertas los lleva anotados desde ese commit y su motivo dice
  // literalmente cual es el arreglo: "el sustituto natural es publicarlos en bluetooth.cpp,
  // y eso es firmware que hay que escribir, no una anotacion". Esto es ese firmware.
  //
  // NO VAN EN EL $STATUS AUNQUE LA ANOTACION LO PROPUSIERA, Y EL MOTIVO ESTA MEDIDO: el
  // $STATUS de esta punta es el que fija el borde del cable. Su payload[155] esta en el
  // TECHO que tramaCompleta[160] admite, su peor caso son 151 caracteres de los 154 que
  // guarda, y la trama entera son 157 de los 159 que el puente acepta antes de DESCARTARLA
  // COMPLETA. Margen 2. Un campo mas -el mas corto imaginable, "RX:" con un digito- se come
  // esos dos bytes y el poste deja de hablar; ademas documentos_03 obliga a que un campo
  // del $STATUS este en las DOS puntas, y el Esclavo esta en el mismo techo. Por eso es un
  // $EVENT: el sitio donde D-32 (2) ya puso el dato gemelo el mismo dia.
  //
  // EL MISMO BLOQUE QUE EL DEL ESCLAVO -constante, cadencia, ORIGEN y contadores-, para que el
  // tecnico compare un poste con otro sin traducir; D-92 solo anade aqui el AMBAR: final.
  //
  // POR QUE FUERA DEL if DEL $STATUS Y CON SU PROPIO RELOJ: colgando de aquel, la cadencia
  // dejaria de ser la que dice DIAG_ENLACE_MS para pasar a ser un multiplo del periodico, y
  // ademas los DOS instrumentos que leen `ahora - tUltimaTelemetria >= N` por TEXTO
  // -esp32_07_presupuesto_bytes y simulador_puente_esp32- estarian midiendo un bloque que
  // ya no es solo el $STATUS.
  //
  // POR QUE 30 s: el del Esclavo, con su motivo escrito alli (la bitacora de 30 entradas de la
  // app). Separarlos seria dos numeros que significan lo mismo y se separan el dia que se toca uno.
  //
  // LIMITE DECLARADO, el mismo que alli: arranca en 0, asi que la primera sale a los
  // DIAG_ENLACE_MS de encender y no en el arranque, cuando todavia puede no haber nadie
  // escuchando.
  //
  // EL ORIGEN ES ENLACE_RF Y NO UNO NUEVO: es el MISMO sujeto que el $EVENT de flanco de
  // aqui arriba -como ve ESTA punta su radio-, y la app ya los distingue por el DETALLE.
  //
  // LA COTA DE det[] ES LA DE tramo[32] DEL $ALARM EN SU FORMA DE ARRIBA, PERO LA CUENTA ES
  // OTRA Y POR ESO VA ESCRITA: los tres son CONTADORES LIBRES de protocolo.cpp -sin techo
  // que prometer, al reves que el RF/RTT/SINRESP del $ALARM, que si tienen cota declarada y
  // por eso se comparan-, asi que se acotan a su tope de TIPO, que son las diez cifras de un
  // unsigned long. Parte fija de este formato: "RX:" + " OK:" + " RUIDO:" = 14.
  // 14 + 30 = 44 caracteres + NUL = 45; un techo inventado seria una cota escrita, no medida.
  // Que el $EVENT entero quepa en su payload lo recalcula esp32_07_presupuesto_bytes.
  //
  // EL SEPARADOR SON ESPACIOS Y NO COMAS, y no es estilo: _camposNmea() de la app parte la
  // trama por ',' y cada trozo por su PRIMER ':', asi que una coma dentro del DETALLE
  // convierte lo que va detras en campos sueltos que el pintor de $EVENT no mira.
  // ---------------------------------------------------------------------------
  if (ahora - tUltimoDiagEnlace >= DIAG_ENLACE_MS) {
    tUltimoDiagEnlace = ahora;
    char det[76];  // D-60: " REIN:" (6) y un %u por TIPO (10); D-92: " AMBAR:ARRANQUE" (15): 44+16+15+NUL
    snprintf(det, sizeof(det), "RX:%lu OK:%lu RUIDO:%lu REIN:%u%s",
             protocolo_bytesRecibidos(), protocolo_tramasValidas(),
             protocolo_tramasDescartadas(), (unsigned)respaldo_reinicios(), ambarOrigenTxt());
    bluetooth_reportarEvento("ENLACE_RF", det);
  }

  diagDegradadoPublicar(ahora);
}
