// ===== js/busqueda_equipos.js =====
// La lista de equipos del modal de Bluetooth: lo que devuelven list() y discoverUnpaired(), la nota que
// dice por que la lista esta como esta, y el boton Desconectar / Reconectar. La fila de cada equipo la
// pinta js/lista_equipos.js; el enlace en si, js/enlace_equipos.js. Sale de app.js en un corte SIN cambio
// de comportamiento (SPEC_4 9.12 tanda 4.a, D-98): el texto es el mismo, linea por linea y con la MISMA
// sangria. Lo que necesita del cierre de app.js se lo pasa app.js al instalarlo, cada cosa por su nombre:
//
//   state, showToast, addEvent el estado, el aviso breve y la anotacion en Eventos de app.js
//   gruposVivos                los grupos de la sesion (reales o de practica), para filtrar la lista
//   closeModal, btModal        cerrar el modal de Bluetooth
//   desconectarEquipo          el envoltorio de app.js (vive en js/enlace_equipos.js)
//   reconectarUltimoEquipo     de js/enlace_equipos.js
//   btDeviceListContainer, btFiltroEl, btnScanBluetoothLive, btnBtDisconnect   elementos de app.js
//
// Devuelve { repintarEquipos, escanearEquipos, actualizarBotonDesconectar }. app.js conserva los tres de
// envoltorio: los llaman los grupos y el filtro, la apertura del modal y el vigilante de silencio.

const BusquedaEquipos = {};

