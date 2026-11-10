// ===== src/bluetooth_acciones.inc (MAESTRO) =====
// EL CUERPO DE procesarComando() DESDE LA GUARDA DE PIN: la cadena de ordenes sobre 'accion', en el orden de
// siempre y con el molde SET_TIEMPOS (CLAUDE.md 2). Una orden nueva entra aqui o, mejor, como <modulo>_orden().
//
// NO ES UN FICHERO SUELTO: lo incluye bluetooth_ordenes.inc DENTRO de procesarComando(), donde estaba. Salio por el
// tope de 500 lineas (roadmap, "Partir"), literal; los instrumentos lo leen dentro de bluetooth.cpp, en su sitio
// (banco/fuente.py, expandir_inc; CLAUDE.md 5).
  if (strcmp(accion, "SET_MODO:AUTO") == 0) {
    modoActual_set(MODO_AUTOMATICO);
    coordinador_iniciarModo();
    enviarTramaConCrc("$ACK,CMD:SET_MODO:AUTO,RESULT:OK");
    bluetooth_reportarEvento("APP_BLUETOOTH", "SET_MODO_AUTO");
  } else if (strcmp(accion, "SET_MODO:MANUAL") == 0) {
    // N-147: en Manual se entra por el todo-rojo QUE NO PROGRAMA NADA. El porque
    // completo esta en modoManual_setup(); aqui se repite la llamada y no el motivo.
    modoActual_set(MODO_MANUAL);
    coordinador_forzarRojoTotal();
    enviarTramaConCrc("$ACK,CMD:SET_MODO:MANUAL,RESULT:OK");
    bluetooth_reportarEvento("APP_BLUETOOTH", "SET_MODO_MANUAL");
  } else if (strcmp(accion, "SET_MODO:AMBAR") == 0) {
    // N-146 (05/09): EL AMBAR CONTESTABA OK Y NO ENCENDIA NADA (cinta del 04/09: seis pulsaciones, seis OK, cruce en
    // rojo). main.cpp solo llama modo_ambar_setup() EN EL FLANCO (modo != modoAnterior), y al par (MODO_AMBAR, luz
    // en rojo) se llega por CMD:FORZAR_ROJO, que cambia LA LUZ y no el MODO. Se re-arma y se contesta DISTINTO
    // (CLAUDE.md 2: un $ACK depende de lo que se hizo). Re-armar manda un todo-rojo: por eso NO se hace desde el
    // aviso repetido del Esclavo (N-142, main.cpp); aqui lo pide una persona. N-152: el motivo se fija aqui para que
    // el aviso de cancelacion del Esclavo no saque al cruce de un ambar pedido en este poste. Ver modo_ambar.h.
    modo_ambar_fijarMotivo("Ambar pedido desde", "la app (celular)");
    const bool yaEnModo = (modoActual_get() == MODO_AMBAR);
    modoActual_set(MODO_AMBAR);
    if (yaEnModo) {
      modo_ambar_setup();
      enviarTramaConCrc("$ACK,CMD:SET_MODO:AMBAR,RESULT:REARMADO");
    } else {
      enviarTramaConCrc("$ACK,CMD:SET_MODO:AMBAR,RESULT:OK");
    }
    bluetooth_reportarEvento("APP_BLUETOOTH", "SET_MODO_AMBAR");
  } else if (strcmp(accion, "SET_MODO:MENU") == 0) {
    // EN DEGRADADO NO SE SALTA AL MENU. Se pide la salida, que es el todo-rojo de 30 s
    // de ROJO_TRANSICION_MS. Sin el, esta unidad cae a rojo mientras la otra sigue
    // dando verde por reloj, y el escenario peligroso de ese modo es exactamente que
    // UNA SOLA PUNTA lo abandone. Es la MISMA puerta que el boton 4 del gabinete.
    if (modoActual_get() == MODO_DEGRADADO) {
      // El bool distingue haber arrancado la salida de encontrarla en marcha; en ninguno se contesta OK: el menu
      // tarda todavia el todo-rojo entero en llegar.
      if (modo_degradado_pedirSalida()) {
        enviarTramaConCrc("$ACK,CMD:SET_MODO:MENU,RESULT:SALIENDO_TODO_ROJO");
        bluetooth_reportarEvento("APP_BLUETOOTH", "SALIDA_DEGRADADO_PEDIDA");
      } else {
        enviarTramaConCrc("$ERR,CMD:SET_MODO:MENU,DESC:YA_VUELVE_AL_MENU");
      }
    } else {
      const bool revocada = modo_degradado_revocarEsperaSiembra();   // D-51: la ventana de D-29 se cierra
      modoActual_set(MENU);
      menu_setup();
      if (revocada) { enviarTramaConCrc("$ACK,CMD:SET_MODO:MENU,RESULT:OK_REANUDACION_CANCELADA");
      } else { enviarTramaConCrc("$ACK,CMD:SET_MODO:MENU,RESULT:OK"); }
      bluetooth_reportarEvento("APP_BLUETOOTH", "SET_MODO_MENU");
    }
  } else if (strcmp(accion, "SET_MODO:INTELIGENTE") == 0) {
    // PIDE PIN porque arranca un ciclo que DA VERDES, y no se entra desde el Degradado: su salida obliga a un
    // todo-rojo de 30 s en las dos puntas, y ciclar sin cumplirlo deja a la otra unidad en su ciclo por reloj.
    // D-65 (CLAUDE.md 2): con el modo ya puesto main.cpp no corre nada (solo en el flanco): SIN_CAMBIO y nada mas,
    // sin re-armar (un REARMADO correria un todo-rojo que nadie pidio; se reinicia pasando por MENU).
    // D-167: SET_MODO:ALCANCE ya no tiene rama y cae en el else final (COMANDO_NO_SOPORTADO).
    if (modoActual_get() == MODO_DEGRADADO) {
      enviarTramaConCrc("$ERR,CMD:SET_MODO:INTELIGENTE,DESC:EN_MARCHA_PARE_EL_MODO");
    } else if (modoActual_get() == MODO_INTELIGENTE) {   // D-65: yaEnModo, leido ANTES de modoActual_set()
      enviarTramaConCrc("$ACK,CMD:SET_MODO:INTELIGENTE,RESULT:SIN_CAMBIO");
    } else {
      modoActual_set(MODO_INTELIGENTE);
      enviarTramaConCrc("$ACK,CMD:SET_MODO:INTELIGENTE,RESULT:OK");
      bluetooth_reportarEvento("APP_BLUETOOTH", "SET_MODO_INTELIGENTE");
    }
  } else if (strncmp(accion, "SET_MODO:DEG_T:", 15) == 0) {
    // D-35 (SPEC_2 7.bis): EL DEGRADADO CON TESTIGO. D-46: es la unica orden de entrada; SET_MODO:DEGRADADO salio y
    // cae en COMANDO_NO_SOPORTADO. Formato ahora,inicio,verde,despeje = HH:MM:SS,HH:MM:SS,v,d: aqui solo se traduce
    // texto (molde SET_TIEMPOS, con el %c que delata lo que sobra). Los rangos (D-59: verde 180..900, x60) y la
    // hora los decide modo_degradado_entrarTestigo() y la respuesta sale de lo que ESA llamada devolvio, un $ERR por
    // motivo.
    int ah = -1, am = -1, as = -1, ih = -1, im = -1, is = -1, v = 0, d = 0;
    char sobra = 0;
    const int n = sscanf(accion + 15, "%d:%d:%d,%d:%d:%d,%d,%d%c",
                         &ah, &am, &as, &ih, &im, &is, &v, &d, &sobra);
    if (n != 8 || !horaDelDiaValida(ah, am, as) || !horaDelDiaValida(ih, im, is)) {
      enviarTramaConCrc("$ERR,CMD:SET_MODO:DEG_T,DESC:FORMATO_INVALIDO");
    } else if (modo_degradado_salidaS() != 0) {   // D-52: ni renueva ni entra en silencio sobre una salida fijada
      enviarTramaConCrc("$ERR,CMD:SET_MODO:DEG_T,DESC:Cancele antes la salida programada");
    } else {
      const MotivoTestigo m = modo_degradado_entrarTestigo(
          (uint32_t)ah * 3600UL + (uint32_t)am * 60UL + (uint32_t)as,
          (uint32_t)ih * 3600UL + (uint32_t)im * 60UL + (uint32_t)is, v, d);
      if (m == MDT_OK) {
        enviarTramaConCrc("$ACK,CMD:SET_MODO:DEG_T,RESULT:OK");
        bluetooth_reportarEvento("APP_BLUETOOTH", "SET_MODO_DEG_T");
      } else if (m == MDT_RENOVADO) {
        enviarTramaConCrc("$ACK,CMD:SET_MODO:DEG_T,RESULT:RENOVADO");
        bluetooth_reportarEvento("APP_BLUETOOTH", "SET_MODO_DEG_T_RENOVADO");
      } else {
        char p[80];   // 29 del prefijo + 31 del motivo mas largo + NUL
        snprintf(p, sizeof(p), "$ERR,CMD:SET_MODO:DEG_T,DESC:%s", modo_degradado_textoTestigo(m));
        enviarTramaConCrc(p);
      }
    }
  } else if (strncmp(accion, "SET_MODO:DEG_FIN:", 17) == 0) {   // D-52, SPEC_2 7.quater (c): molde DEG_T
    int ah = -1, am = -1, as = -1, sh = -1, sm = -1, ss = -1;
    char sobra = 0;
    const bool cancelar = strcmp(accion + 17, "CANCELAR") == 0;
    const int n = cancelar ? 0 : sscanf(accion + 17, "%d:%d:%d,%d:%d:%d%c", &ah, &am, &as, &sh, &sm, &ss, &sobra);
    if (!cancelar && (n != 6 || !horaDelDiaValida(ah, am, as) || !horaDelDiaValida(sh, sm, ss))) {
      enviarTramaConCrc("$ERR,CMD:SET_MODO:DEG_FIN,DESC:FORMATO_INVALIDO");
    } else {
      const MotivoSalida m = cancelar ? modo_degradado_cancelarSalida() : modo_degradado_programarSalida(
          (uint32_t)ah * 3600UL + (uint32_t)am * 60UL + (uint32_t)as,
          (uint32_t)sh * 3600UL + (uint32_t)sm * 60UL + (uint32_t)ss);
      char p[96];   // 31 del prefijo + 30 del motivo mas largo + NUL
      if (m <= MDF_CANCELADA) {
        enviarTramaConCrc(m == MDF_PROGRAMADA ? "$ACK,CMD:SET_MODO:DEG_FIN,RESULT:PROGRAMADA"
            : m == MDF_REPROGRAMADA ? "$ACK,CMD:SET_MODO:DEG_FIN,RESULT:REPROGRAMADA"
            : m == MDF_PROGRAMADA_SIN_RESPALDO ? "$ACK,CMD:SET_MODO:DEG_FIN,RESULT:PROGRAMADA_SIN_RESPALDO"
            : "$ACK,CMD:SET_MODO:DEG_FIN,RESULT:CANCELADA");
        char det[40];
        snprintf(det, sizeof(det), cancelar ? "SALIDA_CANCELADA" : "SALIDA_PROGRAMADA_%02d:%02d:%02d", sh, sm, ss);
        bluetooth_reportarEvento("DEGRADADO", det);
      } else {
        snprintf(p, sizeof(p), "$ERR,CMD:SET_MODO:DEG_FIN,DESC:%s", modo_degradado_textoSalida(m));
        enviarTramaConCrc(p);
      }
    }
  } else if (strcmp(accion, "CONSULTA_DEG_FIN") == 0) {
    // D-52: solo mira. SALE_HHMMSS_FALTAN_<s>S_<RESPALDADA|SIN_RESPALDO>, sin ':' dentro del valor.
    const uint32_t sal = modo_degradado_salidaS();
    if (sal == 0) {
      enviarTramaConCrc("$ACK,CMD:CONSULTA_DEG_FIN,RESULT:NINGUNA");
    } else {
      const uint32_t dia = sal % 86400UL;
      uint32_t falta = (dia + 86400UL - reloj_segundosDelDia()) % 86400UL;
      if (falta > 43200UL) falta = 0;   // ya paso: sale en esta vuelta
      char p[80];
      snprintf(p, sizeof(p), "$ACK,CMD:CONSULTA_DEG_FIN,RESULT:SALE_%02lu%02lu%02lu_FALTAN_%luS_%s",
               (unsigned long)(dia / 3600UL), (unsigned long)(dia / 60UL % 60UL), (unsigned long)(dia % 60UL),
               (unsigned long)falta, modo_degradado_salidaRespaldada() ? "RESPALDADA" : "SIN_RESPALDO");
      enviarTramaConCrc(p);
    }
  } else if (strncmp(accion, "SET_DEG_AUTO:", 13) == 0) {
    // A-15 (SPEC_2 7.ter (a)): la opcion del Degradado automatico. Molde SET_TIEMPOS: el
    // %c delata lo que sobra, y la respuesta sale de lo que devolvio degAuto_orden(). Si se
    // acepta NO se contesta aqui: el $ACK sale de bluetooth_loop() cuando el ECO del Esclavo
    // refleja el cambio (molde REINICIAR_RELOJ).
    int v = -1;
    char sobra = 0;
    if (sscanf(accion + 13, "%d%c", &v, &sobra) != 1 || (v != 0 && v != 1)) {
      enviarTramaConCrc("$ERR,CMD:SET_DEG_AUTO,DESC:FORMATO_INVALIDO");
    } else {
      const DegAutoOrden r = degAuto_orden(v == 1);
      if (r == DAO_SIN_ENLACE) {
        enviarTramaConCrc("$ERR,CMD:SET_DEG_AUTO,DESC:SIN_ENLACE_CON_EL_OTRO_POSTE");
      } else if (r == DAO_EN_DEGRADADO) {
        enviarTramaConCrc("$ERR,CMD:SET_DEG_AUTO,DESC:EN_DEGRADADO_SALGA_PRIMERO");
      } else {
        bluetooth_reportarEvento("APP_BLUETOOTH", v == 1 ? "SET_DEG_AUTO_1" : "SET_DEG_AUTO_0");
      }
    }
  } else if (strcmp(accion, "CONSULTA_DEG_AUTO") == 0) {
    // A-15: la opcion propia, el ultimo APTO oido del otro y el APTO propio. No toca nada.
    char p[64];
    snprintf(p, sizeof(p), "$ACK,CMD:CONSULTA_DEG_AUTO,RESULT:ESTE_%s_OTRO_%s_APTO_%s",
             respaldo_degAuto() ? "ON" : "OFF", respaldo_otroApto() ? "ON" : "OFF",
             degAuto_aptoPropio() ? "SI" : "NO");
    enviarTramaConCrc(p);
  } else if (strcmp(accion, "FORZAR_ROJO") == 0) {
    const ResultadoRojoTotal r = modo_degradado_forzarRojo();   // D-51: en Degradado, rojo fijo en ESTE poste
    if (r == RRT_ROJO_FIJO) { enviarTramaConCrc("$ACK,CMD:FORZAR_ROJO,RESULT:ROJO_FIJO_EN_ESTE_POSTE");
    } else if (r == RRT_YA_EN_ROJO_FIJO) { enviarTramaConCrc("$ACK,CMD:FORZAR_ROJO,RESULT:YA_EN_ROJO_FIJO");
    } else if (r == RRT_SALIDA_EN_CURSO) { enviarTramaConCrc("$ACK,CMD:FORZAR_ROJO,RESULT:SALIDA_YA_EN_CURSO");
    } else if (r == RRT_REANUDACION_CANCELADA) {
      enviarTramaConCrc("$ACK,CMD:FORZAR_ROJO,RESULT:OK_REANUDACION_CANCELADA");
    } else { enviarTramaConCrc("$ACK,CMD:FORZAR_ROJO,RESULT:OK"); }
    bluetooth_reportarEvento("APP_BLUETOOTH", "FORZAR_ROJO_TOTAL");
  } else if (strcmp(accion, "MANUAL:CAMBIAR_TURNO") == 0) {
    // Antes se mandaba OK pasara lo que pasara. coordinador_pedirCambio() abandona en
    // silencio si el coordinador no esta en reposo -esta a mitad de una transicion, que
    // incluye un todo-rojo de despeje en curso-, asi que el operario recibia la
    // confirmacion, no cambiaba nada, y volvia a pulsar creyendo que no le hacian caso.
    //
    // No se fuerza el cambio: el rechazo es correcto -partir un despeje por la mitad es
    // justo lo que no se puede hacer-. Lo que faltaba era DECIRLO.
    // N-151: el modo se mira ANTES que el estado del coordinador, y el orden importa.
    // Al reves, la primera pulsacion en ambar volveria a colarse -alli estadoC vale
    // C_IDLE- y volveria a trabar el cruce, que es el defecto entero.
    if (!modoMueveElCoordinador()) {
      enviarTramaConCrc("$ERR,CMD:CAMBIAR_TURNO,DESC:MODO_SIN_CICLO_SALGA_PRIMERO");
    } else if (pedirCambioVerificado()) {
      enviarTramaConCrc("$ACK,CMD:CAMBIAR_TURNO,RESULT:OK");
      bluetooth_reportarEvento("APP_BLUETOOTH", "CAMBIAR_TURNO_MANUAL");
    } else {
      enviarTramaConCrc("$ERR,CMD:CAMBIAR_TURNO,DESC:EN_TRANSICION_REINTENTE");
    }
  } else if (strcmp(accion, "TEST_LEDS") == 0) {
    // N-82.bis: el test solo corre fuera de servicio -el porque, en testLedsAdmitido()
    // de semaforo.cpp, que es quien decide-. Aqui se PREGUNTA si quedo armado, dentro
    // del if, y cada motivo de rechazo lleva su $ERR (molde SET_TIEMPOS).
    semaforo_iniciarTestLeds();
    if (semaforo_testLedsEnCurso()) {
      enviarTramaConCrc("$ACK,CMD:TEST_LEDS,RESULT:STARTING_6S");
      bluetooth_reportarEvento("APP_BLUETOOTH", "TEST_LEDS_INICIADO");
    } else if (semaforo_estado() == S_FALLO) {
      enviarTramaConCrc("$ERR,CMD:TEST_LEDS,DESC:SIN_ENLACE_AMBAR_NO_SE_PRUEBA");
    } else if (modoActual_get() == MENU && !coordinador_rojoEsclavoConfirmado()) {
      enviarTramaConCrc("$ERR,CMD:TEST_LEDS,DESC:ESPERANDO_ROJO_DEL_ESCLAVO");
    } else {
      enviarTramaConCrc("$ERR,CMD:TEST_LEDS,DESC:EN_SERVICIO_PASE_A_MENU");
    }
  } else if (strncmp(accion, "SET_TIEMPOS:", 12) == 0) {
    // N-69: los tiempos del ciclo, desde el celular en vez de subiendo a la pantalla.
    //
    // Los limites NO se comprueban aqui: los tiene modoAutomatico_fijarTiempos(), que
    // es quien conoce el ciclo. Este sitio solo traduce texto a numeros. Repetir los
    // rangos en los dos lados seria una segunda copia que alguien tendria que
    // sincronizar -y el dia que difieran, la app dejaria pasar lo que el ciclo rechaza-.
    //
    // EL %c DE MAS NO SE LEE NUNCA: ESTA PARA QUE sscanf DELATE LO QUE SOBRA.
    //
    // sscanf cuenta CAMPOS CONVERTIDOS y no exige que la cadena se acabe: con dos ordenes
    // pegadas -"8,8,50CMD:FORZAR_ROJO", una trama a medias y la siguiente detras- convertia
    // 3, ejecutaba la VIEJA con OK y perdia la nueva (quiza el rojo de emergencia). Con el
    // %c una cadena limpia convierte 3 y una con basura 4: se exige el 3 EXACTO y lo demas
    // es FORMATO_INVALIDO; ante dos ordenes pegadas no se ejecuta ninguna y se dice.
    int v = 0, r = 0, d = 0;
    char sobra = 0;
    if (sscanf(accion + 12, "%d,%d,%d%c", &v, &r, &d, &sobra) != 3) {
      enviarTramaConCrc("$ERR,CMD:SET_TIEMPOS,DESC:FORMATO_INVALIDO");
    } else if (modoAutomatico_enMarcha()) {
      // Con el ciclo corriendo no se tocan: bajar un tiempo a mitad de fase acortaria
      // la fase EN CURSO, y una de esas fases es el todo-rojo de despeje.
      enviarTramaConCrc("$ERR,CMD:SET_TIEMPOS,DESC:EN_MARCHA_PARE_EL_MODO");
    } else if (!modoAutomatico_fijarTiempos(v, r, d)) {   // D-131: sin (uint8_t), 259 no es 3
      enviarTramaConCrc("$ERR,CMD:SET_TIEMPOS,DESC:RANGO");
    } else {
      enviarTramaConCrc("$ACK,CMD:SET_TIEMPOS,RESULT:OK");
      bluetooth_reportarEvento("APP_BLUETOOTH", "TIEMPOS_CAMBIADOS");
    }
  } else if (strcmp(accion, "REINICIAR_RELOJ") == 0) {
    // N-31. PIDE PIN porque BORRA LA HORA Y TODO EL RESPALDO -ciclo acordado, marca de
    // sincronizacion e indicador del Degradado-, o sea la autorizacion de la que cuelga
    // el unico modo que da verde sin el otro extremo.
    //
    // El bool no se ignora: dice si el oscilador arranco tras el reinicio. Contestar OK
    // en los dos casos mandaria al tecnico a poner la hora en un equipo que no puede
    // contarla. Y no se nombra ninguna pieza -N-45 quito "Es Y2: toca hardware" de la
    // pantalla por afirmar sin haber medido-: lo que sigue lo dice CONSULTA RELOJ.
    //
    // 1.49(a) - Y "ARRANCO" NO ES "CUENTA": EL SI SE DA CUANDO SE SABE, NO ANTES. Aqui salia
    // CRISTAL_OK_PONGA_LA_HORA en cuanto el oscilador arrancaba, y el cristal que arranca y
    // no cuenta -1.22, la cinta del Sisga- lo detecta vigilarCristal() CNT_VENTANA_MS
    // despues: el tecnico se iba a poner la hora en un equipo que no la puede contar, con un
    // $ACK en la mano. Ahora la rama NO contesta: deja la orden pendiente, publica que esta
    // verificando, y el veredicto -$ACK o $ERR, uno de los dos y una sola vez- sale de
    // bluetooth_loop() cuando reloj_estadoCristal() lo tiene. Sin bloquear: la ventana son
    // dos segundos y el perro muerde a los cuatro.
    //
    // Y UNA SEGUNDA ORDEN MIENTRAS SE VERIFICA SE RECHAZA SIN TOCAR NADA. Volver a reiniciar
    // el dominio reanclaria la vigilancia -otra ventana entera- y dejaria dos ordenes
    // esperando un solo veredicto: una de las dos se quedaria sin respuesta.
    if (pendienteReloj) {
      enviarTramaConCrc("$ERR,CMD:REINICIAR_RELOJ,DESC:REPITA_EN_UNOS_SEGUNDOS");
    } else if (reloj_reiniciarDominioRespaldo()) {
      pendienteReloj = true;
      bluetooth_reportarEvento("APP_BLUETOOTH", "RELOJ_REINICIADO_VERIFICANDO");
    } else {
      enviarTramaConCrc("$ERR,CMD:REINICIAR_RELOJ,DESC:SIGUE_PARADO_VEA_CONSULTA_RELOJ");
      // El mismo motivo que tenia SET_RTC -este $ERR nombra una consulta que no se puede
      // abrir. Y aqui el dato vale mas todavia, porque el dominio de respaldo ACABA de
      // reiniciarse: los bits dicen si el oscilador se quedo sin pedir -lseOn=0, que no
      // es el cristal- o pedido y sin arrancar -lseOn=1, lseRdy=0, que si lo es-.
      reportarBitsDelReloj();
    }
  } else if (strcmp(accion, "DEMANDA") == 0) {
    // NO ES SOLICITAR_PASO, y la diferencia no es de nombre: alli el Esclavo PIDE a
    // este equipo (SFTY-27), y aqui el que decide ya es este. La peticion entra por la
    // misma puerta que la camara y la gobierna modo_inteligente.cpp.
    if (modoActual_get() != MODO_INTELIGENTE) {
      // Fuera del Modo Inteligente nadie lee la demanda: registrarla y contestar OK
      // seria apuntar una peticion que ningun ciclo va a mirar.
      enviarTramaConCrc("$ERR,CMD:DEMANDA,DESC:SOLO_EN_MODO_INTELIGENTE");
    } else if (demanda_solicitar()) {
      enviarTramaConCrc("$ACK,CMD:DEMANDA,RESULT:REGISTRADA");
      bluetooth_reportarEvento("APP_BLUETOOTH", "DEMANDA_LOCAL");
    } else {
      // No se finge una peticion que se descarto: si el operario no sabe que su
      // pulsacion cayo en la ventana de silencio, volvera a pulsar creyendo que no le
      // hacen caso.
      enviarTramaConCrc("$ERR,CMD:DEMANDA,DESC:REPITA_EN_UNOS_SEGUNDOS");
    }
  } else if (strcmp(accion, "REINICIOS") == 0) {
    // D-60/D-61/D-62: solo mira y no va al Diario, como VERSION. char[96]: peor caso 89 con
    // las cotas de tipo (uint8 REIN y ANTES). El $ACK depende de que la trama cupiera.
    char r[96];
    const int n = snprintf(r, sizeof(r),
        "$ACK,CMD:REINICIOS,RESULT:OK,NODE:MAESTRO,REIN:%u,CAUSA:%s,UP:%lu,ANTES:%u",
        (unsigned)respaldo_reinicios(), respaldo_causa(),
        (unsigned long)respaldo_segundosEncendido(), (unsigned)respaldo_horasAnterior());
    if (n > 0 && n < (int)sizeof(r)) enviarTramaConCrc(r);
    else enviarTramaConCrc("$ERR,CMD:REINICIOS,DESC:NO_CABE,NODE:MAESTRO");
  } else if (strcmp(accion, "VERSION") == 0) {
    // 1.51 - EL EQUIPO DICE QUE FIRMWARE LLEVA. Ver version_fw.h para el sello.
    //
    // POR QUE NO VA EN EL $STATUS, AUNQUE FUERA LO COMODO. MEDIDO: el payload de ese
    // $STATUS es de 155 B y es EL BORDE DEL CABLE de este proyecto -su propio comentario
    // lo dice unas lineas mas abajo, y el Esclavo esta en el mismo techo-. Meter el sello
    // ahi le come el margen a los campos que cambian cada dos segundos para publicar uno
    // que no cambia NUNCA mientras el equipo esta encendido, y lo publica 43.200 veces al
    // dia. Va en su propia trama, que sale cuando alguien pregunta.
    //
    // NO TOCA NADA Y POR ESO NO PIDE PIN. La guarda de arriba lo acepta por las dos
    // puertas; el motivo entero esta escrito alli y no se repite.
    //
    // LLEVA NODE:, Y ES EL PRIMER $ACK DE ESTA PUNTA QUE LO LLEVA. No es simetria: es que
    // esta trama esta hecha para VIAJAR SOLA -pegada en un acta, en un diario de campo o
    // en un mensaje-, y sin el nodo dentro no se puede decir de que poste sale. En los
    // demas acuses el nodo lo dice el $STATUS que va al lado (app.js lo deja escrito); un
    // sello sin nodo es exactamente la atribucion que CLAUDE.md 0.2 viene a cerrar, movida
    // un metro. El parser de la app parte por ',' y luego por el primer ':', asi que un
    // campo mas entra sin tocar una linea de JavaScript.
    //
    // LAS DOS RESPUESTAS LLEVAN SU LITERAL DENTRO DE SU RAMA (N-89). Sacarlas a un
    // compositor dejaria a los dos bloques sin literal y el pack seguiria verde midiendo
    // nada. Y la del #else NO es relleno: es lo que contesta un binario compilado sin
    // sello -sin git, o el arnes de banco que compila este mismo .cpp con g++ de host-, y
    // es un $ERR y no un $ACK porque CLAUDE.md 2 no admite un OK que no depende de lo que
    // se pudo componer: el equipo no sabe que firmware lleva, y decirlo con RESULT:OK
    // seria la mentira con formato de exito.
    //
    // NO HAY snprintf NI BUFFER: el sello es un hecho del tiempo de compilacion, asi que
    // la trama entera es UN literal concatenado por el preprocesador. Por eso aqui no
    // puede existir el truncado que vigila vigilante_parteDeArranque(): no hay nada que
    // truncar. La cota -que el literal quepa en el tramaCompleta[160] de
    // enviarTramaConCrc() con su *XX y su CRLF- la RECALCULA version_01_sello_de_firmware
    // en cada corrida, leyendo los dos numeros del C++. No se pone un static_assert aqui
    // porque haria falta un nombre para ese 160, y esp32_07 y esp32_09 leen esa
    // declaracion por TEXTO -"char tramaCompleta[(\d+)]"-: nombrarla los ABORTA a los dos
    // (CLAUDE.md 5).
#if FW_SELLADO
    enviarTramaConCrc("$ACK,CMD:VERSION,RESULT:OK,NODE:MAESTRO,FW:" FW_TEXTO);
#else
    enviarTramaConCrc("$ERR,CMD:VERSION,DESC:SIN_SELLAR,NODE:MAESTRO,FW:" FW_SIN_SELLO);
#endif
    // NO se anota en el Diario. Preguntar la version no cambia el equipo, y una linea por
    // consulta inunda la bitacora donde hay que encontrar el fallo -es el umbral de J17 y
    // N-73 otra vez-. Lo que queda es la respuesta, que la app ya guarda.
  } else if (const char* r = interruptores_orden(accion)) {   // D-94: molde SET_TIEMPOS, alli
    enviarTramaConCrc(r);
  } else if (const char* r = instalacion_orden(accion)) {     // N-187: local, sin radio
    enviarTramaConCrc(r);
  } else {
    enviarTramaConCrc("$ERR,CMD:DESCONOCIDO,DESC:COMANDO_NO_SOPORTADO");
  }
