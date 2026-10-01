// ===== js/carteles_equipo.js =====
// CARTELES FIJOS QUE ABRE UN $EVENT DEL FIRMWARE, Y QUE EL FIRMWARE REPITE.
//
// Un $EVENT que el firmware repite cada poco (hoy: ENLACE_DISPONIBLE cada 60 s mientras
// el poste en Degradado oye al otro) no puede ser un toast y una linea cada vez: llena la
// bitacora y el operario deja de leerla. Va a un CARTEL fijo con la hora del ULTIMO aviso;
// toast y linea solo la PRIMERA vez por conexion. Molde: js/aviso_corte.js.
//
// UNA ENTRADA MAS = una fila en ENTRADAS, con la clave 'ORIGEN|DETALLE' de la trama:
//   titulo, accion   texto del cartel (la hora del ultimo aviso la pone cartel())
//   toast, tono      lo que sale la primera vez por conexion (toast y linea de Eventos)
//   modo             el cartel se quita cuando un $STATUS trae MODO distinto de este;
//                    null = solo lo quita Entendido o soltar el enlace
// El HTML no cambia: pintar() crea un recuadro por cartel abierto dentro de
// #carteles-equipo, con su boton Entendido (id btn-<id>-ok).

const CartelesEquipo = {
  ENTRADAS: {
    'DEGRADADO|ENLACE_DISPONIBLE': {
      id: 'aviso-radio-deg', modo: 'DEGRADADO', tono: 'red',
      toast: 'LA RADIO VOLVIO - siguen en Degradado',
      titulo: 'LA RADIO VOLVIO entre los dos postes. Siguen en Degradado.',
      accion: 'Para volver al ciclo: VOLVER AL MENU en el POSTE 1 (el Poste 2 lo sigue ' +
              'solo por radio) y despues el modo que quiera.'
    }
  },

  _abiertos: {},   // clave -> { hora, poste, cerrado }

  _clave(data) {
    return data ? (data.ORIGEN || '') + '|' + (data.DETALLE || '') : '';
  },

  // null si la trama no abre cartel; {repetido:true} si ya se aviso en esta conexion
  // (o el operario lo cerro); si no, {tono, texto, toast}.
  ver(data) {
    const k = this._clave(data);
    const e = this.ENTRADAS[k];
    if (!e) return null;
    const ya = this._abiertos[k];
    if (ya) {
      ya.hora = data.HORA || ya.hora;
      return { repetido: true };
    }
    this._abiertos[k] = { hora: data.HORA || null, poste: data.NODE || null, cerrado: false };
    return { tono: e.tono, toast: e.toast, texto: e.titulo + ' ' + e.accion };
  },

  // Cada $STATUS: el cartel cuyo modo ya no es el del poste se quita, y el siguiente
  // aviso cuenta como episodio nuevo (otra vez toast y linea).
  alStatus(modo) {
    if (modo === undefined) return;
    Object.keys(this._abiertos).forEach(k => {
      const e = this.ENTRADAS[k];
      if (e.modo && modo !== e.modo) delete this._abiertos[k];
    });
  },

  // Los carteles a ensenar: [{id, titulo, accion, cuando}].
  carteles() {
    return Object.keys(this._abiertos).filter(k => !this._abiertos[k].cerrado).map(k => {
      const e = this.ENTRADAS[k];
      const a = this._abiertos[k];
      return { id: e.id, titulo: e.titulo, accion: e.accion,
               cuando: 'Ultimo aviso del equipo' + (a.poste ? ' (' + a.poste + ')' : '') + ': ' +
                       (a.hora || 'la trama no trae hora') + '.' };
    });
  },

  // Entendido: cerrado hasta soltar el enlace; las repeticiones no lo reabren.
  cerrar(id) {
    Object.keys(this._abiertos).forEach(k => {
      if (this.ENTRADAS[k].id === id) this._abiertos[k].cerrado = true;
    });
  },

  olvidar() { this._abiertos = {}; },

  // Rehace el contenido de cont (#carteles-equipo). alCerrar: lo que hace app.js tras
  // Entendido (repintar).
  pintar(cont, alCerrar) {
    if (!cont) return;
    const doc = cont.ownerDocument;
    while (cont.firstChild) cont.removeChild(cont.firstChild);
    this.carteles().forEach(c => {
      const caja = doc.createElement('div');
      caja.className = 'card card-camara-pluma';
      caja.id = c.id;
      [['strong', 'camara-pluma-titulo', '⚠️ ' + c.titulo],
       ['p', 'camara-pluma-medida', c.cuando],
       ['p', 'camara-pluma-accion', c.accion]].forEach(([tag, cls, txt]) => {
        const el = doc.createElement(tag);
        el.className = cls;
        el.textContent = txt;
        caja.appendChild(el);
      });
      const ok = doc.createElement('button');
      ok.type = 'button';
      ok.className = 'btn-top';
      ok.id = 'btn-' + c.id + '-ok';
      ok.textContent = 'Entendido';
      ok.addEventListener('click', () => { this.cerrar(c.id); if (alCerrar) alCerrar(); });
      caja.appendChild(ok);
      cont.appendChild(caja);
    });
    cont.hidden = !cont.firstChild;
  }
};

if (typeof module !== 'undefined' && module.exports) {
  module.exports = CartelesEquipo;
}
