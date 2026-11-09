// ===== js/depuracion_vista.js =====
// La pantalla de depuracion: los contadores, la nota de lo que la cinta NO es, la lista de las
// ultimas tramas y sus cinco botones (exportar, copiar, vaciar, todas, rechazadas). La CINTA en si
// -lo que se anota y como se juzga- es RegistroCrudo, en js/depuracion.js; esto solo la pinta y la
// saca del poste. Sale de app.js en un corte SIN cambio de comportamiento (SPEC_4 9.12 tanda 3,
// D-98): el texto es el mismo, linea por linea, con dos espacios mas de sangria. Lo que necesita del
// cierre de app.js se lo pasa app.js al instalarlo, cada cosa por su nombre:
//
//   state      el objeto de estado de app.js (site, depuSoloRechazadas)
//   showToast  el aviso breve de app.js
//   addEvent   la anotacion en la lista de Eventos de app.js
//   horaDe     _horaDe(ms) de app.js, la hora de pared de una marca
//
// Devuelve { render, texto }. app.js conserva renderDepuracion() y textoDepuracion() como
// envoltorio, porque los llaman el envio de ordenes, las ramas de trama, el conmutador de
// pestanas, el arranque y el «Exportar registro» de Eventos.

const DepuracionVista = {
  instalar(dep) {
    const state = dep.state;
    const showToast = dep.showToast;
    const addEvent = dep.addEvent;
    const _horaDe = dep.horaDe;

    // Modo depuracion: la cinta de tramas en crudo (pestana propia, no mezclada con la
    // pantalla de operacion). Ver el bloque 1.quinquies.
    const depuContadoresEl = document.getElementById('depu-contadores');
    const depuListaEl = document.getElementById('depu-lista');
    const depuNotaEl = document.getElementById('depu-nota');
    const depuTextoEl = document.getElementById('depu-texto');
    const btnDepuTodas = document.getElementById('btn-depu-todas');
    const btnDepuRechazadas = document.getElementById('btn-depu-rechazadas');
    const btnDepuExport = document.getElementById('btn-depu-export');
    const btnDepuCopiar = document.getElementById('btn-depu-copiar');
    const btnDepuLimpiar = document.getElementById('btn-depu-limpiar');

    // ---- La pantalla de depuracion ----------------------------------------

    function _textoContadores(c) {
      const min = Math.round(c.ventanaMs / 60000);
      if (!c.total) {
        return 'En los ultimos ' + min + ' minutos no ha entrado ni salido ninguna trama.';
      }
      // LO QUE ENTRO Y LO QUE SALIO, POR SEPARADO. Sumarlos convertiria "el equipo hablo
      // poco" en "el equipo hablo mucho" segun cuanto hubiera pulsado el tecnico, que es
      // justo el numero que se mira para saber si la radio va bien.
      if (!c.entradas) {
        return 'Ultimos ' + min + ' min: no ha entrado ninguna trama del equipo. ' +
               (c.enviadas ? 'Si salieron ' + c.enviadas + ' ordenes de esta app: se ' +
                             'escribieron al cable y nadie contesto.'
                           : '');
      }
      let t = 'Ultimos ' + min + ' min: ' + c.entradas + ' tramas · ' +
              c.aceptadas + ' aceptadas · ' + c.rechazadas + ' rechazadas';
      if (c.enviadas) t += ' · ' + c.enviadas + ' ordenes enviadas';
      if (c.conReparos) {
        t += ' · ' + c.conReparos + ' aceptadas con algun campo sin medida';
      }
      const motivos = Object.keys(c.porMotivo).sort();
      if (motivos.length) {
        t += '. Motivos: ' + motivos.map(m => m + ' x' + c.porMotivo[m]).join(', ');
      }
      if (c.fueraDeVentana) {
        t += '. (' + c.fueraDeVentana + ' mas antiguas siguen en la cinta y no se ' +
             'cuentan en esta ventana.)';
      }
      return t;
    }

    function renderDepuracion() {
      if (!depuListaEl && !depuContadoresEl && !depuNotaEl) return;
      // NO SE REPINTA UNA VISTA QUE NADIE ESTA MIRANDO. Con el equipo delante entra un
      // $STATUS por segundo, y rehacer sesenta filas de DOM cada segundo mientras el
      // tecnico esta en la pantalla de trafico se paga en bateria del telefono y en
      // nada mas. Al abrir la pestana se pinta entera, que es lo unico que hace falta:
      // la cinta la guarda RegistroCrudo, no el DOM.
      //
      // Esta salida temprana OBLIGA a que el conmutador de pestanas llame aqui al abrir
      // la vista; si no, se abriria en blanco -que es exactamente la clase de fallo
      // silencioso que este fichero persigue-. La llamada esta en el bloque 9.
      const seccion = document.getElementById('tab-depuracion');
      if (seccion && !seccion.classList.contains('active')) return;
      const cuenta = RegistroCrudo.contadores();

      if (depuContadoresEl) depuContadoresEl.textContent = _textoContadores(cuenta);

      if (depuNotaEl) {
        // La declaracion de lo que esta cinta NO es. Va siempre a la vista, no en un
        // pliegue: un tecnico que la exporte creyendo que se lleva la noche entera se
        // va del poste con menos de lo que cree.
        let n = 'Esta cinta guarda las DOS direcciones: lo que entro del equipo y las ' +
                'ordenes que esta app escribio al cable (el PIN sale tapado). Vive en la ' +
                'MEMORIA de la app y cabe ' + RegistroCrudo.TOPE + ' tramas -unos ' +
                Math.round(RegistroCrudo.horizonteMinutos()) + ' minutos de trafico ' +
                'seguido-. Al cerrar la app se pierde: exportela antes de bajar del ' +
                'poste. Lo que si sobrevive al cierre es la bitacora del enlace de la ' +
                'pestana de Eventos, donde cada racha de rechazos deja su anotacion.';
        if (cuenta.descartados) {
          n += ' RECORTADA: se tiraron las ' + cuenta.descartados + ' tramas mas ' +
               'antiguas al llegar al tope.';
        }
        depuNotaEl.textContent = n;
      }

      if (btnDepuTodas) {
        btnDepuTodas.className = 'depu-filtro' + (state.depuSoloRechazadas ? '' : ' activo');
      }
      if (btnDepuRechazadas) {
        btnDepuRechazadas.className = 'depu-filtro' + (state.depuSoloRechazadas ? ' activo' : '');
      }

      if (!depuListaEl) return;
      depuListaEl.innerHTML = '';
      const todas = RegistroCrudo.recientes(60);
      const vista = state.depuSoloRechazadas
        ? todas.filter(r => r.veredicto === RegistroCrudo.RECHAZADA) : todas;

      if (!vista.length) {
        const vacio = document.createElement('p');
        vacio.className = 'depu-vacio';
        // NUNCA una trama de ejemplo. Lo que sustituye a un dato que no se tiene no es
        // una simulacion: es decirlo.
        vacio.textContent = todas.length
          ? 'Ninguna trama rechazada entre las ultimas ' + todas.length + ' que entraron.'
          : 'Todavia no ha entrado ninguna trama en esta sesion. Esta lista se llena ' +
            'sola con un equipo delante; aqui no se ensena ninguna trama de ejemplo.';
        depuListaEl.appendChild(vacio);
        return;
      }

      vista.forEach(r => {
        const fila = document.createElement('div');
        fila.className = 'depu-fila depu-' + r.veredicto.toLowerCase() +
                         (r.reparos.length ? ' depu-conreparos' : '');

        const cab = document.createElement('div');
        cab.className = 'depu-cab';
        const hora = document.createElement('span');
        hora.className = 'depu-hora';
        hora.textContent = _horaDe(r.ms);
        const marca = document.createElement('span');
        marca.className = 'depu-marca';
        // LOS TRES VEREDICTOS, CADA UNO CON SU PALABRA. Esta linea decia
        // `RECHAZADA ? ... : 'ACEPTADA'`, o sea que TODO lo que no fuera rechazado se
        // rotulaba como aceptado: al entrar ENVIADA, las ordenes salientes se habrian
        // pintado "ACEPTADA · CMD" -la app diciendo que pinto una trama que ella misma
        // escribio-. Un dos-ramas donde hay tres casos no falla: miente en el tercero.
        //
        // La FLECHA va delante y es lo que se lee de un vistazo recorriendo la lista:
        // "<--" entro, "-->" salio. El color no decide nada por su cuenta.
        marca.textContent =
          r.veredicto === RegistroCrudo.ENVIADA
            ? '--> ENVIADA' + (r.tipo ? ' · ' + r.tipo : '')
            : (r.veredicto === RegistroCrudo.RECHAZADA
                ? '<-- RECHAZADA · ' + r.motivo
                : '<-- ACEPTADA' + (r.tipo ? ' · ' + r.tipo : ''));
        cab.appendChild(hora);
        cab.appendChild(marca);
        fila.appendChild(cab);

        // LA TRAMA EN CRUDO, y por textContent: es texto que viene del cable y no se
        // interpreta como HTML ni de broma. Los caracteres de control salen escapados
        // para que un CR suelto no parta la lista y ensene una trama que no es la que
        // entro; lo que se escapa es como se ve, no lo que se guarda.
        const crudo = document.createElement('code');
        crudo.className = 'depu-crudo';
        crudo.textContent = RegistroCrudo.escapar(r.linea) +
          (r.cortada ? '   [CORTADA: llegaron ' + r.largoOriginal + ' caracteres]' : '');
        fila.appendChild(crudo);

        if (r.detalle) {
          const d = document.createElement('div');
          d.className = 'depu-detalle';
          d.textContent = r.detalle;
          fila.appendChild(d);
        }
        r.reparos.forEach(x => {
          const d = document.createElement('div');
          d.className = 'depu-reparo';
          d.textContent = 'se pinto, pero: ' + x;
          fila.appendChild(d);
        });

        depuListaEl.appendChild(fila);
      });
    }

    // ---- Sacar el registro del poste --------------------------------------
    //
    // EN EL CRUCE PUEDE NO HABER INTERNET, asi que ninguna de las dos salidas lo
    // necesita: las dos componen el texto aqui dentro. (El reporte de WhatsApp de la
    // pestana de Eventos SI abre api.whatsapp.com, y por eso no es la salida de esto.)
    //
    // Y son DOS a proposito. La descarga es la comoda, pero dentro de un WebView de
    // Android una descarga puede no llegar a ninguna parte sin que la pagina se entere
    // -no hay forma fiable de preguntarselo-, y entonces el tecnico se baja del poste
    // creyendo que lleva el fichero. La segunda salida es la que no puede fallar: el
    // texto entero a la vista, seleccionado, para pegarlo donde sea.
    function textoDepuracion() { return Exportar.textoCinta(state, _horaDe); }

    function nombreFicheroDepuracion() {
      return 'Tramas_' + state.site.replace(/[\s\·\/]+/g, '_') + '_' +
             new Date().toISOString().slice(0, 10) + '.txt';
    }

    if (btnDepuExport) {
      btnDepuExport.addEventListener('click', () => {
        if (!RegistroCrudo.todas().length) {
          showToast('La cinta esta vacia: no ha entrado ninguna trama que sacar');
          return;
        }
        const blob = new Blob([textoDepuracion()], { type: 'text/plain;charset=utf-8;' });
        const link = document.createElement('a');
        link.href = URL.createObjectURL(blob);
        link.download = nombreFicheroDepuracion();
        link.click();
        // El toast NO afirma que el fichero este en el telefono: la pagina no puede
        // saberlo. Dice lo que se ha intentado y donde esta la salida que no falla.
        showToast('Fichero pedido. Si no aparece, use Copiar y pegue el texto');
      });
    }

    if (btnDepuCopiar) {
      btnDepuCopiar.addEventListener('click', () => {
        if (!depuTextoEl) return;
        const texto = textoDepuracion();
        depuTextoEl.value = texto;
        depuTextoEl.hidden = false;
        try {
          depuTextoEl.focus();
          depuTextoEl.select();
        } catch (e) {
          // Seleccionar puede negarse en algun WebView; el texto ya esta a la vista,
          // que es lo que de verdad hace falta.
        }
        if (navigator.clipboard && typeof navigator.clipboard.writeText === 'function') {
          navigator.clipboard.writeText(texto).then(
            () => showToast('Texto en el portapapeles, y tambien abajo para revisarlo'),
            () => showToast('El portapapeles se nego: el texto esta abajo, seleccionelo')
          );
        } else {
          showToast('El texto esta abajo, seleccionado: peguelo donde quiera');
        }
        if (navigator.share && typeof navigator.share === 'function') {
          navigator.share({ title: 'Tramas IOT-VIAL', text: texto }).catch(() => {});
        }
      });
    }

    if (btnDepuLimpiar) {
      btnDepuLimpiar.addEventListener('click', () => {
        Dialogo.confirmar('¿Vaciar la cinta de tramas?', 'Se vacía la de esta sesión. Si no la ha ' +
                          'exportado, se pierde.', 'Vaciar la cinta', () => {
          RegistroCrudo.limpiar();
          if (depuTextoEl) {
            depuTextoEl.value = '';
            depuTextoEl.hidden = true;
          }
          renderDepuracion();
          addEvent('cyan', 'Cinta de tramas en crudo vaciada a peticion del usuario.');
        });
      });
    }

    if (btnDepuTodas) {
      btnDepuTodas.addEventListener('click', () => {
        state.depuSoloRechazadas = false;
        renderDepuracion();
      });
    }
    if (btnDepuRechazadas) {
      btnDepuRechazadas.addEventListener('click', () => {
        state.depuSoloRechazadas = true;
        renderDepuracion();
      });
    }

    return { render: renderDepuracion, texto: textoDepuracion };
  }
};

if (typeof module !== 'undefined' && module.exports) {
  module.exports = DepuracionVista;
}
