// ===== js/dialogo.js =====
// LAS PREGUNTAS LAS HACE LA APP, NO EL NAVEGADOR (SPEC_4 9.1). prompt/confirm/alert salen en el idioma del sistema,
// bloquean el hilo -colgo una corrida E2E, N75-4- y en jsdom no se pueden contestar. Este dialogo vive en
// #dialogo-modal de index.html y contesta en el mismo toque: la accion llama a `alAceptar`; Cancelar, la X o tocar
// fuera no llaman a nada. Por eso lo que borra o guarda va DENTRO de alAceptar y nunca detras de la llamada.
// Se cierra ANTES de llamar: crear un cruce abre la segunda pregunta desde la primera.
const Dialogo = {
  _alAceptar: null,
  _alCancelar: null,
  _el(id) { return document.getElementById(id); },

  // op: { titulo, texto, accion, entrada (bool), valor, placeholder }. Sin #dialogo-modal no pregunta ni actua.
  _abrir(op, alAceptar, alCancelar) {
    const modal = this._el('dialogo-modal');
    if (!modal) return;
    this._el('dialogo-titulo').textContent = op.titulo || '';
    this._el('dialogo-texto').textContent = op.texto || '';
    this._el('dialogo-texto').hidden = !op.texto;
    const entrada = this._el('dialogo-entrada');
    entrada.hidden = !op.entrada;
    entrada.value = op.valor || '';
    entrada.placeholder = op.placeholder || '';
    this._el('btn-dialogo-aceptar').textContent = op.accion;
    this._alAceptar = alAceptar;
    this._alCancelar = alCancelar || null;
    modal.classList.add('active');
    if (op.entrada && typeof entrada.focus === 'function') entrada.focus();
  },

  _cerrar(acepta) {
    const modal = this._el('dialogo-modal');
    if (!modal || !modal.classList.contains('active')) return;
    const cb = acepta ? this._alAceptar : this._alCancelar;
    const valor = this._el('dialogo-entrada').value;
    this._alAceptar = this._alCancelar = null;
    modal.classList.remove('active');
    if (cb) cb(valor);
  },

  // Pregunta antes de algo que no se deshace. `accion` es el verbo del boton («Vaciar la cinta»), nunca «OK».
  confirmar(titulo, texto, accion, alAceptar) {
    this._abrir({ titulo, texto, accion }, alAceptar);
  },

  // Pide un texto, en un campo que llega VACIO salvo que se pase `valor` (renombrar trae el nombre actual): un valor
  // propuesto se guarda con un toque sin que nadie lo lea. Vacio no llama a alAceptar, salvo con `opcional`.
  pedirTexto(op, alAceptar, alCancelar) {
    this._abrir(Object.assign({ entrada: true, accion: 'Guardar' }, op), (v) => {
      const t = (v || '').trim();
      if (t || op.opcional) alAceptar(t);
    }, alCancelar);
  },

  iniciar() {
    const modal = this._el('dialogo-modal');
    if (!modal) return;
    this._el('btn-dialogo-aceptar').addEventListener('click', () => this._cerrar(true));
    this._el('btn-dialogo-cancelar').addEventListener('click', () => this._cerrar(false));
    this._el('dialogo-cerrar').addEventListener('click', () => this._cerrar(false));
    modal.addEventListener('click', (e) => { if (e.target === modal) this._cerrar(false); });
    this._el('dialogo-entrada').addEventListener('keydown', (e) => { if (e.key === 'Enter') this._cerrar(true); });
  }
};
document.addEventListener('DOMContentLoaded', () => Dialogo.iniciar());
