// ===== js/lista_equipos.js =====
// La LISTA DE EQUIPOS del modal de Bluetooth: una fila por modulo visto, con la letra
// final del rotulo (-M / -E) aparte y el aviso de los modulos sin matricula. Solo PINTA:
// el acumulador de lo visto (equiposVistos), la nota de la lista y las dos busquedas
// -list() y discoverUnpaired()- siguen en app.js, apartados 6.bis y 6.ter.
//
// Sale de app.js el 28/09 en un commit de CORTE, sin cambio de comportamiento: el
// texto de abajo es el mismo, linea por linea y con la misma sangria. Lo unico que
// cambia es la firma: repintarEquipos() recibe el contenedor, el mapa y la nota en vez
// de leerlos del cierre de app.js, que conserva repintarEquipos() como envoltorio.

const ListaEquipos = (() => {
  // N-129: LOS DOS MODULOS SE LLAMAN CASI IGUAL, Y ESE "CASI" ES TODA LA DIFERENCIA.
  //
  // MEDIDO en ESP32_Expansion/include/contrato.h:258-259: el rotulo es ROTULO_PREFIJO
  // "SEM-" mas la serie mas "-M" o "-E", y el modulo lo aprende del $STATUS para la
  // arrancada SIGUIENTE; hasta entonces se anuncia con ROTULO_PROVISIONAL =
  // "SEM-SIN-MATRICULA". O sea que dos modulos virgenes se llaman EXACTAMENTE IGUAL en
  // la lista del telefono. Reportado desde el banco: "como tienen nombres tan similares,
  // para no confundirnos, a ver cual es cual".
  //
  // Lo unico que la app puede hacer SIN INVENTAR es separar la letra final y ensenarla
  // aparte. NO se escribe MAESTRO ni ESCLAVO en la fila: la punta la declara el equipo
  // en NODE: del $STATUS cuando habla, y ponerla aqui seria la misma invencion que los
  // dos data-mac escritos a mano que N-124 tuvo que retirar. Se pinta la letra que el
  // modulo dice tener, no la conclusion que esa letra sugiere.
  function sufijoDeRotulo(nombre) {
    const txt = String(nombre || '');
    if (txt === 'SEM-SIN-MATRICULA') return { letra: '', virgen: true };
    const m = /^SEM-.+-([ME])$/.exec(txt);
    return { letra: m ? m[1] : '', virgen: false };
  }

  // APP-1.1: EL MODULO DEL POSTE SE PERDIA ENTRE AURICULARES, EL COCHE Y DECENAS DE MAC
  // SIN NOMBRE. Los que se anuncian con ROTULO_PREFIJO "SEM-" (contrato.h) van arriba y
  // a la vista; el resto se pliega bajo "Otros dispositivos (N)" y sigue pulsable, porque
  // un modulo con otro nombre (un HC-05 de banco) tambien es conectable. Sin ningun SEM-
  // a la vista el plegado sale ABIERTO: esconderlo todo seria esconder el unico candidato.
  // Si el tecnico lo abre o lo cierra, se respeta en los repintados siguientes.
  const esSem = (eq) => /^SEM-/.test(eq.nombre);
  const esAnonimo = (eq) => eq.nombre === eq.mac;   // sin nombre: pintarEquipos() pone el MAC
  let plegadoElegido = null;

  function repintarEquipos(btDeviceListContainer, equiposVistos, notaLista) {
    if (!btDeviceListContainer) return 0;
    btDeviceListContainer.innerHTML = '';
    let pintados = 0;
    // Los emparejados arriba: son los que conectan de un toque. Dentro de cada grupo, por
    // nombre, para que dos modulos hermanos caigan juntos y la letra final quede una
    // debajo de otra, que es donde se ve la diferencia. Los anonimos, al final de todo.
    const orden = Array.from(equiposVistos.values()).sort((a, b) => {
      if (esAnonimo(a) !== esAnonimo(b)) return esAnonimo(a) ? 1 : -1;
      if (a.emparejado !== b.emparejado) return a.emparejado ? -1 : 1;
      return a.nombre.localeCompare(b.nombre);
    });
    const sem = orden.filter(esSem);
    const otros = orden.filter(eq => !esSem(eq));
    let destinoOtros = btDeviceListContainer;
    if (otros.length) {
      const plegado = document.createElement('details');
      plegado.className = 'bt-otros';
      const inicial = plegadoElegido !== null ? plegadoElegido : sem.length === 0;
      plegado.open = inicial;
      // El navegador tambien avisa 'toggle' del `open` de la linea de arriba: ese no es
      // una eleccion del tecnico y no se guarda.
      plegado.addEventListener('toggle', () => {
        if (plegadoElegido !== null || plegado.open !== inicial) plegadoElegido = plegado.open;
      });
      const titulo = document.createElement('summary');
      titulo.textContent = 'Otros dispositivos (' + otros.length + ')';
      plegado.appendChild(titulo);
      destinoOtros = plegado;
    }
    sem.concat(otros).forEach(eq => {
      const item = document.createElement('div');
      item.className = 'bt-device-item' + (eq.emparejado ? '' : ' sin-emparejar');
      item.setAttribute('data-name', eq.nombre);
      item.setAttribute('data-mac', eq.mac);
      // data-emparejado lo lee el manejador de clic para avisar ANTES de conectar de que
      // Android va a pedir vinculacion. NO se escribe data-node: una fila no puede saber
      // que punta hay al otro lado. Lo dice el equipo en NODE: del $STATUS.
      item.setAttribute('data-emparejado', eq.emparejado ? '1' : '0');

      const icono = document.createElement('div');
      icono.className = 'bt-dev-icon';
      icono.textContent = '📡';

      const info = document.createElement('div');
      info.className = 'bt-dev-info';
      const titulo = document.createElement('strong');
      const suf = sufijoDeRotulo(eq.nombre);
      if (suf.letra) {
        // La letra sale del nombre en un <span> propio y no en negrita dentro del texto:
        // el CSS le da recuadro y color, y es lo unico que el ojo tiene que comparar
        // entre dos filas que por lo demas dicen lo mismo.
        // textContent en las dos piezas y no innerHTML: el nombre lo pone el modulo del
        // otro lado, y un texto que llega de fuera no se inyecta como marcado.
        const base = document.createElement('span');
        base.textContent = eq.nombre.slice(0, eq.nombre.length - 1);
        const letra = document.createElement('span');
        letra.className = 'bt-dev-sufijo';
        letra.textContent = suf.letra;
        titulo.appendChild(base);
        titulo.appendChild(letra);
      } else {
        titulo.textContent = eq.nombre;
      }
      const detalle = document.createElement('small');
      detalle.textContent = 'MAC: ' + eq.mac;
      info.appendChild(titulo);
      info.appendChild(detalle);
      if (suf.virgen) {
        // El caso que hace peligroso a todo lo demas: sin matricula aprendida las DOS
        // puntas se anuncian con este mismo nombre, asi que la fila no distingue nada.
        // Se dice en la fila, que es donde se elige, y no en un manual.
        const aviso = document.createElement('small');
        aviso.className = 'bt-dev-aviso';
        aviso.textContent = 'sin matricula: las DOS puntas se llaman asi. Hasta que el ' +
                            'equipo hable solo las separa el MAC.';
        info.appendChild(aviso);
      }

      const etiqueta = document.createElement('span');
      etiqueta.className = 'bt-dev-badge' + (eq.emparejado ? '' : ' sin-emparejar');
      // "emparejado" / "sin emparejar" es lo unico que consta de esta fila. Poner
      // "Maestro" o "Esclavo" seria la misma invencion que los dos MAC que se retiraron.
      etiqueta.textContent = eq.emparejado ? 'emparejado' : 'sin emparejar';

      item.appendChild(icono);
      item.appendChild(info);
      item.appendChild(etiqueta);
      (esSem(eq) ? btDeviceListContainer : destinoOtros).appendChild(item);
      pintados += 1;
    });
    if (destinoOtros !== btDeviceListContainer) btDeviceListContainer.appendChild(destinoOtros);
    if (notaLista) {
      const p = document.createElement('p');
      p.className = 'modal-desc';
      p.textContent = notaLista;
      btDeviceListContainer.appendChild(p);
    }
    return pintados;
  }

  return { sufijoDeRotulo, repintarEquipos };
})();

if (typeof module !== 'undefined' && module.exports) {
  module.exports = ListaEquipos;
}
