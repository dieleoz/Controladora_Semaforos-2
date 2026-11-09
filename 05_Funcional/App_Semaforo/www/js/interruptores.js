// ===== js/interruptores.js =====
// LOS DOS INTERRUPTORES DE ADMINISTRACION DE SPEC_8 6 (D-94): sacar las camaras del veto y
// sacar la barrera de servicio. Son ORDENES de la app al poste conectado, no viajan por radio.
//
// DONDE VIVEN: en Diagnostico (#tab-diag, admin-only), asi que solo se ven tras la clave de
// administracion (D-56); y como toda orden, salen con el PIN del equipo.
//
// OFF ES EL INTERRUPTOR PUESTO Y SE PREGUNTA ANTES (SPEC_8 6.1). ON es el lado seguro y sale sin
// dialogo. La consecuencia se dice con las palabras de la tabla de SPEC_8 6; la de la barrera,
// confirmada ARRIBA por el responsable (D-96): el brazo sube ya y se queda arriba tambien en rojo.
//
// LA RESPUESTA LA DICE EL EQUIPO, NO EL BOTON (molde SET_TIEMPOS, CLAUDE.md 2): OK solo si lo
// releido es lo pedido; SIN_CAMBIO si ya estaba; $ERR FORMATO_INVALIDO o RESPALDO_INVALIDO (D-148).
// La spec no fija si el CMD del acuse vuelve con el argumento o sin el (SET_TIEMPOS vuelve sin):
// se aceptan las dos formas y, sin argumento, se usa el ultimo valor pedido desde esta app.
//
// EL AVISO DE ARRANQUE (D-152): un $EVENT de una sola vez si al arrancar sigue puesta alguna de las
// dos banderas de D-95. La spec no fija su literal: se reconoce por el NOMBRE de la bandera en
// DETALLE del firmware (ARRANQUE_CAM_NOVETO, ARRANQUE_BARRERA_FUERA, o los dos juntos). Si cambia, la linea sale en crudo
// por el camino generico, que es lo que impide que se pierda.
//
// Sin jerga en lo que ve el operario (D-80): toasts y dialogos no citan ordenes ni campos.

const Interruptores = {
  ORDENES: {
    'SET_VETO_CAMARA': {
      dialogo: {
        titulo: 'Sacar las cámaras del veto',
        texto: 'Solo si la cámara no deja bajar la barrera. Saca del veto a las dos cámaras de este ' +
               'poste: la barrera vuelve a trabajar, sube con el verde y baja tras el rojo con su ' +
               'retardo, aunque haya algo debajo del brazo.',
        accion: 'Sacar del veto'
      },
      OFF: { tono: 'amber',
        texto: 'Equipo: las cámaras de este poste quedan FUERA DEL VETO. La barrera baja tras el rojo ' +
               'aunque haya algo debajo. Cuando la cámara vuelva a ver bien, devuelva el veto.',
        toast: 'Cámaras fuera del veto en este poste' },
      ON: { tono: 'green',
        texto: 'Equipo: veto de las cámaras DEVUELTO. La barrera vuelve a esperar a que no haya nada ' +
               'debajo del brazo para bajar.',
        toast: 'Veto de las cámaras devuelto' },
      SIN_VALOR: { tono: 'cyan',
        texto: 'Equipo: orden del veto de las cámaras aplicada. La fila CÁMARAS dice si quedó sin veto.',
        toast: 'Veto de las cámaras: orden aplicada' },
      SIN_CAMBIO: { tono: 'amber',
        texto: 'Equipo: el veto de las cámaras ya estaba como lo pidió: no se cambió nada.',
        toast: 'Ya estaba así: nada que cambiar' },
      arranque: 'Al arrancar, este poste sigue con las cámaras FUERA DEL VETO: la barrera baja aunque ' +
                'haya algo debajo. Si la cámara ya ve bien, devuelva el veto desde Diagnóstico.'
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
                'también en rojo. Si ya está reparada, póngala en servicio desde Diagnóstico.'
    }
  },

  RECHAZOS: {
    // D-148: el arranque encontro el respaldo invalido y lo borro. Un interruptor que no
    // sobreviviria al corte no se pone; el ON si se acepta, asi que esto solo llega con OFF.
    RESPALDO_INVALIDO: {
      texto: 'El equipo NO lo puso: al arrancar encontró perdido lo que guarda con la pila (pila ' +
             'agotada, equipo nuevo o pila recién cambiada), y este interruptor no sobreviviría a un ' +
             'corte de luz. Compruebe la pila, reinicie el equipo una vez y repita.',
      toast: 'No se puso: revise la pila y reinicie el equipo'
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

  // Ultimo valor pedido por orden, para leer un acuse que vuelve sin argumento.
  _pedido: {},

  _partir(cmd) {
    const t = String(cmd || '');
    const sep = t.indexOf(':');
    const orden = sep > 0 ? t.slice(0, sep) : t;
    const valor = sep > 0 ? t.slice(sep + 1) : null;
    return this.ORDENES[orden] ? { orden, valor } : null;
  },

  acuse(data) {
    const p = data && this._partir(data.CMD);
    if (!p) return null;
    const t = this.ORDENES[p.orden];
    if (data.RESULT === 'SIN_CAMBIO') return t.SIN_CAMBIO;
    if (data.RESULT !== 'OK') return null;
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
    if (/CAM_NOVETO/.test(txt)) partes.push(this.ORDENES.SET_VETO_CAMARA.arranque);
    if (/BARRERA_FUERA/.test(txt)) partes.push(this.ORDENES.SET_BARRERA_SERVICIO.arranque);
    if (!partes.length) return null;
    return { tono: 'amber', texto: partes.join(' '), toast: 'Este poste arrancó con un interruptor puesto' };
  },

  _enviar(c, orden, valor) {
    if (!c.state.pinVerificado) { c.pedirPin(() => this._enviar(c, orden, valor)); return; }
    if (!c.enviarComandoFirmware(orden, valor)) return;
    this._pedido[orden] = valor;
    c.addEvent('cyan', 'Técnico: orden ' + orden + ':' + valor + ' enviada al equipo. Espere el acuse.');
  },

  iniciar(c) {
    if (typeof document === 'undefined') return;
    document.querySelectorAll('[data-interruptor]').forEach(btn => {
      btn.addEventListener('click', () => {
        const p = this._partir(btn.getAttribute('data-interruptor'));
        if (!p || (p.valor !== 'ON' && p.valor !== 'OFF')) return;
        if (p.valor === 'ON') { this._enviar(c, p.orden, p.valor); return; }
        const dl = this.ORDENES[p.orden].dialogo;
        Dialogo.confirmar(dl.titulo, dl.texto, dl.accion, () => this._enviar(c, p.orden, p.valor));
      });
    });
  }
};

if (typeof module !== 'undefined' && module.exports) {
  module.exports = Interruptores;
}
