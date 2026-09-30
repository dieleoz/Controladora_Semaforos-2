// ===== js/aviso_degradado.js =====
// AVISO DE PALETEROS (decision del responsable, 29/09).
//
// Con el poste conectado en Degradado ($STATUS MODO:DEGRADADO, manual o automatico) y
// sin radio, cambiar el modo de UN poste es peligroso: el otro no se entera y sigue
// dando verdes por su reloj; si este da paso, hay verde contra verde o contra ambar.
// La regla: ese cambio se hace con PALETEROS en los dos extremos.
//
// Se engancha en UN sitio, enviarComandoFirmware() de app.js, para que ningun boton
// quede fuera. Como esa funcion no sabe que boton la llamo, este modulo apunta el boton
// del click en curso (escucha en captura) y, al confirmar, lo vuelve a pulsar: el mismo
// patron que pedirPin() y confirmarVia(), que re-evaluan todas las barreras de delante.
// Confirmar deja un vale de un solo uso para ESA orden, que caduca a los 30 s.
//
// Modal propio del DOM, nunca confirm(): el nativo bloquea el hilo (N75-4).

const AvisoDegradado = {
  VIGENCIA_MS: 30000,
  _vale: null,        // { orden, ms }: la orden ya confirmada, un solo uso
  _pendiente: null,   // { orden, boton, addEvent }: lo que espera respuesta
  _boton: null,       // boton del click que se esta atendiendo ahora mismo
  _el: null,

  // Las ordenes que cambian el modo de ESTE poste.
  aplica(orden, node) {
    if (/^SET_MODO:/.test(orden) || orden === 'SET_DEG_AUTO:0') return true;
    return node === 'ESCLAVO' && (orden === 'AMBAR_EMERGENCIA' || orden === 'CANCELAR_AMBAR');
  },

  // true: se puede enviar ya. false: se abrio el aviso (o no hay DOM) y NO sale nada.
  permite(orden, state, addEvent) {
    if (!state || state.modo !== 'DEGRADADO' || !this.aplica(orden, state.node)) return true;
    const v = this._vale;
    if (v && v.orden === orden && Date.now() - v.ms <= this.VIGENCIA_MS) {
      this._vale = null;
      return true;
    }
    const el = this._elementos();
    if (!el) return false;
    this._pendiente = { orden, boton: this._boton, addEvent };
    el.chk.checked = false;
    el.ok.disabled = true;
    if (el.orden) el.orden.textContent = 'Orden: ' + orden + '.';
    el.modal.classList.add('active');
    return false;
  },

  _elementos() {
    if (this._el) return this._el;
    if (typeof document === 'undefined') return null;
    const $ = id => document.getElementById(id);
    const el = { modal: $('aviso-deg-modal'), chk: $('chk-aviso-deg'), ok: $('btn-aviso-deg-confirmar'),
                 no: $('btn-aviso-deg-cancelar'), x: $('modal-aviso-deg-close'), orden: $('aviso-deg-orden') };
    if (!el.modal || !el.chk || !el.ok) return null;
    el.chk.addEventListener('change', () => { el.ok.disabled = !el.chk.checked; });
    el.ok.addEventListener('click', () => this._confirmar());
    [el.no, el.x].forEach(b => b && b.addEventListener('click', () => this._cancelar()));
    el.modal.addEventListener('click', e => { if (e.target === el.modal) this._cancelar(); });
    this._el = el;
    return el;
  },

  _cerrar() {
    const p = this._pendiente;
    this._pendiente = null;
    if (this._el) this._el.modal.classList.remove('active');
    return p;
  },

  _confirmar() {
    if (!this._el || !this._el.chk.checked) return;
    const p = this._cerrar();
    if (!p) return;
    this._vale = { orden: p.orden, ms: Date.now() };
    if (p.boton && typeof p.boton.click === 'function') p.boton.click();
    else if (p.addEvent) p.addEvent('cyan', 'Aviso de degradado confirmado: pulse otra vez ' + p.orden + '.');
  },

  _cancelar() {
    const p = this._cerrar();
    if (p && p.addEvent) {
      p.addEvent('cyan', 'Orden ' + p.orden + ' cancelada en el aviso de degradado: no se envio nada.');
    }
  },
};

if (typeof document !== 'undefined') {
  document.addEventListener('click', e => {
    const b = e.target && e.target.closest ? e.target.closest('button') : null;
    if (!b || b.id === 'btn-aviso-deg-confirmar') return;
    AvisoDegradado._boton = b;
    setTimeout(() => { if (AvisoDegradado._boton === b) AvisoDegradado._boton = null; }, 0);
  }, true);
}

if (typeof module !== 'undefined' && module.exports) {
  module.exports = AvisoDegradado;
}
