// ===== js/practica.js =====
// PRACTICA SIN POSTE (SPEC_4 9.10; D-58, D-86..D-89). Una flota de cuatro equipos que repite, linea a linea, tramas
// que emitio un firmware en campo (js/practica_cinta_<n>.js, copiadas de evidencia/). No pinta nada por su cuenta:
// sustituye window.bluetoothSerial por PuertoPractica y la app la enlaza, la oye y la juzga por el mismo camino que a
// un equipo (abrirEnlace, subscribe, parseNmeaTelemetry, juzgarTrama). La barrera que roza es la de N-75 (un tablero
// que pinta un cruce que no existe): por eso la franja SIMULADO va encima de todo y el fichero sale marcado.
// Solo con la clave de administracion (body.admin) y nunca con un enlace abierto: la practica no desconecta a nadie.
// Lo que no ensena: el tiempo real de respuesta, el emparejado de Android ni la radio entre postes.
const Practica = {
  FLOTA: [typeof PRACTICA_CINTA_1 !== 'undefined' ? PRACTICA_CINTA_1 : null,
          typeof PRACTICA_CINTA_2 !== 'undefined' ? PRACTICA_CINTA_2 : null,
          typeof PRACTICA_CINTA_3 !== 'undefined' ? PRACTICA_CINTA_3 : null,
          typeof PRACTICA_CINTA_4 !== 'undefined' ? PRACTICA_CINTA_4 : null].filter(Boolean),
  CADENCIA_MS: 1000,       // una linea por segundo, como el $STATUS del firmware
  RESPUESTA_MS: 100,       // el acuse llega a los 100 ms de la orden (D-88)
  MARCA: 'Simulado: SI - practica sin poste, no vale como evidencia',
  _real: null,             // el bluetoothSerial de verdad, guardado mientras dura

  activa() { return this._real !== null; },

  // Los avisos van por app.js (window.appAvisar: toast y Eventos); sin el, en silencio.
  _avisar(tipo, txt) { if (typeof window.appAvisar === 'function') window.appAvisar(tipo, txt); },

  entrar() {
    if (this.activa() || !document.body.classList.contains('admin')) return false;
    if (typeof window.appEnlazada !== 'function' || window.appEnlazada()) {
      this._avisar('red', 'Practica sin poste NO iniciada: hay un enlace abierto con un equipo. Desconecte ' +
                          'primero; la practica no suelta a nadie por su cuenta.');
      return false;
    }
    // Lo anotado del poste real sale a su fichero ANTES: durante la practica RegistroSesion no escribe (D-89).
    if (typeof RegistroSesion !== 'undefined') RegistroSesion.volcar();
    this._real = window.bluetoothSerial || null;
    if (this._real === null) this._real = false;   // sin radio: se restaura a "nada", no a un objeto
    window.bluetoothSerial = PuertoPractica;
    document.body.classList.add('simulado');
    const franja = document.getElementById('franja-simulado');
    if (franja) franja.hidden = false;
    this._avisar('cyan', 'PRACTICA SIN POSTE: ' + this.FLOTA.length + ' equipos que repiten tramas de campo. ' +
                         'Ningun equipo real; el fichero exportado sale como SIMULADO. No ensena el tiempo real ' +
                         'de respuesta, el emparejado de Android ni la radio entre postes.');
    const btn = document.getElementById('btnDevice');
    if (btn) btn.click();
    return true;
  },

  salir() {
    if (!this.activa()) return;
    // Por desconectarEquipo() de app.js (el boton del modal), que llama al disconnect del puerto falso.
    const desc = document.getElementById('btn-bt-disconnect');
    if (typeof window.appEnlazada === 'function' && window.appEnlazada() && desc) desc.click();
    PuertoPractica._parar();
    window.bluetoothSerial = this._real === false ? undefined : this._real;
    this._real = null;
    document.body.classList.remove('simulado');
    const franja = document.getElementById('franja-simulado');
    if (franja) franja.hidden = true;
    this._avisar('cyan', 'Practica sin poste terminada: la app vuelve a la radio de verdad.');
  },

  // La orden que la app escribio, sin "CMD:", sin el PIN y sin el fin de linea.
  _orden(data) { return String(data || '').trim().replace(/^CMD:/, '').replace(/^PIN:[^:]*:/, ''); },

  // El primer $ACK/$ERR grabado de ese equipo cuyo CMD: es la orden (o su comando sin argumentos). Sin grabar: null.
  respuesta(equipo, data) {
    if (!equipo) return null;
    const orden = this._orden(data);
    const cmd = (t) => { const m = /CMD:([^,*]+)/.exec(t); return m ? m[1] : null; };
    const r = equipo.respuestas || [];
    return r.find(t => cmd(t) === orden) || r.find(t => cmd(t) && orden.indexOf(cmd(t) + ':') === 0) || null;
  },

  _tick() { PuertoPractica._siguiente(); }
};

// El cable falso. Mismos nombres y firmas que cordova-plugin-bluetooth-serial en lo que app.js usa.
const PuertoPractica = {
  _equipo: null,
  _i: 0,
  _cb: null,
  _reloj: null,
  _ok: false,

  isEnabled(ok) { if (ok) ok(); },
  list(ok) {
    if (ok) ok(Practica.FLOTA.map(c => ({ name: c.rotulo, id: c.direccion, address: c.direccion })));
  },
  connect(mac, ok, fail) {
    const eq = Practica.FLOTA.find(c => c.direccion === mac);
    if (!eq) { if (fail) fail('no es un equipo de la practica'); return; }
    this._parar();
    this._equipo = eq;
    this._i = 0;
    this._ok = true;
    if (ok) ok();
    this._programar();
  },
  subscribe(delim, cb) { this._cb = cb; },
  // La orden ya la anoto app.js (diario y cinta "-->"). Se contesta con lo grabado, o no se contesta.
  write(data, ok) {
    if (ok) ok();
    const r = Practica.respuesta(this._equipo, data);
    if (r) setTimeout(() => this._entregar(r), Practica.RESPUESTA_MS);
  },
  disconnect(ok) { this._parar(); if (ok) ok(); },
  isConnected(ok, fail) { if (this._ok) { if (ok) ok(); } else if (fail) fail(); },

  _entregar(t) { if (this._ok && this._cb) this._cb(t + '\n'); },
  _siguiente() {
    const eq = this._equipo;
    if (!this._ok || !eq || !eq.tramas.length) return;
    this._entregar(eq.tramas[this._i % eq.tramas.length]);
    this._i = (this._i + 1) % eq.tramas.length;   // al acabar vuelve a empezar
  },
  _programar() {
    if (this._reloj) clearTimeout(this._reloj);
    this._reloj = null;
    if (!this._ok || !(Practica.CADENCIA_MS > 0)) return;
    this._reloj = setTimeout(() => { this._siguiente(); this._programar(); }, Practica.CADENCIA_MS);
  },
  _parar() {
    if (this._reloj) clearTimeout(this._reloj);
    this._reloj = null;
    this._ok = false;
    this._equipo = null;
  }
};

// Los dos botones por delegacion: el documento existe al cargar y no depende de DOMContentLoaded.
if (typeof document !== 'undefined' && document.addEventListener) {
  document.addEventListener('click', (ev) => {
    const t = ev.target && ev.target.closest ? ev.target : null;
    if (!t) return;
    if (t.closest('#btn-practica')) Practica.entrar();
    else if (t.closest('#btn-franja-salir')) Practica.salir();
  });
}

if (typeof module !== 'undefined' && module.exports) {
  module.exports = Practica;
  module.exports.PuertoPractica = PuertoPractica;
}
