// ===== js/practica.js =====
// PRACTICA SIN POSTE (SPEC_4 9.10 y 9.14 (f); D-58, D-86, D-89, D-120). Una flota de postes simulados con estado
// (js/simulador.js): cada uno contesta cada orden con lo que emitiria su firmware. No pinta nada por su cuenta:
// sustituye window.bluetoothSerial por PuertoPractica y la app lo enlaza, lo oye y lo juzga por el mismo camino que a
// un equipo (abrirEnlace, subscribe, parseNmeaTelemetry, juzgarTrama). La barrera que roza es la de N-75 (un tablero
// que pinta un cruce que no existe): por eso la franja SIMULADO va encima de todo y el fichero sale marcado.
// Solo con la clave de administracion (body.admin) y nunca con un enlace abierto: la practica no desconecta a nadie.
// Sus grupos viven en un almacen en memoria (almacen()), nunca en el de la app. Las cintas js/practica_cinta_*.js
// quedan de referencia (D-121), fuera de index.html: tests/dom_simulador.js compara con ellas.
// Lo que no ensena: el tiempo real de respuesta, el emparejado de Android, la radio entre postes ni el puente ESP32.
const Practica = (() => {
  const mac = (n) => '02:00:00:00:00:' + String(n).padStart(2, '0');   // administradas localmente: no son de nadie
  const poste = (n, nodo, serie, fw, pareja, extra) => Object.assign({
    nodo, serie, fw, pareja, direccion: mac(n), rotulo: 'SEM-' + serie + '-' + nodo[0], radio: true, rf: 100, rtt: 196
  }, extra || {});
  const NOCHE = (rojo, fase) => ({ tr: 180, fase, destino: 'AUTO',
                                    franjas: [{ dias: 127, inicio: 22 * 60, fin: 5 * 60, rojo }] });
  // Un caso por poste (SPEC_4 9.14 (f)). Series y direcciones son de la practica; FW, los sellos de las cintas.
  const FLOTA = [
    poste(1, 'MAESTRO', 'A20001', 'd810f81', 'km20', { modo: 'AUTO' }),                     // en ciclo, PIN de fabrica
    poste(2, 'ESCLAVO', 'A20002', 'd810f81', 'km20', { modo: 'SUBORDINADO' }),
    poste(3, 'MAESTRO', 'A80001', 'd810f81', 'km80', { modo: 'NOCHE', noche: NOCHE(false, 'DESTELLO') }),
    poste(4, 'ESCLAVO', 'A80002', 'd810f81', 'km80', { modo: 'SUBORDINADO' }),
    poste(5, 'MAESTRO', 'C12001', 'd810f81', 'km120', { modo: 'MENU', radio: false, rf: 0 }),   // MENU sin radio
    poste(6, 'ESCLAVO', 'C12002', '8974932', 'km120', { modo: 'DEGRADADO', radio: false, rf: null }),
    poste(7, 'MAESTRO', 'D12001', 'd3606be', 'pr12', { modo: 'NOCHE', noche: NOCHE(true, 'SALIENDO') }),
    poste(8, 'ESCLAVO', 'D12002', 'd3606be', 'pr12', { modo: 'SUBORDINADO' }),
    poste(9, 'MAESTRO', 'D45001', 'd3606be', 'pr45', { modo: 'INTELIGENTE' }),              // FW distinto entre postes
    poste(10, 'ESCLAVO', 'D45E2B', '6bd1e4f', 'pr45', { modo: 'SUBORDINADO' }),             // y serie cambiada
    poste(11, 'MAESTRO', 'B0B0A1', 'd810f81', null, { modo: 'MENU', radio: false, rf: 0, rotulo: 'SEM-SIN-MATRICULA' })
  ];
  // Los grupos de la practica: 2 vias y 5 grupos; el poste 11 sin grupo. El ESCLAVO de PR 45 se matriculo con otra
  // serie (aviso de serie cambiada) y los dos de PR 45 con su FW (aviso de FW distinto).
  const hueco = (n, serie) => ({ mac: mac(n), serie: serie || FLOTA[n - 1].serie, fw: FLOTA[n - 1].fw,
                                 fecha: 'practica' });
  const grupo = (id, via, nombre, m, e, serieE) => ({ id: 'practica-' + id, via, nombre, pr: '', MAESTRO: hueco(m),
                                                     ESCLAVO: hueco(e, serieE) });
  const GRUPOS = [grupo('km20', 'Via 1', 'Km 20', 1, 2), grupo('km80', 'Via 1', 'Km 80', 3, 4),
                  grupo('km120', 'Via 1', 'Km 120', 5, 6), grupo('pr12', 'Via 2', 'PR 12', 7, 8),
                  grupo('pr45', 'Via 2', 'PR 45', 9, 10, 'D45E02')];

  return {
    FLOTA,
    RESPUESTA_MS: 100,       // el acuse llega a los 100 ms de la orden, como en la cinta (D-88)
    MARCA: 'Simulado: SI - practica sin poste, no vale como evidencia',
    reloj: null,             // inyectable (tests); null = el del telefono
    _real: null,             // el bluetoothSerial de verdad, guardado mientras dura
    _sim: null,

    activa() { return this._real !== null; },
    // Un almacen nuevo, en memoria, con los grupos de la practica: matricular aqui no toca el de la app.
    almacen() {
      let t = JSON.stringify({ v: 1, grupos: GRUPOS });
      return { leer: () => t, escribir: (x) => { t = x; } };
    },

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
      this._sim = Simulador.crear({ reloj: this.reloj || Simulador.relojReal(),
                                    postes: JSON.parse(JSON.stringify(FLOTA)),
                                    emitir: (p, t) => PuertoPractica._emitido(p, t) });
      this._real = window.bluetoothSerial || null;
      if (this._real === null) this._real = false;   // sin radio: se restaura a "nada", no a un objeto
      window.bluetoothSerial = PuertoPractica;
      document.body.classList.add('simulado');
      const franja = document.getElementById('franja-simulado');
      if (franja) franja.hidden = false;
      this._avisar('cyan', 'PRACTICA SIN POSTE: ' + FLOTA.length + ' postes simulados en 2 vias, que contestan ' +
                           'como su firmware. Ningun equipo real; el fichero exportado sale como SIMULADO. No ensena ' +
                           'el tiempo real de respuesta, el emparejado de Android ni la radio entre postes.');
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
      if (this._sim) this._sim.parar();   // ningun temporizador de la practica sobrevive a Salir
      this._sim = null;
      window.bluetoothSerial = this._real === false ? undefined : this._real;
      this._real = null;
      document.body.classList.remove('simulado');
      const franja = document.getElementById('franja-simulado');
      if (franja) franja.hidden = true;
      this._avisar('cyan', 'Practica sin poste terminada: la app vuelve a la radio de verdad.');
    },

    _tick() { PuertoPractica._siguiente(); }
  };
})();

