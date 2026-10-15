// ===== js/noche.js =====
// MODO NOCHE en la app (D-106..D-111): SPEC_0 §5.10 (d), (f)-(i) y SPEC_4 §3.1.
//
// Ordenes, solo del Maestro (el Esclavo no guarda franjas, D-107):
//     SET_NOCHE:<n>,<dias>,<HHMM>,<HHMM>,<A|R> y SET_NOCHE:<n>,OFF   con PIN
//     SET_NOCHE_TR:<s>                                               con PIN
//     CONSULTA_NOCHE                                                 sin PIN (SIN_PIN de app.js)
// La salida por la app es SET_MODO:AUTO/INTELIGENTE/MANUAL con PIN: dentro de la noche el
// Maestro contesta SALIENDO_TODO_ROJO y cambia de modo al cumplirse el todo-rojo (D-111).
//
// LO QUE ESTE MODULO NO HACE, A PROPOSITO:
//   - No pinta lo guardado por haber pulsado: lo pinta la respuesta de CONSULTA_NOCHE, que
//     se pide al conectar al Poste 1 (detras de la del automatico, para no cruzar dos
//     ordenes) y tras cada RESULT:OK.
//   - No escribe al cable: todo sale por enviarComandoFirmware(), cuyo bool se mira.
//   - El todo-rojo no baja de la propuesta por distancia ni de 180 s, y si la propuesta pasa
//     de 900 s no se recorta: no se guarda (D-116). El firmware solo conoce su rango (10..900 s),
//     no la distancia, asi que esa barrera es de la app.

