// ===== js/nmea_parser.js =====
// Parser y Formateador de Tramas NMEA de Telemetría Semafórica

// Vocabulario de ausencia, el MISMO que RF_NO_MEDIDO de app.js: las tres cifras de la
// trama se marcan igual, asi que tienen que leerse igual. Si esta lista y la de app.js
// dejan de coincidir, una punta declarara la ausencia y la otra la pintara como dato.
const _SIN_DATO = ['', '-', '--', '?', 'NA', 'N/A', 'NULL', 'NO_MEDIDO', 'SIN_DATO'];

function _numeroOMarca(v, conv, base) {
  const crudo = String(v).trim();
  if (_SIN_DATO.indexOf(crudo.toUpperCase()) >= 0) return crudo;
  const n = base === undefined ? conv(crudo) : conv(crudo, base);
  // Un campo que no sea ni numero ni marca conocida tampoco se convierte en 0: se
  // devuelve entero, para que quien pinte vea que no lo entiende en vez de heredar
  // una cifra que nadie midio.
  return Number.isNaN(n) ? crudo : n;
}

const NMEAParser = {
  /**
   * EL UNICO SITIO QUE PARTE UNA TRAMA EN CAMPOS. Devuelve las claves TAL CUAL VIAJAN.
   *
   * Hasta el 05/09 esta operacion -partir por ',' y luego por el PRIMER ':'- estaba
   * escrita TRES veces, en tres ficheros, con tres contratos distintos:
   *
   *   app.js  _camposNmea()          claves verbatim. ES LA QUE CORRE EN EL TELEFONO,
   *                                  y sirve a las CINCO cabeceras que la app lee.
   *   este fichero  parseStatus()    su propio bucle, y detras un switch que RENOMBRA
   *                                  a minuscula. Solo $STATUS.
   *   test_unitarios_app.js          una tercera copia, con `pair.length` en vez del
   *                                  primer ':', y solo la ejercen sus propias pruebas.
   *
   * Tres copias de una regla que ya se equivoco una vez: N-62 -HORA:18:25:00 truncado a
   * "18" por un split(':') sin limite- hubo que arreglarlo en cada copia por separado, y
   * la tercera sigue sin arreglar hoy (usa pair.length, que casualmente acierta). Esto es
   * exactamente la "segunda copia del firmware escrita a mano que alguien sincroniza" de
   * CLAUDE.md 3.bis, aplicada al parser en vez de al modelo.
   *
   * EL CONVENIO QUE GANA ES EL DEL CABLE, y el motivo no es de gusto:
   *
   *   1. Es el que corre en el telefono. Cambiar la app al otro convenio dejaria CIEGO
   *      al censo de documentos_03_trama_status, que cuenta los `data.X` en mayusculas
   *      de la rama de $STATUS: encontraria CERO campos y aprobaria vacuamente. Un
   *      instrumento que deja de medir sin ponerse rojo es N-89 otra vez.
   *   2. No lleva lista. El switch de parseStatus() enumera los campos, asi que es una
   *      copia mas del contrato que alguien tiene que sincronizar; esto no puede
   *      quedarse viejo porque no sabe que campos existen.
   *   3. Sirve a las cinco cabeceras. El switch solo sabe de $STATUS y DESCARTA en
   *      silencio lo que no nombra -los cuatro contadores de $ALARM, por ejemplo-.
   *
   * El bloque viene LITERAL de _camposNmea() en app.js (CLAUDE.md 3.bis: reescribir
   * logica ya probada para renombrar llamadas es como se cuelan los errores).
   *
   * Recibe las PARTES ya separadas por ',' -incluida la cabecera en el indice 0, que se
   * salta- porque asi la recibe el juez de la app, que ya ha partido la trama para
   * decidir el tipo. Partirla otra vez seria una cuarta copia de medio bucle.
   */
  camposDeTrama(parts) {
    const data = {};
    for (let i = 1; i < parts.length; i++) {
      // N-62: el separador de campo es ',' y el de clave/valor es el PRIMER ':'.
      // Con split(':') a secas, HORA:18:25:00 entraba como '18' y el reloj en vivo
      // mostraba la hora truncada. Se corta por el primer ':' y el resto es valor.
      const sep = parts[i].indexOf(':');
      const k = sep > 0 ? parts[i].slice(0, sep) : null;
      const v = sep > 0 ? parts[i].slice(sep + 1) : undefined;
      if (k && v !== undefined) data[k] = v;
    }
    return data;
  },

  /**
   * Calcula el checksum XOR estándar NMEA (formato hexadecimal de 2 caracteres)
   */
  calcularChecksum(payload) {
    let crc = 0;
    for (let i = 0; i < payload.length; i++) {
      crc ^= payload.charCodeAt(i);
    }
    return crc.toString(16).toUpperCase().padStart(2, '0');
  },

  /**
   * Formatea un comando con prefijo $, checksum *CRC y terminador \r\n
   */
  formatearTrama(payload) {
    const crc = this.calcularChecksum(payload);
    return `$${payload}*${crc}\r\n`;
  },

  /**
   * Valida si una trama NMEA recibida tiene formato y checksum correctos
   *
   * ESTA FUNCION ESTUVO ESCRITA Y SIN UN SOLO LLAMADOR, y no era un detalle: mientras
   * nadie la llamaba, la app PINTABA TRAMAS CON EL CHECKSUM MALO. parseNmeaTelemetry()
   * hacia `line.split('*')[0]` y tiraba el CRC sin mirarlo, asi que una trama que llego
   * corrompida por la radio -un byte cambiado en ESTADO:, en T: o en MODO:- entraba en
   * la pantalla como si fuera buena. El pack app_07 la tenia fichada como huerfana
   * conocida, que es la unica razon de que se supiera.
   *
   * Desde hoy la llama parseNmeaTelemetry(). Lo que se calcula es EL MISMO XOR que el
   * firmware: Maestro/src/bluetooth.cpp, enviarTramaConCrc() hace el XOR sobre
   * `payload + 1`, o sea saltando el '$', y lo escribe con "%02X". Aqui se salta el '$'
   * igual y se compara en mayusculas.
   *
   * Devuelve, ademas del `error` de siempre, un `motivo` en clave. El texto es para el
   * tecnico y el motivo es para contar: agrupar rechazos por una frase en castellano se
   * rompe el dia que alguien corrija una tilde.
   */
  validarTrama(linea) {
    const trimmed = String(linea === undefined || linea === null ? '' : linea).trim();
    if (!trimmed.startsWith('$') || !trimmed.includes('*')) {
      return {
        valida: false,
        motivo: 'SIN_FORMA',
        error: 'Formato NMEA inválido (falta $ o *)'
      };
    }

    const payloadConDolar = trimmed.substring(0, trimmed.indexOf('*'));
    const payload = payloadConDolar.substring(1);
    const crcRecibido = trimmed.substring(trimmed.indexOf('*') + 1, trimmed.indexOf('*') + 3).toUpperCase();
    const crcCalculado = this.calcularChecksum(payload);

    if (crcRecibido !== crcCalculado) {
      return {
        valida: false,
        motivo: 'CHECKSUM',
        esperado: crcCalculado,
        recibido: crcRecibido,
        error: `Checksum inválido (esperado: ${crcCalculado}, recibido: ${crcRecibido})`
      };
    }

    return { valida: true, payload };
  },

  /**
   * LA VISTA TIPADA de un $STATUS: los mismos campos, con nombre corto y ya convertidos.
   *
   * QUE CAMBIO EL 05/09 Y QUE NO. Antes esto tenia SU PROPIO bucle de partir la trama, y
   * ahi estaba el defecto: dos parsers, dos convenios, y el que tenia las pruebas no era
   * el que corria en el telefono. Ahora parte con camposDeTrama() -la MISMA funcion que
   * usa la app-, y lo unico que queda aqui es la capa de arriba: renombrar y convertir.
   *
   * O sea que ya no hay dos parsers. Hay UNO, y encima una vista.
   *
   * POR QUE ESTA VISTA NO SE RETIRA, que es la pregunta honesta: tiene un consumidor de
   * produccion fuera de la app. `01_Firmware/Simulaciones/simulador_app_bluetooth.py`
   * -que corre en la compuerta- carga ESTE fichero con node y compara campo a campo lo
   * que el micro modelado emitio contra lo que el parser devuelve, y su tabla CAMPOS
   * espera estos nombres cortos Y ESTOS TIPOS: `restante` entero, `rf`/`rtt` entero o
   * marca, `bat` decimal, `esc` de un conjunto cerrado. Borrarla dejaria ese arnes en
   * ABORTADO, que es una puerta abierta y no una casilla pendiente (CLAUDE.md 3.quater).
   *
   * LO QUE SIGUE ABIERTO, escrito aqui para que no se lea como cerrado: el `switch` de
   * abajo es una LISTA de campos, o sea una copia mas del contrato del cable. Desde hoy
   * la vigila `app_12_un_solo_parser`, que exige que esta lista y la que la app lee de
   * verdad en su rama de $STATUS sean la MISMA. Mientras esa lista exista hay algo que
   * sincronizar; lo que ya no hay es alguien que lo sincronice a mano sin que nadie mire.
   */
  parseStatus(payload) {
    // Contrato REAL, leido de 01_Firmware/Maestro/src/bluetooth.cpp:216. Antes este
    // parser describia FASE, TOT_FASE, BAT1, BAT2, RADIO y TELA, que no emite ninguna
    // punta, y rellenaba defaults -AUTO / VERDE_P1 / 12.0 V- que hacian pasar por
    // equipo sano una trama vacia. El pack documentos_03_trama_status compara esta
    // lista contra la del firmware: si divergen, falla.
    // Ejemplo: STATUS,NODE:MAESTRO,SERIE:M-2026-A1B2,MODO:AUTO,ESTADO:V1_R2,T:38,RF:98%,RTT:82ms,BAT:12.6,HORA:18:25:00
    const tokens = payload.split(',');
    if (tokens[0] !== 'STATUS') return null;

    // Sin valores por defecto: lo que la trama no trae, no aparece. Un campo ausente
    // tiene que notarse, no rellenarse.
    const data = { tipo: 'STATUS' };

    // El bucle propio que habia aqui SE RETIRA: era la segunda copia de la regla del
    // primer ':' -la de N-62-, y una regla arreglada en dos sitios es una regla que
    // alguien va a arreglar en uno.
    const crudos = this.camposDeTrama(tokens);
    for (const k of Object.keys(crudos)) {
      const v = crudos[k];
      switch (k) {
        case 'NODE': data.node = v; break;
        case 'SERIE': data.serie = v; break;
        case 'MODO': data.modo = v; break;
        case 'ESTADO': data.estado = v; break;
        // El `|| 0` que habia aqui es el MISMO defecto que app.js documenta haber
        // quitado de su propio camino, y este modulo se lo quedo: con un campo que no
        // fuera un numero -"--", vacio, "N/A"- devolvia **0**, o sea el peor enlace
        // medible, la bateria a cero y una latencia perfecta, sin que nadie hubiera
        // medido nada. "No lo se" y "va fatal" son cosas distintas.
        //
        // Desde N-108 las dos puntas MARCAN la ausencia en vez de inventarse la cifra,
        // asi que el valor llegaba bueno y era el parser quien lo estropeaba. Ahora el
        // marcador se devuelve tal cual y decide quien pinta, que sabe declararlo.
        //
        // 🔴 Y ESE ARREGLO ALCANZO A RF, RTT Y BAT Y DEJO FUERA A `T`, JUSTO DEBAJO DE
        // ESTE PARRAFO. Diez lineas explicando por que el `|| 0` esta mal y el `case`
        // siguiente conservandolo: la frase justificadora no protege al codigo que
        // tiene debajo, solo tapa que sigue ahi. Se cierra el 04/09 con N-139, que es
        // cuando `T` paso a poder valer "--": el Esclavo lo manda SIEMPRE asi
        // (Esclavo/src/bluetooth.cpp:776, "T:--" literal en el snprintf) y el Maestro
        // cuando no sabe cuanto falta (Maestro/src/bluetooth.cpp:833, "T:%s"). Con el
        // `|| 0` la app habria pintado un CERO -"faltan 0 segundos, cambia ya"- sobre
        // los dos casos en que el equipo acaba de decir que no lo sabe.
        //
        // Y OJO AL CASO QUE OBLIGA A NO COLAPSARLO: `T:0` es LEGITIMO -el ultimo
        // segundo de la fase-, asi que 0 y "no se sabe" tienen que llegar distintos a
        // quien pinta. Por eso se usa la misma _numeroOMarca() que los otros tres y no
        // un `|| null`: devuelve el 0 como numero y el "--" como texto.
        case 'T': data.restante = _numeroOMarca(v, parseInt, 10); break;
        case 'RF': data.rf = _numeroOMarca(v, parseInt, 10); break;
        case 'RTT': data.rtt = _numeroOMarca(v, parseInt, 10); break;
        case 'BAT': data.bat = _numeroOMarca(v, parseFloat); break;
        case 'HORA': data.hora = v; break;
        // N-149: lo que el MAESTRO sabe del ESCLAVO. Solo viaja en el $STATUS del
        // Maestro, y su cuarto valor -'?'- NO es un color: es esa punta declarando que
        // no lo sabe. Se devuelve TAL CUAL, sin traducir y sin defecto: quien pinta
        // (renderLights) es el que sabe distinguir "no vino" de "vino un ?" de "vino un
        // color", y son tres cosas distintas. Un `|| 'ROJO'` aqui seria el mismo `|| 0`
        // que este fichero documenta haber quitado tres casos mas arriba, con la
        // agravante de que aqui el valor inventado es una LUZ.
        case 'ESC': data.esc = v; break;
        // N-153: el estado de la TALANQUERA de la punta que habla. Lo emiten las DOS
        // -las dos placas llevan el motor en PB2-, asi que aqui no hay asimetria que
        // recordar. Se devuelve TAL CUAL y sin defecto, por el mismo motivo que ESC:
        // quien pinta tiene que poder distinguir "no vino" -firmware anterior- de
        // "vino ARRIBA" de "vino un literal que no conozco", y un valor inventado aqui
        // seria una BARRERA inventada, que es peor que una luz inventada: el conductor
        // le hace mas caso a la barrera que a la lampara.
        case 'PLUMA': data.pluma = v; break;
      }
    }

    return data;
  },

  /**
   * Parsea una trama de alarma $ALARM
   */
  parseAlarm(payload) {
    // Contrato REAL, bluetooth.cpp:50. Antes se leia por posicion -tokens[1] era el
    // codigo- y la trama del firmware es EVENTO:..,CAUSA:..,ACCION:..; el codigo que
    // salia era la cadena 'NODE:MAESTRO'.
    // Ejemplo: ALARM,NODE:MAESTRO,EVENTO:RADIO_FAIL,CAUSA:Timeout RS485,ACCION:AMBAR
    const tokens = payload.split(',');
    if (tokens[0] !== 'ALARM') return null;

    const data = { tipo: 'ALARM' };
    const crudos = this.camposDeTrama(tokens);
    for (const k of Object.keys(crudos)) {
      const v = crudos[k];
      switch (k) {
        case 'NODE': data.node = v; break;
        case 'EVENTO': data.codigo = v; break;
        case 'CAUSA': data.causa = v; break;
        case 'ACCION': data.accion = v; break;
        case 'HORA': data.hora = v; break;
      }
    }
    return data;
  },

  /**
   * Parsea una trama de error $ERR
   *
   * 🔴 ESTA FUNCION LEIA UN PROTOCOLO QUE NINGUNA PUNTA HABLA, Y SUS PRUEBAS TAMBIEN.
   *
   * Lo que habia era lectura POR POSICION: `cmd: tokens[1]`, `desc: tokens.slice(2)`.
   * Sobre la trama REAL -`ERR,CMD:SET_TIEMPOS,DESC:RANGO`, que es la que emiten las dos
   * bluetooth.cpp y el despachador del ESP32- eso devuelve `cmd = "CMD:SET_TIEMPOS"` y
   * `desc = "DESC:RANGO"`: el nombre del campo pegado delante del valor, en los dos.
   *
   * Y no saltaba porque la unica prueba que la ejercia le daba de comer
   * `'ERR,SET_MODO,PIN_INCORRECTO'`, una trama SIN claves que no sale de ningun micro
   * de este proyecto. Es la forma exacta que CLAUDE.md 3.quater ya conto una vez -"un
   * parser de un protocolo que ninguna punta habla"-, sostenida por un ejemplo inventado.
   * La prueba se invierte junto con esto (CLAUDE.md 8.quater).
   *
   * Se lee con la MISMA camposDeTrama() que el resto, que es lo que hace que no pueda
   * volver a desviarse: no queda ningun sitio donde escribir un segundo criterio.
   */
  parseError(payload) {
    const tokens = payload.split(',');
    if (tokens[0] !== 'ERR') return null;
    const crudos = this.camposDeTrama(tokens);
    // Sin defecto inventado para CMD. El 'UNKNOWN' que habia aqui es el mismo `|| 0`
    // que este fichero documenta haber quitado de BAT y de T: un valor de relleno que
    // se lee como dato. Si la trama no dice que orden rechazo, lo que hay que devolver
    // es que no lo dice.
    return {
      tipo: 'ERR',
      cmd: crudos.CMD,
      desc: crudos.DESC,
      // El $ERR del PUENTE trae NODE:PUENTE y el del STM32 no lo trae. Quien va a
      // destapar un conector necesita saber cual de los dos modulos se queja, y la app
      // ya lo distingue en su rama de $ERR: aqui se devuelve para que las dos puntas
      // del contrato digan lo mismo.
      node: crudos.NODE
    };
  },

  /**
   * Generador de tramas de comando protegidas por PIN
   */
  generarComando(pin, comando, args = '') {
    if (!pin || pin.length !== 4 || !/^\d{4}$/.test(pin)) {
      throw new Error('PIN inválido: debe contener exactamente 4 dígitos numéricos');
    }
    const payload = args ? `CMD:PIN:${pin}:${comando}:${args}` : `CMD:PIN:${pin}:${comando}`;
    // NO se envuelve con formatearTrama(). Las dos direcciones del cable NO tienen la
    // misma forma, y esta funcion llevaba tiempo componiendo la de la direccion
    // contraria: el firmware manda '$STATUS,...*XX' -con dolar y checksum- y la app
    // manda 'CMD:PIN:...' pelado y terminado en CR LF, que es lo que bluetooth.cpp
    // compara con strncmp.
    // Envuelta, la trama empieza por '$' y el despachador no la reconoce: el comando se
    // pierde entero. Es la forma que usa enviarComandoFirmware() de app.js, que es la
    // que de verdad viaja.
    return payload + '\r\n';
  }
};

if (typeof module !== 'undefined' && module.exports) {
  module.exports = NMEAParser;
}