// El cable falso. Mismos nombres y firmas que cordova-plugin-bluetooth-serial en lo que app.js usa. Cada enlace es
// una sesion: lo que estaba pendiente de una sesion no se entrega en otra ni tras Salir.
const PuertoPractica = {
  _poste: null,
  _cb: null,
  _ok: false,
  _sesion: 0,
  _timers: [],
  _durante: null,

  isEnabled(ok) { if (ok) ok(); },
  list(ok) { if (ok) ok(Practica.FLOTA.map(c => ({ name: c.rotulo, id: c.direccion, address: c.direccion }))); },
  connect(mac, ok, fail) {
    const p = Practica._sim ? Practica._sim.poste(mac) : null;
    if (!p) { if (fail) fail('no es un equipo de la practica'); return; }
    this._parar();
    this._poste = p;
    this._ok = true;
    if (ok) ok();
    this._programar(this._sesion);
  },
  subscribe(delim, cb) { this._cb = cb; },
  // La orden ya la anoto app.js (diario y cinta "-->"). La contesta el simulador, a los RESPUESTA_MS.
  write(data, ok) {
    if (ok) ok();
    if (!this._ok || !this._poste || !Practica._sim) return;
    // Lo que el poste emite al atender la orden ($EVENT) va DETRAS de su $ACK, como en el firmware y la cinta.
    this._durante = [];
    const ses = this._sesion, out = Practica._sim.recibir(this._poste, data).concat(this._durante);
    this._durante = null;
    if (out.length) this._tras(() => out.forEach(t => this._entregar(t, ses)), Practica.RESPUESTA_MS);
  },
  disconnect(ok) { this._parar(); if (ok) ok(); },
  isConnected(ok, fail) { if (this._ok) { if (ok) ok(); } else if (fail) fail(); },

  _reloj() { return Practica._sim ? Practica._sim.reloj : null; },
  _tras(fn, ms) {
    const r = this._reloj();
    if (!r) return;
    const h = r.programar(() => { this._timers = this._timers.filter(x => x !== h); fn(); }, ms);
    this._timers.push(h);
  },
  _entregar(t, ses) { if (this._ok && ses === this._sesion && this._cb) this._cb(t + '\r\n'); },
  // Lo que el simulador emite solo ($ACK diferido, $EVENT): llega si es del poste enlazado.
  _emitido(p, t) {
    if (p === this._poste) { if (this._durante) this._durante.push(t); else this._entregar(t, this._sesion); }
  },
  _siguiente() {
    if (this._ok && this._poste && Practica._sim) this._entregar(Practica._sim.status(this._poste), this._sesion);
  },
  _programar(ses) {
    if (!this._ok || ses !== this._sesion) return;
    this._tras(() => { this._siguiente(); this._programar(ses); }, Simulador.C.STATUS_MS);
  },
  _parar() {
    const r = this._reloj();
    if (r) this._timers.forEach(h => r.cancelar(h));
    this._timers = [];
    this._sesion++;
    this._ok = false;
    this._poste = null;
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
