// ===== js/clave_admin.js =====
// LAS CLAVES ENSENAN; EL PIN DEL EQUIPO AUTORIZA (SPEC_4 9.2 y 9.16, D-56, D-57, D-177). Deciden el MODO: la de
// tecnico ensena Tiempos, Ajustes y Tramas; la de Degradado (o la de soporte) solo el asistente de radio caida
// (js/asistente_deg.js). No arman state.pinVerificado ni viajan por el cable, y ninguna orden sale sin el PIN del
// equipo (SPEC_4 4). Por eso no son seguridad del equipo: son barrera de uso.
// Se guardan como huella FNV-1a de 32 bits con sal propia, no en claro; la de soporte, que abre el Degradado y deja
// poner una de tecnico nueva si se olvida, solo existe aqui como huella. Con 4 cifras la huella no es secreta.
const ClaveAdmin = {
  ALMACEN: 'iotvial.clave_admin.v1',
  SAL: 'IOTVIAL-SEM2-ADMIN:',
  SAL_SOPORTE: 'IOTVIAL-SEM2-SOPORTE:',
  HUELLA_SOPORTE: 'f1fa993c',
  DE_FABRICA: '1234',
  ALMACEN_DEG: 'iotvial.clave_deg.v1',   // D-177: la del modo Degradado, con su sal
  SAL_DEG: 'IOTVIAL-SEM2-DEG:',
  DE_FABRICA_DEG: '0000',
  FORMA: /^\d{4,8}$/,
  _modo: 'entrar',
  _alEntrar: null,
  _asistente: false,          // tras entrar, abrir el asistente (lo pide el cartel de radio caida)

  fnv1a(texto) {
    let h = 0x811c9dc5;
    const bytes = unescape(encodeURIComponent(texto));
    for (let i = 0; i < bytes.length; i++) {
      h ^= bytes.charCodeAt(i);
      h = Math.imul(h, 0x01000193) >>> 0;
    }
    return h.toString(16).padStart(8, '0');
  },
  huella(clave) { return this.fnv1a(this.SAL + clave); },

  // Sin almacen (navegador privado) vale la de fabrica: la barrera sigue, y cambiarla solo dura la sesion.
  _guardada() {
    try { return localStorage.getItem(this.ALMACEN) || this.huella(this.DE_FABRICA); } catch (e) { return this.huella(this.DE_FABRICA); }
  },
  _guardar(clave) {
    try { localStorage.setItem(this.ALMACEN, this.huella(clave)); } catch (e) { this._sinAlmacen = this.huella(clave); }
  },
  comprobar(clave) { return this.huella(clave) === (this._sinAlmacen || this._guardada()); },
  huellaDeg(clave) { return this.fnv1a(this.SAL_DEG + clave); },
  comprobarDeg(clave) {
    let h = this._sinAlmacenDeg;
    try { h = h || localStorage.getItem(this.ALMACEN_DEG); } catch (e) { /* sin almacen: la de fabrica */ }
    return this.huellaDeg(clave) === (h || this.huellaDeg(this.DE_FABRICA_DEG));
  },
  esSoporte(clave) { return this.fnv1a(this.SAL_SOPORTE + clave) === this.HUELLA_SOPORTE; },
  // El modo que abre una clave. La de tecnico y la de Degradado no pueden coincidir: se rechaza al guardar.
  rolDe(clave) {
    if (this.comprobar(clave)) return 'TECNICO';
    return this.comprobarDeg(clave) || this.esSoporte(clave) ? 'DEGRADADO' : null;
  },

  _el(id) { return document.getElementById(id); },
  TEXTOS: {
    entrar: ['Clave', 'Operario no necesita clave. Ponga la de Degradado o la de técnico. Cada orden sigue ' +
             'pidiendo el PIN del equipo.', 'Entrar'],   // 9.9 (h)
    nuevaDeg: ['Clave de Degradado', 'Clave nueva de Degradado, de 4 a 8 cifras:', 'Guardar'],
    actual: ['Cambiar la clave', 'Clave actual:', 'Seguir'],
    soporte: ['He olvidado la clave', 'Clave de soporte (la da el responsable):', 'Seguir'],
    nueva: ['Clave nueva', 'Clave nueva, de 4 a 8 cifras:', 'Guardar']
  },
  _poner(modo, msg) {
    this._modo = modo;
    const t = this.TEXTOS[modo];
    this._el('clave-titulo').textContent = t[0];
    this._el('clave-texto').textContent = t[1];
    this._el('btn-clave-entrar').textContent = t[2];
    this._el('clave-msg').textContent = msg || '';
    this._el('clave-entrada').value = '';
    const extra = modo === 'entrar';
    this._el('btn-clave-cambiar').hidden = !extra;
    this._el('btn-clave-olvido').hidden = !extra;
  },

  // Abre el dialogo; alEntrar se llama con la clave buena. Sin el dialogo en la pagina no entra nadie.
  pedir(alEntrar) {
    const modal = this._el('clave-modal');
    if (!modal) return;
    this._alEntrar = alEntrar;
    this._asistente = false;
    this._poner('entrar');
    modal.classList.add('active');
  },
  // Solo con la de tecnico puesta (Ajustes): sin pedir la anterior (D-177).
  pedirNuevaDeg() {
    const modal = this._el('clave-modal');
    if (!modal) return;
    this._alEntrar = null;
    this._poner('nuevaDeg');
    modal.classList.add('active');
  },
  cerrar() {
    const modal = this._el('clave-modal');
    if (modal) modal.classList.remove('active');
    this._el('clave-entrada').value = '';
    this._alEntrar = null;
  },

  _aceptar() {
    const v = this._el('clave-entrada').value.trim();
    if (this._modo === 'entrar') {
      const rol = this.rolDe(v);
      if (!rol) { this._poner('entrar', 'Clave incorrecta.'); return; }
      const cb = this._alEntrar, asis = this._asistente;
      this.cerrar();
      if (cb) cb(rol);
      if ((rol === 'DEGRADADO' || asis) && typeof AsistenteDeg !== 'undefined') AsistenteDeg.abrir();
    } else if (this._modo === 'actual') {
      if (this.comprobar(v)) this._poner('nueva'); else this._poner('actual', 'Clave incorrecta.');
    } else if (this._modo === 'soporte') {
      if (this.esSoporte(v)) this._poner('nueva');
      else this._poner('soporte', 'Clave de soporte incorrecta.');
    } else if (this._modo === 'nueva' || this._modo === 'nuevaDeg') {
      const deg = this._modo === 'nuevaDeg';
      if (!this.FORMA.test(v)) { this._poner(this._modo, 'Tiene que ser de 4 a 8 cifras.'); return; }
      if (deg ? this.comprobar(v) : this.comprobarDeg(v)) {
        this._poner(this._modo, 'No puede ser igual a la de ' + (deg ? 'técnico.' : 'Degradado.'));
        return;
      }
      if (!deg) { this._guardar(v); this._poner('entrar', 'Clave cambiada: entre con la nueva.'); return; }
      const h = this.huellaDeg(v);
      try { localStorage.setItem(this.ALMACEN_DEG, h); } catch (e) { this._sinAlmacenDeg = h; }
      this.cerrar();
      if (typeof window !== 'undefined' && window.appAvisar) window.appAvisar('cyan', 'Clave de Degradado cambiada.');
    }
  },

  iniciar() {
    if (!this._el('clave-modal')) return;
    this._el('btn-clave-entrar').addEventListener('click', () => this._aceptar());
    this._el('btn-clave-cancelar').addEventListener('click', () => this.cerrar());
    this._el('clave-cerrar').addEventListener('click', () => this.cerrar());
    this._el('btn-clave-cambiar').addEventListener('click', () => this._poner('actual'));
    this._el('btn-clave-olvido').addEventListener('click', () => this._poner('soporte'));
    this._el('clave-entrada').addEventListener('keydown', (e) => { if (e.key === 'Enter') this._aceptar(); });
  }
};
document.addEventListener('DOMContentLoaded', () => ClaveAdmin.iniciar());
