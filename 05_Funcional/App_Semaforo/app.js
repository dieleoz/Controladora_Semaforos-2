// ===== IOT-VIAL · Centro de Control Semafórico V9.0 =====
// JavaScript Logic: Full Interactive Controller with TDD-validated Features

document.addEventListener('DOMContentLoaded', () => {

  // --- STATE VARIABLES ---
  const state = {
    role: 'OPERARIO', // 'OPERARIO' o 'TECNICO'
    connected: false,
    connectionType: 'NONE',
    // NADA DE ESTO SE SABE AL ABRIR LA APP, y aqui venia relleno: un Maestro llamado
    // M-2026-A1B2 en AUTO, con la fase V1_R2 corriendo, 38 s de cuenta, RF al 98%, el
    // RTT en 82 ms y la bateria a 12,6 V. Ninguno de esos numeros salio nunca de una
    // trama: son el aspecto que tiene un cruce sano, escritos a mano. La app arrancaba
    // enseñando un equipo en servicio y solo dejaba de mentir si alguien conectaba uno
    // de verdad. Ahora arranca sin saber, y renderLights()/marcarSinEnlace() lo pintan
    // como lo que es. Lo que no viene del equipo se queda en null.
    deviceName: null,
    deviceMac: null,
    site: 'KM 12 · EL SISGA',
    node: null, // 'MAESTRO', 'ESCLAVO', 'REPETIDOR' - lo dice $STATUS
    serie: null,
    modo: null, // 'AUTO', 'MANUAL', 'AMBAR', 'ROJO_TOTAL'
    estadoLuces: null, // V1_R2, Y1_R2, R1_R2, R1_V2, R1_Y2, AMBAR_FAIL, ALL_RED
    countdown: 0,
    countdownMax: 0,
    tiempoVerdeMin: 2,
    tiempoRojoMin: 2,
    tiempoDespejeSeg: 15,
    // rfQuality/rfRtt son EL ULTIMO VALOR QUE VINO EN UNA TRAMA, y null mientras no
    // haya venido ninguno. No se tocan en ningun otro sitio: los escribe pintarEnlace()
    // -ver 2.bis- y los lee el reporte de WhatsApp, que hasta hoy publicaba
    // "Enlace RF: null%" porque el unico camino que los escribia era el del puente de
    // PC, que ya no existe.
    rfQuality: null,
    rfRtt: null,
    // Instante de la ultima MEDIDA de enlace, que NO es el instante de ahora. Sin este
    // sello, "70%" en pantalla no se distingue de "70% hace veinte minutos".
    rfMedidaMs: null,
    // Tramo del indicador (BIEN / JUSTO / CAYENDO / SIN_DATO) con el que se anoto la
    // ultima vez. Cambiar de tramo es lo que dispara una anotacion fuera de turno: es
    // el instante que el tecnico esta buscando en el registro.
    rfTramo: null,
    rfUltimaMuestraMs: null,
    battery: null,
    hora: null,
    pin: '',
    correctPin: '1234',
    // N-75: la app conocia el PIN y lo inyectaba en TODOS los comandos, asi que la
    // barrera del firmware -que guarda lo que ABRE paso o mueve luces- la abria ella
    // misma. Un comando que sale con un PIN que el operario no tecleo no es una
    // autorizacion. Esta bandera dice si alguien lo puso en esta sesion.
    pinVerificado: false,
    accionPendiente: null,
    // El equipo esta HABLANDO (llega $STATUS). Distinta de connected, que solo dice
    // que el socket serie esta abierto. Ver el watchdog de enlace mas abajo.
    telemetriaViva: false,
    // Instante de la ultima trama $STATUS recibida. null = todavia no ha hablado
    // ningun equipo en esta sesion, que NO es lo mismo que un equipo con enlace.
    ultimoStatusMs: null,
    // Hubo una caida que todavia no tiene su REGRESO anotado. Distinta de
    // telemetriaViva: la primera conexion de la sesion no es una vuelta de nada.
    huboCaida: false,
    // El estrangulador de anotaciones de RECHAZO en la bitacora. Ver
    // registrarRechazoEnlace(): mil tramas malas seguidas son UN suceso, y anotarlas
    // una a una vaciaria el tope de 400 de la bitacora con basura, llevandose por
    // delante la historia del enlace que es justo lo que se quiere conservar.
    rechazoMotivo: null,
    rechazoDesdeMs: null,
    rechazoUltimaMs: null,
    rechazoCuenta: 0,
    rechazoAnotadoMs: null,
    // Filtro de la vista de depuracion. Es SOLO un filtro de lo que se pinta: no borra
    // nada de la cinta y el rotulo dice siempre cuantas se estan ocultando.
    depuSoloRechazadas: false,
    courierSnapshot: null,
    courierTimerInterval: null,
    courierSecondsElapsed: 0,
    events: [
      { time: new Date().toLocaleTimeString(), type: 'cyan', msg: 'Sistema IOT-VIAL V9.0 inicializado correctamente.' }
    ]
  };

  // --- DOM ELEMENTS ---
  const btnToggleRole = document.getElementById('btn-toggle-role');
  const roleIconEl = document.getElementById('role-icon');
  const roleLabelEl = document.getElementById('role-label');
  const adminTabs = document.querySelectorAll('.admin-tab');

  const btnDevice = document.getElementById('btnDevice');
  const btModal = document.getElementById('bt-modal');
  const modalBtClose = document.getElementById('modal-bt-close');
  const btnScanBluetoothLive = document.getElementById('btn-scan-bluetooth-live');
  const btDeviceListContainer = document.getElementById('bt-device-list-container');
  const btStatusDot = document.getElementById('bt-status-dot');
  const btBtnText = document.getElementById('bt-btn-text');

  const currentSiteNameEl = document.getElementById('current-site-name');
  const btnSelectSite = document.getElementById('btn-select-site');
  const siteModal = document.getElementById('site-modal');
  const modalSiteClose = document.getElementById('modal-site-close');
  const dynamicSiteListEl = document.getElementById('dynamic-site-list');
  const siteSearchInput = document.getElementById('site-search-input');
  const btnOpenAddSite = document.getElementById('btn-open-add-site');

  const nodeNameEl = document.getElementById('node-name');
  const rssiTextEl = document.getElementById('rssi-text');

  // Traffic Light Lamps & Central Ring
  const s1Red = document.getElementById('s1-red');
  const s1Amber = document.getElementById('s1-amber');
  const s1Green = document.getElementById('s1-green');
  const s1Text = document.getElementById('s1-text');

  const s2Red = document.getElementById('s2-red');
  const s2Amber = document.getElementById('s2-amber');
  const s2Green = document.getElementById('s2-green');
  const s2Text = document.getElementById('s2-text');

  const cdNumEl = document.getElementById('cd-num');
  const ringProgressEl = document.getElementById('ring-progress');
  const phaseDescEl = document.getElementById('phase-desc');
  const badgeModoEl = document.getElementById('badge-modo');

  // Metrics
  const rfQualityEl = document.getElementById('rf-quality');
  const rfRttEl = document.getElementById('rf-rtt');
  const rfEstadoEl = document.getElementById('rf-estado');
  const rfBarraEl = document.getElementById('rf-barra');
  const rfSelloEl = document.getElementById('rf-sello');
  const batVoltageEl = document.getElementById('bat-voltage');
  const batStatusEl = document.getElementById('bat-status');

  // Bitacora del enlace (pestana de eventos)
  const registroTiraEl = document.getElementById('registro-tira');
  const registroListaEl = document.getElementById('registro-lista');
  const registroResumenEl = document.getElementById('registro-resumen');
  const btnRegistroCsv = document.getElementById('btn-registro-csv');
  const btnRegistroLimpiar = document.getElementById('btn-registro-limpiar');

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

  // Operario Field Buttons
  const btnOpAuto = document.getElementById('btn-op-auto');
  const btnOpStep = document.getElementById('btn-op-step');
  const btnOpAmber = document.getElementById('btn-op-amber');
  const btnOpEmergency = document.getElementById('btn-op-emergency');
  const btnOpAmbarEmergencia = document.getElementById('btn-op-ambar-emergencia');
  const btnOpCancelarAmbar = document.getElementById('btn-op-cancelar-ambar');
  const emergenciaHintEl = document.getElementById('emergencia-hint');
  const emergenciaSubMaestroEl = document.getElementById('emergencia-maestro-sub');
  const emergenciaSubEsclavoEl = document.getElementById('emergencia-esclavo-sub');
  const btnOpMenu = document.getElementById('btn-op-menu');
  const padPosteEl = document.getElementById('pad-poste');

  // Mandos de tecnico que no salen por data-cmd porque cada uno tiene una condicion
  // propia: ALCANCE viaja sin PIN, DEMANDA solo vale en un modo concreto y DEGRADADO
  // pasa antes por un dialogo.
  const btnModoAlcance = document.getElementById('btn-modo-alcance');
  const btnDemanda = document.getElementById('btn-demanda');
  const demandaHintEl = document.getElementById('demanda-hint');
  const btnModoDegradado = document.getElementById('btn-modo-degradado');
  const degradadoModal = document.getElementById('degradado-modal');
  const modalDegradadoClose = document.getElementById('modal-degradado-close');
  const chkDegradadoVerificado = document.getElementById('chk-degradado-verificado');
  const btnDegradadoCancelar = document.getElementById('btn-degradado-cancelar');
  const btnDegradadoConfirmar = document.getElementById('btn-degradado-confirmar');

  // Courier RTC Elements
  const btnCourierCapture = document.getElementById('btn-courier-capture');
  const btnCourierInject = document.getElementById('btn-courier-inject');
  const courierSnapshotText = document.getElementById('courier-snapshot-text');
  const courierTimerDigits = document.getElementById('courier-timer-digits');
  const btnSyncRtc = document.getElementById('btn-sync-rtc');
  const rtcSyncDigits = document.getElementById('rtc-sync-digits');
  const btnStartTestLeds = document.getElementById('btn-start-test-leds');

  // Form Tiempos
  const formTiempos = document.getElementById('form-tiempos');
  const numTiempoVerde = document.getElementById('num-tiempo-verde');
  const numTiempoRojo = document.getElementById('num-tiempo-rojo');
  const numTiempoDespeje = document.getElementById('num-tiempo-despeje');

  // Events Feed & Export
  const eventFeedEl = document.getElementById('event-feed');
  const btnShareWhatsapp = document.getElementById('btn-share-whatsapp');
  const btnExportCsv = document.getElementById('btn-export-csv');

  // PIN Modal
  const pinModal = document.getElementById('pin-modal');
  const modalPinClose = document.getElementById('modal-pin-close');
  const pinDots = [
    document.getElementById('pin-d1'),
    document.getElementById('pin-d2'),
    document.getElementById('pin-d3'),
    document.getElementById('pin-d4')
  ];
  const btnPinClear = document.getElementById('btn-pin-clear');
  const btnPinOk = document.getElementById('btn-pin-ok');

  // Toast
  const toastMsg = document.getElementById('toast-msg');

  // =========================================================================
  // 1. PROTOCOLO DE COMANDOS FIRMWARE C++ (STM32 BLUETOOTH CONTRACT)
  // =========================================================================
  // Las ordenes que el firmware acepta SIN autenticar, con el nombre EXACTO que viaja
  // por el cable, para que el censo del pack app_01_comandos vea lo mismo que ve el
  // micro. El criterio no lo elige la app: esta escrito en el despachador y es el
  // mismo para las tres -el PIN guarda lo que ABRE paso, no lo que lo para-.
  //
  //   FORZAR_ROJO      Maestro/src/bluetooth.cpp. Rojo fijo en las dos vias.
  //   AMBAR_EMERGENCIA Esclavo/src/bluetooth.cpp. Ambar intermitente y talanquera
  //                    ABIERTA. Es la MISMA tecla de emergencia con OTRA maniobra, y
  //                    por eso lleva otro nombre: hasta el 28/08 esa punta tambien
  //                    respondia a FORZAR_ROJO y acusaba "rojo forzado, correcto" sin
  //                    haber puesto un solo rojo. Ahora el Esclavo rechaza el literal
  //                    viejo y el motivo del $ERR nombra el nuevo.
  //   SET_MODO:MENU    Maestro/src/bluetooth.cpp. Deja el equipo en la pantalla, sin ciclo.
  //   SET_MODO:ALCANCE Maestro/src/bluetooth.cpp. Deja el equipo en rojo fijo.
  //
  // Las cuatro se aceptan TAMBIEN con PIN en el micro, asi que meterlas aqui no abre
  // ninguna puerta nueva: lo que hace es que se puedan usar sin teclear nada, que es
  // justo el motivo por el que el firmware las dejo sin clave. En particular MENU es
  // la vuelta atras universal, y una vuelta atras que exige recordar una clave delante
  // de un cruce parado no es una vuelta atras.
  //
  // Y la exencion vale para LAS DOS ordenes de emergencia, no solo para la que dice
  // "rojo": la caida segura del Esclavo es su ambar, y una caida segura que pide clave
  // no es una caida segura. El criterio no cambia con el nombre del literal.
  const SIN_PIN = ['FORZAR_ROJO', 'AMBAR_EMERGENCIA', 'SET_MODO:MENU', 'SET_MODO:ALCANCE'];

  // DEVUELVE SI LA ORDEN LLEGO A SALIR, y el que llama TIENE QUE MIRARLO.
  //
  // Esta funcion se planta y no escribe un byte cuando falta el PIN, y hasta hoy no
  // se lo decia a nadie: era un `return` seco. Tres manejadores -el reloj del
  // celular, la inyeccion del Courier y el formulario de tiempos- imprimian su linea
  // de exito EN VERDE Y EN PASADO justo debajo de la llamada, asi que sin PIN
  // verificado la app anunciaba tres cosas que no habia enviado. Es la barrera del
  // $ACK que no mira (CLAUDE.md 6) con las puntas cambiadas: aqui el que miente es
  // el telefono, y el tecnico se va del poste igual.
  //
  // El molde de como se hace bien esta en la botonera de campo de mas abajo -"orden
  // enviada", en cyan, y el resultado lo pintan $ACK y $STATUS-. Devolver un bool es
  // lo que permite copiarlo: pulsar un boton no es saber que el equipo obedecio, y
  // ni siquiera es saber que la orden salio.
  function enviarComandoFirmware(comando, args = '') {
    // La excepcion es el rojo de emergencia: bluetooth.cpp:70-82 lo acepta SIN PIN a
    // proposito -el PIN guarda lo que abre paso, no lo que lo para-.
    if (!SIN_PIN.includes(comando) && !state.pinVerificado) {
      console.warn('[TX BLOQUEADO] sin PIN verificado:', comando);
      addEvent('red', 'Comando ' + comando + ' no enviado: falta autorizacion con PIN.');
      return false;
    }
    const pin = state.correctPin;
    let rawCmd = '';
    if (SIN_PIN.includes(comando)) {
      rawCmd = 'CMD:' + comando + '\r\n';
    } else if (args) {
      rawCmd = `CMD:PIN:${pin}:${comando}:${args}\r\n`;
    } else {
      rawCmd = `CMD:PIN:${pin}:${comando}\r\n`;
    }

    console.log('[TX BLUETOOTH STM32]:', rawCmd.trim());

    // 1. Si está en App Nativa Android (APK con Bluetooth físico)
    if (typeof window !== 'undefined' && window.bluetoothSerial && state.connected) {
      window.bluetoothSerial.write(rawCmd, 
        () => console.log(`[BT TX SUCCESS] -> ${rawCmd.trim()}`),
        (err) => console.error(`[BT TX ERROR] -> ${err}`)
      );
    }

    // 2. Si está en Navegador / Web (Puente directo con Python Firmware Simulator)
    if (typeof fetch !== 'undefined') {
      fetch('/api/cmd', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ cmd: rawCmd.trim() })
      })
      .then(res => res.json())
      .then(data => {
        console.log('[PUENTE PYTHON STM32]:', data);
        if (data.resp_nmea) {
          addEvent('cyan', `STM32 Sim: ${data.resp_nmea}`);
        }
      })
      .catch(() => {
        // Modo offline sin servidor puente
      });
    }

    // `true` significa EXACTAMENTE "la orden se escribio a la salida", ni un milimetro
    // mas. No dice que el equipo la recibiera -la radio puede estar caida-, ni que la
    // aceptara -eso lo dice $ACK- ni que la ejecutara -eso lo dice $STATUS-. Quien
    // llame no puede escribir "hecho" con este bool; solo puede escribir "enviada".
    return true;
  }

  // =========================================================================
  // 1.bis LA HORA QUE SE MANDA AL EQUIPO SE COMPONE A MANO, NUNCA CON EL LOCALE
  // =========================================================================
  // EL RELOJ ENTRABA 12 HORAS TARDE Y LAS DOS PUNTAS CONTESTABAN RESULT:OK.
  //
  // Las dos puertas de SET_RTC formaban la hora con `new Date().toLocaleTimeString()`.
  // MEDIDO en este equipo, con el locale de campo:
  //
  //     es-CO  ->  "6:25:00 p. m."     <- para las 18:25
  //     es-ES  ->  "18:25:00"
  //     en-US  ->  "6:25:00 PM"
  //
  // Y MEDIDO en C, compilando el mismo sscanf que usa el firmware
  // (Maestro/src/bluetooth.cpp:307, Esclavo/src/bluetooth.cpp:242):
  //
  //     sscanf("2026-08-31,6:25:00 p. m.", "%d-%d-%d,%d:%d:%d")  ->  n=6, h=6
  //
  // El sufijo se queda fuera de la conversion, `n` vale 6, EL COMANDO SE ACEPTA y el
  // equipo se queda con las 06:25. Nada en la cadena puede detectarlo: 06:25 es una
  // hora perfectamente valida. Los tres $ACK son HONESTOS -miraron, y lo que miraron
  // entro bien-, que es lo que lo vuelve invisible. Y falla SOLO POR LA TARDE, asi
  // que una prueba de manana lo da por bueno. (De propina, tambien medido:
  // "12:05:00 a. m." -las 00:05- entra como 12:05.)
  //
  // LO QUE CUELGA DE ESA HORA: es la que autoriza el Modo Degradado, el unico modo
  // que enciende verde sin confirmacion del otro extremo.
  //
  // toTimeString() SI vale -su formato lo fija la especificacion, no el locale- pero
  // se compone con los getters para que no haya que saberse esa diferencia de
  // memoria, y para que el pack app_06_formato_de_hora pueda exigirlo por texto.
  function horaLocal24(d = new Date()) {
    const dd = (n) => String(n).padStart(2, '0');
    return dd(d.getHours()) + ':' + dd(d.getMinutes()) + ':' + dd(d.getSeconds());
  }

  // Y LA MISMA LINEA MANDABA EL DIA DE MANANA CADA NOCHE.
  //
  // La fecha salia de `new Date().toISOString().slice(0, 10)`, y toISOString() es UTC.
  // MEDIDO: local 2026-08-31 19:30 en UTC-5 -> "2026-09-01T00:30:00.000Z" -> el
  // comando viaja con el dia 1. Desde las 19:00 locales, todas las noches, el dia del
  // mes que entra al RTC es el siguiente. Se compone tambien a mano, con los getters
  // locales, que es lo unico que no depende ni de la zona ni del idioma.
  function fechaLocalISO(d = new Date()) {
    const dd = (n) => String(n).padStart(2, '0');
    return d.getFullYear() + '-' + dd(d.getMonth() + 1) + '-' + dd(d.getDate());
  }

  // --- TOAST & FEEDBACK HELPERS ---
  function showToast(text, duration = 3000) {
    if (!toastMsg) return;
    toastMsg.textContent = text;
    toastMsg.classList.add('show');
    if ('vibrate' in navigator) navigator.vibrate(50);
    setTimeout(() => {
      toastMsg.classList.remove('show');
    }, duration);
  }

  function addEvent(type, msg) {
    const time = new Date().toLocaleTimeString();
    state.events.unshift({ time, type, msg });
    if (state.events.length > 30) state.events.pop();
    renderEvents();
  }

  function renderEvents() {
    if (!eventFeedEl) return;
    eventFeedEl.innerHTML = '';
    state.events.forEach(ev => {
      const item = document.createElement('div');
      // La clase no es cosmetica: es como el arnes de DOM comprueba que un rechazo
      // del firmware ($ERR) se muestra en vez de tragarselo.
      item.className = 'event-item';
      item.style.padding = '8px 10px';
      item.style.marginBottom = '6px';
      item.style.borderRadius = '8px';
      item.style.background = 'rgba(255,255,255,0.03)';
      item.style.borderLeft = `3px solid var(--${ev.type || 'cyan'}-lamp, var(--cyan-neon))`;
      item.style.fontSize = '11px';
      item.innerHTML = `
        <span style="color: var(--text-muted); font-family: var(--font-mono); margin-right: 6px;">[${ev.time}]</span>
        <span style="color: var(--text-primary);">${ev.msg}</span>
      `;
      eventFeedEl.appendChild(item);
    });
  }

  // =========================================================================
  // 1.ter EL INDICADOR DE ENLACE - Y LA DIFERENCIA ENTRE "VA MAL" Y "NO LO SE"
  // =========================================================================
  // QUE MIDE ESTE NUMERO, QUE NO ES LO QUE PARECE UNA BARRA DE SENAL.
  //
  // El campo RF: de $STATUS sale de coordinador_calidadEnlace()
  // (Maestro/src/coordinador.cpp:845): el PORCENTAJE DE LOS ULTIMOS 10 LATIDOS QUE
  // CONTESTARON, uno cada 3 s, o sea una ventana de 30 s. Y RTT: sale de
  // coordinador_tiempoRespuestaMs() (:857), una media exponencial del viaje de ida y
  // vuelta.
  //
  // NO ES UN RSSI Y LA PANTALLA TIENE QUE DECIRLO. No hay dBm en ninguna parte de este
  // enlace, y por decision del 31/08 no se va a medir potencia. La consecuencia
  // practica, que es la que le importa a quien esta subido a la escalera: este numero
  // NO dice si la culpa es de la antena, del cable, del conector o de un camion
  // aparcado delante. Dice cuantos latidos volvieron. Un tecnico que lea "40%" como
  // "poca senal" cambiara la antena de un equipo cuyo problema es otro.
  //
  // LOS TRES TRAMOS Y EL CUARTO ESTADO, QUE NO ES UN TRAMO.
  //
  // BIEN / JUSTO / CAYENDO son tres tramos del MISMO dato medido. "SIN DATO" no es un
  // cuarto tramo peor que CAYENDO: es la ausencia del dato, y hoy se confundian. La
  // app pintaba `(parseInt(data.RF, 10) || 0) + '%'`, asi que un RF: que no fuera un
  // numero -vacio, "--", "N/A", lo que emita el firmware el dia que deje de mentir-
  // aterrizaba en pantalla como **0%**, que es el peor valor medible que existe. Un
  // campo que no se pudo leer no es un enlace a cero: nadie ha medido cero latidos.
  //
  // (El Esclavo emite hoy RF:98 y RTT:85 como LITERALES en su snprintf -no mide nada-.
  // Eso se arregla en el firmware y no aqui; lo que si es de aqui es no depender de la
  // forma exacta que tenga el arreglo: se lee lo que venga y lo que no se entienda se
  // declara, en vez de exigir un formato que todavia no esta escrito.)

  // Umbrales de los tramos, en % de latidos contestados. RF_BIEN > RF_JUSTO no es un
  // detalle de estilo: si alguien los cruzara, el tramo del medio desapareceria sin
  // que nada fallara. El pack app_09 recalcula esa desigualdad desde este fuente.
  const RF_BIEN = 70;
  const RF_JUSTO = 40;

  // Lo que se acepta como "el equipo dice explicitamente que no lo midio". Se admite
  // una lista y no un solo literal a proposito: el formato final del Esclavo todavia
  // se esta escribiendo, y cualquiera de estas formas se entiende igual. Lo que NO se
  // hace es adivinar un numero cuando llega algo que no esta aqui: eso tambien es "no
  // medido", solo que ademas se guarda el texto crudo para poder diagnosticarlo.
  const RF_NO_MEDIDO = ['', '-', '--', '?', 'NA', 'N/A', 'NULL', 'NO_MEDIDO', 'SIN_DATO'];

  // La lectura que significa "no hay nada que pintar". Es una constante y no un objeto
  // nuevo en cada llamada para que el censo del pack pueda distinguir de un vistazo
  // los dos unicos origenes legitimos del indicador: esto, o lecturaDeEnlace().
  const ENLACE_SIN_DATO = { medido: false, pct: null, rtt: null, crudo: null, crudoRtt: null };

  // Los rotulos. Los cuatro textos son DISTINTOS y ninguno de los tres tramos se
  // parece al de sin dato: el color no puede ser el unico canal -a pleno sol y con la
  // pantalla sucia el color es lo primero que se pierde- y ademas hay quien no
  // distingue el rojo del verde.
  const ENLACE_ROTULO = {
    BIEN: 'ENLACE BUENO',
    JUSTO: 'ENLACE JUSTO',
    CAYENDO: 'ENLACE CAYENDO',
    SIN_DATO: 'NO SE SABE'
  };

  function _pctDeTrama(crudo) {
    if (crudo === null || crudo === undefined) return null;
    const s = String(crudo).trim().replace(/%$/, '').toUpperCase();
    if (RF_NO_MEDIDO.indexOf(s) >= 0) return null;
    if (!/^\d{1,3}$/.test(s)) return null;
    const n = parseInt(s, 10);
    return n >= 0 && n <= 100 ? n : null;
  }

  function _enteroDeTrama(crudo) {
    if (crudo === null || crudo === undefined) return null;
    const s = String(crudo).trim().replace(/\s*ms$/i, '').toUpperCase();
    if (RF_NO_MEDIDO.indexOf(s) >= 0) return null;
    if (!/^\d{1,6}$/.test(s)) return null;
    return parseInt(s, 10);
  }

  // LA UNICA FABRICA DE LECTURAS DE ENLACE, y solo se alimenta de los campos de una
  // trama $STATUS. Cualquier otro origen -un JSON de un puente, un valor de ejemplo,
  // lo que la app acabe de pedir- no tiene por donde entrar aqui.
  function lecturaDeEnlace(data) {
    const crudoRf = data.RF === undefined ? null : String(data.RF);
    const crudoRtt = data.RTT === undefined ? null : String(data.RTT);
    const pct = _pctDeTrama(crudoRf);
    return {
      medido: pct !== null,
      pct: pct,
      rtt: _enteroDeTrama(crudoRtt),
      crudo: crudoRf,
      crudoRtt: crudoRtt
    };
  }

  function clasificarEnlace(lectura) {
    if (!lectura || lectura.medido !== true) return 'SIN_DATO';
    if (lectura.pct >= RF_BIEN) return 'BIEN';
    if (lectura.pct >= RF_JUSTO) return 'JUSTO';
    return 'CAYENDO';
  }

  // EL UNICO SITIO DE LA APP QUE ESCRIBE LOS WIDGETS DE ENLACE.
  //
  // Que sea uno solo es la propiedad, no una comodidad: mientras hubiera dos caminos,
  // uno de ellos acabaria pintando un valor que no vino de una trama -es exactamente
  // lo que pasaba con el bloque del puente de PC que se retiro hoy, que escribia
  // rf-quality desde un JSON-. Con un solo escritor, "de donde sale lo que se ve" es
  // una pregunta con una respuesta.
  //
  // marcarSinEnlace() tambien pasa por aqui, con ENLACE_SIN_DATO: declarar que no se
  // sabe es pintar el indicador, no saltarselo.
  function pintarEnlace(lectura) {
    const tramo = clasificarEnlace(lectura);
    const medido = tramo !== 'SIN_DATO';

    if (medido) {
      state.rfQuality = lectura.pct;
      state.rfRtt = lectura.rtt;
      state.rfMedidaMs = Date.now();
    }

    if (rfQualityEl) rfQualityEl.textContent = medido ? lectura.pct + '%' : '--';

    // rf-rtt lleva el numero que vino, y NADA MAS: la unidad la pone el rotulo fijo
    // del HTML. Si no vino, la casilla queda con dos guiones y la unidad se retira -un
    // "-- ms" es una medida en milisegundos que nadie hizo-. Quien declara la ausencia
    // es el rotulo del tramo y el sello de hora, no un valor de relleno.
    const hayRtt = lectura && lectura.rtt !== null && lectura.rtt !== undefined;
    if (rfRttEl) rfRttEl.textContent = hayRtt ? String(lectura.rtt) : '--';
    const rfRttUnidadEl = document.getElementById('rf-rtt-unidad');
    if (rfRttUnidadEl) rfRttUnidadEl.style.visibility = hayRtt ? 'visible' : 'hidden';

    if (rfEstadoEl) {
      rfEstadoEl.textContent = ENLACE_ROTULO[tramo];
      rfEstadoEl.className = 'enlace-estado enlace-' + tramo.toLowerCase().replace('_', '');
    }

    // La barra: sin dato se queda a CERO ANCHO Y CON EL FONDO RAYADO, que es distinto
    // de una barra corta. Una barra al 3% "por poner algo" seria un valor pintado.
    if (rfBarraEl) {
      rfBarraEl.style.width = medido ? lectura.pct + '%' : '0%';
      rfBarraEl.className = 'progress-bar enlace-barra enlace-' +
                            tramo.toLowerCase().replace('_', '');
    }

    // El sello de hora es lo que impide que un numero viejo se lea como uno de ahora.
    if (rfSelloEl) {
      if (medido) {
        rfSelloEl.textContent = 'medido a las ' + new Date().toTimeString().split(' ')[0];
      } else if (state.rfMedidaMs) {
        rfSelloEl.textContent = 'sin medida nueva; la ultima fue de las ' +
          new Date(state.rfMedidaMs).toTimeString().split(' ')[0] +
          ' (' + state.rfQuality + '%)';
      } else {
        rfSelloEl.textContent = 'ninguna medida en esta sesion';
      }
    }
    return tramo;
  }

  // =========================================================================
  // 1.quater LA BITACORA DEL ENLACE: LO QUE PASA CUANDO NADIE MIRA
  // =========================================================================
  // El indicador de arriba solo existe mientras alguien tiene el telefono en la mano.
  // En el momento de la caida el tecnico no esta delante, y cuando llega el enlace ya
  // volvio. Esto guarda la tira -ver js/registro_enlace.js, que lleva escrito por que
  // no interpola y por que un hueco no es un cero-.

  function _horaDe(ms) {
    return new Date(ms).toTimeString().split(' ')[0];
  }

  // El valor del enlace QUE ACOMPANA A UN EVENTO. Es la mitad que faltaba: una alarma
  // sin saber como iba la radio en ese momento no dice si la causa fue la radio.
  //
  // Y caduca. Si la ultima medida es mas vieja que el watchdog, ya no describe "ahora"
  // y se devuelve como no medida: pegarle a un $ALARM de las 14:36 el 70% que se midio
  // a las 14:32 seria inventar el dato mas importante de la linea.
  function enlaceDeAhora() {
    if (state.rfMedidaMs === null) return ENLACE_SIN_DATO;
    if (Date.now() - state.rfMedidaMs > TIMEOUT_ENLACE_MS) return ENLACE_SIN_DATO;
    return {
      medido: true, pct: state.rfQuality, rtt: state.rfRtt, crudo: null, crudoRtt: null
    };
  }

  // Se anota una MUESTRA cuando pasa algo que merece una linea, no una por segundo:
  //   - cambia el tramo del enlace (el instante que se busca en el registro),
  //   - o se cumple el periodo de rutina.
  // Cada anotacion sale de la lectura de UNA trama concreta. Entre dos anotaciones no
  // se rellena nada: eso es lo que dibuja los huecos como huecos.
  function registrarMuestraEnlace(lectura) {
    const tramo = clasificarEnlace(lectura);
    const ahora = Date.now();
    const cambioTramo = tramo !== state.rfTramo;
    const tocaRutina = !state.rfUltimaMuestraMs ||
      (ahora - state.rfUltimaMuestraMs) >= RegistroEnlace.PERIODO_MUESTRA_MS;
    if (!cambioTramo && !tocaRutina) return;

    const antes = state.rfTramo;
    state.rfTramo = tramo;
    state.rfUltimaMuestraMs = ahora;

    let texto;
    if (!lectura.medido) {
      texto = 'el equipo hablo pero el enlace no venia medido (RF:' +
              (lectura.crudo === null ? 'ausente' : lectura.crudo) + ')';
    } else if (cambioTramo && antes) {
      texto = 'el enlace paso de ' + ENLACE_ROTULO[antes] + ' a ' + ENLACE_ROTULO[tramo];
    } else {
      texto = ENLACE_ROTULO[tramo] + ' (latidos contestados en los ultimos 30 s)';
    }
    RegistroEnlace.anotar('MUESTRA', lectura, texto, ahora);
    renderRegistroEnlace();
  }

  function renderRegistroEnlace() {
    if (!registroTiraEl && !registroListaEl && !registroResumenEl) return;
    const estado = RegistroEnlace.cargar();
    const secuencia = RegistroEnlace.tramos(estado.registros);

    if (registroResumenEl) {
      if (!secuencia.length) {
        registroResumenEl.textContent =
          'Todavia no hay ninguna anotacion. Se llenara sola mientras la app este ' +
          'abierta con un equipo delante.';
      } else {
        const primero = estado.registros[0];
        const ultimo = estado.registros[estado.registros.length - 1];
        let t = estado.registros.length + ' anotaciones, de las ' + _horaDe(primero.ms) +
                ' a las ' + _horaDe(ultimo.ms) + ' del ' +
                new Date(ultimo.ms).toLocaleDateString() + '.';
        if (estado.descartados) {
          // Un registro recortado en silencio se lee como uno completo, y entonces
          // "no hay ninguna caida antes de las 9" se confunde con "no guarde nada".
          t += ' RECORTADO: se tiraron las ' + estado.descartados + ' anotaciones mas ' +
               'antiguas al llegar al tope de ' + RegistroEnlace.TOPE + '.';
        }
        registroResumenEl.textContent = t;
      }
      if (!RegistroEnlace.disponible) {
        registroResumenEl.textContent +=
          ' ATENCION: el registro NO se esta guardando (' +
          RegistroEnlace.motivoNoDisponible + '). Lo que se ve se pierde al cerrar.';
      }
    }

    // LA TIRA. Una celda por anotacion y una celda de HUECO por cada interrupcion, sin
    // unir nada: no hay linea, no hay pendiente, no hay valor entre dos medidas.
    if (registroTiraEl) {
      registroTiraEl.innerHTML = '';
      secuencia.slice(-80).forEach(r => {
        const c = document.createElement('span');
        if (r.clase === RegistroEnlace.CLASE_HUECO) {
          c.className = 'tira-celda tira-hueco';
          c.title = _horaDe(r.ms) + ' → ' + _horaDe(r.hastaMs) + ': ' + r.texto;
        } else if (r.rf === null) {
          c.className = 'tira-celda tira-sindato';
          c.title = _horaDe(r.ms) + ' [' + r.clase + '] ' + r.texto;
        } else {
          const tramo = clasificarEnlace({ medido: true, pct: r.rf });
          c.className = 'tira-celda tira-' + tramo.toLowerCase();
          // La ALTURA de la celda es el valor. Se pone en el estilo porque es un dato,
          // no una decoracion: 40% de alto = 40% de latidos contestados.
          c.style.height = Math.max(8, r.rf) + '%';
          c.title = _horaDe(r.ms) + ' [' + r.clase + '] ' + r.rf + '% · ' + r.texto;
        }
        registroTiraEl.appendChild(c);
      });
    }

    if (registroListaEl) {
      registroListaEl.innerHTML = '';
      secuencia.slice(-40).reverse().forEach(r => {
        const fila = document.createElement('div');
        fila.className = 'registro-fila registro-' + r.clase.toLowerCase();
        const valor = r.clase === RegistroEnlace.CLASE_HUECO ? 'HUECO'
                    : (r.rf === null ? 'sin medir' : r.rf + '%');
        fila.innerHTML =
          '<span class="registro-hora">' + _horaDe(r.ms) + '</span>' +
          '<span class="registro-clase">' + r.clase + '</span>' +
          '<span class="registro-valor">' + valor + '</span>' +
          '<span class="registro-texto"></span>';
        // El detalle va por textContent y no dentro del innerHTML de arriba: lleva
        // texto que viene del equipo ($EVENT/$ALARM) y ese no se interpreta como HTML.
        fila.querySelector('.registro-texto').textContent = r.texto;
        registroListaEl.appendChild(fila);
      });
    }
  }

  if (btnRegistroCsv) {
    btnRegistroCsv.addEventListener('click', () => {
      const estado = RegistroEnlace.cargar();
      if (!estado.registros.length) {
        showToast('El registro del enlace esta vacio: no hay nada que exportar');
        return;
      }
      const csv = RegistroEnlace.aCsv(estado.registros, estado.descartados);
      const blob = new Blob([csv], { type: 'text/csv;charset=utf-8;' });
      const link = document.createElement('a');
      link.href = URL.createObjectURL(blob);
      link.download = 'Enlace_' + state.site.replace(/[\s\·\/]+/g, '_') + '_' +
                      new Date().toISOString().slice(0, 10) + '.csv';
      link.click();
      showToast('Registro del enlace exportado (' + estado.registros.length + ' filas)');
    });
  }

  if (btnRegistroLimpiar) {
    btnRegistroLimpiar.addEventListener('click', () => {
      // Se pide confirmacion porque esto BORRA LA PRUEBA de una caida que a lo mejor
      // todavia no ha visto nadie.
      if (typeof window.confirm === 'function' &&
          !window.confirm('Se borra la bitacora del enlace guardada en este telefono. ' +
                          'Si no la has exportado, se pierde.')) return;
      RegistroEnlace.limpiar();
      state.rfTramo = null;
      state.rfUltimaMuestraMs = null;
      renderRegistroEnlace();
      addEvent('cyan', 'Bitacora del enlace borrada a peticion del usuario.');
    });
  }

  // =========================================================================
  // 1.quinquies MODO DEPURACION: LAS TRAMAS EN CRUDO Y LAS QUE NO ENTRARON
  // =========================================================================
  // "Hoy el problema es no saber cuanto se va cuando se va, y POR QUE se va."
  //
  // La bitacora de arriba contesta al "cuanto": guarda la tira y los huecos. Este
  // bloque contesta al "por que", y para eso hace falta el CONTENIDO, no solo el hueco:
  // hasta hoy la app parseaba cada linea, se quedaba con los campos que entendia y
  // TIRABA LA LINEA. Diez milisegundos despues de llegar, la trama que no cuadro no
  // existia en ninguna parte.
  //
  // POR QUE ESTO ES UNA PESTANA APARTE Y NO UN PANEL EN LA DE TRAFICO.
  //
  // Porque un panel que escribe en LOS MISMOS widgets que el dato real es la version de
  // interfaz de la prueba que no mide nada, y esta app ya lo pago: traia un "SIMULADOR
  // DE PRUEBAS - DEMO EN VIVO" que pintaba fases inventadas sobre los mismos semaforos
  // que la telemetria. Aqui no se pinta ni un semaforo, ni el contador, ni la barra de
  // enlace: se ensena texto que entro por el cable. Y cuando no ha entrado nada, se
  // DICE. Nunca hay una trama de ejemplo en esta vista.
  //
  // Y ESTE BLOQUE CAMBIA COMPORTAMIENTO, QUE ES LO QUE HAY QUE MIRAR AL REVISARLO.
  //
  // Para poder decir "rechazada por checksum" hay que validar el checksum, y hasta hoy
  // NADIE lo validaba: parseNmeaTelemetry() hacia `line.split('*')[0]` y tiraba el CRC
  // sin leerlo, mientras NMEAParser.validarTrama() llevaba meses escrita y sin un solo
  // llamador -fichada como huerfana en el pack app_07, que es la unica razon de que se
  // supiera-. Conectarla NO es solo instrumentacion: desde hoy una trama con el
  // checksum malo NO se pinta. Antes se pintaba, con los bytes que hubiera traido la
  // radio dentro de ESTADO:, de MODO: o de T:.

  // Las cabeceras que esta app sabe leer. La lista se escribe UNA vez y la usan el juez
  // y la pantalla: un tipo que no este aqui no se pinta, y se cuenta como rechazado
  // NOMBRANDOLO, que es distinto de ignorarlo en silencio como se hacia antes -un
  // `else if` sin `else` final se traga lo que no reconoce y no deja rastro-.
  const TIPOS_QUE_LA_APP_LEE = ['$STATUS', '$ALARM', '$ACK', '$EVENT', '$ERR'];

  // EL JUEZ. Una sola funcion decide si una linea entra, y devuelve POR QUE no cuando
  // no entra. No pinta, no anota y no toca el estado: eso lo hacen sus llamadores.
  function juzgarTrama(linea) {
    const v = NMEAParser.validarTrama(linea);
    if (!v.valida) {
      return {
        aceptada: false,
        motivo: v.motivo,
        detalle: v.error,
        tipo: '',
        partes: null
      };
    }
    // El payload vuelve SIN el '$' -validarTrama lo salta para el XOR, igual que
    // enviarTramaConCrc() en el firmware-. Se le devuelve para que el resto del camino
    // siga comparando cabeceras con dolar, que es como estan escritas las ramas.
    const partes = ('$' + v.payload).split(',');
    const tipo = partes[0];
    if (TIPOS_QUE_LA_APP_LEE.indexOf(tipo) < 0) {
      return {
        aceptada: false,
        motivo: 'TIPO_DESCONOCIDO',
        detalle: 'la cabecera ' + tipo + ' no la lee ninguna rama de esta app',
        tipo: tipo,
        partes: null
      };
    }
    return { aceptada: true, motivo: null, detalle: '', tipo: tipo, partes: partes };
  }

  // LA ANOTACION DE RECHAZO EN LA BITACORA, ESTRANGULADA.
  //
  // Un rechazo va a la linea de tiempo persistida porque "a las 03:41 empezo a llegar
  // basura" es exactamente lo que se busca al dia siguiente, y distingue dos averias
  // que desde el suelo se ven igual: "no llegaba nada" (hueco) contra "llegaba basura"
  // (esto). Lo que NO va es una anotacion por trama: con la radio en mal estado pueden
  // ser cientos por minuto, y el tope de 400 de la bitacora se vaciaria de historia del
  // enlace para llenarse de la misma linea repetida.
  //
  // Se anota cuando CAMBIA el motivo -que es el instante que interesa- o cuando se
  // cumple el periodo de rutina, y la anotacion lleva la CUENTA acumulada desde que
  // empezo la racha. La trama en crudo va dentro del texto, recortada: la cinta de
  // js/depuracion.js se pierde al cerrar la app y esto no, asi que la muestra tiene que
  // viajar con la anotacion o el ejemplo se pierde.
  const RECHAZO_MUESTRA_MAX = 120;

  function registrarRechazoEnlace(veredicto, linea) {
    const ahora = Date.now();
    const motivo = veredicto.motivo || 'SIN_FORMA';
    // Una racha se da por terminada cuando pasa un periodo entero SIN NINGUN rechazo, y
    // no con la primera trama buena que llegue. La diferencia importa en el caso que se
    // quiere medir: con la radio marginal se alternan buenas y malas, y cortar la racha
    // en cada buena anotaria una linea por cada mala -que es justo lo que el
    // estrangulador existe para impedir-. Un rechazo suelto una hora despues, en
    // cambio, es un suceso nuevo y merece su propia anotacion.
    const rachaVieja = state.rechazoUltimaMs !== null &&
      (ahora - state.rechazoUltimaMs) > RegistroEnlace.PERIODO_MUESTRA_MS;
    const cambioMotivo = motivo !== state.rechazoMotivo || rachaVieja;
    state.rechazoUltimaMs = ahora;
    if (cambioMotivo) {
      state.rechazoMotivo = motivo;
      state.rechazoDesdeMs = ahora;
      state.rechazoCuenta = 0;
      state.rechazoAnotadoMs = null;
    }
    state.rechazoCuenta++;
    const tocaRutina = !state.rechazoAnotadoMs ||
      (ahora - state.rechazoAnotadoMs) >= RegistroEnlace.PERIODO_MUESTRA_MS;
    if (!cambioMotivo && !tocaRutina) return;
    state.rechazoAnotadoMs = ahora;

    const cruda = RegistroCrudo.escapar(String(linea === undefined ? '' : linea));
    const muestra = cruda.length > RECHAZO_MUESTRA_MAX
      ? cruda.slice(0, RECHAZO_MUESTRA_MAX) + '...' : cruda;
    const texto = 'Entro algo por el cable y la app NO lo pinto: ' +
      (RegistroCrudo.MOTIVOS[motivo] || motivo) + '. ' +
      state.rechazoCuenta + ' desde las ' + _horaDe(state.rechazoDesdeMs) + '. ' +
      (veredicto.detalle ? veredicto.detalle + '. ' : '') +
      'Muestra: ' + muestra;
    RegistroEnlace.anotar('RECHAZO', enlaceDeAhora(), texto);
    renderRegistroEnlace();
  }

  // LOS REPAROS DE UNA TRAMA QUE SI ENTRO. No es lo mismo "no se pinto nada" que "se
  // pinto con huecos", y meter las dos cosas en el saco de rechazadas diria que la
  // segunda no llego. Los reparos se calculan DESPUES de parsear, con las mismas
  // funciones que decidieron lo que se pinta -no con una segunda copia de la regla-,
  // que es la unica forma de que la cinta cuente lo que de verdad paso.
  function reparosDeStatus(data, lectura) {
    const r = [];
    if (Object.keys(data).length === 0) {
      r.push('no traia ningun campo con forma clave:valor. El checksum es bueno, asi ' +
             'que el equipo hablo y llego entero: sirve de latido y no de medida');
    }
    if (!lectura.medido) {
      r.push('el enlace no venia medido (RF:' +
             (lectura.crudo === null ? 'ausente' : lectura.crudo) + ')');
    }
    if (lectura.rtt === null) {
      r.push('el RTT no venia medido (RTT:' +
             (lectura.crudoRtt === null ? 'ausente' : lectura.crudoRtt) + ')');
    }
    if (data.BAT !== undefined && state.battery === null) {
      r.push('la bateria no venia medida (BAT:' + data.BAT + ')');
    }
    return r;
  }

  // ---- La pantalla de depuracion ----------------------------------------

  function _textoContadores(c) {
    const min = Math.round(c.ventanaMs / 60000);
    if (!c.total) {
      return 'En los ultimos ' + min + ' minutos no ha entrado ninguna trama.';
    }
    let t = 'Ultimos ' + min + ' min: ' + c.total + ' tramas · ' +
            c.aceptadas + ' aceptadas · ' + c.rechazadas + ' rechazadas';
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
      let n = 'Esta cinta vive en la MEMORIA de la app y cabe ' +
              RegistroCrudo.TOPE + ' tramas -unos ' +
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
      marca.textContent = r.veredicto === RegistroCrudo.RECHAZADA
        ? 'RECHAZADA · ' + r.motivo : 'ACEPTADA' + (r.tipo ? ' · ' + r.tipo : '');
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
  function textoDepuracion() {
    return RegistroCrudo.aTexto(Date.now(), {
      Cruce: state.site,
      Equipo: state.node === null ? 'sin identificar (ningun $STATUS con NODE)' : state.node,
      Serie: state.serie === null ? 'sin identificar' : state.serie,
      Enlace: state.rfQuality === null
        ? 'no medido en esta sesion'
        : state.rfQuality + '% a las ' + _horaDe(state.rfMedidaMs)
    });
  }

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
    });
  }

  if (btnDepuLimpiar) {
    btnDepuLimpiar.addEventListener('click', () => {
      if (typeof window.confirm === 'function' &&
          !window.confirm('Se vacia la cinta de tramas de esta sesion. Si no la ha ' +
                          'exportado, se pierde.')) return;
      RegistroCrudo.limpiar();
      if (depuTextoEl) {
        depuTextoEl.value = '';
        depuTextoEl.hidden = true;
      }
      renderDepuracion();
      addEvent('cyan', 'Cinta de tramas en crudo vaciada a peticion del usuario.');
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

  // =========================================================================
  // 2. RENDER DE SEMAFOROS: SE PINTA LA PUNTA QUE HABLA, Y SE DECLARA LA OTRA
  // =========================================================================
  // $STATUS TRAE EL ESTADO DE UN SOLO POSTE, Y ESTA PANTALLA TIENE DOS.
  //
  // El vocabulario que este bloque leia -V1_R2, Y1_R2, R1_R2, ALL_RED, R1_V2, R1_Y2,
  // AMBAR_FAIL- describe LOS DOS semaforos a la vez, y NO LO EMITE NINGUN FIRMWARE:
  // sale del puente de PC (servidor_puente_simulador.py:28), que es un simulador.
  // Lo que manda el equipo es el enum de cuatro valores de semaforo_nombreEstado()
  // -Maestro/src/semaforo.cpp:336-344 y Esclavo/src/semaforo.cpp:331-339, identicos-:
  //
  //     "ROJO"   "VERDE"   "AMARILLO"   "FALLO COM"
  //
  // MEDIDO: la interseccion de las dos listas es VACIA. Con el enlace vivo no casaba
  // ni un case, las seis lamparas se quedaban apagadas por el forEach de arriba y
  // nadie volvia a encenderlas, y s1Text/s2Text/phase-desc conservaban lo ultimo que
  // hubiera -que tras marcarSinEnlace() es "SIN ENLACE - sin datos del equipo"-. O
  // sea: el tablero declaraba que no tenia datos MIENTRAS los estaba recibiendo.
  //
  // POR QUE ESTO NO SE ARREGLA RENOMBRANDO LOS `case`.
  //
  // No hay a que renombrarlos. `V1_R2` afirma algo de DOS postes y la trama habla de
  // UNO: el que dice NODE:. La app NO TIENE el estado del otro extremo -no llega por
  // Bluetooth, y ninguna punta lo pone en $STATUS-. Deducirlo -"si este da verde, el
  // otro estara en rojo"- seria inventar justo la cifra que decide si se cruza, y
  // seria FALSO precisamente cuando mas se mira el telefono: en Modo Degradado cada
  // punta cicla por su cuenta, un ambar de emergencia es local, y con la radio caida
  // las dos puntas se van a S_FALLO por separado.
  //
  // Asi que se pinta el poste que habla, con su valor literal, y del otro se dice que
  // no se sabe. Un tablero quieto que admite que no sabe es honesto; uno que pinta un
  // cruce que no ha medido le miente a quien decide sobre el trafico mirandolo
  // (CLAUDE.md 3.quinquies).
  //
  // Y el otro sentido, que se cobra al reves: "FALLO COM" NO ES "APAGADO" NI ES ROJO.
  // Es ambar intermitente CON LA TALANQUERA ARRIBA -SFTY-6, la politica que eligio el
  // cliente el 27/08-: por ahi SE PASA con precaucion. El anillo lo pintaba de rojo
  // (ver mas abajo), que dice lo contrario de lo que hace el equipo.
  const ESTADOS = {
    'ROJO':      { lampara: 'red',   texto: 'ROJO (ESPERA)',  color: 'var(--red-text)',   anillo: 'red',
                   frase: 'ROJO, este poste no da paso' },
    'VERDE':     { lampara: 'green', texto: 'VERDE (PASO)',   color: 'var(--green-lamp)', anillo: 'green',
                   frase: 'VERDE, este poste da paso' },
    'AMARILLO':  { lampara: 'amber', texto: 'AMARILLO',       color: 'var(--amber-lamp)', anillo: 'amber',
                   frase: 'AMARILLO, este poste está cerrando su paso' },
    'FALLO COM': { lampara: 'amber', texto: 'ÁMBAR DESTELLO', color: 'var(--amber-lamp)', anillo: 'amber',
                   frase: 'FALLO COM: ámbar intermitente y TALANQUERA ARRIBA, se pasa con precaución' }
  };

  // MODO: los diez literales que las dos puntas pueden emitir. Nueve son los `case`
  // de obtenerNombreModo() (Maestro/src/bluetooth.cpp:367-379) y el decimo es el
  // literal fijo que el Esclavo escribe dentro de su propio snprintf,
  // MODO:SUBORDINADO (Esclavo/src/bluetooth.cpp:328).
  //
  // La cadena de `if` anterior tenia CUATRO ramas -AUTO, MANUAL, AMBAR y un
  // ROJO_TOTAL que no emite nadie- y ningun `else`: con MENU, INTELIGENTE, ALCANCE,
  // HORA, DEGRADADO, DESCONOCIDO o SUBORDINADO el badge se quedaba con el modo
  // ANTERIOR pintado y con su color, o sea un modo vencido con aspecto de vigente. El
  // que mas duele de esa lista es DEGRADADO: es el unico modo que da verde SIN
  // confirmacion del otro extremo, y era invisible en la pantalla.
  const MODOS = {
    'AUTO':        { texto: '🟢 AUTOMÁTICO',                fondo: 'rgba(0,230,118,0.15)',   borde: 'var(--green-lamp)', color: 'var(--green-lamp)' },
    'MANUAL':      { texto: '✋ MODO MANUAL',                fondo: 'rgba(0,240,255,0.15)',   borde: 'var(--cyan-neon)',  color: 'var(--cyan-neon)' },
    'AMBAR':       { texto: '🟡 ÁMBAR PRECAUCIÓN',          fondo: 'rgba(255,179,0,0.15)',   borde: 'var(--amber-lamp)', color: 'var(--amber-lamp)' },
    'MENU':        { texto: '☰ EN MENÚ · SIN CICLO',        fondo: 'rgba(0,240,255,0.15)',   borde: 'var(--cyan-neon)',  color: 'var(--cyan-neon)' },
    'INTELIGENTE': { texto: '👁 INTELIGENTE · POR DEMANDA',  fondo: 'rgba(0,240,255,0.15)',   borde: 'var(--cyan-neon)',  color: 'var(--cyan-neon)' },
    'ALCANCE':     { texto: '🛑 ALCANCE · ROJO FIJO',       fondo: 'rgba(255,30,68,0.2)',    borde: 'var(--red-lamp)',   color: 'var(--red-lamp)' },
    'HORA':        { texto: '🕐 AJUSTANDO HORA',            fondo: 'rgba(0,240,255,0.15)',   borde: 'var(--cyan-neon)',  color: 'var(--cyan-neon)' },
    'DEGRADADO':   { texto: '⚠️ DEGRADADO · SIN ENLACE ENTRE POSTES', fondo: 'rgba(255,179,0,0.15)', borde: 'var(--amber-lamp)', color: 'var(--amber-lamp)' },
    'SUBORDINADO': { texto: '🔗 SUBORDINADO AL MAESTRO',    fondo: 'rgba(0,240,255,0.15)',   borde: 'var(--cyan-neon)',  color: 'var(--cyan-neon)' },
    // Este no es "la app no lo conoce": es el equipo diciendo que no sabe en que modo
    // esta -el `default` de su propio switch-. Se distingue del de abajo a proposito.
    'DESCONOCIDO': { texto: '❔ EL EQUIPO NO SABE SU MODO',  fondo: 'rgba(148,163,184,0.15)', borde: 'var(--text-muted)', color: 'var(--text-muted)' }
  };

  function pintarBadgeModo() {
    if (!badgeModoEl) return;
    const info = MODOS[state.modo];
    badgeModoEl.className = 'badge badge-auto';
    if (info) {
      badgeModoEl.style.background = info.fondo;
      badgeModoEl.style.borderColor = info.borde;
      badgeModoEl.style.color = info.color;
      badgeModoEl.textContent = info.texto;
    } else {
      // Un MODO que esta tabla no conoce se DICE, con el literal a la vista. Callarlo
      // dejaria el badge anterior en pantalla; ensenarlo hace visible el dia que el
      // firmware estrene un modo y a esta lista se le olvide crecer.
      badgeModoEl.style.background = 'rgba(148,163,184,0.15)';
      badgeModoEl.style.borderColor = 'var(--text-muted)';
      badgeModoEl.style.color = 'var(--text-muted)';
      badgeModoEl.textContent = state.modo
        ? '❔ MODO NO RECONOCIDO: ' + state.modo
        : 'SIN DATOS DE MODO';
    }
  }

  function renderLights() {
    if (!s1Red || !s1Amber || !s1Green || !s2Red || !s2Amber || !s2Green) return;

    [s1Red, s1Amber, s1Green, s2Red, s2Amber, s2Green].forEach(l => l.classList.remove('active'));

    // Cual de las dos columnas es la que habla. El reparto no se inventa aqui: es el
    // mismo que ya rotula el HTML -POSTE 1 = MAESTRO, POSTE 2 = ESCLAVO- y el mismo
    // que usa la cabecera al leer NODE:.
    const esEsclavo = state.node === 'ESCLAVO';
    const propias = esEsclavo ? [s2Red, s2Amber, s2Green] : [s1Red, s1Amber, s1Green];
    const textoPropio = esEsclavo ? s2Text : s1Text;
    const textoAjeno = esEsclavo ? s1Text : s2Text;
    const rotuloAjeno = esEsclavo ? 'POSTE 1 · MAESTRO' : 'POSTE 2 · ESCLAVO';

    // EL OTRO EXTREMO NO SE PINTA NUNCA. Sus tres lamparas se quedan apagadas por el
    // forEach de arriba, y el texto de debajo dice por que: tres lamparas apagadas a
    // secas se leen como "ese poste esta apagado", que seria otra afirmacion sin
    // medida. Lo que consta es que ESTA trama no habla de el.
    function declararAjeno() {
      if (!textoAjeno) return;
      textoAjeno.textContent = 'SIN DATOS · no viaja en esta trama';
      textoAjeno.style.color = 'var(--text-muted)';
    }

    pintarBadgeModo();

    if (!state.node) {
      // Ni siquiera se sabe QUE punta hay al otro lado, asi que no hay columna que
      // pintar. Ocurre entre la conexion y el primer $STATUS.
      [s1Text, s2Text].forEach(t => {
        if (!t) return;
        t.textContent = 'SIN DATOS';
        t.style.color = 'var(--text-muted)';
      });
      if (phaseDescEl) phaseDescEl.textContent = 'EQUIPO SIN IDENTIFICAR - esperando NODE en $STATUS';
      return;
    }

    const info = ESTADOS[state.estadoLuces];
    if (!info) {
      // Llego un ESTADO que no esta en el enum del firmware. No se adivina: se ensena
      // el literal. Es lo que hace visible el desajuste en vez de dejar la pantalla
      // congelada, que es justo como este defecto sobrevivio.
      declararAjeno();
      if (textoPropio) {
        textoPropio.textContent = 'ESTADO NO RECONOCIDO';
        textoPropio.style.color = 'var(--text-muted)';
      }
      if (phaseDescEl) {
        phaseDescEl.textContent = state.estadoLuces
          ? 'ESTADO no reconocido: "' + state.estadoLuces + '"'
          : 'SIN ESTADO en la última trama';
      }
      return;
    }

    propias[{ red: 0, amber: 1, green: 2 }[info.lampara]].classList.add('active');
    if (textoPropio) {
      textoPropio.textContent = info.texto;
      textoPropio.style.color = info.color;
    }
    declararAjeno();

    // La linea del centro deja de anunciar una FASE DEL CRUCE -"FASE: SENTIDO 1"-,
    // que es una afirmacion sobre los dos postes, y pasa a decir DE QUIEN es el dato
    // que se esta viendo y que dice. Es la misma diferencia de arriba, escrita donde
    // el operario la lee.
    if (phaseDescEl) {
      phaseDescEl.textContent = (esEsclavo ? 'ESCLAVO' : 'MAESTRO') + ': ' + info.frase +
                                ' · ' + rotuloAjeno + ' no informa';
    }
  }

  function updateCountdownRing() {
    if (!cdNumEl || !ringProgressEl) return;
    const current = Math.max(0, state.countdown);
    cdNumEl.textContent = current;

    const circumference = 251.32;
    // LA FRACCION SOLO SE DIBUJA SI HAY UN TOTAL QUE VENGA DE FUERA DE ESTA APP.
    //
    // countdownMax no llega en $STATUS -la trama trae T: y nada mas-, asi que el
    // unico que lo rellenaba era el formulario de tiempos de la propia app: el anillo
    // dibujaba una fraccion contra un total que se habia inventado el telefono. Sin
    // total conocido se deja el arco vacio, que es lo que se sabe; el numero de
    // segundos si viene del equipo y se sigue mostrando.
    const total = state.countdownMax > 0 ? state.countdownMax : 0;
    ringProgressEl.style.strokeDashoffset = total > 0
      ? circumference - (Math.min(1, current / total) * circumference)
      : circumference;

    // EL COLOR SE DECIDE POR EL VALOR, NO POR LAS LETRAS QUE LLEVA DENTRO.
    //
    // Esto era `.includes('V')` / `.includes('Y')` sobre el vocabulario del
    // simulador. MEDIDO contra el enum real: "AMARILLO" NO contiene 'Y' y "FALLO COM"
    // tampoco, asi que los dos caian al else y salian en ROJO -el equipo en ambar, y
    // en el caso de FALLO COM con la talanquera ARRIBA, mientras el anillo decia
    // ESPERA-.
    const info = ESTADOS[state.estadoLuces];
    const clase = info ? info.anillo : 'red';
    ringProgressEl.className = 'ring-fill ' + clase;
    cdNumEl.className = 'ring-num ' + clase;
  }

  // =========================================================================
  // 3. ROLE MANAGEMENT (OPERARIO VS TÉCNICO)
  // =========================================================================
  function setRole(newRole) {
    state.role = newRole;
    if (newRole === 'TECNICO') {
      if (btnToggleRole) btnToggleRole.classList.add('admin');
      if (roleIconEl) roleIconEl.textContent = '🛡️';
      if (roleLabelEl) roleLabelEl.textContent = 'Técnico';
      adminTabs.forEach(t => t.style.display = 'flex');
      showToast('🛡️ Modo Técnico Desbloqueado');
      addEvent('cyan', 'Sesión: Perfil Técnico / Administrador activado.');
    } else {
      if (btnToggleRole) btnToggleRole.classList.remove('admin');
      if (roleIconEl) roleIconEl.textContent = '👷';
      if (roleLabelEl) roleLabelEl.textContent = 'Operario';
      adminTabs.forEach(t => t.style.display = 'none');
      
      const activeTab = document.querySelector('.tab-content.active');
      if (activeTab && activeTab.classList.contains('admin-only')) {
        document.querySelectorAll('.tab-content').forEach(tc => tc.classList.remove('active'));
        document.querySelectorAll('.nav-item').forEach(ni => ni.classList.remove('active'));
        document.getElementById('tab-estado').classList.add('active');
        document.querySelector('.nav-item[data-tab="tab-estado"]').classList.add('active');
      }
      showToast('👷 Modo Operario Activo');
    }
  }

  if (btnToggleRole) {
    btnToggleRole.addEventListener('click', () => {
      if (state.role === 'OPERARIO') {
        openPinModal();
      } else {
        setRole('OPERARIO');
      }
    });
  }

  // Pide el PIN y REPITE la pulsacion cuando entra. Repetir el click en vez de
  // ejecutar media accion evita que la pantalla se pinte con un estado que el
  // firmware no llego a recibir.
  function pedirPin(reintento) {
    state.accionPendiente = reintento;
    openPinModal();
  }

  // =========================================================================
  // 4. BOTONERA DE CAMPO DEL OPERARIO (ACCIONES A 1 TOQUE)
  // =========================================================================
  // PULSAR UN BOTON NO ES SABER QUE EL EQUIPO OBEDECIO.
  //
  // Los cuatro botones de esta botonera pintaban el resultado que ESPERABAN: al pedir
  // AUTO se ponian V1_R2 y arrancaban la cuenta; al pedir cambio de turno se animaba
  // una secuencia completa -ambar, despeje, verde del otro lado- con setTimeout, con
  // tiempos sacados del formulario de la app y no del equipo. Eso es un panel de demo
  // escribiendo en los MISMOS widgets que la telemetria (CLAUDE.md 3.quinquies): con
  // el Bluetooth caido, o con la orden rechazada por $ERR, la pantalla seguia pintando
  // un cruce que se movia. Quien esta de pie en la calzada mirando el telefono no tiene
  // como distinguirlo del cruce de verdad.
  //
  // Ahora el boton hace una sola cosa: mandar la orden y decir que la mando. Las luces,
  // el modo y el contador los pinta $STATUS -que llega cada segundo desde el equipo- y
  // el acuse lo pinta $ACK. Si no llega ninguno de los dos, no se pinta nada, que es
  // exactamente lo que se sabe.
  // Las tres ordenes van con su literal a la vista y NO por un ayudante con el comando
  // en un parametro. El pack app_01_comandos censa la interfaz buscando justamente
  // la llamada con el comando escrito entero en el fuente: pasarlas por una tabla las hace
  // invisibles para el censo, y un comando que el censo no ve es un comando que puede
  // desaparecer de la app sin que nadie se entere (CLAUDE.md 5).
  // LOS TRES MANDOS DE LA BOTONERA NO CONSULTABAN A QUE PUNTA IBAN.
  //
  // MEDIDO: SET_MODO y MANUAL:CAMBIAR_TURNO estan en SOLO_MAESTRO -el despachador del
  // Esclavo no tiene esas ramas, Esclavo/src/bluetooth.cpp-. Contra un Esclavo estos
  // tres botones salian al cable y volvian como $ERR,CMD:DESCONOCIDO: el operario ve
  // un boton que "no hace nada" y un error que no dice de que habla. El resto de las
  // ordenes de la app SI preguntaban -el despachador de data-cmd, MENU, ALCANCE,
  // DEGRADADO y los dos mandos de emergencia-, asi que estos tres eran el hueco, no la
  // norma.
  //
  // La guarda va ANTES del PIN, igual que en el despachador de data-cmd: pedir una
  // clave para una orden que no se va a mandar es hacer teclear al operario delante de
  // un cruce parado para nada.
  if (btnOpAuto) {
    btnOpAuto.addEventListener('click', () => {
      const punta = puntaCorrecta('SET_MODO');
      if (punta) { avisarOtraPunta('SET_MODO:AUTO', punta); return; }
      if (!state.pinVerificado) { pedirPin(() => btnOpAuto.click()); return; }
      enviarComandoFirmware('SET_MODO', 'AUTO');
      addEvent('cyan', 'Operario: orden MODO AUTOMATICO enviada al equipo.');
    });
  }

  if (btnOpStep) {
    btnOpStep.addEventListener('click', () => {
      const punta = puntaCorrecta('MANUAL:CAMBIAR_TURNO');
      if (punta) { avisarOtraPunta('MANUAL:CAMBIAR_TURNO', punta); return; }
      if (!state.pinVerificado) { pedirPin(() => btnOpStep.click()); return; }
      enviarComandoFirmware('MANUAL:CAMBIAR_TURNO');
      addEvent('cyan', 'Operario: orden CAMBIAR TURNO enviada al equipo.');
    });
  }

  if (btnOpAmber) {
    btnOpAmber.addEventListener('click', () => {
      const punta = puntaCorrecta('SET_MODO');
      if (punta) { avisarOtraPunta('SET_MODO:AMBAR', punta); return; }
      if (!state.pinVerificado) { pedirPin(() => btnOpAmber.click()); return; }
      enviarComandoFirmware('SET_MODO', 'AMBAR');
      addEvent('cyan', 'Operario: orden MODO AMBAR enviada al equipo.');
    });
  }

  // =========================================================================
  // 4.bis LA PARADA DE EMERGENCIA NO ES LA MISMA MANIOBRA EN LAS DOS PUNTAS
  // =========================================================================
  // Contra el MAESTRO el equipo se pone en ROJO FIJO en las dos vias: para el trafico.
  // Contra el ESCLAVO no para nada: pone AMBAR INTERMITENTE y ABRE la talanquera, que
  // por decision del cliente del 27/08 significa pasar con precaucion en los dos
  // sentidos. Son maniobras opuestas y hasta el 28/08 se pedian con el mismo literal
  // -FORZAR_ROJO- y se acusaban con el mismo texto -"rojo forzado, correcto"- aunque
  // en esa punta no se encendia un solo rojo. El firmware ya no lo esconde: el Esclavo
  // atiende AMBAR_EMERGENCIA y RECHAZA el nombre viejo.
  //
  // La app no puede seguir escondiendolo con un boton unico. Un rotulo que dice lo
  // mismo para dos maniobras contrarias es la misma clase de mentira que se acaba de
  // quitar del C++, y aqui la lee alguien de pie en la calzada decidiendo sobre el
  // trafico. Asi que son DOS mandos, cada uno con su maniobra escrita, y en pantalla
  // solo esta el de la punta que el equipo declara.
  //
  // Cada orden va con su literal a la vista y NO por una tabla: el pack
  // app_01_comandos censa la interfaz buscando la llamada con el comando escrito
  // entero, y un comando que el censo no ve puede desaparecer sin que nadie se entere
  // (CLAUDE.md 5).
  //
  // NINGUNO DE LOS DOS PINTA NADA: mandan y esperan. Lo que el equipo hizo lo dicen
  // $ACK y $STATUS; si el literal va a la punta que no es, llega un $ERR con su motivo
  // y lo ensena el camino de rechazos de siempre.
  if (btnOpEmergency) {
    btnOpEmergency.addEventListener('click', () => {
      // La guarda mira state.node en vez de llamar a puntaCorrecta() tal cual: aquella
      // resuelve por descarte -"si no es ESCLAVO, es MAESTRO"- y con la punta todavia
      // sin identificar retiraria una de las dos emergencias por no saber. Aqui solo se
      // rechaza cuando consta que al otro lado hay la punta contraria.
      if (state.node === 'ESCLAVO') { avisarOtraPunta('FORZAR_ROJO', 'MAESTRO'); return; }
      // Forma SIN PIN: es la que el firmware espera para la parada de emergencia, y
      // la rama que la construye llevaba desde el rewrite sin un solo llamador.
      enviarComandoFirmware('FORZAR_ROJO');
      addEvent('red', 'ALERTA: orden ROJO TOTAL DE EMERGENCIA enviada al MAESTRO. ' +
                      'Si el equipo la acepta deja las dos vias en rojo fijo.');
    });
  }

  if (btnOpAmbarEmergencia) {
    btnOpAmbarEmergencia.addEventListener('click', () => {
      if (state.node === 'MAESTRO') { avisarOtraPunta('AMBAR_EMERGENCIA', 'ESCLAVO'); return; }
      enviarComandoFirmware('AMBAR_EMERGENCIA');
      addEvent('red', 'ALERTA: orden AMBAR DE EMERGENCIA enviada al ESCLAVO. Si el ' +
                      'equipo la acepta queda en ambar intermitente y la talanquera ' +
                      'ABIERTA: no es un rojo, los dos sentidos pasan con precaucion.');
    });
  }

  // R-3: RETIRAR EL AMBAR DE EMERGENCIA DEL ESCLAVO.
  //
  // Es la unica orden de la botonera de campo que PIDE PIN, y la asimetria esta
  // razonada en el firmware (Esclavo/src/bluetooth.cpp:331): poner el ambar es la
  // accion segura y por eso viaja sin clave; QUITARLO devuelve el cruce a dar verdes
  // -abre paso-, que es exactamente lo que el PIN custodia. Por eso NO esta en SIN_PIN.
  //
  // Y NO SE PINTA NADA AQUI. Esta orden tiene TRES respuestas distintas y solo el
  // equipo sabe cual toca -si queda el latch del mando, el ambar sigue puesto aunque
  // la orden se haya aceptado-. Escribir "ambar retirado" al pulsar seria la mentira
  // exacta que se quiere evitar: el operario se iria creyendo que el cruce vuelve a
  // ciclar. Lo que sale de aqui es "orden enviada"; lo que pasa lo dicen $ACK y $ERR.
  if (btnOpCancelarAmbar) {
    btnOpCancelarAmbar.addEventListener('click', () => {
      const punta = puntaCorrecta('CANCELAR_AMBAR');
      if (punta) { avisarOtraPunta('CANCELAR_AMBAR', punta); return; }
      if (!state.pinVerificado) { pedirPin(() => btnOpCancelarAmbar.click()); return; }
      enviarComandoFirmware('CANCELAR_AMBAR');
      addEvent('cyan', 'Tecnico: orden RETIRAR AMBAR enviada al ESCLAVO. Espere el ' +
                       'acuse: el equipo dira si quedaba algun ambar y si queda otro ' +
                       'puesto desde el mando.');
    });
  }

  // El rotulo dice QUE maniobra; esto dice CONTRA QUE PUNTA, que es la mitad sin la
  // cual el rotulo no significa nada. Se gobierna con state.node -que sale del $STATUS
  // del equipo, o del poste que el tecnico eligio en el modal de enlace-, nunca de lo
  // que la app acabe de pedir.
  //
  // Y NO cuelga de state.telemetriaViva a proposito. La identidad de la punta no
  // envejece como el contador o la bateria: mientras el socket siga abierto, al otro
  // lado sigue estando el mismo poste. Colgarla del watchdog dejaria la emergencia
  // escondida justo cuando la telemetria se ha caido, que es cuando mas falta hace
  // poder pararlo -es la razon por la que el TX de emergencia tampoco cuelga de el-.
  function actualizarEmergencia() {
    const punta = (state.node === 'MAESTRO' || state.node === 'ESCLAVO') ? state.node : null;

    // Cuelga de aqui y no de sus propias llamadas para que no puedan desincronizarse:
    // esta funcion ya se invoca en los tres momentos en que la punta puede cambiar
    // -al llegar un $STATUS, al conectar y al arrancar-, y una segunda lista de sitios
    // donde llamar es una lista que alguien tiene que acordarse de actualizar.
    actualizarMandosDePunta(punta);

    // Sin punta identificada se ensenan LAS DOS, cada una con su poste delante.
    // Esconder una obligaria a la app a elegir un poste que no conoce, y esconder las
    // dos retiraria la caida segura por no saber. Ensenar las dos nombradas es lo unico
    // que ni inventa ni quita.
    if (btnOpEmergency) {
      btnOpEmergency.style.display = (punta === 'ESCLAVO') ? 'none' : 'flex';
    }
    if (btnOpAmbarEmergencia) {
      btnOpAmbarEmergencia.style.display = (punta === 'MAESTRO') ? 'none' : 'flex';
    }
    // RETIRAR AMBAR solo sale con un ESCLAVO CONFIRMADO delante, y aqui el criterio es
    // el contrario al de los dos de arriba a proposito. Aquellos son la caida segura y
    // se ensenan aunque no se sepa la punta -retirar una parada de emergencia por no
    // saber es peor que ensenar dos-. Esta ABRE PASO: sin punta identificada no se
    // ofrece, porque una orden que abre paso no se ensena "por si acaso".
    if (btnOpCancelarAmbar) {
      btnOpCancelarAmbar.style.display = (punta === 'ESCLAVO') ? 'flex' : 'none';
    }
    // MAESTRO y ESCLAVO son el vocabulario del protocolo, no el de quien esta de pie
    // en la calzada: un banderillero no tiene por que saber cual de los dos manda.
    // POSTE 1 y POSTE 2 son los rotulos que esta misma pantalla lleva escritos encima
    // de cada semaforo, asi que el mando y la luz se nombran igual. Los dos nombres
    // tecnicos siguen enteros donde hacen falta -la cabecera, la bitacora de Eventos y
    // las pestanas de tecnico-, que es donde los lee quien diagnostica.
    if (emergenciaSubMaestroEl) {
      emergenciaSubMaestroEl.textContent = punta
        ? 'Ambas vías en rojo fijo'
        : 'POSTE 1 · ambas vías en rojo fijo';
    }
    if (emergenciaSubEsclavoEl) {
      emergenciaSubEsclavoEl.textContent = punta
        ? 'Intermitente · talanquera ABIERTA'
        : 'POSTE 2 · intermitente, talanquera ABIERTA';
    }
    if (!emergenciaHintEl) return;
    // El texto de la punta sin identificar carga con mas de lo que parece: los <small>
    // de los mandos estan APAGADOS por CSS en cualquier pantalla de menos de 740 px de
    // alto -o sea en todos los telefonos-, asi que cuando se ensenan las dos
    // emergencias esta linea es el UNICO sitio donde se dice cual es de cada poste.
    if (punta === 'MAESTRO') {
      emergenciaHintEl.textContent = 'POSTE 1: la parada de emergencia deja las dos ' +
        'vías en ROJO FIJO y detiene el tráfico.';
    } else if (punta === 'ESCLAVO') {
      emergenciaHintEl.textContent = 'POSTE 2: la parada de emergencia NO deja rojo. ' +
        'Pone ÁMBAR INTERMITENTE y ABRE la talanquera: los dos sentidos pasan con ' +
        'precaución. En gris, lo que solo se manda desde el POSTE 1.';
    } else {
      emergenciaHintEl.textContent = 'No se sabe qué poste hay al otro lado: ROJO ' +
        'TOTAL es del POSTE 1 y ÁMBAR EMERGENCIA del POSTE 2.';
    }
  }

  // =========================================================================
  // 4.ter QUE MANDOS HAY EN ESTE POSTE, VISTO ANTES DE PULSAR
  // =========================================================================
  // El cruce se opera desde el POSTE 1 -decision del responsable del 04/09-, y eso no
  // es un detalle de reparto: los cuatro mandos de ciclo -AUTOMATICO, DAR PASO, AMBAR
  // y VOLVER AL MENU- solo existen en el despachador de esa punta. Contra el POSTE 2 la
  // app ya frenaba la orden y avisaba, y ESE AVISO SE QUEDA porque es el unico que dice
  // por que. Lo que cambia es que dejar de servir se vea ANTES del dedo: enterarse
  // pulsando cuesta un intento delante de un cruce parado, y el que espera es el
  // trafico.
  //
  // LA DISPONIBILIDAD SE PREGUNTA A puntaCorrecta(), QUE ES LA MISMA FUNCION QUE DECIDE
  // SI LA ORDEN SALE AL CABLE. Escribir aqui una segunda lista de quien atiende que
  // seria la copia a mano que este repositorio ya pago tres veces (N-36, N-39 y la
  // propia compuerta): el boton diria una cosa, el cable otra, y las dos estarian
  // "bien" cada una por su lado hasta que alguien lo viera en el poste.
  //
  // Y NO SE APAGA NADA POR NO SABER. Con la punta sin identificar puntaCorrecta() de
  // SET_MODO y MANUAL:CAMBIAR_TURNO devuelve null -su guarda pregunta === 'ESCLAVO'- y
  // los cuatro salen normales, que es lo correcto: no saber no es saber que no.
  const MANDOS_DE_CICLO = [
    [btnOpAuto, 'SET_MODO'],
    [btnOpStep, 'MANUAL:CAMBIAR_TURNO'],
    [btnOpAmber, 'SET_MODO'],
    [btnOpMenu, 'SET_MODO'],
  ];

  function marcarDisponible(el, disponible) {
    if (!el) return;
    el.classList.toggle('no-disponible', !disponible);
    if (disponible) el.removeAttribute('aria-disabled');
    else el.setAttribute('aria-disabled', 'true');
  }

  function actualizarMandosDePunta(punta) {
    if (padPosteEl) {
      // Tres rotulos y no dos: "NO SE SABE" no es un poste, es la ausencia del dato, y
      // por eso tiene rotulo y color propios en vez de quedarse en blanco.
      padPosteEl.textContent = punta === 'MAESTRO' ? 'POSTE 1'
                             : punta === 'ESCLAVO' ? 'POSTE 2'
                             : 'NO SE SABE';
      padPosteEl.classList.toggle('pad-poste-sindato', !punta);
    }
    for (const par of MANDOS_DE_CICLO) {
      marcarDisponible(par[0], !puntaCorrecta(par[1]));
    }
    // Los dos de emergencia NO pasan por puntaCorrecta() a proposito, y es el mismo
    // motivo por el que sus manejadores tampoco: aquella resuelve SOLO_ESCLAVO por
    // descarte -su !== 'ESCLAVO' incluye el null- y con la punta sin identificar
    // apagaria el AMBAR DE EMERGENCIA, que es justo la caida segura que se ensena
    // cuando no se sabe con quien se habla. Aqui solo se apaga cuando CONSTA que al
    // otro lado hay la punta contraria.
    marcarDisponible(btnOpEmergency, state.node !== 'ESCLAVO');
    marcarDisponible(btnOpAmbarEmergencia, state.node !== 'MAESTRO');
  }

  // VOLVER AL MENU: la salida de cualquier modo, sin PIN (ver SIN_PIN arriba).
  //
  // Ni aqui ni en ningun otro sitio se pinta "ya esta en el menu": desde el Degradado
  // el firmware NO cambia de modo al recibirla, contesta
  // $ACK,...,RESULT:SALIENDO_TODO_ROJO y tarda todavia el todo-rojo entero. Lo que el
  // equipo hizo lo dicen $ACK y $STATUS; esta app solo sabe que mando la orden.
  if (btnOpMenu) {
    btnOpMenu.addEventListener('click', () => {
      const punta = puntaCorrecta('SET_MODO');
      if (punta) { avisarOtraPunta('SET_MODO:MENU', punta); return; }
      enviarComandoFirmware('SET_MODO:MENU');
      addEvent('cyan', 'Operario: orden VOLVER AL MENU enviada al equipo.');
    });
  }

  // PRUEBA DE ALCANCE: rojo fijo mientras se mide el enlace. Sin PIN, como MENU.
  if (btnModoAlcance) {
    btnModoAlcance.addEventListener('click', () => {
      const punta = puntaCorrecta('SET_MODO');
      if (punta) { avisarOtraPunta('SET_MODO:ALCANCE', punta); return; }
      enviarComandoFirmware('SET_MODO:ALCANCE');
      addEvent('cyan', 'Tecnico: orden PRUEBA DE ALCANCE enviada al equipo.');
    });
  }

  // =========================================================================
  // 4.ter MODO DEGRADADO: LA UNICA ORDEN QUE ENCIENDE UN VERDE SIN LA OTRA PUNTA
  // =========================================================================
  // Las otras ordenes de modo se piden y el coordinador confirma. Esta no: el equipo
  // se pone a dar paso guiandose por su reloj, y el escenario malo -las dos unidades
  // en verde a la vez- no lo detiene ninguna radio porque no hay radio. Por eso no
  // esta en la reja de mandos junto a las demas, sino detras de un dialogo que dice
  // lo que va a pasar y de una casilla que hay que marcar.
  //
  // El dialogo es del propio DOM y no un confirm() del navegador: el nativo bloquea
  // el hilo -colgo una corrida E2E, N75-4- y encima solo deja una linea de texto.
  //
  // El orden de las dos barreras no es indiferente: primero el PIN -que es la que el
  // firmware exige de verdad- y despues la casilla. Al reves, el operario leeria la
  // advertencia, la aceptaria, y le saldria un teclado despues de haber dicho que si.
  function abrirConfirmacionDegradado() {
    if (chkDegradadoVerificado) chkDegradadoVerificado.checked = false;
    if (btnDegradadoConfirmar) btnDegradadoConfirmar.disabled = true;
    openModal(degradadoModal);
  }

  if (btnModoDegradado) {
    btnModoDegradado.addEventListener('click', () => {
      const punta = puntaCorrecta('SET_MODO');
      if (punta) { avisarOtraPunta('SET_MODO:DEGRADADO', punta); return; }
      if (!state.pinVerificado) { pedirPin(() => btnModoDegradado.click()); return; }
      abrirConfirmacionDegradado();
    });
  }

  if (chkDegradadoVerificado && btnDegradadoConfirmar) {
    chkDegradadoVerificado.addEventListener('change', () => {
      btnDegradadoConfirmar.disabled = !chkDegradadoVerificado.checked;
    });
  }

  if (btnDegradadoCancelar) {
    btnDegradadoCancelar.addEventListener('click', () => {
      closeModal(degradadoModal);
      addEvent('cyan', 'Modo Degradado: entrada cancelada por el tecnico.');
    });
  }

  if (modalDegradadoClose) {
    modalDegradadoClose.addEventListener('click', () => closeModal(degradadoModal));
  }

  if (btnDegradadoConfirmar) {
    btnDegradadoConfirmar.addEventListener('click', () => {
      if (chkDegradadoVerificado && !chkDegradadoVerificado.checked) return;
      // La guarda de punta se repite AQUI aunque el boton que abre el dialogo ya la
      // tenga. No es redundancia decorativa: quien escribe al cable es esta linea, y
      // una barrera que vive en otro manejador solo protege mientras nadie llame a
      // este por otro camino. Ademas es la unica forma de que un censo pueda
      // comprobarlo sin seguir una cadena de dialogos -y una propiedad que solo se
      // puede verificar leyendo es una que se rompe sin que nadie se entere-.
      const puntaDeg = puntaCorrecta('SET_MODO');
      if (puntaDeg) { avisarOtraPunta('SET_MODO:DEGRADADO', puntaDeg); return; }
      closeModal(degradadoModal);
      // ESTA PUERTA NO TIENE GUARDA DE PIN DELANTE Y SET_MODO:DEGRADADO NO ESTA EN
      // SIN_PIN, asi que sin PIN verificado la llamada no escribia un byte y la linea
      // de abajo se imprimia igual: "orden enviada... esperando respuesta" sobre una
      // orden que no salio, y el tecnico esperando un $ACK que no puede llegar. Lo
      // encontro el pack app_05_sin_exito_mudo, no una lectura: era el quinto sitio
      // con este defecto y el unico que no estaba en la lista de partida. Y es el peor
      // de los cinco por lo que pide: el Degradado es el unico modo que enciende verde
      // sin confirmacion del otro extremo.
      if (!enviarComandoFirmware('SET_MODO:DEGRADADO')) return;
      // NO se pinta ningun modo: la puerta de modo_degradado_evaluarEntrada() rechaza
      // por seis motivos distintos y el que decide es el equipo. Si dice que no, llega
      // un $ERR con el motivo concreto y lo ensena el camino de rechazos de siempre.
      addEvent('cyan', 'Tecnico: orden MODO DEGRADADO enviada al equipo. Esperando ' +
                       'respuesta: el equipo puede rechazarla y dira por que.');
    });
  }

  // =========================================================================
  // 4.quater DEMANDA: UN CONTROL QUE REFLEJA EL MODO EN VEZ DE GASTAR UN RECHAZO
  // =========================================================================
  // El firmware solo mira la demanda estando en Modo Inteligente; en cualquier otro
  // contesta $ERR,CMD:DEMANDA,DESC:SOLO_EN_MODO_INTELIGENTE. Un boton que siempre se
  // puede pulsar y casi siempre falla ensena a no fiarse de los botones, asi que el
  // control sigue al MODO que dice $STATUS.
  //
  // Y sin telemetria queda apagado, no habilitado: no saber en que modo esta el equipo
  // no es lo mismo que saber que esta en Inteligente.
  function actualizarDemanda() {
    if (!btnDemanda) return;
    const disponible = state.telemetriaViva && state.modo === 'INTELIGENTE';
    btnDemanda.disabled = !disponible;
    if (!demandaHintEl) return;
    if (disponible) {
      demandaHintEl.textContent = 'El equipo esta en Modo Inteligente: la peticion ' +
                                  'entra en el ciclo.';
    } else if (state.telemetriaViva) {
      demandaHintEl.textContent = 'Solo sirve en Modo Inteligente. El equipo dice ' +
                                  'estar en ' + (state.modo || 'un modo sin nombre') + '.';
    } else {
      demandaHintEl.textContent = 'Sin datos del equipo: no se sabe en que modo esta.';
    }
  }


  // =========================================================================
  // 4.ante UN $ACK NO SIEMPRE SIGNIFICA "YA ESTA": HAY CUATRO ORDENES CON VARIOS SIES
  // =========================================================================
  // LA APP PINTABA TODOS LOS ACUSES IGUAL: "orden [X] ACEPTADA (LITERAL)". Y el
  // firmware se habia tomado el trabajo contrario -inventar un literal distinto para
  // cada final posible- precisamente porque NO significan lo mismo. Tragarselos en un
  // texto unico deshace ese trabajo en la ultima pantalla, que es la unica que alguien
  // lee de pie en la calzada.
  //
  // MEDIDO en las dos bluetooth.cpp: son CUATRO ordenes con mas de un RESULT posible.
  //
  //   CANCELAR_AMBAR    RETIRADO / RETIRADO_QUEDA_MANDO
  //   AMBAR_EMERGENCIA  OK / YA_EN_AMBAR_LATCH_PUESTO / SALIENDO_TODO_ROJO /
  //                     SALIDA_YA_EN_CURSO
  //   SET_MODO:MENU     OK / SALIENDO_TODO_ROJO
  //   SET_RTC           OK / HORA_PUESTA_SIN_PROPAGAR
  //
  // Y TODAS TIENEN LA MISMA FORMA DE TRAMPA: uno de los literales dice "aceptada, pero
  // lo que pediste TODAVIA NO HA PASADO" o "aceptada, pero SIGUE HABIENDO otra cosa".
  // Pintados como un si a secas, el tecnico se va del poste creyendo que termino.
  //
  // La tabla lleva los literales ESCRITOS ENTEROS para que el pack app_10 pueda cruzar
  // sus claves contra los literales del C++ en las dos direcciones: una respuesta que
  // el firmware manda y la tabla no nombra, y una que la tabla nombra y el firmware ya
  // no manda. Las dos hacen dano y no son la misma averia.
  const ACK_TEXTO = {
    'CANCELAR_AMBAR|RETIRADO': {
      tono: 'green',
      texto: 'Equipo: ambar de emergencia RETIRADO. No queda ningun ambar puesto en ' +
             'esa punta. Ojo: esta orden no mueve la luz, solo levanta el veto; la luz ' +
             'vuelve a moverse con la siguiente orden del Maestro.',
      toast: 'Ambar retirado por el equipo'
    },
    // LA QUE IMPORTA. El otro latch -el del mando del gabinete- NO se quita por radio:
    // los tres vetos de Esclavo/src/main.cpp miran las dos banderas, asi que con la del
    // mando puesta la luz sigue vetada. Un "ambar retirado" a secas manda al operario a
    // esperar un cambio de fase que no va a llegar hasta que alguien suba a hacer el
    // A.A.A, y mientras tanto el cruce sigue en ambar con la talanquera abierta.
    'CANCELAR_AMBAR|RETIRADO_QUEDA_MANDO': {
      tono: 'red',
      texto: 'Equipo: se retiro el ambar que puso la APP, pero QUEDA OTRO AMBAR PUESTO ' +
             'DESDE EL MANDO del gabinete. El cruce NO vuelve a ciclar: ese no se quita ' +
             'por radio. Hay que ir al poste y hacer la secuencia en el mando.',
      toast: 'QUEDA el ambar del mando: hay que ir al poste'
    },
    'AMBAR_EMERGENCIA|OK': {
      tono: 'red',
      texto: 'Equipo: AMBAR DE EMERGENCIA puesto. Intermitente en las dos vias y ' +
             'talanquera ABIERTA: no es un rojo, los dos sentidos pasan con precaucion.',
      toast: 'Ambar de emergencia puesto'
    },
    // Ya estaba en ambar por SFTY-6, por el watchdog o por un B.B.B. Lo que esta orden
    // cambio NO es la luz: es el latch, que convierte un ambar que la siguiente orden
    // del Maestro se llevaria en uno vetado. Contestar "puesto" ocultaria que lo unico
    // nuevo es la proteccion, y con ella el tecnico decide distinto.
    'AMBAR_EMERGENCIA|YA_EN_AMBAR_LATCH_PUESTO': {
      tono: 'green',
      texto: 'Equipo: YA ESTABA en ambar por otro motivo. Lo que anade esta orden es la ' +
             'PROTECCION: a partir de ahora el ambar no se lo lleva la siguiente orden ' +
             'del Maestro. Para quitarlo hace falta RETIRAR AMBAR.',
      toast: 'Ya estaba en ambar - ahora ademas queda protegido'
    },
    // El ambar NO esta puesto todavia: el equipo esta saliendo del Degradado por
    // todo-rojo y eso tarda hasta 90 s (cfgDespeje). Decir OK seria dar por hecho un
    // cambio de luz que aun no ha ocurrido.
    'AMBAR_EMERGENCIA|SALIENDO_TODO_ROJO': {
      tono: 'red',
      texto: 'Equipo: orden ACEPTADA pero el ambar TODAVIA NO ESTA PUESTO. La unidad ' +
             'esta saliendo del Modo Degradado por TODO ROJO y eso tarda hasta 90 s. ' +
             'El ambar entra al terminar esa salida: no se vaya sin verlo.',
      toast: 'Aceptada - el ambar entra al acabar el todo-rojo (hasta 90 s)'
    },
    'AMBAR_EMERGENCIA|SALIDA_YA_EN_CURSO': {
      tono: 'red',
      texto: 'Equipo: orden ACEPTADA pero el ambar TODAVIA NO ESTA PUESTO. Ya habia una ' +
             'salida en curso (rendicion por el limite de 48 h) que termina en ambar; ' +
             'esta orden no la arranca, lo que anade es que ese ambar quede protegido.',
      toast: 'Aceptada - ya habia una salida en curso que acaba en ambar'
    },
    'SET_MODO:MENU|OK': {
      tono: 'green',
      texto: 'Equipo: orden VOLVER AL MENU aceptada. La unidad queda en la pantalla, ' +
             'sin ciclo.',
      toast: 'El equipo vuelve al menu'
    },
    // En Degradado no se salta al menu: se pide la salida, que es un todo-rojo de 30 s.
    'SET_MODO:MENU|SALIENDO_TODO_ROJO': {
      tono: 'red',
      texto: 'Equipo: orden ACEPTADA pero TODAVIA NO ESTA EN EL MENU. Estaba en Modo ' +
             'Degradado y la salida pasa por un TODO ROJO de transicion; el menu llega ' +
             'cuando ese todo-rojo termine.',
      toast: 'Aceptada - sale del Degradado por todo-rojo antes de llegar al menu'
    },
    'SET_RTC|OK': {
      tono: 'green',
      texto: 'Equipo: hora puesta en el Maestro y propagada al Esclavo.',
      toast: 'Hora puesta y propagada'
    },
    // La hora entro AQUI y no salio hacia la otra punta. Es medio arreglo, y un "OK" lo
    // haria pasar por entero: el Esclavo se quedaria con la hora vieja, que es
    // exactamente de lo que cuelga la autorizacion del Modo Degradado.
    'SET_RTC|HORA_PUESTA_SIN_PROPAGAR': {
      tono: 'red',
      texto: 'Equipo: la hora entro en el MAESTRO pero NO se propago al ESCLAVO. La ' +
             'otra punta sigue con la hora anterior, y de esa hora cuelga la ' +
             'autorizacion del Modo Degradado. Repita la sincronizacion.',
      toast: 'Hora puesta solo en el Maestro: NO llego al Esclavo'
    }
  };

  // Los rechazos ya se leen como rechazos lleve el texto que lleve, asi que aqui solo
  // se traduce lo que el literal en crudo haria entender MAL. NO_HAY_AMBAR_VIGENTE
  // parece una averia y no lo es: es el equipo diciendo que no habia nada que quitar,
  // que es un dato util -si el cruce sigue en ambar, viene de otro sitio-.
  const ERR_TEXTO = {
    'CANCELAR_AMBAR|NO_HAY_AMBAR_VIGENTE': {
      texto: 'Equipo: NO habia ningun ambar de emergencia puesto desde la app, asi que ' +
             'no se retiro nada. Si el cruce sigue en ambar viene del mando del ' +
             'gabinete o de un fallo de comunicacion (SFTY-6), y esta orden no quita ' +
             'ninguno de los dos.',
      toast: 'No habia ambar de la app que retirar'
    }
  };

  // =========================================================================
  // 4.bis INGESTA DE TELEMETRIA DEL EQUIPO ($STATUS / $ALARM / $ERR)
  //
  // N-75: el rewrite de V9.0 borro este camino entero. La app quedo SORDA: mandaba
  // comandos y pintaba un estado inventado por el propio telefono -renderLights() con
  // lo que el handler local acababa de escribir en state-, asi que el tecnico veia en
  // la pantalla un verde que nadie le habia confirmado. Es la version movil de la
  // prueba muerta: todo en pantalla decia PASS sin haber medido el equipo.
  //
  // El bloque vuelve LITERAL de 8d75f4c. Los campos son los que emite bluetooth.cpp:
  // NODE, SERIE, MODO, ESTADO, T, RF, RTT, BAT, HORA -y el pack documentos_03 exige
  // que esta lista y la del firmware sean la misma-.
  // =========================================================================
  function _camposNmea(parts) {
    const data = {};
    for (let i = 1; i < parts.length; i++) {
      // N-62: el separador de campo es ',' y el de clave/valor es el PRIMER ':'.
      // Con split(':') a secas, HORA:18:25:00 entraba como '18' y el reloj en vivo
      // mostraba la hora truncada. Se corta por el primer ':' y el resto es valor.
      const sep = parts[i].indexOf(':');
      const k = sep > 0 ? parts[i].slice(0, sep) : null;
      const v = sep > 0 ? parts[i].slice(sep + 1) : undefined;
      if (k && v !== undefined) data[k] = v;
    }
    return data;
  }

  function parseNmeaTelemetry(line) {
    if (!line || typeof line !== 'string') return;

    // LA PUERTA. Antes aqui habia `if (!line.startsWith('$')) return;` y luego
    // `line.split('*')[0]`, que TIRA EL CHECKSUM SIN MIRARLO: una trama corrompida por
    // la radio entraba a pintar con los bytes que trajera dentro. Ahora se juzga, se
    // anota en la cinta lo que llego, y lo que no pasa NO PINTA NADA -pero deja rastro
    // con su motivo, que es lo contrario de la vuelta silenciosa de antes-.
    const veredicto = juzgarTrama(line);
    const anotado = RegistroCrudo.anotar(line, veredicto);
    if (!veredicto.aceptada) {
      registrarRechazoEnlace(veredicto, line);
      renderDepuracion();
      return;
    }
    const parts = veredicto.partes;
    const header = veredicto.tipo;

    if (header === '$STATUS') {
      const data = _camposNmea(parts);

      // Cada $STATUS es la prueba de que el equipo sigue ahi. El watchdog de abajo la
      // usa para decidir si el tablero puede seguir mostrando lo que muestra.
      state.ultimoStatusMs = Date.now();
      if (!state.telemetriaViva) marcarConEnlace();

      if (data.NODE) {
        state.node = data.NODE;
        if (nodeNameEl) {
          nodeNameEl.textContent = data.NODE === 'MAESTRO' ? 'MAESTRO (POSTE 1)' : 'ESCLAVO (POSTE 2)';
        }
        // La emergencia no es la misma maniobra en cada poste: el mando que se ensena
        // lo decide esta trama, no lo que el operario supone que tiene delante.
        actualizarEmergencia();
      }
      // N-62: aqui se leian SITE y PAIR, y NINGUNA punta los emite. No daba error:
      // dejaba la pantalla igual para siempre, que es la forma silenciosa del fallo.
      // SITE lo gobierna el gestor de cruces de la propia app (LocalStorage) y PAIR
      // depende de FW-PAIR, que todavia no esta en el firmware.
      if (data.SERIE) {
        state.serie = data.SERIE;
      }
      // Se guarda lo que VENGA, incluido el vacio. Los dos switch del firmware tienen
      // salida por abajo -semaforo_nombreEstado() devuelve "" si el estado no es
      // ninguno de los cuatro; obtenerNombreModo() devuelve "DESCONOCIDO"-, y un
      // ESTADO vacio con el `if (data.ESTADO)` de antes NO se guardaba: la pantalla se
      // quedaba con el ultimo estado bueno pintado como si fuera de ahora. Un campo
      // que llega vacio es un dato -dice que el equipo no sabe-, no una trama sin
      // campo. renderLights() lo declara.
      if (data.MODO !== undefined) {
        state.modo = data.MODO;
      }
      if (data.ESTADO !== undefined) {
        state.estadoLuces = data.ESTADO;
      }
      if (data.MODO !== undefined || data.ESTADO !== undefined) {
        renderLights();
      }
      // El control de DEMANDA se abre y se cierra con el MODO que acaba de llegar, no
      // con lo que la app haya pedido: pedir Inteligente no es estar en Inteligente.
      if (data.MODO) {
        actualizarDemanda();
      }
      if (data.T) {
        state.countdown = parseInt(data.T, 10) || 0;
        updateCountdownRing();
      }
      // EL ENLACE. Un solo camino: la trama -> lecturaDeEnlace() -> pintarEnlace().
      //
      // Aqui habia `rfQualityEl.textContent = (parseInt(data.RF, 10) || 0) + '%'`, y
      // ese `|| 0` es el defecto: con un RF: que no fuera un numero -ausente, vacio,
      // "--", o lo que emita el firmware el dia que el Esclavo deje de inventarselo-
      // la pantalla escribia **0%**, o sea el peor enlace medible, sin que nadie
      // hubiera medido nada. "No lo se" y "va fatal" son cosas distintas y esa linea
      // las juntaba. Ahora se pregunta SIEMPRE, aunque el campo no venga: es
      // pintarEnlace() quien decide, y sabe declarar la ausencia.
      const lectura = lecturaDeEnlace(data);
      pintarEnlace(lectura);
      registrarMuestraEnlace(lectura);
      if (data.BAT !== undefined) {
        // N-108 (31/08): EL FIRMWARE DEJO DE INVENTARSE ESTE NUMERO Y AHORA MANDA "--".
        //
        // Hasta ese commit las dos puntas emitian el literal "BAT:12.6" en su snprintf
        // -sin divisor ni ADC detras- y la app lo pintaba con una nota debajo, que era
        // lo unico que se podia hacer desde este lado. Ahora el campo llega MARCADO, y
        // la app tiene que leer la marca en vez de seguir describiendo el defecto
        // anterior: un texto que dice "valor fijo del firmware" debajo de un "-- V"
        // acusa al equipo de algo que ya no hace.
        //
        // Se usa el MISMO vocabulario de ausencia que el enlace (RF_NO_MEDIDO): las
        // tres cifras de la trama se marcan igual, asi que se leen igual.
        const crudoBat = String(data.BAT).trim();
        const noMedida = RF_NO_MEDIDO.indexOf(crudoBat.toUpperCase()) >= 0;
        const volt = noMedida ? NaN : parseFloat(crudoBat);
        state.battery = Number.isFinite(volt) ? volt : null;
        if (batVoltageEl) {
          batVoltageEl.textContent = state.battery === null
            ? '-- V' : state.battery.toFixed(1) + 'V';
        }
        if (batStatusEl) {
          batStatusEl.textContent = state.battery === null
            ? 'El equipo declara que no la mide: falta el divisor y la entrada analogica'
            : 'Medida del equipo';
        }
      }
      if (data.HORA) {
        state.hora = data.HORA;
      }
      // Los reparos se apuntan AQUI ABAJO y no en el juez, y el sitio importa: a esta
      // altura ya esta decidido lo que se pinto -la lectura de enlace y state.battery
      // salieron de las mismas funciones que escriben la pantalla-. Calcularlos antes
      // obligaria a una segunda copia de la regla, y una segunda copia es la que se
      // queda vieja sin avisar y hace que la cinta cuente algo distinto de lo que paso.
      if (anotado) {
        reparosDeStatus(data, lectura).forEach(x => anotado.reparos.push(x));
      }
    } else if (header === '$ALARM') {
      const data = _camposNmea(parts);
      showToast('ALERTA: ' + (data.EVENTO || 'Fallo detectado'));

      // N-108: EL $ALARM YA NO DICE SOLO QUE SE CAYO, DICE DESDE DONDE VENIA.
      //
      // El firmware mete en la propia alarma el ultimo tramo del enlace, y NO es el
      // mismo dato en las dos puntas, porque las dos puntas no saben lo mismo:
      //
      //   MAESTRO   RF, RTT y SINRESP  -el manda los latidos, asi que sabe cuantos
      //             salieron y cuantos volvieron-.
      //   ESCLAVO   RX, OK y RUIDO     -solo contesta: no tiene ventana de latidos ni
      //             ida y vuelta que cronometrar. Los tres contadores separan tres
      //             averias que desde el suelo se ven todas igual: RX en 0 es que no
      //             llega nada; RX alto con OK en 0 es que llega basura; los dos
      //             subiendo con RUIDO alto es enlace marginal-.
      //
      // Se prefiere ESTE dato al ultimo que vio la app, y la diferencia importa: el de
      // la alarma lo midio el equipo EN EL INSTANTE DE LA CAIDA, mientras que el de la
      // app es el ultimo $STATUS que llego -que por definicion es anterior, y puede ser
      // de bastante antes si la app estaba en segundo plano-.
      const lecturaAlarma = lecturaDeEnlace(data);
      const tramo = [];
      if (data.SINRESP !== undefined) tramo.push('latidos sin respuesta: ' + data.SINRESP);
      if (data.RX !== undefined) tramo.push('bytes recibidos: ' + data.RX);
      if (data.OK !== undefined) tramo.push('tramas validas: ' + data.OK);
      if (data.RUIDO !== undefined) tramo.push('tramas descartadas: ' + data.RUIDO);
      if (data.RTT !== undefined) tramo.push('RTT: ' + data.RTT);

      // Cuando la alarma NO trae enlace medido -la del ESCLAVO no lo trae nunca, porque
      // esa punta no puede medirlo- la columna del registro se queda VACIA y el ultimo
      // valor que si vio la app va en el texto CON SU HORA. NO se rellena con el:
      // seria pegarle a la alarma de las 03:41 un numero de las 03:40, y encima uno que
      // en este caso puede venir de la OTRA punta -la app pinta el $STATUS del equipo
      // que este hablando-. Es la misma regla que la anotacion de CAIDA.
      const ultimoVisto = (!lecturaAlarma.medido && state.rfQuality !== null)
        ? ' [la app vio por ultima vez ' + state.rfQuality + '% a las ' +
          _horaDe(state.rfMedidaMs) + ', que puede ser de la otra punta]'
        : '';
      const textoAlarma = 'Caja Negra: ' + (data.EVENTO || 'FALLO') + ' - ' +
                          (data.CAUSA || '') + ' (Accion: ' + (data.ACCION || '') + ')' +
                          (tramo.length ? ' [ultimo tramo: ' + tramo.join(', ') + ']' : '') +
                          ultimoVisto;
      addEvent('red', textoAlarma);
      // El registro de eventos de la app vivia SOLO en memoria: al cerrarse la app se
      // perdia, y el $ALARM de la caida de las 03:40 no lo leia nunca nadie. Ahora la
      // alarma se guarda, y con el tramo que midio el equipo al lado.
      RegistroEnlace.anotar('ALARMA', lecturaAlarma, textoAlarma);
      renderRegistroEnlace();
    } else if (header === '$ACK') {
      // El firmware acusa CADA orden que acepta, y la app se lo tragaba: solo pintaba
      // los rechazos. De modo que el operario veia "orden enviada" -que es lo que sabe
      // la app- y nunca "orden aceptada" -que es lo que sabe el equipo-, que son cosas
      // distintas cuando el enlace es una radio. Ahora la unica confirmacion que se
      // muestra viene del equipo.
      const data = _camposNmea(parts);
      const cual = data.CMD || '?';
      const clave = cual + '|' + (data.RESULT || '');
      const dicho = ACK_TEXTO[clave];
      if (dicho) {
        addEvent(dicho.tono, dicho.texto);
        showToast(dicho.toast);
      } else {
        // FALLBACK, Y SE QUEDA A PROPOSITO. Un literal nuevo del firmware tiene que
        // llegar a la pantalla aunque nadie le haya escrito todavia su texto: lo que no
        // puede pasar es que desaparezca. El pack app_10 se pone rojo cuando aparece un
        // RESULT que la tabla no nombra, asi que este camino es la red, no el destino.
        addEvent('green', 'Equipo: orden [' + cual + '] ACEPTADA' +
                          (data.RESULT ? ' (' + data.RESULT + ')' : ''));
        showToast('Aceptado por el equipo: ' + cual);
      }
    } else if (header === '$EVENT') {
      // $EVENT es la bitacora del propio equipo -quien movio que y desde donde-. No la
      // leia nadie, asi que el registro de eventos de la app solo contenia lo que la
      // app misma habia hecho: una bitacora que no sabe nada de lo que pasa en el poste.
      const data = _camposNmea(parts);
      const textoEvento = 'Equipo [' + (data.ORIGEN || 'FIRMWARE') + ']: ' +
                          (data.DETALLE || '') + (data.HORA ? ' - ' + data.HORA : '');
      addEvent('cyan', textoEvento);
      RegistroEnlace.anotar('EVENTO', enlaceDeAhora(), textoEvento);
      renderRegistroEnlace();
    } else if (header === '$ERR') {
      const data = _camposNmea(parts);
      // Un rechazo del firmware NO se oculta: si el equipo dijo que no, la pantalla
      // tiene que decirlo tambien, o el operario se va creyendo que la orden entro.
      //
      // NO_HAY_AMBAR_VIGENTE se traduce porque es la tercera respuesta de R-3 y su
      // literal, leido tal cual, se confunde con una averia. No lo es: es el equipo
      // diciendo que no habia nada que quitar, y eso es una respuesta util -significa
      // que si el cruce sigue en ambar, viene de otro sitio-.
      const negado = ERR_TEXTO[(data.CMD || '?') + '|' + (data.DESC || '')];
      if (negado) {
        addEvent('red', negado.texto);
        showToast(negado.toast);
      } else {
        addEvent('red', 'Rechazo de Firmware: [' + (data.CMD || '?') + '] ' + (data.DESC || ''));
        showToast('Rechazado por el equipo: ' + (data.DESC || 'ver eventos'));
      }
    }
    // La cinta se repinta una sola vez por trama y al final, cuando los reparos ya
    // estan puestos: hacerlo arriba ensenaria la trama sin ellos y el usuario veria la
    // linea cambiar sola.
    renderDepuracion();
  }

  // Despachador unico de los botones que declaran su orden en data-cmd. Un solo sitio
  // que traduce el atributo a comando, para que el boton del HTML y lo que sale por
  // Bluetooth no puedan divergir: el pack app_01_comandos lee justo ese atributo.
  //
  // LAS DOS PUNTAS ACEPTAN CONJUNTOS DISTINTOS, y la app mandaba a ciegas. SOLICITAR_PASO
  // solo lo entiende el Esclavo (Esclavo/src/bluetooth.cpp:128) y SET_MODO solo el
  // Maestro (Maestro/src/bluetooth.cpp:123-137): pulsados contra la punta equivocada
  // devolvian $ERR,CMD:DESCONOCIDO y el boton parecia roto. El nodo lo dice $STATUS.
  // DEMANDA y REINICIAR_RELOJ tampoco existen en el Esclavo: su despachador no las
  // conoce y contestaria $ERR,CMD:DESCONOCIDO, que es el error que no dice nada.
  //
  // Y desde el 28/08 la emergencia tambien esta repartida, que es el caso nuevo: no es
  // que a una punta le falte la orden, es que CADA UNA TIENE LA SUYA y hacen cosas
  // distintas. El Esclavo dejo de aceptar FORZAR_ROJO -lo rechaza nombrando el literal
  // nuevo- y el Maestro no conoce AMBAR_EMERGENCIA. Los dos mandos de emergencia
  // consultan state.node por su cuenta (ver 4.bis) porque no pueden resolver por
  // descarte con la punta sin identificar, pero el reparto se apunta aqui, que es el
  // unico sitio donde esta escrito quien atiende que.
  const SOLO_MAESTRO = ['SET_MODO', 'MANUAL:CAMBIAR_TURNO', 'SET_TIEMPOS', 'TEST_LEDS',
                        'DEMANDA', 'REINICIAR_RELOJ', 'FORZAR_ROJO'];
  // CANCELAR_AMBAR (R-3, 31/08) vive en el Esclavo por la misma razon que
  // AMBAR_EMERGENCIA: es esa punta la que tiene el latch. El Maestro no conoce el
  // literal, asi que sin esta linea el boton saldria al cable contra un Maestro y
  // volveria como $ERR,CMD:DESCONOCIDO -el error que parece un boton roto-.
  const SOLO_ESCLAVO = ['SOLICITAR_PASO', 'AMBAR_EMERGENCIA', 'CANCELAR_AMBAR'];

  // 🟠 N-124, VENTANA CONOCIDA Y ABIERTA A PROPOSITO - NO ES UN DESCUIDO.
  //
  // `SOLO_MAESTRO` pregunta `=== 'ESCLAVO'`, asi que con `state.node` en null -el segundo
  // escaso entre que el socket abre y llega el primer $STATUS- una orden de Maestro SI
  // sale al cable. `SOLO_ESCLAVO` no tiene esa ventana: su `!== 'ESCLAVO'` incluye null.
  //
  // Antes la tapaba una punta ADIVINADA de una fila fija del HTML, cuyo MAC ademas era
  // falso. Al dejar que la punta la diga el equipo (N-124) la ventana queda al aire.
  //
  // SE PROBO A CERRARLA -poniendo `!== 'MAESTRO'`- y se retiro, porque rompe el arnes del
  // puente: `simulador_puente_esp32.py` pulsa AUTOMATICO en el instante siguiente a
  // conectar, sin esperar al primer $STATUS, y la app dejaba de emitir nada. Arreglarlo
  // de verdad es que el arnes entregue un $STATUS antes de pulsar -que es lo que pasa en
  // la realidad-, y eso se toca con calma, no con prisa y sobre una guarda de seguridad.
  //
  // LO QUE CUESTA MIENTRAS TANTO, medido y acotado: ~1 s por conexion en el que una orden
  // SOLO_MAESTRO puede salir contra un Esclavo. El firmware la rechaza con
  // $ERR,CMD:DESCONOCIDO, asi que no mueve una luz; lo que se pierde es el aviso claro al
  // operario. Queda anotado en el roadmap y no se cierra desde aqui.
  function puntaCorrecta(comando) {
    if (SOLO_MAESTRO.includes(comando) && state.node === 'ESCLAVO') return 'MAESTRO';
    if (SOLO_ESCLAVO.includes(comando) && state.node !== 'ESCLAVO') return 'ESCLAVO';
    return null;
  }

  // El aviso vive en una sola funcion porque lo usan tanto el despachador de data-cmd
  // como los mandos que llevan su literal a la vista: dos textos para el mismo hecho
  // acabarian diciendo cosas distintas del mismo rechazo.
  function avisarOtraPunta(orden, punta) {
    showToast('Esa orden la atiende el ' + punta + ', no esta punta');
    addEvent('red', 'Orden ' + orden + ' no enviada: la acepta el ' + punta +
                    ' y ahora mismo hay un ' + (state.node || '?') + ' al otro lado.');
  }

  document.querySelectorAll('[data-cmd]').forEach(btn => {
    btn.addEventListener('click', () => {
      const orden = btn.getAttribute('data-cmd');
      if (!orden) return;
      const sep = orden.indexOf(':');
      const comando = sep > 0 ? orden.slice(0, sep) : orden;
      const args = sep > 0 ? orden.slice(sep + 1) : '';

      const punta = puntaCorrecta(comando);
      if (punta) {
        avisarOtraPunta(orden, punta);
        return;
      }

      if (!state.pinVerificado) { pedirPin(() => btn.click()); return; }
      enviarComandoFirmware(comando, args);
      addEvent('cyan', 'Tecnico: orden ' + orden + ' enviada al equipo.');
    });
  });

  // =========================================================================
  // 5. GESTOR DE CRUCES VIALES (RENDERIZADO Y SELECCIÓN)
  // =========================================================================
  function renderSiteList(filter = '') {
    if (!dynamicSiteListEl) return;
    const sites = SiteManager.filtrarCruces(filter);
    dynamicSiteListEl.innerHTML = '';

    sites.forEach(site => {
      const item = document.createElement('div');
      item.className = 'bt-device-item site-card';
      item.style.marginBottom = '8px';
      item.innerHTML = `
        <div class="bt-dev-icon">📍</div>
        <div class="bt-dev-info">
          <strong class="site-card-title">${site.nombre}</strong>
          <small>${site.ubicacion}</small>
        </div>
        <button class="btn-top btn-edit-site" title="Renombrar" style="padding: 4px 8px; font-size: 10px;">✏️</button>
        <button class="btn-top btn-delete-site" title="Eliminar" style="padding: 4px 8px; font-size: 10px;">🗑️</button>
        <button class="btn-top" style="padding: 4px 10px; font-size: 10px;">Seleccionar</button>
      `;

      // Los dos mandos de la tarjeta paran la propagacion: pulsar la papelera no
      // puede ademas seleccionar el cruce que se esta borrando.
      item.querySelector('.btn-edit-site').addEventListener('click', (ev) => {
        ev.stopPropagation();
        const nombre = prompt('Nuevo nombre del cruce:', site.nombre);
        if (!nombre || !nombre.trim()) return;
        SiteManager.actualizarCruce(site.id, nombre.trim(), site.ubicacion, site.p1, site.p2);
        // Si el cruce editado es el que esta activo, la cabecera cambia con el:
        // un rotulo que se queda con el nombre viejo es un cruce mal identificado.
        if (currentSiteNameEl && state.site && site.nombre.includes(state.site)) {
          state.site = nombre.replace('📍', '').trim();
          currentSiteNameEl.textContent = '📍 ' + state.site;
        }
        renderSiteList(siteSearchInput ? siteSearchInput.value : '');
        addEvent('cyan', 'Cruce renombrado a ' + nombre.trim());
      });

      item.querySelector('.btn-delete-site').addEventListener('click', (ev) => {
        ev.stopPropagation();
        // SiteManager se niega a dejar la lista vacia: un gestor sin ningun cruce
        // deja la app sin identidad de sitio.
        SiteManager.eliminarCruce(site.id);
        renderSiteList(siteSearchInput ? siteSearchInput.value : '');
      });

      item.addEventListener('click', () => {
        state.site = site.nombre.replace('📍', '').trim();
        if (currentSiteNameEl) currentSiteNameEl.textContent = `📍 ${state.site}`;
        closeModal(siteModal);
        showToast(`📍 Cruce seleccionado: ${state.site}`);
        addEvent('cyan', `Cruce seleccionado: ${state.site}`);
      });
      dynamicSiteListEl.appendChild(item);
    });
  }

  if (siteSearchInput) {
    siteSearchInput.addEventListener('input', (e) => {
      renderSiteList(e.target.value);
    });
  }

  if (btnOpenAddSite) {
    btnOpenAddSite.addEventListener('click', () => {
      const nombre = prompt('Ingrese el nombre del nuevo cruce:', 'Tramo Km ' + Math.floor(Math.random() * 80 + 10));
      if (nombre && nombre.trim()) {
        const ubicacion = prompt('Ingrese la ubicación o PR:', 'PR ' + Math.floor(Math.random() * 50) + '+000');
        const nuevo = SiteManager.agregarCruce(nombre.trim(), ubicacion ? ubicacion.trim() : '');
        // Se activa en el acto. Crear un cruce y quedarse en el anterior obliga a un
        // segundo paso que nadie recuerda, y el rotulo de la cabecera es lo unico que
        // le dice al tecnico sobre que cruce esta mandando ordenes.
        state.site = nuevo.nombre.replace('📍', '').trim();
        if (currentSiteNameEl) currentSiteNameEl.textContent = '📍 ' + state.site;
        renderSiteList();
        showToast(`✓ Cruce "${nuevo.nombre}" guardado.`);
      }
    });
  }

  // =========================================================================
  // 6. GESTIÓN BLUETOOTH Y DISPOSITIVOS
  // =========================================================================
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
      state.connected = false;
      state.deviceName = name;
      state.deviceMac = mac;

      // N-124: LA PUNTA NO SE ADIVINA. Aqui se leia un data-node de la fila, y las dos
      // filas venian escritas a mano en el HTML: la app afirmaba tener un MAESTRO
      // delante antes de que ningun equipo hubiera hablado. Con la lista pintada por el
      // escaneo eso ya no se puede saber al pulsar, y no hace falta: NODE: del $STATUS
      // llega cada segundo y lo escribe abajo (parseNmeaTelemetry). Hasta entonces el
      // rotulo dice lo unico que consta, que es que todavia no se sabe.
      state.node = null;
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
        window.bluetoothSerial.connect(
          mac,
          () => {
            state.connected = true;
            if (btnDevice) btnDevice.className = 'btn-top btn-device connected';
            showToast(`🔗 Enlazado a: ${name}`);
            addEvent('green', `Bluetooth conectado: ${name} (${mac})`);

            // N-75: sin esto la app no oye al equipo. El firmware emite $STATUS solo,
            // cada segundo, asi que NO se pide nada al conectar (N-66: GET_STATUS no
            // existe en ninguna punta y lo primero que veia el tecnico era un $ERR).
            window.bluetoothSerial.subscribe(
              '\n',
              (data) => {
                if (data && String(data).trim()) parseNmeaTelemetry(String(data).trim());
              },
              (err) => console.warn('Error en suscripcion serie:', err)
            );
          },
          (err) => {
            // NO CONECTAR NO ES CONECTAR. Se dice, y se deja el estado en falso: pintar
            // "Enlazado" sobre un socket que no abrio es lo que hizo que el banco
            // gastara cuatro pasos buscando el fallo en el modulo.
            state.connected = false;
            if (btnDevice) btnDevice.className = 'btn-top btn-device';
            showToast('❌ No se pudo conectar');
            addEvent('red', `Bluetooth NO conectado a ${name} (${mac}): ${err}. ` +
                            `Comprueba que el equipo este emparejado en Ajustes de Android.`);
          }
        );
      } else {
        // Fuera del APK no hay radio. Se declara en vez de simular un enlace.
        state.connected = false;
        addEvent('red', 'Sin radio en este entorno: no se abrio ningun enlace.');
      }

      closeModal(btModal);
    });
  }

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
  function mensajeEnLista(texto) {
    if (!btDeviceListContainer) return;
    btDeviceListContainer.innerHTML = '';
    const p = document.createElement('p');
    p.className = 'modal-desc';
    p.textContent = texto;
    btDeviceListContainer.appendChild(p);
  }

  function pintarEquipos(dispositivos) {
    if (!btDeviceListContainer) return 0;
    const lista = Array.isArray(dispositivos) ? dispositivos : [];
    btDeviceListContainer.innerHTML = '';
    let pintados = 0;
    lista.forEach(d => {
      // El MAC es lo unico que connect() sabe marcar, y el plugin lo entrega en
      // `address` o en `id` segun la version. Un equipo sin direccion no se pinta: una
      // fila que no se puede pulsar es peor que una fila que no esta.
      const mac = d && (d.address || d.id);
      if (!mac) return;
      const nombre = (d && d.name && String(d.name).trim()) || String(mac);

      const item = document.createElement('div');
      item.className = 'bt-device-item';
      item.setAttribute('data-name', nombre);
      item.setAttribute('data-mac', String(mac));
      // NO se escribe data-node: una fila no puede saber que punta hay al otro lado.
      // Lo dice el equipo en NODE: del $STATUS.

      const icono = document.createElement('div');
      icono.className = 'bt-dev-icon';
      icono.textContent = '📡';

      const info = document.createElement('div');
      info.className = 'bt-dev-info';
      const titulo = document.createElement('strong');
      // textContent y no innerHTML: el nombre lo pone el modulo del otro lado, no la
      // app, y un texto que llega de fuera no se inyecta como marcado.
      titulo.textContent = nombre;
      const detalle = document.createElement('small');
      detalle.textContent = 'MAC: ' + mac;
      info.appendChild(titulo);
      info.appendChild(detalle);

      const etiqueta = document.createElement('span');
      etiqueta.className = 'bt-dev-badge';
      // "emparejado" es lo unico que consta de esta fila. Poner "Maestro" o "Esclavo"
      // seria la misma invencion que los dos MAC que se acaban de retirar.
      etiqueta.textContent = 'emparejado';

      item.appendChild(icono);
      item.appendChild(info);
      item.appendChild(etiqueta);
      btDeviceListContainer.appendChild(item);
      pintados += 1;
    });
    return pintados;
  }

  // porPeticion = lo pulso el tecnico. Al abrir el modal se llama con false: si hay
  // radio la lista se rellena sola -es la misma busqueda que hace el boton- y si no la
  // hay se vuelve en silencio dejando el aviso que trae el HTML. Anunciar "no hay
  // radio" cada vez que se abre el modal en un navegador seria ruido; callarselo
  // cuando alguien PIDE la busqueda seria lo otro, que es peor: se le contesta que no
  // se pudo buscar, que no es lo mismo que no haber encontrado.
  function escanearEquipos(porPeticion) {
    const hayRadio = typeof window !== 'undefined' && window.bluetoothSerial &&
                     typeof window.bluetoothSerial.list === 'function';
    if (!hayRadio) {
      if (!porPeticion) return;
      showToast('Escaneo no disponible fuera del APK');
      addEvent('red', 'Escaneo Bluetooth no realizado: no hay radio disponible en ' +
                      'este entorno. No se sabe que equipos hay, que no es lo mismo ' +
                      'que no haberlos encontrado.');
      mensajeEnLista('Sin radio en este entorno: no se pudo buscar. Esta lista está ' +
                     'vacía porque no hubo búsqueda, no porque no haya equipos.');
      return;
    }
    // El rotulo NO nombra el modulo. En obra ya se ha cambiado de radio mas de una vez
    // -del HC-05 al ESP32-, asi que un texto que diga "HC-05" queda desmintiendo al
    // equipo el dia que se sustituya. "El enlace" vale para todos.
    if (porPeticion) showToast('🔍 Buscando equipos...');
    window.bluetoothSerial.list(
      (dispositivos) => {
        const pintados = pintarEquipos(dispositivos);
        if (!pintados) {
          // CERO EMPAREJADOS NO ES CERO EQUIPOS. list() devuelve lo que ESTE telefono
          // tiene emparejado en Ajustes de Android, no lo que hay encendido al lado.
          mensajeEnLista('Este teléfono no tiene ningún equipo emparejado. El ' +
                         'emparejamiento se hace en Ajustes de Android; aquí solo se ' +
                         'listan los que ya lo están.');
          addEvent('red', 'Escaneo Bluetooth: 0 equipos emparejados en este telefono. ' +
                          'Empareje el modulo en Ajustes de Android y repita. No se ha ' +
                          'buscado equipos nuevos: list() solo lee los emparejados.');
          return;
        }
        showToast(pintados + ' equipo(s) emparejado(s)');
        addEvent('green', 'Escaneo Bluetooth: ' + pintados + ' equipo(s) emparejado(s) ' +
                          'listados con su MAC real.');
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
      }
    );
  }

  if (btnScanBluetoothLive) {
    btnScanBluetoothLive.addEventListener('click', () => escanearEquipos(true));
  }

  // =========================================================================
  // 7. FORMULARIO DE TIEMPOS DE CICLO (GUARDAR Y APLICAR)
  // =========================================================================
  if (formTiempos) {
    formTiempos.addEventListener('submit', (e) => {
      e.preventDefault();

      // SET_TIEMPOS es del Maestro (SOLO_MAESTRO): el Esclavo no tiene esa rama y
      // contesta $ERR,CMD:DESCONOCIDO. Se pregunta ANTES de validar los tres numeros,
      // porque hacer rellenar bien un formulario cuya orden no se va a mandar es peor
      // que negarla de entrada.
      const puntaTiempos = puntaCorrecta('SET_TIEMPOS');
      if (puntaTiempos) { avisarOtraPunta('SET_TIEMPOS', puntaTiempos); return; }

      const verde = parseInt(numTiempoVerde.value, 10);
      const rojo = parseInt(numTiempoRojo.value, 10);
      const despeje = parseInt(numTiempoDespeje.value, 10);

      // TODA COMPARACION CON NaN ES FALSE, incluida la de "fuera de rango".
      //
      // Un campo vacio da parseInt('') === NaN, y con NaN las seis comparaciones de
      // la guarda anterior salian false, asi que la guarda dejaba pasar y al cable se
      // iba SET_TIEMPOS:NaN,2,15 -reproducido en node-. El firmware lo rechaza, pero
      // eso es suerte: la guarda de rangos de la app no estaba guardando nada. Se
      // pregunta primero si hay tres enteros, y el rango despues.
      const enRango = (n, min, max) => Number.isInteger(n) && n >= min && n <= max;
      // LOS RANGOS SON COPIA DEL C++, Y ESO ES LO PELIGROSO DE ESTA LINEA.
      //
      // Los cuatro numeros viven de verdad en Maestro/src/modo_automatico.cpp:32-34
      // -VERDE_MIN_MIN/MAX, ROJO_MIN_MIN/MAX, DESPEJE_SEG_MIN/MAX-. Aqui son una SEGUNDA
      // COPIA escrita a mano en otro lenguaje, que es exactamente lo que contrato.h llama
      // R-9: "el dia que difieran, una punta deja pasar lo que la otra rechaza".
      //
      // Se sincronizan el 04/09 con el minimo nuevo de 3 minutos, y a partir de hoy hay
      // un pack -app_11_rangos_de_tiempos- que relee los seis numeros del C++ y de aqui
      // en cada corrida y FALLA si divergen. Sin ese pack, esta linea envejece sola.
      //
      // Y LA APP NO ES LA GUARDA: la guarda esta en el firmware, que rechaza con
      // $ERR,CMD:SET_TIEMPOS,DESC:RANGO. Esto solo evita que el operario teclee algo que
      // el equipo va a rechazar y se quede sin saber por que.
      if (!enRango(verde, 3, 15) || !enRango(rojo, 3, 15) || !enRango(despeje, 10, 90)) {
        showToast('❌ Error: Tiempos vacíos o fuera de rango permitido.');
        return;
      }

      // Estos tres son la memoria del FORMULARIO, no telemetria.
      state.tiempoVerdeMin = verde;
      state.tiempoRojoMin = rojo;
      state.tiempoDespejeSeg = despeje;

      // AQUI SE ESCRIBIAN state.countdown Y state.countdownMax, que son los del
      // ANILLO DE TELEMETRIA, y se llamaba a updateCountdownRing(): el formulario
      // arrancaba una cuenta atras de verde*60 s que no habia mandado ningun equipo.
      // Es el panel de demo escribiendo en los mismos widgets que el dato real
      // (CLAUDE.md 3.quinquies). El contador lo pinta T: de $STATUS y nadie mas.
      if (!enviarComandoFirmware('SET_TIEMPOS', `${verde},${rojo},${despeje}`)) return;
      // "Enviada", no "guardados". Los tiempos los acepta o los rechaza el equipo
      // -SET_TIEMPOS es justo la rama que este repositorio usa de molde porque
      // pregunta dentro del `if` y tiene un $ERR por cada motivo-, y esa respuesta
      // llega por $ACK/$ERR, que ya tienen quien los pinte.
      showToast(`Orden enviada: Verde ${verde}m · Rojo ${rojo}m · Despeje ${despeje}s`);
      addEvent('cyan', `Orden SET_TIEMPOS enviada al equipo: Verde=${verde}min, Rojo=${rojo}min, Despeje=${despeje}seg. Espere el acuse.`);
    });
  }

  // =========================================================================
  // 8. ASISTENTE COURIER RTC & SINCRONIZACIÓN
  // =========================================================================
  if (btnCourierCapture) {
    btnCourierCapture.addEventListener('click', () => {
      // 24 h compuesta a mano. Con toLocaleTimeString() esto capturaba
      // "6:25:00 p. m." y el Courier lo partia por ':' -ver js/courier_rtc.js-, de
      // modo que compensaba los segundos de traslado con exactitud de reloj sobre una
      // hora 12 horas equivocada.
      const now = horaLocal24();
      state.courierSnapshot = CourierRTC.capturarMaestro(now, state.estadoLuces, state.countdown);
      state.courierSecondsElapsed = 0;

      if (courierSnapshotText) {
        courierSnapshotText.textContent = `Capturado en P1 a las ${now} (${state.estadoLuces})`;
        courierSnapshotText.style.color = 'var(--cyan-neon)';
      }

      if (btnCourierInject) {
        btnCourierInject.disabled = false;
        btnCourierInject.classList.add('step-inject-active');
      }

      if (state.courierTimerInterval) clearInterval(state.courierTimerInterval);
      state.courierTimerInterval = setInterval(() => {
        state.courierSecondsElapsed++;
        const mins = String(Math.floor(state.courierSecondsElapsed / 60)).padStart(2, '0');
        const secs = String(state.courierSecondsElapsed % 60).padStart(2, '0');
        if (courierTimerDigits) courierTimerDigits.textContent = `⏱️ ${mins}:${secs}`;
      }, 1000);

      showToast('📸 Hora del Maestro capturada. Inicie traslado a Poste 2.');
      addEvent('cyan', `Courier RTC: Captura en Maestro realizada a las ${now}.`);
    });
  }

  if (btnCourierInject) {
    btnCourierInject.addEventListener('click', () => {
      if (!state.courierSnapshot) return;
      if (state.courierTimerInterval) clearInterval(state.courierTimerInterval);

      // calcularCompensacion() ahora RECHAZA una hora que no sea HH:MM:SS en vez de
      // tragarsela: antes `Number("00 p. m.")` daba NaN y el `ss || 0` de al lado lo
      // convertia en 0 -NaN es falsy-, asi que no reventaba, seguia, y devolvia una
      // hora limpia y falsa. Si se niega, se dice; no se manda nada.
      let comp;
      try {
        comp = CourierRTC.calcularCompensacion(state.courierSnapshot, Date.now());
      } catch (e) {
        showToast('No se pudo compensar la hora capturada');
        addEvent('red', 'Courier RTC: no se inyectó nada. ' + e.message);
        return;
      }
      // La fecha se compone con los getters locales: toISOString() es UTC y desde las
      // 19:00 locales mandaba el dia siguiente.
      const today = fechaLocalISO();
      if (!enviarComandoFirmware('SET_RTC', `${today},${comp.horaCompensada}`)) return;
      // "Enviada", no "exitosa". Esta app no sabe si el Esclavo puso la hora: su
      // despachador contesta $ERR,SIN_CRISTAL o $ERR,FORMATO_INVALIDO cuando no puede
      // -Esclavo/src/bluetooth.cpp:251-257-, y con Y2 muerto en hardware (N-17) ese
      // rechazo es hoy la respuesta habitual, no la rara.
      showToast(`Orden enviada al Esclavo: ${today} ${comp.horaCompensada}`);
      addEvent('cyan', `Courier RTC: orden SET_RTC enviada (${today} ${comp.horaCompensada}, ` +
                       `traslado ${comp.elapsedSeg}s). Espere el acuse del equipo.`);

      if (btnCourierInject) btnCourierInject.disabled = true;
    });
  }

  if (btnSyncRtc) {
    btnSyncRtc.addEventListener('click', () => {
      // Una sola lectura del reloj para los dos campos: con dos `new Date()` la fecha
      // y la hora pueden caer a distinto lado de la medianoche.
      const ahora = new Date();
      const now = horaLocal24(ahora);
      const today = fechaLocalISO(ahora);
      if (!enviarComandoFirmware('SET_RTC', `${today},${now}`)) return;
      // NO SE NOMBRA NINGUNA PIEZA -"RTC DS3231"-: es lo mismo que N-45 quito de la
      // pantalla del equipo por senalar un componente sin haberlo medido, y ademas el
      // DS3231 hoy cuelga del ESP32, no de esta puerta.
      //
      // Y NO SE DICE "sincronizado": el Maestro tiene TRES finales para este comando
      // -FORMATO_INVALIDO, SIN_CRISTAL_VEA_CONSULTA_RELOJ y OK
      // (Maestro/src/bluetooth.cpp:307-330)-, y con Y2 confirmado muerto en hardware
      // (N-17) el del medio es el habitual. La app solo sabe que la orden salio.
      showToast(`Orden de ajuste de hora enviada: ${today} ${now}`);
      addEvent('cyan', `Orden SET_RTC enviada al equipo con la hora del celular: ${today} ${now}. ` +
                       `Espere el acuse; si no llega, el reloj NO quedó puesto.`);
    });
  }

  if (btnStartTestLeds) {
    btnStartTestLeds.addEventListener('click', () => {
      // AL ESCLAVO NO SE LE MANDA NADA QUE MUEVA LUCES.
      //
      // El Esclavo rechaza TEST_LEDS a proposito (Esclavo/src/bluetooth.cpp:146-157):
      // la secuencia enciende VERDE sin mirar nada, y ese verde sale mientras el
      // Maestro puede estar dando paso al otro sentido. La app lo mandaba igual y
      // ademas pintaba la secuencia completa por su cuenta, asi que sobre un Esclavo
      // el tecnico veia un test que se ejecutaba y terminaba "con exito" mientras el
      // equipo lo habia rechazado. Se para aqui, en vez de gastar un $ERR.
      if (state.node === 'ESCLAVO') {
        showToast('El Esclavo no acepta el test de luces: use el Maestro');
        addEvent('red', 'Test de focos no enviado: esta punta es el ESCLAVO y mover ' +
                        'luces desde el subordinado no esta permitido (use el Maestro).');
        return;
      }
      if (!state.pinVerificado) { pedirPin(() => btnStartTestLeds.click()); return; }
      enviarComandoFirmware('TEST_LEDS');
      // Ni la secuencia ni el "completado con exito" se pintan: la app no puede saber
      // si el equipo la ejecuto ni como acabo. Lo dira $ACK, y las luces $STATUS.
      addEvent('cyan', 'Tecnico: orden TEST DE FOCOS enviada al equipo.');
    });
  }

  // =========================================================================
  // 9. NAVEGACIÓN INFERIOR (TABS)
  // =========================================================================
  const navItems = document.querySelectorAll('.nav-item');
  const tabContents = document.querySelectorAll('.tab-content');

  navItems.forEach(tabBtn => {
    tabBtn.addEventListener('click', () => {
      const targetId = tabBtn.getAttribute('data-tab');
      navItems.forEach(b => b.classList.remove('active'));
      tabContents.forEach(tc => tc.classList.remove('active'));

      tabBtn.classList.add('active');
      const targetEl = document.getElementById(targetId);
      if (targetEl) {
        targetEl.classList.add('active');
        window.scrollTo({ top: 0, behavior: 'smooth' });
      }
      // La cinta de tramas no se repinta mientras esta pestana esta cerrada -a un
      // $STATUS por segundo eso serian sesenta filas de DOM por segundo gastando
      // bateria para nadie-, asi que al abrirla hay que pintarla aqui o se abriria en
      // blanco enseñando "todavia no ha entrado ninguna trama" con la cinta llena.
      if (targetId === 'tab-depuracion') renderDepuracion();
    });
  });

  // =========================================================================
  // 10. MODALES Y TECLADO PIN
  // =========================================================================
  function openModal(modal) {
    if (modal) modal.classList.add('active');
  }

  function closeModal(modal) {
    if (!modal) return;
    modal.classList.remove('active');
    // Cerrar el teclado es CANCELAR, y hasta hoy no cancelaba nada. Quedaban dos
    // restos vivos detras de un modal que ya nadie ve:
    //
    //   state.pin            los cuatro digitos BUENOS seguian en memoria despues de
    //                        una validacion correcta -validatePin() no los borraba-,
    //                        asi que un OK con el teclado cerrado volvia a armar la
    //                        sesion sin teclear un solo digito. Mientras pinVerificado
    //                        no caduque eso no cambia nada; el dia que caduque, ese
    //                        resto convierte la caducidad en un adorno.
    //
    //   state.accionPendiente  la orden que pedirPin() dejo en cola sobrevivia al
    //                        cierre, y la disparaba la SIGUIENTE validacion, que puede
    //                        ser de otro operario y por otro motivo -subir a Tecnico-.
    //                        Una orden que mueve luces no se ejecuta media hora tarde
    //                        porque alguien tecleo la clave para otra cosa.
    if (modal === pinModal) {
      state.pin = '';
      state.accionPendiente = null;
      updatePinDisplay();
    }
  }

  if (btnDevice && btModal) {
    btnDevice.addEventListener('click', () => {
      openModal(btModal);
      // N-124: la lista ya no viene en el HTML, asi que el modal se abriria vacio y
      // haria falta un segundo toque para ver algo. Con radio se rellena al abrir -es
      // la misma llamada que hace el boton-; sin radio esto vuelve en silencio y queda
      // el aviso del HTML diciendo que hay que pulsar Buscar.
      escanearEquipos(false);
    });
  }
  if (modalBtClose && btModal) {
    modalBtClose.addEventListener('click', () => closeModal(btModal));
  }

  if (btnSelectSite && siteModal) {
    btnSelectSite.addEventListener('click', () => {
      renderSiteList();
      openModal(siteModal);
    });
  }
  if (modalSiteClose && siteModal) {
    modalSiteClose.addEventListener('click', () => closeModal(siteModal));
  }

  [btModal, siteModal, pinModal, degradadoModal].forEach(m => {
    if (m) {
      m.addEventListener('click', (e) => {
        if (e.target === m) closeModal(m);
      });
    }
  });

  function openPinModal() {
    state.pin = '';
    updatePinDisplay();
    openModal(pinModal);
  }

  function updatePinDisplay() {
    pinDots.forEach((dot, idx) => {
      if (idx < state.pin.length) {
        dot.classList.add('filled');
      } else {
        dot.classList.remove('filled');
      }
    });
  }

  if (modalPinClose) {
    modalPinClose.addEventListener('click', () => closeModal(pinModal));
  }

  // LA BARRERA ES EL TECLADO ABIERTO, ASI QUE HAY QUE PREGUNTARLE AL TECLADO.
  //
  // Los tres manejadores de abajo escribian en state.pin y llamaban a validatePin()
  // sin mirar si el modal estaba delante del operario. Con el teclado cerrado los
  // botones siguen en el arbol -solo estan ocultos por CSS-, asi que cualquier cosa
  // que dispare un click sobre ellos ARMA state.pinVerificado sin que la barrera se
  // haya abierto nunca: es el estado de una barrera armado por fuera de la barrera.
  //
  // Que nadie llegue ahi con el dedo no lo vuelve teorico. En N-83 el arnes tecleaba
  // 1234 sobre un modal que la guarda de punta jamas abrio, la sesion se autorizaba
  // sola, y SOLICITAR_PASO seguia dando [OK] mientras seis comprobaciones de al lado
  // caian: el defecto no se cobro en la calle, se cobro QUITANDOLE CAPACIDAD DE
  // DETECTAR A OTRA PRUEBA, que es la forma cara de pagarlo.
  function tecladoPinAbierto() {
    return !!pinModal && pinModal.classList.contains('active');
  }

  const pinButtons = document.querySelectorAll('.pin-btn[data-key]');
  pinButtons.forEach(btn => {
    btn.addEventListener('click', () => {
      if (!tecladoPinAbierto()) return;
      if (state.pin.length < 4) {
        state.pin += btn.getAttribute('data-key');
        updatePinDisplay();
        if (state.pin.length === 4) {
          validatePin();
        }
      }
    });
  });

  if (btnPinClear) {
    btnPinClear.addEventListener('click', () => {
      if (!tecladoPinAbierto()) return;
      state.pin = state.pin.slice(0, -1);
      updatePinDisplay();
    });
  }

  if (btnPinOk) {
    btnPinOk.addEventListener('click', () => {
      if (!tecladoPinAbierto()) return;
      if (state.pin.length === 4) validatePin();
    });
  }

  function validatePin() {
    // La guarda se repite AQUI a proposito, y no es un adorno de la de arriba: esta
    // es la unica linea del fichero que arma state.pinVerificado. Guardar solo las
    // tres entradas deja la puerta abierta a la CUARTA que alguien anada manana; los
    // manejadores frenan la pulsacion, esta frena la autorizacion.
    if (!tecladoPinAbierto()) return;
    if (state.pin === state.correctPin) {
      // Se recoge ANTES de cerrar porque cerrar el teclado ya cancela la cola (ver
      // closeModal): esta accion no se cancela, se acaba de autorizar.
      // Dos usos distintos: autorizar un comando pendiente NO asciende a TECNICO.
      // El operario que da paso desde el suelo no queda con el menu de ajustes abierto.
      const accion = state.accionPendiente;
      closeModal(pinModal);
      state.pinVerificado = true;
      if (accion) { accion(); return; }
      setRole('TECNICO');
    } else {
      showToast('❌ PIN Incorrecto. Reintente.');
      state.pin = '';
      updatePinDisplay();
    }
  }

  // =========================================================================
  // 11. EXPORTACIÓN WHATSAPP Y CSV
  // =========================================================================
  if (btnShareWhatsapp) {
    btnShareWhatsapp.addEventListener('click', () => {
      let r = `🚦 *REPORTE SEMAFÓRICO IOT-VIAL*\n`;
      r += `📍 *Cruce:* ${state.site}\n`;
      // La punta puede no estar identificada todavia -N-124: ya no se adivina al
      // pulsar la fila-, y este reporte SALE DE LA APP: es el sitio donde menos
      // puede aparecer la palabra `null` haciendose pasar por un dato.
      r += `👑 *Nodo:* ${state.node || 'sin identificar'} | Modo: ${state.modo || 'sin datos'}\n`;
      // "Enlace RF: null%" es lo que salia aqui: state.rfQuality no lo escribia el
      // camino de $STATUS -solo el del puente de PC, que ya no existe- asi que el
      // reporte que el tecnico manda por WhatsApp publicaba la palabra `null`. Ahora
      // se dice el valor con SU HORA, o se declara que no se midio. Un reporte que
      // viaja fuera de la app es el sitio donde menos puede haber un numero sin sello.
      const enlaceTexto = state.rfQuality === null
        ? 'no medido en esta sesion'
        : `${state.rfQuality}% de latidos contestados (medido a las ${_horaDe(state.rfMedidaMs)})`;
      const bateriaTexto = state.battery === null
        ? 'no medida por el equipo' : `${state.battery.toFixed(1)} V`;
      r += `🔋 *Batería:* ${bateriaTexto} | *Enlace:* ${enlaceTexto}\n`;
      r += `_El enlace es el % de latidos que contestaron, no potencia de señal._\n\n`;
      r += `*Últimos Eventos:*\n`;
      state.events.slice(0, 5).forEach(ev => {
        r += `• [${ev.time}] ${ev.msg}\n`;
      });
      window.open(`https://api.whatsapp.com/send?text=${encodeURIComponent(r)}`, '_blank');
    });
  }

  if (btnExportCsv) {
    btnExportCsv.addEventListener('click', () => {
      let csv = 'Timestamp,Tipo,Mensaje\n';
      state.events.forEach(ev => {
        csv += `"${ev.time}","${ev.type}","${ev.msg}"\n`;
      });
      const blob = new Blob([csv], { type: 'text/csv;charset=utf-8;' });
      const link = document.createElement('a');
      link.href = URL.createObjectURL(blob);
      link.download = `Log_Semaforos_${state.site.replace(/[\s\·\/]+/g, '_')}.csv`;
      link.click();
      showToast('📥 Archivo CSV descargado.');
    });
  }

  // =========================================================================
  // 12. SIMULADOR DE PRUEBAS RÁPIDAS (DEMO)
  // =========================================================================
  // =========================================================================
  // 13. TICKER PERIÓDICO (SINCRONIZACIÓN EN VIVO CON PUENTE PYTHON STM32)
  // =========================================================================
  // N-75: AQUI HABIA UN CICLO DE SEMAFORO SIMULADO, Y CORRIA SOLO.
  //
  // Mientras no hubiera equipo conectado, esta funcion avanzaba el contador y rotaba
  // verde -> ambar -> todo-rojo -> verde del otro lado, indefinidamente. La pantalla
  // mostraba un cruce completo, creible y en movimiento SIN QUE NADIE LO PIDIERA y sin
  // que hubiera nada al otro lado del Bluetooth. Es el mismo defecto que el panel de
  // demo que se retiro con el, pero peor: alli habia que pulsar un boton.
  //
  // Lo que hace ahora es no saber, y decirlo. Las luces se quedan como estaban -no se
  // inventa un estado "apagado" que tampoco consta- y el rotulo lo declara. Un tablero
  // quieto que admite que no tiene datos es honesto; uno que anima un cruce que no
  // existe le esta mintiendo a alguien que esta de pie en la calzada.
  // UN "ENLAZADO" QUE NO CADUCA ES UN DATO INVENTADO CON OTRO NOMBRE.
  //
  // state.connected se ponia a true al pulsar el equipo en el modal y NO VOLVIA A
  // false nunca: ni al apagarse el poste, ni al salirse de cobertura, ni al agotarse
  // la bateria. El rotulo seguia diciendo "Enlazado" y el tablero seguia mostrando la
  // ultima fase recibida, quieta, indistinguible de un cruce que no cambia de fase
  // porque le toca. Las dos puntas emiten $STATUS CADA SEGUNDO, asi que el silencio
  // es medible: pasados TIMEOUT_ENLACE_MS sin una sola trama, lo que hay no es un
  // tablero al dia, es un tablero sin datos, y se dice.
  //
  // Y son DOS banderas, no una. state.connected dice que el socket serie esta abierto
  // y es lo unico que puede autorizar un TX; state.telemetriaViva dice que el equipo
  // esta HABLANDO y es lo unico que puede autorizar a pintar un dato. Colgar el TX del
  // watchdog dejaria el ROJO DE EMERGENCIA sin salir justo cuando la telemetria se ha
  // caido, que es cuando mas falta hace poder parar el trafico.
  const TIMEOUT_ENLACE_MS = 5000;

  function marcarConEnlace() {
    const estabaCaido = !state.telemetriaViva;
    state.telemetriaViva = true;
    if (btStatusDot) btStatusDot.className = 'status-dot connected';
    if (btBtnText) btBtnText.textContent = 'Enlazado';
    actualizarDemanda();
    if (estabaCaido && state.huboCaida) {
      state.huboCaida = false;
      // El REGRESO se anota sin valor de enlace a proposito: esta linea se escribe con
      // la PRIMERA trama que vuelve, y en ese instante todavia no se ha leido su RF.
      // La muestra con el numero llega inmediatamente despues, con su propia hora.
      RegistroEnlace.anotar('REGRESO', ENLACE_SIN_DATO,
                            'el equipo vuelve a emitir telemetria');
      renderRegistroEnlace();
    }
  }

  function marcarSinEnlace() {
    const eraConectado = state.telemetriaViva;
    state.telemetriaViva = false;

    if (btStatusDot) btStatusDot.className = 'status-dot';
    if (btBtnText) btBtnText.textContent = 'Sin enlace';
    if (btnDevice) btnDevice.className = 'btn-top btn-device';

    // Las luces se dejan como estaban a proposito: apagarlas seria afirmar que el
    // cruce esta apagado, y eso tampoco consta. Lo que se retira es todo numero que
    // envejece -contador, RF, RTT, bateria, hora-, porque un numero viejo pintado
    // como si fuera de ahora es exactamente el dato inventado que se quiere evitar.
    if (phaseDescEl) phaseDescEl.textContent = 'SIN ENLACE - sin datos del equipo';
    if (cdNumEl) cdNumEl.textContent = '--';
    // El enlace se retira POR EL MISMO CAMINO que lo pinta, con la lectura vacia.
    // Escribir aqui '--' a mano abriria un segundo escritor de esos widgets, y con dos
    // escritores vuelve a poder aparecer uno que pinte algo que no vino en una trama.
    pintarEnlace(ENLACE_SIN_DATO);
    if (batVoltageEl) batVoltageEl.textContent = '-- V';
    if (batStatusEl) batStatusEl.textContent = 'Sin datos del equipo';
    if (rssiTextEl) rssiTextEl.textContent = '(sin enlace)';
    if (s1Text) { s1Text.textContent = 'SIN DATOS'; s1Text.style.color = 'var(--text-muted)'; }
    if (s2Text) { s2Text.textContent = 'SIN DATOS'; s2Text.style.color = 'var(--text-muted)'; }
    if (badgeModoEl) {
      badgeModoEl.className = 'badge badge-sin-enlace';
      // Los tres estilos en linea se limpian porque renderLights() los deja puestos al
      // pintar MANUAL/AMBAR/ROJO: sin esto, el badge de "sin enlace" se quedaria con el
      // color del ultimo modo conocido y seguiria pareciendo un modo vigente.
      badgeModoEl.style.background = '';
      badgeModoEl.style.borderColor = '';
      badgeModoEl.style.color = '';
      badgeModoEl.textContent = 'SIN ENLACE';
    }

    // Sin telemetria no se sabe el modo, asi que el control que depende de el se
    // apaga. Dejarlo habilitado con el ultimo modo conocido seria decidir con un dato
    // vencido, que es lo mismo que se retira dos lineas mas arriba con los numeros.
    actualizarDemanda();

    if (eraConectado) {
      addEvent('red', 'Enlace perdido: el equipo lleva mas de ' +
                      (TIMEOUT_ENLACE_MS / 1000) + ' s sin emitir telemetria.');
      state.huboCaida = true;
      // LA CAIDA SE ANOTA SIN VALOR DE ENLACE. En este instante no se ha medido nada
      // -por eso es una caida-, asi que la columna del enlace queda VACIA. El ultimo
      // valor que si se midio va en el texto CON SU HORA, que es como se lee sin
      // confundirlo con una medida de ahora: "iba al 40% a las 14:35, se fue a las
      // 14:36". Poner ese 40 en la columna diria que a las 14:36 se midio 40.
      const ultimo = state.rfQuality === null
        ? 'no se llego a medir el enlace en esta sesion'
        : 'la ultima medida fue ' + state.rfQuality + '% a las ' + _horaDe(state.rfMedidaMs);
      RegistroEnlace.anotar('CAIDA', ENLACE_SIN_DATO,
                            'silencio de mas de ' + (TIMEOUT_ENLACE_MS / 1000) +
                            ' s; ' + ultimo);
      state.rfTramo = null;
      renderRegistroEnlace();
    }
  }

  function vigilarEnlace() {
    if (!state.ultimoStatusMs) { marcarSinEnlace(); return; }
    if (Date.now() - state.ultimoStatusMs > TIMEOUT_ENLACE_MS) marcarSinEnlace();
  }


  // AQUI HABIA UN SEGUNDO ORIGEN DE TELEMETRIA, Y ERA UN JSON QUE NADIE EMITE.
  //
  // Cada segundo se pedia /api/status_json y, si contestaba, se escribian a pelo el
  // modo, el estado de las luces, el contador, la bateria, el RF y el RTT -incluido
  // `rfQualityEl.textContent = ${data.rf}%`-. O sea: un camino que pintaba el tablero
  // entero, y el indicador de enlace, SIN QUE HUBIERA PASADO UNA SOLA TRAMA. Es la
  // pantalla de campo hablando el idioma de un simulador, que es el defecto que
  // app_04_valores_de_status existe para prohibir.
  //
  // Y MEDIDO: ese endpoint YA NO EXISTE. La cascara de demo lo retiro a proposito y
  // dejo escrito el porque -servidor_puente_simulador.py:186: "NO existe
  // /api/status_json, y su 404 es la respuesta CORRECTA"-. De modo que esto llevaba
  // desde entonces cayendo al .catch() en cada tick: codigo muerto, pero de la clase
  // que despierta el dia que alguien monte cualquier servidor que conteste ahi. Un
  // camino que puede pintar un cruce inventado no se deja dormido; se quita.
  //
  // Lo que queda es el reloj de la cabecera. El enlace lo vigila el watchdog de abajo,
  // que mira la unica senal que sirve en las dos vias: cuando hablo el equipo.
  setInterval(() => {
    const now = new Date();
    if (rtcSyncDigits) rtcSyncDigits.textContent = now.toTimeString().split(' ')[0];
  }, 1000);

  // Inicialización: el tablero nace declarando que no tiene datos, no fingiendo un
  // cruce en marcha. En cuanto llegue el primer $STATUS lo pintara la telemetria.
  renderEvents();
  // La bitacora del enlace SI sobrevive al cierre de la app -es su motivo de existir-,
  // asi que se pinta antes de nada: lo primero que quiere ver quien abre la app tras
  // una caida nocturna es a que hora se fue, no el estado de ahora.
  renderRegistroEnlace();
  // Y la cinta de tramas nace VACIA Y DICIENDOLO. Es la vista donde mas tentador seria
  // dejar un ejemplo "para que se vea el formato": no lo lleva.
  renderDepuracion();
  marcarSinEnlace();
  // Arranca sin punta: los dos mandos de emergencia a la vista, cada uno con su poste
  // escrito. En cuanto se sepa cual hay delante quedara solo el que corresponde.
  actualizarEmergencia();
  setInterval(vigilarEnlace, 1000);
});
