// ===== js/diario_vista.js =====
// La VISTA del diario de ordenes de la pestana Depuracion: el resumen, la lista de tres
// lineas por orden, la nota de lo que el diario no sabe, y sus tres botones (exportar,
// copiar, vaciar). El DIARIO en si -lo que se anota y como se juzga- es DiarioOrdenes,
// en js/depuracion.js; esto solo lo pinta y lo saca del poste.
//
// Sale de app.js el 28/09 en un commit de CORTE, sin cambio de comportamiento: el
// texto de abajo es el mismo, linea por linea, con dos espacios mas de sangria. No es
// un modulo puro como avisos_equipo.js: necesita el estado de la app y tres funciones
// suyas, y se las pide app.js al instalarlo en vez de leerlas de globales:
//
//   state      el objeto de estado de app.js (cruce, nodo, serie, modo, luces, hora)
//   showToast  el aviso breve de app.js
//   addEvent   la anotacion en la lista de Eventos de app.js
//   horaDe     _horaDe(ms) de app.js, la hora de pared de una marca
//
// Devuelve { render }. app.js conserva renderDiario() como envoltorio, porque la llaman
// el envio de ordenes, la rama de $ACK, el conmutador de pestanas y el arranque.

const DiarioVista = {
  instalar(dep) {
    const state = dep.state;
    const showToast = dep.showToast;
    const addEvent = dep.addEvent;
    const _horaDe = dep.horaDe;

    // El diario de ordenes. Vive en la misma pestana que la cinta pero en su propia
    // tarjeta y con sus propias salidas: son dos registros distintos y se exportan por
    // separado, porque lo que se manda por WhatsApp es el diario y no los 300 bytes.
    const diarioResumenEl = document.getElementById('diario-resumen');
    const diarioListaEl = document.getElementById('diario-lista');
    const diarioNotaEl = document.getElementById('diario-nota');
    const diarioTextoEl = document.getElementById('diario-texto');
    const btnDiarioExport = document.getElementById('btn-diario-export');
    const btnDiarioCopiar = document.getElementById('btn-diario-copiar');
    const btnDiarioLimpiar = document.getElementById('btn-diario-limpiar');

    // ---- El diario de ordenes ----------------------------------------------
    //
    // TRES LINEAS POR ORDEN: QUE SE MANDO, QUE CONTESTO EL EQUIPO Y QUE SE VIO CAMBIAR.
    // Juntas y en ese orden, un defecto se lee sin diagnosticarlo -es N-42 en tres
    // renglones-. Separadas, hay que cruzar la cinta con la memoria de quien pulso.
    //
    // Cada una de las tres se pinta con SU estado, y cuatro de los estados posibles son
    // "no lo se" dicho de cuatro maneras. Eso es deliberado: el unico veredicto que
    // AFIRMA algo -"no cambio nada"- exige haber comparado dos $STATUS de verdad, uno de
    // antes y otro de despues. Todo lo demas se declara (CLAUDE.md 3.quinquies).

    // Los estados que la vista pinta como "esto es un hallazgo" y no como rutina. El
    // color NUNCA va solo: la palabra va escrita al lado, porque a pleno sol y con la
    // pantalla sucia el color es lo primero que se pierde.
    const DIARIO_CLASE = {
      LLEGO: 'diario-ok',
      ESPERANDO: 'diario-espera',
      SIN_RESPUESTA: 'diario-malo',
      NO_SALIO: 'diario-malo',
      CAMBIO: 'diario-ok',
      SIN_CAMBIO: 'diario-malo',
      NO_SE_PUDO_VER: 'diario-nosabe',
      NO_APLICA: 'diario-nosabe'
    };

    function _textoResumenDiario(c) {
      if (!c.ordenes && !c.sueltas) {
        return 'Todavia no se ha dado ninguna orden en esta sesion.';
      }
      let t = c.ordenes + ' ordenes';
      if (c.noSalieron) t += ' · ' + c.noSalieron + ' no llegaron a salir';
      if (c.enCurso) t += ' · ' + c.enCurso + ' en curso';
      if (c.sinRespuesta) t += ' · ' + c.sinRespuesta + ' sin respuesta';
      if (c.rechazadas) t += ' · ' + c.rechazadas + ' rechazadas por el equipo';
      if (c.sinCambio) t += ' · ' + c.sinCambio + ' no movieron MODO ni ESTADO';
      if (c.noSePudoVer) t += ' · ' + c.noSePudoVer + ' con efecto que no se pudo ver';
      if (c.sueltas) t += ' · ' + c.sueltas + ' respuestas sueltas';
      return t;
    }

    function renderDiario() {
      if (!diarioListaEl && !diarioResumenEl && !diarioNotaEl) return;
      // Misma salida temprana que la cinta y por el mismo motivo: no se repinta una
      // vista que nadie mira. Y con la misma consecuencia -el conmutador de pestanas
      // tiene que llamar aqui al abrir, o la vista se abriria en blanco con el diario
      // lleno, que es justo la mentira que esta pantalla existe para no contar-.
      const seccion = document.getElementById('tab-depuracion');
      if (seccion && !seccion.classList.contains('active')) return;

      const ahora = Date.now();
      // Se cuenta UNA vez y se reparte: contadores() recorre el diario entero y resuelve
      // los dos veredictos de cada entrada, y llamarlo dos veces por render lo hace dos
      // veces por trama con el equipo delante.
      const c = DiarioOrdenes.contadores(ahora);
      if (diarioResumenEl) {
        diarioResumenEl.textContent = _textoResumenDiario(c);
      }
      if (diarioNotaEl) {
        // LO QUE ESTE DIARIO NO SABE, SIEMPRE A LA VISTA Y NO EN UN PLIEGUE. Un tecnico
        // que lea "NO CAMBIO NADA" y entienda "el equipo desobedecio" se lleva del poste
        // una conclusion que este registro no puede sostener.
        let n = 'Este diario vive en la MEMORIA de la app y cabe ' + DiarioOrdenes.TOPE +
                ' ordenes. Al cerrar la app se pierde: exportelo antes de bajar del poste. ' +
                'Y no sabe tanto como parece: sabe que la app escribio la orden al cable, ' +
                'no que el equipo la recibiera; ve lo que el equipo DICE por $STATUS, no ' +
                'las luces del poste; y no se entera de ordenes dadas desde otro telefono, ' +
                'desde el mando de reles o desde los botones de la tarjeta.';
        if (c.descartados) {
          n += ' RECORTADO: se tiraron las ' + c.descartados + ' entradas mas antiguas al ' +
               'llegar al tope.';
        }
        diarioNotaEl.textContent = n;
      }

      if (!diarioListaEl) return;
      diarioListaEl.innerHTML = '';
      const vista = DiarioOrdenes.recientes(40);
      if (!vista.length) {
        const vacio = document.createElement('p');
        vacio.className = 'depu-vacio';
        // NUNCA una orden de ejemplo. Igual que la cinta: lo que sustituye a un dato que
        // no se tiene no es una simulacion, es decirlo.
        vacio.textContent = 'Todavia no se ha dado ninguna orden en esta sesion. Esta ' +
          'lista se llena sola al pulsar un mando; aqui no se ensena ningun ejemplo.';
        diarioListaEl.appendChild(vacio);
        return;
      }

      vista.forEach(r => {
        const fila = document.createElement('div');
        fila.className = 'diario-fila';

        if (r.clase === 'RESPUESTA_SUELTA') {
          fila.className += ' diario-suelta';
          fila.appendChild(_filaDiario('diario-nosabe', _horaDe(r.ms), 'RESP. SUELTA',
                                       r.respuesta.linea));
          const p = document.createElement('div');
          p.className = 'diario-porque';
          p.textContent = r.respuesta.motivoSuelta;
          fila.appendChild(p);
          diarioListaEl.appendChild(fila);
          return;
        }

        const estR = DiarioOrdenes.estadoRespuesta(r, ahora);
        const estE = DiarioOrdenes.estadoEfecto(r, ahora);
        if (!r.salio || estR === 'SIN_RESPUESTA' || estE === 'SIN_CAMBIO') {
          fila.className += ' diario-conhallazgo';
        }

        fila.appendChild(_filaDiario('', _horaDe(r.ms), 'ORDEN',
                                     DiarioOrdenes.textoOrden(r)));
        fila.appendChild(_filaDiario(DIARIO_CLASE[estR] || '',
                                     estR === 'LLEGO' ? _horaDe(r.respuesta.ms) : '',
                                     'RESPUESTA', DiarioOrdenes.textoRespuesta(r, ahora)));
        fila.appendChild(_filaDiario(DIARIO_CLASE[estE] || '', '', 'EFECTO',
                                     DiarioOrdenes.textoEfecto(r, ahora)));
        diarioListaEl.appendChild(fila);
      });
    }

    // Una de las tres lineas. Por textContent: aqui hay texto que vino del cable y no se
    // interpreta como HTML ni de broma.
    function _filaDiario(clase, hora, rotulo, texto) {
      const l = document.createElement('div');
      l.className = 'diario-linea ' + clase;
      const h = document.createElement('span');
      h.className = 'diario-hora';
      h.textContent = hora;
      const e = document.createElement('span');
      e.className = 'diario-rotulo';
      e.textContent = rotulo;
      const c = document.createElement('span');
      c.className = 'diario-cuerpo';
      c.textContent = texto;
      l.appendChild(h);
      l.appendChild(e);
      l.appendChild(c);
      return l;
    }

    // ---- Sacar el diario del poste ----------------------------------------
    //
    // LAS MISMAS DOS SALIDAS QUE LA CINTA, y por los mismos dos motivos: en el cruce
    // puede no haber internet, y dentro de un WebView una descarga puede no llegar a
    // ninguna parte sin que la pagina se entere.
    function textoDiario() {
      return DiarioOrdenes.aTexto(Date.now(), {
        App: 'IOT-VIAL V9.0 (Controladora Semaforos)',
        Cruce: state.site,
        Equipo: state.node === null ? 'sin identificar (ningun $STATUS con NODE)' : state.node,
        Serie: state.serie === null ? 'sin identificar' : state.serie,
        Firmware: Exportar.textoFirmware(state.firmware),
        Modo: state.modo === null ? 'sin telemetria' : state.modo,
        Estado: state.estadoLuces === null ? 'sin telemetria' : state.estadoLuces,
        Hora_RTC: state.hora === null ? 'sin telemetria' : state.hora
      });
    }

    function nombreFicheroDiario() {
      return 'Ordenes_' + state.site.replace(/[\s\·\/]+/g, '_') + '_' +
             new Date().toISOString().slice(0, 10) + '.txt';
    }

    if (btnDiarioExport) {
      btnDiarioExport.addEventListener('click', () => {
        if (!DiarioOrdenes.todas().length) {
          showToast('El diario esta vacio: no se ha dado ninguna orden que sacar');
          return;
        }
        const blob = new Blob([textoDiario()], { type: 'text/plain;charset=utf-8;' });
        const link = document.createElement('a');
        link.href = URL.createObjectURL(blob);
        link.download = nombreFicheroDiario();
        link.click();
        showToast('Fichero pedido. Si no aparece, use Copiar y pegue el texto');
      });
    }

    if (btnDiarioCopiar) {
      btnDiarioCopiar.addEventListener('click', () => {
        if (!diarioTextoEl) return;
        const texto = textoDiario();
        diarioTextoEl.value = texto;
        diarioTextoEl.hidden = false;
        try {
          diarioTextoEl.focus();
          diarioTextoEl.select();
        } catch (e) {
          // Seleccionar puede negarse en algun WebView; el texto ya esta a la vista.
        }
        if (navigator.clipboard && typeof navigator.clipboard.writeText === 'function') {
          navigator.clipboard.writeText(texto).then(
            () => showToast('Diario en el portapapeles, y tambien abajo para revisarlo'),
            () => showToast('El portapapeles se nego: el texto esta abajo, seleccionelo')
          );
        } else {
          showToast('El texto esta abajo, seleccionado: peguelo donde quiera');
        }
        if (navigator.share && typeof navigator.share === 'function') {
          navigator.share({ title: 'Diario de Órdenes IOT-VIAL', text: texto }).catch(() => {});
        }
      });
    }

    if (btnDiarioLimpiar) {
      btnDiarioLimpiar.addEventListener('click', () => {
        if (typeof window.confirm === 'function' &&
            !window.confirm('Se vacia el diario de ordenes de esta sesion. Si no lo ha ' +
                            'exportado, se pierde.')) return;
        DiarioOrdenes.limpiar();
        if (diarioTextoEl) {
          diarioTextoEl.value = '';
          diarioTextoEl.hidden = true;
        }
        renderDiario();
        addEvent('cyan', 'Diario de ordenes vaciado a peticion del usuario.');
      });
    }

    return { render: renderDiario, texto: textoDiario };
  }
};

if (typeof module !== 'undefined' && module.exports) {
  module.exports = DiarioVista;
}
