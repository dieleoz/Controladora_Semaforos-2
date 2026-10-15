// ===== js/registro_equipo.js =====
// SPEC_4 9.8 (D-75): EL REGISTRO DEL PUENTE, LADO APP. El puente guarda en su flash lo que subio del equipo con o
// sin telefono delante (D-72..D-74); la app lo pide con CMD:LOG,DESDE:<N> y recuerda por SERIE el ultimo N
// RECIBIDO (D-100). Es PASADO, no estado: va a Eventos (linea gris) y al .txt, nunca a AvisoCorte, AvisosEquipo,
// AvisoCamaraPluma, DiagnosticoEnlace ni al diario de ordenes. Escribe al cable solo por el enviar() que le pasa
// app.js (enviarComandoFirmware), y solo si el parte de arranque trajo LOG: (con un puente viejo la linea
// cruzaria a J17 y el STM32 contestaria AUTH_FAILED por una clave que nadie tecleo).
const RegistroEquipo = {
  CLAVE: 'iotvial_log_equipo_v1:',
  LOTES_MAX: 64,
  TOPE: 4096,
  _lista: [],
  descartados: 0,
  _ult: null,          // ULT del ultimo parte; null = puente sin registro (o sin parte todavia)
  _pedir: false,       // falta la primera peticion de esta conexion
  _serie: null,
  _lotes: 0,
  _reinicio: false,
  _ack: null,          // campos del ultimo $ACK CMD:LOG (contadores para el .txt)
  _hueco: false,

  limpiar() {
    this._lista = []; this.descartados = 0; this._ult = null; this._pedir = false;
    this._serie = null; this._lotes = 0; this._reinicio = false; this._ack = null; this._hueco = false;
  },

  _leer(serie) {
    try { return parseInt(localStorage.getItem(this.CLAVE + serie) || '0', 10) || 0; } catch (e) { return 0; }
  },
  _guardar(serie, n) {
    try { localStorage.setItem(this.CLAVE + serie, String(n)); } catch (e) { /* sin almacen: desde 0 */ }
  },

  // El parte de arranque del puente: con LOG: este puente lleva registro y se arma UNA peticion por conexion.
  verParte(data) {
    if (!data || data.NODE !== 'PUENTE' || data.EVT !== 'ARRANQUE') return false;
    const ok = /^\d+$/.test(data.LOG || '');
    this._ult = ok ? parseInt(data.LOG, 10) : null;
    this._pedir = ok; this._lotes = 0; this._reinicio = false; this._hueco = false;
    return ok;
  },

  _pedirDesde(n, enviar, addEvent) {
    if (this._lotes >= this.LOTES_MAX) {
      const quedan = this._ack ? Number(this._ack.ULT) - n + 1 : '?';
      if (addEvent) addEvent('cyan', 'Registro del equipo: quedan ' + quedan + ' entradas en el equipo: reconecte.');
      return false;
    }
    this._lotes++;
    return enviar('LOG,DESDE', String(n));
  },

  // Con el primer $STATUS con SERIE tras el parte (molde REINICIOS, D-64).
  peticion(serie, enviar) {
    if (!this._pedir || !serie) return false;
    this._pedir = false; this._serie = serie;
    const rec = this._leer(serie);
    if (rec === this._ult) return false;          // D-100: ya lo tiene todo; ULT < rec lo resuelve el $ACK
    return this._pedirDesde(rec + 1, enviar);
  },

  // El $ACK no manda ordenes (app_05): si hay que volver a pedir, se arma y sale con el $STATUS siguiente.
  verAck(data, addEvent) {
    if (!data || data.CMD !== 'LOG' || data.NODE !== 'PUENTE' || !this._serie) return false;
    this._ack = data;
    const ult = Number(data.ULT) || 0;
    const primero = Number(data.PRIMERO) || 0;
    const rec = this._leer(this._serie);
    if (ult < rec) {
      this._guardar(this._serie, 0);
      addEvent('cyan', 'Registro del equipo: el puente tiene ' + ult + ' entradas y ya se habian recibido ' + rec +
               ' (particion borrada o puente cambiado): se vuelve a pedir desde 1 con el siguiente $STATUS.');
      if (!this._reinicio) { this._reinicio = true; this._pedir = true; }
      return true;
    }
    if (primero > rec + 1) {
      addEvent('cyan', 'Registro del equipo: el equipo ya no tiene las entradas ' + (rec + 1) + '..' +
               (primero - 1) + '.');
      this._guardar(this._serie, primero - 1);   // D-100: sin hueco despues
    }
    return true;
  },

  _decodificar(d) {
    return String(d || '').replace(/;/g, ',').replace(/=/g, ':');
  },

  verRegistro(data, addEvent, enviar) {
    if (!data || data.NODE !== 'PUENTE' || data.EVT !== 'LOG') return false;
    const n = Number(data.N) || 0;
    const cuando = data.FECHA && data.FECHA !== '-' ? data.FECHA + ' ' + (data.HORA || '')
                                                    : '+' + (data.UP || '?') + ' s, sin hora';
    const r = { n: n, cuando: cuando, c: data.C || '?', texto: this._decodificar(data.D) };
    addEvent('gray', 'Equipo [LOG] N:' + n + ' ' + cuando + ' ' + r.c + ': ' + r.texto);
    this._lista.push(r);
    if (this._lista.length > this.TOPE) { this._lista.shift(); this.descartados++; }
    if (!this._serie) return true;
    const rec = this._leer(this._serie);
    if (n > rec) this._guardar(this._serie, n);       // D-100: el ultimo RECIBIDO; N <= rec no lo mueve
    if (n > rec + 1 && !this._hueco) {                // una vez por conexion, y avanza igual
      this._hueco = true;
      addEvent('cyan', 'Registro del equipo: hueco en el registro del equipo: faltan ' + (rec + 1) + '..' +
               (n - 1) + '.');
    }
    const hasta = this._ack ? Number(this._ack.HASTA) || 0 : 0;
    const ult = this._ack ? Number(this._ack.ULT) || 0 : 0;
    if (n === hasta && hasta < ult) this._pedirDesde(hasta + 1, enviar, addEvent);
    return true;
  },

  // Traduccion de los dos $ERR de CMD:LOG (para _traducirRechazo de app.js); null si no es de LOG.
  rechazo(data) {
    if (!data || data.CMD !== 'LOG' || data.NODE !== 'PUENTE') return null;
    if (data.DESC === 'SIN_FLASH') {
      return { texto: 'el puente no encuentra su particion de registro: reportelo.',
               toast: 'El puente no encuentra su registro: reportelo' };
    }
    if (data.DESC === 'FORMATO_INVALIDO') {
      return { texto: 'defecto de la app: reportelo con esta linea.', toast: 'Defecto de la app: reportelo' };
    }
    return null;
  },

  aTexto() {
    const a = this._ack;
    const l = ['IOT-VIAL - REGISTRO DEL EQUIPO',
               'Lo que el puente guardo en su flash (SPEC_4 9.8), pedido por la app en esta sesion.',
               a ? 'Ultimo $ACK: ' + ['DESDE', 'HASTA', 'ULT', 'PRIMERO', 'CONG', 'CRC', 'LARGO', 'ANILLO', 'FLASH',
                                      'MALOS'].map(k => k + ':' + (a[k] === undefined ? '?' : a[k])).join(' ')
                 : 'Sin $ACK de LOG en esta sesion (puente sin registro o sin pedir).'];
    if (this.descartados) l.push('RECORTADO: se tiraron las ' + this.descartados + ' entradas mas antiguas.');
    l.push('');
    if (!this._lista.length) l.push('  (vacio: no ha llegado ninguna entrada del registro del equipo)');
    this._lista.forEach(r => l.push('N:' + r.n + '  ' + r.cuando + '  ' + r.c + '  ' + r.texto));
    return l.join('\n') + '\n';
  }
};

if (typeof module !== 'undefined' && module.exports) module.exports = RegistroEquipo;
