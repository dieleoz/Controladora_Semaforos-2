// ===== js/testigo.js =====
// DEGRADADO CON TESTIGO (D-35): SPEC_4 §3.ter, SPEC_2 §7.bis, SPEC_6 A.1.bis y A.2.bis.
//
// Para cuando la radio entre postes no vuelve en semanas. UNA orden por poste,
//     SET_MODO:DEG_T:ahora,inicio,verde,despeje      (HH:MM:SS,HH:MM:SS,180,d; con PIN)
// primero al MAESTRO y despues, con el MISMO testigo guardado aqui, al ESCLAVO.
//
// LO QUE ESTE MODULO NO HACE, Y ES A PROPOSITO:
//   - No pinta ningun modo. Pulsar es "orden enviada"; lo que el equipo hizo lo dicen su
//     $ACK (texto en ACK_TEXTO de app.js) o su $ERR (texto aqui, en MOTIVOS).
//   - No escribe al cable: todo sale por enviarComandoFirmware(), cuyo bool se mira.
//   - No lleva al ESCLAVO un testigo que el MAESTRO no haya ACUSADO: una punta dentro y
//     la otra no es el caso peor de SPEC_6 §8. Sin acuse queda ENVIADO, y ENVIADO no se
//     ofrece al Esclavo.
//   - El SET_RTC previo NO bloquea (SPEC_4 §3.ter 2, SPEC_6 A.1.bis): el testigo sale con su $ACK, con su $ERR o
//     pasados ESPERA_RTC_MS sin contestar. Lo que protege es `ahora`, que el firmware valida contra su reloj.
//
// Los rechazos del firmware traen el MOTIVO EN TEXTO (modo_degradado_textoTestigo() del
// Maestro y degradado_textoRechazoTestigo() del Esclavo); por eso no van en ERR_TEXTO,
// cuyo censo (app_10) solo lee DESC en mayusculas. Se casan aqui por el literal EXACTO.

