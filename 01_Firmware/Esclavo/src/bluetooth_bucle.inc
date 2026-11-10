// ===== src/bluetooth_bucle.inc (ESCLAVO) =====
// bluetooth_loop(): la lectura del puerto, la telemetria $STATUS y los diagnosticos periodicos.
//
// NO SE COMPILA SOLO: lo incluye bluetooth.cpp en su sitio, dentro de su unidad de compilacion, y por eso usa sus
// static (btBufIn, enviarTramaConCrc, procesarComando...). Salio de bluetooth.cpp por el tope de 500 lineas
// (roadmap, "Partir"), literal; los instrumentos lo leen dentro de bluetooth.cpp (banco/fuente.py, expandir_inc).
void bluetooth_loop() {
  const unsigned long ahora = millis();

  // SOSTENEDOR DEL AMBAR PEDIDO POR LA APP - LA SEGUNDA MITAD DEL MOLDE (N-106).
  //
  // Es mando.cpp:274 con la otra bandera, y existe por el mismo motivo que alli: la orden
  // de quien la dio tiene que sobrevivir a lo que pase despues. En concreto al todo-rojo
  // de salida del Modo Degradado, que TERMINA EN INACTIVO, NO EN AMBAR: copiada solo la
  // primera mitad del molde -la del despachador-, el todo-rojo de despedida acababa y
  // nadie encendia nada, con lo que el equipo se quedaba en ROJO despues de haber
  // contestado SALIENDO_TODO_ROJO. Eso es lo contrario de lo que se pidio, con el $ACK ya
  // enviado.
  //
  // Se RE-ARMA en vez de encenderse una sola vez porque un ambar que se apaga solo no es
  // un estado seguro, es un parpadeo.
  //
  // Las tres guardas, y ninguna sobra:
  // Las guardas eran TRES. La que falta -!semaforo_senalEnCurso()- protegia los destellos
  // de confirmacion del mando, y salio con el mando el 14/09 (D-30): ya no hay ninguna
  // senal que pueda ocupar las lamparas por encima de la logica.
  //   !degradado_gobiernaLuz()    durante el todo-rojo de despedida la luz es del modo. Es
  //                              tambien lo que impide que esto pise al Degradado.
  //   estado() != S_FALLO         si la rendicion de 48 h ya encendio el ambar en
  //                              DEG_RENDIDO, no se reinicia el parpadeo por gusto.
  //
  // Y AQUI YA NO HAY REVOCACION AUTOMATICA. La habia -"si la luz salio de S_FALLO, tira el
  // latch"- y se retira con su porque escrito en la cabecera de ambarEmergencia: durante
  // el todo-rojo de despedida la luz esta en ROJO, asi que mataba el latch milisegundos
  // despues de armarse. Desde R-3 el latch se quita pidiendolo: CANCELAR_AMBAR.
  if (ambarEmergencia && !degradado_gobiernaLuz() &&
      semaforo_estado() != S_FALLO) {
    semaforo_iniciarFallo();
  }

  // D-31 (3) y (4), 12/09 - EL PLAZO DEL ACUSE, SU UNICO REENVIO Y EL DESMENTIDO.
  //
  // POR QUE VIVE EN EL BUCLE Y NO EN LA RAMA QUE MANDA EL AVISO: porque el ambar NO
  // ESPERA. La rama enciende la luz, manda la trama y contesta al telefono en la misma
  // vuelta; bloquear el bucle por una radio de 2.4 kbps es peor que no confirmar nada
  // (N-130, y esa frase la firma este repositorio despues de medirla). Aqui solo se mira
  // un reloj que ya corre.
  //
  // EL PLAZO ES TIMEOUT_ACK_MS, no un numero nuevo: es lo que este firmware ya llama "un
  // viaje de radio con margen". Vive en protocolo.h como AVISO_AMBAR_TIMEOUT_MS porque
  // esta punta no ve las constantes del coordinador, y que sigan siendo el MISMO numero
  // lo mide un static_assert alli, no este comentario.
  //
  // UN REINTENTO. La parte que sigue siendo cierta: la distancia no alarga el viaje -a
  // 8 km la senal tarda 27 us mas- pero si sube la probabilidad de perder la trama, y
  // contra una trama perdida sirve REPETIR.
  // La que NO: aqui ponia que "cuantas veces lo dice el presupuesto de radio". Corregido
  // el 12/09 -el presupuesto NO es la restriccion-. AVISO_AMBAR_REINTENTOS esta ELEGIDO,
  // y lo que de verdad mueve subirlo es cuanto tarda esta punta en declarar que la otra
  // no contesta. El motivo entero, en protocolo.h.
  //
  // LA ALARMA DICE "NO HE PODIDO CONFIRMARLO", NUNCA "el otro poste no se entero", Y ESA
  // DIFERENCIA ES TODA LA DECISION: con un reintento de por medio, una trama perdida por
  // lluvia se ve EXACTAMENTE IGUAL que un transmisor muerto. Afirmar la causa seria
  // inventarsela, y lo que el tecnico puede actuar -cerrar el paso alli o avisar al
  // Poste 1 antes de irse- es lo mismo en los dos casos.
  //
  // QUE PROMETE CADA CAMPO DE ESTA ALARMA, escrito para que nadie lea de mas:
  //   EVENTO AVISO_RF        el aviso de ambar por radio, no la radio en general. La
  //                          caida de enlace es otra alarma y tiene su propio evento
  //                          (FALLO_RF): mezclarlas mandaria a revisar la antena cuando
  //                          lo roto puede ser solo el sentido de ida.
  //   CAUSA SIN_CONFIRMAR    no volvio acuse tras el intento y su reintento. NO dice que
  //                          el aviso no llegara: dice que esta punta no lo sabe.
  //   ACCION AVISE_POSTE_1   lo unico que cierra la ventana desde donde esta el tecnico.
  //
  // Y LAS TRES CABEN EN LA COTA DE bluetooth_reportarAlarma() SIN TOCARLA (N-154, margen
  // CERO): su peor caso son EVENTO 10 + CAUSA 19 + ACCION 14, y estos miden 8, 13 y 13.
  // Se comprueba aqui porque agrandar el buffer no es la cura y no se va a agrandar.
  if (avisoAmbarEsperando && (ahora - tAvisoAmbar) > AVISO_AMBAR_TIMEOUT_MS) {
    if (avisoAmbarReenvios < AVISO_AMBAR_REINTENTOS) {
      avisoAmbarReenvios++;
      tAvisoAmbar = ahora;
      protocolo_enviarPaquete(CMD_AMBAR_ESCLAVO);
    } else {
      // Se apaga ANTES de la alarma: si reportarAlarma() tardara o se reentrara, un
      // desmentido por aviso es lo prometido, no dos.
      avisoAmbarEsperando = false;
      bluetooth_reportarAlarma("AVISO_RF", "SIN_CONFIRMAR", "AVISE_POSTE_1");
    }
  }

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

  // 1. Recepción de Comandos desde la App Móvil
  while (SerialBT.available() > 0) {
    char c = (char)SerialBT.read();
    if (c == '\n' || c == '\r') {
      if (btIdxIn > 0) {
        btBufIn[btIdxIn] = '\0';
        procesarComando(btBufIn);
        btIdxIn = 0;
        // A3: DESPUES de despachar, no antes. La linea que llega puede ser el ambar de
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

    const char* estadoStr = semaforo_nombreEstado();

    // 12 y no 16, igual que en el Maestro y por el mismo motivo (05/09). %02u sobre los
    // uint8_t de reloj.h no pasa de tres cifras por campo, asi que lo mas largo que este
    // snprintf puede escribir son 11 caracteres. El pack del presupuesto acota cada
    // campo por SU BUFFER -lo unico que snprintf garantiza sin modelar tipos-, asi que
    // un buffer holgado no es prudencia: es margen que se resta del payload.
    char horaBuf[12];
    if (reloj_enHora()) {
      snprintf(horaBuf, sizeof(horaBuf), "%02u:%02u:%02u", reloj_hora(), reloj_minuto(), reloj_segundo());
    } else {
      strncpy(horaBuf, "--:--:--", sizeof(horaBuf));
    }

    char serieTxt[7];
    identidad_texto(serieTxt);

    // N-108 - RF, RTT Y BAT DEJAN DE INVENTARSE. Los tres eran LITERALES: "RF:98%",
    // "RTT:85ms" y "BAT:12.6" salian iguales del equipo estuviera como estuviera, asi que
    // el tablero ensenaba un enlace perfecto y una bateria sana mientras el poste se
    // quedaba sin radio. Un dato fijo con aspecto de medida es peor que un hueco: el
    // hueco hace preguntar, y el numero bueno hace descartar la causa sin mirarla.
    //
    // SE MARCAN, NO SE RETIRAN, y el motivo es que el campo si tiene que existir: el dia
    // que se midan, la app y el Manual 10 no cambian de contrato. Retirarlos ademas
    // dejaria las dos puntas emitiendo tramas distintas, que es lo que
    // documentos_03_trama_status impide con razon -la app es una sola-.
    //
    // POR QUE NINGUNO SE PUEDE MEDIR HOY EN ESTA PUNTA, medido y no supuesto:
    //   RTT   esta punta nunca ORIGINA una peticion. Recibe CMD_PING y contesta
    //         CMD_PONG tras el retardo de cortesia de SFTY-17. No hay ida y vuelta que
    //         cronometrar: lo unico que podria medir es su propio retardo.
    //   RF    no hay ventana de latidos aqui -los latidos los manda el Maestro-, y los
    //         contadores de SFTY-15 no dan una tasa: ver el porque entero en
    //         bluetooth_reportarAlarma(), donde SI se publican, en crudo y sin cociente.
    //   BAT   no hay un solo analogRead() en Esclavo/src ni en Esclavo/include -ni en las
    //         dos carpetas equivalentes del Maestro-. MEDIDO: grep sin coincidencias.
    //         Sin divisor y sin entrada analogica no hay bateria que leer.
    //
    // Y por decision del responsable (31/08) NO se anade RSSI: no se mide potencia. Lo
    // que hay se llega a latidos, y el estado del enlace se indica visualmente -abajo, en
    // el $EVENT, y en el $ALARM de la caida-.
    //
    // T TAMBIEN SE MARCA, Y POR EL MISMO MOTIVO QUE LOS OTROS TRES (04/09).
    //
    // Aqui ponia `(ahora / 1000UL) % 60UL` -el SEGUNDERO DEL TIEMPO ENCENDIDO, de 0 a
    // 59 y vuelta a empezar-, publicado en un campo que promete los segundos que le
    // quedan a la fase. Igual que el BAT:12.6, se movia y por eso parecia un dato.
    //
    // POR QUE ESTA PUNTA NO PUEDE DAR EL NUMERO, censado y no supuesto: el Esclavo es
    // SUBORDINADO. Su luz la decide el Maestro con CMD_GO_GREEN / CMD_GO_RED, y esas
    // tramas no llevan duracion: dicen QUE hacer, no HASTA CUANDO. El plazo del despeje
    // y el del verde viven en el coordinador del Maestro y en sus modos; aqui no llega
    // ninguno de los dos. Inventar una cuenta atras seria adivinar cuando el otro
    // extremo va a mandar la siguiente orden.
    //
    // LA EXCEPCION QUE SI EXISTE, ESCRITA PARA QUE NO SE PIERDA: en Modo Degradado esta
    // punta SI conoce su fase, porque la calcula por reloj, y el numero ya esta hecho en
    // degradado_segundosParaCambio() -sin llamador desde el menu; D-44 la retiro-. No se
    // publica aqui a proposito: el Maestro NO tiene el getter equivalente -su
    // modo_degradado.cpp calcula el restante para el LCD y no lo expone-, asi que
    // publicarlo en una sola punta dejaria al operario con un numero en un poste y "--"
    // en el otro para el MISMO ciclo. Cuando el Maestro exponga el suyo, las dos se
    // encienden en el mismo commit.
    // N-153: PLUMA: - ESTA PUNTA TAMBIEN TIENE TALANQUERA, Y TAMBIEN LA PUBLICA.
    //
    // No es simetria de oficio: las dos placas son la MISMA y las dos llevan el motor en
    // PB2 -pines.h:31, "opto U15 -> MOSFET Q10 -> bornera J15" en las dos puntas-, asi
    // que cada equipo publica LA SUYA, la que el tecnico tiene delante en ese poste. Es
    // lo contrario del caso de ESC:, que solo emite el Maestro porque el Esclavo no
    // tiene a quien preguntarle por el otro poste.
    //
    // El valor no se recalcula aqui: lo da semaforo.cpp, que es el unico que decide
    // (SFTY-28) y deja anotado lo que escribio en el pin.
    //
    // EL BUFFER SE QUEDA EN 128, Y LA CUENTA SE REHACE POR BUFFER, NO POR RANGO (05/09).
    //
    // Aqui ponia "HORA 8" porque el formato es HH:MM:SS. Es el rango, no la cota: %02u
    // sobre un uint8_t llega a tres cifras, y con horaBuf[16] lo que snprintf permitia
    // escribir eran 15 caracteres. La cuenta buena -la que rehace esp32_07 sin creerse a
    // nadie- acota cada campo por SU BUFFER:
    //
    //   parte fija de la plantilla  84   (los cuatro campos que esta punta no mide
    //                                     viajan como "--" DENTRO del literal)
    //   SERIE                        6   serieTxt[7]
    //   MODO                        11   literal: el mas largo de obtenerNombreModo()
    //                                    ("SUBORDINADO"; "DEGRADADO" 9 y "RENDIDO" 7)
    //   ESTADO                       9   literal "FALLO COM", el mas largo
    //   HORA                        11   horaBuf[12]
    //   PLUMA                        6   literal "ARRIBA"
    //   CAM                          6   literal "PEGADA", el mas largo de camara_estado()
    //
    //   84 + 49 = 133 caracteres + NUL = 134 B, y con el envoltorio 138 B de los 160 de
    //   tramaCompleta.
    //
    // A-11 (05/09): LA CUENTA NO CAMBIA AL PUBLICAR EL MODO REAL, Y NO ES SUERTE. Lo que
    // salio de la plantilla fue el literal "SUBORDINADO" -11 caracteres- y lo que entro
    // es un campo acotado por ESE MISMO literal, que sigue siendo el mas largo que la
    // funcion puede devolver. 95 - 11 = 84 en la parte fija, +11 en el desglose: 133 en
    // los dos lados. Se deja escrita la resta porque la cuenta de arriba se lee como una
    // suma de campos y este es el unico que se movio de columna sin cambiar de tamano.
    //
    // Con horaBuf[16] la misma cuenta daba 126 caracteres y el margen era 1 B: cabia,
    // pero por los pelos y sin que nadie lo hubiera mirado. Aqui no habia trama que no
    // cupiera -eso era el Maestro-; lo que habia era la misma cuenta hecha por rango.
    //
    // D-13 fase 1 (05/09): CAM: - LA MISMA CAMARA QUE VIGILA EL MAESTRO, EN ESTE POSTE.
    //
    // Va en las DOS puntas por lo mismo que PLUMA:: las dos placas son la misma y las
    // dos llevan sus camaras en J16, asi que cada equipo publica LAS SUYAS. No es el
    // caso de ESC:, que solo lo emite el Maestro porque el Esclavo no tiene a quien
    // preguntarle por el otro poste.
    //
    // Y el valor es el PEOR de las dos camaras -el orden de gravedad vive en el enum de
    // botones.cpp, OK < ? < CIEGA < PEGADA-, con "?" pesando MAS que OK a proposito:
    // es el estado de arranque, o sea "todavia no se si esta camara ve", que no es lo
    // mismo que "esta bien".
    //
    // EL BUFFER SUBE DE 128 A 155, Y NO ES HOLGURA: 155 es el maximo que tramaCompleta
    // [160] admite -payload mas el cierre "*XX" con su CR y su LF-, el mismo techo que el
    // Maestro. Con las dos puntas en el techo, la cota que puede fallar es UNA y es la que importa: que el
    // contenido quepa. Aqui el campo cabia subiendo a 134; se pone en el mismo sitio que
    // el Maestro para que las dos plantillas se midan contra el mismo borde y no haya
    // que recordar cual de los dos numeros era el de cada punta.
    // A-11: se asigna a un `const char*` local en vez de llamar dentro del snprintf, y
    // no es estilo: es lo que permite acotarlo. esp32_07_presupuesto_bytes sigue la
    // funcion hasta sus `return "..."` cuando el argumento es una variable asignada
    // desde una llamada; una llamada CON ARGUMENTO metida en el propio snprintf no la
    // sabe acotar y el pack aborta antes que estimar. Es la misma forma que modoStr en
    // el Maestro.
    const char* modoStr = obtenerNombreModo(degradado_estado());

    char payload[155];
    snprintf(payload, sizeof(payload),
             "$STATUS,NODE:ESCLAVO,SERIE:%s,MODO:%s,ESTADO:%s,T:--,RF:--,RTT:--,BAT:--,HORA:%s,PLUMA:%s,CAM:%s",
             serieTxt, modoStr, estadoStr, horaBuf,
             semaforo_plumaFuera() ? "FUERA" : semaforo_plumaArriba() ? "ARRIBA" : "ABAJO",   // D-97/D-150
             interruptores_camNoVeto() ? "NOVETO" : camara_estado());                        // D-151

    enviarTramaConCrc(payload);

    // N-108 - EL $EVENT QUE FALTABA ES EL DE LA VUELTA DEL ENLACE, NO EL DE LA CAIDA.
    //
    // La caida YA tiene dueno y ya deja rastro: SFTY-6 la decide en main.cpp y manda el
    // $ALARM, que desde hoy lleva ademas los tres contadores de linea. Lo que no anunciaba
    // nadie era el REGRESO, y por eso en la caja negra una radio que va y viene con la
    // lluvia -el reporte del 27/08- se leia como una sola caida en vez de como doce. Sin
    // la vuelta no se puede contar el numero de cortes, que es el dato que separa "se
    // mojo el conector" de "esta al limite de alcance".
    //
    // AQUI NO HAY RELOJ NI UMBRAL, Y ES DELIBERADO. La primera version media el silencio
    // con SFTY6_SILENCIO_MS y el simulador del puente la tumbo con razon: ese umbral es el
    // del watchdog de la RADIO, y traerlo a este fichero -el del puerto del telefono- es
    // la precondicion de que alguien acabe alimentando uno con el otro. El dia que eso
    // pase, un ESP32 colgado mandaria el cruce a ambar y, peor, un telefono conectado
    // SALVARIA al cruce de una caida de radio real. Son dos silencios y dos instrumentos.
    //
    // Asi que este bloque no decide nada: OBSERVA. La caida la declara quien manda la
    // alarma; aqui solo se anota que ocurrio y se avisa cuando vuelven a entrar tramas.
    // Vale con "cualquier alarma" porque en esta punta hay UNA sola llamada a
    // bluetooth_reportarAlarma() -main.cpp:577, MEDIDO- y es justo la del fallo de RF; si
    // algun dia hay otra, el peor caso es un aviso de vuelta de mas, que se ve.
    {
      static unsigned long validasAnt = 0;
      const unsigned long validas = protocolo_tramasValidas();
      if (enlaceCaidoAnunciado && validas != validasAnt) {
        enlaceCaidoAnunciado = false;
        char det[40];
        snprintf(det, sizeof(det), "RECUPERADO_OK:%lu_RUIDO:%lu",
                 validas, protocolo_tramasDescartadas());
        bluetooth_reportarEvento("ENLACE_RF", det);
      }
      validasAnt = validas;
    }
  }

  // D-23 (13/09) - EL DIAGNOSTICO DEL ENLACE DE ESTA PUNTA, A SU PROPIA CADENCIA.
  // VA FUERA DEL if DEL $STATUS Y CON SU PROPIO RELOJ, y no es colocacion: colgando de
  // aquel, la cadencia de este dato dejaria de ser la que dice DIAG_ENLACE_MS para pasar
  // a ser un multiplo del periodico, y ademas los DOS instrumentos que leen
  // `ahora - tUltimaTelemetria >= N` por TEXTO -esp32_07_presupuesto_bytes y
  // simulador_puente_esp32- estarian midiendo un bloque que ya no es solo el $STATUS.
  //
  // LOS TRES CONTADORES SON LOS MISMOS QUE LOS DEL $ALARM, Y ESA ES LA MITAD DEL VALOR:
  // el tecnico compara la linea de un poste sano con la de la caida sin traducir nada.
  // Lo que cambia es el SEPARADOR -espacios y no comas- y eso NO es estilo: _camposNmea()
  // de la app parte la trama por ',' y cada trozo por su PRIMER ':', asi que una coma
  // dentro del DETALLE convierte lo que va detras en campos sueltos que el pintor de
  // $EVENT no mira; los bits saldrian al cable y no llegarian a la pantalla. Es el mismo
  // motivo que ya lleva escrito el $EVENT de ORIGEN:RELOJ del Maestro.
  //
  // LA COTA DE det[] ES LA DE tramo[45] DEL $ALARM, POR EL MISMO MOTIVO Y CON LA MISMA
  // CUENTA -el porque entero esta alli y no se repite-: los tres son CONTADORES LIBRES de
  // protocolo.cpp, sin techo que prometer, asi que se acotan a su tope de TIPO, que son
  // las diez cifras de un unsigned long. Parte fija de ESTE formato: "RX:" + " OK:" +
  // " RUIDO:" = 14, los mismos 14 que la del $ALARM. 14 + 30 = 44 caracteres + NUL = 45.
  // Inventarles un techo para estrechar el buffer seria una cota escrita en vez de
  // medida. Que el $EVENT ENTERO quepa luego en su payload no se afirma aqui: lo
  // recalcula esp32_07_presupuesto_bytes leyendo este snprintf y el del emisor.
  //
  // EL ORIGEN ES ENLACE_RF Y NO UNO NUEVO: es el MISMO sujeto que el $EVENT de la vuelta
  // del enlace -como ve ESTA punta su radio-, y la app ya los distingue por el DETALLE.
  if (ahora - tUltimoDiagEnlace >= DIAG_ENLACE_MS) {
    tUltimoDiagEnlace = ahora;
    char det[61];  // D-60: " REIN:" (6) y un %u por TIPO (10): 44 + 16 + NUL
    snprintf(det, sizeof(det), "RX:%lu OK:%lu RUIDO:%lu REIN:%u",
             protocolo_bytesRecibidos(), protocolo_tramasValidas(),
             protocolo_tramasDescartadas(), (unsigned)respaldo_reinicios());
    bluetooth_reportarEvento("ENLACE_RF", det);
  }

  diagDegradadoPublicar(ahora);
}
