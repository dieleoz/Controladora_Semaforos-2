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
   * Parsea una trama de telemetría $STATUS
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

    for (let i = 1; i < tokens.length; i++) {
      // El separador de clave/valor es el PRIMER ':' y no cualquiera: HORA:18:25:00
      // se trunca a '18' con un split(':') a secas (N-62).
      const sep = tokens[i].indexOf(':');
      if (sep <= 0) continue;
      const k = tokens[i].slice(0, sep);
      const v = tokens[i].slice(sep + 1);
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
    for (let i = 1; i < tokens.length; i++) {
      const sep = tokens[i].indexOf(':');
      if (sep <= 0) continue;
      const k = tokens[i].slice(0, sep);
      const v = tokens[i].slice(sep + 1);
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
   */
  parseError(payload) {
    const tokens = payload.split(',');
    if (tokens[0] !== 'ERR') return null;
    return {
      tipo: 'ERR',
      cmd: tokens[1] || 'UNKNOWN',
      desc: tokens.slice(2).join(',')
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
