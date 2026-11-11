// ===== js/instalacion.js =====
// INSTALACION: CAMARAS Y BARRERA (SPEC_8 8.6, D-161..D-165; N-187). Ajustes > plegado "Instalacion: camaras y
// barrera", tras la clave de administracion (D-56) como "Averias". Ordenes locales al poste, con PIN como las de 6.1:
//   SET_CAMARA:<C|D>:<NA|NC>   SET_PLUMA_RETARDO:<s>   y CONSULTA_INSTALACION sin PIN (SIN_PIN de app.js).
// LO QUE SE PINTA ES LO QUE CONTESTA EL EQUIPO, nunca lo pulsado: al abrir el plegado se consulta, y tras cada OK
// se vuelve a consultar. Formato de la consulta (8.6): RESULT:C_NA_D_NA_R3,VE:C0D0. Cada $ACK lleva VE:.
// El firmware devuelve el CMD con su valor (CMD:SET_CAMARA:C:NC); sin coma en el valor, la coma no parte el campo.
// Firmware sin N-187 (Maestro y Esclavo de 9edb650): la consulta sale sin PIN, no esta en su lista sin PIN y los
// dos contestan $ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO, sin repetir el CMD. El panel lo dice (rechazo()).
// CADA POSTE EL SUYO (SPEC_4 6): al cambiar de poste o desconectar el panel vuelve a "--" y, abierto, consulta.
// EL RANGO DEL RETARDO VIVE SOLO AQUI (RETARDO): lo usa el selector y lo lee js/sim_instalacion.js. Decidido por el
// responsable el 09/11 (P3): 3..5 s enteros, fabrica 3 s. Si el firmware lo cambia, se cambia aqui y en su fuente.

