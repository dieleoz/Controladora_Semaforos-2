// ===== js/enlace_equipos.js =====
// Abrir, cambiar, soltar y reconectar el enlace Bluetooth con un poste, y el clic en una fila de la lista
// de equipos. Sale de app.js en un corte SIN cambio de comportamiento (SPEC_4 9.12 tanda 4.a, D-98): el
// texto es el mismo, linea por linea y con la MISMA sangria (el cuerpo de instalar va a dos espacios, como
// en el cierre de app.js). Una sola linea cambia: TIMEOUT_ENLACE_MS se lee como dep.TIMEOUT_ENLACE_MS,
// porque app.js lo declara (const) despues de instalar esto y leerlo aqui al instalar daria ReferenceError;
// dep lo trae con un getter, que se evalua al conectar. Lo que necesita del cierre de app.js se lo pasa
// app.js al instalarlo, cada cosa por su nombre:
//
//   state                      el objeto de estado de app.js
//   showToast, addEvent        el aviso breve y la anotacion en Eventos
//   renderLights, revisarGrupo, actualizarEmergencia, renderDiagnosticoEnlace, renderAvisoCamaraPluma,
//   renderAvisoCorte, renderCarteles      los pintores que se refrescan al soltar o abrir el enlace
//   parseNmeaTelemetry         la puerta unica de las tramas del equipo
//   enviarComandoFirmware      el envio de ordenes (se pasa a Exportar.pedirVersionAlConectar)
//   closeModal, btModal        cerrar el modal de Bluetooth
//   actualizarBotonDesconectar el envoltorio de app.js (vive en js/busqueda_equipos.js)
//   btnDevice, btStatusDot, btBtnText, nodeNameEl, rssiTextEl, btDeviceListContainer   elementos de app.js
//   TIMEOUT_ENLACE_MS          getter (ver arriba)
//
// Devuelve { desconectarEquipo, reconectarUltimoEquipo }. app.js conserva desconectarEquipo() de
// envoltorio (lo llama el vigilante de silencio) y pasa reconectarUltimoEquipo a js/busqueda_equipos.js.

const EnlaceEquipos = {};

