// ===== js/interruptores.js =====
// LOS INTERRUPTORES DE ADMINISTRACION DE SPEC_8 6 (D-94) Y LAS CAMARAS DEGRADADAS DE SPEC_8 9 (D-166, D-169):
// sacar las camaras del veto (las dos, o una: C o D), sacar la barrera de servicio y elegir que hace la barrera con
// una camara mala o con dos (MALA1/MALA2: BAJA o ARRIBA). ORDENES de la app al poste conectado, sin radio.
//
// DONDE VIVEN: en Ajustes > "Averias" (#tab-diag, admin-only), asi que solo se ven tras la clave de
// administracion (D-56); y como toda orden, salen con el PIN del equipo. CONSULTA_VETO sale sin PIN al abrir el
// plegado y tras cada OK: lo que se pinta (#veto-estado) es lo que contesta el equipo, nunca lo pulsado.
// D-177 (SPEC_4 9.16 (c)): cinco selectores preseleccionados con esa consulta y un «Aplicar cambios» que manda solo lo
// que difiere, en cola (la siguiente al acuse de la anterior); las mismas ordenes, dialogos y textos de abajo.
//
// LO QUE NO ES DE FABRICA SE PREGUNTA ANTES (SPEC_8 6.1): OFF y BAJA. ON y ARRIBA son el lado seguro y salen sin
// dialogo. La consecuencia se dice con las palabras de las tablas de SPEC_8 6 y 9.3.
//
// LA RESPUESTA LA DICE EL EQUIPO, NO EL BOTON (molde SET_TIEMPOS, CLAUDE.md 2): OK solo si lo
// releido es lo pedido; SIN_CAMBIO si ya estaba; $ERR FORMATO_INVALIDO o RESPALDO_INVALIDO (D-148).
// El firmware devuelve el CMD con su valor (CMD:SET_VETO_CAMARA:C:OFF); sin el, se usa el ultimo valor pedido.
//
// EL AVISO DE ARRANQUE (D-152): un $EVENT de una sola vez si al arrancar sigue puesta alguna bandera. Se reconoce
// por el NOMBRE en DETALLE (interruptores_avisoArranque()): ARRANQUE_CAM_NOVETO, ..._CAM_NOVETO_C o _D (D-166),
// ARRANQUE_BARRERA_FUERA, o juntos. Si cambia, la linea sale en crudo por el camino generico.
//
// Sin jerga en lo que ve el operario (D-80): toasts y dialogos no citan ordenes ni campos.

