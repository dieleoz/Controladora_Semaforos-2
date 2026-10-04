// ===== js/deg_fin.js =====
// SALIDA PROGRAMADA (D-52) y ROJO TOTAL (D-51): SPEC_4 §3.ter.ter, SPEC_2 §7.quater.
//
// El Degradado sin radio se deja a una HORA, igual en los dos postes, como el testigo la fija a la entrada:
//     SET_MODO:DEG_FIN:ahora,salida     (HH:MM:SS,HH:MM:SS; con PIN) · SET_MODO:DEG_FIN:CANCELAR
//     CONSULTA_DEG_FIN                  (sin PIN)
// LO QUE ESTE MODULO NO HACE, Y ES A PROPOSITO:
//   - No pinta ningun modo ni da la salida por hecha: la dicen el $ACK (texto en ACK_TEXTO de app.js), el $ERR
//     (MOTIVOS, aqui) y la consulta. Ningun texto promete nada del OTRO poste: sin radio nadie lo sabe.
//   - No escribe al cable: todo sale por DegAuto.enviarFin()/consultarFin() -> enviarComandoFirmware(), con su bool.
// Lo reparte DegAuto (acuse, rechazo, alarma, evento, alStatus, iniciar): app.js ya lo llama.

const DegFin = {
  ORDEN_FIN: 'SET_MODO:DEG_FIN',
  CONSULTA_FIN: 'CONSULTA_DEG_FIN',
  CLAVE: 'iotvial.degfin.v1',
  TRASLADO_DEF_MIN: 20,
  MEDIO_DIA_S: 43200,         // SPEC_2 7.quater (c): mas de 12 h por delante es "Salida ya vencida"
  CONSULTA_TRAS_MS: 2500,     // despues de la de DegAuto: no se cruzan

  // $ERR DESC literal del firmware (modo_degradado_textoSalida) -> que hacer. Ni cifras de plazo ni "reintente".
  MOTIVOS: {
    'FORMATO_INVALIDO': 'El equipo no entendio la orden (horas mal escritas). No ha cambiado nada: ' +
      'anote el literal y avise.',
    'No esta en Degradado': 'Este poste no esta en Degradado: no hay salida que programar. ' +
      'No ha cambiado nada.',
    'Ya esta saliendo': 'Este poste ya esta saliendo del Degradado: no se programa nada.',
    'Falta: reloj sin poner en hora': 'Falta: reloj sin poner en hora. Este poste no tiene hora ' +
      'fiable: pongala en hora y vuelva a programar.',
    'Ahora no coincide': 'Ahora no coincide: el reloj del telefono y el del poste difieren mas de ' +
      'la tolerancia. Revise la hora del telefono.',
    'Salida ya vencida': 'Salida ya vencida: la hora ya paso o queda demasiado lejos. ' +
      'Reprograme la salida en el Maestro o cancelela.',
    'Salida antes del inicio': 'Salida antes del inicio: cae antes de la hora de inicio del ' +
      'testigo. Ponga una salida posterior.',
    'En verde: repita en rojo': 'En verde: la salida solo se guarda con el poste en rojo. ' +
      'Espere a verlo en rojo y repita.',
    'No se pudo guardar la salida': 'No se pudo guardar la salida en la memoria del poste: NO ' +
      'queda programada. Si se repite, avise a mantenimiento.',
    'No hay salida programada': 'No hay salida programada en este poste: no hay nada que cancelar.',
    'Cancele antes la salida programada': 'Hay una salida programada en este poste: cancelela ' +
      'antes de entrar de nuevo con testigo.',
  },

  ctx: null,
  _el: {},
  _sale: null,                // { hms, finMs, respaldada } de la ultima consulta; null = ninguna
  _consultada: false,
  _consultaEnCurso: false,
  _sinOpcion: false,
  _cancelando: false,         // punta a la que esta app envio CANCELAR; solo la del Maestro borra el registro local
  _timerConsulta: null,

  // ---- Puras ---------------------------------------------------------------

  // NINGUNA -> {ninguna:true}; SALE_HHMMSS_FALTAN_<s>S_<RESPALDADA|SIN_RESPALDO> -> {hms, faltan, respaldada}.
  leerConsulta(result) {
    const r = String(result || '');
    if (r === 'NINGUNA') return { ninguna: true };
    const m = /^SALE_(\d\d)(\d\d)(\d\d)_FALTAN_(\d+)S_(RESPALDADA|SIN_RESPALDO)$/.exec(r);
    return m ? { hms: m[1] + ':' + m[2] + ':' + m[3], faltan: Number(m[4]), respaldada: m[5] === 'RESPALDADA' } : null;
  },

  _seg(hms) { const p = String(hms).split(':').map(Number); return p[0] * 3600 + p[1] * 60 + p[2]; },

  // `hora` es horaLocal24() de app.js. Salida: la escrita (HH:MM[:SS]) o `ahora` + traslado, en las proximas 12 h.
  componer(ahoraD, trasladoMin, texto, hora) {
    const ahora = hora(ahoraD);
    let salida;
    const t = String(texto || '').trim();
    if (t) {
      const m = /^(\d\d?):(\d\d)(?::(\d\d))?$/.exec(t);
      if (!m || Number(m[1]) > 23 || Number(m[2]) > 59 || Number(m[3] || 0) > 59) {
        return { error: 'Salida invalida: escriba HH:MM:SS.' };
      }
      salida = m[1].padStart(2, '0') + ':' + m[2] + ':' + (m[3] || '00');
    } else {
      const n = Number(trasladoMin);
      if (!Number.isInteger(n) || n < 1 || n > this.MEDIO_DIA_S / 60) {
        return { error: 'Traslado invalido: minutos enteros de 1 a ' + this.MEDIO_DIA_S / 60 + '.' };
      }
      salida = hora(new Date(ahoraD.getTime() + n * 60000));
    }
    const falta = ((this._seg(salida) - this._seg(ahora)) % 86400 + 86400) % 86400;
    if (falta < 1 || falta > this.MEDIO_DIA_S) {
      return { error: 'Salida invalida: debe caer dentro de las proximas 12 h.' };
    }
    return { ahora, salida, salidaMs: ahoraD.getTime() + falta * 1000, args: ahora + ',' + salida };
  },

  // El Esclavo lleva la salida GUARDADA del Maestro (aceptada); solo `ahora` es fresco.
  componerEsclavo(g, ahoraD, hora) {
    if (!g || g.estado !== 'ACEPTADO') {
      return { error: 'No hay en este telefono una salida ACEPTADA por el POSTE 1. ' +
                      'Programela primero en el Maestro.' };
    }
    if (ahoraD.getTime() >= g.salidaMs) {
      return { error: 'La salida guardada (' + g.salida + ') ya paso: no se envia. ' +
                      'Reprogramela en el Maestro.' };
    }
    const ahora = hora(ahoraD);
    return { ahora, salida: g.salida, args: ahora + ',' + g.salida };
  },

  // ---- Almacen: sobrevive a cerrar la app ----------------------------------

  leer() {
    try { return JSON.parse(localStorage.getItem(this.CLAVE) || 'null'); } catch (e) { return null; }
  },
  guardar(g) {
    try {
      if (g) localStorage.setItem(this.CLAVE, JSON.stringify(g)); else localStorage.removeItem(this.CLAVE);
    } catch (e) { /* sin almacen: nada que guardar */ }
  },

  // ---- Enganches (los llama DegAuto) ----------------------------------------

  // Un poste nuevo (o el mismo tras reconectar) se consulta una vez, un poco despues que la del automatico.
  alStatus(node) {
    if (!this.ctx || !node || node === this.ctx.state.node) return;
    this._sale = null; this._consultada = false; this._sinOpcion = false;
    clearTimeout(this._timerConsulta);
    this._timerConsulta = setTimeout(() => {
      if (this.ctx.state.node === node) this._consultar();
    }, this.CONSULTA_TRAS_MS);
    setTimeout(() => this.render(), 0);
  },

  // $ACK de la orden (null: el texto lo pone ACK_TEXTO) o de la consulta ({tono, texto, toast}).
  acuse(data) {
    if (data.CMD === this.CONSULTA_FIN) {
      this._consultaEnCurso = false;
      const c = this.leerConsulta(data.RESULT);
      if (!c) return null;
      this._consultada = true;
      this._sale = c.ninguna ? null : { hms: c.hms, respaldada: c.respaldada, finMs: Date.now() + c.faltan * 1000 };
      this.render();
      return { tono: 'cyan', toast: 'Salida programada consultada', texto: c.ninguna
        ? 'Este poste no tiene salida programada.'
        : 'Salida programada en este poste a las ' + c.hms + ', faltan ' + c.faltan + ' s' +
          (c.respaldada ? '.' : ' (SIN respaldo: un corte de luz la pierde).') };
    }
    const g = this.leer();
    const node = this.ctx && this.ctx.state.node;
    const res = String(data.RESULT || '');
    if (/^(PROGRAMADA|REPROGRAMADA|PROGRAMADA_SIN_RESPALDO)$/.test(res) && g) {
      if (node === 'MAESTRO') { g.estado = 'ACEPTADO'; delete g.previa; } else if (node === 'ESCLAVO') g.esclavo = true;
      this.guardar(g);
    } else if (res === 'CANCELADA' && this._cancelando === 'MAESTRO' && node === 'MAESTRO') this.guardar(null);
    else if (res === 'CANCELADA' && this._cancelando && g) { delete g.esclavo; this.guardar(g); }  // el registro es del Maestro
    this._cancelando = false;
    this._consultar();
    this.render();
    return null;
  },

  // $ERR de la orden, o de DEG_T con una salida pendiente. {texto, toast} o null.
  rechazo(data) {
    if (!this.ctx || !data) return null;
    const desc = String(data.DESC || '');
    const otraConsulta = typeof DegAuto !== 'undefined' && DegAuto._consultaEnCurso;
    const noLaLleva = data.CMD === 'DESCONOCIDO' && desc === 'COMANDO_NO_SOPORTADO' &&
                      data.NODE !== 'PUENTE' && !otraConsulta;
    if (this._consultaEnCurso && (data.CMD === this.CONSULTA_FIN || noLaLleva)) {
      this._consultaEnCurso = false; this._sinOpcion = true; this.render();
      return { texto: 'Este firmware no lleva la salida programada.',
               toast: 'Salida programada: no esta en este firmware' };
    }
    const esFin = data.CMD === this.ORDEN_FIN;
    if (!esFin && !(data.CMD === 'SET_MODO:DEG_T' && desc.indexOf('Cancele antes') === 0)) return null;
    if (esFin) {
      const g = this.leer();
      if (g && g.estado === 'ENVIADO') this.guardar(g.previa || null);   // la anterior sigue en el poste
      this._cancelando = false;
      this._consultar();
      this.render();
    }
    const texto = this.MOTIVOS[desc];
    return texto ? { texto, toast: 'Salida programada: ' + desc } : null;
  },

  // $ALARM DEGRADADO,CAUSA:ROJO_TOTAL,ACCION:ROJO_FIJO: lo puso una persona; no es ROJO_SIN_HORA.
  alarma(data) {
    if (!data || data.EVENTO !== 'DEGRADADO' || data.CAUSA !== 'ROJO_TOTAL') return null;
    return { tono: 'red', toast: 'Rojo total puesto en este poste',
      texto: 'Rojo total puesto por una persona en este poste: ROJO FIJO. El otro poste sigue ' +
             'alternando por su reloj: vaya a el.' };
  },

  // $EVENT DEGRADADO,SALIDA_PROGRAMADA_<HH:MM:SS|EJECUTADA>.
  evento(data) {
    if (!data || data.ORIGEN !== 'DEGRADADO') return null;
    const m = /^SALIDA_PROGRAMADA_(.+)$/.exec(String(data.DETALLE || ''));
    if (!m) return null;
    if (m[1] === 'EJECUTADA') {
      this._sale = null; this.guardar(null); this.render();
      return { tono: 'red', toast: 'Salida programada ejecutada',
        texto: 'La salida programada se ejecuto: este poste sale del Degradado ahora (primero rojo). ' +
               'Compruebe la luz.' };
    }
    if (/^\d\d:\d\d:\d\d$/.test(m[1])) {
      return { tono: 'cyan', toast: 'Salida programada aceptada',
        texto: 'Este poste saldra del Degradado a las ' + m[1] + ' por su reloj, sin radio.' };
    }
    return null;
  },

  // ---- Pantalla: la tarjeta se arma aqui (index.html es de mas de 500 lineas y no crece) ----

  iniciar(ctx) {
    this.ctx = ctx;
    const ref = document.getElementById('card-deg-auto');
    if (!ref || !ref.parentNode) return;
    const card = document.createElement('div');
    card.className = 'card card-degradado';
    card.id = 'card-deg-fin';
    card.innerHTML =
      '<div class="card-header-flex"><h3>⏲️ Salida programada del Degradado</h3>' +
      '<span class="badge badge-degradado">PELIGROSO</span></div>' +
      '<p class="card-sub"><strong>Sin radio el otro poste no se entera: programe la salida en los ' +
      'DOS postes, con paleteros en los dos extremos.</strong></p>' +
      '<div id="degfin-maestro"><div class="form-group">' +
      '<label for="num-degfin-traslado">Traslado hasta el POSTE 2 (min)</label>' +
      '<input type="number" id="num-degfin-traslado" class="form-input" min="1" max="720" value="' +
      this.TRASLADO_DEF_MIN + '">' +
      '<label for="txt-degfin-salida">Hora de salida (HH:MM:SS; vacio = ahora + traslado)</label>' +
      '<input type="text" id="txt-degfin-salida" class="form-input" placeholder="HH:MM:SS"></div>' +
      '<button class="btn-degradado" id="btn-degfin-maestro">Programar salida del Degradado</button>' +
      '<button class="btn-degradado" id="btn-degfin-reprogramar">Reprogramar salida</button></div>' +
      '<div id="degfin-esclavo"><button class="btn-degradado" id="btn-degfin-esclavo">' +
      'Aplicar salida guardada</button></div>' +
      '<button class="btn-secondary-bt" id="btn-degfin-cancelar">Cancelar salida</button>' +
      '<p class="card-sub" id="degfin-estado"></p><p class="card-sub" id="degfin-cartel"></p>';
    ref.parentNode.insertBefore(card, ref);
    const $ = (id) => document.getElementById(id);
    this._el = { bloqueM: $('degfin-maestro'), bloqueE: $('degfin-esclavo'), traslado: $('num-degfin-traslado'),
                 salida: $('txt-degfin-salida'), btnE: $('btn-degfin-esclavo'), reprog: $('btn-degfin-reprogramar'),
                 estado: $('degfin-estado'), cartel: $('degfin-cartel') };
    $('btn-degfin-maestro').addEventListener('click', () => this._pulsar('MAESTRO'));
    this._el.reprog.addEventListener('click', () => this._pulsar('MAESTRO'));
    this._el.btnE.addEventListener('click', () => this._pulsar('ESCLAVO'));
    $('btn-degfin-cancelar').addEventListener('click', () => this._cancelar());
    this.render();
    setInterval(() => this.render(), 1000);
  },

  render() {
    const el = this._el;
    if (!this.ctx || !el.estado) return;
    const node = this.ctx.state.node;
    el.bloqueM.style.display = node === 'MAESTRO' ? '' : 'none';
    el.bloqueE.style.display = node === 'ESCLAVO' ? '' : 'none';
    el.reprog.style.display = this._sale ? '' : 'none';
    const g = this.leer();
    const vivo = g && g.estado === 'ACEPTADO' && Date.now() < g.salidaMs;
    el.btnE.textContent = vivo ? 'Aplicar salida guardada - Maestro ' + (g.serie || '?') + ', salida ' + g.salida
                               : 'Aplicar salida guardada';
    let t;
    if (!node) t = 'Conectese a un poste identificado.';
    else if (this._sinOpcion) t = 'Este firmware no lleva la salida programada.';
    else if (!this._consultada) t = 'Consultando la salida de este poste...';
    else if (!this._sale) t = 'Este poste: sin salida programada.';
    else {
      const f = Math.round((this._sale.finMs - Date.now()) / 1000);
      t = 'Este poste: sale a ' + this._sale.hms + (f > 0 ? ', faltan ' + f + ' s' : ', ya le toca') +
          (this._sale.respaldada ? '.' : ' - sin respaldo: un corte de luz la pierde.');
    }
    el.estado.textContent = t;
    // Sin radio el firmware no sabe si el otro poste la tiene (SPEC_2 7.quater (h).1): el cartel queda
    // fijo hasta que la salida pase, se cancele desde aqui o el Esclavo la acuse.
    el.cartel.textContent = vivo && !g.esclavo
      ? 'Salida programada en el Maestro; FALTA el Esclavo: aplique alli la salida guardada.' : '';
  },

  _consultar() {
    if (DegAuto.consultarFin()) this._consultaEnCurso = true;
  },

  _aviso(texto) {
    this.ctx.showToast(texto);
    this.ctx.addEvent('red', 'Salida programada: ' + texto);
  },

  // Si el aviso de paleteros se abre, al confirmar llama a `otra` (no re-pulsa un boton que ya no es este).
  _enviar(args, otra) {
    const av = typeof AvisoDegradado !== 'undefined' ? AvisoDegradado : null;
    if (av) av._reintento = otra;
    try { return DegAuto.enviarFin(args); } finally { if (av) av._reintento = null; }
  },

  // Maestro y Esclavo comparten el camino: punta, validar, PIN, `ahora` releido, aviso de paleteros, envio.
  _pulsar(punta) {
    const c = this.ctx;
    if (c.state.node !== punta) {
      this._aviso('este paso es del ' + (punta === 'MAESTRO' ? 'POSTE 1' : 'POSTE 2') +
                  ' y al otro lado hay ' + (c.state.node || 'un poste sin identificar') + '.');
      return;
    }
    const el = this._el;
    const comp = (d) => punta === 'MAESTRO'
      ? this.componer(d, el.traslado && el.traslado.value, el.salida && el.salida.value, c.horaLocal24)
      : this.componerEsclavo(this.leer(), d, c.horaLocal24);
    const previa = comp(new Date());
    if (previa.error) { this._aviso(previa.error); return; }
    if (!c.state.pinVerificado) { c.pedirPin(() => this._pulsar(punta)); return; }
    const ahoraD = new Date();   // `ahora` se LEE otra vez, justo antes de enviar
    const o = comp(ahoraD);
    if (o.error) { this._aviso(o.error); return; }
    if (!this._enviar(o.args, () => this._pulsar(punta))) return;
    if (punta === 'MAESTRO') {
      const ant = this.leer();
      this.guardar({ estado: 'ENVIADO', salida: o.salida, salidaMs: o.salidaMs, serie: c.state.serie || null,
                     previa: ant && ant.estado === 'ACEPTADO' ? ant : (ant && ant.previa) || null });
    }
    c.addEvent('cyan', 'Tecnico: orden ' + this.ORDEN_FIN + ':' + o.args + ' (SALIDA PROGRAMADA) enviada al ' +
               (punta === 'MAESTRO' ? 'POSTE 1' : 'POSTE 2') + '. El equipo puede rechazarla y dira por que.');
    c.showToast('Salida enviada: espere la respuesta del equipo');
    this.render();
  },

  _cancelar() {
    const c = this.ctx;
    if (!c.state.node) { c.showToast('Conectese a un poste primero'); return; }
    if (!c.state.pinVerificado) { c.pedirPin(() => this._cancelar()); return; }
    if (!this._enviar('CANCELAR', () => this._cancelar())) return;
    this._cancelando = c.state.node;
    c.addEvent('cyan', 'Tecnico: orden ' + this.ORDEN_FIN + ':CANCELAR enviada a este poste. ' +
                       'Cancele tambien en el otro.');
  },
};

if (typeof module !== 'undefined' && module.exports) {
  module.exports = DegFin;
}
