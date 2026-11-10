// ===== src/bluetooth_ordenes.inc (MAESTRO) =====
// procesarComando(), EL DESPACHADOR: las lineas que entran sin PIN, la guarda de PIN y, en bluetooth_acciones.inc,
// la cadena de ordenes que la pasan. Es UNA funcion en dos ficheros: el corte no la reparte en dos contratos.
//
// NO SE COMPILA SOLO: lo incluye bluetooth.cpp en su sitio, dentro de su unidad de compilacion. Salio de
// bluetooth.cpp por el tope de 500 lineas (roadmap, "Partir"), literal; los instrumentos lo leen dentro de
// bluetooth.cpp, en su sitio (banco/fuente.py, expandir_inc; CLAUDE.md 5).
// D-35: una hora del dia de SET_MODO:DEG_T, mirada sobre el int y antes de castear.
static bool horaDelDiaValida(int h, int m, int s) {
  return h >= 0 && h <= 23 && m >= 0 && m <= 59 && s >= 0 && s <= 59;
}

static void procesarComando(const char* cmd) {
  // AB-1 - LA LINEA RESERVADA DEL PUENTE, Y VA LA PRIMERA DE TODAS: antes que el rojo de emergencia y la guarda de
  // PIN, porque tiene que salir SIN TOCAR NADA y sin contestar; cualquier rama que la adelante le anadiria un efecto.
  // El puente la emite cada LATIDO_MS (contrato.h del ESP32) para que los tres contadores de silencio de J17
  // signifiquen algo: hasta hoy contaban silencios del PUERTO, no muertes del puente. NO CONTESTA, y es toda la razon
  // de la rama (medido el 04/09): sin ella caeria en la guarda de PIN y devolveria
  // $ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO, que la app saca como un aviso ROJO cada dos segundos acusando al operario
  // de una clave que nadie tecleo: el FALLA PERMANENTE de CLAUDE.md 2. Un ACK cada dos segundos seria el mismo ruido
  // con otro color. NO ROMPE 6.4: no ejecuta nada, no mueve una luz ni un modo; solo cierra un silencio en
  // j17RegistrarLinea(). El literal empieza por '$' a proposito: las ordenes son "CMD:..." y lo que empieza por '$'
  // lo EMITE este equipo, asi que ni un bit cambiado la convierte en otra.
  if (strcmp(cmd, "$LATIDO") == 0) {
    return;
  }
  // D-110/D-111: las ordenes de noche y, en la noche, las salidas; antes del rojo sin PIN (D-158), que
  // en la noche saca del modo. Lo que no contesta aqui sigue por el despachador de siempre.
  if (modoNoche_orden(cmd, enviarTramaConCrc)) return;

  // SFTY - EL ROJO DE EMERGENCIA NO PIDE PIN, Y ES DELIBERADO: "lo seguro, facil; lo peligroso, dificil". Detener el
  // trafico es la accion SEGURA y una clave solo retrasa a quien ve el incidente; el PIN guarda lo que ABRE paso. La
  // forma con PIN, mas abajo, hace lo mismo: la app la envia asi y el manual la documenta.
  if (strcmp(cmd, "CMD:FORZAR_ROJO") == 0) {
    const ResultadoRojoTotal r = modo_degradado_forzarRojo();   // D-51: en Degradado, rojo fijo en ESTE poste
    if (r == RRT_ROJO_FIJO) { enviarTramaConCrc("$ACK,CMD:FORZAR_ROJO,RESULT:ROJO_FIJO_EN_ESTE_POSTE");
    } else if (r == RRT_YA_EN_ROJO_FIJO) { enviarTramaConCrc("$ACK,CMD:FORZAR_ROJO,RESULT:YA_EN_ROJO_FIJO");
    } else if (r == RRT_SALIDA_EN_CURSO) { enviarTramaConCrc("$ACK,CMD:FORZAR_ROJO,RESULT:SALIDA_YA_EN_CURSO");
    } else if (r == RRT_REANUDACION_CANCELADA) {
      enviarTramaConCrc("$ACK,CMD:FORZAR_ROJO,RESULT:OK_REANUDACION_CANCELADA");
    } else { enviarTramaConCrc("$ACK,CMD:FORZAR_ROJO,RESULT:OK"); }
    bluetooth_reportarEvento("APP_BLUETOOTH", "FORZAR_ROJO_SIN_PIN");
    return;
  }

  // D-20 / D-26 (11/09) - LA HORA DEL ESP32 DE ESTE POSTE. SIN PIN, Y AQUI SE ACEPTA SIEMPRE.
  //
  // QUIEN LA MANDA: el ESP32 de este poste, desde su DS3231 releido, al arrancar, tras
  // cada puesta en hora y cada ~5 min (D-26 (2); ~~cada hora, A-15~~). Es la UNA
  // excepcion a "el puente no origina" (esp32_05), y el puente TIRA esta misma linea si le
  // llega del telefono: por eso va antes de la guarda de PIN, como el rojo de emergencia
  // -con PIN caeria en AUTH_FAILED en cada siembra-. SUSTITUYE a la antigua rama de
  // SET_RTC, que desde el 11/09 atiende SOLO el puente: el STM32 ya no recibe los bytes
  // del telefono sino la hora que el DS3231 releyo (roadmap 3.16-C). Que esa hora se ponga
  // SIN PIN en el ESP32 es riesgo ACEPTADO por el responsable (D-26 (1)).
  //
  // POR QUE AQUI SE ACEPTA SIEMPRE: D-20, "la autoridad de la hora es el ESP32, siempre y
  // para todo", y en esta punta ese ESP32 es EL SUYO; la radio va de aqui al Esclavo, no
  // al reves. El que filtra es el Esclavo -su rama gemela-: alli manda la radio mientras
  // se oiga (D-26 (3)).
  //
  // Y SOLO SI ENTRO SE PROPAGA: coordinador_sincronizarHora() va DENTRO del if. El Esclavo
  // hace caso SIEMPRE a la radio, asi que empujarle una hora que esta punta descarto seria
  // mandarle una hora que nadie adopto (N-160).
  //
  // D-26 (2) - SE PROPAGA EN CADA SIEMBRA, NO CADA INTERVALO_SYNC_MS (decision medida): con la radio viva el Esclavo
  // no siembra de su ESP32 y su hora corre sobre SU HSI; a 25000 ppm son 90 s por hora de separacion posible contra
  // los 29 s que aguanta el cruce. Propagando en cada siembra quedan 7,5 s por punta (300 s x 25000 ppm) por ~0,3 %
  // del canal (7 tramas a ~0,13 s, SFTY-11), sin competir con las luces: el coordinador lo arranca con el bus libre.
  //
  // NO CONTESTA, ni $ACK ni $ERR: no la origino el telefono. Lo que queda es el Diario, y
  // SOLO EN EL CAMBIO: la primera siembra buena tras el arranque o tras una alarma. Una
  // linea cada 5 min serian 288 al dia identicas en la bitacora donde hay que encontrar el
  // fallo (N-73 por inundacion, la misma razon del umbral de J17). Si la siembra NO sirve,
  // no es una linea de diario: es la alarma de D-26 (5), en el acto y una vez, y luego la
  // repite horaEsp32Vigilar() mientras dure.
  //
  // CON EL DEGRADADO GOBERNANDO la siembra mueve de golpe reloj_segundosDelDia(), que es de
  // donde sale la fase. Un salto mayor que el margen del cruce lo pasa por rojo
  // modo_degradado.cpp (D-26 (4)), que es quien gobierna la luz: esta rama no decide luces.
  //
  // H6-i (11/09) - EL DIARIO MIRA LO QUE DEVOLVIO LA PROPAGACION (molde SET_TIEMPOS,
  // CLAUDE.md 2). Aqui se tiraba el bool de coordinador_sincronizarHora() y el diario decia
  // SEMBRADA pasara lo que pasara. MEDIDO, Y SE DICE PARA QUE NADIE LO LEA COMO UN ARREGLO
  // DE UN DEFECTO VIVO: hoy ese false NO PUEDE SALIR en esta rama -su unica guarda es
  // reloj_enHora(), la misma horaValida que la siembra acaba de poner en true-. Lo que se
  // cierra es la forma: el dia que la propagacion gane otra guarda, el diario dira la verdad
  // en vez de afirmar un envio que no se encolo. Es de diario y no de alarma, y sin cerrojo
  // de cambio: si alguna vez sale, sale en cada siembra, y eso tambien es un dato.
  if (strncmp(cmd, "CMD:HORA_ESP32:", 15) == 0) {
    if (reloj_sembrarDesdeIso(cmd + 15)) {
      // D-35: y el dia que trae, para la edad del testigo (aviso de 28 dias). Sin el, el testigo no entra
      // (Falta: reloj sin poner en hora) y uno en curso sigue con el dia que ya llevaba.
      reloj_guardarFechaEsp32(cmd + 15);
      horaEsp32Rechazada = false;
      horaEsp32Llego = true;
      if (!coordinador_sincronizarHora()) {
        bluetooth_reportarEvento("ESP32", "HORA_ESP32_SEMBRADA_SIN_PROPAGAR");
      } else if (horaEsp32Estado != HE_SEMBRADA) {
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

  // Validación estricta de PIN de 4 dígitos (1234)
  //
  // OTRAS ORDENES ENTRAN SIN PIN, POR EL MISMO CRITERIO DE ARRIBA: MENU no abre paso (D-167: ALCANCE salio) y VERSION y
  // las consultas no tocan nada: solo CUENTAN lo que este binario es o lo que tiene guardado. El PIN guarda lo que ABRE,
  // no lo que para ni lo que mira. Se aceptan tambien con PIN, igual que FORZAR_ROJO: la app antepone la clave a todo
  // lo que no este en su lista SIN_PIN, y una orden que solo se aceptara sin PIN seria inalcanzable desde el celular.
  // Lo que se mueve es DONDE EMPIEZA LA ACCION, no la cadena de comparaciones: una segunda cadena serian dos
  // contratos que sincronizar, y el dia que uno se quede atras el comando contesta DESCONOCIDO.
  const char* accion;
  if (strncmp(cmd, "CMD:PIN:1234:", 13) == 0) {
    accion = cmd + 13;
  } else if (strncmp(cmd, "CMD:", 4) == 0 &&
             (strcmp(cmd + 4, "SET_MODO:MENU") == 0 ||
              strcmp(cmd + 4, "CONSULTA_DEG_AUTO") == 0 ||   // A-15: solo mira
              strcmp(cmd + 4, "CONSULTA_DEG_FIN") == 0 ||    // D-52: solo mira
              strcmp(cmd + 4, "REINICIOS") == 0 ||           // D-60: solo mira
              strcmp(cmd + 4, "CONSULTA_INSTALACION") == 0 ||   // N-187: solo mira
              strcmp(cmd + 4, "CONSULTA_VETO") == 0 ||          // D-166: solo mira
              strcmp(cmd + 4, "VERSION") == 0)) {
    accion = cmd + 4;
  } else {
    enviarTramaConCrc("$ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO");
    return;
  }

#include "bluetooth_acciones.inc"   // la cadena de ordenes sobre accion, aqui mismo dentro de procesarComando()
}