const Instalacion = {
  CONSULTA: 'CONSULTA_INSTALACION',
  SIN_FW: 'Este firmware no tiene la instalación de cámaras y barrera. Qué falta: cargar el firmware nuevo en ' +
          'este poste.',
  RETARDO: { MIN: 3, MAX: 5, FABRICA: 3 },
  // Borde de rechazo(): un AUTH_FAILED se toma por la consulta solo dentro de este plazo desde que salio. Elegido,
  // no medido: la consulta es local al poste (sin radio) y el AUTH_FAILED sale en el mismo parseo de la orden.
  PLAZO_MS: 3000,
  VE_AYUDA: 'Con nadie delante debe salir 0; si sale 1, contacto mal declarado o algo delante.',
  CONTACTO: {
    NA: 'NA: la cámara CIERRA el contacto cuando ve algo (de fábrica).',
    NC: 'NC: la cámara ABRE el contacto cuando ve algo.'
  },

  RECHAZOS: {
    FORMATO_INVALIDO: () => ({
      texto: 'El equipo no entendió la orden y no cambió nada. Qué falta: repetirla desde estos botones; si ' +
             'pasa siempre, la app y el equipo no hablan la misma versión: avise.',
      toast: 'El equipo no entendió la orden' }),
    FUERA_DE_RANGO: () => ({
      texto: 'El equipo NO cambió el retardo: está fuera de ' + Instalacion._rango() + '. Qué falta: elegir ' +
             'un retardo de ' + Instalacion._rango() + ' y aplicarlo otra vez.',
      toast: 'Retardo fuera de ' + Instalacion._rango() }),
    PLUMA_VETADA: () => ({
      texto: 'El equipo NO cambió el contacto: la cámara está reteniendo la barrera ahora mismo, y el cambio no ' +
             'suelta un veto en curso. Qué falta, en este orden: 1) en Averías, «Sacar las cámaras del veto» (la ' +
             'barrera baja tras su retardo); 2) cambiar aquí el contacto; 3) consultar y comprobar que la ' +
             'presencia da 0 con nadie delante; 4) en Averías, «Devolver el veto».',
      toast: 'No cambiado: la cámara retiene la barrera (ver pasos)' }),
    NO_QUEDO_PUESTO: () => ({
      texto: 'El equipo NO lo dejó puesto: después de guardarlo lo volvió a leer y no es lo pedido. Qué falta: ' +
             'consultar, repetir la orden y, si vuelve a pasar, avisar: la memoria del equipo puede estar mal.',
      toast: 'No quedó puesto: repita y, si se repite, avise' })
  },

  _rango() { return this.RETARDO.MIN + ' a ' + this.RETARDO.MAX + ' s'; },

  _orden(cmd) {
    const o = String(cmd || '').split(':')[0];
    return ['SET_CAMARA', 'SET_PLUMA_RETARDO', this.CONSULTA].indexOf(o) >= 0 ? o : null;
  },

  // VE:C<0|1>D<0|1> -> frase; null si no viene o no se reconoce (se muestra en crudo).
  _ve(ve) {
    const m = /^C([01])D([01])$/.exec(String(ve || ''));
    if (!m) return null;
    const unos = ['C', 'D'].filter((x, i) => m[i + 1] === '1');
    return { c: m[1], d: m[2], unos,
      texto: 'Presencia ahora: cámara C ' + m[1] + ', cámara D ' + m[2] + '. ' + this.VE_AYUDA +
             (unos.length ? ' Qué falta: con nadie delante, revisar el contacto declarado de la cámara ' +
                            unos.join(' y ') + ' o lo que tiene delante.' : '') };
  },

  _pintar(id, txt) {
    const el = typeof document !== 'undefined' && document.getElementById(id);
    if (el) el.textContent = txt;
  },

  // RESULT:C_NA_D_NC_R4 -> el panel. Lo que no casa se pinta en crudo, no se adivina.
  _pintarConsulta(data) {
    const m = /^C_(NA|NC)_D_(NA|NC)_R(\d+)$/.exec(String(data.RESULT || ''));
    const ve = this._ve(data.VE);
    if (!m) {
      ['inst-c', 'inst-d', 'inst-r'].forEach(id => this._pintar(id, '!'));
      this._pintar('inst-falta', 'Qué falta: el equipo contestó "' + (data.RESULT || '') + '" y esta app no lo ' +
                   'reconoce. Avise: la app y el equipo no hablan la misma versión.');
    } else {
      this._pintar('inst-c', m[1]); this._pintar('inst-d', m[2]); this._pintar('inst-r', m[3] + ' s');
      this._pintar('inst-falta', ve && ve.unos.length ? ve.texto.slice(ve.texto.indexOf('Qué falta:'))
        : 'Qué falta: nada si coincide con lo instalado. Compruebe con nadie delante que la presencia da 0.');
    }
    this._pintar('inst-ve', ve ? 'C ' + ve.c + ' · D ' + ve.d : (data.VE ? '! ' + data.VE : '--'));
    return { m, ve };
  },

  acuse(data) {
    const o = data && this._orden(data.CMD);
    if (!o) return null;
    if (o === this.CONSULTA) this._enCurso = false;
    const ve = this._ve(data.VE), cola = ve ? ' ' + ve.texto : ' El equipo no trajo la presencia: consulte.';
    if (o === this.CONSULTA) {
      const r = this._pintarConsulta(data);
      if (!r.m) return { tono: 'amber', texto: 'Equipo: instalación sin reconocer (' + (data.RESULT || '') + ').',
                         toast: 'Instalación: respuesta no reconocida' };
      return { tono: ve && ve.unos.length ? 'amber' : 'cyan',
               texto: 'Equipo: cámara C ' + r.m[1] + ', cámara D ' + r.m[2] + ', retardo ' + r.m[3] + ' s.' + cola,
               toast: 'Instalación consultada' };
    }
    const que = o === 'SET_CAMARA' ? 'el contacto de la cámara' : 'el retardo de la barrera';
    if (data.RESULT === 'SIN_CAMBIO') {
      return { tono: 'amber', texto: 'Equipo: ' + que + ' ya estaba como lo pidió: no se cambió nada.' + cola,
               toast: 'Ya estaba así: nada que cambiar' };
    }
    if (data.RESULT !== 'OK') return null;
    this._consultar();   // lo que se pinta lo dice el equipo, no el boton
    return { tono: ve && ve.unos.length ? 'amber' : 'green',
             texto: 'Equipo: ' + que + ' guardado y releído.' + cola, toast: 'Instalación guardada en el equipo' };
  },

  // AUTH_FAILED no nombra la orden: se toma por la consulta solo si es lo ultimo que salio de este panel y dentro
  // de PLAZO_MS (M1: una consulta sin respuesta no se queda la culpa del siguiente PIN mal tecleado). Va despues de
  // Noche.rechazo() (app.js), que se queda el suyo si la de noche tambien esta en vuelo. DESCONOCIDO no: ningun
  // firmware lo contesta a la consulta.
  rechazo(data) {
    const cmd = data && String(data.CMD || '');
    const enVuelo = this._enCurso && Date.now() - this._enCursoDesde <= this.PLAZO_MS;
    if (cmd === 'AUTH_FAILED' || cmd === this.CONSULTA) this._enCurso = false;
    if (enVuelo && cmd === 'AUTH_FAILED' && data.NODE !== 'PUENTE') {
      this._vaciar(this.SIN_FW);
      return { texto: this.SIN_FW, toast: 'Instalación: no está en este firmware' };
    }
    const o = cmd && this._orden(cmd), f = o && this.RECHAZOS[data.DESC];
    return f ? f() : null;
  },

  _consultar() {
    if (!this.ctx || !this.ctx.enviarComandoFirmware(this.CONSULTA)) return;
    this._enCurso = true; this._enCursoDesde = Date.now();
  },

  _vaciar(falta) {
    ['inst-c', 'inst-d', 'inst-r', 'inst-ve'].forEach(id => this._pintar(id, '--'));
    this._pintar('inst-falta', falta);
  },

  // Molde Noche.alStatus(): otro poste (o ninguno) -> lo pintado era del anterior. Se vacia y, con el plegado
  // abierto y un poste delante, se consulta sin esperar a cerrar y abrir (_abierto no la frena).
  alStatus(node) {
    if (typeof Interruptores !== 'undefined') Interruptores.alStatus(node);   // D-177: los selectores de Averias, igual
    if (node === this._nodo) return;
    this._nodo = node; this._enCurso = false;
    this._vaciar(node ? 'Consultando este poste.' : 'Qué falta: conectar con un poste y consultar.');
    const det = typeof document !== 'undefined' && document.getElementById('instalacion');
    if (node && det && det.open) this._consultar();
  },

  _enviar(orden, valor) {
    const c = this.ctx;
    if (!c.state.pinVerificado) { c.pedirPin(() => this._enviar(orden, valor)); return; }
    if (!c.enviarComandoFirmware(orden, valor)) return;
    this._enCurso = false;   // lo ultimo que salio ya no es la consulta
    c.addEvent('cyan', 'Técnico: orden ' + orden + ':' + valor + ' enviada al equipo. Espere el acuse.');
  },

  // El cuerpo del plegado se escribe aqui (index.html no crece): una fila por valor con lo que se espera de el.
  _cuerpo() {
    const fila = (id, nombre) => '<p class="ajuste"><strong>' + nombre + ':</strong> <span id="inst-' + id +
      '">sin consultar</span><small id="inst-' + id + '-ayuda"></small></p>';
    const camara = (x) => fila(x.toLowerCase(), 'Cámara ' + x) + '<div class="ctrl-grid">' + ['NA', 'NC'].map(v =>
      '<button class="ctrl-btn" data-instalacion="SET_CAMARA:' + x + ':' + v + '"><strong>Cámara ' + x + ': ' + v +
      '</strong></button>').join('') + '</div>';
    let ops = '';
    for (let s = this.RETARDO.MIN; s <= this.RETARDO.MAX; s++) {
      ops += '<option value="' + s + '">' + s + ' s' + (s === this.RETARDO.FABRICA ? ' (fábrica)' : '') + '</option>';
    }
    return '<p class="card-sub">Solo este poste. Cambiar pide PIN; consultar, no. Tras cada carga de firmware ' +
      'el equipo vuelve a fábrica (NA, NA, ' + this.RETARDO.FABRICA + ' s): consulte y vuelva a ponerlo.</p>' +
      camara('C') + camara('D') + fila('r', 'Retardo de bajada') +
      '<div class="form-group"><select id="inst-retardo" class="form-input">' + ops + '</select> ' +
      '<button type="button" class="btn-save-site" id="btn-inst-retardo">Aplicar retardo</button></div>' +
      fila('ve', 'Presencia ahora') + '<p class="card-sub" id="inst-falta"></p>' +
      '<button type="button" class="btn-secondary-bt" id="btn-inst-consultar">Consultar otra vez</button>';
  },

  iniciar(ctx) {
    this.ctx = ctx;
    if (typeof document === 'undefined') return;
    const det = document.getElementById('instalacion'), caja = document.getElementById('instalacion-cuerpo');
    if (!det || !caja) return;
    caja.innerHTML = this._cuerpo();
    const ayudaContacto = this.CONTACTO.NA + ' ' + this.CONTACTO.NC;
    ['c', 'd'].forEach(x => this._pintar('inst-' + x + '-ayuda', ayudaContacto));
    this._pintar('inst-r-ayuda', 'Segundos que la barrera espera, con la luz ya en ROJO, antes de bajar. De ' +
                 this._rango() + '; de fábrica ' + this.RETARDO.FABRICA + ' s. Nunca baja en amarillo ni en ámbar.');
    this._pintar('inst-ve-ayuda', this.VE_AYUDA);
    // Se consulta al ABRIR; el navegador puede dar mas de un toggle por apertura: uno solo por apertura.
    det.addEventListener('toggle', () => {
      if (!det.open) { this._abierto = false; return; }
      if (this._abierto) return;
      this._abierto = true; this._consultar();
    });
    caja.querySelectorAll('[data-instalacion]').forEach(btn => btn.addEventListener('click', () => {
      const t = btn.getAttribute('data-instalacion'), sep = t.indexOf(':');
      this._enviar(t.slice(0, sep), t.slice(sep + 1));
    }));
    const sel = document.getElementById('inst-retardo');
    const aplicar = () => this._enviar('SET_PLUMA_RETARDO', sel.value);
    document.getElementById('btn-inst-retardo').addEventListener('click', aplicar);
    document.getElementById('btn-inst-consultar').addEventListener('click', () => this._consultar());
  }
};

if (typeof module !== 'undefined' && module.exports) {
  module.exports = Instalacion;
}