const Noche = {
  CONSULTA: 'CONSULTA_NOCHE',
  ORDEN: 'SET_NOCHE',
  ORDEN_TR: 'SET_NOCHE_TR',
  DIAS: 'LMXJVSD',
  FRANJAS: 4,
  TR_MINIMO: 180,   // D-116: el minimo de la app (el fabrica del firmware); el firmware admite desde 10
  TR_TECHO: 900,    // NOCHE_TR_MAX_S (D-109)
  COLOR: { A: 'ámbar en los dos', R: 'POSTE 1 ámbar, POSTE 2 rojo' },
  DESTINO: { AUTO: 'AUTOMÁTICO', INTELIGENTE: 'INTELIGENTE', MANUAL: 'MANUAL' },
  ACUSE_OK: {
    SET_NOCHE: 'Equipo: franja de noche guardada y releída de su memoria. Se vuelve a consultar lo guardado.',
    SET_NOCHE_TR: 'Equipo: todo-rojo de la noche (entrada y salida) guardado. Se vuelve a consultar lo guardado.',
  },
  RECHAZO: {
    FORMATO_INVALIDO: 'el equipo no entendió la orden. No ha cambiado nada: anote el literal y avise.',
    RANGO: 'valor fuera de rango (franja 1 a 4, horas 00:00 a 23:59, todo-rojo 10 a 900 s). No ha cambiado nada.',
    EN_NOCHE_SALGA_PRIMERO: 'esa franja está en curso y no se cambia. Salga primero de la noche (con PIN) y ' +
      'repita. No ha cambiado nada.',
    FLASH_NO_RELEE: 'el equipo no pudo guardarlo en su memoria. No se guardó: repita y, si vuelve, avise a ' +
      'mantenimiento.',
  },

  ctx: null,
  _cfg: null,               // { tr, f: [{dias:[7 bool], ini, fin, color|null}] } de la ultima consulta
  _pendiente: false,        // consultar al conectar, detras de la consulta del automatico
  _consultaEnCurso: false,
  _sinNoche: false,         // el Maestro no la atiende: firmware sin modo noche

  // ---- Puras ---------------------------------------------------------------

  // D-116: distancia / 20 km/h, redondeada ARRIBA a 10 s, nunca bajo 180 y SIN recortar al techo:
  // por encima de 900 la app no guarda (_guardarTr). En enteros: s/10 = m * 3,6 / 20 / 10 =
  // m * 18 / 1000 (20/3,6 en coma flotante da 190 por 180).
  propuesta(metros) {
    const m = Number(metros);
    if (!(m > 0)) return null;
    return Math.max(this.TR_MINIMO, Math.ceil(m * 18 / 1000) * 10);
  },

  // D-116: por que no se guarda una propuesta mayor que el techo, sin jerga.
  _sobreTecho(p) {
    return 'Con esa distancia hacen falta ' + p + ' s con los dos postes en rojo para que el tramo se vacíe, y ' +
      'el equipo admite como mucho ' + this.TR_TECHO + ' s (15 min). No se puede guardar: revise la distancia.';
  },

  // 'HH:MM' -> 'HHMM', o null
  _hhmm(t) {
    const m = /^(\d{2}):(\d{2})$/.exec(String(t || ''));
    return m && +m[1] <= 23 && +m[2] <= 59 ? m[1] + m[2] : null;
  },

  // Los argumentos de SET_NOCHE, o { error } con lo que falta. inicio == fin es «apagada».
  armarFranja(n, dias, ini, fin, color) {
    const i = this._hhmm(ini), f = this._hhmm(fin);
    if (!dias.some(Boolean)) return { error: 'marque al menos un día, o use Apagar' };
    if (!i || !f) return { error: 'ponga la hora de inicio y la de fin' };
    if (i === f) return { error: 'inicio y fin iguales es una franja apagada: use Apagar' };
    if (color !== 'A' && color !== 'R') return { error: 'elija el color' };
    const d = dias.map((x, k) => (x ? this.DIAS[k] : '-')).join('');
    return { args: n + ',' + d + ',' + i + ',' + f + ',' + color };
  },

  // $ACK,CMD:NOCHE,RESULT:OK,TR:<s>,F1:<dias>-<HHMM>-<HHMM>-<A|R|OFF>,...; otra forma, null.
  leerConsulta(data) {
    const tr = Number(data && data.TR);
    if (!data || data.RESULT !== 'OK' || !(tr > 0)) return null;
    const f = [];
    for (let n = 1; n <= this.FRANJAS; n++) {
      const m = /^([LMXJVSD-]{7})-(\d{2})(\d{2})-(\d{2})(\d{2})-(A|R|OFF)$/.exec(String(data['F' + n] || ''));
      if (!m) return null;
      f.push({ dias: m[1].split('').map(c => c !== '-'), ini: m[2] + ':' + m[3], fin: m[4] + ':' + m[5],
               color: m[6] === 'OFF' ? null : m[6] });
    }
    return { tr, f };
  },

  _franjaTexto(fr) {
    if (!fr.color) return 'apagada';
    const d = fr.dias.map((x, k) => (x ? this.DIAS[k] : '')).filter(Boolean).join(' ');
    return d + ' · ' + fr.ini + ' a ' + fr.fin + ' · ' + this.COLOR[fr.color];
  },

  _tr() { return this._cfg ? 'de ' + this._cfg.tr + ' s' : 'programado'; },

  // ---- Enganches de app.js ---------------------------------------------------

  alStatus(node) {
    if (this.ctx && node && node !== this.ctx.state.node) {
      this._cfg = null; this._sinNoche = false; this._pendiente = node === 'MAESTRO';
    }
    setTimeout(() => this.render(), 0);   // app.js guarda NODE y MODO justo despues
  },

  // Todo $ACK, antes de traducirlo: la consulta de noche sale detras de la del automatico.
  anotarAcuse(data) {
    if (!this.ctx || !data || data.CMD !== 'CONSULTA_DEG_AUTO' || !this._pendiente) return;
    this._pendiente = false;
    this._consultar();
  },

  acuse(data) {
    if (!this.ctx || !data) return null;
    const cmd = String(data.CMD || ''), res = String(data.RESULT || '');
    if (cmd === 'NOCHE') {
      this._consultaEnCurso = false;
      this._cfg = this.leerConsulta(data);
      this._rellenar();
      this.render();
      return this._cfg ? { tono: 'cyan', toast: 'Modo noche consultado',
        texto: 'Modo noche guardado en el POSTE 1: todo-rojo ' + this._cfg.tr + ' s; ' +
               this._cfg.f.map((fr, k) => 'franja ' + (k + 1) + ' ' + this._franjaTexto(fr)).join('; ') + '.' } : null;
    }
    if (this.ACUSE_OK[cmd] && res === 'OK') {
      this._consultar();
      return { tono: 'green', toast: 'Modo noche: guardado', texto: this.ACUSE_OK[cmd] };
    }
    const sal = /^SET_MODO:(AUTO|INTELIGENTE|MANUAL)$/.exec(cmd);
    if (sal && res === 'SALIENDO_TODO_ROJO') {
      return { tono: 'cyan', toast: 'Saliendo de la noche: todo-rojo',
        texto: 'Saliendo del modo noche: rojo en los dos postes, todo-rojo ' + this._tr() + ' y después ' +
               this.DESTINO[sal[1]] + '. No mande otra salida mientras tanto.' };
    }
    return null;
  },

  rechazo(data) {
    if (!this.ctx || !data) return null;
    const cmd = String(data.CMD || ''), desc = String(data.DESC || '');
    if (this._consultaEnCurso && (cmd === this.CONSULTA ||
        ((cmd === 'DESCONOCIDO' || cmd === 'AUTH_FAILED') && data.NODE !== 'PUENTE'))) {
      this._consultaEnCurso = false; this._sinNoche = true; this.render();
      return { texto: 'Este firmware no lleva el modo noche.', toast: 'Modo noche: no está en este firmware' };
    }
    if ((cmd === this.ORDEN || cmd === this.ORDEN_TR) && this.RECHAZO[desc]) {
      return { texto: 'Modo noche: ' + this.RECHAZO[desc], toast: 'Modo noche: ' + desc };
    }
    if (/^SET_MODO:/.test(cmd) && desc === 'EN_NOCHE_SALGA_PRIMERO') {   // DEG_T y AMBAR, SPEC_0 5.10 (f)
      return { texto: 'Modo noche: esta orden no se hace dentro de la noche. Salga primero (con PIN) a ' +
               'AUTOMÁTICO, INTELIGENTE o MANUAL, o pase a MENU, y repita.', toast: 'En modo noche: salga primero' };
    }
    if (/^SET_MODO:/.test(cmd) && desc === 'SALIDA_EN_CURSO') {
      return { texto: 'Modo noche: ya hay una salida en curso (todo-rojo en los dos postes). Espere a que ' +
               'termine; esta segunda orden no se hace.', toast: 'Salida de la noche ya en curso' };
    }
    return null;
  },

  // $EVENT ORIGEN:NOCHE (SPEC_0 §5.10 (g)): el aviso al entrar y al salir.
  evento(data) {
    if (!data || data.ORIGEN !== 'NOCHE') return null;
    const t = {
      ENTRADA: ['Modo noche: empieza la franja', 'Modo noche: empieza la franja. Primero todo-rojo ' + this._tr() +
        ' en los dos postes y después destello, con la pluma arriba en los dos.'],
      SALIDA_HORA: ['Modo noche: acaba la franja', 'Modo noche: acaba la franja. Todo-rojo ' + this._tr() +
        ' en los dos postes y después vuelve al modo de antes de la noche.'],
      SALIDA_APP: ['Modo noche: salida desde la app', 'Modo noche: salida pedida desde la app. Rojo en los dos ' +
        'postes; a AUTO, INTELIGENTE o MANUAL tras el todo-rojo, a MENU, ALCANCE o rojo total al momento.'],
      FRANJA_SALTADA: ['Modo noche: franja saltada', 'Modo noche: franja saltada. El POSTE 1 no estaba en AUTO ni ' +
        'INTELIGENTE al empezar; no entra hasta la próxima franja.'],
    }[String(data.DETALLE || '')];
    return t ? { tono: 'cyan', toast: t[0], texto: t[1] } : null;
  },

  alarma(data) {
    if (!data || data.EVENTO !== 'NOCHE' || data.CAUSA !== 'HORA_NO_FIABLE') return null;
    return { tono: 'red', toast: 'Noche sin hora fiable: salga por la app',
      texto: 'Modo noche: el POSTE 1 perdió la hora fiable y sigue en noche (la luz no depende del reloj). No ' +
             'saldrá solo por hora: salga desde la app, con PIN, y ponga la hora.' };
  },

  // ---- Pantalla ------------------------------------------------------------

  iniciar(ctx) {
    this.ctx = ctx;
    const caja = document.getElementById('noche-franjas'), sal = document.getElementById('noche-salida');
    if (!caja) return;
    let h = '';
    for (let n = 1; n <= this.FRANJAS; n++) {
      const p = 'noche-f' + n;
      h += '<div class="form-group"><label>Franja ' + n + ' · <span id="' + p + '-guardada">sin consultar</span>' +
        '</label>' +
        '<div>' + this.DIAS.split('').map((c, k) => '<label><input type="checkbox" id="' + p + '-d' + k + '"> ' + c +
        '</label> ').join('') + '</div>' +
        '<input type="time" id="' + p + '-ini" class="form-input"> a ' +
        '<input type="time" id="' + p + '-fin" class="form-input">' +
        '<select id="' + p + '-color" class="form-input"><option value="A">' + this.COLOR.A + '</option>' +
        '<option value="R">' + this.COLOR.R + '</option></select>' +
        '<button type="button" class="btn-save-site" id="btn-' + p + '-guardar">Guardar franja ' + n + '</button> ' +
        '<button type="button" class="btn-secondary-bt" id="btn-' + p + '-apagar">Apagar</button></div>';
    }
    caja.innerHTML = h;
    for (let n = 1; n <= this.FRANJAS; n++) {
      document.getElementById('btn-noche-f' + n + '-guardar').addEventListener('click', () => this._guardar(n));
      document.getElementById('btn-noche-f' + n + '-apagar').addEventListener('click', () => this._apagar(n));
    }
    if (sal) {
      sal.innerHTML = '<p class="card-sub"><strong>El POSTE 1 está en modo noche.</strong> Salir (con PIN): ' +
        'primero todo-rojo en los dos postes y después el modo elegido.</p>' +
        Object.keys(this.DESTINO).map(d => '<button type="button" class="btn-secondary-bt" id="btn-noche-salir-' +
          d.toLowerCase() + '">Salir a ' + this.DESTINO[d] + '</button> ').join('');
      Object.keys(this.DESTINO).forEach(d => document.getElementById('btn-noche-salir-' + d.toLowerCase())
        .addEventListener('click', () => this._salir(d)));
    }
    const dist = document.getElementById('num-noche-distancia'), btr = document.getElementById('btn-noche-tr');
    if (dist) dist.addEventListener('input', () => this._proponer());
    if (btr) btr.addEventListener('click', () => this._guardarTr());
    this.render();
    setInterval(() => this.render(), 1000);   // la desconexion no avisa a este modulo
  },

  // D-109: la propuesta se ensena y sube el campo si estaba por debajo; nunca lo baja.
  _proponer() {
    const p = this.propuesta(document.getElementById('num-noche-distancia').value);
    const tr = document.getElementById('num-noche-tr'), txt = document.getElementById('noche-tr-propuesta');
    const tope = p === this.TR_TECHO ? ', tope del equipo' : '';
    if (txt) txt.textContent = !p ? 'Ponga la distancia entre postes: la app propone el todo-rojo con ella.'
      : p > this.TR_TECHO ? this._sobreTecho(p)
      : 'Propuesta: ' + p + ' s (a 20 km/h' + tope + '). Puede subirlo, no bajarlo.';
    if (p && p <= this.TR_TECHO && tr) { tr.min = String(p); if (!(Number(tr.value) >= p)) tr.value = String(p); }
  },

  _rellenar() {
    const c = this._cfg;
    if (!c) return;
    const tr = document.getElementById('num-noche-tr');
    if (tr) tr.value = String(c.tr);
    c.f.forEach((fr, k) => {
      const p = 'noche-f' + (k + 1), el = (s) => document.getElementById(p + s);
      if (!el('-ini')) return;
      fr.dias.forEach((x, i) => { el('-d' + i).checked = x; });
      el('-ini').value = fr.ini; el('-fin').value = fr.fin; el('-color').value = fr.color || 'A';
    });
  },

  render() {
    if (!this.ctx) return;
    const st = this.ctx.state, $ = (id) => document.getElementById(id);
    const est = $('noche-estado'), sal = $('noche-salida');
    if (est) {
      est.textContent = !st.node ? 'Sin conectar.'
        : st.node !== 'MAESTRO' ? 'Las franjas se programan conectado al POSTE 1 (Maestro): este poste solo obedece.'
        : this._sinNoche ? 'Este firmware no lleva el modo noche.'
        : this._cfg ? 'Guardado en el POSTE 1: todo-rojo ' + this._cfg.tr + ' s.' : 'Consultando lo guardado...';
    }
    for (let n = 1; n <= this.FRANJAS; n++) {
      const g = $('noche-f' + n + '-guardada');
      if (g) g.textContent = this._cfg && st.node === 'MAESTRO'
        ? 'en el equipo: ' + this._franjaTexto(this._cfg.f[n - 1]) : 'sin consultar';
    }
    if (sal) sal.hidden = !(st.node === 'MAESTRO' && st.modo === 'NOCHE');
  },

  _consultar() {
    if (this.ctx.enviarComandoFirmware(this.CONSULTA)) this._consultaEnCurso = true;
    this.render();
  },

  // Comun a las ordenes de programar: solo en el Maestro, y con PIN.
  _puerta(reintento) {
    const c = this.ctx;
    if (c.state.node !== 'MAESTRO') {
      c.showToast('El modo noche se programa en el POSTE 1'); this.render(); return false;
    }
    if (!c.state.pinVerificado) { c.pedirPin(reintento); return false; }
    return true;
  },

  _guardar(n) {
    const c = this.ctx, p = 'noche-f' + n, el = (s) => document.getElementById(p + s);
    const r = this.armarFranja(n, [0, 1, 2, 3, 4, 5, 6].map(i => el('-d' + i).checked),
                               el('-ini').value, el('-fin').value, el('-color').value);
    if (r.error) { c.showToast('Franja ' + n + ': ' + r.error); return; }
    if (!this._puerta(() => this._guardar(n))) return;
    if (c.enviarComandoFirmware(this.ORDEN, r.args)) {
      c.addEvent('cyan', 'Técnico: franja de noche ' + n + ' enviada (' + r.args + '). Espere el acuse.');
    }
  },

  _apagar(n) {
    if (!this._puerta(() => this._apagar(n))) return;
    const args = n + ',OFF';
    if (this.ctx.enviarComandoFirmware(this.ORDEN, args)) {
      this.ctx.addEvent('cyan', 'Técnico: apagar la franja de noche ' + n + ' enviada. Espere el acuse.');
    }
  },

  _guardarTr() {
    const c = this.ctx, p = this.propuesta(document.getElementById('num-noche-distancia').value);
    const s = Number(document.getElementById('num-noche-tr').value);
    if (!p) { c.showToast('Ponga primero la distancia entre postes'); this._proponer(); return; }
    if (p > this.TR_TECHO) { c.showToast(this._sobreTecho(p)); this._proponer(); return; }
    if (!Number.isInteger(s) || s < p || s > this.TR_TECHO) {
      c.showToast('Todo-rojo entre ' + p + ' y ' + this.TR_TECHO + ' s: no baja de la propuesta');
      return;
    }
    if (!this._puerta(() => this._guardarTr())) return;
    const args = String(s);
    if (c.enviarComandoFirmware(this.ORDEN_TR, args)) {
      c.addEvent('cyan', 'Técnico: todo-rojo de la noche ' + s + ' s enviado. Espere el acuse.');
    }
  },

  // D-111: la salida por la app, con PIN. Tres llamadas literales: el censo de app_01 lee la orden entera.
  _salir(d) {
    const c = this.ctx;
    if (!c.state.pinVerificado) { c.pedirPin(() => this._salir(d)); return; }
    const ok = d === 'AUTO' ? c.enviarComandoFirmware('SET_MODO', 'AUTO')
      : d === 'INTELIGENTE' ? c.enviarComandoFirmware('SET_MODO', 'INTELIGENTE')
      : c.enviarComandoFirmware('SET_MODO', 'MANUAL');
    if (ok) c.addEvent('cyan', 'Técnico: salida de la noche a ' + this.DESTINO[d] + ' enviada. Espere el acuse.');
  },
};

if (typeof module !== 'undefined' && module.exports) {
  module.exports = Noche;
}