const Testigo = {
  CLAVE: 'iotvial.testigo.v1',
  ORDEN: 'SET_MODO:DEG_T',
  VERDE_S: 180,               // SPEC_2 §7.bis: viaja fijo (otro valor es FORMATO_INVALIDO)
  DESPEJE_MIN: 30,            // SPEC_2 §7.bis: TESTIGO_DESPEJE_MIN/MAX, no los 10-90
  DESPEJE_MAX: 255,
  TRASLADO_DEF_MIN: 20,       // SPEC_4 §3.ter paso 1
  // SPEC_2 §7.bis: el firmware da por vencido un inicio si (inicio - reloj) mod 86400 >
  // 12 h. Un traslado de mas de 12 h llegaria vencido: el tope del formulario sale de ahi.
  MEDIO_DIA_S: 43200,
  ESPERA_RTC_MS: 10000,       // cuanto se espera el $ACK del SET_RTC (NODE:PUENTE)

  // SPEC_6 A.2.bis: el literal exacto que emite el firmware, y que hacer.
  MOTIVOS: {
    'FORMATO_INVALIDO': 'Formato invalido: el equipo no entendio la orden (horas, verde ' +
      '180 o despeje). No es algo que corrija el operario: anote el literal y avise.',
    'Falta: reloj sin poner en hora': 'Falta: reloj sin poner en hora. Este poste no ' +
      'tiene una hora propia fiable con que comparar. Ponga la hora y repita.',
    'Ahora no coincide': 'Ahora no coincide. El reloj del telefono y el del poste ' +
      'difieren mas de la tolerancia: revise la hora del telefono y repita.',
    'Despeje fuera de rango (30-255)': 'Despeje fuera de rango (30-255). Corrija el ' +
      'numero en la app antes de reenviar.',
    'Inicio ya vencido': 'Inicio ya vencido. El reloj de este poste ya paso de inicio ' +
      'al recibir la orden: repita el flujo.',
    'Inicio ya vencido: repita el testigo en el Maestro': 'Inicio ya vencido: el ' +
      'testigo caduco. Repita el paso completo en el Maestro, con un traslado nuevo. No ' +
      'se reintenta con el mismo inicio.',
    'Ambar de emergencia puesto': 'Ambar de emergencia puesto. Alguien lo dejo ' +
      'encendido: hay que RETIRAR EL AMBAR (CANCELAR_AMBAR) antes de reintentar.',
    'En verde: repita en rojo': 'En verde: repita en rojo. El poste esta dando verde ' +
      'ahora; espere a verlo en rojo y repita la orden.',
    'No se pudo guardar el testigo': 'No se pudo guardar el testigo en la memoria del ' +
      'poste: NO ha entrado. Repita; si vuelve a salir, el equipo tiene una averia.',
  },

  ctx: null,
  _espera: null,              // { punta, timer } mientras se espera el $ACK del SET_RTC
  _el: {},

  // ---- Puras ---------------------------------------------------------------

  // `hora` es horaLocal24() de app.js: la hora se compone con getters, nunca con locale.
  // El inicio puede caer pasada la medianoche: HH:MM:SS se lee modulo 86400 (§7.bis).
  componer(ahoraDate, trasladoMin, despeje, hora) {
    const t = Number(trasladoMin);
    const d = Number(despeje);
    const tope = this.MEDIO_DIA_S / 60;
    if (!Number.isInteger(t) || t < 1 || t > tope) {
      return { error: 'Traslado invalido: minutos enteros de 1 a ' + tope + '.' };
    }
    if (!Number.isInteger(d) || d < this.DESPEJE_MIN || d > this.DESPEJE_MAX) {
      return { error: 'Despeje fuera de rango (' + this.DESPEJE_MIN + '-' +
                      this.DESPEJE_MAX + ' s).' };
    }
    const ini = new Date(ahoraDate.getTime() + t * 60000);
    const ahora = hora(ahoraDate);
    const inicio = hora(ini);
    return { ahora, inicio, inicioMs: ini.getTime(), verde: this.VERDE_S, despeje: d,
             args: ahora + ',' + inicio + ',' + this.VERDE_S + ',' + d };
  },

  _segDelDia(hhmmss) {
    const p = String(hhmmss).split(':').map(Number);
    return p[0] * 3600 + p[1] * 60 + p[2];
  },

  // La MISMA regla del firmware, sobre el reloj del telefono, mas la absoluta: un testigo
  // de ayer puede caer dentro de las 12 h de la regla modular y aqui no se lleva.
  vencido(g, ahoraDate, hora) {
    const falta = ((this._segDelDia(g.inicio) - this._segDelDia(hora(ahoraDate))) %
                   86400 + 86400) % 86400;
    return ahoraDate.getTime() >= g.inicioMs || falta > this.MEDIO_DIA_S;
  },

  // La orden del ESCLAVO: inicio, verde y despeje GUARDADOS; solo `ahora` es fresco.
  componerEsclavo(g, ahoraDate, hora) {
    if (!g || g.estado !== 'ACEPTADO') {
      return { error: 'No hay en este telefono un testigo ACEPTADO por el POSTE 1. ' +
                      'Hagalo primero en el Maestro.' };
    }
    if (this.vencido(g, ahoraDate, hora)) {
      return { error: 'El inicio del testigo (' + g.inicio + ') ya paso: no se envia. ' +
                      'Repita el testigo en el Maestro.' };
    }
    const ahora = hora(ahoraDate);
    return { ahora, args: ahora + ',' + g.inicio + ',' + g.verde + ',' + g.despeje };
  },

  // ---- Almacen: sobrevive a cerrar la app ----------------------------------

  _almacen() {
    try {
      if (typeof localStorage === 'undefined' || localStorage === null) return null;
      return localStorage;
    } catch (e) { return null; }
  },
  leer() {
    const a = this._almacen();
    if (!a) return null;
    try { return JSON.parse(a.getItem(this.CLAVE) || 'null'); } catch (e) { return null; }
  },
  guardar(g) {
    const a = this._almacen();
    if (!a) return false;
    try { a.setItem(this.CLAVE, JSON.stringify(g)); return true; } catch (e) { return false; }
  },
  borrar() {
    const a = this._almacen();
    if (a) { try { a.removeItem(this.CLAVE); } catch (e) { /* nada que borrar */ } }
  },

  // ---- Enganches de app.js: $ACK y $ERR -------------------------------------

  // Todo $ACK pasa por aqui (el texto lo pone ACK_TEXTO). Dos efectos:
  //  - el $ACK del SET_RTC del PUENTE libera el testigo que esperaba;
  //  - el $ACK del testigo en el MAESTRO lo deja ACEPTADO, que es lo unico que habilita
  //    llevarlo al ESCLAVO. OK y RENOVADO son acuse; lo que cambia es la luz (ACK_TEXTO).
  anotarAcuse(data, node) {
    if (!this.ctx || !data) return;
    if (data.CMD === 'SET_RTC' && data.NODE === 'PUENTE' && this._espera) {
      const punta = this._espera.punta;
      clearTimeout(this._espera.timer);
      this._espera = null;
      this._enviarTestigo(punta);
      return;
    }
    if (data.CMD !== this.ORDEN) return;
    const g = this.leer();
    if (node === 'MAESTRO' && g && (g.estado === 'ENVIADO' || g.estado === 'ACEPTADO')) {
      g.estado = 'ACEPTADO';
      g.aceptadoMs = Date.now();
      this.guardar(g);
    }
    if (node === 'MAESTRO' && g) {
      this.ctx.addEvent('red', 'Testigo del POSTE 1 guardado en este telefono: inicio ' +
        g.inicio + ', verde ' + g.verde + ' s, despeje ' + g.despeje + ' s. Lleve el ' +
        'MISMO testigo al POSTE 2 antes de las ' + g.inicio + ' con "Aplicar testigo guardado": ' +
        'hasta que el POSTE 2 lo acepte, una punta va por testigo y la otra no.');
    } else if (node === 'ESCLAVO') {
      this.ctx.addEvent('red', 'POSTE 2 con el testigo. Quedese a ver un ciclo completo y ' +
        'compruebe CON LOS OJOS que nunca hay verde en los dos postes a la vez.');
    }
    this.render();
  },

  // Primero en _traducirRechazo(). Devuelve el texto, o null si no es de esta orden.
  rechazo(data, node) {
    if (!data) return null;
    if (data.CMD === 'SET_RTC' && this._espera) {
      clearTimeout(this._espera.timer);
      const punta = this._espera.punta;
      this._espera = null;
      this._aviso('el puente rechazo la hora (motivo al lado). El testigo sale igual: el poste valida ' +
                  '`ahora` con su propio reloj y dira si no cuadra.');
      this._enviarTestigo(punta);
      return null;
    }
    if (data.CMD !== this.ORDEN) return null;
    const g = this.leer();
    if (node === 'MAESTRO' && g && g.estado === 'ENVIADO') this.borrar();
    this.render();
    const texto = this.MOTIVOS[String(data.DESC || '')];
    return texto ? { texto, toast: 'Testigo rechazado: ' + data.DESC } : null;
  },

  // ---- Pantalla ------------------------------------------------------------

  iniciar(ctx) {
    this.ctx = ctx;
    const $ = (id) => document.getElementById(id);
    this._el = {
      bloqueM: $('testigo-maestro'), bloqueE: $('testigo-esclavo'),
      traslado: $('num-testigo-traslado'), despeje: $('num-testigo-despeje'),
      chk: $('chk-testigo-verificado'), btnM: $('btn-testigo-maestro'),
      btnE: $('btn-testigo-esclavo'), estado: $('testigo-estado'),
    };
    const el = this._el;
    if (el.despeje) {
      const conf = Number(ctx.state.tiempoDespejeSeg) || this.DESPEJE_MIN;
      el.despeje.value = Math.min(this.DESPEJE_MAX, Math.max(this.DESPEJE_MIN, conf));
    }
    if (el.traslado && !el.traslado.value) el.traslado.value = this.TRASLADO_DEF_MIN;
    if (el.btnM) el.btnM.addEventListener('click', () => this._pulsar('MAESTRO'));
    if (el.btnE) el.btnE.addEventListener('click', () => this._pulsar('ESCLAVO'));
    this.render();
    setInterval(() => this.render(), 1000);
  },

  _mmss(ms) {
    const s = Math.max(0, Math.round(ms / 1000));
    const dd = (n) => String(n).padStart(2, '0');
    return dd(Math.floor(s / 60)) + ':' + dd(s % 60);
  },

  render() {
    const el = this._el;
    if (!this.ctx || !el.estado) return;
    const node = this.ctx.state.node;
    if (el.bloqueM) el.bloqueM.style.display = node === 'MAESTRO' ? '' : 'none';
    if (el.bloqueE) el.bloqueE.style.display = node === 'ESCLAVO' ? '' : 'none';
    const g = this.leer();
    const vivo = g && !this.vencido(g, new Date(), this.ctx.horaLocal24);
    let txt;
    if (!g) {
      txt = 'Sin testigo guardado en este telefono.';
    } else if (g.estado === 'ENVIADO') {
      txt = 'Testigo enviado al POSTE 1 (serie ' + (g.serie || '?') + ') a las ' +
            g.emitido + '; esperando su acuse. Sin acuse no hay nada que llevar al POSTE 2.';
    } else if (vivo) {
      txt = 'Testigo del POSTE 1 (serie ' + (g.serie || '?') + '): inicio ' + g.inicio +
            ', verde ' + g.verde + ' s, despeje ' + g.despeje + ' s. Faltan ' +
            this._mmss(g.inicioMs - Date.now()) + ' para el inicio.';
    } else {
      txt = 'El inicio del testigo (' + g.inicio + ') ya paso. Para el POSTE 2 hay que ' +
            'repetir el testigo en el Maestro.';
    }
    if (this._espera) txt = 'Esperando el acuse de la hora del puente... ' + txt;
    if (!node) txt = 'Conectese a un poste identificado (POSTE 1 o POSTE 2). ' + txt;
    el.estado.textContent = txt;
    if (el.btnE) {
      el.btnE.textContent = (g && g.estado === 'ACEPTADO' && vivo)
        ? 'Aplicar testigo guardado — Maestro ' + (g.serie || '?') + ', inicio ' + g.inicio
        : 'Aplicar testigo guardado';
    }
  },

  _aviso(texto) {
    this.ctx.showToast(texto);
    this.ctx.addEvent('red', 'Degradado con testigo: ' + texto);
  },

  _componerPara(punta, ahoraD) {
    const el = this._el;
    return punta === 'MAESTRO'
      ? this.componer(ahoraD, el.traslado && el.traslado.value,
                      el.despeje && el.despeje.value, this.ctx.horaLocal24)
      : this.componerEsclavo(this.leer(), ahoraD, this.ctx.horaLocal24);
  },

  // Maestro y Esclavo comparten el camino: PIN, casilla, SET_RTC, su $ACK, `ahora` fresco.
  _pulsar(punta) {
    const c = this.ctx;
    if (this._espera) { this._aviso('ya hay un envio en curso; espere su resultado.'); return; }
    if (c.state.node !== punta) {
      this._aviso('este paso es del ' + (punta === 'MAESTRO' ? 'POSTE 1' : 'POSTE 2') +
                  ' y al otro lado hay ' + (c.state.node || 'un poste sin identificar') + '.');
      return;
    }
    const otra = c.puntaCorrecta(this.ORDEN);
    if (otra) { c.avisarOtraPunta(this.ORDEN, otra); return; }
    // Validar antes de pedir nada: un error de tecleo no gasta un PIN ni un SET_RTC.
    const previa = this._componerPara(punta, new Date());
    if (previa.error) { this._aviso(previa.error); return; }
    if (!this._el.chk || !this._el.chk.checked) {
      this._aviso('marque la casilla: ha verificado en persona el poste y la radio caida.');
      return;
    }
    if (!c.state.pinVerificado) { c.pedirPin(() => this._pulsar(punta)); return; }
    const d0 = new Date();
    if (!c.enviarComandoFirmware('SET_RTC', c.fechaLocalISO(d0) + ',' + c.horaLocal24(d0))) return;
    c.addEvent('cyan', 'Degradado con testigo: orden SET_RTC enviada con la hora del ' +
                       'telefono. El testigo sale con su acuse, con su rechazo o a los ' + (this.ESPERA_RTC_MS / 1000) + ' s.');
    this._espera = { punta, timer: setTimeout(() => {
      this._espera = null;
      this._aviso('el puente no acuso la hora en ' + (this.ESPERA_RTC_MS / 1000) + ' s. El testigo sale ' +
                  'igual: el poste valida `ahora` con su propio reloj.');
      this._enviarTestigo(punta);
    }, this.ESPERA_RTC_MS) };
    this.render();
  },

  _enviarTestigo(punta) {
    const c = this.ctx;
    if (c.state.node !== punta) { this._aviso('el poste cambio antes de enviar: no se envia.'); return; }
    const ahoraD = new Date();  // `ahora` se LEE otra vez, justo antes de enviar
    const o = this._componerPara(punta, ahoraD);
    if (o.error) { this._aviso(o.error); return; }
    if (!c.enviarComandoFirmware('SET_MODO:DEG_T', o.args)) return;
    if (punta === 'MAESTRO') {
      this.guardar({ estado: 'ENVIADO', inicio: o.inicio, inicioMs: o.inicioMs,
                     verde: o.verde, despeje: o.despeje, serie: c.state.serie || null,
                     emitidoMs: ahoraD.getTime(), emitido: o.ahora });
    }
    c.addEvent('cyan', 'Tecnico: orden ' + this.ORDEN + ':' + o.args + ' (DEGRADADO CON ' +
                       'TESTIGO) enviada al ' + (punta === 'MAESTRO' ? 'POSTE 1' : 'POSTE 2') +
                       '. Esperando respuesta: el equipo puede rechazarla y dira por que.');
    c.showToast('Testigo enviado: espere la respuesta del equipo');
    this.render();
  },
};

if (typeof module !== 'undefined' && module.exports) {
  module.exports = Testigo;
}