const Interruptores = {
  CONSULTA: 'CONSULTA_VETO',
  ORDENES: {
    'SET_VETO_CAMARA': {
      dialogo: {
        titulo: 'Sacar las cámaras del veto',
        texto: 'Solo si la cámara no deja bajar la barrera. Saca del veto a las dos cámaras de este ' +
               'poste: la barrera vuelve a trabajar, sube con el verde y baja tras el rojo con su ' +
               'retardo, aunque haya algo debajo del brazo.',
        accion: 'Sacar del veto'
      },
      dialogos: {
        'C:OFF': { titulo: 'Sacar la cámara C del veto',
          texto: 'Solo si la cámara C está averiada. Este poste deja de mirarla: ni retiene la barrera ni pide ' +
                 'verde. La cámara D sigue reteniendo la barrera si ve algo debajo.', accion: 'Sacar la C' },
        'D:OFF': { titulo: 'Sacar la cámara D del veto',
          texto: 'Solo si la cámara D está averiada. Este poste deja de mirarla: ni retiene la barrera ni pide ' +
                 'verde. La cámara C sigue reteniendo la barrera si ve algo debajo.', accion: 'Sacar la D' }
      },
      OFF: { tono: 'amber',
        texto: 'Equipo: las cámaras de este poste quedan FUERA DEL VETO. La barrera baja tras el rojo ' +
               'aunque haya algo debajo. Cuando la cámara vuelva a ver bien, devuelva el veto.',
        toast: 'Cámaras fuera del veto en este poste' },
      ON: { tono: 'green',
        texto: 'Equipo: veto de las cámaras DEVUELTO. La barrera vuelve a esperar a que no haya nada ' +
               'debajo del brazo para bajar.',
        toast: 'Veto de las cámaras devuelto' },
      'C:OFF': { tono: 'amber',
        texto: 'Equipo: la cámara C queda FUERA: no retiene la barrera ni pide verde; la D sigue mirando. ' +
               'Qué falta: reparar o reapuntar la C y devolverle el veto.', toast: 'Cámara C fuera del veto' },
      'D:OFF': { tono: 'amber',
        texto: 'Equipo: la cámara D queda FUERA: no retiene la barrera ni pide verde; la C sigue mirando. ' +
               'Qué falta: reparar o reapuntar la D y devolverle el veto.', toast: 'Cámara D fuera del veto' },
      'C:ON': { tono: 'green', texto: 'Equipo: la cámara C vuelve al veto y a pedir verde.',
        toast: 'Cámara C con veto otra vez' },
      'D:ON': { tono: 'green', texto: 'Equipo: la cámara D vuelve al veto y a pedir verde.',
        toast: 'Cámara D con veto otra vez' },
      SIN_VALOR: { tono: 'cyan',
        texto: 'Equipo: orden del veto de las cámaras aplicada. La fila CÁMARAS dice si quedó sin veto.',
        toast: 'Veto de las cámaras: orden aplicada' },
      SIN_CAMBIO: { tono: 'amber',
        texto: 'Equipo: el veto de las cámaras ya estaba como lo pidió: no se cambió nada.',
        toast: 'Ya estaba así: nada que cambiar' },
      arranque: 'Al arrancar, este poste sigue con las cámaras FUERA DEL VETO: la barrera baja aunque ' +
                'haya algo debajo. Si la cámara ya ve bien, devuelva el veto desde Ajustes.'
    },
    'SET_BARRERA_SERVICIO': {
      dialogo: {
        titulo: 'Sacar la barrera de servicio',
        texto: 'Solo si la barrera está rota. El brazo sube AHORA y se queda arriba también con la luz ' +
               'en rojo, hasta volver a ponerla en servicio: el cruce queda solo con el semáforo.',
        accion: 'Sacar de servicio'
      },
      OFF: { tono: 'amber',
        texto: 'Equipo: barrera FUERA DE SERVICIO. El brazo queda arriba, también con la luz en rojo, ' +
               'hasta volver a ponerla en servicio. El cruce sigue solo con el semáforo.',
        toast: 'Barrera fuera de servicio: brazo arriba' },
      ON: { tono: 'green',
        texto: 'Equipo: barrera EN SERVICIO otra vez. El brazo vuelve a obedecer a la luz: sube con el ' +
               'verde y baja tras el rojo.',
        toast: 'Barrera en servicio otra vez' },
      SIN_VALOR: { tono: 'cyan',
        texto: 'Equipo: orden de la barrera aplicada. La fila de la pluma dice si quedó fuera de uso.',
        toast: 'Barrera: orden aplicada' },
      SIN_CAMBIO: { tono: 'amber',
        texto: 'Equipo: la barrera ya estaba como lo pidió: no se cambió nada.',
        toast: 'Ya estaba así: nada que cambiar' },
      arranque: 'Al arrancar, este poste sigue con la barrera FUERA DE SERVICIO: el brazo está arriba ' +
                'también en rojo. Si ya está reparada, póngala en servicio desde Ajustes.'
    },
    // D-169 (SPEC_8 9.2): 1 = una cámara mala y queda otra mirando; 2 = no queda ninguna. Fábrica ARRIBA.
    'SET_CAMARA_MALA': {
      dialogos: {
        '1:BAJA': { titulo: 'Con una cámara mala: bajar',
          texto: 'Con una cámara averiada, la barrera bajará tras el rojo si la otra no ve nada, aunque la ' +
                 'averiada viera algo. Úselo solo si las dos cámaras miran el mismo brazo.', accion: 'Bajar' },
        '2:BAJA': { titulo: 'Sin ninguna cámara: bajar',
          texto: 'Sin ninguna cámara que mire, la barrera bajará tras el rojo SIN MIRAR lo que hay debajo.',
          accion: 'Bajar sin mirar' }
      },
      '1:BAJA': { tono: 'amber', texto: 'Equipo: con UNA cámara mala la barrera baja tras el rojo si la otra no ' +
        've nada.', toast: 'Una cámara mala: la barrera baja' },
      '1:ARRIBA': { tono: 'green', texto: 'Equipo: con UNA cámara mala la barrera se queda arriba en el rojo ' +
        '(de fábrica) hasta reparar o sacar la cámara.', toast: 'Una cámara mala: barrera arriba' },
      '2:BAJA': { tono: 'amber', texto: 'Equipo: sin ninguna cámara que mire la barrera baja tras el rojo sin ' +
        'mirar.', toast: 'Sin cámaras: la barrera baja' },
      '2:ARRIBA': { tono: 'green', texto: 'Equipo: sin ninguna cámara que mire la barrera se queda arriba en el ' +
        'rojo (de fábrica).', toast: 'Sin cámaras: barrera arriba' },
      SIN_VALOR: { tono: 'cyan', texto: 'Equipo: orden de cámara mala aplicada.', toast: 'Cámara mala: aplicada' },
      SIN_CAMBIO: { tono: 'amber', texto: 'Equipo: ya estaba como lo pidió: no se cambió nada.',
        toast: 'Ya estaba así: nada que cambiar' }
    }
  },

  RECHAZOS: {
    // D-148: el arranque encontro el respaldo invalido y lo borro. Lo que no sobreviviria al corte no se
    // pone; lo de fabrica si se acepta, asi que esto solo llega con OFF o BAJA.
    RESPALDO_INVALIDO: {
      texto: 'El equipo NO cambió nada: esta placa pierde sus ajustes en cada corte de luz (sin pila de ' +
             'respaldo o gastada). Sigue como de fábrica: las dos cámaras vetan la barrera y la barrera en ' +
             'servicio. Que falta: poner o cambiar la pila de la placa y repetir; reiniciar no basta.',
      toast: 'No se cambió: la placa no tiene pila de respaldo'
    },
    // SPEC_8 6.1 (D-149): escrito y releido, lo leido no es lo pedido. No es NO_QUEDO_PUESTA (reloj, textos_err.js).
    NO_QUEDO_PUESTO: {
      texto: 'El equipo NO lo dejó puesto: después de escribirlo lo volvió a leer y no es lo pedido. ' +
             'Mire la barrera y las cámaras en la pantalla, repita la orden y, si vuelve a pasar, avise.',
      toast: 'No quedó puesto: repita la orden y, si se repite, avise'
    },
    FORMATO_INVALIDO: {
      texto: 'El equipo no entendió la orden y no se ejecutó nada. Si pasa siempre, la app y el ' +
             'equipo no hablan la misma versión: avise.',
      toast: 'El equipo no entendió la orden'
    }
  },

  // CONSULTA_VETO (SPEC_8 9.5): C<1 con veto|0 fuera><O|?|C|P>_D.._M1<A|B>_M2<A|B>.
  ESTADO: { O: 'bien', '?': 'sin detección aún', C: 'CIEGA (sin detecciones)', P: 'PEGADA (contacto fijo)' },

  // Ultimo valor pedido por orden, para leer un acuse que vuelve sin argumento.
  _pedido: {},

  _partir(cmd) {
    const t = String(cmd || '');
    const sep = t.indexOf(':');
    const orden = sep > 0 ? t.slice(0, sep) : t;
    const valor = sep > 0 ? t.slice(sep + 1) : null;
    return this.ORDENES[orden] ? { orden, valor } : null;
  },

  // RESULT de CONSULTA_VETO -> { texto, falta }; null si no casa (se pinta en crudo).
  leerConsulta(result) {
    const m = /^C([01])([O?CP])_D([01])([O?CP])_M1([AB])_M2([AB])$/.exec(String(result || ''));
    if (!m) return null;
    const cam = (x, veto, e) => 'Cámara ' + x + ': ' + (veto === '1' ? 'con veto' : 'FUERA del veto') + ', ' +
                                this.ESTADO[e];
    const mala = (v) => (v === 'A' ? 'barrera arriba' : 'la barrera baja');
    const falta = [];
    [['C', m[1], m[2]], ['D', m[3], m[4]]].forEach(([x, veto, e]) => {
      const malaX = e === 'C' || e === 'P';
      if (veto === '1' && malaX) falta.push('revisar la cámara ' + x + ' (' + this.ESTADO[e] + ') o sacarla del veto');
      else if (veto === '0' && malaX) falta.push('reparar la cámara ' + x + ' y devolverle el veto');
      else if (veto === '0') falta.push('si la cámara ' + x + ' ya ve bien, devolverle el veto');
    });
    return { texto: cam('C', m[1], m[2]) + ' · ' + cam('D', m[3], m[4]) + ' · Con una cámara mala: ' + mala(m[5]) +
                    ' · Sin ninguna: ' + mala(m[6]) + '.',
             falta: 'Qué falta: ' + (falta.length ? falta.join('; ') : 'nada, las dos cámaras miran') + '.' };
  },

  acuse(data) {
    if (data && data.CMD === this.CONSULTA) {
      const l = this.leerConsulta(data.RESULT);
      const txt = l ? l.texto + ' ' + l.falta : 'El equipo contestó "' + (data.RESULT || '') + '" y esta app no ' +
                  'lo reconoce. Qué falta: avisar; la app y el equipo no hablan la misma versión.';
      this._pintar(txt);
      if (l) this._leer(data.RESULT);
      return { tono: l ? 'cyan' : 'amber', texto: 'Equipo: ' + txt, toast: 'Cámaras: estado leído del equipo' };
    }
    const p = data && this._partir(data.CMD);
    if (!p) return null;
    const t = this.ORDENES[p.orden];
    const valor = p.valor || this._pedido[p.orden];
    if (data.RESULT === 'OK' || data.RESULT === 'SIN_CAMBIO') this._hecho(p.orden, valor);
    if (data.RESULT === 'SIN_CAMBIO') return t.SIN_CAMBIO;
    if (data.RESULT !== 'OK') return null;
    if (p.orden !== 'SET_BARRERA_SERVICIO') this._consultar();   // lo pintado lo dice el equipo
    return t[valor] || t.SIN_VALOR;
  },

  rechazo(data) {
    const p = data && this._partir(data.CMD);
    if (!p) return null;
    const r = this.RECHAZOS[data.DESC] || null, resto = this._cortar(p.orden, 'el equipo la rechazó', !!r);
    return r && resto ? { texto: r.texto + ' ' + resto, toast: r.toast } : r;
  },

  evento(data) {
    if (!data) return null;
    const txt = String(data.ORIGEN || '') + ' ' + String(data.DETALLE || '');
    const partes = [];
    const una = /CAM_NOVETO_([CD])(?![A-Za-z0-9])/.exec(txt);
    if (una) {
      partes.push('Al arrancar, este poste sigue con la cámara ' + una[1] + ' FUERA DEL VETO: no retiene la ' +
                  'barrera ni pide verde. Si ya ve bien, devuélvale el veto desde Ajustes.');
    } else if (/CAM_NOVETO/.test(txt)) {
      partes.push(this.ORDENES.SET_VETO_CAMARA.arranque);
    }
    if (/BARRERA_FUERA/.test(txt)) partes.push(this.ORDENES.SET_BARRERA_SERVICIO.arranque);
    if (!partes.length) return null;
    return { tono: 'amber', texto: partes.join(' '), toast: 'Este poste arrancó con un interruptor puesto' };
  },

  _pintar(txt) {
    const el = typeof document !== 'undefined' && document.getElementById('veto-estado');
    if (el) el.textContent = txt;
  },

  _consultar() {
    if (this._c) this._c.enviarComandoFirmware(this.CONSULTA);
  },

  // ---- D-177 (SPEC_4 9.16 (c)): cinco selectores y un «Aplicar cambios» en vez de doce botones ----------------
  // Cada selector: [clave, rotulo, orden, prefijo del valor, [fabrica, rotulo], [puesto, rotulo]]. El blanco es «sin
  // leer»: lo que el equipo aun no ha dicho (sin CONSULTA_VETO contestada, firmware sin ella, otro poste).
  SELECTORES: [
    ['C', 'Cámara C', 'SET_VETO_CAMARA', 'C:', ['ON', 'Retiene la barrera (de fábrica)'], ['OFF', 'Fuera: no la mira']],
    ['D', 'Cámara D', 'SET_VETO_CAMARA', 'D:', ['ON', 'Retiene la barrera (de fábrica)'], ['OFF', 'Fuera: no la mira']],
    ['M1', 'Con una cámara mala', 'SET_CAMARA_MALA', '1:', ['ARRIBA', 'Barrera arriba (de fábrica)'],
      ['BAJA', 'La barrera baja']],
    ['M2', 'Sin ninguna cámara', 'SET_CAMARA_MALA', '2:', ['ARRIBA', 'Barrera arriba (de fábrica)'],
      ['BAJA', 'Baja sin mirar']],
    ['B', 'Barrera', 'SET_BARRERA_SERVICIO', '', ['ON', 'En servicio (de fábrica)'],
      ['OFF', 'Fuera: brazo arriba, también en rojo']]
  ],
  FRASE: { 'SET_VETO_CAMARA:OFF': 'sacar las dos cámaras del veto', 'SET_VETO_CAMARA:ON': 'devolver el veto a las dos',
    'SET_VETO_CAMARA:C:OFF': 'sacar la cámara C', 'SET_VETO_CAMARA:C:ON': 'devolver la C',
    'SET_VETO_CAMARA:D:OFF': 'sacar la cámara D', 'SET_VETO_CAMARA:D:ON': 'devolver la D',
    'SET_CAMARA_MALA:1:BAJA': 'con una mala: bajar', 'SET_CAMARA_MALA:1:ARRIBA': 'con una mala: arriba',
    'SET_CAMARA_MALA:2:BAJA': 'sin ninguna: bajar', 'SET_CAMARA_MALA:2:ARRIBA': 'sin ninguna: arriba',
    'SET_BARRERA_SERVICIO:OFF': 'sacar la barrera de servicio', 'SET_BARRERA_SERVICIO:ON': 'barrera en servicio' },
  PLAZO_MS: 8000,             // sin acuse en este plazo, la cola se corta y se dice
  _leido: {},                 // clave -> valor que dijo el equipo (null: sin leer)
  _cola: [],                  // [{orden, valor}] que faltan por salir
  _enVuelo: null,             // { orden, valor, timer } la que espera su acuse

  _sel(k) { return typeof document !== 'undefined' && document.getElementById('sel-averia-' + k); },
  _frase(o) { return this.FRASE[o.orden + ':' + o.valor] || o.orden + ':' + o.valor; },

  // RESULT de CONSULTA_VETO -> lo leido; la barrera, de la pluma del $STATUS (no viene en la consulta). Un selector
  // que la persona ya cambio no se pisa.
  _leer(result) {
    const m = /^C([01])[O?CP]_D([01])[O?CP]_M1([AB])_M2([AB])$/.exec(String(result || ''));
    if (!m) return;
    const pl = this._c && this._c.state.pluma;
    const nuevo = { C: m[1] === '1' ? 'ON' : 'OFF', D: m[2] === '1' ? 'ON' : 'OFF',
                    M1: m[3] === 'A' ? 'ARRIBA' : 'BAJA', M2: m[4] === 'A' ? 'ARRIBA' : 'BAJA',
                    B: this._leido.B || (pl ? (pl === 'FUERA' ? 'OFF' : 'ON') : null) };
    Object.keys(nuevo).forEach(k => {
      const s = this._sel(k);
      if (s && (!s.value || s.value === (this._leido[k] || ''))) s.value = nuevo[k] || '';
    });
    this._leido = nuevo;
    this._pintarAplicar();
  },

  // Otro poste (o ninguno): lo leido era del anterior. Lo llama Instalacion.alStatus(), en el mismo momento.
  alStatus(node) {
    if (node === this._nodo) return;
    this._nodo = node; this._leido = {};
    this.SELECTORES.forEach(([k]) => { const s = this._sel(k); if (s) s.value = ''; });
    if (this._enVuelo) this._cortar(this._enVuelo.orden, 'cambió el poste');
    this._pintarAplicar();
  },

  // Lo elegido que difiere de lo leido, como ordenes. C y D al mismo valor: UNA orden, la de las dos.
  _cambios() {
    const v = {}, out = [];
    this.SELECTORES.forEach(([k]) => {
      const s = this._sel(k);
      if (s && s.value && s.value !== this._leido[k]) v[k] = s.value;
    });
    if (v.C && v.C === v.D) out.push({ orden: 'SET_VETO_CAMARA', valor: v.C });
    else ['C', 'D'].forEach(k => { if (v[k]) out.push({ orden: 'SET_VETO_CAMARA', valor: k + ':' + v[k] }); });
    if (v.M1) out.push({ orden: 'SET_CAMARA_MALA', valor: '1:' + v.M1 });
    if (v.M2) out.push({ orden: 'SET_CAMARA_MALA', valor: '2:' + v.M2 });
    if (v.B) out.push({ orden: 'SET_BARRERA_SERVICIO', valor: v.B });
    return out;
  },

  _pintarAplicar() {
    const b = typeof document !== 'undefined' && document.getElementById('btn-averias-aplicar');
    if (b) b.disabled = !this._cambios().length;
  },
  _decir(txt) {
    const el = typeof document !== 'undefined' && document.getElementById('averias-cola');
    if (el) el.textContent = txt;
  },

  // Lo que no es de fabrica (OFF, BAJA) se pregunta antes, en UN dialogo; despues UN PIN y la cola.
  _aplicar(c) {
    if (this._enVuelo) { c.showToast('Espere la respuesta del equipo a lo anterior'); return; }
    const lista = this._cambios();
    if (!lista.length) return;
    const dls = lista.map(o => { const t = this.ORDENES[o.orden];
      return (t.dialogos && t.dialogos[o.valor]) || (o.valor === 'OFF' && t.dialogo) || null; }).filter(Boolean);
    const ir = () => this._enviarCola(c, lista);
    if (!dls.length) { ir(); return; }   // lo de fabrica, sin preguntar
    if (dls.length === 1) { Dialogo.confirmar(dls[0].titulo, dls[0].texto, dls[0].accion, ir); return; }
    Dialogo.confirmar('Aplicar ' + lista.length + ' cambios', dls.map(d => d.titulo + ': ' + d.texto).join(' '),
                      'Aplicar', ir);
  },
  _enviarCola(c, lista) {
    if (!c.state.pinVerificado) { c.pedirPin(() => this._enviarCola(c, lista)); return; }
    this._cola = lista.slice();
    this._decir('');
    this._siguiente();
  },
  _siguiente() {
    const c = this._c, o = this._cola.shift();
    if (!o) { this._pintarAplicar(); return; }
    if (!c.enviarComandoFirmware(o.orden, o.valor)) {
      this._cola.unshift(o); this._cortar(null, 'no salió'); return;
    }
    this._pedido[o.orden] = o.valor;
    c.addEvent('cyan', 'Técnico: orden ' + o.orden + ':' + o.valor + ' enviada al equipo. Espere el acuse.');
    this._enVuelo = { orden: o.orden, valor: o.valor,
                      timer: setTimeout(() => this._cortar(o.orden, 'el equipo no contestó'), this.PLAZO_MS) };
    this._pintarAplicar();
  },
  // OK o SIN_CAMBIO de la que esperaba: queda leida y sale la siguiente.
  _hecho(orden, valor) {
    const v = this._enVuelo;
    if (!v || v.orden !== orden) return;
    clearTimeout(v.timer); this._enVuelo = null;
    const p = String(valor || v.valor).split(':'), x = p.pop();
    const ks = orden === 'SET_BARRERA_SERVICIO' ? ['B']
      : orden === 'SET_CAMARA_MALA' ? ['M' + p[0]] : p[0] ? [p[0]] : ['C', 'D'];
    ks.forEach(k => { this._leido[k] = x; });
    this._siguiente();
  },
  // Rechazo, plazo o envio fallido: la cola se corta y se dice que no se aplico. Devuelve la frase o ''. `callar`:
  // el que llama la pone en su propio texto (el rechazo traducido).
  _cortar(orden, motivo, callar) {
    const v = this._enVuelo;
    if (orden && (!v || v.orden !== orden)) return '';
    const no = (v ? [v] : []).concat(this._cola);
    if (v) clearTimeout(v.timer);
    this._enVuelo = null; this._cola = [];
    this._pintarAplicar();
    if (!no.length) return '';
    const txt = 'No se aplicaron (' + motivo + '): ' + no.map(o => this._frase(o)).join('; ') + '.';
    this._decir(txt);
    if (!callar && this._c) { this._c.addEvent('amber', 'Averías: ' + txt); this._c.showToast(txt); }
    return txt;
  },

  _armar(cont, c) {
    cont.innerHTML = this.SELECTORES.map(([k, nom, , , f, p]) => '<p class="ajuste"><label for="sel-averia-' + k +
      '">' + nom + '</label> <select class="form-input" id="sel-averia-' + k + '"><option value="">' +
      '— sin leer: elija —</option><option value="' + f[0] + '">' + f[1] + '</option><option value="' + p[0] +
      '">' + p[1] + '</option></select></p>').join('') +
      '<small class="hint">En blanco: el equipo aún no lo ha dicho; elija y aplique igual.</small>' +
      '<button class="ctrl-btn" id="btn-averias-aplicar" disabled><strong>Aplicar cambios</strong></button>' +
      '<p class="card-sub" id="averias-cola"></p>';
    cont.addEventListener('change', () => this._pintarAplicar());
    document.getElementById('btn-averias-aplicar').addEventListener('click', () => this._aplicar(c));
  },

  iniciar(c) {
    this._c = c;
    if (typeof document === 'undefined') return;
    const cont = document.getElementById('interruptores');
    if (cont) this._armar(cont, c);
    // Se consulta al ABRIR; el navegador puede dar mas de un toggle por apertura: uno solo por apertura.
    const det = document.getElementById('averias');
    if (det) {
      det.addEventListener('toggle', () => {
        if (!det.open) { this._abierto = false; return; }
        if (this._abierto) return;
        this._abierto = true; this._consultar();
      });
    }
  }
};

if (typeof module !== 'undefined' && module.exports) {
  module.exports = Interruptores;
}