EnlaceEquipos.instalar = function (dep) {
  const state = dep.state;
  const showToast = dep.showToast;
  const addEvent = dep.addEvent;
  const renderLights = dep.renderLights;
  const revisarGrupo = dep.revisarGrupo;
  const actualizarEmergencia = dep.actualizarEmergencia;
  const renderDiagnosticoEnlace = dep.renderDiagnosticoEnlace;
  const renderAvisoCamaraPluma = dep.renderAvisoCamaraPluma;
  const renderAvisoCorte = dep.renderAvisoCorte;
  const renderCarteles = dep.renderCarteles;
  const parseNmeaTelemetry = dep.parseNmeaTelemetry;
  const enviarComandoFirmware = dep.enviarComandoFirmware;
  const closeModal = dep.closeModal;
  const actualizarBotonDesconectar = dep.actualizarBotonDesconectar;
  const btModal = dep.btModal;
  const btnDevice = dep.btnDevice;
  const btStatusDot = dep.btStatusDot;
  const btBtnText = dep.btBtnText;
  const nodeNameEl = dep.nodeNameEl;
  const rssiTextEl = dep.rssiTextEl;
  const btDeviceListContainer = dep.btDeviceListContainer;


  // =========================================================================
  // 6. GESTIÓN BLUETOOTH Y DISPOSITIVOS
  // =========================================================================
  //
  // 6.pre CAMBIAR DE POSTE ES DESCONECTAR PRIMERO, Y ENTERARSE CUANDO NO LO HAS HECHO TU
  //
  // REPORTE DE BANCO DEL 04/09, con los dos modulos encendidos: "dejé todo igual, los dos
  // conectados, y me quería conectar al maestro. Y solito desvinculó este. Y me le dio
  // ese." Funcionaba -- y funcionaba POR ACCIDENTE, que es lo que hay que arreglar.
  //
  // MEDIDO en el plugin instalado, no supuesto:
  //   BluetoothSerial.java:110-113    connect() hace `connectCallback = callbackContext;`
  //                                   y DESPUES bluetoothSerialService.connect(...)
  //   BluetoothSerialService.java:127 ese connect() hace `mConnectedThread.cancel()`
  //                                   sobre la conexion anterior
  //   BluetoothSerialService.java:467 al cerrarse ese socket el hilo de lectura cae en
  //                                   connectionLost()
  //   BluetoothSerial.java:413-418    connectionLost llama a connectCallback.error(...)
  //
  // Junta las cuatro y sale la avería: al conectar con el POSTE 1 estando en el POSTE 2,
  // el `error` de la conexion QUE SE CAE aterriza en el callback DEL POSTE NUEVO, porque
  // la primera linea ya lo habia sustituido. La app pintaria "no se pudo conectar" de un
  // enlace que si abrio. Es una carrera, y por eso el sintoma que se vio en banco fue
  // "solito desvinculó este": el trabajo lo hizo el sistema y la app se entero por los
  // datos, que es enterarse tarde.
  //
  // LA CURA ES PEDIRLO EN VOZ ALTA. disconnect() -- MEDIDO: existe en
  // www/bluetoothSerial.js:13, y en Java (BluetoothSerial.java:122-125) pone
  // `connectCallback = null` ANTES de parar el servicio -- cierra el anterior sin que su
  // connectionLost tenga a quien avisar. Solo entonces se llama al connect() del nuevo, y
  // ese callback ya es suyo y de nadie mas.
  //
  // Y LA OTRA MITAD: SABER CUANDO TE HAN DESCONECTADO. notifyConnectionSuccess() manda su
  // PluginResult con setKeepCallback(true) (BluetoothSerial.java:420-425), asi que el
  // callback de FALLO del connect() sigue vivo DESPUES de haber conectado, y es por donde
  // llega "Device connection was lost". Un error que entra con state.connected ya en true
  // NO es "no se pudo conectar": es "se ha caido". Misma funcion, significados opuestos.
  // Es la familia de N-122 al reves -- alli la app se pintaba enlazada sin haber abierto
  // socket; aqui se quedaria creyendose enlazada despues de que se lo cerraran.
  //
  // LO QUE ESTA APP NO PUEDE SABER, y va escrito para que nadie lo de por cubierto: por que se cayo. El plugin entrega
  // un texto fijo, no un motivo. Si el socket lo cerro el sistema para dejar sitio a otra aplicacion, el aviso llega
  // igual y no lo dice. La causa no se inventa.

  // El estado del enlace se apaga SIEMPRE por aqui: "he desconectado" y "me han desconectado" no dejan la pantalla
  // en estados distintos (un boton que dice enlazado sin estarlo). D-86: js/practica.js pregunta y avisa por estas dos.
  window.appEnlazada = () => state.connected === true; window.appAvisar = (t, m) => { showToast(m); addEvent(t, m); };
  // D-130: la espera de SET_DEG_AUTO es de la conexion que la envio (deviceMac aun suyo): sin "no contesto" en otra.
  function soltarEsperaDegAuto() {
    const e = DegAuto._espera; if (!e) return;
    clearTimeout(e.timer); DegAuto._espera = null; DegAuto.render(); addEvent('cyan', 'SET_DEG_AUTO:' + e.valor +
      ' enviada a ' + state.deviceMac + ': sin acuse en esa conexion, mire la consulta');
  }
  function olvidarEnlace() {
    state.connected = false; soltarEsperaDegAuto(); revisarGrupo(); PantallaOperario.sinPareja(null);   // QA de U, D3
    state.node = null; Instalacion.alStatus(null); state.firmware = {}; renderLights();   // sin enlace: «SIN ENLACE»
    // D-23: los contadores de radio son de UN poste y arrancan con su micro. Si no se
    // olvidan al soltar el enlace, la primera muestra del SIGUIENTE poste se resta contra
    // la ultima del anterior y sale un delta inventado -y casi siempre negativo, que
    // ademas se leeria como "este poste se ha reiniciado"-. Es la misma cautela que A-9
    // sobre el reloj: una lectura atribuida al poste equivocado fabrica una averia.
    if (typeof DiagnosticoEnlace !== 'undefined') { DiagnosticoEnlace.olvidar(); renderDiagnosticoEnlace(); }
    // Y por lo mismo el cartel de la barrera retenida: es de UN poste. Colgado sobre el
    // siguiente mandaria a mirar debajo del brazo que no es, y ademas dice "no va a
    // bajar sola" de una barrera que nadie ha vuelto a mirar.
    if (typeof AvisoCamaraPluma !== 'undefined') { AvisoCamaraPluma.olvidar(); renderAvisoCamaraPluma(); }
    if (typeof AvisoCorte !== 'undefined') { AvisoCorte.olvidar(); renderAvisoCorte(); } CartelesEquipo.olvidar(); renderCarteles();
    if (btnDevice) btnDevice.className = 'btn-top btn-device'; if (btStatusDot) btStatusDot.className = 'status-dot';
    // "Sin equipo" y "Sin enlace" NO son lo mismo, y la diferencia es la que decide si
    // hay algo que reintentar. Se acaba de soltar un socket: si quedaba un equipo
    // elegido, lo que hay es un enlace caido -y un boton de RECONECTAR debajo-, no una
    // app sin equipo. marcarSinEnlace() ya escribe 'Sin enlace' por su cuenta; esto es
    // para que la llamada de despues no se lo pise con algo mas pesimista.
    if (btBtnText) btBtnText.textContent = (state.ultimoEquipo && state.ultimoEquipo.mac)
      ? 'Sin enlace' : 'Sin equipo';
    if (nodeNameEl) nodeNameEl.textContent = '---'; if (rssiTextEl) rssiTextEl.textContent = '(sin enlace)';
    actualizarBotonDesconectar();
    // La punta deja de constar, asi que los mandos vuelven al reparto de "no se sabe que
    // poste hay": las dos emergencias visibles y RETIRAR AMBAR retirado. Lo hace la misma
    // funcion que al conectar, para que las dos no puedan desincronizarse.
    actualizarEmergencia();
  }

  // Cierra el enlace, a peticion del tecnico o para dejar sitio al siguiente poste.
  // `alTerminar` corre en los DOS desenlaces -- se pudo cerrar o no --, porque lo que
  // sigue es abrir el otro y eso hay que intentarlo igual: pararse porque disconnect()
  // se quejo dejaria al tecnico sin ninguno de los dos postes.
  function desconectarEquipo(alTerminar) {
    const bt = (typeof window !== 'undefined') ? window.bluetoothSerial : null;
    const quien = state.deviceName || state.deviceMac || 'el equipo';
    state.telemetriaViva = false; state.ultimoStatusMs = null;   // soltar a proposito no es caida (SPEC_4 9.9 (d))
    if (!bt || typeof bt.disconnect !== 'function') {
      // No se promete lo que no se puede hacer. El estado interno si se limpia -- la app
      // deja de mandar ordenes por un enlace del que ya no se fia --, pero se DICE que el
      // cierre no se llego a pedir al sistema.
      addEvent('red', 'Desconexion NO solicitada al sistema: esta version de la app no ' +
                      'dispone de disconnect(). La app deja de usar el enlace, pero el ' +
                      'socket puede seguir abierto hasta que Android lo cierre.');
      olvidarEnlace();
      if (alTerminar) alTerminar();
      return;
    }
    bt.disconnect(
      () => {
        addEvent('cyan', 'Bluetooth desconectado de ' + quien + ' a peticion de la app.');
        olvidarEnlace();
        if (alTerminar) alTerminar();
      },
      (err) => {
        addEvent('red', 'La desconexion de ' + quien + ' fallo: ' + String(err || '') +
                        '. Se deja de usar el enlace igualmente.');
        olvidarEnlace();
        if (alTerminar) alTerminar();
      }
    );
  }

  // Abre el enlace con un equipo. Aqui YA no queda enlace anterior: de eso se ocupa
  // cambiarDeEquipo(), que es quien decide si habia algo que cerrar antes.
  function abrirEnlace(mac, name, emparejada) {
    const bt = window.bluetoothSerial;
    // Los rotulos de espera se ponen AQUI y no en el manejador del clic: entre pulsar la
    // fila y llegar a esta linea puede haber pasado un disconnect(), y olvidarEnlace()
    // deja escrito "Sin equipo". Ponerlos antes seria dejar que el cierre del poste viejo
    // borrara el aviso del nuevo.
    if (btStatusDot) btStatusDot.className = 'status-dot';
    if (btBtnText) btBtnText.textContent = 'Esperando equipo';
    if (nodeNameEl) nodeNameEl.textContent = 'IDENTIFICANDO...';
    if (!emparejada) {
      // El emparejamiento lo pide ANDROID, no la app, y su dialogo sale por encima de
      // esta pantalla. Avisado antes es un paso mas; sin avisar es una ventana rara que
      // se cancela por instinto y deja un "no se pudo conectar" sin explicacion.
      showToast('Android va a pedir vincular este equipo');
      addEvent('cyan', 'Conectando a ' + name + ' (' + mac + '), que NO esta emparejado ' +
                       'en este telefono: Android pedira vincular antes de abrir el ' +
                       'enlace. Acepte ese aviso del sistema.');
    }
    bt.connect(
      mac,
      () => {
        state.connected = true; renderLights(); PantallaOperario.sinPareja(null);   // D-80: «esperando su primer dato»
        revisarGrupo();   // 9.14 (b): el grupo de ESTA MAC desde el enlace
        if (btnDevice) btnDevice.className = 'btn-top btn-device connected';
        actualizarBotonDesconectar();
        showToast(`🔗 Enlazado a: ${name}`);
        addEvent('green', `Bluetooth conectado: ${name} (${mac})`);

        // N-75: sin esto la app no oye al equipo. El firmware emite $STATUS solo,
        // cada segundo, asi que NO se pide el estado al conectar (N-66: GET_STATUS no
        // existe en ninguna punta y lo primero que veia el tecnico era un $ERR).
        bt.subscribe(
          '\n',
          (data) => {
            if (data && String(data).trim()) parseNmeaTelemetry(String(data).trim());
          },
          (err) => console.warn('Error en suscripcion serie:', err)
        );
        Exportar.pedirVersionAlConectar(state, enviarComandoFirmware, addEvent, dep.TIMEOUT_ENLACE_MS);
      },
      (err) => {
        // LAS DOS DESGRACIAS ENTRAN POR AQUI Y NO SON LA MISMA -- ver 6.pre. Con
        // state.connected ya en true el socket llego a abrir, o sea que esto es una
        // CAIDA; con state.connected en false es que no abrio nunca. Decirle "no se pudo
        // conectar" a alguien que lleva diez minutos operando el cruce le manda a buscar
        // el fallo donde no esta.
        const seCayo = state.connected === true;
        olvidarEnlace();
        if (seCayo) {
          showToast('⚠️ Se perdió el enlace con ' + name);
          addEvent('red', 'ENLACE PERDIDO con ' + name + ' (' + mac + '): ' +
                          String(err || '') + '. La app ya NO esta hablando con ese ' +
                          'poste: lo que quedo en pantalla es lo ultimo que dijo, no lo ' +
                          'que hace ahora. Vuelva a conectar para seguir operando.');
        } else {
          // NO CONECTAR NO ES CONECTAR. Se dice, y se deja el estado en falso: pintar
          // "Enlazado" sobre un socket que no abrio es lo que hizo que el banco
          // gastara cuatro pasos buscando el fallo en el modulo.
          // D-144 (9.9 (g) 5): sin emparejar, el aviso reintenta ESTE equipo por cambiarDeEquipo(); emparejado, no hay aviso.
          if (emparejada) showToast('❌ No se pudo conectar'); PantallaOperario.sinPareja(emparejada ? null : () => cambiarDeEquipo(mac, name, false, ''));
          addEvent('red', `Bluetooth NO conectado a ${name} (${mac}): ${err}. ` +
                          (emparejada
                            ? 'Comprueba que el equipo este encendido y a la vista.'
                            : 'Este equipo no estaba emparejado: si cancelo el aviso de ' +
                              'vinculacion de Android, vuelva a pulsarlo y acepte.'));
        }
      }
    );
  }

  // La puerta unica para elegir equipo. Un solo enlace a la vez, y el cambio se anuncia
  // NOMBRANDO LOS DOS POSTES: con dos modulos que se llaman casi igual (N-129),
  // enterarse tarde de a cual se le esta hablando es el error caro de esta app.
  function cambiarDeEquipo(mac, name, emparejada, veniaDe) {
    // Se guarda para poder RECONECTAR sin volver a buscar. Es el dato que le faltaba a
    // la app cuando el equipo se reinicia solo: sabia con quien HABIA hablado pero no
    // si aquella fila estaba emparejada, que es lo que decide si Android va a pedir
    // vinculacion al reintentar.
    state.ultimoEquipo = { mac: mac, name: name, emparejada: !!emparejada }; PantallaOperario.sinPareja(null);   // el aviso era del anterior

    // SE DESCONECTA SIEMPRE QUE HAYA HABIDO UN ENLACE ANTES, aunque la app ya lo de por
    // caido. Y ese "aunque" es el bloqueo de campo del 04/09, no una precaucion:
    //
    //   "no conecta despues de que el maestro se apaga y prende. Y se apaga y se
    //    prende, no vuelve a conectar... reinicie la aplicacion y comenzo."
    //
    // MEDIDO en el plugin, y es la carrera de 6.pre vista desde el otro lado. Cuando el
    // equipo se reinicia, su socket muere y el hilo de lectura cae en connectionLost()
    // (BluetoothSerialService.java:467). Si el tecnico pulsa la fila para reintentar, la
    // primera linea de connect() SUSTITUYE connectCallback por el del intento nuevo
    // (BluetoothSerial.java:110-113) y el error de la conexion MUERTA aterriza encima:
    // la app pinta "no se pudo conectar" sobre un enlace que estaba abriendo bien. Desde
    // fuera se ve exactamente como lo describio el responsable -- no vuelve a conectar --
    // y cerrar la app "lo arregla" porque estrena el plugin entero.
    //
    // disconnect() rompe la carrera: pone `connectCallback = null` ANTES de parar el
    // servicio (BluetoothSerial.java:122-125), asi que el connectionLost del socket
    // muerto ya no tiene a quien avisar. Sobre un servicio ya parado es inofensivo --
    // stop() es idempotente --, asi que se paga una llamada de mas por no depender de
    // que state.connected acierte.
    const habiaSocket = state.connected || !!state.deviceMac;
    if (veniaDe) {
      showToast('Soltando ' + veniaDe + ' para ir a ' + name);
      addEvent('cyan', 'CAMBIO DE POSTE: la app se desconecta de ' + veniaDe + ' para ' +
                       'conectarse a ' + name + '. El Bluetooth clasico es punto a ' +
                       'punto: no se puede hablar con los dos a la vez.');
    } else if (habiaSocket) {
      addEvent('cyan', 'Se cierra el enlace anterior antes de abrir el nuevo, para que ' +
                       'un socket muerto -por ejemplo el que deja el equipo al ' +
                       'reiniciarse- no se confunda con un fallo del intento de ahora.');
    }
    if (habiaSocket) {
      desconectarEquipo(() => abrirEnlace(mac, name, emparejada));
      return;
    }
    abrirEnlace(mac, name, emparejada);
  }

  // RECONECTAR CON EL ULTIMO EQUIPO, SIN CERRAR LA APP NI VOLVER A BUSCAR.
  //
  // Es la mitad que faltaba del bloqueo: la app YA sabia declarar la caida -- el
  // vigilante de silencio de abajo lleva ahi desde antes de este encargo, con su cota
  // de TIMEOUT_ENLACE_MS -- pero despues de declararla no ofrecia nada, y el unico
  // camino de vuelta era el que uso el responsable: cerrar y abrir.
  function reconectarUltimoEquipo() {
    const eq = state.ultimoEquipo;
    if (!eq || !eq.mac) {
      showToast('No hay ningun equipo al que volver');
      return;
    }
    showToast('Reconectando con ' + eq.name + '...');
    addEvent('cyan', 'Reintento de enlace con ' + eq.name + ' (' + eq.mac + '). Se ' +
                     'cierra primero el socket anterior: si el equipo se reinicio, el ' +
                     'que quedaba abierto es de una sesion que ya no existe.');
    // Pasa por la MISMA puerta que elegir una fila: si reconectar tuviera su propio
    // camino, seria una segunda copia del orden desconectar-antes-de-conectar, y la
    // proxima vez que ese orden cambie solo se acordaria uno de los dos.
    cambiarDeEquipo(eq.mac, eq.name, eq.emparejada, '');
  }

  // N-124: EL OYENTE VIVE EN EL CONTENEDOR, NO EN CADA FILA.
  //
  // Las filas ya no las trae index.html: las pinta pintarEquipos() con lo que devuelve
  // el sistema, y se sustituyen enteras en cada escaneo. Un querySelectorAll().forEach()
  // al cargar dejaria los manejadores colgados de elementos que ya no estan en el
  // documento: el clic no haria nada y nadie avisaria -la prueba muerta con forma de
  // interfaz-. El contenedor si sobrevive a los repintados, asi que el oyente va en el.
  if (btDeviceListContainer) {
    btDeviceListContainer.addEventListener('click', (ev) => {
      const dev = ev.target && ev.target.closest ? ev.target.closest('.bt-device-item') : null;
      if (!dev) return;
      const name = dev.getAttribute('data-name');
      const mac = dev.getAttribute('data-mac');
      // '0' = el escaneo lo encontro encendido pero este telefono no lo conoce. Android
      // va a abrir su propio dialogo de vinculacion al conectar, y el tecnico tiene que
      // saberlo ANTES de pulsar: si no, ve aparecer una ventana del sistema encima de la
      // app y la respuesta natural es cancelarla.
      const emparejada = dev.getAttribute('data-emparejado') !== '0';
      // DE QUE POSTE SE VIENE, LEIDO AQUI Y NO MAS ABAJO. Doce lineas mas adelante este
      // mismo manejador machaca state.deviceName y pone state.connected en false, asi que
      // preguntarlo despues seria preguntarle al equipo nuevo de donde viene. Con dos
      // modulos que se llaman casi igual (N-129), ese es justo el dato que hay que
      // acertar para poder decir "suelto A y voy a B".
      const veniaDe = state.connected ? (state.deviceName || state.deviceMac || '') : '';
      // Sin direccion no hay a quien llamar: connect() marca el MAC. Una fila sin el no
      // se pinta (ver pintarEquipos), y si aun asi llegara aqui se para ANTES de tocar
      // el estado, para no dejar la app "eligiendo" un equipo al que no puede llamar.
      if (!mac) return;

      // N-75: el PIN que viajara en CMD:PIN: sale del selector del modal, no de un
      // literal. Si alguien cambia el PIN del equipo, se cambia aqui y el pack
      // documentos_03 comprueba que el elegido sea uno de los que acepta el C++.
      const pinSel = document.querySelector('input[name="bt_pin_opt"]:checked');
      if (pinSel) state.correctPin = pinSel.value;

      // N-122: state.connected NO se pone aqui. Pulsar una fila elige un equipo; no abre
      // un socket. Lo pone a true unicamente el callback de exito de connect(), abajo.
      state.connected = false; soltarEsperaDegAuto();   // D-130: antes de que deviceMac pase al poste nuevo
      state.deviceName = name;
      state.deviceMac = mac; revisarGrupo();

      // N-124: LA PUNTA NO SE ADIVINA. Aqui se leia un data-node de la fila, y las dos
      // filas venian escritas a mano en el HTML: la app afirmaba tener un MAESTRO
      // delante antes de que ningun equipo hubiera hablado. Con la lista pintada por el
      // escaneo eso ya no se puede saber al pulsar, y no hace falta: NODE: del $STATUS
      // llega cada segundo y lo escribe abajo (parseNmeaTelemetry). Hasta entonces el
      // rotulo dice lo unico que consta, que es que todavia no se sabe.
      state.node = null; Instalacion.alStatus(null);
      if (nodeNameEl) nodeNameEl.textContent = 'IDENTIFICANDO...';
      // Con la punta sin identificar actualizarEmergencia() ensena LAS DOS emergencias
      // -cada una con su poste delante- y retira RETIRAR AMBAR, que es la unica que
      // abre paso. Ese reparto ya estaba escrito alli y es justo el que toca ahora.
      actualizarEmergencia();
      // Abrir el socket NO es tener al equipo hablando: el modulo Bluetooth puede
      // emparejar con el poste alimentado y el micro colgado. Hasta que no llegue el
      // primer $STATUS la cabecera dice "Esperando", y quien lo pone en verde es
      // marcarConEnlace() desde la trama. Antes se pintaba "Enlazado" aqui mismo, con
      // lo cual el rotulo confirmaba el clic del usuario, no la presencia del equipo.
      //
      // Las dos cosas que se pintan aqui dicen COSAS DISTINTAS, y antes decian la misma:
      //   - btnDevice .connected = hay un equipo ELEGIDO y la suscripcion serie abierta.
      //   - el punto de estado y el rotulo = el equipo esta HABLANDO. Eso no se sabe
      //     todavia, asi que se queda apagado hasta el primer $STATUS.
      // El modulo Bluetooth empareja igual con un poste alimentado y el micro colgado.
      //
      // N-122: el `.connected` del boton TAMPOCO se pinta aqui. El comentario de arriba
      // decia que era cierto "en este instante" porque daba por abierta la suscripcion;
      // no lo estaba. Se pinta en el callback de exito de connect(), que es el unico
      // momento en que hay socket de verdad.
      if (btStatusDot) btStatusDot.className = 'status-dot';
      if (btBtnText) btBtnText.textContent = 'Esperando equipo';

      // 🔴 N-122: AQUI FALTABA connect(), Y ES LA LINEA QUE BLOQUEO EL BANCO DEL 03/09.
      //
      // Esto llamaba a subscribe() SIN HABER ABIERTO NINGUN SOCKET. En
      // cordova-plugin-bluetooth-serial, subscribe() y write() operan sobre la conexion
      // que abre connect(mac): sin ella no hay BluetoothSerialService, asi que la
      // suscripcion no engancha y el write() de enviarComandoFirmware() (linea 278) se
      // va al vacio. La app se pintaba "Enlazado" por haber pulsado una fila.
      //
      // Y no era un olvido invisible: `js/bluetooth_driver.js` tiene la llamada escrita
      // -conectarNativoSPP()- y esta en la lista de HUERFANOS_CONOCIDOS de
      // app_07_generadores_de_trama con el motivo "app.js habla por window.
      // bluetoothSerial". Ese motivo era MEDIO cierto: app.js usa write, subscribe y
      // list... y no usa connect, que es justo la que hace funcionar a las otras tres.
      // Un huerfano aceptado por una razon que no se comprobo entera.
      //
      // EL ORDEN IMPORTA Y ES LA MITAD DEL ARREGLO: state.connected solo puede ser
      // cierto DESPUES de que el socket abra. Antes se ponia a true al pulsar la fila,
      // asi que la guarda de enviarComandoFirmware() -`&& state.connected`- daba paso a
      // ordenes que no tenian por donde salir, y el operario las veia aceptadas.
      if (typeof window !== 'undefined' && window.bluetoothSerial &&
          typeof window.bluetoothSerial.connect === 'function') {
        // EL BLUETOOTH CLASICO ES PUNTO A PUNTO: para hablar con el otro poste hay que
        // soltar este. Si habia un enlace abierto se cierra A PROPOSITO y se dice de que
        // poste se sale, ANTES de abrir el siguiente. Ver el bloque 6.pre de arriba.
        cambiarDeEquipo(mac, name, emparejada, veniaDe);
      } else {
        // Fuera del APK no hay radio. Se declara en vez de simular un enlace.
        state.connected = false;
        addEvent('red', 'Sin radio en este entorno: no se abrio ningun enlace.');
      }

      closeModal(btModal);
    });
  }


  return { desconectarEquipo: desconectarEquipo, reconectarUltimoEquipo: reconectarUltimoEquipo };
};

if (typeof module !== 'undefined' && module.exports) {
  module.exports = EnlaceEquipos;
}
