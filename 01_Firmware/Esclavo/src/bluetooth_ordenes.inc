// ===== src/bluetooth_ordenes.inc (ESCLAVO) =====
// procesarComando(), EL DESPACHADOR: las lineas que entran sin PIN, la guarda de PIN y, en bluetooth_acciones.inc,
// la cadena de ordenes que la pasan. Es UNA funcion en dos ficheros: el corte no la reparte en dos contratos.
//
// NO SE COMPILA SOLO: lo incluye bluetooth.cpp en su sitio, dentro de su unidad de compilacion. Salio de
// bluetooth.cpp por el tope de 500 lineas (roadmap, "Partir"), literal; los instrumentos lo leen dentro de
// bluetooth.cpp, en su sitio (banco/fuente.py, expandir_inc; CLAUDE.md 5).
// D-16: SIN TELEFONO NO HAY FORMA DE OPERAR ESTE POSTE, y este despachador es la razon:
// es la UNICA superficie de mando que le queda. Es una propiedad declarada del sistema,
// no una averia, y aqui pesa mas que en el otro extremo -alli al menos queda la pantalla
// del gabinete-. Consecuencia directa de D-1 (el mando se fue) y de D-17.bis (la pantalla
// y su menu se retiran del equipo): ni ambar, ni volver a obedecer, ni parar el cruce sin
// un telefono emparejado.
//
// LO QUE ESO OBLIGA, Y NO ES UNA NOTA DE ESTILO: cada rama de aqui abajo tiene que
// contestar lo que de verdad paso. Cuando esta era una via mas entre varias, un acuse
// optimista lo corregia el operario mirando la pantalla; hoy no hay pantalla que mirar.
// D-35: una hora del dia de SET_MODO:DEG_T, mirada sobre el int y antes de castear.
static bool horaDelDiaValida(int h, int m, int s) {
  return h >= 0 && h <= 23 && m >= 0 && m <= 59 && s >= 0 && s <= 59;
}

