// ===== js/interruptores.js =====
// LOS INTERRUPTORES DE ADMINISTRACION DE SPEC_8 6 (D-94) Y LAS CAMARAS DEGRADADAS DE SPEC_8 9 (D-166, D-169):
// sacar las camaras del veto (las dos, o una: C o D), sacar la barrera de servicio y elegir que hace la barrera con
// una camara mala o con dos (MALA1/MALA2: BAJA o ARRIBA). ORDENES de la app al poste conectado, sin radio.
//
// DONDE VIVEN: en Ajustes > "Averias" (#tab-diag, admin-only), asi que solo se ven tras la clave de
// administracion (D-56); y como toda orden, salen con el PIN del equipo. CONSULTA_VETO sale sin PIN al abrir el
// plegado y tras cada OK: lo que se pinta (#veto-estado) es lo que contesta el equipo, nunca lo pulsado.
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
      return { tono: l ? 'cyan' : 'amber', texto: 'Equipo: ' + txt, toast: 'Cámaras: estado leído del equipo' };
    }
    const p = data && this._partir(data.CMD);
    if (!p) return null;
    const t = this.ORDENES[p.orden];
    if (data.RESULT === 'SIN_CAMBIO') return t.SIN_CAMBIO;
    if (data.RESULT !== 'OK') return null;
    if (p.orden !== 'SET_BARRERA_SERVICIO') this._consultar();   // lo pintado lo dice el equipo
    const valor = p.valor || this._pedido[p.orden];
    return t[valor] || t.SIN_VALOR;
  },

  rechazo(data) {
    const p = data && this._partir(data.CMD);
    return p ? (this.RECHAZOS[data.DESC] || null) : null;
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

  _enviar(c, orden, valor) {
    if (!c.state.pinVerificado) { c.pedirPin(() => this._enviar(c, orden, valor)); return; }
    if (!c.enviarComandoFirmware(orden, valor)) return;
    this._pedido[orden] = valor;
    c.addEvent('cyan', 'Técnico: orden ' + orden + ':' + valor + ' enviada al equipo. Espere el acuse.');
  },

  iniciar(c) {
    this._c = c;
    if (typeof document === 'undefined') return;
    document.querySelectorAll('[data-interruptor]').forEach(btn => {
      btn.addEventListener('click', () => {
        const p = this._partir(btn.getAttribute('data-interruptor'));
        if (!p || !p.valor) return;
        const t = this.ORDENES[p.orden];
        const dl = (t.dialogos && t.dialogos[p.valor]) || (p.valor === 'OFF' && t.dialogo);
        if (!dl) { this._enviar(c, p.orden, p.valor); return; }   // lo de fabrica, sin preguntar
        Dialogo.confirmar(dl.titulo, dl.texto, dl.accion, () => this._enviar(c, p.orden, p.valor));
      });
    });
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
