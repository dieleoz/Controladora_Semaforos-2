// ===== js/asistente_deg.js =====
// EL ASISTENTE DE RADIO CAIDA (D-177; SPEC_4 9.16, 3.ter y 3.ter.ter). Una hoja que guia, paso a paso y en los dos
// postes, la entrada en Degradado con testigo y su salida programada, con los valores por defecto ya puestos.
//
// LO QUE ESTE MODULO NO HACE, Y ES A PROPOSITO:
//   - No manda nada por su cuenta: escribe los campos de las tarjetas de Ajustes y pulsa Testigo._pulsar() o
//     DegFin._pulsar(). El PIN, el SET_RTC previo, el aviso de paleteros (D-81), lo guardado y los textos de rechazo
//     son los de js/testigo.js y js/deg_fin.js, sin copia.
//   - No lleva la cuenta de pasos: la pantalla sale de lo GUARDADO (testigo y salida) y del poste conectado, asi que
//     sobrevive a cerrar la app. Lo unico en memoria es la comprobacion en persona del POSTE 1 y la eleccion del flujo.
//   - No decide si la radio esta caida: lo comprueba la persona (pantallas 1 y 6); ni la app ni el firmware lo miran.
// Se ve con la clave de Degradado (body.deg) o la de tecnico (body.admin); la barra «Volver al asistente», solo en
// Degradado. El cartel de «falta el otro poste» (#cartel-testigo) se ve en los tres modos. Sin jerga (SPEC_4 9.9 (a)).
const AsistenteDeg = {
  _abierta: false,
  _flujo: null,               // null (elegir) | 'entrar' | 'salir'
  _comprobado: false,         // la casilla de la pantalla 1, solo en memoria
  _rehacerDe: null,           // emitidoMs de un testigo que la persona dio por terminado o vencido
  _key: null,

  c() { return typeof Testigo !== 'undefined' ? Testigo.ctx : null; },
  $(id) { return document.getElementById(id); },
  ESC: { '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;' },
  esc(t) { return String(t == null ? '' : t).replace(/[&<>"]/g, (x) => this.ESC[x]); },
  practica() { return typeof Practica !== 'undefined' && Practica.activa(); },
  _g() { const g = Testigo.leer(); return g && g.emitidoMs === this._rehacerDe ? null : g; },
  _viejo(g) { return !!(g && g.inicioMs && Date.now() - g.inicioMs > Testigo.MEDIO_DIA_S * 1000); },

  // La pareja segun los grupos (js/grupos.js): null si no se sabe; si no, { ok, nombre, espera, es }.
  pareja(serieM) {
    const c = this.c(), st = c.state;
    const serieE = st.serieDe === st.deviceMac ? st.serie : null;
    let gs = [];
    try { gs = c.grupos ? c.grupos().todos() : []; } catch (e) { gs = []; }
    const g = gs.find(x => x.MAESTRO && serieM && x.MAESTRO.serie === serieM);
    if (!g || !g.ESCLAVO || !g.ESCLAVO.serie || !serieE) return null;
    return { ok: g.ESCLAVO.serie === serieE, nombre: g.via + ' · ' + g.nombre, espera: g.ESCLAVO.serie, es: serieE };
  },

  textoRechazo(r) {
    if (!r) return '';
    const t = Testigo.MOTIVOS[r.desc] || DegFin.MOTIVOS[r.desc] || 'El poste contestó «' + r.desc + '».';
    return /^Cancele antes/.test(r.desc) ? t + ' Cancelar la salida es cosa del técnico: avísele.' : t;
  },

  // ---- Que pantalla toca ----------------------------------------------------
  pantalla() {
    const st = this.c().state;
    if (this._flujo === 'salir') return this._pantallaSalir();
    const g = this._g(), viejo = this._viejo(g);
    if (this._flujo === null) {
      const f = DegFin.leer();
      const finEnCurso = f && (f.estado === 'ENVIADO' ||
                               (f.estado === 'ACEPTADO' && !f.esclavo && Date.now() < f.salidaMs));
      if (g && !viejo && !(g.estado === 'ACEPTADO' && g.esclavo)) this._flujo = 'entrar';
      else if (finEnCurso) { this._flujo = 'salir'; return this._pantallaSalir(); } else return 'p0';
    }
    if (Testigo._espera && Testigo._espera.punta === 'MAESTRO') return 'p4e';
    if (g && g.estado === 'ENVIADO') return 'p4e';
    if (g && g.estado === 'ACEPTADO' && !viejo) {
      if (g.esclavo) return 'p7';
      if (Testigo.vencido(g, new Date(), this.c().horaLocal24)) return 'p5v';
      if (st.node !== 'ESCLAVO') return 'p5';
      const p = this.pareja(g.serie);
      return p && !p.ok ? 'p6x' : 'p6';
    }
    const r = Testigo.ultimoRechazo;
    if (r && r.punta === 'MAESTRO') return 'p4r';
    if (!this._comprobado) return 'p1';
    return st.node === 'MAESTRO' ? 'p3' : 'p2';
  },
  _pantallaSalir() {
    const st = this.c().state, f = DegFin.leer();
    const vivo = f && f.estado === 'ACEPTADO' && Date.now() < f.salidaMs;
    if (vivo && f.esclavo) return 's4';
    if (vivo && st.node === 'ESCLAVO') { const p = this.pareja(f.serie); return p && !p.ok ? 's3x' : 's3'; }
    if (vivo) return 's2';
    if (f && f.estado === 'ENVIADO') return 's1e';
    if (st.node !== 'MAESTRO') return 's0';
    if (st.modo !== 'DEGRADADO') return 's1nada';
    return typeof DegAutoEleccion !== 'undefined' && !DegAutoEleccion.sinRadio(st) ? 's1radio' : 's1';
  },

  // ---- Lo que dice cada pantalla ----------------------------------------------
  _parejaHtml(serieM) {
    const p = this.pareja(serieM);
    if (!p) {
      return '<p><strong>Este teléfono no sabe si este POSTE 2 es la pareja del POSTE 1' +
             (serieM ? ' (serie ' + this.esc(serieM) + ')' : '') + ': compruébelo en persona.</strong></p>';
    }
    if (p.ok) return '<p>Es la pareja del POSTE 1 en «' + this.esc(p.nombre) + '».</p>';
    return '<p><strong>Este POSTE 2 (serie ' + this.esc(p.es) + ') no es la pareja del POSTE 1: en «' +
           this.esc(p.nombre) + '» va el de serie ' + this.esc(p.espera) + '. No se envía nada: conéctese al POSTE 2 ' +
           'correcto.</strong></p><button class="btn-degradado" id="asis-buscar">Buscar el poste</button>';
  },
  html(k) {
    const st = this.c().state, g = this._g() || {}, f = DegFin.leer() || {};
    const tg = Testigo._el || {}, fd = DegFin._el || {};
    const val = (e, d) => this.esc(e && e.value ? e.value : d);
    const boton = (id, txt, cls, apagado) => '<button class="' + (cls || 'btn-degradado') + '" id="' + id + '"' +
      (apagado ? ' disabled' : '') + '>' + txt + '</button>';
    const buscar = boton('asis-buscar', 'Buscar el poste');
    const hora = (id) => '<span class="asis-hora" id="' + id + '">--:--</span>';
    const vivo = '<p id="asis-enviando"></p><p id="asis-rechazo"></p>';
    const T = {
      p0: '<h3>Radio caída entre los dos postes</h3><p>Este asistente pone los dos semáforos a funcionar por reloj, ' +
        'sin radio. Son dos visitas: primero el POSTE 1 y después el POSTE 2.</p>' + boton('asis-empezar', 'Empezar') +
        boton('asis-ir-salir', 'Terminar el Degradado', 'btn-secondary-bt'),
      p1: '<h3>Antes de empezar</h3><p>Compruebe en persona:</p><ul><li>Está delante del POSTE 1.</li><li>Los dos ' +
        'postes no se oyen: la radio entre ellos está caída.</li><li>Hay una persona con paleta en cada extremo ' +
        'de la obra.</li></ul><label><input type="checkbox" id="asis-chk1"> Lo he comprobado</label>' +
        boton('asis-seguir1', 'Seguir', '', true),
      p2: '<h3>Conéctese al POSTE 1</h3><p>' + (st.node === 'ESCLAVO'
        ? 'Está conectado al POSTE 2. Este paso es en el POSTE 1.'
        : 'Pulse para buscar el POSTE 1 y elíjalo en la lista.') + '</p>' + (this.practica()
        ? '<p>En la práctica elija el POSTE 1 de Km 120 (SEM-C12001-M): sus dos postes están sin radio.</p>'
        : '') + buscar,
      p3: '<h3>¿Cuánto tarda en llegar al POSTE 2?</h3><label for="asis-traslado">Minutos hasta el POSTE 2</label>' +
        '<input type="number" id="asis-traslado" class="form-input" min="1" max="720" value="' +
        val(tg.traslado, Testigo.TRASLADO_DEF_MIN) + '"><p>Los dos quedarán en ROJO hasta las ' + hora('asis-hora-p3') +
        ' y después alternarán solos.</p><details><summary>Tiempos (no hace falta tocarlos)</summary>' +
        '<label for="asis-verde">Verde de cada lado (minutos, 3 a 15)</label><input type="number" id="asis-verde" ' +
        'class="form-input" min="3" max="15" value="' + val(tg.verde, Testigo.VERDE_DEF_MIN) + '"><label ' +
        'for="asis-despeje">Rojo para vaciar la obra (segundos, 30 a 255)</label><input type="number" ' +
        'id="asis-despeje" class="form-input" min="30" max="255" value="' + val(tg.despeje, Testigo.DESPEJE_MIN) +
        '"></details>' + boton('asis-poste1', 'Poner el POSTE 1 en rojo y por reloj') +
        '<p>Le pedirá el número del equipo (PIN).</p>',
      p4e: st.node === 'ESCLAVO' ? '<h3>POSTE 1</h3><p>El POSTE 1 no confirmó la orden. Vuelva al POSTE 1, ' +
        'conéctese y pulse Volver a enviar.</p>' : '<h3>POSTE 1</h3><p>Enviando al POSTE 1… espere su ' +
        'respuesta.</p>' + (g.estado === 'ENVIADO'
        ? boton('asis-reenviar', 'Volver a enviar', 'btn-secondary-bt') +
          '<p>Se reenvía lo mismo: no cambia la hora.</p>'
        : ''),
      p4r: '<h3>POSTE 1</h3><p><strong>El POSTE 1 no lo aceptó:</strong> ' +
        this.esc(this.textoRechazo(Testigo.ultimoRechazo)) + '</p>' + boton('asis-reintentar', 'Volver a intentar'),
      p5: '<h3>Ahora vaya al POSTE 2</h3><p>POSTE 1 listo. Tiene hasta las <span class="asis-hora" id="asis-inicio">' +
        '</span> para llegar al POSTE 2 (quedan <span id="asis-quedan"></span>).</p><p>Si cierra la app no se ' +
        'pierde: al volver ponga la clave y siga.</p>' + boton('asis-conectar2', 'Ya estoy en el POSTE 2: conectar'),
      p5v: '<h3>Se pasó la hora</h3><p>El POSTE 2 no recibió el testigo a tiempo. El POSTE 1 alterna por su reloj ' +
        'desde las ' + this.esc(g.inicio || '--:--:--') + ' y el POSTE 2 no: mantenga los paleteros en los dos ' +
        'extremos y vuelva a empezar en el POSTE 1.</p>' + boton('asis-rehacer', 'Volver a empezar'),
      p6: '<h3>POSTE 2</h3><p>Se usa la misma hora y los mismos tiempos del POSTE 1 (empiezan a las ' +
        this.esc(g.inicio || '--:--:--') + '). No hay que escribir nada.</p>' + this._parejaHtml(g.serie) + vivo +
        '<p>Compruebe en persona:</p><ul><li>Está delante del POSTE 2 de este mismo cruce.</li><li>Hay una persona ' +
        'con paleta en cada extremo de la obra.</li></ul><label><input type="checkbox" id="asis-chk6"> Lo he ' +
        'comprobado</label>' + boton('asis-poste2', 'Poner el POSTE 2 en rojo y por reloj', '', true) +
        '<p>Le pedirá el PIN otra vez.</p>',
      p7: '<h3>Listo</h3><p>Los dos postes están en rojo y empezarán a alternar a las ' + this.esc(g.inicio || '') +
        '. Quédese a ver una vuelta completa y compruebe con sus ojos que nunca están los dos en verde. Los ' +
        'paleteros siguen hasta entonces.</p>' + boton('asis-a-salida', 'Programar ya la hora de salida') +
        boton('asis-terminado', 'Terminado', 'btn-secondary-bt'),
      s0: '<h3>Terminar el Degradado</h3><p>' + (st.node === 'ESCLAVO' ? 'Está en el POSTE 2: la salida se programa ' +
        'primero en el POSTE 1.' : 'Conéctese al POSTE 1 para programar la salida.') + '</p>' + buscar,
      s1radio: '<h3>Terminar el Degradado</h3><p>La radio volvió. Para terminar, en Tráfico pulse VOLVER AL MENÚ ' +
        'en el POSTE 1.</p>' + boton('asis-terminado', 'Ver el semáforo', 'btn-secondary-bt'),
      s1nada: '<h3>Terminar el Degradado</h3><p>Este poste no está en Degradado: no hay nada que terminar.</p>' +
        boton('asis-terminado', 'Ver el semáforo', 'btn-secondary-bt'),
      s1: '<h3>¿Cuándo termina el Degradado?</h3><label for="asis-fin-traslado">Minutos hasta el POSTE 2</label>' +
        '<input type="number" id="asis-fin-traslado" class="form-input" min="1" max="720" value="' +
        val(fd.traslado, DegFin.TRASLADO_DEF_MIN) + '"><p>Saldrán a las ' + hora('asis-hora-s1') + '.</p><details>' +
        '<summary>Prefiero poner la hora</summary><label for="asis-fin-hora">Hora de salida (HH:MM)</label><input ' +
        'type="text" id="asis-fin-hora" class="form-input" placeholder="HH:MM"></details><p>A esa hora cada poste se ' +
        'pone en rojo y después queda en ámbar (amarillo intermitente) hasta que vuelva la radio o el técnico lo ' +
        'ponga en marcha.</p>' + vivo + boton('asis-fin1', 'Programar la salida en el POSTE 1') +
        '<p>Le pedirá el PIN.</p>',
      s1e: '<h3>Terminar el Degradado</h3><p>Enviando al POSTE 1… espere su respuesta.</p>' + boton('asis-fin-otra',
        'No contesta: programar otra vez', 'btn-secondary-bt') + '<p>Vuelve a la hora de salida; no cambia nada.</p>',
      s2: '<h3>Vaya al POSTE 2</h3><p>POSTE 1 programado: sale a las <span class="asis-hora" ' +
        'id="asis-salida"></span>. Vaya al POSTE 2 antes de esa hora.</p>' +
        boton('asis-conectar2', 'Ya estoy en el POSTE 2: conectar'),
      s3: '<h3>POSTE 2</h3><p>La misma salida del POSTE 1, a las ' + this.esc(f.salida || '--:--:--') +
        '. No hay que escribir nada.</p>' + this._parejaHtml(f.serie) + vivo +
        boton('asis-fin2', 'Programar la misma salida en el POSTE 2'),
      s4: '<h3>Listo</h3><p>Los dos saldrán a las ' + this.esc(f.salida || '') +
        '. Esté allí a esa hora para mirar los dos postes.</p>' +
        boton('asis-terminado', 'Terminado', 'btn-secondary-bt')
    };
    T.p6x = '<h3>POSTE 2</h3>' + this._parejaHtml(g.serie);
    T.s3x = '<h3>POSTE 2</h3>' + this._parejaHtml(f.serie);
    return T[k] || '';
  },
  CLAVES: ['p0', 'p1', 'p2', 'p3', 'p4e', 'p4r', 'p5', 'p5v', 'p6', 'p6x', 'p7', 's0', 's1radio', 's1nada', 's1',
           's1e', 's2', 's3', 's3x', 's4'],

  // ---- Pintar -----------------------------------------------------------------
  render() {
    const hoja = this.$('asistente-deg');
    if (!hoja || !this.c()) return;
    const b = document.body.classList;
    if (!b.contains('deg') && !b.contains('admin')) this._abierta = false;
    hoja.hidden = !this._abierta;
    const volver = this.$('asistente-volver');
    if (volver) volver.hidden = !(b.contains('deg') && !this._abierta);
    this._cartel();
    if (!this._abierta) { this._key = null; return; }
    const k = this.pantalla();
    if (k !== this._key) { this._key = k; this.$('asis-cuerpo').innerHTML = this.html(k); this._atar(); }
    this._vivo();
  },

  _vivo() {
    const c = this.c(), ahora = Date.now(), g = this._g(), f = DegFin.leer();
    const set = (id, t) => { const e = this.$(id); if (e && e.textContent !== t) e.textContent = t; };
    if (g && g.inicio) { set('asis-inicio', g.inicio); set('asis-quedan', Testigo._mmss(g.inicioMs - ahora)); }
    if (f && f.salida) set('asis-salida', f.salida);
    const t = this.$('asis-traslado');
    if (t) {
      const n = Number(t.value);
      const ok = Number.isInteger(n) && n >= 1 && n <= 720;
      set('asis-hora-p3', ok ? c.horaLocal24(new Date(ahora + n * 60000)).slice(0, 5) : '--:--');
    }
    const ft = this.$('asis-fin-traslado');
    if (ft) {
      const o = DegFin.componer(new Date(ahora), ft.value, (this.$('asis-fin-hora') || {}).value, c.horaLocal24);
      set('asis-hora-s1', o.error ? '--:--' : o.salida.slice(0, 5));
    }
    const esE = this._key === 'p6' || this._key === 's3', r = this._key && this._key[0] === 's' ? DegFin.ultimoRechazo
      : Testigo.ultimoRechazo;
    const suyo = r && r.punta === (esE ? 'ESCLAVO' : 'MAESTRO');
    set('asis-rechazo', suyo ? 'El poste no lo aceptó: ' + this.textoRechazo(r) : '');
    set('asis-enviando', Testigo._espera ? 'Enviando… espere la respuesta del poste.' : '');
  },

  // Mientras un poste lleva el testigo (o la salida) y el otro no, se dice fuera del asistente, en los tres modos.
  textoCartel() {
    const g = Testigo.leer(), f = DegFin.leer(), ahora = Date.now();
    if (g && g.estado === 'ACEPTADO' && !g.esclavo && !this._viejo(g)) {
      return ahora < g.inicioMs
        ? 'El POSTE 1 ya tiene el testigo y el POSTE 2 todavía no: llévelo antes de las ' + g.inicio + '. Mantenga ' +
          'los paleteros en los dos extremos.'
        : 'El POSTE 1 alterna por su reloj desde las ' + g.inicio + ' y el POSTE 2 no recibió el testigo a tiempo. ' +
          'Mantenga los paleteros en los dos extremos y repita desde el POSTE 1.';
    }
    if (f && f.estado === 'ACEPTADO' && !f.esclavo && ahora < f.salidaMs) {
      return 'Salida programada en el POSTE 1 a las ' + f.salida + ' y el POSTE 2 todavía no la tiene: prográmela ' +
             'allí antes de esa hora. Mantenga los paleteros en los dos extremos.';
    }
    return '';
  },
  _cartel() {
    const el = this.$('cartel-testigo'), t = this.textoCartel();
    if (!el) return;
    el.hidden = !t;
    const tx = this.$('cartel-testigo-txt');
    if (tx && tx.textContent !== t) tx.textContent = t;
  },

  // ---- Lo que hace cada boton -------------------------------------------------
  _atar() {
    const on = (id, fn) => {
      const e = this.$(id);
      if (e) e.addEventListener('click', () => { fn(); this.render(); });
    };
    const casilla = (cid, bid) => {
      const c = this.$(cid), b = this.$(bid);
      if (c && b) c.addEventListener('change', () => { b.disabled = !c.checked; });
    };
    const tg = Testigo._el || {}, fd = DegFin._el || {};
    const poner = (e, v) => { if (e) e.value = v; };
    const marcar = () => { if (tg.chk) tg.chk.checked = true; };
    const conectar = () => { const b = this.$('btnDevice'); if (b) b.click(); };
    casilla('asis-chk1', 'asis-seguir1');
    casilla('asis-chk6', 'asis-poste2');
    on('asis-empezar', () => { const g = Testigo.leer(); this._rehacerDe = g ? g.emitidoMs : null;
      this._flujo = 'entrar'; this._comprobado = false; });
    on('asis-ir-salir', () => { this._flujo = 'salir'; });
    on('asis-a-salida', () => { this._flujo = 'salir'; });
    on('asis-seguir1', () => { this._comprobado = true; });
    on('asis-buscar', conectar);
    on('asis-conectar2', conectar);
    on('asis-poste1', () => {
      poner(tg.traslado, this.$('asis-traslado').value); poner(tg.verde, this.$('asis-verde').value);
      poner(tg.despeje, this.$('asis-despeje').value); marcar(); Testigo._pulsar('MAESTRO');
    });
    on('asis-reenviar', () => { marcar(); Testigo._pulsar('MAESTRO'); });
    on('asis-reintentar', () => { Testigo.ultimoRechazo = null; });
    on('asis-rehacer', () => {
      const g = Testigo.leer(); this._rehacerDe = g ? g.emitidoMs : null; this._comprobado = false;
    });
    on('asis-poste2', () => { marcar(); Testigo._pulsar('ESCLAVO'); });
    on('asis-fin1', () => {
      poner(fd.traslado, this.$('asis-fin-traslado').value); poner(fd.salida, this.$('asis-fin-hora').value);
      DegFin._pulsar('MAESTRO');
    });
    on('asis-fin2', () => { DegFin._pulsar('ESCLAVO'); });
    on('asis-fin-otra', () => { const f = DegFin.leer(); DegFin.guardar(f && f.previa ? f.previa : null); });
    on('asis-terminado', () => this.terminar());
  },

  abrir() { this._abierta = true; this._flujo = null; this._key = null; this.render(); },
  cerrar() { this._abierta = false; this.render(); },
  // Terminado: se cierra y, en el modo Degradado, se vuelve a Operario (como la pastilla).
  terminar() {
    this._abierta = false; this._flujo = null;
    if (document.body.classList.contains('deg')) { const r = this.$('btn-toggle-role'); if (r) r.click(); }
  },

  // La hoja, la barra y el cartel se arman aqui: index.html es de mas de 500 lineas y no crece.
  iniciar() {
    if (this.$('asistente-deg') || !document.body) return;
    const hoja = document.createElement('section');
    hoja.id = 'asistente-deg'; hoja.hidden = true;
    hoja.innerHTML = '<div class="card-header-flex"><strong>Radio caída: Degradado</strong><button ' +
      'class="btn-secondary-bt" id="asis-cerrar">Ver el semáforo</button></div><div id="asis-cuerpo"></div>';
    document.body.appendChild(hoja);
    const ref = this.$('carteles-equipo');
    if (ref && ref.parentNode) {
      ref.insertAdjacentHTML('beforebegin', '<button class="btn-degradado" id="asistente-volver" hidden>Volver al ' +
        'asistente</button><div class="card card-camara-pluma" id="cartel-testigo" hidden><strong ' +
        'class="camara-pluma-titulo" id="cartel-testigo-txt"></strong><button type="button" ' +
        'class="btn-degradado" id="btn-cartel-asistente">Abrir el asistente de radio caída</button></div>');
    }
    const card = this.$('card-testigo');
    if (card) {
      card.insertAdjacentHTML('beforeend', '<button class="btn-degradado" id="btn-asistente-abrir">Asistente paso a ' +
        'paso</button><button class="btn-secondary-bt" id="btn-clave-deg">Cambiar la clave de Degradado</button>');
    }
    const on = (id, fn) => { const e = this.$(id); if (e) e.addEventListener('click', fn); };
    on('asis-cerrar', () => this.cerrar());
    on('asistente-volver', () => this.abrir());
    on('btn-asistente-abrir', () => this.abrir());
    on('btn-clave-deg', () => ClaveAdmin.pedirNuevaDeg());
    on('btn-cartel-asistente', () => DegAutoEleccion.irATestigo());
    this.$('asis-cuerpo').addEventListener('input', () => this._vivo());
    if (typeof MutationObserver !== 'undefined') {
      const mo = new MutationObserver(() => this.render());
      mo.observe(document.body, { attributes: true, attributeFilter: ['class'] });
    }
  }
};
document.addEventListener('DOMContentLoaded', () => AsistenteDeg.iniciar());

if (typeof module !== 'undefined' && module.exports) {
  module.exports = AsistenteDeg;
}
