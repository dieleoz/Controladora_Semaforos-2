// ===== js/nmea_parser.js =====
// Parser y Formateador de Tramas NMEA de Telemetría Semafórica

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
   */
  validarTrama(linea) {
    const trimmed = linea.trim();
    if (!trimmed.startsWith('$') || !trimmed.includes('*')) {
      return { valida: false, error: 'Formato NMEA inválido (falta $ o *)' };
    }

    const payloadConDolar = trimmed.substring(0, trimmed.indexOf('*'));
    const payload = payloadConDolar.substring(1);
    const crcRecibido = trimmed.substring(trimmed.indexOf('*') + 1, trimmed.indexOf('*') + 3).toUpperCase();
    const crcCalculado = this.calcularChecksum(payload);

    if (crcRecibido !== crcCalculado) {
      return { valida: false, error: `Checksum inválido (esperado: ${crcCalculado}, recibido: ${crcRecibido})` };
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
        case 'T': data.restante = parseInt(v, 10) || 0; break;
        case 'RF': data.rf = parseInt(v, 10) || 0; break;
        case 'RTT': data.rtt = parseInt(v, 10) || 0; break;
        case 'BAT': data.bat = parseFloat(v) || 0; break;
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
    return this.formatearTrama(payload);
  }
};

if (typeof module !== 'undefined' && module.exports) {
  module.exports = NMEAParser;
}