static void procesarComando(const char* cmd) {
  // AB-1 - LA LINEA RESERVADA DEL PUENTE, Y VA LA PRIMERA DE TODAS.
  //
  // Antes que el rojo de emergencia, antes que la guarda de PIN, antes que nada. No
  // porque sea mas importante -no lo es-, sino porque tiene que salir de aqui SIN
  // TOCAR NADA y sin contestar, y cualquier rama que la adelante le anadiria un efecto.
  //
  // POR QUE EXISTE. El puente emite esta linea cada LATIDO_MS -contrato.h del ESP32-
  // para que los tres contadores de silencio de J17 de este mismo fichero signifiquen
  // algo. Hasta hoy contaban SILENCIOS DEL PUERTO, no muertes del puente: por J17 solo
  // entraba lo que un dedo pulsa en la app, asi que un puente vivo y uno muerto eran
  // indistinguibles desde aqui.
  //
  // POR QUE NO CONTESTA, QUE ES TODA LA RAZON DE QUE ESTA RAMA EXISTA. Se midio el
  // 04/09: sin ella, el latido caeria en el ultimo else de la guarda de PIN y devolveria
  // $ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO. La app no traduce ese par y lo saca por el
  // ramal generico: un aviso ROJO cada dos segundos acusando al operario de una clave
  // que nadie tecleo. Eso es el FALLA PERMANENTE de CLAUDE.md seccion 2 -un rechazo que
  // nadie puede apagar ensena a ignorar los rechazos de verdad-, y por eso la rama
  // devuelve muda en vez de con un $ACK: un ACK cada dos segundos seria el mismo ruido
  // con otro color.
  //
  // Y NO ROMPE 6.4. La regla dice que el puente no origina ORDENES. Esta linea no
  // ejecuta nada, no mueve una luz, no cambia un modo y no contesta. Lo unico que
  // produce es que j17RegistrarLinea() -mas abajo, en el troceador- cierre un silencio.
  // No se manda: se respira.
  //
  // EL LITERAL EMPIEZA POR '$' A PROPOSITO. Las ordenes son "CMD:..."; lo que empieza
  // por '$' son las tramas que este equipo EMITE. No hay ninguna orden a un byte de
  // distancia de esto, asi que ni un bit cambiado en el cable la convierte en otra cosa.
  if (strcmp(cmd, "$LATIDO") == 0) {
    return;
  }

  // SFTY - EL AMBAR DE EMERGENCIA NO PIDE PIN, Y ES DELIBERADO.
  //
  // N-83: LA RAZON DE ANTES ERA FALSA ("el PIN guarda lo que ABRE paso; no lo que las para": esto no para el trafico,
  // deja el equipo en S_FALLO, ambar a 500 ms con la talanquera ARRIBA, decision del cliente y del PMT).
  // La exencion sigue siendo correcta por esto otro: un ambar intermitente NO LE DA PRIORIDAD A NADIE;
  // no concede el paso a un sentido contra el otro (lo que el PIN custodia) sino que pone a los dos a pasar con
  // precaucion. Es la caida segura universal, la misma a la que el equipo llega solo al perder el enlace (SFTY-6) o
  // al reiniciar el watchdog, y una caida segura que exige recordar una clave delante de un accidente no lo es: quien
  // ve el incidente debe poder pedirla. Se acepta tambien la forma con PIN, mas abajo: la app la envia asi y el
  // manual la documenta.
  //
  // N-106 (31/08) - Y NO SE SALTA EL MODO DEGRADADO (la tabla entera es el Manual 10 S4.5.2; aqui el porque vial).
  // Antes era semaforo_iniciarFallo() a secas: con el Degradado gobernando la luz saltaba de un VERDE POR RELOJ
  // directo a ambar, y mando.cpp ya tenia escrito lo que eso significa: "le daria a quien ya venia lanzado una senal
  // que invita a negociar el paso mientras aun cree tener prioridad". El Degradado entra y sale SIEMPRE por
  // todo-rojo, y el B.B.B del mando sale por ahi. R-1 (31/08): se acepta que el ambar tarde de 10 a 90 s (el
  // todo-rojo de despedida sale de cfgDespeje): es lo unico que protege a quien venia lanzado, y lo que ya cuesta el
  // B.B.B.
  //
  // LAS DOS PUERTAS -esta sin PIN y la de 'accion' con PIN- LLEVAN EL MISMO BLOQUE, letra por letra: un parche a una
  // sola deja media puerta contestando el $ACK viejo (lo vigilan esclavo_07 y esclavo_08).
  //
  // 🔴 N-142 / §3.16-A (11/09) - HASTA HOY ESA FRASE ERA FALSA, MEDIDA EN EL FUENTE:
  // protocolo_enviarPaquete(CMD_AMBAR_ESCLAVO) tenia UN SOLO llamador en todo Esclavo/src y estaba en la puerta CON
  // PIN, mientras la app manda AMBAR_EMERGENCIA por la SIN PIN (esta, D-158): el aviso de N-142 no lo habia disparado
  // nadie desde un telefono, esta punta se iba a S_FALLO y el Maestro seguia su ciclo pudiendo dar VERDE en el Poste
  // 1 hacia un carril cuyo otro extremo ya no controla nadie. Es el candidato mas firme del DAR PASO del Sisga
  // (roadmap §3.16).
  if (strcmp(cmd, "CMD:AMBAR_EMERGENCIA") == 0) {
    if (!degradado_gobiernaLuz()) {
      // Filas A y B: nadie mas gobierna la luz, el ambar se enciende ya. No hay $ERR
      // posible en este camino y no se inventa uno: semaforo_iniciarFallo() no tiene
      // guarda y no puede fallar, y armar el latch es una asignacion.
      const bool yaEnAmbar = (semaforo_estado() == S_FALLO);
      semaforo_iniciarFallo();
      ambarEmergencia = true;

      // N-142: SE LE DICE AL MAESTRO, TAMBIEN POR AQUI. El porque entero -que sin el
      // aviso el Maestro no tiene forma de enterarse, que esta punta sigue contestando
      // PONG en ambar y que por eso el enlace le parece perfecto- esta escrito una sola
      // vez, en la puerta gemela de 'accion'. D-31 (12/09): y desde hoy el PRIMERO se
      // ESPERA -avisoAmbarSalio() arranca el plazo-, igual que alli.
      protocolo_enviarPaquete(CMD_AMBAR_ESCLAVO);
      avisoAmbarSalio();

      // 🔴 Y EL $ACK DICE SI ESA TRAMA PUEDE HABER SIDO OIDA, PORQUE NO ES LO MISMO. CLAUDE.md §2: un $ACK que no
      // depende de lo que la llamada devolvio es una mentira con formato de exito. protocolo_enviarPaquete() es void
      // a proposito (no hay acuse que esperar), asi que se mira lo unico que esta punta sabe de la radio del Maestro
      // ahora: el dato con el que ya se publica el $ALARM FALLO_RF y su $EVENT de regreso (N-108). QUE PROMETE CADA
      // LITERAL:
      //   OK / YA_EN_AMBAR_LATCH_PUESTO  ambar AQUI y el aviso salio con el enlace sin caida declarada; NO promete
      //                                  que la trama llegara (no se acusa), solo que esta punta oye al Maestro.
      //   ..._SIN_RADIO                  ambar AQUI y esta punta ya declaro que se quedo sin radio: el aviso sale
      //                                  igual y casi seguro no llega; quien lo pidio debe saberlo antes de irse.
      //   ..._POSTE1_AVISADO             D-31: el Poste 1 ACUSO por radio un aviso anterior de ESTE ambar; es la
      //                                  unica que promete algo del OTRO extremo (hubo trama de vuelta).
      // NO SE INVENTA UN SEGUNDO RELOJ DE SILENCIO: el que hay lo arma main.cpp con SFTY6_SILENCIO_MS y lo baja el
      // regreso de tramas validas en bluetooth_loop(); dos cuentas del mismo silencio divergirian. D-31: Y EL PRIMER
      // $ACK NO ESPERA AL ACUSE: sale ya, y si el acuse no llega en su plazo el desmentido va DESPUES por $ALARM
      // AVISO_RF (casar un $ERR con una orden contestada hace segundos es lo que N-130 descarto). SIN_RADIO GANA A
      // POSTE1_AVISADO si las dos son ciertas: el acuse es de HACE UN RATO y la caida de AHORA.
      const bool sinRadio = enlaceCaidoAnunciado;
      const bool yaAvisado = avisoAmbarConfirmado && !sinRadio;
      if (yaEnAmbar) {
        // Fila B. Lo que esta orden cambia NO es la luz -ya estaba en ambar por SFTY-6,
        // por el watchdog o por un B.B.B-: es el latch, que convierte un ambar que el
        // siguiente CMD_GO_RED se llevaria en uno vetado. Contestar OK ocultaria que lo
        // unico nuevo es la proteccion.
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
      bluetooth_reportarEvento("APP_BLUETOOTH", "AMBAR_EMERGENCIA_SIN_PIN");
    } else if (salidaDegradadoIniciada()) {
      // Fila C. El RESULT no es OK a proposito: el ambar no esta puesto todavia y decir
      // OK seria dar por hecho un cambio de luz que tarda hasta 90 s. Lo que garantiza
      // que llegue es el sostenedor de bluetooth_loop() -la segunda mitad del molde-.
      ambarEmergencia = true;
      enviarTramaConCrc("$ACK,CMD:AMBAR_EMERGENCIA,RESULT:SALIENDO_TODO_ROJO");
      bluetooth_reportarEvento("APP_BLUETOOTH", "AMBAR_EMERGENCIA_SIN_PIN");
    } else if (degradado_rendicionEnCurso()) {
      // Fila D con la salida en curso terminando en AMBAR: es la rendicion de 48 h, que
      // acaba en DEG_RENDIDO encendiendo el ambar por su cuenta (D-38: la hora va a rojo
      // fijo, y desde alli esta orden cae en la fila C). La orden no arranca nada pero el latch
      // SI cambia algo: sin el, ese ambar duraria hasta la siguiente orden del Maestro. R-2.
      ambarEmergencia = true;
      enviarTramaConCrc("$ACK,CMD:AMBAR_EMERGENCIA,RESULT:SALIDA_YA_EN_CURSO");
      bluetooth_reportarEvento("APP_BLUETOOTH", "AMBAR_EMERGENCIA_SIN_PIN");
    } else {
      // Fila D con la salida en curso terminando en ROJO -la pidio otro: el A.A.A del
      // mando, la pantalla o el regreso del radio-. NO se arma el latch y NO se contesta
      // OK, y las dos cosas van juntas: armarlo seria quedarse con el final de una salida
      // que mando otro, y contestar OK seria prometer un ambar que nadie va a encender.
      //
      // La espera no cuesta seguridad: mientras dura esa salida el equipo esta en TODO
      // ROJO, que es mas seguro que el ambar que se pide. Al terminar, el mismo comando
      // cae en la fila A y enciende el ambar de inmediato; por eso el motivo dice REPITA.
      enviarTramaConCrc("$ERR,CMD:AMBAR_EMERGENCIA,DESC:SALIDA_A_ROJO_EN_CURSO_REPITA");
      bluetooth_reportarEvento("APP_BLUETOOTH", "AMBAR_EMERGENCIA_RECHAZADO");
    }
    return;
  }

  // N-83: EL NOMBRE VIEJO SE RECHAZA ENSENANDO EL BUENO, NO EN SILENCIO.
  //
  // "FORZAR_ROJO" prometia rojo y hacia ambar con la pluma arriba, que es casi lo
  // contrario. El comportamiento es el correcto y no se toca; lo que se corrige es que
  // el equipo declare otra cosa de la que hace.
  //
  // Dejarlo como alias mudo conservaria la mentira: quien lo mande seguira creyendo
  // que detuvo el cruce. Y dejarlo caer al $ERR,CMD:DESCONOCIDO del final tampoco
  // sirve: quien manda esto es alguien con una app o un manual anteriores al cambio, y
  // lo que necesita no es enterarse de que el comando no existe, sino de como se llama
  // ahora. Es el mismo patron con el que esta punta rechaza TEST_LEDS: se dice el
  // motivo.
  //
  // La forma con PIN se rechaza en la cadena de 'accion', mas abajo, para no repetir
  // aqui el literal del PIN.
  //
  // OJO AL MAESTRO: alli CMD:FORZAR_ROJO se queda, y alli SI hace rojo de verdad. Que
  // las dos puntas usen literales distintos es lo correcto, porque hacen cosas
  // distintas; llamarlas igual era el defecto.
  if (strcmp(cmd, "CMD:FORZAR_ROJO") == 0) {
    enviarTramaConCrc("$ERR,CMD:FORZAR_ROJO,DESC:RENOMBRADO_USE_AMBAR_EMERGENCIA");
    bluetooth_reportarEvento("APP_BLUETOOTH", "FORZAR_ROJO_RENOMBRADO");
    return;
  }

  // D-20 / D-26 (11/09) - LA HORA DEL ESP32 DE ESTE POSTE. SIN PIN, Y AQUI SOLO SI LA
  // RADIO NO MANDA.
  //
  // QUIEN LA MANDA: el ESP32 de este poste, desde su DS3231 releido, al arrancar, tras
  // cada puesta en hora y cada ~5 min (D-26 (2); ~~cada hora, A-15~~). Es la UNA
  // excepcion a "el puente no origina" (esp32_05), y el puente TIRA esta misma linea si le
  // llega del telefono: por eso va antes de la guarda de PIN, como el ambar de emergencia
  // -con PIN caeria en AUTH_FAILED en cada siembra-. SUSTITUYE a la antigua rama de
  // SET_RTC, que desde el 11/09 atiende SOLO el puente: una orden en las dos listas son dos
  // acuses a una sola orden (D-15). Sin PIN en el ESP32: riesgo ACEPTADO, D-26 (1).
  //
  // POR QUE AQUI NO SE ACEPTA SIEMPRE, Y EN EL MAESTRO SI. D-26 (3): "CON radio, manda la
  // hora del Maestro; SIN radio, acepta la de su propio ESP32 -su DS3231 y la que el
  // usuario le ponga desde el telefono en el gabinete-". reloj_radioManda() es la
  // pregunta, con "sin radio" definido por el MISMO silencio que dispara $ALARM FALLO_RF
  // (SFTY6_SILENCIO_MS): esa alarma es la que manda al usuario a este poste a poner la
  // hora, y esa hora tiene que entrar. Ver el porque de la fuente en reloj.h.
  //
  // ~~Con hora de radio puesta, NI AUNQUE LA RADIO SE CAIGA: cambiar de fuente al perder
  // la radio seria un salto SIN COTA entre dos DS3231 puestos por separado~~ -> SUSTITUIDO
  // el 11/09 por D-26 (3)+(4)+(5). El salto existe y D-26 lo acepta con su cota: lo que
  // difieran los dos DS3231 (+-2 ppm cada uno, ~10 s/mes en el peor caso) mas lo que
  // derivo el HSI de cada punta desde su ultima siembra. Lo que lo hace tolerable no es
  // negarlo: es que un salto mayor que el margen del cruce se aplica PASANDO POR ROJO
  // (modo_degradado.cpp, D-26 (4)) y que la perdida de radio ALARMA (FALLO_RF, main.cpp).
  // La regla vieja dejaba a esta punta sin radio extrapolando con el HSI -hasta 90 s por
  // hora, contra 29 s de margen- durante todo el Degradado: una cota peor que la que
  // queria evitar. (Y cuenta que esa cuenta del peor caso la rehace esp32_13.)
  //
  // La guarda va ANTES que la siembra y la siembra vive en su else: al reves, la hora del
  // ESP32 ya habria pisado la del Maestro cuando alguien preguntara.
  //
  // Y NO RENUEVA NADA DEL DEGRADADO: ni degradado_registrarSync() ni respaldo_marcarSync().
  // El limite de 48 h cuenta desde la ultima hora DEL MAESTRO; una del DS3231 de este poste
  // no demuestra que las dos puntas sigan en fase.
  //
  // NO CONTESTA, ni $ACK ni $ERR: no la origino el telefono. Lo que queda es el Diario, SOLO
  // EN EL CAMBIO de quien manda -la primera IGNORADA, la primera SEMBRADA, tras el arranque
  // o tras una alarma-: cada 5 min serian 288 lineas identicas al dia (N-73 por
  // inundacion). Y cada linea sale de lo que la llamada devolvio (CLAUDE.md 2, N-160). Una
  // siembra que no sirve no es una linea de diario: es la alarma de D-26 (5), aqui en el
  // acto y despues la repite horaEsp32Vigilar() mientras dure.
  if (strncmp(cmd, "CMD:HORA_ESP32:", 15) == 0) {
    if (reloj_radioManda()) {
      reloj_fecharDesdeEsp32(cmd + 15);   // A-15 (H4): la fecha si, la hora no (D-26 (3))
      horaEsp32Rechazada = false;
      horaEsp32Llego = true;
      if (horaEsp32Estado != HE_IGNORADA) {
        horaEsp32Estado = HE_IGNORADA;
        bluetooth_reportarEvento("ESP32", "HORA_ESP32_IGNORADA_MANDA_RADIO");
      }
    } else if (reloj_sembrarDesdeIso(cmd + 15)) {
      // D-35: y el dia que trae, para la edad del testigo (aviso de 28 dias). Sin el, el testigo no entra
      // (Falta: reloj sin poner en hora) y uno en curso sigue con el dia que ya llevaba.
      reloj_guardarFechaEsp32(cmd + 15);
      horaEsp32Rechazada = false;
      horaEsp32Llego = true;
      if (horaEsp32Estado != HE_SEMBRADA) {
        horaEsp32Estado = HE_SEMBRADA;
        bluetooth_reportarEvento("ESP32", "HORA_ESP32_SEMBRADA");
      }
    } else {
      horaEsp32Rechazada = true;
      if (horaEsp32Estado != HE_ALARMA) {
        horaEsp32Estado = HE_ALARMA;
        bluetooth_reportarAlarma("HORA_ESP32", "RECHAZADA_FORMATO", "SIGUE_SU_HORA");
      }
    }
    return;
  }

  // 1.51 - LA VERSION DEL FIRMWARE, SIN PIN. Ver version_fw.h para el sello, y la rama
  // gemela de mas abajo -con el PIN puesto- para el porque de cada decision de la trama,
  // que esta escrito UNA sola vez.
  //
  // POR QUE ENTRA SIN PIN: no toca nada, solo CUENTA lo que este binario es, y el PIN
  // guarda lo que ABRE paso o mueve luces. Y las DOS formas se aceptan porque la version
  // es lo primero que se pregunta cuando algo va mal: un AUTH_FAILED ahi manda a buscar
  // una clave en vez de a mirar el firmware.
  //
  // VA ANTES DE LA GUARDA DE PIN Y DESPUES DEL AMBAR DE EMERGENCIA: el orden de este
  // fichero es "lo que para la via primero"; una consulta no para nada y no adelanta a
  // nada.
  if (strcmp(cmd, "CMD:VERSION") == 0) {
#if FW_SELLADO
    enviarTramaConCrc("$ACK,CMD:VERSION,RESULT:OK,NODE:ESCLAVO,FW:" FW_TEXTO);
#else
    enviarTramaConCrc("$ERR,CMD:VERSION,DESC:SIN_SELLAR,NODE:ESCLAVO,FW:" FW_SIN_SELLO);
#endif
    return;
  }

  // D-60/D-61/D-62: REINICIOS solo mira; entra con y sin PIN, y es gemela de la del Maestro.
  if (strcmp(cmd, "CMD:REINICIOS") == 0 || strcmp(cmd, "CMD:PIN:1234:REINICIOS") == 0) {
    char r[96];
    const int n = snprintf(r, sizeof(r),
        "$ACK,CMD:REINICIOS,RESULT:OK,NODE:ESCLAVO,REIN:%u,CAUSA:%s,UP:%lu,ANTES:%u",
        (unsigned)respaldo_reinicios(), respaldo_causa(),
        (unsigned long)respaldo_segundosEncendido(), (unsigned)respaldo_horasAnterior());
    if (n > 0 && n < (int)sizeof(r)) enviarTramaConCrc(r);
    else enviarTramaConCrc("$ERR,CMD:REINICIOS,DESC:NO_CABE,NODE:ESCLAVO");
    return;
  }

  // A-15 (SPEC_4 3.2): la consulta del Degradado automatico no toca nada: entra con y sin
  // PIN, por lo mismo que la version.
  // D-52: CONSULTA_DEG_FIN tambien entra con y sin PIN, y solo mira.
  // SALE_HHMMSS_FALTAN_<s>S_<RESPALDADA|SIN_RESPALDO>.
  if (strcmp(cmd, "CMD:CONSULTA_DEG_FIN") == 0 || strcmp(cmd, "CMD:PIN:1234:CONSULTA_DEG_FIN") == 0) {
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
    return;
  }

  if (strcmp(cmd, "CMD:CONSULTA_DEG_AUTO") == 0 || strcmp(cmd, "CMD:PIN:1234:CONSULTA_DEG_AUTO") == 0) {
    char p[64];
    snprintf(p, sizeof(p), "$ACK,CMD:CONSULTA_DEG_AUTO,RESULT:ESTE_%s_OTRO_%s_APTO_%s",
             respaldo_degAuto() ? "ON" : "OFF", respaldo_otroApto() ? "ON" : "OFF",
             degAuto_aptoPropio() ? "SI" : "NO");
    enviarTramaConCrc(p);
    return;
  }

  // N-187: CONSULTA_INSTALACION solo mira; entra con y sin PIN, gemela de la del Maestro.
  if (strcmp(cmd, "CMD:CONSULTA_INSTALACION") == 0 || strcmp(cmd, "CMD:PIN:1234:CONSULTA_INSTALACION") == 0) {
    enviarTramaConCrc(instalacion_orden("CONSULTA_INSTALACION"));
    return;
  }

  // Validación estricta de PIN de 4 dígitos (1234)
  if (strncmp(cmd, "CMD:PIN:1234:", 13) != 0) {
    enviarTramaConCrc("$ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO");
    return;
  }

  const char* accion = cmd + 13;

#include "bluetooth_acciones.inc"   // la cadena de ordenes sobre accion, aqui mismo dentro de procesarComando()
}
