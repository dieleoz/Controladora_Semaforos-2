// ===== js/deg_auto.js =====
// DEGRADADO AUTOMATICO (A-15): SPEC_4 §3.ter.bis y su tabla de acuses, SPEC_2 §7.ter.
//
// Una OPCION por poste, que se pone en los DOS. Con la opcion en los dos y la radio
// caida el tiempo que fija el firmware, los dos pasan solos a rojo y arrancan a una hora
// fija, como el testigo pero sin telefono. Ordenes (las atienden las DOS puntas, por eso
// no van ni en SOLO_MAESTRO ni en SOLO_ESCLAVO de app.js):
//     SET_DEG_AUTO:1 / SET_DEG_AUTO:0   con PIN; el $ACK sale DIFERIDO, con el ECO del otro
//     CONSULTA_DEG_AUTO                 sin PIN (SIN_PIN de app.js)
//
// LO QUE ESTE MODULO NO HACE, Y ES A PROPOSITO:
//   - No pinta el estado por haber pulsado. Lo pinta la respuesta de CONSULTA_DEG_AUTO,
//     que se pide al identificar el poste y tras cada respuesta a SET_DEG_AUTO. Mientras
//     el $ACK diferido no llega, los botones dicen "esperando al otro poste".
//   - No escribe al cable: todo sale por enviarComandoFirmware(), cuyo bool se mira.
//   - Ningun texto lleva la cifra del plazo de acuse del firmware (CLAUDE.md §14): la
//     espera de aqui es la de la pantalla, mas larga, y no se ensena.

