// ===== src/bluetooth_acciones.inc (ESCLAVO) =====
// EL CUERPO DE procesarComando() DESDE LA GUARDA DE PIN: la cadena de ordenes sobre 'accion', en el orden de
// siempre y con el molde SET_TIEMPOS (CLAUDE.md 2). Una orden nueva entra aqui o, mejor, como <modulo>_orden().
//
// NO ES UN FICHERO SUELTO: lo incluye bluetooth_ordenes.inc DENTRO de procesarComando(), donde estaba. Salio por el
// tope de 500 lineas (roadmap, "Partir"), literal; los instrumentos lo leen dentro de bluetooth.cpp, en su sitio
// (banco/fuente.py, expandir_inc; CLAUDE.md 5).
  // LA MISMA PUERTA QUE ARRIBA, CON EL PIN PUESTO, Y CON EL MISMO BLOQUE LETRA POR LETRA. El porque de cada fila esta
  // escrito una vez, en la puerta sin PIN. Lo que NO se puede repartir es el codigo: los packs leen el bloque de CADA
  // rama, y una respuesta en una funcion comun dejaria a los dos instrumentos midiendo un bloque vacio (N-89).
  if (strcmp(accion, "AMBAR_EMERGENCIA") == 0) {
    if (!degradado_gobiernaLuz()) {
      const bool yaEnAmbar = (semaforo_estado() == S_FALLO);
      semaforo_iniciarFallo();
      ambarEmergencia = true;

      // N-142 (04/09): SE LE DICE AL MAESTRO. Antes solo se le decia al TELEFONO: sin el aviso el Maestro no podia
      // enterarse (esta punta sigue contestando PONG en ambar) y si estaba en VERDE convivian Maestro en verde y
      // Esclavo en ambar hasta 3 minutos, con los dos sentidos pudiendo entrar al carril.
      //
      // D-31 (12/09) - ESTE PARRAFO SE REESCRIBE: DECIA "SIN ESPERAR ACUSE Y SIN REINTENTO" Y ESO ERA EL DEFECTO. No
      // se bloquea el bucle: el ambar se enciende arriba, el $ACK sale abajo y un plazo de bluetooth_loop() manda, si
      // vence, un DESMENTIDO por $ALARM (patron de CMD_ACK_DEMANDA, N-130). Hacia falta porque la red anterior NO
      // CUBRE LA AVERIA FEA: si solo muere el TRANSMISOR de esta punta, el Maestro le sigue hablando, no agota
      // reintentos y el tecnico se va con un $ACK identico al de la radio sana (bloque H4 del arnes de las dos
      // puntas).
      protocolo_enviarPaquete(CMD_AMBAR_ESCLAVO);
      avisoAmbarSalio();

      // El $ACK dice si esa trama puede haber sido oida. El porque completo -y por que
      // no se mira lo que devolvio la llamada, que es void, y que promete cada literal-
      // esta en la puerta sin PIN.
      const bool sinRadio = enlaceCaidoAnunciado;
      const bool yaAvisado = avisoAmbarConfirmado && !sinRadio;
      if (yaEnAmbar) {
        enviarTramaConCrc(sinRadio
            ? "$ACK,CMD:AMBAR_EMERGENCIA,RESULT:YA_EN_AMBAR_LATCH_PUESTO_SIN_RADIO"
            : yaAvisado
                ? "$ACK,CMD:AMBAR_EMERGENCIA,RESULT:YA_EN_AMBAR_POSTE1_AVISADO"
                : "$ACK,CMD:AMBAR_EMERGENCIA,RESULT:YA_EN_AMBAR_LATCH_PUESTO");
      } else {
        enviarTramaConCrc(sinRadio
            ? "$ACK,CMD:AMBAR_EMERGENCIA,RESULT:OK_SIN_RADIO"
            : yaAvisado ? "$ACK,CMD:AMBAR_EMERGENCIA,RESULT:OK_POSTE1_AVISADO"
                        : "$ACK,CMD:AMBAR_EMERGENCIA,RESULT:OK");
      }
      bluetooth_reportarEvento("APP_BLUETOOTH", "AMBAR_EMERGENCIA_LOCAL");
    } else if (salidaDegradadoIniciada()) {
      ambarEmergencia = true;
      enviarTramaConCrc("$ACK,CMD:AMBAR_EMERGENCIA,RESULT:SALIENDO_TODO_ROJO");
      bluetooth_reportarEvento("APP_BLUETOOTH", "AMBAR_EMERGENCIA_LOCAL");
    } else if (degradado_rendicionEnCurso()) {
      ambarEmergencia = true;
      enviarTramaConCrc("$ACK,CMD:AMBAR_EMERGENCIA,RESULT:SALIDA_YA_EN_CURSO");
      bluetooth_reportarEvento("APP_BLUETOOTH", "AMBAR_EMERGENCIA_LOCAL");
    } else {
      enviarTramaConCrc("$ERR,CMD:AMBAR_EMERGENCIA,DESC:SALIDA_A_ROJO_EN_CURSO_REPITA");
      bluetooth_reportarEvento("APP_BLUETOOTH", "AMBAR_EMERGENCIA_RECHAZADO");
    }
  } else if (strcmp(accion, "CANCELAR_AMBAR") == 0) {
    // R-3 (31/08) - EL AMBAR DE EMERGENCIA SE REVOCA DESDE LA APP, CON PIN.
    //
    // Hasta hoy la unica salida era subir al gabinete y hacer A.A.A en el mando, y ademas
    // el latch se caia solo -ver la cabecera de ambarEmergencia-. Con la app como
    // superficie de mando, el tecnico que pidio el ambar por telefono no podia deshacer su
    // propia orden sin escalera.
    //
    // PIDE PIN, y aqui la asimetria de mando.cpp se lee al derecho: pedir el ambar es la
    // accion SEGURA y no pide clave; QUITARLO devuelve el cruce a dar verdes, o sea que
    // ABRE PASO, y eso es justo lo que el PIN existe para custodiar.
    //
    // Y no se toca la luz: retirar el latch no ordena nada, solo levanta el veto. Quien
    // vuelve a mover la luz es el Maestro con su siguiente orden. Por eso el RESULT dice
    // RETIRADO y no OK -un OK se leeria como "el ambar ya no esta", y sigue estando hasta
    // que llegue esa orden-.
    // N-152 (05/09): Y SE LE DICE AL MAESTRO, QUE ES LA MITAD QUE FALTABA.
    //
    // La frase de arriba -"quien vuelve a mover la luz es el Maestro con su siguiente
    // orden"- era cierta cuando se escribio y N-142 la dejo falsa el 04/09: desde
    // entonces el Maestro se va a MODO_AMBAR al recibir el aviso del armado, y en ese
    // modo CALLA a proposito (SFTY-21). O sea que no hay siguiente orden. El operario
    // del Poste 2 cancelaba, veia su ACK, y el cruce seguia en ambar hasta que alguien
    // caminara hasta el Poste 1. El porque completo esta en protocolo.h, en la cabecera
    // de CMD_CANCELA_AMBAR_ESCLAVO; aqui vive el cuando.
    //
    // Se manda SIN esperar acuse y sin reintento, igual que el aviso del armado: el
    // operario esta esperando delante y lo que confirma es que el cruce se mueva.
    if (!ambarEmergencia) {
      // N-152: LA RED DE LA TRAMA PERDIDA, Y ADEMAS LA SALIDA DE UN CALLEJON.
      //
      // Si el aviso de arriba se pierde, el Maestro sigue callado en su ambar y NO hay
      // red que lo saque: la de N-142 -agotar reintentos y caer a C_FALLO- no vale aqui
      // porque un Maestro que no pregunta nada no puede agotar nada. Lo unico que queda
      // de este lado es que el operario vuelva a pulsar, y eso hasta hoy contestaba
      // "NO_HAY_AMBAR_VIGENTE" delante de un semaforo que estaba en ambar: el latch ya
      // se habia quitado en la primera pulsacion, asi que la segunda no tenia salida.
      //
      // La condicion no es "no hay latch": es que ESTA PUNTA SIGA EN AMBAR. Si la luz ya
      // no esta en ambar no hay nada que pedir.
      //
      // D-30 (14/09): sobraba ademas "y que no lo sostenga el mando" -!mando_ambarLocal()-.
      // Ese latch ya no tiene armador, asi que el termino solo sabia dar una respuesta y la
      // condicion queda en lo que de verdad decide: que la luz siga en ambar.
      //
      // NO PROMETE que el ambar se vaya: el Maestro puede estar en un ambar que pidio
      // otra persona -y entonces ignora el aviso a proposito-. Dice lo que hizo, que es
      // volver a pedirlo. Es el reparto de siempre: esta punta pide.
      if (semaforo_estado() == S_FALLO) {
        protocolo_enviarPaquete(CMD_CANCELA_AMBAR_ESCLAVO);
        enviarTramaConCrc("$ACK,CMD:CANCELAR_AMBAR,RESULT:REENVIADO_AL_MAESTRO");
        bluetooth_reportarEvento("APP_BLUETOOTH", "CANCELA_AMBAR_REENVIADA");
      } else {
        // No se finge una revocacion que no ocurrio: si el operario no sabe que no habia
        // nada que quitar, se ira creyendo que desactivo algo que sigue puesto por otra via.
        enviarTramaConCrc("$ERR,CMD:CANCELAR_AMBAR,DESC:NO_HAY_AMBAR_VIGENTE");
      }
    } else {
      // D-31 (2), 12/09: EL CANCELAR ES LO QUE BORRA LA MEMORIA DEL ACUSE, Y VA EN LA
      // MISMA SENTENCIA QUE EL LATCH PARA QUE NADIE PUEDA SEPARARLOS.
      //
      // Armado y cancelado son la MISMA maquina. Sin este borrado, el proximo
      // AMBAR_EMERGENCIA de este poste contestaria "el Poste 1 ya lo sabe" apoyandose en
      // un acuse de un ambar que ya no existe, y NO ESPERARIA confirmacion: la memoria
      // pasaria a mentir justo cuando hace falta que diga la verdad. Es el estado que
      // partir la maquina deja, y por eso el cancelar entro en el mismo lote.
      //
      // Se borra tambien cuando queda el latch del mando y NO sale ningun aviso al
      // Maestro: el aviso vigente se acabo igual, y el siguiente arranca su plazo desde
      // cero. Un borrado de mas cuesta una espera de 3,5 s que nadie nota; uno de menos
      // cuesta la ceguera entera.
      //
      // Y SE PARA EL PLAZO QUE ESTUVIERA CORRIENDO, QUE ES LO QUE MAS IMPORTA DE ESTAS
      // TRES LINEAS: si no, el reintento del aviso saldria DESPUES de esta cancelacion y
      // volveria a meter al Maestro en MODO_AMBAR por un ambar que acaba de retirarse.
      // El desmentido se pierde con el, y esta bien que se pierda: quien pidio el ambar
      // ya no lo quiere, y una alarma sobre algo cancelado manda a mirar donde no hay
      // nada.
      ambarEmergencia = false;
      avisoAmbarConfirmado = false;
      avisoAmbarEsperando = false;
      // D-8: los dos vetos del ambar de emergencia son INDEPENDIENTES, y aqui se ve por
      // que eso no es redundancia. Quitar el de la app no quita el del gabinete: quien lo
      // puso fue otra persona, por otra via, y una orden no puede revocar la de alguien a
      // quien no ha visto. Por eso este comando dice lo que hizo en vez de prometer que
      // el ambar se fue.
      //
      // D-30 (14/09): AQUI HABIA DOS ACUSES Y AHORA HAY UNO, Y ESTO SE LE DICE A LA APP.
      //
      // Habia una rama para "quedaba tambien el latch del mando" que contestaba
      // RESULT:RETIRADO_QUEDA_MANDO y NO avisaba al Maestro. Solo se tomaba con
      // mando_ambarLocal() puesto, y esa bandera se quedo sin armador cuando el firmware
      // dejo de leer J16 p5/p8: era una rama que ya no alcanzaba nadie. Se retira con su
      // acuse, asi que ese RESULT no vuelve a salir por el puerto -no es un cambio de
      // conducta, es la retirada de una respuesta que ya no podia emitirse-.
      //
      // Queda el unico caso real: se retira el cerrojo de la app, se avisa al Maestro y se
      // contesta lo que se hizo. D-8 no se toca: los dos vetos eran independientes y el
      // que desaparece es el que ya no tenia quien lo pusiera.
      protocolo_enviarPaquete(CMD_CANCELA_AMBAR_ESCLAVO);
      enviarTramaConCrc("$ACK,CMD:CANCELAR_AMBAR,RESULT:RETIRADO");
      bluetooth_reportarEvento("APP_BLUETOOTH", "AMBAR_EMERGENCIA_REVOCADO");
    }
  } else if (strcmp(accion, "FORZAR_ROJO") == 0) {
    // N-83: la misma puerta que arriba, con el PIN puesto (ver alli el porque de rechazar en vez de aliasar). Una app
    // vieja lo manda por las dos y las dos deben contestar lo mismo: sin esta rama la forma con PIN caeria al
    // DESCONOCIDO generico, el mismo error contestado de dos maneras segun por donde entre.
    enviarTramaConCrc("$ERR,CMD:FORZAR_ROJO,DESC:RENOMBRADO_USE_AMBAR_EMERGENCIA");
    bluetooth_reportarEvento("APP_BLUETOOTH", "FORZAR_ROJO_RENOMBRADO");
  } else if (strncmp(accion, "SET_MODO:DEG_T:", 15) == 0) {
    // D-46: SOLICITAR_PASO y SET_MODO:DEGRADADO salieron de esta punta (caen en COMANDO_NO_SOPORTADO); al Degradado
    // se entra por el testigo (D-35): la MISMA orden que en el Maestro, con su tabla (RechazoTestigo). Aqui solo se
    // traduce texto (molde SET_TIEMPOS, con el %c que delata lo que sobra; el verde, D-59, lo juzga la puerta) y la
    // respuesta sale de lo que devolvio degradado_entrarTestigo(), un $ERR por motivo.
    int ah = -1, am = -1, as = -1, ih = -1, im = -1, is = -1, v = 0, d = 0;
    char sobra = 0;
    const int n = sscanf(accion + 15, "%d:%d:%d,%d:%d:%d,%d,%d%c",
                         &ah, &am, &as, &ih, &im, &is, &v, &d, &sobra);
    if (n != 8 || !horaDelDiaValida(ah, am, as) || !horaDelDiaValida(ih, im, is)) {
      enviarTramaConCrc("$ERR,CMD:SET_MODO:DEG_T,DESC:FORMATO_INVALIDO");
    } else if (modo_degradado_salidaS() != 0) {   // D-52: ni renueva ni entra en silencio sobre una salida fijada
      enviarTramaConCrc("$ERR,CMD:SET_MODO:DEG_T,DESC:Cancele antes la salida programada");
    } else {
      const RechazoTestigo r = degradado_entrarTestigo(
          (uint32_t)ah * 3600UL + (uint32_t)am * 60UL + (uint32_t)as,
          (uint32_t)ih * 3600UL + (uint32_t)im * 60UL + (uint32_t)is, v, d);
      if (r == DEG_T_ACEPTADO) {
        enviarTramaConCrc("$ACK,CMD:SET_MODO:DEG_T,RESULT:OK");
        bluetooth_reportarEvento("APP_BLUETOOTH", "SET_MODO_DEG_T");
      } else if (r == DEG_T_RENOVADO) {
        enviarTramaConCrc("$ACK,CMD:SET_MODO:DEG_T,RESULT:RENOVADO");
        bluetooth_reportarEvento("APP_BLUETOOTH", "SET_MODO_DEG_T_RENOVADO");
      } else {
        char p[96];   // 29 del prefijo + 50 del motivo mas largo + NUL
        snprintf(p, sizeof(p), "$ERR,CMD:SET_MODO:DEG_T,DESC:%s", degradado_textoRechazoTestigo(r));
        enviarTramaConCrc(p);
        bluetooth_reportarEvento("APP_BLUETOOTH", "SET_MODO_DEG_T_RECHAZADO");
      }
    }
  } else if (strncmp(accion, "SET_MODO:DEG_FIN:", 17) == 0) {
    // D-52 (SPEC_2 7.quater (c)): gemela de la del Maestro, con el mismo MotivoSalida y los mismos textos. Solo se
    // traduce texto; el $ACK/$ERR sale de lo que devolvio la llamada, y solo hay $ACK si quedo programada (o
    // cancelada).
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
      char p[112];   // 31 del prefijo + 52 del motivo mas largo + NUL
      if (m <= MDF_CANCELADA) {
        snprintf(p, sizeof(p), "$ACK,CMD:SET_MODO:DEG_FIN,RESULT:%s", modo_degradado_textoSalida(m));
        enviarTramaConCrc(p);
        char det[40];
        if (cancelar) { snprintf(det, sizeof(det), "SALIDA_CANCELADA");
        } else { snprintf(det, sizeof(det), "SALIDA_PROGRAMADA_%02d:%02d:%02d", sh, sm, ss); }
        bluetooth_reportarEvento("DEGRADADO", det);
      } else {
        snprintf(p, sizeof(p), "$ERR,CMD:SET_MODO:DEG_FIN,DESC:%s", modo_degradado_textoSalida(m));
        enviarTramaConCrc(p);
      }
    }
  } else if (strncmp(accion, "SET_DEG_AUTO:", 13) == 0) {
    // A-15 (SPEC_2 7.ter (a)): gemela de la del Maestro. Molde SET_TIEMPOS; aceptada, el
    // $ACK sale de bluetooth_loop() cuando el ECO del Maestro refleja el cambio.
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
  } else if (strcmp(accion, "TEST_LEDS") == 0) {
    // RECHAZADO A PROPOSITO, y no es una limitacion pendiente de quitar.
    //
    // D-44: el test de esta punta salio del firmware. Encendia 6 s de secuencia -rojo,
    // ambar y VERDE- sin mirar nada. Lanzado sobre un Esclavo en servicio, ese verde saldria mientras el
    // Maestro esta dando paso al otro sentido: dos vehiculos entrando de frente al
    // tramo. Y da igual que el tecnico se haya conectado al Esclavo correcto: el
    // peligro no es equivocarse de poste, es que esta punta acepte mover luces.
    //
    // Vuelve cuando exista un estado FUERA DE SERVICIO que el propio equipo conozca,
    // no una promesa del manual.
    enviarTramaConCrc("$ERR,CMD:TEST_LEDS,DESC:NO_EN_SERVICIO_USE_EL_MAESTRO");
    bluetooth_reportarEvento("APP_BLUETOOTH", "TEST_LEDS_RECHAZADO");
  } else if (strcmp(accion, "VERSION") == 0) {
    // 1.51 - EL EQUIPO DICE QUE FIRMWARE LLEVA. Ver version_fw.h para el sello.
    //
    // POR QUE NO VA EN EL $STATUS. MEDIDO: el payload de ese $STATUS es de 155 B y esta en
    // el mismo techo que el del Maestro -son los dos el borde del cable-. Meter el sello
    // ahi le come el margen a los campos que cambian cada dos segundos para publicar uno
    // que NO CAMBIA NUNCA mientras el equipo esta encendido, 43.200 veces al dia. Va en su
    // propia trama, que sale cuando alguien pregunta.
    //
    // LLEVA NODE:, Y ES EL PRIMER $ACK DE ESTA PUNTA QUE LO LLEVA, porque esta trama esta
    // hecha para VIAJAR SOLA -pegada en un acta o en un mensaje- y sin el nodo dentro no
    // se puede decir de que poste sale. Y en ESTE cruce vale el doble: las dos puntas
    // tienen que llevar la MISMA version (protocolo.h, CMD_HORA_D: un Maestro que no
    // emita el dia deja al Esclavo sin poder aplicar ninguna hora), y sin el nodo los dos
    // sellos del mismo poste no se distinguen de dos lecturas del mismo equipo.
    //
    // LAS DOS RESPUESTAS LLEVAN SU LITERAL DENTRO DE SU RAMA (N-89), y la del #else es un
    // $ERR y no un $ACK porque CLAUDE.md 2 no admite un OK que no depende de lo que se
    // pudo componer: un binario sin sellar NO SABE que firmware lleva, y decirlo con
    // RESULT:OK seria la mentira con formato de exito.
    //
    // NO HAY snprintf NI BUFFER -la trama es un literal concatenado por el preprocesador-,
    // asi que el truncado que vigila vigilante_parteDeArranque() aqui no puede existir. La
    // cota contra el tramaCompleta[160] de enviarTramaConCrc() la recalcula
    // version_01_sello_de_firmware en cada corrida; no se pone un static_assert porque
    // nombrar ese 160 ABORTA esp32_07 y esp32_09, que leen la declaracion por TEXTO.
    // D-168: detras del sello, la polaridad de la barrera de ESTE binario (pines.h, fijada al compilar).
#if FW_SELLADO
    enviarTramaConCrc("$ACK,CMD:VERSION,RESULT:OK,NODE:ESCLAVO,FW:" FW_TEXTO ",J15:" TALANQUERA_J15);
#else
    enviarTramaConCrc("$ERR,CMD:VERSION,DESC:SIN_SELLAR,NODE:ESCLAVO,FW:" FW_SIN_SELLO ",J15:" TALANQUERA_J15);
#endif
    // NO se anota en el Diario: preguntar la version no cambia el equipo, y una linea por
    // consulta inunda la bitacora donde hay que encontrar el fallo (N-73).
  } else if (const char* r = interruptores_orden(accion)) {   // D-94: local, sin radio
    enviarTramaConCrc(r);
  } else if (const char* r = instalacion_orden(accion)) {     // N-187: local, sin radio
    enviarTramaConCrc(r);
  } else {
    enviarTramaConCrc("$ERR,CMD:DESCONOCIDO,DESC:COMANDO_NO_SOPORTADO_EN_ESCLAVO");
  }