BusquedaEquipos.instalar = function (dep) {
  const state = dep.state;
  const showToast = dep.showToast;
  const addEvent = dep.addEvent;
  const gruposVivos = dep.gruposVivos;
  const closeModal = dep.closeModal;
  const btModal = dep.btModal;
  const desconectarEquipo = dep.desconectarEquipo;
  const reconectarUltimoEquipo = dep.reconectarUltimoEquipo;
  const btDeviceListContainer = dep.btDeviceListContainer;
  const btFiltroEl = dep.btFiltroEl;
  const btnScanBluetoothLive = dep.btnScanBluetoothLive;
  const btnBtDisconnect = dep.btnBtDisconnect;


  // =========================================================================
  // 6.bis LA LISTA DE EQUIPOS LA PINTA EL SISTEMA, NO EL HTML
  // =========================================================================
  //
  // N-124: este boton NO PINTABA NADA. Llamaba a list(), contaba lo que volvia y hacia
  // un toast con el numero: el resultado de la unica busqueda real de la app no llegaba
  // jamas a la pantalla, y debajo seguian las dos filas fijas del HTML con el MAC
  // escrito a mano. Una de ellas empezaba por 98:D3:31 -prefijo de los HC-05, los
  // modulos que el ESP32 sustituyo el 28/08- y la otra era relleno, asi que connect()
  // marcaba un numero que no existe por muy bien que N-122 hubiera puesto la llamada.
  // LO QUE SE HA VISTO EN ESTA BUSQUEDA, POR MAC. Dos fuentes escriben aqui y no
  // llegan a la vez: list() devuelve los emparejados en milisegundos y
  // discoverUnpaired() tarda lo que dure el barrido de Android. Sin un acumulador la
  // segunda tanda borraria a la primera, que es justo la que el tecnico esta mirando
  // mientras espera.
  //
  // EL EMPAREJAMIENTO GANA CUANDO UN EQUIPO SALE EN LAS DOS LISTAS: es lo que decide si
  // al pulsar la fila Android va a pedir vinculacion o no, y eso hay que acertarlo.
  let equiposVistos = new Map();
  // Por que la lista esta como esta. Va SIEMPRE, tenga filas o no: una lista vacia y una
  // lista corta significan cosas distintas segun lo que se haya podido buscar, y esta
  // frase es lo unico que lo dice.
  let notaLista = '';

  function mensajeEnLista(texto) {
    notaLista = texto;
    repintarEquipos();
  }

  // Mete lo que devuelve una fuente en el acumulador. `emparejados` dice de cual viene, y
  // es un dato del TELEFONO, no del equipo: un modulo encendido al lado sale "sin
  // emparejar" aunque sea el mismo poste de ayer, porque lo que falta es la vinculacion.
  function pintarEquipos(dispositivos, emparejados) {
    const lista = Array.isArray(dispositivos) ? dispositivos : [];
    let nuevos = 0;
    lista.forEach(d => {
      // El MAC es lo unico que connect() sabe marcar, y el plugin lo entrega en
      // `address` o en `id` segun la version. Un equipo sin direccion no se guarda: una
      // fila que no se puede pulsar es peor que una fila que no esta.
      const mac = d && (d.address || d.id);
      if (!mac) return;
      const clave = String(mac);
      const yaEstaba = equiposVistos.get(clave);
      if (yaEstaba && yaEstaba.emparejado) return;
      if (!yaEstaba) nuevos += 1;
      equiposVistos.set(clave, {
        mac: clave,
        nombre: (d && d.name && String(d.name).trim()) || clave,
        emparejado: !!emparejados
      });
    });
    repintarEquipos();
    return nuevos;
  }

  // La fila de cada equipo y el rotulo -M / -E se pintan en js/lista_equipos.js
  // (corte del 28/09, sin cambio de comportamiento). Este envoltorio les pasa lo
  // que vive aqui: el contenedor del modal, lo visto en esta busqueda y la nota.
  function repintarEquipos() {
    return ListaEquipos.repintarEquipos(btDeviceListContainer, equiposVistos, notaLista,
      { grupos: gruposVivos().todos(), filtro: btFiltroEl ? btFiltroEl.value : '' });   // SPEC_4 9.14 (d)
  }

  // porPeticion = lo pulso el tecnico. Al abrir el modal se llama con false: si hay
  // radio la lista se rellena sola -es la misma busqueda que hace el boton- y si no la
  // hay se vuelve en silencio dejando el aviso que trae el HTML. Anunciar "no hay
  // radio" cada vez que se abre el modal en un navegador seria ruido; callarselo
  // cuando alguien PIDE la busqueda seria lo otro, que es peor: se le contesta que no
  // se pudo buscar, que no es lo mismo que no haber encontrado.
  //
  // =========================================================================
  // 6.ter list() NO BUSCA: LEE. LA BUSQUEDA DE VERDAD ES discoverUnpaired()
  // =========================================================================
  // REPORTE DE BANCO DEL 04/09, con las dos placas encendidas: "cuando intentas
  // conectarte por Bluetooth al otro ESP, solamente me sale el maestro... apagaste el
  // maestro para ver si encuentras el Bluetooth del esclavo. No aparece. Para que me
  // salga, yo creo que tengo que desvincular ese".
  //
  // Y tenia razon en el diagnostico: list() devuelve getBondedDevices(), o sea lo que
  // ESTE telefono tiene emparejado en Ajustes de Android, NO lo que hay encendido al
  // lado. El propio comentario de abajo ya lo decia bien desde N-124 -"CERO EMPAREJADOS
  // NO ES CERO EQUIPOS"-, y esa frase honesta era todo lo que la app hacia: describir
  // con precision una busqueda que no buscaba. Es §3.quinquies leido por la mitad -no
  // simular el dato que falta- sin la otra mitad, que es IR A POR EL cuando se puede.
  //
  // MEDIDO en el plugin instalado, no supuesto (node_modules/cordova-plugin-bluetooth-
  // serial):
  //   www/bluetoothSerial.js:111  discoverUnpaired: function (success, failure)
  //   src/android/.../BluetoothSerial.java:291  registra un BroadcastReceiver de
  //     ACTION_FOUND, llama a bluetoothAdapter.startDiscovery(), y devuelve el array
  //     entero en ACTION_DISCOVERY_FINISHED. Cada elemento trae {name, address, id}.
  //
  // POR QUE LAS DOS Y NO SOLO EL DESCUBRIMIENTO: el barrido de Android tarda lo suyo y
  // no siempre ve un modulo que ya esta emparejado y a mano. Se pinta primero lo que
  // list() da al instante -que es con lo que el tecnico conecta de un toque- y encima
  // se anaden los que aparezcan.
  //
  // Y SI NO ESTA, NO SE TAPA. Si el plugin de este APK no trae discoverUnpaired -o si
  // falla-, la lista se queda con los emparejados y se DICE que no se llego a buscar
  // los demas. Un "no hay mas equipos" sobre una busqueda que no ocurrio es la mentira
  // que este apartado existe para no repetir.
  function escanearEquipos(porPeticion) {
    const bt = (typeof window !== 'undefined') ? window.bluetoothSerial : null;
    const hayRadio = bt && typeof bt.list === 'function';
    if (!hayRadio) {
      if (!porPeticion) return;
      showToast('Escaneo no disponible fuera del APK');
      addEvent('red', 'Escaneo Bluetooth no realizado: no hay radio disponible en ' +
                      'este entorno. No se sabe que equipos hay, que no es lo mismo ' +
                      'que no haberlos encontrado.');
      equiposVistos = new Map();
      mensajeEnLista('Sin radio en este entorno: no se pudo buscar. Esta lista está ' +
                     'vacía porque no hubo búsqueda, no porque no haya equipos.');
      return;
    }
    // Cada busqueda parte de cero: una lista que acumula entre escaneos ensena equipos
    // que ya no estan, y en un cruce eso manda a conectar contra un poste apagado.
    equiposVistos = new Map();
    notaLista = '';
    // El rotulo NO nombra el modulo. En obra ya se ha cambiado de radio mas de una vez
    // -del HC-05 al ESP32-, asi que un texto que diga "HC-05" queda desmintiendo al
    // equipo el dia que se sustituya. "El enlace" vale para todos.
    if (porPeticion) showToast('🔍 Buscando equipos...');
    bt.list(
      (dispositivos) => {
        const pintados = pintarEquipos(dispositivos, true);
        if (!pintados) {
          // CERO EMPAREJADOS NO ES CERO EQUIPOS. list() devuelve lo que ESTE telefono
          // tiene emparejado en Ajustes de Android, no lo que hay encendido al lado.
          // Ya no es el final del camino -debajo se busca de verdad-, pero sigue siendo
          // el dato que explica por que la lista esta vacia en este instante.
          addEvent('red', 'Escaneo Bluetooth: 0 equipos emparejados en este telefono. ' +
                          'list() solo lee los emparejados; la busqueda de los demas ' +
                          'va a continuacion.');
        } else {
          addEvent('green', 'Escaneo Bluetooth: ' + pintados + ' equipo(s) ' +
                            'emparejado(s) listados con su MAC real.');
        }
        buscarNoEmparejados(porPeticion);
      },
      (err) => {
        // No se finge un resultado vacio: no encontrar y no poder buscar son cosas
        // distintas, y solo una de las dos permite concluir que no hay equipo.
        // N-125: UN FALLO DE PERMISO NO SE PARECE EN NADA A UN FALLO DE RADIO, Y ANTES
        // SE VEIAN IGUAL. En banco (04/09) el tecnico emparejo el ESP32, la app dijo "el
        // escaneo fallo", y ese texto no le decia que hacer: la causa era que Android 12+
        // exige BLUETOOTH_CONNECT concedido EN RUNTIME para listar emparejados, y nadie lo
        // pedia. Se pide desde MainActivity, pero si el usuario dice que NO -o si esta
        // corriendo una version vieja- el texto tiene que llevar la salida encima.
        //
        // Se mira el texto del error porque es lo unico que el plugin entrega: no hay un
        // codigo. Y si no casa, NO se inventa una causa: se dice el error tal cual.
        var txt = String(err || '');
        var esPermiso = /permis|security|denied|denegad/i.test(txt);
        showToast(esPermiso ? 'Falta el permiso de Bluetooth' : 'El escaneo fallo');
        addEvent('red', 'Escaneo Bluetooth fallido: ' + txt + '. No se sabe que hay.');
        mensajeEnLista(esPermiso
          ? 'Falta el permiso «Dispositivos cercanos». Ajustes de Android → Aplicaciones → ' +
            'IOT VIAL → Permisos → Dispositivos cercanos → Permitir. Después vuelva a pulsar Buscar.'
          : 'El escaneo falló: no se sabe qué equipos hay. Si se repite, compruebe en Ajustes de ' +
            'Android → Aplicaciones → IOT VIAL → Permisos que «Dispositivos cercanos» está ' +
            'permitido; sin ese permiso Android no deja ver los equipos emparejados.');
        // Se busca igual: que no se puedan LEER los emparejados no dice nada de si se
        // pueden DESCUBRIR los demas, y son dos permisos distintos en Android 12+
        // (BLUETOOTH_CONNECT contra BLUETOOTH_SCAN). Dar por perdidas las dos por una
        // seria descartar por eliminacion con la tabla incompleta.
        buscarNoEmparejados(porPeticion);
      }
    );
  }

  // La segunda mitad del escaneo. Va aparte porque su resultado se AÑADE al de list() y
  // llega mucho mas tarde: mientras tanto la lista de emparejados ya esta en pantalla.
  function buscarNoEmparejados(porPeticion) {
    const bt = (typeof window !== 'undefined') ? window.bluetoothSerial : null;
    if (!bt || typeof bt.discoverUnpaired !== 'function') {
      // NO SE TAPA CON UN SILENCIO. Sin esta llamada la lista es la de siempre -solo
      // emparejados-, y quien la mire tiene que saber que lo que no sale puede estar
      // encendido delante de el.
      notaLista = 'Solo se listan los equipos EMPAREJADOS: esta versión no puede buscar ' +
                  'los que no lo están. Si falta un poste, empárejelo en Ajustes de ' +
                  'Android → Bluetooth y vuelva a pulsar Buscar.';
      repintarEquipos();
      if (porPeticion) {
        addEvent('red', 'Busqueda de equipos NO emparejados no realizada: esta version ' +
                        'de la app no dispone de discoverUnpaired(). Lo que no aparece ' +
                        'en la lista no esta descartado, esta sin buscar.');
      }
      return;
    }
    notaLista = 'Buscando también los equipos NO emparejados… (Android tarda unos ' +
                'segundos; los emparejados ya están arriba)';
    repintarEquipos();
    if (porPeticion) showToast('🔍 Buscando también los no emparejados…');
    bt.discoverUnpaired(
      (dispositivos) => {
        const nuevos = pintarEquipos(dispositivos, false);
        if (nuevos) {
          notaLista = 'Los marcados «sin emparejar» están encendidos cerca pero este ' +
                      'teléfono no los conoce: al pulsarlos Android pedirá vincular ' +
                      'antes de conectar.';
          addEvent('green', 'Busqueda de no emparejados: ' + nuevos + ' equipo(s) ' +
                            'nuevo(s) encontrado(s) encendidos cerca.');
        } else {
          // CERO ENCONTRADOS SI ES UN DATO AQUI, al reves que en list(): esta busqueda
          // si mira lo que hay encendido. Lo que sigue sin cubrir -y por eso se dice- es
          // el modulo que esta ya conectado a OTRO telefono: ese no se anuncia.
          notaLista = 'La búsqueda terminó sin encontrar equipos nuevos sin emparejar. ' +
                      'Un módulo ya conectado a otro teléfono no se anuncia.';
          addEvent('cyan', 'Busqueda de no emparejados: terminada, ningun equipo nuevo.');
        }
        repintarEquipos();
      },
      (err) => {
        var txt = String(err || '');
        // Es OTRO permiso que el de list(): en Android 12+ el barrido pide
        // BLUETOOTH_SCAN, y en las versiones anteriores ubicacion. El plugin pide
        // ACCESS_COARSE_LOCATION el solo (BluetoothSerial.java:216-221) y NO pide
        // BLUETOOTH_SCAN, asi que en un telefono moderno este es el camino que se cae.
        var esPermiso = /permis|security|denied|denegad|location|ubicac/i.test(txt);
        notaLista = (esPermiso
          ? 'No se pudieron buscar los NO emparejados: falta el permiso «Dispositivos ' +
            'cercanos» o «Ubicación». Ajustes de Android → Aplicaciones → IOT VIAL → ' +
            'Permisos. Arriba solo están los ya emparejados.'
          : 'No se pudieron buscar los NO emparejados (' + txt + '). Arriba solo están ' +
            'los ya emparejados: lo que falte está SIN BUSCAR, no descartado.');
        repintarEquipos();
        addEvent('red', 'Busqueda de no emparejados fallida: ' + txt + '. La lista de ' +
                        'arriba son solo emparejados; no se sabe que mas hay encendido.');
        if (porPeticion) showToast('No se pudo buscar los no emparejados');
      }
    );
  }

  if (btnScanBluetoothLive) {
    btnScanBluetoothLive.addEventListener('click', () => escanearEquipos(true));
  }

  // El boton de desconectar SOLO existe mientras hay enlace. Un mando que no desconecta
  // nada es un mando que no hace nada, y esta pantalla ya tuvo cuatro de esos (N-75).
  // Se refresca desde aqui y desde olvidarEnlace(), que son los dos unicos sitios donde
  // state.connected cambia de valor.
  // EL MISMO BOTON CONTESTA A LAS DOS SITUACIONES, PORQUE SON LA MISMA PREGUNTA: que
  // hago con el equipo que tengo delante. Con enlace vivo, soltarlo; con un enlace que
  // se cayo, volver a el. Dos botones -- uno de ellos siempre apagado -- serian otra vez
  // la botonera en gris que el responsable acaba de mandar quitar de la pantalla del
  // Esclavo.
  function actualizarBotonDesconectar() {
    if (!btnBtDisconnect) return;
    const eq = state.ultimoEquipo;
    if (state.connected) {
      btnBtDisconnect.style.display = 'block';
      btnBtDisconnect.classList.remove('reconectar');
      // NOMBRA EL POSTE DEL QUE SE VA A SALIR. Con dos modulos que se llaman casi igual
      // (N-129), un "Desconectar" a secas deja al tecnico soltando el que no era.
      btnBtDisconnect.textContent = '🔌 Desconectar de ' +
                                    (state.deviceName || state.deviceMac || 'este poste');
    } else if (eq && eq.mac) {
      btnBtDisconnect.style.display = 'block';
      btnBtDisconnect.classList.add('reconectar');
      btnBtDisconnect.textContent = '🔄 Reconectar con ' + eq.name;
    } else {
      // Sin equipo previo no hay nada que ofrecer: un boton que no puede hacer nada es
      // el adorno que da verde de CLAUDE.md 8.bis, con forma de interfaz.
      btnBtDisconnect.style.display = 'none';
    }
  }

  if (btnBtDisconnect) {
    btnBtDisconnect.addEventListener('click', () => {
      if (!state.connected) { reconectarUltimoEquipo(); closeModal(btModal); return; }
      const quien = state.deviceName || state.deviceMac || 'el equipo';
      showToast('Desconectando de ' + quien);
      // Sin `alTerminar`: aqui no se va a conectar nada detras. La lista se queda como
      // estaba para que el siguiente poste este a un toque, que es de lo que se quejaba
      // el banco -"tengo que reiniciar la aplicacion"-.
      desconectarEquipo(() => {
        actualizarBotonDesconectar();
        showToast('Sin equipo: elija otro de la lista');
      });
    });
  }


  return { repintarEquipos: repintarEquipos, escanearEquipos: escanearEquipos,
           actualizarBotonDesconectar: actualizarBotonDesconectar };
};

if (typeof module !== 'undefined' && module.exports) {
  module.exports = BusquedaEquipos;
}
