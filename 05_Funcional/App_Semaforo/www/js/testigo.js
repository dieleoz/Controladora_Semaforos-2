// ===== js/testigo.js =====
// DEGRADADO CON TESTIGO (D-35): SPEC_4 §3.ter, SPEC_2 §7.bis, SPEC_6 A.1.bis y A.2.bis.
//
// Para cuando la radio entre postes no vuelve en semanas. UNA orden por poste,
//     SET_MODO:DEG_T:ahora,inicio,verde,despeje      (HH:MM:SS,HH:MM:SS,v,d; con PIN)
// D-59: v = minutos*60, con minutos enteros 3..15 elegidos en el campo (180..900 s).
// primero al MAESTRO y despues, con el MISMO testigo guardado aqui, al ESCLAVO.
//
// LO QUE ESTE MODULO NO HACE, Y ES A PROPOSITO:
//   - No pinta ningun modo. Pulsar es "orden enviada"; lo que el equipo hizo lo dicen su
//     $ACK (texto en ACK_TEXTO, js/textos_ack.js) o su $ERR (texto aqui, en MOTIVOS).
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
  // D-59: el verde se elige en minutos enteros 3..15 y viaja en segundos (min*60). Un firmware
  // anterior a D-59 solo admite 180: otro valor lo rechaza con FORMATO_INVALIDO.
  VERDE_S: 180,               // el unico verde que entiende el firmware viejo (3 min)
  VERDE_MIN_MIN: 3,
  VERDE_MAX_MIN: 15,
  VERDE_DEF_MIN: 3,
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
      'o despeje). No es algo que corrija el operario: anote el literal y avise.',
    'Verde fuera de rango (3-15 min)': 'Verde fuera de rango: el poste admite de 3 a 15 min. ' +
      'Corrija el verde en la app antes de reenviar.',
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
  // verdeMin omitido = VERDE_DEF_MIN; un campo vacio llega como '' y es error.
  componer(ahoraDate, trasladoMin, despeje, hora, verdeMin) {
    const t = Number(trasladoMin);
    const d = Number(despeje);
    const vm = verdeMin === undefined ? this.VERDE_DEF_MIN
      : (String(verdeMin).trim() === '' ? NaN : Number(verdeMin));
    const tope = this.MEDIO_DIA_S / 60;
    if (!Number.isInteger(t) || t < 1 || t > tope) {
      return { error: 'Traslado invalido: minutos enteros de 1 a ' + tope + '.' };
    }
    if (!Number.isInteger(vm) || vm < this.VERDE_MIN_MIN || vm > this.VERDE_MAX_MIN) {
      return { error: 'Verde invalido: minutos enteros de ' + this.VERDE_MIN_MIN + ' a ' +
                      this.VERDE_MAX_MIN + '.' };
    }
    const v = vm * 60;
    if (!Number.isInteger(d) || d < this.DESPEJE_MIN || d > this.DESPEJE_MAX) {
      return { error: 'Despeje fuera de rango (' + this.DESPEJE_MIN + '-' +
                      this.DESPEJE_MAX + ' s).' };
    }
    const ini = new Date(ahoraDate.getTime() + t * 60000);
    const ahora = hora(ahoraDate);
    const inicio = hora(ini);
    return { ahora, inicio, inicioMs: ini.getTime(), verde: v, despeje: d,
             args: ahora + ',' + inicio + ',' + v + ',' + d };
  },

  // Un verde guardado vale si este modulo pudo componerlo: multiplo de 60 en 180..900.
  _verdeValido(v) {
    return Number.isInteger(v) && v % 60 === 0 &&
           v >= this.VERDE_MIN_MIN * 60 && v <= this.VERDE_MAX_MIN * 60;
  },
  _min(v) { return (Number(v) / 60) + ' min'; },

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
  // D-59: el verde NUNCA sale del campo: otro que el del POSTE 1 da verdes cruzados.
  componerEsclavo(g, ahoraDate, hora) {
    if (!g || g.estado !== 'ACEPTADO') {
      return { error: 'No hay en este telefono un testigo ACEPTADO por el POSTE 1. ' +
                      'Hagalo primero en el Maestro.' };
    }
    if (this.vencido(g, ahoraDate, hora)) {
      return { error: 'El inicio del testigo (' + g.inicio + ') ya paso: no se envia. ' +
                      'Repita el testigo en el Maestro.' };
    }
    if (!this._verdeValido(g.verde)) {
      return { error: 'El testigo guardado no trae un verde valido (' + g.verde + '): no se ' +
                      'envia. Repita el testigo en el Maestro.' };
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
    // D-78: la app no ve el ciclo del POSTE 1 (D-60); la medida es el acuse. OK con enCaliente es entrada nueva.
    // El toast va tras el de ACK_TEXTO, que app.js pone despues de este enganche.
    if (node === 'MAESTRO' && g && g.enCaliente && data.RESULT === 'OK') {
      const t = 'el POSTE 1 cambio de ciclo en marcha: el POSTE 2 sigue con el anterior hasta aceptar este ' +
                'testigo; mantenga el paletero en los dos extremos';
      this.ctx.addEvent('red', 'Degradado con testigo: ' + t);
      setTimeout(() => this.ctx.showToast(t, 6000), 0);
    }
    if (node === 'MAESTRO' && g) {
      this.ctx.addEvent('red', 'Testigo del POSTE 1 guardado en este telefono: inicio ' +
        g.inicio + ', verde ' + this._min(g.verde) + ', despeje ' + g.despeje + ' s. Lleve el ' +
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
    const desc = String(data.DESC || '');
    // 14/10: el POSTE 2 llega tarde y el POSTE 1 ya va con otro ciclo (testigo puesto en caliente,
    // o este poste ya en Degradado con su ciclo anterior): puede haber verde en los dos sentidos.
    const vencidoE = /^(INICIO_VENCIDO|Inicio ya vencido)/.test(desc);
    if (node === 'ESCLAVO' && vencidoE && g && g.estado === 'ACEPTADO' &&
        (g.enCaliente || this.ctx.state.modo === 'DEGRADADO')) {
      return { texto: 'Verde cruzado posible: el POSTE 1 va con ' + this._min(g.verde) + ' y este con el ' +
                      'ciclo anterior. Que falta: mantenga el paletero; repita el testigo en el POSTE 1 y ' +
                      'traigalo aqui antes del inicio.',
               toast: 'Verde cruzado posible: mantenga el paletero' };
    }
    // D-59: un verde distinto de 180 rechazado por FORMATO_INVALIDO delata firmware anterior a D-59
    // (el nuevo contesta «Verde fuera de rango (3-15 min)», que va en MOTIVOS).
    const pedido = g ? Number(g.verde) : 0;
    if (pedido && pedido !== this.VERDE_S && desc === 'FORMATO_INVALIDO') {
      const falta = node === 'ESCLAVO'
        ? ' El POSTE 1 ya va con el testigo de ' + this._min(pedido) + '. Que falta: cargar el ' +
          'firmware nuevo en este poste y repetir aqui antes de las ' + g.inicio + ', o repetir ' +
          'el testigo en el POSTE 1 con Verde 3 min y traerlo otra vez. El firmware nuevo va en ' +
          'los dos postes.'
        : ' Que falta: cargar el firmware nuevo en los dos postes, o repetir con Verde 3 min.';
      return { texto: 'Este poste necesita el firmware nuevo para un verde distinto de 3 min ' +
                      '(pidio ' + this._min(pedido) + '; motivo ' + desc + '). NO ha entrado.' + falta,
               toast: 'Testigo rechazado: verde ' + this._min(pedido) + ' necesita firmware nuevo' };
    }
    const texto = this.MOTIVOS[desc];
    return texto ? { texto, toast: 'Testigo rechazado: ' + data.DESC } : null;
  },

  // ---- Pantalla ------------------------------------------------------------

  iniciar(ctx) {
    this.ctx = ctx;
    const $ = (id) => document.getElementById(id);
    this._el = {
      bloqueM: $('testigo-maestro'), bloqueE: $('testigo-esclavo'),
      traslado: $('num-testigo-traslado'), despeje: $('num-testigo-despeje'),
      verde: $('num-testigo-verde'),
      chk: $('chk-testigo-verificado'), btnM: $('btn-testigo-maestro'),
      btnE: $('btn-testigo-esclavo'), estado: $('testigo-estado'),
    };
    const el = this._el;
    if (el.despeje) {
      const conf = Number(ctx.state.tiempoDespejeSeg) || this.DESPEJE_MIN;
      el.despeje.value = Math.min(this.DESPEJE_MAX, Math.max(this.DESPEJE_MIN, conf));
    }
    if (el.traslado && !el.traslado.value) el.traslado.value = this.TRASLADO_DEF_MIN;
    if (el.verde && !el.verde.value) el.verde.value = this.VERDE_DEF_MIN;
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
            ', verde ' + this._min(g.verde) + ', despeje ' + g.despeje + ' s. Faltan ' +
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

  // Con el testigo del POSTE 1 ENVIADO y sin acuse, un reenvio NO recompone desde los campos:
  // reusa lo guardado (inicio, verde, despeje; solo `ahora` fresco). Un acuse tardio del primer
  // envio marcaria ACEPTADO un registro que el poste no tiene, y el POSTE 2 cruzaria verdes.
  _reenvio(ahoraD) {
    const g = this.leer();
    if (!g || g.estado !== 'ENVIADO' || this.vencido(g, ahoraD, this.ctx.horaLocal24)) return null;
    if (!this._verdeValido(g.verde)) return null;
    const ahora = this.ctx.horaLocal24(ahoraD);
    const el = this._el;
    const campos = this.componer(ahoraD, el.traslado && el.traslado.value,
                                 el.despeje && el.despeje.value, this.ctx.horaLocal24,
                                 el.verde ? el.verde.value : undefined);
    const cambiado = !!campos.error || campos.verde !== g.verde || campos.despeje !== g.despeje;
    return { ahora, inicio: g.inicio, inicioMs: g.inicioMs, verde: g.verde, despeje: g.despeje,
             args: ahora + ',' + g.inicio + ',' + g.verde + ',' + g.despeje, reenvio: true, cambiado };
  },

  _componerPara(punta, ahoraD) {
    const el = this._el;
    if (punta === 'MAESTRO') {
      const r = this._reenvio(ahoraD);
      if (r) return r;
    }
    return punta === 'MAESTRO'
      ? this.componer(ahoraD, el.traslado && el.traslado.value,
                      el.despeje && el.despeje.value, this.ctx.horaLocal24,
                      el.verde ? el.verde.value : undefined)
      : this.componerEsclavo(this.leer(), ahoraD, this.ctx.horaLocal24);
  },

  // Maestro y Esclavo comparten el camino: PIN, casilla, SET_RTC, su $ACK, `ahora` fresco.
  // 14/10 (responsable): con el POSTE 1 YA en Degradado se permite cambiar verde o despeje en
  // caliente, con riesgo aceptado: hasta que el POSTE 2 acepte el MISMO testigo los dos ciclos
  // difieren. Se pregunta si cambian respecto al testigo guardado, o si no hay con que comparar.
  _cambioEnCaliente(punta, previa) {
    if (punta !== 'MAESTRO' || this.ctx.state.modo !== 'DEGRADADO') return false;
    const g = this.leer();   // D-78: tambien si el guardado es de otra SERIE
    return !g || previa.verde !== g.verde || previa.despeje !== g.despeje ||
           !!(g.serie && this.ctx.state.serie && g.serie !== this.ctx.state.serie);
  },

  _pulsar(punta, confirmado) {
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
    if (!confirmado && this._cambioEnCaliente(punta, previa)) {
      // D-81 (SPEC_4 9.9 (e)): UN solo aviso, el de paleteros, con lo que exigian los dos; la barrera no afloja.
      const av = typeof AvisoDegradado !== 'undefined' ? AvisoDegradado : null;
      if (!av || !av.preguntar(this.ORDEN, 'Cambio de tiempos con el Degradado en marcha', 'Hasta que el POSTE 2 ' +
        'acepte el MISMO testigo, los dos postes llevan ciclos distintos y puede haber verde en los dos sentidos. ' +
        'Sin radio el otro poste no se entera y sigue dando verdes por su reloj. Ponga PALETEROS en los dos ' +
        'extremos ANTES de seguir y lleve el testigo al POSTE 2 antes de las ' + previa.inicio + '.',
        c.addEvent, () => this._pulsar(punta, true))) this._aviso('no se puede preguntar: no se envia.');
      return;
    }
    if (!c.state.pinVerificado) { c.pedirPin(() => this._pulsar(punta, confirmado)); return; }
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
    // En Degradado el envio pasa por el aviso de paleteros (js/aviso_degradado.js); al confirmarlo
    // se reintenta este envio, con `ahora` releido, como hace deg_fin.js.
    const av = typeof AvisoDegradado !== 'undefined' ? AvisoDegradado : null;
    if (av) av._reintento = () => this._enviarTestigo(punta);
    let salio;
    try { salio = c.enviarComandoFirmware('SET_MODO:DEG_T', o.args); } finally { if (av) av._reintento = null; }
    if (!salio) return;
    const previo = this.leer();
    const enCaliente = c.state.modo === 'DEGRADADO' ||
                       !!(o.reenvio && previo && previo.enCaliente);
    if (o.reenvio && o.cambiado) {
      this._aviso('se reenvia lo ya enviado (inicio ' + o.inicio + ', verde ' + this._min(o.verde) +
                  ', despeje ' + o.despeje + ' s), no lo que dicen ahora los campos. Para cambiarlo ' +
                  'espere el acuse o el rechazo del POSTE 1 y repita.');
    }
    if (punta === 'MAESTRO') {
      this.guardar({ estado: 'ENVIADO', inicio: o.inicio, inicioMs: o.inicioMs,
                     verde: o.verde, despeje: o.despeje, serie: c.state.serie || null,
                     emitidoMs: ahoraD.getTime(), emitido: o.ahora, enCaliente });
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
