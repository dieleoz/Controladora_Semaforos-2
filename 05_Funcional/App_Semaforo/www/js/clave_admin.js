// ===== js/clave_admin.js =====
// LA CLAVE DE ADMINISTRACION ENSENA; EL PIN DEL EQUIPO AUTORIZA (SPEC_4 9.2, D-56, D-57). Esta clave solo decide si
// se ven Tiempos, Diagnostico (Degradado, hora) y Depuracion: no arma state.pinVerificado ni viaja por el cable, y
// ninguna orden sale sin el PIN del equipo (SPEC_4 4). Por eso no es seguridad del equipo: es barrera de uso.
// Se guarda como huella FNV-1a de 32 bits con sal, no en claro; la de soporte, que permite poner una nueva si se
// olvida, solo existe aqui como huella. Con 4 cifras la huella no es secreta para quien lea esto, y no lo pretende.
const ClaveAdmin = {
  ALMACEN: 'iotvial.clave_admin.v1',
  SAL: 'IOTVIAL-SEM2-ADMIN:',
  SAL_SOPORTE: 'IOTVIAL-SEM2-SOPORTE:',
  HUELLA_SOPORTE: 'f1fa993c',
  DE_FABRICA: '1234',
  FORMA: /^\d{4,8}$/,
  _modo: 'entrar',
  _alEntrar: null,

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

  _el(id) { return document.getElementById(id); },
  TEXTOS: {
    entrar: ['Clave de administración', 'Tiempos, Degradado, hora y depuración. Las órdenes piden además el PIN del equipo.', 'Entrar'],
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
    this._poner('entrar');
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
      if (!this.comprobar(v)) { this._poner('entrar', 'Clave incorrecta.'); return; }
      const cb = this._alEntrar;
      this.cerrar();
      if (cb) cb();
    } else if (this._modo === 'actual') {
      if (this.comprobar(v)) this._poner('nueva'); else this._poner('actual', 'Clave incorrecta.');
    } else if (this._modo === 'soporte') {
      if (this.fnv1a(this.SAL_SOPORTE + v) === this.HUELLA_SOPORTE) this._poner('nueva');
      else this._poner('soporte', 'Clave de soporte incorrecta.');
    } else if (this._modo === 'nueva') {
      if (!this.FORMA.test(v)) { this._poner('nueva', 'Tiene que ser de 4 a 8 cifras.'); return; }
      this._guardar(v);
      this._poner('entrar', 'Clave cambiada: entre con la nueva.');
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