const DegAuto = {
  ORDEN: 'SET_DEG_AUTO',
  CONSULTA: 'CONSULTA_DEG_AUTO',
  ESPERA_MS: 20000,           // la pantalla deja de esperar; el firmware contesta antes
  // La consulta al conectar sale un poco DESPUES del primer $STATUS: no se cruza con lo
  // que el operario pulse nada mas conectar ni con la ventana de N-124 de app.js.
  CONSULTA_TRAS_MS: 2000,

  ACUSE: {
    'ON_EFECTIVO': { tono: 'green', toast: 'Degradado automatico ACTIVO en los dos postes',
      texto: 'Equipo: degradado automatico ACTIVO y el otro poste tambien lo tiene. Si la ' +
             'radio cae, los dos pasan solos a rojo y arrancan a una hora fija.' },
    'ON_FALTA_EL_OTRO_POSTE': { tono: 'red', toast: 'Activado aqui: falta el otro poste',
      texto: 'Equipo: activado en ESTE poste, pero el otro no esta listo: asi NO hace ' +
             'nada. Vaya al otro poste y activelo tambien alli.' },
    'OFF': { tono: 'green', toast: 'Degradado automatico desactivado en este poste',
      texto: 'Equipo: degradado automatico DESACTIVADO en este poste; el otro ya lo sabe. ' +
             'Sin radio este poste queda en ambar, como siempre.' },
  },

  RECHAZO: {
    'SIN_ENLACE_CON_EL_OTRO_POSTE': 'No hay radio con el otro poste: la opcion solo se ' +
      'cambia con radio, para que el otro se entere. No ha cambiado nada.',
    'EN_DEGRADADO_SALGA_PRIMERO': 'Este poste esta en degradado: saquelo primero (Volver ' +
      'al menu en el Poste 1) y repita. No ha cambiado nada.',
    'FORMATO_INVALIDO': 'El equipo no entendio la orden. No ha cambiado nada: anote el ' +
      'literal y avise.',
    'CAMBIADO_AQUI_SIN_ACUSE_DEL_OTRO_POSTE': 'Cambiado en ESTE poste, pero el otro no lo ' +
      'ha acusado. Mire la consulta de abajo y, si hace falta, repita con radio.',
  },

  // CAUSA:AUTO_NO_<codigo>: nombreMotivo() del Maestro y nombreRechazo() del Esclavo, los
  // mismos codigos en las dos puntas (cortos: la CAUSA del $ALARM cabe en 19, esp32_07).
  MOTIVO: {
    'HORA': 'reloj sin poner en hora; pongalo en hora y entre con testigo',
    'DESFASE': 'la hora del poste no cuadra; ponga en hora los dos postes',
    'DESPEJE': 'despeje fuera de rango (30-255); avise a mantenimiento',
    'INICIO': 'la hora de arranque ya habia pasado; entre con testigo a mano',
    'AMBAR': 'hay un ambar de emergencia puesto; quitelo antes',
    'EN_VERDE': 'estaba en verde y el testigo solo se guarda en rojo; entre con testigo en rojo',
    'GUARDADO': 'no se pudo guardar el testigo; repita y, si vuelve, avise a mantenimiento',
    // OK: el default de los dos, p.ej. si entrarTestigo() devuelve MDT_RENOVADO.
    'OK': 'respuesta inesperada del poste; mire el modo y avise a mantenimiento',
  },

  RENOVAR: { tono: 'red', toast: 'Renueve el testigo en los dos postes',
    texto: 'Lleva 28 dias o mas sin renovar el testigo: vaya a los dos postes y repita el ' +
           'Degradado con testigo para volver a poner en hora los relojes.' },

  ctx: null,
  _el: {},
  _estado: null,              // { este, otro, apto } de la ultima consulta, o null
  _sinOpcion: false,          // el firmware contesto DESCONOCIDO a la consulta
  _consultaEnCurso: false,
  _timerConsulta: null,
  _espera: null,              // { valor, timer } mientras falta el $ACK diferido

  // ---- Puras ---------------------------------------------------------------

  // RESULT:ESTE_<ON/OFF>_OTRO_<ON/OFF>_APTO_<SI/NO>; cualquier otra forma es null.
  leerConsulta(result) {
    const m = /^ESTE_(ON|OFF)_OTRO_(ON|OFF)_APTO_(SI|NO)$/.exec(String(result || ''));
    return m ? { este: m[1] === 'ON', otro: m[2] === 'ON', apto: m[3] === 'SI' } : null;
  },

  _motivo(causa) {
    const nombre = String(causa || '').replace(/^AUTO_NO_/, '');
    return this.MOTIVO[nombre] || nombre || 'sin motivo';
  },

  // $EVENT ORIGEN:DEGRADADO. DETALLE lleva la hora dentro: no cabe en AvisosEquipo.
  evento(data) {
    if (!data || data.ORIGEN !== 'DEGRADADO') return null;
    const d = String(data.DETALLE || '');
    const m = /^AUTO_ENTRADA_INICIO_(\d{2}):(\d{2}):\d{2}$/.exec(d);
    if (m) {
      return { tono: 'red', toast: 'Degradado automatico: rojo hasta ' + m[1] + ':' + m[2],
        texto: 'Sin radio 5 min: modo degradado automatico, rojo hasta ' + m[1] + ':' + m[2] +
               '. Despues los dos postes alternan solos por reloj.' };
    }
    // El aviso de limite que ya publica el firmware (SYNC:.. AVISO:SI VENCIDA:..): con
    // testigo sale a los 28 dias. El testigo ya no vence (29/09): solo pide renovarlo.
    if (/(^| )AVISO:SI( |$)/.test(d)) return this.RENOVAR;
    if (d === 'ENLACE_DISPONIBLE') {
      return { tono: 'red', toast: 'La radio volvio: sigue en degradado',
        texto: 'La radio volvio. Sigue en degradado: para volver al ciclo, Volver al menu ' +
               'en el Poste 1.' };
    }
    return null;
  },

  // $ALARM EVENTO:DEGRADADO. El resto de causas siguen en AvisosEquipo.
  alarma(data) {
    if (!data || data.EVENTO !== 'DEGRADADO') return null;
    const causa = String(data.CAUSA || '');
    if (causa.indexOf('AUTO_NO_') === 0) {
      // Maestro con MDT_NO_GUARDADO: ACCION:QUEDA_ROJO (rojo fijo); el resto, SIGUE_AMBAR.
      const rojo = data.ACCION === 'QUEDA_ROJO';
      return { tono: 'red', toast: 'No pudo entrar solo en degradado',
        texto: 'No pudo entrar solo: ' + this._motivo(causa) + (rojo
          ? '. Queda en ROJO FIJO en este poste, no en ambar: avise a mantenimiento.'
          : '. Sigue en ambar.') };
    }
    if (causa === 'OTRO_EN_DEGRADADO') {
      return { tono: 'red', toast: 'El otro poste esta en degradado',
        texto: 'El otro poste esta en degradado y este no: vaya al otro poste.' };
    }
    if (causa === 'RENOVAR_TESTIGO') return this.RENOVAR;
    return null;
  },

  // ---- Enganches de app.js ---------------------------------------------------

  // Cada $STATUS con NODE, ANTES de que app.js lo guarde: un poste nuevo (o el mismo
  // tras reconectar, que deja state.node en null) se consulta una vez.
  alStatus(node) {
    if (!this.ctx || !node || node === this.ctx.state.node) return;
    this._estado = null;
    this._sinOpcion = false;
    clearTimeout(this._timerConsulta);
    this._timerConsulta = setTimeout(() => {
      if (this.ctx.state.node === node) this._consultar();
    }, this.CONSULTA_TRAS_MS);
    setTimeout(() => this.render(), 0);  // app.js guarda el NODE justo despues
  },

  // Todo $ACK. Devuelve {tono, texto, toast} si es de este modulo; si no, null.
  acuse(data) {
    if (!this.ctx || !data) return null;
    if (data.CMD === this.CONSULTA) {
      this._consultaEnCurso = false;
      this._estado = this.leerConsulta(data.RESULT);
      this.render();
      const e = this._estado;
      if (!e) return null;  // forma desconocida: la pinta el fallback de app.js, en crudo
      return { tono: 'cyan', toast: 'Degradado automatico consultado',
        texto: 'Degradado automatico: ' + this._lineas(e).join(' · ') + '.' };
    }
    if (data.CMD !== this.ORDEN) return null;
    this._fin();
    return this.ACUSE[String(data.RESULT || '')] || null;
  },

  // Todo $ERR, primero en _traducirRechazo(). {texto, toast} o null.
  rechazo(data) {
    if (!this.ctx || !data) return null;
    // DESCONOCIDO no nombra la orden: solo se toma por la consulta si es el rechazo de
    // una punta (no del PUENTE) por orden no soportada y la consulta es lo que espera.
    const noLaLleva = data.CMD === 'DESCONOCIDO' && data.DESC === 'COMANDO_NO_SOPORTADO' &&
                      data.NODE !== 'PUENTE';
    if (this._consultaEnCurso && (data.CMD === this.CONSULTA || noLaLleva)) {
      this._consultaEnCurso = false;
      this._sinOpcion = true;
      this.render();
      return { texto: 'Este firmware no lleva el degradado automatico.',
               toast: 'Degradado automatico: no esta en este firmware' };
    }
    if (data.CMD !== this.ORDEN) return null;
    this._fin();
    const t = this.RECHAZO[String(data.DESC || '')];
    return t ? { texto: t, toast: 'Degradado automatico: ' + data.DESC } : null;
  },

  // ---- Pantalla ------------------------------------------------------------

  iniciar(ctx) {
    this.ctx = ctx;
    const $ = (id) => document.getElementById(id);
    this._el = { este: $('degauto-este'), otro: $('degauto-otro'), apto: $('degauto-apto'),
                 on: $('btn-degauto-on'), off: $('btn-degauto-off') };
    if (this._el.on) this._el.on.addEventListener('click', () => this._pulsar('1'));
    if (this._el.off) this._el.off.addEventListener('click', () => this._pulsar('0'));
    this.render();
    setInterval(() => this.render(), 1000);  // la desconexion no avisa a este modulo
  },

  _lineas(e) {
    return ['este poste ' + (e.este ? 'ON' : 'OFF'),
            'el otro poste ' + (e.otro ? 'ON' : 'OFF') + ' (ultimo oido por radio)',
            'listo para entrar solo ' + (e.apto ? 'SI' : 'NO')];
  },

  render() {
    const el = this._el;
    if (!this.ctx || !el.este) return;
    const e = this._estado;
    let l;
    if (!this.ctx.state.node) l = ['sin conectar', '-', '-'];
    else if (this._sinOpcion) l = ['este firmware no la lleva', '-', '-'];
    else if (!e) l = ['consultando...', '-', '-'];
    else l = [e.este ? 'ON' : 'OFF', (e.otro ? 'ON' : 'OFF') + ' (ultimo oido por radio)',
              e.apto ? 'SI' : 'NO'];
    el.este.textContent = l[0];
    el.otro.textContent = l[1];
    el.apto.textContent = l[2];
    const esperando = 'Esperando al otro poste...';
    if (el.on) {
      el.on.disabled = !!this._espera;
      el.on.textContent = this._espera && this._espera.valor === '1' ? esperando : 'Activar';
    }
    if (el.off) {
      el.off.disabled = !!this._espera;
      el.off.textContent = this._espera && this._espera.valor === '0' ? esperando : 'Desactivar';
    }
  },

  _consultar() {
    if (this.ctx.enviarComandoFirmware(this.CONSULTA)) this._consultaEnCurso = true;
    this.render();
  },

  // Respuesta (o fin de la espera) a SET_DEG_AUTO: se suelta el boton y se pregunta.
  _fin() {
    if (!this._espera) return;
    clearTimeout(this._espera.timer);
    this._espera = null;
    this._consultar();
  },

  _pulsar(valor) {
    const c = this.ctx;
    if (this._espera) { c.showToast('Ya hay una orden esperando al otro poste'); return; }
    if (!c.state.node) { c.showToast('Conectese a un poste primero'); return; }
    if (!c.state.pinVerificado) { c.pedirPin(() => this._pulsar(valor)); return; }
    if (!c.enviarComandoFirmware(this.ORDEN, valor)) return;
    this._espera = { valor, timer: setTimeout(() => {
      c.addEvent('red', 'Degradado automatico: el equipo no contesto. Mire la consulta.');
      this._fin();
      this.render();
    }, this.ESPERA_MS) };
    c.addEvent(valor === '1' ? 'red' : 'cyan', 'Tecnico: orden ' + this.ORDEN + ':' + valor +
      ' enviada. ' + (valor === '1'
        ? 'OJO: sin radio, los dos postes pasan solos a rojo y despues alternan por reloj, ' +
          'sin nadie mirando. Solo sirve si el otro poste tambien la tiene. '
        : '') + 'Esperando el acuse del otro poste.');
    this.render();
  },
};

if (typeof module !== 'undefined' && module.exports) {
  module.exports = DegAuto;
}
